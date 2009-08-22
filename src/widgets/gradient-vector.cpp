/*
 * Gradient vector selection widget
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2004 Monash University
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2006 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#ifdef HAVE_STRING_H
#endif
#include <gtk/gtk.h>
#include "macros.h"
#include <glibmm/i18n.h>
#include "../widgets/gradient-image.h"
#include "../inkscape.h"
#include "../document-private.h"
#include "../gradient-chemistry.h"
#include "gradient-vector.h"
#include "../helper/window.h"

#include "xml/repr.h"

#include "../dialogs/dialog-events.h"
#include "../preferences.h"
#include "svg/css-ostringstream.h"
#include "sp-stop.h"

#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>

enum {
    VECTOR_SET,
    LAST_SIGNAL
};

static void sp_gradient_vector_selector_class_init (SPGradientVectorSelectorClass *klass);
static void sp_gradient_vector_selector_init (SPGradientVectorSelector *gvs);
static void sp_gradient_vector_selector_destroy (GtkObject *object);

static void sp_gvs_gradient_release (SPObject *obj, SPGradientVectorSelector *gvs);
static void sp_gvs_defs_release (SPObject *defs, SPGradientVectorSelector *gvs);
static void sp_gvs_defs_modified (SPObject *defs, guint flags, SPGradientVectorSelector *gvs);

static void sp_gvs_rebuild_gui_full (SPGradientVectorSelector *gvs);
static void sp_gvs_gradient_activate (GtkMenuItem *mi, SPGradientVectorSelector *gvs);

static GtkVBoxClass *parent_class;
static guint signals[LAST_SIGNAL] = {0};

// TODO FIXME kill these globals!!!
static GtkWidget *dlg = NULL;
static win_data wd;
static gint x = -1000, y = -1000, w = 0, h = 0; // impossible original values to make sure they are read from prefs
static Glib::ustring const prefs_path = "/dialogs/gradienteditor/";

GType sp_gradient_vector_selector_get_type(void)
{
    static GType type = 0;
    if (!type) {
        static const GTypeInfo info = {
            sizeof(SPGradientVectorSelectorClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) sp_gradient_vector_selector_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof(SPGradientVectorSelector),
            0,    /* n_preallocs */
            (GInstanceInitFunc) sp_gradient_vector_selector_init,
            0,    /* value_table */
        };

        type = g_type_register_static( GTK_TYPE_VBOX,
                                       "SPGradientVectorSelector",
                                       &info,
                                       static_cast< GTypeFlags > (0) );
    }
    return type;
}

static void
sp_gradient_vector_selector_class_init (SPGradientVectorSelectorClass *klass)
{
    GtkObjectClass *object_class;

    object_class = GTK_OBJECT_CLASS (klass);

    parent_class = (GtkVBoxClass*)gtk_type_class (GTK_TYPE_VBOX);

    signals[VECTOR_SET] = gtk_signal_new ("vector_set",
                                          GTK_RUN_LAST,
                                          GTK_CLASS_TYPE(object_class),
                                          GTK_SIGNAL_OFFSET (SPGradientVectorSelectorClass, vector_set),
                                          gtk_marshal_NONE__POINTER,
                                          GTK_TYPE_NONE, 1,
                                          GTK_TYPE_POINTER);

    object_class->destroy = sp_gradient_vector_selector_destroy;
}

static void
sp_gradient_vector_selector_init (SPGradientVectorSelector *gvs)
{
    gvs->idlabel = TRUE;

    gvs->doc = NULL;
    gvs->gr = NULL;

    new (&gvs->gradient_release_connection) sigc::connection();
    new (&gvs->defs_release_connection) sigc::connection();
    new (&gvs->defs_modified_connection) sigc::connection();

    gvs->menu = gtk_option_menu_new ();
    gtk_widget_show (gvs->menu);
    gtk_box_pack_start (GTK_BOX (gvs), gvs->menu, TRUE, TRUE, 0);
}

static void
sp_gradient_vector_selector_destroy (GtkObject *object)
{
    SPGradientVectorSelector *gvs;

    gvs = SP_GRADIENT_VECTOR_SELECTOR (object);

    if (gvs->gr) {
        gvs->gradient_release_connection.disconnect();
        gvs->gr = NULL;
    }

    if (gvs->doc) {
        gvs->defs_release_connection.disconnect();
        gvs->defs_modified_connection.disconnect();
        gvs->doc = NULL;
    }

    gvs->gradient_release_connection.~connection();
    gvs->defs_release_connection.~connection();
    gvs->defs_modified_connection.~connection();

    if (((GtkObjectClass *) (parent_class))->destroy)
        (* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

GtkWidget *
sp_gradient_vector_selector_new (SPDocument *doc, SPGradient *gr)
{
    GtkWidget *gvs;

    g_return_val_if_fail (!gr || SP_IS_GRADIENT (gr), NULL);
    g_return_val_if_fail (!gr || (SP_OBJECT_DOCUMENT (gr) == doc), NULL);

    gvs = (GtkWidget*)gtk_type_new (SP_TYPE_GRADIENT_VECTOR_SELECTOR);

    if (doc) {
        sp_gradient_vector_selector_set_gradient (SP_GRADIENT_VECTOR_SELECTOR (gvs), doc, gr);
    } else {
        sp_gvs_rebuild_gui_full (SP_GRADIENT_VECTOR_SELECTOR (gvs));
    }

    return gvs;
}

void
sp_gradient_vector_selector_set_gradient (SPGradientVectorSelector *gvs, SPDocument *doc, SPGradient *gr)
{
    static gboolean suppress = FALSE;

    g_return_if_fail (gvs != NULL);
    g_return_if_fail (SP_IS_GRADIENT_VECTOR_SELECTOR (gvs));
    g_return_if_fail (!gr || (doc != NULL));
    g_return_if_fail (!gr || SP_IS_GRADIENT (gr));
    g_return_if_fail (!gr || (SP_OBJECT_DOCUMENT (gr) == doc));
    g_return_if_fail (!gr || SP_GRADIENT_HAS_STOPS (gr));

    if (doc != gvs->doc) {
        /* Disconnect signals */
        if (gvs->gr) {
            gvs->gradient_release_connection.disconnect();
            gvs->gr = NULL;
        }
        if (gvs->doc) {
            gvs->defs_release_connection.disconnect();
            gvs->defs_modified_connection.disconnect();
            gvs->doc = NULL;
        }
        /* Connect signals */
        if (doc) {
            gvs->defs_release_connection = SP_DOCUMENT_DEFS(doc)->connectRelease(sigc::bind<1>(sigc::ptr_fun(&sp_gvs_defs_release), gvs));
            gvs->defs_modified_connection = SP_DOCUMENT_DEFS(doc)->connectModified(sigc::bind<2>(sigc::ptr_fun(&sp_gvs_defs_modified), gvs));
        }
        if (gr) {
            gvs->gradient_release_connection = gr->connectRelease(sigc::bind<1>(sigc::ptr_fun(&sp_gvs_gradient_release), gvs));
        }
        gvs->doc = doc;
        gvs->gr = gr;
        sp_gvs_rebuild_gui_full (gvs);
        if (!suppress) g_signal_emit (G_OBJECT (gvs), signals[VECTOR_SET], 0, gr);
    } else if (gr != gvs->gr) {
        /* Harder case - keep document, rebuild menus and stuff */
        /* fixme: (Lauris) */
        suppress = TRUE;
        sp_gradient_vector_selector_set_gradient (gvs, NULL, NULL);
        sp_gradient_vector_selector_set_gradient (gvs, doc, gr);
        suppress = FALSE;
        g_signal_emit (G_OBJECT (gvs), signals[VECTOR_SET], 0, gr);
    }
    /* The case of setting NULL -> NULL is not very interesting */
}

SPDocument *
sp_gradient_vector_selector_get_document (SPGradientVectorSelector *gvs)
{
    g_return_val_if_fail (gvs != NULL, NULL);
    g_return_val_if_fail (SP_IS_GRADIENT_VECTOR_SELECTOR (gvs), NULL);

    return gvs->doc;
}

SPGradient *
sp_gradient_vector_selector_get_gradient (SPGradientVectorSelector *gvs)
{
    g_return_val_if_fail (gvs != NULL, NULL);
    g_return_val_if_fail (SP_IS_GRADIENT_VECTOR_SELECTOR (gvs), NULL);

    return gvs->gr;
}

static void
sp_gvs_rebuild_gui_full (SPGradientVectorSelector *gvs)
{
    /* Clear old menu, if there is any */
    if (gtk_option_menu_get_menu (GTK_OPTION_MENU (gvs->menu))) {
        gtk_option_menu_remove_menu (GTK_OPTION_MENU (gvs->menu));
    }

    /* Create new menu widget */
    GtkWidget *m = gtk_menu_new ();
    gtk_widget_show (m);

    /* Pick up all gradients with vectors */
    GSList *gl = NULL;
    if (gvs->gr) {
        const GSList *gradients = sp_document_get_resource_list (SP_OBJECT_DOCUMENT (gvs->gr), "gradient");
        for (const GSList *l = gradients; l != NULL; l = l->next) {
            if (SP_GRADIENT_HAS_STOPS (l->data)) {
                gl = g_slist_prepend (gl, l->data);
            }
        }
    }
    gl = g_slist_reverse (gl);

    gint pos = 0;
    gint idx = 0;

    if (!gvs->doc) {
        GtkWidget *i;
        i = gtk_menu_item_new_with_label (_("No document selected"));
        gtk_widget_show (i);
        gtk_menu_append (GTK_MENU (m), i);
        gtk_widget_set_sensitive (gvs->menu, FALSE);
    } else if (!gl) {
        GtkWidget *i;
        i = gtk_menu_item_new_with_label (_("No gradients in document"));
        gtk_widget_show (i);
        gtk_menu_append (GTK_MENU (m), i);
        gtk_widget_set_sensitive (gvs->menu, FALSE);
    } else if (!gvs->gr) {
        GtkWidget *i;
        i = gtk_menu_item_new_with_label (_("No gradient selected"));
        gtk_widget_show (i);
        gtk_menu_append (GTK_MENU (m), i);
        gtk_widget_set_sensitive (gvs->menu, FALSE);
    } else {
        while (gl) {
            SPGradient *gr;
            GtkWidget *i, *w;
            gr = SP_GRADIENT (gl->data);
            gl = g_slist_remove (gl, gr);

            /* We have to know: */
            /* Gradient destroy */
            /* Gradient name change */
            i = gtk_menu_item_new ();
            gtk_widget_show (i);
            g_object_set_data (G_OBJECT (i), "gradient", gr);
            g_signal_connect (G_OBJECT (i), "activate", G_CALLBACK (sp_gvs_gradient_activate), gvs);

            w = sp_gradient_image_new (gr);
            gtk_widget_show (w);

            if (gvs->idlabel) {
                GtkWidget *hb, *l;
                hb = gtk_hbox_new (FALSE, 4);
                gtk_widget_show (hb);
                l = gtk_label_new (SP_OBJECT_ID (gr));
                gtk_widget_show (l);
                gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
                gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
                gtk_box_pack_start (GTK_BOX (hb), w, FALSE, FALSE, 0);
                w = hb;
            }

            gtk_container_add (GTK_CONTAINER (i), w);

            gtk_menu_append (GTK_MENU (m), i);

            if (gr == gvs->gr) pos = idx;
            idx += 1;
        }
        gtk_widget_set_sensitive (gvs->menu, TRUE);
    }

    gtk_option_menu_set_menu (GTK_OPTION_MENU (gvs->menu), m);
    /* Set history */
    gtk_option_menu_set_history (GTK_OPTION_MENU (gvs->menu), pos);
}

static void
sp_gvs_gradient_activate (GtkMenuItem *mi, SPGradientVectorSelector *gvs)
{
    SPGradient *gr, *norm;

    gr = (SPGradient*)g_object_get_data (G_OBJECT (mi), "gradient");
    /* Hmmm... bad things may happen here, if actual gradient is something new */
    /* Namely - menuitems etc. will be fucked up */
    /* Hmmm - probably we can just re-set it as menuitem data (Lauris) */

    //g_print ("SPGradientVectorSelector: gradient %s activated\n", SP_OBJECT_ID (gr));

    norm = sp_gradient_ensure_vector_normalized (gr);
    if (norm != gr) {
        //g_print ("SPGradientVectorSelector: become %s after normalization\n", SP_OBJECT_ID (norm));
        /* But be careful that we do not have gradient saved anywhere else */
        g_object_set_data (G_OBJECT (mi), "gradient", norm);
    }

    /* fixme: Really we would want to use _set_vector */
    /* Detach old */
    if (gvs->gr) {
        gvs->gradient_release_connection.disconnect();
        gvs->gr = NULL;
    }
    /* Attach new */
    if (norm) {
        gvs->gradient_release_connection = norm->connectRelease(sigc::bind<1>(sigc::ptr_fun(&sp_gvs_gradient_release), gvs));
        gvs->gr = norm;
    }

    g_signal_emit (G_OBJECT (gvs), signals[VECTOR_SET], 0, norm);

    if (norm != gr) {
        /* We do extra undo push here */
        /* If handler has already done it, it is just NOP */
        // FIXME: looks like this is never a valid undo step, consider removing this
        sp_document_done (SP_OBJECT_DOCUMENT (norm), SP_VERB_CONTEXT_GRADIENT,
                          /* TODO: annotate */ "gradient-vector.cpp:350");
    }
}

static void
sp_gvs_gradient_release (SPObject */*obj*/, SPGradientVectorSelector *gvs)
{
    /* Disconnect gradient */
    if (gvs->gr) {
        gvs->gradient_release_connection.disconnect();
        gvs->gr = NULL;
    }

    /* Rebuild GUI */
    sp_gvs_rebuild_gui_full (gvs);
}

static void
sp_gvs_defs_release (SPObject */*defs*/, SPGradientVectorSelector *gvs)
{
    gvs->doc = NULL;

    gvs->defs_release_connection.disconnect();
    gvs->defs_modified_connection.disconnect();

    /* Disconnect gradient as well */
    if (gvs->gr) {
        gvs->gradient_release_connection.disconnect();
        gvs->gr = NULL;
    }

    /* Rebuild GUI */
    sp_gvs_rebuild_gui_full (gvs);
}

static void
sp_gvs_defs_modified (SPObject */*defs*/, guint /*flags*/, SPGradientVectorSelector *gvs)
{
    /* fixme: We probably have to check some flags here (Lauris) */

    sp_gvs_rebuild_gui_full (gvs);
}

/*##################################################################
  ###                 Vector Editing Widget
  ##################################################################*/

#include "../widgets/sp-color-notebook.h"
#include "../widgets/sp-color-preview.h"
#include "../widgets/widget-sizes.h"
#include "../xml/node-event-vector.h"
#include "../svg/svg-color.h"


#define PAD 4

static GtkWidget *sp_gradient_vector_widget_new (SPGradient *gradient, SPStop *stop);

static void sp_gradient_vector_widget_load_gradient (GtkWidget *widget, SPGradient *gradient);
static gint sp_gradient_vector_dialog_delete (GtkWidget *widget, GdkEvent *event, GtkWidget *dialog);
static void sp_gradient_vector_dialog_destroy (GtkObject *object, gpointer data);

static void sp_gradient_vector_widget_destroy (GtkObject *object, gpointer data);
static void sp_gradient_vector_gradient_release (SPObject *obj, GtkWidget *widget);
static void sp_gradient_vector_gradient_modified (SPObject *obj, guint flags, GtkWidget *widget);
static void sp_gradient_vector_color_dragged (SPColorSelector *csel, GtkObject *object);
static void sp_gradient_vector_color_changed (SPColorSelector *csel, GtkObject *object);
static void update_stop_list( GtkWidget *mnu, SPGradient *gradient, SPStop *new_stop);

static gboolean blocked = FALSE;

static void grad_edit_dia_stop_added_or_removed (Inkscape::XML::Node */*repr*/, Inkscape::XML::Node */*child*/, Inkscape::XML::Node */*ref*/, gpointer data)
{
    GtkWidget *vb = GTK_WIDGET(data);
    GtkWidget *mnu = (GtkWidget *)g_object_get_data (G_OBJECT(vb), "stopmenu");
    SPGradient *gradient = (SPGradient *)g_object_get_data (G_OBJECT(vb), "gradient");
    update_stop_list (mnu, gradient, NULL);
}

//FIXME!!! We must also listen to attr changes on all children (i.e. stops) too,
//otherwise the dialog does not reflect undoing color or offset change. This is a major
//hassle, unless we have a "one of the descendants changed in some way" signal.
static Inkscape::XML::NodeEventVector grad_edit_dia_repr_events =
{
    grad_edit_dia_stop_added_or_removed, /* child_added */
    grad_edit_dia_stop_added_or_removed, /* child_removed */
    NULL, /* attr_changed*/
    NULL, /* content_changed */
    NULL  /* order_changed */
};

static void
verify_grad(SPGradient *gradient)
{
    int i = 0;
    SPStop *stop = NULL;
    /* count stops */
    for ( SPObject *ochild = sp_object_first_child(SP_OBJECT(gradient)) ; ochild != NULL ; ochild = SP_OBJECT_NEXT(ochild) ) {
        if (SP_IS_STOP (ochild)) {
            i++;
            stop = SP_STOP(ochild);
        }
    }

    Inkscape::XML::Document *xml_doc;
    xml_doc = SP_OBJECT_REPR(gradient)->document();

    if (i < 1) {
        gchar c[64];
        sp_svg_write_color (c, sizeof(c), 0x00000000);

        Inkscape::CSSOStringStream os;
        os << "stop-color:" << c << ";stop-opacity:" << 1.0 << ";";

        Inkscape::XML::Node *child;

        child = xml_doc->createElement("svg:stop");
        sp_repr_set_css_double(child, "offset", 0.0);
        child->setAttribute("style", os.str().c_str());
        SP_OBJECT_REPR (gradient)->addChild(child, NULL);
        Inkscape::GC::release(child);

        child = xml_doc->createElement("svg:stop");
        sp_repr_set_css_double(child, "offset", 1.0);
        child->setAttribute("style", os.str().c_str());
        SP_OBJECT_REPR (gradient)->addChild(child, NULL);
        Inkscape::GC::release(child);
    }
    if (i < 2) {
        sp_repr_set_css_double(SP_OBJECT_REPR(stop), "offset", 0.0);
        Inkscape::XML::Node *child = SP_OBJECT_REPR(stop)->duplicate(SP_OBJECT_REPR(gradient)->document());
        sp_repr_set_css_double(child, "offset", 1.0);
        SP_OBJECT_REPR(gradient)->addChild(child, SP_OBJECT_REPR (stop));
        Inkscape::GC::release(child);
    }
}

static void
select_stop_in_list( GtkWidget *mnu, SPGradient *gradient, SPStop *new_stop)
{
    int i = 0;
    for ( SPObject *ochild = sp_object_first_child(SP_OBJECT(gradient)) ; ochild != NULL ; ochild = SP_OBJECT_NEXT(ochild) ) {
        if (SP_IS_STOP (ochild)) {
            if (SP_OBJECT (ochild) == SP_OBJECT(new_stop)) {
                gtk_option_menu_set_history (GTK_OPTION_MENU (mnu), i);
                break;
            }
            i++;
        }
    }
}

static void
update_stop_list( GtkWidget *mnu, SPGradient *gradient, SPStop *new_stop)
{

    if (!SP_IS_GRADIENT (gradient))
        return;

    blocked = TRUE;

    /* Clear old menu, if there is any */
    if (gtk_option_menu_get_menu (GTK_OPTION_MENU (mnu))) {
        gtk_option_menu_remove_menu (GTK_OPTION_MENU (mnu));
    }

    /* Create new menu widget */
    GtkWidget *m = gtk_menu_new ();
    gtk_widget_show (m);
    GSList *sl = NULL;
    if (gradient->has_stops) {
        for ( SPObject *ochild = sp_object_first_child (SP_OBJECT(gradient)) ; ochild != NULL ; ochild = SP_OBJECT_NEXT(ochild) ) {
            if (SP_IS_STOP (ochild)) {
                sl = g_slist_append (sl, ochild);
            }
        }
    }
    if (!sl) {
        GtkWidget *i = gtk_menu_item_new_with_label (_("No stops in gradient"));
        gtk_widget_show (i);
        gtk_menu_append (GTK_MENU (m), i);
        gtk_widget_set_sensitive (mnu, FALSE);
    } else {

        for (; sl != NULL; sl = sl->next){
            SPStop *stop;
            GtkWidget *i;
            if (SP_IS_STOP(sl->data)){
                stop = SP_STOP (sl->data);
                i = gtk_menu_item_new ();
                gtk_widget_show (i);
                g_object_set_data (G_OBJECT (i), "stop", stop);
                GtkWidget *hb = gtk_hbox_new (FALSE, 4);
                GtkWidget *cpv = sp_color_preview_new(sp_stop_get_rgba32(stop));
                gtk_widget_show (cpv);
                gtk_container_add ( GTK_CONTAINER (hb), cpv );
                g_object_set_data ( G_OBJECT (i), "preview", cpv );
                Inkscape::XML::Node *repr = SP_OBJECT_REPR((SPItem *) sl->data);
                GtkWidget *l = gtk_label_new (repr->attribute("id"));
                gtk_widget_show (l);
                gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
                gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
                gtk_widget_show (hb);
                gtk_container_add (GTK_CONTAINER (i), hb);
                gtk_menu_append (GTK_MENU (m), i);
            }
        }

        gtk_widget_set_sensitive (mnu, TRUE);
    }
    gtk_option_menu_set_menu (GTK_OPTION_MENU (mnu), m);

    /* Set history */
    if (new_stop == NULL) {
        gtk_option_menu_set_history (GTK_OPTION_MENU (mnu), 0);
    } else {
        select_stop_in_list (mnu, gradient, new_stop);
    }

    blocked = FALSE;
}


/*user selected existing stop from list*/
static void
sp_grad_edit_select (GtkOptionMenu *mnu,  GtkWidget *tbl)
{
    SPGradient *gradient = (SPGradient *)g_object_get_data (G_OBJECT(tbl), "gradient");

    GObject *item = G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu))));
    SPStop *stop = SP_STOP (g_object_get_data (item, "stop"));
    if (!stop) return;

    blocked = TRUE;

    SPColorSelector *csel = (SPColorSelector*)g_object_get_data (G_OBJECT (tbl), "cselector");
    guint32 const c = sp_stop_get_rgba32(stop);
    csel->base->setAlpha(SP_RGBA32_A_F (c));
    SPColor color( SP_RGBA32_R_F (c), SP_RGBA32_G_F (c), SP_RGBA32_B_F (c) );
    // set its color, from the stored array
    csel->base->setColor( color );
    GtkWidget *offspin = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "offspn"));
    GtkWidget *offslide =GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "offslide"));

    GtkAdjustment *adj = (GtkAdjustment*)gtk_object_get_data (GTK_OBJECT (tbl), "offset");

    bool isEndStop = false;

    SPStop *prev = NULL;
    prev = sp_prev_stop(stop, gradient);
    if (prev != NULL )  {
        adj->lower = prev->offset;
    } else {
        isEndStop = true;
        adj->lower = 0;
    }

    SPStop *next = NULL;
    next = sp_next_stop(stop);
    if (next != NULL ) {
        adj->upper = next->offset;
    } else {
        isEndStop = true;
        adj->upper = 1.0;
    }

    //fixme: does this work on all possible input gradients?
    if (!isEndStop) {
        gtk_widget_set_sensitive (offslide, TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET (offspin), TRUE);
    } else {
        gtk_widget_set_sensitive (offslide, FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (offspin), FALSE);
    }

    gtk_adjustment_set_value (adj, stop->offset);

    gtk_adjustment_changed (adj);

    blocked = FALSE;
}




static void
offadjustmentChanged( GtkAdjustment *adjustment, GtkWidget *vb)
{
    if (blocked)
        return;

    blocked = TRUE;

    GtkOptionMenu *mnu = (GtkOptionMenu *)g_object_get_data (G_OBJECT(vb), "stopmenu");
    if (!g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "stop")) return;
    SPStop *stop = SP_STOP(g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "stop"));

    stop->offset = adjustment->value;
    sp_repr_set_css_double(SP_OBJECT_REPR(stop), "offset", stop->offset);

    sp_document_done (SP_OBJECT_DOCUMENT (stop), SP_VERB_CONTEXT_GRADIENT,
                      _("Change gradient stop offset"));

    blocked = FALSE;
}

guint32
sp_average_color (guint32 c1, guint32 c2, gdouble p = 0.5)
{
    guint32 r = (guint32) (SP_RGBA32_R_U (c1) * p + SP_RGBA32_R_U (c2) * (1 - p));
    guint32 g = (guint32) (SP_RGBA32_G_U (c1) * p + SP_RGBA32_G_U (c2) * (1 - p));
    guint32 b = (guint32) (SP_RGBA32_B_U (c1) * p + SP_RGBA32_B_U (c2) * (1 - p));
    guint32 a = (guint32) (SP_RGBA32_A_U (c1) * p + SP_RGBA32_A_U (c2) * (1 - p));

    return SP_RGBA32_U_COMPOSE (r, g, b, a);
}


static void
sp_grd_ed_add_stop (GtkWidget */*widget*/,  GtkWidget *vb)
{
    SPGradient *gradient = (SPGradient *) g_object_get_data (G_OBJECT(vb), "gradient");
    verify_grad (gradient);
    GtkOptionMenu *mnu = (GtkOptionMenu *)g_object_get_data (G_OBJECT(vb), "stopmenu");

    SPStop *stop = (SPStop *) g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "stop");

    if (stop == NULL)
        return;

    Inkscape::XML::Node *new_stop_repr = NULL;

    SPStop *next = sp_next_stop (stop);

    if (next == NULL) {
        SPStop *prev = sp_prev_stop (stop, gradient);
        if (prev != NULL) {
            next = stop;
            stop = prev;
        }
    }

    if (next != NULL) {
        new_stop_repr = SP_OBJECT_REPR(stop)->duplicate(SP_OBJECT_REPR(gradient)->document());
        SP_OBJECT_REPR(gradient)->addChild(new_stop_repr, SP_OBJECT_REPR(stop));
    } else {
        next = stop;
        new_stop_repr = SP_OBJECT_REPR(sp_prev_stop(stop, gradient))->duplicate(SP_OBJECT_REPR(gradient)->document());
        SP_OBJECT_REPR(gradient)->addChild(new_stop_repr, SP_OBJECT_REPR(sp_prev_stop(stop, gradient)));
    }

    SPStop *newstop = (SPStop *) SP_OBJECT_DOCUMENT(gradient)->getObjectByRepr(new_stop_repr);

    newstop->offset = (stop->offset + next->offset) * 0.5 ;

    guint32 const c1 = sp_stop_get_rgba32(stop);
    guint32 const c2 = sp_stop_get_rgba32(next);
    guint32 cnew = sp_average_color (c1, c2);

    Inkscape::CSSOStringStream os;
    gchar c[64];
    sp_svg_write_color (c, sizeof(c), cnew);
    gdouble opacity = (gdouble) SP_RGBA32_A_F (cnew);
    os << "stop-color:" << c << ";stop-opacity:" << opacity <<";";
    SP_OBJECT_REPR (newstop)->setAttribute("style", os.str().c_str());
    sp_repr_set_css_double( SP_OBJECT_REPR(newstop), "offset", (double)newstop->offset);

    sp_gradient_vector_widget_load_gradient (vb, gradient);
    Inkscape::GC::release(new_stop_repr);
    update_stop_list(GTK_WIDGET(mnu), gradient, newstop);
    GtkWidget *offspin = GTK_WIDGET (g_object_get_data (G_OBJECT (vb), "offspn"));
    GtkWidget *offslide =GTK_WIDGET (g_object_get_data (G_OBJECT (vb), "offslide"));
    gtk_widget_set_sensitive (offslide, TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET (offspin), TRUE);
    sp_document_done (SP_OBJECT_DOCUMENT (gradient), SP_VERB_CONTEXT_GRADIENT,
                      _("Add gradient stop"));
}

static void
sp_grd_ed_del_stop (GtkWidget */*widget*/,  GtkWidget *vb)
{
    SPGradient *gradient = (SPGradient *)g_object_get_data (G_OBJECT(vb), "gradient");

    GtkOptionMenu *mnu = (GtkOptionMenu *)g_object_get_data (G_OBJECT(vb), "stopmenu");
    if (!g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "stop")) return;
    SPStop *stop = SP_STOP(g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "stop"));
    if (gradient->vector.stops.size() > 2) { // 2 is the minimum

        // if we delete first or last stop, move the next/previous to the edge
        if (stop->offset == 0) {
            SPStop *next = sp_next_stop (stop);
            if (next) {
                next->offset = 0;
                sp_repr_set_css_double (SP_OBJECT_REPR (next), "offset", 0);
            }
        } else if (stop->offset == 1) {
            SPStop *prev = sp_prev_stop (stop, gradient);
            if (prev) {
                prev->offset = 1;
                sp_repr_set_css_double (SP_OBJECT_REPR (prev), "offset", 1);
            }
        }

        SP_OBJECT_REPR(gradient)->removeChild(SP_OBJECT_REPR(stop));
        sp_gradient_vector_widget_load_gradient (vb, gradient);
        update_stop_list(GTK_WIDGET(mnu), gradient, NULL);
        sp_document_done (SP_OBJECT_DOCUMENT (gradient), SP_VERB_CONTEXT_GRADIENT,
                          _("Delete gradient stop"));
    }

}

static GtkWidget *
sp_gradient_vector_widget_new (SPGradient *gradient, SPStop *select_stop)
{
    GtkWidget *vb, *w, *f, *csel;

    g_return_val_if_fail (!gradient || SP_IS_GRADIENT (gradient), NULL);

    vb = gtk_vbox_new (FALSE, PAD);
    g_signal_connect (G_OBJECT (vb), "destroy", G_CALLBACK (sp_gradient_vector_widget_destroy), NULL);

    w = sp_gradient_image_new (gradient);
    g_object_set_data (G_OBJECT (vb), "preview", w);
    gtk_widget_show (w);
    gtk_box_pack_start (GTK_BOX (vb), w, TRUE, TRUE, PAD);

    sp_repr_add_listener (SP_OBJECT_REPR(gradient), &grad_edit_dia_repr_events, vb);
    GtkTooltips *tt = gtk_tooltips_new ();

    /* Stop list */
    GtkWidget *mnu = gtk_option_menu_new ();
    /* Create new menu widget */
    update_stop_list (GTK_WIDGET(mnu), gradient, NULL);
    gtk_signal_connect (GTK_OBJECT (mnu), "changed", GTK_SIGNAL_FUNC (sp_grad_edit_select), vb);
    gtk_widget_show (mnu);
    gtk_object_set_data (GTK_OBJECT (vb), "stopmenu", mnu);
    gtk_box_pack_start (GTK_BOX (vb), mnu, FALSE, FALSE, 0);

    /* Add and Remove buttons */
    GtkWidget *hb = gtk_hbox_new (FALSE, 1);
    // TRANSLATORS: "Stop" means: a "phase" of a gradient
    GtkWidget *b = gtk_button_new_with_label (_("Add stop"));
    gtk_widget_show (b);
    gtk_container_add (GTK_CONTAINER (hb), b);
    gtk_tooltips_set_tip (tt, b, _("Add another control stop to gradient"), NULL);
    gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (sp_grd_ed_add_stop), vb);
    b = gtk_button_new_with_label (_("Delete stop"));
    gtk_widget_show (b);
    gtk_container_add (GTK_CONTAINER (hb), b);
    gtk_tooltips_set_tip (tt, b, _("Delete current control stop from gradient"), NULL);
    gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (sp_grd_ed_del_stop), vb);

    gtk_widget_show (hb);
    gtk_box_pack_start (GTK_BOX (vb),hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);


    /*  Offset Slider and stuff   */
    hb = gtk_hbox_new (FALSE, 0);

    /* Label */
    GtkWidget *l = gtk_label_new (_("Offset:"));
    gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
    gtk_box_pack_start (GTK_BOX (hb),l, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    gtk_widget_show (l);

    /* Adjustment */
    GtkAdjustment *Offset_adj = NULL;
    Offset_adj= (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 1.0, 0.01, 0.01, 0.0);
    gtk_object_set_data (GTK_OBJECT (vb), "offset", Offset_adj);
    GtkMenu *m = GTK_MENU(gtk_option_menu_get_menu (GTK_OPTION_MENU(mnu)));
    SPStop *stop = SP_STOP (g_object_get_data (G_OBJECT (gtk_menu_get_active (m)), "stop"));
    gtk_adjustment_set_value (Offset_adj, stop->offset);

    /* Slider */
    GtkWidget *slider = gtk_hscale_new(Offset_adj);
    gtk_scale_set_draw_value( GTK_SCALE(slider), FALSE );
    gtk_widget_show (slider);
    gtk_box_pack_start (GTK_BOX (hb),slider, TRUE, TRUE, AUX_BETWEEN_BUTTON_GROUPS);
    gtk_object_set_data (GTK_OBJECT (vb), "offslide", slider);

    /* Spinbutton */
    GtkWidget *sbtn = gtk_spin_button_new (GTK_ADJUSTMENT (Offset_adj), 0.01, 2);
    sp_dialog_defocus_on_enter (sbtn);
    gtk_widget_show (sbtn);
    gtk_box_pack_start (GTK_BOX (hb),sbtn, FALSE, TRUE, AUX_BETWEEN_BUTTON_GROUPS);
    gtk_object_set_data (GTK_OBJECT (vb), "offspn", sbtn);

    if (stop->offset>0 && stop->offset<1) {
        gtk_widget_set_sensitive (slider, TRUE);
        gtk_widget_set_sensitive (GTK_WIDGET (sbtn), TRUE);
    } else {
        gtk_widget_set_sensitive (slider, FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (sbtn), FALSE);
    }


    /* Signals */
    gtk_signal_connect (GTK_OBJECT (Offset_adj), "value_changed",
                        GTK_SIGNAL_FUNC (offadjustmentChanged), vb);

    // gtk_signal_connect (GTK_OBJECT (slider), "changed",  GTK_SIGNAL_FUNC (offsliderChanged), vb);
    gtk_widget_show (hb);
    gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, PAD);

    // TRANSLATORS: "Stop" means: a "phase" of a gradient
    f = gtk_frame_new (_("Stop Color"));
    gtk_widget_show (f);
    gtk_box_pack_start (GTK_BOX (vb), f, TRUE, TRUE, PAD);
    csel = (GtkWidget*)sp_color_selector_new (SP_TYPE_COLOR_NOTEBOOK);
    g_object_set_data (G_OBJECT (vb), "cselector", csel);
    gtk_widget_show (csel);
    gtk_container_add (GTK_CONTAINER (f), csel);
    g_signal_connect (G_OBJECT (csel), "dragged", G_CALLBACK (sp_gradient_vector_color_dragged), vb);
    g_signal_connect (G_OBJECT (csel), "changed", G_CALLBACK (sp_gradient_vector_color_changed), vb);

    gtk_widget_show (vb);

    sp_gradient_vector_widget_load_gradient (vb, gradient);

    if (select_stop)
        select_stop_in_list (GTK_WIDGET(mnu), gradient, select_stop);

    return vb;
}



GtkWidget *
sp_gradient_vector_editor_new (SPGradient *gradient, SPStop *stop)
{
    GtkWidget *wid;

    if (dlg == NULL) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();

        dlg = sp_window_new (_("Gradient editor"), TRUE);
        if (x == -1000 || y == -1000) {
            x = prefs->getInt(prefs_path + "x", -1000);
            y = prefs->getInt(prefs_path + "y", -1000);
        }
        if (w ==0 || h == 0) {
            w = prefs->getInt(prefs_path + "w", 0);
            h = prefs->getInt(prefs_path + "h", 0);
        }

        if (x<0) x=0;
        if (y<0) y=0;

        if (x != 0 || y != 0)
            gtk_window_move ((GtkWindow *) dlg, x, y);
        else
            gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
        if (w && h) gtk_window_resize ((GtkWindow *) dlg, w, h);
        sp_transientize (dlg);
        wd.win = dlg;
        wd.stop = 0;
        g_signal_connect (G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_transientize_callback), &wd);
        gtk_signal_connect (GTK_OBJECT (dlg), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg);
        gtk_signal_connect (GTK_OBJECT (dlg), "destroy", G_CALLBACK (sp_gradient_vector_dialog_destroy), dlg);
        gtk_signal_connect (GTK_OBJECT (dlg), "delete_event", G_CALLBACK (sp_gradient_vector_dialog_delete), dlg);
        g_signal_connect (G_OBJECT (INKSCAPE), "shut_down", G_CALLBACK (sp_gradient_vector_dialog_delete), dlg);
        g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), dlg );
        g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), dlg );

        gtk_container_set_border_width (GTK_CONTAINER (dlg), PAD);

        wid = (GtkWidget*)sp_gradient_vector_widget_new (gradient, stop);
        g_object_set_data (G_OBJECT (dlg), "gradient-vector-widget", wid);
        /* Connect signals */
        gtk_widget_show (wid);
        gtk_container_add (GTK_CONTAINER (dlg), wid);
    } else {
        // FIXME: temp fix for 0.38
        // Simply load_gradient into the editor does not work for multi-stop gradients,
        // as the stop list and other widgets are in a wrong state and crash readily.
        // Instead we just delete the window (by sending the delete signal)
        // and call sp_gradient_vector_editor_new again, so it creates the window anew.

        GdkEventAny event;
        GtkWidget *widget = (GtkWidget *) dlg;
        event.type = GDK_DELETE;
        event.window = widget->window;
        event.send_event = TRUE;
        g_object_ref (G_OBJECT (event.window));
        gtk_main_do_event ((GdkEvent*)&event);
        g_object_unref (G_OBJECT (event.window));

        g_assert (dlg == NULL);
        sp_gradient_vector_editor_new (gradient, stop);
    }

    return dlg;
}

static void
sp_gradient_vector_widget_load_gradient (GtkWidget *widget, SPGradient *gradient)
{
    blocked = TRUE;

    SPGradient *old;

    old = (SPGradient*)g_object_get_data (G_OBJECT (widget), "gradient");

    if (old != gradient) {
        sigc::connection *release_connection;
        sigc::connection *modified_connection;

        release_connection = (sigc::connection *)g_object_get_data(G_OBJECT(widget), "gradient_release_connection");
        modified_connection = (sigc::connection *)g_object_get_data(G_OBJECT(widget), "gradient_modified_connection");

        if (old) {
            g_assert( release_connection != NULL );
            g_assert( modified_connection != NULL );
            release_connection->disconnect();
            modified_connection->disconnect();
            sp_signal_disconnect_by_data (old, widget);
        }

        if (gradient) {
            if (!release_connection) {
                release_connection = new sigc::connection();
            }
            if (!modified_connection) {
                modified_connection = new sigc::connection();
            }
            *release_connection = gradient->connectRelease(sigc::bind<1>(sigc::ptr_fun(&sp_gradient_vector_gradient_release), widget));
            *modified_connection = gradient->connectModified(sigc::bind<2>(sigc::ptr_fun(&sp_gradient_vector_gradient_modified), widget));
        } else {
            if (release_connection) {
                delete release_connection;
                release_connection = NULL;
            }
            if (modified_connection) {
                delete modified_connection;
                modified_connection = NULL;
            }
        }

        g_object_set_data(G_OBJECT(widget), "gradient_release_connection", release_connection);
        g_object_set_data(G_OBJECT(widget), "gradient_modified_connection", modified_connection);
    }

    g_object_set_data (G_OBJECT (widget), "gradient", gradient);

    if (gradient) {
        gtk_widget_set_sensitive (widget, TRUE);

        sp_gradient_ensure_vector (gradient);

        GtkOptionMenu *mnu = (GtkOptionMenu *)g_object_get_data (G_OBJECT(widget), "stopmenu");
        SPStop *stop = SP_STOP(g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "stop"));
        guint32 const c = sp_stop_get_rgba32(stop);

        /// get the color selector
        SPColorSelector *csel = SP_COLOR_SELECTOR(g_object_get_data (G_OBJECT (widget), "cselector"));
        // set alpha
        csel->base->setAlpha(SP_RGBA32_A_F (c));
        SPColor color( SP_RGBA32_R_F (c), SP_RGBA32_G_F (c), SP_RGBA32_B_F (c) );
        // set color
        csel->base->setColor( color );

        /* Fill preview */
        GtkWidget *w = static_cast<GtkWidget *>(g_object_get_data(G_OBJECT(widget), "preview"));
        sp_gradient_image_set_gradient (SP_GRADIENT_IMAGE (w), gradient);

        update_stop_list (GTK_WIDGET(mnu), gradient, NULL);

        // Once the user edits a gradient, it stops being auto-collectable
        if (SP_OBJECT_REPR(gradient)->attribute("inkscape:collect")) {
            SPDocument *document = SP_OBJECT_DOCUMENT (gradient);
            bool saved = sp_document_get_undo_sensitive(document);
            sp_document_set_undo_sensitive (document, false);
            SP_OBJECT_REPR(gradient)->setAttribute("inkscape:collect", NULL);
            sp_document_set_undo_sensitive (document, saved);
        }
    } else { // no gradient, disable everything
        gtk_widget_set_sensitive (widget, FALSE);
    }

    blocked = FALSE;
}

static void
sp_gradient_vector_dialog_destroy (GtkObject */*object*/, gpointer /*data*/)
{
    sp_signal_disconnect_by_data (INKSCAPE, dlg);
    wd.win = dlg = NULL;
    wd.stop = 0;
}

static gboolean
sp_gradient_vector_dialog_delete (GtkWidget */*widget*/, GdkEvent */*event*/, GtkWidget */*dialog*/)
{
    gtk_window_get_position ((GtkWindow *) dlg, &x, &y);
    gtk_window_get_size ((GtkWindow *) dlg, &w, &h);

    if (x<0) x=0;
    if (y<0) y=0;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt(prefs_path + "x", x);
    prefs->setInt(prefs_path + "y", y);
    prefs->setInt(prefs_path + "w", w);
    prefs->setInt(prefs_path + "h", h);

    return FALSE; // which means, go ahead and destroy it
}

/* Widget destroy handler */

static void
sp_gradient_vector_widget_destroy (GtkObject *object, gpointer /*data*/)
{
    GObject *gradient;

    gradient = (GObject*)g_object_get_data (G_OBJECT (object), "gradient");

    sigc::connection *release_connection = (sigc::connection *)g_object_get_data(G_OBJECT(object), "gradient_release_connection");
    sigc::connection *modified_connection = (sigc::connection *)g_object_get_data(G_OBJECT(object), "gradient_modified_connection");

    if (gradient) {
        g_assert( release_connection != NULL );
        g_assert( modified_connection != NULL );
        release_connection->disconnect();
        modified_connection->disconnect();
        sp_signal_disconnect_by_data (gradient, object);
    }

    if (gradient && SP_OBJECT_REPR(gradient)) {
        sp_repr_remove_listener_by_data (SP_OBJECT_REPR(gradient), object);
    }
}

static void
sp_gradient_vector_gradient_release (SPObject */*object*/, GtkWidget *widget)
{
    sp_gradient_vector_widget_load_gradient (widget, NULL);
}

static void
sp_gradient_vector_gradient_modified (SPObject *object, guint /*flags*/, GtkWidget *widget)
{
    SPGradient *gradient=SP_GRADIENT(object);
    if (!blocked) {
        blocked = TRUE;
        sp_gradient_vector_widget_load_gradient (widget, gradient);
        blocked = FALSE;
    }
}

static void sp_gradient_vector_color_dragged(SPColorSelector *csel, GtkObject *object)
{
    SPGradient *gradient, *ngr;

    if (blocked) return;

    gradient = (SPGradient*)g_object_get_data (G_OBJECT (object), "gradient");
    if (!gradient) return;

    blocked = TRUE;

    ngr = sp_gradient_ensure_vector_normalized (gradient);
    if (ngr != gradient) {
        /* Our master gradient has changed */
        sp_gradient_vector_widget_load_gradient (GTK_WIDGET (object), ngr);
    }

    sp_gradient_ensure_vector (ngr);

    GtkOptionMenu *mnu = (GtkOptionMenu *)g_object_get_data (G_OBJECT(object), "stopmenu");
    SPStop *stop = SP_STOP(g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "stop"));


    csel->base->getColorAlpha(stop->specified_color, &stop->opacity);
    stop->currentColor = false;

    blocked = FALSE;
}

static void
sp_gradient_vector_color_changed (SPColorSelector *csel, GtkObject *object)
{
    SPColor color;
    float alpha;
    guint32 rgb;

    if (blocked) return;

    SPGradient *gradient = (SPGradient*)g_object_get_data (G_OBJECT (object), "gradient");
    if (!gradient) return;

    blocked = TRUE;

    SPGradient *ngr = sp_gradient_ensure_vector_normalized (gradient);
    if (ngr != gradient) {
        /* Our master gradient has changed */
        sp_gradient_vector_widget_load_gradient (GTK_WIDGET (object), ngr);
    }

    sp_gradient_ensure_vector (ngr);

    /* Set start parameters */
    /* We rely on normalized vector, i.e. stops HAVE to exist */
    g_return_if_fail (sp_first_stop(ngr) != NULL);

    GtkOptionMenu *mnu = (GtkOptionMenu *)g_object_get_data (G_OBJECT(object), "stopmenu");
    SPStop *stop = SP_STOP(g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "stop"));

    csel = (SPColorSelector*)g_object_get_data (G_OBJECT (object), "cselector");
    csel->base->getColorAlpha( color, &alpha );
    rgb = color.toRGBA32( 0x00 );

    sp_repr_set_css_double (SP_OBJECT_REPR (stop), "offset", stop->offset);
    Inkscape::CSSOStringStream os;
    gchar c[64];
    sp_svg_write_color (c, sizeof(c), rgb);
    os << "stop-color:" << c << ";stop-opacity:" << (gdouble) alpha <<";";
    SP_OBJECT_REPR (stop)->setAttribute("style", os.str().c_str());
    // g_snprintf (c, 256, "stop-color:#%06x;stop-opacity:%g;", rgb >> 8, (gdouble) alpha);
    //SP_OBJECT_REPR (stop)->setAttribute("style", c);

    sp_document_done (SP_OBJECT_DOCUMENT (ngr), SP_VERB_CONTEXT_GRADIENT,
                      _("Change gradient stop color"));

    blocked = FALSE;

    SPColorPreview *cpv = (SPColorPreview *)g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "preview");
    sp_color_preview_set_rgba32(cpv, sp_stop_get_rgba32(stop));
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
