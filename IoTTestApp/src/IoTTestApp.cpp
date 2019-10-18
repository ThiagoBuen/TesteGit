/*
	NovaGenesis

	Name:		IoT Test Application
	Object:		IoTTestApp
	File:		IoTTestApp.cpp
	Author:		Antonio M. Alberti
	Date:		01/2016
	Version:	0.1
*/

#ifndef _IOTTESTAPP_H
#include "IoTTestApp.h"
#endif

#ifndef _CORE_H
#include "Core.h"
#endif

IoTTestApp::IoTTestApp(string _LN, string _ULN, key_t _Key, string _Path):Process(_LN,_Key,_Path)
{
	Block	*PCoreB=0;
	Core	*PCore=0;
	string 	CoreLN="Core";

	NewBlock(CoreLN,_ULN,PCoreB);

	PCore=(Core*)PCoreB;

	// Run the base class GW
	RunGateway();
}

IoTTestApp::~IoTTestApp()
{
}

// Allocate a new block based on a name and add a Block on Blocks container
int IoTTestApp::NewBlock(string _LN, string _ULN, Block *&_PB)
{
	if (_LN == "Core" )
		{
			Block 	*PGWB=0;
			Block 	*PHTB=0;
			GW		*PGW=0;
			HT		*PHT=0;
			string 	GWLN="GW";
			string 	HTLN="HT";

			GetBlock(GWLN,PGWB);

			PGW=(GW*)PGWB;

			GetBlock(HTLN,PHTB);

			PHT=(HT*)PHTB;

			unsigned int Index=0;

			Index=GetBlocksSize();

			Core *PCore=new Core(_LN,_ULN, this,Index,PGW,PHT,GetPath());

			_PB=(Block*)PCore;

			InsertBlock(_PB);

			return OK;
		}

	return ERROR;
}



