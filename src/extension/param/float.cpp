/*
 * Copyright (C) 2005-2007 Authors:
 *   Ted Gould <ted@gould.cx>
 *   Johan Engelen <johan@shouraizou.nl> *
 *   Jon A. Cruz <jon@joncruz.org>
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/scale.h>
#include "ui/widget/spinbutton.h"
#include "ui/widget/spin-scale.h"

#include "xml/node.h"
#include "extension/extension.h"
#include "preferences.h"
#include "float.h"

namespace Inkscape {
namespace Extension {


/** Use the superclass' allocator and set the \c _value. */
ParamFloat::ParamFloat (const gchar * name,
                        const gchar * guitext,
                        const gchar * desc,
                        const Parameter::_scope_t scope,
                        bool gui_hidden,
                        const gchar * gui_tip,
                        Inkscape::Extension::Extension * ext,
                        Inkscape::XML::Node * xml,
                        AppearanceMode mode) :
        Parameter(name, guitext, desc, scope, gui_hidden, gui_tip, ext),
                  _value(0.0), _mode(mode), _indent(0), _min(0.0), _max(10.0)
{
    const gchar * defaultval = NULL;
    if (xml->firstChild() != NULL) {
        defaultval = xml->firstChild()->content();
    }
    if (defaultval != NULL) {
        _value = g_ascii_strtod (defaultval,NULL);
    }

    const char * maxval = xml->attribute("max");
    if (maxval != NULL) {
        _max = g_ascii_strtod (maxval,NULL);
    }

    const char * minval = xml->attribute("min");
    if (minval != NULL) {
        _min = g_ascii_strtod (minval,NULL);
    }

    _precision = 1;
    const char * precision = xml->attribute("precision");
    if (precision != NULL) {
        _precision = atoi(precision);
    }

    /* We're handling this by just killing both values */
    if (_max < _min) {
        _max = 10.0;
        _min = 0.0;
    }

    const char * indent = xml->attribute("indent");
    if (indent != NULL) {
        _indent = atoi(indent) * 12;
    }

    gchar * pref_name = this->pref_name();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    _value = prefs->getDouble(extension_pref_root + pref_name, _value);
    g_free(pref_name);

    // std::cout << "New Float::  value: " << _value << "  max: " << _max << "  min: " << _min << std::endl;

    if (_value > _max) {
        _value = _max;
    }
    if (_value < _min) {
        _value = _min;
    }

    return;
}

/**
 * A function to set the \c _value.
 *
 * This function sets the internal value, but it also sets the value
 * in the preferences structure.  To put it in the right place, \c PREF_DIR
 * and \c pref_name() are used.
 *
 * @param  in   The value to set to.
 * @param  doc  A document that should be used to set the value.
 * @param  node The node where the value may be placed.
 */
float ParamFloat::set(float in, SPDocument * /*doc*/, Inkscape::XML::Node * /*node*/)
{
    _value = in;
    if (_value > _max) {
        _value = _max;
    }
    if (_value < _min) {
        _value = _min;
    }

    gchar * prefname = this->pref_name();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble(extension_pref_root + prefname, _value);
    g_free(prefname);

    return _value;
}

void ParamFloat::string(std::string &string) const
{
    char startstring[G_ASCII_DTOSTR_BUF_SIZE];
    g_ascii_dtostr(startstring, G_ASCII_DTOSTR_BUF_SIZE, _value);
    string += startstring;
    return;
}

/** A class to make an adjustment that uses Extension params. */
class ParamFloatAdjustment : public Gtk::Adjustment {
    /** The parameter to adjust. */
    ParamFloat * _pref;
    SPDocument * _doc;
    Inkscape::XML::Node * _node;
    sigc::signal<void> * _changeSignal;
public:
    /** Make the adjustment using an extension and the string
                describing the parameter. */
    ParamFloatAdjustment (ParamFloat * param, SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal) :
            Gtk::Adjustment(0.0, param->min(), param->max(), 0.1, 1.0, 0), _pref(param), _doc(doc), _node(node), _changeSignal(changeSignal) {
        this->set_value(_pref->get(NULL, NULL) /* \todo fix */);
        this->signal_value_changed().connect(sigc::mem_fun(this, &ParamFloatAdjustment::val_changed));
        return;
    };

    void val_changed (void);
}; /* class ParamFloatAdjustment */

/**
 * A function to respond to the value_changed signal from the adjustment.
 *
 * This function just grabs the value from the adjustment and writes
 * it to the parameter.  Very simple, but yet beautiful.
 */
void ParamFloatAdjustment::val_changed(void)
{
    //std::cout << "Value Changed to: " << this->get_value() << std::endl;
    _pref->set(this->get_value(), _doc, _node);
    if (_changeSignal != NULL) {
        _changeSignal->emit();
    }
    return;
}

/**
 * Creates a Float Adjustment for a float parameter.
 *
 * Builds a hbox with a label and a float adjustment in it.
 */
Gtk::Widget * ParamFloat::get_widget(SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal)
{
    if (_gui_hidden) {
        return NULL;
    }

    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox(false, 4));

#if WITH_GTKMM_3_0
    ParamFloatAdjustment * pfa = new ParamFloatAdjustment(this, doc, node, changeSignal);
    Glib::RefPtr<Gtk::Adjustment> fadjust(pfa);
#else
    ParamFloatAdjustment * fadjust = Gtk::manage(new ParamFloatAdjustment(this, doc, node, changeSignal));
#endif
    
    if (_mode == FULL) {

        UI::Widget::SpinScale *scale = new UI::Widget::SpinScale(_text, fadjust, _precision);
        scale->set_size_request(400, -1);
        scale->show();
        hbox->pack_start(*scale, false, false);

    }
    else if (_mode == MINIMAL) {

        Gtk::Label * label = Gtk::manage(new Gtk::Label(_text, Gtk::ALIGN_START));
        label->show();
        hbox->pack_start(*label, true, true, _indent);

#if WITH_GTKMM_3_0
    Inkscape::UI::Widget::SpinButton * spin = Gtk::manage(new Inkscape::UI::Widget::SpinButton(fadjust, 0.1, _precision));
#else
    Inkscape::UI::Widget::SpinButton * spin = Gtk::manage(new Inkscape::UI::Widget::SpinButton(*fadjust, 0.1, _precision));
#endif
        spin->show();
        hbox->pack_start(*spin, false, false);
    }

    hbox->show();

    return dynamic_cast<Gtk::Widget *>(hbox);
}


}  /* namespace Extension */
}  /* namespace Inkscape */
