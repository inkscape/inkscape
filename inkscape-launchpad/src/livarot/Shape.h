/*
 *  Digraph.h
 *  nlivarot
 *
 *  Created by fred on Thu Jun 12 2003.
 *
 */

#ifndef my_shape
#define my_shape

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <2geom/point.h>

#include "livarot/LivarotDefs.h"

class Path;
class FloatLigne;

class SweepTree;
class SweepTreeList;
class SweepEventQueue;

enum {
  tweak_mode_grow,
  tweak_mode_push,
  tweak_mode_repel,
  tweak_mode_roughen
};

/*
 * the Shape class (was the Digraph class, as the header says) stores digraphs (no kidding!) of which 
 * a very interesting kind are polygons.
 * the main use of this class is the ConvertToShape() (or Booleen(), quite the same) function, which
 * removes all problems a polygon can present: duplicate points or edges, self-intersection. you end up with a
 * full-fledged polygon
 */

// possible values for the "type" field in the Shape class:
enum
{
  shape_graph = 0,                // it's just a graph; a bunch of edges, maybe intersections
  shape_polygon = 1,                // a polygon: intersection-free, edges oriented so that the inside is on their left
  shape_polypatch = 2                // a graph without intersection; each face is a polygon (not yet used)
};

class IntLigne;
class BitLigne;
class AlphaLigne;

class Shape
{
public:

    struct back_data
    {
        int pathID, pieceID;
        double tSt, tEn;
    };
    
    struct voronoi_point
    {                                // info for points treated as points of a voronoi diagram (obtained by MakeShape())
        double value;                // distance to source
        int winding;                // winding relatively to source
    };
    
    struct voronoi_edge
    {                                // info for edges, treated as approximation of edges of the voronoi diagram
        int leF, riF;                // left and right site
        double leStX, leStY, riStX, riStY;        // on the left side: (leStX,leStY) is the smallest vector from the source to st
        // etc...
        double leEnX, leEnY, riEnX, riEnY;
    };

    struct quick_raster_data
    {
        double x;                            // x-position on the sweepline
        int    bord;                        // index of the edge
        int    ind;       // index of qrsData elem for edge (ie inverse of the bord)
        int    next,prev; // dbl linkage
    };

    enum sTreeChangeType
    {
        EDGE_INSERTED = 0,
        EDGE_REMOVED = 1,
        INTERSECTION = 2
    };
  
    struct sTreeChange
    {
        sTreeChangeType type;                // type of modification to the sweepline:
        int ptNo;                        // point at which the modification takes place

        Shape *src;                        // left edge (or unique edge if not an intersection) involved in the event
        int bord;
        Shape *osrc;                // right edge (if intersection)
        int obord;
        Shape *lSrc;                // edge directly on the left in the sweepline at the moment of the event
        int lBrd;
        Shape *rSrc;                // edge directly on the right
        int rBrd;
    };
    
    struct incidenceData
    {
        int nextInc;                // next incidence in the linked list
        int pt;                        // point incident to the edge (there is one list per edge)
        double theta;                // coordinate of the incidence on the edge
    };
    
    Shape();
    virtual ~Shape();

    void MakeBackData(bool nVal);
    void MakeVoronoiData(bool nVal);

    void Affiche();

    // insertion/deletion/movement of elements in the graph
    void Copy(Shape *a);
    // -reset the graph, and ensure there's room for n points and m edges
    void Reset(int n = 0, int m = 0);
    //  -points:
    int AddPoint(const Geom::Point x);        // as the function name says
    // returns the index at which the point has been added in the array
    void SubPoint(int p);        // removes the point at index p
    // nota: this function relocates the last point to the index p
    // so don't trust point indices if you use SubPoint
    void SwapPoints(int a, int b);        // swaps 2 points at indices a and b
    void SwapPoints(int a, int b, int c);        // swaps 3 points: c <- a <- b <- c
    void SortPoints();        // sorts the points if needed (checks the need_points_sorting flag)

    //  -edges:
    // add an edge between points of indices st and en    
    int AddEdge(int st, int en);
    // return the edge index in the array
    
    // add an edge between points of indices st and en    
    int AddEdge(int st, int en, int leF, int riF);
    // return the edge index in the array
    
    // version for the voronoi (with faces IDs)
    void SubEdge(int e);                // removes the edge at index e (same remarks as for SubPoint)
    void SwapEdges(int a, int b);        // swaps 2 edges
    void SwapEdges(int a, int b, int c);        // swaps 3 edges
    void SortEdges();        // sort the edges if needed (checks the need_edges_sorting falg)

    // primitives for topological manipulations
  
    // endpoint of edge at index b that is different from the point p      
    inline int Other(int p, int b) const
    {
        if (getEdge(b).st == p) {
            return getEdge(b).en;
        }
        return getEdge(b).st;
    }

    // next edge (after edge b) in the double-linked list at point p  
    inline int NextAt(int p, int b) const
    {
        if (p == getEdge(b).st) {
            return getEdge(b).nextS;
        }
        else if (p == getEdge(b).en) {
            return getEdge(b).nextE;
        }

        return -1;
    }

    // previous edge
    inline int PrevAt(int p, int b) const
    {
        if (p == getEdge(b).st) {
            return getEdge(b).prevS;
        }
        else if (p == getEdge(b).en) {
            return getEdge(b).prevE;
        }

        return -1;
    }

    // same as NextAt, but the list is considered circular  
    inline int CycleNextAt(int p, int b) const
    {
        if (p == getEdge(b).st) {
            if (getEdge(b).nextS < 0) {
                return getPoint(p).incidentEdge[FIRST];
            }
            return getEdge(b).nextS;
        } else if (p == getEdge(b).en) {
            if (getEdge(b).nextE < 0) {
                return getPoint(p).incidentEdge[FIRST];
            }

            return getEdge(b).nextE;
        }

        return -1;
    }

    // same as PrevAt, but the list is considered circular  
    inline int CyclePrevAt(int p, int b) const
    {
        if (p == getEdge(b).st) {
            if (getEdge(b).prevS < 0) {
                return getPoint(p).incidentEdge[LAST];
            }
            return getEdge(b).prevS;
        } else if (p == getEdge(b).en) {
            if (getEdge(b).prevE < 0) {
                return getPoint(p).incidentEdge[LAST];
            }
            return getEdge(b).prevE;
        }

        return -1;
    }
    
    void ConnectStart(int p, int b);        // set the point p as the start of edge b
    void ConnectEnd(int p, int b);        // set the point p as the end of edge b
    void DisconnectStart(int b);        // disconnect edge b from its start point
    void DisconnectEnd(int b);        // disconnect edge b from its end point

    // reverses edge b (start <-> end)    
    void Inverse(int b);
    // calc bounding box and sets leftX,rightX,topY and bottomY to their values
    void CalcBBox(bool strict_degree = false);
    
    // debug function: plots the graph (mac only)
    void Plot(double ix, double iy, double ir, double mx, double my, bool doPoint,
              bool edgesNo, bool pointNo, bool doDir, char *fileName);

    // transforms a polygon in a "forme" structure, ie a set of contours, which can be holes (see ShapeUtils.h)
    // return NULL in case it's not possible
    void ConvertToForme(Path *dest);
    
    // version to use when conversion was done with ConvertWithBackData(): will attempt to merge segment belonging to 
    // the same curve
    // nota: apparently the function doesn't like very small segments of arc
    void ConvertToForme(Path *dest, int nbP, Path **orig, bool splitWhenForced = false);
    // version trying to recover the nesting of subpaths (ie: holes)
    void ConvertToFormeNested(Path *dest, int nbP, Path **orig, int wildPath, int &nbNest,
                              int *&nesting, int *&contStart, bool splitWhenForced = false);
  
    // sweeping a digraph to produce a intersection-free polygon
    // return 0 if everything is ok and a return code otherwise (see LivarotDefs.h)
    // the input is the Shape "a"
    // directed=true <=> non-zero fill rule    
    int ConvertToShape(Shape *a, FillRule directed = fill_nonZero, bool invert = false);
    // directed=false <=> even-odd fill rule
    // invert=true: make as if you inverted all edges in the source
    int Reoriente(Shape *a);        // subcase of ConvertToShape: the input a is already intersection-free
    // all that's missing are the correct directions of the edges
    // Reoriented is equivalent to ConvertToShape(a,false,false) , but faster sicne
    // it doesn't computes interections nor adjacencies
    void ForceToPolygon();        // force the Shape to believe it's a polygon (eulerian+intersection-free+no
    // duplicate edges+no duplicate points)
    // be careful when using this function

    // the coordinate rounding function
    inline static double Round(double x)
    {
        return ldexp(rint(ldexp(x, 9)), -9);
    }
    
    // 2 miscannellous variations on it, to scale to and back the rounding grid
    inline static double HalfRound(double x)
    {
        return ldexp(x, -9);
    }
    
    inline static double IHalfRound(double x)
    {
        return ldexp(x, 9);
    }

    // boolean operations on polygons (requests intersection-free poylygons)
    // boolean operation types are defined in LivarotDefs.h
    // same return code as ConvertToShape
    int Booleen(Shape *a, Shape *b, BooleanOp mod, int cutPathID = -1);

    // create a graph that is an offseted version of the graph "of"
    // the offset is dec, with joins between edges of type "join" (see LivarotDefs.h)
    // the result is NOT a polygon; you need a subsequent call to ConvertToShape to get a real polygon
    int MakeOffset(Shape *of, double dec, JoinType join, double miter, bool do_profile=false, double cx = 0, double cy = 0, double radius = 0, Geom::Affine *i2doc = NULL);

    int MakeTweak (int mode, Shape *a, double dec, JoinType join, double miter, bool do_profile, Geom::Point c, Geom::Point vector, double radius, Geom::Affine *i2doc);
  
    int PtWinding(const Geom::Point px) const; // plus rapide
    int Winding(const Geom::Point px) const;
  
    // rasterization
    void BeginRaster(float &pos, int &curPt);
    void EndRaster();
    void BeginQuickRaster(float &pos, int &curPt);
    void EndQuickRaster();
  
    void Scan(float &pos, int &curP, float to, float step);
    void QuickScan(float &pos, int &curP, float to, bool doSort, float step);
    void DirectScan(float &pos, int &curP, float to, float step);
    void DirectQuickScan(float &pos, int &curP, float to, bool doSort, float step);

    void Scan(float &pos, int &curP, float to, FloatLigne *line, bool exact, float step);
    void Scan(float &pos, int &curP, float to, FillRule directed, BitLigne *line, bool exact, float step);
    void Scan(float &pos, int &curP, float to, AlphaLigne *line, bool exact, float step);

    void QuickScan(float &pos, int &curP, float to, FloatLigne* line, float step);
    void QuickScan(float &pos, int &curP, float to, FillRule directed, BitLigne* line, float step);
    void QuickScan(float &pos, int &curP, float to, AlphaLigne* line, float step);

    void Transform(Geom::Affine const &tr)
        {for(std::vector<dg_point>::iterator it=_pts.begin();it!=_pts.end();++it) it->x*=tr;}

    std::vector<back_data> ebData;
    std::vector<voronoi_point> vorpData;
    std::vector<voronoi_edge> voreData;

    int nbQRas;
    int firstQRas;
    int lastQRas;
    quick_raster_data *qrsData;

    std::vector<sTreeChange> chgts;
    int nbInc;
    int maxInc;

    incidenceData *iData;
    // these ones are allocated at the beginning of each sweep and freed at the end of the sweep
    SweepTreeList *sTree;
    SweepEventQueue *sEvts;
    
    // bounding box stuff
    double leftX, topY, rightX, bottomY;

    // topological information: who links who?
    struct dg_point
    {
        Geom::Point x;                // position
        int dI, dO;                // indegree and outdegree
        int incidentEdge[2];    // first and last incident edge
        int oldDegree;

        int totalDegree() const { return dI + dO; }
    };
    
    struct dg_arete
    {
        Geom::Point dx;                // edge vector
        int st, en;                // start and end points of the edge
        int nextS, prevS;        // next and previous edge in the double-linked list at the start point
        int nextE, prevE;        // next and previous edge in the double-linked list at the end point
    };

    // lists of the nodes and edges
    int maxPt; // [FIXME: remove this]
    int maxAr; // [FIXME: remove this]
    
    // flags
    int type;
    
    inline int numberOfPoints() const { return _pts.size(); }
    inline bool hasPoints() const { return (_pts.empty() == false); }
    inline int numberOfEdges() const { return _aretes.size(); }
    inline bool hasEdges() const { return (_aretes.empty() == false); }

    inline void needPointsSorting() { _need_points_sorting = true; }
    inline void needEdgesSorting()  { _need_edges_sorting = true; }
    
    inline bool hasBackData() const { return _has_back_data; }
    
    inline dg_point const &getPoint(int n) const { return _pts[n]; }
    inline dg_arete const &getEdge(int n) const { return _aretes[n]; }

private:

    friend class SweepTree;
    friend class SweepEvent;
    friend class SweepEventQueue;
  
    // temporary data for the various algorithms
    struct edge_data
    {
        int weight;                        // weight of the edge (to handle multiple edges)
        Geom::Point rdx;                // rounded edge vector
        double length, sqlength, ilength, isqlength;        // length^2, length, 1/length^2, 1/length
        double siEd, coEd;                // siEd=abs(rdy/length) and coEd=rdx/length
        edge_data() : weight(0), length(0.0), sqlength(0.0), ilength(0.0), isqlength(0.0), siEd(0.0), coEd(0.0) {}
        // used to determine the "most horizontal" edge between 2 edges
    };
    
    struct sweep_src_data
    {
        void *misc;                        // pointer to the SweepTree* in the sweepline
        int firstLinkedPoint;        // not used
        int stPt, enPt;                // start- end end- points for this edge in the resulting polygon
        int ind;                        // for the GetAdjacencies function: index in the sliceSegs array (for quick deletions)
        int leftRnd, rightRnd;        // leftmost and rightmost points (in the result polygon) that are incident to
        // the edge, for the current sweep position
        // not set if the edge doesn't start/end or intersect at the current sweep position
        Shape *nextSh;                // nextSh and nextBo identify the next edge in the list
        int nextBo;                        // they are used to maintain a linked list of edge that start/end or intersect at
        // the current sweep position
        int curPoint, doneTo;
        double curT;
    };
    
    struct sweep_dest_data
    {
        void *misc;                        // used to check if an edge has already been seen during the depth-first search
        int suivParc, precParc;        // previous and current next edge in the depth-first search
        int leW, riW;                // left and right winding numbers for this edge
        int ind;                        // order of the edges during the depth-first search
    };
    
    struct raster_data
    {
        SweepTree *misc;                // pointer to the associated SweepTree* in the sweepline
        double lastX, lastY, curX, curY;        // curX;curY is the current intersection of the edge with the sweepline
        // lastX;lastY is the intersection with the previous sweepline
        bool sens;                        // true if the edge goes down, false otherwise
        double calcX;                // horizontal position of the intersection of the edge with the
        // previous sweepline
        double dxdy, dydx;                // horizontal change per unit vertical move of the intersection with the sweepline
        int guess;
    };
    
    struct point_data
    {
        int oldInd, newInd;                // back and forth indices used when sorting the points, to know where they have
        // been relocated in the array
        int pending;                // number of intersection attached to this edge, and also used when sorting arrays
        int edgeOnLeft;                // not used (should help speeding up winding calculations)
        int nextLinkedPoint;        // not used
        Shape *askForWindingS;
        int askForWindingB;
        Geom::Point  rx;                // rounded coordinates of the point
    };
    
    
    struct edge_list
    {                                // temporary array of edges for easier sorting
        int no;
        bool starting;
        Geom::Point x;
    };

    void initialisePointData();
    void initialiseEdgeData();
    void clearIncidenceData();

    void _countUpDown(int P, int *numberUp, int *numberDown, int *upEdge, int *downEdge) const;
    void _countUpDownTotalDegree2(int P, int *numberUp, int *numberDown, int *upEdge, int *downEdge) const;
    void _updateIntersection(int e, int p);
  
    // activation/deactivation of the temporary data arrays
    void MakePointData(bool nVal);
    void MakeEdgeData(bool nVal);
    void MakeSweepSrcData(bool nVal);
    void MakeSweepDestData(bool nVal);
    void MakeRasterData(bool nVal);
    void MakeQuickRasterData(bool nVal);

    void SortPoints(int s, int e);
    void SortPointsByOldInd(int s, int e);

    // fonctions annexes pour ConvertToShape et Booleen
    void ResetSweep();        // allocates sweep structures
    void CleanupSweep();        // deallocates them

    // edge sorting function    
    void SortEdgesList(edge_list *edges, int s, int e);
  
    void TesteIntersection(SweepTree *t, Side s, bool onlyDiff);        // test if there is an intersection
    bool TesteIntersection(SweepTree *iL, SweepTree *iR, Geom::Point &atx, double &atL, double &atR, bool onlyDiff);
    bool TesteIntersection(Shape *iL, Shape *iR, int ilb, int irb,
                           Geom::Point &atx, double &atL, double &atR,
                           bool onlyDiff);
    bool TesteAdjacency(Shape *iL, int ilb, const Geom::Point atx, int nPt,
                        bool push);
    int PushIncidence(Shape *a, int cb, int pt, double theta);
    int CreateIncidence(Shape *a, int cb, int pt);
    void AssemblePoints(Shape *a);
    int AssemblePoints(int st, int en);
    void AssembleAretes(FillRule directed = fill_nonZero);
    void AddChgt(int lastPointNo, int lastChgtPt, Shape *&shapeHead,
                 int &edgeHead, sTreeChangeType type, Shape *lS, int lB, Shape *rS,
                 int rB);
    void CheckAdjacencies(int lastPointNo, int lastChgtPt, Shape *shapeHead, int edgeHead);
    void CheckEdges(int lastPointNo, int lastChgtPt, Shape *a, Shape *b, BooleanOp mod);
    void Avance(int lastPointNo, int lastChgtPt, Shape *iS, int iB, Shape *a, Shape *b, BooleanOp mod);
    void DoEdgeTo(Shape *iS, int iB, int iTo, bool direct, bool sens);
    void GetWindings(Shape *a, Shape *b = NULL, BooleanOp mod = bool_op_union, bool brutal = false);

    void Validate();

    int Winding(int nPt) const;
    void SortPointsRounded();
    void SortPointsRounded(int s, int e);
    
    void CreateEdge(int no, float to, float step);
    void AvanceEdge(int no, float to, bool exact, float step);
    void DestroyEdge(int no, float to, FloatLigne *line);
    void AvanceEdge(int no, float to, FloatLigne *line, bool exact, float step);
    void DestroyEdge(int no, BitLigne *line);
    void AvanceEdge(int no, float to, BitLigne *line, bool exact, float step);
    void DestroyEdge(int no, AlphaLigne *line);
    void AvanceEdge(int no, float to, AlphaLigne *line, bool exact, float step);
  
    void AddContour(Path * dest, int nbP, Path **orig, int startBord,
                   int curBord, bool splitWhenForced);
    int ReFormeLineTo(int bord, int curBord, Path *dest, Path *orig);
    int ReFormeArcTo(int bord, int curBord, Path *dest, Path *orig);
    int ReFormeCubicTo(int bord, int curBord, Path *dest, Path *orig);
    int ReFormeBezierTo(int bord, int curBord, Path *dest, Path *orig);
    void ReFormeBezierChunk(const Geom::Point px, const Geom::Point nx,
                            Path *dest, int inBezier, int nbInterm,
                            Path *from, int p, double ts, double te);

    int QuickRasterChgEdge(int oBord, int nbord, double x);
    int QuickRasterAddEdge(int bord, double x, int guess);
    void QuickRasterSubEdge(int bord);
    void QuickRasterSwapEdge(int a, int b);
    void QuickRasterSort();

    bool _need_points_sorting;  ///< points have been added or removed: we need to sort the points again
    bool _need_edges_sorting;   ///< edges have been added: maybe they are not ordered clockwise
    ///< nota: if you remove an edge, the clockwise order still holds
    bool _has_points_data;      ///< the pData array is allocated
    bool _point_data_initialised;///< the pData array is up to date
    bool _has_edges_data;       ///< the eData array is allocated
    bool _has_sweep_src_data;   ///< the swsData array is allocated
    bool _has_sweep_dest_data;  ///< the swdData array is allocated
    bool _has_raster_data;      ///< the swrData array is allocated
    bool _has_quick_raster_data;///< the swrData array is allocated
    bool _has_back_data;        //< the ebData array is allocated
    bool _has_voronoi_data;
    bool _bbox_up_to_date;      ///< the leftX/rightX/topY/bottomY are up to date

    std::vector<dg_point> _pts;
    std::vector<dg_arete> _aretes;
  
    // the arrays of temporary data
    // these ones are dynamically kept at a length of maxPt or maxAr
    std::vector<edge_data> eData;
    std::vector<sweep_src_data> swsData;
    std::vector<sweep_dest_data> swdData;
    std::vector<raster_data> swrData;
    std::vector<point_data> pData;
    
    static int CmpQRs(const quick_raster_data &p1, const quick_raster_data &p2) {
        if ( fabs(p1.x - p2.x) < 0.00001 ) {
            return 0;
        }

        return ( ( p1.x < p2.x ) ? -1 : 1 );
    };

    // edge direction comparison function    
    static int CmpToVert(const Geom::Point ax, const Geom::Point bx, bool as, bool bs);
};

bool directedEulerian(Shape const *s);
double distance(Shape const *s, Geom::Point const &p);
bool distanceLessThanOrEqual(Shape const *s, Geom::Point const &p, double const max_l2);

#endif
