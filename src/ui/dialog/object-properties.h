/** 
 * @file Object properties dialog.
 */
/* Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2012 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_DIALOGS_ITEM_PROPERTIES_H
#define SEEN_DIALOGS_ITEM_PROPERTIES_H

#include <gtkmm/entry.h>
#include <gtkmm/expander.h>
#include <gtkmm/frame.h>
#include <gtkmm/textview.h>

#include "ui/widget/panel.h"
#include "ui/dialog/desktop-tracker.h"
#include "widgets/sp-attribute-widget.h"

class SPDesktop;
class SPItem;

namespace Inkscape {
namespace UI {
namespace Dialog {

/**
 * A dialog widget to show object properties.
 *
 * A widget to enter an ID, label, title and description for an object.
 * In addition it allows to edit the properties of an object.
 */
class ObjectProperties : public Widget::Panel {
public:
    ObjectProperties ();
    ~ObjectProperties ();
    
    static ObjectProperties &getInstance() { return *new ObjectProperties(); }
    
    /**
     * Updates entries and other child widgets on selection change, object modification, etc.
     */
    void widget_setup(void);

private:
    bool blocked;
    SPItem *CurrentItem; //to store the current item, for not wasting resources
    std::vector<Glib::ustring> int_labels;
    
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
    
    SPDesktop *desktop;
    DesktopTracker deskTrack;
    sigc::connection desktopChangeConn;
    sigc::connection selectChangedConn;
    sigc::connection subselChangedConn;
    
    /**
     * Constructor auxiliary function creating the child widgets.
     */
    void MakeWidget(void);
    
    /**
     * Sets object properties (ID, label, title, description) on user input.
     */
    void label_changed(void);
    
	/**
     * Callback for checkbox Lock.
     */
    void sensitivity_toggled (void);
    
	/**
     * Callback for checkbox Hide.
     */
    void hidden_toggled(void);
    
    /*
     * On signal modified, invokes an update.
     */
    //void selectionModifiedCB( guint flags );
    
    /**
     * Can be invoked for setting the desktop. Currently not used.
     */
    void setDesktop(SPDesktop *desktop);
    
    /**
     * Is invoked by the desktop tracker when the desktop changes.
     */
    void setTargetDesktop(SPDesktop *desktop);

};

}
}
}

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
