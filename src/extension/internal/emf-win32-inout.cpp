/** \file
 * Enhanced Metafile Input and Output.
 */
/*
 * Authors:
 *   Ulf Erikson <ulferikson@users.sf.net>
 *
 * Copyright (C) 2006-2008 Authors
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

//#include "inkscape.h"
#include "sp-path.h"
#include "style.h"
//#include "color.h"
//#include "display/curve.h"
//#include "libnr/nr-point-matrix-ops.h"
//#include "gtk/gtk.h"
#include "print.h"
//#include "glibmm/i18n.h"
//#include "extension/extension.h"
#include "extension/system.h"
#include "extension/print.h"
#include "extension/db.h"
#include "extension/output.h"
//#include "document.h"
#include "display/nr-arena.h"
#include "display/nr-arena-item.h"

//#include "libnr/nr-rect.h"
//#include "libnr/nr-matrix.h"
//#include "libnr/nr-pixblock.h"

//#include <stdio.h>
//#include <string.h>

//#include <vector>
//#include <string>

//#include "io/sys.h"

#include "unit-constants.h"

#include "clear-n_.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "win32.h"
#include "emf-win32-print.h"
#include "emf-win32-inout.h"


#define PRINT_EMF_WIN32 "org.inkscape.print.emf.win32"

#ifndef PS_JOIN_MASK
#define PS_JOIN_MASK (PS_JOIN_BEVEL|PS_JOIN_MITER|PS_JOIN_ROUND)
#endif


namespace Inkscape {
namespace Extension {
namespace Internal {


EmfWin32::EmfWin32 (void) // The null constructor
{
    return;
}


EmfWin32::~EmfWin32 (void) //The destructor
{
    return;
}


bool
EmfWin32::check (Inkscape::Extension::Extension * /*module*/)
{
    if (NULL == Inkscape::Extension::db.get(PRINT_EMF_WIN32))
        return FALSE;
    return TRUE;
}


static void
emf_print_document_to_file(SPDocument *doc, gchar const *filename)
{
    Inkscape::Extension::Print *mod;
    SPPrintContext context;
    gchar const *oldconst;
    gchar *oldoutput;
    unsigned int ret;

    sp_document_ensure_up_to_date(doc);

    mod = Inkscape::Extension::get_print(PRINT_EMF_WIN32);
    oldconst = mod->get_param_string("destination");
    oldoutput = g_strdup(oldconst);
    mod->set_param_string("destination", (gchar *)filename);

/* Start */
    context.module = mod;
    /* fixme: This has to go into module constructor somehow */
    /* Create new arena */
    mod->base = SP_ITEM(sp_document_root(doc));
    mod->arena = NRArena::create();
    mod->dkey = sp_item_display_key_new(1);
    mod->root = sp_item_invoke_show(mod->base, mod->arena, mod->dkey, SP_ITEM_SHOW_DISPLAY);
    /* Print document */
    ret = mod->begin(doc);
    if (ret) {
        throw Inkscape::Extension::Output::save_failed();
    }
    sp_item_invoke_print(mod->base, &context);
    ret = mod->finish();
    /* Release arena */
    sp_item_invoke_hide(mod->base, mod->dkey);
    mod->base = NULL;
    mod->root = NULL;
    nr_object_unref((NRObject *) mod->arena);
    mod->arena = NULL;
/* end */

    mod->set_param_string("destination", oldoutput);
    g_free(oldoutput);

    return;
}


void
EmfWin32::save (Inkscape::Extension::Output *mod, SPDocument *doc, const gchar *uri)
{
    Inkscape::Extension::Extension * ext;

    ext = Inkscape::Extension::db.get(PRINT_EMF_WIN32);
    if (ext == NULL)
        return;

    bool old_textToPath  = ext->get_param_bool("textToPath");
    bool new_val         = mod->get_param_bool("textToPath");
    ext->set_param_bool("textToPath", new_val);

    gchar * final_name;
    final_name = g_strdup_printf("%s", uri);
    emf_print_document_to_file(doc, final_name);
    g_free(final_name);

    ext->set_param_bool("textToPath", old_textToPath);

    return;
}



typedef struct {
    int type;
    int level;
    ENHMETARECORD *lpEMFR;
} EMF_OBJECT, *PEMF_OBJECT;

typedef struct emf_device_context {
    struct SPStyle style;
    class SPTextStyle tstyle;
    bool stroke_set;
    bool fill_set;

    SIZEL sizeWnd;
    SIZEL sizeView;
    float PixelsInX, PixelsInY;
    float PixelsOutX, PixelsOutY;
    POINTL winorg;
    POINTL vieworg;
    double ScaleInX, ScaleInY;
    double ScaleOutX, ScaleOutY;
    COLORREF textColor;
    bool textColorSet;
    DWORD textAlign;
    XFORM worldTransform;
    POINTL cur;
} EMF_DEVICE_CONTEXT, *PEMF_DEVICE_CONTEXT;

#define EMF_MAX_DC 128

typedef struct emf_callback_data {
    Glib::ustring *outsvg;
    Glib::ustring *path;

    EMF_DEVICE_CONTEXT dc[EMF_MAX_DC+1]; // FIXME: This should be dynamic..
    int level;
    
    double xDPI, yDPI;
    bool pathless_stroke;
    bool inpath;

    float MMX;
    float MMY;
    float dwInchesX;
    float dwInchesY;

    unsigned int id;
    CHAR *pDesc;

    int n_obj;
    PEMF_OBJECT emf_obj;
} EMF_CALLBACK_DATA, *PEMF_CALLBACK_DATA;


static void
output_style(PEMF_CALLBACK_DATA d, int iType)
{
    SVGOStringStream tmp_id;
    SVGOStringStream tmp_style;
    char tmp[1024] = {0};

    float fill_rgb[3];
    sp_color_get_rgb_floatv( &(d->dc[d->level].style.fill.value.color), fill_rgb );
    
    float stroke_rgb[3];
    sp_color_get_rgb_floatv(&(d->dc[d->level].style.stroke.value.color), stroke_rgb);

    tmp_id << "\n\tid=\"" << (d->id++) << "\"";
    *(d->outsvg) += tmp_id.str().c_str();
    *(d->outsvg) += "\n\tstyle=\"";
    if (iType == EMR_STROKEPATH || !d->dc[d->level].fill_set) {
        tmp_style << "fill:none;";
    } else {
        snprintf(tmp, 1023,
                 "fill:#%02x%02x%02x;",
                 SP_COLOR_F_TO_U(fill_rgb[0]),
                 SP_COLOR_F_TO_U(fill_rgb[1]),
                 SP_COLOR_F_TO_U(fill_rgb[2]));
        tmp_style << tmp;
        snprintf(tmp, 1023,
                 "fill-rule:%s;",
                 d->dc[d->level].style.fill_rule.value == 0 ? "evenodd" : "nonzero");
        tmp_style << tmp;
        tmp_style << "fill-opacity:1;";

        if (d->dc[d->level].fill_set && d->dc[d->level].stroke_set && d->dc[d->level].style.stroke_width.value == 1 &&
            fill_rgb[0]==stroke_rgb[0] && fill_rgb[1]==stroke_rgb[1] && fill_rgb[2]==stroke_rgb[2])
        {
            d->dc[d->level].stroke_set = false;
        }
    }

    if (iType == EMR_FILLPATH || !d->dc[d->level].stroke_set) {
        tmp_style << "stroke:none;";
    } else {
        snprintf(tmp, 1023,
                 "stroke:#%02x%02x%02x;",
                 SP_COLOR_F_TO_U(stroke_rgb[0]),
                 SP_COLOR_F_TO_U(stroke_rgb[1]),
                 SP_COLOR_F_TO_U(stroke_rgb[2]));
        tmp_style << tmp;

        tmp_style << "stroke-width:" <<
            MAX( 0.001, d->dc[d->level].style.stroke_width.value ) << "px;";

        tmp_style << "stroke-linecap:" <<
            (d->dc[d->level].style.stroke_linecap.computed == 0 ? "butt" :
             d->dc[d->level].style.stroke_linecap.computed == 1 ? "round" :
             d->dc[d->level].style.stroke_linecap.computed == 2 ? "square" :
             "unknown") << ";";

        tmp_style << "stroke-linejoin:" <<
            (d->dc[d->level].style.stroke_linejoin.computed == 0 ? "miter" :
             d->dc[d->level].style.stroke_linejoin.computed == 1 ? "round" :
             d->dc[d->level].style.stroke_linejoin.computed == 2 ? "bevel" :
             "unknown") << ";";

        if (d->dc[d->level].style.stroke_linejoin.computed == 0) {
            tmp_style << "stroke-miterlimit:" <<
                MAX( 0.01, d->dc[d->level].style.stroke_miterlimit.value ) << ";";
        }

        if (d->dc[d->level].style.stroke_dasharray_set &&
            d->dc[d->level].style.stroke_dash.n_dash && d->dc[d->level].style.stroke_dash.dash)
        {
            tmp_style << "stroke-dasharray:";
            for (int i=0; i<d->dc[d->level].style.stroke_dash.n_dash; i++) {
                if (i)
                    tmp_style << ",";
                tmp_style << d->dc[d->level].style.stroke_dash.dash[i];
            }
            tmp_style << ";";
            tmp_style << "stroke-dashoffset:0;";
        } else {
            tmp_style << "stroke-dasharray:none;";
        }
        tmp_style << "stroke-opacity:1;";
    }
    tmp_style << "\" ";

    *(d->outsvg) += tmp_style.str().c_str();
}


static double
_pix_x_to_point(PEMF_CALLBACK_DATA d, double px)
{
    double tmp = px - d->dc[d->level].winorg.x;
    tmp *= d->dc[d->level].ScaleInX ? d->dc[d->level].ScaleInX : 1.0;
    tmp += d->dc[d->level].vieworg.x;
    return tmp;
}

static double
_pix_y_to_point(PEMF_CALLBACK_DATA d, double px)
{
    double tmp = px - d->dc[d->level].winorg.y;
    tmp *= d->dc[d->level].ScaleInY ? d->dc[d->level].ScaleInY : 1.0;
    tmp += d->dc[d->level].vieworg.y;
    return tmp;
}


static double
pix_to_x_point(PEMF_CALLBACK_DATA d, double px, double py)
{
    double ppx = _pix_x_to_point(d, px);
    double ppy = _pix_y_to_point(d, py);

    double x = ppx * d->dc[d->level].worldTransform.eM11 + ppy * d->dc[d->level].worldTransform.eM21 + d->dc[d->level].worldTransform.eDx;
    x *= d->dc[d->level].ScaleOutX ? d->dc[d->level].ScaleOutX : DEVICESCALE;
    
    return x;
}

static double
pix_to_y_point(PEMF_CALLBACK_DATA d, double px, double py)
{
    double ppx = _pix_x_to_point(d, px);
    double ppy = _pix_y_to_point(d, py);

    double y = ppx * d->dc[d->level].worldTransform.eM12 + ppy * d->dc[d->level].worldTransform.eM22 + d->dc[d->level].worldTransform.eDy;
    y *= d->dc[d->level].ScaleOutY ? d->dc[d->level].ScaleOutY : DEVICESCALE;
    
    return y;
}

static double
pix_to_size_point(PEMF_CALLBACK_DATA d, double px)
{
    double ppx = px * (d->dc[d->level].ScaleInX ? d->dc[d->level].ScaleInX : 1.0);
    double ppy = 0;

    double dx = ppx * d->dc[d->level].worldTransform.eM11 + ppy * d->dc[d->level].worldTransform.eM21;
    dx *= d->dc[d->level].ScaleOutX ? d->dc[d->level].ScaleOutX : DEVICESCALE;
    double dy = ppx * d->dc[d->level].worldTransform.eM12 + ppy * d->dc[d->level].worldTransform.eM22;
    dy *= d->dc[d->level].ScaleOutY ? d->dc[d->level].ScaleOutY : DEVICESCALE;

    double tmp = sqrt(dx * dx + dy * dy);
    return tmp;
}


static void
select_pen(PEMF_CALLBACK_DATA d, int index)
{
    PEMRCREATEPEN pEmr = NULL;

    if (index >= 0 && index < d->n_obj)
        pEmr = (PEMRCREATEPEN) d->emf_obj[index].lpEMFR;

    if (!pEmr)
        return;

    switch (pEmr->lopn.lopnStyle & PS_STYLE_MASK) {
        case PS_DASH:
        case PS_DOT:
        case PS_DASHDOT:
        case PS_DASHDOTDOT:
        {
            int i = 0;
            int penstyle = (pEmr->lopn.lopnStyle & PS_STYLE_MASK);
            d->dc[d->level].style.stroke_dash.n_dash =
                penstyle == PS_DASHDOTDOT ? 6 : penstyle == PS_DASHDOT ? 4 : 2;
            if (d->dc[d->level].style.stroke_dash.dash && (d->level==0 || (d->level>0 && d->dc[d->level].style.stroke_dash.dash!=d->dc[d->level-1].style.stroke_dash.dash)))
                delete[] d->dc[d->level].style.stroke_dash.dash;
            d->dc[d->level].style.stroke_dash.dash = new double[d->dc[d->level].style.stroke_dash.n_dash];
            if (penstyle==PS_DASH || penstyle==PS_DASHDOT || penstyle==PS_DASHDOTDOT) {
                d->dc[d->level].style.stroke_dash.dash[i++] = 3;
                d->dc[d->level].style.stroke_dash.dash[i++] = 1;
            }
            if (penstyle==PS_DOT || penstyle==PS_DASHDOT || penstyle==PS_DASHDOTDOT) {
                d->dc[d->level].style.stroke_dash.dash[i++] = 1;
                d->dc[d->level].style.stroke_dash.dash[i++] = 1;
            }
            if (penstyle==PS_DASHDOTDOT) {
                d->dc[d->level].style.stroke_dash.dash[i++] = 1;
                d->dc[d->level].style.stroke_dash.dash[i++] = 1;
            }
            
            d->dc[d->level].style.stroke_dasharray_set = 1;
            break;
        }
        
        case PS_SOLID:
        default:
        {
            d->dc[d->level].style.stroke_dasharray_set = 0;
            break;
        }
    }

    switch (pEmr->lopn.lopnStyle & PS_ENDCAP_MASK) {
        case PS_ENDCAP_ROUND:
        {
            d->dc[d->level].style.stroke_linecap.computed = 1;
            break;
        }
        case PS_ENDCAP_SQUARE:
        {
            d->dc[d->level].style.stroke_linecap.computed = 2;
            break;
        }
        case PS_ENDCAP_FLAT:
        default:
        {
            d->dc[d->level].style.stroke_linecap.computed = 0;
            break;
        }
    }

    switch (pEmr->lopn.lopnStyle & PS_JOIN_MASK) {
        case PS_JOIN_BEVEL:
        {
            d->dc[d->level].style.stroke_linejoin.computed = 2;
            break;
        }
        case PS_JOIN_MITER:
        {
            d->dc[d->level].style.stroke_linejoin.computed = 0;
            break;
        }
        case PS_JOIN_ROUND:
        default:
        {
            d->dc[d->level].style.stroke_linejoin.computed = 1;
            break;
        }
    }

    d->dc[d->level].stroke_set = true;

    if (pEmr->lopn.lopnStyle == PS_NULL) {
        d->dc[d->level].style.stroke_width.value = 0;
        d->dc[d->level].stroke_set = false;
    } else if (pEmr->lopn.lopnWidth.x) {
        int cur_level = d->level;
        d->level = d->emf_obj[index].level;
        double pen_width = pix_to_size_point( d, pEmr->lopn.lopnWidth.x );
        d->level = cur_level;
        d->dc[d->level].style.stroke_width.value = pen_width;
    } else { // this stroke should always be rendered as 1 pixel wide, independent of zoom level (can that be done in SVG?)
        //d->dc[d->level].style.stroke_width.value = 1.0;
        int cur_level = d->level;
        d->level = d->emf_obj[index].level;
        double pen_width = pix_to_size_point( d, 1 );
        d->level = cur_level;
        d->dc[d->level].style.stroke_width.value = pen_width;
    }

    double r, g, b;
    r = SP_COLOR_U_TO_F( GetRValue(pEmr->lopn.lopnColor) );
    g = SP_COLOR_U_TO_F( GetGValue(pEmr->lopn.lopnColor) );
    b = SP_COLOR_U_TO_F( GetBValue(pEmr->lopn.lopnColor) );
    d->dc[d->level].style.stroke.value.color.set( r, g, b );
}


static void
select_extpen(PEMF_CALLBACK_DATA d, int index)
{
    PEMREXTCREATEPEN pEmr = NULL;

    if (index >= 0 && index < d->n_obj)
        pEmr = (PEMREXTCREATEPEN) d->emf_obj[index].lpEMFR;

    if (!pEmr)
        return;

    switch (pEmr->elp.elpPenStyle & PS_STYLE_MASK) {
        case PS_USERSTYLE:
        {
            if (pEmr->elp.elpNumEntries) {
                d->dc[d->level].style.stroke_dash.n_dash = pEmr->elp.elpNumEntries;
                if (d->dc[d->level].style.stroke_dash.dash && (d->level==0 || (d->level>0 && d->dc[d->level].style.stroke_dash.dash!=d->dc[d->level-1].style.stroke_dash.dash)))
                    delete[] d->dc[d->level].style.stroke_dash.dash;
                d->dc[d->level].style.stroke_dash.dash = new double[pEmr->elp.elpNumEntries];
                for (unsigned int i=0; i<pEmr->elp.elpNumEntries; i++) {
                    int cur_level = d->level;
                    d->level = d->emf_obj[index].level;
                    double dash_length = pix_to_size_point( d, pEmr->elp.elpStyleEntry[i] );
                    d->level = cur_level;
                    d->dc[d->level].style.stroke_dash.dash[i] = dash_length;
                }
                d->dc[d->level].style.stroke_dasharray_set = 1;
            } else {
                d->dc[d->level].style.stroke_dasharray_set = 0;
            }
            break;
        }

        case PS_DASH:
        case PS_DOT:
        case PS_DASHDOT:
        case PS_DASHDOTDOT:
        {
            int i = 0;
            int penstyle = (pEmr->elp.elpPenStyle & PS_STYLE_MASK);
            d->dc[d->level].style.stroke_dash.n_dash =
                penstyle == PS_DASHDOTDOT ? 6 : penstyle == PS_DASHDOT ? 4 : 2;
            if (d->dc[d->level].style.stroke_dash.dash && (d->level==0 || (d->level>0 && d->dc[d->level].style.stroke_dash.dash!=d->dc[d->level-1].style.stroke_dash.dash)))
                delete[] d->dc[d->level].style.stroke_dash.dash;
            d->dc[d->level].style.stroke_dash.dash = new double[d->dc[d->level].style.stroke_dash.n_dash];
            if (penstyle==PS_DASH || penstyle==PS_DASHDOT || penstyle==PS_DASHDOTDOT) {
                d->dc[d->level].style.stroke_dash.dash[i++] = 3;
                d->dc[d->level].style.stroke_dash.dash[i++] = 2;
            }
            if (penstyle==PS_DOT || penstyle==PS_DASHDOT || penstyle==PS_DASHDOTDOT) {
                d->dc[d->level].style.stroke_dash.dash[i++] = 1;
                d->dc[d->level].style.stroke_dash.dash[i++] = 2;
            }
            if (penstyle==PS_DASHDOTDOT) {
                d->dc[d->level].style.stroke_dash.dash[i++] = 1;
                d->dc[d->level].style.stroke_dash.dash[i++] = 2;
            }
            
            d->dc[d->level].style.stroke_dasharray_set = 1;
            break;
        }
        
        case PS_SOLID:
        default:
        {
            d->dc[d->level].style.stroke_dasharray_set = 0;
            break;
        }
    }

    switch (pEmr->elp.elpPenStyle & PS_ENDCAP_MASK) {
        case PS_ENDCAP_ROUND:
        {
            d->dc[d->level].style.stroke_linecap.computed = 1;
            break;
        }
        case PS_ENDCAP_SQUARE:
        {
            d->dc[d->level].style.stroke_linecap.computed = 2;
            break;
        }
        case PS_ENDCAP_FLAT:
        default:
        {
            d->dc[d->level].style.stroke_linecap.computed = 0;
            break;
        }
    }

    switch (pEmr->elp.elpPenStyle & PS_JOIN_MASK) {
        case PS_JOIN_BEVEL:
        {
            d->dc[d->level].style.stroke_linejoin.computed = 2;
            break;
        }
        case PS_JOIN_MITER:
        {
            d->dc[d->level].style.stroke_linejoin.computed = 0;
            break;
        }
        case PS_JOIN_ROUND:
        default:
        {
            d->dc[d->level].style.stroke_linejoin.computed = 1;
            break;
        }
    }

    d->dc[d->level].stroke_set = true;

    if (pEmr->elp.elpPenStyle == PS_NULL) {
        d->dc[d->level].style.stroke_width.value = 0;
        d->dc[d->level].stroke_set = false;
    } else if (pEmr->elp.elpWidth) {
        int cur_level = d->level;
        d->level = d->emf_obj[index].level;
        double pen_width = pix_to_size_point( d, pEmr->elp.elpWidth );
        d->level = cur_level;
        d->dc[d->level].style.stroke_width.value = pen_width;
    } else { // this stroke should always be rendered as 1 pixel wide, independent of zoom level (can that be done in SVG?)
        //d->dc[d->level].style.stroke_width.value = 1.0;
        int cur_level = d->level;
        d->level = d->emf_obj[index].level;
        double pen_width = pix_to_size_point( d, 1 );
        d->level = cur_level;
        d->dc[d->level].style.stroke_width.value = pen_width;
    }

    double r, g, b;
    r = SP_COLOR_U_TO_F( GetRValue(pEmr->elp.elpColor) );
    g = SP_COLOR_U_TO_F( GetGValue(pEmr->elp.elpColor) );
    b = SP_COLOR_U_TO_F( GetBValue(pEmr->elp.elpColor) );

    d->dc[d->level].style.stroke.value.color.set( r, g, b );
}


static void
select_brush(PEMF_CALLBACK_DATA d, int index)
{
    PEMRCREATEBRUSHINDIRECT pEmr = NULL;

    if (index >= 0 && index < d->n_obj)
        pEmr = (PEMRCREATEBRUSHINDIRECT) d->emf_obj[index].lpEMFR;

    if (!pEmr)
        return;

    if (pEmr->lb.lbStyle == BS_SOLID) {
        double r, g, b;
        r = SP_COLOR_U_TO_F( GetRValue(pEmr->lb.lbColor) );
        g = SP_COLOR_U_TO_F( GetGValue(pEmr->lb.lbColor) );
        b = SP_COLOR_U_TO_F( GetBValue(pEmr->lb.lbColor) );
        d->dc[d->level].style.fill.value.color.set( r, g, b );
    }

    d->dc[d->level].fill_set = true;
}


static void
select_font(PEMF_CALLBACK_DATA d, int index)
{
    PEMREXTCREATEFONTINDIRECTW pEmr = NULL;

    if (index >= 0 && index < d->n_obj)
        pEmr = (PEMREXTCREATEFONTINDIRECTW) d->emf_obj[index].lpEMFR;

    if (!pEmr)
        return;

    int cur_level = d->level;
    d->level = d->emf_obj[index].level;
    double font_size = pix_to_size_point( d, pEmr->elfw.elfLogFont.lfHeight );
    d->level = cur_level;
    d->dc[d->level].style.font_size.computed = font_size;
    d->dc[d->level].style.font_weight.value =
        pEmr->elfw.elfLogFont.lfWeight == FW_THIN ? SP_CSS_FONT_WEIGHT_100 :
        pEmr->elfw.elfLogFont.lfWeight == FW_EXTRALIGHT ? SP_CSS_FONT_WEIGHT_200 :
        pEmr->elfw.elfLogFont.lfWeight == FW_LIGHT ? SP_CSS_FONT_WEIGHT_300 :
        pEmr->elfw.elfLogFont.lfWeight == FW_NORMAL ? SP_CSS_FONT_WEIGHT_400 :
        pEmr->elfw.elfLogFont.lfWeight == FW_MEDIUM ? SP_CSS_FONT_WEIGHT_500 :
        pEmr->elfw.elfLogFont.lfWeight == FW_SEMIBOLD ? SP_CSS_FONT_WEIGHT_600 :
        pEmr->elfw.elfLogFont.lfWeight == FW_BOLD ? SP_CSS_FONT_WEIGHT_700 :
        pEmr->elfw.elfLogFont.lfWeight == FW_EXTRABOLD ? SP_CSS_FONT_WEIGHT_800 :
        pEmr->elfw.elfLogFont.lfWeight == FW_HEAVY ? SP_CSS_FONT_WEIGHT_900 :
        pEmr->elfw.elfLogFont.lfWeight == FW_NORMAL ? SP_CSS_FONT_WEIGHT_NORMAL :
        pEmr->elfw.elfLogFont.lfWeight == FW_BOLD ? SP_CSS_FONT_WEIGHT_BOLD :
        pEmr->elfw.elfLogFont.lfWeight == FW_EXTRALIGHT ? SP_CSS_FONT_WEIGHT_LIGHTER :
        pEmr->elfw.elfLogFont.lfWeight == FW_EXTRABOLD ? SP_CSS_FONT_WEIGHT_BOLDER :
        FW_NORMAL;
    d->dc[d->level].style.font_style.value = (pEmr->elfw.elfLogFont.lfItalic ? SP_CSS_FONT_STYLE_ITALIC : SP_CSS_FONT_STYLE_NORMAL);
    d->dc[d->level].style.text_decoration.underline = pEmr->elfw.elfLogFont.lfUnderline;
    d->dc[d->level].style.text_decoration.line_through = pEmr->elfw.elfLogFont.lfStrikeOut;
    if (d->dc[d->level].tstyle.font_family.value)
        g_free(d->dc[d->level].tstyle.font_family.value);
    d->dc[d->level].tstyle.font_family.value =
        (gchar *) g_utf16_to_utf8( (gunichar2*) pEmr->elfw.elfLogFont.lfFaceName, -1, NULL, NULL, NULL );
    d->dc[d->level].style.text_transform.value = ((pEmr->elfw.elfLogFont.lfEscapement + 3600) % 3600) / 10;
}

static void
delete_object(PEMF_CALLBACK_DATA d, int index)
{
    if (index >= 0 && index < d->n_obj) {
        d->emf_obj[index].type = 0;
        if (d->emf_obj[index].lpEMFR)
            free(d->emf_obj[index].lpEMFR);
        d->emf_obj[index].lpEMFR = NULL;
    }
}


static void
insert_object(PEMF_CALLBACK_DATA d, int index, int type, ENHMETARECORD *pObj)
{
    if (index >= 0 && index < d->n_obj) {
        delete_object(d, index);
        d->emf_obj[index].type = type;
        d->emf_obj[index].level = d->level;
        d->emf_obj[index].lpEMFR = pObj;
    }
}

static void
assert_empty_path(PEMF_CALLBACK_DATA d, const char * /*fun*/)
{
    if (!d->path->empty()) {
        // g_debug("emf-win32-inout: assert_empty_path failed for %s\n", fun);

        *(d->outsvg) += "<!--\n";
        *(d->outsvg) += "    <path \t";
        output_style(d, EMR_STROKEPATH);
        if (strstr(d->path->c_str(), "d=\"") == NULL) {
            *(d->outsvg) += "d=\"";
            *(d->outsvg) += "\n\t";
        }
        *(d->outsvg) += *(d->path);
        *(d->outsvg) += " \" /> \n";
        *(d->outsvg) += "-->\n";

        *(d->path) = "";
    }
}


static int CALLBACK
myEnhMetaFileProc(HDC /*hDC*/, HANDLETABLE * /*lpHTable*/, ENHMETARECORD *lpEMFR, int /*nObj*/, LPARAM lpData)
{
    PEMF_CALLBACK_DATA d;
    SVGOStringStream tmp_outsvg;
    SVGOStringStream tmp_path;
    SVGOStringStream tmp_str;
    SVGOStringStream dbg_str;

    d = (PEMF_CALLBACK_DATA) lpData;

    if (d->pathless_stroke) {
        if (lpEMFR->iType!=EMR_POLYBEZIERTO && lpEMFR->iType!=EMR_POLYBEZIERTO16 &&
            lpEMFR->iType!=EMR_POLYLINETO && lpEMFR->iType!=EMR_POLYLINETO16 &&
            lpEMFR->iType!=EMR_LINETO && lpEMFR->iType!=EMR_ARCTO &&
            lpEMFR->iType!=EMR_SETBKCOLOR && lpEMFR->iType!=EMR_SETROP2 &&
            lpEMFR->iType!=EMR_SETBKMODE)
        {
            *(d->outsvg) += "    <path ";
            output_style(d, EMR_STROKEPATH);
            *(d->outsvg) += "\n\t";
            *(d->outsvg) += *(d->path);
            *(d->outsvg) += " \" /> \n";
            *(d->path) = "";
            d->pathless_stroke = false;
        }
    }

    switch (lpEMFR->iType)
    {
        case EMR_HEADER:
        {
            dbg_str << "<!-- EMR_HEADER -->\n";

            *(d->outsvg) += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";

            if (d->pDesc) {
                *(d->outsvg) += "<!-- ";
                *(d->outsvg) += d->pDesc;
                *(d->outsvg) += " -->\n";
            }

            ENHMETAHEADER *pEmr = (ENHMETAHEADER *) lpEMFR;
            tmp_outsvg << "<svg\n";
            tmp_outsvg << "  xmlns:svg=\"http://www.w3.org/2000/svg\"\n";
            tmp_outsvg << "  xmlns=\"http://www.w3.org/2000/svg\"\n";
            tmp_outsvg << "  version=\"1.0\"\n";

            d->xDPI = 2540;
            d->yDPI = 2540;

            d->dc[d->level].PixelsInX = pEmr->rclFrame.right - pEmr->rclFrame.left;
            d->dc[d->level].PixelsInY = pEmr->rclFrame.bottom - pEmr->rclFrame.top;

            d->MMX = d->dc[d->level].PixelsInX / 100.0;
            d->MMY = d->dc[d->level].PixelsInY / 100.0;

            d->dc[d->level].PixelsOutX = d->MMX * PX_PER_MM;
            d->dc[d->level].PixelsOutY = d->MMY * PX_PER_MM;
            
            tmp_outsvg <<
                "  width=\"" << d->MMX << "mm\"\n" <<
                "  height=\"" << d->MMY << "mm\"\n";
            tmp_outsvg <<
                "  id=\"" << (d->id++) << "\">\n";

            tmp_outsvg <<
                "<g\n" <<
                "  id=\"" << (d->id++) << "\">\n";

            if (pEmr->nHandles) {
                d->n_obj = pEmr->nHandles;
                d->emf_obj = new EMF_OBJECT[d->n_obj];
                
                // Init the new emf_obj list elements to null, provided the
                // dynamic allocation succeeded.
                if ( d->emf_obj != NULL )
                {
                    for( int i=0; i < d->n_obj; ++i )
                        d->emf_obj[i].lpEMFR = NULL;
                } //if

            } else {
                d->emf_obj = NULL;
            }

            break;
        }
        case EMR_POLYBEZIER:
        {
            dbg_str << "<!-- EMR_POLYBEZIER -->\n";

            PEMRPOLYBEZIER pEmr = (PEMRPOLYBEZIER) lpEMFR;
            DWORD i,j;

            if (pEmr->cptl<4)
                break;

            if (!d->inpath) {
                assert_empty_path(d, "EMR_POLYBEZIER");

                *(d->outsvg) += "    <path ";
                output_style(d, EMR_STROKEPATH);
                *(d->outsvg) += "\n\td=\"";
            }

            tmp_str <<
                "\n\tM " <<
                pix_to_x_point( d, pEmr->aptl[0].x, pEmr->aptl[0].y ) << " " <<
                pix_to_y_point( d, pEmr->aptl[0].x, pEmr->aptl[0].y) << " ";

            for (i=1; i<pEmr->cptl; ) {
                tmp_str << "\n\tC ";
                for (j=0; j<3 && i<pEmr->cptl; j++,i++) {
                    tmp_str <<
                        pix_to_x_point( d, pEmr->aptl[i].x, pEmr->aptl[i].y ) << " " <<
                        pix_to_y_point( d, pEmr->aptl[i].x, pEmr->aptl[i].y ) << " ";
                }
            }

            if (d->inpath) {
                tmp_path << tmp_str.str().c_str();
            }
            else {
                *(d->outsvg) += tmp_str.str().c_str();
                *(d->outsvg) += " \" /> \n";
            }

            break;
        }
        case EMR_POLYGON:
        {
            dbg_str << "<!-- EMR_POLYGON -->\n";

            EMRPOLYGON *pEmr = (EMRPOLYGON *) lpEMFR;
            DWORD i;

            if (pEmr->cptl < 2)
                break;

            assert_empty_path(d, "EMR_POLYGON");

            *(d->outsvg) += "    <path ";
            output_style(d, EMR_STROKEANDFILLPATH);
            *(d->outsvg) += "\n\td=\"";

            tmp_str <<
                "\n\tM " <<
                pix_to_x_point( d, pEmr->aptl[0].x, pEmr->aptl[0].y ) << " " <<
                pix_to_y_point( d, pEmr->aptl[0].x, pEmr->aptl[0].y ) << " ";

            for (i=1; i<pEmr->cptl; i++) {
                tmp_str <<
                    "\n\tL " <<
                    pix_to_x_point( d, pEmr->aptl[i].x, pEmr->aptl[i].y ) << " " <<
                    pix_to_y_point( d, pEmr->aptl[i].x, pEmr->aptl[i].y ) << " ";
            }

            *(d->outsvg) += tmp_str.str().c_str();
            *(d->outsvg) += " z \" /> \n";

            break;
        }
        case EMR_POLYLINE:
        {
            dbg_str << "<!-- EMR_POLYLINE -->\n";

            EMRPOLYLINE *pEmr = (EMRPOLYLINE *) lpEMFR;
            DWORD i;

            if (pEmr->cptl<2)
                break;

            if (!d->inpath) {
                assert_empty_path(d, "EMR_POLYLINE");

                *(d->outsvg) += "    <path ";
                output_style(d, EMR_STROKEPATH);
                *(d->outsvg) += "\n\td=\"";
            }

            tmp_str <<
                "\n\tM " <<
                pix_to_x_point( d, pEmr->aptl[0].x, pEmr->aptl[0].y ) << " " <<
                pix_to_y_point( d, pEmr->aptl[0].x, pEmr->aptl[0].y ) << " ";

            for (i=1; i<pEmr->cptl; i++) {
                tmp_str <<
                    "\n\tL " <<
                    pix_to_x_point( d, pEmr->aptl[i].x, pEmr->aptl[i].y ) << " " <<
                    pix_to_y_point( d, pEmr->aptl[i].x, pEmr->aptl[i].y ) << " ";
            }

            if (d->inpath) {
                tmp_path << tmp_str.str().c_str();
            }
            else {
                *(d->outsvg) += tmp_str.str().c_str();
                *(d->outsvg) += " \" /> \n";
            }

            break;
        }
        case EMR_POLYBEZIERTO:
        {
            dbg_str << "<!-- EMR_POLYBEZIERTO -->\n";

            PEMRPOLYBEZIERTO pEmr = (PEMRPOLYBEZIERTO) lpEMFR;
            DWORD i,j;

            if (d->path->empty()) {
                d->pathless_stroke = true;
                *(d->path) = "d=\"";
            }

            for (i=0; i<pEmr->cptl;) {
                tmp_path << "\n\tC ";
                for (j=0; j<3 && i<pEmr->cptl; j++,i++) {
                    tmp_path <<
                        pix_to_x_point( d, pEmr->aptl[i].x, pEmr->aptl[i].y ) << " " <<
                        pix_to_y_point( d, pEmr->aptl[i].x, pEmr->aptl[i].y ) << " ";
                }
            }

            break;
        }
        case EMR_POLYLINETO:
        {
            dbg_str << "<!-- EMR_POLYLINETO -->\n";

            PEMRPOLYLINETO pEmr = (PEMRPOLYLINETO) lpEMFR;
            DWORD i;

            if (d->path->empty()) {
                d->pathless_stroke = true;
                *(d->path) = "d=\"";
            }

            for (i=0; i<pEmr->cptl;i++) {
                tmp_path <<
                    "\n\tL " <<
                    pix_to_x_point( d, pEmr->aptl[i].x, pEmr->aptl[i].y ) << " " <<
                    pix_to_y_point( d, pEmr->aptl[i].x, pEmr->aptl[i].y ) << " ";
            }

            break;
        }
        case EMR_POLYPOLYLINE:
        case EMR_POLYPOLYGON:
        {
            if (lpEMFR->iType == EMR_POLYPOLYLINE)
                dbg_str << "<!-- EMR_POLYPOLYLINE -->\n";
            if (lpEMFR->iType == EMR_POLYPOLYGON)
                dbg_str << "<!-- EMR_POLYPOLYGON -->\n";

            PEMRPOLYPOLYGON pEmr = (PEMRPOLYPOLYGON) lpEMFR;
            unsigned int n, i, j;

            if (!d->inpath) {
                assert_empty_path(d, lpEMFR->iType == EMR_POLYPOLYGON ? "EMR_POLYPOLYGON" : "EMR_POLYPOLYLINE");

                *(d->outsvg) += "    <path ";
                output_style(d, lpEMFR->iType==EMR_POLYPOLYGON ? EMR_STROKEANDFILLPATH : EMR_STROKEPATH);
                *(d->outsvg) += "\n\td=\"";
            }

            POINTL *aptl = (POINTL *) &pEmr->aPolyCounts[pEmr->nPolys];

            i = 0;
            for (n=0; n<pEmr->nPolys && i<pEmr->cptl; n++) {
                SVGOStringStream poly_path;

                poly_path << "\n\tM " <<
                    pix_to_x_point( d, aptl[i].x, aptl[i].y ) << " " <<
                    pix_to_y_point( d, aptl[i].x, aptl[i].y ) << " ";
                i++;

                for (j=1; j<pEmr->aPolyCounts[n] && i<pEmr->cptl; j++) {
                    poly_path << "\n\tL " <<
                        pix_to_x_point( d, aptl[i].x, aptl[i].y ) << " " <<
                        pix_to_y_point( d, aptl[i].x, aptl[i].y ) << " ";
                    i++;
                }

                tmp_str << poly_path.str().c_str();
                if (lpEMFR->iType == EMR_POLYPOLYGON)
                    tmp_str << " z";
                tmp_str << " \n";
            }

            if (d->inpath) {
                tmp_path << tmp_str.str().c_str();
            }
            else {
                *(d->outsvg) += tmp_str.str().c_str();
                *(d->outsvg) += " \" /> \n";
            }

            break;
        }
        case EMR_SETWINDOWEXTEX:
        {
            dbg_str << "<!-- EMR_SETWINDOWEXTEX -->\n";

            PEMRSETWINDOWEXTEX pEmr = (PEMRSETWINDOWEXTEX) lpEMFR;

            d->dc[d->level].sizeWnd = pEmr->szlExtent;

            if (!d->dc[d->level].sizeWnd.cx || !d->dc[d->level].sizeWnd.cy) {
                d->dc[d->level].sizeWnd = d->dc[d->level].sizeView;
                if (!d->dc[d->level].sizeWnd.cx || !d->dc[d->level].sizeWnd.cy) {
                    d->dc[d->level].sizeWnd.cx = d->dc[d->level].PixelsOutX;
                    d->dc[d->level].sizeWnd.cy = d->dc[d->level].PixelsOutY;
                }
            }

            if (!d->dc[d->level].sizeView.cx || !d->dc[d->level].sizeView.cy) {
                d->dc[d->level].sizeView = d->dc[d->level].sizeWnd;
            }

            d->dc[d->level].PixelsInX = d->dc[d->level].sizeWnd.cx;
            d->dc[d->level].PixelsInY = d->dc[d->level].sizeWnd.cy;
            
            if (d->dc[d->level].PixelsInX && d->dc[d->level].PixelsInY) {
                d->dc[d->level].ScaleInX = (double) d->dc[d->level].sizeView.cx / (double) d->dc[d->level].PixelsInX;
                d->dc[d->level].ScaleInY = (double) d->dc[d->level].sizeView.cy / (double) d->dc[d->level].PixelsInY;
            }
            else {
                d->dc[d->level].ScaleInX = 1;
                d->dc[d->level].ScaleInY = 1;
            }

            if (d->dc[d->level].sizeView.cx && d->dc[d->level].sizeView.cy) {
                d->dc[d->level].ScaleOutX = (double) d->dc[d->level].PixelsOutX / (double) d->dc[d->level].sizeView.cx;
                d->dc[d->level].ScaleOutY = (double) d->dc[d->level].PixelsOutY / (double) d->dc[d->level].sizeView.cy;
            }
            else {
                d->dc[d->level].ScaleOutX = DEVICESCALE;
                d->dc[d->level].ScaleOutY = DEVICESCALE;
            }

            break;
        }
        case EMR_SETWINDOWORGEX:
        {
            dbg_str << "<!-- EMR_SETWINDOWORGEX -->\n";

            PEMRSETWINDOWORGEX pEmr = (PEMRSETWINDOWORGEX) lpEMFR;
            d->dc[d->level].winorg = pEmr->ptlOrigin;
            break;
        }
        case EMR_SETVIEWPORTEXTEX:
        {
            dbg_str << "<!-- EMR_SETVIEWPORTEXTEX -->\n";

            PEMRSETVIEWPORTEXTEX pEmr = (PEMRSETVIEWPORTEXTEX) lpEMFR;

            d->dc[d->level].sizeView = pEmr->szlExtent;

            if (!d->dc[d->level].sizeView.cx || !d->dc[d->level].sizeView.cy) {
                d->dc[d->level].sizeView = d->dc[d->level].sizeWnd;
                if (!d->dc[d->level].sizeView.cx || !d->dc[d->level].sizeView.cy) {
                    d->dc[d->level].sizeView.cx = d->dc[d->level].PixelsOutX;
                    d->dc[d->level].sizeView.cy = d->dc[d->level].PixelsOutY;
                }
            }

            if (!d->dc[d->level].sizeWnd.cx || !d->dc[d->level].sizeWnd.cy) {
                d->dc[d->level].sizeWnd = d->dc[d->level].sizeView;
            }

            d->dc[d->level].PixelsInX = d->dc[d->level].sizeWnd.cx;
            d->dc[d->level].PixelsInY = d->dc[d->level].sizeWnd.cy;
            
            if (d->dc[d->level].PixelsInX && d->dc[d->level].PixelsInY) {
                d->dc[d->level].ScaleInX = (double) d->dc[d->level].sizeView.cx / (double) d->dc[d->level].PixelsInX;
                d->dc[d->level].ScaleInY = (double) d->dc[d->level].sizeView.cy / (double) d->dc[d->level].PixelsInY;
            }
            else {
                d->dc[d->level].ScaleInX = 1;
                d->dc[d->level].ScaleInY = 1;
            }

            if (d->dc[d->level].sizeView.cx && d->dc[d->level].sizeView.cy) {
                d->dc[d->level].ScaleOutX = (double) d->dc[d->level].PixelsOutX / (double) d->dc[d->level].sizeView.cx;
                d->dc[d->level].ScaleOutY = (double) d->dc[d->level].PixelsOutY / (double) d->dc[d->level].sizeView.cy;
            }
            else {
                d->dc[d->level].ScaleOutX = DEVICESCALE;
                d->dc[d->level].ScaleOutY = DEVICESCALE;
            }

            break;
        }
        case EMR_SETVIEWPORTORGEX:
        {
            dbg_str << "<!-- EMR_SETVIEWPORTORGEX -->\n";

            PEMRSETVIEWPORTORGEX pEmr = (PEMRSETVIEWPORTORGEX) lpEMFR;
            d->dc[d->level].vieworg = pEmr->ptlOrigin;
            break;
        }
        case EMR_SETBRUSHORGEX:
            dbg_str << "<!-- EMR_SETBRUSHORGEX -->\n";
            break;
        case EMR_EOF:
        {
            dbg_str << "<!-- EMR_EOF -->\n";

            assert_empty_path(d, "EMR_EOF");
            tmp_outsvg << "</g>\n";
            tmp_outsvg << "</svg>\n";
            break;
        }
        case EMR_SETPIXELV:
            dbg_str << "<!-- EMR_SETPIXELV -->\n";
            break;
        case EMR_SETMAPPERFLAGS:
            dbg_str << "<!-- EMR_SETMAPPERFLAGS -->\n";
            break;
        case EMR_SETMAPMODE:
            dbg_str << "<!-- EMR_SETMAPMODE -->\n";
            break;
        case EMR_SETBKMODE:
            dbg_str << "<!-- EMR_SETBKMODE -->\n";
            break;
        case EMR_SETPOLYFILLMODE:
        {
            dbg_str << "<!-- EMR_SETPOLYFILLMODE -->\n";

            PEMRSETPOLYFILLMODE pEmr = (PEMRSETPOLYFILLMODE) lpEMFR;
            d->dc[d->level].style.fill_rule.value =
                (pEmr->iMode == ALTERNATE ? 0 :
                 pEmr->iMode == WINDING ? 1 : 0);
            break;
        }
        case EMR_SETROP2:
            dbg_str << "<!-- EMR_SETROP2 -->\n";
            break;
        case EMR_SETSTRETCHBLTMODE:
            dbg_str << "<!-- EMR_SETSTRETCHBLTMODE -->\n";
            break;
        case EMR_SETTEXTALIGN:
        {
            dbg_str << "<!-- EMR_SETTEXTALIGN -->\n";

            PEMRSETTEXTALIGN pEmr = (PEMRSETTEXTALIGN) lpEMFR;
            d->dc[d->level].textAlign = pEmr->iMode;
            break;
        }
        case EMR_SETCOLORADJUSTMENT:
            dbg_str << "<!-- EMR_SETCOLORADJUSTMENT -->\n";
            break;
        case EMR_SETTEXTCOLOR:
        {
            dbg_str << "<!-- EMR_SETTEXTCOLOR -->\n";

            PEMRSETTEXTCOLOR pEmr = (PEMRSETTEXTCOLOR) lpEMFR;
            d->dc[d->level].textColor = pEmr->crColor;
            d->dc[d->level].textColorSet = true;
            break;
        }
        case EMR_SETBKCOLOR:
            dbg_str << "<!-- EMR_SETBKCOLOR -->\n";
            break;
        case EMR_OFFSETCLIPRGN:
            dbg_str << "<!-- EMR_OFFSETCLIPRGN -->\n";
            break;
        case EMR_MOVETOEX:
        {
            dbg_str << "<!-- EMR_MOVETOEX -->\n";

            PEMRMOVETOEX pEmr = (PEMRMOVETOEX) lpEMFR;

            if (d->path->empty()) {
                d->pathless_stroke = true;
                *(d->path) = "d=\"";
            }

            d->dc[d->level].cur = pEmr->ptl;

            tmp_path <<
                "\n\tM " <<
                pix_to_x_point( d, pEmr->ptl.x, pEmr->ptl.y ) << " " <<
                pix_to_y_point( d, pEmr->ptl.x, pEmr->ptl.y ) << " ";
            break;
        }
        case EMR_SETMETARGN:
            dbg_str << "<!-- EMR_SETMETARGN -->\n";
            break;
        case EMR_EXCLUDECLIPRECT:
            dbg_str << "<!-- EMR_EXCLUDECLIPRECT -->\n";
            break;
        case EMR_INTERSECTCLIPRECT:
            dbg_str << "<!-- EMR_INTERSECTCLIPRECT -->\n";
            break;
        case EMR_SCALEVIEWPORTEXTEX:
            dbg_str << "<!-- EMR_SCALEVIEWPORTEXTEX -->\n";
            break;
        case EMR_SCALEWINDOWEXTEX:
            dbg_str << "<!-- EMR_SCALEWINDOWEXTEX -->\n";
            break;
        case EMR_SAVEDC:
            dbg_str << "<!-- EMR_SAVEDC -->\n";

            if (d->level < EMF_MAX_DC) {
                d->dc[d->level + 1] = d->dc[d->level];
                d->level = d->level + 1;
            }
            break;
        case EMR_RESTOREDC:
        {
            dbg_str << "<!-- EMR_RESTOREDC -->\n";
            
            PEMRRESTOREDC pEmr = (PEMRRESTOREDC) lpEMFR;
            int old_level = d->level;
            if (pEmr->iRelative >= 0) {
                if (pEmr->iRelative < d->level)
                    d->level = pEmr->iRelative;
            }
            else {
                if (d->level + pEmr->iRelative >= 0)
                    d->level = d->level + pEmr->iRelative;
            }
            while (old_level > d->level) {
                if (d->dc[old_level].style.stroke_dash.dash && (old_level==0 || (old_level>0 && d->dc[old_level].style.stroke_dash.dash!=d->dc[old_level-1].style.stroke_dash.dash)))
                    delete[] d->dc[old_level].style.stroke_dash.dash;
                old_level--;
            }
            break;
        }
        case EMR_SETWORLDTRANSFORM:
        {
            dbg_str << "<!-- EMR_SETWORLDTRANSFORM -->\n";

            PEMRSETWORLDTRANSFORM pEmr = (PEMRSETWORLDTRANSFORM) lpEMFR;
            d->dc[d->level].worldTransform = pEmr->xform;
            break;
        }
        case EMR_MODIFYWORLDTRANSFORM:
        {
            dbg_str << "<!-- EMR_MODIFYWORLDTRANSFORM -->\n";

            PEMRMODIFYWORLDTRANSFORM pEmr = (PEMRMODIFYWORLDTRANSFORM) lpEMFR;
            switch (pEmr->iMode)
            {
                case MWT_IDENTITY:
                    d->dc[d->level].worldTransform.eM11 = 1.0;
                    d->dc[d->level].worldTransform.eM12 = 0.0;
                    d->dc[d->level].worldTransform.eM21 = 0.0;
                    d->dc[d->level].worldTransform.eM22 = 1.0;
                    d->dc[d->level].worldTransform.eDx  = 0.0;
                    d->dc[d->level].worldTransform.eDy  = 0.0;
                    break;
                case MWT_LEFTMULTIPLY:
                {
//                    d->dc[d->level].worldTransform = pEmr->xform * worldTransform;

                    float a11 = pEmr->xform.eM11;
                    float a12 = pEmr->xform.eM12;
                    float a13 = 0.0;
                    float a21 = pEmr->xform.eM21;
                    float a22 = pEmr->xform.eM22;
                    float a23 = 0.0;
                    float a31 = pEmr->xform.eDx;
                    float a32 = pEmr->xform.eDy;
                    float a33 = 1.0;

                    float b11 = d->dc[d->level].worldTransform.eM11;
                    float b12 = d->dc[d->level].worldTransform.eM12;
                    //float b13 = 0.0;
                    float b21 = d->dc[d->level].worldTransform.eM21;
                    float b22 = d->dc[d->level].worldTransform.eM22;
                    //float b23 = 0.0;
                    float b31 = d->dc[d->level].worldTransform.eDx;
                    float b32 = d->dc[d->level].worldTransform.eDy;
                    //float b33 = 1.0;

                    float c11 = a11*b11 + a12*b21 + a13*b31;;
                    float c12 = a11*b12 + a12*b22 + a13*b32;;
                    //float c13 = a11*b13 + a12*b23 + a13*b33;;
                    float c21 = a21*b11 + a22*b21 + a23*b31;;
                    float c22 = a21*b12 + a22*b22 + a23*b32;;
                    //float c23 = a21*b13 + a22*b23 + a23*b33;;
                    float c31 = a31*b11 + a32*b21 + a33*b31;;
                    float c32 = a31*b12 + a32*b22 + a33*b32;;
                    //float c33 = a31*b13 + a32*b23 + a33*b33;;

                    d->dc[d->level].worldTransform.eM11 = c11;;
                    d->dc[d->level].worldTransform.eM12 = c12;;
                    d->dc[d->level].worldTransform.eM21 = c21;;
                    d->dc[d->level].worldTransform.eM22 = c22;;
                    d->dc[d->level].worldTransform.eDx = c31;
                    d->dc[d->level].worldTransform.eDy = c32;
                    
                    break;
                }
                case MWT_RIGHTMULTIPLY:
                {
//                    d->dc[d->level].worldTransform = worldTransform * pEmr->xform;

                    float a11 = d->dc[d->level].worldTransform.eM11;
                    float a12 = d->dc[d->level].worldTransform.eM12;
                    float a13 = 0.0;
                    float a21 = d->dc[d->level].worldTransform.eM21;
                    float a22 = d->dc[d->level].worldTransform.eM22;
                    float a23 = 0.0;
                    float a31 = d->dc[d->level].worldTransform.eDx;
                    float a32 = d->dc[d->level].worldTransform.eDy;
                    float a33 = 1.0;

                    float b11 = pEmr->xform.eM11;
                    float b12 = pEmr->xform.eM12;
                    //float b13 = 0.0;
                    float b21 = pEmr->xform.eM21;
                    float b22 = pEmr->xform.eM22;
                    //float b23 = 0.0;
                    float b31 = pEmr->xform.eDx;
                    float b32 = pEmr->xform.eDy;
                    //float b33 = 1.0;

                    float c11 = a11*b11 + a12*b21 + a13*b31;;
                    float c12 = a11*b12 + a12*b22 + a13*b32;;
                    //float c13 = a11*b13 + a12*b23 + a13*b33;;
                    float c21 = a21*b11 + a22*b21 + a23*b31;;
                    float c22 = a21*b12 + a22*b22 + a23*b32;;
                    //float c23 = a21*b13 + a22*b23 + a23*b33;;
                    float c31 = a31*b11 + a32*b21 + a33*b31;;
                    float c32 = a31*b12 + a32*b22 + a33*b32;;
                    //float c33 = a31*b13 + a32*b23 + a33*b33;;

                    d->dc[d->level].worldTransform.eM11 = c11;;
                    d->dc[d->level].worldTransform.eM12 = c12;;
                    d->dc[d->level].worldTransform.eM21 = c21;;
                    d->dc[d->level].worldTransform.eM22 = c22;;
                    d->dc[d->level].worldTransform.eDx = c31;
                    d->dc[d->level].worldTransform.eDy = c32;

                    break;
                }
//                case MWT_SET:
                default:
                    d->dc[d->level].worldTransform = pEmr->xform;
                    break;
            }
            break;
        }
        case EMR_SELECTOBJECT:
        {
            dbg_str << "<!-- EMR_SELECTOBJECT -->\n";

            PEMRSELECTOBJECT pEmr = (PEMRSELECTOBJECT) lpEMFR;
            unsigned int index = pEmr->ihObject;

            if (index >= ENHMETA_STOCK_OBJECT) {
                index -= ENHMETA_STOCK_OBJECT;
                switch (index) {
                    case NULL_BRUSH:
                        d->dc[d->level].fill_set = false;
                        break;
                    case BLACK_BRUSH:
                    case DKGRAY_BRUSH:
                    case GRAY_BRUSH:
                    case LTGRAY_BRUSH:
                    case WHITE_BRUSH:
                    {
                        float val = 0;
                        switch (index) {
                            case BLACK_BRUSH:
                                val = 0.0 / 255.0;
                                break;
                            case DKGRAY_BRUSH:
                                val = 64.0 / 255.0;
                                break;
                            case GRAY_BRUSH:
                                val = 128.0 / 255.0;
                                break;
                            case LTGRAY_BRUSH:
                                val = 192.0 / 255.0;
                                break;
                            case WHITE_BRUSH:
                                val = 255.0 / 255.0;
                                break;
                        }
                        d->dc[d->level].style.fill.value.color.set( val, val, val );

                        d->dc[d->level].fill_set = true;
                        break;
                    }
                    case NULL_PEN:
                        d->dc[d->level].stroke_set = false;
                        break;
                    case BLACK_PEN:
                    case WHITE_PEN:
                    {
                        float val = index == BLACK_PEN ? 0 : 1;
                        d->dc[d->level].style.stroke_dasharray_set = 0;
                        d->dc[d->level].style.stroke_width.value = 1.0;
                        d->dc[d->level].style.stroke.value.color.set( val, val, val );

                        d->dc[d->level].stroke_set = true;

                        break;
                    }
                }
            } else {
                if ( /*index >= 0 &&*/ index < (unsigned int) d->n_obj) {
                    switch (d->emf_obj[index].type)
                    {
                        case EMR_CREATEPEN:
                            select_pen(d, index);
                            break;
                        case EMR_CREATEBRUSHINDIRECT:
                            select_brush(d, index);
                            break;
                        case EMR_EXTCREATEPEN:
                            select_extpen(d, index);
                            break;
                        case EMR_EXTCREATEFONTINDIRECTW:
                            select_font(d, index);
                            break;
                    }
                }
            }
            break;
        }
        case EMR_CREATEPEN:
        {
            dbg_str << "<!-- EMR_CREATEPEN -->\n";

            PEMRCREATEPEN pEmr = (PEMRCREATEPEN) lpEMFR;
            int index = pEmr->ihPen;

            EMRCREATEPEN *pPen =
                (EMRCREATEPEN *) malloc( sizeof(EMRCREATEPEN) );
            pPen->lopn = pEmr->lopn;
            insert_object(d, index, EMR_CREATEPEN, (ENHMETARECORD *) pPen);

            break;
        }
        case EMR_CREATEBRUSHINDIRECT:
        {
            dbg_str << "<!-- EMR_CREATEBRUSHINDIRECT -->\n";

            PEMRCREATEBRUSHINDIRECT pEmr = (PEMRCREATEBRUSHINDIRECT) lpEMFR;
            int index = pEmr->ihBrush;

            EMRCREATEBRUSHINDIRECT *pBrush =
                (EMRCREATEBRUSHINDIRECT *) malloc( sizeof(EMRCREATEBRUSHINDIRECT) );
            pBrush->lb = pEmr->lb;
            insert_object(d, index, EMR_CREATEBRUSHINDIRECT, (ENHMETARECORD *) pBrush);

            break;
        }
        case EMR_DELETEOBJECT:
            dbg_str << "<!-- EMR_DELETEOBJECT -->\n";
            break;
        case EMR_ANGLEARC:
            dbg_str << "<!-- EMR_ANGLEARC -->\n";
            break;
        case EMR_ELLIPSE:
        {
            dbg_str << "<!-- EMR_ELLIPSE -->\n";

            PEMRELLIPSE pEmr = (PEMRELLIPSE) lpEMFR;
            RECTL rclBox = pEmr->rclBox;

            double l = pix_to_x_point( d, pEmr->rclBox.left, pEmr->rclBox.top );
            double t = pix_to_y_point( d, pEmr->rclBox.left, pEmr->rclBox.top );
            double r = pix_to_x_point( d, pEmr->rclBox.right, pEmr->rclBox.bottom );
            double b = pix_to_y_point( d, pEmr->rclBox.right, pEmr->rclBox.bottom );

            double cx = (l + r) / 2.0;
            double cy = (t + b) / 2.0;
            double rx = fabs(l - r) / 2.0;
            double ry = fabs(t - b) / 2.0;

            SVGOStringStream tmp_ellipse;
            tmp_ellipse << "cx=\"" << cx << "\" ";
            tmp_ellipse << "cy=\"" << cy << "\" ";
            tmp_ellipse << "rx=\"" << rx << "\" ";
            tmp_ellipse << "ry=\"" << ry << "\" ";

            assert_empty_path(d, "EMR_ELLIPSE");

            *(d->outsvg) += "    <ellipse ";
            output_style(d, lpEMFR->iType);
            *(d->outsvg) += "\n\t";
            *(d->outsvg) += tmp_ellipse.str().c_str();
            *(d->outsvg) += "/> \n";
            *(d->path) = "";
            break;
        }
        case EMR_RECTANGLE:
        {
            dbg_str << "<!-- EMR_RECTANGLE -->\n";

            PEMRRECTANGLE pEmr = (PEMRRECTANGLE) lpEMFR;
            RECTL rc = pEmr->rclBox;

            double l = pix_to_x_point( d, rc.left, rc.top );
            double t = pix_to_y_point( d, rc.left, rc.top );
            double r = pix_to_x_point( d, rc.right, rc.bottom );
            double b = pix_to_y_point( d, rc.right, rc.bottom );

            SVGOStringStream tmp_rectangle;
            tmp_rectangle << "d=\"";
            tmp_rectangle << "\n\tM " << l << " " << t << " ";
            tmp_rectangle << "\n\tL " << r << " " << t << " ";
            tmp_rectangle << "\n\tL " << r << " " << b << " ";
            tmp_rectangle << "\n\tL " << l << " " << b << " ";
            tmp_rectangle << "\n\tz";

            assert_empty_path(d, "EMR_RECTANGLE");

            *(d->outsvg) += "    <path ";
            output_style(d, lpEMFR->iType);
            *(d->outsvg) += "\n\t";
            *(d->outsvg) += tmp_rectangle.str().c_str();
            *(d->outsvg) += " \" /> \n";
            *(d->path) = "";
            break;
        }
        case EMR_ROUNDRECT:
            dbg_str << "<!-- EMR_ROUNDRECT -->\n";
            break;
        case EMR_ARC:
            dbg_str << "<!-- EMR_ARC -->\n";
            break;
        case EMR_CHORD:
            dbg_str << "<!-- EMR_CHORD -->\n";
            break;
        case EMR_PIE:
            dbg_str << "<!-- EMR_PIE -->\n";
            break;
        case EMR_SELECTPALETTE:
            dbg_str << "<!-- EMR_SELECTPALETTE -->\n";
            break;
        case EMR_CREATEPALETTE:
            dbg_str << "<!-- EMR_CREATEPALETTE -->\n";
            break;
        case EMR_SETPALETTEENTRIES:
            dbg_str << "<!-- EMR_SETPALETTEENTRIES -->\n";
            break;
        case EMR_RESIZEPALETTE:
            dbg_str << "<!-- EMR_RESIZEPALETTE -->\n";
            break;
        case EMR_REALIZEPALETTE:
            dbg_str << "<!-- EMR_REALIZEPALETTE -->\n";
            break;
        case EMR_EXTFLOODFILL:
            dbg_str << "<!-- EMR_EXTFLOODFILL -->\n";
            break;
        case EMR_LINETO:
        {
            dbg_str << "<!-- EMR_LINETO -->\n";

            PEMRLINETO pEmr = (PEMRLINETO) lpEMFR;

            if (d->path->empty()) {
                d->pathless_stroke = true;
                *(d->path) = "d=\"";
            }

            tmp_path <<
                "\n\tL " <<
                pix_to_x_point( d, pEmr->ptl.x, pEmr->ptl.y ) << " " <<
                pix_to_y_point( d, pEmr->ptl.x, pEmr->ptl.y ) << " ";
            break;
        }
        case EMR_ARCTO:
            dbg_str << "<!-- EMR_ARCTO -->\n";
            break;
        case EMR_POLYDRAW:
            dbg_str << "<!-- EMR_POLYDRAW -->\n";
            break;
        case EMR_SETARCDIRECTION:
            dbg_str << "<!-- EMR_SETARCDIRECTION -->\n";
            break;
        case EMR_SETMITERLIMIT:
        {
            dbg_str << "<!-- EMR_SETMITERLIMIT -->\n";

            PEMRSETMITERLIMIT pEmr = (PEMRSETMITERLIMIT) lpEMFR;

            float miterlimit = pEmr->eMiterLimit;
            miterlimit = miterlimit * 4.0 / 10.0;
            d->dc[d->level].style.stroke_miterlimit.value = pix_to_size_point( d, miterlimit );
            if (d->dc[d->level].style.stroke_miterlimit.value < 1)
                d->dc[d->level].style.stroke_miterlimit.value = 4.0;
            break;
        }
        case EMR_BEGINPATH:
        {
            dbg_str << "<!-- EMR_BEGINPATH -->\n";

            tmp_path << "d=\"";
            *(d->path) = "";
            d->inpath = true;
            break;
        }
        case EMR_ENDPATH:
        {
            dbg_str << "<!-- EMR_ENDPATH -->\n";

            tmp_path << "\"";
            d->inpath = false;
            break;
        }
        case EMR_CLOSEFIGURE:
        {
            dbg_str << "<!-- EMR_CLOSEFIGURE -->\n";

            tmp_path << "\n\tz";
            break;
        }
        case EMR_FILLPATH:
        case EMR_STROKEANDFILLPATH:
        case EMR_STROKEPATH:
        {
            if (lpEMFR->iType == EMR_FILLPATH)
                dbg_str << "<!-- EMR_FILLPATH -->\n";
            if (lpEMFR->iType == EMR_STROKEANDFILLPATH)
                dbg_str << "<!-- EMR_STROKEANDFILLPATH -->\n";
            if (lpEMFR->iType == EMR_STROKEPATH)
                dbg_str << "<!-- EMR_STROKEPATH -->\n";

            *(d->outsvg) += "    <path ";
            output_style(d, lpEMFR->iType);
            *(d->outsvg) += "\n\t";
            *(d->outsvg) += *(d->path);
            *(d->outsvg) += " /> \n";
            *(d->path) = "";
            break;
        }
        case EMR_FLATTENPATH:
            dbg_str << "<!-- EMR_FLATTENPATH -->\n";
            break;
        case EMR_WIDENPATH:
            dbg_str << "<!-- EMR_WIDENPATH -->\n";
            break;
        case EMR_SELECTCLIPPATH:
            dbg_str << "<!-- EMR_SELECTCLIPPATH -->\n";
            break;
        case EMR_ABORTPATH:
            dbg_str << "<!-- EMR_ABORTPATH -->\n";
            break;
        case EMR_GDICOMMENT:
        {
            dbg_str << "<!-- EMR_GDICOMMENT -->\n";
            
            PEMRGDICOMMENT pEmr = (PEMRGDICOMMENT) lpEMFR;

            CHAR *szTxt = (CHAR *) pEmr->Data;

            for (DWORD i = 0; i < pEmr->cbData; i++) {
                if ( *szTxt) {
                    if ( *szTxt >= ' ' && *szTxt < 'z' && *szTxt != '<' && *szTxt != '>' ) {
                        tmp_str << *szTxt;
                    }
                    szTxt++;
                }
            }

            if (0 && strlen(tmp_str.str().c_str())) {
                tmp_outsvg << "    <!-- \"";
                tmp_outsvg << tmp_str.str().c_str();
                tmp_outsvg << "\" -->\n";
            }
            
            break;
        }
        case EMR_FILLRGN:
            dbg_str << "<!-- EMR_FILLRGN -->\n";
            break;
        case EMR_FRAMERGN:
            dbg_str << "<!-- EMR_FRAMERGN -->\n";
            break;
        case EMR_INVERTRGN:
            dbg_str << "<!-- EMR_INVERTRGN -->\n";
            break;
        case EMR_PAINTRGN:
            dbg_str << "<!-- EMR_PAINTRGN -->\n";
            break;
        case EMR_EXTSELECTCLIPRGN:
            dbg_str << "<!-- EMR_EXTSELECTCLIPRGN -->\n";
            break;
        case EMR_BITBLT:
            dbg_str << "<!-- EMR_BITBLT -->\n";
            break;
        case EMR_STRETCHBLT:
            dbg_str << "<!-- EMR_STRETCHBLT -->\n";
            break;
        case EMR_MASKBLT:
            dbg_str << "<!-- EMR_MASKBLT -->\n";
            break;
        case EMR_PLGBLT:
            dbg_str << "<!-- EMR_PLGBLT -->\n";
            break;
        case EMR_SETDIBITSTODEVICE:
            dbg_str << "<!-- EMR_SETDIBITSTODEVICE -->\n";
            break;
        case EMR_STRETCHDIBITS:
            dbg_str << "<!-- EMR_STRETCHDIBITS -->\n";
            break;
        case EMR_EXTCREATEFONTINDIRECTW:
        {
            dbg_str << "<!-- EMR_EXTCREATEFONTINDIRECTW -->\n";

            PEMREXTCREATEFONTINDIRECTW pEmr = (PEMREXTCREATEFONTINDIRECTW) lpEMFR;
            int index = pEmr->ihFont;

            EMREXTCREATEFONTINDIRECTW *pFont =
                (EMREXTCREATEFONTINDIRECTW *) malloc( sizeof(EMREXTCREATEFONTINDIRECTW) );
            pFont->elfw = pEmr->elfw;
            insert_object(d, index, EMR_EXTCREATEFONTINDIRECTW, (ENHMETARECORD *) pFont);
            break;
        }
        case EMR_EXTTEXTOUTA:
        {
            dbg_str << "<!-- EMR_EXTTEXTOUTA -->\n";
            break;
        }
        case EMR_EXTTEXTOUTW:
        {
            dbg_str << "<!-- EMR_EXTTEXTOUTW -->\n";

            PEMREXTTEXTOUTW pEmr = (PEMREXTTEXTOUTW) lpEMFR;

            double x1 = pEmr->emrtext.ptlReference.x;
            double y1 = pEmr->emrtext.ptlReference.y;
            
            if (d->dc[d->level].textAlign & TA_UPDATECP) {
                x1 = d->dc[d->level].cur.x;
                y1 = d->dc[d->level].cur.y;
            }

            if (!(d->dc[d->level].textAlign & TA_BOTTOM))
                y1 += fabs(d->dc[d->level].style.font_size.computed);
            
            double x = pix_to_x_point(d, x1, y1);
            double y = pix_to_y_point(d, x1, y1);

            wchar_t *wide_text = (wchar_t *) ((char *) pEmr + pEmr->emrtext.offString);

            gchar *ansi_text =
                (gchar *) g_utf16_to_utf8( (gunichar2 *) wide_text, pEmr->emrtext.nChars, NULL, NULL, NULL );

            if (ansi_text) {
                gchar *p = ansi_text;
                while (*p) {
                    if (*p < 32 || *p >= 127) {
                        g_free(ansi_text);
                        ansi_text = g_strdup("");
                        break;
                    }
                    p++;
                }

                SVGOStringStream ts;

                gchar *escaped_text = g_markup_escape_text(ansi_text, -1);

                float text_rgb[3];
                sp_color_get_rgb_floatv( &(d->dc[d->level].style.fill.value.color), text_rgb );

                if (!d->dc[d->level].textColorSet) {
                    d->dc[d->level].textColor = RGB(SP_COLOR_F_TO_U(text_rgb[0]),
                                       SP_COLOR_F_TO_U(text_rgb[1]),
                                       SP_COLOR_F_TO_U(text_rgb[2]));
                }

                char tmp[128];
                snprintf(tmp, 127,
                         "fill:#%02x%02x%02x;",
                         GetRValue(d->dc[d->level].textColor),
                         GetGValue(d->dc[d->level].textColor),
                         GetBValue(d->dc[d->level].textColor));

                bool i = (d->dc[d->level].style.font_style.value == SP_CSS_FONT_STYLE_ITALIC);
                //bool o = (d->dc[d->level].style.font_style.value == SP_CSS_FONT_STYLE_OBLIQUE);
                bool b = (d->dc[d->level].style.font_weight.value == SP_CSS_FONT_WEIGHT_BOLD) ||
                    (d->dc[d->level].style.font_weight.value >= SP_CSS_FONT_WEIGHT_500 && d->dc[d->level].style.font_weight.value <= SP_CSS_FONT_WEIGHT_900);
                int lcr = ((d->dc[d->level].textAlign & TA_CENTER) == TA_CENTER) ? 2 : ((d->dc[d->level].textAlign & TA_RIGHT) == TA_RIGHT) ? 1 : 0;

                assert_empty_path(d, "EMR_EXTTEXTOUTW");

                ts << "    <text\n";
                ts << "        id=\"" << (d->id++) << "\"\n";
                ts << "        xml:space=\"preserve\"\n";
                ts << "        x=\"" << x << "\"\n";
                ts << "        y=\"" << y << "\"\n";
                if (d->dc[d->level].style.text_transform.value) {
                    ts << "        transform=\""
                       << "rotate(-" << d->dc[d->level].style.text_transform.value
                       << " " << x << " " << y << ")"
                       << "\"\n";
                }
                ts << "        style=\""
                   << "font-size:" << fabs(d->dc[d->level].style.font_size.computed) << "px;"
                   << tmp
                   << "font-style:" << (i ? "italic" : "normal") << ";"
                   << "font-weight:" << (b ? "bold" : "normal") << ";"
                   << "text-align:" << (lcr==2 ? "center" : lcr==1 ? "end" : "start") << ";"
                   << "text-anchor:" << (lcr==2 ? "middle" : lcr==1 ? "end" : "start") << ";"
                   << "font-family:" << d->dc[d->level].tstyle.font_family.value << ";"
                   << "\"\n";
                ts << "    >";
                ts << escaped_text;
                ts << "</text>\n";
                
                *(d->outsvg) += ts.str().c_str();
                
                g_free(escaped_text);
                g_free(ansi_text);
            }
            
            break;
        }
        case EMR_POLYBEZIER16:
        {
            dbg_str << "<!-- EMR_POLYBEZIER16 -->\n";

            PEMRPOLYBEZIER16 pEmr = (PEMRPOLYBEZIER16) lpEMFR;
            POINTS *apts = (POINTS *) pEmr->apts; // Bug in MinGW wingdi.h ?
            DWORD i,j;

            if (pEmr->cpts<4)
                break;

            if (!d->inpath) {
                assert_empty_path(d, "EMR_POLYBEZIER16");

                *(d->outsvg) += "    <path ";
                output_style(d, EMR_STROKEPATH);
                *(d->outsvg) += "\n\td=\"";
            }

            tmp_str <<
                "\n\tM " <<
                pix_to_x_point( d, apts[0].x, apts[0].y ) << " " <<
                pix_to_y_point( d, apts[0].x, apts[0].y ) << " ";

            for (i=1; i<pEmr->cpts; ) {
                tmp_str << "\n\tC ";
                for (j=0; j<3 && i<pEmr->cpts; j++,i++) {
                    tmp_str <<
                        pix_to_x_point( d, apts[i].x, apts[i].y ) << " " <<
                        pix_to_y_point( d, apts[i].x, apts[i].y ) << " ";
                }
            }

            if (d->inpath) {
                tmp_path << tmp_str.str().c_str();
            }
            else {
                *(d->outsvg) += tmp_str.str().c_str();
                *(d->outsvg) += " \" /> \n";
            }

            break;
        }
        case EMR_POLYGON16:
        {
            dbg_str << "<!-- EMR_POLYGON16 -->\n";

            PEMRPOLYGON16 pEmr = (PEMRPOLYGON16) lpEMFR;
            POINTS *apts = (POINTS *) pEmr->apts; // Bug in MinGW wingdi.h ?
            SVGOStringStream tmp_poly;
            unsigned int i;
            unsigned int first = 0;

            assert_empty_path(d, "EMR_POLYGON16");

            *(d->outsvg) += "    <path ";
            output_style(d, EMR_STROKEANDFILLPATH);
            *(d->outsvg) += "\n\td=\"";
            
            // skip the first point?
            tmp_poly << "\n\tM " <<
                pix_to_x_point( d, apts[first].x, apts[first].y ) << " " <<
                pix_to_y_point( d, apts[first].x, apts[first].y ) << " ";

            for (i=first+1; i<pEmr->cpts; i++) {
                tmp_poly << "\n\tL " <<
                    pix_to_x_point( d, apts[i].x, apts[i].y ) << " " <<
                    pix_to_y_point( d, apts[i].x, apts[i].y ) << " ";
            }

            *(d->outsvg) += tmp_poly.str().c_str();
            *(d->outsvg) += " z \" /> \n";

            break;
        }
        case EMR_POLYLINE16:
        {
            dbg_str << "<!-- EMR_POLYLINE16 -->\n";

            EMRPOLYLINE16 *pEmr = (EMRPOLYLINE16 *) lpEMFR;
            POINTS *apts = (POINTS *) pEmr->apts; // Bug in MinGW wingdi.h ?
            DWORD i;

            if (pEmr->cpts<2)
                break;

            if (!d->inpath) {
                assert_empty_path(d, "EMR_POLYLINE16");

                *(d->outsvg) += "    <path ";
                output_style(d, EMR_STROKEPATH);
                *(d->outsvg) += "\n\td=\"";
            }

            tmp_str <<
                "\n\tM " <<
                pix_to_x_point( d, apts[0].x, apts[0].y ) << " " <<
                pix_to_y_point( d, apts[0].x, apts[0].y ) << " ";

            for (i=1; i<pEmr->cpts; i++) {
                tmp_str <<
                    "\n\tL " <<
                    pix_to_x_point( d, apts[i].x, apts[i].y ) << " " <<
                    pix_to_y_point( d, apts[i].x, apts[i].y ) << " ";
            }

            if (d->inpath) {
                tmp_path << tmp_str.str().c_str();
            }
            else {
                *(d->outsvg) += tmp_str.str().c_str();
                *(d->outsvg) += " \" /> \n";
            }

            break;
        }
        case EMR_POLYBEZIERTO16:
        {
            dbg_str << "<!-- EMR_POLYBEZIERTO16 -->\n";

            PEMRPOLYBEZIERTO16 pEmr = (PEMRPOLYBEZIERTO16) lpEMFR;
            POINTS *apts = (POINTS *) pEmr->apts; // Bug in MinGW wingdi.h ?
            DWORD i,j;

            if (d->path->empty()) {
                d->pathless_stroke = true;
                *(d->path) = "d=\"";
            }

            for (i=0; i<pEmr->cpts;) {
                tmp_path << "\n\tC ";
                for (j=0; j<3 && i<pEmr->cpts; j++,i++) {
                    tmp_path <<
                        pix_to_x_point( d, apts[i].x, apts[i].y ) << " " <<
                        pix_to_y_point( d, apts[i].x, apts[i].y ) << " ";
                }
            }

            break;
        }
        case EMR_POLYLINETO16:
        {
            dbg_str << "<!-- EMR_POLYLINETO16 -->\n";

            PEMRPOLYLINETO16 pEmr = (PEMRPOLYLINETO16) lpEMFR;
            POINTS *apts = (POINTS *) pEmr->apts; // Bug in MinGW wingdi.h ?
            DWORD i;

            if (d->path->empty()) {
                d->pathless_stroke = true;
                *(d->path) = "d=\"";
            }

            for (i=0; i<pEmr->cpts;i++) {
                tmp_path <<
                    "\n\tL " <<
                    pix_to_x_point( d, apts[i].x, apts[i].y ) << " " <<
                    pix_to_y_point( d, apts[i].x, apts[i].y ) << " ";
            }

            break;
        }
        case EMR_POLYPOLYLINE16:
        case EMR_POLYPOLYGON16:
        {
            if (lpEMFR->iType == EMR_POLYPOLYLINE16)
                dbg_str << "<!-- EMR_POLYPOLYLINE16 -->\n";
            if (lpEMFR->iType == EMR_POLYPOLYGON16)
                dbg_str << "<!-- EMR_POLYPOLYGON16 -->\n";

            PEMRPOLYPOLYGON16 pEmr = (PEMRPOLYPOLYGON16) lpEMFR;
            unsigned int n, i, j;

            if (!d->inpath) {
                assert_empty_path(d, lpEMFR->iType == EMR_POLYPOLYGON16 ? "EMR_POLYPOLYGON16" : "EMR_POLYPOLYLINE16");

                *(d->outsvg) += "    <path ";
                output_style(d, lpEMFR->iType==EMR_POLYPOLYGON16 ? EMR_STROKEANDFILLPATH : EMR_STROKEPATH);
                *(d->outsvg) += "\n\td=\"";
            }

            POINTS *apts = (POINTS *) &pEmr->aPolyCounts[pEmr->nPolys];

            i = 0;
            for (n=0; n<pEmr->nPolys && i<pEmr->cpts; n++) {
                SVGOStringStream poly_path;

                poly_path << "\n\tM " <<
                    pix_to_x_point( d, apts[i].x, apts[i].y ) << " " <<
                    pix_to_y_point( d, apts[i].x, apts[i].y ) << " ";
                i++;

                for (j=1; j<pEmr->aPolyCounts[n] && i<pEmr->cpts; j++) {
                    poly_path << "\n\tL " <<
                        pix_to_x_point( d, apts[i].x, apts[i].y ) << " " <<
                        pix_to_y_point( d, apts[i].x, apts[i].y ) << " ";
                    i++;
                }

                tmp_str << poly_path.str().c_str();
                if (lpEMFR->iType == EMR_POLYPOLYGON16)
                    tmp_str << " z";
                tmp_str << " \n";
            }

            if (d->inpath) {
                tmp_path << tmp_str.str().c_str();
            }
            else {
                *(d->outsvg) += tmp_str.str().c_str();
                *(d->outsvg) += " \" /> \n";
            }

            break;
        }
        case EMR_POLYDRAW16:
            dbg_str << "<!-- EMR_POLYDRAW16 -->\n";
            break;
        case EMR_CREATEMONOBRUSH:
            dbg_str << "<!-- EMR_CREATEMONOBRUSH -->\n";
            break;
        case EMR_CREATEDIBPATTERNBRUSHPT:
            dbg_str << "<!-- EMR_CREATEDIBPATTERNBRUSHPT -->\n";
            break;
        case EMR_EXTCREATEPEN:
        {
            dbg_str << "<!-- EMR_EXTCREATEPEN -->\n";

            PEMREXTCREATEPEN pEmr = (PEMREXTCREATEPEN) lpEMFR;
            int index = pEmr->ihPen;

            EMREXTCREATEPEN *pPen =
                (EMREXTCREATEPEN *) malloc( sizeof(EMREXTCREATEPEN) +
                                            sizeof(DWORD) * pEmr->elp.elpNumEntries );
            pPen->ihPen = pEmr->ihPen;
            pPen->offBmi = pEmr->offBmi;
            pPen->cbBmi = pEmr->cbBmi;
            pPen->offBits = pEmr->offBits;
            pPen->cbBits = pEmr->cbBits;
            pPen->elp = pEmr->elp;
            for (unsigned int i=0; i<pEmr->elp.elpNumEntries; i++) {
                pPen->elp.elpStyleEntry[i] = pEmr->elp.elpStyleEntry[i];
            }
            insert_object(d, index, EMR_EXTCREATEPEN, (ENHMETARECORD *) pPen);

            break;
        }
        case EMR_POLYTEXTOUTA:
            dbg_str << "<!-- EMR_POLYTEXTOUTA -->\n";
            break;
        case EMR_POLYTEXTOUTW:
            dbg_str << "<!-- EMR_POLYTEXTOUTW -->\n";
            break;
        case EMR_SETICMMODE:
            dbg_str << "<!-- EMR_SETICMMODE -->\n";
            break;
        case EMR_CREATECOLORSPACE:
            dbg_str << "<!-- EMR_CREATECOLORSPACE -->\n";
            break;
        case EMR_SETCOLORSPACE:
            dbg_str << "<!-- EMR_SETCOLORSPACE -->\n";
            break;
        case EMR_DELETECOLORSPACE:
            dbg_str << "<!-- EMR_DELETECOLORSPACE -->\n";
            break;
        case EMR_GLSRECORD:
            dbg_str << "<!-- EMR_GLSRECORD -->\n";
            break;
        case EMR_GLSBOUNDEDRECORD:
            dbg_str << "<!-- EMR_GLSBOUNDEDRECORD -->\n";
            break;
        case EMR_PIXELFORMAT:
            dbg_str << "<!-- EMR_PIXELFORMAT -->\n";
            break;
        default:
            dbg_str << "<!-- EMR_??? -->\n";
            break;
    }
    
//    *(d->outsvg) += dbg_str.str().c_str();
    *(d->outsvg) += tmp_outsvg.str().c_str();
    *(d->path) += tmp_path.str().c_str();

    return 1;
}

static int CALLBACK
myMetaFileProc(HDC /*hDC*/, HANDLETABLE * /*lpHTable*/, METARECORD * /*lpMFR*/, int /*nObj*/, LPARAM /*lpData*/)
{
    g_warning("Unable to import Windows Meta File.\n");
    return 0;
}

// Aldus Placeable Header ===================================================
// Since we are a 32bit app, we have to be sure this structure compiles to
// be identical to a 16 bit app's version. To do this, we use the #pragma
// to adjust packing, we use a WORD for the hmf handle, and a SMALL_RECT
// for the bbox rectangle.
#pragma pack( push )
#pragma pack( 2 )
typedef struct
{
    DWORD       dwKey;
    WORD        hmf;
    SMALL_RECT  bbox;
    WORD        wInch;
    DWORD       dwReserved;
    WORD        wCheckSum;
} APMHEADER, *PAPMHEADER;
#pragma pack( pop )


SPDocument *
EmfWin32::open( Inkscape::Extension::Input * /*mod*/, const gchar *uri )
{
    EMF_CALLBACK_DATA d;

    memset(&d, 0, sizeof(d));

    d.dc[0].worldTransform.eM11 = 1.0;
    d.dc[0].worldTransform.eM12 = 0.0;
    d.dc[0].worldTransform.eM21 = 0.0;
    d.dc[0].worldTransform.eM22 = 1.0;
    d.dc[0].worldTransform.eDx  = 0.0;
    d.dc[0].worldTransform.eDy  = 0.0;
    
    gsize bytesRead = 0;
    gsize bytesWritten = 0;
    GError* error = NULL;
    gchar *local_fn =
        g_filename_from_utf8( uri, -1,  &bytesRead,  &bytesWritten, &error );

    if (local_fn == NULL) {
        return NULL;
    }

    d.outsvg = new Glib::ustring("");
    d.path = new Glib::ustring("");

    CHAR *ansi_uri = (CHAR *) local_fn;
    gunichar2 *unicode_fn = g_utf8_to_utf16( local_fn, -1, NULL, NULL, NULL );
    WCHAR *unicode_uri = (WCHAR *) unicode_fn;

    DWORD filesize = 0;
    HANDLE fp = NULL;

    HMETAFILE hmf;
    HENHMETAFILE hemf;

    if (PrintWin32::is_os_wide()) {
        fp = CreateFileW(unicode_uri, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    else {
        fp = CreateFileA(ansi_uri, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    if ( fp != INVALID_HANDLE_VALUE ) {
        filesize = GetFileSize(fp, NULL);
        CloseHandle(fp);
    }

    // Try open as Enhanced Metafile
    if (PrintWin32::is_os_wide())
        hemf = GetEnhMetaFileW(unicode_uri);
    else
        hemf = GetEnhMetaFileA(ansi_uri);

    if (!hemf) {
        // Try open as Windows Metafile
        if (PrintWin32::is_os_wide())
            hmf = GetMetaFileW(unicode_uri);
        else
            hmf = GetMetaFileA(ansi_uri);

        METAFILEPICT mp;
        HDC hDC;

        if (!hmf) {
            if (PrintWin32::is_os_wide()) {
                WCHAR szTemp[MAX_PATH];

                DWORD dw = GetShortPathNameW( unicode_uri, szTemp, MAX_PATH );
                if (dw) {
                    hmf = GetMetaFileW( szTemp );
                }
            } else {
                CHAR szTemp[MAX_PATH];

                DWORD dw = GetShortPathNameA( ansi_uri, szTemp, MAX_PATH );
                if (dw) {
                    hmf = GetMetaFileA( szTemp );
                }
            }
        }

        if (hmf) {
            DWORD nSize = GetMetaFileBitsEx( hmf, 0, NULL );

            if (!nSize)
                nSize = filesize;
            
            if (nSize) {
                BYTE *lpvData = new BYTE[nSize];
                if (lpvData) {
                    DWORD dw = GetMetaFileBitsEx( hmf, nSize, lpvData );
                    if (dw) {
                        // Fill out a METAFILEPICT structure
                        mp.mm = MM_ANISOTROPIC;
                        mp.xExt = 1000;
                        mp.yExt = 1000;
                        mp.hMF = NULL;
                        // Get a reference DC
                        hDC = GetDC( NULL );
                        // Make an enhanced metafile from the windows metafile
                        hemf = SetWinMetaFileBits( nSize, lpvData, hDC, &mp );
                        // Clean up
                        ReleaseDC( NULL, hDC );
                        DeleteMetaFile( hmf );
                        hmf = NULL;
                    }
                    else {
                        // EnumMetaFile
                    }
                    delete[] lpvData;
                }
                else {
                    DeleteMetaFile( hmf );
                    hmf = NULL;
                }
            }
            else {
                DeleteMetaFile( hmf );
                hmf = NULL;
            }
        }
        else {
            // Try open as Aldus Placeable Metafile
            HANDLE hFile;
            if (PrintWin32::is_os_wide())
                hFile = CreateFileW( unicode_uri, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
            else
                hFile = CreateFileA( ansi_uri, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

            if (hFile != INVALID_HANDLE_VALUE) {
                DWORD nSize = GetFileSize( hFile, NULL );
                if (nSize) {
                    BYTE *lpvData = new BYTE[nSize];
                    if (lpvData) {
                        DWORD dw = ReadFile( hFile, lpvData, nSize, &nSize, NULL );
                        if (dw) {
                            if ( ((PAPMHEADER)lpvData)->dwKey == 0x9ac6cdd7l ) {
                                // Fill out a METAFILEPICT structure
                                mp.mm = MM_ANISOTROPIC;
                                mp.xExt = ((PAPMHEADER)lpvData)->bbox.Right - ((PAPMHEADER)lpvData)->bbox.Left;
                                mp.xExt = ( mp.xExt * 2540l ) / (DWORD)(((PAPMHEADER)lpvData)->wInch);
                                mp.yExt = ((PAPMHEADER)lpvData)->bbox.Bottom - ((PAPMHEADER)lpvData)->bbox.Top;
                                mp.yExt = ( mp.yExt * 2540l ) / (DWORD)(((PAPMHEADER)lpvData)->wInch);
                                mp.hMF = NULL;
                                // Get a reference DC
                                hDC = GetDC( NULL );
                                // Create an enhanced metafile from the bits
                                hemf = SetWinMetaFileBits( nSize, lpvData+sizeof(APMHEADER), hDC, &mp );
                                // Clean up
                                ReleaseDC( NULL, hDC );
                            }
                        }
                        delete[] lpvData;
                    }
                }
                CloseHandle( hFile );
            }
        }
    }

    if ((!hemf && !hmf) || !d.outsvg || !d.path) {
        if (d.outsvg)
            delete d.outsvg;
        if (d.path)
            delete d.path;
        if (local_fn)
            g_free(local_fn);
        if (unicode_fn)
            g_free(unicode_fn);
        if (hemf)
            DeleteEnhMetaFile(hemf);
        if (hmf)
            DeleteMetaFile(hmf);
        return NULL;
    }

    d.pDesc = NULL;

    if (hemf) {
        DWORD dwNeeded = GetEnhMetaFileDescriptionA( hemf, 0, NULL );
        if ( dwNeeded > 0 ) {
            d.pDesc = (CHAR *) malloc( dwNeeded + 1 );
            if ( GetEnhMetaFileDescription( hemf, dwNeeded, d.pDesc ) == 0 )
                lstrcpy( d.pDesc, "" );
            if ( lstrlen( d.pDesc ) > 1 )
                d.pDesc[lstrlen(d.pDesc)] = '#';
        }

        EnumEnhMetaFile(NULL, hemf, myEnhMetaFileProc, (LPVOID) &d, NULL);
        DeleteEnhMetaFile(hemf);
    }
    else {
        EnumMetaFile(NULL, hmf, myMetaFileProc, (LPARAM) &d);
        DeleteMetaFile(hmf);
    }
    
    if (d.pDesc)
        free( d.pDesc );

//    std::cout << "SVG Output: " << std::endl << *(d.outsvg) << std::endl;

    SPDocument *doc = sp_document_new_from_mem(d.outsvg->c_str(), d.outsvg->length(), TRUE);

    delete d.outsvg;
    delete d.path;

    if (d.emf_obj) {
        int i;
        for (i=0; i<d.n_obj; i++)
            delete_object(&d, i);
        delete[] d.emf_obj;
    }
    
    if (d.dc[0].style.stroke_dash.dash)
        delete[] d.dc[0].style.stroke_dash.dash;

    if  (local_fn)
        g_free(local_fn);
    if  (unicode_fn)
        g_free(unicode_fn);

    return doc;
}


void
EmfWin32::init (void)
{
    Inkscape::Extension::Extension * ext;

    /* EMF in */
    ext = Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("EMF Input") "</name>\n"
            "<id>org.inkscape.input.emf.win32</id>\n"
            "<input>\n"
                "<extension>.emf</extension>\n"
                "<mimetype>image/x-emf</mimetype>\n"
                "<filetypename>" N_("Enhanced Metafiles (*.emf)") "</filetypename>\n"
                "<filetypetooltip>" N_("Enhanced Metafiles") "</filetypetooltip>\n"
                "<output_extension>org.inkscape.output.emf.win32</output_extension>\n"
            "</input>\n"
        "</inkscape-extension>", new EmfWin32());

    /* WMF in */
    ext = Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("WMF Input") "</name>\n"
            "<id>org.inkscape.input.wmf.win32</id>\n"
            "<input>\n"
                "<extension>.wmf</extension>\n"
                "<mimetype>image/x-wmf</mimetype>\n"
                "<filetypename>" N_("Windows Metafiles (*.wmf)") "</filetypename>\n"
                "<filetypetooltip>" N_("Windows Metafiles") "</filetypetooltip>\n"
                "<output_extension>org.inkscape.output.emf.win32</output_extension>\n"
            "</input>\n"
        "</inkscape-extension>", new EmfWin32());

    /* EMF out */
    ext = Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("EMF Output") "</name>\n"
            "<id>org.inkscape.output.emf.win32</id>\n"
            "<param name=\"textToPath\" gui-text=\"" N_("Convert texts to paths") "\" type=\"boolean\">true</param>\n"
            "<output>\n"
                "<extension>.emf</extension>\n"
                "<mimetype>image/x-emf</mimetype>\n"
                "<filetypename>" N_("Enhanced Metafile (*.emf)") "</filetypename>\n"
                "<filetypetooltip>" N_("Enhanced Metafile") "</filetypetooltip>\n"
            "</output>\n"
        "</inkscape-extension>", new EmfWin32());

    return;
}


} } }  /* namespace Inkscape, Extension, Implementation */


#endif /* WIN32 */


/*
  Local Variables:
  mode:cpp
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
