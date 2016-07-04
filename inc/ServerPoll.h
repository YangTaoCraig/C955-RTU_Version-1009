/************************************************************
*                              *
*  Project:    C830 RTU                  *
*  Module:    SPM                      *
*  File :      ServerPoll.h                *
*  Author:    Yu Wei                    *
*                              *
*  Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************

**************************************************************

DESCRIPTION

  This file implements the Server Polling module.

*************************************************************/


#ifndef  _SERVERPOLL_H
#define  _SERVERPOLL_H

#include "Common.h"

#define NO_WAIT     0x00000000
#define WAIT_FOREVER   0xFFFFFFFF

//Modbus constants
#define MODBUS_MIN_CMD_SIZE      8    //minimum Modbus cmd size
#define MODBUS_MAX_CMD_SIZE      264    //maximum Modbus cmd size
#define MODBUS_FNCODE4_CMD_SIZE    8    //Modbus fn code 4 cmd size
//#define MODBUS_SEND_REPLY_TIMEOUT  5000  //send reply timeout in ms
#define MODBUS_EXCEPTION1      1    //error in received function code
#define MODBUS_EXCEPTION2      2    //error in received address
#define MODBUS_EXCEPTION3      3    //error in received data value


//ServerPoll constants
#define SERVERPOLL_TABLE_POLL_SIZE  64*1024  //polling table size in word

//send buffer size in byte, add 5 bytes for Modbus fn code 4 reply fields:
// slave address(1 byte), fn code(1 byte), no. of bytes read(1 byte) and CRC(2 bytes)
// add 3 bytes to round up to 8
#define SERVERPOLL_SEND_BUFFER_SIZE  (SERVERPOLL_TABLE_POLL_SIZE*2 + 8)

#define SERVERPOLL_SWC_COPY_MAX_TRY  5    //maximum no. of attempts to copy SWCs' tables

#define SPM_MAIN_WATCHDOG_TIMEOUT  20000  //Main task watchdog timeout
#define SPM_COPY_WATCHDOG_TIMEOUT  10000  //Copy task watchdog timeout

//ServerPoll parameters
typedef struct
{
  //ServerPoll main task parameters
  char acTaskNameMain[32];
  int nPriorityMain;  // Main task priority
  int nStackSizeMain; // Main task stack size

  //ServerPoll copy task parameters
  char acTaskNameCopy[32];
  int nPriorityCopy;  // Copy task priority
  int nStackSizeCopy; // Copy task stack size

  //Receive timeout in millisecond
  int nRecvTimeout;

  //listen ip
  char acListenIP[16];

  //Modbus slave address
  unsigned char ucModbusSlaveAddress;

  //polling table start and end word address, inclusive
  unsigned short unTablePollStart;
  unsigned short unTablePollEnd;
} tServerPollParam;


//ServerPoll task functions
int ServerPollTaskSpawn(void);
int ServerPollTaskSpawnOne(int);

void StopAllServerPollTask(void );

void *ServerPoll1MainTask(void *arg);
void *ServerPoll1CopyTask(void *arg);

void *ServerPoll2MainTask(void *arg);
void *ServerPoll2CopyTask(void *arg);

void *ServerPoll3MainTask(void *arg);
void *ServerPoll3CopyTask(void *arg);

void *ServerPoll4MainTask(void *arg);
void *ServerPoll4CopyTask(void *arg);

void *ServerPoll5MainTask(void *arg);
void *ServerPoll5CopyTask(void *arg);


//ServerPoll Modbus functions
void ModbusSendErrorReply(int nFd, unsigned char ucSlaveAddress, unsigned char ucFnCode,
              unsigned char ucExceptionCode);



//ServerPoll class
class CServerPoll
{
  //member functions
  public:
    CServerPoll(unsigned short unListenPort, int nObjectID);
    ~CServerPoll();
    void MainTask();
    void CopySWCsTables();

//    #ifdef CFG_ENABLE_SPA
//    int Get_SocketListen();
//    int Get_SocketClient();
//    #endif // CFG_ENABLE_SPA
//
//    #ifdef CFG_ENABLE_MODULE_TEST
//    E_ERR_T LogSrvPollValues(int nNum, const CHAR *pfilename);  //for test.
//    BOOL_T m_isLogEnable;
//    CHAR   *m_pLogFilename;
//    #endif // CFG_ENABLE_MODULE_TEST

  private:
    int SPMModbusCmdCheck(int nFd, unsigned char *pucCmd, int nCmdSize);  //040510 Yu Wei
    void ClientHandler(int nSockClient);
    E_ERR_T copySWCCommunicationLinkStatusToRTUStatusTable();

  //member variables
  public:
    pthread_t    m_nServerPollMainTaskID;  //ServerPoll main task ID.
    pthread_t    m_nServerPollCopyTaskID;  //ServerPoll copy task ID.
    bool         m_bTaskWorking;        //The flag for task working.
    pthread_t    m_threadID;
    CHAR         m_threadName[50];


    unsigned short m_unTablePoll[SERVERPOLL_TABLE_POLL_SIZE];  //polling table

    int    m_tWatchdogIDMain;  //Watchdog ID for main task.
    int    m_tWatchdogIDCopy;  //Watchdog ID for copy task.

    // Not use
    //timer_t         serv_poll_main_timer_id;


    int    m_nCurrentStateFlag;    //Current SPM working state.  //040322 Yu Wei
    CHAR m_acListenIP[16];
    UINT16 m_unListenPort;    //listen port ID.
    CHAR m_acClientIP[16];
    UINT16 m_unClientPort;

  private:
    // Watchdog timer ID for the copy task
    timer_t         serv_poll_copy_timer_id;

    int m_nSockListen;        //listen socket
    int m_nSockClient;        //client socket

    int    m_nObjectID;        //Serverpoll object ID (0~4).

    int m_nClientOK;  //FALSE=no client connected or client error
                      // TRUE=client connected and client ok


    //temporary copy of polling table
    unsigned short m_unTablePollTemp[SERVERPOLL_TABLE_POLL_SIZE];

//    sem_t *m_semTablePollTemp;  //temporary polling table semaphore
    pthread_mutex_t m_semTablePollTemp;

    //buffer to send Modbus reply
    unsigned char m_ucSendBuffer[SERVERPOLL_SEND_BUFFER_SIZE];

    //no. of failed attempts to copy SWC table, add 1 at the end for RTU status table
    int m_nSWCCopyFailed[SWC_MAX_NUMBER + 1];


};

extern CServerPoll    *g_apServerPoll[SERVER_POLLING_SOCKET_MAX];



#endif

