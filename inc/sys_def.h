#ifndef _SYS_DEF_H_
#define _SYS_DEF_H_

// For the usage of printing data block
#define SYS_DEBUG_MSG_BUFF_SZ   1024
#define SYS_HEADERLINE_BUFF_SZ  300
#define SYS_CONFIGFILENAME_SZ   100

#define SYS_UNIX_EPOCH_START_1900  1900
typedef enum e_time_t{
  E_SYS_TIME_Local,
  E_SYS_TIME_UTC,
  E_SYS_TIME_EndOfList
} E_SYS_TIME;

typedef enum e_ipaddr_t{
  E_SYS_IPADDR_Reserved_0,
  E_SYS_IPADDR_IP,
  E_SYS_IPADDR_SubnetMask,
  E_SYS_IPADDR_EndOfList
} E_SYS_IPADDR;

// The label must be in sequence as of the sys_thrd_attr_lut from sys_lut.h
typedef enum e_thread_label{
  E_SYS_TLB_Reserved_0,
  //E_SYS_TLB_SYS_Scheduler,
 // E_SYS_TLB_FLG_SystemLog,
//  #ifdef CFG_ENABLE_SPA
//  E_SYS_TLB_SerialReceive,
//  E_SYS_TLB_SerialTransmit,
//  E_SYS_TLB_SpaReceiveCmd,
//  #endif // CFG_ENABLE_SPA
  E_SYS_TLB_NTPClient,
  E_SYS_TLB_NTPClient_LinkManagement,

  //E_SYS_TLB_CmmWatchdogMon,
  E_SYS_TLB_CmmWatchdog,
  E_SYS_TLB_CmmTimer,
  E_SYS_TLB_CmmLog,

  E_SYS_TLB_SpmMain_1,
  E_SYS_TLB_SpmMain_2,
  E_SYS_TLB_SpmMain_3,
  E_SYS_TLB_SpmMain_4,
  E_SYS_TLB_SpmMain_5,
  E_SYS_TLB_SpmCopy_1,
  E_SYS_TLB_SpmCopy_2,
  E_SYS_TLB_SpmCopy_3,
  E_SYS_TLB_SpmCopy_4,
  E_SYS_TLB_SpmCopy_5,

  E_SYS_TLB_SWC_NTPClientComm,
  E_SYS_TLB_RMM_OthRTULinkSock1,
  E_SYS_TLB_RMM_OthRTULinkSock2,
  E_SYS_TLB_RMM_OthRTULinkTask,
  E_SYS_TLB_RMM_OthRTUConnTask1,
  E_SYS_TLB_RMM_OthRTUConnTask2,
  E_SYS_TLB_RMM_StdbyReadSock1,
  E_SYS_TLB_RMM_StdbyReadSock2,
  E_SYS_TLB_RMM_ServerSocketTask,
  E_SYS_TLB_RMM_Task,
  E_SYS_TLB_MMM_Task,

  /*--------------------------------------------------------------------------
    CAUTION: The following section is reserved to SWC tasks only. System will
             use offset from here. Other task can be added above this section
  ----------------------------------------------------------------------------*/
  E_SYS_TLB_SWC_MgmtTask,
  E_SYS_TLB_SWC_ServerTask,
  E_SYS_TLB_EndOfList
} E_SYS_TLB;

#define SYS_THRD_NAME_LEN 50
struct thread_attribute_st{
  E_SYS_TLB thrd_label;
  CHAR    thrd_name[SYS_THRD_NAME_LEN];
  INT32   priority;
  UINT8   inherit_schd;
  INT32   policy;
  size_t  stack_sz;
  VOID    *pstack_addr;
  VOID*   (*start_routine)(VOID *);
  //E_ERR_T   (*start_routine)(VOID *);
  VOID     *arg;
};

struct thrd_ctrl_st{
  E_SYS_TLB      thrd_label;
  CHAR           thrd_name[SYS_THRD_NAME_LEN];
  pthread_attr_t attr;
  pthread_t      thrd_id;
  VOID*         (*start_routine)(VOID *);
  VOID*          arg;
};


// channel label
typedef enum e_channel_label{
  E_SYS_CHL_Reserved_0,
  E_SYS_CHL_SerRxCmdToSPAParseCmd,
  E_SYS_CHL_EndOfList
} E_SYS_CHL;
// channel control block

/* Channel pulse range from 0 - 127 */
/* Warning: Original RTU application is uing 0-15. Be careful. Conflict will
            multiple session of triggers
*/
// TODO: Need to consolidate all timers
typedef enum sys_pulse_enum{
  E_SYS_PULSE_NTP_Polling = _PULSE_CODE_MINAVAIL + 50,
  E_SYS_PULSE_NTPC_LinkMgmt,
  E_SYS_PULSE_NTPC_RxCmdTxResp,
  E_SYS_PULSE_RMM_NotationMgmt,
  E_SYS_PULSE_EndOfList = _PULSE_CODE_MAXAVAIL
}E_SYS_PULSE;
/*
struct chnl_ctrl_st{
  E_SYS_CHL ch_label;
  INT32 chid;
  struct _msg_info info;
};
*/
typedef union sys_pulse_msg_u{
  struct _pulse pulse;
  /* your other message structures would go
     here too */
} SYS_CH_MSG_T;

// message queue
typedef enum e_msqueue_label{
  E_SYS_MSQL_Reserved_0,
  E_SYS_MSQL_FLG_SystemLogReceiving,
//  #ifdef CFG_ENABLE_SPA
//  E_SYS_MSQL_SerCmdToSPAParseCmd,
//  E_SYS_MSQL_SPAResultToSerOutput,
//  #endif // CFG_ENABLE_SPA
  E_SYS_MSQL_NTPC_TxExternalSystemMsg,
  E_SYS_MSQL_EndOfList
} E_SYS_MSQL;

// internal message
typedef enum e_sys_module_comm_protol_label{
  E_SYS_MCP_Reserved_0,
  E_SYS_MCP_SWCToNTPClient,
  E_SYS_MCP_NTPClientToSWC,
  E_SYS_MCP_EndOfList
} E_SYS_MCP;

#define SYS_MCP_MAX_MSG_SZ   80
#define SYS_MCP_HEADER       "MCPr\0"
typedef struct ntp_messageq_st{
  CHAR header[4];
  E_SYS_MCP msgtype;
  UINT32 msgsz;
  UINT8 msgbuff[SYS_MCP_MAX_MSG_SZ];
} SYS_MSGQ_MCPR;


typedef enum e_process_label{
  E_SYS_PROC_devc_ser8250_index0,
  E_SYS_PROC_devc_ser8250_index1,
  E_SYS_PROC_EndOfList
} E_SYS_PROC;

#ifdef NOT_USED
typedef enum e_mutex_label{
  E_SYS_MTX_SER_SendWithReply,
  E_SYS_MTX_LAN_SendWithReply,
  E_SYS_MTX_EndOfList
} E_SYS_MTX;
#endif // NOT_USED

// Support priority range from 1-31 (MQ_PRIO_MAX - 1)
#define SYS_MSQ_PRIOR_NORMAL 10

#define SYS_MSQ_PATHNAME_SZ 100
struct msq_lut_st{
  E_SYS_MSQL msq_label;
  CHAR msq_pathname[SYS_MSQ_PATHNAME_SZ];
  UINT32 oflag;
  UINT32 mode;
};

struct msq_ctrl_st{
  struct msq_lut_st *pmsq_desc;
  INT32             msq_id;
};

struct process_ctrl_st{
  CHAR appl_name[50];
  INT32  pid;
};

struct mutex_ctrl_st{
  CHAR mutex_name[50];
  pthread_mutex_t mtx;
  pthread_mutexattr_t attr;
};

// RTU_LOCATION max character size
#define INM_RTU_LOCATION_SZ 4

// Maintenance terminal timeout in seconds
#define INM_RTU_MAINT_TERMINAL_MAX_TIMEOUT  ((INT32)(24*24*60*60))

//Max VDU user number
#define VDU_MAX_USER  10
#define INM_USERNAME_MAX_SZ 8
#define INM_PASSWORD_MAX_SZ 8
#define SERVER_POLLING_SOCKET_MAX  5

//VDU name structure (18 bytes)
struct tVDUName
{
  char  acVDUName[9];    //VDU user name  //040309 Yu Wei
  char  acVDUPassword[9];  //VDU password  //040309 Yu Wei
};


//RTU table structure (STATUS, POLLING, COMMAND) (4 bytes)
struct tRTUTableStructure
{
  //char    acTableName[16];      //RTU table name
  //int      nTableID;          //RTU table ID
  unsigned short  unTableStartAddress;  //Table start address
  unsigned short  unTableEndAddress;    //Table end address
};

struct lan_link
{
  char ip[20];
  unsigned short port;
};

typedef enum e_clock_protocol{
  E_CLKP_Other,
  E_CLKP_Generic,
  E_CLKP_NTP,
  E_CLKP_Modbus,
  E_CLKP_EndOfList
} E_CLKP;

struct link_protocol
{
  //int protocol; //modbus or clock
  E_CLKP protocol;
  unsigned char addr;   //modbus slave address
  unsigned char command; //read command 3 or 4
};

struct tLANParameter
{
  char    acLAN_IP[20];      //IP address
  char    acLAN_Gateway[20];    //Gateway address
  char    acLAN_Netmask[20];    //Netmask address
  unsigned short  anOtherRTULinkSocketID;  //The port number for other RTU connection.
                    //040309 Yu Wei
  //char    acLAN_Protocol[16];    //Protocol MODBUS, etc.
  char    cLAN_SlaveAddress;    //Modbus slave address
  char    cModbusReadCommand;    //Modbus read command code (0x03 or 0x04)
  struct link_protocol tPrtc; //20090629
  int      nPollingTime;      //Polling time
  int      nPollingTimeout;    //Polling timeout
  int      nPollingRetryNumber;  //Retry times number

};


//Serial port parameter structure (20 bytes)
struct tSerialPortPara
{
  //char acPortName[16];    //port name.
  unsigned short unPortID;  //port ID.
  int nBaudRate;        //9600, 19200, etc.
  //int  nStartBit;        //1=1 bit
  int nDataBit;        //5 to 8
  int nParity;        //0=none, 1=odd, 2=even
  int  nStopBit;        //1=1 bit, 2=2 bits, 3=1.5 bits
  //char  acProtocol[16];    //Protocol: MODBUS, PR_CLK.
  char  cSlaveAddress;    //Modbus slave address
  char  cModbusReadCommand;  //Modbus read command code (0x03 or 0x04)
  struct link_protocol tPrtc;//20090629

};

//SWC header configuration structure (113 bytes)
struct tSWCCFGHeaderStructure
{
  char               acName[11];          //SWC name
  unsigned char      ucID;            //SWC ID
  int                nLinkType;          //Interface link type.
  int                nLinkGroup;          //Gropu info for multidrop link.
  tSerialPortPara  tLinkPara[2];        //Serial link parameters
  struct lan_link tLanLink[2];  //20090629
  struct link_protocol tProtocol[2]; //20090629
  //int        nWireNumber;      //Wire number
  int        nStandbyPollingFrequency;  //Standby link polling frequency
  int        nFailedPollingFrequency;  //Failed link polling frequency
  int        nReceiveTimeout;      //Timeout to resend message frame if no reply from SWC
  int        nRetryNumber;        //Retry number before to declare a SWC link failure
  int        nFastPollingFrequency;    //Primary Link Fast polling frequency
  int        nSlowPollingFrequency;    //Slow polling frequency
  int        nSpuriousTime;        //For clock synchronisation only
  int        nSendTimeSynHour;      //Hour which the time synchronization message is send.
  int        nSendTimeSynMinute;      //Minute which the time synchronization message is send.
  unsigned short  unSendTimeSynStartAddr;    //Start address which the time synchronization message is send.
  unsigned short  unSendTimeSynLength;    //Length which the time synchronization message is send.

  unsigned short  unSendKeepAliveAddr;    //The address for KeepAlive signal.    //040812 Yu Wei.
  int        nKeepAliveInterval;      //The interval for sending KeepAlive signal.  //040812 Yu Wei

  unsigned short  nNotation;          //SWC Notation  //040317 Yu Wei
  unsigned short  nSocketID;          //Socket ID for server direct Read/Write Control //040309 Yu Wei
  int        nLANLinkTimeout;      //Timeout in milisecond before closing the connection.
  #ifdef CFG_EXTENDED_MODBUS
  BOOL_T     bExtendedModbus;      //Enable to support extended modbus protocol
  #endif // CFG_EXTENDED_MODBUS

  //040707 Yu Wei
  int        nSWCBusyTimeout;      //for delay interval in ms to resend time sync command to SWC.
  int        nExcepTimeSyncRetry;    //for number of retry counter before stopping time sync.

  int        nExcepRetry;        //for number of retry counter before declare exception failure.
  //040707 Yu Wei
  bool bPingCheck;//20090708

};

//Modbus address structure (8 bytes)
struct tModbusAddress
{
  unsigned short  unStart;    //Start address
//  unsigned short  unEnd;      //End address
  unsigned short  unLength;    //Table length in word
  unsigned short  unServerStart;  //Server table Start address
//  unsigned short  unServerEnd;  //Server table End address
  //Server table length in word including 3 bytes of UTC timestamp
  unsigned short  unServerLength;
};

//SWC polling address structure (28 bytes)
struct tSWCPollingAddress
{
  tModbusAddress  tFastPollingAddress;  //Modbus table for fast polling
  tModbusAddress  tSlowPollingAddress;  //Modbus table for slow polling
  tModbusAddress  tTimerAddress;      //Modbus table for timer polling
  int        nTimerInterval;      //The time interval in one hour for timer polling.
};

//SWC configuration structure (141 bytes)
struct tSWCCFGStructure
{
  tSWCCFGHeaderStructure  tHeader;        //SWC header configuration
  //int      nTableNumber;                  //Total table number
  //tSWCTableStructure          tTable[SWC_MAX_TABLE];  //SWC Table    //040322 Yu Wei
  tSWCPollingAddress          tPollingAddress;    //Polling information
};

/******************************************************************************
Purpose
  The data structure will hold all the parameters found from system
  configuration file.

History
  Name          Date          Remark
  ----          ----          ------
 Bryan Chong   05-May-2010   Add nTotalEnableSwc to keep track of total number
                             of SWC being enabled.

******************************************************************************/
//RTU configuration structure (3181 bytes)  //040812 Yu Wei
typedef struct tSystemConfiguration
{
  //Version number of the RTU configuration file
  unsigned short  unVersion;

  //Local time zone in seconds.  //040329 Yu Wei.
  int nLocalTimeZone;

  //Identification number of the RTU
  unsigned short  unRTUIdentification;

  //Location of the RTU
  char    acRTULocalion[5];  //040317 Yu Wei

  // CPU Maximum Temperature
  UINT16    ncpuMaxTemperature;

  // Minimum Free Memory Space
  UINT16   nminFreeMemorySpace;

  //1 -- RTU1,  2--RTU2.
  int      nRTUID;

  //RTU maintenance terminal socket ID
  unsigned short    nMaintTerminalSocketID;      //040309 Yu Wei

  //RTU maintenance terminal timeout in second
  int      nMaintTerminalTimeout;

  //Timeout in milisecond before removing command
  //int      nCommandTimeoutP;

  //from command queue if connection with SWC is not OK, the Command will be purge after timeout.
  //int      nCommandTimeoutS;

  //Timeout in milisecond before confirming the LAN down during initialization.
  int      nInitLANCheckTimeout;

  //Number of serial COM card installed (eg. Moxa PCI serial communication card)
  //int      nM45Number;
  int      nNumberOfSerialCOMCard;

  tVDUName    tVDUNameList[VDU_MAX_USER];    //List of VDU user names, max 10 accounts, each 8 bytes code
                          //and associated password, each max 8 bytes password
  int        nVDUUserNumber;          //The VDU user number that is defined in config.txt file.

  tLANParameter      tLANPara[CFG_LNC_TOTAL_ACTIVE_PORT];      //RTU-RTU LAN1,2 link Parameters

  char    acOtherRTUIPAddress[2][20];      //Other RTU IP address

  tRTUTableStructure    tRTUStatusTable;    //Definition of the RTU STATUS table
  tRTUTableStructure    tRTUPollingTable;    //Definition of the RTU POLLING table  //040512 Yu Wei
  tRTUTableStructure    tRTUComanmdTable;    //Definition of the RTU COMMAND table

  //int      nServerNotion;          //Server weightage

  unsigned short  nRTUCommandSoctetID;      //Read/Write RTU Status Table/Command Table Socket ID
                          //040309 Yu Wei

  int      nRTUPollingSocketNumber;      //Read RTU Polling Socket port number

  unsigned short  nRTUPollingSocketID[SERVER_POLLING_SOCKET_MAX];  //Read RTU Polling Socket ID
                          //040309 Yu Wei

  int      nServerCMDTimeout;          //Time out with the server in ms

  int      nSWCNumber;              //Number of external system

  int      anSWCIndex[SWC_MAX_NUMBER];      //Store SWC ID that installs in the system.

  unsigned short  anMultidorpSWCChain[SWC_MAX_NUMBER];  //Store multidrop link SWC information.
                              //The value is another multidrop link SWC ID.
                              //i.e. PSD1 (ID=8) and PSD2 (ID=9) is multidrop link,
                              // so anMultidorpSWCChain[8] = 9 and
                              // anMultidorpSWCChain[9] = 8.

  char    acSWCAddress[SWC_MAX_NUMBER];    //SWC address
  int     nSwcEnabled[SWC_MAX_NUMBER];      //configured or not //20090626 tong
  int     nTotalEnableSwc;                 // Total number of SWC enabled.
  int     nFirstSwcEnabled;
  tSWCCFGStructure tSWC[SWC_MAX_NUMBER];      //SWC parameter

  int      nGreacTime;        //For clock interface for ms field. //040525 Yu Wei
}SYS_CONFIGURATION_T;


//Global variables
typedef struct tGlobalStatus
{
  int    nRTUStateFlag;                  //RTU state flag;
  unsigned short unRTURMTLinkStatus;            //RMT link status.  //040419 Yu Wei
  unsigned short unRTUServerLinkStatus;          //RTU server link status.
  bool abServerLinkStatusI[SERVER_POLLING_SOCKET_MAX];  //Server socket i link status. (i=1 ~ 5)
  bool abOtherLinkStatus[2];                //Link status between RTUs on LAN2 and LAN1.
  bool bCFGFileDownloadRequired;              //config.txt file downloading required flag.
  bool bCFCardError;
} SYS_GLOBALSTATUS_T;



typedef struct system_control_block_st{

  CHAR headerline[SYS_HEADERLINE_BUFF_SZ];
  CHAR config_filename[SYS_CONFIGFILENAME_SZ];

  // Thread control block, each SWC uses 2 threads
  struct thrd_ctrl_st thrd_ctrl[
    E_SYS_TLB_EndOfList + (CFG_NUM_OF_THREADS_PER_SWC*CFG_TOTAL_NUM_OF_SWC)];
  struct msq_ctrl_st  msq_ctrl[E_SYS_MSQL_EndOfList];
  //struct chnl_ctrl_st chnl_ctrl[E_SYS_CHL_EndOfList];

  // last error buffer for SPA
  struct system_error_st{
    E_ERR_T lastErr;
    CHAR    prnErrBuff[ERR_STR_SZ];
  }err;

  struct process_ctrl_st proc_ctrl[E_SYS_PROC_EndOfList];
  //struct mutex_ctrl_st mutex_ctrl[E_SYS_MTX_EndOfList];

}SYS_CB_T;


#endif //_SYS_DEF_H_
