#include <cxxtest/TestSuite.h>

#include "display/curve.h"
#include <2geom/curves.h>
#include <2geom/path.h>
#include <2geom/pathvector.h>

class CurveTest : public CxxTest::TestSuite {
private:
    Geom::Path path1;
    Geom::Path path2;
    Geom::Path path3;
    Geom::Path path4;

public:
    CurveTest() : path4(Geom::Point(3,5)) // Just a moveto
    {
        // Closed path
        path1.append(Geom::LineSegment(Geom::Point(0,0),Geom::Point(1,0)));
        path1.append(Geom::LineSegment(Geom::Point(1,0),Geom::Point(1,1)));
        path1.close();
        // Closed path (ClosingSegment is zero length)
        path2.append(Geom::LineSegment(Geom::Point(2,0),Geom::Point(3,0)));
        path2.append(Geom::CubicBezier(Geom::Point(3,0),Geom::Point(2,1),Geom::Point(1,1),Geom::Point(2,0)));
        path2.close();
        // Open path
        path3.setStitching(true);
        path3.append(Geom::EllipticalArc(Geom::Point(4,0),1,2,M_PI,false,false,Geom::Point(5,1)));
        path3.append(Geom::LineSegment(Geom::Point(5,1),Geom::Point(5,2)));
        path3.append(Geom::LineSegment(Geom::Point(6,4),Geom::Point(2,4)));
    }
    virtual ~CurveTest() {}

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static CurveTest *createSuite() { return new CurveTest(); }
    static void destroySuite( CurveTest *suite ) { delete suite; }

    void testGetSegmentCount()
    {
        { // Zero segments
            Geom::PathVector pv;
            SPCurve curve(pv);
            TS_ASSERT_EQUALS(curve.get_segment_count() , 0u);
        }
        { // Zero segments
            Geom::PathVector pv;
            pv.push_back(Geom::Path());
            SPCurve curve(pv);
            TS_ASSERT_EQUALS(curve.get_segment_count() , 0u);
        }
        { // Individual paths
            Geom::PathVector pv((Geom::Path()));
            pv[0] = path1;
            TS_ASSERT_EQUALS(SPCurve(pv).get_segment_count() , 3u);
            pv[0] = path2;
            TS_ASSERT_EQUALS(SPCurve(pv).get_segment_count() , 2u);
            pv[0] = path3;
            TS_ASSERT_EQUALS(SPCurve(pv).get_segment_count() , 4u);
            pv[0] = path4;
            TS_ASSERT_EQUALS(SPCurve(pv).get_segment_count() , 0u);
            pv[0].close();
            TS_ASSERT_EQUALS(SPCurve(pv).get_segment_count() , 0u);
        }
        { // Combination
            Geom::PathVector pv;
            pv.push_back(path1);
            pv.push_back(path2);
            pv.push_back(path3);
            pv.push_back(path4);
            SPCurve curve(pv);
            TS_ASSERT_EQUALS(curve.get_segment_count() , 9u);
        }
    }

    void testNodesInPath()
    {
        { // Zero segments
            Geom::PathVector pv;
            SPCurve curve(pv);
            TS_ASSERT_EQUALS(curve.nodes_in_path() , 0u);
        }
        { // Zero segments
            Geom::PathVector pv;
            pv.push_back(Geom::Path());
            SPCurve curve(pv);
            TS_ASSERT_EQUALS(curve.nodes_in_path() , 1u);
        }
        { // Individual paths
            Geom::PathVector pv((Geom::Path()));
            pv[0] = path1;
            TS_ASSERT_EQUALS(SPCurve(pv).nodes_in_path() , 3u);
            pv[0] = path2;
            TS_ASSERT_EQUALS(SPCurve(pv).nodes_in_path() , 2u); // zero length closing segments do not increase the nodecount.
            pv[0] = path3;
            TS_ASSERT_EQUALS(SPCurve(pv).nodes_in_path() , 5u);
            pv[0] = path4;
            TS_ASSERT_EQUALS(SPCurve(pv).nodes_in_path() , 1u);
            pv[0].close();
            TS_ASSERT_EQUALS(SPCurve(pv).nodes_in_path() , 1u);
        }
        { // Combination
            Geom::PathVector pv;
            pv.push_back(path1);
            pv.push_back(path2);
            pv.push_back(path3);
            pv.push_back(path4);
            SPCurve curve(pv);
            TS_ASSERT_EQUALS(curve.nodes_in_path() , 12u);
        }
    }

    void testIsEmpty()
    {
        TS_ASSERT(SPCurve(Geom::PathVector()).is_empty());
        TS_ASSERT(!SPCurve(path1).is_empty());
        TS_ASSERT(!SPCurve(path2).is_empty());
        TS_ASSERT(!SPCurve(path3).is_empty());
        TS_ASSERT(!SPCurve(path4).is_empty());
    }

    void testIsClosed()
    {
        TS_ASSERT(!SPCurve(Geom::PathVector()).is_closed());
        Geom::PathVector pv((Geom::Path()));
        TS_ASSERT(!SPCurve(pv).is_closed());
        pv[0].close();
        TS_ASSERT(SPCurve(pv).is_closed());
        TS_ASSERT(SPCurve(path1).is_closed());
        TS_ASSERT(SPCurve(path2).is_closed());
        TS_ASSERT(!SPCurve(path3).is_closed());
        TS_ASSERT(!SPCurve(path4).is_closed());
    }

    void testLastFirstSegment()
    {
        Geom::PathVector pv(path4);
        TS_ASSERT_EQUALS(SPCurve(pv).first_segment() , (void*)0);
        TS_ASSERT_EQUALS(SPCurve(pv).last_segment()  , (void*)0);
        pv[0].close();
        TS_ASSERT_DIFFERS(SPCurve(pv).first_segment() , (void*)0);
        TS_ASSERT_DIFFERS(SPCurve(pv).last_segment()  , (void*)0);
        /* Geom::Curve can't be compared very easily (?)
        Geom::PathVector pv(1, path4);
        pv[0].close();
        TS_ASSERT_EQUALS(*SPCurve(pv).first_segment() , pv[0][0]);
        TS_ASSERT_EQUALS(*SPCurve(pv).last_segment()  , pv[0][0]);
        pv[0] = path1;
        TS_ASSERT_EQUALS(*SPCurve(pv).first_segment() , pv[0][0]);
        TS_ASSERT_EQUALS(*SPCurve(pv).last_segment()  , pv[0][2]);
        pv[0] = path2;
        TS_ASSERT_EQUALS(*SPCurve(pv).first_segment() , pv[0][0]);
        TS_ASSERT_EQUALS(*SPCurve(pv).last_segment()  , pv[0][1]);
        pv[0] = path3;
        TS_ASSERT_EQUALS(*SPCurve(pv).first_segment() , pv[0][0]);
        TS_ASSERT_EQUALS(*SPCurve(pv).last_segment()  , pv[0][3]);
        pv[0] = path4;
        TS_ASSERT_EQUALS(SPCurve(pv).first_segment() , (void*)0);
        TS_ASSERT_EQUALS(SPCurve(pv).last_segment()  , (void*)0);
        pv.clear();
        pv.push_back(path1);
        pv.push_back(path2);
        pv.push_back(path3);
        TS_ASSERT_EQUALS(*SPCurve(pv).first_segment() , pv[0][0]);
        TS_ASSERT_EQUALS(*SPCurve(pv).last_segment()  , pv[2][3]);*/
    }

    void testLastFirstPath()
    {
        Geom::PathVector pv;
        TS_ASSERT_EQUALS(SPCurve(pv).first_path() , (void*)0);
        TS_ASSERT_EQUALS(SPCurve(pv).last_path()  , (void*)0);
        pv.push_back(path1);
        TS_ASSERT_EQUALS(*SPCurve(pv).first_path() , pv[0]);
        TS_ASSERT_EQUALS(*SPCurve(pv).last_path()  , pv[0]);
        pv.push_back(path2);
        TS_ASSERT_EQUALS(*SPCurve(pv).first_path() , pv[0]);
        TS_ASSERT_EQUALS(*SPCurve(pv).last_path()  , pv[1]);
        pv.push_back(path3);
        TS_ASSERT_EQUALS(*SPCurve(pv).first_path() , pv[0]);
        TS_ASSERT_EQUALS(*SPCurve(pv).last_path()  , pv[2]);
        pv.push_back(path4);
        TS_ASSERT_EQUALS(*SPCurve(pv).first_path() , pv[0]);
        TS_ASSERT_EQUALS(*SPCurve(pv).last_path()  , pv[3]);
    }

    void testFirstPoint()
    {
        TS_ASSERT_EQUALS(*(SPCurve(path1).first_point()) , Geom::Point(0,0));
        TS_ASSERT_EQUALS(*(SPCurve(path2).first_point()) , Geom::Point(2,0));
        TS_ASSERT_EQUALS(*(SPCurve(path3).first_point()) , Geom::Point(4,0));
        TS_ASSERT_EQUALS(*(SPCurve(path4).first_point()) , Geom::Point(3,5));
        Geom::PathVector pv;
        TS_ASSERT(!SPCurve(pv).first_point());
        pv.push_back(path1);
        pv.push_back(path2);
        pv.push_back(path3);
        TS_ASSERT_EQUALS(*(SPCurve(pv).first_point()) , Geom::Point(0,0));
        pv.insert(pv.begin(), path4);
        TS_ASSERT_EQUALS(*(SPCurve(pv).first_point()) , Geom::Point(3,5));
    }

    void testLastPoint()
    {
        TS_ASSERT_EQUALS(*(SPCurve(path1).last_point()) , Geom::Point(0,0));
        TS_ASSERT_EQUALS(*(SPCurve(path2).last_point()) , Geom::Point(2,0));
        TS_ASSERT_EQUALS(*(SPCurve(path3).last_point()) , Geom::Point(8,4));
        TS_ASSERT_EQUALS(*(SPCurve(path4).last_point()) , Geom::Point(3,5));
        Geom::PathVector pv;
        TS_ASSERT(!SPCurve(pv).last_point());
        pv.push_back(path1);
        pv.push_back(path2);
        pv.push_back(path3);
        TS_ASSERT_EQUALS(*(SPCurve(pv).last_point()) , Geom::Point(8,4));
        pv.push_back(path4);
        TS_ASSERT_EQUALS(*(SPCurve(pv).last_point()) , Geom::Point(3,5));
    }

    void testSecondPoint()
    {
        TS_ASSERT_EQUALS( *(SPCurve(path1).second_point()) , Geom::Point(1,0));
        TS_ASSERT_EQUALS( *(SPCurve(path2).second_point()) , Geom::Point(3,0));
        TS_ASSERT_EQUALS( *(SPCurve(path3).second_point()) , Geom::Point(5,1));
        TS_ASSERT_EQUALS( *(SPCurve(path4).second_point()) , Geom::Point(3,5));
        Geom::PathVector pv;
        pv.push_back(path1);
        pv.push_back(path2);
        pv.push_back(path3);
        TS_ASSERT_EQUALS( *(SPCurve(pv).second_point()) , Geom::Point(1,0));
        pv.insert(pv.begin(), path4);
        TS_ASSERT_EQUALS( *SPCurve(pv).second_point(), Geom::Point(0,0) );
    }

    void testPenultimatePoint()
    {
        TS_ASSERT_EQUALS( *(SPCurve(Geom::PathVector(path1)).penultimate_point()) , Geom::Point(1,1));
        TS_ASSERT_EQUALS( *(SPCurve(Geom::PathVector(path2)).penultimate_point()) , Geom::Point(3,0));
        TS_ASSERT_EQUALS( *(SPCurve(Geom::PathVector(path3)).penultimate_point()) , Geom::Point(6,4));
        TS_ASSERT_EQUALS( *(SPCurve(Geom::PathVector(path4)).penultimate_point()) , Geom::Point(3,5));
        Geom::PathVector pv;
        pv.push_back(path1);
        pv.push_back(path2);
        pv.push_back(path3);
        TS_ASSERT_EQUALS( *(SPCurve(pv).penultimate_point()) , Geom::Point(6,4));
        pv.push_back(path4);
        TS_ASSERT_EQUALS( *(SPCurve(pv).penultimate_point()) , Geom::Point(8,4));
    }

    // TODO: Rest of the methods
};

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
