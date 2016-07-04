/************************************************************
*                              *
*  Project:    C830 RTU                  *
*  Module:    CMM                      *
*  File :      CMM_Timer.h                  *
*  Author:    Yu Wei                    *
*                              *
*  Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************

**************************************************************

DESCRIPTION

  This is header file for CMM_Timer.cpp.

*************************************************************
Modiffication History
---------------------

Version: CMM D.1.3.6
  01  20 May 2004, Yu Wei,
    Added this file for timer. The timer cannot use system
    clock. When the time is synchronized to Clock, the timer
    will jump.
    Refer to PR:OASYS-PMS 0201.

**************************************************************/
#ifndef  _CMM_TIMER_H
#define  _CMM_TIMER_H



#define TIMER_LOW_MSEC  10000000  //10000 seconds, 10000000 milli-seconds.

struct tTimerValue
{
  unsigned int Low;        //Timer low part. in ms.
  unsigned int High;        //Timer high part. 1 = 10000000 ms.
};

void *Timer(void *arg);
void StopTimer(void);
void GetTimerValue(tTimerValue *);
E_ERR_T Init_Timer();



#endif /* _CMM_TIMER_H */


