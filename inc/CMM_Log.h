
#ifndef  _CMM_LOG_H
#define  _CMM_LOG_H

#define MIN_DAY_FILE_STORE  1    //Minimum store 1 day log file.
#define MAX_DAY_FILE_STORE  30    //Maximum store 30 days log file.
#define ONE_DAY_SECONDS     86400  //One day = 60*60*24 = 86400 seconds

#define MIN_FREE_DISK_SPACE  5*1024*1024    //5M free space is min (in bytes)

//Log buffer size defination.	//040525 Yu Wei
#define ONE_SECOND_MAX_LOG  500    //Max log number in one second.
#define SAVE_LOG_INTERVAL   5    //Save log interval in second. Default: 5
#define LOG_NUMBER_LIMIT  ONE_SECOND_MAX_LOG * SAVE_LOG_INTERVAL
#define BUFFER_RATE      2    //Buffer rate
#define BUFFER_SIZE      LOG_NUMBER_LIMIT * BUFFER_RATE

//The flag to indicate whether the log should be writen to log file.
struct tDebugLogFlag
{
  bool bLogFlag;      //If all of below are false, it is false.
  bool bLinkRetry;    //For serial link re-try.
  bool bSocketConnect;  //For socket connect and disconnect.
  bool bServerCommand;  //For Server command and reply.
  bool bSWCCommand;    //For SWC command and reply.
};

//The structure for log		//040525 Yu Wei
//typedef struct tLogMessage
//{
//	unsigned int	unTimeSecond;	//System clock seconds.
//	unsigned int	unTimeMSecond;	//System clock milli-seconds.
//	unsigned int	unTimerLow;		//Timer low part value.
//	char			acLogMsg[256];	//Log.
//};

//#define LOG_MSG_SZ 512
#define LOG_MSG_SZ 2048
struct tLogMessage
{
  BOOL_T   isTimeStamp;   // include time stamp flag
  double   unTimeSecond;  //System clock seconds.
  long     unTimeMSecond;  //System clock milli-seconds.
  unsigned int  unTimerLow;    //Timer low part value.
  char     acLogMsg[LOG_MSG_SZ];  //Log.
};


class CLOG
{

private:

  char m_acLogFilePrefix[10];  //The prefix of log file name. One or two alphabet.
  char m_acLogFileName[20];  //Log file name. Prefix + 6 digits (YYMMDD) + extension.

  int m_nStoreLogDays;    //The day when the log file kepts in the disk.

  int m_nCurrentYear;      //The current year.  //040308 Yu Wei
  int m_nCurrentMonth;    //The current month.//040308 Yu Wei
  int m_nCurrentDate;      //The current date.  //040308 Yu Wei

  pthread_mutex_t m_tWriteFileMutex;
  pthread_mutex_t LogBufferMutex;
  tLogMessage m_tLogBufferCircle[BUFFER_SIZE];  //040525 Yu Wei.
//	tLogMessage *m_tLogBufferCircle;

  int    m_nLogStartPoint;  //Start point for log. 040525 Yu Wei.
  int    m_nLogEndPoint;    //End point for log. 040525 Yu Wei.

  int    m_nWriteFileFailTimes;
  int GetFileName(char *);
  E_ERR_T DeleteUnusedFile(timespec tTime);
  bool CheckLogFile(char *acFindFile,char *acCheckFile);

public:
  int    m_nLogNumber;    //The log number that have not saved to disk. 040525 Yu Wei.

  CLOG(char *);
  ~CLOG(void);

  void LogMessage(char *acLogMessage);
  void LogStart(void);
  void CheckDaySwitch(void);
  int GetDiskFreeSpace(char *acDirName);
  E_ERR_T SendMsg(char *acLogMessage);
  E_ERR_T SaveLog(void);    //040525 Yu Wei
//	void LogAtStartUp(char *);

};



void LogTaskSpawn(void);  //040525 Yu Wei
//void LogTask(void);			//040525 Yu Wei
void *LogTaskThread(void *arg);
void StopLogTask(void);    //040525 Yu Wei


extern CLOG *g_pEventLog;            //Object for Event log.
extern CLOG *g_pDebugLog;            //Object for Debug log.
extern tDebugLogFlag g_tDebugLogFlag;      //Flag for Debug log.
                   //To determine wether the message should log to debug log file.


#endif /* _CMM_LOG_H */

