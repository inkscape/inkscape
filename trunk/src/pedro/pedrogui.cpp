/*
 * Simple demo GUI for the Pedro mini-XMPP client.
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2005-2007 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "pedrogui.h"

#include <stdarg.h>

namespace Pedro
{



//#########################################################################
//# I C O N S
//#########################################################################

static const guint8 icon_available[] =
{ ""
  /* Pixbuf magic (0x47646b50) */
  "GdkP"
  /* length: header (24) + pixel_data (672) */
  "\0\0\2\270"
  /* pixdata_type (0x1010002) */
  "\1\1\0\2"
  /* rowstride (48) */
  "\0\0\0""0"
  /* width (12) */
  "\0\0\0\14"
  /* height (14) */
  "\0\0\0\16"
  /* pixel_data: */
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0""333\377333"
  "\377333\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0""333\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377333\377\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0""333\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\37733"
  "3\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0""333\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377333\377\377\377\377\0\377\377\377\0\377\377"
  "\377\0""333\377\377\377\0\377\377\377\0\377\0\0\0\377\377\377\0\377\0"
  "\0\0\377\377\377\0\377\377\377\0\377333\377\377\377\377\0\377\377\377"
  "\0\377\377\377\0""333\377\377\377\0\377\377\377\0\377\0\0\0\377\377\377"
  "\0\377\0\0\0\377\377\377\0\377\377\377\0\377333\377\377\377\377\0\377"
  "\377\377\0\377\377\377\0""333\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\0\0\0\377\377\377\0\377\377\377\0\377\377\377\0\377333\377\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0""333\377\377\377"
  "\0\377\377\377\0\377\0\0\0\377\377\377\0\377\377\377\0\377333\377\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0""3"
  "33\377\377\377\0\377\377\377\0\377\0\0\0\377\377\377\0\377\377\377\0"
  "\377333\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0""333\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377333\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0LLL\377\0\0\0\377\0\0\0\377"
  "\0\0\0\377LLL\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0LLL\377\0\0\0\377\0\0\0\377"
  "\0\0\0\377LLL\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\0\0\0\377"
  "\0\0\0\377\0\0\0\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0"};


static const guint8 icon_away[] =
{ ""
  /* Pixbuf magic (0x47646b50) */
  "GdkP"
  /* length: header (24) + pixel_data (672) */
  "\0\0\2\270"
  /* pixdata_type (0x1010002) */
  "\1\1\0\2"
  /* rowstride (48) */
  "\0\0\0""0"
  /* width (12) */
  "\0\0\0\14"
  /* height (14) */
  "\0\0\0\16"
  /* pixel_data: */
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0""333\377333\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0""333\377\377\377\0\377\377\377\0\377333\377\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0""333\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377333\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\0\0\377\377\0\0\377\377\0\0\377\377\0\0\377\377\0"
  "\0\377\377\0\0\377\377\0\0\377\377\0\0\377\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\0\0\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\0\0\377\377\377\377\0\377\0\0\377\377\377\377"
  "\377\0\0\0\377\377\377\377\377\377\377\377\377\0\0\0\377\377\377\377"
  "\377\0\0\0\377\0\0\0\377\377\377\377\377\377\377\377\377\377\0\0\377"
  "\377\0\0\377\377\377\377\377\0\0\0\377\0\0\0\377\377\377\377\377\0\0"
  "\0\377\0\0\0\377\377\377\377\377\377\377\377\377\0\0\0\377\377\377\377"
  "\377\377\0\0\377\377\0\0\377\377\377\377\377\0\0\0\377\377\377\377\377"
  "\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\0\0\0\377\377\377"
  "\377\377\377\0\0\377\377\0\0\377\377\377\377\377\0\0\0\377\377\377\377"
  "\377\377\377\377\377\0\0\0\377\0\0\0\377\377\377\377\377\377\377\377"
  "\377\0\0\0\377\377\377\377\377\377\0\0\377\377\377\377\0\377\0\0\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\0\0"
  "\377\377\377\377\0\377\377\377\0\377\377\377\0\377\0\0\377\377\0\0\377"
  "\377\0\0\377\377\0\0\377\377\0\0\377\377\0\0\377\377\0\0\377\377\0\0"
  "\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0LLL\377333\377\0\0\0\377\0\0\0\377LLL\377\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0""333\377333\377333\377\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0"};


static const guint8 icon_chat[] =
{ ""
  /* Pixbuf magic (0x47646b50) */
  "GdkP"
  /* length: header (24) + pixel_data (672) */
  "\0\0\2\270"
  /* pixdata_type (0x1010002) */
  "\1\1\0\2"
  /* rowstride (48) */
  "\0\0\0""0"
  /* width (12) */
  "\0\0\0\14"
  /* height (14) */
  "\0\0\0\16"
  /* pixel_data: */
  "\377\377\377\0\377\377\377\0\377\377\377\0""333\377333\377333\377\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0""333\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377333\377\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0""333\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377333\377\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0""333\377\377\377\0\377\377"
  "\377\0\377fff\377\377\377\0\377fff\377\377\377\0\377\377\377\0\37733"
  "3\377\377\377\377\0\377\377\377\0\377\377\377\0""333\377\377\377\0\377"
  "fff\377\377\377\0\377fff\377\377\377\0\377fff\377\377\377\0\377333\377"
  "\377\377\377\0\377\377\377\0\377\377\377\0""333\377\377\377\0\377fff"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\0\0\0\377\0\0\0\377\0"
  "\0\0\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0""33"
  "3\377\377\377\0\377fff\377\377\377\0\377\0\0\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\0\0\0\377\377\377\377\0\377\377\377\0\377\377"
  "\377\0""333\377\377\377\0\377fff\377\0\0\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\0\0\0\377\377\377\377"
  "\0\377\377\377\0\377\377\377\0""333\377\0\0\0\377\377\377\0\377\377\377"
  "\0\377\0\0\0\377\377\377\0\377\0\0\0\377\377\377\0\377\377\377\0\377"
  "\0\0\0\377\377\377\377\0\377\377\377\0""333\377\0\0\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\0\0\0\377\377\377\377\0\377\377\377\0LLL\377\0\0\0\377"
  "\377\377\0\377\377\377\0\377\0\0\0\377\377\377\0\377\0\0\0\377\377\377"
  "\0\377\377\377\0\377\0\0\0\377\377\377\377\0\377\377\377\0LLL\377333"
  "\377\0\0\0\377\377\377\0\377\377\377\0\377\0\0\0\377\377\377\0\377\377"
  "\377\0\377\0\0\0\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0""333\377333\377\0\0\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\0\0\0\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\0\0\0\377"
  "\0\0\0\377\0\0\0\377\377\377\377\0\377\377\377\0\377\377\377\0"};


static const guint8 icon_dnd[] =
{ ""
  /* Pixbuf magic (0x47646b50) */
  "GdkP"
  /* length: header (24) + pixel_data (672) */
  "\0\0\2\270"
  /* pixdata_type (0x1010002) */
  "\1\1\0\2"
  /* rowstride (48) */
  "\0\0\0""0"
  /* width (12) */
  "\0\0\0\14"
  /* height (14) */
  "\0\0\0\16"
  /* pixel_data: */
  "\377\377\377\0\377\377\377\0\377\377\377\0""333\377333\377333\377\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0""333\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377333\377\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0""333\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377333\377\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0""333\377\377\377\0\377\377"
  "\377\0\377fff\377\377\377\0\377fff\377\377\377\0\377\377\377\0\37733"
  "3\377\377\377\377\0\377\377\377\0\377\377\377\0""333\377\377\377\0\377"
  "fff\377\377\377\0\377fff\377\377\377\0\377fff\377\377\377\0\377333\377"
  "\377\377\377\0\377\377\377\0\377\377\377\0""333\377\377\377\0\377fff"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\177\0\0\377\177\0\0\377"
  "\177\0\0\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
  "333\377\377\377\0\377fff\377\377\377\0\377\177\0\0\377\377\0\0\377\377"
  "\0\0\377\377\0\0\377\177\0\0\377\377\377\377\0\377\377\377\0\377\377"
  "\377\0""333\377\377\377\0\377fff\377\177\0\0\377\377\377\377\377fff\377"
  "\377\0\0\377fff\377\377\377\377\377\177\0\0\377\377\377\377\0\377\377"
  "\377\0\377\377\377\0""333\377\177\0\0\377\377\0\0\377fff\377\377\377"
  "\377\377fff\377\377\377\377\377fff\377\377\0\0\377\177\0\0\377\377\377"
  "\377\0\377\377\377\0""333\377\177\0\0\377\377\0\0\377\377\0\0\377fff"
  "\377\377\377\377\377fff\377\377\0\0\377\377\0\0\377\177\0\0\377\377\377"
  "\377\0\377\377\377\0LLL\377\177\0\0\377\377\0\0\377fff\377\377\377\377"
  "\377fff\377\377\377\377\377fff\377\377\0\0\377\177\0\0\377\377\377\377"
  "\0\377\377\377\0LLL\377333\377\177\0\0\377\377\377\377\377fff\377\377"
  "\0\0\377fff\377\377\377\377\377\177\0\0\377\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0""333\377333\377\177\0\0\377\377\0\0\377"
  "\377\0\0\377\377\0\0\377\177\0\0\377\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\177\0\0\377\177\0\0\377\177\0\0\377\377\377\377\0\377\377"
  "\377\0\377\377\377\0"};


static const guint8 icon_error[] =
{ ""
  /* Pixbuf magic (0x47646b50) */
  "GdkP"
  /* length: header (24) + pixel_data (672) */
  "\0\0\2\270"
  /* pixdata_type (0x1010002) */
  "\1\1\0\2"
  /* rowstride (48) */
  "\0\0\0""0"
  /* width (12) */
  "\0\0\0\14"
  /* height (14) */
  "\0\0\0\16"
  /* pixel_data: */
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0""333\377333\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0""333\377"
  "\377\377\377\0\0\0\0\377\350\350\350\377333\377\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0""333\377"
  "\350\350\350\377fff\377\0\0\0\377\350\350\350\377\350\350\350\377333"
  "\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0""333\377"
  "\350\350\350\377\350\350\350\377\350\350\350\377\0\0\0\377\350\350\350"
  "\377\350\350\350\377\350\350\350\377333\377\377\377\377\0\377\377\377"
  "\0\377\377\377\0""333\377\350\350\350\377\350\350\350\377\0\0\0\377\0"
  "\0\0\377fff\377\350\350\350\377\350\350\350\377333\377\377\377\377\0"
  "\377\377\377\0\377\377\377\0""333\377\350\350\350\377\350\350\350\377"
  "\0\0\0\377\350\350\350\377\0\0\0\377\0\0\0\377\0\0\0\377333\377\377\377"
  "\377\0\377\377\377\0\377\377\377\0""333\377\350\350\350\377\0\0\0\377"
  "\350\350\350\377\0\0\0\377\350\350\350\377\350\350\350\377\350\350\350"
  "\377333\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0""3"
  "33\377\350\350\350\377\0\0\0\377\0\0\0\377\0\0\0\377\350\350\350\377"
  "fff\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0""333\377\0\0\0\377\350\350\350\377\350\350\350\377\350\350\350"
  "\377\0\0\0\377333\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0""333\377\350\350\350\377\350\350\350"
  "\377\350\350\350\377333\377\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0LLL\377\0\0\0"
  "\377\0\0\0\377\0\0\0\377LLL\377\377\377\377\0\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0LLL\377\0\0"
  "\0\377\0\0\0\377\0\0\0\377LLL\377\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\0\0\0\377\0\0\0\377\0\0\0\377\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"};


static const guint8 icon_offline[] =
{ ""
  /* Pixbuf magic (0x47646b50) */
  "GdkP"
  /* length: header (24) + pixel_data (672) */
  "\0\0\2\270"
  /* pixdata_type (0x1010002) */
  "\1\1\0\2"
  /* rowstride (48) */
  "\0\0\0""0"
  /* width (12) */
  "\0\0\0\14"
  /* height (14) */
  "\0\0\0\16"
  /* pixel_data: */
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0""333\377333"
  "\377333\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0""333\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377333\377\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0""333\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377333\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0""333\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377333\377\377\377"
  "\377\0\377\377\377\0\377\377\377\0""333\377\377\377\377\377\377\377\377"
  "\377\0\0\0\377\377\377\377\377\0\0\0\377\377\377\377\377\377\377\377"
  "\377333\377\377\377\377\0\377\377\377\0\377\377\377\0""333\377\377\377"
  "\377\377\377\377\377\377\0\0\0\377\377\377\377\377\0\0\0\377\377\377"
  "\377\377\377\377\377\377333\377\377\377\377\0\377\377\377\0\377\377\377"
  "\0""333\377\377\377\377\377\377\377\377\377\377\377\377\377\0\0\0\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377333\377\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0""333\377\377\377\377\377\377"
  "\377\377\377\0\0\0\377\377\377\377\377\377\377\377\377333\377\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0""333\377"
  "\377\377\377\377\377\377\377\377\0\0\0\377\377\377\377\377\377\377\377"
  "\377333\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0""333\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377333\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0LLL\377\0\0\0\377\0\0"
  "\0\377\0\0\0\377LLL\377\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0LLL\377\0\0\0\377"
  "\0\0\0\377\0\0\0\377LLL\377\377\377\377\0\377\377\377\0\377\377\377\0"
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
  "\0\0\0\0\377\0\0\0\377\0\0\0\377\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"};


static const guint8 icon_xa[] =
{ ""
  /* Pixbuf magic (0x47646b50) */
  "GdkP"
  /* length: header (24) + pixel_data (672) */
  "\0\0\2\270"
  /* pixdata_type (0x1010002) */
  "\1\1\0\2"
  /* rowstride (48) */
  "\0\0\0""0"
  /* width (12) */
  "\0\0\0\14"
  /* height (14) */
  "\0\0\0\16"
  /* pixel_data: */
  "\377\377\377\0\377\377\377\0\377\377\377\0\377\0\0\377333\377333\377"
  "333\377\377\0\0\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\0\0\377333\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377333\377\377\0\0\377\377\377\377\0\377\377\377"
  "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\0\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\0\0\377"
  "\177\0\0\377\177\0\0\377\377\377\377\0\377\377\377\0""333\377\377\0\0"
  "\377\377\377\0\377fff\377\377\377\0\377\177\0\0\377\177\0\0\377\377\0"
  "\0\377\377\377\377\377\177\0\0\377\377\377\377\0\377\377\377\0""333\377"
  "\377\0\0\377\177\0\0\377\177\0\0\377\177\0\0\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\0\0\0\377\0\0\0\377\177\0\0\377\177\0\0"
  "\377\177\0\0\377\377\0\0\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\262\262\262\377\377\377\377\377\0\0\0\377\0\0\0\377\377\377"
  "\377\377\177\0\0\377\177\0\0\377\377\377\377\377\262\262\262\377\0\0"
  "\0\377\262\262\262\377\0\0\0\377\377\377\377\377\0\0\0\377\377\377\377"
  "\377\262\262\262\377\0\0\0\377\177\0\0\377\177\0\0\377\377\377\377\377"
  "\0\0\0\377\377\377\377\377\0\0\0\377\0\0\0\377\377\377\377\377\0\0\0"
  "\377\0\0\0\377\377\377\377\377\0\0\0\377\177\0\0\377\177\0\0\377\377"
  "\377\377\377\0\0\0\377\377\377\377\377\0\0\0\377\377\377\377\377\0\0"
  "\0\377\0\0\0\377\377\377\377\377\377\377\377\377\377\377\377\377\177"
  "\0\0\377\377\377\377\0\177\0\0\377\262\262\262\377\0\0\0\377\262\262"
  "\262\377\377\377\377\377\377\377\377\377\377\377\377\377\177\0\0\377"
  "\177\0\0\377\177\0\0\377\377\377\377\0\377\377\377\0\177\0\0\377\377"
  "\377\377\377\377\377\377\377\177\0\0\377\177\0\0\377\177\0\0\377\177"
  "\0\0\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\177\0\0\377\177\0\0\377\177\0\0\377333\377\0\0\0\377\0\0\0"
  "\377LLL\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
  "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0""333\377333\377"
  "333\377\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
  "\377\0\377\377\377\0\377\377\377\0"};


//#########################################################################
//# R O S T E R
//#########################################################################


void Roster::doubleClickCallback(const Gtk::TreeModel::Path &path,
                   Gtk::TreeViewColumn *col)
{
    Glib::RefPtr<Gtk::TreeModel> model = rosterView.get_model();
    Glib::RefPtr<Gtk::TreeSelection> sel = rosterView.get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    DOMString nick = iter->get_value(rosterColumns.userColumn);
    //printf("Double clicked:%s\n", nick.c_str());
    if (parent)
        parent->doChat(nick);

}

void Roster::chatCallback()
{
    Glib::RefPtr<Gtk::TreeModel> model = rosterView.get_model();
    Glib::RefPtr<Gtk::TreeSelection> sel = rosterView.get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    DOMString nick = iter->get_value(rosterColumns.userColumn);
    //printf("Chat with:%s\n", nick.c_str());
    if (parent)
        parent->doChat(nick);
}

void Roster::sendFileCallback()
{
    Glib::RefPtr<Gtk::TreeModel> model = rosterView.get_model();
    Glib::RefPtr<Gtk::TreeSelection> sel = rosterView.get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    DOMString nick = iter->get_value(rosterColumns.userColumn);
    //printf("Send file to:%s\n", nick.c_str());
    if (parent)
        parent->doSendFile(nick);
}

bool Roster::buttonPressCallback(GdkEventButton* event)
{
    if( (event->type == GDK_BUTTON_PRESS) && (event->button == 3) )
        {
        Gtk::Widget *wid = uiManager->get_widget("/PopupMenu");
        Gtk::Menu *popupMenu = dynamic_cast<Gtk::Menu*>(wid);
        popupMenu->popup(event->button, event->time);
        return true;
        }
    else
        return false;
}

bool Roster::doSetup()
{
    set_size_request(200,200);

    pixbuf_available = Gdk::Pixbuf::create_from_inline(
                      sizeof(icon_available), icon_available, false);
    pixbuf_away     = Gdk::Pixbuf::create_from_inline(
                      sizeof(icon_away), icon_away, false);
    pixbuf_chat     = Gdk::Pixbuf::create_from_inline(
                      sizeof(icon_chat), icon_chat, false);
    pixbuf_dnd      = Gdk::Pixbuf::create_from_inline(
                      sizeof(icon_dnd), icon_dnd, false);
    pixbuf_error    = Gdk::Pixbuf::create_from_inline(
                      sizeof(icon_error), icon_error, false);
    pixbuf_offline  = Gdk::Pixbuf::create_from_inline(
                      sizeof(icon_offline), icon_offline, false);
    pixbuf_xa       = Gdk::Pixbuf::create_from_inline(
                      sizeof(icon_xa), icon_xa, false);

    rosterView.setParent(this);
    treeStore = Gtk::TreeStore::create(rosterColumns);
    rosterView.set_model(treeStore);

    Gtk::CellRendererText *rend0 = new Gtk::CellRendererText();
    //rend0->property_background() = "gray";
    //rend0->property_foreground() = "black";
    rosterView.append_column("Group", *rend0);
    Gtk::TreeViewColumn *col0 = rosterView.get_column(0);
    col0->add_attribute(*rend0, "text", 0);

    Gtk::CellRendererPixbuf *rend1 = new Gtk::CellRendererPixbuf();
    rosterView.append_column("Status", *rend1);
    Gtk::TreeViewColumn *col1 = rosterView.get_column(1);
    col1->add_attribute(*rend1, "pixbuf", 1);

    Gtk::CellRendererText *rend2 = new Gtk::CellRendererText();
    rosterView.append_column("Item", *rend2);
    Gtk::TreeViewColumn *col2 = rosterView.get_column(2);
    col2->add_attribute(*rend2, "text", 2);

    Gtk::CellRendererText *rend3 = new Gtk::CellRendererText();
    rosterView.append_column("Name", *rend3);
    Gtk::TreeViewColumn *col3 = rosterView.get_column(3);
    col3->add_attribute(*rend3, "text", 3);

    Gtk::CellRendererText *rend4 = new Gtk::CellRendererText();
    rosterView.append_column("Subscription", *rend4);
    Gtk::TreeViewColumn *col4 = rosterView.get_column(4);
    col4->add_attribute(*rend4, "text", 4);

    rosterView.signal_row_activated().connect(
        sigc::mem_fun(*this, &Roster::doubleClickCallback) );

    add(rosterView);
    set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);

    //##### POPUP MENU
    Glib::RefPtr<Gtk::ActionGroup> actionGroup = Gtk::ActionGroup::create();

    actionGroup->add( Gtk::Action::create("UserMenu", "_User Menu") );

    actionGroup->add( Gtk::Action::create("Chat",
                                  Gtk::Stock::CONNECT, "Chat"),
    sigc::mem_fun(*this, &Roster::chatCallback) );
    actionGroup->add( Gtk::Action::create("SendFile",
                                  Gtk::Stock::CONNECT, "Send file"),
    sigc::mem_fun(*this, &Roster::sendFileCallback) );


    uiManager = Gtk::UIManager::create();

    uiManager->insert_action_group(actionGroup, 0);

    Glib::ustring ui_info =
        "<ui>"
        "  <popup name='PopupMenu'>"
        "    <menuitem action='Chat'/>"
        "    <menuitem action='SendFile'/>"
        "  </popup>"
        "</ui>";

    uiManager->add_ui_from_string(ui_info);


    show_all_children();

    return true;
}


/**
 * Clear the roster
 */
void Roster::clear()
{
    treeStore->clear();
}

/**
 * Regenerate the roster
 */
void Roster::refresh()
{
   if (!parent)
        return;
    treeStore->clear();
    std::vector<XmppUser> items = parent->client.getRoster();

    //## Add in tree fashion
    DOMString lastGroup = "";
    Gtk::TreeModel::Row row = *(treeStore->append());
    row[rosterColumns.groupColumn] = "";
    for (unsigned int i=0 ; i<items.size() ; i++)
        {
        XmppUser user = items[i];
        if (user.group != lastGroup)
            {
            if (lastGroup.size()>0)
                row = *(treeStore->append());
            row[rosterColumns.groupColumn] = user.group;
            lastGroup = user.group;
            }
        Glib::RefPtr<Gdk::Pixbuf> pb = pixbuf_offline;
        if (user.show == "available")
            pb = pixbuf_available;
        else if (user.show == "away")
            pb = pixbuf_away;
        else if (user.show == "chat")
            pb = pixbuf_chat;
        else if (user.show == "dnd")
            pb = pixbuf_dnd;
        else if (user.show == "xa")
            pb = pixbuf_xa;
        else
            {
            //printf("Unknown show for %s:'%s'\n", user.c_str(), show.c_str());
            }
        Gtk::TreeModel::Row childRow = *(treeStore->append(row.children()));
        childRow[rosterColumns.statusColumn] = pb;
        childRow[rosterColumns.userColumn]   = user.jid;
        childRow[rosterColumns.nameColumn]   = user.nick;
        childRow[rosterColumns.subColumn]    = user.subscription;
        }
    rosterView.expand_all();
}

//#########################################################################
//# M E S S A G E    L I S T
//#########################################################################

bool MessageList::doSetup()
{
    set_size_request(400,200);

    messageListBuffer = Gtk::TextBuffer::create();
    messageList.set_buffer(messageListBuffer);
    messageList.set_editable(false);
    messageList.set_wrap_mode(Gtk::WRAP_WORD_CHAR);

    Glib::RefPtr<Gtk::TextBuffer::TagTable> table =
        messageListBuffer->get_tag_table();
    Glib::RefPtr<Gtk::TextBuffer::Tag> color0 =
        Gtk::TextBuffer::Tag::create("color0");
    color0->property_foreground() = "DarkGreen";
    color0->property_weight()     = Pango::WEIGHT_BOLD;
    table->add(color0);
    Glib::RefPtr<Gtk::TextBuffer::Tag> color1 =
        Gtk::TextBuffer::Tag::create("color1");
    color1->property_foreground() = "chocolate4";
    color1->property_weight()     = Pango::WEIGHT_BOLD;
    table->add(color1);
    Glib::RefPtr<Gtk::TextBuffer::Tag> color2 =
        Gtk::TextBuffer::Tag::create("color2");
    color2->property_foreground() = "red4";
    color2->property_weight()     = Pango::WEIGHT_BOLD;
    table->add(color2);
    Glib::RefPtr<Gtk::TextBuffer::Tag> color3 =
        Gtk::TextBuffer::Tag::create("color3");
    color3->property_foreground() = "MidnightBlue";
    color3->property_weight()     = Pango::WEIGHT_BOLD;
    table->add(color3);
    Glib::RefPtr<Gtk::TextBuffer::Tag> color4 =
        Gtk::TextBuffer::Tag::create("color4");
    color4->property_foreground() = "turquoise4";
    color4->property_weight()     = Pango::WEIGHT_BOLD;
    table->add(color4);
    Glib::RefPtr<Gtk::TextBuffer::Tag> color5 =
        Gtk::TextBuffer::Tag::create("color5");
    color5->property_foreground() = "OliveDrab";
    color5->property_weight()     = Pango::WEIGHT_BOLD;
    table->add(color5);
    Glib::RefPtr<Gtk::TextBuffer::Tag> color6 =
        Gtk::TextBuffer::Tag::create("color6");
    color6->property_foreground() = "purple4";
    color6->property_weight()     = Pango::WEIGHT_BOLD;
    table->add(color6);
    Glib::RefPtr<Gtk::TextBuffer::Tag> color7 =
        Gtk::TextBuffer::Tag::create("color7");
    color7->property_foreground() = "VioletRed4";
    color7->property_weight()     = Pango::WEIGHT_BOLD;
    table->add(color7);

    add(messageList);
    set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);


    show_all_children();

    return true;
}

/**
 * Clear all messages from the list
 */
void MessageList::clear()
{
    messageListBuffer->erase(messageListBuffer->begin(),
                             messageListBuffer->end());
}


/**
 * Post a message to the list
 */
void MessageList::postMessage(const DOMString &from, const DOMString &msg)
{
    DOMString out = "<";
    out.append(from);
    out.append("> ");

    int val = 0;
    for (unsigned int i=0 ; i<from.size() ; i++)
       val += from[i];
    val = val % 8;

    char buf[16];
    sprintf(buf, "color%d", val);
    DOMString tagName = buf;

    messageListBuffer->insert_with_tag(
            messageListBuffer->end(), out, tagName);
    messageListBuffer->insert(messageListBuffer->end(), msg);
    messageListBuffer->insert(messageListBuffer->end(), "\n");
    //Gtk::Adjustment *adj = get_vadjustment();
    //adj->set_value(adj->get_upper()-adj->get_page_size());
    Glib::RefPtr<Gtk::TextBuffer::Mark> mark =
        messageListBuffer->create_mark("temp", messageListBuffer->end());
    messageList.scroll_mark_onscreen(mark);
    messageListBuffer->delete_mark(mark);
}



//#########################################################################
//# U S E R     L I S T
//#########################################################################
void UserList::doubleClickCallback(const Gtk::TreeModel::Path &path,
                   Gtk::TreeViewColumn *col)
{
    Glib::RefPtr<Gtk::TreeModel> model = userList.get_model();
    Glib::RefPtr<Gtk::TreeSelection> sel = userList.get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    DOMString nick = iter->get_value(userListColumns.userColumn);
    //printf("Double clicked:%s\n", nick.c_str());
    if (parent)
        parent->doChat(nick);

}

void UserList::chatCallback()
{
    Glib::RefPtr<Gtk::TreeModel> model = userList.get_model();
    Glib::RefPtr<Gtk::TreeSelection> sel = userList.get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    DOMString nick = iter->get_value(userListColumns.userColumn);
    //printf("Chat with:%s\n", nick.c_str());
    if (parent)
        parent->doChat(nick);
}

void UserList::sendFileCallback()
{
    Glib::RefPtr<Gtk::TreeModel> model = userList.get_model();
    Glib::RefPtr<Gtk::TreeSelection> sel = userList.get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    DOMString nick = iter->get_value(userListColumns.userColumn);
    //printf("Send file to:%s\n", nick.c_str());
    if (parent)
        parent->doSendFile(nick);
}

bool UserList::buttonPressCallback(GdkEventButton* event)
{
    if( (event->type == GDK_BUTTON_PRESS) && (event->button == 3) )
        {
        Gtk::Widget *wid = uiManager->get_widget("/PopupMenu");
        Gtk::Menu *popupMenu = dynamic_cast<Gtk::Menu*>(wid);
        popupMenu->popup(event->button, event->time);
        return true;
        }
    else
        return false;
}

bool UserList::doSetup()
{
    set_size_request(200,200);

    setParent(NULL);

    pixbuf_available = Gdk::Pixbuf::create_from_inline(
                      sizeof(icon_available), icon_available, false);
    pixbuf_away     = Gdk::Pixbuf::create_from_inline(
                      sizeof(icon_away), icon_away, false);
    pixbuf_chat     = Gdk::Pixbuf::create_from_inline(
                      sizeof(icon_chat), icon_chat, false);
    pixbuf_dnd      = Gdk::Pixbuf::create_from_inline(
                      sizeof(icon_dnd), icon_dnd, false);
    pixbuf_error    = Gdk::Pixbuf::create_from_inline(
                      sizeof(icon_error), icon_error, false);
    pixbuf_offline  = Gdk::Pixbuf::create_from_inline(
                      sizeof(icon_offline), icon_offline, false);
    pixbuf_xa       = Gdk::Pixbuf::create_from_inline(
                      sizeof(icon_xa), icon_xa, false);

    userList.setParent(this);
    userListStore = Gtk::ListStore::create(userListColumns);
    userList.set_model(userListStore);

    Gtk::CellRendererPixbuf *rend0 = new Gtk::CellRendererPixbuf();
    userList.append_column("Status", *rend0);
    Gtk::TreeViewColumn *col0 = userList.get_column(0);
    col0->add_attribute(*rend0, "pixbuf", 0);

    Gtk::CellRendererText *rend1 = new Gtk::CellRendererText();
    //rend1->property_background() = "gray";
    //rend1->property_foreground() = "black";
    userList.append_column("User", *rend1);
    Gtk::TreeViewColumn *col1 = userList.get_column(1);
    col1->add_attribute(*rend1, "text", 1);

    userList.set_headers_visible(false);

    userList.signal_row_activated().connect(
        sigc::mem_fun(*this, &UserList::doubleClickCallback) );

    add(userList);
    set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);

    //##### POPUP MENU
    Glib::RefPtr<Gtk::ActionGroup> actionGroup = Gtk::ActionGroup::create();

    actionGroup->add( Gtk::Action::create("UserMenu", "_User Menu") );

    actionGroup->add( Gtk::Action::create("Chat",
                                  Gtk::Stock::CONNECT, "Chat"),
    sigc::mem_fun(*this, &UserList::chatCallback) );
    actionGroup->add( Gtk::Action::create("SendFile",
                                  Gtk::Stock::CONNECT, "Send file"),
    sigc::mem_fun(*this, &UserList::sendFileCallback) );


    uiManager = Gtk::UIManager::create();

    uiManager->insert_action_group(actionGroup, 0);

    Glib::ustring ui_info =
        "<ui>"
        "  <popup name='PopupMenu'>"
        "    <menuitem action='Chat'/>"
        "    <menuitem action='SendFile'/>"
        "  </popup>"
        "</ui>";

    uiManager->add_ui_from_string(ui_info);

    show_all_children();

    return true;
}

/**
 * Clear all messages from the list
 */
void UserList::clear()
{
    userListStore->clear();
}


/**
 * Add a user to the list
 */
void UserList::addUser(const DOMString &user, const DOMString &show)
{
    Glib::RefPtr<Gdk::Pixbuf> pb = pixbuf_offline;
    if (show == "available")
        pb = pixbuf_available;
    else if (show == "away")
        pb = pixbuf_away;
    else if (show == "chat")
        pb = pixbuf_chat;
    else if (show == "dnd")
        pb = pixbuf_dnd;
    else if (show == "xa")
        pb = pixbuf_xa;
    else
        {
        //printf("Unknown show for %s:'%s'\n", user.c_str(), show.c_str());
        }
    Gtk::TreeModel::Row row = *(userListStore->append());
    row[userListColumns.userColumn]   = user;
    row[userListColumns.statusColumn] = pb;
}




//#########################################################################
//# C H A T    W I N D O W
//#########################################################################
ChatWindow::ChatWindow(PedroGui &par, const DOMString jidArg)
                        : parent(par)
{
    jid = jidArg;
    doSetup();
}

ChatWindow::~ChatWindow()
{
}

void ChatWindow::leaveCallback()
{
    hide();
    parent.chatDelete(jid);
}


void ChatWindow::hideCallback()
{
    hide();
    parent.chatDelete(jid);
}


void ChatWindow::textEnterCallback()
{
    DOMString str = inputTxt.get_text();
    if (str.size() > 0)
        parent.client.message(jid, str);
    inputTxt.set_text("");
    messageList.postMessage(parent.client.getJid(), str);
}

bool ChatWindow::doSetup()
{
    DOMString title = "Private Chat - ";
    title.append(jid);
    set_title(title);

    set_size_request(500,300);

    signal_hide().connect(
           sigc::mem_fun(*this, &ChatWindow::hideCallback) );

    Glib::RefPtr<Gtk::ActionGroup> actionGroup = Gtk::ActionGroup::create();
    actionGroup->add( Gtk::Action::create("MenuFile", "_File") );
    actionGroup->add( Gtk::Action::create("Leave",  Gtk::Stock::CANCEL),
        sigc::mem_fun(*this, &ChatWindow::leaveCallback) );


    Glib::RefPtr<Gtk::UIManager> uiManager = Gtk::UIManager::create();

    uiManager->insert_action_group(actionGroup, 0);
    add_accel_group(uiManager->get_accel_group());

    Glib::ustring ui_info =
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='MenuFile'>"
        "      <menuitem action='Leave'/>"
        "    </menu>"
        "  </menubar>"
        "</ui>";

    add(vbox);

    uiManager->add_ui_from_string(ui_info);
    Gtk::Widget* pMenuBar = uiManager->get_widget("/MenuBar");
    vbox.pack_start(*pMenuBar, Gtk::PACK_SHRINK);

    vbox.pack_end(vPaned);

    vPaned.add1(messageList);
    vPaned.add2(inputTxt);

    inputTxt.signal_activate().connect(
           sigc::mem_fun(*this, &ChatWindow::textEnterCallback) );

    show_all_children();

    return true;
}

bool ChatWindow::postMessage(const DOMString &data)
{
    messageList.postMessage(jid, data);
    return true;
}

//#########################################################################
//# G R O U P    C H A T    W I N D O W
//#########################################################################

GroupChatWindow::GroupChatWindow(PedroGui &par,
                                 const DOMString &groupJidArg,
                                 const DOMString &nickArg)
                               : parent(par)
{
    groupJid = groupJidArg;
    nick     = nickArg;
    doSetup();
}

GroupChatWindow::~GroupChatWindow()
{
}


void GroupChatWindow::leaveCallback()
{
    parent.client.groupChatLeave(groupJid, nick);
    hide();
    parent.groupChatDelete(groupJid, nick);
}

void GroupChatWindow::hideCallback()
{
    parent.client.groupChatLeave(groupJid, nick);
    hide();
    parent.groupChatDelete(groupJid, nick);
}

void GroupChatWindow::textEnterCallback()
{
    DOMString str = inputTxt.get_text();
    if (str.size() > 0)
        parent.client.groupChatMessage(groupJid, str);
    inputTxt.set_text("");
}

bool GroupChatWindow::doSetup()
{
    DOMString title = "Group Chat - ";
    title.append(groupJid);
    set_title(title);

    userList.setParent(this);

    set_size_request(500,300);

    signal_hide().connect(
           sigc::mem_fun(*this, &GroupChatWindow::hideCallback) );

    Glib::RefPtr<Gtk::ActionGroup> actionGroup = Gtk::ActionGroup::create();
    actionGroup->add( Gtk::Action::create("MenuFile", "_File") );
    actionGroup->add( Gtk::Action::create("Leave",  Gtk::Stock::CANCEL),
        sigc::mem_fun(*this, &GroupChatWindow::leaveCallback) );


    Glib::RefPtr<Gtk::UIManager> uiManager = Gtk::UIManager::create();

    uiManager->insert_action_group(actionGroup, 0);
    add_accel_group(uiManager->get_accel_group());

    Glib::ustring ui_info =
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='MenuFile'>"
         "      <menuitem action='Leave'/>"
        "    </menu>"
        "  </menubar>"
        "</ui>";

    add(vbox);
    uiManager->add_ui_from_string(ui_info);
    Gtk::Widget* pMenuBar = uiManager->get_widget("/MenuBar");
    vbox.pack_start(*pMenuBar, Gtk::PACK_SHRINK);

    vbox.pack_end(vPaned);

    vPaned.add1(hPaned);
    vPaned.add2(inputTxt);
    inputTxt.signal_activate().connect(
           sigc::mem_fun(*this, &GroupChatWindow::textEnterCallback) );


    hPaned.add1(messageList);
    hPaned.add2(userList);


    show_all_children();


    return true;
}

bool GroupChatWindow::receiveMessage(const DOMString &from,
                                     const DOMString &data)
{
    messageList.postMessage(from, data);
    return true;
}

bool GroupChatWindow::receivePresence(const DOMString &fromNick,
                                      bool presence,
                                      const DOMString &show,
                                      const DOMString &status)
{

    DOMString presStr = "";
    presStr.append(fromNick);
    if (!presence)
        presStr.append(" left the group");
    else
        {
        if (show.size()<1)
            presStr.append(" joined the group");
        else
            {
            presStr.append(" : ");
            presStr.append(show);
            }
        }

    if (presStr != "xa")
        messageList.postMessage("*", presStr);

    userList.clear();
    std::vector<XmppUser> memberList =
         parent.client.groupChatGetUserList(groupJid);
    for (unsigned int i=0 ; i<memberList.size() ; i++)
        {
        XmppUser user = memberList[i];
        userList.addUser(user.nick, user.show);
        }
    return true;
}


void GroupChatWindow::doChat(const DOMString &nick)
{
    printf("##Chat with %s\n", nick.c_str());
    DOMString fullJid = groupJid;
    fullJid.append("/");
    fullJid.append(nick);
    parent.doChat(fullJid);
}

void GroupChatWindow::doSendFile(const DOMString &nick)
{
    printf("##Send file to %s\n", nick.c_str());
    DOMString fullJid = groupJid;
    fullJid.append("/");
    fullJid.append(nick);
    parent.doSendFile(fullJid);

}




//#########################################################################
//# C O N F I G    D I A L O G
//#########################################################################


void ConfigDialog::okCallback()
{
    Glib::ustring pass     = passField.get_text();
    Glib::ustring newpass  = newField.get_text();
    Glib::ustring confpass = confField.get_text();
    if ((pass.size()     < 5 || pass.size()    > 12 ) ||
        (newpass.size()  < 5 || newpass.size() > 12 ) ||
        (confpass.size() < 5 || confpass.size()> 12 ))
        {
        Gtk::MessageDialog dlg(*this, "Password must be 5 to 12 characters",
            false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        dlg.run();
        }
    else if (newpass != confpass)
        {
        Gtk::MessageDialog dlg(*this, "New password and confirmation do not match",
            false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        dlg.run();
        }
    else
        {
        //response(Gtk::RESPONSE_OK);
        hide();
        }
}

void ConfigDialog::cancelCallback()
{
    //response(Gtk::RESPONSE_CANCEL);
    hide();
}

void ConfigDialog::on_response(int response_id)
{
    if (response_id == Gtk::RESPONSE_OK)
        okCallback();
    else
        cancelCallback();
}

bool ConfigDialog::doSetup()
{
    set_title("Change Password");
    set_size_request(300,200);

    Glib::RefPtr<Gtk::ActionGroup> actionGroup = Gtk::ActionGroup::create();
    actionGroup->add( Gtk::Action::create("MenuFile", "_File") );
    actionGroup->add( Gtk::Action::create("Change", Gtk::Stock::OK, "Change Password"),
        sigc::mem_fun(*this, &ConfigDialog::okCallback) );
    actionGroup->add( Gtk::Action::create("Cancel",  Gtk::Stock::CANCEL, "Cancel"),
         sigc::mem_fun(*this, &ConfigDialog::cancelCallback) );


    Glib::RefPtr<Gtk::UIManager> uiManager = Gtk::UIManager::create();

    uiManager->insert_action_group(actionGroup, 0);
    add_accel_group(uiManager->get_accel_group());

    Glib::ustring ui_info =
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='MenuFile'>"
        "      <menuitem action='Change'/>"
        "      <separator/>"
        "      <menuitem action='Cancel'/>"
        "    </menu>"
        "  </menubar>"
        "</ui>";

    uiManager->add_ui_from_string(ui_info);
    Gtk::Widget* pMenuBar = uiManager->get_widget("/MenuBar");
    get_vbox()->pack_start(*pMenuBar, Gtk::PACK_SHRINK);

    table.resize(3, 2);
    get_vbox()->pack_start(table);

    passLabel.set_text("Current Password");
    table.attach(passLabel, 0, 1, 0, 1);
    passField.set_visibility(false);
    passField.set_text(parent.client.getPassword());
    table.attach(passField, 1, 2, 0, 1);

    newLabel.set_text("New Password");
    table.attach(newLabel, 0, 1, 1, 2);
    newField.set_visibility(false);
    table.attach(newField, 1, 2, 1, 2);

    confLabel.set_text("Confirm New Password");
    table.attach(confLabel, 0, 1, 2, 3);
    confField.set_visibility(false);
    confField.signal_activate().connect(
           sigc::mem_fun(*this, &ConfigDialog::okCallback) );
    table.attach(confField, 1, 2, 2, 3);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::OK,     Gtk::RESPONSE_OK);

    show_all_children();

    return true;
}

//#########################################################################
//# P A S S W O R D      D I A L O G
//#########################################################################


void PasswordDialog::okCallback()
{
    Glib::ustring pass     = passField.get_text();
    Glib::ustring newpass  = newField.get_text();
    Glib::ustring confpass = confField.get_text();
    if ((pass.size()     < 5 || pass.size()    > 12 ) ||
        (newpass.size()  < 5 || newpass.size() > 12 ) ||
        (confpass.size() < 5 || confpass.size()> 12 ))
        {
        Gtk::MessageDialog dlg(*this, "Password must be 5 to 12 characters",
            false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        dlg.run();
        }
    else if (newpass != confpass)
        {
        Gtk::MessageDialog dlg(*this, "New password and confirmation do not match",
            false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        dlg.run();
        }
    else
        {
        //response(Gtk::RESPONSE_OK);
        hide();
        }
}

void PasswordDialog::cancelCallback()
{
    //response(Gtk::RESPONSE_CANCEL);
    hide();
}

void PasswordDialog::on_response(int response_id)
{
    if (response_id == Gtk::RESPONSE_OK)
        okCallback();
    else
        cancelCallback();
}

bool PasswordDialog::doSetup()
{
    set_title("Change Password");
    set_size_request(300,200);

    Glib::RefPtr<Gtk::ActionGroup> actionGroup = Gtk::ActionGroup::create();
    actionGroup->add( Gtk::Action::create("MenuFile", "_File") );
    actionGroup->add( Gtk::Action::create("Change", Gtk::Stock::OK, "Change Password"),
        sigc::mem_fun(*this, &PasswordDialog::okCallback) );
    actionGroup->add( Gtk::Action::create("Cancel",  Gtk::Stock::CANCEL, "Cancel"),
        sigc::mem_fun(*this, &PasswordDialog::cancelCallback) );


    Glib::RefPtr<Gtk::UIManager> uiManager = Gtk::UIManager::create();

    uiManager->insert_action_group(actionGroup, 0);
    add_accel_group(uiManager->get_accel_group());

    Glib::ustring ui_info =
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='MenuFile'>"
        "      <menuitem action='Change'/>"
        "      <separator/>"
        "      <menuitem action='Cancel'/>"
        "    </menu>"
        "  </menubar>"
        "</ui>";

    uiManager->add_ui_from_string(ui_info);
    Gtk::Widget* pMenuBar = uiManager->get_widget("/MenuBar");
    get_vbox()->pack_start(*pMenuBar, Gtk::PACK_SHRINK);

    table.resize(3, 2);
    get_vbox()->pack_start(table);

    passLabel.set_text("Current Password");
    table.attach(passLabel, 0, 1, 0, 1);
    passField.set_visibility(false);
    passField.set_text(parent.client.getPassword());
    table.attach(passField, 1, 2, 0, 1);

    newLabel.set_text("New Password");
    table.attach(newLabel, 0, 1, 1, 2);
    newField.set_visibility(false);
    table.attach(newField, 1, 2, 1, 2);

    confLabel.set_text("Confirm New Password");
    table.attach(confLabel, 0, 1, 2, 3);
    confField.set_visibility(false);
    confField.signal_activate().connect(
           sigc::mem_fun(*this, &PasswordDialog::okCallback) );
    table.attach(confField, 1, 2, 2, 3);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::OK,     Gtk::RESPONSE_OK);

    show_all_children();

    return true;
}

//#########################################################################
//# C H A T    D I A L O G
//#########################################################################


void ChatDialog::okCallback()
{
    response(Gtk::RESPONSE_OK);
    hide();
}

void ChatDialog::cancelCallback()
{
    response(Gtk::RESPONSE_CANCEL);
    hide();
}


bool ChatDialog::doSetup()
{
    set_title("Chat with User");
    set_size_request(300,200);

    Glib::RefPtr<Gtk::ActionGroup> actionGroup = Gtk::ActionGroup::create();
    actionGroup->add( Gtk::Action::create("MenuFile", "_File") );
    actionGroup->add( Gtk::Action::create("Chat", Gtk::Stock::CONNECT, "Chat"),
    sigc::mem_fun(*this, &ChatDialog::okCallback) );
    actionGroup->add( Gtk::Action::create("Cancel",  Gtk::Stock::CANCEL, "Cancel"),
    sigc::mem_fun(*this, &ChatDialog::cancelCallback) );


    Glib::RefPtr<Gtk::UIManager> uiManager = Gtk::UIManager::create();

    uiManager->insert_action_group(actionGroup, 0);
    add_accel_group(uiManager->get_accel_group());

    Glib::ustring ui_info =
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='MenuFile'>"
        "      <menuitem action='Chat'/>"
        "      <separator/>"
        "      <menuitem action='Cancel'/>"
        "    </menu>"
        "  </menubar>"
        "</ui>";

    uiManager->add_ui_from_string(ui_info);
    Gtk::Widget* pMenuBar = uiManager->get_widget("/MenuBar");
    get_vbox()->pack_start(*pMenuBar, Gtk::PACK_SHRINK);

    table.resize(2, 2);
    get_vbox()->pack_start(table);

    userLabel.set_text("User");
    table.attach(userLabel, 0, 1, 0, 1);
    //userField.set_text("");
    table.attach(userField, 1, 2, 0, 1);

    //userField.set_text("");
    table.attach(textField, 0, 2, 1, 2);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_OK);

    show_all_children();

    return true;
}

//#########################################################################
//# G R O U P    C H A T   D I A L O G
//#########################################################################


void GroupChatDialog::okCallback()
{
    response(Gtk::RESPONSE_OK);
    hide();
}

void GroupChatDialog::cancelCallback()
{
    response(Gtk::RESPONSE_CANCEL);
    hide();
}


bool GroupChatDialog::doSetup()
{
    set_title("Join Group Chat");
    set_size_request(300,200);

    Glib::RefPtr<Gtk::ActionGroup> actionGroup = Gtk::ActionGroup::create();
    actionGroup->add( Gtk::Action::create("MenuFile", "_File") );
    actionGroup->add( Gtk::Action::create("Join", Gtk::Stock::CONNECT, "Join Group"),
    sigc::mem_fun(*this, &GroupChatDialog::okCallback) );
    actionGroup->add( Gtk::Action::create("Cancel",  Gtk::Stock::CANCEL, "Cancel"),
    sigc::mem_fun(*this, &GroupChatDialog::cancelCallback) );


    Glib::RefPtr<Gtk::UIManager> uiManager = Gtk::UIManager::create();

    uiManager->insert_action_group(actionGroup, 0);
    add_accel_group(uiManager->get_accel_group());

    Glib::ustring ui_info =
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='MenuFile'>"
        "      <menuitem action='Join'/>"
        "      <separator/>"
        "      <menuitem action='Cancel'/>"
        "    </menu>"
        "  </menubar>"
        "</ui>";

    uiManager->add_ui_from_string(ui_info);
    Gtk::Widget* pMenuBar = uiManager->get_widget("/MenuBar");
    get_vbox()->pack_start(*pMenuBar, Gtk::PACK_SHRINK);

    table.resize(4, 2);
    get_vbox()->pack_start(table);

    groupLabel.set_text("Group");
    table.attach(groupLabel, 0, 1, 0, 1);
    groupField.set_text(parent.config.getMucGroup());
    table.attach(groupField, 1, 2, 0, 1);

    hostLabel.set_text("Host");
    table.attach(hostLabel, 0, 1, 1, 2);
    hostField.set_text(parent.config.getMucHost());
    table.attach(hostField, 1, 2, 1, 2);

    nickLabel.set_text("Alt Name");
    table.attach(nickLabel, 0, 1, 2, 3);
    nickField.set_text(parent.config.getMucNick());
    table.attach(nickField, 1, 2, 2, 3);

    passLabel.set_text("Password");
    table.attach(passLabel, 0, 1, 3, 4);
    passField.set_visibility(false);
    passField.set_text(parent.config.getMucPassword());
    table.attach(passField, 1, 2, 3, 4);


    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_OK);

    show_all_children();

    return true;
}




//#########################################################################
//# C O N N E C T    D I A L O G
//#########################################################################


void ConnectDialog::okCallback()
{
    response(Gtk::RESPONSE_OK);
    hide();
}

void ConnectDialog::saveCallback()
{
    Gtk::Entry txtField;
    Gtk::Dialog dlg("Account name", *this, true, true);
    dlg.get_vbox()->pack_start(txtField);
    txtField.signal_activate().connect(
           sigc::bind(sigc::mem_fun(dlg, &Gtk::Dialog::response),
              Gtk::RESPONSE_OK  ));
    dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dlg.add_button(Gtk::Stock::OK,     Gtk::RESPONSE_OK);
    dlg.show_all_children();
    int ret = dlg.run();
    if (ret != Gtk::RESPONSE_OK)
        return;

    Glib::ustring name = txtField.get_text();
    if (name.size() < 1)
        {
        parent.error("Account name too short");
        return;
        }

    if (parent.config.accountExists(name))
        {
        parent.config.accountRemove(name);          
        }

    XmppAccount account;
    account.setName(name);
    account.setHost(getHost());
    account.setPort(getPort());
    account.setUsername(getUser());
    account.setPassword(getPass());
    parent.config.accountAdd(account);

    refresh();

    parent.configSave();
}

void ConnectDialog::cancelCallback()
{
    response(Gtk::RESPONSE_CANCEL);
    hide();
}


void ConnectDialog::doubleClickCallback(
                   const Gtk::TreeModel::Path &path,
                   Gtk::TreeViewColumn *col)
{
    Glib::RefPtr<Gtk::TreeModel> model = accountView.get_model();
    Glib::RefPtr<Gtk::TreeSelection> sel = accountView.get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    DOMString name = iter->get_value(accountColumns.nameColumn);
    //printf("Double clicked:%s\n", name.c_str());
    XmppAccount account;
    if (!parent.config.accountFind(name, account))
        return;
    setHost(account.getHost()); 
    setPort(account.getPort()); 
    setUser(account.getUsername()); 
    setPass(account.getPassword()); 

    response(Gtk::RESPONSE_OK);
    hide();
}

void ConnectDialog::selectedCallback()
{
    Glib::RefPtr<Gtk::TreeModel> model = accountView.get_model();
    Glib::RefPtr<Gtk::TreeSelection> sel = accountView.get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    DOMString name = iter->get_value(accountColumns.nameColumn);
    //printf("Single clicked:%s\n", name.c_str());
    XmppAccount account;
    if (!parent.config.accountFind(name, account))
        return;
    setHost(account.getHost()); 
    setPort(account.getPort()); 
    setUser(account.getUsername()); 
    setPass(account.getPassword()); 
}

void ConnectDialog::deleteCallback()
{
    Glib::RefPtr<Gtk::TreeModel> model = accountView.get_model();
    Glib::RefPtr<Gtk::TreeSelection> sel = accountView.get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    DOMString name = iter->get_value(accountColumns.nameColumn);

    parent.config.accountRemove(name);
    refresh();
    parent.configSave();
    
}



void ConnectDialog::buttonPressCallback(GdkEventButton* event)
{
    if( (event->type == GDK_BUTTON_PRESS) && (event->button == 3) )
        {
        Gtk::Widget *wid = accountUiManager->get_widget("/PopupMenu");
        Gtk::Menu *popupMenu = dynamic_cast<Gtk::Menu*>(wid);
        popupMenu->popup(event->button, event->time);
        }
}


bool ConnectDialog::doSetup()
{
    set_title("Connect");
    set_size_request(300,400);

    Glib::RefPtr<Gtk::ActionGroup> actionGroup = Gtk::ActionGroup::create();
    actionGroup->add( Gtk::Action::create("MenuFile", "_File") );
    actionGroup->add( Gtk::Action::create("Connect",
        Gtk::Stock::CONNECT, "Connect"),
        sigc::mem_fun(*this, &ConnectDialog::okCallback) );
    actionGroup->add( Gtk::Action::create("Save",
        Gtk::Stock::CONNECT, "Save as account"),
        sigc::mem_fun(*this, &ConnectDialog::saveCallback) );
    actionGroup->add( Gtk::Action::create("Cancel",
        Gtk::Stock::CANCEL, "Cancel"),
        sigc::mem_fun(*this, &ConnectDialog::cancelCallback) );


    Glib::RefPtr<Gtk::UIManager> uiManager = Gtk::UIManager::create();

    uiManager->insert_action_group(actionGroup, 0);
    add_accel_group(uiManager->get_accel_group());

    Glib::ustring ui_info =
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='MenuFile'>"
        "      <menuitem action='Connect'/>"
        "      <separator/>"
        "      <menuitem action='Save'/>"
        "      <separator/>"
        "      <menuitem action='Cancel'/>"
        "    </menu>"
        "  </menubar>"
        "</ui>";

    uiManager->add_ui_from_string(ui_info);
    Gtk::Widget* pMenuBar = uiManager->get_widget("/MenuBar");
    get_vbox()->pack_start(*pMenuBar, Gtk::PACK_SHRINK);

    table.resize(6, 2);
    get_vbox()->pack_start(table);

    parent.client.setHost("broadway.dynalias.com");
    parent.client.setPort(5222);
    parent.client.setUsername("");
    parent.client.setPassword("");
    parent.client.setResource("pedroXmpp");

    hostLabel.set_text("Host");
    table.attach(hostLabel, 0, 1, 0, 1);
    hostField.set_text(parent.client.getHost());
    table.attach(hostField, 1, 2, 0, 1);

    portLabel.set_text("Port");
    table.attach(portLabel, 0, 1, 1, 2);
    portSpinner.set_digits(0);
    portSpinner.set_range(1, 65000);
    portSpinner.set_value(parent.client.getPort());
    table.attach(portSpinner, 1, 2, 1, 2);

    userLabel.set_text("Username");
    table.attach(userLabel, 0, 1, 2, 3);
    userField.set_text(parent.client.getUsername());
    table.attach(userField, 1, 2, 2, 3);

    passLabel.set_text("Password");
    table.attach(passLabel, 0, 1, 3, 4);
    passField.set_visibility(false);
    passField.set_text(parent.client.getPassword());
    passField.signal_activate().connect(
           sigc::mem_fun(*this, &ConnectDialog::okCallback) );
    table.attach(passField, 1, 2, 3, 4);

    resourceLabel.set_text("Resource");
    table.attach(resourceLabel, 0, 1, 4, 5);
    resourceField.set_text(parent.client.getResource());
    table.attach(resourceField, 1, 2, 4, 5);

    registerLabel.set_text("Register");
    table.attach(registerLabel, 0, 1, 5, 6);
    registerButton.set_active(false);
    table.attach(registerButton, 1, 2, 5, 6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_OK);


    //######################
    //# ACCOUNT LIST
    //######################


    accountListStore = Gtk::ListStore::create(accountColumns);
    accountView.set_model(accountListStore);

    accountView.signal_row_activated().connect(
        sigc::mem_fun(*this, &ConnectDialog::doubleClickCallback) );

    accountView.get_selection()->signal_changed().connect(
        sigc::mem_fun(*this, &ConnectDialog::selectedCallback) );

    accountView.append_column("Account", accountColumns.nameColumn);
    accountView.append_column("Host",    accountColumns.hostColumn);

    //accountView.signal_row_activated().connect(
    //    sigc::mem_fun(*this, &AccountDialog::connectCallback) );

    accountScroll.add(accountView);
    accountScroll.set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);

    get_vbox()->pack_start(accountScroll);

    //##### POPUP MENU
    accountView.signal_button_press_event().connect_notify(
        sigc::mem_fun(*this, &ConnectDialog::buttonPressCallback) );

    Glib::RefPtr<Gtk::ActionGroup> accountActionGroup =
                Gtk::ActionGroup::create();

    accountActionGroup->add( Gtk::Action::create("PopupMenu", "_Account") );

    accountActionGroup->add( Gtk::Action::create("Delete",
                                  Gtk::Stock::DELETE, "Delete"),
        sigc::mem_fun(*this, &ConnectDialog::deleteCallback) );


    accountUiManager = Gtk::UIManager::create();

    accountUiManager->insert_action_group(accountActionGroup, 0);

    Glib::ustring account_ui_info =
        "<ui>"
        "  <popup name='PopupMenu'>"
        "    <menuitem action='Delete'/>"
        "  </popup>"
        "</ui>";

    accountUiManager->add_ui_from_string(account_ui_info);
    //Gtk::Widget* accountMenuBar = uiManager->get_widget("/PopupMenu");
    //get_vbox()->pack_start(*accountMenuBar, Gtk::PACK_SHRINK);

    refresh();

    show_all_children();

    return true;
}


/**
 * Regenerate the account list
 */
void ConnectDialog::refresh()
{
    accountListStore->clear();

    std::vector<XmppAccount> accounts = parent.config.getAccounts();
    for (unsigned int i=0 ; i<accounts.size() ; i++)
        {
        XmppAccount account = accounts[i];
        Gtk::TreeModel::Row row = *(accountListStore->append());
        row[accountColumns.nameColumn] = account.getName();
        row[accountColumns.hostColumn] = account.getHost();
        }
    accountView.expand_all();
}



//#########################################################################
//# F I L E    S E N D    D I A L O G
//#########################################################################


void FileSendDialog::okCallback()
{
    response(Gtk::RESPONSE_OK);
    hide();
}

void FileSendDialog::cancelCallback()
{
    response(Gtk::RESPONSE_CANCEL);
    hide();
}


void FileSendDialog::buttonCallback()
{
    Gtk::FileChooserDialog dlg("Select a file to send",
                      Gtk::FILE_CHOOSER_ACTION_OPEN);
    dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dlg.add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_OK);
    int ret = dlg.run();
    if (ret == Gtk::RESPONSE_OK || ret == Gtk::RESPONSE_ACCEPT)
        {
        fileName = dlg.get_filename();
        fileNameField.set_text(fileName);
        }
}

bool FileSendDialog::doSetup()
{
    set_title("Send file to user");
    set_size_request(400,150);

    Glib::RefPtr<Gtk::ActionGroup> actionGroup = Gtk::ActionGroup::create();
    actionGroup->add( Gtk::Action::create("MenuFile", "_File") );
    actionGroup->add( Gtk::Action::create("Send", Gtk::Stock::NETWORK, "Send File"),
        sigc::mem_fun(*this, &FileSendDialog::okCallback) );
    actionGroup->add( Gtk::Action::create("Cancel",  Gtk::Stock::CANCEL, "Cancel"),
        sigc::mem_fun(*this, &FileSendDialog::cancelCallback) );


    Glib::RefPtr<Gtk::UIManager> uiManager = Gtk::UIManager::create();

    uiManager->insert_action_group(actionGroup, 0);
    add_accel_group(uiManager->get_accel_group());

    Glib::ustring ui_info =
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='MenuFile'>"
        "      <menuitem action='Send'/>"
        "      <separator/>"
        "      <menuitem action='Cancel'/>"
        "    </menu>"
        "  </menubar>"
        "</ui>";

    uiManager->add_ui_from_string(ui_info);
    Gtk::Widget* pMenuBar = uiManager->get_widget("/MenuBar");
    get_vbox()->pack_start(*pMenuBar, Gtk::PACK_SHRINK);

    table.resize(2, 2);
    get_vbox()->pack_start(table);

    jidLabel.set_text("User ID");
    table.attach(jidLabel, 0, 1, 0, 1);
    jidField.set_text("inkscape");
    table.attach(jidField, 1, 2, 0, 1);

    selectFileButton.set_label("Select");
    selectFileButton.signal_clicked().connect(
           sigc::mem_fun(*this, &FileSendDialog::buttonCallback) );
    table.attach(selectFileButton, 0, 1, 1, 2);

    fileName = "";
    fileNameField.set_text("No file selected");
    fileNameField.set_editable(false);
    table.attach(fileNameField, 1, 2, 1, 2);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_OK);

    show_all_children();

    return true;
}


//#########################################################################
//# F I L E    R E C E I V E   D I A L O G
//#########################################################################


void FileReceiveDialog::okCallback()
{
    response(Gtk::RESPONSE_OK);
    hide();
}

void FileReceiveDialog::cancelCallback()
{
    response(Gtk::RESPONSE_CANCEL);
    hide();
}

void FileReceiveDialog::buttonCallback()
{
    Gtk::FileChooserDialog dlg("Select a file to save",
                      Gtk::FILE_CHOOSER_ACTION_SAVE);
    dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dlg.add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_OK);
    int ret = dlg.run();
    if (ret == Gtk::RESPONSE_OK || ret == Gtk::RESPONSE_ACCEPT)
        {
        fileName = dlg.get_filename();
        fileNameField.set_text(fileName);
        }
}


bool FileReceiveDialog::doSetup()
{
    set_title("File being sent by user");
    set_size_request(450,250);

    Glib::RefPtr<Gtk::ActionGroup> actionGroup = Gtk::ActionGroup::create();
    actionGroup->add( Gtk::Action::create("MenuFile", "_File") );
    actionGroup->add( Gtk::Action::create("Send", Gtk::Stock::NETWORK, "Send File"),
        sigc::mem_fun(*this, &FileReceiveDialog::okCallback) );
    actionGroup->add( Gtk::Action::create("Cancel",  Gtk::Stock::CANCEL, "Cancel"),
        sigc::mem_fun(*this, &FileReceiveDialog::cancelCallback) );


    Glib::RefPtr<Gtk::UIManager> uiManager = Gtk::UIManager::create();

    uiManager->insert_action_group(actionGroup, 0);
    add_accel_group(uiManager->get_accel_group());

    Glib::ustring ui_info =
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='MenuFile'>"
        "      <menuitem action='Send'/>"
        "      <separator/>"
        "      <menuitem action='Cancel'/>"
        "    </menu>"
        "  </menubar>"
        "</ui>";

    uiManager->add_ui_from_string(ui_info);
    Gtk::Widget* pMenuBar = uiManager->get_widget("/MenuBar");
    get_vbox()->pack_start(*pMenuBar, Gtk::PACK_SHRINK);

    table.resize(6, 2);
    get_vbox()->pack_start(table);

    jidLabel.set_text("User ID");
    table.attach(jidLabel, 0, 1, 0, 1);
    jidField.set_text(jid);
    jidField.set_editable(false);
    table.attach(jidField, 1, 2, 0, 1);

    offeredLabel.set_text("File Offered");
    table.attach(offeredLabel, 0, 1, 1, 2);
    offeredField.set_text(offeredName);
    offeredField.set_editable(false);
    table.attach(offeredField, 1, 2, 1, 2);

    descLabel.set_text("Description");
    table.attach(descLabel, 0, 1, 2, 3);
    descField.set_text(desc);
    descField.set_editable(false);
    table.attach(descField, 1, 2, 2, 3);

    char buf[32];
    snprintf(buf, 31, "%ld", fileSize);
    sizeLabel.set_text("Size");
    table.attach(sizeLabel, 0, 1, 3, 4);
    sizeField.set_text(buf);
    sizeField.set_editable(false);
    table.attach(sizeField, 1, 2, 3, 4);

    hashLabel.set_text("MD5 Hash");
    table.attach(hashLabel, 0, 1, 4, 5);
    hashField.set_text(hash);
    hashField.set_editable(false);
    table.attach(hashField, 1, 2, 4, 5);

    selectFileButton.set_label("Select");
    selectFileButton.signal_clicked().connect(
           sigc::mem_fun(*this, &FileReceiveDialog::buttonCallback) );
    table.attach(selectFileButton, 0, 1, 5, 6);

    fileName = "";
    fileNameField.set_text("No file selected");
    fileNameField.set_editable(false);
    table.attach(fileNameField, 1, 2, 5, 6);


    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_OK);

    show_all_children();

    return true;
}


//#########################################################################
//# M A I N    W I N D O W
//#########################################################################

PedroGui::PedroGui()
{
    doSetup();
}

PedroGui::~PedroGui()
{
    chatDeleteAll();
    groupChatDeleteAll();
}


void PedroGui::error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    gchar * buffer = g_strdup_vprintf(fmt, args);
    va_end(args) ;

    Gtk::MessageDialog dlg(buffer,
                           false,
                           Gtk::MESSAGE_ERROR,
                           Gtk::BUTTONS_OK,
                           true);
    dlg.run();
    g_free(buffer);
}

void PedroGui::status(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    gchar * buffer = g_strdup_vprintf(fmt, args);
    va_end(args) ;
    messageList.postMessage("STATUS", buffer);
    g_free(buffer);
}

//################################
//# CHAT WINDOW MANAGEMENT
//################################
bool PedroGui::chatCreate(const DOMString &userJid)
{
    std::vector<ChatWindow *>::iterator iter;
    for (iter=chats.begin() ; iter != chats.end() ; iter++)
        {
        if (userJid == (*iter)->getJid())
            return false;
        }
    ChatWindow *chat = new ChatWindow(*this, userJid);
    chat->show();
    chats.push_back(chat);
    return true;
}

bool PedroGui::chatDelete(const DOMString &userJid)
{
    std::vector<ChatWindow *>::iterator iter;
    for (iter=chats.begin() ; iter != chats.end() ; )
        {
        if (userJid == (*iter)->getJid())
            {
            delete(*iter);
            iter = chats.erase(iter);
            }
        else
            iter++;
        }
    return true;
}

bool PedroGui::chatDeleteAll()
{
    std::vector<ChatWindow *>::iterator iter;
    for (iter=chats.begin() ; iter != chats.end() ; )
        {
        delete(*iter);
        iter = chats.erase(iter);
        }
    return true;
}

bool PedroGui::chatMessage(const DOMString &from, const DOMString &data)
{
    std::vector<ChatWindow *>::iterator iter;
    for (iter=chats.begin() ; iter != chats.end() ; iter++)
        {
        if (from == (*iter)->getJid())
            {
            (*iter)->postMessage(data);
            return true;
            }
        }
    ChatWindow *chat = new ChatWindow(*this, from);
    chat->show();
    chats.push_back(chat);
    chat->postMessage(data);
    return true;
}


//################################
//# GROUP CHAT WINDOW MANAGEMENT
//################################

bool PedroGui::groupChatCreate(const DOMString &groupJid, const DOMString &nick)
{
    std::vector<GroupChatWindow *>::iterator iter;
    for (iter=groupChats.begin() ; iter != groupChats.end() ; iter++)
        {
        if (groupJid == (*iter)->getGroupJid())
            return false;
        }
    GroupChatWindow *chat = new GroupChatWindow(*this, groupJid, nick);
    chat->show();
    groupChats.push_back(chat);
    return true;
}


bool PedroGui::groupChatDelete(const DOMString &groupJid, const DOMString &nick)
{
    std::vector<GroupChatWindow *>::iterator iter;
    for (iter=groupChats.begin() ; iter != groupChats.end() ;)
        {
        if (groupJid == (*iter)->getGroupJid() &&
            nick     == (*iter)->getNick())
            {
            delete(*iter);
            iter = groupChats.erase(iter);
            }
        else
            iter++;
        }
    return true;
}


bool PedroGui::groupChatDeleteAll()
{
    std::vector<GroupChatWindow *>::iterator iter;
    for (iter=groupChats.begin() ; iter != groupChats.end() ; )
        {
        delete(*iter);
        iter = groupChats.erase(iter);
        }
    return true;
}


bool PedroGui::groupChatMessage(const DOMString &groupJid,
              const DOMString &from, const DOMString &data)
{
    std::vector<GroupChatWindow *>::iterator iter;
    for (iter=groupChats.begin() ; iter != groupChats.end() ; iter++)
        {
        if (groupJid == (*iter)->getGroupJid())
            {
            (*iter)->receiveMessage(from, data);
            }
        }
    return true;
}

bool PedroGui::groupChatPresence(const DOMString &groupJid,
                      const DOMString &nick, bool presence,
                      const DOMString &show,
                      const DOMString &status)
{
    std::vector<GroupChatWindow *>::iterator iter;
    for (iter=groupChats.begin() ; iter != groupChats.end() ; iter++)
        {
        if (groupJid == (*iter)->getGroupJid())
            {
            (*iter)->receivePresence(nick, presence, show, status);
            }
        }
    return true;
}

//################################
//# EVENTS
//################################

/**
 *
 */
void PedroGui::padlockEnable()
{
    padlockIcon.set(Gtk::Stock::DIALOG_AUTHENTICATION,
               Gtk::ICON_SIZE_MENU);
}

/**
 *
 */
void PedroGui::padlockDisable()
{
    padlockIcon.clear();
}


/**
 *
 */
void PedroGui::handleConnectEvent()
{
     status("##### CONNECTED");
     actionEnable("Connect",    false);
     actionEnable("Chat",       true);
     actionEnable("GroupChat",  true);
     actionEnable("Disconnect", true);
     actionEnable("RegPass",    true);
     actionEnable("RegCancel",  true);
     DOMString title = "Pedro - ";
     title.append(client.getJid());
     set_title(title);
}


/**
 *
 */
void PedroGui::handleDisconnectEvent()
{
     status("##### DISCONNECTED");
     actionEnable("Connect",    true);
     actionEnable("Chat",       false);
     actionEnable("GroupChat",  false);
     actionEnable("Disconnect", false);
     actionEnable("RegPass",    false);
     actionEnable("RegCancel",  false);
     padlockDisable();
     DOMString title = "Pedro";
     set_title(title);
     chatDeleteAll();
     groupChatDeleteAll();
     roster.clear();
}


/**
 *
 */
void PedroGui::doEvent(const XmppEvent &event)
{

    int typ = event.getType();
    switch (typ)
        {
        case XmppEvent::EVENT_STATUS:
            {
            //printf("##### STATUS: %s\n", event.getData().c_str());
            status("%s", event.getData().c_str());
            break;
            }
        case XmppEvent::EVENT_ERROR:
            {
            //printf("##### ERROR: %s\n", event.getData().c_str());
            error("%s", event.getData().c_str());
            padlockDisable();
            break;
            }
        case XmppEvent::EVENT_SSL_STARTED:
            {
            padlockEnable();
            break;
            }
        case XmppEvent::EVENT_CONNECTED:
            {
            handleConnectEvent();
            break;
            }
        case XmppEvent::EVENT_DISCONNECTED:
            {
            handleDisconnectEvent();
            break;
            }
        case XmppEvent::EVENT_MESSAGE:
            {
            status("##### MESSAGE: %s\n", event.getFrom().c_str());
            chatMessage(event.getFrom(), event.getData());
            break;
            }
        case XmppEvent::EVENT_PRESENCE:
            {
            status("##### PRESENCE: %s\n", event.getFrom().c_str());
            roster.refresh();
            break;
            }
        case XmppEvent::EVENT_ROSTER:
            {
            status("##### ROSTER\n");
            roster.refresh();
            break;
            }
        case XmppEvent::EVENT_MUC_JOIN:
            {
            status("##### GROUP JOINED: %s\n", event.getGroup().c_str());
            break;
            }
        case XmppEvent::EVENT_MUC_MESSAGE:
            {
            //printf("##### MUC_MESSAGE: %s\n", event.getGroup().c_str());
            groupChatMessage(event.getGroup(),
                    event.getFrom(), event.getData());
            break;
            }
        case XmppEvent::EVENT_MUC_PRESENCE:
            {
            //printf("##### MUC_USER LIST: %s\n", event.getFrom().c_str());
            groupChatPresence(event.getGroup(),
                              event.getFrom(),
                              event.getPresence(),
                              event.getShow(),
                              event.getStatus());
            break;
            }
        case XmppEvent::EVENT_MUC_LEAVE:
            {
            status("##### GROUP LEFT: %s\n", event.getGroup().c_str());
            groupChatDelete(event.getGroup(), event.getFrom());
            break;
            }
        case XmppEvent::EVENT_FILE_RECEIVE:
            {
            status("##### FILE RECEIVE: %s\n", event.getFileName().c_str());
            doReceiveFile(event.getFrom(), event.getIqId(), event.getStreamId(),
                       event.getFileName(), event.getFileDesc(),
                       event.getFileSize(), event.getFileHash());
            break;
            }
        case XmppEvent::EVENT_REGISTRATION_NEW:
            {
            status("##### REGISTERED: %s at %s\n",
                       event.getTo().c_str(), event.getFrom().c_str());
            break;
            }
        case XmppEvent::EVENT_REGISTRATION_CHANGE_PASS:
            {
            status("##### PASSWORD CHANGED: %s at %s\n",
                       event.getTo().c_str(), event.getFrom().c_str());
            break;
            }
        case XmppEvent::EVENT_REGISTRATION_CANCEL:
            {
            //client.disconnect();
            status("##### REGISTERATION CANCELLED: %s at %s\n",
                       event.getTo().c_str(), event.getFrom().c_str());
            break;
            }
        default:
            {
            printf("unknown event type: %d\n", typ);
            break;
            }
        }

}

/**
 *
 */
bool PedroGui::checkEventQueue()
{
    while (client.eventQueueAvailable() > 0)
        {
        XmppEvent evt = client.eventQueuePop();
        doEvent(evt);
        }

    while( Gtk::Main::events_pending() )
        Gtk::Main::iteration();

    return true;
}


//##################
//# COMMANDS
//##################
void PedroGui::doChat(const DOMString &jid)
{
    if (jid.size()>0)
        {
        chatCreate(jid);
        return;
        }

    FileSendDialog dlg(*this);
    int ret = dlg.run();
    if (ret == Gtk::RESPONSE_OK)
        {
        chatCreate(dlg.getJid());
        }

}

void PedroGui::doSendFile(const DOMString &jid)
{
    FileSendDialog dlg(*this);
    if (jid.size()>0)
        dlg.setJid(jid);
    int ret = dlg.run();
    if (ret == Gtk::RESPONSE_OK)
        {
        DOMString fileName = dlg.getFileName();
        printf("fileName:%s\n", fileName.c_str());
        DOMString offeredName = "";
        DOMString desc = "";
        client.fileSendBackground(jid, offeredName, fileName, desc);
        }

}

void PedroGui::doReceiveFile(
                      const DOMString &jid,
                      const DOMString &iqId,
                      const DOMString &streamId,
                      const DOMString &offeredName,
                      const DOMString &desc,
                      long  size,
                      const DOMString &hash
                      )

{
    FileReceiveDialog dlg(*this, jid, iqId, streamId,
                          offeredName, desc, size, hash);
    int ret = dlg.run();
    if (ret == Gtk::RESPONSE_OK)
        {
        DOMString fileName = dlg.getFileName();
        printf("fileName:%s\n", fileName.c_str());
        client.fileReceiveBackground(jid, iqId, streamId, fileName, size, hash);
        }

}


//##################
//# CALLBACKS
//##################
void PedroGui::connectCallback()
{
    ConnectDialog dialog(*this);
    int result = dialog.run();
    dialog.hide();
    if (result == Gtk::RESPONSE_OK)
        {
        client.setHost(dialog.getHost());
        client.setPort(dialog.getPort());
        client.setUsername(dialog.getUser());
        client.setPassword(dialog.getPass());
        client.setResource(dialog.getResource());
        client.setDoRegister(dialog.getRegister());
        client.connect();
        }
}



void PedroGui::chatCallback()
{
    ChatDialog dialog(*this);
    int result = dialog.run();
    dialog.hide();
    if (result == Gtk::RESPONSE_OK)
        {
        client.message(dialog.getUser(), dialog.getText());
        }
}



void PedroGui::groupChatCallback()
{
    GroupChatDialog dialog(*this);
    int result = dialog.run();
    dialog.hide();
    if (result != Gtk::RESPONSE_OK)
        return;
    DOMString groupJid = dialog.getGroup();
    groupJid.append("@");
    groupJid.append(dialog.getHost());
    if (client.groupChatExists(groupJid))
        {
        error("Group chat %s already exists", groupJid.c_str());
        return;
        }
    groupChatCreate(groupJid, dialog.getNick());
    client.groupChatJoin(groupJid, dialog.getNick(), dialog.getPass() );
    config.setMucGroup(dialog.getGroup());
    config.setMucHost(dialog.getHost());
    config.setMucNick(dialog.getNick());
    config.setMucPassword(dialog.getPass());
 
    configSave();
}


void PedroGui::disconnectCallback()
{
    client.disconnect();
}


void PedroGui::quitCallback()
{
    Gtk::Main::quit();
}


void PedroGui::fontCallback()
{
    Gtk::FontSelectionDialog dlg;
    int ret = dlg.run();
    if (ret == Gtk::RESPONSE_OK)
        {
        Glib::ustring fontName = dlg.get_font_name();
        Pango::FontDescription fontDesc(fontName);
        modify_font(fontDesc);
        }
}

void PedroGui::colorCallback()
{
    Gtk::ColorSelectionDialog dlg;
    int ret = dlg.run();
    if (ret == Gtk::RESPONSE_OK)
        {
        Gdk::Color col = dlg.get_colorsel()->get_current_color();
        modify_bg(Gtk::STATE_NORMAL, col);
        }
}

void PedroGui::regPassCallback()
{
    PasswordDialog dlg(*this);
    int ret = dlg.run();
    if (ret == Gtk::RESPONSE_OK)
        {
        DOMString newpass = dlg.getNewPass();
        client.inBandRegistrationChangePassword(newpass);
        }
}


void PedroGui::regCancelCallback()
{
    Gtk::MessageDialog dlg(*this, "Do you want to cancel your registration on the server?",
        false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true);
    int ret = dlg.run();
    if (ret == Gtk::RESPONSE_YES)
        {
        client.inBandRegistrationCancel();
        }
}



void PedroGui::sendFileCallback()
{
    doSendFile("");
}



void PedroGui::aboutCallback()
{
    Gtk::AboutDialog dlg;
    std::vector<Glib::ustring>authors;
    authors.push_back("Bob Jamison");
    dlg.set_authors(authors);
    DOMString comments = "A simple XMPP gui client  ";
    comments.append("Based on the Pedro XMPP client");
    dlg.set_comments(comments);
    dlg.set_version("1.0");
    dlg.run();
}



void PedroGui::actionEnable(const DOMString &name, bool val)
{
    DOMString path = "/ui/MenuBar/";
    path.append(name);
    Glib::RefPtr<Gtk::Action> action = uiManager->get_action(path);
    if (!action)
        {
        path = "/ui/MenuBar/MenuFile/";
        path.append(name);
        action = uiManager->get_action(path);
        }
    if (!action)
        {
        path = "/ui/MenuBar/MenuEdit/";
        path.append(name);
        action = uiManager->get_action(path);
        }
    if (!action)
        {
        path = "/ui/MenuBar/MenuRegister/";
        path.append(name);
        action = uiManager->get_action(path);
        }
    if (!action)
        {
        path = "/ui/MenuBar/MenuTransfer/";
        path.append(name);
        action = uiManager->get_action(path);
        }
    if (!action)
        {
        path = "/ui/MenuBar/MenuHelp/";
        path.append(name);
        action = uiManager->get_action(path);
        }
    if (!action)
        return;
    action->set_sensitive(val);
}


bool PedroGui::configLoad()
{
    if (!config.readFile("pedro.ini"))
        return false;
    return true;
}


bool PedroGui::configSave()
{
    if (!config.writeFile("pedro.ini"))
        return false;
    return true;
}




bool PedroGui::doSetup()
{
    configLoad();

    set_title("Pedro XMPP Client");
    set_size_request(500, 300);
    add(mainBox);

    Glib::RefPtr<Gtk::ActionGroup> actionGroup = Gtk::ActionGroup::create();

    //### FILE MENU
    actionGroup->add( Gtk::Action::create("MenuFile", "_File") );

    actionGroup->add( Gtk::Action::create("Connect",
                                  Gtk::Stock::CONNECT, "Connect"),
    sigc::mem_fun(*this, &PedroGui::connectCallback) );

    actionGroup->add( Gtk::Action::create("Chat",
                                  Gtk::Stock::CONNECT, "Chat"),
    sigc::mem_fun(*this, &PedroGui::chatCallback) );

    actionGroup->add( Gtk::Action::create("GroupChat",
                                  Gtk::Stock::CONNECT, "Group Chat"),
    sigc::mem_fun(*this, &PedroGui::groupChatCallback) );

    actionGroup->add( Gtk::Action::create("Disconnect",
                                  Gtk::Stock::DISCONNECT, "Disconnect"),
    sigc::mem_fun(*this, &PedroGui::disconnectCallback) );

    actionGroup->add( Gtk::Action::create("Quit", Gtk::Stock::QUIT),
    sigc::mem_fun(*this, &PedroGui::quitCallback) );

    //### EDIT MENU
    actionGroup->add( Gtk::Action::create("MenuEdit", "_Edit") );
    actionGroup->add( Gtk::Action::create("SelectFont",
                                  Gtk::Stock::SELECT_FONT, "Select Font"),
    sigc::mem_fun(*this, &PedroGui::fontCallback) );
    actionGroup->add( Gtk::Action::create("SelectColor",
                                  Gtk::Stock::SELECT_COLOR, "Select Color"),
    sigc::mem_fun(*this, &PedroGui::colorCallback) );

    //### REGISTER MENU
    actionGroup->add( Gtk::Action::create("MenuRegister", "_Registration") );
    actionGroup->add( Gtk::Action::create("RegPass",
                                  Gtk::Stock::DIALOG_AUTHENTICATION, "Change Password"),
    sigc::mem_fun(*this, &PedroGui::regPassCallback) );
    actionGroup->add( Gtk::Action::create("RegCancel",
                                  Gtk::Stock::CANCEL, "Cancel Registration"),
    sigc::mem_fun(*this, &PedroGui::regCancelCallback) );

    //### TRANSFER MENU
    actionGroup->add( Gtk::Action::create("MenuTransfer", "_Transfer") );
    actionGroup->add( Gtk::Action::create("SendFile",
                                  Gtk::Stock::NETWORK, "Send File"),
    sigc::mem_fun(*this, &PedroGui::sendFileCallback) );

    //### HELP MENU
    actionGroup->add( Gtk::Action::create("MenuHelp", "_Help") );
    actionGroup->add( Gtk::Action::create("About",
                                  Gtk::Stock::ABOUT, "About Pedro"),
    sigc::mem_fun(*this, &PedroGui::aboutCallback) );

    uiManager = Gtk::UIManager::create();

    uiManager->insert_action_group(actionGroup, 0);
    add_accel_group(uiManager->get_accel_group());

    Glib::ustring ui_info =
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='MenuFile'>"
        "      <menuitem action='Connect'/>"
        "      <separator/>"
        "      <menuitem action='Chat'/>"
        "      <menuitem action='GroupChat'/>"
        "      <separator/>"
        "      <menuitem action='Disconnect'/>"
        "      <menuitem action='Quit'/>"
        "    </menu>"
        "    <menu action='MenuEdit'>"
        "      <menuitem action='SelectFont'/>"
        "      <menuitem action='SelectColor'/>"
        "    </menu>"
        "    <menu action='MenuRegister'>"
        "      <menuitem action='RegPass'/>"
        "      <menuitem action='RegCancel'/>"
        "    </menu>"
        "    <menu action='MenuTransfer'>"
        "      <menuitem action='SendFile'/>"
        "    </menu>"
        "    <menu action='MenuHelp'>"
        "      <menuitem action='About'/>"
        "    </menu>"
        "  </menubar>"
        "</ui>";

    uiManager->add_ui_from_string(ui_info);
    Gtk::Widget* pMenuBar = uiManager->get_widget("/MenuBar");
    menuBarBox.pack_start(*pMenuBar, Gtk::PACK_SHRINK);

    padlockDisable();
    menuBarBox.pack_end(padlockIcon, Gtk::PACK_SHRINK);

    mainBox.pack_start(menuBarBox, Gtk::PACK_SHRINK);

    actionEnable("Connect",    true);
    actionEnable("Chat",       false);
    actionEnable("GroupChat",  false);
    actionEnable("Disconnect", false);
    actionEnable("RegPass",    false);
    actionEnable("RegCancel",  false);

    mainBox.pack_start(vPaned);
    vPaned.add1(roster);
    vPaned.add2(messageList);
    roster.setParent(this);

    show_all_children();

    //# Start a timer to check the queue every nn milliseconds
    Glib::signal_timeout().connect(
           sigc::mem_fun(*this, &PedroGui::checkEventQueue), 20 );

    //client.addXmppEventListener(*this);
    client.eventQueueEnable(true);

    return true;
}


} // namespace Pedro



//########################################################################
//# E N D    O F     F I L E
//########################################################################

