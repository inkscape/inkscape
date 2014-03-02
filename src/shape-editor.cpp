/*
 * Inkscape::ShapeEditor
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <glibmm/i18n.h>

#include "sp-object.h"
#include "sp-item.h"
#include "sp-lpe-item.h"
#include "live_effects/lpeobject.h"
#include "selection.h"
#include "desktop.h"
#include "document.h"
#include "desktop-handles.h"
#include "knotholder.h"
#include "live_effects/parameter/point.h"
#include "xml/node-event-vector.h"
#include "preferences.h"
#include "object-edit.h"
#include "style.h"
#include "display/curve.h"
#include <2geom/pathvector.h>

#include "shape-editor.h"

using Inkscape::createKnotHolder;

bool ShapeEditor::_blockSetItem = false;

ShapeEditor::ShapeEditor(SPDesktop *dt) {
    this->desktop = dt;
    this->knotholder = NULL;
    this->knotholder_listener_attached_for = NULL;
}

ShapeEditor::~ShapeEditor() {
    unset_item(SH_KNOTHOLDER);
}

void ShapeEditor::unset_item(SubType type, bool keep_knotholder) {
    switch (type) {
        case SH_NODEPATH:
            // defunct
            break;
        case SH_KNOTHOLDER:
            if (this->knotholder) {
                Inkscape::XML::Node *old_repr = this->knotholder->repr;
                if (old_repr && old_repr == knotholder_listener_attached_for) {
                    sp_repr_remove_listener_by_data(old_repr, this);
                    Inkscape::GC::release(old_repr);
                    knotholder_listener_attached_for = NULL;
                }

                if (!keep_knotholder) {
                    delete this->knotholder;
                    this->knotholder = NULL;
                }
            }
            break;
    }
}

bool ShapeEditor::has_nodepath () {
    return false;
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
            // defunct
            return false;
        case SH_KNOTHOLDER:
            return (this->knotholder && this->knotholder->local_change != 0);
        default:
            g_assert_not_reached();
    }
}

void ShapeEditor::decrement_local_change (SubType type) {
    switch (type) {
        case SH_NODEPATH:
            // defunct
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
            // defunct
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
    // defunct stub
    return NULL;
}

void ShapeEditor::restore_nodepath_selection (GList */*saved*/) {
    // defunct stub
}

void ShapeEditor::shapeeditor_event_attr_changed(gchar const *name)
{
    gboolean changed_kh = FALSE;

    if (has_knotholder())
    {
        changed_kh = !has_local_change(SH_KNOTHOLDER);
        decrement_local_change(SH_KNOTHOLDER);
        if (changed_kh) {
            // this can happen if an LPEItem's knotholder handle was dragged, in which case we want
            // to keep the knotholder; in all other cases (e.g., if the LPE itself changes) we delete it
            reset_item(SH_KNOTHOLDER, !strcmp(name, "d"));
        }
    }
}


static void shapeeditor_event_attr_changed(Inkscape::XML::Node */*repr*/, gchar const *name,
                                           gchar const */*old_value*/, gchar const */*new_value*/,
                                           bool /*is_interactive*/, gpointer data)
{
    g_assert(data);
    ShapeEditor *sh = static_cast<ShapeEditor *>(data);

    sh->shapeeditor_event_attr_changed(name);
}

static Inkscape::XML::NodeEventVector shapeeditor_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    shapeeditor_event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};


void ShapeEditor::set_item(SPItem *item, SubType type, bool keep_knotholder) {
    if (_blockSetItem) {
        return;
    }

    // this happens (and should only happen) when for an LPEItem having both knotholder and
    // nodepath the knotholder is adapted; in this case we don't want to delete the knotholder
    // since this freezes the handles
    unset_item(type, keep_knotholder);

    if (item) {
        Inkscape::XML::Node *repr;
        switch(type) {
            case SH_NODEPATH:
                // defunct
                break;

            case SH_KNOTHOLDER:
                if (!this->knotholder) {
                    // only recreate knotholder if none is present
                    this->knotholder = createKnotHolder(item, desktop);
                }
                if (this->knotholder) {
                    this->knotholder->update_knots();
                    // setting new listener
                    repr = this->knotholder->repr;
                    if (repr != knotholder_listener_attached_for) {
                        Inkscape::GC::anchor(repr);
                        sp_repr_add_listener(repr, &shapeeditor_repr_events, this);
                        knotholder_listener_attached_for = repr;
                    }
                }
                break;
        }
    }
}


/** FIXME: This thing is only called when the item needs to be updated in response to repr change.
   Why not make a reload function in NodePath and in KnotHolder? */
void ShapeEditor::reset_item (SubType type, bool keep_knotholder)
{
    switch (type) {
        case SH_NODEPATH:
            // defunct
            break;
        case SH_KNOTHOLDER:
            if ( knotholder ) {
                SPObject *obj = sp_desktop_document(desktop)->getObjectByRepr(knotholder_listener_attached_for); /// note that it is not certain that this is an SPItem; it could be a LivePathEffectObject.
                set_item(SP_ITEM(obj), SH_KNOTHOLDER, keep_knotholder);
            }
            break;
    }
}

void ShapeEditor::nodepath_destroyed () {
}

bool ShapeEditor::has_selection() {
    return false; //  so far, knotholder cannot have selection
}

/**
 * Returns true if this ShapeEditor has a knot above which the mouse currently hovers.
 */
bool ShapeEditor::knot_mouseover() const {
    if (this->knotholder) {
        return knotholder->knot_mouseover();
    }

    return false;
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
