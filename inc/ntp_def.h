/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   ntp_def.h                                           D1.0.4

 COMPONENT

   NTP - Network Time Protocol for client

 DESCRIPTION

   This file contains the type definitions for NTP client management


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     25-Jan-2010      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef NTP_DEF_H
#define NTP_DEF_H

// total byte for input command size from message queue
#define NTP_MSQ_INPUT_CMD_SZ   100


typedef struct ntp_server_config_st{
  CHAR   ipaddr[16];
  BOOL_T isConnectionGood;
  /*
  file descriptor,
    TYP_ERROR when connection is NG. Retry and recycle for nfd
    TYP_NULL when ipaddr is zero. No retry and no recycle for nfd
    +ve value. Valid file descriptor. No retry and no recycle for nfd
  */
  INT32  nfd;
}NTP_SERVER_CONFIG_T;

typedef enum e_ntp_query_label{
  E_NTP_MCP_CMD_Reserved_0,
  E_NTP_MCP_CMD_RequestForStatus,
  E_NTP_MCP_CMD_EndOfList
}E_NTP_MCP_CMD;

typedef struct ntp_msgq_reply_status_st{
  INT8 activeServer;  // activer server index (0 to (CGF_NTP_TOTAL_SERVER - 1))
                      // TYP_ERROR when there is no active server
  NTP_SERVER_CONFIG_T server[CGF_NTP_TOTAL_SERVER];
} NTP_MSGQ_MCP_STATUS;

#define NTP_SERVER_PORT   123
#define NTP_GETFILEDESC_TIMEOUT  500

// NTP version number
#define NTP_VN           3

/* 1970 - 1900 in seconds */
#define NTP_JAN_1970 2208988800UL

typedef union pulse_msg_u{
  struct _pulse pulse;
  /* your other message structures would go
     here too */
} NTP_CH_MSG_T;

struct fixedpt32_st{
  UINT16 integer_part;
  UINT16 fraction_part;
};

struct fixedpt64_st {
 UINT32 integer_part;
 UINT32 fraction_part;
};


typedef struct ntp_protocol_st{
  union{
    struct flags_bit_st{
      UINT8 mode: 2;
      UINT8 versionNo: 3;
      UINT8 leapIndicator: 2;
    }bitctrl;
    UINT8 allbits;
  }flags;
  UINT8 peerClkStranum;
  UINT8 peerPollingInterval;
  INT8 peerClockPrecision;
  struct fixedpt32_st rootDelay;
  struct fixedpt32_st rootDispersion;
  UINT32 referenceClockID;
  struct fixedpt64_st referenceClockUpdateTime;
  struct fixedpt64_st originateTimeStamp;
  struct fixedpt64_st receiveTimeStamp;
  struct fixedpt64_st transmitTimeStamp;
} NTP_PACKET_T;



typedef struct ntp_control_blk_st{

  NTP_SERVER_CONFIG_T server[CGF_NTP_TOTAL_SERVER];
  NTP_SERVER_CONFIG_T *pactiveserver;
  INT32  npolling_channelID;
  timer_t polling_timer_id;
  // spurious time or tolerance in millisecond.
  UINT32 threshold;
  // no of attempts to recover connection to SNTP server
  UINT8  connectRetry;
  UINT32 notation;
  // polling frequency in second (1 - 3600 seconds)
  UINT32 poll_frequency;
  // retry in seconds after link down (1 - 3600 seconds)
  UINT32 linkDownAttempt;
} NTP_CB;

/*----------------------------------------------------------------------------
NTP_MASTER_IP=10.1.1.12;   NTP master Server, 0 no NTP server
NTP_SLAVE_IP=192.168.0.12;  NTP slave Server, 0 no NTP server
NTP_BACKUP_IP=175.168.0.12;  NTP backup Server, 0 no NTP server
NTP_SPURIOUS_TIME=1000;    ms
NTP_RETRY=5;               retry counter to connect NTP Server
NTP_NOTATION=1;

NTP_POLLING=30;            in second, Range 1 s to 1 hour
NTP_LINKDOWN_RETRY=60;

 RFC Section 4: NTP packet block definition

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |LI | VN  | Mode|   Stratum     |     Poll      |  Precision    |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                   Synchronizing Distance                      |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                  Synchronizing Dispersion                     |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                 Reference Clock Identifier                    |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    |                Reference Timestamp (64 bits)                  |
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    |                Originate Timestamp (64 bits)                  |
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    |                Receive Timestamp (64 bits)                    |
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    |               Transmit Timestamp (64 bits)                    |
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                 Key Identifier (optional) (32)                |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    |                                                               |
    |                 Message Digest (optional) (128)               |
    |                                                               |
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

----------------------------------------------------------------------------*/

#endif /* NTP_DEF_H */
