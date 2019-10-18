//============================================================================
// Name        : Internet of Things Test Application
// Author      : Antonio Alberti
// Version     : v0.1
// Copyright   : Alberti, INATEL 2016
// Description : Client application of IoT raw data
//============================================================================

#include "IoTTestApp.h"

#ifndef _TIME_H
#include <time.h>
#endif

#ifndef _SYS_TIME_H
#include <sys/time.h>
#endif

int main(int argc, char *argv[])
{
	int 			R=0;
	bool		 	Problem=false;

	if (argc != 0)
		{
			if (argc == 3)
				{

					string _ULN = argv[1]; //Unique legible name

					string Path=argv[2];

					cout << "(The unique legible name is "<<_ULN<< ")"<<endl;

					cout << "(The I/O path is "<<Path<< ")"<<endl;

					long long Temp=(long long)&R;

					// Initialize the random generator
					srand ((unsigned int)Temp*time(NULL));

					// Generates a random key
					R=1+(rand()%2147483647);

					// Set the IPC key
					key_t Key=R;

					// Create a process instance
					IoTTestApp execIoTTestApp("IoTTestApp",_ULN,Key,Path);
				}
			else
				{
					Problem=true;

					cout<< "(ERROR: Wrong number of main() arguments)"<<endl;
				}
		}
	else
		{
			Problem=true;

			cout<< "(ERROR: No argument supplied)"<<endl;
		}

	if(Problem == true)
		{
			cout<< "(Usage: ./IoTTestApp _ULN Path)"<< endl;
			cout<< "(_ULN example: APP01)"<<endl;
			cout<< "(Path example: /home/myprofile/workspace/novagenesis/IO/IoTTestApp/)"<<endl;
		}

	return 0;
}




