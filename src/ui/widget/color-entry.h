/** @file
 * Entry widget for typing color value in css form
 *//*
 * Authors:
 *   Tomasz Boczkowski <penginsbacon@gmail.com>
 *
 * Copyright (C) 2014 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_COLOR_ENTRY_H
#define SEEN_COLOR_ENTRY_H

#include <gtkmm/entry.h>
#include "ui/selected-color.h"

namespace Inkscape {
namespace UI {
namespace Widget {

class ColorEntry : public Gtk::Entry
{
public:
    ColorEntry(SelectedColor &color);
    virtual ~ColorEntry();

protected:
    void on_changed();

private:
    void _onColorChanged();

    SelectedColor &_color;
    sigc::connection _color_changed_connection;
    sigc::connection _color_dragged_connection;
    bool _updating;
    bool _updatingrgba;
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
