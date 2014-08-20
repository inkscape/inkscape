#ifndef SEEN_FILL_N_STROKE_FACTORY_H
#define SEEN_FILL_N_STROKE_FACTORY_H
/* Authors:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2010 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "fill-or-stroke.h"

namespace Gtk {
class Widget;
}

namespace Inkscape {
namespace Widgets {

Gtk::Widget *createStyleWidget( FillOrStroke kind );
Gtk::Widget *createStrokeStyleWidget( );

} // namespace Widgets
} // namespace Inkscape

#endif // !SEEN_FILL_N_STROKE_FACTORY_H

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
