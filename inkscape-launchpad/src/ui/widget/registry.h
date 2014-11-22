/*
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */
#ifndef INKSCAPE_UI_WIDGET_REGISTRY__H
#define INKSCAPE_UI_WIDGET_REGISTRY__H

namespace Gtk {
    class Object;
}

namespace Inkscape {
namespace UI {
namespace Widget {
   
class Registry {
public:
    Registry();
    ~Registry();
    
    bool               isUpdating();
    void               setUpdating (bool);

protected:
    bool _updating;
};

} // namespace Dialog
} // namespace UI
} // namespace Widget

#endif // INKSCAPE_UI_WIDGET_REGISTRY__H

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
