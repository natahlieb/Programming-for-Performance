//
//  CSRGraph.c
//  hw4
//
//  Created by Justin Hust on 10/18/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <sstream>

#include "CSRGraph.h"
#include "Util.h"
#include <deque>

#include <cassert>

#define EDGE_DISTANCE_MIN 1
#define EDGE_DISTANCE_MAX 100

// --------------------------------------------------------
CSRGraph * CSRGraph_alloc(int size)
{
  // *** must use new, since vectors need it ***
  CSRGraph *g = NULL;
  g = new CSRGraph();
  if(g == NULL) return NULL;
  
  g->size = size;
  
  g->endPtrs = new int[size];
  int i;
  for(i=0;i<size;i++)
    g->endPtrs[i] = 0;
    
  g->edges.clear();
  g->distances.clear();
  return g;
}

// --------------------------------------------------------
void CSRGraph_free(CSRGraph *g)
{
  delete(g->endPtrs);
  g->endPtrs = NULL;

  delete(g);
  g = NULL;
}

// ---------------------------------------------------------
void CSRGraph_growToDensity(CSRGraph *g, double targetDensity)
{  
  int distancesNeeded = (g->size * g->size) * targetDensity;
  
  int distancesHeld = CSRGraph_getNumDistances(g);
  
  printf("Growing Graph to %d edges...\n", distancesNeeded);
  
  // *** expand out but remain connected ***
  while(distancesHeld < distancesNeeded)
  {
    int src = random_between(0, g->size - 1);
    int dest = random_between(0, g->size - 1);
    int distance = random_between(EDGE_DISTANCE_MIN, EDGE_DISTANCE_MAX);
        
    // if edge doesn't exist,
    // and either: NO GRAPH YET, or WOULD CONNECT, add it.
    if( src != dest && 
        (CSRGraph_getDistance(g, src, dest) == 0) &&
        ((distancesHeld == 0) ||
         ((CSRGraph_isConnected(g, src) || CSRGraph_isConnected(g, dest)))))
    {
      CSRGraph_setDistance(g, src, dest, distance);
      CSRGraph_setDistance(g, dest, src, distance);
      distancesHeld += 2;
      
      if(distancesHeld % 20000 == 0)
        printf("Now %d edges.\n", distancesHeld);
    }
  }
}

// --------------------------------------------------------
// used for BFS traversal.
std::vector<int> CSRGraph_getNeighbors(CSRGraph *g, int src)
{
  assert(src >= 0 && src < g->size);

  int startPtr = (src > 0) ? g->endPtrs[src - 1] : 0;
  int endPtr = g->endPtrs[src];

  std::vector<int> neighbors;
  
  int edgeIndex;
  for(edgeIndex=startPtr;edgeIndex < endPtr;edgeIndex++)
  {
    int edge = g->edges[edgeIndex];
    neighbors.push_back(edge);
  }

  return neighbors;
}

// --------------------------------------------------------
int CSRGraph_getDistance(CSRGraph *g, int src, int dest)
{
  int startPtr = (src > 0) ? g->endPtrs[src - 1] : 0;
  int endPtr = g->endPtrs[src];

  int edgeIndex;
  for(edgeIndex=startPtr;edgeIndex < endPtr;edgeIndex++)
  {
    int edge = g->edges[edgeIndex];
    
    if(edge == dest)
    {
      return g->distances[edgeIndex];
    }
  }
  
  return 0;
}

// --------------------------------------------------------
int CSRGraph_getNumDistances(CSRGraph *g)
{
  return g->edges.size();
}

// --------------------------------------------------------
void CSRGraph_setDistance(CSRGraph *g, int src, int dest, int dist)
{
  int currEndPtr = g->endPtrs[src];

  // make spot for new data by shifting edge/distance data down
  g->edges.push_back(0);
  g->distances.push_back(0);
  
  // shift data
  int shift;
  for(shift= g->edges.size() - 2;shift >= currEndPtr; shift--)
  {
    g->edges[shift + 1] = g->edges[shift];
    g->distances[shift + 1] = g->distances[shift];
  }
  
  // make room for new data & add it
  g->edges[currEndPtr] = dest;
  g->distances[currEndPtr] = dist;
  
  // increase all end pointers to match the insert.
  int i;
  for(i=src;i< g->size;i++)
  {
    g->endPtrs[i] += 1;
  }  
}

// --------------------------------------------------------
int CSRGraph_isConnected(CSRGraph *g, int node)
{
  assert(node >= 0 && node < g->size);
  
  // not a source
  if(node == 0)
    if(g->endPtrs[node] == 0) return 0;
  else if(node > 0)
    if(g->endPtrs[node] == g->endPtrs[node - 1]) return 0;

  // not a destination
  int i;
  for(i=0;i<g->edges.size();i++)
    if(g->edges.at(i) == node)
      return 1;
  
  return 0;
}

// --------------------------------------------------------
void CSRGraph_print(CSRGraph *g)
{
  int i = 0;
  while(i < g->size ||
        i < g->edges.size() ||
        i < g->distances.size())
  {
    if(i < g->size)
      printf("%d %d\t", i, g->endPtrs[i]);
    else
      printf("\t");
  
    if(i < g->edges.size())
      printf("%d\t", g->edges.at(i));
    else
      printf("\t");
      
    if(i < g->distances.size())
      printf("%d\n", g->distances.at(i));
    else
      printf("\n");
  
    i++;
  }
}

// --------------------------------------------------------
CSRGraph * CSRGraph_loadSortedFile(const char *filePath, int offsetNodes)
{
  CSRGraph *g = NULL;

  std::ifstream fin(filePath);
  
  std::string line;
  std::string token;

  int numNodes;
  int edgesAdded = 0;
  
  int lastSrc; 
  int endPtr = 0;
  
  while(getline(fin, line))
  {
    if(line[0] == 'p')
    {
      std::istringstream ss(line);
      getline(ss, token, ' ');  // p
      getline(ss, token, ' ');  // sp
      getline(ss, token, ' ');  // num nodes
      numNodes = atoi(token.c_str());
      g = CSRGraph_alloc(numNodes);
      printf("Graph has %d nodes.\n", numNodes);
    }
    else if(line[0] == 'a')
    {
      std::istringstream ss(line);
      getline(ss, token, ' ');  // a
      getline(ss, token, ' ');  // src
      int src =  atoi(token.c_str()) + offsetNodes;
      getline(ss, token, ' ');  // dest
      int dest =  atoi(token.c_str()) + offsetNodes;
      getline(ss, token, ' ');  // dist
      int dist =  atoi(token.c_str());
      
      g->edges.push_back(dest);
      g->distances.push_back(dist);

      if(src != lastSrc)
      {
        g->endPtrs[src - 1] = endPtr;
        lastSrc = src;
      }

      endPtr++;

      edgesAdded++;
      if(edgesAdded % 20000 == 0)
        printf("Added %d edges.\n", edgesAdded);
    }
  }

  fin.close();
  
  return g;
}

// --------------------------------------------------------
CSRGraph * CSRGraph_loadFile(const char *filePath, int offsetNodes)
{
  CSRGraph *g = NULL;

  std::ifstream fin(filePath);
  
  std::string line;
  std::string token;
    
  int numNodes = 0;
  int edgesAdded = 0;
    
  while(getline(fin, line))
  {
    if(line[0] == 'p')
    {
      std::istringstream ss(line);
      getline(ss, token, ' ');  // p
      getline(ss, token, ' ');  // sp
      getline(ss, token, ' ');  // num nodes
      numNodes = atoi(token.c_str());
      g = CSRGraph_alloc(numNodes);
      printf("Graph has %d nodes.\n", numNodes);
    }
    else if(line[0] == 'a')
    {
      std::istringstream ss(line);
      getline(ss, token, ' ');  // a
      getline(ss, token, ' ');  // src
      int src =  atoi(token.c_str());
      getline(ss, token, ' ');  // dest
      int dest =  atoi(token.c_str());
      getline(ss, token, ' ');  // dist
      int dist =  atoi(token.c_str());
      
      CSRGraph_setDistance(g, src + offsetNodes, dest + offsetNodes, dist);
     
      edgesAdded++;
      if(edgesAdded % 20000 == 0)
        printf("Added %d edges.\n", edgesAdded);
    }
  }

  fin.close();
  
  return g;
}

// ---------------------------------------------------------
// stores node levels into array
void CSRGraph_BFS(CSRGraph *g, int *nodeLabels)
{
  std::deque<int> q;
  
  // *** look for a start node ***
  while( q.empty() )
  {
    int src = random_between(0, g->size - 1);
    int dest = random_between(0, g->size - 1);
  
    if(CSRGraph_getDistance(g, src, dest) > 0)
    {
      q.push_back(src);
      nodeLabels[src] = 0;
    }
  }
  
  // *** perform DFS ***
  while(! q.empty() )
  {
    int node = q.front();
    q.pop_front();
    //printf("\tExploring %d\n", node);
    
    std::vector<int> neighbors = CSRGraph_getNeighbors(g, node);
    int neighbor;
    for(neighbor=0;neighbor < neighbors.size(); neighbor++)
    {
      int dest = neighbors[neighbor];
      
      assert(dest >= 0 && dest < g->size);
      
      // found a neighbor we haven't explored yet.
      if( (nodeLabels[dest] > nodeLabels[node] + 1) )
      {
        q.push_back(dest);
        nodeLabels[dest] = nodeLabels[node] + 1;
      }
    }
  }
}