#ifndef __INK_EXTENSION_PARAMDESCRIPTION_H__
#define __INK_EXTENSION_PARAMDESCRIPTION_H__

/*
 * Copyright (C) 2005-2007 Authors:
 *   Ted Gould <ted@gould.cx>
 *   Johan Engelen <johan@shouraizou.nl> *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "parameter.h"

class SPDocument;

namespace Gtk {
	class Widget;
}

namespace Inkscape {
namespace Xml {
	class Node;
}

namespace Extension {

/** \brief  A description parameter */
class ParamDescription : public Parameter {
public:
    enum AppearanceMode {
        DESC, HEADER
    };
    ParamDescription(const gchar * name,
                     const gchar * guitext,
                     const gchar * desc,
                     const Parameter::_scope_t scope,
                     bool gui_hidden,
                     const gchar * gui_tip, 
                     Inkscape::Extension::Extension * ext,
                     Inkscape::XML::Node * xml,
                     AppearanceMode mode);
    Gtk::Widget * get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);
private:
    /** \brief  Internal value. */
    gchar * _value;
    AppearanceMode _mode;
    int _indent;
    const gchar* _context;
};

}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* __INK_EXTENSION_PARAMDESCRIPTION_H__ */

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
