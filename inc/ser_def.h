#ifndef _SER_DEF_H_
#define _SER_DEF_H_

#ifdef __cplusplus
extern "C"
{
#endif


/*  Serial Communication Port define */
#define  ser1  "/dev/ser1"
#define  ser2  "/dev/ser2"
#define  ser3  "/dev/ser3"
#define  ser4  "/dev/ser4"
#define  ser5  "/dev/ser5"
#define  ser6  "/dev/ser6"
#define  ser7  "/dev/ser7"
#define  ser8  "/dev/ser8"
#define  ser9  "/dev/ser9"
#define  ser10  "/dev/ser10"
#define  ser11  "/dev/ser11"
#define  ser12  "/dev/ser12"
#define  ser13  "/dev/ser13"
#define  ser14  "/dev/ser14"
#define  ser15  "/dev/ser15"
#define  ser16  "/dev/ser16"
#define  ser17  "/dev/ser17"
#define  ser18  "/dev/ser18"
#define  ser19  "/dev/ser19"
#define  ser20  "/dev/ser20"
#define  ser21  "/dev/ser21"
#define  ser22  "/dev/ser22"
#define  ser23  "/dev/ser23"
#define  ser24  "/dev/ser24"

/* Total character receiving buffer size */
#define SER_RX_BUFF_SZ      1024

/* Total command buffer size */
#define SER_CMD_BUFF_SZ     1024

typedef enum serial_port_e{
  E_SER_COM_Port_Reserved,

  // onboard serial com port
  E_SER_COM_Port_1,
  E_SER_COM_Port_2,
  E_SER_COM_Port_3,
  E_SER_COM_Port_4,

  // Moxa card 1 PCI based serial com port
  E_SER_COM_Port_5,
  E_SER_COM_Port_6,
  E_SER_COM_Port_7,
  E_SER_COM_Port_8,
  E_SER_COM_Port_9,
  E_SER_COM_Port_10,
  E_SER_COM_Port_11,
  E_SER_COM_Port_12,

  // Moxa card 2 PCI based serial com port
  E_SER_COM_Port_13,
  E_SER_COM_Port_14,
  E_SER_COM_Port_15,
  E_SER_COM_Port_16,
  E_SER_COM_Port_17,
  E_SER_COM_Port_18,
  E_SER_COM_Port_19,
  E_SER_COM_Port_20,

  E_SER_COM_EndOfList
} E_SER_COM;

#define SER_SCPICMD_PORT   E_SER_COM_Port_3

typedef enum serial_parity_enum{
  E_SER_PARITY_Disable,
  E_SER_PARITY_Odd,
  E_SER_PARITY_Even,
  E_SER_PARITY_EndOfList
} E_SER_PARITY;

typedef enum serial_databits_enum{
  E_SER_DATABIT_Reserved,
  E_SER_DATABIT_5 = 5,
  E_SER_DATABIT_6,
  E_SER_DATABIT_7,
  E_SER_DATABIT_8,
  E_SER_DATABIT_EndOfList
} E_SER_DATABIT;

typedef enum serial_stopbits_enum{
  E_SER_STOPBIT_Reserved,
  E_SER_STOPBIT_1,
  E_SER_STOPBIT_2,
  E_SER_STOPBIT_EndOfList
} E_SER_STOPBIT;

/* Total path name length */
#define SER_PATH_NAME_LEN   20

typedef struct serial_port_descriptor_st{
  E_SER_COM     port;
  CHAR          path_name[SER_PATH_NAME_LEN];
  speed_t       baudrate;
  E_SER_PARITY  parity;
  E_SER_DATABIT databit;
  E_SER_STOPBIT stopbit;
  UINT32        accessmode;
} SER_PD_T;

#define SER_HISTORY_TOTAL_MSG  5
#define SER_HISTORY_MSG_LEN    50
typedef struct serial_control_block_st{
  SER_PD_T *pport_desc; //port descriptor
  INT32    fd;        // file descriptor
  struct latency_check_st{
    FP64 bytepersecond;
  }latency;
  CHAR history[SER_HISTORY_TOTAL_MSG][SER_HISTORY_MSG_LEN];
  UINT8 nextEmptyHistoryId;
} SER_CB_T;

#ifdef __cplusplus
}
#endif

#endif //_SER_DEF_H_
