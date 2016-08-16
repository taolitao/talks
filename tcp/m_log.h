#ifndef _LEELOG_H
#define _LEELOG_H
//#include <pthread.h>
#include <stdio.h>


void m_log(char *msg, int size, FILE *fs, int mode);

#endif

#ifndef _LEE_MEMORY
#define _LEE_MEMORY


#define Malloc(ptr, log_fs, s)  malloc((s)*sizeof(*(ptr)));\
    m_log(#ptr, (s)*sizeof(*(ptr)), log_fs, 1)

#define Free(ptr, log_fs) m_log(#ptr, sizeof(*(ptr)), log_fs, 0);\
    free(ptr)

#endif
