#ifndef __SP_SVG_H__
#define __SP_SVG_H__

/*
 * SVG data parser
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include <glib/gtypes.h>
#include <vector>
#include <cstring>
#include <string>

#include "svg/svg-length.h"
#include "libnr/nr-forward.h"

/* Generic */

/*
 * These are very-very simple:
 * - they accept everything libc strtod accepts
 * - no valid end character checking
 * Return FALSE and let val untouched on error
 */
 
unsigned int sp_svg_number_read_f (const gchar *str, float *val);
unsigned int sp_svg_number_read_d (const gchar *str, double *val);

/*
 * No buffer overflow checking is done, so better wrap them if needed
 */
unsigned int sp_svg_number_write_de (gchar *buf, double val, unsigned int tprec, int min_exp, unsigned int padf);

/* Length */

/*
 * Parse number with optional unit specifier:
 * - for px, pt, pc, mm, cm, computed is final value accrding to SVG spec
 * - for em, ex, and % computed is left untouched
 * - % is divided by 100 (i.e. 100% is 1.0)
 * !isalnum check is done at the end
 * Any return value pointer can be NULL
 */

unsigned int sp_svg_length_read_computed_absolute (const gchar *str, float *length);
std::vector<SVGLength> sp_svg_length_list_read (const gchar *str);
unsigned int sp_svg_length_read_ldd (const gchar *str, SVGLength::Unit *unit, double *value, double *computed);

std::string sp_svg_length_write_with_units(SVGLength const &length);

bool sp_svg_transform_read(gchar const *str, NR::Matrix *transform);

gchar *sp_svg_transform_write(NR::Matrix const &transform);
gchar *sp_svg_transform_write(NR::Matrix const *transform);

double sp_svg_read_percentage (const char * str, double def);

/* NB! As paths can be long, we use here dynamic string */

NArtBpath * sp_svg_read_path (const char * str);
char * sp_svg_write_path (const NArtBpath * bpath);


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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
