#ifndef SEEN_UI_WIDGET_OBJECT_COMPOSITE_SETTINGS_H
#define SEEN_UI_WIDGET_OBJECT_COMPOSITE_SETTINGS_H

/*
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Gustav Broberg <broberg@kth.se>
 *
 * Copyright (C) 2004--2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/box.h>
#include <gtkmm/alignment.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>
#include <glibmm/ustring.h>

#include "ui/widget/filter-effect-chooser.h"
#include "ui/widget/spinbutton.h"

class SPDesktop;
struct InkscapeApplication;

namespace Inkscape {

namespace UI {
namespace Widget {

class StyleSubject;

/*
 * A widget for controlling object compositing (filter, opacity, etc.)
 */
class ObjectCompositeSettings : public Gtk::VBox {
public:
    ObjectCompositeSettings(unsigned int verb_code, char const *history_prefix, int flags);
    ~ObjectCompositeSettings();

    void setSubject(StyleSubject *subject);

private:
    unsigned int    _verb_code;
    Glib::ustring   _blur_tag;
    Glib::ustring   _opacity_tag;

    Gtk::VBox       _opacity_vbox;
    Inkscape::UI::Widget::SpinScale _opacity_scale;

    StyleSubject *_subject;

    SimpleFilterModifier _fe_cb;
    Gtk::VBox       _fe_vbox;

    bool _blocked;
    gulong _desktop_activated;
    sigc::connection _subject_changed;
    
    static void _on_desktop_activate(SPDesktop *desktop, ObjectCompositeSettings *w);
    static void _on_desktop_deactivate(SPDesktop *desktop, ObjectCompositeSettings *w);
    void _subjectChanged();
    void _blendBlurValueChanged();
    void _opacityValueChanged();

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
