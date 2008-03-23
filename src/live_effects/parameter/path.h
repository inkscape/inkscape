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

#include <gtkmm/tooltips.h>

#include "live_effects/parameter/parameter.h"

#include <sigc++/sigc++.h>

namespace Inkscape {

namespace LivePathEffect {

class PathParam : public Parameter {
public:
    PathParam ( const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key,
                Inkscape::UI::Widget::Registry* wr,
                Effect* effect,
                const gchar * default_value = "M0,0 L1,1");
    virtual ~PathParam();

    std::vector<Geom::Path> const & get_pathvector();
    Geom::Piecewise<Geom::D2<Geom::SBasis> > const & get_pwd2();

    virtual Gtk::Widget * param_newWidget(Gtk::Tooltips * tooltips);

    bool param_readSVGValue(const gchar * strvalue);
    gchar * param_writeSVGValue() const;

    void param_set_default();
    void param_set_and_write_default();
    void param_set_and_write_new_value (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & newpath);

    virtual void param_editOncanvas(SPItem * item, SPDesktop * dt);
    virtual void param_setup_nodepath(Inkscape::NodePath::Path *np);

    virtual void param_transform_multiply(Geom::Matrix const& /*postmul*/, bool /*set*/);

    sigc::signal <void> signal_path_pasted;
    sigc::signal <void> signal_path_changed;

protected:
    std::vector<Geom::Path> _pathvector;   // this is primary data storage, since it is closest to SVG.

    Geom::Piecewise<Geom::D2<Geom::SBasis> > _pwd2; // secondary, hence the bool must_recalculate_pwd2
    bool must_recalculate_pwd2; // set when _pathvector was updated, but _pwd2 not
    void ensure_pwd2();  // ensures _pwd2 is up to date

    bool referring;   // set when referring to another path, i.e. does not have its own pwd2, but should get it from another path
    void update_from_referred();  // updates path data by looking up refered path

    void on_edit_button_click();
    void on_paste_button_click();
    void on_copy_button_click();

    gchar * defvalue;

private:
    PathParam(const PathParam&);
    PathParam& operator=(const PathParam&);
};


} //namespace LivePathEffect

} //namespace Inkscape

#endif
