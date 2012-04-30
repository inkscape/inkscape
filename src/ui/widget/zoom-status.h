#ifndef SEEN_UI_WIDGET_ZOOMSTATUS_H
#define SEEN_UI_WIDGET_ZOOMSTATUS_H
/*
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005 The Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtkmm/adjustment.h>
#include "ui/widget/spinbutton.h"

struct SPDesktop;

namespace Gtk { class Widget; }
namespace Inkscape {
    namespace UI {
        namespace Widget {

/**
 * Enhanced spinbutton.
 */
class ZoomStatus : public Inkscape::UI::Widget::SpinButton
{
public:
    ZoomStatus();
    ~ZoomStatus();

    void init (SPDesktop*);
    void update();

protected:
#if WITH_GTKMM_3_0
    Glib::RefPtr<Gtk::Adjustment> _adj;
#else
    Gtk::Adjustment _adj;
#endif
    SPDesktop *_dt;
    bool _upd_f;

    virtual int  on_input (double*);
    virtual bool on_output();
    virtual void on_value_changed();
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape


#endif // SEEN_UI_WIDGET_ZOOMSTATUS_H


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
