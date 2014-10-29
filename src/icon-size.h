#ifndef SEEN_ICON_SIZE_H
#define SEEN_ICON_SIZE_H

/*
 * Generic icon widget
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtk.h>

namespace Inkscape {

    enum IconSize {
        ICON_SIZE_INVALID = ::GTK_ICON_SIZE_INVALID,
        ICON_SIZE_MENU = ::GTK_ICON_SIZE_MENU,
        ICON_SIZE_SMALL_TOOLBAR = ::GTK_ICON_SIZE_SMALL_TOOLBAR,
        ICON_SIZE_LARGE_TOOLBAR = ::GTK_ICON_SIZE_LARGE_TOOLBAR,
        ICON_SIZE_BUTTON = ::GTK_ICON_SIZE_BUTTON,
        ICON_SIZE_DND = ::GTK_ICON_SIZE_DND,
        ICON_SIZE_DIALOG = ::GTK_ICON_SIZE_DIALOG,
        ICON_SIZE_DECORATION
    };

    GtkIconSize getRegisteredIconSize( IconSize size );
} // namespace Inkscape

#endif // SEEN_ICON_SIZE_H
