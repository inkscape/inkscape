#ifndef __UI_WIDGET_ZOOMSTATUS_H__
#define __UI_WIDGET_ZOOMSTATUS_H__

/** \file
 * Enhanced spinbutton.
 *
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005 The Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/adjustment.h>
#include <gtkmm/spinbutton.h>

struct SPDesktop;

namespace Gtk { class Widget; }
namespace Inkscape {
    namespace UI {
        namespace Widget {

class ZoomStatus : public Gtk::SpinButton
{
public:
    ZoomStatus();
    ~ZoomStatus();

    void init (SPDesktop*);
    void update();

protected:
    Gtk::Adjustment _adj;
    SPDesktop *_dt;
    bool _upd_f;

    virtual int  on_input (double*);
    virtual bool on_output();
    virtual void on_value_changed();
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape


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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
