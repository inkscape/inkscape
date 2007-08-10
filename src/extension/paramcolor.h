#ifndef __INK_EXTENSION_PARAMCOLOR_H__
#define __INK_EXTENSION_PARAMCOLOR_H__
/*
 * Copyright (C) 2005-2007 Authors:
 *   Ted Gould <ted@gould.cx>
 *   Johan Engelen <johan@shouraizou.nl> *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/widget.h>
#include <xml/node.h>
#include <document.h>
#include "color.h"
#include "parameter.h"

namespace Inkscape {
namespace Extension {

class ParamColor : public Parameter {
private:
    SPColor* _value;
public:
    ParamColor(const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    ~ParamColor(void);
    /** \brief  Returns \c _value, with a \i const to protect it. */
    SPColor* get (const SPDocument * doc, const Inkscape::XML::Node * node) { return _value; }
    SPColor* set (SPColor* in, SPDocument * doc, Inkscape::XML::Node * node);
    Gtk::Widget * get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);
    Glib::ustring * string (void);
	sigc::signal<void> * _changeSignal;
}; /* class ParamColor */

}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* __INK_EXTENSION_PARAMCOLOR_H__ */

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
