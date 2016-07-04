/*----------------------------------------------------------------------------

            Copyright (c) 2009 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   str_def.h                                          D1.0.4

 COMPONENT

   n/a

 DESCRIPTION

   This file contains the type definitions for string management


 AUTHOR

   Bryan K. W. Chong


 REVISION HISTORY by Bryan Chong

 D1.1.3
 ------
 27-Apr-2011  
  
 - CRTUStatus::ClearFlag 
     Add reset m_bWaitLinkCheckCompletedFlag 
 - Update to add modification history [C955 PR96]

 09-Sep-2009  

 - Created initial version 1.0
      
----------------------------------------------------------------------------*/
#ifndef STR_DEF_H
#define STR_DEF_H

#define STR_CMP_MAX_SZ            255
#define STR_BUFF_SZ               1000
#define STR_APPEND_STRING_BUFF_SZ 1000

#define atow(strA,strW,lenW) \
  MultiByteToWideChar(CP_ACP,0,strA,-1,strW,lenW)

typedef enum string_recognize_terminating_char_t{
  E_STR_Backspace = 0x08,
  E_STR_ReturnChar = 0x0A,
  E_STR_CarriageChar = 0x0D,
  E_STR_SpaceChar = 0x20,
  E_STR_EndOfList
} STR_CHAR_T;

typedef enum string_conversion_supporting_size_t{
  E_STR_BCONV_4_BIT,
  E_STR_BCONV_8_BIT,
  E_STR_BCONV_12_BIT,
  E_STR_BCONV_16_BIT,
  E_STR_BCONV_20_BIT,
  E_STR_BCONV_24_BIT,
  E_STR_BCONV_28_BIT,
  E_STR_BCONV_32_BIT,
  E_STR_BCONV_EndOfList
} STR_BYTE_CONV_T;

#define STR_BIT_DESC_LENGTH 100
typedef struct bit_description_st{
  const CHAR true_description[STR_BIT_DESC_LENGTH];
  const CHAR false_description[STR_BIT_DESC_LENGTH];
}STR_BD;

typedef struct field_16bit_description_st{
  const CHAR description[100];
}STR_16FD;


typedef enum str_value_range_option_st{
  E_STR_VR_ExclusiveMinAndMax,
  E_STR_VR_ExclusiveMinAndInclusiveMax,
  E_STR_VR_InclusiveMinAndExclusiveMax,
  E_STR_VR_InclusiveMinAndMax,  
  E_STR_VR_EndOfList
}STR_VR_OPT_T;
/*-----------------------------------------------------------------------------

  MACRO

    STR_SET_BIT_16BIT_REG

  Description
    This macro will set a designated bit high or low based on the 16-bit input
    register, the bit location, and the bit value to set

  Parameters

    preg      pointer to a 16-bit register
    bit_loc   bit location range from 0-15. 0 being the least significant bit
    setval    boolean value. 0 or 1

  AUTHOR

    Bryan K.W. Chong

  HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong     16-Nov-2008      Created initial revision
-----------------------------------------------------------------------------*/
#define STR_SET_BIT_16BIT_REG(preg, bit_loc, setval) \
  (UINT16) ((*preg & ~(0x1 << bit_loc)) | (setval << bit_loc))
/*-----------------------------------------------------------------------------

  MACRO

    STR_GET_BIT_16BIT_REG

  Description
    This macro will query a designated bit whether is high or low based on the
    16-bit input register, the bit location, and the bit value to set

  Parameters

    preg      pointer to a 16-bit register
    bit_loc   bit location range from 0-15. 0 being the least significant bit
    setval    boolean value. 0 or 1

  AUTHOR

    Bryan K.W. Chong

  HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong     16-Nov-2008      Created initial revision
-----------------------------------------------------------------------------*/
#define STR_GET_BIT_16BIT_REG(reg, bit_loc) (BOOL)(reg && (0x1 << (bit_loc))
/*-----------------------------------------------------------------------------

  MACRO

    STR_SWAP_16BIT_REG

  Description
    This macro will swap the lower byte to upper byte and upper byte to lower
    byte for a 16-bit register or variable.

  Parameters



  AUTHOR

    Bryan K.W. Chong

  HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong     16-Nov-2008      Created initial revision
-----------------------------------------------------------------------------*/
#define STR_SWAP_16BIT_REG(reg) \
  (UINT16) ((0x00FF & reg) << 8 | (0xFF00 & reg) >> 8);

/*-----------------------------------------------------------------------------

  MACRO

    STR_SET_BIT_32BIT_REG

  Description
    This macro will set a designated bit high or low based on the 32-bit input
    register, the bit location, and the bit value to set

  Parameters

    reg       the register which has the 32-bit value
    bitloc    bit location range from 0-31. 0 being the least significant bit
    setval    boolean value. 0 or 1

  AUTHOR

    Bryan K.W. Chong

  HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong     18-Jan-2010      Created initial revision
-----------------------------------------------------------------------------*/
#define STR_SET_BIT_32BIT_REG(reg, bitloc, setval) \
  (UINT32) ((~(0x1 << bitloc) & reg) | (setval << bitloc))
#endif // STR_DEF_H
