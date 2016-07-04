/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics Ltd.

 PROPRIETARY RIGHTS of ST Electronics Ltd are involved in the subject matter
 of this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  sysi.cpp                                               D1.0.4

 COMPONENT

  SYS - Application System

 DESCRIPTION

  This file consists of initialization routine for the application

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

  Bryan Chong    07-Sep-2009        Initial revision

----------------------------------------------------------------------------*/
#include <stdio.h>
#include <pthread.h>
#include <fixerrno.h>
#include <string.h>
#include <sys/neutrino.h>
#include <termios.h>
#include <iostream.h>
#include <mqueue.h>

#include "cfg_def.h"
#include "type_def.h"
#include "sys_ass.h"
#include "err_def.h"
#include "err_ext.h"
#include "cgf_def.h"
#include "cgf_ext.h"
#include "ser_def.h"
#include "ser_ext.h"
//#ifdef CFG_ENABLE_SPA
//#include "spa_def.h"
//#include "spa_ext.h"
//#endif // CFG_ENABLE_SPA
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
#include "hwp_def.h"
#include "hwp_ext.h"
//#include "flg_def.h"
//#include "flg_ext.h"
#include "ntp_def.h"
#include "ntp_ext.h"
#include "lnc_def.h"
#include "lnc_ext.h"
#include "str_def.h"
#include "str_ext.h"
#include "rmm_def.h"

#include "Init.h"
#include "Init_Config.h"
#include "CMM_Log.h"
#include "CMM_Timer.h"
#include "CMM_Watchdog.h"
#include "ServerPoll.h"
#include "CMM_Listen.h"
#include "Init_SWC.h"
#include "RMM.h"
#include "RMM_RTULink.h"
#include "MMM.h"
#include "SWC.h"

#include "sys_lut.h"
/*----------------------------------------------------------------------------
  Public variables declaration
----------------------------------------------------------------------------*/
SYS_CB_T *pSYS_CB;


//RTU configuration parameters.
SYS_CONFIGURATION_T  g_tRTUConfig;
//RTU State flag and LAN link status.
SYS_GLOBALSTATUS_T   g_tRTUStatus;

BOOL_T  SYS_isExitAppl;
/*----------------------------------------------------------------------------
  Public Prototypes declaration
----------------------------------------------------------------------------*/
E_ERR_T SYS_Initialization();
E_ERR_T SYS_ThrdInit(E_SYS_TLB thrd_lbl);
//E_ERR_T SYS_MutexInit(E_SYS_MTX mutex_id);
E_ERR_T SYS_CreateMsgQueue(E_SYS_MSQL msq_label);
E_ERR_T SYS_InitCtrlBlk(VOID);

//#if CFG_ENABLE_PCI_DETECTION
//E_ERR_T SYS_InitiateDevcSer8250Manager(VOID);
//E_ERR_T SYS_DeinitializeDevc8250Manager(VOID);
//#endif // (CFG_ENABLE_PCI_DETECTION)

/*----------------------------------------------------------------------------
  Local variables declaration
----------------------------------------------------------------------------*/
static SYS_CB_T sys_CtrlBlk;

#if CFG_ENABLE_PCI_DETECTION
// Device ID 1180h. Bottom bay is using int 7, whereas top is using int 5
static CHAR *devc_ser8250_int5_index0[] =
  {(CHAR *)"/sbin/devc-ser8250", (CHAR *)"-c", (CHAR *)"1843200/2",
   (CHAR *)"-T8", (CHAR *)"-t8", (CHAR *)"-u5", (CHAR *)"a900,5", (CHAR *)"-u6",
   (CHAR *)"a908,5", (CHAR *)"-u7", (CHAR *)"a910,5", (CHAR *)"-u8",
   (CHAR *)"a918,5", (CHAR *)"-u9", (CHAR *)"a920,5", (CHAR *)"-u10",
   (CHAR *)"a928,5", (CHAR *)"-u11", (CHAR *)"a930,5", (CHAR *)"-u12",
   (CHAR *)"a938,5", (CHAR *)NULL};

static CHAR *devc_ser8250_int7_index0[] =
  {(CHAR *)"/sbin/devc-ser8250", (CHAR *)"-c", (CHAR *)"1843200/2",
   (CHAR *)"-T8", (CHAR *)"-t8", (CHAR *)"-u5", (CHAR *)"a900,7", (CHAR *)"-u6",
   (CHAR *)"a908,7", (CHAR *)"-u7", (CHAR *)"a910,7", (CHAR *)"-u8",
   (CHAR *)"a918,7", (CHAR *)"-u9", (CHAR *)"a920,7", (CHAR *)"-u10",
   (CHAR *)"a928,7", (CHAR *)"-u11", (CHAR *)"a930,7", (CHAR *)"-u12",
   (CHAR *)"a938,7", (CHAR *)NULL};

static CHAR *devc_ser8250_int5_index1[] =
  {(CHAR *)"/sbin/devc-ser8250", (CHAR *)"-c", (CHAR *)"1843200/2",
   (CHAR *)"-T8", (CHAR *)"-t8", (CHAR *)"-u13", (CHAR *)"a600,5",
   (CHAR *)"-u14", (CHAR *)"a608,5", (CHAR *)"-u15", (CHAR *)"a610,5",
   (CHAR *)"-u16", (CHAR *)"a618,5", (CHAR *)"-u17", (CHAR *)"a620,5",
   (CHAR *)"-u18", (CHAR *)"a628,5", (CHAR *)"-u19", (CHAR *)"a630,5",
   (CHAR *)"-u20", (CHAR *)"a638,5", (CHAR *)NULL};

// Device ID 1380h. Bottom bay is using int 10, top is int 11
static CHAR *devc_ser8250_int10_index0[] =
  {(CHAR *)"/sbin/devc-ser8250", (CHAR *)"-c", (CHAR *)"1843200/2",
   (CHAR *)"-T8", (CHAR *)"-t8", (CHAR *)"-u5", (CHAR *)"a900,10",
   (CHAR *)"-u6", (CHAR *)"a908,10", (CHAR *)"-u7", (CHAR *)"a910,10",
   (CHAR *)"-u8", (CHAR *)"a918,10", (CHAR *)"-u9", (CHAR *)"a920,10",
   (CHAR *)"-u10", (CHAR *)"a928,10", (CHAR *)"-u11", (CHAR *)"a930,10",
   (CHAR *)"-u12", (CHAR *)"a938,10", (CHAR *)NULL};
static CHAR *devc_ser8250_int11_index0[] =
  {(CHAR *)"/sbin/devc-ser8250", (CHAR *)"-c", (CHAR *)"1843200/2",
   (CHAR *)"-T8", (CHAR *)"-t8", (CHAR *)"-u5", (CHAR *)"a900,11",
   (CHAR *)"-u6", (CHAR *)"a908,11", (CHAR *)"-u7", (CHAR *)"a910,11",
   (CHAR *)"-u8", (CHAR *)"a918,11", (CHAR *)"-u9", (CHAR *)"a920,11",
   (CHAR *)"-u10", (CHAR *)"a928,11", (CHAR *)"-u11", (CHAR *)"a930,11",
   (CHAR *)"-u12", (CHAR *)"a938,11", (CHAR *)NULL};

static CHAR *devc_ser8250_int11_index1[] =
  {(CHAR *)"/sbin/devc-ser8250", (CHAR *)"-c", (CHAR *)"1843200/2",
   (CHAR *)"-T8", (CHAR *)"-t8", (CHAR *)"-u13", (CHAR *)"a900,11",
   (CHAR *)"-u14", (CHAR *)"a908,11", (CHAR *)"-u15", (CHAR *)"a910,11",
   (CHAR *)"-u16", (CHAR *)"a918,11", (CHAR *)"-u17", (CHAR *)"a920,11",
   (CHAR *)"-u18", (CHAR *)"a928,11", (CHAR *)"-u19", (CHAR *)"a930,11",
   (CHAR *)"-u20", (CHAR *)"a938,11", (CHAR *)NULL};
#endif // CFG_ENABLE_PCI_DETECTION

/*----------------------------------------------------------------------------
  Local Prototypes declaration
----------------------------------------------------------------------------*/
static E_ERR_T sys_CreateFolderStructure(VOID);
static VOID   sys_sigHandler(int sig_number);
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_Initialization

 DESCRIPTION

  This routine will initialize all component items

 CALLED BY

  Application main

 CALLS

  SYS_Main

 PARAMETER

  pconfigfilename   [in] pointer to config filename

 RETURN

   E_ERR_Success                 the routine executed successfully
   E_ERR_SWC_InitializationFail  Initialization Fail

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    07-Sep-2009      Created initial revision
   Bryan Chong    27-Mar-2012      Add input parameter for config filename

-----------------------------------------------------------------------------*/
E_ERR_T SYS_Initialization(CHAR *pconfigfilename)
{
  E_ERR_T rstatus = E_ERR_Success;
  char cLog[1000];
  #ifdef CFG_PRN_ERR
  CHAR errmsgbuff[ERR_STR_SZ] = {0};
  #endif // CFG_PRN_ERR

  struct stat tPara;

  SYS_isExitAppl = E_TYPE_False;

  rstatus = SYS_InitCtrlBlk();
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  #if CFG_ENABLE_READ_CONFIG_FILE_PARAMETER
  if((pconfigfilename != TYP_NULL) &&
     (strlen(pconfigfilename) < SYS_CONFIGFILENAME_SZ))
    strcpy(pSYS_CB->config_filename, pconfigfilename);
  #endif // CFG_ENABLE_READ_CONFIG_FILE_PARAMETER

  rstatus = SYS_CreateHeaderString();
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  // initialize system log file
//  pFLG_CtrlBlk = new FLG("/dev/shmem/", "systemlog.txt");
//  SYS_ASSERT_RETURN(pFLG_CtrlBlk->CtrlBlk.isInit == E_TYPE_No,
//                    E_ERR_InvalidParameter);

//  rstatus = SYS_ThrdInit(E_SYS_TLB_FLG_SystemLog);
//  if(rstatus != E_ERR_Success)
//  {
//    #ifdef CFG_DEBUG_MSG
//    printf("ERR  [SYS] SYS_Initialization, create system log thread fail\n");
//    #endif // CFG_DEBUG_MSG
//    return rstatus;
//  }

//  pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SystemLevel,
//   "SYS_Initialization, system log starts\n");

  // Hardware peripherals control
  rstatus =  HWP_Initialization();
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  // Serial Link Management
  rstatus = SER_Initialization();
  if(rstatus != E_ERR_Success)
  {
    #ifdef CFG_DEBUG_MSG
    printf("ERR  [SYS] SYS_Initialization, serial initialization fail\n");
    #endif // CFG_DEBUG_MSG
    return rstatus;
  }

  rstatus =  HWP_LCM_Initialization();
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  // Build directory strucutre
  rstatus = sys_CreateFolderStructure();
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  /* Initialize event log object */
  g_pEventLog = new CLOG((CHAR *)CFG_LOG_PREFIX_EV);
  /* Log start RTU Application */
  g_pEventLog->LogStart();

//  #ifdef CFG_ENABLE_SPA
//  // SCPI Parser management
//  rstatus = SPA_Initialization();
//  if(rstatus != E_ERR_Success)
//  {
//    #ifdef CFG_DEBUG_MSG
//    printf("ERR  [SYS] SYS_Initialization, SCPI initialization fail\n");
//    #endif // CFG_DEBUG_MSG
//    return rstatus;
//  } // SPA_Initialization
//
//  rstatus = SYS_ThrdInit(E_SYS_TLB_SpaReceiveCmd);
//  if(rstatus != E_ERR_Success)
//  {
//    #ifdef CFG_DEBUG_MSG
//    printf("ERR  [SYS] SYS_Initialization, create SCPI parser receiving "
//           "command thread fail\n");
//    #endif // CFG_DEBUG_MSG
//    return rstatus;
//  }
//
//  // create receiving message queue
//  rstatus = SYS_CreateMsgQueue(E_SYS_MSQL_SPAResultToSerOutput);
//  if(rstatus != E_ERR_Success)
//  {
//    #ifdef CFG_PRN_ERR
//    printf("ERR  [SYS] SYS_Initialization, fail to create message queue\n");
//    #endif // CFG_DEBUG_MSG
//    return rstatus;
//  }
//
//  rstatus = SER_SendMsg(SER_SCPICMD_PORT, (CHAR *)"\n\n");
//  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
//
//  rstatus = SER_SendMsg(SER_SCPICMD_PORT, (CHAR *)CFG_SYS_REVISION_STRING);
//  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
//
//  rstatus = SYS_ThrdInit(E_SYS_TLB_SerialReceive);
//  if(rstatus != E_ERR_Success)
//  {
//    #ifdef CFG_DEBUG_MSG
//    printf("ERR  [SYS] SYS_Initialization, create serial port receiving command "
//           "thread fail\n");
//    #endif // CFG_DEBUG_MSG
//    return rstatus;
//  }
//
//  rstatus = SYS_ThrdInit(E_SYS_TLB_SerialTransmit);
//  if(rstatus != E_ERR_Success)
//  {
//    #ifdef CFG_DEBUG_MSG
//    printf("ERR  [SYS] SYS_Initialization, create serial port transmit result "
//           "thread fail\n");
//    #endif // CFG_DEBUG_MSG
//    return rstatus;
//  }
//  #endif // CFG_ENABLE_SPA

  // Initialize timer
  rstatus = Init_Timer();
  if(rstatus != E_ERR_Success)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR [SYS] SYS_Initialization, initialize timer fail\n");
    #endif // CFG_PRN_ERR
    return rstatus;
  }
  rstatus = SYS_ThrdInit(E_SYS_TLB_CmmTimer);
  if(rstatus != E_ERR_Success)
  {
    #ifdef CFG_DEBUG_MSG
    printf("ERR  [SYS] SYS_Initialization, create CMM Timer thread fail\n");
    #endif // CFG_DEBUG_MSG
    return rstatus;
  }

  rstatus = CGF_Initialization();
  if(rstatus != E_ERR_Success)
  {
    #ifdef CFG_PRN_ERR
    printf("[SYS] ERR, initialize config file fail\n");
    #endif // CFG_PRN_ERR
    return rstatus;
  }

  rstatus = GetSystemConfig(pSYS_CB->config_filename);
  if(rstatus != E_ERR_Success)
  {
    #ifdef CFG_PRN_ERR
    ERR_GetMsgString(rstatus, errmsgbuff);
    printf("ERR  [SYS] SYS_Initialization, %s\n", errmsgbuff);
    #endif // CFG_PRN_ERR
    LogInitError((CHAR *)"GetSystemConfig Error");
    return rstatus;
  }

  // Get RTU Configuration file creation time
  if(stat (pSYS_CB->config_filename, &tPara) == EOK)
  {
    //use local time, get high and low word respectively
    g_tRTUPerPara.unCFGDownloadUTCHigh =
      (UINT16)((tPara.st_mtime & 0xFFFF0000)>> 16);
    g_tRTUPerPara.unCFGDownloadUTCLow =
      (UINT16)(tPara.st_mtime & 0x0000FFFF );
  }
  else
  {
    return E_ERR_SYS_GetConfigFileDownloadTimeFail;
  }

//  #if CFG_ENABLE_PCI_DETECTION
//  // initiate devc-8250 manager for PCI based serial controller
//  rstatus = SYS_InitiateDevcSer8250Manager();
//  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
//  #endif // (CFG_ENABLE_PCI_DETECTION)

  rstatus = LNC_Initialization();
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  rstatus = NTP_Initialization();
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  rstatus = SYS_ThrdInit(E_SYS_TLB_NTPClient);
  if(rstatus != E_ERR_Success)
  {
    #ifdef CFG_DEBUG_MSG
    printf("ERR  [SYS] SYS_Initialization, create scheduler thread fail\n");
    #endif // CFG_DEBUG_MSG
    return rstatus;
  }

  rstatus = SYS_ThrdInit(E_SYS_TLB_NTPClient_LinkManagement);
  if(rstatus != E_ERR_Success)
  {
    #ifdef CFG_DEBUG_MSG
    printf("ERR  [SYS] SYS_Initialization, create link managent thread fail\n");
    #endif // CFG_DEBUG_MSG
    return rstatus;
  }

  // create transmitting message queue
  rstatus = SYS_CreateMsgQueue(E_SYS_MSQL_NTPC_TxExternalSystemMsg);
  if(rstatus != E_ERR_Success)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SYS] SYS_Initialization, fail to create message queue "
           "for E_SYS_MSQL_NTPC_TxExternalSystemMsg\n");
    #endif // CFG_DEBUG_MSG
    return rstatus;
  }

  #ifdef NOT_USED
  // initialize mutexes
  rstatus = SYS_MutexInit(E_SYS_MTX_SER_SendWithReply);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  rstatus = SYS_MutexInit(E_SYS_MTX_LAN_SendWithReply);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
  #endif // NOT_USED

  //Initialize LAN for Common Module
  rstatus = LANInit();
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  // Display IP1 address on LCD
  rstatus = HWP_LCM_PrnMsgCtrl(E_HWP_LCM_MSG_IPAddress);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  // Create Debug Log
  if(g_tDebugLogFlag.bLogFlag == true)
  {
    g_pDebugLog = new CLOG((CHAR *)CFG_LOG_PREFIX_DB);
    g_pDebugLog->LogStart();  /* Log start RTU Application */

//    rstatus = pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SystemLevel,
//                FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//                "%s", pSYS_CB->headerline);
    sprintf(cLog,"%s", pSYS_CB->headerline);
    g_pDebugLog->LogMessage(cLog);
//    SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
  }

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SYS))
  printf("[SYS] SYS_Initialization, spawn watchdog  task\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SYS))
  WDG_Initialization();

  // create watchdog server thread
  rstatus = SYS_ThrdInit(E_SYS_TLB_CmmWatchdog);
  if(rstatus != E_ERR_Success)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [CMM] WatchdogTaskSpawn, create CMM Watchdog Server thread "
           "fail\n");
    #endif // CFG_PRN_ERR
  }

  // Initial all SWC
  if(SWCInitialization() == ERROR)
  {
    printf("SWCInitialization error\n");
    LogInitError((CHAR *)"SWCInitialization error");
    return E_ERR_SWC_InitializationFail;
  }

  // RMM
  //Initial Redundancy Management module and spawn task
  if(RMMInitialization() == ERROR)
  {
    StopAllSWC();
    printf("RMMInitialization error\n");
    LogInitError((CHAR *)"RMMInitialization error");
    return E_ERR_SWC_InitializationFail;
  }

  // ServerPollTaskSpawn
  if(ServerPollTaskSpawn() == ERROR)
  {
    StopAllSWC();
    StopRMM();
    printf("ServerPollTaskSpawn ERROR\n");
    LogInitError((CHAR *)"ServerPollTaskSpawn ERROR");
    return E_ERR_SPM_InitializationFail;          //Should be exit here.  //040429 Yu Wei
  }

  // Spawn Threads
  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SYS))
  printf("[SYS] SYS_Initialization, spawn mmm task\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SYS))
  MMMTaskSpawn();



  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SYS))
  printf("[SYS] SYS_Initialization, spawn logtask\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SYS))
  LogTaskSpawn();    //Spawn log task.

  // ADD a signal handler
  // make sure program won't exit ,In case of writing data to a closed socket
  // WARNING: removing this line will cause application to exit undeterminely
  // Effect of removing this line: Closing LAN simulator will cause application
  //   to exit undeterminely.
  signal(SIGPIPE , sys_sigHandler);

  return E_ERR_Success;
} // SYS_Initialization
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_ThrdInit

 DESCRIPTION

  This routine will initialize the thread control block

 CALLED BY

  SYS_Initialization

 CALLS

  None

 PARAMETER

  thrd_lbl                               thread label defined by E_SYS_TLB type

 RETURN

  E_ERR_Success                          the routine executed successfully
  E_ERR_SYS_InvalidThrdLabel             invalid thread label
  E_ERR_SYS_ThrdAttrInitFail             fail to set thread attribute
  E_ERR_SYS_ThrdAttrSetInheritSchdFail   fail to set inherit schedule
  E_ERR_SYS_ThrdAttrSetSchdPolicyFail    fail to set schedule policy
  E_ERR_SYS_ThrdAttrSetSchdParamFail     fail to set schedule parameters
  E_ERR_SYS_ThrdAttrSetStackSzFail       fail to set thread stack size
  E_ERR_SYS_CreateThrdFail               create thread fail
  E_ERR_SYS_InvalidThrdID                invalid thread ID

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    08-Sep-2009      Created initial revision
   Bryan Chong    27-Apr-2012      Include create thread for SWC which
                                   consist of CFG_NUM_OF_THREADS_PER_SWC
                                   number of threads

-----------------------------------------------------------------------------*/
E_ERR_T SYS_ThrdInit(E_SYS_TLB thrd_lbl)
{

  struct thread_attribute_st *pthrdlut;
  struct thrd_ctrl_st *ptcb;
  struct sched_param params;
  UINT8 swc_mgmt_loc;
  UINT8 swcindex;

  switch(thrd_lbl)
  {
    /*
    case E_SYS_TLB_SYS_Scheduler:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_SYS_Scheduler];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_SYS_Scheduler];
      break;*/
//    case E_SYS_TLB_FLG_SystemLog:
//      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_FLG_SystemLog];
//      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_FLG_SystemLog];
//      break;

//    #ifdef CFG_ENABLE_SPA
//    case E_SYS_TLB_SerialReceive:
//      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_SerialReceive];
//      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_SerialReceive];
//      break;
//
//    case E_SYS_TLB_SerialTransmit:
//      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_SerialTransmit];
//      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_SerialTransmit];
//      break;
//
//    case E_SYS_TLB_SpaReceiveCmd:
//      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_SpaReceiveCmd];
//      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_SpaReceiveCmd];
//      break;
//    #endif // CFG_ENABLE_SPA

    case E_SYS_TLB_NTPClient:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_NTPClient];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_NTPClient];
      break;

    case E_SYS_TLB_NTPClient_LinkManagement:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_NTPClient_LinkManagement];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_NTPClient_LinkManagement];
      break;

    case E_SYS_TLB_CmmWatchdog:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_CmmWatchdog];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_CmmWatchdog];
      break;

    case E_SYS_TLB_CmmTimer:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_CmmTimer];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_CmmTimer];
      break;

    case E_SYS_TLB_CmmLog:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_CmmLog];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_CmmLog];
      break;

    case E_SYS_TLB_SpmMain_1:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_SpmMain_1];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_SpmMain_1];
      break;

    case E_SYS_TLB_SpmMain_2:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_SpmMain_2];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_SpmMain_2];
      break;

    case E_SYS_TLB_SpmMain_3:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_SpmMain_3];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_SpmMain_3];
      break;

    case E_SYS_TLB_SpmMain_4:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_SpmMain_4];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_SpmMain_4];
      break;

    case E_SYS_TLB_SpmMain_5:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_SpmMain_5];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_SpmMain_5];
      break;

    case E_SYS_TLB_SpmCopy_1:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_SpmCopy_1];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_SpmCopy_1];
      break;

    case E_SYS_TLB_SpmCopy_2:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_SpmCopy_2];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_SpmCopy_2];
      break;

    case E_SYS_TLB_SpmCopy_3:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_SpmCopy_3];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_SpmCopy_3];
      break;

    case E_SYS_TLB_SpmCopy_4:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_SpmCopy_4];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_SpmCopy_4];
      break;

    case E_SYS_TLB_SpmCopy_5:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_SpmCopy_5];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_SpmCopy_5];
      break;

    case E_SYS_TLB_SWC_NTPClientComm:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_SWC_NTPClientComm];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_SWC_NTPClientComm];
      break;

    case E_SYS_TLB_RMM_OthRTULinkSock1:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_RMM_OthRTULinkSock1];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_RMM_OthRTULinkSock1];
      break;

    case E_SYS_TLB_RMM_OthRTULinkSock2:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_RMM_OthRTULinkSock2];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_RMM_OthRTULinkSock2];
      break;

    case E_SYS_TLB_RMM_OthRTULinkTask:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_RMM_OthRTULinkTask];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_RMM_OthRTULinkTask];
      break;

    case E_SYS_TLB_RMM_OthRTUConnTask1:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_RMM_OthRTUConnTask1];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_RMM_OthRTUConnTask1];
      break;

    case E_SYS_TLB_RMM_OthRTUConnTask2:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_RMM_OthRTUConnTask2];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_RMM_OthRTUConnTask2];
      break;

    case E_SYS_TLB_RMM_StdbyReadSock1:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_RMM_StdbyReadSock1];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_RMM_StdbyReadSock1];
      break;

    case E_SYS_TLB_RMM_StdbyReadSock2:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_RMM_StdbyReadSock2];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_RMM_StdbyReadSock2];
      break;

    case E_SYS_TLB_RMM_ServerSocketTask:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_RMM_ServerSocketTask];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_RMM_ServerSocketTask];
      break;

    case E_SYS_TLB_RMM_Task:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_RMM_Task];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_RMM_Task];
      break;

    case E_SYS_TLB_MMM_Task:
      pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_MMM_Task];
      ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_MMM_Task];
      break;

    default:

      if(thrd_lbl > (E_SYS_TLB_SWC_MgmtTask +
           (CFG_NUM_OF_THREADS_PER_SWC * CFG_TOTAL_NUM_OF_SWC)))
      {
        #ifdef CFG_PRN_ERR
        printf("ERR  [SYS] SYS_ThrdInit, invalid thread label\n");
        #endif // CFG_PRN_ERR
        return E_ERR_SYS_InvalidThrdLabel;
      }
      swcindex = ((thrd_lbl - E_SYS_TLB_SWC_MgmtTask) /
                  CFG_NUM_OF_THREADS_PER_SWC);
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SYS)
      printf("[SYS] SYS_ThrdInit, thrd_lbl = %d, swcidx %d\n",
             thrd_lbl, swcindex);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SYS)
      switch((thrd_lbl - E_SYS_TLB_SWC_MgmtTask) %
             CFG_NUM_OF_THREADS_PER_SWC)
      {
        case 0:
          swc_mgmt_loc =
            CFG_NUM_OF_THREADS_PER_SWC * swcindex;
          pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_SWC_MgmtTask];
          ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_SWC_MgmtTask + swc_mgmt_loc];
          ptcb->arg = (VOID *)&g_apSWCObject[swcindex]->m_nSWCID;
          #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_SYS)
          printf("[SYS] SYS_ThrdInit, E_SYS_TLB_SWC_MgmtTask\n"
                 "  SWC idx %d, arg = 0x%08x\n",
                 swcindex, ptcb->arg);
          #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SYS)
          break;

        case 1:
          swc_mgmt_loc =
            CFG_NUM_OF_THREADS_PER_SWC * swcindex;

          pthrdlut = &sys_thrd_attr_lut[E_SYS_TLB_SWC_ServerTask];
          ptcb = &sys_CtrlBlk.thrd_ctrl[E_SYS_TLB_SWC_ServerTask +
                                        swc_mgmt_loc];
          ptcb->arg = (VOID *)&g_apSWCObject[swcindex]->m_nSWCID;
          #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_SYS)
          printf("[SYS] SYS_ThrdInit, E_SYS_TLB_SWC_ServerTask\n"
                 "  SWC idx %d, arg = 0x%08x\n",
                 swcindex, ptcb->arg);
          #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SYS)
          break;

        default:
          #ifdef CFG_PRN_ERR
          printf("ERR  [SYS] SYS_ThrdInit, not supporting thread more than\n"
                 "%d\n", CFG_NUM_OF_THREADS_PER_SWC);
          #endif // CFG_PRN_ERR
          break;
      }// switch((thrd_lbl - E_SYS_TLB_SWC_MgmtTask) %
             // CFG_NUM_OF_THREADS_PER_SWC)

  } // switch(thrd_lbl)

  if(EOK != pthread_attr_init(&ptcb->attr))
  {
    #ifdef CFG_DEBUG_MSG
    printf("ERR  [SYS] sys_ThrdCBInit attribute init fail\n");
    #endif // CFG_DEBUG_MSG
    return E_ERR_SYS_ThrdAttrInitFail;
  }

  if(EOK !=
    (pthread_attr_setinheritsched(&ptcb->attr, pthrdlut->inherit_schd)))
  {
    #ifdef CFG_DEBUG_MSG
    printf("ERR  [SYS] sys_ThrdCBInit, set inherit schedule fail\n");
    #endif //  CFG_DEBUG_MSG
    return E_ERR_SYS_ThrdAttrSetInheritSchdFail;
  }

  if((EOK != pthread_attr_setschedpolicy(&ptcb->attr, pthrdlut->policy)))
  {
    #ifdef CFG_DEBUG_MSG
    printf("ERR  [SYS] sys_ThrdCBInit, set schedule policy fail\n");
    #endif //  CFG_DEBUG_MSG
    return E_ERR_SYS_ThrdAttrSetSchdPolicyFail;
  }

  params.sched_priority = pthrdlut->priority;

  if((EOK != pthread_attr_setschedparam(&ptcb->attr, &params)))
  {
    #ifdef CFG_DEBUG_MSG
    printf("ERR  [SYS] sys_ThrdCBInit, set schedule parameters fail\n");
    #endif //  CFG_DEBUG_MSG
    return E_ERR_SYS_ThrdAttrSetSchdParamFail;
  }

  if((pthread_attr_setstacksize(&ptcb->attr, pthrdlut->stack_sz)) != EOK)
  {
    printf("setstachsize erro \n");
    #ifdef CFG_DEBUG_MSG
    printf("ERR  [SYS] sys_ThrdCBInit, set stack size fail\n");
    #endif //  CFG_DEBUG_MSG
    return E_ERR_SYS_ThrdAttrSetStackSzFail;
  }

  ptcb->start_routine = pthrdlut->start_routine;
  //ptcb->arg           = pthrdlut->arg;


  // create the relevant thread
  if(EOK !=
     pthread_create(&ptcb->thrd_id, &ptcb->attr, ptcb->start_routine,
                    (VOID *)ptcb->arg))
  {
    #ifdef CFG_DEBUG_MSG
    printf("ERR  [SYS] sys_ThrdCBInit, create thread %s fail\n",
           ptcb->thrd_name);
    #endif //  CFG_DEBUG_MSG
    return E_ERR_SYS_CreateThrdFail;
  }

  // check for valid thread ID
  if(ptcb->thrd_id == (pthread_t)NULL)
  {
    #ifdef CFG_DEBUG_MSG
    printf("ERR  [SYS] sys_ThrdCBInit, invalid thread id (0x%x)\n",
           ptcb->thrd_id);
    #endif //  CFG_DEBUG_MSG
    return E_ERR_SYS_InvalidThrdID;
  }

  // copy label and thread into thread control block
  ptcb->thrd_label = thrd_lbl;
  memcpy(ptcb->thrd_name,
         pthrdlut->thrd_name, sizeof(pthrdlut->thrd_name));

  if(thrd_lbl < E_SYS_TLB_SWC_MgmtTask)
  {
    pthread_setname_np(ptcb->thrd_id, ptcb->thrd_name);
  }else{
    strcat(ptcb->thrd_name, g_apSWCObject[swcindex]->m_acSWCName);
    pthread_setname_np(ptcb->thrd_id, ptcb->thrd_name);
  }

  return E_ERR_Success;
} // SYS_ThrdInit

//#ifdef NOT_USED
///*-----------------------------------------------------------------------------
//
// PUBLIC ROUTINE
//
//  SYS_MutexInit
//
// DESCRIPTION
//
//  This routine will initialize the mutex block
//
// CALLED BY
//
//  SYS_Initialization
//
// CALLS
//
//  None
//
// PARAMETER
//
//  thrd_lbl                               thread label defined by E_SYS_TLB type
//
// RETURN
//
//  E_ERR_Success                          the routine executed successfully
//  E_ERR_SYS_InvalidMutexID               invalid mutex ID
//
// AUTHOR
//
//  Bryan KW Chong
//
//
// HISTORY
//
//    NAME            DATE                    REMARKS
//
//   Bryan Chong    23-Apr-2010      Created initial revision
//
//-----------------------------------------------------------------------------*/
//E_ERR_T SYS_MutexInit(E_SYS_MTX mutex_id)
//{
//  switch(mutex_id)
//  {
//    case E_SYS_MTX_SER_SendWithReply:
//      pthread_mutexattr_init(
//        &pSYS_CB->mutex_ctrl[E_SYS_MTX_SER_SendWithReply].attr);
//      pthread_mutex_init(&pSYS_CB->mutex_ctrl[E_SYS_MTX_SER_SendWithReply].mtx,
//                       &pSYS_CB->mutex_ctrl[E_SYS_MTX_SER_SendWithReply].attr);
//      strcpy(pSYS_CB->mutex_ctrl[E_SYS_MTX_SER_SendWithReply].mutex_name,
//        "E_SYS_MTX_SER_SendWithReply");
//      break;
//
//    case E_SYS_MTX_LAN_SendWithReply:
//      pthread_mutexattr_init(
//        &pSYS_CB->mutex_ctrl[E_SYS_MTX_LAN_SendWithReply].attr);
//      pthread_mutex_init(&pSYS_CB->mutex_ctrl[E_SYS_MTX_LAN_SendWithReply].mtx,
//                       &pSYS_CB->mutex_ctrl[E_SYS_MTX_LAN_SendWithReply].attr);
//      strcpy(pSYS_CB->mutex_ctrl[E_SYS_MTX_LAN_SendWithReply].mutex_name,
//        "E_SYS_MTX_LAN_SendWithReply");
//      break;
//
//    default:
//      #ifdef CFG_PRN_ERR
//      printf("ERR  [SYS] SYS_MutexInit, invalid mutex ID\n");
//      #endif // CFG_PRN_ERR
//      return E_ERR_SYS_InvalidMutexID;
//  }
//
//  return E_ERR_Success;
//} // SYS_MutexInit
//#endif // NOT_USED

/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_CreateMsgQueue

 DESCRIPTION

  This routine will create message queue according to the
  sys_MessageQueueCtrlLut look-up table

 CALLED BY

  FLG constructor
  SPA_Initialization
  SYS_Initialization

 CALLS

  mq_open  Open a message queue

 PARAMETER

  E_SYS_MSQL  [in] message queue label defined by E_SYS_MSQL type

 RETURN

   E_ERR_Success      the routine executed successfully
   E_ERR_SYS_InvalidMsgQLabel
     invalid message queue label or not supported message queue label

   E_ERR_InvalidNullPointer  invalid null pointer

   E_ERR_SYS_InvalidMessageQueueID
     fail to open message queue and mq_open return negative value.

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

  Bryan Chong    08-Sep-2009      Created initial revision

-----------------------------------------------------------------------------*/
E_ERR_T SYS_CreateMsgQueue(E_SYS_MSQL msq_label)
{
  struct msq_lut_st *plut;
  struct msq_ctrl_st *pcb;

  plut = &sys_MessageQueueCtrlLut[msq_label];

  switch(msq_label)
  {
    // currently supported message queue
//    #ifdef CFG_ENABLE_SPA
//    case E_SYS_MSQL_SerCmdToSPAParseCmd:
//    case E_SYS_MSQL_SPAResultToSerOutput:
//    #endif // CFG_ENABLE_SPA
    case E_SYS_MSQL_FLG_SystemLogReceiving:
    //case E_SYS_MSQL_NTPC_RxExternalSystemMsg:
    case E_SYS_MSQL_NTPC_TxExternalSystemMsg:
      pcb = &pSYS_CB->msq_ctrl[msq_label];
      break;

    default:
      return E_ERR_SYS_InvalidMsgQLabel;
  }

  if((pcb == TYP_NULL) || (plut == TYP_NULL))
    return E_ERR_InvalidNullPointer;

  pcb->msq_id =
      mq_open((const CHAR *)plut->msq_pathname, plut->oflag,
              plut->mode, NULL);

  if(pcb->msq_id < 0)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SYS] SYS_CreateMsgQueue, fail to open message queue\n");
    #endif // CFG_PRN_ERR
    return E_ERR_SYS_InvalidMessageQueueID;
  }

  pcb->pmsq_desc = plut;

  return E_ERR_Success;
} // SYS_CreateMsgQueue
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_InitCtrlBlk

 DESCRIPTION

  This routine will initialize the system management control block.

 CALLED BY

  Application main

 CALLS

  [TBD]

 PARAMETER

  None

 RETURN

   E_ERR_Success      the routine executed successfully

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    08-Mar-2010      Created initial revision

-----------------------------------------------------------------------------*/
E_ERR_T SYS_InitCtrlBlk(VOID)
{
  pSYS_CB = &sys_CtrlBlk;
  memset(pSYS_CB, 0, sizeof(SYS_CB_T));

  strcpy(pSYS_CB->config_filename, (CHAR *)CFG_CONFIG_FILE_DEFAULT);

  return E_ERR_Success;
} // SYS_InitCtrlBlk
//#if (CFG_ENABLE_PCI_DETECTION)
///*-----------------------------------------------------------------------------
//
// PUBLIC ROUTINE
//
//  SYS_InitiateDevcSer8250Manager
//
// DESCRIPTION
//
//  This routine will initiate dev-ser8250 manager for the PCI-based serial
//  controller.
//
//  Supporting auto detecting of PCI Serial Controller card and issue the
//  correct comment to spawnv routine. Successful initialization of
//  devc-ser8250 driver will return a process ID.
//  acTemp will show the correct command issued at command line.
//  One-card or two-card configurations can be determined by index number.
//  index 0: For 1-card configuration. The card can be at either one of the slot.
//           Different slot will have different interrupt number depends on
//           different device ID
//  index 1: For 2-card configuration.
//  The index number information can be listed out using command "pci -v" at
//  command line.
//
// CALLED BY
//
//  SYS_Initialization
//
// CALLS
//
//  None
//
// PARAMETER
//
//  None
//
// RETURN
//
//  E_ERR_Success                          the routine executed successfully
//  E_ERR_SYS_InvalidTotalComPortPerCard   invalid total number of COM port per
//                                         card
//  E_ERR_SYS_InvalidProcessID             invalid process ID
//
// AUTHOR
//
//  Bryan KW Chong
//
//
// HISTORY
//
//    NAME            DATE                    REMARKS
//
//   Bryan Chong    10-Mar-2010      Created initial revision
//
//-----------------------------------------------------------------------------*/
//E_ERR_T SYS_InitiateDevcSer8250Manager(VOID)
//{
//  char acTemp[200];
//  UINT8 uccomnum = 0;
//  INT32 pid;
//
//  if(g_tRTUConfig.nNumberOfSerialCOMCard != 0)
//  {
//    for(UINT8 uccnt = 0; uccnt < pHWP_CB->ucnumOfPciCardFound; uccnt++)
//    {
//      uccomnum = CFG_HWP_PCI_SERIAL_COM_START_NUM +
//                 (CFG_HWP_TOTAL_COM_PORT_PER_CARD * uccnt);
//
//      if(CFG_HWP_TOTAL_COM_PORT_PER_CARD != 8)
//      {
//        #ifdef CFG_PRN_ERR
//        printf("[INM] GetRTUConfig, invalid total com port per card, %d\n",
//                CFG_HWP_TOTAL_COM_PORT_PER_CARD);
//        #endif // CFG_PRN_ERR
//        return E_ERR_SYS_InvalidTotalComPortPerCard;
//      }
//
//      memset(acTemp, 0, sizeof(acTemp));
//      switch(uccnt)
//      {
//        case 0: // index 0. 1-card configuration
//          sprintf(acTemp, "devc-ser8250 -c 1843200/2 -T8 -t8 -u%d a900,%d "
//                  "-u%d a908,%d -u%d a910,%d -u%d a918,%d -u%d a920,%d -u%d "
//                  "a928,%d -u%d a930,%d -u%d a938,%d",
//            uccomnum, pHWP_CB->serialComCard[uccnt].usirqNumber,
//            (uccomnum + 1), pHWP_CB->serialComCard[uccnt].usirqNumber,
//            (uccomnum + 2), pHWP_CB->serialComCard[uccnt].usirqNumber,
//            (uccomnum + 3), pHWP_CB->serialComCard[uccnt].usirqNumber,
//            (uccomnum + 4), pHWP_CB->serialComCard[uccnt].usirqNumber,
//            (uccomnum + 5), pHWP_CB->serialComCard[uccnt].usirqNumber,
//            (uccomnum + 6), pHWP_CB->serialComCard[uccnt].usirqNumber,
//            (uccomnum + 7), pHWP_CB->serialComCard[uccnt].usirqNumber);
//
//          pHWP_CB->serialComCard[uccnt].uccom_start_number = uccomnum;
//          pHWP_CB->serialComCard[uccnt].uccom_end_number = (uccomnum + 7);
//
//          #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
//          printf("[INM] GetRTUConfig, devc-ser8250 cmd: %s\n", acTemp);
//          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
//          switch(pHWP_CB->serialComCard[uccnt].usirqNumber)
//          {
//             case 5:
//               pid = spawnv(P_NOWAIT, "/sbin/devc-ser8250",
//                                      devc_ser8250_int5_index0);
//               break;
//             case 7:
//               pid = spawnv(P_NOWAIT, "/sbin/devc-ser8250",
//                           devc_ser8250_int7_index0);
//               break;
//             case 10:
//               pid = spawnv(P_NOWAIT, "/sbin/devc-ser8250",
//                            devc_ser8250_int10_index0);
//               break;
//
//             case 11:
//               pid = spawnv(P_NOWAIT, "/sbin/devc-ser8250",
//                            devc_ser8250_int11_index0);
//                break;
//             default:
//               #ifdef CFG_PRN_ERR
//               printf("ERR  [INM] GetRTUConfig, int %d is not supported at "
//                      "index %d\n",
//                      pHWP_CB->serialComCard[uccnt].usirqNumber, uccnt);
//               #endif // CFG_PRN_ERR
//               return E_ERR_SYS_InvalidDevc8250IrqNumber;
//          } // switch(pHWP_CB->serialComCard[uccnt].usirqNumber)
//          #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
//          printf("[INM] GetRTUConfig, index %d devc-ser8250 pid: %d\n",
//                 uccnt, pid);
//          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
//          break; // case 0
//
//        case 1: // index 1. 2-card configuration
//
//          sprintf(acTemp, "devc-ser8250 -c 1843200/2 -T8 -t8 -u%d a600,%d "
//                  "-u%d a608,%d -u%d a610,%d -u%d a618,%d -u%d a620,%d -u%d "
//                  "a628,%d -u%d a630,%d -u%d a638,%d",
//            uccomnum, pHWP_CB->serialComCard[uccnt].usirqNumber,
//            (uccomnum + 1), pHWP_CB->serialComCard[uccnt].usirqNumber,
//            (uccomnum + 2), pHWP_CB->serialComCard[uccnt].usirqNumber,
//            (uccomnum + 3), pHWP_CB->serialComCard[uccnt].usirqNumber,
//            (uccomnum + 4), pHWP_CB->serialComCard[uccnt].usirqNumber,
//            (uccomnum + 5), pHWP_CB->serialComCard[uccnt].usirqNumber,
//            (uccomnum + 6), pHWP_CB->serialComCard[uccnt].usirqNumber,
//            (uccomnum + 7), pHWP_CB->serialComCard[uccnt].usirqNumber);
//
//          pHWP_CB->serialComCard[uccnt].uccom_start_number = uccomnum;
//          pHWP_CB->serialComCard[uccnt].uccom_end_number = (uccomnum + 7);
//
//          #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
//          printf("[INM] GetRTUConfig, devc-ser8250 cmd: %s\n", acTemp);
//          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
//
//          switch(pHWP_CB->serialComCard[uccnt].usirqNumber)
//          {
//            case 5:
//              pid = spawnv(P_NOWAIT, "/sbin/devc-ser8250",
//                           devc_ser8250_int5_index1);
//              #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
//              printf("[INM] GetRTUConfig, devc-ser8250 pid: %d\n", pid);
//              #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
//              break;
//
//            case 11:
//              pid = spawnv(P_NOWAIT, "/sbin/devc-ser8250",
//                           devc_ser8250_int11_index1);
//              break;
//
//            default:
//              #ifdef CFG_PRN_ERR
//              printf("ERR  [INM] GetRTUConfig, int %d is not supported at index "
//                     "%d\n",
//                     pHWP_CB->serialComCard[uccnt].usirqNumber, uccnt);
//              #endif // CFG_PRN_ERR
//              return E_ERR_SYS_InvalidDevc8250IrqNumber;
//          } // switch(pHWP_CB->serialComCard[uccnt].usirqNumber)
//
//          #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
//          printf("[INM] GetRTUConfig, index %d devc-ser8250 pid: %d\n",
//                 uccnt, pid);
//          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
//          break; // case 1:
//
//        default:
//          #ifdef CFG_PRN_ERR
//          printf("ERR  [INM] GetRTUConfig, index %d is currently not "
//                 "supported\n", uccnt);
//          #endif // CFG_DEBUG_MSG
//          return E_ERR_SYS_InvalidDevc8250IndexNumber;
//      }// switch (uccnt)
//
//
//      if(pid == TYP_ERROR)
//      {
//         #ifdef CFG_PRN_ERR
//         printf("ERR  [INM] GetRTUConfig, devc-ser8250 fail to start\n");
//         #endif // CFG_PRN_ERR
//         return E_ERR_SYS_InvalidProcessID;
//      }
//      strcpy(pSYS_CB->proc_ctrl[uccnt].appl_name, "devc-ser8250\0");
//      pSYS_CB->proc_ctrl[uccnt].pid = pid;
//
//    } // for (UINT8 uccnt = 0; uccnt < pHWP_CB->ucnumOfPciCardFound; uccnt++)
//  } // if(g_tRTUConfig.nNumberOfSerialCOMCard != 0)
//  return E_ERR_Success;
//} // SYS_InitiateDevcSer8250Manager
///*-----------------------------------------------------------------------------
//
// PUBLIC ROUTINE
//
//  SYS_DeinitializeDevc8250Manager
//
// DESCRIPTION
//
//  This routine will de-initiate dev-ser8250 manager for the PCI-based serial
//  controller.
//
// CALLED BY
//
//  Application
//
// CALLS
//
//  None
//
// PARAMETER
//
//  None
//
// RETURN
//
//  E_ERR_Success                          the routine executed successfully
//  E_ERR_SYS_InvalidProcessID             invalid process ID
//
// AUTHOR
//
//  Bryan KW Chong
//
//
// HISTORY
//
//    NAME            DATE                    REMARKS
//
//   Bryan Chong    10-Mar-2010      Created initial revision
//
//-----------------------------------------------------------------------------*/
//E_ERR_T SYS_DeinitializeDevc8250Manager(VOID)
//{
//  CHAR actemp[20];
//
//  if(g_tRTUConfig.nNumberOfSerialCOMCard <= 0)
//  {
//    #ifdef CFG_PRN_WARN
//    printf("WARN [SYS] SYS_DeinitializeDevc8250Manager, %d card or invalid "
//           "number of serial controller card to be de-initialized\n",
//           g_tRTUConfig.nNumberOfSerialCOMCard);
//    #endif // CFG_PRN_WARN
//    return E_ERR_Success;
//  }
//
//  for(UINT8 uccnt = 0; uccnt < pHWP_CB->ucnumOfPciCardFound; uccnt++)
//  {
//    if(pSYS_CB->proc_ctrl[uccnt].pid == TYP_NULL)
//      return E_ERR_SYS_InvalidProcessID;
//
//    sprintf(actemp, "kill %d",
//      pSYS_CB->proc_ctrl[uccnt].pid);
//
//    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SYS_EXIT))
//    printf("[SYS] SYS_DeinitializeDevc8250Manager, stopping %s, %s\n",
//      pSYS_CB->proc_ctrl[uccnt].appl_name, actemp);
//    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SYS_EXIT))
//
//    system(actemp);
//    pSYS_CB->proc_ctrl[uccnt].pid = TYP_NULL;
//  } // for(UINT8 uccnt = 0; uccnt < pHWP_CB->ucnumOfPciCardFound; uccnt++)
//
//  //delay(10);
//  return E_ERR_Success;
//}// SYS_DeinitializeDevc8250Manager
//#endif // (CFG_ENABLE_PCI_DETECTION)

/*-----------------------------------------------------------------------------

 PRIVATE ROUTINE

  sys_CreateFolderStructure

 DESCRIPTION

  This routine will create folder for log file within the /RTU folder

 CALLED BY

  SYS_Initialization

 CALLS

  None

 PARAMETER

  None

 RETURN

   E_ERR_Success                       the routine executed successfully
   E_ERR_SYS_FailToCreateSubDirectory  fail to create sub directory

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    19-Oct-2009      Created initial revision

-----------------------------------------------------------------------------*/
static E_ERR_T sys_CreateFolderStructure(VOID)
{
  INT32 nretval;

//  #ifdef CFG_ENABLE_CREATING_RTU_ROOT_DIRECTORY
//  // Check if RTU root directory exist
//  if(chdir(CFG_RTU_ROOT_FOLDER) == ERROR)
//  {
//    // if RTU root directory not exist, create.
//    #ifdef CFG_DEBUG_MSG
//    printf("[SYS] sys_CreateFolderStructure, creating %s folder...\n",
//           CFG_RTU_ROOT_FOLDER);
//    #endif // CFG_DEBUG_MSG
//
//
//    if(mkdir(CFG_RTU_ROOT_FOLDER, S_IRWXU| S_IRGRP| S_IXGRP| S_IROTH| S_IXOTH)
//       != 0)
//    {
//      #ifdef CFG_PRN_ERR
//      printf("[SYS] sys_CreateFolderStructure, creating %s folder...\n",
//             CFG_RTU_ROOT_FOLDER);
//      #endif // CFG_PRN_ERR
//      return E_ERR_SYS_FailToCreateRTUFolder;
//    }
//
//    if(chdir(CFG_RTU_ROOT_FOLDER) == ERROR)
//    {
//      #ifdef CFG_PRN_ERR
//      printf("[SYS] sys_CreateFolderStructure, fail to change to %s folder\n",
//             CFG_RTU_ROOT_FOLDER);
//      #endif // CFG_PRN_ERR
//      return E_ERR_SYS_FailToChangeDirectory;
//    }
//  }
//
//  #endif // CFG_ENABLE_CREATING_RTU_ROOT_DIRECTORY

  nretval = mkdir(CFG_RTU_LOG_FOLDER,
               S_IRWXU| S_IRGRP| S_IXGRP| S_IROTH| S_IXOTH);
  if(((nretval == ERROR) && (errno == EEXIST)) || (nretval == 0))
    return E_ERR_Success;

  #ifdef CFG_PRN_ERR
  printf("ERR  [SYS] sys_CreateFolderStructure, fail to create %s folder\n",
          CFG_RTU_LOG_FOLDER);
  #endif // CFG_PRN_ERR

  return E_ERR_SYS_FailToCreateSubDirectory;

} // sys_CreateFolderStructure
/*-----------------------------------------------------------------------------

 PRIVATE ROUTINE

  sys_sigHandler

 DESCRIPTION

  This routine will print a  warning message when closing socket pipe event
  triggered

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  None

 RETURN

  None

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    09-Mar-2010      Created initial revision

-----------------------------------------------------------------------------*/
static VOID sys_sigHandler(int sig_number)
{
  #ifdef CFG_PRN_ERR
  printf("WARN [SYS] sys_sigHandler, detected signal %d\n", sig_number);
  #endif // CFG_PRN_ERR
} // SYS_SigHandler
