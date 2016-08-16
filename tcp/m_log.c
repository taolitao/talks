#include "m_log.h"
#include <stdio.h>
//#include <pthread.h>
#define debug(format, args...) fprintf(stderr, format, ##args)

void m_log(char *msg, int size, FILE *fs, int mode)
{
    //FILE *fs = fopen("./server.log", "aw");
    if (mode) //allocte log
        fprintf(fs, "+m:%d \tBytes\t| %s\n", size, msg);
    else //free log
        fprintf(fs, "-m:%d \tBytes\t| %s\n", size, msg);
    int ret = fflush(fs);
    if (ret)
        debug("fflush failed\n");
}
