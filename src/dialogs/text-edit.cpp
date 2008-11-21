/** @file
 * @brief Text editing dialog
 */
/* Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 1999-2007 Authors
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <libnrtype/font-instance.h>
#include <gtk/gtk.h>

#ifdef WITH_GTKSPELL
extern "C" {
# include <gtkspell/gtkspell.h>
}
#endif

#include "macros.h"
#include <glibmm/i18n.h>
#include "helper/window.h"
#include "../widgets/font-selector.h"
#include "../inkscape.h"
#include "../document.h"
#include "../desktop-style.h"
#include "../desktop-handles.h"
#include "../selection.h"
#include "../style.h"
#include "../sp-text.h"
#include "../sp-flowtext.h"
#include "../text-editing.h"
#include "../inkscape-stock.h"
#include <libnrtype/font-style-to-pos.h>

#include "dialog-events.h"
#include "../preferences.h"
#include "../verbs.h"
#include "../interface.h"
#include "svg/css-ostringstream.h"
#include "widgets/icon.h"
#include <xml/repr.h>

#define VB_MARGIN 4
#define MIN_ONSCREEN_DISTANCE 50

static void sp_text_edit_dialog_selection_modified (Inkscape::Application *inkscape, Inkscape::Selection *sel, guint flags, GtkWidget *dlg);
static void sp_text_edit_dialog_selection_changed (Inkscape::Application *inkscape, Inkscape::Selection *sel, GtkWidget *dlg);
static void sp_text_edit_dialog_subselection_changed ( Inkscape::Application *inkscape, SPDesktop *desktop, GtkWidget *dlg);

static void sp_text_edit_dialog_set_default (GtkButton *button, GtkWidget *dlg);
static void sp_text_edit_dialog_apply (GtkButton *button, GtkWidget *dlg);
static void sp_text_edit_dialog_close (GtkButton *button, GtkWidget *dlg);

static void sp_text_edit_dialog_read_selection (GtkWidget *dlg, gboolean style, gboolean content);

static void sp_text_edit_dialog_text_changed (GtkTextBuffer *tb, GtkWidget *dlg);
static void sp_text_edit_dialog_font_changed (SPFontSelector *fontsel, font_instance *font, GtkWidget *dlg);
static void sp_text_edit_dialog_any_toggled (GtkToggleButton *tb, GtkWidget *dlg);
static void sp_text_edit_dialog_line_spacing_changed (GtkEditable *editable, GtkWidget *dlg);

static SPItem *sp_ted_get_selected_text_item (void);
static unsigned sp_ted_get_selected_text_count (void);


static const gchar *spacings[] = {"50%", "80%", "90%", "100%", "110%", "120%", "130%", "140%", "150%", "200%", "300%", NULL};

static GtkWidget *dlg = NULL;
static win_data wd;
// impossible original values to make sure they are read from prefs
static gint x = -1000, y = -1000, w = 0, h = 0;
static Glib::ustring const prefs_path = "/dialogs/textandfont/";




static void
sp_text_edit_dialog_destroy( GtkObject */*object*/, gpointer /*data*/ )
{
    sp_signal_disconnect_by_data (INKSCAPE, dlg);
    wd.win = dlg = NULL;
    wd.stop = 0;
}



static gboolean
sp_text_edit_dialog_delete( GtkObject */*object*/, GdkEvent */*event*/, gpointer /*data*/ )
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


/**
    These callbacks set the eatkeys flag when the text editor is entered and cancel it when it's left.
    This flag is used to prevent passing keys from the dialog to canvas, so that the text editor
    can handle keys like Esc and Ctrl+Z itself.
 */
gboolean
text_view_focus_in( GtkWidget */*w*/, GdkEventKey */*event*/, gpointer data )
{
    GObject *dlg = (GObject *) data;
    g_object_set_data (dlg, "eatkeys", GINT_TO_POINTER (TRUE));
    return FALSE;
}

gboolean
text_view_focus_out (GtkWidget */*w*/, GdkEventKey */*event*/, gpointer data)
{
    GObject *dlg = (GObject *) data;
    g_object_set_data (dlg, "eatkeys", GINT_TO_POINTER (FALSE));
    return FALSE;
}


void
sp_text_edit_dialog (void)
{
    if (!dlg) {

        gchar title[500];
        sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_DIALOG_TEXT), title);
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();

        dlg = sp_window_new (title, TRUE);
        if (x == -1000 || y == -1000) {
            x = prefs->getInt(prefs_path + "x", -1000);
            y = prefs->getInt(prefs_path + "y", -1000);
        }
        if (w ==0 || h == 0) {
            w = prefs->getInt(prefs_path + "w", 0);
            h = prefs->getInt(prefs_path + "h", 0);
        }

//        if (x<0) x=0;
//        if (y<0) y=0;

        if (w && h)
            gtk_window_resize ((GtkWindow *) dlg, w, h);
        if (x >= 0 && y >= 0 && (x < (gdk_screen_width()-MIN_ONSCREEN_DISTANCE)) && (y < (gdk_screen_height()-MIN_ONSCREEN_DISTANCE))) {
            gtk_window_move ((GtkWindow *) dlg, x, y);
        } else {
            gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
        }


        sp_transientize (dlg);
        wd.win = dlg;
        wd.stop = 0;
        g_signal_connect ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_transientize_callback), &wd );

        gtk_signal_connect ( GTK_OBJECT (dlg), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg );

        gtk_signal_connect ( GTK_OBJECT (dlg), "destroy", G_CALLBACK (sp_text_edit_dialog_destroy), dlg );
        gtk_signal_connect ( GTK_OBJECT (dlg), "delete_event", G_CALLBACK (sp_text_edit_dialog_delete), dlg );
        g_signal_connect ( G_OBJECT (INKSCAPE), "shut_down", G_CALLBACK (sp_text_edit_dialog_delete), dlg );

        g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), dlg );
        g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), dlg );

        gtk_window_set_policy (GTK_WINDOW (dlg), TRUE, TRUE, FALSE);

        GtkTooltips *tt = gtk_tooltips_new();

        // box containing the notebook and the bottom buttons
        GtkWidget *mainvb = gtk_vbox_new (FALSE, 0);
        gtk_container_add (GTK_CONTAINER (dlg), mainvb);

        // notebook
        GtkWidget *nb = gtk_notebook_new ();
        gtk_box_pack_start (GTK_BOX (mainvb), nb, TRUE, TRUE, 0);
        g_object_set_data (G_OBJECT (dlg), "notebook", nb);



        // Font tab
        {
            GtkWidget *l = gtk_label_new (_("Font"));
            GtkWidget *vb = gtk_vbox_new (FALSE, VB_MARGIN);
            gtk_container_set_border_width (GTK_CONTAINER (vb), VB_MARGIN);
            gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);

            /* HBox containing font selection and layout */
            GtkWidget *hb = gtk_hbox_new (FALSE, 0);
            gtk_box_pack_start (GTK_BOX (vb), hb, TRUE, TRUE, 0);

            // font and style selector
            GtkWidget *fontsel = sp_font_selector_new ();
            g_signal_connect ( G_OBJECT (fontsel), "font_set", G_CALLBACK (sp_text_edit_dialog_font_changed), dlg );

            g_signal_connect_swapped ( G_OBJECT (g_object_get_data (G_OBJECT(fontsel), "family-treeview")),
                                      "row-activated",
                                      G_CALLBACK (gtk_window_activate_default),
                                      dlg);

            gtk_box_pack_start (GTK_BOX (hb), fontsel, TRUE, TRUE, 0);
            g_object_set_data (G_OBJECT (dlg), "fontsel", fontsel);

            // Layout
            {
                GtkWidget *f = gtk_frame_new (_("Layout"));
                gtk_box_pack_start (GTK_BOX (hb), f, FALSE, FALSE, 4);
                GtkWidget *l_vb = gtk_vbox_new (FALSE, VB_MARGIN);
                gtk_container_add (GTK_CONTAINER (f), l_vb);

                {
                    GtkWidget *row = gtk_hbox_new (FALSE, VB_MARGIN);
                    GtkWidget *group;

                    // align left
                    {
                        // TODO - replace with Inkscape-specific call
                        GtkWidget *px = gtk_image_new_from_stock ( GTK_STOCK_JUSTIFY_LEFT, GTK_ICON_SIZE_LARGE_TOOLBAR );
                        GtkWidget *b = group = gtk_radio_button_new (NULL);
                        gtk_tooltips_set_tip (tt, b, _("Align lines left"), NULL);
                        gtk_button_set_relief (GTK_BUTTON (b), GTK_RELIEF_NONE);
                        g_signal_connect ( G_OBJECT (b), "toggled", G_CALLBACK (sp_text_edit_dialog_any_toggled), dlg);
                        gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (b), FALSE );
                        gtk_container_add (GTK_CONTAINER (b), px);
                        gtk_box_pack_start (GTK_BOX (row), b, FALSE, FALSE, 0);
                        g_object_set_data (G_OBJECT (dlg), "text_anchor_start", b);
                    }

                    // align center
                    {
                        // TODO - replace with Inkscape-specific call
                        GtkWidget *px = gtk_image_new_from_stock ( GTK_STOCK_JUSTIFY_CENTER, GTK_ICON_SIZE_LARGE_TOOLBAR );
                        GtkWidget *b = gtk_radio_button_new (gtk_radio_button_group (GTK_RADIO_BUTTON (group)));
                        /* TRANSLATORS: `Center' here is a verb. */
                        gtk_tooltips_set_tip (tt, b, _("Center lines"), NULL);
                        gtk_button_set_relief (GTK_BUTTON (b), GTK_RELIEF_NONE);
                        g_signal_connect ( G_OBJECT (b), "toggled", G_CALLBACK (sp_text_edit_dialog_any_toggled), dlg );
                        gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (b), FALSE);
                        gtk_container_add (GTK_CONTAINER (b), px);
                        gtk_box_pack_start (GTK_BOX (row), b, FALSE, FALSE, 0);
                        g_object_set_data (G_OBJECT (dlg), "text_anchor_middle", b);
                    }

                    // align right
                    {
                        // TODO - replace with Inkscape-specific call
                        GtkWidget *px = gtk_image_new_from_stock ( GTK_STOCK_JUSTIFY_RIGHT, GTK_ICON_SIZE_LARGE_TOOLBAR );
                        GtkWidget *b = gtk_radio_button_new (gtk_radio_button_group (GTK_RADIO_BUTTON (group)));
                        gtk_tooltips_set_tip (tt, b, _("Align lines right"), NULL);
                        gtk_button_set_relief (GTK_BUTTON (b), GTK_RELIEF_NONE);
                        g_signal_connect ( G_OBJECT (b), "toggled", G_CALLBACK (sp_text_edit_dialog_any_toggled), dlg );
                        gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (b), FALSE);
                        gtk_container_add (GTK_CONTAINER (b), px);
                        gtk_box_pack_start (GTK_BOX (row), b, FALSE, FALSE, 0);
                        g_object_set_data (G_OBJECT (dlg), "text_anchor_end", b);
                    }

                    // align justify
                    {
                        // TODO - replace with Inkscape-specific call
                        GtkWidget *px = gtk_image_new_from_stock ( GTK_STOCK_JUSTIFY_FILL, GTK_ICON_SIZE_LARGE_TOOLBAR );
                        GtkWidget *b = gtk_radio_button_new (gtk_radio_button_group (GTK_RADIO_BUTTON (group)));
                        gtk_tooltips_set_tip (tt, b, _("Justify lines"), NULL);
                        gtk_button_set_relief (GTK_BUTTON (b), GTK_RELIEF_NONE);
                        g_signal_connect ( G_OBJECT (b), "toggled", G_CALLBACK (sp_text_edit_dialog_any_toggled), dlg );
                        gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (b), FALSE);
                        gtk_container_add (GTK_CONTAINER (b), px);
                        gtk_box_pack_start (GTK_BOX (row), b, FALSE, FALSE, 0);
                        g_object_set_data (G_OBJECT (dlg), "text_anchor_justify", b);
                    }

                    gtk_box_pack_start (GTK_BOX (l_vb), row, FALSE, FALSE, 0);
                }


                {
                    GtkWidget *row = gtk_hbox_new (FALSE, VB_MARGIN);
                    GtkWidget *group;

                    // horizontal
                    {
                        GtkWidget *px = sp_icon_new( Inkscape::ICON_SIZE_LARGE_TOOLBAR,
                                                      INKSCAPE_STOCK_WRITING_MODE_LR );
                        GtkWidget *b = group = gtk_radio_button_new (NULL);
                        gtk_tooltips_set_tip (tt, b, _("Horizontal text"), NULL);
                        gtk_button_set_relief (GTK_BUTTON (b), GTK_RELIEF_NONE);
                        g_signal_connect ( G_OBJECT (b), "toggled", G_CALLBACK (sp_text_edit_dialog_any_toggled), dlg );
                        gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (b), FALSE);
                        gtk_container_add (GTK_CONTAINER (b), px);
                        gtk_box_pack_start (GTK_BOX (row), b, FALSE, FALSE, 0);
                        g_object_set_data (G_OBJECT (dlg), INKSCAPE_STOCK_WRITING_MODE_LR, b);
                    }

                    // vertical
                    {
                        GtkWidget *px = sp_icon_new( Inkscape::ICON_SIZE_LARGE_TOOLBAR,
                                                      INKSCAPE_STOCK_WRITING_MODE_TB );
                        GtkWidget *b = gtk_radio_button_new (gtk_radio_button_group (GTK_RADIO_BUTTON (group)));
                        gtk_tooltips_set_tip (tt, b, _("Vertical text"), NULL);
                        gtk_button_set_relief (GTK_BUTTON (b), GTK_RELIEF_NONE);
                        g_signal_connect ( G_OBJECT (b), "toggled", G_CALLBACK (sp_text_edit_dialog_any_toggled), dlg );
                        gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (b), FALSE);
                        gtk_container_add (GTK_CONTAINER (b), px);
                        gtk_box_pack_start (GTK_BOX (row), b, FALSE, FALSE, 0);
                        g_object_set_data (G_OBJECT (dlg), INKSCAPE_STOCK_WRITING_MODE_TB, b);
                    }

                    gtk_box_pack_start (GTK_BOX (l_vb), row, FALSE, FALSE, 0);
                }

                {
                    GtkWidget *row = gtk_hbox_new (FALSE, VB_MARGIN);

                    l = gtk_label_new (_("Line spacing:"));
                    gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
                    gtk_box_pack_start (GTK_BOX (row), l, FALSE, FALSE, VB_MARGIN);

                    gtk_box_pack_start (GTK_BOX (l_vb), row, FALSE, FALSE, 0);
                }

                {
                    GtkWidget *row = gtk_hbox_new (FALSE, VB_MARGIN);

                    GtkWidget *c = gtk_combo_new ();
                    gtk_combo_set_value_in_list ((GtkCombo *) c, FALSE, FALSE);
                    gtk_combo_set_use_arrows ((GtkCombo *) c, TRUE);
                    gtk_combo_set_use_arrows_always ((GtkCombo *) c, TRUE);
                    gtk_widget_set_size_request (c, 90, -1);

                    { /* Setup strings */
                        GList *sl = NULL;
                        for (int i = 0; spacings[i]; i++) {
                            sl = g_list_prepend (sl, (void *) spacings[i]);
                        }
                        sl = g_list_reverse (sl);
                        gtk_combo_set_popdown_strings ((GtkCombo *) c, sl);
                        g_list_free (sl);
                    }

                    g_signal_connect ( (GObject *) ((GtkCombo *) c)->entry,
                                       "changed",
                                       (GCallback) sp_text_edit_dialog_line_spacing_changed,
                                       dlg );
                    gtk_box_pack_start (GTK_BOX (row), c, FALSE, FALSE, VB_MARGIN);
                    g_object_set_data (G_OBJECT (dlg), "line_spacing", c);

                    gtk_box_pack_start (GTK_BOX (l_vb), row, FALSE, FALSE, VB_MARGIN);
                }
            }

            /* Font preview */
            GtkWidget *preview = sp_font_preview_new ();
            gtk_box_pack_start (GTK_BOX (vb), preview, TRUE, TRUE, 4);
            g_object_set_data (G_OBJECT (dlg), "preview", preview);
        }


        // Text tab
        {
            GtkWidget *l = gtk_label_new (_("Text"));
            GtkWidget *vb = gtk_vbox_new (FALSE, VB_MARGIN);
            gtk_container_set_border_width (GTK_CONTAINER (vb), VB_MARGIN);
            gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);

            GtkWidget *scroller = gtk_scrolled_window_new ( NULL, NULL );
            gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW (scroller),
                                             GTK_POLICY_AUTOMATIC,
                                             GTK_POLICY_AUTOMATIC );
            gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scroller), GTK_SHADOW_IN );
            gtk_widget_show (scroller);

            GtkTextBuffer *tb = gtk_text_buffer_new (NULL);
            GtkWidget *txt = gtk_text_view_new_with_buffer (tb);
            gtk_text_view_set_wrap_mode ((GtkTextView *) txt, GTK_WRAP_WORD);
#ifdef WITH_GTKSPELL
            GError *error = NULL;
            char *errortext = NULL;
            /* todo: Use computed xml:lang attribute of relevant element, if present, to specify the
               language (either as 2nd arg of gtkspell_new_attach, or with explicit
               gtkspell_set_language call in; see advanced.c example in gtkspell docs).
               sp_text_edit_dialog_read_selection looks like a suitable place. */
            if (gtkspell_new_attach(GTK_TEXT_VIEW(txt), NULL, &error) == NULL) {
                g_print("gtkspell error: %s\n", error->message);
                errortext = g_strdup_printf("GtkSpell was unable to initialize.\n"
                                            "%s", error->message);
                g_error_free(error);
            }
#endif
            gtk_widget_set_size_request (txt, -1, 64);
            gtk_text_view_set_editable (GTK_TEXT_VIEW (txt), TRUE);
            gtk_container_add (GTK_CONTAINER (scroller), txt);
            gtk_box_pack_start (GTK_BOX (vb), scroller, TRUE, TRUE, 0);
            g_signal_connect ( G_OBJECT (tb), "changed",
                               G_CALLBACK (sp_text_edit_dialog_text_changed), dlg );
            g_signal_connect (G_OBJECT (txt), "focus-in-event", G_CALLBACK (text_view_focus_in), dlg);
            g_signal_connect (G_OBJECT (txt), "focus-out-event", G_CALLBACK (text_view_focus_out), dlg);
            g_object_set_data (G_OBJECT (dlg), "text", tb);
            g_object_set_data (G_OBJECT (dlg), "textw", txt);
        }

        /* Buttons */
        GtkWidget *hb = gtk_hbox_new (FALSE, VB_MARGIN);
        gtk_container_set_border_width (GTK_CONTAINER (hb), 4);
        gtk_box_pack_start (GTK_BOX (mainvb), hb, FALSE, FALSE, 0);

        {
            GtkWidget *b = gtk_button_new_with_label (_("Set as default"));
            g_signal_connect ( G_OBJECT (b), "clicked",
                               G_CALLBACK (sp_text_edit_dialog_set_default),
                               dlg );
            gtk_box_pack_start (GTK_BOX (hb), b, FALSE, FALSE, 0);
            g_object_set_data (G_OBJECT (dlg), "default", b);
        }

        {
            GtkWidget *b = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
            g_signal_connect ( G_OBJECT (b), "clicked",
                               G_CALLBACK (sp_text_edit_dialog_close), dlg );
            gtk_box_pack_end (GTK_BOX (hb), b, FALSE, FALSE, 0);
        }

        {
            GtkWidget *b = gtk_button_new_from_stock (GTK_STOCK_APPLY);
            GTK_WIDGET_SET_FLAGS (b, GTK_CAN_DEFAULT | GTK_HAS_DEFAULT);
            g_signal_connect ( G_OBJECT (b), "clicked",
                               G_CALLBACK (sp_text_edit_dialog_apply), dlg );
            gtk_box_pack_end ( GTK_BOX (hb), b, FALSE, FALSE, 0 );
            g_object_set_data (G_OBJECT (dlg), "apply", b);
        }

        g_signal_connect ( G_OBJECT (INKSCAPE), "modify_selection",
                           G_CALLBACK (sp_text_edit_dialog_selection_modified), dlg);
        g_signal_connect ( G_OBJECT (INKSCAPE), "change_selection",
                           G_CALLBACK (sp_text_edit_dialog_selection_changed), dlg);
        g_signal_connect (INKSCAPE, "change_subselection", G_CALLBACK (sp_text_edit_dialog_subselection_changed), dlg);

        gtk_widget_show_all (dlg);

        sp_text_edit_dialog_read_selection (dlg, TRUE, TRUE);
    }

    gtk_window_present ((GtkWindow *) dlg);

} // end of sp_text_edit_dialog()



static void
sp_text_edit_dialog_selection_modified( Inkscape::Application */*inkscape*/,
                                        Inkscape::Selection */*sel*/,
                                        guint flags,
                                        GtkWidget *dlg )
{
    gboolean style, content;

    style =
        ((flags & ( SP_OBJECT_CHILD_MODIFIED_FLAG |
                    SP_OBJECT_STYLE_MODIFIED_FLAG  )) != 0 );

    content =
        ((flags & ( SP_OBJECT_CHILD_MODIFIED_FLAG |
                    SP_TEXT_CONTENT_MODIFIED_FLAG  )) != 0 );

    sp_text_edit_dialog_read_selection (dlg, style, content);

}



static void
sp_text_edit_dialog_selection_changed( Inkscape::Application */*inkscape*/,
                                       Inkscape::Selection */*sel*/,
                                       GtkWidget *dlg )
{
    sp_text_edit_dialog_read_selection (dlg, TRUE, TRUE);
}

static void sp_text_edit_dialog_subselection_changed( Inkscape::Application */*inkscape*/, SPDesktop */*desktop*/, GtkWidget *dlg )
{
    sp_text_edit_dialog_read_selection (dlg, TRUE, FALSE);
}

static void
sp_text_edit_dialog_update_object_text ( SPItem *text )
{
        GtkTextBuffer *tb;
        GtkTextIter start, end;
        gchar *str;

        tb = (GtkTextBuffer*)g_object_get_data (G_OBJECT (dlg), "text");

        /* write text */
        if (gtk_text_buffer_get_modified (tb)) {
            gtk_text_buffer_get_bounds (tb, &start, &end);
            str = gtk_text_buffer_get_text (tb, &start, &end, TRUE);
            sp_te_set_repr_text_multiline (text, str);
            g_free (str);
            gtk_text_buffer_set_modified (tb, FALSE);
        }
}

SPCSSAttr *
sp_get_text_dialog_style ()
{
        GtkWidget *fontsel = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "fontsel");

        SPCSSAttr *css = sp_repr_css_attr_new ();

        /* font */
        font_instance *font = sp_font_selector_get_font (SP_FONT_SELECTOR (fontsel));

        if ( font ) {
            Glib::ustring fontName = font_factory::Default()->ConstructFontSpecification(font);
            sp_repr_css_set_property (css, "-inkscape-font-specification", fontName.c_str());

            gchar c[256];

            font->Family(c, 256);
            sp_repr_css_set_property (css, "font-family", c);

            font->Attribute( "weight", c, 256);
            sp_repr_css_set_property (css, "font-weight", c);

            font->Attribute("style", c, 256);
            sp_repr_css_set_property (css, "font-style", c);

            font->Attribute("stretch", c, 256);
            sp_repr_css_set_property (css, "font-stretch", c);

            font->Attribute("variant", c, 256);
            sp_repr_css_set_property (css, "font-variant", c);

            Inkscape::CSSOStringStream os;
            os << sp_font_selector_get_size (SP_FONT_SELECTOR (fontsel)) << "px"; // must specify px, see inkscape bug 1221626 and 1610103
            sp_repr_css_set_property (css, "font-size", os.str().c_str());

            font->Unref();
            font=NULL;
        }

        /* Layout */
        GtkWidget *b = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "text_anchor_start");

        // Align Left
        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (b))) {
            sp_repr_css_set_property (css, "text-anchor", "start");
            sp_repr_css_set_property (css, "text-align", "start");
        } else {
            // Align Center
            b = (GtkWidget*)g_object_get_data ( G_OBJECT (dlg),
                                                "text_anchor_middle");
            if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (b))) {
                sp_repr_css_set_property (css, "text-anchor", "middle");
                sp_repr_css_set_property (css, "text-align", "center");
            } else {
                // Align Right
                b = (GtkWidget*)g_object_get_data ( G_OBJECT (dlg),
                                                    "text_anchor_end");
                if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (b))) {
                    sp_repr_css_set_property (css, "text-anchor", "end");
                    sp_repr_css_set_property (css, "text-align", "end");
                } else {
                    // Align Justify
                    sp_repr_css_set_property (css, "text-anchor", "start");
                    sp_repr_css_set_property (css, "text-align", "justify");
                }
            }
        }

        b = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), INKSCAPE_STOCK_WRITING_MODE_LR );

        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (b))) {
            sp_repr_css_set_property (css, "writing-mode", "lr");
        } else {
            sp_repr_css_set_property (css, "writing-mode", "tb");
        }

        // Note that CSS 1.1 does not support line-height; we set it for consistency, but also set
        // sodipodi:linespacing for backwards compatibility; in 1.2 we use line-height for flowtext
        GtkWidget *combo = (GtkWidget*)g_object_get_data ((GObject *) dlg, "line_spacing");
        const char *sstr = gtk_entry_get_text ((GtkEntry *) ((GtkCombo *) (combo))->entry);
        sp_repr_css_set_property (css, "line-height", sstr);

        return css;
}


static void
sp_text_edit_dialog_set_default( GtkButton */*button*/, GtkWidget *dlg )
{
    GtkWidget *def = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "default");

    SPCSSAttr *css = sp_get_text_dialog_style ();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    g_object_set_data (G_OBJECT (dlg), "blocked", GINT_TO_POINTER (TRUE));
    prefs->mergeStyle("/tools/text/style", css);
    g_object_set_data (G_OBJECT (dlg), "blocked", GINT_TO_POINTER (FALSE));

    sp_repr_css_attr_unref (css);

    gtk_widget_set_sensitive (def, FALSE);
}



static void
sp_text_edit_dialog_apply( GtkButton */*button*/, GtkWidget *dlg )
{
    g_object_set_data (G_OBJECT (dlg), "blocked", GINT_TO_POINTER (TRUE));

    GtkWidget *apply = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "apply");
    GtkWidget *def = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "default");
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    unsigned items = 0;
    const GSList *item_list = sp_desktop_selection(desktop)->itemList();
    SPCSSAttr *css = sp_get_text_dialog_style ();
    sp_desktop_set_style(desktop, css, true);

    for (; item_list != NULL; item_list = item_list->next) {
        // apply style to the reprs of all text objects in the selection
        if (SP_IS_TEXT (item_list->data)) {

            // backwards compatibility:
            SP_OBJECT_REPR(item_list->data)->setAttribute("sodipodi:linespacing", sp_repr_css_property (css, "line-height", NULL));

            ++items;
        }
        else if (SP_IS_FLOWTEXT (item_list->data))
            // no need to set sodipodi:linespacing, because Inkscape never supported it on flowtext
            ++items;
    }

    if (items == 0) {
        // no text objects; apply style to prefs for new objects
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->mergeStyle("/tools/text/style", css);
        gtk_widget_set_sensitive (def, FALSE);
    } else if (items == 1) {
        /* exactly one text object; now set its text, too */
        SPItem *item = sp_desktop_selection(SP_ACTIVE_DESKTOP)->singleItem();
        if (SP_IS_TEXT (item) || SP_IS_FLOWTEXT(item)) {
            sp_text_edit_dialog_update_object_text (item);
        }
    }

    // complete the transaction
    sp_document_done (sp_desktop_document (SP_ACTIVE_DESKTOP), SP_VERB_CONTEXT_TEXT,
                      _("Set text style"));
    gtk_widget_set_sensitive (apply, FALSE);
    sp_repr_css_attr_unref (css);
    g_object_set_data (G_OBJECT (dlg), "blocked", GINT_TO_POINTER (FALSE));
}

static void
sp_text_edit_dialog_close( GtkButton */*button*/, GtkWidget *dlg )
{
    gtk_widget_destroy (GTK_WIDGET (dlg));
}

static void
sp_text_edit_dialog_read_selection ( GtkWidget *dlg,
                                     gboolean dostyle,
                                     gboolean docontent )
{
    if (g_object_get_data (G_OBJECT (dlg), "blocked"))
        return;

    g_object_set_data (G_OBJECT (dlg), "blocked", GINT_TO_POINTER (TRUE));

    GtkWidget *notebook = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "notebook");
    GtkWidget *textw = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "textw");
    GtkWidget *fontsel = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "fontsel");
    GtkWidget *preview = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "preview");
    GtkWidget *apply = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "apply");
    GtkWidget *def = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "default");

    GtkTextBuffer *tb = (GtkTextBuffer*)g_object_get_data (G_OBJECT (dlg), "text");

    SPItem *text = sp_ted_get_selected_text_item ();

    Inkscape::XML::Node *repr;
    if (text)
    {
        guint items = sp_ted_get_selected_text_count ();
        if (items == 1) {
            gtk_widget_set_sensitive (textw, TRUE);
        } else {
            gtk_widget_set_sensitive (textw, FALSE);
        }
        gtk_widget_set_sensitive (apply, FALSE);
        gtk_widget_set_sensitive (def, TRUE);

        if (docontent) {
            gchar *str;
            str = sp_te_get_string_multiline (text);

            if (str) {
                int pos;
                pos = 0;

                if (items == 1) {
                    gtk_text_buffer_set_text (tb, str, strlen (str));
                    gtk_text_buffer_set_modified (tb, FALSE);
                }
                sp_font_preview_set_phrase (SP_FONT_PREVIEW (preview), str);
                g_free (str);

            } else {
                gtk_text_buffer_set_text (tb, "", 0);
                sp_font_preview_set_phrase (SP_FONT_PREVIEW (preview), NULL);
            }
        } // end of if (docontent)
        repr = SP_OBJECT_REPR (text);

    } else {
        gtk_widget_set_sensitive (textw, FALSE);
        gtk_widget_set_sensitive (apply, FALSE);
        gtk_widget_set_sensitive (def, FALSE);
    }

    if (dostyle) {

        // create temporary style
        SPStyle *query = sp_style_new (SP_ACTIVE_DOCUMENT);
        // query style from desktop into it. This returns a result flag and fills query with the style of subselection, if any, or selection
        //int result_fontspec = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONT_SPECIFICATION);
        int result_family = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTFAMILY);
        int result_style = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTSTYLE);
        int result_numbers = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FONTNUMBERS);

        // If querying returned nothing, read the style from the text tool prefs (default style for new texts)
        // (Ok to not get a font specification - must just rely on the family and style in that case)
        if (result_family == QUERY_STYLE_NOTHING || result_style == QUERY_STYLE_NOTHING
                || result_numbers == QUERY_STYLE_NOTHING) {
            sp_style_read_from_prefs(query, "/tools/text");
        }

        // FIXME: process result_family/style == QUERY_STYLE_MULTIPLE_DIFFERENT by showing "Many" in the lists

        // Get a font_instance using the font-specification attribute stored in SPStyle if available
        font_instance *font = font_factory::Default()->FaceFromStyle(query);


        if (font) {
            // the font is oversized, so we need to pass the true size separately
            sp_font_selector_set_font (SP_FONT_SELECTOR (fontsel), font, query->font_size.computed);
            sp_font_preview_set_font (SP_FONT_PREVIEW (preview), font, SP_FONT_SELECTOR(fontsel));
            font->Unref();
            font=NULL;
        }

        GtkWidget *b;
        if (query->text_anchor.computed == SP_CSS_TEXT_ANCHOR_START) {
            if (query->text_align.computed == SP_CSS_TEXT_ALIGN_JUSTIFY) {
                b = (GtkWidget*)g_object_get_data ( G_OBJECT (dlg), "text_anchor_justify" );
            } else {
                b = (GtkWidget*)g_object_get_data ( G_OBJECT (dlg), "text_anchor_start" );
            }
        } else if (query->text_anchor.computed == SP_CSS_TEXT_ANCHOR_MIDDLE) {
            b = (GtkWidget*)g_object_get_data ( G_OBJECT (dlg), "text_anchor_middle" );
        } else {
            b = (GtkWidget*)g_object_get_data ( G_OBJECT (dlg), "text_anchor_end" );
        }
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b), TRUE);

        if (query->writing_mode.computed == SP_CSS_WRITING_MODE_LR_TB) {
            b = (GtkWidget*)g_object_get_data ( G_OBJECT (dlg), INKSCAPE_STOCK_WRITING_MODE_LR );
        } else {
            b = (GtkWidget*)g_object_get_data ( G_OBJECT (dlg), INKSCAPE_STOCK_WRITING_MODE_TB );
        }
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b), TRUE);

        GtkWidget *combo = (GtkWidget*)g_object_get_data ( G_OBJECT (dlg), "line_spacing" );
        double height;
        if (query->line_height.normal) height = Inkscape::Text::Layout::LINE_HEIGHT_NORMAL;
        else if (query->line_height.unit == SP_CSS_UNIT_PERCENT)
            height = query->line_height.value;
        else height = query->line_height.computed;
        gchar *sstr = g_strdup_printf ("%d%%", (int) floor(height * 100 + 0.5));
        gtk_entry_set_text ((GtkEntry *) ((GtkCombo *) (combo))->entry, sstr);
        g_free(sstr);

        sp_style_unref(query);
    }

    g_object_set_data (G_OBJECT (dlg), "blocked", NULL);
}


static void
sp_text_edit_dialog_text_changed (GtkTextBuffer *tb, GtkWidget *dlg)
{
    GtkWidget *textw, *preview, *apply, *def;
    GtkTextIter start, end;
    gchar *str;

    if (g_object_get_data (G_OBJECT (dlg), "blocked"))
        return;

    SPItem *text = sp_ted_get_selected_text_item ();

    textw = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "textw");
    preview = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "preview");
    apply = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "apply");
    def = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "default");

    gtk_text_buffer_get_bounds (tb, &start, &end);
    str = gtk_text_buffer_get_text (tb, &start, &end, TRUE);

    if (str && *str) {
        sp_font_preview_set_phrase (SP_FONT_PREVIEW (preview), str);
    } else {
        sp_font_preview_set_phrase (SP_FONT_PREVIEW (preview), NULL);
    }
    g_free (str);

    if (text) {
        gtk_widget_set_sensitive (apply, TRUE);
    }
    gtk_widget_set_sensitive (def, TRUE);

} // end of sp_text_edit_dialog_text_changed()

void
sp_text_edit_dialog_default_set_insensitive ()
{
    if (!dlg) return;
    gpointer data = g_object_get_data (G_OBJECT (dlg), "default");
    if (!data) return;
    gtk_widget_set_sensitive (GTK_WIDGET (data), FALSE);
}

static void
sp_text_edit_dialog_font_changed ( SPFontSelector *fsel,
                                   font_instance *font,
                                   GtkWidget *dlg )
{
    GtkWidget *preview, *apply, *def;

    if (g_object_get_data (G_OBJECT (dlg), "blocked"))
        return;

    SPItem *text = sp_ted_get_selected_text_item ();

    preview = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "preview");
    apply = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "apply");
    def = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "default");

    sp_font_preview_set_font (SP_FONT_PREVIEW (preview), font, SP_FONT_SELECTOR(fsel));

    if (text)
    {
        gtk_widget_set_sensitive (apply, TRUE);
    }
    gtk_widget_set_sensitive (def, TRUE);

} // end of sp_text_edit_dialog_font_changed()



static void
sp_text_edit_dialog_any_toggled( GtkToggleButton */*tb*/, GtkWidget *dlg )
{
    GtkWidget *apply, *def;

    if (g_object_get_data (G_OBJECT (dlg), "blocked"))
        return;

    SPItem *text = sp_ted_get_selected_text_item ();

    apply = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "apply");
    def = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "default");

    if (text) {
        gtk_widget_set_sensitive (apply, TRUE);
    }
    gtk_widget_set_sensitive (def, TRUE);
}



static void
sp_text_edit_dialog_line_spacing_changed( GtkEditable */*editable*/, GtkWidget *dlg )
{
    GtkWidget *apply, *def;

    if (g_object_get_data ((GObject *) dlg, "blocked"))
        return;

    SPItem *text = sp_ted_get_selected_text_item ();

    apply = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "apply");
    def = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "default");

    if (text) {
        gtk_widget_set_sensitive (apply, TRUE);
    }
    gtk_widget_set_sensitive (def, TRUE);
}



static SPItem *
sp_ted_get_selected_text_item (void)
{
    if (!SP_ACTIVE_DESKTOP)
        return NULL;

    for (const GSList *item = sp_desktop_selection(SP_ACTIVE_DESKTOP)->itemList();
         item != NULL;
         item = item->next)
    {
        if (SP_IS_TEXT(item->data) || SP_IS_FLOWTEXT(item->data))
            return SP_ITEM (item->data);
    }

    return NULL;
}



static unsigned
sp_ted_get_selected_text_count (void)
{
    if (!SP_ACTIVE_DESKTOP)
        return 0;

    unsigned int items = 0;

    for (const GSList *item = sp_desktop_selection(SP_ACTIVE_DESKTOP)->itemList();
         item != NULL;
         item = item->next)
    {
        if (SP_IS_TEXT(item->data) || SP_IS_FLOWTEXT(item->data))
            ++items;
    }

    return items;
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
