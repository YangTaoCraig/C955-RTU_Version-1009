/************************************************************
*															*
*	Project:    C830 RTU									*
*	Module:		CMM											*
*	File :	    CMM_Listen.h								*
*															*
*	Copyright 2003 Singapore Technologies Electronics Ltd.	*
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
	01	3 April 2003, PKK
		Start to write.

	02	30 May 2003, PKK
		Client connection will be closed on receive timeout.

Version: CMM D.1.3.6
	01	21 May 2004, Yu Wei
		Added "CMM_Timer.h" to use relative timer.
		Change m_tsLastRecvOK to type "tTimerValue".
		Refer to PR:OASYS-PMS 0201.

**************************************************************/


#ifndef  _LISTEN_H
#define  _LISTEN_H


#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
//#include "CMM_Timer.h"    //040521 Yu Wei

//Listen constants
#define TIMEOUT        (-2)    //timeout return value from Send() and Recv()


//Listen class
class CListen
{
	//member functions
  public:
    CListen(char *pcListenIP, unsigned short unListenPort, int nRecvTimeout);
    ~CListen();

    void ListenTask();

    int Send(char *cBuffer, int nSize, int nTimeout);
    int SendSWC(char *cBuffer, int nSize, int nTimeout);
    int Recv(char *cBuffer, int nSize, int nTimeout, int nExact);
    void Close(int);
  private:
    void Listen();
  //  void Close(int);

  //member variables
  private:
    char m_acListenIP[16];      //listen ip
    unsigned short m_unListenPort;  //listen port
    int m_nRecvTimeout;        //receive timeout in ms
    struct tTimerValue m_tsLastRecvOK;  //time when last receive is OK  //040521 Yu Wei
    pthread_mutex_t m_semClientClosed ;    //semaphore to indicate client connection closed


  public:
    int m_nSockClient;        //client socket
    int m_nSockListen;        //listen socket
    bool m_bSockAccept;  // Socket Accept flag
};



#endif

