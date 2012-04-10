/*
 * Gradient vector widget
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2010 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <gtk/gtk.h>

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
static void sp_gradient_selector_spread_changed (GtkComboBox *widget, SPGradientSelector *sel);

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

    parent_class = (GtkVBoxClass*)g_type_class_peek_parent (klass);

    signals[GRABBED] =  g_signal_new ("grabbed",
                                        G_TYPE_FROM_CLASS(object_class),
                                        (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
                                        G_STRUCT_OFFSET (SPGradientSelectorClass, grabbed),
					NULL, NULL,
                                        g_cclosure_marshal_VOID__VOID,
                                        G_TYPE_NONE, 0);
    signals[DRAGGED] =  g_signal_new ("dragged",
                                        G_TYPE_FROM_CLASS(object_class),
                                        (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
                                        G_STRUCT_OFFSET (SPGradientSelectorClass, dragged),
					NULL, NULL,
                                        g_cclosure_marshal_VOID__VOID,
                                        G_TYPE_NONE, 0);
    signals[RELEASED] = g_signal_new ("released",
                                        G_TYPE_FROM_CLASS(object_class),
                                        (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
                                        G_STRUCT_OFFSET (SPGradientSelectorClass, released),
					NULL, NULL,
                                        g_cclosure_marshal_VOID__VOID,
                                        G_TYPE_NONE, 0);
    signals[CHANGED] =  g_signal_new ("changed",
                                        G_TYPE_FROM_CLASS(object_class),
                                        (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
                                        G_STRUCT_OFFSET (SPGradientSelectorClass, changed),
					NULL, NULL,
                                        g_cclosure_marshal_VOID__VOID,
                                        G_TYPE_NONE, 0);

    object_class->destroy = sp_gradient_selector_destroy;
}

static void sp_gradient_selector_init(SPGradientSelector *sel)
{
    sel->safelyInit = true;
    new (&sel->nonsolid) std::vector<GtkWidget*>();

    sel->mode = SPGradientSelector::MODE_LINEAR;

    sel->gradientUnits = SP_GRADIENT_UNITS_USERSPACEONUSE;
    sel->gradientSpread = SP_GRADIENT_SPREAD_PAD;

    /* Vectors */
    sel->vectors = sp_gradient_vector_selector_new (NULL, NULL);
    gtk_widget_show (sel->vectors);
    gtk_box_pack_start (GTK_BOX (sel), sel->vectors, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (sel->vectors), "vector_set", G_CALLBACK (sp_gradient_selector_vector_set), sel);

    /* Create box for buttons */
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
    GtkWidget *hb = gtk_hbox_new( FALSE, 0 );
#endif
    sel->nonsolid.push_back(hb);
    gtk_box_pack_start( GTK_BOX(sel), hb, FALSE, FALSE, 0 );

    sel->add = gtk_button_new_with_label (_("Duplicate"));
    sel->nonsolid.push_back(sel->add);
    gtk_box_pack_start (GTK_BOX (hb), sel->add, TRUE, TRUE, 0);
    g_signal_connect (G_OBJECT (sel->add), "clicked", G_CALLBACK (sp_gradient_selector_add_vector_clicked), sel);
    gtk_widget_set_sensitive (sel->add, FALSE);

    sel->edit = gtk_button_new_with_label (_("Edit..."));
    sel->nonsolid.push_back(sel->edit);
    gtk_box_pack_start (GTK_BOX (hb), sel->edit, TRUE, TRUE, 0);
    g_signal_connect (G_OBJECT (sel->edit), "clicked", G_CALLBACK (sp_gradient_selector_edit_vector_clicked), sel);
    gtk_widget_set_sensitive (sel->edit, FALSE);

    gtk_widget_show_all(hb);

    /* Spread selector */
#if GTK_CHECK_VERSION(3,0,0)
    hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
    hb = gtk_hbox_new( FALSE, 0 );
#endif
    sel->nonsolid.push_back(hb);
    gtk_widget_show(hb);
    gtk_box_pack_start( GTK_BOX(sel), hb, FALSE, FALSE, 0 );

// The GtkComboBoxText API only appeared in Gtk 2.24 but Inkscape supports
// builds for Gtk >= 2.20.
// Older versions need to use now-deprecated parts of
// the GtkComboBox API instead.
#if GTK_CHECK_VERSION(2,24,0)
    sel->spread = gtk_combo_box_text_new ();
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (sel->spread), _("none"));
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (sel->spread), _("reflected"));
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (sel->spread), _("direct"));
#else
    sel->spread = gtk_combo_box_new_text ();
    gtk_combo_box_append_text (GTK_COMBO_BOX (sel->spread), _("none"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (sel->spread), _("reflected"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (sel->spread), _("direct"));
#endif

    sel->nonsolid.push_back(sel->spread);
    gtk_widget_show(sel->spread);
    gtk_box_pack_end( GTK_BOX(hb), sel->spread, FALSE, FALSE, 0 );
    gtk_widget_set_tooltip_text( sel->spread,
                          // TRANSLATORS: for info, see http://www.w3.org/TR/2000/CR-SVG-20000802/pservers.html#LinearGradientSpreadMethodAttribute
                          _("Whether to fill with flat color beyond the ends of the gradient vector "
                            "(spreadMethod=\"pad\"), or repeat the gradient in the same direction "
                            "(spreadMethod=\"repeat\"), or repeat the gradient in alternating opposite "
                            "directions (spreadMethod=\"reflect\")"));

    g_signal_connect (G_OBJECT (sel->spread), "changed", 
		    G_CALLBACK (sp_gradient_selector_spread_changed), sel);

    sel->spreadLbl = gtk_label_new( _("Repeat:") );
    sel->nonsolid.push_back(sel->spreadLbl);
    gtk_widget_show( sel->spreadLbl );
    gtk_box_pack_end( GTK_BOX(hb), sel->spreadLbl, FALSE, FALSE, 4 );
}

static void sp_gradient_selector_destroy(GtkObject *object)
{
    SPGradientSelector *sel = SP_GRADIENT_SELECTOR( object );

    if ( sel->safelyInit ) {
        sel->safelyInit = false;
        using std::vector;
        sel->nonsolid.~vector<GtkWidget*>();
    }

    if (((GtkObjectClass *) (parent_class))->destroy) {
        (* ((GtkObjectClass *) (parent_class))->destroy) (object);
    }
}

GtkWidget *
sp_gradient_selector_new (void)
{
    SPGradientSelector *sel;

    sel = (SPGradientSelector*)g_object_new (SP_TYPE_GRADIENT_SELECTOR, NULL);

    return (GtkWidget *) sel;
}

void SPGradientSelector::setMode(SelectorMode mode)
{
    if (mode != this->mode) {
        this->mode = mode;
        if (mode == MODE_SWATCH) {
            for (std::vector<GtkWidget*>::iterator it = nonsolid.begin(); it != nonsolid.end(); ++it)
            {
                gtk_widget_hide(*it);
            }

            SPGradientVectorSelector* vs = SP_GRADIENT_VECTOR_SELECTOR(vectors);
            vs->setSwatched();
        }
    }
}

void SPGradientSelector::setUnits(SPGradientUnits units)
{
    gradientUnits = units;
}

void SPGradientSelector::setSpread(SPGradientSpread spread)
{
    gradientSpread = spread;
    gtk_combo_box_set_active (GTK_COMBO_BOX(this->spread), gradientSpread);
}

SPGradientUnits SPGradientSelector::getUnits()
{
    return gradientUnits;
}

SPGradientSpread SPGradientSelector::getSpread()
{
    return gradientSpread;
}

void SPGradientSelector::setVector(SPDocument *doc, SPGradient *vector)
{
    g_return_if_fail(!vector || SP_IS_GRADIENT(vector));
    g_return_if_fail(!vector || (vector->document == doc));

    if (vector && !vector->hasStops()) {
        return;
    }

    sp_gradient_vector_selector_set_gradient(SP_GRADIENT_VECTOR_SELECTOR(vectors), doc, vector);

    if (vector) {
        if ( (mode == MODE_SWATCH) && vector->isSwatch() ) {
            if ( vector->isSolid() ) {
                for (std::vector<GtkWidget*>::iterator it = nonsolid.begin(); it != nonsolid.end(); ++it)
                {
                    gtk_widget_hide(*it);
                }
            } else {
                for (std::vector<GtkWidget*>::iterator it = nonsolid.begin(); it != nonsolid.end(); ++it)
                {
                    gtk_widget_show_all(*it);
                }
            }
        }

        if (edit) {
            gtk_widget_set_sensitive(edit, TRUE);
        }
        if (add) {
            gtk_widget_set_sensitive(add, TRUE);
        }
    } else {
        if (edit) {
            gtk_widget_set_sensitive(edit, FALSE);
        }
        if (add) {
            gtk_widget_set_sensitive(add, (doc != NULL));
        }
    }
}

SPGradient *SPGradientSelector::getVector()
{
    /* fixme: */
    return SP_GRADIENT_VECTOR_SELECTOR(vectors)->gr;
}

static void
sp_gradient_selector_vector_set (SPGradientVectorSelector */*gvs*/, SPGradient *gr, SPGradientSelector *sel)
{
    static gboolean blocked = FALSE;

    if (!blocked) {
        blocked = TRUE;
        gr = sp_gradient_ensure_vector_normalized (gr);
        sel->setVector((gr) ? gr->document : 0, gr);
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
    SPDocument *doc = sp_gradient_vector_selector_get_document (
                                                                SP_GRADIENT_VECTOR_SELECTOR (sel->vectors));

    if (!doc)
        return;

    SPGradient *gr = sp_gradient_vector_selector_get_gradient(
                                                              SP_GRADIENT_VECTOR_SELECTOR (sel->vectors));
    Inkscape::XML::Document *xml_doc = doc->getReprDoc();

    Inkscape::XML::Node *repr = NULL;

    if (gr) {
        repr = gr->getRepr()->duplicate(xml_doc);
    } else {
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

    doc->getDefs()->getRepr()->addChild(repr, NULL);

    gr = static_cast<SPGradient *>(doc->getObjectByRepr(repr));
    sp_gradient_vector_selector_set_gradient(
                                             SP_GRADIENT_VECTOR_SELECTOR (sel->vectors), doc, gr);

    Inkscape::GC::release(repr);
}

static void
sp_gradient_selector_spread_changed (GtkComboBox *widget, SPGradientSelector *sel)
{
	sel->gradientSpread = (SPGradientSpread) gtk_combo_box_get_active (GTK_COMBO_BOX(widget));
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
