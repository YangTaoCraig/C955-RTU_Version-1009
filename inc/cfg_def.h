/*----------------------------------------------------------------------------

            Copyright (c) 2009 ST Electronics

 PROPRIETARY RIGHTS of ST Electronics are involved in the subject matter of
 this material. All manufacturing, reproduction, use, and sales rights
 pertaining to this subject matter are governed by the license agreement.
 The recipient of this software implicitly accepts the terms of the  license.

----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------

 FILE NAME

  cfg_def.h

 COMPONENT

  n/a

 DESCRIPTION

  This file contains the configuration for compilation

 AUTHOR

  Bryan K. W. Chong

 REVISION HISTORY by Bryan Chong

 1.0.0.0
 -------
 07-May-2012
 - update to official release version 1.0.0.0

 D1.1.3
 ------

 29-Apr-2011

 - remove all compiler definition CFG_ENABLE_QNX641

 09-Sep-2009

 - Created initial version 1.0

----------------------------------------------------------------------------*/
#ifndef CFG_DEF_H
#define CFG_DEF_H
/*-----------------------------------------------------------------------------
 Software reversion convention
 Ref: PMS SCMP.doc, Rev B, 26-Jan-2010, pg 15/23.
      Figure 2-5: Baseline Software Version Identification

 A.B.C.D (internal control code)

 A: Number to track release version (1..9)
 B: Number to track new feature (0..9)
 C: Number to track customization (0..9)
 D: Number to track fault fixed (0..9)

 internal control code:
 (date code.release number)
 date code: yyyymmdd
   yyyy - year
   mm   - month
   dd   - day
 release number: 00..99
------------------------------------------------------------------------------*/
#define CFG_SYS_REVISION_STRING \
  "RTU (c) ST Electronics 2015, release 1.0.1.0(20160628.01)\r\0"


#define CFG_SYS_IDN_STR CFG_SYS_REVISION_STRING

#define CFG_YES  1
#define CFG_NO   0

//Definition for test release, include features for test release
// Enable watchdog timer to timeout
#define CFG_TEST_RELEASE

// Temporary disable for debugging purpose
#define ENABLE_CLOSE_LISTEN_SOCKET

#define CFG_SYS_ASSERT
#ifndef CFG_SYS_ASSERT_RETURN
#define CFG_SYS_ASSERT_RETURN
#endif // CFG_SYS_ASSERT_RETURN

#ifndef CFG_SYS_ASSERT_PRN_RETURN
#define CFG_SYS_ASSERT_PRN_RETURN
#endif // CFG_SYS_ASSERT_PRN_RETURN

// Enable print release information
#define CFG_PRN_REL_INFO

// Enable watchdog reset
#define CFG_ENABLE_WATCHDOG_RESET

// Enable system shutdown
#define CFG_ENABLE_SYSTEM_SHUTDOWN       CFG_YES
// Enable application to take in config file arguement
#define CFG_ENABLE_READ_CONFIG_FILE_PARAMETER  CFG_NO

// Enable print debugging messages
//#define CFG_DEBUG_MSG

// Enable print error messages
//#define CFG_PRN_ERR

// Enable LAN Port 0 to be primary LAN port
//#define CFG_PRIMARY_LAN_PORT_1

// Enable LAN Port 0 to be primary LAN port
#define CFG_PRIMARY_LAN_PORT_4
#define CFG_DEFAULT_LAN_PORT_1_IP_ADDR  "10.1.1.1\0"

#if ((defined CFG_PRIMARY_LAN_PORT_1) && (defined CFG_PRIMARY_LAN_PORT_4))
#error ERR  cfg_def.h, only ONE port is allowed to be configured as the primary \
LAN port. Either define CFG_PRIMARY_LAN_PORT_1 or CFG_PRIMARY_LAN_PORT_4 but \
not both.
#endif // ((defined CFG_PRIMARY_LAN_PORT_0) && (defined CFG_PRIMARY_LAN_PORT_4))

#if ((!defined CFG_PRIMARY_LAN_PORT_1) && (!defined CFG_PRIMARY_LAN_PORT_4))
#error ERR  cfg_def.h, must have ONE port to be configured as the primary \
LAN port. Either define CFG_PRIMARY_LAN_PORT_1 or CFG_PRIMARY_LAN_PORT_4 but \
not both.
#endif // ((!defined CFG_PRIMARY_LAN_PORT_0) &&
       //  (!defined CFG_PRIMARY_LAN_PORT_4))

#define CFG_PRN_ERR_MDB               CFG_NO
#define CFG_PRN_ERR_MDB_LENGTH        CFG_NO

// Enable error message for SCPI command
#define CFG_PRN_ERR_SPA_CMD           CFG_NO

// LNC
#define CFG_PRN_ERR_LNC               CFG_NO

// SWC
#define CFG_PRN_ERR_SWC               CFG_NO

// Enable print warning messages
//#define CFG_PRN_WARN

// Enable print warning message in link monitor

#define CFG_PRN_WARN_CMM_LISTEN_SOCK  CFG_NO
#define CFG_PRN_WARN_CMM_RECV         CFG_NO
#define CFG_PRN_WARN_CMM_RECM         CFG_NO
#define CFG_PRN_WARN_SENDREPLY        CFG_NO
#define CFG_PRN_WARN_RMMSVRCMD        CFG_NO
#define CFG_PRN_WARN_RMMLINK          CFG_NO
#define CFG_PRN_WARN_RMMNOTATION      CFG_NO
#define CFG_PRN_WARN_RMM_STATE        CFG_NO
#define CFG_PRN_WARN_SWC_LINKMON      CFG_NO
#define CFG_PRN_WARN_SWC_LINKCHK      CFG_NO
#define CFG_PRN_WARN_SWC_LAN_LINKMON  CFG_NO
#define CFG_PRN_WARN_NTP              CFG_NO
#define CFG_PRN_WARN_LNC              CFG_NO
#define CFG_PRN_WARN_SER              CFG_NO
#define CFG_PRN_WARN_SYS              CFG_NO
#define CFG_PRN_WARN_LOG              CFG_NO
#define CFG_PRN_WARN_MMM              CFG_NO
#define CFG_PRN_WARN_SWC              CFG_NO
#define CFG_PRN_WARN_SWC_LAN          CFG_NO
#define CFG_PRN_WARN_SWC_MODBUS       CFG_NO

// Enable PCI-Serial card detection
#define CFG_ENABLE_PCI_DETECTION  CFG_NO

// SYS
// Enable SYS messages
#define CFG_VALID_YEAR_START      2008
#define CFG_DEBUG_SYS             CFG_NO
#define CFG_DEBUG_SYS_EXIT        CFG_NO
// Enable print timezone info
#define CFG_DEBUG_TIME            CFG_NO

// FLG
// Enable print file logging messages
#define CFG_DEBUG_FLG             CFG_NO
// Enable HWP debugging messages
#define CFG_DEBUG_HWP             CFG_NO
// Enable HWP LCM debugging messages
#define CFG_DEBUG_HWP_LCM         CFG_NO

// SPA
// Enable print debugging messages for SPA
#define CFG_DEBUG_SPA             CFG_NO
// Enable print SPA received message
#define CFG_DEBUG_SPA_RX_MSG      CFG_NO

// Enable debugging message for SCPI command
#define CFG_DEBUG_SPA_CMD         CFG_NO

// Enable print INM message
#define CFG_DEBUG_INM             CFG_NO

// Enable print debugging message for Server Poll Management (SPM)
#define CFG_DEBUG_SPM             CFG_NO
//#define CFG_DEBUG_SPM_TX_PACKET

// RMM
// Print debug message on system notation for every interval
//#define CFG_RMM_NOTATION_THRD
#define CFG_DEBUG_RMM_NOTATION_MGMT     CFG_NO
// Enable print debug message for RMM
#define CFG_DEBUG_RMM                   CFG_NO
#define CFG_DEBUG_RMMLINK               CFG_NO
#define CFG_DEBUG_RMMLINKCHK            CFG_NO
#define CFG_DEBUG_RMMLINK_WRITESWCTBL   CFG_NO
#define CFG_DEBUG_RMMLINK_LINKMASTER    CFG_NO
#define CFG_DEBUG_RMMLINK_LINKSLAVE     CFG_NO
#define CFG_DEBUG_RMMLINK_STATUS        CFG_NO
#define CFG_DEBUG_RMMLINK_SENDCMD       CFG_NO
#define CFG_DEBUG_RMMLINK_RXCMD         CFG_NO
#define CFG_DEBUG_RMMLINK_GETRESPONSE   CFG_NO
#define CFG_DEBUG_RMMLINK_WRITECMD      CFG_NO
#define CFG_DEBUG_RMMLINK_GETNEXTSWCTBL CFG_NO
#define CFG_DEBUG_RMMLINK_SWITCH        CFG_NO
#define CFG_DEBUG_RMM_RX_CMD            CFG_NO
#define CFG_DEBUG_RMM_STATE             CFG_NO
#define CFG_DEBUG_RMM_NOTATION          CFG_NO
#define CFG_DEBUG_RMM_SCPI              CFG_NO

// SWC
#define CFG_TOTAL_NUM_OF_SWC            20
#define CFG_NUM_OF_THREADS_PER_SWC      2
#define CFG_SWC_MAXRXBYTE               600

#define CFG_SWC_LAN_ENABLE_WATCHDOG       CFG_NO
// Enable print debug message for SWC-LAN
#define CFG_DEBUG_SWC_LAN                 CFG_NO
#define CFG_DEBUG_SWC_LAN_RQ_TX           CFG_NO
#define CFG_DEBUG_SWC_LAN_READ_FASTP_TBL  CFG_NO
#define CFG_DEBUG_SWC_LAN_READ_SLOWP_TBL  CFG_NO
#define CFG_DEBUG_SWC_LAN_READ_TIMER_TBL  CFG_NO
#define CFG_DEBUG_SWC_LAN_FASTPOLLING     CFG_NO
#define CFG_DEBUG_SWC_LAN_SLOWPOLLING     CFG_NO
#define CFG_DEBUG_SWC_LAN_TIMERPOLLING    CFG_NO
#define CFG_DEBUG_SWC_LAN_TIMESYNC        CFG_NO
#define CFG_DEBUG_SWC_LAN_LINKMON         CFG_NO
#define CFG_DEBUG_SWC_LAN_SPCMD           CFG_NO
#define CFG_DEBUG_SWC_LAN_KEEPALIVE       CFG_NO
#define CFG_DEBUG_SWC_LAN_STBYLINKCHK     CFG_NO

// Enable print debug message for SWC serial
// debug message limited to the following SWCs,
#define CFG_DEBUG_SWC_INDEX          (m_nSWCID == 1)

#define CFG_DEBUG_SWC                   CFG_NO

// Enable print debug message for request and receive and
// transmit data block
#define CFG_DEBUG_SWC_RQ_RX             CFG_NO
#define CFG_DEBUG_SWC_RQ_TX             CFG_NO

// Enable print debug message for fast polling
#define CFG_DEBUG_SWC_FASTPOLLING       CFG_NO

// Enable print debug message for slow polling
#define CFG_DEBUG_SWC_SLOWPOLLING       CFG_NO

// Enable print timing information for polling
#define CFG_DEBUG_SWC_TIMERPOLLING      CFG_NO

// Enable print debug message for SWC link check
#define CFG_DEBUG_SWC_PRIMARYLINKCHECK  CFG_NO

// Enable print debug message for SWC time sync
#define CFG_DEBUG_SWC_TIMESYNC          CFG_NO

// Enable print debug message for SWC link
#define CFG_DEBUG_SWC_LINKCHK           CFG_NO

// Enable debug link monitor
#define CFG_DEBUG_SWC_LINKMON           CFG_NO
// Enable Timeout message for Standby RTU
#define _CFG_DEBUG_SWC_LINKMON           CFG_NO
// Enable debug NTP client communication
#define CFG_DEBUG_SWC_NTPC              CFG_NO

// Enable debug Keep alive message
#define CFG_DEBUG_SWC_KEEPALIVE         CFG_NO

// Enable debug SWC SCPI
#define CFG_DEBUG_SWC_SCPI              CFG_NO

// Enable debug SWC Specific Command
#define CFG_DEBUG_SWC_SPCMD             CFG_NO

#define CFG_DEBUG_SWC_MODBUS            CFG_NO

// Watchdog debug message
#define CFG_DEBUG_WD                    CFG_NO

// NTP Client
// Enable print debug message for NTP
#define CFG_DEBUG_NTP                   CFG_NO
#define CFG_DEBUG_NTP_POLL              CFG_NO
#define CFG_DEBUG_NTP_RXCMD             CFG_NO
#define CFG_DEBUG_NTP_TXRESP            CFG_NO
#define CFG_DEBUG_NTP_SWC_TXCMD         CFG_NO
#define CFG_DEBUG_NTP_SWC_RXRESP        CFG_NO

#define CFG_NTP_CLIENT_SWC_INDEX        19
#define CFG_NTP_CMD_BUFF_SZ             100
#define CFG_NTP_RESP_BUFF_SZ            100
#define CFG_NTP_DEFAULT_NAME            "NTPCli"


// LAN
// Enable LAN transmit and receive packet using SendReply_PLC
//#define CFG_DEBUG_LAN_TX
#define CFG_DEBUG_LAN_SEND               CFG_NO
#define CFG_DEBUG_LAN                    CFG_NO

// MMM
#define CFG_DEBUG_MMM                    CFG_NO



// Enable CMM (Common) debug messag
#define CFG_DEBUG_CMM                    CFG_NO
#define CFG_DEBUG_CMM_SENDREPLY          CFG_NO
#define CFG_DEBUG_CMM_LISTEN             CFG_NO
#define CFG_DEBUG_CMM_CONNECT            CFG_NO
#define CFG_DEBUG_CMM_LISTEN_ACCP_SOCKET CFG_NO
#define CFG_DEBUG_CMM_RXM                CFG_NO
// Enable print LAN recv packet message
#define CFG_DEBUG_CMM_RECV               CFG_NO

// Enable log module debugging
#define CFG_DEBUG_LOG                    CFG_NO
// Define directory structure for RTU
//#define CFG_ENABLE_CREATING_RTU_ROOT_DIRECTORY
#define CFG_RTU_ROOT_FOLDER              "/RTU/"
#define CFG_RTU_LOG_FOLDER               "/RTU/log/"


// Configuration for SPA component
// Maximum output result syntax length
#define CFG_SPA_OUTPUT_BUFF_SZ      2048
// Maximum input command syntax length
#define CFG_SPA_INPUT_CMD_SZ        100
// Maximum output command syntax length
#define CFG_SPA_OUTPUT_CMD_SZ       1024
// Maximum length for each parameter
#define CFG_SPA_MAX_PARA_LENGTH     30
// Maximum number of input command parameter
#define CFG_SPA_MAX_INPUT_CMD_PARAM 10
// Command terminate character
#define CFG_TERM_STR "\r"           // '\r' = 0xD, '\n' = 0xA
#define CFG_FIRST_TERM_CHAR '\r'
#define CFG_TERM_CHAR_NEWLINE '\n'
#define CFG_SPA_ENABLE_CASEINSENSITIVE_PARAMETERS


// SER
// Enable SER to get data rate
//#define CFG_ENABLE_SER_GETDATARATE

// Print '\r' character
//#define CFG_ENABLE_PRN_TERM_CHAR
// Print '\n' character
//#define CFG_ENABLE_PRN_NEWLINE_CHAR
#define CFG_DEBUG_SER               CFG_NO
#define CFG_DEBUG_SER_HOSTCMD       CFG_NO

// LAN Communication (LNC)
#define CFG_DEBUG_LNC               CFG_NO
#define CFG_DEBUG_LNC_SENDREPLY     CFG_NO
#define CFG_DEBUG_LNC_SENDMSG       CFG_NO
#define CFG_DEBUG_LNC_GETFD         CFG_NO
#define CFG_LNC_TOTAL_ACTIVE_PORT   6

// CMM Modbus
#define CFG_DEBUG_MODBUS            CFG_NO

// String Management Module (STR)
#define CFG_DEBUG_STR               CFG_NO

/* Hardware Peripherals definition */
#define CFG_HWP_TOTAL_NUM_LED   5
#define CFG_HWP_TOTAL_PCI_SLOT  2
#define CFG_HWP_TOTAL_LAN_PORT  6
#define CFG_HWP_PCI_SERIAL_COM_START_NUM  5
#define CFG_HWP_TOTAL_COM_PORT_PER_CARD   8

// Development and Testing
#define CFG_DEFAULT_FILENAME_SZ         100
#define CFG_DEV_GLOBAL_FILENAME_SZ      CFG_DEFAULT_FILENAME_SZ
#define CFG_DEFAULT_GLOBAL_FILENAME     "/RTU/dev_global.txt"
#define CFG_DEFAULT_ERRORLIST_FILENAME  "/RTU/errorlist.txt"
#define CFG_DEFAULT_ERRORLIST_FILENAME_SZ  CFG_DEFAULT_FILENAME_SZ

// Enable print debugging messages for DEV component
#define CFG_DEBUG_DEV             CFG_NO
// Enable debugging messages for SCPI
#define CFG_DEBUG_DEV_SCPI        CFG_NO

// Enable module testing routine
//#define CFG_ENABLE_MODULE_TEST

// RTU Parameter Configuration file
#define CFG_CONFIG_FILE_DEFAULT         "/RTU/C955RTU.cfg"

// RTU Log file prefix
#define CFG_LOG_PREFIX_EV               "EV"
#define CFG_LOG_PREFIX_DB               "DB"


// Module Test
//#define CFG_MODULE_TEST

// Module Test for CMM
#define CFG_MDT_CMM       CFG_NO
// Module Test for SWC-LAN
#define CFG_MDT_SWC_LAN   CFG_NO
// Define log file for CMM module test
#define CFG_MDT_CMM_LOG   "/RTU/log/PathLog.txt"
// Module Test for SPA
// #define CFG_MDT_SPA       CFG_YES
// Define log file for SPA module test
#define CFG_MDT_SPA_LOG   "/RTU/log/SPA_PathLog.txt"

#endif // CFG_DEF_H
