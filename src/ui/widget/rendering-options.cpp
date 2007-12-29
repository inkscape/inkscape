/**
 * \brief Rendering options widget
 *
 * Author:
 *   Kees Cook <kees@outflux.net>
 *
 * Copyright (C) 2007 Kees Cook
 * Copyright (C) 2004 Bryce Harrington
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/i18n.h>

#include "unit-constants.h"
#include "rendering-options.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 *    Construct a Rendering Options widget
 *
 */
  
RenderingOptions::RenderingOptions () :
      Gtk::VBox (),
      //Labelled(label, tooltip, new Gtk::VBox(), suffix, icon, mnemonic),
      _radio_cairo ( new Gtk::RadioButton () ),
      //_radio_bitmap( new Gtk::RadioButton (_radio_cairo->get_group ()), 
      _radio_bitmap( new Gtk::RadioButton () ),
      _widget_cairo( Glib::ustring(_("_Vector")),
                     Glib::ustring(_("Render using Cairo vector operations.  The resulting image is usually smaller in file "
                        "size and can be arbitrarily scaled, but some "
                        "filter effects will not be correctly rendered.")),
                     _radio_cairo,
                     Glib::ustring(""), Glib::ustring(""),
                     true),
      _widget_bitmap(Glib::ustring(_("_Bitmap")),
                     Glib::ustring(_("Render everything as bitmap.  The resulting image "
                        "is usually larger in file size and cannot be "
                        "arbitrarily scaled without quality loss, but all "
                        "objects will be rendered exactly as displayed.")),
                     _radio_bitmap,
                     Glib::ustring(""), Glib::ustring(""),
                     true),
      _dpi( _("DPI"), Glib::ustring(_("Preferred resolution of rendering, in dots per inch.")),
                     1,
                     Glib::ustring(""), Glib::ustring(""),
                     false)
{
    set_border_width(2);

    // default to cairo operations
    _radio_cairo->set_active (true);
    Gtk::RadioButtonGroup group = _radio_cairo->get_group ();
    _radio_bitmap->set_group (group);

    // configure default DPI
    _dpi.setRange(PT_PER_IN,2400.0);
    _dpi.setValue(PT_PER_IN);

    // fill up container
    add (_widget_cairo);
    add (_widget_bitmap);
    add (_dpi);

    show_all_children ();
}

bool
RenderingOptions::as_bitmap ()
{
    return _radio_bitmap->get_active();
}

double
RenderingOptions::bitmap_dpi ()
{
    return _dpi.getValue();
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape

/* 
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
