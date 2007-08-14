#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_PATH_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_PATH_H

/*
 * Inkscape::LivePathEffectParameters
 *
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib/gtypes.h>
#include <2geom/path.h>

#include "ui/widget/registry.h"
#include <gtkmm/tooltips.h>

#include "live_effects/parameter/parameter.h"

#include <sigc++/sigc++.h>

namespace Inkscape {

namespace LivePathEffect {

class PathParam : public Geom::Piecewise<Geom::D2<Geom::SBasis> >, public Parameter {
public:
    PathParam(const Glib::ustring& label, const Glib::ustring& tip, const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr, Effect* effect);;
    ~PathParam();

    Gtk::Widget * param_getWidget();

    bool param_readSVGValue(const gchar * strvalue);
    gchar * param_writeSVGValue() const;

    sigc::signal <void> signal_path_pasted;

private:
    PathParam(const PathParam&);
    PathParam& operator=(const PathParam&);

    Gtk::Widget * _widget;
    Gtk::Tooltips * _tooltips;

    void param_write_to_repr(const char * svgd);

    void on_edit_button_click();
    void on_paste_button_click();
};


}; //namespace LivePathEffect

}; //namespace Inkscape

#endif
