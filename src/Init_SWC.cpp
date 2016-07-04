///************************************************************
//*                              *
//*  Project:    C830 RTU                  *
//*  Module:    Initialization (INM)            *
//*  File :      Init_SWC.cpp                *
//*  Author:    Yu Wei                    *
//*                              *
//*  Copyright 2003 Singapore Technologies Electronics Ltd.  *
//*************************************************************
//
//
//**************************************************************
//
//DESCRIPTION
//
//  This file will initialize spamn all SWC task except SWC-CLK.
//
//*************************************************************/
#include <sys/neutrino.h>
#include <stdio.h>
#include <iostream.h>
#include <termios.h>
#include <mqueue.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "sys_ass.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
#include "swc_def.h"



#include "CMM_Timer.h"
#include "CMM_Listen.h"
#include "Init_SWC.h"
#include "RMM.h"
#include "cgf_def.h"	//kvmrt 20140626 Su
#include "cgf_ext.h"	//kvmrt 20140626 Su

tTaskParameter g_atMainTaskPara[SWC_MAX_NUMBER];    //Main task parameters.
tTaskParameter g_atServerTaskPara[SWC_MAX_NUMBER];  //Server task parameter.

/*******************************************************************************
Purpose:
  This CSU create all SWC objects and spawn all SWC main tasks, server socket
  tasks and multidrop link tasks. If return is ERROR, it will delete all created
  objects and spawned tasks.

Return
  OK    -- create objects and spawn tasks OK.
  ERROR  -- create one object or spawn one task error.

History

      Name         Date       Remark
      ----         ----       ------
 Bryan Chong   17-May-2010  Update to include bSWCExit flag for terminating
                             application
                             Update to initialize for enabled SWCs only
 Bryan Chong   28-Apr-2011  Move StopAllSWC routine into conditional check of
                             nReturn == ERROR. [C955 PR92]
*******************************************************************************/
int SWCInitialization(void)
{
  int nI;
  int nReturn = OK;
  // 20100517 BC (Rqd ZSL)
  bSWCExit = E_TYPE_False;




  // Initial enabled SWC and spawn the respective task.
  // 20100517 BC (Rqd ZSL)
  for(nI=0; nI<g_tRTUConfig.nSWCNumber; nI++)
  {
    if(g_tRTUConfig.nSwcEnabled[nI] == 0)
      continue;

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
    printf("[INM] SWCInitialization, SWC(%02d of %02d) = %s\n",
      (nI+1), g_tRTUConfig.nSWCNumber,
      g_tRTUConfig.nSwcEnabled[nI] ? "Enable" : "Disable");
    #endif // CFG_DEBUG_MSG

    nReturn = InitializeOneSWC(nI);
    if(nReturn == ERROR )
    {
      #ifdef CFG_PRN_ERR
      printf("ERR  [SWC] SWCInitialization, fail at SWC %d\n", nI);
      #endif // CFG_PRN_ERR
      StopAllSWC();
      nReturn = 1;
      break;
    }
  } // for(nI=0; nI<g_tRTUConfig.nSWCNumber; nI++)

  if(nReturn != ERROR)
  {

    g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX] = new CSWC(E_SWC_T_NTP_Client);
    if(g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX] != TYP_NULL)
    {
      memcpy(g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_acSWCName,
             CFG_NTP_DEFAULT_NAME, sizeof(CFG_NTP_DEFAULT_NAME));

      g_tRTUConfig.nSwcEnabled[CFG_NTP_CLIENT_SWC_INDEX] = E_TYPE_Yes;
      g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_nSWCID =
        CFG_NTP_CLIENT_SWC_INDEX;
      g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_nMasterLinkID =
        g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_nPrimaryPortID = 0;
      g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_anLinkStatus[
        g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_nMasterLinkID] = NOLINK;
      g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_anLinkStatus[
        g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_nMasterLinkID^1] = NOLINK;

      // Enable RTU-External System Link
      g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_bPollingEnable = true;

      g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_nLinkType = SWC_LINK_LAN;
      g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_nFastExcepCounter = 0;
      g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_nSlowExcepCounter = 0;
      g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_nTimerExcepCounter = 0;

      //g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_bStartupCheckLink = false;
    }

    g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_bStartupCheckLink = true;

 }
  return nReturn;
}
//
/*******************************************************************************
Purpose:
  This CSU create one SWC objects and spawn SWC main tasks, server socket
  tasks.

Input
  nI    -- SWC ID index.

Return
  OK    -- create objects and spawn tasks OK.
  ERROR  -- create one object or spawn one task error.

History

     Name        Date        Remark
     ----        ----        ------
 Bryan Chong  10-Jun-2011  Update with flag m_bIsModbusZeroByteCntAllowed to
                           support of zero byte at byte count field within
                           Modbus packet [C955 PR109]

*******************************************************************************/
int InitializeOneSWC(int nI)
{
  E_ERR_T rstatus = E_ERR_Success;
  char acTemp[200];
  int nReturn = OK;
  int nIndex;
  nIndex = nI;

  if(g_tRTUConfig.tSWC[nIndex].tHeader.nLinkType == SWC_LINK_LAN)
  {
    g_apSWCObject[nIndex] = new CSWC_LAN(
              g_tRTUConfig.tSWC[nIndex],
              nIndex,
              g_tRTUConfig.tLANPara[0].acLAN_IP);
  }
  else if(g_tRTUConfig.tSWC[nIndex].tHeader.nLinkType == SWC_LINK_RS_485 ||
          g_tRTUConfig.tSWC[nIndex].tHeader.nLinkType == SWC_LINK_RS_232 ||
          g_tRTUConfig.tSWC[nIndex].tHeader.nLinkType == SWC_LINK_RS_422)
  {

    g_apSWCObject[nIndex] = new CSWC(
              g_tRTUConfig.tSWC[nIndex],
              nIndex,
              g_tRTUConfig.tLANPara[0].acLAN_IP);
  }

  // create object fail, log and exit
  if ((g_apSWCObject[nIndex] == TYP_NULL) ||
      (g_apSWCObject[nIndex]->m_bCreateObjectResult == false) )

  {
    sprintf(acTemp,"%s: create object failed\n",
      g_tRTUConfig.tSWC[nIndex].tHeader.acName);
    g_pEventLog->LogMessage(acTemp);
    printf("%s \n" , acTemp);
    nReturn = ERROR;
  }
  // create object success, start to spawn SWC threads
  else
  {
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
    printf("[SWC] InitializeOneSWC, initialize object %02d %s %s\n", nIndex,
      g_tRTUConfig.tSWC[nIndex].tHeader.acName,
      g_apSWCObject[nIndex]->m_bCreateObjectResult ? "PASS" : "FAIL");
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))

    rstatus = SYS_ThrdInit((E_SYS_TLB)
      (E_SYS_TLB_SWC_MgmtTask + (nIndex * CFG_NUM_OF_THREADS_PER_SWC)));
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_PRN_ERR
      printf("ERR  [SWC] InitializeOneSWC, create SWC Management thread "
             "for %s fail\n", g_apSWCObject[nI]->m_acSWCName);
      #endif // CFG_PRN_ERR
      memset(acTemp, 0, sizeof(acTemp));
      sprintf(acTemp,
        "ERR  [SWC] InitializeOneSWC, create SWC Management thread "
        "for %s fail\n",
        g_apSWCObject[nIndex]->m_acSWCName);
      g_pEventLog->LogMessage(acTemp);
      return ERROR;;
    }

    g_apSWCObject[nIndex]->m_nMainTaskID =
      pSYS_CB->thrd_ctrl[E_SYS_TLB_SWC_MgmtTask +
        (nIndex * CFG_NUM_OF_THREADS_PER_SWC)].thrd_id;

    rstatus = SYS_ThrdInit((E_SYS_TLB)
      (E_SYS_TLB_SWC_ServerTask + (nIndex * CFG_NUM_OF_THREADS_PER_SWC)));
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_PRN_ERR
      printf("ERR  [SWC] InitializeOneSWC, create SWC Server thread "
             "for %s fail\n", g_apSWCObject[nI]->m_acSWCName);
      #endif // CFG_PRN_ERR
      memset(acTemp, 0, sizeof(acTemp));
      sprintf(acTemp,
        "ERR  [SWC] InitializeOneSWC, create SWC Server thread "
        "for %s fail\n",
        g_apSWCObject[nIndex]->m_acSWCName);
      g_pEventLog->LogMessage(acTemp);
      return ERROR;;
    }

    g_apSWCObject[nIndex]->m_nServerSocketTaskID =
      pSYS_CB->thrd_ctrl[E_SYS_TLB_SWC_ServerTask +
        (nIndex * CFG_NUM_OF_THREADS_PER_SWC)].thrd_id;

    #ifdef CFG_EXTENDED_MODBUS
    // extended modbus
    if(g_tRTUConfig.tSWC[nIndex].tHeader.bExtendedModbus == E_TYPE_Yes)
    {
      g_apSWCObject[nIndex]->m_bIsExtendedModbus = E_TYPE_Yes;
    }

    //20110610 BC
    if(strcmp("ECS", g_apSWCObject[nIndex]->m_acSWCName) == 0)
    {
      g_apSWCObject[nIndex]->m_bIsModbusZeroByteCntAllowed = E_TYPE_Yes;
    }
    #endif // CFG_EXTENDED_MODBUS
  }// end else
  return nReturn;
}

/*******************************************************************************
Purpose:
  Stop all SWC tasks.

History

      Name         Date       Remark
      ----         ----       ------
 Bryran Chong   17-May-2010  Update to include bSWCExit flag for terminating
                             application
                             Update to check and detached threads for enabled
                             SWCs only

*******************************************************************************/
void StopAllSWC(void)
{
  int nI;

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
  printf("[SWC] StopAllSWC, ready to stop all SWCs\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))

  // 20100517 BC (Rqd ZSL)
  for(nI=0; nI<g_tRTUConfig.nSWCNumber; nI++)
  {
    if(g_tRTUConfig.nSwcEnabled[nI] == 0)
      continue;

    if(g_apSWCObject[nI] != NULL)
    {
      // Delete SWC server socket task.
      if((g_apSWCObject[nI]->m_nServerSocketTaskID != ERROR) &&
        (g_apSWCObject[nI]->m_nServerSocketTaskID != NULL))
      {
        //detach a thread from this process
        pthread_detach(g_apSWCObject[nI]->m_nServerSocketTaskID);
        pthread_cancel(g_apSWCObject[nI]->m_nServerSocketTaskID);
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
        printf("[SWC] StopAllSWC, stop %s, %02d, server socket Task\n",
               g_apSWCObject[nI]->m_acSWCName, nI);
               //g_apSWCObject[nI]->m_acSWCName);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
      }

      // Delete SWC main task.
      if( (g_apSWCObject[nI]->m_nMainTaskID != ERROR) &&
        (g_apSWCObject[nI]->m_nMainTaskID != NULL))
      {
        g_apSWCObject[nI]->m_bTaskWorking = false;
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
        printf("[SWC] StopAllSWC, stop SWC(%02d) main Task\n",
               nI);
               //g_apSWCObject[nI]->m_acSWCName);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
      }

    }
  }
}
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  INM_GetSWCIndex

 DESCRIPTION

  This routine will return SWC index for the designated SWC's name

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  pswclable  [in] pointer to SWC's name
  poutswcidx [out] return of SWC index value through user defined pointer

 RETURN

  E_ERR_Success             the routine executed successfully
  E_ERR_InvalidNullPointer  invalid null pointer

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    04-Aug-2011      Created initial revision

-----------------------------------------------------------------------------*/
//E_ERR_T INM_GetSWCIndex(CHAR *pswclabel, INT8 *poutswcidx)
//{
//  UINT8 cnt = 0;
//
//  if((pswclabel == TYP_NULL) || (poutswcidx == TYP_NULL))
//    return E_ERR_InvalidNullPointer;
//
//  *poutswcidx = (INT8)TYP_ERROR;
//
//  for(cnt = 0; cnt < g_tRTUConfig.nSWCNumber; cnt++)
//  {
//    if((g_apSWCObject[cnt]->m_acSWCName == TYP_NULL) ||
//         (g_tRTUConfig.nSwcEnabled[cnt] == 0))
//      continue;
//
//    if(strcmp(pswclabel, g_apSWCObject[cnt]->m_acSWCName) == 0)
//    {
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
//      printf("[INM] INM_GetSWCIndex, %d, %s(%d)\n",
//        g_apSWCObject[cnt]->m_nSWCID, g_apSWCObject[cnt]->m_acSWCName,
//        strlen(g_apSWCObject[cnt]->m_acSWCName));
//      #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
//
//      *poutswcidx = cnt;
//      break;
//    }
//  } // for(cnt = 0; cnt < (g_tRTUConfig.nSWCNumber - 1); cnt++)
//
//  return E_ERR_Success;
//} // INM_GetSWCIndex
