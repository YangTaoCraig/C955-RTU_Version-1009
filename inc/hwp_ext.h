/*----------------------------------------------------------------------------

            Copyright (c) 2009 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   hwp_ext.h                                           D1.0.4

 COMPONENT

   HWP - Error Management

 DESCRIPTION

   This file contains the external declaration for hardware peripheral
   management


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     29-Dec-2009      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef HWP_EXT_H
#define HWP_EXT_H

#ifdef __cplusplus
extern "C"
{
#endif

/*----------------------------------------------------------------------------
  Global variable definition
----------------------------------------------------------------------------*/
extern HWP_CB *pHWP_CB;
/*----------------------------------------------------------------------------
  Public prototype declaration
----------------------------------------------------------------------------*/
extern E_ERR_T HWP_Initialization(VOID);
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  HWP_LCM_Initialization


 DESCRIPTION

  This routine will initialize the hardware LCD display module using onboard
  serial COM port 2. This routine is dependent to the initialization of
  pSER_CB.

 CALLED BY

  SYS_Initialization

 CALLS

  HWP_LCM_SendCmd    transmit command to LCM and receive acknowledgement

 PARAMETER

  None

 RETURN

   E_ERR_Success      the routine executed successfully

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    10-Mar-2010      Created initial revision

-----------------------------------------------------------------------------*/
extern E_ERR_T HWP_LCM_Initialization(VOID);

extern E_ERR_T HWP_TPS_GetCPUTemperature(INT16 *pstemperature);
extern E_ERR_T HWP_ReadSys(VOID);
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  HWP_LED_Management

 DESCRIPTION

  This routine will maange led turning on and off

 CALLED BY

  Application main

 CALLS

  None

 PARAMETER

  ledID        [in] LED ID. 1 to 5
  onoffstatus  [in] turn on is 1, turn off is 0

 RETURN

  E_ERR_Success      the routine executed successfully

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    22-Feb-2009      Created initial revision

-----------------------------------------------------------------------------*/
extern E_ERR_T HWP_LED_Management(UINT8 ledID, BOOL_T onoffstatus);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  HWP_LCM_SendCmd

 DESCRIPTION

  This routine will transmit command LCM

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  uclcmaddr   [in] LCM address
  ucdata      [in] data to be written

 RETURN

  E_ERR_Success  the routine executed successfully


 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      12-Feb-2010      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T HWP_LCM_SendCmd(E_LCM_ADDR uclcmaddr, UINT8 ucdata);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  HWP_LCM_PrnMsg

 DESCRIPTION

  This routine will transmit the designated string to LCM and receive 
  acknowledgement

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  uclcmaddr   [in] LCM address
  ucdata      [in] data to be written

 RETURN

  E_ERR_Success  the routine executed successfully


 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      15-Feb-2010      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T HWP_LCM_PrnMsg(UINT8 uccursormode, UINT8 ucinposition, 
                             CHAR *pstring);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  HWP_LCM_PrnMsgCtrl

 DESCRIPTION

  This routine will print message to LCD from look-up table or pre-defined
  variables, such as IP address.

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  uclcmaddr   [in] LCM address
  ucdata      [in] data to be written

 RETURN

  E_ERR_Success  the routine executed successfully


 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      17-Feb-2010      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T HWP_LCM_PrnMsgCtrl(E_HWP_LCM_MSG msgtype);
/*----------------------------------------------------------------------------
  SCPI supporting routine
----------------------------------------------------------------------------*/
extern E_ERR_T HWPS_ManageLED       (VOID *pin, VOID *pout, UINT16 sz_in,
                                    UINT16 sz_out);
extern E_ERR_T HWPS_CPUTemperature_Query
                                   (VOID *pin, VOID *pout, UINT16 sz_in,
                                    UINT16 sz_out);
extern E_ERR_T HWPS_PciInfo_Query   (VOID *pin, VOID *pout, UINT16 sz_in,
                                    UINT16 sz_out);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  HWPS_LCM_BackLightCtrl

 DESCRIPTION

  This routine will control the brightness level of the LCD module backlight

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  pin         [in] pointer to input parameter structure
  sz_in       [in] size of input parameter structure
  pout        [out] pointer to output parameter structure
  sz_out      [out] size of output parameter structure


 RETURN

  E_ERR_Success  the routine executed successfully


 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      12-Feb-2010      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T HWPS_LCM_BackLightCtrl(VOID *pin, VOID *pout, UINT16 sz_in,
                                     UINT16 sz_out);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  HWPS_LCM_BuzzerCtrl

 DESCRIPTION

  This routine will turn on or off the buzzer from the LCD module

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  pin         [in] pointer to input parameter structure
  sz_in       [in] size of input parameter structure
  pout        [out] pointer to output parameter structure
  sz_out      [out] size of output parameter structure


 RETURN

  E_ERR_Success  the routine executed successfully


 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      12-Mar-2010      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T HWPS_LCM_BuzzerCtrl(VOID *pin, VOID *pout, UINT16 sz_in,
                                  UINT16 sz_out);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  HWPS_LCM_PrnChar

 DESCRIPTION

  This routine will print a character on the screen

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  pin         [in] pointer to input parameter structure
  sz_in       [in] size of input parameter structure
  pout        [out] pointer to output parameter structure
  sz_out      [out] size of output parameter structure


 RETURN

  E_ERR_Success  the routine executed successfully


 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      11-Mar-2010      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T HWPS_LCM_PrnChar(VOID *pin, VOID *pout, UINT16 sz_in,
                               UINT16 sz_out);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  HWPS_LCM_PrnMsg

 DESCRIPTION

  This routine will print a message on the screen

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  pin         [in] pointer to input parameter structure
  sz_in       [in] size of input parameter structure
  pout        [out] pointer to output parameter structure
  sz_out      [out] size of output parameter structure


 RETURN

  E_ERR_Success  the routine executed successfully


 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      12-Mar-2010      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T HWPS_LCM_PrnMsg(VOID *pin, VOID *pout, UINT16 sz_in,
                              UINT16 sz_out);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  HWPS_LCM_Clear

 DESCRIPTION

  This routine will clear LCD screen

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  pin         [in] pointer to input parameter structure
  sz_in       [in] size of input parameter structure
  pout        [out] pointer to output parameter structure
  sz_out      [out] size of output parameter structure


 RETURN

  E_ERR_Success  the routine executed successfully


 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      15-Mar-2010      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T HWPS_LCM_Clear(VOID *pin, VOID *pout, UINT16 sz_in,
                             UINT16 sz_out);
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  HWPS_LCM_Command

 DESCRIPTION

  This routine will execute LCM command

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  pin         [in] pointer to input parameter structure
  sz_in       [in] size of input parameter structure
  pout        [out] pointer to output parameter structure
  sz_out      [out] size of output parameter structure


 RETURN

  E_ERR_Success  the routine executed successfully


 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      15-Mar-2010      Created initial revision

----------------------------------------------------------------------------*/
extern E_ERR_T HWPS_LCM_Command(VOID *pin, VOID *pout, UINT16 sz_in,
                              UINT16 sz_out);
#ifdef __cplusplus
}
#endif
#endif // HWP_EXT_H

