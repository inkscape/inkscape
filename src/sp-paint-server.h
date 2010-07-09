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

#include <libnr/nr-pixblock.h>
#include "sp-object.h"
#include "uri-references.h"

class SPPainter;

#define SP_TYPE_PAINT_SERVER (SPPaintServer::getType())
#define SP_PAINT_SERVER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_PAINT_SERVER, SPPaintServer))
#define SP_PAINT_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_PAINT_SERVER, SPPaintServerClass))
#define SP_IS_PAINT_SERVER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_PAINT_SERVER))
#define SP_IS_PAINT_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_PAINT_SERVER))

typedef enum {
    SP_PAINTER_IND,
    SP_PAINTER_DEP
} SPPainterType;

typedef void (* SPPainterFillFunc) (SPPainter *painter, NRPixBlock *pb);

/* fixme: I do not like that class thingie (Lauris) */
struct SPPainter {
    SPPainter *next;
    SPPaintServer *server;
    GType server_type;
    SPPainterType type;
    SPPainterFillFunc fill;
};

struct SPPaintServer : public SPObject {
    /** List of paints */
    SPPainter *painters;

protected:
    bool swatch;
public:

    static GType getType(void);

    bool isSwatch() const;
    bool isSolid() const;

private:
    static void init(SPPaintServer *ps);
};

struct SPPaintServerClass {
    SPObjectClass sp_object_class;
    /** Get SPPaint instance. */
    SPPainter * (* painter_new) (SPPaintServer *ps, Geom::Matrix const &full_transform, Geom::Matrix const &parent_transform, const NRRect *bbox);
    /** Free SPPaint instance. */
    void (* painter_free) (SPPaintServer *ps, SPPainter *painter);
};

SPPainter *sp_paint_server_painter_new (SPPaintServer *ps, Geom::Matrix const &full_transform, Geom::Matrix const &parent_transform, const NRRect *bbox);

SPPainter *sp_painter_free (SPPainter *painter);

class SPPaintServerReference : public Inkscape::URIReference {
public:
    SPPaintServerReference (SPObject *obj) : URIReference(obj) {}
    SPPaintServerReference (SPDocument *doc) : URIReference(doc) {}
    SPPaintServer *getObject() const {
        return static_cast<SPPaintServer *>(URIReference::getObject());
    }
protected:
    virtual bool _acceptObject(SPObject *obj) const {
        return SP_IS_PAINT_SERVER (obj);
    }
};

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
