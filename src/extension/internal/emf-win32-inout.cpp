/** \file
 * Enhanced Metafile Input and Output.
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

#include "win32.h"
#include "emf-win32-print.h"
#include "emf-win32-inout.h"
#include "inkscape.h"
#include "sp-path.h"
#include "style.h"
#include "color.h"
#include "display/curve.h"
#include "libnr/n-art-bpath.h"
#include "libnr/nr-point-matrix-ops.h"
#include "gtk/gtk.h"
#include "print.h"
#include "glibmm/i18n.h"
#include "extension/extension.h"
#include "extension/system.h"
#include "extension/print.h"
#include "extension/db.h"
#include "extension/output.h"
#include "document.h"
#include "display/nr-arena.h"
#include "display/nr-arena-item.h"

#include "libnr/nr-rect.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-pixblock.h"

#include <stdio.h>
#include <string.h>

#include <vector>
#include <string>

#include "io/sys.h"

#include "unit-constants.h"

#include "clear-n_.h"


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
EmfWin32::check (Inkscape::Extension::Extension * module)
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
    nr_arena_item_unref(mod->root);
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

//    bool old_textToPath  = ext->get_param_bool("textToPath");
//    bool new_val         = mod->get_param_bool("textToPath");
//    ext->set_param_bool("textToPath", new_val);

    gchar * final_name;
    final_name = g_strdup_printf("%s", uri);
    emf_print_document_to_file(doc, final_name);
    g_free(final_name);

//    ext->set_param_bool("textToPath", old_textToPath);

    return;
}



typedef struct {
    int type;
    ENHMETARECORD *lpEMFR;
} EMF_OBJECT, *PEMF_OBJECT;

typedef struct emf_callback_data {
    Glib::ustring *outsvg;
    Glib::ustring *path;
    struct SPStyle style;
    bool stroke_set;
    bool fill_set;
    double xDPI, yDPI;

    SIZEL sizeWnd;
    SIZEL sizeView;
    float PixelsX;
    float PixelsY;
    float MMX;
    float MMY;
    float dwInchesX;
    float dwInchesY;
    POINTL winorg;
    POINTL vieworg;
    double ScaleX, ScaleY;

    int n_obj;
    PEMF_OBJECT emf_obj;
} EMF_CALLBACK_DATA, *PEMF_CALLBACK_DATA;


static void
output_style(PEMF_CALLBACK_DATA d, int iType)
{
    SVGOStringStream tmp_style;
    char tmp[1024] = {0};

    *(d->outsvg) += "\n\tstyle=\"";
    if (iType == EMR_STROKEPATH || !d->fill_set) {
        tmp_style << "fill:none;";
    } else {
        float rgb[3];
        sp_color_get_rgb_floatv( &(d->style.fill.value.color), rgb );
        snprintf(tmp, 1023,
                 "fill:#%02x%02x%02x;",
                 SP_COLOR_F_TO_U(rgb[0]),
                 SP_COLOR_F_TO_U(rgb[1]),
                 SP_COLOR_F_TO_U(rgb[2]));
        tmp_style << tmp;
        snprintf(tmp, 1023,
                 "fill-rule:%s;",
                 d->style.fill_rule.value != 0 ? "evenodd" : "nonzero");
        tmp_style << tmp;
        tmp_style << "fill-opacity:1;";
    }

    if (iType == EMR_FILLPATH || !d->stroke_set) {
        tmp_style << "stroke:none;";
    } else {
        float rgb[3];
        sp_color_get_rgb_floatv(&(d->style.stroke.value.color), rgb);
        snprintf(tmp, 1023,
                 "stroke:#%02x%02x%02x;",
                 SP_COLOR_F_TO_U(rgb[0]),
                 SP_COLOR_F_TO_U(rgb[1]),
                 SP_COLOR_F_TO_U(rgb[2]));
        tmp_style << tmp;

        tmp_style << "stroke-width:" <<
            MAX( 0.001, d->style.stroke_width.value ) << "px;";

        tmp_style << "stroke-linejoin:" <<
            (d->style.stroke_linejoin.computed == 0 ? "miter" :
             d->style.stroke_linejoin.computed == 1 ? "round" :
             d->style.stroke_linejoin.computed == 2 ? "bevel" :
             "unknown") << ";";

        if (d->style.stroke_linejoin.computed == 0) {
            tmp_style << "stroke-miterlimit:" <<
                MAX( 0.01, d->style.stroke_miterlimit.value ) << ";";
        }

        if (d->style.stroke_dasharray_set &&
            d->style.stroke_dash.n_dash && d->style.stroke_dash.dash)
        {
            tmp_style << "stroke-dasharray:";
            for (int i=0; i<d->style.stroke_dash.n_dash; i++) {
                if (i)
                    tmp_style << ",";
                tmp_style << d->style.stroke_dash.dash[i];
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
pix_x_to_point(PEMF_CALLBACK_DATA d, double px)
{
    double tmp = px - d->winorg.x;
    tmp *= (double) PX_PER_IN / d->ScaleX;
    tmp += d->vieworg.x;
    return tmp;
}


static double
pix_y_to_point(PEMF_CALLBACK_DATA d, double px)
{
    double tmp = px - d->winorg.y;
    tmp *= (double) PX_PER_IN / d->ScaleY;
    tmp += d->vieworg.y;
    return tmp;
}


static double
pix_size_to_point(PEMF_CALLBACK_DATA d, double px)
{
    double tmp = px;
    tmp *= (double) PX_PER_IN / d->ScaleX;
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

    switch (pEmr->lopn.lopnStyle) {
        default:
        {
            d->style.stroke_dasharray_set = 0;
            break;
        }
    }

    if (pEmr->lopn.lopnWidth.x) {
        d->style.stroke_width.value = pix_size_to_point( d, pEmr->lopn.lopnWidth.x );
    } else { // this stroke should always be rendered as 1 pixel wide, independent of zoom level (can that be done in SVG?)
        d->style.stroke_width.value = 1.0;
    }

    double r, g, b;
    r = SP_COLOR_U_TO_F( GetRValue(pEmr->lopn.lopnColor) );
    g = SP_COLOR_U_TO_F( GetGValue(pEmr->lopn.lopnColor) );
    b = SP_COLOR_U_TO_F( GetBValue(pEmr->lopn.lopnColor) );
    sp_color_set_rgb_float( &(d->style.stroke.value.color), r,g,b );

    d->style.stroke_linejoin.computed = 1;

    d->stroke_set = true;
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
                d->style.stroke_dash.n_dash = pEmr->elp.elpNumEntries;
                if (d->style.stroke_dash.dash)
                    delete[] d->style.stroke_dash.dash;
                d->style.stroke_dash.dash = new double[pEmr->elp.elpNumEntries];
                for (unsigned int i=0; i<pEmr->elp.elpNumEntries; i++) {
                    d->style.stroke_dash.dash[i] = pix_size_to_point( d, pEmr->elp.elpStyleEntry[i] );
                }
                d->style.stroke_dasharray_set = 1;
            } else {
                d->style.stroke_dasharray_set = 0;
            }
            break;
        }
        default:
        {
            d->style.stroke_dasharray_set = 0;
            break;
        }
    }

    switch (pEmr->elp.elpPenStyle & PS_ENDCAP_MASK) {
        case PS_ENDCAP_ROUND:
        {
            d->style.stroke_linecap.computed = 1;
            break;
        }
        case PS_ENDCAP_SQUARE:
        {
            d->style.stroke_linecap.computed = 2;
            break;
        }
        case PS_ENDCAP_FLAT:
        default:
        {
            d->style.stroke_linecap.computed = 0;
            break;
        }
    }

    switch (pEmr->elp.elpPenStyle & PS_JOIN_MASK) {
        case PS_JOIN_BEVEL:
        {
            d->style.stroke_linejoin.computed = 2;
            break;
        }
        case PS_JOIN_MITER:
        {
            d->style.stroke_linejoin.computed = 0;
            break;
        }
        case PS_JOIN_ROUND:
        default:
        {
            d->style.stroke_linejoin.computed = 1;
            break;
        }
    }

    d->style.stroke_width.value = pix_size_to_point( d, pEmr->elp.elpWidth );

    double r, g, b;
    r = SP_COLOR_U_TO_F( GetRValue(pEmr->elp.elpColor) );
    g = SP_COLOR_U_TO_F( GetGValue(pEmr->elp.elpColor) );
    b = SP_COLOR_U_TO_F( GetBValue(pEmr->elp.elpColor) );

    sp_color_set_rgb_float( &(d->style.stroke.value.color), r,g,b );

    d->stroke_set = true;
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
        sp_color_set_rgb_float( &(d->style.fill.value.color), r,g,b );
    }

    d->fill_set = true;
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
        d->emf_obj[index].lpEMFR = pObj;
    }
}


static int CALLBACK
myEnhMetaFileProc(HDC hDC, HANDLETABLE *lpHTable, ENHMETARECORD *lpEMFR, int nObj, LPARAM lpData)
{
    PEMF_CALLBACK_DATA d;
    SVGOStringStream tmp_outsvg;
    SVGOStringStream tmp_path;
    SVGOStringStream tmp_str;

    d = (PEMF_CALLBACK_DATA) lpData;

    switch (lpEMFR->iType)
    {
        case EMR_HEADER:
        {
            ENHMETAHEADER *pEmr = (ENHMETAHEADER *) lpEMFR;
            tmp_outsvg << "<svg\n";

            d->xDPI = 2540;
            d->yDPI = 2540;

            d->PixelsX = pEmr->rclFrame.right - pEmr->rclFrame.left;
            d->PixelsY = pEmr->rclFrame.bottom - pEmr->rclFrame.top;

            d->MMX = d->PixelsX / 100.0;
            d->MMY = d->PixelsY / 100.0;

            tmp_outsvg <<
                "  width=\"" << d->MMX << "mm\"\n" <<
                "  height=\"" << d->MMY << "mm\">\n";

            if (pEmr->nHandles) {
                d->n_obj = pEmr->nHandles;
                d->emf_obj = new EMF_OBJECT[d->n_obj];
            } else {
                d->emf_obj = NULL;
            }

            break;
        }
        case EMR_POLYBEZIER:
        {
            PEMRPOLYBEZIER pEmr = (PEMRPOLYBEZIER) lpEMFR;
            DWORD i,j;

            if (pEmr->cptl<4)
                break;

            *(d->outsvg) += "    <path ";
            output_style(d, EMR_STROKEPATH);
            *(d->outsvg) += "\n\td=\"";

            tmp_str <<
                "\n\tM " <<
                pix_x_to_point( d, pEmr->aptl[0].x ) << " " <<
                pix_x_to_point( d, pEmr->aptl[0].y) << " ";

            for (i=1; i<pEmr->cptl; ) {
                tmp_str << "\n\tC ";
                for (j=0; j<3 && i<pEmr->cptl; j++,i++) {
                    tmp_str <<
                        pix_x_to_point( d, pEmr->aptl[i].x ) << " " <<
                        pix_y_to_point( d, pEmr->aptl[i].y ) << " ";
                }
            }

            *(d->outsvg) += tmp_str.str().c_str();
            *(d->outsvg) += " \" /> \n";

            break;
        }
        case EMR_POLYGON:
        {
            EMRPOLYGON *pEmr = (EMRPOLYGON *) lpEMFR;
            DWORD i;

            if (pEmr->cptl < 2)
                break;

            *(d->outsvg) += "    <path ";
            output_style(d, EMR_STROKEANDFILLPATH);
            *(d->outsvg) += "\n\td=\"";

            tmp_str <<
                "\n\tM " <<
                pix_x_to_point( d, pEmr->aptl[0].x ) << " " <<
                pix_y_to_point( d, pEmr->aptl[0].y ) << " ";

            for (i=1; i<pEmr->cptl; i++) {
                tmp_str <<
                    "\n\tL " <<
                    pix_x_to_point( d, pEmr->aptl[i].x ) << " " <<
                    pix_y_to_point( d, pEmr->aptl[i].y ) << " ";
            }

            *(d->outsvg) += tmp_str.str().c_str();
            *(d->outsvg) += " z \" /> \n";

            break;
        }
        case EMR_POLYLINE:
        {
            EMRPOLYLINE *pEmr = (EMRPOLYLINE *) lpEMFR;
            DWORD i;

            if (pEmr->cptl<2)
                break;

            *(d->outsvg) += "    <path ";
            output_style(d, EMR_STROKEPATH);
            *(d->outsvg) += "\n\td=\"";

            tmp_str <<
                "\n\tM " <<
                pix_x_to_point( d, pEmr->aptl[0].x ) << " " <<
                pix_y_to_point( d, pEmr->aptl[0].y ) << " ";

            for (i=1; i<pEmr->cptl; i++) {
                tmp_str <<
                    "\n\tL " <<
                    pix_x_to_point( d, pEmr->aptl[i].x ) << " " <<
                    pix_y_to_point( d, pEmr->aptl[i].y ) << " ";
            }

            *(d->outsvg) += tmp_str.str().c_str();
            *(d->outsvg) += " \" /> \n";

            break;
        }
        case EMR_POLYBEZIERTO:
        {
            PEMRPOLYBEZIERTO pEmr = (PEMRPOLYBEZIERTO) lpEMFR;
            DWORD i,j;

            for (i=0; i<pEmr->cptl;) {
                tmp_path << "\n\tC ";
                for (j=0; j<3 && i<pEmr->cptl; j++,i++) {
                    tmp_path <<
                        pix_x_to_point( d, pEmr->aptl[i].x ) << " " <<
                        pix_y_to_point( d, pEmr->aptl[i].y ) << " ";
                }
            }

            break;
        }
        case EMR_POLYLINETO:
        {
            PEMRPOLYLINETO pEmr = (PEMRPOLYLINETO) lpEMFR;
            DWORD i;

            for (i=0; i<pEmr->cptl;i++) {
                tmp_path <<
                    "\n\tL " <<
                    pix_x_to_point( d, pEmr->aptl[i].x ) << " " <<
                    pix_y_to_point( d, pEmr->aptl[i].y ) << " ";
            }

            break;
        }
        case EMR_POLYPOLYLINE:
            break;
        case EMR_POLYPOLYGON:
            break;
        case EMR_SETWINDOWEXTEX:
        {
            PEMRSETWINDOWEXTEX pEmr = (PEMRSETWINDOWEXTEX) lpEMFR;

            d->sizeWnd = pEmr->szlExtent;
            d->PixelsX = d->sizeWnd.cx;
            d->PixelsY = d->sizeWnd.cy;

            d->ScaleX = d->xDPI / (100*d->MMX / d->PixelsX);
            d->ScaleY = d->yDPI / (100*d->MMY / d->PixelsY);

            break;
        }
        case EMR_SETWINDOWORGEX:
        {
            PEMRSETWINDOWORGEX pEmr = (PEMRSETWINDOWORGEX) lpEMFR;
            d->winorg = pEmr->ptlOrigin;
            break;
        }
        case EMR_SETVIEWPORTEXTEX:
        {
            PEMRSETVIEWPORTEXTEX pEmr = (PEMRSETVIEWPORTEXTEX) lpEMFR;

            d->sizeView = pEmr->szlExtent;

            if (d->sizeWnd.cx && d->sizeWnd.cy) {
                HDC hScreenDC = GetDC( NULL );

                float scrPixelsX = (float)GetDeviceCaps( hScreenDC, HORZRES );
                float scrPixelsY = (float)GetDeviceCaps( hScreenDC, VERTRES );
                float scrMMX = (float)GetDeviceCaps( hScreenDC, HORZSIZE );
                float scrMMY = (float)GetDeviceCaps( hScreenDC, VERTSIZE );

                ReleaseDC( NULL, hScreenDC );

                d->dwInchesX = d->sizeView.cx / (25.4f*scrPixelsX/scrMMX);
                d->dwInchesY = d->sizeView.cy / (25.4f*scrPixelsY/scrMMY);
                d->xDPI = d->sizeWnd.cx / d->dwInchesX;
                d->yDPI = d->sizeWnd.cy / d->dwInchesY;

                if (1) {
                    d->xDPI = 2540;
                    d->yDPI = 2540;
                    d->dwInchesX = d->PixelsX / d->xDPI;
                    d->dwInchesY = d->PixelsY / d->yDPI;
                    d->ScaleX = d->xDPI;
                    d->ScaleY = d->yDPI;
                }

                d->MMX = d->dwInchesX * MM_PER_IN;
                d->MMY = d->dwInchesY * MM_PER_IN;
            }

            break;
        }
        case EMR_SETVIEWPORTORGEX:
        {
            PEMRSETVIEWPORTORGEX pEmr = (PEMRSETVIEWPORTORGEX) lpEMFR;
            d->vieworg = pEmr->ptlOrigin;
            break;
        }
        case EMR_SETBRUSHORGEX:
            break;
        case EMR_EOF:
        {
            tmp_outsvg << "</svg>\n";
            break;
        }
        case EMR_SETPIXELV:
            break;
        case EMR_SETMAPPERFLAGS:
            break;
        case EMR_SETMAPMODE:
            break;
        case EMR_SETBKMODE:
            break;
        case EMR_SETPOLYFILLMODE:
        {
            PEMRSETPOLYFILLMODE pEmr = (PEMRSETPOLYFILLMODE) lpEMFR;
            d->style.fill_rule.value =
                (pEmr->iMode == WINDING ? 0 :
                 pEmr->iMode == ALTERNATE ? 1 : 0);
            break;
        }
        case EMR_SETROP2:
            break;
        case EMR_SETSTRETCHBLTMODE:
            break;
        case EMR_SETTEXTALIGN:
            break;
        case EMR_SETCOLORADJUSTMENT:
            break;
        case EMR_SETTEXTCOLOR:
            break;
        case EMR_SETBKCOLOR:
            break;
        case EMR_OFFSETCLIPRGN:
            break;
        case EMR_MOVETOEX:
        {
            PEMRMOVETOEX pEmr = (PEMRMOVETOEX) lpEMFR;
            tmp_path <<
                "\n\tM " <<
                pix_x_to_point( d, pEmr->ptl.x ) << " " <<
                pix_y_to_point( d, pEmr->ptl.y ) << " ";
            break;
        }
        case EMR_SETMETARGN:
            break;
        case EMR_EXCLUDECLIPRECT:
            break;
        case EMR_INTERSECTCLIPRECT:
            break;
        case EMR_SCALEVIEWPORTEXTEX:
            break;
        case EMR_SCALEWINDOWEXTEX:
            break;
        case EMR_SAVEDC:
            break;
        case EMR_RESTOREDC:
            break;
        case EMR_SETWORLDTRANSFORM:
            break;
        case EMR_MODIFYWORLDTRANSFORM:
            break;
        case EMR_SELECTOBJECT:
        {
            PEMRSELECTOBJECT pEmr = (PEMRSELECTOBJECT) lpEMFR;
            unsigned int index = pEmr->ihObject;

            if (index >= ENHMETA_STOCK_OBJECT) {
                index -= ENHMETA_STOCK_OBJECT;
                switch (index) {
                    case NULL_BRUSH:
                        d->fill_set = false;
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
                        sp_color_set_rgb_float( &(d->style.fill.value.color), val,val,val );

                        d->fill_set = true;
                        break;
                    }
                    case NULL_PEN:
                        d->stroke_set = false;
                        break;
                    case BLACK_PEN:
                    case WHITE_PEN:
                    {
                        float val = index == BLACK_PEN ? 0 : 1;
                        d->style.stroke_dasharray_set = 0;
                        d->style.stroke_width.value = 1.0;
                        sp_color_set_rgb_float( &(d->style.stroke.value.color), val,val,val );

                        d->stroke_set = true;

                        break;
                    }
                }
            } else {
                if (index >= 0 && index < d->n_obj) {
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
                    }
                }
            }
            break;
        }
        case EMR_CREATEPEN:
        {
            PEMRCREATEPEN pEmr = (PEMRCREATEPEN) lpEMFR;
            int index = pEmr->ihPen;

            EMRCREATEPEN *pPen =
                (EMRCREATEPEN *) malloc( sizeof(EMREXTCREATEPEN) );
            pPen->lopn = pEmr->lopn;
            insert_object(d, index, EMR_CREATEPEN, (ENHMETARECORD *) pPen);

            break;
        }
        case EMR_CREATEBRUSHINDIRECT:
        {
            PEMRCREATEBRUSHINDIRECT pEmr = (PEMRCREATEBRUSHINDIRECT) lpEMFR;
            int index = pEmr->ihBrush;

            EMRCREATEBRUSHINDIRECT *pBrush =
                (EMRCREATEBRUSHINDIRECT *) malloc( sizeof(EMRCREATEBRUSHINDIRECT) );
            pBrush->lb = pEmr->lb;
            insert_object(d, index, EMR_CREATEBRUSHINDIRECT, (ENHMETARECORD *) pBrush);

            break;
        }
        case EMR_DELETEOBJECT:
            break;
        case EMR_ANGLEARC:
            break;
        case EMR_ELLIPSE:
            break;
        case EMR_RECTANGLE:
            break;
        case EMR_ROUNDRECT:
            break;
        case EMR_ARC:
            break;
        case EMR_CHORD:
            break;
        case EMR_PIE:
            break;
        case EMR_SELECTPALETTE:
            break;
        case EMR_CREATEPALETTE:
            break;
        case EMR_SETPALETTEENTRIES:
            break;
        case EMR_RESIZEPALETTE:
            break;
        case EMR_REALIZEPALETTE:
            break;
        case EMR_EXTFLOODFILL:
            break;
        case EMR_LINETO:
        {
            PEMRLINETO pEmr = (PEMRLINETO) lpEMFR;
            tmp_path <<
                "\n\tL " <<
                pix_x_to_point( d, pEmr->ptl.x ) << " " <<
                pix_y_to_point( d, pEmr->ptl.y ) << " ";
            break;
        }
        case EMR_ARCTO:
            break;
        case EMR_POLYDRAW:
            break;
        case EMR_SETARCDIRECTION:
            break;
        case EMR_SETMITERLIMIT:
        {
            PEMRSETMITERLIMIT pEmr = (PEMRSETMITERLIMIT) lpEMFR;
            d->style.stroke_miterlimit.value = pix_size_to_point( d, pEmr->eMiterLimit );

            if (d->style.stroke_miterlimit.value < 1)
                d->style.stroke_miterlimit.value = 1.0;

            break;
        }
        case EMR_BEGINPATH:
        {
            tmp_path << " d=\"";
            *(d->path) = "";
            break;
        }
        case EMR_ENDPATH:
        {
            tmp_path << "\"";
            break;
        }
        case EMR_CLOSEFIGURE:
        {
            tmp_path << "\n\tz";
            break;
        }
        case EMR_FILLPATH:
        case EMR_STROKEANDFILLPATH:
        case EMR_STROKEPATH:
        {
            *(d->outsvg) += "    <path ";
            output_style(d, lpEMFR->iType);
            *(d->outsvg) += "\n\t";
            *(d->outsvg) += *(d->path);
            *(d->outsvg) += " /> \n";
            break;
        }
        case EMR_FLATTENPATH:
            break;
        case EMR_WIDENPATH:
            break;
        case EMR_SELECTCLIPPATH:
            break;
        case EMR_ABORTPATH:
            break;
        case EMR_GDICOMMENT:
            break;
        case EMR_FILLRGN:
            break;
        case EMR_FRAMERGN:
            break;
        case EMR_INVERTRGN:
            break;
        case EMR_PAINTRGN:
            break;
        case EMR_EXTSELECTCLIPRGN:
            break;
        case EMR_BITBLT:
            break;
        case EMR_STRETCHBLT:
            break;
        case EMR_MASKBLT:
            break;
        case EMR_PLGBLT:
            break;
        case EMR_SETDIBITSTODEVICE:
            break;
        case EMR_STRETCHDIBITS:
            break;
        case EMR_EXTCREATEFONTINDIRECTW:
            break;
        case EMR_EXTTEXTOUTA:
            break;
        case EMR_EXTTEXTOUTW:
            break;
        case EMR_POLYBEZIER16:
        {
            PEMRPOLYBEZIER16 pEmr = (PEMRPOLYBEZIER16) lpEMFR;
            POINTS *apts = (POINTS *) pEmr->apts; // Bug in MinGW wingdi.h ?
            DWORD i,j;

            if (pEmr->cpts<4)
                break;

            *(d->outsvg) += "    <path ";
            output_style(d, EMR_STROKEPATH);
            *(d->outsvg) += "\n\td=\"";

            tmp_str <<
                "\n\tM " <<
                pix_x_to_point( d, apts[0].x ) << " " <<
                pix_y_to_point( d, apts[0].y ) << " ";

            for (i=1; i<pEmr->cpts; ) {
                tmp_str << "\n\tC ";
                for (j=0; j<3 && i<pEmr->cpts; j++,i++) {
                    tmp_str <<
                        pix_x_to_point( d, apts[i].x ) << " " <<
                        pix_y_to_point( d, apts[i].y ) << " ";
                }
            }

            *(d->outsvg) += tmp_str.str().c_str();
            *(d->outsvg) += " \" /> \n";

            break;
        }
        case EMR_POLYGON16:
        {
            PEMRPOLYGON16 pEmr = (PEMRPOLYGON16) lpEMFR;
            POINTS *apts = (POINTS *) pEmr->apts; // Bug in MinGW wingdi.h ?
            unsigned int i;

            *(d->outsvg) += "<path ";
            output_style(d, EMR_STROKEANDFILLPATH);
            *(d->outsvg) += "\n\td=\"";

            // skip the first point?
            tmp_path << "\n\tM " <<
                pix_x_to_point( d, apts[1].x ) << " " <<
                pix_y_to_point( d, apts[1].y ) << " ";

            for (i=2; i<pEmr->cpts; i++) {
                tmp_path << "\n\tL " <<
                    pix_x_to_point( d, apts[i].x ) << " " <<
                    pix_y_to_point( d, apts[i].y ) << " ";
            }

            *(d->outsvg) += tmp_path.str().c_str();
            *(d->outsvg) += " z \" /> \n";

            break;
        }
        case EMR_POLYLINE16:
        {
            EMRPOLYLINE16 *pEmr = (EMRPOLYLINE16 *) lpEMFR;
            POINTS *apts = (POINTS *) pEmr->apts; // Bug in MinGW wingdi.h ?
            DWORD i;

            if (pEmr->cpts<2)
                break;

            *(d->outsvg) += "    <path ";
            output_style(d, EMR_STROKEPATH);
            *(d->outsvg) += "\n\td=\"";

            tmp_str <<
                "\n\tM " <<
                pix_x_to_point( d, apts[0].x ) << " " <<
                pix_y_to_point( d, apts[0].y ) << " ";

            for (i=1; i<pEmr->cpts; i++) {
                tmp_str <<
                    "\n\tL " <<
                    pix_x_to_point( d, apts[i].x ) << " " <<
                    pix_y_to_point( d, apts[i].y ) << " ";
            }

            *(d->outsvg) += tmp_str.str().c_str();
            *(d->outsvg) += " \" /> \n";

            break;
        }
        case EMR_POLYBEZIERTO16:
        {
            PEMRPOLYBEZIERTO16 pEmr = (PEMRPOLYBEZIERTO16) lpEMFR;
            POINTS *apts = (POINTS *) pEmr->apts; // Bug in MinGW wingdi.h ?
            DWORD i,j;

            for (i=0; i<pEmr->cpts;) {
                tmp_path << "\n\tC ";
                for (j=0; j<3 && i<pEmr->cpts; j++,i++) {
                    tmp_path <<
                        pix_x_to_point( d, apts[i].x ) << " " <<
                        pix_y_to_point( d, apts[i].y ) << " ";
                }
            }

            break;
        }
        case EMR_POLYLINETO16:
        {
            PEMRPOLYLINETO16 pEmr = (PEMRPOLYLINETO16) lpEMFR;
            POINTS *apts = (POINTS *) pEmr->apts; // Bug in MinGW wingdi.h ?
            DWORD i;

            for (i=0; i<pEmr->cpts;i++) {
                tmp_path <<
                    "\n\tL " <<
                    pix_x_to_point( d, apts[i].x ) << " " <<
                    pix_y_to_point( d, apts[i].y ) << " ";
            }

            break;
        }
        case EMR_POLYPOLYLINE16:
            break;
        case EMR_POLYPOLYGON16:
        {
            PEMRPOLYPOLYGON16 pEmr = (PEMRPOLYPOLYGON16) lpEMFR;
            unsigned int n, i, j;

            *(d->outsvg) += "<path ";
            output_style(d, EMR_STROKEANDFILLPATH);
            *(d->outsvg) += "\n\td=\"";

            i = pEmr->nPolys-1; // ???
            for (n=0; n<pEmr->nPolys /*&& i<pEmr->cpts*/; n++) {
                SVGOStringStream poly_path;

                poly_path << "\n\tM " <<
                    pix_x_to_point( d, pEmr->apts[i].x ) << " " <<
                    pix_y_to_point( d, pEmr->apts[i].y ) << " ";
                i++;

                for (j=1; j<pEmr->aPolyCounts[n] /*&& i<pEmr->cpts*/; j++) {
                    poly_path << "\n\tL " <<
                        pix_x_to_point( d, pEmr->apts[i].x ) << " " <<
                        pix_y_to_point( d, pEmr->apts[i].y ) << " ";
                    i++;
                }

                *(d->outsvg) += poly_path.str().c_str();
                *(d->outsvg) += " z \n";
            }

            *(d->outsvg) += " \" /> \n";
            break;
        }
        case EMR_POLYDRAW16:
            break;
        case EMR_CREATEMONOBRUSH:
            break;
        case EMR_CREATEDIBPATTERNBRUSHPT:
            break;
        case EMR_EXTCREATEPEN:
        {
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
            break;
        case EMR_POLYTEXTOUTW:
            break;
        case EMR_SETICMMODE:
            break;
        case EMR_CREATECOLORSPACE:
            break;
        case EMR_SETCOLORSPACE:
            break;
        case EMR_DELETECOLORSPACE:
            break;
        case EMR_GLSRECORD:
            break;
        case EMR_GLSBOUNDEDRECORD:
            break;
        case EMR_PIXELFORMAT:
            break;
    }

    *(d->outsvg) += tmp_outsvg.str().c_str();
    *(d->path) += tmp_path.str().c_str();

    return 1;
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
	DWORD		dwKey;
	WORD		hmf;
	SMALL_RECT	bbox;
	WORD		wInch;
	DWORD		dwReserved;
	WORD		wCheckSum;
} APMHEADER, *PAPMHEADER;
#pragma pack( pop )


SPDocument *
EmfWin32::open( Inkscape::Extension::Input *mod, const gchar *uri )
{
    EMF_CALLBACK_DATA d = {0};

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

    // Try open as Enhanced Metafile
    HENHMETAFILE hemf;
    if (PrintWin32::is_os_wide())
        hemf = GetEnhMetaFileW(unicode_uri);
    else
        hemf = GetEnhMetaFileA(ansi_uri);

    if (!hemf) {
        // Try open as Windows Metafile
        HMETAFILE hmf;
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
                    }
                    delete[] lpvData;
                }
                DeleteMetaFile( hmf );
            }
        } else {
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

    if (!hemf || !d.outsvg || !d.path) {
        if (d.outsvg)
            delete d.outsvg;
        if (d.path)
            delete d.path;
        if  (local_fn)
            g_free(local_fn);
        if  (unicode_fn)
            g_free(unicode_fn);
        return NULL;
    }

    EnumEnhMetaFile(NULL, hemf, myEnhMetaFileProc, (LPVOID) &d, NULL);
    DeleteEnhMetaFile(hemf);

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
    
    if (d.style.stroke_dash.dash)
        delete[] d.style.stroke_dash.dash;

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
        "<inkscape-extension>\n"
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
        "<inkscape-extension>\n"
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
        "<inkscape-extension>\n"
            "<name>" N_("EMF Output") "</name>\n"
            "<id>org.inkscape.output.emf.win32</id>\n"
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
