/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/widget/registered-widget.h"
#include "live_effects/parameter/random.h"
#include "live_effects/effect.h"
#include <glibmm/i18n.h>
#include "svg/svg.h"
#include "ui/widget/random.h"

#include "svg/stringstream.h"

#include "verbs.h"

#define noLPERANDOMPARAM_DEBUG

namespace Inkscape {

namespace LivePathEffect {


RandomParam::RandomParam( const Glib::ustring& label, const Glib::ustring& tip,
                      const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                      Effect* effect, gdouble default_value, long default_seed)
    : Parameter(label, tip, key, wr, effect)
{
    defvalue = default_value;
    value = defvalue;
    min = -Geom::infinity();
    max = Geom::infinity();
    integer = false;

    defseed = default_seed;
    startseed = defseed;
    seed = startseed;
}

RandomParam::~RandomParam()
{
}

bool
RandomParam::param_readSVGValue(const gchar * strvalue)
{
    double newval, newstartseed;
    gchar** stringarray = g_strsplit (strvalue, ";", 2);
    unsigned int success = sp_svg_number_read_d(stringarray[0], &newval);
    if (success == 1) {
        success += sp_svg_number_read_d(stringarray[1], &newstartseed);
        if (success == 2) {
            param_set_value(newval, static_cast<long>(newstartseed));
        } else {
            param_set_value(newval, defseed);
        }
        g_strfreev(stringarray);
        return true;
    }
    g_strfreev(stringarray);
    return false;
}

gchar *
RandomParam::param_getSVGValue() const
{
    Inkscape::SVGOStringStream os;
    os << value << ';' << startseed;
    gchar * str = g_strdup(os.str().c_str());
    return str;
}

void
RandomParam::param_set_default()
{
    param_set_value(defvalue, defseed);
}

void
RandomParam::param_set_value(gdouble val, long newseed)
{
    value = val;
    if (integer)
        value = round(value);
    if (value > max)
        value = max;
    if (value < min)
        value = min;

    startseed = setup_seed(newseed);
    seed = startseed;
}

void
RandomParam::param_set_range(gdouble min, gdouble max)
{
    this->min = min;
    this->max = max;
}

void
RandomParam::param_make_integer(bool yes)
{
    integer = yes;
}

void
RandomParam::resetRandomizer()
{
    seed = startseed;
}


Gtk::Widget *
RandomParam::param_newWidget()
{
    Inkscape::UI::Widget::RegisteredRandom* regrandom = Gtk::manage(
        new Inkscape::UI::Widget::RegisteredRandom( param_label,
                                                    param_tooltip,
                                                    param_key,
                                                    *param_wr,
                                                    param_effect->getRepr(),
                                                    param_effect->getSPDoc() )  );

    regrandom->setValue(value, startseed);
    if (integer) {
        regrandom->setDigits(0);
        regrandom->setIncrements(1, 10);
    }
    regrandom->setRange(min, max);
    regrandom->setProgrammatically = false;

    regrandom->set_undo_parameters(SP_VERB_DIALOG_LIVE_PATH_EFFECT, _("Change random parameter"));

    return dynamic_cast<Gtk::Widget *> (regrandom);
}

RandomParam::operator gdouble()
{
    return rand() * value;
};

/* RNG stolen from /display/nr-filter-turbulence.cpp */
#define RAND_m 2147483647 /* 2**31 - 1 */
#define RAND_a 16807 /* 7**5; primitive root of m */
#define RAND_q 127773 /* m / a */
#define RAND_r 2836 /* m % a */
#define BSize 0x100

long
RandomParam::setup_seed(long lSeed)
{
  if (lSeed <= 0) lSeed = -(lSeed % (RAND_m - 1)) + 1;
  if (lSeed > RAND_m - 1) lSeed = RAND_m - 1;
  return lSeed;
}

// generates random number between 0 and 1
gdouble
RandomParam::rand()
{
  long result;
  result = RAND_a * (seed % RAND_q) - RAND_r * (seed / RAND_q);
  if (result <= 0) result += RAND_m;
  seed = result;

  gdouble dresult = (gdouble)(result % BSize) / BSize;
  return dresult;
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
