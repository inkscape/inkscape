#ifndef __PERSP3D_H__
#define __PERSP3D_H__

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

#define SP_TYPE_PERSP3D         (persp3d_get_type ())
#define SP_PERSP3D(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_PERSP3D, Persp3D))
#define SP_PERSP3D_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SP_TYPE_PERSP3D, Persp3DClass))
#define SP_IS_PERSP3D(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_PERSP3D))
#define SP_IS_PERSP3D_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_PERSP3D))

#include <list>
#include <vector>
#include <map>
#include "sp-item.h"
#include "transf_mat_3x4.h"
#include "document.h"
#include "inkscape.h"

class SPBox3D;
class Box3DContext;

struct Persp3D : public SPObject {
    Proj::TransfMat3x4 tmat;

    // Also write the list of boxes into the xml repr and vice versa link boxes to their persp3d?
    std::vector<SPBox3D *> boxes;
    std::map<SPBox3D *, bool> boxes_transformed; // TODO: eventually we should merge this with 'boxes'
    SPDocument *document; // should this rather be the SPDesktop?

    // for debugging only
    int my_counter;
};

struct Persp3DClass {
    SPItemClass parent_class;
};


/* Standard GType function */
GType persp3d_get_type (void);

// FIXME: Make more of these inline!
inline Persp3D * persp3d_get_from_repr (Inkscape::XML::Node *repr) {
    return SP_PERSP3D(SP_ACTIVE_DOCUMENT->getObjectByRepr(repr));
}
inline Proj::Pt2 persp3d_get_VP (Persp3D *persp, Proj::Axis axis) {
    return persp->tmat.column(axis);
}
NR::Point persp3d_get_PL_dir_from_pt (Persp3D *persp, NR::Point const &pt, Proj::Axis axis); // convenience wrapper around the following two
NR::Point persp3d_get_finite_dir (Persp3D *persp, NR::Point const &pt, Proj::Axis axis);
NR::Point persp3d_get_infinite_dir (Persp3D *persp, Proj::Axis axis);
double persp3d_get_infinite_angle (Persp3D *persp, Proj::Axis axis);
bool persp3d_VP_is_finite (Persp3D *persp, Proj::Axis axis);
void persp3d_toggle_VP (Persp3D *persp, Proj::Axis axis, bool set_undo = true);
void persp3d_toggle_VPs (std::list<Persp3D *>, Proj::Axis axis);
void persp3d_set_VP_state (Persp3D *persp, Proj::Axis axis, Proj::VPState state);
void persp3d_rotate_VP (Persp3D *persp, Proj::Axis axis, double angle, bool alt_pressed); // angle is in degrees
void persp3d_update_with_point (Persp3D *persp, Proj::Axis const axis, Proj::Pt2 const &new_image);
void persp3d_apply_affine_transformation (Persp3D *persp, NR::Matrix const &xform);
gchar * persp3d_pt_to_str (Persp3D *persp, Proj::Axis const axis);

void persp3d_add_box (Persp3D *persp, SPBox3D *box);
void persp3d_remove_box (Persp3D *persp, SPBox3D *box);
bool persp3d_has_box (Persp3D *persp, SPBox3D *box);

void persp3d_add_box_transform (Persp3D *persp, SPBox3D *box);
void persp3d_remove_box_transform (Persp3D *persp, SPBox3D *box);
void persp3d_set_box_transformed (Persp3D *persp, SPBox3D *box, bool transformed = true);
bool persp3d_was_transformed (Persp3D *persp);
bool persp3d_all_transformed(Persp3D *persp);
void persp3d_unset_transforms(Persp3D *persp);

void persp3d_update_box_displays (Persp3D *persp);
void persp3d_update_box_reprs (Persp3D *persp);
void persp3d_update_z_orders (Persp3D *persp);
inline unsigned int persp3d_num_boxes (Persp3D *persp) { return persp->boxes.size(); }
std::list<SPBox3D *> persp3d_list_of_boxes(Persp3D *persp);

bool persp3d_perspectives_coincide(const Persp3D *lhs, const Persp3D *rhs);
void persp3d_absorb(Persp3D *persp1, Persp3D *persp2);

Persp3D * persp3d_create_xml_element (SPDocument *document, Persp3D *dup = NULL);
Persp3D * persp3d_document_first_persp (SPDocument *document);

bool persp3d_has_all_boxes_in_selection (Persp3D *persp);
std::map<Persp3D *, std::list<SPBox3D *> > persp3d_unselected_boxes(Inkscape::Selection *selection);
void persp3d_split_perspectives_according_to_selection(Inkscape::Selection *selection);

void persp3d_print_debugging_info (Persp3D *persp);
void persp3d_print_debugging_info_all(SPDocument *doc);
void persp3d_print_all_selected();

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
