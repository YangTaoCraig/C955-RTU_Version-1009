/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics Ltd.

 PROPRIETARY RIGHTS of ST Electronics Ltd are involved in the subject matter
 of this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  entry.cpp                                            D1.0.4

 COMPONENT

  None

 DESCRIPTION

  This file consists of the entry point to unit test or application main
  routine

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

  Bryan Chong    08-Mar-2010            Initial revision

----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
#include "err_ext.h"
#include <semaphore.h>
#include <fcntl.h>		//20151117 YT semaphore
int main(int argc, char *argv[])
{
  CHAR *pfile;

  pfile = (CHAR *)argv[1];

  #ifdef CFG_PRN_ERR
  CHAR errmsgbuff[ERR_STR_SZ] = {0};
  #endif // CFG_PRN_ERR

  E_ERR_T rstatus = E_ERR_Success;

  //20151117 YT semaphore
  sem_t * sem;
  int rsem;
  sem = sem_open("/EntryPoint", (O_CREAT|O_EXCL), 0777, 1);
  if(sem == SEM_FAILED)
  {
	  perror("The user attempts to run the application, multiple instances.\n");
	  return EXIT_SUCCESS;
  }
  rsem = sem_wait(sem);
  if(rsem != 0)
  {
	  perror("The state of the semaphore is unchanged. ");
	  return EXIT_SUCCESS;
  }
  else
  {
	  perror("sem_wait OK! ");
  }
  //20151117 YT semaphore


  rstatus = SYS_Main(pfile);

  #ifdef CFG_PRN_ERR
  ERR_GetMsgString(rstatus, errmsgbuff);
  printf("ERR  [entry] main, exit error %s\n", errmsgbuff);
  #endif // CFG_PRN_ERR

  return EXIT_SUCCESS;
}// main




