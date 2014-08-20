#ifndef INK_EXTENSION_PARAMENUM_H_SEEN
#define INK_EXTENSION_PARAMENUM_H_SEEN

/** \file
 * Enumeration parameter for extensions.
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


// \brief  A class to represent a notebookparameter of an extension
class ParamComboBox : public Parameter {
private:
    /** \brief  Internal value.  This should point to a string that has
                been allocated in memory.  And should be free'd.
                It is the value of the current selected string */
    gchar * _value;
    int _indent;
    GSList * choices; /**< A table to store the choice strings  */

public:
    ParamComboBox(const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, bool gui_hidden, const gchar * gui_tip, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    virtual ~ParamComboBox(void);
    Gtk::Widget * get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);

    // Explicitly call superclass version to avoid method being hidden.
    virtual void string(std::list <std::string> &list) const { return Parameter::string(list); }

    virtual void string(std::string &string) const;

    gchar const *get(SPDocument const * /*doc*/, Inkscape::XML::Node const * /*node*/) const { return _value; }

    const gchar * set (const gchar * in, SPDocument * doc, Inkscape::XML::Node * node);

    /**
     * @returns true if guitext is part of this enum
     */
    bool contains(const gchar * guitext, SPDocument const * /*doc*/, Inkscape::XML::Node const * /*node*/) const;

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
