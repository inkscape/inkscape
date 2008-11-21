/** @file
 * \brief  Display settings dialog
 */
/* Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_DIALOGS_ITEM_PROPERTIES_H
#define SEEN_DIALOGS_ITEM_PROPERTIES_H

#include <glib.h>
#include <gtk/gtkwidget.h>
#include "../forward.h"

GtkWidget *sp_item_widget_new (void);

void sp_item_dialog (void);

#endif

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
