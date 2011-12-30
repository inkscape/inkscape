/** @file
 * \brief  Display settings dialog
 */
/* Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2011 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_DIALOGS_ITEM_PROPERTIES_H
#define SEEN_DIALOGS_ITEM_PROPERTIES_H

#include <gtk/gtk.h>
#include <gtkmm.h>
#include "dialog-events.h"
#include "../widgets/sp-attribute-widget.h"

GtkWidget *sp_item_widget_new(void);

void sp_item_dialog(void);

class SPItemDialog : public Gtk::Widget {
public:
    SPItemDialog ();
    ~SPItemDialog ();
    void widget_setup(void);

private:
    Glib::ustring const prefs_path;
    gint x;
    gint y;
    gint w;
    gint h;
    
    bool blocked;
    bool closing;
    
    Gtk::Window* window; // the window
    win_data wd;

    Gtk::VBox vb; //the VBox
    
    Gtk::Table TopTable; //the table with the object properties
    Gtk::Label LabelID; //the label for the object ID
    Gtk::Entry EntryID; //the entry for the object ID
    Gtk::Label LabelLabel; //the label for the object label
    Gtk::Entry EntryLabel; //the entry for the object label
    Gtk::Label LabelTitle; //the label for the object title
    Gtk::Entry EntryTitle; //the entry for the object title
    
    Gtk::Label LabelDescription; //the label for the object description
    Gtk::Frame FrameDescription; //the frame for the object description
    Gtk::Frame FrameTextDescription; //the frame for the text of the object description
    Gtk::TextView TextViewDescription; //the text view object showing the object description
    
    Gtk::HBox HBoxCheck; // the HBox for the check boxes
    Gtk::Table CheckTable; //the table for the check boxes
    Gtk::CheckButton CBHide; //the check button hide
    Gtk::CheckButton CBLock; //the check button lock
    Gtk::Button BSet; //the button set
    
    Gtk::Label LabelInteractivity; //the label for interactivity
    Gtk::Expander EInteractivity; //the label for interactivity

    SPAttributeTable attrTable; //the widget for showing the on... names at the bottom
    
    SPItem *CurrentItem; //to store the current item, for not wasting resources
    
    void MakeWidget(void);
    void label_changed(void);
    void sensitivity_toggled (void);
    void hidden_toggled(void);
};

#endif

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
