#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "accessory_methods.h"
#include <mmintrin.h>    // MMX
#include <xmmintrin.h>  // SSE
#include <emmintrin.h>  //SSE2
#include <pmmintrin.h> //SSE3
#include <smmintrin.h>
#include <stdint.h>
#include <malloc.h>
#include <sys/time.h>

void main(){

    float *x,*y, *temp_x, *temp_y, *h, *temp_h;

    int size_x=1600;
    int size_y=800;


    if( NULL == (x = memalign(16, sizeof(float)*size_x))){
        printf("\nfailed for x\n");
        exit(1);
    }
    if(NULL == ( y = memalign(16, sizeof(float)*size_y))){
        printf("\nfailed for y\n");
        free(x);
        exit(1);

    }
    if(NULL == (h = memalign(16, sizeof(float)*16))){
        printf("\nfailed for h\n");
        free(x);
        free(y);
        exit(1);
    }


    int i;

    //fill up our arrays
    for(i=0; i < size_x; i++){
        x[i]= (float)(i*1000);
    }
    for(i=0; i < size_y; i++){
        if(i<4){
            y[i]= (float)0;
        }
        else {
            y[i]=(float)(i*100);
        }
    }

    int count =0;
    for(i=0; i<16; i+=4){
        h[i]=1+count;
        h[i+1]=1+count;
        h[i+2]=1+count;
        h[i+3]=1+count;
        count+=1;
    }


    // print_arrays(x, size_x, y, size_y, h, 0);

    //call actual functions -> uncomment for type of function desired
    // warmup_scalar_part2(x, y, size_y, h);
    warmup_vector_part2(x, y, size_y, h);


    //print results after operations
    // print_arrays(x, size_x, y, size_y, h, 0);


    free(x);
    free(y);
    free(h);


}

void warmup_scalar_part2(float* x, float* y, int size, float* h){
    int i;
    for(i=4; i< size; ++i){
        y[i]= h[3*4] * x[i-3] + h[2*4] * x[i-2] + h[1*4] * x[i-1] + h[0] * x[i];
    }

}


void warmup_vector_part2(float* x, float* y, int size, float* h){
    int i;
    struct timeval start;
    struct timeval end;
    __m128 RY, RX_3, RX_2, RX_1, RX, R_h, R_h2, R_h3, R_h4, R_temp1, R_temp2;

    //load stationary vector values
    R_h = _mm_load_ps(&h[0]);
    R_h2= _mm_load_ps(&h[4]);
    R_h3 = _mm_load_ps(&h[8]);
    R_h4 = _mm_load_ps(&h[12]);

    gettimeofday(&start, NULL);
    //perform actual vector information
    for(i=4; i<size; i+=4){
        RX_3 = _mm_load_ps(&x[i-3]);
        RX_2 = _mm_load_ps(&x[i-2]);
        RX_1 = _mm_load_ps(&x[i-1]);
        RX = _mm_load_ps(&x[i]);
        R_temp1 = _mm_mul_ps(R_h4, RX_3);
        R_temp2 = _mm_mul_ps(R_h3, RX_2);
        RY = _mm_add_ps(R_temp1, R_temp2);
        R_temp1 = _mm_mul_ps(R_h2, RX_1);
        R_temp2 = _mm_add_ps(R_temp1, RX);
        RY = _mm_add_ps(RY, R_temp2); 
        _mm_store_ps(&y[i], RY);     
    }

    //printout timeing information
    gettimeofday(&end, NULL);
    long int seconds = (end.tv_sec - start.tv_sec);
    long int microseconds = (end.tv_usec - start.tv_usec);
    printf("/n/nruntime was %ld seconds, %ld microseconds\n\n", seconds, microseconds);

}


