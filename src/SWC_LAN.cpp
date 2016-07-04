/************************************************************
*                              *
*  Project:    Taoyuan RTU                  *
*  Module:    CMM                      *
*  File :      SWC_LAN.cpp                  *
*  Author:    Yu Wei                        *
*                              *
*  Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************

**************************************************************

DESCRIPTION

  This file defines common LAN SWC(PLC) class which is inherit from SWC class.

*************************************************************
Modiffication History
---------------------
Version: SWC 1.0.0.3
  01  06 May 2013, Yang Tao
    (1)Modified the FastPolling, SlowPolling, TimerPolling. PrimaryLinkCheck,TimeSynchronization
       LinkCheck, LinkMonitor module: Categorize the result of RequestandReceiveWithModbusCheck
       function into 3 categories: Success, Link Error, Exceptional Error. Unify the exception
       counter to m_nFastExcepCounter.
    (2)Remove the unused Function: CSWC_LAN::KeepAlive(void)
    (3)Add in Function code and Slave address Checking in CSWC_LAN::RequestandReceiveWithModbusCheck

**************************************************************/
#include <sys/neutrino.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <iostream.h>
#include <termios.h>

#include <fixerrno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "err_ext.h"
#include "str_def.h"
#include "flg_def.h"
#include "flg_ext.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
#include "lnc_def.h"
#include "lnc_ext.h"
#include "mdb_def.h"
#include "str_ext.h"

#include "Common.h"
#include "CMM_Timer.h"
#include "CMM_Listen.h"
#include "SWC_LAN.h"
#include "CMM_Log.h"
//#include "CMM_Comm.h"
#include "RMM.h"
#include "CMM_Modbus.h"
#include "CMM_Modbus_PLC.h"
#include "CMM_Watchdog.h"

#include "Common_qnxWatchDog.h"

/*-----------------------------------------------------------------------------
  Local prototype declaration
-----------------------------------------------------------------------------*/
static int Ping(char *ip);
static int checkReply(char *ptr, ssize_t len, struct msghdr *msg,
                      struct timeval *tvrecv ,pid_t IPpid);
static void tv_sub (struct timeval *out, struct timeval *in);
static UINT16 in_cksum (uint16_t * addr, int len);

CSWC_LAN::CSWC_LAN(struct tSWCCFGStructure tSWCPara,int nDeviceID,
                   char * acIPAddress): CSWC()
{
  E_ERR_T rstatus = E_ERR_Success;
  char acTemp[200];
  tModbusAddress tTempAdr;
  pthread_mutexattr_t *attrm = NULL;
  pthread_condattr_t *attrc = NULL;
  char cLog[1000];
  m_bCreateObjectResult = true;  //040309 Yu Wei
  m_bTaskWorking = true;
  m_bPollingEnable = true;  //Default the polling enable.

  m_nLinkType = tSWCPara.tHeader.nLinkType;  //Set interface type.
  strncpy(m_LanSWC_IP , tSWCPara.tHeader.tLanLink[0].ip, 20);
  m_tLanSWC_SockID =   tSWCPara.tHeader.tLanLink[0].port;
  m_nModeBusSeqence = 0;

//  ClearVariables();

  m_bCreateObjectResult = true;  //040309 Yu Wei
  m_bTaskWorking = true;
  m_bPollingEnable = true;  //Default the polling enable.
  m_nLinkType = tSWCPara.tHeader.nLinkType;  //Set interface type.
  attrm = (pthread_mutexattr_t *) malloc(sizeof(pthread_mutexattr_t));
  attrc = (pthread_condattr_t *) malloc(sizeof(pthread_condattr_t));
  pthread_mutexattr_init( attrm );
  pthread_condattr_init( attrc );
  pthread_mutex_init(& m_tAccessTableSemID, attrm );
  pthread_mutex_init(& SWC_main_wd_mutex, attrm ); //for watchdog
  pthread_cond_init(& m_tAccessTableSemID, attrc );
  pthread_cond_init(& SWC_main_wd_cond, attrc );//for watchdog
  free(attrm);
  free(attrc);

  char buffer[32]={0x00};
  strncpy(buffer ,SWC_MSGQ  , strlen(SWC_MSGQ));
  strcat(buffer , tSWCPara.tHeader.acName);
  m_tMSGQID = mq_open(buffer, O_RDWR|O_CREAT|O_NONBLOCK, S_IRUSR|S_IWUSR, NULL);
  if(m_tMSGQID == -1 )
    printf("m_tMSGQID  create failed \n");


  strcpy(m_acSWCName,tSWCPara.tHeader.acName);  //SWC name.

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN)
  printf("[SWC] CSWC_LAN::CSWC_LAN, SWC_NAME=%s\n", m_acSWCName);
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN)

  m_nSWCID = nDeviceID;              //SWC index.
  m_ucSWCAddress = tSWCPara.tHeader.ucID;      //SWC address.

  //Set SWC work in "INITIALIZATION" state.
  m_nSWCCurrentStateFlag = STATEFLAG_INITIALIZATION;

  //Set serial link and polling command buffer.
  m_nTotalLinkNumber = 0;

  if((tSWCPara.tHeader.nSendTimeSynHour < 24) &&
     (tSWCPara.tHeader.nSendTimeSynMinute <60))
  {
    m_bTimeSynFlag = true;
    m_bTimeSynStartFlag = true;
    m_nSendTimeSynHour = tSWCPara.tHeader.nSendTimeSynHour;
    m_nSendTimeSynMinute = tSWCPara.tHeader.nSendTimeSynMinute;
    ActivateTimeOut(PLC_TIME_SYN_CHK);
  }
  else
  {
    m_bTimeSynFlag = false;
    m_bTimeSynStartFlag = false;
  }

  //Check keep alive command.  //040812 Yu Wei
  if(tSWCPara.tHeader.nKeepAliveInterval != 0)
  {
    m_bKeepAliveFlag = true;
    m_nKeepAliveInterval = tSWCPara.tHeader.nKeepAliveInterval;
    m_unSendKeepAliveAddr = tSWCPara.tHeader.unSendKeepAliveAddr;
    m_unKeepAliveCmdWord = 0x0000;        //default is 00;
  }
  else
  {
    m_bKeepAliveFlag = false;
    m_nKeepAliveInterval = 0;
    m_unKeepAliveCmdWord = 0x0000;
  }



  int wcount = 0;
  while( wcount ++ < 4  )
  {

    if(Ping(tSWCPara.tHeader.tLanLink[0].ip)< 0)
    {
      if (wcount == 4)
      {
        #ifdef CFG_PRN_ERR
        printf("ERR  [SWC] CSWC_LAN::CSWC_LAN, the SWC LAN IP %s is down \n" ,
          tSWCPara.tHeader.tLanLink[0].ip);
        #endif // CFG_PRN_ERR
        break;
      }
    }
    else
    {
      break;
    }
  } //  while( wcount ++ < 4  )

  //m_anCommfd[0] = Connect(m_LanSWC_IP ,m_tLanSWC_SockID);//create connection
  rstatus = LNC_GetConnectFileDescriptor((const CHAR *)m_LanSWC_IP,
                                         (const UINT16) m_tLanSWC_SockID,
                                         SOCK_STREAM,
                                         2000, &m_anCommfd[0]);
  if(m_anCommfd[0] <= 0 )
  {
    m_anCommfd[0] = TYP_ERROR;
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_Init,
//      FLG_HDR_T_TimeStamp_DateAndTime,
//      FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//      "ERR  [SWC] CSWC_LAN::CSWC_LAN, %s fail to get file descriptor\n",
//      m_acSWCName);
    sprintf(cLog, "ERR  [SWC] CSWC_LAN::CSWC_LAN, %s fail to get file descriptor\n",
    	      m_acSWCName);
    g_pDebugLog->LogMessage(cLog);

  }
  else
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_Init,
//      FLG_HDR_T_TimeStamp_DateAndTime,
//      FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//      "[SWC] CSWC_LAN::CSWC_LAN,\n"
//      "  %s create LAN connection to %s:%d OK\n",
//      m_acSWCName, m_LanSWC_IP, m_tLanSWC_SockID);
    sprintf(cLog, "[SWC] CSWC_LAN::CSWC_LAN,\n"
    	      "  %s create LAN connection to %s:%d OK\n",
    	      m_acSWCName, m_LanSWC_IP, m_tLanSWC_SockID);
    g_pDebugLog->LogMessage(cLog);
  }



  m_nTotalLinkNumber = 1;

  int nI = 0 ;
  int nJ = 0;

  //106 command word , 107 Table1'status ,108 Table2'status ,109 Table3'status ,

  //fast polling command used when PLC is in state 0x0002
  //Fast polling command.
  SetReadCommand( m_acFastPollingCommand[nI],
          tSWCPara.tHeader.tProtocol[nJ].addr,
          PLC_MODBUS_READ_CMD_SIZE - PLC_MODBUS_HEARDER_LENTH + 1,
          tSWCPara.tHeader.tProtocol[nJ].command,
          tSWCPara.tPollingAddress.tFastPollingAddress);

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
  printf("[SWC] CSWC_LAN::CSWC_LAN, setup fast polling command: \n  0x");
  for(int pr=0; pr < PLC_MODBUS_READ_CMD_SIZE - PLC_MODBUS_HEARDER_LENTH + 1;
      pr++)
    printf("%02x " , (UINT8)m_acFastPollingCommand[nI][pr]);
  printf("\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))

  //polling command when start up 0x0008,to read all PLC status
  tTempAdr.unStart = tSWCPara.tPollingAddress.tFastPollingAddress.unStart;
  //30 records + 4words
  tTempAdr.unLength =
    ((tSWCPara.tPollingAddress.tFastPollingAddress.unLength - 4)/4 + 4);
  m_nFastPollingAllDataRecevLen =
    ((tSWCPara.tPollingAddress.tFastPollingAddress.unLength - 4)/4 + 4)*2+9;


  SetReadCommand( m_acFastPollingAllDataCommand[nI],
          tSWCPara.tHeader.tProtocol[nJ].addr,
          PLC_MODBUS_READ_CMD_SIZE - PLC_MODBUS_HEARDER_LENTH + 1,
          tSWCPara.tHeader.tProtocol[nJ].command,
          tTempAdr);      //Fast polling command.

  //slow polling command
  SetReadCommand(  m_acSlowPollingCommand[nI],
          tSWCPara.tHeader.tProtocol[nJ].addr,
          PLC_MODBUS_READ_CMD_SIZE - PLC_MODBUS_HEARDER_LENTH + 1,
          tSWCPara.tHeader.tProtocol[nJ].command,
          tSWCPara.tPollingAddress.tSlowPollingAddress);

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
  printf("[SWC] CSWC_LAN::CSWC_LAN, setup slow polling command: \n  0x");
  for(int pr=0; pr < PLC_MODBUS_READ_CMD_SIZE - PLC_MODBUS_HEARDER_LENTH + 1;
      pr++)
    printf("%02x " , (UINT8)m_acSlowPollingCommand[nI][pr]);
  printf("\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))

  m_nStatusPollingRecevLen = 17;//reply 4 word status

  tTempAdr.unStart = tSWCPara.tPollingAddress.tFastPollingAddress.unStart;
  tTempAdr.unLength = 0x0004;//4 words

  SetReadCommand( m_acStatusCommand[nI],
          tSWCPara.tHeader.tProtocol[nJ].addr,
          PLC_MODBUS_READ_CMD_SIZE - PLC_MODBUS_HEARDER_LENTH + 1,
          tSWCPara.tHeader.tProtocol[nJ].command,//0x03 or 0x04
          tTempAdr);

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
  printf("[SWC] CSWC_LAN::CSWC_LAN, setup timer polling command: \n  0x");
  for(int pr=0; pr < PLC_MODBUS_READ_CMD_SIZE - PLC_MODBUS_HEARDER_LENTH + 1;
          pr++)
    printf("%02x " , (UINT8)m_acStatusCommand[nI][pr]);
  printf("\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
  tTempAdr.unLength = 0x0001;//write 1 commmand word
  //get status polling command
  SetWriteCommand( m_acWriteStatusCommand[nI],
           acTemp,
           tSWCPara.tHeader.tProtocol[nJ].addr,
           0x10,
           tTempAdr);

  //Timer polling command.
  SetReadCommand(m_acTimerPollingCommand[nI],
          tSWCPara.tHeader.tProtocol[nJ].addr,
          PLC_MODBUS_READ_CMD_SIZE - PLC_MODBUS_HEARDER_LENTH + 1,
          tSWCPara.tHeader.tProtocol[nJ].command,
          tSWCPara.tPollingAddress.tTimerAddress);

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_TIMERPOLLING)
  printf("[SWC] CSWC_LAN, timer polling, MI, cmd for PLC %d:\n", nI);
  SYS_PrnDataBlock((const UINT8 *)m_acTimerPollingCommand[nI],
                   sizeof(MB_PLC_RQ_T), 10);
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_FASTPOLLING)

  if(m_bTimeSynFlag == true)
  {
    tTempAdr.unStart = tSWCPara.tHeader.unSendTimeSynStartAddr;
    tTempAdr.unLength = tSWCPara.tHeader.unSendTimeSynLength;
    SetWriteCommand(   m_acTimeSynCommand[nI],
                         acTemp,
                         //tSWCPara.tHeader.tLinkPara[nJ].cSlaveAddress,
                         tSWCPara.tHeader.tProtocol[0].addr,
                         0x10,
                         tTempAdr);
    m_nTimeSynCMDLength = tSWCPara.tHeader.unSendTimeSynLength*2 + 9;
    //printf("m_nTimeSynCMDLength == %d \n",m_nTimeSynCMDLength);
  }

  if(m_bKeepAliveFlag == true)
  {
    *(unsigned short *)&acTemp[0] =STR_ByteSwap16Bit( m_unKeepAliveCmdWord);
    tTempAdr.unStart = m_unSendKeepAliveAddr;
    tTempAdr.unLength = 0x0001;
    m_nKeepAliveCMDLength = SetWriteCommand(  m_acKeepAliveCommand[nI],
                          acTemp,
                          //tSWCPara.tHeader.tLinkPara[nJ].cSlaveAddress,
                          tSWCPara.tHeader.tProtocol[0].addr,
                          0x10,
                          tTempAdr);
  }

  m_anStandbyRecevLen[nI] = 0;    //Clear standby received data length.

  m_atPollingFlag[nI].bFast = false;
  m_atPollingFlag[nI].bSlow = false;
  m_atPollingFlag[nI].bStatus = false;
  m_atPollingFlag[nI].bSpecial = false;
  m_atPollingFlag[nI].bLinkCheck = false;
  m_atPollingFlag[nI].bTimeSyn = false;

  //Set primary link port ID.
  m_nPrimaryPortID = 0;
  m_nStandbyPortID = 0;
  m_nMasterLinkID = 0;
  m_anLinkStatus[0] = LINKERROR;
  m_anLinkStatus[1]  = NOLINK ;


  m_unCurrentLinkStatus = 0;  //Serial link status.

  //Polling parameter.
  m_nReceiveTimeout      = tSWCPara.tHeader.nReceiveTimeout;
  m_nFastPollingValue      = tSWCPara.tHeader.nFastPollingFrequency;
  m_nSlowPollingValue      = tSWCPara.tHeader.nSlowPollingFrequency;
  m_nRetryNumber        = tSWCPara.tHeader.nRetryNumber;
  m_nStandbyPollingValue    = tSWCPara.tHeader.nStandbyPollingFrequency;
  m_nStandbyPollingFailedValue= tSWCPara.tHeader.nFailedPollingFrequency;

  //Fast polling table init
  //2008 TCPIP ModeBus max data lenth is 0xff-9
  m_nFastPollingRecevLen =
    tSWCPara.tPollingAddress.tFastPollingAddress.unLength*2+9;
  m_nFastDataLen = tSWCPara.tPollingAddress.tFastPollingAddress.unLength;
  m_nServerFastTableStartAddr =
    tSWCPara.tPollingAddress.tFastPollingAddress.unServerStart;
  m_nServerFastTableLen =
    tSWCPara.tPollingAddress.tFastPollingAddress.unServerLength;
  if(m_nFastDataLen == 0)    //040813 Yu Wei
  {
    m_bCreateObjectResult = false;  //040309 Yu Wei
    #ifdef CFG_PRN_ERR
    printf("[SWC_LAN] CSWC_LAN::CSWC_LAN, create object fail\n");
    #endif // CFG_PRN_ERR
  }

  #if ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SWC_LAN_READ_FASTP_TBL))
  printf("[SWC] CSWC_LAN::CSWC_LAN, m_nFastPollingRecevLen = %d, "
         "m_nFastDataLen = %d, m_nServerFastTableStartAddr = %d, "
         "m_nServerFastTableLen = %d\n",
         m_nFastPollingRecevLen, m_nFastDataLen, m_nServerFastTableStartAddr,
         m_nServerFastTableLen);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))

  //Slow polling table init
  m_nSlowPollingRecevLen =
    tSWCPara.tPollingAddress.tSlowPollingAddress.unLength*2+9;//len*2+5
  m_nSlowDataLen = tSWCPara.tPollingAddress.tSlowPollingAddress.unLength;
  m_nServerSlowTableStartAddr =
    tSWCPara.tPollingAddress.tSlowPollingAddress.unServerStart;
  m_nServerSlowTableLen =
    tSWCPara.tPollingAddress.tSlowPollingAddress.unServerLength;

  //Startup, slow polling will do. 040312 Yu Wei
  m_bSlowPollingStart      = true;

  if(m_nSlowDataLen != 0)
  {
    m_bSlowPollingFlag = true;
  }
  else
    m_bSlowPollingFlag = false;

  //Timer polling table init
  m_nTimerPollingRecevLen    =
    tSWCPara.tPollingAddress.tTimerAddress.unLength*2+9;
  m_nTimerDataLen =
    tSWCPara.tPollingAddress.tTimerAddress.unLength;
  m_nServerTimerTableStartAddr =
    tSWCPara.tPollingAddress.tTimerAddress.unServerStart;
  m_nServerTimerTableLen =
    tSWCPara.tPollingAddress.tTimerAddress.unServerLength;

  m_nTimerPollingValue = tSWCPara.tPollingAddress.nTimerInterval;

  //Startup, timer polling will do. 040312 Yu Wei
  m_bTimerPollingStart    = true;

  if(m_nTimerDataLen != 0)
  {
    m_bTimerPollingFlag = true;
  }
  else
    m_bTimerPollingFlag = false;

  //Initialize m_tLanSWC_SockIDet listen object.
  m_pServerSocket = new CListen(
            acIPAddress,
            tSWCPara.tHeader.nSocketID,
            tSWCPara.tHeader.nLANLinkTimeout);

  #ifdef _CFG_DEBUG_MSG
  printf("[SWC] CSWC_LAN::CSWC_LAN, m_pServerSocket = 0x%08x\n",
          m_pServerSocket);
  #endif // CFG_DEBUG_MSG
  //Reset timeout counter.
  for(nI=0; nI<MAX_TIMEOUT_TYPE; nI++)
  {
    m_atTimeOut[nI].Low = 0;    //040520 Yu Wei
    m_atTimeOut[nI].High = 0;    //040520 Yu Wei
  }

  //Set standby RTU monitor link timeout type ID.
  m_anStandbyMonitorTimeout[0] = STANDBY_RTU_LINK1;
  m_anStandbyMonitorTimeout[1] = STANDBY_RTU_LINK2;

  m_bLinkCheckFlag = false;

  m_unFastPollingStartAdd =
    tSWCPara.tPollingAddress.tFastPollingAddress.unStart;
  m_unSlowPollingStartAdd =
    tSWCPara.tPollingAddress.tSlowPollingAddress.unStart;
  m_unTimerPollingStartAdd=
    tSWCPara.tPollingAddress.tTimerAddress.unStart;    //040322 Yu Wei

  if(WatchDogChid == TYP_NULL)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SWC] CSWC_LAN::CSWC_LAN, invalid WatchDogChid\n");
    return;
   #endif // CFG_PRN_ERR
  }
  SWC_main_timer_id = wdCreate(WatchDogChid , SWC_main_timer_id , SWC_MAIN ,
                               nDeviceID);

  if(SWC_main_timer_id == TYP_ERROR)
  {
    sprintf(acTemp,"%s Create watchdog ID error.\n",m_acSWCName);
    g_pEventLog->LogMessage(acTemp);
    m_bCreateObjectResult = false;  //040309 Yu Wei
  }

  m_nSpecificCommandLen = 0;    //040302 Yu Wei

  m_bStartupCheckLink = true;    //Startup, check link.  //040520 Yu Wei

  //040707 Yu Wei
  m_nSWCBusyTimeout = tSWCPara.tHeader.nSWCBusyTimeout;
  m_nExcepTimeSyncRetry = tSWCPara.tHeader.nExcepTimeSyncRetry;
  m_nTimeSyncExcepCounter = 0;

  m_nExcepRetry = tSWCPara.tHeader.nExcepRetry;
  m_nFastExcepCounter = 0;
  m_nSlowExcepCounter = 0;
  m_nTimerExcepCounter = 0;
  //040707 Yu Wei

  m_bCheckingBeforeSwitch = false;//20050518

#ifdef ModuleTest
  LogSwcInitValues(m_acSWCName);    //for test.
#endif



  m_nRecordEndPoint = 0 ;
  m_nRecordStartPoint = 0 ;
  m_usPacketTransactionNum = 0;

}


CSWC_LAN::~CSWC_LAN()
{
}


/*******************************************************************************
Purpose
  This routine will send request to SWC, acquire reply message and decode the
  received message.

Input
  None

Return
  None

History
    Name         Date          Remark
    ----         ----          ------
  Bryan Chong  08-Dec-2009   Update m_nStatusPollingRecevLen as specified from
                             the config file
  Bryan Chong  08-Jun-2010   Update to use routine
                             RequestAndReceiveWithModbusCheck for polling
  Bryan Chong  07-Sep-2011   Replace array index zero with m_nSWCID to allow
                             flexibility of PLC indexing [C955, PR56]
*******************************************************************************/
VOID CSWC_LAN::FastPolling(VOID)
{
  E_ERR_T rstatus = E_ERR_Success;
  char acRecevData[2048] = {0};
  char acTemp[200];
  //int nModbusReturn;
  MB_PLC_RQ_T *pmb_req;
  UINT16 datarxsz;
  CHAR acErrMsg[100] = {0};
  char cLog[1000];

  if((m_anLinkStatus[m_nPrimaryPortID] == LINKERROR) ||
     (m_anCommfd[m_nPrimaryPortID] == TYP_ERROR) ||
     (CheckTimeOut(FASTPOLLING,m_nFastPollingValue) == false))
  {
    return;
  }

  ActivateTimeOut(FASTPOLLING);

  /* get fast polling table length as specified from the config file */
  // BC 20110907
  m_acStatusCommand[m_nPrimaryPortID][11] =
    g_tRTUConfig.tSWC[m_nSWCID].tPollingAddress.tFastPollingAddress.unLength;

  pmb_req = (MB_PLC_RQ_T *)&m_acStatusCommand[m_nPrimaryPortID];

  /* receive header length is 9 bytes, swap and add header to compute the
     message length */
  datarxsz = STR_SWAP_16BIT_REG(pmb_req->data_word_length);
  m_nStatusPollingRecevLen = (sizeof(MB_PLC_RESP_T)) +
                             (2*datarxsz) - 1;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_FASTPOLLING)
  printf("[SWC] CSWC_LAN::FastPolling, idx %d, m_nStatusPollingRecevLen = %d, "
          "word length = %d\n",
         m_nSWCID, m_nStatusPollingRecevLen,
         datarxsz);
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_FASTPOLLING)

//  pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_FastPolling,
//      FLG_HDR_T_TimeStamp_DateAndTime,
//      FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//      "[SWC] CSWC_LAN::FastPolling, %s fd %d tx %d bytes:\n",
//      m_acSWCName, m_anCommfd[m_nPrimaryPortID], sizeof(MB_PLC_RQ_T));
//
//  pFLG_CtrlBlk->SendDataBlock(E_FLG_MSG_SWC_LAN_FastPolling, FLG_LOG_T_Debug,
//    (UINT8 *)m_acStatusCommand[m_nPrimaryPortID], sizeof(MB_PLC_RQ_T), 10);

  rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nPrimaryPortID],
      (UINT8 *)m_acStatusCommand[m_nPrimaryPortID], sizeof(MB_PLC_RQ_T),
      acRecevData, (UINT32)m_nStatusPollingRecevLen, m_nReceiveTimeout,
      (UINT8)m_nRetryNumber, (UINT8)m_nExcepRetry,
      (UINT8 *)&m_nFastExcepCounter);

  switch(rstatus)
  {
    case E_ERR_Success:
    case E_ERR_SWC_ModbusExceptionError:
    	if(rstatus==E_ERR_Success)
       		m_nFastExcepCounter = 0;
    	else
    		m_nFastExcepCounter++;

        m_anLinkStatus[m_nPrimaryPortID] = LINKOK;


      // copy data from SWC table to server table starting from time stamp
      memset(m_aunSWCTableFast, 0, sizeof(m_aunSWCTableFast));
      WriteSWCTable(m_aunSWCTableFast, &acRecevData[9], m_nServerFastTableLen);
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_FASTPOLLING)
      printf("[SWC] CSWC_LAN::FastPolling, fd %d rx %d bytes:\n",
             m_anCommfd[m_nPrimaryPortID], m_nStatusPollingRecevLen);
      SYS_PrnDataBlock((const UINT8 *)acRecevData, m_nStatusPollingRecevLen,
                       20);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_FASTPOLLING)
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_FastPolling,
//        FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//        "[SWC] CSWC_LAN::FastPolling, %s fd %d rx %d bytes:\n",
//        m_acSWCName, m_anCommfd[m_nPrimaryPortID], m_nStatusPollingRecevLen);
//      sprintf(cLog, "[SWC] CSWC_LAN::FastPolling, %s fd %d rx %d bytes:\n",
//    	        m_acSWCName, m_anCommfd[m_nPrimaryPortID], m_nStatusPollingRecevLen);
//      g_pDebugLog->LogMessage(cLog);
//
//      pFLG_CtrlBlk->SendDataBlock(E_FLG_MSG_SWC_LAN_FastPolling,
//        FLG_LOG_T_Debug, (UINT8 *)acRecevData, m_nStatusPollingRecevLen, 10);
      break;


    case E_ERR_SWC_ExceedRetryLimit:
//      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
//      close(m_anCommfd[m_nPrimaryPortID]);
//      m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
//
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_FASTPOLLING)
//      printf("[SWC] CSWC_LAN::FastPolling, %s exceed retry limit. "
//             "Set link = LINKERROR \n",
//             m_acSWCName, m_nRetryNumber);
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
//      break;

//    case E_ERR_SWC_ExceedExceptionRetryLimit:
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_FASTPOLLING)
//      printf("[SWC] CSWC_LAN::FastPolling, exceed exception retry "
//             "limit, %d/%d\n", m_nFastExcepCounter, m_nExcepRetry);
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
//      break;

    case E_ERR_LNC_SelectReadError:
    case E_ERR_LNC_SelectReadTimeOut:
    case E_ERR_LNC_FailToWriteSocket:
    default:
//    case E_ERR_SWC_NoResponseByte:
//    case E_ERR_LNC_FailToConnectStreamSocket:
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_FASTPOLLING)
      printf("[SWC] CSWC_LAN::FastPolling, %s select read timeout link "
             "down. Close socket fd %d\n",
             m_acSWCName, m_anCommfd[m_nPrimaryPortID]);
      #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_FASTPOLLING)
      ERR_GetMsgString(rstatus, acErrMsg);

      sprintf(cLog, "[SWC] CSWC_LAN::FastPolling, %s link down, close fd %d. %s\n",
    	        m_acSWCName, m_anCommfd[m_nPrimaryPortID], acErrMsg);
      g_pDebugLog->LogMessage(cLog);
      printf("[SWC] CSWC_LAN::FastPolling, %s link down, close fd %d. %s\n",
          	        m_acSWCName, m_anCommfd[m_nPrimaryPortID], acErrMsg);
// YT 3/20/2015
            m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
            close(m_anCommfd[m_nPrimaryPortID]);
            m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
      ActivateTimeOut(LinkDownMoreThanOneDay);

      //Write event log.
      sprintf(acTemp,"%s FastPolling Primary LAN link(%d) is down.",
              m_acSWCName, m_nPrimaryPortID);
      g_pEventLog->LogMessage(acTemp);
      break;

//    default:
//      #ifdef CFG_PRN_ERR
//      ERR_GetMsgString(rstatus, acErrMsg);
//      printf("ERR  [SWC] CSWC_LAN::FastPolling, %s unhandled error, %s\n",
//             m_acSWCName, acErrMsg);
//      #endif //CFG_PRN_ERR
//      break;
  }// switch(rstatus)
} // CSWC_LAN::FastPolling
/*******************************************************************************
Purpose
  This routine will manage link check time out, activate timer, and increment
  transaction number for modbus packet.

Input
  None

Return
  None

History
    Name         Date          Remark
    ----         ----          ------
  Yu, Wei      22-Mar-2004   Initial revision
  Bryan Chong  08-Dec-2009   Update m_nStatusPollingRecevLen as specified from
                             the config file
  Bryan Chong  08-Jun-2010   Update to use routine
                             RequestAndReceiveWithModbusCheck for polling

*******************************************************************************/
void CSWC_LAN::PrimaryLinkCheck(void)
{
  E_ERR_T rstatus = E_ERR_Success;
  char acRecevData[2048];
  UINT8 ucexceptionCnt = 0;
  CHAR acErrMsg[100] = {0};
  char cLog[1000];
  char acTemp1[200];int nI;
  if(m_bStartupCheckLink == false)
    return;

  // check link status
  if(m_anLinkStatus[m_nPrimaryPortID] != LINKERROR)
  {
    #if ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)
    MDT_PrnToLog(CFG_MDT_CMM_LOG, m_acSWCName,
                 (CHAR *)"[MT] CSWC_LAN::PrimaryLinkCheck, found link = "
                 "LINKERROR\n");
    #endif // ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)
    CheckLinkStatusChange();
    return;
  }

  // check if link down more than one day
  if(CheckTimeOut(LinkDownMoreThanOneDay,24*60*60*1000+1)== true)
  {
    //WriteCommandWordToPLC(0x0004);
    ActivateTimeOut(LinkDownMoreThanOneDay);
  }

  if(CheckTimeOut(PRIMARYSTATUS, m_nStandbyPollingFailedValue) == false)
    return;

  //Link check will trigger at every F_POLLING interval

  ActivateTimeOut(PRIMARYSTATUS);

  if((m_anCommfd[m_nPrimaryPortID] == TYP_ERROR) ||
     (m_anCommfd[m_nPrimaryPortID] == TYP_NULL))
  {
    rstatus = LNC_GetConnectFileDescriptor((const CHAR *)m_LanSWC_IP,
                m_tLanSWC_SockID, SOCK_STREAM, 500,
                (INT32 *)&m_anCommfd[m_nPrimaryPortID]);
    #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_SWC_LAN)
    printf("[SWC] CSWC_LAN::PrimaryLinkCheck, connect to PLC %s:%d,\n"
           "rstatus %d\n",
           m_LanSWC_IP, m_tLanSWC_SockID, rstatus);
    #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
    if(rstatus != E_ERR_Success)
    {
      m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
      #if ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)
      MDT_PrnToLog(CFG_MDT_CMM_LOG, m_acSWCName,
                   (CHAR *)"[MT] CSWC_LAN::PrimaryLinkCheck, invalid fd\n");
      #endif // ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN)
      printf("[SWC] CSWC_LAN::PrimaryLinkCheck, connect to PLC %s:%d fail\n",
             m_LanSWC_IP, m_tLanSWC_SockID, rstatus);
      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
      CheckLinkStatusChange();
      return;
    }
  }// if((m_anCommfd[m_nPrimaryPortID] == TYP_ERROR) ||
     //  (m_anCommfd[m_nPrimaryPortID] == TYP_NULL))

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN)
  printf("[SWC] CSWC_LAN::PrimaryLinkCheck, fd %d tx %d bytes:\n",
         m_anCommfd[m_nPrimaryPortID], sizeof(MB_PLC_RQ_T));
  SYS_PrnDataBlock((const UINT8 *) m_acStatusCommand[m_nPrimaryPortID],
                   sizeof(MB_PLC_RQ_T), 20);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))

//  pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_PrimaryLinkCheck,
//    FLG_HDR_T_TimeStamp_DateAndTime,
//    FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//    "[SWC] CSWC_LAN::PrimaryLinkCheck, fd %d tx %d bytes:\n",
//    m_anCommfd[m_nPrimaryPortID], sizeof(MB_PLC_RQ_T));
//
//  pFLG_CtrlBlk->SendDataBlock(E_FLG_MSG_SWC_LAN_PrimaryLinkCheck,
//    FLG_LOG_T_Debug, (UINT8 *)m_acStatusCommand[m_nPrimaryPortID],
//    sizeof(MB_PLC_RQ_T), 10);

  rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nPrimaryPortID],
      (UINT8 *)m_acStatusCommand[m_nPrimaryPortID], sizeof(MB_PLC_RQ_T),
      acRecevData, (UINT32)m_nStatusPollingRecevLen, m_nReceiveTimeout,
      1, (UINT8)m_nExcepRetry, &ucexceptionCnt);

  switch(rstatus)
  {
    case E_ERR_Success:
    case E_ERR_SWC_ModbusExceptionError:
    	if(rstatus==E_ERR_Success)
       		m_nFastExcepCounter = 0;
    	else
    		m_nFastExcepCounter++;
      //WriteSWCTable(m_aunSWCTableFast,&acRecevData[3], m_nFastDataLen); 20150603 Su, no need to update primarylinkcheck data to server
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN)
      printf("[SWC] CSWC_LAN::PrimaryLinkCheck, rx %d bytes:\n",
              m_nStatusPollingRecevLen);
      SYS_PrnDataBlock((const UINT8 *)acRecevData, m_nStatusPollingRecevLen,
                       20);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))

      //enable slow polling flag
      m_bSlowPollingStart = true;

      //enable timer polling flag
      m_bTimerPollingStart = true;

      if(m_anLinkStatus[m_nPrimaryPortID] != LINKOK)
      {
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN)
        printf("[SWC] CSWC_LAN::PrimaryLinkCheck, %s update link from %d "
               "to LINKOK, fd %d\n",
               m_acSWCName, m_anLinkStatus[m_nPrimaryPortID],
               m_anCommfd[m_nPrimaryPortID]);
        #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
        m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
      }

      CheckLinkStatusChange();

//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_PrimaryLinkCheck,
//        FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//        "[SWC] CSWC_LAN::PrimaryLinkCheck, fd %d rx %d bytes:\n",
//        m_anCommfd[m_nPrimaryPortID], m_nStatusPollingRecevLen);

      for(nI=0; nI<m_nStatusPollingRecevLen; nI++)
      sprintf(&acTemp1[nI*2], "%02X",(unsigned char)acRecevData[nI]);
      sprintf(cLog, "[SWC] CSWC_LAN::PrimaryLinkCheck, fd %d rx %d bytes:%s\n",
    	        m_anCommfd[m_nPrimaryPortID], m_nStatusPollingRecevLen,acTemp1);
      g_pDebugLog->LogMessage(cLog);

//      pFLG_CtrlBlk->SendDataBlock(E_FLG_MSG_SWC_LAN_PrimaryLinkCheck,
//        FLG_LOG_T_Debug, (UINT8 *)acRecevData, m_nStatusPollingRecevLen, 10);

      break;

    case E_ERR_SWC_ExceedRetryLimit:
//      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
//      close(m_anCommfd[m_nPrimaryPortID]);
//      m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
//
//      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
//      printf("[SWC] CSWC_LAN::PrimaryLinkCheck, %s exceed retry limit. "
//             "Set link = LINKERROR \n",
//             m_acSWCName, m_nRetryNumber);
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
//      break;
//
//    case E_ERR_SWC_ExceedExceptionRetryLimit:
//      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
//      close(m_anCommfd[m_nPrimaryPortID]);
//      m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;

//      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
//      printf("[SWC] CSWC_LAN::PrimaryLinkCheck, exceed exception retry "
//             "limit, %d/%d\n", ucexceptionCnt, m_nExcepRetry);
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
//      break;


    case E_ERR_LNC_SelectReadTimeOut:
    case E_ERR_LNC_FailToWriteSocket:
    case E_ERR_SWC_NoResponseByte:
    case E_ERR_LNC_FailToConnectStreamSocket:
//      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
//      printf("[SWC] CSWC_LAN::PrimaryLinkCheck, select read timeout link "
//             "down. Close socket fd %d\n",
//             m_anCommfd[m_nPrimaryPortID]);
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
//      ERR_GetMsgString(rstatus, acErrMsg);
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_PrimaryLinkCheck,
//        FLG_HDR_T_TimeStamp_DateAndTime,
//        FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//        "[SWC] CSWC_LAN::PrimaryLinkCheck, %s link down, close fd %d\n",
//        m_acSWCName, m_anCommfd[m_nPrimaryPortID], acErrMsg);
//
//      close(m_anCommfd[m_nPrimaryPortID]);
//      m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
//      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
//      break;

    default:
      ERR_GetMsgString(rstatus, acErrMsg);
      #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_LAN)
      printf("WARN [SWC] CSWC_LAN::PrimaryLinkCheck, %s unhandled error, %s\n",
             m_acSWCName, acErrMsg);
      #endif //((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_LAN)
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_PrimaryLinkCheck,
//        FLG_HDR_T_TimeStamp_DateAndTime,
//        FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//        "[SWC] CSWC_LAN::PrimaryLinkCheck, %s Primarylink Check error, %s. link down, "
//        "close fd %d\n",
//        m_acSWCName, acErrMsg, m_anCommfd[m_nPrimaryPortID]);
      sprintf(cLog, "[SWC] CSWC_LAN::PrimaryLinkCheck, %s Primarylink Check error, %s. link down, "
    	        "close fd %d\n",
    	        m_acSWCName, acErrMsg, m_anCommfd[m_nPrimaryPortID]);
      g_pDebugLog->LogMessage(cLog);
      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
      close(m_anCommfd[m_nPrimaryPortID]);
      m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;

      break;
  }// switch(rstatus)
  // if((CheckTimeOut(PRIMARYSTATUS,m_nStandbyPollingFailedValue)== true) ||
                     // (m_bStartupCheckLink == true))

  #if ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)
  MDT_PrnToLog(CFG_MDT_CMM_LOG, m_acSWCName,
               (CHAR *)"[MT] CSWC_LAN::PrimaryLinkCheck, completing primary "
               "link check\n");
  #endif // ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)
  CheckLinkStatusChange();

}  // CSWC_LAN::PrimaryLinkCheck
/*******************************************************************************
Purpose
  This routine will get slow polling data and updata SWC table at the
  slow polling column.

Input
  None

Return
  None

History
    Name         Date          Remark
    ----         ----          ------
  Yu, Wei      22-Mar-2004   Initial revision
  Bryan Chong  02-Jun-2011   Update to use routine
                             RequestAndReceiveWithModbusCheck for polling
  Bryan Chong  06-Jan-2012   Update ActivateTimeOut from TIMERPOLLING to
                             SLOWPOLLING
*******************************************************************************/
void CSWC_LAN::SlowPolling(void)
{
  char acRecevData[2048];
  char acTemp[200];
  char cLog[1000];
  E_ERR_T rstatus = E_ERR_Success;
  MB_PLC_RQ_T *pmb_req;
  UINT16 datarxsz;
  CHAR acErrMsg[100] = {0};

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SLOWPOLLING)
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_TIMERPOLLING)

  if((m_anLinkStatus[m_nPrimaryPortID] == LINKERROR) ||
     (m_anCommfd[m_nPrimaryPortID] == TYP_ERROR) ||
     (m_bSlowPollingFlag == false))
    return;


  //Slow polling will happen at TIMER_INTERVAL time
  if((CheckTimeOut(SLOWPOLLING, m_nSlowPollingValue) == false) &&
     (m_bSlowPollingStart == false))
    return;

  // 20120106 BC
  ActivateTimeOut(SLOWPOLLING);

  pmb_req = (MB_PLC_RQ_T *)m_acSlowPollingCommand[m_nPrimaryPortID];
  datarxsz = STR_SWAP_16BIT_REG(pmb_req->data_word_length);
  m_nSlowPollingRecevLen = sizeof(MB_PLC_RESP_T) + 2*datarxsz - 1;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SLOWPOLLING)
  printf("[SWC] CSWC_LAN::SlowPolling, fd %d tx %d bytes to %s:\n",
         m_anCommfd[m_nPrimaryPortID], sizeof(MB_PLC_RQ_T), m_acSWCName);
  SYS_PrnDataBlock((const UINT8 *)m_acSlowPollingCommand[m_nPrimaryPortID],
                   sizeof(MB_PLC_RQ_T), 20);
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_FASTPOLLING)

//  pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_SlowPolling,
//    FLG_HDR_T_TimeStamp_DateAndTime,
//    FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//    "[SWC] CSWC_LAN::SlowPolling, %s fd %d tx %d bytes:\n",
//    m_acSWCName, m_anCommfd[m_nPrimaryPortID], sizeof(MB_PLC_RQ_T));
//
//  pFLG_CtrlBlk->SendDataBlock(E_FLG_MSG_SWC_LAN_SlowPolling,
//    FLG_LOG_T_Debug, (UINT8 *)m_acSlowPollingCommand[m_nPrimaryPortID],
//    sizeof(MB_PLC_RQ_T), 20);

  rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nPrimaryPortID],
    (UINT8 *)m_acSlowPollingCommand[m_nPrimaryPortID], sizeof(MB_PLC_RQ_T),
    acRecevData, (UINT32)m_nSlowPollingRecevLen, m_nReceiveTimeout,
    (UINT8)m_nRetryNumber, (UINT8)m_nExcepRetry,
    (UINT8 *)&m_nFastExcepCounter);

  switch(rstatus)
  {
    case E_ERR_Success:
    case E_ERR_SWC_ModbusExceptionError:
    	if(rstatus==E_ERR_Success)
       		m_nFastExcepCounter = 0;
    	else
    		m_nFastExcepCounter++;
//      if(m_anLinkStatus[m_nPrimaryPortID] == LINKERROR)
//      {
        m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
        m_bSlowPollingStart = false;

      // copy data from SWC table to server table starting from time stamp
      // 20100804 BC (Rqd ZSL)
      memset(m_aunSWCTableSlow, 0, sizeof(m_aunSWCTableSlow));
      WriteSWCTable(m_aunSWCTableSlow, &acRecevData[9], m_nSlowDataLen);
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SLOWPOLLING)
      rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                           E_SYS_TIME_Local);
      printf("[SWC] %d.%02d.%02d %02d:%02d:%02d.%03d "
             "CSWC_LAN::SlowPolling, %s fd %d rx %d bytes:\n",
             (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
             pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
             pbdtime->tm_sec, tspec.tv_nsec/1000000,
             m_acSWCName, m_anCommfd[m_nPrimaryPortID],
             m_nSlowPollingRecevLen);
      SYS_PrnDataBlock((const UINT8 *)acRecevData, m_nSlowPollingRecevLen,
                       20);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN_TIMERPOLLING))
      break;

    case E_ERR_SWC_ExceedRetryLimit:
//      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
//      close(m_anCommfd[m_nPrimaryPortID]);
//      m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
//
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SLOWPOLLING)
//      printf("[SWC] CSWC_LAN::SlowPolling, %s exceed retry limit. "
//             "Set link = LINKERROR \n",
//             m_acSWCName, m_nRetryNumber);
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
//      break;

//    case E_ERR_SWC_ExceedExceptionRetryLimit:
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SLOWPOLLING)
//      printf("[SWC] CSWC_LAN::SlowPolling, %s exceed exception retry "
//             "limit, %d/%d\n",
//             m_acSWCName, m_nSlowExcepCounter, m_nExcepRetry);
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
//      break;
    case E_ERR_LNC_SelectReadError:
    case E_ERR_LNC_SelectReadTimeOut:
    case E_ERR_LNC_FailToWriteSocket:
    default:
//    case E_ERR_SWC_NoResponseByte:
//    case E_ERR_LNC_FailToConnectStreamSocket:
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SLOWPOLLING)
      printf("[SWC] CSWC_LAN::SlowPolling, %s select read timeout link "
             "down. Close socket fd %d\n",
             m_acSWCName, m_anCommfd[m_nPrimaryPortID]);
      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
      ERR_GetMsgString(rstatus, acErrMsg);
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_SlowPolling,
//        FLG_HDR_T_TimeStamp_DateAndTime,
//        FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//        "[SWC] CSWC_LAN::SlowPolling, %s link down, close fd %d. %s\n",
//        m_acSWCName, m_anCommfd[m_nPrimaryPortID], acErrMsg);
      sprintf(cLog,  "[SWC] CSWC_LAN::SlowPolling, %s link down, close fd %d. %s\n",
    	        m_acSWCName, m_anCommfd[m_nPrimaryPortID], acErrMsg);
      g_pDebugLog->LogMessage(cLog);

      close(m_anCommfd[m_nPrimaryPortID]);
      m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
      m_bSlowPollingStart = false;
      ActivateTimeOut(LinkDownMoreThanOneDay);

      //Write event log.
      sprintf(acTemp,"[SWC] CSWC_LAN::SlowPolling, link down, %s close fd "
              "%d\n",
              m_acSWCName, m_anCommfd[m_nPrimaryPortID]);
      g_pEventLog->LogMessage(acTemp);

      break;

//    default:
//      #ifdef CFG_PRN_ERR
//      ERR_GetMsgString(rstatus, acErrMsg);
//      printf("ERR  [SWC] CSWC_LAN::SlowPolling, %s unhandled error, %s\n",
//             m_acSWCName, acErrMsg);
//      #endif //CFG_PRN_ERR
//      break;
  }// switch(rstatus)
} // CSWC_LAN::TimerPolling


/*******************************************************************************
Purpose
  This routine will get timer polling data and updata SWC table at the  timer
  polling column.

Input
  None

Return
  None

History
    Name         Date          Remark
    ----         ----          ------
  Yu, Wei      22-Mar-2004   Initial revision
  Bryan Chong  09-Jun-2010   Update to use routine
                             RequestAndReceiveWithModbusCheck for polling
  Bryan Chong  04-Aug-2010   Rectify timer polling write to incorrect positiion
                             at server table
  Bryan Chong  05-Dec-2011   Clear MI receiving buffer, m_aunSWCTableTimer,
                             before copying to avoid MI overlapping DI table.
                             [C955,PR86]
*******************************************************************************/
void CSWC_LAN::TimerPolling(void)
{
  char acRecevData[2048] = {0};
  char acTemp[200];
  char cLog[1000];
  E_ERR_T rstatus = E_ERR_Success;
  MB_PLC_RQ_T *pmb_req;
  UINT16 datarxsz;
  CHAR acErrMsg[100] = {0};

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_TIMERPOLLING)
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_TIMERPOLLING)

  if((m_anLinkStatus[m_nPrimaryPortID] == LINKERROR) ||
     (m_anCommfd[m_nPrimaryPortID] == TYP_ERROR) ||
     (m_bTimerPollingFlag == false))
  {
    return;
  }

  //Timer polling will happen at TIMER_INTERVAL time
  if((CheckTimeOut(TIMERPOLLING, m_nTimerPollingValue)== false) &&
     (m_bTimerPollingStart == false))
    return;


  ActivateTimeOut(TIMERPOLLING);

  pmb_req = (MB_PLC_RQ_T *)m_acTimerPollingCommand[m_nPrimaryPortID];
  datarxsz = STR_SWAP_16BIT_REG(pmb_req->data_word_length);
  m_nTimerPollingRecevLen = sizeof(MB_PLC_RESP_T) + 2*datarxsz - 1;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_TIMERPOLLING)
  printf("[SWC] CSWC_LAN::TimerPolling, fd %d tx %d bytes to PLC:\n",
         m_anCommfd[m_nPrimaryPortID], sizeof(MB_PLC_RQ_T));
  SYS_PrnDataBlock((const UINT8 *)m_acTimerPollingCommand[m_nPrimaryPortID],
                   sizeof(MB_PLC_RQ_T), 20);
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_FASTPOLLING)

//  pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_TimerPolling,
//    FLG_HDR_T_TimeStamp_DateAndTime,
//    FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//    "[SWC] CSWC_LAN::TimerPolling, %s fd %d tx %d bytes:\n",
//    m_acSWCName, m_anCommfd[m_nPrimaryPortID], sizeof(MB_PLC_RQ_T));
//
//  pFLG_CtrlBlk->SendDataBlock(E_FLG_MSG_SWC_LAN_TimerPolling,
//    FLG_LOG_T_Debug, (UINT8 *)m_acTimerPollingCommand[m_nPrimaryPortID],
//    sizeof(MB_PLC_RQ_T), 10);

  rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nPrimaryPortID],
    (UINT8 *)m_acTimerPollingCommand[m_nPrimaryPortID], sizeof(MB_PLC_RQ_T),
    acRecevData, (UINT32)m_nTimerPollingRecevLen, m_nReceiveTimeout,
    (UINT8)m_nRetryNumber, (UINT8)m_nExcepRetry,
    (UINT8 *)&m_nFastExcepCounter);

  switch(rstatus)
  {
    case E_ERR_Success:
    case E_ERR_SWC_ModbusExceptionError:

    	if(rstatus==E_ERR_Success)
       		m_nFastExcepCounter = 0;
    	else
    		m_nFastExcepCounter++;

      m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
      m_bTimerPollingStart = false;

      // copy data from SWC table to server table starting from time stamp
      // 20100804 BC (Rqd ZSL)
      // 20111205 BC
      memset(m_aunSWCTableTimer, 0, sizeof(m_aunSWCTableTimer));
      WriteSWCTable(m_aunSWCTableTimer, &acRecevData[9], m_nTimerDataLen);
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_TIMERPOLLING)
      rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                           E_SYS_TIME_Local);
      printf("[SWC] %d.%02d.%02d %02d:%02d:%02d.%03d "
             "CSWC_LAN::TimerPolling, %s fd %d rx %d bytes:\n",
             (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
             pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
             pbdtime->tm_sec, tspec.tv_nsec/1000000,
             m_acSWCName, m_anCommfd[m_nPrimaryPortID],
             m_nTimerPollingRecevLen);
      SYS_PrnDataBlock((const UINT8 *)acRecevData, m_nTimerPollingRecevLen,
                       20);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN_TIMERPOLLING))

      break;

    case E_ERR_SWC_ExceedRetryLimit:
      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
      close(m_anCommfd[m_nPrimaryPortID]);
      m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;

      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_TIMERPOLLING)
      printf("[SWC] CSWC_LAN::TimerPolling, %s exceed retry limit. "
             "Set link = LINKERROR \n",
             m_acSWCName, m_nRetryNumber);
      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
      break;

//    case E_ERR_SWC_ExceedExceptionRetryLimit:
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_TIMERPOLLING)
//      printf("[SWC] CSWC_LAN::TimerPolling, %s exceed exception retry "
//             "limit, %d/%d\n",
//             m_acSWCName, m_nTimerExcepCounter, m_nExcepRetry);
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
//      break;
    case E_ERR_LNC_SelectReadError:
    case E_ERR_LNC_SelectReadTimeOut:
    case E_ERR_LNC_FailToWriteSocket:
    default:
//    case E_ERR_SWC_NoResponseByte:
//    case E_ERR_LNC_FailToConnectStreamSocket:
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_TIMERPOLLING)
      printf("[SWC] CSWC_LAN::TimerPolling, %s select read timeout link "
             "down. Close socket fd %d\n",
             m_acSWCName, m_anCommfd[m_nPrimaryPortID]);
      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
      ERR_GetMsgString(rstatus, acErrMsg);
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_TimerPolling,
//        FLG_HDR_T_TimeStamp_DateAndTime,
//        FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//        "[SWC] CSWC_LAN::TimerPolling, %s link down, close fd %d. %s\n",
//        m_acSWCName, m_anCommfd[m_nPrimaryPortID], acErrMsg);
      sprintf(cLog, "[SWC] CSWC_LAN::TimerPolling, %s link down, close fd %d. %s\n",
    	        m_acSWCName, m_anCommfd[m_nPrimaryPortID], acErrMsg);
      g_pDebugLog->LogMessage(cLog);


      close(m_anCommfd[m_nPrimaryPortID]);
      m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
      m_bTimerPollingStart = false;
      ActivateTimeOut(LinkDownMoreThanOneDay);

      //Write event log.
      sprintf(acTemp,"%s TimerPolling Primary LAN link(%d) breakdown.",
              m_acSWCName, m_nPrimaryPortID);
      g_pEventLog->LogMessage(acTemp);
      break;

//    default:
//      #ifdef CFG_PRN_ERR
//      ERR_GetMsgString(rstatus, acErrMsg);
//      printf("ERR  [SWC] CSWC_LAN::TimerPolling, %s unhandled error, %s\n",
//             m_acSWCName, acErrMsg);
//      #endif //CFG_PRN_ERR
//      break;
  }// switch(rstatus)
} // CSWC_LAN::TimerPolling
/*******************************************************************************
Purpose
  Construct time syn modbus packet

Parameter
  unReady  [in] Time validation flag

Return
  None

History
    Name         Date          Remark
    ----         ----          ------
  Yu, Wei      22-Mar-2004   Initial revision
  Bryan Chong  22-Oct-2010   Update to use local time

*******************************************************************************/
void CSWC_LAN::SetTimeSynCommand(unsigned short unReady)
{
  struct timespec tTime;
  struct tm *tFormatTime;

  clock_gettime(CLOCK_REALTIME, &tTime);    //Get current UTC time
  // 20101022 BC
  tFormatTime = localtime(&tTime.tv_sec);    //Change format to tm

  //Set time data.
  m_acTimeSynCommand[m_nPrimaryPortID][5] = 17;


  m_acTimeSynCommand[m_nPrimaryPortID][10] = 0;//number of word .high
  m_acTimeSynCommand[m_nPrimaryPortID][11] = 5;//number of word .low
  m_acTimeSynCommand[m_nPrimaryPortID][12] = 10;//number of bytes

  m_acTimeSynCommand[m_nPrimaryPortID][13] =
    HexToBCD((char)tFormatTime->tm_mday);
  m_acTimeSynCommand[m_nPrimaryPortID][14] =
    HexToBCD((char)(tFormatTime->tm_mon + 1));
  m_acTimeSynCommand[m_nPrimaryPortID][15] =
    HexToBCD((char)((tFormatTime->tm_year + 1900)/100));
  m_acTimeSynCommand[m_nPrimaryPortID][16] =
    HexToBCD((char)((tFormatTime->tm_year + 1900)%100));
  m_acTimeSynCommand[m_nPrimaryPortID][17] =
    HexToBCD((char)tFormatTime->tm_hour);
  m_acTimeSynCommand[m_nPrimaryPortID][18] =
    HexToBCD((char)tFormatTime->tm_min);
  m_acTimeSynCommand[m_nPrimaryPortID][19] =
    HexToBCD((char)tFormatTime->tm_sec);
  m_acTimeSynCommand[m_nPrimaryPortID][20] = 0;  //Not used
  *(unsigned short *)&m_acTimeSynCommand[m_nPrimaryPortID][21] =  unReady;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_TIMESYNC)
  printf("[SWC] CSWC_LAN::SetTimeSynCommand, m_acTimeSynCommand[%d]:\n",
         m_nPrimaryPortID);
  SYS_PrnDataBlock((const UINT8 *)m_acTimeSynCommand[m_nPrimaryPortID], 23, 20);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
} // CSWC_LAN::SetTimeSynCommand


/*******************************************************************************
Purpose
  Implements SWC time synchronization

Input
  None

Return
  None

History
    Name         Date          Remark
    ----         ----          ------
  Yu, Wei      22-Mar-2004   Initial revision
  Bryan Chong  10-Jun-2010   Update to use routine
                             RequestAndReceiveWithModbusCheck for polling
  Bryan Chong  22-Oct-2010   Update to use local time
*******************************************************************************/
void CSWC_LAN::TimeSynchronization(void)
{
  char acRecevData[2048];
  struct timespec tTime;
  struct tm *tFormatTime;
  char acTemp[200];
  E_ERR_T rstatus = E_ERR_Success;
  CHAR acErrMsg[100] = {0};
  char cLog[1000],acTemp1[200];int nI;
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_TIMESYNC)
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_TIMESYNC)

  if((m_bTimeSynFlag == false) ||
     (m_anCommfd[m_nPrimaryPortID] == TYP_ERROR) ||
     (m_anLinkStatus[m_nPrimaryPortID] != LINKOK))
    return;


  clock_gettime(CLOCK_REALTIME, &tTime);    //Get current UTC time

  //20101022 BC
  tFormatTime = localtime(&tTime.tv_sec);      //Change format to tm

  // The PLC_TIME_SYN_CHK will make sure the specific time update only happens
  // once because the next time it happens will be 61 seconds later when the
  // hour and min conditions are not valid
  if((m_nSendTimeSynHour == tFormatTime->tm_hour) &&
     (m_nSendTimeSynMinute == tFormatTime->tm_min) &&
     (CheckTimeOut(PLC_TIME_SYN_CHK,61000)== true))
  {
    m_bTimeSynStartFlag = true;
    ActivateTimeOut(PLC_TIME_SYN_CHK);
  }

  //if((m_bTimeSynStartFlag == true) &&
    // (m_anLinkStatus[m_nPrimaryPortID] == LINKOK))

  if(m_bTimeSynStartFlag == false)
    return;

  //Delay m_nSWCBusyTimeout send time sync //040707 Yu Wei
  delay(m_nSWCBusyTimeout);

  SetTimeSynCommand(1);

  //time syn
  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN_TIMESYNC))
  rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec, E_SYS_TIME_Local);
  printf("[SWC] %d.%02d.%02d %02d:%02d:%02d.%03d "
         "CSWC_LAN::TimeSynchronization, fd %d tx 23 bytes:\n",
         (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
         pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
         pbdtime->tm_sec, tspec.tv_nsec/1000000,
         m_anCommfd[m_nPrimaryPortID]);
  SYS_PrnDataBlock((const UINT8 *)m_acTimeSynCommand[m_nPrimaryPortID],
                   23, 20);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))

//  pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_TimeSynchronization,
//    FLG_HDR_T_TimeStamp_DateAndTime,
//    FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//    "[SWC] CSWC_LAN::TimeSynchronization, fd %d tx 23 bytes:\n",
//    m_anCommfd[m_nPrimaryPortID]);
  for(nI=0; nI<23; nI++)
  sprintf(&acTemp1[nI*2], "%02X",(unsigned char)m_acTimeSynCommand[m_nPrimaryPortID][nI]);
  sprintf(cLog,  "[SWC] CSWC_LAN::TimeSynchronization, fd %d tx 23 bytes:%s\n",
		    m_anCommfd[m_nPrimaryPortID],acTemp1);
  g_pDebugLog->LogMessage(cLog);

//  pFLG_CtrlBlk->SendDataBlock(E_FLG_MSG_SWC_LAN_TimeSynchronization,
//    FLG_LOG_T_Debug, (UINT8 *)m_acTimeSynCommand[m_nPrimaryPortID],
//    23, 20);

  rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nPrimaryPortID],
    (UINT8 *)m_acTimeSynCommand[m_nPrimaryPortID], 23,
    acRecevData, (UINT32)PLC_MODBUS_WRITE_REPLY_SIZE, m_nReceiveTimeout,
    (UINT8)m_nRetryNumber, (UINT8)m_nExcepTimeSyncRetry,
    (UINT8 *)&m_nFastExcepCounter);

  switch(rstatus)
  {
    case E_ERR_Success:
    case E_ERR_SWC_ModbusExceptionError:
    	if(rstatus==E_ERR_Success)
       		m_nFastExcepCounter = 0;
    	else
    		m_nFastExcepCounter++;
      m_bTimeSynStartFlag = false;

      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_TIMESYNC)
      rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                           E_SYS_TIME_Local);
      printf("[SWC] %d.%02d.%02d %02d:%02d:%02d.%03d "
             "CSWC_LAN::TimeSynchronization, fd %d rx %d bytes:\n",
             (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
             pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
             pbdtime->tm_sec, tspec.tv_nsec/1000000,
             m_anCommfd[m_nPrimaryPortID], PLC_MODBUS_WRITE_REPLY_SIZE);
      SYS_PrnDataBlock((const UINT8 *)acRecevData,
                       PLC_MODBUS_WRITE_REPLY_SIZE, 20);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN_TIMESYNC))
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_TimeSynchronization,
//        FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//        "[SWC] CSWC_LAN::TimeSynchronization, fd %d rx %d bytes:\n",
//        m_anCommfd[m_nPrimaryPortID], PLC_MODBUS_WRITE_REPLY_SIZE);

      for(nI=0; nI<PLC_MODBUS_WRITE_REPLY_SIZE; nI++)
      sprintf(&acTemp1[nI*2], "%02X",(unsigned char)acRecevData[nI]);
      sprintf(cLog, "[SWC] CSWC_LAN::TimeSynchronization, fd %d rx %d bytes:%s\n",
    	        m_anCommfd[m_nPrimaryPortID], PLC_MODBUS_WRITE_REPLY_SIZE,acTemp1);
      g_pDebugLog->LogMessage(cLog);

//      pFLG_CtrlBlk->SendDataBlock(E_FLG_MSG_SWC_LAN_TimeSynchronization,
//        FLG_LOG_T_Debug, (UINT8 *)acRecevData, PLC_MODBUS_WRITE_REPLY_SIZE,
//        20);
      break;

    case E_ERR_SWC_ExceedRetryLimit:
//      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
//      if(m_anCommfd[m_nPrimaryPortID] != TYP_ERROR)
//        close(m_anCommfd[m_nPrimaryPortID]);
//      m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
//
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_TIMERPOLLING)
//      printf("[SWC] CSWC_LAN::TimeSynchronization, %s exceed retry limit. "
//             "Set link = LINKERROR \n",
//             m_acSWCName, m_nRetryNumber);
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
//      break;

//    case E_ERR_SWC_ExceedExceptionRetryLimit:
//      m_bTimeSynStartFlag = false;
//      m_nTimeSyncExcepCounter = 0;
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_TIMESYNC)
//      printf("[SWC] CSWC_LAN::TimeSynchronization, exceed exception retry "
//             "limit, %d/%d\n",
//             m_nTimeSyncExcepCounter, m_nExcepTimeSyncRetry);
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
//      break;

    case E_ERR_LNC_SelectReadTimeOut:
    case E_ERR_LNC_FailToWriteSocket:
    case E_ERR_SWC_NoResponseByte:
    case E_ERR_LNC_FailToConnectStreamSocket:
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_TIMESYNC)
      printf("[SWC] CSWC_LAN::TimeSynchronization, select read timeout link "
             "down. Close socket fd %d\n",
             m_anCommfd[m_nPrimaryPortID]);
      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
      ERR_GetMsgString(rstatus, acErrMsg);
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_TimeSynchronization,
//        FLG_HDR_T_TimeStamp_DateAndTime,
//        FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//        "[SWC] CSWC_LAN::TimeSynchronization, %s link down, close fd %d.\n",
//        m_acSWCName, m_anCommfd[m_nPrimaryPortID], acErrMsg);

      sprintf(cLog, "[SWC] CSWC_LAN::TimeSynchronization, %s link down, close fd %d.\n",
    	        m_acSWCName, m_anCommfd[m_nPrimaryPortID], acErrMsg);
      g_pDebugLog->LogMessage(cLog);

      close(m_anCommfd[m_nPrimaryPortID]);
      m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
      m_bTimeSynStartFlag = false;
      ActivateTimeOut(LinkDownMoreThanOneDay);

      //Write event log.
      sprintf(acTemp,"%s TimeSynchronization Primary LAN link(%d) down.",
              m_acSWCName, m_nPrimaryPortID);
      g_pEventLog->LogMessage(acTemp);
      break;

    default:
      #ifdef CFG_PRN_ERR
      ERR_GetMsgString(rstatus, acErrMsg);
      printf("ERR  [SWC] CSWC_LAN::TimeSynchronization, unhandled error, "
             "%s\n", acErrMsg);
      #endif //CFG_PRN_ERR
      break;
  }// switch(rstatus)
} // CSWC_LAN::TimeSynchronization
/*******************************************************************************
Purpose
  Send "KeepAlive" signal to SWC

Input
  None

Return
  None

History
    Name         Date          Remark
    ----         ----          ------
  Yu, Wei      22-Mar-2004   Initial revision
  Bryan Chong  02-Jun-2011   Update to use routine
                             RequestAndReceiveWithModbusCheck for polling

*******************************************************************************/
// PLC does not use Keep Alive message
void CSWC_LAN::KeepAlive(void)
{
/*  E_ERR_T rstatus = E_ERR_Success;
  INT32 nI;
  CHAR acRecevData[2048];
  char acTemp[200];
  INT32 excepcnt;
  CHAR acErrMsg[100] = {0};

  if((m_anLinkStatus[m_nPrimaryPortID] == LINKERROR) ||
     (m_anCommfd[m_nPrimaryPortID] == TYP_ERROR) ||
     (m_bKeepAliveFlag == false) ||
     (CheckTimeOut(KEEPALIVE_CMD, m_nKeepAliveInterval) == false))
    return;

  ActivateTimeOut(KEEPALIVE_CMD);

  for(nI=0; nI<m_nTotalLinkNumber; nI++)
  {
    if(m_anLinkStatus[nI] != LINKOK)
      continue;

    *(unsigned short *)&m_acKeepAliveCommand[nI][7] =
       STR_ByteSwap16Bit(m_unKeepAliveCmdWord);

    rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nPrimaryPortID],
      (UINT8 *)m_acKeepAliveCommand[m_nPrimaryPortID],
      m_nKeepAliveCMDLength, acRecevData, 8, m_nReceiveTimeout,
      (UINT8)m_nRetryNumber, (UINT8)m_nExcepRetry, (UINT8 *)&excepcnt);

    switch(rstatus)
    {
      case E_ERR_Success:
        break;

      case E_ERR_SWC_ExceedRetryLimit:
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_KEEPALIVE)
        printf("[SWC] CSWC_LAN::KeepAlive, %s exceed retry limit. "
               "Set link = LINKERROR \n",
               m_acSWCName, m_nRetryNumber);
        #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
        break;

      case E_ERR_SWC_ExceedExceptionRetryLimit:
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_KEEPALIVE)
        printf("[SWC] CSWC_LAN::KeepAlive, %s exceed exception retry "
               "limit, %d/%d\n",
               m_acSWCName, m_nTimerExcepCounter, m_nExcepRetry);
        #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
        break;

      case E_ERR_LNC_SelectReadTimeOut:
      case E_ERR_LNC_FailToWriteSocket:
      case E_ERR_SWC_NoResponseByte:
      case E_ERR_LNC_FailToConnectStreamSocket:
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_KEEPALIVE)
        printf("[SWC] CSWC_LAN::KeepAlive, %s select read timeout link "
               "down. Close socket fd %d\n",
               m_acSWCName, m_anCommfd[m_nPrimaryPortID]);
        #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
        ERR_GetMsgString(rstatus, acErrMsg);
        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_KeepAlivePolling,
          FLG_HDR_T_TimeStamp_DateAndTime,
          FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
          "[SWC] CSWC_LAN::KeepAlive, %s link down, close fd %d. %s\n",
          m_acSWCName, m_anCommfd[m_nPrimaryPortID], acErrMsg);

        close(m_anCommfd[m_nPrimaryPortID]);
        m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
        m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
        ActivateTimeOut(LinkDownMoreThanOneDay);

        //Write event log.
        sprintf(acTemp,"%s KeepAlive Primary LAN link %d is down.",
                m_acSWCName, m_nPrimaryPortID);
        g_pEventLog->LogMessage(acTemp);
        break;

      default:
        #ifdef CFG_PRN_ERR
        ERR_GetMsgString(rstatus, acErrMsg);
        printf("ERR  [SWC] CSWC_LAN::KeepAlive, %s unhandled error, %s\n",
               m_acSWCName, acErrMsg);
        #endif //CFG_PRN_ERR
        break;
    }// switch(rstatus)
  }

  m_unKeepAliveCmdWord = 0;  //Reset server status.
*/
} // KeepAlive

/*******************************************************************************
Purpose
  Checks multidrop link SWC link status. When primary RTU's SWC link down,
  standby RTU will check link status.

Input
  None

Return
  None

History
    Name         Date          Remark
    ----         ----          ------
  Yu, Wei      22-Mar-2004   Initial revision
  Bryan Chong  02-Jun-2011   Update to use routine
                              RequestAndReceiveWithModbusCheck for polling
  Bryan Chong  19-Apr-2012   Replace polling retry variable, m_nRetryNumber,
                              with constant SWC_LINKCHK_RETRY to reduce
                              role switching time
*******************************************************************************/
void CSWC_LAN::LinkCheck(void)
{
  E_ERR_T rstatus = E_ERR_Success;
  char acRecevData[2048];
  tRMM_MSG tRMMMessageSend;
  //char acTemp[128] = {0};
 // UINT8 ucexceptionCnt = 0;
  CHAR acErrMsg[100] = {0};

  if(m_bLinkCheckFlag == false)
    return;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_STBYLINKCHK)
  printf("[SWC] CSWC_LAN::LinkCheck, perform link check\n");
  #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_STBYLINKCHK)

  rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nPrimaryPortID],
      (UINT8 *)m_acStatusCommand[m_nPrimaryPortID], sizeof(MB_PLC_RQ_T),
      acRecevData, (UINT32)m_nStatusPollingRecevLen, m_nReceiveTimeout,
      (UINT8)SWC_LINKCHK_RETRY, (UINT8)m_nExcepRetry, (UINT8 *)&m_nFastExcepCounter);

  switch(rstatus)
  {
    case E_ERR_Success:
    case E_ERR_SWC_ModbusExceptionError:
    	if(rstatus==E_ERR_Success)
       		m_nFastExcepCounter = 0;
    	else
    		m_nFastExcepCounter++;
      m_anLinkStatus[m_nPrimaryPortID] = LINKOK;

//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_StbyLinkCheck,
//        FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//        "[SWC] CSWC_LAN::LinkCheck, %s fd %d rx %d bytes:\n",
//        m_acSWCName, m_anCommfd[m_nPrimaryPortID], m_nStatusPollingRecevLen);
//
//      pFLG_CtrlBlk->SendDataBlock(E_FLG_MSG_SWC_LAN_StbyLinkCheck,
//        FLG_LOG_T_Debug, (UINT8 *)acRecevData, m_nStatusPollingRecevLen, 20);
      break;

//    case E_ERR_SWC_ExceedRetryLimit:
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_STBYLINKCHK)
//      printf("[SWC] CSWC_LAN::LinkCheck, %s exceed retry limit. "
//             "Set link = LINKERROR \n",
//             m_acSWCName, SWC_LINKCHK_RETRY);
//      #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_STBYLINKCHK)
//      break;

//    case E_ERR_SWC_ExceedExceptionRetryLimit:
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_STBYLINKCHK)
//      printf("[SWC] CSWC_LAN::LinkCheck, exceed exception retry "
//             "limit, %d/%d\n", ucexceptionCnt, m_nExcepRetry);
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
//      break;

    case E_ERR_SWC_ExceedRetryLimit:
    case E_ERR_LNC_SelectReadTimeOut:
    case E_ERR_LNC_FailToWriteSocket:
//    case E_ERR_SWC_NoResponseByte:
//    case E_ERR_LNC_FailToConnectStreamSocket:
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_STBYLINKCHK)
//      printf("[SWC] CSWC_LAN::LinkCheck, %s select read timeout link "
//             "down. Close socket fd %d\n",
//             m_acSWCName, m_anCommfd[m_nPrimaryPortID]);
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
//      ERR_GetMsgString(rstatus, acErrMsg);
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_StbyLinkCheck,
//        FLG_HDR_T_TimeStamp_DateAndTime,
//        FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//        "[SWC] CSWC_LAN::LinkCheck, %s standby link down, close fd %d. %s\n",
//        m_acSWCName, m_anCommfd[m_nPrimaryPortID], acErrMsg);
//
//      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
//      close(m_anCommfd[m_nPrimaryPortID]);
//      m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
//      break;

    case E_ERR_SWC_InvalidFileDescriptor:
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_STBYLINKCHK)
//      ERR_GetMsgString(rstatus, acErrMsg);
//      printf("[SWC] CSWC_LAN::LinkCheck, %s, err %s\n",
//             m_acSWCName, acErrMsg);
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN_STBYLINKCHK))
//      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
//      close(m_anCommfd[m_nPrimaryPortID]);
//      m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
//      break;

    default:
      ERR_GetMsgString(rstatus, acErrMsg);
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_STBYLINKCHK)
      printf("[SWC] CSWC_LAN::LinkCheck, %s unhandled error, %s\n",
             m_acSWCName, acErrMsg);
      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN_STBYLINKCHK))
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_StbyLinkCheck,
//        FLG_HDR_T_TimeStamp_DateAndTime,
//        FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//        "[SWC] CSWC_LAN::LinkCheck, %s check error, %s\n",
//        m_acSWCName, acErrMsg);

      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
      close(m_anCommfd[m_nPrimaryPortID]);
      m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
      break;
  }// switch(rstatus)

  //Get SWC communication status.
  m_unCurrentLinkStatus = GetLinkStatus();
  printf(" Number 3 m_unCurrentLinkStatus  =%02x \n",m_unCurrentLinkStatus);
  //Send link check completed message to RMM
  tRMMMessageSend.nDeviceID = m_nSWCID;
  tRMMMessageSend.nMessageID = SWC_MESSAGE_REPLY_LINK_CHECK;
  tRMMMessageSend.unStatus = (m_unCurrentLinkStatus);
  g_pRTUStatusTable->SendMsg(tRMMMessageSend);
} // LinkCheck


/*******************************************************************************
Purpose
  Construct the read command header for modbus packet. The header consists of
  2-byte-transaction-number, 2-byte-protocol-id, and 2-byte-data-length,
  1-byte-slave-address, 1-byte-function-code, and 2-byte-modbus-start-address
  in sequence.

Input
  acCommand      [inout] pointer to a buffer
  cSlaveAdr      [in] slave address
  nCMDLenth      [in] data length in byte
  cCommandCode   [in] function code
  tAddress       [in] modbus start address

Return
  None

History
    Name         Date          Remark
    ----         ----          ------
  Yu, Wei     22-Mar-2004   Initial revision

*******************************************************************************/
void CSWC_LAN::SetReadCommand(char *acCommand, char cSlaveAdr, int nCMDLenth ,
                              char cCommandCode, tModbusAddress tAddress )
{
  for (int nI=0;nI<5;nI++)
  {
    acCommand[nI] = 0;
  }
  acCommand[5] = 0x06; //command lenth
  acCommand[6] = cSlaveAdr;//cSlaveAdr;
  acCommand[7] = cCommandCode;//cCommandCode;
  //start address High
  acCommand[8] =  (unsigned char )(tAddress.unStart >> 8) ;
  //start address Low
  acCommand[9] =  (unsigned char )(tAddress.unStart & 0x00FF);
  *(unsigned short *)&acCommand[10] = STR_ByteSwap16Bit(tAddress.unLength);
  #ifdef CFG_DEBUG_MSG
  printf("[SWC] CSWC_LAN::SetReadCommand, command code = 0x%03x, slave addr = "
         "0x%04x(%d), address length = 0x%04x(%d) \n",
    cCommandCode, cSlaveAdr, cSlaveAdr, tAddress.unLength, tAddress.unLength);
  #endif  // CFG_DEBUG_MSG
}// SetReadCommand

/*******************************************************************************
Purpose
  Construct the write command header for modbus packet. The header consists of
  2-byte-transaction-number, 2-byte-protocol-id, and 2-byte-data-length,
  1-byte-slave-address, 1-byte-function-code, and 1-byte-modbus-data-length
  in sequence.

Input
  acCommand      [inout] pointer to a buffer
  aunWriteData   Reserved
  cSlaveAdr      [in] slave address
  cCommandCode   [in] function code
  tAddress       [in] modbus start address

Return
  OK    Success

History
    Name         Date          Remark
    ----         ----          ------
  Yu, Wei     22-Mar-2004   Initial revision

*******************************************************************************/
int CSWC_LAN::SetWriteCommand(char *acCommand, char *aunWriteData,
                              char cSlaveAdr, char cCommandCode,
                              tModbusAddress tAddress)
{

  unsigned char ucDataLength = tAddress.unLength;

  for (int nI=0;nI<5;nI++)
  {
    acCommand[nI] = 0;
  }
//  acCommand[5]need to check the data lenth
  acCommand[6] = cSlaveAdr;
  acCommand[7] = cCommandCode;

  //start address High
  acCommand[8] = (unsigned char )(tAddress.unStart >> 8);
  //start address Low
  acCommand[9] = (unsigned char )(tAddress.unStart & 0x00FF);

  if(cCommandCode == 0x10)      //Write word.
  {
    acCommand[10] = 0x00;
      acCommand[11] = ucDataLength;
    acCommand[12] = ucDataLength * 2;
  }
//  else if(cCommandCode == 0x0F)    //Write bit.
//  {
//    if((ucDataLength & 0x07) ==0)
//      ucDataLength >>= 3;      //divid 8;
//    else
//    {
//      ucDataLength >>= 3;      //divid 8;
//      ucDataLength ++;
//    }
//
//    acCommand[10] = 0x00;//bit  Number high
//      acCommand[11] = ucDataLength; //bit Number low
//    acCommand[12] = ucDataLength * 2;
//  }

  return OK;
} // CSWC_LAN::SetWriteCommand

/*******************************************************************************
Purpose
  This is the process that performs tasks which include setting SWC state flag,
  primary link checking, fast, slow, and timer pollings; timer
  synchronization, link checking or link monitoring, check for server command,
  message queue, and link status change. Watchdog will be reset while going
  through the routine.

Parameters
  None

Return
  None

History
    Name         Date          Remark
    ----         ----          ------
  Yu, Wei      22-Mar-2004  Initial revision
  Bryan Chong  16-Jun-2011  Add the log message when watchdog fail to restart

*******************************************************************************/
void CSWC_LAN::MainProcess(void)
{
  #if CFG_SWC_LAN_ENABLE_WATCHDOG
  if(wdStart(SWC_main_timer_id, 30 , 0 , 0 , 0)<0)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SWC] CSWC_LAN::MainProcess, watch dog start fail\n");
    #endif // CFG_PRN_ERR
    //20110616 BC
    g_pEventLog->LogMessage((CHAR *)
       "ERR  [RMM] CSWC_LAN::MainProcess, fail to restart watchdog\n");
  }
  #endif // CFG_SWC_LAN_ENABLE_WATCHDOG
  SetStateFlag();  //Set SWC state .

  #if ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)
  MDT_PrnToLog(CFG_MDT_CMM_LOG, m_acSWCName,
               (CHAR *)"[MT] CSWC_LAN::MainProcess, Path - 1\n");
  #endif // ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)

  if((m_nSWCCurrentStateFlag == STATEFLAG_PRIMARY) &&
     (m_bPollingEnable == true))
  {
    //During standby RTU check link status, stop polling.//tong
    if(m_bLinkCheckFlag == false && m_bCheckingBeforeSwitch == false)
    {
      #if ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)
      MDT_PrnToLog(CFG_MDT_CMM_LOG, m_acSWCName,
                   (CHAR *)"[MT] CSWC_LAN::MainProcess, Path - 2\n");
      #endif // ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)

      PrimaryLinkCheck();    //Check link status first.  //040813 Yu Wei
      FastPolling();
      SlowPolling();
      TimerPolling();
      TimeSynchronization();
//      KeepAlive();      //Send keep alive command.  //040812 Yu Wei
      PrimaryLinkCheck(); // In case of exceed retry count, try renew fd
                          // before declare link status
    }
  }
  //Link inhibit, don't monitor and check link. //040430 Yu Wei
  else if((m_nSWCCurrentStateFlag == STATEFLAG_STANDBY) &&
          (m_bPollingEnable == true))
  {
    //Standby RTU check link status.if primary is link down
    if(m_bLinkCheckFlag == true)
    {
      #if ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)
      MDT_PrnToLog(CFG_MDT_CMM_LOG, m_acSWCName,
                   (CHAR *)"[MT] CSWC_LAN::MainProcess, Path - 3\n");
      delay(100);
      #endif // ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)
      LinkCheck();
      m_bLinkCheckFlag = false;
    }
    else
    {
      #if ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)
      MDT_PrnToLog(CFG_MDT_CMM_LOG, m_acSWCName,
                   (CHAR *)"[MT] CSWC_LAN::MainProcess, Path - 4\n");
      #endif // ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)

      //Monitor link status  and  get PLC status
      LinkMonitor();//To test if this function causing memory leak
    }
  }

  #if ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)
  MDT_PrnToLog(CFG_MDT_CMM_LOG, m_acSWCName,
               (CHAR *)"[MT] CSWC_LAN::MainProcess, Path - 5\n");
  #endif // ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)

  //Process server specific command.
  CheckServerCommand();

  //Check message queue.
  CheckMessageQ();

  //Check link status.
  CheckLinkStatusChange();

  if(SWC_main_timer_id != NULL)
    wdCancel(SWC_main_timer_id);

  #if ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)
  MDT_PrnToLog(CFG_MDT_CMM_LOG, m_acSWCName,
               (CHAR *)"[MT] CSWC_LAN::MainProcess, Path - 7\n");
  #endif // ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)

} // CSWC_LAN::MainProcess

/*******************************************************************************
Purpose
  This routine will get link status and update RMM using message queue.

Input
  None

Return
  None

History
    Name         Date          Remark
    ----         ----          ------
  Yu, Wei     22-Mar-2004   Initial revision

*******************************************************************************/
void CSWC_LAN::CheckLinkStatusChange(void)
{
  unsigned short nLinkStatus;
  tRMM_MSG tRMMMessageSend;
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN)
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN)

  if(m_nSWCCurrentStateFlag != STATEFLAG_INITIALIZATION)
  {
    #if ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)
    MDT_PrnToLog(CFG_MDT_CMM_LOG, m_acSWCName,
                 (CHAR *)"[MT] CSWC_LAN::CheckLinkStatusChange, Path - 6\n");
    #endif // ((defined CFG_MODULE_TEST) && CFG_MDT_SWC_LAN)

    nLinkStatus = GetLinkStatus();

    if(nLinkStatus != m_unCurrentLinkStatus)
    {
      //Link down   1000 0000 0000 0000
      if(((m_unCurrentLinkStatus & VALIDAION_BIT) == 0) &&

         ((nLinkStatus & VALIDAION_BIT) != 0))        //Link resume
      {
         m_bTimeSynStartFlag = true;    //Link resume, sync time.
      }
      m_unCurrentLinkStatus = nLinkStatus;
     // printf(" Number 2 m_unCurrentLinkStatus  =%02x \n",m_unCurrentLinkStatus);
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN)
      printf("[SWC] CSWC_LAN::CheckLinkStatusChange, %s\n"
             "  current status = 0x%04x, fd %d\n",
             SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
             STR_ByteSwap16Bit(m_unCurrentLinkStatus),
             m_anCommfd[m_nPrimaryPortID]);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN)
      tRMMMessageSend.nDeviceID = m_nSWCID;
      tRMMMessageSend.nMessageID = SWC_LINK_STATUS_CHANGE;
      tRMMMessageSend.unStatus = m_unCurrentLinkStatus;
      g_pRTUStatusTable->SendMsg(tRMMMessageSend);
    }
  }
} // CSWC_LAN::CheckLinkStatusChange
/*******************************************************************************
Purpose
  Get LAN link status when RTU is in standby mode.

Input/Output
  None

Return
  None

History
  Name         Date          Remark
  ----         ----          ------
  Bryan Chong  11-Nov-2009   Increment ModBus transaction number
  Bryan Chong  09-Dec-2009   Update quantity of register in modbus packet as
                             we only use register length for link monitor
  Bryan Chong  08-Jun-2010   Update to use routine
                             RequestAndReceiveWithModbusCheck for polling

*******************************************************************************/
void CSWC_LAN::LinkMonitor(void)
{
  E_ERR_T rstatus = E_ERR_Success;
  char acRecevData[2048];
  char acTemp[200];
  //int nModbusReturn;
  MB_PLC_RQ_T *pmb_req;
  //UINT8 ucexceptionCnt = 0;

  #if ((defined CFG_PRN_ERR) && CFG_PRN_ERR_LNC)
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};
  #endif // ((defined CFG_PRN_ERR) && CFG_PRN_ERR_LNC)

  #if ((defined CFG_PRN_ERR) && \
       (CFG_PRN_ERR_LNC || CFG_PRN_WARN_SWC_LAN_LINKMON))
  CHAR acErrMsg[100] = {0};
  #endif // ((defined CFG_PRN_ERR) && CFG_PRN_ERR_LNC)

  if(m_anLinkStatus[m_nPrimaryPortID] != NOLINK &&
    (CheckTimeOut(STANDBYSTATUS,m_nStandbyPollingValue) == true))
  {
    ActivateTimeOut(STANDBYSTATUS);

    if((m_anCommfd[m_nPrimaryPortID] == TYP_ERROR) ||
       (m_anCommfd[m_nPrimaryPortID] == TYP_NULL))
    {
      rstatus = LNC_GetConnectFileDescriptor((const CHAR *)m_LanSWC_IP,
                  m_tLanSWC_SockID, SOCK_STREAM, 500,
                  (INT32 *)&m_anCommfd[m_nPrimaryPortID]);
      if(rstatus != E_ERR_Success)
      {
        #if ((defined CFG_PRN_ERR) && CFG_PRN_ERR_LNC)
        ERR_GetMsgString(rstatus, acErrMsg);
        printf("ERR  [SWC] CSWC_LAN::LinkMonitor, %s\n"
               "  connect to PLC %s:%d fail, %s\n",
               SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
               m_LanSWC_IP, m_tLanSWC_SockID, acErrMsg);
        #endif //((defined CFG_PRN_ERR) && #define CFG_PRN_ERR_LNC)
        m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
        m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
        return;
      }

    }

    pmb_req = (MB_PLC_RQ_T *)m_acStatusCommand[m_nPrimaryPortID];

    pmb_req->data_byte_length = STR_SWAP_16BIT_REG(6);
    // set link monitor modbus quantity of register to 1 register length
    pmb_req->data_word_length = STR_SWAP_16BIT_REG(MB_LINK_MONITOR_QTY_OF_REG);

    m_nStatusPollingRecevLen = sizeof(MB_PLC_RESP_T) +
                               (2*MB_LINK_MONITOR_QTY_OF_REG) - 1;

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_LINKMON)
    printf("[SWC] CSWC_LAN::LinkMonitor, fd %d connect to PLC %s:%d, "
           "tx %d bytes:\n",
           m_anCommfd[m_nPrimaryPortID], m_LanSWC_IP,  m_tLanSWC_SockID,
           sizeof(MB_PLC_RQ_T));
    SYS_PrnDataBlock((const UINT8 *)m_acStatusCommand[m_nPrimaryPortID],
                     sizeof(MB_PLC_RQ_T), 10);
    #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_LinkMonitor,
//      FLG_HDR_T_TimeStamp_DateAndTime,
//      FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//      "[SWC] CSWC_LAN::LinkMonitor, fd %d connect to PLC %s:%d, tx %d bytes:\n",
//      m_anCommfd[m_nPrimaryPortID], m_LanSWC_IP,  m_tLanSWC_SockID,
//      sizeof(MB_PLC_RQ_T));
//
//    pFLG_CtrlBlk->SendDataBlock(E_FLG_MSG_SWC_LAN_LinkMonitor, FLG_LOG_T_Debug,
//      (UINT8 *)m_acStatusCommand[m_nPrimaryPortID], sizeof(MB_PLC_RQ_T), 10);

    rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nPrimaryPortID],
        (UINT8 *)m_acStatusCommand[m_nPrimaryPortID], sizeof(MB_PLC_RQ_T),
        acRecevData, (UINT32)m_nStatusPollingRecevLen, m_nReceiveTimeout,
        (UINT8)m_nRetryNumber, (UINT8)m_nExcepRetry, (UINT8 *)&m_nFastExcepCounter);

    switch(rstatus)
    {
      case E_ERR_Success:
      case E_ERR_SWC_ModbusExceptionError:

      	if(rstatus==E_ERR_Success)
         		m_nFastExcepCounter = 0;
      	else
      		    m_nFastExcepCounter++;

          m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
        //delay(m_nReceiveTimeout*3);
        m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_LINKMON)
        printf("[SWC] CSWC_LAN::LinkMonitor, fd %d rx %d bytes:\n",
               m_anCommfd[m_nPrimaryPortID], m_nStatusPollingRecevLen);
        SYS_PrnDataBlock((const UINT8 *)acRecevData, m_nStatusPollingRecevLen,
                         10);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
//        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_LinkMonitor,
//          FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//          "[SWC] CSWC_LAN::LinkMonitor, fd %d rx %d bytes:\n",
//          m_anCommfd[m_nPrimaryPortID], m_nStatusPollingRecevLen);

//        pFLG_CtrlBlk->SendDataBlock(E_FLG_MSG_SWC_LAN_LinkMonitor,
//                                    FLG_LOG_T_Debug, (UINT8 *)acRecevData,
//                                    m_nStatusPollingRecevLen, 10);
        break;

      case E_ERR_SWC_ExceedRetryLimit:
        #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_LAN_LINKMON)
        printf("WARN [SWC] CSWC_LAN::LinkMonitor, exceed retry limit, "
               "link down. Close socket fd %d\n",
               m_anCommfd[m_nPrimaryPortID]);
        #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
        close(m_anCommfd[m_nPrimaryPortID]);
        m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
        m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;

        if((CheckTimeOut(m_anStandbyMonitorTimeout[m_nPrimaryPortID],
            m_nStandbyPollingFailedValue + m_nFastPollingValue)== true))
        {
          ActivateTimeOut(m_anStandbyMonitorTimeout[m_nPrimaryPortID]);
          if(m_anLinkStatus[m_nPrimaryPortID] == LINKOK)
          {
            sprintf(acTemp,"%s LAN SWC Link Monitor LAN link(%d) down.",
                    m_acSWCName, m_nPrimaryPortID);
            g_pEventLog->LogMessage(acTemp);        //Write event log.

            m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
          }
        }

        #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_LAN_LINKMON)
        printf("[SWC] CSWC_LAN::LinkMonitor, %s exceed retry limit. "
               "Set link = LINKERROR \n",
               m_acSWCName, m_nRetryNumber);
        #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
        break;

//      case E_ERR_SWC_ExceedExceptionRetryLimit:
//        #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_LAN_LINKMON)
//        printf("WARN [SWC] CSWC_LAN::LinkMonitor, exceed exception retry "
//               "limit\n");
//        #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
//        break;

//      case E_ERR_LNC_SelectReadTimeOut:
      case E_ERR_LNC_FailToWriteSocket:
      case E_ERR_LNC_FailToReadSocket:
      case E_ERR_LNC_SelectReadError:
//      case E_ERR_SWC_NoResponseByte:
//      case E_ERR_LNC_FailToConnectStreamSocket:
//        #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_LAN_LINKMON)
//        printf("WARN [SWC] CSWC_LAN::LinkMonitor, select read timeout link "
//               "down. Close socket fd %d\n",
//               m_anCommfd[m_nPrimaryPortID]);
//        #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
//        close(m_anCommfd[m_nPrimaryPortID]);
//        m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
//        m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
//        break;

      default:
        #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_LAN_LINKMON)
        ERR_GetMsgString(rstatus, acErrMsg);
        printf("[SWC] CSWC_LAN::LinkMonitor, %s unhandled error, %s\n",
               m_acSWCName, acErrMsg);
        #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
        close(m_anCommfd[m_nPrimaryPortID]);
        m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
        m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
        break;
    }// switch(rstatus)

    //delay(m_nReceiveTimeout*5);
    CheckLinkStatusChange();

  }
} // CSWC_LAN::LinkMonitor

/*******************************************************************************
Purpose:
  Copy SWC table to server table buffer.

Input/Output
  aunServerTable  -- [out] The buffer for server table.
  nTimeoutType  --   [in] Semiphone timeout.

Return
  OK    -- Transfer table successfully.
  ERROR  -- Semiphone timeout. The other module is accessing the table.

*******************************************************************************/
int CSWC_LAN::ReadSWCTable(unsigned short *aunServerTable, int nTimeoutType)
{
  int nI = 0;

  if(pthread_mutex_lock( & m_tAccessTableSemID) >= 0)
  {
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_READ_FASTP_TBL)
    if(g_tRTUStatus.nRTUStateFlag == STATEFLAG_PRIMARY)
      printf("[SWC] CSWC_LAN::ReadSWCTable, fast polling %d words, start from\n"
             " server addr word no %d\n 0x",
            m_nServerFastTableLen, m_nServerFastTableStartAddr);
    #endif // ((defined CFG_DEBUG_MSG) &&
           // (defined CFG_DEBUG_SWC_LAN_READ_FASTP_TBL))

    for(nI=0; nI<m_nServerFastTableLen; nI++)
    {
      aunServerTable[nI + m_nServerFastTableStartAddr] =
        STR_ByteSwap16Bit(m_aunSWCTableFast[nI]);
    }

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_READ_FASTP_TBL)
    printf("[SWC] CSWC_LAN::ReadSWCTable, %s read fast table %d bytes:\n",
           m_acSWCName, (m_nServerFastTableLen * 2));
    SYS_PrnDataBlock(
      (const UINT8 *) &aunServerTable[m_nServerFastTableStartAddr],
      (m_nServerFastTableLen * 2), 20);
    #endif // ((defined CFG_DEBUG_MSG) &&
           // (defined CFG_DEBUG_SWC_LAN_READ_FASTP_TBL))

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_READ_SLOWP_TBL)
    printf("\n[SWC] CSWC_LAN::ReadSWCTable, slow polling");
    #endif // CFG_DEBUG_MSG
    //Transfer slow table
    if(m_bSlowPollingFlag == true)
    {
      for(nI=0; nI<m_nServerSlowTableLen; nI++)
      {
        aunServerTable[nI + m_nServerSlowTableStartAddr] =
          STR_ByteSwap16Bit( m_aunSWCTableSlow[nI]);
      }

      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN_READ_SLOWP_TBL))
      printf("[SWC] CSWC_LAN::ReadSWCTable, %s read slow table %d bytes:\n",
           m_acSWCName, (m_nServerSlowTableLen * 2));
      SYS_PrnDataBlock((const UINT8 *) m_aunSWCTableSlow,
                       (m_nServerSlowTableLen * 2), 20);
      #endif // ((defined CFG_DEBUG_MSG) &&
               // (defined CFG_DEBUG_SWC_LAN_READ_SLOWP_TBL))
    }

    //Transfer Timer table
    if(m_bTimerPollingFlag == true)
    {
      #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_SWC_LAN_TIMERPOLLING)
      printf("[SWC] CSWC_LAN::ReadSWCTable, timer polling\n");
      #endif // CFG_DEBUG_MSG
      for(nI=0; nI<m_nServerTimerTableLen; nI++)
      {
        aunServerTable[nI + m_nServerTimerTableStartAddr] =
          STR_ByteSwap16Bit(m_aunSWCTableTimer[nI]);
      }
    }

    pthread_mutex_unlock( & m_tAccessTableSemID);
    return OK;
  } // if(pthread_mutex_lock( & m_tAccessTableSemID) >= 0)

  return ERROR;
} // ReadSWCTable
/*******************************************************************************
Purpose:
  Send specific command to SWC and get response.

Input/Output
  acReceData    -- [out] The buffer for replying from SWC.
  nReceDataLen  -- [in] Expect data size of SWC replying.

Return
  The size of response command.

History

    Name          Date          Remark
    ----          ----          ------
  Yu, Wei       04-Jul-2007   Initial revision
  Bryan Chong   18-Jun-2010   Fix write bit issue by implementing routine
                              RequestAndReceiveWithModbusCheck [PR43]
  Bryan Chong   18-Oct-2010   Move Specific Command log to event log file [PR80]
  Bryan Chong   11-Nov-2010   Translate write bit command to write word command
                              sent by server before forwarding to PLC.
                              Reply packet to server is based on write bit
                              command [C955 PR87]
  Bryan Chong   02-Dec-2010   Update to fix write bit address endianess for
                              acknowledgement packet to server
  Bryan Chong   28-Apr-2011   Update pwriteBitReq->total_words_to_write field
                              after receiving reply packet from LAN SWC.
                              [C955 PR88]

*******************************************************************************/
int CSWC_LAN::SpecificCommand(char *acReceData, int nReceDataLen)
{
  E_ERR_T rstatus = E_ERR_Success;
  UINT8 ucexceptioncnt = 0;
  #ifdef CFG_PRN_ERR
  CHAR acErrMsg[100] = {0};
  #endif // CFG_PRN_ERR
  char acTemp[1000],acTemp1[1000];
  MDB_LAN_WRITE_RQ_T *pwriteBitReq;
  UINT8 *pdata;
  UINT16 *pwdata;   // pointer to word data
  UINT16 total_bits_to_write;
  UINT16 packet_length;
  UINT16 org_byte_to_write;
  UINT16 start_bit_addr;
  UINT16 start_word_addr;
  UINT8  translatedCmd[2048] = {0};
  UINT16 bitcnt;
  UINT16 wordmask = 0;
  UINT16 remainingbit;
  UINT16 wordcnt = 0;
  UINT8 bitoffset = 0;
  int nI;
  char cLog[1000];
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)
  UINT16 bytecnt = 0;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)

  //When Link is inhibited, reply timeout to server directly. //040707 Yu Wei
  if( m_bPollingEnable == false )
    return (SpecificCommandSWCError(acReceData));


  pwriteBitReq = (MDB_LAN_WRITE_RQ_T *)m_acWriteCommand;

  if(pwriteBitReq->u_slave_addr_n_fn_code.sladdr_fn_comp.function_code == 0x0f)
  {
    // translate write bit command to write word command
    org_byte_to_write =
      pwriteBitReq->u_bytes_wdata.bytes_wdata_comp.total_bytes_to_write;

    memcpy(translatedCmd, m_acWriteCommand, (org_byte_to_write + 13));

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_SPCMD)
    printf("[SWC] CSWC_LAN::SpecificCommand, translate cmd %d bytes:\n",
           (org_byte_to_write + 13));
    SYS_PrnDataBlock((const UINT8 *) translatedCmd, (org_byte_to_write + 13),
                     20);
    #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_SPCMD))

    pwriteBitReq = (MDB_LAN_WRITE_RQ_T *)translatedCmd;

    // set pointer to the first byte of data
    pdata = &pwriteBitReq->u_bytes_wdata.bytes_wdata_comp.byteDataStart;
    pwdata = (UINT16 *)pdata;

    total_bits_to_write =
      STR_SWAP_16BIT_REG(pwriteBitReq->total_words_to_write);

    pwriteBitReq->total_words_to_write =
          STR_SWAP_16BIT_REG(pwriteBitReq->total_words_to_write);

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)
    printf("[SWC] CSWC_LAN::SpecificCommand, trans no: 0x%04x, prt no: 0x%04x\n  "
           "total bytes: 0x%04x, slv addr = 0x%02x, fn code: 0x%02x\n  "
           "start addr: 0x%04x, total words to write: 0x%04x\n  "
           "total bits to write: 0x%04x, total byte: 0x%02x\n",
           pwriteBitReq->transaction_number, pwriteBitReq->protocol_type,
           pwriteBitReq->total_data_byte,
           pwriteBitReq->u_slave_addr_n_fn_code.sladdr_fn_comp.slave_addr,
           pwriteBitReq->u_slave_addr_n_fn_code.sladdr_fn_comp.function_code,
           pwriteBitReq->start_addr, pwriteBitReq->total_words_to_write,
           total_bits_to_write, org_byte_to_write);
    printf("  Data: [0x%08x] 0x%02x\n", pdata, *pdata);
    SYS_PrnDataBlock((const UINT8 *) translatedCmd, (org_byte_to_write + 13),
                     20);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)

    // change to write word function code
    pwriteBitReq->u_slave_addr_n_fn_code.sladdr_fn_comp.function_code = 0x10;

    // remember original start address
    start_bit_addr = STR_SWAP_16BIT_REG(pwriteBitReq->start_addr);

    // translate start_bit_addr into bit address
    start_word_addr = (UINT16) (start_bit_addr/16);
    bitoffset = start_bit_addr % 16;

    // offset start address to 400 (0x190)
    pwriteBitReq->start_addr = SWC_LAN_WRITE_BIT_REGISTER_OFFSET +
                               start_word_addr;

    // convert number of bits to words
    remainingbit = total_bits_to_write % 16;


    if (remainingbit)
    {
      pwriteBitReq->total_words_to_write = (total_bits_to_write/16) + 1;

      for(bitcnt = 0; bitcnt < remainingbit; bitcnt++)
      {
        wordmask |= STR_SET_BIT_16BIT_REG(&wordmask, bitcnt, 1);
      }
    } else {
      pwriteBitReq->total_words_to_write = (total_bits_to_write/16);
    }

    pwriteBitReq->u_bytes_wdata.bytes_wdata_comp.total_bytes_to_write =
      pwriteBitReq->total_words_to_write * 2;

    pwriteBitReq->total_data_byte = 7 +
      pwriteBitReq->u_bytes_wdata.bytes_wdata_comp.total_bytes_to_write;

    packet_length = 6 + pwriteBitReq->total_data_byte;

    // mask out the last word bits if the last word has remaing bits
    if(remainingbit == 1)
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)
      printf("[SWC] CSWC_LAN::SpecificCommand, *pdata: 0x%08x, 0x%02x\n"
             "*pwdata: 0x%08x, 0x%04x\n",
             pdata, *pdata, pwdata, *pwdata);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)
      if(*pdata == 1)
      {
        pwdata[pwriteBitReq->total_words_to_write - 1] = 0;
        pwdata[pwriteBitReq->total_words_to_write - 1] =
          STR_SET_BIT_16BIT_REG(&pwdata[pwriteBitReq->total_words_to_write - 1],
                                bitoffset, 1);
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)
        printf("[SWC] CSWC_LAN::SpecificCommand, set bit %d to 1\n",
               bitoffset);
        #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)
      }else if (*pdata == 0){
        pwdata[pwriteBitReq->total_words_to_write - 1] =
          STR_SET_BIT_16BIT_REG(&pwdata[pwriteBitReq->total_words_to_write - 1],
                                bitoffset, 0);
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)
        printf("[SWC] CSWC_LAN::SpecificCommand, set bit %d to 0\n",
               bitoffset);
        #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)
      }else{
        #ifdef CFG_PRN_WARN
        printf("[SWC] CSWC_LAN::SpecificCommand, data byte %d not supported\n",
               *pdata);
        #endif //CFG_PRN_WARN
      }
    }else{
      #ifdef CFG_PRN_WARN
      printf("[SWC] CSWC_LAN::SpecificCommand, total bits to write, %d,"
             "not supported\n", remainingbit);
      #endif //CFG_PRN_WARN
    }

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)
    printf("[SWC] CSWC_LAN::SpecificCommand, pwdata = 0x%04x remaing bit: %d\n"
           "  wordmask = 0x%04x start addr: 0x%04x, *pdata = 0x%02x\n",
           *pwdata, remainingbit, wordmask, pwriteBitReq->start_addr, *pdata);
    for(bytecnt = 0; bytecnt < packet_length; bytecnt++)
    {
      printf(" %02x", translatedCmd[bytecnt]);
    }
    printf("\n");
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)

    // swap endianess for word data
    for(wordcnt = 0; wordcnt < pwriteBitReq->total_words_to_write; wordcnt++)
    {
      pwdata[wordcnt] = STR_SWAP_16BIT_REG(pwdata[wordcnt]);
    }

    // swap endianess for all manipulater words
    pwriteBitReq->start_addr = STR_SWAP_16BIT_REG(pwriteBitReq->start_addr);
    pwriteBitReq->total_words_to_write =
      STR_SWAP_16BIT_REG(pwriteBitReq->total_words_to_write);
    pwriteBitReq->total_data_byte =
      STR_SWAP_16BIT_REG(pwriteBitReq->total_data_byte);

    #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_SWC_LAN_SPCMD)
    printf("[SWC] CSWC_LAN::SpecificCommand, total word to write: 0x%04x "
           " total bytes to write: 0x%02x\n",
           pwriteBitReq->total_words_to_write,
           pwriteBitReq->u_bytes_wdata.bytes_wdata_comp.total_bytes_to_write);
    printf("[SWC] CSWC_LAN::SpecificCommand, pdata: 0x%08x, 0x%02x "
           "pwdata: 0x%08x, 0x%04x\n",
           pdata, *pdata, pwdata, *pwdata);

    for(cnt = 0; cnt < packet_length; cnt++)
    {
      printf(" %02x", translatedCmd[cnt]);
    }
    printf("\n");
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)


    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)
    printf("[SWC] CSWC_LAN::SpecificCommand, fd %d tx %d bytes to %s\n",
            m_anCommfd[m_nPrimaryPortID], packet_length, m_acSWCName);
    SYS_PrnDataBlock((const UINT8 *)translatedCmd, packet_length, 20);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)

//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_SpecificCommand,
//      FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "[SWC] CSWC_LAN::SpecificCommand, fd %d tx %d bytes to %s:\n",
//      m_anCommfd[m_nPrimaryPortID], packet_length, m_acSWCName);


//    pFLG_CtrlBlk->SendDataBlock(E_FLG_MSG_SWC_LAN_SpecificCommand,
//      FLG_LOG_T_Event, (UINT8 *)translatedCmd, packet_length, 20);

  	for(nI=0; nI<packet_length; nI++)
  	sprintf(&acTemp1[nI*2], "%02X",(unsigned char)translatedCmd[nI]);
  	sprintf(acTemp,"%s Send to device: %s",
  	                  m_acSWCName,(acTemp1+12));
  	g_pEventLog->LogMessage(acTemp);

//  	for(nI=0; nI<m_nSpecificCommandLen; nI++)
//  	sprintf(&acTemp1[nI*2], "%02X",(unsigned char)m_acWriteCommand[nI]);
//  	sprintf(acTemp,"%s Send to device: %s\n",
//  	                  m_acSWCName,acTemp1);
//  	g_pEventLog->LogMessage(acTemp);


    rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nPrimaryPortID],
      (UINT8 *)translatedCmd, packet_length, acReceData,
      PLC_MODBUS_WRITE_REPLY_SIZE, m_nReceiveTimeout, (UINT8)m_nRetryNumber,
      (UINT8)m_nExcepRetry, (UINT8 *)&ucexceptioncnt);

    // restore function code and start address
    pwriteBitReq = (MDB_LAN_WRITE_RQ_T *)acReceData;
    pwriteBitReq->u_slave_addr_n_fn_code.sladdr_fn_comp.function_code = 0x0f;

    // 20101202, 20110428 BC
    pwriteBitReq->start_addr = STR_SWAP_16BIT_REG(start_bit_addr);
    pwriteBitReq->total_words_to_write =
      STR_SWAP_16BIT_REG(total_bits_to_write);

  }else{
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)
    printf("[SWC] CSWC_LAN::SpecificCommand, fd %d tx %d bytes to %s:\n",
           m_anCommfd[m_nPrimaryPortID], m_nSpecificCommandLen, m_acSWCName);
    SYS_PrnDataBlock((const UINT8 *)m_acWriteCommand, m_nSpecificCommandLen,
                     10);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)

    // 20101018 BC
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_SpecificCommand,
//      FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "[SWC] CSWC_LAN::SpecificCommand, fd %d tx %d bytes to %s:\n",
//      m_anCommfd[m_nPrimaryPortID], m_nSpecificCommandLen, m_acSWCName);
//
//    pFLG_CtrlBlk->SendDataBlock(E_FLG_MSG_SWC_LAN_SpecificCommand,
//      FLG_LOG_T_Event, (UINT8 *)m_acWriteCommand, m_nSpecificCommandLen, 10);


  	for(nI=0; nI<m_nSpecificCommandLen; nI++)
  	sprintf(&acTemp1[nI*2], "%02X",(unsigned char)m_acWriteCommand[nI]);
  	sprintf(acTemp,"%s Send to device: %s",
  	                  m_acSWCName,(acTemp1+12));
  	g_pEventLog->LogMessage(acTemp);


    rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nPrimaryPortID],
      (UINT8 *)m_acWriteCommand, m_nSpecificCommandLen, acReceData,
      PLC_MODBUS_WRITE_REPLY_SIZE, m_nReceiveTimeout, (UINT8)m_nRetryNumber,
      (UINT8)m_nExcepRetry, (UINT8 *)&ucexceptioncnt);
  }

  switch(rstatus)
  {
    case E_ERR_Success:
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)
      printf("[SWC] CSWC_LAN::SpecificCommand, fd %d rx %d bytes fr %s:\n",
             m_anCommfd[m_nPrimaryPortID], PLC_MODBUS_WRITE_REPLY_SIZE,
             m_acSWCName);
      SYS_PrnDataBlock((const UINT8 *)acReceData, PLC_MODBUS_WRITE_REPLY_SIZE,
                       20);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)
      // 20101018 BC
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_LAN_SpecificCommand,
//        FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//        "[SWC] CSWC_LAN::SpecificCommand, fd %d rx %d bytes fr %s:\n",
//        m_anCommfd[m_nPrimaryPortID], PLC_MODBUS_WRITE_REPLY_SIZE,
//        m_acSWCName);
//
//      pFLG_CtrlBlk->SendDataBlock(E_FLG_MSG_SWC_LAN_SpecificCommand,
//        FLG_LOG_T_Event, (UINT8 *)acReceData, PLC_MODBUS_WRITE_REPLY_SIZE, 10);

    	for(nI=0; nI<PLC_MODBUS_WRITE_REPLY_SIZE; nI++)
    	sprintf(&acTemp1[nI*2], "%02X",(unsigned char)acReceData[nI]);
    	sprintf(acTemp,"%s Recv from device: %s",
    	                  m_acSWCName,(acTemp1+12));
    	g_pEventLog->LogMessage(acTemp);

      return PLC_MODBUS_WRITE_REPLY_SIZE;
      break;

    case E_ERR_SWC_ExceedRetryLimit:
      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
      close(m_anCommfd[m_nPrimaryPortID]);
      m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;

      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)
      printf("[SWC] CSWC_LAN::SpecificCommand, %s exceed retry limit. "
             "Set link = LINKERROR \n",
             m_acSWCName, m_nRetryNumber);
      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN))
      break;

//    case E_ERR_SWC_ExceedExceptionRetryLimit:
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)
//      printf("[SWC] CSWC_LAN::SpecificCommand, exceed exception retry "
//             "limit, %d/%d\n", m_nFastExcepCounter, m_nExcepRetry);
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_teLAN))
//      break;


//    case E_ERR_LNC_SelectReadTimeOut:
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)
//      printf("[SWC] CSWC_LAN::SpecificCommand, select read timeout link "
//             "down. Close socket fd %d\n",
//             m_anCommfd[m_nPrimaryPortID]);
//      #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_FASTPOLLING)
//      close(m_anCommfd[m_nPrimaryPortID]);
//      m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
//      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
//
//      //Write event log.
//
//      break;

    case E_ERR_LNC_FailToWriteSocket:
      if(m_anCommfd[m_nPrimaryPortID] != TYP_ERROR)
        close(m_anCommfd[m_nPrimaryPortID]);
      m_anCommfd[m_nPrimaryPortID] = TYP_ERROR;
      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
      break;

    default:
      #ifdef CFG_PRN_ERR
      ERR_GetMsgString(rstatus, acErrMsg);
      printf("ERR  [SWC] CSWC_LAN::SpecificCommand, %s unhandled error, %s\n",
             m_acSWCName, acErrMsg);
      #endif //CFG_PRN_ERR
      break;
  }// switch(rstatus)
  return SpecificCommandSWCError(acReceData);
} // CSWC_LAN::SpecificCommand

void CSWC_LAN::UpdateCommand(char *cData,int nCommandLen, char cSlaveAddress)
{

  for(int nI = 0 ; nI < 5 ;nI++)
    m_acWriteCommand[nI] = 0;

  m_acWriteCommand[5] = nCommandLen -2;
  m_acWriteCommand[6] = cSlaveAddress;
  //nCommandLen-2  delete last 2 bytes of CRC
  memcpy(&m_acWriteCommand[7] , &cData[1],nCommandLen-2);
  m_nSpecificCommandLen = nCommandLen + 4;
} // CSWC_LAN::UpdateCommand
/*******************************************************************************
Purpose:
  Process server command.

Input/Output
  acServerRecvBuffer  --  The buffer receiving from server. (in)
  nServerRecvNumber  --  The length of server command. (in)

History

    Name          Date          Remark
    ----          ----          ------
  Yu, Wei       04-Jul-2007   Initial revision
  Bryan Chong   18-Jun-2010   Update write bit message to debug log [PR43]

*******************************************************************************/
void CSWC_LAN::ServerCommand(char *acServerRecvBuffer, int nServerRecvNumber)
{
  int nSWCReplyLen = 0;
  char acSWCRecvBuffer[MODBUS_MAX_CMD_SIZE];
  int nI;
  char acTemp[2048],acTemp1[2048];
  unsigned short tmp = 0;


  #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_SWC_LAN_SPCMD)
  MDB_WRBIT_RQ_T *preqcmd;

  preqcmd = (MDB_WRBIT_RQ_T *)acServerRecvBuffer;
  printf("[SWC] CSWC_LAN::ServerComman, slv addr 0x%02x, fn code 0x%02x, "
         "coil addr 0x%04x, qty coils 0x%04x, byte cnt 0x%02x, start data "
         "0x%02x\n",
         preqcmd->u_slave_addr_n_fn_code.sladdr_fn_comp.slave_addr,
         preqcmd->u_slave_addr_n_fn_code.sladdr_fn_comp.function_code,
         preqcmd->coil_addr_start, preqcmd->qty_coils,
         preqcmd->u_bytes_bdata.bytes_bdata_comp.total_bytes_to_write,
         preqcmd->u_bytes_bdata.bytes_bdata_comp.byteDataStart);
  #endif // ((defined CFG_DEBUG_MSG_ && CFG_DEBUG_SWC_LAN_SPCMD)

  UpdateCommand(acServerRecvBuffer,nServerRecvNumber,
                m_acFastPollingCommand[m_nPrimaryPortID][6]);

  switch(acServerRecvBuffer[1])
  {
////   Yang Tao 17041. The control command only have 0x0F & 0x10
//  case 0x02:
//    /*tmp = STR_ByteSwap16Bit(*(unsigned short *)&acServerRecvBuffer[4]);
//    nSWCReplyLen = (tmp >>3);  //divid 8, convert bits to bytes
//    if((tmp & 0x07) != 0)    //remainder is not 0,
//      nSWCReplyLen ++;
//    nSWCReplyLen += 5;    //Get total received data number*/
//    break;
//  case 0x03:
//  case 0x04:
//    nSWCReplyLen = tmp*2 +5; //Get total received data number
//    break;

  case 0x0F:
  case 0x10:
    nSWCReplyLen = 8;
    break;

  default:      //040310 Yu Wei
    nSWCReplyLen = 0;
    break;
  }

  tmp = STR_ByteSwap16Bit(*(unsigned short *)&acServerRecvBuffer[2]);
  //Received keep alive command.    //040812 Yu Wei
  if((tmp == m_unSendKeepAliveAddr) && (m_bKeepAliveFlag == true))
  {
    tmp = STR_ByteSwap16Bit(*(unsigned short *)&acServerRecvBuffer[7]);
    m_unKeepAliveCmdWord = tmp;  //Update server status.

    *(unsigned short *)&acServerRecvBuffer[nSWCReplyLen-2] =
      STR_ByteSwap16Bit(ModbusCRC((unsigned char *)acServerRecvBuffer,nSWCReplyLen-2));

    m_pServerSocket->Send(acServerRecvBuffer,nSWCReplyLen,0);
    m_pServerSocket->Close(6);	//20151125 Su, must close also for LAN SWC
    //printf("server command 5\n");
    if(g_tDebugLogFlag.bServerCommand == true)
    {
      //Write command that send to server to debug log.
      for(nI=0; nI<nSWCReplyLen; nI++)
        sprintf(&acTemp1[nI*2],"%02X",acServerRecvBuffer[nI]);

      sprintf(acTemp,"%s ServerCommand Send to server (Directly): %s\n\n",
              m_acSWCName,acTemp1);
      g_pEventLog->LogMessage(acTemp);
    }
  }else{

    #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_SWC_SPCMD)
    printf("[SWC] CSWC_LAN::ServerCommand, rx cmd %d bytes:\n", nServerRecvNumber);
    SYS_PrnDataBlock((const UINT8 *) acServerRecvBuffer, nServerRecvNumber, 10);
    #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))

    //Send command to SWC and get response.
    nSWCReplyLen = SpecificCommand(acSWCRecvBuffer,nSWCReplyLen);

    if(nSWCReplyLen == PLC_MODBUS_WRITE_REPLY_SIZE)
    {
      nSWCReplyLen = 8;//switch Net Modebus to serial Modebus
    }

    memcpy(&acServerRecvBuffer[1] , &acSWCRecvBuffer[7] , 5);

    *(unsigned short *)&acServerRecvBuffer[nSWCReplyLen-2] =
      STR_ByteSwap16Bit(ModbusCRC((unsigned char *)acServerRecvBuffer,nSWCReplyLen-2));

    //Send command that receive from SWC to server.
    m_pServerSocket->Send(acServerRecvBuffer, nSWCReplyLen,0);

    for(nI=0; nI<nSWCReplyLen; nI++)
      sprintf(&acTemp1[nI*2], "%02X",(unsigned char)acServerRecvBuffer[nI]);
      sprintf(acTemp,"%s Send to server (from device): %s",
            m_acSWCName,acTemp1);
     g_pEventLog->LogMessage(acTemp);

    m_pServerSocket->Close(6);	//20151125 Su, must close also for LAN SWC

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)
    printf("[SWC] CSWC_LAN::ServerCommand, fd %d tx %d bytes to server:\n",
           m_anCommfd[m_nPrimaryPortID], nSWCReplyLen);
    SYS_PrnDataBlock((const UINT8 *)acServerRecvBuffer, nSWCReplyLen,
                     10);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_SPCMD)
  } // else for if((tmp == m_unSendKeepAliveAddr) && (m_bKeepAliveFlag == true))
} // CSWC_LAN::ServerCommand
/*******************************************************************************
Purpose:
  Set exception response command.

Output
  acReceData  -- The buffer for exception response command.

Return
  The size of command.
*******************************************************************************/
int CSWC_LAN::SpecificCommandSWCError(char *acReceData)
{
  acReceData[0] = m_acWriteCommand[6];//slave address
  acReceData[1] = m_acWriteCommand[7] |0x80; //reply exception
  acReceData[2] = 0x00;
  return 5;
} // CSWC_LAN::SpecificCommandSWCError
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  RequestAndReceiveWithModbusCheck

 DESCRIPTION

  This routine will send request to SWC, wait for response, and determine for
  modbus error upon receiving the response from SWC.

  Unlike Modbus from serial interface, Modbus LAN interface does not have
  modbus checking. We assume the integrity check has beed done at TCP packet
  level.

  The transaction number of the replying packet is similar to that of
  trasmitted earlier.

  Eg. tx 0x0001, rx 0x0001; tx 0x0002, rx 0x0002; ...

  If transaction number mismatch occur, m_usPacketTransactionNum will be
  adjusted to follow the transactiion number from received packet.
  Then receive one packet and drop it.

 CALLED BY

  CSWC_LAN::FastPolling
  CSWC_LAN::PrimaryLinkCheck
  CSWC_LAN::SlowPolling
  CSWC_LAN::TimerPolling

 CALLS

  SYS_GetCurrentTimeInString
  SYS_PrnDataBlock
  SYS_SetMutex
  STR_SWAP_16BIT_REG
  LNC_SendWithReply
  LNC_GetConnectFileDescriptor
  LNC_ReceiveMsg

 PARAMETER

  fd              [in] file descriptor
  ptxcmd          [in] pointer to transmit command, or request command, buffer
  txcmdsz         [in] length of transmit command
  prxbuff         [in] pointer to receiving buffer
  expectedRxSz    [in] expected length to receive
  timeout_ms      [in] time out in millisecond
  retry           [in] max number of polling retry.
  exceptionRetry  [in] max number of exception retry. Must be greater than
                       or equal to the number of poutexceptionRetryCnt
  poutexceptionRetryCnt  [out] current number of exception retry has elapsed


 RETURN

  E_ERR_Success                        the routine executed successfully
  E_ERR_SWC_ExceedRetryLimit           exceed retry limit
  E_ERR_SWC_ModbusExceptionError       modbus exception error
  E_ERR_SWC_ModbusCRCError             modbus CRC error
  E_ERR_SWC_ExceedExceptionRetryLimit  exceed exception retry limit
  E_ERR_LNC_FailToWriteSocket          fail to write socket
  E_ERR_LNC_FailToReadSocket           fail to read socket

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE           REMARKS

 Bryan Chong      03-Jun-2010    Initial revision
 Bryan Chong      08-Feb-2011    Change E_ERR_SER_SelectReadTimeOut to
                                 E_ERR_LNC_SelectReadTimeOut
 Bryan Chong      18-Mar-2011    Add transaction number checking for each
                                 replying packet. The transaction number for
                                 the replying packet is increment of 1
 Bryan Chong      15-Dec-2011    When DI data overlap to MI data, revert
                                 m_usPacketTransactionNum to rxtransno and
                                 receive the late packet and drop it.
                                 [C955,PR86]
 Bryan Chong      23-Dec-2011    update with mutex unlock before return with
                                 E_ERR_SWC_ExceedRetryLimit
 Bryan Chong      04-May-2012    Add case to handle E_ERR_LNC_FailToWriteSocket,
                                 and E_ERR_LNC_FailToReadSocket. This will
                                 allow retry duration when PLC is performing
                                 role switching [C955,PR114]
----------------------------------------------------------------------------*/
E_ERR_T CSWC_LAN::RequestAndReceiveWithModbusCheck(INT32 fd,
               UINT8 *ptxcmd,
               UINT8 txcmdsz,
               VOID *prxbuff,
               UINT32 expectedRxSz,
               INT32 timeout_ms,
               UINT8 retry,
               UINT8 exceptionRetry,
               UINT8 *poutexceptionRetryCnt)
{
  E_ERR_T rstatus = E_ERR_Success;
  UINT8 ucretrycnt = 0;
  //UINT8 ucexceptionretrycnt = 0;
  INT32 nactualRxSz;

  #if (((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_LAN) || \
       (defined CFG_PRN_ERR))
  CHAR acErrMsg[100] = {0};
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};
  #endif // (((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_LAN) ||
         //  (defined CFG_PRN_ERR))

  INT32 nmodbusrval = 0;
  UINT16 *ptxpacketnum;
  UINT16 rxtransno;
  UINT16 txtransno;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_RQ_TX)
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))

  ucretrycnt = 0;
  if(poutexceptionRetryCnt != TYP_NULL)
    *poutexceptionRetryCnt = 0;

  if(m_anCommfd[m_nPrimaryPortID] == TYP_ERROR)
    return E_ERR_SWC_InvalidFileDescriptor;

  // appending packet number before sending
  m_usPacketTransactionNum++;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_RQ_TX)
  printf("[SWC] CSWC_LAN::RequestAndReceiveWithModbusCheck, packet "
         "number 0x%04d\n", m_usPacketTransactionNum);
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_RQ_TX)

  memset((CHAR *)prxbuff, 0, expectedRxSz);
  ptxpacketnum = (UINT16 *)ptxcmd;

  *ptxpacketnum =
      (UINT16)STR_SWAP_16BIT_REG(m_usPacketTransactionNum);

  txtransno = m_usPacketTransactionNum;

  while((ucretrycnt++ < retry) && (nmodbusrval != MODBUS_MESSAGE_OK))
  {
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_RQ_TX)
    rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec, E_SYS_TIME_Local);
    printf("[SWC] %d.%02d.%02d %02d:%02d:%02d.%03d "
           "CSWC_LAN::RequestAndReceiveWithModbusCheck, retry = %d/%d, fd %d "
           "tx %d bytes:\n",
           (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
           pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
           pbdtime->tm_sec, tspec.tv_nsec/1000000,
           ucretrycnt, retry, fd, txcmdsz);
    SYS_PrnDataBlock((const UINT8 *)ptxcmd, txcmdsz, 20);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))

    // Send command and wait for reply
    rstatus = LNC_SendWithReply(fd, ptxcmd, txcmdsz, prxbuff, expectedRxSz,
                                &nactualRxSz, timeout_ms);
    switch(rstatus)
    {
      case E_ERR_Success:
        break; // E_ERR_Success

      case E_ERR_LNC_SelectReadTimeOut:
        if(ucretrycnt >= retry)
        {
          #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_LAN)
          printf("WARN [SWC] CSWC_LAN::RequestAndReceiveWithModbusCheck, %s\n"
                 "  exceed retry limit %d timeout=%d\n",
                 SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
                 retry,timeout_ms);
          #endif // CFG_PRN_ERR
          return E_ERR_SWC_ExceedRetryLimit;
        }

        break; // E_ERR_LNC_SelectReadTimeOut

      // 20120504, BC
      case E_ERR_LNC_FailToWriteSocket:
      case E_ERR_LNC_FailToReadSocket:
      case E_ERR_LNC_SelectReadError:
        #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_LAN)
        ERR_GetMsgString(rstatus, acErrMsg);
        printf("WARN [SWC] CSWC_LAN::RequestAndReceiveWithModbusCheck, %s\n"
               "  retry %d, %s\n",
               SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
               retry, acErrMsg);
        #endif // CFG_PRN_ERR

        rstatus = LNC_GetConnectFileDescriptor((const CHAR *)m_LanSWC_IP,
                  m_tLanSWC_SockID, SOCK_STREAM, 500,
                  (INT32 *)&m_anCommfd[m_nPrimaryPortID]);
        if(rstatus != E_ERR_Success)
        {
          #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_LAN)
          ERR_GetMsgString(rstatus, acErrMsg);
          printf("WARN [SWC] CSWC_LAN::RequestAndReceiveWithModbusCheck, %s\n"
                 "  renew fd fail, m_anCommfd[%d] = %d, %s\n",
                 SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
                 m_nPrimaryPortID, m_anCommfd[m_nPrimaryPortID],
                 acErrMsg);
          #endif // CFG_PRN_ERR
          close(fd);
          fd = TYP_ERROR;
          continue;
        }

        if((m_anCommfd[m_nPrimaryPortID] > 0) &&
           (fd != m_anCommfd[m_nPrimaryPortID]))
        {
          close(fd);
          fd = m_anCommfd[m_nPrimaryPortID];
          continue;
        }
        break; // E_ERR_LNC_FailToWriteSocket

      default:
        #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_LAN)
        ERR_GetMsgString(rstatus, acErrMsg);
        printf("WARN [SWC] CSWC_LAN::RequestAndReceiveWithModbusCheck, %s\n"
               "  retry %d unhandled rstatus %s\n",
               SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
               retry, acErrMsg);
        #endif // CFG_PRN_ERR
        break; // default
    } // switch(rstatus)

    // 20110318 BC
    // Add transaction number checking
    rxtransno = STR_SWAP_16BIT_REG(((UINT16 *)prxbuff)[0]);

    #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_SWC_LAN)
    printf("[SWC] CSWC_LAN::RequestAndReceiveWithModbusCheck, "
             "transaction number, tx 0x%04x vs rx 0x%04x\n",
             txtransno, rxtransno);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN)

    if((nactualRxSz > 0) && (txtransno != rxtransno))
    {
  #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_LAN)
      printf("ERR  [SWC] CSWC_LAN::RequestAndReceiveWithModbusCheck, %s "
             "invalid transaction number, tx 0x%04x vs rx 0x%04x, "
             "m_usPacketTransactionNum = 0x%04x, rx %d byte(s)\n",
             m_acSWCName, txtransno, rxtransno,
             m_usPacketTransactionNum, nactualRxSz);
      #endif // CFG_PRN_ERR

      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_RQ_TX)
      printf("ERR  [SWC] CSWC_LAN::RequestAndReceiveWithModbusCheck, %s "
             "Adjust m_usPacketTransactionNum to 0x%04x\n",
             m_acSWCName, rxtransno);
      SYS_PrnDataBlock((const UINT8 *) prxbuff, nactualRxSz, 20);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN)

      m_usPacketTransactionNum = txtransno = rxtransno;
      *ptxpacketnum =
      (UINT16)STR_SWAP_16BIT_REG(m_usPacketTransactionNum);

      // drop the late coming packet here
      rstatus = LNC_ReceiveMsg(fd, (VOID *)prxbuff, (INT32 *)&nactualRxSz,
                             expectedRxSz, timeout_ms);

    }// if((nactualRxSz > 0) && (txtransno != rxtransno))

    // Unlike Modbus serial, Modbus LAN packet is checked at TCP packet level,
    // hence no checking is required
    if(nactualRxSz > 0)
    {
      nmodbusrval = MODBUS_MESSAGE_OK;
    }

    if(nactualRxSz == 0)
      continue;

    switch(nmodbusrval)
    {
      case MODBUS_MESSAGE_OK:
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LAN_RQ_TX)
        printf("[SWC] CSWC_LAN::RequestAndReceiveWithModbusCheck, fd %d "
               "rx %d bytes:\n", fd, expectedRxSz);
        SYS_PrnDataBlock((const UINT8 *)prxbuff, expectedRxSz, 20);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
        *poutexceptionRetryCnt = 0;
        break;

      case MODBUS_ERROR_CRC:
        #ifdef CFG_PRN_WARN
        printf("WARN [SWC] CSWC_LAN::RequestAndReceiveWithModbusCheck, modbus "
               "CRC error\n");
        #endif // CFG_PRN_WARN
        rstatus = E_ERR_SWC_ModbusCRCError;
        break;

      case MODBUS_ERROR_EXCEPTION:
        #ifdef CFG_PRN_WARN
        printf("WARN [SWC] CSWC_LAN::RequestAndReceiveWithModbusCheck, modbus "
               "exception error\n");
        #endif // CFG_PRN_WARN
        if((poutexceptionRetryCnt == TYP_NULL) || (exceptionRetry == TYP_NULL))
        {
          #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LAN_RQ_TX))
          printf("[SWC] CSWC_LAN::RequestAndReceiveWithModbusCheck, not "
                 "handling modbus exception error.\n  Exception retry cnt not "
                 "defined\n");
          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
          continue;
        }

        if(*poutexceptionRetryCnt < exceptionRetry)
        {
          (*poutexceptionRetryCnt)++;
          rstatus = E_ERR_SWC_ModbusExceptionError;
        }
        else{
          rstatus = E_ERR_SWC_ExceedExceptionRetryLimit;
        }
        break;

      // no response byte found
      case 0:
        #ifdef CFG_PRN_WARN
        printf("WARN [SWC] CSWC_LAN::RequestAndReceiveWithModbusCheck, modbus "
               "return 0,\n"
               "  no response byte found, retry cnt %d/%d\n",
               ucretrycnt, retry);
        #endif // CFG_PRN_WARN
        rstatus = E_ERR_SWC_NoResponseByte;
        break;

      default:
        #ifdef CFG_PRN_WARN
        printf("WARN [SWC] CSWC_LAN::RequestAndReceiveWithModbusCheck, "
               "unhandle modbus error, 0x%04x\n", nmodbusrval);
        #endif // CFG_PRN_WARN
        rstatus = E_ERR_SWC_UnhandleModbusError;
    } // switch(nmodbusrval)

  }

  return rstatus;
} // CSWC::RequestAndReceiveWithModbusCheck
/*----------------------------------------------------------------------------

 PRIVATE ROUTINE

  Ping

 DESCRIPTION

  This routine will initiate ping process

 CALLED BY

  CSWC_LAN::CSWC_LAN

 CALLS

  none

 PARAMETER

  ip          [in] pointer to IP address

 RETURN

  -1          Error
  1           OK

 AUTHOR

  Inherited from orginal code, author unknown

 HISTORY

    NAME            DATE           REMARKS

 Bryan Chong      14-May-2012    Initial revision
----------------------------------------------------------------------------*/
static int Ping(char *ip)
{
   int     size = 0;
   int    sockfd = 0;
   int     datalen = 56;
   int     nsent = 0;
   int     sendLenth = 0;
//   int    nLan = 0 ;
   char    recvbuf[512]={0x00};
   char    controlbuf[512]={0x00};
   char    SendBuffer[512] = {0x00};
   char    MsgName[32] = {0x00};

   struct  msghdr   msg;
   struct  iovec     iov;
   struct  timeval   tval;
   struct  addrinfo   *addif  ;
   struct  icmp     *icmp ;

   pid_t IPpid;
   ssize_t n;

  //prepare address infomation
  IPpid = getpid() & 0xffff;

  //comment for test to reduce getaddrinfo 's time
  if(getaddrinfo (ip, NULL, 0, &addif)<0)
  {
    perror("Get address info Error");
    return -1;
  }

  //initialize socket
   sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
   if(sockfd < 0)
   {
     freeaddrinfo(addif);
     close(sockfd);
     return -1;
   }
   setuid(getuid());           /* don't need special permissions any more */
   size =  2048;           /* OK if setsockopt fails */
   setsockopt (sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof (size));
   setsockopt (sockfd, SOL_SOCKET, SO_SNDBUF, &size, sizeof (size));


  //send icmp
   icmp = (struct icmp *) SendBuffer;
   icmp->icmp_type = ICMP_ECHO;
   icmp->icmp_code = 0;
   icmp->icmp_id = IPpid;
   icmp->icmp_seq = nsent++;
   memset (icmp->icmp_data, 0xa5, datalen); /* fill with pattern */
   gettimeofday ((struct timeval *) icmp->icmp_data, NULL);
   sendLenth = 8 + datalen;           /* checksum ICMP header and data */
   icmp->icmp_cksum = 0;
   icmp->icmp_cksum = in_cksum ((u_short *) icmp, sendLenth);

   if(sendto(sockfd, SendBuffer, sendLenth, 0, addif->ai_addr,
             addif->ai_addrlen) < 0)
   {
       freeaddrinfo(addif);
       close(sockfd);
       return -1;
   }

  //receive the reply
  iov.iov_base = recvbuf;
  iov.iov_len = sizeof (recvbuf);
//     msg.msg_name = calloc (1, g_addinfo[nLan]->ai_addrlen);
   msg.msg_name = &MsgName;
   msg.msg_iov = &iov;
   msg.msg_iovlen = 1;
   msg.msg_control = controlbuf;

   struct sigevent event;
   uint64_t timeout;
   int continuecout = 0;

   timeout =1*100000000;
   event.sigev_notify = SIGEV_UNBLOCK;
   for (;;)
   {
     n = -1;
     msg.msg_namelen = addif->ai_addrlen;
     msg.msg_controllen = sizeof (controlbuf);
     TimerTimeout(CLOCK_REALTIME ,
      _NTO_TIMEOUT_SEND | _NTO_TIMEOUT_REPLY |_NTO_TIMEOUT_RECEIVE ,  &event ,
      &timeout , NULL);
     n = recvmsg (sockfd, &msg, 0);
     if (n < 0)
     {
       if (errno == EINTR)
       {
         delay(10);
         if(continuecout++ > 2)
         {
            freeaddrinfo(addif);
//          free(msg.msg_name);
            close(sockfd);
            return -1 ;
         }
         continue;
       } else {
         freeaddrinfo(addif);
//        free(msg.msg_name);
         close(sockfd);
        return -1 ;
       }
     }
     gettimeofday (&tval, NULL);//
     if(checkReply(recvbuf, n, &msg, &tval ,IPpid)< 0 )
     {
       freeaddrinfo(addif);

       close(sockfd);

       return -1 ;
     } else {
       freeaddrinfo(addif);

       close(sockfd);
       return 1;
     }
   }
   freeaddrinfo(addif);
//  free(addif);
   return 1;
}// Ping
/*----------------------------------------------------------------------------

 PRIVATE ROUTINE

  checkReply

 DESCRIPTION

  Check for Ping response

 CALLED BY

  Ping

 CALLS

  none

 PARAMETER

  ptr   [in] pointer to receiving buffer
  n     [in] size of the buffer
  msg   [in] reference to received message
  tval  [in] reference to time value
  IPpid [in] the process ID of the calling process


 RETURN

  -1          Error
  1           OK

 AUTHOR

  Inherited from orginal code, author unknown

 HISTORY

    NAME            DATE           REMARKS

 Bryan Chong      14-May-2012    Initial revision
----------------------------------------------------------------------------*/
static int checkReply(char *ptr, ssize_t len, struct msghdr *msg,
                      struct timeval *tvrecv ,pid_t IPpid)
{
  int         nHeadLen, icmplen;
  double      rtt;
  struct ip     *ip;
  struct icmp   *icmp;
  struct timeval *tvsend;


  ip = (struct ip *) ptr;      /* start of IP header */
  nHeadLen = ip->ip_hl << 2;      /* length of IP header */
  if (ip->ip_p != IPPROTO_ICMP)
    return -1;                  /* not ICMP */

  icmp = (struct icmp *) (ptr + nHeadLen);   /* start of ICMP header */
  if ( (icmplen = len - nHeadLen) < 8)
    return -1;                  /* malformed packet */

  if (icmp->icmp_type == ICMP_ECHOREPLY)
  {
     if (icmp->icmp_id != IPpid)
       return -1;                /* not a response to our ECHO_REQUEST */
     if (icmplen < 16)
       return -1;                /* not enough data to use */

     tvsend = (struct  timeval  *) icmp->icmp_data;
     tv_sub (tvrecv, tvsend);
     rtt = tvrecv->tv_sec * 1000.0 + tvrecv->tv_usec / 1000.0;
     return 1;
  }
  return 1;
}
/*----------------------------------------------------------------------------

 PRIVATE ROUTINE

  tv_sub

 DESCRIPTION

  Get the time difference based on end time and start time

 CALLED BY

  checkReply

 CALLS

  none

 PARAMETER

  out   [in] pointer to end time
  in    [in] pointer to start time

 RETURN

  None

 AUTHOR

  Inherited from orginal code, author unknown

 HISTORY

    NAME            DATE           REMARKS

 Bryan Chong      14-May-2012    Initial revision
----------------------------------------------------------------------------*/
static void tv_sub (struct timeval *out, struct timeval *in)
{
     if ((out->tv_usec -= in->tv_usec) < 0)
     {
         --out->tv_sec;
         out->tv_usec += 1000000;
     }
    out->tv_sec -= in->tv_sec;
}
/*----------------------------------------------------------------------------

 PRIVATE ROUTINE

  in_cksum

 DESCRIPTION

  Check for checksum based on start address and wor length

 CALLED BY

  Ping

 CALLS

  none

 PARAMETER

  addr  [in] pointer to start word address
  len   [in] word length


 RETURN

  checksum value

 AUTHOR

  Inherited from orginal code, author unknown

 HISTORY

    NAME            DATE           REMARKS

 Bryan Chong      14-May-2012    Initial revision
----------------------------------------------------------------------------*/
static UINT16 in_cksum (uint16_t * addr, int len)
{
  int     nleft = len;
  uint32_t sum = 0;
  uint16_t *w = addr;
  uint16_t answer = 0;


  while (nleft > 1) {
    sum += *w++;
    nleft -= 2;
  }

  if (nleft == 1)
  {
    * (unsigned char *) (&answer) = * (unsigned char *) w;
    sum += answer;
  }

  sum = (sum >> 16) + (sum & 0xffff); /* add high 16 to low 16 */
  sum += (sum >> 16);     /* add carry */
  answer = ~sum;     /* truncate to 16 bits */

  return ((uint16_t) answer);
}
