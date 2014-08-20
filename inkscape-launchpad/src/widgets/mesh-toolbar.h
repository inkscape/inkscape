#ifndef SEEN_MESH_TOOLBAR_H
#define SEEN_MESH_TOOLBAR_H

/*
 * Mesh aux toolbar
 *
 * Authors:
 *   bulia byak <bulia@dr.com>
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 2012 authors
 * Copyright (C) 2005 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

class SPDesktop;

typedef struct _GtkActionGroup GtkActionGroup;
typedef struct _GObject GObject;

void sp_mesh_toolbox_prep(    SPDesktop * /*desktop*/, GtkActionGroup* mainActions, GObject* holder);

#endif /* !SEEN_MESH_TOOLBAR_H */
