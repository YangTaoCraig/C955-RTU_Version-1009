/************************************************************
*                              *
*  Project:    C830 RTU                  *
*  Module:    CMM                      *
*  File :      CMM_Listen.cpp                *
*                              *
*  Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************

**************************************************************

DESCRIPTION

  This file implements the Common module: specific command
  listen socket.

*************************************************************
Modification History
---------------------

Version:
  CMM D.1.1.1
  01  3 April 2003, PKK
    Start to write.

  02  30 May 2003, PKK
    Client connection will be closed on receive error/timeout.

Version: CMM D.1.3.6
  01  21 May 2004, Yu Wei
    Modified CListen(), Listen() and Recv() to use relative
    timer and avoid the timer jump.
    Refer to PR:OASYS-PMS 0201.

  02  04 June 2004, Yu Wei
    Modified Recv(). To fix bug.
    Refer to PR:OASYS-PMS 0217.

**************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/neutrino.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"
//#include "flg_def.h"
//#include "flg_ext.h"

#include "Common.h"
#include "CMM_Timer.h"
#include "CMM_Listen.h"
#include "CMM_Log.h"
#include <errno.h>


//************************************************************************
//  Listen class
//************************************************************************
//pthread_mutex_t m_mutexRecv;    //mutex to control the access of Recv

CListen::CListen(
          char *pcListenIP,        //in: IP to listen on
          unsigned short unListenPort,  //in: port to listen on
          int nRecvTimeout        //in: receive timeout in ms
        )
{
  int nReturn;
  pthread_mutexattr_t *attr = NULL;
  strncpy(m_acListenIP, pcListenIP, sizeof(m_acListenIP) - 1);
  m_acListenIP[sizeof(m_acListenIP) - 1] = 0;

  m_unListenPort = unListenPort;

  m_nRecvTimeout = nRecvTimeout;

  m_nSockClient = ERROR;

  m_tsLastRecvOK.Low = 0;    //040521 Yu Wei
  m_tsLastRecvOK.High = 0;  //040521 Yu Wei
  m_bSockAccept=true;      // 130722 Yang Tao

  //create semaphore

  attr = (pthread_mutexattr_t *) malloc(sizeof(pthread_mutexattr_t));
  pthread_mutexattr_init( attr );
  nReturn = pthread_mutex_init(& m_semClientClosed, attr );
  free(attr);
//  m_semClientClosed = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
//
  if (nReturn < 0 )
  {
    g_pEventLog->LogMessage((CHAR *)"Listen: create semaphore failed\n");
  }

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_CMM_LISTEN)
  printf("[CMM] CListen::CListen, run successfully\n");
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_CMM_LISTEN)
}

CListen::~CListen()
{
  //close client socket
  if (m_nSockClient != ERROR)
  {
    shutdown(m_nSockClient, 1);  //send disallowed
    close(m_nSockClient);
  }
//
  if (m_nSockListen != ERROR)
  {
    close(m_nSockListen);
  }
//
//  //delete semaphore

  pthread_mutex_destroy(&m_semClientClosed);

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_CMM_LISTEN)
  printf("[CMM] CListen::~CListen, run successfully\n");
  #endif // ((defined CFG_DEBUG_MSG) && CFG_DEBUG_CMM_LISTEN)

}

/*------------------------------------------------------------------------------
Purpose:
  Listen task function to be spawned

History

      Name         Date       Remark
      ----         ----       ------
 Bryran Chong   17-May-2010  Update with SYS_isExitAppl flag for terminating
                             the application
------------------------------------------------------------------------------*/
void CListen::ListenTask()
{
  while(1)
  {
//Yang Tao: Debug message for DO command.
//    printf("[CMM] CListen::ListenTask, Begin \n");
    Listen();          //Listen() will return when there is error

    delay(388);
    //Yang Tao: Debug message for DO command.
    //printf("[CMM] CListen::ListenTask, End \n");
    // 20100517 BC (Rqd by ZSL)
    if(SYS_isExitAppl == E_TYPE_Yes)
    {
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_CMM))
      printf("[CMM] CListen::ListenTask, exit task\n");
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_CMM))
      break;
    }
  }
}

/*
Purpose:
  Listen for client connection
*/
void CListen::Listen()
{
//  int nSockListen;        //listen socket
  struct sockaddr_in  addrListen;  //listen socket address
  int nOn = 1;          //set socket option value for re-use socket address
  char acTemp[200];
  struct sockaddr_in  addrClient;  //client socket address
  int nAddrSize;          //size of address
  char cLog[128];          //error log buffer
  CHAR thrdnamebuff[SYS_THRD_NAME_LEN] = {0};

  //pthread_mutex_lock(& m_semClientClosed);

  //create listen socket
  m_nSockListen = socket(AF_INET, SOCK_STREAM , 0);
  if (m_nSockListen <= 0 )
  {
    g_pEventLog->LogMessage((CHAR *)"Listen: create listen socket failed, Too many open fd.\n");
    //#ifdef CFG_PRN_ERR
    perror("ERR  [CMM]CListen::Listen, create socket fail: errno");
    //#endif // CFG_PRN_ERR

    sprintf(cLog, "CListen::Listen: Slay command due to create listen socket failed ");
	g_pEventLog->LogMessage(cLog);
	delay(5000);	//20151117 YT
    system("slay rtu_rel");	//20151117 YT  If failed to create fd will slay the application
							// For auto reboot
    return;
  }

  //enable re-use socket address option
  if (setsockopt(m_nSockListen, SOL_SOCKET , SO_REUSEADDR , (char *)&nOn,
      sizeof(nOn)) < 0 )
  {
    g_pEventLog->LogMessage((CHAR *)"Listen: set socket re-use address failed\n");
    close(m_nSockListen);
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_CMM_LISTEN))
    printf("[CMM]CListen::Listen, close listen socket %d\n", m_nSockListen);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_CMM_LISTEN))
    return;
  }


  //init listen socket address
  memset(&addrListen, 0, sizeof(struct sockaddr_in));
  addrListen.sin_family = AF_INET;
  addrListen.sin_port = htons(m_unListenPort);
  addrListen.sin_addr.s_addr = inet_addr(m_acListenIP);

  //bind listen socket
  if ( bind(m_nSockListen, (struct sockaddr *)&addrListen,
            sizeof(struct sockaddr)) < 0 )
  {
    sprintf(cLog, "Listen (%s: %d) bind listen socket failed return",
            m_acListenIP, m_unListenPort);
    g_pEventLog->LogMessage(cLog);
    close(m_nSockListen);
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_CMM_LISTEN))
    printf("[CMM]CListen::Listen, close socket %d when bind %s fail\n",
            m_nSockListen, m_acListenIP);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_CMM_LISTEN))
    return;
  }
  else
  {
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_CMM_LISTEN))
    printf("[CMM] CListen::Listen, bind socket %d for %s OK\n",
           m_nSockListen, m_acListenIP);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_CMM_LISTEN))
  }
  //enable connections to listen socket,
  // maximum number of 1 unaccepted connection that can be pending at one time
  if (listen(m_nSockListen, 1) == ERROR)
  {
    g_pEventLog->LogMessage((CHAR *)"Listen: start listen socket failed");
    close(m_nSockListen);
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_CMM_LISTEN))
    printf("[CMM]CListen::Listen, close socket %d, when listen fail\n",
            m_nSockListen);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_CMM_LISTEN))
    return;
  }
//31 May 2013 Yang Tao: Comment out the while
 while(1)
  {
	 if(m_bSockAccept==false)
	 {
	   delay(5);
	   continue;
	 }

    //accept client socket
    nAddrSize = sizeof(struct sockaddr_in);

    m_tsLastRecvOK.Low = 0;    //040521 Yu Wei
    m_tsLastRecvOK.High = 0;  //040521 Yu Wei
    //pthread_mutex_lock(& m_semClientClosed);// 22072013: Yang Tao Change
    m_nSockClient = accept(m_nSockListen, (struct sockaddr *)&addrClient,
                           &nAddrSize);

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_CMM_LISTEN_ACCP_SOCKET)
    printf("[CMM] CListen::Listen,  m_nSockListen %d update m_nSockClient to "
           "%d\n",
           m_nSockListen, m_nSockClient);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_CMM_LISTEN_ACCP_SOCKET))

    if (m_nSockClient <= 0 )	//20151116 Su, == ERROR)
    {
//      #if ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_CMM_LISTEN_SOCK))
//      sprintf(acTemp,"CListen::Listen, accept fail, invalid socket client "
//             "socket %d\n   listen at %s socket %d. Close listen socket\n",
//             m_nSockClient, m_acListenIP, m_nSockListen);
//      g_pEventLog->LogMessage(acTemp);
//      #endif // ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_CMM_LISTEN))
      close(m_nSockListen);
//	  sprintf(cLog, "Listen: m_nSockClient FD is out of limitation Slay command failed to run");
//				g_pEventLog->LogMessage(cLog);
//	  delay(5000);
//	  system("slay rtu_rel");
      return;
    }
    else
    {
    	if(m_nSockClient > FD_SETSIZE)	//20151126 Su
    	{
    	    g_pEventLog->LogMessage((CHAR *)"Listen: accept connection on socket FAILED, Too many open fd.\n");
    	    //#ifdef CFG_PRN_ERR
    	    perror("ERR  [CMM]CListen::Listen: accept connection on socket FAILED: errno");
    	    //#endif // CFG_PRN_ERR

    	    sprintf(cLog, "CListen::Listen: Slay command due to accept connection on socket FAILED");
    		g_pEventLog->LogMessage(cLog);
    		delay(5000);
    	    system("slay rtu_rel");
    	}
      //#if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_CMM_LISTEN)
      printf("[CMM] CListen::Listen, %s, %d accepted client %s,"
             " %d\n",
             m_acListenIP, m_nSockListen, inet_ntoa(addrClient.sin_addr),
             m_nSockClient);
      //#endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_CMM_LISTEN))

       m_bSockAccept=false;
		sprintf(acTemp,"[CMM] Listen, client %s, fd %d, connected on local %s, fd %d:%u",
			  inet_ntoa(addrClient.sin_addr), m_nSockClient, m_acListenIP, m_nSockListen, m_unListenPort);
		g_pEventLog->LogMessage(acTemp);
//        printf("Listen: client %s at fd %d connected on local %s : %d at fd %d\n",
//                inet_ntoa(addrClient.sin_addr),m_nSockClient,m_acListenIP,m_unListenPort,m_nSockListen);
    }

    if (m_nSockClient == ERROR)
    {
      close(m_nSockListen);
      break;
    }
    //set last recv OK time to now
    GetTimerValue(&m_tsLastRecvOK);    //040521 Yu Wei

    //wait until client socket closed due to receive error in Recv()
   pthread_mutex_lock(& m_semClientClosed);
  }
}// Listen

// Yang Tao:20130702 SendSWC

 int CListen::SendSWC(
          char *cBuffer,  //in: buffer to send
          int nSize,    //in: size of buffer to send
          int nTimeout  //in: timeout in millisecond, 0=send immediately
         )
{
  fd_set fdsWrite;
  struct timeval tv;
  int nStatus, nBytesSent;
  char acTemp[512] = {0};
  char acTemp1[512] = {0};


  if (m_nSockClient == ERROR)
  {
    return ERROR;
  }

  while(1)
  {
    if (nSize <= 0)
    {
      return ERROR;
    }

    FD_ZERO(&fdsWrite);
    FD_SET(m_nSockClient, &fdsWrite);

    nTimeout += 20;
    tv.tv_usec = (nTimeout % 1000) * 1000;
    tv.tv_sec = nTimeout / 1000;
    //return no. of file descriptors with activity, 0 if timed out, or ERROR
    nStatus = select(m_nSockClient + 1, NULL, &fdsWrite, NULL, &tv);

    if (nStatus == ERROR)
    {

      return ERROR;

    }

    if (nStatus == 0)
    {
    	printf("[CListen] SockClient: %d Timeout:\n", m_nSockClient);
      return TIMEOUT;
    }

    if ( (nStatus > 0) && (FD_ISSET(m_nSockClient, &fdsWrite)) )
    {
      //return no. of bytes sent, or ERROR
      nBytesSent = write(m_nSockClient, cBuffer, nSize);
//      printf("[CListen] Number of %d bytes has been send to %d:\n", nBytesSent,m_nSockClient);
      if (nBytesSent <= 0)
      {
        g_pDebugLog->LogMessage((CHAR *)"Listen->Send data  <= 0 ,Got a signal");
        return ERROR;
      }
      //Yang Tao: Comments out his else for large message to send
      else
      {
  		for(int nI=0; nI<nSize; nI++)
  		sprintf(&acTemp1[nI*2],"%02X",(unsigned char)cBuffer[nI]);
  		sprintf(acTemp,"[CMM] Listen: SendSWC, Device Sent to Server: (fd %d)(m_bSockAccept %d)(%d bytes) %s", m_nSockClient, m_bSockAccept, nBytesSent, acTemp1);
  		g_pEventLog->LogMessage(acTemp);

//  		delay(300);
//      	Close(6);
//      	if(g_tDebugLogFlag.bSWCCommand== true)
//      	{
//  		sprintf(acTemp,"[CMM] Listen: SendSWC, Close(6) for %u, (m_bSockAccept %d)\n", m_unListenPort, m_bSockAccept);
//  		g_pEventLog->LogMessage(acTemp);
//      	}

        return OK;
      }

      nSize -= nBytesSent;
      cBuffer += nBytesSent;

      if (nSize == 0)
      {
        return OK;

      }
    }
    else
    {
      return ERROR;
    }
  }

    return OK;
}// Send

/*
Purpose:
  Send a message to client with timeout
Return:
  OK (0)
  ERROR (-1)
  TIMEOUT (-2)
*/
int CListen::Send(
          char *cBuffer,  //in: buffer to send
          int nSize,    //in: size of buffer to send
          int nTimeout  //in: timeout in millisecond, 0=send immediately
         )
{
  fd_set fdsWrite;
  struct timeval tv;
  int nStatus, nBytesSent;


  if (m_nSockClient == ERROR)
  {
    return ERROR;
  }

  while(1)
  {
    if (nSize <= 0)
    {
      return ERROR;
    }

    FD_ZERO(&fdsWrite);
    FD_SET(m_nSockClient, &fdsWrite);

    nTimeout += 20;
    tv.tv_usec = (nTimeout % 1000) * 1000;
    tv.tv_sec = nTimeout / 1000;
    //return no. of file descriptors with activity, 0 if timed out, or ERROR
    nStatus = select(m_nSockClient + 1, NULL, &fdsWrite, NULL, &tv);

    if (nStatus == ERROR)
    {
      return ERROR;
    }

    if (nStatus == 0)
    {
    	printf("[CListen] SockClient: %d Timeout:\n", m_nSockClient);
      return TIMEOUT;
    }

    if ( (nStatus > 0) && (FD_ISSET(m_nSockClient, &fdsWrite)) )
    {
      //return no. of bytes sent, or ERROR
      nBytesSent = write(m_nSockClient, cBuffer, nSize);
//      printf("[CListen] Number of %d bytes has been send to %d:\n", nBytesSent,m_nSockClient);
      if (nBytesSent <= 0)
      {
        g_pDebugLog->LogMessage((CHAR *)"Listen->Send data  <= 0 ,Got a signal");
        return ERROR;
      }

      else
      {
    	//Close(6);
    	m_bSockAccept=true;
    	//close(m_nSockClient);
        return OK;
      }

      nSize -= nBytesSent;
      cBuffer += nBytesSent;

      if (nSize == 0)
      {

        return OK;
      }
    }
    else
    {
      return ERROR;
    }
  }

    return OK;
}// Send

/*------------------------------------------------------------------------------
Purpose:
  Receive a message from client with timeout

Parameter:
  cBuffer  [out] receiving buffer
  nSize    [in]  max size of receive buffer
  nTimeout [in]  timeout in millisecond, 0=recv immediately
  nExact   [in]  TRUE=recv exactly nSize bytes, FALSE=recv any size

Return:
  >0:        No. of bytes received, connection will not be closed
  ERROR   (-1):  Connection will be closed
  TIMEOUT (-2):  Connection will be closed if total timeout >=
                 specified recv timeout
------------------------------------------------------------------------------*/
int CListen::Recv(char *cBuffer, int nSize, int nTimeout, int nExact)
{
  fd_set fdsRead;
  struct timeval tv;
  int nStatus, nBytesRecv, nSizeToRecv;
  struct tTimerValue tsNow;    //040521 Yu Wei
  int nMilliSecondDiff;
  char cLog[128];

  //Use relative timer. //040521 Yu Wei
  if ((m_nSockClient < 0))
  {
    #if ((defined CFG_PRN_WARN) && _CFG_PRN_WARN_CMM_RECV)
    if(nTimeout == 60)
    printf("WARN [CMM] CListen::Recv, invalid m_nSockClient %d\n",
           m_nSockClient);
    #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_CMM_RECV)
    return ERROR;
  }
 // delay(50);
 //pthread_mutex_lock(&m_mutexRecv);
  nSizeToRecv = nSize;

  #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_CMM_RECV)
  if(nTimeout == 60)
  {
    printf("[CMM] CListen::Recv, fd %d, nSize %d, timeout %d, nExact %d\n",
            m_nSockClient, nSize, nTimeout, nExact);
  }
  #endif // (defined(CFG_DEBUG_MSG) && CFG_DEBUG_CMM_RECV)
  //Yang Tao: Disable disconnect of timeout and recv 0 byte.
  while(1)
  {
    if (nSizeToRecv <= 0)
    {
      #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_CMM_RECV)
      printf("WARN [CMM] CListen::Recv, rx negative byte, %d\n", nSizeToRecv);
      #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_CMM_RECV)
     // pthread_mutex_unlock(&m_mutexRecv);
      Close(1);
      return ERROR;
    }

    FD_ZERO(&fdsRead);
    FD_SET(m_nSockClient, &fdsRead);

    /*If timeout is NULL, select() blocks until one of the selected conditions occurs*/


    #if ((defined CFG_DEBUG_MSG) && _CFG_DEBUG_CMM_RECV)
    if(nTimeout == 60)
    {
      printf("[CMM] CListen::Recv, fd %d enter while loop\n", m_nSockClient);
    }
    #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_CMM_RECV)

    tv.tv_usec = (nTimeout % 1000) * 1000;
    tv.tv_sec = nTimeout / 1000;  //20151208 Su, changed to /1000. previously /10
    //so changed to a specific timeout
    //return no. of file descriptors with activity, 0 if timed out, or ERROR
    nStatus = select(m_nSockClient + 1, &fdsRead, NULL, NULL, &tv);

    #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_CMM_RECV)
    if(nTimeout == 60)
    {
      printf("[CMM] CListen::Recv, fd %d select status %d\n",
             m_nSockClient, nStatus);
    }
    #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_CMM_RECV)
    if ((nStatus == ERROR) || (m_nSockClient == 0))
    {
      #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_CMM_RECV)
      //printf("WARN [CMM] CListen::Recv, select error\n");
      perror("WARN [CMM] CListen::Recv, select error");
      #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_CMM_RECV)
     // pthread_mutex_unlock(&m_mutexRecv);
      Close(2);
      return ERROR;
    }

    //close connection if total timeout >= m_nRecvTimeout
    else if (nStatus == 0)
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_CMM_RECV)
      if(nTimeout == 60)
      printf("[CMM] CListen::Recv, select return 0\n");
      #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_CMM_RECV)

      //Yang Tao: Keep alive will hold the sokect for Server Com Timeout
      // Set KeepAliveSend flag to terminate the timeout and close the connection
      // KeepAliveSend=1 close, KeepAliveSend=0; continue the normal rountie

//       if (KeepAliveSend==1)
//       {
//
//
//       pthread_mutex_unlock(&m_mutexRecv);
//       Close(3);
//       return TIMEOUT;
//       KeepAliveSend=0;
//
//       }

      //clock_gettime(CLOCK_REALTIME, &tsNow);    //040521 Yu Wei
      GetTimerValue(&tsNow);              //040521 Yu Wei

      /*Will use relative timer.      //040521 Yu Wei
      nMilliSecondDiff = ((int)tsNow.tv_sec - (int)m_tsLastRecvOK.tv_sec)*(int)1000 +
                 ((int)tsNow.tv_nsec - (int)m_tsLastRecvOK.tv_nsec)/(int)1000000;*/

      nMilliSecondDiff = (tsNow.High - m_tsLastRecvOK.High) * TIMER_LOW_MSEC;

      nMilliSecondDiff += tsNow.Low;
      nMilliSecondDiff -= m_tsLastRecvOK.Low;
                        //040521 Yu Wei

      if (nMilliSecondDiff >= m_nRecvTimeout)
      {
       // pthread_mutex_unlock(&m_mutexRecv);
        Close(3);
        return TIMEOUT;
      }
    }

    else if ( (nStatus > 0) && (FD_ISSET(m_nSockClient, &fdsRead)) )
    {
       //return no. of bytes received, or ERROR

       nBytesRecv = read(m_nSockClient, cBuffer, nSizeToRecv);
       #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_CMM_RECV)
       if(nTimeout == 60)
       printf("[CMM] CListen::Recv, fd %d rx %d bytes\n",
              m_nSockClient, nBytesRecv);
       #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_CMM_RECV)
       if (nBytesRecv <= 0)
       {
       #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_CMM_RECV)
//    	   sprintf(cLog,"WARN [CMM] CListen::Recv, fd %d rx %d byte\n",
//                m_nSockClient, nBytesRecv);
//         g_pEventLog->LogMessage(cLog);
         if(nBytesRecv < 0)
           perror("WARN [CMM] CListen::Recv, perror");
        #endif // ((defined CFG_PRN_WARN) && (CFG_PRN_WARN_CMM_LISTEN_SOCK))
           //pthread_mutex_unlock(&m_mutexRecv);
           Close(4);

         return ERROR;
       }

       //set last recv OK time to now
       GetTimerValue(&m_tsLastRecvOK);        //040521 Yu Wei

       if (nExact == 0)
       {
         GetTimerValue(&m_tsLastRecvOK);
       //  pthread_mutex_unlock(&m_mutexRecv);
         #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_CMM_RECV)
         if(nTimeout == 60)
         {
           printf("[CMM] CListen::Recv, fd %d return %d bytes\n",
                  m_nSockClient, nBytesRecv);
         }
         #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_CMM_RECV)
         return nBytesRecv;
       }

       nSizeToRecv -= nBytesRecv;
       cBuffer += nBytesRecv;

       if (nSizeToRecv <= 0)  //040604 Yu Wei
       {
         #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_CMM_RECV)
         printf("[CMM] CListen::Recv, return %d bytes, fd %d rx %d bytes\n",
                nSize, m_nSockClient, nBytesRecv);
         SYS_PrnDataBlock((const UINT8 *) cBuffer, nBytesRecv, 10);
         #endif // CFG_PRN_ERR
        ///pthread_mutex_unlock(&m_mutexRecv);
         return nSize;
       }
    }
    else
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_CMM_RECV)
      printf("[CMM] CListen::Recv, close 5 return error\n");
      #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_CMM_RECV)
      //pthread_mutex_unlock(&m_mutexRecv);
      Close(5);
      return ERROR;
    }
  } // while(1)
  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_CMM_RECV)
  if(nTimeout == 60)
  {
     printf("[CMM] CListen::Recv, return error\n");
  }
  #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_CMM_RECV)
  return ERROR;
}// Recv

/*
Purpose:
  Close client socket if connected
*/
void CListen::Close( int nClosePoint  )  //Which point close the socket.
{
  char cLog[128];  //error log buffer
  char acTemp[128];


  if (m_nSockClient != ERROR)
  {
    #ifdef ENABLE_CLOSE_LISTEN_SOCKET
    shutdown(m_nSockClient, SHUT_WR);  //send disallowed
    close(m_nSockClient);
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_CMM_LISTEN))
    printf("[CMM] CListen::Close, m_nSockClient %d\n", m_nSockClient);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_CMM_LISTEN))
    m_bSockAccept=true;
    m_nSockClient = ERROR;
    close(m_nSockListen);	//20151125 Su, must close Listen socket too
    #endif // ENABLE_CLOSE_LISTEN_SOCKET
// Yang Tao: Enable below debug message to observe
//    if(g_tDebugLogFlag.bSocketConnect == true)

//    {

      sprintf(cLog, "Listen: client disconnected on %s:%u (%d)\n",
              m_acListenIP, m_unListenPort, nClosePoint);
      g_pEventLog->LogMessage(cLog);

//      printf("Listen: client disconnected on %s:%u (%d)\n",
//              m_acListenIP, m_unListenPort, nClosePoint);
//    }

    pthread_mutex_unlock(&m_semClientClosed);
//    delay(100);
  }
}// Close
