#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_H

/*
 * Inkscape::LivePathEffectParameters
 *
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/ustring.h>
#include <2geom/forward.h>
#include <2geom/pathvector.h>

// In gtk2, this wasn't an issue; we could toss around
// G_MAXDOUBLE and not worry about size allocations. But
// in gtk3, it is an issue: it allocates widget size for the maxmium
// value you pass to it, leading to some insane lengths.
// If you need this to be more, please be conservative about it.
const double SCALARPARAM_G_MAXDOUBLE = 10000000000.0; // TODO fixme: using an arbitrary large number as a magic value seems fragile.

class KnotHolder;
class SPLPEItem;
class SPDesktop;
class SPItem;

namespace Gtk {
    class Widget;
}

namespace Inkscape {

namespace NodePath {
    class Path ;
}

namespace UI {
namespace Widget {
    class Registry;
}
}

namespace LivePathEffect {

class Effect;

class Parameter {
public:
    Parameter(  const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key,
                Inkscape::UI::Widget::Registry* wr,
                Effect* effect);
    virtual ~Parameter() {};

    virtual bool param_readSVGValue(const gchar * strvalue) = 0;   // returns true if new value is valid / accepted.
    virtual gchar * param_getSVGValue() const = 0;
    void write_to_SVG();
 
    virtual void param_set_default() = 0;

    // This creates a new widget (newed with Gtk::manage(new ...);)
    virtual Gtk::Widget * param_newWidget() = 0;

    virtual Glib::ustring * param_getTooltip() { return &param_tooltip; };

    // overload these for your particular parameter to make it provide knotholder handles or canvas helperpaths
    virtual bool providesKnotHolderEntities() const { return false; }
    virtual void addKnotHolderEntities(KnotHolder */*knotholder*/, SPDesktop */*desktop*/, SPItem */*item*/) {};
    virtual void addCanvasIndicators(SPLPEItem const*/*lpeitem*/, std::vector<Geom::PathVector> &/*hp_vec*/) {};

    virtual void param_editOncanvas(SPItem * /*item*/, SPDesktop * /*dt*/) {};
    virtual void param_setup_nodepath(Inkscape::NodePath::Path */*np*/) {};

    virtual void param_transform_multiply(Geom::Affine const& /*postmul*/, bool /*set*/) {};

    Glib::ustring param_key;
    Inkscape::UI::Widget::Registry * param_wr;
    Glib::ustring param_label;

    bool oncanvas_editable;
    bool widget_is_visible;

protected:
    Glib::ustring param_tooltip;

    Effect* param_effect;

    void param_write_to_repr(const char * svgd);

private:
    Parameter(const Parameter&);
    Parameter& operator=(const Parameter&);
};


class ScalarParam : public Parameter {
public:
    ScalarParam(  const Glib::ustring& label,
                const Glib::ustring& tip,
                const Glib::ustring& key,
                Inkscape::UI::Widget::Registry* wr,
                Effect* effect,
                gdouble default_value = 1.0,
                bool no_widget = false);
    virtual ~ScalarParam();

    virtual bool param_readSVGValue(const gchar * strvalue);
    virtual gchar * param_getSVGValue() const;

    virtual void param_set_default();
    void param_set_value(gdouble val);
    void param_make_integer(bool yes = true);
    void param_set_range(gdouble min, gdouble max);
    void param_set_digits(unsigned digits);
    void param_set_increments(double step, double page);

    void addSlider(bool add_slider_widget) { add_slider = add_slider_widget; };

    void param_overwrite_widget(bool overwrite_widget);
    virtual Gtk::Widget * param_newWidget();

    inline operator gdouble() const { return value; };

protected:
    gdouble value;
    gdouble min;
    gdouble max;
    bool integer;
    gdouble defvalue;
    unsigned digits;
    double inc_step;
    double inc_page;
    bool add_slider;
    bool overwrite_widget;
    bool hide_widget;

private:
    ScalarParam(const ScalarParam&);
    ScalarParam& operator=(const ScalarParam&);
};

} //namespace LivePathEffect

} //namespace Inkscape

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
