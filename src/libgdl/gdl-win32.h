#ifndef __INKSCAPE_GDL_WIN32_H__
#define __INKSCAPE_GDL_WIN32_H__

/*
 * Windows stuff
 *
 * Author:
 *   Albin Sunnanbo
 *
 * This code is in public domain
 */



#define WIN32_MAJORVERSION_VISTA               0x0006



#include <config.h>
#include <windows.h>
#include <gdk/gdk.h>

#ifndef WIN32
#error "This file is only usable for Windows"
#endif

/* Platform detection */
gboolean is_os_vista();

#endif /* __INKSCAPE_GDL_WIN32_H__ */
