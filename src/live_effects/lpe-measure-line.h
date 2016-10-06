#ifndef INKSCAPE_LPE_MEASURE_LINE_H
#define INKSCAPE_LPE_MEASURE_LINE_H

/*
 * Author(s):
 *     Jabiertxo Arraiza Cenoz <jabier.arraiza@marker.es>
 *
 * Copyright (C) 2014 Author(s)
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/enum.h"
#include "live_effects/parameter/fontbutton.h"
#include "live_effects/parameter/text.h"
#include "live_effects/parameter/unit.h"
#include "live_effects/parameter/bool.h"
#include "live_effects/parameter/originalpath.h"
#include <libnrtype/font-lister.h>
#include <2geom/angle.h>
#include <2geom/ray.h>
#include <2geom/point.h>

namespace Inkscape {
namespace LivePathEffect {

enum OrientationMethod {
    OM_HORIZONTAL,
    OM_VERTICAL,
    OM_PARALLEL,
    OM_END
};

class LPEMeasureLine : public Effect {
public:
    LPEMeasureLine(LivePathEffectObject *lpeobject);
    virtual ~LPEMeasureLine();
    virtual void doBeforeEffect (SPLPEItem const* lpeitem);
    virtual void doOnApply(SPLPEItem const* lpeitem);
    virtual void doOnRemove (SPLPEItem const* lpeitem);
    virtual void doOnVisibilityToggled(SPLPEItem const* /*lpeitem*/);
    virtual Geom::PathVector doEffect_path(Geom::PathVector const &path_in);
    void createLine(Geom::Point start,Geom::Point end,Glib::ustring id, bool main, bool overflow, bool remove, bool arrows = false);
    void createTextLabel(Geom::Point pos, double length, Geom::Coord angle, bool remove);
    void onExpanderChanged();
    void toObjects();
    void createArrowMarker(Glib::ustring mode);
    void saveDefault();
    virtual Gtk::Widget *newWidget();
private:
    UnitParam unit;
    FontButtonParam fontbutton;
    EnumParam<OrientationMethod> orientation;
    ScalarParam curve_linked;
    ScalarParam precision;
    ScalarParam position;
    ScalarParam text_top_bottom;
    ScalarParam text_right_left;
    ScalarParam helpline_distance;
    ScalarParam helpline_overlap;
    ScalarParam scale;
    TextParam format;
    BoolParam arrows_outside;
    BoolParam flip_side;
    BoolParam scale_insensitive;
    BoolParam local_locale;
    BoolParam line_group_05;
    BoolParam rotate_anotation;
    BoolParam hide_back;
    TextParam dimline_format;
    TextParam helperlines_format;
    TextParam anotation_format;
    TextParam arrows_format;
    Glib::ustring display_unit;
    bool expanded;
    Gtk::Expander * expander;
    double doc_scale;
    double fontsize;
    double anotation_width;
    double arrow_gap;
    bool erase;
/*    Geom::Affine affine_over;*/
    LPEMeasureLine(const LPEMeasureLine &);
    LPEMeasureLine &operator=(const LPEMeasureLine &);

};

} //namespace LivePathEffect
} //namespace Inkscape

#endif

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
