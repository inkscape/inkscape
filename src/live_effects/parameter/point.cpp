#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_POINT_CPP

/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/point.h"
#include "live_effects/parameter/pointparam-knotholder.h"
#include "live_effects/effect.h"
#include "svg/svg.h"
#include "svg/stringstream.h"
#include <gtkmm.h>
#include "ui/widget/point.h"
#include "widgets/icon.h"
#include "ui/widget/registered-widget.h"
#include "inkscape.h"
#include "verbs.h"

// needed for on-canvas editting:
#include "tools-switch.h"
#include "node-context.h"
#include "shape-editor.h"
#include "desktop.h"
#include "selection.h"

namespace Inkscape {

namespace LivePathEffect {

PointParam::PointParam( const Glib::ustring& label, const Glib::ustring& tip,
                        const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                        Effect* effect, Geom::Point default_value )
    : Geom::Point(default_value), Parameter(label, tip, key, wr, effect), defvalue(default_value)
{
    oncanvas_editable = true;

    knot_shape = SP_KNOT_SHAPE_SQUARE;
    knot_mode  = SP_KNOT_MODE_XOR;
    knot_color = 0x00ff0000;
}

PointParam::~PointParam()
{
}

void
PointParam::param_set_default()
{
    param_setValue(defvalue);
}

bool
PointParam::param_readSVGValue(const gchar * strvalue)
{
    gchar ** strarray = g_strsplit(strvalue, ",", 2);
    double newx, newy;
    unsigned int success = sp_svg_number_read_d(strarray[0], &newx);
    success += sp_svg_number_read_d(strarray[1], &newy);
    g_strfreev (strarray);
    if (success == 2) {
        param_setValue( Geom::Point(newx, newy) );
        return true;
    }
    return false;
}

gchar *
PointParam::param_writeSVGValue() const
{
    Inkscape::SVGOStringStream os;
    os << (*this)[0] << "," << (*this)[1];
    gchar * str = g_strdup(os.str().c_str());
    return str;
}

Gtk::Widget *
PointParam::param_newWidget(Gtk::Tooltips * tooltips)
{
    Inkscape::UI::Widget::RegisteredPoint * pointwdg = Gtk::manage(
        new Inkscape::UI::Widget::RegisteredPoint( param_label,
                                                   param_tooltip,
                                                   param_key,
                                                   *param_wr,
                                                   param_effect->getRepr(),
                                                   param_effect->getSPDoc() ) );
    pointwdg->setValue( (*this)[0], (*this)[1] );
    pointwdg->clearProgrammatically();
    pointwdg->set_undo_parameters(SP_VERB_DIALOG_LIVE_PATH_EFFECT, _("Change point parameter"));

    Gtk::Widget*  pIcon = Gtk::manage( sp_icon_get_icon( "draw_node", Inkscape::ICON_SIZE_BUTTON) );
    Gtk::Button * pButton = Gtk::manage(new Gtk::Button());
    pButton->set_relief(Gtk::RELIEF_NONE);
    pIcon->show();
    pButton->add(*pIcon);
    pButton->show();
    pButton->signal_clicked().connect(sigc::mem_fun(*this, &PointParam::on_button_click));

    Gtk::HBox * hbox = Gtk::manage( new Gtk::HBox() );
    static_cast<Gtk::HBox*>(hbox)->pack_start(*pButton, true, true);
    static_cast<Gtk::HBox*>(hbox)->pack_start(*pointwdg, true, true);
    static_cast<Gtk::HBox*>(hbox)->show_all_children();

    tooltips->set_tip(*pButton, _("Edit on-canvas"));

    return dynamic_cast<Gtk::Widget *> (hbox);
}

void
PointParam::param_setValue(Geom::Point newpoint)
{
    *dynamic_cast<Geom::Point *>( this ) = newpoint;
}

void
PointParam::param_set_and_write_new_value (Geom::Point newpoint)
{
    Inkscape::SVGOStringStream os;
    os << newpoint[0] << "," << newpoint[1];
    gchar * str = g_strdup(os.str().c_str());
    param_write_to_repr(str);
    g_free(str);
}

void
PointParam::param_editOncanvas(SPItem * item, SPDesktop * dt)
{
    // If not already in nodecontext, goto it!
    if (!tools_isactive(dt, TOOLS_NODES)) {
        tools_switch_current(TOOLS_NODES);
    }

    PointParamKnotHolder * kh =  pointparam_knot_holder_new( dt, SP_OBJECT(param_effect->getLPEObj()), param_key.c_str(), item);
    if (kh) {
        kh->add_knot(* dynamic_cast<Geom::Point *>( this ), NULL, knot_shape, knot_mode, knot_color, param_getTooltip()->c_str() );

        ShapeEditor * shape_editor = SP_NODE_CONTEXT( dt->event_context )->shape_editor;
        shape_editor->set_knotholder(kh);
    }
}



void
PointParam::param_transform_multiply(Geom::Matrix const& postmul, bool /*set*/)
{
    param_set_and_write_new_value( (*this) * postmul );
}


void
PointParam::set_oncanvas_looks(SPKnotShapeType shape, SPKnotModeType mode, guint32 color)
{
    knot_shape = shape;
    knot_mode  = mode;
    knot_color = color;
}


// CALLBACKS:

void
PointParam::on_button_click()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    SPItem * item = sp_desktop_selection(desktop)->singleItem();
    if (item != NULL) {
        param_editOncanvas(item, desktop);
    }
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
