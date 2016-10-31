#ifndef _LEELOG_H
#define _LEELOG_H
//#include <pthread.h>
#include <stdio.h>

typedef enum {ALLOC, FREE} LOG_TYPE;

void m_log(char *msg, int size, LOG_TYPE mode);

#define debug(format, args...) fprintf(stderr, format, ##args)

#define Malloc(ptr, s) ({\
    ptr = malloc((s)*sizeof(*(ptr)));\
    if (ptr == NULL) m_log(#ptr, 0, ALLOC);\
    else m_log(#ptr, (s)*sizeof(*(ptr)), ALLOC);\
    })

#define Free(ptr) ({\
        if (ptr != NULL){\
            m_log(#ptr, sizeof(*(ptr)), FREE);\
            free(ptr);\
            ptr = NULL;\
        }\
        })

#endif
