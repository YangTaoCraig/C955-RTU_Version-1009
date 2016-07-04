#ifndef MODBUS_PLC_H_
#define MODBUS_PLC_H_

//Modbus constants
#define PLC_MODBUS_MIN_CMD_SIZE      8    //minimum Modbus cmd size
#define PLC_MODBUS_MIN_REPLY_SIZE    5    //minimum Modbus reply cmd size
#define PLC_MODBUS_MAX_CMD_SIZE      264    //maximum Modbus cmd size
#define PLC_MODBUS_FNCODE4_CMD_SIZE    8    //Modbus fn code 4 cmd size
#define PLC_MODBUS_EXCEPTION0      0    //Device no reply.
#define PLC_MODBUS_EXCEPTION1      1    //error in received function code
#define PLC_MODBUS_EXCEPTION2      2    //error in received address
#define PLC_MODBUS_EXCEPTION3      3    //error in received data value
#define PLC_MODBUS_EXCEPTION4      4    //Command error or cannot execute.




#define PLC_MODBUS_HEARDER_LENTH    7
#define PLC_MODBUS_READ_CMD_SIZE    12    //Modbus read fn cmd size
#define PLC_MODBUS_EXCEPTION_REPLY_SIZE  9    //Exception Modbus reply size
#define PLC_MODBUS_WRITE_REPLY_SIZE    12    //Modbus write command reply size


#define PLC_MODBUS_MAX_READ_BIT_CMD_SIZE  2000  //maximum Modbus read bit command size
#define PLC_MODBUS_MAX_READ_BYTE_CMD_SIZE  125    //maximum Modbus read byte command size
#define PLC_MODBUS_MAX_WRITE_BIT_CMD_SIZE  1968  //maximum Modbus write bit command size
#define PLC_MODBUS_MAX_WRITE_BYTE_CMD_SIZE  123    //maximum Modbus write byte command size

#define PLC_MODBUS_SEND_REPLY_TIMEOUT  5000  //send reply timeout in ms

//Modbus message error code
#define PLC_MODBUS_MESSAGE_OK      0x0001  //Message OK
#define PLC_MODBUS_ERROR_MESSAGE_LEN  0xF001  //Message length error
#define PLC_MODBUS_ERROR_EXCEPTION    0xF002  //Message exception
#define PLC_MODBUS_ERROR_CRC      0xF003  //CRC check error
#define PLC_MODBUS_ERROR_SLAVE_ADD    0xF004  //slave address error
#define PLC_MODBUS_ERROR_EXCEPTION1    0xFF01  //exception1: function code error
#define PLC_MODBUS_ERROR_EXCEPTION2    0xFF02  //exception2: start address error
#define PLC_MODBUS_ERROR_EXCEPTION3    0xFF03  //exception3: quantity of inputs error
#define PLC_MODBUS_ERROR_EXCEPTION4    0xFF04  //exception4: read discrete error
#define PLC_MODBUS_ERROR_EXCEPTION5    0xFF05    //sequence is not match


typedef struct plc_modbus_request_st{
  UINT16 transaction_no;
  UINT16 protocol_id;
  UINT16 data_byte_length;
  UINT8  slave_addr;
  UINT8  funtion_code;
  UINT16 start_addr;
  UINT16 data_word_length;
}MB_PLC_RQ_T;


typedef struct plc_modbus_response_header_st{
  UINT16 transaction_no;
  UINT16 protocol_id;
  UINT16 data_byte_length;
  UINT8 slave_addr;
  UINT8 funtion_code;
  UINT8 data_byte_count;
  UINT8 reserved;
}MB_PLC_RESP_T;

//unsigned short ModbusCRC_PLC(unsigned char *,int);
//UINT32 ModbusReplyCheck_PLC(UINT8 *aucRecvMessage, INT32 nRecvMessageLen,
                            //UINT8 *aucSendBuffer);


#endif /*MODBUS_PLC_H_*/
