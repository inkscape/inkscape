/** @file
 * @brief Static style swatch (fill, stroke, opacity)
 */
/* Authors:
 *   buliabyak@gmail.com
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2005-2008 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_CURRENT_STYLE_H
#define INKSCAPE_UI_CURRENT_STYLE_H

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/enums.h>

#include "desktop.h"
#include "preferences.h"

class SPStyle;
class SPCSSAttr;

namespace Gtk {
#if WITH_GTKMM_3_0
class Grid;
#else
class Table;
#endif
}

namespace Inkscape {

namespace Util {
    class Unit;
}

namespace UI {
namespace Widget {

class StyleSwatch : public Gtk::HBox
{
public:
    StyleSwatch (SPCSSAttr *attr, gchar const *main_tip);

    ~StyleSwatch();

    void setStyle(SPStyle *style);
    void setStyle(SPCSSAttr *attr);
    SPCSSAttr *getStyle();

    void setWatchedTool (const char *path, bool synthesize);

    void setClickVerb(sp_verb_t verb_t);
    void setDesktop(SPDesktop *desktop);
    bool on_click(GdkEventButton *event);

private:
    class ToolObserver;
    class StyleObserver;

    SPDesktop *_desktop;
    sp_verb_t _verb_t;
    SPCSSAttr *_css;
    ToolObserver *_tool_obs;
    StyleObserver *_style_obs;
    Glib::ustring _tool_path;

    Gtk::EventBox _swatch;

#if WITH_GTKMM_3_0
    Gtk::Grid *_table;
#else
    Gtk::Table *_table;
#endif

    Gtk::Label _label[2];
    Gtk::EventBox _place[2];
    Gtk::EventBox _opacity_place;
    Gtk::Label _value[2];
    Gtk::Label _opacity_value;
    Gtk::Widget *_color_preview[2];
    Glib::ustring __color[2];
    Gtk::HBox _stroke;
    Gtk::EventBox _stroke_width_place;
    Gtk::Label _stroke_width;

    Inkscape::Util::Unit *_sw_unit;

friend class ToolObserver;
};


} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_BUTTON_H

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
