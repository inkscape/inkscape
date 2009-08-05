/*
 * Copyright (C) 2005-2007 Authors:
 *   Ted Gould <ted@gould.cx>
 *   Johan Engelen <johan@shouraizou.nl>
 *   Christopher Brown <audiere@gmail.com>
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
#include "widgets/sp-color-selector.h"
#include "widgets/sp-color-notebook.h"
#include "preferences.h"


namespace Inkscape {
namespace Extension {

void sp_color_param_changed(SPColorSelector *csel, GObject *cp);


/** \brief  Free the allocated data. */
ParamColor::~ParamColor(void)
{

}

guint32
ParamColor::set( guint32 in, Document * /*doc*/, Inkscape::XML::Node * /*node*/ )
{
    _value = in;

    gchar * prefname = this->pref_name();
    std::string value;
    string(value);
    
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setString(extension_pref_root + prefname, value);
    g_free(prefname);

    return _value;
}

/** \brief  Initialize the object, to do that, copy the data. */
ParamColor::ParamColor (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, bool gui_hidden, const gchar * gui_tip, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
    Parameter(name, guitext, desc, scope, gui_hidden, gui_tip, ext)
{
    const char * defaulthex = NULL;
    if (sp_repr_children(xml) != NULL)
        defaulthex = sp_repr_children(xml)->content();

    gchar * pref_name = this->pref_name();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Glib::ustring paramval = prefs->getString(extension_pref_root + pref_name);
    g_free(pref_name);

    if (!paramval.empty())
        defaulthex = paramval.data();

    _value = atoi(defaulthex);

    return;
}

void
ParamColor::string (std::string &string)
{
    char str[16];
    sprintf(str, "%i", _value);
    string += str;
    return;
}

Gtk::Widget *
ParamColor::get_widget( Document * /*doc*/, Inkscape::XML::Node * /*node*/, sigc::signal<void> * changeSignal )
{
	if (_gui_hidden) return NULL;

    _changeSignal = new sigc::signal<void>(*changeSignal);
    Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox(false, 4));
    SPColorSelector* spColorSelector = (SPColorSelector*)sp_color_selector_new(SP_TYPE_COLOR_NOTEBOOK);

    ColorSelector* colorSelector = spColorSelector->base;
    if (_value < 1) {
        _value = 0xFF000000;
    }
    SPColor *color = new SPColor( _value );
    float alpha = (_value & 0xff) / 255.0F;
    colorSelector->setColorAlpha(*color, alpha);

    hbox->pack_start (*Glib::wrap(&spColorSelector->vbox), true, true, 0);
    g_signal_connect(G_OBJECT(spColorSelector), "changed",  G_CALLBACK(sp_color_param_changed), (void*)this);

    gtk_widget_show(GTK_WIDGET(spColorSelector));
    hbox->show();

    return dynamic_cast<Gtk::Widget *>(hbox);
}

void
sp_color_param_changed(SPColorSelector *csel, GObject *obj)
{
    const SPColor color = csel->base->getColor();
    float alpha = csel->base->getAlpha();

    ParamColor* ptr = (ParamColor*)obj;
    ptr->set(color.toRGBA32( alpha ), NULL, NULL);

    ptr->_changeSignal->emit();
}

};  /* namespace Extension */
};  /* namespace Inkscape */
