/*
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Authors:
 *   Maximilian Albert
 *   Johan Engelen
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/widget/registered-widget.h"
#include <glibmm/i18n.h>

#include "live_effects/parameter/text.h"
#include "live_effects/effect.h"
#include "svg/svg.h"
#include "svg/stringstream.h"
#include "widgets/icon.h"
#include "inkscape.h"
#include "verbs.h"
#include "display/canvas-text.h"

#include <2geom/sbasis-geometric.h>

namespace Inkscape {

namespace LivePathEffect {

TextParam::TextParam( const Glib::ustring& label, const Glib::ustring& tip,
                      const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                      Effect* effect, const Glib::ustring default_value )
    : Parameter(label, tip, key, wr, effect),
      value(default_value),
      defvalue(default_value)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP; // FIXME: we shouldn't use this!
    canvas_text = (SPCanvasText *) sp_canvastext_new(desktop->getTempGroup(), desktop, Geom::Point(0,0), "");
    sp_canvastext_set_text (canvas_text, default_value.c_str());
    sp_canvastext_set_coords (canvas_text, 0, 0);
}

void
TextParam::param_set_default()
{
    param_setValue(defvalue);
}

void
TextParam::setPos(Geom::Point pos)
{
    sp_canvastext_set_coords (canvas_text, pos);
}

void
TextParam::setPosAndAnchor(const Geom::Piecewise<Geom::D2<Geom::SBasis> > &pwd2,
                           const double t, const double length, bool /*use_curvature*/)
{
    using namespace Geom;

    Piecewise<D2<SBasis> > pwd2_reparam = arc_length_parametrization(pwd2, 2 , 0.1);
    double t_reparam = pwd2_reparam.cuts.back() * t;
    Point pos = pwd2_reparam.valueAt(t_reparam);
    Point dir = unit_vector(derivative(pwd2_reparam).valueAt(t_reparam));
    Point n = -rot90(dir);
    double angle = Geom::angle_between(dir, Point(1,0));

    sp_canvastext_set_coords(canvas_text, pos + n * length);
    sp_canvastext_set_anchor_manually(canvas_text, std::sin(angle), -std::cos(angle));
}

void
TextParam::setAnchor(double x_value, double y_value)
{
    anchor_x = x_value;
    anchor_y = y_value;
    sp_canvastext_set_anchor_manually (canvas_text, anchor_x, anchor_y);
}

bool
TextParam::param_readSVGValue(const gchar * strvalue)
{
    param_setValue(strvalue);
    return true;
}

gchar *
TextParam::param_getSVGValue() const
{
    return g_strdup(value.c_str());
}

Gtk::Widget *
TextParam::param_newWidget()
{
    Inkscape::UI::Widget::RegisteredText *rsu = Gtk::manage(new Inkscape::UI::Widget::RegisteredText(
        param_label, param_tooltip, param_key, *param_wr, param_effect->getRepr(), param_effect->getSPDoc()));

    rsu->setText(value.c_str());
    rsu->setProgrammatically = false;

    rsu->set_undo_parameters(SP_VERB_DIALOG_LIVE_PATH_EFFECT, _("Change text parameter"));

    return dynamic_cast<Gtk::Widget *> (rsu);
}

void
TextParam::param_setValue(const Glib::ustring newvalue)
{
    value = newvalue;

    sp_canvastext_set_text (canvas_text, newvalue.c_str());
}

} /* namespace LivePathEffect */

} /* namespace Inkscape */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
