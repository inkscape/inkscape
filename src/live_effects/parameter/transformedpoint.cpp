/*
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/widget/registered-widget.h"
#include "live_effects/parameter/transformedpoint.h"
#include "sp-lpe-item.h"
#include "knotholder.h"
#include "svg/svg.h"
#include "svg/stringstream.h"

#include "live_effects/effect.h"
#include "desktop.h"
#include "verbs.h"

#include <glibmm/i18n.h>

namespace Inkscape {

namespace LivePathEffect {

TransformedPointParam::TransformedPointParam( const Glib::ustring& label, const Glib::ustring& tip,
                        const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                        Effect* effect, Geom::Point default_vector,
                        bool dontTransform)
    : Parameter(label, tip, key, wr, effect),
      defvalue(default_vector),
      origin(0.,0.),
      vector(default_vector),
      noTransform(dontTransform)
{
    vec_knot_shape = SP_KNOT_SHAPE_DIAMOND;
    vec_knot_mode  = SP_KNOT_MODE_XOR;
    vec_knot_color = 0xffffb500;
}

TransformedPointParam::~TransformedPointParam()
{

}

void
TransformedPointParam::param_set_default()
{
    setOrigin(Geom::Point(0.,0.));
    setVector(defvalue);
}

bool
TransformedPointParam::param_readSVGValue(const gchar * strvalue)
{
    gchar ** strarray = g_strsplit(strvalue, ",", 4);
    if (!strarray) {
        return false;
    }
    double val[4];
    unsigned int i = 0;
    while (i < 4 && strarray[i]) {
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
TransformedPointParam::param_getSVGValue() const
{
    Inkscape::SVGOStringStream os;
    os << origin << " , " << vector;
    gchar * str = g_strdup(os.str().c_str());
    return str;
}

Gtk::Widget *
TransformedPointParam::param_newWidget()
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
TransformedPointParam::set_and_write_new_values(Geom::Point const &new_origin, Geom::Point const &new_vector)
{
    setValues(new_origin, new_vector);
    gchar * str = param_getSVGValue();
    param_write_to_repr(str);
    g_free(str);
}

void
TransformedPointParam::param_transform_multiply(Geom::Affine const& postmul, bool /*set*/)
{
    if (!noTransform) {
        set_and_write_new_values( origin * postmul, vector * postmul.withoutTranslation() );
    }
}


void
TransformedPointParam::set_vector_oncanvas_looks(SPKnotShapeType shape, SPKnotModeType mode, guint32 color)
{
    vec_knot_shape = shape;
    vec_knot_mode  = mode;
    vec_knot_color = color;
}

void
TransformedPointParam::set_oncanvas_color(guint32 color)
{
    vec_knot_color = color;
}

class TransformedPointParamKnotHolderEntity_Vector : public KnotHolderEntity {
public:
    TransformedPointParamKnotHolderEntity_Vector(TransformedPointParam *p) : param(p) { }
    virtual ~TransformedPointParamKnotHolderEntity_Vector() {}

    virtual void knot_set(Geom::Point const &p, Geom::Point const &/*origin*/, guint /*state*/) {
        Geom::Point const s = p - param->origin;
        /// @todo implement angle snapping when holding CTRL
        param->setVector(s);
        sp_lpe_item_update_patheffect(SP_LPE_ITEM(item), false, false);
    };
    virtual Geom::Point knot_get() const{
        return param->origin + param->vector;
    };
    virtual void knot_click(guint /*state*/){
        g_print ("This is the vector handle associated to parameter '%s'\n", param->param_key.c_str());
    };

private:
    TransformedPointParam *param;
};

void
TransformedPointParam::addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item)
{
    TransformedPointParamKnotHolderEntity_Vector *vector_e = new TransformedPointParamKnotHolderEntity_Vector(this);
    vector_e->create(desktop, item, knotholder, Inkscape::CTRL_TYPE_UNKNOWN, handleTip(), vec_knot_shape, vec_knot_mode, vec_knot_color);
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
