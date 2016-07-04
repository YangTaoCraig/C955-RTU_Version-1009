/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics Ltd.

 PROPRIETARY RIGHTS of ST Electronics Ltd are involved in the subject matter
 of this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  seri.cpp                                               D1.0.4

 COMPONENT

  SER - Serial Link Management

 DESCRIPTION

  This file consists of initialization routine for the Serial Link Management

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

  Bryan Chong    07-Sep-2009        Initial revision

----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/neutrino.h>
#include <termios.h>
#include <fixerrno.h>
#include <sys/dcmd_chr.h>
#include <devctl.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "ser_def.h"
#include "ser_ext.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
#include "sys_ass.h"
#include "hwp_def.h"
#include "hwp_ext.h"
/*----------------------------------------------------------------------------
  Public variables declaration
----------------------------------------------------------------------------*/
SER_CB_T *pSER_CB;

CHAR *SER_acSerialPortName[24] = {
  /* (CHAR *)ser1, (CHAR *)ser2,  (CHAR *)ser3,  (CHAR *)ser4, */
  (CHAR *)ser5,
  (CHAR *)ser6, (CHAR *)ser7,  (CHAR *)ser8,  (CHAR *)ser9, (CHAR *)ser10,
  (CHAR *)ser11, (CHAR *)ser12, (CHAR *)ser13, (CHAR *)ser14, (CHAR *)ser15,
  (CHAR *)ser16, (CHAR *)ser17, (CHAR *)ser18, (CHAR *)ser19, (CHAR *)ser20,
  (CHAR *)ser21, (CHAR *)ser22, (CHAR *)ser23, (CHAR *)ser24
};


/*----------------------------------------------------------------------------
  Public Prototypes declaration
----------------------------------------------------------------------------*/
E_ERR_T SER_Initialization();
E_ERR_T SER_SendMsg(INT32 *pfd, CHAR *pmsg);
//E_ERR_T SER_ClosePort(INT32 *pfd);
//E_ERR_T SER_ReInitializePort(const E_SER_COM comport);

/*----------------------------------------------------------------------------
  Private variables declaration
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Private Prototypes declaration
----------------------------------------------------------------------------*/
static E_ERR_T ser_OpenPort(E_SER_COM com_port, const CHAR *p_pathname,
                           INT32 *pfd, UINT32 accessmode);

static SER_CB_T ser_ControlBlk[E_SER_COM_EndOfList];

/*----------------------------------------------------------------------------
  Private lookup table declaration
----------------------------------------------------------------------------*/
static SER_PD_T ser_PortDescriptor[] = {
  /* port,                 path name,     baudrate, parity,
     databit,         stopbit, */

  // onboard serial com port
  {E_SER_COM_Port_Reserved, "reserved\0",  B0,    E_SER_PARITY_Disable,
     E_SER_DATABIT_Reserved, E_SER_STOPBIT_Reserved},
  {E_SER_COM_Port_1,        "/dev/ser1",   B38400, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_1, O_RDWR},
  {E_SER_COM_Port_2,        "/dev/ser2",   B57600, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_2, O_RDWR},
  {E_SER_COM_Port_3,        "/dev/ser3",   B38400, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_1, O_RDWR},
  {E_SER_COM_Port_4,        "/dev/ser4",   B57600, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_2, O_RDWR},

  // Moxa card 1 PCI based serial com port
  {E_SER_COM_Port_5,        "/dev/ser5",   B19200, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_1, (O_RDWR | O_NONBLOCK)},
  {E_SER_COM_Port_6,        "/dev/ser6",   B19200, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_1, (O_RDWR | O_NONBLOCK)},
  {E_SER_COM_Port_7,        "/dev/ser7",   B19200, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_2, (O_RDWR | O_NONBLOCK)},
  {E_SER_COM_Port_8,        "/dev/ser8",   B19200, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_2, (O_RDWR | O_NONBLOCK)},
  {E_SER_COM_Port_9,        "/dev/ser9",   B19200, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_2, (O_RDWR | O_NONBLOCK)},
  {E_SER_COM_Port_10,       "/dev/ser10",  B19200, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_1, (O_RDWR | O_NONBLOCK)},
  {E_SER_COM_Port_11,       "/dev/ser11",  B19200, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_1, (O_RDWR | O_NONBLOCK)},
  {E_SER_COM_Port_12,       "/dev/ser12",  B19200, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_1, (O_RDWR | O_NONBLOCK)},

  // Moxa card 2 PCI based serial com port
  {E_SER_COM_Port_13,       "/dev/ser13",  B19200, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_1, (O_RDWR | O_NONBLOCK)},
  {E_SER_COM_Port_14,       "/dev/ser14",  B19200, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_1, (O_RDWR | O_NONBLOCK)},
  {E_SER_COM_Port_15,       "/dev/ser15",  B19200, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_2, (O_RDWR | O_NONBLOCK)},
  {E_SER_COM_Port_16,       "/dev/ser16",  B19200, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_2, (O_RDWR | O_NONBLOCK)},
  {E_SER_COM_Port_17,       "/dev/ser17",  B19200, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_2, (O_RDWR | O_NONBLOCK)},
  {E_SER_COM_Port_18,       "/dev/ser18",  B19200, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_2, (O_RDWR | O_NONBLOCK)},
  {E_SER_COM_Port_19,       "/dev/ser19",  B19200, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_2, (O_RDWR | O_NONBLOCK)},
  {E_SER_COM_Port_20,       "/dev/ser20",  B19200, E_SER_PARITY_Disable,
     E_SER_DATABIT_8, E_SER_STOPBIT_2, (O_RDWR | O_NONBLOCK)},

  {E_SER_COM_EndOfList,     "End Of List", B0,     E_SER_PARITY_Disable,
     E_SER_DATABIT_EndOfList, E_SER_STOPBIT_EndOfList}
};
/*-----------------------------------------------------------------------------

 PRIVATE ROUTINE

  SER_Initialization

 DESCRIPTION

  This routine will initialize serial link management. Use ser_PortDescriptor
  look-up table to add or remove serial port.

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

   Bryan Chong    07-Sep-2009      Created initial revision

-----------------------------------------------------------------------------*/
E_ERR_T SER_Initialization()
{
  E_ERR_T rstatus = E_ERR_Success;
  UINT8 port_cnt = E_SER_COM_Port_1;

  SER_PD_T *ppd; // port descriptor array
  SER_CB_T *pcb; // port control block

  ppd = ser_PortDescriptor;
  pcb = ser_ControlBlk;

  // initialize global pointer to serial management control block
  pSER_CB = ser_ControlBlk;

  if(pHWP_CB->uctotalSerialCOMPort > E_SER_COM_EndOfList)
    return E_ERR_SER_SerialCOMPortOutOfRange;

  for(port_cnt = (UINT8)E_SER_COM_Port_1;
      port_cnt <= pHWP_CB->uctotalSerialCOMPort; port_cnt++)
  {
    ppd = &ser_PortDescriptor[port_cnt];
    pcb = &ser_ControlBlk[port_cnt];
    rstatus = ser_OpenPort(ppd->port, ppd->path_name, &pcb->fd,
                           ppd->accessmode);
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_PRN_ERR
      printf("[SER] SER_Initialization, open port %d fail\n", port_cnt);
      #endif // CFG_PRN_ERR
    }

    rstatus = SER_SetAttribute(ppd->port, ppd->baudrate, ppd->parity,
                               ppd->databit, ppd->stopbit);
    SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);


    delay(10);
    // flush input and output data
    tcflush(pcb->fd, TCIOFLUSH );

    pcb->pport_desc = ppd;

    #if ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SER))
    printf("[SER] SER_Initialization, port %d, port fd = %d\n",
           port_cnt, pcb->fd);
    #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SER))

  }

  memset(pcb->history, 0, sizeof(pcb->history));
  pcb->nextEmptyHistoryId = 0;
  #if ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SER_HOSTCMD))
  printf("[SER] SER_Initialization, history size %d bytes, start id %d\n",
         sizeof(pcb->history), pcb->nextEmptyHistoryId);
  #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SER))
  return E_ERR_Success;
} // SER_Initialization

/*-----------------------------------------------------------------------------

 PRIVATE ROUTINE

  SER_ReInitializePort

 DESCRIPTION

  This routine will re-initialize the designated serial port.

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

   Bryan Chong    22-Oct-2010      Created initial revision

-----------------------------------------------------------------------------*/
//E_ERR_T SER_ReInitializePort(const E_SER_COM comport)
//{
//  E_ERR_T rstatus = E_ERR_Success;
//  INT32 fd;
//  SER_PD_T *ppd; // port descriptor array
//  SER_CB_T *pcb; // port control block
//
//  if((comport > E_SER_COM_EndOfList) || (comport < E_SER_COM_Port_1))
//    return E_ERR_SER_SerialCOMPortOutOfRange;
//
//  fd = pSER_CB[comport].fd;
//
//  if(fd != TYP_NULL)
//    rstatus = SER_ClosePort(&fd);
//
//  ppd = &ser_PortDescriptor[comport];
//    pcb = &ser_ControlBlk[comport];
//
//  rstatus = ser_OpenPort(ppd->port, ppd->path_name, &pcb->fd,
//                         ppd->accessmode);
//  if(rstatus != E_ERR_Success)
//  {
//    #ifdef CFG_PRN_ERR
//    printf("[SER] SER_Initialization, open port %d fail\n", comport);
//    #endif // CFG_PRN_ERR
//    return rstatus;
//  }
//
//  rstatus = SER_SetAttribute(ppd->port, ppd->baudrate, ppd->parity,
//                             ppd->databit, ppd->stopbit);
//  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
//
//  delay(10);
//  // flush input and output data
//  tcflush(pcb->fd, TCIOFLUSH );
//
//  pcb->pport_desc = ppd;
//
//  #if ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SER))
//  printf("[SER] SER_ReInitializePort, port %d, fd %d\n",
//          comport, pcb->fd);
//  #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SER))
//
//  return E_ERR_Success;
//} // SER_ReInitializePort

/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SER_ClosePort

 DESCRIPTION

  This routine will close the serial port

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

   Bryan Chong    07-Sep-2009      Created initial revision

-----------------------------------------------------------------------------*/
//E_ERR_T SER_ClosePort(INT32 *pfd)
//{
//  if(pfd == TYP_NULL)
//    return E_ERR_InvalidNullPointer;
//
//  close(*pfd);
//  return E_ERR_Success;
//} // SER_ClosePort
/*-----------------------------------------------------------------------------

 PRIVATE ROUTINE

  ser_OpenPort

 DESCRIPTION

  This routine will open a serial port

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

   Bryan Chong    07-Sep-2009      Created initial revision

-----------------------------------------------------------------------------*/
static E_ERR_T ser_OpenPort(E_SER_COM com_port, const CHAR *p_pathname,
                           INT32 *pfd, UINT32 accessmode)
{
  INT32 fd;

  //if((fd = open(p_pathname, (O_RDWR))) < 0)
  if((fd = open(p_pathname, accessmode)) < 0)
  {
    #ifdef CFG_DEBUG_MSG
    printf("[SER] ser_OpenPort, open port %d fail\n", (UINT8)com_port);
    #endif // CFG_DEBUG_MSG
    return E_ERR_SER_OpenPortFail;
  }

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SER))
  printf("[SER] ser_OpenPort, fd = %d\r\n", fd);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SER))

  *pfd = fd;

  return E_ERR_Success;
} // ser_OpenPort
