/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   flg_def.h                                          D1.0.4

 COMPONENT

   n/a

 DESCRIPTION

   This file contains the type definitions and declaration for file logging
   component


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     14-Jan-2010      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef FLG_DEF_H
#define FLG_DEF_H

// Log buffer size use in SendMessage routine. The defined size is less than
// or equal to message queue size, mqstat.mq_msgsize.
#define FLG_LOG_BUFFER_SZ 1024

typedef enum e_header_type{
  FLG_HDR_T_NoTimeStamp,
  FLG_HDR_T_TimeStamp_Time,
  FLG_HDR_T_TimeStamp_DateAndTime,
  FLG_HDR_T_EndOfList
} FLG_HDR_T;

typedef enum e_second_type{
  FLG_SEC_T_MilliSec,
  FLG_SEC_T_MicroSec,
  FLG_SEC_T_NanoSec,
  FLG_SEC_T_EndOfList
} FLG_SEC_T;

typedef enum e_flg_tag{
  E_FLG_Reserved_0,

  E_FLG_EndOfList
} E_FLG;

struct register_record_st{
  E_FLG regtag;

};

// Log file type
typedef enum e_flg_logfile_t{
  FLG_LOG_T_Reserved_0,
  FLG_LOG_T_Debug,
  FLG_LOG_T_Event,
  FLG_LOG_T_System,
  FLG_LOG_T_EndOfList
} FLG_LOG_T;

// Note: The sequence definition must be consistent with message_bit_st below
typedef enum e_flg_message_t{
  E_FLG_MSG_SystemLevel,
  E_FLG_MSG_INM_ConfigFileErr,
  E_FLG_MSG_SWC_LAN_Init,
  E_FLG_MSG_SWC_LAN_PrimaryLinkCheck,
  E_FLG_MSG_SWC_LAN_FastPolling,
  E_FLG_MSG_SWC_LAN_SlowPolling,
  E_FLG_MSG_SWC_LAN_TimerPolling,
  E_FLG_MSG_SWC_LAN_TimeSynchronization,
  E_FLG_MSG_SWC_LAN_KeepAlivePolling,
  E_FLG_MSG_SWC_LAN_StbyLinkCheck,
  E_FLG_MSG_SWC_LAN_LinkMonitor,  
  E_FLG_MSG_SWC_LAN_SpecificCommand,
  E_FLG_MSG_SWC_LAN_TransNumberMismatch,
  
  E_FLG_MSG_SWC_Serial_PrimaryLinkCheck,
  E_FLG_MSG_SWC_Serial_FastPolling,
  E_FLG_MSG_SWC_Serial_SlowPolling,
  E_FLG_MSG_SWC_Serial_TimerPolling,
  E_FLG_MSG_SWC_Serial_TimeSynchronization,
  E_FLG_MSG_SWC_Serial_UnhandledError,
  E_FLG_MSG_SWC_Serial_ModbusError,
  E_FLG_MSG_SWC_SER_ServerCommand,
  E_FLG_MSG_SWC_Serial_Link,
  E_FLG_MSG_RMM_ServerCommand,
  E_FLG_MSG_RMM_SystemAlert,
  E_FLG_MSG_RMM_StateChange,
  E_FLG_MSG_SPM_ServerLink,
  E_FLG_MSG_SPM_TxRtuStatusTbl,
  E_FLG_MSG_WDG_Timeout,
  E_FLG_MSG_CMM_Listen,
  E_FLG_MSG_EndOfList
}FLG_MSG_T;

typedef struct flg_control_block_st{
  BOOL_T isInit;
  // log temperature alert only when temperature changes
  BOOL_T isOverTemperatureLog;
  
  FLG_MSG_T msgtype;

  union{
    // The sequence definition is consistent with FLG_MSG_T above
    // Note: Make sure a msgXX is deleted when new field is added to
    //       mantain 32 fields
    struct message_bit_st{
      UINT32 systemlevel: 1;
      UINT32 inm_cfg: 1;

      UINT32 swc_lan_init: 1;
      UINT32 swc_lan_primarylinkcheck: 1;      
      UINT32 swc_lan_fastpolling: 1;
      UINT32 swc_lan_slowpolling: 1;
      UINT32 swc_lan_timerpolling: 1;
      UINT32 swc_lan_timesync: 1;
      UINT32 swc_lan_keepalivepolling: 1; 
      UINT32 swc_lan_stbylinkcheck: 1;
      UINT32 swc_lan_linkmonitor: 1;      
      UINT32 swc_lan_specificcmd: 1;
      UINT32 swc_lan_trsnummismatch: 1;
      
      UINT32 swc_ser_primarylinkcheck: 1;
      UINT32 swc_ser_fastpolling: 1;
      UINT32 swc_ser_slowpolling: 1;
      UINT32 swc_ser_timerpolling: 1;      
      UINT32 swc_ser_timesync: 1;  
      UINT32 swc_ser_unhandlederror: 1;
      UINT32 swc_ser_modbuserror: 1;
      UINT32 swc_ser_servercmd: 1;
      UINT32 swc_ser_link: 1;

      UINT32 rmm_servercmd: 1;
      UINT32 rmm_systemalert: 1;
      UINT32 rmm_statechange: 1;

      UINT32 spm_serverlink: 1;
      UINT32 spm_txrtustatustbl: 1;
      UINT32 wdg_timeout: 1;
      
      UINT32 cmm_listen: 1;
      UINT32 msg29: 1;
      UINT32 msg30: 1;
      UINT32 msg31: 1;
      UINT32 msg32: 1;
    }msgbitctrl;
    UINT32 allbits;
  }msgctrl;
  
  
} FLG_CB;

#define FLG_DIRECTORYPATH_SZ 100
#define FLG_FILENAME_SZ      80
#define FLG_MSGQ_RX_BUFF_SZ  1024

class FLG
{

private:
  
  pthread_mutex_t mtx_syslogfile;
  CHAR directorypath[FLG_DIRECTORYPATH_SZ];
  CHAR filename[FLG_FILENAME_SZ];
  CHAR pathfilename[FLG_DIRECTORYPATH_SZ + FLG_FILENAME_SZ];
  E_ERR_T flg_createTimeStampString(const FLG_HDR_T hdr_type,
                                   const FLG_SEC_T sec_type,
                                   CHAR *poutstr, UINT8 outstrbuffsz);
public:

  // File logging control block
  FLG_CB CtrlBlk;

  // management routines
  FLG(const CHAR *pdirectorypath, const CHAR *pfilename);
  ~FLG(VOID);
  E_ERR_T SendMessage(FLG_MSG_T msgtype, const CHAR *pformat, ...);
  E_ERR_T SendMessage(const FLG_MSG_T msgtype, const FLG_HDR_T hdr_type,
                     const FLG_SEC_T sec_type, const CHAR *pformat, ...);
  E_ERR_T SendMessage(const FLG_MSG_T msgtype, const FLG_HDR_T hdr_type,
                     const FLG_SEC_T sec_type, const FLG_LOG_T logfiletype,
                     const CHAR *pformat, ...);
  E_ERR_T SendDataBlock(FLG_MSG_T msgtype, const UINT8 *pbuff,
                       const UINT32 logsz, const UINT8 byteperrow);
  E_ERR_T SendDataBlock(FLG_MSG_T msgtype, const FLG_LOG_T logfiletype, 
                       const UINT8 *pbuff, const UINT32 logsz, 
                       const UINT8 byteperrow);                         
  E_ERR_T WriteMsgToFile(CHAR *pmsg);


}; // FLG


#endif /* FLG_DEF_H */
