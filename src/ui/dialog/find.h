/**
 * \brief Find dialog
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004, 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_FIND_H
#define INKSCAPE_UI_DIALOG_FIND_H

#include <glibmm/i18n.h>

#include "dialog.h"
#include "ui/widget/button.h"
#include "ui/widget/entry.h" 
#include <gtkmm/separator.h>

#include "message-stack.h"
#include "helper/window.h"
#include "macros.h"
#include "inkscape.h"
#include "document.h"
#include "desktop.h"
#include "selection.h"
#include "desktop-handles.h" 

#include "dialog-events.h"
#include "../prefs-utils.h"
#include "../verbs.h"
#include "../interface.h"
#include "../sp-text.h"
#include "../sp-flowtext.h"
#include "../text-editing.h"
#include "../sp-tspan.h"
#include "../selection-chemistry.h"
#include "../sp-defs.h"
#include "../sp-rect.h"
#include "../sp-ellipse.h"
#include "../sp-star.h"
#include "../sp-spiral.h"
#include "../sp-path.h"
#include "../sp-line.h"
#include "../sp-polyline.h"
#include "../sp-item-group.h"
#include "../sp-use.h"
#include "../sp-image.h"
#include "../sp-offset.h"
#include <xml/repr.h>

using namespace Inkscape::UI::Widget;

namespace Inkscape {
namespace UI {
namespace Dialog {

class Find : public Dialog {
public:
    Find();
    virtual ~Find();

    static Find *create() { return new Find(); }

protected:
    // Widgets:
    Entry    _entry_text;
    Entry    _entry_id;
    Entry    _entry_style;
    Entry    _entry_attribute;

    CheckButton    _check_search_selection;
    CheckButton    _check_search_layer;
    CheckButton    _check_include_hidden;
    CheckButton    _check_include_locked;
    
    // Type checkbutton widgets... 
    CheckButton    _check_all;
    CheckButton    _check_all_shapes;
    CheckButton    _check_rects;
    CheckButton    _check_ellipses;
    CheckButton    _check_stars;
    CheckButton    _check_spirals;
    CheckButton    _check_paths;
    CheckButton    _check_texts;
    CheckButton    _check_groups;
    CheckButton    _check_clones;
    CheckButton    _check_images;
    CheckButton    _check_offsets;  
    
    // Button-click handlers
    void    onClear();
    void    onFind();             
    void    onToggleAlltypes();
    void    onToggleShapes();    


    // onFind helper functions
    bool        item_id_match (SPItem *item, const gchar *id, bool exact);
    bool        item_text_match (SPItem *item, const gchar *text, bool exact);
    bool        item_style_match (SPItem *item, const gchar *text, bool exact);
    bool        item_attr_match (SPItem *item, const gchar *name, bool exact);
    GSList *    filter_fields (GSList *l, bool exact);
    bool        item_type_match (SPItem *item);
    GSList *    filter_types (GSList *l);
    GSList *    filter_list (GSList *l, bool exact);
    GSList *    all_items (SPObject *r, GSList *l, bool hidden, bool locked);
    GSList *    all_selection_items (Inkscape::Selection *s, GSList *l, SPObject *ancestor, bool hidden, bool locked);

    void        squeeze_window();
    
private:
    Find(Find const &d);
    Find& operator=(Find const &d);
    
    Button    _button_clear;
    Button    _button_find;
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_FIND_H

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
