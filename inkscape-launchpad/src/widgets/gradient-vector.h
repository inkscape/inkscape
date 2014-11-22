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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtkmm/liststore.h>
#include <sigc++/connection.h>
#include "gradient-selector.h"

#define SP_TYPE_GRADIENT_VECTOR_SELECTOR (sp_gradient_vector_selector_get_type ())
#define SP_GRADIENT_VECTOR_SELECTOR(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_GRADIENT_VECTOR_SELECTOR, SPGradientVectorSelector))
#define SP_GRADIENT_VECTOR_SELECTOR_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), SP_TYPE_GRADIENT_VECTOR_SELECTOR, SPGradientVectorSelectorClass))
#define SP_IS_GRADIENT_VECTOR_SELECTOR(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_GRADIENT_VECTOR_SELECTOR))
#define SP_IS_GRADIENT_VECTOR_SELECTOR_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), SP_TYPE_GRADIENT_VECTOR_SELECTOR))

class SPDocument;
class SPObject;
class SPGradient;
class SPStop;

struct SPGradientVectorSelector {
#if GTK_CHECK_VERSION(3,0,0)
    GtkBox vbox;
#else
    GtkVBox vbox;
#endif

    guint idlabel : 1;

    bool swatched;

    SPDocument *doc;
    SPGradient *gr;

    /* Gradient vectors store */
    Glib::RefPtr<Gtk::ListStore> store;
    SPGradientSelector::ModelColumns *columns;

    sigc::connection gradient_release_connection;
    sigc::connection defs_release_connection;
    sigc::connection defs_modified_connection;
    sigc::connection tree_select_connection;

    void setSwatched();
};

struct SPGradientVectorSelectorClass {
#if GTK_CHECK_VERSION(3,0,0)
    GtkBoxClass parent_class;
#else
    GtkVBoxClass parent_class;
#endif

    void (* vector_set) (SPGradientVectorSelector *gvs, SPGradient *gr);
};

GType sp_gradient_vector_selector_get_type(void);

GtkWidget *sp_gradient_vector_selector_new (SPDocument *doc, SPGradient *gradient);

void sp_gradient_vector_selector_set_gradient (SPGradientVectorSelector *gvs, SPDocument *doc, SPGradient *gr);

SPDocument *sp_gradient_vector_selector_get_document (SPGradientVectorSelector *gvs);
SPGradient *sp_gradient_vector_selector_get_gradient (SPGradientVectorSelector *gvs);

/* fixme: rethink this (Lauris) */
GtkWidget *sp_gradient_vector_editor_new (SPGradient *gradient, SPStop *stop = NULL);

guint32 sp_average_color(guint32 c1, guint32 c2, gdouble p = 0.5);

Glib::ustring gr_prepare_label (SPObject *obj);
Glib::ustring gr_ellipsize_text(Glib::ustring const &src, size_t maxlen);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
