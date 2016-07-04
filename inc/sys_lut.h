/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   sys_lut.h                                           D1.0.4

 COMPONENT

   n/a

 DESCRIPTION

   This file contains the private look-up table for system management


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     14-Jan-2010      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef _SYS_LUT_H_
#define _SYS_LUT_H_

/*----------------------------------------------------------------------------
  Look up table for creating threads
  The higher the number, the higher the priority

  The label sequence must be consistent with the definition from E_SYS_TLB

   thread label, thread name, priority, inherit scheduling,
    policy, stack size, stack_addr, thread routine, routine arguement
  Note: Try limiting thread name to 20 character as pidin can only display
        max 20 characters
----------------------------------------------------------------------------*/
static struct thread_attribute_st sys_thrd_attr_lut[E_SYS_TLB_EndOfList] = {
  {E_SYS_TLB_Reserved_0, "Reserved\0", 1, PTHREAD_EXPLICIT_SCHED,
    SCHED_RR, 10*1024, NULL, NULL, NULL},
//  {E_SYS_TLB_FLG_SystemLog,  "tFLG_SystemLog\0", 9, PTHREAD_EXPLICIT_SCHED,
//    SCHED_RR, 10*1024, NULL, &FLG_ReceivingThrdFunc, NULL},

//  #ifdef CFG_ENABLE_SPA
//  {E_SYS_TLB_SerialReceive,  "tSER_Receive\0", 15, PTHREAD_EXPLICIT_SCHED,
//    SCHED_RR, 10*1024, NULL, &SER_ReceiveHostCommandThrdFunc, NULL},
//  {E_SYS_TLB_SerialTransmit, "tSER_Transmit\0", 15, PTHREAD_EXPLICIT_SCHED,
//    SCHED_RR, 10*1024, NULL, &SER_TransmitResultThrdFunc,     NULL},
//  {E_SYS_TLB_SpaReceiveCmd,  "tSPA_Receive\0", 14, PTHREAD_EXPLICIT_SCHED,
//    SCHED_RR, 10*1024, NULL, &SPA_ReceiveCommandThrdFunc,     NULL},
//  #endif // CFG_ENABLE_SPA

  {E_SYS_TLB_NTPClient,
   "tNTPCli\0",
   9, PTHREAD_EXPLICIT_SCHED,
   SCHED_RR, 10*1024, NULL, &NTP_PollingThrdFunc,     NULL},
  {E_SYS_TLB_NTPClient_LinkManagement,
   "tNTPCli_LinkMgmt\0",
   9, PTHREAD_EXPLICIT_SCHED,
   SCHED_RR, 10*1024, NULL, &NTP_LinkManagementThrdFunc, NULL},

  {E_SYS_TLB_CmmWatchdog, "tWatchdog\0",        15, PTHREAD_EXPLICIT_SCHED,
    SCHED_RR, 10*1024, NULL, &WatchdogFunction,               NULL},
  {E_SYS_TLB_CmmTimer,       "tTimer\0",           10, PTHREAD_EXPLICIT_SCHED,
    SCHED_RR, 10*1024, NULL, &Timer, NULL},
  {E_SYS_TLB_CmmLog,         "tLog\0",             20, PTHREAD_EXPLICIT_SCHED,
    SCHED_RR, 50*1024, NULL, &LogTaskThread, NULL},
  {E_SYS_TLB_SpmMain_1,      "tSPM_Main_1\0",      11, PTHREAD_EXPLICIT_SCHED,
    SCHED_RR, 50*1024, NULL, &ServerPoll1MainTask,            NULL},
  {E_SYS_TLB_SpmMain_2,      "tSPM_Main_2\0",      11, PTHREAD_EXPLICIT_SCHED,
    SCHED_RR, 50*1024, NULL, &ServerPoll2MainTask,            NULL},
  {E_SYS_TLB_SpmMain_3,      "tSPM_Main_3\0",      11, PTHREAD_EXPLICIT_SCHED,
    SCHED_RR, 50*1024, NULL, &ServerPoll3MainTask,            NULL},
  {E_SYS_TLB_SpmMain_4,      "tSPM_Main_4\0",      11, PTHREAD_EXPLICIT_SCHED,
    SCHED_RR, 50*1024, NULL, &ServerPoll4MainTask,            NULL},
  {E_SYS_TLB_SpmMain_5,      "tSPM_Main_5\0",      11, PTHREAD_EXPLICIT_SCHED,
    SCHED_RR, 50*1024, NULL, &ServerPoll5MainTask,            NULL},
  {E_SYS_TLB_SpmCopy_1,      "tSPM_Copy_1\0",      10, PTHREAD_EXPLICIT_SCHED,
    SCHED_RR, 50*1024, NULL, &ServerPoll1CopyTask,            NULL},
  {E_SYS_TLB_SpmCopy_2,      "tSPM_Copy_2\0",      10, PTHREAD_EXPLICIT_SCHED,
    SCHED_RR, 50*1024, NULL, &ServerPoll2CopyTask,            NULL},
  {E_SYS_TLB_SpmCopy_3,      "tSPM_Copy_3\0",      10, PTHREAD_EXPLICIT_SCHED,
    SCHED_RR, 50*1024, NULL, &ServerPoll3CopyTask,            NULL},
  {E_SYS_TLB_SpmCopy_4,      "tSPM_Copy_4\0",      10, PTHREAD_EXPLICIT_SCHED,
    SCHED_RR, 50*1024, NULL, &ServerPoll4CopyTask,            NULL},
  {E_SYS_TLB_SpmCopy_5,      "tSPM_Copy_5\0",      10, PTHREAD_EXPLICIT_SCHED,
    SCHED_RR, 50*1024, NULL, &ServerPoll5CopyTask,            NULL},
  {E_SYS_TLB_SWC_NTPClientComm,
   "tSWC_NTPCliComm\0",
   15,
   PTHREAD_EXPLICIT_SCHED,
   SCHED_RR, 10*1024, NULL, &SWC_NTPClientCommunicationThrd, NULL},

  {E_SYS_TLB_RMM_OthRTULinkSock1,
   "tORTULS1\0",
   RMM_RTU_LINK_SERVER_TASK_PRI,
   PTHREAD_EXPLICIT_SCHED,
   SCHED_RR, 40*1024, NULL, &OtherRTUServerSocketTask1, NULL},

  {E_SYS_TLB_RMM_OthRTULinkSock2,
   "tORTULS2\0",
   RMM_RTU_LINK_SERVER_TASK_PRI,
   PTHREAD_EXPLICIT_SCHED,
   SCHED_RR, 40*1024, NULL, &OtherRTUServerSocketTask2, NULL},

  {E_SYS_TLB_RMM_OthRTULinkTask,
   "tORTULink\0",
   RMM_RTU_LINK_TASK_PRI,
   PTHREAD_EXPLICIT_SCHED,
   SCHED_RR, 40*1024, NULL, &OtherRTULinkTask, NULL},

  {E_SYS_TLB_RMM_OthRTUConnTask1,
   "tORTUCN1\0",
   RMM_RTU_LINK_CONNECT_TASK_PRI,
   PTHREAD_EXPLICIT_SCHED,
   SCHED_RR, 20*1024, NULL, &OtherRTUConnectionTask1, NULL},

  {E_SYS_TLB_RMM_OthRTUConnTask2,
   "tORTUCN2\0",
   RMM_RTU_LINK_CONNECT_TASK_PRI,
   PTHREAD_EXPLICIT_SCHED,
   SCHED_RR, 20*1024, NULL, &OtherRTUConnectionTask2, NULL},

  {E_SYS_TLB_RMM_StdbyReadSock1,
   "tStdbyRxLnk1\0",
   RMM_RTU_LINK_CONNECT_TASK_PRI,
   PTHREAD_EXPLICIT_SCHED,
   SCHED_RR, 40*1024, NULL, &StandByReadSocketLink1, NULL},

  {E_SYS_TLB_RMM_StdbyReadSock2,
   "tStdbyRxLnk2\0",
   RMM_RTU_LINK_CONNECT_TASK_PRI,
   PTHREAD_EXPLICIT_SCHED,
   SCHED_RR, 40*1024, NULL, &StandByReadSocketLink2, NULL},

  {E_SYS_TLB_RMM_ServerSocketTask,
   "tRMMSocket\0",
   RMM_SERVER_TASK_PRI,
   PTHREAD_EXPLICIT_SCHED,
   SCHED_RR, 20*1024, NULL, &RMMServerSocketTask, NULL},

  {E_SYS_TLB_RMM_Task,
   "tRMM\0",
   RMM_MAIN_TASK_PRI,
   PTHREAD_EXPLICIT_SCHED,
   SCHED_RR, 40*1024, NULL, &RMMTask, NULL},

  {E_SYS_TLB_MMM_Task,
   "tMMMMain\0",
   MMM_TASK_PRI,
   PTHREAD_EXPLICIT_SCHED,
   SCHED_RR, 8*1024, NULL, &MMMMainTask, NULL},

  /*--------------------------------------------------------------------------
    CAUTION: The following section is reserved to SWC tasks only. System will
             use offset from here. Other task can be added above this section
  ----------------------------------------------------------------------------*/

  {E_SYS_TLB_SWC_MgmtTask,
   "tSWC_Mgmt_\0",
   10,
   PTHREAD_EXPLICIT_SCHED,
   SCHED_RR, 40*1024, NULL, &SWC_MgmtThread, NULL},

  {E_SYS_TLB_SWC_ServerTask,
   "tSWC_Svr_\0",
   10,
   PTHREAD_EXPLICIT_SCHED,
   SCHED_RR, 20*1024, NULL, &SWC_ServerListeningThread, NULL}
}; // sys_thrd_attr_lut

/*----------------------------------------------------------------------------
  Look up table for creating message queues
----------------------------------------------------------------------------*/
static struct msq_lut_st sys_MessageQueueCtrlLut[E_SYS_MSQL_EndOfList]=
{
  {E_SYS_MSQL_Reserved_0, "/RTU/msq_reserved", NULL, NULL},

  {E_SYS_MSQL_FLG_SystemLogReceiving,
   "/RTU/msq_FLG_SystemLogReceiving",
   O_RDWR|O_CREAT,
   S_IRUSR|S_IWUSR
  },

//  #ifdef CFG_ENABLE_SPA
//  {E_SYS_MSQL_SerCmdToSPAParseCmd,
//   "/RTU/msq_SER_CmdToSPAParser",
//   //O_RDWR|O_CREAT|O_NONBLOCK,
//   O_RDWR|O_CREAT,
//   S_IRUSR|S_IWUSR
//  },
//
//  {E_SYS_MSQL_SPAResultToSerOutput,
//   "/RTU/msq_SPA_ResultToSerOutput",
//   //O_RDWR|O_CREAT|O_NONBLOCK,
//   O_RDWR|O_CREAT,
//   S_IRUSR|S_IWUSR
//  },
//  #endif // CFG_ENABLE_SPA

  {E_SYS_MSQL_NTPC_TxExternalSystemMsg,
   "/RTU/msq_NTPC_TxExternalSystemMsg",
   O_RDWR|O_CREAT,
   //O_RDWR|O_CREAT,
   S_IRUSR|S_IWUSR
  }
};

#endif //_SYS_LUT_H_

