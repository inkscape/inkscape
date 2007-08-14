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

#include "extension.h"
#include "paramcolor.h"

#include "color.h"
#include "widgets/sp-color-selector.h"
#include "widgets/sp-color-notebook.h"


namespace Inkscape {
namespace Extension {
	
void sp_color_param_changed(SPColorSelector *csel, GObject *cp);

     
/** \brief  Free the allocated data. */
ParamColor::~ParamColor(void)
{
    g_free(_value);
}
     
SPColor* 
ParamColor::set (SPColor* in, SPDocument * doc, Inkscape::XML::Node * node)
{
    _value = in;

    gchar * prefname = this->pref_name();
    prefs_set_string_attribute(PREF_DIR, prefname, this->string()->c_str());
    g_free(prefname);

    return _value;
}

/** \brief  Initialize the object, to do that, copy the data. */
ParamColor::ParamColor (const gchar * name, const gchar * guitext, const gchar * desc, const Parameter::_scope_t scope, Inkscape::Extension::Extension * ext, Inkscape::XML::Node * xml) :
    Parameter(name, guitext, desc, scope, ext), _value(NULL)
{
    const char * defaulthex = NULL;
    if (sp_repr_children(xml) != NULL)
        defaulthex = sp_repr_children(xml)->content();

    gchar * pref_name = this->pref_name();
    const gchar * paramval = prefs_get_string_attribute(PREF_DIR, pref_name);
    g_free(pref_name);

    if (paramval != NULL)
        defaulthex = paramval;
    
	const char* hexMark = strchr(defaulthex, '#');
	if (hexMark != NULL)
	    defaulthex++;// = hexMark;
	
	if (strlen(defaulthex) == 6) {
		int r = 0, g = 0, b = 0;	
		std::stringstream ss;
		ss << g_strndup(defaulthex, 2);
		ss >> std::hex >> r;
		ss << g_strndup(defaulthex + 2, 2);
		ss >> std::hex >> g;
		ss << defaulthex + 4;
		ss >> std::hex >> b;
		g_free(ss);
			
		SPColor defaultColor;
		sp_color_set_rgb_float(&defaultColor, r / 255.0, g / 255.0, b / 255.0);
		_value = &defaultColor;
	}

    return;
}

/** \brief  Return the value as a string */
Glib::ustring *
ParamColor::string (void)
{
    float rgb[3];
    sp_color_get_rgb_floatv(_value, rgb);
    char hex[8];
	snprintf(hex, 8, "#%02X%02X%02X", (int)(rgb[0] * 255), (int)(rgb[1] * 255), (int)(rgb[2] * 255));
	
	Glib::ustring* ret = new Glib::ustring(hex);
	
	printf("ParamColor::string = '%s'\n", hex);
	
    return ret;
}

Gtk::Widget *
ParamColor::get_widget (SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal)
{
	_changeSignal = new sigc::signal<void>(*changeSignal);
	Gtk::HBox * hbox = Gtk::manage(new Gtk::HBox(false, 4));
	SPColorSelector* spColorSelector = (SPColorSelector*)sp_color_selector_new(SP_TYPE_COLOR_NOTEBOOK, SP_COLORSPACE_TYPE_RGB);
	
	ColorSelector* colorSelector = spColorSelector->base;
	if (_value == NULL) {
		_value = new SPColor();
		sp_color_set_rgb_float(_value, 1.0, 0.0, 0.0);
	}
    colorSelector->setColor(*_value);

	hbox->pack_start (*Glib::wrap(&spColorSelector->vbox), true, true, 0);
	g_signal_connect(G_OBJECT(spColorSelector), "dragged",  G_CALLBACK(sp_color_param_changed), (void*)this);
	g_signal_connect(G_OBJECT(spColorSelector), "released", G_CALLBACK(sp_color_param_changed), (void*)this);
	g_signal_connect(G_OBJECT(spColorSelector), "changed",  G_CALLBACK(sp_color_param_changed), (void*)this);

	gtk_widget_show(GTK_WIDGET(spColorSelector));
	hbox->show();
    
    return dynamic_cast<Gtk::Widget *>(hbox);
}

void
sp_color_param_changed(SPColorSelector *csel, GObject *obj)
{
	SPColor color;
    float alpha;
    csel->base->getColorAlpha(color, &alpha);
    guint32 rgba = sp_color_get_rgba32_falpha(&color, alpha);
	

    ParamColor* ptr = (ParamColor* )obj;
	ptr->set(&color, NULL, NULL); 
	
	ptr->_changeSignal->emit();
}

};  /* namespace Extension */
};  /* namespace Inkscape */
