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

#include "ui/widget/filter-effect-chooser.h"

namespace Inkscape {
namespace UI {
namespace Widget {

class ObjectCompositeSettings : public Gtk::VBox {
public:
    ObjectCompositeSettings();
    ~ObjectCompositeSettings();

private:
    Gtk::VBox       _opacity_vbox;
    Gtk::HBox       _opacity_label_box;
    Gtk::HBox       _opacity_hbox;
    Gtk::Label      _opacity_label;
    Gtk::Adjustment _opacity_adjustment;
    Gtk::HScale     _opacity_hscale;
    Gtk::SpinButton _opacity_spin_button;

    SimpleFilterModifier _fe_cb;
    Gtk::VBox       _fe_vbox;
    Gtk::Alignment  _fe_alignment;

    void selectionChanged(Inkscape::Application *inkscape,
                          Inkscape::Selection *selection);

    static void on_selection_changed(Inkscape::Application *inkscape,
                                     Inkscape::Selection *selection,
                                     ObjectCompositeSettings *w);

    static void on_selection_modified(Inkscape::Application *inkscape,
                                      Inkscape::Selection *selection,
                                      guint flags,
                                      ObjectCompositeSettings *w);

    void _blendBlurValueChanged();
    void _opacityValueChanged();

    bool _blocked;

    gulong _sel_changed;
    gulong _subsel_changed;
    gulong _sel_modified;
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
