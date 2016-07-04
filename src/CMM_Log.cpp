/************************************************************
*                              *
*  Project:    C830 RTU                  *
*  Module:    CMM                      *
*  File :      CMM_Log.cpp                  *
*  Author:    Yu Wei                    *
*                              *
*  Copyright 2003 Singapore Technologies Electronics Ltd.  *
*************************************************************

**************************************************************

DESCRIPTION

  This file define log class.
  The log file name is defined as follow:
  PPYYMMDD
  PP  -- Two characters prefix.
  YY  -- Year.
  MM  -- Month.
  DD  -- Date.

*************************************************************
Modiffication History
---------------------

Version: CMM D.1.1.1
  01  3 April 2003, Yu Wei,
    Start to write.

  02 3 June 2003,
    Added log flag. If the flag is false, don't write the log
    to file.
    LogStart will write version.


Version: CMM D.1.3.0
  01  13 August 2003, Yu Wei,
    The log object and variables are declared in this defination
    files.

  02  01 September 2003, Yu Wei
    If disk free space is less than MIN_FREE_DISK_SPACE,
    m_nStoreLogDays--. The store log file is not fixed to 30 days.
    If m_nStoreLogDays = MIN_DAY_FILE_STORE, don't write log file.
    When day switch over, the m_nStoreLogDays reset to
    MAX_DAY_FILE_STORE.

Version: CMM D.1.3.4
  01  03 March 2004, Yu Wei
    Fixed bug in DeleteUnusedFile().
    Prevent recursive loop found using WindNavigator  call tree.
    DeleteUnusedFile() will call LogMessage() again if cannot deleted file.
    LogMessage() will call DeleteUnusedFile() when disk full.
    Refer to PR:OASYS-PMS 0026.

Version: CMM D.1.3.5
  01  08 March 2004, Yu Wei
    Modified GetFileName() to store year, month, date for day switch
    checking.
    Modified CheckDaySwitch(). Day switch over should check year,
    month, date and when day switch over, write year information to
    log file.
    Refer to PR:OASYS-PMS 0150.

  02  08 April 2004, Yu Wei
    Converted GetDiskFreeSpace()'s ioctl(), LogMessage()'s sprintf,
    data type and deleted DeleteUnusedFile()'s acTemp[] to avoid
    compiler warning.

Version: CMM D.1.3.6
  01  25 May 2004, Yu Wei
    Changed log file processing.
    Added a log task. Every 5 seconds or log number reach the max
    number, the task writes log to CompactFlash disk. The other task
    only write log to memory. This can reduce writing CompactFlash
    times.
    Added LogTaskSpawn(), LogTask(), StopLogTask() and SaveLog().
    Added g_bLogTaskWorking and g_nLogTaskID.
    Re-designed LogMessage().
    Refer to PR:OASYS-PMS 0212.
**************************************************************/
#include <sys/neutrino.h>
#include <pthread.h>
#include <iostream.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <dirent.h>
#include <sys/statvfs.h>

#include "cfg_def.h"
#include "type_def.h"
#include "err_def.h"
#include "sys_ass.h"
#include "swc_def.h"
#include "sys_def.h"
#include "sys_ext.h"

#include "Common.h"
#include "CMM_Log.h"
#include "CMM_Timer.h"      //040525 Yu Wei

CLOG *g_pEventLog;            //Object for Event log.
CLOG *g_pDebugLog;            //Object for Debug log.
tDebugLogFlag g_tDebugLogFlag;      //Flag for Debug log.
//To determine wether the message should log to debug log file.
bool g_bLogTaskWorking;          //The flag for  log task working. 040525 Yu Wei
pthread_t g_nLogTaskID;            //Log task ID.  //040525 Yu Wei

/*******************************************************************************
Purpose:
  This routine create a write log process thread.
  Added in 25 May 2004, Yu Wei

*******************************************************************************/
void LogTaskSpawn(void)
{
  E_ERR_T rstatus = E_ERR_Success;
  g_bLogTaskWorking = true;

  rstatus = SYS_ThrdInit(E_SYS_TLB_CmmLog);
  if(rstatus != E_ERR_Success)
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [CMM] LogTaskSpawn, create Log thread\n");
    #endif // CFG_PRN_ERR
    g_pEventLog->LogMessage((CHAR *)
      "[CMM] LogTaskSpawn, fail to Log thread\n");
    return;
  }

  g_nLogTaskID =
    pSYS_CB->thrd_ctrl[E_SYS_TLB_CmmLog].thrd_id;
}
/*******************************************************************************
Purpose:
  This routine task process log file writing.
  Added in 25 May 2004, Yu Wei

*******************************************************************************/
void *LogTaskThread(void *arg)
{
  E_ERR_T rstatus = E_ERR_Success;
  struct tTimerValue tTimer;
  int nSaveLog=0;  // number of times to attempt

  while(1)
  {
    if(g_bLogTaskWorking == false)
    {
      #if ((CFG_DEBUG_LOG) && (defined CFG_DEBUG_MSG))
      printf("[CMM] LogTaskThread, log thread not working\n");
      #endif // ((CFG_DEBUG_LOG) && (defined CFG_DEBUG_MSG))
      pthread_detach(pthread_self());
      GetTimerValue(&tTimer);
      g_pEventLog->SaveLog();
      if(g_tDebugLogFlag.bLogFlag == true)
      {
        rstatus = g_pDebugLog->SaveLog();
        if(rstatus != E_ERR_Success)
        {
          #ifdef CFG_PRN_ERR
          printf("ERR  [CMM] LogTaskThread, save log fail, rstatus = %d\n",
                 rstatus);
          #endif // CFG_PRN_ERR
        }
      }
      delay(100);
      return NULL;
    }

    nSaveLog++;
    if((nSaveLog >= SAVE_LOG_INTERVAL) ||
       (g_pEventLog->m_nLogNumber > LOG_NUMBER_LIMIT) ||
       ((g_pDebugLog->m_nLogNumber > LOG_NUMBER_LIMIT) &&
       (g_tDebugLogFlag.bLogFlag == true)))
    {
      GetTimerValue(&tTimer);

      nSaveLog = 0;
      g_pEventLog->SaveLog();

      if(g_tDebugLogFlag.bLogFlag == true)
      {
        rstatus = g_pDebugLog->SaveLog();
        if(rstatus != E_ERR_Success)
        {
          #ifdef CFG_PRN_ERR
          printf("ERR  [CMM] LogTaskThread, save log fail, rstatus = %d\n",
                 rstatus);
          #endif // CFG_PRN_ERR
        }
      }
    }

    //Check day switch over, process log files.
    g_pEventLog->CheckDaySwitch();

    if(g_tDebugLogFlag.bLogFlag == true)  //030703 Yu Wei
    {
      g_pDebugLog->CheckDaySwitch();
    }

    delay(CMM_LOG_SLEEP);
  }
}

/*******************************************************************************
Purpose:
  This routine stops warchdog task.
  Added in 25 May 2004, Yu Wei
*******************************************************************************/
void StopLogTask(void)
{
  g_bLogTaskWorking = false;
}

/*******************************************************************************
Purpose:
  CLOG constructor. Create semiphone for read/write log file and get log
  filename.

Input
  acFilePrefix  -- Log file name prefix. (in)

*******************************************************************************/
CLOG::CLOG(char *acFilePrefix)
{
  pthread_mutexattr_t attr;
  memset(m_acLogFilePrefix, 0, sizeof(m_acLogFilePrefix));
  strcpy(m_acLogFilePrefix,acFilePrefix);
  pthread_mutexattr_init(&attr);
  pthread_mutex_init(&m_tWriteFileMutex , &attr);
  pthread_mutex_init(&LogBufferMutex, &attr);

  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LOG))
  printf("[CMM] CLOG::CLOG, create filename using log prefix %s, file prefix "
         "%s\n",
         m_acLogFilePrefix, acFilePrefix);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LOG))
  GetFileName(m_acLogFilePrefix);

  m_nStoreLogDays = MAX_DAY_FILE_STORE;

  m_nLogStartPoint = 0;  //040525 Yu Wei.
  m_nLogEndPoint = 0;    //040525 Yu Wei.
  m_nLogNumber = 0;    //040525 Yu Wei.

  memset(m_tLogBufferCircle, 0, BUFFER_SIZE);

}

/*******************************************************************************
Purpose:
  CLOG destructor.

*******************************************************************************/
CLOG::~CLOG(void)
{
  pthread_mutex_destroy(& m_tWriteFileMutex);
  pthread_mutex_destroy(& LogBufferMutex);
  //delete semaphore
}

/*******************************************************************************
Purpose:
  Write start message. It includes software version number.
  It will be called when RTU power up. It also delete the un-used
  file that the file was created before m_nStoreLogDays days ago.

History

  Name         Date         Remark
  ----         ----         ------
 Bryan Chong  22-Oct-2010  Remove add offset for local time zone.
                           DeleteUnusedFile routine will use local time zone

*******************************************************************************/
void CLOG::LogStart(void)
{
  FILE *tfd;
  struct timespec tTime;
  char acTemp[128];

  clock_gettime(CLOCK_REALTIME, &tTime);

  DeleteUnusedFile(tTime);

  if((tfd = fopen(m_acLogFileName,"a+")) != NULL)
  {
    fputs("\n", tfd);
    fputs("*****RTU (c) ST Electronics 2015, release 1.0.0.9(20160215.01)*****\n", tfd);
    //sprintf(acTemp, "RTU Software Version: %04X\n", VERSION_NUMBER);
   // fputs(acTemp, tfd);

    fputs("*********Property of Issuing Entities(ST ELECTRONICS 2015)*********\n", tfd);
    fputs("*****Cannot be distributed or reproduced without authorization*****\n", tfd);
    fclose(tfd);
  }
  LogMessage((CHAR *)"Start\n");
}

/*******************************************************************************
Purpose:
  Get log file name.

Input
  acFilePrefix  -- Log file name prefix. (in)

Return
  OK    -- Get file name.
  ERROR  -- The file is invalid.

*******************************************************************************/
int CLOG::GetFileName(char *acFilePrefix)
{
  struct timespec tTime;
  struct tm *tFormatTime;
  FILE *tfd;
  int nReturn = ERROR;

  clock_gettime(CLOCK_REALTIME, &tTime);  //Get current date and time.
  //tTime.tv_sec += g_tRTUConfig.nLocalTimeZone;//timezone GMT+8:00
  //tFormatTime = gmtime(&tTime.tv_sec);
  tFormatTime = localtime(&tTime.tv_sec);

  sprintf(m_acLogFileName, "%s%s%02d%02d%02d.log",
    CFG_RTU_LOG_FOLDER,
    acFilePrefix,
    (tFormatTime->tm_year -100),
    (tFormatTime->tm_mon + 1),
    tFormatTime->tm_mday);    //Get log file name.

  pthread_mutex_lock(&m_tWriteFileMutex);

  //Check whether the file is valid.
  if((tfd = fopen(m_acLogFileName,"a+")) == TYP_NULL)
  {
    #ifdef CFG_PRN_ERR
    printf("[CMM] CLOG::GetFileName, cannot open file %s\n", m_acLogFileName);
    #endif // CFG_PRN_ERR
    nReturn = ERROR;
  }
  else
  {
    m_nCurrentYear = tFormatTime->tm_year;    //040308 Yu Wei
    m_nCurrentMonth = tFormatTime->tm_mon;    //040308 Yu Wei
    m_nCurrentDate = tFormatTime->tm_mday;    //040308 Yu Wei
    fclose(tfd);
    nReturn = OK;
  }
  pthread_mutex_unlock(&m_tWriteFileMutex);

  return nReturn;
}

/*******************************************************************************
Purpose:
  Check date switch over and deletes un-used files.

Return
  Nil.

HISTORY

   NAME            DATE                    REMARKS

 Bryan Chong     21-Sep-2010      Update log header string to use local time


*******************************************************************************/
void CLOG::CheckDaySwitch(void)
{
  struct timespec tTime;
  struct tm *tFormatTime;
  FILE *tfd;

  //Get current UTC time
  clock_gettime(CLOCK_REALTIME, &tTime);

  //timezone GMT+8:00
  //tTime.tv_sec += g_tRTUConfig.nLocalTimeZone;

  //Change format to tm
  //tFormatTime = gmtime(&tTime.tv_sec);
  // 20100921 BC
  tFormatTime = localtime(&tTime.tv_sec);

  //Check date, month, and day switch  040308 Yu Wei
  if(( m_nCurrentDate != tFormatTime->tm_mday ) ||
     ( m_nCurrentMonth != tFormatTime->tm_mon ) ||
     ( m_nCurrentYear != tFormatTime->tm_year ) )
  {
    m_nStoreLogDays = MAX_DAY_FILE_STORE;

    //Get new file name
    GetFileName(m_acLogFilePrefix);

    //Log current year, month, date 040308 Yu Wei
    if((tfd = fopen(m_acLogFileName,"a+")) != NULL)
    {
      fputs(ctime (&tTime.tv_sec),tfd);
      fputs("******************************\n",tfd);
      fclose(tfd);
    }

    //Delete log file that the creating day is more than m_nStoreLogDays
    DeleteUnusedFile(tTime);

  }

}

/*******************************************************************************
Purpose:
  Delete the log file that had been created before m_nStoreLogDays ago.

Input
  tTime  -- Current date and time.

Return
  Nil.

History

  Name         Date         Remark
  ----         ----         ------
 Bryan Chong  05-Jan-2010  Update log directory to log folder using definition
                           CFG_RTU_LOG_FOLDER [PR27]
 Bryan Chong  20-Jun-2010  Construct the filename with absolute path before
                           passing in to remove() routine [PR45]
 Bryan Chong  05-Jul-2010  Implement filename prefix checking before continuing
 Bryan Chong  07-Sep-2010  Add missing condition for checking DB files
 Bryan Chong  22-Oct-2010  Update to use local time
*******************************************************************************/
E_ERR_T CLOG::DeleteUnusedFile(timespec tTime)
{
  struct tm *tFormatTime;
  char acCheckDate[16];
  DIR *tDirHandle;
  dirent *tFileName;
  int nTry=1000;
  CHAR absfilename[100] = {0};

  //Get the time m_nStoreLogDays ago.
  tTime.tv_sec -= ONE_DAY_SECONDS * m_nStoreLogDays;

  //Change format to tm
  //tFormatTime = gmtime(&tTime.tv_sec);
  tFormatTime = localtime(&tTime.tv_sec);
  sprintf(acCheckDate,"%02d%02d%02d",
    (tFormatTime->tm_year -100),(tFormatTime->tm_mon + 1),
     tFormatTime->tm_mday);

  //Open log file directory.
  if((tDirHandle = opendir(CFG_RTU_LOG_FOLDER)) == TYP_NULL)
  {
     #ifdef CFG_PRN_ERR
     printf("ERR  [CMM] CLOG::DeleteUnusedFile, cannot open directory %s, %s\n",
            CFG_RTU_LOG_FOLDER, strerror(errno));
     #endif // CFG_PRN_ERR
     return E_ERR_LOG_OpenDirectoryFail;
  }


  while(nTry--)
  {
    if((tFileName = readdir(tDirHandle)) == TYP_NULL)  //Search files.
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_LOG)
      printf("ERR  [CMM] CLOG::DeleteUnusedFile, read directory encounters the "
             "end of the directory stream\n");
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LOG))
      break;
    }
    else
    {
      #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_LOG)
      printf("[CMM] CLOG::DeleteUnusedFile, open and read directory. "
             "Dir handle 0x%x, filename %s\n", tDirHandle, tFileName->d_name);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LOG))

      // 20100705 BC. Implement filename prefix checking befor continuing
      // 20100907 BC (Rqd by ZSL)
      if((strncasecmp((const char *)tFileName->d_name, CFG_LOG_PREFIX_EV, 2)
         != 0) &&
         (strncasecmp((const char *)tFileName->d_name, CFG_LOG_PREFIX_DB, 2)
         != 0))
        continue;

      // Check if the file created is expired based on duration
      // defined at m_nStoreLogDays .
      if(CheckLogFile(tFileName->d_name, acCheckDate) == false)
      {
        #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_LOG)
        printf("[CMM] CLOG::DeleteUnusedFile, deleting log file %s, date %s\n",
               tFileName->d_name, acCheckDate);
        #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LOG))

        // [PR45]
        // Construct the absolute path filename, and delete the file
        sprintf(absfilename, "%s%s", CFG_RTU_LOG_FOLDER, tFileName->d_name);
        if(remove(absfilename) != TYP_NULL)
        {
          #ifdef CFG_PRN_ERR
          printf("ERR  [CMM] CLOG::DeleteUnusedFile, cannot delete file %s.\n",
                 absfilename);
          perror("ERR  [CMM] CLOG::DeleteUnusedFile, errno");
          #endif // CFG_PRN_ERR
        }else{
          #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_LOG)
          printf("[CMM] CLOG::DeleteUnusedFile, deleted filename %s\n",
                 absfilename);
          #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LOG))
        } //  if(remove(absfilename) != TYP_NULL)

      }// if(CheckLogFile(tFileName->d_name, acCheckDate) == false)
    }// if((tFileName = readdir(tDirHandle)) == TYP_NULL)
  } // while(nTry--)

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_LOG)
  printf("[CMM] CLOG::DeleteUnusedFile, close directory handle 0x%x\n",
         tDirHandle);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LOG))
  closedir(tDirHandle);

  return E_ERR_Success;
}// DeleteUnusedFile

/*******************************************************************************
Purpose:
  Check if the file is created in m_nStoreLogDays ago.

Input
  acFindFile  -- The buffer for being checked file name.
  acCheckFile  -- The buffer for check point.

Return
  ture  -- The file is created after m_nStoreLogDays ago
        or the file is not created by log boject.
  false  -- The file is created before m_nStoreLogDays ago.

*******************************************************************************/
bool CLOG::CheckLogFile(char *acFindFile,char *acCheckFile)
{
  bool bReturn = true;
  int nI;
  int nCMPResult;

  char acTemp[16];

  memset(acTemp, 0, sizeof(acTemp));
  strcpy(acTemp, acFindFile);

  #if ((defined CFG_DEBUG_MSG) && CFG_DEBUG_LOG)
  printf("[CMM] CLOG::CheckLogFile, check file %s for date %s\n",
         acFindFile, acCheckFile);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LOG))
  //check file prefix
  if(strncmp(acTemp, m_acLogFilePrefix, strlen(m_acLogFilePrefix)) == 0)
  {
    for(nI=2;nI<8;nI++)    //Check if it is a standard log file
    {
      if((acTemp[nI] < '0') ||(acTemp[nI] > '9'))
        break;
    }

    //Not a standard log file name.
    if(nI<8)
    {
      bReturn = true;
    }
    else
    {
      acTemp[8] = '\0';
      nCMPResult = strncmp(&acTemp[2], acCheckFile, strlen(&acTemp[2]));

      //delete old log file
      if(nCMPResult <= 0)
        bReturn = false;
      else
        bReturn = true;
    }
  }
  else
  {
    bReturn = true;
  }

  return bReturn;
}

/*******************************************************************************
Purpose:
  Get the free space size of the disk.

Input
  acDirName  -- Disk name.

Return
  Free space size of the disk.

*******************************************************************************/
int CLOG::GetDiskFreeSpace(char *acDirName)
{

  /*struct statvfs *tStatBuf;*/
  struct statvfs tStatBuf;
  unsigned long nDiskSpace = 0;

  /*tStatBuf = ( struct statvfs * )malloc(sizeof( struct statvfs));*/
  statvfs( acDirName, &tStatBuf );

  nDiskSpace = (tStatBuf.f_bavail) *(tStatBuf.f_bsize );
  /*free(buf);*/
  #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LOG))
  printf("[CMM] GetDiskFreeSpace, free disk space for %s: %d\n",
         acDirName, nDiskSpace);
  #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LOG))
  return nDiskSpace;
}

/*******************************************************************************
Purpose:
  Write message to log buffer with time stamp
  Re-designed in 25 May, 2004, Yu Wei

Input
  acLogMessage  -- Buffer for log message.

Return
  Nil.

*******************************************************************************/
void CLOG::LogMessage(char *acLogMessage)
{
  struct timespec tTime;
  struct tTimerValue tTimer;
  int nMsgLen;
  //struct tm *tFormatTime;

  //Get log time.
  clock_gettime(CLOCK_REALTIME, &tTime);
  //tFormatTime = localtime(&tTime.tv_sec);

  GetTimerValue(&tTimer);

  m_tLogBufferCircle[m_nLogEndPoint].unTimeSecond =(double) tTime.tv_sec ;
  m_tLogBufferCircle[m_nLogEndPoint].unTimeMSecond = tTime.tv_nsec/1000000;
  m_tLogBufferCircle[m_nLogEndPoint].unTimerLow = tTimer.Low;
  m_tLogBufferCircle[m_nLogEndPoint].isTimeStamp = E_TYPE_Yes;


  nMsgLen = strlen(acLogMessage);
  if(nMsgLen >= 256)
  {
    nMsgLen = 256;
    acLogMessage[255] = '\0';
  }

  memcpy(m_tLogBufferCircle[m_nLogEndPoint].acLogMsg, acLogMessage,nMsgLen);
  m_nLogEndPoint++;
  m_nLogNumber ++;
  if(m_nLogEndPoint >= BUFFER_SIZE)
  {
    m_nLogEndPoint = 0;
  }
  if(m_nLogEndPoint == m_nLogStartPoint-1)
  {
    memset(m_tLogBufferCircle[m_nLogStartPoint].acLogMsg, 0, 256);
    m_nLogStartPoint++;    //Delete first log.
    m_nLogNumber--;
  }

}
/*******************************************************************************
Purpose:
  Send message to circular log buffer. If message size greater than LOG_MSG_SZ,
  the message will truncated to LOG_MSG_SZ - 1.

Input
  acLogMessage  -- Buffer for log message.

Return
  Nil.

History

     Name       Date          Remark
     ----       ----          ------
 Bryan Chong  11-Jun-2010  Create initial revision

*******************************************************************************/
E_ERR_T CLOG::SendMsg(char *acLogMessage)
{
  int nMsgLen;

  nMsgLen = strlen(acLogMessage);
  if(nMsgLen >= LOG_MSG_SZ)
  {
    // message exceed LOG_MSG_SZ and truncate to LOG_MSG_SZ - 1
    nMsgLen = LOG_MSG_SZ;
    acLogMessage[LOG_MSG_SZ - 1] = '\0';
    #if ((defined CFG_PRN_WARN) && CFG_PRN_WARN_LOG)
    printf("WARN [LOG] CLOG::SendMsg, log string size %d exceed defined limit "
           "%d\n", nMsgLen, LOG_MSG_SZ);
    printf("  %s ...\n", acLogMessage);
    #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_LOG)
  }

  #if ((defined CFG_DEBUG_LOG) && CFG_DEBUG_LOG)
  printf("[LOG] CLOG::SendMsg, %d bytes msg:\n%s\n", nMsgLen, acLogMessage);
  #endif // ((defined CFG_PRN_WARN) && CFG_PRN_WARN_LOG)

  memset(m_tLogBufferCircle[m_nLogEndPoint].acLogMsg, 0, LOG_MSG_SZ);
  memcpy(m_tLogBufferCircle[m_nLogEndPoint].acLogMsg, acLogMessage, nMsgLen);
  m_tLogBufferCircle[m_nLogStartPoint].isTimeStamp = E_TYPE_No;
  m_nLogEndPoint++;
  m_nLogNumber ++;
  if(m_nLogEndPoint >= BUFFER_SIZE)
  {
    m_nLogEndPoint = 0;
  }
  if(m_nLogEndPoint == m_nLogStartPoint-1)
  {
    memset(m_tLogBufferCircle[m_nLogStartPoint].acLogMsg, 0, LOG_MSG_SZ);
    m_nLogStartPoint++;    //Delete first log.
    m_nLogNumber--;
  }

  return E_ERR_Success;

}

/*******************************************************************************
Purpose:
  Save log buffer to disk.
  If the disk free space is less than MIN_FREE_DISK_SPACE,
  the message will not be written to file.
  Added in 25 May, 2004, Yu Wei

Return
  Nil.

History

  Name         Date         Remark
  ----         ----         ------
 Bryan Chong  05-Jan-2010  Update log directory to log folder using definition
                           CFG_RTU_LOG_FOLDER [PR27]


*******************************************************************************/
E_ERR_T CLOG::SaveLog(void)
{
  E_ERR_T rstatus = E_ERR_Success;
  struct timespec tTime;
  char buffer[1024], buffer2[128];
  FILE *tfd;

  //No log in the buffer.
  if(m_nLogEndPoint == m_nLogStartPoint && m_nLogNumber == 0)
  {
    return rstatus;
  }

  //Check disk free space
  while(GetDiskFreeSpace((CHAR *) CFG_RTU_LOG_FOLDER) < MIN_FREE_DISK_SPACE)
  {
    m_nStoreLogDays--;

    clock_gettime(CLOCK_REALTIME, &tTime);
    rstatus = DeleteUnusedFile(tTime);
    SYS_ASSERT_RETURN(rstatus != E_ERR_Success, rstatus);

    if(m_nStoreLogDays <= MIN_DAY_FILE_STORE)
    {
      #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LOG))
      printf("[CMM] CLOG::SaveLog, log store day less than defined % day(s)\n",
              MIN_DAY_FILE_STORE);
      #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LOG))
      return rstatus;
    }
  }

  pthread_mutex_lock(&LogBufferMutex);
  if((tfd = fopen(m_acLogFileName,"a")) != NULL)
  {
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LOG))
    printf("[CMM] CLOG::SaveLog, open file %s handle, 0x%x\n",
            m_acLogFileName, tfd);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LOG))
    while(m_nLogEndPoint != m_nLogStartPoint)
    {
      if(m_tLogBufferCircle[m_nLogStartPoint].isTimeStamp)
      {
        tTime.tv_sec =
          (unsigned )m_tLogBufferCircle[m_nLogStartPoint].unTimeSecond;
        tTime.tv_nsec = 0;

        //show GMT+8:00
        tTime.tv_sec += g_tRTUConfig.nLocalTimeZone;

        sprintf(buffer2, "%s", ctime(&tTime.tv_sec));
        buffer2[19] = '\0';
        sprintf(buffer,"%s.%03d %s\n", &buffer2[4],
        m_tLogBufferCircle[m_nLogStartPoint].unTimeMSecond,
        m_tLogBufferCircle[m_nLogStartPoint].acLogMsg);
      }else{
        memset(buffer, 0, sizeof(buffer));
        memcpy(buffer, m_tLogBufferCircle[m_nLogStartPoint].acLogMsg,
               strlen(m_tLogBufferCircle[m_nLogStartPoint].acLogMsg));
      }

      if(fputs(buffer,tfd) == EOF)
      {
        m_nWriteFileFailTimes++;
      }
      else
      {
        g_tRTUStatus.bCFCardError = false;
        if(m_nWriteFileFailTimes > 0)
        {
          m_nWriteFileFailTimes = 0;
        }
      }
      if(m_nWriteFileFailTimes > 5 )
      {
        //CF is bad,marked it ,need to change a new CF
        g_tRTUStatus.bCFCardError = true;
      }
      //after log the message ,clear log message buffer
      memset(m_tLogBufferCircle[m_nLogStartPoint].acLogMsg, 0, LOG_MSG_SZ);
      m_tLogBufferCircle[m_nLogStartPoint].isTimeStamp = E_TYPE_No;
      m_nLogStartPoint++;
      m_nLogNumber--;
      if(m_nLogStartPoint >= BUFFER_SIZE)
      {
        m_nLogStartPoint = 0;
      }
    }
    fclose(tfd);
    #if ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LOG))
    printf("[CMM] CLOG::SaveLog, close file %s handle, 0x%x\n",
            m_acLogFileName, tfd);
    #endif // ((defined CFG_DEBUG_MSG) && (CFG_DEBUG_LOG))
    pthread_mutex_unlock(&LogBufferMutex);
  }
  else
  {
    #ifdef CFG_PRN_ERR
    printf("ERR  [CMM] CLOG::SaveLog, open file %s fail, %s\n",
           m_acLogFileName, strerror(errno));
    #endif // CFG_PRN_ERR

    #ifdef _CFG_PRN_ERR
    perror("ERR  [CMM] CLOG::SaveLog, Error");
    #endif // CFG_PRN_ERR
    return E_ERR_LOG_OpenFileFail;
  }

  return E_ERR_Success;
}// SaveLog
