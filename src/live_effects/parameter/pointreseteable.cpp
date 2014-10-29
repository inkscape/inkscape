/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/widget/registered-widget.h"
#include "live_effects/parameter/pointreseteable.h"
#include "live_effects/effect.h"
#include "svg/svg.h"
#include "svg/stringstream.h"
#include "ui/widget/point.h"
#include "widgets/icon.h"
#include "inkscape.h"
#include "verbs.h"
#include "knotholder.h"
#include <glibmm/i18n.h>
#include "ui/tools-switch.h"
#include "ui/tools/node-tool.h"

// needed for on-canvas editting:
#include "desktop.h"

namespace Inkscape {

namespace LivePathEffect {

PointReseteableParam::PointReseteableParam( const Glib::ustring& label, const Glib::ustring& tip,
                        const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                        Effect* effect, const gchar *htip, Geom::Point default_value)
    : Geom::Point(default_value), Parameter(label, tip, key, wr, effect), defvalue(default_value)
{
    knot_shape = SP_KNOT_SHAPE_DIAMOND;
    knot_mode  = SP_KNOT_MODE_XOR;
    knot_color = 0xffffff00;
    handle_tip = g_strdup(htip);
}

PointReseteableParam::~PointReseteableParam()
{
    if (handle_tip)
        g_free(handle_tip);
}

void
PointReseteableParam::param_set_default()
{
    param_setValue(defvalue);
}

void
PointReseteableParam::param_set_and_write_default()
{
    param_set_and_write_new_value(defvalue);
}

void
PointReseteableParam::param_update_default(Geom::Point newpoint)
{
    this->defvalue = newpoint;
}

bool
PointReseteableParam::param_readSVGValue(const gchar * strvalue)
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
PointReseteableParam::param_getSVGValue() const
{
    Inkscape::SVGOStringStream os;
    os << *dynamic_cast<Geom::Point const *>( this );
    gchar * str = g_strdup(os.str().c_str());
    return str;
}

Gtk::Widget *
PointReseteableParam::param_newWidget()
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
PointReseteableParam::param_setValue(Geom::Point newpoint)
{
    *dynamic_cast<Geom::Point *>( this ) = newpoint;
    if(SP_ACTIVE_DESKTOP){
        SPDesktop* desktop = SP_ACTIVE_DESKTOP;
        if (tools_isactive( desktop, TOOLS_NODES)) {
            Inkscape::UI::Tools::NodeTool *nt = static_cast<Inkscape::UI::Tools::NodeTool*>( desktop->event_context);
            nt->update_helperpath();
        }
    }
}

void
PointReseteableParam::param_set_and_write_new_value (Geom::Point newpoint)
{
    Inkscape::SVGOStringStream os;
    os << newpoint;
    gchar * str = g_strdup(os.str().c_str());
    param_write_to_repr(str);
    g_free(str);
}

void
PointReseteableParam::param_transform_multiply(Geom::Affine const& postmul, bool /*set*/)
{
    param_set_and_write_new_value( (*this) * postmul );
}


void
PointReseteableParam::set_oncanvas_looks(SPKnotShapeType shape, SPKnotModeType mode, guint32 color)
{
    knot_shape = shape;
    knot_mode  = mode;
    knot_color = color;
}

class PointReseteableParamKnotHolderEntity : public KnotHolderEntity {
public:
    PointReseteableParamKnotHolderEntity(PointReseteableParam *p) { this->pparam = p; }
    virtual ~PointReseteableParamKnotHolderEntity() {}

    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get() const;
    virtual void knot_click(guint state);

private:
    PointReseteableParam *pparam;
};

void
PointReseteableParamKnotHolderEntity::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    Geom::Point const s = snap_knot_position(p, state);
    pparam->param_setValue(s);
    sp_lpe_item_update_patheffect(SP_LPE_ITEM(item), false, false);
}

Geom::Point
PointReseteableParamKnotHolderEntity::knot_get() const
{
    return *pparam;
}

void
PointReseteableParamKnotHolderEntity::knot_click(guint state)
{
    if (state & GDK_CONTROL_MASK) {
            if (state & GDK_MOD1_MASK) {
                this->pparam->param_set_default();
                sp_lpe_item_update_patheffect(SP_LPE_ITEM(item), false, false);
            }
    }
}

void
PointReseteableParam::addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item)
{
    PointReseteableParamKnotHolderEntity *e = new PointReseteableParamKnotHolderEntity(this);
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
