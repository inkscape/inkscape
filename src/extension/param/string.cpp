/*
 * Copyright (C) 2005-2007 Authors:
 *   Ted Gould <ted@gould.cx>
 *   Johan Engelen <johan@shouraizou.nl> *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/spinbutton.h>

#include "xml/node.h"
#include "extension/extension.h"
#include "string.h"
#include "preferences.h"

namespace Inkscape {
namespace Extension {
    

/** \brief  Free the allocated data. */
ParamString::~ParamString(void)
{
    g_free(_value);
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
ParamString::set (const gchar * in, Document * /*doc*/, Inkscape::XML::Node * /*node*/)
{
    if (in == NULL) return NULL; /* Can't have NULL string */

    if (_value != NULL)
        g_free(_value);
    _value = g_strdup(in);

    gchar * prefname = this->pref_name();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setString(extension_pref_root + prefname, _value);
    g_free(prefname);

    return _value;
}

/** \brief  Return the value as a string */
void
ParamString::string (std::string &string)
{
    if (_value == NULL)
        return;

    string += _value;
    return;
}

/** \brief  Initialize the object, to do that, copy the data. */
ParamString::ParamString (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, bool gui_hidden, const gchar * gui_tip, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
    Parameter(name, guitext, desc, scope, gui_hidden, gui_tip, ext), _value(NULL)
{
    const char * defaultval = NULL;
    if (sp_repr_children(xml) != NULL)
        defaultval = sp_repr_children(xml)->content();

    gchar * pref_name = this->pref_name();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Glib::ustring paramval = prefs->getString(extension_pref_root + pref_name);
    g_free(pref_name);

    if (!paramval.empty())
        defaultval = paramval.data();
    if (defaultval != NULL)
        _value = g_strdup(defaultval);
    
    _max_length = 0;

    return;
}

/** \brief  A special type of Gtk::Entry to handle string parameteres */
class ParamStringEntry : public Gtk::Entry {
private:
    ParamString * _pref;
    Document * _doc;
    Inkscape::XML::Node * _node;
    sigc::signal<void> * _changeSignal;
public:
    /** \brief  Build a string preference for the given parameter
        \param  pref  Where to get the string from, and where to put it
                      when it changes.
    */
    ParamStringEntry (ParamString * pref, Document * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal) :
        Gtk::Entry(), _pref(pref), _doc(doc), _node(node), _changeSignal(changeSignal) {
        if (_pref->get(NULL, NULL) != NULL)
            this->set_text(Glib::ustring(_pref->get(NULL, NULL)));
        this->set_max_length(_pref->getMaxLength()); //Set the max lenght - default zero means no maximum
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
ParamString::get_widget (Document * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal)
{
	if (_gui_hidden) return NULL;

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

}  /* namespace Extension */
}  /* namespace Inkscape */
