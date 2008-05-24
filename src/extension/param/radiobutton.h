#ifndef INK_EXTENSION_PARAMRADIOBUTTON_H_SEEN
#define INK_EXTENSION_PARAMRADIOBUTTON_H_SEEN

/** \file
 * Radiobutton parameter for extensions.
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



// \brief  A class to represent a radiobutton parameter of an extension
class ParamRadioButton : public Parameter {
public:
    enum AppearanceMode {
        FULL, COMPACT, MINIMAL
    };

    ParamRadioButton(const gchar * name,
                     const gchar * guitext,
                     const gchar * desc,
                     const Parameter::_scope_t scope,
                     bool gui_hidden,
                     const gchar * gui_tip,
                     Inkscape::Extension::Extension * ext,
                     Inkscape::XML::Node * xml,
                     AppearanceMode mode);
    virtual ~ParamRadioButton(void);
    Gtk::Widget * get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);
    void string (std::string &string);

    const gchar * get (const SPDocument * /*doc*/, const Inkscape::XML::Node * /*node*/) { return _value; }
    const gchar * set (const gchar * in, SPDocument * doc, Inkscape::XML::Node * node);

private:
    /** \brief  Internal value.  This should point to a string that has
                been allocated in memory.  And should be free'd.
                It is the value of the current selected string */
    gchar * _value;
    AppearanceMode _mode;

    GSList * choices; /**< A table to store the choice strings  */

}; /* class ParamRadioButton */





}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* INK_EXTENSION_PARAMRADIOBUTTON_H_SEEN */

