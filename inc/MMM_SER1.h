#ifndef MMM_SER1_H_
#define MMM_SER1_H_

class MMM_SER1
{
private:
	
  char m_MMM_SER1_CMD[100];
  int m_MMM_SER1_fd;
  speed_t m_MMM_SER1_BaudRate; 
  unsigned int m_MMM_SER1_ucParity ;
  unsigned int m_MMM_SER1_DataBits ;
  unsigned int m_MMM_SER1_StopBits ;
  bool openSerial;
	
public:
	
   MMM_SER1();
   MMM_SER1(char *, speed_t, unsigned int, unsigned int, unsigned int);
  ~MMM_SER1();
	
  void MMM_SER1_Main();
  bool MMM_SER1_GetCMD();
  bool MMM_SER1_ProcessCMD();
};


bool MMM_SER1_TaskSpawn();
void * MMM_SER1_Task(void *arg);
void Stop_MMM_SER1_task();

#endif /*MMM_SER1_H_*/
