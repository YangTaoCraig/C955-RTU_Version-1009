/************************************************************
*                              *
*  Project:    C830 RTU                  *
*  Module:    Initialization                *
*  File :      Init_Config.cpp                *
*  Author:    Yu Wei                    *
*                              *
*  Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************


**************************************************************

DESCRIPTION

  This file will read configuration file (config.txt) and decode
  the information, then store in the structure g_tRTUConfig
*************************************************************
Modiffication History
---------------------

Version: INM D.1.1.1
  01  1 April 2003, Yu Wei,
    Start to write.

Version: INM D.1.1.2
  01  2 June 2003,
    Added debug log flag.
    Added server polling socket ID. The max socket ID is 5
    Deleted PA and CCTV. The max SWC number is 20.

  02  1 July 2003, Yu Wei
    The data field format is not fix to Hex or decimal.
    The prefix with "0x" is hex value. Otherwise is decimal
    value.

Version: INM D.1.3.0
  01  01 Auguest 2003, Yu Wei
    All SWC use same class except Clock.

  02  14 August 2003, Yu Wei
    Added SWC time synchronization.

  03  18 August 2003, Yu Wei
    Change trig polling to timer polling.

  04  01 September 2003, Yu Wei
    RTU identification is fixed to odd number for RTU1 and
    even number for RTU2.

Version: INM D.1.3.1
  01  18 September 2003, Yu Wei
    Modify GetSWCPollingAddress. Don't search "BGN_W".
    Refer to PR:OASYS-PMS 0001.

  02  24 September 2003, Yu Wei
    Remove nCommandTimeoutP, nCommandTimeoutS and nServerNotion.

Version: INM D.1.3.2
  01  01 Ocrober 2003, Yu Wei
    Modify GetVDUName() and GetVDUPassword().

  02  02 October 2003, Yu Wei
    Changed tSWCTableStructure for MMM.
    Deleted nTableNumber.
    Modify GetSWCTable();

  03  06 October 2003, Yu Wei
    Deleted all not used variables:
    tRTUPollingTable, (modify GetAllRTUTable())
    tRTUTableStructure.acTableName,
    tRTUTableStructure.nTableID,
    tModbusAddress.unEnd,
    tModbusAddress.unServerEnd,
    tSWCCFGHeaderStructure.nWireNumber,
    tLANParameter.acLAN_Protocol,
    tSerialPortPara.nStartBit,
    tSerialPortPara.acProtocol,

    Moved SER_acSerialPortName[24] to common.cpp,

    Changed tSerialPortPara.acPortName[16] to
    tSerialPortPara.unPortID.

  04  12 November 2003, Yu Wei
    SWC_MAX_TABLE change to 4.
    MI table that is polled by slow polling uses 300 as table ID.
    Table ID = 450 is not used.

  05  18 November 2003, Yu Wei
    Modify GetRTUConfig() to parse RTU_LAN1_GWAY and RTU_LAN2_GWAY parameters.
    Refer to PR:OASYS-PMS 0019.

  06  20 November 2003, Yu Wei
    Added SWC_CLOCK_INDEX for clock index.
    Modified GetAllSWCParameter().
      g_tRTUConfig.anSWCIndex[SWC_CLOCK_INDEX] = SWC_CLOCK;

  07  21 November 2003, Yu Wei
    Modified GetAllSWCParameter(). anMultidorpSWCChain is used to store
    multidrop link SWC information.

Version: INM D.1.3.3
  01  15 January 2004, Yu Wei
    Added g_bCFGFileErrorLogFlag to log config.txt file error.

  02  27 January 2004, Yu Wei
    Fix bug in GetDigit().
    Refer to PR:OASYS-PMS 0030.

Version: INM D.1.3.5
  01  09 March 2004, Yu Wei
    Fixed bug in GetVDUName() and GetVDUPassword().
    The VDU name and password length can be 0 or greater than VDU_MAX_USER (10).
    The loop will not terminat.
    Refer to PR:OASYS-PMS 0040.
    Refer to PR:OASYS-PMS 0047.

    Fixed bug in GetValue(). If there is no value field, return error.
    Refer to PR:OASYS-PMS 0047.

    Fixed bug in GetAllRTUTable(). No RTU_STATUS, no error return.
    When the key word "BGN_SERVER_CFG" dose not exist, the while will not break.
    Refer to PR:OASYS-PMS 0047.

    Socket ID change to unsigned short instead of int.
    Refer to PR:OASYS-PMS 0047.

    The config.txt must include fast polling parameters,
    otherwise systrm will return error.
    Modified SearchSWCTable().
    Refer to PR:OASYS-PMS 0047.

  02  11 March 2004, Yu Wei
    Change 'strncmp' to 'strcmp' for value string comparing exectly.
    Modified GetLinkType(), GetDeviceIndex().

    Fixed bug in ReadOneLine(). If End-Of-File, it should return a NULL string.
    Refer to PR:OASYS-PMS 0047.

  03  15~25 March 2004, Yu Wei
    (A) Change g_bCFGFileErrorLogFlag to g_nCFGFileErrorLogFlag.
    The debug log can write to file or printf. (Improvement)
    Added WriteDebugLog().
    Modified follow routines:
      1. GetSystemConfig().
      2. ReadOneLine().
      3. SearchKeyWord().
      4. GetVDUPassword().
      5. GetAllRTUTable().
      6. GetSWCConfig().
      7. GetSWCParameter().
      8. SearchSWCTable().
      9. SearchSWCTable().

    (B) Added g_unProtocolType.  To store and check protocol type.
    Modified GetLinkProtocol().
    Refer to PR:OASYS-PMS 0047.

    (C) Added g_acServerTableMap[64*1024] to check server polling overlap.
    Added AllocateServerTable(), AllocateSWCPollingTable().
    Modified GetSystemConfig().
    Refer to PR:OASYS-PMS 0047.

    (D) Added g_acSWCKeyWordName[SWC_MAX_NUMBER] for SWC index searching and
    reducing complexity.
    Modified GetSWCConfig().
    Refer to PR:OASYS-PMS 0044.

    (E)Added boundary check for configuration value.
    Refer to PR:OASYS-PMS 0047.
      1. Modified GetValue() to remove ' ' in the beginning and end.
      2. Added DeleteBlank() to remove ' ' in the beginning and end.
      3. Re-design GetDigit(). Added GetDecimalDigit() and GetHexDigit().
         Modified all GetDigit() caller.
      4. Modified GetIntegerValue(), GetuShortValue(), GetCharValue().
      5. Added GetIPAddrValue().
      6. Changed name GetVersion() to GetCFGVersion() and re-designed.
      7. Modified GetOtherRTUIPAddress().
      8. Modified GetRTUConfig().
      9. Modified GetLinkProtocol(). The acProtocol is not used.
            10.Modified GetAllRTUTable().
      11.Modified GetRTUTable()
      12.Modified GetAllSWCParameter() to check multidrop link SWC's group conflict.
      13.Re-designed GetSWCConfig().
      14.Modified GetSerialPortPara().
      15.Modified GetSWCParameter().
      16.Re-designed GetSWCNSvrTable().
      17.Added GetSWCTable().
      18.Modified SearchSWCTable() to check if the fast polling table exists.
      19.Added AllocateServerTable().
      20.Added AllocateSWCPollingTable().
      21.Added SerialPortIDCheck() for srieal port ID conflict check.

    (F) To reduce complexity
    Refer to PR:OASYS-PMS 0044.
      1. Added GetDebugInfo() and changed GetVersion() to GetCFGVersion().
      2. Modified GetRTUConfig() and added GetLink1Info(), GetLink2Info().
      3. Modified GetSWCConfig().
      4. Modified GetSWCParameter() and
         added GetLinkSetting(), GetSWCPollingTimeInfo().
        5. Changed GetSWCTable() to GetSWCNSvrTable().
         Modified GetSWCNSvrTable().
         Added GetSWCTable().

    (G) Config.txt changed. SWC_ADDRESS_AREA is removed. The fast, slow and timer
      polling table address is read from SWC_Table. (Improvement)
      1. Modified GetSWCParameter().
      2. Modified GetSWCNSvrTable().
      3. Removed GetSWCPollingAddress().
      4. Changed name SearchSWCTable_PollingAddress() to SearchSWCTable().

  04  29 March 2004, Yu Wei
    Config.txt file added LOCAL_TIME_ZONE for UTC time convert to local time.
    Added GetLocalTimeZone() and modified GetRTUConfig().
    Refer to PR:OASYS-PMS 0151.

Version: INM D.1.3.6
  01  21 April 2004, Yu Wei
    Modified GetIPAddrValue() to fixed bug. When IP address error, the buffer was
    not cleared, RTU will use wrong IP to set LAN IP.
    Refer to PR:OASYS-PMS 0165.

  02  29 April 2004, Yu Wei
    Modified GetVDUName() and GetVDUPassword() to fixed bug. The last user
    name and password's length is not checked.
    Refer to PR:OASYS-PMS 0179.

  03  29 April 2004, Yu Wei
    Modified GetAllSWCParameter().
        The multidrop device must include two device.
    Refer to PR:OASYS-PMS 0181.

  04  12 May 2004, Yu Wei
    Modified GetAllRTUTable(). Deleted tRTUPollingTable and g_tRTUConfig
    to store polling table address.
    Refer to PR:OASYS-PMS 0196.

  05  25 May 2004, Yu Wei
    Modified GetSWCPollingTimeInfo() to get nGreacTime.
    Refer to PR:OASYS-PMS 0213.

  06  04 June 2004, Yu Wei
    Modified GetLink1Info() and GetLink2Info(). Fixed bug.
    Modified GetSerialPortPara(). The "break" should be taken out.
    Refer to PR:OASYS-PMS 0217.

Version: INM E.1.0.0
  01  07 July 2004, Yu Wei
    Modified GetSWCTable() to get nSWCBusyTimeout and
    nExcepTimeSyncRetry for time sync command exception process.
    Refer to PR:OASYS-PMS 0224.

  02  07 July 2004, Yu Wei
    Modified GetSWCPollingTimeInfo() to get nExcepRetry for polling
    command exception process.
    Refer to PR:OASYS-PMS 0225.

Version: INM E.1.0.1
  01  12 August 2004, Yu Wei
    Modified GetSWCNSvrTable(). Table 900 hasn't server table.
    Modified GetSWCTable() to read table 900 and time interval.
    Modified SearchSWCTable() to set no keep alive command as default.
    Refer to PR:OASYS-PMS 0233.
**************************************************************/
#include <stdio.h>
#include <process.h>
#include <sys/neutrino.h>
#include <iostream.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "sys_ass.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
#include "hwp_def.h"
#include "hwp_ext.h"
#include "cgf_def.h"
#include "cgf_ext.h"
//#include "flg_def.h"
//#include "flg_ext.h"



#include "Common.h"
#include "Init_Config.h"
#include "CMM_Log.h"
#include "init.h"


/*------------------------------------------------------------------------------
  Local variable declaration
------------------------------------------------------------------------------*/

/*
  Error log control
    0    No config file error log.
    1    Log config file to Event log.
    >=2  Printf all config file lines after CFG_ERR_LOG.
*/
static int g_nCFGFileErrorLogFlag = 0;

/*
Protocol type control
    0       Not defined.
    1       PR_CLK protocol.
    0xFFFF  Modbus.
    Other   Error.
*/
static unsigned short g_unProtocolType;

/*
  Server table map and SWC table.
  Server table map 64K. For server table defination check.
    0xFF  -- blank.
    0x00  -- allocated.
    0x01  -- allocated to RTU status table or SWC polling table.
*/
static unsigned char g_acServerTableMap[64*1024];

//SWC Key word list for searching SWC configuration.
static const char *g_acSWCKeyWordName[SWC_MAX_NUMBER] =
  {"SWC1", "SWC2","SWC3","SWC4","SWC5", "SWC6", "SWC7","SWC8","SWC9","SWC10",
   "SWC11", "SWC12","SWC13","SWC14","SWC15", "SWC16", "SWC17","SWC18","SWC19",
   "SWC20"};
/*------------------------------------------------------------------------------
  Global variable declaration
------------------------------------------------------------------------------*/
char g_acSWCDeviceName[SWC_MAX_NUMBER][64]={{0}, {0}};

/*------------------------------------------------------------------------------
  Local Prototype declaration
------------------------------------------------------------------------------*/
static int GetLink3Info(FILE *tFD);
static int GetLink6Info(FILE *tFD);
static bool IsValidIP(char *ip);

/*******************************************************************************
Purpose
  Read configuration file (config.txt) and decode the information,
  then store in the structure g_tRTUConfig.

Input
  acFileName -- File name.

Return
  E_ERR_Success                      routine executed successfully
  E_ERR_InvalidNullPointer           invalid null pointer
  E_ERR_InvalidParameter             invalid parameter, filename
  E_ERR_SYS_InvalidConfigFile        access config file fail
  E_ERR_INM_GetIntegerValueFail      fail to get integer value
  E_ERR_INM_GetRTUConfigFail         fail to get RTU config file
  E_ERR_INM_GetAllRTUTableFail       fail to get rtu table
  E_ERR_INM_GetServerDefFail         fail to get server definition
  E_ERR_INM_GetAllSWCParamterFail    fail to get SWC parameters


History
  Name          Date          Remark
  ----          ----          ------
 Bryan Chong   11-Feb-2010   Update to include NTP client parameters

*******************************************************************************/
E_ERR_T GetSystemConfig(char *acFileName)
{
  E_ERR_T rstatus = E_ERR_Success;
  int nReturn = CFGFILE_OK;  //Return value.
  FILE *tFDConfig;      //Configuration file descriptor
  int nTemp;
  char cLog[1000];
  if(acFileName == TYP_NULL)
    return E_ERR_InvalidNullPointer;

  if(*acFileName == TYP_NULL)
    return E_ERR_InvalidParameter;

  memset(g_acServerTableMap, 0xFF, sizeof(g_acServerTableMap));  //040319 Yu Wei

  if((tFDConfig = fopen(acFileName, "r")) == TYP_NULL)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetSystemConfig, cannot find %s\n",
           acFileName);
    #endif //  CFG_PRN_ERR
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetSystemConfig, open file %s error\n", acFileName);
    sprintf(cLog,"ERR  [INM] GetSystemConfig, open file %s error\n", acFileName);
    g_pEventLog->LogMessage(cLog);
    return E_ERR_SYS_InvalidConfigFile;
  }

  #if ((defined CFG_DEBUG_MSG) &&(CFG_DEBUG_INM))
  printf("[INM] GetSystemConfig, using config file: %s\n", acFileName);
  #endif // ((defined CFG_DEBUG_MSG) &&(CFG_DEBUG_INM))

  if(GetIntegerValue(tFDConfig,
      (CHAR *)"CFG_ERR_LOG", &g_nCFGFileErrorLogFlag) != OK)
    return E_ERR_INM_GetIntegerValueFail;

  if(g_nCFGFileErrorLogFlag != 0)
  {
    if ( g_nCFGFileErrorLogFlag > 1 )
      g_nCFGFileErrorLogFlag = 2;
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
    printf("[INM] GetSystemConfig, writing config.txt to event log\n");
    #endif // ((defined CFG_DEBUG_MSG) &&(CFG_DEBUG_INM))
  }

  WriteDebugLog((char *)
    "\n"
    "---------------------------\n"
    "Starting of config file log\n"
    "---------------------------\n");

  rstatus = GetCFGVersion(tFDConfig);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  if(GetIntegerValue(tFDConfig, (CHAR *)"SERIAL_LINK_RETRY", &nTemp) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetSystemConfig, invalid SERIAL_LINK_RETRY\n");
    sprintf(cLog,"ERR  [INM] GetSystemConfig, invalid SERIAL_LINK_RETRY\n");
    g_pEventLog->LogMessage(cLog);

    return E_ERR_INM_GetIntegerValueFail;
  }

  if(nTemp != 0){
    g_tDebugLogFlag.bLinkRetry = true;
    g_tDebugLogFlag.bLogFlag = true;
  }else{
    g_tDebugLogFlag.bLinkRetry = false;
  }

  if((rstatus = GetDebugInfo(tFDConfig)) != E_ERR_Success)
    return rstatus;

  //Get RTU configuration and debug log flag.
  if(GetRTUConfig(tFDConfig) == ERROR)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetSystemConfig, fail to get config\n");
    #endif // CFG_PRN_ERR

    nReturn = CFGFILE_ERROR_RTU;
    return E_ERR_INM_GetRTUConfigFail;
  }

  if(GetAllRTUTable(tFDConfig) == ERROR) //Get server table
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetSystemConfig, fail to get rtu table\n");
    #endif // CFG_PRN_ERR

    g_pEventLog->LogMessage(
      (CHAR *)"Field of RTU Table is Error in config file.");
    nReturn = CFGFILE_ERROR_RTU_TABLE;
    return E_ERR_INM_GetAllRTUTableFail;
  }

  if(GetServerDef(tFDConfig) == ERROR) //Get server definition
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetSystemConfig, fail to get server definition\n");
    #endif // CFG_PRN_ERR

    g_pEventLog->
      LogMessage((CHAR *)"Field of Server Definition is Error in config file.");
    nReturn = CFGFILE_ERROR_SERVER;
    return E_ERR_INM_GetServerDefFail;
  }

  if(GetAllSWCParameter(tFDConfig) == ERROR) //Get SWC interface parameter
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetSystemConfig, found invalid SWC parameters\n");
    #endif // CFG_PRN_ERR

    nReturn = CFGFILE_ERROR_SWC;
    return E_ERR_INM_GetAllSWCParamterFail;
  }

  // Get NTP parameter
  if(g_tRTUConfig.nSwcEnabled[SWC_MAX_NUMBER - 1] == E_TYPE_False)
  {
    rstatus = CGF_GetNTPParameter(tFDConfig);
    SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus;)
  }

  if( g_nCFGFileErrorLogFlag != 0 )  //040319 Yu Wei
  {
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
    printf("[INM] GetSystemConfig, Write config.txt to event log completed\n");
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  }

  WriteDebugLog((char *)
    "\n"
    "----------------------\n"
    "End of config file log\n"
    "----------------------\n");

  return E_ERR_Success;
} // GetSystemConfig


/*******************************************************************************
Purpose
  Read one line from configuration file (config.txt)

Input/Output
  acBuffer  -- Buffer to store a line data. (out)
  tFD      -- file descriptor. (in)

Return
  Nil.
********************************************************************************/
void ReadOneLine(char *acBuffer, FILE *tFD)
{
  if( fgets(acBuffer, INM_MAXSIZE_LINE, tFD) == TYP_NULL)    //040311 Yu Wei
  {
    acBuffer[0] = 0;
  }
  else
  {
    acBuffer[INM_MAXSIZE_LINE-1] = '\0';
  }
}

/*******************************************************************************
Purpose
  Write one line of config file to debug log or printf.
  Added in 18 March 2004, Yu Wei

Input/Output
  acBuffer -- Message buffer.(in)

Return
  Nil
********************************************************************************/
void WriteDebugLog(char *acBuffer)
{
  switch ( g_nCFGFileErrorLogFlag )
  {
  case 1:
    g_pEventLog->LogMessage(acBuffer);
    break;

  case 2:
    printf(acBuffer);
    break;

  default:
    break;
  }
}

/*******************************************************************************
Purpose
  Get parameter value string. The value is between '=' and ';'.
  The ' ' in the begining and end will be deleted. //040316 Yu Wei

Input/Output
  acBuffer -- Buffer to store orginal string and the value string.(in/out)

Return
  OK    -- Get a string correctly.
  ERROR  -- The '=' or ';' dose not exsit or value string length is 0.
********************************************************************************/
int GetValue(char *acBuffer)
{
  int nReturn = OK;    //return value.
  int nValueStringLength,nValueStringPosition;  //Value length and start position.
  char *pcEqual, *pcSemicolon;  //Point for '=' and ';'.

  pcEqual = strchr(acBuffer,'=');
  pcSemicolon = strchr(acBuffer,';');

  if((pcEqual == TYP_NULL) || (pcSemicolon == TYP_NULL))  //040309 Yu Wei
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetValue, invalid pointers\n"
           "  pcEqual = 0x%0x, pcSemicolon = 0x%0x\n",
           pcEqual, pcSemicolon);
    #endif // CFG_PRN_ERR
    nReturn = TYP_ERROR;
  }
  else
  {
    //Get start position of value string.
    nValueStringPosition = pcEqual - acBuffer;
    //Get length of value string.
    nValueStringLength = pcSemicolon - pcEqual;

    if(nValueStringLength <= 1)
    {
      #ifdef CFG_PRN_ERR
      printf("ERR  [INM] GetValue, invalid string length, %d\n",
             nValueStringLength);
      #endif // CFG_PRN_ERR
      nReturn = TYP_ERROR;
    }
    else
    {
      //Get value string.
      acBuffer[nValueStringPosition + nValueStringLength] = '\0';
      strcpy(acBuffer, &acBuffer[nValueStringPosition+1]);
      // Delete blank in the begining and end of the string.
      DeleteBlank(acBuffer);
    }
  }

  return nReturn;
}

/*******************************************************************************
Purpose
  Search a key word.

Input/Output
  tFD      -- file descriptor (in).
  acKeyWord  -- The buffer for key word (in).
  acBuffer  -- The buffer for one line string. (out)

Return
  true  -- Found key word.
  false  -- Not found.

History
  Name         Date          Remark
  ----         ----          ------
  Bryan Chong  08-Oct-2009   Change nMaxTry from 1024 to 2*1024
                             to resolve reading wrong section issue.
                             Suppose to SWC20 but stop at SWC18 due to nMaxTry
                             is not enough
********************************************************************************/
bool SearchKeyWord(FILE *tFD, char *acKeyWord, char *acBuffer)
{
  bool bReturn = false;  //Return value.

  int nMaxTry = 2*1024;    //Search key word in max lines.
  int nStrLen = strlen(acKeyWord);  //Key word length.

  if((acKeyWord == TYP_NULL) || (acBuffer == TYP_NULL))
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] SearchKeyWord, invalid null pointer, 0x%08x, 0x%08x\n",
           (UINT32)acKeyWord, (UINT32)acBuffer);
    #endif // CFG_PRN_ERR
    return false;
  }

  if(((INT32)tFD == TYP_NULL) || ((INT32)tFD == TYP_ERROR))
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] SearchKeyWord, invalid file descriptor, %d\n", tFD);
    #endif // CFG_PRN_ERR
    return false;
  }

  while(nMaxTry--)
  {
    ReadOneLine(acBuffer,tFD);  //Read one line.
    if(strncmp(acBuffer,acKeyWord,nStrLen) == 0)  //Check if the key word exsit
    {
      WriteDebugLog(acBuffer);  //040319 Yu Wei
      bReturn = true;
      break;
    }
  }

  #ifdef CFG_PRN_ERR
  if (bReturn == false)
  {
    printf("ERR  [INM] SearchKeyWord, nMaxTry %d, acKeyWord %s, acBuffer %s\n",
           nMaxTry, acKeyWord, acBuffer);
  }
  #endif // CFG_PRN_ERR
  return bReturn;
}

/**********************************************************************************
Purpose
  Search a key word and get string value.

Input/Output
  tFD      -- file descriptor (in).
  acKeyWord  -- The buffer for key word (in).
  acOutString  -- The buffer for string value. (out)

Return
  OK    -- Get a string correctly.
  ERROR  -- The value dose not exist or value string length is 0.
************************************************************************************/
E_ERR_T GetStringValue(FILE *tFD, char *acKeyWord, char *acOutString)
{
  char acReadBuffer[INM_MAXSIZE_LINE+1];  //Buffer for string with key word.
  char cLog[1000];
  if(SearchKeyWord(tFD,acKeyWord,acReadBuffer) == false)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetStringValue, fail at SearchKeyWord, %s\n", acKeyWord);
    sprintf(cLog, "ERR  [INM] GetStringValue, fail at SearchKeyWord, %s\n", acKeyWord);
    g_pEventLog->LogMessage(cLog);

    return E_ERR_INM_SearchKeyWordFail;
  }

  if(GetValue(acReadBuffer) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetStringValue, fail at GetValue, %s\n", acKeyWord);
    sprintf(cLog, "ERR  [INM] GetStringValue, fail at GetValue, %s\n", acKeyWord);
    g_pEventLog->LogMessage(cLog);

    return E_ERR_INM_GetValueFail;
  }

  strcpy(acOutString,acReadBuffer);

  return E_ERR_Success;
}// GetStringValue
/*******************************************************************************
Purpose
  Delete ' ' from a string. The ' ' is in begining and end of the string.
  When the string blank that contain more than one ' ', the return string will be
  only one ' '.
  Added in 16 March 2004, by Yu Wei.

Input/Output
  acOutString  -- The buffer for string. (in/out)

Return
  Nil.
********************************************************************************/
void DeleteBlank(char *acBuffer)
{
  char acTemp[INM_MAXSIZE_LINE];
  int nI = 1;
  int nStringLen;        //String length.

  strcpy ( acTemp, acBuffer );

  while( acTemp[0] == ' ') //Delete ' ' in the begining.
  {
    nStringLen = strlen ( acTemp );

    if(nStringLen > 1)
    {
      strcpy ( acTemp, &acBuffer[nI] );  //delete one ' '.
      nI++;
    }
    else
    {
      break;
    }

    if(nI >= INM_MAXSIZE_LINE)  //out of string range.
    {
      break;
    }
  }

  nStringLen = strlen ( acTemp );
  while( acTemp[nStringLen-1] == ' ' )  //End with ' '.
  {
    acTemp[nStringLen-1] = '\0';    //Delete one ' ' in the end.
    nStringLen --;      //Get new string length.
    if ( nStringLen <= 1 )
    {
      break;
    }
  }
  strcpy ( acBuffer, acTemp );  //Get new string thare are no ' ' in the begining.
}

/*******************************************************************************
Purpose
  Check if the string is a valid integer string.
  The minus vale is unaccetped.
  Convert string to an integer value.
  re-design in 15 March 2004, by Yu Wei.

Input/Output
  acBuffer  -- The buffer for string  (in).
  pnValue    -- Integer value is converted from string. (out)

Return
  OK    -- Get the integer value.
  ERROR  -- The string is an invalid int string.
*******************************************************************************/
int GetDigit(char *acBuffer, int *pnValue)
{
  char acTemp[16] = {0};
  int nReturn = OK;  //Check result.
  int nStringLen;    //String length
  int nI = 1;

  *pnValue = 0;    //set initial value.

  if(strlen(acBuffer) > sizeof(acTemp))
    return TYP_ERROR;

  strcpy(acTemp,acBuffer);  //Copy string.

  //Kick out beginning character that is '0' execpt '0x' or '0X'.
  while( 1 )
  {
    nStringLen = strlen( acTemp );  //Get string length.

    //detection of "0x" for hexadecimal format
    if((( acTemp[0] == '0' ) &&    //start with '0'.
      ((acTemp[1] != 'x') && (acTemp[1] != 'X')))&&  //execpt '0x' or '0X'.
       ( nStringLen > 1))
    {
      strcpy(acTemp,&acBuffer[nI]);
      nI ++ ;
    }
    else      //start with other character.
    {
      if ( nStringLen == 0 )
      {
        nReturn = ERROR;
      }
      break;
    }

    if( nI >= INM_MAXSIZE_LINE )  //End of string.
    {
      nReturn = ERROR;
      break;
    }
  }

  if( nReturn == OK)
  {
    if((strncmp(acTemp,"0x",2) == 0) || (strncmp(acTemp,"0X",2) == 0))
    {
      if ( GetHexDigit(acTemp, pnValue) == ERROR)
      {
        nReturn = ERROR;
      }
    }
    else
    {
      if ( GetDecimalDigit(acTemp, pnValue) == ERROR)
      {
        nReturn = ERROR;
      }
    }
  }
  if( (unsigned int)*pnValue >= 0x80000000)  //The minus value is not accepted.
    nReturn = ERROR;

  return nReturn;
}

/*******************************************************************************
Purpose
  Convert decimal string to an integer value.
  Added in 16 March 2004, by Yu Wei.

Input/Output
  acBuffer  -- The buffer for string  (in).
  pnValue    -- Integer value is converted from string. (out)

Return
  OK    -- Get the integer value.
  ERROR  -- The string is an invalid int string.
*******************************************************************************/
int GetDecimalDigit(char *acBuffer, int *pnValue)
{
  int nReturn = OK;  //Check result.
  int nStringLen;    //String length
  unsigned int nTemp1,nTemp2;
  int nI;

  nStringLen = strlen( acBuffer );

  if( (nStringLen > 10) )    //Greater than 0xFFFFFFFF. over int.
                //Max int value = 4294967295 is 10 digits.
  {
    nReturn = ERROR;
  }
  else
  {
    for(nI=0; nI<nStringLen; nI++)  //Check the character is '0' ~ '9'
    {
      if ( isdigit(acBuffer[nI]) == 0)
      {
        nReturn = ERROR;
        break;
      }
    }
  }

  if( nReturn == OK)
  {
    if(nStringLen == 10)  //Check if value > 4294967295
    {
      nTemp1 = strtoul(acBuffer,NULL,10);

      if ((( nTemp1 % 10) == 5) && ( acBuffer [9] != '5')) //result error.
      {                //if value over, return 4294967295.
        nReturn = ERROR;
      }
      else
      {
        acBuffer [9]--;    // set acBuffer[9] = '4'.
        nTemp2 = strtoul(acBuffer,NULL,10);
        if( nTemp1 == nTemp2)  //The value is still over, return is same.
        {
          nReturn = ERROR;
        }
        else  //Get correct value.
        {
          *pnValue = nTemp1;
        }
      }
    }
    else  // less than 9 digits, convert directly.
    {
      *pnValue = strtoul(acBuffer,NULL,10);
    }
  }
  return nReturn;
}
/*******************************************************************************
Purpose
  Convert hexadecimal string to an integer value.
  Added in 16 March 2004, by Yu Wei.

Input/Output
  acBuffer  -- The buffer for string  (in).
  pnValue    -- Integer value is converted from string. (out)

Return
  OK    -- Get the integer value.
  ERROR  -- The string is an invalid int string.
*******************************************************************************/
int GetHexDigit(char *acBuffer, int *pnValue)
{
  int nStringLen;    //String length.
  int nReturn = OK;  //return result.
  int nI;

  nStringLen = strlen( acBuffer );

  if( (nStringLen <= 2) ||  //Only with '0x'
    (nStringLen > 10) )    //Greater than 0xFFFFFFFF. over int.
  {
    nReturn = ERROR;
  }
  else
  {
    for(nI=2; nI<nStringLen; nI++)  //Check the character is '0' ~ 'F'
    {
      if ( isxdigit(acBuffer[nI]) == 0)
      {
        nReturn = ERROR;
        break;
      }
    }
  }

  if( nReturn == OK)
    *pnValue = strtoul(acBuffer,NULL,16);

  return nReturn;
}

/*******************************************************************************
Purpose
  Search a key word and get integer value.

Input/Output
  tFD      -- file descriptor (in).
  acKeyWord  -- The buffer for key word (in).
  pnOutValue  -- The point for integer value. (out)

Return
  OK    -- Get an integer value correctly.
  ERROR  -- Cannot get an integer value correctly.
*******************************************************************************/
int GetIntegerValue(FILE *tFD, char *acKeyWord, int *pnOutValue)
{
  int nReturn = ERROR;  //Return value.

  char acReadBuffer[INM_MAXSIZE_LINE+1] = {0};  //Buffer for string with key word.

  if(SearchKeyWord(tFD,acKeyWord,acReadBuffer) == true)
  {
    if(GetValue(acReadBuffer) == OK)      //040315 Yu Wei
    {
      if( GetDigit(acReadBuffer,pnOutValue) == OK)  //040315 Yu Wei
        nReturn = OK;
    }
  }

  return nReturn;
}

/*******************************************************************************
Purpose
  Search a key word and get short integer value.

Input/Output
  tFD      -- file descriptor (in).
  acKeyWord  -- The buffer for key word (in).
  pnOutValue  -- The point for integer value. (out)

Return
  OK    -- Get a short integer value correctly.
  ERROR  -- Cannot get the value correctly.
*******************************************************************************/
int GetuShortValue(FILE *tFD, char *acKeyWord, unsigned short *pnOutValue)
{
  int nReturn = ERROR;  //Return value.
  int nTemp=0;    //040315 Yu Wei

  char acReadBuffer[INM_MAXSIZE_LINE+1] = {0};  //Buffer for string with key word.

  if(acKeyWord == TYP_NULL)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetuShortValue, invalid null pointer, 0x%08x, 0x%08x\n",
           acKeyWord, pnOutValue);
    #endif // CFG_PRN_ERR
    return ERROR;
  }

  if(((INT32)tFD == TYP_NULL) || ((INT32)tFD == TYP_ERROR))
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetuShortValue, invalid tFD, %d\n", tFD);
    #endif // CFG_PRN_ERR
    return ERROR;
  }

  if(SearchKeyWord(tFD,acKeyWord,acReadBuffer) == true)
  {
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
    printf("[INM] GetuShortValue, SearchKeyWord %s OK\n", acKeyWord);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
    if(GetValue(acReadBuffer) == OK)      //040315 Yu Wei
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
      printf("[INM] GetuShortValue, GetValue for %s OK\n", acReadBuffer);
      #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
      if( GetDigit(acReadBuffer,&nTemp) == OK)  //040315 Yu Wei
      {
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
        printf("[INM] GetuShortValue, GetDigit convert string %s to %d OK\n",
               acReadBuffer, nTemp);
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
        if( nTemp <= 0xFFFF)    //unsigned short 040315 Yu Wei
        {
          *pnOutValue = (unsigned short)nTemp;
          nReturn = OK;
        }
      }
    }
  }
  if(nReturn == ERROR)
  {
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
    printf("[INM] GetuShortValue, acKeyWord %s\n", acKeyWord);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
  }

  return nReturn;
}

/*******************************************************************************
Purpose
  Search a key word and get char value (in HEX).

Input/Output
  tFD      -- file descriptor (in).
  acKeyWord  -- The buffer for key word (in).
  pnOutValue  -- The point for char value. (out)

Return
  OK    -- Get a char value correctly.
  ERROR  -- Cannot get the value correctly.
*******************************************************************************/
int GetCharValue(FILE *tFD, char *acKeyWord, char *pnOutValue)
{
  int nReturn = ERROR;  //Return value.
  int nTemp = 0;        //040315 Yu Wei

  char acReadBuffer[INM_MAXSIZE_LINE+1];  //Buffer for string with key word.

  if(SearchKeyWord(tFD,acKeyWord,acReadBuffer) == true)
  {
    if(GetValue(acReadBuffer) == OK)      //040315 Yu Wei
    {
      if( GetDigit(acReadBuffer,&nTemp) == OK)  //040315 Yu Wei
      {
        if( nTemp <= 0xFF)    //char 040315 Yu Wei
        {
          *pnOutValue = (char)nTemp;
          nReturn = OK;
        }
      }
    }
  }

  return nReturn;
}
/*******************************************************************************
Purpose
  Search a key word, get IP address and check if the IP address is valid.
  Added in 17 March 2004, Yu Wei

Input/Output
  tFD      -- file descriptor (in).
  acKeyWord  -- The buffer for key word (in).
  pnOutValue  -- The Buffer for IP address. (out)

Return
  OK    -- Get a char value correctly.
  ERROR  -- Cannot get the value correctly.
*******************************************************************************/
int GetIPAddrValue(FILE *tFD, char *acKeyWord, char *pnOutValue)
{
  int nReturn = ERROR;  //Return value.
  char acTemp[INM_MAXSIZE_LINE+1];
  char acOneString[10];
  int nTemp;

  if( GetStringValue (tFD, acKeyWord, acTemp) == E_ERR_Success )
  {
    strcpy ( pnOutValue, acTemp );

    //First field.
    if(SearchMultiValue(acTemp,acOneString,'.') == true)  //More than one digit.
                                //Otherwise return error.
    {
      if( GetDigit (acOneString, &nTemp) == OK)  //Get first field value.
      {
        if ( nTemp <= 0xFF)  //Check if the value is valid.
        {
          //Second field
          if(SearchMultiValue(acTemp,acOneString,'.') == true)  //More than two digit.
                                      //Otherwise return error.
          {
            if( GetDigit (acOneString, &nTemp) == OK)  //Get second field value.
            {
              if ( nTemp <= 0xFF)  //Check if the value is valid.
              {
                //Third field
                if(SearchMultiValue(acTemp,acOneString,'.') == true)  //More than three digit.
                  //Otherwise return error.
                {
                  if( GetDigit (acOneString, &nTemp) == OK)  //Get third field value.
                  {
                    if ( nTemp <= 0xFF)  //Check if the value is valid.
                    {
                      if(SearchMultiValue(acTemp,acOneString,'.') == false)  //Four digit only
                      {
                        if( GetDigit (acOneString, &nTemp) == OK)  //Get last field value.
                        {
                          if ( nTemp <= 0xFF)  //Check if the value is valid.
                          {
                            nReturn = OK;
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  if(nReturn == ERROR)  //If IP address error, clear buffer to fixed bug:
    pnOutValue[0] = 0;  //RTU use wrong IP to set LAN IP. //040421 Yu Wei

  return nReturn;
}

/******************************************************************************
Purpose
  Get debug log info and version number
  Added in 16 March 2004, Yu Wei

Input
  tFD -- file descriptor.(in)

Return
  OK    -- Successful.
  ERROR  -- one info error.

History

  Name            Date          Remark
  ----            ----          ------
 Bryan Chong    04-Mar-2010   Move GetCFGVersion to GetSystemConfig routine

*******************************************************************************/
E_ERR_T GetDebugInfo(FILE *tFD)
{
  int nTemp;

  //Get all debug log flag.
  g_tDebugLogFlag.bLogFlag = false;

  //Get debug log flag: server command
  if(GetIntegerValue(tFD, (CHAR *)"SERVER_COMMAND", &nTemp) == ERROR)
    return E_ERR_INM_GetIntegerValueFail;

  if(nTemp != 0){
    g_tDebugLogFlag.bServerCommand = true;
    g_tDebugLogFlag.bLogFlag = true;
  } else {
    g_tDebugLogFlag.bServerCommand = false;
  }

  //Get debug log flag: SWC command
  if(GetIntegerValue(tFD, (CHAR *)"SWC_COMMAND", &nTemp) == ERROR)
    return E_ERR_INM_GetIntegerValueFail;

  if(nTemp != 0){
    g_tDebugLogFlag.bSWCCommand = true;
    g_tDebugLogFlag.bLogFlag = true;
  } else {
    g_tDebugLogFlag.bSWCCommand = false;
  }

  return E_ERR_Success;
} // GetDebugInfo

/*******************************************************************************
Purpose
  Get configuration file version number. The version number must be standard
  format:
    x.x.x;  x -- 0~9 or 0x0~0xF.

  Re-design in 16 March 2004, Yu Wei.

Input
  tFD -- file descriptor.(in)

Return
  OK    -- Get version number.
  ERROR  -- Cannot get version number.

History
  Name         Date          Remark
  ----         ----          ------
  Bryan Chong  09-Oct-2009   Update to accommodate error handling mechnism.
                             To simplify the version acquiring process

*******************************************************************************/
E_ERR_T GetCFGVersion(FILE *tFD)
{
  E_ERR_T rstatus = E_ERR_Success;
  char acTemp[200];
  char acOneString[10];
  int nTemp;

  rstatus = GetStringValue(tFD, (CHAR *)"VER_RTU_CFG", acTemp);
  SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

  if(SearchMultiValue(acTemp, acOneString, '.') == false)
    return E_ERR_INM_SearchMultiValueFail;

  //more than one digit valid
  if( GetDigit( acOneString, &nTemp ) != OK)
    return E_ERR_INM_GetDigitFail;

  if(nTemp <= 0x0F)
    g_tRTUConfig.unVersion = ((UINT16)nTemp) << 8;


  if(SearchMultiValue(acTemp, acOneString, '.') == false)
    return E_ERR_INM_SearchMultiValueFail;

  //check validity of the first group of digits before first dot
  if( GetDigit(acOneString, &nTemp) != OK)
    return E_ERR_INM_GetDigitFail;

  if(nTemp <= 0x0F)
    g_tRTUConfig.unVersion += ((UINT16)nTemp) << 4;


  if (SearchMultiValue(acTemp, acOneString, '.') == true )
    return E_ERR_INM_InvalidVersionNumber;

  //check validity of the second group of digits before second dot
  if( GetDigit(acOneString, &nTemp) != OK)
    return E_ERR_INM_GetDigitFail;

  if(nTemp <= 0x0F)
    g_tRTUConfig.unVersion += ((UINT16)nTemp);

  sprintf(acTemp, "Configuration File Version: %d.%d.%d\n",
          (g_tRTUConfig.unVersion>>8),
          ((g_tRTUConfig.unVersion&0x00F0)>>4),
          (g_tRTUConfig.unVersion&0x000F));

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  printf("[INM] GetCFGVersion, config file, %s, %d.%d.%d\n",
          pSYS_CB->config_filename, (g_tRTUConfig.unVersion>>8),
          ((g_tRTUConfig.unVersion&0x00F0)>>4),
          (g_tRTUConfig.unVersion&0x000F));
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))

  g_pEventLog->LogMessage(acTemp);

  return E_ERR_Success;
}

/*******************************************************************************
Purpose
  Get other RTU IP Address

Input/Output
  acLocalIP -- Local RTU IP Adderss.(in)
  acOtherIP -- Output other RTU IP Address. (out)
  nRTUID    -- RTU ID. 1 or 2. (in)

Return
  OK    -- Get correct IP address.
  ERROR  -- IP Address error or RTU ID error.
*******************************************************************************/
int GetOtherRTUIPAddress(char *acLocalIP, char *acOtherIP, int nRTUID)
{
  int nPoint, nTemp;
  char acTemp[10];
  char *pcPoint;
  char cLog[1000];

  //find the last occurance of '.'
  pcPoint = strrchr(acLocalIP,'.');

  if(pcPoint != NULL)
    nPoint = pcPoint - acLocalIP + 1;
  else{
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetOtherRTUIPAddress, invalid IP RTU_LAN1_IP %s\n",
//      acLocalIP);
    sprintf(cLog, "ERR  [INM] GetOtherRTUIPAddress, invalid IP RTU_LAN1_IP %s\n",acLocalIP);
    g_pEventLog->LogMessage(cLog);

    return ERROR;
  }

  //Get latest field.
  if ( GetDigit(&acLocalIP[nPoint], &nTemp) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetOtherRTUIPAddress, non digit in IP RTU_LAN1_IP %s\n",
//      acLocalIP);
    sprintf(cLog, "ERR  [INM] GetOtherRTUIPAddress, non digit in IP RTU_LAN1_IP %s\n",acLocalIP);
    g_pEventLog->LogMessage(cLog);

    return ERROR;
  }

  //++ or --
  if(nRTUID ==1)
    nTemp++;
  else if(nRTUID ==2)
    nTemp--;
  else
    return ERROR;

  if((nTemp<0)||(nTemp>255))
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetOtherRTUIPAddress, out of range IP RTU_LAN1_IP %s\n",
//      acLocalIP);
    sprintf(cLog, "ERR  [INM] GetOtherRTUIPAddress, out of range IP RTU_LAN1_IP %s\n",acLocalIP);
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  //Get other RTU IP.
  sprintf(acTemp,"%d",nTemp);
  strncpy(acOtherIP,acLocalIP,nPoint);
  strcat(acOtherIP,acTemp);

  return OK;
}

/*******************************************************************************
Purpose
  Get local time zone offset in seconds.

Input
  tFD        -- file descriptor.(in)
  nLocalTimeZone  -- Local time zone in seconds. (out)

Return
  offset in second  -- positive or negative offset in seconds
                       Eg. +8 hour will return 8 * 60 * 60 = 28800 seconds
  ERROR (-1)        -- Parameter error or cannot find it.
*******************************************************************************/
int GetLocalTimeZone(FILE *tFD, int *nLocalTimeZone)
{
  char acTempString[128];
  char acTemp1[128];
  char *pcPlus, *pcMinus;  //Point for '+' and '-'.
  int nValueStringPosition;  //Value start position.
  int nSign, nTemp, nTemp1;
  char cLog[1000];
  //Search LOCAL_TIME_ZONE key word.
  if(GetStringValue(tFD, (CHAR *)"LOCAL_TIME_ZONE", acTempString) !=
     E_ERR_Success)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetLocalTimeZone, invalid LOCAL_TIME_ZONE\n");
    sprintf(cLog, "ERR  [INM] GetLocalTimeZone, invalid LOCAL_TIME_ZONE\n");
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  if(strncmp(acTempString,"GMT",3) == 0)  //The string is started with "GMT".
  {
    if (strlen (acTempString) == 3)  //GMT means GMT+0.
    {
      *nLocalTimeZone = 0;
      return OK;
    }

    pcPlus = strchr(acTempString,'+');    //Search '+'.
    pcMinus = strchr(acTempString,'-');    //Search '-'.

    if (( pcPlus != NULL) && ( pcMinus == TYP_NULL))  //Positive value.
    {
      nSign = 1;
      nValueStringPosition = pcPlus - acTempString;
    }
    else if (( pcPlus == TYP_NULL) && ( pcMinus != NULL))  //Minus value.
    {
      nSign = -1;
      nValueStringPosition = pcMinus - acTempString;
    }
    else
    {
      return ERROR;
    }

    strcpy(acTempString, &acTempString[nValueStringPosition+1]);

    if(SearchMultiValue(acTempString,acTemp1,':') == false)
    {  //No minutes. Get hour only.
      DeleteBlank(acTemp1);
      if ( GetDigit(acTemp1, &nTemp) == ERROR)
        return ERROR;

      if ( nTemp > 13 )  //Max value is +13:00
        return ERROR;
      *nLocalTimeZone = nSign * nTemp * 3600; //Convert to seconds.
    }
    else
    {
      DeleteBlank(acTemp1);
      if ( GetDigit(acTemp1, &nTemp) == ERROR)
        return ERROR;

      if ( nTemp > 13 )  //Max value is +13:00
        return ERROR;

      if(SearchMultiValue(acTempString,acTemp1,':') == false)
      {  //Get minutes.
        DeleteBlank(acTemp1);
        if ( GetDigit(acTemp1, &nTemp1) == ERROR)
          return ERROR;

        if ( nTemp1 > 59 )  //Max value is 59
          return ERROR;

        //Convert to seconds.
        *nLocalTimeZone = nSign * ((nTemp *60) + nTemp1) * 60;
      }
      else  //More than two fields. The second is not accepted.
      {
        return ERROR;
      }
    }
  }
  else
  {
    return ERROR;
  }
  return OK;
}
/*******************************************************************************
Purpose
  Get RTU parameters.
  Debug log flag is done by GetDebugInfo().
  Able to detect the number of serial COM card installed and the exact slot.
  Serial COM driver will be initialized based on the detection.

Input
  tFD  [in] file descriptor.

Return
  OK      Get all parameters.
  ERROR   At least one parameter error or cannot find it.

History
   Name          Date             Remark
   ----          ----             ------
 Bryan Chong   12-Jan-2010     Add PCI Serial controller detection and
                               initialization. [PR21]
 Bryan Chong   04-Feb-2010     Use spawnv to initialize devc-ser8250 and
                               able to kill the driver directly before exit
                               application.
 Bryan Chong   26-Jul-2010     Enable checking on number of PCI Serial
                               controller cards are installed.
 Bryan Chong   20-Sep-2010     Added LAN link 6 parameters [PR74]
 Bryan Chong   29-Aug-2011     Add error to event log for RTU_IDENTIFICATION
 Bryan Chong   31-Aug-2011     Update config error to debug log
 Bryan Chong   17-Nov-2011     Add LAN 4 port to resolve loopback issue.
                               [C955,PR111]
******************************************************************************/
int GetRTUConfig(FILE *tFD)
{
  char acTemp[200];
  char acTempString[128];
  char cLog[1000];
  //Get local time zone, LOCAL_TIME_ZONE.
  if ( GetLocalTimeZone(tFD, &g_tRTUConfig.nLocalTimeZone) == ERROR)
    return ERROR;

  //Get RTU identification.
  //Socket ID is changed to unsigned short  //040309 Yu Wei
  if(GetuShortValue(tFD, (CHAR *)"RTU_IDENTIFICATION",
      &g_tRTUConfig.unRTUIdentification) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetRTUConfig, invalid RTU_IDENTIFICATION\n");
    sprintf(cLog,"ERR  [INM] GetRTUConfig, invalid RTU_IDENTIFICATION\n");
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  //Get RTU ID (RTU1 or RTU2).
  if((g_tRTUConfig.unRTUIdentification % 2) == 1)
  {
    //Odd number is RTU1.
    g_tRTUConfig.nRTUID = 1;
  }else{
    //Even number is RTU2.
    g_tRTUConfig.nRTUID = 2;
  }

  if(GetStringValue(tFD, (CHAR *)"RTU_LOCATION", acTempString) != E_ERR_Success)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetRTUConfig, invalid RTU_LOCATION\n");
    sprintf(cLog,"ERR  [INM] GetRTUConfig, invalid RTU_LOCATION\n");
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }
  else
  {
    if (strlen (acTempString ) > INM_RTU_LOCATION_SZ )  //longer than 32 bits
    {
      //Local is two word, otherwise the config file error. //040316 Yu Wei
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
      printf("[INM] GetRTUConfig, RTU_LOCATION characters, %s,exceed limit "
             "%d\n",
             acTempString, INM_RTU_LOCATION_SZ);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetRTUConfig, RTU_LOCATION characters, %s, exceed limit "
//          "%d\n",
//          acTempString, INM_RTU_LOCATION_SZ);
      sprintf(cLog,"ERR  [INM] GetRTUConfig, RTU_LOCATION characters, %s, exceed limit "
              "%d\n",
              acTempString, INM_RTU_LOCATION_SZ);
      g_pEventLog->LogMessage(cLog);
      return ERROR;
    }
    else
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
      printf("[INM] GetRTUConfig, RTU location: %s\n" , acTempString);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
      strcpy( g_tRTUConfig.acRTULocalion, acTempString );
    }
  }

  //Write RTU identification and location to event log.
  sprintf(acTemp,"Identication (RTU%1d): %4d/%4s\n"
          "-----------------------------------------------------\n",
    g_tRTUConfig.nRTUID, g_tRTUConfig.unRTUIdentification,
    g_tRTUConfig.acRTULocalion);

  g_pEventLog->LogMessage(acTemp);

  // Get maximum CPU temperature
  if(GetuShortValue(tFD, (CHAR *)"CPU_MAX_TEMPERATURE",
                     &g_tRTUConfig.ncpuMaxTemperature) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetRTUConfig, invalid CPU_MAX_TEMPERATURE\n");
    sprintf(cLog,"ERR  [INM] GetRTUConfig, invalid CPU_MAX_TEMPERATURE\n");
    g_pEventLog->LogMessage(cLog);

    return ERROR;
  }

  // Get minimum free memory space
  if(GetuShortValue(tFD, (CHAR *)"CF_FREE_SPACE",
                     &g_tRTUConfig.nminFreeMemorySpace) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetRTUConfig, invalid CF_FREE_SPACE\n");

    sprintf(cLog,"ERR  [INM] GetRTUConfig, invalid CF_FREE_SPACE\n");
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  if(GetuShortValue(tFD, (CHAR *)"RTU_MAINT_TERMINAL_SID",
                     &g_tRTUConfig.nMaintTerminalSocketID) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetRTUConfig, invalid RTU_MAINT_TERMINAL_SID\n");
    sprintf(cLog,"ERR  [INM] GetRTUConfig, invalid RTU_MAINT_TERMINAL_SID\n");
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }
  if(GetIntegerValue(tFD, (CHAR *)"TIMEOUT_RTU_MAINT_TERMINAL",
                     &g_tRTUConfig.nMaintTerminalTimeout) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetRTUConfig, invalid TIMEOUT_RTU_MAINT_TERMINAL\n");
    sprintf(cLog,"ERR  [INM] GetRTUConfig, invalid TIMEOUT_RTU_MAINT_TERMINAL\n");
    g_pEventLog->LogMessage(cLog);

    return ERROR;
  }
  else
  {
    //More than this value, the milisecond will be out of range.
    if (g_tRTUConfig.nMaintTerminalTimeout > INM_RTU_MAINT_TERMINAL_MAX_TIMEOUT)
    {
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetRTUConfig, TIMEOUT_RTU_MAINT_TERMINAL exceed limit "
//          "%d\n", INM_RTU_MAINT_TERMINAL_MAX_TIMEOUT);
      sprintf(cLog,"ERR  [INM] GetRTUConfig, TIMEOUT_RTU_MAINT_TERMINAL exceed limit "
              "%d\n", INM_RTU_MAINT_TERMINAL_MAX_TIMEOUT);
      g_pEventLog->LogMessage(cLog);

      return ERROR;
    }
    else
    {
      //This timeout is based on seconds.
      g_tRTUConfig.nMaintTerminalTimeout *= 1000;
    }
  }

  if(GetIntegerValue(tFD, (CHAR *)"TIMEOUT_INIT_LAN_CHK",
                     &g_tRTUConfig.nInitLANCheckTimeout) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetRTUConfig, invalid TIMEOUT_INIT_LAN_CHK\n");
    sprintf(cLog,"ERR  [INM] GetRTUConfig, invalid TIMEOUT_INIT_LAN_CHK\n");
    g_pEventLog->LogMessage(cLog);

    return ERROR;
  }

  // Get number of serial communication card installed
  if(GetIntegerValue(tFD, (CHAR *)"NB_SERIAL_COM",
                     &g_tRTUConfig.nNumberOfSerialCOMCard) ==  ERROR)
  {
    #ifdef CFG_PRN_ERR
    printf("[INM] GetRTUConfig, invalid value for NB_SERIAL_COM\n");
    #endif // CFG_PRN_ERR
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetRTUConfig, invalid NB_SERIAL_COM\n");
    sprintf(cLog,"ERR  [INM] GetRTUConfig, invalid NB_SERIAL_COM\n");
    g_pEventLog->LogMessage(cLog);

    return ERROR;
  }

  // 20100726 BC (Rqd ZSL)
  // Check for installed serial communication card
  if ((g_tRTUConfig.nNumberOfSerialCOMCard > CFG_HWP_TOTAL_PCI_SLOT) ||
      (g_tRTUConfig.nNumberOfSerialCOMCard > pHWP_CB->ucnumOfPciCardFound))
  {
    #ifdef CFG_PRN_ERR
    printf("[INM] GetRTUConfig, NB_SERIAL_COM definition, %d, does not matach"
           " with slot number or exceed card found number(%d)\n",
           g_tRTUConfig.nNumberOfSerialCOMCard,
           pHWP_CB->ucnumOfPciCardFound);
    #endif // CFG_PRN_ERR
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetRTUConfig, g_tRTUConfig.nNumberOfSerialCOMCard "
//          "exceed\n"
//          "  CFG_HWP_TOTAL_PCI_SLOT, %d, or pHWP_CB->ucnumOfPciCardFound, "
//          "%d\n", CFG_HWP_TOTAL_PCI_SLOT, pHWP_CB->ucnumOfPciCardFound);
    sprintf(cLog,"ERR  [INM] GetRTUConfig, g_tRTUConfig.nNumberOfSerialCOMCard "
            "exceed\n"
            "  CFG_HWP_TOTAL_PCI_SLOT, %d, or pHWP_CB->ucnumOfPciCardFound, "
            "%d\n", CFG_HWP_TOTAL_PCI_SLOT, pHWP_CB->ucnumOfPciCardFound);
    g_pEventLog->LogMessage(cLog);

    return ERROR;
  }

  //VDU Name and Password
  if(GetStringValue(tFD, (CHAR *)"USER_NAME_LIST", acTempString) !=
     E_ERR_Success)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetRTUConfig, USER_NAME_LIST error");
    #endif // CFG_PRN_ERR
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetRTUConfig, invalid USER_NAME_LIST\n");

    sprintf(cLog, "ERR  [INM] GetRTUConfig, invalid USER_NAME_LIST\n");
    g_pEventLog->LogMessage(cLog);

    return ERROR;
  }
  if(GetVDUName(acTempString) == ERROR)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetRTUConfig, GetVDUName error\n");
    #endif // CFG_PRN_ERR
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetRTUConfig, USER_NAME_LIST invalid number of names\n"
//          "  valid range is 1 - %d\n", VDU_MAX_USER);
    sprintf(cLog,"ERR  [INM] GetRTUConfig, USER_NAME_LIST invalid number of names\n"
            "  valid range is 1 - %d\n", VDU_MAX_USER);
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  if(GetStringValue(tFD, (CHAR *)"PASSWORD_LIST", acTempString) !=
     E_ERR_Success)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetRTUConfig, PASSWORD_LIST error\n");
    #endif // CFG_PRN_ERR
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetRTUConfig, invalid PASSWORD_LIST\n");
    sprintf(cLog,"ERR  [INM] GetRTUConfig, invalid PASSWORD_LIST\n");
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  if(GetVDUPassword(acTempString) == ERROR)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetRTUConfig, GetVDUPassword error\n");
    #endif // CFG_PRN_ERR
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetRTUConfig, invalid PASSWORD_LIST. Exceed max size %d or "
//      "max number of passwords %d\n",
//      INM_PASSWORD_MAX_SZ, VDU_MAX_USER);
    sprintf(cLog, "ERR  [INM] GetRTUConfig, invalid PASSWORD_LIST. Exceed max size %d or "
    	      "max number of passwords %d\n",
    	      INM_PASSWORD_MAX_SZ, VDU_MAX_USER);
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  if ( GetLink1Info(tFD) == ERROR)  //040322 Yu Wei
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetRTUConfig, GetLink1Info error\n");
    #endif // CFG_PRN_ERR
    return ERROR;
  }

  if ( GetLink2Info(tFD) == ERROR)  //040322 Yu Wei
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetRTUConfig, GetLink2Info error\n");
    #endif // CFG_PRN_ERR
    return ERROR;
  }

  if ( GetLink3Info(tFD) == ERROR)  //040322 Yu Wei
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetRTUConfig, GetLink3Info error\n");
    #endif // CFG_PRN_ERR
    return ERROR;
  }



  if ( GetLink6Info(tFD) == ERROR)  // 20100920 BC (Rqd by ZSL)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetRTUConfig, GetLink6Info error");
    #endif // CFG_PRN_ERR
    return ERROR;
  }

  return OK;
} // GetRTUConfig
#ifdef CFG_PRIMARY_LAN_PORT_1
/*******************************************************************************
Purpose
  Get RTU Link 1 info.
  Added to decreas the complexity in 22 March 2004, Yu Wei

Input
  tFD -- file descriptor.(in)

Return
  OK    -- Get all parameter.
  ERROR  -- At least one parameter error or cannot find it.

History

  Name             Date       Remark
  ----             ----       ------
  Bryan Chong    14-Oct-09  Update to check RTU_LANx_PRO must carry parameter
                              "MODBUS,0x01,0x04"
  Bryan Chong    15-Oct-09  Update g_tRTUConfig.tLAN_Para[x].cLAN_SlaveAddress
                              to g_tRTUConfig.tLAN_Para[x].tPrtc.addr due to
                              parameter change in latest Init_Config.cpp
  Bryan Chong    21-Oct-09  Fix invalid IP by clearing memory from acTempString
                              using memset.
  Zhang Shi Lun  22-Oct-09  Add memset(&g_tRTUConfig.acOtherRTUIPAddress[0], 0,
                              sizeof(g_tRTUConfig.acOtherRTUIPAddress[0]));
                              to clear other IP address buffer writing.
  Bryan Chong    02-Aug-11  Set max value for LAN1_N_POLLING, LAN1_TIME_OUT, and
                              LAN1_N_RETRY.
  Bryan Chong    01-Sep-11  Add error message to event log

********************************************************************************/
int GetLink1Info(FILE *tFD)
{
  char acTempString[128];

  //LAN1 link parameter
  if(GetIPAddrValue(tFD, (CHAR *)"RTU_LAN1_IP",
                    g_tRTUConfig.tLANPara[0].acLAN_IP) == ERROR)
  {
    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
          "ERR  [INM] GetLink1Info, invalid RTU_LAN1_IP\n");
    return ERROR;
  }

  if(GetIPAddrValue(tFD, (CHAR *)"RTU_LAN1_NETMASK", g_tRTUConfig.tLANPara[0].acLAN_Netmask) == ERROR)
  {
    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
          "ERR  [INM] GetLink1Info, invalid RTU_LAN1_NETMASK\n");
    return ERROR;
  }

  //Cannot get gateway IP, set it to '\0'.
  if (GetIPAddrValue(tFD, (CHAR *)"RTU_LAN1_GWAY",
                     g_tRTUConfig.tLANPara[0].acLAN_Gateway)== ERROR)
    g_tRTUConfig.tLANPara[0].acLAN_Gateway[0] = 0;

  if(GetuShortValue(tFD, (CHAR *)"OTHER_RTU_LINK2_SOCKET_ID",
                    &g_tRTUConfig.tLANPara[0].anOtherRTULinkSocketID) == ERROR)
  {
    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
          "ERR  [INM] GetLink1Info, invalid OTHER_RTU_LINK2_SOCKET_ID\n");
    return ERROR;
  }

  if(GetStringValue(tFD, (CHAR *)"RTU_LAN1_PRO", acTempString) !=
     E_ERR_Success)
  {
    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
          "ERR  [INM] GetLink1Info, invalid RTU_LAN1_PRO\n");
    return ERROR;
  }

  if(strncmp(acTempString, RTU_LAN_PRO_PARA, strlen(RTU_LAN_PRO_PARA)) != 0)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetLink1Info, RTU_LAN1_PRO != %s\n", RTU_LAN_PRO_PARA);
    #endif // CFG_PRN_ERR
    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
          "ERR  [INM] GetLink1Info, invalid RTU_LAN1_PRO != %s\n",
          RTU_LAN_PRO_PARA);
    return ERROR;
  }

  g_unProtocolType = 0;  //Set protocol is not defined.  //040319 Yu Wei

  if (GetLinkProtocol(acTempString, &g_tRTUConfig.tLANPara[0].tPrtc) == ERROR)
    return ERROR;

  memset(acTempString, 0, sizeof(acTempString));
  //Should put here.  //040604 Yu Wei
  strcpy(acTempString,g_tRTUConfig.tLANPara[0].acLAN_IP);
  memset(&g_tRTUConfig.acOtherRTUIPAddress[0], 0,
         sizeof(g_tRTUConfig.acOtherRTUIPAddress[0]));
  if(GetOtherRTUIPAddress(acTempString, g_tRTUConfig.acOtherRTUIPAddress[0],
                          g_tRTUConfig.nRTUID)==ERROR)
    return ERROR;

  if(GetIntegerValue(tFD, (CHAR *)"LAN1_N_POLLING",
                     &g_tRTUConfig.tLANPara[0].nPollingTime) == ERROR)
    return ERROR;

  if(g_tRTUConfig.tLANPara[0].nPollingTime >
       INM_MAX_RTU_TO_RTU_LAN_POLLING_TIME)
  {
    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
      "ERR  [INM] GetLink1Info, RTU_LAN1_N_POLLING exceed %d\n",
      INM_MAX_RTU_TO_RTU_LAN_POLLING_TIME);
    return TYP_ERROR;
  }

  if(GetIntegerValue(tFD, (CHAR *)"LAN1_TIME_OUT",
                     &g_tRTUConfig.tLANPara[0].nPollingTimeout) == ERROR)
  {
    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
      "ERR  [INM] GetLink1Info, invalid LAN1_TIME_OUT empty parameter\n");
    return TYP_ERROR;
  }

  if(g_tRTUConfig.tLANPara[0].nPollingTimeout >
       INM_MAX_RTU_LAN_POLLING_TIMEOUT)
  {
    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
      "ERR  [INM] GetLink1Info, LAN1_TIME_OUT exceeds max value %d\n",
      INM_MAX_RTU_LAN_POLLING_TIMEOUT);
    return TYP_ERROR;
  }

  if(GetIntegerValue(tFD, (CHAR *)"LAN1_N_RETRY",
                     &g_tRTUConfig.tLANPara[0].nPollingRetryNumber) == ERROR)
  {
    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
      "ERR  [INM] GetLink1Info, invalid LAN1_N_RETRY empty parameter\n");
    return TYP_ERROR;
  }


  if(g_tRTUConfig.tLANPara[0].nPollingRetryNumber >
       INM_MAX_RTU_LAN_POLLING_RETRY)
  {
    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
      "ERR  [INM] GetLink1Info, LAN1_N_RETRY exceed max retry %d\n",
      INM_MAX_RTU_LAN_POLLING_RETRY);
    return TYP_ERROR;
  }

  return OK;
}
#endif // CFG_PRIMARY_LAN_PORT_1

#ifdef CFG_PRIMARY_LAN_PORT_4
/*******************************************************************************
Purpose
  Get RTU Link 1 info. Using LAN4 parameters

Input
  tFD -- file descriptor.(in)

Return
  OK    -- Get all parameter.
  ERROR  -- At least one parameter error or cannot find it.

History

  Name             Date       Remark
  ----             ----       ------
  Bryan Chong    18-Nov-11  Update LAN4 to populate primary LAN related
                            variables.

********************************************************************************/
int GetLink1Info(FILE *tFD)
{
  char acTempString[128];
  char cLog[1000];
  //LAN1 link parameter
  if(GetIPAddrValue(tFD, (CHAR *)"RTU_LAN4_IP",
                    g_tRTUConfig.tLANPara[0].acLAN_IP) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetLink1Info, invalid RTU_LAN4_IP\n");
    sprintf(cLog,"ERR  [INM] GetLink1Info, invalid RTU_LAN4_IP\n");
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  if(GetIPAddrValue(tFD, (CHAR *)"RTU_LAN4_NETMASK",
                    g_tRTUConfig.tLANPara[0].acLAN_Netmask) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetLink1Info, invalid RTU_LAN4_NETMASK\n");
    sprintf(cLog,"ERR  [INM] GetLink1Info, invalid RTU_LAN4_NETMASK\n");
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  //Cannot get gateway IP, set it to '\0'.
  if (GetIPAddrValue(tFD, (CHAR *)"RTU_LAN4_GWAY",
                     g_tRTUConfig.tLANPara[0].acLAN_Gateway)== ERROR)
    g_tRTUConfig.tLANPara[0].acLAN_Gateway[0] = 0;

  if(GetuShortValue(tFD, (CHAR *)"OTHER_RTU_LINK2_SOCKET_ID",
                    &g_tRTUConfig.tLANPara[0].anOtherRTULinkSocketID) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetLink1Info, invalid OTHER_RTU_LINK2_SOCKET_ID\n");
    sprintf(cLog,"ERR  [INM] GetLink1Info, invalid OTHER_RTU_LINK2_SOCKET_ID\n");
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  if(GetStringValue(tFD, (CHAR *)"RTU_LAN4_PRO", acTempString) !=
     E_ERR_Success)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetLink1Info, invalid RTU_LAN4_PRO\n");
    sprintf(cLog,"ERR  [INM] GetLink1Info, invalid RTU_LAN4_PRO\n");
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  if(strncmp(acTempString, RTU_LAN_PRO_PARA, strlen(RTU_LAN_PRO_PARA)) != 0)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetLink1Info, RTU_LAN4_PRO != %s\n", RTU_LAN_PRO_PARA);
    #endif // CFG_PRN_ERR
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetLink1Info, invalid RTU_LAN4_PRO != %s\n",
//          RTU_LAN_PRO_PARA);
    sprintf(cLog,"ERR  [INM] GetLink1Info, invalid RTU_LAN4_PRO != %s\n",
            RTU_LAN_PRO_PARA);
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  g_unProtocolType = 0;  //Set protocol is not defined.  //040319 Yu Wei

  if (GetLinkProtocol(acTempString, &g_tRTUConfig.tLANPara[0].tPrtc) == ERROR)
    return ERROR;

  memset(acTempString, 0, sizeof(acTempString));
  //Should put here.  //040604 Yu Wei
  strcpy(acTempString,g_tRTUConfig.tLANPara[0].acLAN_IP);
  memset(&g_tRTUConfig.acOtherRTUIPAddress[0], 0,
         sizeof(g_tRTUConfig.acOtherRTUIPAddress[0]));
  if(GetOtherRTUIPAddress(acTempString, g_tRTUConfig.acOtherRTUIPAddress[0],
                          g_tRTUConfig.nRTUID)==ERROR)
    return ERROR;

  if(GetIntegerValue(tFD, (CHAR *)"LAN4_N_POLLING",
                     &g_tRTUConfig.tLANPara[0].nPollingTime) == ERROR)
    return ERROR;

  if(g_tRTUConfig.tLANPara[0].nPollingTime >
       INM_MAX_RTU_TO_RTU_LAN_POLLING_TIME)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetLink1Info, RTU_LAN4_N_POLLING exceed %d\n",
//      INM_MAX_RTU_TO_RTU_LAN_POLLING_TIME);
    sprintf(cLog,"ERR  [INM] GetLink1Info, RTU_LAN4_N_POLLING exceed %d\n",
    	      INM_MAX_RTU_TO_RTU_LAN_POLLING_TIME);
    g_pEventLog->LogMessage(cLog);
    return TYP_ERROR;
  }

  if(GetIntegerValue(tFD, (CHAR *)"LAN4_TIME_OUT",
                     &g_tRTUConfig.tLANPara[0].nPollingTimeout) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetLink1Info, invalid LAN4_TIME_OUT empty parameter\n");
    sprintf(cLog,"ERR  [INM] GetLink1Info, invalid LAN4_TIME_OUT empty parameter\n");
    g_pEventLog->LogMessage(cLog);
    return TYP_ERROR;
  }

  if(g_tRTUConfig.tLANPara[0].nPollingTimeout >
       INM_MAX_RTU_LAN_POLLING_TIMEOUT)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetLink1Info, LAN4_TIME_OUT exceeds max value %d\n",
//      INM_MAX_RTU_LAN_POLLING_TIMEOUT);

    sprintf(cLog, "ERR  [INM] GetLink1Info, LAN4_TIME_OUT exceeds max value %d\n",
    	      INM_MAX_RTU_LAN_POLLING_TIMEOUT);
    g_pEventLog->LogMessage(cLog);

    return TYP_ERROR;
  }

  if(GetIntegerValue(tFD, (CHAR *)"LAN4_N_RETRY",
                     &g_tRTUConfig.tLANPara[0].nPollingRetryNumber) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetLink1Info, invalid LAN4_N_RETRY empty parameter\n");
    sprintf(cLog,"ERR  [INM] GetLink1Info, invalid LAN4_N_RETRY empty parameter\n");
    g_pEventLog->LogMessage(cLog);
    return TYP_ERROR;
  }


  if(g_tRTUConfig.tLANPara[0].nPollingRetryNumber >
       INM_MAX_RTU_LAN_POLLING_RETRY)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetLink1Info, LAN4_N_RETRY exceed max retry %d\n",
//      INM_MAX_RTU_LAN_POLLING_RETRY);
    sprintf(cLog, "ERR  [INM] GetLink1Info, LAN4_N_RETRY exceed max retry %d\n",
    	      INM_MAX_RTU_LAN_POLLING_RETRY);
    g_pEventLog->LogMessage(cLog);
    return TYP_ERROR;
  }

  return OK;
}// GetLink1Info
#endif // CFG_PRIMARY_LAN_PORT_4
/*******************************************************************************
Purpose
  Get RTU Link 2 info.
  Added to decreas the complexity in 22 March 2004, Yu Wei

Input
  tFD -- file descriptor.(in)

Return
  OK    -- Get all parameter.
  ERROR  -- At least one parameter error or cannot find it.

History

  Name         Date       Remark
  ----         ----       ------
  Bryan Chong  14-Oct-09  Update to check RTU_LANx_PRO must carry parameter
                          "MODBUS,0x01,0x04" defined as RTU_LAN_PRO_PARA
  Bryan Chong  15-Oct-09  Update g_tRTUConfig.tLAN_Para[x].cLAN_SlaveAddress
                          to g_tRTUConfig.tLAN_Para[x].tPrtc.addr due to
                          parameter change in latest Init_Config.cpp
  Bryan Chong  21-Oct-09  Fix invalid IP by clearing memory from acTempString
                          using memset.
  Zhang Shi Lun  22-Oct-09  Add memset(&g_tRTUConfig.acOtherRTUIPAddress[0], 0,
                              sizeof(g_tRTUConfig.acOtherRTUIPAddress[0]));
                              to clear other IP address buffer writing.
  Bryan Chong  02-Aug-11  Set max value for LAN2_N_POLLING, LAN2_TIME_OUT, and
                            LAN2_N_RETRY.
*******************************************************************************/
int GetLink2Info(FILE *tFD)
{
  char acTempString[128];
  char cLog[1000];
  //LAN2 link parameter //040316 Yu Wei
  if(GetIPAddrValue(tFD, (CHAR *)"RTU_LAN2_IP",
                    g_tRTUConfig.tLANPara[1].acLAN_IP) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetLink2Info, invalid RTU_LAN2_IP\n");
    sprintf(cLog,"ERR  [INM] GetLink2Info, invalid RTU_LAN2_IP\n");
    g_pEventLog->LogMessage(cLog);

    return ERROR;
  }

  //040316 Yu Wei
  if(GetIPAddrValue(tFD, (CHAR *)"RTU_LAN2_NETMASK",
                    g_tRTUConfig.tLANPara[1].acLAN_Netmask) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetLink2Info, invalid RTU_LAN2_NETMASK\n");
    sprintf(cLog,"ERR  [INM] GetLink2Info, invalid RTU_LAN2_NETMASK\n");
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  //Cannot get gateway IP, set it to '\0'.  //040316 Yu Wei
  if (GetIPAddrValue(tFD, (CHAR *)"RTU_LAN2_GWAY",
                     g_tRTUConfig.tLANPara[1].acLAN_Gateway)== ERROR)
    g_tRTUConfig.tLANPara[1].acLAN_Gateway[0] = 0;

  //040309 Yu Wei
  if(GetuShortValue(tFD, (CHAR *)"OTHER_RTU_LINK1_SOCKET_ID",
                    &g_tRTUConfig.tLANPara[1].anOtherRTULinkSocketID) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetLink2Info, invalid OTHER_RTU_LINK1_SOCKET_ID\n");
    sprintf(cLog,"ERR  [INM] GetLink2Info, invalid OTHER_RTU_LINK1_SOCKET_ID\n");
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  if(GetStringValue(tFD, (CHAR *)"RTU_LAN2_PRO", acTempString)
     != E_ERR_Success)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetLink2Info, invalid RTU_LAN2_PRO\n");
    sprintf(cLog,"ERR  [INM] GetLink2Info, invalid RTU_LAN2_PRO\n");
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  if(strncmp(acTempString, RTU_LAN_PRO_PARA, strlen(RTU_LAN_PRO_PARA)) != 0)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetLink2Info, RTU_LAN2_PRO != %s\n", RTU_LAN_PRO_PARA);
    #endif // CFG_PRN_ERR
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetLink2Info, invalid RTU_LAN2_PRO != %s\n",
//          RTU_LAN_PRO_PARA);
    sprintf(cLog, "ERR  [INM] GetLink2Info, invalid RTU_LAN2_PRO != %s\n",
            RTU_LAN_PRO_PARA);
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  if (GetLinkProtocol(acTempString, &g_tRTUConfig.tLANPara[1].tPrtc) == ERROR)
    return ERROR;

  if(g_tRTUConfig.tLANPara[1].tPrtc.addr!= g_tRTUConfig.tLANPara[0].tPrtc.addr)
  {
    WriteDebugLog(
      (CHAR *)" *** The slave address of LAN1 and LAN4 is different ***\n");
    return ERROR;
  }

  memset(acTempString, 0, sizeof(acTempString));
  strcpy(acTempString,g_tRTUConfig.tLANPara[1].acLAN_IP);
  memset(&g_tRTUConfig.acOtherRTUIPAddress[1], 0,
         sizeof(g_tRTUConfig.acOtherRTUIPAddress[1]));
  if(GetOtherRTUIPAddress(acTempString,
     g_tRTUConfig.acOtherRTUIPAddress[1], g_tRTUConfig.nRTUID) == ERROR)
    return ERROR;

  if(GetIntegerValue(tFD, (CHAR *)"LAN2_N_POLLING",
     &g_tRTUConfig.tLANPara[1].nPollingTime) == ERROR)
    return ERROR;

  if(g_tRTUConfig.tLANPara[1].nPollingTime >
     INM_MAX_RTU_TO_RTU_LAN_POLLING_TIME)
  {
//     pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//       FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//       "ERR  [INM] GetLink2Info, RTU_LAN2_N_POLLING exceed %d\n",
//       INM_MAX_RTU_TO_RTU_LAN_POLLING_TIME);

     sprintf(cLog,"ERR  [INM] GetLink2Info, RTU_LAN2_N_POLLING exceed %d\n",
    	       INM_MAX_RTU_TO_RTU_LAN_POLLING_TIME);
     g_pEventLog->LogMessage(cLog);

     return TYP_ERROR;
  }

  if(GetIntegerValue(tFD, (CHAR *)"LAN2_TIME_OUT",
                     &g_tRTUConfig.tLANPara[1].nPollingTimeout) == ERROR)
  {
//     pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//       FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//       "ERR  [INM] GetLink2Info, invalid LAN2_TIME_OUT empty parameter\n");
     sprintf(cLog,"ERR  [INM] GetLink2Info, invalid LAN2_TIME_OUT empty parameter\n");
     g_pEventLog->LogMessage(cLog);

     return TYP_ERROR;
  }

  if(g_tRTUConfig.tLANPara[1].nPollingTimeout > INM_MAX_RTU_LAN_POLLING_TIMEOUT)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetLink2Info, LAN2_TIME_OUT exceeds max value %d\n",
//      INM_MAX_RTU_LAN_POLLING_TIMEOUT);
    sprintf(cLog,"ERR  [INM] GetLink2Info, LAN2_TIME_OUT exceeds max value %d\n",
    	      INM_MAX_RTU_LAN_POLLING_TIMEOUT);
    g_pEventLog->LogMessage(cLog);

    return TYP_ERROR;
  }

  if(GetIntegerValue(tFD, (CHAR *)"LAN2_N_RETRY",
                     &g_tRTUConfig.tLANPara[1].nPollingRetryNumber) == ERROR)
  {
//     pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//       FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//       "ERR  [INM] GetLink2Info, invalid LAN2_N_RETRY empty parameter\n");
     sprintf(cLog,"ERR  [INM] GetLink2Info, invalid LAN2_N_RETRY empty parameter\n");
     g_pEventLog->LogMessage(cLog);

     return TYP_ERROR;
  }

  if(g_tRTUConfig.tLANPara[1].nPollingRetryNumber >
     INM_MAX_RTU_LAN_POLLING_RETRY)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetLink2Info, LAN1_N_RETRY exceed max retry %d\n",
//      INM_MAX_RTU_LAN_POLLING_RETRY);

    sprintf(cLog,"ERR  [INM] GetLink2Info, LAN1_N_RETRY exceed max retry %d\n",
    	      INM_MAX_RTU_LAN_POLLING_RETRY);
    g_pEventLog->LogMessage(cLog);
    return TYP_ERROR;
  }

  return OK;
}// GetLink2Info
/*******************************************************************************
Purpose
  Acquire LAN link 3 information for SWC-LAN communication

Parameter
  tFD   [in] pointer to the opened file descriptor

Return
  OK    Success.
  ERROR Fail

History

  Name         Date       Remark
  ----         ----       ------
  Bryan Chong  03-Jan-10  Initial revision

*******************************************************************************/
static int GetLink3Info(FILE *tFD)
{
	char cLog[1000];
  //LAN3 link parameter
  if (GetIPAddrValue(tFD, (CHAR *)"RTU_LAN3_IP",
       g_tRTUConfig.tLANPara[2].acLAN_IP) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetLink3Info, invalid RTU_LAN3_IP\n");
    sprintf(cLog, "ERR  [INM] GetLink3Info, invalid RTU_LAN3_IP\n");
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  if (GetIPAddrValue(tFD, (CHAR *)"RTU_LAN3_NETMASK",
       g_tRTUConfig.tLANPara[2].acLAN_Netmask) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetLink3Info, invalid RTU_LAN3_NETMASK\n");

    sprintf(cLog, "ERR  [INM] GetLink3Info, invalid RTU_LAN3_NETMASK\n");
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  //Cannot get gateway IP, set it to '\0'.
  if (GetIPAddrValue(tFD, (CHAR *)"RTU_LAN3_GWAY",
        g_tRTUConfig.tLANPara[2].acLAN_Gateway)== ERROR)
    g_tRTUConfig.tLANPara[2].acLAN_Gateway[0] = 0;

  return OK;
}// GetLink3Info

/*******************************************************************************
Purpose
  Acquire LAN link 6 information for RMT communication and debugging purpose

Input
  tFD   [in] pointer to the opened file descriptor

Return
  OK    Success
  ERROR Fail

History

  Name           Date       Remark
  ----           ----       ------
  Bryan Chong  20-Sep-10  Initial revision

*******************************************************************************/
static int GetLink6Info(FILE *tFD)
{ char cLog[1000];
  //LAN6 link parameter
  if (GetIPAddrValue(tFD, (CHAR *)"RTU_LAN6_IP",
       g_tRTUConfig.tLANPara[5].acLAN_IP) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetLink6Info, invalid RTU_LAN6_IP\n");
    sprintf(cLog,"ERR  [INM] GetLink6Info, invalid RTU_LAN6_IP\n");
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  if (GetIPAddrValue(tFD, (CHAR *)"RTU_LAN6_NETMASK",
       g_tRTUConfig.tLANPara[5].acLAN_Netmask) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetLink6Info, invalid RTU_LAN6_NETMASK\n");
    sprintf(cLog,"ERR  [INM] GetLink6Info, invalid RTU_LAN6_NETMASK\n");
    g_pEventLog->LogMessage(cLog);
    return ERROR;
  }

  //Cannot get gateway IP, set it to '\0'.
  if (GetIPAddrValue(tFD, (CHAR *)"RTU_LAN6_GWAY",
        g_tRTUConfig.tLANPara[5].acLAN_Gateway)== ERROR)
    g_tRTUConfig.tLANPara[5].acLAN_Gateway[0] = 0;

  return OK;
}// GetLink6Info

/*******************************************************************************
Purpose
  Get one parameter string from a string that consists of more than one
  parameter.

Input/Output
  acMutliValue    -- orginal string (in)
               and store new string that kick out first parameter (out)
  acOneValue      -- Output one parameter string (out)
  cDivideCharacter  -- Divid character (in)

Reture
  false -- Last parameter
  true  -- Not last parameter.
*******************************************************************************/
bool SearchMultiValue(char *acMutliValue, char *acOneValue,
                      char cDivideCharacter)
{
  char *pcComma;
  int nPosition;

  pcComma = strchr(acMutliValue,cDivideCharacter);
  if(pcComma == TYP_NULL)
  {
    strcpy(acOneValue,acMutliValue);
    return false;
  }
  else
  {
    nPosition = pcComma - acMutliValue;
    strncpy(acOneValue,acMutliValue,nPosition);
    acOneValue[nPosition] = '\0';
    strcpy(acMutliValue, &acMutliValue[nPosition+1]);
    return true;
  }
}

/*******************************************************************************
Purpose
  Get VDU user's name.

Input
  acBuffer  -- orginal string (in)

Reture
  OK    -- At least get one user's name.
  ERROR  -- nVDUUserNumber =0.
*******************************************************************************/
int GetVDUName(char *acBuffer)    //031001
{
  char acOneString[10];
  int nI;

  for(nI=0; nI < VDU_MAX_USER; nI++)
    g_tRTUConfig.tVDUNameList[nI].acVDUName[0] = 0;

  g_tRTUConfig.nVDUUserNumber = 0;

  while(1)
  {
    if(SearchMultiValue(acBuffer,acOneString,',') == false)
    {
      strcpy(g_tRTUConfig.tVDUNameList[g_tRTUConfig.nVDUUserNumber].acVDUName,
             acOneString);
      g_tRTUConfig.nVDUUserNumber++;

      if((strlen(acOneString) <= 0) ||
         (strlen(acOneString) > INM_USERNAME_MAX_SZ))  //040429 Yu Wei
      {                        //Check string length
        g_tRTUConfig.nVDUUserNumber = 0;
      }
      break;
    }
    else
    {
      strcpy(g_tRTUConfig.tVDUNameList[g_tRTUConfig.nVDUUserNumber].acVDUName,
             acOneString);
      g_tRTUConfig.nVDUUserNumber++;
      if(g_tRTUConfig.nVDUUserNumber > VDU_MAX_USER)  //040309 Yu Wei
        break;
    }

    if((strlen(acOneString) <= 0) ||
       (strlen(acOneString) > INM_USERNAME_MAX_SZ))  //040309 Yu Wei
    {
      g_tRTUConfig.nVDUUserNumber = 0;
      break;
    }
  }

  if((g_tRTUConfig.nVDUUserNumber <= 0) ||
     (g_tRTUConfig.nVDUUserNumber > VDU_MAX_USER))
    return ERROR;
  else
    return OK;
}

/*******************************************************************************
Purpose
  Get VDU user's password.

Input
  acBuffer  -- orginal string (in)

Return
  OK    -- User's password number is same as user's name.
  ERROR  -- User's password number is different to user's name (nVDUUserNumber).
*******************************************************************************/
int GetVDUPassword(char *acBuffer)    //031001
{
  char acOneString[10];
  int nI=0;
  char cLog[1000];

  while(1)
  {
    if(SearchMultiValue(acBuffer,acOneString,',') == false)
    {
      strcpy(g_tRTUConfig.tVDUNameList[nI].acVDUPassword,acOneString);
      nI++;

      if((strlen(acOneString) <=0) ||
         (strlen(acOneString) > INM_PASSWORD_MAX_SZ))
      {
        //Check string length.
        nI = 0;
//        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetVDUPassword, last password exceed max str sz %d\n",
//          INM_PASSWORD_MAX_SZ);
        sprintf(cLog,"ERR  [INM] GetVDUPassword, last password exceed max str sz %d\n",
                INM_PASSWORD_MAX_SZ);
        g_pEventLog->LogMessage(cLog);
      }

      break;
    }
    else
    {
      strcpy(g_tRTUConfig.tVDUNameList[nI].acVDUPassword,acOneString);
      nI++;
      if(nI > VDU_MAX_USER)    //040309 Yu Wei
      {
//        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetVDUPassword, exceed max no of password %d\n",
//          VDU_MAX_USER);
//
        sprintf(cLog,"ERR  [INM] GetVDUPassword, exceed max no of password %d\n",
                VDU_MAX_USER);
        g_pEventLog->LogMessage(cLog);
        break;
      }
    }

    if((strlen(acOneString) <= 0) ||
       (strlen(acOneString) > INM_PASSWORD_MAX_SZ))  //040309 Yu Wei
    {
      nI = 0;
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//        FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//        "ERR  [INM] GetVDUPassword, exceed password max str sz %d\n",
//        INM_PASSWORD_MAX_SZ);

      sprintf(cLog,"ERR  [INM] GetVDUPassword, exceed password max str sz %d\n",
    	        INM_PASSWORD_MAX_SZ);
      g_pEventLog->LogMessage(cLog);
      break;
    }
  }

  if(nI == g_tRTUConfig.nVDUUserNumber)
    return OK;
  else
  {
    //WriteDebugLog((CHAR *)
      //" *** The User number and password number are different ***\n");
    return ERROR;      //040322 Yu Wei
  }

}

/*******************************************************************************
Purpose
  Get Modbus protocol's slave address and read command code.
  The read command code can be 0x03 or 0x04.
  //Get protocol name, slave address if it exist and Modbus read command code if
  //it is Modbus protocol.

Input/Output
  acBuffer  -- orginal string. (in)
  //acProtocol  -- Protocol name. (out)  Not used.
  cAddress  -- Slave address. (out)
  cCommandCode-- Modbus read command code. (out)

Reture
  OK    -- Successsful.
  ERROR  -- protocol name is not MODBUS, slave address error or read command
            error.
*******************************************************************************/
int GetLinkProtocol(char *acBuffer, struct link_protocol* pPrt)
{
  int nTemp;
  char acOneString[50] = {0};

  if(SearchMultiValue(acBuffer,acOneString,',') == false)
  {
    //Only protocol name exist.
    if ( strcmp(acOneString, "PR_CLK") == 0)  //PR_CLK protocol.
    {
      //pPrt->protocol = PRT_CLOCK;
      pPrt->protocol = E_CLKP_Generic;
      g_unProtocolType = E_CLKP_Generic;

//    }else if(strcmp(acOneString, "PR_NTP") == 0){
//      pPrt->protocol = E_CLKP_NTP;
//      g_unProtocolType = E_CLKP_NTP;
    }
    else
      return ERROR;
  }
  else
  {
    //Check protocol name.
    if ( strcmp(acOneString, "MODBUS") != 0)  //protocol name error.
    {
      return TYP_ERROR;
    }
    //pPrt->protocol = PRT_MODBUS;
    pPrt->protocol = E_CLKP_Modbus;
    g_unProtocolType = E_CLKP_Modbus;

    if(SearchMultiValue(acBuffer,acOneString,',') == false)
    {
      return TYP_ERROR;
    }
    else
    {

      if( GetDigit(acOneString, &nTemp) == OK)
      {
        if (( nTemp == 0 ) || (nTemp > 247))  //slave address range.
          return TYP_ERROR;
        else
        {
          pPrt->addr = (char)nTemp;
          if (SearchMultiValue(acBuffer,acOneString,',') == false)
          {
            if( GetDigit(acOneString, &nTemp) == OK)
            {
              if( ( nTemp != 0x03 ) && ( nTemp != 0x04 ) )  //read command code.
              {
                return ERROR;
              }
              else
              {
                pPrt->command = nTemp;
              }
            }
            else
            {
              return ERROR;
            }
          }
          else    //configuration file error.
          {
            return ERROR;
          }
        }
      }
      else
      {
        return ERROR;
      }
    }
  }

  return OK;
}// GetLinkProtocol


/**********************************************************************************
Purpose
  Get Link type. For multidrop link, it also gets group info.

Input/Output
  acBuffer  -- orginal string. (in)
  pnLinkType  -- Link type ID. (out)
  pnGroup    -- Group info. (out)

Return
  OK    -- Get all parameter.
  ERROR  -- At least one parameter error or cannot find it.
************************************************************************************/
int GetLinkType(char *acBuffer, int *pnLinkType, int *pnGroup)
{
  char acOneString[10];
  char cLog[1000];
  if(strlen(acBuffer) > sizeof(acOneString))
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetLinkType, defined INTERFACE parameter exceeds %d "
//      "characters\n",
//      sizeof(acOneString));
    sprintf(cLog, "ERR  [INM] GetLinkType, defined INTERFACE parameter exceeds %d "
    	      "characters\n",
    	      sizeof(acOneString));
    g_pEventLog->LogMessage(cLog);

    return TYP_ERROR;
  }

  if(SearchMultiValue(acBuffer,acOneString,',') == false)
  {  //Point to point link
    *pnGroup = 0x00;  //No group info.
    if(strcmp(acOneString,"RS_485") == 0)      //040311 Yu Wei
    {
      *pnLinkType = SWC_LINK_RS_485;
    }
    else if(strcmp(acOneString,"LAN") == 0)      //040311 Yu Wei
    {
      *pnLinkType = SWC_LINK_LAN;
    }
//    else if(strcmp(acOneString,"RS_485_M") == 0)      //040311 Yu Wei
//    {
//      *pnLinkType = SWC_LINK_RS_485M;
//    }
//
//    else if(strcmp(acOneString,"RS_422") == 0)    //040311 Yu Wei
//    {
//      *pnLinkType = SWC_LINK_RS_422;
//    }
//    else if(strcmp(acOneString,"RS_232") == 0)    //040311 Yu Wei
//    {
//      *pnLinkType = SWC_LINK_RS_232;
//    }
    else
      return ERROR;
  }
  return OK;
}

/**********************************************************************************
Purpose
  Get all RTU server table. Max 3 tables.

Input
  tFD -- file descriptor.(in)

Return
  OK    -- Get all parameter.
  ERROR  -- At least one parameter error or cannot find it.

History

   Name         Date         Remark
   ----         ----         ------
 Yu, Wei      09-Mar-2009  Create initial revision
 Bryan Chong  07-Sep-2011  Add condition to check if RTU_TABLE_END equals to
                           INM_RTU_STATUS_SZ [C955, PR110]
************************************************************************************/
int GetAllRTUTable(FILE *tFD)
{
  char acReadBuffer[INM_MAXSIZE_LINE+1];
  char acTempString[128];
  int nGetTableNumber = 0;
  char cLog[1000];
  while(1)
  {
    ReadOneLine(acReadBuffer,tFD);

    if( acReadBuffer[0] == 0 )
      return ERROR;

    if(strncmp(acReadBuffer,"BGN_RTU_TABLE",strlen("BGN_RTU_TABLE")) == 0)
    {
      WriteDebugLog(acReadBuffer);

      if(GetStringValue(tFD, (CHAR *)"RTU_TABLE_NAME", acTempString)
         != E_ERR_Success)
      {
//        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetAllRTUTable, invalid RTU_TABLE_NAME\n");
        sprintf(cLog, "ERR  [INM] GetAllRTUTable, invalid RTU_TABLE_NAME\n");
        g_pEventLog->LogMessage(cLog);

        return TYP_ERROR;
      }

      //RTU status table
      if(strcmp(acTempString,"RTU_STATUS") == 0)
      {
        //Get one table 040309 Yu Wei
        nGetTableNumber ++;

        if(GetRTUTable(tFD,&g_tRTUConfig.tRTUStatusTable)== ERROR)
          return ERROR;

        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
        printf("[INM] GetAllRTUTable, start addr %d, end addr %d\n",
               g_tRTUConfig.tRTUStatusTable.unTableStartAddress,
               g_tRTUConfig.tRTUStatusTable.unTableEndAddress);
        #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)

        if(g_tRTUConfig.tRTUStatusTable.unTableEndAddress != INM_RTU_STATUS_SZ)
        {
//          pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//            FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//            "ERR  [INM] GetAllRTUTable, invalid RTU_TABLE_END != %d\n",
//            INM_RTU_STATUS_SZ);
          sprintf(cLog,"ERR  [INM] GetAllRTUTable, invalid RTU_TABLE_END != %d\n",
                  INM_RTU_STATUS_SZ);
          g_pEventLog->LogMessage(cLog);
          return TYP_ERROR;
        }

      }
      //RTU polling table. Used for server table overlap checking.
      else if(strcmp(acTempString,"RTU_POLLING") == 0)
      {
        if(GetRTUTable(tFD,&g_tRTUConfig.tRTUPollingTable)== ERROR)
        {
          return ERROR;
        }
        else
        {
          if(AllocateServerTable(
               g_tRTUConfig.tRTUPollingTable.unTableStartAddress,
               g_tRTUConfig.tRTUPollingTable.unTableEndAddress -
               g_tRTUConfig.tRTUPollingTable.unTableStartAddress + 1)
             == ERROR)    //040512 Yu Wei
          {
            WriteDebugLog((CHAR *)" *** RTU polling table overlap ***\n");
            return ERROR;
          }
        }
      }

      //RTU command table.
      else if(strcmp(acTempString,"RTU_COMMAND") == 0)  //040311 Yu Wei
      {
        nGetTableNumber ++;    //Get one table 040309 Yu Wei

        if(GetRTUTable(tFD,&g_tRTUConfig.tRTUComanmdTable)== ERROR)
          return ERROR;
      }
    }
    else if(strncmp(acReadBuffer,"BGN_SERVER_CFG",strlen("BGN_SERVER_CFG")) == 0)
    {
      WriteDebugLog(acReadBuffer);
      if ( nGetTableNumber < 2 )    //The table number in config.txt less than 2. 040309 Yu Wei
        return ERROR;
      else
        break;
    }
  }
  return OK;
}

/**********************************************************************************
Purpose
  Get one RTU server table.

Input/Output
  tFD      -- file descriptor.(in)
  ptRTUTable  -- RTU table structure. (out)

Return
  OK    -- Get all parameter.
  ERROR  -- At least one parameter error or cannot find it.
************************************************************************************/
int GetRTUTable(FILE *tFD, tRTUTableStructure *ptRTUTable)
{
	char cLog[1000];
  if(GetuShortValue(tFD, (CHAR *)"RTU_TABLE_BGN",
                    &ptRTUTable->unTableStartAddress) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetRTUTable, invalid RTU_TABLE_BGN\n");
    sprintf(cLog,"ERR  [INM] GetRTUTable, invalid RTU_TABLE_BGN\n");
    g_pEventLog->LogMessage(cLog);

    return TYP_ERROR;
  }

  if(GetuShortValue(tFD, (CHAR *)"RTU_TABLE_END",
                    &ptRTUTable->unTableEndAddress) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetRTUTable, invalid RTU_TABLE_END\n");
    sprintf(cLog,"ERR  [INM] GetRTUTable, invalid RTU_TABLE_END\n");
    g_pEventLog->LogMessage(cLog);
    return TYP_ERROR;
  }

  if( ptRTUTable->unTableEndAddress < ptRTUTable->unTableStartAddress)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetRTUTable, invalid value RTU_TABLE_END < RTU_TABLE_BGN\n");
    sprintf(cLog,"ERR  [INM] GetRTUTable, invalid value RTU_TABLE_END < RTU_TABLE_BGN\n");
    g_pEventLog->LogMessage(cLog);
    return TYP_ERROR;
  }
  return OK;
}

/**********************************************************************************
Purpose
  Get server definition

Input
  tFD    -- file descriptor.(in)

Return
  OK    -- Get all parameter.
  ERROR  -- At least one parameter error or cannot be found it.
*************************************************************************************/
int GetServerDef(FILE *tFD)
{
	char cLog[1000];
  if(GetuShortValue(tFD, (CHAR *)"RTU_COMMAND_SOCKET_ID",
                    &g_tRTUConfig.nRTUCommandSoctetID) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetServerDef, invalid RTU_COMMAND_SOCKET_ID\n");
    sprintf(cLog,"ERR  [INM] GetServerDef, invalid RTU_COMMAND_SOCKET_ID\n");
    g_pEventLog->LogMessage(cLog);
    return TYP_ERROR;
  }

  //Get polling socket number and socket ID.
  if(GetIntegerValue(tFD, (CHAR *)"RTU_POLLING_SOCKET_NUMBER",
                     &g_tRTUConfig.nRTUPollingSocketNumber) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetServerDef, invalid RTU_POLLING_SOCKET_NUMBER\n");

    sprintf(cLog,"ERR  [INM] GetServerDef, invalid RTU_POLLING_SOCKET_NUMBER\n");
    g_pEventLog->LogMessage(cLog);
    return TYP_ERROR;
  }
  else
  {
    if((g_tRTUConfig.nRTUPollingSocketNumber > SERVER_POLLING_SOCKET_MAX) ||
      (g_tRTUConfig.nRTUPollingSocketNumber <= 0))
    {
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//        FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//        "ERR  [INM] GetServerDef, RTU_POLLING_SOCKET_NUMBER exceeds max size "
//        "%d\n", SERVER_POLLING_SOCKET_MAX);
      sprintf(cLog,"ERR  [INM] GetServerDef, RTU_POLLING_SOCKET_NUMBER exceeds max size "
    	        "%d\n", SERVER_POLLING_SOCKET_MAX);
      g_pEventLog->LogMessage(cLog);
      return ERROR;
    }
  }

  if(GetuShortValue(tFD, (CHAR *)"RTU_POLLING_SOCKET_ID1",
                    &g_tRTUConfig.nRTUPollingSocketID[0]) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetServerDef, invalid RTU_POLLING_SOCKET_ID1\n");
    sprintf(cLog,"ERR  [INM] GetServerDef, invalid RTU_POLLING_SOCKET_ID1\n");
    g_pEventLog->LogMessage(cLog);

    return TYP_ERROR;
  }

  if(g_tRTUConfig.nRTUPollingSocketNumber >= 2)
  {
    if(GetuShortValue(tFD, (CHAR *)"RTU_POLLING_SOCKET_ID2",
                      &g_tRTUConfig.nRTUPollingSocketID[1]) == ERROR)
    {
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//        FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//        "ERR  [INM] GetServerDef, invalid RTU_POLLING_SOCKET_ID2\n");
      sprintf(cLog,"ERR  [INM] GetServerDef, invalid RTU_POLLING_SOCKET_ID2\n");
      g_pEventLog->LogMessage(cLog);
      return TYP_ERROR;
    }
  }
//
  if(g_tRTUConfig.nRTUPollingSocketNumber >= 3)
  {
    if(GetuShortValue(tFD, (CHAR *)"RTU_POLLING_SOCKET_ID3",
                      &g_tRTUConfig.nRTUPollingSocketID[2]) == ERROR)
    {
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//        FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//        "ERR  [INM] GetServerDef, invalid RTU_POLLING_SOCKET_ID3\n");
      sprintf(cLog,"ERR  [INM] GetServerDef, invalid RTU_POLLING_SOCKET_ID3\n");
      g_pEventLog->LogMessage(cLog);
      return TYP_ERROR;
    }
  }

  if(g_tRTUConfig.nRTUPollingSocketNumber >= 4)
  {
    if(GetuShortValue(tFD, (CHAR *)"RTU_POLLING_SOCKET_ID4",
                      &g_tRTUConfig.nRTUPollingSocketID[3]) == ERROR)
    {
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//        FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//        "ERR  [INM] GetServerDef, invalid RTU_POLLING_SOCKET_ID4\n");
      sprintf(cLog, "ERR  [INM] GetServerDef, invalid RTU_POLLING_SOCKET_ID4\n");
      g_pEventLog->LogMessage(cLog);
      return TYP_ERROR;
    }
  }

  if(g_tRTUConfig.nRTUPollingSocketNumber >= 5)
  {
    if(GetuShortValue(tFD, (CHAR *)"RTU_POLLING_SOCKET_ID5",
                      &g_tRTUConfig.nRTUPollingSocketID[4]) == ERROR)
    {
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//        FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//        "ERR  [INM] GetServerDef, invalid RTU_POLLING_SOCKET_ID5\n");
      sprintf(cLog,"ERR  [INM] GetServerDef, invalid RTU_POLLING_SOCKET_ID5\n");
      g_pEventLog->LogMessage(cLog);
      return TYP_ERROR;
    }
  }

  //Get timeout for server command.
  if(GetIntegerValue(tFD, (CHAR *)"TIMEOUT_COM_SERVER",
                     &g_tRTUConfig.nServerCMDTimeout) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetServerDef, invalid TIMEOUT_COM_SERVER\n");
    sprintf(cLog,"ERR  [INM] GetServerDef, invalid TIMEOUT_COM_SERVER\n");
    g_pEventLog->LogMessage(cLog);
    return TYP_ERROR;
  }

  return OK;
}

/******************************************************************************
Function Name : GetAllSWCParameter
Purpose       : Get all SWC parameter. It also determine the device ID of
                multidrop link SWC.
Input Parameter: tFD, file descriptor.
Output Parameter: None
Global Modified : g_tRTUConfig.anSWCIndex, g_tRTUConfig.anMultidorpSWCChain,
                    g_tRTUConfig.nSwcEnabled

Revisions:
  Date         Name         Remarks
  04-Mar-2009  Yu Wei       Initial revision
  21-Oct-2009  Bryan Chong  Increase acTemp size from 256 to INM_MAXSIZE_LINE.
                            This fixed the issue of missing symbol in
                            debugger
  10-Feb-2010  Bryan Chong  Add NTP client parameter setup
  17-May-2010  Bryan Chong  Add global variable g_tRTUConfig.nTotalEnableSwc
                            to keep track of the totatl enabled SWCs
********************************************************************************/
int GetAllSWCParameter(FILE *tFD)//20090626 tong rewrite
{
  char acTemp[INM_MAXSIZE_LINE] = {0};
  char acSection[20] = {0};

  for(int n=0; n<SWC_MAX_NUMBER; n++)    //initializate the buffer. 040325 YU Wei
  {
    g_tRTUConfig.anSWCIndex[n] = 0xFFFF;
    g_tRTUConfig.anMultidorpSWCChain[n] = 0xFFFF;
  }

  if(GetSWCConfig(tFD) == ERROR)  //SWC configuration.
  {
    return ERROR;
  }

  // 20100517 BC (Rqd ZSL)
  g_tRTUConfig.nTotalEnableSwc = 0;
  g_tRTUConfig.nFirstSwcEnabled = TYP_ERROR;
  for(int nI=0; nI<SWC_MAX_NUMBER; nI++)
  {
    if(g_tRTUConfig.nSwcEnabled[nI] == 1)
    {
      //find the section
      sprintf(acSection, "[%s]", g_acSWCKeyWordName[nI]);
      if (SearchKeyWord(tFD, acSection, acTemp) == false)
      {
        return ERROR;
      }



      if (GetSWCParameter(tFD, nI) == ERROR)  //One SWC parameter.
      {
        return ERROR;
      }
      // 20100517 BC (Rqd ZSL)
      g_tRTUConfig.nTotalEnableSwc++;

      if(g_tRTUConfig.nFirstSwcEnabled == TYP_ERROR)
        g_tRTUConfig.nFirstSwcEnabled = nI;

    } // if(g_tRTUConfig.nSwcEnabled[nI] == 1)
  }

  if( SerialPortIDCheck() == ERROR)  //Check serial port conflict
    return ERROR;

  return (CheckSwcName() == OK ? OK:ERROR);

}// GetAllSWCParameter

// Check duplication of name
int CheckSwcName(void)
{
  for(int i = 0; i < SWC_MAX_NUMBER; i++)
  {
    if(g_tRTUConfig.nSwcEnabled[i] != 1)
      continue;

    for (int j = i+1; j < SWC_MAX_NUMBER; j++)
    {
      if(g_tRTUConfig.nSwcEnabled[j] != 1)
        continue;

      if (strcmp(g_acSWCDeviceName[i], g_acSWCDeviceName[j]) == 0)
      {
        printf("Duplicated SWC names.(%s)", g_acSWCDeviceName[i]);
        return ERROR;
      }
    }
  }
  return OK;

}

/**********************************************************************************
Purpose
  Get SWC configuration.
  Re-design in 16 March 2004, Yu Wei

Input
  tFD -- file descriptor.(in)

Return
  OK    -- Get all parameter.
  ERROR  -- At least one parameter error or cannot find it.
************************************************************************************/
int GetSWCConfig(FILE *tFD) //20090626 tong rewrite
{
  char acReadBuffer[INM_MAXSIZE_LINE+1];
  char acTemp[INM_MAXSIZE_LINE+1];
  int nReturn = OK;
  int nValueStringPosition;
  char *pcEqual;
  int nK, nJ=0;
  int nTemp;
  char cLog[1000];


  if(SearchKeyWord(tFD,(CHAR *)"BGN_EXT_DEF",acReadBuffer) == false)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//      FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//      "ERR  [INM] GetSWCConfig, missing BGN_EXT_DEF\n");
    sprintf(cLog,"ERR  [INM] GetSWCConfig, missing BGN_EXT_DEF\n");
    g_pEventLog->LogMessage(cLog);
    nReturn = ERROR;
  }

  g_tRTUConfig.nSWCNumber = 0;

  for(nK=0; nK<SWC_MAX_NUMBER; nK++)
  {
    g_tRTUConfig.acSWCAddress[nK] = 0;
  }

  while( nReturn == OK )
  {
    ReadOneLine(acReadBuffer,tFD);

    if ( acReadBuffer[0] == 0 ) //End of flie.
    {
      nReturn = ERROR;
    }

    if(strncmp(acReadBuffer,"END_EXT_DEF",strlen("END_EXT_DEF")) == 0)
    {
      WriteDebugLog(acReadBuffer);
      break;    //End of SWC defination.
    }

    if( acReadBuffer[0] == 'S' )  //Find a SWC.
    {
      WriteDebugLog(acReadBuffer);
      g_tRTUConfig.nSWCNumber ++;   //Increase SWC number.

      if ( g_tRTUConfig.nSWCNumber > SWC_MAX_NUMBER)
      {             //SWC number error.
        nReturn = ERROR;
      }

      strcpy(acTemp, acReadBuffer);

      if( GetValue(acTemp) == ERROR )
      {
        nReturn = ERROR;
      }
      else
      {
        if(GetDigit(acTemp, &nTemp) == ERROR)
        {
          nReturn = ERROR;
        }
      }

      //Get SWC key word.
      if( nReturn == OK)
      {
        pcEqual = strchr(acReadBuffer,'=');

        if(pcEqual == TYP_NULL)   //No '=', config file error.
        {
          nReturn = ERROR;
        }
        else
        {
          nValueStringPosition = pcEqual - acReadBuffer;
          acReadBuffer[nValueStringPosition] = '\0';  //Get "SWC_1"~"SWC_20".
        }
      }

      if( nReturn == OK)
      {
        for(nJ=0; nJ < SWC_MAX_NUMBER; nJ++)  //Search SWC index.
        {
          if( strcmp(acReadBuffer, g_acSWCKeyWordName[nJ]) == 0)
          {
            g_tRTUConfig.nSwcEnabled[nJ] = (char)nTemp;
            break;
          }
        }

        if(nJ >= SWC_MAX_NUMBER)  //Cannot find SWC index.
        {
          nReturn = ERROR;
        }
      }
    }
  }

  return nReturn;
}// GetSWCConfig

/**********************************************************************************
Purpose
  Get serial link parameters.

Input
  acBuffer    -- String for serial link parameters. (in)
  tPortParameter  -- The structure for serial port parameter. (out)

Reture
  OK    -- Get all parameter.
  ERROR  -- At least one parameter error or cannot find it.
************************************************************************************/
int GetSerialPortPara(char *acBuffer, tSerialPortPara *tPortParameter)
{
  int nTemp;  //040322 Yu Wei
  char acOneString[10];

  if(SearchMultiValue(acBuffer,acOneString,',') == false)
  {
    return ERROR;
  }
  else
  {
    if(GetDigit(acOneString, &nTemp) == OK)  //040322 Yu Wei
    {
      tPortParameter->nBaudRate = nTemp;
    }
    else
      return ERROR;

    if(SearchMultiValue(acBuffer,acOneString,',') == false)
      return ERROR;
    else
    {
      //tPortParameter->nStartBit = GetDigit(acOneString);  //Start bit number.
                                  //Start bit not used.
      if(SearchMultiValue(acBuffer,acOneString,',') == false)
        return ERROR;
      else
      {
        if(GetDigit(acOneString, &nTemp) == OK)  //040322 Yu Wei
        {
          tPortParameter->nDataBit = nTemp;  //Data bit number.
        }
        else
          return ERROR;
        if(SearchMultiValue(acBuffer,acOneString,',') == false)
          return ERROR;
        else
        {
          if(GetDigit(acOneString, &nTemp) == OK)  //040322 Yu Wei
          {
            tPortParameter->nStopBit = nTemp;  //Stop bit number.
          }
          else
            return ERROR;

          //Parity.
          SearchMultiValue(acBuffer,acOneString,',');    //040322 Yu Wei
          if (strlen(acOneString) != 1)
            return ERROR;

          switch(acOneString[0])
          {
          case 'E':
            tPortParameter->nParity = 2;
            break;
          case 'O':
            tPortParameter->nParity = 1;
            break;
          case 'N':
            tPortParameter->nParity = 0;
            break;
          default:
            return ERROR;
            //break;  //040604 Yu Wei
          }
        }
      }
    }
  }
  return OK;
}


/**********************************************************************************
Purpose
  Get one SWC parameter.
  Based on 19/03/04 config.txt to modify. //040319 Yu Wei

Input
  tFD    -- file descriptor.(in)
  pnSWCID  -- SWC ID. (out)

Return
  OK    -- Get all parameter.
  ERROR  -- At least one parameter error or cannot find it.
************************************************************************************/
int GetSWCParameter(FILE *tFD, int index)
{
  char acReadBuffer[INM_MAXSIZE_LINE+1];
  char acTempString[200];
  char cLog[1000];
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
  printf("[INM] GetSWCParameter, SWC %d\n", index);
  #endif // CFG_DEBUG_MSG


  //Start SWC configuration.
  if(SearchKeyWord(tFD,(CHAR *)"BGN_SWC_CFG",acReadBuffer) == false)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetSWCParameter, %s missing of BGN_SWC_CFG\n",
//          g_acSWCDeviceName[index]);
    sprintf(cLog,"ERR  [INM] GetSWCParameter, %s missing of BGN_SWC_CFG\n");
    g_pEventLog->LogMessage(cLog);
    return TYP_ERROR;
  }

  //SWC name is similiar to that of defined from config file
  if(GetStringValue(tFD, (CHAR *)"SWC_NAME", g_acSWCDeviceName[index])
     != E_ERR_Success)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetSWCParameter, %s missing of SWC_NAME\n",
//          g_acSWCDeviceName[index]);
    sprintf(cLog,"ERR  [INM] GetSWCParameter, %s missing of SWC_NAME\n");
    g_pEventLog->LogMessage(cLog);

    return TYP_ERROR;
  }
  //printf("----Device Name: %s\n", g_acSWCDeviceName[index]);
  strcpy(g_tRTUConfig.tSWC[index].tHeader.acName, g_acSWCDeviceName[index]);

  if(GetCharValue(tFD,(CHAR *)"SWC_ID",
                  (char *)&g_tRTUConfig.tSWC[index].tHeader.ucID) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetSWCParameter, %s missing of SWC_ID\n",
//          g_acSWCDeviceName[index]);
    sprintf(cLog,"ERR  [INM] GetSWCParameter, %s missing of SWC_ID\n");
    g_pEventLog->LogMessage(cLog);
    return TYP_ERROR;
  }

  if(GetStringValue(tFD, (CHAR *)"INTERFACE",acTempString) != E_ERR_Success)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetSWCParameter, %s missing of INTERFACE\n",
//          g_acSWCDeviceName[index]);
    sprintf(cLog,"ERR  [INM] GetSWCParameter, %s missing of INTERFACE\n",
            g_acSWCDeviceName[index]);
    g_pEventLog->LogMessage(cLog);
    return TYP_ERROR;
  }

  if(GetLinkType(acTempString, &g_tRTUConfig.tSWC[index].tHeader.nLinkType,
                 &g_tRTUConfig.tSWC[index].tHeader.nLinkGroup) == TYP_ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetSWCParameter, %s found invalid INTERFACE\n",
//          g_acSWCDeviceName[index]);
    sprintf(cLog,"ERR  [INM] GetSWCParameter, %s found invalid INTERFACE\n",
            g_acSWCDeviceName[index]);
    g_pEventLog->LogMessage(cLog);
    return TYP_ERROR;
  }

  //Get serial link 1,2 setting.
  if( GetLinkSetting (tFD, index) == ERROR)
    return TYP_ERROR;

  if (GetSWCPollingTimeInfo(tFD, index) != E_ERR_Success)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetSWCParameter, %s found polling time error\n",
//          g_acSWCDeviceName[index]);
    sprintf(cLog,"ERR  [INM] GetSWCParameter, %s found polling time error\n");
    g_pEventLog->LogMessage(cLog);
    return TYP_ERROR;
  }

  if(GetuShortValue(tFD, (CHAR *)"NOTATION",&g_tRTUConfig.tSWC[index].tHeader.nNotation) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetSWCParameter, %s missing of NOTATION\n",
//          g_acSWCDeviceName[index]);
    sprintf(cLog,"ERR  [INM] GetSWCParameter, %s missing of NOTATION\n",
            g_acSWCDeviceName[index]);
    g_pEventLog->LogMessage(cLog);
    return TYP_ERROR;
  }

  if(GetuShortValue(tFD, (CHAR *)"SOCKET_ID",&g_tRTUConfig.tSWC[index].tHeader.nSocketID) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetSWCParameter, %s missing of SOCKET_ID\n",
//          g_acSWCDeviceName[index]);
    sprintf(cLog,"ERR  [INM] GetSWCParameter, %s missing of SOCKET_ID\n",
            g_acSWCDeviceName[index]);
    g_pEventLog->LogMessage(cLog);
    return TYP_ERROR;
  }
  if(GetIntegerValue(tFD, (CHAR *)"TIMEOUT_COM_SERVER",&g_tRTUConfig.tSWC[index].tHeader.nLANLinkTimeout) == ERROR)
  {
//    pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetSWCParameter, %s missing of TIMEOUT_COM_SERVER\n",
//          g_acSWCDeviceName[index]);
    sprintf(cLog,"ERR  [INM] GetSWCParameter, %s missing of TIMEOUT_COM_SERVER\n",
            g_acSWCDeviceName[index]);
    g_pEventLog->LogMessage(cLog);

    return TYP_ERROR;
  }

  if(index != SWC_CLOCK_INDEX)    //Clock no SWC table.
  {
    //Get SWC table.
    if(SearchSWCTable(tFD,index) == ERROR)
      return ERROR;
  }

  return OK;

}

/*******************************************************************************
Purpose
  Get SWC polling time info.
  Added to decrease complexity in 19 March 2004, Yu Wei

Input
  tFD      -- file descriptor.(in)
  nSWCIndex  -- SWC index number.(in)

Return
  OK    -- Get all parameter.
  ERROR  -- At least one parameter error or cannot find it.

History
  Name         Date          Remark
  ----         ----          ------
  Bryan Chong  08-Oct-2009   Re-write the routine with switch case and defined
                             enumeration type for clock protocol
********************************************************************************/
E_ERR_T GetSWCPollingTimeInfo(FILE *tFD, int nSWCIndex)
{

  switch(g_unProtocolType)
  //switch(g_tRTUConfig.tSWC[nSWCIndex].tHeader.tProtocol[0]);
  {
    case E_CLKP_Other:
      break;

    case E_CLKP_Modbus:
      if(GetIntegerValue(tFD, (CHAR *)"POL_FAST_SWC",
          &g_tRTUConfig.tSWC[nSWCIndex].tHeader.nFastPollingFrequency) ==
          ERROR)
        return E_ERR_INM_InvalidModbusPollFastSWC;

      if(GetIntegerValue(tFD, (CHAR *)"POL_SLOW_SWC",
          &g_tRTUConfig.tSWC[nSWCIndex].tHeader.nSlowPollingFrequency) ==
          ERROR)
        return E_ERR_INM_InvalidModbusPollSlowSWC;

      if(GetIntegerValue(tFD, (CHAR *)"S_POLLING",
          &g_tRTUConfig.tSWC[nSWCIndex].tHeader.nStandbyPollingFrequency) ==
          ERROR)
        return E_ERR_INM_InvalidModbusStandbyPollingFreq;

      if(GetIntegerValue(tFD, (CHAR *)"F_POLLING",
          &g_tRTUConfig.tSWC[nSWCIndex].tHeader.nFailedPollingFrequency) ==
          ERROR)
        return E_ERR_INM_InvalidModbusFailedPollingFreq;

      //040707 Yu Wei
      if(GetIntegerValue(tFD, (CHAR *)"EX_N_RETRY",
          &g_tRTUConfig.tSWC[nSWCIndex].tHeader.nExcepRetry) ==
          ERROR)
        return E_ERR_INM_InvalidModbusExceptionRetry;


      break;

    case E_CLKP_Generic:
      if(GetIntegerValue(tFD, (CHAR *)"SPURIOUS_TIME",
           &g_tRTUConfig.tSWC[nSWCIndex].tHeader.nSpuriousTime) ==
           ERROR)
        return E_ERR_INM_InvalidGenericSpuriousTime;

      //040525 Yu Wei
      if(GetIntegerValue(tFD, (CHAR *)"GRACE_TIME",
           &g_tRTUConfig.nGreacTime) == ERROR)
        return E_ERR_INM_InvalidGraceTime;

      if(g_tRTUConfig.nGreacTime >= 1000)  //Max value is 999 ms.
        g_tRTUConfig.nGreacTime = 999;
      break;

    case E_CLKP_NTP:
      if(GetIntegerValue(tFD, (CHAR *)"NTP_SPURIOUS_TIME",
           &g_tRTUConfig.tSWC[nSWCIndex].tHeader.nSpuriousTime) ==
           ERROR)
        return E_ERR_INM_InvalidNTPSpuriousTime;
      if(GetIntegerValue(tFD, (CHAR *)"NTP_RETRY",
           &g_tRTUConfig.tSWC[nSWCIndex].tHeader.nRetryNumber) == ERROR)
        return E_ERR_INM_InvalidNTPRetry;

      if(GetIntegerValue(tFD, (CHAR *)"NTP_POLLING",
          &g_tRTUConfig.tSWC[nSWCIndex].tHeader.nStandbyPollingFrequency) ==
          ERROR)
        return E_ERR_INM_InvalidNTPPolling;
      break;

    default:
      #ifdef CFG_PRN_ERR
      printf("ERR  [INM] GetSWCPollingTimeInfo, undefined clock protocol\n");
      #endif // CFG_PRN_ERR
      return E_ERR_INM_UndefinedClockProtocol;
  }// switch(g_unProtocolType)

  switch(g_unProtocolType)
  {
    case E_CLKP_Other:
    case E_CLKP_Modbus:
    case E_CLKP_Generic:
      if(GetIntegerValue(tFD, (CHAR *)"TIME_OUT",
           &g_tRTUConfig.tSWC[nSWCIndex].tHeader.nReceiveTimeout) == ERROR)
        return E_ERR_INM_InvalidClkTimeOut;

      if(GetIntegerValue(tFD, (CHAR *)"N_RETRY",
           &g_tRTUConfig.tSWC[nSWCIndex].tHeader.nRetryNumber) == ERROR)
        return E_ERR_INM_InvalidClkRetry;
      break;
  } // switch(g_unProtocolType)

  return E_ERR_Success;
}

/*******************************************************************************
Purpose
  Acquire and verify SWC link setting info from config file

Input
  tFD      -- file descriptor.(in)
  nSWCIndex  -- SWC index number.(in)

Return
  OK     -- Get all parameter.
  ERROR  -- At least one parameter error or cannot find it.

History
  Name         Date          Remark
  ----         ----          ------
  Bryan Chong  27-Jul-2010   Update Link 2 HW_LNK_2 verification using
                             CFG_HWP_PCI_SERIAL_COM_START_NUM [PR69]
********************************************************************************/
int GetLinkSetting(FILE *tFD, int nSWCIndex)
{
  int nTemp;
  char acTempString[200];
  char cLog[1000];

  if (g_tRTUConfig.tSWC[nSWCIndex].tHeader.nLinkType == SWC_LINK_LAN)
  {
    unsigned short usTemp;
    if ( GetStringValue(tFD, (CHAR *)"SWC_IP_ADDR1",
         g_tRTUConfig.tSWC[nSWCIndex].tHeader.tLanLink[0].ip) != E_ERR_Success)
    {
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//        FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//        "ERR  [INM] GetLinkSetting, %s found invalid SWC_IP_ADDR1\n",
//        g_acSWCDeviceName[nSWCIndex]);
      sprintf(cLog,"ERR  [INM] GetLinkSetting, %s found invalid SWC_IP_ADDR1\n",
    	        g_acSWCDeviceName[nSWCIndex]);
      g_pEventLog->LogMessage(cLog);
      return TYP_ERROR;
    }
    if ( GetuShortValue(tFD, (CHAR *)"SWC_SOCKET_ID1",
         &g_tRTUConfig.tSWC[nSWCIndex].tHeader.tLanLink[0].port) == ERROR)
    {
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//        FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//        "ERR  [INM] GetLinkSetting, %s found invalid SWC_SOCKET_ID1\n",
//        g_acSWCDeviceName[nSWCIndex]);

      sprintf(cLog,"ERR  [INM] GetLinkSetting, %s found invalid SWC_SOCKET_ID1\n",
    	        g_acSWCDeviceName[nSWCIndex]);
      g_pEventLog->LogMessage(cLog);
      return TYP_ERROR;
    }

    if ( GetStringValue(tFD, (CHAR *)"SWC_IP_ADDR2",
         g_tRTUConfig.tSWC[nSWCIndex].tHeader.tLanLink[1].ip) != E_ERR_Success)
    {
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//        FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//        "ERR  [INM] GetLinkSetting, %s found invalid SWC_IP_ADDR2\n",
//        g_acSWCDeviceName[nSWCIndex]);

      sprintf(cLog,"ERR  [INM] GetLinkSetting, %s found invalid SWC_IP_ADDR2\n",
    	        g_acSWCDeviceName[nSWCIndex]);
      g_pEventLog->LogMessage(cLog);
      return TYP_ERROR;
    }
    if ( GetuShortValue(tFD, (CHAR *)"SWC_SOCKET_ID2",
         &g_tRTUConfig.tSWC[nSWCIndex].tHeader.tLanLink[1].port) == ERROR)
    {
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//        FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//        "ERR  [INM] GetLinkSetting, %s found invalid SWC_SOCKET_ID2\n",
//        g_acSWCDeviceName[nSWCIndex]);
      sprintf(cLog,"ERR  [INM] GetLinkSetting, %s found invalid SWC_SOCKET_ID2\n",
    	        g_acSWCDeviceName[nSWCIndex]);
      g_pEventLog->LogMessage(cLog);

      return TYP_ERROR;
    }

    if ( GetuShortValue(tFD, (CHAR *)"LAN_PING_CHECK", &usTemp) == ERROR)
    {
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//        FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//        "ERR  [INM] GetLinkSetting, %s found invalid LAN_PING_CHECK\n",
//        g_acSWCDeviceName[nSWCIndex]);
      sprintf(cLog,"ERR  [INM] GetLinkSetting, %s found invalid LAN_PING_CHECK\n",
    	        g_acSWCDeviceName[nSWCIndex]);
      g_pEventLog->LogMessage(cLog);
      return TYP_ERROR;
    }
    else
      g_tRTUConfig.tSWC[nSWCIndex].tHeader.bPingCheck = (usTemp==1);

    if ((!IsValidIP(g_tRTUConfig.tSWC[nSWCIndex].tHeader.tLanLink[0].ip)) &&
        (!IsValidIP(g_tRTUConfig.tSWC[nSWCIndex].tHeader.tLanLink[1].ip)))
    {
//      pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//        FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//        "ERR  [INM] GetLinkSetting, %s found invalid IP address\n",
//        g_acSWCDeviceName[nSWCIndex]);

      sprintf(cLog, "ERR  [INM] GetLinkSetting, %s found invalid IP address\n",
    	        g_acSWCDeviceName[nSWCIndex]);
      g_pEventLog->LogMessage(cLog);
      return TYP_ERROR;
    }

    if  (IsValidIP(g_tRTUConfig.tSWC[nSWCIndex].tHeader.tLanLink[0].ip))
    {
      if(GetStringValue(tFD, (CHAR *)"PR_LNK_1", acTempString) != E_ERR_Success)
      {
//        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetLinkSetting, %s found invalid PR_LNK_1\n",
//          g_acSWCDeviceName[nSWCIndex]);
        sprintf(cLog,"ERR  [INM] GetLinkSetting, %s found invalid PR_LNK_1\n",
                g_acSWCDeviceName[nSWCIndex]);
        g_pEventLog->LogMessage(cLog);

        return TYP_ERROR;
      }
      if(GetLinkProtocol(acTempString,
         &g_tRTUConfig.tSWC[nSWCIndex].tHeader.tProtocol[0]) == ERROR)
      {
//        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetLinkSetting, %s found invalid protocol definition at "
//          "PR_LNK1\n",
//          g_acSWCDeviceName[nSWCIndex]);
        sprintf(cLog, "ERR  [INM] GetLinkSetting, %s found invalid protocol definition at "
                "PR_LNK1\n",
                g_acSWCDeviceName[nSWCIndex]);
        g_pEventLog->LogMessage(cLog);

        return TYP_ERROR;
      }
    }
    if  (IsValidIP(g_tRTUConfig.tSWC[nSWCIndex].tHeader.tLanLink[1].ip))
    {
      if(GetStringValue(tFD, (CHAR *)"PR_LNK_2", acTempString) != E_ERR_Success)
      {
//        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetLinkSetting, %s found invalid PR_LNK_2\n",
//          g_acSWCDeviceName[nSWCIndex]);
        sprintf(cLog,"ERR  [INM] GetLinkSetting, %s found invalid PR_LNK_2\n",
                g_acSWCDeviceName[nSWCIndex]);
        g_pEventLog->LogMessage(cLog);
        return TYP_ERROR;
      }
      if(GetLinkProtocol(acTempString,
         &g_tRTUConfig.tSWC[nSWCIndex].tHeader.tProtocol[1]) == ERROR)
      {
//        pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//          FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//          "ERR  [INM] GetLinkSetting, %s found invalid protocol definition at "
//          "PR_LNK2\n",
//          g_acSWCDeviceName[nSWCIndex]);
        sprintf(cLog,"ERR  [INM] GetLinkSetting, %s found invalid protocol definition at "
                "PR_LNK2\n",
                g_acSWCDeviceName[nSWCIndex]);
        g_pEventLog->LogMessage(cLog);
        return TYP_ERROR;
      }
    }
    //g_pEventLog->LogMessage("SWC_IP_ADDR1 - PR_LNK_2 ok\n");
    return OK;
  }

  //Link 1 setting.
  if(GetIntegerValue(tFD, (CHAR *)"HW_LNK_1",&nTemp) == ERROR)
  {
//     pFLG_CtrlBlk->SendMessage(E_FLG_MSG_INM_ConfigFileErr,
//       FLG_HDR_T_NoTimeStamp, FLG_SEC_T_MilliSec, FLG_LOG_T_Event,
//       "ERR  [INM] GetLinkSetting, %s found invalid definition at "
//       "HW_LNK_1\n",
//       g_acSWCDeviceName[nSWCIndex]);
     sprintf(cLog,"ERR  [INM] GetLinkSetting, %s found invalid definition at "
    	       "HW_LNK_1\n",
    	       g_acSWCDeviceName[nSWCIndex]);
     g_pEventLog->LogMessage(cLog);

     return TYP_ERROR;
  }

  if( nTemp == 0)  //The link is not used.
  {
    g_tRTUConfig.tSWC[nSWCIndex].tHeader.tLinkPara[0].unPortID = 0xFFFF;
  }
  //else if( nTemp <= 8*g_tRTUConfig.nNumberOfSerialCOMCard +
    //       CFG_HWP_PCI_SERIAL_COM_START_NUM - 1)
  else if( nTemp <= pHWP_CB->uctotalSerialCOMPort)
  {
    g_tRTUConfig.tSWC[nSWCIndex].tHeader.tLinkPara[0].unPortID = nTemp-1;

    if(GetStringValue(tFD, (CHAR *)"PA_LNK_1",acTempString) != E_ERR_Success)
      return ERROR;

    if(GetSerialPortPara(acTempString,
       &g_tRTUConfig.tSWC[nSWCIndex].tHeader.tLinkPara[0]) == ERROR)
      return ERROR;

    if(GetStringValue(tFD, (CHAR *)"PR_LNK_1", acTempString) != E_ERR_Success)
      return ERROR;


    if(GetLinkProtocol(acTempString,
       &g_tRTUConfig.tSWC[nSWCIndex].tHeader.tProtocol[0]) == ERROR)
      return ERROR;
  }
  else  // Link number is invalid.
  {
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
    printf("[INM] GetLinkSetting, HW_LNK_1=%d exceeds max number %d\n",
           nTemp, pHWP_CB->uctotalSerialCOMPort);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
    return ERROR;
  }

  //Link 2 setting.
  if(GetIntegerValue(tFD, (CHAR *)"HW_LNK_2",&nTemp) == ERROR)
    return ERROR;

  if( nTemp == 0)
  {
    g_tRTUConfig.tSWC[nSWCIndex].tHeader.tLinkPara[1].unPortID = 0xFFFF;
  }
  // 20100727 BC (Rqd by ZSL)
  else if( nTemp <= pHWP_CB->uctotalSerialCOMPort)
  {
    g_tRTUConfig.tSWC[nSWCIndex].tHeader.tLinkPara[1].unPortID = nTemp-1;

    if(GetStringValue(tFD, (CHAR *)"PA_LNK_2",acTempString) != E_ERR_Success)
      return ERROR;

    if(GetSerialPortPara(acTempString,
       &g_tRTUConfig.tSWC[nSWCIndex].tHeader.tLinkPara[1]) == ERROR)
      return ERROR;

    if(GetStringValue(tFD, (CHAR *)"PR_LNK_2", acTempString) != E_ERR_Success)
      return ERROR;
    if(GetLinkProtocol(acTempString,
       &g_tRTUConfig.tSWC[nSWCIndex].tHeader.tProtocol[1]) == ERROR)
      return ERROR;
  }
  else
  {
    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
    printf("[INM] GetLinkSetting, HW_LNK_2=%d exceeds max number %d\n",
           nTemp, pHWP_CB->uctotalSerialCOMPort);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
    return ERROR;
  }

  return OK;
}


/**********************************************************************************
Purpose
  Get one SWC Table and Server table. Routine name change to GetSWCNSvrTable().
  Re-design in 19 March 2004, Yu Wei
  The fast polling is table 100 or 200.
  The slow polling is table 300.
  The timer polling is table 400.
  A SWC can only one fast polling, one slow polling and one timer polling.
  When there are more than two tables, it means the config file error.

Input
  tFD      -- file descriptor.(in)
  nSWCIndex  -- SWC index number.(in)
  nTableID  -- Table ID. (in)

Return
  OK    -- Get all parameter.
  ERROR  -- At least one parameter error or cannot find it.
************************************************************************************/
int GetSWCNSvrTable(FILE *tFD, int nSWCIndex)
{
  int    nTableID;
  unsigned short unStartAddress, unEndAddress, nTableLen;
  unsigned short unSrvStartAddress, unSrvEndAddress;

  if(GetIntegerValue(tFD, (CHAR *)"SWC_TABLE_ID",&nTableID) == ERROR)
    return ERROR;

  switch(nTableID)
  {
  case 101:    //Output table not used for SWC interface.
  case 201:
  case 301:
    return OK;
  }

  if(GetuShortValue(tFD, (CHAR *)"SWC_TABLE_BGN",&unStartAddress) == ERROR)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetSWCNSvrTable, get SWC_TABLE_BGN address error\n");
    #endif // CFG_PRN_ERR
    return ERROR;
  }

  if(GetuShortValue(tFD, (CHAR *)"SWC_TABLE_END",&unEndAddress) == ERROR)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetSWCNSvrTable, get SWC_TABLE_END address error\n");
    #endif // CFG_PRN_ERR
    return ERROR;
  }

  if( unEndAddress < unStartAddress )
  {  //Table range error
    g_pEventLog->LogMessage((CHAR *)" *** SWC table End Address < Start "
                            "Address ***\n");
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetSWCNSvrTable, end address greater than start "
           "address\n");
    #endif // CFG_PRN_ERR
    return ERROR;
  }
  else if (unEndAddress == 0) nTableLen = 0;
  else
    nTableLen = unEndAddress - unStartAddress + 1;

  if (GetSWCTable(tFD, nSWCIndex, nTableID, unStartAddress, nTableLen)== ERROR)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetSWCNSvrTable, GetSWCTable error\n");
    #endif // CFG_PRN_ERR
    return ERROR;
  }

  //Table 800 and 900 have no server table
  if(( nTableID == 800) || ( nTableID == 900))
  {
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
    printf("[INM] GetSWCNSvrTable, No SVR_TABLE available for table 800 or "
           "900\n");
    #endif // CFG_PRN_ERR
    return OK;
  }

  //Get SWC table address in RTU polling table.
  if(GetuShortValue(tFD, (CHAR *)"SVR_TABLE_BGN",&unSrvStartAddress) == ERROR)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetSWCNSvrTable, get SVR_TABLE_BGN address error\n");
    #endif // CFG_PRN_ERR
    return ERROR;
  }

  if(GetuShortValue(tFD, (CHAR *)"SVR_TABLE_END",&unSrvEndAddress) == ERROR)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetSWCNSvrTable, get SVR_TABLE_END address error\n");
    #endif // CFG_PRN_ERR
    return ERROR;
  }

  if( unSrvEndAddress < unSrvStartAddress )
  {  //Table range error
    g_pEventLog->LogMessage((CHAR *)" *** SWC Polling table End Address < \
Start Address ***\n");
   #ifdef CFG_PRN_ERR
   printf("ERR  [INM] GetSWCNSvrTable, end address less than start address\n");
   #endif // CFG_PRN_ERR
    return ERROR;
  }


  unSrvEndAddress = unSrvEndAddress - unSrvStartAddress + 1;

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  printf("[INM] GetSWCNSvrTable, unSrvEndAddress = %d, "
         "unSrvStartAddress = %d\n",
         unSrvEndAddress, unSrvStartAddress);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))

  if (nTableLen == 0) unSrvEndAddress = 0;

  // server polling has 3 words time stamp.
  if ( nTableLen > 0 && unSrvEndAddress != (nTableLen + 3))
  {
    g_pEventLog->LogMessage((CHAR *)" *** SWC polling table size != SWC table \
size + 3 ***\n");
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetSWCNSvrTable, invalid table size. nTableLen = %d, "
           "unSrvEndAddress = %d, Expect SVR_TABLE_END = %d\n",
      nTableLen, unSrvEndAddress, (unSrvStartAddress + nTableLen + 2));
    #endif // CFG_PRN_ERR
    return ERROR;
  }

  switch(nTableID)
  {
  case 100:
  case 200:
    g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tFastPollingAddress.
       unServerStart = unSrvStartAddress;
    g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tFastPollingAddress.
       unServerLength = unSrvEndAddress;
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
    printf("[INM] GetSWCNSvrTable, fast polling unServerStart = %d, "
           "unServerLength = %d\n",
           g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tFastPollingAddress.
             unServerStart,
           g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tFastPollingAddress.
             unServerLength);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
    break;

  case 300:
    g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tSlowPollingAddress.
      unServerStart = unSrvStartAddress;
    g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tSlowPollingAddress.
      unServerLength = unSrvEndAddress;
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
    printf("[INM] GetSWCNSvrTable, slow polling unServerStart = %d, "
           "unServerLength = %d\n",
           g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tSlowPollingAddress.
             unServerStart,
           g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tSlowPollingAddress.
             unServerLength);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
    break;

  case 400:
    g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tTimerAddress.unServerStart =
      unSrvStartAddress;
    g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tTimerAddress.unServerLength =
      unSrvEndAddress;
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
    printf("[INM] GetSWCNSvrTable, timer polling unServerStart = %d, "
           "unServerLength = %d\n",
           g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tTimerAddress.
             unServerStart,
           g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tTimerAddress.
             unServerLength);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
    break;

  default:
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetSWCNSvrTable, invalid table ID, %d\n", nTableID);
    #endif // CFG_PRN_ERR
    return ERROR;
  }

  if ( AllocateSWCPollingTable (unSrvStartAddress, unSrvEndAddress) == ERROR)
  {
    g_pEventLog->LogMessage(
      (CHAR *)" *** SWC polling table overlap or out of range ***\n");
    #ifdef CFG_PRN_ERR
    printf("ERR  [INM] GetSWCNSvrTable, SWC polling table overlap or out of "
           "range\n");
    #endif // CFG_PRN_ERR
    printf("Start Addr:%d, end:%d\n", unSrvStartAddress, unSrvEndAddress);
    return ERROR;
  }

  return OK;
}

/*******************************************************************************
Purpose
  Get all SWC table info.
  Get hour and minute for time synchronization.
  Get timer polling interval.
  New design to decrease complexity.
  Added in 22 March 2004, Yu Wei

Input
  tFD        -- file descriptor.(in)
  nSWCIndex    -- SWC index number.(in)
  nTableID    -- Table ID.(in)
  unStartAddress  -- SWC table start address. (in)
  nTableLen    -- SWC table length. (in)

Return
  OK    -- Get all parameter.
  ERROR  -- Table address error.
        At least one parameter error or cannot find it.
History
  Name          Date        Remark
  ----          ----        ------
 Bryan Chong  26-Feb-2010  Update to conver timer interval into milliseconds

********************************************************************************/
int GetSWCTable(FILE *tFD,
        int nSWCIndex,
        int nTableID,
        unsigned short unStartAddress,
        unsigned short nTableLen)
{
  switch(nTableID)
  {
  case 800:  //Table 800 for time synchronization.
    g_tRTUConfig.tSWC[nSWCIndex].tHeader.unSendTimeSynStartAddr = unStartAddress;
    g_tRTUConfig.tSWC[nSWCIndex].tHeader.unSendTimeSynLength = nTableLen;

    if(GetIntegerValue(tFD, (CHAR *)"SEND_TIME_HOUR",&g_tRTUConfig.tSWC[nSWCIndex].tHeader.nSendTimeSynHour) == ERROR)
      return ERROR;

    if(GetIntegerValue(tFD, (CHAR *)"SEND_TIME_MIN",&g_tRTUConfig.tSWC[nSWCIndex].tHeader.nSendTimeSynMinute) == ERROR)
      return ERROR;

    if( ( g_tRTUConfig.tSWC[nSWCIndex].tHeader.nSendTimeSynHour >= 24) ||    //Invalid time.
      ( g_tRTUConfig.tSWC[nSWCIndex].tHeader.nSendTimeSynMinute >= 60))
      return ERROR;    //When hour>=24 || minute>=60, the parameters error.

    //040707 Yu Wei
    if(GetIntegerValue(tFD, (CHAR *)"SLAVE_BUSY_TIMEOUT",&g_tRTUConfig.tSWC[nSWCIndex].tHeader.nSWCBusyTimeout) == ERROR)
      return ERROR;

    if(GetIntegerValue(tFD, (CHAR *)"EX_TIME_SYNC_N_RETRY",&g_tRTUConfig.tSWC[nSWCIndex].tHeader.nExcepTimeSyncRetry) == ERROR)
      return ERROR;
    //040707 Yu Wei

    return OK;

  case 100:
  case 200:
    if ( g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tFastPollingAddress.unLength != 0)
    {    //Fast polling table exist.
      WriteDebugLog((CHAR *)" *** Fast polling has existed ***\n");
      return ERROR;
    }

    g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tFastPollingAddress.unStart = unStartAddress;
    g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tFastPollingAddress.unLength = nTableLen;

    break;

  case 300:
    if (g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tSlowPollingAddress.unLength != 0)
    {  //Slow polling table exist.
      WriteDebugLog((CHAR *)" *** Slow polling has existed ***\n");
      return ERROR;
    }

    g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tSlowPollingAddress.unStart = unStartAddress;
    g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tSlowPollingAddress.unLength = nTableLen;

    break;

  case 400:
    if (g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tTimerAddress.unLength != 0)
    {  //Timer polling table exist.
      WriteDebugLog((CHAR *)" *** Timer polling has existed ***\n");
      return ERROR;
    }

    g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tTimerAddress.unStart = unStartAddress;
    g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tTimerAddress.unLength = nTableLen;

    if(GetIntegerValue(tFD, (CHAR *)"TIMER_INTERVAL",&g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.nTimerInterval) == ERROR)
      return ERROR;

    // convert timer interval to millisecond
    g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.nTimerInterval *= (60 * 1000);

    break;

  case 900:  //Table 900 for sending KeepAlive signal.  //040812 Yu Wei
    g_tRTUConfig.tSWC[nSWCIndex].tHeader.unSendKeepAliveAddr = unStartAddress;

    if(GetIntegerValue(tFD, (CHAR *)"KEEP_ALIVE_SWC",
        &g_tRTUConfig.tSWC[nSWCIndex].tHeader.nKeepAliveInterval) == ERROR)
      return ERROR;
    break;

  default:
    return ERROR;
  }

  return OK;
}


/**********************************************************************************
Purpose
  Get all SWC table info and polling table info.
  //Change routine name to SearchSWCTable(). 040322 Yu Wei

Input
  tFD      -- file descriptor.(in)
  nSWCIndex  -- SWC index number.(in)

Return
  OK    -- Get all parameter.
  ERROR  -- At least one parameter error or cannot find it.
************************************************************************************/
int SearchSWCTable(FILE *tFD, int nSWCIndex)
{
  int nMaxTry = 1000;
  char acReadBuffer[INM_MAXSIZE_LINE+1];

  //Reset polling table length. The table length !=0 means the table exist.  //040322 Yu Wei
  g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tFastPollingAddress.unLength = 0;
  g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tSlowPollingAddress.unLength = 0;
  g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tTimerAddress.unLength = 0;
  g_tRTUConfig.tSWC[nSWCIndex].tHeader.nSendTimeSynHour = 99;
  g_tRTUConfig.tSWC[nSWCIndex].tHeader.nSendTimeSynMinute = 99;

  g_tRTUConfig.tSWC[nSWCIndex].tHeader.nKeepAliveInterval = 0;  //Default no keep alive command //040812 Yu Wei

  while(nMaxTry--)
  {
    ReadOneLine(acReadBuffer,tFD);

    if(strncmp(acReadBuffer,"BGN_SWC_TABLE",strlen("BGN_SWC_TABLE")) == 0)
    {
      WriteDebugLog(acReadBuffer);  //040324 Yu Wei
      if(GetSWCNSvrTable(tFD,nSWCIndex) == ERROR)  //040324 Yu Wei
        return ERROR;
    }
    else if(strncmp(acReadBuffer,"END_SWC_CFG",strlen("END_SWC_CFG")) == 0)  //040324 Yu Wei
    {
      WriteDebugLog(acReadBuffer);  //040324 Yu Wei
      if(g_tRTUConfig.tSWC[nSWCIndex].tPollingAddress.tFastPollingAddress.unLength == 0)
      {      //No Fast polling table defined. 040324 Yu Wei.
        WriteDebugLog((CHAR *)" *** The fast polling table (ID = 100 or ID = 200) isn't defined ***\n");
        LogInitError((CHAR *)" *** The fast polling table (ID = 100 or ID = 200) isn't defined ***\n");
        return ERROR;
      }
      else
        return OK;
    }
  }
  return ERROR;
}

/**********************************************************************************
Purpose
  Allocate RTU status table and polling table in the map.
  It also check if the table overlap.
  Added in 18 March 2004, Yu Wei

Input
  unStartAddress  -- Polling table start address.(in)
  unLength    -- Polling table length.(in)

Return
  OK    -- Allocted successfully.
  ERROR  -- Table overlap.
************************************************************************************/
int AllocateServerTable(unsigned short unStartAddress, unsigned short unLength)
{
  int nI;
  int nReturn = OK;

  memset(&g_acServerTableMap[unStartAddress], 0x00, unLength);  //allocate polling table.

  for ( nI= g_tRTUConfig.tRTUStatusTable.unTableStartAddress;
      nI <= g_tRTUConfig.tRTUStatusTable.unTableEndAddress; nI++)
  {    //Check table overlap.
    if ( g_acServerTableMap[nI] != 0xFF)
    {  //Overlap.
      nReturn = ERROR;
      break;
    }
  }

  //allocate RTU status table.
  memset(&g_acServerTableMap[g_tRTUConfig.tRTUStatusTable.unTableStartAddress],
      0x01,
      (g_tRTUConfig.tRTUStatusTable.unTableEndAddress -
      g_tRTUConfig.tRTUStatusTable.unTableStartAddress +1 ));

  return nReturn;
}

/**********************************************************************************
Purpose
  Check if SWC polling table overlap and allocate SWC polling in the map.
  Added in 18 March 2004, Yu Wei

Input
  unStartAddress  -- SWC polling table start address.(in)
  unLength    -- SWC polling table length.(in)

Return
  OK    -- Allocted successfully.
  ERROR  -- Table overlap.
************************************************************************************/
int AllocateSWCPollingTable(unsigned short unStartAddress, unsigned short unLength)
{
  int nReturn = OK;
  int nI;

  if ( unLength != 0)
  {
    for ( nI= unStartAddress; nI < unLength + unStartAddress; nI++)
    {    //Check table overlap.
      if ( g_acServerTableMap[nI] != 0x00)
      {  //Overlap.
        nReturn = ERROR;
        break;
      }
    }
    //Allocate map.
    memset(&g_acServerTableMap[unStartAddress], 0x01, unLength);
  }

  return nReturn;
}

/**********************************************************************************
Purpose
  Check if SWC serial port ID conficts.
  Added in 25 March 2004, Yu Wei

Input

Return
  OK    -- No conflict.
  ERROR  -- At least one port conflict.
************************************************************************************/
int SerialPortIDCheck(void)
{
  char acTemp[256];
  int nI, nJ, nIIndex;//,nJIndex; //20090814
  unsigned short unPortIDI1, unPortIDI2, unPortIDJ1, unPortIDJ2;

  for ( nI=0; nI<SWC_MAX_NUMBER; nI++)
  {

    if(g_tRTUConfig.nSwcEnabled[nI] != 1) continue;
    nIIndex = nI;
    if (g_tRTUConfig.tSWC[nIIndex].tHeader.nLinkType == SWC_LINK_LAN) continue; //20090629

    unPortIDI1 = g_tRTUConfig.tSWC[nIIndex].tHeader.tLinkPara[0].unPortID;
    unPortIDI2 = g_tRTUConfig.tSWC[nIIndex].tHeader.tLinkPara[1].unPortID;

    if(unPortIDI1 == unPortIDI2)  //Link 1 and 2 conflict.
    {
      printf(" ***%d: %s Link1 and Link2 use same serial port %x ***\n", nIIndex, g_acSWCDeviceName[nIIndex], unPortIDI2);
      sprintf(acTemp," *** %s Link1 and Link2 use same serial port ***\n",g_acSWCDeviceName[nIIndex]);
      WriteDebugLog(acTemp);
      return ERROR;
    }
    for( nJ=nI+1; nJ<SWC_MAX_NUMBER; nJ++)
    {//20090814 rewrite this portion

      if (g_tRTUConfig.nSwcEnabled[nJ] != 1) continue;
      if (g_tRTUConfig.tSWC[nJ].tHeader.nLinkType == SWC_LINK_LAN) continue;

      unPortIDJ1 = g_tRTUConfig.tSWC[nJ].tHeader.tLinkPara[0].unPortID;
      unPortIDJ2 = g_tRTUConfig.tSWC[nJ].tHeader.tLinkPara[1].unPortID;

      if(unPortIDJ1 == unPortIDJ2)  //Link 1 and 2 conflict.
      {
        sprintf(acTemp," *** %s Link1 and Link2 use same serial port ***\n",
                g_acSWCDeviceName[nJ]);
        WriteDebugLog(acTemp);
        return ERROR;
      }

      if( ( unPortIDI1 == unPortIDJ1 ) && ( unPortIDJ1 != 0xFFFF))
      {
        sprintf(acTemp," *** %s Link1 and  %s Link1 use same serial port ***\n",
          g_acSWCDeviceName[nIIndex],g_acSWCDeviceName[nJ]);
        WriteDebugLog(acTemp);
        return ERROR;
      }
      if( ( unPortIDI1 == unPortIDJ2 ) && ( unPortIDJ2 != 0xFFFF))
      {
        sprintf(acTemp," *** %s Link1 and  %s Link2 use same serial port ***\n",
          g_acSWCDeviceName[nIIndex],g_acSWCDeviceName[nJ]);
        WriteDebugLog(acTemp);
        return ERROR;
      }
      if( ( unPortIDI2 == unPortIDJ1 ) && ( unPortIDJ1 != 0xFFFF))
      {
        sprintf(acTemp," *** %s Link2 and  %s Link1 use same serial port ***\n",
          g_acSWCDeviceName[nIIndex],g_acSWCDeviceName[nJ]);
        WriteDebugLog(acTemp);
        return ERROR;
      }
      if( ( unPortIDI2 == unPortIDJ2 ) && ( unPortIDJ2 != 0xFFFF))
      {
        sprintf(acTemp," *** %s Link2 and  %s Link2 use same serial port ***\n",
          g_acSWCDeviceName[nIIndex],g_acSWCDeviceName[nJ]);
        WriteDebugLog(acTemp);
        return ERROR;
      }
    }
  }

  return OK;
}// SerialPortIDCheck
/**********************************************************************************
Purpose:
  Check if IP address valid

Input:
  ip    -- IP address string to be check

Return:
  true  -- valid IP
  false  -- wrong IP

***********************************************************************************/
static bool IsValidIP(char *ip)
{
  //printf(ip);
  int i1, i2, i3, i4;
  int n = strlen(ip);
  if (n<1 || n>15) return false;

  if ((i1=atoi(ip)) > 255) return false;

  char *p = strstr(ip, ".");
  if (p != NULL && strlen(p) > 1) p++;
  else return false;
  if ((i2=atoi(p)) > 255) return false;

  p = strstr(p, ".");
  if (p != NULL && strlen(p) > 1) p++;
  else return false;
  if ((i3=atoi(p)) > 255) return false;

  p = strstr(p, ".");
  if (p != NULL && strlen(p) > 1) p++;
  else return false;
  if ((i4=atoi(p)) > 255) return false;

  sprintf(ip, "%d.%d.%d.%d", i1, i2, i3, i4);
  //printf(ip);
  return true;
}// IsValidIP
