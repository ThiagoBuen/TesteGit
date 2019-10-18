/*
	NovaGenesis

	Name:		CoreRunEvaluate01
	Object:		CoreRunEvaluate01
	File:		CoreRunEvaluate01.cpp
	Author:		Antonio M. Alberti
	Date:		12/2012
	Version:	0.1
*/

#ifndef _CORERUNEVALUATE01_H
#include "CoreRunEvaluate01.h"
#endif

#ifndef _IOTTESTAPP_H
#include "IoTTestApp.h"
#endif

#ifndef _CORE_H
#include "Core.h"
#endif

#ifndef _GW_H
#include "GW.h"
#endif

CoreRunEvaluate01::CoreRunEvaluate01(string _LN, Block *_PB, MessageBuilder *_PMB):Action(_LN,_PB,_PMB)
{
	// Send hello
	//zmq::message_t request (6);
	//memcpy ((void *) request.data (), "Hello", 5);
	//std::cout << "Sending Hello " << std::endl;
	//socket.send (request);

	// Get the reply.
	//zmq::message_t reply;
	//socket.recv (&reply);
	//std::cout << "Received" << std::endl;

	TimeToExpose=0;

	// Set the file name
	string DataFileName="Data.txt";

	// Open the file to write
	F2.OpenOutputFile(DataFileName,PB->GetPath(),"DEFAULT");
}

CoreRunEvaluate01::~CoreRunEvaluate01()
{
}

// Run the actions behind a received command line
// ng -run --evaluate 0.1
int CoreRunEvaluate01::Run(Message *_ReceivedMessage, CommandLine *_PCL, vector<Message*> &ScheduledMessages, Message *&InlineResponseMessage)
{
	int 					Status=OK;

	string					Offset = "                    ";
	Core					*PCore=0;
	IoTTestApp				*PIoTTestApp=0;
	bool					ClearScheduledMessage=true;
	double					Time=GetTime();

	PCore=(Core*)PB;

	PIoTTestApp=(IoTTestApp*)PB->PP;

	//PB->S << Offset <<  this->GetLegibleName() << endl;

	PB->S << Offset <<  "(Time = "<<Time<<")"<<endl;

	PB->S << Offset <<  "(NextPeerEvaluationTime = " << PCore->NextPeerEvaluationTime << ")"<<endl;

	CheckForPSSAwareness(ScheduledMessages,ClearScheduledMessage);

	CheckForNewPeerApplication(ScheduledMessages,ClearScheduledMessage);

	ShowTheDiscoveredPeers();

	CheckSubscriptions(ScheduledMessages,ClearScheduledMessage);

	if (ClearScheduledMessage == true)
		{
			if (ScheduledMessages.size() > 0)
				{
					Message *Temp=ScheduledMessages.at(0);

					Temp->MarkToDelete();

					// Make the scheduled messages vector empty
					ScheduledMessages.clear();
				}
		}

	//PB->S << Offset <<  "(Deleting the previous marked messages)" << endl;

	//PB->S << Offset <<  "(Done)" << endl << endl << endl;

	return Status;
}


int CoreRunEvaluate01::CheckForPSSAwareness(vector<Message*> &_ScheduledMessages, bool &_ClearScheduledMessage)
{
	int 					Status=OK;

	string					Offset = "                    ";
	string					Offset1 = "                              ";
	Core					*PCore=0;
	vector<string> 			*Values=new vector<string>;
	vector<Tuple*>  		PSSs;

	PCore=(Core*)PB;

	PB->S << Offset <<  "(1. Check for PSS awareness.)"<<endl;

	// *************************************************************
	// Check for PSS discovery first Step
	// *************************************************************
	if (PB->PP->DiscoverHomonymsEntitiesIDsFromLN(2,"PSS",Values,PB) == ERROR)
		{
			vector<string> Cat2Keywords;

			Cat2Keywords.push_back("OS");
			Cat2Keywords.push_back("PSS");
			Cat2Keywords.push_back("PS");

			vector<string> Cat9Keywords;

			Cat9Keywords.push_back("Host");

			// Schedule a first step of PSS discovery
			PCore->DiscoveryFirstStep(PB->PP->Intra_OS,&Cat2Keywords,&Cat9Keywords,_ScheduledMessages);

			PB->S << Offset1 << "(Not aware of any PSS on Categories 2 and 9. Prepare discover first step)"<<endl;

			_ClearScheduledMessage=false;
		}
	else
		{
			PB->S << Offset1 << "(Aware of a PSS on Categories 2 and 9)"<<endl;
		}

	// *************************************************************
	// Check for PSS discovery second Step
	// *************************************************************
	if (PB->PP->DiscoverHomonymsEntitiesTuplesFromProcessAndBlockLegibleNames("PSS","PS",PSSs,PB) == ERROR)
		{
			vector<string> Cat2Keywords;

			Cat2Keywords.push_back("OS");
			Cat2Keywords.push_back("PSS");
			Cat2Keywords.push_back("PS");

			vector<string> Cat9Keywords;

			Cat9Keywords.push_back("Host");

			// Schedule a second step of PSS discovery
			PCore->DiscoverySecondStep(PB->PP->Intra_OS,&Cat2Keywords,&Cat9Keywords,_ScheduledMessages);

			PB->S << Offset1 << "(Not aware of any PSS on Categories 5 and 6. Prepare discover second step)"<<endl;

			_ClearScheduledMessage=false;
		}
	else
		{
			// Test storage of new PSS if there is some candidate
			if (PSSs.size() > 0)
				{
					// Loop over the discovered candidates
					for (unsigned int g=0; g<PSSs.size(); g++)
						{
							// Auxiliary flag
							bool StoreFlag=true;

							// Loop over the already stored tuples
							for (unsigned int h=0; h<PCore->PSSTuples.size(); h++)
								{
									if(PSSs[g]->Values[2] == PCore->PSSTuples[h]->Values[2])
										{
											StoreFlag=false;

											PB->S << Offset1 << "(The IoTTestApp is already aware of this PSS)"<<endl;
										}
								}

							if (StoreFlag == true)
								{
									PCore->PSSTuples.push_back(PSSs[g]);

									PB->S << Offset1 << "(Discovered a PSS)"<<endl;
								}
						}
				}
			else
				{
					PB->S << Offset1 << "(There is no candidate for PSS)"<<endl;
				}

			// Run other procedures if it is aware of at least one PSS
			if (PCore->PSSTuples.size() > 0)
				{
					if (PCore->RunExpose == true && TimeToExpose < GetTime())
						{
							PB->S << Offset1 << "(Run expose until find the peer)"<<endl;

							PCore->Exposition(PB->PP->GetDomainSelfCertifyingName(),_ScheduledMessages);

							_ClearScheduledMessage=false;

							PB->State="operational";

							TimeToExpose=GetTime()+10;
						}
				}
		}

	return Status;
}

int CoreRunEvaluate01::CheckForNewPeerApplication(vector<Message*> &_ScheduledMessages, bool &_ClearScheduledMessage)
{
	int Status=ERROR;

	string					Offset = "                    ";
	string					Offset1 = "                              ";
	Core					*PCore=0;
	IoTTestApp				*PIoTTestApp=0;
	vector<string> 			*Candidates=new vector<string>;
	vector<string> 			*Subject=new vector<string>;
	Message 				*Run=0;
	vector<Tuple*>  		Apps;
	CommandLine				*PCL=0;
	double					Time=GetTime();

	PCore=(Core*)PB;

	PIoTTestApp=(IoTTestApp*)PB->PP;

	PB->S << Offset << "(2. Check for new peer application tuples)"<<endl;

	// *************************************************************
	// Check for new peer app tuples
	// *************************************************************
	if (PCore->PSSTuples.size() > 0)
		{
			if (PCore->NextPeerEvaluationTime < Time)
				{
					// *******************
					// Check for a PGCS
					// *******************
					if (PB->PP->DiscoverHomonymsEntitiesIDsFromLN(2,"Gateway",Candidates,PB) == OK &&
						PB->PP->DiscoverHomonymsEntitiesIDsFromLN(2,"Controller",Subject,PB) == OK &&
						PB->PP->DiscoverHomonymsEntitiesIDsFromLN(2,"Proxy",Subject,PB) == OK &&
						PB->PP->DiscoverHomonymsEntitiesTuplesFromProcessAndBlockLegibleNames("PGCS","Core",Apps,PB) == OK)
						{
							// Check for new servers
							for (unsigned int i=0; i<Candidates->size(); i++)
								{
									PB->S << Offset1 << "(Testing the candidate = " << Candidates->at(i) << ")" << endl;

									// Is the server different than this process
									if (Candidates->at(i) != PB->PP->GetSelfCertifyingName())
										{
											for (unsigned int j=0; j<Subject->size(); j++)
												{
													// Has the peer the same subject?
													if (Candidates->at(i) == Subject->at(j))
														{
															// Run over Apps
															for (unsigned int k=0; k<Apps.size(); k++)
																{
																	// Is the server an application with core block
																	if (Candidates->at(i) == Apps[k]->Values[2])
																		{
																			// Auxiliary flag
																			bool StoreFlag=true;

																			// Loop over the already stored server tuples
																			for (unsigned int h=0; h<PCore->PeerAppTuples.size(); h++)
																				{
																					// Is the server already stored?
																					if(Candidates->at(i) == PCore->PeerAppTuples[h]->Values[2])
																						{
																							StoreFlag=false;
																						}
																					else
																						{
																							PB->S << Offset1 << "(The candidate peer is already stored)"<<endl;
																						}
																				}

																			if (StoreFlag == true)
																				{
																					// Store the learned tuple on the peer server app tuples
																					PCore->PeerAppTuples.push_back(Apps[k]);

																					unsigned int _I=PCore->PeerAppTuples.size()-1;

																					PB->S << Offset1 << "(Discovered a PGCS)" << endl;

																					PCore->PeerAppTuples[_I]->LN="PGCS";
																					PB->S << Offset1 << "(HID = " << PCore->PeerAppTuples[_I]->Values[0] << ")" << endl;
																					PB->S << Offset1 << "(OSID = " << PCore->PeerAppTuples[_I]->Values[1] << ")" << endl;
																					PB->S << Offset1 << "(PID = " << PCore->PeerAppTuples[_I]->Values[2] << ")" << endl;
																					PB->S << Offset1 << "(BID = " << PCore->PeerAppTuples[_I]->Values[3] << ")" << endl;

																					PCore->DelayBeforeRunPeriodic=60;

																					PCore->RunExpose = false;

																					if (_ScheduledMessages.size() > 0)
																						{
																							PB->S << Offset1 << "(There is a scheduled message with a ng -invite command line)" << endl;

																							Run=_ScheduledMessages.at(0);

																							if (Run != 0)
																								{
																									// Adding a ng -run --invitation command line
																									Run->NewCommandLine("-run","--invite","0.1",PCL);

																									PCL->NewArgument(2);

																									PCL->SetArgumentElement(0,0,"PGCS");

																									PCL->SetArgumentElement(0,1,PB->IntToString(_I));

																									_ClearScheduledMessage=false;

																									PCore->DelayBeforeRunPeriodic=30;
																								}
																						}
																				}
																			else
																				{
																					PB->S << Offset1 << "(The IoTTestApp is already aware of this candidate)"<<endl;
																				}
																		}
																}
														}
												}
										}
									else
										{
											PB->S << Offset1 << "(The candidate is the own process)"<<endl;
										}
								}
						}

					// Clear the vectors
					Subject->clear();
					Apps.clear();

					PCore->NextPeerEvaluationTime=Time+PCore->DelayBeforeANewPeerEvaluation;
				}
			else
				{
					PB->S << Offset1 << "(Too early for that. Wait next ng -run --evaluate)"<<endl;
				}
			}
		else
			{
				PB->S << Offset1 << "(Waiting for PSS discovery)"<<endl;
			}

	delete Candidates;
	delete Subject;

	return Status;
}

int CoreRunEvaluate01::ShowTheDiscoveredPeers()
{
	int Status=ERROR;

	string					Offset = "                    ";
	string					Offset1 = "                              ";
	Core					*PCore=0;
	IoTTestApp				*PIoTTestApp=0;

	PCore=(Core*)PB;

	PIoTTestApp=(IoTTestApp*)PB->PP;

	PB->S << Offset <<  "(3. Show the discovered peer App(s))"<<endl;

	// *************************************************************
	// Show the discovered server App(s)
	// *************************************************************
	if (PCore->PeerAppTuples.size() > 0)
		{
			for (unsigned int i=0; i<PCore->PeerAppTuples.size(); i++)
				{
					PB->S << Offset1 << "(Aware of the Application " << i << " )" << endl;

					PB->S << Offset1 << "(LN = " << PCore->PeerAppTuples[i]->LN << ")" << endl;
					PB->S << Offset1 << "(HID = " << PCore->PeerAppTuples[i]->Values[0] << ")" << endl;
					PB->S << Offset1 << "(OSID = " << PCore->PeerAppTuples[i]->Values[1] << ")" << endl;
					PB->S << Offset1 << "(PID = " << PCore->PeerAppTuples[i]->Values[2] << ")" << endl;
					PB->S << Offset1 << "(BID = " << PCore->PeerAppTuples[i]->Values[3] << ")" << endl;
				}
		}
	else
		{
			PB->S << Offset1 << "(Not aware of any peer application)" << endl;
		}

	return Status;
}

int CoreRunEvaluate01::CheckSubscriptions(vector<Message*> &_ScheduledMessages, bool _ClearScheduledMessage)
{
	int Status=ERROR;

	string					Offset = "                    ";
	string					Offset1 = "                              ";
	Core					*PCore=0;
	IoTTestApp				*PIoTTestApp=0;
	unsigned int 			Index;
	Subscription 			*PS=0;

	PCore=(Core*)PB;

	PIoTTestApp=(IoTTestApp*)PB->PP;

	PB->S << Offset <<  "(4. Check subscriptions)"<<endl;

	// *************************************************************
	// Check subscriptions
	// *************************************************************
	if (PCore->PSSTuples.size() > 0)
		{
			// Looping over existent Subscriptions
			for (unsigned int i=0; i<PCore->Subscriptions.size(); i++)
				{
					PS=PCore->Subscriptions[i];

					PB->S << Offset1 << "(Testing subscription "<<i<<")" << endl;

					PB->S << Offset1 << "(Subscription status is "<<PS->Status<<")" << endl;

					string 	Publisher_LN="Unknown";

					if (PS->Publisher.Values.size() > 0 &&
					    PCore->GetPeerAppTupleIndex(PS->Publisher.Values[2],Index) == OK)
						{
							Publisher_LN=PCore->PeerAppTuples[Index]->LN;

							PB->S << Offset1 << "(The publisher is a "<<Publisher_LN<<" and have the index "<< Index << ")" << endl;

							PB->S << Offset1 << "(HID = " << PS->Publisher.Values[0] << ")" << endl;
							PB->S << Offset1 << "(OSID = " << PS->Publisher.Values[1] << ")" << endl;
							PB->S << Offset1 << "(PID = " << PS->Publisher.Values[2] << ")" << endl;
							PB->S << Offset1 << "(BID = " << PS->Publisher.Values[3] << ")" << endl;
						}
					else
						{
							PB->S << Offset1 << "(Warning: The publisher is unknown)" << endl;
						}

					// Subscription requiring a scheduling
					if (PS->Status == "Scheduling required")
						{
							ScheduleASubscripion(PS,_ScheduledMessages,_ClearScheduledMessage);
						}

					// Subscription requiring processing of a delivered content
					if (PS->Status == "Processing required")
						{
							ProcessASubscribedPayload(Publisher_LN, PS,_ScheduledMessages,_ClearScheduledMessage);
						}

					// Deleting finished subscriptions
					if (PS->Status == "Delete")
						{
							PB->S << Offset1 <<  "(Deleting the subscription with index = "<<i<<")" << endl;

							PCore->DeleteSubscription(PS);
						}
				}
		}
	else
		{
			PB->S << Offset1 << "(Waiting for PSS discovery)"<<endl;
		}

	return Status;
}

int CoreRunEvaluate01::ScheduleASubscripion(Subscription *_PS, vector<Message*> &_ScheduledMessages, bool _ClearScheduledMessage)
{
	Message 				*Run=0;
	CommandLine				*PCL=0;
	Core					*PCore=0;
	string					Offset1 = "                              ";
	vector<string>			Limiters;
	vector<string>			Sources;
	vector<string>			Destinations;
	string 					Offset = " ";
	Message 				*SubData=0;

	PCore=(Core*)PB;

	//Set limiter, sources and destinations to it self
	Limiters.push_back(PB->PP->Intra_Process);
	Sources.push_back(PB->GetSelfCertifyingName());
	Destinations.push_back(PB->GetSelfCertifyingName());

	//Schedule and create header
	PB->PP->NewMessage(GetTime(),1,true,SubData);
	PMB->NewConnectionLessCommandLine("0.1",&Limiters,&Sources,&Destinations,SubData,PCL);
	SubData->NewCommandLine("-run","--subscribe","0.1",PCL);

	//Put arguments into message
	PCL->NewArgument(1);
	PCL->SetArgumentElement(0,0,PB->IntToString(_PS->Category));
	PCL->NewArgument(1);
	PCL->SetArgumentElement(1,0,_PS->Key);

	//Create SCN
	PB->GenerateSCNFromMessageBinaryPatterns(SubData,SCN);
	PMB->NewSCNCommandLine("0.1",SCN,SubData,PCL);

	//Push to input queue
	PCore->PGW->PushToInputQueue(SubData);

	PB->S << Offset1 << "(Going to subscribe the key = "<<_PS->Key<<")" << endl;

	_PS->Status = "Waiting delivery";

	PB->S << Offset1 << "(Changing subscription status from \"Scheduling required\" to \"Waiting delivery\")" << endl;

	// Storing a timestamp for performance evaluation
	_PS->Timestamp=GetTime();

	_ClearScheduledMessage=false;

#ifdef DEBUG

	PCore->Debug.OpenOutputFile();

	PCore->Debug << "Evaluating a scheduling required with key " << _PS->Key << ". Creating a ng -run --subscribe message. Status of the subscription is " <<_PS->Status<< endl;

	PCore->Debug.CloseFile();

#endif

	return OK;
}

int CoreRunEvaluate01::ProcessASubscribedPayload(string _Publisher_LN, Subscription *_PS, vector<Message*> &_ScheduledMessages, bool _ClearScheduledMessage)
{
	Core					*PCore=0;
	string					Offset1 = "                              ";
	string::size_type		Pos=0;
	string 					Extension;

	PCore=(Core*)PB;

	PB->S << Offset1 << "(Checking the file received from the peer with name " << _PS->FileName << ")" << endl;

	Pos=_PS->FileName.rfind('.');

	if(Pos != string::npos)
		{
			Extension = _PS->FileName.substr(Pos+1);
		}

	PB->S << Offset1 << "(File extension = " << Extension << ")" << endl;


	if (Extension == "txt")
		{
			ProcessTXTFile(_Publisher_LN, _PS->FileName);
		}

	if (Extension == "json")
		{
			ProcessJsonFile(_Publisher_LN, _PS->FileName, _PS->Key);
		}

	_PS->Status = "Delete";

	return OK;
}

int CoreRunEvaluate01::ProcessTXTFile(string _Publisher_LN, string _FileName)
{
	File 					F1;
	Core					*PCore=NULL;
	string					Offset1 = "                              ";

	PCore=(Core*)PB;

	F1.OpenInputFile(_FileName,PB->GetPath(),"DEFAULT");

	F1.seekg(0);

	// ------------------------------------------------------------------------------------------------------------------------------
	// Functions to customize the application. Put here the code to treat your TXT file as you wish
	// ------------------------------------------------------------------------------------------------------------------------------

	string Temp = _FileName.substr(0,16);

	if (Temp == "Service_Accepted")
		{
			if (_Publisher_LN == "PGCS")
				{
					PB->S << Offset1 << "(The peer accepted the contract!)"<< endl;

#ifdef DEBUG

					PCore->Debug.OpenOutputFile();

					PCore->Debug << "The peer accepted the contract!"<< endl;

					PCore->Debug.CloseFile();

#endif
				}
		}

	F1.CloseFile();

	return OK;
}

int CoreRunEvaluate01::ProcessJsonFile(string _Publisher_LN, string _FileName, string _HashPayload)
{
	File 					F1;
	Core					*PCore=NULL;
	string					Offset = "                    ";
	string					Offset1 = "                              ";
	vector<string>			Limiters;
	vector<string>			Sources;
	vector<string>			Destinations;
	Message 				*Revoke=0;
	CommandLine				*PCL=0;
	CommandLine				*PRVKCL=0;

	PCore=(Core*)PB;

	F1.OpenInputFile(_FileName,PB->GetPath(),"DEFAULT");

	F1.seekg(0);

	// ------------------------------------------------------------------------------------------------------------------------------
	// Functions to customize the application. Put here the code to treat your TXT file as you wish
	// ------------------------------------------------------------------------------------------------------------------------------

	F2.OpenOutputFile();

	string Temp;

	string Sample;

	F1 >> Temp;

	F1 >> Temp;

	F1 >> Sample;

	F2 << setprecision(10) << GetTime() <<" "<<Sample<<endl;

	F2.CloseFile();

	F1.CloseFile();
	
	// ------------------------------------------------------------------------------------------------------------------------------
	// Added in May 25th, 2016. Remove local file after receiving data
	// ------------------------------------------------------------------------------------------------------------------------------
	
	std::remove((PB->GetPath()+_FileName).c_str());

	// ------------------------------------------------------------------------------------------------------------------------------
	// Added in May 25th, 2016. Revoke HTS file after receiving data
	// ------------------------------------------------------------------------------------------------------------------------------

	PB->S << Offset <<  "(Generating a message to revoke the sample file key at domain level)"<<endl;

	// Setting up the OSID as the space limiter
	Limiters.push_back(PB->PP->Intra_Domain);

	// Setting up the this OS as the 1st source SCN
	Sources.push_back(PB->PP->GetHostSelfCertifyingName());

	// Setting up the this OS as the 2nd source SCN
	Sources.push_back(PB->PP->GetOperatingSystemSelfCertifyingName());

	// Setting up the this process as the 3rd source SCN
	Sources.push_back(PB->PP->GetSelfCertifyingName());

	// Setting up the PS block SCN as the 4th source SCN
	Sources.push_back(PB->GetSelfCertifyingName());

	// Setting up the destination
	Destinations.push_back(PCore->PSSTuples[0]->Values[0]);

	// Setting up the destination 2nd source
	Destinations.push_back(PCore->PSSTuples[0]->Values[1]);

	// Setting up the destination 3rd source
	Destinations.push_back(PCore->PSSTuples[0]->Values[2]);

	// Setting up the destination 4th source
	Destinations.push_back(PCore->PSSTuples[0]->Values[3]);

	// Creating a new message
	PB->PP->NewMessage(GetTime(),1,false,Revoke);

	// Creating the ng -cl -m command line
	PMB->NewConnectionLessCommandLine("0.1",&Limiters,&Sources,&Destinations,Revoke,PCL);

	// ***************************************************
	// Generate the ng -rvk --binding
	// ***************************************************

	Revoke->NewCommandLine("-rvk","--b","0.1",PRVKCL);

	// First argument
	PRVKCL->NewArgument(1);

	PRVKCL->SetArgumentElement(0,0,"18");

	// Second argument
	PRVKCL->NewArgument(1);

	PRVKCL->SetArgumentElement(1,0,_HashPayload);

	// ******************************************************
	// Generate the SCN
	// ******************************************************

	PB->GenerateSCNFromMessageBinaryPatterns(Revoke,SCN);

	// Creating the ng -scn --s command line
	PMB->NewSCNCommandLine("0.1",SCN,Revoke,PCL);

	PB->S << Offset <<"(The following message contains a revoke binding message to the PSS/GIRS/HTS)"<< endl;

	PB->S << "(" << endl << *Revoke << ")"<< endl;

	// ******************************************************
	// Finish
	// ******************************************************

	// Push the message to the GW input queue
	PCore->PGW->PushToInputQueue(Revoke);

	return OK;
}
