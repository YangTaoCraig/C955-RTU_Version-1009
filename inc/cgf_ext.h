/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics Ltd.

 PROPRIETARY RIGHTS of ST Electronics Ltd are involved in the subject matter
 of this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  cgf_ext.h                                            D1.0.4

 COMPONENT

  CGF - Configuration File

 DESCRIPTION

  This file consists of external declaration for the CGF component


 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

  Bryan Chong    10-Feb-2010        Initial revision

----------------------------------------------------------------------------*/
#ifndef CGF_EXT_H
#define CGF_EXT_H

#ifdef __cplusplus
extern "C"
{
#endif
/*----------------------------------------------------------------------------
  Global variable definition
----------------------------------------------------------------------------*/
extern CGF_CB *pCGF_CB;
/*----------------------------------------------------------------------------
  Public prototype declaration
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  CGF_Initialization

 DESCRIPTION

  This routine will initialize control block for the configuration file

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

   Bryan Chong    10-Feb-2009      Created initial revision

-----------------------------------------------------------------------------*/
extern E_ERR_T CGF_Initialization(VOID);
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

-----------------------------------------------------------------------------*/
extern E_ERR_T CGF_GetNTPParameter(FILE *pfd);

#ifdef __cplusplus
}
#endif

#endif // CGF_EXT_H