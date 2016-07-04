/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics Ltd.

 PROPRIETARY RIGHTS of ST Electronics Ltd are involved in the subject matter
 of this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  cgfm.cpp                                             D1.0.4

 COMPONENT

  CGF - Configuration File

 DESCRIPTION

  This file consists of management routine for Configuration File


 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

  Bryan Chong    10-Feb-2010        Initial revision

----------------------------------------------------------------------------*/
#include <stdio.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <stdlib.h>
#include <string.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "sys_ass.h"
#include "cgf_def.h"
#include "cgf_ext.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"


#include "Init_Config.h"

/*----------------------------------------------------------------------------
  Public variables declaration
----------------------------------------------------------------------------*/
E_ERR_T CGF_GetNTPParameter(FILE *pfd);
/*----------------------------------------------------------------------------
  Public Prototypes declaration
----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
  Local variables declaration
----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
  Local Prototypes declaration
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  CGF_GetNTPParameter

 DESCRIPTION

  This routine will acquire NTP parameters from the configuration file

 CALLED BY

  CGF_GetNTPParameter

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

   Bryan Chong    10-Feb-2009      Created initial revision
   Bryan Chong    17-Mar-2009      Add NTP backup IP address
   Bryan Chong    20-Sep-2010      Change NTP parameters' label [PR75]
-----------------------------------------------------------------------------*/
E_ERR_T CGF_GetNTPParameter(FILE *pfd)
{
  E_ERR_T rstatus = E_ERR_Success;
  CHAR acReadBuffer[INM_MAXSIZE_LINE + 1] = {0};
  CHAR acTempString[128] = {0};

  if(SearchKeyWord(pfd, (CHAR *)"BGN_NTP_CONFIG", acReadBuffer) == E_TYPE_False)
  {
    return E_ERR_CGF_SearchKeyWordFail;
  }

  rstatus = GetStringValue(pfd, (CHAR *)"SNTP_Server", acTempString);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
  pCGF_CB->ntp.isServer = (BOOL_T)atoi(acTempString);

  rstatus = GetStringValue(pfd, (CHAR *)"NTP_Client", acTempString);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
  pCGF_CB->ntp.isClient = (BOOL_T)atoi(acTempString);

  rstatus = GetStringValue(pfd, (CHAR *)"NTP_MASTER_IP", acTempString);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
  strcpy(pCGF_CB->ntp.ipaddr[0], acTempString);

  rstatus = GetStringValue(pfd, (CHAR *)"NTP_BACKUP1_IP", acTempString);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
  strcpy(pCGF_CB->ntp.ipaddr[1], acTempString);

  rstatus = GetStringValue(pfd, (CHAR *)"NTP_BACKUP2_IP", acTempString);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
  strcpy(pCGF_CB->ntp.ipaddr[2], acTempString);

  rstatus = GetStringValue(pfd, (CHAR *)"NTP_SPURIOUS_TIME", acTempString);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
  pCGF_CB->ntp.spuriousTime = atoi(acTempString);

  rstatus = GetStringValue(pfd, (CHAR *)"NTP_RETRY", acTempString);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
  pCGF_CB->ntp.retry = atoi(acTempString);

  rstatus = GetStringValue(pfd, (CHAR *)"NTP_NOTATION", acTempString);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
  pCGF_CB->ntp.notation = atoi(acTempString);

  rstatus = GetStringValue(pfd, (CHAR *)"NTP_POLLING", acTempString);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
  pCGF_CB->ntp.pollingFreq = atoi(acTempString);

  rstatus = GetStringValue(pfd, (CHAR *)"NTP_LINKDOWN_RETRY", acTempString);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
  pCGF_CB->ntp.linkDownRetryFreq = atoi(acTempString);

  return E_ERR_Success;
} // CGF_GetNTPParameter
