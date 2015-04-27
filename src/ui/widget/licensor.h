/*
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_LICENSOR_H
#define INKSCAPE_UI_WIDGET_LICENSOR_H

#include <gtkmm/box.h>

class SPDocument;

namespace Inkscape {
    namespace UI {
        namespace Widget {

class EntityEntry;
class Registry;


/**
 * Widget for specifying a document's license; part of document
 * preferences dialog.
 */
class Licensor : public Gtk::VBox {
public:
    Licensor();
    virtual ~Licensor();
    void init (Registry&);
    void update (SPDocument *doc);

protected: 
    EntityEntry          *_eentry;
};


} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_LICENSOR_H

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
