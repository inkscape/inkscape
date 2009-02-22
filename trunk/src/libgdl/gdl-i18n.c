/*
 * Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation
 * All rights reserved.
 *
 * This file is part of the Gnome Devtools Library.
 *
 * The Gnome Devtools Library is free software; you can redistribute
 * it and/or modify it under the terms of the GNU Library General
 * Public License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * The Gnome Devtools Library is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gdl-i18n.h"

char *
gdl_gettext (const char *msgid)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
/* 		bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR); */
		bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
		initialized = TRUE;
	}

	return dgettext (GETTEXT_PACKAGE, msgid);
}


