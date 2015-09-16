#include <stdio.h>
#include <time.h>
#include <stdlib.h>


#define _MM_DENORMALS_ZERO_MASK   0x0040
#define _MM_DENORMALS_ZERO_ON     0x0040
#define _MM_DENORMALS_ZERO_OFF    0x0000
 
#define _MM_SET_DENORMALS_ZERO_MODE(mode)                                   \
                _mm_setcsr((_mm_getcsr() & ~_MM_DENORMALS_ZERO_MASK) | (mode))
#define _MM_GET_DENORMALS_ZERO_MODE()                                       \
                (_mm_getcsr() & _MM_DENORMALS_ZERO_MASK)


float return_rand_float(){
    time_t t;
    srand((unsigned) time(&t));
    return (float)(rand() % 100);
}

int return_rand_int(){
//    srand(time(NULL));
    int a = rand();
    return a;
}


