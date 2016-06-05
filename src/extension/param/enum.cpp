/** \file
 * extension parameter for enumerations.
 *
 * It uses a Gtk:ComboBoxText widget in the extension UI.
 */

/*
 * Author:
 *   Johan Engelen <johan@shouraizou.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2006-2007 Johan Engelen
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtkmm/box.h>
#include <gtkmm/comboboxtext.h>
#include <glibmm/i18n.h>

#include "xml/node.h"
#include "extension/extension.h"
#include "document-private.h"
#include "sp-object.h"
#include "enum.h"
#include "preferences.h"

namespace Inkscape {
namespace Extension {

/* For internal use only.
     Note that value and guitext MUST be non-NULL. This is ensured by newing only at one location in the code where non-NULL checks are made. */
class enumentry {
public:
    enumentry (Glib::ustring &val, Glib::ustring &text) :
        value(val),
        guitext(text)
    {}

    Glib::ustring value;
    Glib::ustring guitext;
};


ParamComboBox::ParamComboBox(const gchar *name, const gchar *guitext, const gchar *desc,
                             const Parameter::_scope_t scope, bool gui_hidden, const gchar *gui_tip,
                             Inkscape::Extension::Extension *ext, Inkscape::XML::Node *xml)
    : Parameter(name, guitext, desc, scope, gui_hidden, gui_tip, ext)
    , _value(NULL)
    , _indent(0)
    , choices(NULL)
{
    const char *xmlval = NULL; // the value stored in XML

    if (xml != NULL) {
        // Read XML tree to add enumeration items:
        for (Inkscape::XML::Node *node = xml->firstChild(); node; node = node->next()) {
            char const * chname = node->name();
            if (!strcmp(chname, INKSCAPE_EXTENSION_NS "item") || !strcmp(chname, INKSCAPE_EXTENSION_NS "_item")) {
                Glib::ustring newguitext, newvalue;
                const char * contents = NULL;
                if (node->firstChild()) {
                    contents = node->firstChild()->content();
                }
                if (contents != NULL) {
                    // don't translate when 'item' but do translate when '_item'
                    // NOTE: internal extensions use build_from_mem and don't need _item but
                    //       still need to include if are to be localized
                    if (!strcmp(chname, INKSCAPE_EXTENSION_NS "_item")) {
                        if (node->attribute("msgctxt") != NULL) {
                            newguitext =  g_dpgettext2(NULL, node->attribute("msgctxt"), contents);
                        } else {
                            newguitext =  _(contents);
                        }
                    } else {
                        newguitext =  contents;
                    }
                } else
                    continue;

                const char * val = node->attribute("value");
                if (val != NULL) {
                    newvalue = val;
                } else {
                    newvalue = contents;
                }

                if ( (!newguitext.empty()) && (!newvalue.empty()) ) {   // logical error if this is not true here
                    choices = g_slist_append( choices, new enumentry(newvalue, newguitext) );
                }
            }
        }
    
        // Initialize _value with the default value from xml
        // for simplicity : default to the contents of the first xml-child
        if (xml->firstChild() && xml->firstChild()->firstChild()) {
            xmlval = xml->firstChild()->attribute("value");
        }

        const char *indent = xml->attribute("indent");
        if (indent != NULL) {
            _indent = atoi(indent) * 12;
        }
    }

    gchar * pref_name = this->pref_name();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Glib::ustring paramval = prefs ? prefs->getString(extension_pref_root + pref_name) : "";
    g_free(pref_name);

    if (!paramval.empty()) {
        _value = g_strdup(paramval.data());
    } else if (xmlval) {
        _value = g_strdup(xmlval);
    }
}

ParamComboBox::~ParamComboBox (void)
{
    //destroy choice strings
    for (GSList * list = choices; list != NULL; list = g_slist_next(list)) {
        delete (reinterpret_cast<enumentry *>(list->data));
    }
    g_slist_free(choices);

    g_free(_value);
}


/**
 * A function to set the \c _value.
 *
 * This function sets ONLY the internal value, but it also sets the value
 * in the preferences structure.  To put it in the right place, \c PREF_DIR
 *  and \c pref_name() are used.
 *
 * To copy the data into _value the old memory must be free'd first.
 * It is important to note that \c g_free handles \c NULL just fine.  Then
 * the passed in value is duplicated using \c g_strdup().
 *
 * @param  in   The value to set.
 * @param  doc  A document that should be used to set the value.
 * @param  node The node where the value may be placed.
 */
const gchar *ParamComboBox::set(const gchar * in, SPDocument * /*doc*/, Inkscape::XML::Node * /*node*/)
{
    if (in == NULL) {
        return NULL; /* Can't have NULL string */
    }

    Glib::ustring settext;
    for (GSList * list = choices; list != NULL; list = g_slist_next(list)) {
        enumentry * entr = reinterpret_cast<enumentry *>(list->data);
        if ( !entr->guitext.compare(in) ) {
            settext = entr->value;
            break;  // break out of for loop
        }
    }
    if (!settext.empty()) {
        if (_value != NULL) {
            g_free(_value);
        }
        _value = g_strdup(settext.data());
        gchar * prefname = this->pref_name();
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setString(extension_pref_root + prefname, _value);
        g_free(prefname);
    }

    return _value;
}

/**
 * function to test if \c guitext is selectable
 */
bool ParamComboBox::contains(const gchar * guitext, SPDocument const * /*doc*/, Inkscape::XML::Node const * /*node*/) const
{
    if (guitext == NULL) {
        return false; /* Can't have NULL string */
    }

    for (GSList * list = choices; list != NULL; list = g_slist_next(list)) {
        enumentry * entr = reinterpret_cast<enumentry *>(list->data);
        if ( !entr->guitext.compare(guitext) )
            return true;
    }
    // if we did not find the guitext in this ParamComboBox:
    return false;
}

void
ParamComboBox::changed (void) {

}

void ParamComboBox::string(std::string &string) const
{
    string += _value;
}




/** A special category of Gtk::Entry to handle string parameteres. */
class ParamComboBoxEntry : public Gtk::ComboBoxText {
private:
    ParamComboBox * _pref;
    SPDocument * _doc;
    Inkscape::XML::Node * _node;
    sigc::signal<void> * _changeSignal;
public:
    /**
     * Build a string preference for the given parameter.
     * @param  pref  Where to get the string from, and where to put it
     *                 when it changes.
     */
    ParamComboBoxEntry (ParamComboBox * pref, SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal) :
        Gtk::ComboBoxText(), _pref(pref), _doc(doc), _node(node), _changeSignal(changeSignal) {
        this->signal_changed().connect(sigc::mem_fun(this, &ParamComboBoxEntry::changed));
    };
    void changed (void);
};

/**
 * Respond to the text box changing.
 *
 * This function responds to the box changing by grabbing the value
 * from the text box and putting it in the parameter.
 */
void
ParamComboBoxEntry::changed (void)
{
    Glib::ustring data = this->get_active_text();
    _pref->set(data.c_str(), _doc, _node);
    if (_changeSignal != NULL) {
        _changeSignal->emit();
    }
}

/**
 * Creates a combobox widget for an enumeration parameter.
 */
Gtk::Widget *ParamComboBox::get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal)
{
    if (_gui_hidden) {
        return NULL;
    }

    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox(false, 4));
    Gtk::Label * label = Gtk::manage(new Gtk::Label(_text, Gtk::ALIGN_START));
    label->show();
    hbox->pack_start(*label, false, false, _indent);

    ParamComboBoxEntry * combo = Gtk::manage(new ParamComboBoxEntry(this, doc, node, changeSignal));
    // add choice strings:
    Glib::ustring settext;
    for (GSList * list = choices; list != NULL; list = g_slist_next(list)) {
        enumentry * entr = reinterpret_cast<enumentry *>(list->data);
        Glib::ustring text = entr->guitext;
        combo->append(text);

        if ( _value && !entr->value.compare(_value) ) {
            settext = entr->guitext;
        }
    }
    if (!settext.empty()) {
        combo->set_active_text(settext);
    }

    combo->show();
    hbox->pack_start(*combo, true, true);

    hbox->show();

    return dynamic_cast<Gtk::Widget *>(hbox);
}


}  // namespace Extension
}  // namespace Inkscape

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
