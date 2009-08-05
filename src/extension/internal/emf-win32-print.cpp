/** @file
 * @brief Enhanced Metafile printing
 */
/* Authors:
 *   Ulf Erikson <ulferikson@users.sf.net>
 *
 * Copyright (C) 2006-2009 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
/*
 * References:
 *  - How to Create & Play Enhanced Metafiles in Win32
 *      http://support.microsoft.com/kb/q145999/
 *  - INFO: Windows Metafile Functions & Aldus Placeable Metafiles
 *      http://support.microsoft.com/kb/q66949/
 *  - Metafile Functions
 *      http://msdn.microsoft.com/library/en-us/gdi/metafile_0whf.asp
 *  - Metafile Structures
 *      http://msdn.microsoft.com/library/en-us/gdi/metafile_5hkj.asp
 */

#ifdef WIN32

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

//#include <string.h>
//#include <signal.h>
//#include <errno.h>

#include <2geom/pathvector.h>
#include <2geom/rect.h>
#include <2geom/bezier-curve.h>
#include <2geom/hvlinesegment.h>
#include "helper/geom.h"
#include "helper/geom-curves.h"
//#include "display/canvas-bpath.h"
#include "sp-item.h"

//#include "glib.h"
//#include "gtk/gtkdialog.h"
//#include "gtk/gtkbox.h"
//#include "gtk/gtkstock.h"

//#include "glibmm/i18n.h"
//#include "enums.h"
//#include "document.h"
#include "style.h"
//#include "sp-paint-server.h"
#include "inkscape-version.h"

//#include "libnrtype/FontFactory.h"
//#include "libnrtype/font-instance.h"
//#include "libnrtype/font-style-to-pos.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "win32.h"
#include "emf-win32-print.h"

#include "unit-constants.h"

//#include "extension/extension.h"
#include "extension/system.h"
#include "extension/print.h"

//#include "io/sys.h"

//#include "macros.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

static float dwDPI = 2540;


PrintEmfWin32::PrintEmfWin32 (void):
    hdc(NULL),
    hbrush(NULL),
    hbrushOld(NULL),
    hpen(NULL),
    stroke_and_fill(false),
    fill_only(false),
    simple_shape(false)
{
}


PrintEmfWin32::~PrintEmfWin32 (void)
{
    if (hdc) {
        HENHMETAFILE metafile = CloseEnhMetaFile( hdc );
        if ( metafile ) {
            DeleteEnhMetaFile( metafile );
        }
        DeleteDC( hdc );
    }

    /* restore default signal handling for SIGPIPE */
#if !defined(_WIN32) && !defined(__WIN32__)
    (void) signal(SIGPIPE, SIG_DFL);
#endif
    return;
}


unsigned int
PrintEmfWin32::setup (Inkscape::Extension::Print * /*mod*/)
{
    return TRUE;
}


unsigned int
PrintEmfWin32::begin (Inkscape::Extension::Print *mod, Document *doc)
{
    gchar const *utf8_fn = mod->get_param_string("destination");

    gsize bytesRead = 0;
    gsize bytesWritten = 0;
    GError* error = NULL;
    gchar *local_fn =
        g_filename_from_utf8( utf8_fn, -1,  &bytesRead,  &bytesWritten, &error );

    if (local_fn == NULL) {
        return 1;
    }

    CHAR *ansi_uri = (CHAR *) local_fn;
    gunichar2 *unicode_fn = g_utf8_to_utf16( local_fn, -1, NULL, NULL, NULL );
    WCHAR *unicode_uri = (WCHAR *) unicode_fn;

    // width and height in px
    _width = sp_document_width(doc);
    _height = sp_document_height(doc);

    NRRect d;
    bool pageBoundingBox;
    pageBoundingBox = mod->get_param_bool("pageBoundingBox");
    if (pageBoundingBox) {
        d.x0 = d.y0 = 0;
        d.x1 = _width;
        d.y1 = _height;
    } else {
        SPItem* doc_item = SP_ITEM(sp_document_root(doc));
        sp_item_invoke_bbox(doc_item, &d, sp_item_i2d_affine(doc_item), TRUE);
    }

    d.x0 *= IN_PER_PX;
    d.y0 *= IN_PER_PX;
    d.x1 *= IN_PER_PX;
    d.y1 *= IN_PER_PX;

    float dwInchesX = (d.x1 - d.x0);
    float dwInchesY = (d.y1 - d.y0);

    // dwInchesX x dwInchesY in .01mm units
    SetRect( &rc, 0, 0, (int) ceil(dwInchesX*2540), (int) ceil(dwInchesY*2540) );

    // Get a Reference DC
    HDC hScreenDC = GetDC( NULL );

    // Get the physical characteristics of the reference DC
    int PixelsX = GetDeviceCaps( hScreenDC, HORZRES );
    int PixelsY = GetDeviceCaps( hScreenDC, VERTRES );
    int MMX = GetDeviceCaps( hScreenDC, HORZSIZE );
    int MMY = GetDeviceCaps( hScreenDC, VERTSIZE );

    CHAR buff[1024];
    ZeroMemory(buff, sizeof(buff));
    snprintf(buff, sizeof(buff)-1, "Inkscape %s (%s)", Inkscape::version_string, __DATE__);
    INT len = strlen(buff);
    CHAR *p1 = strrchr(ansi_uri, '\\');
    CHAR *p2 = strrchr(ansi_uri, '/');
    CHAR *p = MAX(p1, p2);
    if (p)
        p++;
    else
        p = ansi_uri;
    snprintf(buff+len+1, sizeof(buff)-len-2, "%s", p);
    
    // Create the Metafile
    if (PrintWin32::is_os_wide()) {
        WCHAR wbuff[1024];
        ZeroMemory(wbuff, sizeof(wbuff));
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buff, sizeof(buff)/sizeof(buff[0]), wbuff, sizeof(wbuff)/sizeof(wbuff[0]));
        hdc = CreateEnhMetaFileW( hScreenDC, unicode_uri, &rc, wbuff );
    }
    else {
        hdc = CreateEnhMetaFileA( hScreenDC, ansi_uri, &rc, buff );
    }

    // Release the reference DC
    ReleaseDC( NULL, hScreenDC );

    // Did we get a good metafile?
    if (hdc == NULL)
    {
        g_free(local_fn);
        g_free(unicode_fn);
        return 1;
    }

    // Anisotropic mapping mode
    SetMapMode( hdc, MM_ANISOTROPIC );

    // Set the Windows extent
    int windowextX = (int) ceil(dwInchesX*dwDPI);
    int windowextY = (int) ceil(dwInchesY*dwDPI);
    SetWindowExtEx( hdc, windowextX, windowextY, NULL );

    // Set the viewport extent to reflect
    // dwInchesX" x dwInchesY" in device units
    int viewportextX = (int)((float)dwInchesX*25.4f*(float)PixelsX/(float)MMX);
    int viewportextY = (int)((float)dwInchesY*25.4f*(float)PixelsY/(float)MMY);
    SetViewportExtEx( hdc, viewportextX, viewportextY, NULL );

    if (1) {
        snprintf(buff, sizeof(buff)-1, "Screen=%dx%dpx, %dx%dmm", PixelsX, PixelsY, MMX, MMY);
        GdiComment(hdc, strlen(buff), (BYTE*) buff);

        snprintf(buff, sizeof(buff)-1, "Drawing=%.1lfx%.1lfpx, %.1lfx%.1lfmm", _width, _height, dwInchesX * MM_PER_IN, dwInchesY * MM_PER_IN);
        GdiComment(hdc, strlen(buff), (BYTE*) buff);
    }

    SetRect( &rc, 0, 0, (int) ceil(dwInchesX*dwDPI), (int) ceil(dwInchesY*dwDPI) );

    g_free(local_fn);
    g_free(unicode_fn);

    m_tr_stack.push( Geom::Scale(1, -1) * Geom::Translate(0, sp_document_height(doc)));

    return 0;
}


unsigned int
PrintEmfWin32::finish (Inkscape::Extension::Print * /*mod*/)
{
    if (!hdc) return 0;

    flush_fill(); // flush any pending fills

    HENHMETAFILE metafile = CloseEnhMetaFile( hdc );
    if ( metafile ) {
        DeleteEnhMetaFile( metafile );
    }
    DeleteDC( hdc );

    hdc = NULL;

    return 0;
}


unsigned int
PrintEmfWin32::comment (Inkscape::Extension::Print * /*module*/,
                        const char * /*comment*/)
{
    if (!hdc) return 0;

    flush_fill(); // flush any pending fills

    return 0;
}


int
PrintEmfWin32::create_brush(SPStyle const *style)
{
    float rgb[3];

    if (style) {
        float opacity = SP_SCALE24_TO_FLOAT(style->fill_opacity.value);
        if (opacity <= 0.0)
            return 1;

        sp_color_get_rgb_floatv( &style->fill.value.color, rgb );
        hbrush = CreateSolidBrush( RGB(255*rgb[0], 255*rgb[1], 255*rgb[2]) );
        hbrushOld = (HBRUSH) SelectObject( hdc, hbrush );

        SetPolyFillMode( hdc,
                         style->fill_rule.computed == 0 ? WINDING :
                         style->fill_rule.computed == 2 ? ALTERNATE : ALTERNATE );
    } else { // if (!style)
        hbrush = CreateSolidBrush( RGB(255, 255, 255) );
        hbrushOld = (HBRUSH) SelectObject( hdc, hbrush );
        SetPolyFillMode( hdc, ALTERNATE );
    }

    return 0;
}


void
PrintEmfWin32::destroy_brush()
{
    SelectObject( hdc, hbrushOld );
    if (hbrush)
        DeleteObject( hbrush );
    hbrush = NULL;
    hbrushOld = NULL;
}


void
PrintEmfWin32::create_pen(SPStyle const *style, const Geom::Matrix &transform)
{
    if (style) {
        float rgb[3];

        sp_color_get_rgb_floatv(&style->stroke.value.color, rgb);

        LOGBRUSH lb;
        ZeroMemory(&lb, sizeof(lb));
        lb.lbStyle = BS_SOLID;
        lb.lbColor = RGB( 255*rgb[0], 255*rgb[1], 255*rgb[2] );

        int linestyle = PS_SOLID;
        int linecap = 0;
        int linejoin = 0;
        DWORD n_dash = 0;
        DWORD *dash = NULL;

        using Geom::X;
        using Geom::Y;

        Geom::Point zero(0, 0);
        Geom::Point one(1, 1);
        Geom::Point p0(zero * transform);
        Geom::Point p1(one * transform);
        Geom::Point p(p1 - p0);

        double scale = sqrt( (p[X]*p[X]) + (p[Y]*p[Y]) ) / sqrt(2);

        DWORD linewidth = MAX( 1, (DWORD) (scale * style->stroke_width.computed * IN_PER_PX * dwDPI) );

        if (style->stroke_linecap.computed == 0) {
            linecap = PS_ENDCAP_FLAT;
        }
        else if (style->stroke_linecap.computed == 1) {
            linecap = PS_ENDCAP_ROUND;
        }
        else if (style->stroke_linecap.computed == 2) {
            linecap = PS_ENDCAP_SQUARE;
        }

        if (style->stroke_linejoin.computed == 0) {
            linejoin = PS_JOIN_MITER;
        }
        else if (style->stroke_linejoin.computed == 1) {
            linejoin = PS_JOIN_ROUND;
        }
        else if (style->stroke_linejoin.computed == 2) {
            linejoin = PS_JOIN_BEVEL;
        }

        if (style->stroke_dash.n_dash   &&
            style->stroke_dash.dash       )
        {
            int i = 0;
            while (linestyle != PS_USERSTYLE &&
                   (i < style->stroke_dash.n_dash)) {
                if (style->stroke_dash.dash[i] > 0.00000001)
                    linestyle = PS_USERSTYLE;
                i++;
            }

            if (linestyle == PS_USERSTYLE) {
                n_dash = style->stroke_dash.n_dash;
                dash = new DWORD[n_dash];
                for (i = 0; i < style->stroke_dash.n_dash; i++) {
                    dash[i] = (DWORD) (style->stroke_dash.dash[i] * IN_PER_PX * dwDPI);
                }
            }
        }

        hpen = ExtCreatePen(
            PS_GEOMETRIC | linestyle | linecap | linejoin,
            linewidth,
            &lb,
            n_dash,
            dash );

        if ( !hpen && linestyle == PS_USERSTYLE ) {
            hpen = ExtCreatePen(
                PS_GEOMETRIC | PS_SOLID | linecap | linejoin,
                linewidth,
                &lb,
                0,
                NULL );
        }

        if ( !hpen ) {
            hpen = CreatePen(
                PS_SOLID,
                linewidth,
                lb.lbColor );
        }

        hpenOld = (HPEN) SelectObject( hdc, hpen );

        if (linejoin == PS_JOIN_MITER) {
            float oldmiterlimit;
            float miterlimit = style->stroke_miterlimit.value;

            miterlimit = miterlimit * 10.0 / 4.0;
            if (miterlimit < 1)
                miterlimit = 10.0;

            miterlimit = miterlimit * IN_PER_PX * dwDPI;

            SetMiterLimit(
                hdc,
                miterlimit,
                &oldmiterlimit );
        }

        if (n_dash) {
            delete[] dash;
        }
    }
    else { // if (!style)
        hpen = CreatePen( PS_SOLID, 1, RGB(0, 0, 0) );
        hpenOld = (HPEN) SelectObject( hdc, hpen );
    }
}


void
PrintEmfWin32::destroy_pen()
{
    SelectObject( hdc, hpenOld );
    if (hpen)
        DeleteObject( hpen );
    hpen = NULL;
}


void
PrintEmfWin32::flush_fill()
{
    if (!fill_pathv.empty()) {
        stroke_and_fill = false;
        fill_only = true;
        print_pathv(fill_pathv, fill_transform);
        fill_only = false;
        if (!simple_shape)
            FillPath( hdc );
        destroy_brush();
        fill_pathv.clear();
    }
}

unsigned int
PrintEmfWin32::bind(Inkscape::Extension::Print * /*mod*/, Geom::Matrix const *transform, float /*opacity*/)
{
    Geom::Matrix tr = *transform;
    
    if (m_tr_stack.size()) {
        Geom::Matrix tr_top = m_tr_stack.top();
        m_tr_stack.push(tr * tr_top);
    } else {
        m_tr_stack.push(tr);
    }

    return 1;
}

unsigned int
PrintEmfWin32::release(Inkscape::Extension::Print * /*mod*/)
{
    m_tr_stack.pop();
    return 1;
}

unsigned int
PrintEmfWin32::fill(Inkscape::Extension::Print * /*mod*/,
                    Geom::PathVector const &pathv, Geom::Matrix const * /*transform*/, SPStyle const *style,
                    NRRect const * /*pbox*/, NRRect const * /*dbox*/, NRRect const * /*bbox*/)
{
    if (!hdc) return 0;

    Geom::Matrix tf = m_tr_stack.top();

    flush_fill(); // flush any pending fills

    if (style->fill.isColor()) {
        if (create_brush(style))
            return 0;
    } else {
        // create_brush(NULL);
        return 0;
    }

    fill_pathv.clear();
    std::copy(pathv.begin(), pathv.end(), std::back_inserter(fill_pathv));
    fill_transform = tf;

    // postpone fill in case of stroke-and-fill

    return 0;
}


unsigned int
PrintEmfWin32::stroke (Inkscape::Extension::Print * /*mod*/,
                       Geom::PathVector const &pathv, const Geom::Matrix * /*transform*/, const SPStyle *style,
                       const NRRect * /*pbox*/, const NRRect * /*dbox*/, const NRRect * /*bbox*/)
{
    if (!hdc) return 0;

    Geom::Matrix tf = m_tr_stack.top();

    stroke_and_fill = ( pathv == fill_pathv );

    if (!stroke_and_fill) {
        flush_fill(); // flush any pending fills
    }

    if (style->stroke.isColor()) {
        create_pen(style, tf);
    } else {
        // create_pen(NULL, tf);
        return 0;
    }

    print_pathv(pathv, tf);

    if (stroke_and_fill) {
        if (!simple_shape)
            StrokeAndFillPath( hdc );
        destroy_brush();
        fill_pathv.clear();
    } else {
        if (!simple_shape)
            StrokePath( hdc );
    }

    destroy_pen();

    return 0;
}


bool
PrintEmfWin32::print_simple_shape(Geom::PathVector const &pathv, const Geom::Matrix &transform)
{
    Geom::PathVector pv = pathv_to_linear_and_cubic_beziers( pathv * transform );
    
    int nodes = 0;
    int moves = 0;
    int lines = 0;
    int curves = 0;

    for (Geom::PathVector::const_iterator pit = pv.begin(); pit != pv.end(); ++pit)
    {
        moves++;
        nodes++;
        
        for (Geom::Path::const_iterator cit = pit->begin(); cit != pit->end_open(); ++cit)
        {
            nodes++;
            
            if ( is_straight_curve(*cit) ) {
                lines++;
            }
            else if (Geom::CubicBezier const *cubic = dynamic_cast<Geom::CubicBezier const*>(&*cit)) {
                cubic = cubic;
                curves++;
            }
        }
    }

    if (!nodes)
        return false;
    
    POINT *lpPoints = new POINT[moves + lines + curves*3];
    int i = 0;

    /**
     * For all Subpaths in the <path>
     */	     
    for (Geom::PathVector::const_iterator pit = pv.begin(); pit != pv.end(); ++pit)
    {
        using Geom::X;
        using Geom::Y;

        Geom::Point p0 = pit->initialPoint();

        p0[X] = (p0[X] * IN_PER_PX * dwDPI);
        p0[Y] = (p0[Y] * IN_PER_PX * dwDPI);
                
        LONG const x0 = (LONG) round(p0[X]);
        LONG const y0 = (LONG) round(rc.bottom-p0[Y]);

        lpPoints[i].x = x0;
        lpPoints[i].y = y0;
        i = i + 1;

        /**
         * For all segments in the subpath
         */
        for (Geom::Path::const_iterator cit = pit->begin(); cit != pit->end_open(); ++cit)
        {
            if ( is_straight_curve(*cit) )
            {
                //Geom::Point p0 = cit->initialPoint();
                Geom::Point p1 = cit->finalPoint();

                //p0[X] = (p0[X] * IN_PER_PX * dwDPI);
                p1[X] = (p1[X] * IN_PER_PX * dwDPI);
                //p0[Y] = (p0[Y] * IN_PER_PX * dwDPI);
                p1[Y] = (p1[Y] * IN_PER_PX * dwDPI);
                
                //LONG const x0 = (LONG) round(p0[X]);
                //LONG const y0 = (LONG) round(rc.bottom-p0[Y]);
                LONG const x1 = (LONG) round(p1[X]);
                LONG const y1 = (LONG) round(rc.bottom-p1[Y]);

                lpPoints[i].x = x1;
                lpPoints[i].y = y1;
                i = i + 1;
            }
            else if (Geom::CubicBezier const *cubic = dynamic_cast<Geom::CubicBezier const*>(&*cit))
            {
                std::vector<Geom::Point> points = cubic->points();
                //Geom::Point p0 = points[0];
                Geom::Point p1 = points[1];
                Geom::Point p2 = points[2];
                Geom::Point p3 = points[3];

                //p0[X] = (p0[X] * IN_PER_PX * dwDPI);
                p1[X] = (p1[X] * IN_PER_PX * dwDPI);
                p2[X] = (p2[X] * IN_PER_PX * dwDPI);
                p3[X] = (p3[X] * IN_PER_PX * dwDPI);
                //p0[Y] = (p0[Y] * IN_PER_PX * dwDPI);
                p1[Y] = (p1[Y] * IN_PER_PX * dwDPI);
                p2[Y] = (p2[Y] * IN_PER_PX * dwDPI);
                p3[Y] = (p3[Y] * IN_PER_PX * dwDPI);
                
                //LONG const x0 = (LONG) round(p0[X]);
                //LONG const y0 = (LONG) round(rc.bottom-p0[Y]);
                LONG const x1 = (LONG) round(p1[X]);
                LONG const y1 = (LONG) round(rc.bottom-p1[Y]);
                LONG const x2 = (LONG) round(p2[X]);
                LONG const y2 = (LONG) round(rc.bottom-p2[Y]);
                LONG const x3 = (LONG) round(p3[X]);
                LONG const y3 = (LONG) round(rc.bottom-p3[Y]);

                POINT pt[3];
                pt[0].x = x1;
                pt[0].y = y1;
                pt[1].x = x2;
                pt[1].y = y2;
                pt[2].x = x3;
                pt[2].y = y3;

                lpPoints[i].x = x1;
                lpPoints[i].y = y1;
                lpPoints[i+1].x = x2;
                lpPoints[i+1].y = y2;
                lpPoints[i+2].x = x3;
                lpPoints[i+2].y = y3;
                i = i + 3;
            }
        }
    }

    bool done = false;
    bool closed = (lpPoints[0].x == lpPoints[i-1].x) && (lpPoints[0].y == lpPoints[i-1].y);
    bool polygon = false;
    bool rectangle = false;
    bool ellipse = false;
    
    if (moves == 1 && moves+lines == nodes && closed) {
        polygon = true;
        if (nodes==5) {
            if (lpPoints[0].x == lpPoints[3].x && lpPoints[1].x == lpPoints[2].x &&
                lpPoints[0].y == lpPoints[1].y && lpPoints[2].y == lpPoints[3].y)
            {
                rectangle = true;
            }
        }
    }
    else if (moves == 1 && nodes == 5 && moves+curves == nodes && closed) {
        if (lpPoints[0].x == lpPoints[1].x && lpPoints[1].x == lpPoints[11].x &&
            lpPoints[5].x == lpPoints[6].x && lpPoints[6].x == lpPoints[7].x &&
            lpPoints[2].x == lpPoints[10].x && lpPoints[3].x == lpPoints[9].x && lpPoints[4].x == lpPoints[8].x &&
            lpPoints[2].y == lpPoints[3].y && lpPoints[3].y == lpPoints[4].y &&
            lpPoints[8].y == lpPoints[9].y && lpPoints[9].y == lpPoints[10].y &&
            lpPoints[5].y == lpPoints[1].y && lpPoints[6].y == lpPoints[0].y && lpPoints[7].y == lpPoints[11].y)
        {
            ellipse = true;
        }
    }

    if (polygon || ellipse) {
        HPEN hpenTmp = NULL;
        HPEN hpenOld = NULL;
        HBRUSH hbrushTmp = NULL;
        HBRUSH hbrushOld = NULL;

        if (!stroke_and_fill) {
            if (fill_only) {
                hpenTmp = (HPEN) GetStockObject(NULL_PEN);
                hpenOld = (HPEN) SelectObject( hdc, hpenTmp );
            }
            else { // if (stroke_only)
                hbrushTmp = (HBRUSH) GetStockObject(NULL_BRUSH);
                hbrushOld = (HBRUSH) SelectObject( hdc, hbrushTmp );
            }
        }

        if (polygon) {
            if (rectangle)
                Rectangle( hdc, lpPoints[0].x, lpPoints[0].y, lpPoints[2].x, lpPoints[2].y );
            else
                Polygon( hdc, lpPoints, nodes );
        }
        else if (ellipse) {
            Ellipse( hdc, lpPoints[6].x, lpPoints[3].y, lpPoints[0].x, lpPoints[9].y);
        }
        
        done = true;

        if (hpenOld)
            SelectObject( hdc, hpenOld );
        if (hpenTmp)
            DeleteObject( hpenTmp );
        if (hbrushOld)
            SelectObject( hdc, hbrushOld );
        if (hbrushTmp)
            DeleteObject( hbrushTmp );
    }

    delete[] lpPoints;
    
    return done;
}

unsigned int
PrintEmfWin32::print_pathv(Geom::PathVector const &pathv, const Geom::Matrix &transform)
{
    simple_shape = print_simple_shape(pathv, transform);

    if (simple_shape)
        return TRUE;

    Geom::PathVector pv = pathv_to_linear_and_cubic_beziers( pathv * transform );
    
    BeginPath( hdc );

    /**
     * For all Subpaths in the <path>
     */	     
    for (Geom::PathVector::const_iterator pit = pv.begin(); pit != pv.end(); ++pit)
    {
        using Geom::X;
        using Geom::Y;

        Geom::Point p0 = pit->initialPoint();

        p0[X] = (p0[X] * IN_PER_PX * dwDPI);
        p0[Y] = (p0[Y] * IN_PER_PX * dwDPI);
                
        LONG const x0 = (LONG) round(p0[X]);
        LONG const y0 = (LONG) round(rc.bottom-p0[Y]);

        MoveToEx( hdc, x0, y0, NULL );

        /**
         * For all segments in the subpath
         */
        for (Geom::Path::const_iterator cit = pit->begin(); cit != pit->end_open(); ++cit)
        {
            if ( is_straight_curve(*cit) )
            {
                //Geom::Point p0 = cit->initialPoint();
                Geom::Point p1 = cit->finalPoint();

                //p0[X] = (p0[X] * IN_PER_PX * dwDPI);
                p1[X] = (p1[X] * IN_PER_PX * dwDPI);
                //p0[Y] = (p0[Y] * IN_PER_PX * dwDPI);
                p1[Y] = (p1[Y] * IN_PER_PX * dwDPI);
                
                //LONG const x0 = (LONG) round(p0[X]);
                //LONG const y0 = (LONG) round(rc.bottom-p0[Y]);
                LONG const x1 = (LONG) round(p1[X]);
                LONG const y1 = (LONG) round(rc.bottom-p1[Y]);

                LineTo( hdc, x1, y1 );
            }
            else if (Geom::CubicBezier const *cubic = dynamic_cast<Geom::CubicBezier const*>(&*cit))
            {
                std::vector<Geom::Point> points = cubic->points();
                //Geom::Point p0 = points[0];
                Geom::Point p1 = points[1];
                Geom::Point p2 = points[2];
                Geom::Point p3 = points[3];

                //p0[X] = (p0[X] * IN_PER_PX * dwDPI);
                p1[X] = (p1[X] * IN_PER_PX * dwDPI);
                p2[X] = (p2[X] * IN_PER_PX * dwDPI);
                p3[X] = (p3[X] * IN_PER_PX * dwDPI);
                //p0[Y] = (p0[Y] * IN_PER_PX * dwDPI);
                p1[Y] = (p1[Y] * IN_PER_PX * dwDPI);
                p2[Y] = (p2[Y] * IN_PER_PX * dwDPI);
                p3[Y] = (p3[Y] * IN_PER_PX * dwDPI);
                
                //LONG const x0 = (LONG) round(p0[X]);
                //LONG const y0 = (LONG) round(rc.bottom-p0[Y]);
                LONG const x1 = (LONG) round(p1[X]);
                LONG const y1 = (LONG) round(rc.bottom-p1[Y]);
                LONG const x2 = (LONG) round(p2[X]);
                LONG const y2 = (LONG) round(rc.bottom-p2[Y]);
                LONG const x3 = (LONG) round(p3[X]);
                LONG const y3 = (LONG) round(rc.bottom-p3[Y]);

                POINT pt[3];
                pt[0].x = x1;
                pt[0].y = y1;
                pt[1].x = x2;
                pt[1].y = y2;
                pt[2].x = x3;
                pt[2].y = y3;

                PolyBezierTo( hdc, pt, 3 );
            }
            else
            {
                g_warning("logical error, because pathv_to_linear_and_cubic_beziers was used");
            }
        }

        if (pit->end_default() == pit->end_closed()) {
            CloseFigure( hdc );
        }
    }

    EndPath( hdc );

    return TRUE;
}


bool
PrintEmfWin32::textToPath(Inkscape::Extension::Print * ext)
{
    return ext->get_param_bool("textToPath");
}

unsigned int
PrintEmfWin32::text(Inkscape::Extension::Print * /*mod*/, char const *text, Geom::Point p,
                    SPStyle const *const style)
{
    if (!hdc) return 0;

    HFONT hfont = NULL;
    
#ifdef USE_PANGO_WIN32
/*
    font_instance *tf = (font_factory::Default())->Face(style->text->font_family.value, font_style_to_pos(*style));
    if (tf) {
        LOGFONT *lf = pango_win32_font_logfont(tf->pFont);
        tf->Unref();
        hfont = CreateFontIndirect(lf);
        g_free(lf);
    }
*/
#endif

    if (!hfont) {
        if (PrintWin32::is_os_wide()) {
            LOGFONTW *lf = (LOGFONTW*)g_malloc(sizeof(LOGFONTW));
            g_assert(lf != NULL);
            
            lf->lfHeight = style->font_size.computed * IN_PER_PX * dwDPI;
            lf->lfWidth = 0;
            lf->lfEscapement = 0;
            lf->lfOrientation = 0;
            lf->lfWeight =
                style->font_weight.computed == SP_CSS_FONT_WEIGHT_100 ? FW_THIN :
                style->font_weight.computed == SP_CSS_FONT_WEIGHT_200 ? FW_EXTRALIGHT :
                style->font_weight.computed == SP_CSS_FONT_WEIGHT_300 ? FW_LIGHT :
                style->font_weight.computed == SP_CSS_FONT_WEIGHT_400 ? FW_NORMAL :
                style->font_weight.computed == SP_CSS_FONT_WEIGHT_500 ? FW_MEDIUM :
                style->font_weight.computed == SP_CSS_FONT_WEIGHT_600 ? FW_SEMIBOLD :
                style->font_weight.computed == SP_CSS_FONT_WEIGHT_700 ? FW_BOLD :
                style->font_weight.computed == SP_CSS_FONT_WEIGHT_800 ? FW_EXTRABOLD :
                style->font_weight.computed == SP_CSS_FONT_WEIGHT_900 ? FW_HEAVY :
                FW_NORMAL;
            lf->lfItalic = (style->font_style.computed == SP_CSS_FONT_STYLE_ITALIC);
            lf->lfUnderline = style->text_decoration.underline;
            lf->lfStrikeOut = style->text_decoration.line_through;
            lf->lfCharSet = DEFAULT_CHARSET;
            lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
            lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
            lf->lfQuality = DEFAULT_QUALITY;
            lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
            
            gunichar2 *unicode_name = g_utf8_to_utf16( style->text->font_family.value, -1, NULL, NULL, NULL );
            wcsncpy(lf->lfFaceName, (wchar_t*) unicode_name, LF_FACESIZE-1);
            g_free(unicode_name);
            
            hfont = CreateFontIndirectW(lf);
            
            g_free(lf);
        }
        else {
            LOGFONTA *lf = (LOGFONTA*)g_malloc(sizeof(LOGFONTA));
            g_assert(lf != NULL);
            
            lf->lfHeight = style->font_size.computed * IN_PER_PX * dwDPI;
            lf->lfWidth = 0;
            lf->lfEscapement = 0;
            lf->lfOrientation = 0;
            lf->lfWeight =
                style->font_weight.computed == SP_CSS_FONT_WEIGHT_100 ? FW_THIN :
                style->font_weight.computed == SP_CSS_FONT_WEIGHT_200 ? FW_EXTRALIGHT :
                style->font_weight.computed == SP_CSS_FONT_WEIGHT_300 ? FW_LIGHT :
                style->font_weight.computed == SP_CSS_FONT_WEIGHT_400 ? FW_NORMAL :
                style->font_weight.computed == SP_CSS_FONT_WEIGHT_500 ? FW_MEDIUM :
                style->font_weight.computed == SP_CSS_FONT_WEIGHT_600 ? FW_SEMIBOLD :
                style->font_weight.computed == SP_CSS_FONT_WEIGHT_700 ? FW_BOLD :
                style->font_weight.computed == SP_CSS_FONT_WEIGHT_800 ? FW_EXTRABOLD :
                style->font_weight.computed == SP_CSS_FONT_WEIGHT_900 ? FW_HEAVY :
                FW_NORMAL;
            lf->lfItalic = (style->font_style.computed == SP_CSS_FONT_STYLE_ITALIC);
            lf->lfUnderline = style->text_decoration.underline;
            lf->lfStrikeOut = style->text_decoration.line_through;
            lf->lfCharSet = DEFAULT_CHARSET;
            lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
            lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
            lf->lfQuality = DEFAULT_QUALITY;
            lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
            
            strncpy(lf->lfFaceName, (char*) style->text->font_family.value, LF_FACESIZE-1);

            hfont = CreateFontIndirectA(lf);
            
            g_free(lf);
        }
    }
    
    HFONT hfontOld = (HFONT) SelectObject(hdc, hfont);

    float rgb[3];
    sp_color_get_rgb_floatv( &style->fill.value.color, rgb );
    SetTextColor(hdc, RGB(255*rgb[0], 255*rgb[1], 255*rgb[2]));

    // Text alignment:
    //   - (x,y) coordinates received by this filter are those of the point where the text
    //     actually starts, and already takes into account the text object's alignment;
    //   - for this reason, the EMF text alignment must always be TA_BASELINE|TA_LEFT.
    SetTextAlign(hdc, TA_BASELINE | TA_LEFT);

    // Transparent text background
    SetBkMode(hdc, TRANSPARENT);

    Geom::Matrix tf = m_tr_stack.top();

    p = p * tf;
    p[Geom::X] = (p[Geom::X] * IN_PER_PX * dwDPI);
    p[Geom::Y] = (p[Geom::Y] * IN_PER_PX * dwDPI);

    LONG const xpos = (LONG) round(p[Geom::X]);
    LONG const ypos = (LONG) round(rc.bottom-p[Geom::Y]);

    if (PrintWin32::is_os_wide()) {
        gunichar2 *unicode_text = g_utf8_to_utf16( text, -1, NULL, NULL, NULL );
        TextOutW(hdc, xpos, ypos, (WCHAR*)unicode_text, wcslen((wchar_t*)unicode_text));
    }
    else {
        TextOutA(hdc, xpos, ypos, (CHAR*)text, strlen((char*)text));
    }

    SelectObject(hdc, hfontOld);
    DeleteObject(hfont);
    
    return 0;
}

void
PrintEmfWin32::init (void)
{
    Inkscape::Extension::Extension * ext;

    /* EMF print */
    ext = Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
        "<name>Enhanced Metafile Print</name>\n"
        "<id>org.inkscape.print.emf.win32</id>\n"
        "<param name=\"destination\" type=\"string\"></param>\n"
        "<param name=\"textToPath\" type=\"boolean\">true</param>\n"
        "<param name=\"pageBoundingBox\" type=\"boolean\">true</param>\n"
        "<print/>\n"
        "</inkscape-extension>", new PrintEmfWin32());

    return;
}


}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* WIN32 */

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
