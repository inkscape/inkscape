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
#include "../extension.h"
#include "bool.h"
#include "preferences.h"

namespace Inkscape {
namespace Extension {

/** \brief  Use the superclass' allocator and set the \c _value */
ParamBool::ParamBool (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, bool gui_hidden, const gchar * gui_tip, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
        Parameter(name, guitext, desc, scope, gui_hidden, gui_tip, ext), _value(false)
{
    const char * defaultval = NULL;
    if (sp_repr_children(xml) != NULL)
        defaultval = sp_repr_children(xml)->content();

    if (defaultval != NULL && (!strcmp(defaultval, "true") || !strcmp(defaultval, "true") || !strcmp(defaultval, "1"))) {
        _value = true;
    } else {
        _value = false;
    }

    gchar * pref_name = this->pref_name();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    _value = prefs->getBool(extension_pref_root + pref_name, _value);
    g_free(pref_name);

    return;
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
ParamBool::set( bool in, Document * /*doc*/, Inkscape::XML::Node * /*node*/ )
{
    _value = in;

    gchar * prefname = this->pref_name();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool(extension_pref_root + prefname, _value);
    g_free(prefname);

    return _value;
}

/** \brief  Returns \c _value */
bool 
ParamBool::get (const Document * doc, const Inkscape::XML::Node * node)
{
	return _value; 
}

/** \brief  A check button which is Param aware.  It works with the
            parameter to change it's value as the check button changes
            value. */
class ParamBoolCheckButton : public Gtk::CheckButton {
private:
    /** \brief  Param to change */
    ParamBool * _pref;
    Document * _doc;
    Inkscape::XML::Node * _node;
    sigc::signal<void> * _changeSignal;
public:
    /** \brief  Initialize the check button
        \param  param  Which parameter to adjust on changing the check button

        This function sets the value of the checkbox to be that of the
        parameter, and then sets up a callback to \c on_toggle.
    */
    ParamBoolCheckButton (ParamBool * param, Document * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal) :
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

/** \brief  Return 'true' or 'false' */
void
ParamBool::string (std::string &string)
{
    if (_value) {
        string += "true";
    } else {
        string += "false";
    }

    return;
}

/**
    \brief  Creates a bool check button for a bool parameter

    Builds a hbox with a label and a check button in it.
*/
Gtk::Widget *
ParamBool::get_widget (Document * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal)
{
	if (_gui_hidden) return NULL;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
