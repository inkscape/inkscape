/** \file
 * extension parameter for radiobuttons.
 *
 * It uses a Gtk:ComboBoxText widget in the extension UI.
 */

/*
 * Author:
 *   Johan Engelen <johan@shouraizou.nl>
 *
 * Copyright (C) 2006-2007 Johan Engelen
 * Copyright (C) 2008 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtkmm/box.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/radiobutton.h>
#include <glibmm/i18n.h>

#include "xml/node.h"
#include "extension/extension.h"
#include "preferences.h"
#include "document-private.h"
#include "sp-object.h"

#include "radiobutton.h"

/**
 * The root directory in the preferences database for extension
 * related parameters.
 */
#define PREF_DIR "extensions"

namespace Inkscape {
namespace Extension {

/* For internal use only.
     Note that value and guitext MUST be non-NULL. This is ensured by newing only at one location in the code where non-NULL checks are made. */
class optionentry {
public:
    optionentry (Glib::ustring * val, Glib::ustring * text) {
        value = val;
        guitext = text;
    }
    ~optionentry() {
        delete value;
        delete guitext;
    }

    Glib::ustring * value;
    Glib::ustring * guitext;
};

ParamRadioButton::ParamRadioButton (const gchar * name,
                                    const gchar * guitext,
                                    const gchar * desc,
                                    const Parameter::_scope_t scope,
                                    bool gui_hidden,
                                    const gchar * gui_tip,
                                    Inkscape::Extension::Extension * ext,
                                    Inkscape::XML::Node * xml,
                                    AppearanceMode mode) :
    Parameter(name, guitext, desc, scope, gui_hidden, gui_tip, ext),
    _value(0), _mode(mode), _indent(0), choices(0)
{
    // Read XML tree to add enumeration items:
    // printf("Extension Constructor: ");
    if (xml != NULL) {
        Inkscape::XML::Node *child_repr = xml->firstChild();
        while (child_repr != NULL) {
            char const * chname = child_repr->name();
            if (!strcmp(chname, INKSCAPE_EXTENSION_NS "option") || !strcmp(chname, INKSCAPE_EXTENSION_NS "_option")) {
                Glib::ustring * newguitext = NULL;
                Glib::ustring * newvalue = NULL;
                const char * contents = child_repr->firstChild()->content();

                if (contents != NULL) {
                    // don't translate when 'item' but do translate when '_option'
                    if (!strcmp(chname, INKSCAPE_EXTENSION_NS "_option")) {
                        if (child_repr->attribute("msgctxt") != NULL) {
                            newguitext =  new Glib::ustring(g_dpgettext2(NULL, child_repr->attribute("msgctxt"), contents));
                        } else {
                            newguitext =  new Glib::ustring(_(contents));
                        }
                    } else {
                        newguitext =  new Glib::ustring(contents);
                    }
                } else {
                    continue;
                }


                const char * val = child_repr->attribute("value");
                if (val != NULL) {
                    newvalue = new Glib::ustring(val);
                } else {
                    newvalue = new Glib::ustring(contents);
                }

                if ( (newguitext) && (newvalue) ) {   // logical error if this is not true here
                    choices = g_slist_append( choices, new optionentry(newvalue, newguitext) );
                }
            }
            child_repr = child_repr->next();
        }
    }

    // Initialize _value with the default value from xml
    // for simplicity : default to the contents of the first xml-child
    const char * defaultval = NULL;
    if (choices) {
        defaultval = (static_cast<optionentry*> (choices->data))->value->c_str();
    }

    const char *indent = xml ? xml->attribute("indent") : NULL;
    if (indent != NULL) {
        _indent = atoi(indent) * 12;
    }

    gchar * pref_name = this->pref_name();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Glib::ustring paramval = prefs->getString(extension_pref_root + pref_name);
    g_free(pref_name);

    if (!paramval.empty()) {
        defaultval = paramval.data();
    }
    if (defaultval != NULL) {
        _value = g_strdup(defaultval);  // allocate space for _value
    }
}

ParamRadioButton::~ParamRadioButton (void)
{
    //destroy choice strings
    for (GSList * list = choices; list != NULL; list = g_slist_next(list)) {
        delete (reinterpret_cast<optionentry *>(list->data));
    }
    g_slist_free(choices);

    g_free(_value);
}


/**
 * A function to set the \c _value.
 *
 * This function sets ONLY the internal value, but it also sets the value
 * in the preferences structure.  To put it in the right place, \c PREF_DIR
 * and \c pref_name() are used.
 *
 * To copy the data into _value the old memory must be free'd first.
 * It is important to note that \c g_free handles \c NULL just fine.  Then
 * the passed in value is duplicated using \c g_strdup().
 *
 * @param  in   The value to set.
 * @param  doc  A document that should be used to set the value.
 * @param  node The node where the value may be placed.
 */
const gchar *ParamRadioButton::set(const gchar * in, SPDocument * /*doc*/, Inkscape::XML::Node * /*node*/)
{
    if (in == NULL) {
        return NULL; /* Can't have NULL string */
    }

    Glib::ustring * settext = NULL;
    for (GSList * list = choices; list != NULL; list = g_slist_next(list)) {
        optionentry * entr = reinterpret_cast<optionentry *>(list->data);
        if ( !entr->value->compare(in) ) {
            settext = entr->value;
            break;  // break out of for loop
        }
    }
    if (settext) {
        if (_value != NULL) {
            g_free(_value);
        }
        _value = g_strdup(settext->c_str());
        gchar * prefname = this->pref_name();
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setString(extension_pref_root + prefname, _value);
        g_free(prefname);
    } else {
        g_warning("Couldn't set ParamRadioButton %s", in);
    }

    return _value;
}

void ParamRadioButton::string(std::string &string) const
{
    string += _value;
}

/** A special radiobutton class to use in ParamRadioButton. */
class ParamRadioButtonWdg : public Gtk::RadioButton {
private:
    ParamRadioButton * _pref;
    SPDocument * _doc;
    Inkscape::XML::Node * _node;
    sigc::signal<void> * _changeSignal;
public:
    /**
     * Build a string preference for the given parameter.
     * @param  pref  Where to put the radiobutton's string when it is selected.
     */
    ParamRadioButtonWdg ( Gtk::RadioButtonGroup& group, const Glib::ustring& label,
                          ParamRadioButton * pref, SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal ) :
        Gtk::RadioButton(group, label), _pref(pref), _doc(doc), _node(node), _changeSignal(changeSignal) {
        add_changesignal();
    };
    ParamRadioButtonWdg ( const Glib::ustring& label,
                          ParamRadioButton * pref, SPDocument * doc, Inkscape::XML::Node * node , sigc::signal<void> * changeSignal) :
        Gtk::RadioButton(label), _pref(pref), _doc(doc), _node(node), _changeSignal(changeSignal) {
        add_changesignal();
    };
    void add_changesignal() {
        this->signal_toggled().connect(sigc::mem_fun(this, &ParamRadioButtonWdg::changed));
    };
    void changed (void);
};

/**
 * Respond to the selected radiobutton changing.
 *
 * This function responds to the radiobutton selection changing by grabbing the value
 * from the text box and putting it in the parameter.
 */
void ParamRadioButtonWdg::changed(void)
{
    if (this->get_active()) {
        Glib::ustring value = _pref->value_from_label(this->get_label());
        _pref->set(value.c_str(), _doc, _node);
    }
    if (_changeSignal != NULL) {
        _changeSignal->emit();
    }
}


class ComboWdg : public Gtk::ComboBoxText {
private:
    ParamRadioButton* _base;
    SPDocument* _doc;
    Inkscape::XML::Node* _node;
    sigc::signal<void> * _changeSignal;

public:
    ComboWdg(ParamRadioButton* base, SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal) :
        _base(base),
        _doc(doc),
        _node(node),
        _changeSignal(changeSignal)
    {
        this->signal_changed().connect(sigc::mem_fun(this, &ComboWdg::changed));
    }
    virtual ~ComboWdg() {}
    void changed (void);
};

void ComboWdg::changed(void)
{
    if ( _base ) {
            Glib::ustring value = _base->value_from_label(get_active_text());
            _base->set(value.c_str(), _doc, _node);
    }
    if (_changeSignal != NULL) {
        _changeSignal->emit();
    }
}

/**
 * Returns the value for the options label parameter
 */
Glib::ustring ParamRadioButton::value_from_label(const Glib::ustring label)
{
    Glib::ustring value = "";

    for (GSList * list = choices; list != NULL; list = g_slist_next(list)) {
        optionentry * entr = reinterpret_cast<optionentry *>(list->data);
        if ( !entr->guitext->compare(label) ) {
            value = *(entr->value);
            break;
        }
    }

    return value;

}

/**
 * Creates a combobox widget for an enumeration parameter.
 */
Gtk::Widget * ParamRadioButton::get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal)
{
    if (_gui_hidden) {
        return NULL;
    }

#if WITH_GTKMM_3_0
    Gtk::Box * hbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 4));
    hbox->set_homogeneous(false);
    Gtk::Box * vbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 0));
    vbox->set_homogeneous(false);
#else
    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox(false, 4));
    Gtk::VBox * vbox = Gtk::manage(new Gtk::VBox(false, 0));
#endif

    Gtk::Label * label = Gtk::manage(new Gtk::Label(_text, Gtk::ALIGN_START, Gtk::ALIGN_START));
    label->show();
    hbox->pack_start(*label, false, false, _indent);

    Gtk::ComboBoxText* cbt = 0;
    bool comboSet = false;
    if (_mode == MINIMAL) {
        cbt = Gtk::manage(new ComboWdg(this, doc, node, changeSignal));
        cbt->show();
        vbox->pack_start(*cbt, false, false);
    }

    // add choice strings as radiobuttons
    // and select last selected option (_value)
    Gtk::RadioButtonGroup group;
    for (GSList * list = choices; list != NULL; list = g_slist_next(list)) {
        optionentry * entr = reinterpret_cast<optionentry *>(list->data);
        Glib::ustring * text = entr->guitext;
        switch ( _mode ) {
            case MINIMAL:
            {
                cbt->append(*text);
                if (!entr->value->compare(_value)) {
                    cbt->set_active_text(*text);
                    comboSet = true;
                }
            }
            break;
            case COMPACT:
            case FULL:
            {
                ParamRadioButtonWdg * radio = Gtk::manage(new ParamRadioButtonWdg(group, *text, this, doc, node, changeSignal));
                radio->show();
                vbox->pack_start(*radio, true, true);
                if (!entr->value->compare(_value)) {
                    radio->set_active();
                }
            }
            break;
        }
    }

    if ( (_mode == MINIMAL) && !comboSet) {
        cbt->set_active(0);
    }

    vbox->show();
    hbox->pack_end(*vbox, false, false);
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
