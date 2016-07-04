/*----------------------------------------------------------------------------

            Copyright (c) 2009 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   err_def.h                                                D1.0.4

 COMPONENT

   n/a

 DESCRIPTION

   This file contains the type definitions for error messages


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     19-Aug-2009      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef ERR_DEF_H
#define ERR_DEF_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum system_error_enum{
  // General
  E_ERR_Success,                          /*    0 */
  E_ERR_InvalidNullPointer,
  E_ERR_InvalidParameter,
  E_ERR_InvalidFileSize,
  E_ERR_ExceedBufferSize,
  E_ERR_OutputBufferSizeUnderrun,
  E_ERR_CurrentlyNotSupporting,
  E_ERR_ApplicationExit,

  // SYS
  E_ERR_SYS_ThrdAttrInitFail,
  E_ERR_SYS_ThrdAttrSetInheritSchdFail,
  E_ERR_SYS_ThrdAttrSetSchdPolicyFail,
  E_ERR_SYS_ThrdAttrSetStackSzFail,
  E_ERR_SYS_ThrdAttrSetSchdParamFail,
  E_ERR_SYS_CreateThrdFail,
  E_ERR_SYS_InvalidThrdID,
  E_ERR_SYS_InvalidThrdLabel,
  E_ERR_SYS_InvalidMutexID,
  E_ERR_SYS_InvalidMsgQLabel,
  E_ERR_SYS_InvalidChannelLabel,
  E_ERR_SYS_InvalidMessageQueueID,
  E_ERR_SYS_InvalidConfigFile,
  E_ERR_SYS_OpenFileFail,
  E_ERR_SYS_InvalidMessageQueueHandle,
  E_ERR_SYS_WriteMsgQueueFail,
  E_ERR_SYS_InvalidYear,
  E_ERR_SYS_InvalidMonth,
  E_ERR_SYS_InvalidDay,
  E_ERR_SYS_InvalidHour,
  E_ERR_SYS_InvalidMinute,
  E_ERR_SYS_InvalidSecond,
  E_ERR_SYS_ClockGetTimeFail,
  E_ERR_SYS_GetLocalTimeFail,
  E_ERR_SYS_ClockSetTimeFail,
  E_ERR_SYS_UpdateHardwareRTCFail,
  E_ERR_SYS_LoadHardwareRTCFail,
  E_ERR_SYS_InvalidGetNetworkParams,
  E_ERR_SYS_ReadWriteFPGABusFail,
  E_ERR_SYS_InvalidHeapCreateHandle,
  E_ERR_SYS_InvalidMemoryAllocSize,
  E_ERR_SYS_FailToCreateRTUFolder,
  E_ERR_SYS_FailToChangeDirectory,
  E_ERR_SYS_FailToCreateSubDirectory,
  E_ERR_SYS_CreateSocketFail,
  E_ERR_SYS_SetIPAddressFail,
  E_ERR_SYS_SetSubnetMaskFail,
  E_ERR_SYS_ConvertIPStringFail,
  E_ERR_SYS_InvalidNetworkNumber4,
  E_ERR_SYS_InvalidNetworkNumber1,
  E_ERR_SYS_InvalidPortIDNumber,
  E_ERR_SYS_InvalidIPAddr,
  E_ERR_SYS_InvalidSubnetMask,
  E_ERR_SYS_InvalidIPType,
  E_ERR_SYS_HeaderSizeExceedBufferSz,
  E_ERR_SYS_GetConfigFileDownloadTimeFail,
  E_ERR_SYS_InvalidTimeType,
  E_ERR_SYS_StartSecGreaterThanEndSec,
  E_ERR_SYS_DividedByZeroException,
  E_ERR_SYS_InvalidTotalComPortPerCard,
  E_ERR_SYS_InvalidDevc8250IrqNumber,
  E_ERR_SYS_InvalidDevc8250IndexNumber,
  E_ERR_SYS_InvalidProcessID,
  E_ERR_SYS_InvalidFilename,
  E_ERR_SYS_FileOpenError,
  E_ERR_SYS_ReadFileError,

  // Serial Link Management
  E_ERR_SER_ExceedTheExpectedSize,
  E_ERR_SER_OpenPortFail,
  E_ERR_SER_OpenWriteError,
  E_ERR_SER_WriteCharError,
  E_ERR_SER_InvalidComPortSelection,
  E_ERR_SER_GetAttributeFail,
  E_ERR_SER_InvalidFileDescriptor,
  E_ERR_SER_FailToWriteDevice,
  E_ERR_SER_SelectReadTimeOut,
  E_ERR_SER_SelectReadError,
  E_ERR_SER_InvalidSelectReturnValue,
  E_ERR_SER_FailToReadDevice,
  E_ERR_SER_SetInputBaudRateFail,
  E_ERR_SER_SetOutputBaudRateFail,
  E_ERR_SER_InvalidDataBitsSelection,
  E_ERR_SER_InvalidStopbitSelection,
  E_ERR_SER_SetAttributeFail,
  E_ERR_SER_SerialCOMPortOutOfRange,


  // SCPI Parser Management (SPA)
  E_ERR_SPA_CreateThreadFail,
  E_ERR_SPA_InvalidCmdLength,
  E_ERR_SPA_NotSupportNullTerminatedCmd,
  E_ERR_SPA_CommandNotFound,
  E_ERR_SPA_SharedLibHandleNotFound,
  E_ERR_SPA_SharedLibCBOverflow,
  E_ERR_SPA_SharedLibOpenFail,
  E_ERR_SPA_SharedLibCloseFail,
  E_ERR_SPA_SharedLibFuncNotFound,

  // Redundancy Management Module
  E_ERR_RMM_UndefinedState,
  E_ERR_RMM_UndefinedStatusTableCode,
  E_ERR_RMM_SendMsgQueueError,
  E_ERR_RMM_InvalidLanID,
  E_ERR_RMM_SWCIndexNotEnable,


  // Maintenance Management Module (MMM)
  E_ERR_MMM_CreateListenSocketFail,
  E_ERR_MMM_SetSocketReuseAddressFail,
  E_ERR_MMM_BindSocketFail,
  E_ERR_MMM_ListenSocketFail,
  E_ERR_MMM_AcceptClientSocketFail,
  E_ERR_MMM_RecvClientCommandFail,
  E_ERR_MMM_LoginFail,
  E_ERR_MMM_ClientLoginTimeout,
  E_ERR_MMM_InvalidUsernamePassword,
  E_ERR_MMM_InvalidStateForTest,
  E_ERR_MMM_InvalidSerialComFd,
  E_ERR_MMM_TxAndRxCharNotMatch,


  // STR
  E_ERR_STR_SizeExceedLimit,
  E_ERR_STR_InvalidStringLength,
  E_ERR_STR_InvalidTargetBufferSize,
  E_ERR_STR_AppendCharacterNotFound,
  E_ERR_STR_AppendStringSizeOverflow,
  E_ERR_STR_InvalidNullCharacterInString,
  E_ERR_STR_InvalidHexCharacter,
  E_ERR_STR_InvalidDataWidth,
  E_ERR_STR_InvalidConversionType,
  E_ERR_STR_InvalidNumOfSample,
  E_ERR_STR_InvalidDataSize,
  E_ERR_STR_InvalidDecodeByteSize,
  E_ERR_STR_OutputOverrunCumulativeBuffer,
  E_ERR_STR_InvalidMemorySize,
  E_ERR_STR_InvalidParameterSize,

  // Init Module
  E_ERR_INM_SWCPollingTableOverlapOrOutOfRange,
  E_ERR_INM_GetIntegerValueFail,
  E_ERR_INM_SearchKeyWordFail,
  E_ERR_INM_GetValueFail,
  E_ERR_INM_SearchMultiValueFail,
  E_ERR_INM_GetDigitFail,
  E_ERR_INM_GetRTUConfigFail,
  E_ERR_INM_GetAllRTUTableFail,
  E_ERR_INM_GetServerDefFail,
  E_ERR_INM_GetAllSWCParamterFail,
  E_ERR_INM_InvalidVersionNumber,
  E_ERR_INM_UndefinedClockProtocol,
  E_ERR_INM_TimeOutIsGreaterThanFastPollingFrequency,
  E_ERR_INM_InvalidModbusPollFastSWC,
  E_ERR_INM_InvalidModbusPollSlowSWC,
  E_ERR_INM_InvalidModbusStandbyPollingFreq,
  E_ERR_INM_InvalidModbusFailedPollingFreq,
  E_ERR_INM_InvalidModbusExceptionRetry,
  E_ERR_INM_InvalidGenericSpuriousTime,
  E_ERR_INM_InvalidGraceTime,
  E_ERR_INM_InvalidClkTimeOut,
  E_ERR_INM_InvalidClkRetry,

  E_ERR_INM_InvalidNTPSpuriousTime,
  E_ERR_INM_InvalidNTPRetry,
  E_ERR_INM_InvalidNTPPolling,

  // Log
  E_ERR_LOG_OpenDirectoryFail,
  E_ERR_LOG_OpenFileFail,

  // Timer
  E_ERR_TMR_CreateThreadFail,

  // Modbus Management
  E_ERR_MDB_InvalidSlaveAddress,

  // Hardware Peripheral (HWP)
  E_ERR_HWP_InitIoThreadFail,
  E_ERR_HWP_InvalidIOType,
  E_ERR_HWP_MmapPortInvalidHandle,
  E_ERR_HWP_LedIDExceedDefinedNumber,
  E_ERR_HWP_InvalidLedOnOffStatus,
  E_ERR_HWP_UnMmapPortFail,
  E_ERR_HWP_PciServerInitializationFail,
  E_ERR_HWP_LCM_ExceedValidBrightnessRange,
  E_ERR_HWP_LCM_InvalidPrnChar,
  E_ERR_HWP_LCM_RowPositionOutOfRange,
  E_ERR_HWP_LCM_InvalidStartPosition,
  E_ERR_HWP_LCM_InvalidMessageType,

  // LCM
  E_ERR_LCM_GetAttributeFail,
  E_ERR_LCM_SetAttributeFail,


  // FLG
  E_ERR_FLG_MessageSizeOutOfRange,
  E_ERR_FLG_TimeStringExceedDefinedBufferSize,
  E_ERR_FLG_MsgQueueSendFail,
  E_ERR_FLG_LogFileTypeNotSupported,

  // NTP


  // LNC
  E_ERR_LNC_FailToWriteSocket,
  E_ERR_LNC_FailToReadSocket,
  E_ERR_LNC_SelectReadTimeOut,
  E_ERR_LNC_SelectReadError,
  E_ERR_LNC_InvalidSocketType,
  E_ERR_LNC_FailToCreateSocket,
  E_ERR_LNC_FailToConnectDatagramSocket,
  E_ERR_LNC_FailToConnectStreamSocket,
  E_ERR_LNC_InvalidFileDescriptor,
  E_ERR_LNC_InvalidIoctlForIPAddr,
  E_ERR_LNC_InvalidIoctlForNetMask,
  E_ERR_LNC_AddGatewayFailToCreateSocket,
  E_ERR_LNC_AddGatewayUpdateRouteTableFail,
  E_ERR_LNC_NotSupportedSocket,

  // CGF
  E_ERR_CGF_SearchKeyWordFail,

  // SWC
  E_ERR_SWC_InitializationFail,
  E_ERR_SWC_ExceedRetryLimit,
  E_ERR_SWC_ExceedExceptionRetryLimit,
  E_ERR_SWC_ExceedBufferSize,
  E_ERR_SWC_NoResponseByte,
  E_ERR_SWC_UnhandleModbusError,
  E_ERR_SWC_ModbusCRCError,
  E_ERR_SWC_ModbusExceptionError,
  E_ERR_SWC_MsgQReplyError,
  E_ERR_SWC_InvalidFileDescriptor,


  // ServerPoll
  E_ERR_SPM_InitializationFail,

  // Development
  E_ERR_DEV_OpenFileFail,


  E_ERR_EndOfList
} E_ERR_T;

#define ERR_STR_SZ         100
#define ERR_STR(errcode)   #errcode
typedef struct err_lut_st{
  E_ERR_T label;
  CHAR desc[ERR_STR_SZ];
} ERR_LUT_T;

#ifdef __cplusplus
}
#endif

#endif /* ERR_DEF_H */
