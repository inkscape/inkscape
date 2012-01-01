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
#include <gtk/gtk.h>
#include <glibmm/i18n.h>

#include "../desktop-handles.h"
#include "../document.h"
#include "../helper/window.h"
#include "../inkscape.h"
#include "../interface.h"
#include "../macros.h"
#include "../preferences.h"
#include "../selection.h"
#include "../sp-item.h"
#include "../verbs.h"
#include "../widgets/sp-widget.h"
#include "item-properties.h"

using Inkscape::DocumentUndo;

#define MIN_ONSCREEN_DISTANCE 50

static SPItemDialog *spid = NULL;

static void sp_item_dialog_delete( GtkObject */*object*/, GdkEvent */*event*/, gpointer /*data*/ );
static void sp_item_widget_modify_selection (SPWidget *spw, Inkscape::Selection *selection, guint flags, GtkWidget *itemw);
static void sp_item_widget_change_selection (SPWidget *spw, Inkscape::Selection *selection, GtkWidget *itemw);

static void sp_item_dialog_delete( GtkObject */*object*/, GdkEvent */*event*/, gpointer /*data*/ )
{
    if (spid)
    {
		delete spid;
		spid = NULL;
    }
}

static void sp_item_widget_modify_selection( SPWidget */*spw*/,
                                 Inkscape::Selection */*selection*/,
                                 guint /*flags*/,
                                 GtkWidget */*itemw*/ )
{
    if (spid)
    {
        spid->widget_setup();
    }
}

static void sp_item_widget_change_selection ( SPWidget */*spw*/,
                                  Inkscape::Selection */*selection*/,
                                  GtkWidget */*itemw*/ )
{
    if (spid)
    {
        spid->widget_setup();
    }
}


/**
 * Create a new static instance of the items dialog.
 */
void sp_item_dialog(void)
{
    if (spid == NULL) {
        spid = new SPItemDialog();
    }
}

/**
 * SPItemDialog class.
 * A widget for showing and editing the properties of an object.
 */
SPItemDialog::SPItemDialog (void) :
    prefs_path("/dialogs/object/"),
    x(-1000),// impossible original value to make sure they are read from prefs
    y(-1000),// impossible original value to make sure they are read from prefs
    w(0),
    h(0),
    blocked (false),
    closing (false),
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
    //intializing dialog
    gchar title[500];
    sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_DIALOG_ITEM), title);
    window = Inkscape::UI::window_new (title, true);
    GtkWidget *dlg;
    dlg = (GtkWidget*)window->gobj();
    
    //reading dialog position from preferences
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (x == -1000 || y == -1000) {
        x = prefs->getInt(prefs_path + "x", -1000);
        y = prefs->getInt(prefs_path + "y", -1000);
    }
    if (w ==0 || h == 0) {
        w = prefs->getInt(prefs_path + "w", 0);
        h = prefs->getInt(prefs_path + "h", 0);
    }
    if (w && h) {
        gtk_window_resize ((GtkWindow *) dlg, w, h);
    }
    if (x >= 0 && y >= 0 && (x < (gdk_screen_width()-MIN_ONSCREEN_DISTANCE)) && (y < (gdk_screen_height()-MIN_ONSCREEN_DISTANCE))) {
        gtk_window_move ((GtkWindow *) dlg, x, y);
    } else {
        gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
    }

    sp_transientize (dlg);
    wd.win = dlg;
    wd.stop = 0;

    //set callback for the new dialog
    g_signal_connect ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_transientize_callback), &wd);
    g_signal_connect ( G_OBJECT (dlg), "event", G_CALLBACK (sp_dialog_event_handler), dlg);
    // g_signal_connect ( G_OBJECT (dlg), "destroy", G_CALLBACK (sp_item_dialog_delete), dlg);
    g_signal_connect ( G_OBJECT (dlg), "delete_event", G_CALLBACK (sp_item_dialog_delete), dlg);
    g_signal_connect ( G_OBJECT (INKSCAPE), "shut_down", G_CALLBACK (sp_item_dialog_delete), dlg);
    g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), dlg);
    g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), dlg);

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

SPItemDialog::~SPItemDialog (void)
{
    if (closing)
    {
       return;
    }
    blocked = true;
    closing = true;
    gtk_window_get_position ((GtkWindow *) wd.win, &x, &y);
    gtk_window_get_size ((GtkWindow *) wd.win, &w, &h);

    if (x<0) x=0;
    if (y<0) y=0;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt(prefs_path + "x", x);
    prefs->setInt(prefs_path + "y", y);
    prefs->setInt(prefs_path + "w", w);
    prefs->setInt(prefs_path + "h", h);

    sp_signal_disconnect_by_data (INKSCAPE, wd.win);
    sp_signal_disconnect_by_data (INKSCAPE, &wd);
    if (window)
    {
        //should actually always  be true, but for safety check
        delete window;
        window = NULL;
        wd.win = NULL;
    }
}

void SPItemDialog::MakeWidget(void)
{
	// if (gtk_widget_get_visible (GTK_WIDGET(spw))) {
		g_signal_connect (G_OBJECT (INKSCAPE), "modify_selection", G_CALLBACK (sp_item_widget_modify_selection), wd.win);
		g_signal_connect (G_OBJECT (INKSCAPE), "change_selection", G_CALLBACK (sp_item_widget_change_selection), wd.win);
		g_signal_connect (G_OBJECT (INKSCAPE), "set_selection", G_CALLBACK (sp_item_widget_change_selection), wd.win);
	// }

    window->add(vb);
    TopTable.set_border_width(4);
    TopTable.set_row_spacings(4);
    TopTable.set_col_spacings(4);
    vb.pack_start (TopTable, true, true, 0);

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
    EntryID.signal_activate().connect(sigc::mem_fun(this, &SPItemDialog::label_changed));
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
    EntryLabel.signal_activate().connect(sigc::mem_fun(this, &SPItemDialog::label_changed));

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
    vb.pack_start (HBoxCheck, FALSE, FALSE, 0);
    CheckTable.set_border_width(0);
    HBoxCheck.pack_start (CheckTable, TRUE, TRUE, 10);

    /* Hide */
    CBHide.set_tooltip_text (_("Check to make the object invisible"));
    CheckTable.attach (CBHide, 0, 1, 0, 1,
                       Gtk::EXPAND | Gtk::FILL,
                       Gtk::AttachOptions(), 0, 0 );
    CBHide.signal_toggled().connect(sigc::mem_fun(this, &SPItemDialog::hidden_toggled));

    /* Lock */
    // TRANSLATORS: "Lock" is a verb here
    CBLock.set_tooltip_text (_("Check to make the object insensitive (not selectable by mouse)"));
    CheckTable.attach (CBLock, 1, 2, 0, 1,
                       Gtk::EXPAND | Gtk::FILL,
                       Gtk::AttachOptions(), 0, 0 );
    CBLock.signal_toggled().connect(sigc::mem_fun(this, &SPItemDialog::sensitivity_toggled));


    /* Button for setting the object's id, label, title and description. */
    HBoxCheck.pack_start (BSet, TRUE, TRUE, 10);
    BSet.signal_clicked().connect(sigc::mem_fun(this, &SPItemDialog::label_changed));

    /* Create the frame for interactivity options */
    EInteractivity.set_label_widget (LabelInteractivity);
    vb.pack_start (EInteractivity, FALSE, FALSE, 0);
    window->show_all ();
    widget_setup();
}

void SPItemDialog::widget_setup(void)
{
    if (blocked)
    {
        return;
    }

    Inkscape::Selection *selection = sp_desktop_selection (SP_ACTIVE_DESKTOP);

    if (!selection->singleItem()) {
        vb.set_sensitive (false);
        CurrentItem = NULL;
        //no selection anymore or multiple objects selected, means that we need
        //to close the connections to the previously selected object
        attrTable.clear();
        return;
    } else {
        vb.set_sensitive (true);
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

void SPItemDialog::label_changed(void)
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

void SPItemDialog::sensitivity_toggled (void)
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

void SPItemDialog::hidden_toggled(void)
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
