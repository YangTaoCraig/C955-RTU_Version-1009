/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics Ltd.

 PROPRIETARY RIGHTS of ST Electronics Ltd are involved in the subject matter
 of this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  sysm.cpp                                               D1.0.4

 COMPONENT

  SYS - Application System

 DESCRIPTION

  This file consists of initialization routine for the application

 AUTHOR

  Bryan KW Chong


 REVISION HISTORY by Bryan Chong

 D1.1.5
 ------
 31-May-2011

 - convert cout to printf for readability issue from the group [PR99]

 D1.1.3
 ------
 29-Apr-2011

 - remove all code under definition CFG_RMM_NOTATION_THRD [C955 PR103]
 - remove CRTUStatus::ReadMultiNodeSWCLinkStatus per ZSL request [C955 PR104]
 - remove CRTUStatus::UpdateMultiNodeSWCLinkStatus per ZSL request [C955 PR104]
 - remove commented code CRTUStatus::CopySWCPollingTable [C955 PR106]

 27-Apr-2011

 - CRTUStatus::ClearFlag
     Add reset m_bWaitLinkCheckCompletedFlag
 - Update to add modification history [C955 PR96]

 08-09-2009

 - Initial revision

----------------------------------------------------------------------------*/
#include <stdio.h>
#include <sys/neutrino.h>
#include <mqueue.h>
#include <string.h>
#include <sys/statvfs.h>
#include <time.h>
#include <iostream.h>
#include <termios.h>
#include <pthread.h>
#include <process.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "err_ext.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
#include "sys_ass.h"
#include "hwp_def.h"
#include "hwp_ext.h"

#include "Common_qnxWatchDog.h"
#include "CMM_Watchdog.h"
#include "Init.h"
/*----------------------------------------------------------------------------
  Public variables declaration
----------------------------------------------------------------------------*/
const CHAR **pSYS_WdayStr;
/*----------------------------------------------------------------------------
  Public Prototypes declaration
----------------------------------------------------------------------------*/
E_ERR_T SYS_Main                  (CHAR *pconfigfilename);
//E_ERR_T SYS_PrnOutputParameter    (VOID *pout, UINT16 *psz_out, CHAR *pbuffer);
E_ERR_T SYS_CreateHeaderString    (VOID);
E_ERR_T SYS_GetCurrentTimeInString(struct tm **poutbdtime,
                                   struct timespec *ptspec,
                                   E_SYS_TIME time_type);
E_ERR_T SYS_PrnDataBlock          (const UINT8 *pbuff, const UINT32 prnsz,
                                   const UINT8 byteperrow);
//E_ERR_T SYS_PrnDataBlockBuffer    (const UINT8 *pbuff, const UINT32 prnsz,
//                                   const UINT8 byteperrow,
//                                   const UINT8 *poutbuff,
//                                   const UINT16 outbuffsz);
E_ERR_T SYS_GetDiskMemorySpace    (UINT64 *poutfreediskspace);

//E_ERR_T SYS_SetMutex              (const E_SYS_MTX mutexID,
//                                   const BOOL_T isLock);
E_ERR_T SYS_CheckToUpdateRTC      (struct tm tmMsg);
E_ERR_T SYS_SetRTCTime            (struct tm tmTime);
E_ERR_T SYS_UpdateHardwareRTC     (VOID);
CHAR *  SYS_GetDefaultThrdName    (CHAR *pbuff, INT32 buffsz);
E_ERR_T SYS_ExecShutDown          (VOID);


//#ifdef NOT_USED
////E_ERR_T SYS_GetTimeInString       (struct tm **poutbdtime,
//                                   struct timespec *ptspec,
//                                   E_SYS_TIME time_type);
//E_ERR_T SYS_GetDataRate           (const struct timespec *ptstart,
//                                   const struct timespec *ptend,
//                                   const UINT32 *ptotalbytes,
//                                   FP64 *poutbyterate);
//#endif // NOT_USED
/*----------------------------------------------------------------------------
  Local variables declaration
----------------------------------------------------------------------------*/
const CHAR *sys_wday_str[] =
  {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
   "Sunday"};
/*----------------------------------------------------------------------------
  Local Prototypes declaration
----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_Main

 DESCRIPTION

  The system application entry point. This routine will perform system
  initialization. The system will exit when thread death event happens

 CALLED BY

  Application main

 CALLS

  SYS_Initialization   System initialization

 PARAMETER

  None

 RETURN

  None

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    08-Mar-2010      Created initial revision
   Bryan Chong    13-Aug-2010      Update to use execl for the system reset
                                   functionality [PR66]
-----------------------------------------------------------------------------*/
E_ERR_T SYS_Main(CHAR *pconfigfilename)
{
  E_ERR_T rstatus = E_ERR_Success;

  #ifdef CFG_PRN_ERR
  CHAR errmsgbuff[ERR_STR_SZ] = {0};
  #endif // CFG_PRN_ERR

  struct _pulse tMsg;
  int CheckTaskAliveChannelID = -1 ;
  int nRcvID = -1;

  #ifdef CFG_PRN_REL_INFO
  printf("\n\n"
         "---------------------------------------------------------\n");
  printf("%s\n", CFG_SYS_REVISION_STRING);
  printf("---------------------------------------------------------\n");
  #endif // CFG_PRN_REL_INFO

  pthread_setname_np(pthread_self(), "tSysEntry");

  pSYS_WdayStr = sys_wday_str;

  if(SYS_Initialization(pconfigfilename) != E_ERR_Success)
  {
    #ifdef CFG_PRN_ERR
    ERR_GetMsgString(rstatus, errmsgbuff);
    printf("ERR  [SYS] SYS_Main, System initialization fail, %s\n",
           errmsgbuff);
    #endif // CFG_PRN_ERR

//    #if CFG_ENABLE_PCI_DETECTION
//    // remove serial driver command
//    SYS_DeinitializeDevc8250Manager();
//    #endif // (CFG_ENABLE_PCI_DETECTION)

    return rstatus;
  }

  //To Check if each Task is alive. When any task exit , ReStart Application
  CheckTaskAliveChannelID = ChannelCreate (_NTO_CHF_THREAD_DEATH);

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SYS))
  printf("[SYS] SYS_Main, System is ready...\r\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SYS))

  while(1)
  {

    nRcvID = MsgReceive(CheckTaskAliveChannelID, &tMsg, sizeof(struct _pulse),
                        NULL);
    if (nRcvID == 0) /* we got a pulse */
    {
      switch(tMsg.code)
      {
        case _PULSE_CODE_THREADDEATH:
          //thread is dead
          if(g_tRTUStatus.nRTUStateFlag != STATEFLAG_HARDWARETEST)
          {
            #ifdef CFG_PRN_ERR
            printf("ERR  [SYS] SYS_Main, pulse _PULSE_CODE_THREADDEATH\n"
                   "type %d, subtype %d, value %d, scoid %d\n",
                   tMsg.type, tMsg.subtype, tMsg.value, tMsg.scoid);
            #endif // CFG_DEBUG_MSG

//            #if CFG_ENABLE_PCI_DETECTION
//            rstatus = SYS_DeinitializeDevc8250Manager();
//            SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
//            #endif // (CFG_ENABLE_PCI_DETECTION)



            #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SYS)
            printf("[SYS] SYS_Main, system exiting application..\n");
            #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SYS))

            #if (CFG_ENABLE_SYSTEM_SHUTDOWN && (defined CFG_TEST_RELEASE))
            ResetRTU();
            #endif // ((defined CFG_ENABLE_SYSTEM_SHUTDOWN) &&
                   //  (defined CFG_TEST_RELEASE))

            #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SYS)
            printf("[SYS] SYS_Main, application exit complete\n");
            #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SYS))

            return E_ERR_ApplicationExit;
          }

          break; // _PULSE_CODE_THREADDEATH
      } // switch(tMsg.code)
    } // if (nRcvID == 0)
  } // while(1)
  #ifdef CFG_PRN_ERR
  printf("ERR  [SYS] SYS_Main, System Exit\n");
  #endif // CFG_PRN_ERR
} // SYS_Main
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_PrnOutputParameter

 DESCRIPTION

  This routine will print output parameter

 CALLED BY

  Application

 CALLS

  None

 INPUTS

  TBA


 OUTPUTS

  E_ERR_Success   routine executed successfully

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      04-Dec-2008      Created initial revision

----------------------------------------------------------------------------*/
//E_ERR_T SYS_PrnOutputParameter(VOID *pout, UINT16 *psz_out, CHAR *pbuffer)
//{
//
//  if((pout == TYP_NULL) || (psz_out == TYP_NULL) || (pbuffer == TYP_NULL))
//    return E_ERR_InvalidNullPointer;
//
//  *psz_out = strlen(pbuffer);
//
//  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SYS))
//  // check if buffer size overflow
//  printf("[SYS] SYS_PrnOutputParameter, pbuffer str length = %d\n",
//         *psz_out);
//  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SYS))
//
//  if(*psz_out > CFG_SPA_OUTPUT_BUFF_SZ - 1) // minus 1 for \0 padding
//  {
//    #ifdef CFG_PRN_ERR
//    printf("ERR  [SYS] SYS_SystemErrorQuery, string size, %d, exceed buffer "
//           "limit, %d. The string will be truncated to max buffer size\n",
//      *psz_out, CFG_SPA_OUTPUT_BUFF_SZ);
//    #endif // CFG_PRN_ERR
//    *psz_out = CFG_SPA_OUTPUT_BUFF_SZ - 1;
//  }
//  memset(pout, 0, *psz_out + 1);
//  memcpy(pout, pbuffer, *psz_out);
//
//  return E_ERR_Success;
//}// SYS_PrnOutput
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_CreateHeaderString

 DESCRIPTION

  This routine will initialize the header string

 CALLED BY

  Application

 CALLS

  None

 INPUTS

  TBA


 OUTPUTS

  E_ERR_Success   routine executed successfully

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      04-Dec-2008      Created initial revision

----------------------------------------------------------------------------*/
E_ERR_T SYS_CreateHeaderString(VOID)
{
  UINT8 header_sz = strlen(CFG_SYS_REVISION_STRING);
  CHAR uctmpbuff[SYS_HEADERLINE_BUFF_SZ] = {0};

  memset(pSYS_CB->headerline, 0, sizeof(pSYS_CB->headerline));

  if(header_sz > SYS_HEADERLINE_BUFF_SZ)
    return E_ERR_SYS_HeaderSizeExceedBufferSz;

  sprintf(pSYS_CB->headerline, "\n\n");
  for(UINT8 uccnt = 0; uccnt < header_sz; uccnt++)
  {
    uctmpbuff[uccnt] = '-';
  }
  strcat(pSYS_CB->headerline, uctmpbuff);

  memset(uctmpbuff, 0, sizeof(uctmpbuff));
  sprintf(uctmpbuff, "\n%s\n", CFG_SYS_REVISION_STRING);
  strcat(pSYS_CB->headerline, uctmpbuff);

  memset(uctmpbuff, 0, sizeof(uctmpbuff));
  for(UINT8 uccnt = 0; uccnt < header_sz; uccnt++)
  {
    uctmpbuff[uccnt] = '-';
  }
  strcat(pSYS_CB->headerline, uctmpbuff);
  memset(uctmpbuff, 0, sizeof(uctmpbuff));
  sprintf(uctmpbuff, "\n");
  strcat(pSYS_CB->headerline, uctmpbuff);

  #ifdef _CFG_DEBUG_MSG
  printf("%s(%d)\n", pSYS_CB->headerline, strlen(pSYS_CB->headerline));
  #endif // CFG_DEBUG_MSG
  return E_ERR_Success;
}// SYS_CreateHeaderString
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_GetCurrentTimeInString

 DESCRIPTION

  This routine will return a pointer reference to the current time break-down
  string.

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  poutbdtime      [out] pointer to a pointer to store return address from gmtime
  ptspec          [in] pointer to a timespec structure variable
  time_t          [in] type of time format. Local or UTC

 RETURN

  E_ERR_Success   routine executed successfully

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      28-Jan-2010      Created initial revision
 Bryan Chong      23-Feb-2010      Add time format selection

----------------------------------------------------------------------------*/
E_ERR_T SYS_GetCurrentTimeInString(struct tm **poutbdtime,
                                  struct timespec *ptspec,
                                  E_SYS_TIME time_type)
{

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_TIME))
  CHAR *tz;
  #endif // CFG_DEBUG_MSG

  if((poutbdtime == TYP_NULL) || (ptspec == TYP_NULL))
    return E_ERR_InvalidNullPointer;

  switch(time_type)
  {
    case E_SYS_TIME_Local:
      // get local time format
      clock_gettime(CLOCK_REALTIME, ptspec);
      *poutbdtime = localtime(&ptspec->tv_sec);
      break;

    case E_SYS_TIME_UTC:
      // get UTC time format
      clock_gettime(CLOCK_REALTIME, ptspec);
      *poutbdtime = gmtime((const time_t*)ptspec);
      break;

    default:
      return E_ERR_SYS_InvalidTimeType;
  } // switch(time_type)

  if(*poutbdtime == TYP_NULL)
    return E_ERR_InvalidNullPointer;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_TIME)
  printf("[SYS] SYS_GetCurrentTimeInString, local time: "
         "%d.%02d.%02d %02d:%02d:%02d.%03d\n",
         ((*poutbdtime)->tm_year + 1900), ((*poutbdtime)->tm_mon + 1),
          (*poutbdtime)->tm_mday, (*poutbdtime)->tm_hour, (*poutbdtime)->tm_min,
          (*poutbdtime)->tm_sec, ptspec->tv_nsec/1000000);
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_TIME)
  return E_ERR_Success;
} // SYS_GetCurrentTimeInString
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_GetDiskMemorySpace

 DESCRIPTION

  This routine will return the free memory size from the Compact Flash device

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  poutfreediskspace  [out] pointer a variable for free memory size

 RETURN

  E_ERR_Success             routine executed successfully
  E_ERR_InvalidNullPointer  invalid pointer for the output parameter

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      10-Feb-2010      Created initial revision

----------------------------------------------------------------------------*/
E_ERR_T SYS_GetDiskMemorySpace(UINT64 *poutfreediskspace)
{
  struct statvfs tStatBuf;

  if(poutfreediskspace == TYP_NULL)
    return E_ERR_InvalidNullPointer;

  *poutfreediskspace = 0;
  statvfs(CFG_RTU_LOG_FOLDER, &tStatBuf );

  *poutfreediskspace = (tStatBuf.f_bavail) *(tStatBuf.f_bsize );
  return E_ERR_Success;
} // SYS_GetDiskMemorySpace
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_PrnDataBlock

 DESCRIPTION

  This routine will print data bytes to debuging screen

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  pbuff       [in]  input buffer to print
  prnsz       [in]  input print string size
  byteperrow  [in]  number of byte per row

 RETURN

  None

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    24-Feb-2010      Created initial revision

-----------------------------------------------------------------------------*/
E_ERR_T SYS_PrnDataBlock(const UINT8 *pbuff, const UINT32 prnsz,
                        const UINT8 byteperrow)
{

  CHAR msg_buffer[SYS_DEBUG_MSG_BUFF_SZ] = {0};
  CHAR prn_buffer[SYS_DEBUG_MSG_BUFF_SZ] = {0};
  UINT32 uncnt;
  UINT32 unbytecnt = 0;

  if(pbuff == TYP_NULL)
    return E_ERR_InvalidNullPointer;

  if((prnsz == TYP_NULL) || (byteperrow == TYP_NULL))
    return E_ERR_InvalidParameter;

  if(prnsz > SYS_DEBUG_MSG_BUFF_SZ)
    return E_ERR_ExceedBufferSize;

  // mutex on
  //pthread_mutex_lock(&pSYS_CB->mutex_ctrl[E_SYS_MTX_PrnDataBlk].mtx);

  memset(msg_buffer, 0, sizeof(msg_buffer));
  memset(prn_buffer, 0, sizeof(prn_buffer));

  sprintf(prn_buffer, "  0x");
  unbytecnt += strlen(prn_buffer);
  strcat(msg_buffer, prn_buffer);

  for(uncnt = 0; uncnt < prnsz; uncnt++)
  {
    sprintf(prn_buffer, "%02x ", pbuff[uncnt]);
    strcat(msg_buffer, prn_buffer);

    // add extra two spaces to separate 10 bytes group when
    // byte per row is more than 10
    if((byteperrow > 10) && ( ((uncnt + 1) % 10) == 0))
    {
      sprintf(prn_buffer, "  ");
      strcat(msg_buffer, prn_buffer);
    }

    // add new line when byte per row is achieved
    if(((uncnt + 1) % byteperrow) == 0)
    {
      sprintf(prn_buffer, "\n    ");
      strcat(msg_buffer, prn_buffer);
    }
  }

  sprintf(prn_buffer, "\n");
  strcat(msg_buffer, prn_buffer);

  printf("%s", msg_buffer);

  // mutex off
  //pthread_mutex_unlock(&pSYS_CB->mutex_ctrl[E_SYS_MTX_PrnDataBlk].mtx);


  return E_ERR_Success;
} // SendDataBlock
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_PrnDataBlockBuffer

 DESCRIPTION

  This routine will print data bytes to the designated output buffer

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  pbuff       [in]  input buffer to print
  prnsz       [in]  input print string size
  byteperrow  [in]  number of byte per row
  poutbuff    [out] output buffer
  outbuffsz   in]   output buffer size

 RETURN

  None

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    06-Dec-2011      Created initial revision

-----------------------------------------------------------------------------*/
//E_ERR_T SYS_PrnDataBlockBuffer(const UINT8 *pbuff, const UINT32 prnsz,
//                               const UINT8 byteperrow, const UINT8 *poutbuff,
//                               const UINT16 outbuffsz)
//{
//
//  CHAR msg_buffer[SYS_DEBUG_MSG_BUFF_SZ] = {0};
//  CHAR prn_buffer[SYS_DEBUG_MSG_BUFF_SZ] = {0};
//  UINT32 uncnt;
//  UINT32 unbytecnt = 0;
//
//  if((pbuff == TYP_NULL) || (poutbuff == TYP_NULL))
//    return E_ERR_InvalidNullPointer;
//
//  if((prnsz == TYP_NULL) || (byteperrow == TYP_NULL))
//    return E_ERR_InvalidParameter;
//
//  if(prnsz > SYS_DEBUG_MSG_BUFF_SZ)
//    return E_ERR_ExceedBufferSize;
//
//  // mutex on
//  //pthread_mutex_lock(&pSYS_CB->mutex_ctrl[E_SYS_MTX_PrnDataBlk].mtx);
//
//  memset(msg_buffer, 0, sizeof(msg_buffer));
//  memset(prn_buffer, 0, sizeof(prn_buffer));
//
//  sprintf(prn_buffer, "  0x");
//  unbytecnt += strlen(prn_buffer);
//  strcat(msg_buffer, prn_buffer);
//
//  for(uncnt = 0; uncnt < prnsz; uncnt++)
//  {
//    sprintf(prn_buffer, "%02x ", pbuff[uncnt]);
//    strcat(msg_buffer, prn_buffer);
//    if(((uncnt + 1) % byteperrow) == 0)
//    {
//      sprintf(prn_buffer, "\r\n    ");
//      strcat(msg_buffer, prn_buffer);
//    }
//  }
//
//  sprintf(prn_buffer, "\r\n");
//
//  strcat(msg_buffer, prn_buffer);
//
//  if(strlen(msg_buffer) > outbuffsz)
//    return E_ERR_ExceedBufferSize;
//
//  sprintf((CHAR *)poutbuff, "%s", msg_buffer);
//
//  return E_ERR_Success;
//} // SendDataBlock
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_CheckToUpdateRTC

 DESCRIPTION

  This routine will check if updating the Real Time Clock is necessary by
  checking on the system clock. Hardware RTC will be updated as well when
  update is necessary.


 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  tmMsg     [in] standard tm format of date and time to be set

 RETURN

  E_ERR_Success               the routine executed successfully
  E_ERR_SYS_GetTimeOfDayFail  fail at time_of_day call
  E_ERR_SYS_ClockSetTimeFail  fail at clock_settime call

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    30-Jun-2011      Created initial revision

-----------------------------------------------------------------------------*/
E_ERR_T SYS_CheckToUpdateRTC(struct tm tmMsg)
{
  E_ERR_T rstatus = E_ERR_Success;
  time_t time_of_day;
  struct tm *rtctime;
  struct timespec stime;

  clock_gettime(CLOCK_REALTIME, &stime);
  time_of_day = stime.tv_sec ;
  if(time_of_day < 0)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SYS] SYS_UpdatRTC, fail at clock_gettime\n");
    #endif // CFG_PRN_ERR
    return E_ERR_SYS_ClockGetTimeFail;
  }

  rtctime = localtime(&time_of_day);
  if(rtctime == TYP_NULL)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SYS] SYS_UpdatRTC, fail at localtime\n");
    #endif // CFG_PRN_ERR
    return E_ERR_SYS_GetLocalTimeFail;
  }

  //update real-time clock if tmMsg date or time is different
  if ( (rtctime->tm_sec  != tmMsg.tm_sec) ||
       (rtctime->tm_min  != tmMsg.tm_min) ||
       (rtctime->tm_hour != tmMsg.tm_hour) ||
       (rtctime->tm_mday != tmMsg.tm_mday) ||
       (rtctime->tm_mon  != tmMsg.tm_mon) ||
       (rtctime->tm_year != tmMsg.tm_year) )
  {
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SYS)
    printf("[SYS] SYS_UpdatRTC, rtctime sec %d, min %d, hr %d, day %d, mon %d, "
           "yr %d\n" ,
           rtctime->tm_sec, rtctime->tm_min, rtctime->tm_hour,
           rtctime->tm_mday, rtctime->tm_mon, rtctime->tm_year);
    printf("  tmMsg sec %d, min %d, hr %d, day %d, mon %d, yr %d\n",
           tmMsg.tm_sec, tmMsg.tm_min, tmMsg.tm_hour, tmMsg.tm_mday,
           tmMsg.tm_mon, tmMsg.tm_year);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SYS)


    rstatus = SYS_SetRTCTime(tmMsg);
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_PRN_ERR
      printf("ERR  [SYS] SYS_UpdatRTC, fail to set RTC\n");
      #endif // CFG_PRN_ERR
      return rstatus;
    }
  }

  return E_ERR_Success;
}// SYS_CheckToUpdateRTC

/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_SetRTCTime

 DESCRIPTION

  This routine will set the Real Time Clock to the designated date and time.

  Take note,
  1. month is zero based, 0..11
  2. year since 1900

  The rest is as follows,

  tm_sec:  0..61, allowing of leap seconds
  tm_min:  0..59
  tm_hour: 0..23
  tm_mday: 1..31
  tm_wday: 0..6
  tm_yday: since Jan 1, 0..365
  tm_isdst: day light saving flag
  tm_gmtoff: offset from UTC
  tm_zone: string for the time zone name


 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  tmTime     [in] standard tm format of date and time to be set

 RETURN

  E_ERR_Success               the routine executed successfully
  E_ERR_SYS_GetTimeOfDayFail  fail at time_of_day call
  E_ERR_SYS_ClockSetTimeFail  fail at clock_settime call

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    30-Jun-2011      Created initial revision
   Bryan Chong    29-May-2012      Update to use spawnl instead of system to
                                   execute RTC process

-----------------------------------------------------------------------------*/
E_ERR_T SYS_SetRTCTime(struct tm tmTime)
{
  time_t time_of_seconds;
  struct timespec tmspc;
  E_ERR_T rstatus = E_ERR_Success;

  time_of_seconds = mktime(&tmTime);

  // convert to timespec format
  tmspc.tv_sec = time_of_seconds;
  tmspc.tv_nsec = 0;

  if(clock_settime(CLOCK_REALTIME, &tmspc) == TYP_ERROR)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SYS] SYS_UpdatRTC, fail at clock_settime\n");
    #endif // CFG_PRN_ERR
    return E_ERR_SYS_ClockSetTimeFail;
  }

  // update current time to hardware BIOS system
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SYS)
  printf("[SYS] SYS_UpdateRTC, rtc\n");
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SYS)

  rstatus = SYS_UpdateHardwareRTC();
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  return E_ERR_Success;
}// SYS_UpdatRTC
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_UpdateHardwareRTC

 DESCRIPTION

  This routine will update the bios hardware clock based on CLOCK_REALTIME.
  The routine is equivalent to invoke the following command at command shell,
  # rtc -s hw

  To check if hardware clock is updated, at target command prompt, type

  # rtc -l hw
  # date

  The first command is to load the hardware clock to CLOCK_REALTIME, and the
  second command is to verify if the UTC time is correct

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  none

 RETURN

  E_ERR_Success                    the routine executed successfully
  E_ERR_SYS_UpdateHardwareRTCFail  update hardware RTU fail

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    30-May-2012      Created initial revision

-----------------------------------------------------------------------------*/
E_ERR_T SYS_UpdateHardwareRTC(VOID)
{
  INT32 rval = 0;

  rval = spawnl(P_WAIT, "/sbin/rtc", "/sbin/rtc", "-s", "hw", NULL);
  if(rval == TYP_ERROR)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SYS] SYS_UpdateRTC, spawnl err %s\n", strerror(errno));
    #endif // CFG_PRN_ERR
    return E_ERR_SYS_UpdateHardwareRTCFail;
  }

  return E_ERR_Success;
} // SYS_UpdateHardwareRTC
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_GetDefaultThrdName

 DESCRIPTION

  This routine will return the name of the calling thread

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  None

 RETURN

  CHAR pointer to the name of the calling thread

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      01-Mar-2012      Created initial revision

----------------------------------------------------------------------------*/
CHAR * SYS_GetDefaultThrdName(CHAR *pbuff, INT32 buffsz)
{

  if(EOK != pthread_getname_np(0, pbuff, buffsz))
  {
    return TYP_NULL;
  }
  return pbuff;
} //SYS_GetDefaultThrdName
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_ExecShutDown

 DESCRIPTION

  This routine will reboot the syste.
  The routine is equivalent to invoke the following command at command shell,
  # shutdown

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  none

 RETURN

  E_ERR_Success                    the routine executed successfully


 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    30-May-2012      Created initial revision

-----------------------------------------------------------------------------*/
E_ERR_T SYS_ExecShutDown(VOID)
{
  INT32 rval = 0;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SYS)
  printf("[SYS] SYS_ExecShutDown, exec shutdown\n");
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SYS)
  //rval = spawnl(P_WAIT, "/bin/shutdown", "/bin/shutdown", "-f", NULL);

  rval = spawnl(P_WAIT, "/bin/shutdown", "/bin/shutdown", NULL);

 // rval = spawnl(P_WAIT, "/RTU", "slay rtu_rel", NULL);	//20151215 Su
  if(rval == TYP_ERROR)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SYS] SYS_ExecShutDown, spawnl err %s\n", strerror(errno));
    #endif // CFG_PRN_ERR
    return E_ERR_SYS_UpdateHardwareRTCFail;
  }

  return E_ERR_Success;
} //SYS_ExecShutDown
//#ifdef NOT_USED
///*-----------------------------------------------------------------------------
//
// PUBLIC ROUTINE
//
//  SYS_SetMutex
//
// DESCRIPTION
//
//  This routine will lock and unlock mutex.
//
// CALLED BY
//
//  Application
//
// CALLS
//
//  None
//
// PARAMETER
//
//  None
//
// RETURN
//
//  E_ERR_Success             Success
//  E_ERR_SYS_InvalidMutexID  Invalid mutex ID
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
//   Bryan Chong    23-Apr-2010      Created initial revision
//
//-----------------------------------------------------------------------------*/
//E_ERR_T SYS_SetMutex(const E_SYS_MTX mutexID, const BOOL_T isLock)
//{
//  switch(mutexID)
//  {
//    case E_SYS_MTX_SER_SendWithReply:
//      //#ifdef ENABLE_SER_MTX
//      isLock ?
//      pthread_mutex_lock(
//        &pSYS_CB->mutex_ctrl[E_SYS_MTX_SER_SendWithReply].mtx):
//      pthread_mutex_unlock(
//        &pSYS_CB->mutex_ctrl[E_SYS_MTX_SER_SendWithReply].mtx);
//      //#endif // ENABLE_SER_MTX
//      break;
//
//    case E_SYS_MTX_LAN_SendWithReply:
//      //#ifdef ENABLE_LAN_MTX
//      isLock ?
//      pthread_mutex_lock(
//        &pSYS_CB->mutex_ctrl[E_SYS_MTX_LAN_SendWithReply].mtx):
//      pthread_mutex_unlock(
//        &pSYS_CB->mutex_ctrl[E_SYS_MTX_LAN_SendWithReply].mtx);
//      //#endif // ENABLE_LAN_MTX
//      break;
//
//    default:
//      #ifdef CFG_PRN_ERR
//      printf("ERR  [SYS]SYS_LockMutex, invalid mutex ID\n");
//      #endif // CFG_PRN_ERR
//      return E_ERR_SYS_InvalidMutexID;
//
//  }
//  return E_ERR_Success;
//} // SYS_SetMutex
///*----------------------------------------------------------------------------
//
// PUBLIC ROUTINE
//
//  SYS_GetTimeInString
//
// DESCRIPTION
//
//  This routine will return a pointer reference to the designated time
//  break-down string.
//
// CALLED BY
//
//  Application
//
// CALLS
//
//  None
//
// PARAMETER
//
//  poutbdtime      [out] pointer to a pointer to store return address from gmtime
//  ptspec          [in] pointer to a timespec structure variable with the
//                       designated time
//  time_t          [in] type of time format. Local or UTC
//
// RETURN
//
//  E_ERR_Success   routine executed successfully
//
// AUTHOR
//
//  Bryan K.W. Chong
//
//
// HISTORY
//
//    NAME            DATE                    REMARKS
//
// Bryan Chong      10-Feb-2011      Created initial revision
//
//----------------------------------------------------------------------------*/
//E_ERR_T SYS_GetTimeInString(struct tm **poutbdtime, struct timespec *ptspec,
//                            E_SYS_TIME time_type)
//{
//
//  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_TIME))
//  CHAR *tz;
//  #endif // CFG_DEBUG_MSG
//
//  if((poutbdtime == TYP_NULL) || (ptspec == TYP_NULL))
//    return E_ERR_InvalidNullPointer;
//
//  switch(time_type)
//  {
//    case E_SYS_TIME_Local:
//      // get local time format
//      *poutbdtime = localtime(&ptspec->tv_sec);
//      break;
//
//    case E_SYS_TIME_UTC:
//      // get UTC time format
//      *poutbdtime = gmtime((const time_t*)ptspec);
//      break;
//
//    default:
//      return E_ERR_SYS_InvalidTimeType;
//  } // switch(time_type)
//
//  if(*poutbdtime == TYP_NULL)
//    return E_ERR_InvalidNullPointer;
//
//  return E_ERR_Success;
//} // SYS_GetCurrentTimeInString
///*-----------------------------------------------------------------------------
//
// PUBLIC ROUTINE
//
//  SYS_GetDataRate
//
// DESCRIPTION
//
//  This routine will compute number of bytes per second, data rate
//
// CALLED BY
//
//  Application
//
// CALLS
//
//  None
//
// PARAMETER
//
//  None
//
// RETURN
//
//  None
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
//   Bryan Chong    26-Feb-2010      Created initial revision
//
//-----------------------------------------------------------------------------*/
//E_ERR_T SYS_GetDataRate(const struct timespec *ptstart,
//                       const struct timespec *ptend,
//                       const UINT32 *ptotalbytes,
//                       FP64 *poutbyterate)
//{
//  struct timespec tdelta;
//  FP64 second;
//
//  if((ptstart == TYP_NULL) || (ptend == TYP_NULL) || (ptotalbytes == TYP_NULL))
//    return E_ERR_InvalidNullPointer;
//
//  if(*ptotalbytes == TYP_NULL)
//    return E_ERR_InvalidParameter;
//
//  if(ptstart->tv_sec > ptend->tv_sec)
//    return E_ERR_SYS_StartSecGreaterThanEndSec;
//
//  tdelta.tv_sec = ptend->tv_sec - ptstart->tv_sec;
//
//  if(ptstart->tv_nsec > ptend->tv_nsec)
//  {
//    tdelta.tv_nsec = ptend->tv_nsec + (1000000000 - ptstart->tv_nsec);
//  }else{
//    tdelta.tv_nsec = ptend->tv_nsec - ptstart->tv_nsec;
//  }
//
//  second = tdelta.tv_sec + (1e-9 * tdelta.tv_nsec);
//
//  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SYS)
//  printf("[SYS] SYS_GetDataRate, tStart: (%d, %d), tEnd: (%d, %d)\n",
//         ptstart->tv_sec, ptstart->tv_nsec, ptend->tv_sec, ptend->tv_nsec);
//  printf("[SYS] SYS_GetDataRate, delta second: %d, delta nsec = %d, "
//         "second = %.09f\n",
//         tdelta.tv_sec, tdelta.tv_nsec, second);
//  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_FLG))
//
//  if(second == 0)
//    return E_ERR_SYS_DividedByZeroException;
//
//  *poutbyterate = (FP64)*ptotalbytes/second;
//
//  return E_ERR_Success;
//} // SYS_GetDataRate
///*-----------------------------------------------------------------------------
//
// PUBLIC ROUTINE
//
//  SYS_RestoreClockFromHardwareRTC
//
// DESCRIPTION
//
//  This routine will restore CLOCK_REALTIME from the bios hardware clock.
//  The routine is equivalent to invoke the following command at command shell,
//  # rtc -l hw
//
// CALLED BY
//
//  Application
//
// CALLS
//
//  None
//
// PARAMETER
//
//  none
//
// RETURN
//
//  E_ERR_Success                    the routine executed successfully
//  E_ERR_SYS_UpdateHardwareRTCFail  update hardware RTU fail
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
//   Bryan Chong    30-May-2012      Created initial revision
//
//-----------------------------------------------------------------------------*/
//E_ERR_T SYS_RestoreClockFromHardwareRTC(VOID)
//{
//  INT32 rval = 0;
//
//  rval = spawnl(P_WAIT, "/sbin/rtc", "/sbin/rtc", "-l", "hw", NULL);
//  if(rval == TYP_ERROR)
//  {
//    #ifdef CFG_PRN_ERR
//    printf("ERR  [SYS] SYS_RestoreClockFromHardwareRTC, spawnl err %s\n",
//           strerror(errno));
//    #endif // CFG_PRN_ERR
//    return E_ERR_SYS_UpdateHardwareRTCFail;
//  }
//  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SYS)
//  printf("[SYS] SYS_RestoreClockFromHardwareRTC, load hw rtc, %d\n", rval);
//  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SYS)
//
//  return E_ERR_Success;
//} // SYS_RestoreClockFromHardwareRTC
//#endif // NOT_USED
