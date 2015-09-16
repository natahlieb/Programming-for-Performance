#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>


#define _MM_DENORMALS_ZERO_MASK   0x0040
#define _MM_DENORMALS_ZERO_ON     0x0040
#define _MM_DENORMALS_ZERO_OFF    0x0000
 
#define _MM_SET_DENORMALS_ZERO_MODE(mode)                                   \
                _mm_setcsr((_mm_getcsr() & ~_MM_DENORMALS_ZERO_MASK) | (mode))
#define _MM_GET_DENORMALS_ZERO_MODE()                                       \
                (_mm_getcsr() & _MM_DENORMALS_ZERO_MASK)

double is_valid_number(double input) {
    //check to see if NAN
    if(__builtin_isnormal(input)==0){
      return 0;
    }
    return input;
}

float return_rand_float(){
    time_t t;
    return (float)(rand());
}

int return_rand_int(){
    int a = rand();
    return a;
}

double return_rand_double(){
    time_t t;
    return (double)(rand()%10);
}

int find_min(int a, int b){
   if(a>b) {
      return b;
   }
   return a;
}

