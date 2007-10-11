#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_POINT_CPP

/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/point.h"
#include "live_effects/effect.h"
#include "svg/svg.h"
#include "svg/stringstream.h"
#include <gtkmm.h>
#include "ui/widget/point.h"
#include "widgets/icon.h"

#include "knot.h"
#include "inkscape.h"
#include "verbs.h"

#define noLPEPOINTPARAM_DEBUG

#define PRM_KNOT_COLOR_NORMAL 0xffffff00
#define PRM_KNOT_COLOR_SELECTED 0x0000ff00

namespace Inkscape {

namespace LivePathEffect {

PointParam::PointParam( const Glib::ustring& label, const Glib::ustring& tip,
                        const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                        Effect* effect, Geom::Point default_value )
    : Geom::Point(default_value), Parameter(label, tip, key, wr, effect), defvalue(default_value)
{
    _widget = NULL;
    pointwdg = NULL;
    knot = NULL;
    _tooltips = NULL;
}

PointParam::~PointParam()
{
    if (pointwdg)
        delete pointwdg;
    if (_tooltips)
        delete _tooltips;

    if (knot)
        g_object_unref (G_OBJECT (knot));
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
PointParam::param_getWidget()
{
    if (!_widget) {
        pointwdg = new Inkscape::UI::Widget::RegisteredPoint();
        pointwdg->init(param_label, param_tooltip, param_key, *param_wr, param_effect->getRepr(), param_effect->getSPDoc());
        pointwdg->setValue( (*this)[0], (*this)[1] );
        pointwdg->set_undo_parameters(SP_VERB_DIALOG_LIVE_PATH_EFFECT, _("Change point parameter"));

        Gtk::Widget*  pIcon = Gtk::manage( sp_icon_get_icon( "draw_node", Inkscape::ICON_SIZE_BUTTON) );
        Gtk::Button * pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(sigc::mem_fun(*this, &PointParam::on_button_click));
#ifndef LPEPOINTPARAM_DEBUG
        pButton->set_sensitive(false);
#endif

        _widget = Gtk::manage( new Gtk::HBox() );
        static_cast<Gtk::HBox*>(_widget)->pack_start(*pButton, true, true);
        static_cast<Gtk::HBox*>(_widget)->pack_start(*(pointwdg->getPoint()), true, true);
        static_cast<Gtk::HBox*>(_widget)->show_all_children();

        _tooltips = new Gtk::Tooltips();
        _tooltips->set_tip(*pButton, _("Edit on-canvas"));
    }
    return dynamic_cast<Gtk::Widget *> (_widget);
}

void
PointParam::param_setValue(Geom::Point newpoint)
{
    *dynamic_cast<Geom::Point *>( this ) = newpoint;
    if (pointwdg)
        pointwdg->setValue(newpoint[0], newpoint[1]);
}


// CALLBACKS:

void
PointParam::on_button_click()
{
    g_message("add knot to canvas on correct location :S");

    if (!knot) {
        // create the knot
        knot = sp_knot_new (SP_ACTIVE_DESKTOP, NULL);
        knot->setMode(SP_KNOT_MODE_XOR);
        knot->setFill(PRM_KNOT_COLOR_NORMAL, PRM_KNOT_COLOR_NORMAL, PRM_KNOT_COLOR_NORMAL);
        knot->setStroke(0x000000ff, 0x000000ff, 0x000000ff);
        sp_knot_update_ctrl(knot);

        // move knot to the given point
        sp_knot_set_position (knot, &NR::Point((*this)[0], (*this)[1]), SP_KNOT_STATE_NORMAL);
        sp_knot_show (knot);
/*
        // connect knot's signals
        if ( (draggable)  // it can be NULL if a node in unsnapped (eg. focus point unsnapped from center)
                           // luckily, midstops never snap to other nodes so are never unsnapped...
             && ( (draggable->point_type == POINT_LG_MID)
                  || (draggable->point_type == POINT_RG_MID1)
                  || (draggable->point_type == POINT_RG_MID2) ) )
        {
            this->handler_id = g_signal_connect (G_OBJECT (this->knot), "moved", G_CALLBACK (gr_knot_moved_midpoint_handler), this);
        } else {
            this->handler_id = g_signal_connect (G_OBJECT (this->knot), "moved", G_CALLBACK (gr_knot_moved_handler), this);
        }
        g_signal_connect (G_OBJECT (this->knot), "clicked", G_CALLBACK (gr_knot_clicked_handler), this);
        g_signal_connect (G_OBJECT (this->knot), "doubleclicked", G_CALLBACK (gr_knot_doubleclicked_handler), this);
        g_signal_connect (G_OBJECT (this->knot), "grabbed", G_CALLBACK (gr_knot_grabbed_handler), this);
        g_signal_connect (G_OBJECT (this->knot), "ungrabbed", G_CALLBACK (gr_knot_ungrabbed_handler), this);
*/
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
