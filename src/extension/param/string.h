#ifndef INK_EXTENSION_PARAMSTRING_H_SEEN
#define INK_EXTENSION_PARAMSTRING_H_SEEN

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

class ParamString : public Parameter {
private:
    /** \brief  Internal value.  This should point to a string that has
                been allocated in memory.  And should be free'd. */
    gchar * _value;
public:
    ParamString(const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, bool gui_hidden, const gchar * gui_tip, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    virtual ~ParamString(void);
    /** \brief  Returns \c _value, with a \i const to protect it. */
    const gchar * get (const SPDocument * /*doc*/, const Inkscape::XML::Node * /*node*/) { return _value; }
    const gchar * set (const gchar * in, SPDocument * doc, Inkscape::XML::Node * node);
    Gtk::Widget * get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);
    void string (std::string &string);
};


}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* INK_EXTENSION_PARAMSTRING_H_SEEN */

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
