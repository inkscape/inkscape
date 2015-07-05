#include <cxxtest/TestSuite.h>
#include "2geom/coord.h"
#include "2geom/curves.h"
#include "2geom/pathvector.h"
#include "svg/svg.h"
#include "preferences.h"
#include "streq.h"
#include <stdio.h>
#include <string>
#include <vector>
#include <glib.h>

class SvgPathGeomTest : public CxxTest::TestSuite
{
private:
    std::vector<std::string> rectanglesAbsoluteClosed;
    std::vector<std::string> rectanglesRelativeClosed;
    std::vector<std::string> rectanglesAbsoluteOpen;
    std::vector<std::string> rectanglesRelativeOpen;
    std::vector<std::string> rectanglesAbsoluteClosed2;
    std::vector<std::string> rectanglesRelativeClosed2;
    Geom::PathVector rectanglepvopen;
    Geom::PathVector rectanglepvclosed;
    Geom::PathVector rectanglepvclosed2;
public:
    SvgPathGeomTest() {
        // Lots of ways to define the same rectangle
        rectanglesAbsoluteClosed.push_back("M 1,2 L 4,2 L 4,8 L 1,8 z");
        rectanglesAbsoluteClosed.push_back("M 1,2 4,2 4,8 1,8 z");
        rectanglesAbsoluteClosed.push_back("M 1,2 H 4 V 8 H 1 z");
        rectanglesRelativeClosed.push_back("m 1,2 l 3,0 l 0,6 l -3,0 z");
        rectanglesRelativeClosed.push_back("m 1,2 3,0 0,6 -3,0 z");
        rectanglesRelativeClosed.push_back("m 1,2 h 3 v 6 h -3 z");
        rectanglesAbsoluteOpen.push_back("M 1,2 L 4,2 L 4,8 L 1,8 L 1,2");
        rectanglesAbsoluteOpen.push_back("M 1,2 4,2 4,8 1,8 1,2");
        rectanglesAbsoluteOpen.push_back("M 1,2 H 4 V 8 H 1 V 2");
        rectanglesRelativeOpen.push_back("m 1,2 l 3,0 l 0,6 l -3,0 l 0,-6");
        rectanglesRelativeOpen.push_back("m 1,2 3,0 0,6 -3,0 0,-6");
        rectanglesRelativeOpen.push_back("m 1,2 h 3 v 6 h -3 v -6");
        rectanglesAbsoluteClosed2.push_back("M 1,2 L 4,2 L 4,8 L 1,8 L 1,2 z");
        rectanglesAbsoluteClosed2.push_back("M 1,2 4,2 4,8 1,8 1,2 z");
        rectanglesAbsoluteClosed2.push_back("M 1,2 H 4 V 8 H 1 V 2 z");
        rectanglesRelativeClosed2.push_back("m 1,2 l 3,0 l 0,6 l -3,0 l 0,-6 z");
        rectanglesRelativeClosed2.push_back("m 1,2 3,0 0,6 -3,0 0,-6 z");
        rectanglesRelativeClosed2.push_back("m 1,2 h 3 v 6 h -3 v -6 z");
        rectanglepvopen.push_back(Geom::Path(Geom::Point(1,2)));
        rectanglepvopen.back().append(Geom::LineSegment(Geom::Point(1,2),Geom::Point(4,2)));
        rectanglepvopen.back().append(Geom::LineSegment(Geom::Point(4,2),Geom::Point(4,8)));
        rectanglepvopen.back().append(Geom::LineSegment(Geom::Point(4,8),Geom::Point(1,8)));
        rectanglepvopen.back().append(Geom::LineSegment(Geom::Point(1,8),Geom::Point(1,2)));
        rectanglepvclosed.push_back(Geom::Path(Geom::Point(1,2)));
        rectanglepvclosed.back().append(Geom::LineSegment(Geom::Point(1,2),Geom::Point(4,2)));
        rectanglepvclosed.back().append(Geom::LineSegment(Geom::Point(4,2),Geom::Point(4,8)));
        rectanglepvclosed.back().append(Geom::LineSegment(Geom::Point(4,8),Geom::Point(1,8)));
        rectanglepvclosed.back().close();
        rectanglepvclosed2.push_back(Geom::Path(Geom::Point(1,2)));
        rectanglepvclosed2.back().append(Geom::LineSegment(Geom::Point(1,2),Geom::Point(4,2)));
        rectanglepvclosed2.back().append(Geom::LineSegment(Geom::Point(4,2),Geom::Point(4,8)));
        rectanglepvclosed2.back().append(Geom::LineSegment(Geom::Point(4,8),Geom::Point(1,8)));
        rectanglepvclosed2.back().append(Geom::LineSegment(Geom::Point(1,8),Geom::Point(1,2)));
        rectanglepvclosed2.back().close();
        // TODO: Also test some (smooth) cubic/quadratic beziers and elliptical arcs
        // TODO: Should we make it mandatory that h/v in the path data results in a H/VLineSegment?
        //       If so, the tests should be modified to reflect this.
    }

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static SvgPathGeomTest *createSuite() { return new SvgPathGeomTest(); }
    static void destroySuite( SvgPathGeomTest *suite ) { delete suite; }

    void testReadRectanglesAbsoluteClosed()
    {
        for(size_t i=0; i<rectanglesAbsoluteClosed.size(); i++) {
            Geom::PathVector pv = sp_svg_read_pathv(rectanglesAbsoluteClosed[i].c_str());
            TSM_ASSERT(rectanglesAbsoluteClosed[i].c_str(), bpathEqual(pv,rectanglepvclosed));
        }
    }

    void testReadRectanglesRelativeClosed()
    {
        for(size_t i=0; i<rectanglesRelativeClosed.size(); i++) {
            Geom::PathVector pv = sp_svg_read_pathv(rectanglesRelativeClosed[i].c_str());
            TSM_ASSERT(rectanglesRelativeClosed[i].c_str(), bpathEqual(pv,rectanglepvclosed));
        }
    }

    void testReadRectanglesAbsoluteOpen()
    {
        for(size_t i=0; i<rectanglesAbsoluteOpen.size(); i++) {
            Geom::PathVector pv = sp_svg_read_pathv(rectanglesAbsoluteOpen[i].c_str());
            TSM_ASSERT(rectanglesAbsoluteOpen[i].c_str(), bpathEqual(pv,rectanglepvopen));
        }
    }

    void testReadRectanglesRelativeOpen()
    {
        for(size_t i=0; i<rectanglesRelativeOpen.size(); i++) {
            Geom::PathVector pv = sp_svg_read_pathv(rectanglesRelativeOpen[i].c_str());
            TSM_ASSERT(rectanglesRelativeOpen[i].c_str(), bpathEqual(pv,rectanglepvopen));
        }
    }

    void testReadRectanglesAbsoluteClosed2()
    {
        for(size_t i=0; i<rectanglesAbsoluteClosed2.size(); i++) {
            Geom::PathVector pv = sp_svg_read_pathv(rectanglesAbsoluteClosed2[i].c_str());
            TSM_ASSERT(rectanglesAbsoluteClosed2[i].c_str(), bpathEqual(pv,rectanglepvclosed2));
        }
    }

    void testReadRectanglesRelativeClosed2()
    {
        for(size_t i=0; i<rectanglesRelativeClosed2.size(); i++) {
            Geom::PathVector pv = sp_svg_read_pathv(rectanglesRelativeClosed2[i].c_str());
            TSM_ASSERT(rectanglesRelativeClosed2[i].c_str(), bpathEqual(pv,rectanglepvclosed2));
        }
    }

    void testReadConcatenatedPaths()
    {
        // Note that finalPoint doesn't actually return the final point of the path, just the last given point... (but since this might be intentional and we're not testing lib2geom here, we just specify the final point explicitly
        Geom::PathVector pv_good;
        pv_good.push_back(rectanglepvclosed.back());
        pv_good.push_back(rectanglepvopen.back() * Geom::Translate(1,2)/* * Geom::Translate(pv_good[0].finalPoint())*/);
        pv_good.push_back(rectanglepvclosed.back() * Geom::Translate(2,4)/* *Geom::Translate(pv_good[1].finalPoint())*/);
        pv_good.push_back(rectanglepvopen.back());
        pv_good[0].close();
        pv_good[1].close(false);
        pv_good[2].close();
        pv_good[3].close(false);
        std::string path_str = rectanglesAbsoluteClosed[0] + rectanglesRelativeOpen[0] + rectanglesRelativeClosed[0] + rectanglesAbsoluteOpen[0];
        Geom::PathVector pv = sp_svg_read_pathv(path_str.c_str());
        TS_ASSERT(bpathEqual(pv,pv_good));
    }

    void testReadZeroLengthSubpaths() {
        // Per the SVG 1.1 specification (section F5) zero-length subpaths are relevant
        Geom::PathVector pv_good;
        pv_good.push_back(Geom::Path(Geom::Point(0,0)));
        pv_good.push_back(Geom::Path(Geom::Point(1,1)));
        pv_good.back().append(Geom::LineSegment(Geom::Point(1,1),Geom::Point(2,2)));
        pv_good.push_back(Geom::Path(Geom::Point(3,3)));
        pv_good.back().close();
        pv_good.push_back(Geom::Path(Geom::Point(4,4)));
        pv_good.back().append(Geom::LineSegment(Geom::Point(4,4),Geom::Point(5,5)));
        pv_good.back().close();
        pv_good.push_back(Geom::Path(Geom::Point(6,6)));
        {   // Test absolute version
            char const * path_str = "M 0,0 M 1,1 L 2,2 M 3,3 z M 4,4 L 5,5 z M 6,6";
            Geom::PathVector pv = sp_svg_read_pathv(path_str);
            TSM_ASSERT(path_str, bpathEqual(pv,pv_good));
        }
        {   // Test relative version
            char const * path_str = "m 0,0 m 1,1 l 1,1 m 1,1 z m 1,1 l 1,1 z m 2,2";
            Geom::PathVector pv = sp_svg_read_pathv(path_str);
            TSM_ASSERT(path_str, bpathEqual(pv,pv_good));
        }
    }

    void testReadImplicitMoveto() {
        TS_WARN("Currently lib2geom (/libnr) has no way of specifying the difference between 'M 0,0 ... z M 0,0 L 1,0' and 'M 0,0 ... z L 1,0', the SVG specification does state that these should be handled differently with respect to markers however, see the description of the 'orient' attribute of the 'marker' element.");
        Geom::PathVector pv_good;
        pv_good.push_back(Geom::Path(Geom::Point(1,1)));
        pv_good.back().append(Geom::LineSegment(Geom::Point(1,1),Geom::Point(2,2)));
        pv_good.back().close();
        pv_good.push_back(Geom::Path(Geom::Point(1,1)));
        pv_good.back().append(Geom::LineSegment(Geom::Point(1,1),Geom::Point(3,3)));
        pv_good.back().close();
        {   // Test absolute version
            char const * path_str = "M 1,1 L 2,2 z L 3,3 z";
            Geom::PathVector pv = sp_svg_read_pathv(path_str);
            TSM_ASSERT(path_str, bpathEqual(pv,pv_good));
        }
        {   // Test relative version
            char const * path_str = "M 1,1 l 1,1 z l 2,2 z";
            Geom::PathVector pv = sp_svg_read_pathv(path_str);
            TSM_ASSERT(path_str, bpathEqual(pv,pv_good));
        }
    }

    void testReadFloatingPoint() {
        Geom::PathVector pv_good1;
        pv_good1.push_back(Geom::Path(Geom::Point(.01,.02)));
        pv_good1.back().append(Geom::LineSegment(Geom::Point(.01,.02),Geom::Point(.04,.02)));
        pv_good1.back().append(Geom::LineSegment(Geom::Point(.04,.02),Geom::Point(1.5,1.6)));
        pv_good1.back().append(Geom::LineSegment(Geom::Point(1.5,1.6),Geom::Point(.01,.08)));
        pv_good1.back().append(Geom::LineSegment(Geom::Point(.01,.08),Geom::Point(.01,.02)));
        pv_good1.back().close();
        {   // Test decimals
            char const * path_str = "M .01,.02 L.04.02 L1.5,1.6L0.01,0.08 .01.02 z";
            Geom::PathVector pv = sp_svg_read_pathv(path_str);
            TSM_ASSERT(path_str, bpathEqual(pv,pv_good1));
        }
        Geom::PathVector pv_good2;
        pv_good2.push_back(Geom::Path(Geom::Point(.01,.02)));
        pv_good2.back().append(Geom::LineSegment(Geom::Point(.01,.02),Geom::Point(.04,.02)));
        pv_good2.back().append(Geom::LineSegment(Geom::Point(.04,.02),Geom::Point(1.5,1.6)));
        pv_good2.back().append(Geom::LineSegment(Geom::Point(1.5,1.6),Geom::Point(.01,.08)));
        pv_good2.back().close();
        {   // Test exponent
            char const * path_str = "M 1e-2,.2e-1 L 0.004e1,0.0002e+2 L0150E-2,1.6e0L1.0e-2,80e-3 z";
            Geom::PathVector pv = sp_svg_read_pathv(path_str);
            TSM_ASSERT(path_str, bpathEqual(pv,pv_good2));
        }
    }

    void testReadImplicitSeparation() {
        // Coordinates need not be separated by whitespace if they can still be read unambiguously
        Geom::PathVector pv_good;
        pv_good.push_back(Geom::Path(Geom::Point(.1,.2)));
        pv_good.back().append(Geom::LineSegment(Geom::Point(.1,.2),Geom::Point(.4,.2)));
        pv_good.back().append(Geom::LineSegment(Geom::Point(.4,.2),Geom::Point(.4,.8)));
        pv_good.back().append(Geom::LineSegment(Geom::Point(.4,.8),Geom::Point(.1,.8)));
        pv_good.back().close();
        {   // Test absolute
            char const * path_str = "M .1.2+0.4.2e0.4e0+8e-1.1.8 z";
            Geom::PathVector pv = sp_svg_read_pathv(path_str);
            TSM_ASSERT(path_str, bpathEqual(pv,pv_good));
        }
        {   // Test relative
            char const * path_str = "m .1.2+0.3.0e0.0e0+6e-1-.3.0 z";
            Geom::PathVector pv = sp_svg_read_pathv(path_str);
            TSM_ASSERT(path_str, bpathEqual(pv,pv_good));
        }
    }

    void testReadErrorMisplacedCharacter() {
        char const * path_str;
        Geom::PathVector pv;
        // Comma in the wrong place (commas may only appear between parameters)
        path_str = "M 1,2 4,2 4,8 1,8 z , m 13,15";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));
        // Comma in the wrong place (commas may only appear between parameters)
        path_str = "M 1,2 4,2 4,8 1,8 z m,13,15";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));
        // Period in the wrong place (no numbers after a 'z')
        path_str = "M 1,2 4,2 4,8 1,8 z . m 13,15";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));
        // Sign in the wrong place (no numbers after a 'z')
        path_str = "M 1,2 4,2 4,8 1,8 z + - m 13,15";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));
        // Digit in the wrong place (no numbers after a 'z')
        path_str = "M 1,2 4,2 4,8 1,8 z 9809 m 13,15";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));
        // Digit in the wrong place (no numbers after a 'z')
        path_str = "M 1,2 4,2 4,8 1,8 z 9809 876 m 13,15";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));
    }

    void testReadErrorUnrecognizedCharacter() {
        char const * path_str;
        Geom::PathVector pv;
        // Unrecognized character
        path_str = "M 1,2 4,2 4,8 1,8 z&m 13,15";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));
        // Unrecognized character
        path_str = "M 1,2 4,2 4,8 1,8 z m &13,15";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));
    }

    void testReadErrorTypo() {
        char const * path_str;
        Geom::PathVector pv;
        // Typo
        path_str = "M 1,2 4,2 4,8 1,8 z j 13,15";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));

        // Typo
        path_str = "M 1,2 4,2 4,8 1,8 L 1,2 x m 13,15";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvopen));
    }

    void testReadErrorIllformedNumbers() {
        char const * path_str;
        Geom::PathVector pv;
        // Double exponent
        path_str = "M 1,2 4,2 4,8 1,8 z m 13e4e5,15";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));
        // Double sign
        path_str = "M 1,2 4,2 4,8 1,8 z m +-13,15";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));
        // Double sign
        path_str = "M 1,2 4,2 4,8 1,8 z m 13e+-12,15";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));
        // No digit
        path_str = "M 1,2 4,2 4,8 1,8 z m .e12,15";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));
        // No digit
        path_str = "M 1,2 4,2 4,8 1,8 z m .,15";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));
        // No digit
        path_str = "M 1,2 4,2 4,8 1,8 z m +,15";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));
        // No digit
        path_str = "M 1,2 4,2 4,8 1,8 z m +.e+,15";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));
    }

    void testReadErrorJunk() {
        char const * path_str;
        Geom::PathVector pv;
        // Junk
        path_str = "M 1,2 4,2 4,8 1,8 z j 357 hkjh.,34e34 90ih6kj4 h5k6vlh4N.,6,45wikuyi3yere..3487 m 13,23";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));
    }

    void testReadErrorStopReading() {
        char const * path_str;
        Geom::PathVector pv;
        // Unrecognized parameter
        path_str = "M 1,2 4,2 4,8 1,8 z m #$%,23,34";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));
        // Invalid parameter
        path_str = "M 1,2 4,2 4,8 1,8 z m #$%,23,34";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));
        // Illformed parameter
        path_str = "M 1,2 4,2 4,8 1,8 z m +-12,23,34";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvclosed));

        // "Third" parameter
        path_str = "M 1,2 4,2 4,8 1,8 1,2,3 M 12,23";
        pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(path_str, bpathEqual(pv,rectanglepvopen));
    }

    void testRoundTrip() {
        // This is the easiest way to (also) test writing path data, as a path can be written in more than one way.
        Geom::PathVector pv;
        Geom::PathVector new_pv;
        std::string org_path_str;
        char * path_str;
        // Rectangle (closed)
        org_path_str = rectanglesAbsoluteClosed[0];
        pv = sp_svg_read_pathv(org_path_str.c_str());
        path_str = sp_svg_write_path(pv);
        new_pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(org_path_str.c_str(), bpathEqual(pv,new_pv));
        g_free(path_str);
        // Rectangle (open)
        org_path_str = rectanglesAbsoluteOpen[0];
        pv = sp_svg_read_pathv(org_path_str.c_str());
        path_str = sp_svg_write_path(pv);
        new_pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(org_path_str.c_str(), bpathEqual(pv,new_pv));
        g_free(path_str);
        // Concatenated rectangles
        org_path_str = rectanglesAbsoluteClosed[0] + rectanglesRelativeOpen[0] + rectanglesRelativeClosed[0] + rectanglesAbsoluteOpen[0];
        pv = sp_svg_read_pathv(org_path_str.c_str());
        path_str = sp_svg_write_path(pv);
        new_pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(org_path_str.c_str(), bpathEqual(pv,new_pv));
        g_free(path_str);
        // Zero-length subpaths
        org_path_str = "M 0,0 M 1,1 L 2,2 M 3,3 z M 4,4 L 5,5 z M 6,6";
        pv = sp_svg_read_pathv(org_path_str.c_str());
        path_str = sp_svg_write_path(pv);
        new_pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(org_path_str.c_str(), bpathEqual(pv,new_pv));
        g_free(path_str);
        // Floating-point
        org_path_str = "M .01,.02 L 0.04,0.02 L.04,.08L0.01,0.08 z""M 1e-2,.2e-1 L 0.004e1,0.0002e+2 L04E-2,.08e0L1.0e-2,80e-3 z";
        pv = sp_svg_read_pathv(org_path_str.c_str());
        path_str = sp_svg_write_path(pv);
        new_pv = sp_svg_read_pathv(path_str);
        TSM_ASSERT(org_path_str.c_str(), bpathEqual(pv, new_pv, 1e-17));
        g_free(path_str);
    }

    void testMinexpPrecision() {
        Geom::PathVector pv;
        char * path_str;
        // Default values
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setBool("/options/svgoutput/allowrelativecoordinates", true);
        prefs->setBool("/options/svgoutput/forcerepeatcommands", false);
        prefs->setInt("/options/svgoutput/numericprecision", 8);
        prefs->setInt("/options/svgoutput/minimumexponent", -8);
        pv = sp_svg_read_pathv("M 123456781,1.23456781e-8 L 123456782,1.23456782e-8 L 123456785,1.23456785e-8 L 10123456400,1.23456785e-8 L 123456789,1.23456789e-8 L 123456789,101.234564e-8 L 123456789,1.23456789e-8");
        path_str = sp_svg_write_path(pv);
        TS_ASSERT_RELATION( streq_rel , "m 123456780,1.2345678e-8 0,0 10,1e-15 9999999210,0 -9999999210,0 0,9.99999921e-7 0,-9.99999921e-7" , path_str );
        g_free(path_str);
    }

private:
    bool bpathEqual(Geom::PathVector const &a, Geom::PathVector const &b, double eps = 1e-16) {
        if (a.size() != b.size()) {
            char temp[100];
            sprintf(temp, "PathVectors not the same size: %u != %u", static_cast<unsigned int>(a.size()),static_cast<unsigned int>( b.size()));
            TS_FAIL(temp);
            return false;
        }
        for(size_t i=0; i<a.size(); i++) {
            Geom::Path const &pa = a[i];
            Geom::Path const &pb = b[i];
            if (pa.closed() && !pb.closed()) {
                char temp[100];
                sprintf(temp, "Left subpath is closed, right subpath is open. Subpath: %u", static_cast<unsigned int>(i));
                TS_FAIL(temp);
                return false;
            }
            if (!pa.closed() && pb.closed()) {
                char temp[100];
                sprintf(temp, "Right subpath is closed, left subpath is open. Subpath: %u", static_cast<unsigned int>(i));
                TS_FAIL(temp);
                return false;
            }
            if (pa.size() != pb.size()) {
                char temp[100];
                sprintf(temp, "Not the same number of segments: %u != %u, subpath: %u", static_cast<unsigned int>(pa.size()), static_cast<unsigned int>(pb.size()), static_cast<unsigned int>(i));
                TS_FAIL(temp);
                return false;
            }
            for(size_t j=0; j<pa.size(); j++) {
                Geom::Curve const* ca = &pa[j];
                Geom::Curve const* cb = &pb[j];
                if (typeid(*ca) == typeid(*cb))
                {
                    if(Geom::LineSegment const *la = dynamic_cast<Geom::LineSegment const*>(ca))
                    {
                        Geom::LineSegment const *lb = dynamic_cast<Geom::LineSegment const*>(cb);
                        if (!Geom::are_near((*la)[0],(*lb)[0], eps)) {
                            char temp[200];
                            sprintf(temp, "Different start of segment: (%g,%g) != (%g,%g), subpath: %u, segment: %u", (*la)[0][Geom::X], (*la)[0][Geom::Y], (*lb)[0][Geom::X], (*lb)[0][Geom::Y], static_cast<unsigned int>(i), static_cast<unsigned int>(j));
                            TS_FAIL(temp);
                            return false;
                        }
                        if (!Geom::are_near((*la)[1],(*lb)[1], eps)) {
                            char temp[200];
                            sprintf(temp, "Different end of segment: (%g,%g) != (%g,%g), subpath: %u, segment: %u", (*la)[1][Geom::X], (*la)[1][Geom::Y], (*lb)[1][Geom::X], (*lb)[1][Geom::Y], static_cast<unsigned int>(i), static_cast<unsigned int>(j));
                            TS_FAIL(temp);
                            return false;
                        }
                    }
                    else if(Geom::CubicBezier const *la = dynamic_cast<Geom::CubicBezier const*>(ca))
                    {
                        Geom::CubicBezier const *lb = dynamic_cast<Geom::CubicBezier const*>(cb);
                        if (!Geom::are_near((*la)[0],(*lb)[0], eps)) {
                            char temp[200];
                            sprintf(temp, "Different start of segment: (%g,%g) != (%g,%g), subpath: %u, segment: %u", (*la)[0][Geom::X], (*la)[0][Geom::Y], (*lb)[0][Geom::X], (*lb)[0][Geom::Y], static_cast<unsigned int>(i), static_cast<unsigned int>(j));
                            TS_FAIL(temp);
                            return false;
                        }
                        if (!Geom::are_near((*la)[1],(*lb)[1], eps)) {
                            char temp[200];
                            sprintf(temp, "Different 1st control point: (%g,%g) != (%g,%g), subpath: %u, segment: %u", (*la)[1][Geom::X], (*la)[1][Geom::Y], (*lb)[1][Geom::X], (*lb)[1][Geom::Y], static_cast<unsigned int>(i), static_cast<unsigned int>(j));
                            TS_FAIL(temp);
                            return false;
                        }
                        if (!Geom::are_near((*la)[2],(*lb)[2], eps)) {
                            char temp[200];
                            sprintf(temp, "Different 2nd control point: (%g,%g) != (%g,%g), subpath: %u, segment: %u", (*la)[2][Geom::X], (*la)[2][Geom::Y], (*lb)[2][Geom::X], (*lb)[2][Geom::Y], static_cast<unsigned int>(i), static_cast<unsigned int>(j));
                            TS_FAIL(temp);
                            return false;
                        }
                        if (!Geom::are_near((*la)[3],(*lb)[3], eps)) {
                            char temp[200];
                            sprintf(temp, "Different end of segment: (%g,%g) != (%g,%g), subpath: %u, segment: %u", (*la)[3][Geom::X], (*la)[3][Geom::Y], (*lb)[3][Geom::X], (*lb)[3][Geom::Y], static_cast<unsigned int>(i), static_cast<unsigned int>(j));
                            TS_FAIL(temp);
                            return false;
                        }
                    }
                    else
                    {
                        char temp[200];
                        sprintf(temp, "Unknown curve type: %s, subpath: %u, segment: %u", typeid(*ca).name(), static_cast<unsigned int>(i), static_cast<unsigned int>(j));
                        TS_FAIL(temp);
                    }
                }
                else // not same type
                {
                    char temp[200];
                    sprintf(temp, "Different curve types: %s != %s, subpath: %u, segment: %u", typeid(*ca).name(), typeid(*cb).name(), static_cast<unsigned int>(i), static_cast<unsigned int>(j));
                    TS_FAIL(temp);
                    return false;
                }
            }
        }
        return true;
    }
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
