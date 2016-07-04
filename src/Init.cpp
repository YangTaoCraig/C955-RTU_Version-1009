/************************************************************
*                              *
*  Project:    C830 RTU                  *
*  Module:    Initialization (INM)            *
*  File :      Init.cpp                  *
*  Author:    Yu Wei                    *
*                              *
*  Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************

**************************************************************

DESCRIPTION

  This file is main initialization function.

*************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/neutrino.h>
#include <errno.h>
#include <string.h>
#include <mqueue.h>
#include <termios.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "err_ext.h"
#include "sys_ass.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
#include "hwp_def.h"
#include "hwp_ext.h"
#include "lnc_def.h"
#include "lnc_ext.h"
#include "ser_def.h"

#include "Init.h"
#include "Common.h"
#include "Init_Config.h"
#include "CMM_Log.h"
#include "CMM_Timer.h"
#include "CMM_Listen.h"
#include "Init_SWC.h"
#include "RMM.h"
#include "ServerPoll.h"
#include "MMM.h"
#include "CMM_Watchdog.h"
#include "CMM_Timer.h"    //040520 Yu Wei
#include "MMM_SER1.h"
#include "RMM_RTULink.h"



//#define ModuleTest

/*******************************************************************************
Purpose:
  Initialize ethernet port and route.
  Initialize RTU states: g_tRTUStatus.


Return
  E_ERR_Success                               the routine executed successfully
   E_ERR_LNC_AddGatewayFailToCreateSocket     fail to create socket
   E_ERR_LNC_AddGatewayUpdateRouteTableFail   fail to update routing table

History

      Name         Date       Remark
      ----         ----       ------
 Bryan Chong   17-May-2010  Update to have different return values when
                            different errors occur
 Bryan Chong   19-May-2010  Replace adding gateway code with LNC_AddGateway
                            routine
 Bryan Chong   17-Nov-2011  Add LAN 4 port to resolve loopback issue.
                            [C955,PR111]

*******************************************************************************/
E_ERR_T LANInit(void)
{
  E_ERR_T rstatus = E_ERR_Success;
  int iSocket;
  int status = OK;
  int nI;
  int len;
  struct ifreq ifreq;
  struct sockaddr_in *sa_in;
  char *ip = NULL ;
  char buffer[50]={0};


  for(nI=0; nI<CFG_LNC_TOTAL_ACTIVE_PORT; nI++)
  {
    #ifdef CFG_PRIMARY_LAN_PORT_4
    if(nI == 3)
    {
      strcpy(g_tRTUConfig.tLANPara[nI].acLAN_IP,
             g_tRTUConfig.tLANPara[0].acLAN_IP);
      strcpy(g_tRTUConfig.tLANPara[nI].acLAN_Netmask,
             g_tRTUConfig.tLANPara[0].acLAN_Netmask);
      strcpy(g_tRTUConfig.tLANPara[nI].acLAN_Gateway,
             g_tRTUConfig.tLANPara[0].acLAN_Gateway);
    }
    #endif // CFG_PRIMARY_LAN_PORT_4

    /* set ip addr and netmask of 1st ethernet port */
    status = OK;
    if ((iSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
      #ifdef CFG_PRN_ERR
      perror("ERR  [INM] LANInit, open socket for set en0 IP");
      #endif // CFG_PRN_ERR

      // 20100517 BC (Rqd by ZSL)
      return E_ERR_LNC_FailToCreateSocket;
    }
    ip = (char *)malloc(20);
    memset(ip , 0 ,20);

    #ifdef CFG_PRIMARY_LAN_PORT_1
    len = strlen(g_tRTUConfig.tLANPara[nI].acLAN_IP);
    strcpy(ip, g_tRTUConfig.tLANPara[nI].acLAN_IP);
    #endif // CFG_PRIMARY_LAN_PORT_1

    #ifdef CFG_PRIMARY_LAN_PORT_4
    if(nI == 0)
    {
      strcpy(ip, CFG_DEFAULT_LAN_PORT_1_IP_ADDR);
    }else{
      len = strlen(g_tRTUConfig.tLANPara[nI].acLAN_IP);
      strcpy(ip, g_tRTUConfig.tLANPara[nI].acLAN_IP);
    }
    #endif // CFG_PRIMARY_LAN_PORT_4

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
    printf("[INM] LANInit, Local port %d, IP %s, fd %d\n",
            (nI + 1), ip, iSocket);
    #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)

    memset(&ifreq, 0, sizeof(ifreq));

    sprintf(buffer , "en%d" , nI);
    strncpy(ifreq.ifr_name, buffer, strlen("en0"));

    sa_in = (struct sockaddr_in *)&ifreq.ifr_addr;
    sa_in->sin_family = AF_INET;
    sa_in->sin_len = sizeof(*sa_in);
    inet_aton(ip, & sa_in->sin_addr);

    if (ioctl(iSocket, SIOCSIFADDR, &ifreq) == -1)
    {
      #ifdef CFG_PRN_ERR
      printf("ERR  [INM] LANInit, invalid ioctl. Buffer: %s\n" , buffer);
      perror("ERR  [INM] LANInit, set ip address");
      #endif // CFG_PRN_ERR

      // 20100517 BC (Rqd by ZSL)
      rstatus = E_ERR_LNC_InvalidIoctlForIPAddr;
    }

    //netmask
    len = strlen(g_tRTUConfig.tLANPara[nI].acLAN_Netmask);
    memset(ip , 0 , 20);
    memcpy( ip ,g_tRTUConfig.tLANPara[nI].acLAN_Netmask , len );

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
    printf("[INM] LANInit, Local subnet mask %d: %s\n", (nI + 1), ip);
    #endif // CFG_DEBUG_MSG

    memset(&ifreq, 0x00, sizeof(ifreq));

    sprintf(buffer , "en%d" , nI);
    strncpy(ifreq.ifr_name, buffer,strlen("en0"));


    sa_in = (struct sockaddr_in *)&ifreq.ifr_addr;
    sa_in->sin_family = AF_INET;
    sa_in->sin_len = sizeof(*sa_in);
    inet_aton(ip, &sa_in->sin_addr);

    if (ioctl(iSocket, SIOCSIFNETMASK, &ifreq) == -1)
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_INM)
      printf("[INM] LANInit, invalid ioctl. Buffer: %s\n" , buffer);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LAN))

      #ifdef CFG_PRN_ERR
      perror("ERR  [INM] LANInit, set netmask address");
      #endif // CFG_PRN_ERR

      // 20100517 BC (Rqd by ZSL)
      rstatus = E_ERR_LNC_InvalidIoctlForNetMask;
    }

    free(ip);
    if (rstatus != E_ERR_Success)
    {
      sprintf(buffer,
        "\n**********\n %d st ethernet port (LAN1) error\n****\n" , nI);
      g_pEventLog->LogMessage(buffer);
      return rstatus;
    }
    close(iSocket);
  }

  //add gateway 20100519 BC (Rqd by ZSL)
  for(nI=0; nI<CFG_LNC_TOTAL_ACTIVE_PORT; nI++)
  {
    #ifdef CFG_PRIMARY_LAN_PORT_4
    if (nI == 0)
      continue;
    #endif // CFG_PRIMARY_LAN_PORT_4

    if((g_tRTUConfig.tLANPara[nI].acLAN_Gateway[0] == 0) ||
       (g_tRTUConfig.tLANPara[nI].acLAN_Netmask[0] == 0))
      continue;

    rstatus = LNC_AddGateway(g_tRTUConfig.tLANPara[nI].acLAN_Gateway,
                             g_tRTUConfig.tLANPara[nI].acLAN_Netmask, 5);
    SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
    printf("[INM] LANInit, add gateway %s to routing table\n",
           g_tRTUConfig.tLANPara[nI].acLAN_Gateway);
    #endif //((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  }

  //Initial RTU status
  g_tRTUStatus.nRTUStateFlag = STATEFLAG_INITIALIZATION;  //Set RTU state
  g_tRTUStatus.unRTUServerLinkStatus = LINK_FAULTY;
  g_tRTUStatus.unRTURMTLinkStatus = LINK_FAULTY;      //040419 Yu Wei
  g_tRTUStatus.abServerLinkStatusI[0] = false;
  g_tRTUStatus.abServerLinkStatusI[1] = false;
  g_tRTUStatus.abServerLinkStatusI[2] = false;
  g_tRTUStatus.abServerLinkStatusI[3] = false;
  g_tRTUStatus.abServerLinkStatusI[4] = false;
  g_tRTUStatus.abOtherLinkStatus[0] = true;
  g_tRTUStatus.abOtherLinkStatus[1] = true;    //Set link2 (LAN1) between RTU to OK.
                          //normally link2 is not polling.
  g_tRTUStatus.bCFGFileDownloadRequired = false;
  g_tRTUStatus.bCFCardError = false;

  return E_ERR_Success;

}// LANInit

/*******************************************************************************
Purpose:
  Stop all RTU tasks.

*******************************************************************************/
void stopRTU(void)
{
  #if (CFG_ENABLE_PCI_DETECTION)
  E_ERR_T rstatus = E_ERR_Success;
  CHAR errmsgbuff[ERR_STR_SZ] = {0};
  #endif // (CFG_ENABLE_PCI_DETECTION)

  SYS_isExitAppl = E_TYPE_True;

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  printf("[INM] stopRTU: stop timer\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  StopTimer();    //Stop timer task.  //040520 Yu Wei

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  printf("[INM] stopRTU: stop all swc\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  StopAllSWC();
  g_pEventLog->LogMessage((CHAR *)"stop all swc ...");


  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  printf("[INM] stopRTU: stop watchdog task\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  //StopWatchdogTask();
  g_pEventLog->LogMessage((CHAR *)"stop watchdog task ...");

  //StopRMM();
  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  printf("[INM] stopRTU: stop server poll task\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  StopAllServerPollTask();
  g_pEventLog->LogMessage((CHAR *)"stop all server poll task ...");

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  printf("[INM] stopRTU: stop MMM task\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  StopMMMMainTask();
  g_pEventLog->LogMessage((CHAR *)"stop mmm task ...");

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  printf("[INM] stopRTU: stop RMM task\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  StopRMM();

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  printf("[INM] stopRTU: stop watchdog function\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  StopWatchDogFunction();

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  printf("[INM] stopRTU: stop other RTU link\n");
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
  OtherRTULinkStop();

  if(g_pMMM != TYP_NULL)
    delete g_pMMM;

  /*
   * WARN: Stopping log task got issue.
  printf("[SYS] stopRTU, stop log task ...\n");
  g_pEventLog->LogMessage((CHAR *)"stop log task ...");
  sleep(5);
  StopLogTask();
  */

//  #if (CFG_ENABLE_PCI_DETECTION)
//  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
//  printf("[INM] stopRTU: killing devc-ser8250\n");
//  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_INM))
//  if((rstatus = SYS_DeinitializeDevc8250Manager()) != E_ERR_Success)
//  {
//    #ifdef CFG_PRN_ERR
//    ERR_GetMsgString(rstatus, errmsgbuff);
//    printf("ERR  [INM] stopRTU, de-initialized devc-ser8250, %s\n",
//           errmsgbuff);
//    #endif // CFG_PRN_ERR
//  }
//  #endif // (CFG_ENABLE_PCI_DETECTION)
} // stopRTU

/*******************************************************************************
Purpose:
  Reset RTU.

HISTORY

   NAME            DATE                    REMARKS

 Bryan Chong      13-Aug-2010      Update to use execl for the system reset
                                   functionality [PR66]
 Bryan Chong      05-Jan-2011      Extend delay to 1 second for log message
                                   saving
 Bryan Chong      27-Apr-2011      Move stopRTU routine before executing
                                   shutdown. [C955 PR91]
*******************************************************************************/
void ResetRTU(void)
{

  #if CFG_ENABLE_SYSTEM_SHUTDOWN
  HWP_LCM_PrnMsgCtrl(E_HWP_LCM_MSG_Blank);
  HWP_LCM_PrnMsgCtrl(E_HWP_LCM_MSG_RTUExit);
  #endif // CFG_ENABLE_SYSTEM_SHUTDOWN

  stopRTU();

  #if CFG_ENABLE_SYSTEM_SHUTDOWN

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_SYS_EXIT)
  printf("[SYS] System Rebooting...\n");
  #endif //((defined CFG_DEBUG_MSG) && CFG_DEBUG_SYS_EXIT)

  SYS_ExecShutDown();
  #endif // CFG_ENABLE_SYSTEM_SHUTDOWN
}


void LogInitError(char * msg)
{
  FILE *fd ;
  time_t LogTime = time(NULL);

  fd = fopen("InitErr.log" , "a+");
  if(fd == TYP_NULL)
  {
    perror("Can Not Open InitErr.log ");
    return;
  }

   fprintf(fd , "\tRTU Initialize at %s \n" , ctime(&LogTime));
   fputs(msg , fd );
   fputs("\n\n", fd);

   fclose(fd);
   delay(100);
   return;

}
/*
#ifdef ModuleTest

void LogGlobalValues(void)
{
  int i,j;
  FILE *fd;

  printf("Start write 'GValues.txt'.\n");
  fd = fopen("GValues.txt", "w+");
  if ( fd == TYP_NULL) {
    printf("Can not save Global Values to 'GValues.txt'.\r\n");
    return;
  }
  //write time
  time_t lTime = time(NULL);
  //if ( lTime != ERROR){
    fprintf(fd,"%s\r\n", ctime(&lTime));
//  }

  //write values
  //g_tRTUConfig
  fprintf(fd,"--------g_tRTUConfig--------\r\n");
  fprintf(fd,"unsigned short  unVersion: %X\r\n", g_tRTUConfig.unVersion);

  //Local time zone in seconds.  //040329 Yu Wei.
  fprintf(fd,"int nLocalTimeZone: %d\n", g_tRTUConfig.nLocalTimeZone);

  //Identification number of the RTU
  fprintf(fd,"unsigned short  unRTUIdentification: %d\r\n", g_tRTUConfig.unRTUIdentification);

  //Location of the RTU
  fprintf(fd,"char acRTULocalion[10]: %s\r\n", g_tRTUConfig.acRTULocalion);

  //1 -- RTU1,  2--RTU2.
  fprintf(fd,"int nRTUID: %d\r\n", g_tRTUConfig.nRTUID);

  //RTU maintenance terminal socket ID
  fprintf(fd,"int nMaintTerminalSocketID: %d\r\n", g_tRTUConfig.nMaintTerminalSocketID);

  //RTU maintenance terminal timeout in second
  fprintf(fd,"int nMaintTerminalTimeout: %d\r\n", g_tRTUConfig.nMaintTerminalTimeout);

  //Timeout in milisecond before confirming the LAN down during initialization.
  fprintf(fd,"int nInitLANCheckTimeout: %d\r\n", g_tRTUConfig.nInitLANCheckTimeout);

  //Number of M45 boards on A12 board
  fprintf(fd,"int nNumberOfSerialCOMCard: %d\r\n", g_tRTUConfig.nNumberOfSerialCOMCard);

  //char  acVDUName[8];    //VDU user name
  //char  acVDUPassword[8];  //VDU password
  for (i=0;i<VDU_MAX_USER;i++){
    fprintf(fd,"tVDUName tVDUNameList[%d].acVDUName[8]: %s\r\n", i, g_tRTUConfig.tVDUNameList[i].acVDUName);    //List of VDU user names, max 10 accounts, each 8 bytes code
    fprintf(fd,"tVDUName tVDUNameList[%d].acVDUPassword[8]: %s\r\n",i, g_tRTUConfig.tVDUNameList[i].acVDUPassword);
  }                    //and associated password, each max 8 bytes password

  fprintf(fd,"int nVDUUserNumber: %d\r\n", g_tRTUConfig.nVDUUserNumber);//The VDU user number that is defined in config.txt file.

  for (i=0;i<2;i++){
    fprintf(fd,"tLANParameter tLANPara[%d].acLAN_IP[20]: %s\r\n", i, g_tRTUConfig.tLANPara[i].acLAN_IP);      //RTU-RTU LAN1,2 link Parameters
    fprintf(fd,"tLANParameter tLANPara[%d].acLAN_Gateway[20]: %s\r\n", i, g_tRTUConfig.tLANPara[i].acLAN_Gateway);
    fprintf(fd,"tLANParameter tLANPara[%d].anOtherRTULinkSocketID: %d\r\n", i, g_tRTUConfig.tLANPara[i].anOtherRTULinkSocketID);
    fprintf(fd,"tLANParameter tLANPara[%d].cLAN_SlaveAddress: %d\r\n", i, g_tRTUConfig.tLANPara[i].cLAN_SlaveAddress);
    fprintf(fd,"tLANParameter tLANPara[%d].cModbusReadCommand: %d\r\n", i, g_tRTUConfig.tLANPara[i].cModbusReadCommand);
    fprintf(fd,"tLANParameter tLANPara[%d].nPollingTime: %d\r\n", i, g_tRTUConfig.tLANPara[i].nPollingTime);
    fprintf(fd,"tLANParameter tLANPara[%d].nPollingTimeout: %d\r\n", i, g_tRTUConfig.tLANPara[i].nPollingTimeout);
    fprintf(fd,"tLANParameter tLANPara[%d].nPollingRetryNumber: %d\r\n", i, g_tRTUConfig.tLANPara[i].nPollingRetryNumber);
  }


  for (i=0;i<2;i++){
    for (j=0;j<20;j++) fprintf(fd,"char acOtherRTUIPAddress[%d][%d]: %c\r\n", i,j, g_tRTUConfig.acOtherRTUIPAddress[i][j]);      //Other RTU IP address
  }

  fprintf(fd,"tRTUTableStructure tRTUStatusTable.unTableStartAddress: %d\r\n",g_tRTUConfig.tRTUStatusTable.unTableStartAddress);    //Definition of the RTU STATUS table
  fprintf(fd,"tRTUTableStructure tRTUStatusTable.unTableEndAddress: %d\r\n",g_tRTUConfig.tRTUStatusTable.unTableEndAddress);    //Definition of the RTU STATUS table

  //Definition of the RTU COMMAND table
  fprintf(fd,"tRTUTableStructure tRTUComanmdTable.unTableStartAddress: %d\r\n",g_tRTUConfig.tRTUComanmdTable.unTableStartAddress);    //Definition of the RTU STATUS table
  fprintf(fd,"tRTUTableStructure tRTUComanmdTable.unTableEndAddress: %d\r\n",g_tRTUConfig.tRTUComanmdTable.unTableEndAddress);    //Definition of the RTU STATUS table

  fprintf(fd,"int nRTUCommandSoctetID: %d\r\n", g_tRTUConfig.nRTUCommandSoctetID);  //Read/Write RTU Status Table/Command Table Socket ID

  fprintf(fd,"int nRTUPollingSocketNumber: %d\r\n", g_tRTUConfig.nRTUPollingSocketNumber);      //Read RTU Polling Socket port number

  for (i=0;i<SERVER_POLLING_SOCKET_MAX;i++)
    fprintf(fd,"int nRTUPollingSocketID[%d]: %d\r\n", i, g_tRTUConfig.nRTUPollingSocketID[i]);  //Read RTU Polling Socket ID

  fprintf(fd,"int nServerCMDTimeout: %d\r\n", g_tRTUConfig.nServerCMDTimeout);          //Time out with the server in ms

  fprintf(fd,"int nSWCNumber: %d\r\n", g_tRTUConfig.nSWCNumber);              //Number of external system

  for (i=0;i<SWC_MAX_NUMBER;i++)
    fprintf(fd,"int anSWCIndex[%d]: %d\r\n", i, g_tRTUConfig.anSWCIndex[i]);      //Store SWC ID that installs in the system.

  for (i=0;i<SWC_MAX_NUMBER;i++)
    fprintf(fd,"unsigned short  anMultidorpSWCChain[%d]: %d\r\n", i, g_tRTUConfig.anMultidorpSWCChain[i]);  //Store multidrop link SWC information.
                              //The value is next link SWC ID.
                              //i.e. PSD1 (ID=8) and PSD2 (ID=9) is multidrop link,
                              // so anMultidorpSWCChain[8] = 9 and
                              // anMultidorpSWCChain[9] = 8.
  for (i=0;i<SWC_MAX_NUMBER;i++)
    fprintf(fd,"char acSWCAddress[%d]: %d\r\n", i, g_tRTUConfig.acSWCAddress[i]);  //SWC address


  for (i=0;i<SWC_MAX_NUMBER;i++){
    //tSWCCFGHeaderStructure  tHeader;        //SWC header configuration
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.acName[10]: %s\r\n", i, g_tRTUConfig.tSWC[i].tHeader.acName);  //SWC parameter
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.ucID: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.ucID);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.nLinkType: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.nLinkType);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.nLinkGroup: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.nLinkGroup);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.tLanLink[0]: %s\r\n",
      i, g_tRTUConfig.tSWC[i].tHeader.tLanLink[0].ip);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.tLanLink[1]: %s\r\n",
      i, g_tRTUConfig.tSWC[i].tHeader.tLanLink[1].ip);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.tLinkPara[0].unPortID: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.tLinkPara[0].unPortID);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.tLinkPara[0].nBaudRate: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.tLinkPara[0].nBaudRate);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.tLinkPara[0].nDataBit: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.tLinkPara[0].nDataBit);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.tLinkPara[0].nParity: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.tLinkPara[0].nParity);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.tLinkPara[0].nStopBit: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.tLinkPara[0].nStopBit);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.tLinkPara[0].cSlaveAddress: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.tLinkPara[0].cSlaveAddress);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.tLinkPara[0].cModbusReadCommand: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.tLinkPara[0].cModbusReadCommand);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.tLinkPara[1].unPortID: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.tLinkPara[1].unPortID);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.tLinkPara[1].nBaudRate: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.tLinkPara[1].nBaudRate);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.tLinkPara[1].nDataBit: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.tLinkPara[1].nDataBit);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.tLinkPara[1].nParity: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.tLinkPara[1].nParity);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.tLinkPara[1].nStopBit: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.tLinkPara[1].nStopBit);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.tLinkPara[1].cSlaveAddress: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.tLinkPara[1].cSlaveAddress);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.tLinkPara[1].cModbusReadCommand: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.tLinkPara[1].cModbusReadCommand);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.nStandbyPollingFrequency: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.nStandbyPollingFrequency);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.nFailedPollingFrequency: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.nFailedPollingFrequency);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.nReceiveTimeout: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.nReceiveTimeout);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.nRetryNumber: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.nRetryNumber);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.nFastPollingFrequency: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.nFastPollingFrequency);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.nSlowPollingFrequency: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.nSlowPollingFrequency);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.nSpuriousTime: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.nSpuriousTime);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.nSendTimeSynHour: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.nSendTimeSynHour);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.nSendTimeSynMinute: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.nSendTimeSynMinute);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.unSendTimeSynStartAddr: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.unSendTimeSynStartAddr);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.unSendTimeSynLength: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.unSendTimeSynLength);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.nNotation: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.nNotation);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.nSocketID: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.nSocketID);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tHeader.nLANLinkTimeout: %d\r\n", i, g_tRTUConfig.tSWC[i].tHeader.nLANLinkTimeout);
    //  tSWCTableStructure          tTable[SWC_MAX_TABLE];  //SWC Table
    //for (j=0; j<SWC_MAX_TABLE; j++){    //The type and variable are removed. 040322 Yu Wei
    //  fprintf(fd,"tSWCTableStructure tSWC[%d].tTable[%d].unStartAddress: %d\r\n", i,j, g_tRTUConfig.tSWC[i].tTable[j].unStartAddress);
    //  fprintf(fd,"tSWCTableStructure tSWC[%d].tTable[%d].nTableLen: %d\r\n", i,j, g_tRTUConfig.tSWC[i].tTable[j].nTableLen);
    //}
    //tSWCPollingAddress          tPollingAddress;    //Polling information
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tPollingAddress.tFastPollingAddress.unStart: %d\r\n", i, g_tRTUConfig.tSWC[i].tPollingAddress.tFastPollingAddress.unStart);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tPollingAddress.tFastPollingAddress.unLength: %d\r\n", i, g_tRTUConfig.tSWC[i].tPollingAddress.tFastPollingAddress.unLength);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tPollingAddress.tFastPollingAddress.unServerStart: %d\r\n", i, g_tRTUConfig.tSWC[i].tPollingAddress.tFastPollingAddress.unServerStart);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tPollingAddress.tFastPollingAddress.unServerLength: %d\r\n", i, g_tRTUConfig.tSWC[i].tPollingAddress.tFastPollingAddress.unServerLength);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tPollingAddress.tSlowPollingAddress.unStart: %d\r\n", i, g_tRTUConfig.tSWC[i].tPollingAddress.tSlowPollingAddress.unStart);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tPollingAddress.tSlowPollingAddress.unLength: %d\r\n", i, g_tRTUConfig.tSWC[i].tPollingAddress.tSlowPollingAddress.unLength);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tPollingAddress.tSlowPollingAddress.unServerStart: %d\r\n", i, g_tRTUConfig.tSWC[i].tPollingAddress.tSlowPollingAddress.unServerStart);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tPollingAddress.tSlowPollingAddress.unServerLength: %d\r\n", i, g_tRTUConfig.tSWC[i].tPollingAddress.tSlowPollingAddress.unServerLength);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tPollingAddress.tTimerAddress.unStart: %d\r\n", i, g_tRTUConfig.tSWC[i].tPollingAddress.tTimerAddress.unStart);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tPollingAddress.tTimerAddress.unLength: %d\r\n", i, g_tRTUConfig.tSWC[i].tPollingAddress.tTimerAddress.unLength);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tPollingAddress.tTimerAddress.unServerStart: %d\r\n", i, g_tRTUConfig.tSWC[i].tPollingAddress.tTimerAddress.unServerStart);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tPollingAddress.tTimerAddress.unServerLength: %d\r\n", i, g_tRTUConfig.tSWC[i].tPollingAddress.tTimerAddress.unServerLength);
    fprintf(fd,"tSWCCFGStructure tSWC[%d].tPollingAddress.nTimerInterval: %d\r\n", i, g_tRTUConfig.tSWC[i].tPollingAddress.nTimerInterval);

  }

  //g_tRTUConfig
  fprintf(fd,"--------g_tRTUStatus--------\r\n");

  fprintf(fd,"int nRTUStateFlag: %d\r\n", g_tRTUStatus.nRTUStateFlag);
  fprintf(fd,"unsigned short unRTUServerLinkStatus: %d\r\n", g_tRTUStatus.unRTUServerLinkStatus);
  for (i=0; i<SERVER_POLLING_SOCKET_MAX; i++)
    fprintf(fd,"bool abServerLinkStatusI[%d]: %d\r\n",i, g_tRTUStatus.abServerLinkStatusI[i]);
  fprintf(fd,"bool abOtherLinkStatus[0]: %d\r\n", g_tRTUStatus.abOtherLinkStatus[0]);
  fprintf(fd,"bool abOtherLinkStatus[1]: %d\r\n", g_tRTUStatus.abOtherLinkStatus[1]);
  fprintf(fd,"bool bCFGFileDownloadRequired: %d\r\n", g_tRTUStatus.bCFGFileDownloadRequired);

  lTime = time(NULL);
  //if ( lTime != ERROR){
    fprintf(fd,"\r\nEnd: %s\r\n", ctime(&lTime));
  //}
  fclose(fd);

  printf("End write 'GValues.txt'.\n");

}

#endif
*/

