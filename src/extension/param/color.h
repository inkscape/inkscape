#ifndef __INK_EXTENSION_PARAMCOLOR_H__
#define __INK_EXTENSION_PARAMCOLOR_H__
/*
 * Copyright (C) 2005-2007 Authors:
 *   Ted Gould <ted@gould.cx>
 *   Johan Engelen <johan@shouraizou.nl> *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/widget.h>
#include "xml/node.h"
#include "document.h"
#include <color.h>
#include "extension/param/parameter.h"

namespace Inkscape {
namespace Extension {

class ParamColor : public Parameter {
private:
    guint32 _value;
public:
    ParamColor(const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, bool gui_hidden, const gchar * gui_tip, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    virtual ~ParamColor(void);
    /** \brief  Returns \c _value, with a \i const to protect it. */
    guint32 get( const SPDocument * /*doc*/, const Inkscape::XML::Node * /*node*/ ) { return _value; }
    guint32 set (guint32 in, SPDocument * doc, Inkscape::XML::Node * node);
    Gtk::Widget * get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);
    void string (std::string &string);
    sigc::signal<void> * _changeSignal;
}; /* class ParamColor */

}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* __INK_EXTENSION_PARAMCOLOR_H__ */
