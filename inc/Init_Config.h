/************************************************************
*                              *
* Project:    C830 RTU                  *
* Module:   Initialization                *
* File :      Init_Config.h               *
* Author:   Yu Wei                    *
*                              *
* Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************

**************************************************************

DESCRIPTION

  This is header file for Init_Config.cpp routine defination.

*************************************************************
Modiffication History
---------------------

Version: INM D.1.1.1
 01 1 April 2003, Yu Wei,
  Start to write.

Version: INM D.1.3.2
 01 01 Ocrober 2003, Yu Wei
  Modify GetVDUName() and GetVDUPassword().

 02 02 October 2003, Yu Wei
  Modify GetSWCTable();

Version: INM D.1.3.5
 01 16~25 March 2004, Yu Wei
  Added boundary check for configuration value.
    Refer to PR:OASYS-PMS 0047.
    Reduce complexity.
    Refer to PR:OASYS-PMS 0044.
    - Change routines name:
    1. GetVersion() to GetCFGVersion().
    2. GetSWCTable() to GetSWCNSvrTable().
    3. SearchSWCTable_PollingAddress() to SearchSWCTable().
    - Modified routine:
    1. GetDigit().
    2. GetLinkProtocol().
    - Added routines:
    1. WriteDebugLog().
    2. DeleteBlank().
    3. GetHexDigit().
    4. GetDecimalDigit().
    5. GetIPAddrValue().
    6. GetDebugInfo().
    7. GetLink1Info().
    8. GetLink2Info().
    9. GetSWCPollingTimeInfo().
    10.GetLinkSetting().
    11.GetSWCTable().
    12.AllocateServerTable().
    13.AllocateSWCPollingTable().
    14.SerialPortIDCheck().
    - Removed routine GetSWCPollingAddress().

  02  29 March 2004, Yu Wei
    Config.txt file added LOCAL_TIME_ZONE for UTC time convert to local time.
    Added GetLocalTimeZone()and modified GetRTUConfig().
    Refer to PR:OASYS-PMS 0151.
**************************************************************/
#ifndef  _INIT_CONFIG_H
#define  _INIT_CONFIG_H

//The max number of characters per line in config.txt file
#define INM_MAXSIZE_LINE   511
#define INM_RTU_STATUS_SZ  32

// Max polling rate on LAN link between RTUs in milliseconds
#define INM_MAX_RTU_TO_RTU_LAN_POLLING_TIME 10000

// Max timeout before declaring LAN failure
#define INM_MAX_RTU_LAN_POLLING_TIMEOUT 30000

// Max retry in case of LAN link failure
#define INM_MAX_RTU_LAN_POLLING_RETRY  20
E_ERR_T GetSystemConfig(char *);
void ReadOneLine(char *, FILE *);

void WriteDebugLog(char *);  //040318 Yu Wei

int GetValue(char *);
void DeleteBlank(char *);    //040316 Yu Wei
E_ERR_T GetStringValue(FILE *tdf, char *acKeyWord, char *acOutString);
extern E_ERR_T GetStringValue(FILE *tdf, char *acKeyWord, char *acOutString);
int GetDigit(char *acBuffer, int *pnValue);
int GetHexDigit(char *, int *);      //040316 Yu Wei
int GetDecimalDigit(char *, int *);    //040316 Yu Wei

int GetIntegerValue(FILE *tdf, char *acKeyWord, int *nOutValue);
int GetuShortValue(FILE *tdf, char *acKeyWord, unsigned short *pnOutValue);
int GetCharValue(FILE *tdf, char *acKeyWord, char *pnOutValue);
bool SearchKeyWord(FILE *tdf, char *acKeyWord, char *acBuffer);
extern bool SearchKeyWord(FILE *tdf, char *acKeyWord, char *acBuffer);
int GetIPAddrValue(FILE *tFD, char *acKeyWord, char *pnOutValue);  //040317 Yu Wei

E_ERR_T GetDebugInfo(FILE *);  //040316 Yu Wei

E_ERR_T GetCFGVersion(FILE *);  //040317 Yu Wei

int GetOtherRTUIPAddress(char *acLocalIP, char *acOtherIP, int nRTUID);
int GetRTUConfig(FILE *tdf);

int GetLink1Info(FILE *);  //040322 Yu Wei
int GetLink2Info(FILE *);  //040322 Yu Wei

int GetLocalTimeZone(FILE *, int *);  //040329 Yu Wei

bool SearchMultiValue(char *acMutliValue, char *acOneValue, char cDivideCharacter);
int GetVDUName(char *acbuffer);
int GetVDUPassword(char *acbuffer);
int GetLinkProtocol(char *acBuffer, struct link_protocol* pPrt);
int GetLinkType(char *acBuffer, int *pnLinkType, int *pnGroup);
int GetAllRTUTable(FILE *tdf);
int GetRTUTable(FILE *tdf, tRTUTableStructure *tRTUTable);
int GetServerDef(FILE *tdf);
int GetAllSWCParameter(FILE *tdf);
int GetSWCConfig(FILE *tdf);

int GetSerialPortPara(char *acbuffer, tSerialPortPara *tPortParameter);
int GetSWCParameter(FILE *tFD, int index);
int CheckSwcName(void);



E_ERR_T GetSWCPollingTimeInfo(FILE *tFD, int nSWCIndex); //040319 Yu Wei
int GetLinkSetting(FILE *tFD, int nSWCIndex); //040322 Yu Wei

int GetSWCNSvrTable(FILE *tdf, int nSWCIndex);  //040319 Yu Wei
int GetSWCTable(FILE *, int, int, unsigned short, unsigned short); //040322 Yu Wei

int SearchSWCTable(FILE *tdf, int nSWCIndex);

int AllocateServerTable(unsigned short, unsigned short); //040318 Yu Wei
int AllocateSWCPollingTable(unsigned short, unsigned short);  //040318 Yu Wei

int SerialPortIDCheck(void);  //040325 Yu Wei

#endif /* _INIT_CONFIG_H */

