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
#include "knotholder.h"
#include "sp-lpe-item.h"

#include <2geom/piecewise.h>
#include <2geom/sbasis-geometric.h>

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
    knot_color = 0xff00ff00;
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
    Geom::Affine transf = desktop->doc2dt();
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


void PowerStrokePointArrayParam::param_transform_multiply(Geom::Affine const& /*postmul*/, bool /*set*/)
{
//    param_set_and_write_new_value( (*this) * postmul );
}


/** call this method to recalculate the controlpoints such that they stay at the same location relative to the new path. Useful after adding/deleting nodes to the path.*/
void
PowerStrokePointArrayParam::recalculate_controlpoints_for_new_pwd2(Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    if (!last_pwd2.empty()) {
        if (last_pwd2.size() > pwd2_in.size()) {
            // Path has become shorter: rescale offsets
            double factor = (double)pwd2_in.size() / (double)last_pwd2.size();
            for (unsigned int i = 0; i < _vector.size(); ++i) {
                _vector[i][Geom::X] *= factor;
            }
        } else if (last_pwd2.size() < pwd2_in.size()) {
            // Path has become longer: probably node added, maintain position of knots
            Geom::Piecewise<Geom::D2<Geom::SBasis> > normal = rot90(unitVector(derivative(pwd2_in)));
            for (unsigned int i = 0; i < _vector.size(); ++i) {
                Geom::Point pt = _vector[i];
                Geom::Point position = last_pwd2.valueAt(pt[Geom::X]) + pt[Geom::Y] * last_pwd2_normal.valueAt(pt[Geom::X]);
                
                double t = nearest_point(position, pwd2_in);
                double offset = dot(position - pwd2_in.valueAt(t), normal.valueAt(t));
                _vector[i] = Geom::Point(t, offset);
            }
        }

        write_to_SVG();
    }
}

void
PowerStrokePointArrayParam::set_pwd2(Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in, Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_normal_in)
{
    last_pwd2 = pwd2_in;
    last_pwd2_normal = pwd2_normal_in;
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

    /** Checks whether the index falls within the size of the parameter's vector */
    bool valid_index(unsigned int index) {
        return (_pparam->_vector.size() > index);
    };

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
    using namespace Geom;

    if (!valid_index(_index)) {
        return;
    }

    /// @todo how about item transforms???
    Piecewise<D2<SBasis> > const & pwd2 = _pparam->get_pwd2();
    Piecewise<D2<SBasis> > const & n = _pparam->get_pwd2_normal();

    Geom::Point const s = snap_knot_position(p);
    double t = nearest_point(s, pwd2);
    double offset = dot(s - pwd2.valueAt(t), n.valueAt(t));
    _pparam->_vector.at(_index) = Geom::Point(t, offset);
    sp_lpe_item_update_patheffect(SP_LPE_ITEM(item), false, false);
}

Geom::Point
PowerStrokePointArrayParamKnotHolderEntity::knot_get()
{
    using namespace Geom;

    if (!valid_index(_index)) {
        return Geom::Point(infinity(), infinity());
    }

    Piecewise<D2<SBasis> > const & pwd2 = _pparam->get_pwd2();
    Piecewise<D2<SBasis> > const & n = _pparam->get_pwd2_normal();

    Point offset_point = _pparam->_vector.at(_index);

    Point canvas_point = pwd2.valueAt(offset_point[X]) + offset_point[Y] * n.valueAt(offset_point[X]);
    return canvas_point;
}

void
PowerStrokePointArrayParamKnotHolderEntity::knot_click(guint state)
{
//g_print ("This is the %d handle associated to parameter '%s'\n", _index, _pparam->param_key.c_str());

    if (state & GDK_CONTROL_MASK) {
        if (state & GDK_MOD1_MASK) {
            // delete the clicked knot
            std::vector<Geom::Point> & vec = _pparam->_vector;
            vec.erase(vec.begin() + _index);
            _pparam->param_set_and_write_new_value(vec);
        } else {
            // add a knot
            std::vector<Geom::Point> & vec = _pparam->_vector;
            vec.insert(vec.begin() + _index, 1, vec.at(_index));
            _pparam->param_set_and_write_new_value(vec);
        }
    } 

}

void
PowerStrokePointArrayParam::addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item)
{
    for (unsigned int i = 0; i < _vector.size(); ++i) {
        PowerStrokePointArrayParamKnotHolderEntity *e = new PowerStrokePointArrayParamKnotHolderEntity(this, i);
        e->create(desktop, item, knotholder, handle_tip, knot_shape, knot_mode, knot_color);
        knotholder->add(e);
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
