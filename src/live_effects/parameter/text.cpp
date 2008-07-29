#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_TEXT_CPP

/*
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Authors:
 *   Maximilian Albert
 *   Johan Engelen
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/text.h"
#include "live_effects/effect.h"
#include "svg/svg.h"
#include "svg/stringstream.h"
#include <gtkmm.h>
#include "widgets/icon.h"
#include "ui/widget/registered-widget.h"
#include "inkscape.h"
#include "verbs.h"

namespace Inkscape {

namespace LivePathEffect {

TextParam::TextParam( const Glib::ustring& label, const Glib::ustring& tip,
                      const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                      Effect* effect, const Glib::ustring default_value )
    : Parameter(label, tip, key, wr, effect),
      defvalue(default_value)
{
    canvas_text = (SPCanvasText *) sp_canvastext_new(sp_desktop_tempgroup(inkscape_active_desktop()), Geom::Point(0,0), "");
    sp_canvastext_set_text (canvas_text, default_value.c_str());
    sp_canvastext_set_coords (canvas_text, 0, 0);
}

TextParam::~TextParam()
{
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
TextParam::setAnchor(double x_value, double y_value)
{
    anchor_x = x_value;
    anchor_y = y_value;
    sp_canvastext_set_anchor (canvas_text, anchor_x, anchor_y);
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
    return (gchar *) defvalue.c_str();
}

Gtk::Widget *
TextParam::param_newWidget(Gtk::Tooltips * /*tooltips*/)
{
    Inkscape::UI::Widget::RegisteredText *rsu = Gtk::manage(new Inkscape::UI::Widget::RegisteredText(
        param_label, param_tooltip, param_key, *param_wr, param_effect->getRepr(), param_effect->getSPDoc()));

    rsu->setText("");
    rsu->setProgrammatically = false;

    rsu->set_undo_parameters(SP_VERB_DIALOG_LIVE_PATH_EFFECT, _("Change text parameter"));

    return dynamic_cast<Gtk::Widget *> (rsu);
}

void
TextParam::param_setValue(const Glib::ustring newvalue)
{
    defvalue = newvalue;

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
