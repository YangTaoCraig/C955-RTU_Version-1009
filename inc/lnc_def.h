/*----------------------------------------------------------------------------

            Copyright (c) 2010 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME                                            VERSION

   lnc_def.h                                          D1.0.4

 COMPONENT

   n/a

 DESCRIPTION

   This file contains the type definitions and declaration for LAN
   Communication component


 AUTHOR

   Bryan K. W. Chong


 HISTORY

      NAME            DATE                    REMARKS

   Bryan Chong     08-Feb-2010      Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef LNC_DEF_H
#define LNC_DEF_H

// tag for each entry for the lookup table
typedef enum e_lnc_endPointConfig {
  E_LNC_EPCONFIG_NTPC_ServerIP1,
  E_LNC_EPCONFIG_NTPC_ServerIP2,
  E_LNC_EPCONFIG_NTPC_ServerIP3,
  //E_LNC_EPCONFIG_RMM_OtherRTUIP,
  
  E_LNC_EPCONFIG_EndOfList
}E_LNC_EPCONFIG;

// struct for the end point configuration lookup table
typedef struct lnc_lanPointConfig_st{
  E_LNC_EPCONFIG label;
  INT32 domain;
  INT32 type;
  INT32 protocol;
  UINT32 port;   // port number in network byte order format
  //const CHAR *pipaddr; // IP address in internet address format

} LNC_EPCONFIG_T;


typedef struct lnc_ctrlBlk_st{
  struct ep_config_st{
    LNC_EPCONFIG_T *plutEntry;
    CHAR *pipaddr;
    INT32 nfd;
  }endPointConfig[E_LNC_EPCONFIG_EndOfList];

} LNC_CB;
#endif /* LNC_DEF_H */
