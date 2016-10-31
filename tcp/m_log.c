#include "m_log.h"
#include <stdio.h>
#include <pthread.h>

static pthread_mutex_t WriteLog = PTHREAD_MUTEX_INITIALIZER;
extern FILE *LogFp;

void m_log(char *msg, int size, LOG_TYPE mode)
{
    if (LogFp == NULL) return;
    if (mode == ALLOC) //allocte log
    {
        pthread_mutex_lock(&WriteLog);
        fprintf(LogFp, "+m:%d \tBytes\t| %s\n", size, msg);
        pthread_mutex_unlock(&WriteLog);
    }
    else //free log
    {
        pthread_mutex_lock(&WriteLog);
        fprintf(LogFp, "-m:%d \tBytes\t| %s\n", size, msg);
        pthread_mutex_unlock(&WriteLog);
    }
    int ret = fflush(LogFp);
    if (ret)
        debug("fflush failed\n");
}
