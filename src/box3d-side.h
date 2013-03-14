#ifndef SEEN_BOX3D_SIDE_H
#define SEEN_BOX3D_SIDE_H

/*
 * 3D box face implementation
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *   Abhishek Sharma
 *
 * Copyright (C) 2007  Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-polygon.h"
#include "axis-manip.h"

#define SP_TYPE_BOX3D_SIDE            (box3d_side_get_type ())
#define SP_BOX3D_SIDE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_BOX3D_SIDE, Box3DSide))
#define SP_BOX3D_SIDE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_BOX3D_SIDE, Box3DSideClass))
#define SP_IS_BOX3D_SIDE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_BOX3D_SIDE))
#define SP_IS_BOX3D_SIDE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_BOX3D_SIDE))

class SPBox3D;
struct Persp3D;

// FIXME: Would it be better to inherit from SPPath instead?
struct Box3DSide : public SPPolygon {
    Box3D::Axis dir1;
    Box3D::Axis dir2;
    Box3D::FrontOrRear front_or_rear;
    int getFaceId();
    static Box3DSide * createBox3DSide(SPBox3D *box);
};

struct Box3DSideClass {
    SPPolygonClass parent_class;
};

GType box3d_side_get_type (void);

void box3d_side_position_set (Box3DSide *side); // FIXME: Replace this by box3d_side_set_shape??

gchar *box3d_side_axes_string(Box3DSide *side);

Persp3D *box3d_side_perspective(Box3DSide *side);


Inkscape::XML::Node *box3d_side_convert_to_path(Box3DSide *side);

#endif // SEEN_BOX3D_SIDE_H

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
