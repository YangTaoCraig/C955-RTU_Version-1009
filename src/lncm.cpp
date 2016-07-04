/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics Ltd.

 PROPRIETARY RIGHTS of ST Electronics Ltd are involved in the subject matter
 of this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  lncm.cpp                                             D1.0.4

 COMPONENT

  LNC - LAN Communication

 DESCRIPTION

  This file consists of management routine for LAN Communication

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

  Bryan Chong    08-Feb-2010        Initial revision

----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <sys/neutrino.h>
#include <sched.h>
#include <sys/netmgr.h>
#include <sys/select.h>
#include <net/if.h>
#include <net/route.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
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
E_ERR_T LNC_SendWithReply(INT32 nfd, const VOID *ptxbuff,
                         const UINT32 txbuff_sz, VOID *prxbuff,
                         const UINT32 rxbuffsz, INT32 *prxdata_sz,
                         INT32 timeout_ms);
E_ERR_T LNC_ReceiveMsg   (INT32 nfd, VOID *prxbuff, INT32 *prxdata_sz,
                         const UINT32 rxbuffsz, INT32 timeout_ms);
E_ERR_T LNC_SendMsg      (INT32 nfd, VOID *ptxbuff, const UINT32 txmsgsz,
                         INT32 *prtxsz);
E_ERR_T LNC_AddGateway   (const CHAR *pgateway_addr, const CHAR *psubnet_mask,
                         const UINT8 retry);
E_ERR_T LNC_GetConnectFileDescriptor(const CHAR *pipaddr, const UINT16 port_num,
                                    const INT32 sock_type,
                                    const UINT32 timeout_ms, INT32 *poutnfd);

/*----------------------------------------------------------------------------
  Local variables declaration
----------------------------------------------------------------------------*/
struct lnc_route_st
{
  struct rt_msghdr rt;
  struct sockaddr_in dst;
  struct sockaddr_in gate;
  struct sockaddr_in mask;
};

/*----------------------------------------------------------------------------
  Local Prototypes declaration
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Local Lookup Table (LUT)
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  LNC_SendWithReply

 DESCRIPTION

  This routine will transmit a request packet and wait for the reply

 CALLED BY

  NTP_PollingThrdFunc
  NTP_LinkManagementThrdFunc
  NTP_Status_Query
  RequestAndReceiveWithModbusCheck, SWC_LAN.cpp

 CALLS

  None

 PARAMETER

  nfd          [in]  file descriptor of the LAN connection
  ptxbuff      [in]  pointer to transmit buffer
  txbuff_sz    [in]  transmit buffer size
  prxbuff      [out] pointer to receive buffer
  rxbuffsz     [in]  receiving buffer size
  rxdata_sz    [out] number of data received. Can be less than or equal to
                     receive buffer size
  timeout_ms   [in]  receiving time out in milliseconds

 RETURN

   E_ERR_Success      the routine executed successfully

   E_ERR_LNC_InvalidFileDescriptor
   E_ERR_LNC_FailToWriteSocket
   E_ERR_LNC_SelectReadTimeOut
   E_ERR_LNC_SelectReadError
   E_ERR_LNC_FailToReadSocket

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    08-Feb-2010      Created initial revision

-----------------------------------------------------------------------------*/
E_ERR_T LNC_SendWithReply(INT32 nfd, const VOID *ptxbuff,
                         const UINT32 txbuff_sz, VOID *prxbuff,
                         const UINT32 rxbuffsz, INT32 *prxdata_sz,
                         INT32 timeout_ms)

{
  struct timeval tv;
  fd_set fdsRead;
  INT32 nrval;
  #if (((defined CFG_PRN_WARN) && CFG_PRN_WARN_LNC) || (defined CFG_PRN_ERR))
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};
  #endif // (((defined CFG_PRN_WARN) && CFG_PRN_WARN_LNC) ||
         //  (defined CFG_PRN_ERR))
  #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_LNC)
  E_ERR_T rstatus = E_ERR_Success;
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_LNC)

  if(nfd == TYP_NULL)
   return E_ERR_LNC_InvalidFileDescriptor;

  nrval = write(nfd, ptxbuff, txbuff_sz);
  #if ((defined CFG_DEBUG_MSG)&& CFG_DEBUG_LNC_SENDREPLY)
  printf("[LNC] LNC_SendWithReply, fd %d tx packet %d bytes:\n",
         nfd, nrval);
  #endif // ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_NTP))
  if(nrval == TYP_ERROR)
  {
    #if ((defined CFG_PRN_ERR) && CFG_PRN_WARN_LNC)
    SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff));
    if(strcmp(thrdnamebuff, "tSWC_Mgmt_PLC1") == 0)
    {
      printf("ERR  [LNC] LNC_SendWithReply, %s fd %d tx packet fail, %s\n",
             SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
             nfd, strerror(errno));
    }
    #endif // ((defined CFG_PRN_ERR) && CFG_PRN_WARN_LNC)
    return E_ERR_LNC_FailToWriteSocket;
  }

  tv.tv_sec = timeout_ms * 1e-3;
  tv.tv_usec = (timeout_ms % 1000) * 1e3;
  FD_ZERO(&fdsRead);
  FD_SET(nfd, &fdsRead);
  nrval = select(nfd + 1, &fdsRead, NULL, NULL, &tv);

  switch(nrval)
  {
    case 0:
      #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_LNC)
      SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff));
      if(strcmp(thrdnamebuff, "tSWC_Mgmt_PLC1") == 0)
      {
        rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                             E_SYS_TIME_Local);
        printf(
          "WARN [LNC] %d.%02d.%02d %02d:%02d:%02d.%03d LNC_SendWithReply, %s\n"
          "  fd %d rx time out, select time out\n",
          (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
          pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
          pbdtime->tm_sec, tspec.tv_nsec/1000000,
          thrdnamebuff, nfd);
      }
      #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_LNC)
      return E_ERR_LNC_SelectReadTimeOut;

    case TYP_ERROR:
      #ifdef CFG_PRN_ERR
      printf("ERR  [LNC] LNC_SendWithReply, %s select read error, %s\n",
             SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
             strerror(errno));
      #endif // ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_NTP))
      return E_ERR_LNC_SelectReadError;

    default:
      #if ((defined CFG_DEBUG_MSG)&& (defined _CFG_DEBUG_LNC))
      printf("[LNC] LNC_SendWithReply, select nrval = %d\n", nrval);
      #endif // ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_NTP))

      memset(prxbuff, 0, rxbuffsz);
      *prxdata_sz = read(nfd, prxbuff, rxbuffsz);

      #if ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_LNC_SENDREPLY))
      printf("[LNC] LNC_SendWithReply, rx packet %d bytes using socket %d\n",
             *prxdata_sz, nfd);
      #endif // ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_NTP))
      if(*prxdata_sz <= TYP_NULL)	//20141111 Su server table data
      {
         #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_LNC)
         printf("WARN [LNC] LNC_SendWithReply, %s fd %d receive packet fail,\n"
                "  %s\n",
                SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
                nfd, strerror(errno));
         #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_NTP))
         return E_ERR_LNC_FailToReadSocket;
      }
  }

  return E_ERR_Success;
} // LNC_SendWithReply
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  LNC_ReceiveMsg

 DESCRIPTION

  This routine will receive message from designated file decriptor for LAN

 CALLED BY



 CALLS

  None

 PARAMETER

  nfd          [in]  file descriptor of the LAN connection
  prxbuff      [out] pointer to receive buffer
  rxbuffsz     [in]  receive buffer size
  rxdata_sz    [out] receive data size. Can be less than or equal to receive
                     buffer size
  timeout_ms   [in]  receiving time out in milliseconds

 RETURN

   E_ERR_Success      the routine executed successfully

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    10-May-2010      Created initial revision

-----------------------------------------------------------------------------*/
E_ERR_T LNC_ReceiveMsg(INT32 nfd, VOID *prxbuff, INT32 *prxdata_sz,
                      const UINT32 rxbuffsz, INT32 timeout_ms)

{
  struct timeval tv;
  fd_set fdsRead;
  INT32 nrval;

  if(nfd == TYP_NULL)
   return E_ERR_LNC_InvalidFileDescriptor;

  tv.tv_sec = timeout_ms * 1e-3;
  tv.tv_usec = (timeout_ms % 1000) * 1e3;
  FD_ZERO(&fdsRead);
  FD_SET(nfd, &fdsRead);
  nrval = select(nfd + 1, &fdsRead, NULL, NULL, &tv);

  switch(nrval)
  {
    case 0:
      #ifdef CFG_PRN_WARN
      printf("WARN [LNC] LNC_ReceiveMsg, fd %d rx time out at %d ms\n",
             nfd, timeout_ms);
      #endif // ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_NTP))
      return E_ERR_LNC_SelectReadTimeOut;
    case TYP_ERROR:
      #ifdef CFG_PRN_ERR
      printf("ERR  [LNC] LNC_ReceiveMsg, select read error, %s\n",
             strerror(errno));
      #endif // ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_NTP))
      return E_ERR_LNC_SelectReadError;
    default:
      #if ((defined CFG_DEBUG_MSG)&& (defined _CFG_DEBUG_LNC))
      printf("[LNC] LNC_ReceiveMsg, select nrval = %d\n", nrval);
      #endif // ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_NTP))

      memset(prxbuff, 0, rxbuffsz);
      *prxdata_sz = 0;
      *prxdata_sz = read(nfd, prxbuff, rxbuffsz);

      #if ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_LNC))
      printf("[LNC] LNC_ReceiveMsg, fd %d rx packet %d bytes:\n",
             nfd, *prxdata_sz);
      #endif // ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_NTP))
      if(*prxdata_sz == TYP_ERROR)
      {
         #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LNC))
         printf("[LNC] LNC_ReceiveMsg, fd %d rx packet fail, %s\n",
                nfd, strerror(errno));
         #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_NTP))
         return E_ERR_LNC_FailToReadSocket;
      }
  }

  return E_ERR_Success;
} // LNC_ReceiveMsg
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  LNC_SendMsg

 DESCRIPTION

  This routine will transmit message using designated file decriptor for LAN
  interface

 CALLED BY



 CALLS

  None

 PARAMETER

  nfd          [in]  file descriptor of the LAN connection
  ptxbuff      [in]  pointer to transmit buffer
  txmsgsz      [in]  transmit message size
  prtxsz       [out] total bytes of data transmitted

 RETURN

   E_ERR_Success                the routine executed successfully
   E_ERR_LNC_FailToWriteSocket  fail to write to the file descriptor
   E_ERR_InvalidNullPointer
   E_ERR_LNC_InvalidFileDescriptor
   E_ERR_InvalidParameter

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    22-Jun-2010      Created initial revision

-----------------------------------------------------------------------------*/
E_ERR_T LNC_SendMsg(INT32 nfd, VOID *ptxbuff, const UINT32 txmsgsz,
                   INT32 *prtxsz)
{

  if((prtxsz == TYP_NULL) || (ptxbuff == TYP_NULL))
    return E_ERR_InvalidNullPointer;

  if((nfd == TYP_NULL) || (nfd == TYP_ERROR))
   return E_ERR_LNC_InvalidFileDescriptor;

  if(txmsgsz == TYP_NULL)
   return E_ERR_InvalidParameter;

  *prtxsz = write(nfd, ptxbuff, txmsgsz);
  #if ((defined CFG_DEBUG_MSG)&& CFG_DEBUG_LNC_SENDMSG)
  printf("[LNC] LNC_SendMsg, fd %d tx packet %d bytes:\n",
         nfd, *prtxsz);
  #endif // ((defined CFG_DEBUG_MSG)&& (CFG_DEBUG_NTP))
  if(*prtxsz == TYP_ERROR)
  {
    #if ((defined CFG_PRN_ERR) && CFG_PRN_ERR_LNC)
    printf("ERR [LNC] LNC_SendMsg, fd %d tx packet fail, %s\n",
           nfd, strerror(errno));
    #endif // ((defined CFG_PRN_ERR) && CFG_PRN_ERR_LNC)
    return E_ERR_LNC_FailToWriteSocket;
  }

  return E_ERR_Success;
} // LNC_SendMsg

/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  LNC_AddGateway

 DESCRIPTION

  This routine will add gateway to routing table

 CALLED BY



 CALLS

  None

 PARAMETER

  gateway_addr    [in] pointer to gateway address in IPv4 dot notation string
  subnetmask_addr [in] pointer to subnetmask address in IPv4 dot notation
                       string
  retry           [in] number of retry count

 RETURN

   E_ERR_Success                              the routine executed successfully
   E_ERR_LNC_AddGatewayFailToCreateSocket     fail to create socket
   E_ERR_LNC_AddGatewayUpdateRouteTableFail   fail to update routing table


 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    18-May-2010      Created initial revision

-----------------------------------------------------------------------------*/
E_ERR_T LNC_AddGateway(const CHAR *pgateway_addr, const CHAR *psubnet_mask,
                      const UINT8 retry)

{
  E_ERR_T rstatus = E_ERR_Success;
  INT32 sockval;
  UINT8 retrycnt = 0;
  struct rt_msghdr *rtm;
  struct sockaddr_in *dst, *gate ,*mask;
  struct lnc_route_st lnc_rt;

  if((pgateway_addr == TYP_NULL) || (psubnet_mask == TYP_NULL))
    return E_ERR_InvalidNullPointer;

  if((*pgateway_addr == 0) || (*psubnet_mask == 0))
    return E_ERR_InvalidParameter;

  if((sockval = socket(AF_ROUTE, SOCK_RAW, 0)) == -1)
  {
     #ifdef CFG_PRN_ERR
     perror("ERR  [LCN] LNC_AddGateway, open socket for set gateway");
     #endif // CFG_PRN_ERR
     return E_ERR_LNC_AddGatewayFailToCreateSocket;
  }

  memset(&lnc_rt, 0, sizeof(struct lnc_route_st));
  rtm  = &lnc_rt.rt;
  dst  = &lnc_rt.dst;
  gate = &lnc_rt.gate;
  mask = &lnc_rt.mask;

  rtm->rtm_type = RTM_ADD;
  rtm->rtm_flags = RTF_UP | RTF_GATEWAY | RTF_STATIC;
  rtm->rtm_msglen = sizeof(lnc_route_st);
  rtm->rtm_version = RTM_VERSION;
  rtm->rtm_seq = 1234;
  rtm->rtm_addrs = RTA_DST | RTA_GATEWAY | RTA_NETMASK;
  rtm->rtm_pid = getpid();

  dst->sin_len    = sizeof(*dst);
  dst->sin_family = AF_INET;

  mask->sin_len    = sizeof(*mask);
  mask->sin_family = AF_INET;

  gate->sin_len    = sizeof(*gate);
  gate->sin_family = AF_INET;
  inet_aton(pgateway_addr, &gate->sin_addr);

  while(retrycnt < retry)
  {
    if(write(sockval, rtm, rtm->rtm_msglen) < 0)
    {
      if((errno == EEXIST) && (rtm->rtm_type == RTM_ADD))
      {
        rtm->rtm_type = RTM_CHANGE;
        delay(10);
        retrycnt++;
        continue;
      }
      #ifdef CFG_PRN_ERR
      perror("ERR  [LNC] LNC_AddGateway, fail to update route table");
      #endif // CFG_PRN_ERR
      rstatus = E_ERR_LNC_AddGatewayUpdateRouteTableFail;
    }

    break;
  } // while(retrycnt++ < retry)

  if(rstatus == E_ERR_Success)
  {
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LNC))
    printf("[LNC] LNC_AddGateway, add gateway %s is ok\n", pgateway_addr);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LAN))
  }

  close(sockval);

  return rstatus;
} // LNC_AddGateway
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  LNC_GetConnectFileDescriptor

 DESCRIPTION

  This routine will create socket establish connection with the socket for
  the designated IP address and port number

 CALLED BY

  SYS_Initialization

 CALLS

  None

 PARAMETER

  pipaddr      [in]  pointer to IP address in ascii format
  port_num     [in]  port number to be connected
  sock_type    [in]  socket type
  timeout_ms   [in]  timeout in millisecond
  poutnfd      [out] pointer for output file descriptor

 RETURN

   E_ERR_Success                         the routine executed successfully
   E_ERR_LNC_FailToCreateSocket          fail to create socket
   E_ERR_LNC_InvalidSocketType           invalid socket type
   E_ERR_LNC_FailToConnectStreamSocket   fail to connect to stream socket
   E_ERR_LNC_NotSupportedSocket          not supported socket type

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    07-Jun-2010      Created initial revision
   Bryan Chong    03-Jul-2012      Update to handle socket return 0 at
                                   SOCK_DGRAM case
-----------------------------------------------------------------------------*/
E_ERR_T LNC_GetConnectFileDescriptor(const CHAR *pipaddr, const UINT16 port_num,
                                     const INT32 sock_type,
                                     const UINT32 timeout_ms, INT32 *poutnfd)
{
  //E_ERR_T rstatus = E_ERR_Success;
  INT32 nrval;
  INT32 nfd;
  //UINT8 cnt = 0;
  struct sockaddr_in clientsockaddr;

  struct sigevent event;
  uint64_t timeout;

  #if (((defined CFG_DEBUG_MSG) && CFG_DEBUG_LNC_GETFD) || \
       ((defined CFG_PRN_ERR) && CFG_PRN_ERR_LNC))
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_LNC_GETFD)

  if(pipaddr == TYP_NULL)
    return E_ERR_InvalidNullPointer;

  //create server socket
  switch(sock_type)
  {
    case SOCK_STREAM:
      //setup server address
      memset(&clientsockaddr, 0, sizeof(clientsockaddr));
      clientsockaddr.sin_family = AF_INET;
      clientsockaddr.sin_port = htons(port_num);
      clientsockaddr.sin_addr.s_addr = inet_addr(pipaddr);

      nfd = socket(AF_INET, SOCK_STREAM, 0);
      SYS_ASSERT_RETURN((nfd == TYP_ERROR) || (nfd == TYP_NULL),
                        E_ERR_LNC_FailToCreateSocket);

      #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_LNC_GETFD)
      printf("[LNC] LNC_GetConnectFileDescriptor, %s\n"
             "  fd %d created stream socket for %s:%d\n",
             SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
             nfd, pipaddr, port_num);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_LNC_GETFD)

      // timeout
      timeout = timeout_ms*1e6;
      event.sigev_notify = SIGEV_UNBLOCK;
      TimerTimeout(CLOCK_REALTIME , _NTO_TIMEOUT_SEND | _NTO_TIMEOUT_REPLY |
                   _NTO_TIMEOUT_RECEIVE ,  &event, &timeout, NULL);

      // To make sure the LAN port is available
      nrval = connect(nfd, (struct sockaddr *)&clientsockaddr,
                      sizeof(clientsockaddr));

      if(nrval == TYP_ERROR)
      {
        #if ((defined CFG_PRN_ERR) && CFG_PRN_ERR_LNC)
        SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff));
        if(strcmp(thrdnamebuff, "tSWC_Mgmt_PLC1") == 0)
        {
          printf("ERR  [LNC] LNC_GetConnectFileDescriptor, %s\n"
                 "  timeout at %d ms. SOCK_STREAM fail to connect to server "
                 "%s:%d using fd %d\n",
                 thrdnamebuff, timeout_ms, pipaddr, port_num, nfd);
        }
        //perror("ERR  [LNC] LNC_GetConnectFileDescriptor, perror");
        #endif // ((defined CFG_PRN_ERR) && CFG_PRN_ERR_LNC)
        close(nfd);
        nfd = TYP_ERROR;
        *poutnfd = TYP_ERROR;

        return E_ERR_LNC_FailToConnectStreamSocket;
      }

      // LAN link resume

      break; // SOCK_STREAM

    case SOCK_DGRAM:
      //setup server address
      memset(&clientsockaddr, 0, sizeof(clientsockaddr));
      clientsockaddr.sin_family = AF_INET;
      clientsockaddr.sin_port = htons(port_num);
      clientsockaddr.sin_addr.s_addr = inet_addr(pipaddr);

      nfd = socket(AF_INET, SOCK_DGRAM, 0);
      SYS_ASSERT_RETURN((nfd == TYP_ERROR) || (nfd == TYP_NULL),
                        E_ERR_LNC_FailToCreateSocket);

      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_LNC_GETFD)
      SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff));
      printf("[LNC] LNC_GetConnectFileDescriptor, %s\n"
             "  fd %d created datagram socket for %s:%d\n",
             thrdnamebuff, nfd, pipaddr, port_num);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_LNC_GETFD)

      // timeout
      timeout = timeout_ms*1e6;
      event.sigev_notify = SIGEV_UNBLOCK;
      TimerTimeout(CLOCK_REALTIME , _NTO_TIMEOUT_SEND | _NTO_TIMEOUT_REPLY |
                   _NTO_TIMEOUT_RECEIVE ,  &event, &timeout, NULL);

      // To make sure the LAN port is available
      nrval = connect(nfd, (struct sockaddr *)&clientsockaddr,
                      sizeof(clientsockaddr));

      if(nrval == TYP_ERROR)
      {
        #if ((defined CFG_PRN_ERR) && CFG_PRN_ERR_LNC)
        printf("ERR  [LNC] LNC_GetConnectFileDescriptor, timeout at %d ms. "
               "SOCK_DGRAM fail to connect to server %s:%d using fd %d\n",
               timeout_ms, pipaddr, port_num, nfd);
        //perror("ERR  [LNC] LNC_GetConnectFileDescriptor, perror");
        #endif // ((defined CFG_PRN_ERR) && CFG_PRN_ERR_LNC)
        close(nfd);
        nfd = TYP_ERROR;
        *poutnfd = TYP_ERROR;

        return E_ERR_LNC_FailToConnectDatagramSocket;
      }
      break; // SOCK_DGRAM

    case SOCK_RAW:
    case SOCK_RDM:
    case SOCK_SEQPACKET:
      #ifdef CFG_PRN_WARN
      printf("WARN [LNC] LNC_GetConnectFileDescriptor, the socket type %d is "
             "currently not supported\n", sock_type);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LNC))
      *poutnfd = TYP_ERROR;
      return E_ERR_LNC_NotSupportedSocket;
      break;

    default:
      *poutnfd =  TYP_ERROR;
      return E_ERR_LNC_InvalidSocketType;
  }

  *poutnfd = nfd;

  return E_ERR_Success;
} // LNC_GetConnectFileDescriptor

