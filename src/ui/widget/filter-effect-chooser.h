#ifndef __FILTER_EFFECT_CHOOSER_H__
#define __FILTER_EFFECT_CHOOSER_H__

/*
 * Filter effect selection selection widget
 *
 * Author:
 *   Nicholas Bishop <nicholasbishop@gmail.com>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/box.h>
#include <gtkmm/combobox.h>

#include "combo-enums.h"
#include "filter-enums.h"
#include "spin-scale.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/* Allows basic control over feBlend and feGaussianBlur effects,
   with an option to use the full filter effect controls. */
class SimpleFilterModifier : public Gtk::VBox
{
public:
    enum Flags {
      NONE=0,
      BLUR=1,
      BLEND=2
    };

    SimpleFilterModifier(int flags);

    sigc::signal<void>& signal_blend_blur_changed();

    const Glib::ustring get_blend_mode();
    // Uses blend mode enum values, or -1 for a complex filter
    void set_blend_mode(const int);

    double get_blur_value() const;
    void set_blur_value(const double);
    void set_blur_sensitive(const bool);
    Gtk::Label *get_blur_label() { return &_lb_blur; };

private:
    int _flags;
    Gtk::HBox _hb_blend;
    Gtk::HBox _hb_blur;
    Gtk::Label _lb_blend, _lb_blur, _lb_blur_unit;

    ComboBoxEnum<Inkscape::Filters::FilterBlendMode> _blend;
    SpinScale _blur;

    sigc::signal<void> _signal_blend_blur_changed;
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
