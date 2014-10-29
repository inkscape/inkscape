#ifndef SEEN_BOX3D_SIDE_H
#define SEEN_BOX3D_SIDE_H

/*
 * 3D box face implementation
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2007  Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-polygon.h"
#include "axis-manip.h"


class SPBox3D;
class Persp3D;

// FIXME: Would it be better to inherit from SPPath instead?
class Box3DSide : public SPPolygon {
public:
	Box3DSide();
	virtual ~Box3DSide();

    Box3D::Axis dir1;
    Box3D::Axis dir2;
    Box3D::FrontOrRear front_or_rear;
    int getFaceId();
    static Box3DSide * createBox3DSide(SPBox3D *box);

	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void set(unsigned int key, char const* value);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);
	virtual void update(SPCtx *ctx, unsigned int flags);

	virtual void set_shape();
};

void box3d_side_position_set (Box3DSide *side); // FIXME: Replace this by box3d_side_set_shape??

char *box3d_side_axes_string(Box3DSide *side);

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
