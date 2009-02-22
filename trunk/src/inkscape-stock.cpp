/*
 * @file inkscape-stock.h GTK+ Stock resources
 *
 * Authors:
 *   Robert Crosbie
 *
 * Copyright (C) 1999-2002 Authors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "gtk/gtkiconfactory.h"

#include "widgets/icon.h"
#include "ui/widget/panel.h"

void
inkscape_gtk_stock_init() {
    static bool stock_initialized = false;

    if (stock_initialized)
        return;

    // Ensure icon internal sizes get set up:
    sp_icon_get_phys_size(GTK_ICON_SIZE_MENU);

    // And also prepare the swatches.
    Inkscape::UI::Widget::Panel::prep();

    GtkIconFactory *icon_factory = gtk_icon_factory_new();
    /* todo: Should we simply remove this file now that we're no longer
     * calling gtk_icon_factory_add here? */
    gtk_icon_factory_add_default(icon_factory);

    stock_initialized = true;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
