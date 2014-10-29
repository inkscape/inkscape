#ifndef SEEN_SP_SVG_H
#define SEEN_SP_SVG_H

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

#include <vector>
#include <cstring>
#include <string>

#include "svg/svg-length.h"
#include <2geom/forward.h>

/* Generic */

/*
 * These are very-very simple:
 * - they accept everything libc strtod accepts
 * - no valid end character checking
 * Return FALSE and let val untouched on error
 */

unsigned int sp_svg_number_read_f( const char *str, float *val );
unsigned int sp_svg_number_read_d( const char *str, double *val );

/*
 * No buffer overflow checking is done, so better wrap them if needed
 */
unsigned int sp_svg_number_write_de( char *buf, int bufLen, double val, unsigned int tprec, int min_exp );

/* Length */

/*
 * Parse number with optional unit specifier:
 * - for px, pt, pc, mm, cm, computed is final value accrding to SVG spec
 * - for em, ex, and % computed is left untouched
 * - % is divided by 100 (i.e. 100% is 1.0)
 * !isalnum check is done at the end
 * Any return value pointer can be NULL
 */

unsigned int sp_svg_length_read_computed_absolute( const char *str, float *length );
std::vector<SVGLength> sp_svg_length_list_read( const char *str );
unsigned int sp_svg_length_read_ldd( const char *str, SVGLength::Unit *unit, double *value, double *computed );

std::string sp_svg_length_write_with_units(SVGLength const &length);

bool sp_svg_transform_read(char const *str, Geom::Affine *transform);

char *sp_svg_transform_write(Geom::Affine const &transform);
char *sp_svg_transform_write(Geom::Affine const *transform);

double sp_svg_read_percentage( const char * str, double def );

/* NB! As paths can be long, we use here dynamic string */

Geom::PathVector sp_svg_read_pathv( char const * str );
char * sp_svg_write_path( Geom::PathVector const &p );
char * sp_svg_write_path( Geom::Path const &p );

#endif // SEEN_SP_SVG_H

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
