#ifndef INK_EXTENSION_PARAMENUM_H_SEEN
#define INK_EXTENSION_PARAMENUM_H_SEEN

/** \file
 * Enumeration parameter for extensions.
 */

/*
 * Author:
 *   Johan Engelen <johan@shouraizou.nl>
 *
 * Copyright (C) 2006-2007 Johan Engelen
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/widget.h>

#include "xml/document.h"
#include <extension/extension-forward.h>

#include "parameter.h"

namespace Inkscape {
namespace Extension {



// \brief  A class to represent a notebookparameter of an extension
class ParamComboBox : public Parameter {
private:
    /** \brief  Internal value.  This should point to a string that has
                been allocated in memory.  And should be free'd.
                It is the value of the current selected string */
    gchar * _value;

    GSList * choices; /**< A table to store the choice strings  */

public:
    ParamComboBox(const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, bool gui_hidden, const gchar * gui_tip, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    virtual ~ParamComboBox(void);
    Gtk::Widget * get_widget(Document * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);
    void string (std::string &string);

    const gchar * get (const Document * /*doc*/, const Inkscape::XML::Node * /*node*/) { return _value; }
    const gchar * set (const gchar * in, Document * doc, Inkscape::XML::Node * node);

    void changed (void);
}; /* class ParamComboBox */





}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* INK_EXTENSION_PARAMENUM_H_SEEN */

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
