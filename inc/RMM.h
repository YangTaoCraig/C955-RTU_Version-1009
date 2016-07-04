/************************************************************
*                              *
*  Project:    C830 RTU                  *
*  Module:    Redundancy Management Module (RMM)      *
*  File :      RMM.h                    *
*  Author:    Yu Wei                    *
*                              *
*  Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************

**************************************************************

DESCRIPTION

  This is header file for all RMM cpp file. It defines constants,
  declares function and class.

*************************************************************
Modiffication History
---------------------

Version: RMM D.1.1.1
  01  16 April 2003, Yu Wei,
    Start to write.

Version: SWC D.1.2.1
  01  14 July 2003, Yu Wei,
    D.1.2.0 send to Ripple. Continue to add RMM.
    Add RTU notation (weight) calculation.

  02  15 July 2003, Yu Wei
    Add other RTU status table accessing function.

  03  21 July 2003, Yu Wei,
    Add SWC link down in primary RTU, standby RTU will check
    the link status. Based on checking result, the primary RTU
    decide whether the state change.

Version: RMM D.1.3.0
  01  01 August 2003, Yu Wei,
    All SWC use same class except Clock.

  02  07 August 2003, Yu Wei,
    Updated polling table is done by RMM.
    Added primary RTU sending polling table to standby RTU.
    Set default Clock link is down and other SWC is OK.

  03  19 August 2003, Yu Wei
    Add all server command.

  04  07 November 2003, Yu Wei,
    Added m_bServerSwitchFlag to indicate that server switches
    primary RTU to standby.

  05  12 November 2003, Yu Wei,
    Added m_unOtherRTUServerLinkStatusPrev and StandbyRTUImproveCheck()
    for standby RTU server link recovered check.

  06  13 November 2003, Yu Wei
    Deleted STATEFLAG_SWITCHING_TEST state. This is not used.

  07  13 November 2003, Yu Wei,
    Added g_unLastServerCommand for MMM using.

  08  14 November 2003, Yu Wei,
    Added CheckSWCLinkStop() for checking SWC link stop.

Version: RMM D.1.3.3
  01  26 January 2004, Yu Wei,
    Delete CRTUStatus() parameter, using global variable.

Version: RMM D.1.3.4
  01  02 March 2004, Yu Wei
    Added LogRmmStatusValues() for module test.
    Refer to PR:OASYS-PMS 0032.

Version: RMM D.1.3.5
  01  10 March 2004, Yu Wei
    Reduce complexity in RMMCheckServerCommand() and
    CRTUStatus::RTUStatusTableProcess().
    Added LogServerCommand() and CRTUStatus::SWCLinkCheckProcess().
    Modified CalculateWeight(), when weight is change, it will update
    RTU status table.
    Refer to PR:OASYS-PMS 0044.

  02  11 March 2004, Yu Wei
    Fixed bug that a SWC link down, the RTU will switch to standby.
    The primary didn't wait for standby check link completed.
    Added m_bWaitLinkCheckCompletedFlag.
    Refer to PR:OASYS-PMS 0066.

  03  22 March 2004, Yu Wei
    Changed STATE4_SWITCHING to STATE4_SWITCHING_PTOS and
    added STATE5_SWITCHING_STOP.
    Refer to PR:OASYS-PMS 0143.

  04  23 March 2004, Yu Wei
    Added m_bSWCSwitchCompletedFlag for checking if all SWC changed to
    STATEFLAG_SWITCHING_PTOS,
    and m_bSendSWCTableCompletedFlag for check if send all SWC latset
    table to standby RTU completed,
    m_bStartSendSWCTableFlag.
    Deleted m_unTablePoll[]. Not used.
    Refer to PR:OASYS-PMS 0143.

    Added ClearVariables() to clear all variables when new object.
    Refer to PR:OASYS-PMS 0054.

Version: RMM D.1.3.6
  01  19 April 2004, Yu Wei
    Added m_bOtherRTUSwitchFlag to store other is switching info.
    Refer to PR:OASYS-PMS 0163.

  02  20 April 2004, Yu Wei
    Added m_nSwitchFlag to store switching status.
    Added 'Switching status' defination.
    Refer to PR:OASYS-PMS 0163.

**************************************************************/
#ifndef  _RMM_H
#define  _RMM_H




#include "Common.h"
#include <mqueue.h>

#define MAX_RMM_MSGQ  50            //Maximum message that store in the message queue buffer.
#define RMM_MESSAGE_SIZE  sizeof(tRMM_MSG)  //Size of message.
#define RTU_STATUS_TABLE_SWC_OFFSET    13    //First address of SWC communication status in RTU status table.
#define RTU_STATUS_TABLE_HEALTH_STATUS    6    //The address of RTU health status.
#define RTU_STATUS_TABLE_SWC_WEIGHT    7    //The address of SWC weight.
#define RTU_STATUS_TABLE_SWC_LINK_CHECK  8    //When SWC link down in primary RTU, the word will set and
                        //standby RTU will check all link that is down.
                        //When Standby RTU start to check link, the word will set.
                        //When the link checking has been completed, the word will reset.
#define RTU_STATUS_TABLE_LENGTH_MAX    33    //Maximum buffer size of RTU status table.

#define SERVERPOLL_SWC_COPY_MAX_TRY    5    //maximum no. of attempts to copy SWCs' tables

//Server command code
#define SERVER_CMD_RESET_RTU        0x8000
#define SERVER_CMD_SWITCH_TO_STANDBY    0x4000
#define SERVER_CMD_SWITCH_TO_PRIMARY    0x2000
#define SERVER_CMD_DOWNLOAD_CFG_COMPLETE  0x1000
#define SERVER_CMD_DOWNLOAD_CFG_REQUIRED  0x0800
#define SERVER_CMD_ENABLE_SWC_POLLING    0x0200
#define SERVER_CMD_INHIBIT_SWC_POLLING    0x0100

#define RMM_MAIN_WATCHDOG_TIMEOUT    30  //Watchdog timeout 30 seconds.

//Switching status    //040420 Yu Wei
#define NOT_IN_SWITCH      0x0000    //Not in switching state.
#define START_SWITCH      0x0010    //Start to switch
#define COMFIRM_SWITCH      0x0020    //Confirm switch
#define COMFIRM_SWITCH_ERROR  0x0021    //onfirm switch error. Standby didn't receive confirm command.

void*   RMMTask(void *arg);
int   RMMInitialization(void);
void   RMMCheckServerCommand(void);
void   RMMServerCommand(char *);
void   RMMServerCommandError(int, char *);
void   LogServerCommand(char *, int, char *);    //040310 Yu Wei
void*   RMMServerSocketTask(void *arg);
void   StopRMM(void);


class CRTUStatus
{

private:

  void ClearVariables(void);  //040323 Yu Wei
public:

  bool m_bServerSwitchFlag;    //Indicate that server switches primary RTU to standby.

  int m_nServerRTUStatusTableLength;    //Length of server RTU status table.

//  sem_t  *m_tAccessRTUStatusTableSemID;  //Semiphone ID for accessing RTU status table.
  pthread_mutex_t m_tAccessRTUStatusTableSemID;

//  MSG_Q_ID  m_tRMM_MSGQ_ID;    //Message queue ID for receiving message that sent by other module.
  mqd_t      m_tRMM_MSGQ_ID;

  //unsigned short m_unTablePoll[SERVERPOLL_TABLE_POLL_SIZE];  //Buffer for Standby RTU polling table
  //040323 Yu Wei                        //Primary RTU send SWC polling table to standby
                                //and standby store the table in this buffer.
                                //When the state is changed to primary, the buffer
                                //will be copied to polling server's RTU polling table.

  bool m_bSWCSwitchCompletedFlag;    // Flag to indicate if all SWCs change to STATEFLAG_SWITCHING_PTOS.
                    //040323 Yu Wei
  //Flag to indicate if the sending of SWC table to the other RTU has completed
  bool m_bSendSWCTableCompletedFlag;

  // Flag to indicate if the sending of all SWC table has started.
  // This will happen before RTU switch role.
  // 1 - start to send. 0 - after start set. 040323 Yu Wei
  bool m_bStartSendSWCTableFlag;

public:

  //Buffer for local RTU status table.
  unsigned short m_aunLocalRTUStatus[RTU_STATUS_TABLE_LENGTH_MAX];

  //Buffer for other RTU status table.
  unsigned short m_aunOtherRTUStatus[RTU_STATUS_TABLE_LENGTH_MAX];

  //Standby RTU Link Check index
  unsigned short m_aunSWCLinkCheckIndex[SWC_MAX_NUMBER];
                                    //about primary RTU link down.
  unsigned short m_aunMultiNodeLinkStatus ;// SWC multinode status
  int  m_nMultiNodeLinkStatusAdd ;//SWC multinode address

  //When standby RTU check link, the flags will be set,
  //when the checking has completed, the flags will be reset.
  //For primary RTU, when the link is stopped, the flag is set.

  // when the SWC is doing link check, set the designated flag to true
  bool m_abSWCLinkCheckFlag[SWC_MAX_NUMBER];

  // If any of the m_bSWCLinkCheckFlags is true, the flag will set to true.
  // When all SWC link check index is false, the flag is set to false
  bool m_bSWCLinkCheckTotalFlag;

  /*
  Flag m_bWaitLinkCheckCompletedFlag:
   At Primary mode, the flag will be set CRTULink::NormalProcess,
   POLLING_WRITE_TABLE and reset at either CRTULink::NormalProcess,
   POLLING_READ_TABLE.

   At Standby mode, the flag will be set at
   CRTUStatus::SetSWCLinkCheckIndexTable, and reset either at
   CRTUStatus::SetLinkCheckCompleted after 1250ms delay
   (< RMM_MAIN_WATCHDOG_TIMEOUT) when RTU is not switching
   over to Primary; or at [RMM] ClearFlag after switching over to Primary mode.

   The delay must be longer than the receiving of switch command from
   WriteCommandProcess: rx CMD_START_SWITCH.

   Otherwise stdby RTU rx SWC_LINK_STATUS_CHANGE will overwrite the link check
   result.

  */
  //When primary RTU set flag asking standby RTU check link and
  //receive a normal response, it will be true.
  //When SWC link down, it will be false.
  //040311 Yu Wei
  bool m_bWaitLinkCheckCompletedFlag;
  struct timespec m_linkChkStart;
  struct timespec m_linkChkStop;

  struct timespec m_tStartTime;  //Starting time for timeout counter.

  unsigned short m_unSWCWeight;          //Weight for local RTU SWC.
  unsigned short m_unOtherSWCWeight;        //Weight for other RTU SWC
  unsigned short m_unOtherSWCWeightPrev;      //Previous weight for other RTU SWC
  unsigned short m_unOtherRTUServerLinkStatus;  //Other RTU server link status.
  unsigned short m_unOtherRTUServerLinkStatusPrev;//Previous other RTU server link status.

  //Other RTU state flag.
  int m_nOtherRTUStateFlag;

  //When primary RTU link down, the flag will set to true. When standby RTU
  //completed link check, the flag will set to false.
  bool m_bSWCLinkDownProcessFlag;

  //When a SWC's link status change to down the flag is set. And when send
  //status table to standby RTU, the flag is reset.
  bool m_bSWCLinkDownFlag;

  /* Other RTU switching flag. 040419 Yu Wei
     When primary RTU switch over and send command to other RTU
     and received other's reply. The flag will be set to true.
     The flag is true, the RTU will not accept server change state command.
     When RTU link ok or time, the flag will be set to false. */
  bool m_bOtherRTUSwitchFlag;

  //040420 Yu Wei
  /*Normal: NOT_IN_SWITCH
    Primary send command to standby switch, or standby receive switching
    command, set to START_SWITCH.
    Primary send switching confirm or standby receive switching confirm,
    set to CONFIRM_SWITCH. */
  int m_nSwitchFlag;

  CRTUStatus(void);
  ~CRTUStatus(void);

  void RTUStatusTableProcess(void);
  void RTUInitLinkCheck(void);
  int ReadRTUStatus(unsigned short *, int);
  void UpdateRTUStatus(unsigned short, int);
  int SendMsg(tRMM_MSG);
  UINT16 CheckRTUStatus(void);
  void CalculateWeight(void);      //040310 Yu Wei
  void SWCLinkCheckProcess(void);    //040310 Yu Wei

  void WriteOtherRTUStatus(unsigned short *);
  void SetSWCLinkCheckIndexTable(void);
  void SendMessageToSWC(int, int);
  //int SendSWCPollingCommand(int, int);
  E_ERR_T SendSWCPollingCommand(INT32 nCommand, INT32 nSWCID);
  bool SetLinkCheckCompleted(void);
  bool CheckSWCLinkStop(void);
  void SetSWCLinkStop(void);
  void SetSWCLinkStart(void);
  void ClearFlag(void);

  //void CopySWCPollingTable(unsigned short *, unsigned short, unsigned short); //040323 Yu Wei
  //int    ReadMultiNodeSWCLinkStatus(unsigned short *aunServerTable, int nTimeoutType);//2008
  //void   UpdateMultiNodeSWCLinkStatus(unsigned short unStatus, int nAddress);

#ifdef ModuleTest
  void LogRmmStatusValues();  //for test
#endif

};


// for RMMState.cpp
enum eCurrentState{
  STATE0_NULL,
  STATE1_PRIMARY,
  STATE2_STANDBY,
  STATE3_INITIALIZATION,
  STATE4_SWITCHING_PTOS,    //040322 Yu Wei.
  STATE5_SWITCHING_STOP,    //040322 Yu Wei.
  STATE6_HARDWARE_TEST,    //040322 Yu Wei.
  STATE7_RESET_SYSTEM,    //040322 Yu Wei.
  STATE_MAX_RMM
};

struct tRTUPermanentParameter
{
  unsigned short  unCFGDownloadUTCHigh;  //Latest UTC time (high word) for config.txt downloading.
  unsigned short  unCFGDownloadUTCLow;  //Latest UTC time (low word) for config.txt downloading.
};
void StateManagement(void);
void StateInitialize(void);
void ChangeStateTo(int);
pthread_t* RMMWatchDogInit(pthread_t tid[2]);

/*for wathcdog*/



void ActivateTimeOut(void);
bool CheckTimeOut(unsigned int);
void StandbyRTUImproveCheck(void);

extern CRTUStatus *g_pRTUStatusTable;
extern unsigned short g_unLastServerCommand;    //Store last server command for MMM reading.

extern int g_nWeightDoubleCheck;//20050616
extern bool g_bRTUInitLinkCheck;
extern bool g_abSWCInitLinkCheck[SWC_MAX_NUMBER];
extern tRTUPermanentParameter g_tRTUPerPara;  //RTU permanent parameters.

#endif /* _RMM_H */

