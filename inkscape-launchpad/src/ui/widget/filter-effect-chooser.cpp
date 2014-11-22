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

#include "filter-effect-chooser.h"
#include <glibmm/i18n.h>

#include "desktop.h"
#include "desktop-handles.h"
#include "document.h"
#include "inkscape.h"

namespace Inkscape {
namespace UI {
namespace Widget {

SimpleFilterModifier::SimpleFilterModifier(int flags)
    : _lb_blend(_("Blend mode:")),
      _lb_blur(_("_Blur:")),
      _lb_blur_unit(_("%")),
      _blend(BlendModeConverter, SP_ATTR_INVALID, false),
      _blur(_("Blur (%)"), 0, 0, 100, 1, 0.01, 1)
{
    _flags = flags;

    if (flags & BLEND) {
        add(_hb_blend);
        _hb_blend.pack_start(_lb_blend, false, false, 0);
        _hb_blend.pack_start(_blend);
    }
    if (flags & BLUR) {
        add(_blur);
    }

    show_all_children();

    _hb_blend.set_spacing(12);
    _lb_blend.set_use_underline();
    _lb_blend.set_mnemonic_widget(_blend);
    _blend.signal_changed().connect(signal_blend_blur_changed());
    _blur.signal_value_changed().connect(signal_blend_blur_changed());
}

sigc::signal<void>& SimpleFilterModifier::signal_blend_blur_changed()
{
    return _signal_blend_blur_changed;
}

const Glib::ustring SimpleFilterModifier::get_blend_mode()
{
    if (!(_flags & BLEND)) {
        return "normal";
    }

    const Util::EnumData<Inkscape::Filters::FilterBlendMode> *d = _blend.get_active_data();
    if (d) {
        return _blend.get_active_data()->key;
    } else
        return "normal";
}

void SimpleFilterModifier::set_blend_mode(const int val)
{
    _blend.set_active(val);
}

double SimpleFilterModifier::get_blur_value() const
{
    return _blur.get_value();
}

void SimpleFilterModifier::set_blur_value(const double val)
{
    _blur.set_value(val);
}

void SimpleFilterModifier::set_blur_sensitive(const bool s)
{
    _blur.set_sensitive(s);
}

}
}
}

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
