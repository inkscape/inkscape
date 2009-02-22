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
/*
  @NOTATION@
 */

/*
 * Handles all of the internationalization configuration options.
 * Author: Tom Tromey <tromey@creche.cygnus.com>
 */

#ifndef __GDL_18N_H__
#define __GDL_18N_H__ 1

#include <glib.h>


G_BEGIN_DECLS

#ifdef ENABLE_NLS
#    include <libintl.h>
#    undef _
#    define _(String) gdl_gettext (String)
#    ifdef gettext_noop
#        define N_(String) gettext_noop (String)
#    else
#        define N_(String) (String)
#    endif
#else
/* Stubs that do something close enough.  */
#    undef textdomain
#    define textdomain(String) (String)
#    undef gettext
#    define gettext(String) (String)
#    undef dgettext
#    define dgettext(Domain,Message) (Message)
#    undef dcgettext
#    define dcgettext(Domain,Message,Type) (Message)
#    undef bindtextdomain
#    define bindtextdomain(Domain,Directory) (Domain)
#    undef bind_textdomain_codeset
#    define bind_textdomain_codeset(Domain,CodeSet) (Domain)
#    undef _
#    define _(String) (String)
#    undef N_
#    define N_(String) (String)
#endif

char *gdl_gettext (const char *msgid);

G_END_DECLS

#endif /* __GDL_I18N_H__ */
