#include <cxxtest/TestSuite.h>
#include "libnr/n-art-bpath.h"
#include "svg/svg.h"
#include "2geom/coord.h"
#include "prefs-utils.h"
#include "streq.h"
#include <string>
#include <vector>
#include <glib/gmem.h>

class SvgPathTest : public CxxTest::TestSuite
{
private:
    std::vector<std::string> rectanglesAbsoluteClosed;
    std::vector<std::string> rectanglesRelativeClosed;
    std::vector<std::string> rectanglesAbsoluteOpen;
    std::vector<std::string> rectanglesRelativeOpen;
    NArtBpath rectangleBpath[5+1];
public:
    SvgPathTest() {
        // Lots of ways to define the same rectangle
        rectanglesAbsoluteClosed.push_back("M 1,2 L 4,2 L 4,8 L 1,8 L 1,2 Z");
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
        rectangleBpath[0].code = NR_MOVETO;
        rectangleBpath[0].x3 = 1;
        rectangleBpath[0].y3 = 2;
        rectangleBpath[1].code = NR_LINETO;
        rectangleBpath[1].x3 = 4;
        rectangleBpath[1].y3 = 2;
        rectangleBpath[2].code = NR_LINETO;
        rectangleBpath[2].x3 = 4;
        rectangleBpath[2].y3 = 8;
        rectangleBpath[3].code = NR_LINETO;
        rectangleBpath[3].x3 = 1;
        rectangleBpath[3].y3 = 8;
        rectangleBpath[4].code = NR_LINETO;
        rectangleBpath[4].x3 = 1;
        rectangleBpath[4].y3 = 2;
        rectangleBpath[5].code = NR_END;
        // TODO: Also test some (smooth) cubic/quadratic beziers and elliptical arcs
    }

    void testReadRectanglesAbsoluteClosed()
    {
        rectangleBpath[0].code = NR_MOVETO;
        for(size_t i=0; i<rectanglesAbsoluteClosed.size(); i++) {
            NArtBpath * bpath = sp_svg_read_path(rectanglesAbsoluteClosed[i].c_str());
            TS_ASSERT(bpathEqual(bpath,rectangleBpath));
            g_free(bpath);
        }
    }

    void testReadRectanglesRelativeClosed()
    {
        rectangleBpath[0].code = NR_MOVETO;
        for(size_t i=0; i<rectanglesRelativeClosed.size(); i++) {
            NArtBpath * bpath = sp_svg_read_path(rectanglesRelativeClosed[i].c_str());
            TS_ASSERT(bpathEqual(bpath,rectangleBpath));
            g_free(bpath);
        }
    }

    void testReadRectanglesAbsoluteOpen()
    {
        rectangleBpath[0].code = NR_MOVETO_OPEN;
        for(size_t i=0; i<rectanglesAbsoluteOpen.size(); i++) {
            NArtBpath * bpath = sp_svg_read_path(rectanglesAbsoluteOpen[i].c_str());
            TS_ASSERT(bpathEqual(bpath,rectangleBpath));
            g_free(bpath);
        }
    }

    void testReadRectanglesRelativeOpen()
    {
        rectangleBpath[0].code = NR_MOVETO_OPEN;
        for(size_t i=0; i<rectanglesRelativeOpen.size(); i++) {
            NArtBpath * bpath = sp_svg_read_path(rectanglesRelativeOpen[i].c_str());
            TS_ASSERT(bpathEqual(bpath,rectangleBpath));
            g_free(bpath);
        }
    }

    void testReadConcatenatedPaths()
    {
        NArtBpath bpath_good[4*5+1];
        for(size_t i=0; i<4; i++) {
            memcpy(bpath_good+i*5,rectangleBpath,sizeof(rectangleBpath[0])*5);
        }
        bpath_good[0*5].code = NR_MOVETO;
        bpath_good[1*5].code = NR_MOVETO_OPEN;
        bpath_good[2*5].code = NR_MOVETO;
        bpath_good[3*5].code = NR_MOVETO_OPEN;
        bpath_good[4*5].code = NR_END;
        for(size_t i=0; i<5; i++) {
            bpath_good[1*5+i].x3 += bpath_good[0*5+4].x3;
            bpath_good[1*5+i].y3 += bpath_good[0*5+4].y3;
        }
        for(size_t i=0; i<5; i++) {
            bpath_good[2*5+i].x3 += bpath_good[1*5+4].x3;
            bpath_good[2*5+i].y3 += bpath_good[1*5+4].y3;
        }
        std::string path_str = rectanglesAbsoluteClosed[0] + rectanglesRelativeOpen[0] + rectanglesRelativeClosed[0] + rectanglesAbsoluteOpen[0];
        NArtBpath * bpath = sp_svg_read_path(path_str.c_str());
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
    }

    void testReadZeroLengthSubpaths() {
        // Per the SVG 1.1 specification (section F5) zero-length subpaths are relevant
        NArtBpath bpath_good[8+1];
        bpath_good[0].code = NR_MOVETO_OPEN;
        bpath_good[0].x3 = bpath_good[0].y3 = 0;
        bpath_good[1].code = NR_MOVETO_OPEN;
        bpath_good[1].x3 = bpath_good[1].y3 = 1;
        bpath_good[2].code = NR_LINETO;
        bpath_good[2].x3 = bpath_good[2].y3 = 2;
        bpath_good[3].code = NR_MOVETO;
        bpath_good[3].x3 = bpath_good[3].y3 = 3;
        bpath_good[4].code = NR_MOVETO;
        bpath_good[4].x3 = bpath_good[4].y3 = 4;
        bpath_good[5].code = NR_LINETO;
        bpath_good[5].x3 = bpath_good[5].y3 = 5;
        bpath_good[6].code = NR_LINETO;
        bpath_good[6].x3 = bpath_good[6].y3 = 4;
        bpath_good[7].code = NR_MOVETO_OPEN;
        bpath_good[7].x3 = bpath_good[7].y3 = 6;
        bpath_good[8].code = NR_END;
        {   // Test absolute version
            char const * path_str = "M 0,0 M 1,1 L 2,2 M 3,3 z M 4,4 L 5,5 z M 6,6";
            NArtBpath * bpath = sp_svg_read_path(path_str);
            TS_ASSERT(bpathEqual(bpath,bpath_good));
            g_free(bpath);
        }
        {   // Test relative version
            char const * path_str = "m 0,0 m 1,1 l 1,1 m 1,1 z m 1,1 l 1,1 z m 2,2";
            NArtBpath * bpath = sp_svg_read_path(path_str);
            TS_ASSERT(bpathEqual(bpath,bpath_good));
            g_free(bpath);
        }
    }

    void testReadImplicitMoveto() {
        NArtBpath bpath_good[6+1];
        bpath_good[0].code = NR_MOVETO;
        bpath_good[0].x3 = bpath_good[0].y3 = 1;
        bpath_good[1].code = NR_LINETO;
        bpath_good[1].x3 = bpath_good[1].y3 = 2;
        bpath_good[2].code = NR_LINETO;
        bpath_good[2].x3 = bpath_good[2].y3 = 1;
        bpath_good[3].code = NR_MOVETO;
        bpath_good[3].x3 = bpath_good[3].y3 = 1;
        bpath_good[4].code = NR_LINETO;
        bpath_good[4].x3 = bpath_good[4].y3 = 3;
        bpath_good[5].code = NR_LINETO;
        bpath_good[5].x3 = bpath_good[5].y3 = 1;
        bpath_good[6].code = NR_END;
        {   // Test absolute version
            char const * path_str = "M 1,1 L 2,2 z L 3,3 z";
            NArtBpath * bpath = sp_svg_read_path(path_str);
            TS_ASSERT(bpathEqual(bpath,bpath_good));
            g_free(bpath);
        }
        {   // Test relative version
            char const * path_str = "M 1,1 L 2,2 z L 3,3 z";
            NArtBpath * bpath = sp_svg_read_path(path_str);
            TS_ASSERT(bpathEqual(bpath,bpath_good));
            g_free(bpath);
        }
    }

    void testReadFloatingPoint() {
        NArtBpath bpath_good[5+1];
        bpath_good[0].code = NR_MOVETO;
        bpath_good[0].x3 = .01;
        bpath_good[0].y3 = .02;
        bpath_good[1].code = NR_LINETO;
        bpath_good[1].x3 = .04;
        bpath_good[1].y3 = .02;
        bpath_good[2].code = NR_LINETO;
        bpath_good[2].x3 = 1.5;
        bpath_good[2].y3 = 1.6;
        bpath_good[3].code = NR_LINETO;
        bpath_good[3].x3 = .01;
        bpath_good[3].y3 = .08;
        bpath_good[4].code = NR_LINETO;
        bpath_good[4].x3 = .01;
        bpath_good[4].y3 = .02;
        bpath_good[5].code = NR_END;
        {   // Test decimals
            char const * path_str = "M .01,.02 L.04.02 L1.5,1.6L0.01,0.08 .01.02 z";
            NArtBpath * bpath = sp_svg_read_path(path_str);
            TS_ASSERT(bpathEqual(bpath,bpath_good));
            g_free(bpath);
        }
        {   // Test exponent
            char const * path_str = "M 1e-2,.2e-1 L 0.004e1,0.0002e+2 L0150E-2,1.6e0L1.0e-2,80e-3 z";
            NArtBpath * bpath = sp_svg_read_path(path_str);
            TS_ASSERT(bpathEqual(bpath,bpath_good));
            g_free(bpath);
        }
    }

    void testReadImplicitSeparation() {
        // Coordinates need not be separated by whitespace if they can still be read unambiguously
        NArtBpath bpath_good[5+1];
        bpath_good[0].code = NR_MOVETO;
        bpath_good[0].x3 = .1;
        bpath_good[0].y3 = .2;
        bpath_good[1].code = NR_LINETO;
        bpath_good[1].x3 = .4;
        bpath_good[1].y3 = .2;
        bpath_good[2].code = NR_LINETO;
        bpath_good[2].x3 = .4;
        bpath_good[2].y3 = .8;
        bpath_good[3].code = NR_LINETO;
        bpath_good[3].x3 = .1;
        bpath_good[3].y3 = .8;
        bpath_good[4].code = NR_LINETO;
        bpath_good[4].x3 = .1;
        bpath_good[4].y3 = .2;
        bpath_good[5].code = NR_END;
        {   // Test absolute
            char const * path_str = "M .1.2+0.4.2e0.4e0+8e-1.1.8 z";
            NArtBpath * bpath = sp_svg_read_path(path_str);
            TS_ASSERT(bpathEqual(bpath,bpath_good));
            g_free(bpath);
        }
        {   // Test relative
            char const * path_str = "m .1.2+0.3.0e0.0e0+6e-1-.3.0 z";
            NArtBpath * bpath = sp_svg_read_path(path_str);
            TS_ASSERT(bpathEqual(bpath,bpath_good));
            g_free(bpath);
        }
    }

    void testReadErrorMisplacedCharacter() {
        char const * path_str;
        NArtBpath * bpath;
        NArtBpath * bpath_good = rectangleBpath;
        bpath_good[0].code = NR_MOVETO;
        // Comma in the wrong place (commas may only appear between parameters)
        path_str = "M 1,2 4,2 4,8 1,8 z , m 13,15";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
        // Comma in the wrong place (commas may only appear between parameters)
        path_str = "M 1,2 4,2 4,8 1,8 z m,13,15";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
        // Period in the wrong place (no numbers after a 'z')
        path_str = "M 1,2 4,2 4,8 1,8 z . m 13,15";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
        // Sign in the wrong place (no numbers after a 'z')
        path_str = "M 1,2 4,2 4,8 1,8 z + - m 13,15";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
        // Digit in the wrong place (no numbers after a 'z')
        path_str = "M 1,2 4,2 4,8 1,8 z 9809 m 13,15";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
        // Digit in the wrong place (no numbers after a 'z')
        path_str = "M 1,2 4,2 4,8 1,8 z 9809 876 m 13,15";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
    }

    void testReadErrorUnrecognizedCharacter() {
        char const * path_str;
        NArtBpath * bpath;
        NArtBpath * bpath_good = rectangleBpath;
        bpath_good[0].code = NR_MOVETO;
        // Unrecognized character
        path_str = "M 1,2 4,2 4,8 1,8 z&m 13,15";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
        // Unrecognized character
        path_str = "M 1,2 4,2 4,8 1,8 z m &13,15";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
    }

    void testReadErrorTypo() {
        char const * path_str;
        NArtBpath * bpath;
        NArtBpath * bpath_good = rectangleBpath;
        bpath_good[0].code = NR_MOVETO;
        // Typo
        path_str = "M 1,2 4,2 4,8 1,8 z j 13,15";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);

        bpath_good[0].code = NR_MOVETO_OPEN;
        // Typo
        path_str = "M 1,2 4,2 4,8 1,8 L 1,2 x m 13,15";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
    }

    void testReadErrorIllformedNumbers() {
        char const * path_str;
        NArtBpath * bpath;
        NArtBpath * bpath_good = rectangleBpath;
        bpath_good[0].code = NR_MOVETO;
        // Double exponent
        path_str = "M 1,2 4,2 4,8 1,8 z m 13e4e5,15";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
        // Double sign
        path_str = "M 1,2 4,2 4,8 1,8 z m +-13,15";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
        // Double sign
        path_str = "M 1,2 4,2 4,8 1,8 z m 13e+-12,15";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
        // No digit
        path_str = "M 1,2 4,2 4,8 1,8 z m .e12,15";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
        // No digit
        path_str = "M 1,2 4,2 4,8 1,8 z m .,15";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
        // No digit
        path_str = "M 1,2 4,2 4,8 1,8 z m +,15";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
        // No digit
        path_str = "M 1,2 4,2 4,8 1,8 z m +.e+,15";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
    }

    void testReadErrorJunk() {
        char const * path_str;
        NArtBpath * bpath;
        NArtBpath * bpath_good = rectangleBpath;
        bpath_good[0].code = NR_MOVETO;
        // Junk
        path_str = "M 1,2 4,2 4,8 1,8 z j 357 hkjh.,34e34 90ih6kj4 h5k6vlh4N.,6,45wikuyi3yere..3487 m 13,23";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
    }

    void testReadErrorStopReading() {
        char const * path_str;
        NArtBpath * bpath;
        NArtBpath * bpath_good = rectangleBpath;
        bpath_good[0].code = NR_MOVETO;
        // Unrecognized parameter
        path_str = "M 1,2 4,2 4,8 1,8 z m #$%,23,34";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
        // Invalid parameter
        path_str = "M 1,2 4,2 4,8 1,8 z m #$%,23,34";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
        // Illformed parameter
        path_str = "M 1,2 4,2 4,8 1,8 z m +-12,23,34";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);

        bpath_good[0].code = NR_MOVETO_OPEN;
        // "Third" parameter
        path_str = "M 1,2 4,2 4,8 1,8 1,2,3 M 12,23";
        bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,bpath_good));
        g_free(bpath);
    }

    void testRoundTrip() {
        // This is the easiest way to (also) test writing path data, as a path can be written in more than one way.
        NArtBpath * bpath;
        NArtBpath * new_bpath;
        char * path_str;
        // Rectangle (closed)
        bpath = sp_svg_read_path(rectanglesAbsoluteClosed[0].c_str());
        path_str = sp_svg_write_path(bpath);
        new_bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,new_bpath));
        g_free(bpath); g_free(path_str); g_free(new_bpath);
        // Rectangle (open)
        bpath = sp_svg_read_path(rectanglesAbsoluteOpen[0].c_str());
        path_str = sp_svg_write_path(bpath);
        new_bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,new_bpath));
        g_free(bpath); g_free(path_str); g_free(new_bpath);
        // Concatenated rectangles
        bpath = sp_svg_read_path((rectanglesAbsoluteClosed[0] + rectanglesRelativeOpen[0] + rectanglesRelativeClosed[0] + rectanglesAbsoluteOpen[0]).c_str());
        path_str = sp_svg_write_path(bpath);
        new_bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,new_bpath));
        g_free(bpath); g_free(path_str); g_free(new_bpath);
        // Zero-length subpaths
        bpath = sp_svg_read_path("M 0,0 M 1,1 L 2,2 M 3,3 z M 4,4 L 5,5 z M 6,6");
        path_str = sp_svg_write_path(bpath);
        new_bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath,new_bpath));
        g_free(bpath); g_free(path_str); g_free(new_bpath);
        // Floating-point
        bpath = sp_svg_read_path("M .01,.02 L 0.04,0.02 L.04,.08L0.01,0.08 z""M 1e-2,.2e-1 L 0.004e1,0.0002e+2 L04E-2,.08e0L1.0e-2,80e-3 z");
        path_str = sp_svg_write_path(bpath);
        new_bpath = sp_svg_read_path(path_str);
        TS_ASSERT(bpathEqual(bpath, new_bpath, 1e-17));
        g_free(bpath); g_free(path_str); g_free(new_bpath);
    }

    void testMinexpPrecision() {
        NArtBpath * bpath;
        char * path_str;
        // Default values
        prefs_set_int_attribute("options.svgoutput", "allowrelativecoordinates", 1);
        prefs_set_int_attribute("options.svgoutput", "forcerepeatcommands", 0);
        prefs_set_int_attribute("options.svgoutput", "numericprecision", 8);
        prefs_set_int_attribute("options.svgoutput", "minimumexponent", -8);
        bpath = sp_svg_read_path("M 123456781,1.23456781e-8 L 123456782,1.23456782e-8 L 123456785,1.23456785e-8 L 10123456400,1.23456785e-8 L 123456789,1.23456789e-8 L 123456789,101.234564e-8 L 123456789,1.23456789e-8");
        path_str = sp_svg_write_path(bpath);
        TS_ASSERT_RELATION( streq_rel , "m 123456780,1.2345678e-8 0,0 10,1e-15 9999999210,0 -9999999210,0 0,9.99999921e-7 0,-9.99999921e-7" , path_str );
        g_free(bpath); g_free(path_str);
    }

private:
    bool bpathEqual(NArtBpath const * a, NArtBpath const * b, double eps = 1e-16) {
        while(a->code != NR_END && b->code == a->code) {
            switch(a->code) {
            case NR_MOVETO:
            case NR_MOVETO_OPEN:
            case NR_LINETO:
                if (!Geom::are_near(a->x3,b->x3, eps) || !Geom::are_near(a->y3,b->y3, eps)) return false;
                break;
            case NR_CURVETO:
                if (!Geom::are_near(a->x1,b->x1, eps) || !Geom::are_near(a->y1,b->y1, eps)) return false;
                if (!Geom::are_near(a->x2,b->x2, eps) || !Geom::are_near(a->y2,b->y2, eps)) return false;
                if (!Geom::are_near(a->x3,b->x3, eps) || !Geom::are_near(a->y3,b->y3, eps)) return false;
                break;
            default:
                TS_FAIL("Unknown path code!");
            }
            a++;
            b++;
        }
        return a->code == b->code;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
