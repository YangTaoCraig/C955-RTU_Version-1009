/************************************************************
*                              *
*	Project:    C830 RTU	                *
*	Module:  	CMM	                    *
*	File :      CMM_Watchdog.cpp	            *
*	Author:  	Yu Wei	                  *
*                              *
*	Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************

**************************************************************

DESCRIPTION

	This file define watchdog function.

*************************************************************
Modiffication History
---------------------

Version: CMM D.1.3.2
	01	13 November 2003, Yu Wei,
  	Start to write.

Version: CMM D.1.3.6
	01	25 May 2004, Yu Wei,
  	Modified WatchdogTask(). The log file date switch over
  	will be done by log task.
  	Refer to PR:OASYS-PMS 0212.
**************************************************************/
#include <sys/neutrino.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
//#include "flg_def.h"
//#include "flg_ext.h"

#include "Common.h"
#include "CMM_Watchdog.h"
#include "CMM_Log.h"
#include "CMM_Timer.h"
#include "RMM.h"
#include "CMM_Timer.h"
#include "CMM_Listen.h"
#include "SWC.h"
#include "ServerPoll.h"
#include "Init.h"
#include "RMM_RTULink.h"
#include "Common_qnxWatchDog.h"

extern char *g_acSWCDeviceName[20];
//Watchdog timeout flag. When a task's watchdog timeout, it will be set to 1.
int g_cWatchdogTaskFlag[WATCHDOG_TASK_MAX_NUMBER];

//Watchdog timeout counter.
int g_cWatchdogTaskCount[WATCHDOG_TASK_MAX_NUMBER];

//Watchdog task ID
pthread_t g_tWatchDogTid;
bool g_bWDTaskWorking;
int WatchDogChid;
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  WDG_Initialization

 DESCRIPTION

  This routine will initialize watchdog resources

 CALLED BY

  SYS_Initialization

 CALLS

  ChannelCreate

 PARAMETER

  n/a

 RETURN

  n/a

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE           REMARKS

 Bryan Chong      26-Apr-2012    Initial revision

----------------------------------------------------------------------------*/
VOID WDG_Initialization(VOID)
{
  WatchDogChid = ChannelCreate(0);
} // WDG_Initialization
/*******************************************************************************
Purpose:
  Process task watchdog timeout. When a task except INM is watchdog timeout,
  this routine will be called.

Input
  nTaskIDIndex  --  Task ID index. (in)
*******************************************************************************/
void *WatchdogFunction(void *arg)
{
  int rcvid;
  struct _pulse msg;
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_WD)
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_WD)
  g_tWatchDogTid = pthread_self();

  while (1)
  {
    rcvid = MsgReceive(WatchDogChid, &msg, sizeof(msg), NULL);

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_WD)
    printf("[WDG] WatchdogFunction, %s pulse triggered\n"
           "  type %d, subtype %d, code %d, value %d, scoid %d\n",
           SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
           msg.type, msg.subtype, msg.code, msg.value, msg.scoid);
    #endif // CFG_DEBUG_MSG

    if (rcvid == 0) /* we got a pulse */
    {
      //set to one means this task  missed a deadline
      g_cWatchdogTaskFlag[msg.value.sival_int] = 1;
      switch(msg.code)
      {
        case  RMM_MAIN_TIMER_PULSE_CODE:
          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_WD)
          printf("[WDG] WatchdogFunction, RMM_MAIN_TIMER_PULSE_CODE\n");
          #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_WD)
          g_pEventLog->LogMessage((CHAR *)
            "WARN [WDG] WatchdogFunction, RMM Task WatchDog Timeout.\n");
          delay(1000);
          //20151117 YT reboot system
          system("slay rtu_rel");
          break;
        case  RMM_LINK_TIMER_PULSE_CODE:
          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_WD)
          printf("[WDG] WatchdogFunction, RMM_LINK_TIMER_PULSE_CODE\n");
          #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_WD)
          g_pEventLog->LogMessage((CHAR *)
            "WARN [WDG] WatchdogFunction, RMM LINK Task WatchDog Timeout.\n");
          break;

        case RMM_CONN1_TIMER_PULSE_CODE:
          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_WD)
          printf("[WDG] WatchdogFunction, RMM_CONN1_TIMER_PULSE_CODE\n");
          #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_WD)
          g_pEventLog->LogMessage((CHAR *)
            "WARN [WDG] WatchdogFunction, Conn1 Task WatchDog Timeout.\n");
          break;

        case RMM_CONN2_TIMER_PULSE_CODE:
          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_WD)
          printf("[WDG] WatchdogFunction, RMM_CONN2_TIMER_PULSE_CODE\n");
          #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_WD)
          g_pEventLog->LogMessage((CHAR *)
            "WARN [WDG] WatchdogFunction, Conn2 Task WatchDog Timeout.\n");
          break;
        case SERV_POLL_COPY_TIMER_PULSE_CODE:
          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_WD)
          printf("[WDG] WatchdogFunction, SERV_POLL_COPY_TIMER_PULSE_CODE\n");
          #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_WD)
          g_pEventLog->LogMessage((CHAR *)
            "WARN [WDG] WatchdogFunction, ServerCopy Task WatchDog Timeout.\n");
          break;
        case SWC_MAIN_TIMER_PULSE_CODE:
          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_WD)
          printf("[WDG] WatchdogFunction, SWC_MAIN_TIMER_PULSE_CODE\n");
          #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_WD)
          g_pEventLog->LogMessage((CHAR *)
            "WARN [WDG] WatchdogFunction, SWC Task watchdog timeout.\n");
//          pFLG_CtrlBlk->SendMessage(E_FLG_MSG_WDG_Timeout,
//            FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec,
//            FLG_LOG_T_Event,
//            "[WDG] WatchdogFunction, rx msg SWC %d, %s timeout\n",
//            msg.value.sival_int,
//            g_apSWCObject[msg.value.sival_int]->m_acSWCName);
          //20151117 YT reboot system
           delay(1000);
           system("slay rtu_rel");
          break;

      } // switch(msg.code)
    } // if (rcvid == 0)
  } // while(1)
  return NULL;
}// WatchdogFunction

void StopWatchDogFunction()
{
  pthread_detach(g_tWatchDogTid);
  pthread_cancel(g_tWatchDogTid);
  ChannelDestroy(WatchDogChid);
}

/*******************************************************************************
Purpose
  Create a watch dog.

Input
  timer_chid    [in] channel ID
  timer_id      [out] timer ID
  event_code    [in] event code
  event_value   [in] event value

Return
  None

History
    Name         Date          Remark
    ----         ----          ------
  Yu, Wei     22-Mar-2004   Initial revision

*******************************************************************************/
timer_t wdCreate(  int timer_chid, //channel ID
          timer_t   timer_id,
          int event_code,
          int event_value)
{
  struct sigevent myevent;

  myevent.sigev_notify = SIGEV_PULSE;
  myevent.sigev_coid = ConnectAttach(0, 0, timer_chid, _NTO_SIDE_CHANNEL, 0);
  myevent.sigev_priority = getprio(0);
  myevent.sigev_code = _PULSE_CODE_MINAVAIL + event_code;
  myevent.sigev_value.sival_int  = event_value ;
  if(timer_create(CLOCK_REALTIME, &myevent, &timer_id)< 0)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [CMM] wdCreate, fail to create thread\n");
    #endif // CFG_PRN_ERR

    return TYP_ERROR;
  }
  return timer_id;
}
/*
 * start a watchdog
*/
int wdStart(timer_t timer_id,
      int FirstSignalSec ,
      int FirstSignalMSec ,
      int intervalSec,
      int intervalMSec )
{
   struct itimerspec     itime;

   itime.it_value.tv_sec = 0;
   itime.it_value.tv_nsec = 0;

   itime.it_interval.tv_sec = 0;
   itime.it_interval.tv_nsec = 0;
   if( timer_settime(timer_id, 0, &itime, NULL) < 0)
   {
      return TYP_ERROR;
   }

   itime.it_value.tv_sec = (uint32_t)FirstSignalSec;
   itime.it_value.tv_nsec = (uint64_t)(FirstSignalMSec)*1000000;

   itime.it_interval.tv_sec = (uint32_t)intervalSec;
   itime.it_interval.tv_nsec = (uint64_t)(intervalMSec)*1000000;

   if( timer_settime(timer_id, 0, &itime, NULL) < 0)
   {
      return TYP_ERROR;
   }

   return 0;
}

/*
 * Cancel a watchdog
*/
int wdCancel(timer_t   timer_id)
{
   struct itimerspec     itime;

   itime.it_value.tv_sec = 0;
   itime.it_value.tv_nsec = 0;

   if( timer_settime(timer_id, 0, &itime, NULL) < 0)
   {
      return -1;
   }
   return 0;
}

