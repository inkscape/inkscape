/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_EXTENSION_PRINT_H__
#define INKSCAPE_EXTENSION_PRINT_H__

#include "extension.h"

#include "libnr/nr-path.h"
#include "display/nr-arena-forward.h"
#include "forward.h"

namespace Inkscape {
namespace Extension {

class Print : public Extension {

public: /* TODO: These are public for the short term, but this should be fixed */
    SPItem *base;            /**< TODO: Document these */
    NRArena *arena;          /**< TODO: Document these */
    NRArenaItem *root;       /**< TODO: Document these */
    unsigned int dkey;       /**< TODO: Document these */

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
    unsigned int  bind        (Geom::Matrix const *transform,
                               float opacity);
    unsigned int  release     (void);
    unsigned int  comment     (const char * comment);
    unsigned int  fill        (Geom::PathVector const &pathv,
                               Geom::Matrix const *ctm,
                               SPStyle const *style,
                               NRRect const *pbox,
                               NRRect const *dbox,
                               NRRect const *bbox);
    unsigned int  stroke      (Geom::PathVector const &pathv,
                               Geom::Matrix const *transform,
                               SPStyle const *style,
                               NRRect const *pbox,
                               NRRect const *dbox,
                               NRRect const *bbox);
    unsigned int  image       (unsigned char *px,
                               unsigned int w,
                               unsigned int h,
                               unsigned int rs,
                               Geom::Matrix const *transform,
                               SPStyle const *style);
    unsigned int  text        (char const *text,
                               Geom::Point p,
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
