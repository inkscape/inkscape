// vim: set cindent 
// vim: ts=4 sw=4 et tw=0 wm=0
#include <vector>

template <class T>
class PairNode;
namespace shortest_paths {

struct Node {
    unsigned id;
    double d;
    Node *p; // predecessor    
    std::vector<Node*> neighbours;
    std::vector<double> nweights;
    PairNode<Node*> *qnode;
};
inline bool compareNodes(Node *const &u, Node *const &v) {
    return u->d < v->d;
}

typedef std::pair<unsigned,unsigned> Edge;

void floyd_warshall(unsigned n, double** D,
        std::vector<Edge>& es,double* eweights); 

void johnsons(unsigned n, double** D,
        std::vector<Edge>& es, double* eweights);

void dijkstra(unsigned s, unsigned n, double* d, 
        std::vector<Edge>& es, double* eweights);
}
