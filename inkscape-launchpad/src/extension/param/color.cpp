/*
 * Copyright (C) 2005-2007 Authors:
 *   Ted Gould <ted@gould.cx>
 *   Johan Engelen <johan@shouraizou.nl>
 *   Christopher Brown <audiere@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <iostream>
#include <sstream>

#include <gtkmm/adjustment.h>
#include <gtkmm/box.h>
#include <gtkmm/spinbutton.h>

#include <xml/node.h>

#include "../extension.h"
#include "color.h"

#include <color.h>
#include "ui/widget/color-notebook.h"
#include "preferences.h"

namespace Inkscape {
namespace Extension {

ParamColor::~ParamColor(void)
{
    _color_changed.disconnect();
}

guint32 ParamColor::set( guint32 in, SPDocument * /*doc*/, Inkscape::XML::Node * /*node*/ )
{
    _color_changed.block(true);
    _color.setValue(in);
    _color_changed.block(false);

    gchar * prefname = this->pref_name();
    std::string value;
    string(value);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setString(extension_pref_root + prefname, value);
    g_free(prefname);

    return in;
}

ParamColor::ParamColor (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, bool gui_hidden, const gchar * gui_tip, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
    Parameter(name, guitext, desc, scope, gui_hidden, gui_tip, ext),
    _changeSignal(0)
{
    const char * defaulthex = NULL;
    if (xml->firstChild() != NULL)
        defaulthex = xml->firstChild()->content();

    gchar * pref_name = this->pref_name();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Glib::ustring paramval = prefs->getString(extension_pref_root + pref_name);
    g_free(pref_name);

    if (!paramval.empty())
        defaulthex = paramval.data();

    if (defaulthex) {
        _color.setValue(atoi(defaulthex));
    }
    _color_changed = _color.signal_changed.connect(sigc::mem_fun(this, &ParamColor::_onColorChanged));

}

void ParamColor::string(std::string &string) const
{
    char str[16];
    snprintf(str, 16, "%i", _color.value());
    string += str;
}

Gtk::Widget *ParamColor::get_widget( SPDocument * /*doc*/, Inkscape::XML::Node * /*node*/, sigc::signal<void> * changeSignal )
{
    using Inkscape::UI::Widget::ColorNotebook;

    if (_gui_hidden) return NULL; 

    if (changeSignal) {
        _changeSignal = new sigc::signal<void>(*changeSignal);
    }

    if (_color.value() < 1) {
        _color_changed.block(true);
        _color.setValue(0xFF000000);
        _color_changed.block(false);
    }

    Gtk::HBox *hbox = Gtk::manage(new Gtk::HBox(false, 4));
    Gtk::Widget *selector = Gtk::manage(new ColorNotebook(_color));
    hbox->pack_start (*selector, true, true, 0);
    selector->show();
    hbox->show();
    return hbox;
}

void ParamColor::_onColorChanged()
{
    if (_changeSignal)
        _changeSignal->emit();
}

};  /* namespace Extension */
};  /* namespace Inkscape */
