#define __SHAPE_EDITOR_CPP__

/*
 * Inkscape::ShapeEditor
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <glibmm/i18n.h>

#include "sp-object.h"
#include "sp-item.h"
#include "live_effects/lpeobject.h"
#include "selection.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "knotholder.h"
#include "live_effects/parameter/pointparam-knotholder.h"
#include "nodepath.h"
#include "xml/node-event-vector.h"
#include "prefs-utils.h"
#include "object-edit.h"
#include "style.h"
#include "display/curve.h"
#include <2geom/pathvector.h>

#include "shape-editor.h"


ShapeEditorsCollective::ShapeEditorsCollective(SPDesktop */*dt*/) {
}

ShapeEditorsCollective::~ShapeEditorsCollective() {
}


void ShapeEditorsCollective::update_statusbar() {

//!!! move from nodepath: sp_nodepath_update_statusbar but summing for all nodepaths

}

ShapeEditor::ShapeEditor(SPDesktop *dt) {
    this->desktop = dt;
    this->grab_node = -1;
    this->nodepath = NULL;
    this->knotholder = NULL;
    this->hit = false;
}

ShapeEditor::~ShapeEditor() {
    unset_item(SH_KNOTHOLDER);
    unset_item(SH_NODEPATH);
}

void ShapeEditor::unset_item(SubType type, bool keep_knotholder) {
    Inkscape::XML::Node *old_repr = NULL;

    switch (type) {
        case SH_NODEPATH:
            if (this->nodepath) {
                old_repr = this->nodepath->repr;
                sp_repr_remove_listener_by_data(old_repr, this);
                Inkscape::GC::release(old_repr);

                this->grab_node = -1;
                sp_nodepath_destroy(this->nodepath);
                this->nodepath = NULL;
            }
            break;
        case SH_KNOTHOLDER:
            if (this->knotholder) {
                old_repr = this->knotholder->repr;
                sp_repr_remove_listener_by_data(old_repr, this);
                Inkscape::GC::release(old_repr);

                if (!keep_knotholder) {
                    delete this->knotholder;
                    this->knotholder = NULL;
                }
            }
            break;
    }
}

bool ShapeEditor::has_nodepath () {
    return (this->nodepath != NULL);
}

bool ShapeEditor::has_knotholder () {
    return (this->knotholder != NULL);
}

void ShapeEditor::update_knotholder () {
    if (this->knotholder)
        this->knotholder->update_knots();
}

bool ShapeEditor::has_local_change (SubType type) {
    switch (type) {
        case SH_NODEPATH:
            return (this->nodepath && this->nodepath->local_change);
        case SH_KNOTHOLDER:
            return (this->knotholder && this->knotholder->local_change != 0);
        default:
            g_assert_not_reached();
    }
}

void ShapeEditor::decrement_local_change (SubType type) {
    switch (type) {
        case SH_NODEPATH:
            if (this->nodepath && this->nodepath->local_change > 0) {
                this->nodepath->local_change--;
            }
            break;
        case SH_KNOTHOLDER:
            if (this->knotholder) {
                this->knotholder->local_change = FALSE;
            }
            break;
        default:
            g_assert_not_reached();
    }
}

const SPItem *ShapeEditor::get_item (SubType type) {
    const SPItem *item = NULL;
    switch (type) {
        case SH_NODEPATH:
            if (this->has_nodepath()) {
                item = this->nodepath->item;
            }
            break;
        case SH_KNOTHOLDER:
            if (this->has_knotholder()) {
                item = this->knotholder->getItem();
            }
            break;
    }
    return item;
}

GList *ShapeEditor::save_nodepath_selection () {
    if (this->nodepath)
        return ::save_nodepath_selection (this->nodepath);
    return NULL;
}

void ShapeEditor::restore_nodepath_selection (GList *saved) {
    if (this->nodepath && saved)
        ::restore_nodepath_selection (this->nodepath, saved);
}

bool ShapeEditor::nodepath_edits_repr_key(gchar const *name) {
    if (nodepath && name) {
        return ( !strcmp(name, nodepath->repr_key) || !strcmp(name, nodepath->repr_nodetypes_key) );
    }

    return false;
}

static void shapeeditor_event_attr_changed(Inkscape::XML::Node */*repr*/, gchar const *name,
                                           gchar const */*old_value*/, gchar const */*new_value*/,
                                           bool /*is_interactive*/, gpointer data)
{
    gboolean changed_np = FALSE;
    gboolean changed_kh = FALSE;

    g_assert(data);
    ShapeEditor *sh = ((ShapeEditor *) data);

    if (sh->has_nodepath() && sh->nodepath_edits_repr_key(name))
    {
        changed_np = !sh->has_local_change(SH_NODEPATH);
        sh->decrement_local_change(SH_NODEPATH);

    }

    if (changed_np) {
        GList *saved = NULL;
        if (sh->has_nodepath()) {
            saved = sh->save_nodepath_selection();
        }

        sh->reset_item(SH_NODEPATH);

        if (sh->has_nodepath() && saved) {
            sh->restore_nodepath_selection(saved);
            g_list_free (saved);
        }
    }


    if (sh->has_knotholder())
    {
        changed_kh = !sh->has_local_change(SH_KNOTHOLDER);
        sh->decrement_local_change(SH_KNOTHOLDER);
        if (changed_kh) {
            // this can happen if an LPEItem's knotholder handle was dragged, in which case we want
            // to keep the knotholder; in all other cases (e.g., if the LPE itself changes) we delete it
            sh->reset_item(SH_KNOTHOLDER, !strcmp(name, "d"));
        }
    }

    sh->update_statusbar(); //TODO: sh->get_container()->update_statusbar();
}

static Inkscape::XML::NodeEventVector shapeeditor_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    shapeeditor_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};


void ShapeEditor::set_item(SPItem *item, SubType type, bool keep_knotholder) {
    // this happens (and should only happen) when for an LPEItem having both knotholder and nodepath the knotholder
    // is adapted; in this case we don't want to delete the knotholder since this freezes the handles
    unset_item(type, keep_knotholder);

    this->grab_node = -1;

    if (item) {
        Inkscape::XML::Node *repr;
        switch(type) {
            case SH_NODEPATH:
                if (SP_IS_LPE_ITEM(item)) {
                    this->nodepath = sp_nodepath_new(desktop, item, (prefs_get_int_attribute("tools.nodes", "show_handles", 1) != 0));
                }
                if (this->nodepath) {
                    this->nodepath->shape_editor = this;

                    // setting new listener
                    repr = SP_OBJECT_REPR(item);
                    Inkscape::GC::anchor(repr);
                    sp_repr_add_listener(repr, &shapeeditor_repr_events, this);
                }
                break;

            case SH_KNOTHOLDER:
                if (!this->knotholder) {
                    // only recreate knotholder if none is present
                    this->knotholder = sp_item_knot_holder(item, desktop);
                }
                if (this->knotholder) {
                    this->knotholder->update_knots();
                    // setting new listener
                    repr = this->knotholder->repr;
                    Inkscape::GC::anchor(repr);
                    sp_repr_add_listener(repr, &shapeeditor_repr_events, this);
                }
                break;
        }
    }
}

/** Please note that this function only works for path parameters.
*  All other parameters probably will crash Inkscape!
*/
void ShapeEditor::set_item_lpe_path_parameter(SPItem *item, SPObject *lpeobject, const char * key)
{
    unset_item(SH_NODEPATH);

    this->grab_node = -1;

    if (lpeobject) {
        this->nodepath = sp_nodepath_new( desktop, lpeobject,
                                          (prefs_get_int_attribute("tools.nodes", "show_handles", 1) != 0),
                                          key, item);
        if (this->nodepath) {
            this->nodepath->shape_editor = this;

            // setting new listener
            Inkscape::XML::Node *repr = SP_OBJECT_REPR(lpeobject);
            if (repr) {
                Inkscape::GC::anchor(repr);
                sp_repr_add_listener(repr, &shapeeditor_repr_events, this);
            }
        }
    }
}

/** 
*  pass a new knotholder to ShapeEditor to manage (and delete)
*/
void
ShapeEditor::set_knotholder(KnotHolder * knot_holder)
{
    unset_item(SH_KNOTHOLDER);

    this->grab_node = -1;

    if (knot_holder) {
        this->knotholder = knot_holder;
    }
}


/** FIXME: think about this. Is this thing only called when the item needs to be updated?
   Why not make a reload function in NodePath and in KnotHolder? */
void ShapeEditor::reset_item (SubType type, bool keep_knotholder)
{
    switch (type) {
        case SH_NODEPATH:
            if ( (this->nodepath) && (IS_LIVEPATHEFFECT(this->nodepath->object)) ) {
                SPItem * item = this->nodepath->item;
                SPObject *obj = this->nodepath->object;
                char * key = g_strdup(this->nodepath->repr_key);
                set_item_lpe_path_parameter(item, obj, key); // the above checks for nodepath, so it is indeed a path that we are editing
                g_free(key);
            } else {
                SPItem * item = (SPItem *) get_item(SH_NODEPATH);
                set_item(item, SH_NODEPATH);
            }                
            break;
        case SH_KNOTHOLDER:
            if (this->knotholder) {
                SPItem * item = (SPItem *) get_item(SH_KNOTHOLDER);
                set_item(item, SH_KNOTHOLDER, keep_knotholder);
            }
            break;
    }
}

void ShapeEditor::nodepath_destroyed () {
    this->nodepath = NULL;
}

void ShapeEditor::update_statusbar () {
    if (this->nodepath)
        sp_nodepath_update_statusbar(this->nodepath);
}

bool ShapeEditor::is_over_stroke (NR::Point event_p, bool remember) {

    if (!this->nodepath)
        return false; // no stroke in knotholder

    const SPItem *item = get_item(SH_NODEPATH);

    //Translate click point into proper coord system
    this->curvepoint_doc = desktop->w2d(event_p);
    this->curvepoint_doc *= sp_item_dt2i_affine(item);

    SPCurve *curve = this->nodepath->curve;   // not sure if np->curve is always up to date...
    Geom::PathVector const &pathv = curve->get_pathvector();
    Geom::PathVectorPosition pvpos = Geom::nearestPoint(pathv, this->curvepoint_doc);

    NR::Point nearest = pathv[pvpos.path_nr].pointAt(pvpos.t);
    NR::Point delta = nearest - this->curvepoint_doc;

    delta = desktop->d2w(delta);

    double stroke_tolerance =
        (( !SP_OBJECT_STYLE(item)->stroke.isNone() ?
           desktop->current_zoom() *
           SP_OBJECT_STYLE (item)->stroke_width.computed * 0.5 *
           NR::expansion(sp_item_i2d_affine(item))
         : 0.0)
         + prefs_get_int_attribute_limited("options.dragtolerance", "value", 0, 0, 100)) /NR::expansion(sp_item_i2d_affine(item)); 
    bool close = (NR::L2 (delta) < stroke_tolerance);

    if (remember && close) {
        // calculate index for nodepath's representation.
        double int_part;
        double t = std::modf(pvpos.t, &int_part);
        unsigned int segment_index = (unsigned int)int_part + 1;
        for (unsigned int i = 0; i < pvpos.path_nr; ++i) {
            segment_index += pathv[i].size() + 1;
            if (pathv[i].closed())
                segment_index += 1;
        }

        this->curvepoint_event[NR::X] = (gint) event_p [NR::X];
        this->curvepoint_event[NR::Y] = (gint) event_p [NR::Y];
        this->hit = true;
        this->grab_t = t;
        this->grab_node = segment_index;
    }

    return close;
}

void ShapeEditor::add_node_near_point() {
    if (this->nodepath) {
        sp_nodepath_add_node_near_point(this->nodepath, this->curvepoint_doc);
    } else if (this->knotholder) {
        // we do not add nodes in knotholder... yet
    }
}

void ShapeEditor::select_segment_near_point(bool toggle) {
    if (this->nodepath) {
        sp_nodepath_select_segment_near_point(this->nodepath, this->curvepoint_doc, toggle);
    }
    if (this->knotholder) {
        // we do not select segments in knotholder... yet?
    }
}

void ShapeEditor::cancel_hit() {
    this->hit = false;
}

bool ShapeEditor::hits_curve() {
    return (this->hit);
}


void ShapeEditor::curve_drag(gdouble eventx, gdouble eventy) {
    if (this->nodepath && !this->nodepath->straight_path) {

        if (this->grab_node == -1) // don't know which segment to drag
            return;

        // We round off the extra precision in the motion coordinates provided
        // by some input devices (like tablets). As we'll store the coordinates
        // as integers in curvepoint_event we need to do this rounding before
        // comparing them with the last coordinates from curvepoint_event.
        // See bug #1593499 for details.

        gint x = (gint) Inkscape::round(eventx);
        gint y = (gint) Inkscape::round(eventy);


        // The coordinates hasn't changed since the last motion event, abort
        if (this->curvepoint_event[NR::X] == x &&
            this->curvepoint_event[NR::Y] == y)
            return;

        NR::Point const delta_w(eventx - this->curvepoint_event[NR::X],
                                eventy - this->curvepoint_event[NR::Y]);
        NR::Point const delta_dt(this->desktop->w2d(delta_w));

        sp_nodepath_curve_drag (this->grab_node, this->grab_t, delta_dt); //!!! FIXME: which nodepath?!!! also uses current!!!
        this->curvepoint_event[NR::X] = x;
        this->curvepoint_event[NR::Y] = y;

    }
    if (this->knotholder) {
        // we do not drag curve in knotholder
    }

}

void ShapeEditor::finish_drag() {
    if (this->nodepath && this->hit) {
        sp_nodepath_update_repr (this->nodepath, _("Drag curve"));
    }
}

void ShapeEditor::select_rect(NR::Rect const &rect, bool add) {
    if (this->nodepath) {
        sp_nodepath_select_rect(this->nodepath, rect, add);
    }
}

bool ShapeEditor::has_selection() {
    if (this->nodepath)
        return this->nodepath->selected;
    return false; //  so far, knotholder cannot have selection
}

void ShapeEditor::deselect() {
    if (this->nodepath)
        sp_nodepath_deselect(this->nodepath);
}

void ShapeEditor::add_node () {
    sp_node_selected_add_node(this->nodepath);
}

void ShapeEditor::delete_nodes () {
    sp_node_selected_delete(this->nodepath);
}

void ShapeEditor::delete_nodes_preserving_shape () {
    if (this->nodepath && this->nodepath->selected) {
        sp_node_delete_preserve(g_list_copy(this->nodepath->selected));
    }
}

void ShapeEditor::delete_segment () {
    sp_node_selected_delete_segment(this->nodepath);
}

void ShapeEditor::set_node_type(int type) {
    sp_node_selected_set_type(this->nodepath, (Inkscape::NodePath::NodeType) type);
}

void ShapeEditor::break_at_nodes() {
    sp_node_selected_break(this->nodepath);
}

void ShapeEditor::join_nodes() {
    sp_node_selected_join(this->nodepath);
}

void ShapeEditor::join_segments() {
    sp_node_selected_join_segment(this->nodepath);
}

void ShapeEditor::duplicate_nodes() {
    sp_node_selected_duplicate(this->nodepath);
}

void ShapeEditor::set_type_of_segments(NRPathcode code) {
    sp_node_selected_set_line_type(this->nodepath, code);
}

void ShapeEditor::move_nodes_screen(gdouble dx, gdouble dy) {
    sp_node_selected_move_screen(this->nodepath, dx, dy);
}
void ShapeEditor::move_nodes(gdouble dx, gdouble dy) {
    sp_node_selected_move(this->nodepath, dx, dy);
}

void ShapeEditor::rotate_nodes(gdouble angle, int which, bool screen) {
    if (this->nodepath)
        sp_nodepath_selected_nodes_rotate (this->nodepath, angle, which, screen);
}

void ShapeEditor::scale_nodes(gdouble const grow, int const which) {
    sp_nodepath_selected_nodes_scale(this->nodepath, grow, which);
}
void ShapeEditor::scale_nodes_screen(gdouble const grow, int const which) {
    sp_nodepath_selected_nodes_scale_screen(this->nodepath, grow, which);
}

void ShapeEditor::select_all (bool invert) {
    if (this->nodepath)
        sp_nodepath_select_all (this->nodepath, invert);
}
void ShapeEditor::select_all_from_subpath (bool invert) {
    if (this->nodepath)
        sp_nodepath_select_all_from_subpath (this->nodepath, invert);
}
void ShapeEditor::select_next () {
    if (this->nodepath) {
        sp_nodepath_select_next (this->nodepath);
        if (this->nodepath->numSelected() >= 1) {
            this->desktop->scroll_to_point(&(this->nodepath->singleSelectedCoords()), 1.0);
        }
    }
}
void ShapeEditor::select_prev () {
    if (this->nodepath) {
        sp_nodepath_select_prev (this->nodepath);
        if (this->nodepath->numSelected() >= 1) {
            this->desktop->scroll_to_point(&(this->nodepath->singleSelectedCoords()), 1.0);
        }
    }
}

void ShapeEditor::show_handles (bool show) {
    if (this->nodepath && !this->nodepath->straight_path)
        sp_nodepath_show_handles (this->nodepath, show);
}

void ShapeEditor::show_helperpath (bool show) {
    if (this->nodepath)
        sp_nodepath_show_helperpath (this->nodepath, show);
}

void ShapeEditor::flip (NR::Dim2 axis, boost::optional<NR::Point> center) {
    if (this->nodepath)
        sp_nodepath_flip (this->nodepath, axis, center);
}

void ShapeEditor::distribute (NR::Dim2 axis) {
    if (this->nodepath)
        sp_nodepath_selected_distribute (this->nodepath, axis);
}
void ShapeEditor::align (NR::Dim2 axis) {
    if (this->nodepath)
        sp_nodepath_selected_align (this->nodepath, axis);
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
