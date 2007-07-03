#define __SP_3DBOX_FACE_C__

/*
 * Face of a 3D box ('perspectivic rectangle')
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007  authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "box3d-face.h"
#include <iostream>

Box3DFace::Box3DFace(SP3DBox *box3d) : dir1 (Box3D::NONE), dir2 (Box3D::NONE), path (NULL), parent_box3d (box3d)
{
    for (int i = 0; i < 4; ++i) {
        this->corners[i] = new NR::Point(0, 0);
    }
} 

Box3DFace::Box3DFace(SP3DBox *box3d, NR::Point &A, NR::Point &B, NR::Point &C, NR::Point &D)
{
    set_corners (A, B, C, D);
}

Box3DFace::~Box3DFace()
{
    for (int i = 0; i < 4; ++i) {
        if (this->corners[i]) {
            delete this->corners[i];
            this->corners[i] = NULL;
        }
    }
} 

void Box3DFace::set_corners(NR::Point &A, NR::Point &B, NR::Point &C, NR::Point &D)
{
    corners[0] = &A;
    corners[1] = &B;
    corners[2] = &C;
    corners[3] = &D;
}

/***
void Box3DFace::set_shape(NR::Point const ul, NR::Point const lr,
                     Box3D::Axis const dir1, Box3D::Axis const dir2,
                     unsigned int shift_count, NR::Maybe<NR::Point> pt_align, bool align_along_PL)
{
    corners[0] = ul;
    if (!pt_align) {
        corners[2] = lr;
    } else {
        if (align_along_PL) {
            Box3D::Axis dir3 = Box3D::third_axis_direction (dir1, dir2);
            Box3D::Line line1(*SP3DBoxContext::current_perspective->get_vanishing_point(dir1), lr);
            Box3D::Line line2(*pt_align, *SP3DBoxContext::current_perspective->get_vanishing_point(dir3));
            corners[2] = *line1.intersect(line2);
        } else {
            corners[2] = Box3D::Line(*pt_align, *SP3DBoxContext::current_perspective->get_vanishing_point(dir1)).closest_to(lr);
        }
    }

    Box3D::PerspectiveLine first_line  (corners[0], dir1);
    Box3D::PerspectiveLine second_line (corners[2], dir2);
    NR::Maybe<NR::Point> ur = first_line.intersect(second_line);

    Box3D::PerspectiveLine third_line  (corners[0], dir2);
    Box3D::PerspectiveLine fourth_line (corners[2], dir1);
    NR::Maybe<NR::Point> ll = third_line.intersect(fourth_line);

    // FIXME: How to handle the case if one of the intersections doesn't exist?
    //        Maybe set them equal to the corresponding VPs? 
    if (!ur) ur = NR::Point(0.0, 0.0);    
    if (!ll) ll = NR::Point(0.0, 0.0);

    corners[1] = *ll;
    corners[3] = *ur;

    this->dir1 = dir1;
    this->dir2 = dir2;

    // FIXME: Can be made more concise
    NR::Point tmp_pt;
    for (unsigned int i=0; i < shift_count; i++) {
    	tmp_pt = corners[3];
    	corners[1] = corners[0];
    	corners[2] = corners[1];
    	corners[3] = corners[2];
    	corners[0] = tmp_pt;
    }
}
***/

Box3DFace::Box3DFace(Box3DFace const &box3dface)
{
    for (int i = 0; i < 4; ++i) {
        this->corners[i] = box3dface.corners[i];
    }
    this->dir1 = box3dface.dir1;
    this->dir2 = box3dface.dir2;
}

/**
 * Construct a 3D box face with opposite corners A and C whose sides are directed
 * along axis1 and axis2. The corners have the following order:
 *
 * A = corners[0]  --> along axis1 --> B = corners[1] --> along axis2 --> C = corners[2]
 *                 --> along axis1 --> D = corners[3] --> along axis2 --> D = corners[0].
 * 
 * Note that several other functions rely on this precise order.
 */
void
Box3DFace::set_face (NR::Point const A, NR::Point const C, Box3D::Axis const axis1, Box3D::Axis const axis2)
{
    *corners[0] = A;
    *corners[2] = C;
    if (!SP_IS_3DBOX_CONTEXT(inkscape_active_event_context()))
        return;
    SP3DBoxContext *bc = SP_3DBOX_CONTEXT(inkscape_active_event_context());
    
    Box3D::PerspectiveLine line1 (A, axis1);
    Box3D::PerspectiveLine line2 (C, axis2);
    NR::Maybe<NR::Point> B = line1.intersect(line2);

    Box3D::PerspectiveLine line3 (*corners[0], axis2);
    Box3D::PerspectiveLine line4 (*corners[2], axis1);
    NR::Maybe<NR::Point> D = line3.intersect(line4);

    // FIXME: How to handle the case if one of the intersections doesn't exist?
    //        Maybe set them equal to the corresponding VPs? 
    if (!D) D = NR::Point(0.0, 0.0);    
    if (!B) B = NR::Point(0.0, 0.0);

    *corners[1] = *B;
    *corners[3] = *D;

    this->dir1 = axis1;
    this->dir2 = axis2;
}


NR::Point Box3DFace::operator[](unsigned int i)
{
    return *corners[i % 4];
}

/**
 * Append the curve's path as a child to the given 3D box (since SP3DBox
 * is derived from SPGroup, so we can append children to its svg representation)
 */
void Box3DFace::hook_path_to_3dbox()
{
    if (this->path) return; // This test can probably be removed.

    SPDesktop *desktop = inkscape_active_desktop();
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(SP_EVENT_CONTEXT_DOCUMENT(inkscape_active_event_context()));
    Inkscape::XML::Node *repr_face = xml_doc->createElement("svg:path");
    sp_desktop_apply_style_tool (desktop, repr_face, "tools.shapes.3dbox", false);
    this->path = SP_PATH(SP_OBJECT(parent_box3d)->appendChildRepr(repr_face));
    Inkscape::GC::release(repr_face);
}

/**
 * Write the path's "d" attribute to the SVG representation.
 */
void Box3DFace::set_path_repr()
{
    SP_OBJECT(this->path)->repr->setAttribute("d", svg_repr_string());
}

void Box3DFace::set_curve()
{
    SPDocument *doc = SP_OBJECT_DOCUMENT(this->parent_box3d);
    gdouble height = sp_document_height(doc);

    if (this->path == NULL) {
        g_warning("this->path is NULL! \n");
        return;
    }
    SPCurve *curve = sp_curve_new();
    sp_curve_moveto(curve, (*corners[0])[NR::X], height - (*corners[0])[NR::Y]);
    sp_curve_lineto(curve, (*corners[1])[NR::X], height - (*corners[1])[NR::Y]);
    sp_curve_lineto(curve, (*corners[2])[NR::X], height - (*corners[2])[NR::Y]);
    sp_curve_lineto(curve, (*corners[3])[NR::X], height - (*corners[3])[NR::Y]);
    sp_curve_closepath(curve);
    sp_shape_set_curve(SP_SHAPE(this->path), curve, true);
    sp_curve_unref(curve);
}

gchar * Box3DFace::svg_repr_string()
{
    SPDocument *doc = SP_OBJECT_DOCUMENT(this->parent_box3d);
    gdouble height = sp_document_height(doc);

    GString *pstring = g_string_new("");
    g_string_sprintf (pstring, "M %f,%f L %f,%f L %f,%f L %f,%f z",
                               (*corners[0])[NR::X], height - (*corners[0])[NR::Y],
                               (*corners[1])[NR::X], height - (*corners[1])[NR::Y],
                               (*corners[2])[NR::X], height - (*corners[2])[NR::Y],
                               (*corners[3])[NR::X], height - (*corners[3])[NR::Y]);
    return pstring->str;
}

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
