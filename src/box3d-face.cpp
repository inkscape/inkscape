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

Box3DFace::Box3DFace(SP3DBox *box3d) : corner1 (0, 0), corner2 (0, 0), corner3 (0, 0), corner4 (0, 0),
                                       dir1 (Box3D::NONE), dir2 (Box3D::NONE), path (NULL), parent_box3d (box3d)
{
} 

void Box3DFace::set_shape(NR::Point const ul, NR::Point const lr,
                     Box3D::PerspDir const dir1, Box3D::PerspDir const dir2,
                     unsigned int shift_count, NR::Maybe<NR::Point> pt_align, bool align_along_PL)
{
    corner1 = ul;
    if (!pt_align) {
        corner3 = lr;
    } else {
        if (align_along_PL) {
        	Box3D::PerspDir dir3;
            if (dir1 == Box3D::X && dir2 == Box3D::Y) dir3 = Box3D::Z;
            if (dir1 == Box3D::X && dir2 == Box3D::Z) dir3 = Box3D::Y;
            if (dir1 == Box3D::Y && dir2 == Box3D::X) dir3 = Box3D::Z;
            if (dir1 == Box3D::Y && dir2 == Box3D::Z) dir3 = Box3D::X;
            if (dir1 == Box3D::Z && dir2 == Box3D::X) dir3 = Box3D::Y;
            if (dir1 == Box3D::Z && dir2 == Box3D::Y) dir3 = Box3D::X;
            Box3D::Line line1(*SP3DBoxContext::current_perspective->get_vanishing_point(dir1), lr);
            Box3D::Line line2(*pt_align, *SP3DBoxContext::current_perspective->get_vanishing_point(dir3));
            corner3 = *line1.intersect(line2);
        } else {
            corner3 = Box3D::Line(*pt_align, *SP3DBoxContext::current_perspective->get_vanishing_point(dir1)).closest_to(lr);
        }
    }

    Box3D::PerspectiveLine first_line  (corner1, dir1);
    Box3D::PerspectiveLine second_line (corner3, dir2);
    NR::Maybe<NR::Point> ur = first_line.intersect(second_line);

    Box3D::PerspectiveLine third_line  (corner1, dir2);
    Box3D::PerspectiveLine fourth_line (corner3, dir1);
    NR::Maybe<NR::Point> ll = third_line.intersect(fourth_line);

    // FIXME: How to handle the case if one of the intersections doesn't exist?
    //        Maybe set them equal to the corresponding VPs? 
    if (!ur) ur = NR::Point(0.0, 0.0);    
    if (!ll) ll = NR::Point(0.0, 0.0);

    corner2 = *ll;
    corner4 = *ur;

    this->dir1 = dir1;
    this->dir2 = dir2;

    // FIXME: More effective with array of corners
    NR::Point tmp_pt;
    for (unsigned int i=0; i < shift_count; i++) {
    	tmp_pt = corner4;
    	corner2 = corner1;
    	corner3 = corner2;
    	corner4 = corner3;
    	corner1 = tmp_pt;
    }
}

Box3DFace::Box3DFace(Box3DFace const &box3dface)
{
	this->corner1 = box3dface.corner1;
	this->corner2 = box3dface.corner2;
	this->corner3 = box3dface.corner3;
	this->corner4 = box3dface.corner4;
	this->dir1 = box3dface.dir1;
	this->dir2 = box3dface.dir2;
}

NR::Point Box3DFace::operator[](unsigned int i)
{
	unsigned int index = i % 4;
    switch (index) {
    	case 0: return corner1; break;
    	case 1: return corner2; break;
    	case 2: return corner3; break;
    	case 3: return corner4; break;
    }
    // The following two lines are just to prevent a compiler warning ("control reaches
    // end of non-void function); they can be removed if desired
    g_message ("Error: This code line hould not be reached\n");
    return NR::Point (0, 0);
}

/**
 * Append the curve's path as a child to the given 3D box (since SP3DBox
 * is derived from SPGroup, so we can append children to its svg representation)
 */
void Box3DFace::hook_path_to_3dbox()
{
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

    SPCurve *curve = sp_curve_new();
    sp_curve_moveto(curve, corner1[NR::X], height - corner1[NR::Y]);
    sp_curve_lineto(curve, corner2[NR::X], height - corner2[NR::Y]);
    sp_curve_lineto(curve, corner3[NR::X], height - corner3[NR::Y]);
    sp_curve_lineto(curve, corner4[NR::X], height - corner4[NR::Y]);
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
                               corner1[NR::X], height - corner1[NR::Y],
                               corner2[NR::X], height - corner2[NR::Y],
                               corner3[NR::X], height - corner3[NR::Y],
                               corner4[NR::X], height - corner4[NR::Y]);
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
