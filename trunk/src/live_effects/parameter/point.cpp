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
#include "ui/widget/registered-widget.h"
#include "inkscape.h"
#include "verbs.h"

// needed for on-canvas editting:
#include "desktop.h"

namespace Inkscape {

namespace LivePathEffect {

PointParam::PointParam( const Glib::ustring& label, const Glib::ustring& tip,
                        const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                        Effect* effect, const gchar *htip, Geom::Point default_value)
    : Geom::Point(default_value), Parameter(label, tip, key, wr, effect), defvalue(default_value)
{
    knot_shape = SP_KNOT_SHAPE_DIAMOND;
    knot_mode  = SP_KNOT_MODE_XOR;
    knot_color = 0xffffff00;
    handle_tip = g_strdup(htip);
}

PointParam::~PointParam()
{
    if (handle_tip)
        g_free(handle_tip);
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
PointParam::param_getSVGValue() const
{
    Inkscape::SVGOStringStream os;
    os << *dynamic_cast<Geom::Point const *>( this );
    gchar * str = g_strdup(os.str().c_str());
    return str;
}

Gtk::Widget *
PointParam::param_newWidget(Gtk::Tooltips * /*tooltips*/)
{
    Inkscape::UI::Widget::RegisteredTransformedPoint * pointwdg = Gtk::manage(
        new Inkscape::UI::Widget::RegisteredTransformedPoint( param_label,
                                                              param_tooltip,
                                                              param_key,
                                                              *param_wr,
                                                              param_effect->getRepr(),
                                                              param_effect->getSPDoc() ) );
    // TODO: fix to get correct desktop (don't use SP_ACTIVE_DESKTOP)
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    Geom::Matrix transf = desktop->doc2dt();
    pointwdg->setTransform(transf);
    pointwdg->setValue( *this );
    pointwdg->clearProgrammatically();
    pointwdg->set_undo_parameters(SP_VERB_DIALOG_LIVE_PATH_EFFECT, _("Change point parameter"));

    Gtk::HBox * hbox = Gtk::manage( new Gtk::HBox() );
    static_cast<Gtk::HBox*>(hbox)->pack_start(*pointwdg, true, true);
    static_cast<Gtk::HBox*>(hbox)->show_all_children();

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
    os << newpoint;
    gchar * str = g_strdup(os.str().c_str());
    param_write_to_repr(str);
    g_free(str);
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

class PointParamKnotHolderEntity : public LPEKnotHolderEntity {
public:
    PointParamKnotHolderEntity(PointParam *p) { this->pparam = p; }
    virtual ~PointParamKnotHolderEntity() {}

    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get();
    virtual void knot_click(guint state);

private:
    PointParam *pparam;
};

void
PointParamKnotHolderEntity::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint /*state*/)
{
    Geom::Point const s = snap_knot_position(p);
    pparam->param_setValue(s);
    sp_lpe_item_update_patheffect(SP_LPE_ITEM(item), false, false);
}

Geom::Point
PointParamKnotHolderEntity::knot_get()
{
    return *pparam;
}

void
PointParamKnotHolderEntity::knot_click(guint /*state*/)
{
    g_print ("This is the handle associated to parameter '%s'\n", pparam->param_key.c_str());
}

void
PointParam::addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item)
{
    PointParamKnotHolderEntity *e = new PointParamKnotHolderEntity(this);
    // TODO: can we ditch handleTip() etc. because we have access to handle_tip etc. itself???
    e->create(desktop, item, knotholder, handleTip(), knot_shape, knot_mode, knot_color);
    knotholder->add(e);

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
