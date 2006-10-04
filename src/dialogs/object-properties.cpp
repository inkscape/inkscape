#define __OBJECT_PROPERTIES_C__

/**
 * \brief  Fill, stroke, and stroke style dialog
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtk/gtk.h>

#include <glibmm/i18n.h>
#include "helper/window.h"
#include "widgets/sp-widget.h"
#include "widgets/icon.h"
#include "macros.h"
#include "inkscape.h"
#include "fill-style.h"
#include "stroke-style.h"
#include "dialog-events.h"
#include "verbs.h"
#include "interface.h"
#include "style.h"
#include "inkscape-stock.h"
#include "prefs-utils.h"
#include "svg/css-ostringstream.h"
#include "sp-gaussian-blur.h"
#include "sp-filter.h"
#include "desktop-handles.h"
#include "desktop-style.h"
#include "document.h"
#include "document-private.h"
#include <selection.h>
#include "xml/repr.h"

static GtkWidget *dlg = NULL;
static win_data wd;

// impossible original values to make sure they are read from prefs
static gint x = -1000, y = -1000, w = 0, h = 0;
static gchar *prefs_path = "dialogs.fillstroke";

static void sp_fillstroke_selection_modified ( Inkscape::Application *inkscape, Inkscape::Selection *selection, guint flags, GtkObject *base );
static void sp_fillstroke_selection_changed ( Inkscape::Application *inkscape, Inkscape::Selection *selection, GtkObject *base );
static void sp_fillstroke_opacity_changed (GtkAdjustment *a, SPWidget *dlg);
static void sp_fillstroke_blur_changed (GtkAdjustment *a, SPWidget *dlg);

static void
sp_object_properties_dialog_destroy (GtkObject *object, gpointer data)
{
    sp_signal_disconnect_by_data (INKSCAPE, dlg);
    wd.win = dlg = NULL;
    wd.stop = 0;
}

static gboolean
sp_object_properties_dialog_delete ( GtkObject *object,
                                     GdkEvent *event,
                                     gpointer data )
{

    gtk_window_get_position ((GtkWindow *) dlg, &x, &y);
    gtk_window_get_size ((GtkWindow *) dlg, &w, &h);

    if (x<0) x=0;
    if (y<0) y=0;

    prefs_set_int_attribute (prefs_path, "x", x);
    prefs_set_int_attribute (prefs_path, "y", y);
    prefs_set_int_attribute (prefs_path, "w", w);
    prefs_set_int_attribute (prefs_path, "h", h);

    return FALSE; // which means, go ahead and destroy it

}


void
sp_object_properties_page( GtkWidget *nb,
                           GtkWidget *page,
                           char *label,
                           char *dlg_name,
                           char *label_image )
{
    GtkWidget *hb, *l, *px;

    hb = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hb);

    px = sp_icon_new( Inkscape::ICON_SIZE_DECORATION, label_image );
    gtk_widget_show (px);
    gtk_box_pack_start (GTK_BOX (hb), px, FALSE, FALSE, 2);

    l = gtk_label_new_with_mnemonic (label);
    gtk_widget_show (l);
    gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);

    gtk_widget_show (page);
    gtk_notebook_append_page (GTK_NOTEBOOK (nb), page, hb);
    gtk_object_set_data (GTK_OBJECT (dlg), dlg_name, page);
}

void
sp_object_properties_dialog (void)
{
    if (!dlg) {
        gchar title[500];
        sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_DIALOG_FILL_STROKE), title);

        dlg = sp_window_new (title, TRUE);
        if (x == -1000 || y == -1000) {
            x = prefs_get_int_attribute (prefs_path, "x", 0);
            y = prefs_get_int_attribute (prefs_path, "y", 0);
        }
        if (w ==0 || h == 0) {
            w = prefs_get_int_attribute (prefs_path, "w", 0);
            h = prefs_get_int_attribute (prefs_path, "h", 0);
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

        g_signal_connect ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_transientize_callback), &wd );

        gtk_signal_connect ( GTK_OBJECT (dlg), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg );

        gtk_signal_connect ( GTK_OBJECT (dlg), "destroy", G_CALLBACK (sp_object_properties_dialog_destroy), dlg );
        gtk_signal_connect ( GTK_OBJECT (dlg), "delete_event", G_CALLBACK (sp_object_properties_dialog_delete), dlg );
        g_signal_connect ( G_OBJECT (INKSCAPE), "shut_down", G_CALLBACK (sp_object_properties_dialog_delete), dlg );

        g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), dlg );
        g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), dlg );

        GtkWidget *vb = gtk_vbox_new (FALSE, 0);
        gtk_widget_show (vb);
        gtk_container_add (GTK_CONTAINER (dlg), vb);

        GtkWidget *nb = gtk_notebook_new ();
        gtk_widget_show (nb);
        gtk_box_pack_start (GTK_BOX (vb), nb, TRUE, TRUE, 0);
        gtk_object_set_data (GTK_OBJECT (dlg), "notebook", nb);

        /* Fill page */
        {
            GtkWidget *page = sp_fill_style_widget_new ();
            sp_object_properties_page(nb, page, _("_Fill"), "fill",
                                      INKSCAPE_STOCK_PROPERTIES_FILL_PAGE);
        }

        /* Stroke paint page */
        {
            GtkWidget *page = sp_stroke_style_paint_widget_new ();
            sp_object_properties_page(nb, page, _("Stroke _paint"), "stroke-paint",
                                      INKSCAPE_STOCK_PROPERTIES_STROKE_PAINT_PAGE);
        }

        /* Stroke style page */
        {
            GtkWidget *page = sp_stroke_style_line_widget_new ();
            sp_object_properties_page(nb, page, _("Stroke st_yle"), "stroke-line",
                                      INKSCAPE_STOCK_PROPERTIES_STROKE_PAGE);
        }


        /* Blur */
        GtkWidget *b_vb = gtk_vbox_new (FALSE, 0);
        gtk_box_pack_start (GTK_BOX (vb), b_vb, FALSE, FALSE, 2);
        gtk_object_set_data (GTK_OBJECT (dlg), "blur", b_vb);

        GtkWidget *blur_l_hb = gtk_hbox_new (FALSE, 4);
        GtkWidget *blur_l = gtk_label_new_with_mnemonic (_("_Blur"));
        gtk_misc_set_alignment (GTK_MISC (blur_l), 0.0, 1.0);
        gtk_box_pack_start (GTK_BOX (blur_l_hb), blur_l, FALSE, FALSE, 4);
        gtk_box_pack_start (GTK_BOX (b_vb), blur_l_hb, FALSE, FALSE, 0);

        GtkWidget *blur_hb = gtk_hbox_new (FALSE, 4);
        gtk_box_pack_start (GTK_BOX (b_vb), blur_hb, FALSE, FALSE, 0);

        GtkObject *blur_a = gtk_adjustment_new (0.0, 0.0, 100.0, 1.0, 1.0, 0.0);
        gtk_object_set_data(GTK_OBJECT(dlg), "blur_adjustment", blur_a);

        GtkWidget *blur_s = gtk_hscale_new (GTK_ADJUSTMENT (blur_a));
        gtk_scale_set_draw_value (GTK_SCALE (blur_s), FALSE);
        gtk_box_pack_start (GTK_BOX (blur_hb), blur_s, TRUE, TRUE, 4);
        gtk_label_set_mnemonic_widget (GTK_LABEL(blur_l), blur_s);

        GtkWidget *blur_sb = gtk_spin_button_new (GTK_ADJUSTMENT (blur_a), 0.01, 3);
        gtk_box_pack_start (GTK_BOX (blur_hb), blur_sb, FALSE, FALSE, 0);

        gtk_signal_connect ( blur_a, "value_changed",
                             GTK_SIGNAL_FUNC (sp_fillstroke_blur_changed),
                             dlg );
                             
        gtk_widget_show_all (b_vb);


        /* Opacity */

        GtkWidget *o_vb = gtk_vbox_new (FALSE, 0);
        gtk_box_pack_start (GTK_BOX (vb), o_vb, FALSE, FALSE, 2);
        gtk_object_set_data (GTK_OBJECT (dlg), "master_opacity", o_vb);

        GtkWidget *l_hb = gtk_hbox_new (FALSE, 4);
        GtkWidget *l = gtk_label_new_with_mnemonic (_("Master _opacity"));
        gtk_misc_set_alignment (GTK_MISC (l), 0.0, 1.0);
        gtk_box_pack_start (GTK_BOX (l_hb), l, FALSE, FALSE, 4);
        gtk_box_pack_start (GTK_BOX (o_vb), l_hb, FALSE, FALSE, 0);

        GtkWidget *hb = gtk_hbox_new (FALSE, 4);
        gtk_box_pack_start (GTK_BOX (o_vb), hb, FALSE, FALSE, 0);

        GtkObject *a = gtk_adjustment_new (1.0, 0.0, 1.0, 0.01, 0.1, 0.0);
        gtk_object_set_data(GTK_OBJECT(dlg), "master_opacity_adjustment", a);

        GtkWidget *s = gtk_hscale_new (GTK_ADJUSTMENT (a));
        gtk_scale_set_draw_value (GTK_SCALE (s), FALSE);
        gtk_box_pack_start (GTK_BOX (hb), s, TRUE, TRUE, 4);
        gtk_label_set_mnemonic_widget (GTK_LABEL(l), s);

        GtkWidget *sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0.01, 3);
        gtk_box_pack_start (GTK_BOX (hb), sb, FALSE, FALSE, 0);

        gtk_signal_connect ( a, "value_changed",
                             GTK_SIGNAL_FUNC (sp_fillstroke_opacity_changed),
                             dlg );

        gtk_widget_show_all (o_vb);

        // these callbacks are only for the master opacity update; the tabs above take care of themselves
        g_signal_connect ( G_OBJECT (INKSCAPE), "change_selection", G_CALLBACK (sp_fillstroke_selection_changed), dlg );
        g_signal_connect ( G_OBJECT (INKSCAPE), "modify_selection", G_CALLBACK (sp_fillstroke_selection_modified), dlg );
        g_signal_connect ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_fillstroke_selection_changed), dlg );

        sp_fillstroke_selection_changed(NULL, NULL, NULL);

        gtk_widget_show (dlg);

    } else {
        gtk_window_present (GTK_WINDOW (dlg));
    }

} // end of sp_object_properties_dialog()

void sp_object_properties_fill (void)
{
    sp_object_properties_dialog ();
    GtkWidget *nb = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (dlg), "notebook");
    gtk_notebook_set_page (GTK_NOTEBOOK (nb), 0);
}

void sp_object_properties_stroke (void)
{
    sp_object_properties_dialog ();
    GtkWidget *nb = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (dlg), "notebook");
    gtk_notebook_set_page (GTK_NOTEBOOK (nb), 1);
}

void sp_object_properties_stroke_style (void)
{
    sp_object_properties_dialog ();
    GtkWidget *nb = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (dlg), "notebook");
    gtk_notebook_set_page (GTK_NOTEBOOK (nb), 2);
}



static void
sp_fillstroke_selection_modified ( Inkscape::Application *inkscape,
                              Inkscape::Selection *selection,
                              guint flags,
                              GtkObject *base )
{
    sp_fillstroke_selection_changed ( inkscape, selection, base );
}


static void
sp_fillstroke_selection_changed ( Inkscape::Application *inkscape,
                              Inkscape::Selection *selection,
                              GtkObject *base )
{
    if (gtk_object_get_data (GTK_OBJECT (dlg), "blocked"))
        return;
    gtk_object_set_data (GTK_OBJECT (dlg), "blocked", GUINT_TO_POINTER (TRUE));

    GtkWidget *opa = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dlg), "master_opacity"));
    GtkAdjustment *a = GTK_ADJUSTMENT(gtk_object_get_data(GTK_OBJECT(dlg), "master_opacity_adjustment"));

    // create temporary style
    SPStyle *query = sp_style_new ();
    // query style from desktop into it. This returns a result flag and fills query with the style of subselection, if any, or selection
    int result = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_MASTEROPACITY);

    switch (result) {
        case QUERY_STYLE_NOTHING:
            gtk_widget_set_sensitive (opa, FALSE);
            break;
        case QUERY_STYLE_SINGLE:
        case QUERY_STYLE_MULTIPLE_AVERAGED: // TODO: treat this slightly differently
        case QUERY_STYLE_MULTIPLE_SAME: 
            gtk_widget_set_sensitive (opa, TRUE);
            gtk_adjustment_set_value(a, SP_SCALE24_TO_FLOAT(query->opacity.value));
            break;
    }


    GtkWidget *b = GTK_WIDGET (gtk_object_get_data (GTK_OBJECT (dlg), "blur"));
    GtkAdjustment *bluradjustment = GTK_ADJUSTMENT(gtk_object_get_data(GTK_OBJECT(dlg), "blur_adjustment"));

    //query now for current average blurring of selection
    int blur_result = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_BLUR);
    switch (blur_result) {
        case QUERY_STYLE_NOTHING: //no blurring
            gtk_widget_set_sensitive (b, FALSE);
            break;
        case QUERY_STYLE_SINGLE:
        case QUERY_STYLE_MULTIPLE_AVERAGED:
        case QUERY_STYLE_MULTIPLE_SAME: 
            gtk_widget_set_sensitive (b, TRUE);
            //update blur widget value
            float radius = query->filter_gaussianBlur_deviation.value;
            // TODO: divide by bbox diagonal
            gtk_adjustment_set_value(bluradjustment, radius);
            break;
    }
    
    
    g_free (query);
    gtk_object_set_data (GTK_OBJECT (dlg), "blocked", GUINT_TO_POINTER (FALSE));
}

static void
sp_fillstroke_opacity_changed (GtkAdjustment *a, SPWidget *dlg)
{
    if (gtk_object_get_data (GTK_OBJECT (dlg), "blocked"))
        return;

    gtk_object_set_data (GTK_OBJECT (dlg), "blocked", GUINT_TO_POINTER (TRUE));

    SPCSSAttr *css = sp_repr_css_attr_new ();

    Inkscape::CSSOStringStream os;
    os << CLAMP (a->value, 0.0, 1.0);
    sp_repr_css_set_property (css, "opacity", os.str().c_str());

    sp_desktop_set_style (SP_ACTIVE_DESKTOP, css);

    sp_repr_css_attr_unref (css);

    sp_document_maybe_done (sp_desktop_document (SP_ACTIVE_DESKTOP), "fillstroke:opacity", SP_VERB_NONE, 
                            /* TODO: annotate */ "object-properties.cpp:311");

    gtk_object_set_data (GTK_OBJECT (dlg), "blocked", GUINT_TO_POINTER (FALSE));
}


/**
 * Creates a filter with blur primitive of specified stdDeviation
 */
static SPFilter *
sp_fillstroke_get_new_filter(SPDocument *document, gdouble stdDeviation)
{
    g_return_val_if_fail(document != NULL, NULL);

    SPDefs *defs = (SPDefs *) SP_DOCUMENT_DEFS(document);

    // create a new private filter
    Inkscape::XML::Node *repr;
    repr = sp_repr_new("svg:filter");
    repr->setAttribute("inkscape:collect", "always");

    //create feGaussianBlur node
    Inkscape::XML::Node *b_repr;
    b_repr = sp_repr_new("svg:feGaussianBlur");
    b_repr->setAttribute("inkscape:collect", "always");
    
    //set stdDeviation attribute
    Inkscape::CSSOStringStream os;
    os << stdDeviation;
    b_repr->setAttribute("stdDeviation", os.str().c_str());
    
    //set feGaussianBlur as child of filter node
    repr->appendChild(b_repr);
    Inkscape::GC::release(b_repr);
    
    // Append the new filter node to defs
    SP_OBJECT_REPR(defs)->appendChild(repr);
    Inkscape::GC::release(repr);

    // get corresponding object
    SPFilter *f = SP_FILTER( document->getObjectByRepr(repr) );
    SPGaussianBlur *b = SP_GAUSSIANBLUR( document->getObjectByRepr(b_repr) );
    
    g_assert(f != NULL);
    g_assert(SP_IS_FILTER(f));
    g_assert(b != NULL);
    g_assert(SP_IS_GAUSSIANBLUR(b));

    return f;
}


static void
sp_fillstroke_blur_changed (GtkAdjustment *a, SPWidget *dlg)
{
    //if dialog is locked, return 
    if (gtk_object_get_data (GTK_OBJECT (dlg), "blocked"))
        return;

     //lock dialog
    gtk_object_set_data (GTK_OBJECT (dlg), "blocked", GUINT_TO_POINTER (TRUE));
    
    //get desktop
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop) {
        return;
    }
    
    //get current selection
    Inkscape::Selection *selection = sp_desktop_selection (desktop);
    //get list of selected items
    GSList const *items = selection->itemList();
    //get current document
    SPDocument *document = sp_desktop_document (desktop);
    
    
    //create new filter with feGaussianBlur primitive
    SPFilter *constructed = sp_fillstroke_get_new_filter(document, a->value);

    //apply created filter to every selected item
    for (GSList const *i = items; i != NULL; i = i->next) {
    
        SPItem * item = SP_ITEM(i->data);
        SPStyle *style = SP_OBJECT_STYLE(item);
        g_assert(style != NULL);

        if(a->value==0.0) //blur set to zero, remove filter
        {
            //if there is a filter attached, remove it
            SPCSSAttr *css = sp_repr_css_attr_new ();
            sp_repr_css_unset_property (css, "filter");
            sp_repr_css_change_recursive(SP_OBJECT_REPR(item), css, "style");
            sp_desktop_set_style (SP_ACTIVE_DESKTOP, css);
            sp_repr_css_attr_unref (css);
        }/* else if( style->filter.filter ) { //item has a filter assigned
            Inkscape::XML::Node *repr = SP_OBJECT_REPR ( style->filter.filter );
            Inkscape::CSSOStringStream os;
            os << a->value;
            repr->firstChild()->setAttribute("stdDeviation", os.str().c_str());
        }*/ else {
            //add new filter attribute to object style
            gchar *val = g_strdup_printf("url(#%s)", SP_OBJECT_ID(constructed));
            SPCSSAttr *css = sp_repr_css_attr_new();
            sp_repr_css_set_property(css, "filter", val);
            g_free(val);
            sp_repr_css_change_recursive(SP_OBJECT_REPR(item), css, "style");
            sp_desktop_set_style (SP_ACTIVE_DESKTOP, css);
            sp_repr_css_attr_unref(css);
        }
        //request update
        SP_OBJECT(item)->requestDisplayUpdate(( SP_OBJECT_MODIFIED_FLAG |
                                            SP_OBJECT_STYLE_MODIFIED_FLAG ));
    }

    sp_document_maybe_done (sp_desktop_document (SP_ACTIVE_DESKTOP), "fillstroke:blur", SP_VERB_NONE,  "object-properties.cpp:467");
    gtk_object_set_data (GTK_OBJECT (dlg), "blocked", GUINT_TO_POINTER (FALSE));
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

