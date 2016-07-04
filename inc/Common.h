/************************************************************
*                              *
*  Project:    C830 RTU                  *
*  Module:    CMM                      *
*  File :      Common.h                  *
*  Author:    Yu Wei                    *
*                              *
*  Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************

**************************************************************

DESCRIPTION

  This is header file. It defines global constants and structures.

*************************************************************/

#ifndef  _COMMON_H
#define  _COMMON_H







#define ERROR   -1
#define OK    1

//#define FALSE -1
#define TRUE   1


//#define ModuleTest

//Software version
#define VERSION_NUMBER  0x1010

//For mudbus command relpy
#define ERROR_EXCEPTION  0xF888

//System Clock
#define SYS_CLOCK_RATE    1000    //system clock rate, must sync with CLOCK_RES
#define SYS_CLOCK_RES     1000000    //system clock resolution in nanosecond,
                    // = 1,000,000,000 / CLOCK_RATE

#define SYS_CLOCK_SECOND  SYS_CLOCK_RATE      //Task delay time = 1 second.
#define SYS_CLOCK_MSECOND  SYS_CLOCK_RATE/1000    //Task delay time = 1 milli-second.

//Socket connect time in seconds
#define SOCKET_CONNECT_TIMEOUT  SYS_CLOCK_SECOND  //Set 1 second.


/* Read config file result */
#define CFGFILE_OK        1
//#define CFGFILE_NOFILE      2
//#define CFGFILE_ERROR_VERSION  3
#define CFGFILE_ERROR_RTU    4
#define CFGFILE_ERROR_RTU_TABLE  5
#define CFGFILE_ERROR_SERVER  6
#define CFGFILE_ERROR_SWC    7
#define CFGFILE_ERROR_NTP    8

//ServerPoll constants
#define SERVERPOLL_TABLE_POLL_SIZE  64*1024  //polling table size in word

/* Log file type */
//#define ERROR_LOG  0
//#define INFO_LOG  1


/* State ID*/
#define  STATEFLAG_NULL            0  //NULL state.
#define  STATEFLAG_PRIMARY         1  //Primary state.
#define  STATEFLAG_STANDBY         2  //Standby state.
#define  STATEFLAG_INITIALIZATION  3  //Initialization state.
#define  STATEFLAG_SWITCHING_PTOS  4  //Switching state from primary to standby.
#define  STATEFLAG_SWITCHING_STOP  5  //Switching state from standby to primary.
#define  STATEFLAG_HARDWARETEST    6  //Hardware test state.
#define  STATEFLAG_SYSTEM_RESET    7  //Syaytem reset state.
                    //040322 Yu Wei

/* SWC index */
#define SWC_CLOCK_INDEX    19    //Clock index ID is always 19.

//SWC Link type
#define  SWC_LINK_RS_485    0          //RS 485 interface
#define  SWC_LINK_RS_422    SWC_LINK_RS_485    //RS 422 interface
#define  SWC_LINK_RS_232    SWC_LINK_RS_485    //RS 232 interface
//#define  SWC_LINK_RS_485MXV  1          //RS 485 interface multidrop link
//#define  SWC_LINK_RS_422M  SWC_LINK_RS_485MXV  //RS 422 interface multidrop link

#define   SWC_LINK_LAN    2          //LAN link, SWC_LINK_RS_LAN

#define SWC_LINK_RS_485M  3

//SWC Table ID    //040317 Yu Wei
#define SWC_TABLE_DI     0  //0 for DI table.
#define SWC_TABLE_DI8    1  //1 for DI8 table.
#define SWC_TABLE_AI_MI  2  //2 for AI and MI (poll by slow polling) table.
#define SWC_TABLE_MI     3  //3 for MI (poll by timer polling) table.
#define SWC_TOTAL_TABLE  4  //Total number of table ID

/* Max server polling socket port number */
//#define SERVER_POLLING_SOCKET_MAX  5

//Redundancy Management Module MessageQ ID
//for structure tRMM_MSG->nMessageID
#define SWC_LINK_STATUS_CHANGE         0
#define SWC_MESSAGE_REPLY_LINK_STOP    1
#define SWC_MESSAGE_REPLY_LINK_CHECK   2
#define SERVER_POLLING_STATUS_CHANGE   3
#define MMM_STATE_CHANGE               4
#define TASK_WATCHDOG_TIMEOUT          5

//SWC Module MessageQ ID
//for structure tSWCCommand_MSG->nMessageID
#define SWC_LINK_CHECK_STANDBY_RTU    0
#define SWC_STOP_LINK_PRIMARY_RTU     1
#define SWC_START_LINK_PRIMARY_RTU    2
#define SWC_POLLING_ENABLE            3
#define SWC_POLLING_INHIBIT           4


//RTU link status
#define LINK_FAULTY         0x0000
#define SERVER_LINK_OK      0x0080
#define RMT_CONNECTED       0x0400    //040419 Yu Wei
#define OTHER_RTU_LINK1_OK  0x0100
#define OTHER_RTU_LINK2_OK  0x0200

#define RTU_LAN_PRO_PARA  "MODBUS,0x01,0x04"

//Timeout for server and other RTU link during initialization
//#define INIT_TIMEOUT_SERVER_LINK  10000  //10 seconds
//#define INIT_TIMEOUT_OTHER_RTU_LINK  10000  //10 seconds

//Constants define for task priority and sleep time (ms).

//Watchdog task
//#define CMM_WATCHDOG_TASK_PRI    15 //105    //highest priority
//#define CMM_WATCHDOG_SLEEP       SYS_CLOCK_SECOND    //1 second

//Log task  //040525 Yu Wei
//#define CMM_LOG_TASK_PRI         10//195   // lowest priority
#define CMM_LOG_SLEEP        SYS_CLOCK_SECOND    //1 second

//RMM
#define RMM_MAIN_TASK_PRI       11//12//12 //140
#define RMM_MAIN_TASK_SLEEP      5 * SYS_CLOCK_MSECOND  //5 milliseconds.
#define RMM_SERVER_TASK_PRI      12//13//13//135

#define RMM_RTU_LINK_SERVER_TASK_PRI  12 //11//11//132
#define RMM_RTU_LINK_CONNECT_TASK_PRI  13 //12 //130
#define RMM_RTU_LINK_CONNECT_TASK_SLEEP  30 * SYS_CLOCK_MSECOND  //30 milliseconds.
#define RMM_RTU_LINK_TASK_PRI      18 //10//11//138
#define RMM_RTU_LINK_TASK_SLEEP      30 * SYS_CLOCK_MSECOND  //30 milliseconds.


//Server polling task
#define SPM_MAIN_TASK_PRI      11 //145
#define SPM_MAIN_TASK_SLEEP      SYS_CLOCK_SECOND
#define SPM_COPY_TASK_PRI      10//150
#define SPM_COPY_TASK_SLEEP      250 * SYS_CLOCK_MSECOND  //250 milliseconds.

//MMM
#define MMM_TASK_PRI        10//190

#define PLC1_TASK_SLEEP        40 * SYS_CLOCK_MSECOND  //30 milliseconds.
//Power1
#define POWER1_TASK_PRI        10//150
#define POWER1_TASK_SLEEP      30 * SYS_CLOCK_MSECOND  //30 milliseconds.
#define POWER1_SERVER_TASK_PRI    10//145

#define SPM_MAIN_TASK_ID_INDEX_START      60  //SPM main task ID index = 60 ~ 64.
#define SPM_COPY_TASK_ID_INDEX_START      65  //SPM main task ID index = 65 ~ 69.

#define RMM_OTHER_LINK_TASK_ID_INDEX      70
//#define RMM_LINK1_SERVER_TASK_ID_INDEX      71
//#define RMM_LINK2_SERVER_TASK_ID_INDEX      72
#define RMM_LINK1_CONNECT_TASK_ID_INDEX      73
#define RMM_LINK2_CONNECT_TASK_ID_INDEX      74
#define RMM_MAIN_TASK_ID_INDEX          75
//#define RMM_SERVER_TASK_ID_INDEX        76

//#define INM_TASK_ID_INDEX            77

#define MMM_MAIN_TASK_ID_INDEX          78

#define WATCHDOG_TASK_MAX_NUMBER        80


//MSG_Q_NAME

#define RMM_MSGQ                 "/RTU/rmm_queue"
#define SWC_MSGQ                 "/RTU/swc_queue"
#define MSG_PRI_NORMAL               10
#define MSG_PRI_URGENT               20

//Yang Tao 201503031

#define VALIDAION_BIT					 0x0008



//SWC message structure for SWC messageQ
struct tSWCCommand_MSG
{
  int nMessageID;  //Message ID;
};

//RMM message structure for RMM messageQ
//When device status is changed, the module will send this structure message.
struct tRMM_MSG
{
  int nDeviceID;        //Device ID. SWC ID or server polling socket ID.
  int nMessageID;        //Message ID.
  int nSWCType;        //0 means this is a sigle node SWC , 1 means this is a multiNode SWC
  int nNodeStatusAddress;
  unsigned short nNodeStatus;
  unsigned short unStatus;  //Device status.
};

struct tMultiDropNode      //store one node
{
  char nodeID;            //Node slave Addrress
  int nodeTimeout;        //Timeout (ms) to resend message frame if no reply from node1
  int nodeRetry;          //retry  times before to declare a node link failure.
  int nTableNumber;
  tModbusAddress    nodeTable[3]  ;      // 0 for currrent value and for server data
                        // 1 for voltage  value and for server data
                        // 2 for enenergy  value and for server data
};

struct tMultiDropSWCSture
{
  char acMDSWCname[11];        //named it in config file
  int  nNodeStausAddress;              ///* Address in Polling Table to store node status for server


  int         nLinkType;        //Interface link type.
  int        nPortNumber;
  tSerialPortPara   tLinkPara;        //Serial link parameters

  int  nTotolNode;            //totol number of node connect to this SWC
  //tMultiDropNode      atMultiDropNode[10]; //max 10 node

};


//extern tMultiDropSWCSture g_tMultiDropSwcConfig[SWC_MAX_NUMBER];

//extern struct  addrinfo *g_addinfo[2];
//extern unsigned short usByteSwap(unsigned short  arg);


#endif /*_COMMON_H*/


