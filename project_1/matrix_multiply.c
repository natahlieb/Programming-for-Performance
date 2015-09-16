#include <stdio.h>
#include <time.h> 
#include "accessory_methods.h"
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
//#include <papi.h>
#include <math.h>
#include <string.h>

void main(int argc, char *argv[]){


    float real_time, proc_time,mflops;
    long long flpops;
    float ireal_time, iproc_time, imflops;
    long long iflpops;
    int retval;
    struct timeval start;
    struct timeval end;
    /***********************************************************************
     *    * if PAPI_FP_OPS is a derived event in your platform, then your       * 
     *       * platform must have at least three counters to support PAPI_flops,   *
     *          * because PAPI needs one counter to cycles. So in UltraSparcIII, even *
     *             * the platform supports PAPI_FP_OPS, but UltraSparcIII only has  two  *
     *                * available hardware counters and PAPI_FP_OPS is a derived event in   *
     *                   * this platform, so PAPI_flops returns an error.                      *
     *                      ***********************************************************************/
    srand(time(NULL));
    /*    if(PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
          exit(0);
          }   
          */
    //set up arrays 
    int dimension = 0;
    dimension = atoi(argv[1]);

    double  **array_a= malloc(dimension * sizeof(double*));
    double  **array_b= malloc(dimension * sizeof(double*));
    double  **array_c= malloc(dimension * sizeof(double*));



    int i, j;
    for(i=0; i < dimension; i++){
        array_a[i]=malloc(dimension * sizeof(double));
        array_b[i]=malloc(dimension * sizeof(double));
        array_c[i]=malloc(dimension * sizeof(double));

    }

    for(i=0; i < dimension; i++){
        for(j=0; j<dimension; j++){
            array_a[i][j]=return_rand_double();
            //                        printf("a[%d][%d]: %f\n", i, j, array_a[i][j]);
            array_b[i][j]=return_rand_double();
            //                      printf("\nb[%d][%d]: %f\n", i, j, array_b[i][j]);
        }
    }


    //PAPI setup
    i=0; 
    j=0; 
    int k=0;

    /* int NUM_EVENTS =6;
       int Events[6]= {PAPI_LD_INS, PAPI_SR_INS, PAPI_FP_INS, PAPI_L1_DCM, PAPI_L2_TCM, PAPI_flops};
       long_long values[NUM_EVENTS];
       */
    int block =0;

    //find largest divisor- set this to our block size;
    if(dimension%2==0){
        block=2;
    }
    else {
        block=1;
    }

    /*
       while(block<100000000000){
       printf("\ndimension is %d, block size is %d, result is %d", dimension, block, dimension%block);
       if(dimension%block!=0){
       block=block-2;
       break;

       }
       block+=2;
       }
       */
    block=101; 

    int cycle_counter=0;
    int ii, jj, kk;
    double temp_a;
    int min_ii, min_kk, min_jj;
    gettimeofday(&start, NULL);

    for(ii=0; ii<dimension; ii+=block){

        for(jj=0; jj< dimension; jj+=block){

            for(kk=0; kk< dimension; kk+=block){
                min_ii = find_min(ii+block, dimension);
                for(i=ii; i< min_ii; i++){
                    min_kk = find_min(kk+block, dimension);
                    for(k=kk; k<min_kk; k++){
                        temp_a = array_a[i][k];
                        min_jj= find_min(jj+block, dimension);
                        for(j=jj; j<min_jj-3; j+=4){
                            array_c[i][j] += temp_a*array_b[k][j];
                            array_c[i][j+1]+= temp_a*array_b[k][j+1];
                            array_c[i][j+2]+= temp_a*array_b[k][j+2];
                            array_c[i][j+3]+= temp_a*array_b[k][j+2];
                        }
                    }

                }   

            }

        }

    }

    /*
     * int ii =0; 
     int jj =0;
     int kk =0;
     int temp_sum =0;
     int min_kk, min_jj;
     for(i; i< dimension; i++) {
     min_jj = find_min(jj+block, dimension);
     for(j=jj; j < min_jj; j++){
     temp_sum=0;
     min_kk = find_min(kk+block, dimension);
     for(k=kk; k<min_kk; k++) {
     temp_sum+= array_a[i][k] * array_b[k][j];
//temp_sum = temp_sum + is_valid_number(array_a[i][k] * array_b[k][j]);
} 
array_c[i][j]= array_c[i][j] + temp_sum; 
cycle_counter+=1;
}
}
*/
gettimeofday(&end, NULL);
//Perform actual multiplication

//original
/*
//ijk multiplication
for(i=0; i <dimension; i++){
for(j=0; j<=dimension; j++){
array_c[i][j]=0;
for(k=0; k<dimension; k++){
array_c[i][j] = array_c[i][j] + array_a[i][k] * array_b[k][j];
//printf("\na[%d][%d]: %d,ba[%d][%d]:%d -----> c[%d][%d]: %d",i, k, array_a[i][k], k, j, array_b[k][j], i, j, array_c[i][j]);
}
}
}
*/
/*if(PAPI_stop_counters(values, NUM_EVENTS) != PAPI_OK) {
  printf("\nERROR2\n");
  exit(0);
  }
  */

/*  if((retval=PAPI_flops( &real_time, &proc_time, &flpops, &mflops))<PAPI_OK)
    {    
    printf("retval: %d\n", retval);
    exit(1);
    }
    */  
long int seconds = (end.tv_sec - start.tv_sec);
long int microseconds = (end.tv_usec - start.tv_usec);
printf("run time was, %ld, seconds, %ld, microseconds\n", seconds, microseconds);

int stop;
for(stop=0; stop <dimension; stop++){
    free(array_a[stop]);
    free(array_b[stop]);
    free(array_c[stop]);        
}
free(array_a);
free(array_b);
free(array_c);


}


