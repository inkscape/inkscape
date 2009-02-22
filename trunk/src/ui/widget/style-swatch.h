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

#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/enums.h>

#include <glibmm/i18n.h>

#include "desktop.h"
#include "verbs.h"
#include "button.h"
#include "preferences.h"

class SPUnit;
class SPStyle;
class SPCSSAttr;

namespace Inkscape {
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
    Gtk::Table _table;
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

    SPUnit *_sw_unit;

    Gtk::Tooltips _tooltips;
    
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
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
