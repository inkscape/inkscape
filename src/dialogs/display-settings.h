#ifndef __SP_DISPLAY_SETTINGS_H__
#define __SP_DISPLAY_SETTINGS_H__

/**
 * \brief Display settings dialog
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

#include <glib.h>

#include <gtk/gtkwidget.h>

void sp_display_dialog       ( void );
void sp_display_dialog_apply ( GtkWidget * widget );
void sp_display_dialog_close ( GtkWidget * widget );

// UPDATE THIS IF YOU'RE CHANGING OR REARRANGING PREFS PAGES.
// Otherwise the commands that open the dialog with a given page may become out of sync.

enum {
    PREFS_PAGE_MOUSE,
    PREFS_PAGE_SCROLLING,
    PREFS_PAGE_STEPS,
    PREFS_PAGE_TOOLS,
    PREFS_PAGE_WINDOWS,
    PREFS_PAGE_CLONES,
    PREFS_PAGE_TRANSFORMS,
    PREFS_PAGE_SELECTING,
    PREFS_PAGE_MISC
};

enum {
    PREFS_PAGE_TOOLS_SELECTOR,
    PREFS_PAGE_TOOLS_NODE,
    PREFS_PAGE_TOOLS_ZOOM,
    PREFS_PAGE_TOOLS_SHAPES,
    PREFS_PAGE_TOOLS_PENCIL,
    PREFS_PAGE_TOOLS_PEN,
    PREFS_PAGE_TOOLS_CALLIGRAPHY,
    PREFS_PAGE_TOOLS_TEXT,
    PREFS_PAGE_TOOLS_GRADIENT,
    PREFS_PAGE_TOOLS_CONNECTOR,
    PREFS_PAGE_TOOLS_DROPPER
};

enum {
    PREFS_PAGE_TOOLS_SHAPES_RECT,
    PREFS_PAGE_TOOLS_SHAPES_ELLIPSE,
    PREFS_PAGE_TOOLS_SHAPES_STAR,
    PREFS_PAGE_TOOLS_SHAPES_SPIRAL
};

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
