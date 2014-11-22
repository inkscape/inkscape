/**
 * \file
 * Reading \& writing of SVG/CSS colors.
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstdlib>
#include <cstdio> // sprintf
#include <cstring>
#include <string>
#include <cassert>
#include <math.h>
#include <glib.h> // g_assert
#include <errno.h>

#include <map>

#include "colorspace.h"
#include "strneq.h"
#include "preferences.h"
#include "svg-color.h"
#include "svg-icc-color.h"

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
#include "color.h"
#include "color-profile.h"
#include "document.h"
#include "inkscape.h"
#include "profile-manager.h"
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
#include "cms-system.h"

using std::sprintf;
using std::string;
using Inkscape::CMSSystem;

struct SPSVGColor {
    unsigned long rgb;
    const string name;
};

/*
 * These are the colors defined in the SVG standard
 */
static SPSVGColor const sp_svg_color_named[] = {
    { 0xF0F8FF, "aliceblue" },
    { 0xFAEBD7, "antiquewhite" },
    { 0x00FFFF, "aqua" },
    { 0x7FFFD4, "aquamarine" },
    { 0xF0FFFF, "azure" },
    { 0xF5F5DC, "beige" },
    { 0xFFE4C4, "bisque" },
    { 0x000000, "black" },
    { 0xFFEBCD, "blanchedalmond" },
    { 0x0000FF, "blue" },
    { 0x8A2BE2, "blueviolet" },
    { 0xA52A2A, "brown" },
    { 0xDEB887, "burlywood" },
    { 0x5F9EA0, "cadetblue" },
    { 0x7FFF00, "chartreuse" },
    { 0xD2691E, "chocolate" },
    { 0xFF7F50, "coral" },
    { 0x6495ED, "cornflowerblue" },
    { 0xFFF8DC, "cornsilk" },
    { 0xDC143C, "crimson" },
    { 0x00FFFF, "cyan" },
    { 0x00008B, "darkblue" },
    { 0x008B8B, "darkcyan" },
    { 0xB8860B, "darkgoldenrod" },
    { 0xA9A9A9, "darkgray" },
    { 0x006400, "darkgreen" },
    { 0xA9A9A9, "darkgrey" },
    { 0xBDB76B, "darkkhaki" },
    { 0x8B008B, "darkmagenta" },
    { 0x556B2F, "darkolivegreen" },
    { 0xFF8C00, "darkorange" },
    { 0x9932CC, "darkorchid" },
    { 0x8B0000, "darkred" },
    { 0xE9967A, "darksalmon" },
    { 0x8FBC8F, "darkseagreen" },
    { 0x483D8B, "darkslateblue" },
    { 0x2F4F4F, "darkslategray" },
    { 0x2F4F4F, "darkslategrey" },
    { 0x00CED1, "darkturquoise" },
    { 0x9400D3, "darkviolet" },
    { 0xFF1493, "deeppink" },
    { 0x00BFFF, "deepskyblue" },
    { 0x696969, "dimgray" },
    { 0x696969, "dimgrey" },
    { 0x1E90FF, "dodgerblue" },
    { 0xB22222, "firebrick" },
    { 0xFFFAF0, "floralwhite" },
    { 0x228B22, "forestgreen" },
    { 0xFF00FF, "fuchsia" },
    { 0xDCDCDC, "gainsboro" },
    { 0xF8F8FF, "ghostwhite" },
    { 0xFFD700, "gold" },
    { 0xDAA520, "goldenrod" },
    { 0x808080, "gray" },
    { 0x808080, "grey" },
    { 0x008000, "green" },
    { 0xADFF2F, "greenyellow" },
    { 0xF0FFF0, "honeydew" },
    { 0xFF69B4, "hotpink" },
    { 0xCD5C5C, "indianred" },
    { 0x4B0082, "indigo" },
    { 0xFFFFF0, "ivory" },
    { 0xF0E68C, "khaki" },
    { 0xE6E6FA, "lavender" },
    { 0xFFF0F5, "lavenderblush" },
    { 0x7CFC00, "lawngreen" },
    { 0xFFFACD, "lemonchiffon" },
    { 0xADD8E6, "lightblue" },
    { 0xF08080, "lightcoral" },
    { 0xE0FFFF, "lightcyan" },
    { 0xFAFAD2, "lightgoldenrodyellow" },
    { 0xD3D3D3, "lightgray" },
    { 0x90EE90, "lightgreen" },
    { 0xD3D3D3, "lightgrey" },
    { 0xFFB6C1, "lightpink" },
    { 0xFFA07A, "lightsalmon" },
    { 0x20B2AA, "lightseagreen" },
    { 0x87CEFA, "lightskyblue" },
    { 0x778899, "lightslategray" },
    { 0x778899, "lightslategrey" },
    { 0xB0C4DE, "lightsteelblue" },
    { 0xFFFFE0, "lightyellow" },
    { 0x00FF00, "lime" },
    { 0x32CD32, "limegreen" },
    { 0xFAF0E6, "linen" },
    { 0xFF00FF, "magenta" },
    { 0x800000, "maroon" },
    { 0x66CDAA, "mediumaquamarine" },
    { 0x0000CD, "mediumblue" },
    { 0xBA55D3, "mediumorchid" },
    { 0x9370DB, "mediumpurple" },
    { 0x3CB371, "mediumseagreen" },
    { 0x7B68EE, "mediumslateblue" },
    { 0x00FA9A, "mediumspringgreen" },
    { 0x48D1CC, "mediumturquoise" },
    { 0xC71585, "mediumvioletred" },
    { 0x191970, "midnightblue" },
    { 0xF5FFFA, "mintcream" },
    { 0xFFE4E1, "mistyrose" },
    { 0xFFE4B5, "moccasin" },
    { 0xFFDEAD, "navajowhite" },
    { 0x000080, "navy" },
    { 0xFDF5E6, "oldlace" },
    { 0x808000, "olive" },
    { 0x6B8E23, "olivedrab" },
    { 0xFFA500, "orange" },
    { 0xFF4500, "orangered" },
    { 0xDA70D6, "orchid" },
    { 0xEEE8AA, "palegoldenrod" },
    { 0x98FB98, "palegreen" },
    { 0xAFEEEE, "paleturquoise" },
    { 0xDB7093, "palevioletred" },
    { 0xFFEFD5, "papayawhip" },
    { 0xFFDAB9, "peachpuff" },
    { 0xCD853F, "peru" },
    { 0xFFC0CB, "pink" },
    { 0xDDA0DD, "plum" },
    { 0xB0E0E6, "powderblue" },
    { 0x800080, "purple" },
    { 0x663399, "rebeccapurple" },
    { 0xFF0000, "red" },
    { 0xBC8F8F, "rosybrown" },
    { 0x4169E1, "royalblue" },
    { 0x8B4513, "saddlebrown" },
    { 0xFA8072, "salmon" },
    { 0xF4A460, "sandybrown" },
    { 0x2E8B57, "seagreen" },
    { 0xFFF5EE, "seashell" },
    { 0xA0522D, "sienna" },
    { 0xC0C0C0, "silver" },
    { 0x87CEEB, "skyblue" },
    { 0x6A5ACD, "slateblue" },
    { 0x708090, "slategray" },
    { 0x708090, "slategrey" },
    { 0xFFFAFA, "snow" },
    { 0x00FF7F, "springgreen" },
    { 0x4682B4, "steelblue" },
    { 0xD2B48C, "tan" },
    { 0x008080, "teal" },
    { 0xD8BFD8, "thistle" },
    { 0xFF6347, "tomato" },
    { 0x40E0D0, "turquoise" },
    { 0xEE82EE, "violet" },
    { 0xF5DEB3, "wheat" },
    { 0xFFFFFF, "white" },
    { 0xF5F5F5, "whitesmoke" },
    { 0xFFFF00, "yellow" },
    { 0x9ACD32, "yellowgreen" }
};

static std::map<string, unsigned long> sp_svg_create_color_hash();

guint32 sp_svg_read_color(gchar const *str, guint32 const dfl)
{
    return sp_svg_read_color(str, NULL, dfl);
}

static guint32 internal_sp_svg_read_color(gchar const *str, gchar const **end_ptr, guint32 def)
{
    static std::map<string, unsigned long> colors;
    guint32 val = 0;

    if (str == NULL) return def;
    while ((*str <= ' ') && *str) str++;
    if (!*str) return def;

    if (str[0] == '#') {
        gint i;
        for (i = 1; str[i]; i++) {
            int hexval;
            if (str[i] >= '0' && str[i] <= '9')
                hexval = str[i] - '0';
            else if (str[i] >= 'A' && str[i] <= 'F')
                hexval = str[i] - 'A' + 10;
            else if (str[i] >= 'a' && str[i] <= 'f')
                hexval = str[i] - 'a' + 10;
            else
                break;
            val = (val << 4) + hexval;
        }
        /* handle #rgb case */
        if (i == 1 + 3) {
            val = ((val & 0xf00) << 8) |
                ((val & 0x0f0) << 4) |
                (val & 0x00f);
            val |= val << 4;
        } else if (i != 1 + 6) {
            /* must be either 3 or 6 digits. */
            return def;
        }
        if (end_ptr) {
            *end_ptr = str + i;
        }
    } else if (strneq(str, "rgb(", 4)) {
        bool hasp, hasd;
        gchar *s, *e;
        gdouble r, g, b;

        s = (gchar *) str + 4;
        hasp = false;
        hasd = false;

        r = g_ascii_strtod(s, &e);
        if (s == e) return def;
        s = e;
        if (*s == '%') {
            hasp = true;
            s += 1;
        } else {
            hasd = true;
        }
        while (*s && g_ascii_isspace(*s)) s += 1;
        if (*s != ',') return def;
        s += 1;
        while (*s && g_ascii_isspace(*s)) s += 1;
        g = g_ascii_strtod(s, &e);
        if (s == e) return def;
        s = e;
        if (*s == '%') {
            hasp = true;
            s += 1;
        } else {
            hasd = true;
        }
        while (*s && g_ascii_isspace(*s)) s += 1;
        if (*s != ',') return def;
        s += 1;
        while (*s && g_ascii_isspace(*s)) s += 1;
        b = g_ascii_strtod(s, &e);
        if (s == e) return def;
        s = e;
        if (*s == '%') {
            hasp = true;
            s += 1;
        } else {
            hasd = true;
        }
        while(*s && g_ascii_isspace(*s)) s += 1;
        if (*s != ')') {
            return def;
        }
        ++s;
        if (hasp && hasd) return def;
        if (hasp) {
            val = static_cast<guint>(floor(CLAMP(r, 0.0, 100.0) * 2.559999)) << 24;
            val |= (static_cast<guint>(floor(CLAMP(g, 0.0, 100.0) * 2.559999)) << 16);
            val |= (static_cast<guint>(floor(CLAMP(b, 0.0, 100.0) * 2.559999)) << 8);
        } else {
            val = static_cast<guint>(CLAMP(r, 0, 255)) << 24;
            val |= (static_cast<guint>(CLAMP(g, 0, 255)) << 16);
            val |= (static_cast<guint>(CLAMP(b, 0, 255)) << 8);
        }
        if (end_ptr) {
            *end_ptr = s;
        }
        return val;
    } else if (strneq(str, "hsl(", 4)) {

        gchar *ptr = (gchar *) str + 4;

        gchar *e; // ptr after read

        double h = g_ascii_strtod(ptr, &e); // Read h (0-360)
        if (ptr == e) return def; // Read failed
        ptr = e;

        while (*ptr && g_ascii_isspace(*ptr)) ptr += 1; // Remove any white space
        if (*ptr != ',') return def; // Need comma
        ptr += 1;
        while (*ptr && g_ascii_isspace(*ptr)) ptr += 1; // Remove any white space

        double s = g_ascii_strtod(ptr, &e); // Read s (percent)
        if (ptr == e) return def; // Read failed
        ptr = e;
        while (*ptr && g_ascii_isspace(*ptr)) ptr += 1; // Remove any white space
        if (*ptr != '%') return def; // Need %
        ptr += 1;

        while (*ptr && g_ascii_isspace(*ptr)) ptr += 1; // Remove any white space
        if (*ptr != ',') return def; // Need comma
        ptr += 1;
        while (*ptr && g_ascii_isspace(*ptr)) ptr += 1; // Remove any white space

        double l = g_ascii_strtod(ptr, &e); // Read l (percent)
        if (ptr == e) return def; // Read failed
        ptr = e;
        while (*ptr && g_ascii_isspace(*ptr)) ptr += 1; // Remove any white space
        if (*ptr != '%') return def; // Need %
        ptr += 1;

        if (end_ptr) {
            *end_ptr = ptr;
        }


        // Normalize to 0..1
        h /= 360.0;
        s /= 100.0;
        l /= 100.0;

        gfloat rgb[3];

        sp_color_hsl_to_rgb_floatv( rgb, h, s, l );

        val  =  static_cast<guint>(floor(CLAMP(rgb[0], 0.0, 1.0) * 255.9999)) << 24;
        val |= (static_cast<guint>(floor(CLAMP(rgb[1], 0.0, 1.0) * 255.9999)) << 16);
        val |= (static_cast<guint>(floor(CLAMP(rgb[2], 0.0, 1.0) * 255.9999)) << 8);
        return val;

    } else {
        gint i;
        if (colors.empty()) {
            colors = sp_svg_create_color_hash();
        }
        gchar c[32];
        for (i = 0; i < 31; i++) {
            if (str[i] == ';' || g_ascii_isspace(str[i])) {
                c[i] = '\0';
                break;
            }
            c[i] = g_ascii_tolower(str[i]);
            if (!str[i]) break;
        }
        c[31] = '\0';

        if (colors.count(string(c))) {
            val = colors[string(c)];
        }
        else {
            return def;
        }
        if (end_ptr) {
            *end_ptr = str + i;
        }
    }

    return (val << 8);
}

guint32 sp_svg_read_color(gchar const *str, gchar const **end_ptr, guint32 dfl)
{
    /* I've been rather hurried in editing the above to add support for end_ptr, so I'm adding
     * this check wrapper. */
    gchar const *end = str;
    guint32 const ret = internal_sp_svg_read_color(str, &end, dfl);
    assert(((ret == dfl) && (end == str))
           || (((ret & 0xff) == 0)
               && (str < end)));
    if (str < end) {
        gchar *buf = (gchar *) g_malloc(end + 1 - str);
        memcpy(buf, str, end - str);
        buf[end - str] = '\0';
        gchar const *buf_end = buf;
        guint32 const check = internal_sp_svg_read_color(buf, &buf_end, 1);
        assert(check == ret
               && buf_end - buf == end - str);
        g_free(buf);

        if ( end_ptr ) {
            *end_ptr = end;
        }
    }
    return ret;
}


/**
 * Converts an RGB colour expressed in form 0x00rrggbb to a CSS/SVG representation of that colour.
 * The result is valid even in SVG Tiny or non-SVG CSS.
 */
static void rgb24_to_css(char *const buf, unsigned const rgb24)
{
    assert(rgb24 < (1u << 24));

    /* SVG 1.1 Full allows additional colour names not supported by SVG Tiny, but we don't bother
     * with them: it's good for these colours to be copyable to non-SVG CSS stylesheets and for
     * documents to be more viewable in SVG Tiny/Basic viewers; and some of the SVG Full names are
     * less meaningful than hex equivalents anyway.  And it's easier for a person to map from the
     * restricted set because the only component values are {00,80,ff}, other than silver 0xc0c0c0.
     */

    char const *src = NULL;
    switch (rgb24) {
        /* Extracted mechanically from the table at
         * http://www.w3.org/TR/REC-html40/types.html#h-6.5 .*/
        case 0x000000: src = "black"; break;
        case 0xc0c0c0: src = "silver"; break;
        case 0x808080: src = "gray"; break;
        case 0xffffff: src = "white"; break;
        case 0x800000: src = "maroon"; break;
        case 0xff0000: src = "red"; break;
        case 0x800080: src = "purple"; break;
        case 0xff00ff: src = "fuchsia"; break;
        case 0x008000: src = "green"; break;
        case 0x00ff00: src = "lime"; break;
        case 0x808000: src = "olive"; break;
        case 0xffff00: src = "yellow"; break;
        case 0x000080: src = "navy"; break;
        case 0x0000ff: src = "blue"; break;
        case 0x008080: src = "teal"; break;
        case 0x00ffff: src = "aqua"; break;

        default: {
            if ((rgb24 & 0xf0f0f) * 0x11 == rgb24) {
                /* Can use the shorter three-digit form #rgb instead of #rrggbb. */
                sprintf(buf, "#%x%x%x",
                        (rgb24 >> 16) & 0xf,
                        (rgb24 >> 8) & 0xf,
                        rgb24 & 0xf);
            } else {
                sprintf(buf, "#%06x", rgb24);
            }
            break;
        }
    }
    if (src) {
        strcpy(buf, src);
    }

    // assert(sp_svg_read_color(buf, 0xff) == (rgb24 << 8));
}

/**
 * Converts an RGBA32 colour to a CSS/SVG representation of the RGB portion of that colour.  The
 * result is valid even in SVG Tiny or non-SVG CSS.
 *
 * \param rgba32 Colour expressed in form 0xrrggbbaa.
 * \pre buflen \>= 8.
 */
void sp_svg_write_color(gchar *buf, unsigned const buflen, guint32 const rgba32)
{
    g_assert(8 <= buflen);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    unsigned const rgb24 = rgba32 >> 8;
    if (prefs->getBool("/options/svgoutput/usenamedcolors")) {
        rgb24_to_css(buf, rgb24);
    } else {
        g_snprintf(buf, buflen, "#%06x", rgb24);
    }
}

static std::map<string, unsigned long>
sp_svg_create_color_hash()
{
    std::map<string, unsigned long> colors;

    for (unsigned i = 0 ; i < G_N_ELEMENTS(sp_svg_color_named) ; i++) {
        colors[sp_svg_color_named[i].name] = sp_svg_color_named[i].rgb;
    }
    return colors;
}

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

void icc_color_to_sRGB(SVGICCColor* icc, guchar* r, guchar* g, guchar* b)
{
    if (icc) {
g_message("profile name: %s", icc->colorProfile.c_str());
        Inkscape::ColorProfile* prof = SP_ACTIVE_DOCUMENT->profileManager->find(icc->colorProfile.c_str());
        if ( prof ) {
            guchar color_out[4] = {0,0,0,0};
            cmsHTRANSFORM trans = prof->getTransfToSRGB8();
            if ( trans ) {
                std::vector<colorspace::Component> comps = colorspace::getColorSpaceInfo( prof );

                size_t count = CMSSystem::getChannelCount( prof );
                size_t cap = std::min(count, comps.size());
                guchar color_in[4];
                for (size_t i = 0; i < cap; i++) {
                    color_in[i] = static_cast<guchar>((((gdouble)icc->colors[i]) * 256.0) * (gdouble)comps[i].scale);
                    g_message("input[%d]: %d", (int)i, (int)color_in[i]);
                }

                CMSSystem::doTransform( trans, color_in, color_out, 1 );
g_message("transform to sRGB done");
            }
            *r = color_out[0];
            *g = color_out[1];
            *b = color_out[2];
        }
    }
}
#endif //defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

/*
 * Some discussion at http://markmail.org/message/bhfvdfptt25kgtmj
 * Allowed ASCII first characters:  ':', 'A'-'Z', '_', 'a'-'z'
 * Allowed ASCII remaining chars add: '-', '.', '0'-'9',
 */
bool sp_svg_read_icc_color( gchar const *str, gchar const **end_ptr, SVGICCColor* dest )
{
    bool good = true;

    if ( end_ptr ) {
        *end_ptr = str;
    }
    if ( dest ) {
        dest->colorProfile.clear();
        dest->colors.clear();
    }

    if ( !str ) {
        // invalid input
        good = false;
    } else {
        while ( g_ascii_isspace(*str) ) {
            str++;
        }

        good = strneq( str, "icc-color(", 10 );

        if ( good ) {
            str += 10;
            while ( g_ascii_isspace(*str) ) {
                str++;
            }

            if ( !g_ascii_isalpha(*str)
                 && ( !(0x080 & *str) )
                 && (*str != '_')
                 && (*str != ':') ) {
                // Name must start with a certain type of character
                good = false;
            } else {
                while ( g_ascii_isdigit(*str) || g_ascii_isalpha(*str)
                        || (*str == '-') || (*str == ':') || (*str == '_') || (*str == '.') ) {
                    if ( dest ) {
                        dest->colorProfile += *str;
                    }
                    str++;
                }
                while ( g_ascii_isspace(*str) || *str == ',' ) {
                    str++;
                }
            }
        }

        if ( good ) {
            while ( *str && *str != ')' ) {
                if ( g_ascii_isdigit(*str) || *str == '.' || *str == '-' || *str == '+') {
                    gchar* endPtr = 0;
                    gdouble dbl = g_ascii_strtod( str, &endPtr );
                    if ( !errno ) {
                        if ( dest ) {
                            dest->colors.push_back( dbl );
                        }
                        str = endPtr;
                    } else {
                        good = false;
                        break;
                    }

                    while ( g_ascii_isspace(*str) || *str == ',' ) {
                        str++;
                    }
                } else {
                    break;
                }
            }
        }

        // We need to have ended on a closing parenthesis
        if ( good ) {
            while ( g_ascii_isspace(*str) ) {
                str++;
            }
            good &= (*str == ')');
        }
    }

    if ( good ) {
        if ( end_ptr ) {
            *end_ptr = str;
        }
    } else {
        if ( dest ) {
            dest->colorProfile.clear();
            dest->colors.clear();
        }
    }

    return good;
}


bool sp_svg_read_icc_color( gchar const *str, SVGICCColor* dest )
{
    return sp_svg_read_icc_color(str, NULL, dest);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
