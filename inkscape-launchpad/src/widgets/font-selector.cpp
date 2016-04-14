/**
 * Font selection widgets
 *
 * Authors:
 *   Chris Lahey <clahey@ximian.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 1999-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 * Copyright (C) -2013 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <libnrtype/font-lister.h>
#include <libnrtype/font-instance.h>

#include <2geom/transforms.h>

#include <gtk/gtk.h>

#include <glibmm/i18n.h>

#include "desktop.h"
#include "widgets/font-selector.h"
#include "preferences.h"

/* SPFontSelector */

struct SPFontSelector
{
#if GTK_CHECK_VERSION(3,0,0)
    GtkBox hbox;
#else
    GtkHBox hbox;
#endif

    unsigned int block_emit : 1;

    GtkWidget *family;
    GtkWidget *style;
    GtkWidget *size;

    GtkWidget *family_treeview;
    GtkWidget *style_treeview;

    NRNameList families;
    NRStyleList styles;
    gfloat fontsize;
    bool fontsize_dirty;
    Glib::ustring *fontspec;
};


struct SPFontSelectorClass
{
#if GTK_CHECK_VERSION(3,0,0)
    GtkBoxClass parent_class;
#else
    GtkHBoxClass parent_class;
#endif

    void (* font_set) (SPFontSelector *fsel, gchar *fontspec);
};

enum {
    FONT_SET,
    LAST_SIGNAL
};

static void sp_font_selector_dispose            (GObject              *object);

static void sp_font_selector_family_select_row  (GtkTreeSelection       *selection,
                                                 SPFontSelector         *fsel);

static void sp_font_selector_style_select_row   (GtkTreeSelection       *selection,
                                                 SPFontSelector         *fsel);

static void sp_font_selector_size_changed       (GtkComboBox            *combobox,
                                                 SPFontSelector         *fsel);

static void sp_font_selector_emit_set           (SPFontSelector         *fsel);
static void sp_font_selector_set_sizes( SPFontSelector *fsel );

static guint fs_signals[LAST_SIGNAL] = { 0 };

#if GTK_CHECK_VERSION(3,0,0)
G_DEFINE_TYPE(SPFontSelector, sp_font_selector, GTK_TYPE_BOX);
#else
G_DEFINE_TYPE(SPFontSelector, sp_font_selector, GTK_TYPE_HBOX);
#endif

static void sp_font_selector_class_init(SPFontSelectorClass *c)
{
    GObjectClass *object_class = G_OBJECT_CLASS(c);

    fs_signals[FONT_SET] = g_signal_new ("font_set",
                                           G_TYPE_FROM_CLASS(object_class),
                                           (GSignalFlags)G_SIGNAL_RUN_FIRST,
                                           G_STRUCT_OFFSET(SPFontSelectorClass, font_set),
					   NULL, NULL,
                                           g_cclosure_marshal_VOID__POINTER,
                                           G_TYPE_NONE,
                                           1, G_TYPE_POINTER);

    object_class->dispose = sp_font_selector_dispose;
}

static void sp_font_selector_set_size_tooltip(SPFontSelector *fsel)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int unit = prefs->getInt("/options/font/unitType", SP_CSS_UNIT_PT);
    Glib::ustring tooltip = Glib::ustring::format(_("Font size"), " (", sp_style_get_css_unit_string(unit), ")");
    gtk_widget_set_tooltip_text (fsel->size, _(tooltip.c_str()));
}


/*
 * Create a widget with children for selecting font-family, font-style, and font-size.
 */
static void sp_font_selector_init(SPFontSelector *fsel)
{
        gtk_box_set_homogeneous(GTK_BOX(fsel), TRUE);
        gtk_box_set_spacing(GTK_BOX(fsel), 4);

        /* Family frame */
        GtkWidget *f = gtk_frame_new(_("Font family"));
        gtk_widget_show (f);
        gtk_box_pack_start (GTK_BOX(fsel), f, TRUE, TRUE, 0);

        GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_widget_show(sw);
        gtk_container_set_border_width(GTK_CONTAINER (sw), 4);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_IN);
        gtk_container_add(GTK_CONTAINER(f), sw);

        fsel->family_treeview = gtk_tree_view_new ();
        gtk_tree_view_set_row_separator_func( GTK_TREE_VIEW(fsel->family_treeview),
                                              GtkTreeViewRowSeparatorFunc ((gpointer)font_lister_separator_func),
                                              NULL, NULL );
        gtk_widget_show_all(GTK_WIDGET (fsel->family_treeview));
        GtkTreeViewColumn *column = gtk_tree_view_column_new ();
        GtkCellRenderer *cell = gtk_cell_renderer_text_new ();
        gtk_tree_view_column_pack_start (column, cell, FALSE);
        gtk_tree_view_column_set_attributes (column, cell, "text", 0, NULL);
        gtk_tree_view_column_set_cell_data_func (column, cell,
                                                 GtkTreeCellDataFunc (font_lister_cell_data_func),
                                                 NULL, NULL );
        gtk_tree_view_append_column (GTK_TREE_VIEW(fsel->family_treeview), column);
        gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(fsel->family_treeview), FALSE);

        /* Muck with style, see text-toolbar.cpp */
        gtk_widget_set_name( GTK_WIDGET(fsel->family_treeview), "font_selector_family" );

#if GTK_CHECK_VERSION(3,0,0)
        GtkCssProvider *css_provider = gtk_css_provider_new();
        gtk_css_provider_load_from_data(css_provider,
                                        "#font_selector_family {\n"
                                        "  -GtkWidget-wide-separators:  true;\n"
                                        "  -GtkWidget-separator-height: 6;\n"
                                        "}\n",
                                        -1, NULL);

        GdkScreen *screen = gdk_screen_get_default();
        gtk_style_context_add_provider_for_screen(screen,
                                                  GTK_STYLE_PROVIDER(css_provider),
                                                  GTK_STYLE_PROVIDER_PRIORITY_USER);
#else
        gtk_rc_parse_string (
            "widget \"*font_selector_family\" style \"fontfamily-separator-style\"");
#endif        

        Inkscape::FontLister* fontlister = Inkscape::FontLister::get_instance();
        Glib::RefPtr<Gtk::ListStore> store = fontlister->get_font_list();
        gtk_tree_view_set_model (GTK_TREE_VIEW(fsel->family_treeview), GTK_TREE_MODEL (Glib::unwrap (store)));
        //gtk_tree_view_set_tooltip_column(GTK_TREE_VIEW(fsel->family_treeview),2);
        gtk_container_add(GTK_CONTAINER(sw), fsel->family_treeview);
        gtk_widget_show_all (sw);

        GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(fsel->family_treeview));
        g_signal_connect (G_OBJECT(selection), "changed", G_CALLBACK (sp_font_selector_family_select_row), fsel);
        g_object_set_data (G_OBJECT(fsel), "family-treeview", fsel->family_treeview);


        /* Style frame */
        f = gtk_frame_new(C_("Font selector", "Style"));
        gtk_widget_show(f);
        gtk_box_pack_start(GTK_BOX (fsel), f, TRUE, TRUE, 0);

#if GTK_CHECK_VERSION(3,0,0)
        GtkWidget *vb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
        gtk_box_set_homogeneous(GTK_BOX(vb), FALSE);
#else
        GtkWidget *vb = gtk_vbox_new(FALSE, 4);
#endif
        gtk_widget_show(vb);
        gtk_container_set_border_width(GTK_CONTAINER (vb), 4);
        gtk_container_add(GTK_CONTAINER(f), vb);

        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_widget_show(sw);
        gtk_container_set_border_width(GTK_CONTAINER (sw), 4);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_IN);
        gtk_box_pack_start(GTK_BOX (vb), sw, TRUE, TRUE, 0);

        fsel->style_treeview = gtk_tree_view_new ();

        // CSS Style name
        cell = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes ("CSS", cell, "text", 0, NULL );
        //gtk_tree_view_column_pack_start (column, cell, FALSE);
        gtk_tree_view_column_set_resizable (column, TRUE);
        gtk_tree_view_append_column (GTK_TREE_VIEW(fsel->style_treeview), column);

        // Display Style name
        cell = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes (_("Face"), cell, "text", 1, NULL );
        //gtk_tree_view_column_pack_start (column, cell, FALSE);
        gtk_tree_view_append_column (GTK_TREE_VIEW(fsel->style_treeview), column);

        //gtk_tree_view_set_tooltip_column(GTK_TREE_VIEW(fsel->style_treeview), 1);
        gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(fsel->style_treeview), TRUE);
        gtk_container_add(GTK_CONTAINER(sw), fsel->style_treeview);
        gtk_widget_show_all (sw);

        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(fsel->style_treeview));
        g_signal_connect (G_OBJECT(selection), "changed", G_CALLBACK (sp_font_selector_style_select_row), fsel);

#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
        GtkWidget *hb = gtk_hbox_new(FALSE, 4);
#endif
        gtk_widget_show(hb);
        gtk_box_pack_start(GTK_BOX(vb), hb, FALSE, FALSE, 0);

        // Font-size
        fsel->size = gtk_combo_box_text_new_with_entry ();

        sp_font_selector_set_size_tooltip(fsel);
        gtk_widget_set_size_request(fsel->size, 90, -1);
        g_signal_connect (G_OBJECT(fsel->size), "changed", G_CALLBACK (sp_font_selector_size_changed), fsel);
        gtk_box_pack_end (GTK_BOX(hb), fsel->size, FALSE, FALSE, 0);

        GtkWidget *l = gtk_label_new(_("Font size:"));
        gtk_widget_show_all (l);
        gtk_box_pack_end(GTK_BOX (hb), l, TRUE, TRUE, 0);

        sp_font_selector_set_sizes(fsel);

        gtk_widget_show_all (fsel->size);

        // Set default size... next two lines must match
        gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(fsel->size))), "18.0");
        fsel->fontsize = 18.0;
        fsel->fontsize_dirty = false;

        fsel->fontspec = new Glib::ustring;
}

static void sp_font_selector_dispose(GObject *object)
{
    SPFontSelector *fsel = SP_FONT_SELECTOR (object);

    if (fsel->fontspec) {
        delete fsel->fontspec;
    }

    if (fsel->families.length > 0) {
        nr_name_list_release(&fsel->families);
        fsel->families.length = 0;
    }

    if (fsel->styles.length > 0) {
        nr_style_list_release(&fsel->styles);
        fsel->styles.length = 0;
    }

    if (G_OBJECT_CLASS(sp_font_selector_parent_class)->dispose) {
        G_OBJECT_CLASS(sp_font_selector_parent_class)->dispose(object);
    }
}

// Callback when family changed, updates style list for new family.
static void sp_font_selector_family_select_row(GtkTreeSelection *selection,
                                               SPFontSelector *fsel)
{

    // We need our own copy of the style list store since the font-family
    // may not be the same in the font-selector as stored in the font-lister
    // TODO: use font-lister class for this by modifying new_font_family to accept an optional style list
    // TODO: add store to SPFontSelector struct and reuse.

    // Start by getting iterator to selected font
    GtkTreeModel *model;
    GtkTreeIter   iter;
    if (!gtk_tree_selection_get_selected (selection, &model, &iter)) return;
    
    Inkscape::FontLister *fontlister = Inkscape::FontLister::get_instance();
    fontlister->ensureRowStyles(model, &iter);

    // Next get family name with its style list
    gchar        *family;
    GList        *list=NULL;
    gtk_tree_model_get (model, &iter, 0, &family, 1, &list, -1);

    // Find best style match for selected family with current style (e.g. of selected text).
    Glib::ustring style = fontlister->get_font_style();
    Glib::ustring best  = fontlister->get_best_style_match (family, style);    

    // Create our own store of styles for selected font-family and find index of best style match
    int path_index = 0;
    int index = 0;
    GtkListStore *store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING); // Where is this deleted?
    for ( ; list ; list = list->next )
    {
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter,
                            0, ((StyleNames*)list->data)->CssName.c_str(),
                            1, ((StyleNames*)list->data)->DisplayName.c_str(),
                            -1);
        if( best.compare( ((StyleNames*)list->data)->CssName ) == 0 ) {
            path_index = index;
        }
        ++index;
    }

    // Attach store to tree view. Can trigger style changed signal (but not FONT_SET):
    gtk_tree_view_set_model (GTK_TREE_VIEW (fsel->style_treeview), GTK_TREE_MODEL (store));
    //gtk_tree_view_set_tooltip_column(GTK_TREE_VIEW(fsel->style_treeview),1);

    // Get path to best style
    GtkTreePath *path = gtk_tree_path_new ();
    gtk_tree_path_append_index (path, path_index);

    // Highlight best style. Triggers style changed signal:
    gtk_tree_selection_select_path (gtk_tree_view_get_selection (GTK_TREE_VIEW (fsel->style_treeview)), path);
    gtk_tree_path_free (path);
}

// Callback when row changed
static void sp_font_selector_style_select_row (GtkTreeSelection * /*selection*/,
                                               SPFontSelector   *fsel)
{
    if (!fsel->block_emit)
    {
        sp_font_selector_emit_set (fsel);
    }
}


/*
 * Set the default list of font sizes, scaled to the users preferred unit
 * TODO: This routine occurs both here and in text-toolbar. Move to font-lister?
 */
static void sp_font_selector_set_sizes( SPFontSelector *fsel )
{
    GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model (GTK_COMBO_BOX(fsel->size)));
    gtk_list_store_clear(store);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int unit = prefs->getInt("/options/font/unitType", SP_CSS_UNIT_PT);

    int sizes[] = {
        4, 6, 8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 22, 24, 28,
        32, 36, 40, 48, 56, 64, 72, 144
    };

    // Array must be same length as SPCSSUnit in style.h
    float ratios[] = {1, 1, 1, 10, 4, 40, 100, 16, 8, 0.16};

    for (unsigned int n = 0; n < G_N_ELEMENTS(sizes); ++n)
    {
        double size = sizes[n] / ratios[unit];

        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(fsel->size), Glib::ustring::format(size).c_str());
    }

}

// Callback when size changed
static void sp_font_selector_size_changed( GtkComboBox */*cbox*/, SPFontSelector *fsel )
{
    char *text = NULL;
    text = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (fsel->size));
    gfloat old_size = fsel->fontsize;

    gchar *endptr;
    gdouble value = -1;
    if (text) {
        value = g_strtod (text, &endptr);
        if (endptr == text) // conversion failed, non-numeric input
            value = -1;
        free (text);
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int max_size = prefs->getInt("/dialogs/textandfont/maxFontSize", 10000); // somewhat arbitrary, but text&font preview freezes with too huge fontsizes

    if (value <= 0) {
        return; // could not parse value 
    }
    if (value > max_size)
        value = max_size;

    fsel->fontsize = value;
    if ( fabs(fsel->fontsize-old_size) > 0.001)
    {
        fsel->fontsize_dirty = true;
    }

    sp_font_selector_emit_set (fsel);
}


// Called from sp_font_selector_style_select_row
// Called from sp_font_selector_size_changed
// Called indirectly for sp_font_selector_family_select_row (since style changes).
// Emits FONT_SET signal (handled by TextEdit::onFontChange, GlyphsPanel::fontChangeCB). 
static void sp_font_selector_emit_set (SPFontSelector *fsel)
{

    GtkTreeSelection *selection_family;
    GtkTreeSelection *selection_style;
    GtkTreeModel     *model_family;
    GtkTreeModel     *model_style;
    GtkTreeIter       iter_family;
    GtkTreeIter       iter_style;
    char             *family=NULL, *style=NULL;

    //We need to check this here since most GtkTreeModel operations are not atomic
    //See GtkListStore documenation, Chapter "Atomic Operations" --mderezynski

    model_family = gtk_tree_view_get_model (GTK_TREE_VIEW (fsel->family_treeview));
    if (!model_family) return;
    model_style  = gtk_tree_view_get_model (GTK_TREE_VIEW (fsel->style_treeview));
    if (!model_style ) return;

    selection_family = gtk_tree_view_get_selection (GTK_TREE_VIEW (fsel->family_treeview));
    selection_style  = gtk_tree_view_get_selection (GTK_TREE_VIEW (fsel->style_treeview ));

    if (!gtk_tree_selection_get_selected (selection_family, NULL, &iter_family)) return;
    if (!gtk_tree_selection_get_selected (selection_style,  NULL, &iter_style )) return;

    gtk_tree_model_get (model_family, &iter_family, 0, &family, -1);
    gtk_tree_model_get (model_style,  &iter_style,  0, &style,  -1);

    if ((!family) || (!style)) return;

    Glib::ustring fontspec = family;
    fontspec += ", ";
    fontspec += style;

    *(fsel->fontspec) = fontspec;

    g_signal_emit(fsel, fs_signals[FONT_SET], 0, fontspec.c_str());
}

GtkWidget *sp_font_selector_new()
{
    SPFontSelector *fsel = SP_FONT_SELECTOR(g_object_new(SP_TYPE_FONT_SELECTOR, NULL));

    return GTK_WIDGET(fsel);
}


/*
 * Sets the values displayed in the font-selector from a fontspec.
 * It is only called from TextEdit with a new selection and from GlyphsPanel
 */
void sp_font_selector_set_fontspec (SPFontSelector *fsel, Glib::ustring fontspec, double size)
{
    if (!fontspec.empty())
    {

        Inkscape::FontLister *font_lister = Inkscape::FontLister::get_instance();
        std::pair<Glib::ustring, Glib::ustring> ui = font_lister->ui_from_fontspec( fontspec );
        Glib::ustring family = ui.first;
        Glib::ustring style = ui.second;

        Gtk::TreePath path;
        try {
            path = font_lister->get_row_for_font (family);
        } catch (...) {
            g_warning( "Couldn't find row for font-family: %s", family.c_str() );
            return;
        }

        // High light selected family and scroll so it is in view.
        fsel->block_emit = TRUE;
        gtk_tree_selection_select_path (gtk_tree_view_get_selection (GTK_TREE_VIEW (fsel->family_treeview)), path.gobj());
        gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (fsel->family_treeview), path.gobj(), NULL, TRUE, 0.5, 0.5);
        fsel->block_emit = FALSE;   // TODO: Should this be moved to the end?


        // We don't need to get best style since this is only called on a new
        // selection where we already know the "best" style.
        // Glib::ustring bestStyle = font_lister->get_best_style_match (family, style);
        // std::cout << "Best: " << bestStyle << std::endl;

        // The "trial" style list and the regular list are the same in this case.
            Gtk::TreePath path_c;
        try {
            path_c = font_lister->get_row_for_style( style );
        } catch (...) {
            g_warning( "Couldn't find row for style: %s (%s)", style.c_str(), family.c_str() );
            return;
        }

        gtk_tree_selection_select_path (gtk_tree_view_get_selection (GTK_TREE_VIEW (fsel->style_treeview)), path_c.gobj());
        gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (fsel->style_treeview), path_c.gobj(), NULL, TRUE, 0.5, 0.5);

        if (size != fsel->fontsize)
        {
            gchar s[8];
            g_snprintf (s, 8, "%.5g", size); // UI, so printf is ok
            gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(fsel->size))), s);
            fsel->fontsize = size;
            sp_font_selector_set_size_tooltip(fsel);
            sp_font_selector_set_sizes(fsel);
        }
    }
    
}

Glib::ustring sp_font_selector_get_fontspec(SPFontSelector *fsel)
{
    return *(fsel->fontspec);
}

/*
 * Return the font size in pixels
 */
double sp_font_selector_get_size(SPFontSelector *fsel)
{
    return fsel->fontsize;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
