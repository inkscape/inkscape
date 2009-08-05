/*
 * Gradient vector widget
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtktooltips.h>

#include "document.h"
#include "../document-private.h"
#include "../gradient-chemistry.h"

#include <glibmm/i18n.h>
#include <xml/repr.h>

#include "gradient-vector.h"

#include "gradient-selector.h"

enum {
    GRABBED,
    DRAGGED,
    RELEASED,
    CHANGED,
    LAST_SIGNAL
};

static void sp_gradient_selector_class_init (SPGradientSelectorClass *klass);
static void sp_gradient_selector_init (SPGradientSelector *selector);
static void sp_gradient_selector_destroy (GtkObject *object);

/* Signal handlers */
static void sp_gradient_selector_vector_set (SPGradientVectorSelector *gvs, SPGradient *gr, SPGradientSelector *sel);
static void sp_gradient_selector_edit_vector_clicked (GtkWidget *w, SPGradientSelector *sel);
static void sp_gradient_selector_add_vector_clicked (GtkWidget *w, SPGradientSelector *sel);

static void sp_gradient_selector_spread_activate (GtkWidget *widget, SPGradientSelector *sel);

static GtkVBoxClass *parent_class;
static guint signals[LAST_SIGNAL] = {0};

GType sp_gradient_selector_get_type(void)
{
    static GType type = 0;
    if (!type) {
        static const GTypeInfo info = {
            sizeof(SPGradientSelectorClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) sp_gradient_selector_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof(SPGradientSelector),
            0,    /* n_preallocs */
            (GInstanceInitFunc) sp_gradient_selector_init,
            0,    /* value_table */
        };

        type = g_type_register_static( GTK_TYPE_VBOX,
                                       "SPGradientSelector",
                                       &info,
                                       static_cast< GTypeFlags > (0) );
    }
    return type;
}

static void
sp_gradient_selector_class_init (SPGradientSelectorClass *klass)
{
    GtkObjectClass *object_class;

    object_class = (GtkObjectClass *) klass;

    parent_class = (GtkVBoxClass*)gtk_type_class (GTK_TYPE_VBOX);

    signals[GRABBED] =  gtk_signal_new ("grabbed",
                                        (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
                                        GTK_CLASS_TYPE(object_class),
                                        GTK_SIGNAL_OFFSET (SPGradientSelectorClass, grabbed),
                                        gtk_marshal_NONE__NONE,
                                        GTK_TYPE_NONE, 0);
    signals[DRAGGED] =  gtk_signal_new ("dragged",
                                        (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
                                        GTK_CLASS_TYPE(object_class),
                                        GTK_SIGNAL_OFFSET (SPGradientSelectorClass, dragged),
                                        gtk_marshal_NONE__NONE,
                                        GTK_TYPE_NONE, 0);
    signals[RELEASED] = gtk_signal_new ("released",
                                        (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
                                        GTK_CLASS_TYPE(object_class),
                                        GTK_SIGNAL_OFFSET (SPGradientSelectorClass, released),
                                        gtk_marshal_NONE__NONE,
                                        GTK_TYPE_NONE, 0);
    signals[CHANGED] =  gtk_signal_new ("changed",
                                        (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
                                        GTK_CLASS_TYPE(object_class),
                                        GTK_SIGNAL_OFFSET (SPGradientSelectorClass, changed),
                                        gtk_marshal_NONE__NONE,
                                        GTK_TYPE_NONE, 0);

    object_class->destroy = sp_gradient_selector_destroy;
}

static void
sp_gradient_selector_init (SPGradientSelector *sel)
{
    GtkWidget *hb, *l, *m, *mi;

    sel->mode = SP_GRADIENT_SELECTOR_MODE_LINEAR;

    sel->gradientUnits = SP_GRADIENT_UNITS_USERSPACEONUSE;
    sel->gradientSpread = SP_GRADIENT_SPREAD_PAD;

    /* Vectors */
    sel->vectors = sp_gradient_vector_selector_new (NULL, NULL);
    gtk_widget_show (sel->vectors);
    gtk_box_pack_start (GTK_BOX (sel), sel->vectors, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (sel->vectors), "vector_set", G_CALLBACK (sp_gradient_selector_vector_set), sel);

    /* Create box for buttons */
    hb = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (sel), hb, FALSE, FALSE, 0);
    GtkTooltips *ttips = gtk_tooltips_new ();

    sel->add = gtk_button_new_with_label (_("Duplicate"));
    gtk_box_pack_start (GTK_BOX (hb), sel->add, TRUE, TRUE, 0);
    g_signal_connect (G_OBJECT (sel->add), "clicked", G_CALLBACK (sp_gradient_selector_add_vector_clicked), sel);
    gtk_widget_set_sensitive (sel->add, FALSE);

    sel->edit = gtk_button_new_with_label (_("Edit..."));
    gtk_box_pack_start (GTK_BOX (hb), sel->edit, TRUE, TRUE, 0);
    g_signal_connect (G_OBJECT (sel->edit), "clicked", G_CALLBACK (sp_gradient_selector_edit_vector_clicked), sel);
    gtk_widget_set_sensitive (sel->edit, FALSE);

    gtk_widget_show_all (hb);

    /* Spread selector */
    hb = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hb);
    gtk_box_pack_start (GTK_BOX (sel), hb, FALSE, FALSE, 0);

    sel->spread = gtk_option_menu_new ();
    gtk_widget_show (sel->spread);
    gtk_box_pack_end (GTK_BOX (hb), sel->spread, FALSE, FALSE, 0);
    gtk_tooltips_set_tip (ttips, sel->spread,
                          // TRANSLATORS: for info, see http://www.w3.org/TR/2000/CR-SVG-20000802/pservers.html#LinearGradientSpreadMethodAttribute
                          _("Whether to fill with flat color beyond the ends of the gradient vector "
                            "(spreadMethod=\"pad\"), or repeat the gradient in the same direction "
                            "(spreadMethod=\"repeat\"), or repeat the gradient in alternating opposite "
                            "directions (spreadMethod=\"reflect\")"), NULL);

    m = gtk_menu_new ();
    mi = gtk_menu_item_new_with_label (_("none"));
    gtk_menu_append (GTK_MENU (m), mi);
    g_object_set_data (G_OBJECT (mi), "gradientSpread", GUINT_TO_POINTER (SP_GRADIENT_SPREAD_PAD));
    g_signal_connect (G_OBJECT (mi), "activate", G_CALLBACK (sp_gradient_selector_spread_activate), sel);
    mi = gtk_menu_item_new_with_label (_("reflected"));
    g_object_set_data (G_OBJECT (mi), "gradientSpread", GUINT_TO_POINTER (SP_GRADIENT_SPREAD_REFLECT));
    g_signal_connect (G_OBJECT (mi), "activate", G_CALLBACK (sp_gradient_selector_spread_activate), sel);
    gtk_menu_append (GTK_MENU (m), mi);
    mi = gtk_menu_item_new_with_label (_("direct"));
    g_object_set_data (G_OBJECT (mi), "gradientSpread", GUINT_TO_POINTER (SP_GRADIENT_SPREAD_REPEAT));
    g_signal_connect (G_OBJECT (mi), "activate", G_CALLBACK (sp_gradient_selector_spread_activate), sel);
    gtk_menu_append (GTK_MENU (m), mi);
    gtk_widget_show_all (m);

    gtk_option_menu_set_menu (GTK_OPTION_MENU (sel->spread), m);

    l = gtk_label_new (_("Repeat:"));
    gtk_widget_show (l);
    gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 4);
}

static void
sp_gradient_selector_destroy (GtkObject *object)
{
    SPGradientSelector *sel;

    sel = SP_GRADIENT_SELECTOR (object);

    if (((GtkObjectClass *) (parent_class))->destroy)
        (* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

GtkWidget *
sp_gradient_selector_new (void)
{
    SPGradientSelector *sel;

    sel = (SPGradientSelector*)gtk_type_new (SP_TYPE_GRADIENT_SELECTOR);

    return (GtkWidget *) sel;
}

void
sp_gradient_selector_set_mode (SPGradientSelector *sel, guint mode)
{
    g_return_if_fail (sel != NULL);
    g_return_if_fail (SP_IS_GRADIENT_SELECTOR (sel));

    sel->mode = mode;
}

void
sp_gradient_selector_set_units (SPGradientSelector *sel, guint units)
{
    g_return_if_fail (sel != NULL);
    g_return_if_fail (SP_IS_GRADIENT_SELECTOR (sel));

    sel->gradientUnits = (SPGradientUnits)units;
}

void
sp_gradient_selector_set_spread (SPGradientSelector *sel, guint spread)
{
    g_return_if_fail (sel != NULL);
    g_return_if_fail (SP_IS_GRADIENT_SELECTOR (sel));

    sel->gradientSpread = (SPGradientSpread)spread;

    gtk_option_menu_set_history (GTK_OPTION_MENU (sel->spread), sel->gradientSpread);
}

SPGradientUnits
sp_gradient_selector_get_units (SPGradientSelector *sel)
{
    return (SPGradientUnits) sel->gradientUnits;
}

SPGradientSpread
sp_gradient_selector_get_spread (SPGradientSelector *sel)
{
    return (SPGradientSpread) sel->gradientSpread;
}

void
sp_gradient_selector_set_vector (SPGradientSelector *sel, Document *doc, SPGradient *vector)
{
    g_return_if_fail (sel != NULL);
    g_return_if_fail (SP_IS_GRADIENT_SELECTOR (sel));
    g_return_if_fail (!vector || SP_IS_GRADIENT (vector));
    g_return_if_fail (!vector || (SP_OBJECT_DOCUMENT (vector) == doc));

    if (vector && !SP_GRADIENT_HAS_STOPS (vector))
        return;

    sp_gradient_vector_selector_set_gradient (SP_GRADIENT_VECTOR_SELECTOR (sel->vectors), doc, vector);

    if (vector) {
        gtk_widget_set_sensitive (sel->edit, TRUE);
        gtk_widget_set_sensitive (sel->add, TRUE);
    } else {
        gtk_widget_set_sensitive (sel->edit, FALSE);
        gtk_widget_set_sensitive (sel->add, (doc != NULL));
    }
}

SPGradient *
sp_gradient_selector_get_vector (SPGradientSelector *sel)
{
    if (sel == NULL || !SP_IS_GRADIENT_SELECTOR (sel))
        return NULL;

    /* fixme: */
    return SP_GRADIENT_VECTOR_SELECTOR (sel->vectors)->gr;
}

static void
sp_gradient_selector_vector_set (SPGradientVectorSelector */*gvs*/, SPGradient *gr, SPGradientSelector *sel)
{
    static gboolean blocked = FALSE;

    if (!blocked) {
        blocked = TRUE;
        gr = sp_gradient_ensure_vector_normalized (gr);
        sp_gradient_selector_set_vector (sel, (gr) ? SP_OBJECT_DOCUMENT (gr) : NULL, gr);
        g_signal_emit (G_OBJECT (sel), signals[CHANGED], 0, gr);
        blocked = FALSE;
    }
}

static void
sp_gradient_selector_edit_vector_clicked (GtkWidget */*w*/, SPGradientSelector *sel)
{
    GtkWidget *dialog;

    /* fixme: */
    dialog = sp_gradient_vector_editor_new (SP_GRADIENT_VECTOR_SELECTOR (sel->vectors)->gr);

    gtk_widget_show (dialog);
}

static void
sp_gradient_selector_add_vector_clicked (GtkWidget */*w*/, SPGradientSelector *sel)
{
    Document *doc = sp_gradient_vector_selector_get_document (
                                                                SP_GRADIENT_VECTOR_SELECTOR (sel->vectors));

    if (!doc)
        return;

    SPGradient *gr = sp_gradient_vector_selector_get_gradient(
                                                              SP_GRADIENT_VECTOR_SELECTOR (sel->vectors));
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

    Inkscape::XML::Node *repr = NULL;

    if (gr)
        repr = SP_OBJECT_REPR (gr)->duplicate(xml_doc);
    else {
        repr = xml_doc->createElement("svg:linearGradient");
        Inkscape::XML::Node *stop = xml_doc->createElement("svg:stop");
        stop->setAttribute("offset", "0");
        stop->setAttribute("style", "stop-color:#000;stop-opacity:1;");
        repr->appendChild(stop);
        Inkscape::GC::release(stop);
        stop = xml_doc->createElement("svg:stop");
        stop->setAttribute("offset", "1");
        stop->setAttribute("style", "stop-color:#fff;stop-opacity:1;");
        repr->appendChild(stop);
        Inkscape::GC::release(stop);
    }

    SP_OBJECT_REPR (SP_DOCUMENT_DEFS (doc))->addChild(repr, NULL);

    gr = (SPGradient *) doc->getObjectByRepr(repr);
    sp_gradient_vector_selector_set_gradient(
                                             SP_GRADIENT_VECTOR_SELECTOR (sel->vectors), doc, gr);

    Inkscape::GC::release(repr);
}



static void
sp_gradient_selector_spread_activate (GtkWidget *widget, SPGradientSelector *sel)
{
    sel->gradientSpread = (SPGradientSpread)GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (widget), "gradientSpread"));

    g_signal_emit (G_OBJECT (sel), signals[CHANGED], 0);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
