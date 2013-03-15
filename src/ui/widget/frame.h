/*
 * Authors:
 *   Murray C
 *
 * Copyright (C) 2012 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_FRAME_H
#define INKSCAPE_UI_WIDGET_FRAME_H

#include <gtkmm/alignment.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * Creates a Gnome HIG style indented frame with bold label
 * See http://developer.gnome.org/hig-book/stable/controls-frames.html.en
 */
class Frame : public Gtk::Frame
{
public:

    /**
     * Construct a Frame Widget.
     *
     * @param label     The frame text.
     */
    Frame(Glib::ustring const &label = "", gboolean label_bold = TRUE);

    /**
     * Return the label widget
     */
    Gtk::Label const *get_label_widget() const;

    /**
     * Add a widget to this frame
     */
    virtual void add(Widget& widget);

    /**
     * Set the frame label text and if bold or not
     */
    void set_label(const Glib::ustring &label, gboolean label_bold = TRUE);

    /**
     * Set the frame padding
     */
    void set_padding (guint padding_top, guint padding_bottom, guint padding_left, guint padding_right);

protected:
    Gtk::Label   _label;
    Gtk::Alignment _alignment;

};

} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_FRAME_H

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
