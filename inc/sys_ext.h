#ifndef _SYS_EXT_H_
#define _SYS_EXT_H_

#ifdef __cplusplus
extern "C"
{
#endif

extern const CHAR **pSYS_WdayStr;

extern SYS_CB_T *pSYS_CB;
extern BOOL_T  SYS_isExitAppl;

//RTU configuration parameters.
extern tSystemConfiguration  g_tRTUConfig;
//RTU State flag and interface link status.
extern tGlobalStatus g_tRTUStatus;
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_Initialization

 DESCRIPTION

  This routine will initialize all component items

 CALLED BY

  Application main

 CALLS

  SYS_Main

 PARAMETER

  pconfigfilename   [in] pointer to config filename

 RETURN

   E_ERR_Success                 the routine executed successfully
   E_ERR_SWC_InitializationFail  Initialization Fail

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    07-Sep-2009      Created initial revision
   Bryan Chong    27-Mar-2012      Add input parameter for config filename
-----------------------------------------------------------------------------*/
extern E_ERR_T SYS_Initialization(CHAR *pconfigfilename);
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_InitiateDevcSer8250Manager

 DESCRIPTION

  This routine will initiate dev-ser8250 manager for the PCI-based serial
  controller.

  Supporting auto detecting of PCI Serial Controller card and issue the
  correct comment to spawnv routine. Successful initialization of
  devc-ser8250 driver will return a process ID.
  acTemp will show the correct command issued at command line.
  index 0: For 1-card configuration. The card can be at either one of the slot.
           Different slot will have different interrupt number depends on
           different device ID
  index 1: For 2-card configuration.

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  None

 RETURN

  None

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    10-Mar-2010      Created initial revision

-----------------------------------------------------------------------------*/
//#if CFG_ENABLE_PCI_DETECTION
//extern E_ERR_T SYS_InitiateDevcSer8250Manager(VOID);
///*-----------------------------------------------------------------------------
//
// PUBLIC ROUTINE
//
//  SYS_DeinitializeDevc8250Manager
//
// DESCRIPTION
//
//  This routine will de-initiate dev-ser8250 manager for the PCI-based serial
//  controller.
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
//   Bryan Chong    10-Mar-2010      Created initial revision
//
//-----------------------------------------------------------------------------*/
//extern E_ERR_T SYS_DeinitializeDevc8250Manager(VOID);
//#endif  // CFG_ENABLE_PCI_DETECTION
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_Main

 DESCRIPTION

  The system application entry point

 CALLED BY

  Application main

 CALLS

  [TBD]

 PARAMETER

  pconfigfilename   [in] pointer to config filename

 RETURN

  None

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    08-Mar-2010      Created initial revision

-----------------------------------------------------------------------------*/
extern E_ERR_T SYS_Main(CHAR *pconfigfilename);
extern E_ERR_T SYS_ThrdInit(E_SYS_TLB thrd_lbl);

extern E_ERR_T SYS_CreateMsgQueue(E_SYS_MSQL msq_label);
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_InitCtrlBlk

 DESCRIPTION

  This routine will initialize the system management control block

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

   Bryan Chong    08-Mar-2010      Created initial revision

-----------------------------------------------------------------------------*/
extern E_ERR_T SYS_InitCtrlBlk(VOID);
extern E_ERR_T SYS_CreateChannel(E_SYS_CHL chlabel);
//extern E_ERR_T SYS_PrnOutputParameter(VOID *pout, UINT16 *psz_out,
//                                     CHAR *pbuffer);
extern E_ERR_T SYS_CreateHeaderString(VOID);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_GetCurrentTimeInString

 DESCRIPTION

  This routine will return a pointer reference to the time break-down string.

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  poutbdtime      [out] pointer to a pointer to store return address from gmtime
  ptspec          [in] pointer to a timespec structure variable
  time_type       [in] type of time format. Local or UTC

 RETURN

  E_ERR_Success   routine executed successfully

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      28-Jan-2010      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T SYS_GetCurrentTimeInString(struct tm **poutbdtime,
                                         struct timespec *ptspec,
                                         E_SYS_TIME time_type);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_GetTimeInString

 DESCRIPTION

  This routine will return a pointer reference to the designated time
  break-down string.

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  poutbdtime      [out] pointer to a pointer to store return address from gmtime
  ptspec          [in] pointer to a timespec structure variable with the
                       designated time
  time_t          [in] type of time format. Local or UTC

 RETURN

  E_ERR_Success   routine executed successfully

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      10-Feb-2011      Created initial revision

----------------------------------------------------------------------------*/
//E_ERR_T SYS_GetTimeInString(struct tm **poutbdtime, struct timespec *ptspec,
//                            E_SYS_TIME time_type);
//extern VOID *SYS_SchedulerThrdFunc(VOID *pThreadParameter);
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
extern E_ERR_T SYS_GetDiskMemorySpace(UINT64 *poutfreediskspace);
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

  None

 RETURN

  None

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    24-Feb-2010      Created initial revision

-----------------------------------------------------------------------------*/
extern E_ERR_T SYS_PrnDataBlock(const UINT8 *pbuff, const UINT32 prnsz,
                               const UINT8 byteperrow);
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
//extern E_ERR_T SYS_PrnDataBlockBuffer(const UINT8 *pbuff, const UINT32 prnsz,
//                                      const UINT8 byteperrow,
//                                      const UINT8 *poutbuff,
//                                      const UINT16 outbuffsz);
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_GetDataRate

 DESCRIPTION

  This routine will compute number of bytes per second, data rate

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  None

 RETURN

  None

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    26-Feb-2010      Created initial revision

-----------------------------------------------------------------------------*/
//E_ERR_T SYS_GetDataRate(const struct timespec *ptstart,
//                       const struct timespec *ptend,
//                       const UINT32 *ptotalbytes,
//                       FP64 *poutbyterate);
#ifdef NOT_USED
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_SetMutex

 DESCRIPTION

  This routine will lock and unlock mutex.

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  None

 RETURN

  E_ERR_Success             Success
  E_ERR_SYS_InvalidMutexID  Invalid mutex ID

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    23-Apr-2010      Created initial revision

-----------------------------------------------------------------------------*/
extern E_ERR_T SYS_SetMutex(const E_SYS_MTX mutexID, const BOOL_T isLock);
#endif // NOT_USED

/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_UpdateRTC

 DESCRIPTION

  This routine will set the Real Time Clock to the date and time according to
  the input of tm format.


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
extern E_ERR_T SYS_CheckToUpdateRTC(struct tm tmMsg);
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_SetRTCTime

 DESCRIPTION

  This routine will set the Real Time Clock to the designated date and time.

  Take note,
  1. month is zero based, 0..11
  2. year since 1900

  The rest are as follows,

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

-----------------------------------------------------------------------------*/
extern E_ERR_T SYS_SetRTCTime(struct tm tmTime);
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
extern E_ERR_T SYS_UpdateHardwareRTC(VOID);
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
extern CHAR * SYS_GetDefaultThrdName(CHAR *pbuff, INT32 buffsz);
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
extern E_ERR_T SYS_ExecShutDown(VOID);
/*----------------------------------------------------------------------------
  Supporting routines for SCPI command
----------------------------------------------------------------------------*/
extern E_ERR_T SYS_IDN_Query         (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);
extern E_ERR_T SYS_Reset             (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  DEV_Help

 DESCRIPTION

  This routine will provide help information

 CALLED BY

  Application

 CALLS

  None

 INPUTS

  pin         [in] pointer to input parameter structure
  sz_in       [in] size of input parameter structure

 OUTPUTS

  pout        [out] pointer to output parameter structure
  sz_out      [out] size of output parameter structure

  E_ERR_Success  the routine executed successfully


 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      25-Aug-2011      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T SYS_Help             (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);

extern E_ERR_T SYS_GetLocalTime      (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);
extern E_ERR_T SYS_SetLocalTime      (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);

extern E_ERR_T SYS_SystemErrorQuery  (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);
extern E_ERR_T SYS_SystemErrorNumber_Query
                                    (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);
extern E_ERR_T SYS_SystemErrorListQuery
                                    (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);

extern E_ERR_T SYS_NetworkInfo_Query (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);
/*
extern E_ERR_T SYS_IsLastCommandComplete
                                    (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);
*/
extern E_ERR_T SYS_Network_IP        (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);
extern E_ERR_T SYS_SystemErrorLogStatus_Query
                                    (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);
extern E_ERR_T SYS_SystemErrorLogStatus
                                    (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);
extern E_ERR_T SYS_ParaConf_Query (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_ParaConf_SetMaxTemperature

 DESCRIPTION

  This routine will update the designated maximum threshold temperature

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  pin         [in] pointer to input parameter structure
  sz_in       [in] size of input parameter structure
  pout        [out] pointer to output parameter structure
  sz_out      [out] size of output parameter structure


 RETURN

  E_ERR_Success  the routine executed successfully


 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      26-Oct-2010      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T SYS_ParaConf_SetMaxTemperature(VOID * pin, VOID * pout,
                                             UINT16 sz_in, UINT16 sz_out);
extern E_ERR_T SYS_RTUIPConf_Query   (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);
extern E_ERR_T SYS_ThreadConfig_Query(VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_ShowThread

 DESCRIPTION

  This routine will list all running threads' name along with their ID and
  other properties

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  pin         [in] pointer to input parameter structure
  sz_in       [in] size of input parameter structure
  pout        [out] pointer to output parameter structure
  sz_out      [out] size of output parameter structure


 RETURN

  E_ERR_Success  the routine executed successfully


 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      09-May-2011      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T SYS_ShowThread_Query(VOID *pin, VOID *pout, UINT16 sz_in,
                                    UINT16 sz_out);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SYS_DeleteThread

 DESCRIPTION

  This routine will delete the designated thread

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  pin         [in] pointer to input parameter structure
  sz_in       [in] size of input parameter structure
  pout        [out] pointer to output parameter structure
  sz_out      [out] size of output parameter structure


 RETURN

  E_ERR_Success  the routine executed successfully


 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      09-May-2011      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T SYS_DeleteThread     (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);
extern E_ERR_T SYS_DiskSpace_Query   (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);
extern E_ERR_T SYS_Quit              (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);
extern E_ERR_T SYS_CodeCoverage     (VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);
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
extern CHAR * SYS_GetDefaultThrdName(CHAR *pbuff, INT32 buffsz);
#ifdef __cplusplus
}
#endif

#endif //_SYS_EXT_H_
