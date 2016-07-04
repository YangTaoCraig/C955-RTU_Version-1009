/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   hwp_lut.h                                           D1.0.4

 COMPONENT

   n/a

 DESCRIPTION

   This file contains the private look-up table for hardware peripheral
  management


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     25-Jan-2010      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef _HWP_LUT_H_
#define _HWP_LUT_H_

/*----------------------------------------------------------------------------
  Local look-up table for PCI Serial Controller
  label, vendor id, device id, interrupt no.
----------------------------------------------------------------------------*/
static struct hwp_pci_controller_lut_st
  hwp_pci_serialcontroller_lut[E_HWP_PCI_TLB_EndOfList] = {
    {E_HWP_PCI_TLB_1_BTM, 0x1393, 0x1380, 10},
    {E_HWP_PCI_TLB_2_TOP, 0x1393, 0x1380, 11},
    {E_HWP_PCI_TLB_3_BTM, 0x1393, 0x1180, 10},
    {E_HWP_PCI_TLB_4_TOP, 0x1393, 0x1180, 11},
    {E_HWP_PCI_TLB_5_BTM, 0x1393, 0x1681, 10},
    {E_HWP_PCI_TLB_6_TOP, 0x1393, 0x1681, 11}
};
/*----------------------------------------------------------------------------
  Local look-up table for LCD display message
  label, message string, display and cursor mode, position
----------------------------------------------------------------------------*/
static struct hwp_lcm_message_lut_st
  hwp_lcm_message_lut[E_HWP_LCM_MSG_EndOfList] = {
    {E_HWP_LCM_MSG_Reserved_0,
     TYP_NULL,
     E_HWP_LCM_CursorOff_BlinkingOff, 0},

    {E_HWP_LCM_MSG_Blank,
     TYP_NULL,
     E_HWP_LCM_CursorOff_BlinkingOff, 0},

    {E_HWP_LCM_MSG_Primary,
     (CHAR *)"Primary",
     E_HWP_LCM_CursorOff_BlinkingOff, 0},

    {E_HWP_LCM_MSG_Standby,
     (CHAR *)"Standby",
     E_HWP_LCM_CursorOff_BlinkingOff, 0},

    {E_HWP_LCM_MSG_IPAddress,
     (CHAR *)"0.0.0.0",
     E_HWP_LCM_CursorOff_BlinkingOff, 16},

    {E_HWP_LCM_MSG_RTUExit,
     (CHAR *)"Rebooting...",
     E_HWP_LCM_CursorOff_BlinkingOff, 0}

}; // hwp_lcm_message_lut_st

#endif //_HWP_LUT_H_
