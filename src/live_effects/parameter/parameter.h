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

struct SPDesktop;
struct SPItem;

namespace Gtk {
    class Widget;
    class Tooltips;
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

enum ParamType {
    GENERAL_PARAM,
    SCALAR_PARAM,
    BOOL_PARAM,
    PATH_PARAM,
    POINT_PARAM,
    RANDOM_PARAM,
    ENUM_PARAM,
    INVALID_PARAM
};

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

    virtual void param_set_default() = 0;

    void printTypeName();
    virtual ParamType paramType() { return GENERAL_PARAM; }

    // This creates a new widget (newed with Gtk::manage(new ...);)
    virtual Gtk::Widget * param_newWidget(Gtk::Tooltips * tooltips) = 0;

    virtual Glib::ustring * param_getTooltip() { return &param_tooltip; };

    virtual void param_editOncanvas(SPItem * /*item*/, SPDesktop * /*dt*/) {};
    virtual void param_setup_nodepath(Inkscape::NodePath::Path */*np*/) {};

    virtual void param_transform_multiply(Geom::Matrix const& /*postmul*/, bool /*set*/) {};

    Glib::ustring param_key;
    Inkscape::UI::Widget::Registry * param_wr;
    Glib::ustring param_label;

    bool oncanvas_editable;

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
                gdouble default_value = 1.0);
    virtual ~ScalarParam();

    virtual ParamType paramType() { return SCALAR_PARAM; }

    virtual bool param_readSVGValue(const gchar * strvalue);
    virtual gchar * param_getSVGValue() const;

    virtual void param_set_default();
    void param_set_value(gdouble val);
    void param_make_integer(bool yes = true);
    void param_set_range(gdouble min, gdouble max);
    void param_set_digits(unsigned digits);
    void param_set_increments(double step, double page);

    virtual Gtk::Widget * param_newWidget(Gtk::Tooltips * tooltips);

    inline operator gdouble()
        { return value; };

protected:
    gdouble value;
    gdouble min;
    gdouble max;
    bool integer;
    gdouble defvalue;
    unsigned digits;
    double inc_step;
    double inc_page;

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
