/*----------------------------------------------------------------------------

            Copyright (c) 2009 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   ser_ext.h                                                D1.0.4

 COMPONENT

   n/a

 DESCRIPTION

   This file contains the external declaration for serial port management


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     09-Sep-2009      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef _SER_EXT_H_
#define _SER_EXT_H_
#ifdef __cplusplus
extern "C"
{
#endif

extern SER_CB_T *pSER_CB;

//Serial communication ports name
extern CHAR *SER_acSerialPortName[24];
//extern INT32 RequestAndReceiveFlag;
extern E_ERR_T SER_Initialization();
//extern E_ERR_T SER_ReInitializePort(const E_SER_COM comport);
//extern E_ERR_T SER_OpenPort(INT32 *pfd, E_SER_COM com_port);
//extern E_ERR_T SER_SendMsg(INT32 *pfd, CHAR *pmsg);
extern E_ERR_T SER_SendMsg(E_SER_COM comport, CHAR *pmsg);
extern E_ERR_T SER_Close(INT32 *pfd);

//#ifdef CFG_ENABLE_SPA
//extern VOID *SER_ReceiveHostCommandThrdFunc (VOID *pThreadParameter);
//#endif // CFG_ENABLE_SPA

//extern VOID *SER_TransmitResultThrdFunc (VOID *pThreadParameter);
extern E_ERR_T SER_SetAttribute(E_SER_COM comport, speed_t unbaudrate,
                               E_SER_PARITY parity, E_SER_DATABIT databit,
                               E_SER_STOPBIT stopbit);
extern E_ERR_T SER_SendChar(const E_SER_COM comport, UINT8 *pbuff, UINT32 txsz);
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SER_SendChar

 DESCRIPTION

  This routine will receive character from the designated port

 CALLED BY

  Application main

 CALLS

  [TBD]

 PARAMETER

  None

 RETURN

   E_ERR_Success      the routine executed successfully

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    11-Feb-2010      Created initial revision

-----------------------------------------------------------------------------*/
extern E_ERR_T SER_RxChar(const E_SER_COM comport, UINT8 *poutbuff,
                         const UINT32 rxpacket_sz, INT32 *poutrxsz,
                         const INT32 timeout_ms);
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SER_SendWithReply

 DESCRIPTION

  This routine will transmit a request packet and wait for the reply.
  The routine is inherited from LNC_SendWithReply

  read will actually read 8 bytes at a time. If the incoming data is more
  than 8 bytes, the read process will loop untill the remaining bytes, which
  is less than 8 bytes. Continue reading will cause timeout from select routine,
  and will result in FD_ISSET(nfd, &fdsRead) equal to FALSE.

 CALLED BY

  SYS_Initialization

 CALLS

  None

 PARAMETER

  nfd          [in]  file descriptor of the LAN connection
  ptxbuff      [in]  pointer to transmit buffer
  txpacket_sz  [in]  transmit packet size in bytes
  prxbuff      [out] pointer to receive buffer
  rxpacket_sz  [in]  expected receive packet size in bytes
  rxdata_sz    [out] receive data size. Can be less than or equal to receive
                     buffer size
  timeout_ms   [in]  receiving time out in milliseconds

 RETURN

   E_ERR_Success      the routine executed successfully

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    25-Feb-2009      Created initial revision

-----------------------------------------------------------------------------*/
extern E_ERR_T SER_SendWithReply(INT32 nfd, const VOID *ptxbuff,
                                const UINT32 txpacket_sz, VOID *prxbuff,
                                const UINT32 rxpacket_sz, INT32 *prxdata_sz,
                                INT32 timeout_ms);
/*-----------------------------------------------------------------------------

 PUBLIC ROUTINE

  SER_ReceiveMsg

 DESCRIPTION

  This routine will read will actually read 8 bytes at a time. If the incoming
  data is more than 8 bytes, the read process will loop untill the remaining
  bytes, which is less than 8 bytes. Continue reading will cause timeout from
  select routine, and will result in FD_ISSET(nfd, &fdsRead) equal to FALSE.

 CALLED BY

  Application

 CALLS

  None

 PARAMETER

  nfd          [in]  file descriptor of the LAN connection
  prxbuff      [out] pointer to receive buffer
  prxdata_sz   [out] pointer to receive data size. Can be less than or equal to
                     receive buffer size
  rxpacket_sz  [in]  expected receive packet size in bytes
  timeout_ms   [in]  receiving time out in milliseconds

 RETURN

   E_ERR_Success                       the routine executed successfully
   E_ERR_SER_InvalidFileDescriptor     invalid file descriptor
   E_ERR_SER_SelectReadTimeOut         select read timeout
   E_ERR_SER_SelectReadError           select read error
   E_ERR_SER_InvalidSelectReturnValue  invalid select return value

 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

   Bryan Chong    06-May-2010      Created initial revision.

-----------------------------------------------------------------------------*/
extern E_ERR_T SER_ReceiveMsg(INT32 nfd, VOID *prxbuff, INT32 *prxdata_sz,
                             const UINT32 rxpacket_sz, INT32 timeout_ms);
#ifdef __cplusplus
}
#endif
#endif //_SER_EXT_H_
