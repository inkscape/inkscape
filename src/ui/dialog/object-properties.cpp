/**
 * @file
 * Object properties dialog.
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *   Abhishek Sharma
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 1999-2011 Authors
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "../../desktop-handles.h"
#include "../../document.h"
#include "object-properties.h"


namespace Inkscape {
namespace UI {
namespace Dialog {

void on_selection_changed(Inkscape::Application */*inkscape*/, Inkscape::Selection */*selection*/, ObjectProperties *dial);

/**
 * Create a new static instance of the object properties dialog.
 */
ObjectProperties::ObjectProperties (void) :
    UI::Widget::Panel ("", "/dialogs/object/", SP_VERB_DIALOG_ITEM),
    blocked (false),
    TopTable (3, 4, false),
    LabelID(_("_ID:"), 1),
    LabelLabel(_("_Label:"), 1),
    LabelTitle(_("_Title:"),1),
    LabelDescription(_("_Description"),1),
    HBoxCheck(FALSE, 0),
    CheckTable(1, 2, TRUE),
    CBHide(_("_Hide"), 1),
    CBLock(_("L_ock"), 1),
    BSet (_("_Set"), 1),
    LabelInteractivity(_("_Interactivity"), 1),
    attrTable(),
    CurrentItem(0)
{
    //initialize labels for the table at the bottom of the dialog
    int_labels.push_back("onclick");
    int_labels.push_back("onmouseover");
    int_labels.push_back("onmouseout");
    int_labels.push_back("onmousedown");
    int_labels.push_back("onmouseup");
    int_labels.push_back("onmousemove");
    int_labels.push_back("onfocusin");
    int_labels.push_back("onfocusout");
    int_labels.push_back("onfocusout");
    int_labels.push_back("onload");
    
    MakeWidget();
}

ObjectProperties::~ObjectProperties (void)
{
}

void ObjectProperties::MakeWidget(void)
{
    // g_signal_connect (G_OBJECT (INKSCAPE), "modify_selection", G_CALLBACK (sp_item_widget_modify_selection), wd.win);
    // g_signal_connect (G_OBJECT (INKSCAPE), "set_selection", G_CALLBACK (sp_item_widget_change_selection), wd.win);
    g_signal_connect (G_OBJECT (INKSCAPE), "change_selection", G_CALLBACK (on_selection_changed), this);
    
    Gtk::Box *contents = _getContents();
    contents->set_spacing(0);
    
    TopTable.set_border_width(4);
    TopTable.set_row_spacings(4);
    TopTable.set_col_spacings(4);
    contents->pack_start (TopTable, true, true, 0);

    /* Create the label for the object id */
    LabelID.set_alignment (1, 0.5);
    TopTable.attach (LabelID, 0, 1, 0, 1,
                       Gtk::SHRINK | Gtk::FILL,
                       Gtk::AttachOptions(), 0, 0 );

    /* Create the entry box for the object id */
    EntryID.set_tooltip_text (_("The id= attribute (only letters, digits, and the characters .-_: allowed)"));
    EntryID.set_max_length (64);
    TopTable.attach (EntryID, 1, 2, 0, 1,
                       Gtk::EXPAND | Gtk::FILL,
                       Gtk::AttachOptions(), 0, 0 );
    LabelID.set_mnemonic_widget (EntryID);

    // pressing enter in the id field is the same as clicking Set:
    EntryID.signal_activate().connect(sigc::mem_fun(this, &ObjectProperties::label_changed));
    // focus is in the id field initially:
    EntryID.grab_focus();

    /* Create the label for the object label */
    LabelLabel.set_alignment (1, 0.5);
    TopTable.attach (LabelLabel, 0, 1, 1, 2,
                       Gtk::SHRINK | Gtk::FILL,
                       Gtk::AttachOptions(), 0, 0 );

    /* Create the entry box for the object label */
    EntryLabel.set_tooltip_text (_("A freeform label for the object"));
    EntryLabel.set_max_length (256);
    TopTable.attach (EntryLabel, 1, 2, 1, 2,
                       Gtk::EXPAND | Gtk::FILL,
                       Gtk::AttachOptions(), 0, 0 );
    LabelLabel.set_mnemonic_widget (EntryLabel);

    // pressing enter in the label field is the same as clicking Set:
    EntryLabel.signal_activate().connect(sigc::mem_fun(this, &ObjectProperties::label_changed));

    /* Create the label for the object title */
    LabelTitle.set_alignment (1, 0.5);
    TopTable.attach (LabelTitle, 0, 1, 2, 3,
                       Gtk::SHRINK | Gtk::FILL,
                       Gtk::AttachOptions(), 0, 0 );

    /* Create the entry box for the object title */
    EntryTitle.set_sensitive (FALSE);
    EntryTitle.set_max_length (256);
    TopTable.attach (EntryTitle, 1, 3, 2, 3,
                       Gtk::EXPAND | Gtk::FILL,
                       Gtk::AttachOptions(), 0, 0 );
    LabelTitle.set_mnemonic_widget (EntryTitle);

    /* Create the frame for the object description */
    FrameDescription.set_label_widget (LabelDescription);
    TopTable.attach (FrameDescription, 0, 3, 3, 4,
                       Gtk::EXPAND | Gtk::FILL,
                       Gtk::EXPAND | Gtk::FILL, 0, 0 );

    /* Create the text view box for the object description */
    FrameTextDescription.set_border_width(4);
    FrameTextDescription.set_sensitive (FALSE);
    FrameDescription.add (FrameTextDescription);
    FrameTextDescription.set_shadow_type (Gtk::SHADOW_IN);

    TextViewDescription.set_wrap_mode(Gtk::WRAP_WORD);
    TextViewDescription.get_buffer()->set_text("");
    FrameTextDescription.add (TextViewDescription);
    TextViewDescription.add_mnemonic_label(LabelDescription);

    /* Check boxes */
    contents->pack_start (HBoxCheck, FALSE, FALSE, 0);
    CheckTable.set_border_width(0);
    HBoxCheck.pack_start (CheckTable, TRUE, TRUE, 10);

    /* Hide */
    CBHide.set_tooltip_text (_("Check to make the object invisible"));
    CheckTable.attach (CBHide, 0, 1, 0, 1,
                       Gtk::EXPAND | Gtk::FILL,
                       Gtk::AttachOptions(), 0, 0 );
    CBHide.signal_toggled().connect(sigc::mem_fun(this, &ObjectProperties::hidden_toggled));

    /* Lock */
    // TRANSLATORS: "Lock" is a verb here
    CBLock.set_tooltip_text (_("Check to make the object insensitive (not selectable by mouse)"));
    CheckTable.attach (CBLock, 1, 2, 0, 1,
                       Gtk::EXPAND | Gtk::FILL,
                       Gtk::AttachOptions(), 0, 0 );
    CBLock.signal_toggled().connect(sigc::mem_fun(this, &ObjectProperties::sensitivity_toggled));


    /* Button for setting the object's id, label, title and description. */
    HBoxCheck.pack_start (BSet, TRUE, TRUE, 10);
    BSet.signal_clicked().connect(sigc::mem_fun(this, &ObjectProperties::label_changed));

    /* Create the frame for interactivity options */
    EInteractivity.set_label_widget (LabelInteractivity);
    contents->pack_start (EInteractivity, FALSE, FALSE, 0);
    show_all ();
    widget_setup();
}

void ObjectProperties::widget_setup(void)
{
    if (blocked)
    {
        return;
    }

    Inkscape::Selection *selection = sp_desktop_selection (SP_ACTIVE_DESKTOP);
    Gtk::Box *contents = _getContents();

    if (!selection->singleItem()) {
        contents->set_sensitive (false);
        CurrentItem = NULL;
        //no selection anymore or multiple objects selected, means that we need
        //to close the connections to the previously selected object
        attrTable.clear();
        return;
    } else {
        contents->set_sensitive (true);
    }
    
    SPItem *item = selection->singleItem();
    if (CurrentItem == item)
    {
        //otherwise we would end up wasting resources through the modify selection
        //callback when moving an object (endlessly setting the labels and recreating attrTable)
        return;
    }
    blocked = true;
    
    CBLock.set_active (item->isLocked());           /* Sensitive */
    CBHide.set_active (item->isExplicitlyHidden()); /* Hidden */
    
    if (item->cloned) {
        /* ID */
        EntryID.set_text ("");
        EntryID.set_sensitive (FALSE);
        LabelID.set_text (_("Ref"));

        /* Label */
        EntryLabel.set_text ("");
        EntryLabel.set_sensitive (FALSE);
        LabelLabel.set_text (_("Ref"));

    } else {
        SPObject *obj = (SPObject*)item;

        /* ID */
        EntryID.set_text (obj->getId());
        EntryID.set_sensitive (TRUE);
        LabelID.set_markup_with_mnemonic (_("_ID:"));

        /* Label */
        EntryLabel.set_text(obj->defaultLabel());
        EntryLabel.set_sensitive (TRUE);

        /* Title */
        gchar *title = obj->title();
        if (title) {
            EntryTitle.set_text(title);
            g_free(title);
        }
        else {
            EntryTitle.set_text("");
        }
        EntryTitle.set_sensitive(TRUE);

        /* Description */
        gchar *desc = obj->desc();
        if (desc) {
            TextViewDescription.get_buffer()->set_text(desc);
            g_free(desc);
        } else {
            TextViewDescription.get_buffer()->set_text("");
        }
        FrameTextDescription.set_sensitive(TRUE);
        
        if (CurrentItem == NULL)
        {
            attrTable.set_object(obj, int_labels, int_labels, (GtkWidget*)EInteractivity.gobj());
        }
        else
        {
            attrTable.change_object(obj);
        }
        attrTable.show_all();
    }
    CurrentItem = item;
    blocked = false;
}

void ObjectProperties::label_changed(void)
{
    if (blocked)
    {
        return;
    }
    
    SPItem *item = sp_desktop_selection(SP_ACTIVE_DESKTOP)->singleItem();
    g_return_if_fail (item != NULL);

    blocked = true;

    /* Retrieve the label widget for the object's id */
    gchar *id = g_strdup(EntryID.get_text().c_str());
    g_strcanon (id, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.:", '_');
    if (!strcmp (id, item->getId())) {
        LabelID.set_markup_with_mnemonic(_("_ID:"));
    } else if (!*id || !isalnum (*id)) {
        LabelID.set_text (_("Id invalid! "));
    } else if (SP_ACTIVE_DOCUMENT->getObjectById(id) != NULL) {
        LabelID.set_text (_("Id exists! "));
    } else {
        SPException ex;
        LabelID.set_markup_with_mnemonic(_("_ID:"));
        SP_EXCEPTION_INIT (&ex);
        item->setAttribute("id", id, &ex);
        DocumentUndo::done(SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM, _("Set object ID"));
    }
    g_free (id);

    /* Retrieve the label widget for the object's label */
    Glib::ustring label = EntryLabel.get_text();
    g_assert(!label.empty());

    /* Give feedback on success of setting the drawing object's label
     * using the widget's label text
     */
    SPObject *obj = (SPObject*)item;
    if (label.compare (obj->defaultLabel())) {
        obj->setLabel(label.c_str());
        DocumentUndo::done(SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM,
                _("Set object label"));
    }

    /* Retrieve the title */
    if (obj->setTitle(EntryTitle.get_text().c_str()))
        DocumentUndo::done(SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM,
                _("Set object title"));

    /* Retrieve the description */
    Gtk::TextBuffer::iterator start, end;
    TextViewDescription.get_buffer()->get_bounds(start, end);
    Glib::ustring desc = TextViewDescription.get_buffer()->get_text(start, end, TRUE);
    if (obj->setDesc(desc.c_str()))
        DocumentUndo::done(SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM,
                _("Set object description"));
    
    blocked = false;
}

void ObjectProperties::sensitivity_toggled (void)
{
    if (blocked)
    {
        return;
    }

    SPItem *item = sp_desktop_selection(SP_ACTIVE_DESKTOP)->singleItem();
    g_return_if_fail (item != NULL);

    blocked = true;
    item->setLocked(CBLock.get_active());
    DocumentUndo::done(SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM,
               CBLock.get_active()? _("Lock object") : _("Unlock object"));
    blocked = false;
}

void ObjectProperties::hidden_toggled(void)
{
    if (blocked)
    {
        return;
    }

    SPItem *item = sp_desktop_selection(SP_ACTIVE_DESKTOP)->singleItem();
    g_return_if_fail (item != NULL);

    blocked = true;
    item->setExplicitlyHidden(CBHide.get_active());
    DocumentUndo::done(SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM,
               CBHide.get_active()? _("Hide object") : _("Unhide object"));
    blocked = false;
}

/**
 * ObjectProperties callback for the selection of an other object.
 */
void on_selection_changed(Inkscape::Application */*inkscape*/, Inkscape::Selection */*selection*/, ObjectProperties *dial)
{
    dial->widget_setup();
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
