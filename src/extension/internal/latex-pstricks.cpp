#define __SP_LATEX_C__

/*
 * LaTeX Printing
 *
 * Author:
 *  Michael Forbes <miforbes@mbhs.edu>
 *
 * Copyright (C) 2004 Authors
 * 
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


#include <signal.h>
#include <errno.h>

#include "libnr/n-art-bpath.h"
#include "sp-item.h"


#include "style.h"

#include "latex-pstricks.h"

#include <unit-constants.h>

#include "extension/system.h"
#include "extension/print.h"

#include "io/sys.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

PrintLatex::PrintLatex (void): _stream(NULL)
{
}

PrintLatex::~PrintLatex (void)
{
    if (_stream) fclose(_stream);

    /* restore default signal handling for SIGPIPE */
#if !defined(_WIN32) && !defined(__WIN32__)
    (void) signal(SIGPIPE, SIG_DFL);
#endif
	return;
}

unsigned int
PrintLatex::setup (Inkscape::Extension::Print *mod)
{
    return TRUE;
}

unsigned int
PrintLatex::begin (Inkscape::Extension::Print *mod, SPDocument *doc)
{
    Inkscape::SVGOStringStream os;
    int res;
    FILE *osf, *osp;
    const gchar * fn;

    fn = mod->get_param_string("destination");

    osf = NULL;
    osp = NULL;

    gsize bytesRead = 0;
    gsize bytesWritten = 0;
    GError* error = NULL;
    gchar* local_fn = g_filename_from_utf8( fn,
                                            -1,  &bytesRead,  &bytesWritten, &error);
    fn = local_fn;

    /* TODO: Replace the below fprintf's with something that does the right thing whether in
     * gui or batch mode (e.g. --print=blah).  Consider throwing an exception: currently one of
     * the callers (sp_print_document_to_file, "ret = mod->begin(doc)") wrongly ignores the
     * return code.
     */
    if (fn != NULL) {
        while (isspace(*fn)) fn += 1;
        Inkscape::IO::dump_fopen_call(fn, "K");
        osf = Inkscape::IO::fopen_utf8name(fn, "w+");
        if (!osf) {
            fprintf(stderr, "inkscape: fopen(%s): %s\n",
                    fn, strerror(errno));
            return 0;
        }
        _stream = osf;
    }

    g_free(local_fn);

    if (_stream) {
        /* fixme: this is kinda icky */
#if !defined(_WIN32) && !defined(__WIN32__)
        (void) signal(SIGPIPE, SIG_IGN);
#endif
    }

    res = fprintf(_stream, "%%LaTeX with PSTricks extensions\n");
    /* flush this to test output stream as early as possible */
    if (fflush(_stream)) {
        /*g_print("caught error in sp_module_print_plain_begin\n");*/
        if (ferror(_stream)) {
            g_print("Error %d on output stream: %s\n", errno,
                    g_strerror(errno));
        }
        g_print("Printing failed\n");
        /* fixme: should use pclose() for pipes */
        fclose(_stream);
        _stream = NULL;
        fflush(stdout);
        return 0;
    }

    // width and height in pt
    _width = sp_document_width(doc) * PT_PER_PX;
    _height = sp_document_height(doc) * PT_PER_PX;

    NRRect d;
    bool   pageLandscape;
    // printf("Page Bounding Box: %s\n", pageBoundingBox ? "TRUE" : "FALSE");
/*    if (pageBoundingBox) {
       d.x0 = d.y0 = 0;
        d.x1 = ceil(_width);
        d.y1 = ceil(_height);
    } else */{ // no bounding boxes for now
        SPItem* doc_item = SP_ITEM(sp_document_root(doc));
        sp_item_invoke_bbox(doc_item, &d, sp_item_i2r_affine(doc_item), TRUE);
        // convert from px to pt
        d.x0 *= PT_PER_PX;
        d.x1 *= PT_PER_PX;
        d.y0 *= PT_PER_PX;
        d.y1 *= PT_PER_PX;
    }


    if (res >= 0) {

        os << "%%Creator: " << PACKAGE_STRING << "\n";
	os << "%%Please note this file requires PSTricks extensions\n";

        // 2004 Dec 10, BFC:
        // The point of the following code is (1) to do the thing that's expected by users
        // who have done File>New>A4_landscape or ...letter_landscape (i.e., rotate
        // the output), while (2) not messing up users who simply want their output wider
        // than it is tall (e.g., small figures for inclusion in LaTeX).
        // The original patch by WQ only had the w>h condition.
        {
             double w = (d.x1 - d.x0); // width and height of bounding box, in pt
             double h = (d.y1 - d.y0);
             pageLandscape = (
                 (w > 0. && h > 0.) // empty documents fail this sanity check, have w<0, h<0
                 && (w > h)   // implies, but does not prove, the user wanted landscape
                 && (w > 600) // approximate maximum printable width of an A4
             )
             ? true : false;
        }

        if (pageLandscape) {
            os << "\\rotate{90}\n";
        }

        os << "\\psset{xunit=.5pt,yunit=.5pt,runit=.5pt}\n";
        // from now on we can output px, but they will be treated as pt
    
        os << "\\begin{pspicture}(" << sp_document_width(doc) << "," << sp_document_height(doc) << ")\n";
    }

    return fprintf(_stream, "%s", os.str().c_str());
}

unsigned int
PrintLatex::finish (Inkscape::Extension::Print *mod)
{
    int res;

    if (!_stream) return 0;

    res = fprintf(_stream, "\\end{pspicture}\n");

    /* Flush stream to be sure. */
    (void) fflush(_stream);

    fclose(_stream);
    _stream = NULL;
    return 0;
}

unsigned int PrintLatex::comment (Inkscape::Extension::Print * module,
		                  const char * comment)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.

    return fprintf(_stream, "%%! %s\n",comment);
}

unsigned int
PrintLatex::fill(Inkscape::Extension::Print *mod,
		 NRBPath const *bpath, NRMatrix const *transform, SPStyle const *style,
		 NRRect const *pbox, NRRect const *dbox, NRRect const *bbox)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.

    if (style->fill.type == SP_PAINT_TYPE_COLOR) {
        Inkscape::SVGOStringStream os;
        float rgb[3];

        sp_color_get_rgb_floatv(&style->fill.value.color, rgb);
        os << "{\n\\newrgbcolor{curcolor}{" << rgb[0] << " " << rgb[1] << " " << rgb[2] << "}\n";

        os << "\\pscustom[fillstyle=solid,fillcolor=curcolor]\n{\n";

        print_bpath(os, bpath->path, transform);

        os << "}\n}\n";

        fprintf(_stream, "%s", os.str().c_str());
    }        

    return 0;
}

unsigned int
PrintLatex::stroke (Inkscape::Extension::Print *mod, const NRBPath *bpath, const NRMatrix *transform, const SPStyle *style,
			      const NRRect *pbox, const NRRect *dbox, const NRRect *bbox)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.

    if (style->stroke.type == SP_PAINT_TYPE_COLOR) {
        Inkscape::SVGOStringStream os;
        float rgb[3];

        sp_color_get_rgb_floatv(&style->stroke.value.color, rgb);
        os << "{\n\\newrgbcolor{curcolor}{" << rgb[0] << " " << rgb[1] << " " << rgb[2] << "}\n";

        os << "\\pscustom[linewidth=" << style->stroke_width.computed<< ",linecolor=curcolor";

        if (style->stroke_dasharray_set &&
                style->stroke_dash.n_dash &&
                style->stroke_dash.dash) {
            int i;
            os << ",linestyle=dashed,dash=";
            for (i = 0; i < style->stroke_dash.n_dash; i++) {
                if ((i)) {
                    os << " ";
                }
                os << style->stroke_dash.dash[i];
            }
        }

        os <<"]\n{\n";

        print_bpath(os, bpath->path, transform);

        os << "}\n}\n";

        fprintf(_stream, "%s", os.str().c_str());
    }

    return 0;
}

void
PrintLatex::print_bpath(SVGOStringStream &os, const NArtBpath *bp, const NRMatrix *transform)
{
    unsigned int closed;
    NR::Matrix tf=*transform;


    os << "\\newpath\n";
    closed = FALSE;
    while (bp->code != NR_END) {
        using NR::X;
        using NR::Y;
        NR::Point const p1(bp->c(1) * tf);
        NR::Point const p2(bp->c(2) * tf);
        NR::Point const p3(bp->c(3) * tf);
        double const x1 = p1[X], y1 = p1[Y];
        double const x2 = p2[X], y2 = p2[Y];
        double const x3 = p3[X], y3 = p3[Y];
        
        switch (bp->code) {
            case NR_MOVETO:
                if (closed) {
                    os << "\\closepath\n";
                }
                closed = TRUE;
                os << "\\moveto(" << x3 << "," << y3 << ")\n";
                break;
            case NR_MOVETO_OPEN:
                if (closed) {
                    os << "\\closepath\n";
                }
                closed = FALSE;
                os << "\\moveto(" << x3 << "," << y3 << ")\n";
                break;
            case NR_LINETO:
                os << "\\lineto(" << x3 << "," << y3 << ")\n";
                break;
            case NR_CURVETO:
                os << "\\curveto(" << x1 << "," << y1 << ")("
                   << x2 << "," << y2 << ")("
                   << x3 << "," << y3 << ")\n";
                break;
            default:
                break;
        }
        bp += 1;
    }
    if (closed) {
        os << "\\closepath\n";
    }
}

bool
PrintLatex::textToPath(Inkscape::Extension::Print * ext)
{
    return ext->get_param_bool("textToPath");
}

#include "clear-n_.h"

void
PrintLatex::init (void)
{
	Inkscape::Extension::Extension * ext;
	
	/* SVG in */
    ext = Inkscape::Extension::build_from_mem(
		"<inkscape-extension>\n"
			"<name>" N_("LaTeX Print") "</name>\n"
			"<id>" SP_MODULE_KEY_PRINT_LATEX "</id>\n"
        		"<param name=\"destination\" type=\"string\"></param>\n"
                        "<param name=\"textToPath\" type=\"boolean\">TRUE</param>\n"
			"<print/>\n"
		"</inkscape-extension>", new PrintLatex());

	return;
}

}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */

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

