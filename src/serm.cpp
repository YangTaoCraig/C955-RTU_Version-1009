/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics Ltd.

 PROPRIETARY RIGHTS of ST Electronics Ltd are involved in the subject matter
 of this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  serm.cpp                                             D1.0.4

 COMPONENT

  SER - Serial Link Management

 DESCRIPTION

  This file consists of management routines for Serial Link Management

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

  Bryan Chong    08-Sep-2009        Initial revision

----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <mqueue.h>
#include <fixerrno.h>
#include <termios.h>
#include <sys/select.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

//#include <pthread.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "sys_ass.h"
#include "ser_def.h"
#include "ser_ext.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"

/*----------------------------------------------------------------------------
  Public Prototypes declaration
----------------------------------------------------------------------------*/
//#ifdef CFG_ENABLE_SPA
//VOID *SER_ReceiveHostCommandThrdFunc (VOID *pThreadParameter);
//#endif // CFG_ENABLE_SPA
//VOID *SER_TransmitResultThrdFunc (VOID *pThreadParameter);
E_ERR_T SER_SendMsg(E_SER_COM comport, CHAR *pmsg);
E_ERR_T SER_SetAttribute(E_SER_COM comport, speed_t unbaudrate,
                        E_SER_PARITY parity, E_SER_DATABIT databit,
                        E_SER_STOPBIT stopbit);
E_ERR_T SER_SendChar(const E_SER_COM comport, UINT8 *pbuff, UINT32 txsz);
E_ERR_T SER_RxChar  (const E_SER_COM comport, UINT8 *poutbuff,
                    const UINT32 rxpacket_sz, INT32 *poutrxsz,
                    const INT32 timeout_ms);

E_ERR_T SER_SendWithReply(INT32 nfd, const VOID *ptxbuff,
                         const UINT32 txpacket_sz, VOID *prxbuff,
                         const UINT32 rxpacket_sz, INT32 *prxdata_sz,
                         INT32 timeout_ms);
E_ERR_T SER_ReceiveMsg(INT32 nfd, VOID *prxbuff, INT32 *prxdata_sz,
                      const UINT32 rxpacket_sz, INT32 timeout_ms);
// Yang Tao
extern bool RequestAndReceiveFlag;
extern long int Sendcount;
extern long int Recvcount;
// Yang Tao Add in One Mutex
pthread_mutex_t sendwithreply = PTHREAD_MUTEX_INITIALIZER;
/*----------------------------------------------------------------------------
  Private Prototypes declaration
----------------------------------------------------------------------------*/
static E_ERR_T ser_getFileDescriptor(E_SER_COM comport, INT32 *poutfd);

//#ifdef CFG_ENABLE_SPA
///*-----------------------------------------------------------------------------
//
// PUBLIC ROUTINE
//
//  SER_ReceiveHostCommandThrdFunc
//
// DESCRIPTION
//
//  This routine will manage the thread to receive SCPI command
//  Special input key from host:
//
//  Up    0x1b 5b 41
//  Down  0x1b 5b 42
//  Right 0x1b 5b 43
//  Left  0x1b 5b 44
//  Esc   0x1b
//
// CALLED BY
//
//  Application main
//
// CALLS
//
//  [TBD]
//
// PARAMETER
//
//  None
//
// RETURN
//
//   E_ERR_Success      the routine executed successfully
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
//   Bryan Chong    08-Sep-2009      Created initial revision
//   Bryan Chong    20-May-2010      Update to handle input command size exceed
//                                   CFG_SPA_INPUT_CMD_SZ receiving buffer size
//-----------------------------------------------------------------------------*/
//VOID *SER_ReceiveHostCommandThrdFunc (VOID *pThreadParameter)
//{
//  E_ERR_T rstatus = E_ERR_Success;
//  INT32 rcnt = 0;
//  INT32  nfd;
//  CHAR acrxbuffer[SER_RX_BUFF_SZ];
//  CHAR *prxbuff, *prxbuffstart;
//  CHAR accmdbuffer[SER_CMD_BUFF_SZ];
//  CHAR *pcmdbuff, *pcmdbuffstart;
//  BOOL_T bisFoundTermChar;
//  //INT32 nconnectid = 0;
//  INT32 msglen = 0;
//  UINT32 uncmd_char_cnt = 0;
//  UINT8 histcnt = 0;
//
//  memset(acrxbuffer, 0, sizeof(acrxbuffer));
//  memset(accmdbuffer, 0, sizeof(acrxbuffer));
//
//  prxbuff = prxbuffstart = acrxbuffer;
//  pcmdbuff = pcmdbuffstart = accmdbuffer;
//
//  nfd = pSER_CB[SER_SCPICMD_PORT].fd;
//  rstatus = SER_SendMsg(SER_SCPICMD_PORT, (CHAR *)"\r\n");
//
//  // Setup channel connection
//  /*
//  nconnectid = ConnectAttach(0, 0,
//    pSYS_CB->chnl_ctrl[E_SYS_CHL_SerRxCmdToSPAParseCmd].chid,
//    _NTO_SIDE_CHANNEL, 0);
//    */
//
//  while(1)
//  {
//    rcnt = read(nfd, acrxbuffer, sizeof(acrxbuffer));
//    //rcnt = readcond(nfd, acrxbuffer, sizeof(acrxbuffer), 1, 0, 0);
//    if(rcnt < 0)
//    {
//      #ifdef CFG_PRN_ERR
//      printf("ERR  [SER] SER_ReceiveHostCommandThrdFunc, read serial com "
//             "error\n");
//      perror("ERR  [SER] SER_ReceiveHostCommandThrdFunc, perror");
//      #endif // CFG_PRN_ERR
//    }
//    prxbuff = acrxbuffer;
//    if(rcnt > SER_RX_BUFF_SZ)
//    {
//      #ifdef CFG_PRN_ERR
//      printf("ERR  [SER] SER_ReceiveHostCommandThrdFunc, receive count exceed "
//             "buffer size\n");
//      #endif // CFG_PRN_ERR
//    }
//    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SER_HOSTCMD)
//    printf("[SER] SER_ReceiveHostCommandThrdFunc, rcv: %s, rcnt: %d\n",
//            acrxbuffer, rcnt);
//    SYS_PrnDataBlock((const UINT8 *) acrxbuffer, rcnt, 10);
//    #endif // CFG_DEBUG_MSG
//
//    prxbuff = acrxbuffer;
//    // echo characters until null character
//    while(*prxbuff != '\0')
//    {
//
//      if(pcmdbuff < pcmdbuffstart)
//      {
//        #ifdef CFG_PRN_ERR
//        printf("ERR  [SER] SER_ReceiveHostCommandThrdFunc, pcmd is out of "
//               "bound error\n");
//        perror("ERR  [SER] SER_ReceiveHostCommandThrdFunc, perror");
//        #endif // CFG_PRN_ERR
//        pcmdbuff = pcmdbuffstart;
//      }
//      *pcmdbuff = *prxbuff;
//      uncmd_char_cnt++;
//      pcmdbuff++;
//
//      switch(*prxbuff)
//      {
//        case CFG_FIRST_TERM_CHAR: // '\r' handling
//          #ifdef CFG_ENABLE_PRN_TERM_CHAR
//          rstatus = SER_SendMsg(SER_SCPICMD_PORT, (CHAR *)"\\r\r\n");
//          #else
//          rstatus = SER_SendMsg(SER_SCPICMD_PORT, (CHAR *)"\r\n");
//          #endif // CFG_ENABLE_PRN_TERM_CHAR
//          strcat(accmdbuffer, "\r\n");
//          bisFoundTermChar = E_TYPE_True;
//          break;
//
//        case CFG_TERM_CHAR_NEWLINE: // '\n' handling
//          #ifdef CFG_ENABLE_PRN_NEWLINE_CHAR
//          rstatus = SER_SendMsg(SER_SCPICMD_PORT, (CHAR *)"\\n\r\n");
//          #else
//          rstatus = SER_SendMsg(SER_SCPICMD_PORT, (CHAR *)"\r\n");
//          #endif // CFG_ENABLE_PRN_NEWLINE_CHAR
//          break;
//
//        case '\b':  // backspace handling
//          if(uncmd_char_cnt == 1)
//          {
//            pcmdbuff--;
//            *pcmdbuff = '\0';
//            uncmd_char_cnt--;
//            rstatus = SER_SendMsg(SER_SCPICMD_PORT, (CHAR *)"\b \b");
//          }else {
//            pcmdbuff--;
//            *pcmdbuff = '\0';
//            pcmdbuff--;
//            *pcmdbuff = '\0';
//            uncmd_char_cnt--;
//            rstatus = SER_SendMsg(SER_SCPICMD_PORT, (CHAR *)"\b \b");
//          }
//          break;
//
//        default:
//          // echo back the command string
//          rstatus = SER_SendMsg(SER_SCPICMD_PORT, prxbuff);
//
//      } // switch(*prxbuff)
//
//      prxbuff++;
//    } // while(*prxbuff != '\0')
//
//    memset(acrxbuffer, 0, sizeof(acrxbuffer));
//
//    if(bisFoundTermChar == E_TYPE_True)
//    {
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SER_HOSTCMD)
//      printf("[SER] SER_ReceiveHostCommandThrdFunc, recv %d bytes cmd:\n",
//        strlen(accmdbuffer), accmdbuffer);
//      SYS_PrnDataBlock((const UINT8 *) accmdbuffer, strlen(accmdbuffer), 10);
//      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SER))
//
//      // check for special keys
//      if((accmdbuffer[0] == 0x1b) && (accmdbuffer[1] == 0x5b) &&
//         (strlen(accmdbuffer) > 3))
//      {
//        switch(accmdbuffer[2])
//        {
//          case 0x41: // Up
//            #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SER_HOSTCMD)
//            printf("[SER] SER_ReceiveHostCommandThrdFunc, rx Up key\n");
//            #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SER_HOSTCMD)
//            memset(accmdbuffer, 0, sizeof(accmdbuffer));
//            histcnt = pSER_CB->nextEmptyHistoryId - 1;
//            sprintf(accmdbuffer, pSER_CB->history[histcnt]);
//            uncmd_char_cnt = strlen(pSER_CB->history[histcnt]);
//            break;
//
//          case 0x42:
//            #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SER_HOSTCMD)
//            printf("[SER] SER_ReceiveHostCommandThrdFunc, rx Down key\n");
//            #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SER_HOSTCMD)
//            break;
//
//          case 0x43:
//            #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SER_HOSTCMD)
//            printf("[SER] SER_ReceiveHostCommandThrdFunc, rx Right key\n");
//            #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SER_HOSTCMD)
//            break;
//
//          case 0x44:
//            #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SER_HOSTCMD)
//            printf("[SER] SER_ReceiveHostCommandThrdFunc, rx Left key\n");
//            #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SER_HOSTCMD)
//            break;
//
//          default:
//            #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SER_HOSTCMD)
//            printf("[SER] SER_ReceiveHostCommandThrdFunc, rx unknown key\n");
//            #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SER_HOSTCMD)
//            break;
//
//
//        }// switch(prxbuff[2])
//      }
//
//      if((accmdbuffer[0] == 0x1b) && (strlen(accmdbuffer) == 3))
//      {
//        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SER_HOSTCMD)
//        printf("[SER] SER_ReceiveHostCommandThrdFunc, rx Esc key\n");
//        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SER_HOSTCMD)
//      }
//
//      if(strlen(accmdbuffer) > CFG_SPA_INPUT_CMD_SZ)
//      {
//        #ifdef CFG_PRN_WARN
//        printf("WARN [SER] SER_ReceiveHostCommandThrdFunc, cmd size %d exceed "
//               "SPA buffer size %d\n",
//               strlen(accmdbuffer), CFG_SPA_INPUT_CMD_SZ);
//        #endif // CFG_PRN_WARN
//        memset(accmdbuffer, 0, sizeof(accmdbuffer));
//        uncmd_char_cnt = 0;
//        pcmdbuff = accmdbuffer;
//        bisFoundTermChar = E_TYPE_False;
//        continue;
//      }
//
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SER_HOSTCMD)
//      printf("[SER] SER_ReceiveHostCommandThrdFunc, tx cmd to msgq %d bytes\n",
//             strlen(accmdbuffer));
//      SYS_PrnDataBlock((const UINT8 *) accmdbuffer, strlen(accmdbuffer), 20);
//      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SER_HOSTCMD)
//
//      // Send rx command to message queue then clear buffer
//      msglen = mq_send(pSYS_CB->msq_ctrl[E_SYS_MSQL_SerCmdToSPAParseCmd].msq_id,
//                       accmdbuffer, strlen(accmdbuffer), SYS_MSQ_PRIOR_NORMAL);
//      if(msglen < 0)
//      {
//        #ifdef CFG_PRN_ERR
//        printf("[SER]: ERR SER_ReceiveHostCommandThrdFunc, mq_send fail, "
//               "%s\n", strerror(errno));
//        #endif // CFG_PRN_ERR
//      } // if(msglen < 0)
//
//      // save to history
//      strcpy(pSER_CB->history[pSER_CB->nextEmptyHistoryId], accmdbuffer);
//      if(pSER_CB->nextEmptyHistoryId >= SER_HISTORY_TOTAL_MSG)
//      {
//        pSER_CB->nextEmptyHistoryId = 0;
//      }else{
//        pSER_CB->nextEmptyHistoryId++;
//      }
//
//      // clear command buffer and reset pointer
//      memset(accmdbuffer, 0, sizeof(accmdbuffer));
//      uncmd_char_cnt = 0;
//      pcmdbuff = accmdbuffer;
//      bisFoundTermChar = E_TYPE_False;
//    } //  if(bisFoundTermChar == E_TYPE_True)
//  }// while (1)
//
//  //return E_ERR_Success;
//} // SER_ReceiveSCPICommandThrdFunc
///*-----------------------------------------------------------------------------
//
// PUBLIC ROUTINE
//
//  SER_TransmitResultThrdFunc
//
// DESCRIPTION
//
//  This routine will manage the transmission of result to host
//
// CALLED BY
//
//  Application main
//
// CALLS
//
//  [TBD]
//
// PARAMETER
//
//  None
//
// RETURN
//
//   E_ERR_Success      the routine executed successfully
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
//   Bryan Chong    11-Sep-2009      Created initial revision
//
//-----------------------------------------------------------------------------*/
//VOID *SER_TransmitResultThrdFunc (VOID *pThreadParameter)
//{
//  E_ERR_T rstatus = E_ERR_Success;
//  struct msq_ctrl_st *pcb;
//  struct mq_attr  mqstat;
//  INT32 msglen;
//  CHAR acrxmsg[CFG_SPA_OUTPUT_BUFF_SZ] = {0};
//
//  pcb = &pSYS_CB->msq_ctrl[E_SYS_MSQL_SPAResultToSerOutput];
//
//  // get message queue attribute
//  mq_getattr(pcb->msq_id, &mqstat);
//
//  while(1)
//  {
//    // receiving SPA result through message queue
//    msglen = mq_receive(pcb->msq_id, acrxmsg, mqstat.mq_msgsize, NULL);
//
//    if(msglen < 0)
//    {
//      #ifdef CFG_PRN_ERR
//      printf("ERR  [SER] SER_TransmitResultThrdFunc, mq_receive fail\n");
//      #endif // CFG_PRN_ERR
//      //continue;
//    }
//
//    rstatus = SER_SendMsg(SER_SCPICMD_PORT, acrxmsg);
//    rstatus = SER_SendMsg(SER_SCPICMD_PORT, (CHAR *)"\r\n\0");
//
//    memset(acrxmsg, 0, sizeof(acrxmsg));
//  } // while(1)
//} // SER_TransmitResultThrdFunc
//#endif // CFG_ENABLE_SPA
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SER_SendMsg

 DESCRIPTION

  This routine will send message to the designated port

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

   Bryan Chong    07-Sep-2009      Created initial revision

-----------------------------------------------------------------------------*/
E_ERR_T SER_SendMsg(E_SER_COM comport, CHAR *pmsg)
{
  E_ERR_T rstatus = E_ERR_Success;
  INT32 rcnt;
  INT32 len = 0;
  INT32 fd;


  if(pmsg == TYP_NULL)
    return E_ERR_InvalidNullPointer;

  rstatus = ser_getFileDescriptor(comport, &fd);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  len = strlen(pmsg);
  rcnt = write(fd, pmsg, len);

  #if ((defined CFG_DEBUG_MSG) && (defined _CFG_DEBUG_SER))
  printf("[SER] SER_SendMsg, comport %d, fd(%d) tx %d bytes, pmsg[0] = 0x%x\n",
         comport, fd, len, pmsg[0]);
  #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SER))

  if (rcnt != len)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SER] SER_SendMsg, failed to write to serial port %d\n",
      comport);
    #endif // CFG_PRN_ERR
    return E_ERR_SER_OpenWriteError;
  }
  #if ((defined CFG_DEBUG_MSG) && (defined _CFG_DEBUG_SER))
  printf("[SER] SER_SendMsg(com %d), return length = %d\n",
          comport, rcnt);
  for(UINT32 cnt = 0; cnt < strlen(pmsg); cnt++)
  {
    printf("%c(0x%02x)  ", pmsg[cnt], pmsg[cnt]);
    if((cnt + 1)%10 == 0)
      printf("\n");
  }
  printf("\n");
  #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SER))

  return E_ERR_Success;
} // SER_SendMsg
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SER_SetAttribute

 DESCRIPTION

  This routine will send message to the designated port

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

   Bryan Chong    07-Sep-2009      Created initial revision

-----------------------------------------------------------------------------*/
E_ERR_T SER_SetAttribute(E_SER_COM comport, speed_t unbaudrate,
                        E_SER_PARITY parity, E_SER_DATABIT databit,
                        E_SER_STOPBIT stopbit)
{
  E_ERR_T rstatus = E_ERR_Success;
  struct termios termios_p;
  //speed_t baud = unbaudrate;
  INT32 nfd;

  rstatus = ser_getFileDescriptor(comport, &nfd);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  //get current settings
  if((tcgetattr(nfd, &termios_p)) < 0)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SER] SER_SetAttribute, get attr fail, %s\n",
      strerror(errno));
    #endif // CFG_PRN_ERR
    return E_ERR_SER_GetAttributeFail;
  }
// Yang Tao : If want to conduct Hardware test, must enable below setting.
 // In command prompt use stty command and qtalk command
//#ifdef NOT_USED
  // ignore modem status lines, enable receiver, hang up on last close
  termios_p.c_cflag =  (CLOCAL|CREAD|HUPCL);
  // disable hardware input flow control
  termios_p.c_cflag &= (~IHFLOW);
  // disable hardware output flow control
  termios_p.c_cflag &= (~OHFLOW);
 //#endif // NOT_USED
  //set input baud rate
  if((cfsetispeed(&termios_p, (speed_t)unbaudrate)) < 0)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SER] SER_SetAttribute, fail to set input baudrate"
           " to %d, %s\n", unbaudrate, strerror(errno));
    #endif // CFG_PRN_ERR
    return E_ERR_SER_SetInputBaudRateFail;
  }
  #ifdef CFG_DEBUG_MSG
  printf("[SER] SER_SetAttribute, input com %d baudrate: %d\n",
    (UINT32)comport, (UINT32)cfgetispeed(&termios_p));
  #endif // CFG_DEBUG_MSG
  //set output baud rate
  if((cfsetospeed(&termios_p, (speed_t)unbaudrate)) < 0)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SER] SER_SetAttribute, fail to set output baudrate to "
           "%d, %s\n", unbaudrate, strerror(errno));
    #endif // CFG_PRN_ERR
    return E_ERR_SER_SetOutputBaudRateFail;
  }
  #ifdef CFG_DEBUG_MSG
  printf("[SER] SER_SetAttribute, output com %d baudrate: %d\n",
    (UINT32)comport, (UINT32)cfgetospeed(&termios_p));
  #endif // CFG_DEBUG_MSG
  //set parity
  switch(parity)
  {
    case E_SER_PARITY_Disable:  //parity disables
      termios_p.c_cflag &= (~PARENB);
      termios_p.c_cflag &= (~PARODD);
      break;

    case E_SER_PARITY_Odd:  //odd parity
      termios_p.c_cflag |= (PARENB | PARODD);
      //termios_p.c_cflag |= PARODD;
      break;

    case E_SER_PARITY_Even:  //even parity
      termios_p.c_cflag |= PARENB;
      termios_p.c_cflag &= (~PARODD);
      break;

    default: //set default to parity disable
      termios_p.c_cflag &= (~PARENB);
      termios_p.c_cflag &= (~PARODD);
      break;
  }// switch(parity)

  termios_p.c_cflag &= (~CSIZE);

  //set data bit
  switch(databit)
  {
    case E_SER_DATABIT_5:
      termios_p.c_cflag |= CS5;
      break;

    case E_SER_DATABIT_6:
      termios_p.c_cflag |= CS6;
      break;

    case E_SER_DATABIT_7:
      termios_p.c_cflag |= CS7;
      break;

    case E_SER_DATABIT_8:
      termios_p.c_cflag |= CS8;
      break;

    default:
      #ifdef CFG_PRN_ERR
      printf("ERR  [SER] SER_SetAttribute, invalid databits\n");
      #endif // CFG_PRN_ERR
      return E_ERR_SER_InvalidDataBitsSelection;
  }

  //set stop bit
  switch(stopbit)
  {
    case E_SER_STOPBIT_1:
      termios_p.c_cflag &= (~CSTOPB);
      break;

    case E_SER_STOPBIT_2:
      termios_p.c_cflag |= CSTOPB;
      break;

    default:
      #ifdef CFG_PRN_ERR
      printf("ERR  [SER] SER_SetAttribute, invalid stopbit\n");
      #endif // CFG_PRN_ERR
      return E_ERR_SER_InvalidStopbitSelection;
  } // switch(stopbit)

  if((tcsetattr(nfd, TCSANOW, &termios_p)) < 0)
  //if((tcsetattr(nfd, TCSADRAIN, &termios_p)) < 0)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SER] SER_SetAttribute, set attribute fail, %s\n",
      strerror(errno));
    #endif // CFG_PRN_ERR
    return E_ERR_SER_SetAttributeFail;
  }

  return E_ERR_Success;
} // SER_SetAttribute

/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SER_SendChar

 DESCRIPTION

  This routine will send character to the designated port

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

   Bryan Chong    11-Feb-2010      Created initial revision

-----------------------------------------------------------------------------*/
E_ERR_T SER_SendChar(const E_SER_COM comport, UINT8 *pbuff, UINT32 txsz)
{
  E_ERR_T rstatus = E_ERR_Success;
  INT32 fd;
  INT32 rcnt;


  if(pbuff == TYP_NULL)
    return E_ERR_InvalidNullPointer;

  rstatus = ser_getFileDescriptor(comport, &fd);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  //tcflush(fd , TCIOFLUSH);
  rcnt = write(fd, pbuff, txsz);

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SER))
  printf("[SER] SER_SendChar, com %d fd %d tx %d bytes, return length = %d, "
         "tx:\n",
         comport, fd, txsz, rcnt);
  rstatus = SYS_PrnDataBlock(pbuff, txsz, 10);
  #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SER))


  if (rcnt == TYP_ERROR)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SER] SER_SendChar, failed to write to serial port %d, "
           "fd = %d\n",
           comport, fd);
    #endif // CFG_PRN_ERR
    return E_ERR_SER_WriteCharError;
  }

  if(rcnt != (INT32)txsz)
  {
    #ifdef CFG_PRN_WARN
    printf("WARN [SER] SER_SendChar, write return count %d not match with tx "
           " size at serial port %d\n",
           rcnt, txsz, comport);
    #endif // CFG_PRN_WARN
  }

  return E_ERR_Success;
} // SER_SendChar
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SER_SendChar

 DESCRIPTION

  This routine will receive character from the designated port

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

   Bryan Chong    11-Feb-2010      Created initial revision

-----------------------------------------------------------------------------*/
E_ERR_T SER_RxChar(const E_SER_COM comport, UINT8 *poutbuff,
                  const UINT32 rxpacket_sz, INT32 *poutrxsz,
                  const INT32 timeout_ms)
{
  E_ERR_T rstatus = E_ERR_Success;
  UINT32 uncnt;
  INT32 nrxsz = 0;
  INT32 nfd;
  INT32 nrval;
  UINT8 *pwalk;
  UINT8 tmpbuff[100] = {0};
  BOOL_T isRxPacketIncomplete = E_TYPE_No;
  fd_set fdsRead;
  struct timeval tv;
  //struct timespec tspecStart, tspecEnd;


  rstatus = ser_getFileDescriptor(comport, &nfd);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  //tcflush(nfd, TCIOFLUSH);
  tv.tv_sec = timeout_ms * 1e-3;
  tv.tv_usec = (timeout_ms % 1000) * 1e3;

  *poutrxsz = 0;
  pwalk = (UINT8 *)poutbuff;

  while(1)
  {
    FD_ZERO(&fdsRead);
    FD_SET(nfd, &fdsRead);
    nrxsz = 0;

    nrval = select(nfd + 1, &fdsRead, NULL, NULL, &tv);

    switch(nrval)
    {
      case 0:
        #ifdef CFG_PRN_WARN
        printf("WARN [SER] SER_RxChar, com %d fd %d rx time out %d ms, rx data "
               "incomplete.\n", comport, nfd, timeout_ms);
        #endif // CFG_PRN_WARN
        memset(pwalk, 0, rxpacket_sz);
        return E_ERR_SER_SelectReadTimeOut;
        break;

      case TYP_ERROR:
        #ifdef CFG_PRN_ERR
        printf("ERR  [SER] SER_RxChar, select read error, %s\n",
               strerror(errno));
        #endif // ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_NTP))
        return E_ERR_SER_SelectReadError;
        break;

      default:
        if(nrval != E_TYPE_True)
        {
          #ifdef CFG_PRN_ERR
          printf("ERR  [SER] SER_RxChar, invalid nrval for select "
                 "%d\n", nrval);
          #endif // CFG_PRN_ERR
          return E_ERR_SER_InvalidSelectReturnValue;
        }

    } // switch(nrval)

    #if ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_SER))
    printf("[SER] SER_RxChar, select nrval = %d, expect to rx %d "
           "bytes\n", nrval, rxpacket_sz);
    #endif // ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_NTP))

    if(FD_ISSET(nfd, &fdsRead))
      nrxsz = read(nfd, tmpbuff, rxpacket_sz);

    if(nrxsz == TYP_ERROR)
    {
       #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SER))
       printf("[SER] SER_RxChar, receive packet fail, %s\n",
              strerror(errno));
       #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SER))
       return E_ERR_SER_FailToReadDevice;
    }

    #if ((defined CFG_DEBUG_MSG) && (_CFG_DEBUG_SER))
    printf("[SER] SER_RxChar, rxpacket = %d, poutrxsz = %d\n",
           rxpacket_sz, *poutrxsz);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SER))

    // check if receiving data size exceed expected receiving size
    if(rxpacket_sz < (UINT32)(*poutrxsz + nrxsz))
    {
       #ifdef CFG_PRN_WARN
       printf("WARN [SER] SER_RxChar, exceed defined rx size, %d "
              "bytes. Receive size = %d\n",
              rxpacket_sz, nrxsz);
       #endif // CFG_PRN_WARN
       isRxPacketIncomplete = E_TYPE_Yes;
       break;
    }

    for(uncnt = 0; uncnt < (UINT32)nrxsz; uncnt++)
    {
      pwalk[*poutrxsz + uncnt] = tmpbuff[uncnt];
    }
    *poutrxsz += nrxsz;
    memset(tmpbuff, 0, sizeof(tmpbuff));

    #if ((defined CFG_DEBUG_MSG)&& CFG_DEBUG_SER)
    printf("[SER] SER_RxChar, fd %d rx data sz = %d, nrxsz = %d\n",
           nfd, *poutrxsz, nrxsz);
    #endif // ((defined CFG_DEBUG_MSG)&& (defined CFG_DEBUG_SER))

    // receive remaining bytes then exit loop
    if(nrxsz < 8)
    {
      #if ((defined CFG_DEBUG_MSG)&& CFG_DEBUG_SER)
      printf("[SER] SER_RxChar, fd %d rx %d bytes less than 8\n",
             nfd, *poutrxsz);
      #endif // ((defined CFG_DEBUG_MSG)&& (defined _CFG_DEBUG_SER))
      break;
    }// if(nrxsz < 8)
  }//while(1)

  #if ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_SER))
  printf("[SER] SER_RxChar, fd %d rx data complete, %d byte(s):\n",
         nfd, *poutrxsz);
  SYS_PrnDataBlock((const UINT8 *) poutbuff, (const UINT32) *poutrxsz, 10);
  #endif // ((defined CFG_DEBUG_MSG)&& (defined _CFG_DEBUG_SER))

  return E_ERR_Success;
} // SER_RxChar
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SER_SendWithReply

 DESCRIPTION

  This routine will transmit a request packet and wait for the reply.
  The routine will actually read 8 bytes at a time. If the incoming data is more
  than 8 bytes, the read process will loop until the remaining bytes, which
  is less than 8 bytes. Continue reading will cause timeout from select routine,
  and will result in FD_ISSET(nfd, &fdsRead) equal to FALSE.

 CALLED BY

  SYS_Initialization

 CALLS

  SER_SendWithReply
  SYS_SetMutex
  ModbusReplyCheck

 PARAMETER

  nfd          [in]  file descriptor of the LAN connection
  ptxbuff      [in]  pointer to transmit buffer
  txpacket_sz  [in]  transmit packet size in bytes
  prxbuff      [out] pointer to receive buffer
  rxpacket_sz  [in]  expected receive packet size in bytes
  prxdata_sz   [out] pointer to receive data size. Can be less than or equal to
                     receive buffer size
  timeout_ms   [in]  receiving time out in milliseconds

 RETURN

   E_ERR_Success                       the routine executed successfully
   E_ERR_SER_InvalidFileDescriptor     invalid file descriptor
   E_ERR_SER_FailToWriteDevice         fail to write device
   E_ERR_SER_SelectReadTimeOut         select read timeout
   E_ERR_SER_SelectReadError           select read error
   E_ERR_SER_InvalidSelectReturnValue  invalid select return value


 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    25-Feb-2009      Created initial revision.
                                   Resolve polling inaccuracy issue [PR30]
   Bryan Chong    29-Jun-2010      Update to handle read 8 bytes at a time
                                   and use remaining_expect_sz to keep track

-----------------------------------------------------------------------------*/
E_ERR_T SER_SendWithReply(INT32 nfd, const VOID *ptxbuff,
                         const UINT32 txpacket_sz, VOID *prxbuff,
                         const UINT32 rxpacket_sz, INT32 *prxdata_sz,
                         INT32 timeout_ms)
{
  #ifdef CFG_ENABLE_SER_GETDATARATE
  E_ERR_T rstatus = E_ERR_Success;
  #endif // CFG_ENABLE_SER_GETDATARATE
  struct timeval tv;
  fd_set fdsRead;
  INT32 nrval;
  UINT8 tmpbuff[1000] = {0};
  INT32 nrxsz = 0;
  INT32 remainsz=0;// this parameter is to record when actual receieve length
  // bigger than the expected
  UINT32 remaining_expect_sz = rxpacket_sz;
  BOOL_T isRxPacketIncomplete = E_TYPE_No;
  UINT32 uncnt;
  UINT8 *pwalk;

  #ifdef CFG_ENABLE_SER_GETDATARATE
  struct timespec tspecStart, tspecEnd;
  #endif // CFG_ENABLE_SER_GETDATARATE

  if((nfd == TYP_NULL) || (nfd == TYP_ERROR))
   return E_ERR_SER_InvalidFileDescriptor;

  // flush input and output data
  tcflush(nfd, TCIOFLUSH);

  nrval = write(nfd, ptxbuff, txpacket_sz);

  #if ((defined CFG_DEBUG_MSG)&& (_CFG_DEBUG_SER))
  printf("[SER] SER_SendWithReply, nfd %d tx packet %d bytes\n", nfd, nrval);
  SYS_PrnDataBlock((const UINT8 *)ptxbuff, nrval, 10);
  #endif // ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_SER))
  if(nrval == TYP_ERROR)
  {
    #if ((defined CFG_DEBUG_MSG) && (_CFG_DEBUG_SER))
    printf("[SER] SER_SendWithReply, tx packet fail, %s\n", strerror(errno));
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_NTP))
    return E_ERR_SER_FailToWriteDevice;
  }

  // set receiving timeout value
  tv.tv_sec = timeout_ms * 1e-3;
  tv.tv_usec = (timeout_ms % 1000) * 1e3;

  *prxdata_sz = 0;
  pwalk = (UINT8 *)prxbuff;

  // receiving loop
  while(1)
    {
      FD_ZERO(&fdsRead);
      FD_SET(nfd, &fdsRead);
      nrxsz = 0;

      #ifdef CFG_ENABLE_SER_GETDATARATE
      // mark start time
      clock_gettime(CLOCK_REALTIME, &tspecStart);
      #endif // CFG_ENABLE_SER_GETDATARATE

      nrval = select(nfd + 1, &fdsRead, NULL, NULL, &tv);

      switch(nrval)
      {
        case 0:
          #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SER)
          printf("WARN [SER] SER_SendWithReply, rx 0 data, time out %d ms\n",
                 timeout_ms);
          //SYS_PrnDataBlock((const UINT8 *)ptxbuff, 8, 10);
          #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SER)
          memset(pwalk, 0, rxpacket_sz);
          return E_ERR_SER_SelectReadTimeOut;
          break;

        case TYP_ERROR:
          #ifdef CFG_PRN_ERR
          printf("ERR  [SER] SER_SendWithReply, select read error, %s\n",
                 strerror(errno));
          #endif // ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_NTP))
          return E_ERR_SER_SelectReadError;
          break;

        default:
          // TODO: Handle pending data properly
          // return number of pending data
          if(nrval != E_TYPE_True)
          {
            #ifdef CFG_PRN_ERR
            printf("ERR  [SER] SER_SendWithReply, invalid nrval for select "
                   "%d\n", nrval);
            #endif // CFG_PRN_ERR
            return E_ERR_SER_InvalidSelectReturnValue;
          }

      } // switch(nrval)

      #if ((defined CFG_DEBUG_MSG)&& (_CFG_DEBUG_SER))
      printf("[SER] SER_SendWithReply, select nrval = %d, expect to rx %d "
             "bytes\n", nrval, rxpacket_sz);
      #endif // ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_NTP))

      // 20100629 BC
      // the read API can only read 8 bytes at a time, deduct 8 bytes for
      // each read until the remainig size
      if(remaining_expect_sz > 8)
      {
        if(FD_ISSET(nfd, &fdsRead))
        {
          nrxsz = read(nfd, tmpbuff, 8);
          remaining_expect_sz -=8;
        }
      }else{
        if(FD_ISSET(nfd, &fdsRead))
        {
          nrxsz = read(nfd, tmpbuff, remaining_expect_sz);
        }
      }

      //if(FD_ISSET(nfd, &fdsRead))
        //nrxsz = read(nfd, tmpbuff, rxpacket_sz);

      if(nrxsz == TYP_ERROR)
      {
         #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SER))
         printf("[SER] SER_SendWithReply, receive packet fail, %s\n",
                strerror(errno));
         #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_NTP))
         return E_ERR_SER_FailToReadDevice;
      }

      if(rxpacket_sz < (UINT32)(*prxdata_sz + nrxsz))
      {
         //#ifdef CFG_PRN_WARN
         printf("WARN [SER] SER_SendWithReply, exceed defined rx size, %d "
                "bytes\n", rxpacket_sz);
        // #endif // CFG_PRN_WARN
        // isRxPacketIncomplete = E_TYPE_Yes;
         remainsz=rxpacket_sz-*prxdata_sz;

         for(uncnt = 0; uncnt < (UINT32)remainsz; uncnt++)
           {
             pwalk[*prxdata_sz + uncnt] = tmpbuff[uncnt];
           }
           *prxdata_sz += remainsz;

         break;
      }

      for(uncnt = 0; uncnt < (UINT32)nrxsz; uncnt++)
      {
        pwalk[*prxdata_sz + uncnt] = tmpbuff[uncnt];
      }
      *prxdata_sz += nrxsz;
      memset(tmpbuff, 0, sizeof(tmpbuff));

      #if ((defined CFG_DEBUG_MSG)&& (_CFG_DEBUG_SER))
      printf("[SER] SER_SendWithReply, rx data sz = %d, nrxsz = %d\n",
             *prxdata_sz, nrxsz);
      #endif // ((defined CFG_DEBUG_MSG)&& (defined CFG_DEBUG_SER))

      // receive remaining bytes then exit loop
      if((nrxsz < 8) || (nrxsz == (INT32)rxpacket_sz))
      {
        #if ((defined CFG_DEBUG_MSG)&& (_CFG_DEBUG_SER))
        printf("[SER] SER_SendWithReply, rx total %d bytes\n", *prxdata_sz);
        #endif // ((defined CFG_DEBUG_MSG)&& (defined _CFG_DEBUG_SER))
        break;
      }// if(nrxsz < 8)
    }//while(1)

  #ifdef CFG_ENABLE_SER_GETDATARATE
  // mark end time
  clock_gettime(CLOCK_REALTIME, &tspecEnd);
  rstatus = SYS_GetDataRate(&tspecStart, &tspecEnd, (UINT32 *)prxdata_sz,
                  &pSER_CB->latency.bytepersecond);
  SYS_ASSERT_RETURN((rstatus != E_ERR_Success), rstatus);

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SER))
  printf("[SER] SER_SendWithReply, data rate = %.02f bytes per second\n",
           pSER_CB->latency.bytepersecond);
  #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SER))
  #endif // CFG_ENABLE_SER_GETDATARATE

  #if ((defined CFG_DEBUG_MSG)&& CFG_DEBUG_SER)
  printf("[SER] SER_SendWithReply, rx packet complete, total %d bytes: \n",
         *prxdata_sz);
  SYS_PrnDataBlock((const UINT8 *)prxbuff, *prxdata_sz, 10);
  #endif // ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_SER))
  return E_ERR_Success;
}

/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SER_ReceiveMsg

 DESCRIPTION

  This routine will read will actually read 8 bytes at a time. If the incoming
  data is more than 8 bytes, the read process will loop untill the remaining
  bytes, which is less than 8 bytes. Continue reading will cause timeout from
  select routine, and will result in FD_ISSET(nfd, &fdsRead) equal to FALSE.

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  nfd          [in]  file descriptor of the LAN connection
  prxbuff      [out] pointer to receive buffer
  prxdata_sz   [out] pointer to receive data size. Can be less than or equal to
                     receive buffer size
  rxpacket_sz  [in]  expected receive packet size in bytes
  timeout_ms   [in]  receiving time out in milliseconds

 RETURN

   E_ERR_Success                       the routine executed successfully
   E_ERR_SER_InvalidFileDescriptor     invalid file descriptor
   E_ERR_SER_SelectReadTimeOut         select read timeout
   E_ERR_SER_SelectReadError           select read error
   E_ERR_SER_InvalidSelectReturnValue  invalid select return value

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    06-May-2010      Created initial revision.

-----------------------------------------------------------------------------*/
E_ERR_T SER_ReceiveMsg(INT32 nfd, VOID *prxbuff, INT32 *prxdata_sz,
                      const UINT32 rxpacket_sz, INT32 timeout_ms)
{
  struct timeval tv;
  fd_set fdsRead;
  INT32 nrval;
  UINT8 tmpbuff[100] = {0};
  INT32 nrxsz = 0;
  BOOL_T isRxPacketIncomplete = E_TYPE_No;
  UINT32 uncnt;
  UINT8 *pwalk;
  char cLog[100];

  #ifdef CFG_PRN_ERR
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};
  #endif // CFG_PRN_ERR

  if(nfd == TYP_NULL)
   return E_ERR_SER_InvalidFileDescriptor;

  // flush input and output data
  //tcflush(nfd, TCIOFLUSH);

  // set receiving timeout value
  tv.tv_sec = timeout_ms * 1e-3;
  tv.tv_usec = (timeout_ms % 1000) * 1e3;

  *prxdata_sz = 0;
  pwalk = (UINT8 *)prxbuff;

  // receiving loop
  while(1)
  {
    // flush input and output data
    //tcflush(nfd, TCIOFLUSH);

    FD_ZERO(&fdsRead);
    FD_SET(nfd, &fdsRead);
    nrxsz = 0;

    nrval = select(nfd + 1, &fdsRead, NULL, NULL, &tv);

    switch(nrval)
    {
      case 0:
        #if (CFG_PRN_WARN_SER)
        printf("WARN [SER] SER_ReceiveMsg, rx 0 data, time out %d ms\n",
               timeout_ms);
        #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SER)
        memset(pwalk, 0, rxpacket_sz);
       // printf(cLog, "SER_ReceiveMsg, Select timeout %d",timeout_ms);

        return E_ERR_SER_SelectReadTimeOut;
        break;

      case TYP_ERROR:
        #ifdef CFG_PRN_ERR
        printf("ERR  [SER] SER_ReceiveMsg, %s\n"
               "  select read error, %s\n",
               SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
               strerror(errno));
        #endif // CFG_PRN_ERR
        return E_ERR_SER_SelectReadError;
        break;

      default:
        if(nrval != E_TYPE_True)
        {
          #ifdef CFG_PRN_ERR
          printf("ERR  [SER] SER_ReceiveMsg, invalid nrval for select "
                 "%d\n", nrval);
          #endif // CFG_PRN_ERR
          return E_ERR_SER_InvalidSelectReturnValue;
        }

    } // switch(nrval)

    #if ((defined CFG_DEBUG_MSG)&& CFG_DEBUG_SER)
    printf("[SER] SER_ReceiveMsg, select nrval = %d, expect to rx %d "
           "bytes\n", nrval, rxpacket_sz);
    #endif // ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_NTP))

    if(FD_ISSET(nfd, &fdsRead))
      nrxsz = read(nfd, tmpbuff, rxpacket_sz);

    if(nrxsz == TYP_ERROR)
    {
       #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SER))
       printf("[SER] SER_ReceiveMsg, receive packet fail, %s\n",
              strerror(errno));
       #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_NTP))
       return E_ERR_SER_FailToReadDevice;
    }

    if(rxpacket_sz < (UINT32)(*prxdata_sz + nrxsz))
    {
       #ifdef CFG_PRN_WARN
       printf("WARN [SER] SER_ReceiveMsg, exceed defined rx size, %d "
              "bytes. Actual rx sz %d\n",
              rxpacket_sz, (*prxdata_sz + nrxsz));
       #endif // CFG_PRN_WARN
       isRxPacketIncomplete = E_TYPE_Yes;
       break;
    }

    for(uncnt = 0; uncnt < (UINT32)nrxsz; uncnt++)
    {
      pwalk[*prxdata_sz + uncnt] = tmpbuff[uncnt];
    }
    *prxdata_sz += nrxsz;
    memset(tmpbuff, 0, sizeof(tmpbuff));

    #if ((defined CFG_DEBUG_MSG)&& (_CFG_DEBUG_SER))
    printf("[SER] SER_ReceiveMsg, rx data sz = %d, nrxsz = %d\n",
           *prxdata_sz, nrxsz);
    #endif // ((defined CFG_DEBUG_MSG)&& (defined CFG_DEBUG_SER))

    // receive remaining bytes then exit loop
    if((nrxsz < 8) || (nrxsz == (INT32)rxpacket_sz))
    {
      #if ((defined CFG_DEBUG_MSG)&& (_CFG_DEBUG_SER))
      printf("[SER] SER_ReceiveMsg, rx total %d bytes\n", *prxdata_sz);
      #endif // ((defined CFG_DEBUG_MSG)&& (defined _CFG_DEBUG_SER))
      break;
    }// if(nrxsz < 8)
  }//while(1)

  #if ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_SER))
  printf("[SER] SER_ReceiveMsg, fd %d rx packet complete, total %d bytes: \n",
         nfd, *prxdata_sz);
  SYS_PrnDataBlock((const UINT8 *)prxbuff, *prxdata_sz, 10);
  #endif // ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_SER))
  return E_ERR_Success;
}// SER_ReceiveMsg
/*-----------------------------------------------------------------------------

 PRIVATE ROUTINE

  ser_getFileDescriptor

 DESCRIPTION

  This routine will acquire file descriptor for the designated serial port

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

   Bryan Chong    17-Sep-2009      Created initial revision

-----------------------------------------------------------------------------*/
static E_ERR_T ser_getFileDescriptor(E_SER_COM comport, INT32 *poutfd)
{
  if(poutfd == TYP_NULL)
    return E_ERR_InvalidNullPointer;

  switch(comport)
  {
    case E_SER_COM_Port_1:
      *poutfd = pSER_CB[E_SER_COM_Port_1].fd;
      break;

    case E_SER_COM_Port_2:
      *poutfd = pSER_CB[E_SER_COM_Port_2].fd;
      break;

    case E_SER_COM_Port_3:
      *poutfd = pSER_CB[E_SER_COM_Port_3].fd;
      break;

    case E_SER_COM_Port_4:
      *poutfd = pSER_CB[E_SER_COM_Port_4].fd;
      break;

    case E_SER_COM_Port_5:
      *poutfd = pSER_CB[E_SER_COM_Port_5].fd;
      break;

    case E_SER_COM_Port_6:
      *poutfd = pSER_CB[E_SER_COM_Port_6].fd;
      break;

    case E_SER_COM_Port_7:
      *poutfd = pSER_CB[E_SER_COM_Port_7].fd;
      break;

    case E_SER_COM_Port_8:
      *poutfd = pSER_CB[E_SER_COM_Port_8].fd;
      break;

    case E_SER_COM_Port_9:
      *poutfd = pSER_CB[E_SER_COM_Port_9].fd;
      break;

    case E_SER_COM_Port_10:
      *poutfd = pSER_CB[E_SER_COM_Port_10].fd;
      break;

    case E_SER_COM_Port_11:
      *poutfd = pSER_CB[E_SER_COM_Port_11].fd;
      break;

    case E_SER_COM_Port_12:
      *poutfd = pSER_CB[E_SER_COM_Port_12].fd;
      break;

    case E_SER_COM_Port_13:
      *poutfd = pSER_CB[E_SER_COM_Port_13].fd;
      break;

    case E_SER_COM_Port_14:
      *poutfd = pSER_CB[E_SER_COM_Port_14].fd;
      break;

    case E_SER_COM_Port_15:
      *poutfd = pSER_CB[E_SER_COM_Port_15].fd;
      break;

    case E_SER_COM_Port_16:
      *poutfd = pSER_CB[E_SER_COM_Port_16].fd;
      break;

    case E_SER_COM_Port_17:
      *poutfd = pSER_CB[E_SER_COM_Port_17].fd;
      break;

    case E_SER_COM_Port_18:
      *poutfd = pSER_CB[E_SER_COM_Port_18].fd;
      break;

    case E_SER_COM_Port_19:
      *poutfd = pSER_CB[E_SER_COM_Port_19].fd;
      break;

    case E_SER_COM_Port_20:
      *poutfd = pSER_CB[E_SER_COM_Port_20].fd;
      break;
    default:
      return E_ERR_SER_InvalidComPortSelection;
  }

  if(*poutfd == (CHAR)TYP_NULL)
    return E_ERR_SER_InvalidFileDescriptor;

  return E_ERR_Success;
} // ser_getFileDescriptor


