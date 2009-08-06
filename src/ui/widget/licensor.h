/** \file
 * \brief Widget for specifying a document's license; part of document
 * preferences dialog.
 *
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_LICENSOR__H
#define INKSCAPE_UI_WIDGET_LICENSOR__H

#include <gtkmm/box.h>

class SPDocument;

namespace Gtk {
    class Tooltips;
}

namespace Inkscape {
    namespace UI {
        namespace Widget {

class EntityEntry;
class Registry;


class Licensor : public Gtk::VBox {
public:
    Licensor();
    virtual ~Licensor();
    void init (Gtk::Tooltips&, Registry&);
    void update (SPDocument *doc);

protected: 
    EntityEntry          *_eentry;
};


} // namespace Widget
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_WIDGET_LICENSOR__H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
