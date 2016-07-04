/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics Ltd.

 PROPRIETARY RIGHTS of ST Electronics Ltd are involved in the subject matter
 of this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  ntpm.cpp                                             D1.0.4

 COMPONENT

  NTP - Network Time Protocol Client

 DESCRIPTION

  This file consists of management routine for NTP.
  The NTP Client is currently support unicast communication only.

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE          REMARKS

  Bryan Chong    26-Jan-2010   Initial revision
  Bryan Chong    01-Apr-2011   Update to add capability to recycle nfd
                               when connection is NG.
                               NTP_LinkManagementThrdFunc [C955 PR94]
  Bryan Chong    05-Apr-2011   update to fix NTP backup server 1 and 2 not
                               able to established connection. This issue
                               resolved through recycling LAN file
                               descriptor for NTP connection.[PR77]
----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <arpa/inet.h>
#include <math.h>
#include <mqueue.h>
#include <sys/socket.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "err_ext.h"
#include "cgf_def.h"
#include "cgf_ext.h"
#include "sys_ass.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
#include "ntp_def.h"
#include "ntp_ext.h"
#include "lnc_def.h"
#include "lnc_ext.h"

/*----------------------------------------------------------------------------
  Public variables declaration
----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
  Public Prototypes declaration
----------------------------------------------------------------------------*/
//E_ERR_T NTP_UpdateRTC(VOID);
VOID *NTP_PollingThrdFunc(VOID *pThreadParameter);
VOID *NTP_LinkManagementThrdFunc(VOID *pThreadParameter);
VOID *NTP_RxCmdTxRespThrdFunc (VOID *pThreadParameter);
//FP64 NTP_Fpst32ToFp64(struct fixedpt32_st sfp);
struct fixedpt32_st NTP_Fp64ToFpst32(FP64 d);
FP64 NTP_Fpst64ToFp64(struct fixedpt64_st lfp);
struct fixedpt64_st NTP_Fp64ToFpst64(FP64 d);
/*----------------------------------------------------------------------------
  Local variables declaration
----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
  Local Prototypes declaration
----------------------------------------------------------------------------*/
static E_ERR_T ntp_McpGetStatus(NTP_MSGQ_MCP_STATUS *pbuff, UINT8 buffsz);
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  NTP_PollingThrdFunc

 DESCRIPTION

  This routine will manage NTP polling session. The client will extract
  timing information from the packet which is t1, t2, t3, and t4. Compute
  the delay and offset. The client will update RTC if offset is exceeding
  the defined spurious threshold, and thereby update the clock to the
  BIOS system.

  Timestamp Name ID When Generated
  --------------------------------
  t1: Originate Timestamp, time request sent by client
  t2: Receive Timestamp, time request received by server
  t3: Transmit Timestamp, time reply sent by server
  t4: Destination Timestamp, time reply received by client

  The roundtrip delay d and local clock offset t are defined as
  d = (t4 - t1) - (t2 - t3)
  t = ((t2 - t1) + (t3 - t4)) / 2.

 CALLED BY

  SYS_Initialization

 CALLS

  None

 PARAMETER

  None

 RETURN

   E_ERR_Success      the routine executed successfully

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    25-Jan-2009      Created initial revision

-----------------------------------------------------------------------------*/
VOID *NTP_PollingThrdFunc(VOID *pThreadParameter)
{
  INT32             chid;
  struct sigevent   event;
  struct itimerspec itime;
  timer_t           timer_id;
  INT32             rcvid;
  NTP_CH_MSG_T      msg;
  E_ERR_T            rstatus;
  #if((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP_POLL)
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP_POLL)
  struct timespec   tspec;
  #if((defined CFG_DEBUG_MSG) || (CFG_DEBUG_NTP))
  struct tm         *tFormatTime;
  #endif // ((defined CFG_DEBUG_MSG) || (CFG_DEBUG_NTP))

  NTP_PACKET_T      tx_ntppack;
  NTP_PACKET_T      rx_ntppack;
  UINT8             ucntp_packet_sz;
  INT32             rlen;

  FP64              t1, t2, t3, t4;
  FP64              tdelay, toffset;
  FP64              tadjust;
  FP64              integer_part;
  timespec          ctspec;

  #ifdef CFG_PRN_WARN
  CHAR              errBuff[100] = {0};
  #endif // CFG_PRN_WARN

  if ((chid = ChannelCreate (0)) == TYP_ERROR)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [NTP] NTP_PollingThrdFunc, fail to create channel, %s\n",
            strerror(errno));
    #endif // CFG_PRN_ERR
    return TYP_NULL;
  }

  pNTP_CB->npolling_channelID = chid;

  // set up the pulse
  event.sigev_notify = SIGEV_PULSE;
  event.sigev_coid =
    ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);
  event.sigev_priority = getprio(0);
  event.sigev_code = E_SYS_PULSE_NTP_Polling;
  timer_create(CLOCK_REALTIME, &event, &timer_id);

  pNTP_CB->polling_timer_id = timer_id;

  while(1)
  {
    rcvid = MsgReceive (chid, &msg, sizeof (msg), NULL);

    if (rcvid < 0)
    {
       //gotAMessage (rcvid, &msg.msg);
       #ifdef CFG_PRN_ERR
       printf("ERR  [NTP] NTP_PollingThrdFunc, error, %s\n",
               strerror(errno));
       #endif //CFG_PRN_ERR
    }

    if (rcvid > 0)
    {
       //gotAMessage (rcvid, &msg.msg);
       #if((defined CFG_DEBUG_MSG) || (CFG_DEBUG_NTP))
       printf("[NTP] NTP_LinkManagementThrdFunc, currently not support message"
              "handling\n");
       #endif //((defined CFG_DEBUG_MSG) || (CFG_DEBUG_NTP))
    }

    // determine where the message came from
    if (msg.pulse.code != E_SYS_PULSE_NTP_Polling)
    {
      #ifdef CFG_PRN_WARN
      printf("WARN [NTP] NTP_PollingThrdFunc, unidentified pulse code"
             "%d\n", msg.pulse.code);
      #endif // CFG_PRN_WARN
      continue;
    }

    #if((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP_POLL)
    rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec, E_SYS_TIME_Local);
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_PRN_WARN
      printf("WARN [NTP] NTP_PollingThrdFunc, get current time fail, %d\n",
              rstatus);
      #endif // CFG_PRN_WARN
      continue;
    }

    printf("[NTP] %d.%02d.%02d %02d:%02d:%02d.%03d NTP_PollingThrdFunc\n",
           (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
           pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
           pbdtime->tm_sec, tspec.tv_nsec/1000000);
    #endif //((defined CFG_DEBUG_MSG) || (CFG_DEBUG_NTP))

    // construct the ntp request packet
    tx_ntppack.flags.allbits = 0xe3;
    tx_ntppack.peerClkStranum = TYP_NULL;
    tx_ntppack.peerPollingInterval = 0x4;
    tx_ntppack.peerClockPrecision = -6;
    tx_ntppack.rootDelay = NTP_Fp64ToFpst32(1.0000);
    tx_ntppack.rootDispersion = NTP_Fp64ToFpst32(1.0000);
    tx_ntppack.referenceClockID = TYP_NULL;
    tx_ntppack.referenceClockUpdateTime = NTP_Fp64ToFpst64(0.0000);
    tx_ntppack.originateTimeStamp = NTP_Fp64ToFpst64(0.0000);
    tx_ntppack.receiveTimeStamp = NTP_Fp64ToFpst64(0.0000);

    // get current time
    clock_gettime(CLOCK_REALTIME, &tspec);
    tx_ntppack.transmitTimeStamp =
      NTP_Fp64ToFpst64(tspec.tv_sec + NTP_JAN_1970 + 1.0e-9 * tspec.tv_nsec);
    ucntp_packet_sz = sizeof(NTP_PACKET_T);

    if(pNTP_CB->pactiveserver == TYP_NULL)
      continue;

    // transmit and receive ntp reply packet
    rstatus = LNC_SendWithReply(
          pNTP_CB->pactiveserver->nfd,
          (const CHAR *)&tx_ntppack,
          ucntp_packet_sz,
          (VOID *)&rx_ntppack,
          ucntp_packet_sz, &rlen, 2000);
    if (rstatus != E_ERR_Success)
    {
      #ifdef CFG_PRN_WARN
      ERR_GetMsgString(rstatus, errBuff);
      printf("ERR  [NTP] NTP_PollingThrdFunc, LNC_SendWithReply error "
             "%s\n", errBuff);
      #endif // CFG_PRN_WARN
      if(rstatus == E_ERR_LNC_SelectReadTimeOut)
      {
        #ifdef CFG_PRN_WARN
        printf("WARN [NTP] NTP_PollingThrdFunc, server %s is down.\n",
                pNTP_CB->pactiveserver->ipaddr);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_NTP))
      }
      pNTP_CB->pactiveserver->isConnectionGood = E_TYPE_No;
      pNTP_CB->pactiveserver = TYP_NULL;
      rlen = 0;

      // disarm timer
      itime.it_value.tv_sec = 0;
      itime.it_value.tv_nsec = 0;
      timer_settime(timer_id, 0, &itime, NULL);
      continue;
    }

    // Get t4, destination timestamp, time reply received by client
    clock_gettime(CLOCK_REALTIME, &tspec);
    t4 = tspec.tv_sec + NTP_JAN_1970 + 1.0e-9 * tspec.tv_nsec;

    // Get t1, originate timestamp, time request sent by client
    t1 = NTP_Fpst64ToFp64(rx_ntppack.originateTimeStamp);

    // Get t2, receive timestamp, time request received by server
    t2 = NTP_Fpst64ToFp64(rx_ntppack.receiveTimeStamp);

    // Get t3, transmit timestamp, time reply sent by server
    t3 = NTP_Fpst64ToFp64(rx_ntppack.transmitTimeStamp);

    // compute the delay and offset in seconds
    tdelay = (t4 - t1) - (t2 - t3);
    toffset = ((t2 - t1) + (t3 - t4))/2;

    // check if delay and offset is within the defined tolerance if not
    // update RTC
    clock_gettime(CLOCK_REALTIME, &tspec);
    tadjust = tspec.tv_sec + NTP_JAN_1970 + 1.0e-9 * tspec.tv_nsec;
    tadjust += toffset;

    #if((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP_POLL)
    tFormatTime = localtime(&tspec.tv_sec);
    printf("[NTP] NTP_PollingThrdFunc, %s:%d fd %d, "
           "%04d-%02d-%02d %02d:%02d:%02d\n"
           "  t1 = %.09f, t2 = %.09f,\n"
           "  t3 = %.09f, t4 = %.09f\n"
           "  delay = %.09f, offset = %.09f tadjust = %.09f\n",
           pNTP_CB->pactiveserver->ipaddr, NTP_SERVER_PORT,
           pNTP_CB->pactiveserver->nfd,
           (tFormatTime->tm_year + 1900), (tFormatTime->tm_mon + 1),
           tFormatTime->tm_mday, tFormatTime->tm_hour, tFormatTime->tm_min,
           tFormatTime->tm_sec, t1, t2, t3, t4, tdelay, toffset, tadjust);
    #endif // ((defined CFG_DEBUG_MSG) || (CFG_DEBUG_NTP))

    // Check if exceed spurious threshold in milliseconds
    if((abs(toffset) * 1e3) < pNTP_CB->threshold)
      continue;

    ctspec.tv_nsec = modf(tadjust, &integer_part) * 1e9;
    ctspec.tv_sec = integer_part - NTP_JAN_1970;

    #if((defined CFG_DEBUG_MSG) || CFG_DEBUG_NTP)
    tFormatTime = localtime(&ctspec.tv_sec);
    printf("[NTP] NTP_PollingThrdFunc, update current time = \n"
           "  sec = %d, nsec = %d\n  %04d-%02d-%02d %02d:%02d:%02d\n",
           ctspec.tv_sec, ctspec.tv_nsec,
           (tFormatTime->tm_year + 1900), (tFormatTime->tm_mon + 1),
           tFormatTime->tm_mday, tFormatTime->tm_hour, tFormatTime->tm_min,
           tFormatTime->tm_sec);
    #endif // ((defined CFG_DEBUG_MSG) || (CFG_DEBUG_NTP))
    if(clock_settime(CLOCK_REALTIME, &ctspec) == TYP_ERROR)
    {
      #ifdef CFG_PRN_ERR
      printf("[NTP] NTP_PollingThrdFunc, set RTC clock fail, error %s\n",
             strerror(errno));
      #endif // CFG_PRN_ERR
    }

    // update current time to hardware BIOS system
    rstatus = SYS_UpdateHardwareRTC();
    if(rstatus != E_ERR_Success)
    {
      #if ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_NTP))
      printf("WARN [NTP] NTP_PollingThrdFunc, update hardware bios RTC fail\n");
      #endif // ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_NTP)
    }

  } // while(1)
  return TYP_NULL;
} // NTP_PollingThrdFunc
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  NTP_LinkManagementThrdFunc

 DESCRIPTION

  This routine will monitor and manage NTP connection. When IP address is 0,
  the relevant file descriptor (nfd) from data structure pNTP_CB->server will
  be set to TYP_NULL(0). When IP address is non-zero, the relevant file
  descriptor (nfd) will be set to TYP_ERROR(-1). The settings can be
  summarized as follows

  Data structure pNTP_CB->server[uccnt]
     ipaddr        nfd         isConnectionGood   Retry and recycle nfd
     ------        ---         ----------------   ---------------------
      zero         TYP_NULL      E_TYPE_No         No
      non-zero     TYP_ERROR     E_TYPE_No         Yes
      non-zero     +ve number    E_TYPE_Yes        No

  Active NTP server established based on priority. Master NTP server has the
  highest priority followed by backup 1 and backup 2 NTP servers.

  NTP_PollingThrdFunc thread will start later at designated polling time after
  valid connection to NTP server is established

  The status of NTP link status will be packed in NTP_MSGQ_MCP_STATUS format
  before sending it out to SWC_NTPClientCommunicationThrd using message queue
  E_SYS_MSQL_NTPC_TxExternalSystemMsg.

 CALLED BY

  NTP_Initialization

 CALLS

  None

 PARAMETER

  None

 RETURN

   TYP_NULL                  null error code
   E_ERR_InvalidNullPointer  invalid null pointer


 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    06-Feb-2010      Created initial revision
   Bryan Chong    28-Sep-2010      Update to include priority sequence on
                                   NTP servers. High priority starts from
                                   master NTP server, followed by NTP backup1
                                   and backup2 servers. [PR76]
   Bryan Chong    01-Apr-2011      Update to add capability to recycle nfd
                                   when connection is NG. [C955 PR94]
   Bryan Chong    05-Apr-2011      update to fix NTP backup server 1 and 2 not
                                   able to established connection. This issue
                                   resolved through recycling LAN file
                                   descriptor for NTP connection.[PR77]
-----------------------------------------------------------------------------*/
//#define CFG_DEBUG_MSG
VOID *NTP_LinkManagementThrdFunc(VOID *pThreadParameter)
{
  E_ERR_T            rstatus = E_ERR_Success;
  INT32             chid;
  struct sigevent   event;
  struct itimerspec itime;
  timer_t           timer_id;
  INT32             rcvid;
  NTP_CH_MSG_T      msg;
  struct timespec   tspec;
  UINT8             uccnt;
  UINT8             ucservercnt;
  BOOL_T            isServerActive = E_TYPE_No;
  NTP_PACKET_T      tx_ntppack;
  NTP_PACKET_T      rx_ntppack;
  UINT8             ucntp_packet_sz;
  UINT8             unretrycnt;
  INT32             rlen;             // receive length


  struct mq_attr    mqstat;
  INT32             msglen;
  SYS_MSGQ_MCPR     msgq;
  struct msq_ctrl_st *pcb;
  NTP_MSGQ_MCP_STATUS ntpcstatus;

  #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_NTP)
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};
  CHAR acErrMsg[100] = {0};
  #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_NTP)

  #if (((defined CFG_PRN_WARN) && CFG_PRN_WARN_NTP) || (defined CFG_PRN_ERR))
  CHAR              errBuff[100] = {0};
  #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_NTP) ||
         //  (defined CFG_PRN_ERR))
  #if((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP)
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP)

  if ((chid = ChannelCreate (0)) == TYP_ERROR)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [NTP] NTP_LinkManagementThrdFunc, fail to create channel, %s\n",
            strerror(errno));
    #endif // CFG_PRN_ERR
    return (VOID *)E_ERR_InvalidNullPointer;
  }

  // set up the pulse
  event.sigev_notify = SIGEV_PULSE;
  event.sigev_coid =
    ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);
  event.sigev_priority = getprio(0);
  event.sigev_code = E_SYS_PULSE_NTPC_LinkMgmt;
  timer_create(CLOCK_REALTIME, &event, &timer_id);

  // set timer value
  // First trigger begin after 10 seconds, subsequent after 60 seconds
  itime.it_value.tv_sec = 15;
  itime.it_value.tv_nsec = 0;
  itime.it_interval.tv_sec = pNTP_CB->poll_frequency;
  itime.it_interval.tv_nsec = 0;
  timer_settime(timer_id, 0, &itime, NULL);

  // construct NTP request packet
  tx_ntppack.flags.allbits = 0xe3;
  tx_ntppack.peerClkStranum = TYP_NULL;
  tx_ntppack.peerPollingInterval = 0x4;
  tx_ntppack.peerClockPrecision = -6;
  tx_ntppack.rootDelay = NTP_Fp64ToFpst32(1.0000);
  tx_ntppack.rootDispersion = NTP_Fp64ToFpst32(1.0000);
  tx_ntppack.referenceClockID = TYP_NULL;
  tx_ntppack.referenceClockUpdateTime = NTP_Fp64ToFpst64(0.0000);
  tx_ntppack.originateTimeStamp = NTP_Fp64ToFpst64(0.0000);
  tx_ntppack.receiveTimeStamp = NTP_Fp64ToFpst64(0.0000);

  ucntp_packet_sz = sizeof(NTP_PACKET_T);

  // set all connection to false
  for(ucservercnt = 0; ucservercnt < CGF_NTP_TOTAL_SERVER; ucservercnt++)
  {
    pNTP_CB->server[ucservercnt].isConnectionGood = E_TYPE_No;
  }

  pcb = &pSYS_CB->msq_ctrl[E_SYS_MSQL_NTPC_TxExternalSystemMsg];

  if(pcb == TYP_NULL)
  {
    #ifdef CFG_PRN_WARN
    printf("ERR  [NTP] NTP_LinkManagementThrdFunc, pcb in null pointer\n");
    #endif // CFG_PRN_WARN
    return TYP_NULL;
  }

  // get message queue attribute
  mq_getattr(pcb->msq_id, &mqstat);
  msgq.msgtype = E_SYS_MCP_NTPClientToSWC;

	  while(1)

  {
    rcvid = MsgReceive (chid, &msg, sizeof (msg), NULL);
    if (rcvid < 0)
    {
      //gotAMessage (rcvid, &msg.msg);
      #ifdef CFG_PRN_ERR
      printf("ERR  [NTP] NTP_LinkManagementThrdFunc, error, %s\n",
             strerror(errno));
      #endif //CFG_PRN_ERR
    }

    if (rcvid > 0)
    {
      //gotAMessage (rcvid, &msg.msg);
      #if((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP)
      printf("[NTP] NTP_LinkManagementThrdFunc, currently not support message"
             "handling\n");
      #endif //((defined CFG_DEBUG_MSG) || (CFG_DEBUG_NTP))
    }

    if (msg.pulse.code != E_SYS_PULSE_NTPC_LinkMgmt)
    {
      #ifdef CFG_PRN_WARN
      printf("WARN [NTP] NTP_LinkManagementThrdFunc, unidentified pulse code"
             "%d\n", msg.pulse.code);
      #endif // CFG_PRN_WARN
      continue;
    }

    isServerActive = E_TYPE_No;
    for(uccnt = 0; uccnt < CGF_NTP_TOTAL_SERVER; uccnt++)
    {
      unretrycnt = 0;
      do{
        if(pNTP_CB->server[uccnt].nfd == TYP_ERROR)
        {
          rstatus = LNC_GetConnectFileDescriptor(
            (const CHAR *)pNTP_CB->server[uccnt].ipaddr,
            (const UINT16) NTP_SERVER_PORT, (const INT32) SOCK_DGRAM,
            NTP_GETFILEDESC_TIMEOUT,
            (INT32 *) &pNTP_CB->server[uccnt].nfd);

          if(rstatus != E_ERR_Success)
          {
            unretrycnt++;
            #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_NTP)
            ERR_GetMsgString(rstatus, acErrMsg);
            printf("WARN [NTP] NTP_LinkManagementThrdFunc, server %s is down\n"
                   "  retry %d/%d, error %s\n",
               pNTP_CB->server[uccnt].ipaddr, unretrycnt, pNTP_CB->connectRetry,
               acErrMsg);
            #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_NTP)
            continue;
          }

          pLNC_CB->endPointConfig[uccnt].nfd = pNTP_CB->server[uccnt].nfd;
        }


        unretrycnt++;

        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP)
        rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                             E_SYS_TIME_Local);
        printf("[NTP] %d.%02d.%02d %02d:%02d:%02d.%03d "
               "NTP_LinkManagementThrdFunc,\n"
               "  server %s, fd %d, retry %d/%d\n",
               (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
               pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
               pbdtime->tm_sec, tspec.tv_nsec/1000000,
               pLNC_CB->endPointConfig[uccnt].pipaddr,
               pNTP_CB->server[uccnt].nfd,
               unretrycnt, pNTP_CB->connectRetry);
        //SYS_PrnDataBlock((const UINT8 *) pbuff,const UINT32 prnsz,const UINT8 byteperrow)
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_NTP))

        clock_gettime(CLOCK_REALTIME, &tspec);
        tx_ntppack.transmitTimeStamp =
         NTP_Fp64ToFpst64(tspec.tv_sec + NTP_JAN_1970 + 1.0e-9 * tspec.tv_nsec);

        rstatus = LNC_SendWithReply(pNTP_CB->server[uccnt].nfd,
                                    (const CHAR *)&tx_ntppack,
                                    ucntp_packet_sz,
                                    (VOID *)&rx_ntppack,
                                    ucntp_packet_sz, &rlen, 500);
        switch(rstatus)
        {
          case E_ERR_Success:
            break;

          case E_ERR_LNC_InvalidFileDescriptor:
          case E_ERR_LNC_SelectReadTimeOut:
          case E_ERR_LNC_FailToWriteSocket:
          case E_ERR_LNC_FailToConnectDatagramSocket:
            #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_NTP)
            ERR_GetMsgString(rstatus, acErrMsg);
            printf("WARN [NTP] NTP_LinkManagementThrdFunc, server %s is down\n"
                   "  fd %d, error %s\n",
               pNTP_CB->server[uccnt].ipaddr, pNTP_CB->server[uccnt].nfd,
               acErrMsg);
            #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_NTP)
            pNTP_CB->server[uccnt].isConnectionGood = E_TYPE_No;
            close(pNTP_CB->server[uccnt].nfd);
            pNTP_CB->server[uccnt].nfd = TYP_ERROR;
            rlen = 0;
            continue; // E_ERR_LNC_SelectReadTimeOut

          default:
            #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_NTP)
            ERR_GetMsgString(rstatus, errBuff);
            printf("WARN [NTP] NTP_LinkManagementThrdFunc, %s\n"
                   "  unhandled rstatus error %s\n",
                   SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
                   errBuff);
            #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_NTP)
            continue;

        }

        if(rlen == ucntp_packet_sz)
        {
          pNTP_CB->server[uccnt].isConnectionGood = E_TYPE_Yes;
          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP)
          printf("[NTP] NTP_LinkManagementThrdFunc, fd %d server %s is good\n",
                 pNTP_CB->server[uccnt].nfd,
                 pNTP_CB->server[uccnt].ipaddr);
          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_NTP))

          // 20100928 BC
          // set active server according to priority
          for(ucservercnt = 0; ucservercnt < CGF_NTP_TOTAL_SERVER;
              ucservercnt++)
          {
            if(pNTP_CB->server[ucservercnt].isConnectionGood == E_TYPE_No)
              continue;

            if(pNTP_CB->pactiveserver == &pNTP_CB->server[ucservercnt])
            {
              isServerActive = E_TYPE_Yes;
              break;
            }

            pNTP_CB->pactiveserver = &pNTP_CB->server[ucservercnt];
            #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP)
            printf("[NTP] NTP_LinkManagementThrdFunc, fd %d server %s is set "
                   "to active\n",
                   pNTP_CB->server[uccnt].nfd,
                   pNTP_CB->pactiveserver->ipaddr);
            #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_NTP))
            isServerActive = E_TYPE_Yes;
            break;
          }

          // set timer value to start NTP polling thread
          itime.it_value.tv_sec = 1;
          itime.it_value.tv_nsec = 0;
          itime.it_interval.tv_sec = pNTP_CB->poll_frequency;
          itime.it_interval.tv_nsec = 0;
          timer_settime(pNTP_CB->polling_timer_id, 0, &itime, NULL);
          rlen = 0;
          break;
        }else{
          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP)
          printf("WARN [NTP] NTP_LinkManagementThrdFunc, invalid rx length, "
               "%d from server %s\n",
               rlen, pNTP_CB->server[uccnt].ipaddr);
          #endif // CFG_PRN_WARN_NTP
          continue;
        }
      }while(unretrycnt < pNTP_CB->connectRetry);

      if(unretrycnt >= pNTP_CB->connectRetry)
      {
        if((pNTP_CB->server[uccnt].ipaddr[0] != 0) &&
           (pNTP_CB->server[uccnt].nfd > 0))
        {
          close(pNTP_CB->server[uccnt].nfd);
          pNTP_CB->server[uccnt].nfd = TYP_ERROR;
          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP)
          printf("[NTP] NTP_LinkManagementThrdFunc, reset fd[%d] to TYP_ERROR\n",
                 uccnt);
          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_NTP))
        }
      }
    } // for(uncnt = 0; uncnt < NTP_TOTAL_SERVER; uncnt++)

    // active server status
    if(isServerActive == E_TYPE_No)
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP)
      printf("[NTP] NTP_LinkManagementThrdFunc, set active server to "
             "TYP_NULL\n");
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP))
      pNTP_CB->pactiveserver = TYP_NULL;
    }

    // pack NTP client status information
    rstatus = ntp_McpGetStatus(&ntpcstatus, sizeof(NTP_MSGQ_MCP_STATUS));
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_PRN_ERR
      ERR_GetMsgString(rstatus, errBuff);
      printf("ERR  [NTP] NTP_LinkManagementThrdFunc, ntp_McpGetStatus "
             "error, %s\n", errBuff);
      #endif // CFG_PRN_ERR
      #if (((defined CFG_PRN_WARN) && CFG_PRN_WARN_NTP) || \
            (defined CFG_PRN_ERR))
      memset(errBuff, 0, sizeof(errBuff));
      #endif  // (((defined CFG_PRN_WARN) && CFG_PRN_WARN_NTP) ||
              //  (defined CFG_PRN_ERR))
    }

    memcpy(msgq.msgbuff, &ntpcstatus, sizeof(NTP_MSGQ_MCP_STATUS));
    msgq.msgsz = sizeof(NTP_MSGQ_MCP_STATUS);

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP_TXRESP)
    printf("[NTP] NTP_LinkManagementThrdFunc, send message queue %d "
           "bytes:\n", sizeof(SYS_MSGQ_MCPR));
    SYS_PrnDataBlock((const UINT8 *) &msgq, sizeof(SYS_MSGQ_MCPR), 20);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP))

    // Send result to serial link using message queue
    msglen = mq_send(
      pSYS_CB->msq_ctrl[E_SYS_MSQL_NTPC_TxExternalSystemMsg].msq_id,
      (CHAR *)&msgq, sizeof(SYS_MSGQ_MCPR), SYS_MSQ_PRIOR_NORMAL);

    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_NTP))
    printf("[NTP] NTP_LinkManagementThrdFunc, tx message queue %s\n",
           (msglen == 0 ? "OK" : "ERROR"));
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP))
    memset(&msgq, 0, sizeof(SYS_MSGQ_MCPR));

    if(pNTP_CB->pactiveserver == TYP_NULL)
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP)
      printf("[NTP] NTP_LinkManagementThrdFunc, set polling time to %d secs\n",
             pCGF_CB->ntp.linkDownRetryFreq);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP))
      itime.it_value.tv_sec = pCGF_CB->ntp.linkDownRetryFreq;
      itime.it_value.tv_nsec = 0;
      itime.it_interval.tv_sec = pCGF_CB->ntp.linkDownRetryFreq;
      itime.it_interval.tv_nsec = 0;
    }else{
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP)
      printf("[NTP] NTP_LinkManagementThrdFunc, set polling time to %d secs\n",
             pNTP_CB->poll_frequency);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP))
      itime.it_value.tv_sec = pNTP_CB->poll_frequency;
      itime.it_value.tv_nsec = 1;
      itime.it_interval.tv_sec = pNTP_CB->poll_frequency;
      itime.it_interval.tv_nsec = 0;
    }
    timer_settime(timer_id, 0, &itime, NULL);

  }// while(1)

} // NTP_LinkManagementThrdFunc
//#undef CFG_DEBUG_MSG
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  fpst32ToFp64

 DESCRIPTION

  This routine will convert floating point struct 32 bit to 64-bit floating
  point value

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  sfp         [in] structure to fixed point 32-bit value

 RETURN

  FP64         64-bit floating point value


 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      31-Dec-2009      Created initial revision

----------------------------------------------------------------------------*/
//FP64 NTP_Fpst32ToFp64(struct fixedpt32_st sfp)
//{
// double ret;
//
// sfp.integer_part = ntohs(sfp.integer_part);
// sfp.fraction_part = ntohs(sfp.fraction_part);
//
// ret = (double)(sfp.integer_part) + ((double)sfp.fraction_part / 0xFFFF);
//
// return (ret);
//} // fpst32ToFp64
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  NTP_Fp64ToFpst32

 DESCRIPTION

  This routine will convert 64-bit floating point value to 32-bit
  floating point structure

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  d           [in] 64-bit floating point value


 RETURN

  fixedpt32_st  32-bit fixed point structure value


 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      31-Dec-2009      Created initial revision

----------------------------------------------------------------------------*/
struct fixedpt32_st NTP_Fp64ToFpst32(FP64 d)
{
 struct fixedpt32_st sfp;

 sfp.integer_part = htons((UINT16)d);
 sfp.fraction_part = htons((UINT16)((d - (UINT16)d) * 0xFFFF));

 return (sfp);
} // NTP_Fp64ToFpst32
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  NTP_Fpst64ToFp64

 DESCRIPTION

  This routine will convert 64-bit fixed point structure to 64-bit
  floating point value

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  d           [in] 64-bit floating point value


 RETURN

  FP64        64-bit floating point value


 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      31-Dec-2009      Created initial revision

----------------------------------------------------------------------------*/
FP64 NTP_Fpst64ToFp64(struct fixedpt64_st lfp)
{
 FP64 ret;

 lfp.integer_part = ntohl(lfp.integer_part);
 lfp.fraction_part = ntohl(lfp.fraction_part);

 ret = (double)(lfp.integer_part) + ((double)lfp.fraction_part / 0xFFFFFFFF);

 return (ret);
} // NTP_Fpst64ToFp64
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  NTP_Fp64ToFpst64

 DESCRIPTION

  This routine will convert 64-bit floating point value to struct 64-bit
  floating point structure

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  d           [in] 64-bit floating point value


 RETURN

  fixedpt64_st  64-bit fixed point structure value


 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      31-Dec-2009      Created initial revision

----------------------------------------------------------------------------*/
struct fixedpt64_st NTP_Fp64ToFpst64(FP64 d)
{
 struct fixedpt64_st lfp;

 lfp.integer_part = htonl((UINT32)d);
 lfp.fraction_part = htonl((UINT32)((d - (UINT32)d) * 0xFFFFFFFF));

 return (lfp);
} // NTP_Fp64ToFpst64
/*-----------------------------------------------------------------------------

 PRIVATE ROUTINE

  ntp_McpGetStatus

 DESCRIPTION

  This routine will construct NTP client status into MCP packet

 CALLED BY

  Application main

 CALLS

  [TBD]

 PARAMETER

  pbuff    [in] pointer to MCP status packed in NTP_MSGQ_MCP_STATUS format
  buffsz   [in] total packet size

 RETURN

   E_ERR_Success             the routine executed successfully
   E_ERR_InvalidNullPointer  found invalid null pointer

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    21-May-2010      Created initial revision

-----------------------------------------------------------------------------*/
static E_ERR_T ntp_McpGetStatus(NTP_MSGQ_MCP_STATUS *pbuff, UINT8 buffsz)
{
  UINT8 uccnt;

  if((pbuff == TYP_NULL) || (pNTP_CB == TYP_NULL))
    return E_ERR_InvalidNullPointer;

  memset((CHAR *)pbuff, 0, buffsz);

  pbuff->activeServer = TYP_ERROR;
  for(uccnt = 0; uccnt < CGF_NTP_TOTAL_SERVER; uccnt++)
  {
    memcpy(pbuff->server[uccnt].ipaddr, pNTP_CB->server[uccnt].ipaddr,
           sizeof(pbuff->server[uccnt].ipaddr));

    pbuff->server[uccnt].isConnectionGood =
      pNTP_CB->server[uccnt].isConnectionGood;

    pbuff->server[uccnt].nfd = pNTP_CB->server[uccnt].nfd;

    if((pNTP_CB->pactiveserver != TYP_NULL) &&
       (pNTP_CB->pactiveserver->nfd == pbuff->server[uccnt].nfd))
      pbuff->activeServer = uccnt;
  }

  return E_ERR_Success;
} // ntp_McpGetStatus
