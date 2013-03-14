#ifndef SEEN_GRADIENT_TOOLBAR_H
#define SEEN_GRADIENT_TOOLBAR_H

/*
 * Gradient aux toolbar
 *
 * Authors:
 *   bulia byak <bulia@dr.com>
 *
 * Copyright (C) 2005 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtk.h>
class SPDesktop;

void sp_gradient_toolbox_prep(SPDesktop * /*desktop*/, GtkActionGroup* mainActions, GObject* holder);

#endif /* !SEEN_GRADIENT_TOOLBAR_H */
