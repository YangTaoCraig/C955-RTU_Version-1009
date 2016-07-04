#ifndef COMMON_QNXWATCHDOG_
#define COMMON_QNXWATCHDOG_

#define RMM_MAIN             0
#define RMM_LINK             1
#define RMM_CONN1            2
#define RMM_CONN2            3
#define SWC_MAIN             4
#define SER_POLL_MAIN        5
#define SER_POLL_COPY        6
#define CLOCK                7
#define SWC_MD               8


extern timer_t               RMM_main_timer_id;
#define RMM_MAIN_TIMER_PULSE_CODE     _PULSE_CODE_MINAVAIL + RMM_MAIN


extern timer_t               RMM_link_timer_id;
#define RMM_LINK_TIMER_PULSE_CODE      _PULSE_CODE_MINAVAIL + RMM_LINK


extern timer_t               RMM_conn1_timer_id;
#define RMM_CONN1_TIMER_PULSE_CODE    _PULSE_CODE_MINAVAIL + RMM_CONN1


extern timer_t               RMM_conn2_timer_id;
#define RMM_CONN2_TIMER_PULSE_CODE    _PULSE_CODE_MINAVAIL + RMM_CONN2


#define SWC_MAIN_TIMER_PULSE_CODE      _PULSE_CODE_MINAVAIL + SWC_MAIN

//extern timer_t                 Clock_timer_id;
//#define CLOCK_TIMER_PULSE_CODE        _PULSE_CODE_MINAVAIL + CLOCK

//#define SWC_MULTI_DROP_TIMER_PULSE_CODE   _PULSE_CODE_MINAVAIL + SWC_MD
#define SERV_POLL_MAIN_TIMER_PULSE_CODE   _PULSE_CODE_MINAVAIL + SER_POLL_MAIN
#define SERV_POLL_COPY_TIMER_PULSE_CODE   _PULSE_CODE_MINAVAIL + SER_POLL_COPY

extern int WatchDogChid;

#endif /*COMMON_QNXWATCHDOG_*/
