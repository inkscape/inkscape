/* Cycle detector that returns a list of 
 * edges involved in cycles in a digraph.
 *
 * Kieran Simpson 2006
*/
#include <iostream>
#include <stack>
#include <vector>
#include <cassert>
#include <cycle_detector.h>

#define VISIT_DEBUG
#define RUN_DEBUG

using namespace std;
using namespace cola;

// a global var representing time
TimeStamp Time;

CycleDetector::CycleDetector(unsigned numVertices, Edges *edges)  {
  this->V = numVertices; 
  this->edges = edges;
  nodes = NULL;

  // make the adjacency matrix
  this->make_matrix();
  assert(nodes->size() == this->V);
}

CycleDetector::~CycleDetector()  {
  if (nodes != NULL)  {
    for (unsigned i = 0; i < nodes->size(); i++)  { if ((*nodes)[i] != NULL)  { delete (*nodes)[i]; } }
    delete nodes;
  }
}

void CycleDetector::make_matrix()  {
  Edges::iterator ei;
  Edge anEdge;
  Node *newNode;

  if (nodes != NULL)  {
    for (unsigned i = 0; i < nodes->size(); i++)  { if ((*nodes)[i] != NULL)  { delete (*nodes)[i]; } }
    delete nodes;
  }

  nodes = new vector<Node *>(this->V, NULL);
  traverse.clear();

  // we should not have an empty array
  assert(!nodes->empty());
  assert(traverse.empty());

  // from the edges passed, fill the adjacency matrix
  for (ei = edges->begin(); ei != edges->end(); ++ei)  {
    anEdge = *ei;
    // the matrix is indexed by the first vertex of the edge
    // the second vertex of the edge is pushed onto another
    // vector of all vertices connected to the first vertex
    // with a directed edge
    #ifdef ADJMAKE_DEBUG
      cout << "vertex1: " << anEdge.first << ", vertex2: " << anEdge.second << endl;
    #endif
    if (!find_node(nodes, anEdge.first))  {
      #ifdef ADJMAKE_DEBUG
        cout << "Making a new vector indexed at: " << anEdge.first << endl;
      #endif

      newNode = new Node(anEdge.first);
      newNode->dests.push_back(anEdge.second);
      (*nodes)[anEdge.first] = newNode;
    }
    else  {
      (*nodes)[anEdge.first]->dests.push_back(anEdge.second);
    }

    // check if the destination vertex exists in the nodes map
    if (!find_node(nodes, anEdge.second))  {
      #ifdef ADJMAKE_DEBUG
        cerr << "Making a new vector indexed at: " << anEdge.second << endl;
      #endif

      newNode = new Node(anEdge.second);
      (*nodes)[anEdge.second] = newNode;
    }
  }

  assert(!nodes->empty());

  // the following block is code to print out
  // the adjacency matrix.
  #ifdef ADJMAKE_DEBUG
    for (unsigned i = 0; i < nodes->size(); i++)  {
      Node *node = (*nodes)[i];
      cout << "nodes[" << node->id << "]: ";
      
      if (isSink(node))  { cout << "SINK"; }
      else  {
        for (unsigned j = 0; j < node->dests.size(); j++)  { cout << node->dests[j] << " "; }
      }
      cout << endl;
    }
  #endif
}

vector<bool> *CycleDetector::detect_cycles()  {
  cyclicEdgesMapping = new vector<bool>(edges->size(), false);

  assert(!nodes->empty());
  assert(!edges->empty());

  // make a copy of the graph to ensure that we have visited all
  // vertices
  traverse.clear(); assert(traverse.empty());
  for (unsigned i = 0; i < V; i++)  { traverse.push_back(i); }
  #ifdef SETUP_DEBUG
    for (unsigned i = 0; i < traverse.size(); i++)  {
      cout << "traverse{" << i << "}: " << traverse[i] << endl;
    }
  #endif

  // find the cycles
  assert(nodes->size() > 1);

  // while we still have vertices to visit, visit.
  while (!traverse.empty())  {
    // mark any vertices seen in a previous run as closed
    while (!seenInRun.empty())  {
      unsigned v = seenInRun.top();
      seenInRun.pop();
      #ifdef RUN_DEBUG
        cout << "Marking vertex(" << v << ") as CLOSED" << endl;
      #endif
      (*nodes)[v]->status = Node::DoneVisiting;
    }

    assert(seenInRun.empty());

    #ifdef VISIT_DEBUG
      cout << "begining search at vertex(" << traverse[0] << ")" << endl;
    #endif

    Time = 0;

    // go go go
    visit(traverse[0]);
  }

  // clean up
  while (!seenInRun.empty())  { seenInRun.pop(); }
  assert(seenInRun.empty());
  assert(traverse.empty());

  return cyclicEdgesMapping;
}

void CycleDetector::mod_graph(unsigned numVertices, Edges *edges)  {
  this->V = numVertices;
  this->edges = edges;
  // remake the adjaceny matrix
  this->make_matrix();
  assert(nodes->size() == this->V);
}

void CycleDetector::visit(unsigned k)  {
  unsigned cycleOpen;
  Node *thisNode = (*nodes)[k];

  // state that we have seen this vertex
  pair< bool, vector<unsigned>::iterator > haveSeen = find_node(traverse, k);
  if (haveSeen.first)  {
    #ifdef VISIT_DEBUG
      cout << "Visiting vertex(" << k << ") for the first time" << endl;
    #endif
    traverse.erase(haveSeen.second);
  }

  seenInRun.push(k);

  // set up this node as being visited.
  thisNode->stamp = ++Time;
  thisNode->status = Node::BeingVisited;

  // traverse to all the vertices adjacent to this vertex.
  for (unsigned n = 0; n < thisNode->dests.size(); n++)  {
    Node *otherNode = (*nodes)[thisNode->dests[n]];

    if (otherNode->status != Node::DoneVisiting)  {
      if (otherNode->status == Node::NotVisited)  {  
        // visit this node
	#ifdef VISIT_DEBUG
          cout << "traversing from vertex(" << k << ") to vertex(" << otherNode->id << ")" << endl;
	#endif
        visit(otherNode->id);
      }

      // if we are part of a cycle get the timestamp of the ancestor
      if (otherNode->cyclicAncestor != NULL)  { cycleOpen = otherNode->cyclicAncestor->stamp; }
      // else just get the timestamp of the node we just visited
      else  { cycleOpen = otherNode->stamp; }

      // compare the stamp of the traversal with our stamp
      if (cycleOpen <= thisNode->stamp)  {
	if (otherNode->cyclicAncestor == NULL)  { otherNode->cyclicAncestor = otherNode; }

	// store the cycle
	for (unsigned i = 0; i < edges->size(); i++)  {
	  if ((*edges)[i] == Edge(k, otherNode->id))  { (*cyclicEdgesMapping)[i] = true; }
          #ifdef OUTPUT_DEBUG
            cout << "Setting cyclicEdgesMapping[" << i << "] to true" << endl;
          #endif
	}

        // this node is part of a cycle
        if (thisNode->cyclicAncestor == NULL)  { thisNode->cyclicAncestor = otherNode->cyclicAncestor; }

	// see if we are part of a cycle with a cyclicAncestor that possesses a lower timestamp
	if (otherNode->cyclicAncestor->stamp < thisNode->cyclicAncestor->stamp)  { thisNode->cyclicAncestor = otherNode->cyclicAncestor; }
      }
    }
  }
}

// determines whether or not a vertex is a sink
bool CycleDetector::isSink(Node *node)  {
  // a vertex is a sink if it has no outgoing edges,
  // or that the adj entry is empty
  if (node->dests.empty())  { return true; }
  else  { return false; }
}

bool CycleDetector::find_node(std::vector<Node *> *& list, unsigned k)  {
  for (unsigned i = 0; i < this->V; i++)  {
    if ((*list)[i] != NULL)  {
      if ((*list)[i]->id == k)  { return true; }
    }
  }

  return false;
}

pair< bool, vector<unsigned>::iterator > CycleDetector::find_node(std::vector<unsigned>& list, unsigned k)  {
  for (vector<unsigned>::iterator ti = traverse.begin(); ti != traverse.end(); ++ti)  {
    if (*ti == k)  { return pair< bool, vector<unsigned>::iterator >(true, ti); }
  }

  return pair< bool, vector<unsigned>::iterator >(false, traverse.end()); 
}
