#include <iostream>
#include <queue>
#include "accessory.h"
#include <time.h>
#include <list>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <papi.h>
#include <unistd.h>

int main(){
    int i, j;
    srand(time(NULL));
    if(PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
        exit(0);
    }


    //generate our adjacency matrix; this is where we're store all of our edges
    int** adj_matrix = (int**) malloc(GRAPH_SIZE*sizeof(int*));
    if(adj_matrix==NULL){
        printf("failed to properly malloc adj_matrix\n");
        exit(1);
    }    

    int valid = generate_matrix_csr_ring(adj_matrix);
    //int valid = generate_matrix_csr_sparse(adj_matrix);

    if(valid==-1){
        printf("failed to generate adj_matrix. exiting\n");
    }
    //print_adj_matrix(adj_matrix);


    int source = find_source(adj_matrix);
    int destination=find_dest(adj_matrix, source);
    printf("source: %d, destination: %d\n", source, destination);


    call_bfs(adj_matrix, source, destination);

    //clean up memory
    for(i=0; i<GRAPH_SIZE; i++){
        free(adj_matrix[i]);
    }
    free(adj_matrix);
}


int call_bfs(int** adj_matrix, int source, int destination){
    struct timeval start,end;
    int NUM_EVENTS =5;
    int Events[5]= {PAPI_TOT_INS, PAPI_TOT_CYC, PAPI_FP_INS, PAPI_L1_TCM, PAPI_L2_TCM};
    long_long values[NUM_EVENTS];

    //bfs using adjacency matrix
    if(PAPI_start_counters(Events, NUM_EVENTS) != PAPI_OK){
        printf("\nERROR1\n");
        exit(0);
    }
    gettimeofday(&start, NULL);
    int foo=bfs_adj_matrix(adj_matrix, source, destination);
    gettimeofday(&end, NULL);
    if(PAPI_stop_counters(values, NUM_EVENTS) != PAPI_OK) {
        printf("\nERROR2\n");
        exit(0);
    }
    printf("seconds:%ld, milliseconds:%ld\n", (end.tv_sec - start.tv_sec), (end.tv_usec- start.tv_usec));
    printf("total instructions: %llu, total cycles: %llu, total floating point instructiosn: %llu, total L1 misses:%llu, total L2 misses: %llu\n", values[0], values[1], values[2], values[3], values[4]);


    //now do bfs using csr
    if(PAPI_start_counters(Events, NUM_EVENTS) != PAPI_OK){
        printf("\nERROR1\n");
        exit(0);
    }
    gettimeofday(&start, NULL);
    foo=bfs_csr(source, destination);
    gettimeofday(&end, NULL);
    if(PAPI_stop_counters(values, NUM_EVENTS) != PAPI_OK) {
        printf("\nERROR2\n");
        exit(0);
    }
    printf("seconds:%ld, milliseconds:%ld\n", (end.tv_sec - start.tv_sec), (end.tv_usec- start.tv_usec));
    printf("total instructions: %llu, total cycles: %llu, total floating point instructiosn: %llu, total L1 misses:%llu, total L2 misses: %llu\n", values[0], values[1], values[2], values[3], values[4]);

    return  1;

}

int bfs_adj_matrix(int** adj_matrix, int source, int destination){
    printf("\n\nBFS using Adj_Matrix\n\n");
    int i;
    //create set
    int set[GRAPH_SIZE]={0};
    //add our source node to our set
    set[source]=1;
    //create queue, add source onto queue
    std::list<int> queue;
    queue.push_back(source);
    while(!queue.empty()){
        int popped = queue.front();
        queue.pop_front();
        //printf("processing node : %d\n", popped);
        if(popped==destination){
            printf("found our destination: %d\n", popped);
            return popped;
        }
        else{
            for(i=0;i<GRAPH_SIZE;i++){
                //see if edge from adj matrix and if have already encountered node
                if(adj_matrix[popped][i]!=0 && set[i]!=1){
                    set[i]=1;
    //                printf("enqueuing node: %d\n", i);
                    queue.push_back(i);
                }
            }
        }
    }
}

int bfs_csr(int source, int destination){
    printf("\n\nBFS using CSR\n\n");
    int i;
    //create set, add source to our set
    int set[GRAPH_SIZE]={0};
    set[source]=1;
    //create queue, add source to queue
    std::list<int> queue;
    queue.push_back(source);
    //set up iterator
    std::vector<int>::size_type it=0;
    while(!queue.empty()){
        int popped= queue.front();
        //printf("processing node : %d\n", popped);
        it = 0;
        queue.pop_front();
        if(popped==destination){
            printf("found our destination: %d\n", popped);
            return popped;
        }
        //if we didn't return our destination, cycle through all edges connected to node
        //use row_vector[popped]->row_vector[popped+1];
        else{
            //search through our vector to find index of the item we're referencing
      //      it=0;
            it =row_vector[popped].first - &column_vector[0];
//d            while(&column_vector[it]!=row_vector[popped].ddfirst){
  //              it++;
    //        } 

            while(row_vector[popped+1].first!=&column_vector[it] && it<column_vector.size()){
                if(set[column_vector[it]]!=1){
                    set[column_vector[it]]=1;
        //            printf("enqueuing node: %d\n", column_vector[it]);
                    queue.push_back(column_vector[it]);
                }
                it++;
            }

        }
    }
}


