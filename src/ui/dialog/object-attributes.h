/** @file
 * Generic object attribute editor
 */
/* Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Licensed under GNU GPL
 */

#ifndef SEEN_DIALOGS_OBJECT_ATTRIBUTES_H
#define SEEN_DIALOGS_OBJECT_ATTRIBUTES_H

#include <glib.h>

#include "desktop.h"
#include "ui/dialog/desktop-tracker.h"
#include "ui/widget/panel.h"
#include "widgets/sp-attribute-widget.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

/**
 * A dialog widget to show object attributes (currently for images and links).
 */
class ObjectAttributes : public Widget::Panel {
public:
    ObjectAttributes ();
    ~ObjectAttributes ();
    
    /**
     * Returns a new instance of the object attributes dialog.
     * 
	 * Auxiliary function needed by the DialogManager.
     */
    static ObjectAttributes &getInstance() { return *new ObjectAttributes(); }
    
    /**
     * Updates entries and other child widgets on selection change, object modification, etc.
     */
    void widget_setup(void);

private:
    /**
     * Is UI update bloched?
     */
    bool blocked;
    
    /**
     * Contains a pointer to the currently selected item (NULL in case nothing is or multiple objects are selected).
     */
    SPItem *CurrentItem;
    
    /**
     * Child widget to show the object attributes.
     * 
     * attrTable makes the labels and edit boxes for the attributes defined
     * in the SPAttrDesc arrays at the top of the cpp-file. This widgets also
     * ensures object attribute modifications by the user are set.
     */
    SPAttributeTable attrTable;
    
    /**
     * Stores the current desktop.
     */
    SPDesktop *desktop;
    
    /**
     * Auxiliary widget to keep track of desktop changes for the floating dialog.
     */
    DesktopTracker deskTrack;
    
    /**
     * Link to callback function for a change in desktop (window).
     */
    sigc::connection desktopChangeConn;
    
    /**
     * Link to callback function for a selection change.
     */
    sigc::connection selectChangedConn;
    sigc::connection subselChangedConn;
    
    /**
     * Link to callback function for a modification of the selected object.
     */
    sigc::connection selectModifiedConn;
    
    /**
     * Callback function invoked by the desktop tracker in case of a modification of the selected object.
     */
    void selectionModifiedCB( guint flags );
    
    /*
     * Can be invoked for setting the desktop. Currently not used.
     */
    // void setDesktop(SPDesktop *desktop);
    
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
