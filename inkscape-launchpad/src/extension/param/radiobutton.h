#ifndef INK_EXTENSION_PARAMRADIOBUTTON_H_SEEN
#define INK_EXTENSION_PARAMRADIOBUTTON_H_SEEN

/** \file
 * Radiobutton parameter for extensions.
 */

/*
 * Authors:
 *   Johan Engelen <johan@shouraizou.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2006-2007 Johan Engelen
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "parameter.h"

namespace Gtk {
class Widget;
}

namespace Inkscape {
namespace Extension {

class Extension;


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

    // Explicitly call superclass version to avoid method being hidden.
    virtual void string(std::list <std::string> &list) const { return Parameter::string(list); }

    virtual void string(std::string &string) const;

    Glib::ustring value_from_label(const Glib::ustring label);

    const gchar *get(const SPDocument * /*doc*/, const Inkscape::XML::Node * /*node*/) const { return _value; }

    const gchar *set(const gchar *in, SPDocument *doc, Inkscape::XML::Node *node);

private:
    /** \brief  Internal value.  This should point to a string that has
                been allocated in memory.  And should be free'd.
                It is the value of the current selected string */
    gchar * _value;
    AppearanceMode _mode;
    int _indent;
    GSList * choices; /**< A table to store the choice strings  */

}; /* class ParamRadioButton */





}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* INK_EXTENSION_PARAMRADIOBUTTON_H_SEEN */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
