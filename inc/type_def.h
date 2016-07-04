/*----------------------------------------------------------------------------

             Copyright (c) 2009 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.
----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  type_def.h                                               D1.0.4

 COMPONENT

  n/a

 DESCRIPTION

  This file contains the type definitions for the system

 AUTHOR

  Bryan K. W. Chong


 HISTORY

    NAME            DATE                    REMARKS

  Bryan Chong     03-Aug-2009      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef TYPE_H
#define TYPE_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef NULL
#define NULL    0
//#define NULL    (void *)0
#endif /* NULL */

#define TRUE    1
#define FALSE   0

#define YES     TRUE
#define NO      FALSE

#define ON      TRUE
#define OFF     FALSE

#define ENABLE  TRUE
#define DISABLE FALSE

#define HIGH    TRUE
#define LOW     FALSE

#define TYP_ERROR  -1
#define TYP_NULL    0
#define END_OF_TABLE    0xFF
#define NULL_CHARACTER  '\0'
#define NULL_TERMINATED 0



/* Define standard data types.  These definitions allow current solution to
   perform in the same manner on different target platforms.  */
                                       /* Number of bytes : range */

#ifdef C_OPTION
/* if -c is define at IAR C compiler option */
typedef char                    INT8;         /* 1 : -128 - 127 */
#endif /* C_OPTION */

typedef char                    CHAR, *PCHAR; /* 1 : 0 - 255 */
typedef unsigned char           UCHAR;        /* 1 : 0 - 255 */
typedef unsigned char          *PUCHAR;
typedef unsigned char           UINT8, BYTE;  /* 1 : 0 - 255 */
typedef unsigned char          *PUINT8;
typedef volatile unsigned char  VUINT8;       /* 1 : 0 - 255 */
typedef volatile unsigned char *PVUINT8;
typedef signed char             INT8;         /* 1 : -128 - 127 */
typedef short                   INT16;        /* 2 : -32,768 - 32,767 */
typedef short                   *PINT16;
typedef int                     INT32;        /* 4 : -65,535 - 65,536 */
//typedef int                     INT;          /* 4 : -65,535 - 65,536 */
typedef unsigned int            UINT32;       /* 4 : 0 - 4,294,967,295 */
typedef unsigned int            *PUINT32;
typedef unsigned short          UINT16;       /* 2 : 0 - 65,535 */
typedef volatile unsigned short VUINT16;      /* 2 : 0 - 65,535 */
typedef volatile unsigned int   VUINT32;      /* 4 : 0 - 4,294,967,295  */
//typedef unsigned long           DATA_ELEMENT;
//typedef DATA_ELEMENT            OPTION;       /* 4 : 0 - 4,294,967,295 */
//typedef enum {FALSE = 0, TRUE = 1}BOOL;         /* 1 : TRUE, FALSE */
typedef unsigned char *         BYTE_PTR;     /* 4 : 8-bit register pointer */
typedef unsigned char *         PUINT8;       /* 4 : 8-bit register pointer */
typedef unsigned short         *PUINT16;      /* 4 : 16-bit pointer */
typedef unsigned int *          PUINT32;      /* 4 : 32-bit pointer */
typedef int *                   PINT;         /* 4 : 32-bit pointer */
typedef volatile unsigned short VUINT16;      /* 2 : 0 - 65,535 */
typedef volatile unsigned short *PVUINT16;
//typedef volatile UINT           VUINT, *PVUINT;
//typedef volatile UINT32         VUINT32, *PVUINT32;
//typedef volatile INT            VINT, *PVINT;
typedef unsigned long long      UINT64, *PUINT64;
typedef signed long long        INT64, *PINT64;
typedef volatile UINT64         VUINT64, *PVUINT64;
typedef float                   FP32;
typedef double                  FP64;
//typedef void                    VOID_T;
#define VOID  void
typedef void                    *PVOID_T;
typedef VOID(*FnPVOID)(VOID);
typedef INT32(*FPTR_INT32_VOID)(VOID);


typedef enum boolean_t {
  E_TYPE_Off = 0,
  E_TYPE_On  = 1,
  E_TYPE_False = 0,
  E_TYPE_True = 1,
  E_TYPE_No   = 0,
  E_TYPE_Yes  = 1
} BOOL_T;


#ifdef __cplusplus
}
#endif

#endif /* TYPE_H */
