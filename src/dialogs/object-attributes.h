/** @file
 * @brief  Generic object attribute editor
 */
/* Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Licensed under GNU GPL
 */

#ifndef SEEN_DIALOGS_OBJECT_ATTRIBUTES_H
#define SEEN_DIALOGS_OBJECT_ATTRIBUTES_H

#include <glib.h>
#include <gtk/gtkwidget.h>
#include "../forward.h"

void sp_object_attributes_dialog (SPObject *object, const gchar *tag);

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
