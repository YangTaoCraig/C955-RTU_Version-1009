/************************************************************
*                              *
*  Project:    C830 RTU                  *
*  Module:    Common Module (CMM)              *
*  File :      CMM_Modbus.cpp                *
*  Author:    Yu Wei                    *
*                              *
*  Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************

**************************************************************

DESCRIPTION

  This file defines serial modbus  function.

*************************************************************
Modiffication History
---------------------

Version: CMM D.1.1.1
  01  21 March 2003, Yu Wei,
    Start to write.

Version: CMM D.1.2.0
  01  04 July 2003, Yu Wei,
    Change ModbusReplyCheck() and ModbusCMDCheck() to single exit.

Version: CMM D.1.3.1
  01  22 September 2003, Yu Wei,
    Move ModbusSendErrorReply() from SPM to this file.

Version: CMM D.1.3.3
  01  11 December 2003, Yu Wei,
    Fixed bug in ModbusCMDCheck(). It will return a fix value MODBUS_MESSAGE_OK.

  02  29 December 2003, Yu Wei,
    Exception response is a valid reply.
    Modified ModbusReplyCheck();

  03  07 January 2003, Yu Wei,
    ModbusWriteWordCheck() base on word field to check data length.

Version: CMM D.1.3.5
  01  30 March 2004, Yu Wei
    Modified ModbusSendErrorReply(). Execption response uses | not +.
    Refer to PR:OASYS-PMS 0153.

  02  31 March 2004, Yu Wei
    Refer to PR:OASYS-PMS 0053 and PR:OASYS-PMS 0153.
    Modified ModbusReadBitCheck(), ModbusReadWordCheck(),
    ModbusWriteBitCheck() and ModbusWriteWordCheck(). When Function code
    is OK and message length error, RTU will return exception 3.

    Re-designed modbus command checking routine ModbusCMDCheck() and
    reply checking routine ModbusReplyCheck() as follow.
    - Modbus Command Check:
    (a) Slave address error. No response.
    (b) CRC error. No response.
    (c) Slave address and CRC Ok, function code != 02,03,04,0F,10.
      reply exception1.
    (d) Function code = 02,03,04,0F,10, command length error.
      reply exception3.
    (e) Command valid. Normal reply.
    (f)  Command cannot execute, reply exception4.
      SWC no response, reply 00.
      Server polling FC != 04, reply exception1.
      MMM FC != 04,10, reply exception1.
      MMM start address != function address, reply exception2.

    - Modbus Reply Check:
    (a) Slave address error. Link down.
    (b) CRC error. Link down.
    (c) Slave address and CRC Ok, FC != command FC.
      Link OK, command not execute.
    (d) FC OK, data length != command asked length.
      Link OK, command not execute.
    (e) Command valid. Link OK, command execute.

Version: CMM D.1.3.6
  01  29 April 2004, Yu Wei
    Modified ModbusReadBitCheck() and ModbusReadWordCheck()
    to fixed bug that the read command data range excess the
    max value.
    Refer to PR:OASYS-PMS 0184.

Version: CMM E.1.0.
  01  07 July 2004, Yu Wei
    Modified ModbusReplyCheck(). The reply's data length field
    should be checked.
    Refer to PR:OASYS-PMS 0227.

Version: CMM E.1.0.2
  01  01 Apr 2005, Tong Zhi Xiong
    Add ModbusReplyCheck2 function for SendReply2 use
**************************************************************/
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/socket.h>
#include <stdio.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "err_ext.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
#include "lnc_def.h"
#include "lnc_ext.h"
#include "str_def.h"
#include "str_ext.h"

#include "CMM_Modbus.h"
#include "Common.h"
#include "CMM_Log.h"

/*******************************************************************************
Purpose:
  Calculate Modbus using CRC-16-CCITT algorithm with polynomial generator of
  x^16 + x^15 + x^2 + 1, which is equalavent to 0xA001, or 1010 0000 0000 0001
  in binary

Input:
  pucCmd    -- Modbus command. (in)
  nCmdSize  -- Modbus cmd size excluding 2 CRC bytes. (in)

Return:
  CRC value with 1st byte transmitted is the least significant one

*******************************************************************************/
unsigned short ModbusCRC(unsigned char *pucCmd,  int nCmdSize)
{
  unsigned short unCRC;

  unCRC = 0xFFFF;

  for (int n=0; n<nCmdSize; n++)
  {
    unCRC ^= pucCmd[n];

    for (int i=0; i<8; i++)
    {
      if (unCRC & 1)
      {
        unCRC >>= 1;
        unCRC ^= 0xA001;
      }
      else
      {
        unCRC >>= 1;
      }
    }
  }

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_MODBUS)
  printf("[CMM] ModbusCRC, rx %d words, found CRC 0x%04x\n",
         nCmdSize, (UINT16)((unCRC << 8) + (unCRC >> 8)));
  #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_MODBUS)

  return (unCRC << 8) + (unCRC >> 8);  //swap high and low bytes
} // ModbusCRC

/*******************************************************************************
Purpose:
  Check modbus reply message for slave address, function code, and CRC.
  Re-designed in 31 march 2004, Yu Wei.

Input:
  aucMessage    -- Modbus reply. (in)
  nMessageLen    -- Modbus reply size. (in)
  aucSendBuffer  -- The buffer for sending command. (in)

Return:
  Check result:
  MODBUS_ERROR_EXCEPTION    -- Exception, command did not execute correctly.
  MODBUS_ERROR_CRC      -- Slave address error or CRC error.
  MODBUS_MESSAGE_OK      -- The message is a correct Modbus response.

History

     Name        Date        Remark

 Bryan Chong  18-Mar-2011  Remove length checking to address ECS requirememnt

*******************************************************************************/
int ModbusReplyCheck(unsigned char *aucMessage, int nMessageLen,
                     unsigned char *aucSendBuffer)
{
  int nDataLen;        //Calculate message size.
  int nReturn;        //Return result;
  unsigned short tmp;
  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_MODBUS))
  E_ERR_T rstatus = E_ERR_Success;
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_MODBUS))

  nReturn = MODBUS_MESSAGE_OK;
  tmp = STR_ByteSwap16Bit(*(unsigned short *)&aucMessage[nMessageLen-2]);

  #if ((defined CFG_DEBUG_MSG) && (_CFG_DEBUG_MODBUS))
  printf("[CMM] ModbusReplyCheck, computed crc val = 0x%04x, packet crc val = "
         " 0x%04x\n",
      STR_ByteSwap16Bit(ModbusCRC(aucMessage, nMessageLen-2)),
      *(UINT16 *)&aucMessage[nMessageLen-2]);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_MODBUS))

  // Slave address error.
  if (aucMessage[0] != aucSendBuffer[0])
  {
    //#ifdef CFG_PRN_ERR
    printf("ERR  [CMM] ModbusReplyCheck, slave address error. 0x%02x(Rx) != "
           "0x%02x(Tx)\n", aucMessage[0], aucSendBuffer[0]);
    //#endif // CFG_PRN_ERR
    nReturn = MODBUS_ERROR_CRC;
  }
  else if( STR_ByteSwap16Bit(ModbusCRC(aucMessage, nMessageLen-2) )!=  //Check CRC
    *(unsigned short *)&aucMessage[nMessageLen-2] )
  {
    //#ifdef CFG_PRN_ERR
    printf("ERR  [CMM] ModbusReplyCheck, CRC error\n");
    //#endif // CFG_PRN_ERR

    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_MODBUS))
    rstatus = SYS_PrnDataBlock((UINT8 *)aucMessage, nMessageLen, 10);
    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_PRN_ERR
      printf("ERR  [CMM] ModbusReplyCheck, rstatus = %d\n", rstatus);
      #endif // CFG_PRN_ERR
    }
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_MODBUS))

    nReturn = MODBUS_ERROR_CRC;
  }
  else
  {
    //Reply FC different to command FC, Link OK, command did not execute
    //correctly.
	  //Yang Tao: Add the error code reply: Query code|0x80
    if((aucMessage[1] != aucSendBuffer[1]) && (aucMessage[1] != (aucSendBuffer[1]|0x80)) )
    {
	  //#if ((defined CFG_PRN_ERR) && CFG_PRN_ERR_MDB)
      printf("ERR  [CMM] ModbusReplyCheck, invalid function code: 0x%02x(Rx) != "
           "0x%02x(Tx)\n", aucMessage[1], aucSendBuffer[1]);
      SYS_PrnDataBlock((const UINT8 *) aucMessage, nMessageLen, 10);
      //#endif // ((defined CFG_PRN_ERR) && CFG_PRN_ERR_MDB_LENGTH)
      nReturn = MODBUS_ERROR_EXCEPTION;
    }
    else
    {
      tmp = STR_ByteSwap16Bit(*(unsigned short *)&aucSendBuffer[4]);
     #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MODBUS)
      printf("[CMM] ModbusReplyCheck, aucSendBuff[1] = 0x%02x\n",
              aucSendBuffer[1]);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_MODBUS))
      switch(aucSendBuffer[1])  //Check reply length.
      {
        case 0x02:  //Read bit command response.

        nDataLen = tmp;
        if((nDataLen % 8 ) == 0 )
          nDataLen /= 8;
        else
        {
          nDataLen /= 8;
          nDataLen ++;
        }

        //Reply data length field error. //040707 Yu Wei
        if( nDataLen != aucMessage[2])
        {
          nDataLen = 0;
        }
        else
        {
          nDataLen += 5;
        }

        break;

      case 0x03:  //Read word command response.
      case 0x04:
        nDataLen = ( tmp ) * 2;   //040707 Yu Wei

        //Reply data length field error. //040707 Yu Wei
        if( nDataLen != aucMessage[2])
        {
          nDataLen = 0;
        }
        else
        {
          nDataLen += 5;  //Response (read command) message size = data length +5,
                  // 5 = 1(slave address)
                  //   + 1(function code)
                  //   + 1(quantity of data in byte)
                  //   + 2(CRC)
        }
        break;

      case 0x0F: // Write bit command
      case 0x10:
    	//Yang Tao: Add the Exception reply from control command query,which the length
    	 // is 5 rather than 8
      if(aucMessage[1] == (aucSendBuffer[1]|0x80))
    	  nDataLen = 5;
      else
          nDataLen = 8;    //Response (write command) message size,
                  // 8 = 1(slave address)
                  //   + 1(function code)
                  //   + 2(start address)
                  //   + 2(quantity of data in bytes)
                  //   + 2(CRC)
        break;

      default:
        nDataLen = nMessageLen;  //Don't check message length.
        nReturn = MODBUS_ERROR_EXCEPTION;
     //   #ifdef CFG_PRN_ERR
        printf("ERR  [CMM] ModbusReplyCheck, MODBUS_ERROR_EXCEPTION\n");
     //   #endif // CFG_PRN_ERR
        break;
      }

      //Check message length
      if((nMessageLen < 0x100) && (nDataLen != nMessageLen))
      {
//        #if ((defined CFG_PRN_ERR) && CFG_PRN_ERR_MDB_LENGTH)
        printf("ERR  [CMM] ModbusReplyCheck, data length(0x%04x) != "
               "message length(0x%04x)\n", nDataLen, nMessageLen);
 //       #endif // ((defined CFG_PRN_ERR) && CFG_PRN_ERR_MDB_LENGTH)
        nReturn = MODBUS_ERROR_EXCEPTION;
      }
    }
  }
  return nReturn;
} // ModbusReplyCheck
/*******************************************************************************
Purpose:
  Check modbus reply message for slave address, function code, But no CRC
  2014.05.30 Yang Tao

Input:
  aucMessage    -- Modbus reply. (in)
  nMessageLen    -- Modbus reply size. (in)
  aucSendBuffer  -- The buffer for sending command. (in)

Return:
  Check result:
  MODBUS_ERROR_EXCEPTION    -- Exception, command did not execute correctly.
  MODBUS_ERROR_CRC      -- Slave address error or CRC error.
  MODBUS_MESSAGE_OK      -- The message is a correct Modbus response.

History

     Name        Date        Remark

 Bryan Chong  18-Mar-2011  Remove length checking to address ECS requirememnt

*******************************************************************************/
// 20140606: DM comments that LAN1 and LAN2 not using standard Modbus protocol
// still need use CRC check
//int ModbusReplyCheck_LANSWC(unsigned char *aucMessage, int nMessageLen,
//                     unsigned char *aucSendBuffer)
//{
//  int nDataLen;        //Calculate message size.
//  int nReturn;        //Return result;
//  unsigned short tmp;
//  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_MODBUS))
//  E_ERR_T rstatus = E_ERR_Success;
//  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_MODBUS))
//
//  nReturn = MODBUS_MESSAGE_OK;
//  tmp = STR_ByteSwap16Bit(*(unsigned short *)&aucMessage[nMessageLen-2]);
//
//  #if ((defined CFG_DEBUG_MSG) && (_CFG_DEBUG_MODBUS))
//  printf("[CMM] ModbusReplyCheck, computed crc val = 0x%04x, packet crc val = "
//         " 0x%04x\n",
//      STR_ByteSwap16Bit(ModbusCRC(aucMessage, nMessageLen-2)),
//      *(UINT16 *)&aucMessage[nMessageLen-2]);
//  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_MODBUS))
//
//  // Slave address error.
//  if (aucMessage[0] != aucSendBuffer[0])
//  {
//    #ifdef CFG_PRN_ERR
//    printf("ERR  [CMM] ModbusReplyCheck, slave address error. 0x%x(Rx) != "
//           "0x%x(Tx)\n", aucMessage[0], aucSendBuffer[0]);
//    #endif // CFG_PRN_ERR
//    nReturn = MODBUS_ERROR_SLAVE_ADD;
//  }
//
//  else
//  {
//    //Reply FC different to command FC, Link OK, command did not execute
//    //correctly.
//	  //Yang Tao: Add the error code reply: Query code|0x80
//    if((aucMessage[1] != aucSendBuffer[1]) && (aucMessage[1] != (aucSendBuffer[1]|0x80)) )
//    {
//  //  #if ((defined CFG_PRN_ERR) && CFG_PRN_ERR_MDB)
//      printf("ERR  [CMM] ModbusReplyCheck, invalid function code:\n");
//      SYS_PrnDataBlock((const UINT8 *) aucMessage, nMessageLen, 10);
//  //  #endif // ((defined CFG_PRN_ERR) && CFG_PRN_ERR_MDB_LENGTH)
//      nReturn = MODBUS_ERROR_FUNC;
//    }
//    else
//    {
//      tmp = STR_ByteSwap16Bit(*(unsigned short *)&aucSendBuffer[4]);
//     #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MODBUS)
//      printf("[CMM] ModbusReplyCheck, aucSendBuff[1] = 0x%02x\n",
//              aucSendBuffer[1]);
//      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_MODBUS))
//      switch(aucSendBuffer[1])  //Check reply length.
//      {
//        case 0x02:  //Read bit command response.
//
//        nDataLen = tmp;
//        if((nDataLen % 8 ) == 0 )
//          nDataLen /= 8;
//        else
//        {
//          nDataLen /= 8;
//          nDataLen ++;
//        }
//
//        //Reply data length field error. //040707 Yu Wei
//        if( nDataLen != aucMessage[2])
//        {
//          nDataLen = 0;
//        }
//        else
//        {
//          nDataLen += 5;
//        }
//
//        break;
//
//      case 0x03:  //Read word command response.
//      case 0x04:
//        nDataLen = ( tmp ) * 2;   //040707 Yu Wei
//
//        //Reply data length field error. //040707 Yu Wei
//        if( nDataLen != aucMessage[2])
//        {
//          nDataLen = 0;
//        }
//        else
//        {
//          nDataLen += 5;  //Response (read command) message size = data length +5,
//                  // 5 = 1(slave address)
//                  //   + 1(function code)
//                  //   + 1(quantity of data in byte)
//                  //   + 2(CRC)
//        }
//        break;
//
//      case 0x0F: // Write bit command
//      case 0x10:
//    	//Yang Tao: Add the Exception reply from control command query,which the length
//    	 // is 5 rather than 8
//      if(aucMessage[1] == (aucSendBuffer[1]|0x80))
//    	  nDataLen=5;
//
//      else
//          nDataLen = 8;    //Response (write command) message size,
//                  // 8 = 1(slave address)
//                  //   + 1(function code)
//                  //   + 2(start address)
//                  //   + 2(quantity of data in bytes)
//                  //   + 2(CRC)
//
//        break;
//
//      default:
//        nDataLen = nMessageLen;  //Don't check message length.
//        nReturn = MODBUS_ERROR_EXCEPTION;
//     //   #ifdef CFG_PRN_ERR
//        printf("ERR  [CMM] ModbusReplyCheck, MODBUS_ERROR_EXCEPTION\n");
//     //   #endif // CFG_PRN_ERR
//        break;
//      }
//
//      //Check message length
//      if((nMessageLen < 0x100) && (nDataLen != nMessageLen))
//      {
////        #if ((defined CFG_PRN_ERR) && CFG_PRN_ERR_MDB_LENGTH)
//        printf("ERR  [CMM] ModbusReplyCheck, data length(0x%04x) != "
//               "message length(0x%04x)\n", nDataLen, nMessageLen);
// //       #endif // ((defined CFG_PRN_ERR) && CFG_PRN_ERR_MDB_LENGTH)
//        nReturn = MODBUS_ERROR_MESSAGE_LEN;
//      }
//    }
//  }
//  return nReturn;
//} // ModbusReplyCheck
/************************************************************************
Purpose:
  Check modbus command.
  Re-designed in 31 march 2004, Yu Wei

Input:
  aucMessage    -- Modbus command. (in)
  nMessageLen    -- Modbus command size. (in)
  ucSlaveAddress  -- Modbus slave address. (in)

Return:
  Check result:
  MODBUS_ERROR_EXCEPTION3    -- Data size error.
  MODBUS_ERROR_EXCEPTION1    -- Function code error.
  MODBUS_ERROR_CRC      -- Slave address error or CRC error.
  MODBUS_MESSAGE_OK      -- The message is a correct Modbus command.

************************************************************************/
int ModbusCMDCheck(unsigned char *aucMessage, int nMessageLen,
                   unsigned char ucSlaveAddress)
{
  int nReturn = MODBUS_MESSAGE_OK;        //Return result;
  unsigned short tmp;


  tmp = STR_ByteSwap16Bit(*(unsigned short *)&aucMessage[nMessageLen-2]);
  if(aucMessage[0] != ucSlaveAddress)    //Check slave address
  {
    nReturn = MODBUS_ERROR_SLAVE_ADD;
  }

  else if((ModbusCRC(aucMessage,nMessageLen-2) !=  tmp))  //Check CRC.
  {
    nReturn = MODBUS_ERROR_CRC;
  }
  else
  {
    switch(aucMessage[1])  //check function code.
    {
    case E_MB_FC_Read_N_Bits:  //Read bit command.
      nReturn = ModbusReadBitCheck(aucMessage, nMessageLen);
      break;

    case E_MB_FC_Read_N_Words:
      nReturn = ModbusReadWordCheck(aucMessage, nMessageLen);
      break;

    case E_MB_FC_Write_N_Bits:  //Write bit command
      nReturn = ModbusWriteBitCheck(aucMessage, nMessageLen);
      break;

    case E_MB_FC_Write_N_Words:  //Write word command.
      nReturn = ModbusWriteWordCheck(aucMessage, nMessageLen);
      break;

    default:
      nReturn = MODBUS_ERROR_EXCEPTION1;
      #ifdef CFG_PRN_ERR
      printf("ERR  [CMM] ModbusCMDCheck, unhandle modbus function code %d\n",
             aucMessage[1]);
      #endif // CFG_PRN_ERR
    }
  }

  return nReturn;
}
//20140530 Yang Tao
/************************************************************************
Purpose:
  Check modbus command for LAN SWC.

Input:
  aucMessage    -- Modbus command. (in)
  nMessageLen    -- Modbus command size. (in)
  ucSlaveAddress  -- Modbus slave address. (in)

Return:
  Check result:
  MODBUS_ERROR_EXCEPTION3    -- Data size error.
  MODBUS_ERROR_EXCEPTION1    -- Function code error.
  MODBUS_MESSAGE_OK      -- The message is a correct Modbus command.

************************************************************************/
// 20140606: DM comments that LAN1 and LAN2 not using standard Modbus protocol
// still need use CRC check
/*int ModbusCMDCheck_LANSWC(unsigned char *aucMessage, int nMessageLen,
                   unsigned char ucSlaveAddress)
{
  int nReturn = MODBUS_MESSAGE_OK;        //Return result;
  unsigned short tmp = -1;

  tmp = STR_ByteSwap16Bit(*(unsigned short *)&aucMessage[nMessageLen-2]);

  if(aucMessage[0] != ucSlaveAddress)    //Check slave address
  {
    nReturn = MODBUS_ERROR_SLAVE_ADD; //F004
  }
  else
  {
    switch(aucMessage[1])  //check function code.
    {
    case E_MB_FC_Read_N_Bits:  //Read bit command.
      nReturn = ModbusReadBitCheck(aucMessage, nMessageLen);
      break;

    case E_MB_FC_Read_N_Words:
      nReturn = ModbusReadWordCheck(aucMessage, nMessageLen);
      break;

    case E_MB_FC_Write_N_Bits:  //Write bit command
      nReturn = ModbusWriteBitCheck(aucMessage, nMessageLen);
      break;

    case E_MB_FC_Write_N_Words:  //Write word command.
      nReturn = ModbusWriteWordCheck(aucMessage, nMessageLen);
      break;

    default:
      nReturn = MODBUS_ERROR_EXCEPTION1;
      #ifdef CFG_PRN_ERR
      printf("ERR  [CMM] ModbusCMDCheck, unhandled modbus function code %d\n",
             aucMessage[1]);
      #endif // CFG_PRN_ERR
    }
  }

  return nReturn;
}
*/
/************************************************************************
Purpose:
  Check modbus read bits command

Input:
  aucMessage    -- Modbus command. (in)
  nMessageLen    -- Modbus command size. (in)

Return:
  Check result:
  MODBUS_ERROR_EXCEPTION3    -- Data size error.
  MODBUS_MESSAGE_OK      -- Read bits command size and data range OK.

************************************************************************/
int ModbusReadBitCheck(unsigned char *aucMessage, int nMessageLen)
{
  int nReturn = MODBUS_MESSAGE_OK;        //Return result;
  unsigned short tmp[2];

  if(nMessageLen != MODBUS_READ_CMD_SIZE)  //Read bit command size must be MODBUS_READ_CMD_SIZE (8)
  {
    nReturn = MODBUS_ERROR_EXCEPTION3;    //040331 Yu Wei
  }
  else
  {
    tmp[0] = STR_ByteSwap16Bit(*(unsigned short *)&aucMessage[4]);
    if((tmp[0] >MODBUS_MAX_READ_BIT_CMD_SIZE)||
      (tmp[0] <1))  //Read bit command data range.
    {
      nReturn = MODBUS_ERROR_EXCEPTION3;
    }
    tmp[1] = STR_ByteSwap16Bit(*(unsigned short *)&aucMessage[2]);
    if(tmp[0] > ((0xFFFF - tmp[1]) + 1))
    {                    //Address range excess max value (0xFFFF)
                        //040429 Yu Wei
      nReturn = MODBUS_ERROR_EXCEPTION3;
    }
  }

  return nReturn;
}

/************************************************************************
Purpose:
  Check modbus read words command

Input:
  aucMessage    -- Modbus command. (in)
  nMessageLen    -- Modbus command size. (in)

Return:
  Check result:
  MODBUS_ERROR_EXCEPTION3    -- Data size error.
  MODBUS_MESSAGE_OK      -- Read words command size and data range OK.

************************************************************************/
int ModbusReadWordCheck(unsigned char *aucMessage, int nMessageLen)
{
  int nReturn = MODBUS_MESSAGE_OK;        //Return result;
  unsigned short tmp;

  if(nMessageLen != MODBUS_READ_CMD_SIZE)  //Read command size must be MODBUS_READ_CMD_SIZE (8)
  {
    nReturn = MODBUS_ERROR_EXCEPTION3;    //040331 Yu Wei
  }
  else
  {
    tmp = STR_ByteSwap16Bit(*(unsigned short *)&aucMessage[4]);
    if(tmp<1)  // No limit for maxmium read data bytes,
    {                    // based on C830 Modbus protocol document.
      nReturn = MODBUS_ERROR_EXCEPTION3;
    }

    // when start addr is 0xffff, this condition becomes true.
    // This indicat the maximum range is from 1 to 0xfffe
    if(tmp > ((0xFFFF - (STR_ByteSwap16Bit(*(unsigned short *)&aucMessage[2]))) + 1))
    {
      //Address range excess max value (0xFFFF)
      //040429 Yu Wei
      nReturn = MODBUS_ERROR_EXCEPTION3;
    }
  }

  return nReturn;
}

/************************************************************************
Purpose:
  Check modbus write bits command

Input:
  aucMessage    -- Modbus command. (in)
  nMessageLen    -- Modbus command size. (in)

Return:
  Check result:
  MODBUS_ERROR_EXCEPTION3    -- Data size error.
  MODBUS_MESSAGE_OK      -- Read words command size and data range OK.

************************************************************************/
int ModbusWriteBitCheck(unsigned char *aucMessage, int nMessageLen)
{
  int nDataLen;        //Calculate message size.
  int nReturn = MODBUS_MESSAGE_OK;        //Return result;
  unsigned short tmp;

  nDataLen = aucMessage[6] + 9;  //command size = data length +9,
                  // 9 = 1(slave address)
                  //   + 1(function code)
                  //   + 2(start address)
                  //   + 2(quantity of data in bit)
                  //   + 1(quantity of data in byte)
                  //   + 2(CRC)

  if(nDataLen != nMessageLen)
  {
    nReturn = MODBUS_ERROR_EXCEPTION3;    //040331 Yu Wei
  }
  else
  {
    tmp = STR_ByteSwap16Bit(*(unsigned short *)&aucMessage[4]);
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MODBUS)
    printf("[CMM] ModbusWriteBitCheck, tmp = 0x%04x\n",
           tmp);
    SYS_PrnDataBlock((const UINT8 *) aucMessage, nMessageLen, 10);
    #endif  // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MODBUS)
    if((tmp >MODBUS_MAX_WRITE_BIT_CMD_SIZE)||
      (tmp<1))  //Check write bit command bit data range.
    {
      nReturn = MODBUS_ERROR_EXCEPTION3;
    }
    else
    {
      //convert bit length to byte length.
      nDataLen = (tmp)/8;
      if(((tmp)&0x0007) != 0)
        nDataLen++;
    }
  }

  return nReturn;
}

/************************************************************************
Purpose:
  Check modbus write words command

Input:
  aucMessage    -- Modbus command. (in)
  nMessageLen    -- Modbus command size. (in)

Return:
  Check result:
  MODBUS_ERROR_EXCEPTION3    -- Data size error.
  MODBUS_MESSAGE_OK      -- Read words command size and data range OK.

************************************************************************/
int ModbusWriteWordCheck(unsigned char *aucMessage, int nMessageLen)
{
  int nDataLen;        //Calculate message size.
  int nReturn = MODBUS_MESSAGE_OK;        //Return result;
  char acTempLog[512];
  unsigned short tmp[2];

  tmp[0] = STR_ByteSwap16Bit(*(unsigned short *)&aucMessage[4]);

  nDataLen = tmp[0] * 2 + 9;   //checked with switch to primary command ks
                          //Command size is same as write bit command.
                          //It is based on word field.

  #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_MODBUS)
  printf("[CMM] ModbusWriteWordCheck, tmp[0] = 0x%04x, nDataLen 0x%04x, "
         "nMessageLen 0x%04x\n",
         tmp[0], nDataLen, nMessageLen);
  #endif  // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MODBUS)

  if(nDataLen != nMessageLen)
  {
    nReturn = MODBUS_ERROR_EXCEPTION3;    //040331 Yu Wei
    sprintf(acTempLog,"[CMM] ModbusWriteWordCheck 1, tmp[0] = 0x%04x, nDataLen 0x%04x, "
           "nMessageLen 0x%04x\n",
           tmp[0], nDataLen, nMessageLen);
    g_pDebugLog->LogMessage(acTempLog);

  }
  else
  {
    tmp[1] = STR_ByteSwap16Bit((unsigned short)aucMessage[6]);

    #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_MODBUS)
    printf("[CMM] ModbusWriteWordCheck, tmp[0] = 0x%04x\n", tmp[0]);
    SYS_PrnDataBlock((const UINT8 *) aucMessage, nMessageLen, 10);
    #endif  // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MODBUS)

    if((tmp[0] > MODBUS_MAX_WRITE_BYTE_CMD_SIZE)||(tmp[0]<1)||
       (tmp[0] != (unsigned short)aucMessage[6]>>1))
    {
        nReturn = MODBUS_ERROR_EXCEPTION3;    //040331 Yu Wei
        sprintf(acTempLog,"[CMM] ModbusWriteWordCheck 2, tmp[0] = 0x%04x, nDataLen 0x%04x, "
               "nMessageLen 0x%04x\n",
               tmp[0], nDataLen, nMessageLen);
        g_pDebugLog->LogMessage(acTempLog);
      nReturn = MODBUS_ERROR_EXCEPTION3;
    }
  }

  return nReturn;
}

/*------------------------------------------------------------------------------
Purpose:
  Contruct and send modbus error message through LAN

Parameter:
  nFd              [in] socket file descriptor
  ucSlaveAddress   [in] slave address
  ucFnCode         [in] function code
  ucExceptionCode  [in] exception code

History
  Name         Date         Remark
  ----         ----         ------
 Bryan Chong  14-Jun-2010  Resolve role switching issue by implementing
                           LNC_ReceiveMsg [PR25]
 Bryan Chong  19-Aug-2010  Replace send with LNC_SendMsg routine
------------------------------------------------------------------------------*/
void ModbusSendErrorReply(int nFd, unsigned char ucSlaveAddress,
                          unsigned char ucFnCode, unsigned char ucExceptionCode)
{
  unsigned char ucErrorReply[MODBUS_EXCEPTION_REPLY_SIZE];  //040330 Yu Wei
  INT32 ntxsz;
  E_ERR_T rstatus = E_ERR_Success;
  #ifdef CFG_PRN_ERR
  CHAR acErrMsg[100] = {0};
  #endif // CFG_PRN_ERR

  ucErrorReply[0] = ucSlaveAddress;
  ucErrorReply[1] = ucFnCode | 0x80;  //Execption use | not +. 040330 Yu Wei
  ucErrorReply[2] = ucExceptionCode;

  *(UINT16 *)&ucErrorReply[3] = STR_ByteSwap16Bit(ModbusCRC(ucErrorReply, 3));


  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MODBUS)
  printf("[CMM] ModbusSendErrorReply, fd %d tx modbus error msg %d bytes:\n",
          nFd, MODBUS_EXCEPTION_REPLY_SIZE);
  SYS_PrnDataBlock((const UINT8 *) ucErrorReply, MODBUS_EXCEPTION_REPLY_SIZE,
                   10);
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_MODBUS))

  // 20100819 BC (Rqd by ZSL)
  rstatus = LNC_SendMsg(nFd, ucErrorReply, MODBUS_EXCEPTION_REPLY_SIZE, &ntxsz);
  if(rstatus != E_ERR_Success)
  {
    #ifdef CFG_PRN_ERR
    ERR_GetMsgString(rstatus, acErrMsg);
    printf("[CMM] ModbusSendErrorReply, err msg %s\n", acErrMsg);
    #endif // CFG_PRN_ERR
  }

}// ModbusSendErrorReply


