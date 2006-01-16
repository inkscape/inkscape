/** \file
 * \brief Widget for specifying page size; part of Document Preferences dialog.
 *
 * Author:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_PAGE_SIZER__H
#define INKSCAPE_UI_WIDGET_PAGE_SIZER__H

#include <sigc++/sigc++.h>
#include <gtkmm/box.h>
#include "ui/widget/registry.h"
#include "ui/widget/registered-widget.h"

namespace Gtk {
    class OptionMenu;
    class RadioButton;
}

namespace Inkscape {    
    namespace UI {
        namespace Widget {

/// Widget containing all widgets for specifying page size.
class PageSizer : public Gtk::VBox {
public:
    PageSizer();
    virtual ~PageSizer();
    void init (Registry& reg);
    void setDim (double w, double h, bool update = false);
    bool                 _landscape;

protected:
    void setDoc (double w, double h);
    int find_paper_size (double w, double h) const;
    void on_portrait();
    void on_landscape();
    void on_value_changed();
    
    RegisteredUnitMenu   _rum;
    RegisteredScalarUnit _rusw, _rush;
    Gtk::OptionMenu     *_omenu_size;
    Gtk::RadioButton    *_rb_port, *_rb_land;
    sigc::connection     _portrait_connection, _landscape_connection;
    sigc::connection     _changedw_connection, _changedh_connection;
    Registry             *_wr;
};

}}}

#endif /* INKSCAPE_UI_WIDGET_PAGE_SIZER__H */

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
