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

#include <glibmm/i18n.h>

#include <xml/node.h>

#include "extension.h"
#include "prefs-utils.h"
#include "document-private.h"
#include "sp-object.h"

#include "parameter.h"
#include "paramnotebook.h"
#include "paramenum.h"
#include "paramradiobutton.h"

/** \brief  The root directory in the preferences database for extension
            related parameters. */
#define PREF_DIR "extensions"

namespace Inkscape {
namespace Extension {

/** \brief  A description parameter */
class ParamDescription : public Parameter {
private:
    /** \brief  Internal value. */
    gchar * _value;
public:
    ParamDescription(const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    Gtk::Widget * get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);
};

/** \brief  A boolean parameter */
class ParamBool : public Parameter {
private:
    /** \brief  Internal value. */
    bool _value;
public:
    ParamBool(const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    /** \brief  Returns \c _value */
    bool get (const SPDocument * doc, const Inkscape::XML::Node * node) { return _value; }
    bool set (bool in, SPDocument * doc, Inkscape::XML::Node * node);
    Gtk::Widget * get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);
    Glib::ustring * string (void);
};

/** \brief  Use the superclass' allocator and set the \c _value */
ParamBool::ParamBool (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
        Parameter(name, guitext, desc, scope, ext), _value(false)
{
    const char * defaultval = NULL;
    if (sp_repr_children(xml) != NULL)
        defaultval = sp_repr_children(xml)->content();

    if (defaultval != NULL && (!strcmp(defaultval, "TRUE") || !strcmp(defaultval, "true") || !strcmp(defaultval, "1"))) {
        _value = true;
    } else {
        _value = false;
    }

    gchar * pref_name = this->pref_name();
    _value = (bool)prefs_get_int_attribute(PREF_DIR, pref_name, _value);
    g_free(pref_name);

    return;
}

class ParamInt : public Parameter {
private:
    /** \brief  Internal value. */
    int _value;
    int _min;
    int _max;
public:
    ParamInt (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    /** \brief  Returns \c _value */
    int get (const SPDocument * doc, const Inkscape::XML::Node * node) { return _value; }
    int set (int in, SPDocument * doc, Inkscape::XML::Node * node);
    int max (void) { return _max; }
    int min (void) { return _min; }
    Gtk::Widget * get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);
    Glib::ustring * string (void);
};

/** \brief  Use the superclass' allocator and set the \c _value */
ParamInt::ParamInt (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
        Parameter(name, guitext, desc, scope, ext), _value(0), _min(0), _max(10)
{
    const char * defaultval = NULL;
    if (sp_repr_children(xml) != NULL)
        defaultval = sp_repr_children(xml)->content();
    if (defaultval != NULL) {
        _value = atoi(defaultval);
    }

    const char * maxval = xml->attribute("max");
    if (maxval != NULL)
        _max = atoi(maxval);

    const char * minval = xml->attribute("min");
    if (minval != NULL)
        _min = atoi(minval);

    /* We're handling this by just killing both values */
    if (_max < _min) {
        _max = 10;
        _min = 0;
    }

    gchar * pref_name = this->pref_name();
    _value = prefs_get_int_attribute(PREF_DIR, pref_name, _value);
    g_free(pref_name);

    // std::cout << "New Int::  value: " << _value << "  max: " << _max << "  min: " << _min << std::endl;

    if (_value > _max) _value = _max;
    if (_value < _min) _value = _min;

    return;
}

class ParamFloat : public Parameter {
private:
    /** \brief  Internal value. */
    float _value;
    float _min;
    float _max;
public:
    ParamFloat (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    /** \brief  Returns \c _value */
    float get (const SPDocument * doc, const Inkscape::XML::Node * node) { return _value; }
    float set (float in, SPDocument * doc, Inkscape::XML::Node * node);
    float max (void) { return _max; }
    float min (void) { return _min; }
    Gtk::Widget * get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);
    Glib::ustring * string (void);
};

/** \brief  Use the superclass' allocator and set the \c _value */
ParamFloat::ParamFloat (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
        Parameter(name, guitext, desc, scope, ext), _value(0.0), _min(0.0), _max(10.0)
{
    const char * defaultval = NULL;
    if (sp_repr_children(xml) != NULL)
        defaultval = sp_repr_children(xml)->content();
    if (defaultval != NULL) {
        _value = atof(defaultval);
    }

    const char * maxval = xml->attribute("max");
    if (maxval != NULL)
        _max = atof(maxval);

    const char * minval = xml->attribute("min");
    if (minval != NULL)
        _min = atof(minval);

    /* We're handling this by just killing both values */
    if (_max < _min) {
        _max = 10.0;
        _min = 0.0;
    }

    gchar * pref_name = this->pref_name();
    _value = prefs_get_double_attribute(PREF_DIR, pref_name, _value);
    g_free(pref_name);

    // std::cout << "New Float::  value: " << _value << "  max: " << _max << "  min: " << _min << std::endl;

    if (_value > _max) _value = _max;
    if (_value < _min) _value = _min;

    return;
}

class ParamString : public Parameter {
private:
    /** \brief  Internal value.  This should point to a string that has
                been allocated in memory.  And should be free'd. */
    gchar * _value;
public:
    ParamString(const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml);
    ~ParamString(void);
    /** \brief  Returns \c _value, with a \i const to protect it. */
    const gchar * get (const SPDocument * doc, const Inkscape::XML::Node * node) { return _value; }
    const gchar * set (const gchar * in, SPDocument * doc, Inkscape::XML::Node * node);
    Gtk::Widget * get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal);
    Glib::ustring * string (void);
};

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
    const gchar * get (const SPDocument * doc, const Inkscape::XML::Node * node) { return _current_choice != NULL ? _current_choice->_value : NULL; }
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
    }

    /* Note: param could equal NULL */
    return param;
}

/** \brief  A function to set the \c _value
    \param  in   The value to set to
    \param  doc  A document that should be used to set the value.
    \param  node The node where the value may be placed

    This function sets the internal value, but it also sets the value
    in the preferences structure.  To put it in the right place, \c PREF_DIR
    and \c pref_name() are used.
*/
bool
ParamBool::set (bool in, SPDocument * doc, Inkscape::XML::Node * node)
{
    _value = in;

    gchar * prefname = this->pref_name();
    prefs_set_int_attribute(PREF_DIR, prefname, _value == true ? 1 : 0);
    g_free(prefname);

    return _value;
}

/** \brief  A function to set the \c _value
    \param  in   The value to set to
    \param  doc  A document that should be used to set the value.
    \param  node The node where the value may be placed

    This function sets the internal value, but it also sets the value
    in the preferences structure.  To put it in the right place, \c PREF_DIR
    and \c pref_name() are used.
*/
int
ParamInt::set (int in, SPDocument * doc, Inkscape::XML::Node * node)
{
    _value = in;
    if (_value > _max) _value = _max;
    if (_value < _min) _value = _min;

    gchar * prefname = this->pref_name();
    prefs_set_int_attribute(PREF_DIR, prefname, _value);
    g_free(prefname);

    return _value;
}

/** \brief  A function to set the \c _value
    \param  in   The value to set to
    \param  doc  A document that should be used to set the value.
    \param  node The node where the value may be placed

    This function sets the internal value, but it also sets the value
    in the preferences structure.  To put it in the right place, \c PREF_DIR
    and \c pref_name() are used.
*/
float
ParamFloat::set (float in, SPDocument * doc, Inkscape::XML::Node * node)
{
    _value = in;
    if (_value > _max) _value = _max;
    if (_value < _min) _value = _min;

    gchar * prefname = this->pref_name();
    prefs_set_double_attribute(PREF_DIR, prefname, _value);
    g_free(prefname);

    return _value;
}

/** \brief  A function to set the \c _value
    \param  in   The value to set to
    \param  doc  A document that should be used to set the value.
    \param  node The node where the value may be placed

    This function sets the internal value, but it also sets the value
    in the preferences structure.  To put it in the right place, \c PREF_DIR
    and \c pref_name() are used.

    To copy the data into _value the old memory must be free'd first.
    It is important to note that \c g_free handles \c NULL just fine.  Then
    the passed in value is duplicated using \c g_strdup().
*/
const gchar *
ParamString::set (const gchar * in, SPDocument * doc, Inkscape::XML::Node * node)
{
    if (in == NULL) return NULL; /* Can't have NULL string */

    if (_value != NULL)
        g_free(_value);
    _value = g_strdup(in);

    gchar * prefname = this->pref_name();
    prefs_set_string_attribute(PREF_DIR, prefname, _value);
    g_free(prefname);

    return _value;
}

/** \brief  Wrapper to cast to the object and use it's function.  */
bool
Parameter::get_bool (const SPDocument * doc, const Inkscape::XML::Node * node)
{
    ParamBool * boolpntr;
    boolpntr = dynamic_cast<ParamBool *>(this);
    if (boolpntr == NULL)
        throw Extension::param_wrong_type();
    return boolpntr->get(doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
int
Parameter::get_int (const SPDocument * doc, const Inkscape::XML::Node * node)
{
    ParamInt * intpntr;
    intpntr = dynamic_cast<ParamInt *>(this);
    if (intpntr == NULL)
        throw Extension::param_wrong_type();
    return intpntr->get(doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
float
Parameter::get_float (const SPDocument * doc, const Inkscape::XML::Node * node)
{
    ParamFloat * floatpntr;
    floatpntr = dynamic_cast<ParamFloat *>(this);
    if (floatpntr == NULL)
        throw Extension::param_wrong_type();
    return floatpntr->get(doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
const gchar *
Parameter::get_string (const SPDocument * doc, const Inkscape::XML::Node * node)
{
    ParamString * stringpntr;
    stringpntr = dynamic_cast<ParamString *>(this);
    if (stringpntr == NULL)
        throw Extension::param_wrong_type();
    return stringpntr->get(doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
bool
Parameter::set_bool (bool in, SPDocument * doc, Inkscape::XML::Node * node)
{
    ParamBool * boolpntr;
    boolpntr = dynamic_cast<ParamBool *>(this);
    if (boolpntr == NULL)
        throw Extension::param_wrong_type();
    return boolpntr->set(in, doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
int
Parameter::set_int (int in, SPDocument * doc, Inkscape::XML::Node * node)
{
    ParamInt * intpntr;
    intpntr = dynamic_cast<ParamInt *>(this);
    if (intpntr == NULL)
        throw Extension::param_wrong_type();
    return intpntr->set(in, doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
float
Parameter::set_float (float in, SPDocument * doc, Inkscape::XML::Node * node)
{
    ParamFloat * floatpntr;
    floatpntr = dynamic_cast<ParamFloat *>(this);
    if (floatpntr == NULL)
        throw Extension::param_wrong_type();
    return floatpntr->set(in, doc, node);
}

/** \brief  Wrapper to cast to the object and use it's function.  */
const gchar *
Parameter::set_string (const gchar * in, SPDocument * doc, Inkscape::XML::Node * node)
{
    ParamString * stringpntr;
    stringpntr = dynamic_cast<ParamString *>(this);
    if (stringpntr == NULL)
        throw Extension::param_wrong_type();
    return stringpntr->set(in, doc, node);
}

/** \brief  Initialize the object, to do that, copy the data. */
ParamString::ParamString (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
    Parameter(name, guitext, desc, scope, ext), _value(NULL)
{
    const char * defaultval = NULL;
    if (sp_repr_children(xml) != NULL)
        defaultval = sp_repr_children(xml)->content();

    gchar * pref_name = this->pref_name();
    const gchar * paramval = prefs_get_string_attribute(PREF_DIR, pref_name);
    g_free(pref_name);

    if (paramval != NULL)
        defaultval = paramval;
    if (defaultval != NULL)
        _value = g_strdup(defaultval);

    return;
}

/** \brief  Free the allocated data. */
ParamString::~ParamString(void)
{
    g_free(_value);
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
Parameter::get_widget (SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal)
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

/** \brief  A class to make an adjustment that uses Extension params */
class ParamFloatAdjustment : public Gtk::Adjustment {
    /** The parameter to adjust */
    ParamFloat * _pref;
    SPDocument * _doc;
    Inkscape::XML::Node * _node;
    sigc::signal<void> * _changeSignal;
public:
    /** \brief  Make the adjustment using an extension and the string
                describing the parameter. */
    ParamFloatAdjustment (ParamFloat * param, SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal) :
            Gtk::Adjustment(0.0, param->min(), param->max(), 0.1), _pref(param), _doc(doc), _node(node), _changeSignal(changeSignal) {
        this->set_value(_pref->get(NULL, NULL) /* \todo fix */);
        this->signal_value_changed().connect(sigc::mem_fun(this, &ParamFloatAdjustment::val_changed));
        return;
    };

    void val_changed (void);
}; /* class ParamFloatAdjustment */

/** \brief  A function to respond to the value_changed signal from the
            adjustment.

    This function just grabs the value from the adjustment and writes
    it to the parameter.  Very simple, but yet beautiful.
*/
void
ParamFloatAdjustment::val_changed (void)
{
    //std::cout << "Value Changed to: " << this->get_value() << std::endl;
    _pref->set(this->get_value(), _doc, _node);
    if (_changeSignal != NULL) {
        _changeSignal->emit();
    }
    return;
}

/** \brief  A class to make an adjustment that uses Extension params */
class ParamIntAdjustment : public Gtk::Adjustment {
    /** The parameter to adjust */
    ParamInt * _pref;
    SPDocument * _doc;
    Inkscape::XML::Node * _node;
    sigc::signal<void> * _changeSignal;
public:
    /** \brief  Make the adjustment using an extension and the string
                describing the parameter. */
    ParamIntAdjustment (ParamInt * param, SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal) :
            Gtk::Adjustment(0.0, param->min(), param->max(), 1.0), _pref(param), _doc(doc), _node(node), _changeSignal(changeSignal) {
        this->set_value(_pref->get(NULL, NULL) /* \todo fix */);
        this->signal_value_changed().connect(sigc::mem_fun(this, &ParamIntAdjustment::val_changed));
        return;
    };

    void val_changed (void);
}; /* class ParamIntAdjustment */

/** \brief  A function to respond to the value_changed signal from the
            adjustment.

    This function just grabs the value from the adjustment and writes
    it to the parameter.  Very simple, but yet beautiful.
*/
void
ParamIntAdjustment::val_changed (void)
{
    //std::cout << "Value Changed to: " << this->get_value() << std::endl;
    _pref->set((int)this->get_value(), _doc, _node);
    if (_changeSignal != NULL) {
        _changeSignal->emit();
    }
    return;
}

/**
    \brief  Creates a Float Adjustment for a float parameter

    Builds a hbox with a label and a float adjustment in it.
*/
Gtk::Widget *
ParamFloat::get_widget (SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal)
{
    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox(false, 4));

    Gtk::Label * label = Gtk::manage(new Gtk::Label(_(_text), Gtk::ALIGN_LEFT));
    label->show();
    hbox->pack_start(*label, true, true);

    ParamFloatAdjustment * fadjust = Gtk::manage(new ParamFloatAdjustment(this, doc, node, changeSignal));
    Gtk::SpinButton * spin = Gtk::manage(new Gtk::SpinButton(*fadjust, 0.1, 1));
    spin->show();
    hbox->pack_start(*spin, false, false);

    hbox->show();

    return dynamic_cast<Gtk::Widget *>(hbox);
}

/**
    \brief  Creates a Int Adjustment for a int parameter

    Builds a hbox with a label and a int adjustment in it.
*/
Gtk::Widget *
ParamInt::get_widget (SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal)
{
    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox(false, 4));

    Gtk::Label * label = Gtk::manage(new Gtk::Label(_(_text), Gtk::ALIGN_LEFT));
    label->show();
    hbox->pack_start(*label, true, true);

    ParamIntAdjustment * fadjust = Gtk::manage(new ParamIntAdjustment(this, doc, node, changeSignal));
    Gtk::SpinButton * spin = Gtk::manage(new Gtk::SpinButton(*fadjust, 1.0, 0));
    spin->show();
    hbox->pack_start(*spin, false, false);

    hbox->show();

    return dynamic_cast<Gtk::Widget *>(hbox);
}

/** \brief  A check button which is Param aware.  It works with the
            parameter to change it's value as the check button changes
            value. */
class ParamBoolCheckButton : public Gtk::CheckButton {
private:
    /** \brief  Param to change */
    ParamBool * _pref;
    SPDocument * _doc;
    Inkscape::XML::Node * _node;
    sigc::signal<void> * _changeSignal;
public:
    /** \brief  Initialize the check button
        \param  param  Which parameter to adjust on changing the check button

        This function sets the value of the checkbox to be that of the
        parameter, and then sets up a callback to \c on_toggle.
    */
    ParamBoolCheckButton (ParamBool * param, SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal) :
            Gtk::CheckButton(), _pref(param), _doc(doc), _node(node), _changeSignal(changeSignal) {
        this->set_active(_pref->get(NULL, NULL) /**\todo fix */);
        this->signal_toggled().connect(sigc::mem_fun(this, &ParamBoolCheckButton::on_toggle));
        return;
    }
    void on_toggle (void);
};

/**
    \brief  A function to respond to the check box changing

    Adjusts the value of the preference to match that in the check box.
*/
void
ParamBoolCheckButton::on_toggle (void)
{
    _pref->set(this->get_active(), NULL /**\todo fix this */, NULL);
    if (_changeSignal != NULL) {
        _changeSignal->emit();
    }
    return;
}

/**
    \brief  Creates a bool check button for a bool parameter

    Builds a hbox with a label and a check button in it.
*/
Gtk::Widget *
ParamBool::get_widget (SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal)
{
    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox(false, 4));

    Gtk::Label * label = Gtk::manage(new Gtk::Label(_(_text), Gtk::ALIGN_LEFT));
    label->show();
    hbox->pack_start(*label, true, true);

    ParamBoolCheckButton * checkbox = new ParamBoolCheckButton(this, doc, node, changeSignal);
    checkbox->show();
    hbox->pack_start(*checkbox, false, false);

    hbox->show();

    return dynamic_cast<Gtk::Widget *>(hbox);
}

/** \brief  A special category of Gtk::Entry to handle string parameteres */
class ParamStringEntry : public Gtk::Entry {
private:
    ParamString * _pref;
    SPDocument * _doc;
    Inkscape::XML::Node * _node;
    sigc::signal<void> * _changeSignal;
public:
    /** \brief  Build a string preference for the given parameter
        \param  pref  Where to get the string from, and where to put it
                      when it changes.
    */
    ParamStringEntry (ParamString * pref, SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal) :
        Gtk::Entry(), _pref(pref), _doc(doc), _node(node), _changeSignal(changeSignal) {
        if (_pref->get(NULL, NULL) != NULL)
            this->set_text(Glib::ustring(_pref->get(NULL, NULL)));
        this->signal_changed().connect(sigc::mem_fun(this, &ParamStringEntry::changed_text));
    };
    void changed_text (void);
};

/** \brief  Respond to the text box changing

    This function responds to the box changing by grabbing the value
    from the text box and putting it in the parameter.
*/
void
ParamStringEntry::changed_text (void)
{
    Glib::ustring data = this->get_text();
    _pref->set(data.c_str(), _doc, _node);
    if (_changeSignal != NULL) {
        _changeSignal->emit();
    }
    return;
}

/**
    \brief  Creates a text box for the string parameter

    Builds a hbox with a label and a text box in it.
*/
Gtk::Widget *
ParamString::get_widget (SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal)
{
    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox(false, 4));

    Gtk::Label * label = Gtk::manage(new Gtk::Label(_(_text), Gtk::ALIGN_LEFT));
    label->show();
    hbox->pack_start(*label, false, false);

    ParamStringEntry * textbox = new ParamStringEntry(this, doc, node, changeSignal);
    textbox->show();
    hbox->pack_start(*textbox, true, true);

    hbox->show();

    return dynamic_cast<Gtk::Widget *>(hbox);
}

/** \brief  Return 'true' or 'false' */
Glib::ustring *
ParamBool::string (void)
{
    Glib::ustring * mystring;

    if (_value)
        mystring = new Glib::ustring("true");
    else
        mystring = new Glib::ustring("false");

    return mystring;
}

/** \brief  Return the value as a string */
Glib::ustring *
ParamInt::string (void)
{
    char startstring[32];
    sprintf(startstring, "%d", _value);
    Glib::ustring * mystring = new Glib::ustring(startstring);
    return mystring;
}

/** \brief  Return the value as a string */
Glib::ustring *
ParamFloat::string (void)
{
    char startstring[G_ASCII_DTOSTR_BUF_SIZE];
    g_ascii_dtostr(startstring, G_ASCII_DTOSTR_BUF_SIZE, _value);
    Glib::ustring * mystring = new Glib::ustring(startstring);
    return mystring;
}

/** \brief  Return the value as a string */
Glib::ustring *
ParamString::string (void)
{
    // FIXME: I think the string should NOT be escaped. Just put between "..."
    // Otherwise \frac{1}{2} will become \\frac{1}{2} and then the LaTeX effect won't work....
    //gchar * esc = g_strescape(_value, NULL);
    Glib::ustring escaped(_value);
    //g_free(esc);
    
#ifdef ESCAPE_DOLLAR_COMMANDLINE // escape the dollar sign 
    Glib::ustring::iterator i;
    for (i = escaped.begin(); i != escaped.end(); ++i) {
        if ( *i == '$') {
            i = escaped.insert(i, '\\');
            i++;
        }
    }
#endif

    Glib::ustring * mystring = new Glib::ustring("");
    *mystring += "\"";
    *mystring += escaped;
    *mystring += "\"";
    
    return mystring;
}

/** \brief  Create a label for the description */
Gtk::Widget *
ParamDescription::get_widget (SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal)
{
    Gtk::Label * label = Gtk::manage(new Gtk::Label(_(_value)));
    label->set_line_wrap();
    label->show();

    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox(false, 4));
    hbox->pack_start(*label, true, true, 5);
    hbox->show();

    return hbox;
}

/** \brief  Initialize the object, to do that, copy the data. */
ParamDescription::ParamDescription (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
    Parameter(name, guitext, desc, scope, ext), _value(NULL)
{
    // printf("Building Description\n");
    const char * defaultval = NULL;
    if (sp_repr_children(xml) != NULL)
        defaultval = sp_repr_children(xml)->content();

    if (defaultval != NULL)
        _value = g_strdup(defaultval);

    return;
}

ParamEnum::ParamEnum (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
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
    *mystring += "\"";
    *mystring += this->get(NULL, NULL);
    *mystring += "\"";
    return mystring;
}

Gtk::Widget *
ParamEnum::get_widget (SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal)
{
    return NULL;
}

const gchar *
ParamEnum::set (const gchar * in, SPDocument * doc, Inkscape::XML::Node * node)
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
