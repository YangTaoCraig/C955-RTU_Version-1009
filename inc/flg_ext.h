/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   flg_ext.h                                           D1.0.4

 COMPONENT

   FLG - File Logging

 DESCRIPTION

   This file contains the external declaration for file logging component


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     14-Jan-2010      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef FLG_EXT_H
#define FLG_EXT_H

#ifdef __cplusplus
extern "C"
{
#endif

/*----------------------------------------------------------------------------
  Global variable definition
----------------------------------------------------------------------------*/
extern FLG *pFLG_CtrlBlk;
/*----------------------------------------------------------------------------
  Public prototype declaration
----------------------------------------------------------------------------*/
extern VOID *FLG_ReceivingThrdFunc (VOID *pThreadParameter);

/*----------------------------------------------------------------------------
  SCPI support prototype declaration
----------------------------------------------------------------------------*/
extern E_ERR_T FLG_LogStatus_Query  (VOID *pin, VOID *pout, UINT16 sz_in,
                                    UINT16 sz_out);
extern E_ERR_T FLG_SetLogLevel      (VOID *pin, VOID *pout, UINT16 sz_in, 
                                    UINT16 sz_out);
/*----------------------------------------------------------------------------
  SCPI supporting routine
----------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
#endif // FLG_EXT_H

