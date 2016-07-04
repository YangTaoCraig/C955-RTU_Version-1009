/************************************************************
*                              *
*	Project:    C830 RTU	                *
*	Module:  	Redundancy Management Module (RMM)      *
*	File :      RMM_RTULink.h	              *
*	Author:  	Yu Wei	                  *
*                              *
*	Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************

**************************************************************

DESCRIPTION

	This is header file for all RMM_RTULink.cpp file. It defines
	constants, declares function and class member.

*************************************************************
Modiffication History
---------------------

Version: SWC D.1.2.1
	01	15 July 2003, Yu Wei
  	Start to write.

Version: RMM D.1.3.0
	01	01 August 2003, Yu Wei,
  	All SWC use same class except Clock.

	02	07 August 2003, Yu Wei,
  	Added primary RTU sending polling table to standby RTU.

Version: RMM D.1.3.2
	01	07 November 2003, Yu Wei,
  	Added m_bChangeToStandbyFlag to indicate that RTU changes to standby and
  	sends status table to another RTU.

	02	18 November 2003, Yu Wei
  	Added watchdog function.
  	Added g_tRMMOtherLinkWatchdogID, RMM_OTHER_LINK_WATCHDOG_TIMEOUT,
  	g_tRMMConnect1WatchdogID, RMM_CONNECT1_WATCHDOG_TIMEOUT,
  	g_tRMMConnect2WatchdogID, RMM_CONNECT2_WATCHDOG_TIMEOUT,

	03	27 November 2003, Yu Wei
  	Added m_anLinkDownTimeout[2] timeout value for declaring link down.

Version: RMM D.1.3.3
	01	30 December 2003, Yu Wei,
  	m_anFailureTimeoutStandby and m_anFailureTimeout are not used.

	02	26 January 2004, Yu Wei,
  	Delete CRTULink() parameter, using global variable.

Version: RMM D.1.3.4
	01	02 March 2004, Yu Wei
  	Added LogLinkValues() for module test.
  	Refer to PR:OASYS-PMS 0035.

Version: RMM D.1.3.5
	01	10 March 2004, Yu Wei
  	Reduce complexity in GetResponseFromOtherRTU() .
  	Added ExceptionProcess() and NormalProcess().
  	Refer to PR:OASYS-PMS 0044.

	02	23 March 2004, Yu Wei
  	Added GetNextSWCTableID().
  	Refer to PR:OASYS-PMS 0143.

  	Added ClearVariables() to clear all variables when new object.
  	Refer to PR:OASYS-PMS 0054.

	03	25 March 2004, Yu Wei
  	Deleted m_bChangeToStandbyFlag. When standby receive
    "Change to primary" command will change to Switching_StoP or
  	Primary, the LinkSlave will not work in these two state.
    (Improvement)

	04	08 April 2004, Yu Wei
  	Added m_anPollingIntervalType[2] to fixed bug that the polling
  	time between RTU is too slow.
  	Added RTU_LINK1_POLLING_TO and RTU_LINK2_POLLING_TO.
  	Deleted UPDATE_POLLING_TABLE. Not used.
  	Refer to PR:OASYS-PMS 0155.

Version: RMM D.1.3.6
	01	20 April 2004, Yu Wei
  	Added 'Other RTU command code' defination.
  	Added WriteCommandProcess() to reduce LinkSlave()'s complexity.
  	Refer to PR:OASYS-PMS 0163.

	02	26 April 2004, Yu Wei
  	Added CHECK_RTU_LINK2, m_anCheckLink2Type, m_anCheckLink2Timeout.
  	Added m_bSendFlag, m_nLANLinkID, m_nCurrentStateFlag.
  	Added Main()and SetStateFlag().
  	Refer to PR:OASYS-PMS 0167.

	03	20 May 2004, Yu Wei
  	Added "CMM_Timer.h" to use relative timer.
  	Change m_atTimeOut to type "tTimerValue".
  	Refer to PR:OASYS-PMS 0201.

**************************************************************/
#ifndef  _RMM_RTU_LINK_H
#define  _RMM_RTU_LINK_H



#include "Common.h"
#include "CMM_Modbus.h"
#include "CMM_Timer.h"    //040520 Yu Wei
//#include "SWC.h"
//Timeout type
#define RTU_LINK1_POLLING    0
#define RTU_LINK2_POLLING    1
#define RTU_LINK1_POLLING_TO  2    //040408 Yu Wei
#define RTU_LINK2_POLLING_TO  3    //040408 Yu Wei
#define RTU_LINK1_FAILURE    4    //040408 Yu Wei
#define RTU_LINK2_FAILURE    5    //040408 Yu Wei
#define CHECK_RTU_LINK2      6    //040426 Yu Wei
//#define UPDATE_POLLING_TABLE	6	  //040408 Yu Wei
#define RTU_SERVER_LINK1_TIMEOUT  7  // 20100624 BC
#define RTU_SERVER_LINK2_TIMEOUT  8  // 20100624 BC

#define RTU_LINK_MAX_TIMEOUT_TYPE  9  //040426 Yu Wei


//For polling command ID
#define POLLING_NO_COMMAND    0  //No polling.
#define POLLING_READ_TABLE    1  //Poll other RTU to read RTU status table.
#define POLLING_WRITE_TABLE    2  //Write local RTU status table to other RTU.
#define POLLING_WRITE_SWC_TABLE  3  //Write SWC polling table to other RTU.
#define WRITE_COMMAND      4  //Send a command to other RTU.

//Send command to other RTU
#define SEND_STATE_CHANGED      0x01
#define SEND_CHECK_SWC_LINK      0x02
#define WRITE_COMMAND_ADDRESS    0xFFFF
#define WRITE_RTU_STATUS_ADDRESS  0x0000
#define READ_COMMAND_ADDRESS    0xFF00
//The maxmimum buffer number of command sent to RTU
#define MAX_COMMAND_BUFFER  25

//The results of sending command to other RTU
#define SEND_COMMPLETE_NULL    0  //No command to send.
#define SEND_COMMPLETE_START  1  //Start to send command.
#define SEND_COMMPLETE_OK    2  //Send command OK.
#define SEND_COMMPLETE_ERROR  3  //Send command error.

#define MODBUS_WRITE_CMD_SIZE  11

//Watchdog timeout 10 seconds.
#define RMM_OTHER_LINK_WATCHDOG_TIMEOUT    10000
//Watchdog timeout 10 times SOCKET_CONNECT_TIMEOUT.
#define RMM_CONNECT1_WATCHDOG_TIMEOUT    10 * SOCKET_CONNECT_TIMEOUT
//Watchdog timeout 10 times SOCKET_CONNECT_TIMEOUT.
#define RMM_CONNECT2_WATCHDOG_TIMEOUT    10 * SOCKET_CONNECT_TIMEOUT

//Other RTU command code	//040420 Yu Wei
#define CMD_START_SWITCH  0x01
#define CMD_COMFIRM_SWITCH  0x02

//Command structure
struct tCommandList
{
  //Start point of command list.
  int    nStartPoint;
  //End point of command list. This position is first blank buffer.
  int    nEndPoint;
  unsigned short  aunCommand[MAX_COMMAND_BUFFER];  //Command list buffer.
};


class CRTULink
{

private:


  char m_acReadRTUStatus[MODBUS_READ_CMD_SIZE];      //Buffer for read status table command that send to other RTU.

  //Command for writing RTU status table to other RTU.
  char m_acWriteRTUStatus[MODBUS_MAX_CMD_SIZE];
  char m_acWriteSWCPollingTable[MODBUS_MAX_CMD_SIZE];    //Buffer for writing RTU status table to other RTU.
  char m_acReplyRTUStatus[MODBUS_MAX_CMD_SIZE];      //Buffer for replying read status table.
  char m_acWriteCommand[MODBUS_WRITE_CMD_SIZE];      //Buffer for write command.

  unsigned short m_unStatusTableLen;      //Status table length in word.
  int  m_nTableBytes;              //Status table length in byte.

  int m_anRTULinkFD[2];  //Connection file descriptor.

  int m_anPollingInterval[2];      //Polling rate between RTU.
  int m_anFailureTimeout[2];      //Timeout before declaring the LAN failure.
  //int m_anFailureTimeoutStandby[2];  //Timeout before declaring the LAN failure for standby RTU.
  int m_anRetry[2];          //Retry times for send command to other RTU.

  char m_acIPAddress[2][20];  //Other RTU LAN IP address (0 -- LAN2, 1 -- LAN1)
  int m_anPort[2];      //LAN port ID.
  // True when ping is responding. Otherwise false
  bool m_Lan2LinkDown;
  bool m_Lan1LinkDown;
  bool m_abConnectFlag[2];  //Flags for LAN link connection.
                //true -- connected, false -- not connected.

  int m_anPollingTimeoutType[2];  //Timeout type for polling timeout.  //040408 Yu Wei.

  int m_anPollingIntervalType[2];  //Timeout type for polling.  //040408 Yu Wei




  struct tTimerValue m_atTimeOut[RTU_LINK_MAX_TIMEOUT_TYPE];  //Buffer for timer counter.//040520 Yu Wei

  tCommandList m_tWriteCommand;  //Store command that will send to other RTU.

  int m_nPollingCommand;    //For polling command ID
  int m_nRetry;        //Record retry times of sending command to other RTU.
  int m_nSWCPollingTableID;  //The ID of SWC polling table.
  int m_nSWCPollingTableTypeID;  //The ID of SWC polling table type.
                  //It will be FAST_TABLE, SLOW_TABLE, TIMER_TABLE.
  #ifdef ENABLE_GET_NEXT_SWC_TABLE_UPDATE
  int m_nWriteTableSWCIndex; // The next SWC index to read SWC table
  #endif // ENABLE_GET_NEXT_SWC_TABLE_UPDATE
  unsigned short  m_nMultiNodeSWCPollingNodeID; //The ID of MultiNOde SWC Node

  int m_nLANLinkID;      //Record actived LAN link.  //040426 Yu Wei

  int m_nCurrentStateFlag;  //Record current state for this class.  //040426 Yu Wei

  void ClearVariables(void);  //040323 Yu Wei
public:

  CRTULink(void);
  ~CRTULink(void);

  /*---------------------------------------------------------------------------
    When the flag is enable (set true), the system will start to send polling
    command to other RTU. The commands include but not limited to
    POLLING_READ_TABLE, POLLING_WRITE_TABLE, and POLLING_WRITE_SWC_TABLE
    When LAN connection fail, the flag will be disable (set false).
  ---------------------------------------------------------------------------*/
  bool m_bSendCmdToRtuFlag;


  int m_anCheckLink2Type;      //Timeout type for check link2 status.  //040426 Yu Wei
  int m_anServerLinkTimeout;    //Timeout value for check link2 status. //040426 Yu Wei

  int  m_nWriteCommandResult;
  //bool m_bChangeToStandbyFlag;  //To indicate that RTU changes to standby and	//040325 Yu Wei
                  //sends status table to another RTU
  int m_anFailureTimeoutType[2];  //Timeout type for link failure.
  int m_anLinkDownTimeout[2];    //Timeout value for declaring link down.
  int m_anMasterLinkDownMonitorTimeout[2];	//20150715 Su 2-4
  int m_anSlaveLinkDownMonitorTimeout[2];	//20150715 Su 2-4
  //int m_anRtuServerTimeoutType[2];
  char m_acRecvOtherRTUBuffer[MODBUS_MAX_CMD_SIZE ];    //Buffer for receive data.
  void Main(void);        //040426 Yu Wei
  void LinkMaster(void);
  int GetResponseFromOtherRTU(int);
  void ExceptionProcess(void);    //040310 Yu Wei
  int NormalProcess(int, int);    //040310 Yu Wei
  void SendCommandToOtherRTU(int);
  void SendOrClose(int, char *, int);
  void WriteSWCTable(int);

  //void ModifiedWriteSWCTable(int );//2008
  //void ModifiedGetNextSWCTableID(void);//2008


  void GetNextSWCTableID(void);  //040323 Yu Wei
  //void GetNextMultiNodeSWCTableID(void);   //2008

  void LinkSlave(void);
  void WriteCommandProcess(int);  //040420 Yu Wei

  int ConnectOtherRTU(int);
  void ConnectLink0(void);
  void ConnectLink1(void);
  void CloseConnectOtherRTU(int);

  void ActivateTimeOut(int);
  bool CheckTimeOut(int, unsigned int);

  //void WriteCommandProcess(void);
  void SetWriteCommand(unsigned short, unsigned short);
  void DeleteWriteCommand(void);
  void SetStateFlag(void);  //040426 Yu Wei


  void StandByGetData(int );
  //void LinkSlave2();
  //void LinkSlave1();
  void LinkSlave(UINT8 lanlinkidx);
  BOOL_T GetConnectStatus(UINT8 ncLanID);
  E_ERR_T GetLinkDownStatus(UINT8 ncLanID, INT8 *prval);
  E_ERR_T GetLinkFileDesc(UINT8 ncLanID, INT32 *prval);
  E_ERR_T PrnDebugInfo(CHAR *outputbuffer, INT32 outputbuffsz);

//  #ifdef ModuleTest
//  void LogLinkValues();    //For test.
//  #endif // ModuleTest

};

int OtherRTULinkInit(void);
//void OtherRTULinkTask(void);
void *OtherRTULinkTask(void *arg);
void OtherRTULinkStop(void);
//void OtherRTUConnectionTask1(void);
//void OtherRTUConnectionTask2(void);
void *OtherRTUConnectionTask1(void *arg);
void *OtherRTUConnectionTask2(void *arg);

void OtherRTUServerSocketTask1(void);
void OtherRTUServerSocketTask2(void);
void *OtherRTUServerSocketTask1(void *arg);
void *OtherRTUServerSocketTask2(void *arg);

void *StandByReadSocketLink1(void *arg);
void *StandByReadSocketLink2(void *arg);

pthread_t* RMMLinkWatchDogInit(pthread_t tid[2]);
pthread_t* RMMConnection1WatchDogInit(pthread_t tid[2]);
pthread_t* RMMConnection2WatchDogInit(pthread_t tid[2]);
//extern void UpdateRTUStatus(unsigned short, int);
extern CRTULink *g_pCRTULink;
extern bool g_bRMMLinkTaskWorking;        // The flag for OtherRTULinkTask working/
                                          // stopping.
extern pthread_t g_anOtherRTULinkSocketTaskID[2];

extern char copy_RecvOtherRTUBuffer[MODBUS_MAX_CMD_SIZE];// For copy the the Recvother RTU Buffer

extern bool m_PrimarycloseFdFlag; // Yang Tao 20150402 Add in this flag to indciate
extern bool m_abOtherLinkStatusSave[2]; //
extern int m_PrimarycloseFdType;//
extern int m_PrimarycloseFdFlagTimeout;//
extern bool Testflag;

#endif /* _RMM_RTU_LINK_H */

