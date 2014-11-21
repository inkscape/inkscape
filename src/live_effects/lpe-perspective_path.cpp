/**
 * @file
 * LPE perspective path effect implementation.
 */
/* Authors:
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Copyright (C) 2007-2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include <gtkmm.h>
#include <glibmm/i18n.h>

#include "persp3d.h"
//#include "transf_mat_3x4.h"
#include "document.h"
#include "document-private.h"
#include "live_effects/lpe-perspective_path.h"
#include "live_effects/lpeobject.h"
#include "sp-item-group.h"
#include "knot-holder-entity.h"
#include "knotholder.h"
#include "desktop.h"
#include <util/units.h>
#include "inkscape.h"

#include <2geom/path.h>

namespace Inkscape {
namespace LivePathEffect {

namespace PP {

class KnotHolderEntityOffset : public LPEKnotHolderEntity
{
public:
    KnotHolderEntityOffset(LPEPerspectivePath *effect) : LPEKnotHolderEntity(effect) {};
    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get() const;
};

} // namespace PP

static Glib::ustring perspectiveID = _("First perspective");


LPEPerspectivePath::LPEPerspectivePath(LivePathEffectObject *lpeobject) :
    Effect(lpeobject),
    // initialise your parameters here:
    scalex(_("Scale x"), _("Scale factor in x direction"), "scalex", &wr, this, 1.0),
    scaley(_("Scale y"), _("Scale factor in y direction"), "scaley", &wr, this, 1.0),
    offsetx(_("Offset x"), _("Offset in x direction"), "offsetx", &wr, this, 0.0),
    offsety(_("Offset y"), _("Offset in y direction"), "offsety", &wr, this, 0.0),
    uses_plane_xy(_("Uses XY plane?"), _("If true, put the path on the left side of an imaginary box, otherwise on the right side"), "uses_plane_xy", &wr, this, true)
{
    // register all your parameters here, so Inkscape knows which parameters this effect has:
    registerParameter( dynamic_cast<Parameter *>(&scalex) );
    registerParameter( dynamic_cast<Parameter *>(&scaley) );
    registerParameter( dynamic_cast<Parameter *>(&offsetx) );
    registerParameter( dynamic_cast<Parameter *>(&offsety) );
    registerParameter( dynamic_cast<Parameter *>(&uses_plane_xy) );

    concatenate_before_pwd2 = true; // don't split the path into its subpaths
    _provides_knotholder_entities = true;
}

LPEPerspectivePath::~LPEPerspectivePath()
{

}
void
LPEPerspectivePath::doOnApply(SPLPEItem const* lpeitem)
{
    Persp3D *persp = persp3d_document_first_persp(lpeitem->document);
    if(persp == 0 ){
        char *msg = _("You need a BOX 3D object");
        Gtk::MessageDialog dialog(msg, false, Gtk::MESSAGE_INFO,
                                  Gtk::BUTTONS_OK, true);
        dialog.run();
        SPLPEItem * item = const_cast<SPLPEItem*>(lpeitem);
        item->removeCurrentPathEffect(false);
    }
}
void
LPEPerspectivePath::doBeforeEffect (SPLPEItem const* lpeitem)
{
    original_bbox(lpeitem, true);
    SPLPEItem * item = const_cast<SPLPEItem*>(lpeitem);
    Persp3D *persp = persp3d_document_first_persp(lpeitem->document);
    if(persp == 0 ){
        char *msg = _("You need a BOX 3D object");
        Gtk::MessageDialog dialog(msg, false, Gtk::MESSAGE_INFO,
                                  Gtk::BUTTONS_OK, true);
        dialog.run();
        return;
    }
    Proj::TransfMat3x4 pmat = persp->perspective_impl->tmat;
    Geom::Affine doc2d = Geom::Scale(1, -1) * Geom::Translate(0, item->document->getHeight().value("px"));
    pmat = pmat * doc2d;
    pmat.copy_tmat(tmat);
    item->apply_to_clippath(item);
    item->apply_to_mask(item);
}

void LPEPerspectivePath::refresh(Gtk::Entry* perspective) {
    perspectiveID = perspective->get_text();
    Persp3D *first = 0;
    Persp3D *persp = 0;
    for ( SPObject *child = this->lpeobj->document->getDefs()->firstChild(); child && !persp; child = child->getNext() ) {
        if (SP_IS_PERSP3D(child) && first == 0) {
            first = SP_PERSP3D(child);
        }
        if (SP_IS_PERSP3D(child) && strcmp(child->getId(), const_cast<const gchar *>(perspectiveID.c_str())) == 0) {
            persp = SP_PERSP3D(child);
            break;
        }
    }
    if(first == 0 ){
        char *msg = _("You need a BOX 3D object");
        Gtk::MessageDialog dialog(msg, false, Gtk::MESSAGE_INFO,
                                  Gtk::BUTTONS_OK, true);
        dialog.run();
        return;
    }
    if(persp == 0){
        persp = first;
        char *msg = _("First perspective selected");
        Gtk::MessageDialog dialog(msg, false, Gtk::MESSAGE_INFO,
                                      Gtk::BUTTONS_OK, true);
        dialog.run();
        perspectiveID = _("First perspective");
    }else{
        char *msg = _("Perspective changed");
        Gtk::MessageDialog dialog(msg, false, Gtk::MESSAGE_INFO,
                                      Gtk::BUTTONS_OK, true);
        dialog.run();
    }
    Proj::TransfMat3x4 pmat = persp->perspective_impl->tmat;
    pmat = pmat * SP_ACTIVE_DESKTOP->doc2dt();
    pmat.copy_tmat(tmat);
};

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEPerspectivePath::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    using namespace Geom;

    Piecewise<D2<SBasis> > path_a_pw = pwd2_in;

    // FIXME: the minus sign is there because the SVG coordinate system goes down;
    //        remove this once we have unified coordinate systems
    path_a_pw = path_a_pw + Geom::Point(offsetx, -offsety);

    D2<Piecewise<SBasis> > B = make_cuts_independent(path_a_pw);
    Piecewise<SBasis> preimage[4];

    //Geom::Point orig = Geom::Point(bounds_X.min(), bounds_Y.middle());
    //orig = Geom::Point(orig[X], sp_document_height(this->lpeobj->document) - orig[Y]);

    //double offset = uses_plane_xy ? boundingbox_X.extent() : 0.0;

    orig = Point(uses_plane_xy ? boundingbox_X.max() : boundingbox_X.min(), boundingbox_Y.middle());

    /**
    g_print ("Orig: (%8.2f, %8.2f)\n", orig[X], orig[Y]);

    g_print ("B[1] - orig[1]: %8.2f\n", (B[1] - orig[1])[0].valueAt(0));
    g_print ("B[0] - orig[0]: %8.2f\n", (B[0] - orig[0])[0].valueAt(0));
    **/

    if (uses_plane_xy) {
        preimage[0] =  (-B[0] + orig[0]) * scalex / 200.0;
        preimage[1] =  ( B[1] - orig[1]) * scaley / 400.0;
        preimage[2] =  B[0] - B[0]; // hack!
    } else {
        preimage[0] =  B[0] - B[0]; // hack!
        preimage[1] =  (B[1] - orig[1]) * scaley / 400.0;
        preimage[2] =  (B[0] - orig[0]) * scalex / 200.0;
    }

    /* set perspective origin to first point of path */
    tmat[0][3] = orig[0];
    tmat[1][3] = orig[1];

    /**
    g_print ("preimage[1]: %8.2f\n", preimage[1][0].valueAt(0));
    g_print ("preimage[2]: %8.2f\n", preimage[2][0].valueAt(0));
    **/

    Piecewise<SBasis> res[3];
    for (int j = 0; j < 3; ++j) {
        res[j] =
              preimage[0] * tmat[j][0]
            + preimage[1] * tmat[j][1]
            + preimage[2] * tmat[j][2]
            +               tmat[j][3];
    }
    D2<Piecewise<SBasis> > result(divide(res[0],res[2], 3),
                                  divide(res[1],res[2], 3));

    Piecewise<D2<SBasis> > output = sectionize(result);

    return output;
}

Gtk::Widget *
LPEPerspectivePath::newWidget()
{
    // use manage here, because after deletion of Effect object, others might still be pointing to this widget.
    Gtk::VBox * vbox = Gtk::manage( new Gtk::VBox(Effect::newWidget()) );

    vbox->set_border_width(5);
    std::vector<Parameter *>::iterator it = param_vector.begin();
    while (it != param_vector.end()) {
        if ((*it)->widget_is_visible) {
            Parameter * param = *it;
            Gtk::Widget * widg = dynamic_cast<Gtk::Widget *>(param->param_newWidget());
            Glib::ustring * tip = param->param_getTooltip();
            if (widg) {
                vbox->pack_start(*widg, true, true, 2);
                if (tip) {
                    widg->set_tooltip_text(*tip);
                } else {
                    widg->set_tooltip_text("");
                    widg->set_has_tooltip(false);
                }
            }
        }

        ++it;
    }
    Gtk::HBox * perspectiveId = Gtk::manage(new Gtk::HBox(true,0));
    Gtk::Label* labelPerspective = Gtk::manage(new Gtk::Label("Perspective ID:", 0., 0.));
    Gtk::Entry* perspective = Gtk::manage(new Gtk::Entry());
    perspective->set_text(perspectiveID);
    perspective->set_tooltip_text("Set the perspective ID to apply");
    perspectiveId->pack_start(*labelPerspective, true, true, 2);
    perspectiveId->pack_start(*perspective, true, true, 2);
    vbox->pack_start(*perspectiveId, true, true, 2);
    Gtk::Button* apply3D = Gtk::manage(new Gtk::Button(Glib::ustring(_("Refresh perspective"))));
    apply3D->set_alignment(0.0, 0.5);
    apply3D->signal_clicked().connect(sigc::bind<Gtk::Entry*>(sigc::mem_fun(*this,&LPEPerspectivePath::refresh),perspective));
    Gtk::Widget* apply3DWidget = dynamic_cast<Gtk::Widget *>(apply3D);
    apply3DWidget->set_tooltip_text("Refresh perspective");
    vbox->pack_start(*apply3DWidget, true, true,2);
    return dynamic_cast<Gtk::Widget *>(vbox);
}

void LPEPerspectivePath::addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item) {
    KnotHolderEntity *e = new PP::KnotHolderEntityOffset(this);
    e->create( desktop, item, knotholder, Inkscape::CTRL_TYPE_UNKNOWN,
               _("Adjust the origin") );
    knotholder->add(e);
};

namespace PP {

void
KnotHolderEntityOffset::knot_set(Geom::Point const &p, Geom::Point const &origin, guint state)
{
    using namespace Geom;
 
    LPEPerspectivePath* lpe = dynamic_cast<LPEPerspectivePath *>(_effect);

    Geom::Point const s = snap_knot_position(p, state);

    lpe->offsetx.param_set_value((s - origin)[Geom::X]);
    lpe->offsety.param_set_value(-(s - origin)[Geom::Y]); // additional minus sign is due to coordinate system flipping

    // FIXME: this should not directly ask for updating the item. It should write to SVG, which triggers updating.
    sp_lpe_item_update_patheffect (SP_LPE_ITEM(item), false, true);
}

Geom::Point
KnotHolderEntityOffset::knot_get() const
{
    LPEPerspectivePath const *lpe = dynamic_cast<LPEPerspectivePath const*>(_effect);
    return lpe->orig + Geom::Point(lpe->offsetx, -lpe->offsety);
}

} // namespace PP

} //namespace LivePathEffect
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
