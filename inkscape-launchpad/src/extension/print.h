/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *   Abhishek Sharma
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_EXTENSION_PRINT_H__
#define INKSCAPE_EXTENSION_PRINT_H__

#include "extension.h"

#include "sp-item.h"

namespace Inkscape {

class Drawing;
class DrawingItem;

namespace Extension {

class Print : public Extension {

public: /* TODO: These are public for the short term, but this should be fixed */
    SPItem *base;
    Inkscape::Drawing *drawing;
    Inkscape::DrawingItem *root;
    unsigned int dkey;

public:
                  Print       (Inkscape::XML::Node * in_repr,
                               Implementation::Implementation * in_imp);
    virtual      ~Print       (void);
    virtual bool  check       (void);

    /* FALSE means user hit cancel */
    unsigned int  setup       (void);
    unsigned int  set_preview (void);

    unsigned int  begin       (SPDocument *doc);
    unsigned int  finish      (void);

    /* Rendering methods */
    unsigned int  bind        (Geom::Affine const &transform,
                               float opacity);
    unsigned int  release     (void);
    unsigned int  comment     (const char * comment);
    unsigned int  fill        (Geom::PathVector const &pathv,
                               Geom::Affine const &ctm,
                               SPStyle const *style,
                               Geom::OptRect const &pbox,
                               Geom::OptRect const &dbox,
                               Geom::OptRect const &bbox);
    unsigned int  stroke      (Geom::PathVector const &pathv,
                               Geom::Affine const &transform,
                               SPStyle const *style,
                               Geom::OptRect const &pbox,
                               Geom::OptRect const &dbox,
                               Geom::OptRect const &bbox);
    unsigned int  image       (unsigned char *px,
                               unsigned int w,
                               unsigned int h,
                               unsigned int rs,
                               Geom::Affine const &transform,
                               SPStyle const *style);
    unsigned int  text        (char const *text,
                               Geom::Point const &p,
                               SPStyle const *style);
    bool          textToPath  (void);
    bool          fontEmbedded  (void);
};

} }  /* namespace Inkscape, Extension */
#endif /* INKSCAPE_EXTENSION_PRINT_H__ */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
