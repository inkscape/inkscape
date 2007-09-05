/**
 * \brief LivePathEffect dialog
 *
 * Authors:
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *
 * Copyright (C) 2007 Author
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/i18n.h>
#include "livepatheffect-editor.h"
#include "verbs.h"
#include "selection.h"
#include "sp-shape.h"
#include "sp-path.h"
#include "live_effects/effect.h"
#include "live_effects/lpeobject.h"
#include "gtkmm/widget.h"
#include <vector>
#include "inkscape.h"
#include "desktop-handles.h"
#include "desktop.h"
#include "document-private.h"
#include "xml/node.h"
#include "xml/document.h"

namespace Inkscape {
class Application;

namespace UI {
namespace Dialog {


/*####################
 * Callback functions
 */
static void lpeeditor_selection_changed (Inkscape::Selection * selection, gpointer data)
{
    LivePathEffectEditor *lpeeditor = static_cast<LivePathEffectEditor *>(data);
    lpeeditor->onSelectionChanged(selection);
}

static void lpeeditor_selection_modified (Inkscape::Selection *selection, guint flags, gpointer data)
{
    lpeeditor_selection_changed (selection, data);
}


static void lpeeditor_desktop_change(Inkscape::Application*, SPDesktop* desktop, void *data)
{
    if (!desktop) {
        return;
    }
    LivePathEffectEditor* editor = reinterpret_cast<LivePathEffectEditor*>(data);
    editor->setDesktop(desktop);
}



/*#######################
 * LivePathEffectEditor
 */
LivePathEffectEditor::LivePathEffectEditor(Behavior::BehaviorFactory behavior_factory) 
    : Dialog (behavior_factory, "dialogs.livepatheffect", SP_VERB_DIALOG_LIVE_PATH_EFFECT),
      combo_effecttype(Inkscape::LivePathEffect::LPETypeConverter),
      button_apply(_("_Apply"), _("Apply chosen effect to selection")),
      button_remove(_("_Remove"), _("Remove effect from selection")),
      effectwidget(NULL),
      explain_label("", Gtk::ALIGN_CENTER),
      effectapplication_frame(_("Apply new effect")),
      effectcontrol_frame(_("Current effect")),
      current_desktop(NULL)
{
    // Top level vbox
    Gtk::VBox *vbox = get_vbox();
    vbox->set_spacing(4);

    effectapplication_vbox.set_spacing(4);
    effectcontrol_vbox.set_spacing(4);

    effectapplication_vbox.pack_start(combo_effecttype, true, true);
    effectapplication_vbox.pack_start(button_apply, true, true);
    effectapplication_frame.add(effectapplication_vbox);

    effectcontrol_vbox.pack_start(explain_label, true, true);
    effectcontrol_vbox.pack_end(button_remove, true, true);
    effectcontrol_frame.add(effectcontrol_vbox);

    vbox->pack_start(effectapplication_frame, true, true);
    vbox->pack_start(effectcontrol_frame, true, true);

    // connect callback functions to buttons
    button_apply.signal_clicked().connect(sigc::mem_fun(*this, &LivePathEffectEditor::onApply));
    button_remove.signal_clicked().connect(sigc::mem_fun(*this, &LivePathEffectEditor::onRemove));

    // connect callback functions to changes in selected desktop.
    g_signal_connect( G_OBJECT(INKSCAPE), "activate_desktop",
                       G_CALLBACK(lpeeditor_desktop_change), this);

    g_signal_connect( G_OBJECT(INKSCAPE), "deactivate_desktop",
                       G_CALLBACK(lpeeditor_desktop_change), this);

    setDesktop(SP_ACTIVE_DESKTOP);
    show_all_children();
		button_remove.hide();
}

LivePathEffectEditor::~LivePathEffectEditor() 
{
    if (effectwidget) {
        effectcontrol_vbox.remove(*effectwidget);
        effectwidget = NULL;
    }

    if (current_desktop) {
        selection_changed_connection.disconnect();
        selection_modified_connection.disconnect();
    }
}

void
LivePathEffectEditor::showParams(LivePathEffect::Effect* effect)
{
    if (effectwidget) {
        effectcontrol_vbox.remove(*effectwidget);
        effectwidget = NULL;
    }

    explain_label.set_markup("<b>" + effect->getName() + "</b>");
    effectwidget = effect->getWidget();
    if (effectwidget) {
        effectcontrol_vbox.pack_start(*effectwidget, true, true);
    }
    button_remove.show();

    effectcontrol_vbox.show_all_children();
    // fixme: do resizing of dialog 
}

void
LivePathEffectEditor::showText(Glib::ustring const &str)
{
    if (effectwidget) {
        effectcontrol_vbox.remove(*effectwidget);
        effectwidget = NULL;
    }

    explain_label.set_label(str);
    button_remove.hide();

    // fixme: do resizing of dialog ?
}

void
LivePathEffectEditor::set_sensitize_all(bool sensitive)
{
    combo_effecttype.set_sensitive(sensitive);
    button_apply.set_sensitive(sensitive);
    button_remove.set_sensitive(sensitive);
}

void
LivePathEffectEditor::onSelectionChanged(Inkscape::Selection *sel)
{
    if ( sel && !sel->isEmpty() ) {
        SPItem *item = sel->singleItem();
        if ( item ) {
            if ( SP_IS_SHAPE(item) ) {
                SPShape *shape = SP_SHAPE(item);
                LivePathEffectObject *lpeobj = sp_shape_get_livepatheffectobject(shape);
                set_sensitize_all(true);
                if (lpeobj) {
                    if (lpeobj->lpe) {
                        showParams(lpeobj->lpe);
                    } else {
                        showText(_("Unknown effect is applied"));
                    }
                } else {
                    showText(_("No effect applied"));
                    button_remove.set_sensitive(false);
                }
            } else {
                showText(_("Item is not a shape or path"));
                set_sensitize_all(false);
            }
        } else {
            showText(_("Only one item can be selected"));
            set_sensitize_all(false);
        }
    } else {
        showText(_("Empty selection"));
        set_sensitize_all(false);
    }
}

void 
LivePathEffectEditor::setDesktop(SPDesktop *desktop)
{

    if ( desktop == current_desktop ) {
        return;
    }

    if (current_desktop) {
        selection_changed_connection.disconnect();
        selection_modified_connection.disconnect();
    }

    current_desktop = desktop;
    if (desktop) {
        Inkscape::Selection *selection = sp_desktop_selection(desktop);
        selection_changed_connection = selection->connectChanged(
            sigc::bind (sigc::ptr_fun(&lpeeditor_selection_changed), this ) );
        selection_modified_connection = selection->connectModified(
            sigc::bind (sigc::ptr_fun(&lpeeditor_selection_modified), this ) );

        onSelectionChanged(selection);
    } else {
        onSelectionChanged(NULL);
    }
}




/*########################################################################
# BUTTON CLICK HANDLERS    (callbacks)
########################################################################*/

// TODO:  factor out the effect applying code which can be called from anywhere. (selection-chemistry.cpp also needs it)

void
LivePathEffectEditor::onApply()
{
    Inkscape::Selection *sel = _getSelection();
    if ( sel && !sel->isEmpty() ) {
        SPItem *item = sel->singleItem();
        if ( item && SP_IS_SHAPE(item) ) {
            SPDocument *doc = current_desktop->doc();

            const Util::EnumData<LivePathEffect::EffectType>* data = combo_effecttype.get_active_data();
            if (!data) return;

            Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);
            Inkscape::XML::Node *repr = xml_doc->createElement("inkscape:path-effect");
            repr->setAttribute("effect", data->key.c_str() );

            SP_OBJECT_REPR(SP_DOCUMENT_DEFS(doc))->addChild(repr, NULL); // adds to <defs> and assigns the 'id' attribute
            const gchar * repr_id = repr->attribute("id");
            Inkscape::GC::release(repr);

            gchar *href = g_strdup_printf("#%s", repr_id);
            sp_shape_set_path_effect(SP_SHAPE(item), href);
            g_free(href);

            // make sure there is an original-d for paths!!!
            if ( SP_IS_PATH(item) ) {
                Inkscape::XML::Node *pathrepr = SP_OBJECT_REPR(item);
                if ( ! pathrepr->attribute("inkscape:original-d") ) {
                    pathrepr->setAttribute("inkscape:original-d", pathrepr->attribute("d"));
                }
            }

            sp_document_done(doc, SP_VERB_DIALOG_LIVE_PATH_EFFECT, 
                             _("Create and apply path effect"));
        }
    }
}

void
LivePathEffectEditor::onRemove()
{
    Inkscape::Selection *sel = _getSelection();
    if ( sel && !sel->isEmpty() ) {
        SPItem *item = sel->singleItem();
        if ( item && SP_IS_SHAPE(item) ) {
            sp_shape_remove_path_effect(SP_SHAPE(item));
            sp_document_done ( sp_desktop_document (current_desktop), SP_VERB_DIALOG_LIVE_PATH_EFFECT, 
                               _("Remove path effect") );
        }
    }
}



} // namespace Dialog
} // namespace UI
} // namespace Inkscape

