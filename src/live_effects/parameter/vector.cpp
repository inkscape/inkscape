#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_VECTOR_CPP

/*
 * Copyright (C) Johan Engelen 2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/vector.h"
#include "sp-lpe-item.h"
#include "knotholder.h"
#include "svg/svg.h"
#include "svg/stringstream.h"
#include <gtkmm.h>

#include "ui/widget/registered-widget.h"
#include "live_effects/effect.h"
#include "desktop.h"

namespace Inkscape {

namespace LivePathEffect {

VectorParam::VectorParam( const Glib::ustring& label, const Glib::ustring& tip,
                        const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                        Effect* effect, Geom::Point default_vector)
    : Parameter(label, tip, key, wr, effect),
      defvalue(default_vector),
      origin(0.,0.),
      vector(default_vector)
{
    vec_knot_shape = SP_KNOT_SHAPE_DIAMOND;
    vec_knot_mode  = SP_KNOT_MODE_XOR;
    vec_knot_color = 0xffffb500;
    ori_knot_shape = SP_KNOT_SHAPE_CIRCLE;
    ori_knot_mode  = SP_KNOT_MODE_XOR;
    ori_knot_color = 0xffffb500;
}

VectorParam::~VectorParam()
{

}

void
VectorParam::param_set_default()
{
    setOrigin(Geom::Point(0.,0.));
    setVector(defvalue);
}

bool
VectorParam::param_readSVGValue(const gchar * strvalue)
{
    gchar ** strarray = g_strsplit(strvalue, ",", 4);
    double val[4];
    unsigned int i = 0;
    while (strarray[i] && i < 4) {
        if (sp_svg_number_read_d(strarray[i], &val[i]) != 0) {
            i++;
        } else {
            break;
        }
    }
    g_strfreev (strarray);
    if (i == 4) {
        setOrigin( Geom::Point(val[0], val[1]) );
        setVector( Geom::Point(val[2], val[3]) );
        return true;
    }
    return false;
}

gchar *
VectorParam::param_getSVGValue() const
{
    Inkscape::SVGOStringStream os;
    os << origin << " , " << vector;
    gchar * str = g_strdup(os.str().c_str());
    return str;
}

Gtk::Widget *
VectorParam::param_newWidget(Gtk::Tooltips * /*tooltips*/)
{
    Inkscape::UI::Widget::RegisteredVector * pointwdg = Gtk::manage(
        new Inkscape::UI::Widget::RegisteredVector( param_label,
                                                    param_tooltip,
                                                    param_key,
                                                    *param_wr,
                                                    param_effect->getRepr(),
                                                    param_effect->getSPDoc() ) );
    pointwdg->setPolarCoords();
    pointwdg->setValue( vector, origin );
    pointwdg->clearProgrammatically();
    pointwdg->set_undo_parameters(SP_VERB_DIALOG_LIVE_PATH_EFFECT, _("Change vector parameter"));

    Gtk::HBox * hbox = Gtk::manage( new Gtk::HBox() );
    static_cast<Gtk::HBox*>(hbox)->pack_start(*pointwdg, true, true);
    static_cast<Gtk::HBox*>(hbox)->show_all_children();

    return dynamic_cast<Gtk::Widget *> (hbox);
}

void
VectorParam::set_and_write_new_values(Geom::Point const &new_origin, Geom::Point const &new_vector)
{
    setValues(new_origin, new_vector);
    gchar * str = param_getSVGValue();
    param_write_to_repr(str);
    g_free(str);
}

void
VectorParam::param_transform_multiply(Geom::Matrix const& postmul, bool /*set*/)
{
        set_and_write_new_values( origin * postmul, vector * postmul.without_translation() );
}


void
VectorParam::set_vector_oncanvas_looks(SPKnotShapeType shape, SPKnotModeType mode, guint32 color)
{
    vec_knot_shape = shape;
    vec_knot_mode  = mode;
    vec_knot_color = color;
}

void
VectorParam::set_origin_oncanvas_looks(SPKnotShapeType shape, SPKnotModeType mode, guint32 color)
{
    ori_knot_shape = shape;
    ori_knot_mode  = mode;
    ori_knot_color = color;
}

class VectorParamKnotHolderEntity_Origin : public LPEKnotHolderEntity {
public:
    VectorParamKnotHolderEntity_Origin(VectorParam *p) : param(p) { }
    virtual ~VectorParamKnotHolderEntity_Origin() {}

    virtual void knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint /*state*/) {
        Geom::Point const s = snap_knot_position(p);
        param->setOrigin(s);
        sp_lpe_item_update_patheffect(SP_LPE_ITEM(item), false, false);
    };
    virtual Geom::Point knot_get(){
        return param->origin;
    };
    virtual void knot_click(guint /*state*/){
        g_print ("This is the origin handle associated to parameter '%s'\n", param->param_key.c_str());
    };

private:
    VectorParam *param;
};

class VectorParamKnotHolderEntity_Vector : public LPEKnotHolderEntity {
public:
    VectorParamKnotHolderEntity_Vector(VectorParam *p) : param(p) { }
    virtual ~VectorParamKnotHolderEntity_Vector() {}

    virtual void knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint /*state*/) {
        Geom::Point const s = p - param->origin;
        /// @todo implement angle snapping when holding CTRL
        param->setVector(s);
        sp_lpe_item_update_patheffect(SP_LPE_ITEM(item), false, false);
    };
    virtual Geom::Point knot_get(){
        return param->origin + param->vector;
    };
    virtual void knot_click(guint /*state*/){
        g_print ("This is the vector handle associated to parameter '%s'\n", param->param_key.c_str());
    };

private:
    VectorParam *param;
};

void
VectorParam::addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item)
{
    VectorParamKnotHolderEntity_Origin *origin_e = new VectorParamKnotHolderEntity_Origin(this);
    origin_e->create(desktop, item, knotholder, handleTip(), ori_knot_shape, ori_knot_mode, ori_knot_color);
    knotholder->add(origin_e);

    VectorParamKnotHolderEntity_Vector *vector_e = new VectorParamKnotHolderEntity_Vector(this);
    vector_e->create(desktop, item, knotholder, handleTip(), vec_knot_shape, vec_knot_mode, vec_knot_color);
    knotholder->add(vector_e);
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
