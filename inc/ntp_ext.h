/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   ntp_ext.h                                           D1.0.4

 COMPONENT

   NTP - Network Time Protocol for client

 DESCRIPTION

   This file contains the external declaration for Network Time Protocol
   client management


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     25-Jan-2010      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef NTP_EXT_H
#define NTP_EXT_H

#ifdef __cplusplus
extern "C"
{
#endif

/*----------------------------------------------------------------------------
  Global variable definition
----------------------------------------------------------------------------*/
extern NTP_CB *pNTP_CB;
/*----------------------------------------------------------------------------
  Public prototype declaration
----------------------------------------------------------------------------*/
extern E_ERR_T NTP_Initialization(VOID);

//extern FP64 NTP_Fpst32ToFp64(struct fixedpt32_st sfp);
extern struct fixedpt32_st NTP_Fp64ToFpst32(FP64 d);
extern FP64 NTP_Fpst64ToFp64(struct fixedpt64_st lfp);
extern struct fixedpt64_st NTP_Fp64ToFpst64(FP64 d);
/*----------------------------------------------------------------------------
  SCPI supporting routine
----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  NTP_Status_Query

 DESCRIPTION

  This routine will request NTP information from server

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

 Bryan Chong      31-Dec-2009      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T NTP_Status_Query(VOID *pin, VOID *pout, UINT16 sz_in,
                               UINT16 sz_out);
extern VOID *NTP_PollingThrdFunc(VOID *pThreadParameter);
extern VOID *NTP_LinkManagementThrdFunc(VOID *pThreadParameter);
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  NTP_RxCmdTxRespThrdFunc

 DESCRIPTION

  This routine will listen to external system and response accordingly

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

   Bryan Chong    20-May-2010      Created initial revision

-----------------------------------------------------------------------------*/
extern VOID *NTP_RxCmdTxRespThrdFunc (VOID *pThreadParameter);
#ifdef __cplusplus
}
#endif
#endif // NTP_EXT_H
