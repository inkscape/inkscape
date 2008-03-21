/** \file
 * Enhanced Metafile Printing.
 */
/*
 * Authors:
 *   Ulf Erikson <ulferikson@users.sf.net>
 *
 * Copyright (C) 2006 Authors
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

#include <string.h>
#include <signal.h>
#include <errno.h>

#include "libnr/n-art-bpath.h"
#include "libnr/nr-point-matrix-ops.h"
#include "libnr/nr-rect.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-matrix-fns.h"
#include "libnr/nr-path.h"
#include "libnr/nr-pixblock.h"
#include "display/canvas-bpath.h"
#include "sp-item.h"

#include "glib.h"
#include "gtk/gtkdialog.h"
#include "gtk/gtkbox.h"
#include "gtk/gtkstock.h"

#include "glibmm/i18n.h"
#include "enums.h"
#include "document.h"
#include "style.h"
#include "sp-paint-server.h"
#include "inkscape_version.h"

#include "FontFactory.h"
#include "libnrtype/font-instance.h"
#include "libnrtype/font-style-to-pos.h"

#include "win32.h"
#include "emf-win32-print.h"

#include "unit-constants.h"

#include "extension/extension.h"
#include "extension/system.h"
#include "extension/print.h"

#include "io/sys.h"

#include "macros.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Inkscape {
namespace Extension {
namespace Internal {

static float dwDPI = 2540;


PrintEmfWin32::PrintEmfWin32 (void):
    hdc(NULL),
    hbrush(NULL),
    hbrushOld(NULL),
    hpen(NULL),
    fill_path(NULL)
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
PrintEmfWin32::setup (Inkscape::Extension::Print *mod)
{
    return TRUE;
}


unsigned int
PrintEmfWin32::begin (Inkscape::Extension::Print *mod, SPDocument *doc)
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
        sp_item_invoke_bbox(doc_item, &d, sp_item_i2r_affine(doc_item), TRUE);
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
    float PixelsX = (float) GetDeviceCaps( hScreenDC, HORZRES );
    float PixelsY = (float) GetDeviceCaps( hScreenDC, VERTRES );
    float MMX = (float) GetDeviceCaps( hScreenDC, HORZSIZE );
    float MMY = (float) GetDeviceCaps( hScreenDC, VERTSIZE );

    // Create the Metafile
    if (PrintWin32::is_os_wide())
        hdc = CreateEnhMetaFileW( hScreenDC, unicode_uri, &rc, NULL );
    else
        hdc = CreateEnhMetaFileA( hScreenDC, ansi_uri, &rc, NULL );

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
    SetWindowExtEx( hdc, (int) (dwInchesX*dwDPI), (int) (dwInchesY*dwDPI), NULL );

    // Set the viewport extent to reflect
    // dwInchesX" x dwInchesY" in device units
    SetViewportExtEx( hdc,
                      (int) ((float) dwInchesX*25.4f*PixelsX/MMX),
                      (int) ((float) dwInchesY*25.4f*PixelsY/MMY),
                      NULL );

    SetRect( &rc, 0, 0, (int) ceil(dwInchesX*dwDPI), (int) ceil(dwInchesY*dwDPI) );

    g_free(local_fn);
    g_free(unicode_fn);

    return 0;
}


unsigned int
PrintEmfWin32::finish (Inkscape::Extension::Print *mod)
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
PrintEmfWin32::comment (Inkscape::Extension::Print * module,
                                const char *comment)
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
PrintEmfWin32::create_pen(SPStyle const *style, const NR::Matrix *transform)
{
    if (style) {
        float rgb[3];

        sp_color_get_rgb_floatv(&style->stroke.value.color, rgb);

        LOGBRUSH lb = {0};
        lb.lbStyle = BS_SOLID;
        lb.lbColor = RGB( 255*rgb[0], 255*rgb[1], 255*rgb[2] );

        int linestyle = PS_SOLID;
        int linecap = 0;
        int linejoin = 0;
        DWORD n_dash = 0;
        DWORD *dash = NULL;
        float oldmiterlimit;

        using NR::X;
        using NR::Y;

        NR::Matrix tf = *transform;

        NR::Point zero(0, 0);
        NR::Point one(1, 1);
        NR::Point p0(zero * tf);
        NR::Point p1(one * tf);
        NR::Point p(p1 - p0);

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
            float miterlimit = style->stroke_miterlimit.value;
            if (miterlimit < 1)
                miterlimit = 4.0;
            SetMiterLimit(
                hdc,
                miterlimit * IN_PER_PX * dwDPI,
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
    if (fill_path) {
        print_bpath(fill_path, &fill_transform, &fill_pbox);
        FillPath( hdc );
        destroy_brush();
        delete[] fill_path;
        fill_path = NULL;
    }
}


NArtBpath *
PrintEmfWin32::copy_bpath(const NArtBpath *bp)
{
    NArtBpath *tmp = (NArtBpath *) bp;
    int num = 1;
    
    while (tmp->code != NR_END) {
        num++;
        tmp += 1;
    }

    tmp = new NArtBpath[num];
    while (num--) {
        tmp[num] = bp[num];
    }

    return tmp;
}


int
PrintEmfWin32::cmp_bpath(const NArtBpath *bp1, const NArtBpath *bp2)
{
    if (!bp1 || !bp2) {
        return 1;
    }
    
    while (bp1->code != NR_END && bp2->code != NR_END) {
        if (bp1->code != bp2->code) {
            return 1;
        }

        if ( fabs(bp1->x1 - bp2->x1) > 0.00000001 ||
             fabs(bp1->y1 - bp2->y1) > 0.00000001 ||
             fabs(bp1->x2 - bp2->x2) > 0.00000001 ||
             fabs(bp1->y2 - bp2->y2) > 0.00000001 ||
             fabs(bp1->x3 - bp2->x3) > 0.00000001 ||
             fabs(bp1->y3 - bp2->y3) > 0.00000001 )
        {
            return 1;
        }
        
        bp1 += 1;
        bp2 += 1;
    }
    
    return bp1->code != NR_END || bp2->code != NR_END;
}

unsigned int
PrintEmfWin32::bind(Inkscape::Extension::Print *mod, NR::Matrix const *transform, float opacity)
{
    text_transform = *transform;
    return 0;
}

unsigned int
PrintEmfWin32::release(Inkscape::Extension::Print *mod)
{
    return 0;
}

unsigned int
PrintEmfWin32::fill(Inkscape::Extension::Print *mod,
               NRBPath const *bpath, NR::Matrix const *transform, SPStyle const *style,
               NRRect const *pbox, NRRect const *dbox, NRRect const *bbox)
{
    if (!hdc) return 0;

    flush_fill(); // flush any pending fills

    if (style->fill.isColor()) {
        if (create_brush(style))
            return 0;
    } else {
        // create_brush(NULL);
        return 0;
    }

    fill_path = copy_bpath( bpath->path );
    fill_transform = *transform;
    fill_pbox = *pbox;

    // postpone fill in case of stroke-and-fill

    return 0;
}


unsigned int
PrintEmfWin32::stroke (Inkscape::Extension::Print *mod,
                  const NRBPath *bpath, const NR::Matrix *transform, const SPStyle *style,
                  const NRRect *pbox, const NRRect *dbox, const NRRect *bbox)
{
    if (!hdc) return 0;

    bool stroke_and_fill = ( cmp_bpath( bpath->path, fill_path ) == 0 );

    if (!stroke_and_fill) {
        flush_fill(); // flush any pending fills
    }

    if (style->stroke.isColor()) {
        create_pen(style, transform);
    } else {
        // create_pen(NULL, transform);
        return 0;
    }

    print_bpath(bpath->path, transform, pbox);

    if (stroke_and_fill) {
        StrokeAndFillPath( hdc );
        destroy_brush();
        delete[] fill_path;
        fill_path = NULL;
    } else {
        StrokePath( hdc );
    }

    destroy_pen();

    return 0;
}


unsigned int
PrintEmfWin32::print_bpath(const NArtBpath *bp, const NR::Matrix *transform, NRRect const *pbox)
{
    unsigned int closed;
    NR::Matrix tf = *transform;

    BeginPath( hdc );
    closed = FALSE;
    while (bp->code != NR_END) {
        using NR::X;
        using NR::Y;

        NR::Point p1(bp->c(1) * tf);
        NR::Point p2(bp->c(2) * tf);
        NR::Point p3(bp->c(3) * tf);

        p1[X] = (p1[X] * IN_PER_PX * dwDPI);
        p2[X] = (p2[X] * IN_PER_PX * dwDPI);
        p3[X] = (p3[X] * IN_PER_PX * dwDPI);
        p1[Y] = (p1[Y] * IN_PER_PX * dwDPI);
        p2[Y] = (p2[Y] * IN_PER_PX * dwDPI);
        p3[Y] = (p3[Y] * IN_PER_PX * dwDPI);

        LONG const x1 = (LONG) round(p1[X]);
        LONG const y1 = (LONG) round(rc.bottom-p1[Y]);
        LONG const x2 = (LONG) round(p2[X]);
        LONG const y2 = (LONG) round(rc.bottom-p2[Y]);
        LONG const x3 = (LONG) round(p3[X]);
        LONG const y3 = (LONG) round(rc.bottom-p3[Y]);

        switch (bp->code) {
            case NR_MOVETO:
                if (closed) {
                    CloseFigure( hdc );
                }
                closed = TRUE;
                MoveToEx( hdc, x3, y3, NULL );
                break;
            case NR_MOVETO_OPEN:
                if (closed) {
                    CloseFigure( hdc );
                }
                closed = FALSE;
                MoveToEx( hdc, x3, y3, NULL );
                break;
            case NR_LINETO:
                LineTo( hdc, x3, y3 );
                break;
            case NR_CURVETO:
            {
                POINT pt[3];
                pt[0].x = x1;
                pt[0].y = y1;
                pt[1].x = x2;
                pt[1].y = y2;
                pt[2].x = x3;
                pt[2].y = y3;

                PolyBezierTo( hdc, pt, 3 );
                break;
            }
            default:
                break;
        }
        bp += 1;
    }
    if (closed) {
        CloseFigure( hdc );
    }
    EndPath( hdc );

    return closed;
}


bool
PrintEmfWin32::textToPath(Inkscape::Extension::Print * ext)
{
    return ext->get_param_bool("textToPath");
}

unsigned int
PrintEmfWin32::text(Inkscape::Extension::Print *mod, char const *text, NR::Point p,
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
                style->font_weight.value == SP_CSS_FONT_WEIGHT_100 ? FW_THIN :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_200 ? FW_EXTRALIGHT :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_300 ? FW_LIGHT :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_400 ? FW_NORMAL :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_500 ? FW_MEDIUM :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_600 ? FW_SEMIBOLD :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_700 ? FW_BOLD :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_800 ? FW_EXTRABOLD :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_900 ? FW_HEAVY :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_NORMAL ? FW_NORMAL :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_BOLD ? FW_BOLD :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_LIGHTER ? FW_EXTRALIGHT :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_BOLDER ? FW_EXTRABOLD :
                FW_NORMAL;
            lf->lfItalic = (style->font_style.value == SP_CSS_FONT_STYLE_ITALIC);
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
                style->font_weight.value == SP_CSS_FONT_WEIGHT_100 ? FW_THIN :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_200 ? FW_EXTRALIGHT :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_300 ? FW_LIGHT :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_400 ? FW_NORMAL :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_500 ? FW_MEDIUM :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_600 ? FW_SEMIBOLD :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_700 ? FW_BOLD :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_800 ? FW_EXTRABOLD :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_900 ? FW_HEAVY :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_NORMAL ? FW_NORMAL :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_BOLD ? FW_BOLD :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_LIGHTER ? FW_EXTRALIGHT :
                style->font_weight.value == SP_CSS_FONT_WEIGHT_BOLDER ? FW_EXTRABOLD :
                FW_NORMAL;
            lf->lfItalic = (style->font_style.value == SP_CSS_FONT_STYLE_ITALIC);
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

    int align =
        style->text_align.value == SP_CSS_TEXT_ALIGN_RIGHT ? TA_RIGHT :
        style->text_align.value == SP_CSS_TEXT_ALIGN_CENTER ? TA_CENTER : TA_LEFT;
    SetTextAlign(hdc, TA_BASELINE | align);

    p = p * text_transform;
    p[NR::X] = (p[NR::X] * IN_PER_PX * dwDPI);
    p[NR::Y] = (p[NR::Y] * IN_PER_PX * dwDPI);

    if (PrintWin32::is_os_wide()) {
        gunichar2 *unicode_text = g_utf8_to_utf16( text, -1, NULL, NULL, NULL );
        TextOutW(hdc, p[NR::X], p[NR::Y], (WCHAR*)unicode_text, wcslen((wchar_t*)unicode_text));
    }
    else {
        TextOutA(hdc, p[NR::X], p[NR::Y], (CHAR*)text, strlen((char*)text));
    }

    SelectObject(hdc, hfontOld);
    DeleteObject(hfont);
    
    return 0;



    return 0;
}

void
PrintEmfWin32::init (void)
{
    Inkscape::Extension::Extension * ext;

    /* EMF print */
    ext = Inkscape::Extension::build_from_mem(
        "<inkscape-extension>\n"
        "<name>Enhanced Metafile Print</name>\n"
        "<id>org.inkscape.print.emf.win32</id>\n"
        "<param name=\"destination\" type=\"string\"></param>\n"
        "<param name=\"textToPath\" type=\"boolean\">TRUE</param>\n"
        "<param name=\"pageBoundingBox\" type=\"boolean\">TRUE</param>\n"
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
