#ifndef SWC_LAN_H_
#define SWC_LAN_H_

#include "SWC.h"

#define SWC_LAN_WRITE_BIT_REGISTER_OFFSET 400

#define DI_MAX_RECORD 256
struct t_DITableRecord
{
  unsigned short usIndex;//record number currently read
  unsigned short usValue; //record number currently added
  unsigned short usHourMin; //total record number, <= PLC_DI_TABLE_MAX
  unsigned short usSecMsec;
};

class CSWC_LAN : public CSWC
{
public:

  char m_LanSWC_IP[20];
  unsigned short m_tLanSWC_SockID;
  int m_nModeBusSeqence; // Not use
  unsigned short m_CommandWordAddress;  // Not use
  char m_acWriteStatusCommand[2][16];

  unsigned short m_nFastPollingAllDataRecevLen;
  char m_acFastPollingAllDataCommand[2][16];

  t_DITableRecord m_tDiRecordQueue[DI_MAX_RECORD]; //Create a FIFO queue
  int m_nRecordStartPoint;
  int m_nRecordEndPoint;
  int m_nNumberOfRecordInQueue;

  /*--------------------------------------------------------------------------
    Packet transaction number. Append this 16-bit value to the header of 
    a packet before sending out
  --------------------------------------------------------------------------*/
  UINT16 m_usPacketTransactionNum;  // packet transaction number

public:
   CSWC_LAN(struct tSWCCFGStructure,int, char * );
  ~CSWC_LAN();



   void FastPolling(void);
   void PrimaryLinkCheck(void);
   void SlowPolling(void);
   void TimerPolling(void);
   void TimeSynchronization(void);
   void KeepAlive(void);
   void LinkCheck(void);

   // return ERROR or set Para success
   int SetSendReplyPara(int nLinkID, int nRetryNumber); 
   void MainProcess(void);


   void CheckLinkStatusChange(void);


   void SetReadCommand(char *,char , int  , char , tModbusAddress );
   int SetWriteCommand(char *,char *,char ,char ,tModbusAddress );

   void LinkMonitor(void);
   void SetTimeSynCommand(unsigned short unReady);
   int DecodeFastData(char *);
   int WriteCommandWordToPLC(unsigned short );
   int PutAllDiRecordToQueue(char *);
   
   int SpecificCommandSWCError(char *);
   void ServerCommand(char *, int);
   void UpdateCommand(char *, int, char);
   int ReadSWCTable(unsigned short *aunServerTable, int nTimeoutType);
   int SpecificCommand(char *acReceData, int nReceDataLen);
   E_ERR_T RequestAndReceiveWithModbusCheck(INT32 fd,
               UINT8 *ptxcmd,
               UINT8 txcmdsz,
               VOID *prxbuff,
               UINT32 expectedRxSz,
               INT32 timeout_ms,
               UINT8 retry,
               UINT8 exceptionRetry,
               UINT8 *poutexceptionRetryCnt);

private:
  E_ERR_T getConnectFd(UINT32 nlinkID);
  
};

#endif /*SWC_LAN_H_*/
