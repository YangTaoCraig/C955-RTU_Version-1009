/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics Ltd.

 PROPRIETARY RIGHTS of ST Electronics Ltd are involved in the subject matter
 of this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  mmm.cpp                                               D1.0.4

 COMPONENT

  MMM - Maintenance Management Module

 DESCRIPTION

  This file implements the Maintenance Manangement Module.

 AUTHOR

  Yu Wei


 HISTORY

    NAME            DATE                    REMARKS

  Yu Wei        19-Jun-2003    RMM D.1.1.2. Created initial version
                                  [Refer to modification history below]
  Bryan Chong   03-Aug-2009    Update comments

----------------------------------------------------------------------------*/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/neutrino.h>
#include <termios.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/dcmd_chr.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "err_ext.h"
#include "ser_def.h"
#include "ser_ext.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
#include "sys_ass.h"
#include "swc_def.h"
#include "ser_def.h"
#include "ser_ext.h"
#include "lnc_def.h"
#include "lnc_ext.h"
#include "str_def.h"
#include "str_ext.h"
#include "mmm_ext.h"

#include "MMM.h"
#include "ServerPoll.h"
#include "CMM_Log.h"
#include "CMM_Timer.h"
#include "CMM_Listen.h"
#include "SWC.h"
#include "CMM_Modbus.h"
#include "RMM.h"
#include "Init_SWC.h"    //040510 Yu Wei

//************************************************************************
//  MMM global variables
//************************************************************************

tMMMParam      g_MMMParam;      //MMM parameters
CMMM        *g_pMMM;      //pointer to MMM object
pthread_t      g_nMMMMainTaskID;  //main task ID

//************************************************************************
//  MMM task functions
//************************************************************************

static const UINT8 loopBackTestByte[] =
  {0x00, 0xFF, 0x55, 0xAA, 0x99, 0x66, 0x96, 0x69};

/*------------------------------------------------------------------------------
Purpose:
  Spawn MMM task

Return:
  OK
  ERROR

History

  Name         Date           Remark
  ----         ----           ------
  Bryan Chong  15-Oct-2009  Update g_tRTUConfig.tLAN_Para[x].cLAN_SlaveAddress
                            to g_tRTUConfig.tLAN_Para[x].tPrtc.addr due to
                            parameter change in latest Init_Config.cpp
  Bryan Chong  20-Sep-2010  Update RMT to listen at port 6 IP address instead of
                            port 1 [PR74]
------------------------------------------------------------------------------*/
int MMMTaskSpawn(void)
{
  E_ERR_T rstatus = E_ERR_Success;
  g_MMMParam.nRecvTimeout = g_tRTUConfig.nMaintTerminalTimeout;

  strcpy(g_MMMParam.acListenIP, g_tRTUConfig.tLANPara[5].acLAN_IP);

  //create MMM object
  g_pMMM = new CMMM(g_tRTUConfig.nMaintTerminalSocketID);

  if (g_pMMM == TYP_NULL)
  {
    g_pEventLog->LogMessage((CHAR *)"MMM: create object failed\n");
    return ERROR;
  }

  rstatus = SYS_ThrdInit(E_SYS_TLB_MMM_Task);
  if(rstatus != E_ERR_Success)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [MMM] MMMTaskSpawn, create MMM Main Task "
           "fail\n");
    #endif // CFG_PRN_ERR
    g_pEventLog->LogMessage((CHAR *)
       "ERR  [MMM] MMMTaskSpawn, create MMM Main Task.\n");
    return ERROR;
  }

  g_nMMMMainTaskID =
    pSYS_CB->thrd_ctrl[E_SYS_TLB_MMM_Task].thrd_id;

  return OK;
}

/*
Purpose:
  MMM main task function to be spawned
*/
void *MMMMainTask(void *arg)
{
  for (;;)
  {
    g_pMMM->Main();        //Main() will return when there is error
    delay(SYS_CLOCK_SECOND);  //delay 1 sec before re-entering Main()
  }
}
/*
Purpose:
  Stop MMM main task.

History

  Name         Date           Remark
  ----         ----           ------
  Bryan Chong  30-May-2012  Comment out delete g_pMM object that refrain from
                            system reset
*/
void StopMMMMainTask(void)
{
  pthread_detach(g_nMMMMainTaskID);
  pthread_cancel(g_nMMMMainTaskID);
  //delete g_pMMM;    //Deleted object.  040604 Yu Wei
}

//************************************************************************
//  Modbus functions
//************************************************************************
/*
Purpose:
  Send Modbus error reply
*/
void ModbusSendErrorReplyMMM
(
  int nFd,            //in: file descriptor
  unsigned char ucSlaveAddress,  //in: slave address
  unsigned char ucFnCode,      //in: cmd fn code
  unsigned char ucExceptionCode  //in: exception code
)
{
  unsigned char ucErrorReply[5];


  ucErrorReply[0] = ucSlaveAddress;
  ucErrorReply[1] = ucFnCode | 0x80; //Execption use | not +. 040330 Yu Wei
  ucErrorReply[2] = ucExceptionCode;
  *(unsigned short *)&ucErrorReply[3] = STR_ByteSwap16Bit(ModbusCRC(ucErrorReply, 3));
//  ucErrorReply[3] = (unsigned char ) (ModbusCRC(ucErrorReply, 3));
//  ucErrorReply[4] = (unsigned char ) (ModbusCRC(ucErrorReply, 3)<< 8 );
  printf("mod bus send ErrorReply MMM ucErrorReply %d \n" ,ucErrorReply);
  send(nFd, (char *)ucErrorReply, sizeof(ucErrorReply), MODBUS_SEND_REPLY_TIMEOUT);
}


//************************************************************************
//  MMM class
//************************************************************************

CMMM::CMMM(UINT16 unListenPort)
{
  m_unListenPort = unListenPort;

  m_nClientOK = -1; //flag for login status 1 or -1

  //zero polling table and temporary copy
  //memset(m_unTablePoll, 0, sizeof(m_unTablePoll));
}

CMMM::~CMMM()
{
}

/*
Purpose:
  Check Modbus cmd and send error reply for Modbus exception 1 and 2
Return:
  OK
  ERROR:        Cmd size, CRC or slave address error
  MODBUS_EXCEPTION1:  Fn code error
  MODBUS_EXCEPTION2:  Address error
*/
int CMMM::ModbusCmdCheck
(
  int nFd,          //in: file descriptor
  unsigned char *pucCmd,    //in: Modbus cmd
  int nCmdSize        //in: Modbus cmd size
)
{
  unsigned short unAddress;  //address of 1st word to read extracted from cmd
  unsigned short tmp;

  switch(ModbusCMDCheck(pucCmd, nCmdSize,  //Check command first. 040331 Yu Wei.
        g_MMMParam.ucModbusSlaveAddress))
  {
  case MODBUS_ERROR_CRC:
  default:
    return ERROR;

  case MODBUS_ERROR_EXCEPTION1:
    ModbusSendErrorReply(nFd, pucCmd[0], pucCmd[1], MODBUS_EXCEPTION1);
    return MODBUS_EXCEPTION1;

  case MODBUS_ERROR_EXCEPTION3:
    ModbusSendErrorReply(nFd, pucCmd[0], pucCmd[1], MODBUS_EXCEPTION3);
    return MODBUS_EXCEPTION3;

  case MODBUS_MESSAGE_OK:

    //check fn code must be read/write word
    if ( (pucCmd[1] != 4) &&(pucCmd[1] != 16) )
    {
      ModbusSendErrorReplyMMM(nFd, pucCmd[0], pucCmd[1], MODBUS_EXCEPTION1);
      return MODBUS_EXCEPTION1;
    }

    //check address
    unAddress = *(unsigned short *)&pucCmd[2];
    tmp = STR_ByteSwap16Bit(unAddress);
//    printf("MMM unAddress %X \n" ,tmp );

    if (pucCmd[1] == 4)
    {
      if ( ReadCmdCheck(tmp) == true)    //040322 Yu Wei
      {
        ModbusSendErrorReplyMMM(nFd, pucCmd[0], pucCmd[1], MODBUS_EXCEPTION2);
        return MODBUS_EXCEPTION2;
      }
    }

    if (pucCmd[1] == 16)
    {
      if ( WriteCmdCheck(tmp) == true)
      {
        ModbusSendErrorReplyMMM(nFd, pucCmd[0], pucCmd[1], MODBUS_EXCEPTION2);
        return MODBUS_EXCEPTION2;
      }
    }
    break;
  }

  return OK;
}

/*
Purpose:
  Check if read command address is valid.
  Added to reduce complexity in 22 March 2004, Yu Wei
*/
bool CMMM::ReadCmdCheck
(
  unsigned short unAddress    //in: read command address
)
{
  if ( ! ( (unAddress == MMM_CMD_MEMORYTEST) ||
         (unAddress == MMM_CMD_GETCLOCK) ||
         ((unAddress >= MMM_CMD_SERIALTEST1)     //Serial port test command
       && (unAddress <= MMM_CMD_SERIALTEST8)) ||
         ((unAddress >= MMM_CMD_SERIALTEST9)
       && (unAddress <= MMM_CMD_SERIALTEST16)) ||
         (unAddress == MMM_CMD_RTU_STATUS) ||
         (unAddress == MMM_CMD_RTU_POLL) ||
         (unAddress == MMM_CMD_RTU_CMD) ||
         ((unAddress > 500)                //Read SWC table command
       && (unAddress < 700)
       && ((unAddress % 10) >=1)
       && ((unAddress % 10) <=4))))
  {
    return true;
  }
  else
    return false;
}

/*
Purpose:
  Check if write command address is valid.
  Added to reduce complexity in 22 March 2004, Yu Wei
*/
bool CMMM::WriteCmdCheck
(
  unsigned short unAddress    //in: write command address
)
{
  if ( (unAddress != MMM_CMD_LOGIN) &&
     (unAddress != MMM_CMD_SET_DOWNLOAD_CFG) &&
     (unAddress != MMM_CMD_SETCLOCK) &&
     (unAddress != MMM_CMD_RESET) &&
     (unAddress != MMM_CMD_SWC_POLLING_ENABLE_INHIBIT)&&
     (unAddress != MMM_CMD_SWC_WRITE_AI_MI) &&    //040322 Yu Wei
     (unAddress != MMM_CMD_SWC_WRITE_DI_DI8))    //040322 Yu Wei
  {
    return true;
  }
  else
    return false;
}
/*-----------------------------------------------------------------------------

 PRIVATE ROUTINE

  CMMM::clientHandler

 DESCRIPTION

  This routine will Receive Modbus cmd from client and send reply

 CALLED BY

  [TBD]

 CALLS

  [TBD]

 PARAMETER

  nSocketClient   [in] client socket


 RETURN

   E_ERR_Success      the routine executed successfully
   TIMEOUT            Timeout value

 AUTHOR

  Yu Wei


 HISTORY

    NAME            DATE                    REMARKS

   Yu Wei         22-Mar-2004      Created initial revision
   Bryan Chong    04-Sep-2009      Update to include error handling and
                                   componentization
   Bryan Chong    20-Oct-2010      Fix serial test command return packet
   Bryan Chong    28-Apr-2011      Update default case with "break;" [C955 PR90]
   Bryan Chong    17-Aug-2011      Add case to handle -1.
-----------------------------------------------------------------------------*/
E_ERR_T CMMM::clientHandler (INT32 nSockClient)
{
  UINT8  ucCmd[MMM_MAX_CMD_SIZE];  //buffer to receive cmd
  INT32  nBytesRecv;              //bytes received from client
  INT32  nDataWordSize;            //size of data in word
  INT32  nParaAddress = 0;        //Parameter address.
  CHAR   acTemp[512];              //For write log.
  UINT16 tmp[2];

  E_ERR_T rstatus = E_ERR_Success;
  #ifdef CFG_PRN_ERR
  CHAR acErrMsg[100] = {0};
  #endif // CFG_PRN_ERR

  INT32 sendlength = 0;
  UINT16 *pserialIndex;
  UINT16 serialIndex;
  INT32 rval = 0;

  while(1)
  {
    rstatus = LNC_ReceiveMsg(nSockClient, (VOID *)ucCmd, &nBytesRecv,
                             sizeof(ucCmd), g_MMMParam.nRecvTimeout);

    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_PRN_ERR
      ERR_GetMsgString(rstatus, acErrMsg);
      printf("ERR  [MMM] CMMM::clientHandler, rx msg fail at fd %d, "
             "rx size %d, buffer sz %d, %s\n",
              nSockClient, nBytesRecv, sizeof(ucCmd), acErrMsg);
      #endif // CFG_PRN_ERR
    }

    if(rstatus == E_ERR_LNC_SelectReadTimeOut)
      nBytesRecv = TIMEOUT;

    switch(nBytesRecv)
    {
      case 0:
         if (m_nClientOK)
           m_nClientOK = -1;
         #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
         printf("[MMM] CMMM::clientHandler,  client disconnected\n");
         #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
         return E_ERR_MMM_RecvClientCommandFail;

      case TIMEOUT:
        if (m_nClientOK)
          m_nClientOK = -1;

        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
        printf("[MMM] CMMM::clientHandler, client login timeout\n");
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
        return E_ERR_MMM_ClientLoginTimeout;

      case -1:
        if (m_nClientOK)
          m_nClientOK = -1;

        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
        printf("[MMM] CMMM::clientHandler, rx client command fail\n");
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
        return E_ERR_MMM_RecvClientCommandFail;


      default:
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
        printf("[MMM] CMMM::clientHandler, rx cmd %d bytes:\n", nBytesRecv);
        SYS_PrnDataBlock((const UINT8 *) ucCmd, nBytesRecv, 20);
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
        break;
    }

    tmp[0] = STR_ByteSwap16Bit(*(UINT16 *)&ucCmd[2]);

    // check if command is valid
    rval = ModbusCmdCheck(nSockClient, ucCmd, nBytesRecv);
    if (rval != OK)
    {
      #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_MMM)
      //client cmd error
      printf("WARN [MMM] CMMM::clientHandler, fd %d rx cmd modbus error, %d\n",
             nSockClient, rval);
      #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_MMM)
      // continue;
    }

    if (m_nClientOK < 0 )
    {
      //check user name and password
      if ((tmp[0] != MMM_CMD_LOGIN) ||
          (CheckLogin((CHAR *)&ucCmd[7], (CHAR *)&ucCmd[16]) !=  E_ERR_Success))
      {
        #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_MMM)
        //login failed
        printf("WARN [MMM] CMMM::clientHandler, login failed\n");
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
        return E_ERR_MMM_LoginFail;
      }

      sprintf(acTemp,
        "[MMM] CMMM::clientHandler, RTU Maintenance Terminal '%s' Login.\n",
        (CHAR *)&ucCmd[7]);  //040322 Yu Wei
      g_pEventLog->LogMessage(acTemp);  //040322 Yu Wei

      //login OK
      m_nClientOK = 1;

      //RMT login OK, set flag. //040419 Yu Wei
      g_tRTUStatus.unRTURMTLinkStatus = RMT_CONNECTED;
    }

    //tmp[1] = STR_ByteSwap16Bit(*(UINT16 *)&ucCmd[nParaAddress]);
    if(tmp[0] == MMM_CMD_RTU_POLL)  //Get polling table,
      nParaAddress = 4;                  //Read data length in position 4.
    else
      nParaAddress = 7;

    tmp[1] = STR_ByteSwap16Bit(*(UINT16 *)&ucCmd[nParaAddress]);

    nDataWordSize = ProcessCmd(tmp[0],(UINT16 *)&ucCmd[nParaAddress]);

    #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_MMM)
    printf("[MMM] CMMM::clientHandler, nDataWordSize = %d\n",
           nDataWordSize);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
    if (nDataWordSize == 0)
    {
      //construct reply for login
      m_ucSendBuffer[0] = ucCmd[0];  //same slave address as cmd
      m_ucSendBuffer[1] = ucCmd[1];  //same fn code as cmd
      m_ucSendBuffer[2] = ucCmd[2];  //same starting address as cmd
      m_ucSendBuffer[3] = ucCmd[3];
      m_ucSendBuffer[4] = ucCmd[4];  //same no of words as cmd
      m_ucSendBuffer[5] = ucCmd[5];

      //calc CRC
      *(UINT16 *)&m_ucSendBuffer[6] =
        STR_ByteSwap16Bit(ModbusCRC(m_ucSendBuffer, 6));

      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      printf("[MMM] CMMM::clientHandler, cmd return 0x%04x, fd %d tx %d "
             "bytes:\n",
             nDataWordSize, nSockClient, 8);
      SYS_PrnDataBlock((const UINT8 *)m_ucSendBuffer, 8, 20);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)

      rstatus = LNC_SendMsg(nSockClient, m_ucSendBuffer, 8, &sendlength);
//      if(rstatus != E_ERR_Success)
//      {
////        #ifdef CFG_PRN_ERR
////        ERR_GetMsgString(rstatus, acErrMsg);
////        printf("WARN [MMM]  CMMM::clientHandler, err msg %s\n", acErrMsg);
////        #endif // CFG_PRN_ERR
//      }
//
     }else if(nDataWordSize == 0xFFFF){
      m_ucSendBuffer[0] = ucCmd[0];      //same slave address as cmd
      m_ucSendBuffer[1] = ucCmd[1] | 0x80;  //Exception reply.
      m_ucSendBuffer[2] = 0x04;        //Exception4, command cannot execute.

      //calc CRC
      *(UINT16 *)&m_ucSendBuffer[3] =
        STR_ByteSwap16Bit(ModbusCRC(m_ucSendBuffer, 3));

      //  m_ucSendBuffer[3] = (unsigned char )(ModbusCRC(m_ucSendBuffer, 3));
      //  m_ucSendBuffer[4] =
      //    (unsigned char )(ModbusCRC(m_ucSendBuffer, 3) << 8 );

      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      printf("[MMM] CMMM::clientHandler, cmd return 0x%04x, fd %d tx %d "
             "bytes:\n",
             nDataWordSize, nSockClient, 5);
      SYS_PrnDataBlock((const UINT8 *)m_ucSendBuffer, 5, 20);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)

      rstatus = LNC_SendMsg(nSockClient, m_ucSendBuffer, 5, &sendlength);
//      if(rstatus != E_ERR_Success)
//      {
//        #ifdef CFG_PRN_ERR
//        ERR_GetMsgString(rstatus, acErrMsg);
//        printf("WARN [MMM]  CMMM::clientHandler, err msg %s\n", acErrMsg);
//        #endif // CFG_PRN_ERR
//      }

    } else {
      pserialIndex = (UINT16 *)&ucCmd[2];
      serialIndex = STR_SWAP_16BIT_REG(*pserialIndex);

      //construct reply packet
      m_ucSendBuffer[0] = ucCmd[0];      //same slave address as cmd
      m_ucSendBuffer[1] = ucCmd[1];      //same fn code as cmd

      if(nDataWordSize <= 127)
        m_ucSendBuffer[2] = nDataWordSize*2;  //no. of bytes read
      else
        m_ucSendBuffer[2] = 0;        //>127 set to 0.


      #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_MMM)
      printf("[MMM] CMMM::clientHandler, serial index = 0x%04x\n",
             serialIndex);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      //20101020 BC (Rqd by ZSL)
      switch(serialIndex)
      {
        case MMM_CMD_SERIALTEST1:
        case MMM_CMD_SERIALTEST2:
        case MMM_CMD_SERIALTEST3:
        case MMM_CMD_SERIALTEST4:
        case MMM_CMD_SERIALTEST5:
        case MMM_CMD_SERIALTEST6:
        case MMM_CMD_SERIALTEST7:
        case MMM_CMD_SERIALTEST8:
        case MMM_CMD_SERIALTEST9:
        case MMM_CMD_SERIALTEST10:
        case MMM_CMD_SERIALTEST11:
        case MMM_CMD_SERIALTEST12:
        case MMM_CMD_SERIALTEST13:
        case MMM_CMD_SERIALTEST14:
        case MMM_CMD_SERIALTEST15:
        case MMM_CMD_SERIALTEST16:
          m_ucSendBuffer[3] = 0;
          m_ucSendBuffer[4] = 1;
          break;
      }

      //calc CRC
      *(UINT16 *)&m_ucSendBuffer[3 + nDataWordSize*2] =
          STR_ByteSwap16Bit(ModbusCRC(m_ucSendBuffer, 3 + nDataWordSize*2));

      rstatus = LNC_SendMsg(nSockClient, m_ucSendBuffer,
                            3 + nDataWordSize*2 + 2, &sendlength);
//      if(rstatus != E_ERR_Success)
//      {
//        #ifdef CFG_PRN_ERR
//        ERR_GetMsgString(rstatus, acErrMsg);
//        printf("WARN [MMM]  CMMM::clientHandler, err msg %s\n", acErrMsg);
//        #endif // CFG_PRN_ERR
//      }
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      printf("[MMM] CMMM::clientHandler, cmd return 0x%04x, fd %d tx %d "
             "bytes:\n",
             nDataWordSize, nSockClient, 3 + nDataWordSize*2 + 2);
      SYS_PrnDataBlock((const UINT8 *) m_ucSendBuffer,
                       3 + nDataWordSize*2 + 2, 20);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)

    } //if (nDataWordSize == 0)
  } // while(1)
} // CMMM::clientHandler

/*
Purpose:
  Start listening for client connection and process client transactions
*/
E_ERR_T CMMM::Main()
{
  int nSockListen;        //listen socket
  struct sockaddr_in addrListen;  //listen socket address
  int nOn = 1;          //set socket option value for re-use socket address

  int nSockClient;        //client socket
  struct sockaddr_in addrClient;  //client socket address
  int nAddrSize;          //size of address
  //char cLog[128];          //error log buffer


  //create listen socket
  nSockListen = socket(AF_INET, SOCK_STREAM, 0);

  if (nSockListen == ERROR)
  {
    g_pEventLog->LogMessage((CHAR *)"MMM: create listen socket failed\n");
    return E_ERR_MMM_CreateListenSocketFail;
  }

  //enable re-use socket address option
  if (setsockopt(nSockListen, SOL_SOCKET, SO_REUSEADDR, (char *)&nOn, sizeof(nOn)) == ERROR)
  {
    g_pEventLog->LogMessage((CHAR *)"MMM: set socket re-use address failed\n");
    close(nSockListen);
    return E_ERR_MMM_SetSocketReuseAddressFail;
  }

  //init listen socket address
  memset(&addrListen, 0, sizeof(addrListen));
  addrListen.sin_family = AF_INET;
  addrListen.sin_port = htons(m_unListenPort);
  addrListen.sin_addr.s_addr = inet_addr(g_MMMParam.acListenIP);

  //bind listen socket
  if (bind(nSockListen, (struct sockaddr *)&addrListen,
           sizeof(struct sockaddr)) == ERROR)
  {
    g_pEventLog->LogMessage((CHAR *)"MMM: bind listen socket failed\n");
    close(nSockListen);
    return E_ERR_MMM_BindSocketFail;
  }

  //enable connections to listen socket, maximum number of 1 unaccepted
  //connection that can be pending at one time
  if (listen(nSockListen, 1) == ERROR)
  {
    g_pEventLog->LogMessage((CHAR *)"MMM: start listen socket failed\n");
    close(nSockListen);
    return E_ERR_MMM_ListenSocketFail;
  }

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
  printf("[MMM] CMMM::Main, MMM is ready and start listening ...\r\n");
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)

  while(1)
  {
    //accept client socket
    nAddrSize = sizeof(struct sockaddr_in);
    delay(10);
    nSockClient = accept(nSockListen, (struct sockaddr *)&addrClient,
                        &nAddrSize);

    if (nSockClient == ERROR)
    {
      g_pEventLog->LogMessage((CHAR *)"MMM: accept client failed\n");
      close(nSockListen);
      return E_ERR_MMM_AcceptClientSocketFail;
    }
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
    printf("[MMM] CMMM::Main, accept client %s:%d, fd %d\n",
           g_MMMParam.acListenIP, m_unListenPort, nSockClient);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)

    //process client transactions
    clientHandler(nSockClient);

    //RMT logout or timeout, set flag to LINK_FAULTY.
    g_tRTUStatus.unRTURMTLinkStatus = LINK_FAULTY;  //040419 Yu Wei

    #ifdef CFG_PRN_ERR
    printf("ERR [MMM] CMMM::Main, MMM is closing socket\r\n");
    #endif // CFG_DEBUG_MSG

    //close client socket
    shutdown(nSockClient, 1);  //send disallowed
    close(nSockClient);
  } // while(1)
} // void CMMM::Main()

/*
Purpose:
  Process MMM cmd code
Return:
  No of words read
  0xFFFF  -- Command error or command cannot execute.
*/

/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  CMMM::ProcessCmdCode

 DESCRIPTION

  This routine will parse MMM command code

 CALLED BY

  [TBD]

 CALLS

  [TBD]

 PARAMETER

  unCmd         [in] input command
  *unParamter   [in] pointer to parameters

 RETURN

  E_ERR_Success           the routine executed successfully
  E_ERR_InvalidNullPointer
  MMM_CMD_SERIAL_TEST     Serial port test
  MMM_CMD_SWC_READ_TABLE  Read SWC table command

 AUTHOR

  Yu Wei


 HISTORY

    NAME            DATE                    REMARKS

   Yu Wei         22-Mar-2004      Created initial revision

-----------------------------------------------------------------------------*/
UINT16 CMMM::ProcessCmdCode(UINT16 unCmd, UINT16 *unParameter)
{
  if((unCmd >= MMM_CMD_SERIALTEST1) &&(unCmd <= MMM_CMD_SERIALTEST8))
  {
    unParameter[0] = unCmd - MMM_CMD_SERIALTEST1;
    unCmd = MMM_CMD_SERIAL_TEST;
  }
  else if((unCmd >= MMM_CMD_SERIALTEST9) && (unCmd <= MMM_CMD_SERIALTEST16))
  {
    unParameter[0] = unCmd - MMM_CMD_SERIALTEST9 + 8;
    unCmd = MMM_CMD_SERIAL_TEST;
  }
  else if((unCmd > 500) && (unCmd < 700))
  {
    unParameter[0] = (unCmd - MMM_CMD_SWC_READ_TABLE)/10;  //SWC ID
    unParameter[1] = unCmd % 10;    //Table ID

    if(unParameter[1] == 1)
      unParameter[1] = 0;
    else if( unParameter[1] == 4)
      unParameter[1] = 1;

    unCmd = MMM_CMD_SWC_READ_TABLE;
  }
  return unCmd;
} // CMMM::ProcessCmdCode


/*
Purpose:
  Process MMM cmd
Return:
  No of words for response message.
   0xFFFF  -- Command error or command cannot execute.
*/

/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  CMMM::ProcessCmd

 DESCRIPTION

  This routine will parse MMM command code

 CALLED BY

  [TBD]

 CALLS

  [TBD]

 PARAMETER

  unCmd         [in] input command
  *unParamter   [in] pointer to parameters

 RETURN

  0      Success
  1      Read RTU last command successful
  3      Read clock value successful
  33     RTU status table length, 33 bytes
  65535  Error. Invalid command or fail in memory test


 AUTHOR

  Yu Wei


 HISTORY

    NAME            DATE                    REMARKS

   Yu Wei         22-Mar-2004      Created initial revision

-----------------------------------------------------------------------------*/
UINT16 CMMM::ProcessCmd (UINT16 unCmd, UINT16 *unParameter)
{
  E_ERR_T rstatus = E_ERR_Success;
  struct timespec tsTime;
  struct tm tmTime;

  //the structure for message Q that send to RMM.
  struct tRMM_MSG tRMMMessageSend;
  unsigned short tmp;
  CHAR errmsg[100] = {0};
  CHAR prnbuff[200] = {0};

  unCmd = ProcessCmdCode(unCmd, unParameter);

  switch (unCmd)
  {
    case MMM_CMD_LOGIN:    //040310
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      printf("[MMM] CMMM::ProcessCmd, rx cmd MMM_CMD_LOGIN\n");
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      return 0;

    case MMM_CMD_RESET:    //Reset RTU
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      printf("[MMM] CMMM::ProcessCmd, rx cmd MMM_CMD_RESET\n");
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      tRMMMessageSend.nDeviceID = MMM_MAIN_TASK_ID_INDEX;
      tRMMMessageSend.nMessageID = MMM_STATE_CHANGE;
      tRMMMessageSend.unStatus = STATEFLAG_SYSTEM_RESET;
      g_pRTUStatusTable->SendMsg(tRMMMessageSend);
      return 0;

    case MMM_CMD_SET_DOWNLOAD_CFG:
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      printf("[MMM] CMMM::ProcessCmd, rx cmd MMM_CMD_SET_DOWNLOAD_CFG\n");
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      g_tRTUStatus.bCFGFileDownloadRequired = true;
      return 0;

    case MMM_CMD_SETCLOCK:  //Update clock.
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      printf("[MMM] CMMM::ProcessCmd, rx cmd MMM_CMD_SETCLOCK\n");
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)

      if((g_pRTUStatusTable->
          m_aunLocalRTUStatus[RTU_STATUS_TABLE_SWC_OFFSET + SWC_CLOCK_INDEX] &
          VALIDAION_BIT) == 0)
      {
        //Clock link down, update the clock. Otherwise don't update.
        tsTime.tv_sec = (time_t)(STR_ByteSwap16Bit(unParameter[0]))* 0x10000 +
                        (time_t)STR_ByteSwap16Bit(unParameter[1]);

        tsTime.tv_nsec = STR_ByteSwap16Bit(unParameter[2]);

        //convert seconds to YYMMDD, HHMMSS
        localtime_r(&tsTime.tv_sec, &tmTime);

        //Check if update RTC and system time is necessary
        rstatus = SYS_CheckToUpdateRTC(tmTime);
        if(rstatus != E_ERR_Success)
        {
          ERR_GetMsgString(rstatus, errmsg);
          sprintf(prnbuff, "[MMM] SYS_UpdatRTC, fail, %s\n", errmsg);
          g_pEventLog->LogMessage(prnbuff);
        }

      }else{
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
        printf("[MMM] CMMM::ProcessCmd, clock link down, update the clock. "
               "Otherwise don't update.\n");
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      }
      return 0;

    case MMM_CMD_SWC_POLLING_ENABLE_INHIBIT:
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      printf("[MMM] CMMM::ProcessCmd, rx cmd "
             "MMM_CMD_SWC_POLLING_ENABLE_INHIBIT\n");
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)

      tmp = STR_ByteSwap16Bit(unParameter[0]);
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      printf("[MMM] CMMM::ProcessCmd, MMM_CMD_SWC_POLLING_ENABLE_INHIBIT "
             "rx cmd bytes: 0x%04x\n", unCmd);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)

      return SetSWCPollingFlag(tmp);  //Set SWC polling flag. 040322 Yu Wei

    case MMM_CMD_SWC_WRITE_AI_MI:  //040322 Yu Wei
    case MMM_CMD_SWC_WRITE_DI_DI8:  //040322 Yu Wei
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)//
//      printf("[MMM] CMMM::ProcessCmd, rx cmd "
//             "MMM_CMD_SWC_WRITE_AI_MI or MMM_CMD_SWC_WRITE_DI_DI8\n");
//      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
//      return WriteSWCTable(unParameter[0], unParameter[1], unParameter[2]);

    case MMM_CMD_MEMORYTEST:
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      printf("[MMM] CMMM::ProcessCmd, rx cmd MMM_CMD_MEMORYTEST\n");
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      return MemoryTest();  //Memory test. 040322 Yu Wei

    case MMM_CMD_GETCLOCK:
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      printf("[MMM] CMMM::ProcessCmd, rx cmd MMM_CMD_GETCLOCK\n");
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      tsTime.tv_sec = 0;
      clock_gettime(CLOCK_REALTIME, &tsTime);

      m_ucSendBuffer[3] = (unsigned char )(tsTime.tv_sec >> 24);
      m_ucSendBuffer[4] = (unsigned char )(tsTime.tv_sec >> 16);
      m_ucSendBuffer[5] = (unsigned char )(tsTime.tv_sec >>  8);
      m_ucSendBuffer[6] = (unsigned char )(tsTime.tv_sec );

      *(unsigned short *)&m_ucSendBuffer[7] = (unsigned short )(tsTime.tv_nsec >> 16);  //Convert to ms from ns.
      return 3;

    case MMM_CMD_SERIAL_TEST:
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      printf("[MMM] CMMM::ProcessCmd, rx cmd MMM_CMD_SERIAL_TEST, para 0 = %d "
             "para 1= %d\n", unParameter[0], unParameter[1]);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)

      rstatus = SerialComLoopBackTest(
        (E_SER_COM)(unParameter[0] + E_SER_COM_Port_5));
      if(rstatus == E_ERR_Success)
      {
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
        printf("[MMM] CMMM::ProcessCmd, serial COM test for com %d is "
               "successful\n",
               (unParameter[0] + E_SER_COM_Port_5));
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
        return 1;
      }
      else
      {
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
        ERR_GetMsgString(rstatus, errmsg);
        printf("[MMM] CMMM::ProcessCmd, serial COM test for port %d fail, %s\n",
               (unParameter[0] + E_SER_COM_Port_5), errmsg);
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
        return 0;
      }

    case MMM_CMD_RTU_STATUS:
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      printf("[MMM] CMMM::ProcessCmd, rx cmd MMM_CMD_RTU_STATUS\n");
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)

      g_pRTUStatusTable->ReadRTUStatus((unsigned short *)&m_ucSendBuffer[3], WAIT_FOREVER);

      return RTU_STATUS_TABLE_LENGTH_MAX;    //Use defined value  //040512 Yu Wei

    case MMM_CMD_SWC_READ_TABLE:
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      printf("[MMM] CMMM::ProcessCmd, rx cmd MMM_CMD_SWC_READ_TABLE\n");
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)

      return ReadSWCTable(unParameter[0], unParameter[1]);  //040322 Yu Wei

    case MMM_CMD_RTU_POLL:
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      printf("[MMM] CMMM::ProcessCmd, rx cmd MMM_CMD_RTU_POLL\n");
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)

      //Polling table, not status table //040512 Yu Wei
      memcpy((unsigned short *)&m_ucSendBuffer[3],
              &g_apServerPoll[0]->
               m_unTablePoll[g_tRTUConfig.tRTUPollingTable.unTableStartAddress],
              unParameter[0]*2);
      //Use socket 0 polling table.  //040512 Yu Wei
      return unParameter[0];

    case MMM_CMD_RTU_CMD:
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      printf("[MMM] CMMM::ProcessCmd, rx cmd MMM_CMD_RTU_CMD\n");
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)

      *(unsigned short *)&m_ucSendBuffer[3] = STR_ByteSwap16Bit(g_unLastServerCommand);//last server command
      return 1;

    default:        //040310 Yu Wei
      #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_MMM)
      printf("WARN [MMM] CMMM::ProcessCmd, unhandled cmd error\n");
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)

      return 0xFFFF;
  } // switch (unCmd)
  return 0;
}

/*------------------------------------------------------------------------------
Purpose:
  Read SWC table.

Parameter:
  unSWCIndex     [in] SWC index number
  unSWCTableID   [in] SWC table ID

Return:
  No of words for response message.
  0xFFFF  -- Command error or command cannot execute.

History
  Name          Date        Remark
  ----          ----        ------
 Bryan Chong  12-Jul-2010   Update nSWCObjectID with m_nSWCID from
                            g_apSWCObject
 Bryan Chong  18-Aug-2011   Validate g_apSWCObject is not a null pointer
                            before proceeding to identify the SWC object ID
------------------------------------------------------------------------------*/
UINT16 CMMM::ReadSWCTable(UINT16 unSWCIndex, UINT16 unSWCTableID)
{
  int nSWCObjectID;
  int nDataWordSize;
  int nTotalWords;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
  printf("[MMM] CMMM::ReadSWCTable,  swc idx %d, tableID %d, g_apSWCObject[%d] "
         "0x%08x\n",
         unSWCIndex, unSWCTableID, unSWCIndex,
         (UINT32)g_apSWCObject[unSWCIndex]);
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)

  if(g_apSWCObject[unSWCIndex] == TYP_NULL)
    return (UINT16)TYP_ERROR;

  // 20100712 BC (Rqd by ZSL)
  nSWCObjectID = g_apSWCObject[unSWCIndex]->m_nSWCID;
  nDataWordSize = 0;
  nTotalWords = 0 ;


  if((nSWCObjectID < g_tRTUConfig.nSWCNumber) &&
     (unSWCTableID < SWC_TOTAL_TABLE))
  {
    //2008 get all data
    if(g_apSWCObject[nSWCObjectID]->m_nLinkType == SWC_LINK_RS_485M)
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      printf("[MMM] CMMM::ReadSWCTable,  485M link read table\n");
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      for(int NodeID  = 0;
          NodeID < g_apSWCObject[nSWCObjectID]->m_nTotolNodeNumber; NodeID ++)
      {
        nDataWordSize = g_apSWCObject[nSWCObjectID]->ReadSWCOneTable(
                (char *)&m_ucSendBuffer[3+nTotalWords*2],NodeID+1,WAIT_FOREVER);
        if(nDataWordSize > 0)
        {
          nTotalWords +=nDataWordSize;
        }
      }
      if(nDataWordSize == 0)
        return 0xFFFF;
      else
        return nTotalWords;
    }
    else
    {
      nDataWordSize = g_apSWCObject[nSWCObjectID]->ReadSWCOneTable(
        (CHAR *)&m_ucSendBuffer[3],unSWCTableID, WAIT_FOREVER);    //040323 Yu Wei

      if(nDataWordSize == 0)
      {
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
        printf("[MMM] CMMM::ReadSWCTable, %s table %d not defined\n",
               g_apSWCObject[nSWCObjectID]->m_acSWCName, unSWCTableID);
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
        return 0xFFFF;
      }else{
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
        switch(unSWCTableID)
        {
          case SWC_TABLE_DI:
          case SWC_TABLE_DI8:
            printf("[MMM] CMMM::ReadSWCTable,  read %s DI table %d, %d "
                   "bytes:\n",
                   g_apSWCObject[nSWCObjectID]->m_acSWCName, unSWCTableID,
                  ((g_apSWCObject[nSWCObjectID]->m_nServerFastTableLen * 2)
                   + 5));
            SYS_PrnDataBlock((const UINT8 *) m_ucSendBuffer,
              ((g_apSWCObject[nSWCObjectID]->m_nServerFastTableLen *2) + 5),
              20);
            break;

          case SWC_TABLE_AI_MI:
            printf("[MMM] CMMM::ReadSWCTable,  read %s AI-MI table %d, %d "
                   "bytes:\n",
                   g_apSWCObject[nSWCObjectID]->m_acSWCName, unSWCTableID,
                  ((g_apSWCObject[nSWCObjectID]->m_nServerSlowTableLen * 2)
                   + 5));
            SYS_PrnDataBlock((const UINT8 *) m_ucSendBuffer,
              ((g_apSWCObject[nSWCObjectID]->m_nServerSlowTableLen *2) + 5),
              20);
            break;

          case SWC_TABLE_MI:
            printf("[MMM] CMMM::ReadSWCTable,  read %s MI table %d, %d "
                   "bytes:\n",
                   g_apSWCObject[nSWCObjectID]->m_acSWCName, unSWCTableID,
                  ((g_apSWCObject[nSWCObjectID]->m_nServerTimerTableLen * 2)
                   + 5));
            SYS_PrnDataBlock((const UINT8 *) m_ucSendBuffer,
              ((g_apSWCObject[nSWCObjectID]->m_nServerTimerTableLen *2) + 5),
              20);
            break;

          default:
            printf("[MMM] CMMM::ReadSWCTable,  read %s undefined table %d\n",
                   g_apSWCObject[nSWCObjectID]->m_acSWCName, unSWCTableID);


        }
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
        return nDataWordSize;
      }
    }
  }
  else
  {
    #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_MMM)
    printf("WARN [MMM] CMMM::ReadSWCTable, invalid SWC index, %d. "
           "unSWCIndex = %d, %d, %d\n",
           g_tRTUConfig.nSWCNumber, unSWCIndex,
           g_tRTUConfig.anSWCIndex[unSWCIndex],
           g_apSWCObject[unSWCIndex]->m_nSWCID);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
    return 0xFFFF;
  }

  return 0;
}

/*
Purpose:
  Write SWC table.
Return:
  0    -- Write OK.
  0xFFFF  -- Command error or command cannot execute.
*/
// Yang Tao 17042013 Remove from MMM.cpp
//unsigned short CMMM::WriteSWCTable
//(
//  unsigned short unSWCIndex,    //in: SWC index number.
//  unsigned short unDataAddress,  //in: The address that data will be written.  //040331 Yu Wei
//  unsigned short unData      //in: write data.
// )
//{
//  int nSWCObjectID;
//  int nI;
//
//  nSWCObjectID = SWC_MAX_NUMBER;
//
//  for(nI=0; nI<g_tRTUConfig.nSWCNumber; nI++)
//  {
//    if(unSWCIndex == g_tRTUConfig.anSWCIndex[nI])
//    {
//      nSWCObjectID = nI;
//      break;
//    }
//  }
//
//  if(nSWCObjectID < g_tRTUConfig.nSWCNumber) //  040331 Yu Wei
//  {
//    if (g_apSWCObject[nSWCObjectID]->MMMWriteSWCTable(unDataAddress,unData) ==
//        0xFFFF)  //040331 Yu Wei
//    {
//      return 0xFFFF;  //040331 Yu Wei
//    }
//    else    //040331 Yu Wei
//    {
//      return 0;
//    }
//  }
//  else
//  {
//    return 0xFFFF;
//  }
//}
//


/*------------------------------------------------------------------------------
Purpose:
  Set SWC polling enable/inhibit flag.

Parameters
  unCommand   [in] the word for command.

Return:
  0    -- command execute OK.
  0xFFFF  -- Command error or command cannot execute.
------------------------------------------------------------------------------*/
unsigned short CMMM::SetSWCPollingFlag(unsigned short unCommand)
{
  E_ERR_T rstatus = E_ERR_Success;

  if((unCommand & 0x0200) != 0)
  {
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
    printf("[MMM] CMMM::SetSWCPollingFlag, SWC_POLLING_ENABLE "
           "unCommand = 0x%04x\n", unCommand);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)

    if(g_pRTUStatusTable->SendSWCPollingCommand(SWC_POLLING_ENABLE,
         (unCommand & 0x00FF)) == E_ERR_RMM_SendMsgQueueError)
    {
      return 0xFFFF;
    }
  }
  else if((unCommand & 0x0100) != 0)
  {
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
    printf("[MMM] CMMM::SetSWCPollingFlag, SWC_POLLING_INHIBIT "
           "(unCommand = 0x%04x & 0x0100) != 0 \n", unCommand);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
    rstatus = g_pRTUStatusTable->SendSWCPollingCommand(SWC_POLLING_INHIBIT,
                (unCommand & 0x00FF));
    if(rstatus != E_ERR_Success)
    {
      return 0xFFFF;
    }
  }
  else
  {
    return 0xFFFF;
  }

  return 0;
}// SetSWCPollingFlag
/*------------------------------------------------------------------------------

Purpose:
  Check user name and password validity

Parameter:
  sUserName   [in] username
  sPassword   [in] password

Return:
  TRUE:  Login OK
  FALSE:  Login failed

History:

  Name          Date         Remark
  ----          ----         ------
 Bryan Chong  08-Jan-2010  Update to fix invalid access by checking on the
                           input length instead of the configuration length
                           [PR22]
------------------------------------------------------------------------------*/
E_ERR_T CMMM::CheckLogin(CHAR *sUserName,  CHAR *sPassword)
{
  int nI;

  #if ((defined CFG_DEBUG_MSG) && (defined _CFG_DEBUG_MMM))
  printf("[MMM] CMMM::CheckLogin, username: %s(%d), password: %s(%d)\n",
          sUserName, strlen(sUserName), sPassword, strlen(sPassword));
  #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_MMM))
  for(nI=0; nI < g_tRTUConfig.nVDUUserNumber; nI++)
  {

    #if ((defined CFG_DEBUG_MSG) && (defined _CFG_DEBUG_MMM))
    printf("[MMM] CMMM::CheckLogin, config username: %s(%d), password: "
           "%s(%d)\n",
           g_tRTUConfig.tVDUNameList[nI].acVDUName,
           strlen(g_tRTUConfig.tVDUNameList[nI].acVDUName),
           g_tRTUConfig.tVDUNameList[nI].acVDUPassword,
           strlen(g_tRTUConfig.tVDUNameList[nI].acVDUPassword));
    #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_MMM))

    if ((strncmp(g_tRTUConfig.tVDUNameList[nI].acVDUName, sUserName,
         strlen(sUserName)) == 0) &&
        (strncmp(g_tRTUConfig.tVDUNameList[nI].acVDUPassword, sPassword,
         strlen(sPassword)) == 0))
    {
      #if ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_MMM))
      printf("[MMM] CMMM::CheckLogin, login valid\n");
      #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_MMM))
      return E_ERR_Success;
    }
  }
  #ifdef CFG_PRN_ERR
  printf("ERR  [MMM] CMMM::CheckLogin, invalid username or password\n");
  #endif // CFG_PRN_ERR
  return E_ERR_MMM_InvalidUsernamePassword;
}// CheckLogin

/*------------------------------------------------------------------------------
Purpose:
  Check state and perform serial port loop back test
Parameter:
  acPortName  [in] serial port name

Return:
  0    -- Test result error.
  1    -- Test result OK.
  0xFFFF  -- Command error or command cannot execute.

History:

  Name          Date         Remark
  ----          ----         ------
 Bryan Chong  22-Oct-2010  Update to fix serial loop back test. [PR78]

------------------------------------------------------------------------------*/
E_ERR_T CMMM::SerialComLoopBackTest(E_SER_COM comport_index)
{
  E_ERR_T rstatus = E_ERR_Success;
  UINT16  cnt = 0;
  UINT8  ucrxbuff;  //bytes to send and receive
  INT32  rxcnt = 0;
  struct tRMM_MSG tRMMMessageSend;  //the structure for message Q that send to RMM.
  UINT16 testbytelen;
  //char cBuff[2048];  //040705 Yu Wei

  // Disable loop back test when system is in Primary, PtoS, StoP, and
  // initialization states
  if ((g_tRTUStatus.nRTUStateFlag == STATEFLAG_PRIMARY) ||
      (g_tRTUStatus.nRTUStateFlag == STATEFLAG_SWITCHING_PTOS) ||
      (g_tRTUStatus.nRTUStateFlag == STATEFLAG_SWITCHING_STOP) ||
      (g_tRTUStatus.nRTUStateFlag == STATEFLAG_INITIALIZATION))
    return E_ERR_MMM_InvalidStateForTest;


  if(g_tRTUStatus.nRTUStateFlag == STATEFLAG_STANDBY)
  {
    //Ask RMM change state to STATEFLAG_HARDWARETEST
    tRMMMessageSend.nDeviceID = MMM_MAIN_TASK_ID_INDEX;
    tRMMMessageSend.nMessageID = MMM_STATE_CHANGE;
    tRMMMessageSend.unStatus = STATEFLAG_HARDWARETEST;
    g_pRTUStatusTable->SendMsg(tRMMMessageSend);
  }


  if ((pSER_CB[comport_index].fd == TYP_NULL) ||
      (pSER_CB[comport_index].fd < TYP_NULL))
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [MMM]CMMM::SerialComLoopBackTest, invalid fd, %d\n",
           pSER_CB[comport_index].fd);
    #endif // CFG_PRN_ERR
    return E_ERR_MMM_InvalidSerialComFd;
  }

  cnt = 0;
  testbytelen = sizeof(loopBackTestByte);
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
  printf("[MMM] CMMM::SerialComLoopBackTest, com %d fd %d tx "
         "total test byte = %d\n",
         comport_index, pSER_CB[comport_index].fd, testbytelen);
  #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_MMM))

  // 20101022 BC
  tcflush(pSER_CB[comport_index].fd, TCIOFLUSH);
  while(cnt < testbytelen)
  {

    rstatus = SER_SendChar((E_SER_COM)comport_index,
                           (UINT8 *)&loopBackTestByte[cnt], 1);
    SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

    rstatus = SER_RxChar((E_SER_COM)comport_index, &ucrxbuff, 1, &rxcnt, 3000);
    SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
    printf("[MMM] CMMM::SerialComLoopBackTest, com %d tx and rx bytes "
           "0x%02x, 0x%02x\n",
           comport_index, loopBackTestByte[cnt], ucrxbuff);
    #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_MMM))
    if (loopBackTestByte[cnt] != ucrxbuff)
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MMM)
      printf("[MMM] CMMM::SerialComLoopBackTest, com %d tx and rx bytes "
             "mismatch, 0x%02x, 0x%02x\n",
             comport_index, loopBackTestByte[cnt], ucrxbuff);
      #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_MMM))
      return E_ERR_MMM_TxAndRxCharNotMatch;
    }
    cnt++;
  }

  return rstatus;
}// SerialComLoopBackTest


/*
Purpose:
  Check state and perform memory test
Return:
  0    -- Test result error.
  1    -- Test result OK.
  0xFFFF  -- Command error or command cannot execute.
*/
unsigned short CMMM::MemoryTest()    //040611 Yu Wei
{
  int nResult;                  //test result value
  int n, i;                    //general index
  unsigned char *pucMemory[8];          //memory pointers
  unsigned char *puc;                //pointer to a byte

  struct tRMM_MSG tRMMMessageSend;  //the structure for message Q that send to RMM.


  if(g_tRTUStatus.nRTUStateFlag == STATEFLAG_STANDBY)  //Modified to reduce complexity. 040322 Yu Wei.
  {    //Ask RMM change state to STATEFLAG_HARDWARETEST.
    tRMMMessageSend.nDeviceID = MMM_MAIN_TASK_ID_INDEX;
    tRMMMessageSend.nMessageID = MMM_STATE_CHANGE;
    tRMMMessageSend.unStatus = STATEFLAG_HARDWARETEST;
    g_pRTUStatusTable->SendMsg(tRMMMessageSend);

    //Before change to STATEFLAG_HARDWARETEST, must close all SWC object.  //040611 Yu Wei
    //if (g_pClock != NULL)    //040705 Yu Wei
    //  StopClockTask();    //040705 Yu Wei

    //StopAllSWC();
    //delay(SYS_CLOCK_SECOND);  //Wait for object closing.

    //040611 Yu Wei
  }

  if ((g_tRTUStatus.nRTUStateFlag == STATEFLAG_PRIMARY) ||
    (g_tRTUStatus.nRTUStateFlag == STATEFLAG_SWITCHING_PTOS) ||  //040322 Yu Wei
    (g_tRTUStatus.nRTUStateFlag == STATEFLAG_SWITCHING_STOP) ||  //040322 Yu Wei
    (g_tRTUStatus.nRTUStateFlag == STATEFLAG_INITIALIZATION))
  {    //This three state cannot test memory.
    return 0xFFFF;
  }
  else  //Test memory.
  {
    m_ucSendBuffer[3] = 0;
    nResult = 1;      //default test result OK

    //allocate memory
    for (n=0; n<8; n++)
    {
      pucMemory[n] = (unsigned char *)malloc(1024*1024);

      if (pucMemory[n] == TYP_NULL)
      {
        break;
      }

      //test memory
      puc = pucMemory[n];

      for (i=0; i<1024*1024; i++)
      {
        //test 0x55
        *puc = 0x55;
        if (*puc != 0x55)
        {
          nResult = 0;
          break;
        }

        //test 0xaa
        *puc = 0xaa;
        if (*puc != 0xaa)
        {
          nResult = 0;
          break;
        }

        puc++;
      }

      if (nResult == 0)  //stop if test failed
      {
        break;
      }
    }
    m_ucSendBuffer[4] = nResult;
    return 1;
  }
}

