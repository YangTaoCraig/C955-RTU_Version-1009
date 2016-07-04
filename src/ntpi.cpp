/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics Ltd.

 PROPRIETARY RIGHTS of ST Electronics Ltd are involved in the subject matter
 of this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  ntpi.cpp                                             D1.0.4

 COMPONENT

  NTP - Network Time Protocol Client

 DESCRIPTION

  This file consists of initialization routine for NTP client.


 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                REMARKS

  Bryan Chong    10-Feb-2010        Initial revision

----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <sys/neutrino.h>

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
NTP_CB *pNTP_CB;
/*----------------------------------------------------------------------------
  Public Prototypes declaration
----------------------------------------------------------------------------*/
E_ERR_T NTP_Initialization(VOID);
/*----------------------------------------------------------------------------
  Local variables declaration
----------------------------------------------------------------------------*/
static NTP_CB ntpCtrlBlk;
/*----------------------------------------------------------------------------
  Local Prototypes declaration
----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  NTP_Initialization

 DESCRIPTION

  This routine will initialize the NTP client.
  This module is dependable two modular control blocks,
   i. pCGF_CB to acquire the rest of the NTP parameters other than IP related
      parameter.
   ii. pLNC_CB to obtain initialized LAN parameter such as valid file
       descriptor.

 CALLED BY

  SYS_Initialization

 CALLS

  None

 PARAMETER

  None

 RETURN

   E_ERR_Success      the routine executed successfully
   E_ERR_InvalidNullPointer   the input pointer is null

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    25-Jan-2009      Created initial revision
   Bryan Chong    17-May-2010      Update to return null pointer error

-----------------------------------------------------------------------------*/
E_ERR_T NTP_Initialization(VOID)
{
  UINT8 uccnt;

  pNTP_CB = &ntpCtrlBlk;

  // 20100517 BC (Rqd by ZSL)
  if (pLNC_CB == TYP_NULL)
    return E_ERR_InvalidNullPointer;

  for(uccnt = 0; uccnt < CGF_NTP_TOTAL_SERVER; uccnt++)
  {
    if(pLNC_CB->endPointConfig[E_LNC_EPCONFIG_NTPC_ServerIP1 + uccnt].
       pipaddr[0] == '0')
    {
      memset(pNTP_CB->server[uccnt].ipaddr, 0,
             sizeof(pNTP_CB->server[uccnt].ipaddr));
      pNTP_CB->server[uccnt].nfd = TYP_NULL;
    }
    else{
      strcpy(pNTP_CB->server[uccnt].ipaddr,
        pLNC_CB->endPointConfig[E_LNC_EPCONFIG_NTPC_ServerIP1 + uccnt].pipaddr);
      pNTP_CB->server[uccnt].nfd =
        pLNC_CB->endPointConfig[E_LNC_EPCONFIG_NTPC_ServerIP1 + uccnt].nfd;
    }
  }
  pNTP_CB->pactiveserver = TYP_NULL;
  pNTP_CB->threshold = pCGF_CB->ntp.spuriousTime;
  pNTP_CB->connectRetry = pCGF_CB->ntp.retry;
  pNTP_CB->notation = pCGF_CB->ntp.notation;
  pNTP_CB->poll_frequency = pCGF_CB->ntp.pollingFreq;
  pNTP_CB->linkDownAttempt = pCGF_CB->ntp.linkDownRetryFreq;
  return E_ERR_Success;
} // NTP_Initialization
