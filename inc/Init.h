/************************************************************
*															*
*	Project:    C830 RTU									*
*	Module:		Initialization (INM)						*
*	File :	    Init.h										*
*	Author:		Yu Wei										*
*															*
*	Copyright 2003 Singapore Technologies Electronics Ltd.	*
*************************************************************

**************************************************************

DESCRIPTION	

	This is header file for Init.cpp routine defination.  

*************************************************************
Modiffication History
---------------------

Version: INM D.1.1.1
	01	3 April 2003, Yu Wei,	
		Start to write.

Version: INM D.1.3.4
	01	02 March 2004, Yu Wei
		Added LogGlobalValues() for module test.
		Refer to PR:OASYS-PMS 0029.

Version: INM D.1.3.5
	01	09 March 2004, Yu Wei
		Reduce complexity in Init().
		Added HardwareInit(), LANInit() and M45Init().
		Refer to PR:OASYS-PMS 0044.
**************************************************************/
#ifndef  _INIT_H
#define  _INIT_H


#include "Common.h"
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>


#include <sys/uio.h>
#include <unistd.h>
#include <net/route.h>
#include <libgen.h>
#include <arpa/inet.h>
#include <process.h>
#include <errno.h>


void Init(void);
int HardwareInit(void);
//void LANInit(void);
E_ERR_T LANInit(void);
int M45Init(void);

void stopRTU(void);
void ResetRTU(void);

void LogInitError(char *);

#ifdef ModuleTest
void LogGlobalValues(void);

#endif


#endif /* _INIT_H */


