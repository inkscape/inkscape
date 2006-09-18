/** \file
 * extension parameter for enumerations. 
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
#include <gtkmm/comboboxtext.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/label.h>

#include <glibmm/i18n.h>

#include <xml/node.h>

#include "extension.h"
#include "prefs-utils.h"
#include "document-private.h"
#include "sp-object.h"

#include "paramenum.h"

/** \brief  The root directory in the preferences database for extension
            related parameters. */
#define PREF_DIR "extensions"

namespace Inkscape {
namespace Extension {

ParamComboBox::ParamComboBox (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
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

ParamComboBox::~ParamComboBox (void)
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
ParamComboBox::set (const gchar * in, SPDocument * doc, Inkscape::XML::Node * node)
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
    \brief  A function to get the currentpage and the parameters in a string form
    \return A string with the 'value' and all the parameters on all pages as command line arguments
*/
Glib::ustring *
ParamComboBox::string (void)
{
    Glib::ustring * param_string = new Glib::ustring("");

    *param_string += "\"";
    *param_string += _value;
    *param_string += "\"";

    return param_string;
}

/** \brief  A special category of Gtk::Entry to handle string parameteres */
class ParamComboBoxEntry : public Gtk::ComboBoxText {
private:
    ParamComboBox * _pref;
    SPDocument * _doc;
    Inkscape::XML::Node * _node;
public:
    /** \brief  Build a string preference for the given parameter
        \param  pref  Where to get the string from, and where to put it
                      when it changes.
    */
    ParamComboBoxEntry (ParamComboBox * pref, SPDocument * doc, Inkscape::XML::Node * node) :
        Gtk::ComboBoxText(), _pref(pref), _doc(doc), _node(node) {
        this->signal_changed().connect(sigc::mem_fun(this, &ParamComboBoxEntry::changed));
    };
    void changed (void);
};

/** \brief  Respond to the text box changing

    This function responds to the box changing by grabbing the value
    from the text box and putting it in the parameter.
*/
void
ParamComboBoxEntry::changed (void)
{
    Glib::ustring data = this->get_active_text();
    _pref->set(data.c_str(), _doc, _node);
    return;
}



/**
    \brief  Creates a combobox widget for an enumeration parameter
*/
Gtk::Widget *
ParamComboBox::get_widget (SPDocument * doc, Inkscape::XML::Node * node)
{
    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox(false, 4));

    Gtk::Label * label = Gtk::manage(new Gtk::Label(_(_text), Gtk::ALIGN_LEFT));
    label->show();
    hbox->pack_start(*label, false, false);

    ParamComboBoxEntry * combo = Gtk::manage(new ParamComboBoxEntry(this, doc, node));
    // add choice strings:         
    for (GSList * list = choices; list != NULL; list = g_slist_next(list)) {
        Glib::ustring * text = reinterpret_cast<Glib::ustring *>(list->data);
        combo->append_text(*text);
    }
    combo->set_active_text(Glib::ustring(_value));

    combo->show();
    hbox->pack_start(*combo, true, true);

    hbox->show();

    return dynamic_cast<Gtk::Widget *>(hbox);
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
