#define __SP_DISPLAY_SETTINGS_C__

/*
* Inkscape Preferences dialog
*
* Authors:
*   Lauris Kaplinski <lauris@ximian.com>
*   bulia byak
*
* Copyright (C) 2001 Ximian, Inc.
* Copyright (C) 2001-2004 Authors
*
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <gtk/gtk.h>

#include "helper/window.h"
#include "../inkscape.h"
#include "../prefs-utils.h"
#include "dialog-events.h"
#include "../macros.h"
#include "../prefs-utils.h"
#include "../verbs.h"
#include "../interface.h"
#include "../message-stack.h"
#include "../enums.h"
#include "../selcue.h"
#include "../selection.h"
#include "../selection-chemistry.h"
#include "../style.h"
#include "../desktop-handles.h"
#include "../unit-constants.h"
#include "xml/repr.h"
#include "ui/widget/style-swatch.h"




static GtkWidget *dlg = NULL;
static win_data wd;

// impossible original values to make sure they are read from prefs
static gint x = -1000, y = -1000;
static gchar *prefs_path = "dialogs.preferences";

extern gint nr_arena_image_x_sample;
extern gint nr_arena_image_y_sample;

#define SB_WIDTH 90
#define SB_LONG_ADJUSTMENT 20
#define SB_MARGIN 1
#define SUFFIX_WIDTH 70
#define HB_MARGIN 4
#define VB_MARGIN 4
#define VB_SKIP 1

static void
sp_display_dialog_destroy (GtkObject *object, gpointer data)
{

    sp_signal_disconnect_by_data (INKSCAPE, dlg);
    wd.win = dlg = NULL;
    wd.stop = 0;

} // edn of sp_display_dialog_destroy()



static gboolean
sp_display_dialog_delete (GtkObject *object, GdkEvent *event, gpointer data)
{
    gtk_window_get_position ((GtkWindow *) dlg, &x, &y);

    prefs_set_int_attribute (prefs_path, "x", x);
    prefs_set_int_attribute (prefs_path, "y", y);

    return FALSE; // which means, go ahead and destroy it

} // end of sp_display_dialog_delete()

static void
prefs_switch_page (GtkNotebook *notebook,
							  GtkNotebookPage *page,
							  guint page_num,
							  gchar *attr)
{
     prefs_set_int_attribute ("dialogs.preferences", attr, page_num);
}

static gint
get_int_value_data(GtkToggleButton *button)
{
    return GPOINTER_TO_INT((gchar const*)gtk_object_get_data(GTK_OBJECT(button), "value"));
}

static void
options_selector_show_toggled (GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active (button)) {
		gchar const *val = (gchar const*)gtk_object_get_data(GTK_OBJECT(button), "value");
		prefs_set_string_attribute ("tools.select", "show", val);
	}
}

static void
options_store_transform_toggled (GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active (button)) {
		guint const val = get_int_value_data(button);
		prefs_set_int_attribute ("options.preservetransform", "value", val);
	}
}

static void
options_clone_compensation_toggled (GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active (button)) {
		guint const val = get_int_value_data(button);
		prefs_set_int_attribute ("options.clonecompensation", "value", val);
	}
}

static void
options_clone_orphans_toggled (GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active (button)) {
		guint const val = get_int_value_data(button);
		prefs_set_int_attribute ("options.cloneorphans", "value", val);
	}
}

static void
options_selcue_toggled (GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active (button)) {
		guint const val = get_int_value_data(button);
		prefs_set_int_attribute ("options.selcue", "value", val);
	}
}

static void
options_scale_origin_toggled (GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active (button)) {
                gchar const *val = (gchar const *) gtk_object_get_data(GTK_OBJECT(button), "value");
		prefs_set_string_attribute ("tools.select", "scale_origin", val);
	}
}


/**
* Small helper function to make options_selector a little less
* verbose.
*
* \param b Another radio button in the group, or NULL for the first.
* \param fb Box to add the button to.
* \param n Label for the button.
* \param tip Tooltip.
* \param v_string Key for the button's value, if it is a string.
* \param v_uint Key for the button's value, if it is a uint.
* \param isint Whether this is astring or uint.
* \param s Initial state of the button.
* \param h Toggled handler function.
*/
static GtkWidget* sp_select_context_add_radio (
    GtkWidget *b,
    GtkWidget *fb,
    GtkTooltips *tt,
    gchar const *n,
    gchar const *tip,
    char const *v_string,
    guint v_uint,
    bool isint,
    gboolean s,
    void (*h)(GtkToggleButton*)
    )
{
	GtkWidget* r = gtk_radio_button_new_with_label (
            b ? gtk_radio_button_group (GTK_RADIO_BUTTON (b)) : NULL, n
            );
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), r, tip, NULL);
	gtk_widget_show (r);

  if (isint)
	gtk_object_set_data (GTK_OBJECT (r), "value", GUINT_TO_POINTER (v_uint));
  else
	gtk_object_set_data (GTK_OBJECT (r), "value", (void*) v_string);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (r), s);
	gtk_box_pack_start (GTK_BOX (fb), r, FALSE, FALSE, 0);
	gtk_signal_connect (GTK_OBJECT (r), "toggled", GTK_SIGNAL_FUNC (h), NULL);

       return r;
}

static GtkWidget *
options_selector ()
{
    GtkWidget *vb, *f, *fb, *b;

    GtkTooltips *tt = gtk_tooltips_new();

    vb = gtk_vbox_new (FALSE, VB_MARGIN);

    f = gtk_frame_new (_("When transforming, show:"));
    gtk_widget_show (f);
    gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

    fb = gtk_hbox_new (FALSE, 10);
    gtk_widget_show (fb);
    gtk_container_add (GTK_CONTAINER (f), fb);

    gchar const *show = prefs_get_string_attribute ("tools.select", "show");

    b = sp_select_context_add_radio (
        NULL, fb, tt, _("Objects"),
        _("Show the actual objects when moving or transforming"),
        "content", 0, false,
        (show == NULL) || !strcmp (show, "content"),
        options_selector_show_toggled
        );

    sp_select_context_add_radio(
        b, fb, tt, _("Box outline"),
        _("Show only a box outline of the objects when moving or transforming"),
        "outline",  0, false,
        show && !strcmp (show, "outline"),
        options_selector_show_toggled
        );

    f = gtk_frame_new (_("Per-object selection cue:"));
    gtk_widget_show (f);
    gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

    fb = gtk_hbox_new (FALSE, 10);
    gtk_widget_show (fb);
    gtk_container_add (GTK_CONTAINER (f), fb);

    gint cue = prefs_get_int_attribute ("options.selcue", "value", Inkscape::SelCue::MARK);

    b = sp_select_context_add_radio (
        NULL, fb, tt, _("None"),
        _("No per-object selection indication"), NULL, Inkscape::SelCue::NONE, true,
        cue == Inkscape::SelCue::NONE,
        options_selcue_toggled
        );

    b = sp_select_context_add_radio (
        b, fb, tt, _("Mark"),
        _("Each selected object has a diamond mark in the top left corner"),
        NULL, Inkscape::SelCue::MARK, true,
        cue == Inkscape::SelCue::MARK,
        options_selcue_toggled
        );

    sp_select_context_add_radio (
        b, fb, tt, _("Box"),
        _("Each selected object displays its bounding box"), NULL, Inkscape::SelCue::BBOX, true,
        cue == Inkscape::SelCue::BBOX,
        options_selcue_toggled
        );

    f = gtk_frame_new (_("Default scale origin:"));
    gtk_widget_show (f);
    gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

    fb = gtk_hbox_new (FALSE, 10);
    gtk_widget_show (fb);
    gtk_container_add (GTK_CONTAINER (f), fb);

    gchar const *scale_orig = prefs_get_string_attribute ("tools.select", "scale_origin");

    b = sp_select_context_add_radio (
        NULL, fb, tt, _("Opposite bounding box edge"),
        _("Default scale origin will be on the bounding box of the item"), "bbox", 0, false,
        (scale_orig == NULL) || !strcmp (scale_orig, "bbox"),
        options_scale_origin_toggled
        );

    sp_select_context_add_radio (
        b, fb, tt, _("Farthest opposite node"),
        _("Default scale origin will be on the bounding box of the item's points"),
        "points", 0, false,
        scale_orig && !strcmp (scale_orig, "points"),
        options_scale_origin_toggled
        );

    return vb;
}


static void
sp_display_dialog_set_oversample (GtkMenuItem *item, gpointer data)
{
    gint os;

    os = GPOINTER_TO_INT (data);

    g_return_if_fail (os >= 0);
    g_return_if_fail (os <= 4);

    nr_arena_image_x_sample = os;
    nr_arena_image_y_sample = os;

    inkscape_refresh_display (INKSCAPE);

    prefs_set_int_attribute ( "options.bitmapoversample", "value", os );

}


static void
options_rotation_steps_changed (GtkMenuItem *item, gpointer data)
{
    gint snaps_new = GPOINTER_TO_INT (data);
    prefs_set_int_attribute ( "options.rotationsnapsperpi", "value", snaps_new );
}

static void
options_dialogs_ontop_changed (GtkMenuItem *item, gpointer data)
{
    gint policy_new = GPOINTER_TO_INT (data);
    prefs_set_int_attribute ( "options.transientpolicy", "value", policy_new );
}

void
options_rotation_steps (GtkWidget *vb, GtkTooltips *tt)
{
    GtkWidget *hb = gtk_hbox_new (FALSE, HB_MARGIN);
    gtk_widget_show (hb);
    gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

    {
        GtkWidget *l = gtk_label_new (_("degrees"));
        gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
        gtk_widget_set_size_request (l, SUFFIX_WIDTH, -1);
        gtk_widget_show (l);
        gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 0);
    }

    {
        GtkWidget *om = gtk_option_menu_new ();
        gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), om, _("Rotating with Ctrl pressed snaps every that much degrees; also, pressing [ or ] rotates by this amount"), NULL);
        gtk_widget_set_size_request (om, SB_WIDTH, -1);
        gtk_widget_show (om);
        gtk_box_pack_end (GTK_BOX (hb), om, FALSE, FALSE, SB_MARGIN);

        GtkWidget *m = gtk_menu_new ();
        gtk_widget_show (m);

        int snaps_current = prefs_get_int_attribute ("options.rotationsnapsperpi", "value", 12);
        int position_current = 0;

        struct RotSteps {
            double degrees;
            int snaps;
        } const rot_snaps[] = {
            {90, 2},
            {60, 3},
            {45, 4},
            {30, 6},
            {15, 12},
            {10, 18},
            {7.5, 24},
            {6, 30},
            {5, 36},
            {3, 60},
            {2, 90},
            {1, 180},
            {1, 0},
        };

        for (unsigned j = 0; j < G_N_ELEMENTS(rot_snaps); ++j) {
            RotSteps const &rs = rot_snaps[j];

            gchar const *label = NULL;
            if (rs.snaps == 0) {
                // sorationsnapsperpi == 0 means no snapping
                label = _("None");
            } else {
                label = g_strdup_printf ("%.2g", rs.degrees);
            }

            if (rs.snaps == snaps_current)
                position_current = j;

            GtkWidget *item = gtk_menu_item_new_with_label (label);
            gtk_signal_connect ( GTK_OBJECT (item), "activate",
                                 GTK_SIGNAL_FUNC (options_rotation_steps_changed),
                                 GINT_TO_POINTER (rs.snaps) );
            gtk_widget_show (item);
            gtk_menu_append (GTK_MENU (m), item);
        }

        gtk_option_menu_set_menu (GTK_OPTION_MENU (om), m);
        gtk_option_menu_set_history ( GTK_OPTION_MENU (om), position_current);
    }

    {
        GtkWidget *l = gtk_label_new (_("Rotation snaps every:"));
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_widget_show (l);
        gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
    }
}

void
options_dialogs_ontop (GtkWidget *vb, GtkTooltips *tt)
{
    GtkWidget *hb = gtk_hbox_new (FALSE, HB_MARGIN);
    gtk_widget_show (hb);
    gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

    { // empty label for alignment
        GtkWidget *l = gtk_label_new ("");
        gtk_widget_set_size_request (l, SUFFIX_WIDTH - SB_LONG_ADJUSTMENT, -1);
        gtk_widget_show (l);
        gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 0);
    }

    {
        GtkWidget *om = gtk_option_menu_new ();
        gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), om, _("None: dialogs are treated as regular windows; Normal: dialogs stay on top of document windows; Aggressive: same as Normal but may work better with some window managers."), NULL);
        gtk_widget_set_size_request (om, SB_WIDTH + SB_LONG_ADJUSTMENT, -1);
        gtk_widget_show (om);
        gtk_box_pack_end (GTK_BOX (hb), om, FALSE, FALSE, SB_MARGIN);

        GtkWidget *m = gtk_menu_new ();
        gtk_widget_show (m);

        int current = prefs_get_int_attribute ("options.transientpolicy", "value", 1);

        {
        const gchar *label = _("None");
        GtkWidget *item = gtk_menu_item_new_with_label (label);
        gtk_signal_connect ( GTK_OBJECT (item), "activate",
                                 GTK_SIGNAL_FUNC (options_dialogs_ontop_changed),
                                 GINT_TO_POINTER (0) );
        gtk_widget_show (item);
        gtk_menu_append (GTK_MENU (m), item);
        }

        {
        const gchar *label = _("Normal");
        GtkWidget *item = gtk_menu_item_new_with_label (label);
        gtk_signal_connect ( GTK_OBJECT (item), "activate",
                                 GTK_SIGNAL_FUNC (options_dialogs_ontop_changed),
                                 GINT_TO_POINTER (1) );
        gtk_widget_show (item);
        gtk_menu_append (GTK_MENU (m), item);
        }

        {
        const gchar *label = _("Aggressive");
        GtkWidget *item = gtk_menu_item_new_with_label (label);
        gtk_signal_connect ( GTK_OBJECT (item), "activate",
                                 GTK_SIGNAL_FUNC (options_dialogs_ontop_changed),
                                 GINT_TO_POINTER (2) );
        gtk_widget_show (item);
        gtk_menu_append (GTK_MENU (m), item);
        }

        gtk_option_menu_set_menu (GTK_OPTION_MENU (om), m);
        gtk_option_menu_set_history ( GTK_OPTION_MENU (om), current);
    }

    {
        GtkWidget *l = gtk_label_new (_("Dialogs on top:"));
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_widget_show (l);
        gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
    }
}


static void
sp_display_dialog_cursor_tolerance_changed (GtkAdjustment *adj, gpointer data)
{
    prefs_set_double_attribute ( "options.cursortolerance", "value",
                                 adj->value );
}

static void
options_freehand_tolerance_changed (GtkAdjustment *adj, gpointer data)
{
    prefs_set_double_attribute ("tools.freehand.pencil", "tolerance",  adj->value);
}

static void
options_changed_double (GtkAdjustment *adj, gpointer data)
{
    const gchar *prefs_path = (const gchar *) data;
    prefs_set_double_attribute (prefs_path, "value",  adj->value);
}

static void
options_changed_int (GtkAdjustment *adj, gpointer data)
{
    const gchar *prefs_path = (const gchar *) data;
    prefs_set_int_attribute (prefs_path, "value",  (int) adj->value);
}

static void
options_changed_percent (GtkAdjustment *adj, gpointer data)
{
    const gchar *prefs_path = (const gchar *) data;
    prefs_set_double_attribute (prefs_path, "value",  (adj->value)/100.0);
}

static void
options_changed_boolean (GtkToggleButton *tb, gpointer data)
{
    const gchar *prefs_path = (const gchar *) data;
    const gchar *prefs_attr = (const gchar *) g_object_get_data (G_OBJECT(tb), "attr");
    prefs_set_int_attribute (prefs_path, prefs_attr, gtk_toggle_button_get_active (tb));
}

void
options_sb (
    gchar const *label,
    gchar const *tooltip, GtkTooltips *tt,
    gchar const *suffix,
    GtkWidget *box,
    gdouble lower, gdouble upper, gdouble step_increment, gdouble page_increment, gdouble page_size,
    gchar const *prefs_path, gchar const *attr, gdouble def,
    bool isint, bool ispercent,
    void (*changed)(GtkAdjustment *, gpointer)
)
{
    GtkWidget *hb = gtk_hbox_new (FALSE, HB_MARGIN);
    gtk_widget_show (hb);
    gtk_box_pack_start (GTK_BOX (box), hb, FALSE, FALSE, VB_SKIP);

    {
        GtkWidget *l = gtk_label_new (suffix);
        gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
        gtk_widget_set_size_request (l, SUFFIX_WIDTH, -1);
        gtk_widget_show (l);
        gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 0);
    }

    {
        GtkObject *a = gtk_adjustment_new(0.0, lower, upper, step_increment, page_increment, page_size);

        gdouble value;
        if (isint)
            if (ispercent)
                value = 100 * (gdouble) prefs_get_double_attribute_limited (prefs_path, attr, def, lower/100.0, upper/100.0);
            else
                value = (gdouble) prefs_get_int_attribute_limited (prefs_path, attr, (int) def, (int) lower, (int) upper);
        else
            value = prefs_get_double_attribute_limited (prefs_path, attr, def, lower, upper);

        gtk_adjustment_set_value (GTK_ADJUSTMENT (a), value);

        GtkWidget *sb;
        if (isint) {
            sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1.0, 0);
        } else {
            if (step_increment < 0.1)
                sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0.01, 3);
            else
                sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0.01, 2);
        }

        gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), sb, tooltip, NULL);
        gtk_entry_set_width_chars (GTK_ENTRY (sb), 6);
        gtk_widget_set_size_request (sb, SB_WIDTH, -1);
        gtk_widget_show (sb);
        gtk_box_pack_end (GTK_BOX (hb), sb, FALSE, FALSE, SB_MARGIN);

        gtk_signal_connect(GTK_OBJECT(a), "value_changed",
                           GTK_SIGNAL_FUNC(changed), (gpointer) prefs_path);
    }

    {
        GtkWidget *l = gtk_label_new (label);
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_widget_show (l);
        gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
    }

}


void
options_checkbox (
    gchar const *label,
    gchar const *tooltip, GtkTooltips *tt,
    GtkWidget *box,
    gchar const *prefs_path, gchar const *attr, gint def,
    void (*changed)(GtkToggleButton *, gpointer)
)
{
    GtkWidget *hb = gtk_hbox_new (FALSE, HB_MARGIN);
    gtk_widget_show (hb);
    gtk_box_pack_start (GTK_BOX (box), hb, FALSE, FALSE, VB_SKIP);

    { // empty label for alignment
        GtkWidget *l = gtk_label_new ("");
        gtk_widget_set_size_request (l, SUFFIX_WIDTH, -1);
        gtk_widget_show (l);
        gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 0);
    }

    {

        GtkWidget *b  = gtk_check_button_new ();

        gint value = prefs_get_int_attribute (prefs_path, attr, def);

        gtk_toggle_button_set_active ((GtkToggleButton *) b, value != 0);

        gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), b, tooltip, NULL);

        gtk_widget_set_size_request (b, SB_WIDTH, -1);
        gtk_widget_show (b);
        gtk_box_pack_end (GTK_BOX (hb), b, FALSE, FALSE, SB_MARGIN);

        g_object_set_data (G_OBJECT(b), "attr", (void *) attr);

        gtk_signal_connect(GTK_OBJECT(b), "toggled",
                           GTK_SIGNAL_FUNC(changed), (gpointer) prefs_path);
    }

    {
        GtkWidget *l = gtk_label_new (label);
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_widget_show (l);
        gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
    }
}

void
selcue_checkbox (GtkWidget *vb, GtkTooltips *tt, const gchar *path)
{
    options_checkbox (
        _("Show selection cue"),
        _("Whether selected objects display a selection cue (the same as in selector)"), tt,
        vb,
        path, "selcue", 1,
        options_changed_boolean
        );
}

void
gradientdrag_checkbox (GtkWidget *vb, GtkTooltips *tt, const gchar *path)
{
    options_checkbox (
        _("Enable gradient editing"),
        _("Whether selected objects display gradient editing controls"), tt,
        vb,
        path, "gradientdrag", 1,
        options_changed_boolean
        );
}


/**
* Helper function for new_objects_style
*
* \param b Another radio button in the group, or NULL for the first.
* \param fb Box to add the button to.
* \param n Label for the button.
* \param tip Tooltip.
* \param v_uint Key for the button's value
* \param s Initial state of the button.
* \param h Toggled handler function.
*/
static GtkWidget* new_objects_style_add_radio (
    GtkWidget* b,
    GtkWidget* fb,
    GtkTooltips* tt,
    const gchar* n,
    const gchar* tip,
    guint v_uint,
    gboolean s,
    void (*h)(GtkToggleButton*, gpointer),
    gchar const *prefs_path,
    gchar const *attr,
    GtkWidget *button
    )
{
	GtkWidget* r = gtk_radio_button_new_with_label (
            b ? gtk_radio_button_group (GTK_RADIO_BUTTON (b)) : NULL, n
            );
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), r, tip, NULL);
	gtk_widget_show (r);

	gtk_object_set_data (GTK_OBJECT (r), "value", GUINT_TO_POINTER (v_uint));
	gtk_object_set_data (GTK_OBJECT (r), "attr", (gpointer) attr);
	gtk_object_set_data (GTK_OBJECT (r), "button_to_activate", (gpointer) button);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (r), s);
	gtk_signal_connect (GTK_OBJECT (r), "toggled", GTK_SIGNAL_FUNC (h), (gpointer) prefs_path);

       return r;
}

static void
style_from_selection_to_tool(GtkWidget *widget, gchar const *prefs_path)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    Inkscape::Selection *selection = SP_DT_SELECTION(desktop);

    if (selection->isEmpty()) {
        SP_DT_MSGSTACK(desktop)->flash(Inkscape::ERROR_MESSAGE,
                                       _("<b>No objects selected</b> to take the style from."));
        return;
    }
    SPItem *item = selection->singleItem();
    if (!item) {
        /* TODO: If each item in the selection has the same style then don't consider it an error.
         * Maybe we should try to handle multiple selections anyway, e.g. the intersection of the
         * style attributes for the selected items. */
        SP_DT_MSGSTACK(desktop)->flash(Inkscape::ERROR_MESSAGE,
                                       _("<b>More than one object selected.</b>  Cannot take style from multiple objects."));
        return;
    }

    SPCSSAttr *css = take_style_from_item (item);

    if (!css) return;

    // only store text style for the text tool
    if (!g_strrstr ((const gchar *) prefs_path, "text")) {
        css = sp_css_attr_unset_text (css);
    }

    // we cannot store properties with uris - they will be invalid in other documents
    css = sp_css_attr_unset_uris (css);

    sp_repr_css_change (inkscape_get_repr (INKSCAPE, prefs_path), css, "style");
    sp_repr_css_attr_unref (css);

    // update the swatch
    Inkscape::UI::Widget::StyleSwatch *swatch = 
        (Inkscape::UI::Widget::StyleSwatch *) gtk_object_get_data (GTK_OBJECT (widget), "swatch");
    if (swatch) {
        Inkscape::XML::Node *tool_repr = inkscape_get_repr(INKSCAPE, prefs_path);
        if (tool_repr) {
            SPCSSAttr *css = sp_repr_css_attr_inherited(tool_repr, "style");
            
            swatch->setStyle (css);

            sp_repr_css_attr_unref(css);
        }
    }
}

static void
options_changed_radio (GtkToggleButton *tb, gpointer data)
{
    const gchar *prefs_path = (const gchar *) data;
    const gchar *prefs_attr = (const gchar *) g_object_get_data (G_OBJECT(tb), "attr");
    const guint val = GPOINTER_TO_INT((const gchar*)gtk_object_get_data(GTK_OBJECT(tb), "value"));

    if (prefs_path && prefs_attr && gtk_toggle_button_get_active (tb)) {
        prefs_set_int_attribute (prefs_path, prefs_attr, val);
    }

    GtkWidget *button = (GtkWidget *) g_object_get_data (G_OBJECT(tb), "button_to_activate");
    if (button)
        gtk_widget_set_sensitive (button, !val);
}

void
new_objects_style (GtkWidget *vb, GtkTooltips *tt, const gchar *path)
{
    GtkWidget *f = gtk_frame_new (_("Create new objects with:"));
    gtk_widget_show (f);
    gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

    GtkWidget *fb = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (fb);
    gtk_container_add (GTK_CONTAINER (f), fb);

    guint usecurrent = prefs_get_int_attribute (path, "usecurrent", 0);

    GtkWidget *take = gtk_button_new_with_label (_("Take from selection"));
    gtk_tooltips_set_tip (tt, take, _("Remember the style of the (first) selected object as this tool's style"), NULL);
    gtk_widget_show (take);

    GtkWidget *b;
    {
        b = new_objects_style_add_radio (
            NULL, fb, tt, _("Last used style"), _("Apply the style you last set on an object"),
            1,
            usecurrent != 0,
            options_changed_radio,
            path, "usecurrent", take
            );

        GtkWidget *hb = gtk_hbox_new(FALSE, HB_MARGIN);
        gtk_box_pack_start (GTK_BOX (hb), b, FALSE, FALSE, 0);
        gtk_widget_show_all(hb);

        gtk_box_pack_start (GTK_BOX (fb), hb, FALSE, FALSE, 0);
    }

    {
        b = new_objects_style_add_radio (
            b, fb, tt, _("This tool's own style:"), _("Each tool may store its own style to apply to the newly created objects. Use the button below to set it."),
            0,
            usecurrent == 0,
            options_changed_radio,
            path, "usecurrent", take
            );

        GtkWidget *hb = gtk_hbox_new(FALSE, HB_MARGIN);
        gtk_box_pack_start (GTK_BOX (hb), b, FALSE, FALSE, 0);

        // style swatch
        Inkscape::XML::Node *tool_repr = inkscape_get_repr(INKSCAPE, path);
        if (tool_repr) {
            SPCSSAttr *css = sp_repr_css_attr_inherited(tool_repr, "style");
            Inkscape::UI::Widget::StyleSwatch *swatch = new Inkscape::UI::Widget::StyleSwatch(css);
            gtk_box_pack_start (GTK_BOX (hb), (GtkWidget *) swatch->gobj(), FALSE, FALSE, 0);
            sp_repr_css_attr_unref(css);
            gtk_object_set_data (GTK_OBJECT (take), "swatch", (gpointer) swatch);
        }

        // add "take from selection" button
        gtk_widget_set_sensitive (take, (usecurrent == 0));
        gtk_box_pack_start (GTK_BOX (hb), take, FALSE, FALSE, 0);
        gtk_signal_connect (GTK_OBJECT (take), "clicked", GTK_SIGNAL_FUNC (style_from_selection_to_tool), (void *) path);

        gtk_widget_show_all(hb);

        gtk_box_pack_start (GTK_BOX (fb), hb, FALSE, FALSE, 0);
    }
}


static GtkWidget *
options_dropper ()
{
    GtkTooltips *tt = gtk_tooltips_new();

    GtkWidget *vb = gtk_vbox_new (FALSE, VB_MARGIN);

    selcue_checkbox (vb, tt, "tools.dropper");
    gradientdrag_checkbox (vb, tt, "tools.dropper");

    return vb;
}

GtkWidget *
new_tab (GtkWidget *nb, const gchar *label)
{
     GtkWidget *l = gtk_label_new (label);
     gtk_widget_show (l);
     GtkWidget *vb = gtk_vbox_new (FALSE, VB_MARGIN);
     gtk_widget_show (vb);
     gtk_container_set_border_width (GTK_CONTAINER (vb), VB_MARGIN);
     gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);
     return vb;
}

void
sp_display_dialog (void)
{

    GtkWidget *nb, *l, *vb, *vbvb, *hb, *om, *m, *i, *frame;

    if (!dlg)
    {

        gchar title[500];
        sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_DIALOG_DISPLAY), title);

        dlg = sp_window_new (title, TRUE);
        gtk_window_set_resizable ((GtkWindow *) dlg, FALSE);
        if (x == -1000 || y == -1000) {
            x = prefs_get_int_attribute (prefs_path, "x", 0);
            y = prefs_get_int_attribute (prefs_path, "y", 0);
        }

        if (x != 0 || y != 0) {
            gtk_window_move ((GtkWindow *) dlg, x, y);
        } else {
            gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
        }


        sp_transientize (dlg);
        wd.win = dlg;
        wd.stop = 0;

        g_signal_connect   ( G_OBJECT (INKSCAPE), "activate_desktop",
                             G_CALLBACK (sp_transientize_callback), &wd);

        gtk_signal_connect ( GTK_OBJECT (dlg), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg);

        gtk_signal_connect ( GTK_OBJECT (dlg), "destroy", G_CALLBACK (sp_display_dialog_destroy), dlg);
        gtk_signal_connect ( GTK_OBJECT (dlg), "delete_event", G_CALLBACK (sp_display_dialog_delete), dlg);
        g_signal_connect   ( G_OBJECT (INKSCAPE), "shut_down", G_CALLBACK (sp_display_dialog_delete), dlg);

        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), dlg);
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), dlg);

        GtkTooltips *tt = gtk_tooltips_new();

        nb = gtk_notebook_new ();
        gtk_widget_show (nb);
        gtk_container_add (GTK_CONTAINER (dlg), nb);

// Mouse
        vb = new_tab (nb, _("Mouse"));

        options_sb (
            /* TRANSLATORS: "Grab" is a noun here.  "Grab sensitivity" is intended to mean how
             * close on the screen you need to be to an object to be able to grab it with mouse (in
             * pixels). */
            _("Grab sensitivity:"),
            _("How close on the screen you need to be to an object to be able to grab it with mouse (in screen pixels)"), tt,
            _("pixels"),
            /* todo: allow real-world units. */
            vb,
            0.0, 30.0, 1.0, 1.0, 1.0,
            "options.cursortolerance", "value", 8.0,
            true, false,
            sp_display_dialog_cursor_tolerance_changed
            );

        options_sb (
            _("Click/drag threshold:"),
            _("Maximum mouse drag (in screen pixels) which is considered a click, not a drag"), tt,
            _("pixels"),
            vb,
            0.0, 20.0, 1.0, 1.0, 1.0,
            "options.dragtolerance", "value", 4.0,
            true, false,
            options_changed_int
            );


// Scrolling
        vb = new_tab (nb, _("Scrolling"));

        options_sb (
            _("Mouse wheel scrolls by:"),
            _("One mouse wheel notch scrolls by this distance in screen pixels (horizontally with Shift)"), tt,
            _("pixels"),
            vb,
            0.0, 1000.0, 1.0, 1.0, 1.0,
            "options.wheelscroll", "value", 40.0,
            true, false,
            options_changed_int
            );

        frame = gtk_frame_new (_("Ctrl+arrows"));
        gtk_widget_show (frame);
        gtk_box_pack_start (GTK_BOX (vb), frame, FALSE, FALSE, 0);
        vbvb = gtk_vbox_new (FALSE, VB_MARGIN);
        gtk_widget_show (vbvb);
        gtk_container_add (GTK_CONTAINER (frame), vbvb);

        options_sb (
            _("Scroll by:"),
            _("Pressing Ctrl+arrow key scrolls by this distance (in screen pixels)"), tt,
            _("pixels"),
            vbvb,
            0.0, 1000.0, 1.0, 1.0, 1.0,
            "options.keyscroll", "value", 10.0,
            true, false,
            options_changed_int
            );

        options_sb (
            _("Acceleration:"),
            _("Pressing and holding Ctrl+arrow will gradually speed up scrolling (0 for no acceleration)"), tt,
            "",
            vbvb,
            0.0, 5.0, 0.01, 1.0, 1.0,
            "options.scrollingacceleration", "value", 0.35,
            false, false,
            options_changed_double
            );

        frame = gtk_frame_new (_("Autoscrolling"));
        gtk_widget_show (frame);
        gtk_box_pack_start (GTK_BOX (vb), frame, FALSE, FALSE, 0);
        vbvb = gtk_vbox_new (FALSE, VB_MARGIN);
        gtk_widget_show (vbvb);
        gtk_container_add (GTK_CONTAINER (frame), vbvb);

        options_sb (
            _("Speed:"),
            _("How fast the canvas autoscrolls when you drag beyond canvas edge (0 to turn autoscroll off)"), tt,
            "",
            vbvb,
            0.0, 5.0, 0.01, 1.0, 1.0,
            "options.autoscrollspeed", "value", 0.7,
            false, false,
            options_changed_double
            );

        options_sb (
            _("Threshold:"),
            _("How far (in screen pixels) you need to be from the canvas edge to trigger autoscroll; positive is outside the canvas, negative is within the canvas"), tt,
            _("pixels"),
            vbvb,
            -600.0, 600.0, 1.0, 1.0, 1.0,
            "options.autoscrolldistance", "value", -10.0,
            true, false,
            options_changed_int
            );

// Steps
        vb = new_tab (nb, _("Steps"));

        options_sb (
            _("Arrow keys move by:"),
            _("Pressing an arrow key moves selected object(s) or node(s) by this distance (in px units)"), tt,
            _("px"),
            vb,
            0.0, 3000.0, 0.01, 1.0, 1.0,
            "options.nudgedistance", "value", 2.0,
            false, false,
            options_changed_double
            );

        options_sb (
            _("> and < scale by:"),
            _("Pressing > or < scales selection up or down by this increment (in px units)"), tt,
            _("px"),
            vb,
            0.0, 3000.0, 0.01, 1.0, 1.0,
            "options.defaultscale", "value", 2.0,
            false, false,
            options_changed_double
            );

        options_sb (
            _("Inset/Outset by:"),
            _("Inset and Outset commands displace the path by this distance (in px units)"), tt,
            _("px"),
            vb,
            0.0, 3000.0, 0.01, 1.0, 1.0,
            "options.defaultoffsetwidth", "value", 2.0,
            false, false,
            options_changed_double
            );

        options_rotation_steps (vb, tt);

options_checkbox (
    _("Compass-like display of angles"),
    // TRANSLATORS: "positive clockwise" means "increasing in clockwise direction"
    _("When on, angles are displayed with 0 at north, 0 to 360 range, positive clockwise; otherwise with 0 at east, -180 to 180 range, positive counterclockwise"), tt,
    vb,
    "options.compassangledisplay", "value", 1,
    options_changed_boolean
    );

        options_sb (
            _("Zoom in/out by:"),
            _("Zoom tool click, +/- keys, and middle click zoom in and out by this multiplier"), tt,
            _("%"),
            vb,
            101.0, 500.0, 1.0, 1.0, 1.0,
            "options.zoomincrement", "value", 1.414213562,
            true, true,
            options_changed_percent
            );

// Tools
        vb = new_tab (nb, _("Tools"));

        GtkWidget *nb_tools = gtk_notebook_new ();
        gtk_widget_show (nb_tools);
        gtk_container_add (GTK_CONTAINER (vb), nb_tools);

        // Selector
        {
            GtkWidget *vb_tool = new_tab (nb_tools, _("Selector"));

            GtkWidget *selector_page = options_selector ();
            gtk_widget_show (selector_page);
            gtk_container_add (GTK_CONTAINER (vb_tool), selector_page);

            gradientdrag_checkbox (vb_tool, tt, "tools.select");
        }

        // Node
        {
            GtkWidget *vb_tool = new_tab (nb_tools, _("Node"));

            selcue_checkbox (vb_tool, tt, "tools.nodes");
            gradientdrag_checkbox (vb_tool, tt, "tools.nodes");
        }

        // Zoom
        {
            GtkWidget *vb_tool = new_tab (nb_tools, _("Zoom"));

            selcue_checkbox (vb_tool, tt, "tools.zoom");
            gradientdrag_checkbox (vb_tool, tt, "tools.zoom");
        }

        { // The 4 shape tools
            GtkWidget *vb_shapes = new_tab (nb_tools, _("Shapes"));

            GtkWidget *nb_shapes = gtk_notebook_new ();
            gtk_widget_show (nb_shapes);
            gtk_container_add (GTK_CONTAINER (vb_shapes), nb_shapes);

            // Rect
            {
            GtkWidget *vb_tool = new_tab(nb_shapes, _("Rectangle"));
            new_objects_style (vb_tool, tt, "tools.shapes.rect");
            }

            // Ellipse
            {
            GtkWidget *vb_tool = new_tab(nb_shapes, _("Ellipse"));
            new_objects_style (vb_tool, tt, "tools.shapes.arc");
            }

            // Star
            {
            GtkWidget *vb_tool = new_tab(nb_shapes, _("Star"));
            new_objects_style (vb_tool, tt, "tools.shapes.star");
            }

            // Spiral
            {
            GtkWidget *vb_tool = new_tab(nb_shapes, _("Spiral"));
            new_objects_style (vb_tool, tt, "tools.shapes.spiral");
            }

            // common for all shapes
            selcue_checkbox (vb_shapes, tt, "tools.shapes");
            gradientdrag_checkbox (vb_shapes, tt, "tools.shapes");

            g_signal_connect(GTK_OBJECT (nb_shapes), "switch-page", GTK_SIGNAL_FUNC (prefs_switch_page), (void *) "page_shapes");
            gtk_notebook_set_current_page (GTK_NOTEBOOK (nb_shapes), prefs_get_int_attribute ("dialogs.preferences", "page_shapes", 0));
        }

        // Pencil
        {
            GtkWidget *vb_tool = new_tab (nb_tools, _("Pencil"));

            options_sb (
                _("Tolerance:"),
                _("This value affects the amount of smoothing applied to freehand lines; lower values produce more uneven paths with more nodes"), tt,
                "",
                vb_tool,
                0.0, 100.0, 0.5, 1.0, 1.0,
                "tools.freehand.pencil", "tolerance", 10.0,
                false, false,
                options_freehand_tolerance_changed
                );

            new_objects_style (vb_tool, tt, "tools.freehand.pencil");

            selcue_checkbox (vb_tool, tt, "tools.freehand.pencil");
        }

        // Pen
        {
            GtkWidget *vb_tool = new_tab (nb_tools, _("Pen"));

            new_objects_style (vb_tool, tt, "tools.freehand.pen");

            selcue_checkbox (vb_tool, tt, "tools.freehand.pen");
        }

        // Calligraphy
        {
            GtkWidget *vb_tool = new_tab (nb_tools, _("Calligraphy"));

            new_objects_style (vb_tool, tt, "tools.calligraphic");
        }

        // Text
        {
            GtkWidget *vb_tool = new_tab (nb_tools, _("Text"));

            new_objects_style (vb_tool, tt, "tools.text");

            selcue_checkbox (vb_tool, tt, "tools.text");
            gradientdrag_checkbox (vb_tool, tt, "tools.text");
        }

        // Gradient
        {
            GtkWidget *vb_tool = new_tab (nb_tools, _("Gradient"));

            selcue_checkbox (vb_tool, tt, "tools.gradient");
        }

        // Connector
        {
            GtkWidget *vb_tool = new_tab (nb_tools, _("Connector"));
            
            selcue_checkbox (vb_tool, tt, "tools.connector");
        }

        // Dropper
        {
            GtkWidget *vb_tool = new_tab (nb_tools, _("Dropper"));

            GtkWidget *dropper_page = options_dropper ();
            gtk_widget_show (dropper_page);
            gtk_container_add (GTK_CONTAINER (vb_tool), dropper_page);
        }

        g_signal_connect(GTK_OBJECT (nb_tools), "switch-page", GTK_SIGNAL_FUNC (prefs_switch_page), (void *) "page_tools");
        gtk_notebook_set_current_page (GTK_NOTEBOOK (nb_tools), prefs_get_int_attribute ("dialogs.preferences", "page_tools", 0));


// Windows
        vb = new_tab (nb, _("Windows"));

        options_dialogs_ontop (vb, tt);

options_checkbox (
    _("Save window geometry"),
    _("Save the window size and position with each document (only for Inkscape SVG format)"), tt,
    vb,
    "options.savewindowgeometry", "value", 1,
    options_changed_boolean
    );

options_checkbox (
    _("Dialogs are hidden in taskbar"),
    _("Whether dialog windows are to be hidden in the window manager taskbar"), tt,
    vb,
    "options.dialogsskiptaskbar", "value", 1,
    options_changed_boolean
    );

options_checkbox (
    _("Zoom when window is resized"),
    _("Zoom drawing when document window is resized, to keep the same area visible (this is the default which can be changed in any window using the button above the right scrollbar)"), tt,
    vb,
    "options.stickyzoom", "value", 0,
    options_changed_boolean
    );


// Clones
        vb = new_tab (nb, _("Clones"));

        // Clone compensation
        {
            GtkWidget *f = gtk_frame_new (_("When the original moves, its clones and linked offsets:"));
            gtk_widget_show (f);
            gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

            GtkWidget *fb = gtk_vbox_new (FALSE, 0);
            gtk_widget_show (fb);
            gtk_container_add (GTK_CONTAINER (f), fb);

            gint compense = prefs_get_int_attribute ("options.clonecompensation", "value", SP_CLONE_COMPENSATION_PARALLEL);

            GtkWidget *b =
            sp_select_context_add_radio (
                NULL, fb, tt, _("Move in parallel"),
                _("Clones are translated by the same vector as their original."),
                NULL, SP_CLONE_COMPENSATION_PARALLEL, true,
                compense == SP_CLONE_COMPENSATION_PARALLEL,
                options_clone_compensation_toggled
                );

            sp_select_context_add_radio (
                b, fb, tt, _("Stay unmoved"),
                _("Clones preserve their positions when their original is moved."),
                NULL, SP_CLONE_COMPENSATION_UNMOVED, true,
                compense == SP_CLONE_COMPENSATION_UNMOVED,
                options_clone_compensation_toggled
                );

            sp_select_context_add_radio (
                b, fb, tt, _("Move according to transform"),
                _("Each clone moves according to the value of its transform= attribute. For example, a rotated clone will move in a different direction than its original."),
                NULL, SP_CLONE_COMPENSATION_NONE, true,
                compense == SP_CLONE_COMPENSATION_NONE,
                options_clone_compensation_toggled
                );
        }

        // Original deletion
        {
            GtkWidget *f = gtk_frame_new (_("When the original is deleted, its clones:"));
            gtk_widget_show (f);
            gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

            GtkWidget *fb = gtk_vbox_new (FALSE, 0);
            gtk_widget_show (fb);
            gtk_container_add (GTK_CONTAINER (f), fb);

            gint orphans = prefs_get_int_attribute ("options.cloneorphans", "value", SP_CLONE_ORPHANS_UNLINK);

            GtkWidget *b =
            sp_select_context_add_radio (
                NULL, fb, tt, _("Are unlinked"), _("Orphaned clones are converted to regular objects."), NULL, SP_CLONE_ORPHANS_UNLINK, true,
                orphans == SP_CLONE_ORPHANS_UNLINK,
                options_clone_orphans_toggled
                );

            sp_select_context_add_radio (
                b, fb, tt, _("Are deleted"), _("Orphaned clones are deleted along with their original."), NULL, SP_CLONE_ORPHANS_DELETE, true,
                orphans == SP_CLONE_ORPHANS_DELETE,
                options_clone_orphans_toggled
                );

/*
            sp_select_context_add_radio (
                b, fb, tt, _("Ask me"), _("Ask me what to do with the clones when their originals are deleted."), NULL, SP_CLONE_ORPHANS_ASKME, true,
                orphans == SP_CLONE_ORPHANS_ASKME,
                options_clone_orphans_toggled
                );
*/
        }

// Transforms
        /* TRANSLATORS: Noun, i.e. transformations. */
        vb = new_tab (nb, _("Transforms"));

options_checkbox (
    _("Scale stroke width"),
    _("When scaling objects, scale the stroke width by the same proportion"), tt,
    vb,
    "options.transform", "stroke", 1,
    options_changed_boolean
    );

options_checkbox (
    _("Scale rounded corners in rectangles"),
    _("When scaling rectangles, scale the radii of rounded corners"), tt,
    vb,
    "options.transform", "rectcorners", 0,
    options_changed_boolean
    );

options_checkbox (
    _("Transform gradients"),
    _("Transform gradients (in fill or stroke) along with the objects"), tt,
    vb,
    "options.transform", "gradient", 1,
    options_changed_boolean
    );

options_checkbox (
    _("Transform patterns"),
    _("Transform patterns (in fill or stroke) along with the objects"), tt,
    vb,
    "options.transform", "pattern", 1,
    options_changed_boolean
    );


     // Store transformation (global)
        {
            /* TRANSLATORS: How to specify the affine transformation in the SVG file. */
            GtkWidget *f = gtk_frame_new (_("Store transformation:"));
            gtk_widget_show (f);
            gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

            GtkWidget *fb = gtk_vbox_new (FALSE, 0);
            gtk_widget_show (fb);
            gtk_container_add (GTK_CONTAINER (f), fb);

            gint preserve = prefs_get_int_attribute ("options.preservetransform", "value", 0);

            GtkWidget *b = sp_select_context_add_radio (
                NULL, fb, tt, _("Optimized"),
                _("If possible, apply transformation to objects without adding a transform= attribute"),
                NULL, 0, true,
                preserve == 0,
                options_store_transform_toggled
                );

            sp_select_context_add_radio (
                b, fb, tt, _("Preserved"),
                _("Always store transformation as a transform= attribute on objects"),
                NULL, 1, true,
                preserve != 0,
                options_store_transform_toggled
                );
        }

// Selecting
        vb = new_tab (nb, _("Selecting"));

            GtkWidget *f = gtk_frame_new (_("Ctrl+A, Tab, Shift+Tab:"));
            gtk_widget_show (f);
            gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

            GtkWidget *fb = gtk_vbox_new (FALSE, 0);
            gtk_widget_show (fb);
            gtk_container_add (GTK_CONTAINER (f), fb);

options_checkbox (
    _("Select only within current layer"),
    _("Uncheck this to make keyboard selection commands work on objects in all layers"), tt,
    fb,
    "options.kbselection", "inlayer", 1,
    options_changed_boolean
    );

options_checkbox (
    _("Ignore hidden objects"),
    _("Uncheck this to be able to select objects that are hidden (either by themselves or by being in a hidden group or layer)"), tt,
    fb,
    "options.kbselection", "onlyvisible", 1,
    options_changed_boolean
    );

options_checkbox (
    _("Ignore locked objects"),
    _("Uncheck this to be able to select objects that are locked (either by themselves or by being in a locked group or layer)"), tt,
    fb,
    "options.kbselection", "onlysensitive", 1,
    options_changed_boolean
    );


// To be broken into: Display, Save, Export, SVG, Commands
        vb = new_tab (nb, _("Misc"));

        options_sb (
            _("Default export resolution:"),
            _("Default bitmap resolution (in dots per inch) in the Export dialog"), tt, // FIXME: add "Used for new exports; once exported, documents remember this value on per-object basis" when implemented
            _("dpi"),
            vb,
            0.0, 6000.0, 1.0, 1.0, 1.0,
            "dialogs.export.defaultxdpi", "value", PX_PER_IN,
            true, false,
            options_changed_int
            );

        options_checkbox (
    /* TRANSLATORS: When on, an imported bitmap creates an <image> element; otherwise it is a
     * rectangle with bitmap fill. */
    _("Import bitmap as <image>"),
    _("When on, an imported bitmap creates an <image> element; otherwise it is a rectangle with bitmap fill"), tt,
    vb,
    "options.importbitmapsasimages", "value", 1,
    options_changed_boolean
    );

        options_checkbox (
    /* TRANSLATORS: When on, the print out (currently Postscript) will have
     * a comment with the each object's label visible, marking the section
     * of the printing commands that represent the given object. */
    _("Add label comments to printing output"),
    _("When on, a comment will be added to the raw print output, marking the rendered output for an object with its label"), tt,
    vb,
    "printing.debug", "show-label-comments", 0,
    options_changed_boolean
    );

        options_checkbox (
    /* TRANSLATORS: When on, enable the effects menu, default is off */
    _("Enable script effects (requires restart) - EXPERIMENTAL"),
    _("When on, the effects menu is enabled, allowing external effect scripts to be called, requires restart before effective - EXPERIMENTAL"), tt,
    vb,
    "extensions", "show-effects-menu", 0,
    options_changed_boolean
    );

        options_sb (
            /* TRANSLATORS: The maximum length of the Open Recent list in the File menu. */
            _("Max recent documents:"),
            _("The maximum length of the Open Recent list in the File menu"), tt,
            "",
            vb,
            0.0, 1000.0, 1.0, 1.0, 1.0,
            "options.maxrecentdocuments", "value", 20.0,
            true, false,
            options_changed_int
            );

        options_sb (
            _("Simplification threshold:"),
            _("How strong is the Simplify command by default. If you invoke this command several times in quick succession, it will act more and more aggressively; invoking it again after a pause restores the default threshold."), tt,
            "",
            vb,
            0.0, 1.0, 0.001, 0.01, 0.01,
            "options.simplifythreshold", "value", 0.002,
            false, false,
            options_changed_double
            );


        /* Oversample */
        hb = gtk_hbox_new (FALSE, HB_MARGIN);
        gtk_widget_show (hb);
        gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

        // empty label for alignment
        l = gtk_label_new ("");
        gtk_widget_set_size_request (l, SUFFIX_WIDTH, -1);
        gtk_widget_show (l);
        gtk_box_pack_end (GTK_BOX (hb), l, FALSE, FALSE, 0);
    
        l = gtk_label_new (_("Oversample bitmaps:"));
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_widget_show (l);
        gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);

        om = gtk_option_menu_new ();
        gtk_widget_set_size_request (om, SB_WIDTH, -1);
        gtk_widget_show (om);
        gtk_box_pack_end (GTK_BOX (hb), om, FALSE, FALSE, SB_MARGIN);
        
        m = gtk_menu_new ();
        gtk_widget_show (m);

        i = gtk_menu_item_new_with_label (_("None"));
        gtk_signal_connect ( GTK_OBJECT (i), "activate",
                             GTK_SIGNAL_FUNC (sp_display_dialog_set_oversample),
                             GINT_TO_POINTER (0) );
        gtk_widget_show (i);
        gtk_menu_append (GTK_MENU (m), i);
        i = gtk_menu_item_new_with_label (_("2x2"));
        gtk_signal_connect ( GTK_OBJECT (i), "activate",
                             GTK_SIGNAL_FUNC (sp_display_dialog_set_oversample),
                             GINT_TO_POINTER (1) );
        gtk_widget_show (i);
        gtk_menu_append (GTK_MENU (m), i);
        i = gtk_menu_item_new_with_label (_("4x4"));
        gtk_signal_connect ( GTK_OBJECT (i), "activate",
                             GTK_SIGNAL_FUNC (sp_display_dialog_set_oversample),
                             GINT_TO_POINTER (2) );
        gtk_widget_show (i);
        gtk_menu_append (GTK_MENU (m), i);
        i = gtk_menu_item_new_with_label (_("8x8"));
        gtk_signal_connect ( GTK_OBJECT (i), "activate",
                             GTK_SIGNAL_FUNC (sp_display_dialog_set_oversample),
                             GINT_TO_POINTER (3));
        gtk_widget_show (i);
        gtk_menu_append (GTK_MENU (m), i);
        i = gtk_menu_item_new_with_label (_("16x16"));
        gtk_signal_connect ( GTK_OBJECT (i), "activate",
                             GTK_SIGNAL_FUNC (sp_display_dialog_set_oversample),
                             GINT_TO_POINTER (4) );
        gtk_widget_show (i);
        gtk_menu_append (GTK_MENU (m), i);

        gtk_option_menu_set_menu (GTK_OPTION_MENU (om), m);

        gtk_option_menu_set_history ( GTK_OPTION_MENU (om),
                                      nr_arena_image_x_sample);

        g_signal_connect(GTK_OBJECT (nb), "switch-page", GTK_SIGNAL_FUNC (prefs_switch_page), (void *) "page_top");
        gtk_notebook_set_current_page (GTK_NOTEBOOK (nb), prefs_get_int_attribute ("dialogs.preferences", "page_top", 0));

    } // end of if (!dlg)

    gtk_window_present ((GtkWindow *) dlg);

} // end of sp_display_dialog()


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
