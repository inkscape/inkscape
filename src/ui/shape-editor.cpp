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

#include "desktop.h"
#include "document.h"
#include "gc-anchored.h"
#include "knotholder.h"
#include "ui/object-edit.h"
#include "sp-item.h"
#include "sp-object.h"
#include "ui/shape-editor.h"
#include "xml/node-event-vector.h"

//using Inkscape::createKnotHolder;

namespace Inkscape {
namespace UI {

bool ShapeEditor::_blockSetItem = false;

ShapeEditor::ShapeEditor(SPDesktop *dt) {
    this->desktop = dt;
    this->knotholder = NULL;
    this->knotholder_listener_attached_for = NULL;
}

ShapeEditor::~ShapeEditor() {
    unset_item();
}

void ShapeEditor::unset_item(bool keep_knotholder) {
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
}

bool ShapeEditor::has_knotholder() {
    return this->knotholder != NULL;
}

void ShapeEditor::update_knotholder() {
    if (this->knotholder)
        this->knotholder->update_knots();
}

bool ShapeEditor::has_local_change() {
    return (this->knotholder && this->knotholder->local_change != 0);
}

void ShapeEditor::decrement_local_change() {
    if (this->knotholder) {
        this->knotholder->local_change = FALSE;
    }
}

const SPItem *ShapeEditor::get_item() {
    const SPItem *item = NULL;
    if (this->has_knotholder()) {
        item = this->knotholder->getItem();
    }
    return item;
}

void ShapeEditor::event_attr_changed(Inkscape::XML::Node *, gchar const *name, gchar const *, gchar const *, bool, void *data)
{
    g_assert(data);
    ShapeEditor *sh = static_cast<ShapeEditor *>(data);
    bool changed_kh = false;

    if (sh->has_knotholder())
    {
        changed_kh = !sh->has_local_change();
        sh->decrement_local_change();
        if (changed_kh) {
            // this can happen if an LPEItem's knotholder handle was dragged, in which case we want
            // to keep the knotholder; in all other cases (e.g., if the LPE itself changes) we delete it
            sh->reset_item(!strcmp(name, "d"));
        }
    }
}

static Inkscape::XML::NodeEventVector shapeeditor_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    ShapeEditor::event_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};


void ShapeEditor::set_item(SPItem *item, bool keep_knotholder) {
    if (_blockSetItem) {
        return;
    }

    // this happens (and should only happen) when for an LPEItem having both knotholder and
    // nodepath the knotholder is adapted; in this case we don't want to delete the knotholder
    // since this freezes the handles
    unset_item(keep_knotholder);

    if (item) {
        Inkscape::XML::Node *repr;
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
    }
}


/** FIXME: This thing is only called when the item needs to be updated in response to repr change.
   Why not make a reload function in KnotHolder? */
void ShapeEditor::reset_item(bool keep_knotholder)
{
    if (knotholder) {
        SPObject *obj = desktop->getDocument()->getObjectByRepr(knotholder_listener_attached_for); /// note that it is not certain that this is an SPItem; it could be a LivePathEffectObject.
        set_item(SP_ITEM(obj), keep_knotholder);
    }
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

} // namespace UI
} // namespace Inkscape

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
