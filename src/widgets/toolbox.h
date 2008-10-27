#ifndef SEEN_TOOLBOX_H
#define SEEN_TOOLBOX_H

/**
 * \brief Main toolbox
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtkstyle.h>
#include <gtk/gtktooltips.h>

#include "forward.h"
#include "icon-size.h"

GtkWidget *sp_tool_toolbox_new ();
void sp_tool_toolbox_set_desktop(GtkWidget *toolbox, SPDesktop *desktop);

GtkWidget *sp_aux_toolbox_new ();
void sp_aux_toolbox_set_desktop(GtkWidget *toolbox, SPDesktop *desktop);

GtkWidget *sp_commands_toolbox_new ();
void sp_commands_toolbox_set_desktop(GtkWidget *toolbox, SPDesktop *desktop);

void show_aux_toolbox(GtkWidget *toolbox);

GtkWidget *sp_toolbox_button_normal_new_from_verb(GtkWidget *t,
                                                  Inkscape::IconSize size,
                                                  Inkscape::Verb * verb,
                                                  Inkscape::UI::View::View *view,
                                                  GtkTooltips *tt);

void aux_toolbox_space(GtkWidget *tb, gint space);

// utility
void sp_toolbox_add_label(GtkWidget *tbl, gchar const *title, bool wide = true);

Inkscape::IconSize prefToSize(Glib::ustring const &path, int base = 0 );

#endif /* !SEEN_TOOLBOX_H */

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
