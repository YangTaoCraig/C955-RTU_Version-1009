/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   lan_ext.h                                           D1.0.4

 COMPONENT

   LAN - LAN communication

 DESCRIPTION

   This file contains the external declaration for LAN communication component


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     08-Feb-2010      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef LAN_EXT_H
#define LAN_EXT_H

#ifdef __cplusplus
extern "C"
{
#endif

/*----------------------------------------------------------------------------
  Global variable definition
----------------------------------------------------------------------------*/
extern LNC_CB *pLNC_CB;
/*----------------------------------------------------------------------------
  Public prototype declaration
----------------------------------------------------------------------------*/
extern E_ERR_T LNC_Initialization(VOID);
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  LNC_SendWithReply

 DESCRIPTION

  This routine will transmit a request packet and wait for the reply

 CALLED BY

  NTP_PollingThrdFunc
  NTP_LinkManagementThrdFunc
  NTP_Status_Query
  RequestAndReceiveWithModbusCheck, SWC_LAN.cpp
  
 CALLS

  None

 PARAMETER

  nfd          [in]  file descriptor of the LAN connection
  ptxbuff      [in]  pointer to transmit buffer
  txbuff_sz    [in]  transmit buffer size
  prxbuff      [out] pointer to receive buffer
  rxbuffsz     [in]  receiving buffer size
  rxdata_sz    [out] number of data received. Can be less than or equal to
                     receive buffer size
  timeout_ms   [in]  receiving time out in milliseconds

 RETURN

   E_ERR_Success      the routine executed successfully
   
   E_ERR_LNC_InvalidFileDescriptor
   E_ERR_LNC_FailToWriteSocket
   E_ERR_LNC_SelectReadTimeOut
   E_ERR_LNC_SelectReadError
   E_ERR_LNC_FailToReadSocket

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    08-Feb-2010      Created initial revision

-----------------------------------------------------------------------------*/
extern E_ERR_T LNC_SendWithReply(INT32 nfd, const VOID *ptxbuff,
                                const UINT32 txbuff_sz, VOID *prxbuff,
                                const UINT32 rxbuffsz, INT32 *prxdata_sz,
                                INT32 timeout_ms);
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  LNC_ReceiveMsg

 DESCRIPTION

  This routine will receive message from designated file decriptor for LAN

 CALLED BY

  

 CALLS

  None

 PARAMETER

  nfd          [in]  file descriptor of the LAN connection
  prxbuff      [out] pointer to receive buffer
  rxdata_sz    [out] receive data size. Can be less than or equal to receive
                     buffer size
  timeout_ms   [in]  receiving time out in milliseconds

 RETURN

   E_ERR_Success      the routine executed successfully

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    10-May-2010      Created initial revision

-----------------------------------------------------------------------------*/
extern E_ERR_T LNC_ReceiveMsg(INT32 nfd, VOID *prxbuff, INT32 *prxdata_sz, 
                             const UINT32 rxbuffsz, INT32 timeout_ms);
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  LNC_SendMsg

 DESCRIPTION

  This routine will transmit message using designated file decriptor for LAN 
  interface

 CALLED BY



 CALLS

  None

 PARAMETER

  nfd          [in]  file descriptor of the LAN connection
  ptxbuff      [in]  pointer to transmit buffer
  txmsgsz      [in]  transmit message size
  prtxsz       [out] total bytes of data transmitted

 RETURN

   E_ERR_Success                the routine executed successfully
   E_ERR_LNC_FailToWriteSocket  fail to write to the file descriptor
   E_ERR_InvalidNullPointer
   E_ERR_LNC_InvalidFileDescriptor
   E_ERR_InvalidParameter

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    22-Jun-2010      Created initial revision

-----------------------------------------------------------------------------*/
extern E_ERR_T LNC_SendMsg(INT32 nfd, VOID *ptxbuff, const UINT32 txmsgsz, 
                          INT32 *prtxsz);
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  LNC_AddGateway

 DESCRIPTION

  This routine will add gateway to routing table

 CALLED BY



 CALLS

  None

 PARAMETER

  gateway_addr    [in] pointer to gateway address in IPv4 dot notation string
  subnetmask_addr [in] pointer to subnetmask address in IPv4 dot notation
                       string
  retry           [in] number of retry count

 RETURN

   E_ERR_Success                              the routine executed successfully
   E_ERR_LNC_AddGatewayFailToCreateSocket     fail to create socket
   E_ERR_LNC_AddGatewayUpdateRouteTableFail   fail to update routing table

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    18-May-2010      Created initial revision

-----------------------------------------------------------------------------*/
extern E_ERR_T LNC_AddGateway(const CHAR *pgateway_addr, const CHAR *psubnet_mask,
                             const UINT8 retry);          
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  LNC_GetConnectFileDescriptor

 DESCRIPTION

  This routine will transmit a request packet and wait for the reply

 CALLED BY

  SYS_Initialization

 CALLS

  None

 PARAMETER

  pipaddr      [in]  pointer to IP address in ascii format
  port_num     [in]  port number to be connected
  sock_type    [in]  socket type
  timeout_ms   [in]  timeout in millisecond
  poutnfd      [out] pointer for output file descriptor

 RETURN

   E_ERR_Success                         the routine executed successfully
   E_ERR_LNC_FailToCreateSocket          fail to create socket
   E_ERR_LNC_InvalidSocketType           invalid socket type
   E_ERR_LNC_FailToConnectStreamSocket   fail to connect to stream socket
   E_ERR_LNC_NotSupportedSocket          not supported socket type

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    07-Jun-2010      Created initial revision

-----------------------------------------------------------------------------*/
extern E_ERR_T LNC_GetConnectFileDescriptor(const CHAR *pipaddr, 
                                           const UINT16 port_num,
                                           const INT32 sock_type, 
                                           const UINT32 timeout_ms, 
                                           INT32 *poutnfd);
/*----------------------------------------------------------------------------
  SCPI support prototype declaration
----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
  SCPI supporting routine
----------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
#endif // LAN_EXT_H

