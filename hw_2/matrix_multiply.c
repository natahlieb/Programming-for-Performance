#include <stdio.h>
#include <time.h> 
#include "accessory_methods.h"
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <papi.h>
//#include "/home/natahlieb/Downloads/papi-5.3.2/src/papi.h"
//first create two matrixes, a and b 
//a is nxm matrix, b is mxp matrix

//matricies should be filled with random numbers- finite number which will not result in inf, nan, or denormal results

void main(int argc, char *argv[]){


    srand(time(NULL));
    if(PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
        exit(0);
    }   





//flush cache
    const int size = 20*1024;
    char *c = (char *)malloc(size);
    int n;
    for(n=0; n<0xffff; n++) {
       int m =0;
       for(m=0; m< size; m++) {
          c[m] = m*n;
        }
     }






    //set up arrays 
    int dimension = 0;
    dimension = atoi(argv[1]); 
//    printf("\n\n%d\n", dimension);
    
    float array_a[dimension][dimension];
    float array_b[dimension][dimension];
    float array_c[dimension][dimension];


//    printf("length of arraya: %d\n", sizeof(array_a)/sizeof(array_a[0]));
//    printf("length of arrayb: %d\n", sizeof(array_b)/sizeof(array_b[0]));
//    printf("length of arrayc: %d\n", sizeof(array_c)/sizeof(array_c[0]));

    int i, j;
    for(i=0; i < dimension; i++){
        for(j=0; j<dimension; j++){
            array_a[i][j]=return_rand_float();
    //        printf("a[%d][%d]: %d", i, j, array_a[i][j]);
            array_b[i][j]=return_rand_float();
  //          printf("\nb[%d][%d]: %d\n", i, j, array_b[i][j]);
        }
    }

//    printf("\nfirst in a: %d, first in b: %d", array_a[0][0], array_b[0][0]);

    //do actual matrix multiplication

    i=0; 
    j=0; 
    int k=0;

    int NUM_EVENTS =5;
    int Events[5]= {PAPI_TOT_INS, PAPI_TOT_CYC, PAPI_FP_INS, PAPI_L1_TCM, PAPI_L2_TCM};
    long_long values[NUM_EVENTS];
    if(PAPI_start_counters(Events, NUM_EVENTS) != PAPI_OK){

        printf("\nERROR1\n");
        exit(0);
    }
  

    for(i=0; i <dimension; i++){
        for(j=0; j<=dimension; j++){
            array_c[i][j]=0;
            for(k=0; k<dimension; k++){
              array_c[i][j] = array_c[i][j] + array_a[i][k] * array_b[k][j];
//              printf("\na[%d][%d]: %d,ba[%d][%d]:%d -----> c[%d][%d]: %d",i, k, array_a[i][k], k, j, array_b[k][j], i, j, array_c[i][j]);
            }
        }
    }

    if(PAPI_stop_counters(values, NUM_EVENTS) != PAPI_OK) {
        printf("\nERROR2\n");
        exit(0);
    }


   printf("%llu, %llu, %llu, %llu, %llu\n", values[0], values[1], values[2], values[3], values[4]);
   // printf("total instructions %llu, total cycles %llu, floating point instructions %llu, l1 misses %llu, l2 misses %llu\n", values[0], values[1], values[2], values[3], values[4]);
}


