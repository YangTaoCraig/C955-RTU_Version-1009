/*----------------------------------------------------------------------------

            Copyright (c) 2009 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of 
 this material. All manufacturing, reproduction, use, and sales rights 
 pertaining to this subject matter are governed by the license agreement. 
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  sys_ass.h                                                 D1.0.4

 COMPONENT

  n/a

 DESCRIPTION

  This file contains the definitions for assertion

 AUTHOR

  Bryan K. W. Chong


 HISTORY

    NAME            DATE                    REMARKS

  Bryan Chong     03-Aug-2009      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef SYS_ASS_H
#define SYS_ASS_H

#ifdef __cplusplus
extern "C"
{
#endif
#ifdef CFG_SYS_ASSERT
#define SYS_ASSERT_PRINTF
#else
#define SYS_ASSERT(module_string, exp) ((void)0)
#endif // SYS_ASSERT_PRINTF


#ifdef SYS_ASSERT_NKDBGPRNW
#define SYS_ASSERT(module, exp) \
  ((void)((exp)?1:( \
       NKDbgPrintfW ( TEXT("%s assertion failed in file %s at line %d \r\n"), \
                      (LPWSTR)module, __FILE__ ,__LINE__ ),    \
                       DebugBreak(), 0  \
                     ) \
   ))
#endif // SYS_ASSERT

#ifdef SYS_ASSERT_PRINTF
#define SYS_ASSERT(module_string, exp) \
  ((void)((exp)?1:( \
  printf ( "%s assertion failed in file %s at line %d\n", \
           (CHAR *)module_string, __FILE__ ,__LINE__ ),    \
           0, 0  \
          ) \
   ))
/*
#define SYS_ASSERT(module_string, exp) \
  ((void)((exp)?1:( \
  printf ( "%s assertion failed in file %s at line %d\n", \
           (CHAR *)module_string, __FILE__ ,__LINE__ ),    \
           DebugBreak(), 0  \
          ) \
   ))*/

#endif // SYS_ASSERT

#ifdef CFG_SYS_ASSERT_RETURN
#define SYS_ASSERT_RETURN(exp, ret_err_code) \
   if(exp){return ret_err_code;}
  //((void)((exp)?(return ret_err_code;):0))
#else
#define SYS_ASSERT_RETURN(exp, ret_err_code) ((VOID)0)
#endif

#ifdef CFG_SYS_ASSERT_PRN_RETURN
#define SYS_ASSERT_PRN_RETURN(exp, err_string, ret_err_code) \
  if(exp){printf("%s\n", err_string); return ret_err_code;}
  //if(exp){return ret_err_code;} 
#else
#define SYS_ASSERT_PRN_RETURN(exp, err_string, ret_err_code) ((VOID)0)
#endif



#ifdef __cplusplus
}
#endif

#endif // SYS_ASS_H
