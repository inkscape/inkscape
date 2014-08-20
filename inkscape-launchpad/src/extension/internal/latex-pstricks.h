#ifndef __INKSCAPE_EXTENSION_INTERNAL_PRINT_LATEX_H__
#define __INKSCAPE_EXTENSION_INTERNAL_PRINT_LATEX_H__

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

#include <config.h>

#include "extension/implementation/implementation.h"
#include "extension/extension.h"

#include "svg/stringstream.h"

#include <stack>

namespace Inkscape {
namespace Extension {
namespace Internal {

class PrintLatex : public Inkscape::Extension::Implementation::Implementation {

    float  _width;
    float  _height;
    FILE * _stream;
    
    std::stack<Geom::Affine> m_tr_stack;

    void print_pathvector(SVGOStringStream &os, Geom::PathVector const &pathv_in, const Geom::Affine & /*transform*/);
    void print_2geomcurve(SVGOStringStream &os, Geom::Curve const & c );

public:
        PrintLatex (void);
        virtual ~PrintLatex (void);

        /* Print functions */
        virtual unsigned int setup (Inkscape::Extension::Print * module);

        virtual unsigned int begin (Inkscape::Extension::Print * module, SPDocument *doc);
        virtual unsigned int finish (Inkscape::Extension::Print * module);

        /* Rendering methods */
        virtual unsigned int bind(Inkscape::Extension::Print *module, Geom::Affine const &transform, float opacity);
        virtual unsigned int release(Inkscape::Extension::Print *module);

        virtual unsigned int fill (Inkscape::Extension::Print *module, Geom::PathVector const &pathv,
                                   Geom::Affine const &ctm, SPStyle const *style,
                                   Geom::OptRect const &pbox, Geom::OptRect const &dbox,
                                   Geom::OptRect const &bbox);
        virtual unsigned int stroke (Inkscape::Extension::Print *module, Geom::PathVector const &pathv,
                                     Geom::Affine const &ctm, SPStyle const *style,
                                     Geom::OptRect const &pbox, Geom::OptRect const &dbox,
                                     Geom::OptRect const &bbox);
        virtual unsigned int comment(Inkscape::Extension::Print *module, const char * comment);
        bool textToPath (Inkscape::Extension::Print * ext);

        static void init (void);
};

}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* __INKSCAPE_EXTENSION_INTERNAL_PRINT_LATEX */

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
