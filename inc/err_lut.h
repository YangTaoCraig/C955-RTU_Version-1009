/*----------------------------------------------------------------------------

            Copyright (c) 2009 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   err_lut.h                                          D1.0.4

 COMPONENT

   n/a

 DESCRIPTION

   This file contains the private error look-up table


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     15-Sep-2009      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef _ERR_LUT_H_
#define _ERR_LUT_H_

static const ERR_LUT_T ERR_ErrorLut[E_ERR_EndOfList + 1] =
{
  {E_ERR_Success, ERR_STR(E_ERR_Success)},
  {E_ERR_InvalidNullPointer, ERR_STR(E_ERR_InvalidNullPointer)},
  {E_ERR_InvalidParameter, ERR_STR(E_ERR_InvalidParameter)},
  {E_ERR_InvalidFileSize, ERR_STR(E_ERR_InvalidFileSize)},
  {E_ERR_ExceedBufferSize, ERR_STR(E_ERR_ExceedBufferSize)},
  {E_ERR_OutputBufferSizeUnderrun, ERR_STR(E_ERR_OutputBufferSizeUnderrun)},
  {E_ERR_CurrentlyNotSupporting, ERR_STR(E_ERR_CurrentlyNotSupporting)},
  {E_ERR_ApplicationExit, ERR_STR(E_ERR_ApplicationExit)},

  // SYS
  {E_ERR_SYS_ThrdAttrInitFail, ERR_STR(E_ERR_SYS_ThrdAttrInitFail)},
  {E_ERR_SYS_ThrdAttrSetInheritSchdFail,
     ERR_STR(E_ERR_SYS_ThrdAttrSetInheritSchdFail)},
  {E_ERR_SYS_ThrdAttrSetSchdPolicyFail,
     ERR_STR(E_ERR_SYS_ThrdAttrSetSchdPolicyFail)},
  {E_ERR_SYS_ThrdAttrSetStackSzFail,
     ERR_STR(E_ERR_SYS_ThrdAttrSetStackSzFail)},
  {E_ERR_SYS_ThrdAttrSetSchdParamFail,
     ERR_STR(E_ERR_SYS_ThrdAttrSetSchdParamFail)},
  {E_ERR_SYS_CreateThrdFail, ERR_STR(E_ERR_SYS_CreateThrdFail)},
  {E_ERR_SYS_InvalidThrdID, ERR_STR(E_ERR_SYS_InvalidThrdID)},
  {E_ERR_SYS_InvalidThrdLabel, ERR_STR(E_ERR_SYS_InvalidThrdLabel)},
  {E_ERR_SYS_InvalidMutexID, ERR_STR(E_ERR_SYS_InvalidMutexID)},
  {E_ERR_SYS_InvalidMsgQLabel, ERR_STR(E_ERR_SYS_InvalidMsgQLabel)},
  {E_ERR_SYS_InvalidChannelLabel, ERR_STR(E_ERR_SYS_InvalidChannelLabel)},
  {E_ERR_SYS_InvalidMessageQueueID, ERR_STR(E_ERR_SYS_InvalidMessageQueueID)},
  {E_ERR_SYS_InvalidConfigFile, ERR_STR(E_ERR_SYS_InvalidConfigFile)},
  {E_ERR_SYS_OpenFileFail, ERR_STR(E_ERR_SYS_OpenFileFail)},
  {E_ERR_SYS_InvalidMessageQueueHandle,
     ERR_STR(E_ERR_SYS_InvalidMessageQueueHandle)},
  {E_ERR_SYS_WriteMsgQueueFail, ERR_STR(E_ERR_SYS_WriteMsgQueueFail)},
  {E_ERR_SYS_InvalidYear, ERR_STR(E_ERR_SYS_InvalidYear)},
  {E_ERR_SYS_InvalidMonth, ERR_STR(E_ERR_SYS_InvalidMonth)},
  {E_ERR_SYS_InvalidDay, ERR_STR(E_ERR_SYS_InvalidDay)},
  {E_ERR_SYS_InvalidHour, ERR_STR(E_ERR_SYS_InvalidHour)},
  {E_ERR_SYS_InvalidMinute, ERR_STR(E_ERR_SYS_InvalidMinute)},
  {E_ERR_SYS_InvalidSecond, ERR_STR(E_ERR_SYS_InvalidSecond)},
  {E_ERR_SYS_ClockGetTimeFail, ERR_STR(E_ERR_SYS_ClockGetTimeFail)},
  {E_ERR_SYS_GetLocalTimeFail, ERR_STR(E_ERR_SYS_GetLocalTimeFail)},
  {E_ERR_SYS_ClockSetTimeFail, ERR_STR(E_ERR_SYS_ClockSetTimeFail)},
  {E_ERR_SYS_UpdateHardwareRTCFail, ERR_STR(E_ERR_SYS_UpdateHardwareRTCFail)},
  {E_ERR_SYS_LoadHardwareRTCFail, ERR_STR(E_ERR_SYS_LoadHardwareRTCFail)},
  {E_ERR_SYS_InvalidGetNetworkParams,
     ERR_STR(E_ERR_SYS_InvalidGetNetworkParams)},
  {E_ERR_SYS_ReadWriteFPGABusFail, ERR_STR(E_ERR_SYS_ReadWriteFPGABusFail)},
  {E_ERR_SYS_InvalidHeapCreateHandle,
     ERR_STR(E_ERR_SYS_InvalidHeapCreateHandle)},
  {E_ERR_SYS_InvalidMemoryAllocSize, ERR_STR(E_ERR_SYS_InvalidMemoryAllocSize)},
  {E_ERR_SYS_FailToCreateRTUFolder, ERR_STR(E_ERR_SYS_FailToCreateRTUFolder)},
  {E_ERR_SYS_FailToChangeDirectory, ERR_STR(E_ERR_SYS_FailToChangeDirectory)},
  {E_ERR_SYS_FailToCreateSubDirectory,
     ERR_STR(E_ERR_SYS_FailToCreateSubDirectory)},
  {E_ERR_SYS_CreateSocketFail, ERR_STR(E_ERR_SYS_CreateSocketFail)},
  {E_ERR_SYS_SetIPAddressFail, ERR_STR(E_ERR_SYS_SetIPAddressFail)},
  {E_ERR_SYS_SetSubnetMaskFail, ERR_STR(E_ERR_SYS_SetSubnetMaskFail)},
  {E_ERR_SYS_ConvertIPStringFail, ERR_STR(E_ERR_SYS_ConvertIPStringFail)},
  {E_ERR_SYS_InvalidNetworkNumber4, ERR_STR(E_ERR_SYS_InvalidNetworkNumber4)},
  {E_ERR_SYS_InvalidNetworkNumber1, ERR_STR(E_ERR_SYS_InvalidNetworkNumber1)},
  {E_ERR_SYS_InvalidPortIDNumber, ERR_STR(E_ERR_SYS_InvalidPortIDNumber)},
  {E_ERR_SYS_InvalidIPAddr, ERR_STR(E_ERR_SYS_InvalidIPAddr)},
  {E_ERR_SYS_InvalidSubnetMask, ERR_STR(E_ERR_SYS_InvalidSubnetMask)},
  {E_ERR_SYS_InvalidIPType, ERR_STR(E_ERR_SYS_InvalidIPType)},
  {E_ERR_SYS_HeaderSizeExceedBufferSz,
     ERR_STR(E_ERR_SYS_HeaderSizeExceedBufferSz)},
  {E_ERR_SYS_GetConfigFileDownloadTimeFail,
     ERR_STR(E_ERR_SYS_GetConfigFileDownloadTimeFail)},
  {E_ERR_SYS_InvalidTimeType, ERR_STR(E_ERR_SYS_InvalidTimeType)},
  {E_ERR_SYS_StartSecGreaterThanEndSec,
     ERR_STR(E_ERR_SYS_StartSecGreaterThanEndSec)},
  {E_ERR_SYS_DividedByZeroException, ERR_STR(E_ERR_SYS_DividedByZeroException)},
  {E_ERR_SYS_InvalidTotalComPortPerCard,
     ERR_STR(E_ERR_SYS_InvalidTotalComPortPerCard)},
  {E_ERR_SYS_InvalidDevc8250IrqNumber,
     ERR_STR(E_ERR_SYS_InvalidDevc8250IrqNumber)},
  {E_ERR_SYS_InvalidDevc8250IndexNumber,
     ERR_STR(E_ERR_SYS_InvalidDevc8250IndexNumber)},
  {E_ERR_SYS_InvalidProcessID, ERR_STR(E_ERR_SYS_InvalidProcessID)},
  {E_ERR_SYS_InvalidFilename, ERR_STR(E_ERR_SYS_InvalidFilename)},
  {E_ERR_SYS_FileOpenError, ERR_STR(E_ERR_SYS_FileOpenError)},
  {E_ERR_SYS_ReadFileError, ERR_STR(E_ERR_SYS_ReadFileError)},
  
  // Serial Link Management
  {E_ERR_SER_OpenPortFail, ERR_STR(E_ERR_SER_OpenPortFail)},
  {E_ERR_SER_OpenWriteError, ERR_STR(E_ERR_SER_OpenWriteError)},
  {E_ERR_SER_WriteCharError, ERR_STR(E_ERR_SER_WriteCharError)},
  {E_ERR_SER_InvalidComPortSelection,
     ERR_STR(E_ERR_SER_InvalidComPortSelection)},
  {E_ERR_SER_GetAttributeFail, ERR_STR(E_ERR_SER_GetAttributeFail)},
  {E_ERR_SER_InvalidFileDescriptor, ERR_STR(E_ERR_SER_InvalidFileDescriptor)},
  {E_ERR_SER_FailToWriteDevice, ERR_STR(E_ERR_SER_FailToWriteDevice)},
  {E_ERR_SER_SelectReadTimeOut, ERR_STR(E_ERR_SER_SelectReadTimeOut)},
  {E_ERR_SER_SelectReadError, ERR_STR(E_ERR_SER_SelectReadError)},
  {E_ERR_SER_InvalidSelectReturnValue,
     ERR_STR(E_ERR_SER_InvalidSelectReturnValue)},
  {E_ERR_SER_FailToReadDevice, ERR_STR(E_ERR_SER_FailToReadDevice)},
  {E_ERR_SER_SetInputBaudRateFail, ERR_STR(E_ERR_SER_SetInputBaudRateFail)},
  {E_ERR_SER_SetOutputBaudRateFail, ERR_STR(E_ERR_SER_SetOutputBaudRateFail)},
  {E_ERR_SER_InvalidDataBitsSelection,
     ERR_STR(E_ERR_SER_InvalidDataBitsSelection)},
  {E_ERR_SER_InvalidStopbitSelection,
     ERR_STR(E_ERR_SER_InvalidStopbitSelection)},
  {E_ERR_SER_SetAttributeFail, ERR_STR(E_ERR_SER_SetAttributeFail)},
  {E_ERR_SER_SerialCOMPortOutOfRange,
     ERR_STR(E_ERR_SER_SerialCOMPortOutOfRange)},

  // SCPI Parser Management (SPA)
  {E_ERR_SPA_CreateThreadFail, ERR_STR(E_ERR_SPA_CreateThreadFail)},
  {E_ERR_SPA_InvalidCmdLength, ERR_STR(E_ERR_SPA_InvalidCmdLength)},
  {E_ERR_SPA_NotSupportNullTerminatedCmd,
     ERR_STR(E_ERR_SPA_NotSupportNullTerminatedCmd)},
  {E_ERR_SPA_CommandNotFound, ERR_STR(E_ERR_SPA_CommandNotFound)},
  {E_ERR_SPA_SharedLibHandleNotFound, 
     ERR_STR(E_ERR_SPA_SharedLibHandleNotFound)},
  {E_ERR_SPA_SharedLibCBOverflow, ERR_STR(E_ERR_SPA_SharedLibCBOverflow)},
  {E_ERR_SPA_SharedLibOpenFail, ERR_STR(E_ERR_SPA_SharedLibOpenFail)},
  {E_ERR_SPA_SharedLibCloseFail, ERR_STR(E_ERR_SPA_SharedLibCloseFail)},
  {E_ERR_SPA_SharedLibFuncNotFound, ERR_STR(E_ERR_SPA_SharedLibFuncNotFound)},

  // Redundancy Management Module (RMM)
  {E_ERR_RMM_UndefinedState, ERR_STR(E_ERR_SPA_CommandNotFound)},
  {E_ERR_RMM_UndefinedStatusTableCode,
     ERR_STR(E_ERR_RMM_UndefinedStatusTableCode)},
  {E_ERR_RMM_SendMsgQueueError, ERR_STR(E_ERR_RMM_SendMsgQueueError)},
  {E_ERR_RMM_InvalidLanID, ERR_STR(E_ERR_RMM_InvalidLanID)},
  {E_ERR_RMM_SWCIndexNotEnable, ERR_STR(E_ERR_RMM_SWCIndexNotEnable)},

  // Maintenance Management Module (MMM)
  {E_ERR_MMM_CreateListenSocketFail, ERR_STR(E_ERR_MMM_CreateListenSocketFail)},
  {E_ERR_MMM_SetSocketReuseAddressFail,
     ERR_STR(E_ERR_MMM_SetSocketReuseAddressFail)},
  {E_ERR_MMM_BindSocketFail, ERR_STR(E_ERR_MMM_BindSocketFail)},
  {E_ERR_MMM_ListenSocketFail, ERR_STR(E_ERR_MMM_ListenSocketFail)},
  {E_ERR_MMM_AcceptClientSocketFail, ERR_STR(E_ERR_MMM_AcceptClientSocketFail)},
  {E_ERR_MMM_RecvClientCommandFail, ERR_STR(E_ERR_MMM_RecvClientCommandFail)},
  {E_ERR_MMM_LoginFail, ERR_STR(E_ERR_MMM_LoginFail)},
  {E_ERR_MMM_ClientLoginTimeout, ERR_STR(E_ERR_MMM_ClientLoginTimeout)},
  {E_ERR_MMM_InvalidUsernamePassword,
     ERR_STR(E_ERR_MMM_InvalidUsernamePassword)},
  {E_ERR_MMM_InvalidStateForTest, ERR_STR(E_ERR_MMM_InvalidStateForTest)},
  {E_ERR_MMM_InvalidSerialComFd, ERR_STR(E_ERR_MMM_InvalidSerialComFd)},
  {E_ERR_MMM_TxAndRxCharNotMatch, ERR_STR(E_ERR_MMM_TxAndRxCharNotMatch)},

  // STR
  {E_ERR_STR_SizeExceedLimit, ERR_STR(E_ERR_STR_SizeExceedLimit)},
  {E_ERR_STR_InvalidStringLength, ERR_STR(E_ERR_STR_InvalidStringLength)},
  {E_ERR_STR_InvalidTargetBufferSize,
     ERR_STR(E_ERR_STR_InvalidTargetBufferSize)},
  {E_ERR_STR_AppendCharacterNotFound,
     ERR_STR(E_ERR_STR_AppendCharacterNotFound)},
  {E_ERR_STR_AppendStringSizeOverflow,
     ERR_STR(E_ERR_STR_AppendStringSizeOverflow)},
  {E_ERR_STR_InvalidNullCharacterInString,
     ERR_STR(E_ERR_STR_InvalidNullCharacterInString)},
  {E_ERR_STR_InvalidHexCharacter, ERR_STR(E_ERR_STR_InvalidHexCharacter)},
  {E_ERR_STR_InvalidDataWidth, ERR_STR(E_ERR_STR_InvalidDataWidth)},
  {E_ERR_STR_InvalidConversionType, ERR_STR(E_ERR_STR_InvalidConversionType)},
  {E_ERR_STR_InvalidNumOfSample, ERR_STR(E_ERR_STR_InvalidNumOfSample)},
  {E_ERR_STR_InvalidDataSize, ERR_STR(E_ERR_STR_InvalidDataSize)},
  {E_ERR_STR_InvalidDecodeByteSize, ERR_STR(E_ERR_STR_InvalidDecodeByteSize)},
  {E_ERR_STR_OutputOverrunCumulativeBuffer,
    ERR_STR(E_ERR_STR_OutputOverrunCumulativeBuffer)},
  {E_ERR_STR_InvalidMemorySize, ERR_STR(E_ERR_STR_InvalidMemorySize)},
  {E_ERR_STR_InvalidParameterSize, ERR_STR(E_ERR_STR_InvalidParameterSize)},
  
  // Init Module
  {E_ERR_INM_SWCPollingTableOverlapOrOutOfRange,
    ERR_STR(E_ERR_INM_SWCPollingTableOverlapOrOutOfRange)},
  {E_ERR_INM_GetIntegerValueFail, ERR_STR(E_ERR_INM_GetIntegerValueFail)},
  {E_ERR_INM_SearchKeyWordFail, ERR_STR(E_ERR_INM_SearchKeyWordFail)},
  {E_ERR_INM_GetValueFail, ERR_STR(E_ERR_INM_GetValueFail)},
  {E_ERR_INM_SearchMultiValueFail, ERR_STR(E_ERR_INM_SearchMultiValueFail)},
  {E_ERR_INM_GetDigitFail, ERR_STR(E_ERR_INM_GetDigitFail)},
  {E_ERR_INM_GetRTUConfigFail, ERR_STR(E_ERR_INM_GetRTUConfigFail)},
  {E_ERR_INM_GetAllRTUTableFail, ERR_STR(E_ERR_INM_GetRTUConfigFail)},
  {E_ERR_INM_GetServerDefFail, ERR_STR(E_ERR_INM_GetServerDefFail)},
  {E_ERR_INM_GetAllSWCParamterFail, ERR_STR(E_ERR_INM_GetAllSWCParamterFail)},
  {E_ERR_INM_InvalidVersionNumber, ERR_STR(E_ERR_INM_InvalidVersionNumber)},
  {E_ERR_INM_UndefinedClockProtocol, ERR_STR(E_ERR_INM_UndefinedClockProtocol)},
  {E_ERR_INM_TimeOutIsGreaterThanFastPollingFrequency,
     ERR_STR(E_ERR_INM_TimeOutIsGreaterThanFastPollingFrequency)},
  {E_ERR_INM_InvalidModbusPollFastSWC,
     ERR_STR(E_ERR_INM_InvalidModbusPollFastSWC)},
  {E_ERR_INM_InvalidModbusPollSlowSWC,
     ERR_STR(E_ERR_INM_InvalidModbusPollSlowSWC)},
  {E_ERR_INM_InvalidModbusStandbyPollingFreq,
     ERR_STR(E_ERR_INM_InvalidModbusStandbyPollingFreq)},
  {E_ERR_INM_InvalidModbusFailedPollingFreq,
     ERR_STR(E_ERR_INM_InvalidModbusFailedPollingFreq)},
  {E_ERR_INM_InvalidModbusExceptionRetry,
     ERR_STR(E_ERR_INM_InvalidModbusExceptionRetry)},
  {E_ERR_INM_InvalidGenericSpuriousTime,
     ERR_STR(E_ERR_INM_InvalidGenericSpuriousTime)},
  {E_ERR_INM_InvalidGraceTime, ERR_STR(E_ERR_INM_InvalidGraceTime)},
  {E_ERR_INM_InvalidClkTimeOut, ERR_STR(E_ERR_INM_InvalidClkTimeOut)},
  {E_ERR_INM_InvalidClkRetry, ERR_STR(E_ERR_INM_InvalidClkRetry)},
  {E_ERR_INM_InvalidNTPSpuriousTime, ERR_STR(E_ERR_INM_InvalidNTPSpuriousTime)},
  {E_ERR_INM_InvalidNTPRetry, ERR_STR(E_ERR_INM_InvalidNTPRetry)},
  {E_ERR_INM_InvalidNTPPolling, ERR_STR(E_ERR_INM_InvalidNTPPolling)},

  // Log
  {E_ERR_LOG_OpenDirectoryFail, ERR_STR(E_ERR_LOG_OpenDirectoryFail)},
  {E_ERR_LOG_OpenFileFail, ERR_STR(E_ERR_LOG_OpenFileFail)},


  // Timer
  {E_ERR_TMR_CreateThreadFail, ERR_STR(E_ERR_TMR_CreateThreadFail)},

  // Modbus Management
  {E_ERR_MDB_InvalidSlaveAddress, ERR_STR(E_ERR_MDB_InvalidSlaveAddress)},

  // HWP
  {E_ERR_HWP_InitIoThreadFail, ERR_STR(E_ERR_HWP_InitIoThreadFail)},
  {E_ERR_HWP_InvalidIOType, ERR_STR(E_ERR_HWP_InvalidIOType)},
  {E_ERR_HWP_MmapPortInvalidHandle, ERR_STR(E_ERR_HWP_MmapPortInvalidHandle)},
  {E_ERR_HWP_LedIDExceedDefinedNumber,
     ERR_STR(E_ERR_HWP_LedIDExceedDefinedNumber)},
  {E_ERR_HWP_InvalidLedOnOffStatus, ERR_STR(E_ERR_HWP_InvalidLedOnOffStatus)},
  {E_ERR_HWP_UnMmapPortFail, ERR_STR(E_ERR_HWP_UnMmapPortFail)},
  {E_ERR_HWP_PciServerInitializationFail,
     ERR_STR(E_ERR_HWP_PciServerInitializationFail)},
  {E_ERR_HWP_LCM_ExceedValidBrightnessRange,
     ERR_STR(E_ERR_HWP_LCM_ExceedValidBrightnessRange)},
  {E_ERR_HWP_LCM_InvalidPrnChar, ERR_STR(E_ERR_HWP_LCM_InvalidPrnChar)},
  {E_ERR_HWP_LCM_RowPositionOutOfRange,
     ERR_STR(E_ERR_HWP_LCM_RowPositionOutOfRange)},
  {E_ERR_HWP_LCM_InvalidStartPosition,
     ERR_STR(E_ERR_HWP_LCM_InvalidStartPosition)},
  {E_ERR_HWP_LCM_InvalidMessageType, ERR_STR(E_ERR_HWP_LCM_InvalidMessageType)},

  // LCM
  {E_ERR_LCM_GetAttributeFail, ERR_STR(E_ERR_LCM_GetAttributeFail)},
  {E_ERR_LCM_SetAttributeFail, ERR_STR(E_ERR_LCM_SetAttributeFail)},

  // FLG
  {E_ERR_FLG_MessageSizeOutOfRange, ERR_STR(E_ERR_FLG_MessageSizeOutOfRange)},
  {E_ERR_FLG_TimeStringExceedDefinedBufferSize,
     ERR_STR(E_ERR_FLG_TimeStringExceedDefinedBufferSize)},
  {E_ERR_FLG_MsgQueueSendFail, ERR_STR(E_ERR_FLG_MsgQueueSendFail)},
  {E_ERR_FLG_LogFileTypeNotSupported,
     ERR_STR(E_ERR_FLG_LogFileTypeNotSupported)},

  // NTP
  //{E_ERR_NTP_FailToCreateSocket, ERR_STR(E_ERR_NTP_FailToCreateSocket)},
  //{E_ERR_NTP_FailToConnectSocket, ERR_STR(E_ERR_NTP_FailToConnectSocket)},
  //{E_ERR_NTP_GetTimeOfDayFail, ERR_STR(E_ERR_NTP_GetTimeOfDayFail)},
  //{E_ERR_NTP_InvalidFileDescriptor, ERR_STR(E_ERR_NTP_InvalidFileDescriptor)},
  //{E_ERR_NTP_FailToWriteSocket, ERR_STR(E_ERR_NTP_FailToWriteSocket)},
  //{E_ERR_NTP_FailToReadSocket, ERR_STR(E_ERR_NTP_FailToReadSocket)},
  //{E_ERR_NTP_SelectReadTimeOut, ERR_STR(E_ERR_NTP_SelectReadTimeOut)},
  //{E_ERR_NTP_SelectReadError, ERR_STR(E_ERR_NTP_SelectReadError)},

  // LNC
  {E_ERR_LNC_FailToWriteSocket, ERR_STR(E_ERR_LNC_FailToWriteSocket)},
  {E_ERR_LNC_FailToReadSocket, ERR_STR(E_ERR_LNC_FailToReadSocket)},
  {E_ERR_LNC_SelectReadTimeOut, ERR_STR(E_ERR_LNC_SelectReadTimeOut)},
  {E_ERR_LNC_SelectReadError, ERR_STR(E_ERR_LNC_SelectReadError)},
  {E_ERR_LNC_InvalidSocketType, ERR_STR(E_ERR_LNC_InvalidSocketType)},
  {E_ERR_LNC_FailToCreateSocket, ERR_STR(E_ERR_LNC_FailToCreateSocket)},
  {E_ERR_LNC_FailToConnectDatagramSocket,
     ERR_STR(E_ERR_LNC_FailToConnectDatagramSocket)},
  {E_ERR_LNC_FailToConnectStreamSocket,
     ERR_STR(E_ERR_LNC_FailToConnectStreamSocket)},
  {E_ERR_LNC_InvalidFileDescriptor, ERR_STR(E_ERR_LNC_InvalidFileDescriptor)},
  {E_ERR_LNC_InvalidIoctlForIPAddr, ERR_STR(E_ERR_LNC_InvalidIoctlForIPAddr)},
  {E_ERR_LNC_InvalidIoctlForNetMask, ERR_STR(E_ERR_LNC_InvalidIoctlForNetMask)},
  {E_ERR_LNC_AddGatewayFailToCreateSocket,
     ERR_STR(E_ERR_LNC_AddGatewayFailToCreateSocket)},
  {E_ERR_LNC_AddGatewayUpdateRouteTableFail,
     ERR_STR(E_ERR_LNC_AddGatewayFailToCreateSocket)},
  {E_ERR_LNC_NotSupportedSocket, ERR_STR(E_ERR_LNC_NotSupportedSocket)},

  // CGF
  {E_ERR_CGF_SearchKeyWordFail, ERR_STR(E_ERR_CGF_SearchKeyWordFail)},

  // SWC
  {E_ERR_SWC_InitializationFail, ERR_STR(E_ERR_SWC_InitializationFail)},
  {E_ERR_SWC_ExceedRetryLimit, ERR_STR(E_ERR_SWC_ExceedRetryLimit)},
  {E_ERR_SWC_ExceedExceptionRetryLimit,
     ERR_STR(E_ERR_SWC_ExceedExceptionRetryLimit)},
  {E_ERR_SWC_ExceedBufferSize, ERR_STR(E_ERR_SWC_ExceedBufferSize)},
  {E_ERR_SWC_NoResponseByte, ERR_STR(E_ERR_SWC_NoResponseByte)},
  {E_ERR_SWC_UnhandleModbusError, ERR_STR(E_ERR_SWC_UnhandleModbusError)},	//20150602 Su, fixed wrong definition, E_ERR_SWC_NoResponseByte
  {E_ERR_SWC_ModbusCRCError, ERR_STR(E_ERR_SWC_ModbusCRCError)},
  {E_ERR_SWC_ModbusExceptionError, ERR_STR(E_ERR_SWC_ModbusExceptionError)},
  {E_ERR_SWC_MsgQReplyError, ERR_STR(E_ERR_SWC_MsgQReplyError)},
  {E_ERR_SWC_InvalidFileDescriptor, ERR_STR(E_ERR_SWC_InvalidFileDescriptor)},

  // ServerPoll
  {E_ERR_SPM_InitializationFail, ERR_STR(E_ERR_SPM_InitializationFail)},

  // Development and Testing (DEV)
  {E_ERR_DEV_OpenFileFail, ERR_STR(E_ERR_DEV_OpenFileFail)},

  {E_ERR_EndOfList, ERR_STR(E_ERR_EndOfList)}
}; // ERR_ErrorLut
#endif //_ERR_LUT_H_

