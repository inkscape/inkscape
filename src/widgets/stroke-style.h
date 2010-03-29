/** @file
 * @brief  Stroke style dialog
 */
/* Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2010 Jon A. Cruz
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_DIALOGS_STROKE_STYLE_H
#define SEEN_DIALOGS_STROKE_STYLE_H

namespace Gtk {
class Widget;
class Container;
}

Gtk::Widget *sp_stroke_style_paint_widget_new(void);
Gtk::Container *sp_stroke_style_line_widget_new(void);

void sp_stroke_style_widget_set_desktop(Gtk::Widget *widget, SPDesktop *desktop);

#endif // SEEN_DIALOGS_STROKE_STYLE_H

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
