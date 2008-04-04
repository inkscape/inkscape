#ifndef INK_EXTENSION_PARAMNOTEBOOK_H_SEEN
#define INK_EXTENSION_PARAMNOTEBOOK_H_SEEN

/** \file
 * Notebook parameter for extensions.
 */

/*
 * Author:
 *   Johan Engelen <johan@shouraizou.nl>
 *
 * Copyright (C) 2006 Author
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
class ParamNotebook : public Parameter {
private:
    /** \brief  Internal value.  This should point to a string that has
                been allocated in memory.  And should be free'd.
                It is the name of the current page. */
    gchar * _value;

    GSList * pages; /**< A table to store the pages with parameters for this notebook.
                              This only gets created if there are pages in this
                              notebook */
public:
    ParamNotebook(const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, bool gui_hidden, const gchar * gui_tip, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    virtual ~ParamNotebook(void);
    Gtk::Widget * get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);
    void string (std::list <std::string> &list);

    const gchar * get (const SPDocument * /*doc*/, const Inkscape::XML::Node * /*node*/) { return _value; }
    const gchar * set (const int in, SPDocument * doc, Inkscape::XML::Node * node);
}; /* class ParamNotebook */





}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* INK_EXTENSION_PARAMNOTEBOOK_H_SEEN */

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
