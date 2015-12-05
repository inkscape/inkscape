/**
 * SVG data parser
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * This code is in public domain
 */

#include <cmath>
#include <cstring>
#include <string>
#include <glib.h>
#include <iostream>

#include "svg.h"
#include "stringstream.h"
#include "util/units.h"

static unsigned sp_svg_length_read_lff(gchar const *str, SVGLength::Unit *unit, float *val, float *computed, char **next);

#ifndef MAX
# define MAX(a,b) ((a < b) ? (b) : (a))
#endif

unsigned int sp_svg_number_read_f(gchar const *str, float *val)
{
    if (!str) {
        return 0;
    }

    char *e;
    float const v = g_ascii_strtod(str, &e);
    if ((gchar const *) e == str) {
        return 0;
    }

    *val = v;
    return 1;
}

unsigned int sp_svg_number_read_d(gchar const *str, double *val)
{
    if (!str) {
        return 0;
    }

    char *e;
    double const v = g_ascii_strtod(str, &e);
    if ((gchar const *) e == str) {
        return 0;
    }

    *val = v;
    return 1;
}

// TODO must add a buffer length parameter for safety:
// rewrite using std::string?
static unsigned int sp_svg_number_write_ui(gchar *buf, unsigned int val)
{
    unsigned int i = 0;
    char c[16u];
    do {
        c[16u - (++i)] = '0' + (val % 10u);
        val /= 10u;
    } while (val > 0u);

    memcpy(buf, &c[16u - i], i);
    buf[i] = 0;

    return i;
}

// TODO unsafe code ingnoring bufLen
// rewrite using std::string?
static unsigned int sp_svg_number_write_i(gchar *buf, int bufLen, int val)
{
    int p = 0;
    unsigned int uval;
    if (val < 0) {
        buf[p++] = '-';
        uval = (unsigned int)-val;
    } else {
        uval = (unsigned int)val;
    }

    p += sp_svg_number_write_ui(buf+p, uval);

    return p;
}

// TODO unsafe code ingnoring bufLen
// rewrite using std::string?
static unsigned sp_svg_number_write_d(gchar *buf, int bufLen, double val, unsigned int tprec, unsigned int fprec)
{
    /* Process sign */
    int i = 0;
    if (val < 0.0) {
        buf[i++] = '-';
        val = fabs(val);
    }

    /* Determine number of integral digits */
    int idigits = 0;
    if (val >= 1.0) {
        idigits = (int) floor(log10(val)) + 1;
    }

    /* Determine the actual number of fractional digits */
    fprec = MAX(static_cast<int>(fprec), static_cast<int>(tprec) - idigits);
    /* Round value */
    val += 0.5 / pow(10.0, fprec);
    /* Extract integral and fractional parts */
    double dival = floor(val);
    double fval = val - dival;
    /* Write integra */
    if (idigits > (int)tprec) {
        i += sp_svg_number_write_ui(buf + i, (unsigned int)floor(dival/pow(10.0, idigits-tprec) + .5));
        for(unsigned int j=0; j<(unsigned int)idigits-tprec; j++) {
            buf[i+j] = '0';
        }
        i += idigits-tprec;
    } else {
        i += sp_svg_number_write_ui(buf + i, (unsigned int)dival);
    }
    int end_i = i;
    if (fprec > 0 && fval > 0.0) {
        buf[i++] = '.';
        do {
            fval *= 10.0;
            dival = floor(fval);
            fval -= dival;
            int const int_dival = (int) dival;
            buf[i++] = '0' + int_dival;
            if (int_dival != 0) {
                end_i = i;
            }
            fprec -= 1;
        } while(fprec > 0 && fval > 0.0);
    }
    buf[end_i] = 0;
    return end_i;
}

unsigned int sp_svg_number_write_de(gchar *buf, int bufLen, double val, unsigned int tprec, int min_exp)
{
    int eval = (int)floor(log10(fabs(val)));
    if (val == 0.0 || eval < min_exp) {
        return sp_svg_number_write_ui(buf, 0);
    }
    unsigned int maxnumdigitsWithoutExp = // This doesn't include the sign because it is included in either representation
        eval<0?tprec+(unsigned int)-eval+1:
        eval+1<(int)tprec?tprec+1:
        (unsigned int)eval+1;
    unsigned int maxnumdigitsWithExp = tprec + ( eval<0 ? 4 : 3 ); // It's not necessary to take larger exponents into account, because then maxnumdigitsWithoutExp is DEFINITELY larger
    if (maxnumdigitsWithoutExp <= maxnumdigitsWithExp) {
        return sp_svg_number_write_d(buf, bufLen, val, tprec, 0);
    } else {
        val = eval < 0 ? val * pow(10.0, -eval) : val / pow(10.0, eval);
        int p = sp_svg_number_write_d(buf, bufLen, val, tprec, 0);
        buf[p++] = 'e';
        p += sp_svg_number_write_i(buf + p, bufLen - p, eval);
        return p;
    }
}

SVGLength::SVGLength()
    : _set(false)
    , unit(NONE)
    , value(0)
    , computed(0)
{
}

/* Length */

bool SVGLength::read(gchar const *str)
{
    if (!str) {
        return false;
    }

    SVGLength::Unit u;
    float v;
    float c;
    if (!sp_svg_length_read_lff(str, &u, &v, &c, NULL)) {
        return false;
    }

    _set = true;
    unit = u;
    value = v;
    computed = c;

    return true;
}

static bool svg_length_absolute_unit(SVGLength::Unit u)
{
    return (u != SVGLength::EM && u != SVGLength::EX && u != SVGLength::PERCENT);
}

bool SVGLength::readAbsolute(gchar const *str)
{
    if (!str) {
        return false;
    }

    SVGLength::Unit u;
    float v;
    float c;
    if (!sp_svg_length_read_lff(str, &u, &v, &c, NULL)) {
        return false;
    }

    if (svg_length_absolute_unit(u) == false) {
        return false;
    }

    _set = true;
    unit = u;
    value = v;
    computed = c;

    return true;
}


unsigned int sp_svg_length_read_computed_absolute(gchar const *str, float *length)
{
    if (!str) {
        return 0;
    }

    SVGLength::Unit unit;
    float computed;
    if (!sp_svg_length_read_lff(str, &unit, NULL, &computed, NULL)) {
        // failed to read
        return 0;
    }

    if (svg_length_absolute_unit(unit) == false) {
        return 0;
    }

    *length = computed;

    return 1;
}

std::vector<SVGLength> sp_svg_length_list_read(gchar const *str)
{
    if (!str) {
        return std::vector<SVGLength>();
    }

    SVGLength::Unit unit;
    float value;
    float computed;
    char *next = (char *) str;
    std::vector<SVGLength> list;

    while (sp_svg_length_read_lff(next, &unit, &value, &computed, &next)) {

        SVGLength length;
        length.set(unit, value, computed);
        list.push_back(length);

        while (next && *next &&
               (*next == ',' || *next == ' ' || *next == '\n' || *next == '\r' || *next == '\t')) {
            // the list can be comma- or space-separated, but we will be generous and accept
            // a mix, including newlines and tabs
            next++;
        }

        if (!next || !*next) {
            break;
        }
    }

    return list;
}


#define UVAL(a,b) (((unsigned int) (a) << 8) | (unsigned int) (b))

static unsigned sp_svg_length_read_lff(gchar const *str, SVGLength::Unit *unit, float *val, float *computed, char **next)
{
/* note: this function is sometimes fed a string with several consecutive numbers, e.g. by sp_svg_length_list_read.
So after the number, the string does not necessarily have a \0 or a unit, it might also contain a space or comma and then the next number!
*/

    if (!str) {
        return 0;
    }

    gchar const *e;
    float const v = g_ascii_strtod(str, (char **) &e);
    if (e == str) {
        return 0;
    }

    if (!e[0]) {
        /* Unitless */
        if (unit) {
            *unit = SVGLength::NONE;
        }
        if (val) {
            *val = v;
        }
        if (computed) {
            *computed = v;
        }
        if (next) {
            *next = NULL; // no more values
        }
        return 1;
    } else if (!g_ascii_isalnum(e[0])) {
        /* Unitless or percent */
        if (e[0] == '%') {
            /* Percent */
            if (e[1] && g_ascii_isalnum(e[1])) {
                return 0;
            }
            if (unit) {
                *unit = SVGLength::PERCENT;
            }
            if (val) {
                *val = v * 0.01;
            }
            if (computed) {
                *computed = v * 0.01;
            }
            if (next) {
                *next = (char *) e + 1;
            }
            return 1;
        } else if (g_ascii_isspace(e[0]) && e[1] && g_ascii_isalpha(e[1])) {
            return 0; // spaces between value and unit are not allowed
        } else {
            /* Unitless */
            if (unit) {
                *unit = SVGLength::NONE;
            }
            if (val) {
                *val = v;
            }
            if (computed) {
                *computed = v;
            }
            if (next) {
                *next = (char *) e;
            }
            return 1;
        }
    } else if (e[1] && !g_ascii_isalnum(e[2])) {
        /* TODO: Allow the number of px per inch to vary (document preferences, X server
         * or whatever).  E.g. don't fill in computed here, do it at the same time as
         * percentage units are done. */
        unsigned int const uval = UVAL(e[0], e[1]);
        switch (uval) {
            case UVAL('p','x'):
                if (unit) {
                    *unit = SVGLength::PX;
                }
                if (computed) {
                    *computed = v;
                }
                break;
            case UVAL('p','t'):
                if (unit) {
                    *unit = SVGLength::PT;
                }
                if (computed) {
                    *computed = Inkscape::Util::Quantity::convert(v, "pt", "px");
                }
                break;
            case UVAL('p','c'):
                if (unit) {
                    *unit = SVGLength::PC;
                }
                if (computed) {
                    *computed = Inkscape::Util::Quantity::convert(v, "pc", "px");
                }
                break;
            case UVAL('m','m'):
                if (unit) {
                    *unit = SVGLength::MM;
                }
                if (computed) {
                    *computed = Inkscape::Util::Quantity::convert(v, "mm", "px");
                }
                break;
            case UVAL('c','m'):
                if (unit) {
                    *unit = SVGLength::CM;
                }
                if (computed) {
                    *computed = Inkscape::Util::Quantity::convert(v, "cm", "px");
                }
                break;
            case UVAL('i','n'):
                if (unit) {
                    *unit = SVGLength::INCH;
                }
                if (computed) {
                    *computed = Inkscape::Util::Quantity::convert(v, "in", "px");
                }
                break;
            case UVAL('f','t'):
                if (unit) {
                    *unit = SVGLength::FOOT;
                }
                if (computed) {
                    *computed = Inkscape::Util::Quantity::convert(v, "ft", "px");
                }
                break;
            case UVAL('e','m'):
                if (unit) {
                    *unit = SVGLength::EM;
                }
                break;
            case UVAL('e','x'):
                if (unit) {
                    *unit = SVGLength::EX;
                }
                break;
            default:
                /* Invalid */
                return 0;
                break;
        }
        if (val) {
            *val = v;
        }
        if (next) {
            *next = (char *) e + 2;
        }
        return 1;
    }

    /* Invalid */
    return 0;
}

unsigned int sp_svg_length_read_ldd(gchar const *str, SVGLength::Unit *unit, double *value, double *computed)
{
    float a;
    float b;
    unsigned int r = sp_svg_length_read_lff(str, unit, &a, &b, NULL);
    if (r) {
        if (value) {
            *value = a;
        }
        if (computed) {
            *computed = b;
        }
    }
    return r;
}

std::string SVGLength::write() const
{
    return sp_svg_length_write_with_units(*this);
}

void SVGLength::set(SVGLength::Unit u, float v)
{
    _set = true;
    unit = u;
    Glib::ustring hack("px");
    switch( unit ) {
        case NONE:
        case PX:
        case EM:
        case EX:
        case PERCENT:
            break;
        case PT:
            hack = "pt";
            break;
        case PC:
            hack = "pc";
            break;
        case MM:
            hack = "pt";
            break;
        case CM:
            hack = "pt";
            break;
        case INCH:
            hack = "pt";
            break;
        case FOOT:
            hack = "pt";
            break;
        case MITRE:
            hack = "m";
            break;
        default:
            break;
    }
    value = v;
    computed =  Inkscape::Util::Quantity::convert(v, hack, "px");
}

void SVGLength::set(SVGLength::Unit u, float v, float c)
{
    _set = true;
    unit = u;
    value = v;
    computed = c;
}

void SVGLength::unset(SVGLength::Unit u, float v, float c)
{
    _set = false;
    unit = u;
    value = v;
    computed = c;
}

void SVGLength::scale(double scale)
{
    value *= scale;
    computed *= scale;
}

void SVGLength::update(double em, double ex, double scale)
{
    if (unit == EM) {
        computed = value * em;
    } else if (unit == EX) {
        computed = value * ex;
    } else if (unit == PERCENT) {
        computed = value * scale;
    }
}

double sp_svg_read_percentage(char const *str, double def)
{
    if (str == NULL) {
        return def;
    }

    char *u;
    double v = g_ascii_strtod(str, &u);
    while (isspace(*u)) {
        if (*u == '\0') {
            return v;
        }
        u++;
    }
    if (*u == '%') {
        v /= 100.0;
    }

    return v;
}

gchar const *sp_svg_length_get_css_units(SVGLength::Unit unit)
{
    switch (unit) {
        case SVGLength::NONE: return "";
        case SVGLength::PX: return "";
        case SVGLength::PT: return "pt";
        case SVGLength::PC: return "pc";
        case SVGLength::MM: return "mm";
        case SVGLength::CM: return "cm";
        case SVGLength::INCH: return "in";
        case SVGLength::FOOT: return "";  // Not in SVG/CSS specification.
        case SVGLength::MITRE: return ""; // Not in SVG/CSS specification.
        case SVGLength::EM: return "em";
        case SVGLength::EX: return "ex";
        case SVGLength::PERCENT: return "%";
    }
    return "";
}

/**
 * N.B.\ This routine will sometimes return strings with `e' notation, so is unsuitable for CSS
 * lengths (which don't allow scientific `e' notation).
 */
std::string sp_svg_length_write_with_units(SVGLength const &length)
{
    Inkscape::SVGOStringStream os;
    if (length.unit == SVGLength::PERCENT) {
        os << 100*length.value << sp_svg_length_get_css_units(length.unit);
    } else if (length.unit == SVGLength::FOOT) {
        os << 12*length.value << sp_svg_length_get_css_units(SVGLength::INCH);
    } else if (length.unit == SVGLength::MITRE) {
        os << 100*length.value << sp_svg_length_get_css_units(SVGLength::CM);
    } else {
        os << length.value << sp_svg_length_get_css_units(length.unit);
    }
    return os.str();
}


void SVGLength::readOrUnset(gchar const *str, Unit u, float v, float c)
{
    if (!read(str)) {
        unset(u, v, c);
    }
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
