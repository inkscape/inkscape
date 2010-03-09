#ifndef SEEN_GRADIENT_VECTOR_H
#define SEEN_GRADIENT_VECTOR_H

/*
 * Gradient vector selection widget
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2010 Jon A. Cruz
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

#include <sigc++/connection.h>

#include <gtk/gtkvbox.h>
#include "../forward.h"

#define SP_TYPE_GRADIENT_VECTOR_SELECTOR (sp_gradient_vector_selector_get_type ())
#define SP_GRADIENT_VECTOR_SELECTOR(o) (GTK_CHECK_CAST ((o), SP_TYPE_GRADIENT_VECTOR_SELECTOR, SPGradientVectorSelector))
#define SP_GRADIENT_VECTOR_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_GRADIENT_VECTOR_SELECTOR, SPGradientVectorSelectorClass))
#define SP_IS_GRADIENT_VECTOR_SELECTOR(o) (GTK_CHECK_TYPE ((o), SP_TYPE_GRADIENT_VECTOR_SELECTOR))
#define SP_IS_GRADIENT_VECTOR_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_GRADIENT_VECTOR_SELECTOR))

struct SPGradientVectorSelector {
    GtkVBox vbox;

    guint idlabel : 1;

    bool swatched;

    SPDocument *doc;
    SPGradient *gr;

    /* Vector menu */
    GtkWidget *menu;

    sigc::connection gradient_release_connection;
    sigc::connection defs_release_connection;
    sigc::connection defs_modified_connection;


    void setSwatched();
};

struct SPGradientVectorSelectorClass {
    GtkVBoxClass parent_class;

    void (* vector_set) (SPGradientVectorSelector *gvs, SPGradient *gr);
};

GType sp_gradient_vector_selector_get_type(void);

GtkWidget *sp_gradient_vector_selector_new (SPDocument *doc, SPGradient *gradient);

void sp_gradient_vector_selector_set_gradient (SPGradientVectorSelector *gvs, SPDocument *doc, SPGradient *gr);

SPDocument *sp_gradient_vector_selector_get_document (SPGradientVectorSelector *gvs);
SPGradient *sp_gradient_vector_selector_get_gradient (SPGradientVectorSelector *gvs);

/* fixme: rethink this (Lauris) */
GtkWidget *sp_gradient_vector_editor_new (SPGradient *gradient, SPStop *stop = NULL);



#endif // SEEN_GRADIENT_VECTOR_H

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
