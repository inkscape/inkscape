/**
 * @file
 * Live Path Effect editing dialog - implementation.
 */
/* Authors:
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *   Steren Giannini <steren.giannini@gmail.com>
 *   Bastien Bouclet <bgkweb@gmail.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2007 Authors
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "livepatheffect-editor.h"
#include <glibmm/i18n.h>
#include <gtkmm/stock.h>
#include <gtkmm/toolbar.h>
#include <vector>

#include "desktop.h"

#include "document.h"
#include "document-undo.h"
#include "gtkmm/widget.h"
#include "inkscape.h"
#include "live_effects/effect.h"
#include "live_effects/lpeobject.h"
#include "live_effects/lpeobject-reference.h"
#include "path-chemistry.h"
#include "selection-chemistry.h"
#include "selection.h"
#include "sp-item-group.h"
#include "sp-lpe-item.h"
#include "sp-path.h"
#include "sp-rect.h"
#include "sp-use.h"
#include "sp-text.h"
#include "sp-shape.h"
#include "ui/icon-names.h"
#include "ui/widget/imagetoggler.h"
#include "verbs.h"
#include "xml/node.h"
#include "livepatheffect-add.h"

namespace Inkscape {
namespace UI {
namespace Dialog {


/*####################
 * Callback functions
 */
void lpeeditor_selection_changed (Inkscape::Selection * selection, gpointer data)
{
    LivePathEffectEditor *lpeeditor = static_cast<LivePathEffectEditor *>(data);
    lpeeditor->lpe_list_locked = false;
    lpeeditor->onSelectionChanged(selection);
}

static void lpeeditor_selection_modified (Inkscape::Selection * selection, guint /*flags*/, gpointer data)
{
    LivePathEffectEditor *lpeeditor = static_cast<LivePathEffectEditor *>(data);
    lpeeditor->onSelectionChanged(selection);
}


/*
 * LivePathEffectEditor
 *
 * TRANSLATORS: this dialog is accessible via menu Path - Path Effect Editor...
 *
 */

LivePathEffectEditor::LivePathEffectEditor()
    : UI::Widget::Panel("", "/dialogs/livepatheffect", SP_VERB_DIALOG_LIVE_PATH_EFFECT),
      deskTrack(),
      lpe_list_locked(false),
      effectwidget(NULL),
      status_label("", Gtk::ALIGN_CENTER),
      effectcontrol_frame(""),
      button_add(),
      button_remove(),
      button_up(),
      button_down(),
      current_desktop(NULL),
      current_lpeitem(NULL)
{
    Gtk::Box *contents = _getContents();
    contents->set_spacing(4);

    //Add the TreeView, inside a ScrolledWindow, with the button underneath:
    scrolled_window.add(effectlist_view);
    scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolled_window.set_shadow_type(Gtk::SHADOW_IN);
    scrolled_window.set_size_request(210, 70);

    effectapplication_hbox.set_spacing(4);
    effectcontrol_vbox.set_spacing(4);

    effectlist_vbox.pack_start(scrolled_window, Gtk::PACK_EXPAND_WIDGET);
    effectlist_vbox.pack_end(toolbar_hbox, Gtk::PACK_SHRINK);
    effectcontrol_frame.add(effectcontrol_vbox);

    button_add.set_tooltip_text(_("Add path effect"));
#if WITH_GTKMM_3_10
    button_add.set_image_from_icon_name(INKSCAPE_ICON("list-add"), Gtk::ICON_SIZE_SMALL_TOOLBAR);
#else
    Gtk::Image *image_add = Gtk::manage(new Gtk::Image());
    image_add->set_from_icon_name(INKSCAPE_ICON("list-add"), Gtk::ICON_SIZE_SMALL_TOOLBAR);
    button_add.set_image(*image_add);
#endif
    button_add.set_relief(Gtk::RELIEF_NONE);

    button_remove.set_tooltip_text(_("Delete current path effect"));
#if WITH_GTKMM_3_10
    button_remove.set_image_from_icon_name(INKSCAPE_ICON("list-remove"), Gtk::ICON_SIZE_SMALL_TOOLBAR);
#else
    Gtk::Image *image_remove = Gtk::manage(new Gtk::Image());
    image_remove->set_from_icon_name(INKSCAPE_ICON("list-remove"), Gtk::ICON_SIZE_SMALL_TOOLBAR);
    button_remove.set_image(*image_remove);
#endif
    button_remove.set_relief(Gtk::RELIEF_NONE);

    button_up.set_tooltip_text(_("Raise the current path effect"));
#if WITH_GTKMM_3_10
    button_up.set_image_from_icon_name(INKSCAPE_ICON("go-up"), Gtk::ICON_SIZE_SMALL_TOOLBAR);
#else
    Gtk::Image *image_up = Gtk::manage(new Gtk::Image());
    image_up->set_from_icon_name(INKSCAPE_ICON("go-up"), Gtk::ICON_SIZE_SMALL_TOOLBAR);
    button_up.set_image(*image_up);
#endif
    button_up.set_relief(Gtk::RELIEF_NONE);

    button_down.set_tooltip_text(_("Lower the current path effect"));
#if WITH_GTKMM_3_10
    button_down.set_image_from_icon_name(INKSCAPE_ICON("go-down"), Gtk::ICON_SIZE_SMALL_TOOLBAR);
#else
    Gtk::Image *image_down = Gtk::manage(new Gtk::Image());
    image_down->set_from_icon_name(INKSCAPE_ICON("go-down"), Gtk::ICON_SIZE_SMALL_TOOLBAR);
    button_down.set_image(*image_down);
#endif
    button_down.set_relief(Gtk::RELIEF_NONE);

    // Add toolbar items to toolbar
    toolbar_hbox.set_layout (Gtk::BUTTONBOX_END);

#if !WITH_GTKMM_3_0
    // TODO: This has been removed from Gtkmm 3.0. Check that
    //       everything still looks OK!
    toolbar_hbox.set_child_min_width( 16 );
#endif

    toolbar_hbox.add( button_add );
    toolbar_hbox.set_child_secondary( button_add , true);
    toolbar_hbox.add( button_remove );
    toolbar_hbox.set_child_secondary( button_remove , true);
    toolbar_hbox.add( button_up );
    toolbar_hbox.add( button_down );

    //Create the Tree model:
    effectlist_store = Gtk::ListStore::create(columns);
    effectlist_view.set_model(effectlist_store);
    effectlist_view.set_headers_visible(false);

    // Handle tree selections
    effectlist_selection = effectlist_view.get_selection();
    effectlist_selection->signal_changed().connect( sigc::mem_fun(*this, &LivePathEffectEditor::on_effect_selection_changed) );

    //Add the visibility icon column:
    Inkscape::UI::Widget::ImageToggler *eyeRenderer = Gtk::manage( new Inkscape::UI::Widget::ImageToggler(
        INKSCAPE_ICON("object-visible"), INKSCAPE_ICON("object-hidden")) );
    int visibleColNum = effectlist_view.append_column("is_visible", *eyeRenderer) - 1;
    eyeRenderer->signal_toggled().connect( sigc::mem_fun(*this, &LivePathEffectEditor::on_visibility_toggled) );
    eyeRenderer->property_activatable() = true;
    Gtk::TreeViewColumn* col = effectlist_view.get_column(visibleColNum);
    if ( col ) {
        col->add_attribute( eyeRenderer->property_active(), columns.col_visible );
    }

    //Add the effect name column:
    effectlist_view.append_column("Effect", columns.col_name);

    contents->pack_start(effectlist_vbox, true, true);
    contents->pack_start(status_label, false, false);
    contents->pack_start(effectcontrol_frame, false, false);

    effectcontrol_frame.hide();

    // connect callback functions to buttons
    button_add.signal_clicked().connect(sigc::mem_fun(*this, &LivePathEffectEditor::onAdd));
    button_remove.signal_clicked().connect(sigc::mem_fun(*this, &LivePathEffectEditor::onRemove));
    button_up.signal_clicked().connect(sigc::mem_fun(*this, &LivePathEffectEditor::onUp));
    button_down.signal_clicked().connect(sigc::mem_fun(*this, &LivePathEffectEditor::onDown));

    desktopChangeConn = deskTrack.connectDesktopChanged( sigc::mem_fun(*this, &LivePathEffectEditor::setDesktop) );
    deskTrack.connect(GTK_WIDGET(gobj()));

    show_all_children();
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

    effectcontrol_frame.set_label(effect.getName());

    effectwidget = effect.newWidget();
    if (effectwidget) {
        effectcontrol_vbox.pack_start(*effectwidget, true, true);
    }
    button_remove.show();

    status_label.hide();
    effectcontrol_frame.show();
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

    status_label.show();
    status_label.set_label(str);

    effectcontrol_frame.hide();

    // fixme: do resizing of dialog ?
}

void
LivePathEffectEditor::set_sensitize_all(bool sensitive)
{
    //combo_effecttype.set_sensitive(sensitive);
    button_add.set_sensitive(sensitive);
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
            SPLPEItem *lpeitem = dynamic_cast<SPLPEItem *>(item);
            if ( lpeitem ) {
                effect_list_reload(lpeitem);

                current_lpeitem = lpeitem;

                set_sensitize_all(true);
                if ( lpeitem->hasPathEffect() ) {
                    Inkscape::LivePathEffect::Effect *lpe = lpeitem->getCurrentLPE();
                    if (lpe) {
                        showParams(*lpe);
                        lpe_list_locked = true;
                        selectInList(lpe);
                    } else {
                        showText(_("Unknown effect is applied"));
                    }
                } else {
                    showText(_("Click button to add an effect"));
                    button_remove.set_sensitive(false);
                    button_up.set_sensitive(false);
                    button_down.set_sensitive(false);
                }
            } else {
                SPUse *use = dynamic_cast<SPUse *>(item);
                if ( use ) {
                    // test whether linked object is supported by the CLONE_ORIGINAL LPE
                    SPItem *orig = use->get_original();
                    if ( dynamic_cast<SPShape *>(orig) ||
                         dynamic_cast<SPText *>(orig) )
                    {
                        // Note that an SP_USE cannot have an LPE applied, so we only need to worry about the "add effect" case.
                        set_sensitize_all(true);
                        showText(_("Click add button to convert clone"));
                        button_remove.set_sensitive(false);
                        button_up.set_sensitive(false);
                        button_down.set_sensitive(false);
                    } else {
                        showText(_("Select a path or shape"));
                        set_sensitize_all(false);
                    }
                } else {
                    showText(_("Select a path or shape"));
                    set_sensitize_all(false);
                }
            }
        } else {
            showText(_("Only one item can be selected"));
            set_sensitize_all(false);
        }
    } else {
        showText(_("Select a path or shape"));
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

    PathEffectList effectlist = lpeitem->getEffectList();
    PathEffectList::iterator it;
    for( it = effectlist.begin() ; it!=effectlist.end(); ++it)
    {
        if ( !(*it)->lpeobject ) {
            continue;
        }

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

    lpe_list_locked = false;
    current_desktop = desktop;
    if (desktop) {
        Inkscape::Selection *selection = desktop->getSelection();
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
LivePathEffectEditor::onAdd()
{
    Inkscape::Selection *sel = _getSelection();
    if ( sel && !sel->isEmpty() ) {
        SPItem *item = sel->singleItem();
        if (item) {
            if ( dynamic_cast<SPLPEItem *>(item) ) {
                // show effectlist dialog
                using Inkscape::UI::Dialog::LivePathEffectAdd;
                LivePathEffectAdd::show(current_desktop);
                if ( !LivePathEffectAdd::isApplied()) {
                    return;
                }

                SPDocument *doc = current_desktop->doc();

                const Util::EnumData<LivePathEffect::EffectType>* data = LivePathEffectAdd::getActiveData();
                if (!data) {
                    return;
                }

                // If item is a SPRect, convert it to path first:
                if ( dynamic_cast<SPRect *>(item) ) {
                    sp_selected_path_to_curves(sel, current_desktop, false);
                    item = sel->singleItem(); // get new item
                }

                LivePathEffect::Effect::createAndApply(data->key.c_str(), doc, item);

                DocumentUndo::done(doc, SP_VERB_DIALOG_LIVE_PATH_EFFECT,
                                   _("Create and apply path effect"));

                lpe_list_locked = false;
                onSelectionChanged(sel);
            } else {
                SPUse *use = dynamic_cast<SPUse *>(item);
                if ( use ) {
                    // item is a clone. do not show effectlist dialog.
                    // convert to path, apply CLONE_ORIGINAL LPE, link it to the cloned path

                    // test whether linked object is supported by the CLONE_ORIGINAL LPE
                    SPItem *orig = use->get_original();
                    if ( dynamic_cast<SPShape *>(orig) ||
                         dynamic_cast<SPText *>(orig) )
                    {
                        // select original
                        sel->set(orig);

                        // delete clone but remember its id and transform
                        gchar *id = g_strdup(item->getRepr()->attribute("id"));
                        gchar *transform = g_strdup(item->getRepr()->attribute("transform"));
                        item->deleteObject(false);
                        item = NULL;

                        // run sp_selection_clone_original_path_lpe 
                        sp_selection_clone_original_path_lpe(current_desktop);

                        SPItem *new_item = sel->singleItem();
                        // Check that the cloning was successful. We don't want to change the ID of the original referenced path!
                        if (new_item && (new_item != orig)) {
                            new_item->getRepr()->setAttribute("id", id);
                            new_item->getRepr()->setAttribute("transform", transform);
                        }
                        g_free(id);
                        g_free(transform);

                        /// \todo Add the LPE stack of the original path?

                        SPDocument *doc = current_desktop->doc();
                        DocumentUndo::done(doc, SP_VERB_DIALOG_LIVE_PATH_EFFECT,
                                           _("Create and apply Clone original path effect"));

                        lpe_list_locked = false;
                        onSelectionChanged(sel);
                    }
                }
            }
        }
    }
}

void
LivePathEffectEditor::onRemove()
{
    Inkscape::Selection *sel = _getSelection();
    if ( sel && !sel->isEmpty() ) {
        SPItem *item = sel->singleItem();
        SPLPEItem *lpeitem  = dynamic_cast<SPLPEItem *>(item);
        if ( lpeitem ) {
            lpeitem->removeCurrentPathEffect(false);

            DocumentUndo::done( current_desktop->getDocument(), SP_VERB_DIALOG_LIVE_PATH_EFFECT,
                                _("Remove path effect") );

            effect_list_reload(lpeitem);
        }
    }

}

void LivePathEffectEditor::onUp()
{
    Inkscape::Selection *sel = _getSelection();
    if ( sel && !sel->isEmpty() ) {
        SPItem *item = sel->singleItem();
        SPLPEItem *lpeitem = dynamic_cast<SPLPEItem *>(item);
        if ( lpeitem ) {
            lpeitem->upCurrentPathEffect();

            DocumentUndo::done( current_desktop->getDocument(), SP_VERB_DIALOG_LIVE_PATH_EFFECT,
                                _("Move path effect up") );

            effect_list_reload(lpeitem);
        }
    }
}

void LivePathEffectEditor::onDown()
{
    Inkscape::Selection *sel = _getSelection();
    if ( sel && !sel->isEmpty() ) {
        SPItem *item = sel->singleItem();
        SPLPEItem *lpeitem = dynamic_cast<SPLPEItem *>(item);
        if ( lpeitem ) {
            lpeitem->downCurrentPathEffect();

            DocumentUndo::done( current_desktop->getDocument(), SP_VERB_DIALOG_LIVE_PATH_EFFECT,
                                _("Move path effect down") );

            effect_list_reload(lpeitem);
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
            current_lpeitem->setCurrentPathEffect(lperef);
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
        DocumentUndo::done( current_desktop->getDocument(), SP_VERB_DIALOG_LIVE_PATH_EFFECT,
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
