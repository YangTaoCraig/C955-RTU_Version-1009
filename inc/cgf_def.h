/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics Ltd.

 PROPRIETARY RIGHTS of ST Electronics Ltd are involved in the subject matter
 of this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

  cgf_def.h                                            D1.0.4

 COMPONENT

  CGF - Configuration File

 DESCRIPTION

  This file consists of the definition of the CGF component


 AUTHOR

  Bryan KW Chong


 HISTORY

    NAME            DATE                    REMARKS

  Bryan Chong    10-Feb-2010        Initial revision

----------------------------------------------------------------------------*/
#ifndef CGF_DEF_H
#define CGF_DEF_H

#define CGF_NTP_TOTAL_SERVER 3
#define CGF_SWC_HEALTHY_LINK_UPDATE_FREQ_SEC  10

#define CFG_NTP_MSGQ_SZ      50

typedef struct configuration_file_control_block_st{


  struct ntp_parameter_st{
    BOOL_T isServer;
    BOOL_T isClient;
    CHAR   ipaddr[CGF_NTP_TOTAL_SERVER][16];
    UINT32 spuriousTime;
    // retry count for each attempt to connect to the NTP Servers
    UINT8  retry; 
    UINT16 notation;
    // Polling frequency. Range 1 - 3600 seconds
    UINT16  pollingFreq; 
    // Retry after link down. Range 1 - 3600 seconds
    UINT16  linkDownRetryFreq; 
  }ntp;
  
}CGF_CB;

#endif // CGF_DEF_H

