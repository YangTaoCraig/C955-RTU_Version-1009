/************************************************************
*                             *
* Project:    C830 RTU                  *
* Module:   MMM                     *
* File :      MMM.h                   *
* Author:   Yu Wei                    *
*                             *
* Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************

DESCRIPTION

  This file implements the Maintenance Manangement Module.

*************************************************************

*************************************************************/

#ifndef  _MMM_H
#define  _MMM_H



#include "Common.h"


//Modbus constants
#define MMM_MIN_CMD_SIZE      8      //minimum Modbus read word cmd size
#define MMM_MAX_CMD_SIZE      (9 + 9*2)  //maximum Modbus write word cmd size

#define MODBUS_SEND_REPLY_TIMEOUT  5000  //send reply timeout in ms
#define MODBUS_EXCEPTION1      1    //error in received function code
#define MODBUS_EXCEPTION2      2    //error in received address
#define MODBUS_EXCEPTION3      3    //error in received data value


//MMM constants
#define MMM_TABLE_POLL_SIZE      64*1024  //polling table size in word

//send buffer size in byte, add 5 bytes for Modbus fn code 4 reply fields:
// slave address(1 byte), fn code(1 byte), no. of bytes read(1 byte) and CRC(2 bytes)
// add 3 bytes to round up to 8
#define MMM_SEND_BUFFER_SIZE    (MMM_TABLE_POLL_SIZE*2 + 8)

#define MMM_SWC_COPY_MAX_TRY    5    //maximum no. of attempts to copy SWCs' tables


//MMM cmd
#define MMM_CMD_LOGIN        10    //login MMM

#define MMM_CMD_SET_DOWNLOAD_CFG  30    //Set RTU download config.txt required flag.

#define MMM_CMD_SETCLOCK      50    //set clock 0x32

#define MMM_CMD_RESET        0    //reset RTU

#define MMM_CMD_SWC_POLLING_ENABLE_INHIBIT    100    //enable/inhibit SWC

#define MMM_CMD_SWC_WRITE_AI_MI    110    //Write a word to SWC AI, MI table  //040322 Yu Wei
#define MMM_CMD_SWC_WRITE_DI_DI8  120    //Write 16 bytes to SWC DI, DI8 table  //040322 Yu Wei

#define MMM_CMD_MEMORYTEST      80    //test memory integrity

#define MMM_CMD_GETCLOCK      60    //get clock

#define MMM_CMD_SERIAL_TEST     300    //Serisl port test command.
#define MMM_CMD_SERIALTEST1     301    //serial port test
#define MMM_CMD_SERIALTEST2     302
#define MMM_CMD_SERIALTEST3     303
#define MMM_CMD_SERIALTEST4     304
#define MMM_CMD_SERIALTEST5     305
#define MMM_CMD_SERIALTEST6     306
#define MMM_CMD_SERIALTEST7     307
#define MMM_CMD_SERIALTEST8     308
#define MMM_CMD_SERIALTEST9     311
#define MMM_CMD_SERIALTEST10    312
#define MMM_CMD_SERIALTEST11    313
#define MMM_CMD_SERIALTEST12    314
#define MMM_CMD_SERIALTEST13    315
#define MMM_CMD_SERIALTEST14    316
#define MMM_CMD_SERIALTEST15    317
#define MMM_CMD_SERIALTEST16    318

#define MMM_CMD_RTU_STATUS      400    //read RTU status table
#define MMM_CMD_RTU_POLL        450    //read polling table
#define MMM_CMD_RTU_CMD         460    //read RTU last command.

#define MMM_CMD_SWC_READ_TABLE  500    //Read SWC tables command
/*
#define MMM_CMD_POWER1_DI     501   //read POWER1 SWC tables
#define MMM_CMD_POWER1_AI     502
#define MMM_CMD_POWER1_MI     503

#define MMM_CMD_POWER2_DI     511   //read POWER2 SWC tables
#define MMM_CMD_POWER2_AI     512
#define MMM_CMD_POWER2_MI     513

#define MMM_CMD_POWER3_DI     521   //read POWER3 SWC tables
#define MMM_CMD_POWER3_AI     522
#define MMM_CMD_POWER3_MI     523

#define MMM_CMD_PLC1_DI       531   //read PLC1 SWC tables
#define MMM_CMD_PLC1_DI8      532
#define MMM_CMD_PLC1_MI       533

#define MMM_CMD_PLC2_DI       541   //read PLC2 SWC tables
#define MMM_CMD_PLC2_DI8      542
#define MMM_CMD_PLC2_MI       543

#define MMM_CMD_PLC3_DI       551   //read PLC3 SWC tables
#define MMM_CMD_PLC3_DI8      552
#define MMM_CMD_PLC3_MI       553

#define MMM_CMD_PLC4_DI       561   //read PLC4 SWC tables
#define MMM_CMD_PLC4_DI8      562
#define MMM_CMD_PLC4_MI       563

#define MMM_CMD_ECS_DI        571   //read ECS SWC tables
#define MMM_CMD_ECS_AI        572

#define MMM_CMD_WILD_DI       581   //read WILD SWC tables

#define MMM_CMD_FPS_DI        591   //read FPS SWC tables

#define MMM_CMD_TWP_DI        601   //read TWP SWC tables
#define MMM_CMD_TWP_DI8       602

#define MMM_CMD_PSD1_DI8      611   //read PSD1 SWC tables
#define MMM_CMD_PSD2_DI8      621   //read PSD2 SWC tables
#define MMM_CMD_PSD3_DI8      631   //read PSD3 SWC tables
#define MMM_CMD_PSD4_DI8      641   //read PSD4 SWC tables
*/

//MMM parameters
typedef struct
{
  //receive timeout in ms
  int nRecvTimeout;

  //listen ip
  char acListenIP[16];

  //Modbus slave address
  unsigned char ucModbusSlaveAddress;

  //polling table start and end word address, inclusive
  //unsigned short unTablePollStart;
  //unsigned short unTablePollEnd;
} tMMMParam;


//MMM task functions
int MMMTaskSpawn(void);

void *MMMMainTask(void *arg);
void StopMMMMainTask(void);

//MMM Modbus functions
void ModbusSendErrorReplyMMM(int nFd, unsigned char ucSlaveAddress, unsigned char ucFnCode,
              unsigned char ucExceptionCode);


//MMM class
class CMMM
{
  //member functions
  public:

    CMMM(unsigned short unListenPort);
    ~CMMM();
    E_ERR_T Main();

  private:
    int ModbusCmdCheck(int nFd, unsigned char *pucCmd, int nCmdSize);
    bool ReadCmdCheck(unsigned short);  //040322 Yu Wei
    bool WriteCmdCheck(unsigned short);  //040322 Yu Wei
    E_ERR_T clientHandler (INT32 nSockClient);
    unsigned short ProcessCmd(unsigned short unCmd,unsigned short *unParameter);
    unsigned short SetSWCPollingFlag(unsigned short); //040322 Yu Wei
    //unsigned short ReadSWCTable(unsigned short,  unsigned short);  //040322 Yu Wei
    UINT16 ReadSWCTable(UINT16 unSWCIndex, UINT16 unSWCTableID);
    unsigned short WriteSWCTable(unsigned short, unsigned short, unsigned short);  //040322 Yu Wei

    unsigned short ProcessCmdCode(unsigned short unCmd,unsigned short *unParameter);
    E_ERR_T CheckLogin(CHAR *sUserName,  CHAR *sPassword);
    E_ERR_T SerialComLoopBackTest(E_SER_COM comport_index);
    //unsigned short SerialTest(char *acPortName);    //040611 Yu Wei
    unsigned short MemoryTest();            //040611 Yu Wei

  //member variables
  private:
    unsigned short m_unListenPort;  //listen port

    int m_nClientOK;  //FALSE=client link down
                      // TRUE=client link ok

    //unsigned short m_unTablePoll[MMM_TABLE_POLL_SIZE];  //polling table

    //temporary copy of polling table
    //unsigned short m_unTablePollTemp[MMM_TABLE_POLL_SIZE];

    //buffer to send Modbus reply
    unsigned char m_ucSendBuffer[MMM_SEND_BUFFER_SIZE];
};


//MMM global variables
extern tMMMParam      g_MMMParam;      //MMM parameters

extern CMMM          *g_pMMM;      //pointer to MMM object
extern pthread_t      g_nMMMMainTaskID;  //main task ID




#endif

