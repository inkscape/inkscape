#define __SP_MODULE_C__
/** \file
 *
 * Inkscape::Extension::Extension: 
 * the ability to have features that are more modular so that they
 * can be added and removed easily.  This is the basis for defining
 * those actions.
 */

/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


#include <glibmm/i18n.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/frame.h>
#include <gtkmm/table.h>
#include <gtkmm/tooltips.h>

#include "inkscape.h"
#include "extension/implementation/implementation.h"

#include "db.h"
#include "dependency.h"
#include "timer.h"
#include "param/parameter.h"

namespace Inkscape {
namespace Extension {

/* Inkscape::Extension::Extension */

std::vector<const gchar *> Extension::search_path;
std::ofstream Extension::error_file;

Parameter * param_shared (const gchar * name, GSList * list);

/**
    \return  none
    \brief   Constructs an Extension from a Inkscape::XML::Node
    \param   in_repr    The repr that should be used to build it

    This function is the basis of building an extension for Inkscape.  It
    currently extracts the fields from the Repr that are used in the
    extension.  The Repr will likely include other children that are
    not related to the module directly.  If the Repr does not include
    a name and an ID the module will be left in an errored state.
*/
Extension::Extension (Inkscape::XML::Node * in_repr, Implementation::Implementation * in_imp)
    : _help(NULL)
    , _gui(true)
{
    repr = in_repr;
    Inkscape::GC::anchor(in_repr);

    id = NULL;
    name = NULL;
    _state = STATE_UNLOADED;
    parameters = NULL;

    if (in_imp == NULL) {
        imp = new Implementation::Implementation();
    } else {
        imp = in_imp;
    }

    // printf("Extension Constructor: ");
    if (repr != NULL) {
        Inkscape::XML::Node *child_repr = sp_repr_children(repr);
        /* TODO: Handle what happens if we don't have these two */
        while (child_repr != NULL) {
            char const * chname = child_repr->name();
			if (!strncmp(chname, INKSCAPE_EXTENSION_NS_NC, strlen(INKSCAPE_EXTENSION_NS_NC))) {
				chname += strlen(INKSCAPE_EXTENSION_NS);
			}
            if (chname[0] == '_') /* Allow _ for translation of tags */
                chname++;
            if (!strcmp(chname, "id")) {
                gchar const *val = sp_repr_children(child_repr)->content();
                id = g_strdup (val);
            } /* id */
            if (!strcmp(chname, "name")) {
                name = g_strdup (sp_repr_children(child_repr)->content());
            } /* name */
            if (!strcmp(chname, "help")) {
                _help = g_strdup (sp_repr_children(child_repr)->content());
            } /* name */
            if (!strcmp(chname, "param") || !strcmp(chname, "_param")) {
                Parameter * param;
                param = Parameter::make(child_repr, this);
                if (param != NULL)
                    parameters = g_slist_append(parameters, param);
            } /* param || _param */
            if (!strcmp(chname, "dependency")) {
                _deps.push_back(new Dependency(child_repr));
            } /* dependency */
            child_repr = sp_repr_next(child_repr);
        }

        db.register_ext (this);
    }
    // printf("%s\n", name);
    timer = NULL;

    return;
}

/**
    \return   none
    \brief    Destroys the Extension

    This function frees all of the strings that could be attached
    to the extension and also unreferences the repr.  This is better
    than freeing it because it may (I wouldn't know why) be referenced
    in another place.
*/
Extension::~Extension (void)
{
//    printf("Extension Destructor: %s\n", name);
    set_state(STATE_UNLOADED);
    db.unregister_ext(this);
    Inkscape::GC::release(repr);
    g_free(id);
    g_free(name);
    delete timer;
    timer = NULL;
    /** \todo Need to do parameters here */
    
    // delete parameters: 
    for (GSList * list = parameters; list != NULL; list = g_slist_next(list)) {
        Parameter * param = reinterpret_cast<Parameter *>(list->data);
        delete param;
    }
    g_slist_free(parameters);
    
    
    for (unsigned int i = 0 ; i < _deps.size(); i++) {
        delete _deps[i];
    }
    _deps.clear();

    return;
}

/**
    \return   none
    \brief    A function to set whether the extension should be loaded
              or unloaded
    \param    in_state  Which state should the extension be in?

    It checks to see if this is a state change or not.  If we're changing
    states it will call the appropriate function in the implementation,
    load or unload.  Currently, there is no error checking in this
    function.  There should be.
*/
void
Extension::set_state (state_t in_state)
{
    if (_state == STATE_DEACTIVATED) return;
    if (in_state != _state) {
        /** \todo Need some more error checking here! */
        switch (in_state) {
            case STATE_LOADED:
                if (imp->load(this))
                    _state = STATE_LOADED;

                if (timer != NULL) {
                    delete timer;
                }
                timer = new ExpirationTimer(this);

                break;
            case STATE_UNLOADED:
                // std::cout << "Unloading: " << name << std::endl;
                imp->unload(this);
                _state = STATE_UNLOADED;

                if (timer != NULL) {
                    delete timer;
                    timer = NULL;
                }
                break;
            case STATE_DEACTIVATED:
                _state = STATE_DEACTIVATED;

                if (timer != NULL) {
                    delete timer;
                    timer = NULL;
                }
                break;
            default:
                break;
        }
    }

    return;
}

/**
    \return   The state the extension is in
    \brief    A getter for the state variable.
*/
Extension::state_t
Extension::get_state (void)
{
    return _state;
}

/**
    \return  Whether the extension is loaded or not
    \brief   A quick function to test the state of the extension
*/
bool
Extension::loaded (void)
{
    return get_state() == STATE_LOADED;
}

/**
    \return  A boolean saying whether the extension passed the checks
    \brief   A function to check the validity of the extension

    This function chekcs to make sure that there is an id, a name, a
    repr and an implemenation for this extension.  Then it checks all
    of the dependencies to see if they pass.  Finally, it asks the
    implmentation to do a check of itself.

    On each check, if there is a failure, it will print a message to the
    error log for that failure.  It is important to note that the function
    keeps executing if it finds an error, to try and get as many of them
    into the error log as possible.  This should help people debug
    installations, and figure out what they need to get for the full
    functionality of Inkscape to be available.
*/
bool
Extension::check (void)
{
    bool retval = true;

    // static int i = 0;
    // std::cout << "Checking module[" << i++ << "]: " << name << std::endl;

    const char * inx_failure = _("  This is caused by an improper .inx file for this extension."
                                 "  An improper .inx file could have been caused by a faulty installation of Inkscape.");
    if (id == NULL) {
        printFailure(Glib::ustring(_("an ID was not defined for it.")) + inx_failure);
        retval = false;
    }
    if (name == NULL) {
        printFailure(Glib::ustring(_("there was no name defined for it.")) + inx_failure);
        retval = false;
    }
    if (repr == NULL) {
        printFailure(Glib::ustring(_("the XML description of it got lost.")) + inx_failure);
        retval = false;
    }
    if (imp == NULL) {
        printFailure(Glib::ustring(_("no implementation was defined for the extension.")) + inx_failure);
        retval = false;
    }

    for (unsigned int i = 0 ; i < _deps.size(); i++) {
        if (_deps[i]->check() == FALSE) {
            // std::cout << "Failed: " << *(_deps[i]) << std::endl;
            printFailure(Glib::ustring(_("a dependency was not met.")));
            error_file << *_deps[i] << std::endl;
            retval = false;
        }
    }

    if (retval)
        return imp->check(this);
    return retval;
}

/** \brief A quick function to print out a standard start of extension
           errors in the log.
    \param reason  A string explaining why this failed

    Real simple, just put everything into \c error_file.
*/
void
Extension::printFailure (Glib::ustring reason)
{
    error_file << _("Extension \"") << name << _("\" failed to load because ");
    error_file << reason.raw();
    error_file << std::endl;
    return;
}

/**
    \return  The XML tree that is used to define the extension
    \brief   A getter for the internal Repr, does not add a reference.
*/
Inkscape::XML::Node *
Extension::get_repr (void)
{
    return repr;
}

/**
    \return  The textual id of this extension
    \brief   Get the ID of this extension - not a copy don't delete!
*/
gchar *
Extension::get_id (void)
{
    return id;
}

/**
    \return  The textual name of this extension
    \brief   Get the name of this extension - not a copy don't delete!
*/
gchar *
Extension::get_name (void)
{
    return name;
}

/**
    \return  None
    \brief   This function diactivates the extension (which makes it
             unusable, but not deleted)

    This function is used to removed an extension from functioning, but
    not delete it completely.  It sets the state to \c STATE_DEACTIVATED to
    mark to the world that it has been deactivated.  It also removes
    the current implementation and replaces it with a standard one.  This
    makes it so that we don't have to continually check if there is an
    implementation, but we are gauranteed to have a benign one.

    \warning It is important to note that there is no 'activate' function.
    Running this function is irreversable.
*/
void
Extension::deactivate (void)
{
    set_state(STATE_DEACTIVATED);

    /* Removing the old implementation, and making this use the default. */
    /* This should save some memory */
    delete imp;
    imp = new Implementation::Implementation();

    return;
}

/**
    \return  Whether the extension has been deactivated
    \brief   Find out the status of the extension
*/
bool
Extension::deactivated (void)
{
    return get_state() == STATE_DEACTIVATED;
}

/**
    \return    Parameter structure with a name of 'name'
    \brief     This function looks through the linked list for a parameter
               structure with the name of the passed in name
    \param     name   The name to search for
    \param     list   The list to look for

    This is an inline function that is used by all the get_param and
    set_param functions to find a param_t in the linked list with
    the passed in name.  It is done as an inline so that it will be
    optimized into a 'jump' by the compiler.

    This function can throw a 'param_not_exist' exception if the
    name is not found.

    The first thing that this function checks is if the list is NULL.
    It could be NULL because there are no parameters for this extension
    or because all of them have been checked (I'll spoil the ending and
    tell you that this function is called recursively).  If the list
    is NULL then the 'param_not_exist' exception is thrown.

    Otherwise, the function looks at the current param_t that the element
    list points to.  If the name of that param_t matches the passed in
    name then that param_t is returned.  Otherwise, this function is
    called again with g_slist_next as a parameter.
*/
Parameter *
param_shared (const gchar * name, GSList * list)
{
    Parameter * output;

    if (name == NULL) {
        throw Extension::param_not_exist();
    }
    if (list == NULL) {
        throw Extension::param_not_exist();
    }

    output = static_cast<Parameter *>(list->data);
    if (!strcmp(output->name(), name)) {
        return output;
    }

    return param_shared(name, g_slist_next(list));
}

/**
    \return   A constant pointer to the string held by the parameters.
    \brief    Gets a parameter identified by name with the string placed
              in value.  It isn't duplicated into the value string.
    \param    name    The name of the parameter to get
    \param    doc    The document to look in for document specific parameters
    \param    node   The node to look in for a specific parameter

    Look up in the parameters list, then execute the function on that
    found parameter.
*/
const gchar *
Extension::get_param_string (const gchar * name, const SPDocument * doc, const Inkscape::XML::Node * node)
{
    Parameter * param;

    param = param_shared(name, parameters);
    return param->get_string(doc, node);
}

const gchar *
Extension::get_param_enum (const gchar * name, const SPDocument * doc, const Inkscape::XML::Node * node)
{
    Parameter* param = param_shared(name, parameters);
    return param->get_enum(doc, node);
}


gchar const *Extension::get_param_optiongroup( gchar const * name, SPDocument const * doc, Inkscape::XML::Node const * node)
{
    Parameter* param = param_shared(name, parameters);
    return param->get_optiongroup(doc, node);
}


/**
    \return   The value of the parameter identified by the name
    \brief    Gets a parameter identified by name with the bool placed
              in value.
    \param    name    The name of the parameter to get
    \param    doc    The document to look in for document specific parameters
    \param    node   The node to look in for a specific parameter

    Look up in the parameters list, then execute the function on that
    found parameter.
*/
bool
Extension::get_param_bool (const gchar * name, const SPDocument * doc, const Inkscape::XML::Node * node)
{
    Parameter * param;

    param = param_shared(name, parameters);
    return param->get_bool(doc, node);
}

/**
    \return   The integer value for the parameter specified
    \brief    Gets a parameter identified by name with the integer placed
              in value.
    \param    name    The name of the parameter to get
    \param    doc    The document to look in for document specific parameters
    \param    node   The node to look in for a specific parameter

    Look up in the parameters list, then execute the function on that
    found parameter.
*/
int
Extension::get_param_int (const gchar * name, const SPDocument * doc, const Inkscape::XML::Node * node)
{
    Parameter * param;

    param = param_shared(name, parameters);
    return param->get_int(doc, node);
}

/**
    \return   The float value for the parameter specified
    \brief    Gets a parameter identified by name with the float placed
              in value.
    \param    name    The name of the parameter to get
    \param    doc    The document to look in for document specific parameters
    \param    node   The node to look in for a specific parameter

    Look up in the parameters list, then execute the function on that
    found parameter.
*/
float
Extension::get_param_float (const gchar * name, const SPDocument * doc, const Inkscape::XML::Node * node)
{
    Parameter * param;
    param = param_shared(name, parameters);
    return param->get_float(doc, node);
}

/**
    \return   The string value for the parameter specified
    \brief    Gets a parameter identified by name with the float placed
              in value.
    \param    name    The name of the parameter to get
    \param    doc    The document to look in for document specific parameters
    \param    node   The node to look in for a specific parameter

    Look up in the parameters list, then execute the function on that
    found parameter.
*/
guint32
Extension::get_param_color (const gchar * name, const SPDocument * doc, const Inkscape::XML::Node * node)
{
    Parameter* param = param_shared(name, parameters);
    return param->get_color(doc, node);
}

/**
    \return   The passed in value
    \brief    Sets a parameter identified by name with the boolean
              in the parameter value.
    \param    name    The name of the parameter to set
    \param    value   The value to set the parameter to
    \param    doc    The document to look in for document specific parameters
    \param    node   The node to look in for a specific parameter

    Look up in the parameters list, then execute the function on that
    found parameter.
*/
bool
Extension::set_param_bool (const gchar * name, bool value, SPDocument * doc, Inkscape::XML::Node * node)
{
    Parameter * param;
    param = param_shared(name, parameters);
    return param->set_bool(value, doc, node);
}

/**
    \return   The passed in value
    \brief    Sets a parameter identified by name with the integer
              in the parameter value.
    \param    name    The name of the parameter to set
    \param    value   The value to set the parameter to
    \param    doc    The document to look in for document specific parameters
    \param    node   The node to look in for a specific parameter

    Look up in the parameters list, then execute the function on that
    found parameter.
*/
int
Extension::set_param_int (const gchar * name, int value, SPDocument * doc, Inkscape::XML::Node * node)
{
    Parameter * param;
    param = param_shared(name, parameters);
    return param->set_int(value, doc, node);
}

/**
    \return   The passed in value
    \brief    Sets a parameter identified by name with the integer
              in the parameter value.
    \param    name    The name of the parameter to set
    \param    value   The value to set the parameter to
    \param    doc    The document to look in for document specific parameters
    \param    node   The node to look in for a specific parameter

    Look up in the parameters list, then execute the function on that
    found parameter.
*/
float
Extension::set_param_float (const gchar * name, float value, SPDocument * doc, Inkscape::XML::Node * node)
{
    Parameter * param;
    param = param_shared(name, parameters);
    return param->set_float(value, doc, node);
}

/**
    \return   The passed in value
    \brief    Sets a parameter identified by name with the string
              in the parameter value.
    \param    name    The name of the parameter to set
    \param    value   The value to set the parameter to
    \param    doc    The document to look in for document specific parameters
    \param    node   The node to look in for a specific parameter

    Look up in the parameters list, then execute the function on that
    found parameter.
*/
const gchar *
Extension::set_param_string (const gchar * name, const gchar * value, SPDocument * doc, Inkscape::XML::Node * node)
{
    Parameter * param;
    param = param_shared(name, parameters);
    return param->set_string(value, doc, node);
}

gchar const * Extension::set_param_optiongroup(gchar const * name, gchar const * value, SPDocument * doc, Inkscape::XML::Node * node)
{
    Parameter * param = param_shared(name, parameters);
    return param->set_optiongroup(value, doc, node);
}


/**
    \return   The passed in value
    \brief    Sets a parameter identified by name with the string
              in the parameter value.
    \param    name    The name of the parameter to set
    \param    value   The value to set the parameter to
    \param    doc    The document to look in for document specific parameters
    \param    node   The node to look in for a specific parameter

    Look up in the parameters list, then execute the function on that
    found parameter.
*/
guint32
Extension::set_param_color (const gchar * name, guint32 color, SPDocument * doc, Inkscape::XML::Node * node)
{
    Parameter* param = param_shared(name, parameters);
    return param->set_color(color, doc, node);
}

/** \brief A function to open the error log file. */
void
Extension::error_file_open (void)
{
    gchar * ext_error_file = profile_path(EXTENSION_ERROR_LOG_FILENAME);
    gchar * filename = g_filename_from_utf8( ext_error_file, -1, NULL, NULL, NULL );
    error_file.open(filename);
    if (!error_file.is_open()) {
        g_warning(_("Could not create extension error log file '%s'"),
                  filename);
    }
    g_free(filename);
    g_free(ext_error_file);
};

/** \brief A function to close the error log file. */
void
Extension::error_file_close (void)
{
    error_file.close();
};

/** \brief  A widget to represent the inside of an AutoGUI widget */
class AutoGUI : public Gtk::VBox {
    Gtk::Tooltips _tooltips;
public:
    /** \brief  Create an AutoGUI object */
    AutoGUI (void) : Gtk::VBox() {};
    /** \brief  Adds a widget with a tool tip into the autogui
        \param  widg  Widget to add
        \param  tooltip   Tooltip for the widget
        
        If there is no widget, nothing happens.  Otherwise it is just
        added into the VBox.  If there is a tooltip (non-NULL) then it
        is placed on the widget.
    */
    void addWidget (Gtk::Widget * widg, gchar const * tooltip) {
        if (widg == NULL) return;
        this->pack_start(*widg, true, true, 2);
        if (tooltip != NULL) {
            _tooltips.set_tip(*widg, Glib::ustring(tooltip));
        }
        return;
    };
};

/** \brief  A function to automatically generate a GUI using the parameters
    \return Generated widget

    This function just goes through each parameter, and calls it's 'get_widget'
    function to get each widget.  Then, each of those is placed into
    a Gtk::VBox, which is then returned to the calling function.

    If there are no visible parameters, this function just returns NULL.
    If all parameters are gui_visible = false NULL is returned as well.    
*/
Gtk::Widget *
Extension::autogui (SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal)
{
    if (!_gui || param_visible_count() == 0) return NULL;

    AutoGUI * agui = Gtk::manage(new AutoGUI());

    //go through the list of parameters to see if there are any non-hidden ones
    for (GSList * list = parameters; list != NULL; list = g_slist_next(list)) {
        Parameter * param = reinterpret_cast<Parameter *>(list->data);
        if (param->get_gui_hidden()) continue; //Ignore hidden parameters
        Gtk::Widget * widg = param->get_widget(doc, node, changeSignal);
        gchar const * tip = param->get_tooltip();
        agui->addWidget(widg, tip);
    }    
    
    agui->show();
    return agui;
};

/**
    \brief  A function to get the parameters in a string form
    \return An array with all the parameters in it.

*/
void
Extension::paramListString (std::list <std::string> &retlist)
{
    for (GSList * list = parameters; list != NULL; list = g_slist_next(list)) {
        Parameter * param = reinterpret_cast<Parameter *>(list->data);
        param->string(retlist);
    }

    return;
}

/* Extension editor dialog stuff */

Gtk::VBox *
Extension::get_info_widget(void)
{
    Gtk::VBox * retval = Gtk::manage(new Gtk::VBox());

    Gtk::Frame * info = Gtk::manage(new Gtk::Frame("General Extension Information"));
    retval->pack_start(*info, true, true, 5);

    Gtk::Table * table = Gtk::manage(new Gtk::Table());
    info->add(*table);

    int row = 0;
    add_val(_("Name:"), _(name), table, &row);
    add_val(_("ID:"), id, table, &row);
    add_val(_("State:"), _state == STATE_LOADED ? _("Loaded") : _state == STATE_UNLOADED ? _("Unloaded") : _("Deactivated"), table, &row);


    retval->show_all();
    return retval;
}

void
Extension::add_val(Glib::ustring labelstr, Glib::ustring valuestr, Gtk::Table * table, int * row)
{
    Gtk::Label * label;
    Gtk::Label * value;

    (*row)++; 
    label = Gtk::manage(new Gtk::Label(labelstr));
    value = Gtk::manage(new Gtk::Label(valuestr));
    table->attach(*label, 0, 1, (*row) - 1, *row);
    table->attach(*value, 1, 2, (*row) - 1, *row);

    label->show();
    value->show();

    return;
}

Gtk::VBox *
Extension::get_help_widget(void)
{
    Gtk::VBox * retval = Gtk::manage(new Gtk::VBox());

    if (_help == NULL) {
        Gtk::Label * content = Gtk::manage(new Gtk::Label(_("Currently there is no help available for this Extension.  Please look on the Inkscape website or ask on the mailing lists if you have questions regarding this extension.")));
        retval->pack_start(*content, true, true, 5);
        content->set_line_wrap(true);
        content->show();
    } else {



    }

    retval->show();
    return retval;
}

Gtk::VBox *
Extension::get_params_widget(void)
{
    Gtk::VBox * retval = Gtk::manage(new Gtk::VBox());
    Gtk::Widget * content = Gtk::manage(new Gtk::Label("Params"));
    retval->pack_start(*content, true, true, 5);
    content->show();
    retval->show();
    return retval;
}

unsigned int Extension::param_visible_count ( ) 
{
    unsigned int _visible_count = 0;
    for (GSList * list = parameters; list != NULL; list = g_slist_next(list)) {
        Parameter * param = reinterpret_cast<Parameter *>(list->data);
        if (!param->get_gui_hidden()) _visible_count++;
    }    
    return _visible_count;
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
