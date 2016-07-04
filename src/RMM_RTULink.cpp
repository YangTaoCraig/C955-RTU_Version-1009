/*----------------------------------------------------------------------------

            Copyright (c) 2011 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME

  RMM_RTULink.cpp

 COMPONENT

  Redundancy Management Module (RMM)

 DESCRIPTION

  This file is for interfacing to other RTU. The local RTU can be a server
  via LAN2 and LAN1 or a client via LAN2 and LAN1. The LAN2 has higher
  priority.

 AUTHOR

  Yu Wei


 HISTORY

    NAME            DATE                    REMARKS

  Yu Wei        19-Jun-2003     RMM D.1.1.2. Created initial version
                                  [Refer to modification history below]
  Bryan Chong   03-Aug-2009     Update comments
  Bryan Chong   31-May-2011     Combined methods, LinkSlave1 and LinkSlave2, to
                                LinkSlave [C955 PR 108]

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
Modiffication History
---------------------

Version: RMM D.1.1.2
  01  19 June 2003, Yu Wei,
    Start to write.

Version: RMM D.1.2.0
  01  03 July 2003, Yu Wei,
    Start to add Redundancy function.

Version: RMM D.1.2.1
  01  14 July 2003, Yu Wei
    D.1.2.0 send to Ripple. Continue to add RMM.
    Add RTU notation (weight) calculation.

  02  15 July 2003, Yu Wei
    Change file name to RMM_RTULink.cpp from RMM_OtherRTU.cpp.
    Add class CRTULink to implement interface between two RTU
    and RMM_RTULink.h to define class member and RMM_RTULink.cpp
    routines.

  03  21 July 2003, Yu Wei,
    Add SWC link down in primary RTU, standby RTU will check
    the link status. Based on checking result, the primary RTU
    decide whether the state change.

Version: RMM D.1.3.0
  01  01 August 2003, Yu Wei,
    All SWC use same class except Clock.

  02  07 August 2003, Yu Wei,
    Added primary RTU sending polling table to standby RTU.

Version: RMM D.1.3.1
  01  17 September 2003, Yu Wei,
    Change bRMMLinkTaskWorking to g_bRMMLinkTaskWorking.

Version: RMM D.1.3.2
  01  29 October 2003, Yu Wei,
    Changed nConnrctionResult to nConnectionResult.

  02  03 November 2003, Yu Wei,
    Modify LinkSlave(). Added nCmdImplementResult. When received
    write command (0x10) to change state, the command code is not
    "0x01", will reply exception 4.
    Refer to "Technical Review Summary - RTU Software Design Specification"
    ANNEX A: (21).

  03  06 November 2003, Yu Wei,
    Modify LinkMaster() and LinkSlave(), if link 1(LAN2) is OK,
    the link 2(LAN1) g_tRTUStatus.abOtherLinkStatus[1] will be set to OK.
    Refer to PR:OASYS-PMS 0017.

  04  07 November 2003, Yu Wei,
    Replace CONNECT_VXWORKS_BLOCK_TIME by SOCKET_CONNECT_TIMEOUT.

    Added m_bChangeToStandbyFlag to indicate that RTU changes to standby and
    sends status table to another RTU.

  05  10 November 2003, Yu Wei,
    Deleted task "OtherRTULinkTask", it is done by RMM main task "RMMTask".
    This change can synchronize two tasks to solve the problems that
    RTU stop poll SWC and remain in switching state.
    Refer to PR:OASYS-PMS 0011, PR:OASYS-PMS 0012, PR:OASYS-PMS 0015 and
    PR:OASYS-PMS 0016.

  06  14 November 2003, Yu Wei,
    The task "OtherRTULinkTask" is added. See above mofification 05 and RMM.cpp
    modification version D.1.3.2 04.

  07  18 November 2003, Yu Wei
    Added watchdog function.
    Added g_tRMMOtherLinkWatchdogID, RMM_OTHER_LINK_WATCHDOG_TIMEOUT,
    g_tRMMConnect1WatchdogID, RMM_CONNECT1_WATCHDOG_TIMEOUT,
    g_tRMMConnect2WatchdogID, RMM_CONNECT2_WATCHDOG_TIMEOUT,

  08  27 November 2003, Yu Wei
    Added m_anLinkDownTimeout[2] timeout value for declaring link down.

Version: RMM D.1.3.3
  01  09 December 2003, Yu Wei,
    Fixed bug. If SWC link down, primary RTU ask standby RTU to check link,
    the message cannot send to standby RTU, RTU will stop poll SWC.
    Modified SendCommandToOtherRTU() and GetResponseFromOtherRTU().

  02  30 December 2003, Yu Wei,
    m_anFailureTimeoutStandby and m_anFailureTimeout are not used.

    Bug fixed. Primary RTU don't do LinkSlave(). It will cause that RTU cannot
    change form LAN2 to LAN1 when LAN2 is down.

  03  26 January 2004, Yu Wei,
    Delete CRTULink() parameter, using global variable.

Version: RMM D.1.3.4
  01  24 February 2004, Yu Wei
    Bug Fixed. When standby RTU is turned off, RTU link1 and link2 down at same time,
    the link status g_tRTUStatus.abOtherLinkStatus[0] will not change to false.
    Modify LinkMaster(). When link2 is down, link1 will also be set to down.
    Refer to PR:OASYS-PMS 0033.

  02  25 February 2004, Yu Wei
    Fixed bug in GetResponseFromOtherRTU().
    Setting g_pRTUStatusTable->m_bSWCLinkDownFlag = false should be under
    case POLLING_WRITE_TABLE rather than POLLING_READ_TABLE.
    Refer to PR:OASYS-PMS 0034.

  03  02 March 2004, Yu Wei
    Added LogLinkValues() for module test.
    Refer to PR:OASYS-PMS 0035.

Version: RMM D.1.3.5
  01  08 March 2004, Yu Wei
    Modified GetResponseFromOtherRTU(),
    The ModbusReplyCheck() will  check slave address and function code.
    Refer to PR:OASYS-PMS 0053.
    The 'else' is not used. Deleted.
    Refer to PR:OASYS-PMS 0044.


  02  10 March 2004, Yu Wei
    Reduce complexity in GetResponseFromOtherRTU() .
    Added ExceptionProcess() and NormalProcess().
    Refer to PR:OASYS-PMS 0044.

  03  10 March 2004, Yu Wei
    Added default case for switch in SendCommandToOtherRTU()and WriteSWCTable().
    Refer to PR:OASYS-PMS 0042.

  04  11 March 2004, Yu Wei
    Fixed bug that a SWC link down, the RTU will switch to standby.
    The primary didn't wait for standby check link completed.
    RMM added m_bWaitLinkCheckCompletedFlag.
    Modified NormalProcess(),
    Refer to PR:OASYS-PMS 0066.

  05  22 March 2004, Yu Wei
    STATEFLAG_SWITCHING is changed, modified OtherRTULinkTask().
    Refer to PR:OASYS-PMS 0143.

  06  23 March 2004, Yu Wei
    Refer to PR:OASYS-PMS 0143.
    Modified WriteSWCTable(), CRTULink(), LinkSlave().
    Mofidied protocol for primary write command to standby.
    Start address = WRITE_RTU_STATUS_ADDRESS, write RTU status table.
    Start address = WRITE_COMMAND_ADDRESS, write cahnge state command.
    Others, write SWC table.
    The addres high byte = SWC ID,
    low byte = table ID (fast, slow, timer).
    Added GetNextSWCTableID(), modified WriteSWCTable(), NormalProcess().

    Added ClearVariables() to clear all variables when new object.
    Refer to PR:OASYS-PMS 0054.

  07  25 March 2004, Yu Wei
    Deleted m_bChangeToStandbyFlag. When standby receive
    "Change to primary" command will change to Switching_StoP or
    Primary, the LinkSlave will not work in these two state.
    (Improvement)

  08  26 March 2004, Yu Wei
    When standby RTU receive change to primary command, change
    STATEFLAG_SWITCHING_STOP.
    Refer to PR:OASYS-PMS 0143.

  09  08 April 2004, Yu Wei
    Added m_anPollingIntervalType[2] to fixed bug that the polling
    time between RTU is too slow.
    Modified CRTULink(), SendCommandToOtherRTU() and WriteSWCTable().
    Refer to PR:OASYS-PMS 0155.

Version: RMM D.1.3.6
  01  19 April 2004, Yu Wei
    Modified NormalProcess(). When other received change stste command,
    set m_bOtherRTUSwitchFlag.
    Modified LinkSlave(). When link estiablish or link down, clear the
    flag m_bOtherRTUSwitchFlag.
    Refer to PR:OASYS-PMS 0163.

  02  20 April 2004, Yu Wei
    Modified LinkSlave() to set g_pRTUStatusTable->m_nSwitchFlag.
    Added WriteCommandProcess() to reduce LinkSlave()'s complexity.
    Refer to PR:OASYS-PMS 0163.

  03  26 April 2004, Yu Wei
    To get LAN1 link status, primary will send data to standby in a
    fix time interval.
    Modified ConnectLink0() and ConnectLink1().
    Modified CRTULink(), LinkMaster(), GetResponseFromOtherRTU(),
    SendCommandToOtherRTU(), WriteSWCTable(), GetNextSWCTableID(),
    NormalProcess(), ExceptionProcess(), LinkSlave().

    Added Main() and SetStateFlag() and modified OtherRTULinkTask().
    When system state has changed, this task must complete a circle
    before changing state. When state is changed, the connection socket
    shouled be closed and timer should be reset.
    Refer to PR:OASYS-PMS 0167.

  04  29 April 2004, Yu Wei
    Modified CRTULink() to clear variables.
    Refer to PR:OASYS-PMS 0182.

  05  20 May 2004, Yu Wei
    Modified OtherRTULinkInit()and CRTULink().
    To reduce RTU link down. The listen socket ID will different to
    connection socket ID.
    The RU1 listen socket ID = base ID+1,
    connection socket ID = base ID.
    The RTU2 listen socket ID = base ID,
    connection socket ID = base ID+1.
    Refer to PR:OASYS-PMS 0163.

  06  20 May 2004, Yu Wei
    Modified ActivateTimeOut() and CheckTimeOut() to use relative timer
    and avoid the timer jump.
    Refer to PR:OASYS-PMS 0201.

  07  24 May 2004, Yu Wei
    Modified WriteCommandProcess(). When server send command
    change primary RTU to standby and primary transfered polling table,
    primary RTU will send confirm switch command to standby.
    Standby RTU received this command will set other RTU status to
    standby.
    Refer to PR:OASYS-PMS 0210.

  08  11 June 2004, Yu Wei
    Modified Main(). Hardware test and system reset states
    will stop exchange data between RTUs.
    Modified SetStateFlag(). Hardware test and system reset
    states will set link between RTUs to down.
    Refer to PR:OASYS-PMS 0180.

Version: RMM E.1.0.0
  01  08 July 2004, Yu Wei
    Modified LinkMaster(). When there is no command sent to standby RTU
    The task will delay 5ms, so that other task can work.
    Refer to PR:OASYS-PMS 0222.

Version: RMM E.1.0.2
  01  07 April 2005, Tong Zhi Xiong
    Modified WriteSWCTable(). Check g_acSWCObject before calling
    Refer to PR:OASYS-PMS 0236.
----------------------------------------------------------------------------*/
#include <sys/neutrino.h>
#include <iostream.h>
#include <termios.h>
#include <time.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "err_ext.h"
#include "sys_ass.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
#include "ser_def.h"
#include "ser_ext.h"
#include "lnc_def.h"
#include "lnc_ext.h"
#include "rmm_def.h"
#include "str_def.h"
#include "str_ext.h"
//#include "flg_def.h"
//#include "flg_ext.h"

#include "Common.h"
#include "CMM_Log.h"
#include "RMM.h"
#include "RMM_RTULink.h"
#include "CMM_Listen.h"
#include "CMM_Modbus.h"
//#include "CMM_Comm.h"
#include "SWC.h"
#include "CMM_Watchdog.h"
#include "Common_qnxWatchDog.h"

//When connect failure, the vxworks will block about 10 sconds
//Standby RTU reports link failure should wait for more than this time.
//#define CONNECT_VXWORKS_BLOCK_TIME  10000


pthread_t g_nOtherRTULinkTaskID;      //Main task ID of link between RTUs.
pthread_t g_anOtherRTUConnectionTaskID[2];  //Tasks ID for connection to other
                                            // RTU as a client.
pthread_t g_anOtherRTULinkSocketTaskID[2];  //Tasks ID for listening to other
                                            // RTU as a server.
CListen *g_apOtherRTUServerSocket[2];  //Object for server socket.
CRTULink *g_pCRTULink;                //Object for RTUs link.

bool g_bRMMLinkTaskWorking;        // The flag for OtherRTULinkTask working/
                                  // stopping.

int  g_tRMMOtherLinkWatchdogID;  //Watchdog ID for other link task.
int  g_tRMMConnect1WatchdogID;  //Watchdog ID for other link1 connection task.
int  g_tRMMConnect2WatchdogID;  //Watchdog ID for other link2 connection task.

timer_t RMM_link_timer_id;
timer_t RMM_conn1_timer_id;
timer_t RMM_conn2_timer_id;

pthread_mutex_t Recv_mutex = PTHREAD_MUTEX_INITIALIZER;
char copy_RecvOtherRTUBuffer[264];// For copy the the Recvother RTU Buffer
/**********************************************************************************
Purpose:
  Create interface link between RTUs objects and spawn tasks.

Return:
  OK    -- Create object and spawn tasks OK.
  ERROR  -- Create one object or spawn one task ERROR.
***********************************************************************************/
int OtherRTULinkInit(void)
{
  //pthread_attr_t attr;
  int nReturn = OK;
  E_ERR_T rstatus = E_ERR_Success;

  g_pCRTULink = new CRTULink();

  if(g_pCRTULink == TYP_NULL)
  {
    g_pEventLog->LogMessage((CHAR *)"RMM create link object error.\n");
    nReturn =  ERROR;
  }
  else
  {
///////////////////////////////////////////////////////////////////////

  //  //RTU1 listen socket port use base port ID +1, RTU2 use base port ID. To reduce
//    //RTU link down.  //040520 Yu Wei
    // server listen task  should start before client connection task ??
    if( g_tRTUConfig.nRTUID == 1)
    {
      g_apOtherRTUServerSocket[0] = new CListen(
        g_tRTUConfig.tLANPara[1].acLAN_IP,
        g_tRTUConfig.tLANPara[1].anOtherRTULinkSocketID+1,
        g_tRTUConfig.tLANPara[1].nPollingTimeout *
        g_tRTUConfig.tLANPara[1].nPollingRetryNumber +
        g_tRTUConfig.tLANPara[1].nPollingTime);  //Use config file value to set.  //040512 Yu Wei
        //10000);//g_tRTUConfig.nServerCMDTimeout);

      g_apOtherRTUServerSocket[1] = new CListen(
        g_tRTUConfig.tLANPara[0].acLAN_IP,
        g_tRTUConfig.tLANPara[0].anOtherRTULinkSocketID+1,
        g_tRTUConfig.tLANPara[0].nPollingTimeout *
        g_tRTUConfig.tLANPara[0].nPollingRetryNumber +
        g_tRTUConfig.tLANPara[0].nPollingTime);
        //g_tRTUConfig.tLANPara[0].nPollingTime * 2 +
        //SOCKET_CONNECT_TIMEOUT);  //Use config file value to set.  //040512 Yu Wei
        //10000); //g_tRTUConfig.nServerCMDTimeout);
    }
    else
    {
      g_apOtherRTUServerSocket[0] = new CListen(
        g_tRTUConfig.tLANPara[1].acLAN_IP,
        g_tRTUConfig.tLANPara[1].anOtherRTULinkSocketID,
        g_tRTUConfig.tLANPara[1].nPollingTimeout *
        g_tRTUConfig.tLANPara[1].nPollingRetryNumber +
        g_tRTUConfig.tLANPara[1].nPollingTime);  //Use config file value to set.  //040512 Yu Wei
        //10000);//g_tRTUConfig.nServerCMDTimeout);

      g_apOtherRTUServerSocket[1] = new CListen(
        g_tRTUConfig.tLANPara[0].acLAN_IP,
        g_tRTUConfig.tLANPara[0].anOtherRTULinkSocketID,
        g_tRTUConfig.tLANPara[0].nPollingTimeout *
        g_tRTUConfig.tLANPara[0].nPollingRetryNumber +
        g_tRTUConfig.tLANPara[0].nPollingTime);
        //g_tRTUConfig.tLANPara[0].nPollingTime * 2 +
        //SOCKET_CONNECT_TIMEOUT);  //Use config file value to set.  //040512 Yu Wei
        //10000); //g_tRTUConfig.nServerCMDTimeout);
    }    //040520 Yu Wei

    if((g_apOtherRTUServerSocket[0] == TYP_NULL) ||
       (g_apOtherRTUServerSocket[1] == TYP_NULL))
    {
      g_pEventLog->LogMessage((CHAR *)
        "RMM create server socket object error.\n");
      nReturn = ERROR;
    }
    else
    {
      rstatus = SYS_ThrdInit(E_SYS_TLB_RMM_OthRTULinkSock1);
      if(rstatus != E_ERR_Success)
      {
         #ifdef CFG_PRN_ERR
         printf("ERR  [RMM] OtherRTULinkInit, create Other RTU Link Socket1 "
                "thread fail\n");
         #endif // CFG_PRN_ERR
         g_pEventLog->LogMessage((CHAR *)
           "RMM spwan other RTU Link1 server socket task error.\n");
         return ERROR;
      }
      g_anOtherRTULinkSocketTaskID[0] =
        pSYS_CB->thrd_ctrl[E_SYS_TLB_RMM_OthRTULinkSock1].thrd_id;

      rstatus = SYS_ThrdInit(E_SYS_TLB_RMM_OthRTULinkSock2);
      if(rstatus != E_ERR_Success)
      {
         #ifdef CFG_PRN_ERR
         printf("ERR  [RMM] OtherRTULinkInit, create Other RTU Link Socket2 "
                "thread fail\n");
         #endif // CFG_PRN_ERR
         g_pEventLog->LogMessage((CHAR *)
           "RMM spwan other RTU Link2 server socket task error.\n");
         return ERROR;
      }
      g_anOtherRTULinkSocketTaskID[1] =
        pSYS_CB->thrd_ctrl[E_SYS_TLB_RMM_OthRTULinkSock2].thrd_id;

    }

    g_bRMMLinkTaskWorking = true;

    RMM_link_timer_id = wdCreate(WatchDogChid , RMM_link_timer_id , RMM_LINK , RMM_OTHER_LINK_TASK_ID_INDEX);
    if(RMM_link_timer_id == -1)
    {
      g_pEventLog->LogMessage((CHAR *)"RMM other link task create watchdog ID error.\n");
    }
    RMM_conn1_timer_id = wdCreate(WatchDogChid , RMM_conn1_timer_id , RMM_CONN1 , RMM_LINK1_CONNECT_TASK_ID_INDEX);
    if(RMM_conn1_timer_id == -1)
    {
      g_pEventLog->LogMessage((CHAR *)"RMM link1 connection task create watchdog ID error.\n");
    }
    RMM_conn2_timer_id = wdCreate(WatchDogChid , RMM_conn2_timer_id , RMM_CONN2 ,RMM_LINK2_CONNECT_TASK_ID_INDEX);
    if(RMM_conn2_timer_id == -1)
    {
      g_pEventLog->LogMessage((CHAR *)"RMM link2 connection task create watchdog ID error.\n");
    }

    //create Other RTU link threads
    rstatus = SYS_ThrdInit(E_SYS_TLB_RMM_OthRTULinkTask);
    if(rstatus != E_ERR_Success)
    {
       #ifdef CFG_PRN_ERR
       printf("ERR  [RMM] OtherRTULinkInit, create Other RTU Link Task "
              "fail\n");
       #endif // CFG_PRN_ERR
       g_pEventLog->LogMessage((CHAR *)
         "RMM spwan other RTU link task error.\n");
       return ERROR;
    }
    g_nOtherRTULinkTaskID =
      pSYS_CB->thrd_ctrl[E_SYS_TLB_RMM_OthRTULinkTask].thrd_id;

    // Other RTU Connection task1
    rstatus = SYS_ThrdInit(E_SYS_TLB_RMM_OthRTUConnTask1);
    if(rstatus != E_ERR_Success)
    {
       #ifdef CFG_PRN_ERR
       printf("ERR  [RMM] OtherRTULinkInit, create Other RTU Connection Task1 "
              "fail\n");
       #endif // CFG_PRN_ERR
       g_pEventLog->LogMessage((CHAR *)
         "RMM spwan other RTU Link1 connection task error.\n");
       return ERROR;
    }
    g_anOtherRTUConnectionTaskID[0] =
      pSYS_CB->thrd_ctrl[E_SYS_TLB_RMM_OthRTUConnTask1].thrd_id;

    rstatus = SYS_ThrdInit(E_SYS_TLB_RMM_OthRTUConnTask2);
    if(rstatus != E_ERR_Success)
    {
       #ifdef CFG_PRN_ERR
       printf("ERR  [RMM] OtherRTULinkInit, create Other RTU Connection Task2 "
              "fail\n");
       #endif // CFG_PRN_ERR
       g_pEventLog->LogMessage((CHAR *)
         "RMM spwan other RTU Link2 connection task error.\n");
       return ERROR;
    }
    g_anOtherRTUConnectionTaskID[1] =
      pSYS_CB->thrd_ctrl[E_SYS_TLB_RMM_OthRTUConnTask2].thrd_id;

    rstatus = SYS_ThrdInit(E_SYS_TLB_RMM_StdbyReadSock2);
    if(rstatus != E_ERR_Success)
    {
       #ifdef CFG_PRN_ERR
       printf("ERR  [RMM] OtherRTULinkInit, create Stdby RTU Read Socket2 "
              "fail\n");
       #endif // CFG_PRN_ERR
       g_pEventLog->LogMessage((CHAR *)
         "StandByReadSocketLink2 task error.\n");
       return ERROR;
    }

    rstatus = SYS_ThrdInit(E_SYS_TLB_RMM_StdbyReadSock1);
    if(rstatus != E_ERR_Success)
    {
       #ifdef CFG_PRN_ERR
       printf("ERR  [RMM] OtherRTULinkInit, create Stdby RTU Read Socket1 "
              "fail\n");
       #endif // CFG_PRN_ERR
       g_pEventLog->LogMessage((CHAR *)
         "StandByReadSocketLink1 task error.\n");
       return ERROR;
    }

    if(nReturn == ERROR)
    {
      OtherRTULinkStop();
    }
  }
  return nReturn;
}

/*******************************************************************************
Purpose:
  Main task for link between RTUs. Primary RTU will connect to Other RTU as a
  client. Standby RTU will listen other RTU as a server. During initialization
  period, It will be a server and a client.

Return:
  Nil.

History

     Name       Date          Remark

 Bryan Chong  16-Jun-2011  Add the log message when watchdog fail to restart

*******************************************************************************/
void *OtherRTULinkTask(void *arg)
{
  while(1)
  {
    delay(10);
    if(wdStart(RMM_link_timer_id , 30 , 0 , 0, 0)<0 ) //timeout 10 -----> 30
    {
      //20110616 BC
      g_pEventLog->LogMessage((CHAR *)
          "ERR  [RMM] OtherRTULinkTask, fail to restart watchdog\n");
    }

    if(g_bRMMLinkTaskWorking == false)
    {
      if(RMM_link_timer_id != -1)
      {
        wdCancel(RMM_link_timer_id);
        timer_delete(RMM_link_timer_id);

      }

      pthread_detach(pthread_self());
      pthread_exit(NULL);

      return NULL;
    }
    g_pCRTULink->Main();  //040426 Yu Wei
  }

  #ifdef CFG_PRN_ERR
  printf("ERR  [RMM] OtherRTULinkTask, thread exit\n");
  #endif // CFG_PRN_ERR
}


/**********************************************************************************
Purpose:
  Stop all interface link between RTUs tasks.

Return:
  Nil.
***********************************************************************************/
void OtherRTULinkStop(void)
{
  int nI;

  if(g_pCRTULink != NULL)
  {
    for(nI=0; nI<2; nI++)
    {
      if(g_anOtherRTULinkSocketTaskID[nI] != ERROR)
      {
        pthread_detach(g_anOtherRTULinkSocketTaskID[nI]);
        pthread_cancel(g_anOtherRTULinkSocketTaskID[nI]);
      }

      if(g_apOtherRTUServerSocket[nI] != NULL)
      {
        delete g_apOtherRTUServerSocket[nI];
      }
    }
    g_bRMMLinkTaskWorking = false;

  }
}
/*************************************************************
Purpose:
  Connect LAN2 (RTU link 1) task.

History

     Name       Date          Remark

 Bryan Chong  16-Jun-2011  Add the log message when watchdog fail to restart

**************************************************************/
void *OtherRTUConnectionTask1(void *arg)
{

  while(1)
  {
    delay(RMM_RTU_LINK_CONNECT_TASK_SLEEP);

    if(g_bRMMLinkTaskWorking == false)
    {
      timer_delete(RMM_conn1_timer_id);
      pthread_detach(pthread_self());
      pthread_exit(NULL);
      return NULL;
    }

    //changed the timeout from 10sec to 15
    if(wdStart(RMM_conn1_timer_id , RMM_CONNECT1_WATCHDOG_TIMEOUT, 0, 0, 0)< 0)
    {
      #ifdef CFG_PRN_ERR
      printf("ERR  [RMM] OtherRTUConnectionTask1, watch dog RMM_conn1 start "
             "error\n");
      #endif // CFG_PRN_ERR
      // 20110616 BC
      g_pEventLog->LogMessage((CHAR *)
          "ERR  [RMM] OtherRTUConnectionTask1, fail to start watchdog\n");
    }

    if(g_tRTUStatus.nRTUStateFlag != STATEFLAG_STANDBY)
    {
      g_pCRTULink->ConnectLink0();
    }

    if(RMM_conn1_timer_id != -1)
    {
      wdCancel(RMM_conn1_timer_id);
    }
  }// while (1)

  #ifdef CFG_PRN_ERR
  printf("ERR  [RMM] OtherRTUConnectionTask1, thread exit\n");
  #endif // CFG_PRN_ERR

}

/*************************************************************
Purpose:
  Connect LAN1 (RTU link 2) task.

History

     Name       Date          Remark

 Bryan Chong  16-Jun-2011  Add error message when watchdog fail to restart

**************************************************************/
void *OtherRTUConnectionTask2(void *arg)
{

  while(1)
  {
    delay(RMM_RTU_LINK_CONNECT_TASK_SLEEP );

    if(g_bRMMLinkTaskWorking == false)
    {
      timer_delete(RMM_conn2_timer_id);
      pthread_detach(pthread_self());
      pthread_exit(NULL);
      return NULL;
    }
    if(RMM_conn2_timer_id != TYP_ERROR)
    {
      if(wdStart(RMM_conn2_timer_id , RMM_CONNECT2_WATCHDOG_TIMEOUT, 0 ,0, 0) < 0 )
      {
        #ifdef CFG_PRN_ERR
        printf("ERR [RMM] OtherRTUConnectionTask2, watch dog RMM_conn2 start "
               "error\n");
        #endif // CFG_PRN_ERR

        //20110616 BC
        g_pEventLog->LogMessage((CHAR *)
          "ERR  [RMM] OtherRTUConnectionTask1, fail to restart watchdog\n");
      }
    }

    if(g_tRTUStatus.nRTUStateFlag != STATEFLAG_STANDBY)
    {
      g_pCRTULink->ConnectLink1();
    }

    if(RMM_conn2_timer_id != NULL)
    {
      wdCancel(RMM_conn2_timer_id);

    }
  }

  #ifdef CFG_PRN_ERR
  printf("ERR  [RMM] OtherRTUConnectionTask2, thread exit\n");
  #endif // CFG_PRN_ERR
}


/*************************************************************
Purpose:
  Link 1 listen task.

Return:
  Nil.
**************************************************************/
void *OtherRTUServerSocketTask1(void *arg)
{
  delay(100);
  g_apOtherRTUServerSocket[0]->ListenTask();

  #ifdef CFG_PRN_ERR
  printf("ERR  [RMM] OtherRTUServerSocketTask1, thread exit\n");
  #endif // CFG_PRN_ERR

  return TYP_NULL;
}

/*************************************************************
Purpose:
  Link 2 listen task.

Return:
**************************************************************/
void *OtherRTUServerSocketTask2(void *arg)
{
  delay(100);
  g_apOtherRTUServerSocket[1]->ListenTask();

  #ifdef CFG_PRN_ERR
  printf("ERR  [RMM] OtherRTUServerSocketTask2, thread exit\n");
  #endif // CFG_PRN_ERR

  return TYP_NULL;
}
/*************************************************************
Purpose:
  Standby LAN2 (RTU link 1) task.

**************************************************************/
void *StandByReadSocketLink1(void *arg)
{
  g_pCRTULink->StandByGetData(1);

  #ifdef CFG_PRN_ERR
  printf("ERR  [RMM] StandByReadSocketLink1, thread exit\n");
  #endif // CFG_PRN_ERR

  return NULL;
}

/*************************************************************
Purpose:
  Standby LAN1 (RTU link 2) task.

**************************************************************/
void *StandByReadSocketLink2(void *arg)
{
  //printf("in StandByReadSocket2\n");
  g_pCRTULink->StandByGetData(0);

  #ifdef CFG_PRN_ERR
  printf("ERR  [RMM] StandByReadSocketLink2, thread exit\n");
  #endif // CFG_PRN_ERR

  return NULL;
}



/**********************************************************************************
Purpose:
  CRTULink constructor. It initializes object.

History

  Name         Date       Remark
  ----         ----       ------
  Bryan Chong  15-May-09  Update g_tRTUConfig.tLAN_Para[x].cLAN_SlaveAddress
                          to g_tRTUConfig.tLAN_Para[x].tPrtc.addr due to
                          parameter change in latest Init_Config.cpp

**********************************************************************************/
CRTULink::CRTULink(void)
{
  int nI;

  ClearVariables();  //Clear all variables. //040429 Yu Wei

  //RTU status table read command
  m_unStatusTableLen =
    g_tRTUConfig.tRTUStatusTable.unTableEndAddress -
    g_tRTUConfig.tRTUStatusTable.unTableStartAddress +1;    //Table length.

  m_nTableBytes = m_unStatusTableLen*2;                //Table length in byte.

  //Construct Read RTU Status command 20100515 BC (Rqd by ZSL)
  m_acReadRTUStatus[0] = g_tRTUConfig.tLANPara[0].tPrtc.addr;
  m_acReadRTUStatus[1] = g_tRTUConfig.tLANPara[1].tPrtc.command;

  *(unsigned short *)&m_acReadRTUStatus[2] =
    g_tRTUConfig.tRTUStatusTable.unTableStartAddress;      //Start address is same as local.

  *(unsigned short *)&m_acReadRTUStatus[4] = STR_ByteSwap16Bit(m_unStatusTableLen);

  *(unsigned short *)&m_acReadRTUStatus[6] =
    STR_ByteSwap16Bit(ModbusCRC((unsigned char *)m_acReadRTUStatus,6));  //Checksum.

  //Write RTU status command. The slave address is same.
  //m_acWriteRTUStatus[0] = g_tRTUConfig.tLANPara[0].cLAN_SlaveAddress;
  m_acWriteRTUStatus[0] = g_tRTUConfig.tLANPara[0].tPrtc.addr;
  m_acWriteRTUStatus[1] = 0x10;                      //Write table;
  *(unsigned short *)&m_acWriteRTUStatus[2] =
    STR_ByteSwap16Bit(WRITE_RTU_STATUS_ADDRESS);

  *(unsigned short *)&m_acWriteRTUStatus[4] = STR_ByteSwap16Bit(m_unStatusTableLen);
  m_acWriteRTUStatus[6] = m_nTableBytes;

  //Write SWC polling table command. The slave address is same.
  //m_acWriteSWCPollingTable[0] = g_tRTUConfig.tLANPara[0].cLAN_SlaveAddress;
  m_acWriteSWCPollingTable[0] = g_tRTUConfig.tLANPara[0].tPrtc.addr;
  m_acWriteSWCPollingTable[1] = 0x10;

  //Set timeout type.
  m_anPollingIntervalType[0] = RTU_LINK1_POLLING;    //040408 Yu Wei
  m_anPollingIntervalType[1] = RTU_LINK2_POLLING;    //040408 Yu Wei
  m_anPollingTimeoutType[0] = RTU_LINK1_POLLING_TO;  //040408 Yu Wei
  m_anPollingTimeoutType[1] = RTU_LINK2_POLLING_TO;  //040408 Yu Wei
  m_anFailureTimeoutType[0] = RTU_LINK1_FAILURE;
  m_anFailureTimeoutType[1] = RTU_LINK2_FAILURE;
  //m_anRtuServerTimeoutType[0] = RTU_SERVER_LINK1_TIMEOUT;
  //m_anRtuServerTimeoutType[1] = RTU_SERVER_LINK2_TIMEOUT;
  m_anCheckLink2Type = CHECK_RTU_LINK2;    //040426 Yu Wei

  for(nI=0; nI<2; nI++)
  {
    strcpy(m_acIPAddress[nI],g_tRTUConfig.acOtherRTUIPAddress[nI^1]);    //copy IP address.

    //RTU1 connection socket use base socket ID,
    //RTU2 connection use base socket ID+1.    //040520 Yu Wei
    if(g_tRTUConfig.nRTUID == 1)
    {
      m_anPort[nI] = g_tRTUConfig.tLANPara[nI^1].anOtherRTULinkSocketID;    //Copy port ID.
    }
    else
    {
      m_anPort[nI] = g_tRTUConfig.tLANPara[nI^1].anOtherRTULinkSocketID+1;    //Copy port ID.
    }    //040520 Yu Wei

    m_anPollingInterval[nI] = g_tRTUConfig.tLANPara[nI^1].nPollingTime;    //Copy Polling rate.
    m_anFailureTimeout[nI] = g_tRTUConfig.tLANPara[nI^1].nPollingTimeout;  //20140603Yang Tao: Enlarge the timeout to test
    //m_anFailureTimeoutStandby[nI] = g_tRTUConfig.tLANPara[nI^1].nPollingTimeout; //Standby RTU failure timeout.
    m_anRetry[nI] = g_tRTUConfig.tLANPara[nI^1].nPollingRetryNumber;    //Copy re-try times.

    m_abConnectFlag[nI] = false;    //Set Link is not connected.

    m_anLinkDownTimeout[nI] = g_tRTUConfig.tLANPara[nI^1].nPollingTimeout *
                  g_tRTUConfig.tLANPara[nI^1].nPollingRetryNumber;// 20140603Yang Tao: Enlarge the timeout to test

    m_anMasterLinkDownMonitorTimeout[nI] = SOCKET_CONNECT_TIMEOUT*5;	//20150715 Su 2-4
    m_anSlaveLinkDownMonitorTimeout[nI] = SOCKET_CONNECT_TIMEOUT*10;	//20150715 Su 2-4
    //Reset timeout counter.
    ActivateTimeOut(m_anPollingIntervalType[nI]);  //040408 Yu Wei
    ActivateTimeOut(m_anPollingTimeoutType[nI]);
    ActivateTimeOut(m_anFailureTimeoutType[nI]);
  }

  m_anServerLinkTimeout = m_anLinkDownTimeout[1];    //040426 Yu Wei
  ActivateTimeOut(m_anCheckLink2Type);

  m_anLinkDownTimeout[1] += SOCKET_CONNECT_TIMEOUT;  //Link 2 should wait for extra time.
  m_PrimarycloseFdFlagTimeout = SOCKET_CONNECT_TIMEOUT*15;// Yang Tao20150402 Set the timeout flag as 10 second.
  m_tWriteCommand.nStartPoint = 0;
  m_tWriteCommand.nEndPoint = 0;

  //040323 Yu Wei
  //m_acWriteCommand[0] = g_tRTUConfig.tLANPara[0].cLAN_SlaveAddress;
  m_acWriteCommand[0] = g_tRTUConfig.tLANPara[0].tPrtc.addr;
  m_acWriteCommand[1] = 0x10;
  *(unsigned short *)&m_acWriteCommand[2] = WRITE_COMMAND_ADDRESS;  //0xffff don't need to change byte order
  m_acWriteCommand[4] = 0x00;
  m_acWriteCommand[5] = 0x01;
  m_acWriteCommand[6] = 2;

  m_nPollingCommand = POLLING_READ_TABLE;
  m_nRetry = 0;
  m_nWriteCommandResult = SEND_COMMPLETE_NULL;

  m_nSWCPollingTableID = 0;
  m_nSWCPollingTableTypeID = FAST_TABLE;
  #ifdef ENABLE_GET_NEXT_SWC_TABLE_UPDATE
  m_nWriteTableSWCIndex = 0;
  #endif // ENABLE_GET_NEXT_SWC_TABLE_UPDATE

  m_bSendCmdToRtuFlag = true;    //040426 Yu Wei

  m_nLANLinkID = 0;    //040426 Yu Wei

  m_nCurrentStateFlag = STATEFLAG_INITIALIZATION;  //040426 Yu Wei
  m_PrimarycloseFdFlag = false;// Yang Tao 20150402 // initialize
  m_Lan2LinkDown = true;
  m_Lan1LinkDown = true;


#ifdef ModuleTest
  LogLinkValues();  //for test.
#endif

}

/**********************************************************************************
Purpose:
  CRTUStatus destructor.

**********************************************************************************/
CRTULink::~CRTULink(void)
{
}

/**********************************************************************************
Purpose:
  Clear object variables.
  Added in 23 March 2004, Yu Wei

***********************************************************************************/
void CRTULink::ClearVariables(void)
{
  memset ( m_acRecvOtherRTUBuffer, 0x00, sizeof(char)*MODBUS_MAX_CMD_SIZE);
  memset ( m_acReadRTUStatus, 0x00, sizeof(char)*MODBUS_MAX_CMD_SIZE);
  memset ( m_acWriteRTUStatus, 0x00, sizeof(char)*MODBUS_MAX_CMD_SIZE);
  memset ( m_acWriteSWCPollingTable, 0x00, sizeof(char)*MODBUS_MAX_CMD_SIZE);
  memset ( m_acReplyRTUStatus, 0x00, sizeof(char)*MODBUS_MAX_CMD_SIZE);
  memset ( m_acWriteCommand, 0x00, sizeof(char)*MODBUS_MAX_CMD_SIZE);
  //m_unStatusTableLen = 0;  //Done by constructor. 040426 Yu Wei
  //m_nTableBytes = 0;    //Done by constructor. 040426 Yu Wei
  memset ( m_anRTULinkFD, 0x00, sizeof(int)*2);
  memset ( m_anPollingInterval, 0x00, sizeof(int)*2);
  memset ( m_anFailureTimeout, 0x00, sizeof(int)*2);
  memset ( m_anRetry, 0x00, sizeof(int)*2);
  memset ( m_acIPAddress, 0x00, sizeof(char)*2*20);
  memset ( m_anPort, 0x00, sizeof(int)*2);
  //m_abConnectFlag[0] = false;  //Done by constructor. 040426 Yu Wei
  //m_abConnectFlag[1] = false;  //Done by constructor. 040426 Yu Wei
  memset ( m_anPollingIntervalType, 0x00, sizeof(int)*2);  //040408 Yu Wei
  memset ( m_anPollingTimeoutType, 0x00, sizeof(int)*2);
  memset ( m_anFailureTimeoutType, 0x00, sizeof(int)*2);
  //memset ( m_anRtuServerTimeoutType, 0, sizeof(int)*2);
  memset ( m_anLinkDownTimeout, 0x00, sizeof(int)*2);
  memset ( m_atTimeOut, 0x00, sizeof(timespec)*RTU_LINK_MAX_TIMEOUT_TYPE);
  //m_nPollingCommand = 0;  //Done by constructor. 040426 Yu Wei
  //m_nRetry = 0;        //Done by constructor. 040426 Yu Wei
  //m_nSWCPollingTableID = 0;  //Done by constructor. 040426 Yu Wei
  //m_nSWCPollingTableTypeID = 0;  //Done by constructor. 040426 Yu Wei
  //m_nWriteCommandResult = 0;  //Done by constructor. 040426 Yu Wei
}

/**********************************************************************************
Purpose:
  RTU communication Main process. Must complete a circle before changing state.
  Added in 26 April 2004, Yu Wei
**********************************************************************************/
void CRTULink::Main(void)
{
	   if( ( m_PrimarycloseFdFlag==true) && (CheckTimeOut(m_PrimarycloseFdType,m_PrimarycloseFdFlagTimeout)==true))
	   {
			  m_PrimarycloseFdFlag=false;
			 // printf("m_PrimarycloseFdFlag reach timeout\n");

	   }
  if ((m_nCurrentStateFlag != STATEFLAG_PRIMARY) &&
    (m_nCurrentStateFlag != STATEFLAG_SWITCHING_PTOS) &&  //Switching P-to-S, don't do slave. 040322 Yu Wei
    (m_nCurrentStateFlag != STATEFLAG_HARDWARETEST) &&    //hardware test and system reset will
    (m_nCurrentStateFlag != STATEFLAG_SYSTEM_RESET) &&    //state stop interface between RTU.  //040611 Yu Wei
    (m_nCurrentStateFlag !=  STATEFLAG_INITIALIZATION))
  {
    delay(100);
  }

  if((m_nCurrentStateFlag == STATEFLAG_INITIALIZATION) ||
    (m_nCurrentStateFlag == STATEFLAG_PRIMARY)||
    (m_nCurrentStateFlag == STATEFLAG_SWITCHING_PTOS))
  {
    LinkMaster();
  }
  SetStateFlag();
}

/*******************************************************************************
Purpose:
  This routine initiate the LAN communication between RTUs. When the
  LAN communication is established, RTU will start to send polling command to
  other RTU based enabling the flag, m_bSendCmdToRtuFlag.

Input:
  None

Return:
  None

History
    Name           Date          Remark
    ----           ----          ------
  Yu, Wei        12-Aug-2004   Initial revision

*******************************************************************************/
void CRTULink::LinkMaster(void)
{
  int nLANLinkID;    //Link ID. (0 -- LAN2,  1 -- LAN1)
  int nConnectionResult;  //Connection result (ERROR/OK).
  int nRWTableResult;    //Read/Write table result (ERROR/OK).
  char cLog[1000];
  nConnectionResult = OK;    //040426 Yu Wei
  nRWTableResult = ERROR;    //040426 Yu Wei
//  char actemp[100] = {0};
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_LINKMASTER)
  CHAR namebuff[100] = {0};
  //E_ERR_T rstatus = E_ERR_Success;
  //struct timespec   tspec;
  //struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_LINKMASTER)
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};

  if(m_bSendCmdToRtuFlag == true)    //040426 Yu Wei
  {
    if(ConnectOtherRTU(0) == OK)
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_LINKMASTER)
      printf("[RMM] CRTULink::LinkMaster, %s\n"
             "  connect to RTU using LAN 2 OK\n",
             SYS_GetDefaultThrdName(namebuff, sizeof(namebuff)));
      #endif // ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_RMMLINK))

      nConnectionResult = OK;

      if((ConnectOtherRTU(1) == OK) &&
         (CheckTimeOut(m_anCheckLink2Type, m_anServerLinkTimeout)== true))
      {
        m_nLANLinkID = 1;      //LAN1 work.
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_LINKMASTER)
        printf("[RMM] CRTULink::LinkMaster, %s fd %d LAN %d, linkID %d connect "
               "to other RTU OK\n",
               SYS_GetDefaultThrdName(namebuff, sizeof(namebuff)),
               m_anRTULinkFD[m_nLANLinkID], ((m_nLANLinkID == 1)? 1 : 2),
               m_nLANLinkID);
        #endif // ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_RMMLINK))
        ActivateTimeOut(m_anCheckLink2Type);

      }
      else
      {
        m_nLANLinkID = 0;      //LAN2 work.
  	  if(CheckTimeOut(m_anFailureTimeoutType[1],
  				  m_anMasterLinkDownMonitorTimeout[1])== true)  //040224 Yu Wei
  		{
  		  ActivateTimeOut(m_anFailureTimeoutType[1]);
  		  g_tRTUStatus.abOtherLinkStatus[1] = false;
  		  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_LINKMASTER)
  		  printf("[RMM] CRTULink::LinkMaster, LAN 4 timeout at %d ms, set "
  				 "abOtherLinkStatus[1] = %d\n",
  				 m_anLinkDownTimeout[1],
  				 g_tRTUStatus.abOtherLinkStatus[1]);
  		  #endif // ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_RMMLINK))
//  		sprintf(cLog, "[RMM] CRTULink::LinkMaster, LAN 4 timeout at %d ms, set "
//  				 "abOtherLinkStatus[1] = %d", m_anLinkDownTimeout[1], g_tRTUStatus.abOtherLinkStatus[1]);
//  		g_pDebugLog->LogMessage(cLog);
  		  CloseConnectOtherRTU(1);
  		}
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_LINKMASTER)
        printf("[RMM] CRTULink::LinkMaster, %s\n"
               "  link %d fd %d connect to RTU using LAN 2 OK\n",
               SYS_GetDefaultThrdName(namebuff, sizeof(namebuff)),
               m_nLANLinkID, m_anRTULinkFD[m_nLANLinkID]);
        #endif // ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_RMMLINK))
      }

    }
    else
    {
      // connect to other RTU is NG

      if(CheckTimeOut(m_anFailureTimeoutType[0],m_anLinkDownTimeout[0])== true)  //040224 Yu Wei
      {
        ActivateTimeOut(m_anFailureTimeoutType[0]);
//        sprintf(cLog,  "[RMM] (4),g_tRTUStatus.abOtherLinkStatus[0] = false;\n");
//			g_pEventLog->LogMessage(cLog);
        g_tRTUStatus.abOtherLinkStatus[0] = false;
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_LINKMASTER)
        printf("[RMM] CRTULink::LinkMaster, LAN 2 timeout at %d ms, set "
               "abOtherLinkStatus[0] = %d\n",
               m_anLinkDownTimeout[0],
               g_tRTUStatus.abOtherLinkStatus[0]);
        #endif // ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_RMMLINK))
        CloseConnectOtherRTU(0);
      }

      m_nLANLinkID = 1;      //LAN1 work.
      if(ConnectOtherRTU(1) == OK)
      {
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_LINKMASTER)
        printf("[RMM] CRTULink::LinkMaster, link %d fd %d using LAN 1 to "
               "connect to RTU OK\n",
               m_nLANLinkID, m_anRTULinkFD[m_nLANLinkID]);
        #endif // ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_RMMLINK))
        nConnectionResult = OK;
      }
      else
      {
        nConnectionResult = ERROR;  //Two link down.
      }
    }
  }    //040426 Yu Wei

  if(nConnectionResult == OK)
  {
    while(m_bSendCmdToRtuFlag == true)    //040426 Yu Wei
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_LINKMASTER)
      if(m_nLANLinkID == 1)
      {
        printf("[RMM] CRTULink::LinkMaster, %s using m_nLANLinkID %d, "
               "fd %d to tx cmd to other RTU, m_bSendCmdToRtuFlag = %d\n",
               SYS_GetDefaultThrdName(namebuff, sizeof(namebuff)),
               m_nLANLinkID, m_anRTULinkFD[m_nLANLinkID], m_bSendCmdToRtuFlag);
      }
      #endif // ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_RMMLINK))
      SendCommandToOtherRTU(m_nLANLinkID);  //040426 Yu Wei

      //No command send to standby RTU.  //040708 Yu Wei
      if(m_bSendCmdToRtuFlag == true)
      {
        //Wait for 5ms, the other task can work.  //040708 Yu Wei
        delay(50*SYS_CLOCK_MSECOND);
      }
    }

    nRWTableResult = GetResponseFromOtherRTU(m_nLANLinkID);  //040426 Yu Wei

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_LINKMASTER)
    printf("[RMM] CRTULink::LinkMaster, %s linkID %d tx cmd %s\n",
           SYS_GetDefaultThrdName(namebuff, sizeof(namebuff)),
           m_nLANLinkID, (nRWTableResult == OK) ? "OK" : "FAIL");
    #endif // ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_RMMLINK))

  }

  if((nConnectionResult == ERROR) || (nRWTableResult == ERROR) )
  {
    for(nLANLinkID=0; nLANLinkID<2; nLANLinkID++)  //040426 Yu Wei
    {
      if(CheckTimeOut(m_anFailureTimeoutType[nLANLinkID],
                      m_anLinkDownTimeout[nLANLinkID])== true)
      {
        ActivateTimeOut(m_anFailureTimeoutType[nLANLinkID]);
        if(g_tRTUStatus.abOtherLinkStatus[nLANLinkID] == true)
        {
//       sprintf(cLog,  "[RMM] (5),nConnectionResult= %d,nRWTableResult= %dg_tRTUStatus.abOtherLinkStatus[%d] = false;\n",nConnectionResult,nRWTableResult,nLANLinkID);
//    			g_pEventLog->LogMessage(cLog);
          g_tRTUStatus.abOtherLinkStatus[nLANLinkID] = false;
          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_LINKMASTER)
          printf("[RMM] CRTULink::LinkMaster, timeout at %d ms, set "
                 "abOtherLinkStatus[%d] = %d\n",
                 m_anLinkDownTimeout[nLANLinkID], nLANLinkID,
                 g_tRTUStatus.abOtherLinkStatus[nLANLinkID]);
          #endif // ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_RMMLINK))

//          pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_ServerCommand,
//            FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec,
//            FLG_LOG_T_Debug,
//            "[RMM] CRTULink::LinkMaster, %s\n"
//            "  RTU-RTU master link fd %d is NG after timeout of %d ms\n",
//            SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
//            m_anRTULinkFD[nLANLinkID], m_anLinkDownTimeout[nLANLinkID]);

//          sprintf(cLog,  "[RMM] CRTULink::LinkMaster, %s: RTU-RTU master link fd %d is NG after timeout of %d ms\n",
//                  SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
//                  m_anRTULinkFD[nLANLinkID], m_anLinkDownTimeout[nLANLinkID]);
//          g_pDebugLog->LogMessage(cLog);
        }

        CloseConnectOtherRTU(nLANLinkID);

      }
    }
  }else{
    if(g_tRTUStatus.abOtherLinkStatus[m_nLANLinkID] == false)
    {
//        sprintf(cLog,  "[RMM] (6),g_tRTUStatus.abOtherLinkStatus[%d] = true;\n",nLANLinkID);
//     			g_pEventLog->LogMessage(cLog);
      g_tRTUStatus.abOtherLinkStatus[m_nLANLinkID] = true;
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_LINKMASTER)
      printf("[RMM] CRTULink::LinkMaster, set abOtherLinkStatus[%d] = %d\n",
             m_nLANLinkID, g_tRTUStatus.abOtherLinkStatus[m_nLANLinkID]);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_PRN_WARN_RMMLINK)
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_ServerCommand,
//        FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//        "[RMM] CRTULink::LinkMaster, %s\n"
//        "  RTU-RTU master link fd %d is OK\n",
//        SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
//        m_anRTULinkFD[m_nLANLinkID]);
//      sprintf(cLog, "[RMM] CRTULink::LinkMaster, %s: RTU-RTU master link fd %d is OK\n",
//    	        SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
//    	        m_anRTULinkFD[m_nLANLinkID]);
//      g_pDebugLog->LogMessage(cLog);
    }
    ActivateTimeOut(m_anFailureTimeoutType[m_nLANLinkID]);
    m_bSendCmdToRtuFlag = true;  //040426 Yu Wei
  }

}// LinkMaster
/**********************************************************************************
Purpose:
  StandBy RTU get data .

Input:
  nLANLinkID  --  LAN link ID.(in) (0 -- LAN2, 1 -- LAN1)

**********************************************************************************/
void CRTULink::StandByGetData( int nLANLinkID)
{
  while(1)
  {
    if(g_bRMMLinkTaskWorking == false)
    {
      pthread_detach(pthread_self());
      pthread_exit(NULL);
    }

    //Switching P-to-S, don't do slave. 040322 Yu Wei
    //hardware test and system reset will
    //state stop interface between RTU.  //040611 Yu Wei
    if ((m_nCurrentStateFlag != STATEFLAG_PRIMARY) &&
        (m_nCurrentStateFlag != STATEFLAG_SWITCHING_PTOS) &&
        (m_nCurrentStateFlag != STATEFLAG_HARDWARETEST) &&
        (m_nCurrentStateFlag != STATEFLAG_SYSTEM_RESET)&&
        (m_nCurrentStateFlag != STATEFLAG_INITIALIZATION) )
    {

      if(nLANLinkID == 0 )
      {
        delay(5);
        LinkSlave(0);
      }
      if(nLANLinkID == 1)
      {
        delay(5);
        LinkSlave(1);
      }
    }
    else
    {
      delay(10);
    }
  }


  #ifdef CFG_PRN_ERR
  printf("ERR  [RMM] OtherRTUServerSocketTask1, thread exit\n");
  #endif // CFG_PRN_ERR
  return;
}

/**********************************************************************************
Purpose:
  Send command to other RTU. If there is error, it will close the connection.

Input:
  nLANLinkID  --  LAN link ID.(in) (0 -- LAN2, 1 -- LAN1)
  acCommand  --  The buffer for command. (in)
  nLength    --  The size of command. (in)
**********************************************************************************/
void CRTULink::SendOrClose(int nLANLinkID, char *acCommand, int nLength)

{
  char cprnbuff[512];
  int sendlength;

  struct timespec   tspec;
  struct tm         *pbdtime;

  E_ERR_T rstatus = E_ERR_Success;

  #ifdef CFG_PRN_ERR
  CHAR errmsg[100] = {0};
  #endif // CFG_PRN_ERR

  if((m_anRTULinkFD[nLANLinkID] == TYP_ERROR) ||
     (m_anRTULinkFD[nLANLinkID] == TYP_NULL))
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [RMM] CRTULink::SendOrClose, invalid file descriptor, %d\n",
            m_anRTULinkFD[nLANLinkID]);
    #endif // CFG_PRN_ERR
    return;
  }
  ActivateTimeOut(m_anPollingTimeoutType[nLANLinkID]);    //040426 Yu Wei

  //timout 5 milliseconds
  //sendlength = Send(m_anRTULinkFD[nLANLinkID], acCommand, nLength, 5);

  rstatus = LNC_SendMsg(m_anRTULinkFD[nLANLinkID], acCommand, nLength,
                        &sendlength);
  if(rstatus != E_ERR_Success)
  {
    #ifdef CFG_PRN_ERR
    ERR_GetMsgString(rstatus, errmsg);
    printf("ERR  [RMM] CRTULink::SendOrClose, errmsg %s\n", errmsg);
    #endif // CFG_PRN_ERR
  }
  if(sendlength < 0)
  {
    rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec, E_SYS_TIME_Local);
    sprintf(cprnbuff, "ERR  [RMM] %d.%02d.%02d %02d:%02d:%02d.%03d "
      "[RMM] CRTULink::SendOrClose, LAN link %d tx cmd %d bytes\n",
      (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
      pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
      pbdtime->tm_sec, tspec.tv_nsec/1000000,
      nLANLinkID, nLength);
    g_pEventLog->LogMessage(cprnbuff);

    CloseConnectOtherRTU(nLANLinkID);
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK)
    printf("[RMM] CRTULink::SendOrClose, tx cmd fail, close connection to "
           "other RTU. Cmd bytes:\n");
    SYS_PrnDataBlock((const UINT8 *)acCommand, nLength, 10);
    #endif // ((define CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))

  }
  #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMMLINK)
  printf("[RMM] CRTULink::SendOrClose, using LAN %d to tx cmd to RTU OK:\n",
         ((nLANLinkID == 1) ? 1 : 2));
  SYS_PrnDataBlock((const UINT8 *)acCommand, nLength, 10);
  #endif // ((define CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
}

/**********************************************************************************
Purpose:
  Send command to other RTU.
  Re-designed in 26 April 2004, Yu Wei

Input:
  nLANLinkID  --  LAN link ID. (0 -- LAN2, 1 -- LAN1)

**********************************************************************************/
void CRTULink::SendCommandToOtherRTU(int nLANLinkID)
{
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
  E_ERR_T rstatus = E_ERR_Success;
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
  #if ((defined CFG_DEBUG_MSG) && \
       (CFG_DEBUG_RMMLINKCHK || CFG_DEBUG_RMMLINK_SENDCMD))
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)

  switch(m_nPollingCommand)
  {
  case WRITE_COMMAND:
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK_SENDCMD))
    printf("[RMM] CRTULink::SendCommandToOtherRTU, %s fd %d tx cmd to RTU, "
           "WRITE_COMMAND\n",
           SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
           m_anRTULinkFD[nLANLinkID]);
    #endif // ((define CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))

    //for caculating every command's time expends,test
    if(m_nRetry < m_anRetry[nLANLinkID])    //040426 Yu Wei
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_SENDCMD)
      printf("[RMM] CRTULink::SendCommandToOtherRTU, fd %d tx cmd to RTU "
             "retry %d/%d\n",
             m_anRTULinkFD[nLANLinkID], m_nRetry, m_anRetry[nLANLinkID]);
      #endif // ((define CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
      m_nPollingCommand = WRITE_COMMAND;
      *(unsigned short *)&m_acWriteCommand[7] =
         STR_ByteSwap16Bit( m_tWriteCommand.aunCommand[m_tWriteCommand.nStartPoint]);

      //Checksum
      *(unsigned short *)&m_acWriteCommand[9] =
         STR_ByteSwap16Bit(ModbusCRC((unsigned char *)m_acWriteCommand,9));


      SendOrClose(nLANLinkID, m_acWriteCommand, MODBUS_WRITE_CMD_SIZE);
      m_nRetry ++;
      m_bSendCmdToRtuFlag = false;
    }
    else if(m_nRetry >= m_anRetry[nLANLinkID])
    {
      m_nRetry = 0;  //040426 Yu Wei
      m_nPollingCommand = POLLING_READ_TABLE;
      DeleteWriteCommand();
      m_nWriteCommandResult = SEND_COMMPLETE_ERROR;
      m_bSendCmdToRtuFlag = true;    //040426 Yu Wei
    }
    break;

  case POLLING_WRITE_SWC_TABLE:
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK_SENDCMD))
    printf("[RMM] CRTULink::SendCommandToOtherRTU, fd %d tx cmd to RTU, "
           "POLLING_WRITE_SWC_TABLE\n", m_anRTULinkFD[nLANLinkID]);
    #endif // ((define CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))

    if(g_apSWCObject[m_nSWCPollingTableID] == TYP_NULL)
      GetNextSWCTableID();

    WriteSWCTable(nLANLinkID);
    break;

  case POLLING_WRITE_TABLE:
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_SENDCMD)
    printf("[RMM] CRTULink::SendCommandToOtherRTU, fd %d tx cmd to RTU, "
           "POLLING_WRITE_TABLE\n", m_anRTULinkFD[nLANLinkID]);
    printf("[RMM] CRTULink::SendCommandToOtherRTU, m_nRetry %d, m_anRetry %d, "
           "m_bSWCLinkDownFlag %d\n",
           m_nRetry, m_anRetry[nLANLinkID],
           g_pRTUStatusTable->m_bSWCLinkDownFlag);
    #endif // ((define CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_SENDCMD)

    if(m_nRetry < m_anRetry[nLANLinkID])
    {
      //Send local RTU status table to other RTU and get response.
      //Get command string that writes local RTU status table to other RTU

      g_pRTUStatusTable->ReadRTUStatus(
        (unsigned short *)&m_acWriteRTUStatus[RTU_STATUS_TABLE_SWC_WEIGHT],10);

      if(g_pRTUStatusTable->m_bSWCLinkDownFlag == true)
      {
        //Set link check flag for other RTU, ask standby RTU check SWC
        //link status.
        *(unsigned short *)&m_acWriteRTUStatus[
          7 + RTU_STATUS_TABLE_SWC_LINK_CHECK*2] = STR_ByteSwap16Bit(0x0001);

        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
        rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                             E_SYS_TIME_Local);
        printf("[RMM] %d.%02d.%02d %02d:%02d:%02d.%03d "
               "CRTULink::SendCommandToOtherRTU, %s\n"
               "  set link check flag and tx local status table to other RTU\n",
               (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
               pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
               pbdtime->tm_sec, tspec.tv_nsec/1000000,
               SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
        //SYS_PrnDataBlock((const UINT8 *) m_acWriteRTUStatus,
          //                m_acWriteRTUStatus[6] + 9, 10);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
        //Must wait for standby RTU received the messsage to clear the flag.
      }

      //Checksum.
      *(unsigned short *)&m_acWriteRTUStatus[m_nTableBytes + 7] =
        STR_ByteSwap16Bit(ModbusCRC((unsigned char *)m_acWriteRTUStatus,
                             m_nTableBytes + 7));

      #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMMLINKCHK)
      printf("[RMM] CRTULink::SendCommandToOtherRTU, tx status table %d "
             "bytes:\n", m_acWriteRTUStatus[6] + 9);
      SYS_PrnDataBlock((const UINT8 *) m_acWriteRTUStatus,
                       m_nTableBytes + 9, 20);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))

      SendOrClose(nLANLinkID, m_acWriteRTUStatus, m_nTableBytes + 9);

      m_nRetry++;
      m_bSendCmdToRtuFlag = false;    //040426 Yu Wei
    }
    else if(m_nRetry >= m_anRetry[nLANLinkID])
    {
      if(g_pRTUStatusTable->m_bSWCLinkDownFlag == true)
      {
        g_pRTUStatusTable->m_bSWCLinkDownFlag = false;  //Standby RTU cannot receive the message
        g_pRTUStatusTable->SetSWCLinkStart();      //start SWC polling.
      }
      m_nRetry = 0;  //040426 Yu Wei
      m_nPollingCommand = POLLING_READ_TABLE;
      m_bSendCmdToRtuFlag = true;    //040426 Yu Wei
    }
    break;    //This is a bug. (Miss break) 040408 Yu Wei

  case POLLING_READ_TABLE:
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK_SENDCMD))
    printf("[RMM] CRTULink::SendCommandToOtherRTU, fd %d tx cmd to RTU, "
           "POLLING_READ_TABLE\n", m_anRTULinkFD[nLANLinkID]);
    #endif // ((define CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))

    if(m_tWriteCommand.nStartPoint != m_tWriteCommand.nEndPoint)
    {

      m_nPollingCommand = WRITE_COMMAND;
      m_nRetry = 0;
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK_SENDCMD))
      printf("[RMM] CRTULink::SendCommandToOtherRTU, m_nPollingCommand = "
             "WRITE_COMMAND\n");
      #endif // ((define CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
    }else if((CheckTimeOut(m_anPollingIntervalType[nLANLinkID],
                m_anPollingInterval[nLANLinkID])== true) &&
             (m_nRetry == 0))
    {
      //Should reset two link.
      ActivateTimeOut(m_anPollingIntervalType[0]);
      ActivateTimeOut(m_anPollingIntervalType[1]);

      SendOrClose(nLANLinkID, m_acReadRTUStatus, MODBUS_READ_CMD_SIZE);
      m_nRetry ++;
      m_bSendCmdToRtuFlag = false;    //040426 Yu Wei
    }
    else if((m_nRetry < m_anRetry[nLANLinkID]) && (m_nRetry != 0))
    {
      SendOrClose(nLANLinkID,m_acReadRTUStatus,MODBUS_READ_CMD_SIZE);
      m_nRetry ++;
      m_bSendCmdToRtuFlag = false;
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK_SENDCMD))
      printf("[RMM] CRTULink::SendCommandToOtherRTU, state %d RTU tx retry "
             "read cmd using active LAN link %d\n",
             g_tRTUStatus.nRTUStateFlag, nLANLinkID);
      SYS_PrnDataBlock((const UINT8 *) m_acReadRTUStatus, MODBUS_READ_CMD_SIZE,
                       10);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
    }
    else if(m_nRetry >= m_anRetry[nLANLinkID])
    {

      m_nRetry = 0;
      m_bSendCmdToRtuFlag = true;    //040426 Yu Wei
    }
    break;

  default:    //040310 Yu Wei
    #ifdef CFG_PRN_ERR
    printf("ERR  [RMM] CRTULink::SendCommandToOtherRTU, fd %d tx unhandle cmd "
           "to other RTU 0x%x\n", m_anRTULinkFD[nLANLinkID], m_nPollingCommand);
    #endif // ((define CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
    m_nRetry = 0;  //040426 Yu Wei
    m_nPollingCommand = POLLING_READ_TABLE;
    m_bSendCmdToRtuFlag = true;    //040426 Yu Wei
  }// switch(m_nPollingCommand)
}// SendCommandToOtherRTU

/*******************************************************************************
Purpose:
  Write SWC table to other RTU.
  Re-designed in 26 April 2004, Yu Wei

Input:
  nLANLinkID  --  LAN link ID. (0 -- LAN2, 1 -- LAN1)

History

  Name         Date          Remark
  ----         ----          ------
 Bryan Chong  12-May-2010  Fix g_apSWCObject[m_nSWCPolliongTableID]. Replace
                           m_nSWCPolliongTableID with SWC index.
 Bryan Chong  02-Jun-2010  Remove m_bGetResponseFromOtherRTU flag as LinkMaster
                           does not require to check the flag

*******************************************************************************/
void CRTULink::WriteSWCTable(int nLANLinkID)
{
  unsigned short unTableLen;
  if(m_nRetry == 0)
  {
    //20050407 PR 236 Tong

    if (!g_apSWCObject[m_nSWCPollingTableID])//20050407 PR 236 Tong
    {
      m_bSendCmdToRtuFlag = false;
      // Add to control getting response from other RTU
      // (20100514 BC, Rqd by ZSL)
      // m_bGetResponseFromOtherRTU = false;
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
      printf("[RMM] CRTULink::WriteSWCTable, when "
             "g_apSWCObject[%d] = NULL or exit routine, "
             "m_bSendCmdToRtuFlag = false\n",
             m_nSWCPollingTableID);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
      return;
    }

    if(g_apSWCObject[m_nSWCPollingTableID]->m_nLinkType == SWC_LINK_RS_485M) //20080220
    {
      if( m_nMultiNodeSWCPollingNodeID  < g_apSWCObject[m_nSWCPollingTableID]->m_nTotolNodeNumber)
      {
        unTableLen = g_apSWCObject[m_nSWCPollingTableID]->ReadSWCOneTable(  //040323 Yu Wei
        &m_acWriteSWCPollingTable[7], (unsigned short)(m_nMultiNodeSWCPollingNodeID+1), 0x00000000);
        if (unTableLen != 0)
        {
          *(unsigned short *)&m_acWriteSWCPollingTable[2] =  //Set start address. 040323 Yu Wei
              STR_ByteSwap16Bit((unsigned short)((m_nSWCPollingTableID <<8)|(unsigned short)m_nMultiNodeSWCPollingNodeID));
          *(unsigned short *)&m_acWriteSWCPollingTable[4] = STR_ByteSwap16Bit(unTableLen); //Set data length. 040323 Yu Wei

          m_acWriteSWCPollingTable[6] = unTableLen *2;  //Set data length in bytes. 040323 Yu Wei

          *(unsigned short *)&m_acWriteSWCPollingTable[(unsigned char)m_acWriteSWCPollingTable[6] + 7] =
              STR_ByteSwap16Bit(ModbusCRC((unsigned char *)m_acWriteSWCPollingTable,
                     (unsigned char)m_acWriteSWCPollingTable[6] + 7));  //Checksum.

          SendOrClose(nLANLinkID,m_acWriteSWCPollingTable,(unsigned char)m_acWriteSWCPollingTable[6] + 9);
          m_nRetry ++;
          m_bSendCmdToRtuFlag = false;
        }
      }

      return; //20080220 multinode
    }
    else
    {
      //Read SWC table for RS485 non multinodes //040323 Yu Wei
      unTableLen = g_apSWCObject[m_nSWCPollingTableID]->ReadSWCOneTable(
        &m_acWriteSWCPollingTable[7], m_nSWCPollingTableTypeID, 0);
      #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMMLINK)
      printf("[RMM] CRTULink::WriteSWCTable, read %s tableID %d, first 10 "
             "bytes:\n",
             g_apSWCObject[m_nSWCPollingTableID]->m_acSWCName,
             m_nSWCPollingTableTypeID);
      SYS_PrnDataBlock((const UINT8 *) m_acWriteSWCPollingTable,
                       (unTableLen *2), 10);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
      if (unTableLen != 0)  //read SWC table successfully. 040323 Yu Wei
      {
        //Set start address. 040323 Yu Wei
        *(unsigned short *)&m_acWriteSWCPollingTable[2] =
          STR_ByteSwap16Bit((unsigned short)(
            (m_nSWCPollingTableID <<8)|m_nSWCPollingTableTypeID));
        //Set data length. 040323 Yu Wei
        *(unsigned short *)&m_acWriteSWCPollingTable[4] =
          STR_ByteSwap16Bit(unTableLen);
        m_acWriteSWCPollingTable[6] = unTableLen *2;  //Set data length in bytes. 040323 Yu Wei
        *(unsigned short *)&m_acWriteSWCPollingTable[(unsigned char)m_acWriteSWCPollingTable[6] + 7] =
          STR_ByteSwap16Bit(ModbusCRC((unsigned char *)m_acWriteSWCPollingTable,
                 (unsigned char)m_acWriteSWCPollingTable[6] + 7));  //Checksum.
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK_WRITESWCTBL))
        printf("[RMM] CRTULink::WriteSWCTable, construct and tx %s table "
               "%d, %d bytes:\n",
               g_apSWCObject[m_nSWCPollingTableID]->m_acSWCName,
               m_nSWCPollingTableTypeID,
               ((unsigned char)m_acWriteSWCPollingTable[6] + 9));
        SYS_PrnDataBlock((const UINT8 *) m_acWriteSWCPollingTable,
                         (unsigned char)m_acWriteSWCPollingTable[6] + 9, 10);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK_WRITESWCTBL))
        // transmit the table to the other RTU
        SendOrClose(nLANLinkID, m_acWriteSWCPollingTable,
                    (unsigned char)m_acWriteSWCPollingTable[6] + 9);
        m_nRetry++;
        m_bSendCmdToRtuFlag = false;    //040426 Yu Wei
      }
      else
      {
        GetNextSWCTableID();  //040322 Yu Wei
      }
    }
  }
  else if((m_nRetry < m_anRetry[nLANLinkID]) && (m_nRetry != 0))  //040426 Yu Wei
  {
    SendOrClose(nLANLinkID,m_acWriteSWCPollingTable,
                (unsigned char)m_acWriteSWCPollingTable[6] + 9); //lenth < 0
    m_nRetry ++;
    m_bSendCmdToRtuFlag = false;    //040426 Yu Wei

  }
  else if(m_nRetry >= m_anRetry[nLANLinkID])
  {
    m_nRetry = 0;
    m_nPollingCommand = POLLING_READ_TABLE;
    m_bSendCmdToRtuFlag = true;    //040426 Yu Wei
  }
}// WriteSWCTable

/*******************************************************************************
Purpose:
  Change to next SWC table by following the sequence of Fast polling, slow
  polling then timer polling. At the end of timer polling, check for next
  enabled SWC table ID.

Input:

Return:

History

  Name          Date          Remark
  ----          ----          ------
 Bryan Chong  14-May-2010   Update to check for next enabled SWC
 Bryan Chong  19-Aug-2010   Update to fix missing PLC table update for SWC 1
*******************************************************************************/
void CRTULink::GetNextSWCTableID(void)
{
  UINT8 cnt;

  switch(m_nSWCPollingTableTypeID)
  {
    case FAST_TABLE:
      m_nSWCPollingTableTypeID = SLOW_TABLE;
      break;

    case SLOW_TABLE:
      m_nSWCPollingTableTypeID = TIMER_TABLE;
      break;

    case TIMER_TABLE:
      m_nSWCPollingTableTypeID = FAST_TABLE;

      m_nSWCPollingTableID++;

      for(cnt = m_nSWCPollingTableID; cnt < (g_tRTUConfig.nSWCNumber - 1); cnt++)
      {
        if(g_tRTUConfig.nSwcEnabled[cnt] == TYP_NULL)
          continue;
        else
          break;
      }

      m_nSWCPollingTableID = cnt;

      if(m_nSWCPollingTableID >= (g_tRTUConfig.nSWCNumber - 1))
      {
        m_nSWCPollingTableID = g_tRTUConfig.nFirstSwcEnabled;

        if( (g_pRTUStatusTable->m_bSWCSwitchCompletedFlag == true) && //Completed send SWC table to standby RTU.
          (g_pRTUStatusTable->m_bStartSendSWCTableFlag == false))
        {
          g_pRTUStatusTable->m_bSendSWCTableCompletedFlag = true;
          m_nPollingCommand = POLLING_READ_TABLE;
        }
      }

      if (g_pRTUStatusTable->m_bSWCSwitchCompletedFlag == false)  //Not for STATEFLAG_SWITCHING_PTOS.
      {
        m_nRetry = 0;   //040426 Yu Wei
        m_nPollingCommand = POLLING_READ_TABLE;
      }
      break;

    default:
      m_nSWCPollingTableTypeID = FAST_TABLE;
      break;
  } // switch(m_nSWCPollingTableTypeID)
} // GetNextSWCTableID


/*******************************************************************************
Purpose:
  Check other RTU response when primary RTU sent a command to it.

Input:
  nLANLinkID  -- LAN link ID. (0 -- LAN2, 1 -- LAN1)

Return:
  OK    --  The response OK.
  ERROR  --  The response error or no response.

History
    Name           Date          Remark
    ----           ----          ------
  Yu, Wei        12-Aug-2004   Initial revision
  Chong, Bryan   07-May-2010   Replace Recv with SER_ReceiveMsg to fix
                               inconsistency in receiving LAN link data

*******************************************************************************/
int CRTULink::GetResponseFromOtherRTU(int nLANLinkID)
{
  int nRWTableResult = ERROR;    //Read/Write table result (ERROR/OK).
  int nRecvDataSize;
  int nReplyCheckResult = 0;
  char acTempLog[512];

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_GETRESPONSE)
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_GETRESPONSE)

 // printf("g_pRTUStatusTable->m_nSwitchFlag= %d\n", g_pRTUStatusTable->m_nSwitchFlag);
  // 20150402 Add in to check the flag status
  E_ERR_T rstatus = E_ERR_Success;
  #ifdef CFG_PRN_ERR
  CHAR acErrMsg[100] = {0};
  #endif // CFG_PRN_ERR

  if((m_anRTULinkFD[nLANLinkID] == TYP_NULL) ||
     (m_anRTULinkFD[nLANLinkID] == TYP_ERROR))
  {
    #ifdef _CFG_PRN_ERR
    printf("ERR  [RMM] CRTULink::GetResponseFromOtherRTU, invalid file "
           "descriptor, m_anRTULinkFD[%d] = %d\n",
           nLANLinkID, m_anRTULinkFD[nLANLinkID]);
    #endif // CFG_PRN_ERR
	  if( m_PrimarycloseFdFlag == true)// Yang Tao 20150402
	  {

		  return OK; //
	  }
	  else
	  {
		 return TYP_ERROR;
	  }
 }
  memset(m_acRecvOtherRTUBuffer, 0, sizeof(m_acRecvOtherRTUBuffer));
  nRecvDataSize = 0;

  rstatus = LNC_ReceiveMsg(m_anRTULinkFD[nLANLinkID], m_acRecvOtherRTUBuffer,
                           &nRecvDataSize, MODBUS_MAX_CMD_SIZE,
                           SOCKET_CONNECT_TIMEOUT/2);

//  printf("[RMM] CRTULink::GetResponseFromOtherRTU, rx msg fail at fd "
//           "%d,\n"
//           " %d\n",
//           m_anRTULinkFD[nLANLinkID], m_acRecvOtherRTUBuffer[13]);

//201406 Tao: Timeout use to be : SOCKET_CONNECT_TIMEOUT/2. Increase time out to reduce the error warning
  #ifdef NOT_USED
  if(rstatus != E_ERR_Success)
  {
    #ifdef CFG_PRN_ERR
    ERR_GetMsgString(rstatus, acErrMsg);
    printf("ERR  [RMM] CRTULink::GetResponseFromOtherRTU, rx msg fail at fd "
           "%d,\n"
           "  %s\n",
           m_anRTULinkFD[nLANLinkID], acErrMsg);
    #endif // CFG_PRN_ERR
    return TYP_ERROR;
  }
  #endif // NOT_USED

  switch(rstatus)
  {
    case E_ERR_Success:
      break;

    case E_ERR_LNC_SelectReadTimeOut:
      #ifdef CFG_PRN_ERR
      printf("ERR  [RMM] CRTULink::GetResponseFromOtherRTU, rx msg fail at fd "
             "%d,\n"
             "  error E_ERR_LNC_SelectReadTimeOut\n",
             m_anRTULinkFD[nLANLinkID]);
      #endif // CFG_PRN_ERR
      //LNC_GetConnectFileDescriptor(const CHAR * pipaddr,const UINT16 port_num,const INT32 sock_type,const UINT32 timeout_ms,INT32 * poutnfd)
      return TYP_ERROR;

    default:
      #ifdef CFG_PRN_ERR
      ERR_GetMsgString(rstatus, acErrMsg);
      printf("ERR  [RMM] CRTULink::GetResponseFromOtherRTU, rx msg fail at fd "
             "%d,\n"
             "  unhandled error %s\n",
             m_anRTULinkFD[nLANLinkID], acErrMsg);
      #endif // CFG_PRN_ERR
      return TYP_ERROR;
  } // switch(rstatus)

  if(nRecvDataSize > 0)
  {
    m_bSendCmdToRtuFlag = true;    //040426 Yu Wei
    switch(m_nPollingCommand)    //040308 Yu Wei
    {
      case POLLING_READ_TABLE:
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_GETRESPONSE)
        rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                             E_SYS_TIME_Local);
        printf("[RMM] %d.%02d.%02d %02d:%02d:%02d.%03d "
               "CRTULink::GetResponseFromOtherRTU, fd %d rx msg from "
               "RTU, POLLING_READ_TABLE\n",
               (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
               pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
               pbdtime->tm_sec, tspec.tv_nsec/1000000,
               m_anRTULinkFD[nLANLinkID], g_tRTUStatus.nRTUStateFlag);
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_GETRESPONSE)
        nReplyCheckResult = ModbusReplyCheck(
          (unsigned char *)m_acRecvOtherRTUBuffer, nRecvDataSize,
          (unsigned char *)m_acReadRTUStatus);
        break;

      case POLLING_WRITE_TABLE:
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK_GETRESPONSE))
        printf("[RMM] CRTULink::GetResponseFromOtherRTU, rx msg from RTU, "
               "POLLING_WRITE_TABLE\n");
        #endif // ((define CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
        nReplyCheckResult = ModbusReplyCheck(
          (unsigned char *)m_acRecvOtherRTUBuffer, nRecvDataSize,
          (unsigned char *)m_acWriteRTUStatus);
        break;

      case POLLING_WRITE_SWC_TABLE:
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK_GETRESPONSE))
        printf("[RMM] CRTULink::GetResponseFromOtherRTU, rx msg from RTU, "
               "POLLING_WRITE_SWC_TABLE\n");
        #endif // ((define CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
        nReplyCheckResult = ModbusReplyCheck(
          (unsigned char *)m_acRecvOtherRTUBuffer, nRecvDataSize,
          (unsigned char *)m_acWriteSWCPollingTable);
        break;

      case WRITE_COMMAND:
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
        printf("[RMM] CRTULink::GetResponseFromOtherRTU, fd %d rx msg from RTU, "
               "WRITE_COMMAND\n", m_anRTULinkFD[nLANLinkID]);
        #endif // ((define CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
        nReplyCheckResult = ModbusReplyCheck(
          (unsigned char *)m_acRecvOtherRTUBuffer, nRecvDataSize,
          (unsigned char *)m_acWriteCommand);
        break;

      default:
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
        printf("[RMM] CRTULink::GetResponseFromOtherRTU, rx unhandled msg from "
               "RTU, %d\n", m_nPollingCommand);
        #endif // ((define CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
        nReplyCheckResult = MODBUS_ERROR_CRC;
        break;
    }// switch(m_nPollingCommand)

    if( nReplyCheckResult == MODBUS_MESSAGE_OK )
    {
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK_GETRESPONSE))
      printf("[RMM] CRTULink::GetResponseFromOtherRTU, fd %d rx msg %d bytes, "
             "modbus OK\n", m_anRTULinkFD[nLANLinkID], nRecvDataSize);
      SYS_PrnDataBlock((const UINT8 *)m_acRecvOtherRTUBuffer, nRecvDataSize,
                       20);
      #endif // ((define CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
      nRWTableResult = NormalProcess(nLANLinkID, nRecvDataSize);    //040310 Yu Wei
    }else if( nReplyCheckResult == MODBUS_ERROR_CRC ){
      #if  ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMLINK)
      printf("WARN [RMM] CRTULink::GetResponseFromOtherRTU, fd %d rx msg "
             "modbus exception, %d bytes:\n",
             m_anRTULinkFD[nLANLinkID], nRecvDataSize);
      SYS_PrnDataBlock((const UINT8 *)m_acRecvOtherRTUBuffer, nRecvDataSize,
                       10);
      #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMLINK)

      sprintf(acTempLog, "GetResponseFromOtherRTU MODBUS_ERROR_EXCEPTION "
              "(%d,%d)= ", nLANLinkID, nRecvDataSize);
      g_pEventLog->LogMessage(acTempLog);

      ExceptionProcess();    //040310 Yu Wei
      //If receive exception code, the link is OK. 040308 Yu Wei
      nRWTableResult = OK;
    }else{

      sprintf(acTempLog, "GetResponseFromOtherRTU Receive(%d,%d) ERROR= %X",
              nLANLinkID, nRecvDataSize, nReplyCheckResult);
      g_pEventLog->LogMessage(acTempLog);
      #ifdef CFG_PRN_WARN
      printf("WARN [RMM] CRTULink::GetResponseFromOtherRTU, msg modbus "
             "unhandled error 0x%04x\n", nReplyCheckResult);
      SYS_PrnDataBlock((const UINT8 *)m_acRecvOtherRTUBuffer, nRecvDataSize,
                       10);
      #endif // ((define CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
    }

    if((g_pRTUStatusTable->m_bSWCSwitchCompletedFlag == true) &&
       (g_pRTUStatusTable->m_bStartSendSWCTableFlag == true))
    {
      g_pRTUStatusTable->m_bStartSendSWCTableFlag = false;
      //Set to first SWC. 040323 Yu Wei
      m_nSWCPollingTableID = 0;
      //Set to first table. 040323 Yu Wei
      m_nSWCPollingTableTypeID = FAST_TABLE;
      //Set send SWC table. 040323 Yu Wei
      m_nPollingCommand = POLLING_WRITE_SWC_TABLE;
    }
  }
  else if(nRecvDataSize == 0)
  {
    #ifdef CFG_PRN_WARN
    printf("WARN [RMM] CRTULink::GetResponseFromOtherRTU, fd %d rx msg size "
           "0. Close fd\n", m_anRTULinkFD[nLANLinkID]);
    #endif // ((define CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
    CloseConnectOtherRTU(nLANLinkID);
  }
  else  //040426 Yu Wei
  {
    #ifdef CFG_PRN_WARN
    printf("WARN [RMM] CRTULink::GetResponseFromOtherRTU, rx msg size "
           "negative\n");
    #endif // ((define CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
    if(CheckTimeOut(m_anPollingTimeoutType[nLANLinkID],
                    m_anFailureTimeout[nLANLinkID])== true)
    {
      //reply timeout, will re-send.
      m_bSendCmdToRtuFlag = true;
    }
  }

  return nRWTableResult;
} // GetResponseFromOtherRTU

/*******************************************************************************
Purpose:
  Manage the communication with other RTU

Input:
  nLANLinkID  --  LAN link ID. (0 -- LAN2, 1 -- LAN1)
  nRecvDataSize  --  Receive data length. (in)

Return:
  OK    --  The response OK.
  ERROR  --  The response error or no response.

History
    Name           Date          Remark
    ----           ----          ------
  Bryan Chong   02-Jun-2010   Update to other RTU state from
                              g_pRTUStatusTable->m_aunOtherRTUStatus to ensure
                              the later RTU boot up will enter to Primary role
  Bryan Chong   24-Aug-2010   Disable g_pRTUStatusTable->m_bSWCLinkDownFlag flag
                              to fix fail polling issue [PR47]

*******************************************************************************/
int CRTULink::NormalProcess(int nLANLinkID, int nRecvDataSize)
{
  int nRWTableResult = ERROR;
  //RMM_RTU_STATUS_TBL statusTbl;
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
  E_ERR_T rstatus = E_ERR_Success;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
  struct timespec   tspec;
  struct tm         *pbdtime;
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)

  switch(m_nPollingCommand)
  {
    case POLLING_READ_TABLE:
      #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMMLINKCHK)
      printf("[RMM] CRTULink::NormalProcess, rx cmd POLLING_READ_TABLE\n");
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINKCHK))
      if(nRecvDataSize == (m_nTableBytes + 5))
      {
        nRWTableResult = OK;
        m_nRetry = 0;
        m_nPollingCommand = POLLING_WRITE_TABLE;

        //Copy other RTU status table to buffer.
        g_pRTUStatusTable->WriteOtherRTUStatus(
                             (UINT16 *)&m_acRecvOtherRTUBuffer[3]);


        if(g_pRTUStatusTable->m_bWaitLinkCheckCompletedFlag == true)
        {
          #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMMLINKCHK)
          rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                                 E_SYS_TIME_Local);
          printf("[RMM] %d.%02d.%02d %02d:%02d:%02d.%03d "
            "CRTULink::NormalProcess, %s rx cmd POLLING_READ_TABLE, "
            " rx stdby RTU status %d bytes:\n",
            (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
            pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
            pbdtime->tm_sec, tspec.tv_nsec/1000000,
            SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
            nRecvDataSize);
          SYS_PrnDataBlock((const UINT8 *) m_acRecvOtherRTUBuffer,
                         nRecvDataSize, 20);
          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINKCHK))

          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
          rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                              E_SYS_TIME_Local);
          printf("[RMM] %d.%02d.%02d %02d:%02d:%02d.%03d "
                 "CRTULink::NormalProcess, \n"
                 "  rx status link check flag = 0x%04x\n",
                 (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
                 pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
                 pbdtime->tm_sec, tspec.tv_nsec/1000000,
                 (UINT16)m_acRecvOtherRTUBuffer[3 +
                     RTU_STATUS_TABLE_SWC_LINK_CHECK*2]);
          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
        }

        //20120321 BC
        if(g_pRTUStatusTable->m_aunOtherRTUStatus[
             RTU_STATUS_TABLE_SWC_LINK_CHECK] != 0)
          m_nPollingCommand = POLLING_READ_TABLE;



        //20100711 BC
        if((g_pRTUStatusTable->m_bSWCLinkDownFlag == true) &&
           (g_pRTUStatusTable->
              m_aunOtherRTUStatus[RTU_STATUS_TABLE_SWC_LINK_CHECK] == 0))
        {
          //Standby RTU completed link checking. 040311 Yu Wei
          if((g_pRTUStatusTable->m_bSWCLinkDownProcessFlag == true) &&
             (g_pRTUStatusTable->m_bWaitLinkCheckCompletedFlag == true))
          {
            #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
            rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                                 E_SYS_TIME_Local);
            printf("[RMM] %d.%02d.%02d %02d:%02d:%02d.%03d "
                   "CRTULink::NormalProcess, pri RTU found stdby RTU link "
                   "check completed.\n"
                   "  Notation for local %d, other %d. SetSWCLinkStart.\n"
                   "  Reset m_bWaitLinkCheckCompletedFlag, "
                   "m_bSWCLinkCheckTotalFlag\n"
                   "  Pri link check stop...\n",
                   (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
                   pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
                   pbdtime->tm_sec, tspec.tv_nsec/1000000,
                   g_pRTUStatusTable->m_unSWCWeight,
                   g_pRTUStatusTable->m_unOtherSWCWeight);
            //SYS_PrnDataBlock(
              //(const UINT8 *) g_pRTUStatusTable->m_aunOtherRTUStatus,
              //sizeof(RMM_RTU_STATUS_TBL), 20);
            #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
            // Resume local SWC operations
            g_pRTUStatusTable->SetSWCLinkStart();
            g_pRTUStatusTable->m_bWaitLinkCheckCompletedFlag = false;
            g_pRTUStatusTable->m_bSWCLinkCheckTotalFlag = E_TYPE_No;

            // 20100824 BC (Rqd ZSL)
            g_pRTUStatusTable->m_bSWCLinkDownFlag = false;

            clock_gettime(CLOCK_REALTIME, &g_pRTUStatusTable->m_linkChkStop);
          }
        }
      }// if(nRecvDataSize == (m_nTableBytes + 5))
      break;

    case POLLING_WRITE_TABLE:
      if(nRecvDataSize == MODBUS_WRITE_REPLY_SIZE)
      {
        //*(unsigned short *)&m_acWriteRTUStatus[
          //7 + RTU_STATUS_TABLE_SWC_LINK_CHECK*2] = 0x0000;

        //040311 Yu Wei
        if ( g_pRTUStatusTable->m_bSWCLinkDownFlag == true )
        {
          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
          rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                               E_SYS_TIME_Local);
          printf("[RMM] %d.%02d.%02d %02d:%02d:%02d.%03d "
                 "CRTULink::NormalProcess, %s\n"
                 "  POLLING_WRITE_TABLE. Reset m_bSWCLinkDownFlag, set "
                 "m_bWaitLinkCheckCompletedFlag\n",
                 (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
                 pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
                 pbdtime->tm_sec, tspec.tv_nsec/1000000,
                 SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK_RXCMD))
          //040225 Yu Wei
          g_pRTUStatusTable->m_bSWCLinkDownFlag = false;
          //040311 Yu Wei
          g_pRTUStatusTable->m_bWaitLinkCheckCompletedFlag = true;
        }

        nRWTableResult = OK;
        m_nRetry = 0;
        m_nSWCPollingTableTypeID = FAST_TABLE;
        m_nMultiNodeSWCPollingNodeID = 0;//2008
        m_nPollingCommand = POLLING_WRITE_SWC_TABLE;
        ActivateTimeOut(m_anPollingTimeoutType[0]);
        ActivateTimeOut(m_anPollingTimeoutType[1]);
      }
      break;

    case POLLING_WRITE_SWC_TABLE:
      if(nRecvDataSize == MODBUS_WRITE_REPLY_SIZE)
      {
        nRWTableResult = OK;
        m_nRetry = 0;

        #if ((defined CFG_DEBUG_MSG) && \
             (CFG_DEBUG_RMMLINK || _CFG_DEBUG_RMMLINKCHK))
        printf("[RMM] CRTULink::NormalProcess, POLLING_WRITE_SWC_TABLE. "
               "Get next swc table ID\n");
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK)
        GetNextSWCTableID();  //040322 Yu Wei
      }
      break;

    case WRITE_COMMAND:
      if(*(unsigned short *)&m_acRecvOtherRTUBuffer[2] == WRITE_COMMAND_ADDRESS)
      {
        nRWTableResult = OK;
        //040426 Yu Wei
        m_nRetry = 0;
        m_nPollingCommand = POLLING_READ_TABLE;

        /*(01 10 ff ff 00 01 01 ed)*/
        /*when user force the current Primary RTU changing to Standby , we
          should send status to another RTU*/
        m_nWriteCommandResult = SEND_COMMPLETE_OK;
        DeleteWriteCommand();

        //Other RTU received change state command. 040420 Yu Wei
        if(g_pRTUStatusTable->m_nSwitchFlag == COMFIRM_SWITCH)
          g_pRTUStatusTable->m_bOtherRTUSwitchFlag = true;
      }
      break;

    default:
      m_nRetry = 0;    //040426 Yu Wei
      m_nPollingCommand = POLLING_READ_TABLE;
      printf("Receive COMMAND error(%d)\n",nLANLinkID);
      break;
  }// switch(m_nPollingCommand

  return nRWTableResult;
}// NormalProcess
/*******************************************************************************
Purpose:
  Handling of exception response from standby RTU.

*******************************************************************************/
void CRTULink::ExceptionProcess(void)
{
  m_nRetry = 0;


  switch(m_nPollingCommand)
  {
  case POLLING_READ_TABLE:
    //m_nPollingCommand = POLLING_WRITE_TABLE;  //040426 Yu Wei
    g_pEventLog->LogMessage((CHAR *)"POLLING_READ_TABLE Read standby RTU status table exception.\n");
    break;

  case POLLING_WRITE_TABLE:
    //m_nSWCPollingTableTypeID = FAST_TABLE;    //040426 Yu Wei
    //m_nPollingCommand = POLLING_WRITE_SWC_TABLE;  //040426 Yu Wei
    g_pEventLog->LogMessage((CHAR *)"POLLING_WRITE_TABLE Write RTU status table to standby RTU exception.\n");
    break;

  case POLLING_WRITE_SWC_TABLE:
    //GetNextSWCTableID();  //040323 Yu Wei    //040426 Yu Wei
    g_pEventLog->LogMessage((CHAR *)"POLLING_WRITE_SWC_TABLE Write SWC tables to standby RTU exception.\n");
    break;

  case WRITE_COMMAND:
    //m_nPollingCommand = POLLING_READ_TABLE;  //040426 Yu Wei
    g_pEventLog->LogMessage((CHAR *)"WRITE_COMMAND Write command to standby RTU exception.\n");
    break;

  default:
    m_nPollingCommand = POLLING_READ_TABLE;
    g_pEventLog->LogMessage((CHAR *)"Command between RTU Error.\n");
    break;
  }


}




/*------------------------------------------------------------------------------
Purpose:
  Implement RTU communication as slave.

History

    Name         Date        Remark

 Bryan Chong  31-May-2011  Combined methods, LinkSlave1 and LinkSlave2, to
                           LinkSlave [C955 PR108]
 Bryan Chong  01-Jun-2012  Combined else if (nRecvNumber == 0) and else case to
                           become one else case and update
                           g_tRTUStatus.abOtherLinkStatus[nLANLinkID] = false

------------------------------------------------------------------------------*/
void CRTULink::LinkSlave(UINT8 lanlinkidx)
{
  int nLANLinkID;
  int nReceiveResult;
  int nRecvNumber = 0;
  //int nCmdImplementResult;
  char acTempLog[512];
  char cLog[512];
  //int nSWCIndex, nSWCTableID;
  char TmpBuffer[MODBUS_MAX_CMD_SIZE];
  int i;
  int rmodval = 0;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_STATUS)
  E_ERR_T rstatus = E_ERR_Success;
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_STATUS)


  nLANLinkID = lanlinkidx;
  nReceiveResult = ERROR;
  nRecvNumber = g_apOtherRTUServerSocket[nLANLinkID]->Recv(
                  TmpBuffer, MODBUS_MAX_CMD_SIZE, m_anLinkDownTimeout[nLANLinkID], 0);

  if(nRecvNumber > 0)
  {
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK_LINKSLAVE))
    printf("[RMM] CRTULink::LinkSlave1, fd %d rx %d bytes from other RTU:\n",
           g_apOtherRTUServerSocket[nLANLinkID]->m_nSockListen, nRecvNumber);
    SYS_PrnDataBlock((const UINT8 *) TmpBuffer, nRecvNumber, 10);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))

    //If receive read command.
    pthread_mutex_lock(&Recv_mutex);
    memcpy(m_acRecvOtherRTUBuffer , TmpBuffer , nRecvNumber);
    rmodval = ModbusCMDCheck((unsigned char *)m_acRecvOtherRTUBuffer,
                               nRecvNumber, m_acReadRTUStatus[0]);// 20140604 Yang Tao change
    // to ModbusCMDCheck_LANSWC
    switch(rmodval)
    {
      case MODBUS_MESSAGE_OK:
        switch(m_acRecvOtherRTUBuffer[1])
        {

          case E_MB_FC_Read_HoldingRegister:
          case E_MB_FC_Read_N_Words:
            m_acReplyRTUStatus[0] = m_acRecvOtherRTUBuffer[0];
            m_acReplyRTUStatus[1] = m_acRecvOtherRTUBuffer[1];
            m_acReplyRTUStatus[2] = m_acRecvOtherRTUBuffer[5]*2;

            // Get Local RTU status table.
            g_pRTUStatusTable->ReadRTUStatus(
              (unsigned short *)&m_acReplyRTUStatus[3],0xFFFFFFFF);


            *(unsigned short *)&m_acReplyRTUStatus[m_nTableBytes + 3] =
               STR_ByteSwap16Bit(ModbusCRC((unsigned char *)m_acReplyRTUStatus,
                                    m_nTableBytes + 3));  //Checksum.

            g_apOtherRTUServerSocket[nLANLinkID]->Send(
              m_acReplyRTUStatus, m_nTableBytes + 5, 0);

            #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMMLINKCHK)
            rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                                 E_SYS_TIME_Local);
            printf("[RMM] %d.%02d.%02d %02d:%02d:%02d.%03d "
                   "CRTULink::LinkSlave, tx RTU Status table %d bytes\n",
                   (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
                   pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
                   pbdtime->tm_sec, tspec.tv_nsec/1000000, m_nTableBytes + 5);
            SYS_PrnDataBlock((const UINT8 *) m_acReplyRTUStatus,
                             m_nTableBytes + 5, 20);
            #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))

            break;

          case E_MB_FC_Write_N_Words:
            //Received a write command.
            #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK_LINKSLAVE))
            printf("[RMM] CRTULink::LinkSlave1, rx write cmd, starts write cmd "
                   "process\n");
            #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
            WriteCommandProcess(nLANLinkID);
            break;

          default:
            //Reply exception 1. Function code error.
            //Standby RTU only accept read/write word command.
            m_acRecvOtherRTUBuffer[1] |= 0x80;
            m_acRecvOtherRTUBuffer[2] = 0x01;
            *(unsigned short *)&m_acRecvOtherRTUBuffer[3] =
              STR_ByteSwap16Bit(ModbusCRC((unsigned char *)m_acRecvOtherRTUBuffer,3));
            g_apOtherRTUServerSocket[nLANLinkID]->
              Send(m_acRecvOtherRTUBuffer,5,0);
            sprintf(acTempLog, "(LinkSlave1 Reply exception 0. Function code "
                    "error)Send default(%d,5)= ", nLANLinkID);
            #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
            printf("[RMM] CRTULink::LinkSlave1, rx error cmd, tx "
                   "exception msg\n");
            SYS_PrnDataBlock((const UINT8 *) m_acRecvOtherRTUBuffer, 5, 10);
            #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
            break;
        }// switch(m_acRecvOtherRTUBuffer[1])

        nReceiveResult = OK; //Receive a valid command.
        break;

      case MODBUS_ERROR_EXCEPTION1:
        //Reply exception 1.
        m_acRecvOtherRTUBuffer[1] |= 0x80;
        m_acRecvOtherRTUBuffer[2] = 0x01;
        *(unsigned short *)&m_acRecvOtherRTUBuffer[3] =
         STR_ByteSwap16Bit(ModbusCRC((unsigned char *)m_acRecvOtherRTUBuffer,3));
        g_apOtherRTUServerSocket[nLANLinkID]->Send(m_acRecvOtherRTUBuffer,5,0);
        sprintf(acTempLog, "(LinkSlave1 Read)Send MODBUS_ERROR_EXCEPTION1(%d,5)= ",nLANLinkID);
          g_pDebugLog->LogMessage(acTempLog);
        nReceiveResult = OK; //Receive a valid command.
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK)
        printf("[RMM] CRTULink::LinkSlave1, tx exception code 1, 5 bytes:\n");
        SYS_PrnDataBlock((const UINT8 *) m_acRecvOtherRTUBuffer, 5, 10);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
        break;

      case MODBUS_ERROR_EXCEPTION3:
        //Reply exception 3.
        m_acRecvOtherRTUBuffer[1] |= 0x80;
        m_acRecvOtherRTUBuffer[2] = 0x03;
        *(unsigned short *)&m_acRecvOtherRTUBuffer[3] =
          STR_ByteSwap16Bit(ModbusCRC((unsigned char *)m_acRecvOtherRTUBuffer,3));
        g_apOtherRTUServerSocket[nLANLinkID]->Send(m_acRecvOtherRTUBuffer,5,0);
        sprintf(acTempLog, "(LinkSlave1 Read)Send MODBUS_ERROR_EXCEPTION3 (%d,5)= ",nLANLinkID);
        g_pDebugLog->LogMessage(acTempLog);
        nReceiveResult = OK; //Receive a valid command.

        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK)
        printf("[RMM] CRTULink::LinkSlave1, tx exception code 3, 5 bytes:\n");
        SYS_PrnDataBlock((const UINT8 *) m_acRecvOtherRTUBuffer, 5, 10);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
        break;

      default:
        nReceiveResult = ERROR;

        sprintf(acTempLog,"R(ERROR)(%d)=", nRecvNumber);
        g_pEventLog->LogMessage(acTempLog);


        for(i=0; i<nRecvNumber; i++)
        {
          sprintf(&acTempLog[i*3],"%02X ",(unsigned char)m_acRecvOtherRTUBuffer[i]);
        }
        g_pEventLog->LogMessage(acTempLog);
        g_pEventLog->LogMessage((CHAR *)"Standby LinkSlave1 RTU received error "
                                  "message\n");
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
        printf("[RMM] CRTULink::LinkSlave1, rx unhandled modbus error "
               "cmd\n");
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
        break;

    } // switch(rmodval)
    pthread_mutex_unlock(&Recv_mutex);
  } else
  {
	  //20150715 Su, 2-4 link slave is only for standby FEP.
	  if(m_nCurrentStateFlag == STATEFLAG_STANDBY)
	  {
		    if(CheckTimeOut(m_anFailureTimeoutType[nLANLinkID],
		       m_anSlaveLinkDownMonitorTimeout[nLANLinkID])== true)	//20150715 Su, 2-4 longer timeout
		    {
		    	ActivateTimeOut(m_anFailureTimeoutType[nLANLinkID]);
				nReceiveResult =  ERROR;
				//20140604 Yang Tao: Need to comment off this to solve the RTU LAN1 & 2 interlink toggle issue.
				// Need to check the timeout first before update the LAN 1 & 2 in status table.
				g_tRTUStatus.abOtherLinkStatus[nLANLinkID] = false;	//20151127 Su
				#if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_STATUS)
				rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
													 E_SYS_TIME_Local);
				printf("[RMM] %d.%02d.%02d %02d:%02d:%02d.%03d "
					   "CRTULink::LinkSlave, linkID %d receive %d byte\n",
					   (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
					   pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
					   pbdtime->tm_sec, tspec.tv_nsec/1000000,
					   nLANLinkID, nReceiveResult);
				#endif // ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_RMMLINK))
		    }
	  }
  }
  if(nReceiveResult == ERROR && m_nCurrentStateFlag == STATEFLAG_STANDBY)
  {
    if(CheckTimeOut(m_anFailureTimeoutType[nLANLinkID],
       m_anLinkDownTimeout[nLANLinkID])== true)
    {
      ActivateTimeOut(m_anFailureTimeoutType[nLANLinkID]);
      if( g_tRTUStatus.abOtherLinkStatus[nLANLinkID] == true )
      {
//        sprintf(cLog,  "[RMM] (7),g_tRTUStatus.abOtherLinkStatus[%d] = false;\n",nLANLinkID);
//       			g_pEventLog->LogMessage(cLog);
        g_tRTUStatus.abOtherLinkStatus[nLANLinkID] = false;
      }

      //Timeout clear flag. 040419 Yu Wei
      g_pRTUStatusTable->m_bOtherRTUSwitchFlag = false;

      //Two link fault //040420 Yu Wei
      //During switching //040420 Yu Wei
      if((g_tRTUStatus.abOtherLinkStatus[nLANLinkID^1] == false) &&
         (g_pRTUStatusTable->m_nSwitchFlag == START_SWITCH))
      {
        //Didn't receive confirm command //040420 Yu Wei
        g_pRTUStatusTable->m_nSwitchFlag = COMFIRM_SWITCH_ERROR;
      }
    }
  }else{
    //Reset one link timer only. //040426 Yu wei
    ActivateTimeOut(m_anFailureTimeoutType[nLANLinkID]);
    if(g_tRTUStatus.abOtherLinkStatus[nLANLinkID] == false)
    {
//        sprintf(cLog,  "[RMM] (9),g_tRTUStatus.abOtherLinkStatus[%d] = false;\n",nLANLinkID);
//     			g_pEventLog->LogMessage(cLog);
      g_tRTUStatus.abOtherLinkStatus[nLANLinkID] = true;
      //g_tRTUStatus.abOtherLinkStatus[nLANLinkID^1] = false;
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_STATUS)
      rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                           E_SYS_TIME_Local);
      printf("[RMM] %d.%02d.%02d %02d:%02d:%02d.%03d CRTULink::LinkSlave,\n"
             "  timeout at %d ms, set abOtherLinkStatus[%d] = %d, [%d] = %d\n",
             (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
             pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
             pbdtime->tm_sec, tspec.tv_nsec/1000000,
             m_anLinkDownTimeout[nLANLinkID], nLANLinkID,
             g_tRTUStatus.abOtherLinkStatus[nLANLinkID],
             nLANLinkID^1, g_tRTUStatus.abOtherLinkStatus[nLANLinkID^1]);
      #endif // ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_RMMLINK))
    }
  }
}// LinkSlave
/*******************************************************************************
Purpose:
  To process primary RTU write command. To reduce LinkSlave()'s complexity added
  in 20 April 2004, Yu Wei

Input/Output
  nLANLinkID  -- LAN link ID (0 -- LAN2, 1 -- LAN1). (in)

History

  Name          Date         Remark
  ----          ----         ------
 Yu, Wei      23-Mar-2004  Initial revision

*******************************************************************************/
void CRTULink::WriteCommandProcess(int nLANLinkID)
{
  int nCmdImplementResult = OK;
  int nSWCIndex, nSWCTableID;
  char acTempLog[200];

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
  // RMM_RTU_STATUS_TBL statusTbl;
  E_ERR_T rstatus = E_ERR_Success;
  struct timespec   tspec;
  struct tm         *pbdtime;
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)

  switch( STR_ByteSwap16Bit(*(UINT16 *)&m_acRecvOtherRTUBuffer[2]))
  {
    //Change state command. 040323 Yu Wei
    case WRITE_COMMAND_ADDRESS:
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK_WRITECMD))
      printf("[RMM] CRTULink::WriteCommandProcess, WRITE_COMMAND_ADDRESS\n");
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
      switch(m_acRecvOtherRTUBuffer[7])
      {
         //Change state flag.  //040420 Yu Wei
         case CMD_START_SWITCH:
           #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
           rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                                E_SYS_TIME_Local);
           printf("[RMM] %d.%02d.%02d %02d:%02d:%02d.%03d "
                  "CRTULink::WriteCommandProcess, stdby RTU rx "
                  "CMD_START_SWITCH cmd %d bytes:\n",
                  (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
                  pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
                  pbdtime->tm_sec, tspec.tv_nsec/1000000,
                  (m_acRecvOtherRTUBuffer[6] + 9));
           SYS_PrnDataBlock((const UINT8 *) m_acRecvOtherRTUBuffer,
                            (m_acRecvOtherRTUBuffer[6] + 9), 20);
           printf("[RMM] CRTULink::WriteCommandProcess, perform RTU switching "
                  "Stdby To Pri\n");
           #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
           //Only accept change to primary command. //040326 Yu Wei
           ChangeStateTo(STATEFLAG_SWITCHING_STOP);
           //Set switching status.
           g_pRTUStatusTable->m_nSwitchFlag = START_SWITCH;
           break;

         case CMD_COMFIRM_SWITCH:  //040420 Yu Wei
           //switching has started.
           if(g_pRTUStatusTable->m_nSwitchFlag == START_SWITCH)
           {
             //Set switching status.
             g_pRTUStatusTable->m_nSwitchFlag = COMFIRM_SWITCH;
           }else{
             //Not in switching state.
             g_pRTUStatusTable->m_nSwitchFlag = NOT_IN_SWITCH;
           }

           //Receive confirm switch command, set other RTU state to standby.
           //040524 Yu Wei
           g_pRTUStatusTable->m_nOtherRTUStateFlag = STATEFLAG_STANDBY;

           break;

         default:
           nCmdImplementResult = ERROR;
           break;
      }
      break;

    //Write RTU status table. 040323 Yu Wei
    case WRITE_RTU_STATUS_ADDRESS:
      /*
        Write other RTU status table to local buffer, m_aunOtherRTUStatus.
        Standby RTU will conduct link check based on link check flag set
        by Primary RTU.
      */

      #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMMLINKCHK)
      memcpy(&statusTbl, &m_acRecvOtherRTUBuffer[7],
             sizeof(RMM_RTU_STATUS_TBL));
      if(statusTbl.isSwcLinkCheckEnable)
      {
        printf("[RMM] CRTULink::WriteCommandProcess, WRITE_RTU_STATUS_ADDRESS, "
               "link check flag is enable\n");
        SYS_PrnDataBlock((const UINT8 *) &statusTbl, sizeof(RMM_RTU_STATUS_TBL),
                         20);
      }
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)

      #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMMLINKCHK)
      printf("[RMM] CRTULink::WriteCommandProcess, WRITE_RTU_STATUS_ADDRESS, "
             "rx %d bytes status table:\n",
             (m_acRecvOtherRTUBuffer[6] + 9));
      SYS_PrnDataBlock((const UINT8 *) m_acRecvOtherRTUBuffer,
                       (m_acRecvOtherRTUBuffer[6] + 9), 20);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
      //Received status table.
      g_pRTUStatusTable->WriteOtherRTUStatus(
                           (UINT16 *)&m_acRecvOtherRTUBuffer[7]);
      //Other RTU switching completed. 040419 Yu Wei
      g_pRTUStatusTable->m_bOtherRTUSwitchFlag = false;

      #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMMLINKCHK)
      printf("[RMM] CRTULink::WriteCommandProcess, rx other RTU status table "
             "%d bytes:\n", sizeof(g_pRTUStatusTable->m_aunOtherRTUStatus));
      SYS_PrnDataBlock((const UINT8 *)g_pRTUStatusTable->m_aunOtherRTUStatus,
        sizeof(g_pRTUStatusTable->m_aunOtherRTUStatus), 10);
      printf("[RMM] CRTULink::WriteCommandProcess, other RTU Status table link "
             "check word = 0x%04x\n", RTU_STATUS_TABLE_SWC_LINK_CHECK,
             STR_ByteSwap16Bit(g_pRTUStatusTable->
               m_aunOtherRTUStatus[RTU_STATUS_TABLE_SWC_LINK_CHECK]));
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)

      if(STR_ByteSwap16Bit(g_pRTUStatusTable->
           m_aunOtherRTUStatus[RTU_STATUS_TABLE_SWC_LINK_CHECK]) != 0)
      {
        g_pRTUStatusTable->UpdateRTUStatus(1, RTU_STATUS_TABLE_SWC_LINK_CHECK);

        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
        rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                             E_SYS_TIME_Local);
        printf("[RMM] %d.%02d.%02d %02d:%02d:%02d.%03d "
               "CRTULink::WriteCommandProcess, %s\n"
               "  WRITE_RTU_STATUS_ADDRESS, pri RTU link down,\n"
               "  Stdby RTU link check start...\n",
               (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
               pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
               pbdtime->tm_sec, tspec.tv_nsec/1000000,
               SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
        printf("  local status tbl link chk = 0x%04x\n",
               (UINT16)g_pRTUStatusTable->
                 m_aunLocalRTUStatus[RTU_STATUS_TABLE_SWC_LINK_CHECK]);
        //SYS_PrnDataBlock((const UINT8 *) g_pRTUStatusTable->m_aunLocalRTUStatus,
          //RTU_STATUS_TABLE_LENGTH_MAX * 2, 20);
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
        clock_gettime(CLOCK_REALTIME, &g_pRTUStatusTable->m_linkChkStart);
        g_pRTUStatusTable->m_linkChkStop.tv_sec = 0;
        g_pRTUStatusTable->m_linkChkStop.tv_nsec = 0;

        //Primary RTU has a link down, standby RTU need check link.
        //Set SWC is checking link status, complete update RTU status.
        g_pRTUStatusTable->SetSWCLinkCheckIndexTable();
      }
      break; // WRITE_RTU_STATUS_ADDRESS

    //Write SWCtable. 040323 Yu Wei
    default:
      nSWCIndex = ((STR_ByteSwap16Bit(
        *(UINT16 *)&m_acRecvOtherRTUBuffer[2])) & 0xFF00) >> 8;
      nSWCTableID = (
        STR_ByteSwap16Bit(*(UINT16 *)&m_acRecvOtherRTUBuffer[2])) & 0x00FF;
      // 20100519 BC (Rqd ZSL)
      if(g_apSWCObject[nSWCIndex] == TYP_NULL)
         break;
      if(g_apSWCObject[nSWCIndex]->m_nLinkType == SWC_LINK_RS_485M)
      {
        UINT16 nNodeID = nSWCTableID;
        g_apSWCObject[nSWCIndex]->
          CopySWCTable((UINT16 *)&m_acRecvOtherRTUBuffer[7], nNodeID);
      }
      else
      {
        //040323 Yu Wei
        g_apSWCObject[nSWCIndex]->
          CopySWCTable((UINT16 *)&m_acRecvOtherRTUBuffer[7],nSWCTableID);
      }
      break;
  }

  if(nCmdImplementResult == OK)
  {
    //Reply write command OK. Checksum
    *(UINT16 *)&m_acRecvOtherRTUBuffer[6] =
      STR_ByteSwap16Bit(ModbusCRC((UINT8 *)m_acRecvOtherRTUBuffer,6));
    g_apOtherRTUServerSocket[nLANLinkID]->Send(m_acRecvOtherRTUBuffer, 8, 0);
  }
  else
  {
    //Reply exception 4
    sprintf(acTempLog,"Standby RTU receives error write command 10 %x\n",
            m_acRecvOtherRTUBuffer[7]);
    g_pEventLog->LogMessage(acTempLog);

    m_acRecvOtherRTUBuffer[1] |= 0x80;
    m_acRecvOtherRTUBuffer[2] = 0x04;
    *(UINT16 *)&m_acRecvOtherRTUBuffer[3] =
      STR_ByteSwap16Bit(ModbusCRC((UINT8 *)m_acRecvOtherRTUBuffer, 3));
    g_apOtherRTUServerSocket[nLANLinkID]->Send(m_acRecvOtherRTUBuffer, 5, 0);
    g_pEventLog->LogMessage((CHAR *)"(Slave Write)Send ERROR 5 bytes");
  }
}// WriteCommandProcess

/*******************************************************************************
Purpose:
  Check if the LAN is OK.

Input
  nLANLinkID  -- LAN link ID (0 -- LAN2, 1 -- LAN1). (in)

Return
  OK    -- Connection OK.
  ERROR  -- Connection error.
*******************************************************************************/
int CRTULink::ConnectOtherRTU(int nLANLinkID)
{
  int nResult = OK;    //Connection result (ERROR/OK).

  if(m_abConnectFlag[nLANLinkID] == false) //Check the link is connected.
  {
    nResult = ERROR;
  }
  return nResult;
}



/*******************************************************************************
Purpose:
  Connect other RTU via LAN2 (link 1).This is primary link between RTUs. If this
  link is on, the link 2 (LAN1) will be closed.
  Don't close standby link, LAN1 will work individually.  //040426 Yu Wei

History
    Name           Date          Remark
    ----           ----          ------
  Bryan Chong    03-Jun-2010   Update m_abConnectFlag, m_Lan2LinkDown,
                               m_bSendCmdToRtuFlag accordingly
  Bryan Chong    10-May-2012   Update Connect with LNC_GetConnectFileDescriptor
                               for consolidation
*******************************************************************************/
void CRTULink::ConnectLink0(void)
{
  E_ERR_T rstatus = E_ERR_Success;
  #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMLINK)
  CHAR errbuff[100] = {0};
  #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMLINK)

  if(m_abConnectFlag[0] == false)
  {
    //m_anRTULinkFD[0] = Connect(m_acIPAddress[0], m_anPort[0]);
    rstatus = LNC_GetConnectFileDescriptor((const CHAR *)m_acIPAddress[0],
                                           (const UINT16) m_anPort[0],
                                           SOCK_STREAM,
                                           m_anLinkDownTimeout[0]/2,
                                           &m_anRTULinkFD[0]);
    if(rstatus != E_ERR_Success)
    {
      #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMLINK)
      ERR_GetMsgString(rstatus, errbuff);
      printf("WARN [RMM] CRTULink::ConnectLink0, fail to get file descriptor\n"
             "  m_anRTULinkFD[0] %d, error %s\n",
             m_anRTULinkFD[0], errbuff);
      #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMLINK)
    }

    if(m_anRTULinkFD[0] == TYP_ERROR)
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK)
      printf("[RMM] CRTULink::ConnectLink0, close %s:%d LAN 2 connection, "
             "socket %d\n",
             m_acIPAddress[0], m_anPort[0], m_anRTULinkFD[0]);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
      CloseConnectOtherRTU(0);
      m_Lan2LinkDown = true;

    }else{
      //g_pDebugLog->LogMessage((CHAR *)"[RMM] CRTULink::ConnectLink0, RTU "
        //                              "(LAN2) connect OK\n");
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
      printf("[RMM] CRTULink::ConnectLink0, LAN 2, fd %d %s:%d connection OK\n",
             m_anRTULinkFD[0], m_acIPAddress[0], m_anPort[0]);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
      m_abConnectFlag[0] = true;
      m_Lan2LinkDown = false;
      m_bSendCmdToRtuFlag = true;
    }
  }
}

/*******************************************************************************
Purpose:
  Connect other RTU via LAN1 (link 2).This is standby link between RTUs. If
  primary link is down, this link will be connected.
  Don't consider primary link status, LAN1 will work individually.


History
    Name           Date          Remark
    ----           ----          ------
  Bryan Chong    03-Jun-2010   Update m_abConnectFlag, m_Lan2LinkDown,
                               m_bSendCmdToRtuFlag accordingly

*******************************************************************************/
void CRTULink::ConnectLink1(void)
{
  E_ERR_T rstatus = E_ERR_Success;
  #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMLINK)
  CHAR errbuff[100] = {0};
  #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMLINK)
  //if((m_abConnectFlag[1] == false) || (m_abConnectFlag[1] == TYP_ERROR))
  if(m_abConnectFlag[1] == false)
  {
    //printf("Start Connect 1\n");
    //m_anRTULinkFD[1] = Connect(m_acIPAddress[1],m_anPort[1]);
    rstatus = LNC_GetConnectFileDescriptor((const CHAR *)m_acIPAddress[1],
                                           (const UINT16) m_anPort[1],
                                           SOCK_STREAM,
                                           m_anLinkDownTimeout[1]/2,
                                           &m_anRTULinkFD[1]);
    if(rstatus != E_ERR_Success)
    {
      #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMLINK)
      ERR_GetMsgString(rstatus, errbuff);
      printf("WARN [RMM] CRTULink::ConnectLink1, fail to get file descriptor\n"
             "  m_anRTULinkFD[1] %d, error %s\n",
             m_anRTULinkFD[1], errbuff);
      #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMLINK)
    }

    if(m_anRTULinkFD[1] == TYP_ERROR)
    {
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
      printf("[RMM] CRTULink::ConnectLink1, close %s:%d LAN 1 connection, "
             "socket %d\n",
             m_acIPAddress[1], m_anPort[1], m_anRTULinkFD[1]);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
      CloseConnectOtherRTU(1);
      m_Lan1LinkDown = true;
    }else{
      //g_pDebugLog->LogMessage((CHAR *)"RTU (LAN1) connect OK\n");
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
      printf("[RMM] CRTULink::ConnectLink1, LAN 1, fd %d %s:%d connection OK\n",
             m_anRTULinkFD[1], m_acIPAddress[1], m_anPort[1]);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK))
      m_abConnectFlag[1] = true;
      m_Lan1LinkDown = false;
      m_bSendCmdToRtuFlag = true;
    }
  }//if(m_abConnectFlag[1] == false)

}// ConnectLink1

/*******************************************************************************
Purpose:
  Close connection that links to other RTU.

Input
  nLANLinkID  -- LAN link ID (0 -- LAN2, 1 -- LAN1). (in)

*******************************************************************************/
void CRTULink::CloseConnectOtherRTU(int nLANLinkID)
{
  shutdown(m_anRTULinkFD[nLANLinkID], SHUT_WR);	//20151126 Su
  close(m_anRTULinkFD[nLANLinkID]);
  m_anRTULinkFD[nLANLinkID] = TYP_ERROR;	//20151126 Su
  m_abConnectFlag[nLANLinkID] = false;

  if(nLANLinkID == 0)
    m_Lan2LinkDown = true;
  else
    m_Lan1LinkDown = true;

  return ;
}

/*******************************************************************************
Purpose:
  Set write command.

Input
  unCommandCode  -- Write command code. (in)
  unParameter    -- Command parameter. (in)

*******************************************************************************/
void CRTULink::SetWriteCommand(unsigned short unCommandCode, unsigned short unParameter)
{
  m_tWriteCommand.aunCommand[m_tWriteCommand.nEndPoint] =
    (unCommandCode <<8) + unParameter;

  m_tWriteCommand.nEndPoint++;
  if(m_tWriteCommand.nEndPoint >= MAX_COMMAND_BUFFER)
    m_tWriteCommand.nEndPoint = 0;

} // SetWriteCommand

/*******************************************************************************
Purpose:
  Delete a write command.

*******************************************************************************/
void CRTULink::DeleteWriteCommand(void)
{
  if(m_tWriteCommand.nStartPoint != m_tWriteCommand.nEndPoint)
  {
    m_tWriteCommand.nStartPoint++;
    if(m_tWriteCommand.nStartPoint >= MAX_COMMAND_BUFFER)
      m_tWriteCommand.nStartPoint = 0;
  }
}


/*******************************************************************************
Purpose:
  Reset a timeout counter.
  Re-design in 20 May 2004, Yu Wei. It will use relative timer.

Input
  iType  -- Timeout type. (in)

Return
  Nil

*******************************************************************************/
void CRTULink::ActivateTimeOut(int iType)
{
  GetTimerValue(&m_atTimeOut[iType]);
}

/*******************************************************************************
Purpose:
  Check for timeout.
  Re-design in 20 May 2004, Yu Wei. It will use relative timer.

Input
  iType    -- Timeout type. (in)
  unTimeValue  -- Timeout value. (in)

Return
  ture  -- Timeout.
  false  -- Not timeout.

*******************************************************************************/
bool CRTULink::CheckTimeOut(int iType, unsigned int unTimeValue)
{
  struct tTimerValue tTime;
  unsigned int unTimerDiff = 0;

  GetTimerValue(&tTime);

  unTimerDiff = ( tTime.High - m_atTimeOut[iType].High ) * TIMER_LOW_MSEC;  //Get high part different.
  unTimerDiff += tTime.Low;          //Added low part different.
  unTimerDiff -= m_atTimeOut[iType].Low;

  if (unTimerDiff >= unTimeValue)
    return true;
  else
    return false;
}

/**************************************************************
Purpose:
  Set State flag and close connection socket, reset timer.
  Added in 26 April 2004, Yu Wei

***************************************************************/
void CRTULink::SetStateFlag(void)
{
  if(m_nCurrentStateFlag != g_tRTUStatus.nRTUStateFlag)
  {
    m_nCurrentStateFlag = g_tRTUStatus.nRTUStateFlag;


    switch(m_nCurrentStateFlag)
    {
    case STATEFLAG_PRIMARY:
      m_bSendCmdToRtuFlag = true;
      ActivateTimeOut(m_anCheckLink2Type);
      m_PrimarycloseFdFlag=true;// Yang Tao 20150402
      ActivateTimeOut(m_PrimarycloseFdType);// Yang Tao 20150402
      m_abOtherLinkStatusSave[0]=g_tRTUStatus.abOtherLinkStatus[0];
      m_abOtherLinkStatusSave[1]=g_tRTUStatus.abOtherLinkStatus[1];
//      printf("STATEFLAG_PRIMARY, start m_PrimarycloseFdFlag \n");
//      printf("---------- Before STATEFLAG_PRIMARY, Close connect to Other RTU \n");
      CloseConnectOtherRTU(0);
      CloseConnectOtherRTU(1);
      ActivateTimeOut(m_anFailureTimeoutType[0]);
      ActivateTimeOut(m_anFailureTimeoutType[1]);
      break;

    case STATEFLAG_STANDBY:
//    	printf("---------- Before STATEFLAG_STANDBY \n");
        ActivateTimeOut(m_anCheckLink2Type);
        m_PrimarycloseFdFlag=true;// Yang Tao 20150402
//        printf("STATEFLAG_STANDBY, start m_PrimarycloseFdFlag \n");
        ActivateTimeOut(m_PrimarycloseFdType);// Yang Tao 20150402
        m_abOtherLinkStatusSave[0]=g_tRTUStatus.abOtherLinkStatus[0];
        m_abOtherLinkStatusSave[1]=g_tRTUStatus.abOtherLinkStatus[1];
      ActivateTimeOut(m_anFailureTimeoutType[0]);
      ActivateTimeOut(m_anFailureTimeoutType[1]);
      break;

    case STATEFLAG_INITIALIZATION:
    case STATEFLAG_SWITCHING_PTOS:
    case STATEFLAG_SWITCHING_STOP:
    default:
      break;

    case STATEFLAG_HARDWARETEST:
    case STATEFLAG_SYSTEM_RESET:
      g_tRTUStatus.abOtherLinkStatus[0] = false;    //Set link down. //040611 Yu Wei
      g_tRTUStatus.abOtherLinkStatus[1] = false;

      break;
    }

  }
}

/**************************************************************
Purpose:
  Get LAN connection status for the designated LAN ID

Parameter

  ncLanID  [in] LAN link index

History

  Name             Date       Remark
  ----             ----       ------
 Bryan Chong   07-May-2010    Initial revision

***************************************************************/
//BOOL_T CRTULink::GetConnectStatus(UINT8 ncLanID)
//{
//  return ((BOOL_T)m_abConnectFlag[ncLanID]);
//} // GetConnectStatus
/*------------------------------------------------------------------------------
Purpose:
  Get LAN Link down status for the designated LAN ID

Parameter:

  ncLanID   [in] 0 - Lan1, 1 - Lan2
  prval     [out] pointer to output result
                  0 - No, 1 - Yes, -1 - Error

Return:

  E_ERR_Success
  E_ERR_RMM_InvalidLANID

History

  Name             Date       Remark
  ----             ----       ------
 Bryan Chong   07-May-2010    Initial revision

------------------------------------------------------------------------------*/
//E_ERR_T CRTULink::GetLinkDownStatus(UINT8 ncLanID, INT8 *prval)
//{
//  switch(ncLanID)
//  {
//    case 0:
//      *prval = m_Lan1LinkDown;
//      break;
//
//    case 1:
//      *prval = m_Lan2LinkDown;
//      break;
//
//    default:
//      *prval = TYP_ERROR;
//      return E_ERR_RMM_InvalidLanID;
//
//  } // switch(ncLanID)
//  return E_ERR_Success;
//} // GetConnectStatus
/*------------------------------------------------------------------------------
Purpose:
  Get LAN Link file descriptor for the designated LAN ID

Parameter:

  ncLanID   [in] 0 - Lan1, 1 - Lan2
  prval     [out] pointer to output result
                  0 - No, 1 - Yes, -1 - Error

Return:

  E_ERR_Success
  E_ERR_RMM_InvalidLANID

History

  Name             Date       Remark
  ----             ----       ------
 Bryan Chong   10-May-2010    Initial revision

------------------------------------------------------------------------------*/
//E_ERR_T CRTULink::GetLinkFileDesc(UINT8 ncLanID, INT32 *prval)
//{
//  switch(ncLanID)
//  {
//    case 0:
//      *prval = m_anRTULinkFD[0];
//      break;
//
//    case 1:
//      *prval = m_anRTULinkFD[1];
//      break;
//
//    default:
//      *prval = TYP_ERROR;
//      return E_ERR_RMM_InvalidLanID;
//
//  } // switch(ncLanID)
//
//  return E_ERR_Success;
//} // GetLinkFileDesc
/*------------------------------------------------------------------------------
Purpose:
  Get LAN Link file descriptor for the designated LAN ID

Parameter:

  ncLanID   [in] 0 - Lan1, 1 - Lan2
  prval     [out] pointer to output result
                  0 - No, 1 - Yes, -1 - Error

Return:

  E_ERR_Success
  E_ERR_RMM_InvalidLANID

History

  Name             Date       Remark
  ----             ----       ------
 Bryan Chong   10-May-2010    Initial revision

------------------------------------------------------------------------------*/
//E_ERR_T CRTULink::PrnDebugInfo(CHAR *outputbuffer, INT32 outputbuffsz)
//{
//  CHAR tmpbuff[200] = {0};
//  if(outputbuffer == TYP_NULL)
//    return E_ERR_InvalidNullPointer;
//
//  if(outputbuffsz < 0)
//    return E_ERR_InvalidParameter;
//
//  sprintf(outputbuffer, "  m_acIPAddress[0] %s, [1] %s\r\n",
//          m_acIPAddress[0], m_acIPAddress[1]);
//  sprintf(tmpbuff, "  m_bSendCmdToRtuFlag = %d\r\n", m_bSendCmdToRtuFlag);
//  strcat(outputbuffer, tmpbuff);
//
//  return E_ERR_Success;
//} // PrnDebugInfo

//#ifdef ModuleTest
//
//void CRTULink::LogLinkValues()
//{
//  int i,j;
//  FILE *fd;
//
//  //char pName[100];
//  //sprintf(pName, "RmmLink.txt", nNum);
//
//  printf("Stert write 'RmmLink.txt'\n");
//
//  fd = fopen("RmmLink.txt", "w+");
//  if ( fd == TYP_NULL) {
//    printf("Can not save RmmLink init values to RmmLink.txt.\r\n");
//    return;
//  }
//  //write time
//  time_t lTime = time(NULL);
//  fprintf(fd,"\tRmmLink values after initialization\r\n"
//          ":::::::::::::::::::::%s::::::::::::::::::::::\r\n", ctime(&lTime));
//
//  //write values
//
//  fprintf(fd,"int  g_nOtherRTULinkTaskID:%d\r\n", g_nOtherRTULinkTaskID);
//  fprintf(fd,"int g_anOtherRTUConnectionTaskID[0]:%d\r\n",
//          g_anOtherRTUConnectionTaskID[0]);
//  fprintf(fd,"int g_anOtherRTUConnectionTaskID[1]:%d\r\n",
//          g_anOtherRTUConnectionTaskID[1]);
//  fprintf(fd,"int g_anOtherRTULinkSocketTaskID[1]:%d\r\n",
//          g_anOtherRTULinkSocketTaskID[1]);
//  fprintf(fd,"int  g_bRMMLinkTaskWorking:%d\r\n", g_bRMMLinkTaskWorking);
//
//  fprintf(fd,"unsigned short g_unLastServerCommand:%d\r\n",
//          g_unLastServerCommand);
//
//  fprintf(fd,"int  m_nTableBytes:%d\r\n", m_nTableBytes);
//
//  fprintf(fd,"int m_anRTULinkFD[0]:%d\r\n", m_anRTULinkFD[0]);
//  fprintf(fd,"int m_anRTULinkFD[1]:%d\r\n", m_anRTULinkFD[1]);
//
//  fprintf(fd,"int m_anPollingInterval[0]:%d\r\n", m_anPollingInterval[0]);
//  fprintf(fd,"int m_anPollingInterval[1]:%d\r\n", m_anPollingInterval[1]);
//
//  fprintf(fd,"int m_anPollingIntervalType[0]:%d\r\n",
//          m_anPollingIntervalType[0]);
//  fprintf(fd,"int m_anPollingIntervalType[1]:%d\r\n",
//          m_anPollingIntervalType[1]);
//
//  fprintf(fd,"int m_anFailureTimeout[0]:%d\r\n", m_anFailureTimeout[0]);
//  fprintf(fd,"int m_anFailureTimeout[1]:%d\r\n", m_anFailureTimeout[1]);
//
//  fprintf(fd,"int m_anRetry[0]:%d\r\n", m_anRetry[0]);
//  fprintf(fd,"int m_anRetry[1]:%d\r\n", m_anRetry[1]);
//
//  fprintf(fd,"char m_acIPAddress[0]:%s\r\n", m_acIPAddress[0]);
//  fprintf(fd,"char m_acIPAddress[1]:%s\r\n", m_acIPAddress[1]);
//
//  fprintf(fd,"int m_anPort[0]:%d\r\n", m_anPort[0]);
//  fprintf(fd,"int m_anPort[1]:%d\r\n", m_anPort[1]);
//
//  fprintf(fd,"int m_anPollingTimeoutType[0]:%d\r\n", m_anPollingTimeoutType[0]);
//  fprintf(fd,"int m_anPollingTimeoutType[1]:%d\r\n", m_anPollingTimeoutType[1]);
//
//  fprintf(fd,"int m_anFailureTimeoutType[0]:%d\r\n", m_anFailureTimeoutType[0]);
//  fprintf(fd,"int m_anFailureTimeoutType[1]:%d\r\n", m_anFailureTimeoutType[1]);
//
//  fprintf(fd,"int m_anLinkDownTimeout[0]:%d\r\n", m_anLinkDownTimeout[0]);
//  fprintf(fd,"int m_anLinkDownTimeout[1]:%d\r\n", m_anLinkDownTimeout[1]);
//
//  fprintf(fd,"int m_abConnectFlag[0]:%d\r\n", m_abConnectFlag[0]);
//  fprintf(fd,"int m_abConnectFlag[1]:%d\r\n", m_abConnectFlag[1]);
//
//
//  fprintf(fd,"struct timespec m_atTimeOut[RTU_LINK_MAX_TIMEOUT_TYPE]:\r\n");
//  for (i=0; i< RTU_LINK_MAX_TIMEOUT_TYPE; i++)
//    fprintf(fd, "    %d Second:%d NSecond:%d\r\n",
//            i, m_atTimeOut[i].tv_sec, m_atTimeOut[i].tv_nsec);
//
//  fprintf(fd,"tCommandList m_tWriteCommand:nStartPoint:%d nEndPoint:%d\r\n",
//    m_tWriteCommand.nStartPoint,m_tWriteCommand.nEndPoint);
//
//  fprintf(fd,"int m_nPollingCommand:%d\r\n", m_nPollingCommand);
//
//  fprintf(fd,"int m_nRetry:%d\r\n", m_nRetry);
//
//  fprintf(fd,"int m_nSWCPollingTableID:%d\r\n", m_nSWCPollingTableID);
//  fprintf(fd,"int m_nSWCPollingTableTypeID:%d\r\n",  m_nSWCPollingTableTypeID);
//
//
//  lTime = time(NULL);
//
//  fprintf(fd,"\r\n:::::::::::::::::::::End: %s::::::::::::::::::::::\r\n",
//          ctime(&lTime));
//
//  fclose(fd);
//
//  printf("End write 'RmmLink.txt'\n");
//}
//#endif

