/*
	NovaGenesis
	
	Name:		IoT Test Application
	Object:		IoTTestApp
	File:		IoTTestApp.h
	Author:		Antonio M. Alberti
	Date:		01/2016
	Version:	0.1
*/

#ifndef _IOTTESTAPP_H
#define _IOTTESTAPP_H

#include "Process.h"

#define ERROR 1
#define OK 0

class IoTTestApp: public Process
{
	public:
		
		// Constructor
		IoTTestApp(string _LN, string _ULN, key_t _Key, string _Path);

		// Destructor
		~IoTTestApp();

		// Allocate a new block based on a name and add a Block on Blocks container
		int NewBlock(string _LN, string _ULN, Block *&_PB);
};

#endif






