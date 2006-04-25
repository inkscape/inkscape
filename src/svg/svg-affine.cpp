#define __SP_SVG_AFFINE_C__

/*
 * SVG data parser
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Raph Levien <raph@acm.org>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 1999 Raph Levien
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib/gstrfuncs.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-matrix-translate-ops.h>
#include <libnr/nr-rotate-fns.h>
#include <libnr/nr-rotate-matrix-ops.h>
#include <libnr/nr-scale-matrix-ops.h>
#include <libnr/nr-translate-matrix-ops.h>
#include <libnr/nr-translate-rotate-ops.h>
#include "svg.h"

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif

bool
sp_svg_transform_read(gchar const *str, NR::Matrix *transform)
{
	int idx;
	char keyword[32];
	double args[6];
	int n_args;
	size_t key_len;

	if (str == NULL) return false;

	NR::Matrix a(NR::identity());

	idx = 0;
	while (str[idx]) {
		/* skip initial whitespace */
		while (g_ascii_isspace (str[idx])) idx++;

		/* parse keyword */
		for (key_len = 0; key_len < sizeof (keyword); key_len++) {
			char c;

			c = str[idx];
			if (g_ascii_isalpha (c) || c == '-') {
				keyword[key_len] = str[idx++];
			} else {
				break;
			}
		}
		if (key_len >= sizeof (keyword)) return false;
		keyword[key_len] = '\0';

		/* skip whitespace */
		while (g_ascii_isspace (str[idx])) idx++;

		if (str[idx] != '(') return false;
		idx++;

		for (n_args = 0; ; n_args++) {
			char c;
			char *end_ptr;

			/* skip whitespace */
			while (g_ascii_isspace (str[idx])) idx++;
			c = str[idx];
			if (g_ascii_isdigit (c) || c == '+' || c == '-' || c == '.') {
				if (n_args == sizeof (args) / sizeof (args[0])) return false; /* Too many args */
				args[n_args] = g_ascii_strtod (str + idx, &end_ptr);
				
				//printf("took %d chars from '%s' to make %f\n",
				//		end_ptr-(str+idx),
				//		str+idx,
				//		args[n_args]);

				idx = end_ptr - (char *) str;

				while (g_ascii_isspace (str[idx])) idx++;

				/* skip optional comma */
				if (str[idx] == ',') idx++;
			} else if (c == ')') {
				break;
			} else {
				return false;
			}
		}
		idx++;

		/* ok, have parsed keyword and args, now modify the transform */
		if (!strcmp (keyword, "matrix")) {
			if (n_args != 6) return false;
			a = NR_MATRIX_D_FROM_DOUBLE(args) * a;
		} else if (!strcmp (keyword, "translate")) {
			if (n_args == 1) {
				args[1] = 0;
			} else if (n_args != 2) {
				return false;
			}
			a = NR::translate(args[0], args[1]) * a;
		} else if (!strcmp (keyword, "scale")) {
			if (n_args == 1) {
				args[1] = args[0];
			} else if (n_args != 2) {
				return false;
			}
			a = NR::scale(args[0], args[1]) * a;
		} else if (!strcmp (keyword, "rotate")) {
			if (n_args != 1 && n_args != 3) {
				return false;
			}
			NR::rotate const rot(rotate_degrees(args[0]));
			if (n_args == 3) {
				a = ( NR::translate(-args[1], -args[2])
				      * rot
				      * NR::translate(args[1], args[2])
				      * a );
			} else {
				a = rot * a;
			}
		} else if (!strcmp (keyword, "skewX")) {
			if (n_args != 1) return false;
			a = ( NR::Matrix(1, 0,
					 tan(args[0] * M_PI / 180.0), 1,
					 0, 0)
			      * a );
		} else if (!strcmp (keyword, "skewY")) {
			if (n_args != 1) return false;
			a = ( NR::Matrix(1, tan(args[0] * M_PI / 180.0),
					 0, 1,
					 0, 0)
			      * a );
		} else {
			return false; /* unknown keyword */
		}
		/* Skip trailing whitespace */
             while (g_ascii_isspace (str[idx])) idx++;
	}

	*transform = a;
	return true;
}

#define EQ(a,b) (fabs ((a) - (b)) < 1e-9)

unsigned
sp_svg_transform_write(gchar str[], unsigned const size, NR::Matrix const &transform)
{
	NRMatrix const t(transform);
	return sp_svg_transform_write(str, size, &t);
}

unsigned
sp_svg_transform_write(gchar str[], unsigned const size, NRMatrix const *transform)
{
	double e;

	if (!transform) {
		*str = 0;
		return 0;
	}

	e = 0.000001 * NR_MATRIX_DF_EXPANSION (transform);

	/* fixme: We could use t1 * t1 + t2 * t2 here instead */
	if (NR_DF_TEST_CLOSE (transform->c[1], 0.0, e) && NR_DF_TEST_CLOSE (transform->c[2], 0.0, e)) {
		if (NR_DF_TEST_CLOSE (transform->c[4], 0.0, e) && NR_DF_TEST_CLOSE (transform->c[5], 0.0, e)) {
			if (NR_DF_TEST_CLOSE (transform->c[0], 1.0, e) && NR_DF_TEST_CLOSE (transform->c[3], 1.0, e)) {
				/* We are more or less identity */
				*str = 0;
				return 0;
			} else {
				/* We are more or less scale */
				gchar c[256];
				unsigned p = 0;
				strcpy (c + p, "scale(");
				p += 6;
				p += sp_svg_number_write_de (c + p, transform->c[0], 6, -8, FALSE);
				c[p++] = ',';
				p += sp_svg_number_write_de (c + p, transform->c[3], 6, -8, FALSE);
				c[p++] = ')';
				g_assert( p <= sizeof(c) );
				p = MIN (p, size - 1 );
				memcpy (str, c, p);
				str[p] = 0;
				return p;
			}
		} else {
			if (NR_DF_TEST_CLOSE (transform->c[0], 1.0, e) && NR_DF_TEST_CLOSE (transform->c[3], 1.0, e)) {
				/* We are more or less translate */
				gchar c[256];
				unsigned p = 0;
				strcpy (c + p, "translate(");
				p += 10;
				p += sp_svg_number_write_de (c + p, transform->c[4], 6, -8, FALSE);
				c[p++] = ',';
				p += sp_svg_number_write_de (c + p, transform->c[5], 6, -8, FALSE);
				c[p++] = ')';
				g_assert( p <= sizeof(c) );
				p = MIN(p, size - 1);
				memcpy (str, c, p);
				str[p] = 0;
				return p;
			} else {
				gchar c[256];
				unsigned p = 0;
				strcpy (c + p, "matrix(");
				p += 7;
				p += sp_svg_number_write_de (c + p, transform->c[0], 6, -8, FALSE);
				c[p++] = ',';
				p += sp_svg_number_write_de (c + p, transform->c[1], 6, -8, FALSE);
				c[p++] = ',';
				p += sp_svg_number_write_de (c + p, transform->c[2], 6, -8, FALSE);
				c[p++] = ',';
				p += sp_svg_number_write_de (c + p, transform->c[3], 6, -8, FALSE);
				c[p++] = ',';
				p += sp_svg_number_write_de (c + p, transform->c[4], 6, -8, FALSE);
				c[p++] = ',';
				p += sp_svg_number_write_de (c + p, transform->c[5], 6, -8, FALSE);
				c[p++] = ')';
				g_assert( p <= sizeof(c) );
				p = MIN(p, size - 1);
				memcpy (str, c, p);
				str[p] = 0;
				return p;
			}
		}
	} else {
		gchar c[256];
		unsigned p = 0;
		strcpy (c + p, "matrix(");
		p += 7;
		p += sp_svg_number_write_de (c + p, transform->c[0], 6, -8, FALSE);
		c[p++] = ',';
		p += sp_svg_number_write_de (c + p, transform->c[1], 6, -8, FALSE);
		c[p++] = ',';
		p += sp_svg_number_write_de (c + p, transform->c[2], 6, -8, FALSE);
		c[p++] = ',';
		p += sp_svg_number_write_de (c + p, transform->c[3], 6, -8, FALSE);
		c[p++] = ',';
		p += sp_svg_number_write_de (c + p, transform->c[4], 6, -8, FALSE);
		c[p++] = ',';
		p += sp_svg_number_write_de (c + p, transform->c[5], 6, -8, FALSE);
		c[p++] = ')';
		g_assert( p <= sizeof(c) );
		p = MIN(p, size - 1);
		memcpy (str, c, p);
		str[p] = 0;
		return p;
	}
}

