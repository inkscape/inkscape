/**
 * \brief LivePathEffect dialog
 *
 * Authors:
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *   Steren Giannini <steren.giannini@gmail.com>
 *   Bastien Bouclet <bgkweb@gmail.com>
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
#include "sp-item-group.h"
#include "sp-path.h"
#include "sp-rect.h"
#include "sp-lpe-item.h"
#include "path-chemistry.h"
#include "live_effects/effect.h"
#include "live_effects/lpeobject.h"
#include "gtkmm/widget.h"
#include <vector>
#include "inkscape.h"
#include "desktop-handles.h"
#include "desktop.h"
#include "document.h"
#include "xml/node.h"
#include <gtkmm/stock.h>
#include <gtkmm/toolbar.h>

#include "live_effects/lpeobject-reference.h"

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
    : UI::Widget::Panel("", "dialogs.livepatheffect", SP_VERB_DIALOG_LIVE_PATH_EFFECT),
      combo_effecttype(Inkscape::LivePathEffect::LPETypeConverter),
      effectwidget(NULL),
      explain_label("", Gtk::ALIGN_CENTER),
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

    effectlist_view.set_rules_hint();
    effectlist_view.set_headers_clickable(true);
    effectlist_view.set_headers_visible(true);

    // Handle tree selections
    effectlist_selection = effectlist_view.get_selection();
    effectlist_selection->signal_changed().connect( sigc::mem_fun(*this, &LivePathEffectEditor::on_effect_selection_changed) );

    effectlist_view.set_headers_visible(false);
    //Add the TreeView's view columns:
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
LivePathEffectEditor::showParams(LivePathEffect::Effect* effect)
{
    if (effectwidget) {
        effectcontrol_vbox.remove(*effectwidget);
        delete effectwidget;
        effectwidget = NULL;
    }

    explain_label.set_markup("<b>" + effect->getName() + "</b>");
    effectwidget = effect->newWidget(&tooltips);
    if (effectwidget) {
        effectcontrol_vbox.pack_start(*effectwidget, true, true);
    }
    button_remove.show();

    effectcontrol_vbox.show_all_children();
    // fixme: add resizing of dialog
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
    effectlist_store->clear();
    current_lpeitem = NULL;

    if ( sel && !sel->isEmpty() ) {
        SPItem *item = sel->singleItem();
        if ( item ) {
            if ( SP_IS_LPE_ITEM(item) ) {
                SPLPEItem *lpeitem = SP_LPE_ITEM(item);

                effect_list_update(lpeitem);

                current_lpeitem = lpeitem;
                
                set_sensitize_all(true);
                if ( sp_lpe_item_has_path_effect(lpeitem) ) {
                    Inkscape::LivePathEffect::Effect *lpe = sp_lpe_item_get_current_lpe(lpeitem);
                    if (lpe) {
                        showParams(lpe);
                    } else {
                        showText(_("Unknown effect is applied"));
                    }
                } else {
                    showText(_("No effect applied"));
                    button_remove.set_sensitive(false);
                }
            }
              else
            {
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

void
LivePathEffectEditor::effect_list_update(SPLPEItem *lpeitem)
{
    effectlist_store->clear();
    
    PathEffectList effectlist = sp_lpe_item_get_effect_list(lpeitem);
    PathEffectList::iterator it;
    for( it = effectlist.begin() ; it!=effectlist.end(); it++ )
    {
         Gtk::TreeModel::Row row = *(effectlist_store->append());
         row[columns.col_name] = (*it)->lpeobject->lpe->getName();
         row[columns.lperef] = *it;
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
                sp_selected_path_to_curves(false);
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

            effect_list_update(SP_LPE_ITEM(item));
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

            effect_list_update(SP_LPE_ITEM(item));
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

            effect_list_update(SP_LPE_ITEM(item));
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
        sp_lpe_item_set_current_path_effect(current_lpeitem, lperef);
        showParams(lperef->lpeobject->lpe);
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
