/** @file
 * @brief Live Path Effect editing dialog - implementation
 */
/* Authors:
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *   Steren Giannini <steren.giannini@gmail.com>
 *   Bastien Bouclet <bgkweb@gmail.com>
 *
 * Copyright (C) 2007 Authors
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/i18n.h>
#include <gtkmm/stock.h>
#include <gtkmm/toolbar.h>
#include <vector>

#include "desktop.h"
#include "desktop-handles.h"
#include "document.h"
#include "gtkmm/widget.h"
#include "inkscape.h"
#include "live_effects/effect.h"
#include "live_effects/lpeobject.h"
#include "live_effects/lpeobject-reference.h"
#include "path-chemistry.h"
#include "selection.h"
#include "sp-item-group.h"
#include "sp-lpe-item.h"
#include "sp-path.h"
#include "sp-rect.h"
#include "sp-shape.h"
#include "ui/icon-names.h"
#include "ui/widget/imagetoggler.h"
#include "verbs.h"
#include "xml/node.h"

#include "livepatheffect-editor.h"

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

static void lpeeditor_selection_modified (Inkscape::Selection * selection, guint /*flags*/, gpointer data)
{
    LivePathEffectEditor *lpeeditor = static_cast<LivePathEffectEditor *>(data);
    lpeeditor->onSelectionChanged(selection);
}


/*#######################
 * LivePathEffectEditor
 */

LivePathEffectEditor::LivePathEffectEditor()
    : UI::Widget::Panel("", "/dialogs/livepatheffect", SP_VERB_DIALOG_LIVE_PATH_EFFECT),
      lpe_list_locked(false),
      combo_effecttype(Inkscape::LivePathEffect::LPETypeConverter),
      effectwidget(NULL),
      explain_label("", Gtk::ALIGN_CENTER),
      // TRANSLATORS: this dialog is accessible via menu Path - Path Effect Editor...
      effectapplication_frame(_("Apply new effect")),
      effectcontrol_frame(_("Current effect")),
      effectlist_frame(_("Effect list")),
      button_up(Gtk::Stock::GO_UP),
      button_down(Gtk::Stock::GO_DOWN),
      button_apply(Gtk::Stock::ADD),
      button_remove(Gtk::Stock::REMOVE),
      current_desktop(NULL),
      current_lpeitem(NULL)
{
    Gtk::Box *contents = _getContents();
    contents->set_spacing(4);

    //Add the TreeView, inside a ScrolledWindow, with the button underneath:
    scrolled_window.add(effectlist_view);
    //Only show the scrollbars when they are necessary:
    scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    effectapplication_hbox.set_spacing(4);
    effectcontrol_vbox.set_spacing(4);
    effectlist_vbox.set_spacing(4);

    effectapplication_hbox.pack_start(combo_effecttype, true, true);
    effectapplication_hbox.pack_start(button_apply, true, true);
    effectapplication_frame.add(effectapplication_hbox);

    effectlist_vbox.pack_start(scrolled_window, Gtk::PACK_EXPAND_WIDGET);
    effectlist_vbox.pack_end(toolbar, Gtk::PACK_SHRINK);
   // effectlist_vbox.pack_end(button_hbox, Gtk::PACK_SHRINK);
    effectlist_frame.add(effectlist_vbox);

    effectcontrol_vbox.pack_start(explain_label, true, true);
    effectcontrol_frame.add(effectcontrol_vbox);

 //   button_hbox.pack_start(button_up, true, true);
 //   button_hbox.pack_start(button_down, true, true);
 //   button_hbox.pack_end(button_remove, true, true);
    toolbar.set_toolbar_style(Gtk::TOOLBAR_ICONS);
 // Add toolbar items to toolbar
    toolbar.append(button_up);
    toolbar.append(button_down);
    toolbar.append(button_remove);


    // Add toolbar
    //add_toolbar(toolbar);
    toolbar.show_all(); //Show the toolbar and all its child widgets.


    //Create the Tree model:
    effectlist_store = Gtk::ListStore::create(columns);
    effectlist_view.set_model(effectlist_store);

    effectlist_view.set_headers_visible(false);

    // Handle tree selections
    effectlist_selection = effectlist_view.get_selection();
    effectlist_selection->signal_changed().connect( sigc::mem_fun(*this, &LivePathEffectEditor::on_effect_selection_changed) );

    //Add the visibility icon column:
    Inkscape::UI::Widget::ImageToggler *eyeRenderer = manage( new Inkscape::UI::Widget::ImageToggler(
        INKSCAPE_ICON_OBJECT_VISIBLE, INKSCAPE_ICON_OBJECT_HIDDEN) );
    int visibleColNum = effectlist_view.append_column("is_visible", *eyeRenderer) - 1;
    eyeRenderer->signal_toggled().connect( sigc::mem_fun(*this, &LivePathEffectEditor::on_visibility_toggled) );
    eyeRenderer->property_activatable() = true;
    Gtk::TreeViewColumn* col = effectlist_view.get_column(visibleColNum);
    if ( col ) {
        col->add_attribute( eyeRenderer->property_active(), columns.col_visible );
    }

    //Add the effect name column:
    effectlist_view.append_column("Effect", columns.col_name);

    contents->pack_start(effectapplication_frame, false, false);
    contents->pack_start(effectlist_frame, true, true);
    contents->pack_start(effectcontrol_frame, false, false);

    // connect callback functions to buttons
    button_apply.signal_clicked().connect(sigc::mem_fun(*this, &LivePathEffectEditor::onApply));
    button_remove.signal_clicked().connect(sigc::mem_fun(*this, &LivePathEffectEditor::onRemove));

    button_up.signal_clicked().connect(sigc::mem_fun(*this, &LivePathEffectEditor::onUp));
    button_down.signal_clicked().connect(sigc::mem_fun(*this, &LivePathEffectEditor::onDown));

    show_all_children();

    //button_remove.hide();
}

LivePathEffectEditor::~LivePathEffectEditor()
{
    if (effectwidget) {
        effectcontrol_vbox.remove(*effectwidget);
        delete effectwidget;
        effectwidget = NULL;
    }

    if (current_desktop) {
        selection_changed_connection.disconnect();
        selection_modified_connection.disconnect();
    }
}

void
LivePathEffectEditor::showParams(LivePathEffect::Effect& effect)
{
    if (effectwidget) {
        effectcontrol_vbox.remove(*effectwidget);
        delete effectwidget;
        effectwidget = NULL;
    }

    explain_label.set_markup("<b>" + effect.getName() + "</b>");
    effectwidget = effect.newWidget(&tooltips);
    if (effectwidget) {
        effectcontrol_vbox.pack_start(*effectwidget, true, true);
    }
    button_remove.show();

    effectcontrol_vbox.show_all_children();
    // fixme: add resizing of dialog
}

void
LivePathEffectEditor::selectInList(LivePathEffect::Effect* effect)
{
    Gtk::TreeNodeChildren chi = effectlist_view.get_model()->children();
    for (Gtk::TreeIter ci = chi.begin() ; ci != chi.end(); ci++) {
        if (ci->get_value(columns.lperef)->lpeobject->get_lpe() == effect)
            effectlist_view.get_selection()->select(ci);
    }
}


void
LivePathEffectEditor::showText(Glib::ustring const &str)
{
    if (effectwidget) {
        effectcontrol_vbox.remove(*effectwidget);
        delete effectwidget;
        effectwidget = NULL;
    }

    explain_label.set_label(str);
    //button_remove.hide();

    // fixme: do resizing of dialog ?
}

void
LivePathEffectEditor::set_sensitize_all(bool sensitive)
{
    combo_effecttype.set_sensitive(sensitive);
    button_apply.set_sensitive(sensitive);
    button_remove.set_sensitive(sensitive);
    effectlist_view.set_sensitive(sensitive);
    button_up.set_sensitive(sensitive);
    button_down.set_sensitive(sensitive);
}


void
LivePathEffectEditor::onSelectionChanged(Inkscape::Selection *sel)
{
    if (lpe_list_locked) {
        // this was triggered by selecting a row in the list, so skip reloading
        lpe_list_locked = false;
        return;
    } 

    effectlist_store->clear();
    current_lpeitem = NULL;

    if ( sel && !sel->isEmpty() ) {
        SPItem *item = sel->singleItem();
        if ( item ) {
            if ( SP_IS_LPE_ITEM(item) ) {
                SPLPEItem *lpeitem = SP_LPE_ITEM(item);

                effect_list_reload(lpeitem);

                current_lpeitem = lpeitem;

                set_sensitize_all(true);
                if ( sp_lpe_item_has_path_effect(lpeitem) ) {
                    Inkscape::LivePathEffect::Effect *lpe = sp_lpe_item_get_current_lpe(lpeitem);
                    if (lpe) {
                        showParams(*lpe);
                        lpe_list_locked = true; 
                        selectInList(lpe);
                    } else {
                        showText(_("Unknown effect is applied"));
                    }
                } else {
                    showText(_("No effect applied"));
                    button_remove.set_sensitive(false);
                }
            } else {
                showText(_("Item is not a path or shape"));
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

/*
 * First clears the effectlist_store, then appends all effects from the effectlist.
 */
void
LivePathEffectEditor::effect_list_reload(SPLPEItem *lpeitem)
{
    effectlist_store->clear();

    PathEffectList effectlist = sp_lpe_item_get_effect_list(lpeitem);
    PathEffectList::iterator it;
    for( it = effectlist.begin() ; it!=effectlist.end(); it++ )
    {
        if ((*it)->lpeobject->get_lpe()) {
            Gtk::TreeModel::Row row = *(effectlist_store->append());
            row[columns.col_name] = (*it)->lpeobject->get_lpe()->getName();
            row[columns.lperef] = *it;
            row[columns.col_visible] = (*it)->lpeobject->get_lpe()->isVisible();
        } else {
            Gtk::TreeModel::Row row = *(effectlist_store->append());
            row[columns.col_name] = _("Unknown effect");
            row[columns.lperef] = *it;
            row[columns.col_visible] = false;
        }
    }
}


void
LivePathEffectEditor::setDesktop(SPDesktop *desktop)
{
    Panel::setDesktop(desktop);

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
        if ( item && SP_IS_LPE_ITEM(item) ) {
            SPDocument *doc = current_desktop->doc();

            const Util::EnumData<LivePathEffect::EffectType>* data = combo_effecttype.get_active_data();
            if (!data) return;

            // If item is a SPRect, convert it to path first:
            if ( SP_IS_RECT(item) ) {
                sp_selected_path_to_curves(current_desktop, false);
                item = sel->singleItem(); // get new item
            }

            LivePathEffect::Effect::createAndApply(data->key.c_str(), doc, item);

            sp_document_done(doc, SP_VERB_DIALOG_LIVE_PATH_EFFECT,
                     _("Create and apply path effect"));

            onSelectionChanged(sel);
        }
    }
}

void
LivePathEffectEditor::onRemove()
{
    Inkscape::Selection *sel = _getSelection();
    if ( sel && !sel->isEmpty() ) {
        SPItem *item = sel->singleItem();
        if ( item && SP_IS_LPE_ITEM(item) ) {
            sp_lpe_item_remove_current_path_effect(SP_LPE_ITEM(item), false);

            sp_document_done ( sp_desktop_document (current_desktop), SP_VERB_DIALOG_LIVE_PATH_EFFECT,
                               _("Remove path effect") );

            effect_list_reload(SP_LPE_ITEM(item));
        }
    }
}

void LivePathEffectEditor::onUp()
{
    Inkscape::Selection *sel = _getSelection();
    if ( sel && !sel->isEmpty() ) {
        SPItem *item = sel->singleItem();
        if ( item && SP_IS_LPE_ITEM(item) ) {
            sp_lpe_item_up_current_path_effect(SP_LPE_ITEM(item));

            sp_document_done ( sp_desktop_document (current_desktop), SP_VERB_DIALOG_LIVE_PATH_EFFECT,
                               _("Move path effect up") );

            effect_list_reload(SP_LPE_ITEM(item));
        }
    }
}

void LivePathEffectEditor::onDown()
{
    Inkscape::Selection *sel = _getSelection();
    if ( sel && !sel->isEmpty() ) {
        SPItem *item = sel->singleItem();
        if ( item && SP_IS_LPE_ITEM(item) ) {
            sp_lpe_item_down_current_path_effect(SP_LPE_ITEM(item));

            sp_document_done ( sp_desktop_document (current_desktop), SP_VERB_DIALOG_LIVE_PATH_EFFECT,
                               _("Move path effect down") );

            effect_list_reload(SP_LPE_ITEM(item));
        }
    }
}

void LivePathEffectEditor::on_effect_selection_changed()
{
    Glib::RefPtr<Gtk::TreeSelection> sel = effectlist_view.get_selection();
    if (sel->count_selected_rows () == 0)
        return;

    Gtk::TreeModel::iterator it = sel->get_selected();
    LivePathEffect::LPEObjectReference * lperef = (*it)[columns.lperef];

    if (lperef && current_lpeitem) {
        if (lperef->lpeobject->get_lpe()) {
            lpe_list_locked = true; // prevent reload of the list which would lose selection
            sp_lpe_item_set_current_path_effect(current_lpeitem, lperef);
            showParams(*lperef->lpeobject->get_lpe());
        }
    }
}

void LivePathEffectEditor::on_visibility_toggled( Glib::ustring const& str )
{
    Gtk::TreeModel::Children::iterator iter = effectlist_view.get_model()->get_iter(str);
    Gtk::TreeModel::Row row = *iter;

    LivePathEffect::LPEObjectReference * lpeobjref = row[columns.lperef];

    if ( lpeobjref && lpeobjref->lpeobject->get_lpe() ) {
        bool newValue = !row[columns.col_visible];
        row[columns.col_visible] = newValue;
        /* FIXME: this explicit writing to SVG is wrong. The lpe_item should have a method to disable/enable an effect within its stack.
         * So one can call:  lpe_item->setActive(lpeobjref->lpeobject); */
        lpeobjref->lpeobject->get_lpe()->getRepr()->setAttribute("is_visible", newValue ? "true" : "false");
        sp_document_done( sp_desktop_document(current_desktop), SP_VERB_DIALOG_LIVE_PATH_EFFECT,
                          newValue ? _("Activate path effect") : _("Deactivate path effect"));
    }
}

} // namespace Dialog
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
