#ifndef INK_EXTENSION_PARAMINT_H_SEEN
#define INK_EXTENSION_PARAMINT_H_SEEN

/*
 * Copyright (C) 2005-2007 Authors:
 *   Ted Gould <ted@gould.cx>
 *   Johan Engelen <johan@shouraizou.nl> *
 *   Jon A. Cruz <jon@joncruz.org>
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "parameter.h"

class SPDocument;

namespace Gtk {
class Widget;
}

namespace Inkscape {
namespace XML {
class Node;
}

namespace Extension {

class ParamInt : public Parameter {
public:
    enum AppearanceMode {
        FULL, MINIMAL
    };
    ParamInt (const gchar * name,
              const gchar * guitext,
              const gchar * desc,
              const Parameter::_scope_t scope,
              bool gui_hidden,
              const gchar * gui_tip,
              Inkscape::Extension::Extension * ext,
              Inkscape::XML::Node * xml,
              AppearanceMode mode);

    /** Returns \c _value. */
    int get(const SPDocument * /*doc*/, const Inkscape::XML::Node * /*node*/) const { return _value; }

    int set (int in, SPDocument * doc, Inkscape::XML::Node * node);

    int max (void) { return _max; }

    int min (void) { return _min; }

    Gtk::Widget * get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);

    // Explicitly call superclass version to avoid method being hidden.
    virtual void string(std::list <std::string> &list) const { return Parameter::string(list); }

    virtual void string(std::string &string) const;

private:
    /** Internal value. */
    int _value;
    AppearanceMode _mode;
    int _indent;
    int _min;
    int _max;
};

}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* INK_EXTENSION_PARAMINT_H_SEEN */

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
