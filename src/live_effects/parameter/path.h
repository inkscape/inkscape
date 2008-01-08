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

namespace Gtk {
    class Button;
}

namespace Inkscape {

namespace LivePathEffect {

class PathParam : public Geom::Piecewise<Geom::D2<Geom::SBasis> >, public Parameter {
public:
    PathParam ( const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key,
                Inkscape::UI::Widget::Registry* wr,
                Effect* effect,
                const gchar * default_value = "M0,0 L1,1");
    virtual ~PathParam();

    Gtk::Widget * param_getWidget();

    bool param_readSVGValue(const gchar * strvalue);
    gchar * param_writeSVGValue() const;

    void param_set_default();

    void param_set_and_write_new_value (Geom::Piecewise<Geom::D2<Geom::SBasis> > newpath);

    void param_editOncanvas(SPItem * item, SPDesktop * dt);
    void param_setup_nodepath(Inkscape::NodePath::Path *np);

    virtual void param_transform_multiply(Geom::Matrix const& /*postmul*/, bool /*set*/);

    sigc::signal <void> signal_path_pasted;
    sigc::signal <void> signal_path_changed;

private:
    PathParam(const PathParam&);
    PathParam& operator=(const PathParam&);

    Gtk::Widget * _widget;
    Gtk::Tooltips * _tooltips;

    void on_edit_button_click();
    void on_paste_button_click();

    gchar * defvalue;

    Gtk::Button * edit_button;
};


} //namespace LivePathEffect

} //namespace Inkscape

#endif
