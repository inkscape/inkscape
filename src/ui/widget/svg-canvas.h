#ifndef SEEN_UI_WIDGET_SVGCANVAS_H
#define SEEN_UI_WIDGET_SVGCANVAS_H

/*
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005 The Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

struct SPCanvas;
struct SPDesktop;
namespace Gtk { class Widget; }
namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Gtkmm facade/wrapper around SPCanvas.
 */
class SVGCanvas
{
public:
    SVGCanvas();
    ~SVGCanvas();
    void init (SPDesktop*);
    Gtk::Widget& widget() const { return *_widget; }
    SPCanvas* spobj() const { return _spcanvas; }

protected:
    Gtk::Widget *_widget;
    SPCanvas *_spcanvas;
    SPDesktop *_dt;

    bool onEvent (GdkEvent *) const;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape


#endif // SEEN_UI_WIDGET_SVGCANVAS_H


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
