//
//  CSRGraph.h
//  hw4
//
//  Created by Justin Hust on 10/18/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#ifndef hw4_CSRGraph_h
#define hw4_CSRGraph_h

#include <vector>

typedef struct CSRGraph_t
{
  int size;
  int * endPtrs;
  std::vector<int> edges;
  std::vector<int> distances;
} CSRGraph;

// -----------------------------------------------------
CSRGraph * CSRGraph_loadSortedFile(const char *filePath, int offsetNodes);
CSRGraph * CSRGraph_loadFile(const char *filePath, int offsetNodes);
CSRGraph * CSRGraph_alloc(int size);

void CSRGraph_free(CSRGraph *g);


void CSRGraph_growToDensity(CSRGraph *g, double targetDensity);
void CSRGraph_setDistance(CSRGraph *g, int src, int dest, int dist);

int CSRGraph_getDistance(CSRGraph *g, int src, int dest);
int CSRGraph_getNumDistances(CSRGraph *g);
int CSRGraph_isConnected(CSRGraph *g, int node);
std::vector<int> CSRGraph_getNeighbors(CSRGraph *g, int src);

void CSRGraph_print(CSRGraph *g);
void CSRGraph_BFS(CSRGraph *g, int *nodeLabels);


#endif
