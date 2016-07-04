/*----------------------------------------------------------------------------

            Copyright (c) 2011 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME

  SWC.cpp

 COMPONENT

  Subsystem Wide Contractor (SWC)

 DESCRIPTION

  This file consists of management routine for all SWC devices.

 AUTHOR

  Yu Wei


 REVISION HISTORY by Bryan Chong

 D1.1.5
 ------
 09-Jun-2011
 - Add support to allow zero byte at the data byte count field for Standby RTU
   when monitoring Modbus packet from ECS only

 31-May-2011
 - convert cout to printf for readability issue from the group [PR99]

 D1.1.3
 ------
 27-Apr-2011

 - CSWC::CheckLinkStatusChange
     Add conditional check on m_bWaitLinkCheckCompletedFlag.
     m_unCurrentLinkStatus will only be updated when
     m_bWaitLinkCheckCompletedFlag is reset

------------------------------------------------------------------------------*/
/*******************************************************************************
*                              *
*  Project:    C830 RTU                  *
*  Module:    CMM                      *
*  File :      SWC.cpp                    *
*  Author:    Yu Wei                    *
*                              *
*  Copyright 2003 Singapore Technologies Electronics Ltd.  *
********************************************************************************

********************************************************************************


DESCRIPTION

  This file defines common SWC class.

********************************************************************************

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
  01  01 August 2003, Yu Wei
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
  01  24 September 2003, Yu Wei
    Modify GetLinkStatus. Update SWC link status bit 2 and bit 12,13.
    Refer to PR:OASYS-PMS 0002.

  02  29 September 2003, Yu Wei
    Removed m_unPollingTableStartAddress, m_unPollingTableLen
    and m_nTimerPollingStatus.

Version: CMM D.1.3.2
  01  02 October 2003, Yu Wei
    Changed tSWCTableStructure for MMM.
    Added m_tInputTable.
    Added MMMReadSWCTable() and m_unFastPollingStartAdd,
    m_unSlowPollingStartAdd.

  02  06 October 2003, Yu Wei
    Changed tSerialPortPara.acPortName[16] to
    tSerialPortPara.unPortID.

  03  30 October 2003, Yu Wei
    Changed CkeckCommand and CkeckExceptionCommand to
    CheckCommand and CheckExceptionCommand.

  04  06 November 2003, Yu Wei
    Added flag m_bMonitorAfterCheck to solve the problem that
    SWC will report link down when standby RTU completed link
    check.

  05  07 November 2003, Yu Wei
    Move "if(m_nSWCCurrentStateFlag != STATEFLAG_INITIALIZATION)" into
    CheckLinkStatusChange().
    After FastPolling() and StandbyLinkCheck(), update link status
    immediatlt. It can get new SWC link status quickly.

  06  12 November 2003, Yu Wei
    SWC_MAX_TABLE change to 4.
    MI table that is polled by slow polling uses 300 as table ID.
    Table ID = 450 is not used.

    Added MMMWriteSWCTable() for MMM writing SWCtable.

  07  13 November 2003, Yu Wei
    Added watchdog function.
    Added m_tWatchdogID, SWC_WATCHDOG_TIMEOUT.

  08  01 December 2003, Yu Wei
    Modify LinkMonitor(), added event log for link breakdown and recovered.

Version: CMM D.1.3.3
  01  16 Decmeber 2003, Yu Wei
    Fixed bug that standby RTU will send special server command to SWC.
    Modified CheckServerCommand() and ServerCommandError().
    Refer to PR:OASYS-PMS 0021.

  02  22 Decmeber 2003, Yu Wei
    Fix bug that Power didn't do slow polling.
    Change slow polling parameters, there is only one slow polling.
    Deleted m_nSlowPollingSequence, m_nTotalTimerPollingNumber,
    m_nTotalSlowPollingNumber

  03  29 Decmeber 2003, Yu Wei
    Exception response is a valid reply.

  04  08 January 2004, Yu Wei
    MMMReadSWCTable() cannot used memcpy().

  05  12 January 2004, Yu Wei
    Watchdog cancel done be main task.

  06  30 January 2004, Yu Wei
    Upon RTU resume link with SWC, RTU should send time sync.
    Modified CheckLinkStatusChange().
    Refer to PR:OASYS-PMS 0025.

Version: CMM D.1.3.4
  01  27 February 2004, Yu Wei
    Fixed bug in CheckMessageQ(). Standby RTU can enable polling.
    Refer to PR:OASYS-PMS 0037.

  02  02 March 2004, Yu Wei
    Added LogSwcInitValues() for module test.
    Refer to PR:OASYS-PMS 0038.

Version: CMM D.1.3.5
  01  09 March 2004, Yu Wei
    Serial link must be 1 or 2, otherwise the CSWC will set
    create object error flag. Added m_bCreateObjectResult.
    Modified CSWC().
    Refer to PR:OASYS-PMS 0047.

  02  10 March 2004, Yu Wei
    Reduce complexity in CClock::CSWC().
    Added SetInputTableLength().
    Refer to PR:OASYS-PMS 0044.

    Added default case for switch in ReadSWCTableI(), MMMReadSWCTable()
    and MMMWriteSWCTable(), ServerCommand(), CheckMessageQ() and
    SetStateFlag().
    Refer to PR:OASYS-PMS 0042.

  03  12 March 2004, Yu Wei
    Upon link established, RTU will send command to poll SWC AI/MI date.
    The SWC will do slow polling and timer polling.
    Added m_bSlowPollingStart and m_bTimerPollingStart.
    Modified CSWC(), FastPolling(), StandbyLinkCheck(), SlowPolling(),
    TimerPolling(),
    Refer to PR:OASYS-PMS 0058.

  04  22 March 2004, Yu Wei
    Removed tSWCTableStructure m_tInputTable[SWC_MAX_TABLE].
    The structure type has removed.
    Removed SetInputTableLength().

    Modified MMMReadSWCTable(). Direct to read fast, slow, timer polling
    table.
    Added m_unTimerPollingStartAdd.
    Refer to PR:OASYS-PMS 0143.

    STATEFLAG_SWITCHING is changed, modified SetStateFlag().
    Refer to PR:OASYS-PMS 0143.

  04  23 March 2004, Yu Wei
    Combined ReadSWCTableI() and MMMReadSWCTable() to ReadSWCOneTable().
    MMM read table should WAIT_FOREVE.
    RMM read table only, don't add start address.
    Added CopySWCTable() to copy primary RTU SWC table to standby RTU.
    Refer to PR:OASYS-PMS 0143.

    Added ClearVariables() to clear all variables when new object.
    Refer to PR:OASYS-PMS 0054.

  05  25 March 2004, Yu Wei
    Modified GetLinkStatus() to fix bug. PSD2's status error.
    Refer to PR:OASYS-PMS 0148.

  06  26 March 2004, Yu Wei
    Modify LinkMonitor() and CheckCommand(), removed CheckExceptionCommand(),
    used ModbusReplyCheck() checking rule to check recevied message.
    Refer to PR:OASYS-PMS 0053.

    Modified SetSendReplyPara(). Link1 and Link2 should use different time.
    Refer to PR:OASYS-PMS 0159.

  07  29 March 2004, Yu Wei
    Modified WriteSWCTable(). Convert local time to UTC time.
    Refer to PR:OASYS-PMS 0151.

  08  30 March 2004, Yu Wei
    Modified LinkCheck() to fixed bug. When checking link, the
    standby link = OK, primary link = ERROR, and then switch over
    to primary, the fast polling will not work.
    Refer to PR:OASYS-PMS 0152.

  09  31 March 2004, Yu Wei
    Modified MMMWriteSWCTable(). When address range error, it will
    return error code.
    Refer to PR:OASYS-PMS 0154.

  10  31 March 2004, Yu Wei
    Modified CheckServerCommand(). Check modbus first.
    Refer to CMM_Modbus.cpp Version: CMM D.1.3.5 (02).

Version: CMM D.1.3.6
  01  30 April 2004, Yu Wei
    Modified SpecificCommand()to check SWC reply using standard
    modbus check function.
    Refer to PR:OASYS-PMS 0183.

  02  30 April 2004, Yu Wei
    Modified MainProcess(). When the link is inhibited, standby RTU
    don't monitor and check link.
    Refer to PR:OASYS-PMS 0177.

  03  20 May 2004, Yu Wei
    Modified CSWC(), ActivateTimeOut() and CheckTimeOut() to use
    relative timer and avoid the timer jump.
    Refer to PR:OASYS-PMS 0201.

    Added m_bStartupCheckLink to check link when system start up.
    Modeified CSWC(),FastPolling(), StandbyLinkCheck() and
    MainProcess(). When system startup, the link must be checked
    one time.
    Refer to PR:OASYS-PMS 0203.

Version: CMM E.1.0.0
  01  07 July 2004, Yu Wei
    Modefied CSWC() to initialize m_nSWCBusyTimeout,
    m_nExcepTimeSyncRetry and m_nTimeSyncExcepCounterfor.
    Modified TimeSynchronization() to process time sync command
    exception.
    Refer to PR:OASYS-PMS 0224.

  02  07 July 2004, Yu Wei
    Modefied CSWC() to initialize m_nExcepRetry, m_nFastExcepCounter,
    m_nSlowExcepCounter and m_nTimerExcepCounter.
    Modified FastPolling(), SlowPolling() and TimerPolling() to set
    and reset execption counters.
    Modified GetLinkStatus() to set execption flag.
    Modified LinkMonitor() to set and reset execption counter for
    standby RTU.
    Modified SetStateFlag(). When state is changed to primary or
    standby, the execption counters will be reset.
    Refer to PR:OASYS-PMS 0225.

  03  07 July 2004, Yu Wei
    Modified SpecificCommand(). When SWC send execption reply for
    server command, RTU will send the reply to server directly.
    When Link is inhibited, reply timeout to server directly.
    Refer to PR:OASYS-PMS 0227.

  04  07 July 2004, Yu Wei
    Modified CheckServerCommand(). When command cannot execute, RTU
    will reply execption 3 instead of 4.
    Refer to PR:OASYS-PMS 0229.

  05  08 July 2004, Yu Wei
    Modified CheckMessageQ(). When link is inhibited, SWC will
    reply RMM link check completed immediately.
    Refer to PR:OASYS-PMS 0221.

Version: CMM E.1.0.1
  01  12 August 2004, Yu Wei
    Modified CSWC() to set keep alive command parameters.
    Modified ClearVariables() to clear keep alive command buffer.
    Added KeepAlive() to send KeepAlive sgnal to SWC.
    Modified MainProcess() to send keep alive command.
    Modified ServerCommand() to process server keep alive command.
    Refer to PR:OASYS-PMS 0233.

  02  13 August 2004, Yu Wei
    Modified CSWC(). Don't set time stamp for SWC table when new object.
    Modified MainProcess(). Before polling, check link status first.
    Changed StandbyLinkCheck() to PrimaryLinkCheck(). The routine will
    check two link status.
    Modified FastPolling(). Checking primary link when two link was down
    will be done by PrimaryLinkCheck().
    Refer to PR:OASYS-PMS 0232.

Version: CMM E.1.0.2
  01  14 Apr 2005, Tong Zhi Xiong
    Remove "error code" log from ServerCommandError, refer to PR:OASYS-PMS 0240

Version: CMM E.1.0.4
  01  27 March 2006, Yu Wei
    Modified PrimaryLinkCheck(), changed m_nStandbyPollingFailedValue to
    m_nStandbyPollingValue to solve PSD standby link monitor breakdown.
    Refer to PR:OASYS-PMS 0248.
Version: SWC 1.0.0.2
  01  27 March 2013, Yang Tao
    Modified the LinkMonitor() module, (1)If recieve incomplete or exceptional data, will
    increment the exception counter and reactivate the timeout;(2)Added function code:83,
    8F and 90 as valid response from SWC.
    Refer to PR: C955 PR116

Version: SWC 1.0.0.3
  01  06 May 2013, Yang Tao
    Modified the LinkMonitor() module: 1)Extend the Receive Message timeout in case of
    reply delay from ECS. 2)Put the invalidfiledescriptor scenario in the exception error
    instead of link error.

    Modified the FastPolling, SlowPolling, TimerPolling. KeepAlive module: Categorize the
    result of RequestandReceiveWithModbusCheck function into 3 categories: Success, Link Error,
    Exceptional Error. Unify the exception counter to m_nFastExcepCounter.

    Remove the unused Function: CSWC::MMMWriteSWCTable


  Name          Date          Remark
  ----          ----          ------
 Bryan Chong  22-Apr-2010  Time sync command not initialize properly. Missing
                           slave address.
                           Change tSWCPara.tHeader.tLinkPara[nJ].cSlaveAddress
                           to tSWCPara.tHeader.tProtocol[0].addr.
                           Resolve [PR34] No Time sync command.
 Bryan Chong  23-Apr-2010  Implementation of SER_SendWithReply routine resolved
                           health status update in RMT [PR35]
 Bryan Chong  28-Apr-2010  Implementation of SER_SendWithReply routine resolved
                           ECS link switch with correct polling time [PR36]
 Yang Tao     02-Apr-2013  Update Link Healthy status judgment method of Standby
                           RTU resolved the ECS Standby link healthy status toggle
                           issue :(1)If recieve incomplete or exceptional data, will
                           increment the exception counter and reactivate the time-
                           -out; (2)  Added function code:83, 8F and 90 as valid
                           response from SWC. [C955 PR116]

 Yang Tao     09-Sep-2013  Improved the link monitor function, to solve the Standby
						   RTU standby link toggle issue. And if the link is down, it
						   will spend less 30 seconds to detect link down.
						   Add in two delay before and after call Sendwithreply
						   to reduce Modbus CRC error.

*******************************************************************************/
#include <sys/neutrino.h>
#include <time.h>
#include <termios.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream.h>
#include <mqueue.h>
#include <sys/dcmd_chr.h>
#include <pthread.h>
#include <sys/netmgr.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "err_ext.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
//#include "flg_def.h"
//#include "flg_ext.h"
#include "sys_ass.h"
#include "ser_def.h"
#include "ser_ext.h"
#include "cgf_def.h"
#include "cgf_ext.h"
#include "ntp_def.h"
#include "ntp_ext.h"
#include "str_def.h"
#include "str_ext.h"
#include "rmm_def.h"
#include "mdb_def.h"


#include "Common.h"
#include "CMM_Timer.h"
#include "CMM_Listen.h"
#include "SWC.h"
#include "CMM_Log.h"

#include "RMM.h"
#include "RMM_RTULink.h"
#include "rmm_ext.h"

#include "CMM_Modbus.h"
#include "CMM_Watchdog.h"
#include "Common_qnxWatchDog.h"

// Terminate SWC flag to exit application
BOOL_T   bSWCExit;
CSWC *g_apSWCObject[SWC_MAX_NUMBER];      //Object for all SWC.

VOID *SWC_MgmtThread(VOID *nindex);
VOID *SWC_ServerListeningThread(VOID *nIndex);
VOID *SWC_NTPClientCommunicationThrd(VOID *pThreadParameter);

static E_ERR_T SWC_InitNTPClient(VOID);

//#ifdef NOT_USED
static int swc_setSerialAttr(int fd , speed_t BaudRate ,unsigned int ucParity ,
                             unsigned int DataBits ,unsigned  int StopBits);
//#endif // NOT_USED

/*******************************************************************************
Purpose
  CSWC constructor. It initialize SWC parameter.

Input/Output
  tSWCPara     [in] SWC parameter structure.
  nDeviceID    [in] SWC index number.
  acIPAddress  [in] RTU LAN1 IP address.

Return

  None

History

  Name          Date          Remark
  ----          ----          ------
 Bryan Chong  26-Nov-2009  Update
                           tSWCPara.tHeader.tLinkPara[nJ].cSlaveAddress to
                           tSWCPara.tHeader.tProtocol[0].addr, and
                           tSWCPara.tHeader.tLinkPara[nJ].cModbusReadCommand to
                           tSWCPara.tHeader.tProtocol[0].command in
                           SetReadCommand and SetWriteCommand

*******************************************************************************/

CSWC::CSWC(struct tSWCCFGStructure tSWCPara,int nDeviceID, char *acIPAddress)
{
  int nI,nJ;
  char acTemp[200];
  tModbusAddress tTempAdr;
  pthread_mutexattr_t *attrm = NULL;
  pthread_condattr_t *attrc = NULL;

  ClearVariables();  //040323 Yu Wei

  m_bCreateObjectResult = true;  //040309 Yu Wei
  m_bTaskWorking = true;

  m_bPollingEnable = true;  //Default the polling enable.

  //Set interface type from other module.
  m_nLinkType = tSWCPara.tHeader.nLinkType;

  attrm = (pthread_mutexattr_t *) malloc(sizeof(pthread_mutexattr_t));
  attrc = (pthread_condattr_t *) malloc(sizeof(pthread_condattr_t));
  pthread_mutexattr_init( attrm );
  pthread_condattr_init( attrc );
  pthread_mutex_init(& m_tAccessTableSemID, attrm );
  pthread_mutex_init(& SWC_main_wd_mutex, attrm ); //for watchdog
  pthread_cond_init(& SWC_main_wd_cond, attrc );//for watchdog
  free(attrm);
  free(attrc);

  memset(m_messageQname , 0 ,32);
  strncpy(m_messageQname, SWC_MSGQ, strlen(SWC_MSGQ));
  strcat(m_messageQname , tSWCPara.tHeader.acName);

  //each messageQ need a specific name
  //  m_tMSGQID = mq_open(SWC_MSGQ,
  //    O_RDWR|O_CREAT|O_NONBLOCK, S_IRUSR|S_IWUSR, NULL );
  m_tMSGQID = mq_open(m_messageQname,
    O_RDWR|O_CREAT|O_NONBLOCK, S_IRUSR|S_IWUSR, NULL );

  if(m_tMSGQID == TYP_ERROR)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SWC] CSWC::CSWC, create msgq failed, m_tMSGQID %d\n");
    #endif // CFG_PRN_ERR
    return;
  }

  strcpy(m_acSWCName,tSWCPara.tHeader.acName);  //SWC name.
  m_nSWCID = nDeviceID;              //SWC index.
  m_ucSWCAddress = tSWCPara.tHeader.ucID;      //SWC address.
  m_nSWCCurrentStateFlag = STATEFLAG_INITIALIZATION;

  //Set serial link and polling command buffer.
  m_nTotalLinkNumber = 0;

  if((tSWCPara.tHeader.nSendTimeSynHour < 24) &&
     (tSWCPara.tHeader.nSendTimeSynMinute <60))
  {
    m_bTimeSynFlag = true;
    m_bTimeSynStartFlag = true;
    m_nSendTimeSynHour = tSWCPara.tHeader.nSendTimeSynHour;
    m_nSendTimeSynMinute = tSWCPara.tHeader.nSendTimeSynMinute;
    ActivateTimeOut(TIME_SYN_CHECK);
  }
  else
  {
    m_bTimeSynFlag = false;
    m_bTimeSynStartFlag = false;
  }

  //Check keep alive command.  //040812 Yu Wei
  if(tSWCPara.tHeader.nKeepAliveInterval != 0)
  {

    m_bKeepAliveFlag = true;
    m_nKeepAliveInterval = tSWCPara.tHeader.nKeepAliveInterval;
    m_unSendKeepAliveAddr = tSWCPara.tHeader.unSendKeepAliveAddr;
    m_unKeepAliveCmdWord = 0x0000;        //default is 00;
  }
  else
  {
    m_bKeepAliveFlag = false;
    m_nKeepAliveInterval = 0;
    m_unKeepAliveCmdWord = 0x0000;
  }
  for(nI=0,nJ=0; nJ<2; nJ++)
  {
    if(tSWCPara.tHeader.tLinkPara[nJ].unPortID != 0xFFFF)
    {
      m_nTotalLinkNumber++;

      // Not second multidrop link SWC
      if( g_tRTUConfig.anMultidorpSWCChain[m_nSWCID] > m_nSWCID )
      {
        // CAUTION: avoid using blocking. Blocking may cause read to be
        //   suspended unless has gone through thorough testing using
        //   throughput test from SCPI command
        //m_anCommfd[nI] = pSER_CB[tSWCPara.tHeader.tLinkPara[nJ].unPortID+1].fd;
        m_anCommfd[nI] = pSER_CB[tSWCPara.tHeader.tLinkPara[nJ].unPortID+1].fd;
        m_anCommPortId[nI] = tSWCPara.tHeader.tLinkPara[nJ].unPortID + 1;
        //Open port.
        //open(SER_acSerialPortName[tSWCPara.tHeader.tLinkPara[nJ].unPortID],
             //O_RDWR | O_NONBLOCK);
               //O_RDWR);

        #ifdef CFG_DEBUG_MSG
        printf("[SWC] CSWC::CSWC, %s is using com %d, fd %d\n",
          m_acSWCName, (tSWCPara.tHeader.tLinkPara[nJ].unPortID + 1),
          m_anCommfd[nI]);
        #endif // CFG_DEBUG_MSG


        if (m_anCommfd[nI] == ERROR)
        {
          sprintf(acTemp,"%s Open port %d failed\n",m_acSWCName,nI);
          #ifdef CFG_PRN_ERR
          printf("ERR  [SWC] CSWC::CSWC, fail to open port %d (ser%d) for %s\n",
            nI, tSWCPara.tHeader.tLinkPara[nJ].unPortID, m_acSWCName);
          #endif // CFG_PRN_ERR
          g_pEventLog->LogMessage(acTemp);
//         m_bCreateObjectResult = false;  //040309 Yu Wei
        }
        else
        {
          #ifdef CFG_DEBUG_MSG
          printf("[SWC] CSWC::CSWC, %s open link %d, com %d, fd %d is "
                 "successful\n",
            m_acSWCName, nI, (tSWCPara.tHeader.tLinkPara[nJ].unPortID + 1),
            m_anCommfd[nI]);
          #endif // CFG_DEBUG_MSG
// 20140602: Enable the below function, can config SWC from config file
          int data = _CTL_DTR_CHG |0;
          swc_setSerialAttr(m_anCommfd[nI],
                   tSWCPara.tHeader.tLinkPara[nJ].nBaudRate,
                   tSWCPara.tHeader.tLinkPara[nJ].nParity ,
                   tSWCPara.tHeader.tLinkPara[nJ].nDataBit,
                   tSWCPara.tHeader.tLinkPara[nJ].nStopBit  );
          devctl(m_anCommfd[nI] ,DCMD_CHR_SERCTL, &data , sizeof(data), NULL);
          //clear the input and output buffer
          tcflush(m_anCommfd[nI],TCIOFLUSH);

          SER_SetAttribute((E_SER_COM)m_anCommPortId[nI],
            tSWCPara.tHeader.tLinkPara[nJ].nBaudRate,
            (E_SER_PARITY) tSWCPara.tHeader.tLinkPara[nJ].nParity,
            (E_SER_DATABIT) tSWCPara.tHeader.tLinkPara[nJ].nDataBit,
            (E_SER_STOPBIT) tSWCPara.tHeader.tLinkPara[nJ].nStopBit);

        }
      }

      //Fast polling command.
      SetReadCommand(  m_acFastPollingCommand[nI],
              tSWCPara.tHeader.tProtocol[nJ].addr,
              tSWCPara.tHeader.tProtocol[nJ].command,
              tSWCPara.tPollingAddress.tFastPollingAddress);

      //Slow polling command.
      SetReadCommand( m_acSlowPollingCommand[nI],
              tSWCPara.tHeader.tProtocol[nJ].addr,
              tSWCPara.tHeader.tProtocol[nJ].command,
              tSWCPara.tPollingAddress.tSlowPollingAddress);

      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
      printf("[SWC] CSWC::CSWC, slow polling command slave addr = 0x%04x, "
              "modbus fn code = 0x%04x\n",
             //g_tRTUConfig.tSWC[nSWCIndex].tHeader.tLinkPara[0].cSlaveAddress,
             tSWCPara.tHeader.tProtocol[nJ].addr,
             tSWCPara.tHeader.tProtocol[nJ].command
             //tTempAdr.unStart
             );
      printf("[SWC] CSWC::CSWC, setup header for slow polling command "
             "m_acStatusCommand[%d] =\n", nI);
      SYS_PrnDataBlock((const UINT8 *)m_acSlowPollingCommand[nI], 8, 10);
      #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SWC))

      tTempAdr.unStart = tSWCPara.tPollingAddress.tFastPollingAddress.unStart;
      tTempAdr.unLength = 0x0001;

      //Status polling command.
      SetReadCommand( m_acStatusCommand[nI],
              tSWCPara.tHeader.tProtocol[nJ].addr,
              tSWCPara.tHeader.tProtocol[nJ].command,
              tTempAdr);

      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
      printf("[SWC] CSWC::CSWC, setup header for status cmd "
             "m_acStatusCommand[%d] =\n", nI);
      SYS_PrnDataBlock((const UINT8 *)m_acStatusCommand[nI], 8, 10);
      #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SWC))

      //Timer polling command.
      SetReadCommand( m_acTimerPollingCommand[nI],
              tSWCPara.tHeader.tProtocol[nJ].addr,
              tSWCPara.tHeader.tProtocol[nJ].command,
              tSWCPara.tPollingAddress.tTimerAddress);

      //Set time syn command.
      if(m_bTimeSynFlag == true)
      {
        acTemp[0] = 0x01;
        tTempAdr.unStart = tSWCPara.tHeader.unSendTimeSynStartAddr;
        tTempAdr.unLength = tSWCPara.tHeader.unSendTimeSynLength;
        memset(m_acTimeSynCommand[nI], 0, sizeof(m_acTimeSynCommand[nI]));
        m_nTimeSynCMDLength = SetWriteCommand(m_acTimeSynCommand[nI],
                                acTemp,
                                tSWCPara.tHeader.tProtocol[nJ].addr,
                                0x10,
                                tTempAdr);
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_TIMESYNC))
        printf("[SWC] CSWC::CSWC, m_acTimeSynCommand[%d], command length = %d, "
               "\n  total buffer size = %d. Preset command packet:\n",
                nI, m_nTimeSynCMDLength, sizeof(m_acTimeSynCommand[nI]));
        SYS_PrnDataBlock((const UINT8 *)m_acTimeSynCommand[nI], 19, 10);
        #endif  // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_TIMESYNC))
      }

      //Set keep alive command.  //040812 Yu Wei
      if(m_bKeepAliveFlag == true)
      {
        *(unsigned short *)&acTemp[0] =STR_ByteSwap16Bit( m_unKeepAliveCmdWord);
        tTempAdr.unStart = m_unSendKeepAliveAddr;
        tTempAdr.unLength = 0x0001;
        m_nKeepAliveCMDLength = SetWriteCommand( m_acKeepAliveCommand[nI],
                             acTemp,
                             tSWCPara.tHeader.tProtocol[nJ].addr,
                             0x10,
                             tTempAdr);
      }

      m_anStandbyRecevLen[nI] = 0;    //Clear standby received data length.

      m_atPollingFlag[nI].bFast = false;
      m_atPollingFlag[nI].bSlow = false;
      m_atPollingFlag[nI].bStatus = false;
      m_atPollingFlag[nI].bSpecial = false;
      m_atPollingFlag[nI].bLinkCheck = false;
      m_atPollingFlag[nI].bTimeSyn = false;
      m_anLinkStatus[nI] = LINKERROR;
      nI++;
    }
    else
    {
      m_anLinkStatus[nJ] = NOLINK;
    }

  }

  #ifdef CFG_DEBUG_MSG
  printf("[SWC] CSWC::CSWC, m_nTotalLinkNumber = %d\n", m_nTotalLinkNumber);
  #endif // CFG_DEBUG_MSG


  //Set primary link port ID.
  switch(m_nTotalLinkNumber)
  {
  case 1:
    m_nPrimaryPortID = 0;
    m_nStandbyPortID = 0;
    break;
  case 2:
    m_nPrimaryPortID = 0;
    m_nStandbyPortID = 1;
    break;
  default:
    m_bCreateObjectResult = false;  //040309 Yu Wei
    break;
  }

  m_unCurrentLinkStatus = 0;  //Serial link status.

  //Polling parameter.
  m_nReceiveTimeout      = tSWCPara.tHeader.nReceiveTimeout;
  m_nFastPollingValue      = tSWCPara.tHeader.nFastPollingFrequency;
  m_nSlowPollingValue      = tSWCPara.tHeader.nSlowPollingFrequency;
  m_nRetryNumber        = tSWCPara.tHeader.nRetryNumber;
  m_nStandbyPollingValue    = tSWCPara.tHeader.nStandbyPollingFrequency;
  m_nStandbyPollingFailedValue= tSWCPara.tHeader.nFailedPollingFrequency;

  //Fast polling table init
  m_nFastPollingRecevLen = tSWCPara.tPollingAddress.tFastPollingAddress.
                                unLength*2+5;
  m_nFastDataLen = tSWCPara.tPollingAddress.tFastPollingAddress.unLength;
  m_nServerFastTableStartAddr = tSWCPara.tPollingAddress.tFastPollingAddress.
                                  unServerStart;
  m_nServerFastTableLen = tSWCPara.tPollingAddress.tFastPollingAddress.
                            unServerLength;
  //040813 Yu Wei
  if(m_nFastDataLen == 0)
  {
    //040309 Yu Wei
    m_bCreateObjectResult = false;
    #ifdef CFG_PRN_ERR
    printf("ERR  [SWC] CSWC::CSWC, create object fail. "
           "m_nFastPollingRecevLen = %d, m_nFastDataLen = %d, "
           "m_nServerFastTableStartAddr = 0x%x, m_nServerFastTableLen = %d\n",
      m_nFastPollingRecevLen, m_nFastDataLen, m_nServerFastTableStartAddr,
      m_nServerFastTableLen );
    #endif // CFG_PRN_ERR
  }

  //Slow polling table init
  m_nSlowPollingRecevLen =
    tSWCPara.tPollingAddress.tSlowPollingAddress.unLength*2+5;
  m_nSlowDataLen = tSWCPara.tPollingAddress.tSlowPollingAddress.unLength;
  m_nServerSlowTableStartAddr =
    tSWCPara.tPollingAddress.tSlowPollingAddress.unServerStart;
  m_nServerSlowTableLen =
    tSWCPara.tPollingAddress.tSlowPollingAddress.unServerLength;
  m_bSlowPollingStart = true;    //Startup, slow polling will do. 040312 Yu Wei

  if(m_nSlowDataLen != 0)
  {
    m_bSlowPollingFlag = true;
  }
  else
    m_bSlowPollingFlag = false;

  //Timer polling table init
  m_nTimerPollingRecevLen = tSWCPara.tPollingAddress.tTimerAddress.unLength*2+5;
  m_nTimerDataLen = tSWCPara.tPollingAddress.tTimerAddress.unLength;
  m_nServerTimerTableStartAddr =
    tSWCPara.tPollingAddress.tTimerAddress.unServerStart;
  m_nServerTimerTableLen =
    tSWCPara.tPollingAddress.tTimerAddress.unServerLength;
  m_nTimerPollingValue = tSWCPara.tPollingAddress.nTimerInterval;
  //Startup, timer polling will do. 040312 Yu Wei
  m_bTimerPollingStart = true;
  if(m_nTimerDataLen != 0)
  {
    m_bTimerPollingFlag = true;
  }
  else
    m_bTimerPollingFlag = false;


  m_nStatusPollingRecevLen = 7;

  //Initialize socket listen object.
  m_pServerSocket = new CListen(
    acIPAddress,
    tSWCPara.tHeader.nSocketID,
    tSWCPara.tHeader.nLANLinkTimeout);

  #ifdef CFG_DEBUG_MSG
  printf("[SWC] CSWC::CSWC, CListen for %s m_pServerSocket = 0x%08x\n",
         acIPAddress, m_pServerSocket);
  #endif // CFG_DEBUG_MSG

  //Reset timeout counter.
  for(nI=0; nI<MAX_TIMEOUT_TYPE; nI++)
  {
    m_atTimeOut[nI].Low = 0;    //040520 Yu Wei
    m_atTimeOut[nI].High = 0;    //040520 Yu Wei
  }

  //Set standby RTU monitor link timeout type ID.
  m_anStandbyMonitorTimeout[0] = STANDBY_RTU_LINK1;
  m_anStandbyMonitorTimeout[1] = STANDBY_RTU_LINK2;

  m_bLinkCheckFlag = false;

  m_unFastPollingStartAdd =
    tSWCPara.tPollingAddress.tFastPollingAddress.unStart;
  m_unSlowPollingStartAdd =
    tSWCPara.tPollingAddress.tSlowPollingAddress.unStart;
  //040322 Yu Wei
  m_unTimerPollingStartAdd= tSWCPara.tPollingAddress.tTimerAddress.unStart;

  SWC_main_timer_id = wdCreate(WatchDogChid , SWC_main_timer_id , SWC_MAIN,
                               nDeviceID);

  if(SWC_main_timer_id == -1 )
  {
    sprintf(acTemp,"%s Create watchdog ID error.\n",m_acSWCName);
    g_pEventLog->LogMessage(acTemp);
    m_bCreateObjectResult = false;  //040309 Yu Wei

//    #ifdef CFG_PRN_ERR
//    printf("ERR  [SWC] CSWC::CSWC, create watchdog fail\n");
//    #endif // CFG_PRN_ERR
  }

  m_nSpecificCommandLen = 0;    //040302 Yu Wei

  m_bStartupCheckLink = true;    //Startup, check link.  //040520 Yu Wei

  //040707 Yu Wei
  m_nSWCBusyTimeout = tSWCPara.tHeader.nSWCBusyTimeout;
  m_nExcepTimeSyncRetry = tSWCPara.tHeader.nExcepTimeSyncRetry;
  m_nTimeSyncExcepCounter = 0;

  m_nExcepRetry = tSWCPara.tHeader.nExcepRetry;
  m_nFastExcepCounter = 0;
  m_nSlowExcepCounter = 0;
  m_nTimerExcepCounter = 0;
  //040707 Yu Wei

  m_bCheckingBeforeSwitch = false;//20050518

  #ifdef ModuleTest
  LogSwcInitValues(m_acSWCName);    //for test.
  #endif // ModuleTest
} // CSWC::CSWC
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  CSWC::CSWC

 DESCRIPTION

  This constructor is designated for the specific type.

 CALLED BY

  SWCInitialization  SWC initialization

 CALLS

  SWC_InitNTPClient  NTP client initialization

 PARAMETER

  swc_type   [in] SWC initialization type

 RETURN

  None

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                REMARKS

   Bryan Chong    21-May-2010      Create initial version
   Bryan Chong    17-Jun-2010      Fix NTP exception error update [PR51]

-----------------------------------------------------------------------------*/
CSWC::CSWC(E_SWC_T swc_type)
{
  E_ERR_T rstatus = E_ERR_Success;

  #ifdef CFG_PRN_ERR
  CHAR errBuff[100] = {0};
  #endif // CFG_PRN_ERR

  switch(swc_type)
  {
    case E_SWC_T_NTP_Client:
      ClearVariables();

      // [PR51]
      m_nExcepRetry = 3;
      m_nFastExcepCounter = 0;
      m_nSlowExcepCounter = 0;
      m_nTimerExcepCounter = 0;

      rstatus = SWC_InitNTPClient();
      if(rstatus != E_ERR_Success)
    {
        #ifdef CFG_PRN_ERR
       ERR_GetMsgString(rstatus, errBuff);
       printf("ERR  [SWC] CSWC::CSWC, initialize NTP client fail, %s\n",
             errBuff);
        #endif // CFG_PRN_ERR

      }
      break;

    default:
      #ifdef CFG_PRN_ERR
      printf("ERR  [SWC] CSWC::CSWC, invalid SWC type, %d\n", swc_type);
      #endif // CFG_PRN_ERR
      break;
  }
} // CSWC::CSWC

CSWC::CSWC(){}

/**********************************************************************************
Purpose:
  CSWC destructor. Stop SWC task and delete socket listen object.

***********************************************************************************/
CSWC::~CSWC()
{
  int nI;
  if(SWC_main_timer_id != NULL)
  {
    timer_delete(SWC_main_timer_id);
  }
  for (nI=0; nI<m_nTotalLinkNumber; nI++)
  {
    if (m_anCommfd[nI] != ERROR)
    {
      close(m_anCommfd[nI]);
    }
  }
  delete m_pServerSocket;

  //delete semaphore
  pthread_mutex_destroy(&m_tAccessTableSemID);

  //Delete message Queue
  if(m_tMSGQID > 0 )
  {
    mq_unlink( m_messageQname );
  }
}

/**********************************************************************************
Purpose:
  Clear object variables.

History:

  Name            Date         Remark
  ----            ----         ------
 Bryan Chong   02-Feb-2010   Add m_usTempStatus for debugging purpose only

***********************************************************************************/
void CSWC::ClearVariables(void)
{
  memset ( m_acSWCName, 0x00, sizeof(char)*10);
  m_nSWCID = 0;
  m_nReceiveTimeout = 0;
  m_nMainTaskID = 0;
  m_nServerSocketTaskID = 0;
  m_bTaskWorking = false;
  m_nLinkType = 0;
  m_bPollingEnable = false;
  memset(m_anCommfd, 0x00, sizeof(int)*2);
  memset (m_atPollingFlag, 0x00, sizeof(tPollingType) * 2);
  memset(m_anRetryNumberStatus, 0x00, sizeof(int)*2);
  m_nSWCCurrentStateFlag=0;

  memset(m_aunSWCTableFast, 0x00, sizeof(unsigned short)*2048);
  memset(m_aunSWCTableSlow, 0x00, sizeof(unsigned short)*2048);
  memset(m_aunSWCTableTimer, 0x00, sizeof(unsigned short)*2048);

  memset(m_anLinkStatus, 0x00, sizeof(int)*2);

  m_nMasterLinkID = 0;
  m_unCurrentLinkStatus = 0;
  m_tMSGQID = NULL;

  m_nFastPollingValue = 0;
  m_nSlowPollingValue = 0;
  m_nTimerPollingValue = 0;
  m_nRetryNumber = 0;
  m_nStandbyPollingValue = 0;
  m_nStandbyPollingFailedValue = 0;

  memset(m_acFastPollingCommand, 0x00, sizeof(char)*2*16);
  memset(m_acSlowPollingCommand, 0x00, sizeof(char)*2*16);
  memset(m_acTimerPollingCommand, 0x00, sizeof(char)*2*16);
  memset(m_acWriteCommand, 0x00, sizeof(char)*2048);
  memset(m_acStatusCommand, 0x00, sizeof(char)*2*16);

  m_bTimeSynFlag = false;
  m_bTimeSynStartFlag = false;

  m_nSendTimeSynHour = 0;
  m_nSendTimeSynMinute = 0;
  m_nTimeSynCMDLength = 0;
  memset(m_acTimeSynCommand, 0, sizeof(char)*2*264);

  //Clear keep alive command buffer.  //040812 Yu Wei
  m_nKeepAliveCMDLength = 0;
  memset(m_acKeepAliveCommand, 0, sizeof(char)*2*264);

  m_nFastPollingRecevLen = 0;
  m_nFastDataLen = 0;
  m_nServerFastTableStartAddr = 0;
  m_nServerFastTableLen = 0;
  m_nSlowPollingRecevLen = 0;
  m_nSlowDataLen = 0;
  m_nServerSlowTableStartAddr = 0;
  m_nServerSlowTableLen = 0;
  m_bSlowPollingFlag = false;
  m_bSlowPollingStart = false;
  m_nTimerPollingRecevLen = 0;
  m_nTimerDataLen = 0;
  m_nServerTimerTableStartAddr = 0;
  m_nServerTimerTableLen = 0;
  m_bTimerPollingFlag = false;
  m_bTimerPollingStart = false;
  m_nStatusPollingRecevLen = 0;
  m_nSpecificCommandLen = 0;
  memset(m_acMultidropReceData, 0x00, sizeof(char)*264);
  m_nMultidropReceDataLen = 0;
  m_ucSWCAddress = 0;
//  m_tAccessTableSemID = NULL;
  pthread_mutex_destroy(&m_tAccessTableSemID);

  memset(m_atTimeOut, 0x00, sizeof(timespec)*MAX_TIMEOUT_TYPE);
  m_nTotalLinkNumber = 0;
  m_nPrimaryPortID = 0;
  m_nStandbyPortID = 0;
  memset(m_acStandbyRecev, 0x00, sizeof(char)*2*1024);
  memset(m_anStandbyRecevLen, 0x00, sizeof(int)*2);
  memset(m_anStandbyMonitorTimeout, 0x00, sizeof(int)*2);
  m_bLinkCheckFlag = false;
  m_unFastPollingStartAdd = 0;
  m_unSlowPollingStartAdd = 0;
  m_unTimerPollingStartAdd = 0;
//  m_tWatchdogID = NULL;
  m_pServerSocket = NULL;

  // Store status for debugging use only
  m_usTempStatus = 0;

}

/**********************************************************************************
Purpose:
  Set Modbus read command buffer.

Input/Output
  acCommand    -- Modbus read command buffer. (out)
  cSlaveAdr    -- Modbus slave address. (in)
  cCommandCode  -- Modbus read command code. (in)
  tAddress    -- Modbus address structure. (in)

Return
  Nil.
***********************************************************************************/
void CSWC::SetReadCommand(char *acCommand,char cSlaveAdr, char cCommandCode, tModbusAddress tAddress)
{
  acCommand[0] = cSlaveAdr;
  acCommand[1] = cCommandCode;
  *(unsigned short *)&acCommand[2] = STR_ByteSwap16Bit(tAddress.unStart);
  *(unsigned short *)&acCommand[4] = STR_ByteSwap16Bit(tAddress.unLength);
  *(unsigned short *)&acCommand[6] = STR_ByteSwap16Bit(ModbusCRC((unsigned char *)acCommand,6));
}

/**********************************************************************************
Purpose:
  Set Modbus write command buffer.

Input/Output
  acCommand    -- Modbus read command buffer. (out)
  aunWriteData  -- The buffer for write data. (in)
  cSlaveAdr    -- Modbus slave address. (in)
  cCommandCode  -- Modbus write command code. (in)
  tAddress    -- Modbus address structure. (in)

Return
  Write command data size.
***********************************************************************************/
int CSWC::SetWriteCommand(
  char *acCommand,
  char *aunWriteData,
  char cSlaveAdr,
  char cCommandCode,
  tModbusAddress tAddress)
{
  unsigned char ucDataLength;
  unsigned char nI;

//  ucDataLength = (char)(STR_ByteSwap16Bit(tAddress.unLength)); assigen value to 1 byte do not need to swap byte order
  ucDataLength = (tAddress.unLength);
// Keep investigate Remove
  if(cCommandCode == 0x10)      //Write word.
    ucDataLength *= 2;
// Yang Tao 20130506: cCommandCode 0x0F will not be used in this function.

/*  else if(cCommandCode == 0x0F)    //Write bit.
  {
    if((ucDataLength & 0x07) ==0)
      ucDataLength >>= 3;      //divid 8;
    else
    {
      ucDataLength >>= 3;      //divid 8;
      ucDataLength ++;
    }
  }
*/
  acCommand[0] = cSlaveAdr;
  acCommand[1] = cCommandCode;
  *(unsigned short *)&acCommand[2] = STR_ByteSwap16Bit(tAddress.unStart);
  *(unsigned short *)&acCommand[4] = STR_ByteSwap16Bit(tAddress.unLength);


  acCommand[6] = ucDataLength;

  for(nI=0; nI<ucDataLength; nI++)
  {
    acCommand[7+nI] = aunWriteData[nI];
  }
  *(unsigned short *)&acCommand[7+nI] =
    STR_ByteSwap16Bit(ModbusCRC((unsigned char *)acCommand,7+nI));
  return (9+nI);
}

/**********************************************************************************
Purpose:
  Update Modbus write command data buffer.

Input
  cData      -- The buffer for write data. (in)
  nCommandLen    -- Write data size. (in)
  cSlaveAdr    -- Modbus slave address. (in)

Return
  Write command data length.
***********************************************************************************/
void CSWC::UpdateCommand(char *cData,int nCommandLen, char cSlaveAddress)
{
  int nI;

  memset(m_acWriteCommand, 0, sizeof(m_acWriteCommand));

  m_acWriteCommand[0] = cSlaveAddress;

  for(nI=1; nI<nCommandLen; nI++)
    m_acWriteCommand[nI] = cData[nI];

  *(unsigned short *)&m_acWriteCommand[nCommandLen-2] =
    STR_ByteSwap16Bit(ModbusCRC((unsigned char *)m_acWriteCommand,nCommandLen-2));
  m_nSpecificCommandLen = nCommandLen;
}

/**********************************************************************************
Purpose:
  Write SWC table.

Input/Output
  acSWCTable    -- The buffer for SWC table. (out)
             Fast, Slow and Timer table.
  acSWCData    -- Polling data. (in)
  nDataLen    -- Data size. (in)

Return
  Nil.
***********************************************************************************/
void CSWC::WriteSWCTable(unsigned short *acSWCTable, char *acSWCData,
                         int nDataLen)
{
  int nI;
  struct timespec tTime;

  clock_gettime(CLOCK_REALTIME, &tTime);

  //use local time
  pthread_mutex_lock( & m_tAccessTableSemID);

  //Write time stamp.
  acSWCTable[0] = (unsigned short)(tTime.tv_sec >> 16);
  acSWCTable[1] = (unsigned short)(tTime.tv_sec & 0x0000FFFF );
  acSWCTable[2] = (unsigned short)(tTime.tv_nsec/1000000);

  //Polling data.
  for(nI=0; nI<nDataLen; nI++)
  {
    // Form a 16-bit word data
    acSWCTable[nI+3] =
      (((((unsigned short)acSWCData[nI*2]) & 0x00FF )<<8) |
         ((unsigned short)acSWCData[nI*2+1])&0x00FF);
  }

  pthread_mutex_unlock( & m_tAccessTableSemID);

  #if ((defined CFG_DEBUG_MSG) && (_CFG_DEBUG_SWC))
  //printf("[SWC] CSWC::WriteSWCTable, timestamp acSWCTable[0..2] = 0x%04x %04x "
    //     "%04x\n", acSWCTable[0],acSWCTable[1],acSWCTable[2]);
  printf("[SWC] CSWC::WriteSWCTable, write %d words to SWC table\n",
          nDataLen);
  rstatus = SYS_PrnDataBlock((const UINT8 *)acSWCTable, nDataLen, 10);
  #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SWC_LAN))
}

/**********************************************************************************
Purpose:
  Write primary RTU SWC table to standby RTU.
  Added in 23 March 2004, Yu Wei

Input/Output
  aunTable    -- The buffer for table data. (in)
  nTableID    -- Table ID. (in)
             0 -- DI, 1 -- DI8(FAST_TABLE),
             2 -- AI MI (SLOW_TABLE),
             3 -- MI (TIMER_TABLE).
  //nDataLen    -- Data size. (in)

Return
  Nil.
***********************************************************************************/
void CSWC::CopySWCTable(unsigned short *aunTable, unsigned short nTableID)
{
  int nI;

  if ( m_bPollingEnable == true )  //When link enable copy.
  {
    pthread_mutex_lock( & m_tAccessTableSemID);

    switch(nTableID)
    {
    case SWC_TABLE_DI:
    case SWC_TABLE_DI8:  //Fast polling always exsit.
      for(nI=0; nI< m_nServerFastTableLen; nI++)
      {
        m_aunSWCTableFast[nI] = STR_ByteSwap16Bit(aunTable[nI]);
      }
      break;

    case SWC_TABLE_AI_MI:
      for(nI=0; nI< m_nServerSlowTableLen; nI++)
      {
        m_aunSWCTableSlow[nI] = STR_ByteSwap16Bit(aunTable[nI]);
      }
      break;

    case SWC_TABLE_MI:
      for(nI=0; nI< m_nServerTimerTableLen; nI++)
      {
        m_aunSWCTableTimer[nI] = STR_ByteSwap16Bit(aunTable[nI]);
      }
      break;

    default:          //040310 Yu Wei
      break;
    }
    pthread_mutex_unlock( & m_tAccessTableSemID);
  }
}

/**********************************************************************************
Purpose:
  Copy SWC table to server table buffer.

Input/Output
  aunServerTable  -- The buffer for server table. (out)
  nTimeoutType  -- Semiphone timeout. (in)

Return
  OK    -- Transfer table successfully.
  ERROR  -- Semiphone timeout. The other module is accessing the table.

***********************************************************************************/
int CSWC::ReadSWCTable(unsigned short *aunServerTable, int nTimeoutType)
{
  int nI;

  if(pthread_mutex_lock( & m_tAccessTableSemID) >= 0)
  {
    for(nI=0; nI<m_nServerFastTableLen; nI++)
    {
       aunServerTable[nI + m_nServerFastTableStartAddr] =
         STR_ByteSwap16Bit(m_aunSWCTableFast[nI]);
    }


    //Transfer slow table
    if(m_bSlowPollingFlag == true)
    {
      for(nI=0; nI<m_nServerSlowTableLen; nI++)
      {
        aunServerTable[nI + m_nServerSlowTableStartAddr] =
          STR_ByteSwap16Bit( m_aunSWCTableSlow[nI]);
      }
    }

    //Transfer Timer table
    if(m_bTimerPollingFlag == true)
    {
      for(nI=0; nI<m_nServerTimerTableLen; nI++)
      {
        aunServerTable[nI + m_nServerTimerTableStartAddr] =
          STR_ByteSwap16Bit(m_aunSWCTableTimer[nI]);
      }
      #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_SWC_TIMERPOLLING)
      printf("[SWC] CSWC::ReadSWCTable, %s start add %d, for "
             "%d bytes:\n",
             m_acSWCName, m_nServerTimerTableStartAddr, m_nServerTimerTableLen);
      SYS_PrnDataBlock((const UINT8 *)
        &aunServerTable[m_nServerTimerTableStartAddr], m_nServerTimerTableLen,
        20);
      #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SPM))
    }

    pthread_mutex_unlock( & m_tAccessTableSemID);
    return OK;
  }

  return ERROR;
}
/*******************************************************************************
Purpose:
  Copy one SWC table DI-DI8 (FAST_TABLE) AI-MI (SLOW_TABLE), MI (TIMER_TABLE) to
  buffer.
  Re-design in 23 March 2004, Yu Wei

Input/Output
  aunTable    -- The buffer for table. (out)
  nTableID    -- Table ID. (in)
             0 -- DI, 1 -- DI8(FAST_TABLE),
             2 -- AI MI (SLOW_TABLE),
             3 -- MI (TIMER_TABLE).
  nTimeoutType  -- Semiphone timeout. (in)

Return
  0  -- Cannot read table or table is not existed.
  Table length in words.
*******************************************************************************/
int CSWC::ReadSWCOneTable(char *aunTable, unsigned short nTableID, int nTimeoutType)
{
  int nI;
  int nReturnValue = 0;  //Default value 0,

  if(pthread_mutex_lock(&m_tAccessTableSemID)>= 0)
  {
    switch(nTableID)
    {
    case SWC_TABLE_DI:
    case SWC_TABLE_DI8:  //Fast polling always exsit.
      for(nI=0; nI< m_nServerFastTableLen; nI++)  //040322 Yu Wei
      {
        *(unsigned short *)&aunTable[nI*2] = STR_ByteSwap16Bit(m_aunSWCTableFast[nI]);  //040322 Yu Wei
      }
      nReturnValue = m_nServerFastTableLen;  //040322 Yu Wei
      break;

    case SWC_TABLE_AI_MI:

      if(m_bSlowPollingFlag == true)
      {
        for(nI=0; nI< m_nServerSlowTableLen; nI++)  //040322 Yu Wei
        {
          *(unsigned short *)&aunTable[nI*2] = STR_ByteSwap16Bit(m_aunSWCTableSlow[nI]);
        }
        nReturnValue = m_nServerSlowTableLen;  //040322 Yu Wei
      }
      break;

    case SWC_TABLE_MI:
      if(m_bTimerPollingFlag == true)
      {
        for(nI=0; nI< m_nServerTimerTableLen; nI++)
        {
          *(unsigned short *)&aunTable[nI*2] = STR_ByteSwap16Bit(m_aunSWCTableTimer[nI]);  //040322 Yu Wei
        }
        nReturnValue = m_nServerTimerTableLen;  //040322 Yu Wei
      }
      break;

    default:          //040310 Yu Wei
      nReturnValue = 0;
      break;
    }
    pthread_mutex_unlock(&m_tAccessTableSemID);
  }

  return nReturnValue;    //040310 Yu Wei
}


/**********************************************************************************
Purpose:
  Write a word to SWC table for MMM.

Input/Output
  unAddress  -- Address in table. (in)
  unData    -- Data to be written. (in)

Return
  Table ID  -- FAST_TABLE, SLOW_TABLE, TIMER_TABLE.  //040331 Yu Wei
        -- 0xFFFF the table is not found.    //040331 Yu Wei
***********************************************************************************/
// Yang Tao 17042013: Removed From SWC.cpp
//int CSWC::MMMWriteSWCTable(unsigned short unAddress, unsigned short unData)  //040331 Yu Wei
//{
//  int nTableID = 0xFFFF;
//
//  //Determine which table will be writen.
//  //Is fast polling table.  //040322 Yu Wei
//  if((unAddress >= m_unFastPollingStartAdd) &&  //Server table has 3 word for time stamp. SWC table no time stamp.
//    (unAddress < (m_unFastPollingStartAdd + m_nServerFastTableLen - 3)))
//
//  {
//    nTableID = FAST_TABLE;
//  }
//
//  //Is slow polling table.  //040322 Yu Wei
//  else if((unAddress >= m_unSlowPollingStartAdd) &&  //Server table has 3 word for time stamp. SWC table no time stamp.
//    (unAddress < (m_unSlowPollingStartAdd + m_nServerSlowTableLen - 3)))
//  {
//    nTableID = SLOW_TABLE;
//  }
//
//  //Is timer polling table.  //040322 Yu Wei
//  else if((unAddress >= m_unTimerPollingStartAdd) &&  //Server table has 3 word for time stamp. SWC table no time stamp.
//    (unAddress < (m_unTimerPollingStartAdd + m_nServerTimerTableLen - 3)))
//  {
//    nTableID = TIMER_TABLE;
//  }
//
//  if(nTableID != 0xFFFF)  //040331 Yu Wei
//  {
//    pthread_mutex_lock(&m_tAccessTableSemID);
//
//    switch(nTableID)
//    {
//    case FAST_TABLE:
//      m_aunSWCTableFast[unAddress - m_unFastPollingStartAdd + 3] = unData;  //First 3 words is time stamp. 040322 Yu Wei
//      break;
//
//    case SLOW_TABLE:
//      m_aunSWCTableSlow[unAddress - m_unSlowPollingStartAdd + 3] = unData; //First 3 words is time stamp. 040322 Yu Wei
//      break;
//
//    case TIMER_TABLE:
//      m_aunSWCTableTimer[unAddress - m_unTimerPollingStartAdd +3] = unData; //First 3 words is time stamp. 040322 Yu Wei
//      break;
//
//    default:    //Do nothing    040310 Yu Wei
//      break;
//    }
//
//      pthread_mutex_unlock(&m_tAccessTableSemID);
//  }
//
//  return nTableID;  //040331 Yu Wei
//}

/*******************************************************************************
Purpose
  This routine will update fast polling table

Input
  None

Return
  None

History
    Name           Date          Remark
    ----           ----          ------
  Yu, Wei        12-Aug-2004   Initial revision
  Bryan Chong    26-Feb-2010   Implement SER_SendWithReply routine to replace
                               SendReply routine [PR30]
  Bryan Chong    28-Apr-2010   Update with RequestAndReceiveWithModbusCheck
                               routine
  Bryan Chong    02-Jul-2010   Update master link with primary link status to
                               resolve link role and link healthiness issue
                               from RMT [PR48]
  Bryan Chong    11-Jul-2010   Reset timer to eliminate extra polling when
                               No Reply is checked in SWC simulator
*******************************************************************************/
void CSWC::FastPolling(void)
{
  E_ERR_T rstatus = E_ERR_Success;
  char acRecevData[MDB_MAX_MODBUS_DATA_SZ + 5];
  char acErrMsg[100] = {0};
  int nrval = 0;
  char cLog[100];

  if(m_anLinkStatus[m_nPrimaryPortID] == LINKERROR)
    return;

  nrval = CheckTimeOut(FASTPOLLING, m_nFastPollingValue);

  if(nrval == false)
    return;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_FASTPOLLING)
  printf("[SWC] CSWC::FastPolling, %s fd %d tx command %d bytes, expected data size is %d\n",
         m_acSWCName, m_anCommfd[m_nPrimaryPortID], 8, m_nFastPollingRecevLen);
 #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))

  ActivateTimeOut(FASTPOLLING);
  rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nPrimaryPortID],
    (UINT8 *)m_acFastPollingCommand[m_nPrimaryPortID], 8, acRecevData,
    (UINT32)m_nFastPollingRecevLen, m_nReceiveTimeout,
    (UINT8)m_nRetryNumber, (UINT8)m_nExcepRetry, (UINT8 *)&m_nFastExcepCounter);
	ERR_GetMsgString(rstatus, acErrMsg);

	//20150603 Su, added log
	if(rstatus != E_ERR_Success)
	{
	printf("ERR [SWC] CSWC::FastPolling, %s, rstatus: %s\n", m_acSWCName, acErrMsg);
	sprintf(cLog, "ERR [SWC] CSWC::FastPolling, %s, rstatus: %s", m_acSWCName, acErrMsg);
	g_pDebugLog->LogMessage(cLog);
	}

  switch(rstatus)
  {
    case E_ERR_Success:

      if(m_anLinkStatus[m_nPrimaryPortID] == LINKERROR)
      {
        m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
        m_nMasterLinkID = m_nPrimaryPortID;
      }
      m_nFastExcepCounter = 0;
      WriteSWCTable(m_aunSWCTableFast,&acRecevData[3], m_nFastDataLen);
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_FASTPOLLING)
      printf("[SWC] CSWC::FastPolling, %s rx %d bytes:\n",
             m_acSWCName, m_nFastPollingRecevLen);
      SYS_PrnDataBlock((const UINT8 *)acRecevData, m_nFastPollingRecevLen, 20);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
      return;
      break;

 // This group case belongs to exception error, and the link will be set OK.
    case E_ERR_SWC_ModbusExceptionError:
    case E_ERR_SER_SelectReadError:
    case E_ERR_SER_InvalidSelectReturnValue:
    case E_ERR_SWC_ModbusCRCError:
      m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
      m_nFastExcepCounter++;
      // Yang Tao 20150522, According to the request from SBST, exceesive exceptional error
      // will cause the link down, and
     if( m_nFastExcepCounter >= m_nExcepRetry)
     {
    	  m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
    	   m_nFastExcepCounter=0;
    	   ERR_GetMsgString(rstatus, acErrMsg);
    	   printf(cLog,"WARN [SWC] CSWC::FastPolling, %s fd %d\n"
   	            " rx Exceptional Error exceed exception limit cause link down %d bytes, Error %s:\n",
   	       m_acSWCName, m_anCommfd[m_nPrimaryPortID],m_nFastPollingRecevLen,acErrMsg);

    	   sprintf(cLog,"WARN [SWC] CSWC::FastPolling, %s fd %d\n"
    	            " rx Exceptional Error exceed exception limit cause link down %d bytes, Error %s:\n",
    	    m_acSWCName, m_anCommfd[m_nPrimaryPortID],m_nFastPollingRecevLen,acErrMsg);

    	    g_pDebugLog->LogMessage(cLog);
     }
     else
     {
    	 // m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
           ERR_GetMsgString(rstatus, acErrMsg);
           printf("WARN [SWC] CSWC::FastPolling, %s fd %d\n"
                   "  rx Exceptional Error %d bytes, Error %s, Exception error counter %d:\n",
                   m_acSWCName, m_anCommfd[m_nPrimaryPortID],m_nFastPollingRecevLen,acErrMsg,m_nFastExcepCounter);

          sprintf(cLog,"WARN [SWC] CSWC::FastPolling, %s fd %d\n"
                 "  rx Exceptional Error %d bytes, Error type is %s, Exception error counter %d:\n",
                 m_acSWCName, m_anCommfd[m_nPrimaryPortID],m_nFastPollingRecevLen,acErrMsg,m_nFastExcepCounter);

        	g_pDebugLog->LogMessage(cLog);
     }
      break;

//  Only in this case the RTU fail to receive the fast polling reply
    case E_ERR_SWC_ExceedRetryLimit:
         m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
         // Yang Tao Add in the log message
         sprintf(cLog, "%s serial link( %d ) breakdown due to Timeout\n",
       		  m_acSWCName, m_nPrimaryPortID);
         g_pDebugLog->LogMessage(cLog);

      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_FASTPOLLING)
      printf("[SWC] CSWC::FastPolling, %s exceed retry limit. "
             "Set link = LINKERROR \n",
             m_acSWCName, m_nRetryNumber);
      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
      break;

//    case E_ERR_SWC_ExceedExceptionRetryLimit:
//      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
//      printf("[SWC] CSWC::FastPolling, exceed exception retry limit\n");
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
//      return;
//      //break;
// This group of case belongs to internal exception, will not happen in normal working
// condition.

    case E_ERR_SER_SelectReadTimeOut:
    case E_ERR_SWC_InvalidFileDescriptor:
    case E_ERR_SER_FailToReadDevice:
    case E_ERR_InvalidNullPointer:
    case E_ERR_SWC_ExceedBufferSize:
    case E_ERR_SER_InvalidFileDescriptor:
    case E_ERR_SER_FailToWriteDevice:
        m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
        ERR_GetMsgString(rstatus, acErrMsg);
        sprintf(cLog,"WARN [SWC] CSWC::FastPolling, %s link %d down. Error %s\n",
               m_acSWCName, m_nPrimaryPortID, acErrMsg);
        g_pDebugLog->LogMessage(cLog);
     #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
        break;

    default:
        #ifdef CFG_PRN_WARN
      ERR_GetMsgString(rstatus, acErrMsg);
      sprintf(cLog,"[SWC] CSWC::FastPolling, %s unhandled error, %s\n",
              m_acSWCName, acErrMsg);
      g_pDebugLog->LogMessage(cLog);
      #endif //CFG_PRN_WARN
      break;
  }// switch(rstatus)

 //  m_anLinkStatus[m_nPrimaryPortID] = LINKERROR; Yang Tao 20150506 To remove this line avoid CRC error cause link down
  // 20100711 BC (Rqd by ZSL)
  ActivateTimeOut(PRIMARYSTATUS);

} // FastPolling
/*******************************************************************************
Purpose

  This routine will perform link status check

Input
  None

Return
  None

History
    Name           Date          Remark
    ----           ----          ------
  Yu, Wei        20-May-2004   Initial revision
  Bryan Chong    26-Feb-2010   Implement SER_SendWithReply routine to replace
                               SendReply routine [PR30]
  Bryan Chong    27-Apr-2010   The implementation of SER_SendWithReply resolved
                               the standby fail polling frequency issue [PR33]
  Bryan Chong    29-Jun-2010   Update to use RequestAndReceiveWithModbusCheck
                               routine and resolve issue on [C955 PR49],
                               [C955 PR50], [C955 PR53]
  Bryan Chong    02-Jul-2010   Update master link resolve link role status and
                               link healthiness issue from RMT [PR48]
  Bryan Chong    26-Jul-2010   Update dual-link SWC to include time sync when
                               master link is recovered. [PR67]
                               Enable dual-link SWC to initiate polling from
                               link 1
  Bryan Chong    29-Jul-2010   Fixed standby fail polling interval
*******************************************************************************/
void CSWC::PrimaryLinkCheck(void)
{
  E_ERR_T rstatus = E_ERR_Success;
  char acRecevData[2048] = {0};
  char acTemp[200];
  char cLog[1000];

  //#ifdef CFG_PRN_WARN
  CHAR acErrMsg[100] = {0};
  //#endif // CFG_PRN_WARN

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)

  if(m_nTotalLinkNumber == 1)
  {
    if(m_anLinkStatus[m_nPrimaryPortID] != LINKERROR)  //All link is down.
        return;

    if((CheckTimeOut(PRIMARYSTATUS, m_nStandbyPollingFailedValue)== false) &&
       (m_bStartupCheckLink == false))
      return;

    ActivateTimeOut(PRIMARYSTATUS);
    m_bStartupCheckLink = false;

    rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nPrimaryPortID],
      (UINT8 *)m_acStatusCommand[m_nPrimaryPortID], 8, acRecevData,
      (UINT32)m_nStatusPollingRecevLen, m_nReceiveTimeout,
      (UINT8)m_nRetryNumber, (UINT8)m_nExcepRetry, (UINT8 *)&m_nPrimaryExcepCounter);
	ERR_GetMsgString(rstatus, acErrMsg);

	//20150603 Su, added log
	if(rstatus != E_ERR_Success)
	{
	printf("ERR [SWC] CSWC::Primary1LinkCheck, %s, rstatus: %s\n", m_acSWCName, acErrMsg);
	sprintf(cLog, "ERR [SWC] CSWC::Primary1LinkCheck, %s, rstatus: %s", m_acSWCName, acErrMsg);
	g_pDebugLog->LogMessage(cLog);
	}

    switch(rstatus)
    {
      case E_ERR_Success:
        m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
        m_bSlowPollingStart = true;
        m_bTimerPollingStart = true;
        m_bTimeSynStartFlag = true;

        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
        printf("[SWC] CSWC::PrimaryLinkCheck, %s rx %d bytes:\n",
               m_acSWCName,  m_nStatusPollingRecevLen);
        SYS_PrnDataBlock((const UINT8 *)acRecevData, m_nStatusPollingRecevLen,
                         10);
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
        break;

      case E_ERR_SWC_ModbusCRCError:
        m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
        #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC)
        printf("WARN [SWC] CSWC::PrimaryLinkCheck, %s fd %d\n"
               "  rx Exception Error %d bytes:\n",
               m_acSWCName, m_anCommfd[m_nPrimaryPortID],
               m_nStatusPollingRecevLen);
        SYS_PrnDataBlock((const UINT8 *)acRecevData, m_nStatusPollingRecevLen,
                         20);
        #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC)
        break;
      case E_ERR_SWC_ExceedRetryLimit:
        m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
        ActivateTimeOut(PRIMARYSTATUS);
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
        sprintf(cLog, "[SWC] CSWC::PrimaryLinkCheck, %s fd %d\n"
               "  exceed retry limit. Set link = LINKERROR\n",
               m_acSWCName, m_anCommfd[m_nPrimaryPortID], m_nRetryNumber);
        g_pDebugLog->LogMessage(cLog);
        #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
        break;
//      E_ERR_SWC_ExceedExceptionRetryLimit is removed from the RequestAndReceiveWithModbusCheck.
      case E_ERR_SWC_ExceedExceptionRetryLimit:
        m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
        printf("[SWC] CSWC::PrimaryLinkCheck, %s exceed exception retry "
               "limit\n",
               m_acSWCName);
        #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
        break;

     case E_ERR_SWC_ModbusExceptionError:
        m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
        printf("[SWC] CSWC::PrimaryLinkCheck, %s found exception error\n",
               m_acSWCName);
        #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
        break;

     case E_ERR_SER_SelectReadTimeOut:
     case E_ERR_SWC_InvalidFileDescriptor:
     case E_ERR_SER_SelectReadError:
        m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
        printf("[SWC] CSWC::PrimaryLinkCheck, E_ERR_SER_SelectReadTimeOut, "
               "link %d down\n", m_nPrimaryPortID);
        #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
        break;

      default:
        m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
        #ifdef CFG_PRN_WARN
        ERR_GetMsgString(rstatus, acErrMsg);
        printf("WARN [SWC] CSWC::PrimaryLinkCheck, %s unhandled error, %s\n",
               m_acSWCName, acErrMsg);
        printf("  fd %d tx 8 cmd bytes:\n", m_anCommfd[m_nPrimaryPortID]);
        SYS_PrnDataBlock((const UINT8 *) m_acStatusCommand[m_nPrimaryPortID], 8,
                         10);
        printf("  fd %d rx %d bytes:\n", m_anCommfd[m_nPrimaryPortID],
               m_nStatusPollingRecevLen);
        SYS_PrnDataBlock((const UINT8 *) acRecevData, m_nStatusPollingRecevLen,
                         10);
        #endif //CFG_PRN_WARN
        break;
    }// switch(rstatus)
  }  // if(m_nTotalLinkNumber == 1)

  if(m_nTotalLinkNumber == 2)
  {
    /*
     Start checking slave link status, if slave link is healthy and standby
     polling interval is timeout, swap slave link to become master link.
     If slave link is down and standby fail polling interval is timeout,
     check if slave link is recovered.
     When slave link is healthy, swap slave link to master when master link is
     previously down. Otherwise, set slave link to down

     The fail polling interval from SWC simulator will be 2x of the defined
     fail polling frequency. This is due to the two links will poll
     alternatively
    */
    #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_SWC_PRIMARYLINKCHECK)
    // Check for ECS
    if (CFG_DEBUG_SWC_INDEX)
    {
      printf("[SWC] CSWC::PrimaryLinkCheck, %s m_anLinkStatus,\n"
        "  m_nPrimaryPortID %s, m_nStandbyPortID %s, m_bStartupCheckLink %d\n",
        m_acSWCName,
        (m_anLinkStatus[m_nPrimaryPortID] == LINKOK) ? "LINKOK" : "LINKERROR",
        (m_anLinkStatus[m_nStandbyPortID] == LINKOK) ? "LINKOK" : "LINKERROR",
        m_bStartupCheckLink);
    }
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_TIMESYNC)

    if(m_bStartupCheckLink)
    {
      m_bTimeSynStartFlag = true;
      m_bTimerPollingStart = true;
      m_bStartupCheckLink = false;
    }  // if(m_anLinkStatus[m_nPrimaryPortID] == LINKOK)

    if(m_anLinkStatus[m_nStandbyPortID] == LINKOK)
    {
      // Stdby link OK

      if(CheckTimeOut(STANDBYSTATUS, m_nStandbyPollingValue)== true)
      {
        ActivateTimeOut(STANDBYSTATUS);

        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
        if(CFG_DEBUG_SWC_INDEX)
        {
          printf("[SWC] CSWC::PrimaryLinkCheck, %s tx %d bytes:\n",
                 m_acSWCName, 8);
          SYS_PrnDataBlock((const UINT8 *)m_acStatusCommand, 8, 10);
        }
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)

        rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nStandbyPortID],
          (UINT8 *)m_acStatusCommand[m_nStandbyPortID], 8, acRecevData,
          (UINT32)m_nStatusPollingRecevLen, m_nReceiveTimeout,
          (UINT8)m_nRetryNumber, (UINT8)m_nExcepRetry, (UINT8 *)&m_nPrimaryExcepCounter);	//20150603 Su, m_nRetryNumber instead of 1 retry
    	ERR_GetMsgString(rstatus, acErrMsg);

    	//20150603 Su, added log
    	if(rstatus != E_ERR_Success)
    	{
    	printf("ERR [SWC] CSWC::Primary2LinkCheck, I, %s, Link %d, rstatus: %s\n", m_acSWCName, m_nStandbyPortID, acErrMsg);
    	sprintf(cLog, "ERR [SWC] CSWC::Primary2LinkCheck, I, %s, Link %d, rstatus: %s", m_acSWCName, m_nStandbyPortID, acErrMsg);
    	g_pDebugLog->LogMessage(cLog);
    	}

        switch(rstatus)
        {
          case E_ERR_Success:
            ActivateTimeOut(STANDBYSTATUS);
            // if link 1 is down swap link 2 to be primary
            if(m_anLinkStatus[m_nPrimaryPortID] == LINKERROR)
            {
              //Don't reset timer, the slow polling must do. 040312 Yu Wei
              //And the timer polling must do also.   040312 Yu Wei
              m_bSlowPollingStart = true;
              m_bTimerPollingStart = true;

              // 20100726 BC (Rqd by ZSL)
              //m_bTimeSynStartFlag = true;

              //Changing Primary and Standby link port
              m_nPrimaryPortID ^= 1;
              m_nStandbyPortID ^= 1;

              m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
              // [PR48]
              m_nMasterLinkID = m_nPrimaryPortID;
            }
            else
            {
              m_anLinkStatus[m_nStandbyPortID] = LINKOK;
            }
            // 20100729 BC (Rqd by ZSL)
            m_bStartupCheckLink = false;
            break;

          case E_ERR_SWC_ExceedRetryLimit:
            //Cannot get response or response error.
            m_anLinkStatus[m_nStandbyPortID] = LINKERROR;  //Set link down.
            #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
            if(CFG_DEBUG_SWC_INDEX)
            {
              printf("[SWC] CSWC::PrimaryLinkCheck, %s link %d exceed retry "
                     "limit\n",
                     m_acSWCName, m_nPrimaryPortID);
            }
            #endif // ((defined CFG_DEBUG_MSG) &&
                   //  (CFG_DEBUG_SWC_PRIMARYLINKCHECK))
            // 20100726 BC (Rqd by ZSL)
            if(m_anLinkStatus[m_nPrimaryPortID] == LINKERROR)
            {
              m_nPrimaryPortID ^= 1;
              m_nStandbyPortID ^= 1;
            }
            sprintf(cLog, "%s PrimaryLinkCheck Standby serial link(%d) breakdown\n",
                    m_acSWCName, m_nStandbyPortID);
            g_pDebugLog->LogMessage(cLog);
            break;

          case E_ERR_SWC_ExceedExceptionRetryLimit:
            #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
            if(CFG_DEBUG_SWC_INDEX)
            {
              printf("[SWC] CSWC::PrimaryLinkCheck, exceed exception retry "
                     "limit\n");
            }
            #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
            break;

          case E_ERR_SER_SelectReadTimeOut:
            m_anLinkStatus[m_nStandbyPortID] = LINKERROR;
            #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
            if(CFG_DEBUG_SWC_INDEX)
            {
              printf("[SWC] CSWC::PrimaryLinkCheck, E_ERR_SER_SelectReadTimeOut, "
                     "link %d down, swap link, \n", m_nStandbyPortID);
            }
            #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
            //m_nPrimaryPortID ^= 1;
            //m_nStandbyPortID ^= 1;
            break;

          default:
            m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
            #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
            if(CFG_DEBUG_SWC_INDEX)
            {
              ERR_GetMsgString(rstatus, acErrMsg);
              printf("[SWC] CSWC::PrimaryLinkCheck, %s unhandled error, %s\n",
                     m_acSWCName, acErrMsg);
              printf("  fd %d tx 8 cmd bytes:\n", m_anCommfd[m_nStandbyPortID]);
              SYS_PrnDataBlock(
                (const UINT8 *) m_acStatusCommand[m_nStandbyPortID], 8, 10);
              printf("  fd %d rx %d bytes:\n", m_anCommfd[m_nStandbyPortID],
                     m_nStatusPollingRecevLen);
              SYS_PrnDataBlock((const UINT8 *) acRecevData,
                               m_nStatusPollingRecevLen, 10);
            }
            #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
            break;
        }// switch(rstatus)

      } // if(CheckTimeOut(STANDBYSTATUS, m_nStandbyPollingValue)== true)
    } // if(m_anLinkStatus[m_nStandbyPortID] == LINKOK)
    else
    {
      // Stdby link NG
      // when slave link is down and standby polling fail interval is timeout
      if((CheckTimeOut(STANDBYSTATUS, m_nStandbyPollingFailedValue)== true) ||
         (m_bStartupCheckLink == true))
      {
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
        if(CFG_DEBUG_SWC_INDEX)
        {
          rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                             E_SYS_TIME_Local);
          printf("[SWC] %d.%02d.%02d %02d:%02d:%02d.%03d "
                 "CSWC::PrimaryLinkCheck, %s\n"
                 "  when timeout at standby fail polling time %d ms, slave "
                 "link %d, fd %d tx cmd n rx data\n",
                 (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
                 pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
                 pbdtime->tm_sec, tspec.tv_nsec/1000000, m_acSWCName,
                 m_nStandbyPollingFailedValue, m_nStandbyPortID,
                 m_anCommfd[m_nStandbyPortID]);
        }
        #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)

        ActivateTimeOut(STANDBYSTATUS);

        // [PR49]
        rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nStandbyPortID],
          (UINT8 *)m_acStatusCommand[m_nStandbyPortID], 8, acRecevData,
          (UINT32)m_nStatusPollingRecevLen, m_nReceiveTimeout,
          (UINT8)m_nRetryNumber, (UINT8)m_nExcepRetry, (UINT8 *)&m_nPrimaryExcepCounter);
    	ERR_GetMsgString(rstatus, acErrMsg);

    	//20150603 Su, added log
    	if(rstatus != E_ERR_Success)
    	{
    	printf("ERR [SWC] CSWC::Primary2LinkCheck, II, %s, Link %d, rstatus: %s\n", m_acSWCName, m_nStandbyPortID, acErrMsg);
    	sprintf(cLog, "ERR [SWC] CSWC::Primary2LinkCheck, II, %s, Link %d, rstatus: %s", m_acSWCName, m_nStandbyPortID, acErrMsg);
    	g_pDebugLog->LogMessage(cLog);
    	}
        switch(rstatus)
        {
          case E_ERR_Success:
            /* when master link is previously down, swap slave link to be
               master link */
            if(m_anLinkStatus[m_nPrimaryPortID] == LINKERROR)
            {
              m_bSlowPollingStart = true;    //Don't reset timer, the slow polling must do. 040312 Yu Wei
              m_bTimerPollingStart = true;  //And the timer polling must do also.   040312 Yu Wei
              m_nPrimaryPortID ^= 1;      //Changing Primary and Standby link port
              m_nStandbyPortID ^= 1;

              m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
              // [PR48]
              m_nMasterLinkID = m_nPrimaryPortID;

              // 20100803 BC (Rqd by ZSL)
              m_bTimeSynStartFlag = true;
              m_bStartupCheckLink = false;

              #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
              if(CFG_DEBUG_SWC_INDEX)
              {
                printf("[SWC] CSWC::PrimaryLinkCheck, %s\n"
                       "  stdby link %d recovered. Set link to master. Poll at "
                       "m_nStandbyPollingFailedValue %d ms\n",
                       m_acSWCName, m_nPrimaryPortID,
                       m_nStandbyPollingFailedValue);
              }
              #endif // ((defined CFG_DEBUG_MSG) &&
                     //  (CFG_DEBUG_SWC_PRIMARYLINKCHECK))
            }else{

              m_anLinkStatus[m_nStandbyPortID] = LINKOK;

              #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_PRIMARYLINKCHECK))
              if(CFG_DEBUG_SWC_INDEX)
              {
                printf("[SWC] CSWC::PrimaryLinkCheck, %s link %d recovered "
                       "Set link to slave\n",
                       m_acSWCName, m_nStandbyPortID,
                       m_nStandbyPollingFailedValue);
              }
              #endif // ((defined CFG_DEBUG_MSG) &&
            }

            break;

          case E_ERR_SWC_ExceedRetryLimit:
            m_anLinkStatus[m_nStandbyPortID] = LINKERROR;
            #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
            if(CFG_DEBUG_SWC_INDEX)
            {
              printf("[SWC] CSWC::PrimaryLinkCheck, %s link %d exceed retry "
                     "limit\n",
                     m_acSWCName, m_nPrimaryPortID);
            }
            #endif // ((defined CFG_DEBUG_MSG) &&
                   //  (CFG_DEBUG_SWC_PRIMARYLINKCHECK))

            //Changing Primary and Standby link port
            if( m_anLinkStatus[m_nPrimaryPortID] == LINKERROR)
            {
              m_nPrimaryPortID ^= 1;
              m_nStandbyPortID ^= 1;
            }
            break;

          case E_ERR_SWC_ExceedExceptionRetryLimit:
            #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
            printf("[SWC] CSWC::PrimaryLinkCheck, exceed exception retry "
                   "limit\n");
            #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
            break;

          case E_ERR_SER_SelectReadTimeOut:
            m_anLinkStatus[m_nStandbyPortID] = LINKERROR;
            //m_nPrimaryPortID ^= 1;
            //m_nStandbyPortID ^= 1;

            #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC)
            printf("WARN [SWC] CSWC::PrimaryLinkCheck, "
                   "E_ERR_SER_SelectReadTimeOut, %s set link %d = LINKERROR\n",
                   m_acSWCName, m_nStandbyPortID);
            #endif // CFG_PRN_WARN
            break;


          default:
            m_anLinkStatus[m_nStandbyPortID] = LINKERROR;
            #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
            ERR_GetMsgString(rstatus, acErrMsg);
            printf("[SWC] CSWC::PrimaryLinkCheck, %s unhandled error, %s\n",
                   m_acSWCName, acErrMsg);
            printf("  fd %d tx 8 cmd bytes:\n", m_anCommfd[m_nStandbyPortID]);
            SYS_PrnDataBlock(
              (const UINT8 *) m_acStatusCommand[m_nStandbyPortID], 8, 10);
            printf("  fd %d rx %d bytes:\n", m_anCommfd[m_nStandbyPortID],
                   m_nStatusPollingRecevLen);
            SYS_PrnDataBlock((const UINT8 *) acRecevData,
                             m_nStatusPollingRecevLen, 10);
            #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
            break;
        }// switch(rstatus)
      } // if((CheckTimeOut(STANDBYSTATUS,m_nStandbyPollingValue)== true) ||    //060327 Yu Wei
        //    (m_bStartupCheckLink == true))    //Startup check link.    //040520 Yu Wei
    } // end else if(m_anLinkStatus[m_nStandbyPortID] == LINKOK)

  } // if(m_nTotalLinkNumber == 2)

  CheckLinkStatusChange();

} // PrimaryLinkCheck
/*******************************************************************************
Purpose
  This routine will update slow polling table

Input
  None

Return
  None

History
    Name           Date          Remark
    ----           ----          ------
  Yu, Wei       12-Aug-2004  Initial revision
  Bryan Chong   26-Feb-2010  Implement SER_SendWithReply routine to replace
                             SendReply routine [PR30]
  Bryan Chong   02-Jul-2010  Update link switch role decision through primary
                             link instead of standby link to resolve link
                             healthiness issue from RMT [PR48]
  Bryan Chong   18-Mar-2011  Allowing ECS reply packet zero byte data length
                             [C955 PR89]
  Bryan Chong   16-May-2011  Remove supporting non-extend modbus of zero
                             data byte count
  Bryan Chong   25-May-2011  Implement RequestAndReceiveWithModbusCheck routine
                             to achieve cohesion and maintainability of code
*******************************************************************************/
void CSWC::SlowPolling(void)
{
  E_ERR_T rstatus = E_ERR_Success;
  char acRecevData[MDB_MAX_MODBUS_DATA_SZ + 5];
  //#ifdef CFG_PRN_WARN
  char acErrMsg[100] = {0};
 // #endif // CFG_PRN_WARN
  int nrval = 0;
  char cLog[100];

  if((m_bSlowPollingFlag == false ) || (m_anLinkStatus[m_nPrimaryPortID] == LINKERROR))
    return;

  nrval = CheckTimeOut(SLOWPOLLING,m_nSlowPollingValue);

  if((nrval == false) && (m_bSlowPollingStart == false))
    return;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_SLOWPOLLING)
  printf("[SWC] CSWC::SlowPolling, %s rx link %d. Poll rx length %d, retry %d, "
         "timeout %d ms, excep retry %d, m_nSlowPollingValue %d\n",
         m_acSWCName, m_nPrimaryPortID, m_nSlowPollingRecevLen, m_nRetryNumber,
         m_nReceiveTimeout, m_nExcepRetry, m_nSlowPollingValue);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))

  ActivateTimeOut(SLOWPOLLING);
  m_bSlowPollingStart = false;

  rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nPrimaryPortID],
    (UINT8 *)m_acSlowPollingCommand[m_nPrimaryPortID], 8, acRecevData,
    (UINT32)m_nSlowPollingRecevLen, m_nReceiveTimeout,
    (UINT8)m_nRetryNumber, (UINT8)m_nExcepRetry, (UINT8 *)&m_nFastExcepCounter);
	ERR_GetMsgString(rstatus, acErrMsg);

	//20150603 Su, added log
	if(rstatus != E_ERR_Success)
	{
	printf("ERR [SWC] CSWC::SlowPolling, %s, rstatus: %s\n", m_acSWCName, acErrMsg);
	sprintf(cLog, "ERR [SWC] CSWC::SlowPolling, %s, rstatus: %s", m_acSWCName, acErrMsg);
	g_pDebugLog->LogMessage(cLog);
	}

  switch(rstatus)
  {
    case E_ERR_Success:
    	m_nMasterLinkID = m_nPrimaryPortID;
        if(m_anLinkStatus[m_nPrimaryPortID] == LINKERROR)
        {
        m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
        }
        m_nFastExcepCounter = 0;
        WriteSWCTable(m_aunSWCTableSlow,&acRecevData[3], m_nSlowDataLen);
		#if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_SLOWPOLLING)
        printf("[SWC] CSWC::SlowPolling, rx %d bytes:\n", m_nSlowPollingRecevLen);
        SYS_PrnDataBlock((const UINT8 *)acRecevData, m_nSlowPollingRecevLen, 20);
		#endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
        return;
        break;

    case E_ERR_SWC_ModbusExceptionError:
    case E_ERR_SER_SelectReadError:
    case E_ERR_SER_InvalidSelectReturnValue:
    case E_ERR_SWC_ModbusCRCError:
      m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
      m_nFastExcepCounter++;
      if( m_nFastExcepCounter >= m_nExcepRetry)
      {
     	  m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
     	   m_nFastExcepCounter=0;
     	   ERR_GetMsgString(rstatus, acErrMsg);
     	   printf(cLog,"WARN [SWC] CSWC::SlowPolling, %s fd %d\n"
    	            " rx Exceptional Error exceed exception limit cause link down %d bytes, Error %s:\n",
    	       m_acSWCName, m_anCommfd[m_nPrimaryPortID],m_nFastPollingRecevLen,acErrMsg);

     	   sprintf(cLog,"WARN [SWC] CSWC::SlowPolling, %s fd %d\n"
     	            " rx Exceptional Error exceed exception limit cause link down %d bytes, Error %s:\n",
     	    m_acSWCName, m_anCommfd[m_nPrimaryPortID],m_nFastPollingRecevLen,acErrMsg);

     	    g_pDebugLog->LogMessage(cLog);
      }
      else
      {
     	 // m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
            ERR_GetMsgString(rstatus, acErrMsg);
            printf("WARN [SWC] CSWC::SlowPolling, %s fd %d\n"
                    "  rx Exceptional Error %d bytes, Error %s, Exception error counter %d:\n",
                    m_acSWCName, m_anCommfd[m_nPrimaryPortID],m_nFastPollingRecevLen,acErrMsg,m_nFastExcepCounter);

           sprintf(cLog,"WARN [SWC] CSWC::SlowPolling, %s fd %d\n"
                  "  rx Exceptional Error %d bytes, Error type is %s, Exception error counter %d:\n",
                  m_acSWCName, m_anCommfd[m_nPrimaryPortID],m_nFastPollingRecevLen,acErrMsg,m_nFastExcepCounter);

         	g_pDebugLog->LogMessage(cLog);
      }
      break;

    case E_ERR_SWC_ExceedRetryLimit:
        m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
        sprintf(cLog, "[SWC] CSWC::SlowPolling, %s link( %d ) breakdown due to Timeout, %s",
        		m_acSWCName, m_nPrimaryPortID, acErrMsg);
         g_pDebugLog->LogMessage(cLog);
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_SLOWPOLLING))
      printf("[SWC] CSWC::SlowPolling, %s exceed retry limit. Set link = LINKERROR \n",
             m_acSWCName, m_nRetryNumber);
      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
      break;

//    case E_ERR_SWC_ExceedExceptionRetryLimit:
//      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_SLOWPOLLING))
//      printf("[SWC] CSWC::SlowPolling, exceed exception retry limit\n");
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
//      break;


    case E_ERR_SER_SelectReadTimeOut:
    case E_ERR_SWC_InvalidFileDescriptor:
    case E_ERR_SER_FailToReadDevice:
    case E_ERR_InvalidNullPointer:
    case E_ERR_SWC_ExceedBufferSize:
    case E_ERR_SER_InvalidFileDescriptor:
    case E_ERR_SER_FailToWriteDevice:
      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;

      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_SLOWPOLLING)
     ERR_GetMsgString(rstatus, acErrMsg);
	 sprintf(cLog,"WARN [SWC] CSWC::SlowPolling, %s link %d down. Error %s",
		   m_acSWCName, m_nPrimaryPortID, acErrMsg);
	 g_pDebugLog->LogMessage(cLog);
	 #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
	 break;

    default:
      #ifdef CFG_PRN_WARN
      ERR_GetMsgString(rstatus, acErrMsg);
      sprintf(cLog,"[SWC] CSWC::SlowPolling, %s unhandled error, %s",
             m_acSWCName, acErrMsg);
      g_pDebugLog->LogMessage(cLog);
      #endif //CFG_PRN_WARN
      break;
  }// switch(rstatus)
} // SlowPolling

/*******************************************************************************
Purpose
  This routine will update timer polling table. The retry number included the
  normal attempt.

Input
  None

Return
  None

History
    Name           Date          Remark
    ----           ----          ------
  Yu, Wei        12-Aug-2004  Initial revision
  Chong, Bryan   26-Feb-2010  Implement SER_SendWithReply routine to replace
                              SendReply routine. [PR30]
  Bryan Chong    02-Jul-2010  Update link switch role decision through primary
                              link instead of standby link to resolve link
                              healthiness issue from RMT [PR48]
  Bryan Chong    22-Oct-2010  Use local time to check polling time [PR81]
  Bryan Chong    18-Mar-2011  Allowing ECS returning of 0 data byte length
                              [C955 PR89]
  Bryan Chong    16-May-2011  Remove supporting non-extend modbus of zero
                              data byte count
  Bryan Chong    25-May-2011  Implement RequestAndReceiveWithModbusCheck routine
                              to achieve cohesion and maintainability of code
*******************************************************************************/
void CSWC::TimerPolling(void)
{
  E_ERR_T rstatus = E_ERR_Success;
  char acRecevData[MDB_MAX_MODBUS_DATA_SZ + 5];
  //#ifdef CFG_PRN_WARN
  char acErrMsg[100] = {0};
  //#endif // CFG_PRN_WARN
  int nrval = 0;
  char cLog[1000];
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_TIMERPOLLING)
  struct timespec   tspec;
  struct tm         *pbdtime;
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_TIMERPOLLING)

  if((m_bTimerPollingFlag == false ) || (m_anLinkStatus[m_nPrimaryPortID] == LINKERROR))
    return;

  nrval = CheckTimeOut(TIMERPOLLING, m_nTimerPollingValue);

  if((nrval == false) && (m_bTimerPollingStart == false))
    return;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_TIMERPOLLING)
  if(m_bTimerPollingStart == true)
    printf("[SWC] CSWC::TimerPolling, triggered by link resume\n");

  rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec, E_SYS_TIME_Local);
  printf("[SWC] %d.%02d.%02d %02d:%02d:%02d.%03d CSWC::TimerPolling, %s\n"
         "  link %d rx length %d, retry %d, timeout %d ms,\n"
         "  excep retry %d, m_nTimerPollingValue %d, m_bTimerPollingStart %d\n",
         (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
         pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
         pbdtime->tm_sec, tspec.tv_nsec/1000000,
         SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
         m_nPrimaryPortID, m_nTimerPollingRecevLen, m_nRetryNumber,
         m_nReceiveTimeout, m_nExcepRetry, m_nTimerPollingValue,
         m_bTimerPollingStart);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))

  ActivateTimeOut(TIMERPOLLING);
  m_bTimerPollingStart = false;

  rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nPrimaryPortID],
    (UINT8 *)m_acTimerPollingCommand[m_nPrimaryPortID], 8, acRecevData,
    (UINT32)m_nTimerPollingRecevLen, m_nReceiveTimeout,
    (UINT8)m_nRetryNumber, (UINT8)m_nExcepRetry, (UINT8 *)&m_nFastExcepCounter);
	ERR_GetMsgString(rstatus, acErrMsg);

	//20150603 Su, added log
	if(rstatus != E_ERR_Success)
	{
	printf("ERR [SWC] CSWC::TimerPolling, %s, rstatus: %s\n", m_acSWCName, acErrMsg);
	sprintf(cLog, "ERR [SWC] CSWC::TimerPolling, %s, rstatus: %s", m_acSWCName, acErrMsg);
	g_pDebugLog->LogMessage(cLog);
	}

  switch(rstatus)
  {
    case E_ERR_Success:

    	m_nFastExcepCounter = 0;
      WriteSWCTable(m_aunSWCTableTimer,&acRecevData[3], m_nTimerDataLen);
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_TIMERPOLLING)
      printf("[SWC] CSWC::TimerPolling, rx %d bytes:\n", m_nTimerPollingRecevLen);
      SYS_PrnDataBlock((const UINT8 *)acRecevData, m_nTimerPollingRecevLen, 20);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
      return;
      break;

    case E_ERR_SWC_ModbusExceptionError:
    case E_ERR_SER_SelectReadError:
    case E_ERR_SER_InvalidSelectReturnValue:
    case E_ERR_SWC_ModbusCRCError:
      m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
      m_nFastExcepCounter++;

      #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC)
      printf("WARN [SWC] CSWC::TimerPolling, %s fd %d\n"
             "  rx Exception Error %d bytes:\n",
             m_acSWCName, m_anCommfd[m_nPrimaryPortID], m_nTimerPollingRecevLen);
      SYS_PrnDataBlock((const UINT8 *)acRecevData, m_nTimerPollingRecevLen,
                       20);
      #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC)
      if( m_nFastExcepCounter >= m_nExcepRetry)
            {
           	  m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
           	   m_nFastExcepCounter=0;
           	   ERR_GetMsgString(rstatus, acErrMsg);
           	   printf(cLog,"WARN [SWC] CSWC::TimerPolling, %s fd %d\n"
          	            " rx Exceptional Error exceed exception limit cause link down %d bytes, Error %s:\n",
          	       m_acSWCName, m_anCommfd[m_nPrimaryPortID],m_nFastPollingRecevLen,acErrMsg);

           	   sprintf(cLog,"WARN [SWC] CSWC::TimerPolling, %s fd %d\n"
           	            " rx Exceptional Error exceed exception limit cause link down %d bytes, Error %s:\n",
           	    m_acSWCName, m_anCommfd[m_nPrimaryPortID],m_nFastPollingRecevLen,acErrMsg);

           	    g_pDebugLog->LogMessage(cLog);
            }
            else
            {
                  ERR_GetMsgString(rstatus, acErrMsg);
                  printf("WARN [SWC] CSWC::TimerPolling, %s fd %d\n"
                          "  rx Exceptional Error %d bytes, Error %s, Exception error counter %d:\n",
                          m_acSWCName, m_anCommfd[m_nPrimaryPortID],m_nFastPollingRecevLen,acErrMsg,m_nFastExcepCounter);

                 sprintf(cLog,"WARN [SWC] CSWC::TimerPolling, %s fd %d\n"
                        "  rx Exceptional Error %d bytes, Error type is %s, Exception error counter %d:\n",
                        m_acSWCName, m_anCommfd[m_nPrimaryPortID],m_nFastPollingRecevLen,acErrMsg,m_nFastExcepCounter);

               	g_pDebugLog->LogMessage(cLog);
            }
      break;

    case E_ERR_SWC_ExceedRetryLimit:
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_TIMERPOLLING))
      sprintf(cLog,"[SWC] CSWC::TimerPolling, %s exceed retry limit. "
             "Set link = LINKERROR \n",
             m_acSWCName, m_nRetryNumber);
      g_pDebugLog->LogMessage(cLog);
      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_TIMERPOLLING))
      break;

//    case E_ERR_SWC_ExceedExceptionRetryLimit:
//      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_TIMERPOLLING))
//      printf("[SWC] CSWC::TimerPolling, exceed exception retry limit\n");
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
//      break;

    case E_ERR_SER_SelectReadTimeOut:
    case E_ERR_SWC_InvalidFileDescriptor:
    case E_ERR_SER_FailToReadDevice:
    case E_ERR_InvalidNullPointer:
    case E_ERR_SWC_ExceedBufferSize:
    case E_ERR_SER_InvalidFileDescriptor:
    case E_ERR_SER_FailToWriteDevice:
        m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
		 #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_TIMERPOLLING))
			  ERR_GetMsgString(rstatus, acErrMsg);
		 printf("WARN [SWC] CSWC::TimerPolling, %s link %d down. Error %s\n",
			   m_acSWCName, m_nPrimaryPortID, acErrMsg);
		 #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
        break;

    default:
      #ifdef CFG_PRN_WARN
      ERR_GetMsgString(rstatus, acErrMsg);
      printf("[SWC] CSWC::TimerPolling, %s unhandled error, %s\n",
             m_acSWCName, acErrMsg);
      #endif //CFG_PRN_WARN
      break;
  }// switch(rstatus)
} // TimerPolling

/**********************************************************************************
Purpose:
  Convert HEX format data to two-digit BCD. Maximum hex value to be converted
  is 0x63, which is decimal value 99.

  Eg. 0x0F -> 0x15

Input
  cData  --  Hex format data.

Return
  BCD format data.
***********************************************************************************/
char CSWC::HexToBCD(char cData)
{
  return(((cData/10)<<4) + (cData%10));
}

/**********************************************************************************
Purpose:
  Set time synchronization command

Parameter
  unReady  [in] Time validation flag

Return
  None

History
    Name         Date          Remark
    ----         ----          ------
  Yu, Wei      22-Mar-2004   Initial revision
  Bryan Chong  22-Oct-2010   Update to use local time
  Bryan Chong  25-Oct-2010   Swap ready flag from time sync command

***********************************************************************************/
void CSWC::SetTimeSynCommand(unsigned short unReady)
{
  struct timespec tTime;
  struct tm *tFormatTime;


  clock_gettime(CLOCK_REALTIME, &tTime);    //Get current UTC time

  // 20101022 BC
  tFormatTime = localtime(&tTime.tv_sec);

  //Set time data.
  m_acTimeSynCommand[m_nPrimaryPortID][7]  =
    HexToBCD((CHAR)tFormatTime->tm_mday);
  m_acTimeSynCommand[m_nPrimaryPortID][8]  =
    HexToBCD((CHAR)(tFormatTime->tm_mon + 1));
  m_acTimeSynCommand[m_nPrimaryPortID][9]  =
    HexToBCD((CHAR)((tFormatTime->tm_year + 1900)/100));
  m_acTimeSynCommand[m_nPrimaryPortID][10] =
    HexToBCD((CHAR)((tFormatTime->tm_year + 1900)%100));
  m_acTimeSynCommand[m_nPrimaryPortID][11] =
    HexToBCD((CHAR)tFormatTime->tm_hour);
  m_acTimeSynCommand[m_nPrimaryPortID][12] =
    HexToBCD((CHAR)tFormatTime->tm_min);
  m_acTimeSynCommand[m_nPrimaryPortID][13] =
    HexToBCD((CHAR)tFormatTime->tm_sec);
  m_acTimeSynCommand[m_nPrimaryPortID][14] = 0;  //Not used
  *(UINT16 *)&m_acTimeSynCommand[m_nPrimaryPortID][15] =
    STR_SWAP_16BIT_REG(unReady);

  //CRC
  *(UINT16 *)&m_acTimeSynCommand[m_nPrimaryPortID][17] =
    STR_ByteSwap16Bit(ModbusCRC((UCHAR *)m_acTimeSynCommand[m_nPrimaryPortID],17));
//  m_acTimeSynCommand[m_nPrimaryPortID][17]=0x00;
//  m_acTimeSynCommand[m_nPrimaryPortID][18]=0x00;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_TIMESYNC)
  printf("[SWC] CSWC::SetTimeSynCommand, m_acTimeSynCommand[%d] = \n",
          m_nPrimaryPortID);
  SYS_PrnDataBlock((const UINT8 *)m_acTimeSynCommand[m_nPrimaryPortID], 19, 20);
  #endif  // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_TIMESYNC))
}// SetTimeSynCommand

/*******************************************************************************
Purpose:
  Implement time synchronization

*******************************************************************************/
/*******************************************************************************
 Purpose
  Send KeepAlive signal to SWC.

 Input
  None

 Return
  None

 History
  Name            Date       Remark
  ----            ----       ------
  Yu, Wei       12-Aug-2004  Initial revision
  Bryan Chong   21-Apr-2010  Implement SER_SendWithReply routine to replace
                             SendReply routine.
  Bryan Chong   28-Jun-2010  Update time sync process based on local time
                             [PR34]
  Bryan Chong   18-Oct-2010  Check time synchronization using local time [PR79]
  Bryan Chong   18-Mar-2011  Allowing ECS returning of 0 data length [C955 PR89]
  Bryan Chong   16-May-2011  Remove supporting non-extend modbus of zero
                             data byte count
  Bryan Chong   25-May-2011  Implement RequestAndReceiveWithModbusCheck routine
                             to achieve cohesion and maintainability of code
  Bryan Chong   20-Jan-2012  Remove code of writing to server table,
                             WriteSWCTable
  Bryan Chong   21-Jun-2012  Add m_bTimeSynStartFlag condition check [PR115]
*******************************************************************************/
void CSWC::TimeSynchronization(void)
{
  E_ERR_T rstatus = E_ERR_Success;
  char acRecevData[MDB_MAX_MODBUS_DATA_SZ + 5];
  char acErrMsg[100] = {0};
  //int nrval = 0;
  //int isTimeOut = 0;
  char cLog[1000],acTemp1[200];int nI;
  struct timespec tTime;
  struct tm *tFormatTime;
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_TIMESYNC)
  struct timespec   tspec;
  struct tm         *pbdtime;
  //CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_TIMESYNC)

  if((m_bTimeSynFlag == false ) || (m_anLinkStatus[m_nPrimaryPortID] == LINKERROR))
    return;

  clock_gettime(CLOCK_REALTIME, &tTime);  //Get current UTC time
  tFormatTime = localtime(&tTime.tv_sec);

  // The TIME_SYN_CHECK will make sure the specific time update only happens
  // once because the next time it happens will be 61 seconds later when the
  // hour and min conditions are not valid
  // 20120621 BC
  if((m_nSendTimeSynHour == tFormatTime->tm_hour) &&
     (m_nSendTimeSynMinute == tFormatTime->tm_min) &&
     (CheckTimeOut(TIME_SYN_CHECK,61000)== true))
  {
    m_bTimeSynStartFlag = true;
    ActivateTimeOut(TIME_SYN_CHECK);
  }

  if(m_bTimeSynStartFlag == false)
    return;

  m_nTimeSynCMDLength = 19;
  m_bTimeSynStartFlag = false;

  //Delay m_nSWCBusyTimeout send time sync //040707 Yu Wei
  delay(m_nSWCBusyTimeout);

  // Pack TimeSyncCommand data
  SetTimeSynCommand(1);

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_TIMESYNC)
  rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec, E_SYS_TIME_Local);
  printf("[SWC] %d.%02d.%02d %02d:%02d:%02d.%03d "
         "CSWC::TimeSynchronization, %s\n"
         "  rx link %d. retry %d, timeout %d ms, excep retry %d, cmd sz %d\n"
         "  tm_hour %d, tm_min %d, m_bTimeSynStartFlag %d\n",
         (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
         pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
         pbdtime->tm_sec, tspec.tv_nsec/1000000,
         m_acSWCName, m_nPrimaryPortID, m_nRetryNumber,
         m_nReceiveTimeout, m_nExcepRetry, m_nTimeSynCMDLength,
         tFormatTime->tm_hour, tFormatTime->tm_min, m_bTimeSynStartFlag);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))

  for(nI=0; nI<m_nTimeSynCMDLength; nI++)
  sprintf(&acTemp1[nI*2], "%02X",(unsigned char)m_acTimeSynCommand[m_nPrimaryPortID][nI]);
  sprintf(cLog, "[SWC] CSWC::TimeSynchronization, %s fd %d tx %d bytes:%s\n",
		    m_acSWCName, m_anCommfd[m_nPrimaryPortID], m_nTimeSynCMDLength,acTemp1);
  g_pEventLog->LogMessage(cLog);

  rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nPrimaryPortID],
    (UINT8 *)m_acTimeSynCommand[m_nPrimaryPortID], m_nTimeSynCMDLength,
    acRecevData, 8, m_nReceiveTimeout, (UINT8)m_nRetryNumber,
    (UINT8)m_nExcepRetry, (UINT8 *)&m_nFastExcepCounter);
  ERR_GetMsgString(rstatus, acErrMsg);

  //20150603 Su, added log
  if(rstatus != E_ERR_Success)
  {
  	printf("ERR [SWC] CSWC::TimeSynchronizatioin, %s, rstatus: %s\n", m_acSWCName, acErrMsg);
	sprintf(cLog, "ERR [SWC] CSWC::TimeSynchronization, %s, rstatus: %s", m_acSWCName, acErrMsg);
	g_pDebugLog->LogMessage(cLog);
  }

  switch(rstatus)
  {
    case E_ERR_Success:
    	m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
    	m_nFastExcepCounter = 0;

    	for(nI=0; nI<8; nI++)
    	sprintf(&acTemp1[nI*2], "%02X",(unsigned char)acRecevData[nI]);
    	sprintf(cLog, "[SWC] CSWC::TimeSynchronization, %s fd %d rx %d bytes:%s\n",
				m_acSWCName, m_anCommfd[m_nPrimaryPortID], 8,acTemp1);
    	g_pEventLog->LogMessage(cLog);

		#if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_TIMESYNC)
		printf("[SWC] CSWC::TimeSynchronization, rx %d bytes:\n", 8);
		SYS_PrnDataBlock((const UINT8 *)acRecevData, 8, 10);
		#endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
		return;
		break;

    case E_ERR_SWC_ModbusExceptionError:
    case E_ERR_SER_SelectReadError:
    case E_ERR_SER_InvalidSelectReturnValue:
    case E_ERR_SWC_ModbusCRCError:
    	m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
    	m_nFastExcepCounter++;
    	//#if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC)
    	sprintf(cLog,"WARN [SWC] CSWC::TimeSynchronization, %s fd %d ,rx %s, %d bytes:",
             m_acSWCName, m_anCommfd[m_nPrimaryPortID], acErrMsg, 8);
    	g_pEventLog->LogMessage(cLog);
    	// #endif // ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_SWC))
		break;

    case E_ERR_SWC_ExceedRetryLimit:
      m_anLinkStatus[m_nPrimaryPortID]=LINKERROR;
      //#if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_TIMESYNC)
      sprintf(cLog,"[SWC] CSWC::TimeSynchronization, %s exceed retry limit. "
             "Set link = LINKERROR", m_acSWCName, m_nRetryNumber);
      g_pEventLog->LogMessage(cLog);
      //#endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
      break;

//    case E_ERR_SWC_ExceedExceptionRetryLimit:
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_TIMESYNC)
//      printf("[SWC] CSWC::TimeSynchronization, exceed exception retry limit\n");
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
//      break;

    case E_ERR_SER_SelectReadTimeOut:
    case E_ERR_SWC_InvalidFileDescriptor:
    case E_ERR_SER_FailToReadDevice:
    case E_ERR_InvalidNullPointer:
    case E_ERR_SWC_ExceedBufferSize:
    case E_ERR_SER_InvalidFileDescriptor:
    case E_ERR_SER_FailToWriteDevice:
    default:
      m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
     // #ifdef CFG_PRN_WARN
      ERR_GetMsgString(rstatus, acErrMsg);
      sprintf(cLog,"[SWC] CSWC::TimeSynchronization, %s unhandled error, %s\n",
             m_acSWCName, acErrMsg);
      g_pEventLog->LogMessage(cLog);
      //#endif //CFG_PRN_WARN
      break;
  }// switch(rstatus)
} // TimeSynchronization
//#undef CFG_DEBUG_MSG
/*******************************************************************************
 Purpose
  Send KeepAlive signal to SWC.

 Input
   None

 Return
   None

 History
    Name           Date          Remark
    ----           ----          ------
  Yu, Wei        12-Aug-2004   Initial revision
  Bryan Chong    21-Apr-2010   Implement SER_SendWithReply routine to replace
                               SendReply routine.
  Bryan Chong    11-Jul-2010   Update to use routine
                               RequestAndReceiveWithModbusCheck
*******************************************************************************/
void CSWC::KeepAlive(void)
{
  E_ERR_T rstatus = E_ERR_Success;

  //#ifdef CFG_PRN_WARN
  CHAR acErrMsg[100] = {0};
  //#endif // CFG_PRN_WARN
  int nI;
  char cLog[1000];
  char acRecevData[MDB_MAX_MODBUS_DATA_SZ + 5];

  if(m_bKeepAliveFlag == false)
    return;

  //20150603 Su, keepalive only in primary link swc
  if(m_anLinkStatus[m_nPrimaryPortID] == LINKERROR)
    return;

  if(CheckTimeOut(KEEPALIVE_CMD, m_nKeepAliveInterval) == false)
    return;

  ActivateTimeOut(KEEPALIVE_CMD);

    *(unsigned short *)&m_acKeepAliveCommand[m_nPrimaryPortID][7] =
      STR_ByteSwap16Bit(m_unKeepAliveCmdWord);
    *(unsigned short *)&m_acKeepAliveCommand[m_nPrimaryPortID][9] =
      STR_ByteSwap16Bit(ModbusCRC((unsigned char *)m_acKeepAliveCommand[m_nPrimaryPortID],9));

   // Print the Keep alive command.
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_KEEPALIVE))
    printf("[SWC] CSWC::KeepAlive, %s tx cmd is %d bytes:\n",
           m_acSWCName, 11);
    SYS_PrnDataBlock((const UINT8 *)m_acKeepAliveCommand[m_nPrimaryPortID], 11, 20);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_KEEPALIVE))

    rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nPrimaryPortID],
        (UINT8 *)m_acKeepAliveCommand[m_nPrimaryPortID], 11, acRecevData, 8,
        m_nReceiveTimeout, (UINT8)m_nRetryNumber, (UINT8)m_nExcepRetry,
        (UINT8 *)&m_nFastExcepCounter);
    ERR_GetMsgString(rstatus, acErrMsg);

    //20150603 Su, added log
    if(rstatus != E_ERR_Success)
    {
    	printf("ERR [SWC] CSWC::KeepAlive, %s, rstatus: %s\n", m_acSWCName, acErrMsg);
	  	sprintf(cLog, "ERR [SWC] CSWC::KeepAlive, %s, rstatus: %s", m_acSWCName, acErrMsg);
	  	g_pDebugLog->LogMessage(cLog);
    }

    switch(rstatus)
    {
      case E_ERR_Success:
        if(m_anLinkStatus[m_nPrimaryPortID] == LINKERROR)
        {
          m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
        }
        m_nFastExcepCounter = 0;

        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_KEEPALIVE))
          printf("[SWC] CSWC::KeepAlive, %s link %d rx %d bytes:\n",
                 m_acSWCName, m_nPrimaryPortID, 8);
          SYS_PrnDataBlock((const UINT8 *)acRecevData, 8, 10);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_KEEPALIVE))
        break;

      case E_ERR_SWC_ExceedRetryLimit:
    	  m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
    	  //#if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_KEEPALIVE))
    	  sprintf(cLog,"[SWC] CSWC::KeepAlive, %s exceed retry limit. Set link = LINKERROR \n", m_acSWCName, m_nRetryNumber);
    	  g_pDebugLog->LogMessage(cLog);
    	  //#endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_KEEPALIVE))
    	  break;

//      case E_ERR_SWC_ExceedExceptionRetryLimit:
//        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_KEEPALIVE))
//        printf("[SWC] CSWC::KeepAlive, exceed exception retry limit\n");
//        #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_KEEPALIVE))
//        break;
      case E_ERR_SWC_ModbusExceptionError:
      case E_ERR_SER_SelectReadError:
      case E_ERR_SER_InvalidSelectReturnValue:
      case E_ERR_SWC_ModbusCRCError:
    	  m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
    	  m_nFastExcepCounter++;
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_KEEPALIVE)
        printf("[SWC] CSWC::KeepAlive,Exception Error, "
               "link %d up\n", m_nPrimaryPortID);
        #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_KEEPALIVE)
        break;

//      case E_ERR_SWC_ModbusCRCError:
//    	 m_anLinkStatus[m_nPrimaryPortID] = LINKOK;
//        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_KEEPALIVE))
//        printf("[SWC] CSWC::KeepAlive, E_ERR_SWC_ModbusCRCError\n");
//        #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_KEEPALIVE))
//        break;

      case E_ERR_SER_SelectReadTimeOut:
      case E_ERR_SWC_InvalidFileDescriptor:
      case E_ERR_SER_FailToReadDevice:
      case E_ERR_InvalidNullPointer:
      case E_ERR_SWC_ExceedBufferSize:
      case E_ERR_SER_InvalidFileDescriptor:
      case E_ERR_SER_FailToWriteDevice:
      default:
    	m_anLinkStatus[m_nPrimaryPortID] = LINKERROR;
        #ifdef CFG_PRN_WARN
        ERR_GetMsgString(rstatus, acErrMsg);
        printf("WARN [SWC] CSWC::KeepAlive, %s unhandled error, %s\n",
               m_acSWCName, acErrMsg);
        #endif //CFG_PRN_WARN
        break;
    }// switch(rstatus)

  m_unKeepAliveCmdWord = 0;  //Reset server status.

}// KeepAlive
/**********************************************************************************
Purpose:
  SWC main process

History

     Name       Date          Remark

 Bryan Chong  16-Jun-2011  Add the log message when watchdog fail to restart

***********************************************************************************/
void CSWC::MainProcess(void)
{
  #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_SWC)
  E_ERR_T rstatus = E_ERR_Success;
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_RMMLINKCHK)
  //char cLog[100];

  if(wdStart(SWC_main_timer_id, 30, 0, 0, 0) < 0)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SWC] CSWC::MainProcess, watchdog start error\n");
    #endif // CFG_PRN_ERR

    //20110616 BC
    g_pEventLog->LogMessage((CHAR *)
       "ERR  [RMM] CSWC::MainProcess, fail to restart watchdog\n");

  }

  #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_SWC)
  if(m_nSWCID == 1)
  {
    rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                       E_SYS_TIME_Local);
    printf("[SWC] %d.%02d.%02d %02d:%02d:%02d.%03d "
           "CSWC::MainProcess, %s enter\n",
           (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
           pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
           pbdtime->tm_sec, tspec.tv_nsec/1000000,
           m_acSWCName);
  }
  #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_PRIMARYLINKCHECK)
  SetStateFlag();  //Set SWC state .

  #if ((defined CFG_MODULE_TEST) && CFG_MDT_CMM)
  MDT_PrnToLog(CFG_MDT_CMM_LOG, m_acSWCName,
               (CHAR *)"[MT] CSWC::MainProcess, Path - 1\n");
  #endif // ((defined CFG_MODULE_TEST) && CFG_MDT_CMM)
//  printf("m_nSWCCurrentStateFlag=%d\n",m_nSWCCurrentStateFlag);
  if((m_nSWCCurrentStateFlag == STATEFLAG_PRIMARY) &&
     (m_bPollingEnable == true) && (m_bLinkCheckFlag == false) &&
     (m_bCheckingBeforeSwitch == false))
  {
    #if ((defined CFG_MODULE_TEST) && CFG_MDT_CMM)
    MDT_PrnToLog(CFG_MDT_CMM_LOG, m_acSWCName,
                 (CHAR *)"[MT] CSWC::MainProcess, Path - 2\n");
    #endif // ((defined CFG_MODULE_TEST) && CFG_MDT_CMM)

    PrimaryLinkCheck();
    FastPolling();
    SlowPolling();
    TimerPolling();
    TimeSynchronization();
    KeepAlive();
  }
  //Link inhibit, don't monitor and check link. //040430 Yu Wei
  else if((m_nSWCCurrentStateFlag == STATEFLAG_STANDBY) &&
          (m_bPollingEnable == true))
  {
    if(m_bLinkCheckFlag == true) //Standby RTU check link status.
    {
      #if ((defined CFG_MODULE_TEST) && CFG_MDT_CMM)
      MDT_PrnToLog(CFG_MDT_CMM_LOG, m_acSWCName,
                   (CHAR *)"[MT] CSWC::MainProcess, Path - 3\n");
      delay(100);
      #endif // ((defined CFG_MODULE_TEST) && CFG_MDT_CMM)
      LinkCheck();
      m_bLinkCheckFlag = false;
    }
    else
    {
      #if ((defined CFG_MODULE_TEST) && CFG_MDT_CMM)
      MDT_PrnToLog(CFG_MDT_CMM_LOG, m_acSWCName,
                   (CHAR *)"[MT] CSWC::MainProcess, Path - 4\n");
      #endif // ((defined CFG_MODULE_TEST) && CFG_MDT_CMM)
//	  sprintf(cLog, "DEC17 [SWC] CSWC::MainProcess, %s enter", m_acSWCName);
//	  g_pDebugLog->LogMessage(cLog);
      //Monitor Standby RTU link status
      LinkMonitor();
    }
  }

  #if ((defined CFG_MODULE_TEST) && CFG_MDT_CMM)
  MDT_PrnToLog(CFG_MDT_CMM_LOG, m_acSWCName,
               (CHAR *)"[MT] CSWC::MainProcess, Path - 5\n");
  #endif // ((defined CFG_MODULE_TEST) && CFG_MDT_CMM)
  //Process server specific command.
  CheckServerCommand();

  //Check message queue.
  CheckMessageQ();

  //Check link status.
  CheckLinkStatusChange();

  if(SWC_main_timer_id != NULL)
    wdCancel(SWC_main_timer_id);

  #if ((defined CFG_MODULE_TEST) && CFG_MDT_CMM)
  MDT_PrnToLog(CFG_MDT_CMM_LOG, m_acSWCName,
               (CHAR *)"[MT] CSWC::MainProcess, Path - 7\n");
  #endif // ((defined CFG_MODULE_TEST) && CFG_MDT_CMM)
}// MainProcess

/**********************************************************************************
Purpose:
  SWC checks if it received a server specific command and processes the command.

History

    Name          Date          Remark
    ----          ----          ------
  Yu, Wei       12-Aug-2004   Initial revision
  Bryan Chong   18-Jun-2010   Update received server command to debug log
                              [PR37] [PR38]
  Bryan Chong   07-Sep-2010   Move server commands to event log
***********************************************************************************/
void CSWC::CheckServerCommand(void)
{
  char acServerRecvBuffer[MODBUS_MAX_CMD_SIZE];
  int nServerRecvNumber;
  int nModbusError,nI;
  char acTemp[1000], acTemp1[1000];
  nServerRecvNumber =
     m_pServerSocket->Recv(acServerRecvBuffer, MODBUS_MAX_CMD_SIZE, 20, 0);
  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC || _CFG_DEBUG_SWC_SPCMD))
  if(nServerRecvNumber != -1)
  {

    printf("[SWC] CSWC::CheckServerCommand, %s rx %d(raw) bytes server cmd:\n",
            m_acSWCName, nServerRecvNumber);
    SYS_PrnDataBlock(
            (const UINT8 *)acServerRecvBuffer, nServerRecvNumber, 20);
  }
  #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SWC))

  //timeout 0--->20
  if(nServerRecvNumber > 0)
  {
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC || CFG_DEBUG_SWC_SPCMD))
    printf("[SWC] CSWC::CheckServerCommand, %s rx %d bytes server cmd:\n",
           m_acSWCName, nServerRecvNumber);
    SYS_PrnDataBlock(
            (const UINT8 *)acServerRecvBuffer, nServerRecvNumber, 20);
  #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SWC))
    sprintf(acTemp,"[SWC] CSWC::CheckServerCommand, %s rx %d bytes server cmd:",
           m_acSWCName, nServerRecvNumber);
    g_pEventLog->LogMessage(acTemp);

   // Yang Tao: Add in log SWC Receive from Server
    for(nI=0; nI<nServerRecvNumber; nI++)
    sprintf(&acTemp1[nI*2], "%02X",(unsigned char)acServerRecvBuffer[nI]);
    sprintf(acTemp,"RTU (Device: %s) Recv ServerCommand from server: %s",
    	                  m_acSWCName,acTemp1);
    g_pEventLog->LogMessage(acTemp);
    //Check modbus packet for the command
    nModbusError = ModbusCMDCheck((UINT8 *)acServerRecvBuffer,
                                  nServerRecvNumber, m_ucSWCAddress);
//        //20140603 Yang Tao, seperate LAN and SER to check modbus command
//    	    //Check modbus packet for the command
//    	    if(m_nLinkType == SWC_LINK_RS_485)
//    	    {
//    	    nModbusError = ModbusCMDCheck((UINT8 *)acServerRecvBuffer,
//    	                                  nServerRecvNumber, m_ucSWCAddress);
//    	    }
//    	    else if(m_nLinkType == SWC_LINK_LAN)
//    	    {
//    	    nModbusError = ModbusCMDCheck_LANSWC((UINT8 *)acServerRecvBuffer,
//    	                                  nServerRecvNumber, m_ucSWCAddress);
//    	    }

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
    printf("[SWC] CSWC::CheckServerCommand, modbus cmd check: %s, rval %d\n",
            ((nModbusError == MODBUS_MESSAGE_OK) ? "OK" : "ERROR"),
            nModbusError);
    #endif // CFG_DEBUG_MSG
    if(nModbusError != MODBUS_MESSAGE_OK)
    {
      ServerCommandError(nModbusError, acServerRecvBuffer);
      return;
    }

    if(m_nSWCCurrentStateFlag == STATEFLAG_PRIMARY)
    {
      //The command Ok and send the command to SWC.
      ServerCommand(acServerRecvBuffer, nServerRecvNumber);
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
      printf("[SWC] CSWC::CheckServerCommand, server cmd sent to %s\n",
              m_acSWCName);
      #endif // CFG_DEBUG_MSG
    }else{
      //Execption code 04 (command cannot executed).
      printf("[SWC] CSWC::CheckServerCommand, target is not in primary mode\n");
      ServerCommandError(MODBUS_ERROR_EXCEPTION3, acServerRecvBuffer);
    }
  }
}// CSWC::CheckServerCommand

/*******************************************************************************
Purpose:
  Process server command.

Input/Output
  acServerRecvBuffer  --  The buffer receiving from server. (in)
  nServerRecvNumber  --  The length of server command. (in)

History

    Name          Date          Remark
    ----          ----          ------
  Yu, Wei       12-Aug-2004   Initial revision
  Bryan Chong   18-Jun-2010   Update transmitted server command to debug log
                              [PR37] [PR38]
  Bryan Chong   01-Jun-2011   Add number of registers field to read word
                              function
*******************************************************************************/
void CSWC::ServerCommand(char *acServerRecvBuffer, int nServerRecvNumber)
{
  //printf("server command 0 \n");
  int nSWCReplyLen;
  char acSWCRecvBuffer[MODBUS_MAX_CMD_SIZE];
  int nI;
  char acTemp[2048],acTemp1[2048];
  unsigned short tmp = 0x0000;

  if(m_anLinkStatus[m_nPrimaryPortID] == LINKERROR)// Yang Tao 20150519 link down reply the exception code.

  {
     ServerCommandError(MODBUS_ERROR_EXCEPTION3, acServerRecvBuffer);
	  return;
  }
  //printf("SWC server command 1 \n");
  UpdateCommand(acServerRecvBuffer, nServerRecvNumber,
                m_acFastPollingCommand[m_nPrimaryPortID][0]);

  // Check function code
  switch(acServerRecvBuffer[1])
  {
//   Yang Tao 17041. The control command only have 0x0F & 0x10
//  case 0x02:
//    tmp = STR_ByteSwap16Bit(*(unsigned short *)&acServerRecvBuffer[4]);
//    nSWCReplyLen = (tmp >>3);  //divid 8, convert bits to bytes
//    if((tmp & 0x07) != 0)    //remainder is not 0,
//      nSWCReplyLen ++;
//    nSWCReplyLen += 5;    //Get total received data number
//    break;
//  case 0x03:
//  case 0x04:
//    // 20110601. Get number of registers field
//    tmp = STR_ByteSwap16Bit(*(unsigned short *)&acServerRecvBuffer[4]);
//    nSWCReplyLen = tmp*2 +5; //Get total received data number
//    break;

  case 0x0F:
  case 0x10:
    nSWCReplyLen = 8;
    break;

  default:      //040310 Yu Wei
    nSWCReplyLen = 0;
    break;
  }

  tmp = STR_ByteSwap16Bit(*(unsigned short *)&acServerRecvBuffer[2]);
  //Received keep alive command.    //040812 Yu Wei
  if((tmp == m_unSendKeepAliveAddr) && (m_bKeepAliveFlag == true))
  {
    tmp = STR_ByteSwap16Bit(*(unsigned short *)&acServerRecvBuffer[7]);
    m_unKeepAliveCmdWord = tmp;  //Update server status.

    *(unsigned short *)&acServerRecvBuffer[nSWCReplyLen-2] =
       STR_ByteSwap16Bit(ModbusCRC((unsigned char *)acServerRecvBuffer,
                  nSWCReplyLen-2));


      //Write command that send to server to debug log.
// Log for the SWC send to server
    for(nI=0; nI<nSWCReplyLen; nI++)
       sprintf(&acTemp1[nI*2], "%02X",(unsigned char)acServerRecvBuffer[nI]);
		sprintf(acTemp,"%s Keepalive message Send to server (Directly): %s",
             m_acSWCName,acTemp1);
      g_pEventLog->LogMessage(acTemp);

    m_pServerSocket->SendSWC(acServerRecvBuffer, nSWCReplyLen, 0);

    m_pServerSocket->Close(6);

#if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_KEEPALIVE)
    printf("[SWC] CSWC::ServerCommand, rx keep alive cmd from server\n");
 #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_KEEPALIVE)


  // m_pServerSocket->Close(6);
  }
  else
  {

    #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_SWC_SPCMD)
    printf("[SWC] CSWC::ServerCommand, pass swc reply len %d to "
           "SpecificCommand\n", nSWCReplyLen);
   #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_SPCMD)
    //Send command to SWC and get response.
    nSWCReplyLen = SpecificCommand(acSWCRecvBuffer,nSWCReplyLen);
// Yang Tao, receive the data from SWC
    //Send command that receive from SWC to server.
    for(nI=1; nI<nSWCReplyLen-2; nI++)
      acServerRecvBuffer[nI] = acSWCRecvBuffer[nI];

    *(unsigned short *)&acServerRecvBuffer[nSWCReplyLen-2] =
      STR_ByteSwap16Bit(ModbusCRC((unsigned char *)acServerRecvBuffer,nSWCReplyLen-2));

    m_pServerSocket->SendSWC(acServerRecvBuffer,nSWCReplyLen,0);
		//delay(300);// YT 159714: Buffer time for Server receive the command.//Yangtao 09022015
        // Disabled due to may cause delay problem.
		m_pServerSocket->Close(6);


    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_SPCMD)
    printf("[SWC] CSWC::ServerCommand, tx %d bytes:\n", nSWCReplyLen);
    SYS_PrnDataBlock((const UINT8 *) acServerRecvBuffer, nSWCReplyLen, 10);
    #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))


//    //Yang Tao: add in log message
//    for(nI=0; nI<nSWCReplyLen; nI++)
//      sprintf(&acTemp1[nI*2], "%02X",(unsigned char)acServerRecvBuffer[nI]);
//      sprintf(acTemp,"%s (RTU) Send to server : %s",
//            m_acSWCName,acTemp1);
//     g_pEventLog->LogMessage(acTemp);
  // m_pServerSocket->Close(6);
  }
}

/*******************************************************************************
Purpose:
  Process error server command.

Input/Output
  nModbusError    --  Modbus error. (in)
  acServerRecvBuffer  --  The buffer receiving from server. (in)

History

  Name          Date                   Remark
  ----          ----                   ------
 Bryan Chong  23-Dec-2009  Set MSb to 1 on function code field for exceptional
                           response

*******************************************************************************/
void CSWC::ServerCommandError(int nModbusError,  char *acServerRecvBuffer)
{
  int nI;
  char acTemp[2048],acTemp1[2048];

  /* according to modbus protocol function code field will need to have MSb to
     be 1 for exceptional response. This makes the function code value in an
     exception response exactly 80 hexadecimal higher than the value would be
     for a normal response. */
  if((UINT8)acServerRecvBuffer[1] < 0x80)
  {
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
    printf("[SWC] CSWC::ServerCommandError, function code = 0x%x\n",
            (UINT8)acServerRecvBuffer[1]);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
    acServerRecvBuffer[1] += 0x80;
  }

  switch(nModbusError)
  {
    case MODBUS_ERROR_MESSAGE_LEN:
    case MODBUS_ERROR_SLAVE_ADD:
    case MODBUS_ERROR_CRC:
      //No response.
      if(g_tDebugLogFlag.bServerCommand == true)
      {
        //sprintf(acTemp,"%s Receive a exception message (error code: %04X)\n",
         //m_acSWCName,nModbusError);
        //g_pDebugLog->LogMessage(acTemp);//20050414
      }
      acServerRecvBuffer[2] = 0xFF;
      break;

    case MODBUS_ERROR_EXCEPTION1:
      //Reply exception 1.
      acServerRecvBuffer[2] = 0x01;
      break;

    case MODBUS_ERROR_EXCEPTION3:
      //Reply exception 3.
      acServerRecvBuffer[2] = 0x03;
      break;

    case MODBUS_ERROR_EXCEPTION4:
      //Reply exception 4.
      acServerRecvBuffer[2] = 0x04;
      break;

    default:
      //Reply exception 0.
      acServerRecvBuffer[2] = 0x00;
      break;
  } // switch(nModbusError)

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
  printf("[SWC] CSWC::ServerCommandError, acServerRecvBuffer[2] = 0x%02x\n",
          (UINT8)acServerRecvBuffer[2]);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
  if((unsigned char)acServerRecvBuffer[2] != 0xFF)
  {
    //printf("11error acServerRecvBuffer == %02X \n",(unsigned char)acServerRecvBuffer[2]);
    //acServerRecvBuffer[1] |= 0x80;
    *(UINT16 *)&acServerRecvBuffer[3] =
      STR_ByteSwap16Bit(ModbusCRC((UINT8 *)acServerRecvBuffer,3));

    //m_pServerSocket->Send(acServerRecvBuffer, 5, 50); // timeout 0 ---> 50 // Yang Tao 20150511:
    m_pServerSocket->SendSWC(acServerRecvBuffer, 5, 50);  // Use SendSWC to relese the socket fD                                                   // Add in SendSWC
														  //
    m_pServerSocket->Close(6);								// Add Close (6) To close the socket as well



    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
    printf("[SWC] CSWC::ServerCommandError, tx 5 bytes:\n");
    SYS_PrnDataBlock((const UINT8 *)acServerRecvBuffer, 5, 10);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))

    if(g_tDebugLogFlag.bServerCommand == true)
    {
      //Write command that send to server to debug log.
      for(nI=0; nI<5; nI++)
      sprintf(&acTemp1[nI*2],"%02X",(unsigned char)acServerRecvBuffer[nI]);
      sprintf(acTemp,"%s Send to server (directly): %s\n",
              m_acSWCName,acTemp1);
      g_pEventLog->LogMessage(acTemp);
    }
  }
  return;
}

/**********************************************************************************
Purpose:
  Send specific command to SWC and get response.

Input/Output
  acReceData    -- The buffer for replying from SWC. (out)
  nReceDataLen  -- Expect data size of SWC replying. (in)

Return
  The size of response command.

History

    Name          Date          Remark
    ----          ----          ------
  Yu, Wei       12-Aug-2004   Initial revision
  Bryan Chong   18-Jun-2010   Implement routine RequestAndReceiveWithModbusCheck
                              to fix issues from write word and write bit
                              commands initiated by the server
                              [PR37] [PR38]
  Bryan Chong   07-Sep-2010   Move specific server command to event log instead
                              of debug log
***********************************************************************************/
int CSWC::SpecificCommand(char *acReceData, int nReceDataLen)
{
  E_ERR_T rstatus = E_ERR_Success;
//  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_SPCMD)
  CHAR acErrMsg[100] = {0};
  char acTemp[1000],acTemp1[1000];
//  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_SPCMD))
  int nRecvLength = 0;
  int nI;
  if( m_bPollingEnable == false )  //When Link is inhibited, reply timeout to server directly. //040707 Yu Wei
  {
    return (SpecificCommandSWCError(acReceData));
  }

  if(m_anLinkStatus[m_nPrimaryPortID] == LINKERROR)
  {
	  return (SpecificCommandSWCError(acReceData)); // if the link is down, then OD cmd cannot pass through.
  }

  memset(acReceData, 0, MODBUS_MAX_CMD_SIZE);

  // [PR37] [PR38]

  // Yang Tao: Add in log SWC Receive from Server
//  	for(nI=0; nI<m_nSpecificCommandLen; nI++)
//  	sprintf(&acTemp1[nI*2], "%02X",(unsigned char)m_acWriteCommand[nI]);
//  	sprintf(acTemp,"%s Send to device: %s",
//  	                  m_acSWCName,acTemp1);
//  	g_pEventLog->LogMessage(acTemp);
  // 201406 Yang Tao
  	for(int nI=0; nI<m_nSpecificCommandLen; nI++)
  	sprintf(&acTemp1[nI*2],"%02X",(unsigned char)m_acWriteCommand[nI]);
  	sprintf(acTemp,"RTU  Sent to (Device %s): %s",
  			m_acSWCName,acTemp1);
  	g_pEventLog->LogMessage(acTemp);

  rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[m_nPrimaryPortID],
      (UINT8 *)m_acWriteCommand, m_nSpecificCommandLen, acReceData,
      (UINT32)nReceDataLen, m_nReceiveTimeout,
      (UINT8)m_nRetryNumber, (UINT8)m_nExcepRetry,
      (UINT8 *)&m_nFastExcepCounter);

//      RequestAndReceiveFlag=0;


  switch(rstatus)
  {
    case E_ERR_Success:
    	// Yang Tao: Enable the Specific Command.
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_SPCMD)
      printf("[SWC] CSWC::SpecificCommand, fd %d tx %d bytes to %s:\n",
             m_anCommfd[m_nPrimaryPortID], m_nSpecificCommandLen, m_acSWCName);
      SYS_PrnDataBlock((const UINT8 *)m_acWriteCommand, m_nSpecificCommandLen,
                       10);
      printf("[SWC] CSWC::SpecificCommand, fd %d rx %d bytes fr %s:\n",
             m_anCommfd[m_nPrimaryPortID], nReceDataLen, m_acSWCName);
      SYS_PrnDataBlock((const UINT8 *)acReceData, nReceDataLen, 10);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
      nRecvLength = nReceDataLen;
      //Yang Tao: Add in the exceptional reply from the SWC, which is 5 byte replay
      if((acReceData[1]&0x80)==0x80)
      nRecvLength=5;


      // Yang Tao: Add in log SWC Receive from Server
      	for(nI=0; nI<nReceDataLen; nI++)
      	sprintf(&acTemp1[nI*2], "%02X",(unsigned char)acReceData[nI]);
      	sprintf(acTemp,"%s (Device) Reply to RTU: %s",
      	                  m_acSWCName,acTemp1);
      	g_pEventLog->LogMessage(acTemp);
      //

      break;

    case E_ERR_SWC_ExceedRetryLimit:
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_SPCMD)
      printf("[SWC] CSWC::SpecificCommand, %s exceed retry limit.\n",
             m_acSWCName, m_nRetryNumber);
      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
      return (SpecificCommandSWCError(acReceData)); //Yang Tao20150514 if the cmd is out of retry
												    // Need to rely as error

      break;
//
//    case E_ERR_SWC_ExceedExceptionRetryLimit:
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_SPCMD)
//      printf("[SWC] CSWC::SpecificCommand, %s exceed exception retry limit\n",
//             m_acSWCName);
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
//      break;

    default:
//      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_SPCMD)
      ERR_GetMsgString(rstatus, acErrMsg);
      printf("[SWC] CSWC::SpecificCommand, %s encounters unhandled error, %s\n",
              m_acSWCName, acErrMsg);
//      #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
      break;
  }// switch(rstatus)
  printf("[SWC] CSWC::SpecificCommand, nRecvLength=%d\n",
		  nRecvLength);
  return nRecvLength;
}

/**********************************************************************************
Purpose:
  Set exception response command.

Output
  acReceData  -- The buffer for exception response command.

Return
  The size of command.
***********************************************************************************/
int CSWC::SpecificCommandSWCError(char *acReceData)
{
  acReceData[0] = m_acWriteCommand[0];
  acReceData[1] = m_acWriteCommand[1] |0x80;
  acReceData[2] = 0x00;
  return 5;
}

/*******************************************************************************
Purpose:
  Check message queue

*******************************************************************************/
void CSWC::CheckMessageQ(void)
{
  int nReturn;
  int nI;
  tSWCCommand_MSG tMessage;
  tRMM_MSG tRMMMessageSend;
  struct mq_attr  mqstat;

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC || CFG_DEBUG_SWC_LINKCHK))
  E_ERR_T rstatus = E_ERR_Success;
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) &&
         //  (CFG_DEBUG_SWC || CFG_DEBUG_SWC_LINKCHK))

  mq_getattr(m_tMSGQID , &mqstat);
  nReturn = mq_receive(m_tMSGQID,(char *) &tMessage, mqstat.mq_msgsize ,NULL);

  if(nReturn > 0 )
  {
    switch(tMessage.nMessageID)
    {
      case SWC_LINK_CHECK_STANDBY_RTU:
       #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC || CFG_DEBUG_SWC_LINKCHK))
       rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                            E_SYS_TIME_Local);
       printf("[SWC] %d.%02d.%02d %02d:%02d:%02d.%03d "
              "CSWC::CheckMessageQ,\n"
              "  %s rx msgq SWC_LINK_CHECK_STANDBY_RTU\n",
              (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
              pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
              pbdtime->tm_sec, tspec.tv_nsec/1000000,
              m_acSWCName);
       #endif // ((defined CFG_DEBUG_MSG) &&
              //  (CFG_DEBUG_SWC || CFG_DEBUG_SWC_LINKCHK))

        //Link is inhibited, direct reply link check completed. //040708 Yu Wei
        if(m_bPollingEnable == false)
        {
        	// printf(" Number 6 m_unCurrentLinkStatus  =%02x \n",m_unCurrentLinkStatus);
          //Send link check completed message to RMM
          tRMMMessageSend.nDeviceID = m_nSWCID;
          tRMMMessageSend.nMessageID = SWC_MESSAGE_REPLY_LINK_CHECK;
          tRMMMessageSend.unStatus = m_unCurrentLinkStatus;
          g_pRTUStatusTable->SendMsg(tRMMMessageSend);
         // printf(" Number 8 m_unCurrentLinkStatus  =%02x \n",m_unCurrentLinkStatus);
        } else {
          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
          printf("[SWC] CSWC::CheckMessageQ, %s set m_bLinkCheckFlag = 1\n",
                  m_acSWCName);
          #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SWC))
          m_bLinkCheckFlag = true; //Set link check flag.
        }    //040708 Yu Wei

        break;

      case SWC_STOP_LINK_PRIMARY_RTU:
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
        printf("[SWC] CSWC::CheckMessageQ, %s rx msg "
               "SWC_STOP_LINK_PRIMARY_RTU stop SWC links at pri RTU, "
               "set m_bLinkCheckFlag = true\n",
               m_acSWCName);
        #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SWC))

        //Set link check flag. Standby RTU will check link.
        m_bLinkCheckFlag = true;

        //Disable serial port tx.
        for(nI=0; nI<2; nI++)
        {
          if(m_anLinkStatus[nI] != NOLINK)
          {
            int data = _CTL_DTR_CHG | 0 ;
            devctl(m_anCommfd[nI], DCMD_CHR_SERCTL, &data , sizeof(data), NULL);
          }
        }

        tRMMMessageSend.nDeviceID = m_nSWCID;
        tRMMMessageSend.nMessageID = SWC_MESSAGE_REPLY_LINK_STOP;
        tRMMMessageSend.unStatus = m_unCurrentLinkStatus;
        g_pRTUStatusTable->SendMsg(tRMMMessageSend);
        break;

      case SWC_START_LINK_PRIMARY_RTU:
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
        printf("[SWC] CSWC::CheckMessageQ, %s rx msg "
               "SWC_START_LINK_PRIMARY_RTU. Start SWC links\n", m_acSWCName);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
        //Enable serial port tx.
        for(nI=0; nI<2; nI++)
        {
          if(m_anLinkStatus[nI] != NOLINK)
          {
            int data = _CTL_DTR_CHG | _CTL_DTR;
            devctl(m_anCommfd[nI], DCMD_CHR_SERCTL, &data, sizeof(data), NULL);
          }
        }
        delay(50);

        m_bLinkCheckFlag = false; //Reset link check flag. Standby RTU completed check link.
        break;

      case SWC_POLLING_ENABLE:
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
        printf("[SWC] CSWC::CheckMessageQ, %s rx msgq SWC_POLLING_ENABLE\n",
               m_acSWCName);
        #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SWC))
        m_bPollingEnable = true;
        //Enable serial port tx.
        if( m_nSWCCurrentStateFlag == STATEFLAG_PRIMARY)    //040227 Yu Wei
        {
          for(nI=0; nI<2; nI++)
          {
            if(m_anLinkStatus[nI] != NOLINK)
            {
              int data = _CTL_DTR_CHG | _CTL_DTR;
              devctl(m_anCommfd[nI] ,DCMD_CHR_SERCTL, &data ,
                     sizeof(data ),NULL );
            }
          }
        }
        break;

      case SWC_POLLING_INHIBIT:
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
        printf("[SWC] CSWC::CheckMessageQ, %s rx msgq SWC_POLLING_INHIBIT\n",
               m_acSWCName);
        #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SWC))
        m_bPollingEnable = false;
        //Disable serial port tx.
        for(nI=0; nI<2; nI++)
        {
          if(m_anLinkStatus[nI] != NOLINK)
          {
            int data = _CTL_DTR_CHG | 0;
            devctl(m_anCommfd[nI], DCMD_CHR_SERCTL, &data, sizeof(data), NULL);
            #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
            printf("[SWC] CSWC::CheckMessageQ, m_anCommfd[%d] = %d \n",
                    nI , m_anCommfd[nI]);
            #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SWC))
          }
        }
        break;

      default:    //Do nothing    040310 Yu Wei
        #ifdef CFG_PRN_ERR
        printf("ERR  [SWC] CSWC::CheckMessageQ, %s rx invalid messge queue\n",
               m_acSWCName);
        #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SWC))
        break;
    }// switch
  }// if (nReturn > 0)
}// CheckMessageQ

/*******************************************************************************
Purpose:
  Send a message to SWC module.

Input
  tMessage  -- Message to be sent.(in)

Return
  OK    -- Successfully.
  ERROR  -- Send message error.

*******************************************************************************/
int CSWC::SendMessageQ(tSWCCommand_MSG tMessage)
{
  int nReturn=ERROR;
  //tSWCCommand_MSG tMessageTemp;
  struct mq_attr mqstat;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
  E_ERR_T rstatus = E_ERR_Success;
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) &&
         //  (CFG_DEBUG_SWC || CFG_DEBUG_SWC_LINKCHK))

  if(m_tMSGQID == TYP_NULL)
  {
    #ifdef CFG_PRN_ERR
    printf("[ERR]  CSWC::SendMessageQ, %s m_tMSGQID = NULL\n", m_acSWCName);
    #endif // CFG_PRN_ERR
    return ERROR;
  }

  mq_getattr(m_tMSGQID, &mqstat);
  #if ((defined CFG_DEBUG_MSG) && (_CFG_DEBUG_SWC))
  printf("[SWC] CSWC::SendMessageQ, tx msgq to SWC m_tMSGQID = %d\n",
         m_tMSGQID);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))

  nReturn = mq_send(m_tMSGQID, (char *)&tMessage, MESSAGE_SIZE, MSG_PRI_NORMAL);
  if (nReturn == ERROR)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SWC] CSWC::SendMessageQ, %s tx msg to swc error. "
           "Msg size = %d, mq_size =%d\n",
           m_acSWCName, MESSAGE_SIZE, mqstat.mq_msgsize);
    perror("ERR  [SWC] CSWC::SendMessageQ, errno\n");
    #endif // CFG_PRN_ERR
  }
  else
  {
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC || _CFG_DEBUG_SWC_LINKCHK))
    rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec, E_SYS_TIME_Local);
    printf("[SWC] %d.%02d.%02d %02d:%02d:%02d.%03d "
           "CSWC::SendMessageQ, %s tx msgq OK\n",
           (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
           pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
           pbdtime->tm_sec, tspec.tv_nsec/1000000,
           m_acSWCName);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
    nReturn = OK;
  }

  return nReturn;
}// SendMessageQ

/*******************************************************************************
Purpose:
  Copy current timer value into the designated timer type

Input
  iType  [in] Timeout type.

Return
  Nil

*******************************************************************************/
void CSWC::ActivateTimeOut(int iType)
{
  GetTimerValue(&m_atTimeOut[iType]);
}

/**********************************************************************************
Purpose:
  Check if the type is timeout.
  Re-design in 20 May 2004, Yu Wei. It will use relative timer.

Input
  iType    -- Timeout type. (in)
  unTimeValue  -- Timeout value. (in)

Return
  ture  -- Timeout.
  false  -- Not timeout.

***********************************************************************************/
bool CSWC::CheckTimeOut(int iType, unsigned int unTimeValue)
{
  struct tTimerValue tTime;
  unsigned int unTimerDiff = 0;

  GetTimerValue(&tTime);
  //Get high part different.
  unTimerDiff = ( tTime.High - m_atTimeOut[iType].High ) * TIMER_LOW_MSEC;
  unTimerDiff += tTime.Low;          //Added low part different.
  unTimerDiff -= m_atTimeOut[iType].Low;

  if (unTimerDiff >= unTimeValue)      //Check timeout.
  {
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
    printf("[SWC] CheckTimeOut, current time high = %d, low = %d, "
           "freq(ms): %d. t/o high: %d, low: %d, delta: %d\n",
           tTime.High, tTime.Low, unTimeValue, m_atTimeOut[iType].High,
           m_atTimeOut[iType].Low, unTimerDiff);
    #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SWC))
    return true;
  }
  else
    return false;
}

/*******************************************************************************
Purpose:
  Check link status. If the link status is changed, send a message to RMM.

History

    Name          Date          Remark
    ----          ----          ------
  Yu, Wei       04-Jul-2007   Initial revision
  Bryan Chong   27-Apr-2010   Add conditional check on
                              m_bWaitLinkCheckCompletedFlag.
                              m_unCurrentLinkStatus will only be updated when
                              m_bWaitLinkCheckCompletedFlag is reset

*******************************************************************************/
void CSWC::CheckLinkStatusChange(void)
{
  unsigned short nLinkStatus;
  tRMM_MSG tRMMMessageSend;
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};

  char cLog[1000];
  /* GDM Comment off on 9-5-2014; to check where did not set the m_bWaitLinkCheckCompletedFlag correctly) */
  //  if((m_nSWCCurrentStateFlag != STATEFLAG_INITIALIZATION) &&
  //     (g_pRTUStatusTable->m_bWaitLinkCheckCompletedFlag == false))
  if((m_nSWCCurrentStateFlag != STATEFLAG_INITIALIZATION))
  {

    #if ((defined CFG_MODULE_TEST) && CFG_MDT_CMM)
    MDT_PrnToLog(CFG_MDT_CMM_LOG, m_acSWCName,
                 (CHAR *)"[MT] CSWC::CheckLinkStatusChange, Path - 6\n");
    #endif // ((defined CFG_MODULE_TEST) && CFG_MDT_CMM)

    nLinkStatus = GetLinkStatus();

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_NTPC)
    if(m_nSWCID == 19)
    {
      printf("[SWC] CSWC::CheckLinkStatusChange, %s link status = 0x%04x, "
             "m_unCurrentLinkStatus = 0x%04x\n",
             m_acSWCName, STR_ByteSwap16Bit(nLinkStatus),
             STR_ByteSwap16Bit(m_unCurrentLinkStatus));
    }
    #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SWC))

    if(nLinkStatus != m_unCurrentLinkStatus)
    {
    	// printf(" Number 4 m_unCurrentLinkStatus  =%02x \n",m_unCurrentLinkStatus);
      if(((STR_ByteSwap16Bit(m_unCurrentLinkStatus) & VALIDAION_BIT) == 0) &&  //Link down
         ((STR_ByteSwap16Bit(nLinkStatus) & VALIDAION_BIT) != 0))        //Link resume
      {
         #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_TIMESYNC)
         printf("[SWC] CSWC::CheckLinkStatusChange, %s link recovered\n",
                m_acSWCName);
         #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_TIMESYNC)
      }
      m_unCurrentLinkStatus = nLinkStatus;

      tRMMMessageSend.nDeviceID = m_nSWCID;
      tRMMMessageSend.nMessageID = SWC_LINK_STATUS_CHANGE;
      tRMMMessageSend.nSWCType = 0;
      tRMMMessageSend.unStatus = m_unCurrentLinkStatus;
      g_pRTUStatusTable->SendMsg(tRMMMessageSend);
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
      printf("[SWC] CSWC::CheckLinkStatusChange, %s\n"
             "  update RMM for the status change to 0x%04x\n",
             SYS_GetDefaultThrdName(thrdnamebuff, sizeof(thrdnamebuff)),
             STR_ByteSwap16Bit(nLinkStatus));
      #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SWC))


      sprintf(cLog, "[SWC] CSWC:: %s update RMM for the status change to 0x%04x\n",
    	        m_acSWCName, STR_ByteSwap16Bit(nLinkStatus));
      g_pDebugLog->LogMessage(cLog);

    }
  }
}

/*******************************************************************************
Purpose:
  Get SWC communication link status.

  Bit  Value  Description
  ---  -----  -----------
  00   1      Line 1 in used
       0      Line 1 not in used
  01   1      Line 2 in used
       0      Line 2 not in used
  02   1      Enable RTU - External System link
       0      Inhibit RTU - External System link
  03   -      Not defined
  04   1      Line 1 is healthy
       0      Line 1 is faulty
  05   1      Line 2 is healthy
       0      Line 2 is faulty
  06   -      Not defined
  07   -      Not defined
  08   1      Line 1 is Master Link
       0      Line 1 is Slave Link
  09   1      Line 2 is Master Link
       0      Line 2 is Slave Link
  10   -      Not defined
  11   1      Multinode Serial Link
  12   1      Serial Link
  13   1      LAN Link
  14   1      No Exception error
       0      Exception error detected
  15   1      Communication valid
       0      Communication invalid


Return
  A word of link status.
*******************************************************************************/
unsigned short CSWC::GetLinkStatus(void)
{
  unsigned short nLinkStatus,Tempstatus;
  char cResult, cTemp;
  int Tableheader;
   Tableheader=RTU_STATUS_TABLE_SWC_OFFSET; // The header length of in the otherRTUstatus table
//  printf("SWC status table m_nSWCCurrentStateFlag =%d\n", (const UINT8 *)g_pCRTULink->m_acRecvOtherRTUBuffer[32]);

   cResult = 1;


  if(m_nLinkType == SWC_LINK_LAN)
  {
    //LAN Link enable
    nLinkStatus = 0x2000;
  }
//  else if(m_nLinkType == SWC_LINK_RS_485M )
//  {
//    // Multinode Serial Link
//    nLinkStatus = 0x0800;
//  }
  else
  {
    // Serial Link
    nLinkStatus = 0x1000;
  }

  //Check line 1 in use status
  cTemp = 0;
  //PSD2 m_nMasterLinkID=1, link1 = 1. 040325 Yu Wei
  if(m_anLinkStatus[0] != NOLINK)
  {
    //Other m_nMasterLinkID=0, link1 = 0. 040325 Yu Wei
    nLinkStatus |= 0x0001;
    cTemp++;
  }

  //Check link 2 in use status
  if(m_anLinkStatus[1] != NOLINK)
  {
    //Other m_nMasterLinkID=0, link2 = 1. 040325 Yu Wei
    nLinkStatus |= 0x0002;
    cTemp++;
  }

  cResult *= cTemp;

  //Check Line 1 health status.
  cTemp = 0;
  //PSD2 m_nMasterLinkID=1, link1 = 1. 040325 Yu Wei
  //if(m_anLinkStatus[m_nMasterLinkID] == LINKOK)
  if(m_anLinkStatus[0] == LINKOK)
  {
   //Other m_nMasterLinkID=0, link1 = 0. 040325 Yu Wei
    nLinkStatus |= 0x0010;
    cTemp++;
  }

  //Check Line 2 health status.
  //PSD2 m_nMasterLinkID=1, link2 = 0. 040325 Yu Wei
  if(m_anLinkStatus[1] == LINKOK)
  {
    //Other m_nMasterLinkID=0, link2 = 1. 040325 Yu Wei
    nLinkStatus |= 0x0020;
    cTemp++;
  }
  cResult *= cTemp;

  //Determine master link and standby link for dual link SWCs
  if(m_nMasterLinkID == 0)
    nLinkStatus |= 0x0100;
  else if(m_nMasterLinkID == 1)
    nLinkStatus |= 0x0200;

  //Link polling enable/disable.
  if(m_bPollingEnable == true)
  {
    nLinkStatus |= 0x0004;
  }
  else
  {
    cResult = 0;
  }

  // Check communication validity
  // Communication valid when
  //   1. line 1 or line 2 or both are in used; and
  //   2. line 1 or line 2 or both are healthy; and
  //   3. m_bPollingEnable = 1


   if(cResult != 0)
   {
   nLinkStatus |= VALIDAION_BIT;// Shift the Bit 15 to Bit 3
   nLinkStatus |= 0x8000;// Bit 15 up
   }
   else
   {
	 nLinkStatus &= 0xFFF7;
   }

// Combine both RTU 1 and RTU bit 3 status.
    if((nLinkStatus&0x0008)!= 0x0008)
    {
	    Tableheader+=m_nSWCID;
	    Tempstatus=STR_ByteSwap16Bit(g_pRTUStatusTable->m_aunOtherRTUStatus[Tableheader]);// Get the
	    // other RTU SWC status
	    //printf(" TheotherRTUSWCis up =%02x, SWCID=%d \n",Tempstatus,Tableheader);
	    if ((Tempstatus&0x0008)==0x0008)
	   {
		   nLinkStatus |= 0x8000; //20150327 Yang Tao
		  // printf("SWC status table m_nSWCCurrentStateFlag =%d \n",m_nSWCCurrentStateFlag);
		  // printf("SWC status table m_nSWCCurrentStateFlag =%02X\n", copy_RecvOtherRTUBuffer[32]);

	   }
	   else
	   {
		   nLinkStatus &= 0x7FFF; //20150327 Yang Tao
		  //printf("SWC status table after role swtiching \n");
	   }
   }
  //Set execption flag  //040707 Yu Wei
  if((m_nFastExcepCounter < m_nExcepRetry) &&  //No Fast polling execption.
     (m_nSlowExcepCounter < m_nExcepRetry) &&  //No Slow polling execption.
     (m_nTimerExcepCounter < m_nExcepRetry))   //No Timer polling execption.
  {
    nLinkStatus |= 0x4000;
  }  //040707 Yu Wei

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
  if(m_usTempStatus != nLinkStatus)
  {
    printf("[SWC] CSWC::GetLinkStatus, %s update nLinkStatus from 0x%04x to "
           "0x%04x\n",
           m_acSWCName, m_usTempStatus, nLinkStatus);
    m_usTempStatus = nLinkStatus;
  }
  #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SWC))
  //printf(" TheotherRTUSWCis up =%02x \n",nLinkStatus);
  return STR_ByteSwap16Bit(nLinkStatus);
} // GetLinkStatus

/**************************************************************
Purpose:
  Set State flag and serial link enable/disable.

***************************************************************/
/*******************************************************************************
Purpose
  Set SWC state flag.

Parameters
  None

Return
  None

History
    Name         Date          Remark
    ----         ----          ------
  Yu, Wei     22-Mar-2004   Initial revision

*******************************************************************************/
void CSWC::SetStateFlag(void)
{
  int nI;
  #if ((defined CFG_MODULE_TEST) && (CFG_MDT_CMM || CFG_MDT_SWC_LAN))
  CHAR strbuff[50] = {0};
  CHAR tmpbuff[100] = {0};
  #endif // ((defined CFG_MODULE_TEST) && CFG_MDT_CMM)

  if(m_nSWCCurrentStateFlag != g_tRTUStatus.nRTUStateFlag)
  {
    m_nSWCCurrentStateFlag = g_tRTUStatus.nRTUStateFlag;
    #if ((defined CFG_MODULE_TEST) && (CFG_MDT_CMM || CFG_MDT_SWC_LAN))
    RMM_GetStateString(m_nSWCCurrentStateFlag, strbuff, sizeof(strbuff));
    sprintf(tmpbuff, "[MT] CSWC::SetStateFlag, current state = %s\n",
            strbuff);
    MDT_PrnToLog(CFG_MDT_CMM_LOG, m_acSWCName, tmpbuff);
    #endif // ((defined CFG_MODULE_TEST) && (CFG_MDT_CMM || CFG_MDT_SWC_LAN))

    if(m_nLinkType == SWC_LINK_RS_485)
    {

      switch(m_nSWCCurrentStateFlag)
      {
      case STATEFLAG_PRIMARY:
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
        printf("[SWC] CSWC::SetStateFlag, %s STATEFLAG_PRIMARY\n", m_acSWCName);
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)

        //Set RS485 link Tx on.
        for(nI=0; nI<2; nI++)
        {
          if(m_anLinkStatus[nI] != NOLINK)
          {
            int data =  _CTL_DTR_CHG | _CTL_DTR;
            devctl(m_anCommfd[nI], DCMD_CHR_SERCTL, &data, sizeof(data), NULL);
          }
        }

        //Reset execption counter.  //040707 Yu Wei
        m_nTimeSyncExcepCounter = 0;
        m_nFastExcepCounter = 0;
        m_nSlowExcepCounter = 0;
        m_nTimerExcepCounter = 0;  //040707 Yu Wei

        // 20100630 BC
        m_bLinkCheckFlag = 0;
        m_bCheckingBeforeSwitch = 0;
      break;

      case STATEFLAG_STANDBY:
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
        printf("[SWC] CSWC::SetStateFlag, %s STATEFLAG_STANDBY\n",
               m_acSWCName);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
        //040707 Yu Wei
        m_nTimeSyncExcepCounter = 0;
        m_nFastExcepCounter = 0;
        m_nSlowExcepCounter = 0;
        m_nTimerExcepCounter = 0;  //040707 Yu Wei

      case STATEFLAG_SWITCHING_PTOS:    //040322 Yu Wei
        //#if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
        printf("[SWC] CSWC::SetStateFlag, %s STATEFLAG_SWITCHING_PToS\n",
               m_acSWCName);
        //#endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
        //Set RS485 link Tx off.
        for(nI=0; nI<2; nI++)
        {
          if(m_anLinkStatus[nI] != NOLINK)
          {
            int data = _CTL_DTR_CHG | 0;
            devctl(m_anCommfd[nI], DCMD_CHR_SERCTL, &data, sizeof(data), NULL);
            m_anStandbyRecevLen[nI] = 0;    //Clear standby received data length.
          }
          ActivateTimeOut(m_anStandbyMonitorTimeout[nI]);  //Set timer counter.
        }
        break;

      case STATEFLAG_SWITCHING_STOP:
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
        printf("[SWC] CSWC::SetStateFlag, %s STATEFLAG_SWITCHING_SToP\n",
               m_acSWCName);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
        break;


      case STATEFLAG_HARDWARETEST:
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
        printf("[SWC] CSWC::SetStateFlag, STATEFLAG_HARDWARETEST\n");
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
        for(nI=0; nI<2; nI++)
        {
          if(m_anLinkStatus[nI] != NOLINK)
          {
            int data = _CTL_DTR_CHG | 0;
            devctl(m_anCommfd[nI], DCMD_CHR_SERCTL, &data, sizeof(data), NULL);
            m_anStandbyRecevLen[nI] = 0;    //Clear standby received data length.
          }
        } // for(nI=0; nI<2; nI++)
        break;

     case STATEFLAG_SYSTEM_RESET:
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
        printf("[SWC] CSWC::SetStateFlag, STATEFLAG_SYSTEM_RESET\n");
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
        break;

      default:
        #ifdef CFG_PRN_ERR
        printf("ERR  [SWC] CSWC::SetStateFlag, StateFlag not defined (%d)\n",
               m_nSWCCurrentStateFlag);
        #endif // CFG_PRN_ERR
        break;
      }// switch(m_nSWCCurrentStateFlag)
    }// if(m_nLinkType == SWC_LINK_RS_485)
  }//  if(m_nSWCCurrentStateFlag != g_tRTUStatus.nRTUStateFlag)
}// SetStateFlag

/*******************************************************************************
Purpose:

  This routine manages listening, determine health status for primary and
  standby link through the receiving length; and update m_nMasterLinkID,
  m_nPrimaryPortID, and m_nStandbyPortID accordingly.

  If the m_nReceiveTimeout, TIME_OUT for the respective SWC is not long enough,
  the primary and standby health status will toggle.

  Normally set uspollfreq is set to 2 x m_nStandbyPollingValue to avoid
  toggling for dual link SWC

  Receiving timeout for single link monitoring will remain to use
  TIME_OUT frequency.

  The data integrity checking is simplify to to check on first 3 byte of the
  packet header, which is slave address, function code, and data byte length,
  instead of receiving the whole packet. This is because standby monitoring
  does not need to manipulate or process the receiving data.

  We will not look at keep alive message, 0x91 10 00 or 0x92 10 00; as both
  link has keep alive which can not be differentiate from one another


History
    Name           Date          Remark
    ----           ----          ------
  Yu, Wei        12-Aug-2004   Initial revision
  Bryan Chong    06-May-2010   Replace Recv with SER_ReceiveMsg to fix
                               inconsistency in receiving serial link data
  Bryan Chong    02-Jul-2010   Update LinkMonitor algorithm handling to
                               resolve link status and link healthiness issue
                               from RMT [PR48]
  Bryan Chong    06-Sep-2010   Change SER_ReceiveMsg delay from
                               (m_nStandbyPollingFailedValue +
                                m_nFastPollingValue) to uspollfreq time frame.
                               This will reduce link check elapse time
  Bryan Chong    18-Oct-2010   Move serial link status to debug log
  Bryan Chong    25-Oct-2010   Update timeout to half of the fast-polling
                               timeout interval to resolve inconsistent FPS
                               link status [C955, PR83]
  Bryan Chong    28-Oct-2010   Update to fix timeout for receiving fast polling
                               table during link monitoring at Standby mode.
                               m_nReceiveTimeout is equivalent to TIME_OUT,
                               a configurable parameter
  Bryan Chong    01-Apr-2011   Update standby RTU dual link SWC health status.
  Bryan Chong    06-Apr-2011   Update receiving timeout to use half of the
                               standby polling frequency when total link is
                               more than 1. [C955 PR95]
  Bryan Chong    09-Jun-2011   Update to support zero byte count field within
                               Modbus packet for link status, fast polling,
                               slow polling and timer polling
                               [C955 PR109]
   Yang Tao     02-Apr-2010    Update Link Healthy status judgment method of Standby
                               RTU resolved the ECS Standby link healthy status toggle
                               issue :(1)If recieve incomplete or exceptional data, will
                               increment the exception counter and reactivate the time-
                               -out; (2)  Added function code:83, 8F and 90 as valid
                               response from SWC. [C955 PR116]
*******************************************************************************/
VOID CSWC::LinkMonitor(VOID)
{
  E_ERR_T rstatus = E_ERR_Success;
  INT32 nI;
  char cLog[100];
  INT32 nRecvLength;    //Reveive dada length
  CHAR acReceData[MDB_MAX_MODBUS_DATA_SZ + 5] = {0};  //Receive data buffer.
  MDB_READ_REPLY_T *preplypack;
  UINT16 uspollfreq;
  CHAR acErrMsg[100] = {0};

  E_ERR_T rstatus1 = E_ERR_Success;
  #if ((defined CFG_DEBUG_MSG) && \
       (CFG_DEBUG_SWC_LINKMON || CFG_PRN_WARN_SWC_LINKMON))
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LINKMON))

  //prolong uspollfreq will affect link check timing
	uspollfreq = m_nReceiveTimeout * 2;	//20151215 Su

  for(nI=0; nI<m_nTotalLinkNumber; nI++)
  {
    if(m_anLinkStatus[nI] == NOLINK)
      continue;

    //Clear buffer
    memset(acReceData, 0, sizeof(acReceData));
    memset(m_acStandbyRecev[nI], 0, sizeof(m_acStandbyRecev[nI]));
    m_anStandbyRecevLen[nI] = 0;

    // 20100906 BC (Rqd by ZSL)
    rstatus = SER_ReceiveMsg(m_anCommfd[nI], acReceData, &nRecvLength, 1, uspollfreq);
    ERR_GetMsgString(rstatus, acErrMsg);
    if(rstatus != E_ERR_Success)
    {
  	  sprintf(cLog, "ERR [SWC] CSWC::LinkMonitor, %s Link %d is %s, current LinkStatus: %d",
  			  m_acSWCName, nI+1, acErrMsg, m_anLinkStatus[nI]);
  	  g_pDebugLog->LogMessage(cLog);
    }
// 20130328 Yang Tao
// When Standby RTU listen to SWC status, as long as receiving data the link is in healthy status
// Active time out means, the last period of time is over, either stay in the same link status
// or updated to different status.
    switch(rstatus)
    {//YangTao: Try to reduce the process of listening,
      case E_ERR_Success:
          if(m_anLinkStatus[nI] == LINKERROR)
          {
        	  m_anLinkStatus[nI] = LINKOK;
        	  //printf("[SWC] CSWC::LinkMonitor, %s link %d is recovered\n", m_acSWCName, nI);
        	  sprintf(cLog, "[SWC] CSWC::LinkMonitor, %s link %d is recovered", m_acSWCName, nI);
        	  g_pDebugLog->LogMessage(cLog);
          }

          if(nRecvLength <= 0)
        	  printf("[SWC] CSWC::LinkMonitor, %s link %d is okie, but zero length\n", m_acSWCName, nI, nRecvLength);

          //clear exception counter
          m_nFastExcepCounter = 0;
          tcflush(m_anCommfd[nI], TCIOFLUSH);
          ActivateTimeOut(m_anStandbyMonitorTimeout[nI]);
          break; // E_ERR_Success

      case E_ERR_SER_SelectReadTimeOut:
		  break;

      default:
    	  #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_LINKMON)
    	  ERR_GetMsgString(rstatus, acErrMsg);
    	  printf("WARN [SWC] CSWC::LinkMonitor, %s link %d unhandling error %s\n",
    			  m_acSWCName, nI, acErrMsg);
    	  #endif // ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_SWC_LINKMON))
    	  //m_nFastExcepCounter++;
    	  m_anLinkStatus[nI] = LINKOK;
    	  tcflush(m_anCommfd[nI], TCIOFLUSH);
    	  ActivateTimeOut(m_anStandbyMonitorTimeout[nI]);
    	  break;
    }

    // Determining link healthiness by checking if the link exceed total
    // timeouts of standby fail polling and fast polling timeouts
    if(CheckTimeOut(m_anStandbyMonitorTimeout[nI],
    	 (m_nStandbyPollingValue*(UINT8)m_nRetryNumber)) == false)
      continue;

    //#if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LINKMON))
    //if(nI == 1)
    {
    	  sprintf(cLog, "[SWC] CSWC::LinkMonitor, %s link %d exceed fail polling interval at %d, reset time",
    			  m_acSWCName, nI+1, (m_nStandbyPollingValue*(UINT8)m_nRetryNumber));
    	  g_pDebugLog->LogMessage(cLog);
    }
    //#endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LINKMON))

    ActivateTimeOut(m_anStandbyMonitorTimeout[nI]);
    //if(m_anLinkStatus[nI] == LINKOK)
    {
      // handle link exceeds total timeout more than standby fail polling interval
      m_anLinkStatus[nI] = LINKERROR;
      ERR_GetMsgString(rstatus, acErrMsg);
      sprintf(cLog, "[SWC] CSWC::LinkMonitor, %s link %d is set to DOWN, %s, after %d ms", m_acSWCName, nI, acErrMsg, (m_nStandbyPollingValue*(UINT8)m_nRetryNumber));
      g_pDebugLog->LogMessage(cLog);
    } // if(m_anLinkStatus[nI] == LINKOK)
  } // for(nI=0; nI<m_nTotalLinkNumber; nI++)
} // LinkMonitor
/*******************************************************************************
 Purpose
  This routine is being used by standby RTU to conduct link checking. This
  event happens when one of the SWC links from primary RTU is down, standby
  RTU will be informed through link check flag from RTU status table sent by
  primary RTU.

 Input
   None

 Return
   None

 History
    Name           Date          Remark
    ----           ----          ------
  Yu, Wei      12-Aug-2004   Initial revision
  Bryan Chong  21-Apr-2010   Implement SER_SendWithReply routine to replace
                              SendReply routine.
  Bryan Chong  19-Apr-2012   Replace polling retry variable, m_nRetryNumber,
                              with constant SWC_LINKCHK_RETRY to reduce
                              role switching time
  Bryan Chong  21-Jun-2012   Update to use m_bStartupCheckLink instead of
                              m_bTimeSynStartFlag and m_bTimerPolllingStart
                              flags. m_bStartupCheckLink will enable the two
                              flags at PrimaryLinkCheck routine
  Bryan Chong  22-Jun-2012   Update to handle dual-link SWC resumed after RTU
                              role-switching
*******************************************************************************/
void CSWC::LinkCheck(void)
{
  E_ERR_T rstatus = E_ERR_Success;
  char acRecevData[MDB_MAX_MODBUS_DATA_SZ + 5] = {0};
  int nI;
  tRMM_MSG tRMMMessageSend;
  char acTemp[128];
  char cLog[300];
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
  CHAR acErrMsg[100] = {0};
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
  UINT8 exceptioncnt = 0;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)


  if(m_nLinkType == SWC_LINK_RS_485)
  {
    for(nI=0; nI<m_nTotalLinkNumber; nI++)
    {
      int data = _CTL_DTR_CHG | _CTL_DTR;
      devctl(m_anCommfd[nI] ,DCMD_CHR_SERCTL , &data, sizeof(data ),NULL );
      delay(20);

      //Write event log.
//      sprintf(acTemp,"[SWC] CSWC::LinkCheck, %s LinkCheck\n",
//              m_acSWCName);
   //   g_pDebugLog->LogMessage(acTemp);
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_SWC_Serial_Link,
//      FLG_HDR_T_TimeStamp_DateAndTime, FLG_SEC_T_MilliSec, FLG_LOG_T_Debug,
//      "[SWC] CSWC::LinkCheck, %s LinkCheck\n",
//      m_acSWCName);

      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
      rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                           E_SYS_TIME_Local);
      printf("[SWC] %d.%02d.%02d %02d:%02d:%02d.%03d "
             "CSWC::LinkCheck, %s start link check on standby RTU link "
             "%d \n",
             (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
             pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
             pbdtime->tm_sec, tspec.tv_nsec/1000000,
             m_acSWCName, nI);
      #endif // ((defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SWC_SBLINK))

      rstatus = RequestAndReceiveWithModbusCheck(m_anCommfd[nI],
          (UINT8 *)m_acStatusCommand[nI], 8, acRecevData,
          (UINT32)m_nStatusPollingRecevLen, m_nReceiveTimeout,
          (UINT8)SWC_LINKCHK_RETRY, (UINT8)m_nExcepRetry, (UINT8 *)&exceptioncnt);


      switch(rstatus)
      {
        case E_ERR_Success:
          m_anLinkStatus[nI] = LINKOK;
          m_nFastExcepCounter=0;
          sprintf(cLog,"LinkCheck, %s fd %d link %d is recovered\n",
                  m_acSWCName, m_anCommfd[nI], nI);
          g_pDebugLog->LogMessage(cLog);
          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
          printf("[SWC] CSWC::LinkCheck, %s fd %d link %d rx %d bytes:\n",
                 m_acSWCName, m_anCommfd[nI], nI, m_nStatusPollingRecevLen);
          SYS_PrnDataBlock((const UINT8 *)acRecevData, m_nStatusPollingRecevLen,
                           10);
          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))

          break;

        case E_ERR_SWC_ModbusCRCError:
          m_anLinkStatus[nI] = LINKOK;
          #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_LINKCHK)
          printf("WARN [SWC] CSWC::LinkCheck, %s fd %d link %d\n"
                 "  rx E_ERR_SWC_ModbusCRCError %d bytes:\n",
                 m_acSWCName, m_anCommfd[nI], nI, m_nStatusPollingRecevLen);
          SYS_PrnDataBlock((const UINT8 *)acRecevData, m_nStatusPollingRecevLen,
                           20);
          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
          break;

        case E_ERR_SWC_ExceedRetryLimit:
          m_anLinkStatus[nI] = LINKERROR;

          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
          printf("[SWC] CSWC::LinkCheck, %s exceed retry limit, %d. "
                 "Set link %d = LINKERROR\n",
                 m_acSWCName, SWC_LINKCHK_RETRY, nI);
          #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_FASTPOLLING))
          break;

        case E_ERR_SWC_ExceedExceptionRetryLimit:
          m_anLinkStatus[nI] = LINKERROR;
          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
          printf("[SWC] CSWC::LinkCheck, exceed exception retry limit\n");
          #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LINKCHK))
          break;

        case E_ERR_SER_SelectReadTimeOut:
        case E_ERR_SER_SelectReadError:
        case E_ERR_SWC_InvalidFileDescriptor:
          m_anLinkStatus[nI] = LINKERROR;
          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
          printf("[SWC] CSWC::LinkCheck, E_ERR_SER_SelectReadTimeOut, "
                 "link %d rx 0 bytes\n", nI);
          #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_LINKCHK))
          break;

        default:
          m_anLinkStatus[nI] = LINKERROR;
          sprintf(cLog, "LinkCheck: %s link ( %d )exceed retry limit, Set to LinkError\n",
                  m_acSWCName, nI);
          g_pDebugLog->LogMessage(cLog);
          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
          ERR_GetMsgString(rstatus, acErrMsg);
          printf("[SWC] CSWC::LinkCheck, %s unhandled error, %s\n",
                 m_acSWCName, acErrMsg);
          #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
          break;
      }// switch(rstatus)

      data = _CTL_DTR_CHG | 0;
      devctl(m_anCommfd[nI], DCMD_CHR_SERCTL, &data, sizeof(data), NULL);
      delay(20);
    }

    #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_SWC_LINKCHK)
    printf("[SWC] CSWC::LinkCheck, %s m_anLinkStatus[m_nPrimaryPortID] %s,\n"
           "  m_anLinkStatus[m_nStandbyPortID] %s\n",
           m_acSWCName,
           m_anLinkStatus[m_nPrimaryPortID] == LINKOK ? "LINKOK" : "LINKERROR",
           m_anLinkStatus[m_nStandbyPortID] == LINKOK ? "LINKOK" : "LINKERROR");
    #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
    // Fixed bug. When checking link, the standby link = OK, primary link =
    // ERROR, and then switch over to primary, the fast polling will not work.
    // 040330 Yu Wei
    if((m_anLinkStatus[m_nPrimaryPortID] == LINKERROR) &&
       (m_anLinkStatus[m_nStandbyPortID] == LINKOK))
    {
      m_nPrimaryPortID ^= 1;
      m_nStandbyPortID ^= 1;
      m_bStartupCheckLink = true;
      sprintf(cLog, "LinkCheck, %s link switch role when pri link( %d ) NG, stdby link ( %d )OK\n",
              m_acSWCName,m_nStandbyPortID,m_nPrimaryPortID);
      g_pDebugLog->LogMessage(cLog);
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
      printf("[SWC] CSWC::LinkCheck, %s link switch role when pri link NG,"
             "  stdby link OK\n",
             m_acSWCName);
      #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
    }

    // Add this condition for dual link SWC, eg. ECS, link resume after RTU
    // role-switching
    if((m_anLinkStatus[m_nPrimaryPortID] == LINKOK) &&
       (m_anLinkStatus[m_nStandbyPortID] == LINKERROR))
    {
      m_bStartupCheckLink = true;
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
      printf("[SWC] CSWC::LinkCheck, %s link NO switch role when pri link OK,"
             "  stdby link NG\n", m_acSWCName);
      #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
    }

    // Add this condition for dual link SWC, eg. ECS, link resume after RTU
    // role-switching
    if((m_anLinkStatus[m_nPrimaryPortID] == LINKOK) &&
       (m_anLinkStatus[m_nStandbyPortID] == LINKOK))
    {
      m_bStartupCheckLink = true;
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
      printf("[SWC] CSWC::LinkCheck, %s link NO switch role when pri link OK,"
             "  stdby link OK\n", m_acSWCName);
      #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
    }

  }// if(m_nLinkType == SWC_LINK_RS_485)

  ActivateTimeOut(m_anStandbyMonitorTimeout[0]);
  if( m_nTotalLinkNumber == 2)
  {
    ActivateTimeOut(m_anStandbyMonitorTimeout[1]);
  }

  m_unCurrentLinkStatus = GetLinkStatus();  //Get SWC communication status.
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_LINKCHK)
  printf("[SWC] CSWC::LinkCheck, %s update m_unCurrentLinkStatus to 0x%04x\n",
         m_acSWCName, STR_ByteSwap16Bit(m_unCurrentLinkStatus));
  #endif // (defined CFG_DEBUG_MSG) && (defined CFG_DEBUG_SWC_LINKCHK))

  //Send link check completed message to RMM
  tRMMMessageSend.nDeviceID = m_nSWCID;
  tRMMMessageSend.nSWCType = 0;
  tRMMMessageSend.nMessageID = SWC_MESSAGE_REPLY_LINK_CHECK;
  tRMMMessageSend.unStatus = (m_unCurrentLinkStatus);
  g_pRTUStatusTable->SendMsg(tRMMMessageSend);
}// LinkCheck

/*----------------------------------------------------------------------------

 PUBLIC ROUTINE

  RequestAndReceiveWithModbusCheck

 DESCRIPTION

  This routine will send request to SWC, wait for response, and determine for
  modbus error upon receiving the response from SWC.

 CALLED BY

  SWC::FastPolling

 CALLS

  SYS_GetCurrentTimeInString
  SYS_PrnDataBlock
  SYS_SetMutex
  SER_SendWithReply
  ModbusReplyCheck

 PARAMETER

  fd              [in] file descriptor
  ptxcmd          [in] pointer to transmit command, or request command, buffer
  txcmdsz         [in] length of transmit command
  prxbuff         [in] pointer to receiving buffer
  expectedRxSz    [in] expected length to receive
  timeout_ms      [in] time out in millisecond
  retry           [in] number of retry. Must be bigger than exceptional retry
  exceptionRetry  [in] number of exceptional retry. Must be less than
                       the number of retry
  poutexceptionRetryCnt  [out] number of exceptional retry has happened


 RETURN

  E_ERR_Success                        the routine executed successfully
  E_ERR_SWC_ExceedRetryLimit           exceed retry limit
  E_ERR_SWC_ModbusExceptionError       modbus exception error
  E_ERR_SWC_ModbusCRCError             modbus CRC error
  E_ERR_SWC_ExceedExceptionRetryLimit  exceed exception retry limit

 AUTHOR

  Bryan K.W. Chong


 HISTORY

    NAME            DATE           REMARKS

 Bryan Chong      28-Apr-2010    Initial revision
 Bryan Chong      06-Apr-2011    Limit modbus check to 8 bytes length keep
                                 alive packet only to avoid return of CRC
                                 error
 Bryan Chong      18-Mar-2011    Allowing ECS returning of 0 data length.
                                 [C955 PR89]
 Bryan Chong      16-May-2011    Remove supporting non-extend modbus of zero
                                 data byte count
 Bryan Chong      18-Aug-2011    Manage exception case
 Bryan Chong      07-Dec-2011    Log invalid packets including invalid slave
                                 address and invalid replied data
                                 [C955,PR58] [C955,PR59]
 Bryan Chong      06-Jan-2012    Remove mutex. Mutex causes polling time
                                 delay when other serial SWCs are not
                                 functioning
----------------------------------------------------------------------------*/
E_ERR_T CSWC::RequestAndReceiveWithModbusCheck(
               INT32 fd,
               UINT8 *ptxcmd,
               const UINT8 txcmdsz,
               VOID *prxbuff,
               const UINT32 expectedRxSz,
               const INT32 timeout_ms,
               const UINT8 retry,
               const UINT8 exceptionRetry,
               UINT8 *poutexceptionRetryCnt)
{
  E_ERR_T rstatus = E_ERR_Success;
  char acTemp[2048],acTemp1[2048],acTemp2[2048];
  int nI;
  INT8 ucretrycnt = 0;
  INT32 nactualRxSz;
  char* prxbuff1;
  //#ifdef CFG_PRN_ERR
  CHAR acErrMsg[100] = {0};
  //#endif // CFG_PRN_ERR
  INT32 nmodbusrval = 0;
  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_RQ_TX))
  struct timespec   tspec;
  struct tm         *pbdtime;
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
  MDB_READ_REPLY_T *preplypack;

  if(poutexceptionRetryCnt == TYP_NULL)
    return E_ERR_InvalidNullPointer;

  if((fd == TYP_ERROR) || (fd == TYP_NULL))
    return E_ERR_SWC_InvalidFileDescriptor;

  if(expectedRxSz > MDB_MAX_MODBUS_DATA_SZ)
    return E_ERR_SWC_ExceedBufferSize;

  do{
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC_RQ_TX)
    rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec, E_SYS_TIME_Local);
    printf("[SWC] %d.%02d.%02d %02d:%02d:%02d.%03d "
           "CSWC::RequestAndReceiveWithModbusCheck, %s\n"
           "  retry = %d/%d, tx %d bytes:\n",
           (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
           pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
           pbdtime->tm_sec, tspec.tv_nsec/1000000,
           m_acSWCName, (ucretrycnt + 1), retry, txcmdsz);
    SYS_PrnDataBlock((const UINT8 *)ptxcmd, txcmdsz, 10);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))

    memset(prxbuff, 0, expectedRxSz);
    rstatus = SER_SendWithReply(fd, ptxcmd, txcmdsz, prxbuff, expectedRxSz,
                                &nactualRxSz, timeout_ms);
    ERR_GetMsgString(rstatus, acErrMsg);



    //#if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_MODBUS)
    if((nactualRxSz != (INT32)expectedRxSz) || (rstatus != E_ERR_Success))
    {
//      printf("ERR [SWC] CSWC::R&R, %s, rstatus: %s, actualSize: %d, expectedSize: %d, retrycnt: (%d/%d)\n",
//    		  m_acSWCName, acErrMsg, nactualRxSz, expectedRxSz, ucretrycnt+1, retry);
      sprintf(acTemp, "ERR [SWC] CSWC::R&R, %s, rstatus: %s, actualSize: %d, expectedSize: %d, retrycnt: (%d/%d)",
    		  m_acSWCName, acErrMsg, nactualRxSz, expectedRxSz, ucretrycnt+1, retry);
      	g_pDebugLog->LogMessage(acTemp);
      //printf("WARN [SWC] CSWC::RequestAndReceiveWithModbusCheck, %s fd %d "
      //       "timeout %d ms, rstatus %d, rx lenght %d (expect %d) bytes:\n",
      //      m_acSWCName, fd, timeout_ms, rstatus, nactualRxSz, expectedRxSz);
      //SYS_PrnDataBlock((const UINT8 *)prxbuff, nactualRxSz, 20);
    }
    //#endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_MODBUS)

    if((rstatus == E_ERR_SER_SelectReadTimeOut) && ((ucretrycnt + 1) >= retry))
    {
      //#if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC)
      printf("WARN [SWC] CSWC::R&R, %s fd %d, timeout and exceed retry %d\n", m_acSWCName, fd, retry);
      sprintf(acTemp, "WARN [SWC] CSWC::R&R, %s fd %d, timeout and exceed retry %d", m_acSWCName, fd, retry);
      g_pDebugLog->LogMessage(acTemp);
      //#endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC)
      return E_ERR_SWC_ExceedRetryLimit;
    }

    if((rstatus == E_ERR_SER_SelectReadTimeOut) && (nactualRxSz == 0))
    {
    	printf("WARN [SWC] CSWC::R&R, %s fd %d, timeout=%d and retrycnt = %d, go back to retry loop\n",
    			m_acSWCName, fd, timeout_ms, ucretrycnt+1);
    	sprintf(acTemp, "WARN [SWC] CSWC::R&R, %s fd %d, timeout=%d and retrycnt = %d, go back to retry loop",
    			m_acSWCName, fd, timeout_ms, ucretrycnt+1);
    	      g_pDebugLog->LogMessage(acTemp);
        continue;
    }

    if(rstatus != E_ERR_Success)
      return rstatus;

    preplypack = (MDB_READ_REPLY_T *)prxbuff;

    // Do modbus check
    nmodbusrval = ModbusReplyCheck((UCHAR *)prxbuff, expectedRxSz,
                                   (UCHAR *)ptxcmd);

    if(nmodbusrval != MODBUS_MESSAGE_OK)
    {
    	printf("WARN [SWC] CSWC::R&R, %s, ModbusReplyCheck: 0x%04x\n", m_acSWCName, nmodbusrval);
    	sprintf(acTemp, "WARN [SWC] CSWC::R&R, %s, ModbusReplyCheck: 0x%04x", m_acSWCName, nmodbusrval);
    	      g_pDebugLog->LogMessage(acTemp);
    }

    switch(nmodbusrval)
    {
      case MODBUS_MESSAGE_OK:
        #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_RQ_RX))
        printf("[SWC] CSWC::RequestAndReceiveWithModbusCheck, rx %d bytes:\n",
               expectedRxSz);
        SYS_PrnDataBlock((const UINT8 *)prxbuff, expectedRxSz, 20);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
        *poutexceptionRetryCnt = 0;
        break;

      case MODBUS_ERROR_CRC:
        #ifdef CFG_PRN_WARN
        printf("WARN [SWC] CSWC::RequestAndReceiveWithModbusCheck, %s modbus "
               "CRC error. Expect Rx %d. Rx %d bytes:\n",
               m_acSWCName, expectedRxSz, nactualRxSz);
        SYS_PrnDataBlock((const UINT8 *) prxbuff, nactualRxSz, 20);
        #endif // CFG_PRN_WARN

        prxbuff1=(char*)prxbuff;
        for(nI=0; nI<txcmdsz; nI++)
          sprintf(&acTemp1[nI*2],"%02X",ptxcmd[nI]);

        for(nI=0; nI<nactualRxSz; nI++)
           sprintf(&acTemp2[nI*2],"%02X",prxbuff1[nI]);

        sprintf(acTemp,"ERR [SWC] CSWC::RequestAndReceiveWithModbusCheck, modbus crc error.",
        		"%s fd %d rx %d instead of %d bytes:\n",
        		"The request is: %s\n",
        		"The response is : %s\n",
                m_acSWCName, m_anCommfd[m_nPrimaryPortID], nactualRxSz, expectedRxSz,acTemp1,acTemp2);
        // Yang Tao 130820 : Add in delay to avoid consist CRC error.
        delay(50);
        rstatus = E_ERR_SWC_ModbusCRCError;
        break;

      case MODBUS_ERROR_EXCEPTION:
        #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_MODBUS)
        printf("WARN [SWC] CSWC::RequestAndReceiveWithModbusCheck, modbus "
               "exception error, 0x%04x\n",
               nmodbusrval);
        #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_MODBUS)
        if(exceptionRetry == 0)
        {
          #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
          printf("[SWC] CSWC::RequestAndReceiveWithModbusCheck, not "
                 "handling modbus exception error.\n  Exception retry cnt not "
                 "defined\n");
          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
          continue;
        }
        if(*poutexceptionRetryCnt < exceptionRetry)
        {
          (*poutexceptionRetryCnt)++;
          rstatus = E_ERR_SWC_ModbusExceptionError;
        }
        else
          return E_ERR_SWC_ExceedExceptionRetryLimit;
        break;

      default:
        #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_MODBUS)
        printf("WARN [SWC] CSWC::RequestAndReceiveWithModbusCheck, unhandled "
               "modbus exception error 0x%04x\n", nmodbusrval);
        #endif //((defined CFG_PRN_WARN) && CFG_PRN_WARN_SWC_MODBUS)
        rstatus = E_ERR_SWC_UnhandleModbusError;
    } // switch(nmodbusrval)

    if(rstatus != E_ERR_Success)
    {
    	ERR_GetMsgString(rstatus, acErrMsg);
    	printf("[SWC] CSWC::R&R: rstatus: %s, nmodbusval: 0x%04x\n, retrycnt: (%d/%d)\n",
    			acErrMsg, nmodbusrval, ucretrycnt+1, retry);
    	sprintf(acTemp, "[SWC] CSWC::R&R: rstatus: %s, nmodbusval: 0x%04x, retrycnt: (%d/%d)",
    			acErrMsg, nmodbusrval, ucretrycnt+1, retry);
    }
  }while((ucretrycnt++ < (retry - 1)) &&
         (nmodbusrval != MODBUS_MESSAGE_OK));

  if(rstatus != E_ERR_Success)
  {
	  ERR_GetMsgString(rstatus, acErrMsg);
	  printf("[SWC] CSWC::R&R: return to main polling, rstatus: %s\n", acErrMsg);
	  sprintf(acTemp, "[SWC] CSWC::R&R: return to main polling, rstatus: %s", acErrMsg);
  }

  return rstatus;
} // CSWC::RequestAndReceiveWithModbusCheck

//#ifdef CFG_MODULE_TEST
///*----------------------------------------------------------------------------
//
// PUBLIC ROUTINE
//
//  MDT_PrnToLog
//
// DESCRIPTION
//
//  This routine will print message to the designated log filename
//
// CALLED BY
//
//  Application
//
// CALLS
//
//  None
//
// PARAMETER
//
//  pfilename       [in] pointer to the designated log filename
//  pswcname        [in] pointer to SWC's name
//  plogmsg         [in] pointer to the message to be logged
//
//
// RETURN
//
//  E_ERR_Success                        the routine executed successfully
//
//
// AUTHOR
//
//  Bryan K.W. Chong
//
//
// HISTORY
//
//    NAME            DATE           REMARKS
//
// Bryan Chong      06-May-2011    Initial revision
//----------------------------------------------------------------------------*/
//void CSWC::MDT_PrnToLog(const CHAR *pfilename, CHAR *pswcname, CHAR *plogmsg)
//{
//  E_ERR_T rstatus = E_ERR_Success;
//  FILE *fd;
//  struct timespec   tspec;
//  struct tm         *pbdtime;
//
//  fd = fopen(pfilename, "a+");
//  if ( fd == TYP_NULL)
//  {
//    printf("ERR [SWC] CSWC::MDT_PrnToLog, error open file %s\n",
//           pfilename);
//  }
//
//  //time_t lTime = time(NULL);
//  rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec, E_SYS_TIME_Local);
//  fprintf(fd,"%d.%02d.%02d %02d:%02d:%02d.%03d %s %s\n",
//          (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
//          pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
//          pbdtime->tm_sec, tspec.tv_nsec/1000000,
//          pswcname, plogmsg);
//  fclose(fd);
//}// MDT_PrnToLog
//#endif // CFG_MODULE_TEST


/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SWC_MgmtThread

 DESCRIPTION

  This routine will manage the main process of SWC.

 CALLED BY



 CALLS

  None

 PARAMETER

  nIndex   [in] SWC index

 RETURN

   none

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    06-Apr-2010      Created initial revision
   Bryan Chong    17-May-2010      Update to include application exit flag

-----------------------------------------------------------------------------*/
VOID *SWC_MgmtThread(VOID *nIndex)
{

  INT32 swcidx;
  INT32 *pidx;
  pidx = (INT32 *)nIndex;
  swcidx = *pidx;

  if(swcidx > SWC_CLOCK_INDEX)
  {
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
    printf("[SWC] SWC_MgmtThread, invalid index %d\n",
           swcidx);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
    return 0;
  }

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
  printf("[SWC] SWC_MgmtThread, %s enter, index %d\n",
         g_apSWCObject[swcidx]->m_acSWCName, swcidx);
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)

  delay(500);
  while(1)
  {

    if(g_apSWCObject[swcidx]->m_bTaskWorking == false)
    {
      // cout<<"m_bTaskWorking ==false "<<endl;
      if(g_apSWCObject[swcidx]->SWC_main_timer_id != NULL)
      {
        wdCancel(g_apSWCObject[swcidx]->SWC_main_timer_id);
        timer_delete(g_apSWCObject[swcidx]->SWC_main_timer_id);
      }
      break;    //040227 Yu Wei
    }
    #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_SWC)
    if(g_apSWCObject[swcidx]->m_nSWCID == 1)
    {
      printf("[SWC] SWC_MgmtThread, %s enter MainProcess\n",
             g_apSWCObject[swcidx]->m_acSWCName);
    }
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
    g_apSWCObject[swcidx]->MainProcess();

    delay(PLC1_TASK_SLEEP);
    #if ((defined CFG_DEBUG_MSG) && (_CFG_DEBUG_SWC))
    printf("[SWC] SWC_MgmtThread, delay %d ms\n", PLC1_TASK_SLEEP);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))

    // 20100517 BC (Rqd by ZSL)
    if(bSWCExit == E_TYPE_Yes)
    {
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
      printf("[SWC] SWC_MgmtThread, exit SWC\n");
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC))
      break;
    }
  } // while(1)
  pthread_detach(g_apSWCObject[(int )nIndex]->m_nMainTaskID);
  delete g_apSWCObject[(int )nIndex];

  #ifdef CFG_PRN_ERR
  printf("ERR  [SWC] SWC_MgmtThread, thread exit\n");
  #endif // CFG_PRN_ERR

  return TYP_NULL;
} // SWC_MgmtThread

VOID *SWC_ServerListeningThread(VOID *nIndex)
{
  INT32 swcidx;
  INT32 *pidx;

  pidx = (INT32 *)nIndex;
  swcidx = *pidx;

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)
  printf("[SWC] SWC_ServerListeningThread, %s enter, index %d\n",
         g_apSWCObject[swcidx]->m_acSWCName, swcidx);
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SWC)

  if(g_apSWCObject[swcidx] == TYP_NULL)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [SWC] SWC_ServerListeningThread, invalid g_apSWCObject[%d]\n",
           swcidx);
    #endif // CFG_PRN_ERR
  }
//Yang Tao: Debug message for DO command.
  printf("[SWC] SWC_ServerListeningThread, g_apSWCObject[%d]\n",
         swcidx);

  delay(100);
  g_apSWCObject[swcidx]->m_pServerSocket->ListenTask();

  #ifdef CFG_PRN_ERR
  printf("ERR  [SWC] SWC_ServerListeningThread, thread exit\n");
  #endif // CFG_PRN_ERR

  return TYP_NULL;
} // SWC_ServerListeningThread
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SWC_InitNTPClient

 DESCRIPTION

  This routine will include NTP Client to SWC system

 CALLED BY

  SYS_Initialization

 CALLS

  None

 PARAMETER

  None

 RETURN

   E_ERR_Success      the routine executed successfully


 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    20-May-2009      Created initial revision
   Bryan Chong    30-Nov-2011      Update nTotalEnableSwc counter for NTP
                                   Client
-----------------------------------------------------------------------------*/
static E_ERR_T SWC_InitNTPClient(VOID)
{

  E_ERR_T rstatus = E_ERR_Success;

  rstatus = SYS_ThrdInit(E_SYS_TLB_SWC_NTPClientComm);
  if(rstatus != E_ERR_Success)
  {
    #ifdef CFG_DEBUG_MSG
    printf("ERR  [SWC] SWC_InitNTPClient, create external system "
           "communication  thread fail\n");
    #endif // CFG_DEBUG_MSG
    return rstatus;
  }

  g_tRTUConfig.nTotalEnableSwc++;
  return E_ERR_Success;
} // SWC_InitNTPClient

/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SWC_NTPClientCommunicationThrd

 DESCRIPTION

  This routine will manage the communication between SWC and NTP client. The
  thread will receive link status update from NTP_LinkManagementThrdFunc
  through message queue E_SYS_MSQL_NTPC_TxExternalSystemMsg

 CALLED BY

  SWC_InitNTPClient  initializes NTP Client threads

 CALLS

  None

 PARAMETER

  pThreadParameter   [in] dummy parameter to comply with thread function format

 RETURN

   None

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                REMARKS

   Bryan Chong    21-May-2010      Create initial version
                                   Update NTP link status accordingly will
                                   update RTU status table [PR42]
   Bryan Chong    01-Oct-2010      Update NTP link status to SWC status table
                                   in order to reflect link status in RMT
-----------------------------------------------------------------------------*/
VOID *SWC_NTPClientCommunicationThrd(VOID *pThreadParameter)
{
  E_ERR_T rstatus = E_ERR_Success;
  INT32 msglen;
  struct msq_ctrl_st *pcb;
  struct mq_attr  mqstat;
  SYS_MSGQ_MCPR  msgq, msgqrx;
  NTP_MSGQ_MCP_STATUS *pmcpstatus;
  UINT8             svrcnt;
  #ifdef CFG_PRN_ERR
  CHAR errBuff[100] = {0};
  #endif // CFG_PRN_ERR

  #if((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP)
  struct tm         *pbdtime;
  struct timespec   tspec;
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP)

  if((pSYS_CB == TYP_NULL) || (pSYS_CB->msq_ctrl == TYP_NULL))
  {
    #ifdef CFG_PRN_ERR
    printf("ERR [SWC] SWC_NTPClientCommunicationThrd, invalid null pointer\n");
    #endif // CFG_PRN_ERR
    return TYP_NULL;
  }

  pcb = &pSYS_CB->msq_ctrl[E_SYS_MSQL_NTPC_TxExternalSystemMsg];

  // get message queue attribute
  mq_getattr(pcb->msq_id, &mqstat);

  // construct MCP message
  memcpy(msgq.header, SYS_MCP_HEADER, sizeof(msgq.header));
  msgq.msgtype = E_SYS_MCP_SWCToNTPClient;
  msgq.msgsz = sizeof(SYS_MSGQ_MCPR);
  msgq.msgbuff[0] = E_NTP_MCP_CMD_RequestForStatus;

  // wait SWC object to be created for NTPCli
  while(g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX] == TYP_NULL)
    delay(0);

  g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_nTotalLinkNumber =
    CGF_NTP_TOTAL_SERVER;
  g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_nMainTaskID =
    g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_nPrimaryPortID = 0;
  g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_nStandbyPortID = 1;

  while(1)
  {
    memset(&msgqrx, 0, sizeof(SYS_MSGQ_MCPR));
    msglen = mq_receive(pcb->msq_id, (CHAR *)&msgqrx, mqstat.mq_msgsize, NULL);
    if(msglen < 0)
    {
      #ifdef CFG_PRN_WARN
      printf("WARN [SWC] SWC_NTPClientCommunicationThrd, message queue reply "
             "error, %d\n", msglen);
      #endif // CFG_PRN_WARN
      continue;
    }

    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_NTP_SWC_RXRESP))
    printf("[SWC] SWC_NTPClientCommunicationThrd, rx %d bytes message queue:\n",
           msglen);
    SYS_PrnDataBlock((const UINT8 *) &msgqrx, msglen, 10);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP))

    pmcpstatus = (NTP_MSGQ_MCP_STATUS *)msgqrx.msgbuff;

    if(rstatus != E_ERR_Success)
    {
      #ifdef CFG_PRN_ERR
      ERR_GetMsgString(rstatus, errBuff);
      printf("[SWC] SWC_NTPClientCommunicationThrd, swap byte error, %s\n",
             errBuff);
      #endif // CFG_PRN_ERR
      continue;
    }

    // initialize master and backup NTP link status to invalid
    g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_anLinkStatus[0] = LINKERROR;
    g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_anLinkStatus[1] = LINKERROR;

    // 20101001 BC (Rqd ZSL)
    // update NTP link status
    if(pmcpstatus->activeServer != TYP_ERROR)
    {
      // update link status for backup NTP servers. If any of the backup server
      // link status is healthy, set line 2 status to healthy
      for(svrcnt = 0; svrcnt < CGF_NTP_TOTAL_SERVER; svrcnt++)
      {

        #if((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP)
        rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec,
                                             E_SYS_TIME_Local);
        printf("[SWC] %d.%02d.%02d %02d.%02d.%02d.%03d "
               "SWC_NTPClientCommunicationThrd,\n"
               "  server %d connection at %s is %s\n",
               (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
               pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
               pbdtime->tm_sec, tspec.tv_nsec/1000000,
               svrcnt, pNTP_CB->server[svrcnt].ipaddr,
               pmcpstatus->server[svrcnt].isConnectionGood ? "OK" : "NG");
        #endif //((defined CFG_DEBUG_MSG) || (CFG_DEBUG_NTP))


        if(pmcpstatus->server[svrcnt].isConnectionGood == E_TYPE_Yes)
        {
           if(svrcnt == 0)
           {
             g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_anLinkStatus[0] =
               LINKOK;
             continue;
           }

           if(svrcnt != 0)
           {
             g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_anLinkStatus[1] =
               LINKOK;
             break;
           }
        } // if(pmcpstatus->server[svrcnt].isConnectionGood == E_TYPE_Yes)
      } // for(svrcnt = 0; svrcnt < CGF_NTP_TOTAL_SERVER; svrcnt++)
    }// if(pmcpstatus->activeServer == TYP_ERROR)

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP)
    rstatus = SYS_GetCurrentTimeInString(&pbdtime, &tspec, E_SYS_TIME_Local);
    printf("[SWC] %d.%02d.%02d %02d.%02d.%02d.%03d "
           "SWC_NTPClientCommunicationThrd,\n"
           "  m_anLinkStatus[0] = %s, [1] =  %s\n",
           (pbdtime->tm_year + 1900), (pbdtime->tm_mon + 1),
           pbdtime->tm_mday, pbdtime->tm_hour, pbdtime->tm_min,
           pbdtime->tm_sec, tspec.tv_nsec/1000000,
           (g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_anLinkStatus[0] ==
              LINKOK) ? "LINKOK" : "LINKERROR",
           (g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_anLinkStatus[1] ==
              LINKOK) ? "LINKOK" : "LINKERROR");
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP)

    g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->CheckLinkStatusChange();

    memset(&msgqrx, 0, sizeof(SYS_MSGQ_MCPR));

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP)
    printf("[SWC] SWC_NTPClientCommunicationThrd,\n"
           "  m_nFastExcepCounter = %d, "
           "m_nSlowExcepCounter = %d, m_nTimerExcepCounter = %d,\n"
           "  m_nExcepRetry = %d\n",
           g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_nFastExcepCounter,
           g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_nSlowExcepCounter,
           g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_nTimerExcepCounter,
           g_apSWCObject[CFG_NTP_CLIENT_SWC_INDEX]->m_nExcepRetry);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_NTP)
  } // while (1)

  #ifdef CFG_PRN_ERR
  printf("ERR  [SWC] SWC_NTPClientCommunicationThrd, thread exit\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_SWC_NTPC))
} // SWC_NTPClientCommunicationThrd

//#ifdef NOT_USED
/*----------------------------------------------------------------------------

 PRIVATE ROUTINE

  swc_setSerialAttr

 DESCRIPTION

  This routine will set serial link attribute

 CALLED BY

  CSWC::CSWC

 CALLS

  None

 PARAMETER

  fd              [in] file descriptor
  BaudRate        [in] baud rate
  ucParity        [in] parity
  DataBits        [in] data bit setting
  StopBits        [in] stop bit setting

 RETURN

  1                the routine executed successfully
  -1               error

 AUTHOR

  Inherited from original release, unknown author


 HISTORY

    NAME            DATE           REMARKS

 Bryan Chong      14-May-2012    Initial revision
----------------------------------------------------------------------------*/
static int swc_setSerialAttr(int fd , speed_t BaudRate ,unsigned int ucParity ,
                             unsigned int DataBits ,unsigned  int StopBits)
{

  struct termios termios_p;
  speed_t baud = BaudRate;


  //get current settings
  if((tcgetattr(fd, &termios_p)) == -1)
  {
    printf("Error get attr: %s\n", strerror(errno));
    return -1;
  }

  //set attr from parameters
  //set input baud rate
  if((cfsetispeed(&termios_p, baud)) == -1)
  {
    printf( "Error set ibuadrate: %s\n", strerror(errno));
    return -1;
  }

  //set output baud rate
  if((cfsetospeed(&termios_p, baud)) == -1)
  {
    printf( "Error set obuadrate: %s\n", strerror(errno));
    return -1;
  }

  //check parity
  switch(ucParity)
  {
    case 0:  //parity disables
      termios_p.c_cflag &= (~PARENB);
      termios_p.c_cflag &= (~PARODD);
      break;

    case 1:  //odd parity
      termios_p.c_cflag |= PARENB;
      termios_p.c_cflag |= PARODD;
      break;

    case 2:  //even parity
      termios_p.c_cflag |= PARENB;
      termios_p.c_cflag &= (~PARODD);
      break;

    default: //parity disable
      termios_p.c_cflag &= (~PARENB);
      termios_p.c_cflag &= (~PARODD);
      break;
  }

  termios_p.c_cflag &= (~CSIZE);
  //set data bit
  switch(DataBits)
  {
    case 5:
      termios_p.c_cflag |= CS5;
      break;

    case 6:
      termios_p.c_cflag |= CS6;
      break;

    case 7:
      termios_p.c_cflag |= CS7;
      break;

    case 8:
      termios_p.c_cflag |= CS8;
      break;

    default:
      fprintf(stderr, "Unrecognized data size\n");
      return -1;
  }

  //set stop bit
  switch(StopBits)
  {
    case 1:
      termios_p.c_cflag &= (~CSTOPB);
      break;

    case 2:
      termios_p.c_cflag |= CSTOPB;
      break;

    default:
      printf( "Unrecognized stop bit\n");
      return -1;
  }

  if((tcsetattr(fd, TCSANOW, &termios_p)) == -1)
  {
    printf( "Error set attr: %s\n", strerror(errno));
    return -1;
  }

  return 1;
}
//#endif // NOT_USED
//
//#ifdef CFG_DEBUG_SWC_MODULE_TEST
//void CSWC::LogSwcInitValues(char* pName)
//{
//  int i,j;
//  FILE *fd;
//
//  char pChar[100];
//  strcpy(pChar, pName);
//  strcat(pChar, "In.txt");
//
//  printf("Start write %s\n",pChar);
//
//  fd = fopen(pChar, "w+");
//  if ( fd == TYP_NULL) {
//    sprintf(pChar, "Can not save swc init values to %sIn.txt.\r\n", pName);
//    printf(pChar);
//    return;
//  }
//  //write time
//  time_t lTime = time(NULL);
//  fprintf(fd,"\t%s values after initialzation\r\n:::::::::::::::::::::%s::::::::::::::::::::::\r\n", pName, ctime(&lTime));
//
//  //write values
//  fprintf(fd,"m_acSWCName[10]: %s\r\n", m_acSWCName);
//
//  fprintf(fd,"int m_nSWCID:%d\r\n", m_nSWCID);            //Store SWC ID Index.
//
//  fprintf(fd,"int m_nReceiveTimeout:%d\r\n", m_nReceiveTimeout);          //Timeout to wait for SWC response.
//
//  fprintf(fd,"int m_nMainTaskID:%d\r\n", m_nMainTaskID);            //SWC main task ID.
//  fprintf(fd,"int m_nServerSocketTaskID:%d\r\n",m_nServerSocketTaskID);        //SWC server socket task ID.
//
//
//  fprintf(fd,"int m_bTaskWorking:%d\r\n", m_bTaskWorking);          //For stoping the task.
//                      //false -- stop task. true -- task live.
//
//  fprintf(fd,"int m_nLinkType:%d\r\n", m_nLinkType);            //Link type: RS_485/RS_422/RS_232,
//                      //       RS_485M/RS_422M multidrop,
//                      //       LAN.
//
//  fprintf(fd,"int m_bPollingEnable:%d\r\n", m_bPollingEnable);          //The flag to enable/inhibit SWC polling.
//
//  fprintf(fd,"int m_nSWCID[0]:%d\r\n", m_anCommfd[0]);            //Serial communication port file descriptor.
//  fprintf(fd,"int m_nSWCID[1]:%d\r\n", m_anCommfd[1]);
//
//  fprintf(fd,"tPollingType m_atPollingFlag[0].bFast:%d, bSlow:%d, bStatus:%d, bSpecial:%d, bLinkCheck:%d, bTimeSyn:%d\r\n",
//    m_atPollingFlag[0].bFast, m_atPollingFlag[0].bSlow, m_atPollingFlag[0].bStatus, m_atPollingFlag[0].bSpecial,
//    m_atPollingFlag[0].bLinkCheck, m_atPollingFlag[0].bTimeSyn);    //Flag for polling status.
//  fprintf(fd,"tPollingType m_atPollingFlag[1].bFast:%d, bSlow:%d, bStatus:%d, bSpecial:%d, bLinkCheck:%d, bTimeSyn:%d\r\n",
//    m_atPollingFlag[1].bFast, m_atPollingFlag[1].bSlow, m_atPollingFlag[1].bStatus, m_atPollingFlag[1].bSpecial,
//    m_atPollingFlag[1].bLinkCheck, m_atPollingFlag[1].bTimeSyn);    //Flag for polling status.
//
//  fprintf(fd,"int m_anRetryNumberStatus[0]:%d\r\n", m_anRetryNumberStatus[0]);      //Buffer for re-try times when polling status.
//  fprintf(fd,"int m_anRetryNumberStatus[1]:%d\r\n", m_anRetryNumberStatus[1]);
//
//  fprintf(fd,"int m_nSWCCurrentStateFlag:%d\r\n", m_nSWCCurrentStateFlag);        //Current SWC working state.
///*
//  fprintf(fd,"unsigned short  m_aunSWCTableFast[2048]:\r\n       ");  //Buffer for fast polling table.
//  for (i=0; i< 2048;i++) fprintf(fd,"%X ", m_aunSWCTableFast[i]);
//
//  fprintf(fd,"\r\nunsigned short  m_aunSWCTableSlow[2048]:\r\n       ");  //Buffer for slow polling table.
//  for (i=0; i< 2048;i++) fprintf(fd,"%X ", m_aunSWCTableSlow[i]);
//
//  fprintf(fd,"\r\nunsigned short  m_aunSWCTableTimer[2048]:\r\n       ");
//  for (i=0; i< 2048;i++) fprintf(fd,"%X ", m_aunSWCTableTimer[i]);
//*/
//  fprintf(fd,"\r\nint m_anLinkStatus[0]:%d\r\n", m_anLinkStatus[0]);          //Record link status. (Link 0 or 1)
//  fprintf(fd,"int m_anLinkStatus[1]:%d\r\n", m_anLinkStatus[1]);
//  fprintf(fd,"int m_nMasterLinkID:%d\r\n",  m_nMasterLinkID);          //Store master link ID.
//
//  fprintf(fd,"unsigned short m_unCurrentLinkStatus:%d\r\n",  m_unCurrentLinkStatus);  //Word for link status for checking if the link status is changed.
//
//  fprintf(fd,"MSG_Q_ID  m_tMSGQID:%d\r\n", m_tMSGQID);          //Message queue ID.
//
//  fprintf(fd,"int m_nFastPollingValue:%d\r\n",  m_nFastPollingValue);        //Fast polling interval in ms.
//  fprintf(fd,"int m_nSlowPollingValue:%d\r\n",  m_nSlowPollingValue);        //Slow polling interval in ms.
//  fprintf(fd,"int m_nTimerPollingValue:%d\r\n",  m_nTimerPollingValue);        //Timer polling interval in minutes.
//  fprintf(fd,"int m_nRetryNumber:%d\r\n",  m_nRetryNumber);            //Re-try times if link no response.
//  fprintf(fd,"int m_nStandbyPollingValue:%d\r\n",  m_nStandbyPollingValue);        //Standby link status checking interval in ms.
//  fprintf(fd,"int m_nStandbyPollingFailedValue:%d\r\n",  m_nStandbyPollingFailedValue);    //When link failed, link status checking interval in ms.
//
//  fprintf(fd,"char m_acFastPollingCommand[0][16]:\r\n    ");  //Buffer for fast polling command.
//  for(i=0; i<16;i++) fprintf(fd,"%X ", m_acFastPollingCommand[0][i]);
//  fprintf(fd,"\r\nchar m_acFastPollingCommand[1][16]:\r\n    ");
//  for(i=0; i<16;i++) fprintf(fd,"%X ", m_acFastPollingCommand[1][i]);
//
//  fprintf(fd,"\r\nchar m_acSlowPollingCommand[0][16]:\r\n    " );    //Buffer for slow polling command.
//  for(i=0; i<16;i++) fprintf(fd,"%X ", m_acSlowPollingCommand[0][i]);
//  fprintf(fd,"\r\nchar m_acSlowPollingCommand[1][16]:\r\n    ");
//  for(i=0; i<16;i++) fprintf(fd,"%X ", m_acSlowPollingCommand[1][i]);
//
//  fprintf(fd,"\r\nchar m_acTimerPollingCommand[0][16]:\r\n    " );  //Buffer for timer polling command.
//  for(i=0; i<16;i++) fprintf(fd,"%X ", m_acTimerPollingCommand[0][i]);
//  fprintf(fd,"\r\nchar m_acTimerPollingCommand[1][16]:\r\n    ");
//  for(i=0; i<16;i++) fprintf(fd,"%X ", m_acTimerPollingCommand[1][i]);
///*
//  fprintf(fd,"\r\nm_acWriteCommand[2048]:\r\n    " );      //Buffer for specific command that will send to SWC from server.
//  for (i=0; i< 2048;i++) fprintf(fd,"%X ", m_acWriteCommand[i]);
//*/
//  fprintf(fd,"\r\nchar m_acStatusCommand[0][16]\r\n    ");      //Buffer for link status checking command.
//  for(i=0; i<16;i++) fprintf(fd,"%X ", m_acStatusCommand[0][i]);
//  fprintf(fd,"\r\nchar m_acStatusCommand[1][16]:\r\n    ");
//  for(i=0; i<16;i++) fprintf(fd,"%X ", m_acStatusCommand[1][i]);
//
//  fprintf(fd,"\r\nint m_bTimeSynFlag:%d\r\n", m_bTimeSynFlag);          //If true, time synchronization valid.
//  fprintf(fd,"int m_bTimeSynStartFlag:%d\r\n", m_bTimeSynStartFlag);      //Re-start to set, when time synchronization had been implemented,
//                      //set to false.
//  fprintf(fd,"int m_nSendTimeSynHour:%d\r\n", m_nSendTimeSynHour);        //Hour which the time synchronization message is send.
//  fprintf(fd,"int m_nSendTimeSynMinute:%d\r\n",   m_nSendTimeSynMinute);      //Minute which the time synchronization message is send.
//  fprintf(fd,"int m_nTimeSynCMDLength:%d\r\n", m_nTimeSynCMDLength);        //Command size for time synchronization.
//  fprintf(fd,"m_acTimeSynCommand[0][264]:\r\n    " );    //Buffer for time synchronization command.
//  for(i=0; i<264;i++) fprintf(fd,"%X ", m_acTimeSynCommand[0][i]);
//  fprintf(fd,"\r\nm_acTimeSynCommand[1][264]:\r\n    " );
//  for(i=0; i<264;i++) fprintf(fd,"%X ", m_acTimeSynCommand[1][i]);
//
//  //Fast polling parameter.
//  fprintf(fd,"\r\nint m_nFastPollingRecevLen:%d\r\n",  m_nFastPollingRecevLen);        //Fast polling receiving data length.
//  fprintf(fd,"int m_nFastDataLen:%d\r\n",  m_nFastDataLen);            //Fast polling command length.
//  fprintf(fd,"int m_nServerFastTableStartAddr:%d\r\n",  m_nServerFastTableStartAddr);    //Fast polling server table start address.
//  fprintf(fd,"int m_nServerFastTableLen:%d\r\n",  m_nServerFastTableLen);        //Fast polling server table length.
//  //fprintf(fd,"int m_bFastPollingFlag:%d\r\n",  m_bFastPollingFlag);        //Flag whether the fast polling implements in the SWC.
//
//  //Slow polling parameter.
//  fprintf(fd,"int m_nSlowPollingRecevLen:%d\r\n",  m_nSlowPollingRecevLen);        //Slow polling receiving data length.
//  fprintf(fd,"int m_nSlowDataLen:%d\r\n",  m_nSlowDataLen);            //Slow polling command length.
//  fprintf(fd,"int m_nServerSlowTableStartAddr:%d\r\n",  m_nServerSlowTableStartAddr);    //Slow polling server table start address.
//  fprintf(fd,"int m_nServerSlowTableLen:%d\r\n",  m_nServerSlowTableLen);        //Slow polling server table length.
//  fprintf(fd,"int m_bSlowPollingFlag:%d\r\n",  m_bSlowPollingFlag);        //Flag whether the slow polling implements in the SWC.
//
//  //Timer polling parameter.
//  fprintf(fd,"int m_nTimerPollingRecevLen:%d\r\n",  m_nTimerPollingRecevLen);      //Timer polling receiving data length.
//  fprintf(fd,"int m_nTimerDataLen:%d\r\n",  m_nTimerDataLen);          //Timer polling command length.
//  fprintf(fd,"int m_nServerTimerTableStartAddr:%d\r\n",  m_nServerTimerTableStartAddr);    //Timer polling server table start address.
//  fprintf(fd,"int m_nServerTimerTableLen:%d\r\n",  m_nServerTimerTableLen);        //Timer polling server table length.
//  fprintf(fd,"int m_bTimerPollingFlag:%d\r\n",  m_bTimerPollingFlag);        //Flag whether the timer polling implements in the SWC.
//
//  fprintf(fd,"int m_nStatusPollingRecevLen:%d\r\n",  m_nStatusPollingRecevLen);  //Link status polling response message size.
//  fprintf(fd,"int m_nSpecificCommandLen:%d\r\n",  m_nSpecificCommandLen);    //Length of specific command
//
//  fprintf(fd,"unsigned char m_ucSWCAddress:%d\r\n", m_ucSWCAddress);  //SWC slave address. For server special command
//
//  fprintf(fd,"SEM_ID  m_tAccessTableSemID:%d\r\n",   m_tAccessTableSemID);  //Semiphone ID for table accessing.
//
//  fprintf(fd,"struct timespec m_atTimeOut[MAX_TIMEOUT_TYPE]:\r\n");  //Buffer for timer counter.
//  for (i=0;i<MAX_TIMEOUT_TYPE;i++) fprintf(fd, "   %d: sec:%d  nsec:%d\r\n",i, m_atTimeOut[i].tv_sec,m_atTimeOut[i].tv_nsec);
//
//  fprintf(fd,"int m_nTotalLinkNumber:%d\r\n",  m_nTotalLinkNumber);      //Total serial link number. (1 or 2)
//  fprintf(fd,"int m_nPrimaryPortID:%d\r\n",  m_nPrimaryPortID);      //Primary link ID (0 or 1)
//  fprintf(fd,"int m_nStandbyPortID:%d\r\n",  m_nStandbyPortID);      //Standby link ID (0 or 1)
//
//  fprintf(fd,"\r\nint m_anStandbyRecevLen[0]:%d\r\n", m_anStandbyRecevLen[0]);    //Received data length.
//  fprintf(fd,"int m_anStandbyRecevLen[1]:%d\r\n", m_anStandbyRecevLen[1]);
//  fprintf(fd,"int m_anStandbyMonitorTimeout[0]:%d\r\n",  m_anStandbyMonitorTimeout[0]);  //Timeout ID for standby RTU monitoring link status.
//  fprintf(fd,"int m_anStandbyMonitorTimeout[1]:%d\r\n",  m_anStandbyMonitorTimeout[1]);
//
//  fprintf(fd,"int m_bLinkCheckFlag:%d\r\n", m_bLinkCheckFlag);    //Link check flag.
//                //If the flag is set in standby RTU, start to check link.
//                //When the link checking completed, the flag will be reset.
//                //When SWC reveived "Stop Link" message in primary RTU, the flag is set and stop polling.
//                //After standby RTU copmleted the link checking, the primary will receive
//                //"Start Link" message, the flag will be reset and start polling.
//
//  /*  //The type and variable are removed. 040322 Yu Wei
//  fprintf(fd,"tSWCTableStructure m_tInputTable[SWC_MAX_TABLE]:\r\n");  //SWC input table.
//  for (i=0;i<SWC_MAX_TABLE;i++) fprintf(fd, "    %d: unStartAddress:%d, nTableLen:%d\r\n",i, m_tInputTable[i].unStartAddress, m_tInputTable[i].nTableLen);
//  */
//
//  fprintf(fd,"unsigned short m_unFastPollingStartAdd:%d\r\n", m_unFastPollingStartAdd);        //Fast polling start address.
//  fprintf(fd,"unsigned short m_unSlowPollingStartAdd:%d\r\n",  m_unSlowPollingStartAdd);        //Slow polling start address.
//
//
//  fprintf(fd,"WDOG_ID    m_tWatchdogID:%d\r\n", m_tWatchdogID);  //Watchdog ID.
//
//  fprintf(fd," CListen *m_pServerSocket:%X\r\n", m_pServerSocket);  //Object for server interface.
//
//  lTime = time(NULL);
//
//  fprintf(fd,"\r\n:::::::::::::::::::::End: %s::::::::::::::::::::::\r\n",
//          ctime(&lTime));
//
//  fclose(fd);
//
//  printf("End write %s\n",pChar);
//
//}
//#endif // CFG_DEBUG_SWC_MODULE_TEST


