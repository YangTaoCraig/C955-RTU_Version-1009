/************************************************************
*															*
*	Project:    C830 RTU									*
*	Module:		CMM											*
*	File :	    CMM_Watchdog.h								*
*	Author:		Yu Wei										*
*															*
*	Copyright 2003 Singapore Technologies Electronics Ltd.	*
*************************************************************

**************************************************************

DESCRIPTION	

	This is header file for CMM_Watchdog.cpp.

*************************************************************
Modiffication History
---------------------

Version: CMM D.1.3.2
	01	13 November 2003, Yu Wei,
		Start to write.

Version: CMM D.1.3.5
	01	10 March 2004, Yu Wei
		Added START_WATCHDOG, STOP_WATCHDOG for watchdog operation.
		For reducing complexity.
		Refer to PR:OASYS-PMS 0044.

**************************************************************/
#ifndef  _CMM_WATCHDOG_H
#define  _CMM_WATCHDOG_H

//#include "Common.h"
//Watchdog operation type
#define START_WATCHDOG        1
#define STOP_WATCHDOG        0

//Object ID
#define SWC_OBJECT_ID        0
#define SWC_MUTLIDROP_OBJECT_ID    1
#define CLOCK_OBJECT_ID        2
#define SPM_OBJECT_ID        3
#define OTHER_OBJECT_ID        4

//Max watchdog times for one task.
//If a task watchdog timeout is more than this number,
//RTU will be reset.
#define MAX_WATCHDOG_TIMES      5  

void *WatchdogFunction(void *arg);
void WatchdogTaskSpawn(void);
void* WatchdogTask(void *arg);
void StopWatchDogFunction(void);


extern pthread_t g_tWatchDogTid;
extern VOID WDG_Initialization(VOID);
extern timer_t wdCreate(int timer_chid, timer_t timer_id, int event_code,
                        int event_value);
extern int wdStart(timer_t timer_id,  int FirstSignalSec, int FirstSignalMSec,
                   int intervalSec, int intervalMSec);
extern int wdCancel(timer_t timer_id);     
#endif /* _CMM_WATCHDOG_H */

