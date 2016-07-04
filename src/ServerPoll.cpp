/*----------------------------------------------------------------------------

            Copyright (c) 2011 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME

  ServerPoll.cpp

 COMPONENT

  Server Polling Module (SPM)

 DESCRIPTION

  This file consists of management routines for SPM

 AUTHOR

  Yu Wei


 REVISION HISTORY by Bryan Chong
 (Latest update is located first)

 D1.1.3
 ------
 27-Apr-2011

 - CServerPoll::copySWCCommunicationLinkStatusToRTUStatusTable
     Use m_unCurrentLinkStatus instead of GetLinkStatus routine to update
     m_aunLocalRTUStatus to prevent overwriting of SWC link check status.


 D1.0.5
 ------
 14-Jun-2010

 - Resolved issue on application fail to operate when simulate link up and down
   to server for multiple times [PR24]

------------------------------------------------------------------------------*/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/neutrino.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <iostream.h>
#include <errno.h>
#include <mqueue.h>
#include <termios.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
#include "err_def.h"
#include "err_ext.h"
#include "lnc_def.h"
#include "lnc_ext.h"
//#include "flg_def.h"
//#include "flg_ext.h"
#include "str_def.h"
#include "str_ext.h"

#include "Common.h"
#include "ServerPoll.h"
//#include "CMM_Comm.h"
#include "CMM_Timer.h"
#include "CMM_Listen.h"
#include "SWC.h"
#include "CMM_Log.h"
#include "RMM.h"
#include "CMM_Modbus.h"
#include "CMM_Watchdog.h"
#include "Common_qnxWatchDog.h"


//Main server task name.
const char *g_acMainTaskName[SERVER_POLLING_SOCKET_MAX] =
{
  "tSPM1Main",
  "tSPM2Main",
  "tSPM3Main",
  "tSPM4Main",
  "tSPM5Main"
};

//Copy task name.
const char *g_acCopyTaskName[SERVER_POLLING_SOCKET_MAX] =
{
  "tSPM1Copy",
  "tSPM2Copy",
  "tSPM3Copy",
  "tSPM4Copy",
  "tSPM5Copy"
};


//************************************************************************
//  ServerPoll global variables
//************************************************************************

//ServerPoll parameters
tServerPollParam  g_tServerPollParam;

//pointer to ServerPoll1 object
CServerPoll      *g_apServerPoll[SERVER_POLLING_SOCKET_MAX];


//************************************************************************
//  ServerPoll task functions
//************************************************************************

/*-----------------------------------------------------------------------------
Purpose:
  Spawn ServerPoll task
Return:
  OK
  ERROR

History

  Name         Date       Remark
  ----         ----       ------
  Bryan Chong  15-Oct-09  Update g_tRTUConfig.tLAN_Para[x].cLAN_SlaveAddress
                          to g_tRTUConfig.tLAN_Para[x].tPrtc.addr due to
                          parameter change in latest Init_Config.cpp

------------------------------------------------------------------------------*/
int ServerPollTaskSpawn(void)
{
  int nI = 0;
  int nReturn = OK;

  //init general ServerPoll parameters
  g_tServerPollParam.nPriorityMain = SPM_MAIN_TASK_PRI;
  g_tServerPollParam.nStackSizeMain = 10*1024;

  g_tServerPollParam.nPriorityCopy = SPM_COPY_TASK_PRI;
  g_tServerPollParam.nStackSizeCopy = 10*1024;

  g_tServerPollParam.nRecvTimeout = g_tRTUConfig.nServerCMDTimeout;

  strcpy(g_tServerPollParam.acListenIP, g_tRTUConfig.tLANPara[0].acLAN_IP);

  //g_tServerPollParam.ucModbusSlaveAddress =
       // g_tRTUConfig.tLANPara[0].cLAN_SlaveAddress;
  g_tServerPollParam.ucModbusSlaveAddress = g_tRTUConfig.tLANPara[0].tPrtc.addr;


  //Use config.txt file parameters to set polling table start and end address.
  //040604 Yu Wei
  g_tServerPollParam.unTablePollStart =
    (g_tRTUConfig.tRTUStatusTable.unTableStartAddress <
     g_tRTUConfig.tRTUPollingTable.unTableStartAddress)?
     g_tRTUConfig.tRTUStatusTable.unTableStartAddress :
     g_tRTUConfig.tRTUPollingTable.unTableStartAddress;

  g_tServerPollParam.unTablePollEnd =
    (g_tRTUConfig.tRTUStatusTable.unTableEndAddress >
     g_tRTUConfig.tRTUPollingTable.unTableEndAddress)?
     g_tRTUConfig.tRTUStatusTable.unTableEndAddress :
     g_tRTUConfig.tRTUPollingTable.unTableEndAddress;

  for(nI=0; nI<g_tRTUConfig.nRTUPollingSocketNumber; nI++)
  {
    delay(10);
    nReturn = ServerPollTaskSpawnOne(nI);
    if( nReturn == ERROR)
    {
      g_pEventLog->LogMessage((CHAR *)"ServerPoll Task spawn error");
      StopAllServerPollTask();
//      break;
    }
  }

  return nReturn;
}

/*
Purpose:
  Spawn one ServerPoll task


Return:
  OK
  ERROR
*/
/*******************************************************************************
Purpose
  Spawn one ServerPoll task

Input
  nI  [in] index of server poll thread

Return
  OK
  ERROR

History
   Name         Date          Remarks
   ----         ----          -------
  Yu Wei       04-Mar-2009  Initial revision
  Bryan Chong  17-Mar-2010  Fix issue on server 2 to 5 threads not able to
                            start [PR23]

*******************************************************************************/
int ServerPollTaskSpawnOne(int nI) //Server ID  ( 0 ~ 4). (in)
{
  E_ERR_T rstatus = E_ERR_Success;
  //pthread_attr_t attrm;
  //pthread_attr_t attrc;
  char acTemp[2048];
  int nReturn = OK;

  strcpy(g_tServerPollParam.acTaskNameMain, g_acMainTaskName[nI]);
  strcpy(g_tServerPollParam.acTaskNameCopy, g_acCopyTaskName[nI]);

  //create ServerPoll object
  g_apServerPoll[nI] = new CServerPoll(g_tRTUConfig.nRTUPollingSocketID[nI],nI);

  if (g_apServerPoll[nI] == TYP_NULL)
  {
    sprintf(acTemp,"ServerPoll %d : create object failed\n",nI+1);
    g_pEventLog->LogMessage(acTemp);

    #ifdef CFG_PRN_ERR
    printf("ERR  [SPM] ServerPollTaskSpawnOne, initializing SPM object %d "
           "fail\n", nI);
    #endif // CFG_DEBUG_MSG
    nReturn = ERROR;
  }


  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SPM)
  printf("[SPM] ServerPollTaskSpawnOne, initializing SPM object %d\n", nI);
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SPM)

  switch(nI){
  case 0:
    rstatus = SYS_ThrdInit(E_SYS_TLB_SpmMain_1);
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_DEBUG_MSG
      printf("ERR  [SPM] ServerPollTaskSpawnOne, create SPM Main 1 thread "
             "fail\n");
      #endif // CFG_DEBUG_MSG
    }

    rstatus = SYS_ThrdInit(E_SYS_TLB_SpmCopy_1);
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_DEBUG_MSG
      printf("ERR  [SPM] ServerPollTaskSpawnOne, create SPM Copy 1 thread "
             "fail\n");
      #endif // CFG_DEBUG_MSG
    }
    break;

  case 1:
    rstatus = SYS_ThrdInit(E_SYS_TLB_SpmMain_2);
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_DEBUG_MSG
      printf("ERR  [SPM] ServerPollTaskSpawnOne, create SPM Main 2 thread "
             "fail\n");
      #endif // CFG_DEBUG_MSG
    }

    rstatus = SYS_ThrdInit(E_SYS_TLB_SpmCopy_2);
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_DEBUG_MSG
      printf("ERR  [SPM] ServerPollTaskSpawnOne, create SPM Copy 2 thread "
             "fail\n");
      #endif // CFG_DEBUG_MSG
    }
    break;

  case 2:

    rstatus = SYS_ThrdInit(E_SYS_TLB_SpmMain_3);
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_DEBUG_MSG
      printf("ERR  [SPM] ServerPollTaskSpawnOne, create SPM Main 3 thread "
             "fail\n");
      #endif // CFG_DEBUG_MSG
    }

    rstatus = SYS_ThrdInit(E_SYS_TLB_SpmCopy_3);
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_DEBUG_MSG
      printf("ERR  [SPM] ServerPollTaskSpawnOne, create SPM Copy 3 thread "
             "fail\n");
      #endif // CFG_DEBUG_MSG
    }
    break;

  case 3:

    rstatus = SYS_ThrdInit(E_SYS_TLB_SpmMain_4);
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_DEBUG_MSG
      printf("ERR  [SPM] ServerPollTaskSpawnOne, create SPM Main 4 thread "
             "fail\n");
      #endif // CFG_DEBUG_MSG
    }

    rstatus = SYS_ThrdInit(E_SYS_TLB_SpmCopy_4);
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_DEBUG_MSG
      printf("ERR  [SPM] ServerPollTaskSpawnOne, create SPM Copy 4 thread "
             "fail\n");
      #endif // CFG_DEBUG_MSG
    }
    break;

  case 4:
    rstatus = SYS_ThrdInit(E_SYS_TLB_SpmMain_5);
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_DEBUG_MSG
      printf("ERR  [SPM] ServerPollTaskSpawnOne, create SPM Main 5 thread "
             "fail\n");
      #endif // CFG_DEBUG_MSG
    }

    rstatus = SYS_ThrdInit(E_SYS_TLB_SpmCopy_5);
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_DEBUG_MSG
      printf("ERR  [SPM] ServerPollTaskSpawnOne, create SPM Copy 5 thread "
             "fail\n");
      #endif // CFG_DEBUG_MSG
    }
    break;

    default:
      #ifdef CFG_PRN_ERR
      printf("ERR  [SPM] ServerPollTaskSpawnOne, unsupport index %d\n", nI);
      #endif // CFG_PRN_ERR
      break;
  }

  return nReturn;
}
/*
Purpose:
  Stop all ServerPoll1 main task and copy task.
*/
void StopAllServerPollTask(void)
{
  int nI;


  for(nI=0; nI<g_tRTUConfig.nRTUPollingSocketNumber; nI++)
  {
    if (g_apServerPoll[nI] != NULL)
    {
      g_apServerPoll[nI]->m_bTaskWorking = false;

      pthread_detach(g_apServerPoll[nI]->m_nServerPollMainTaskID);
      pthread_cancel(g_apServerPoll[nI]->m_nServerPollMainTaskID);
    }
  }
}

/*
Purpose:
  ServerPoll1 main task function to be spawned
*/
void *ServerPoll1MainTask(void *arg)
{
  while(1)
  {
    #if ((defined CFG_DEBUG_MSG) && (_CFG_DEBUG_SPM))
    printf("[SPM] ServerPoll1MainTask, loop\n");
    #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))
    //Main() will return when there is error
    g_apServerPoll[0]->MainTask();
    delay(SPM_MAIN_TASK_SLEEP);  //delay 1 sec before re-entering Main()
  }

  #ifdef CFG_PRN_ERR
  printf("ERR  [SPM] ServerPoll1MainTask, thread exit\n");
  #endif // CFG_PRN_ERR
}

/*
Purpose:
  ServerPoll1 copy task function to be spawned
*/
void *ServerPoll1CopyTask(void *arg)
{
  g_apServerPoll[0]->CopySWCsTables();

  #ifdef CFG_PRN_ERR
  printf("ERR  [SPM] ServerPoll1CopyTask, thread exit\n");
  #endif // CFG_PRN_ERR

  return NULL;
}

/*
Purpose:
  ServerPoll2 main task function to be spawned
*/
void *ServerPoll2MainTask(void *arg)
{
  while(1)
  {
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SPM))
    printf("[SPM] ServerPoll2MainTask, loop\n");
    #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))
    //Main() will return when there is error
    g_apServerPoll[1]->MainTask();
    //delay 1 sec before re-entering Main()
    delay(SPM_MAIN_TASK_SLEEP);
  }

  #ifdef CFG_PRN_ERR
  printf("ERR  [SPM] ServerPoll2MainTask, thread exit\n");
  #endif // CFG_PRN_ERR
  return NULL;
}

/*
Purpose:
  ServerPoll2 copy task function to be spawned
*/
void *ServerPoll2CopyTask(void *arg)
{
  g_apServerPoll[1]->CopySWCsTables();

  #ifdef CFG_PRN_ERR
  printf("ERR  [SPM] ServerPoll2CopyTask, thread exit\n");
  #endif // CFG_PRN_ERR

  return NULL;
}

/*
Purpose:
  ServerPoll3 main task function to be spawned
*/
void *ServerPoll3MainTask(void *arg)
{
  while(1)
  {
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SPM))
    printf("[SPM] ServerPoll3MainTask, loop\n");
    #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))
    g_apServerPoll[2]->MainTask();    //Main() will return when there is error
    delay(SPM_MAIN_TASK_SLEEP);  //delay 1 sec before re-entering Main()
  }

  #ifdef CFG_PRN_ERR
  printf("ERR  [SPM] ServerPoll3MainTask, thread exit\n");
  #endif // CFG_PRN_ERR
}

/*
Purpose:
  ServerPoll3 copy task function to be spawned
*/
void *ServerPoll3CopyTask(void *arg)
{

  g_apServerPoll[2]->CopySWCsTables();

  #ifdef CFG_PRN_ERR
  printf("ERR  [SPM] ServerPoll3CopyTask, thread exit\n");
  #endif // CFG_PRN_ERR
  return NULL;
}

/*
Purpose:
  ServerPoll4 main task function to be spawned
*/
void *ServerPoll4MainTask(void *arg)
{
  while(1)
  {
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SPM))
    printf("[SPM] ServerPoll4MainTask, loop\n");
    #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))
    g_apServerPoll[3]->MainTask();    //Main() will return when there is error
    delay(SPM_MAIN_TASK_SLEEP);  //delay 1 sec before re-entering Main()
  }

  #ifdef CFG_PRN_ERR
  printf("ERR  [SPM] ServerPoll4MainTask, thread exit\n");
  #endif // CFG_PRN_ERR
  return NULL;
}

/*
Purpose:
  ServerPoll4 copy task function to be spawned
*/
void *ServerPoll4CopyTask(void* arg)
{
  g_apServerPoll[3]->CopySWCsTables();

  #ifdef CFG_PRN_ERR
  printf("ERR  [SPM] ServerPoll4CopyTask, thread exit\n");
  #endif // CFG_PRN_ERR
  return NULL;
}

/*
Purpose:
  ServerPoll5 main task function to be spawned
*/
void *ServerPoll5MainTask(void *arg)
{
  while(1)
  {
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SPM))
    printf("[SPM] ServerPoll5MainTask, loop\n");
    #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))
    g_apServerPoll[4]->MainTask();    //Main() will return when there is error
    delay(SPM_MAIN_TASK_SLEEP);  //delay 1 sec before re-entering Main()
  }

  #ifdef CFG_PRN_ERR
  printf("ERR  [SPM] ServerPoll5MainTask, thread exit\n");
  #endif // CFG_PRN_ERR
  return NULL;
}

/*
Purpose:
  ServerPoll5 copy task function to be spawned
*/
void *ServerPoll5CopyTask(void *arg)
{
  g_apServerPoll[4]->CopySWCsTables();

  #ifdef CFG_PRN_ERR
  printf("ERR  [SPM] ServerPoll5CopyTask, thread exit\n");
  #endif // CFG_PRN_ERR
  return NULL;
}

/************************************************************************
Function Name:
  ServerPoll class

Parameter:
  unListenPort  [in] Server port number.
  nObjectID     [in] Server ID (0~4)

History:
  Name       Date          Remark
  Yu, Wei   10-May-2004   Initial revision


************************************************************************/

CServerPoll::CServerPoll(unsigned short unListenPort,  int nObjectID)
{
  int nReturn;
  pthread_mutexattr_t *attrm = NULL;

  m_bTaskWorking = true;

  m_unListenPort = unListenPort;

  m_nObjectID = nObjectID;
  m_nClientOK = FALSE;


  //create semaphore for temp polling table
    /*m_semTablePollTemp = semBCreate(SEM_Q_FIFO, SEM_FULL);*/
  attrm = (pthread_mutexattr_t *)malloc(sizeof(pthread_mutexattr_t));
  pthread_mutexattr_init(attrm);
  nReturn = pthread_mutex_init(&m_semTablePollTemp , attrm);
  if (nReturn < 0)
  {
    g_pEventLog->LogMessage((CHAR *)"ServerPoll: create semaphore for temp polling table failed\n");
  }
  free(attrm);
  //zero polling table and temporary copy
  memset(m_unTablePoll, 0, sizeof(m_unTablePoll));
  memset(m_unTablePollTemp, 0, sizeof(m_unTablePollTemp));

  //zero no. of failed attempts to copy SWC table
  for (int n=0; n<SWC_MAX_NUMBER + 1; n++)  //add 1 for RTU status table
  {
    m_nSWCCopyFailed[n] = 0;
  }
  //create watch dog for each object
  //serv_poll_main_timer_id = wdCreate(WatchDogChid , serv_poll_main_timer_id ,
  //  SER_POLL_MAIN ,m_nObjectID + SPM_MAIN_TASK_ID_INDEX_START);
  serv_poll_copy_timer_id = wdCreate(WatchDogChid , serv_poll_copy_timer_id ,
    SER_POLL_COPY ,m_nObjectID + SPM_COPY_TASK_ID_INDEX_START);

  #ifdef CFG_ENABLE_MODULE_TEST
  if((m_isLogEnable == E_TYPE_Yes) && (*m_pLogFilename != NULL))
    LogSrvPollValues(nObjectID, m_pLogFilename);    //for test.
  #endif // CFG_ENABLE_MODULE_TEST

}

CServerPoll::~CServerPoll()
{
  //delete semaphore
  pthread_mutex_destroy(&m_semTablePollTemp);

  if (m_nSockListen != ERROR)
  {
    close(m_nSockListen);
  }
}


/*******************************************************************************
Function Name    : CServerPoll::SPMModbusCmdCheck
  Purpose        : Check Modbus cmd and send error reply for Modbus exception 1,
                   2 and 3
 Input Parameter :
   nFd    - [in] file descriptor
   pucCmd - [in] pointer to Modbus command
   nCmdSize - [in] Modbus command size

 Output Parameter:
   OK                  Success
   ERROR:              Cmd size, CRC or slave address error
   MODBUS_EXCEPTION1:  Fn code error
   MODBUS_EXCEPTION2:  Address error
   MODBUS_EXCEPTION3:  Data value error

 Global Modified:  None

 Revisions:
   Name       Date          Remarks
  Yu, Wei   10-May-2004   Initial revision

*******************************************************************************/
int CServerPoll::SPMModbusCmdCheck(int nFd, unsigned char *pucCmd, int nCmdSize)
{
  unsigned short unAddress;  //address of 1st word to read extracted from cmd
  unsigned short unNoWord;  //no. of words to read extracted from cmd

  //Check command first. 040331 Yu Wei.
  switch(ModbusCMDCheck(pucCmd, nCmdSize,
         g_tServerPollParam.ucModbusSlaveAddress))
  {

    case MODBUS_ERROR_CRC:
    default:
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SPM)
      printf("[SPM] CServerPoll::SPMModbusCmdCheck, MODBUS_ERROR_CRC\n");
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SPM)
      return ERROR;

    case MODBUS_ERROR_EXCEPTION1:
      #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SPM)
      printf("WARN [SPM] CServerPoll::SPMModbusCmdCheck, "
             "tx MODBUS_ERROR_EXCEPTION1 msg, invalid function code\n");
      #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SPM)
      ModbusSendErrorReply(nFd, pucCmd[0], pucCmd[1], MODBUS_EXCEPTION1);
      return MODBUS_EXCEPTION1;

    case MODBUS_ERROR_EXCEPTION3:
      #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SPM)
      printf("WARN [SPM] CServerPoll::SPMModbusCmdCheck, "
             "tx MODBUS_ERROR_EXCEPTION3 msg, invalid data value\n");
      #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SPM)
      ModbusSendErrorReply(nFd, pucCmd[0], pucCmd[1], MODBUS_EXCEPTION3);
      return MODBUS_EXCEPTION3;

    case MODBUS_MESSAGE_OK:
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SPM)
      printf("[SPM] CServerPoll::SPMModbusCmdCheck, "
             "MODBUS_MESSAGE_OK\n");
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SPM)

      //check fn code must be 4 (read word)
      if (pucCmd[1] != E_MB_FC_Read_N_Words)
      {
        ModbusSendErrorReply(nFd, pucCmd[0], pucCmd[1], MODBUS_EXCEPTION1);
        #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SPM)
        printf("[SPM] CServerPoll::SPMModbusCmdCheck, invalid function code "
               "%d, tx MODBUS_EXCEPTION1 msg\n",
                pucCmd[1]);
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SPM)
        return MODBUS_EXCEPTION1;
      }

      //check address of 1ast word to read
      unAddress = *(unsigned short *)&pucCmd[2];
      unAddress = STR_ByteSwap16Bit(unAddress);

      if( (unAddress < g_tServerPollParam.unTablePollStart) ||
          (unAddress > g_tServerPollParam.unTablePollEnd) )
      {
        ModbusSendErrorReply(nFd, pucCmd[0], pucCmd[1], MODBUS_EXCEPTION2);
        printf("MODBUS_EXCEPTION2");
        #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SPM)
        printf("WARN [SPM] CServerPoll::SPMModbusCmdCheck, invalid address, "
               "0x%x, tx MODBUS_EXCEPTION2 msg\n",
               unAddress);
        #endif // (((defined CFG_PRN_WARN) && CFG_PRN_WARN_SPM)
        return MODBUS_EXCEPTION2;
      }

      //check no. of words to read
      unNoWord = *(unsigned short *)&pucCmd[4];
      unNoWord = STR_ByteSwap16Bit(unNoWord);

      if ( (unNoWord == 0) ||
        ((int)unAddress + (int)unNoWord - 1 > (int)g_tServerPollParam.unTablePollEnd) )
      {
        ModbusSendErrorReply(nFd, pucCmd[0], pucCmd[1], MODBUS_EXCEPTION3);
        printf("MODBUS_EXCEPTION3");
        #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SPM)
        printf("WARN [SPM] CServerPoll::SPMModbusCmdCheck, invalid no of word, "
               "%d, tx MODBUS_EXCEPTION3 msg\n", unNoWord);
        #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SPM)
        return MODBUS_EXCEPTION3;
      }
      break;
  }
  return 0;
} // SPMModbusCmdCheck

/*------------------------------------------------------------------------------
Purpose:
  Receive modbus packet and verify if the packet received correctly. Update SWC
  communication link status to RTU Status Table

Parameter:
  nSockClient   [in] client socket

History
  Name         Date         Remark
  ----         ----         ------
 Bryan Chong  14-Jun-2010  Resolve role switching issue by implementing
                           LNC_ReceiveMsg [PR25]
 Bryan Chong  18-Aug-2010  Replace Send with LNC_SendMsg routine. This fixes
                           the not responding issue when invalid commands are
                           sent from Server, as well as no timeout when invalid
                           command byte count was sent
                           [C955 PR54] [C955 PR55]
 Bryan Chong  19-Aug-2010  Update to filter out exception result, as
                           error reply message is returned to server by
                           ModbusSendErrorReply routine
------------------------------------------------------------------------------*/
void CServerPoll::ClientHandler(int nSockClient)
{
  unsigned char ucCmd[MODBUS_MAX_CMD_SIZE];  //buffer to receive cmd
  int nBytesRecv;                //bytes received from client
  unsigned short unAddress;        //address of 1st word to read extracted from cmd
  unsigned short unNoWord;        //no. of words to read extracted from cmd
  char cLog[2048];            //error log buffer
  tRMM_MSG tRMMMessageSend;        //the structure for message Q that send to RMM.
  int  nModbusResult;            //Modbus command check result.  //040510 Yu Wei

  tRMMMessageSend.nDeviceID = m_nObjectID;
  tRMMMessageSend.nMessageID = SERVER_POLLING_STATUS_CHANGE;

  UINT16 totalRTUStatusTblLength = 0;
  E_ERR_T rstatus = E_ERR_Success;
  INT32 sendlength = 0;

  #ifdef CFG_PRN_ERR
  CHAR acErrMsg[100] = {0};
  #endif // CFG_PRN_ERR
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};

  while(1)
  {
    nBytesRecv = 0;

    //[PR25]
    rstatus = LNC_ReceiveMsg(nSockClient, (VOID *)ucCmd, &nBytesRecv,
                             sizeof(ucCmd), g_tServerPollParam.nRecvTimeout);

    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_PRN_ERR
      ERR_GetMsgString(rstatus, acErrMsg);
      printf("ERR  [SPM] CServerPoll::ClientHandler, rx msg fail at fd %d, "
             "rx size %d, buffer sz %d, %s\n",
              nSockClient, nBytesRecv, sizeof(ucCmd), acErrMsg);
      #endif // CFG_PRN_ERR
    }

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SPM)
    printf("[SPM] CServerPoll::ClientHandler, fd %d rx %d bytes from server:\n",
           nSockClient, nBytesRecv);
    SYS_PrnDataBlock((const UINT8 *) ucCmd, nBytesRecv, 10);
    #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))

    if ( (nBytesRecv == ERROR) || (nBytesRecv == 0) )  //error or connection closed
    {
      m_nClientOK = FALSE;

      if( g_tDebugLogFlag.bSocketConnect == true)    //040309 Yu Wei
      {
        sprintf(cLog, "ServerPoll: client disconnected from port %u\n",
                m_unListenPort);
        g_pDebugLog->LogMessage(cLog);
      }

      tRMMMessageSend.unStatus = 0;
      g_pRTUStatusTable->SendMsg(tRMMMessageSend);
      return;
    }

    if (nBytesRecv == TIMEOUT)
    {
      m_nClientOK = FALSE;

      if( g_tDebugLogFlag.bSocketConnect == true)    //040309 Yu Wei
      {
        sprintf(cLog, "ServerPoll: client timeout on port %u\n",
                m_unListenPort);
        g_pDebugLog->LogMessage(cLog);
        sprintf(cLog, "Server timeout (port: %u)\n", m_unListenPort);
        g_pEventLog->LogMessage(cLog);
      }
      tRMMMessageSend.unStatus = 0;
      g_pRTUStatusTable->SendMsg(tRMMMessageSend);
      return;
    }

    // TODO: Correct Modbus check cheating here
    //Exception has replied by this routine. //040510 Yu Wei
    nModbusResult = SPMModbusCmdCheck(nSockClient, ucCmd, nBytesRecv);
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SPM)
    printf("[SPM] CServerPoll::ClientHandler, modbus result = %d\n",
           nModbusResult);
    #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))
    //Not CRC or slave address error, the link is OK. 040510 Yu Wei
    // 20100819 BC (rqd by ZSL)
    if ((nModbusResult != ERROR) &&
        (nModbusResult != MODBUS_EXCEPTION1) &&
        (nModbusResult != MODBUS_EXCEPTION2) &&
        (nModbusResult != MODBUS_EXCEPTION3))
    {
      tRMMMessageSend.unStatus = 1;
      g_pRTUStatusTable->SendMsg(tRMMMessageSend);

      if (!m_nClientOK)
      {
        m_nClientOK = TRUE;

        if( g_tDebugLogFlag.bSocketConnect == true)    //040309 Yu Wei
        {
          sprintf(cLog, "ServerPoll: client OK on port %u\n", m_unListenPort);
          g_pDebugLog->LogMessage(cLog);
          sprintf(cLog, "Server link recovered (port: %u)\n", m_unListenPort);
          g_pEventLog->LogMessage(cLog);
        }

//        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SPM_ServerLink,
//          FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec,
//          FLG_LOG_T_Debug,
//          "[SPM] CServerPoll::ClientHandler, %s\n"
//          "  server link %s:%d fd %d connecting to client %s:%d is OK\n",
//          SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
//          m_acListenIP, m_unListenPort, m_nSockClient,
//          m_acClientIP, m_unClientPort);
//        sprintf(cLog, "[SPM] CServerPoll::ClientHandler, %s\n"
//        		       " server link %s:%d fd %d connecting to client %s:%d is OK\n",
//        		         SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
//        	             m_acListenIP, m_unListenPort, m_nSockClient,
//        		         m_acClientIP, m_unClientPort);
//        g_pDebugLog->LogMessage(cLog);

      } // if (!m_nClientOK)

      if (nModbusResult == 0)  //Command OK, reply. 040510 Yu Wei
      {
        //send reply for Modbus fn code 4 read word cmd
        unAddress = *(unsigned short *)&ucCmd[2];
        unAddress = STR_ByteSwap16Bit(unAddress);
        unNoWord = *(unsigned short *)&ucCmd[4];
        unNoWord = STR_ByteSwap16Bit(unNoWord);

        m_ucSendBuffer[0] = ucCmd[0];  //reply has same slave address as cmd
        m_ucSendBuffer[1] = ucCmd[1];  //reply has same fn code as cmd

        if (unNoWord > 127)
        {
          m_ucSendBuffer[2] = 0;    //set to 0 if no. of bytes read exceed 255
        }
        else
        {
          m_ucSendBuffer[2] = ((unNoWord * 2));  //actual no. of bytes read
        }

        // The size of the RTU Status Table is total of 3 words to time stamp,
        // number of words in byte, and 2-byte CRC
        totalRTUStatusTblLength = (3 + unNoWord*2 + 2);

        // append SWC communication link status to RTU status table
        copySWCCommunicationLinkStatusToRTUStatusTable();

        //copy words read from temp polling table
        pthread_mutex_lock(&m_semTablePollTemp);
        memcpy(&m_ucSendBuffer[3],
               &m_unTablePollTemp[unAddress -
                                  g_tServerPollParam.unTablePollStart],
               unNoWord * 2);
        pthread_mutex_unlock(&m_semTablePollTemp);
        //calc CRC
        *(unsigned short *)&m_ucSendBuffer[3 + unNoWord*2] =
          STR_ByteSwap16Bit(ModbusCRC(m_ucSendBuffer, 3 + unNoWord*2));

        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SPM)
        printf("[SPM] CServerPoll::ClientHandler, start addr = 0x%x, "
               "no of word = 0x%x, unTablePollStart = 0x%x\n",
               unAddress, unNoWord, g_tServerPollParam.unTablePollStart);
        printf("[SPM] CServerPoll::ClientHandler, fd %d tx %d bytes\n",
               nSockClient, totalRTUStatusTblLength);
        SYS_PrnDataBlock((const UINT8 *) m_ucSendBuffer,
                         totalRTUStatusTblLength, 10);
        #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))

        #if ((defined CFG_DEBUG_MSG) && (_CFG_DEBUG_SPM))
        printf("[SPM] CServerPoll::ClientHandler, fd %d tx %d bytes:\n",
          nSockClient, totalRTUStatusTblLength);
        #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))

//        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SPM_TxRtuStatusTbl,
//          FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//          "[SPM] CServerPoll::ClientHandler, fd %d tx %d bytes:\n",
//          nSockClient, totalRTUStatusTblLength);

//        sprintf(cLog, "[SPM] CServerPoll::ClientHandler, fd %d tx %d bytes:\n",
//                nSockClient, totalRTUStatusTblLength);
//        g_pDebugLog->LogMessage(cLog);
//
//        pFLG_CtrlBlk->SendDataBlock(E_FLG_MSG_SPM_TxRtuStatusTbl,
//          FLG_LOG_T_Debug, (UINT8 *)m_ucSendBuffer, totalRTUStatusTblLength,
//          10);

        // 20100818 BC (Rqd by ZSL) [C955 PR54] [C955 PR55]
        rstatus = LNC_SendMsg(nSockClient, m_ucSendBuffer,
                              totalRTUStatusTblLength, &sendlength);
        if(rstatus != E_ERR_Success)
        {
          #ifdef CFG_PRN_ERR
          ERR_GetMsgString(rstatus, acErrMsg);
          printf("[SPM] CServerPoll::ClientHandler, err msg %s\n", acErrMsg);
          #endif // CFG_PRN_ERR
        }
      }    //Command OK, reply. 040510 Yu Wei
    }
    else  /* Added this "else" based on latest PKK's file that he sent to
             TZX and flow chart. */
    {
      if (m_nClientOK)
      {
        m_nClientOK = FALSE;

        if( g_tDebugLogFlag.bSocketConnect == true)    //040309 Yu Wei
        {
          sprintf(cLog, "ServerPoll: client cmd error on port %u\n",
                  m_unListenPort);
          g_pDebugLog->LogMessage(cLog);
          sprintf(cLog, "Server data error (port: %u)\n", m_unListenPort);
          g_pEventLog->LogMessage(cLog);
        }
      }
    }    //Added "else"
  }
}// ClientHandler

/*------------------------------------------------------------------------------
Purpose:
  Listen for client connection and process client transactions


History

  Name         Date         Remark
  ----         ----         ------
 Bryan Chong  14-Jun-2010  Resolve memory fault issue by re-writing printf
                           command [PR28]
------------------------------------------------------------------------------*/
void CServerPoll::MainTask()
{
  struct sockaddr_in addrListen;  //listen socket address
  int nOn = 1;          //set socket option value for re-use socket address
  struct sockaddr_in addrClient;  //client socket address
  int nAddrSize;          //size of address
  char cLog[2048] = {0};        //error log buffer
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};


  //create listen socket
  m_nSockListen = socket(AF_INET, SOCK_STREAM, 0);

  if (m_nSockListen < 0)
  {
    if( g_tDebugLogFlag.bSocketConnect == true)    //040309 Yu Wei
    {
      sprintf(cLog, "ServerPoll(%d): create listen socket failed ",
              m_nObjectID);
      g_pDebugLog->LogMessage(cLog);
    }
    close(m_nSockListen);
    #if ((defined CFG_DEBUG_MSG) && (_CFG_DEBUG_SPM))
    printf("[SPM] CServerPoll::MainTask, create listening socket fail, "
           "close socket 0x%x\n", m_nSockListen);
    #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))
    return ;
  }

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SPM))
  printf("[SPM] CServerPoll::MainTask, created listening socket fd %d\n",
          m_nSockListen);
  #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))

  //enable re-use socket address option
  if (setsockopt(m_nSockListen, SOL_SOCKET, SO_REUSEPORT, (char *)&nOn,
                 sizeof(nOn)) == ERROR)
  {
    if( g_tDebugLogFlag.bSocketConnect == true)    //040309 Yu Wei
    {
      sprintf(cLog,"ServerPoll(%d): set socket re-use address failed ", m_nObjectID);
      g_pDebugLog->LogMessage(cLog);
    }
    close(m_nSockListen);
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SPM))
    printf("[SPM] CServerPoll::MainTask, set socket option fail, close socket "
           "0x%x\n", m_nSockListen);
    #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))
    return;
  }

  //init listen socket address
  memset(&addrListen, 0, sizeof(addrListen));
  addrListen.sin_family = AF_INET;
  addrListen.sin_port = htons(m_unListenPort);

  inet_aton(g_tServerPollParam.acListenIP, &addrListen.sin_addr);
  strcpy(m_acListenIP, inet_ntoa(addrListen.sin_addr));


  //bind listen socket
  if (bind(m_nSockListen, (struct sockaddr *)&addrListen,
           sizeof(struct sockaddr)) < 0)
  {
    if( g_tDebugLogFlag.bSocketConnect == true)
    {
      sprintf(cLog,"ServerPoll(%d) (%s: %d) bind listen socket failed return\n",
              m_nObjectID ,g_tServerPollParam.acListenIP, m_unListenPort);
      g_pDebugLog->LogMessage(cLog);
    }
    close(m_nSockListen);
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SPM))
    printf("[SPM] CServerPoll::MainTask, bind socket fail, close socket "
           "0x%x\n", m_nSockListen);
    #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))
    return;
  }

  //enable connections to listen socket,
  // maximum number of 1 unaccepted connection that can be pending at one time
  if (listen(m_nSockListen, 1) < 0 )
  {
    if( g_tDebugLogFlag.bSocketConnect == true)    //040309 Yu Wei
    {
      sprintf(cLog,"ServerPoll(%d): start listen socket failed\n", m_nObjectID);
      g_pDebugLog->LogMessage(cLog);
    }
    close(m_nSockListen);
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SPM))
    printf("[SPM] CServerPoll::MainTask, listen socket fail, close socket "
           "0x%x\n", m_nSockListen);
    #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))
    return;
  }

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SPM))
  // [PR28]
  printf("[SPM] CServerPoll::MainTask, fd %d created from IP %s:%d\n",
         m_nSockListen, g_tServerPollParam.acListenIP, m_unListenPort);
  #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))
  while(1)
  {

    //accept client socket
    nAddrSize = sizeof(struct sockaddr_in);
    m_nSockClient = accept(m_nSockListen, (struct sockaddr *)&addrClient,
                           &nAddrSize);

    if (m_nSockClient < 0)
    {
      if(g_tDebugLogFlag.bSocketConnect == true)    //040309 Yu Wei
      {
        sprintf(cLog,"[SPM] ServerPoll(%d): accept client failed, %s\n",
                m_nObjectID, strerror(errno));
        g_pDebugLog->LogMessage(cLog);
      }
      close(m_nSockListen);
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SPM))
      printf("[SPM] CServerPoll::MainTask, accept socket fail, close client "
             "socket 0x%x\n", m_nSockClient);
      #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))
      m_nSockClient = ERROR;
    }
    else
    {
      if( g_tDebugLogFlag.bSocketConnect == true)    //040309 Yu Wei
      {
        sprintf(cLog, "accepted host %u.%u.%u.%u local target %s:%u\n",
          (unsigned char)(addrClient.sin_addr.s_addr),
          (unsigned char)(addrClient.sin_addr.s_addr>>8),
          (unsigned char)(addrClient.sin_addr.s_addr>>16),
          (unsigned char)(addrClient.sin_addr.s_addr>>24),
          g_tServerPollParam.acListenIP, m_unListenPort);
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SPM))
        printf("[SPM] CServerPoll::MainTask, listen fd %d, client fd "
               "%d, %s\n", m_nSockListen, m_nSockClient, cLog);
        #endif // CFG_DEBUG_MSG
        g_pDebugLog->LogMessage(cLog);
      }

      strcpy(m_acClientIP, inet_ntoa(addrClient.sin_addr));
      m_unClientPort = ntohs(addrClient.sin_port);

      //process client transactions
      ClientHandler(m_nSockClient);

//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SPM_ServerLink,
//        FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec,
//        FLG_LOG_T_Debug,
//        "[SPM] CServerPoll::MainTask, %s\n"
//        "  server link %s:%d fd %d connecting to client %s:%d is NG\n",
//        SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
//        m_acListenIP, m_unListenPort, m_nSockClient,
//        m_acClientIP, m_unClientPort);

      sprintf(cLog, "[SPM] CServerPoll::MainTask, %s\n"
    	        "  server link %s:%d fd %d connecting to client %s:%d is NG\n",
    	        SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
    	        m_acListenIP, m_unListenPort, m_nSockClient,
    	        m_acClientIP, m_unClientPort);
      g_pDebugLog->LogMessage(cLog);
      //close client socket
      shutdown(m_nSockClient, SHUT_WR);  //send disallowed
      close(m_nSockClient);
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SPM))
      printf("[SPM] CServerPoll::MainTask, shutdown and close client socket "
             "0x%x\n", m_nSockClient);
      #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))

      m_nSockClient = ERROR;
    }

    if (m_nSockClient == ERROR)
    {
      close(m_nSockListen);	//20141111 Su netstat -n
      break;
    }
  }
  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SPM))
  printf("[SPM] CServerPoll::MainTask, exit MainTask\n");
  #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))
}// MainTask
/*******************************************************************************
Purpose
  Copy SWCs communication link status to RTU status table

Input
  None

Return
  None

History
  Name         Date          Remark
  ----         ----          ------
  Bryan Chong  25-Nov-2009   Initial revision
  Bryan Chong  27-Apr-2011   Use m_unCurrentLinkStatus instead of
                             GetLinkStatus routine to update
                             m_aunLocalRTUStatus to prevent overwriting of SWC
                             link check status

*******************************************************************************/
E_ERR_T CServerPoll::copySWCCommunicationLinkStatusToRTUStatusTable()
{
  UINT8 cindex;

  for(cindex = 0; cindex < g_tRTUConfig.nSWCNumber; cindex++)
  {
    if (g_apSWCObject[cindex] == false)
      continue;

    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SPM || _CFG_DEBUG_RMMLINKCHK))
    if(cindex == CFG_NTP_CLIENT_SWC_INDEX)
    {
      printf("[SPM] CServerPoll::copySWCCommunicationLinkStatusToRTUStatusTable, "
           "SWC(%d) %s status = 0x%04x\n",
           (cindex + 1),
           ((g_apSWCObject[cindex]->m_nLinkType ==  SWC_LINK_LAN) ?
           "LAN" : "Serial"), g_apSWCObject[cindex]->m_unCurrentLinkStatus);
    }
    #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))

    // Update SWC communication link status on RTU Status Table
    g_pRTUStatusTable->m_aunLocalRTUStatus[cindex +
      RTU_STATUS_TABLE_SWC_OFFSET] =
        g_apSWCObject[cindex]->m_unCurrentLinkStatus;
   // printf(" Number 1 m_unCurrentLinkStatus  =%02x \n",g_apSWCObject[cindex]->m_unCurrentLinkStatus);
  } // for(cindex = 0; cindex < g_tRTUConfig.nSWCNumber; cindex++)

  return E_ERR_Success;
} // copySWCCommunicationLinkStatusToRTUStatusTable


/*------------------------------------------------------------------------------
Purpose:
  Copy RTU status table and SWCs' tables to polling table

History

     Name       Date          Remark

 Bryan Chong  16-Jun-2011  Add the error message when watchdog fail to start

------------------------------------------------------------------------------*/
void CServerPoll::CopySWCsTables()
{
  int nI;    //Yu Wei
  char acTemp[2048];  //Yu Wei

  int nSemiTimeOut;  //040322 Yu Wei
  int rval;
  //int tblsz;

  memset(m_threadName, 0, sizeof(m_threadName));
  m_threadID = pthread_self();
  rval = pthread_getname_np(m_threadID, m_threadName, sizeof(m_threadName));
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SPM)
  printf("[SPM] CServerPoll::CopySWCsTables, thrdID %d, name %s\n",
         (int)m_threadID, m_threadName);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SPA))
  while(1)
  {

    if(serv_poll_copy_timer_id != NULL)
    {
      if(wdStart(serv_poll_copy_timer_id , 60, 0, 0, 0) < 0)
      {
        #ifdef CFG_PRN_ERR
        printf("ERR  [SPM] CServerPoll::CopySWCsTables, watch dog Serv_poll "
               "copy error\n");
        #endif // CFG_PRN_ERR

        //20110616 BC
        g_pEventLog->LogMessage((CHAR *)
          "ERR  [SPM] CServerPoll::CopySWCsTables, fail to start watchdog\n");
        break;
      }

    }

    if(m_bTaskWorking == false)
    {
      if(serv_poll_copy_timer_id != NULL)
      {
        wdCancel(serv_poll_copy_timer_id);
        timer_delete(serv_poll_copy_timer_id);
        pthread_detach(pthread_self());
        pthread_exit(NULL);
      }

      return;
    }

    delay(SPM_COPY_TASK_SLEEP);  //delay 250 ms

    nSemiTimeOut = 0x00000000;  //Default copy SWC table is NO_WAIT. 040322 Yu Wei

    if(m_nCurrentStateFlag != g_tRTUStatus.nRTUStateFlag)  //Check state changing
    {
      if(//(m_nCurrentStateFlag == STATEFLAG_STANDBY) &&    //When change state from P(1)-->SW(4)-->S(2)
                                  //and S(2)-->SW(5)-->P(1), this task will change
                                  //state SW(4)-->SW(5). So it will remain in SW(4).
         (g_tRTUStatus.nRTUStateFlag == STATEFLAG_SWITCHING_STOP))
      {  //When state change to switch_stop, must copy SWC table.  //040322 Yu Wei
        nSemiTimeOut = 0xFFFFFFFF;  //Must copy SWC table. 040322 Yu Wei.
      }
    }

    //Hardware test state don't copy SWC table.
    if( m_nCurrentStateFlag != STATEFLAG_HARDWARETEST)
    {
      // clear buffer before populating the SWC tables
      memset(m_unTablePoll, 0, SERVERPOLL_TABLE_POLL_SIZE);
      //The all SWC task has been closed.   //040510 Yu Wei
      for(nI=0; nI<g_tRTUConfig.nSWCNumber; nI++)    //By Yu Wei
      {
        //copy each SWC's tables to polling table
        if (g_apSWCObject[nI] == TYP_NULL)
        {
          continue;//2008
        }

        if (g_apSWCObject[nI]->ReadSWCTable(m_unTablePoll, nSemiTimeOut) == OK)
        {
          m_nSWCCopyFailed[nI] = 0;

          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SPM)
          if(m_nSockClient > 0)
          {
            printf("[SPM] CServerPoll::CopySWCsTables, %s fd %d "
                   "m_unTablePoll[501] %d bytes:\n",
                   m_threadName, m_nSockClient, 104);
            SYS_PrnDataBlock((const UINT8 *) &m_unTablePoll[501], 104, 20);
          }
          #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))

        }
        else
        {
          m_nSWCCopyFailed[nI]++;

          if (m_nSWCCopyFailed[nI] >= SERVERPOLL_SWC_COPY_MAX_TRY)
          {
            #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SPM)
            printf("[SPM] CServerPoll::CopySWCsTables, read SWC(%d) table "
                   "retry %d\n",
                   g_tRTUConfig.anSWCIndex[nI], m_nSWCCopyFailed[nI]++);
            #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SPA))
            g_apSWCObject[nI]->ReadSWCTable(m_unTablePoll, WAIT_FOREVER);
            m_nSWCCopyFailed[nI] = 0;

            if( g_tDebugLogFlag.bSocketConnect == true)
            {
              sprintf(acTemp,"ServerPoll: %d copy failed\n",
                      g_tRTUConfig.anSWCIndex[nI]);
              g_pDebugLog->LogMessage(acTemp);
            }
          } // if (m_nSWCCopyFailed[nI] >= SERVERPOLL_SWC_COPY_MAX_TRY)
        }// end else if (g_apSWCObject[nI]->ReadSWCTable
      }// for(nI=0; nI<g_tRTUConfig.nSWCNumber; nI++)

    }// if( m_nCurrentStateFlag != STATEFLAG_HARDWARETEST)

    //copy RTU status table to polling table

    if (g_pRTUStatusTable->ReadRTUStatus(
      &m_unTablePoll[g_tRTUConfig.tRTUStatusTable.unTableStartAddress],
      nSemiTimeOut) == OK) //040322 Yu Wei
    {
      m_nSWCCopyFailed[SWC_MAX_NUMBER] = 0;  //last index is for RTU status table
    }
    else
    {
      m_nSWCCopyFailed[SWC_MAX_NUMBER]++;

      if (m_nSWCCopyFailed[SWC_MAX_NUMBER] >= SERVERPOLL_SWC_COPY_MAX_TRY)
      {

        g_pRTUStatusTable->ReadRTUStatus(
          &m_unTablePoll[g_tRTUConfig.tRTUStatusTable.unTableStartAddress],
          WAIT_FOREVER);  //By Yu Wei
        m_nSWCCopyFailed[SWC_MAX_NUMBER] = 0;

        if( g_tDebugLogFlag.bSocketConnect == true)    //040309 Yu Wei
        {
          g_pDebugLog->LogMessage(
            (CHAR *)"ServerPoll: RTU status table copy failed\n");
        }
      }
    }

    // lock temp polling table and copy polling table to temp polling table
    // semTake(m_semTablePollTemp, WAIT_FOREVER);
    pthread_mutex_lock(&m_semTablePollTemp);

    memcpy(m_unTablePollTemp, m_unTablePoll,
      (g_tServerPollParam.unTablePollEnd - g_tServerPollParam.unTablePollStart +
       1) * 2);
    pthread_mutex_unlock(&m_semTablePollTemp);

    //Check state changing  040322 Yu Wei
    if( m_nCurrentStateFlag != g_tRTUStatus.nRTUStateFlag)
    {
      if( g_tRTUStatus.nRTUStateFlag == STATEFLAG_SWITCHING_STOP)
      {
        //Completed copy latest SWC table that received from primary RTU.
        if( nSemiTimeOut == 0x0FFFFFFF)
        {
          //After copying table, updated SPM state.
          m_nCurrentStateFlag = g_tRTUStatus.nRTUStateFlag;
        }
      }
      else
      {
        m_nCurrentStateFlag = g_tRTUStatus.nRTUStateFlag;
      }
    }
  }// while(1)
}

//#ifdef CFG_ENABLE_SPA
//int CServerPoll::Get_SocketListen()
//{
//  return (m_nSockListen);
//}
//
//int CServerPoll::Get_SocketClient()
//{
//  return (m_nSockClient);
//}
//#endif // CFG_ENABLE_SPA
//
//#ifdef CFG_ENABLE_MODULE_TEST
//
//E_ERR_T CServerPoll::LogSrvPollValues(int nNum, const CHAR *pfilename)
//{
//  int i;
//  FILE *fd;
//
//  char pName[100];
//
//  if(pfilename == TYP_NULL)
//    return E_ERR_InvalidNullPointer;
//
//
//  // combine log filename and relevant server number
//  sprintf(pName, "%s_%d.txt", pfilename, nNum);
//
//  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SPM))
//  printf("[SPM] CServerPoll::LogSrvPollValues, creating file %s\n", pName);
//  #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))
//
//
//  fd = fopen(pName, "w+");
//  if ( fd == TYP_NULL) {
//    #ifdef CFG_PRN_ERR
//    printf("ERR  [SPM] CServerPoll::LogSrvPollValues, can not create filename "
//           "%s\n", pName);
//    #endif // CFG_PRN_ERR
//    return E_ERR_DEV_OpenFileFail;
//  }
//
//  //write time
//  time_t lTime = time(NULL);
//  fprintf(fd,"\tServer Poll %d values after initialization\r\n::::::::::::::::"
//          ":::::%s::::::::::::::::::::::\r\n", nNum, ctime(&lTime));
//
//  //write values
//  fprintf(fd,"int m_nServerPollMainTaskID: %d\r\n", m_nServerPollMainTaskID);
//
//  fprintf(fd,"int m_nServerPollCopyTaskID:%d\r\n", m_nServerPollCopyTaskID);
//
//  fprintf(fd,"int m_bTaskWorking:%d\r\n", m_bTaskWorking);
//
//  //unsigned short m_unTablePoll[SERVERPOLL_TABLE_POLL_SIZE];
//
//  fprintf(fd,"WDOG_ID    m_tWatchdogIDMain:%d\r\n", m_tWatchdogIDMain);
//  fprintf(fd,"WDOG_ID m_tWatchdogIDCopy:%d\r\n",m_tWatchdogIDCopy);
//
//
//  fprintf(fd,"int m_nSockListen:%d\r\n", m_nSockListen);
//
//  fprintf(fd,"int m_nSockClient:%d\r\n", m_nSockClient);
//
//  fprintf(fd,"int m_nObjectID:%d\r\n", m_nObjectID);
//
//  fprintf(fd,"int m_nCurrentStateFlag:%d\r\n", m_nCurrentStateFlag);
//  fprintf(fd,"unsigned short m_unListenPort:%d\r\n",  m_unListenPort);
//
//  fprintf(fd,"int m_nClientOK:%d\r\n", m_nClientOK);
//  fprintf(fd,"SEM_ID m_semTablePollTemp:%d\r\n", m_semTablePollTemp);
//
//  fprintf(fd,"int m_nSWCCopyFailed:\r\n");
//  for (i=0; i< SWC_MAX_NUMBER + 1; i++)
//    fprintf(fd,"%d:%d ", i, m_nSWCCopyFailed[i]);
//
//
//  fprintf(fd,"\r\ng_tServerPollParam:\r\n       ");
//  fprintf(fd,"acTaskNameMain[32]:%s nPriorityMain:%d nStackSizeMain:%d \r\n       ",
//    g_tServerPollParam.acTaskNameMain, g_tServerPollParam.nPriorityMain,
//    g_tServerPollParam.nStackSizeMain);
//
//  fprintf(fd,"acTaskNameCopy[32]:%s nPriorityCopy:%d nStackSizeCopy:%d \r\n       ",
//    g_tServerPollParam.acTaskNameCopy, g_tServerPollParam.nPriorityCopy,
//    g_tServerPollParam.nStackSizeCopy);
//
//  fprintf(fd,"nRecvTimeout:%d acListenIP:%s ucModbusSlaveAddress:%d \r\n       ",
//    g_tServerPollParam.nRecvTimeout, g_tServerPollParam.acListenIP,
//    g_tServerPollParam.ucModbusSlaveAddress);
//
//
//  fprintf(fd,"unTablePollStart:%d unTablePollEnd:%d \r\n",
//    g_tServerPollParam.unTablePollStart, g_tServerPollParam.unTablePollEnd);
//
//
//  lTime = time(NULL);
//
//  fprintf(fd,"\r\n:::::::::::::::::::::End: %s::::::::::::::::::::::\r\n",
//          ctime(&lTime));
//
//  fclose(fd);
//
//  //printf("End write %s\n", pName);
//  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SPM))
//  printf("[ISCS] CServerPoll::LogSrvPollValues, finish writing file %.\n",
//    *pName);
//  #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))
//
//  return E_ERR_Success;
//
//}// LogSrvPollValues
//#endif // CFG_ENABLE_MODULE_TEST

