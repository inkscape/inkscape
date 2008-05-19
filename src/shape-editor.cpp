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
#include "node-context.h"
#include "xml/node-event-vector.h"
#include "prefs-utils.h"
#include "object-edit.h"
#include "splivarot.h"
#include "style.h"

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
    unset_item();
}

void ShapeEditor::unset_item() {

    Inkscape::XML::Node *old_repr = NULL;

    if (this->nodepath) {
        old_repr = this->nodepath->repr;
    }

    if (!old_repr && this->knotholder) {
        old_repr = this->knotholder->repr;
    }

    if (old_repr) { // remove old listener
        sp_repr_remove_listener_by_data(old_repr, this);
        Inkscape::GC::release(old_repr);
    }

    if (this->nodepath) {
        this->grab_node = -1;
        sp_nodepath_destroy(this->nodepath);
        this->nodepath = NULL;
    }

    if (this->knotholder) {
        sp_knot_holder_destroy(this->knotholder);
        this->knotholder = NULL;
    }
}

bool ShapeEditor::has_nodepath () {
    return (this->nodepath != NULL);
}

bool ShapeEditor::has_knotholder () {
    return (this->knotholder != NULL);
}

bool ShapeEditor::has_local_change () {
    if (this->nodepath)
        return (this->nodepath->local_change != 0);
    else if (this->knotholder)
        return (this->knotholder->local_change != 0);
    else
        return false;
}

void ShapeEditor::decrement_local_change () {
    if (this->nodepath) {
        if (this->nodepath->local_change > 0)
            this->nodepath->local_change--;
    } else if (this->knotholder) {
        this->knotholder->local_change = FALSE;
    }
}

SPItem *ShapeEditor::get_item () {
    SPItem *item = NULL;
    if (this->has_nodepath()) {
        item = this->nodepath->item;
    } else if (this->has_knotholder()) {
        item = SP_ITEM(this->knotholder->item);
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
    gboolean changed = FALSE;

    g_assert(data);
    ShapeEditor *sh = ((ShapeEditor *) data);

    if ( sh->has_knotholder() || ( sh->has_nodepath() && sh->nodepath_edits_repr_key(name) ) )
    {
        changed = !sh->has_local_change();
        sh->decrement_local_change();
    }

    if (changed) {
        GList *saved = NULL;
        if (sh->has_nodepath()) {
            saved = sh->save_nodepath_selection();
        }

        sh->reset_item ();

        if (sh->has_nodepath() && saved) {
            sh->restore_nodepath_selection(saved);
            g_list_free (saved);
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


void ShapeEditor::set_item(SPItem *item) {

    unset_item();

    this->grab_node = -1;

    if (item) {
        SPLPEItem *lpeitem = SP_LPE_ITEM(item);
        if (!sp_lpe_item_has_path_effect(lpeitem) ||
            !sp_lpe_item_get_livepatheffect(lpeitem)->isVisible() ||
            !sp_lpe_item_get_livepatheffect(lpeitem)->providesKnotholder()) {
            // only create nodepath if the item either doesn't have an LPE
            // or the LPE is invisible or it doesn't provide a knotholder itself
            this->nodepath =
                sp_nodepath_new(desktop, item, (prefs_get_int_attribute("tools.nodes", "show_handles", 1) != 0));
        }
        if (this->nodepath) {
            this->nodepath->shape_editor = this;
        }
        this->knotholder = sp_item_knot_holder(item, desktop);

        if (this->nodepath || this->knotholder) {
            // setting new listener
            Inkscape::XML::Node *repr;
            if (this->knotholder)
                repr = this->knotholder->repr;
            else
                repr = SP_OBJECT_REPR(item);
            if (repr) {
                Inkscape::GC::anchor(repr);
                sp_repr_add_listener(repr, &shapeeditor_repr_events, this);
            }
        }
    }
}

/** Please note that this function only works for path parameters.
*  All other parameters probably will crash Inkscape!
*/
void ShapeEditor::set_item_lpe_path_parameter(SPItem *item, SPObject *lpeobject, const char * key)
{
    unset_item();

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
ShapeEditor::set_knotholder(SPKnotHolder * knot_holder)
{
    unset_item();

    this->grab_node = -1;

    if (knot_holder) {
        this->knotholder = knot_holder;
    }
}


/** FIXME: think about this. Is this thing only called when the item needs to be updated?
   Why not make a reload function in NodePath and in KnotHolder? */
void ShapeEditor::reset_item ()
{
    if ( (this->nodepath) && (IS_LIVEPATHEFFECT(this->nodepath->object)) ) {
        SPItem * item = this->nodepath->item;
        SPObject *obj = this->nodepath->object;
        char * key = g_strdup(this->nodepath->repr_key);
        set_item_lpe_path_parameter(item, obj, key); // the above checks for nodepath, so it is indeed a path that we are editing
        g_free(key);
    } else {
        SPItem * item = get_item();
        set_item(item);
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

    SPItem *item = get_item();

    //Translate click point into proper coord system
    this->curvepoint_doc = desktop->w2d(event_p);
    this->curvepoint_doc *= sp_item_dt2i_affine(item);

    sp_nodepath_ensure_livarot_path(this->nodepath);

    NR::Maybe<Path::cut_position> position = get_nearest_position_on_Path(this->nodepath->livarot_path, this->curvepoint_doc);
    if (!position) {
        return false;
    }

    NR::Point nearest = get_point_on_Path(this->nodepath->livarot_path, position->piece, position->t);
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
        this->curvepoint_event[NR::X] = (gint) event_p [NR::X];
        this->curvepoint_event[NR::Y] = (gint) event_p [NR::Y];
        this->hit = true;
        this->grab_t = position->t;
        this->grab_node = position->piece;
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
    } else if (this->knotholder) {
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

    } else if (this->knotholder) {
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

void ShapeEditor::flip (NR::Dim2 axis, NR::Maybe<NR::Point> center) {
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
