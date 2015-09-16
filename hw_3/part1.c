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


void main(){

    float *x,*y, *temp_x, *temp_y, *alpha, *temp_alpha;
    // int size_x=50;
    // int size_y=10;

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
    if(NULL == (alpha = memalign(16, sizeof(float)*4))){
        printf("\nfailed for alpha\n");
        free(x);
        free(y);
        exit(1);
    }
    int i;

    //fill up arrays
    for(i=0; i < size_x; i++){
        x[i]= (float)(i*1000);
    }
    for(i=0; i < size_y; i++){
        y[i]= (float)(i*1000);
    }
    for(i=0; i <4; i++){
        alpha[i]=(float)1;
    }


    print_arrays(x, size_x, y, size_y, alpha,1);    

    //warmup_scalar(x, y, size_y, alpha);
    warmup_vector(x, y, size_y, alpha);

    print_arrays(x, size_x, y, size_y, alpha,1);

    free(x);
    free(y);
    free(alpha);

}

void warmup_scalar(float* x, float* y, int size, float* alpha){
    int i;
    for(i=0; i< size; ++i){
        y[i]=x[2*i] *x[2*i] + x[2* i+1]/alpha[0];
    }

}


void warmup_vector(float* x, float* y, int size, float* alpha){
    int i;
    __m128 RX_2, RX_2i, RY, R_t1, R_t2, R_alpha;
    int x_2;
    int x_2i;
    R_alpha = _mm_load_ps(&alpha[0]);
    for(i=0; i<size; i+=4){
        //load our various x values
        RX_2 = _mm_load_ps(&x[i*2]);   
        RX_2i = _mm_load_ps(&x[2*i+1]);
        //perform x[2*i+1]/alpha -> store in R_t1
        R_t1 = _mm_div_ps(RX_2i, R_alpha);
        //multiply x[2*i] by x[2*i] -> store in R_t2
        R_t2= _mm_mul_ps(RX_2, RX_2);
        //add all our variables together; this will be stored as our new y[i] values (in RY)
        //issue resulting in seg fault seems to reside in below code.... not sure what's going on
        //tried increasing the size of things in our array to see if we were getting some negative values
        //something able to set to handle negatives?
        //  RY = _mm_add_ps(R_t1, R_t2);
        RY = _mm_add_ps(R_t1, R_t2);
        // copy everything to y[i]
        _mm_store_ps(&y[i], RY);     
    }

}


