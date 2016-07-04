/*----------------------------------------------------------------------------

            Copyright (c) 2009 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   str_ext.h                                                D1.0.4

 COMPONENT

   n/a

 DESCRIPTION

   This file contains the external declaration for string management


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     09-Sep-2009      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef STR_EXT_H
#define STR_EXT_H

#ifdef __cplusplus
extern "C"
{
#endif
/*----------------------------------------------------------------------------
  Global variable definition
----------------------------------------------------------------------------*/
//extern E_ERR_T STR_CopyNullTerminatedBuffer (CHAR *pdest,
//                                            CHAR *psrc,
//                                            UINT16 destmaxsz,
//                                            UINT16 *prcnt);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  STR_CompareBufferString

 DESCRIPTION

  This routine will check if the strings are identical. The terminated
  character can be \0 or \n.
  This will allow flexbility for a command to either have parameter or
  without any parameter

 CALLED BY

  Application

 CALLS

  None

 PARAMETERS

  porg          [in] the original string
  pbuf          [in] the target string to be compared with the original string
  pcompare_sz   [out] number of characters from buffer are compared
  prcmpresult   [out] the output result of the comparison. TRUE or FALSE.

 RETURN

  E_ERR_Success   Scope system initialized successfully

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE               REMARKS

 Bryan Chong      19-Dec-2008      Initial revision
----------------------------------------------------------------------------*/
//extern E_ERR_T STR_CompareBufferString (const CHAR *porg, const CHAR *pbuf,
//                                   UINT16 *pcompare_sz, BOOL_T *prcmpresult);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  STR_AppendString

 DESCRIPTION

  This routine will append the target string to target buffer with optional
  suffix. The append process will not exceed the specified tgtMaxBuffSz.
  The appending process is limited to the defined STR_APPEND_STRING_BUFF_SZ.

 CALLED BY

  Application

 CALLS

  None

 INPUTS

  ptgtBuff          [in] target buffer
  pappdChar         [in] first found character to be replaced by appending
                         string
  pappdString       [in] string to be appended to target buffer
  pappdPrefix       [in] string to be appended before the appending string.
                         Must be null terminated.
  pappdSuffix       [in] suffix to be appended to target buffer after
                         string appended. Must be null terminated.
  appdStringLength  [in] the length of the string to be appended
  tgtMaxBuffSz      [in] maximum size of the target buffer


 OUTPUTS

  pisFoundAppdChar  [out] is the replaced character found? (YES, NO)
  prtotalappdcnt    [out] Total number of bytes appended

  E_ERR_Success     Scope system initialized successfully

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      28-Dec-2007      Created initial revision

----------------------------------------------------------------------------*/
//extern E_ERR_T STR_AppendString(CHAR *ptgtBuff, CHAR *pappdChar,
//                               CHAR *pappdString, CHAR *pappdPrefix,
//                               CHAR *pappdSuffix, UINT16 appdStringLength,
//                               UINT16 tgtMaxBuffSz, BOOL_T *pisFoundAppdChar,
//                               UINT16 *prtotalappdcnt);
/*------------------------------------------------------------------------------

 PUBLIC ROUTINE

  STR_DecodeRegisterBit

 DESCRIPTION

  This routine will print decoded message into a designated cumulative buffer.
  The short buffer size must be larger than the length of the longest defined
  sentence in the look-up table (LUT). The short buffer is a working buffer.
  The routine will gather all the relevant sentence from the LUT and append to
  the cumulative buffer

  The routine support decoding of bits for 8-bit, 16-bit, and 32-bit
  registers only.

  The routine will detect if buffer would overrun before appending to
  cumulative buffer


  TODO: Check buffer overrun for pshorbuff
 CALLED BY

  Application

 CALLS

  None

 PARAMETERS

  preg         [in] pointer to the register
  plut         [in] pointer to the look-up table that host the bit
                    description for 0 and 1 cases
  pshortbuff   [in] pointer to a temporary buffer. The length must be longer
                    than the longest description in the LUT
  shortbuffsz  [in] the length of the working buffer.
  pcumbuff     [out] pointer to the cumulative buffer. It is recommend to
                     define the size to be as large as the LUT
  cumbuffsz    [in] cumulative buffer size
  bytesz       [in] number of bytes to be decoded. Valid size is 1, 2, and 4.

 RETURN

  E_ERR_Success                      routine executes successfully
  E_ERR_STR_InvalidDecodeByteSize    Invalid byte size

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      24-Nov-2009      Created initial revision

------------------------------------------------------------------------------*/
//extern E_ERR_T STR_DecodeRegisterBit(const UINT32 reg, const STR_BD *plut,
//                             CHAR *pshortbuff, const UINT32 shortbuffsz,
//                             CHAR *pcumbuff, const UINT32 cumbuffsz,
//                             const UINT8 bytesz);
/*------------------------------------------------------------------------------

 PUBLIC ROUTINE

  STR_SwapEndianessBuffer

 DESCRIPTION

  This routine will swap the designated endianess and load the data
  to the designated buffer.

  For example if source buffer consist of
   {0x0201, 0x0403, 0x0607}

  destination buffer will become
   {0x0102, 0x0304, 0x0706}

 CALLED BY

  Application

 CALLS

  None

 PARAMETERS

  pdest            [in] pointer to the destination buffer
  psource          [in] pointer to the source buffer
  buffer_sz        [in] buffer size in bytes
  byte_conv_type   [in] byte convertion type

 RETURN

  E_ERR_Success                      routine executes successfully

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      20-May-2009      Created initial revision

------------------------------------------------------------------------------*/
extern E_ERR_T STR_SwapEndianessBuffer(VOID *pdest, VOID *psource,
                                      UINT32 buffer_sz,
                                      STR_BYTE_CONV_T conv_type);
/*------------------------------------------------------------------------------

 PUBLIC ROUTINE

  STR_ByteSwap16Bit

 DESCRIPTION

  This routine will return the byte swap of the designated 16-bit word
  Eg. 0x0201 return 0x0102

 CALLED BY

  Application

 CALLS

  None

 PARAMETERS

  wordarg          [in] 16-bit word

 RETURN

  16-bit word after byte swap

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Yu, Wei          DD-MM-2003      Created initial revision

------------------------------------------------------------------------------*/
extern UINT16 STR_ByteSwap16Bit(UINT16 wordarg);
#ifdef __cplusplus
}
#endif

#endif // STR_EXT_H
