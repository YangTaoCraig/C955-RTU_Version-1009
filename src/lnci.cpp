/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics Ltd.

 PROPRIETARY RIGHTS of ST Electronics Ltd are involved in the subject matter
 of this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  lnci.cpp                                             D1.0.4

 COMPONENT

  LNC - LAN Communication

 DESCRIPTION

  This file consists of initialization routine for LAN Communication

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
LNC_CB *pLNC_CB;

/*----------------------------------------------------------------------------
  Public Prototypes declaration
----------------------------------------------------------------------------*/
E_ERR_T LNC_Initialization(VOID);
/*----------------------------------------------------------------------------
  Local variables declaration
----------------------------------------------------------------------------*/
static LNC_CB lncCtrlBlk;
/*----------------------------------------------------------------------------
  Local Prototypes declaration
----------------------------------------------------------------------------*/
static E_ERR_T lnc_datagramSocketInit(E_LNC_EPCONFIG label, CHAR *pipaddr,
                                     INT32 *poutfd);
//#ifdef CFG_DEV_USED
//static E_ERR_T lnc_streamSocketInit(E_LNC_EPCONFIG label, CHAR *pipaddr,
//                                     INT32 *poutfd);
//#endif // CFG_DEV_USED

/*----------------------------------------------------------------------------
  Local Lookup Table (LUT)
----------------------------------------------------------------------------*/
static const LNC_EPCONFIG_T lnc_EndPointConfigLUT[] =
{
  {E_LNC_EPCONFIG_NTPC_ServerIP1,
     AF_INET, SOCK_DGRAM, TYP_NULL, NTP_SERVER_PORT},
  {E_LNC_EPCONFIG_NTPC_ServerIP2,
     AF_INET, SOCK_DGRAM, TYP_NULL, NTP_SERVER_PORT},
  {E_LNC_EPCONFIG_NTPC_ServerIP3,
     AF_INET, SOCK_DGRAM, TYP_NULL, NTP_SERVER_PORT},
  //{E_LNC_EPCONFIG_RMM_OtherRTUIP,
    // AF_INET, SOCK_STREAM, TYP_NULL, 5000},
  {E_LNC_EPCONFIG_EndOfList,
     TYP_NULL, TYP_NULL, TYP_NULL, TYP_NULL}
};
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  LNC_Initialization

 DESCRIPTION

  This routine will initialize the LAN communciation control block and connect
  to create relevant file descriptor.
  Currently the routine is supporting Datagram connection only.


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

   Bryan Chong    08-Feb-2009      Created initial revision
   Bryan Chong    17-May-2010      Update to add switch cases for SOCK_STREAM
                                   and SOCK_DGRAM supports

-----------------------------------------------------------------------------*/
E_ERR_T LNC_Initialization(VOID)
{
  E_ERR_T rstatus = E_ERR_Success;
  INT32 nfd = 0;
  UINT8 uccnt;
  const LNC_EPCONFIG_T *plut;
  CGF_CB *pcgf;

  //pLNC_CB = &lncCtrlBlk;
  plut = lnc_EndPointConfigLUT;
  pcgf = pCGF_CB;

  for(uccnt = 0; uccnt < E_LNC_EPCONFIG_EndOfList; uccnt++)
  {
    // 20100517 BC (Rqd by ZSL)
    switch(plut[uccnt].type)
    {
      case SOCK_STREAM:
        // TODO: Add listen before connecting. Incorporate other RTU IP into
        //    cgf
//        #ifdef CFG_DEV_USED
//        rstatus = lnc_streamSocketInit((E_LNC_EPCONFIG)uccnt,
//                    "10.1.1.3", &nfd);
//        SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
//        #endif // CFG_DEV_USED
        break;

      case SOCK_DGRAM:
        rstatus = lnc_datagramSocketInit((E_LNC_EPCONFIG)uccnt,
                    pcgf->ntp.ipaddr[uccnt], &nfd);
        SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
        break;

      case SOCK_RAW:
      case SOCK_RDM:
      case SOCK_SEQPACKET:
        #ifdef CFG_PRN_WARN
        printf("WARN [LNC] LNC_Initialization, the socket type %d is currently "
               "not supported\n", plut[uccnt].type);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LNC))
        break;

      default:
        return E_ERR_LNC_InvalidSocketType;
    }// switch(plut[uccnt].type)

    pLNC_CB->endPointConfig[uccnt].nfd = nfd;
    pLNC_CB->endPointConfig[uccnt].plutEntry = (LNC_EPCONFIG_T *)&plut[uccnt];
    pLNC_CB->endPointConfig[uccnt].pipaddr = pcgf->ntp.ipaddr[uccnt];
    #ifdef CFG_DEBUG_MSG
    printf("[LNC] LNC_Initialization, initialized end point %s, fd %d\n",
           pLNC_CB->endPointConfig[uccnt].pipaddr,
           pLNC_CB->endPointConfig[uccnt].nfd);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LNC))
  }// for(uccnt = 0; uccnt < E_LNC_EPCONFIG_EndOfList, uccnt++)

  return E_ERR_Success;
} // LNC_Initialization
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  lnc_datagramSocketInit

 DESCRIPTION

  This routine will initialize socket for datagram type.

 CALLED BY

  LNC_Initialization

 CALLS

  None

 PARAMETER

  label    label for the parameter look-up table
  pipaddr  the designated IP address
  poutfd   output file descriptor

 RETURN

   E_ERR_Success      the routine executed successfully

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    12-Mar-2010      Created initial revision

-----------------------------------------------------------------------------*/
static E_ERR_T lnc_datagramSocketInit(E_LNC_EPCONFIG label, CHAR *pipaddr,
                                     INT32 *poutfd)
{
  INT32 nrval;
  INT32 nfd;
  struct sockaddr_in clientsockaddr;
  const LNC_EPCONFIG_T *plut;
  //CGF_CB *pcgf;

  pLNC_CB = &lncCtrlBlk;
  plut = lnc_EndPointConfigLUT;
  //pcgf = pCGF_CB;

  //create server socket
  nfd = socket(plut[label].domain, plut[label].type, plut[label].protocol);
  SYS_ASSERT_RETURN(nfd == TYP_ERROR, E_ERR_LNC_FailToCreateSocket);

  //setup server address
  memset(&clientsockaddr, 0, sizeof(clientsockaddr));
  clientsockaddr.sin_family = plut[label].domain;
  clientsockaddr.sin_port = htons(plut[label].port);
  clientsockaddr.sin_addr.s_addr = inet_addr(pipaddr);

  // To make sure the LAN port is available
  nrval = connect(nfd, (struct sockaddr *)&clientsockaddr,
                  sizeof(clientsockaddr));
  //nrval = bind(nfd, (struct sockaddr *)&clientsockaddr, namelen);
  if(nrval == TYP_ERROR)
  {
    close(nfd);
    #ifdef CFG_PRN_ERR
    printf("ERR  [LNC] LNC_Initialization, fail to connect to server %s\n",
           pipaddr);
    perror("ERR  [LNC] LNC_Initialization, errno");
    #endif // CFG_PRN_ERR
    *poutfd = TYP_NULL;
    return E_ERR_LNC_FailToConnectDatagramSocket;
  }

  *poutfd = nfd;

  return E_ERR_Success;
} // lnc_datagramSocketInit
//#ifdef CFG_DEV_USED
///*-----------------------------------------------------------------------------
//
// PUBLIC ROUTINE
//
//  lnc_streamSocketInit
//
// DESCRIPTION
//
//  This routine will initialize socket for datagram type.
//
// CALLED BY
//
//  LNC_Initialization
//
// CALLS
//
//  None
//
// PARAMETER
//
//  label    label for the parameter look-up table
//  pipaddr  the designated IP address
//  poutfd   output file descriptor
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
//   Bryan Chong    12-Mar-2010      Created initial revision
//
//-----------------------------------------------------------------------------*/
//static E_ERR_T lnc_streamSocketInit(E_LNC_EPCONFIG label, CHAR *pipaddr,
//                                   INT32 *poutfd)
//{
//  INT32 nrval;
//  INT32 nfd;
//  struct sockaddr_in clientsockaddr;
//  const LNC_EPCONFIG_T *plut;
//  //CGF_CB *pcgf;
//
//  pLNC_CB = &lncCtrlBlk;
//  plut = lnc_EndPointConfigLUT;
//  //pcgf = pCGF_CB;
//
//  //create server socket
//  nfd = socket(plut[label].domain, plut[label].type, plut[label].protocol);
//  SYS_ASSERT_RETURN(nfd == TYP_ERROR, E_ERR_LNC_FailToCreateSocket);
//
//  //setup server address
//  memset(&clientsockaddr, 0, sizeof(clientsockaddr));
//  clientsockaddr.sin_family = plut[label].domain;
//  clientsockaddr.sin_port = htons(plut[label].port);
//  clientsockaddr.sin_addr.s_addr = inet_addr(pipaddr);
//
//  // To make sure the LAN port is available
//  nrval = connect(nfd, (struct sockaddr *)&clientsockaddr,
//                  sizeof(clientsockaddr));
//
//  if(nrval == TYP_ERROR)
//  {
//    close(nfd);
//    #ifdef CFG_PRN_ERR
//    printf("ERR  [LNC] LNC_Initialization, fail to connect to server %s\n",
//           pipaddr);
//    perror("ERR  [LNC] LNC_Initialization, errno");
//    #endif // CFG_PRN_ERR
//    *poutfd = TYP_NULL;
//    return E_ERR_LNC_FailToConnectStreamSocket;
//  }
//
//  *poutfd = nfd;
//
//  return E_ERR_Success;
//} // lnc_streamSocketInit
//#endif // CFG_DEV_USED
