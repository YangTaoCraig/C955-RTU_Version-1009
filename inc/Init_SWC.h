/************************************************************
*															*
*	Project:    C830 RTU									*
*	Module:		Initialization (INM)						*
*	File :	    Init_SWC.h									*
*	Author:		Yu Wei										*
*															*
*	Copyright 2003 Singapore Technologies Electronics Ltd.	*
*************************************************************


**************************************************************

DESCRIPTION

	This is header file for Init_SWC.cpp routine defination.

*************************************************************
Modiffication History
---------------------

Version: INM D.1.1.1
	01	1 April 2003, Yu Wei,
		Start to write.

Version: INM D.1.3.0
	01	31 July 2003, Yu Wei
		All SWC use same class except Clock.

Version: INM D.1.3.1
	01	26 September 2003, Yu Wei
		Move tTaskParameter to this file from common.h

**************************************************************/
#ifndef  _INIT_SWC_H
#define  _INIT_SWC_H

#include "Common.h"
#include "SWC.h"
#include "SWC_LAN.h"
#include "CMM_Log.h"

#ifdef __cplusplus
extern "C"
{
#endif


//Task parameters
struct tTaskParameter
{
  char  acName[16];    //Task name.
  int    nPriority;    //Task priority.
  int    nStackSize;    //Stack size.
};

int SWCInitialization(void);
int InitializeOneSWC(int);

void StopAllSWC(void);

/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  INM_GetSWCIndex

 DESCRIPTION

  This routine will return SWC index for the designated SWC's label

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  pswclable  [in] pointer to SWC's name
  poutswcidx [out] return of SWC index value through user defined pointer

 RETURN

  E_ERR_Success             the routine executed successfully
  E_ERR_InvalidNullPointer  invalid null pointer

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    04-Aug-2011      Created initial revision

-----------------------------------------------------------------------------*/
//extern E_ERR_T INM_GetSWCIndex(CHAR *pswclabel, INT8 *poutswcidx);

#ifdef __cplusplus
}
#endif


#endif /* _INIT_SWC_H */


