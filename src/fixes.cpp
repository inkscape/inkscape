/*
 *
 * This is the header file to include to fix any broken definitions or funcs
 *
 * $Id$
 *
 * 2004 Kees Cook
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * http://www.gnu.org/copyleft/gpl.html
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

//#if defined(g_ascii_strtod)
#if 0
/*
 * until 2004-04-22, g_ascii_strtod could not handle having a locale-based
 * decimal separator immediately following the number ("5,4" would
 * parse to "5,4" instead of "5.0" in fr_FR)
 *
 * This is the corrected function, lifted from 1.107 gstrfuncs.c in glib
 */
extern "C" {
#include <glib.h>
#include <locale.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

gdouble
fixed_g_ascii_strtod (const gchar *nptr,
		gchar      **endptr)
{
  gchar *fail_pos;
  gdouble val;
  struct lconv *locale_data;
  const char *decimal_point;
  int decimal_point_len;
  const char *p, *decimal_point_pos;
  const char *end = NULL; /* Silence gcc */

  g_return_val_if_fail (nptr != NULL, 0);

  fail_pos = NULL;

  locale_data = localeconv ();
  decimal_point = locale_data->decimal_point;
  decimal_point_len = strlen (decimal_point);

  g_assert (decimal_point_len != 0);

  decimal_point_pos = NULL;
  if (decimal_point[0] != '.' ||
      decimal_point[1] != 0)
    {
      p = nptr;
      /* Skip leading space */
      while (g_ascii_isspace (*p))
	p++;

      /* Skip leading optional sign */
      if (*p == '+' || *p == '-')
	p++;

      if (p[0] == '0' &&
	  (p[1] == 'x' || p[1] == 'X'))
	{
	  p += 2;
	  /* HEX - find the (optional) decimal point */
	
	  while (g_ascii_isxdigit (*p))
	    p++;
	
	  if (*p == '.')
	    {
	      decimal_point_pos = p++;
	
	      while (g_ascii_isxdigit (*p))
		p++;
	
	      if (*p == 'p' || *p == 'P')
		p++;
	      if (*p == '+' || *p == '-')
		p++;
	      while (g_ascii_isdigit (*p))
		p++;
	    }
	}
      else
	{
	  while (g_ascii_isdigit (*p))
	    p++;
	
	  if (*p == '.')
	    {
	      decimal_point_pos = p++;
	
	      while (g_ascii_isdigit (*p))
		p++;
	
	      if (*p == 'e' || *p == 'E')
		p++;
	      if (*p == '+' || *p == '-')
		p++;
	      while (g_ascii_isdigit (*p))
		p++;
	    }
	}
      /* For the other cases, we need not convert the decimal point */
      end = p;
    }

  /* Set errno to zero, so that we can distinguish zero results
     and underflows */
  errno = 0;

  if (decimal_point_pos)
    {
      char *copy, *c;

      /* We need to convert the '.' to the locale specific decimal point */
      copy = (char*)g_malloc (end - nptr + 1 + decimal_point_len);

      c = copy;
      memcpy (c, nptr, decimal_point_pos - nptr);
      c += decimal_point_pos - nptr;
      memcpy (c, decimal_point, decimal_point_len);
      c += decimal_point_len;
      memcpy (c, decimal_point_pos + 1, end - (decimal_point_pos + 1));
      c += end - (decimal_point_pos + 1);
      *c = 0;

      val = strtod (copy, &fail_pos);

      if (fail_pos)
	{
	  if (fail_pos - copy > decimal_point_pos - nptr)
	    fail_pos = (char *)nptr + (fail_pos - copy) - (decimal_point_len - 1);
	  else
	    fail_pos = (char *)nptr + (fail_pos - copy);
	}

      g_free (copy);
	
    }
  else if (decimal_point[0] != '.' ||
	   decimal_point[1] != 0)
    {
      char *copy;

      copy = (char*)g_malloc (end - (char *)nptr + 1);
      memcpy (copy, nptr, end - nptr);
      *(copy + (end - (char *)nptr)) = 0;

      val = strtod (copy, &fail_pos);

      if (fail_pos)
	{
	  fail_pos = (char *)nptr + (fail_pos - copy);
	}

      g_free (copy);
    }
  else
    {
      val = strtod (nptr, &fail_pos);
    }

  if (endptr)
    *endptr = fail_pos;

  return val;
}
}

#endif /* BROKEN_G_ASCII_STRTOD */


