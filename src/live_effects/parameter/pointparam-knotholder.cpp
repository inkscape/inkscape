#define INKSCAPE_LPE_POINTPARAM_KNOTHOLDER_C

/*
 * Container for PointParamKnotHolder visual handles
 *
 * Authors:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2008 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/pointparam-knotholder.h"
#include "live_effects/lpeobject.h"
#include "document.h"
#include "sp-shape.h"
#include "knot.h"
#include "knotholder.h"

#include <glibmm/i18n.h>
#include <2geom/point.h>
#include <2geom/matrix.h>
#include "svg/stringstream.h"
#include "xml/repr.h"

class SPDesktop;

namespace Inkscape {

static void pointparam_knot_clicked_handler (SPKnot *knot, guint state, PointParamKnotHolder *kh);
static void pointparam_knot_moved_handler(SPKnot *knot, Geom::Point const *p, guint state, PointParamKnotHolder *kh);
static void pointparam_knot_ungrabbed_handler (SPKnot *knot, unsigned int state, PointParamKnotHolder *kh);

PointParamKnotHolder::PointParamKnotHolder(SPDesktop *desktop, SPObject *lpeobject, const gchar * key, SPItem *item)
{
    if (!desktop || !item || !SP_IS_ITEM(item)) {
        g_print ("Error! Throw an exception, please!\n");
    }

    this->desktop = desktop;
    this->item = item;
    this->lpeobject = LIVEPATHEFFECT(lpeobject);
    g_object_ref(G_OBJECT(item));
    g_object_ref(G_OBJECT(lpeobject));

    this->released = NULL;

    this->repr = lpeobject->repr;
    this->repr_key = key;

    this->local_change = FALSE;
}

PointParamKnotHolder::~PointParamKnotHolder()
{
    g_object_unref(G_OBJECT(this->item));
    g_object_unref(G_OBJECT(this->lpeobject));
}

class KnotHolderEntityPointParam : public LPEKnotHolderEntity {
public:
    virtual Geom::Point knot_get();
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
};

Geom::Point
KnotHolderEntityPointParam::knot_get() {
    return Geom::Point(0,0);
}

void
KnotHolderEntityPointParam::knot_set(Geom::Point const &/*p*/, Geom::Point const &/*origin*/, guint /*state*/) {
}

void
PointParamKnotHolder::add_knot (
    Geom::Point         & p,
// TODO: check if knot_click being ignored is bad:
    PointParamKnotHolderClickedFunc /*knot_click*/,
    SPKnotShapeType     shape,
    SPKnotModeType      mode,
    guint32             color,
    const gchar *tip )
{
    /* create new SPKnotHolderEntry */
    KnotHolderEntity *e = new KnotHolderEntityPointParam();
    e->create(this->desktop, this->item, this, tip, shape, mode, color);

    entity.push_back(e);

    // Move to current point.
    Geom::Point dp = p * sp_item_i2d_affine(item);
    sp_knot_set_position(e->knot, dp, SP_KNOT_STATE_NORMAL);

    e->handler_id = g_signal_connect(e->knot, "moved", G_CALLBACK(pointparam_knot_moved_handler), this);
    e->_click_handler_id = g_signal_connect(e->knot, "clicked", G_CALLBACK(pointparam_knot_clicked_handler), this);
    e->_ungrab_handler_id = g_signal_connect(e->knot, "ungrabbed", G_CALLBACK(pointparam_knot_ungrabbed_handler), this);

    sp_knot_show(e->knot);
}

static void pointparam_knot_clicked_handler(SPKnot */*knot*/, guint /*state*/, PointParamKnotHolder */*kh*/)
{

}

/**
 * \param p In desktop coordinates.
 *  This function does not write to XML, but tries to write directly to the PointParam to quickly live update the effect
 */
static void pointparam_knot_moved_handler(SPKnot */*knot*/, Geom::Point const *p, guint /*state*/, PointParamKnotHolder *kh)
{
    Geom::Matrix const i2d(sp_item_i2d_affine(kh->getItem()));
    Geom::Point pos = (*p) * i2d.inverse();

    Inkscape::SVGOStringStream os;
    os << pos;

    kh->lpeobject->lpe->setParameter(kh->repr_key, os.str().c_str());
}

static void pointparam_knot_ungrabbed_handler(SPKnot *knot, unsigned int /*state*/, PointParamKnotHolder *kh)
{
    Geom::Matrix const i2d(sp_item_i2d_affine(kh->getItem()));
    Geom::Point pos = sp_knot_position(knot) * i2d.inverse();

    Inkscape::SVGOStringStream os;
    os << pos;

    kh->repr->setAttribute(kh->repr_key , os.str().c_str());

    sp_document_done(SP_OBJECT_DOCUMENT (kh->lpeobject), SP_VERB_CONTEXT_LPE, _("Change LPE point parameter"));
}

} // namespace Inkscape

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
