/*----------------------------------------------------------------------------

            Copyright (c) 2009 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   rmm_ext.h                                              D1.0.4

 COMPONENT

   n/a

 DESCRIPTION

   This file contains the external declaration for Redundancy Module Management


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     14-Oct-2009      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef _RMM_EXT_H_
#define _RMM_EXT_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*----------------------------------------------------------------------------
  Public variable definitions
----------------------------------------------------------------------------*/
extern eCurrentState g_eCurrentState;
//extern CRTUStatus *g_pRTUStatusTable;


/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  RMM_HealthStatus_Query

 DESCRIPTION

  This routine will query RMM health status

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

 Bryan Chong      14-Oct-2009      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T RMM_HealthStatus_Query(VOID *pin, VOID *pout, UINT16 sz_in, 
                                     UINT16 sz_out);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  RMM_Config_Status

 DESCRIPTION

  This routine will configure RMM status

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

 Bryan Chong      14-Oct-2009      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T RMM_Config_Status(VOID *pin, VOID *pout, UINT16 sz_in,
                                UINT16 sz_out);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  RMM_Notation_Query

 DESCRIPTION

  This routine will query RMM notation (weightage)

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

 Bryan Chong      13-Apr-2010      Created initial revision
 

----------------------------------------------------------------------------*/
extern E_ERR_T RMM_Notation_Query(VOID *pin, VOID *pout, UINT16 sz_in, 
                                 UINT16 sz_out);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  RMM_StatusTable_Query

 DESCRIPTION

  This routine will query RTU Status Table

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

 Bryan Chong      06-May-2010      Created initial revision
 

----------------------------------------------------------------------------*/
extern E_ERR_T RMM_StatusTable_Query(VOID *pin, VOID *pout, UINT16 sz_in, 
                                    UINT16 sz_out);                  
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  RMM_GetStateString

 DESCRIPTION

  This routine will convert state integer to string format. The designated
  output buffer will be initialized before writing

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  stateval          [in] State in integer format
  poutStateString   [out] pointer to output string buffer

 RETURN

  E_ERR_Success                  the routine executed successfully
  E_ERR_InvalidParameter         invalid state value
  E_ERR_InputBufferSizeUnderrun  output buffer size under defined

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      26-May-2011      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T RMM_GetStateString(const UINT8 stateval, CHAR *poutStateString,
                                  const UINT32 outbuffsz);
#ifdef __cplusplus
}
#endif

#endif //_RMM_EXT_H_
