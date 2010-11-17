/** @file
 * @brief  Fill style configuration
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2010 Jon A. Cruz
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_DIALOGS_SP_FILL_STYLE_H
#define SEEN_DIALOGS_SP_FILL_STYLE_H

namespace Gtk {
class Widget;
}

class SPDesktop;

Gtk::Widget *sp_fill_style_widget_new(void);

void sp_fill_style_widget_set_desktop(Gtk::Widget *widget, SPDesktop *desktop);

#endif // SEEN_DIALOGS_SP_FILL_STYLE_H

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
