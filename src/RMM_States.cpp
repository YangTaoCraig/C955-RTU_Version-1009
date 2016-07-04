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

  This file consists of RTU mode management

 AUTHOR

  Yu Wei


 REVISION HISTORY by Bryan Chong

 D1.1.3
 ------
 29-Apr-2011

 - cleanning up all #ifdef NOT_USED code [C955 PR107]

------------------------------------------------------------------------------*/

/************************************************************
*                              *
*  Project:    C830 RTU                  *
*  Module:    Redundancy Management Module (RMM)      *
*  File :      RMM_States.cpp                *
*  Author:    Yu Wei                    *
*                              *
*  Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************

**************************************************************

DESCRIPTION

  This file is state machine to prscess RTU state flag changing.

*************************************************************
Modiffication History
---------------------

Version: RMM D.1.1.2
  01  16 June 2003, Yu Wei,
    Start to write.

Version: RMM D.1.2.1
  01  14 July 2003, Yu Wei
    D.1.2.0 send to Ripple. Continue to add RMM.
    To be completed all states function.

  02  21 July 2003, Yu Wei,
    Add SWC link down in primary RTU, standby RTU will check
    the link status. Based on checking result, the primary RTU
    decide whether the state change.

Version: RMM D.1.3.2
  01  03 October 2003, Yu Wei
    When standby RTU weigth is changed more than previous, primary
    RTU will process link check. Modify GoState1_Primary() and
    added m_unOtherSWCWeightPrev.

  02  03 October 2003, Yu Wei
    Modify GoState1_Primary.
    When server link is down and RTU link is Down, change state to standby.
    Refer to PR:OASYS-PMS 0017.

  03  12 October 2003, Yu Wei
    Modify GoState1_Primary(). When primary RTU server link is OK and
    standby RTU server link is down, don't change primary RTU state.

  04  12 November 2003, Yu Wei,
    Added m_unOtherRTUServerLinkStatusPrev and StandbyRTUImproveCheck()
    for standby RTU server link recovered check.

  05  13 November 2003, Yu Wei
    Deleted STATEFLAG_SWITCHING_TEST state. This is not used.

  06  26 November 2003, Yu Wei
    Modify GoState1_Primary().
    If RTU's link is down, to check server link status and change state
    to switching, the system should wait for SOCKET_CONNECT_TIMEOUT to
    confirm whether the link is down.

Version: RMM D.1.3.3
  01  29 January 2004, Yu Wei,
    Modified GoState4_Switching(). Using switch instead of if-else. The all
    case will be covered.

Version: RMM D.1.3.4
  01  03 March 2004, Yu Wei
    Fixed bug in ChangeStateTo().
    Prevent recursive loop found using WindNavigator call tree.
    ChangeStateTo() will call StateInitialize().
    StateInitialize() will call ChangeStateTo().
    Refer to PR:OASYS-PMS 0026.

Version: RMM D.1.3.5
  01  23 March 2004, Yu Wei
    STATEFLAG_SWITCHING is changed, modified GoState1_Primary(),

    Changed GoState4_Switching() to GoState4_Switching_PtoS() and
    added GoState5_Switching_StoP().
    When standby RTU change to primary, it will wait for all server
    polling task completed to copy latest SWC polling to server polling
    table.
    Refer to PR:OASYS-PMS 0143.

    Added m_bSWCSwitchCompletedFlag for checking if all SWC changed to
    STATEFLAG_SWITCHING_PTOS
    and m_bSendSWCTableCompletedFlag for checking if send all SWC latset
    table to standby RTU completed.
    Modified GoState4_Switching_PtoS().
    Modified ChangeStateTo().
    Refer to PR:OASYS-PMS 0143.

Version: RMM D.1.3.6
  01  20 April 2004, Yu Wei
    Modified GoState4_Switching_PtoS() and GoState5_Switching_StoP(),
    based on m_nSwitchFlag to change state.
    Added CheckPrimarySwitchCompleted() to reduce
    GoState5_Switching_StoP()'s complexity.
    Refer to PR:OASYS-PMS 0163.

  02  30 April 2004, Yu Wei
    Modified StandbyRTUImproveCheck(). When SWC link is enable and link down,
    send check link command to SWC.
    Refer to PR:OASYS-PMS 0177.

  03  20 May 2004, Yu Wei
    Added "CMM_Timer.h" to use relative timer.
    Change g_tRMMTimeOut to type "tTimerValue".
    Modified ActivateTimeOut() and CheckTimeOut() to use relative timer
    and avoid the timer jump.
    Refer to PR:OASYS-PMS 0201.

  04  24 May 2004, Yu Wei
    Modified CheckPrimarySwitchCompleted(). When server send command
    change primary RTU to standby and primary transfered polling table,
    primary RTU will send confirm switch command to standby.
    Standby RTU received this command will set other RTU status to
    standby.
    Refer to PR:OASYS-PMS 0210.

  05  04 June 2004, Yu Wei
    Modified ChangeStateTo().
    Refer to PR:OASYS-PMS 0217.

Version: RMM E.1.0.3
  01  11 May 2004, Tong Zhi Xiong
    Add g_nWeightDoubleCheck to check again before Primary switch to Standby
    Refer to PR:OASYS-PMS 0245.

  02  15 June 2004, Tong Zhi Xiong
    Modify GoState1_Primary to check CLOCK link again before Primary switch to Standby
    Refer to PR:OASYS-PMS 0245.
**************************************************************/
#include <sys/neutrino.h>
#include <stdio.h>
#include <stdlib.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
#include "swc_def.h"
//#include "flg_def.h"
//#include "flg_ext.h"
#include "err_def.h"
#include "err_ext.h"
#include "hwp_def.h"
#include "hwp_ext.h"
#include "str_def.h"
#include "str_ext.h"

#include "Common.h"
#include "CMM_Log.h"
#include "RMM.h"
#include "CMM_Timer.h"
#include "CMM_Listen.h"
#include "CMM_Modbus.h"
#include "SWC.h"
#include "RMM_RTULink.h"
#include "Init.h"

#include "ServerPoll.h"    //GoState5_Switching_StoP() need check if Serverpoll copy
              //task completed update polling table. 040323 Yu Wei
#include "CMM_Timer.h"    //040520 Yu Wei

//During initialization, timeout in milisecond before declaring server down.
//#define  INIT_WAIT_SERVER_LINK_TIMEOUT  1000

eCurrentState g_eCurrentState;  //Current state of the state machine
bool g_bRMMServerLinkCheck;    //The flag is for server link checking during initialization.
bool g_bRMMOtherRTULinkCheck;  //The flag is for LAN link between RTUs checking during initialization.
//static bool rmmIsTimeOutStart = E_TYPE_No;
extern bool g_bRTUInitLinkCheck;

struct tTimerValue g_tRMMTimeOut;  //Timer counter.  //040520 Yu Wei

//State function declare.
void GoState0_NULL(void);
void GoState1_Primary(void);
void GoState2_Standby(void);
void GoState3_Initialization(void);
void GoState4_Switching_PtoS(void);  //040322 Yu Wei
void CheckPrimarySwitchCompleted(void); //040420 Yu Wei
void GoState5_Switching_StoP(void);  //040322 Yu Wei
void GoState6_HardwareTest(void);  //040322 Yu Wei
void GoState7_ResetSystem(void);  //040322 Yu Wei

void (*const RTUStateTable[STATE_MAX]) (void) =
{
  GoState0_NULL,
  GoState1_Primary,
  GoState2_Standby,
  GoState3_Initialization,
  GoState4_Switching_PtoS,
  GoState5_Switching_StoP,
  GoState6_HardwareTest,
  GoState7_ResetSystem

};

/**************************************************************
Purpose:
  State machine entry point.

***************************************************************/
void StateManagement(void)
{
  switch(g_eCurrentState)
  {
    case STATE1_PRIMARY:
      GoState1_Primary();
      break;
    case STATE2_STANDBY:
      GoState2_Standby();
      break;
    case STATE3_INITIALIZATION:
      GoState3_Initialization();
      break;
    case STATE4_SWITCHING_PTOS:
      GoState4_Switching_PtoS();
      break;
    case STATE5_SWITCHING_STOP:
      GoState5_Switching_StoP();
      break;
    case STATE6_HARDWARE_TEST:
      GoState6_HardwareTest();
      break;
    case STATE7_RESET_SYSTEM:
      GoState7_ResetSystem();
      break;
    default:
      StateInitialize();
      break;
  }// switch(g_eCurrentState)
}// StateManagement

/**************************************************************
Purpose:
  NULL state.

***************************************************************/
void GoState0_NULL(void)
{
  StateInitialize();
}
int g_nWeightDoubleCheck = 0;//Tong: 20050511
/*******************************************************************************
Purpose:
  Primary state.

History

    Name          Date          Remark
    ----          ----          ------
  Bryan Chong   28-Jun-2010   Update to include timeout for server
                              which is correspondent to LAN1_TIME_OUT

*******************************************************************************/
void GoState1_Primary(void)
{
  //If link between RTU is not down, auto switching will implement.
  if((g_tRTUStatus.abOtherLinkStatus[0] == true) ||
     (g_tRTUStatus.abOtherLinkStatus[1] == true))
  {
    //SWC link down processing completed or not in SWC link down status.
    //When primary RTU link down, m_bSWCLinkDownProcessFlag  will set to true,
    //When standby RTU completed link check, the flag will set to false.
    if(g_pRTUStatusTable->m_bSWCLinkDownProcessFlag == false)
    {

      if(g_tRTUStatus.unRTUServerLinkStatus !=
         g_pRTUStatusTable->m_unOtherRTUServerLinkStatus)
      {
        //RTU1 and RTU2 server link is same. (All are Link OK or Link down)
        if(g_tRTUStatus.unRTUServerLinkStatus == LINK_FAULTY)
        {
          #if ((defined CFG_DEBUG_MSG) && \
               (CFG_DEBUG_RMM || CFG_DEBUG_RMMLINKCHK))
          printf("[RMM] GoState1_Primary, PtoS, due to server link faulty\n");
          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK)
          ChangeStateTo(STATEFLAG_SWITCHING_PTOS);  //040322 Yu Wei
        }
      }
      else if(g_bRTUInitLinkCheck == false)
      {
        if(g_pRTUStatusTable->m_unSWCWeight <
           g_pRTUStatusTable->m_unOtherSWCWeight)
        {
          #if ((defined CFG_DEBUG_MSG) && \
               (CFG_DEBUG_RMM || CFG_DEBUG_RMMLINKCHK))
          printf("[RMM] GoState1_Primary, PtoS,\n"
                 "  local RTU notation, %d, > other RTU, %d\n",
                 g_pRTUStatusTable->m_unSWCWeight,
                 g_pRTUStatusTable->m_unOtherSWCWeight);
          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK)
          ChangeStateTo(STATEFLAG_SWITCHING_PTOS);  //040322 Yu Wei
          g_nWeightDoubleCheck = 0;
        }
        else
        {
          if (g_nWeightDoubleCheck != 0)
            g_nWeightDoubleCheck = 0;

          StandbyRTUImproveCheck();
        }
      }
    }
    else
    {
      //primary RTU link down , standby RTU has'n completed link check
      g_pRTUStatusTable->CheckSWCLinkStop();
    }

    g_pRTUStatusTable->m_unOtherRTUServerLinkStatusPrev =
      g_pRTUStatusTable->m_unOtherRTUServerLinkStatus;
    g_pRTUStatusTable->m_unOtherSWCWeightPrev =
      g_pRTUStatusTable->m_unOtherSWCWeight;

    ActivateTimeOut();
  }
  else if((g_tRTUStatus.unRTUServerLinkStatus == LINK_FAULTY) &&
          (CheckTimeOut(SOCKET_CONNECT_TIMEOUT*1.5) == true)) //kvmrt 20131028 Su
  {
    #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMLINK)
    printf("[RMM] GoState1_Primary, PtoS, due to server link faulty and\n"
           "  SOCKET_CONNECT_TIMEOUT %d sec\n", SOCKET_CONNECT_TIMEOUT);
    #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMLINK)
    ChangeStateTo(STATEFLAG_SWITCHING_PTOS);  //040322 Yu Wei
  }
  else if(g_pRTUStatusTable->m_bSWCLinkDownProcessFlag == true)
  {
    g_pRTUStatusTable->SetSWCLinkStart();
  }
}// GoState1_Primary

/**************************************************************
Purpose:
  Standby state.

***************************************************************/
void GoState2_Standby(void)
{
}

/*******************************************************************************
Purpose:
  This routine manage system initialization state by checking on the LAN link
  status between RTU and server; and LAN link between two RTUs

Input:
  None

Return:
  None

History
    Name           Date          Remark
    ----           ----          ------
  Yu, Wei        12-Aug-2004   Initial revision

*******************************************************************************/
void GoState3_Initialization(void)
{

  if(g_bRMMServerLinkCheck == true)  //Check server link.
  {
    clock_gettime(CLOCK_REALTIME, &g_pRTUStatusTable->m_tStartTime);
    if(g_tRTUStatus.unRTUServerLinkStatus == LINK_FAULTY)
    {
      #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMM_STATE)
      printf("WARN [RMM] GoState3_Initialization, server link faulty\n");
      #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMLINK)

      if(CheckTimeOut(g_tRTUConfig.nInitLANCheckTimeout) == true)
      {
        //Timeout. Confirm the link down.
        g_bRMMServerLinkCheck = false;
        //Check link status between RTU.
        g_bRMMOtherRTULinkCheck = true;
        ActivateTimeOut();
      }
    }else{
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM_STATE)
      printf("[RMM] GoState3_Initialization, server link healthy, change state "
             "to STATEFLAG_STANDBY\n");
      #endif // ((defined CFG_PRN_WARN) && (CFG_DEBUG_RMM))
      //Server link is OK, goto standby state.
      g_bRTUInitLinkCheck = false;
      ChangeStateTo(STATEFLAG_STANDBY);
    }
  }

  if(g_bRMMOtherRTULinkCheck == true)
  {
    if((CheckTimeOut(SOCKET_CONNECT_TIMEOUT *2 -
                     g_tRTUConfig.nInitLANCheckTimeout) == true) ||
       (g_tRTUConfig.nInitLANCheckTimeout >= SOCKET_CONNECT_TIMEOUT*2) ||
       (g_tRTUStatus.abOtherLinkStatus[0] == true) ||
       (g_tRTUStatus.abOtherLinkStatus[1] == true))
    {
      //Check LAN 2 link
      if(g_tRTUStatus.abOtherLinkStatus[0] == false)
      {
        //Check LAN 1 link
        if(g_tRTUStatus.abOtherLinkStatus[1] == false)
        {
          #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMM_STATE)
          printf("WARN [RMM] GoState3_Initialization, both RTU-RTU LAN links "
                 "down, change to STATEFLAG_STANDBY\n");
          #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMMLINK)
          //Two link are down. Set to standby state.
          g_bRTUInitLinkCheck = false;
          ChangeStateTo(STATEFLAG_STANDBY);
        }else{
          //Link2 between RTU is OK
          //Stop check RTU link.
          g_bRMMOtherRTULinkCheck = false;
        }
      }
      else
      {
        #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_RMM_STATE)
        printf("WARN [RMM] GoState3_Initialization, at least one RTU-RTU LAN "
               "link is healthy other is down\n");
        #endif // ((defined CFG_PRN_WARN) && (CFG_DEBUG_RMM))
        //Link1 between RTU is OK
        //Stop check RTU link.
        g_bRMMOtherRTULinkCheck = false;
      }
    }// //Stop check RTU link.

    if(g_bRMMOtherRTULinkCheck == false)
    {
      // check the role of other RTU. If other RTU is on standby, set RTU to
      // primary; otherwise, set RTU to standby
      if(g_pRTUStatusTable->m_nOtherRTUStateFlag == STATEFLAG_PRIMARY)
      {
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM_STATE)
        printf("[RMM] GoState3_Initialization, other RTU is on Primary "
               "set local RTU to standby\n");
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM_STATE)
        g_bRTUInitLinkCheck = false;
        ChangeStateTo(STATEFLAG_STANDBY);
      }else if(g_pRTUStatusTable->m_nOtherRTUStateFlag == STATEFLAG_STANDBY){
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM_STATE)
        printf("[RMM] GoState3_Initialization, other RTU is on Standby "
               "set local RTU to Primary\n");
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM_STATE)
        g_pRTUStatusTable->m_bSWCSwitchCompletedFlag = true ;//2008
        ChangeStateTo(STATEFLAG_PRIMARY);
      }else{
        #if ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_RMM_STATE))
        printf("WARN [RMM] GoState3_Initialization, cannot determine other RTU "
               "role set local RTU to Standby\n");
        #endif // ((defined CFG_PRN_WARN) && (CFG_DEBUG_RMM))
        g_bRTUInitLinkCheck = false;
        ChangeStateTo(STATEFLAG_STANDBY);
      }
    }// if(g_bRMMOtherRTULinkCheck == false)
  }// if(g_bRMMOtherRTULinkCheck == true)
}// GoState3_Initialization


/**************************************************************
Purpose:
  Switching state from primary to standby. //040420 Yu Wei

***************************************************************/
void GoState4_Switching_PtoS(void)
{

  //bool bSwitchCompletedFlag = true;
  //Flag to check if the switching completed.
  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM || CFG_DEBUG_RMMLINKCHK))
  printf("[RMM] GoState4_Switching_PtoS, switching pri to standby 0x%04x\n",
         g_pCRTULink->m_nWriteCommandResult);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM || CFG_DEBUG_RMMLINKCHK))
  switch(g_pCRTULink->m_nWriteCommandResult)
  {
  case SEND_COMMPLETE_NULL:
    CheckPrimarySwitchCompleted();
    break;

  case SEND_COMMPLETE_OK:
    //g_pCRTULink->m_bChangeToStandbyFlag = true;    //040325 Yu Wei
    switch(g_pRTUStatusTable->m_nSwitchFlag)  //040420 Yu Wei
    {
    case START_SWITCH:  //Standby reply switching command properly.
      g_pCRTULink->SetWriteCommand(CMD_COMFIRM_SWITCH, STATEFLAG_PRIMARY);
      g_pCRTULink->m_nWriteCommandResult = SEND_COMMPLETE_START;
      g_pRTUStatusTable->m_nSwitchFlag = COMFIRM_SWITCH;    //Set switching status.
      break;

    case COMFIRM_SWITCH:  //Standby reply confirm switching command properly.
      ChangeStateTo(STATEFLAG_STANDBY);  //Send command OK, change to standby state.
      g_pCRTULink->m_nWriteCommandResult = SEND_COMMPLETE_NULL;
      g_pRTUStatusTable->m_nSwitchFlag = NOT_IN_SWITCH;    //Set switching status.
      break;

    default:
      break;
    }      //040420 Yu Wei
    break;

  case SEND_COMMPLETE_ERROR:
    switch(g_pRTUStatusTable->m_nSwitchFlag)  //040420 Yu Wei
    {
    case START_SWITCH:  //Standby didn't reply switching command properly.
              //Standby will not receive confirm command and it will change to standby.
      ChangeStateTo(STATEFLAG_PRIMARY);  //Send command ERROR, return to primary state.
      g_pCRTULink->m_nWriteCommandResult = SEND_COMMPLETE_NULL;
      g_pRTUStatusTable->m_nSwitchFlag = NOT_IN_SWITCH;    //Set switching status.
      break;

    case COMFIRM_SWITCH://Standby didn't reply confirm switching command properly.
              //Maybe standby receive confirm command and it will change to primary.
      ChangeStateTo(STATEFLAG_STANDBY);
      g_pCRTULink->m_nWriteCommandResult = SEND_COMMPLETE_NULL;
      g_pRTUStatusTable->m_nSwitchFlag = NOT_IN_SWITCH;    //Set switching status.
      break;

    default:
      break;
    }      //040420 Yu Wei
    break;

  case SEND_COMMPLETE_START:
    break;

  default:
    g_pCRTULink->m_nWriteCommandResult = SEND_COMMPLETE_NULL;
    break;
  }
}

/*****************************************************8*************************
Purpose:
  Check if all SWC module change to switching state and primary
  RTU send all SWC tables to standby RTU.
  This routine is added to reduce GoState4_Switching_PtoS()'s complexity.
  Added in 20 April 2004, Yu Wei

History

  Name         Date       Remark
  ----         ----       ------
  Bryan Chong  15-Oct-09  Fix exit error when check m_bSWCSwitchCompletedFlag
                          for uninitialized SWC object, null pointer.
                          Also do not need to check for the last NTP at the
                          last of the SWC list
  Bryan Chong  29-Nov-11  Exclude NTP Client from the SWC checking list
*******************************************************************************/
void CheckPrimarySwitchCompleted(void)
{
  int nI;

  bool bSwitchCompletedFlag = true;  //Flag to check if the switching completed.

  if( g_pRTUStatusTable->m_bSWCSwitchCompletedFlag == false)  //040323 Yu Wei
  {
    for(nI=0; nI<g_tRTUConfig.nSWCNumber; nI++)
    {
      if((g_tRTUConfig.nSwcEnabled[nI] == false) ||
         (g_apSWCObject[nI]->m_nSWCID == 19))
        continue;

      if(g_apSWCObject[nI]->m_nSWCCurrentStateFlag !=
         STATEFLAG_SWITCHING_PTOS)
      {
        bSwitchCompletedFlag = false;
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK_SWITCH))
        printf("[RMM] CheckPrimarySwitchCompleted, waiting for %s to switch "
               "from Pri to Stdby, currently at state %d\n",
               g_apSWCObject[nI]->m_acSWCName, g_apSWCObject[nI]->m_nSWCCurrentStateFlag);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK_SWITCH))
     //   g_apSWCObject[nI]->m_nSWCCurrentStateFlag =STATEFLAG_SWITCHING_PTOS;
        break;    //040323 Yu Wei
      }
    }

    //All SWC have changed state to STATEFLAG_SWITCHING_PTOS  040323 Yu Wei
    if( bSwitchCompletedFlag == true)
    {
      g_pRTUStatusTable->m_bSWCSwitchCompletedFlag = true;
      g_pRTUStatusTable->m_bStartSendSWCTableFlag = true;
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK_SWITCH))
      printf("[RMM] CheckPrimarySwitchCompleted, switching completed\n");
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMMLINK_SWITCH))
    }
  }
  else
  {
    g_pRTUStatusTable->m_bSendSWCTableCompletedFlag = false;
    g_pRTUStatusTable->m_bSWCSwitchCompletedFlag = false;
    g_pRTUStatusTable->m_bStartSendSWCTableFlag = false;

    if((g_tRTUStatus.abOtherLinkStatus[0] == false) &&
       (g_tRTUStatus.abOtherLinkStatus[1] == false))
    {
      ChangeStateTo(STATEFLAG_STANDBY);
      if(g_pRTUStatusTable->m_bServerSwitchFlag == true)
        g_pRTUStatusTable->m_bServerSwitchFlag = false;
    }
    else if(g_pRTUStatusTable->m_bServerSwitchFlag == true)    //040524 Yu Wei
    {
      g_pCRTULink->SetWriteCommand(CMD_COMFIRM_SWITCH, STATEFLAG_STANDBY);
      g_pCRTULink->m_nWriteCommandResult = SEND_COMMPLETE_START;
      g_pRTUStatusTable->m_nSwitchFlag = COMFIRM_SWITCH;    //Set switching status.

      g_pRTUStatusTable->m_bServerSwitchFlag = false;

    }                              //040524 Yu Wei
    else
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
      printf("[RMM] CheckPrimarySwitchCompleted, pri RTU tx cmd to other RTU "
             "to switch to pri RTU\n");
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
      g_pCRTULink->SetWriteCommand(CMD_START_SWITCH, STATEFLAG_PRIMARY); //040420 Yu Wei
      g_pCRTULink->m_nWriteCommandResult = SEND_COMMPLETE_START;
      g_pRTUStatusTable->m_nSwitchFlag = START_SWITCH;    //Set switching status. 040420 Yu Wei

      //g_pRTUStatusTable->m_bOtherRTUSwitchFlag = true;    //Other RTU start to switch. //040420 Yu Wei
    }
  }
}

/**************************************************************
Purpose:
  Switching for standby to primary. When standby RTU receive changing
  state to primary, go to this state for waiting for server polling
  tasks completing copy SWC table.
  Added in 22 March 2004, Yu Wei
***************************************************************/
void GoState5_Switching_StoP(void)
{
//  int nI;
  bool bStateChangeFlag = true;

  if(bStateChangeFlag == true)  //All SPM task completed state changing.
  {
    switch(g_pRTUStatusTable->m_nSwitchFlag)  //040420 Yu Wei
    {
    case COMFIRM_SWITCH:  //Standby RTU received confirm command and switch to primary.
       //printf("GoState5_Switching_StoP CONFIRM_SWITCH\n");
      ChangeStateTo(STATEFLAG_PRIMARY);
      g_pRTUStatusTable->m_bSWCSwitchCompletedFlag = true;//2008
      g_pRTUStatusTable->m_nSwitchFlag = NOT_IN_SWITCH;    //Set switching status.
      break;

    case COMFIRM_SWITCH_ERROR:  //Standby RTU didn't receive confirm command and switch back to standby.
      //printf("GoState5_Switching_StoP CONFIRM_SWITCH_ERROR\n");
      ChangeStateTo(STATEFLAG_STANDBY);
      g_pRTUStatusTable->m_nSwitchFlag = NOT_IN_SWITCH;    //Set switching status.
      break;

    default:
//      printf("GoState5_Switching_StoP default\n");
      break;
    }  //040420 Yu Wei
  }
}

/**************************************************************
Purpose:
  Hardware test state.

***************************************************************/
void GoState6_HardwareTest(void)
{

}

/**************************************************************
Purpose:
  RTU reset state.

***************************************************************/
void GoState7_ResetSystem(void)
{
  #if ((defined CFG_ENABLE_WATCHDOG_RESET) && (defined CFG_TEST_RELEASE))
  ResetRTU();
  #endif // ((defined CFG_ENABLE_WATCHDOG_RESET) && (defined CFG_TEST_RELEASE))
}

/**************************************************************
Purpose:
  Initialize state machine.

***************************************************************/
void StateInitialize(void)
{
  ChangeStateTo(STATEFLAG_INITIALIZATION);
  g_bRMMServerLinkCheck = true;
  g_bRMMOtherRTULinkCheck = false;
  ActivateTimeOut();
}

/*******************************************************************************
Purpose:
  Set new RTU state flag and state machine state.

Input
  nNewState  -- The new RTU state flag. (in)

*******************************************************************************/
/*******************************************************************************
Purpose:
  Set new RTU state flag and state machine state.

Input/Output
  nNewState  -- [in] The new RTU state flag.

Return
  None

History

    Name          Date          Remark
    ----          ----          ------
  Yu, Wei       04-Jul-2007   Initial revision
  Bryan Chong   18-Oct-2010   Add state change to log [PR80]

*******************************************************************************/
void ChangeStateTo(int nNewState)
{
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM_STATE)
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM_STATE)
  #ifdef CFG_PRN_ERR
  CHAR acerrmsg[200];
  #endif // CFG_PRN_ERR
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};
  char cLog[500];
  E_ERR_T rstatus = E_ERR_Success;

  switch(nNewState)
  {
  case STATEFLAG_PRIMARY:
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM_STATE)
    rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                         E_SYS_TIME_Local);
    printf("\n\n[RMM] %d.%02d.%02d %02d:%02d:%02d.%03d ChangeStateTo,\n"
           "  STATEFLAG_PRIMARY\n",
           (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
           pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
           pbdtime->tm_sec, tspec.tv_nsec/1000000);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM_STATE))
    // Display state on LCD
    rstatus = HWP_LCM_PrnMsgCtrl(E_HWP_LCM_MSG_Primary);
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_PRN_ERR
      ERR_GetMsgString(rstatus, acerrmsg);
      printf("[RMM] ChangeStateTo, print LCD msg %d error, %s\n",
             E_HWP_LCM_MSG_Primary, acerrmsg);
      #endif // CFG_PRN_ERR
    }

    g_pRTUStatusTable->m_unOtherSWCWeight = 0;
    g_pRTUStatusTable->m_unOtherSWCWeightPrev = 10000;
    if( g_tRTUStatus.nRTUStateFlag == STATEFLAG_SWITCHING_STOP)  //040604 Yu Wei
    {
      g_pRTUStatusTable->m_unSWCWeight = 10001;
    }

    g_pRTUStatusTable->ClearFlag();
    g_eCurrentState = STATE1_PRIMARY;
    ActivateTimeOut();

    // 20101018 BC
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_StateChange,
//      FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//      "[RMM] ChangeStateTo, %s. RTU set to Primary\n\n",
//       SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
     sprintf(cLog,"[RMM] ChangeStateTo, %s. RTU set to Primary\n\n",
    	       SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
     g_pDebugLog->LogMessage(cLog);
    break;

  case STATEFLAG_STANDBY:
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM_STATE)
    rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                         E_SYS_TIME_Local);
    printf("\n\n[RMM] %d.%02d.%02d %02d:%02d:%02d.%03d ChangeStateTo,\n"
           "  STATEFLAG_STANDBY\n",
           (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
           pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
           pbdtime->tm_sec, tspec.tv_nsec/1000000);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM_STATE))

    // Display state on LCD
    rstatus = HWP_LCM_PrnMsgCtrl(E_HWP_LCM_MSG_Standby);
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_PRN_ERR
      ERR_GetMsgString(rstatus, acerrmsg);
      printf("[RMM] ChangeStateTo, print LCD msg %d error, %s\n",
               E_HWP_LCM_MSG_Standby, acerrmsg);
      #endif // CFG_PRN_ERR
    }

    g_eCurrentState = STATE2_STANDBY;
    g_pRTUStatusTable->ClearFlag();

    // 20101018 BC
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_StateChange,
//      FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//      "[RMM] ChangeStateTo, %s. RTU set to Standby\n\n",
//       SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
    sprintf(cLog,"[RMM] ChangeStateTo, %s. RTU set to Standby\n\n",
    	       SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
    g_pDebugLog->LogMessage(cLog);
    break;

  case STATEFLAG_INITIALIZATION:
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM_STATE)
      printf("[RMM] ChangeStateTo, STATEFLAG_INITIALIZATION\n");
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM_STATE))
      g_eCurrentState = STATE3_INITIALIZATION;
      ActivateTimeOut();

      // 20101018 BC
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_StateChange,
//        FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//        "[RMM] ChangeStateTo, %s\n"
//        "  RTU set to Initialization\n",
//         SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
       sprintf(cLog, "[RMM] ChangeStateTo, %s\n RTU set to Initialization\n",
    	         SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
            g_pDebugLog->LogMessage(cLog);
      break;

  default:
    //State falg error, set to STATEFLAG_INITIALIZATION state.
    //040301 Yu Wei
    #ifdef CFG_PRN_WARN
    printf("WARN [RMM] ChangeStateTo, state flag error, set to "
           "STATEFLAG_INITIALIZATION\n");
    #endif // CFG_PRN_WARN
    g_eCurrentState = STATE3_INITIALIZATION;
    ActivateTimeOut();
    break;

  case STATEFLAG_SWITCHING_PTOS:
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM_STATE)
    rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                         E_SYS_TIME_Local);
    printf("\n\n[RMM] %d.%02d.%02d %02d:%02d:%02d.%03d ChangeStateTo,\n"
           "  STATEFLAG_SWITCHING_PTOS\n",
           (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
           pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
           pbdtime->tm_sec, tspec.tv_nsec/1000000);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM_STATE))
    g_pRTUStatusTable->m_bSWCSwitchCompletedFlag = false;    //20150327 Yang Tao
    g_pRTUStatusTable->m_bSendSWCTableCompletedFlag = false;  //040323 Yu Wei
    g_pRTUStatusTable->m_bStartSendSWCTableFlag = false;    //040323 Yu Wei
    g_eCurrentState = STATE4_SWITCHING_PTOS;          //040323 Yu Wei

    // 20101018 BC
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_StateChange,
//      FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//      "[RMM] ChangeStateTo, %s. RTU set Primary to Stdby\n",
//       SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
//    sprintf(cLog, "[RMM] ChangeStateTo, %s. RTU set Primary to Stdby\n",
//    	       SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
//    g_pDebugLog->LogMessage(cLog);
    break;

  case STATEFLAG_SWITCHING_STOP:
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM_STATE)
    rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                         E_SYS_TIME_Local);
    printf("\n\n[RMM] %d.%02d.%02d %02d:%02d:%02d.%03d ChangeStateTo,\n"
           "  STATEFLAG_SWITCHING_STOP\n",
           (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
           pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
           pbdtime->tm_sec, tspec.tv_nsec/1000000);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM_STATE))
    g_eCurrentState = STATE5_SWITCHING_STOP;  //040323 Yu Wei

    // 20101018 BC
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_StateChange,
//      FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//      "[RMM] ChangeStateTo, %s. RTU set Stdby to Primary\n",
//       SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
//    sprintf(cLog, "\n\n[RMM] ChangeStateTo, %s. RTU set Stdby to Primary\n\n",
//    	       SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
//    g_pDebugLog->LogMessage(cLog);

    break;

  case STATEFLAG_HARDWARETEST:
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM_STATE)
    printf("[RMM] ChangeStateTo, STATEFLAG_HARDWARETEST\n");
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM_STATE))
    g_eCurrentState = STATE6_HARDWARE_TEST;    //040323 Yu Wei

    // 20101018 BC
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_StateChange,
//      FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//      "[RMM] ChangeStateTo, %s. RTU set to Hardware Test\n\n",
//       SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
    sprintf(cLog, "[RMM] ChangeStateTo, %s. RTU set to Hardware Test\n\n",
    	       SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
    g_pDebugLog->LogMessage(cLog);
    break;

  case STATEFLAG_SYSTEM_RESET:
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMM_STATE)
    printf("[RMM] ChangeStateTo, STATEFLAG_SYSTEM_RESET\n");
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM_STATE))
    g_eCurrentState = STATE7_RESET_SYSTEM;    //040323 Yu Wei
    SYS_isExitAppl = E_TYPE_True;

    // 20101018 BC
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_RMM_StateChange,
//      FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//      "[RMM] ChangeStateTo, %s. RTU set to Reset\n\n",
//       SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
    sprintf(cLog,"[RMM] ChangeStateTo, %s. RTU set to Reset\n\n",
    	       SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)));
    g_pDebugLog->LogMessage(cLog);
    break;

  }

  g_tRTUStatus.nRTUStateFlag = nNewState;
}// ChangeStateTo


/*******************************************************************************
Purpose:
  Reset timeout counter.
  Re-design in 20 May 2004, Yu Wei. It will use relative timer.

*******************************************************************************/
void ActivateTimeOut(void)
{
  GetTimerValue(&g_tRMMTimeOut);
}

/*******************************************************************************
Purpose:
  Check timeout.
  Re-design in 20 May 2004, Yu Wei. It will use relative timer.

Input
  unTimeValue  -- Timeout value. (in)

Return
  1  - Timeout.
  0  - Not timeout.

*******************************************************************************/
bool CheckTimeOut(unsigned int unTimeValue)
{
  struct tTimerValue tTime;
  unsigned int unTimerDiff = 0;
  GetTimerValue(&tTime);

  unTimerDiff = ( tTime.High - g_tRMMTimeOut.High ) * TIMER_LOW_MSEC;  //Get high part different.

  unTimerDiff += tTime.Low;      //Added low part different.
  unTimerDiff -= g_tRMMTimeOut.Low;

  #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_RMM)
  printf("[RMM] CheckTimeOut, unTimerDiff = %d, unTimeValue = %d\n",
         unTimerDiff, unTimeValue);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_RMM_STATE))
  if (unTimerDiff >= unTimeValue)    //Check timeout.
    return true;
  else
    return false;
}

/*******************************************************************************
Purpose:
  When standby RTU server link recovered or weight is greater than previous,
  the primary RTU will check the SWC status that primary RTU's link is
  down except clock.

*******************************************************************************/
void StandbyRTUImproveCheck(void)
{
  int nI;
  int nSWCStatus = OK;

  if(((g_pRTUStatusTable->m_unOtherRTUServerLinkStatusPrev == LINK_FAULTY) &&
      (g_pRTUStatusTable->m_unOtherRTUServerLinkStatus == SERVER_LINK_OK)) ||
      (g_pRTUStatusTable->m_unOtherSWCWeightPrev <
       g_pRTUStatusTable->m_unOtherSWCWeight))
  {

    //If primary RTU has link down SWC except Clock.
    for(nI=0; nI<g_tRTUConfig.nSWCNumber; nI++)
    {
      //Link enable, SWC can check link. //040430 Yu Wei
      if(((STR_ByteSwap16Bit(
             g_pRTUStatusTable->m_aunOtherRTUStatus[g_tRTUConfig.anSWCIndex[nI]
             + RTU_STATUS_TABLE_SWC_OFFSET]) & VALIDAION_BIT) == 0)&&
         ((STR_ByteSwap16Bit(
             g_pRTUStatusTable->m_aunOtherRTUStatus[g_tRTUConfig.anSWCIndex[nI]
             + RTU_STATUS_TABLE_SWC_OFFSET]) & 0x0004) == 1))
      {

        nSWCStatus = ERROR;
        break;
      }
    }

    if(nSWCStatus == ERROR)
    {
      #ifdef CFG_DEBUG_WARN
      printf("WARN [RMM] StandbyRTUImproveCheck, found invalid condition, stop "
             "all SWC links\n");
     #endif // CFG_DEBUG_WARN
      //Process standby RTU link checking.
      g_pRTUStatusTable->SetSWCLinkStop();          //Stop all down SWC's link.
      g_pRTUStatusTable->m_bSWCLinkDownProcessFlag = true;
      g_pRTUStatusTable->m_bSWCLinkCheckTotalFlag = true;
    }
  }
}

