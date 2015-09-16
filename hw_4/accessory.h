#include <stdio.h>
#include <stdlib.h>
#define GRAPH_SIZE 12000 

std::vector<std::pair<int*, int*> > row_vector;
std::vector<int > column_vector;
std::vector<int> weight_vector;


//graph node struct

int call_bfs(int** adj_matrix, int source, int destination);
int bfs_adj_matrix(int** adj_matrix, int source, int destination);
int bfs_csr(int source, int destination);

int find_source(int** adj_matrix){
    int i ,j;
    //find valid source and destination nodes
    for(i=0; i< GRAPH_SIZE; i++){
        for(j=0; j< GRAPH_SIZE; j++){
            if(adj_matrix[i][j]!=0){
                return j;
            }
        }
    }
    return 1;
}

int find_dest(int** adj_matrix, int source){
    int i ,j;
    //find valid source and destination nodes
    for(i=0; i< GRAPH_SIZE; i++){
        for(j=0; j< GRAPH_SIZE; j++){
            if(adj_matrix[i][j]!=0 && j!=source){
                return j;
            }
        }
    }
    return 1;
}

void print_adj_matrix(int** adj_matrix){

    //print adj_matrix
    printf("\n\nAdj. Matrix:\n\n");
    int i, j;
    for(i=0; i<GRAPH_SIZE; i++){
        for(j=0;j<GRAPH_SIZE;j++){
            printf("%d ", adj_matrix[i][j]);
        }
        printf("\n");
    } 

    printf("\n\nCSR:\n\n");
    //print out csr
    std::vector<int>::size_type it=0;
    for(i=0; i < GRAPH_SIZE-1; i++){
        if(row_vector[i].first==&column_vector[it]) {
            while(row_vector[i+1].first!=&column_vector[it]){
                printf("%d:%d ", column_vector[it], weight_vector[it]);
                it++;
            }
        }
        printf("\n");
    }
    while(it!=column_vector.size()){
        printf("%d:%d ", column_vector[it], weight_vector[it]);
        it++;
    }
    printf("\n\n\n");

}

int generate_matrix_csr_sparse(int** adj_matrix){
    int i, j;
    //allocate our adjacency matrix
    for(i=0; i < GRAPH_SIZE; i++){
        adj_matrix[i]= (int*) calloc(GRAPH_SIZE, sizeof(int));
        if(adj_matrix[i]==NULL){
            for(j=0;j<i; j++){
                free(adj_matrix[j]);
            }
            free(adj_matrix);
            printf("failed to malloc part of adj_matrix : %d\n", i);
            return -1;
        }

    }

    //create array which will store all of our edge weights
    int edge_weights[GRAPH_SIZE];
    for(i=0;i<GRAPH_SIZE; i++){
        int rand_weight = rand() %75+1;
        edge_weights[i]=rand_weight;
    }


    //begin creating our edges-> fill up adj matrix first, then the csr
    int num_edges, source_weight, destination, destination_weight;
   
    for(i=0; i< GRAPH_SIZE; i++){
        num_edges = GRAPH_SIZE/2;
        source_weight = edge_weights[i];
        for(j=0; j< num_edges; j++){
            destination = rand()%GRAPH_SIZE+0;
            while(destination==i){
                destination=rand()%GRAPH_SIZE+0;
            }
            destination_weight=edge_weights[destination];
            //adj matrix will store at any point the value of the edge which does not correspond to the particular row. for  ex) edge between nodes 1 and 5. in
            //our adj matrix, we would store the weight of node 5 at [0][4], and the weight of node 1 at [4][0]
            //do not overwrite previously created edges

            if(adj_matrix[i][destination]==0 && adj_matrix[destination][i]==0){
//                printf("edge between %d and %d; source has weight %d and destination has weight %d\n", i, destination, source_weight, destination_weight);
                adj_matrix[i][destination]=destination_weight;
                adj_matrix[destination][i]=source_weight;
            }
        }
    }

    int edge_counter=0;
    int counter_before=0;
    //now that have adj_matrix full, fill csr

    //create array to hold number of edges for each node to help in 
    //constructing the csr
    int num_edge_array[GRAPH_SIZE];
    edge_counter=0;

    //loop through adj_matrix to find edges and construct colum and weight vectors
    for(i=0; i<GRAPH_SIZE; i++){
        for(j=0;j<GRAPH_SIZE; j++){
            if(adj_matrix[i][j]!=0){
                //if adj_matrix has an edge, push the node it is at
                //as well as the weight to our vectors
                column_vector.push_back(j);
                weight_vector.push_back(adj_matrix[i][j]);
                //printf("adding edge\n");
                edge_counter+=1;
            }
        }
        num_edge_array[i]=edge_counter;
        //printf("clearing_edge counter\n");
        edge_counter=0;
    }

    //special case, add in first node to our row vector
    std::pair<int*, int*> temp_pair;
    temp_pair = std::make_pair(&column_vector[0], &weight_vector[0]);
    row_vector.push_back(temp_pair);

    //add values to our row vector
    int temp_num=0;
    for(i=1; i< GRAPH_SIZE;i++){
        temp_num+=num_edge_array[i-1];
        //printf("temp_num: %d\n", temp_num);

        //if there are no edges attached to a node, set pointer to be the same
        //as the pointer in the first item in our row vector to indicate this
        if(num_edge_array[i-1]==0){
            temp_pair = std::make_pair(row_vector[0].first, row_vector[0].second);
            row_vector.push_back(temp_pair);      
        }
        else{
            temp_pair = std::make_pair(&column_vector[temp_num], &weight_vector[temp_num]);
            row_vector.push_back(temp_pair); 
        }
    }
}

int generate_matrix_csr_ring(int** adj_matrix){
    int i, j;
    //allocate our adjacency matrix
    for(i=0; i < GRAPH_SIZE; i++){
        adj_matrix[i]= (int*) calloc(GRAPH_SIZE, sizeof(int));
        if(adj_matrix[i]==NULL){
            for(j=0;j<i; j++){
                free(adj_matrix[j]);
            }
            free(adj_matrix);
            printf("failed to malloc part of adj_matrix : %d\n", i);
            return -1;
        }

    }

    //create array which will store all of our edge weights
    int edge_weights[GRAPH_SIZE];
    for(i=0;i<GRAPH_SIZE; i++){
        int rand_weight = rand() %75+1;
        edge_weights[i]=rand_weight;
    }


    //begin creating our edges-> fill up adj matrix first, then the csr
    int num_edges, source_weight, destination, destination_weight;
    for(i=0; i<GRAPH_SIZE-1; i++){
        source_weight = edge_weights[i];
        destination_weight=edge_weights[i+1];
        if(adj_matrix[i][i+1]==0 && adj_matrix[i+1][i]==0){
            adj_matrix[i][i+1]=destination_weight;
            adj_matrix[i+1][i]=source_weight;
        }     
    
    }
    adj_matrix[GRAPH_SIZE-1][0]=edge_weights[0];
    adj_matrix[0][GRAPH_SIZE-1]=edge_weights[GRAPH_SIZE-1];

    int edge_counter=0;
    int counter_before=0;
    //now that have adj_matrix full, fill csr

    //create array to hold number of edges for each node to help in 
    //constructing the csr
    int num_edge_array[GRAPH_SIZE];
    edge_counter=0;

    //loop through adj_matrix to find edges and construct colum and weight vectors
    for(i=0; i<GRAPH_SIZE; i++){
        for(j=0;j<GRAPH_SIZE; j++){
            if(adj_matrix[i][j]!=0){
                //if adj_matrix has an edge, push the node it is at
                //as well as the weight to our vectors
                column_vector.push_back(j);
                weight_vector.push_back(adj_matrix[i][j]);
                //printf("adding edge\n");
                edge_counter+=1;
            }
        }
        num_edge_array[i]=edge_counter;
        //printf("clearing_edge counter\n");
        edge_counter=0;
    }

    //special case, add in first node to our row vector
    std::pair<int*, int*> temp_pair;
    temp_pair = std::make_pair(&column_vector[0], &weight_vector[0]);
    row_vector.push_back(temp_pair);

    //add values to our row vector
    int temp_num=0;
    for(i=1; i< GRAPH_SIZE;i++){
        temp_num+=num_edge_array[i-1];
        //printf("temp_num: %d\n", temp_num);

        //if there are no edges attached to a node, set pointer to be the same
        //as the pointer in the first item in our row vector to indicate this
        if(num_edge_array[i-1]==0){
            temp_pair = std::make_pair(row_vector[0].first, row_vector[0].second);
            row_vector.push_back(temp_pair);      
        }
        else{
            temp_pair = std::make_pair(&column_vector[temp_num], &weight_vector[temp_num]);
            row_vector.push_back(temp_pair); 
        }
    }
}
