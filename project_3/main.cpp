#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <deque>
#include <thread>
#include <mutex>
#include <queue>
#include "CSRGraph.h"
#include "Util.h"

#define MAX_THREADS 32 
#define THREAD_INCREMENT 4
#define ENABLE_TIME
#define PRINT_DISTANCES

// ------------Global Variables Needed---------------

std::mutex queue_lock;
long start_nsec, end_nsec;
double secs;
const unsigned long long int ONE_BILLION = 1000000000;



// ----Priority Queue Structure/Compare Function-----

struct Priority_struct {
    int node_name;
    int node_weight;
};

class Compare_priority{
    public:
        //return true if item1 has a lower or equal weight to item 2
        //our priority queue operates by placing higher priority on edges
        //with lower weights
        bool operator()(Priority_struct &item1, Priority_struct &item2){
            if(item1.node_weight < item2.node_weight) {return true;}
            if(item1.node_weight == item2.node_weight){return true;}
            return false;
        }
};

struct Thread_struct{
    int thread_id;
    std::priority_queue<Priority_struct, std::vector<Priority_struct>, Compare_priority> thread_queue;
    //std::mutex thread_lock;
};

std::vector<Thread_struct> queue_vector;
// ----------------Timing/Various Utility Functions-------------------

long clock_nanoseconds()
{
#ifdef ENABLE_TIME
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ONE_BILLION * ts.tv_sec + ts.tv_nsec);
#else
    return 0;
#endif
}

void start_timing()
{
    start_nsec = clock_nanoseconds();
}

void stop_timing()
{
    end_nsec = clock_nanoseconds();
    secs = 1.0f * (end_nsec - start_nsec) / ONE_BILLION;
}

void flush_cache()
{
    int size = 20*1024*1024;
    char *buf = (char *) malloc(size);

    int i;
    for(i=0; i < size; i++)
        buf[i] = random_between(0, 0xFF);

    free(buf);
}


// ------------------FIFO Scheduling Implementation------------------------

void dijkstra_FIFO_nothreads(CSRGraph *g)
{
    int *dist = new int[g->size];
    int i;
    for(i=0;i<g->size;i++)
    {
        dist[i] = MAX_INT;
    }

    int source = 0;
    dist[source] = 0;

    std::deque<int> deq;
    deq.push_back(source);

    printf("\n");
    for(int i=0; i< deq.size();i++){
	printf(" %d", deq[i]);
    }
    printf("\n");

    flush_cache();
    start_timing();

    while(! deq.empty() )
    {
        int u = deq.front();
        deq.pop_front();
	
    printf("Popped\n");
    for(int i=0; i< deq.size();i++){
	printf(" %d", deq[i]);
    }
    printf("\n");

        std::vector<int> neighbors = CSRGraph_getNeighbors(g, u);
        int neighbor;
        for(neighbor=0;neighbor < neighbors.size(); neighbor++)
        {
            int v = neighbors[neighbor];
            int alt = dist[u] + CSRGraph_getDistance(g, u, v);

            if(alt < dist[v])
            {
                dist[v] = alt;
                deq.push_back(v);
		
    printf("Pushed\n");
    for(int i=0; i< deq.size();i++){
	printf(" %d", deq[i]);
    }
    printf("\n");
            }      
        }
    }

    stop_timing();

    //Print calculated distances to each graph
    i=0;
    while(i < g->size && i < 10)
    {
        printf("%d ", dist[i]);
        i++;
    }
    printf("\n");

    delete(dist);
    dist = NULL;
}

void dijkstra_FIFO_thread(CSRGraph *g, int *dist, std::deque<int> &deq, int threadNum, std::mutex *dist_locks)
{
    while(!deq.empty())
    {
        queue_lock.lock();

        if(deq.empty()){
            queue_lock.unlock();
            break;
        }

        int u = deq.front();
        deq.pop_front();
	
    printf("Popped\n");
    for(int i=0; i< deq.size();i++){
	printf(" %d", deq[i]);
    }
    printf("\n");
        int udist = dist[u];
        queue_lock.unlock();

        std::vector<int> neighbors = CSRGraph_getNeighbors(g, u);
        int neighbor;
        for(neighbor=0;neighbor < neighbors.size(); neighbor++)
        {
            int v = neighbors[neighbor];
            int uvdist = CSRGraph_getDistance(g, u, v);
            int alt = udist + uvdist;

            dist_locks[v].lock();
            if(alt < dist[v])
            {
                dist[v] = alt;
                dist_locks[v].unlock();

                queue_lock.lock();                
                deq.push_back(v);
		
    printf("Pushed\n");
    for(int i=0; i< deq.size();i++){
	printf(" %d", deq[i]);
    }
    printf("\n");
                queue_lock.unlock();                    
            }
            else
            {
                dist_locks[v].unlock();
            }


        }
    }
    //printf("Thread %d ended.\n", threadNum);
}


void dijkstra_FIFO_threaded(CSRGraph *g, int numThreads)
{
    int *dist = new int[g->size];
    std::mutex *dist_locks = new std::mutex[g->size];
    int i;

    for(i=0;i<g->size;i++)
    {
        dist[i] = MAX_INT;
    }

    int source = 0;
    dist[source] = 0;

    std::deque<int> deq;
    deq.push_back(source);

    
    printf("Pushed initial\n");
    for(int i=0; i< deq.size();i++){
	printf(" %d", deq[i]);
    }
    printf("\n");
    flush_cache();
    start_timing();

    // make threads

    std::vector<std::thread> threads;
    for(i = 0; i < numThreads; ++i){
        threads.push_back(std::thread(dijkstra_FIFO_thread, g, dist, std::ref(deq), i, dist_locks));
    }

    for (auto it = threads.begin(); it != threads.end(); ++it) {
        std::thread &t = *it;
        t.join();
    }

    stop_timing();

#ifdef PRINT_DISTANCES
    i=0;
    while(i < g->size && i < 10)
    {
        printf("%d ", dist[i]);
        i++;
    }
    printf("\n");
#endif

    delete(dist);
    delete(dist_locks);
}

// ------------------LIFO Scheduling Implementation------------------------

void dijkstra_LIFO_nothreads(CSRGraph *g)
{
    int *dist = new int[g->size];
    int *onstack = new int[g->size];
    int i;

    for(i=0;i<g->size;i++)
    {
        dist[i] = MAX_INT;
        onstack[i]= 0;
    }

    int source = 0;
    dist[source] = 0;

    std::deque<int> deq;
    deq.push_back(source);
    onstack[source] = 1;
    
    printf("Pushed initial\n");
    for(int i=0; i< deq.size();i++){
	printf(" %d", deq[i]);
    }
    printf("\n");

    flush_cache();
    start_timing();

    // *** perform DFS ***
    while(! deq.empty() )
    {
        int u = deq.back();
        deq.pop_back();
	
    printf("Popped\n");
    for(int i=0; i< deq.size();i++){
	printf(" %d", deq[i]);
    }
    printf("\n");
        onstack[u] = 0;

        std::vector<int> neighbors = CSRGraph_getNeighbors(g, u);
        int neighbor;
        for(neighbor=0;neighbor < neighbors.size(); neighbor++)
        {
            int v = neighbors[neighbor];
            int alt = dist[u] + CSRGraph_getDistance(g, u, v);
            if(alt < dist[v])
            {
                dist[v] = alt;

                if(onstack[v] == 0)
                {
                    deq.push_back(v);
		    
    printf("Pushed\n");
    for(int i=0; i< deq.size();i++){
	printf(" %d", deq[i]);
    }
    printf("\n");
                    onstack[v] = 1;
                }
            }
        }
    }

    stop_timing();

#ifdef PRINT_DISTANCES
    i=0;
    while(i < g->size && i < 10)
    {
        printf("%d ", dist[i]);
        i++;
    }
    printf("\n");
#endif

    delete(onstack);
    delete(dist);
    dist = NULL;
}

void dijkstra_LIFO_thread(CSRGraph *g, int *dist, std::deque<int> &deq, int threadNum, int *onstack, std::mutex *dist_locks)
{
    while(!deq.empty())
    {
        queue_lock.lock();

        if(deq.empty()){
            queue_lock.unlock();
            break;
        }

        int u = deq.back();
        deq.pop_back();
	
    printf("Popped\n");
    for(int i=0; i< deq.size();i++){
	printf(" %d", deq[i]);
    }
    printf("\n");
        onstack[u]=0;

        int udist = dist[u];
        queue_lock.unlock();

        std::vector<int> neighbors = CSRGraph_getNeighbors(g, u);
        int neighbor;
        for(neighbor=0;neighbor < neighbors.size(); neighbor++)
        {
            int v = neighbors[neighbor];
            int uvdist = CSRGraph_getDistance(g, u, v);
            int alt = udist + uvdist;

            dist_locks[v].lock();
            if(alt < dist[v])
            {
                dist[v] = alt;
                dist_locks[v].unlock();

                queue_lock.lock();                
                if(onstack[v] == 0) 
                {
                    deq.push_back(v);
		    
    printf("Pushed\n");
    for(int i=0; i< deq.size();i++){
	printf(" %d", deq[i]);
    }
    printf("\n");
                    onstack[v]=1;
                }
                queue_lock.unlock();                    
            }
            else
            {
                dist_locks[v].unlock();
            }
        }
    }
    //printf("Thread %d ended.\n", threadNum);
}


void dijkstra_LIFO_threaded(CSRGraph *g, int numThreads)
{
    int *dist = new int[g->size];
    std::mutex *dist_locks = new std::mutex[g->size];
    int *onstack = new int[g->size];
    int i;

    for(i=0;i<g->size;i++)
    {
        dist[i] = MAX_INT;
        onstack[i]=0;
    }

    int source = 0;
    dist[source] = 0;

    std::deque<int> deq;
    deq.push_back(source);
    onstack[source]=1;

    printf("Pushed initial\n");
    for(int i=0; i< deq.size();i++){
	printf(" %d", deq[i]);
    }
    printf("\n");

    flush_cache();
    start_timing();

    // make threads
    std::vector<std::thread> threads;
    for(i = 0; i < numThreads; ++i){
        threads.push_back(std::thread(dijkstra_LIFO_thread, g, dist, std::ref(deq), i, onstack, dist_locks));
    }

    for (auto it = threads.begin(); it != threads.end(); ++it) {
        std::thread &t = *it;
        t.join();
    }

    stop_timing();

#ifdef PRINT_DISTANCES
    i=0;
    while(i < g->size && i < 10)
    {
        printf("%d ", dist[i]);
        i++;
    }
    printf("\n");
#endif
    delete(onstack);
    delete(dist);
    delete(dist_locks);
}

// ------------------Priority Queue Scheduling Implementation------------------------

void dijkstra_pq_nothreads(CSRGraph *g)
{
    int *dist = new int[g->size];
    int *visited = new int[g->size];
    int *changed = new int[g->size];

    int i;
    for(i=0;i<g->size;i++)
    {
        dist[i] = MAX_INT;
        visited[i] = 0;
        changed[i]= 0;
    }

    int source = 0;
    dist[source] = 0;
    visited[source] = 1;
    changed[source] = 1;

    std::priority_queue<Priority_struct, std::vector<Priority_struct>, Compare_priority> pq;
    Priority_struct temp;
    temp.node_name=source;
    temp.node_weight=0;
    pq.push(temp);
    
    printf("Pushed\n");
    for(int i=0; i< pq.size();i++){
	printf(" %d", pq[i].node_name);
    }

    printf("\n");

    flush_cache();
    start_timing();

    // *** perform DFS ***
    while(!pq.empty() )
    {
        Priority_struct temp_struct = pq.top();
        int u = temp_struct.node_name;
        pq.pop();
	
    printf("Popped\n");
    for(int i=0; i< pq.size();i++){
	printf(" %d", pq[i]);
    }
    printf("\n");

        std::vector<int> neighbors = CSRGraph_getNeighbors(g, u);
        int neighbor;
        for(neighbor=0;neighbor < neighbors.size(); neighbor++)
        {
            int v = neighbors[neighbor];
            int alt = dist[u] + CSRGraph_getDistance(g, u, v);

            if(alt < dist[v])
            {
                Priority_struct temp2;
                temp2.node_name=v;
                temp2.node_weight= CSRGraph_getDistance(g, u, v); 
                dist[v] = alt;
                changed[v]++;
                visited[v]++;    
                pq.push(temp2);
		
    printf("Pushed\n");
    for(int i=0; i< pq.size();i++){
	printf(" %d", pq[i]);
    }
    printf("\n");
            }

        }
    }

    stop_timing();

#ifdef PRINT_DISTANCES
    i=0;
    while(i < g->size && i < 10)
    {

        printf("%d ", dist[i]);
        i++;
    }
    printf("\n");
#endif

    delete(dist);
    delete(visited);
    delete(changed);
}

void dijkstra_pq_thread(CSRGraph *g, int *dist, std::priority_queue<Priority_struct, std::vector<Priority_struct>, Compare_priority> &pq, int threadNum, std::mutex *dist_locks)
{
    while(!pq.empty())
    {
        queue_lock.lock();

        if(pq.empty())
        {
            queue_lock.unlock();
            break;
        }

        Priority_struct temp_struct = pq.top();
        int u = temp_struct.node_name;
        pq.pop();
	
    printf("Popped\n");
    for(int i=0; i< pq.size();i++){
	printf(" %d", pq[i]);
    }
    printf("\n");

        int udist = dist[u];

        queue_lock.unlock();

        std::vector<int> neighbors = CSRGraph_getNeighbors(g, u);

        int neighbor;
        for(neighbor=0;neighbor < neighbors.size(); neighbor++)
        {
            int v = neighbors[neighbor];
            int uvdist = CSRGraph_getDistance(g, u, v);
            int alt = udist + uvdist;

            dist_locks[v].lock();
            if(alt < dist[v])
            {
                dist[v] = alt;
                dist_locks[v].unlock();

                Priority_struct temp2;
                temp2.node_name=v;
                temp2.node_weight = uvdist;

                queue_lock.lock();                
                pq.push(temp2);
		
    printf("Pushed\n");
    for(int i=0; i< pq.size();i++){
	printf(" %d", pq[i]);
    }
    printf("\n");
                queue_lock.unlock();                    
            }
            else
            {
                dist_locks[v].unlock();
            }
        }
    }

    //printf("Thread %d ended.\n", threadNum);
}


void dijkstra_pq_threaded(CSRGraph *g, int numThreads)
{
    int *dist = new int[g->size];
    std::mutex *dist_locks = new std::mutex[g->size];
    int i;

    for(i=0;i<g->size;i++)
    {
        dist[i] = MAX_INT;
    }

    int source = 0;
    dist[source] = 0;

    std::priority_queue<Priority_struct, std::vector<Priority_struct>, Compare_priority> pq;
    Priority_struct temp;
    temp.node_name=source;
    temp.node_weight=0;
    pq.push(temp);

    printf("Pushed Initial\n");
    for(int i=0; i< pq.size();i++){
	printf(" %d", pq[i]);
    }
    printf("\n");

    flush_cache();
    start_timing();

    // make threads
    std::vector<std::thread> threads;
    for(i = 0; i < numThreads; ++i){
        threads.push_back(std::thread(dijkstra_pq_thread, g, dist, std::ref(pq), i, dist_locks));
    }

    for (auto it = threads.begin(); it != threads.end(); ++it) {
        std::thread &t = *it;
        t.join();
    }

    stop_timing();

#ifdef PRINT_DISTANCES
    i=0;
    while(i < g->size && i < 10)
    {
        printf("%d ", dist[i]);
        i++;
    }
    printf("\n");
#endif

    delete(dist);
    delete(dist_locks);
    dist = NULL;

}

// ----------------Work Stealing Functions------------------------------



void dijkstra_workstealing_thread(CSRGraph *g, int *dist, int thread_num, std::mutex *dist_locks, std::mutex *queue_locks)
{
    //get queue for thread in question

work_exists:
    //while the thread has work
    while(!queue_vector[thread_num-1].thread_queue.empty())
    {
        queue_locks[thread_num-1].lock();
        if(queue_vector[thread_num-1].thread_queue.empty())
        {
          queue_locks[thread_num - 1].unlock();
          break;
        }

        //get the first element off the queue
        Priority_struct temp_struct = queue_vector[thread_num-1].thread_queue.top();
        int u = temp_struct.node_name;
        queue_vector[thread_num-1].thread_queue.pop();
        queue_locks[thread_num - 1].unlock();
   
    
        std::vector<int> neighbors = CSRGraph_getNeighbors(g, u);
        int neighbor;
        for(neighbor=0;neighbor < neighbors.size(); neighbor++)
        {
            int v = neighbors[neighbor];

            dist_locks[v].lock();
            int alt = dist[u] + CSRGraph_getDistance(g, u, v);
            if(alt < dist[v])
            {
                Priority_struct temp2;
                temp2.node_name=v;
                temp2.node_weight = CSRGraph_getDistance(g, u, v);
                dist[v] = alt;

                queue_locks[thread_num-1].lock();
                queue_vector[thread_num-1].thread_queue.push(temp2);
                queue_locks[thread_num-1].unlock();
            }
            dist_locks[v].unlock();
        }
    }

    if(queue_vector[thread_num-1].thread_queue.empty()){
        //search for work
        int i;
        //by declaring size outside of the loop over queue vector, hoping to avoid
        //issues where the queue vector continues to grow
        int size = queue_vector.size();
        //iterate through all items in the threads vector
        iterate_through_queues:
        for (i=0; i<size; i++){
            //see if thread has work, is not our current thread, is not locked
            if(queue_vector[i].thread_queue.size()>=1 && i!=thread_num-1 && queue_locks[i].try_lock()==true){
               if(queue_vector[i].thread_queue.empty()){
                    queue_locks[i].unlock();
                    goto iterate_through_queues;
                }

                //lock our current thread's queue
                queue_locks[thread_num-1].lock();
                Priority_struct  work = queue_vector[i].thread_queue.top();
                queue_vector[thread_num-1].thread_queue.push(work);
                queue_vector[i].thread_queue.pop();
                queue_locks[thread_num-1].unlock();
                queue_locks[i].unlock();
               goto work_exists;
            }
        }
        return;
    }

}


void dijkstra_workstealing(CSRGraph *g, int numThreads)
{
    int *dist = new int[g->size];
    std::mutex *dist_locks = new std::mutex[g->size];
    std::mutex *queue_locks = new std::mutex[g->size];
    int i;

    for(i=0;i<g->size;i++)
    {
        dist[i] = MAX_INT;
    }

    int source = 0;
    dist[source] = 0;


    Thread_struct ithread_struct;
    //fill up the thread struct
    Priority_struct original_node;
    original_node.node_name=source;
    original_node.node_weight=0;
    ithread_struct.thread_queue.push(original_node);
    ithread_struct.thread_id=1;
    queue_vector.push_back(ithread_struct);


    flush_cache();
    start_timing();

    // make threads
    std::vector<std::thread> threads;
    threads.push_back(std::thread(dijkstra_workstealing_thread, g, dist,1, dist_locks, queue_locks));
    //    }

    for(int i=2; i <numThreads; ++i){
       Thread_struct tthread_struct;
       tthread_struct.thread_id=i;
       queue_vector.push_back(tthread_struct);
       threads.push_back(std::thread(dijkstra_workstealing_thread, g , dist, i, dist_locks, queue_locks));
}
    for (auto it = threads.begin(); it != threads.end(); ++it) {
        std::thread &t = *it;
        t.join();
    }

stop_timing();

#ifdef PRINT_DISTANCES
i=0;
while(i < g->size && i < 10)
{
    printf("%d ", dist[i]);
    i++;
}
printf("\n");
#endif

delete(dist);
//delete(dist_locks);
//delete(queue_locks);
dist = NULL;

}




// ----------------Caller Functions------------------------------

void dijkstra_FIFO(CSRGraph *g)
{
    printf("\n\n---------- FIFO ----------\n");
    dijkstra_FIFO_nothreads(g);
    double baseline = secs;
    printf("\tUnthreaded\t%0.4f\t%0.2f\n\n", baseline, 1.0f);

    int t=THREAD_INCREMENT;
    while(t <= MAX_THREADS)
    {
        dijkstra_FIFO_threaded(g, t);
        printf("\t%d threads\t%0.4f\t%0.2f\n", t, secs, baseline/(1.0f*t*secs) * 100.0f);
        t+=2;
        // t += THREAD_INCREMENT;
    }

}

void dijkstra_LIFO(CSRGraph *g)
{
    printf("\n\n---------- LIFO ----------\n");
    dijkstra_LIFO_nothreads(g);
    double baseline = secs;
    printf("\tUnthreaded\t%0.4f\t%0.2f\n\n", baseline, 1.0f);

    int t=THREAD_INCREMENT;
    while(t <= MAX_THREADS)
    {
        dijkstra_LIFO_threaded(g, t);
        printf("\t%d threads\t%0.4f\t%0.2f\n", t, secs, baseline/(1.0f*t*secs) * 100.0f);
        t+=2;
        //t += THREAD_INCREMENT;
    }

}

void dijkstra_pq(CSRGraph *g)
{
   double baseline = secs;
    printf("\n\n---------- PQueue ----------\n");
   // dijkstra_pq_nothreads(g);
   // printf("\tUnthreaded\t%0.4f\t%0.2f\n\n", baseline, 1.0f);


    int t=THREAD_INCREMENT;
    while(t <= MAX_THREADS)
    {
        dijkstra_pq_threaded(g, t);
        printf("\t%d threads\t%0.4f\t%0.2f\n", t, secs, baseline/(1.0f*t*secs) * 100.0f);
        t+=2;
           // t += THREAD_INCREMENT;
    }
    
}


void work_stealing(CSRGraph *g){
   printf("\n\n----------------Work Stealing------------------\n");
   double baseline = 0;
   //int t = THREAD_INCREMENT;
   int t = 10;
   int m = MAX_THREADS;
   dijkstra_workstealing(g, t);
   printf("\t%d threads\t%0.4f\t%0.2f\n", t, secs, baseline/(1.0f*t*secs) * 100.0f);

}






// ---------------Main Function-------------------
int main(int argc, const char * argv[])
{
    //  if(argc != 2 && argc != 3) return 0;
    //
    //  
    //  printf("Reading Graph %s...\n", argv[1]);
    //  CSRGraph *g = NULL;
    // 
    //  if(argc == 3)
    //  {
    //    int offsetNodes = atoi(argv[2]);
    //    printf("Offsetting node numbers by %d.\n", offsetNodes);
    //    g = CSRGraph_loadFile(argv[1], offsetNodes);
    //  }
    //  else if(argc == 2)
    //  {
    //    g = CSRGraph_loadFile(argv[1], 0);
    //  }

        srand(0);
      CSRGraph *g = CSRGraph_alloc(500);
    CSRGraph_growToDensity(g, 0.3);  

    //const char *filePath = "/Users/jkhust/Documents/School/CS 378 - Prog Perf/Proj3/Proj3/graphs/mid_graph.txt";
    //const char *filePath = "graphs/mid_graph.txt";
    //CSRGraph *g = CSRGraph_loadSortedFile(filePath, 0);

    //const char *filePath = "/Users/jkhust/Documents/School/CS 378 - Prog Perf/Proj3/Proj3/graphs/ny-graph.gr";

    //const char *filePath = "../graphs/ny-graph.gr";
    //CSRGraph *g = CSRGraph_loadSortedFile(filePath, -1);  

    //CSRGraph_print(g);
    dijkstra_FIFO(g);
    //dijkstra_LIFO(g);
   // dijkstra_pq(g);  
    work_stealing(g);
    CSRGraph_free(g);
    g = NULL;
}

