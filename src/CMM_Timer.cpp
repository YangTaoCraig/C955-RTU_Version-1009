/************************************************************
*                              *
*  Project:    C830 RTU                  *
*  Module:    CMM                      *
*  File :      CMM_Timer.cpp                *
*  Author:    Yu Wei                    *
*                              *
*  Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************

**************************************************************

DESCRIPTION

  This file define timer function.

*************************************************************
Modiffication History
---------------------

Version: CMM D.1.3.6
  01  20 May 2004, Yu Wei,
    Added this file for timer. The timer cannot use system
    clock. It should use relative timer. When the time is
    synchronized to Clock, the timer will jump.
    Refer to PR:OASYS-PMS 0201.

**************************************************************/
#include <sys/neutrino.h>
#include <pthread.h>
#include <iostream.h>
#include <termios.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"

#include "CMM_Timer.h"
#include "CMM_Log.h"
#include "CMM_Watchdog.h"

struct tTimerValue g_tTimer;  //Store time escape since timer start.

pthread_t  g_nTimerTaskID;        //Timer task ID.
bool g_bTimerTaskWorking;    //Timer task working flag.

pthread_mutex_t Timer_mutex = PTHREAD_MUTEX_INITIALIZER;

/**********************************************************************************
Purpose:
  This is timer task.
  Every time period of SYS_CLOCK_MSECOND escape, the counter will increase.

***********************************************************************************/

#define MYTIMER_PULSE_CODE   _PULSE_CODE_MINAVAIL+15

void *Timer(void *arg)
{

   timer_t                 sys_timer_id = 0;
   int                     timerChid;
   int                     rcvid;
   struct _pulse msg;


  timerChid = ChannelCreate(0);
  sys_timer_id = wdCreate(timerChid , sys_timer_id ,15 , 0);
  //change 1 msecond to 10 msencond 20080220
  wdStart(sys_timer_id , 1 , 0, 0, 10);
  g_nTimerTaskID = pthread_self();

  while(1)
  {
    if(g_bTimerTaskWorking == false)
    {
      timer_delete(sys_timer_id);
      ChannelDestroy(timerChid);
      pthread_detach(g_nTimerTaskID);

      return NULL;
    }
    rcvid = MsgReceive(timerChid, &msg, sizeof(msg), NULL);
    if (rcvid == 0)
    { /* we got a pulse */
      if (msg.code == MYTIMER_PULSE_CODE)
      {
        pthread_mutex_lock(&Timer_mutex);
        g_tTimer.Low += 10;
        if(g_tTimer.Low >= TIMER_LOW_MSEC)
        {
          g_tTimer.Low = 0;
          g_tTimer.High++;
        }
        pthread_mutex_unlock(&Timer_mutex);
      }

    }
  }
}

/**********************************************************************************
Purpose:
  Set flag to stop timer task.

***********************************************************************************/
void StopTimer(void)
{
  g_bTimerTaskWorking = false;
}


/**********************************************************************************
Purpose:
  Get current timer value.

Output
  tValue  --  current timer value.
***********************************************************************************/
void GetTimerValue(tTimerValue *tValue)
{
  pthread_mutex_lock(&Timer_mutex);
  tValue->Low = g_tTimer.Low;
  tValue->High = g_tTimer.High;
  pthread_mutex_unlock(&Timer_mutex);
}

/**********************************************************************************
Purpose:
  This routine initialize timer paramets and spawn Timer task.

***********************************************************************************/
E_ERR_T Init_Timer()
{
  g_tTimer.Low = 0;    //Reset timer.
  g_tTimer.High = 0;
  g_bTimerTaskWorking = true;    //Set timer task working.
  return E_ERR_Success;
}


