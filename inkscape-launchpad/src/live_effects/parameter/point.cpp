/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/widget/registered-widget.h"
#include "live_effects/parameter/point.h"
#include "live_effects/effect.h"
#include "svg/svg.h"
#include "svg/stringstream.h"
#include "ui/widget/point.h"
#include "widgets/icon.h"
#include "inkscape.h"
#include "verbs.h"
#include "knotholder.h"
#include <glibmm/i18n.h>

// needed for on-canvas editting:
#include "desktop.h"

namespace Inkscape {

namespace LivePathEffect {

PointParam::PointParam( const Glib::ustring& label, const Glib::ustring& tip,
                        const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                        Effect* effect, const gchar *htip, Geom::Point default_value,
                        bool live_update )
    :   Parameter(label, tip, key, wr, effect), 
        defvalue(default_value),
        liveupdate(live_update),
        knoth(NULL)
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
    param_setValue(defvalue,true);
}

void
PointParam::param_set_liveupdate( bool live_update)
{
    liveupdate = live_update;
}

Geom::Point 
PointParam::param_get_default() const{
    return defvalue;
}

void
PointParam::param_update_default(Geom::Point newpoint)
{
    defvalue = newpoint;
}

void
PointParam::param_setValue(Geom::Point newpoint, bool write)
{
    *dynamic_cast<Geom::Point *>( this ) = newpoint;
    if(write){
        Inkscape::SVGOStringStream os;
        os << newpoint;
        gchar * str = g_strdup(os.str().c_str());
        param_write_to_repr(str);
        g_free(str);
    }
    if(knoth && liveupdate){
        knoth->update_knots();
    }
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

void
PointParam::param_transform_multiply(Geom::Affine const& postmul, bool /*set*/)
{
    param_setValue( (*this) * postmul, true);
}

Gtk::Widget *
PointParam::param_newWidget()
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
    Geom::Affine transf = desktop->doc2dt();
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
PointParam::set_oncanvas_looks(SPKnotShapeType shape, SPKnotModeType mode, guint32 color)
{
    knot_shape = shape;
    knot_mode  = mode;
    knot_color = color;
}

class PointParamKnotHolderEntity : public KnotHolderEntity {
public:
    PointParamKnotHolderEntity(PointParam *p) { this->pparam = p; }
    virtual ~PointParamKnotHolderEntity() { this->pparam->knoth = NULL;}

    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get() const;
    virtual void knot_click(guint state);

private:
    PointParam *pparam;
};

void
PointParamKnotHolderEntity::knot_set(Geom::Point const &p, Geom::Point const &origin, guint state)
{
    Geom::Point s = snap_knot_position(p, state);
    if (state & GDK_CONTROL_MASK) {
        Geom::Point A(origin[Geom::X],p[Geom::Y]);
        Geom::Point B(p[Geom::X],origin[Geom::Y]);
        double distanceA = Geom::distance(A,p);
        double distanceB = Geom::distance(B,p);
        if(distanceA > distanceB){
            s = B;
        } else {
            s = A;
        }
    }
    pparam->param_setValue(s, this->pparam->liveupdate);
    SPLPEItem * splpeitem = dynamic_cast<SPLPEItem *>(item);
    if(splpeitem && this->pparam->liveupdate){
        sp_lpe_item_update_patheffect(splpeitem, false, false);
    }
}

Geom::Point
PointParamKnotHolderEntity::knot_get() const
{
    return *pparam;
}

void
PointParamKnotHolderEntity::knot_click(guint state)
{
    if (state & GDK_CONTROL_MASK) {
            if (state & GDK_MOD1_MASK) {
                this->pparam->param_set_default();
                SPLPEItem * splpeitem = dynamic_cast<SPLPEItem *>(item);
                if(splpeitem){
                    sp_lpe_item_update_patheffect(splpeitem, false, false);
                }
            }
    }
}

void
PointParam::addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item)
{
    knoth = knotholder;
    PointParamKnotHolderEntity *e = new PointParamKnotHolderEntity(this);
    // TODO: can we ditch handleTip() etc. because we have access to handle_tip etc. itself???
    e->create(desktop, item, knotholder, Inkscape::CTRL_TYPE_UNKNOWN, handleTip(), knot_shape, knot_mode, knot_color);
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
