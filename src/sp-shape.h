#ifndef SEEN_SP_SHAPE_H
#define SEEN_SP_SHAPE_H

/*
 * Base class for shapes, including <path> element
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2008 Johan Engelen
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/display-forward.h"
#include "sp-lpe-item.h"
#include "sp-marker-loc.h"
#include <2geom/forward.h>

#include <sigc++/connection.h>

#define SP_TYPE_SHAPE (SPShape::getType ())
#define SP_SHAPE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_SHAPE, SPShape))
#define SP_SHAPE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_SHAPE, SPShapeClass))
#define SP_IS_SHAPE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_SHAPE))
#define SP_IS_SHAPE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_SHAPE))

#define SP_SHAPE_WRITE_PATH (1 << 2)

struct SPDesktop;

class SPShape : public SPLPEItem {
public:
    SPCurve *curve;

    SPObject *marker[SP_MARKER_LOC_QTY];
    sigc::connection release_connect [SP_MARKER_LOC_QTY];
    sigc::connection modified_connect [SP_MARKER_LOC_QTY];
	
    static GType getType (void);
    void setShape ();
    SPCurve * getCurve ();
    void setCurve (SPCurve *curve, unsigned int owner);
    void setCurveInsync (SPCurve *curve, unsigned int owner);
    int hasMarkers () const;
    int numberOfMarkers (int type);

private:
    static void sp_shape_init (SPShape *shape);
    static void sp_shape_finalize (GObject *object);

    static void sp_shape_build (SPObject * object, SPDocument * document, Inkscape::XML::Node * repr);
    static void sp_shape_release (SPObject *object);

    static void sp_shape_set(SPObject *object, unsigned key, gchar const *value);
    static void sp_shape_update (SPObject *object, SPCtx *ctx, unsigned int flags);
    static void sp_shape_modified (SPObject *object, unsigned int flags);
    static Inkscape::XML::Node *sp_shape_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

    static void sp_shape_bbox(SPItem const *item, NRRect *bbox, Geom::Matrix const &transform, unsigned const flags);
    static NRArenaItem *sp_shape_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
    static void sp_shape_hide (SPItem *item, unsigned int key);
    static void sp_shape_snappoints (SPItem const *item, std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs);

    static void sp_shape_update_marker_view (SPShape *shape, NRArenaItem *ai);



    friend class SPShapeClass;	
};

class SPShapeClass {
public:
    SPLPEItemClass item_class;

    /* Build bpath from extra shape attributes */
    void (* set_shape) (SPShape *shape);

private:
    static SPLPEItemClass *parent_class;
    static void sp_shape_class_init (SPShapeClass *klass);

    friend class SPShape;
};

void sp_shape_set_marker (SPObject *object, unsigned int key, const gchar *value);

Geom::Matrix sp_shape_marker_get_transform(Geom::Curve const & c1, Geom::Curve const & c2);
Geom::Matrix sp_shape_marker_get_transform_at_start(Geom::Curve const & c);
Geom::Matrix sp_shape_marker_get_transform_at_end(Geom::Curve const & c);

#endif // SEEN_SP_SHAPE_H

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
