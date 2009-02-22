/*
 * Windows stuff
 *
 * Author:
 *   Albin Sunnanbo
 *
 * This code is in public domain
 */
#ifndef __INKSCAPE_GDL_WIN32_H__
#define __INKSCAPE_GDL_WIN32_H__
#ifdef WIN32

#include <gdk/gdk.h>

#define WIN32_MAJORVERSION_VISTA               0x0006

/* Platform detection */
gboolean is_os_vista();

#endif // ifdef WIN32
#endif /* __INKSCAPE_GDL_WIN32_H__ */
