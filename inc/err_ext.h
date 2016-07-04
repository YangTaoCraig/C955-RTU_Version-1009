/*----------------------------------------------------------------------------

            Copyright (c) 2009 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   err_ext.h                                                D1.0.4

 COMPONENT

   ERR - Error Management

 DESCRIPTION

   This file contains the external declaration for error management


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     06-Oct-2009      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef ERR_EXT_H
#define ERR_EXT_H

#ifdef __cplusplus
extern "C"
{
#endif
/*----------------------------------------------------------------------------
  Global variable definition
----------------------------------------------------------------------------*/
extern E_ERR_T ERR_GetMsgString(E_ERR_T err_no, CHAR *poutstr);
#ifdef __cplusplus
}
#endif
#endif // ERR_EXT_H

