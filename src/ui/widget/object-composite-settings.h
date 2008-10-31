#ifndef SEEN_UI_WIDGET_OBJECT_COMPOSITE_SETTINGS_H
#define SEEN_UI_WIDGET_OBJECT_COMPOSITE_SETTINGS_H

/*
 * A widget for controlling object compositing (filter, opacity, etc.)
 *
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
#include <gtkmm/spinbutton.h>
#include <gtkmm/scale.h>

#include <glibmm/ustring.h>

#include "ui/widget/filter-effect-chooser.h"
#include "ui/widget/style-subject.h"

namespace Inkscape {
namespace UI {
namespace Widget {

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
    Gtk::HBox       _opacity_label_box;
    Gtk::HBox       _opacity_hbox;
    Gtk::Label      _opacity_label;
    Gtk::Adjustment _opacity_adjustment;
    Gtk::HScale     _opacity_hscale;
    Gtk::SpinButton _opacity_spin_button;

    StyleSubject *_subject;

    SimpleFilterModifier _fe_cb;
    Gtk::VBox       _fe_vbox;
    Gtk::Alignment  _fe_alignment;

    static void _on_desktop_activate(Inkscape::Application *application, SPDesktop *desktop, ObjectCompositeSettings *w);
    static void _on_desktop_deactivate(Inkscape::Application *application, SPDesktop *desktop, ObjectCompositeSettings *w);
    void _subjectChanged();
    sigc::connection _subject_changed;

    void _blendBlurValueChanged();
    void _opacityValueChanged();

    bool _blocked;

    gulong _desktop_activated;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
