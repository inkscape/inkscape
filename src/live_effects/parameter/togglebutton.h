#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_TOGGLEBUTTON_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_TOGGLEBUTTON_H

/*
 * Copyright (C) Jabiertxo Arraiza Cenoz 2014
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <sigc++/connection.h>
#include <sigc++/signal.h>

#include "live_effects/parameter/parameter.h"
#include "icon-size.h"
#include "ui/widget/registered-widget.h"

namespace Inkscape {

namespace LivePathEffect {

/**
 * class ToggleButtonParam:
 *    represents a Gtk::ToggleButton as a Live Path Effect parameter
 */
class ToggleButtonParam : public Parameter {
public:
    ToggleButtonParam( const Glib::ustring& label,
               const Glib::ustring& tip,
               const Glib::ustring& key,
               Inkscape::UI::Widget::Registry* wr,
               Effect* effect,
               bool default_value = false,
               const Glib::ustring& inactive_label = "",
               char const * icon_active = NULL,
               char const * icon_inactive = NULL,
               Inkscape::IconSize icon_size  = Inkscape::ICON_SIZE_SMALL_TOOLBAR);
    virtual ~ToggleButtonParam();

    virtual Gtk::Widget * param_newWidget();

    virtual bool param_readSVGValue(const gchar * strvalue);
    virtual gchar * param_getSVGValue() const;

    void param_setValue(bool newvalue);
    virtual void param_set_default();

    bool get_value() const { return value; };

    inline operator bool() const { return value; };
    
    sigc::signal<void>& signal_toggled() { return _signal_toggled; }
    virtual void toggled();

private:
    ToggleButtonParam(const ToggleButtonParam&);
    ToggleButtonParam& operator=(const ToggleButtonParam&);

    void refresh_button();
    bool value;
    bool defvalue;
    const Glib::ustring inactive_label;
    const char * _icon_active;
    const char * _icon_inactive;
    Inkscape::IconSize  _icon_size;
    Inkscape::UI::Widget::RegisteredToggleButton * checkwdg;

    sigc::signal<void> _signal_toggled;
    sigc::connection _toggled_connection;
};


} //namespace LivePathEffect

} //namespace Inkscape

#endif
