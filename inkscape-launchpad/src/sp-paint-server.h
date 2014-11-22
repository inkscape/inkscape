#ifndef SEEN_SP_PAINT_SERVER_H
#define SEEN_SP_PAINT_SERVER_H

/*
 * Base class for gradients and patterns
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2010 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cairo.h>
#include <2geom/rect.h>
#include <sigc++/slot.h>
#include "sp-object.h"

namespace Inkscape {

class Drawing;
class DrawingPattern;

}

#define SP_PAINT_SERVER(obj) (dynamic_cast<SPPaintServer*>((SPObject*)obj))
#define SP_IS_PAINT_SERVER(obj) (dynamic_cast<const SPPaintServer*>((SPObject*)obj) != NULL)

class SPPaintServer : public SPObject {
public:
	SPPaintServer();
	virtual ~SPPaintServer();

    bool isSwatch() const;
    bool isSolid() const;
    virtual bool isValid() const;

    //There are two ways to render a paint. The simple one is to create cairo_pattern_t structure
    //on demand by pattern_new method. It is used for gradients. The other one is to add elements
    //representing PaintServer in NR tree. It is used by hatches and patterns.
    //Either pattern new or all three methods show, hide, setBBox need to be implemented
    virtual Inkscape::DrawingPattern *show(Inkscape::Drawing &drawing, unsigned int key, Geom::OptRect bbox); // TODO check passing bbox by value. Looks suspicious.
    virtual void hide(unsigned int key);
    virtual void setBBox(unsigned int key, Geom::OptRect const &bbox);

    virtual cairo_pattern_t* pattern_new(cairo_t *ct, Geom::OptRect const &bbox, double opacity);

protected:
    bool swatch;
};

/**
 * Returns the first of {src, src-\>ref-\>getObject(),
 * src-\>ref-\>getObject()-\>ref-\>getObject(),...}
 * for which \a match is true, or NULL if none found.
 *
 * The raison d'être of this routine is that it correctly handles cycles in the href chain (e.g., if
 * a gradient gives itself as its href, or if each of two gradients gives the other as its href).
 *
 * \pre SP_IS_GRADIENT(src).
 */
template <class PaintServer>
PaintServer *chase_hrefs(PaintServer *src, sigc::slot<bool, PaintServer const *> match) {
    /* Use a pair of pointers for detecting loops: p1 advances half as fast as p2.  If there is a
       loop, then once p1 has entered the loop, we'll detect it the next time the distance between
       p1 and p2 is a multiple of the loop size. */
    PaintServer *p1 = src, *p2 = src;
    bool do1 = false;
    for (;;) {
        if (match(p2)) {
            return p2;
        }

        p2 = p2->ref->getObject();
        if (!p2) {
            return p2;
        }
        if (do1) {
            p1 = p1->ref->getObject();
        }
        do1 = !do1;

        if ( p2 == p1 ) {
            /* We've been here before, so return NULL to indicate that no matching gradient found
             * in the chain. */
            return NULL;
        }
    }
}

#endif // SEEN_SP_PAINT_SERVER_H
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
