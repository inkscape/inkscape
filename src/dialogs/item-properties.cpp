/** @file
 * @brief Object properties dialog
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 1999-2006 Authors
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtktable.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkexpander.h>
#include <gtk/gtktextview.h>
#include <gtk/gtktooltips.h>

#include "../desktop-handles.h"
#include "dialog-events.h"
#include "../document.h"
#include <glibmm/i18n.h>
#include "../helper/window.h"
#include "../inkscape.h"
#include "../interface.h"
#include "../macros.h"
#include "../preferences.h"
#include "../selection.h"
#include "../sp-item.h"
#include "../verbs.h"
#include "../widgets/sp-attribute-widget.h"
#include "../widgets/sp-widget.h"

#define MIN_ONSCREEN_DISTANCE 50

static GtkWidget *dlg = NULL;
static win_data wd;

// impossible original values to make sure they are read from prefs
static gint x = -1000, y = -1000, w = 0, h = 0;
static Glib::ustring const prefs_path = "/dialogs/object/";

static void sp_item_widget_modify_selection (SPWidget *spw, Inkscape::Selection *selection, guint flags, GtkWidget *itemw);
static void sp_item_widget_change_selection (SPWidget *spw, Inkscape::Selection *selection, GtkWidget *itemw);
static void sp_item_widget_setup (SPWidget *spw, Inkscape::Selection *selection);
static void sp_item_widget_sensitivity_toggled (GtkWidget *widget, SPWidget *spw);
static void sp_item_widget_hidden_toggled (GtkWidget *widget, SPWidget *spw);
static void sp_item_widget_label_changed (GtkWidget *widget, SPWidget *spw);

static void
sp_item_dialog_destroy( GtkObject */*object*/, gpointer /*data*/ )
{
    sp_signal_disconnect_by_data (INKSCAPE, dlg);
    wd.win = dlg = NULL;
    wd.stop = 0;
}

static gboolean
sp_item_dialog_delete( GtkObject */*object*/, GdkEvent */*event*/, gpointer /*data*/ )
{
    gtk_window_get_position ((GtkWindow *) dlg, &x, &y);
    gtk_window_get_size ((GtkWindow *) dlg, &w, &h);

    if (x<0) x=0;
    if (y<0) y=0;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt(prefs_path + "x", x);
    prefs->setInt(prefs_path + "y", y);
    prefs->setInt(prefs_path + "w", w);
    prefs->setInt(prefs_path + "h", h);

    return FALSE; // which means, go ahead and destroy it

}

/**
 * \brief  Creates new instance of item properties widget
 *
 */
GtkWidget *
sp_item_widget_new (void)
{

    GtkWidget *spw, *vb, *t, *cb, *l, *f, *tf, *pb, *int_expander, *int_label;
    GtkTextBuffer *desc_buffer;

    GtkTooltips *tt = gtk_tooltips_new();

    /* Create container widget */
    spw = sp_widget_new_global (INKSCAPE);
    gtk_signal_connect ( GTK_OBJECT (spw), "modify_selection",
                         GTK_SIGNAL_FUNC (sp_item_widget_modify_selection),
                         spw );
    gtk_signal_connect ( GTK_OBJECT (spw), "change_selection",
                         GTK_SIGNAL_FUNC (sp_item_widget_change_selection),
                         spw );

    vb = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (spw), vb);

    t = gtk_table_new (3, 4, FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(t), 4);
    gtk_table_set_row_spacings (GTK_TABLE (t), 4);
    gtk_table_set_col_spacings (GTK_TABLE (t), 4);
    gtk_box_pack_start (GTK_BOX (vb), t, TRUE, TRUE, 0);


    /* Create the label for the object id */
    l = gtk_label_new_with_mnemonic (_("_Id"));
    gtk_misc_set_alignment (GTK_MISC (l), 1, 0.5);
    gtk_table_attach ( GTK_TABLE (t), l, 0, 1, 0, 1,
                       (GtkAttachOptions)( GTK_SHRINK | GTK_FILL ),
                       (GtkAttachOptions)0, 0, 0 );
    gtk_object_set_data (GTK_OBJECT (spw), "id_label", l);

    /* Create the entry box for the object id */
    tf = gtk_entry_new ();
    gtk_tooltips_set_tip (tt, tf, _("The id= attribute (only letters, digits, and the characters .-_: allowed)"), NULL);
    gtk_entry_set_max_length (GTK_ENTRY (tf), 64);
    gtk_table_attach ( GTK_TABLE (t), tf, 1, 2, 0, 1,
                       (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                       (GtkAttachOptions)0, 0, 0 );
    gtk_object_set_data (GTK_OBJECT (spw), "id", tf);
    gtk_label_set_mnemonic_widget (GTK_LABEL(l), tf);

    // pressing enter in the id field is the same as clicking Set:
    g_signal_connect ( G_OBJECT (tf), "activate", G_CALLBACK (sp_item_widget_label_changed), spw);
    // focus is in the id field initially:
    gtk_widget_grab_focus (GTK_WIDGET (tf));

    /* Button for setting the object's id, label, title and description. */
    pb = gtk_button_new_with_mnemonic (_("_Set"));
    gtk_table_attach ( GTK_TABLE (t), pb, 2, 3, 0, 1,
                       (GtkAttachOptions)( GTK_SHRINK | GTK_FILL ),
                       (GtkAttachOptions)0, 0, 0 );
    gtk_signal_connect ( GTK_OBJECT (pb), "clicked",
                         GTK_SIGNAL_FUNC (sp_item_widget_label_changed),
                         spw );

    /* Create the label for the object label */
    l = gtk_label_new_with_mnemonic (_("_Label"));
    gtk_misc_set_alignment (GTK_MISC (l), 1, 0.5);
    gtk_table_attach ( GTK_TABLE (t), l, 0, 1, 1, 2,
                       (GtkAttachOptions)( GTK_SHRINK | GTK_FILL ),
                       (GtkAttachOptions)0, 0, 0 );
    gtk_object_set_data (GTK_OBJECT (spw), "label_label", l);

    /* Create the entry box for the object label */
    tf = gtk_entry_new ();
    gtk_tooltips_set_tip (tt, tf, _("A freeform label for the object"), NULL);
    gtk_entry_set_max_length (GTK_ENTRY (tf), 256);
    gtk_table_attach ( GTK_TABLE (t), tf, 1, 2, 1, 2,
                       (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                       (GtkAttachOptions)0, 0, 0 );
    gtk_object_set_data (GTK_OBJECT (spw), "label", tf);
    gtk_label_set_mnemonic_widget (GTK_LABEL(l), tf);

    // pressing enter in the label field is the same as clicking Set:
    g_signal_connect ( G_OBJECT (tf), "activate", G_CALLBACK (sp_item_widget_label_changed), spw);

    /* Create the label for the object title */
    l = gtk_label_new_with_mnemonic (_("_Title"));
    gtk_misc_set_alignment (GTK_MISC (l), 1, 0.5);
    gtk_table_attach ( GTK_TABLE (t), l, 0, 1, 2, 3,
                       (GtkAttachOptions)( GTK_SHRINK | GTK_FILL ),
                       (GtkAttachOptions)0, 0, 0 );
    gtk_object_set_data (GTK_OBJECT (spw), "title_label", l);

    /* Create the entry box for the object title */
    tf = gtk_entry_new ();
    gtk_widget_set_sensitive (GTK_WIDGET (tf), FALSE);
    gtk_entry_set_max_length (GTK_ENTRY (tf), 256);
    gtk_table_attach ( GTK_TABLE (t), tf, 1, 3, 2, 3,
                       (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                       (GtkAttachOptions)0, 0, 0 );
    gtk_object_set_data (GTK_OBJECT (spw), "title", tf);
    gtk_label_set_mnemonic_widget (GTK_LABEL(l), tf);

    /* Create the frame for the object description */
    l = gtk_label_new_with_mnemonic (_("_Description"));
    f = gtk_frame_new (NULL);
    gtk_frame_set_label_widget (GTK_FRAME (f),l);
    gtk_table_attach ( GTK_TABLE (t), f, 0, 3, 3, 4,
                       (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                       (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ), 0, 0 );

    /* Create the text view box for the object description */
    GtkWidget *textframe = gtk_frame_new(NULL);
    gtk_container_set_border_width(GTK_CONTAINER(textframe), 4);
    gtk_widget_set_sensitive (GTK_WIDGET (textframe), FALSE);
    gtk_container_add (GTK_CONTAINER (f), textframe);
    gtk_frame_set_shadow_type (GTK_FRAME (textframe), GTK_SHADOW_IN);
    gtk_object_set_data(GTK_OBJECT(spw), "desc_frame", textframe);

    tf = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(tf), GTK_WRAP_WORD);
    desc_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tf));
    gtk_text_buffer_set_text(desc_buffer, "", -1);
    gtk_container_add (GTK_CONTAINER (textframe), tf);
    gtk_object_set_data (GTK_OBJECT (spw), "desc", tf);
    gtk_label_set_mnemonic_widget (GTK_LABEL (gtk_frame_get_label_widget (GTK_FRAME (f))), tf);

    /* Check boxes */
    GtkWidget *hb_cb = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vb), hb_cb, FALSE, FALSE, 0);
    t = gtk_table_new (1, 2, TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(t), 0);
    gtk_box_pack_start (GTK_BOX (hb_cb), t, TRUE, TRUE, 10);

    /* Hide */
    cb = gtk_check_button_new_with_mnemonic (_("_Hide"));
    gtk_tooltips_set_tip (tt, cb, _("Check to make the object invisible"), NULL);
    gtk_table_attach ( GTK_TABLE (t), cb, 0, 1, 0, 1,
                       (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                       (GtkAttachOptions)0, 0, 0 );
    g_signal_connect (G_OBJECT(cb), "toggled", G_CALLBACK(sp_item_widget_hidden_toggled), spw);
    gtk_object_set_data(GTK_OBJECT(spw), "hidden", cb);

    /* Lock */
    // TRANSLATORS: "Lock" is a verb here
    cb = gtk_check_button_new_with_mnemonic (_("L_ock"));
    gtk_tooltips_set_tip (tt, cb, _("Check to make the object insensitive (not selectable by mouse)"), NULL);
    gtk_table_attach ( GTK_TABLE (t), cb, 1, 2, 0, 1,
                       (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                       (GtkAttachOptions)0, 0, 0 );
    gtk_signal_connect ( GTK_OBJECT (cb), "toggled",
                         GTK_SIGNAL_FUNC (sp_item_widget_sensitivity_toggled),
                         spw );
    gtk_object_set_data (GTK_OBJECT (spw), "sensitive", cb);

    /* Create the frame for interactivity options */
    int_label = gtk_label_new_with_mnemonic (_("_Interactivity"));
    int_expander = gtk_expander_new (NULL);
    gtk_expander_set_label_widget (GTK_EXPANDER (int_expander),int_label);
    gtk_object_set_data (GTK_OBJECT (spw), "interactivity", int_expander);

    gtk_box_pack_start (GTK_BOX (vb), int_expander, FALSE, FALSE, 0);

    gtk_widget_show_all (spw);

    sp_item_widget_setup (SP_WIDGET (spw), sp_desktop_selection (SP_ACTIVE_DESKTOP));

    return (GtkWidget *) spw;

} //end of sp_item_widget_new()



static void
sp_item_widget_modify_selection( SPWidget *spw,
                                 Inkscape::Selection *selection,
                                 guint /*flags*/,
                                 GtkWidget */*itemw*/ )
{
    sp_item_widget_setup (spw, selection);
}



static void
sp_item_widget_change_selection ( SPWidget *spw,
                                  Inkscape::Selection *selection,
                                  GtkWidget */*itemw*/ )
{
    sp_item_widget_setup (spw, selection);
}


/**
*  \param selection Selection to use; should not be NULL.
*/
static void
sp_item_widget_setup ( SPWidget *spw, Inkscape::Selection *selection )
{
    g_assert (selection != NULL);

    if (gtk_object_get_data (GTK_OBJECT (spw), "blocked"))
        return;

    if (!selection->singleItem()) {
        gtk_widget_set_sensitive (GTK_WIDGET (spw), FALSE);
        return;
    } else {
        gtk_widget_set_sensitive (GTK_WIDGET (spw), TRUE);
    }

    gtk_object_set_data (GTK_OBJECT (spw), "blocked", GUINT_TO_POINTER (TRUE));

    SPItem *item = selection->singleItem();

    /* Sensitive */
    GtkWidget *w = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), "sensitive"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), item->isLocked());

    /* Hidden */
    w = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), "hidden"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), item->isExplicitlyHidden());

    if (SP_OBJECT_IS_CLONED (item)) {

        /* ID */
        w = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), "id"));
        gtk_entry_set_text (GTK_ENTRY (w), "");
        gtk_widget_set_sensitive (w, FALSE);
        w = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), "id_label"));
        gtk_label_set_text (GTK_LABEL (w), _("Ref"));

        /* Label */
        w = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), "label"));
        gtk_entry_set_text (GTK_ENTRY (w), "");
        gtk_widget_set_sensitive (w, FALSE);
        w = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), "label_label"));
        gtk_label_set_text (GTK_LABEL (w), _("Ref"));

    } else {
        SPObject *obj = (SPObject*)item;

        /* ID */
        w = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), "id"));
        gtk_entry_set_text (GTK_ENTRY (w), obj->id);
        gtk_widget_set_sensitive (w, TRUE);
        w = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), "id_label"));
        gtk_label_set_markup_with_mnemonic (GTK_LABEL (w), _("_Id"));

        /* Label */
        w = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), "label"));
        gtk_entry_set_text (GTK_ENTRY (w), obj->defaultLabel());
        gtk_widget_set_sensitive (w, TRUE);

        /* Title */
        w = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), "title"));
        gchar *title = obj->title();
        if (title) {
            gtk_entry_set_text(GTK_ENTRY(w), title);
            g_free(title);
        }
        else gtk_entry_set_text(GTK_ENTRY(w), "");
        gtk_widget_set_sensitive(w, TRUE);

        /* Description */
        w = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), "desc"));
        GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w));
        gchar *desc = obj->desc();
        if (desc) {
            gtk_text_buffer_set_text(buf, desc, -1);
            g_free(desc);
        } else {
            gtk_text_buffer_set_text(buf, "", 0);
        }
        w = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), "desc_frame"));
        gtk_widget_set_sensitive(w, TRUE);

        w = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), "interactivity"));

        GtkWidget* int_table = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), "interactivity_table"));
        if (int_table){
            gtk_container_remove(GTK_CONTAINER(w), int_table);
        }

        const gchar* int_labels[10] = {"onclick", "onmouseover", "onmouseout", "onmousedown", "onmouseup", "onmousemove","onfocusin", "onfocusout", "onactivate", "onload"};

        int_table = sp_attribute_table_new (obj, 10, int_labels, int_labels);
        gtk_widget_show_all (int_table);
        gtk_object_set_data(GTK_OBJECT(spw), "interactivity_table", int_table);

        gtk_container_add (GTK_CONTAINER (w), int_table);

    }

    gtk_object_set_data (GTK_OBJECT (spw), "blocked", GUINT_TO_POINTER (FALSE));


} // end of sp_item_widget_setup()



static void
sp_item_widget_sensitivity_toggled (GtkWidget *widget, SPWidget *spw)
{
    if (gtk_object_get_data (GTK_OBJECT (spw), "blocked"))
        return;

    SPItem *item = sp_desktop_selection(SP_ACTIVE_DESKTOP)->singleItem();
    g_return_if_fail (item != NULL);

    gtk_object_set_data (GTK_OBJECT (spw), "blocked", GUINT_TO_POINTER (TRUE));

    item->setLocked(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));

    sp_document_done (SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM,
             gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))? _("Lock object") : _("Unlock object"));

    gtk_object_set_data (GTK_OBJECT (spw), "blocked", GUINT_TO_POINTER (FALSE));
}

void
sp_item_widget_hidden_toggled(GtkWidget *widget, SPWidget *spw)
{
    if (gtk_object_get_data (GTK_OBJECT (spw), "blocked"))
        return;

    SPItem *item = sp_desktop_selection(SP_ACTIVE_DESKTOP)->singleItem();
    g_return_if_fail (item != NULL);

    gtk_object_set_data (GTK_OBJECT (spw), "blocked", GUINT_TO_POINTER (TRUE));

    item->setExplicitlyHidden(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));

    sp_document_done (SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM,
             gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))? _("Hide object") : _("Unhide object"));

    gtk_object_set_data (GTK_OBJECT (spw), "blocked", GUINT_TO_POINTER (FALSE));
}

static void
sp_item_widget_label_changed( GtkWidget */*widget*/, SPWidget *spw )
{
    if (gtk_object_get_data (GTK_OBJECT (spw), "blocked"))
        return;

    SPItem *item = sp_desktop_selection(SP_ACTIVE_DESKTOP)->singleItem();
    g_return_if_fail (item != NULL);

    gtk_object_set_data (GTK_OBJECT (spw), "blocked", GUINT_TO_POINTER (TRUE));

    /* Retrieve the label widget for the object's id */
    GtkWidget *id_entry = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), "id"));
    gchar *id = (gchar *) gtk_entry_get_text (GTK_ENTRY (id_entry));
    g_strcanon (id, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.:", '_');
    GtkWidget *id_label = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), "id_label"));
    if (!strcmp (id, SP_OBJECT_ID(item))) {
        gtk_label_set_markup_with_mnemonic (GTK_LABEL (id_label), _("_Id"));
    } else if (!*id || !isalnum (*id)) {
        gtk_label_set_text (GTK_LABEL (id_label), _("Id invalid! "));
    } else if (SP_ACTIVE_DOCUMENT->getObjectById(id) != NULL) {
        gtk_label_set_text (GTK_LABEL (id_label), _("Id exists! "));
    } else {
        SPException ex;
        gtk_label_set_markup_with_mnemonic (GTK_LABEL (id_label), _("_Id"));
        SP_EXCEPTION_INIT (&ex);
        sp_object_setAttribute (SP_OBJECT (item), "id", id, &ex);
        sp_document_done (SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM,
                                _("Set object ID"));
    }

    /* Retrieve the label widget for the object's label */
    GtkWidget *label_entry = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), "label"));
    gchar *label = (gchar *)gtk_entry_get_text (GTK_ENTRY (label_entry));
    g_assert(label != NULL);

    /* Give feedback on success of setting the drawing object's label
     * using the widget's label text
     */
    SPObject *obj = (SPObject*)item;
    if (strcmp (label, obj->defaultLabel())) {
        obj->setLabel(label);
        sp_document_done (SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM,
                                _("Set object label"));
    }

    /* Retrieve the title */
    GtkWidget *w = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), "title"));
    gchar *title = (gchar *)gtk_entry_get_text(GTK_ENTRY (w));
    if (obj->setTitle(title))
        sp_document_done(SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM,
                         _("Set object title"));

    /* Retrieve the description */
    GtkTextView *tv = GTK_TEXT_VIEW(gtk_object_get_data(GTK_OBJECT(spw), "desc"));
    GtkTextBuffer *buf = gtk_text_view_get_buffer(tv);
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buf, &start, &end);
    gchar *desc = gtk_text_buffer_get_text(buf, &start, &end, TRUE);
    if (obj->setDesc(desc))
        sp_document_done(SP_ACTIVE_DOCUMENT, SP_VERB_DIALOG_ITEM,
                         _("Set object description"));
    g_free(desc);

    gtk_object_set_data (GTK_OBJECT (spw), "blocked", GUINT_TO_POINTER (FALSE));

} // end of sp_item_widget_label_changed()


/**
 * \brief  Dialog
 *
 */
void
sp_item_dialog (void)
{
    if (dlg == NULL) {

        gchar title[500];
        sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_DIALOG_ITEM), title);
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();

        dlg = sp_window_new (title, TRUE);
        if (x == -1000 || y == -1000) {
            x = prefs->getInt(prefs_path + "x", -1000);
            y = prefs->getInt(prefs_path + "y", -1000);
        }
        if (w ==0 || h == 0) {
            w = prefs->getInt(prefs_path + "w", 0);
            h = prefs->getInt(prefs_path + "h", 0);
        }

//        if (x<0) x=0;
//        if (y<0) y=0;

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

        g_signal_connect   ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_transientize_callback), &wd);
        gtk_signal_connect ( GTK_OBJECT (dlg), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg);
        gtk_signal_connect ( GTK_OBJECT (dlg), "destroy", G_CALLBACK (sp_item_dialog_destroy), dlg);
        gtk_signal_connect ( GTK_OBJECT (dlg), "delete_event", G_CALLBACK (sp_item_dialog_delete), dlg);
        g_signal_connect   ( G_OBJECT (INKSCAPE), "shut_down", G_CALLBACK (sp_item_dialog_delete), dlg);
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), dlg);
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), dlg);

        // Dialog-specific stuff
        GtkWidget *itemw = sp_item_widget_new ();
        gtk_widget_show (itemw);
        gtk_container_add (GTK_CONTAINER (dlg), itemw);

    }

    gtk_window_present ((GtkWindow *) dlg);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
