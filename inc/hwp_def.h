/*----------------------------------------------------------------------------

            Copyright (c) 2009 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   hwp_def.h                                           D1.0.4

 COMPONENT

   n/a

 DESCRIPTION

   This file contains the type definitions for hardware peripheral management


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     29-Dec-2009      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef HWP_DEF_H
#define HWP_DEF_H

#ifdef __cplusplus
extern "C"
{
#endif

// LED
#define HWP_LED_MAXNUM          5

// LCD Module (LCM)
#define HWP_LCM_BACKLIGHT_MIN   0
#define HWP_LCM_BACKLIGHT_MAX   10
#define HWP_LCM_MAX_CHARACTER   32
// LCM smart display packet header size
#define HWP_LCM_PACKET_HDR_SZ   5


typedef enum e_lcm_addr{
  E_HWP_LCM_ADDR_CommandByte = 0x90,
  E_HWP_LCM_ADDR_Reserved_0x91,
  E_HWP_LCM_ADDR_DataByte,
  E_HWP_LCM_ADDR_Reserved_0x93,
  E_HWP_LCM_ADDR_Reserved_0x94,
  E_HWP_LCM_ADDR_BackLight,
  E_HWP_LCM_ADDR_Buzzer,
  E_HWP_LCM_ADDR_EasyStringDisplay,
  E_HWP_LCM_ADDR_EndOfList
} E_LCM_ADDR;

typedef enum e_lcm_cursor_mode{
  E_HWP_LCM_CursorOff_BlinkingOff,
  E_HWP_LCM_CursorOff_BlinkingOn,
  E_HWP_LCM_CursorOn_BlinkingOff,
  E_HWP_LCM_CursorOn_BlinkingOn,
  E_HWP_LCM_Cursor_EndOfList
} E_LCM_CURSOR;

typedef struct lcm_smartdisplay_packet{
  UINT8 cursormode;
  UINT8 position;
  UINT8 strsz;
  CHAR *pinputstring;
} E_LCM_MSG;

typedef enum e_hwp_lcm_command {
  E_LCM_CMD_Reserved_0,
  E_LCM_CMD_ClearScreen,
  E_LCM_CMD_CursorReturnHome,
  E_LCM_CMD_Reserved_3,
  E_LCM_CMD_EntryModeSet_Normal,
  E_LCM_CMD_EntryModeSet_Shift,
  E_LCM_CMD_EntryModeSet_Insert,
  E_LCM_CMD_EntryModeSet_InsertShift,
  E_LCM_CMD_DisplayOffCursorOffBlinkingOff,
  E_LCM_CMD_DisplayOffCursorOffBlinkingOn,
  E_LCM_CMD_DisplayOffCursorOnBlinkingOff,
  E_LCM_CMD_DisplayOffCursorOnBlinkingOn,
  E_LCM_CMD_DisplayOnCursorOffBlinkingOff,
  E_LCM_CMD_DisplayOnCursorOffBlinkingOn,
  E_LCM_CMD_DisplayOnCursorOnBlinkingOff,
  E_LCM_CMD_DisplayOnCursorOnBlinkingOn,
  E_LCM_CMD_CursorShiftLeft,
  E_LCM_CMD_Reserved_17,
  E_LCM_CMD_Reserved_18,
  E_LCM_CMD_Reserved_19,
  E_LCM_CMD_CursorShiftRight,
  E_LCM_CMD_Reserved_21,
  E_LCM_CMD_Reserved_22,
  E_LCM_CMD_Reserved_23,
  E_LCM_CMD_StringAndCursorShiftLeft,
  E_LCM_CMD_Reserved_25,
  E_LCM_CMD_Reserved_26,
  E_LCM_CMD_Reserved_27,
  E_LCM_CMD_StringAndCursorShiftRight,
  E_LCM_CMD_Reserved_29,
  E_LCM_CMD_Reserved_30,
  E_LCM_CMD_Reserved_31,
  E_LCM_CMD_Fn_4bitDL_1Line_508Font,
  E_LCM_CMD_Reserved_33,
  E_LCM_CMD_Reserved_34,
  E_LCM_CMD_Reserved_35,
  E_LCM_CMD_Fn_4bitDL_1Line_511Font,
  E_LCM_CMD_Reserved_37,
  E_LCM_CMD_Reserved_38,
  E_LCM_CMD_Reserved_39,
  E_LCM_CMD_Fn_4bitDL_2Line_508Font,
  E_LCM_CMD_Reserved_41,
  E_LCM_CMD_Reserved_42,
  E_LCM_CMD_Reserved_43,
  E_LCM_CMD_Fn_4bitDL_2Line_511Font,
  E_LCM_CMD_Reserved_45,
  E_LCM_CMD_Reserved_46,
  E_LCM_CMD_Reserved_47,
  E_LCM_CMD_Fn_8bitDL_1Line_508Font,
  E_LCM_CMD_Reserved_49,
  E_LCM_CMD_Reserved_50,
  E_LCM_CMD_Reserved_51,
  E_LCM_CMD_Fn_8bitDL_1Line_511Font,
  E_LCM_CMD_Reserved_53,
  E_LCM_CMD_Reserved_54,
  E_LCM_CMD_Reserved_55,
  E_LCM_CMD_Fn_8bitDL_2Line_508Font,
  E_LCM_CMD_Reserved_57,
  E_LCM_CMD_Reserved_58,
  E_LCM_CMD_Reserved_59,
  E_LCM_CMD_Fn_8bitDL_2Line_511Font,


  E_LCM_CMD_EndOfList
} E_LCM_CMD;

typedef enum e_hwp_lcm_message{
  E_HWP_LCM_MSG_Reserved_0,
  E_HWP_LCM_MSG_Blank,
  E_HWP_LCM_MSG_Primary,
  E_HWP_LCM_MSG_Standby,
  E_HWP_LCM_MSG_IPAddress,
  E_HWP_LCM_MSG_RTUExit,
  E_HWP_LCM_MSG_EndOfList
}E_HWP_LCM_MSG;

struct hwp_lcm_message_lut_st{
  E_HWP_LCM_MSG label;
  CHAR *pstring;
  E_LCM_CURSOR cursorAndBlinkingMode;
  UINT8 startposition;
};

typedef enum e_hwp_pci_tlb{
  E_HWP_PCI_TLB_1_BTM,
  E_HWP_PCI_TLB_2_TOP,
  E_HWP_PCI_TLB_3_BTM,
  E_HWP_PCI_TLB_4_TOP,
  E_HWP_PCI_TLB_5_BTM,
  E_HWP_PCI_TLB_6_TOP,
  E_HWP_PCI_TLB_EndOfList
} E_HWP_PCI_TLB;

struct hwp_pci_controller_lut_st{
  E_HWP_PCI_TLB label;
  UINT16        vendor_id;
  UINT16        device_id;
  UINT8         irq_no;
};

typedef enum e_iobase{
  E_IO_Index_Temperature = 0x295,
  E_IO_Data_Temperature,
  E_IO_EndOfList
} IO_BASE;

typedef struct hwp_control_block_st{
  UINT8 ucnumOfPciCardFound;
  UINT8 uctotalSerialCOMPort;
  struct pci_card{
    UINT16 usvendorId;
    UINT16 usdeviceId;
    UINT16 ussubSystemVendorId;
    UINT16 ussystemId;
    UINT16 usbusNumber;
    UINT16 usirqNumber;
    UINT32 uncommunicationClass;
    UINT8  uccom_start_number;
    UINT8  uccom_end_number;
    UINT16 usreserved1;
  }serialComCard[CFG_HWP_TOTAL_PCI_SLOT];

  struct lcm_cb_st{
    // pointer to look-up table
    struct hwp_lcm_message_lut_st *pmsglut;
    // current message display on row 1 and 2
    E_HWP_LCM_MSG curr_msg[2];
  }lcm;
} HWP_CB;

#ifdef __cplusplus
}
#endif

#endif /* HWP_DEF_H */
