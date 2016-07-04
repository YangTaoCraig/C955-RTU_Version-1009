#ifndef _RMM_DEF_H_
#define _RMM_DEF_H_
#ifdef __cplusplus
extern "C"
{
#endif

typedef enum e_status_tbl{
  E_RMM_STATUSTBL_Reserved,
  E_RMM_STATUSTBL_PrimaryToStandby = 0x0018,
  E_RMM_STATUSTBL_StandbyToPrimary = 0x0028,
  E_RMM_STATUSTBL_Primary          = 0x0010,
  E_RMM_STATUSTBL_Standby          = 0x0020,
  E_RMM_STATUSTBL_InitToStandby    = 0x0021,
  E_RMM_STATUSTBL_Initialization   = 0x0001,
  E_RMM_STATUSTBL_EndOfList
} E_RMM_STATUSTBL;
//The order is set for VALIDAION_BIT == 0x0008 if need to change back to 0x8000, pls reverse the change.
typedef union commStatusSwc_u{
  struct flags_bit_st{
    UINT16 isLine1InUsed: 1;
    UINT16 isLine2InUsed: 1;
    UINT16 isRTUToExternalSystemLinkEnable: 1;
    UINT16 communicationIsValid: 1; ///UINT16 reserved03: 1; Yang Tao 20150331 Switch the Bit 15 to Bit 3
    UINT16 isLine1Healthy: 1;
    UINT16 isLine2Healthy: 1;
    UINT16 reserved06: 1;
    UINT16 reserved07: 1;
    UINT16 isLine1MasterLink: 1;
    UINT16 isLine2SlaveLink: 1;
    UINT16 reserved10: 1;
    UINT16 reserved11: 1;
    UINT16 isSerialLinkType: 1;
    UINT16 isLANLinkType: 1;
    UINT16 isNoExceptionErrorFound: 1;
    UINT16 reserved03: 1;
  }bitctrl;

  UINT16 allbits;
} RMM_RTU_SWC_COMM_HEALTH;


typedef union rtu_health_status_table_st{
  struct flags_bit_st{
    UINT16 wasReset: 1; // 0: RTU just been reset within 10s
    UINT16 isOverTemperature: 1;//1
    UINT16 downloadIsRequires: 1;//2
    UINT16 reserved_3: 1;//3 RTU is swithing
    UINT16 isPrimaryRTU: 1;//4
    UINT16 isStandbyRTU: 1;//5
    UINT16 compactFlashCardIsBelowMin: 1;//6
    UINT16 isLANLinkHealthy: 1;//7: is the LAN link is healthy
    UINT16 isRTUToRTULan2Healthy: 1;//8 RTU to RTU LAN 2 is healthy
    UINT16 isRTUToRTULan1Healthy: 1;// 9 RTU to RTU LAN 1 (LAN 4) is healthy
    UINT16 isRMTConnected: 1;// 10
    UINT16 isRTUServerConnectedAtID1: 1;// 11
    UINT16 isRTUServerConnectedAtID2: 1;// 12
    UINT16 isRTUServerConnectedAtID3: 1;// 13
    UINT16 isRTUServerConnectedAtID4: 1;// 14
    UINT16 isRTUServerConnectedAtID5: 1;// 15
  }bitctrl;
  UINT16 allbits;
} RMM_RTU_HEALTH;

typedef struct rtu_status_table_st{
  UINT16 utcTimeStamp[3]; //6 Byte UTC
  UINT16 identification;  // 1 RTU identification
  UINT16 location[2];	//2&3 RTU Location (H) and (L)
  RMM_RTU_HEALTH healthStatus; //4 :RTU Status
  UINT16 swcNotation;// 5: External system notation
  UINT16 isSwcLinkCheckEnable; // 6 link check flag
  UINT16 configFileVersion;//7 RTU config file version
  UINT16 configFileUTC[2];// 8&9 UTC of config file (H) and (L)
  UINT16 softwareVersion; // 10 softwareVersion
  RMM_RTU_SWC_COMM_HEALTH swcCommStatus[CFG_TOTAL_NUM_OF_SWC];
} RMM_RTU_STATUS_TBL;

#ifdef __cplusplus
}
#endif

#endif //_RMM_DEF_H_
