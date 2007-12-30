#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_CPP

/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/parameter.h"
#include "live_effects/effect.h"
#include "svg/svg.h"
#include "libnr/nr-values.h"

#include <gtkmm.h>
#include "ui/widget/scalar.h"

#include "svg/stringstream.h"

#include "verbs.h"

#define noLPEREALPARAM_DEBUG

namespace Inkscape {

namespace LivePathEffect {


Parameter::Parameter( const Glib::ustring& label, const Glib::ustring& tip,
                      const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                      Effect* effect )
    : oncanvas_editable(false)
{
    param_label = label;
    param_tooltip = tip;
    param_key = key;
    param_wr = wr;
    param_effect = effect;
}



/*###########################################
 *   REAL PARAM
 */
ScalarParam::ScalarParam( const Glib::ustring& label, const Glib::ustring& tip,
                      const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                      Effect* effect, gdouble default_value)
    : Parameter(label, tip, key, wr, effect)
{
    defvalue = default_value;
    value = defvalue;
    min = -NR_HUGE;
    max = NR_HUGE;
    integer = false;
    rsu = NULL;
    inc_step = 0.1;
    inc_page = 1;
    digits = 2;
}

ScalarParam::~ScalarParam()
{
    if (rsu)
        delete rsu;
}

bool
ScalarParam::param_readSVGValue(const gchar * strvalue)
{
    double newval;
    unsigned int success = sp_svg_number_read_d(strvalue, &newval);
    if (success == 1) {
        param_set_value(newval);
        return true;
    }
    return false;
}

gchar *
ScalarParam::param_writeSVGValue() const
{
    Inkscape::SVGOStringStream os;
    os << value;
    gchar * str = g_strdup(os.str().c_str());
    return str;
}

void
ScalarParam::param_set_default()
{
    param_set_value(defvalue);
}

void
ScalarParam::param_set_value(gdouble val)
{
    value = val;
    if (integer)
        value = round(value);
    if (value > max)
        value = max;
    if (value < min)
        value = min;

    if (rsu && !rsu->is_updating())
        rsu->setValue(value);
}

void
ScalarParam::param_set_range(gdouble min, gdouble max)
{
    this->min = min;
    this->max = max;
    if (rsu)
        rsu->getS()->setRange(this->min, this->max);

    param_set_value(value); // reset value to see whether it is in ranges
}

void
ScalarParam::param_make_integer(bool yes)
{
    integer = yes;
    digits = 0;
    inc_step = 1;
    inc_page = 10;
    if (rsu) {
        rsu->getS()->setDigits(digits);
        rsu->getS()->setIncrements(inc_step, inc_page);
    }
}

Gtk::Widget *
ScalarParam::param_getWidget()
{
    if (!rsu) {
        rsu = new Inkscape::UI::Widget::RegisteredScalar();
        rsu->init(param_label, param_tooltip, param_key, *param_wr, param_effect->getRepr(), param_effect->getSPDoc());
        rsu->setValue(value);
        rsu->getS()->setDigits(digits);
        rsu->getS()->setIncrements(inc_step, inc_page);
        rsu->getS()->setRange(min, max);

        rsu->set_undo_parameters(SP_VERB_DIALOG_LIVE_PATH_EFFECT, _("Change scalar parameter"));
    }
    return dynamic_cast<Gtk::Widget *> (rsu->getS());
}

void
ScalarParam::param_set_digits(unsigned digits)
{
    this->digits = digits;
    if (rsu) {
        rsu->getS()->setDigits(digits);
    }
}

void
ScalarParam::param_set_increments(double step, double page)
{
    inc_step = step;
    inc_page = page;
    if (rsu) {
        rsu->getS()->setIncrements(inc_step, inc_page);
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
