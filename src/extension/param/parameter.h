#ifndef __INK_EXTENSION_PARAM_H__
#define __INK_EXTENSION_PARAM_H__

/** \file
 * Parameters for extensions.
 */

/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2005-2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/** \brief  The root directory in the preferences database for extension-related parameters. */
#define PREF_DIR "extensions"

#include <gtkmm/widget.h>

#include "xml/document.h"
#include "xml/node.h"
#include "document.h"
#include <extension/extension-forward.h>
#include "prefs-utils.h"

#include <glibmm/i18n.h>

#include <color.h>

namespace Inkscape {
namespace Extension {

/** \brief  A class to represent the parameter of an extension

    This is really a super class that allows them to abstract all
    the different types of parameters into some that can be passed
    around.  There is also a few functions that are used by all the
    different parameters.
*/
class Parameter {
private:
    /** \brief  Which extension is this parameter attached to? */
    Inkscape::Extension::Extension * extension;
    /** \brief  The name of this parameter. */
    gchar *       _name;

protected:
    /** \brief  Description of the parameter. */
    gchar *       _desc;
    /** \brief  List of possible scopes. */
    typedef enum {
        SCOPE_USER,     /**<  Parameter value is saved in the user's configuration file. (default) */
        SCOPE_DOCUMENT, /**<  Parameter value is saved in the document. */
        SCOPE_NODE      /**<  Parameter value is attached to the node. */
    } _scope_t;
    /** \brief  Scope of the parameter. */
    _scope_t _scope;
    /** \brief  Text for the GUI selection of this. */
    gchar *  _text;
    /** \brief  Whether the GUI is visible */
    bool _gui_hidden;
    /** \brief  A tip for the GUI if there is one */
    gchar *  _gui_tip;


    /* **** funcs **** */
    gchar *               pref_name (void);
    Inkscape::XML::Node * find_child (Inkscape::XML::Node * adult);
    Inkscape::XML::Node * document_param_node (SPDocument * doc);
    Inkscape::XML::Node * new_child (Inkscape::XML::Node * parent);

public:
                  Parameter  (const gchar * name,
                              const gchar * guitext,
                              const gchar * desc,
                              const Parameter::_scope_t scope,
                              bool gui_hidden,
                              const gchar * gui_tip,
                              Inkscape::Extension::Extension * ext);
                  Parameter  (const gchar * name,
                              const gchar * guitext,
                              Inkscape::Extension::Extension * ext) {
                      Parameter(name, guitext, NULL, Parameter::SCOPE_USER, false, NULL, ext);
                  };
    virtual      ~Parameter  (void);
    bool          get_bool   (const SPDocument * doc,
                              const Inkscape::XML::Node * node);
    int           get_int    (const SPDocument * doc,
                              const Inkscape::XML::Node * node);
    float         get_float  (const SPDocument * doc,
                              const Inkscape::XML::Node * node);
    const gchar * get_string (const SPDocument * doc,
                              const Inkscape::XML::Node * node);
    guint32       get_color  (const SPDocument * doc,
                              const Inkscape::XML::Node * node);
    const gchar * get_enum   (const SPDocument * doc,
                              const Inkscape::XML::Node * node);

    bool          set_bool   (bool in,          SPDocument * doc, Inkscape::XML::Node * node);
    int           set_int    (int  in,          SPDocument * doc, Inkscape::XML::Node * node);
    float         set_float  (float in,         SPDocument * doc, Inkscape::XML::Node * node);
    const gchar * set_string (const gchar * in, SPDocument * doc, Inkscape::XML::Node * node);
    guint32       set_color  (guint32 in, SPDocument * doc, Inkscape::XML::Node * node);

    const gchar * name       (void) {return _name;}

    static Parameter * make (Inkscape::XML::Node * in_repr, Inkscape::Extension::Extension * in_ext);
    virtual Gtk::Widget * get_widget (SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);

    gchar const * get_tooltip (void) { return _desc; }

    /** \brief  Indicates if the GUI for this parameter is hidden or not */
    bool         get_gui_hidden ()   { return _gui_hidden; }

    virtual void string (std::list <std::string> &list);
    virtual void string (std::string &string);
};

}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* __INK_EXTENSION_PARAM_H__ */

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
