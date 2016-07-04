/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics Ltd.

 PROPRIETARY RIGHTS of ST Electronics Ltd are involved in the subject matter
 of this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  hwpi.cpp                                             D1.0.4

 COMPONENT

  HWP - Hardware peripheral management

 DESCRIPTION

  This file consists of initialization routine for onboard peripherals

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

  Bryan Chong    29-Dec-2009        Initial revision

----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <hw/pci.h>
#include <hw/pci_devices.h>
#include <sys/neutrino.h>
#include <termios.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "sys_ass.h"
#include "hwp_def.h"
#include "hwp_ext.h"
#include "ser_def.h"
#include "ser_ext.h"

#include "hwp_lut.h"

/*----------------------------------------------------------------------------
  Public variables declaration
----------------------------------------------------------------------------*/
HWP_CB *pHWP_CB;


/*----------------------------------------------------------------------------
  Public Prototypes declaration
----------------------------------------------------------------------------*/
E_ERR_T HWP_Initialization(VOID);
E_ERR_T HWP_LCM_Initialization(VOID);

/*----------------------------------------------------------------------------
  Local variables declaration
----------------------------------------------------------------------------*/
static HWP_CB hwp_ctrlBlk;

/*----------------------------------------------------------------------------
  Local Prototypes declaration
----------------------------------------------------------------------------*/
static E_ERR_T hwp_initIoThread(VOID);
static E_ERR_T hwp_PCI_SerialComDetection(VOID);

/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  HWP_Initialization

 DESCRIPTION

  This routine will initialize all add-on hardware to the main board. The add-on
  hardware includes
   - PCI based serial COM controller card
   - LED
   - LCD display module

 CALLED BY

  SYS_Initialization

 CALLS

  hwp_initIoThread

 PARAMETER

  None

 RETURN

   E_ERR_Success      the routine executed successfully

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    31-Dec-2009      Created initial revision

-----------------------------------------------------------------------------*/
E_ERR_T HWP_Initialization(VOID)
{
  E_ERR_T rstatus = E_ERR_Success;
  UINT8 uccnt;

  memset(&hwp_ctrlBlk, 0, sizeof(HWP_CB));
  pHWP_CB = &hwp_ctrlBlk;

  rstatus = hwp_initIoThread();
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  rstatus = hwp_PCI_SerialComDetection();
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);


  for(uccnt = 1; uccnt < HWP_LED_MAXNUM; uccnt++)
  {
    rstatus = HWP_LED_Management(uccnt, E_TYPE_Off);
    SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
  }

  //rstatus = HWP_LCM_Initialization();
  //SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  return E_ERR_Success;
} // HWP_Initialization
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
E_ERR_T HWP_LCM_Initialization(VOID)
{
  E_ERR_T rstatus = E_ERR_Success;
  struct termios term;
  INT32 fd;

  if(pSER_CB == TYP_NULL)
    return E_ERR_InvalidNullPointer;

  fd = pSER_CB[E_SER_COM_Port_2].fd;

  #ifdef CFG_DEBUG_MSG
  printf("[HWP] hwp_LCM_Initialization, LCM is using com %d fd %d\n", 
         E_SER_COM_Port_2, fd);
  #endif // CFG_DEBUG_MSG

  //get device struct
  if (tcgetattr(fd, &term) != 0)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [HWP] hwp_LCM_Initialization, tcgetattr failed\n");
    #endif // CFG_PRN_ERR
    return E_ERR_LCM_GetAttributeFail;
  }

  // Clear LCD screen
  rstatus = HWP_LCM_SendCmd(E_HWP_LCM_ADDR_CommandByte, E_LCM_CMD_ClearScreen);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  // Turn off buzzer
  rstatus = HWP_LCM_SendCmd(E_HWP_LCM_ADDR_Buzzer, E_TYPE_Off);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  // initialize lcd row status
  pHWP_CB->lcm.curr_msg[0] = E_HWP_LCM_MSG_Blank;
  pHWP_CB->lcm.curr_msg[1] = E_HWP_LCM_MSG_Blank;

  // initialize pointer to look-up table
  pHWP_CB->lcm.pmsglut = hwp_lcm_message_lut;

  return E_ERR_Success;
} // HWP_LCM_Initialization

/*-----------------------------------------------------------------------------

 LOCAL ROUTINE

  hwp_initIoThread

 DESCRIPTION

  Request I/O privileges; let the thread execute the in8 and out8 calls on
  architectures where it has the appropriate privilege, and let it attach IRQ
  handlers. Without executing this routine, access using in8 and out8 will
  cause program termination.

 CALLED BY

  HWP_Initialization

 CALLS

  None

 PARAMETERS

  None

 RETURN

  E_ERR_Success     routine executed successfully


 AUTHOR

  Bryan K. W. Chong


 HISTORY

    NAME            DATE                    REMARKS

 Bryan Chong      29-Dec-2009      Created initial revision

-----------------------------------------------------------------------------*/
static E_ERR_T hwp_initIoThread(VOID)
{
  int reg;

  reg = ThreadCtl(_NTO_TCTL_IO, 0);
  if (reg == -1)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [HWP] hw_initIoThread, NOT ALLOWED\n");
    #endif // CFG_PRN_ERR
    fflush(stdout);
    return E_ERR_HWP_InitIoThreadFail;
  }

  return E_ERR_Success;
} // hw_initIoThread

/*-----------------------------------------------------------------------------

 PRIVATE ROUTINE

  hwp_PCI_SerialComDetection

 DESCRIPTION

  This routine will check the existence of PCI based Moxa Serial Communication
  card and update class variable ucnumOfPciCardFound for number of cards found
  in the PCI slot

 CALLED BY

  Application main

 CALLS

  None

 PARAMETER

  None

 RETURN

   E_ERR_Success      the routine executed successfully

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    11-Jan-2010      Created initial revision

-----------------------------------------------------------------------------*/
static E_ERR_T hwp_PCI_SerialComDetection(VOID)
{
  E_ERR_T rstatus = E_ERR_Success;
  UINT32 pidx;
  VOID* hdl;
  INT32 phdl;
  UINT8 uccnt;
  struct pci_dev_info inf;
  UINT8 ucsupportcnt = 0;

  /* Connect to the PCI server */
  phdl = pci_attach( 0 );
  if( phdl == -1 ) {
    #ifdef CFG_PRN_ERR
    printf("ERR  [HWP] hwp_pciSerialComDetection, PCI server initialization "
           "fail\n");
    #endif // CFG_PRN_ERR
    return E_ERR_HWP_PciServerInitializationFail;
  }

  for(uccnt = 0; uccnt < CFG_HWP_TOTAL_PCI_SLOT; uccnt++)
  {
    /* Initialize the pci_dev_info structure */
    memset( &inf, 0, sizeof( inf ) );
    pidx = uccnt;

    hdl = 0;
    while(ucsupportcnt < E_HWP_PCI_TLB_EndOfList)
    {
      inf.VendorId = hwp_pci_serialcontroller_lut[ucsupportcnt].vendor_id;
      inf.DeviceId = hwp_pci_serialcontroller_lut[ucsupportcnt].device_id;
      inf.Irq = hwp_pci_serialcontroller_lut[ucsupportcnt].irq_no;

      hdl = pci_attach_device(TYP_NULL , PCI_INIT_ALL, pidx, &inf );
      if(hdl != 0)
        break;
      ucsupportcnt++;
    }

    if( hdl == TYP_NULL )
    {
      #ifdef CFG_PRN_WARN
      printf("WARN [HWP] hwp_pciSerialComDetection, unable to locate PCI "
             "RS485 Moxa adapter. Card %d initialization fail\n", (pidx + 1));
      #endif // CFG_PRN_ERR
      //if(pidx == 0)
        //rstatus = E_ERR_SYS_PciRs485MoxaAdapterInitFail;
    } else {
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_HWP))
      printf("[HWP] Found PCI Card Vendor ID: 0x%04x, Device ID: 0x%04x\n",
             inf.VendorId, inf.DeviceId);
      printf("  Moxa Serial Communication PCI adapter card, index %d\n", pidx);
      printf("  Subsystem ID: 0x%04x\n", inf.SubsystemId);
      printf("  Subsystem Vendor ID: 0x%04x\n", inf.SubsystemVendorId);
      printf("  Index number: 0x%x\n", pidx);
      printf("  Bus Number: 0x%x\n", inf.BusNumber);
      printf("  IRQ Number: 0x%x\n", inf.Irq);
      printf("  Revision: 0x%x\n", inf.Revision);
      printf("  Device Handle: 0x%x\n", hdl);
      #endif // CFG_DEBUG_MSG

      hwp_ctrlBlk.serialComCard[uccnt].usvendorId = inf.VendorId;
      hwp_ctrlBlk.serialComCard[uccnt].usdeviceId = inf.DeviceId;
      hwp_ctrlBlk.serialComCard[uccnt].ussubSystemVendorId =
        inf.SubsystemVendorId;
      hwp_ctrlBlk.serialComCard[uccnt].ussystemId = inf.SubsystemId;
      hwp_ctrlBlk.serialComCard[uccnt].usbusNumber = inf.BusNumber;
      hwp_ctrlBlk.serialComCard[uccnt].usirqNumber = inf.Irq;
      //hwp_ctrlBlk.serialComCard[uccnt].undevicehandle = (UINT32)hdl;

      if(inf.Class & PCI_CLASS_COMMUNICATIONS)
      {
        #ifdef _CFG_DEBUG_MSG
        printf("  Class: 0x%04x, PCI Communication Class\n", inf.Class);
        #endif // CFG_DEBUG_MSG
        hwp_ctrlBlk.serialComCard[uccnt].uncommunicationClass = inf.Class;
      }

      hwp_ctrlBlk.ucnumOfPciCardFound++;
      ucsupportcnt++;

      pci_detach_device(hdl);
    }
  } //for(uccnt = 0; uccnt < CFG_HWP_TOTAL_PCI_SLOT; uccnt++)

  hwp_ctrlBlk.uctotalSerialCOMPort = (8 * hwp_ctrlBlk.ucnumOfPciCardFound +
                                      CFG_HWP_PCI_SERIAL_COM_START_NUM - 1);
  
  /* Disconnect from the PCI server */
  pci_detach(phdl);

  return rstatus;
} // hwp_pciSerialComDetection


