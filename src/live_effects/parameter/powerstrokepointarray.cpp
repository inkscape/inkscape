#define INKSCAPE_LIVEPATHEFFECT_POWERSTROKE_POINT_ARRAY_CPP

/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/powerstrokepointarray.h"

#include "live_effects/effect.h"
#include "svg/svg.h"
#include "svg/stringstream.h"
#include <gtkmm.h>
#include "ui/widget/point.h"
#include "widgets/icon.h"
#include "ui/widget/registered-widget.h"
#include "inkscape.h"
#include "verbs.h"
#include "knotholder.h"

// needed for on-canvas editting:
#include "desktop.h"

namespace Inkscape {

namespace LivePathEffect {

PowerStrokePointArrayParam::PowerStrokePointArrayParam( const Glib::ustring& label, const Glib::ustring& tip,
                        const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                        Effect* effect, const gchar *htip)
    : ArrayParam<Geom::Point>(label, tip, key, wr, effect, 0)
{
    knot_shape = SP_KNOT_SHAPE_DIAMOND;
    knot_mode  = SP_KNOT_MODE_XOR;
    knot_color = 0xffffff00;
    handle_tip = g_strdup(htip);
}

PowerStrokePointArrayParam::~PowerStrokePointArrayParam()
{
    if (handle_tip)
        g_free(handle_tip);
}

Gtk::Widget *
PowerStrokePointArrayParam::param_newWidget(Gtk::Tooltips * /*tooltips*/)
{
    return NULL;
/*
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
*/
}


void
PowerStrokePointArrayParam::param_transform_multiply(Geom::Matrix const& postmul, bool /*set*/)
{
//    param_set_and_write_new_value( (*this) * postmul );
}


void
PowerStrokePointArrayParam::set_oncanvas_looks(SPKnotShapeType shape, SPKnotModeType mode, guint32 color)
{
    knot_shape = shape;
    knot_mode  = mode;
    knot_color = color;
}

class PowerStrokePointArrayParamKnotHolderEntity : public LPEKnotHolderEntity {
public:
    PowerStrokePointArrayParamKnotHolderEntity(PowerStrokePointArrayParam *p, unsigned int index);
    virtual ~PowerStrokePointArrayParamKnotHolderEntity() {}

    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get();
    virtual void knot_click(guint state);

private:
    PowerStrokePointArrayParam *_pparam;
    unsigned int _index;
};

PowerStrokePointArrayParamKnotHolderEntity::PowerStrokePointArrayParamKnotHolderEntity(PowerStrokePointArrayParam *p, unsigned int index) 
  : _pparam(p), 
    _index(index)
{ 
}

void
PowerStrokePointArrayParamKnotHolderEntity::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint /*state*/)
{
//    Geom::Point const s = snap_knot_position(p);
//    pparam->param_setValue(s);
//    sp_lpe_item_update_patheffect(SP_LPE_ITEM(item), false, false);
}

Geom::Point
PowerStrokePointArrayParamKnotHolderEntity::knot_get()
{
    Geom::Point canvas_point;
    return canvas_point;
}

void
PowerStrokePointArrayParamKnotHolderEntity::knot_click(guint /*state*/)
{
    g_print ("This is the %d handle associated to parameter '%s'\n", _index, _pparam->param_key.c_str());
}

void
PowerStrokePointArrayParam::addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item)
{
    //PowerStrokePointArrayParamKnotHolderEntity *e = new PowerStrokePointArrayParamKnotHolderEntity(this);
    // TODO: can we ditch handleTip() etc. because we have access to handle_tip etc. itself???
    //e->create(desktop, item, knotholder, handleTip(), knot_shape, knot_mode, knot_color);
    //knotholder->add(e);

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
