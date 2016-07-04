/*----------------------------------------------------------------------------

            Copyright (c) 2009 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   mdb_def.h                                           D1.0.4

 COMPONENT

   MDB - Modbus management

 DESCRIPTION

   This file contains the type definitions for Modbus management


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     28-Dec-2009      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef _MDB_DEF_H_
#define _MDB_DEF_H_
#ifdef __cplusplus
extern "C"
{
#endif

#define MDB_MAX_MODBUS_DATA_SZ   2048

#ifdef NOT_USED
// Standard modbus function code reference
typedef enum e_modbus_function_code{
  E_MDB_FNC_Reserved_0x00,
  E_MDB_FNC_ReadCoilBits,
  E_MDB_FNC_ReadDiscreteInputBits,
  E_MDB_FNC_ReadHoldingRegisterWords,
  E_MDB_FNC_ReadInputRegisterWords,
  E_MDB_FNC_WriteSingleCoilBit,
  E_MDB_FNC_WriteSingleRegisterWord,
  E_MDB_FNC_Reserved_0x06,
  E_MDB_FNC_Reserved_0x07,
  E_MDB_FNC_Reserved_0x08,
  E_MDB_FNC_Reserved_0x09,
  E_MDB_FNC_Reserved_0x0A,
  E_MDB_FNC_Reserved_0x0B,
  E_MDB_FNC_Reserved_0x0C,
  E_MDB_FNC_Reserved_0x0D,
  E_MDB_FNC_Reserved_0x0E,
  E_MDB_FNC_WriteMultipleCoilBits,
  E_MDB_FNC_WriteMultipleRegisterWords,
  E_MDB_FNC_Reserved_0x11,
  E_MDB_FNC_EndOfList
}E_MDB_FNC_T;

// Standard modbus exception code reference
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
} E_MDB_ECODE;
#endif // NOT_USED

/*
Modbus read command packet
(Description, byte count):
-------------------------------------------------
| slv  | fn   | read addr | Qty of | Byte | CRC |
| addr | code | start     | regs   | cnt  |     |
-------------------------------------------------
|   1  |   1  |     2     |    2   |   1  |  2  |
-------------------------------------------------
*/
typedef struct modbus_read_request_packet_st{
  UINT8  slave_addr;
  UINT8  function_code;
  UINT16 start_addr;
  union word_quantity{
    UINT16 num_of_bits;
    UINT16 num_of_words;
  }quantity;
  union byte_cnt_crc{
    struct word_data_st{
      UINT8 byte_cnt;
      UINT8 *pdatabyte;
      UINT16 *pcrc_val;
    };
    UINT16 crc_val;
  }crc_word;

} MDB_READ_RQ_T;


/*
Modbus read command reply packet
(Description, byte count):
-----------------------------------------------
| slv  | fn   | total byte | data   ... | CRC |
| addr | code | read       | start      |     |
-----------------------------------------------
|   1  |   1  |     1      |    ...     |  2  |
-----------------------------------------------
*/

typedef struct modbus_read_reply_packet_st{
  UINT8  slave_addr;
  UINT8  function_code;
  UINT8  total_bytes;
  //UINT8  *pdata;
} MDB_READ_REPLY_T;

/*
Modbus read command reply packet using pointer structure 
(Description, byte count):
-----------------------------------------------
| slv  | fn   | total byte | data   ... | CRC |
| addr | code | read       | start      |     |
-----------------------------------------------
|   1  |   1  |     1      |    ...     |  2  |
-----------------------------------------------
*/
typedef struct modbus_read_reply_pointer_packet_st{
  UINT8  *pslave_addr;
  UINT8  *pfunction_code;
  UINT8  *ptotal_bytes;
  UINT8  *pdata;
  UINT16 *pcrc;
} MDB_READ_REPLYP_T;

/*
Write bit request packet,
(Description, byte count):
-------------------------------------------------------------------------------
| slv  | fn   | coil addr | Qty of | Byte | Data  ...
| addr | code | start     | coils  | cnt  | start
-------------------------------------------------------------------------------
|   1  |   1  |     2     |    2   |   1  | ...
-------------------------------------------------------------------------------
*/
typedef struct modbus_writebit_packet_st{
  union {
    UINT16 header;
    struct {
      UINT8 slave_addr;
      UINT8 function_code;
    }sladdr_fn_comp;
  }u_slave_addr_n_fn_code;
  UINT16  coil_addr_start;
  UINT16  qty_coils;
  union {
    UINT16 combined;
    struct {
      UINT8 total_bytes_to_write;
      UINT8 byteDataStart;  // start with least significant byte
    } bytes_bdata_comp;
  }u_bytes_bdata;
} MDB_WRBIT_RQ_T;

/*
LAN write bit request packet:
-------------------------------------------------------------------------------
| trans | proto | total data byte | slv addr | fn code | start addr | total
|       |       |                 |          |         |            | data word
-------------------------------------------------------------------------------
|   2   |   2   |     2           |    1     |   1     |      2     |    2
|       |       |                 |          |         |            |
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
| total      | data   |  . . .
| data byte  | start  |
-------------------------------------------------------------------------------
|   1        |        |
-------------------------------------------------------------------------------
*/
typedef struct lan_modbus_write_bit_packet_st{
  UINT16 transaction_number;
  UINT16 protocol_type;
  UINT16 total_data_byte;         // total data length for the packet
  union {
    UINT16 combined;
    struct {
      UINT8  slave_addr;
      UINT8  function_code;
    }sladdr_fn_comp;
  }u_slave_addr_n_fn_code;
  UINT16 start_addr;
  UINT16 total_words_to_write;
  union{
    UINT16 combined;
    struct {
      UINT8 total_bytes_to_write;
      UINT8 byteDataStart;
    } bytes_wdata_comp;
  }u_bytes_wdata;

} MDB_LAN_WRITE_RQ_T;

typedef struct server_modbus_request_packet_st{
  union {
    UINT16 combined;
    struct {
      UINT8  slave_addr;
      UINT8  function_code;
    }sladdr_fn_comp;
  }u_slave_addr_n_fn_code;
  UINT16 start_addr;
  union{
    UINT16 rq_words_to_write;
    UINT16 rq_bis_to_write;
  }u_rq_write_bit_word;
  union{
    UINT16 combined;
    struct {
      UINT8 rq_bytes_to_write;
      UINT8 cmd_byte_start;
    } rq_wrbytes_wrdata_comp;
  }u_rq_wrbytes_wrdata_comp;
}MDB_SVR_RQ_T;

#ifdef __cplusplus
}
#endif

#endif //_MDB_DEF_H_
