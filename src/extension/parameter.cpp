/** \file
 * Parameters for extensions.
 */

/*
 * Author:
 *   Ted Gould <ted@gould.cx>
 *   Johan Engelen <johan@shouraizou.nl>
 *
 * Copyright (C) 2005-2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef linux  // does the dollar sign need escaping when passed as string parameter?
# define ESCAPE_DOLLAR_COMMANDLINE
#endif

#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/spinbutton.h>

#include <xml/node.h>

#include "extension.h"
#include "document-private.h"
#include "sp-object.h"
#include "color.h"
#include "widgets/sp-color-selector.h"
#include "widgets/sp-color-notebook.h"

#include "parameter.h"
#include "parambool.h"
#include "paramcolor.h"
#include "paramdescription.h"
#include "paramenum.h"
#include "paramfloat.h"
#include "paramint.h"
#include "paramnotebook.h"
#include "paramradiobutton.h"
#include "paramstring.h"

namespace Inkscape {
namespace Extension {

class ParamEnum : public Parameter {
private:
    class Choice {
    public:
        gchar * _gui_name;
        gchar * _value;
        Choice(gchar * gui_name, gchar * value) : _gui_name(NULL), _value(NULL) {
            if (gui_name != NULL)
                _gui_name = g_strdup(_(gui_name));
            if (value != NULL)
                _value = g_strdup(value);
            return;
        };
        ~Choice (void) {
            g_free(_gui_name);
            g_free(_value);
        };
    }; /* class Choice */
    /** \brief  Internal value.  This should point to a string that has
                been allocated in memory.  And should be free'd. */
    Choice * _current_choice;
    typedef std::list<Choice *> choice_list_t;
    choice_list_t _choice_list;
public:
    ParamEnum(const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    ~ParamEnum(void);
    /** \brief  Returns \c _value, with a \i const to protect it. */
    const gchar * get (const SPDocument * /*doc*/, const Inkscape::XML::Node * /*node*/) { return _current_choice != NULL ? _current_choice->_value : NULL; }
    const gchar * set (const gchar * in, SPDocument * doc, Inkscape::XML::Node * node);
    Gtk::Widget * get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);
    Glib::ustring * string (void);
}; /* class ParamEnum */

/**
    \return None
    \brief  This function creates a parameter that can be used later.  This
            is typically done in the creation of the extension and defined
            in the XML file describing the extension (it's private so people
            have to use the system) :)
    \param  in_repr  The XML describing the parameter

    This function first grabs all of the data out of the Repr and puts
    it into local variables.  Actually, these are just pointers, and the
    data is not duplicated so we need to be careful with it.  If there
    isn't a name or a type in the XML, then no parameter is created as
    the function just returns.

    From this point on, we're pretty committed as we've allocated an
    object and we're starting to fill it.  The name is set first, and
    is created with a strdup to actually allocate memory for it.  Then
    there is a case statement (roughly because strcmp requires 'ifs')
    based on what type of parameter this is.  Depending which type it
    is, the value is interpreted differently, but they are relatively
    straight forward.  In all cases the value is set to the default
    value from the XML and the type is set to the interpreted type.
*/
Parameter *
Parameter::make (Inkscape::XML::Node * in_repr, Inkscape::Extension::Extension * in_ext)
{
    const char * name;
    const char * type;
    const char * guitext;
    const char * desc;
    const char * scope_str;
    Parameter::_scope_t scope = Parameter::SCOPE_USER;

    name = in_repr->attribute("name");
    type = in_repr->attribute("type");
    guitext = in_repr->attribute("gui-text");
    if (guitext == NULL)
        guitext = in_repr->attribute("_gui-text");
    desc = in_repr->attribute("gui-description");
    if (desc == NULL)
        desc = in_repr->attribute("_gui-description");
    scope_str = in_repr->attribute("scope");

    /* In this case we just don't have enough information */
    if (name == NULL || type == NULL) {
        return NULL;
    }

    if (scope_str != NULL) {
        if (!strcmp(scope_str, "user")) {
            scope = Parameter::SCOPE_USER;
        } else if (!strcmp(scope_str, "document")) {
            scope = Parameter::SCOPE_DOCUMENT;
        } else if (!strcmp(scope_str, "node")) {
            scope = Parameter::SCOPE_NODE;
        }
    }

    Parameter * param = NULL;
    if (!strcmp(type, "boolean")) {
        param = new ParamBool(name, guitext, desc, scope, in_ext, in_repr);
    } else if (!strcmp(type, "int")) {
        param = new ParamInt(name, guitext, desc, scope, in_ext, in_repr);
    } else if (!strcmp(type, "float")) {
        param = new ParamFloat(name, guitext, desc, scope, in_ext, in_repr);
    } else if (!strcmp(type, "string")) {
        param = new ParamString(name, guitext, desc, scope, in_ext, in_repr);
    } else if (!strcmp(type, "description")) {
        param = new ParamDescription(name, guitext, desc, scope, in_ext, in_repr);
    } else if (!strcmp(type, "enum")) {
        param = new ParamComboBox(name, guitext, desc, scope, in_ext, in_repr);
    } else if (!strcmp(type, "notebook")) {
        param = new ParamNotebook(name, guitext, desc, scope, in_ext, in_repr);
    } else if (!strcmp(type, "optiongroup")) {
        param = new ParamRadioButton(name, guitext, desc, scope, in_ext, in_repr);
    } else if (!strcmp(type, "color")) {
        param = new ParamColor(name, guitext, desc, scope, in_ext, in_repr);
    }

    /* Note: param could equal NULL */
    return param;
}



/** \brief  Wrapper to cast to the object and use it's function.  */
bool
Parameter::get_bool (const SPDocument * doc, const Inkscape::XML::Node * node)
{
    ParamBool * boolpntr = dynamic_cast<ParamBool *>(this);
    if (boolpntr == NULL)
        throw Extension::param_not_bool_param();
    return boolpntr->get(doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
int
Parameter::get_int (const SPDocument * doc, const Inkscape::XML::Node * node)
{
    ParamInt * intpntr = dynamic_cast<ParamInt *>(this);
    if (intpntr == NULL)
        throw Extension::param_not_int_param();
    return intpntr->get(doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
float
Parameter::get_float (const SPDocument * doc, const Inkscape::XML::Node * node)
{
    ParamFloat * floatpntr = dynamic_cast<ParamFloat *>(this);
    if (floatpntr == NULL)
        throw Extension::param_not_float_param();
    return floatpntr->get(doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
const gchar *
Parameter::get_string (const SPDocument * doc, const Inkscape::XML::Node * node)
{
    ParamString * stringpntr = dynamic_cast<ParamString *>(this);
    if (stringpntr == NULL)
        throw Extension::param_not_string_param();
    return stringpntr->get(doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
const gchar *
Parameter::get_enum (const SPDocument * doc, const Inkscape::XML::Node * node)
{
    ParamComboBox * param = dynamic_cast<ParamComboBox *>(this);
    if (param == NULL)
        throw Extension::param_not_enum_param();
    return param->get(doc, node);
}

guint32
Parameter::get_color(const SPDocument* doc, const Inkscape::XML::Node* node)
{
    ParamColor* param = dynamic_cast<ParamColor *>(this);
    if (param == NULL)
        throw Extension::param_not_color_param();
    return param->get(doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
bool
Parameter::set_bool (bool in, SPDocument * doc, Inkscape::XML::Node * node)
{
    ParamBool * boolpntr = dynamic_cast<ParamBool *>(this);
    if (boolpntr == NULL)
        throw Extension::param_not_bool_param();
    return boolpntr->set(in, doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
int
Parameter::set_int (int in, SPDocument * doc, Inkscape::XML::Node * node)
{
    ParamInt * intpntr = dynamic_cast<ParamInt *>(this);
    if (intpntr == NULL)
        throw Extension::param_not_int_param();
    return intpntr->set(in, doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
float
Parameter::set_float (float in, SPDocument * doc, Inkscape::XML::Node * node)
{
    ParamFloat * floatpntr;
    floatpntr = dynamic_cast<ParamFloat *>(this);
    if (floatpntr == NULL)
        throw Extension::param_not_float_param();
    return floatpntr->set(in, doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
const gchar *
Parameter::set_string (const gchar * in, SPDocument * doc, Inkscape::XML::Node * node)
{
    ParamString * stringpntr = dynamic_cast<ParamString *>(this);
    if (stringpntr == NULL)
        throw Extension::param_not_string_param();
    return stringpntr->set(in, doc, node);
}
/** \brief  Wrapper to cast to the object and use it's function.  */
guint32
Parameter::set_color (guint32 in, SPDocument * doc, Inkscape::XML::Node * node)
{
    ParamColor* param = dynamic_cast<ParamColor *>(this);
    if (param == NULL)
        throw Extension::param_not_color_param();
    return param->set(in, doc, node);
}


/** \brief  Oop, now that we need a parameter, we need it's name.  */
Parameter::Parameter (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext) :
    extension(ext), _name(NULL), _desc(NULL), _scope(scope), _text(NULL)
{
    if (name != NULL)
        _name = g_strdup(name);
    if (desc != NULL) {
        _desc = g_strdup(desc);
        // printf("Adding description: '%s' on '%s'\n", _desc, _name);
    }


    if (guitext != NULL)
        _text = g_strdup(guitext);
    else
        _text = g_strdup(name);

    return;
}

/** \brief  Just free the allocated name. */
Parameter::~Parameter (void)
{
    g_free(_name);
    g_free(_text);
}

/** \brief  Build the name to write the parameter from the extension's
            ID and the name of this parameter. */
gchar *
Parameter::pref_name (void)
{
    return g_strdup_printf("%s.%s", extension->get_id(), _name);
}

Inkscape::XML::Node *
Parameter::find_child (Inkscape::XML::Node * adult)
{
    return sp_repr_lookup_child(adult, "name", _name);
}

Inkscape::XML::Node *
Parameter::new_child (Inkscape::XML::Node * parent)
{
    Inkscape::XML::Node * retval;
    retval = parent->document()->createElement("inkscape:extension-param");
    retval->setAttribute("name", _name);

    parent->appendChild(retval);
    return retval;
}

Inkscape::XML::Node *
Parameter::document_param_node (SPDocument * doc)
{
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);
    Inkscape::XML::Node * defs = SP_OBJECT_REPR(SP_DOCUMENT_DEFS(doc));
    Inkscape::XML::Node * params = NULL;

    GQuark const name_quark = g_quark_from_string("inkscape:extension-params");

    for (Inkscape::XML::Node * child = defs->firstChild();
            child != NULL;
            child = child->next()) {
        if ((GQuark)child->code() == name_quark &&
                !strcmp(child->attribute("extension"), extension->get_id())) {
            params = child;
            break;
        }
    }

    if (params == NULL) {
        params = xml_doc->createElement("inkscape:extension-param");
        params->setAttribute("extension", extension->get_id());
        defs->appendChild(params);
    }

    return params;
}

/** \brief  Basically, if there is no widget pass a NULL. */
Gtk::Widget *
Parameter::get_widget (SPDocument * /*doc*/, Inkscape::XML::Node * /*node*/, sigc::signal<void> * /*changeSignal*/)
{
    return NULL;
}

/** \brief  If I'm not sure which it is, just don't return a value. */
Glib::ustring *
Parameter::string (void)
{
    Glib::ustring * mystring = new Glib::ustring("");
    return mystring;
}





ParamEnum::ParamEnum (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * /*xml*/) :
    Parameter(name, guitext, desc, scope, ext), _current_choice(NULL)
{
    return;
}

ParamEnum::~ParamEnum (void)
{

}

/** \brief  Return the value as a string */
Glib::ustring *
ParamEnum::string (void)
{
    Glib::ustring * mystring = new Glib::ustring("");
    *mystring += this->get(NULL, NULL);
    return mystring;
}

Gtk::Widget *
ParamEnum::get_widget (SPDocument * /*doc*/, Inkscape::XML::Node * /*node*/, sigc::signal<void> * /*changeSignal*/)
{
    return NULL;
}

const gchar *
ParamEnum::set (const gchar * /*in*/, SPDocument * /*doc*/, Inkscape::XML::Node * /*node*/)
{
    return NULL;
}


}  /* namespace Extension */
}  /* namespace Inkscape */

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
