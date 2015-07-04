/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/dialog/lpe-powerstroke-properties.h"
#include "live_effects/parameter/powerstrokepointarray.h"

#include "live_effects/effect.h"
#include "knotholder.h"
#include "sp-lpe-item.h"

#include <2geom/piecewise.h>
#include <2geom/sbasis-geometric.h>

#include "preferences.h" // for proportional stroke/path scaling behavior

#include <glibmm/i18n.h>

namespace Inkscape {

namespace LivePathEffect {

PowerStrokePointArrayParam::PowerStrokePointArrayParam( const Glib::ustring& label, const Glib::ustring& tip,
                        const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                        Effect* effect)
    : ArrayParam<Geom::Point>(label, tip, key, wr, effect, 0)
{
    knot_shape = SP_KNOT_SHAPE_DIAMOND;
    knot_mode  = SP_KNOT_MODE_XOR;
    knot_color = 0xff88ff00;
}

PowerStrokePointArrayParam::~PowerStrokePointArrayParam()
{
}

Gtk::Widget *
PowerStrokePointArrayParam::param_newWidget()
{
    return NULL;
}

void PowerStrokePointArrayParam::param_transform_multiply(Geom::Affine const &postmul, bool /*set*/)
{
    // Check if proportional stroke-width scaling is on
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool transform_stroke = prefs ? prefs->getBool("/options/transform/stroke", true) : true;
    if (transform_stroke) {
        std::vector<Geom::Point> result;
        result.reserve(_vector.size()); // reserve space for the points that will be added in the for loop
        for (std::vector<Geom::Point>::const_iterator point_it = _vector.begin(), e = _vector.end();
             point_it != e; ++point_it)
        {
            // scale each width knot with the average scaling in X and Y
            Geom::Coord const A = (*point_it)[Geom::Y] * ((postmul.expansionX() + postmul.expansionY()) / 2);
            result.push_back(Geom::Point((*point_it)[Geom::X], A));
        }
        param_set_and_write_new_value(result);
    }
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
                
                double t = nearest_time(position, pwd2_in);
                double offset = dot(position - pwd2_in.valueAt(t), normal.valueAt(t));
                _vector[i] = Geom::Point(t, offset);
            }
        }

        write_to_SVG();
    }
}

float PowerStrokePointArrayParam::median_width()
{
	size_t size = _vector.size();
	if (size > 0)
	{
		if (size % 2 == 0)
		{
			return (_vector[size / 2 - 1].y() + _vector[size / 2].y()) / 2;
		}
		else
		{
			return _vector[size / 2].y();
		}
	}
	return 1;
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
/*
class PowerStrokePointArrayParamKnotHolderEntity : public KnotHolderEntity {
public:
    PowerStrokePointArrayParamKnotHolderEntity(PowerStrokePointArrayParam *p, unsigned int index);
    virtual ~PowerStrokePointArrayParamKnotHolderEntity() {}

    virtual void knot_set(Geom::Point const &p, Geom::Point const &origin, guint state);
    virtual Geom::Point knot_get() const;
    virtual void knot_click(guint state);

    // Checks whether the index falls within the size of the parameter's vector
    bool valid_index(unsigned int index) const {
        return (_pparam->_vector.size() > index);
    };

private:
    PowerStrokePointArrayParam *_pparam;
    unsigned int _index;
};*/

PowerStrokePointArrayParamKnotHolderEntity::PowerStrokePointArrayParamKnotHolderEntity(PowerStrokePointArrayParam *p, unsigned int index) 
  : _pparam(p), 
    _index(index)
{ 
}

void
PowerStrokePointArrayParamKnotHolderEntity::knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint state)
{
    using namespace Geom;

    if (!valid_index(_index)) {
        return;
    }

    /// @todo how about item transforms???
    Piecewise<D2<SBasis> > const & pwd2 = _pparam->get_pwd2();
    Piecewise<D2<SBasis> > const & n = _pparam->get_pwd2_normal();

    Geom::Point const s = snap_knot_position(p, state);
    double t = nearest_time(s, pwd2);
    double offset = dot(s - pwd2.valueAt(t), n.valueAt(t));
    _pparam->_vector.at(_index) = Geom::Point(t, offset);
    sp_lpe_item_update_patheffect(SP_LPE_ITEM(item), false, false);
}

Geom::Point
PowerStrokePointArrayParamKnotHolderEntity::knot_get() const
{
    using namespace Geom;

    if (!valid_index(_index)) {
        return Geom::Point(infinity(), infinity());
    }

    Piecewise<D2<SBasis> > const & pwd2 = _pparam->get_pwd2();
    Piecewise<D2<SBasis> > const & n = _pparam->get_pwd2_normal();

    Point offset_point = _pparam->_vector.at(_index);
    if (offset_point[X] > pwd2.size() || offset_point[X] < 0) {
        g_warning("Broken powerstroke point at %f, I won't try to add that", offset_point[X]);
        return Geom::Point(infinity(), infinity());
    }
    Point canvas_point = pwd2.valueAt(offset_point[X]) + offset_point[Y] * n.valueAt(offset_point[X]);
    return canvas_point;
}

void PowerStrokePointArrayParamKnotHolderEntity::knot_set_offset(Geom::Point offset)
{
	_pparam->_vector.at(_index) = Geom::Point(offset.x(), offset.y() / 2);
	this->parent_holder->knot_ungrabbed_handler(this->knot, 0);
}

void
PowerStrokePointArrayParamKnotHolderEntity::knot_click(guint state)
{
    if (state & GDK_CONTROL_MASK) {
        if (state & GDK_MOD1_MASK) {
            // delete the clicked knot
            std::vector<Geom::Point> & vec = _pparam->_vector;
            vec.erase(vec.begin() + _index);
            _pparam->param_set_and_write_new_value(vec);

            // remove knot from knotholder
            parent_holder->entity.remove(this);
            // shift knots down one index
            for(std::list<KnotHolderEntity *>::iterator ent = parent_holder->entity.begin(); ent != parent_holder->entity.end(); ++ent) {
                PowerStrokePointArrayParamKnotHolderEntity *pspa_ent = dynamic_cast<PowerStrokePointArrayParamKnotHolderEntity *>(*ent);
                if ( pspa_ent && pspa_ent->_pparam == this->_pparam ) {  // check if the knotentity belongs to this powerstrokepointarray parameter
                    if (pspa_ent->_index > this->_index) {
                        --pspa_ent->_index;
                    }
                }
            };
            // delete self and return
            delete this;
            return;
        } else {
            // add a knot to XML
            std::vector<Geom::Point> & vec = _pparam->_vector;
            vec.insert(vec.begin() + _index, 1, vec.at(_index)); // this clicked knot is duplicated
            _pparam->param_set_and_write_new_value(vec);

            // shift knots up one index
            for(std::list<KnotHolderEntity *>::iterator ent = parent_holder->entity.begin(); ent != parent_holder->entity.end(); ++ent) {
                PowerStrokePointArrayParamKnotHolderEntity *pspa_ent = dynamic_cast<PowerStrokePointArrayParamKnotHolderEntity *>(*ent);
                if ( pspa_ent && pspa_ent->_pparam == this->_pparam ) {  // check if the knotentity belongs to this powerstrokepointarray parameter
                    if (pspa_ent->_index > this->_index) {
                        ++pspa_ent->_index;
                    }
                }
            };
            // add knot to knotholder
            PowerStrokePointArrayParamKnotHolderEntity *e = new PowerStrokePointArrayParamKnotHolderEntity(_pparam, _index+1);
            e->create( this->desktop, this->item, parent_holder, Inkscape::CTRL_TYPE_UNKNOWN,
                       _("<b>Stroke width control point</b>: drag to alter the stroke width. <b>Ctrl+click</b> adds a control point, <b>Ctrl+Alt+click</b> deletes it, <b>Shift+click</b> launches width dialog."),
                        _pparam->knot_shape, _pparam->knot_mode, _pparam->knot_color);
            parent_holder->add(e);
        }
    }
    else if ((state & GDK_MOD1_MASK) || (state & GDK_SHIFT_MASK))
    {
    	Geom::Point offset = Geom::Point(_pparam->_vector.at(_index).x(), _pparam->_vector.at(_index).y() * 2);
    	Inkscape::UI::Dialogs::PowerstrokePropertiesDialog::showDialog(this->desktop, offset, this);
    } 
}

void PowerStrokePointArrayParam::addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item)
{
    for (unsigned int i = 0; i < _vector.size(); ++i) {
        PowerStrokePointArrayParamKnotHolderEntity *e = new PowerStrokePointArrayParamKnotHolderEntity(this, i);
        e->create( desktop, item, knotholder, Inkscape::CTRL_TYPE_UNKNOWN,
                   _("<b>Stroke width control point</b>: drag to alter the stroke width. <b>Ctrl+click</b> adds a control point, <b>Ctrl+Alt+click</b> deletes it, <b>Shift+click</b> launches width dialog."),
                   knot_shape, knot_mode, knot_color);
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
