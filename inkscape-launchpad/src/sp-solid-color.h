#ifndef SEEN_SP_SOLIDCOLOR_H
#define SEEN_SP_SOLIDCOLOR_H

/** \file
 * SPSolidColor: SVG <solidColor> implementation.
 */
/*
 * Authors: Tavmjong Bah
 * Copyright (C) 2012 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include "color.h"
#include "sp-paint-server.h"

#define SP_SOLIDCOLOR(obj) (dynamic_cast<SPSolidColor*>((SPObject*)obj))
#define SP_IS_SOLIDCOLOR(obj) (dynamic_cast<const SPSolidColor*>((SPObject*)obj) != NULL)

/** Gradient SolidColor. */
class SPSolidColor : public SPPaintServer {
public:
    SPSolidColor();
    virtual ~SPSolidColor();

    virtual cairo_pattern_t* pattern_new(cairo_t *ct, Geom::OptRect const &bbox, double opacity);

protected:
    virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
    virtual void set(unsigned int key, const gchar* value);
    virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, guint flags);
};

#endif /* !SEEN_SP_SOLIDCOLOR_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
