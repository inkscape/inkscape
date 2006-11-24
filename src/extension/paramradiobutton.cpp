/** \file
 * extension parameter for radiobuttons. 
 *
 * It uses a Gtk:ComboBoxText widget in the extension UI.
 */

/*
 * Author:
 *   Johan Engelen <johan@shouraizou.nl>
 *
 * Copyright (C) 2006 Author
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


#include <gtkmm/box.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/radiobuttongroup.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/label.h>

#include <glibmm/i18n.h>

#include <xml/node.h>

#include "extension.h"
#include "prefs-utils.h"
#include "document-private.h"
#include "sp-object.h"

#include "paramradiobutton.h"

/** \brief  The root directory in the preferences database for extension
            related parameters. */
#define PREF_DIR "extensions"

namespace Inkscape {
namespace Extension {

ParamRadioButton::ParamRadioButton (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
    Parameter(name, guitext, desc, scope, ext)
{              
    choices = NULL;
    _value = NULL;
    
    // Read XML tree to add enumeration items:
    // printf("Extension Constructor: ");
    if (xml != NULL) {
        Inkscape::XML::Node *child_repr = sp_repr_children(xml);
        while (child_repr != NULL) {
            char const * chname = child_repr->name();
            if (!strcmp(chname, "item")) {
                Glib::ustring * newitem = NULL;
                const char * contents = sp_repr_children(child_repr)->content();
                if (contents != NULL)
                     newitem = new Glib::ustring(contents);
                if (newitem != NULL) choices = g_slist_append(choices, newitem);
            }
            child_repr = sp_repr_next(child_repr);
        }
    }
    
    // Initialize _value with the default value from xml
    // for simplicity : default to the contents of the first xml-child
    const char * defaultval = NULL;
    if (sp_repr_children(sp_repr_children(xml)) != NULL)
        defaultval = sp_repr_children(sp_repr_children(xml))->content();
    
    gchar * pref_name = this->pref_name();
    const gchar * paramval = prefs_get_string_attribute(PREF_DIR, pref_name);
    g_free(pref_name);

    if (paramval != NULL)
        defaultval = paramval;
    if (defaultval != NULL)
        _value = g_strdup(defaultval);  // allocate space for _value
        
    return;
}

ParamRadioButton::~ParamRadioButton (void)
{                  
    //destroy choice strings
    for (GSList * list = choices; list != NULL; list = g_slist_next(list)) {
        Glib::ustring * text = reinterpret_cast<Glib::ustring *>(list->data);
        delete text;
    }
    g_slist_free(choices);

    g_free(_value);
}


/** \brief  A function to set the \c _value
    \param  in   The value to set
    \param  doc  A document that should be used to set the value.
    \param  node The node where the value may be placed

    This function sets ONLY the internal value, but it also sets the value
    in the preferences structure.  To put it in the right place, \c PREF_DIR
    and \c pref_name() are used.

    To copy the data into _value the old memory must be free'd first.
    It is important to note that \c g_free handles \c NULL just fine.  Then
    the passed in value is duplicated using \c g_strdup().
*/
const gchar *
ParamRadioButton::set (const gchar * in, SPDocument * doc, Inkscape::XML::Node * node)
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


/**
    \brief  A function to get the current value of the parameter in a string form
    \return A string with the 'value' as command line argument
*/
Glib::ustring *
ParamRadioButton::string (void)
{
    Glib::ustring * param_string = new Glib::ustring("");

    *param_string += "\"";
    *param_string += _value;
    *param_string += "\"";

    return param_string;
}

/** \brief  A special radiobutton class to use in ParamRadioButton */
class ParamRadioButtonWdg : public Gtk::RadioButton {
private:
    ParamRadioButton * _pref;
    SPDocument * _doc;
    Inkscape::XML::Node * _node;
public:
    /** \brief  Build a string preference for the given parameter
        \param  pref  Where to put the radiobutton's string when it is selected.
    */
    ParamRadioButtonWdg ( Gtk::RadioButtonGroup& group, const Glib::ustring& label, 
                          ParamRadioButton * pref, SPDocument * doc, Inkscape::XML::Node * node ) :
        Gtk::RadioButton(group, label), _pref(pref), _doc(doc), _node(node) {
        add_changesignal();
    };
    ParamRadioButtonWdg ( const Glib::ustring& label, 
                          ParamRadioButton * pref, SPDocument * doc, Inkscape::XML::Node * node ) :
        Gtk::RadioButton(label), _pref(pref), _doc(doc), _node(node) {
        add_changesignal();
    };
    void add_changesignal() {
        this->signal_toggled().connect(sigc::mem_fun(this, &ParamRadioButtonWdg::changed));
    };
    void changed (void);
};

/** \brief  Respond to the selected radiobutton changing

    This function responds to the radiobutton selection changing by grabbing the value
    from the text box and putting it in the parameter.
*/
void
ParamRadioButtonWdg::changed (void)
{
    if (this->get_active()) {
        Glib::ustring data = this->get_label();
        g_message(data.c_str());
        _pref->set(data.c_str(), _doc, _node);
    }
}



/**
    \brief  Creates a combobox widget for an enumeration parameter
*/
Gtk::Widget *
ParamRadioButton::get_widget (SPDocument * doc, Inkscape::XML::Node * node)
{
    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox(false, 4));
    Gtk::VBox * vbox = Gtk::manage(new Gtk::VBox(false, 0));

    Gtk::Label * label = Gtk::manage(new Gtk::Label(_(_text), Gtk::ALIGN_LEFT, Gtk::ALIGN_TOP));
    label->show();
    hbox->pack_start(*label, false, false);

    // add choice strings as radiobuttons
    // and select last selected option (_value)
    bool first = true;
    ParamRadioButtonWdg * radio;
    Gtk::RadioButtonGroup group;
    for (GSList * list = choices; list != NULL; list = g_slist_next(list)) {
        Glib::ustring * text = reinterpret_cast<Glib::ustring *>(list->data);
        if (first) {
            radio = Gtk::manage(new ParamRadioButtonWdg(*text, this, doc, node));
            group = radio->get_group();
            first = false;
        } else {
            radio = Gtk::manage(new ParamRadioButtonWdg(group, *text, this, doc, node));
        } 
        radio->show();
        vbox->pack_start(*radio, true, true);
        if (!strcmp(text->c_str(), _value)) {
            radio->set_active();
        } 
    }
    vbox->show();
    hbox->pack_end(*vbox, false, false);
    hbox->show();
    

    return dynamic_cast<Gtk::Widget *>(hbox);
}


}  /* namespace Extension */
}  /* namespace Inkscape */

