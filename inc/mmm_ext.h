/*----------------------------------------------------------------------------

            Copyright (c) 2009 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   mmm_ext.h                                           D1.0.4

 COMPONENT

   n/a

 DESCRIPTION

   This file contains the external declaration for SWC Management


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     05-Jan-2011      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef _MMM_EXT_H_
#define _MMM_EXT_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*----------------------------------------------------------------------------
  Public variable definitions
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
  SCPI supporting routine
-----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  MMM_Status_Query

 DESCRIPTION

  This routine will generate all global listing into a designated text file

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

 Bryan Chong      05-Jan-2011      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T MMM_Status_Query (VOID *pin, VOID *pout, UINT16 sz_in, 
                                UINT16 sz_out);
#ifdef __cplusplus
}
#endif

#endif //_SWC_EXT_H_
