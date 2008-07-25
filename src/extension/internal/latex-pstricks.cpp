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


#include "libnr/nr-matrix.h" 
#include "libnr/nr-matrix-ops.h" 
#include "libnr/nr-matrix-scale-ops.h"
#include "libnr/nr-matrix-translate-ops.h"
#include "libnr/nr-scale-translate-ops.h"
#include "libnr/nr-translate-scale-ops.h"
#include <libnr/nr-matrix-fns.h>
#include <2geom/pathvector.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/bezier-curve.h>
#include <2geom/hvlinesegment.h>

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

    os.setf(std::ios::fixed);

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

    if (res >= 0) {

        os << "%%Creator: " << PACKAGE_STRING << "\n";
	os << "%%Please note this file requires PSTricks extensions\n";

        os << "\\psset{xunit=.5pt,yunit=.5pt,runit=.5pt}\n";
        // from now on we can output px, but they will be treated as pt
    
        os << "\\begin{pspicture}(" << sp_document_width(doc) << "," << sp_document_height(doc) << ")\n";
    }

    m_tr_stack.push( NR::scale(1, -1) * NR::translate(0, sp_document_height(doc)));

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

unsigned int
PrintLatex::bind(Inkscape::Extension::Print *mod, NR::Matrix const *transform, float opacity)
{
    NR::Matrix tr = *transform;
    
    if(m_tr_stack.size()){
        NR::Matrix tr_top = m_tr_stack.top();
        m_tr_stack.push(tr * tr_top);
    }else
        m_tr_stack.push(tr);

    return 1;
}

unsigned int
PrintLatex::release(Inkscape::Extension::Print *mod)
{
    m_tr_stack.pop();
    return 1;
}

unsigned int PrintLatex::comment (Inkscape::Extension::Print * module,
		                  const char * comment)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.

    return fprintf(_stream, "%%! %s\n",comment);
}

unsigned int
PrintLatex::fill(Inkscape::Extension::Print *mod,
        Geom::PathVector const &pathv, NR::Matrix const *transform, SPStyle const *style,
        NRRect const *pbox, NRRect const *dbox, NRRect const *bbox)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.

    if (style->fill.isColor()) {
        Inkscape::SVGOStringStream os;
        float rgb[3];

        os.setf(std::ios::fixed);

        sp_color_get_rgb_floatv(&style->fill.value.color, rgb);
        os << "{\n\\newrgbcolor{curcolor}{" << rgb[0] << " " << rgb[1] << " " << rgb[2] << "}\n";

        os << "\\pscustom[linestyle=none,fillstyle=solid,fillcolor=curcolor]\n{\n";

        print_pathvector(os, pathv, transform);

        os << "}\n}\n";

        fprintf(_stream, "%s", os.str().c_str());
    }        

    return 0;
}

unsigned int
PrintLatex::stroke (Inkscape::Extension::Print *mod, Geom::PathVector const &pathv, const NR::Matrix *transform, const SPStyle *style,
			      const NRRect *pbox, const NRRect *dbox, const NRRect *bbox)
{
    if (!_stream) return 0; // XXX: fixme, returning -1 as unsigned.

    if (style->stroke.isColor()) {
        Inkscape::SVGOStringStream os;
        float rgb[3];
        NR::Matrix tr_stack = m_tr_stack.top();
        double const scale = expansion(tr_stack);
        os.setf(std::ios::fixed);

        sp_color_get_rgb_floatv(&style->stroke.value.color, rgb);
        os << "{\n\\newrgbcolor{curcolor}{" << rgb[0] << " " << rgb[1] << " " << rgb[2] << "}\n";

        os << "\\pscustom[linewidth=" << style->stroke_width.computed*scale<< ",linecolor=curcolor";

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

        print_pathvector(os, pathv, transform);

        os << "}\n}\n";

        fprintf(_stream, "%s", os.str().c_str());
    }

    return 0;
}

// FIXME: why is 'transform' argument not used?
void
PrintLatex::print_pathvector(SVGOStringStream &os, Geom::PathVector const &pathv_in, const NR::Matrix * /*transform*/)
{
    if (pathv_in.empty())
        return;

//    NR::Matrix tf=*transform;   // why was this here?
    NR::Matrix tf_stack=m_tr_stack.top(); // and why is transform argument not used?
    Geom::PathVector pathv = pathv_in * to_2geom(tf_stack); // generates new path, which is a bit slow, but this doesn't have to be performance optimized

    os << "\\newpath\n";

    for(Geom::PathVector::const_iterator it = pathv.begin(); it != pathv.end(); ++it) {

        os << "\\moveto(" << it->initialPoint()[Geom::X] << "," << it->initialPoint()[Geom::Y] << ")\n";

        for(Geom::Path::const_iterator cit = it->begin(); cit != it->end_open(); ++cit) {
            print_2geomcurve(os, *cit);
        }

        if (it->closed()) {
            os << "\\closepath\n";
        }

    }
}

void
PrintLatex::print_2geomcurve(SVGOStringStream &os, Geom::Curve const & c )
{
    using Geom::X;
    using Geom::Y;

    if( dynamic_cast<Geom::LineSegment const*>(&c) ||
        dynamic_cast<Geom::HLineSegment const*>(&c) ||
        dynamic_cast<Geom::VLineSegment const*>(&c) )
    {
        os << "\\lineto(" << c.finalPoint()[X] << "," << c.finalPoint()[Y] << ")\n";
    }
    else if(Geom::CubicBezier const *cubic_bezier = dynamic_cast<Geom::CubicBezier const*>(&c)) {
        std::vector<Geom::Point> points = cubic_bezier->points();
        os << "\\curveto(" << points[1][X] << "," << points[1][Y] << ")("
                           << points[2][X] << "," << points[2][Y] << ")("
                           << points[3][X] << "," << points[3][Y] << ")\n";
    }                                             
    else {
        //this case handles sbasis as well as all other curve types
        Geom::Path sbasis_path = Geom::cubicbezierpath_from_sbasis(c.toSBasis(), 0.1);

        for(Geom::Path::iterator iter = sbasis_path.begin(); iter != sbasis_path.end(); ++iter) {
            print_2geomcurve(os, *iter);
        }
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
		"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
			"<name>" N_("LaTeX Print") "</name>\n"
			"<id>" SP_MODULE_KEY_PRINT_LATEX "</id>\n"
        		"<param name=\"destination\" type=\"string\"></param>\n"
                        "<param name=\"textToPath\" type=\"boolean\">true</param>\n"
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

