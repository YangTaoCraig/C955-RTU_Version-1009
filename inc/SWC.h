/************************************************************
*                              *
*  Project:    C830 RTU                  *
*  Module:    CMM                      *
*  File :      SWC.h                    *
*  Author:    Yu Wei                    *
*                              *
*  Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************

**************************************************************

DESCRIPTION

  This is header file for SWC.cpp class defination.

*************************************************************
Modiffication History
---------------------

Version: SWC D.1.1.1
  01  21 March 2003, Yu Wei,
    Start to write.

Version: SWC D.1.2.0
  01  09 July 2003, Yu Wei
    Add comment.
    Start to add RMM. SWC will handle state flag change.

Version: SWC D.1.2.1
  01  11 July 2003, Yu Wei
    D.1.2.0 send to Ripple. Continue to add RMM.
    Add standby RTU link monitoring.

Version: CMM D.1.3.0
  01  01 Auguest 2003, Yu Wei
    All SWC use same class except Clock.

  02  07 Auguest 2003, Yu Wei
    Added m_unPollingTableStartAddress and m_unPollingTableLen
    for copying polling from primary RTU to standby RTU.

  03  14 August 2003, Yu Wei
    Default link status is link error.
    Added time synchronization.

  04  18 August 2003, Yu Wei
    Change trig polling to timer polling.

  05  19 August 2003, Yu Wei
    Add all server command.

Version: CMM D.1.3.1
  01  29 September 2003, Yu Wei
    Removed m_unPollingTableStartAddress, m_unPollingTableLen
    and m_nTimerPollingStatus.

Version: CMM D.1.3.2
  01  02 October 2003, Yu Wei
    Changed tSWCTableStructure for MMM.
    Added m_tInputTable.
    Added MMMReadSWCTable() and m_unFastPollingStartAdd,
    m_unSlowPollingStartAdd.

  02  30 October 2003, Yu Wei
    Changed CkeckCommand and CkeckExceptionCommand to
    CheckCommand and CheckExceptionCommand.


  03  06 November 2003, Yu Wei
    Added flag m_bMonitorAfterCheck to solve the problem that
    SWC will report link down when standby RTU completed link
    check.

  04  12 November 2003, Yu Wei
    SWC_MAX_TABLE change to 4.

    Added MMMWriteSWCTable() for MMM writing SWCtable.

  05  13 November 2003, Yu Wei
    Added watchdog function.
    Added m_tWatchdogID, SWC_WATCHDOG_TIMEOUT.

  06  27 November 2003, Yu Wei
    Change m_acStandbyRecev[2][264] to m_acStandbyRecev[2][1024]

Version: CMM D.1.3.4
  01  24 February 2004, Yu Wei
    Changed m_acMultidropReceData and m_acTimeSynCommand size to
    [2][264].

  02  02 March 2004, Yu Wei
    Added LogSwcInitValues() for module test.
    Refer to PR:OASYS-PMS 0038.

Version: CMM D.1.3.5
  01  09 March 2004, Yu Wei
    Added m_bCreateObjectResult for new object result.
    Deleted m_bFastPollingFlag. The system must include fast polling.
    Refer to PR:OASYS-PMS 0047.

  02  10 March 2004, Yu Wei
    Reduce complexity in CClock::CSWC().
    Added SetInputTableLength().
    Refer to PR:OASYS-PMS 0044.

  03  12 March 2004, Yu Wei
    Upon link established, RTU will send command to poll SWC AI/MI date.
    The SWC will do slow polling and timer polling.
    Added m_bSlowPollingStart and m_bTimerPollingStart.
    Refer to PR:OASYS-PMS 0058.

  04  22 March 2004, Yu Wei
    Removed tSWCTableStructure m_tInputTable[SWC_MAX_TABLE].
    The structure type has removed.
    Removed SetInputTableLength().

    Added m_unTimerPollingStartAdd for RMT write SWC table.
    Refer to PR:OASYS-PMS 0143.

  04  23 March 2004, Yu Wei
    Re-defined FAST_TABLE, SLOW_TABLE, TIMER_TABLE.
    Removed "Read SWC polling table results" definition.
    Added CopySWCTable() to copy primary RTU SWC table to standby RTU.
    Refer to PR:OASYS-PMS 0143.

    Added ClearVariables() to clear all variables when new object.
    Refer to PR:OASYS-PMS 0054.

  05  26 March 2004, Yu Wei
    Modify LinkMonitor() and CheckCommand(), removed CheckExceptionCommand(),
    used ModbusReplyCheck() checking rule to check recevied message.
    Refer to PR:OASYS-PMS 0053.

  06  31 March 2004, Yu Wei
    Modified MMMWriteSWCTable(). When address range error, it will
    return error code.
    Refer to PR:OASYS-PMS 0154.

Version: CMM D.1.3.6
  01  20 May 2004, Yu Wei
    Added "CMM_Timer.h" to use relative timer.
    Change m_atTimeOut to type "tTimerValue".
    Refer to PR:OASYS-PMS 0201.

    Added m_bStartupCheckLink to check link when system start up.
    Refer to PR:OASYS-PMS 0203.

Version: CMM E.1.0.0
  01  07 July 2004, Yu Wei
    Added m_nSWCBusyTimeout, m_nExcepTimeSyncRetry and
    m_nTimeSyncExcepCounter for time sync command exception process.
    Refer to PR:OASYS-PMS 0224.

  02  07 July 2004, Yu Wei
    Added m_nExcepRetry, m_nFastExcepCounter, m_nSlowExcepCounter,
    and m_nTimerExcepCounter for polling command exception process.
    Refer to PR:OASYS-PMS 0225.

Version: CMM E.1.0.1
  01  12 August 2004, Yu Wei
    Added KeepAlive(), m_bKeepAliveFlag, m_unSendKeepAliveAddr,
    m_nKeepAliveCMDLength,m_acKeepAliveCommand, m_nKeepAliveInterval,
    m_unKeepAliveCmdWord, KEEPALIVE_CMD to send KeepAlive sgnal to SWC.
    Refer to PR:OASYS-PMS 0233.

  02  13 August 2004, Yu Wei
    Changed StandbyLinkCheck() to PrimaryLinkCheck().
    Refer to PR:OASYS-PMS 0232.

Version: CMM E.1.0.2
  01  01 Apr 2005, Tong Zhi Xiong
    Changed following funtions to virtual and public, refer to PR:OASYS-PMS 0243.
      FastPolling, PrimaryLinkCheck, SlowPolling, TimerPolling, TimeSynchronization, KeepAlive, LinkCheck

**************************************************************/
#ifndef  _SWC_H
#define  _SWC_H



#include "Common.h"
//#include "CMM_Listen.h"
//#include "CMM_Comm.h"
//#include "CMM_Timer.h"    //040520 Yu Wei

//Message queue constants
#define MAX_MSGQ  1                //Maxmium message that queue in the buffer
#define MESSAGE_SIZE  sizeof(tSWCCommand_MSG)    //Message size.

//Timeout type
#define FASTPOLLING      0
#define SLOWPOLLING      1
#define TIMERPOLLING    2
#define PRIMARYSTATUS    3  // Primary Link Check timer
#define STANDBYSTATUS    4
#define TIME_SYN_CHECK    5
#define STANDBY_RTU_LINK1  6
#define STANDBY_RTU_LINK2  7
#define KEEPALIVE_CMD    8    //Send keep alive command timeout.  //040812 Yu Wei
#define LinkDownMoreThanOneDay 9
#define PLC_TIME_SYN_CHK  10
#define MAX_TIMEOUT_TYPE  11    //040812 Yu Wei

//SWC table type
#define FAST_TABLE    SWC_TABLE_DI8    //040323 Yu Wei
#define SLOW_TABLE    SWC_TABLE_AI_MI    //040323 Yu Wei
#define TIMER_TABLE    SWC_TABLE_MI    //040323 Yu Wei

//Read SWC polling table results
/*
#define READ_TABLE_OK    0
#define READ_TABLE_ERROR  (-1)
#define READ_TABLE_NOT    8000
*/

//Serial link status
#define LINKOK    0
#define LINKERROR  (-1)
#define NOLINK    8000

//Watchdog timeout
#define SWC_WATCHDOG_TIMEOUT 30000  //30 seconds

//Polling type
struct tPollingType
{
  bool bFast;      //Flag for fast polling.
  bool bSlow;      //Flag for slow polling.
  bool bStatus;    //Flag for polling link status.
  bool bSpecial;    //Flag for sending special server command.
  bool bLinkCheck;  //Flag for link status checking.
  bool bTimeSyn;    //Flag for time synchonization.
};



class CSWC
{
public:


  bool m_bCreateObjectResult;        //Create object result. //040309 Yu Wei
                      //true -- successful. false -- fail.

  char m_acSWCName[15];          //Store SWC name.

  int m_nSWCID;              //Store SWC ID Index.

  int m_nReceiveTimeout;          //Timeout to wait for SWC response.

//  int m_nMainTaskID;            //SWC main task ID.
  pthread_t m_nMainTaskID;
//  int m_nServerSocketTaskID;        //SWC server socket task ID.
  pthread_t m_nServerSocketTaskID;

  bool m_bTaskWorking;          //For stoping the task.
                      //false -- stop task. true -- task live.

  UINT8 ucretrycnt;   // RequestAndReceiveWithModbusCheck actual retry counter

  int m_nLinkType;            //Link type: RS_485/RS_422/RS_232,
                      //       RS_485M/RS_422M multidrop,
                      //       LAN.
  int m_nTotolNodeNumber;          //for multinode SWC
  bool m_bPollingEnable;          //The flag to enable/inhibit SWC polling.

  #ifdef CFG_EXTENDED_MODBUS
  // Support extended modbus protocol. Byte field is zero when received packet
  // is more than 255.
  BOOL_T m_bIsExtendedModbus;
  #endif // CFG_EXTENDED_MODBUS

  #ifdef CFG_EXTENDED_MODBUS
  // Allow zero byte count when modbus data size is less than 255 bytes
  BOOL_T m_bIsModbusZeroByteCntAllowed;
  #endif // CFG_EXTENDED_MODBUS

  int  m_anCommfd[2];            //Serial communication port file descriptor.
  int  m_anCommPortId[2];        //Comm port ID
  tPollingType m_atPollingFlag[2];    //Flag for polling status.
                      //ture -- implement, false -- idle

  int m_anRetryNumberStatus[2];      //Buffer for re-try times when polling status.

  int  m_nSWCCurrentStateFlag;        //Current SWC working state.

  unsigned short  m_aunSWCTableFast[2048];  //Buffer for fast polling table.
  unsigned short  m_aunSWCTableSlow[2048];  //Buffer for slow polling table.
  unsigned short  m_aunSWCTableTimer[2048];  //Buffer for Timer polling table.

  //Record link status. (Link 0 or 1)
  int m_anLinkStatus[2];
  int m_nMasterLinkID;          //Store master link ID.
                      // 0, first link is primary link,the second link is standby.
                      // 1, second link is primary link, the first link is standby.

  unsigned short m_unCurrentLinkStatus;  //Word for link status for checking if the link status is changed.
  UINT16  m_usTempStatus;

  mqd_t  m_tMSGQID;          //Message queue ID.
  char m_messageQname[32];        //message queue name

  int m_nFastPollingValue;        //Fast polling interval in ms.
  int m_nSlowPollingValue;        //Slow polling interval in ms.
  int m_nTimerPollingValue;        //Timer polling interval in minutes.
  int m_nRetryNumber;            //Re-try times if link no response.
  int m_nStandbyPollingValue;        //Standby link status checking interval in ms.
  int m_nStandbyPollingFailedValue;    //When link failed, link status checking interval in ms.

  char m_acFastPollingCommand[2][16];    //Buffer for fast polling command.
  char m_acSlowPollingCommand[2][16];    //Buffer for slow polling command.
  char m_acTimerPollingCommand[2][16];  //Buffer for timer polling command.
  char m_acWriteCommand[2048];      //Buffer for specific command that will send to SWC from server.
  char m_acStatusCommand[2][16];      //Buffer for link status checking command.

  bool  m_bTimeSynFlag;          //If true, time synchronization valid.
  bool  m_bTimeSynStartFlag;      //Re-start to set, when time synchronization had been implemented,
                      //set to false.
  int    m_nSendTimeSynHour;        //Hour which the time synchronization message is sent.
  int    m_nSendTimeSynMinute;      //Minute which the time synchronization message is sent.
  int m_nTimeSynCMDLength;        //Command size for time synchronization.
  char m_acTimeSynCommand[2][264];    //Buffer for time synchronization command.

  unsigned short m_unSendKeepAliveAddr;  //Keep Alive command address.        //040812 Yu Wei
  int  m_nKeepAliveInterval;        //Keep Alive command interval.        //040812 Yu Wei
  int m_nKeepAliveCMDLength;        //Command size for sending KeepAlive signal.//040812 Yu Wei
  char m_acKeepAliveCommand[2][264];    //Buffer for KeepAlive command.        //040812 Yu Wei
  unsigned short m_unKeepAliveCmdWord;  //The server status word for keep alive command. //040812 Yu Wei

  //Fast polling parameter.
  int m_nFastPollingRecevLen;        //Fast polling receiving data length.
  int m_nFastDataLen;            //Fast polling command length.
  int m_nServerFastTableStartAddr;    //Fast polling server table start address.
  int m_nServerFastTableLen;        //Fast polling server table length.
  //bool m_bFastPollingFlag;        //Flag whether the fast polling implements in the SWC.
                      //040309 Yu Wei

  //Slow polling parameter.
  int m_nSlowPollingRecevLen;        //Slow polling receiving data length.
  int m_nSlowDataLen;            //Slow polling command length.
  int m_nServerSlowTableStartAddr;    //Slow polling server table start address.
  int m_nServerSlowTableLen;        //Slow polling server table length.
  bool m_bSlowPollingFlag;        //Flag whether the slow polling implements in the SWC.

  bool m_bSlowPollingStart;        //When startup and link recovere, the flag is set,
                      //the slow polling will do.
                      //040312 Yu Wei

  //Timer polling parameter.
  int m_nTimerPollingRecevLen;      //Timer polling receiving data length.
  int m_nTimerDataLen;          //Timer polling command length.
  int m_nServerTimerTableStartAddr;    //Timer polling server table start address.
  int m_nServerTimerTableLen;        //Timer polling server table length.
  bool m_bTimerPollingFlag;        //Flag whether the timer polling implements in the SWC.

  bool m_bTimerPollingStart;        //When startup and link recovere, the flag is set,
                      //the timer polling will do.
                      //040312 Yu Wei
  bool m_bKeepAliveFlag;          //The flag for Keep Alive signal sending.  //040812 Yu Wei

                                      // 0 transmit complete; 1 waiting for reply or timeout.//290513 Yang Tao

  int m_nStatusPollingRecevLen;  //Link status polling response message size.
  int m_nSpecificCommandLen;    //Length of specific command
  int m_nLinkMonitorRxTimeOut;   // Timeout to receive a complete fast polling
                                 // table during link monitoring at Standby

  char m_acMultidropReceData[264];//For multidrop link receive data.
  int m_nMultidropReceDataLen;  //For multidrop link receive data size.

  unsigned char m_ucSWCAddress;  //SWC slave address. For server special command


  //Semiphone ID for table accessing.
  pthread_mutex_t         m_tAccessTableSemID;
  pthread_mutex_t         SWC_main_wd_mutex;
  pthread_cond_t           SWC_main_wd_cond;
  timer_t             SWC_main_timer_id;




  struct tTimerValue m_atTimeOut[MAX_TIMEOUT_TYPE];  //Buffer for timer counter.  //040520 Yu Wei

  int m_nTotalLinkNumber;      //Total serial link number. (1 or 2)
  int m_nPrimaryPortID;      //Primary link ID (0 or 1)
  int m_nStandbyPortID;      //Standby link ID (0 or 1)

  char m_acStandbyRecev[2][2048];  //Buffer for standby RTU receive data that reply from SWC.
  int m_anStandbyRecevLen[2];    //Received data length.

  //tSendReplyPara m_tSendReply;  //The parameter is used by SendReply().

  //Timeout timer ID for standby RTU monitoring link status.
  int m_anStandbyMonitorTimeout[2];

  bool m_bLinkCheckFlag;    //Link check flag.
                //If the flag is set in standby RTU, start to check link.
                //When the link checking completed, the flag will be reset.
                //When SWC received "Stop Link" message in primary RTU, the
                //flag is set and stop polling.
                //After standby RTU copmleted the link checking, the primary
                // will receive "Start Link" message, the flag will be reset
                // and start polling.

  // tSWCTableStructure m_tInputTable[SWC_MAX_TABLE];  //SWC input table.
                //The type and variable are removed. 040322 Yu Wei

  unsigned short m_unFastPollingStartAdd;        //Fast polling start address.
  unsigned short m_unSlowPollingStartAdd;        //Slow polling start address.
  unsigned short m_unTimerPollingStartAdd;      //Timer polling start address.  //040322 Yu Wei

//  WDOG_ID    m_tWatchdogID;  //Watchdog ID.
  int    m_tWatchdogID;

  //true -- to check link, don't check timeout.
  //false-- to check link base on timeout.
  // This flag is used to detect link recovered. It will be used by
  //   PrimaryLinkCheck routine to enable other startup flag such as
  //   m_bTimerPollingStart and m_bTimSynStartFlag
  bool    m_bStartupCheckLink;

  //040707 Yu Wei
  // for delay interval in ms to resend time sync command to SWC.
  // correspond to SLAVE_BUSY_TIMEOUT in config file
  int      m_nSWCBusyTimeout;
  int      m_nExcepTimeSyncRetry;    //for number of retry counter before stopping time sync.
  int      m_nTimeSyncExcepCounter;  //The counter for time sync command exception.

  int      m_nExcepRetry;        //for number of retry counter before declare exception failure.
  int      m_nPrimaryExcepCounter; // The counter for primary exception for
                                   // testing purpose. (Added by ZSL)
  int      m_nFastExcepCounter;    //The counter for fast polling exception.
  int      m_nSlowExcepCounter;    //The counter for slow polling exception.
  int      m_nTimerExcepCounter;    //The counter for timer polling exception.

  //040707 Yu Wei
  bool     m_bCheckingBeforeSwitch;

private:

  void ClearVariables(void);    //040323 Yu Wei

  //void SetInputTableLength(void);    //040310 Yu Wei    //040322 Yu Wei


public:
  char HexToBCD(char);//2008  private ---> public

  //change to virtual and public, tong
  virtual void FastPolling(void);
  virtual  void PrimaryLinkCheck(void);      //040813 Yu Wei
  virtual void SlowPolling(void);
  virtual void TimerPolling(void);
  virtual void TimeSynchronization(void);
  virtual void KeepAlive(void);      //040812 Yu Wei
  virtual void LinkCheck(void);
  virtual E_ERR_T RequestAndReceiveWithModbusCheck(INT32 fd,
                   UINT8 *ptxcmd,
                   UINT8 txcmdsz,
                   VOID *prxbuff,
                   UINT32 expectedRxSz,
                   INT32 timeout_ms,
                   UINT8 retry,
                   UINT8 exceptionRetry,
                   UINT8 *poutexceptionRetryCnt);
  //virtual void PrimaryLinkCheckBeforeSwitching(void);//20050518 Tong

  CSWC(struct tSWCCFGStructure,int, char *);
  CSWC(E_SWC_T swc_type);
  CSWC();
  virtual ~CSWC();

  CListen *m_pServerSocket;  //Object for server interface.

  virtual void SetReadCommand(char *,char,char,tModbusAddress);
  virtual int SetWriteCommand(char *,char *,char,char,tModbusAddress);
  virtual void UpdateCommand(char *,int, char);
  virtual void WriteSWCTable(unsigned short *, char *, int);
  virtual void CopySWCTable(unsigned short *, unsigned short);  //040323 Yu Wei
  virtual int ReadSWCTable(unsigned short *, int);
  //int ReadSWCTableI(char *, int, int);        //040323 Yu Wei
  virtual int ReadSWCOneTable(char *, unsigned short, int);  //040323 Yu Wei
  int MMMWriteSWCTable(unsigned short, unsigned short);
  virtual void MainProcess(void);
  void CheckServerCommand(void);
  virtual void ServerCommand(char *, int);
  void ServerCommandError(int, char *);
  virtual int SpecificCommand(char *, int);
  virtual int SpecificCommandSWCError(char *acReceData);
  void CheckMessageQ(void);
  int SendMessageQ(tSWCCommand_MSG);
  void ActivateTimeOut(int);
  bool CheckTimeOut(int, unsigned int);
  unsigned short GetLinkStatus(void);
  virtual void CheckLinkStatusChange(void);
  void SetStateFlag(void);
  virtual VOID LinkMonitor(VOID);
  //virtual int CheckCommand(char *, int , char);    //040326 Yu Wei
  //bool CheckExceptionCommand(char, int *);  //040326 Yu Wei
  virtual void SetTimeSynCommand(unsigned short);

  //void SetSendReplyPara(int, int);


  //int  ReadSWCAllTable(char *aunServerTable, unsigned short,int nTimeoutType);//2008
  //void CopySWCAllTable(unsigned short *aunTable, unsigned short nTableID);//2008
  //virtual int GetRecordFromLocalQueue(unsigned short *);
  //virtual VOID *SWC_MgmtThread(VOID *nindex);

  #ifdef CFG_MODULE_TEST
  virtual void MDT_PrnToLog(const CHAR *pfilename, CHAR *pswcname, CHAR *plogmsg);
  #endif // CFG_MODULE_TEST

#ifdef ModuleTest
  void LogSwcInitValues(char* pName);
#endif

};


// Terminate SWC flag to exit application
extern BOOL_T   bSWCExit;
extern CSWC *g_apSWCObject[SWC_MAX_NUMBER];      //Object for all SWC.


void SWCMainWatchDogInit();
extern VOID *SWC_MgmtThread(VOID *nindex);
extern VOID *SWC_ServerListeningThread(VOID *nIndex);
extern VOID *SWC_NTPClientCommunicationThrd(VOID *pThreadParameter);
#endif /* _SWC_H */


