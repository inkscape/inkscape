/** @file
 * @brief Find dialog
 */
/* Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 2004, 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_FIND_H
#define INKSCAPE_UI_DIALOG_FIND_H

#include <glibmm/i18n.h>

#include "ui/widget/panel.h"
#include "ui/widget/button.h"
#include "ui/widget/entry.h" 
#include <gtkmm/separator.h>
#include <glib.h>

class SPItem;
class SPObject;

namespace Inkscape {
class Selection;

namespace UI {
namespace Dialog {

class Find : public UI::Widget::Panel {
public:
    Find();
    virtual ~Find();

    static Find &getInstance() { return *new Find(); }

protected:
    // Widgets:
    Inkscape::UI::Widget::Entry    _entry_text;
    Inkscape::UI::Widget::Entry    _entry_id;
    Inkscape::UI::Widget::Entry    _entry_style;
    Inkscape::UI::Widget::Entry    _entry_attribute;

    Inkscape::UI::Widget::CheckButton    _check_search_selection;
    Inkscape::UI::Widget::CheckButton    _check_search_layer;
    Inkscape::UI::Widget::CheckButton    _check_include_hidden;
    Inkscape::UI::Widget::CheckButton    _check_include_locked;
    
    // Type checkbutton widgets... 
    Inkscape::UI::Widget::CheckButton    _check_all;
    Inkscape::UI::Widget::CheckButton    _check_all_shapes;
    Inkscape::UI::Widget::CheckButton    _check_rects;
    Inkscape::UI::Widget::CheckButton    _check_ellipses;
    Inkscape::UI::Widget::CheckButton    _check_stars;
    Inkscape::UI::Widget::CheckButton    _check_spirals;
    Inkscape::UI::Widget::CheckButton    _check_paths;
    Inkscape::UI::Widget::CheckButton    _check_texts;
    Inkscape::UI::Widget::CheckButton    _check_groups;
    Inkscape::UI::Widget::CheckButton    _check_clones;
    Inkscape::UI::Widget::CheckButton    _check_images;
    Inkscape::UI::Widget::CheckButton    _check_offsets;  
    
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
    
    Inkscape::UI::Widget::Button    _button_clear;
    Inkscape::UI::Widget::Button    _button_find;
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
