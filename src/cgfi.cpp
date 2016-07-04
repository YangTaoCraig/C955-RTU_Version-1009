/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics Ltd.

 PROPRIETARY RIGHTS of ST Electronics Ltd are involved in the subject matter
 of this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  cgfi.cpp                                             D1.0.4

 COMPONENT

  CGF - Configuration File

 DESCRIPTION

  This file consists of iniitialization routine for Configuration File


 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

  Bryan Chong    10-Feb-2010        Initial revision

----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/neutrino.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "sys_ass.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
#include "cgf_def.h"
#include "cgf_ext.h"

#include "Common.h"
#include "Init_Config.h"

/*----------------------------------------------------------------------------
  Public variables declaration
----------------------------------------------------------------------------*/
CGF_CB *pCGF_CB;

/*----------------------------------------------------------------------------
  Public Prototypes declaration
----------------------------------------------------------------------------*/
E_ERR_T CGF_Initialization(VOID);

/*----------------------------------------------------------------------------
  Local variables declaration
----------------------------------------------------------------------------*/
static CGF_CB cgfCtrlBlk;

/*----------------------------------------------------------------------------
  Local Prototypes declaration
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
E_ERR_T CGF_Initialization(VOID)
{
  E_ERR_T rstatus = E_ERR_Success;

  pCGF_CB = &cgfCtrlBlk;

  memset(pCGF_CB, 0, sizeof(CGF_CB));
  return rstatus;
} // CGF_Initialization
