/*----------------------------------------------------------------------------

            Copyright (c) 2009 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   swc_def.h                                              D1.0.4

 COMPONENT

   n/a

 DESCRIPTION

   This file contains the external declaration for SWC Management


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     24-Nov-2009      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef _SWC_DEF_H_
#define _SWC_DEF_H_

// Total number of supporting SWCs
#define SWC_MAX_NUMBER     CFG_TOTAL_NUM_OF_SWC

// Set this constant to minimum to reduce RTU role switching time
#define SWC_LINKCHK_RETRY  1

typedef enum e_tabletype{
  E_SWC_TBL_Reserved_0,
  E_SWC_TBL_FastPolling,
  E_SWC_TBL_SlowPolling,
  E_SWC_TBL_TimerPolling,
  E_SWC_TBL_LinkCheck,
  E_SWC_TBL_EndOfList
} E_SWC_TBL;

typedef union swc_communication_link_status_st{
  struct flags_bit_st{
    UINT16 isLine1Used:          1;
    UINT16 isLine2Used:          1;
    UINT16 isSWCEnable:          1;
    UINT16 isCommunicationValid: 1;
    UINT16 isLine1Healthy:       1;
    UINT16 isLine2Healthy:       1;
    UINT16 reserved2:            2;
    UINT16 isLine1MasterLink:    1;
    UINT16 isLine2SlaveLink:     1;
    UINT16 reserved3:            2;
    UINT16 isSerialLink:         1;
    UINT16 isLANLink:            1;
    UINT16 hasNoExceptionError:  1;
    UINT16 reserved1:            1; // Yang Tao 20150506 Swap the bit for 1007 update.
  }bitctrl;
  UINT16 allbits;
} SWC_LINK_STATUS;

typedef enum e_swc_type{
  E_SWC_T_Reserved_0,
  E_SWC_T_NTP_Client,
  E_SWC_T_EndOfList
} E_SWC_T;

#endif //_SWC_DEF_H_
