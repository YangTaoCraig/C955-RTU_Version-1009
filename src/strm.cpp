/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics Ltd.

 PROPRIETARY RIGHTS of ST Electronics Ltd are involved in the subject matter
 of this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  strm.cpp                                               D1.0.4

 COMPONENT

  STR - String Management

 DESCRIPTION

  This file consists of management routine for the string management component

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

  Bryan Chong    09-Sep-2009        Initial revision

----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "str_def.h"
#include "str_ext.h"
#include "sys_ass.h"

/*----------------------------------------------------------------------------
  Public Prototype declarations
----------------------------------------------------------------------------*/
//E_ERR_T STR_CopyNullTerminatedBuffer
//    (CHAR *pdest, CHAR *psrc, UINT16 deststringsz, UINT16 *prcnt);

//E_ERR_T STR_CompareBufferString (const CHAR *porg, const CHAR *pbuf,
//                                UINT16 *pcompare_sz, BOOL_T *prcmpresult);
//E_ERR_T STR_AppendString
//    (CHAR *ptgtBuff, CHAR *pappdChar, CHAR *pappdString,
//     CHAR *pappdPrefix, CHAR *pappdSuffix, UINT16 appdStringLength,
//     UINT16 tgtMaxBuffSz, BOOL_T *pisFoundAppdChar, UINT16 *prtotalappdcnt);


//E_ERR_T STR_DecodeRegisterBit(const UINT32 reg, const STR_BD *plut,
//                             CHAR *pshortbuff, const UINT32 shortbuffsz,
//                             CHAR *pcumbuff, const UINT32 cumbuffsz,
//                             const UINT8 bytesz);
E_ERR_T STR_SwapEndianessBuffer(VOID *pdest, VOID *psource, UINT32 buffer_sz,
                               STR_BYTE_CONV_T conv_type);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  STR_CopyNullTerminatedBuffer

 DESCRIPTION

  This routine will copy the original string to a targeted buffer. The process
  will terminate when null character is found from original string or
  it has reached the maximum count of the specified destmaxsz.

 CALLED BY

  Application

 CALLS

  None

 INPUTS

  pdest         [in] the target buffer location
  psrc          [in] the original string to copy
  destmaxsz     [in] number of bytes from the original string to copy


 OUTPUTS

  prcnt       [out] Number of bytes copied from the original string
  E_ERR_Success   Scope system initialized successfully

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      12-Dec-2007      Created initial revision

----------------------------------------------------------------------------*/
//E_ERR_T STR_CopyNullTerminatedBuffer (CHAR *pdest,
//                                     CHAR *psrc,
//                                     UINT16 destmaxsz,
//                                     UINT16 *prcnt)
//{
//  UINT16 cnt = 0;
//
//  if ((pdest == TYP_NULL) | (psrc == TYP_NULL))
//    return E_ERR_InvalidNullPointer;
//
//  while((cnt < destmaxsz) && (*psrc != (CHAR)TYP_NULL))
//  {
//    *(pdest++) = *(psrc++);
//    cnt++;
//  }
//
//  *prcnt = cnt;
//
//  return E_ERR_Success;
//} // STR_CopyNullTerminatedBuffer
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
  E_ERR_InvalidNullPointer  invalid null pointer

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE               REMARKS

 Bryan Chong      19-Dec-2008      Initial revision
----------------------------------------------------------------------------*/
//E_ERR_T STR_CompareBufferString (const CHAR *porg, const CHAR *pbuf,
//                                UINT16 *pcompare_sz, BOOL_T *prcmpresult)
//{
//  UINT8 down_cnt;
//
//  down_cnt = STR_CMP_MAX_SZ;
//  if((porg == TYP_NULL) || (pbuf == TYP_NULL))
//    return E_ERR_InvalidNullPointer;
//
//  while((*porg != '\0') && (*porg == *pbuf) && (down_cnt > 0))
//  {
//    porg++;
//    pbuf++;
//    down_cnt--;
//  }
//
//  if((*porg == '\0') && (*pbuf == '\0') && (down_cnt > 0))
//    *prcmpresult = E_TYPE_True;
//  else if((*porg == '\0') && (*pbuf == '\n') && (down_cnt > 0))
//    *prcmpresult = E_TYPE_True;
//  else
//    *prcmpresult = E_TYPE_False;
//
//  *pcompare_sz = STR_CMP_MAX_SZ - down_cnt;
//
//  return E_ERR_Success;
//}  // STR_CompareBufferString
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

 PARAMETER

  ptgtBuff          [in] target buffer
  pappdChar         [in] first found character to be replaced by appending
                         string
  pappdString       [in] target string to be appended to target buffer
  pappdPrefix       [in] string to be appended before the appending string.
                         Included null terminated character '\0'.
  pappdSuffix       [in] suffix to be appended to target buffer after
                         string appended. Included null terminated character
                         '\0'.
  appdStringLength  [in] the length of the string to be appended
  tgtMaxBuffSz      [in] maximum size of the target buffer
  pisFoundAppdChar  [out] is the replaced character found? (YES, NO)
  prtotalappdcnt    [out] Total number of bytes appended

  E_ERR_Success     Scope system initialized successfully

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      28-Dec-2007      Created initial revision

----------------------------------------------------------------------------*/
//E_ERR_T STR_AppendString(CHAR *ptgtBuff, CHAR *pappdChar, CHAR *pappdString,
//                        CHAR *pappdPrefix, CHAR *pappdSuffix,
//                        UINT16 appdStringLength, UINT16 tgtMaxBuffSz,
//                        BOOL_T *pisFoundAppdChar, UINT16 *prtotalappdcnt)
//{
//  E_ERR_T rstatus = E_ERR_Success;
//  CHAR *pwalk, *pstart, *pmax;
//  UINT16 cnt = 0;
//  UINT16 remainBuffSz = 0;
//  CHAR *pappdStringBuff, *pstartBuff, *pwalkBuff, *pmaxBuff;
//  UINT16 buffCnt = 0;
//  UINT16 strCnt = 0;
//
//  if((pappdChar == TYP_NULL) && (pappdString == TYP_NULL) &&
//     (pisFoundAppdChar == TYP_NULL) && (prtotalappdcnt == TYP_NULL))
//    return E_ERR_InvalidNullPointer;
//
//  //if(appdStringLength == 0)
//    //return E_ERR_STR_InvalidStringLength;
//
//  if(tgtMaxBuffSz == 0)
//    return E_ERR_STR_InvalidTargetBufferSize;
//
//  // consolidate prefix, string, and suffix into one temparory buffer
//  pappdStringBuff = (CHAR *)malloc(STR_APPEND_STRING_BUFF_SZ);
//  memset(pappdStringBuff, 0, STR_APPEND_STRING_BUFF_SZ);
//
//  pstartBuff = pwalkBuff = pappdStringBuff;
//  pmaxBuff = pappdStringBuff + STR_APPEND_STRING_BUFF_SZ;
//
//  // Append for prefix
//  if((pappdPrefix != TYP_NULL) && (*pappdPrefix != (CHAR)TYP_NULL))
//  {
//    while((pwalkBuff < pmaxBuff) && (*pappdPrefix != '\0'))
//    {
//      *pwalkBuff++ = *pappdPrefix++;
//      buffCnt++;
//    }
//  }
//
//  // Append for string
//  if((*pappdString != '\0') && (appdStringLength == 0))
//  {
//    while((pwalkBuff < pmaxBuff) && (*pappdString != '\0'))
//    {
//      *pwalkBuff++ = *pappdString++;
//      buffCnt++;
//    }
//  } else if ((*pappdString != '\0') && (appdStringLength > 0))
//  {
//    while ((pwalkBuff < pmaxBuff) && (strCnt < appdStringLength))
//    {
//      *pwalkBuff++ = *pappdString++;
//      buffCnt++;
//      strCnt++;
//    }
//
//  } else {
//    #ifdef CFG_DEBUG_STR
//    printf("[STR]: ERR, STR_AppendString, invalid null character in "
//           "appending string\r\n");
//    #endif // CFG_DEBUG_STR
//    return E_ERR_STR_InvalidNullCharacterInString;
//  }
//
//  // Append for suffix
//  if((pappdSuffix != TYP_NULL))
//  {
//    while((pwalkBuff < pmaxBuff) && (*pappdSuffix != '\0'))
//    {
//      *pwalkBuff++ = *pappdSuffix++;
//      buffCnt++;
//    }
//  }
//
//  // Check for the first found replacing character
//  pwalk = pstart = ptgtBuff;
//  pmax = ptgtBuff + tgtMaxBuffSz;
//
//  while((*pwalk != *pappdChar) && (pwalk < pmax))
//  {
//    pwalk++;
//    cnt++;
//  }
//
//  if(pwalk == pmax)
//  {
//    #ifdef CFG_DEBUG_STR
//    printf("[STR]: ERR, STR_AppendString, append character not found\r\n");
//    #endif // CFG_DEBUG_STR
//    return E_ERR_STR_AppendCharacterNotFound;
//  }
//
//  // found the character to be replaced
//  *pisFoundAppdChar = E_TYPE_Yes;
//  pstart = pwalk;
//  //pstring = pappdString;
//  remainBuffSz = tgtMaxBuffSz - cnt;
//
//  // check if target buffer size is greater than the total size of string to
//  // be appended
//  if(buffCnt > remainBuffSz)
//  {
//    #ifdef CFG_DEBUG_STR
//    printf("[STR]: ERR, STR_AppendString, append character not found\r\n");
//    #endif // CFG_DEBUG_STR
//    return E_ERR_STR_InvalidTargetBufferSize;
//  }
//
//  //buffCnt += 1;
//  pwalkBuff = pstartBuff;
//  cnt = 0;
//  while((pwalk < pmax) && (cnt < buffCnt))
//  {
//    *pwalk++ = *pwalkBuff++;
//    cnt++;
//  }
//
//  *prtotalappdcnt = cnt;
//  free (pappdStringBuff);
//
//  return rstatus;
//} // STR_AppendString

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


  TODO: Check buffer overrun for pshortbuff
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
//E_ERR_T STR_DecodeRegisterBit(const UINT32 reg, const STR_BD *plut,
//                             CHAR *pshortbuff, const UINT32 shortbuffsz,
//                             CHAR *pcumbuff, const UINT32 cumbuffsz,
//                             const UINT8 bytesz)
//
//{
//  UINT8 uncnt = 0;
//  UINT32 cumbuffbytecnt = strlen(pcumbuff);
//  CHAR *pcumbuffend = pcumbuff + cumbuffsz;
//
//  if((plut == TYP_NULL) || (pshortbuff == TYP_NULL) || (pcumbuff == TYP_NULL))
//    return E_ERR_InvalidNullPointer;
//
//  // check for byte size.
//  if((bytesz != 1) && (bytesz != 2) && (bytesz != 4))
//    return E_ERR_STR_InvalidDecodeByteSize;
//
//  for(uncnt = 0; uncnt < (bytesz * 8); uncnt++)
//  {
//      memset(pshortbuff, 0, shortbuffsz);
//      if((reg >> uncnt) & 0x1)
//      {
//         if(plut[uncnt].true_description[0] == TYP_NULL)
//           sprintf(pshortbuff, "%02d: Not defined\r\n", uncnt);
//         else
//           sprintf(pshortbuff, plut[uncnt].true_description);
//
//         cumbuffbytecnt += strlen(pshortbuff);
//         #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_STR)
//         // check for byte count to the cumulative buffer
//         printf("[STR] STR_DecodeRegisterBit, bit is 1. Cumbuff bytecnt = %d, "
//                "pcumbuff = 0x%08x, pcumbuffend = 0x%08x\n",
//                cumbuffbytecnt, pcumbuff, pcumbuffend);
//         #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_STR))
//         if(pcumbuffend < (cumbuffbytecnt + pcumbuff))
//           return E_ERR_STR_OutputOverrunCumulativeBuffer;
//         strcat(pcumbuff, pshortbuff);
//      }else{
//         if(plut[uncnt].false_description[0] == (const CHAR)TYP_NULL)
//           sprintf(pshortbuff, "%02d: Not defined\r\n", uncnt);
//         else
//           sprintf(pshortbuff, plut[uncnt].false_description);
//
//         cumbuffbytecnt += strlen(pshortbuff);
//         #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_STR)
//         // check for byte count to the cumulative buffer
//         printf("[STR] STR_DecodeRegisterBit, bit is 0. Cumbuff bytecnt = %d, "
//                "pcumbuff = 0x%08x, pcumbuffend = 0x%08x\n",
//                cumbuffbytecnt, pcumbuff, pcumbuffend);
//         #endif // (defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_STR))
//         if(pcumbuffend < (cumbuffbytecnt + pcumbuff))
//           return E_ERR_STR_OutputOverrunCumulativeBuffer;
//         strcat(pcumbuff, pshortbuff);
//      }
//  }// for(uncnt = 0; uncnt < (bytesz * 8)); uncnt++)
//  return E_ERR_Success;
//} // STR_DecodeRegisterBit
/*------------------------------------------------------------------------------

 PUBLIC ROUTINE

  STR_SwapEndianessBuffer

 DESCRIPTION

  This routine will swap the designated indianess and load the data
  to the designated array

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
E_ERR_T STR_SwapEndianessBuffer(VOID *pdest, VOID *psource, UINT32 buffer_sz,
                               STR_BYTE_CONV_T conv_type)
{
  UINT16 *psrc16;
  UINT16 *pdest16;
  UINT32 cnt = 0;
  UINT32 datacnt = 0;

  if((pdest == TYP_NULL) || (psource == TYP_NULL))
    return E_ERR_InvalidNullPointer;

  memset(pdest, 0, buffer_sz);

  switch(conv_type)
  {
    case E_STR_BCONV_16_BIT:
      if((buffer_sz % 2) != 0)
        return E_ERR_STR_InvalidDataWidth;

      datacnt = buffer_sz/2;
      psrc16 = (UINT16 *)psource;
      pdest16 = (UINT16 *)pdest;
      for(cnt = 0; cnt < datacnt; cnt++)
      {
        *pdest16 = (*psrc16 >> 8 | *psrc16 << 8);
        pdest16++;
        psrc16++;
      }
      break;

    case E_STR_BCONV_32_BIT:
      break;

    default:
      return E_ERR_STR_InvalidConversionType;
  } // switch(conv_type)

  return E_ERR_Success;
} // STR_SwapIndianessBuffer

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
UINT16 STR_ByteSwap16Bit(UINT16 wordarg)
{
  wordarg = ((wordarg << 8) + (wordarg>>8));
  return wordarg;
} // STR_ByteSwap16Bit
