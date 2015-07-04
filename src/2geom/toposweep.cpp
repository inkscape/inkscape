#include <2geom/toposweep.h>

#include <2geom/path-intersection.h>
#include <2geom/basic-intersection.h>

//using namespace Geom;

namespace Geom {

TopoGraph::Edge &TopoGraph::Vertex::operator[](unsigned ix) {
    ix %= degree();
    return ix < enters.size() ? enters[ix] : exits[ix - enters.size()];
}

TopoGraph::Edge TopoGraph::Vertex::operator[](unsigned ix) const {
    ix %= degree();
    return ix < enters.size() ? enters[ix] : exits[ix - enters.size()];
}

void TopoGraph::Vertex::erase(unsigned ix) {
    ix %= degree();
    if(ix < enters.size())
        enters.erase(enters.begin() + ix);
    else
        exits.erase(exits.begin() + (ix - enters.size()));
}

void TopoGraph::Vertex::insert(unsigned ix, Edge v) {
    ix %= degree();
    if(ix < enters.size())
        enters.insert(enters.begin() + ix, v);
    else
        exits.insert(exits.begin() + (ix - enters.size()), v);
}

unsigned TopoGraph::Vertex::find_section(boost::shared_ptr<Section> section) const {
    unsigned i = 0;
    for(; i < degree(); i++)
        if((*this)[i].section == section) return i;
    return i;
}

TopoGraph::Edge TopoGraph::remove_edge(unsigned ix, unsigned jx) {
    Vertex &v = vertices[ix];
    if(v.degree()) {
        jx %= v.degree();
        Edge &ret = v[jx];
        v.erase(jx);
        v = vertices[ret.other];
        if(v.degree()) {
            v.erase(v.find_section(ret.section));
            return ret;
        }
    }
    assert(0);
    return v[0]; // if assert is disabled, return first Edge.
}

void TopoGraph::cannonize() {
    std::vector<unsigned> vix;
    unsigned ix = 0;
    for(unsigned i = 0; i < vertices.size(); i++) {
        vix.push_back(ix);
        if(vertices[i].degree() != 0) vertices[ix++] = vertices[i];
    }
    
    for(unsigned i = 0; i < ix; i++)
        for(unsigned j = 0; j < vertices[i].degree(); j++)
            vertices[i][j].other = vix[vertices[i][j].other];
}


void TopoGraph::assert_invariants() const {
    for(unsigned i = 0; i < vertices.size(); i++) {
        for(unsigned j = 0; j < vertices[i].degree(); j++) {
            Edge e = vertices[i][j];
            assert(e.other != i);
            assert(are_near(e.section->fp, vertices[i].avg, tol) || are_near(e.section->tp, vertices[i].avg, tol));
            assert(!are_near(e.section->fp, e.section->tp, tol));
            assert(e.section.get());
            unsigned oix = vertices[e.other].find_section(e.section);
            assert(oix != vertices[e.other].degree());
        }
    }
}

//near predicate utilized in process_splits
template<typename T>
struct NearPredicate { bool operator()(T x, T y) { return are_near(x, y); } };

// ensures that f and t are elements of a vector, sorts and uniqueifies
// also asserts that no values fall outside of f and t
// if f is greater than t, the sort is in reverse
void process_splits(std::vector<double> &splits, double f, double t) {
    splits.push_back(f);
    std::sort(splits.begin(), splits.end());
    while(are_near(splits.back(), t)) splits.erase(splits.end() - 1);
    splits.push_back(t);
    if(f > t) std::reverse(splits.begin(), splits.end());

    //remove any splits which fall outside t / f
    while(!splits.empty() && splits.front() != f) splits.erase(splits.begin());
    while(!splits.empty() && splits.back() != t) splits.erase(splits.end() - 1);
    
    std::vector<double>::iterator end = std::unique(splits.begin(), splits.end(), NearPredicate<double>());
    splits.resize(end - splits.begin());
}

// A little sugar for appending a list to another
template<typename T>
void concatenate(T &a, T const &b) { a.insert(a.end(), b.begin(), b.end()); }

//returns a list of monotonic sections of a path
//TODO: handle saddle points
std::vector<boost::shared_ptr<Section> > mono_sections(PathVector const &ps, Dim2 d) {
    std::vector<boost::shared_ptr<Section> > monos;
    for(unsigned i = 0; i < ps.size(); i++) {
        //TODO: necessary? can we have empty paths?
        if(ps[i].size()) {
            for(unsigned j = 0; j < ps[i].size(); j++) {
                //find the points of 0 derivative
                Curve* deriv = ps[i][j].derivative();
                std::vector<double> splits = deriv->roots(0, X);
                concatenate(splits, deriv->roots(0, Y));
                delete deriv;
                process_splits(splits, 0, 1);
                //split on points of 0 derivative
                for(unsigned k = 1; k < splits.size(); k++)
                    monos.push_back(boost::shared_ptr<Section>(new Section(CurveIx(i,j), splits[k-1], splits[k], ps, d)));
            }
        }
    }
    return monos;
}

//finds the t-value on a section, which corresponds to a particular horizontal or vertical line
//d indicates the dimension along which the roots is performed.
//-1 is returned if no root is found
double section_root(Section const &s, PathVector const &ps, double v, Dim2 d) {
    std::vector<double> roots = s.curve.get(ps).roots(v, d);
    for(unsigned j = 0; j < roots.size(); j++)
        if(Interval(s.f, s.t).contains(roots[j])) return roots[j];
    return -1;
}

bool SectionSorter::section_order(Section const &a, double at, Section const &b, double bt) const {
    Point ap = a.curve.get(ps).pointAt(at);
    Point bp = b.curve.get(ps).pointAt(bt);
    if(are_near(ap[dim], bp[dim], tol)) {
        // since the sections are monotonic, if the endpoints are on opposite sides of this
        // coincidence, the order is determinable
        if(a.tp[dim] < ap[dim] && b.tp[dim] > bp[dim]) return true;
        if(a.tp[dim] > ap[dim] && b.tp[dim] < bp[dim]) return false;
        //TODO: sampling / higher derivatives when unit tangents match
        Point ad = a.curve.get(ps).unitTangentAt(a.f);
        Point bd = b.curve.get(ps).unitTangentAt(b.f);
        // tangent can point backwards
        if(ad[1-dim] < 0) ad = -ad;
        if(bd[1-dim] < 0) bd = -bd;
        return ad[dim] < bd[dim];
    }
    return ap[dim] < bp[dim];
}

bool SectionSorter::operator()(Section const &a, Section const &b) const {
    if(&a == &b) return false;
    Rect ra = a.bbox(), rb = b.bbox();
    //TODO: should we use tol in these conditions?
    if(ra[dim].max() <= rb[dim].min()) return true;
    if(rb[dim].max() <= ra[dim].min()) return false;
    //we know that the rects intersect on dim
    //by referencing f / t we are assuming that the section was constructed with 1-dim
    if(ra[1-dim].intersects(rb[1-dim])) {
        if(are_near(a.fp[1-dim], b.fp[1-dim], tol)) {
            return section_order(a, a.f > a.t ? a.f - 0.01 : a.f + 0.01,
                                 b, b.f > b.t ? b.f - 0.01 : b.f + 0.01);
        } else if(a.fp[1-dim] < b.fp[1-dim]) {
            //b inside a
            double ta = section_root(a, ps, b.fp[1-dim], Dim2(1-dim));
            //TODO: fix bug that necessitates this
            if(ta == -1) ta = (a.t + a.f) / 2;
            return section_order(a, ta, b, b.f);
        } else {
            //a inside b
            double tb = section_root(b, ps, a.fp[1-dim], Dim2(1-dim));
            //TODO: fix bug that necessitates this
            if(tb == -1) tb = (b.t + b.f) / 2;
            return section_order(a, a.f, b, tb);
        }
    }
    
    return Point::LexLessRt(dim)(a.fp, b.fp);
}

// splits a section into pieces, as specified by an array of doubles, mutating the section to
// represent the first part, and returning the rest
//TODO: output iterator?
std::vector<boost::shared_ptr<Section> > split_section(boost::shared_ptr<Section> s, PathVector const &ps, std::vector<double> &cuts, Dim2 d) {
    std::vector<boost::shared_ptr<Section> > ret;
    
    process_splits(cuts, s->f, s->t);
    if(cuts.size() <= 2) return ret;
    
    s->t = cuts[1];
    s->tp = s->curve.get(ps)(cuts[1]);
    assert(Point::LexLessRt(d)(s->fp, s->tp));
    
    ret.reserve(cuts.size() - 2);
    for(int i = cuts.size() - 1; i > 1; i--) ret.push_back(boost::shared_ptr<Section>(new Section(s->curve, cuts[i-1], cuts[i], ps, d)));
    return ret;
}

//merges the sorted lists a and b according to comparison z
template<typename X, typename Z>
void merge(X &a, X const &b, Z const &z) {
     a.reserve(a.size() + b.size());
     unsigned start = a.size();
     concatenate(a, b);
     std::inplace_merge(a.begin(), a.begin() + start, a.end(), z);
}

//TODO: faster than linear
unsigned find_vertex(std::vector<TopoGraph::Vertex> const &vertices, Point p, double tol) {
    for(unsigned i = 0; i < vertices.size(); i++)
        if(are_near(vertices[i].avg, p, tol)) return i;
    return vertices.size();
}

//takes a vector of T pointers, and returns a vector of T with copies
template<typename T>
std::vector<T> deref_vector(std::vector<boost::shared_ptr<T> > const &xs, unsigned from = 0) {
    std::vector<T> ret;
    ret.reserve(xs.size() - from);
    for(unsigned i = from; i < xs.size(); i++)
        ret.push_back(T(*xs[i]));
    return ret;
}

//used to create reversed sorting predicates
template<typename C>
struct ReverseAdapter {
    typedef typename C::second_argument_type first_argument_type;
    typedef typename C::first_argument_type second_argument_type;
    typedef typename C::result_type result_type;
    const C &comp;
    ReverseAdapter(const C &c) : comp(c) {}
    result_type operator()(const first_argument_type &a, const second_argument_type &b) const { return comp(b, a); }
};

//used to sort std::vector<Section*>
template<typename C>
struct DerefAdapter {
    typedef typename boost::shared_ptr<typename C::first_argument_type> first_argument_type;
    typedef typename boost::shared_ptr<typename C::second_argument_type> second_argument_type;
    typedef typename C::result_type result_type;
    const C &comp;
    DerefAdapter(const C &c) : comp(c) {}
    result_type operator()(const first_argument_type a, const second_argument_type b) const {
        if(!a) return false;
        if(!b) return true;
        return comp(*a, *b);
    }
};

struct EdgeSorter {
    typedef TopoGraph::Edge first_argument_type;
    typedef TopoGraph::Edge second_argument_type;
    typedef bool result_type;
    SectionSorter s;
    EdgeSorter(const PathVector &rs, Dim2 d, double t) : s(rs, d, t) {}
    bool operator()(TopoGraph::Edge const &e1, TopoGraph::Edge const &e2) const { return s(*e1.section, *e2.section); }
};

#ifdef SWEEP_GRAPH_DEBUG
//used for debugging purposes - each element represents a subsequent iteration of the algorithm.
std::vector<std::vector<Section> > monoss;
std::vector<std::vector<Section> > chopss;
std::vector<std::vector<Section> > contexts;
#endif

/*
 1) take item off sweep sorted todo
 2) find all of the to-values before the beginning of this section
 3) sort these lexicographically, process them in order, grouping other sections in the context, and constructing a vertex in one fell swoop.
 4) add our section into context, splitting on intersections
 
 3 is novel, we perform it by storing 
 */

template<typename A, typename B, typename Z>
struct MergeIterator {
    A const &a;
    B &b;
    Z const &z;
    unsigned ai;
    bool on_a;
    MergeIterator(A const &av, B &bv, Z const &zv) : a(av), b(bv), z(zv), ai(0), on_a(b.empty() || z(a[0], b.back())) {}
    MergeIterator &operator++() {
        if(!done()) {
            on_a = b.empty() ? true : (ai >= a.size() ? false : z(a[ai], b.back()));
            if(on_a) {
                ++ai;
                if(ai >= a.size()) on_a = false;
            } else {
                b.erase(b.end());
                if(b.empty()) on_a = true;
            }
        }
        return *this;
    }
    typename A::value_type operator*() {
        assert(!done());
        return on_a ? a[ai] : b.back();
    }
    bool done() { return b.empty() && ai >= a.size() - 1; }
    typename A::value_type operator->() { assert(!done()); return on_a ? a[ai] : b.back(); }
};

void modify_windings(std::vector<int> &windings, boost::shared_ptr<Section> sec, Dim2 d) {
    unsigned k = sec->curve.path;
    if(k >= windings.size() || sec->fp[d] == sec->tp[d]) return;
    if(sec->f < sec->t) windings[k]++;
    if(sec->f > sec->t) windings[k]--;
}

struct Context {
    boost::shared_ptr<Section> section;
    int from_vert;
    int to_vert;
    Context(boost::shared_ptr<Section> sect, int from) : section(sect), from_vert(from), to_vert(-1) {}
};

template<typename C>
struct ContextAdapter {
    typedef Context first_argument_type;
    typedef typename C::second_argument_type second_argument_type;
    typedef typename C::result_type result_type;
    const C &comp;
    ContextAdapter(const C &c) : comp(c) {}
    result_type operator()(const Context &a, const second_argument_type &b) const { return comp(a.section, b); }
};

#define DINF std::numeric_limits<double>::infinity()

TopoGraph::TopoGraph(PathVector const &ps, Dim2 d, double t) : dim(d), tol(t) {
    //s_sort = vertical section order
    ContextAdapter<DerefAdapter<SectionSorter> > s_sort = DerefAdapter<SectionSorter>(SectionSorter(ps, (Dim2)(1-d), tol));
    //sweep_sort = horizontal sweep order
    DerefAdapter<SweepSorter> sweep_sort = DerefAdapter<SweepSorter>(SweepSorter(d));
    //heap_sort = reverse horizontal sweep order
    ReverseAdapter<DerefAdapter<SweepSorter> > heap_sort = ReverseAdapter<DerefAdapter<SweepSorter> >(sweep_sort);
    //edge_sort = sorter for edges
    EdgeSorter edge_sort = EdgeSorter(ps, (Dim2)(1-d), tol);
    
    std::vector<boost::shared_ptr<Section> > input_sections = mono_sections(ps, d), chops;
    std::sort(input_sections.begin(), input_sections.end(), sweep_sort);
    
    std::vector<Context> context;
    
    vertices.reserve(input_sections.size());
    
    //std::vector<unsigned> to_process;
    
    std::vector<int> windings(ps.size(), 0);
    for(MergeIterator<Area, Area, DerefAdapter<SweepSorter> > iter(input_sections, chops, sweep_sort); ; ++iter) {
        //represents our position in the sweep, which controls what we finalize
        //if we have no more to process, finish the rest by setting our position to infinity
        Point lim;
        if(iter.done()) lim[X] = lim[Y] = DINF; else lim = iter->fp;
        
        /*
        //finalize vertices
        for(unsigned i = 0; i < to_process.size(); i++)  {
            if(vertices[to_process[i]].avg[d] + tol < lim[d])
            for(unsigned j = 0; j < context.size(); j++) {
                
            }
        } */
        
        //find all sections to remove
        for(int i = context.size() - 1; i >= 0; i--) {
            boost::shared_ptr<Section> sec = context[i].section;
            if(Point::LexLessRt(d)(lim, sec->tp)) {
                //sec->tp is less than or equal to lim
                if(context[i].to_vert == -1) {
                    //we need to create a new vertex; add everything that enters it
                    //Point avg;
                    //unsigned cnt;
                    std::vector<Edge> enters;
                    std::fill(windings.begin(), windings.end(), 0);
                    for(unsigned j = 0; j < context.size(); j++) {
                        modify_windings(windings, context[j].section, d);
                        if(are_near(sec->tp, context[j].section->tp, tol)) {
                            assert(-1 == context[j].to_vert);
                            context[j].section->windings = windings;
                            context[j].to_vert = vertices.size();
                            enters.push_back(Edge(context[j].section, context[j].from_vert));
                            //avg += context[j].section->tp;
                            //cnt++;
                        }
                    }
                    //Vertex &v(avg / (double)cnt);
                    Vertex v(context[i].section->tp);
                    v.enters = enters;
                    vertices.push_back(v);
                    //to_process.push_back(vertices.size() - 1);
                }
                context.erase(context.begin() + i);
            }
        }
        
        if(!iter.done()) {
            boost::shared_ptr<Section> s = *iter;
            
            //create a new context, associate a beginning vertex, and insert it in the proper location
            unsigned ix = find_vertex(vertices, s->fp, tol);
            if(ix == vertices.size()) {
                vertices.push_back(Vertex(s->fp));
                //to_process.push_back(vertices.size() - 1);
            }
            unsigned context_ix = std::lower_bound(context.begin(), context.end(), s, s_sort) - context.begin();

            context.insert(context.begin() + context_ix, Context(s, ix));
            
            Interval si = Interval(s->fp[1-d], s->tp[1-d]);
            
            // Now we intersect with neighbors - do a sweep!
            std::vector<double> this_splits;
            for(unsigned i = 0; i < context.size(); i++) {
                if(context[i].section == context[context_ix].section) continue;
                
                boost::shared_ptr<Section> sec = context[i].section;
                
                if(!si.intersects(Interval(sec->fp[1-d], sec->tp[1-d]))) continue;
                
                std::vector<double> other_splits;
                Crossings xs = mono_intersect(s->curve.get(ps), Interval(s->f, s->t),
                                              sec->curve.get(ps), Interval(sec->f, sec->t));
                if(xs.empty()) continue;
                
                for(unsigned j = 0; j < xs.size(); j++) {
                    this_splits.push_back(xs[j].ta);
                    other_splits.push_back(xs[j].tb);
                }
                merge(chops, split_section(sec, ps, other_splits, d), heap_sort);
            }
            if(!this_splits.empty())
                merge(chops, split_section(context[context_ix].section, ps, this_splits, d), heap_sort);
            
            std::sort(chops.begin(), chops.end(), heap_sort);
            
            if(context[context_ix].section->tp[d] - context[context_ix].section->fp[d] <= tol) {
                if(!are_near(context[context_ix].section->tp, context[context_ix].section->fp, tol)) {
                    ix = find_vertex(vertices, context[context_ix].section->tp, tol);
                    if(ix != vertices.size()) {
                        boost::shared_ptr<Section> sec = context[context_ix].section;
                        Edge e(sec, context[context_ix].from_vert);
                        
                        std::vector<Edge>::iterator it = std::lower_bound(vertices[ix].enters.begin(), vertices[ix].enters.end(), e, edge_sort);
                        
                        if(vertices[ix].enters.empty()) {                        
                            std::fill(windings.begin(), windings.end(), 0);
                            for(unsigned j = 0; j <= context_ix; j++) modify_windings(windings, context[j].section, d);
                        } else if(it == vertices[ix].enters.end()) {
                            windings = (it-1)->section->windings;
                            modify_windings(windings, (it-1)->section, d);
                        } else {
                            windings = it->section->windings;
                        }
                                   
                        sec->windings = windings;
                        modify_windings(windings, sec, d);
                        
                        for(std::vector<Edge>::iterator it2 = it; it2 != vertices[ix].enters.end(); ++it2) {
                            it2->section->windings = windings;
                            modify_windings(windings, it2->section, d);
                        }
                        
                        vertices[ix].enters.insert(it, e);
                        context.erase(context.begin() + context_ix);
                    }
                } else context.erase(context.begin() + context_ix);
            }
        }
        
        #ifdef SWEEP_GRAPH_DEBUG
        std::vector<Section> rem;
        for(unsigned i = iter.ai + 1; i < iter.a.size(); i++) rem.push_back(*iter.a[i]);
        monoss.push_back(rem);
        chopss.push_back(deref_vector(iter.b));
        rem.clear();
        for(unsigned i = 0; i < context.size(); i++) rem.push_back(*context[i].section);
        contexts.push_back(rem);
        #endif
        
        if(iter.done() && context.empty()) return;
    }
}

void trim_whiskers(TopoGraph &g) {
    std::vector<unsigned> affected;
    
    for(unsigned i = 0; i < g.size(); i++)
        if(g[i].degree() == 1) affected.push_back(i);
    
    while(!affected.empty()) {
        unsigned j = 0;
        for(unsigned i = 0; i < affected.size(); i++)
            if(g[affected[i]].degree() == 1)
                affected[j++] = g.remove_edge(affected[i], 0).other;
        affected.resize(j);
    }
}

void add_edge_at(TopoGraph &g, unsigned ix, boost::shared_ptr<Section> s, TopoGraph::Edge jx, bool before = true) {
    TopoGraph::Vertex &v = g[ix];
    for(unsigned i = 0; i < v.enters.size(); i++) {
        if(v.enters[i].section == s) {
            v.enters.insert(v.enters.begin() + (before ? i : i + 1), jx);
            return;
        }
    }
    for(unsigned i = 0; i < v.exits.size(); i++) {
        if(v.exits[i].section == s) {
            v.exits.insert(v.exits.begin() + (before ? i : i + 1), jx);
            return;
        }
    }
    //TODO: fix the fall through to here
    //assert(false);
}

void double_whiskers(TopoGraph &g) {
    for(unsigned i = 0; i < g.size(); i++) {
        if(g[i].degree() == 1) {
            unsigned j = i;
            TopoGraph::Edge e = g[i][0];
            while(true) {
                TopoGraph::Edge next_edge = g[j][1 - g[j].find_section(e.section)];
                boost::shared_ptr<Section> new_section = boost::shared_ptr<Section>(new Section(*e.section));
                add_edge_at(g, j,       e.section, TopoGraph::Edge(new_section, e.other), false);
                add_edge_at(g, e.other, e.section, TopoGraph::Edge(new_section, j), true);
                
                if(g[e.other].degree() == 3) {
                    j = e.other;
                    e = next_edge;
                } else break;
            }
        }
    }
}

/*
void remove_degenerate(TopoGraph &g) {
    for(unsigned i = 0; i < g.size(); i++) {
        for(int j = g[i].degree(); j >= 0; j--) {
            if(g[i][j].other == i) 
        }
    }
}*/

/*
void remove_vestigial(TopoGraph &g) {
    for(unsigned i = 0; i < g.size(); i++) {
        if(g[i].enters.size() == 1 && g[i].exits.size() == 1) {
            TopoGraph::Edge &e1 = g[i][0], &e2 = g[i][1];
            if(e1.section == e2.section) {
                //vestigial vert
                Section *new_section = new Section(e1.section->curve,
                                         e1.section->f,  e2.section->t,
                                         e1.section->fp, e2.section->tp);
                
                e1.other
                
                Vertex *v1 = e1.other, *v2 = e2.other;
                v1->lookup_section(e1.section) = Edge(new_section, v2);
                v2->lookup_section(e2.section) = Edge(new_section, v1);
                g.erase(g.begin() + i);
            }    
        }
    }
}*/

//planar area finding
//linear on number of edges
Areas traverse_areas(TopoGraph const &g) {
    Areas ret;
    
    //stores which edges we've visited
    std::vector<std::vector<bool> > visited;
    for(unsigned i = 0; i < g.size(); i++) visited.push_back(std::vector<bool>(g[i].degree(), false));
    
    for(unsigned vix = 0; vix < g.size(); vix++) {
        while(true) {
            //find an unvisited edge to start on
            
            unsigned e_ix = std::find(visited[vix].begin(), visited[vix].end(), false) - visited[vix].begin();
            if(e_ix == g[vix].degree()) break;
            
            unsigned start = e_ix;
            unsigned cur = vix;
            
            Area area;
            //std::vector<std::vector<bool> > before(visited);
            while(cur < g.size() && !visited[cur][e_ix]) {
                visited[cur][e_ix] = true;
                
                TopoGraph::Edge e = g[cur][e_ix];
                
                area.push_back(e.section);
                
                //go to clockwise edge
                cur = e.other;
                unsigned deg = g[cur].degree();
                e_ix = g[cur].find_section(e.section);
                
                if(deg == 1 || e_ix == deg) {
                   visited[cur][e_ix] = true;
                   break;
                }
                
                e_ix = (e_ix + 1) % deg;
                
                if(cur == vix && start == e_ix) break;
            }
            //if(vix == cur && start == e_ix) {
                ret.push_back(area);
            //} else visited = before;
        }
    }
    return ret;
}

void remove_area_whiskers(Areas &areas) {
    for(int i = areas.size() - 1; i >= 0; i--)
        if(areas[i].size() == 2 && *areas[i][0] == *areas[i][1]) 
            areas.erase(areas.begin() + i);
}

Path area_to_path(PathVector const &ps, Area const &area) {
    Path ret;
    ret.setStitching(true);
    if(area.size() == 0) return ret;
    Point prev = area[0]->fp;
    for(unsigned i = 0; i < area.size(); i++) {
        bool forward = are_near(area[i]->fp, prev, 0.01);
        Curve *curv = area[i]->curve.get(ps).portion(
                          forward ? area[i]->f : area[i]->t,
                          forward ? area[i]->t : area[i]->f);
        ret.append(*curv);
        delete curv;
        prev = forward ? area[i]->tp : area[i]->fp;
    }
    ret.setStitching(false);
    return ret;
}

PathVector areas_to_paths(PathVector const &ps, Areas const &areas) {
    PathVector ret;
    //ret.reserve(areas.size());
    for(unsigned i = 0; i < areas.size(); ++i)
        ret.push_back(area_to_path(ps, areas[i]));
    return ret;
}

} // end namespace Geom
