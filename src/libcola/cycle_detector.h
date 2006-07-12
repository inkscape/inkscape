#ifndef CYCLE_DETECTOR_H
#define CYCLE_DETECTOR_H

#include <map>
#include <vector>
#include <stack>
#include "cola.h"

typedef unsigned TimeStamp;
typedef std::vector<cola::Edge> Edges;
typedef std::vector<bool> CyclicEdges;
class Node;

class CycleDetector  {
  public:
    CycleDetector(unsigned numVertices, Edges *edges);
    ~CycleDetector();
    std::vector<bool> *detect_cycles();
    void mod_graph(unsigned numVertices, Edges *edges);
    unsigned getV()  { return this->V; }
    Edges *getEdges()  { return this->edges; }

  private:
    // attributes
    unsigned V;
    Edges *edges;

    // internally used variables.
    std::map<unsigned, Node *> nodes; // the nodes in the graph
    std::map<cola::Edge, bool> cyclicEdges; // the cyclic edges in the graph.
    std::map<unsigned, bool> traverse; // nodes still left to visit in the graph
    std::stack<unsigned> seenInRun; // nodes visited in a single pass.

    // internally used methods
    void make_matrix();
    void visit(unsigned k);
    bool isSink(Node *node);
};

class Node  {
  public:
    enum StatusType { NotVisited, BeingVisited, DoneVisiting };

    unsigned id;
    TimeStamp stamp;
    Node *cyclicAncestor;
    vector<unsigned> dests;
    StatusType status;

    Node(unsigned id)  { this->id = id; cyclicAncestor = NULL; status = NotVisited; }
    ~Node()  {}
};

#endif
