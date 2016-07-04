/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics Ltd.

 PROPRIETARY RIGHTS of ST Electronics Ltd are involved in the subject matter
 of this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  hwpm.cpp                                             D1.0.4

 COMPONENT

  HWP - Hardware peripheral management

 DESCRIPTION

  This file consists of management routine for onboard peripherals

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

  Bryan Chong    29-Dec-2009        Initial revision

----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include <sys/neutrino.h>
#include <hw/pci.h>
#include <hw/pci_devices.h>
#include <hw/inout.h>
#include <termios.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "sys_ass.h"
#include "hwp_def.h"
#include "hwp_ext.h"
#include "ser_def.h"
#include "ser_ext.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
/*----------------------------------------------------------------------------
  Public variables declaration
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Public Prototypes declaration
----------------------------------------------------------------------------*/
E_ERR_T HWP_TPS_GetCPUTemperature(INT16 *pstemperature);
E_ERR_T HWP_ReadSys(VOID);
E_ERR_T HWP_LED_Management(UINT8 ledID, BOOL_T onoffstatus);
E_ERR_T HWP_LCM_SendCmd(E_LCM_ADDR uclcmaddr, UINT8 ucdata);
E_ERR_T HWP_LCM_PrnMsg(UINT8 uccursormode, UINT8 ucinposition, CHAR *pstring);
E_ERR_T HWP_LCM_PrnMsgCtrl(E_HWP_LCM_MSG msgtype);
/*----------------------------------------------------------------------------
  Local variables declaration
----------------------------------------------------------------------------*/
static E_ERR_T hwp_close_iobase_handle(IO_BASE iobase);
static E_ERR_T hwp_get_iobase_handle(IO_BASE iobase, uintptr_t *phandle);

/*----------------------------------------------------------------------------
  Local Prototypes declaration
----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  HWP_TPS_GetCPUTemperature

 DESCRIPTION

  This routine will onboard temperature sensor on MB935

 CALLED BY

  Application main

 CALLS

  [TBD]

 PARAMETER

  None

 RETURN

   E_ERR_Success      the routine executed successfully

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    29-Dec-2009      Created initial revision

-----------------------------------------------------------------------------*/
E_ERR_T HWP_TPS_GetCPUTemperature(INT16 *pstemperature)
{
  E_ERR_T rstatus = E_ERR_Success;
  uintptr_t index_io, data_io;
  int rval;

  rstatus = hwp_get_iobase_handle(E_IO_Index_Temperature, &index_io);
  if(rstatus != E_ERR_Success)
    return rstatus;
  #ifdef _CFG_DEBUG_MSG
  printf("[HWP] HWP_TPS_GetCPUTemperature, index_io = 0x%x\n", index_io);
  #endif // CFG_DEBUG_MSG

  rstatus = hwp_get_iobase_handle(E_IO_Data_Temperature, &data_io);
  if(rstatus != E_ERR_Success)
    return rstatus;

  #ifdef _CFG_DEBUG_MSG
  printf("[HWP] HWP_TPS_GetCPUTemperature, data_io = 0x%x\n", data_io);
  #endif // CFG_DEBUG_MSG

  out8(index_io, 0x4e);

  rval = in8(data_io);
  rval &= 0xF8;
  rval |= 0x01;

  out8(index_io, 0x4e);
  out8(data_io, rval);
  out8(index_io, 0x50);

  rval = in8(data_io);

  #ifdef _CFG_DEBUG_MSG
  printf("[HWP] HWP_TPS_GetCPUTemperature, CPU Temperature = %d'C\n", rval);
  #endif //CFG_DEBUG_MSG
  *pstemperature = rval;

  fflush(stdout);

  rstatus = hwp_close_iobase_handle(E_IO_Index_Temperature);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
  rstatus = hwp_close_iobase_handle(E_IO_Data_Temperature);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  return E_ERR_Success;
} // HPM_TPS_GetCPUTemperature
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
E_ERR_T HWP_LED_Management(UINT8 ledID, BOOL_T onoffstatus)
{

  UINT32 led, i=0;
  UINT8 pos = ledID;

  //Enter Extended Function Mode
  out8(0x4e,0x87);
  out8(0x4e,0x87);

  //Select logic device 7
  out8(0x4e,0x07);
  out8(0x4f, 0x7);

  //Assign pin121-128 bit(7...2) to be 1
  out8(0x4e, 0x2f);
  i = in8(0x4f);
  i |= 0xfc;
  out8(0x4e, 0x2f);
  out8(0x4f, i);

  //Active logic device 7
  out8(0x4e, 0x30);
  out8(0x4f, 0x1);
  //Select I/O mode
  out8(0x4e, 0xf0);
  i = in8(0x4f);
  i &= 0x3;
  out8(0x4e, 0xf0);
  out8(0x4f, i);

  //Select inversion mode
  out8(0x4e, 0xf2);
  i = in8(0x4f);
  i &= 0x3;
  out8(0x4e, 0xf2);
  out8(0x4f, i);


  /* Value of Inversion Register :
     Only high nibble is available for this function.
     When set to a 1, the incoming/outgoing port value is inverted.
     When set to a 0, the incoming/outgoing port value is the same as in
     Data Register. Value of I/O Selection Register :
     Only high nibble is available for this function.
     When set to a 1, respective GPIO port is programmed as an input port.
     When set to a 0, respective GPIO port is programmed as an output port.
     Value of Output Data / Input Data :
     Only high nibble is available for this function.
     If a port is assigned to be an output port,then its respective bit can be
     read/written.
     If a port is assigned to be an input port, then its respective bit can be
     read only.
  */
  switch (pos) {
    case 1:
      out8(0x4e, 0xf1);//GP10 , LED1
      led = in8(0x4f);
      led &= 0xff;
      led = (onoffstatus == E_TYPE_On ? (led | 1) :(led & 0xfe));
      out8(0x4e, 0xf1);
      out8(0x4f, led);
      break;

    case 2:
      out8(0x4e, 0xf1);//GP11 , LED2
      led = in8(0x4f);
      led &= 0xff;
      led = (onoffstatus == E_TYPE_On ?(led | 2) :(led & 0xfd));
      out8(0x4e, 0xf1);
      out8(0x4f, led);
      break;

    case 3:
      out8(0x4e, 0xf1);//GP12 , LED3
      led = in8(0x4f);
      led &= 0xff;
      led = (onoffstatus == E_TYPE_On ?(led | 4) :(led & 0xfb));
      out8(0x4e, 0xf1);
      out8(0x4f, led);
      break;

    case 4:
      out8(0x4e, 0xf1);//GP13 , LED4
      led = in8(0x4f);
      led &= 0xff;
      led = (onoffstatus == E_TYPE_On ?(led | 8) :(led & 0xf7));
      out8(0x4e, 0xf1);
      out8(0x4f,led);
      break;

    case 5:
      out8(0x4e,0xf1);//GP14 , LED5
      led = in8(0x4f);
      led &= 0xff;
      led = (onoffstatus == E_TYPE_On ?(led | 0x10) :(led & 0xef));
      out8(0x4e,0xf1);
      out8(0x4f,led);
      break;
  }// switch (pos)

  out8(0x4e, 0xaa); // Exit extended function mode

  return E_ERR_Success;
} // LED_Management
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  HWP_LCM_SendCmd

 DESCRIPTION

  This routine will transmit command LCM and receive acknowledgement

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
E_ERR_T HWP_LCM_SendCmd(E_LCM_ADDR uclcmaddr, UINT8 ucdata)
{
  E_ERR_T rstatus = E_ERR_Success;
  UINT8 chksum;
  INT32 nrxsize;
  UINT8 ncrxbuff[10];

  SER_SendChar(E_SER_COM_Port_2, (UINT8 *)&uclcmaddr, 1);
  SER_SendChar(E_SER_COM_Port_2, &ucdata, 1);
  chksum = uclcmaddr + ucdata;
  chksum &= 0x7F;
  SER_SendChar(E_SER_COM_Port_2, &chksum, 1);

  rstatus = SER_RxChar(E_SER_COM_Port_2, ncrxbuff, 2, &nrxsize, 100);
  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP_LCM))
  printf("[HWP] HWP_LCM_SendCmd, acknowledge cmd (0x%02x) rx %d bytes\n",
          HWP_LCM_SendCmd, nrxsize);
  SYS_PrnDataBlock(ncrxbuff, nrxsize, 10);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP_LCM))

  return E_ERR_Success;
} // HWP_LCM_SendCmd
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
E_ERR_T HWP_LCM_PrnMsg(UINT8 uccursormode, UINT8 ucinposition, CHAR *pstring)
{
  E_ERR_T rstatus = E_ERR_Success;
  UINT8 ucstrsum = 0;
  UINT8 ucposition;
  UINT8 ucpacketsz;
  UINT8 uccnt;
  UINT8 ucloopcnt = 0;
  UINT8 ucchksum;
  UINT8 uclcmaddr = (UINT8)E_HWP_LCM_ADDR_EasyStringDisplay;
  UINT8 ucretry = 3;
  UINT8 ncrxbuff[10];
  INT32 nrxsize;
  BOOL_T isLCDSendOk = E_TYPE_No;
  UINT8 uctmp;

  if(uccursormode > E_HWP_LCM_CursorOn_BlinkingOn)
    return E_ERR_InvalidParameter;

  if(ucinposition > HWP_LCM_MAX_CHARACTER - 1)
    return E_ERR_HWP_LCM_InvalidStartPosition;

  if(pstring == TYP_NULL)
    return E_ERR_InvalidNullPointer;

  if(ucinposition < 16)
    ucposition = 0x80 + ucinposition;
  else
    ucposition = 0x80 + 0x40 + (ucinposition - 16);

  ucpacketsz = strlen(pstring) + HWP_LCM_PACKET_HDR_SZ;

  for(uccnt = 0; uccnt < strlen(pstring); uccnt++)
    ucstrsum += pstring[uccnt];

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP_LCM))
  printf("[HWP] HWP_LCM_PrnMsg, cursor mode = %d, position = 0x%02x\n"
         "input string = %s (%d), packet size = %d, string sum = 0x%02x\n",
         uccursormode, ucposition, pstring,
         (ucpacketsz - HWP_LCM_PACKET_HDR_SZ), ucpacketsz, ucstrsum);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP_LCM))

  ucchksum = uclcmaddr + uccursormode + ucposition + ucpacketsz + ucstrsum;
  ucchksum &= 0x7F;

  ucloopcnt = 0;
  do{
    if(ucloopcnt++ > ucretry)
    {
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP_LCM))
      printf("[HWP] HWP_LCM_PrnMsg, exceed retry limit\n");
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP_LCM))
      break;
    }

    rstatus = SER_SendChar(E_SER_COM_Port_2, &uclcmaddr, 1);
    SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

    rstatus = SER_SendChar(E_SER_COM_Port_2, &ucpacketsz, 1);
    SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

    rstatus = SER_SendChar(E_SER_COM_Port_2, &uccursormode, 1);
    SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

    rstatus = SER_SendChar(E_SER_COM_Port_2, &ucposition, 1);
    SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

    for(uccnt = 0; uccnt < (ucpacketsz - HWP_LCM_PACKET_HDR_SZ); uccnt++)
    {
      rstatus = SER_SendChar(E_SER_COM_Port_2, (UINT8 *)&pstring[uccnt],
                             1);
      SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
    }

    rstatus = SER_SendChar(E_SER_COM_Port_2, &ucchksum, 1);
    SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);


    rstatus = SER_RxChar(E_SER_COM_Port_2, ncrxbuff, 2, &nrxsize, 100);
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP_LCM))
    printf("[HWP] HWP_LCM_PrnMsg, acknowledge cmd (0x%02x) rx %d bytes\n",
         E_HWP_LCM_ADDR_EasyStringDisplay, nrxsize);
    SYS_PrnDataBlock(ncrxbuff, nrxsize, 10);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP_LCM))

    if(rstatus == E_ERR_SER_SelectReadTimeOut)
      continue;

    uctmp = ncrxbuff[0] & ncrxbuff[1];

    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP_LCM))
    printf("[HWP] HWP_LCM_PrnMsg, retry %d/%d\n", (ucloopcnt - 1), ucretry);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP_LCM))

    if(uctmp == 0x1a)
    {
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP_LCM))
      printf("[HWP] HWP_LCM_PrnMsg, LCM checksum error, continue\n");
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP_LCM))
      continue;
    }

    if((ncrxbuff[0] & 0x7F) != ncrxbuff[1])
    {
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP_LCM))
      printf("[HWP] HWP_LCM_PrnMsg, host checksum error, continue\n");
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP_LCM))
      continue;
    }

    isLCDSendOk = E_TYPE_Yes;
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP_LCM))
    printf("[HWP] HWP_LCM_PrnMsg, send OK\n");
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP_LCM))
  } while(isLCDSendOk == E_TYPE_No);

  return E_ERR_Success;
} // HWP_LCM_PrnMsg
/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  HWP_LCM_PrnMsgCtrl

 DESCRIPTION

  This routine will print message to LCD from look-up table or pre-defined
  variables, such as IP address. Before printing, the routine if the same
  message has already been displayed.

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
E_ERR_T HWP_LCM_PrnMsgCtrl(E_HWP_LCM_MSG msgtype)
{
  E_ERR_T rstatus = E_ERR_Success;

  // Check if the message is currently on display, if yes do nothing
  if((msgtype == pHWP_CB->lcm.curr_msg[0]) ||
     (msgtype == pHWP_CB->lcm.curr_msg[1]))
    return E_ERR_Success;

  switch(msgtype)
  {
    case E_HWP_LCM_MSG_Blank:
      break;

    // print IP address at row 2
    case E_HWP_LCM_MSG_IPAddress:
      rstatus = HWP_LCM_PrnMsg(
        pHWP_CB->lcm.pmsglut[msgtype].cursorAndBlinkingMode,
        pHWP_CB->lcm.pmsglut[msgtype].startposition,
        g_tRTUConfig.tLANPara[0].acLAN_IP);
      SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
      pHWP_CB->lcm.curr_msg[1] = msgtype;
      break;

    // // print other messages at row 1
    case E_HWP_LCM_MSG_Primary:
    case E_HWP_LCM_MSG_Standby:
    case E_HWP_LCM_MSG_RTUExit:
      rstatus = HWP_LCM_PrnMsg(
        pHWP_CB->lcm.pmsglut[msgtype].cursorAndBlinkingMode,
        pHWP_CB->lcm.pmsglut[msgtype].startposition,
        pHWP_CB->lcm.pmsglut[msgtype].pstring);
      SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
      pHWP_CB->lcm.curr_msg[0] = msgtype;
      break;

    default:
      return E_ERR_HWP_LCM_InvalidMessageType;
  } // switch(msgtype)

  return E_ERR_Success;
} // HWP_LCM_PrnMsgCtrl

/*-----------------------------------------------------------------------------

 PRIVATE ROUTINE

  hwp_get_iobase_handle

 DESCRIPTION

  This routine will return iobase handle

 CALLED BY

  Application main

 CALLS

  [TBD]

 PARAMETER

  None

 RETURN

   E_ERR_Success      the routine executed successfully

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    29-Dec-2009      Created initial revision

-----------------------------------------------------------------------------*/
static E_ERR_T hwp_close_iobase_handle(IO_BASE iobase)
{
  INT8 rval;

  switch(iobase)
  {
    case E_IO_Index_Temperature:
      rval = munmap_device_io(E_IO_Index_Temperature, 1);
      break;

    case E_IO_Data_Temperature:
      rval = munmap_device_io(E_IO_Data_Temperature, 1);
      break;

    default:
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP))
      printf("ERR  [HWP] hwp_close_iobase_handle, invalid iobase\n");
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP))
      return E_ERR_HWP_InvalidIOType;
  } // switch(iobase)

  if (rval == TYP_ERROR)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [HWP] hwp_close_iobase_handle, unmmap port failure\n");
    #endif // CFG_DEBUG_SYS_ERR
    fflush(stdout);
    return E_ERR_HWP_UnMmapPortFail;
  }

  #ifdef _CFG_DEBUG_MSG
  printf("[HWP] hwp_close_iobase_handle, close IO port 0x%x successfully\n",
         iobase);
  #endif // CFG_DEBUG_MSG

  return E_ERR_Success;
} // hwp_close_iobase_handle
/*-----------------------------------------------------------------------------

 PRIVATE ROUTINE

  hwp_get_iobase_handle

 DESCRIPTION

  This routine will return iobase handle

 CALLED BY

  Application main

 CALLS

  [TBD]

 PARAMETER

  None

 RETURN

   E_ERR_Success      the routine executed successfully

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    29-Dec-2009      Created initial revision

-----------------------------------------------------------------------------*/
static E_ERR_T hwp_get_iobase_handle(IO_BASE iobase, uintptr_t *phandle)
{
  uintptr_t port_handle = TYP_NULL;

  switch(iobase)
  {
    case E_IO_Index_Temperature:
      port_handle = mmap_device_io(1, E_IO_Index_Temperature);
      break;

    case E_IO_Data_Temperature:
      port_handle = mmap_device_io(1, E_IO_Data_Temperature);
      break;

    default:
      #ifdef CFG_PRN_ERR
      printf("ERR  [HWP] hwp_get_iobase_handle, invalid IO type\n");
      #endif // CFG_PRN_ERR
      return E_ERR_HWP_InvalidIOType;
  } // switch(iobase)

  if ((port_handle == MAP_DEVICE_FAILED) || (port_handle == TYP_NULL))
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [HWP] hwp_get_iobase_handle, mmap port failure\n");
    #endif // CFG_DEBUG_SYS_ERR
    fflush(stdout);
    return E_ERR_HWP_MmapPortInvalidHandle;
  }

  #ifdef _CFG_DEBUG_MSG
  printf("[HWP] hwp_get_iobase_handle, get port handle 0x%04x successfully\n",
          port_handle);
  #endif // CFG_DEBUG_MSG

  *phandle = port_handle;

  return E_ERR_Success;
} // hwp_get_iobase_handle




