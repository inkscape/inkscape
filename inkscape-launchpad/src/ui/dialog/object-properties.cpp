/**
 * @file Object properties dialog.
 */
/* 
 * Inkscape, an Open Source vector graphics editor
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Copyright (C) 2012 Kris De Gussem <Kris.DeGussem@gmail.com>
 * c++ version based on former C-version (GPL v2+) with authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *   Abhishek Sharma
 */

#include "object-properties.h"
#include "widgets/sp-attribute-widget.h"

#include "document.h"
#include "document-undo.h"
#include "verbs.h"
#include "inkscape.h"
#include "selection.h"
#include "desktop.h"
#include "sp-item.h"
#include "sp-image.h"
#include "xml/repr.h"
#include <glibmm/i18n.h>

#if WITH_GTKMM_3_0
# include <gtkmm/grid.h>
#else
# include <gtkmm/table.h>
#endif


namespace Inkscape {
namespace UI {
namespace Dialog {

ObjectProperties::ObjectProperties()
    : UI::Widget::Panel ("", "/dialogs/object/", SP_VERB_DIALOG_ITEM)
    , _blocked (false)
    , _current_item(NULL)
    , _label_id(_("_ID:"), 1)
    , _label_label(_("_Label:"), 1)
    , _label_title(_("_Title:"), 1)
    , _label_image_rendering(_("_Image Rendering:"), 1)
    , _cb_hide(_("_Hide"), 1)
    , _cb_lock(_("L_ock"), 1)
    , _attr_table(Gtk::manage(new SPAttributeTable()))
    , _desktop(NULL)
{
    //initialize labels for the table at the bottom of the dialog
    _int_attrs.push_back("onclick");
    _int_attrs.push_back("onmouseover");
    _int_attrs.push_back("onmouseout");
    _int_attrs.push_back("onmousedown");
    _int_attrs.push_back("onmouseup");
    _int_attrs.push_back("onmousemove");
    _int_attrs.push_back("onfocusin");
    _int_attrs.push_back("onfocusout");
    _int_attrs.push_back("onload");

    _int_labels.push_back("onclick:");
    _int_labels.push_back("onmouseover:");
    _int_labels.push_back("onmouseout:");
    _int_labels.push_back("onmousedown:");
    _int_labels.push_back("onmouseup:");
    _int_labels.push_back("onmousemove:");
    _int_labels.push_back("onfocusin:");
    _int_labels.push_back("onfocusout:");
    _int_labels.push_back("onload:");

    _desktop_changed_connection = _desktop_tracker.connectDesktopChanged(
        sigc::mem_fun(*this, &ObjectProperties::_setTargetDesktop)
    );
    _desktop_tracker.connect(GTK_WIDGET(gobj()));

    _init();
}

ObjectProperties::~ObjectProperties()
{
    _subselection_changed_connection.disconnect();
    _selection_changed_connection.disconnect();
    _desktop_changed_connection.disconnect();
    _desktop_tracker.disconnect();
}

void ObjectProperties::_init()
{
    Gtk::Box *contents = _getContents();
    contents->set_spacing(0);

#if WITH_GTKMM_3_0
    Gtk::Grid *grid_top = Gtk::manage(new Gtk::Grid());
    grid_top->set_row_spacing(4);
    grid_top->set_column_spacing(0);
#else
    Gtk::Table *grid_top = Gtk::manage(new Gtk::Table(4, 4));
    grid_top->set_row_spacings(4);
    grid_top->set_col_spacings(0);
#endif

    grid_top->set_border_width(4);

    contents->pack_start(*grid_top, false, false, 0);


    /* Create the label for the object id */
    _label_id.set_label(_label_id.get_label() + " ");
    _label_id.set_alignment(1, 0.5);

#if WITH_GTKMM_3_0
    _label_id.set_valign(Gtk::ALIGN_CENTER);
    grid_top->attach(_label_id, 0, 0, 1, 1);
#else
    grid_top->attach(_label_id, 0, 1, 0, 1,
                     Gtk::SHRINK | Gtk::FILL,
                     Gtk::AttachOptions(), 0, 0 );
#endif


    /* Create the entry box for the object id */
    _entry_id.set_tooltip_text(_("The id= attribute (only letters, digits, and the characters .-_: allowed)"));
    _entry_id.set_max_length(64);

#if WITH_GTKMM_3_0
    _entry_id.set_valign(Gtk::ALIGN_CENTER);
    grid_top->attach(_entry_id, 1, 0, 1, 1);
#else
    grid_top->attach(_entry_id, 1, 2, 0, 1,
                     Gtk::EXPAND | Gtk::FILL,
                     Gtk::AttachOptions(), 0, 0 );
#endif

    _label_id.set_mnemonic_widget(_entry_id);

    // pressing enter in the id field is the same as clicking Set:
    _entry_id.signal_activate().connect(sigc::mem_fun(this, &ObjectProperties::_labelChanged));
    // focus is in the id field initially:
    _entry_id.grab_focus();


    /* Create the label for the object label */
    _label_label.set_label(_label_label.get_label() + " ");
    _label_label.set_alignment(1, 0.5);

#if WITH_GTKMM_3_0
    _label_label.set_valign(Gtk::ALIGN_CENTER);
    grid_top->attach(_label_label, 0, 1, 1, 1);
#else
    grid_top->attach(_label_label, 0, 1, 1, 2,
                     Gtk::SHRINK | Gtk::FILL,
                     Gtk::AttachOptions(), 0, 0 );
#endif


    /* Create the entry box for the object label */
    _entry_label.set_tooltip_text(_("A freeform label for the object"));
    _entry_label.set_max_length(256);

#if WITH_GTKMM_3_0
    _entry_label.set_hexpand();
    _entry_label.set_valign(Gtk::ALIGN_CENTER);
    grid_top->attach(_entry_label, 1, 1, 1, 1);
#else
    grid_top->attach(_entry_label, 1, 2, 1, 2,
                     Gtk::EXPAND | Gtk::FILL,
                     Gtk::AttachOptions(), 0, 0 );
#endif

    _label_label.set_mnemonic_widget(_entry_label);

    // pressing enter in the label field is the same as clicking Set:
    _entry_label.signal_activate().connect(sigc::mem_fun(this, &ObjectProperties::_labelChanged));


    /* Create the label for the object title */
    _label_title.set_label(_label_title.get_label() + " ");
    _label_title.set_alignment (1, 0.5);

#if WITH_GTKMM_3_0
    _label_title.set_valign(Gtk::ALIGN_CENTER);
    grid_top->attach(_label_title, 0, 2, 1, 1);
#else
    grid_top->attach(_label_title, 0, 1, 2, 3,
                     Gtk::SHRINK | Gtk::FILL,
                     Gtk::AttachOptions(), 0, 0 );
#endif

    /* Create the entry box for the object title */
    _entry_title.set_sensitive (FALSE);
    _entry_title.set_max_length (256);

#if WITH_GTKMM_3_0
    _entry_title.set_hexpand();
    _entry_title.set_valign(Gtk::ALIGN_CENTER);
    grid_top->attach(_entry_title, 1, 2, 1, 1);
#else
    grid_top->attach(_entry_title, 1, 2, 2, 3,
                     Gtk::EXPAND | Gtk::FILL,
                     Gtk::AttachOptions(), 0, 0 );
#endif

    _label_title.set_mnemonic_widget(_entry_title);
    // pressing enter in the label field is the same as clicking Set:
    _entry_title.signal_activate().connect(sigc::mem_fun(this, &ObjectProperties::_labelChanged));

    /* Create the frame for the object description */
    Gtk::Label *label_desc = Gtk::manage(new Gtk::Label(_("_Description:"), 1));
    UI::Widget::Frame *frame_desc = Gtk::manage(new UI::Widget::Frame("", FALSE));
    frame_desc->set_label_widget(*label_desc);
    frame_desc->set_padding (0,0,0,0);
    contents->pack_start(*frame_desc, true, true, 0);

    /* Create the text view box for the object description */
    _ft_description.set_border_width(4);
    _ft_description.set_sensitive(FALSE);
    frame_desc->add(_ft_description);
    _ft_description.set_shadow_type(Gtk::SHADOW_IN);

    _tv_description.set_wrap_mode(Gtk::WRAP_WORD);
    _tv_description.get_buffer()->set_text("");
    _ft_description.add(_tv_description);
    _tv_description.add_mnemonic_label(*label_desc);

    /* Image rendering */
    /* Create the label for the object ImageRendering */
    _label_image_rendering.set_label(_label_image_rendering.get_label() + " ");
    _label_image_rendering.set_alignment(1, 0.5);

#if WITH_GTKMM_3_0
    _label_image_rendering.set_valign(Gtk::ALIGN_CENTER);
    grid_top->attach(_label_image_rendering, 0, 3, 1, 1);
#else
    grid_top->attach(_label_image_rendering, 0, 1, 3, 4,
                     Gtk::SHRINK | Gtk::FILL,
                     Gtk::AttachOptions(), 0, 0 );
#endif

    /* Create the combo box text for the 'image-rendering' property  */
    _combo_image_rendering.append( "auto" );
    _combo_image_rendering.append( "optimizeQuality" );
    _combo_image_rendering.append( "optimizeSpeed" );
    _combo_image_rendering.set_tooltip_text(_("The 'image-rendering' property can influence how a bitmap is up-scaled:\n\t'auto' no preference;\n\t'optimizeQuality' smooth;\n\t'optimizeSpeed' blocky.\nNote that this behaviour is not defined in the SVG 1.1 specification and not all browsers follow this interpretation."));

#if WITH_GTKMM_3_0
    _combo_image_rendering.set_valign(Gtk::ALIGN_CENTER);
    grid_top->attach(_combo_image_rendering, 1, 3, 1, 1);
#else
    grid_top->attach(_combo_image_rendering, 1, 2, 3, 4,
                     Gtk::EXPAND | Gtk::FILL,
                     Gtk::AttachOptions(), 0, 0 );
#endif

    _label_image_rendering.set_mnemonic_widget(_combo_image_rendering);

    _combo_image_rendering.signal_changed().connect(
        sigc::mem_fun(this, &ObjectProperties::_imageRenderingChanged)
    );

    /* Check boxes */
    Gtk::HBox *hb_checkboxes = Gtk::manage(new Gtk::HBox());
    contents->pack_start(*hb_checkboxes, FALSE, FALSE, 0);

#if WITH_GTKMM_3_0
    Gtk::Grid *grid_cb = Gtk::manage(new Gtk::Grid());
    grid_cb->set_row_homogeneous();
    grid_cb->set_column_homogeneous(true);
#else
    Gtk::Table *grid_cb = Gtk::manage(new Gtk::Table(1, 2, true));
#endif

    grid_cb->set_border_width(4);
    hb_checkboxes->pack_start(*grid_cb, true, true, 0);

    /* Hide */
    _cb_hide.set_tooltip_text (_("Check to make the object invisible"));

#if WITH_GTKMM_3_0
    _cb_hide.set_hexpand();
    _cb_hide.set_valign(Gtk::ALIGN_CENTER);
    grid_cb->attach(_cb_hide, 0, 0, 1, 1);
#else
    grid_cb->attach(_cb_hide, 0, 1, 0, 1,
                       Gtk::EXPAND | Gtk::FILL,
                       Gtk::AttachOptions(), 0, 0 );
#endif

    _cb_hide.signal_toggled().connect(sigc::mem_fun(this, &ObjectProperties::_hiddenToggled));

    /* Lock */
    // TRANSLATORS: "Lock" is a verb here
    _cb_lock.set_tooltip_text(_("Check to make the object insensitive (not selectable by mouse)"));

#if WITH_GTKMM_3_0
    _cb_lock.set_hexpand();
    _cb_lock.set_valign(Gtk::ALIGN_CENTER);
    grid_cb->attach(_cb_lock, 1, 0, 1, 1);
#else
    grid_cb->attach(_cb_lock, 1, 2, 0, 1,
                    Gtk::EXPAND | Gtk::FILL,
                    Gtk::AttachOptions(), 0, 0 );
#endif

    _cb_lock.signal_toggled().connect(sigc::mem_fun(this, &ObjectProperties::_sensitivityToggled));


    /* Button for setting the object's id, label, title and description. */
    Gtk::Button *btn_set = Gtk::manage(new Gtk::Button(_("_Set"), 1));
#if WITH_GTKMM_3_0
    btn_set->set_hexpand();
    btn_set->set_valign(Gtk::ALIGN_CENTER);
    grid_cb->attach(*btn_set, 2, 0, 1, 1);
#else
    grid_cb->attach(*btn_set, 2, 3, 0, 1,
                    Gtk::EXPAND | Gtk::FILL,
                    Gtk::AttachOptions(), 0, 0 );
#endif

    btn_set->signal_clicked().connect(sigc::mem_fun(this, &ObjectProperties::_labelChanged));

    /* Create the frame for interactivity options */
    Gtk::Label *label_interactivity = Gtk::manage(new Gtk::Label(_("_Interactivity"), 1));
    _exp_interactivity.set_label_widget(*label_interactivity);
    contents->pack_start(_exp_interactivity, FALSE, FALSE, 0);

    show_all();
    update();
}

void ObjectProperties::update()
{
    if (_blocked || !_desktop) {
        return;
    }
    if (SP_ACTIVE_DESKTOP != _desktop) {
        return;
    }

    Inkscape::Selection *selection = SP_ACTIVE_DESKTOP->getSelection();
    Gtk::Box *contents = _getContents();

    if (!selection->singleItem()) {
        contents->set_sensitive (false);
        _current_item = NULL;
        //no selection anymore or multiple objects selected, means that we need
        //to close the connections to the previously selected object
        _attr_table->clear();
        return;
    } else {
        contents->set_sensitive (true);
    }
    
    SPItem *item = selection->singleItem();
    if (_current_item == item)
    {
        //otherwise we would end up wasting resources through the modify selection
        //callback when moving an object (endlessly setting the labels and recreating _attr_table)
        return;
    }
    _blocked = true;
    
    _cb_lock.set_active(item->isLocked());           /* Sensitive */
    _cb_hide.set_active(item->isExplicitlyHidden()); /* Hidden */
    
    if (item->cloned) {
        /* ID */
        _entry_id.set_text("");
        _entry_id.set_sensitive(FALSE);
        _label_id.set_text(_("Ref"));

        /* Label */
        _entry_label.set_text("");
        _entry_label.set_sensitive(FALSE);
        _label_label.set_text(_("Ref"));

    } else {
        SPObject *obj = static_cast<SPObject*>(item);

        /* ID */
        _entry_id.set_text(obj->getId());
        _entry_id.set_sensitive(TRUE);
        _label_id.set_markup_with_mnemonic(_("_ID:") + Glib::ustring(" "));

        /* Label */
        _entry_label.set_text(obj->defaultLabel());
        _entry_label.set_sensitive(TRUE);

        /* Title */
        gchar *title = obj->title();
        if (title) {
            _entry_title.set_text(title);
            g_free(title);
        }
        else {
            _entry_title.set_text("");
        }
        _entry_title.set_sensitive(TRUE);

        /* Image Rendering */
        if (SP_IS_IMAGE(item)) {
            _combo_image_rendering.show();
            _label_image_rendering.show();
            char const *str = obj->getStyleProperty( "image-rendering", "auto" );
            if (strcmp( str, "auto" ) == 0) {
                _combo_image_rendering.set_active(0);
            } else if (strcmp(str, "optimizeQuality") == 0) {
                _combo_image_rendering.set_active(1);
            } else {
                _combo_image_rendering.set_active(2);
            }
        } else {
            _combo_image_rendering.hide();
            _combo_image_rendering.unset_active();
            _label_image_rendering.hide();
        }

        /* Description */
        gchar *desc = obj->desc();
        if (desc) {
            _tv_description.get_buffer()->set_text(desc);
            g_free(desc);
        } else {
            _tv_description.get_buffer()->set_text("");
        }
        _ft_description.set_sensitive(TRUE);
        
        if (_current_item == NULL) {
            _attr_table->set_object(obj, _int_labels, _int_attrs, (GtkWidget*) _exp_interactivity.gobj());
        } else {
            _attr_table->change_object(obj);
        }
        _attr_table->show_all();
    }
    _current_item = item;
    _blocked = false;
}

void ObjectProperties::_labelChanged()
{
    if (_blocked) {
        return;
    }
    
    SPItem *item = SP_ACTIVE_DESKTOP->getSelection()->singleItem();
    g_return_if_fail (item != NULL);

    _blocked = true;

    /* Retrieve the label widget for the object's id */
    gchar *id = g_strdup(_entry_id.get_text().c_str());
    g_strcanon(id, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.:", '_');
    if (strcmp(id, item->getId()) == 0) {
        _label_id.set_markup_with_mnemonic(_("_ID:") + Glib::ustring(" "));
    } else if (!*id || !isalnum (*id)) {
        _label_id.set_text(_("Id invalid! "));
    } else if (SP_ACTIVE_DOCUMENT->getObjectById(id) != NULL) {
        _label_id.set_text(_("Id exists! "));
    } else {
        SPException ex;
        _label_id.set_markup_with_mnemonic(_("_ID:") + Glib::ustring(" "));
        SP_EXCEPTION_INIT(&ex);
        item->setAttribute("id", id, &ex);
        DocumentUndo::done(SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM, _("Set object ID"));
    }
    g_free(id);

    /* Retrieve the label widget for the object's label */
    Glib::ustring label = _entry_label.get_text();

    /* Give feedback on success of setting the drawing object's label
     * using the widget's label text
     */
    SPObject *obj = static_cast<SPObject*>(item);
    if (label.compare(obj->defaultLabel())) {
        obj->setLabel(label.c_str());
        DocumentUndo::done(SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM,
                _("Set object label"));
    }

    /* Retrieve the title */
    if (obj->setTitle(_entry_title.get_text().c_str())) {
        DocumentUndo::done(SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM,
                _("Set object title"));
    }

    /* Retrieve the description */
    Gtk::TextBuffer::iterator start, end;
    _tv_description.get_buffer()->get_bounds(start, end);
    Glib::ustring desc = _tv_description.get_buffer()->get_text(start, end, TRUE);
    if (obj->setDesc(desc.c_str())) {
        DocumentUndo::done(SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM,
                _("Set object description"));
    }
    
    _blocked = false;
}

void ObjectProperties::_imageRenderingChanged()
{
    if (_blocked) {
        return;
    }
    
    SPItem *item = SP_ACTIVE_DESKTOP->getSelection()->singleItem();
    g_return_if_fail (item != NULL);

    _blocked = true;

    Glib::ustring scale = _combo_image_rendering.get_active_text();

    // We should unset if the parent computed value is auto and the desired value is auto.
    SPCSSAttr *css = sp_repr_css_attr_new();
    sp_repr_css_set_property(css, "image-rendering", scale.c_str());
    Inkscape::XML::Node *image_node = item->getRepr();
    if (image_node) {
        sp_repr_css_change(image_node, css, "style");
        DocumentUndo::done(SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM,
                _("Set image rendering option"));
    }
    sp_repr_css_attr_unref(css);
        
    _blocked = false;
}

void ObjectProperties::_sensitivityToggled()
{
    if (_blocked) {
        return;
    }

    SPItem *item = SP_ACTIVE_DESKTOP->getSelection()->singleItem();
    g_return_if_fail(item != NULL);

    _blocked = true;
    item->setLocked(_cb_lock.get_active());
    DocumentUndo::done(SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM,
                       _cb_lock.get_active() ? _("Lock object") : _("Unlock object"));
    _blocked = false;
}

void ObjectProperties::_hiddenToggled()
{
    if (_blocked) {
        return;
    }

    SPItem *item = SP_ACTIVE_DESKTOP->getSelection()->singleItem();
    g_return_if_fail(item != NULL);

    _blocked = true;
    item->setExplicitlyHidden(_cb_hide.get_active());
    DocumentUndo::done(SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM,
               _cb_hide.get_active() ? _("Hide object") : _("Unhide object"));
    _blocked = false;
}

void ObjectProperties::_setDesktop(SPDesktop *desktop)
{
    Panel::setDesktop(desktop);
    _desktop_tracker.setBase(desktop);
}

void ObjectProperties::_setTargetDesktop(SPDesktop *desktop)
{
    if (this->_desktop != desktop) {
        if (this->_desktop) {
            _subselection_changed_connection.disconnect();
            _selection_changed_connection.disconnect();
        }
        this->_desktop = desktop;
        if (desktop && desktop->selection) {
            _selection_changed_connection = desktop->selection->connectChanged(
                sigc::hide(sigc::mem_fun(*this, &ObjectProperties::update))
            );
            _subselection_changed_connection = desktop->connectToolSubselectionChanged(
                sigc::hide(sigc::mem_fun(*this, &ObjectProperties::update))
            );
        }
        update();
    }
}

}
}
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
