/*----------------------------------------------------------------------------

            Copyright (c) 2011 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME

  RMM.cpp

 COMPONENT

  Redundancy Management Module (RMM)

 DESCRIPTION

  This file is to manage redundancy process

 AUTHOR

  Yu Wei


 REVISION HISTORY by Bryan Chong

 D1.1.5
 ------
 31-May-2011

 - remove duplicated header file [C955 PR97]
 - convert cout to printf for readability issue from the group [C955 PR99]
 - added nReturn == ERROR as a return value [C955 PR101]
 - remove ENABLE_RTUINITLINKCHK_UPDATE [C955 PR105]
 - remove code when g_nRMMSocketTaskID == ERROR, StopRMM routine [C955 PR100]

 D1.1.3
 ------
 29-Apr-2011

 - remove all code under definition CFG_RMM_NOTATION_THRD [C955 PR103]
 - remove CRTUStatus::ReadMultiNodeSWCLinkStatus per ZSL request [C955 PR104]
 - remove CRTUStatus::UpdateMultiNodeSWCLinkStatus per ZSL request [C955 PR104]
 - remove commented code CRTUStatus::CopySWCPollingTable [C955 PR106]

 27-Apr-2011

 - CRTUStatus::ClearFlag
     Add reset m_bWaitLinkCheckCompletedFlag
 - Update to add modification history [C955 PR96]

------------------------------------------------------------------------------*/
/************************************************************
*                              *
*  Project:    C830 RTU                  *
*  Module:    Redundancy Management Module (RMM)      *
*  File :      RMM.cpp                    *
*  Author:    Yu Wei                    *
*                              *
*  Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************

**************************************************************

DESCRIPTION

  This file RMM module main control and RTU status table access
  object.

*************************************************************
Modiffication History
---------------------

Version: RMM D.1.1.1
  01  16 April 2003, Yu Wei,
    Start to write.

Version: RMM D.1.2.0
  01  03 July 2003, Yu Wei
    Start to add Redundancy function.
    Add Debug log date switch process.

Version: RMM D.1.2.1
  01  14 July 2003, Yu Wei
    D.1.2.0 send to Ripple. Continue to add RMM.
    Add RTU notation (weight) calculation.

  02  15 July 2003, Yu Wei
    Add other RTU status table accessing function.

  03  21 July 2003, Yu Wei,
    Add SWC link down in primary RTU, standby RTU will check
    the link status. Based on checking result, the primary RTU
    decide whether the state change.

Version: RMM D.1.3.0
  01  01 August 2003, Yu Wei,
    All SWC use same class except Clock.

  02  07 August 2003, Yu Wei,
    Updated polling table is done by RMM.
    Added primary RTU sending polling table to standby RTU.
    Set default Clock link is down and other SWC is OK.

  03  19 August 2003, Yu Wei
    Add all server command.

  04  03 September 2003, Yu Wei
    Added g_bRTUInitLinkCheck and g_abSWCInitLinkCheck. When
    system startup and RTU changed to primary from init state,
    RMM must wait for all SWC check link, then ask standby RTU
    to check link that the SWC's link is down.

Version: RMM D.1.3.1
  01  17 September 2003, Yu Wei,
    Change bRMMTaskWorking to g_bRMMTaskWorking.

  02  24 September 2003, Yu Wei,
    Modify CheckRTUStatus(), when RTU is switching, the status
    word 4 should be 1 (primary RTU). When RTU is maintanence,
    the status word 5 should be 1 (standby RTU).

Version: RMM D.1.3.2
  01  03 October 2003, Yu Wei,
    Server command for SWC enable/inhibit change, modify
    SendSWCPollingCommand().
    Modify RMMServerCommand(). If RTU received server command change state to
    primary, check other RTU state first. If other is work in primary state,
    it will reply exception 04.

  02  06 October 2003, Yu Wei,
    Clock will not accept link enable/inhibit command.

  03  07 November 2003, Yu Wei,
    Added m_bServerSwitchFlag to indicate that server switches primary RTU
    to standby.

  04  10 November 2003, Yu Wei,
    Deleted task "OtherRTULinkTask", it is done by RMM main task "RMMTask".
    This change can synchronize two tasks to solve the problems that
    RTU stop poll SWC and remain in switching state.
    Refer to PR:OASYS-PMS 0011, PR:OASYS-PMS 0012, PR:OASYS-PMS 0015 and
    PR:OASYS-PMS 0016.

  05  11 November 2003, Yu Wei,
    When standby RTU check link, the flags m_aunSWCLinkCheckIndex will not
    set, the SWC link status is always updated. Modify RTUStatusTableProcess().

  06  13 November 2003, Yu Wei,
    Added m_unLastServerCommand for MMM using.

  07  14 November 2003, Yu Wei,
    Added CheckSWCLinkStop() for checking SWC link stop.
    The task "OtherRTULinkTask" is added. See above mofification 04.

    Added watchdog function.

  08  17 November 2003, Yu Wei,
    Log file day switching will be checked by watchdog task.

  09  18 November 2003, Yu Wei
    Added watchdog function.
    Added g_tRMMMainWatchdogID, RMM_MAIN_WATCHDOG_TIMEOUT.

  10  20 November 2003, Yu Wei
    Modified RTUStatusTableProcess(). The tMessage.nMessageID
    is index number.

  11  21 November 2003, Yu Wei
    Modified SetSWCLinkStop(). When multidrop link SWC link down,
    another SWC also need stop the link.

    Added ClearFlag() to clear flags.

Version: RMM D.1.3.3
  01  08 December 2003, Yu Wei,
    modify RTUStatusTableProcess() to fix bug that RTU will stop
    poll SWC when RTU link is down and SWC is down.

  02  26 January 2004, Yu Wei,
    Delete CRTUStatus() parameter, using global variable.

    Modified RMMServerCommand().
    When RTU work in standby state and server ask it change to
    standby, RTU do nothing instead of changing to switching state
    then standby state.

Version: RMM D.1.3.4
  01  27 February 2004, Yu Wei
    When g_bRMMTaskWorking == false, use break instand of return.

  02  02 March 2004, Yu Wei
    Added LogRmmStatusValues() for module test.
    Refer to PR:OASYS-PMS 0032.

Version: RMM D.1.3.5
  01  10 March 2004, Yu Wei
    Reduce complexity in RMMCheckServerCommand() and
    CRTUStatus::RTUStatusTableProcess().
    Added LogServerCommand() and SWCLinkCheckProcess().
    Modified CalculateWeight(), when weight is change, it will update
    RTU status table.
    Refer to PR:OASYS-PMS 0044.

  02  10 March 2004, Yu Wei
    Added default case for switch in RMMServerCommandError()
    and RTUStatusTableProcess().
    Refer to PR:OASYS-PMS 0042.

  03  11 March 2004, Yu Wei
    Fixed bug that a SWC link down, the RTU will switch to standby.
    The primary didn't wait for standby check link completed.
    Added m_bWaitLinkCheckCompletedFlag.
    Modified CRTUStatus(), RTUStatusTableProcess().
    Refer to PR:OASYS-PMS 0066.

  04  22 March 2004, Yu Wei
    Changed nWeight type from int to unsigned short in CalculateWeight().
    m_unSWCWeight and g_tRTUConfig.tSWC[].tHeader.nNotation are unsigned short.
    Refer to PR:OASYS-PMS 0047.

    STATEFLAG_SWITCHING is changed, modified RMMServerCommand() and
    CheckRTUStatus().
    Refer to PR:OASYS-PMS 0143.

  05  23 March 2004, Yu Wei
    Remove CopySWCPollingTable(), the routine is not used now.
    The SWC table is copied  to standby RTU SWC table.
    Refer to PR:OASYS-PMS 0143.

    Added m_bSWCSwitchCompletedFlag for checking if all SWC changed to
    STATEFLAG_SWITCHING_PTOS
    and m_bSendSWCTableCompletedFlag for check if primary RTU send all SWC
    latset table to standby RTU, m_bStartSendSWCTableFlag indicate the
    starting  of sending SWC's table.
    Modified CRTUStatus().
    Refer to PR:OASYS-PMS 0143.

    Added ClearVariables() to clear all variables when new object.
    Refer to PR:OASYS-PMS 0054.

  06  26 March 2004, Yu Wei
    When standby RTU receive change to primary command, change
    STATEFLAG_SWITCHING_STOP.
    Refer to PR:OASYS-PMS 0143.

  07  29 March 2004, Yu Wei
    Modified UpdateRTUStatus(). Convert local time to UTC time.
    Refer to PR:OASYS-PMS 0151.

Version: RMM D.1.3.6
  01  19 April 2004, Yu Wei
    Modified CheckRTUStatus() to update RMT link status.
    Refer to PR:OASYS-PMS 0162.

  02  19 April 2004, Yu Wei
    Added m_bOtherRTUSwitchFlag to store other is switching info.
    Modified CRTUStatus() to clear m_bOtherRTUSwitchFlag.
    Modified RMMServerCommand(). When other is switching, it will not accept
    change to primary RTU command.
    Refer to PR:OASYS-PMS 0163.

  03  20 April 2004, Yu Wei
    Modified CRTUStatus() to init m_nSwitchFlag.
    Modified RMMServerCommand(), set m_nSwitchFlag to swtich to primary.
    Refer to PR:OASYS-PMS 0163.

  04  29 April 2004, Yu Wei
    Modified RMMServerCommand(), convert local time to UTC time for
    config file downloading.
    Refer to PR:OASYS-PMS 0176.

  05  30 April 2004, Yu Wei
    Modified CalculateWeight(). When SWC link is inhibited, the notation
    will not calculate.
    Modified SetSWCLinkStop(). When SWC link is inhibited, the SWC
    will not check.
    Modified RTUStatusTableProcess(). When SWC link is enable and link down,
    send check link command to SWC.
    Modified SetSWCLinkCheckIndexTable(). When SWC link is enable,
    send check link command to SWC.
    Refer to PR:OASYS-PMS 0177.

  06  20 May 2004, Yu Wei
    Modified ReadRTUStatus() and WriteOtherRTUStatus(). Using memcpy to
    copy data will be fast.
    Refer to PR:OASYS-PMS 0163.

  07  21 May 2004, Yu Wei
    Modified RMMServerCommand(). Receive config file download complete
    command, set flag only.
    Refer to PR:OASYS-PMS 0176.

  08  03 June 2004, Yu Wei
    Modified SetSWCLinkCheckIndexTable(). If SWC is checking link,
    don't check again. Send message to SWC will delay time and reply
    other RTU will be delayed.
    Modified SetLinkCheckCompleted(). Clear flag to avoid check
    link again.
    Modified ClearFlag(). Clear flag to avoid send check link
    when state change to primary.
    Refer to PR:OASYS-PMS 0163.

  09  09 June 2004, Yu Wei
    Modified SetSWCLinkCheckIndexTable() and SWCLinkCheckProcess().
    Standby RTU checking link don't base on m_bSWCLinkCheckTotalFlag.
    Refer to PR:OASYS-PMS 0168.

  10  11 June 2004, Yu Wei
    Modified CalculateWeight(). Hardware test and system reset states
    will set weigth to 0.
    Modified CheckRTUStatus(). Hardware test and system reset states
    will clear RTU status switching, primary, standby flags.
    Refer to PR:OASYS-PMS 0180.

Version: RMM E.1.0.0
  01  07 July 2004, Yu Wei
    Modified RMMServerCommand(). When command cannot execute, RTU
    will reply execption 3 instead of 4.
    Refer to PR:OASYS-PMS 0229.

Version: RMM E.1.0.2
  01  14 Apr 2005, Tong Zhi Xiong
    Remove "error code" log from RMMServerCommandError, refer to PR:OASYS-PMS 0240

Version: RMM E.1.0.3
  01  12 May 2005, Tong Zhi Xiong
    Add GetOtherRtuBetterSwcIndex to check out the item better weight


**************************************************************/
#include <iostream.h>
#include <sys/neutrino.h>
#include <fixerrno.h>
#include <termios.h>
#include <stdio.h>
#include <sys/netmgr.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "err_ext.h"
#include "hwp_def.h"
#include "hwp_ext.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
#include "sys_ass.h"
#include "rmm_def.h"
#include "str_def.h"
#include "str_ext.h"
#include "cgf_def.h"
#include "cgf_ext.h"
//#include "flg_def.h"
//#include "flg_ext.h"

#include "Common.h"
#include "CMM_Log.h"
#include "RMM.h"
#include "CMM_Timer.h"
#include "CMM_Listen.h"
#include "CMM_Modbus.h"
#include "RMM_RTULink.h"
#include "rmm_ext.h"
#include "SWC.h"
#include "CMM_Watchdog.h"
#include "Common_qnxWatchDog.h"



timer_t       RMM_main_timer_id;

CRTUStatus *g_pRTUStatusTable;  //Object for RTU status table process.
CListen    *g_pRMMServerSocket;  //Object for server socket.
tRTUPermanentParameter g_tRTUPerPara;  //RTU permanent parameters.

bool g_bRMMTaskWorking = false;
pthread_t g_nRMMTaskID;        //RMM main task ID.
pthread_t g_nRMMSocketTaskID;      //RMM server socket task ID.

/*
  If it is set to true, primary RTU will not ask standby RTU
  to check SWC's link status when SWC link is down. It will be
  set to false when all primary RTU's SWC check link at least one
  time.
*/
 bool g_bRTUInitLinkCheck = true;

 bool g_OverTemperature=false;
/*
The flag will set to false initially. When SWC checks link at least one
time, it will be set to true.
*/
bool g_abSWCInitLinkCheck[SWC_MAX_NUMBER];

unsigned short g_unLastServerCommand = 0;  //Store last server command for MMM reading.

int    	g_tRMMMainWatchdogID;
 bool m_PrimarycloseFdFlag; // Yang Tao 20150402 Add in this flag to indciate
bool 	m_abOtherLinkStatusSave[2];							 // True = start to close.
int 	m_PrimarycloseFdType;
 int m_PrimarycloseFdFlagTimeout;//

/**********************************************************************************
Purpose:
  Initialize Redundancy Management Module, create RTU status table access object
  and  spawn all RMM tasks.If return is ERROR, it will delete all created objects
  and spawned tasks.

Return:
  OK    -- Create objects and spawn tasks OK.
  ERROR  -- Create one object or spawn one task ERROR.

History

    Name          Date         Remark

 Bryan Chong   31-May-2011  added nReturn == ERROR as a return value [PR101]
 Bryan Chong   01-Aug-2011  return the error directly when error resulted from
                            creating watchdog [C955 PR101]
*******************************************************************************/
int RMMInitialization(void)
{
  E_ERR_T rstatus = E_ERR_Success;
  int nReturn = OK;

  g_pRTUStatusTable = new CRTUStatus();

  if(g_pRTUStatusTable == TYP_NULL)
  {
    g_pEventLog->LogMessage((CHAR *)"RMM create CRTUStatus object error.\n");
    nReturn =  ERROR;
  }else{
    clock_gettime(CLOCK_REALTIME, &g_pRTUStatusTable->m_tStartTime);

    g_bRMMTaskWorking = true;
    StateInitialize();    //Initialize RTU state flag.

    RMM_main_timer_id = wdCreate( WatchDogChid ,
                     RMM_main_timer_id , RMM_MAIN ,RMM_MAIN_TASK_ID_INDEX);

    if(RMM_main_timer_id == TYP_ERROR)
    {
      g_pEventLog->LogMessage((CHAR *)"RMM Create watchdog ID error.\n");
      nReturn =  ERROR;

      // 20110801 BC [C955 PR101]
      return nReturn;
    }

    if(OtherRTULinkInit() == ERROR)    //Initialize Other RTU Link task.
    {
      g_pEventLog->LogMessage((CHAR *)"otherRTULinInit error.\n");
      nReturn = ERROR;
    }else{
      g_pRMMServerSocket = new CListen(
        g_tRTUConfig.tLANPara[0].acLAN_IP,
        g_tRTUConfig.nRTUCommandSoctetID,
        g_tRTUConfig.nServerCMDTimeout);
      if(g_pRMMServerSocket == TYP_NULL)
      {
        #ifdef CFG_PRN_ERR
        printf("ERR  [RMM] RMMInitialization, create listen object error\n");
        #endif // CFG_PRN_ERR
        g_pEventLog->LogMessage((CHAR *)"RMM g_pRMMServerSocket object error.\n");
        nReturn =  ERROR;
      }else{

        rstatus = SYS_ThrdInit(E_SYS_TLB_RMM_ServerSocketTask);
        if(rstatus != E_ERR_Success)
        {
          #ifdef CFG_PRN_ERR
          printf("ERR  [RMM] RMMInitialization, create RMM Server Socket Task "
                 "fail\n");
          #endif // CFG_PRN_ERR
          g_pEventLog->LogMessage((CHAR *)
            "RMM spwan RMMServerSocketTask error.\n");
          nReturn =  ERROR;
        }

        g_nRMMSocketTaskID =
          pSYS_CB->thrd_ctrl[E_SYS_TLB_RMM_ServerSocketTask].thrd_id;

        delay(1000);

        rstatus = SYS_ThrdInit(E_SYS_TLB_RMM_Task);
        if(rstatus != E_ERR_Success)
        {
          #ifdef CFG_PRN_ERR
          printf("ERR  [RMM] RMMInitialization, create RMM Task "
                 "fail\n");
          #endif // CFG_PRN_ERR
          g_pEventLog->LogMessage((CHAR *)"RMM spwan task error.\n");
          nReturn =  ERROR;
        }
        g_nRMMTaskID =
          pSYS_CB->thrd_ctrl[E_SYS_TLB_RMM_Task].thrd_id;

      }// if(g_pRMMServerSocket == TYP_NULL)
    }// if(OtherRTULinkInit() == ERROR)
  }// if(g_pRTUStatusTable == TYP_NULL)

  if(nReturn == ERROR)
  {
    StopRMM();
  }

  return nReturn;
}

/**********************************************************************************
Purpose:
  Delete RTU status table access object and stop all RMM tasks.

Return:
  Nil.

History

  Name         Date        Remark

 Bryan Chong  31-May-2011 Remove code when g_nRMMSocketTaskID == ERROR
                          [C955 PR100]
***********************************************************************************/
void StopRMM(void)
{
  if(g_pRTUStatusTable != NULL)
  {

    if(g_nRMMSocketTaskID != ERROR)
    {
      //detach this thread from this process
      pthread_detach(g_nRMMSocketTaskID);
      pthread_cancel(g_nRMMSocketTaskID);
    }

    if(g_pRMMServerSocket != NULL)
    {
      delete g_pRMMServerSocket;
    }

    g_bRMMTaskWorking = false;
  }

}

/*******************************************************************************
Purpose:
  Main RMM task.

Return:
  Nil.

History

     Name        Date        Remark

 Bryan Chong  16-Jun-2011  When watchdog fail to start, log an error message
                           into event log [C955 PR 101]

*******************************************************************************/
void *RMMTask(void *arg)
{
  int nI;
//  int i = 0;


  for(nI=0; nI<SWC_MAX_NUMBER; nI++)
  {
    g_abSWCInitLinkCheck[nI] = false;
  }

  while(1)
  {
    if(RMM_main_timer_id != -1)
    {
      if(wdStart(RMM_main_timer_id ,  30, 0, 0, 0) < 0)
      {
        #ifdef CFG_PRN_ERR
        printf("ERR [RMM] RMMTask, can not start the RMM task wathcdog\n");
        #endif // CFG_PRN_ERR

        // 20110616 BC
        g_pEventLog->LogMessage((CHAR *)
          "ERR  [RMM] RMMTask, fail to restart watchdog\n");
      }
    }

    StateManagement();

    if(g_bRTUInitLinkCheck == true)
    {
      g_pRTUStatusTable->RTUInitLinkCheck();
    }
    else
    {
      g_pRTUStatusTable->RTUStatusTableProcess();
    }

    //Process server command
    RMMCheckServerCommand();

    delay(RMM_MAIN_TASK_SLEEP);

    if(g_bRMMTaskWorking == false)
    {
      if(RMM_main_timer_id != TYP_NULL)
      {
        wdCancel(RMM_main_timer_id);
        mq_unlink(RMM_MSGQ);
        timer_delete(RMM_main_timer_id);
      }
      pthread_detach(pthread_self());
      pthread_exit(NULL);
      break;    //040227 Yu Wei
    }
  }
  #ifdef CFG_PRN_ERR
  printf("ERR  [RMM] RMMTask, thread exit\n");
  #endif // CFG_PRN_ERR
  return NULL;
}// RMMTask

/**********************************************************************************
Purpose:
  Check if it received a specifical command that is sent by server.

Return:
  Nil.
***********************************************************************************/
void RMMCheckServerCommand(void)
{
  char acServerRecvBuffer[MODBUS_MAX_CMD_SIZE];
  int nServerRecvNumber;
  int nModbusError;
  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM_RX_CMD))
  unsigned char buffer[11];
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))

  if((nServerRecvNumber = g_pRMMServerSocket->
       Recv(acServerRecvBuffer,MODBUS_MAX_CMD_SIZE,0,0)) > 0)//20140604 Yang Tao Q: RMM recv time out is o
  {
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM_RX_CMD)
    memcpy(buffer , (unsigned char *)acServerRecvBuffer , 11);
    printf("[RMM] RMMCheckServerCommand, recv\n");
    SYS_PrnDataBlock((const UINT8 *)buffer, nServerRecvNumber, 10);
    #endif // CFG_DEBUG_MSG

    nModbusError = ModbusCMDCheck((unsigned char *)acServerRecvBuffer,
                nServerRecvNumber,
                g_tRTUConfig.tLANPara[0].tPrtc.addr);

    if(nModbusError != MODBUS_MESSAGE_OK)
    {
      g_pEventLog->LogMessage(
          (CHAR *)"RMM_CheckServerCommand nModbusError != MODBUS_MESSAGE_OK\n");
      RMMServerCommandError(nModbusError, acServerRecvBuffer);
    }
    else
    {
      RMMServerCommand(acServerRecvBuffer);
    }
  }
}

/*******************************************************************************
Purpose:
  Log command sent by server or the response to server.
  Added in 10 March 2004, Yu Wei

Input/Output
  acData    -- The buffer for command data. (in)
  nDataLength  -- Data buffer length. (in)
  acMessage  -- The buffer for log header.(in)
***********************************************************************************/
void LogServerCommand(char *acData, int nDataLength, char *acMessage)
{
  int nI;
  char acTemp[1024],acTemp1[1024];

  if(g_tDebugLogFlag.bServerCommand == true)
  {
    //Write to debug log.
    for(nI=0; nI<nDataLength; nI++)
      sprintf(&acTemp1[nI*3],"%02X",(unsigned char)acData[nI]);
    sprintf(acTemp,"%s %s",acMessage, acTemp1);
    g_pDebugLog->LogMessage(acTemp);
  }
}

/*******************************************************************************
Purpose:
  Parse server command and forward command to SWC using message queue

Parameters
  acServerRecvBuffer  [in]The buffer for receiving command sent by server.

Return
  None

History

      Name       Date              Remark
      ----       ----              ------
  Bryan Chong  11-Dec-2009  Update to fix Inhibit SWC Polling and Enable SWC
                            Polling
  Bryan Chong  17-May-2010  Update to include RMM_RTU_HEALTH_TBL data structure
                            for health status mapping [C955 PR40]
  Bryan Chong  02-Jul-2010  Update to fix inhibit SWC polling command by
                            swapping bytes
  Bryan Chong  05-Apr-2011  Update to fix issue of enable and inhibit for
                            designated SWC from server command. [C955 PR93]
*******************************************************************************/
void RMMServerCommand(char *acServerRecvBuffer)
{
  E_ERR_T rstatus = E_ERR_Success;
  int nReplyCMDLen = 8;
  char acTemp[1024];
  char cLog[1000],acTemp1[200];
  int nI;
  UINT16 ServerCommand;
  // 20100517 BC (Rqd ZSL)
  RMM_RTU_HEALTH healthStatus;
  UINT8 swcidx = 0;
  #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMSVRCMD)
  CHAR  errBuff[100] = {0};
  #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMSVRCMD)
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};


  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
  printf("[RMM] RMMServerCommand, rx 11 bytes from server\n");
  SYS_PrnDataBlock((const UINT8 *)acServerRecvBuffer, 11, 10);
  #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))

//  pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_ServerCommand,
//    FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//    "[RMM] RMMServerCommand, %s\n"
//    "  rx specific cmd 11 bytes from server:\n",
//    SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
//  pFLG_CtrlBlk->SendDataBlock(E_FLG_MSG_RMM_ServerCommand,
//    FLG_LOG_T_Event, (UINT8 *)acServerRecvBuffer, 11, 20);
  //printf("1\n");
  for(nI=0; nI<11; nI++)
  sprintf(&acTemp1[nI*2], "%02X",(unsigned char)acServerRecvBuffer[nI]);
  sprintf(cLog,"[RMM] %s: rx specific cmd 11 bytes from server:%s\n",
		    SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),acTemp1);
  g_pEventLog->LogMessage(cLog);
  // 20100517 BC (Rqd ZSL)
  healthStatus.allbits = STR_ByteSwap16Bit(g_pRTUStatusTable->m_aunOtherRTUStatus[6]);

  if(acServerRecvBuffer[1] == 0x10)  //Only process write word command.
  {
    if(*(unsigned short *)&acServerRecvBuffer[2] ==
       g_tRTUConfig.tRTUComanmdTable.unTableStartAddress)
    {
      g_unLastServerCommand =
          ((unsigned short)acServerRecvBuffer[7] & 0x00FF) << 8 |
          ((unsigned short)acServerRecvBuffer[8] & 0x00FF);
      ServerCommand = g_unLastServerCommand;

      switch(ServerCommand & 0xFF00)
      {
      case SERVER_CMD_RESET_RTU:
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
        printf("[RMM] RMMServerCommand, rx SERVER_CMD_RESET_RTU (VALIDAION_BIT)\n");
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
//        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_ServerCommand,
//          FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "[RMM] RMMServerCommand, %s\n"
//          "  rx specific cmd SERVER_CMD_RESET_RTU\n",
//          SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
        //printf("2\n");
        sprintf(cLog,"[RMM] RMMServerCommand, %s\n"
                "  rx specific cmd SERVER_CMD_RESET_RTU\n",
                SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
        g_pEventLog->LogMessage(cLog);
        delay(1000);// 20151123 Yang Tao add in for log it's server cmd
        ChangeStateTo(STATEFLAG_SYSTEM_RESET);
        break;

      case SERVER_CMD_SWITCH_TO_STANDBY:
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
        printf("[RMM] RMMServerCommand, rx SERVER_CMD_SWITCH_TO_STANDBY "
               "(0x4000)\n");
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
//        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_ServerCommand,
//          FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "[RMM] RMMServerCommand, %s\n"
//          "  rx SERVER_CMD_SWITCH_TO_STANDBY\n",
//          SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
       // printf("3\n");
        sprintf(cLog,"[RMM] RMMServerCommand, %s\n"
                "  rx SERVER_CMD_SWITCH_TO_STANDBY\n",
                SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
        g_pEventLog->LogMessage(cLog);
        if(g_tRTUStatus.nRTUStateFlag == STATEFLAG_PRIMARY)
        {
          printf("g_tRTUStatus.nRTUStateFlag == STATEFLAG_PRIMARY) \n");
          ChangeStateTo(STATEFLAG_SWITCHING_PTOS);
          g_pRTUStatusTable->m_bServerSwitchFlag = true;
        }
        else
        {
          acServerRecvBuffer[1] |= 0x80;
          acServerRecvBuffer[2] = MODBUS_EXCEPTION3;  //040707 Yu Wei
          nReplyCMDLen = 5;
        }

        break;

      case SERVER_CMD_SWITCH_TO_PRIMARY:
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
        printf("[RMM] RMMServerCommand, rx SERVER_CMD_SWITCH_TO_PRIMARY "
               "(0x2000)\n");
        printf("[RMM] RMMServerCommand, read other RTU status\n");
        SYS_PrnDataBlock(
            (const UINT8 *)g_pRTUStatusTable->m_aunOtherRTUStatus, 33, 10);
        #endif // ((defined CFG_DEBUG_MSG) &&
               // (defined CFG_DEBUG_RMM_OTHER_STATUS))
//        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_ServerCommand,
//          FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "[RMM] RMMServerCommand, %s\n"
//          "  rx specific cmd SERVER_CMD_SWITCH_TO_PRIMARY\n",
//          SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
       // printf("4\n");
        sprintf(cLog,"[RMM] RMMServerCommand, %s\n"
                "  rx specific cmd SERVER_CMD_SWITCH_TO_PRIMARY\n",
                SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
        g_pEventLog->LogMessage(cLog);

        // check if state flag is Primary
        if(((STR_ByteSwap16Bit(
               g_pRTUStatusTable->m_aunOtherRTUStatus[6])) & 0x0010) == 0x0010)
        {
          g_pRTUStatusTable->m_nOtherRTUStateFlag = STATEFLAG_PRIMARY;
        }
        // check if state flag is Standby
        else if((STR_ByteSwap16Bit(
                   g_pRTUStatusTable->m_aunOtherRTUStatus[6]))&0x0020
                == 0x0020)
        {
          g_pRTUStatusTable->m_nOtherRTUStateFlag = STATEFLAG_STANDBY;
        }

        //When other is switching, not accept change to primary command.
        if(((g_pRTUStatusTable->m_nOtherRTUStateFlag == STATEFLAG_PRIMARY) &&
           ((g_tRTUStatus.abOtherLinkStatus[0] == true) ||
            (g_tRTUStatus.abOtherLinkStatus[1] == true))) ||
           (g_tRTUStatus.nRTUStateFlag != STATEFLAG_STANDBY) ||
           (g_pRTUStatusTable->m_bOtherRTUSwitchFlag == true))
        {
        	// Yang Tao: Add in as the Server command debug messagef
//          printf("m_nOtherRTUStateFlag = %d\n",g_pRTUStatusTable->m_nOtherRTUStateFlag);
//          printf("g_tRTUStatus.abOtherLinkStatus[0] = %d\n",g_tRTUStatus.abOtherLinkStatus[0]);
//          printf("g_tRTUStatus.abOtherLinkStatus[1] = %d\n",g_tRTUStatus.abOtherLinkStatus[1]);
//          printf("g_tRTUStatus.nRTUStateFlag = %d\n",g_tRTUStatus.nRTUStateFlag);
//          printf("g_pRTUStatusTable->m_bOtherRTUSwitchFlag = %d\n",g_pRTUStatusTable->m_bOtherRTUSwitchFlag);

          acServerRecvBuffer[1] |= 0x80;
          acServerRecvBuffer[2] = MODBUS_EXCEPTION3;  //040707 Yu Wei
          nReplyCMDLen = 5;
        }
        else
        {
          memset(acTemp, 0, RTU_STATUS_TABLE_LENGTH_MAX*2);
          g_pRTUStatusTable->WriteOtherRTUStatus((unsigned short *)&acTemp);
          ChangeStateTo(STATEFLAG_SWITCHING_STOP);  //040326 Yu Wei
          //Set switching status. //040420 Yu Wei
          g_pRTUStatusTable->m_nSwitchFlag = COMFIRM_SWITCH;
        }
        break;

      case SERVER_CMD_DOWNLOAD_CFG_COMPLETE:
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
        printf("[RMM] RMMServerCommand, rx SERVER_CMD_DOWNLOAD_CFG_COMPLETE "
               "(0x1000)\n");
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
//        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_ServerCommand,
//          FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "[RMM] RMMServerCommand, %s\n"
//          "  rx specific cmd SERVER_CMD_DOWNLOAD_CFG_COMPLETE\n",
//          SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
        //printf("5\n");
        sprintf(cLog,"[RMM] RMMServerCommand, %s\n"
                "  rx specific cmd SERVER_CMD_DOWNLOAD_CFG_COMPLETE\n",
                SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
        g_pEventLog->LogMessage(cLog);

        g_tRTUStatus.bCFGFileDownloadRequired = false;
        break;

      case SERVER_CMD_DOWNLOAD_CFG_REQUIRED:
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
        printf("[RMM] RMMServerCommand, rx SERVER_CMD_DOWNLOAD_CFG_REQUIRED "
               "(0x0800)\n");
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
//        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_ServerCommand,
//          FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "[RMM] RMMServerCommand, %s\n"
//          "  rx specific cmd SERVER_CMD_DOWNLOAD_CFG_REQUIRED\n",
//          SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
       // printf("6\n");
        sprintf(cLog,"[RMM] RMMServerCommand, %s\n"
                "  rx specific cmd SERVER_CMD_DOWNLOAD_CFG_REQUIRED\n",
                SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
        g_pEventLog->LogMessage(cLog);
        g_tRTUStatus.bCFGFileDownloadRequired = true;
        break;

      case SERVER_CMD_ENABLE_SWC_POLLING:
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
        printf("[RMM] RMMServerCommand, rx SERVER_CMD_ENABLE_SWC_POLLING "
               "(0x%04x) for SWC 0x%02x\n",
               SERVER_CMD_ENABLE_SWC_POLLING, (ServerCommand & 0x00FF));
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
//        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_ServerCommand,
//          FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "[RMM] RMMServerCommand, %s\n"
//          "  rx specific cmd SERVER_CMD_ENABLE_SWC_POLLING\n",
//          SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
       // printf("7\n");
        sprintf(cLog,"[RMM] RMMServerCommand, %s\n"
                "  rx specific cmd SERVER_CMD_ENABLE_SWC_POLLING\n",
                SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
        g_pEventLog->LogMessage(cLog);

        if((int)(STR_ByteSwap16Bit(g_unLastServerCommand )& 0x00FF) == 3) //Clock don't accept this command.
        {
          acServerRecvBuffer[1] |= 0x80;
          acServerRecvBuffer[2] = MODBUS_EXCEPTION3;  //040707 Yu Wei
          nReplyCMDLen = 5;
        }
        else
        {
          // 20110405 BC
          swcidx = (UINT8)(ServerCommand & 0x00FF);

          if(g_tRTUConfig.nSwcEnabled[swcidx] == E_TYPE_No)
          {
            #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMSVRCMD)
            printf("WARN [RMM] RMMServerCommand, SWC %d is not enabled\n",
                   swcidx);
            #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMSVRCMD)
            break;
          }
          rstatus =
            g_pRTUStatusTable->SendSWCPollingCommand(SWC_POLLING_ENABLE,
                                                     swcidx);
          if(rstatus != E_ERR_Success)
          {
            #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMSVRCMD)
            ERR_GetMsgString(rstatus, errBuff);
            printf("WARN [RMM] RMMServerCommand, tx cmd to %s error %s\n",
                   g_apSWCObject[swcidx]->m_acSWCName, errBuff);
            #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMSVRCMD)
          }
        }
        break;

      case SERVER_CMD_INHIBIT_SWC_POLLING:
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM)
        printf("[RMM] RMMServerCommand, rx SERVER_CMD_ENABLE_SWC_POLLING "
               "(0x%04x) for SWC 0x%02x\n",
               SERVER_CMD_INHIBIT_SWC_POLLING, (ServerCommand & 0x00FF));
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))

//        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_ServerCommand,
//          FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "[RMM] RMMServerCommand, %s\n"
//          "  rx specific cmd SERVER_CMD_INHIBIT_SWC_POLLING\n",
//          SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
        //printf("8\n");
        sprintf(cLog, "[RMM] RMMServerCommand, %s\n"
                "  rx specific cmd SERVER_CMD_INHIBIT_SWC_POLLING\n",
                SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
        g_pEventLog->LogMessage(cLog);


        //20100702 BC
        if((int)(STR_ByteSwap16Bit(g_unLastServerCommand )& 0x00FF) == 3)
        {
          #ifdef CFG_PRN_ERR
          printf("ERR  [RMM] RMMServerCommand, modbus exception 3\n");
          #endif // CFG_PRN_ERR
          acServerRecvBuffer[1] |= 0x80;
          acServerRecvBuffer[2] = MODBUS_EXCEPTION3;
          nReplyCMDLen = 5;
        }
        else
        {
          // 20110405 BC
          swcidx = (UINT8)(ServerCommand & 0x00FF);

          if(g_tRTUConfig.nSwcEnabled[swcidx] == E_TYPE_No)
          {
            #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMSVRCMD)
            printf("WARN [RMM] RMMServerCommand, SWC %d is not enabled\n",
                   swcidx);
            #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMSVRCMD)
            break;
          }
          rstatus =
            g_pRTUStatusTable->SendSWCPollingCommand(SWC_POLLING_INHIBIT,
                                                     swcidx);
          if(rstatus != E_ERR_Success)
          {
            #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMSVRCMD)
            ERR_GetMsgString(rstatus, errBuff);
            printf("WARN [RMM] RMMServerCommand, tx cmd to %s error %s\n",
                   g_apSWCObject[swcidx]->m_acSWCName, errBuff);
            #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMSVRCMD)
          }
        }
        break;

      default:
        #ifdef CFG_PRN_ERR
        printf("ERR  [RMM] RMMServerCommand, undefined RMM command\n");
        #endif // CFG_PRN_ERR
//        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_ServerCommand,
//          FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [RMM] RMMServerCommand, %s\n"
//          "  rx undefined RMM command\n",
//          SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));

        sprintf(cLog,"ERR  [RMM] RMMServerCommand, %s\n"
                "  rx undefined RMM command\n",
                SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
        g_pEventLog->LogMessage(cLog);
        acServerRecvBuffer[1] |= 0x80;
        acServerRecvBuffer[2] = MODBUS_EXCEPTION3;    //Invalid command.
        nReplyCMDLen = 5;
        break;
      }
    }
    else
    {
      acServerRecvBuffer[1] |= 0x80;
      acServerRecvBuffer[2] = MODBUS_EXCEPTION2;  //Address error
      nReplyCMDLen = 5;
    }
  }
  else  //Function code error.
  {
    acServerRecvBuffer[1] |= 0x80;
    acServerRecvBuffer[2] = MODBUS_EXCEPTION1;
    nReplyCMDLen = 5;
  }

  UINT16 returnTmp =
    STR_ByteSwap16Bit(ModbusCRC((unsigned char *)acServerRecvBuffer,nReplyCMDLen -2));

  //prepare data  for replying to server
  *(unsigned short *)&acServerRecvBuffer[nReplyCMDLen -2] = returnTmp;
  g_pRMMServerSocket->Send(acServerRecvBuffer, nReplyCMDLen, 0);
  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
  printf("[RMM] RMMServerCommand, tx %d bytes for specific command "
         "0x%04x to server:\n", nReplyCMDLen, ServerCommand);
  SYS_PrnDataBlock((const UINT8 *)acServerRecvBuffer, nReplyCMDLen, 10);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
//  pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_ServerCommand,
//    FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//    "[RMM] RMMServerCommand, %s\n"
//    "  tx acknowledgement %d bytes:\n",
//    SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)), nReplyCMDLen);
//  pFLG_CtrlBlk->SendDataBlock(E_FLG_MSG_RMM_ServerCommand,
//    FLG_LOG_T_Event, (UINT8 *)acServerRecvBuffer, nReplyCMDLen, 20);


  for(nI=0; nI<nReplyCMDLen; nI++)
  sprintf(&acTemp1[nI*2], "%02X",(unsigned char)acServerRecvBuffer[nI]);
  sprintf(cLog,"[RMM] %s: tx acknowledgement %d bytes:%s\n",
		    SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),nReplyCMDLen,acTemp1);
  g_pEventLog->LogMessage(cLog);
  //LogServerCommand(acServerRecvBuffer, nReplyCMDLen,
    //               (CHAR *)"RMM Send to server: ");
}// RMMServerCommand

/**********************************************************************************
Purpose:
  Process error server command.

Input/Output
  nModbusError    --  Modbus error code. (in)
  acServerRecvBuffer  --  The buffer for receiving command sent server. (in)

***********************************************************************************/
void RMMServerCommandError(int nModbusError, char *acServerRecvBuffer)
{
//  char acTemp[1024];

  switch(nModbusError)
  {
  case MODBUS_ERROR_MESSAGE_LEN:
  case MODBUS_ERROR_SLAVE_ADD:
  case MODBUS_ERROR_CRC:
  default:      //040310 Yu Wei
    //No response.
    /*if(g_tDebugLogFlag.bServerCommand == true)//20050414
    {
      sprintf(acTemp,"RMM Receive a exception message (error code: %04X)\n",nModbusError);
      g_pDebugLog->LogMessage(acTemp);
    }*/
    break;

  case MODBUS_ERROR_EXCEPTION1:
    acServerRecvBuffer[1] |= 0x80;
    acServerRecvBuffer[2] = 0x01;
    *(unsigned short *)&acServerRecvBuffer[3] = STR_ByteSwap16Bit(ModbusCRC((unsigned char *)acServerRecvBuffer,3));
    g_pRMMServerSocket->Send(acServerRecvBuffer,5,0);

    //LogServerCommand(acServerRecvBuffer, 5, (char *)"RMM Send to server: ");//040310 Yu Wei
    break;

  case MODBUS_ERROR_EXCEPTION3:
    acServerRecvBuffer[1] |= 0x80;
    acServerRecvBuffer[2] = 0x03;
    *(unsigned short *)&acServerRecvBuffer[3] = STR_ByteSwap16Bit(ModbusCRC((unsigned char *)acServerRecvBuffer,3));
    g_pRMMServerSocket->Send(acServerRecvBuffer,5,0);

    //LogServerCommand(acServerRecvBuffer, 5, (char *)"RMM Send to server: "); //040310 Yu Wei
    break;
  }
}

/**********************************************************************************
Purpose:
  Server Link listen task.

Return:
  Nil.
**********************************************************************************/
void * RMMServerSocketTask(void *arg)
{
  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
  printf("[RMM] RMMServerSocketTask, start listen thread\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
  delay(100);
  g_pRMMServerSocket->ListenTask();

  #ifdef CFG_PRN_ERR
  printf("ERR  [RMM] RMMServerSocketTask, thread exit\n");
  #endif // CFG_PRN_ERR
  return NULL;
}


/*******************************************************************************
Purpose:
  CRTUStatus constructor. It initializes RTU status table parameters.

*******************************************************************************/
CRTUStatus::CRTUStatus(void)
{
  int nI;
  struct mq_attr mqattr;
  pthread_mutexattr_t * attr = NULL ;

  memset(&mqattr, 0, sizeof(mqattr));
  mqattr.mq_maxmsg = MAX_RMM_MSGQ;
  mqattr.mq_msgsize = RMM_MESSAGE_SIZE;

  ClearVariables();  //040323 Yu Wei
  m_nServerRTUStatusTableLength  =
    g_tRTUConfig.tRTUStatusTable.unTableEndAddress -
    g_tRTUConfig.tRTUStatusTable.unTableStartAddress +1;

  if(m_nServerRTUStatusTableLength > RTU_STATUS_TABLE_LENGTH_MAX)
    m_nServerRTUStatusTableLength = RTU_STATUS_TABLE_LENGTH_MAX;


  attr = (pthread_mutexattr_t *) malloc(sizeof(pthread_mutexattr_t));
  pthread_mutexattr_init( attr );
  if(pthread_mutex_init(& m_tAccessRTUStatusTableSemID, attr ) != EOK )
  {
    perror("RMM mutex init");
  }

  free(attr);
//  m_tRMM_MSGQ_ID = mq_open(RMM_MSGQ, O_RDWR|O_CREAT|  O_NONBLOCK  , S_IRUSR|S_IWUSR,&mqattr);
  m_tRMM_MSGQ_ID = mq_open(RMM_MSGQ, O_RDWR|O_CREAT|  O_NONBLOCK  ,
                           S_IRUSR|S_IWUSR, NULL);
  if(m_tRMM_MSGQ_ID<0)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [RMM] CRTUStatus::CRTUStatus, m_tRMM_MSGQ_ID create fail\n");
    #endif // CFG_PRN_ERR
  }

  //RTU identification.
  m_aunLocalRTUStatus[3]  = g_tRTUConfig.unRTUIdentification;
  m_aunLocalRTUStatus[3] = STR_ByteSwap16Bit(g_tRTUConfig.unRTUIdentification);

  #ifdef CFG_DEBUG_MSG
  printf("[RMM] CRTUStatus, configured RTU location: %s\n",
          g_tRTUConfig.acRTULocalion);
  #endif // CFG_DEBUG_MSG

  //RTU location.
  m_aunLocalRTUStatus[4]  =
    ((((unsigned short)(g_tRTUConfig.acRTULocalion[0])))|
    (((unsigned short)g_tRTUConfig.acRTULocalion[1]) << 8));
  m_aunLocalRTUStatus[5]  =
    ((((unsigned short)(g_tRTUConfig.acRTULocalion[2])))|
    (((unsigned short)g_tRTUConfig.acRTULocalion[3]) << 8 ));
//  (unsigned long)m_aunLocalRTUStatus[4] = ((((unsigned long)g_tRTUConfig.acRTULocalion[3])<< 24)|
//                       (((unsigned long)g_tRTUConfig.acRTULocalion[2])<< 16)|
//                       (((unsigned long)g_tRTUConfig.acRTULocalion[1])<< 8 )|
//                       (((unsigned long)g_tRTUConfig.acRTULocalion[0])     )      );
//
  m_aunLocalRTUStatus[6] = 0;    //RTU status.
  m_aunLocalRTUStatus[7] = 0;    //RTU notation.

  m_aunLocalRTUStatus[8] = 0;      //Not link check.
  m_aunLocalRTUStatus[9]  = g_tRTUConfig.unVersion;  //Configuration file version.
  m_aunLocalRTUStatus[9] = STR_ByteSwap16Bit(m_aunLocalRTUStatus[9]);

  m_aunLocalRTUStatus[10]  = g_tRTUPerPara.unCFGDownloadUTCHigh;  //Latest UTC time (high word) for config.txt downloading.
  m_aunLocalRTUStatus[10] = STR_ByteSwap16Bit(m_aunLocalRTUStatus[10]);
  m_aunLocalRTUStatus[11]  = g_tRTUPerPara.unCFGDownloadUTCLow;  //Latest UTC time (low word) for config.txt downloading.
  m_aunLocalRTUStatus[11] = STR_ByteSwap16Bit(m_aunLocalRTUStatus[11]);

  m_aunLocalRTUStatus[12]  = VERSION_NUMBER;  //RTU software version number.
  m_aunLocalRTUStatus[12] = STR_ByteSwap16Bit(m_aunLocalRTUStatus[12]);

  for(nI=0; nI<SWC_MAX_NUMBER; nI++)
  {
    m_aunLocalRTUStatus[nI+RTU_STATUS_TABLE_SWC_OFFSET] = 0;  //Default link status is down.
    m_aunSWCLinkCheckIndex[nI] = 0;
    m_abSWCLinkCheckFlag[nI] = false;
  }

  m_unSWCWeight = 0;
  m_unOtherSWCWeight = 0;
  m_unOtherSWCWeightPrev = 10000;
  m_unOtherRTUServerLinkStatusPrev = SERVER_LINK_OK;
  m_unOtherRTUServerLinkStatus = LINK_FAULTY;
  m_nOtherRTUStateFlag = 0;

  m_bSWCLinkDownProcessFlag = false;
  m_bSWCLinkCheckTotalFlag = false;

  m_bServerSwitchFlag = false;

  m_bSWCLinkDownFlag = false;  //040302 Yu Wei

  m_bWaitLinkCheckCompletedFlag = false;  //040311 Yu Wei

  m_bSWCSwitchCompletedFlag = false;    //040323 Yu Wei
  m_bSendSWCTableCompletedFlag = false;  //040323 Yu Wei
  m_bStartSendSWCTableFlag = false;    //040323 Yu Wei

  m_bOtherRTUSwitchFlag = false;      //040419 Yu Wei

  m_nSwitchFlag = NOT_IN_SWITCH;      //040420 Yu Wei


  m_nMultiNodeLinkStatusAdd = 0;        //2008
  m_aunMultiNodeLinkStatus = 0;      //2008

#ifdef ModuleTest
  LogRmmStatusValues();  //for test.
#endif

}

/**********************************************************************************
Purpose:
  CRTUStatus destructor.

**********************************************************************************/
CRTUStatus::~CRTUStatus(void)
{
}

/**********************************************************************************
Purpose:
  Clear object variables.

***********************************************************************************/
void CRTUStatus::ClearVariables(void)
{
  m_bServerSwitchFlag = false;      //Done by constructor. 040419 Yu Wei
  m_nServerRTUStatusTableLength = 0;  //Done by constructor. 040419 Yu Wei
//  m_tAccessRTUStatusTableSemID = 0;  //Done by constructor. 040419 Yu Wei
  pthread_mutex_destroy( &m_tAccessRTUStatusTableSemID );
  m_tRMM_MSGQ_ID = NULL;        //Done by constructor. 040419 Yu Wei
  m_bSWCSwitchCompletedFlag = false;  //Done by constructor. 040419 Yu Wei
  m_bSendSWCTableCompletedFlag = false;  //Done by constructor. 040419 Yu Wei
  m_bStartSendSWCTableFlag = false;    //Done by constructor. 040419 Yu Wei
  memset (m_aunLocalRTUStatus, 0,
          sizeof(unsigned short)*RTU_STATUS_TABLE_LENGTH_MAX);
  memset (m_aunOtherRTUStatus, 0,
          sizeof(unsigned short)*RTU_STATUS_TABLE_LENGTH_MAX);
  memset (m_aunSWCLinkCheckIndex, 0,
          sizeof(unsigned short)*SWC_MAX_NUMBER);
                      //Done by constructor. 040419 Yu Wei
  m_tStartTime.tv_sec = 0;
  m_tStartTime.tv_nsec = 0;
  m_unSWCWeight = 0;          //Done by constructor. 040419 Yu Wei
  m_unOtherSWCWeight = 0;        //Done by constructor. 040419 Yu Wei
  m_unOtherSWCWeightPrev = 0;      //Done by constructor. 040419 Yu Wei
  m_unOtherRTUServerLinkStatus = 0;    //Done by constructor. 040419 Yu Wei
  m_unOtherRTUServerLinkStatusPrev = 0;  //Done by constructor. 040419 Yu Wei
  m_nOtherRTUStateFlag = 0;        //Done by constructor. 040419 Yu Wei
  m_bSWCLinkDownProcessFlag = false;  //Done by constructor. 040419 Yu Wei
  m_bSWCLinkDownFlag = false;      //Done by constructor. 040419 Yu Wei
}
/**********************************************************************************
Purpose:
  Copy RTU Status table into server/ other RTU polling table.

Input/Output
  aunServerTable  -- The buffer for server polling table. (out)
  nTimeoutType  -- Semiphone timeout. (in)

Return:
  OK    -- Copy successfully.
  ERROR  -- The table is accessed by other task.
**********************************************************************************/
int CRTUStatus::ReadRTUStatus(unsigned short *aunServerTable, int nTimeoutType)
{
  int nI = OK;


  #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMM)
  printf("[RMM] CRTUStatus::ReadRTUStatus, read local RTU status table %d\n",
         m_nServerRTUStatusTableLength*2);
  SYS_PrnDataBlock((const UINT8 *) m_aunLocalRTUStatus,
                   m_nServerRTUStatusTableLength*2, 10);
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
  pthread_mutex_lock(&m_tAccessRTUStatusTableSemID);
  if(memcpy(aunServerTable,m_aunLocalRTUStatus,m_nServerRTUStatusTableLength*2)
     == TYP_NULL)
  {
    nI = ERROR;
  }
  //20140604 Yang Tao
// if (Testflag==1)
// {
//  UpdateRTUStatus(0x0000, RTU_STATUS_TABLE_SWC_LINK_CHECK);
//  for(nI=0; nI<18; nI++)	//20130906 Yang Tao
//    {
//      m_aunSWCLinkCheckIndex[nI] = 0;
//    }
// }
//  Testflag=0;
  pthread_mutex_unlock(&m_tAccessRTUStatusTableSemID);

  return nI;
}

/*******************************************************************************
Purpose:
  Update RTU Status table.

Input
  unStatus  -- Status. (in)
  nAddress  -- The offset of the status word in the RTU status table. (in)

Return:
  Nil.
*******************************************************************************/
void CRTUStatus::UpdateRTUStatus(unsigned short unStatus, int nAddress)
{
  struct timespec tTime;

  #if ((defined CFG_DEBUG_MSG) && (defined _CFG_DEBUG_RMM))
  printf("[RMM] CRTUStatus::UpdateRTUStatus, status: 0x%04x, "
         "addr: 0x%x\n", unStatus, nAddress);
  #endif // CFG_DEBUG_MSG

  clock_gettime(CLOCK_REALTIME, &tTime);  //Get time

  //Convert local time to UTC time. 040329 Yu Wei
  pthread_mutex_lock(&m_tAccessRTUStatusTableSemID);
  // Set time stamp
  m_aunLocalRTUStatus[0] = STR_ByteSwap16Bit((unsigned short)(tTime.tv_sec>>16));
  m_aunLocalRTUStatus[1] = STR_ByteSwap16Bit((unsigned short)(tTime.tv_sec));
  m_aunLocalRTUStatus[2] = STR_ByteSwap16Bit((unsigned short)(tTime.tv_nsec/1000000));
  m_aunLocalRTUStatus[nAddress] = unStatus;
  pthread_mutex_unlock(&m_tAccessRTUStatusTableSemID);

  //if(nAddress == 16) printf("----RTU table CLOCK = %X----\n", unStatus);
}

/*******************************************************************************
Purpose
 Update RTU health status 16-bit value for the RTU Status Table. The 16-bit RTU
status value is located at word 6 of the RTU Status Table starting from time
stamp location. The status bit is defined as follows,

  Bit No  Value   Description
  ------  -----   -----------
  00      1       The RTU has just reset (After reset, this bit is set to 1
                  within 10s period)
  01      1       Over temperature
          0       Normal temperature
  02      1       Download is required
  03      1       The RTU is switching
  04      1       Primary RTU
          0       Not primary RTU
  05      1       Standby RTU
          0       Not standby RTU
  06      1       Compact flash card error enable when lower than minimum
                  memory space
  07      1       The RTU LAN link is healthy
          0       The RTU LAN link is faulty
  08      1       The communication between RTU1 and RTU2 on LAN2 link is
                  healthy
          0       The communication between RTU1 and RTU2 on LAN2 link is
                  faulty
  09      1       The communication between RTU1 and RTU2 on LAN1 link is
                  healthy
          0       The communication between RTU1 and RTU2 on LAN1 link is
                  faulty
  10      1       A RTU Maintenance Terminal (RMT) is connected
  11      1       RTU - Server Socket ID1 link is connected
  12      1       RTU - Server Socket ID2 link is connected
  13      1       RTU - Server Socket ID3 link is connected
  14      1       RTU - Server Socket ID4 link is connected
  15      1       RTU - Server Socket ID5 link is connected

Input/Output
  None

Return
  Swap byte 16-bit RTU Status in RTU Status table

History
  Name         Date          Remark
  ----         ----          ------
  Yu Wei       22-Mar-2004   Create initial revision
  Bryan Chong  11-Feb-2010   Added temperature and disk space size monitor to
                             RTU Status Table
  Bryan Chong  22-Feb-2010   Add LED indication for polling server link status.
                             Update to turn on Compact Flash Card error bit when
                             fail to write log file.
  Bryan Chong  15-Mar-2010   Update to display state on LCD screen
  Bryan Chong  09-Apr-2010   Change bit location 5 to 6 for
                             STR_SET_BIT_16BIT_REG under uldiskspace check
                             [PR46]
  Bryan Chong  26-Oct-2010   Log overtemperature once when reading exceed
                             CPU_MAX_TEMPERATURE. The logging is controlled by
                             pFLG_CtrlBlk->CtrlBlk.isOverTemperatureLog flag
*******************************************************************************/
UINT16 CRTUStatus::CheckRTUStatus(void)
{
  E_ERR_T rstatus = E_ERR_Success;
  int nI;
  unsigned short unRTUStatus = 0;
  struct timespec tTime;
  INT16  stemperatureInCelcius;
  UINT64 uldiskspace = 0;
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};
  char cLog[1000];
  //Get current time
  clock_gettime(CLOCK_REALTIME, &tTime);

  if((tTime.tv_sec - m_tStartTime.tv_sec) <10)
    unRTUStatus |= 0x0001;

  // Check temperature
  rstatus = HWP_TPS_GetCPUTemperature(&stemperatureInCelcius);
  if(rstatus != E_ERR_Success)
  {
    #ifdef CFG_PRN_WARN
    printf("WARN [RMM] CRTUStatus::CheckRTUStatus, get temperature fail\n");
    #endif // CFG_PRN_WARN
  }
  if((rstatus == E_ERR_Success) &&
     (stemperatureInCelcius > g_tRTUConfig.ncpuMaxTemperature))
  {
    unRTUStatus = STR_SET_BIT_16BIT_REG(&unRTUStatus, 1, 1);

    // log when temperature is 5 degrees different from the previous reading
    if(g_OverTemperature== false)
    {
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_SystemAlert,
//          FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "WARN [RMM] CRTUStatus::CheckRTUStatus, %s\n"
//          "  current temperature %d exceed max temperature %d\n",
//          SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
//          stemperatureInCelcius, g_tRTUConfig.ncpuMaxTemperature);

      sprintf(cLog,"WARN [RMM] CRTUStatus::CheckRTUStatus, %s\n"
              "  current temperature %d exceed max temperature %d\n",
              SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
              stemperatureInCelcius, g_tRTUConfig.ncpuMaxTemperature);
      g_pEventLog->LogMessage(cLog);

      g_OverTemperature= true;
    }
  }

  if((rstatus == E_ERR_Success) &&
     (stemperatureInCelcius <= g_tRTUConfig.ncpuMaxTemperature) &&
     (g_OverTemperature= true))
  {
	  g_OverTemperature= false;
  }

  if((rstatus == E_ERR_Success) &&
     (stemperatureInCelcius <= g_tRTUConfig.ncpuMaxTemperature))
  {
    unRTUStatus = STR_SET_BIT_16BIT_REG(&unRTUStatus, 1, 0);
  }

  // Check for minimum disk memory space
  rstatus = SYS_GetDiskMemorySpace(&uldiskspace);
  if(rstatus != E_ERR_Success)
  {
    #ifdef CFG_PRN_WARN
    printf("WARN [RMM] CRTUStatus::CheckRTUStatus, get disk space fail\n");
    #endif // CFG_PRN_WARN
  }

  // CF Card error bit turn on when free memory space is below minimum defined
  // level or fail to write log file [PR46]
  if(uldiskspace < (g_tRTUConfig.nminFreeMemorySpace * 1e6))
  {
    unRTUStatus = STR_SET_BIT_16BIT_REG(&unRTUStatus, 6, 1);
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_SystemAlert,
//        FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//        "[RMM] CRTUStatus::CheckRTUStatus, current disk space %d is less "
//        "than min %d MB\n",
//        uldiskspace, g_tRTUConfig.nminFreeMemorySpace);
  //  printf("12\n");
    sprintf(cLog,"[RMM] CRTUStatus::CheckRTUStatus, current disk space %d is less "
            "than min %d MB\n",
            uldiskspace, g_tRTUConfig.nminFreeMemorySpace);
    g_pEventLog->LogMessage(cLog);
  }

  // CF Card error bit turn off when free memory space is above minimum defined
  // level and no issue on writing log file
  if(uldiskspace >= (g_tRTUConfig.nminFreeMemorySpace * 1e6))
  {
    unRTUStatus = STR_SET_BIT_16BIT_REG(&unRTUStatus, 6, 0);
  }

  //Check if download is required
  if(g_tRTUStatus.bCFGFileDownloadRequired == true)
    unRTUStatus |= 0x0004;

  //RTU state flag.
  switch(g_tRTUStatus.nRTUStateFlag)
  {
    //If RTU is STATEFLAG_INITIALIZATION, not set this bit.
    case STATEFLAG_INITIALIZATION:
      break;

    case STATEFLAG_SWITCHING_PTOS:
      unRTUStatus |= 0x0018;
      break;

    case STATEFLAG_SWITCHING_STOP:
      unRTUStatus |= 0x0028;
      break;

    case STATEFLAG_PRIMARY:
      //Primary RTU
      unRTUStatus |= 0x0010;
      break;

    case STATEFLAG_STANDBY:
      //Standby RTU
      unRTUStatus |= 0x0020;
      break;

    default:
      //Other state clear switching, primary, standby flags.
      unRTUStatus &= 0xFFC7;
      break;
  }

  //LAN link 1, connect other RTU through LAN port 2
 {
	 if(g_tRTUStatus.abOtherLinkStatus[0] == true) // 20150402: CheckRTUStatus, RTU Table LAN2 update
  {
    unRTUStatus |= OTHER_RTU_LINK1_OK;
  }

  //LAN link 2, connect other RTU through LAN port 1
  if(g_tRTUStatus.abOtherLinkStatus[1] == true)
  {
    unRTUStatus |= OTHER_RTU_LINK2_OK;
  }
  }


  //Server link status.
  g_tRTUStatus.unRTUServerLinkStatus = LINK_FAULTY;
  for(nI=0; nI < g_tRTUConfig.nRTUPollingSocketNumber; nI++)
  {
    if(g_tRTUStatus.abServerLinkStatusI[nI] == true)
    {
      g_tRTUStatus.unRTUServerLinkStatus = SERVER_LINK_OK;
      //unRTUStatus |= (0x0800 << nI);
      unRTUStatus = STR_SET_BIT_16BIT_REG(&unRTUStatus, (11 + nI), E_TYPE_On);

      // turn on the respective LED
      rstatus = HWP_LED_Management(nI + 1, E_TYPE_On);
      SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
    }else{

      unRTUStatus = STR_SET_BIT_16BIT_REG(&unRTUStatus, (11 + nI), E_TYPE_Off);

      // turn off the respective LED
      rstatus = HWP_LED_Management(nI + 1, E_TYPE_Off);
      SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
    }
  }

  //RTU LAN Link status.
  unRTUStatus |= g_tRTUStatus.unRTUServerLinkStatus;
  //RMT connection.
  unRTUStatus |= g_tRTUStatus.unRTURMTLinkStatus;

  return STR_ByteSwap16Bit(unRTUStatus);
}

/*******************************************************************************
Purpose:
  When state changes to Primary from Initialization, it checks if all SWC
  complete first time link check. If all SWC completed link check at least one
  time, it sets g_bRTUInitLinkCheck = false and this routine will not implement.

Return:
  Nil.

History

  Name            Date         Remark
  ----            ----         ------
 Bryan Chong   14-Jun-2010   Update flag g_bRTUInitLinkCheck by checking enabled
                             SWC for the entire SWC list to resolve RMM
                             switching issue [PR39]
 Bryan Chong   28-Jul-2010   To enable notation update when RTU is not connected
                             to server by updating local RTU status table.
                             [PR73]
*******************************************************************************/
void CRTUStatus::RTUInitLinkCheck(void)
{
  tRMM_MSG tMessage;
  int nMessageNumber;
  int nI;
  int nSwcLinkCheckedCnt = 0;


  //Check message queue that the message is sent by other module.
  struct mq_attr  mqstat;
  mq_getattr(m_tRMM_MSGQ_ID , &mqstat);

  nMessageNumber = mq_receive(m_tRMM_MSGQ_ID, (char *)&tMessage,
                              mqstat.mq_msgsize, NULL);

//  if((nMessageNumber != 0) && (nMessageNumber != ERROR))
  if( nMessageNumber >0 )
  {
    switch(tMessage.nMessageID)
    {
      case SWC_LINK_STATUS_CHANGE:

        // 20100728 BC (Rqd by ZSL)
        UpdateRTUStatus(tMessage.unStatus,
                        tMessage.nDeviceID + RTU_STATUS_TABLE_SWC_OFFSET);
          //g_tRTUConfig.anSWCIndex[tMessage.nDeviceID] +
          //RTU_STATUS_TABLE_SWC_OFFSET);

        g_abSWCInitLinkCheck[tMessage.nDeviceID] = true;
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
        printf("[RMM] CRTUStatus::RTUInitLinkCheck, rx msgq "
               "SWC_LINK_STATUS_CHANGE.\n   Set %s init link check = %d\n",
              g_apSWCObject[tMessage.nDeviceID]->m_acSWCName,
              g_abSWCInitLinkCheck[tMessage.nDeviceID]);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
        break;

      case SERVER_POLLING_STATUS_CHANGE:
        if(tMessage.unStatus == 0)
        {
          g_tRTUStatus.abServerLinkStatusI[tMessage.nDeviceID] = false;
        }
        else
        {
          g_tRTUStatus.abServerLinkStatusI[tMessage.nDeviceID] = true;
        }
        break;

      default:
        break;
    } // switch
  }// if( nMessageNumber >0 )

  #if ((defined CFG_DEBUG_MSG) && (_CFG_DEBUG_RMM))
  // occasionally during link check, RTU will quickly swicth to use
  // CalculateWeigth in RTUStatusTableProcess
  printf("[RMM] CRTUStatus::RTUInitLinkCheck, calculate weight\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
  CalculateWeight();  //Calculate SWC weight.  //040310 Yu Wei

  //Set RTU status.
  UpdateRTUStatus(CheckRTUStatus(), RTU_STATUS_TABLE_HEALTH_STATUS);

  // [PR39]
  nSwcLinkCheckedCnt = 0;
  for(nI=0; nI<g_tRTUConfig.nSWCNumber; nI++)
  {
    if((g_tRTUConfig.nSwcEnabled[nI] == 0) &&
       (g_abSWCInitLinkCheck[nI] == false))
      continue;

    nSwcLinkCheckedCnt++;

    #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMM)
    printf("[RMM] CRTUStatus::RTUInitLinkCheck, SWC %d, nSwcLinkCheckedCnt "
           "%d\n",
           nI, nSwcLinkCheckedCnt);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
  }

  #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMM)
  printf("[RMM] CRTUStatus::RTUInitLinkCheck, nSwcLinkCheckedCnt %d "
         "g_tRTUConfig.nTotalEnableSwc %d\n",
         nSwcLinkCheckedCnt, g_tRTUConfig.nTotalEnableSwc);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))

  if(nSwcLinkCheckedCnt == g_tRTUConfig.nTotalEnableSwc)
  {
    g_bRTUInitLinkCheck = false;
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM)
    printf("[RMM] CRTUStatus::RTUInitLinkCheck, all SWCs completed first link "
           "check\n");
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
  }

}// RTUInitLinkCheck

/**********************************************************************************
Purpose:
  Check if it received the message from other module and process the message.
  Update RTU status table.

Return:
  Nil.

History

      Name       Date              Remark
      ----       ----              ------
  Bryan Chong  17-May-2010  Update to include SWC_LINK_STATUS data structure
                            for SWC link status mapping
  Bryan Chong  28-Jul-2010  To enable notation update when RTU is not connected
                            to server by updating local RTU status table.
**********************************************************************************/
void CRTUStatus::RTUStatusTableProcess(void)
{
  tRMM_MSG tMessage;
  int nMessageNumber;
  struct mq_attr  mqstat;

  // 20100517 BC (Rqd ZSL)
  SWC_LINK_STATUS swcLinkStatus;
  // 20100630 BC
  RMM_RTU_STATUS_TBL otherRtuStatusTbl;
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};
  char cLog[1000];
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM)
  CHAR strbuff[50] = {0};
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM)

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINKCHK || CFG_DEBUG_RMM))
  E_ERR_T rstatus = E_ERR_Success;
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)

  // 20100630 BC
  memcpy(&otherRtuStatusTbl, m_aunOtherRTUStatus, sizeof(RMM_RTU_STATUS_TBL));

  //Check message queue that the message is sent by other module.
  mq_getattr(m_tRMM_MSGQ_ID , &mqstat);
  nMessageNumber = mq_receive(m_tRMM_MSGQ_ID, (char *)&tMessage,
                              mqstat.mq_msgsize, NULL);

  swcLinkStatus.allbits = STR_ByteSwap16Bit(tMessage.unStatus);
  if(nMessageNumber >0)
  {
    switch(tMessage.nMessageID)
    {
      case SWC_LINK_STATUS_CHANGE: //0
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
        printf("[RMM] CRTUStatus::RTUStatusTableProcess, stdby RTU rx msgq "
               "from %s SWC_LINK_STATUS_CHANGE\n",
               g_apSWCObject[tMessage.nDeviceID]->m_acSWCName);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))

        if(g_tRTUStatus.nRTUStateFlag == STATEFLAG_STANDBY)
        {
          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
          printf("[RMM] CRTUStatus::RTUStatusTableProcess, stdby RTU rx cmd "
                 "from %s, change status to 0x%04x\n",
                 g_apSWCObject[tMessage.nDeviceID]->m_acSWCName,
                 STR_ByteSwap16Bit(tMessage.unStatus));
          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
          // update standby RTU SWC link status according to primary RTU's
          //otherRtuStatusTbl.swcCommStatus[tMessage.nDeviceID].allbits =
            //tMessage.unStatus;

          // 20100728 BC (Rqd by ZSL)
          UpdateRTUStatus(tMessage.unStatus,
                          tMessage.nDeviceID + RTU_STATUS_TABLE_SWC_OFFSET);

        }else{

          // 20100728 BC (Rqd by ZSL)
          UpdateRTUStatus(tMessage.unStatus,
                          tMessage.nDeviceID + RTU_STATUS_TABLE_SWC_OFFSET);

          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM)
//          rstatus = RMM_GetStateString(g_tRTUStatus.nRTUStateFlag,
//                                                 strbuff, sizeof(strbuff));
          printf("[RMM] CRTUStatus::RTUStatusTableProcess, %s RTU rx cmd, "
                 "update %s status 0x%04x to Server Table word %d\n",
                 strbuff, g_apSWCObject[tMessage.nDeviceID]->m_acSWCName,
                 STR_ByteSwap16Bit(tMessage.unStatus),
                 tMessage.nDeviceID + RTU_STATUS_TABLE_SWC_OFFSET);
          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))

          if(g_tRTUConfig.tSWC[tMessage.nDeviceID].tHeader.nNotation == 0)
            break;

          // 0x8000 Link down. 0x0004 Link enable. Primary RTU's SWC link down.
          // Either RTU-RTU LAN Link is OK. 040311 Yu Wei
          // 20100517 BC (Rqd by ZSL)
          if((swcLinkStatus.bitctrl.isCommunicationValid == E_TYPE_False) &&
             (swcLinkStatus.bitctrl.isSWCEnable == E_TYPE_True) &&
             (g_tRTUStatus.nRTUStateFlag == STATEFLAG_PRIMARY) &&
             ((g_tRTUStatus.abOtherLinkStatus[0] == true) ||
              (g_tRTUStatus.abOtherLinkStatus[1] == true)))
          {
            #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
            rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                                 E_SYS_TIME_Local);
            printf("[RMM] %d.%02d.%02d %02d:%02d:%02d.%03d "
                   "CRTUStatus::RTUStatusTableProcess,\n"
                   "  SWC_LINK_STATUS_CHANGE to 0x%04x by %s\n"
                   "  pri RTU stops all SWC links, preparing link check for "
                   "other RTU\n"
                   "  set m_bSWCLinkDownProcessFlag, m_bSWCLinkCheckTotalFlag; "
                   "reset m_bWaitLinkCheckCompletedFlag\n"
                   "  Pri RTU link check start... \n",
                   (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
                   pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
                   pbdtime->tm_sec, tspec.tv_nsec/1000000,
                   STR_ByteSwap16Bit(tMessage.unStatus),
                   g_apSWCObject[tMessage.nDeviceID]->m_acSWCName);
            #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
            //Stop all down SWC's link.
            SetSWCLinkStop();
            m_bSWCLinkDownProcessFlag = true;
            m_bSWCLinkCheckTotalFlag = true;
            m_bWaitLinkCheckCompletedFlag = false;

//            pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_StateChange,
//              FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec,
//              FLG_LOG_T_Debug,
//              "[RMM] CRTUStatus::RTUStatusTableProcess, %s\n"
//              "  %s trigger SWC_LINK_STATUS_CHANGE to 0x%04x\n",
//              SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
//              g_apSWCObject[tMessage.nDeviceID]->m_acSWCName,
//              STR_ByteSwap16Bit(tMessage.unStatus),
//              g_apSWCObject[tMessage.nDeviceID]->m_acSWCName);
//            printf("13\n");

//            sprintf(cLog, "[RMM] CRTUStatus::RTUStatusTableProcess, %s\n   %s trigger SWC_LINK_STATUS_CHANGE to 0x%04x\n",
//                    SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
//                    g_apSWCObject[tMessage.nDeviceID]->m_acSWCName,
//                    STR_ByteSwap16Bit(tMessage.unStatus),
//                    g_apSWCObject[tMessage.nDeviceID]->m_acSWCName);
//            g_pDebugLog->LogMessage(cLog);

            clock_gettime(CLOCK_REALTIME, &g_pRTUStatusTable->m_linkChkStart);
            g_pRTUStatusTable->m_linkChkStop.tv_sec = 0;
            g_pRTUStatusTable->m_linkChkStop.tv_nsec = 0;
          }
        }
        break;

      case SWC_MESSAGE_REPLY_LINK_STOP:
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
        printf("[RMM] CRTUStatus::RTUStatusTableProcess, stdby RTU rx msgq "
               "SWC_MESSAGE_REPLY_LINK_STOP\n");
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
        if(m_bSWCLinkDownProcessFlag == true)
        {
          m_abSWCLinkCheckFlag[tMessage.nDeviceID] = false;
        }
        break;

      //Standby RTU SWC link check completed. //message ID = 2
      case SWC_MESSAGE_REPLY_LINK_CHECK:
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
        rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                             E_SYS_TIME_Local);
        printf("[RMM] %d.%02d.%02d %02d:%02d:%02d.%03d "
               "CRTUStatus::RTUStatusTableProcess, stdby RTU rx msgq "
               "SWC_MESSAGE_REPLY_LINK_CHECK, reset %s "
               "m_abSWCLinkCheckFlag[%d], update SWC status 0x%04x\n",
               (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
               pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
               pbdtime->tm_sec, tspec.tv_nsec/1000000,
               g_apSWCObject[tMessage.nDeviceID]->m_acSWCName,
               tMessage.nDeviceID, STR_ByteSwap16Bit(tMessage.unStatus));
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)

        // 20100805 BC (Rqd by ZSL)
        UpdateRTUStatus(tMessage.unStatus,
                        tMessage.nDeviceID + RTU_STATUS_TABLE_SWC_OFFSET);
        m_abSWCLinkCheckFlag[tMessage.nDeviceID] = false;
        #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMMLINKCHK)
        printf("[RMM] CRTUStatus::RTUStatusTableProcess, updated local RTU "
               "%s status tbl:\n",
               g_apSWCObject[tMessage.nDeviceID]->m_acSWCName);
        SYS_PrnDataBlock((const UINT8 *) m_aunLocalRTUStatus,
                         sizeof(RMM_RTU_STATUS_TBL), 20);
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
        break;

      case SERVER_POLLING_STATUS_CHANGE:
        #if ((defined CFG_DEBUG_MSG) && (defined _CFG_DEBUG_RMM))
        printf("[RMM] CRTUStatus::RTUStatusTableProcess, rx msgq "
               "SERVER_POLLING_STATUS_CHANGE\n");
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))

        if(tMessage.unStatus == 0)
        {
          #if ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_RMM))
          printf("[RMM] CRTUStatus::RTUStatusTableProcess, detect "
                 "server link down\n");
          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
          g_tRTUStatus.abServerLinkStatusI[tMessage.nDeviceID] = false;
        }
        else
        {
          g_tRTUStatus.abServerLinkStatusI[tMessage.nDeviceID] = true;
        }
        break;

      case MMM_STATE_CHANGE:
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
        printf("[RMM] CRTUStatus::RTUStatusTableProcess, rx msgq "
               "MMM_STATE_CHANGE\n");
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
        ChangeStateTo(tMessage.unStatus);
        break;

      default:      //Do nothing  040310 Yu Wei
        #ifdef CFG_PRN_ERR
        printf("ERR  [RMM] CRTUStatus::RTUStatusTableProcess, rx msgq "
               "invalid message from SWC %d",
               tMessage.nDeviceID);
        #endif // CFG_PRN_ERR
        break;
     }// switch
  }// if(nMessageNumber >0)

  #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMM)
  // most of the time using RTUStatusTableProcess
  printf("[RMM] CRTUStatus::RTUStatusTableProcess, calculate weight\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
  //Calculate SWC weight.  //040310 Yu Wei
  CalculateWeight();

  //Set RTU status.
  UpdateRTUStatus(CheckRTUStatus(), RTU_STATUS_TABLE_HEALTH_STATUS);

  SWCLinkCheckProcess();    //040310 Yu Wei

}// RTUStatusTableProcess

/**********************************************************************************
Purpose:
  Standby RTU check if link checking completed when SWC(Primary) link down.
  Primary RTU check if all link down SWC's serial link are stopped.
  Added in 10 March 2004, Yu Wei

Return:
  Nil.
**********************************************************************************/
void CRTUStatus::SWCLinkCheckProcess(void)
{
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
  E_ERR_T rstatus = E_ERR_Success;
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)

  //Standby don't need to check m_bSWCLinkCheckTotalFlag
  if(g_tRTUStatus.nRTUStateFlag == STATEFLAG_STANDBY)
  {
    //Based on this flag to check SWCLink completing.
    //if((m_aunLocalRTUStatus[RTU_STATUS_TABLE_SWC_LINK_CHECK] & 0x0001) != 0)
    if(m_aunLocalRTUStatus[RTU_STATUS_TABLE_SWC_LINK_CHECK] != 0)
       //(m_bWaitLinkCheckCompletedFlag == 0))
    {
      #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMMLINKCHK)
      printf("[RMM] CRTUStatus::SWCLinkCheckProcess, set link complete "
             "m_bSWCLinkCheckTotalFlag %d, m_bWaitLinkCheckCompletedFlag %d\n",
             m_bSWCLinkCheckTotalFlag, m_bWaitLinkCheckCompletedFlag);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
      SetLinkCheckCompleted();
    }
    //20120321 BC
    if((m_aunOtherRTUStatus[RTU_STATUS_TABLE_SWC_LINK_CHECK] == 0) &&
       (m_bWaitLinkCheckCompletedFlag == E_TYPE_Yes))
    {
      m_bWaitLinkCheckCompletedFlag = E_TYPE_No;
      m_bSWCLinkCheckTotalFlag = E_TYPE_No;

      clock_gettime(CLOCK_REALTIME, &m_linkChkStop);
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
      rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                           E_SYS_TIME_Local);
      printf("[RMM] %d.%02d.%02d %02d.%02d.%02d.%03d "
             "CRTUStatus::SWCLinkCheckProcess,\n"
             "  reset m_bWaitLinkCheckCompletedFlag, "
             "m_bSWCLinkCheckTotalFlag\n"
             "  Stdby RTU link check stop...\n",
             (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1), pbdtime->tm_mday,
              pbdtime->tm_hour, pbdtime->tm_min, pbdtime->tm_sec,
             (UINT32)(tspec.tv_nsec/1e6));
      #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
    }
  }
  else if(m_bSWCLinkCheckTotalFlag == true)  //SWC link checking set.
  {
    if(CheckSWCLinkStop() == true)
    {
      // when all SWCs' Link check have completed and stopped
      m_bSWCLinkCheckTotalFlag = false;

      //RTU link is OK
      if((g_tRTUStatus.abOtherLinkStatus[0] == false) &&
         (g_tRTUStatus.abOtherLinkStatus[1] == false))
      {
        SetSWCLinkStart();
      }
    }
  }
}

/**********************************************************************************
Purpose:
  Send a message to this object.

Input
  tMessage  -- The structure for message. (in)

Return:
  OK.
**********************************************************************************/
int CRTUStatus::SendMsg(tRMM_MSG  tMessage)
{
  int nReturn=0;

  if(m_tRMM_MSGQ_ID == TYP_NULL)
    return TYP_ERROR;

  nReturn = mq_send(m_tRMM_MSGQ_ID, (char *)&tMessage, RMM_MESSAGE_SIZE,
                    MSG_PRI_NORMAL);

  if(nReturn < 0)
  {
     #ifdef CFG_PRN_ERR
     printf("ERR  [RMM] CRTUStatus::SendMsg, send message failed\n");
     #endif // CFG_PRN_ERR
     return nReturn;
  }

  return OK;

}// SendMsg

/*******************************************************************************
Purpose:
  Calculate SWC weight. When the weight is changed, the RTU status table will be
  updated.    //040310 Yu Wei

Return:
  Nil.

History

      Name       Date        Remark
      ----       ----        ------
  Bryan Chong  20-May-2010  Update bit check using data structure
                            RMM_RTU_STATUS_TBL upon determining total notation
  Bryan Chong  24-May-2010  Update NTP notation [PR41] and [PR42]

*******************************************************************************/
void CRTUStatus::CalculateWeight(void)  //040310 Yu Wei
{
  unsigned short nWeight = 0;
  int nI;
  E_ERR_T rstatus = E_ERR_Success;

  #ifdef CFG_PRN_ERR
  CHAR errBuff[100] = {0};
  #endif // CFG_PRN_ERR

  RMM_RTU_STATUS_TBL statusTbl;

  //This two state will set weigth to 0. 040611 Yu Wei
  if((g_tRTUStatus.nRTUStateFlag == STATEFLAG_HARDWARETEST) ||
     (g_tRTUStatus.nRTUStateFlag == STATEFLAG_SYSTEM_RESET))
  {
    nWeight = 0;
  }
  else
  {
    rstatus = STR_SwapEndianessBuffer(&statusTbl, m_aunLocalRTUStatus,
                                      sizeof(RMM_RTU_STATUS_TBL),
                                      E_STR_BCONV_16_BIT);
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_PRN_ERR
      ERR_GetMsgString(rstatus, errBuff);
      printf("[RMM] CRTUStatus::CalculateWeight, swap byte error, %s\n",
             errBuff);
      #endif // CFG_PRN_ERR
    }

    for(nI=0; nI<SWC_MAX_NUMBER; nI++)
    {
      if(g_tRTUConfig.nSwcEnabled[nI] == false)
        continue;

      if(statusTbl.swcCommStatus[nI].bitctrl.communicationIsValid &&
         statusTbl.swcCommStatus[nI].bitctrl.isRTUToExternalSystemLinkEnable)
      {
        nWeight += g_tRTUConfig.tSWC[nI].tHeader.nNotation;
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM_NOTATION)
        printf("[RMM] CRTUStatus::CalculateWeight, %s notation = %d "
               "total notation = %d\n",
               g_apSWCObject[nI]->m_acSWCName,
               g_tRTUConfig.tSWC[nI].tHeader.nNotation,
               nWeight);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
      }

    } // for(nI=0; nI<SWC_MAX_NUMBER; nI++)

    // [PR41] and [PR42] update NTP client notation 20100524 BC (Rqd ZSL)
    if((g_tRTUConfig.nSwcEnabled[CFG_NTP_CLIENT_SWC_INDEX] == false) &&
       (statusTbl.swcCommStatus[CFG_NTP_CLIENT_SWC_INDEX].
          bitctrl.communicationIsValid) &&
        (statusTbl.swcCommStatus[CFG_NTP_CLIENT_SWC_INDEX].
          bitctrl.isRTUToExternalSystemLinkEnable))
    {
      nWeight += pCGF_CB->ntp.notation;
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM_NOTATION)
      printf("[RMM] CRTUStatus::CalculateWeight, update %s notation, "
             "ntp notation = %d, total weight = %d\n",
             g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_acSWCName,
             pCGF_CB->ntp.notation, nWeight);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
    }

  } // if((g_tRTUStatus.nRTUStateFlag == STATEFLAG_HARDWARETEST) ||
    //    (g_tRTUStatus.nRTUStateFlag == STATEFLAG_SYSTEM_RESET))


  if(nWeight != m_unSWCWeight)    //040310 Yu Wei
  {
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM_NOTATION)
    printf("[RMM] CRTUStatus::CalculateWeight, notation change from %d to %d\n",
           m_unSWCWeight, nWeight);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
    //Update when notation has changed
    m_unSWCWeight = nWeight;
    //Set SWC weight.
    UpdateRTUStatus(STR_ByteSwap16Bit(m_unSWCWeight), RTU_STATUS_TABLE_SWC_WEIGHT);
  }                  //040310 Yu Wei

}// CalculateWeight

/*******************************************************************************
Purpose:
  Copy other RTU Status table to local buffer. Update m_nOtherRTUStateFlag,
  m_unOtherSWCWeight, and m_unOtherRTUServerLinkStatus accordingly.

Input
  aunBuffer  -- The buffer for other RTU table. (in)

*******************************************************************************/
void CRTUStatus::WriteOtherRTUStatus(unsigned short *aunBuffer)
{
  RMM_RTU_HEALTH healthStatus;

  memcpy(m_aunOtherRTUStatus, aunBuffer, m_nServerRTUStatusTableLength*2);
  #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMMLINK_STATUS)
  printf("[RMM] CRTUStatus::WriteOtherRTUStatus, update m_aunOtherRTUStatus %d "
         "bytes:\n", m_nServerRTUStatusTableLength*2);
  SYS_PrnDataBlock((const UINT8 *) m_aunOtherRTUStatus,
    m_nServerRTUStatusTableLength*2, 20);
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINK_STATUS)
  healthStatus.allbits = STR_ByteSwap16Bit(
    m_aunOtherRTUStatus[RTU_STATUS_TABLE_HEALTH_STATUS]);

  if(healthStatus.bitctrl.isPrimaryRTU)
    m_nOtherRTUStateFlag = STATEFLAG_PRIMARY;
  else if(healthStatus.bitctrl.isStandbyRTU)
    m_nOtherRTUStateFlag = STATEFLAG_STANDBY;

  //tong
  //static int nOriWeight = m_unOtherSWCWeight;

  m_unOtherSWCWeight =
    STR_ByteSwap16Bit(m_aunOtherRTUStatus[RTU_STATUS_TABLE_SWC_WEIGHT]);
  m_unOtherRTUServerLinkStatus =
    STR_ByteSwap16Bit(m_aunOtherRTUStatus[RTU_STATUS_TABLE_HEALTH_STATUS]) &
    SERVER_LINK_OK ;

}// WriteOtherRTUStatus

/**********************************************************************************
Purpose:
  Set SWC link check index table. If other RTU status table's SWC link is down,
  the index table flag will be set and link check command will be sent to SWC.

History

      Name       Date        Remark
      ----       ----        ------
  Bryan Chong  24-Jun-2010  Update bit check using data structure
                            RMM_RTU_STATUS_TBL [PR49]
                            Note: Continue to check FPS m_bLinkCheckFlag alway
                                  true
  Bryan Chong  27-Apr-2011  Start to wait for link complete by setting flag
                            m_bWaitLinkCheckCompletedFlag.
  Bryan Chong  02-Dec-2011  Exclude NTPClient (SWC ID = 19) from setting
                            m_abSWCLinkCheckFlag

*******************************************************************************/
void CRTUStatus::SetSWCLinkCheckIndexTable(void)
{
  int nI;
  E_ERR_T rstatus = E_ERR_Success;
  RMM_RTU_STATUS_TBL statusTbl;

  #ifdef CFG_PRN_ERR
  CHAR errBuff[100] = {0};
  #endif // CFG_PRN_ERR

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)

  rstatus = STR_SwapEndianessBuffer(&statusTbl, m_aunOtherRTUStatus,
                                    sizeof(RMM_RTU_STATUS_TBL),
                                    E_STR_BCONV_16_BIT);
  if(rstatus != E_ERR_Success)
  {
    #ifdef CFG_PRN_ERR
    ERR_GetMsgString(rstatus, errBuff);
    printf("ERR  [RMM] CRTUStatus::SetSWCLinkCheckIndexTable, swap byte error, "
           "%s\n", errBuff);
    #endif // CFG_PRN_ERR
  }

  for(nI=0; nI<g_tRTUConfig.nSWCNumber; nI++)
  {
    if(g_tRTUConfig.nSwcEnabled[nI] == false)
      continue;

    //anSWCIndex doesn't include Clock. SWC link enable, can check link.
    //If SWC is checking link, don't check again.
    //040430 Yu Wei
    #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMMLINKCHK)
    if(m_aunSWCLinkCheckIndex[nI])
    {
      printf("[RMM] CRTUStatus::SetSWCLinkCheckIndexTable, %s %02d link status "
             "= 0x%x\n",
             g_apSWCObject[nI]->m_acSWCName,
             nI, statusTbl.swcCommStatus[nI].allbits);
      printf("  %s m_aunSWCLinkCheckIndex[%d] = %d\n",
             g_apSWCObject[nI]->m_acSWCName, nI, m_aunSWCLinkCheckIndex[nI]);
    }
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))

    // exclude NTPClient (SwcID = 19)
    if((statusTbl.swcCommStatus[nI].bitctrl.communicationIsValid ==
        E_TYPE_False) &&
       (statusTbl.swcCommStatus[nI].bitctrl.isRTUToExternalSystemLinkEnable ==
        E_TYPE_True) && (nI != 19) &&
       (m_aunSWCLinkCheckIndex[nI] == 0)
      )
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
      rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                           E_SYS_TIME_Local);
      printf("[RMM] %d.%02d.%02d %02d:%02d:%02d.%03d "
             "CRTUStatus::SetSWCLinkCheckIndexTable,\n"
             "  stdby RTU tx SWC_LINK_CHECK_STANDBY_RTU to %s\n"
             "  set m_aunSWCLinkCheckIndex[%d], m_abSWCLinkCheckFlag[%d], "
             "m_bSWCLinkCheckTotalFlag\n",
             (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
             pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
             pbdtime->tm_sec, tspec.tv_nsec/1000000,
             g_apSWCObject[nI]->m_acSWCName, nI, nI);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINKCHK))
      m_aunSWCLinkCheckIndex[nI] = 1;
      m_abSWCLinkCheckFlag[nI] = true;
      m_bSWCLinkCheckTotalFlag = true;
      SendMessageToSWC(nI, SWC_LINK_CHECK_STANDBY_RTU);
    }

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
    printf("[RMM] CRTUStatus::SetSWCLinkCheckIndexTable, %s link status "
           "= 0x%x\n"
           "  m_aunSWCLinkCheckIndex[%d] = %d\n",
           g_apSWCObject[nI]->m_acSWCName,
           statusTbl.swcCommStatus[nI].allbits, nI, m_aunSWCLinkCheckIndex[nI]);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
  }

  #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMMLINKCHK)
  printf("[RMM] CRTUStatus::SetSWCLinkCheckIndexTable, "
         "m_bSWCLinkCheckTotalFlag = %d, m_bWaitLinkCheckCompletedFlag = %d\n",
         m_bSWCLinkCheckTotalFlag, m_bWaitLinkCheckCompletedFlag);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))

  //There is SWC that link is down.
  if(m_bSWCLinkCheckTotalFlag == true)
  {
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
    printf("[RMM] CRTUStatus::SetSWCLinkCheckIndexTable, stdby RTU "
           "set status tbl link chk flag, "
           "m_bWaitLinkCheckCompletedFlag\n");
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
    //Set SWC is checking link status.

    //m_bSWCLinkCheckTotalFlag = false;  //040609 Yu Wei

    UpdateRTUStatus(0x0001, RTU_STATUS_TABLE_SWC_LINK_CHECK);
    m_bWaitLinkCheckCompletedFlag = true;
  }
}// SetSWCLinkCheckIndexTable

/**********************************************************************************
Purpose:
  Send a message to SWC module.

Input
  nSWCObjectID  --  SWC object ID.
  nMessageID    --  Message ID.
**********************************************************************************/
void CRTUStatus::SendMessageToSWC(int nSWCObjectID, int nMessageID)
{
  tSWCCommand_MSG tMessage;
  tMessage.nMessageID = nMessageID;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)

  if(g_tRTUConfig.nSwcEnabled[nSWCObjectID] == 1)
  {
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
    printf("[RMM] CRTUStatus::SendMessageToSWC, %s sending msgq to %s:\n",
           SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
           g_apSWCObject[nSWCObjectID]->m_acSWCName);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
    g_apSWCObject[nSWCObjectID]->SendMessageQ(tMessage);
  }
}// SendMessageToSWC

/*******************************************************************************
Purpose:
  Send a message using message queue to SWC module.

Parameters
  nCommand  [in] Server command.(Enable/inhibit SWC polling)
  nSWCID    [in] SWC ID.

Return
  E_ERR_Success                 Send command to message queue successfully
  E_ERR_RMM_SendMsgQueueError   Cannot send command.

History

      Name       Date        Remark
      ----       ----        ------
  Bryan Chong  10-Dec-2009   Update routine with error code
  Bryan Chong  02-Jul-2010   Update to fix handling of polling command sent
                             by server and RMT
*******************************************************************************/
E_ERR_T CRTUStatus::SendSWCPollingCommand(INT32 nCommand, INT32 nSWCID)
{
  //int nI;

  // 20100702 BC
  if(g_tRTUConfig.nSwcEnabled[nSWCID] == false)
      return E_ERR_RMM_SWCIndexNotEnable;


  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM)
  printf("[RMM] CRTUStatus::SendSWCPollingCommand, nSWCObjectID = %d, "
         "nCommand = 0x%x\n", nSWCID, nCommand);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
  if((nSWCID < g_tRTUConfig.nSWCNumber) && (nSWCID >= 0))
  {
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM)
    printf("[RMM] CRTUStatus::SendSWCPollingCommand, RTU sending cmd msgq to "
           "%s\n", g_apSWCObject[nSWCID]->m_acSWCName);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
    SendMessageToSWC(nSWCID, nCommand);
    return E_ERR_Success;

  } else {

    #ifdef CFG_PRN_ERR
    printf("ERR  [RMM] CRTUStatus::SendSWCPollingCommand, send msgq error\n");
    #endif // CFG_PRN_ERR
    return E_ERR_RMM_SendMsgQueueError;
  } // if((nSWCObjectID < g_tRTUConfig.nSWCNumber) && (nSWCObjectID >= 0))

  return E_ERR_Success;
}// SendSWCPollingCommand

/**********************************************************************************
Purpose:
  Check whether the standby RTU completed to check SWC link.

Return
  true  -- Link checking completed
  false  -- Link checking doesn't completed

History
  Name          Date        Remark
  ----          ----        ------
 Bryan Chong  06-May-2010  Update to check only enabled SWC
 Bryan Chong  27-Apr-2011  Add delay 1.25 secs to wait for link check complete
                           before resetting m_bWaitLinkCheckCompletedFlag and
                           allowing SWC status table update in link monitoring
                           process. Set m_bWaitLinkCheckCompletedFlag when
                           m_abSWCLinkCheckFlag is set
**********************************************************************************/
bool CRTUStatus::SetLinkCheckCompleted(void)
{
  int nI;
  bool nReturn = true;

  // 20100506 BC (Rqd by ZSL)
  for(nI=0; nI<g_tRTUConfig.nSWCNumber; nI++)
  {
    if(g_tRTUConfig.nSwcEnabled[nI] == false)
      continue;

    if(m_abSWCLinkCheckFlag[nI] == true)
    {
      nReturn = false;
      break;
    }
  }

  if(nReturn == true)
  {
    //SWC link checking completed.
    UpdateRTUStatus(0, RTU_STATUS_TABLE_SWC_LINK_CHECK);

    // 20100506 BC (Rqd by ZSL)
    for(nI=0; nI<g_tRTUConfig.nSWCNumber; nI++)
    {
      if(g_tRTUConfig.nSwcEnabled[nI] == E_TYPE_No)
        continue;

      //Clear flag.
      m_aunSWCLinkCheckIndex[nI] = 0;
      #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMM)
      printf("[RMM] CRTUStatus::SetLinkCheckCompleted, m_aunSWCLinkCheckIndex "
             "%d = 0\n", nI);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM)
    }

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
    printf("[RMM] CRTUStatus::SetLinkCheckCompleted,\n"
           "  clear RTU status tbl link check flag, reset all SWC link check\n"
           "  index, m_aunSWCLinkCheckIndex\n");
    //SYS_PrnDataBlock((const UINT8 *) m_aunLocalRTUStatus, 60, 20);
    #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)

    //m_bWaitLinkCheckCompletedFlag = E_TYPE_No;
    //m_bSWCLinkCheckTotalFlag = E_TYPE_No;

    //clock_gettime(CLOCK_REALTIME, &m_linkChkStop);
  }
  return nReturn;
}
/*******************************************************************************
Purpose:
  Check if the primary RTU's SWC link has completely stopped.

Return
  true  -- All SWC links are stop
  false -- Not all SWC links are stop, some links are still running

History

  Name           Date        Remark
  ----           ----        ------
 Bryan Chong  05-May-2010  Update the routine by including enabled SWC check

*******************************************************************************/
bool CRTUStatus::CheckSWCLinkStop(void)
{
  int nI;
  bool nReturn = true;

  for(nI=0; nI<g_tRTUConfig.nSWCNumber; nI++)
  {
    if(m_abSWCLinkCheckFlag[nI] == true)
    {
      nReturn = false;
      break;
    }
  }

  if(nReturn == true)
  {
    for(nI=0; nI<g_tRTUConfig.nSWCNumber; nI++)
    {
      if((m_aunSWCLinkCheckIndex[nI] == 1) &&
         ((STR_ByteSwap16Bit(m_aunLocalRTUStatus[g_tRTUConfig.anSWCIndex[nI] +
                      RTU_STATUS_TABLE_SWC_OFFSET]) & VALIDAION_BIT) != 0))
      {
        //Link is recovered self.
        m_aunSWCLinkCheckIndex[nI] = 0;
        SendMessageToSWC(nI,SWC_START_LINK_PRIMARY_RTU);
      }
      else
      {
        break;
      }
    }

    if( nI >= g_tRTUConfig.nSWCNumber)
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
      printf("[RMM] CRTUStatus::CheckSWCLinkStop, reset "
             "m_bSWCLinkDownProcessFlag\n");
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
      //All SWC down link are recovered self or link inhibit. //040430 Yu Wei
      m_bSWCLinkDownProcessFlag = false;
    }
    else
    {
      m_bSWCLinkDownFlag = true;
    }
  }

  return nReturn;
}// CheckSWCLinkStop
/*******************************************************************************
Purpose:
  This CSU will disable all SWC's TX that the link is down, when a SWC link
  status is changed to down or standby RTU's weight is greater than previous.
  For primary RTU only.

*******************************************************************************/
void CRTUStatus::SetSWCLinkStop(void)
{
  int nI;

  for(nI=0; nI<g_tRTUConfig.nSWCNumber; nI++)
  {
    if(g_tRTUConfig.nSwcEnabled[nI] == false)
      continue;
    /*
    When communication is valid (0x8000), Enable RTU - External System Link;
    when Enable RTU - External System Link (0x0004), when primary RTU link
    is not down; when the SWC is enable.
    Do not have to send stop message to all the SWC in the list.
    */
    if(((STR_ByteSwap16Bit(m_aunLocalRTUStatus[
           g_tRTUConfig.anSWCIndex[nI]+RTU_STATUS_TABLE_SWC_OFFSET]) & VALIDAION_BIT)
           == 0) &&
       ((STR_ByteSwap16Bit(m_aunLocalRTUStatus[
           g_tRTUConfig.anSWCIndex[nI]+RTU_STATUS_TABLE_SWC_OFFSET]) & 0x0004)
           != 0) &&
       (m_aunSWCLinkCheckIndex[nI] == 0) &&
       (g_tRTUConfig.nSwcEnabled[nI] == 1))
    {
      // mark primary RTU link is down, 1
      m_aunSWCLinkCheckIndex[nI] = 1;
      m_abSWCLinkCheckFlag[nI] = true;

      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
      printf("[RMM] CRTUStatus::SetSWCLinkStop, pri RTU tx msgq "
             "SWC_STOP_LINK_PRIMARY_RTU to %s\n",
             g_apSWCObject[nI]->m_acSWCName);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
      SendMessageToSWC(nI, SWC_STOP_LINK_PRIMARY_RTU);
    }
  }
}// SetSWCLinkStop

/*******************************************************************************
Purpose:
  This CSU will send check link message to all SWC that the Primary RTU's SWC link
  is down. For Standby RTU Only.

  Name           Date        Remark
  ----           ----        ------
 Bryan Chong  05-May-2010  Update the routine by including enabled SWC check

*******************************************************************************/
void CRTUStatus::SetSWCLinkStart(void)
{
  int nI;

  for(nI=0; nI<g_tRTUConfig.nSWCNumber; nI++)
  {
    if((g_tRTUConfig.nSwcEnabled[nI] == false) ||
       (g_apSWCObject[nI]->m_tMSGQID == TYP_NULL))
      continue;

    //if((m_aunSWCLinkCheckIndex[nI] == 1) || (nI == 0))
    //{
      m_aunSWCLinkCheckIndex[nI] = 0;
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
      printf("[RMM] CRTUStatus::SetSWCLinkStart, RTU send msgq to %s: "
             "SWC_START_LINK_PRIMARY_RTU\n", g_apSWCObject[nI]->m_acSWCName);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM))
      SendMessageToSWC(nI, SWC_START_LINK_PRIMARY_RTU);

    //}
  }

  m_bSWCLinkDownProcessFlag = false;
}

/**********************************************************************************
Purpose:
  This CSU will clear RMM's flags.

History
  Name          Date        Remark
  ----          ----        ------
 Bryan Chong  27-Apr-2011  Add reset m_bWaitLinkCheckCompletedFlag

**********************************************************************************/
void CRTUStatus::ClearFlag(void)
{
  int nI;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM)
  printf("[RMM] CRTUStatus::ClearFlag, clear flags\n");
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM)

  m_bSWCLinkDownProcessFlag = false;
  m_bSWCLinkCheckTotalFlag = false;

  //Clear flag to avoid send check link when state change to primary.
  //040603 Yu Wei
  UpdateRTUStatus(0x0000, RTU_STATUS_TABLE_SWC_LINK_CHECK);

  for(nI=0; nI<g_tRTUConfig.nSWCNumber; nI++)
  {
    m_aunSWCLinkCheckIndex[nI] = 0;
    m_abSWCLinkCheckFlag[nI] = false;
  }

  m_bWaitLinkCheckCompletedFlag = false;
}

#ifdef ModuleTest

void CRTUStatus::LogRmmStatusValues()
{
  int i;
  FILE *fd;

  printf("Start write 'RmmState.txt'\n");

  fd = fopen("/RTU/RmmState.txt", "w+");
  if ( fd == TYP_NULL) {
    printf("Can not save RmmState init values to RmmState.txt.\r\n");
    return;
  }
  //write time
  time_t lTime = time(NULL);
  fprintf(fd,"\tRmmState values after initialization\r\n:::::::::::::::::::::%s::::::::::::::::::::::\r\n", ctime(&lTime));

  //write values

  fprintf(fd,"int  g_bRMMTaskWorking:%d\r\n", g_bRMMTaskWorking);
  fprintf(fd,"int  g_nRMMTaskID:%d\r\n", g_nRMMTaskID);
  fprintf(fd,"int  g_nRMMSocketTaskID:%d\r\n", g_nRMMSocketTaskID);
  fprintf(fd,"int  g_bRTUInitLinkCheck:%d\r\n", g_bRTUInitLinkCheck);
  fprintf(fd,"unsigned short g_unLastServerCommand:%d\r\n", g_unLastServerCommand);
  fprintf(fd,"bool g_abSWCInitLinkCheck[SWC_MAX_NUMBER]:\r\n");
  for (i=0; i< SWC_MAX_NUMBER; i++) fprintf(fd, "    %d :%d\r\n", i, g_abSWCInitLinkCheck[i]);

  fprintf(fd,"int m_nServerRTUStatusTableLength: %d\r\n", m_nServerRTUStatusTableLength);

  fprintf(fd,"SEM_ID  m_tAccessRTUStatusTableSemID:%d\r\n", m_tAccessRTUStatusTableSemID);

  fprintf(fd,"MSG_Q_ID  m_tRMM_MSGQ_ID:%d\r\n", m_tRMM_MSGQ_ID);

  fprintf(fd,"int m_bServerSwitchFlag:%d\r\n", m_bServerSwitchFlag);

  fprintf(fd,"bool m_abSWCLinkCheckFlag[SWC_MAX_NUMBER]:\r\n");
  for (i=0; i< SWC_MAX_NUMBER; i++) fprintf(fd, "    %d %d\r\n", i, m_abSWCLinkCheckFlag[i]);

  fprintf(fd,"int m_bSWCLinkCheckTotalFlag:%d\r\n", m_bSWCLinkCheckTotalFlag);

  fprintf(fd,"unsigned short m_unSWCWeight:%d\r\n",  m_unSWCWeight);

  fprintf(fd,"unsigned short m_unOtherSWCWeight:%d\r\n",  m_unOtherSWCWeight);
  fprintf(fd,"unsigned short m_unOtherSWCWeightPrev:%d\r\n",  m_unOtherSWCWeightPrev);
  fprintf(fd,"unsigned short m_unOtherRTUServerLinkStatus:%d\r\n",  m_unOtherRTUServerLinkStatus);
  fprintf(fd,"unsigned short m_unOtherRTUServerLinkStatusPrev:%d\r\n",  m_unOtherRTUServerLinkStatusPrev);

  fprintf(fd,"int m_nOtherRTUStateFlag:%d\r\n", m_nOtherRTUStateFlag);
  fprintf(fd,"int m_bSWCLinkDownProcessFlag:%d\r\n", m_bSWCLinkDownProcessFlag);
  fprintf(fd,"int m_bSWCLinkDownFlag:%d\r\n", m_bSWCLinkDownFlag);

  lTime = time(NULL);

  fprintf(fd,"\r\n:::::::::::::::::::::End: %s::::::::::::::::::::::\r\n", ctime(&lTime));

  fclose(fd);
  printf("End write 'RmmState.txt'\n");
}

#endif
