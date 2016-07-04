/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics Ltd.

 PROPRIETARY RIGHTS of ST Electronics Ltd are involved in the subject matter
 of this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  errm.cpp                                               D1.0.4

 COMPONENT

  ERR - Application System

 DESCRIPTION

  This file consists of management routine for error handling routine

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

  Bryan Chong    06-Oct-2009        Initial revision

----------------------------------------------------------------------------*/
#include <stdio.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "err_ext.h"
#include "err_lut.h"

/*----------------------------------------------------------------------------
  Public variables declaration
----------------------------------------------------------------------------*/
E_ERR_T ERR_GetMsgString(E_ERR_T err_no, CHAR *poutstr);

/*----------------------------------------------------------------------------
  Public Prototypes declaration
----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
  Local variables declaration
----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
  Local Prototypes declaration
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Local Look-up table declaration
----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------

 PRIVATE ROUTINE

  ERR_GetMsgString

 DESCRIPTION

  This routine will acquire the error message string

 CALLED BY

  Application main

 CALLS

  [TBD]

 PARAMETER

  err_no   [in] error number
  poutstr  [out] pointer to output buffer

 RETURN

   E_ERR_Success      the routine executed successfully

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    06-Oct-2009      Created initial revision

-----------------------------------------------------------------------------*/
E_ERR_T ERR_GetMsgString(E_ERR_T err_no, CHAR *poutstr)
{
  //E_ERR_T rstatus = E_ERR_Success;
  const ERR_LUT_T *perrlut;
  
  perrlut = ERR_ErrorLut;
  
  if((poutstr == TYP_NULL) || (perrlut == TYP_NULL))
    return E_ERR_InvalidNullPointer;
    
  while((perrlut->label != err_no) &&
        (perrlut->label != E_ERR_EndOfList))
  {
    perrlut++;
  }

  if(perrlut->label != E_ERR_EndOfList)
  {
    sprintf(poutstr, "%s(%d) ", perrlut->desc, err_no);
  }else{
    sprintf(poutstr, "ERR %d Not Defined ", err_no);
  }

  return E_ERR_Success;
} // ERR_GetMsgString

