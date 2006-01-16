#ifndef __COLOR_PREVIEW_H__
#define __COLOR_PREVIEW_H__

/** \file
 * A simple color preview widget, mainly used within a picker button.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2001-2005 Authors
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/eventbox.h>

namespace Inkscape {
    namespace UI {
        namespace Widget {

class ColorPreview : public Gtk::Widget {
public:
    ColorPreview (guint32 rgba);
    void setRgba32 (guint32 rgba);

protected:
    virtual void on_size_request (Gtk::Requisition *req);
    virtual void on_size_allocate (Gtk::Allocation &all);
    virtual bool on_expose_event (GdkEventExpose *event);
    void paint (GdkRectangle *area);
   
    guint32 _rgba;
};

}}}

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
