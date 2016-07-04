/************************************************************
*                             *
* Project:    C830 RTU                  *
* Module:   Common Module               *
* File :      CMM_Modbus.h                *
* Author:   Yu Wei                    *
*                             *
* Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************

**************************************************************

DESCRIPTION

  This is header file for modbus function.

*************************************************************
Modiffication History
---------------------

Version: CMM D.1.1.1
  01  21 March 2003, Yu Wei,
    Start to write.

Version: CMM D.1.3.1
  01  22 September 2003, Yu Wei,
    Move ModbusSendErrorReply from SPM to this file.

Version: CMM D.1.3.5
  01  31 March 2004, Yu Wei
    Modified ModbusReplyCheck().
    Refer to PR:OASYS-PMS 0053 and PR:OASYS-PMS 0153.

Version: CMM E.1.0.2
  01  01 Apr 2005, Tong Zhi Xiong
    Add ModbusReplyCheck2 function for SendReply2 use
**************************************************************/
#ifndef  _CMM_MODBUS_H
#define  _CMM_MODBUS_H

//Modbus constants
#define MODBUS_MIN_CMD_SIZE      8    //minimum Modbus cmd size
#define MODBUS_MIN_REPLY_SIZE    5    //minimum Modbus reply cmd size
#define MODBUS_MAX_CMD_SIZE      264    //maximum Modbus cmd size
#define MODBUS_FNCODE4_CMD_SIZE    8    //Modbus fn code 4 cmd size
#define MODBUS_EXCEPTION0      0    //Device no reply.
#define MODBUS_EXCEPTION1      1    //error in received function code
#define MODBUS_EXCEPTION2      2    //error in received address
#define MODBUS_EXCEPTION3      3    //error in received data value
#define MODBUS_EXCEPTION4      4    //Command error or cannot execute.
#define MODBUS_READ_CMD_SIZE    8    //Modbus read fn cmd size
#define MODBUS_EXCEPTION_REPLY_SIZE  5    //Exception Modbus reply size
#define MODBUS_WRITE_REPLY_SIZE    8    //Modbus write command reply size

#define MODBUS_MAX_READ_BIT_CMD_SIZE  2000  //maximum Modbus read bit command size
#define MODBUS_MAX_READ_BYTE_CMD_SIZE  130    //maximum Modbus read byte command size// 27 06 2016 YT: Change to 130 to solve the GDCC PWR 1 DI Table size is 127.
#define MODBUS_MAX_WRITE_BIT_CMD_SIZE  1968  //maximum Modbus write bit command size
#define MODBUS_MAX_WRITE_BYTE_CMD_SIZE  130   //maximum Modbus write byte command size, // 27 06 2016 YT: Change to 130 to solve the GDCC PWR 1 DI Table size is 127.
//#define MODBUS_MAX_WRITE_BYTE_CMD_SIZE  123    //maximum Modbus write byte command size

#define MODBUS_SEND_REPLY_TIMEOUT  5000  //send reply timeout in ms

//Modbus message error code
#define MODBUS_MESSAGE_OK      0x0001  //Message OK
#define MODBUS_ERROR_MESSAGE_LEN  0xF001  //Message length error
#define MODBUS_ERROR_EXCEPTION    0xF002  //Message exception
#define MODBUS_ERROR_CRC      0xF003  //CRC check error
#define MODBUS_ERROR_SLAVE_ADD    0xF004  //slave address error
#define MODBUS_ERROR_FUNC    0xF005  //function code error
#define MODBUS_ERROR_EXCEPTION1    0xFF01  //exception1: function code error
#define MODBUS_ERROR_EXCEPTION2    0xFF02  //exception2: start address error
#define MODBUS_ERROR_EXCEPTION3    0xFF03  //exception3: quantity of inputs error
#define MODBUS_ERROR_EXCEPTION4    0xFF04  //exception4: read discrete error

// modbus quantity of register during link monitor session
#define MB_LINK_MONITOR_QTY_OF_REG 4

typedef enum e_modbus_function_code{
  E_MB_FC_Reserved0,
  E_MB_FC_ReadCoils,
  E_MB_FC_Read_N_Bits,  // Read discrete input
  E_MB_FC_Read_HoldingRegister,
  E_MB_FC_Read_N_Words,  // Read input register
  E_MB_FC_WriteSingleCoil,
  E_MB_FC_WriteSingleRegister,
  E_MB_FC_ReadExceptionStatus,
  E_MB_FC_Diagnostic,
  E_MB_FC_GetCommEventCounter = 0x0B,
  E_MB_FC_GetCommEventLog,
  E_MB_FC_Write_N_Bits = 0x0F, // Write multiple coils
  E_MB_FC_Write_N_Words,       // Write multiple register
  E_MB_FC_ReportSlaveID,
  E_MB_FC_ReadFileRecord = 0x14,
  E_MB_FC_WriteFileRecord,
  E_MB_FC_MaskWriteRegister,
  E_MB_FC_ReadWriteMultipleRegister,
  E_MB_FC_ReadFifoQueue,
  E_MB_FC_EncapsulatedInterfaceTransport = 0x2B,
  E_MB_FC_EndOfList = 0xFF
}E_MB_FC;

typedef enum e_modbus_exception_code{
  E_MB_ECODE_Reserved_00,         // 00
  E_MB_ECODE_IllegalFunction,     // 01
  E_MB_ECODE_IllegalDataAddress,  // 02
  E_MB_ECODE_IllegalDataValue,    // 03
  E_MB_ECODE_SlaveDeviceFailure,  // 04
  E_MB_ECODE_Acknowledge,         // 05
  E_MB_ECODE_SlaveDeviceBusy,     // 06
  E_MB_ECODE_Reserved_07,         // 07
  E_MB_ECODE_MemoryParityError,   // 08
  E_MB_ECODE_Reserved_09,         // 09
  E_MB_ECODE_GatewayPathUnavailable, // 0A
  E_MB_ECODE_GatewayTargetDeviceFailedToRespond, // 0B
  E_MB_ECODE_EndOfList = 0xFF
} E_MB_ECODE;

// Read bits and read word query
typedef struct modbus_read_querry_st{
  UINT8 slave_addr;
  UINT8 func_code;
  UINT16 start_addr;
  UINT16 read_qty;
  UINT16 crc_val;
}MB_READQ_T;

// Since structure member is 16-bit alignment, dataword_start marks the
// starting position of data word
typedef struct modbus_write_querry_st{
  UINT8 slave_addr;
  UINT8 func_code;
  UINT16 start_addr;
  UINT16 write_qty;
  UINT8  total_bytes;
  UINT8  dataword_start;
}MB_WRITEQ_T;


unsigned short ModbusCRC(unsigned char *,int);
int ModbusReplyCheck(unsigned char *, int, unsigned char *);//040331 Yu Wei
//int ModbusReplyCheck_LANSWC(unsigned char *, int, unsigned char *);  // 20140530 Yang Tao
int ModbusCMDCheck(unsigned char *,  int, unsigned char);
//int ModbusCMDCheck_LANSWC(unsigned char *,  int, unsigned char);	//20140530 Yang Tao
int ModbusReadBitCheck(unsigned char *, int);
int ModbusReadWordCheck(unsigned char *, int);
int ModbusWriteBitCheck(unsigned char *, int);
int ModbusWriteWordCheck(unsigned char *, int);
void ModbusSendErrorReply(int, unsigned char, unsigned char, unsigned char);

#endif /* _CMM_MODBUS_H */


