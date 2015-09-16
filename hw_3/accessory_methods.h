#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
void warmup_scalar(float* x, float* y, int size, float* alpha);
void warmup_vector(float* x, float* y, int size, float* alpha);



void warmup_scalar_part2(float* x, float* y, int size, float* h);
void warmup_vector_part2(float* x, float* y, int size, float* h);

void print_arrays(float* x, int size_x, float* y, int size_y, float* alpha, int part_1){

    int i;
    printf("\nArray y\n [");
    for(i=0; i<size_y;i++){
        printf(" %f ", y[i]);
    }
    printf("]\n\n");

    printf("\nArray x\n [");
    for(i=0; i<size_x;i++){
        printf(" %f ", x[i]);
    }
    printf("]\n\n");

    printf("\nArray Alpha\n [");
    if(part_1==1){
        for(i=0; i<4;i++){
            printf(" %f ", alpha[i]);
        }
        printf("]\n\n--------------------------------------------------------------------------\n\n");
    }

    else {
        for(i=0; i<16;i++){
            printf(" %f ", alpha[i]);
        }
        printf("]\n\n--------------------------------------------------------------------------\n\n");

    }
}

float return_rand_float(){
    time_t t;
    return (float)(rand() % 100);
}


