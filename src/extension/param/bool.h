#ifndef __INK_EXTENSION_PARAMBOOL_H__
#define __INK_EXTENSION_PARAMBOOL_H__
/*
 * Copyright (C) 2005-2007 Authors:
 *   Ted Gould <ted@gould.cx>
 *   Johan Engelen <johan@shouraizou.nl> *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/widget.h>
#include <xml/node.h>
#include <document.h>
#include "parameter.h"

namespace Inkscape {
namespace Extension {

/** \brief  A boolean parameter */
class ParamBool : public Parameter {
private:
    /** \brief  Internal value. */
    bool _value;
public:
    ParamBool(const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, bool gui_hidden, const gchar * gui_tip, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    bool get (const Document * doc, const Inkscape::XML::Node * node);
    bool set (bool in, Document * doc, Inkscape::XML::Node * node);
    Gtk::Widget * get_widget(Document * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);
    void string (std::string &string);
};

}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* __INK_EXTENSION_PARAMBOOL_H__ */

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
