#ifndef SEEN_PERSP3D_H
#define SEEN_PERSP3D_H

/*
 * Implementation of 3D perspectives as SPObjects
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007  Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define SP_PERSP3D(obj) (dynamic_cast<Persp3D*>((SPObject*)obj))
#define SP_IS_PERSP3D(obj) (dynamic_cast<const Persp3D*>((SPObject*)obj) != NULL)

#include <list>
#include <map>
#include <vector>

#include "transf_mat_3x4.h"
#include "document.h"
#include "inkscape.h" // for SP_ACTIVE_DOCUMENT
#include "sp-object.h"

class SPBox3D;

namespace Inkscape {
namespace UI {
namespace Tools {

class Box3dTool;

}
}
}


class Persp3DImpl {
public:
    Persp3DImpl();

//private:
    Proj::TransfMat3x4 tmat;

    // Also write the list of boxes into the xml repr and vice versa link boxes to their persp3d?
    std::vector<SPBox3D *> boxes;
    SPDocument *document;

    // for debugging only
    int my_counter;

//    friend class Persp3D;
};

class Persp3D : public SPObject {
public:
	Persp3D();
	virtual ~Persp3D();

    Persp3DImpl *perspective_impl;

protected:
	virtual void build(SPDocument* doc, Inkscape::XML::Node* repr);
	virtual void release();

	virtual void set(unsigned int key, char const* value);

	virtual void update(SPCtx* ctx, unsigned int flags);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, unsigned int flags);
};


// FIXME: Make more of these inline!
inline Persp3D * persp3d_get_from_repr (Inkscape::XML::Node *repr) {
    return SP_PERSP3D(SP_ACTIVE_DOCUMENT->getObjectByRepr(repr));
}
inline Proj::Pt2 persp3d_get_VP (Persp3D *persp, Proj::Axis axis) {
    return persp->perspective_impl->tmat.column(axis);
}
Geom::Point persp3d_get_PL_dir_from_pt (Persp3D *persp, Geom::Point const &pt, Proj::Axis axis); // convenience wrapper around the following two
Geom::Point persp3d_get_finite_dir (Persp3D *persp, Geom::Point const &pt, Proj::Axis axis);
Geom::Point persp3d_get_infinite_dir (Persp3D *persp, Proj::Axis axis);
double persp3d_get_infinite_angle (Persp3D *persp, Proj::Axis axis);
bool persp3d_VP_is_finite (Persp3DImpl *persp_impl, Proj::Axis axis);
void persp3d_toggle_VP (Persp3D *persp, Proj::Axis axis, bool set_undo = true);
void persp3d_toggle_VPs (std::list<Persp3D *>, Proj::Axis axis);
void persp3d_set_VP_state (Persp3D *persp, Proj::Axis axis, Proj::VPState state);
void persp3d_rotate_VP (Persp3D *persp, Proj::Axis axis, double angle, bool alt_pressed); // angle is in degrees
void persp3d_apply_affine_transformation (Persp3D *persp, Geom::Affine const &xform);

void persp3d_add_box (Persp3D *persp, SPBox3D *box);
void persp3d_remove_box (Persp3D *persp, SPBox3D *box);
bool persp3d_has_box (Persp3D *persp, SPBox3D *box);

void persp3d_update_box_displays (Persp3D *persp);
void persp3d_update_box_reprs (Persp3D *persp);
void persp3d_update_z_orders (Persp3D *persp);
inline unsigned int persp3d_num_boxes (Persp3D *persp) { return persp->perspective_impl->boxes.size(); }
std::list<SPBox3D *> persp3d_list_of_boxes(Persp3D *persp);

bool persp3d_perspectives_coincide(Persp3D const *lhs, Persp3D const *rhs);
void persp3d_absorb(Persp3D *persp1, Persp3D *persp2);

Persp3D * persp3d_create_xml_element (SPDocument *document, Persp3DImpl *dup = NULL);
Persp3D * persp3d_document_first_persp (SPDocument *document);

bool persp3d_has_all_boxes_in_selection (Persp3D *persp, Inkscape::Selection *selection);

void persp3d_print_debugging_info (Persp3D *persp);
void persp3d_print_debugging_info_all(SPDocument *doc);
void persp3d_print_all_selected();

void print_current_persp3d(char *func_name, Persp3D *persp);

#endif /* __PERSP3D_H__ */

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
