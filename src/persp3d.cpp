/*
 * Class modelling a 3D perspective as an SPObject
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "persp3d.h"
#include "perspective-line.h"
#include "attributes.h"
#include "document-private.h"
#include "document-undo.h"
#include "vanishing-point.h"
#include "ui/tools/box3d-tool.h"
#include "box3d.h"
#include "svg/stringstream.h"
#include "xml/document.h"
#include "xml/node-event-vector.h"
#include "desktop.h"

#include <glibmm/i18n.h>
#include "verbs.h"
#include "util/units.h"

using Inkscape::DocumentUndo;

static void persp3d_on_repr_attr_changed (Inkscape::XML::Node * repr, const gchar *key, const gchar *oldval, const gchar *newval, bool is_interactive, void * data);

static int global_counter = 0;

/* Constructor/destructor for the internal class */

Persp3DImpl::Persp3DImpl() :
    tmat (Proj::TransfMat3x4 ()),
    document (NULL)
{
    my_counter = global_counter++;
}

static Inkscape::XML::NodeEventVector const persp3d_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    persp3d_on_repr_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};


Persp3D::Persp3D() : SPObject() {
    this->perspective_impl = new Persp3DImpl();
}

Persp3D::~Persp3D() {
}


/**
 * Virtual build: set persp3d attributes from its associated XML node.
 */
void Persp3D::build(SPDocument *document, Inkscape::XML::Node *repr) {
	SPObject::build(document, repr);

    this->readAttr( "inkscape:vp_x" );
    this->readAttr( "inkscape:vp_y" );
    this->readAttr( "inkscape:vp_z" );
    this->readAttr( "inkscape:persp3d-origin" );

    if (repr) {
        repr->addListener (&persp3d_repr_events, this);
    }
}

/**
 * Virtual release of Persp3D members before destruction.
 */
void Persp3D::release() {
    delete this->perspective_impl;
    this->getRepr()->removeListenerByData(this);
}


/**
 * Virtual set: set attribute to value.
 */
// FIXME: Currently we only read the finite positions of vanishing points;
//        should we move VPs into their own repr (as it's done for SPStop, e.g.)?
void Persp3D::set(unsigned key, gchar const *value) {

    // Read values are in 'user units'.
    double scale_x = 1.0;
    double scale_y = 1.0;
    SPRoot *root = document->getRoot();
    if( root->viewBox_set ) {
        scale_x = root->width.computed / root->viewBox.width();
        scale_y = root->height.computed / root->viewBox.height();
    }

    switch (key) {
        case SP_ATTR_INKSCAPE_PERSP3D_VP_X: {
            if (value) {
                Proj::Pt2 pt (value);
                Proj::Pt2 ptn ( pt[0]*scale_x, pt[1]*scale_y, pt[2] );
                perspective_impl->tmat.set_image_pt( Proj::X, ptn );
            }
            break;
        }
        case SP_ATTR_INKSCAPE_PERSP3D_VP_Y: {
            if (value) {
                Proj::Pt2 pt (value);
                Proj::Pt2 ptn ( pt[0]*scale_x, pt[1]*scale_y, pt[2] );
                perspective_impl->tmat.set_image_pt( Proj::Y, ptn );
            }
            break;
        }
        case SP_ATTR_INKSCAPE_PERSP3D_VP_Z: {
            if (value) {
                Proj::Pt2 pt (value);
                Proj::Pt2 ptn ( pt[0]*scale_x, pt[1]*scale_y, pt[2] );
                perspective_impl->tmat.set_image_pt( Proj::Z, ptn );
            }
            break;
        }
        case SP_ATTR_INKSCAPE_PERSP3D_ORIGIN: {
            if (value) {
                Proj::Pt2 pt (value);
                Proj::Pt2 ptn ( pt[0]*scale_x, pt[1]*scale_y, pt[2] );
                perspective_impl->tmat.set_image_pt( Proj::W, ptn );
            }
            break;
        }
        default: {
        	SPObject::set(key, value);
            break;
        }
    }

    // FIXME: Is this the right place for resetting the draggers?
    Inkscape::UI::Tools::ToolBase *ec = INKSCAPE.active_event_context();
    if (SP_IS_BOX3D_CONTEXT(ec)) {
        Inkscape::UI::Tools::Box3dTool *bc = SP_BOX3D_CONTEXT(ec);
        bc->_vpdrag->updateDraggers();
        bc->_vpdrag->updateLines();
        bc->_vpdrag->updateBoxHandles();
        bc->_vpdrag->updateBoxReprs();
    }
}

void Persp3D::update(SPCtx *ctx, guint flags) {
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* TODO: Should we update anything here? */

    }

    SPObject::update(ctx, flags);
}

Persp3D *persp3d_create_xml_element(SPDocument *document, Persp3DImpl *dup) {// if dup is given, copy the attributes over
    SPDefs *defs = document->getDefs();
    Inkscape::XML::Document *xml_doc = document->getReprDoc();
    Inkscape::XML::Node *repr;

    /* if no perspective is given, create a default one */
    repr = xml_doc->createElement("inkscape:perspective");
    repr->setAttribute("sodipodi:type", "inkscape:persp3d");

    // Use 'user-units'
    double width = document->getWidth().value("px");
    double height = document->getHeight().value("px");
    if( document->getRoot()->viewBox_set ) {
        Geom::Rect vb = document->getRoot()->viewBox;
        width = vb.width();
        height = vb.height();
    }

    Proj::Pt2 proj_vp_x = Proj::Pt2 (0.0,   height/2.0, 1.0);
    Proj::Pt2 proj_vp_y = Proj::Pt2 (0.0,   1000.0,     0.0);
    Proj::Pt2 proj_vp_z = Proj::Pt2 (width, height/2.0, 1.0);
    Proj::Pt2 proj_origin = Proj::Pt2 (width/2.0, height/3.0, 1.0 );

    if (dup) {
        proj_vp_x = dup->tmat.column (Proj::X);
        proj_vp_y = dup->tmat.column (Proj::Y);
        proj_vp_z = dup->tmat.column (Proj::Z);
        proj_origin = dup->tmat.column (Proj::W);
    }

    gchar *str = NULL;
    str = proj_vp_x.coord_string();
    repr->setAttribute("inkscape:vp_x", str);
    g_free (str);
    str = proj_vp_y.coord_string();
    repr->setAttribute("inkscape:vp_y", str);
    g_free (str);
    str = proj_vp_z.coord_string();
    repr->setAttribute("inkscape:vp_z", str);
    g_free (str);
    str = proj_origin.coord_string();
    repr->setAttribute("inkscape:persp3d-origin", str);
    g_free (str);

    /* Append the new persp3d to defs */
    defs->getRepr()->addChild(repr, NULL);
    Inkscape::GC::release(repr);

    return reinterpret_cast<Persp3D *>( defs->get_child_by_repr(repr) );
}

Persp3D *persp3d_document_first_persp(SPDocument *document)
{
    Persp3D *first = 0;
    for ( SPObject *child = document->getDefs()->firstChild(); child && !first; child = child->getNext() ) {
        if (SP_IS_PERSP3D(child)) {
            first = SP_PERSP3D(child);
        }
    }
    return first;
}

/**
 * Virtual write: write object attributes to repr.
 */
Inkscape::XML::Node* Persp3D::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {

    if ((flags & SP_OBJECT_WRITE_BUILD & SP_OBJECT_WRITE_EXT) && !repr) {
        // this is where we end up when saving as plain SVG (also in other circumstances?);
        // hence we don't set the sodipodi:type attribute
        repr = xml_doc->createElement("inkscape:perspective");
    }

    if (flags & SP_OBJECT_WRITE_EXT) {

        // Written values are in 'user units'.
        double scale_x = 1.0;
        double scale_y = 1.0;
        SPRoot *root = document->getRoot();
        if( root->viewBox_set ) {
            scale_x = root->viewBox.width() / root->width.computed;
            scale_y = root->viewBox.height() / root->height.computed;
        }
        {
            Proj::Pt2 pt = perspective_impl->tmat.column( Proj::X );
            Inkscape::SVGOStringStream os;
            os << pt[0] * scale_x  << " : " << pt[1] * scale_y << " : " << pt[2];
            repr->setAttribute("inkscape:vp_x", os.str().c_str());
        }
        {
            Proj::Pt2 pt = perspective_impl->tmat.column( Proj::Y );
            Inkscape::SVGOStringStream os;
            os << pt[0] * scale_x  << " : " << pt[1] * scale_y << " : " << pt[2];
            repr->setAttribute("inkscape:vp_y", os.str().c_str());
        }
        {
            Proj::Pt2 pt = perspective_impl->tmat.column( Proj::Z );
            Inkscape::SVGOStringStream os;
            os << pt[0] * scale_x  << " : " << pt[1] * scale_y << " : " << pt[2];
            repr->setAttribute("inkscape:vp_z", os.str().c_str());
        }
        {
            Proj::Pt2 pt = perspective_impl->tmat.column( Proj::W );
            Inkscape::SVGOStringStream os;
            os << pt[0] * scale_x  << " : " << pt[1] * scale_y << " : " << pt[2];
            repr->setAttribute("inkscape:persp3d-origin", os.str().c_str());
        }
    }

    SPObject::write(xml_doc, repr, flags);

    return repr;
}

/* convenience wrapper around persp3d_get_finite_dir() and persp3d_get_infinite_dir() */
Geom::Point persp3d_get_PL_dir_from_pt (Persp3D *persp, Geom::Point const &pt, Proj::Axis axis) {
    if (persp3d_VP_is_finite(persp->perspective_impl, axis)) {
        return persp3d_get_finite_dir(persp, pt, axis);
    } else {
        return persp3d_get_infinite_dir(persp, axis);
    }
}

Geom::Point
persp3d_get_finite_dir (Persp3D *persp, Geom::Point const &pt, Proj::Axis axis) {
    Box3D::PerspectiveLine pl(pt, axis, persp);
    return pl.direction();
}

Geom::Point
persp3d_get_infinite_dir (Persp3D *persp, Proj::Axis axis) {
    Proj::Pt2 vp(persp3d_get_VP(persp, axis));
    if (vp[2] != 0.0) {
        g_print ("VP should be infinite but is (%f : %f : %f)\n", vp[0], vp[1], vp[2]);
        g_return_val_if_fail(vp[2] != 0.0, Geom::Point(0.0, 0.0));
    }
    return Geom::Point(vp[0], vp[1]);
}

double
persp3d_get_infinite_angle (Persp3D *persp, Proj::Axis axis) {
    return persp->perspective_impl->tmat.get_infinite_angle(axis);
}

bool
persp3d_VP_is_finite (Persp3DImpl *persp_impl, Proj::Axis axis) {
    return persp_impl->tmat.has_finite_image(axis);
}

void
persp3d_toggle_VP (Persp3D *persp, Proj::Axis axis, bool set_undo) {
    persp->perspective_impl->tmat.toggle_finite(axis);
    // FIXME: Remove this repr update and rely on vp_drag_sel_modified() to do this for us
    //        On the other hand, vp_drag_sel_modified() would update all boxes;
    //        here we can confine ourselves to the boxes of this particular perspective.
    persp3d_update_box_reprs (persp);
    persp->updateRepr(SP_OBJECT_WRITE_EXT);
    if (set_undo) {
        DocumentUndo::done(SP_ACTIVE_DESKTOP->getDocument(), SP_VERB_CONTEXT_3DBOX,
                           _("Toggle vanishing point"));
    }
}

/* toggle VPs for the same axis in all perspectives of a given list */
void
persp3d_toggle_VPs (std::list<Persp3D *> p, Proj::Axis axis) {
    for (std::list<Persp3D *>::iterator i = p.begin(); i != p.end(); ++i) {
        persp3d_toggle_VP((*i), axis, false);
    }
    DocumentUndo::done(SP_ACTIVE_DESKTOP->getDocument(), SP_VERB_CONTEXT_3DBOX,
                       _("Toggle multiple vanishing points"));
}

void
persp3d_set_VP_state (Persp3D *persp, Proj::Axis axis, Proj::VPState state) {
    if (persp3d_VP_is_finite(persp->perspective_impl, axis) != (state == Proj::VP_FINITE)) {
        persp3d_toggle_VP(persp, axis);
    }
}

void
persp3d_rotate_VP (Persp3D *persp, Proj::Axis axis, double angle, bool alt_pressed) { // angle is in degrees
    // FIXME: Most of this functionality should be moved to trans_mat_3x4.(h|cpp)
    if (persp->perspective_impl->tmat.has_finite_image(axis)) {
        // don't rotate anything for finite VPs
        return;
    }
    Proj::Pt2 v_dir_proj (persp->perspective_impl->tmat.column(axis));
    Geom::Point v_dir (v_dir_proj[0], v_dir_proj[1]);
    double a = Geom::atan2 (v_dir) * 180/M_PI;
    a += alt_pressed ? 0.5 * ((angle > 0 ) - (angle < 0)) : angle; // the r.h.s. yields +/-0.5 or angle
    persp->perspective_impl->tmat.set_infinite_direction (axis, a);

    persp3d_update_box_reprs (persp);
    persp->updateRepr(SP_OBJECT_WRITE_EXT);
}

void
persp3d_apply_affine_transformation (Persp3D *persp, Geom::Affine const &xform) {
    persp->perspective_impl->tmat *= xform;
    persp3d_update_box_reprs(persp);
    persp->updateRepr(SP_OBJECT_WRITE_EXT);
}

void
persp3d_add_box (Persp3D *persp, SPBox3D *box) {
    Persp3DImpl *persp_impl = persp->perspective_impl;

    if (!box) {
        return;
    }
    if (std::find (persp_impl->boxes.begin(), persp_impl->boxes.end(), box) != persp_impl->boxes.end()) {
        return;
    }
    persp_impl->boxes.push_back(box);
}

void
persp3d_remove_box (Persp3D *persp, SPBox3D *box) {
    Persp3DImpl *persp_impl = persp->perspective_impl;

    std::vector<SPBox3D *>::iterator i = std::find (persp_impl->boxes.begin(), persp_impl->boxes.end(), box);
    if (i != persp_impl->boxes.end())
        persp_impl->boxes.erase(i);
}

bool
persp3d_has_box (Persp3D *persp, SPBox3D *box) {
    Persp3DImpl *persp_impl = persp->perspective_impl;

    // FIXME: For some reason, std::find() does not seem to compare pointers "correctly" (or do we need to
    //        provide a proper comparison function?), so we manually traverse the list.
    for (std::vector<SPBox3D *>::iterator i = persp_impl->boxes.begin(); i != persp_impl->boxes.end(); ++i) {
        if ((*i) == box) {
            return true;
        }
    }
    return false;
}

void
persp3d_update_box_displays (Persp3D *persp) {
    Persp3DImpl *persp_impl = persp->perspective_impl;

    if (persp_impl->boxes.empty())
        return;
    for (std::vector<SPBox3D *>::iterator i = persp_impl->boxes.begin(); i != persp_impl->boxes.end(); ++i) {
        box3d_position_set(*i);
    }
}

void
persp3d_update_box_reprs (Persp3D *persp) {
    if (!persp) {
        // Hmm, is it an error if this happens?
        return;
    }
    Persp3DImpl *persp_impl = persp->perspective_impl;

    if (persp_impl->boxes.empty())
        return;
    for (std::vector<SPBox3D *>::iterator i = persp_impl->boxes.begin(); i != persp_impl->boxes.end(); ++i) {
        (*i)->updateRepr(SP_OBJECT_WRITE_EXT);
        box3d_set_z_orders(*i);
    }
}

void
persp3d_update_z_orders (Persp3D *persp) {
    Persp3DImpl *persp_impl = persp->perspective_impl;

    if (persp_impl->boxes.empty())
        return;
    for (std::vector<SPBox3D *>::iterator i = persp_impl->boxes.begin(); i != persp_impl->boxes.end(); ++i) {
        box3d_set_z_orders(*i);
    }
}

// FIXME: For some reason we seem to require a vector instead of a list in Persp3D, but in vp_knot_moved_handler()
//        we need a list of boxes. If we can store a list in Persp3D right from the start, this function becomes
//        obsolete. We should do this.
std::list<SPBox3D *>
persp3d_list_of_boxes(Persp3D *persp) {
    Persp3DImpl *persp_impl = persp->perspective_impl;

    std::list<SPBox3D *> bx_lst;
    for (std::vector<SPBox3D *>::iterator i = persp_impl->boxes.begin(); i != persp_impl->boxes.end(); ++i) {
        bx_lst.push_back(*i);
    }
    return bx_lst;
}

bool
persp3d_perspectives_coincide(const Persp3D *lhs, const Persp3D *rhs)
{
    return lhs->perspective_impl->tmat == rhs->perspective_impl->tmat;
}

void
persp3d_absorb(Persp3D *persp1, Persp3D *persp2) {
    /* double check if we are called in sane situations */
    g_return_if_fail (persp3d_perspectives_coincide(persp1, persp2) && persp1 != persp2);

    std::vector<SPBox3D *>::iterator boxes;

    // Note: We first need to copy the boxes of persp2 into a separate list;
    //       otherwise the loop below gets confused when perspectives are reattached.
    std::list<SPBox3D *> boxes_of_persp2 = persp3d_list_of_boxes(persp2);

    for (std::list<SPBox3D *>::iterator i = boxes_of_persp2.begin(); i != boxes_of_persp2.end(); ++i) {
        box3d_switch_perspectives((*i), persp2, persp1, true);
        (*i)->updateRepr(SP_OBJECT_WRITE_EXT); // so that undo/redo can do its job properly
    }
}

static void
persp3d_on_repr_attr_changed ( Inkscape::XML::Node * /*repr*/,
                               const gchar */*key*/,
                               const gchar */*oldval*/,
                               const gchar */*newval*/,
                               bool /*is_interactive*/,
                               void * data )
{
    if (!data)
        return;

    Persp3D *persp = (Persp3D*) data;
    persp3d_update_box_displays (persp);
}

/* checks whether all boxes linked to this perspective are currently selected */
bool
persp3d_has_all_boxes_in_selection (Persp3D *persp, Inkscape::Selection *selection) {
    Persp3DImpl *persp_impl = persp->perspective_impl;

    std::list<SPBox3D *> selboxes = selection->box3DList();

    for (std::vector<SPBox3D *>::iterator i = persp_impl->boxes.begin(); i != persp_impl->boxes.end(); ++i) {
        if (std::find(selboxes.begin(), selboxes.end(), *i) == selboxes.end()) {
            // we have an unselected box in the perspective
            return false;
        }
    }
    return true;
}

/* some debugging stuff follows */

void
persp3d_print_debugging_info (Persp3D *persp) {
    Persp3DImpl *persp_impl = persp->perspective_impl;
    g_print ("=== Info for Persp3D %d ===\n", persp_impl->my_counter);
    gchar * cstr;
    for (int i = 0; i < 4; ++i) {
        cstr = persp3d_get_VP(persp, Proj::axes[i]).coord_string();
        g_print ("  VP %s:   %s\n", Proj::string_from_axis(Proj::axes[i]), cstr);
        g_free(cstr);
    }
    cstr = persp3d_get_VP(persp, Proj::W).coord_string();
    g_print ("  Origin: %s\n", cstr);
    g_free(cstr);

    g_print ("  Boxes: ");
    for (std::vector<SPBox3D *>::iterator i = persp_impl->boxes.begin(); i != persp_impl->boxes.end(); ++i) {
        g_print ("%d (%d)  ", (*i)->my_counter, box3d_get_perspective(*i)->perspective_impl->my_counter);
    }
    g_print ("\n");
    g_print ("========================\n");
}

void persp3d_print_debugging_info_all(SPDocument *document)
{
    for ( SPObject *child = document->getDefs()->firstChild(); child; child = child->getNext() ) {
        if (SP_IS_PERSP3D(child)) {
            persp3d_print_debugging_info(SP_PERSP3D(child));
        }
    }
    persp3d_print_all_selected();
}

void
persp3d_print_all_selected() {
    g_print ("\n======================================\n");
    g_print ("Selected perspectives and their boxes:\n");

    std::list<Persp3D *> sel_persps = SP_ACTIVE_DESKTOP->getSelection()->perspList();

    for (std::list<Persp3D *>::iterator j = sel_persps.begin(); j != sel_persps.end(); ++j) {
        Persp3D *persp = SP_PERSP3D(*j);
        Persp3DImpl *persp_impl = persp->perspective_impl;
        g_print ("  %s (%d):  ", persp->getRepr()->attribute("id"), persp->perspective_impl->my_counter);
        for (std::vector<SPBox3D *>::iterator i = persp_impl->boxes.begin();
             i != persp_impl->boxes.end(); ++i) {
            g_print ("%d ", (*i)->my_counter);
        }
        g_print ("\n");
    }
    g_print ("======================================\n\n");
 }

void print_current_persp3d(gchar *func_name, Persp3D *persp) {
    g_print ("%s: current_persp3d is now %s\n",
             func_name,
             persp ? persp->getRepr()->attribute("id") : "NULL");
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
