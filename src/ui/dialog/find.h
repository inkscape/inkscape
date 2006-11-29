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

#include <gtkmm/notebook.h>
#include <glibmm/i18n.h>

#include "dialog.h"
#include "ui/widget/notebook-page.h"
#include "ui/widget/button.h"
#include "ui/widget/entry.h"

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
