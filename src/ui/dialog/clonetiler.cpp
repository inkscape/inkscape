
/**
 * @file
 * Clone tiling dialog
 */
/* Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *   Romain de Bossoreille
 *
 * Copyright (C) 2004-2011 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "clonetiler.h"

#include <climits>

#include <glibmm/i18n.h>
#include <2geom/transforms.h>
#include <gtkmm/adjustment.h>

#include "desktop.h"

#include "display/cairo-utils.h"
#include "display/drawing.h"
#include "display/drawing-context.h"
#include "display/drawing-item.h"
#include "document.h"
#include "document-undo.h"
#include "filter-chemistry.h"
#include "ui/widget/unit-menu.h"
#include "util/units.h"
#include "helper/window.h"
#include "inkscape.h"
#include "ui/interface.h"
#include "macros.h"
#include "message-stack.h"
#include "preferences.h"
#include "selection.h"
#include "sp-filter.h"
#include "sp-namedview.h"
#include "sp-use.h"
#include "style.h"
#include "svg/svg-color.h"
#include "svg/svg.h"
#include "ui/icon-names.h"
#include "ui/widget/spinbutton.h"
#include "unclump.h"
#include "verbs.h"
#include "widgets/icon.h"
#include "xml/repr.h"
#include "sp-root.h"

using Inkscape::DocumentUndo;
using Inkscape::Util::unit_table;

namespace Inkscape {
namespace UI {
namespace Dialog {

#define SB_MARGIN 1
#define VB_MARGIN 4

static Glib::ustring const prefs_path = "/dialogs/clonetiler/";

static Inkscape::Drawing *trace_drawing = NULL;
static unsigned trace_visionkey;
static gdouble trace_zoom;
static SPDocument *trace_doc = NULL;


CloneTiler::CloneTiler () :
    UI::Widget::Panel ("", "/dialogs/clonetiler/", SP_VERB_DIALOG_CLONETILER),
    dlg(NULL),
    desktop(NULL),
    deskTrack(),
    table_row_labels(NULL)
{
    Gtk::Box *contents = _getContents();
    contents->set_spacing(0);

    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();

        dlg = GTK_WIDGET(gobj());

#if GTK_CHECK_VERSION(3,0,0)
        GtkWidget *mainbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
        gtk_box_set_homogeneous(GTK_BOX(mainbox), FALSE);
#else
        GtkWidget *mainbox = gtk_vbox_new(FALSE, 4);
#endif
        gtk_container_set_border_width (GTK_CONTAINER (mainbox), 6);

        contents->pack_start (*Gtk::manage(Glib::wrap(mainbox)), true, true, 0);

        GtkWidget *nb = gtk_notebook_new ();
        gtk_box_pack_start (GTK_BOX (mainbox), nb, FALSE, FALSE, 0);


        // Symmetry
        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("_Symmetry"));

        /* TRANSLATORS: For the following 17 symmetry groups, see
             * http://www.bib.ulb.ac.be/coursmath/doc/17.htm (visual examples);
             * http://www.clarku.edu/~djoyce/wallpaper/seventeen.html (English vocabulary); or
             * http://membres.lycos.fr/villemingerard/Geometri/Sym1D.htm (French vocabulary).
             */
            struct SymGroups {
                gint group;
                gchar const *label;
            } const sym_groups[] = {
                // TRANSLATORS: "translation" means "shift" / "displacement" here.
                {TILE_P1, _("<b>P1</b>: simple translation")},
                {TILE_P2, _("<b>P2</b>: 180&#176; rotation")},
                {TILE_PM, _("<b>PM</b>: reflection")},
                // TRANSLATORS: "glide reflection" is a reflection and a translation combined.
                //  For more info, see http://mathforum.org/sum95/suzanne/symsusan.html
                {TILE_PG, _("<b>PG</b>: glide reflection")},
                {TILE_CM, _("<b>CM</b>: reflection + glide reflection")},
                {TILE_PMM, _("<b>PMM</b>: reflection + reflection")},
                {TILE_PMG, _("<b>PMG</b>: reflection + 180&#176; rotation")},
                {TILE_PGG, _("<b>PGG</b>: glide reflection + 180&#176; rotation")},
                {TILE_CMM, _("<b>CMM</b>: reflection + reflection + 180&#176; rotation")},
                {TILE_P4, _("<b>P4</b>: 90&#176; rotation")},
                {TILE_P4M, _("<b>P4M</b>: 90&#176; rotation + 45&#176; reflection")},
                {TILE_P4G, _("<b>P4G</b>: 90&#176; rotation + 90&#176; reflection")},
                {TILE_P3, _("<b>P3</b>: 120&#176; rotation")},
                {TILE_P31M, _("<b>P31M</b>: reflection + 120&#176; rotation, dense")},
                {TILE_P3M1, _("<b>P3M1</b>: reflection + 120&#176; rotation, sparse")},
                {TILE_P6, _("<b>P6</b>: 60&#176; rotation")},
                {TILE_P6M, _("<b>P6M</b>: reflection + 60&#176; rotation")},
            };

            gint current = prefs->getInt(prefs_path + "symmetrygroup", 0);

        // Create a list structure containing all the data to be displayed in
        // the symmetry group combo box.
        GtkListStore *store = gtk_list_store_new (1, G_TYPE_STRING);
        GtkTreeIter iter;

        for (unsigned j = 0; j < G_N_ELEMENTS(sym_groups); ++j) {
            SymGroups const &sg = sym_groups[j];

            // Add the description of the symgroup to a new row
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, sg.label, -1);
        }

        // Add a new combo box widget with the list of symmetry groups to the vbox
        GtkWidget *combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (store));
        gtk_widget_set_tooltip_text (combo, _("Select one of the 17 symmetry groups for the tiling"));
        gtk_box_pack_start (GTK_BOX (vb), combo, FALSE, FALSE, SB_MARGIN);

        // Specify the rendering of data from the list in a combo box cell
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
        gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, FALSE);
        gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, "markup", 0, NULL);

        gtk_combo_box_set_active (GTK_COMBO_BOX (combo), current);

        g_signal_connect(G_OBJECT(combo), "changed",
                    G_CALLBACK(clonetiler_symgroup_changed), NULL);
        }

        table_row_labels = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

        // Shift
        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("S_hift"));

            GtkWidget *table = clonetiler_table_x_y_rand (3);
            gtk_box_pack_start (GTK_BOX (vb), table, FALSE, FALSE, 0);

            // X
            {
                GtkWidget *l = gtk_label_new ("");
                    // TRANSLATORS: "shift" means: the tiles will be shifted (offset) horizontally by this amount
                    // xgettext:no-c-format
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Shift X:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 2, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (
                    // xgettext:no-c-format
                   _("Horizontal shift per row (in % of tile width)"), "shiftx_per_j",
                   -10000, 10000, "%");
                clonetiler_table_attach (table, l, 0, 2, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (
                    // xgettext:no-c-format
                   _("Horizontal shift per column (in % of tile width)"), "shiftx_per_i",
                   -10000, 10000, "%");
                clonetiler_table_attach (table, l, 0, 2, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Randomize the horizontal shift by this percentage"), "shiftx_rand",
                                                   0, 1000, "%");
                clonetiler_table_attach (table, l, 0, 2, 4);
            }

            // Y
            {
                GtkWidget *l = gtk_label_new ("");
                    // TRANSLATORS: "shift" means: the tiles will be shifted (offset) vertically by this amount
                    // xgettext:no-c-format
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Shift Y:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 3, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (
                    // xgettext:no-c-format
                                                   _("Vertical shift per row (in % of tile height)"), "shifty_per_j",
                                                   -10000, 10000, "%");
                clonetiler_table_attach (table, l, 0, 3, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (
                    // xgettext:no-c-format
                                                   _("Vertical shift per column (in % of tile height)"), "shifty_per_i",
                                                   -10000, 10000, "%");
                clonetiler_table_attach (table, l, 0, 3, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (
                                                   _("Randomize the vertical shift by this percentage"), "shifty_rand",
                                                   0, 1000, "%");
                clonetiler_table_attach (table, l, 0, 3, 4);
            }

            // Exponent
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Exponent:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 4, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (
                                                   _("Whether rows are spaced evenly (1), converge (<1) or diverge (>1)"), "shifty_exp",
                                                   0, 10, "", true);
                clonetiler_table_attach (table, l, 0, 4, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (
                                                   _("Whether columns are spaced evenly (1), converge (<1) or diverge (>1)"), "shiftx_exp",
                                                   0, 10, "", true);
                clonetiler_table_attach (table, l, 0, 4, 3);
            }

            { // alternates
                GtkWidget *l = gtk_label_new ("");
                // TRANSLATORS: "Alternate" is a verb here
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Alternate:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 5, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Alternate the sign of shifts for each row"), "shifty_alternate");
                clonetiler_table_attach (table, l, 0, 5, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Alternate the sign of shifts for each column"), "shiftx_alternate");
                clonetiler_table_attach (table, l, 0, 5, 3);
            }

            { // Cumulate
                GtkWidget *l = gtk_label_new ("");
                // TRANSLATORS: "Cumulate" is a verb here
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Cumulate:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 6, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Cumulate the shifts for each row"), "shifty_cumulate");
                clonetiler_table_attach (table, l, 0, 6, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Cumulate the shifts for each column"), "shiftx_cumulate");
                clonetiler_table_attach (table, l, 0, 6, 3);
            }

            { // Exclude tile width and height in shift
                GtkWidget *l = gtk_label_new ("");
                // TRANSLATORS: "Cumulate" is a verb here
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Exclude tile:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 7, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Exclude tile height in shift"), "shifty_excludeh");
                clonetiler_table_attach (table, l, 0, 7, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Exclude tile width in shift"), "shiftx_excludew");
                clonetiler_table_attach (table, l, 0, 7, 3);
            }

        }


        // Scale
        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("Sc_ale"));

            GtkWidget *table = clonetiler_table_x_y_rand (2);
            gtk_box_pack_start (GTK_BOX (vb), table, FALSE, FALSE, 0);

            // X
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Scale X:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 2, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (
                    // xgettext:no-c-format
                                                   _("Horizontal scale per row (in % of tile width)"), "scalex_per_j",
                                                   -100, 1000, "%");
                clonetiler_table_attach (table, l, 0, 2, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (
                    // xgettext:no-c-format
                                                   _("Horizontal scale per column (in % of tile width)"), "scalex_per_i",
                                                   -100, 1000, "%");
                clonetiler_table_attach (table, l, 0, 2, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Randomize the horizontal scale by this percentage"), "scalex_rand",
                                                   0, 1000, "%");
                clonetiler_table_attach (table, l, 0, 2, 4);
            }

            // Y
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Scale Y:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 3, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (
                    // xgettext:no-c-format
                                                   _("Vertical scale per row (in % of tile height)"), "scaley_per_j",
                                                   -100, 1000, "%");
                clonetiler_table_attach (table, l, 0, 3, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (
                    // xgettext:no-c-format
                                                   _("Vertical scale per column (in % of tile height)"), "scaley_per_i",
                                                   -100, 1000, "%");
                clonetiler_table_attach (table, l, 0, 3, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Randomize the vertical scale by this percentage"), "scaley_rand",
                                                   0, 1000, "%");
                clonetiler_table_attach (table, l, 0, 3, 4);
            }

            // Exponent
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Exponent:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 4, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Whether row scaling is uniform (1), converge (<1) or diverge (>1)"), "scaley_exp",
                                                   0, 10, "", true);
                clonetiler_table_attach (table, l, 0, 4, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Whether column scaling is uniform (1), converge (<1) or diverge (>1)"), "scalex_exp",
                                                   0, 10, "", true);
                clonetiler_table_attach (table, l, 0, 4, 3);
            }

            // Logarithmic (as in logarithmic spiral)
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Base:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 5, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Base for a logarithmic spiral: not used (0), converge (<1), or diverge (>1)"), "scaley_log",
                                                   0, 10, "", false);
                clonetiler_table_attach (table, l, 0, 5, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Base for a logarithmic spiral: not used (0), converge (<1), or diverge (>1)"), "scalex_log",
                                                   0, 10, "", false);
                clonetiler_table_attach (table, l, 0, 5, 3);
            }

            { // alternates
                GtkWidget *l = gtk_label_new ("");
                // TRANSLATORS: "Alternate" is a verb here
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Alternate:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 6, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Alternate the sign of scales for each row"), "scaley_alternate");
                clonetiler_table_attach (table, l, 0, 6, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Alternate the sign of scales for each column"), "scalex_alternate");
                clonetiler_table_attach (table, l, 0, 6, 3);
            }

            { // Cumulate
                GtkWidget *l = gtk_label_new ("");
                // TRANSLATORS: "Cumulate" is a verb here
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Cumulate:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 7, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Cumulate the scales for each row"), "scaley_cumulate");
                clonetiler_table_attach (table, l, 0, 7, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Cumulate the scales for each column"), "scalex_cumulate");
                clonetiler_table_attach (table, l, 0, 7, 3);
            }

        }


        // Rotation
        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("_Rotation"));

            GtkWidget *table = clonetiler_table_x_y_rand (1);
            gtk_box_pack_start (GTK_BOX (vb), table, FALSE, FALSE, 0);

            // Angle
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Angle:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 2, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (
                    // xgettext:no-c-format
                                                   _("Rotate tiles by this angle for each row"), "rotate_per_j",
                                                   -180, 180, "&#176;");
                clonetiler_table_attach (table, l, 0, 2, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (
                    // xgettext:no-c-format
                                                   _("Rotate tiles by this angle for each column"), "rotate_per_i",
                                                   -180, 180, "&#176;");
                clonetiler_table_attach (table, l, 0, 2, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Randomize the rotation angle by this percentage"), "rotate_rand",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 2, 4);
            }

            { // alternates
                GtkWidget *l = gtk_label_new ("");
                // TRANSLATORS: "Alternate" is a verb here
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Alternate:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 3, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Alternate the rotation direction for each row"), "rotate_alternatej");
                clonetiler_table_attach (table, l, 0, 3, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Alternate the rotation direction for each column"), "rotate_alternatei");
                clonetiler_table_attach (table, l, 0, 3, 3);
            }

            { // Cumulate
                GtkWidget *l = gtk_label_new ("");
                // TRANSLATORS: "Cumulate" is a verb here
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Cumulate:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 4, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Cumulate the rotation for each row"), "rotate_cumulatej");
                clonetiler_table_attach (table, l, 0, 4, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Cumulate the rotation for each column"), "rotate_cumulatei");
                clonetiler_table_attach (table, l, 0, 4, 3);
            }

        }


        // Blur and opacity
        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("_Blur & opacity"));

            GtkWidget *table = clonetiler_table_x_y_rand (1);
            gtk_box_pack_start (GTK_BOX (vb), table, FALSE, FALSE, 0);


            // Blur
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Blur:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 2, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Blur tiles by this percentage for each row"), "blur_per_j",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 2, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Blur tiles by this percentage for each column"), "blur_per_i",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 2, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Randomize the tile blur by this percentage"), "blur_rand",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 2, 4);
            }

            { // alternates
                GtkWidget *l = gtk_label_new ("");
                // TRANSLATORS: "Alternate" is a verb here
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Alternate:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 3, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Alternate the sign of blur change for each row"), "blur_alternatej");
                clonetiler_table_attach (table, l, 0, 3, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Alternate the sign of blur change for each column"), "blur_alternatei");
                clonetiler_table_attach (table, l, 0, 3, 3);
            }



            // Dissolve
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Opacity:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 4, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Decrease tile opacity by this percentage for each row"), "opacity_per_j",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 4, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Decrease tile opacity by this percentage for each column"), "opacity_per_i",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 4, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Randomize the tile opacity by this percentage"), "opacity_rand",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 4, 4);
            }

            { // alternates
                GtkWidget *l = gtk_label_new ("");
                // TRANSLATORS: "Alternate" is a verb here
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Alternate:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 5, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Alternate the sign of opacity change for each row"), "opacity_alternatej");
                clonetiler_table_attach (table, l, 0, 5, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Alternate the sign of opacity change for each column"), "opacity_alternatei");
                clonetiler_table_attach (table, l, 0, 5, 3);
            }
        }


        // Color
        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("Co_lor"));

            {
#if GTK_CHECK_VERSION(3,0,0)
            GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
            gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
            GtkWidget *hb = gtk_hbox_new (FALSE, 0);
#endif

            GtkWidget *l = gtk_label_new (_("Initial color: "));
            gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);

            guint32 rgba = 0x000000ff | sp_svg_read_color (prefs->getString(prefs_path + "initial_color").data(), 0x000000ff);
            color_picker = new Inkscape::UI::Widget::ColorPicker (*new Glib::ustring(_("Initial color of tiled clones")), *new Glib::ustring(_("Initial color for clones (works only if the original has unset fill or stroke)")), rgba, false);
            color_changed_connection = color_picker->connectChanged (sigc::ptr_fun(on_picker_color_changed));

            gtk_box_pack_start (GTK_BOX (hb), reinterpret_cast<GtkWidget*>(color_picker->gobj()), FALSE, FALSE, 0);

            gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);
            }


            GtkWidget *table = clonetiler_table_x_y_rand (3);
            gtk_box_pack_start (GTK_BOX (vb), table, FALSE, FALSE, 0);

            // Hue
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>H:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 2, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Change the tile hue by this percentage for each row"), "hue_per_j",
                                                   -100, 100, "%");
                clonetiler_table_attach (table, l, 0, 2, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Change the tile hue by this percentage for each column"), "hue_per_i",
                                                   -100, 100, "%");
                clonetiler_table_attach (table, l, 0, 2, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Randomize the tile hue by this percentage"), "hue_rand",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 2, 4);
            }


            // Saturation
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>S:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 3, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Change the color saturation by this percentage for each row"), "saturation_per_j",
                                                   -100, 100, "%");
                clonetiler_table_attach (table, l, 0, 3, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Change the color saturation by this percentage for each column"), "saturation_per_i",
                                                   -100, 100, "%");
                clonetiler_table_attach (table, l, 0, 3, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Randomize the color saturation by this percentage"), "saturation_rand",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 3, 4);
            }

            // Lightness
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>L:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 4, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Change the color lightness by this percentage for each row"), "lightness_per_j",
                                                   -100, 100, "%");
                clonetiler_table_attach (table, l, 0, 4, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Change the color lightness by this percentage for each column"), "lightness_per_i",
                                                   -100, 100, "%");
                clonetiler_table_attach (table, l, 0, 4, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (_("Randomize the color lightness by this percentage"), "lightness_rand",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 4, 4);
            }


            { // alternates
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Alternate:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 5, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Alternate the sign of color changes for each row"), "color_alternatej");
                clonetiler_table_attach (table, l, 0, 5, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (_("Alternate the sign of color changes for each column"), "color_alternatei");
                clonetiler_table_attach (table, l, 0, 5, 3);
            }

        }

        // Trace
        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("_Trace"));


        {
#if GTK_CHECK_VERSION(3,0,0)
            GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, VB_MARGIN);
            gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
            GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);
#endif
            gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

            GtkWidget *b  = gtk_check_button_new_with_label (_("Trace the drawing under the tiles"));
            g_object_set_data (G_OBJECT(b), "uncheckable", GINT_TO_POINTER(TRUE));
            bool old = prefs->getBool(prefs_path + "dotrace");
            gtk_toggle_button_set_active ((GtkToggleButton *) b, old);
            gtk_widget_set_tooltip_text (b, _("For each clone, pick a value from the drawing in that clone's location and apply it to the clone"));
            gtk_box_pack_start (GTK_BOX (hb), b, FALSE, FALSE, 0);

            g_signal_connect(G_OBJECT(b), "toggled",
                               G_CALLBACK(clonetiler_do_pick_toggled), (gpointer)dlg);
        }

        {
#if GTK_CHECK_VERSION(3,0,0)
            GtkWidget *vvb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
            gtk_box_set_homogeneous(GTK_BOX(vvb), FALSE);
#else
            GtkWidget *vvb = gtk_vbox_new (FALSE, 0);
#endif
            gtk_box_pack_start (GTK_BOX (vb), vvb, FALSE, FALSE, 0);
            g_object_set_data (G_OBJECT(dlg), "dotrace", (gpointer) vvb);


            {
                GtkWidget *frame = gtk_frame_new (_("1. Pick from the drawing:"));
                gtk_box_pack_start (GTK_BOX (vvb), frame, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3,0,0)
                GtkWidget *table = gtk_grid_new();
                gtk_grid_set_row_spacing(GTK_GRID(table), 4);
                gtk_grid_set_column_spacing(GTK_GRID(table), 6);
#else
                GtkWidget *table = gtk_table_new (3, 3, FALSE);
                gtk_table_set_row_spacings (GTK_TABLE (table), 4);
                gtk_table_set_col_spacings (GTK_TABLE (table), 6);
#endif
                gtk_container_add(GTK_CONTAINER(frame), table);


                GtkWidget* radio;
                {
                    radio = gtk_radio_button_new_with_label (NULL, _("Color"));
                    gtk_widget_set_tooltip_text (radio, _("Pick the visible color and opacity"));
                    clonetiler_table_attach (table, radio, 0.0, 1, 1);
                    g_signal_connect (G_OBJECT (radio), "toggled",
                                        G_CALLBACK (clonetiler_pick_switched), GINT_TO_POINTER(PICK_COLOR));
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), prefs->getInt(prefs_path + "pick", 0) == PICK_COLOR);
                }
                {
                    radio = gtk_radio_button_new_with_label (gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio)), _("Opacity"));
                    gtk_widget_set_tooltip_text (radio, _("Pick the total accumulated opacity"));
                    clonetiler_table_attach (table, radio, 0.0, 2, 1);
                    g_signal_connect (G_OBJECT (radio), "toggled",
                                        G_CALLBACK (clonetiler_pick_switched), GINT_TO_POINTER(PICK_OPACITY));
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), prefs->getInt(prefs_path + "pick", 0) == PICK_OPACITY);
                }
                {
                    radio = gtk_radio_button_new_with_label (gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio)), _("R"));
                    gtk_widget_set_tooltip_text (radio, _("Pick the Red component of the color"));
                    clonetiler_table_attach (table, radio, 0.0, 1, 2);
                    g_signal_connect (G_OBJECT (radio), "toggled",
                                        G_CALLBACK (clonetiler_pick_switched), GINT_TO_POINTER(PICK_R));
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), prefs->getInt(prefs_path + "pick", 0) == PICK_R);
                }
                {
                    radio = gtk_radio_button_new_with_label (gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio)), _("G"));
                    gtk_widget_set_tooltip_text (radio, _("Pick the Green component of the color"));
                    clonetiler_table_attach (table, radio, 0.0, 2, 2);
                    g_signal_connect (G_OBJECT (radio), "toggled",
                                        G_CALLBACK (clonetiler_pick_switched), GINT_TO_POINTER(PICK_G));
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), prefs->getInt(prefs_path + "pick", 0) == PICK_G);
                }
                {
                    radio = gtk_radio_button_new_with_label (gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio)), _("B"));
                    gtk_widget_set_tooltip_text (radio, _("Pick the Blue component of the color"));
                    clonetiler_table_attach (table, radio, 0.0, 3, 2);
                    g_signal_connect (G_OBJECT (radio), "toggled",
                                        G_CALLBACK (clonetiler_pick_switched), GINT_TO_POINTER(PICK_B));
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), prefs->getInt(prefs_path + "pick", 0) == PICK_B);
                }
                {
                    radio = gtk_radio_button_new_with_label (gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio)), C_("Clonetiler color hue", "H"));
                    gtk_widget_set_tooltip_text (radio, _("Pick the hue of the color"));
                    clonetiler_table_attach (table, radio, 0.0, 1, 3);
                    g_signal_connect (G_OBJECT (radio), "toggled",
                                        G_CALLBACK (clonetiler_pick_switched), GINT_TO_POINTER(PICK_H));
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), prefs->getInt(prefs_path + "pick", 0) == PICK_H);
                }
                {
                    radio = gtk_radio_button_new_with_label (gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio)), C_("Clonetiler color saturation", "S"));
                    gtk_widget_set_tooltip_text (radio, _("Pick the saturation of the color"));
                    clonetiler_table_attach (table, radio, 0.0, 2, 3);
                    g_signal_connect (G_OBJECT (radio), "toggled",
                                        G_CALLBACK (clonetiler_pick_switched), GINT_TO_POINTER(PICK_S));
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), prefs->getInt(prefs_path + "pick", 0) == PICK_S);
                }
                {
                    radio = gtk_radio_button_new_with_label (gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio)), C_("Clonetiler color lightness", "L"));
                    gtk_widget_set_tooltip_text (radio, _("Pick the lightness of the color"));
                    clonetiler_table_attach (table, radio, 0.0, 3, 3);
                    g_signal_connect (G_OBJECT (radio), "toggled",
                                        G_CALLBACK (clonetiler_pick_switched), GINT_TO_POINTER(PICK_L));
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), prefs->getInt(prefs_path + "pick", 0) == PICK_L);
                }

            }

            {
                GtkWidget *frame = gtk_frame_new (_("2. Tweak the picked value:"));
                gtk_box_pack_start (GTK_BOX (vvb), frame, FALSE, FALSE, VB_MARGIN);

#if GTK_CHECK_VERSION(3,0,0)
                GtkWidget *table = gtk_grid_new();
                gtk_grid_set_row_spacing(GTK_GRID(table), 4);
                gtk_grid_set_column_spacing(GTK_GRID(table), 6);
#else
                GtkWidget *table = gtk_table_new (4, 2, FALSE);
                gtk_table_set_row_spacings (GTK_TABLE (table), 4);
                gtk_table_set_col_spacings (GTK_TABLE (table), 6);
#endif

                gtk_container_add(GTK_CONTAINER(frame), table);

                {
                    GtkWidget *l = gtk_label_new ("");
                    gtk_label_set_markup (GTK_LABEL(l), _("Gamma-correct:"));
                    clonetiler_table_attach (table, l, 1.0, 1, 1);
                }
                {
                    GtkWidget *l = clonetiler_spinbox (_("Shift the mid-range of the picked value upwards (>0) or downwards (<0)"), "gamma_picked",
                                                       -10, 10, "");
                    clonetiler_table_attach (table, l, 0.0, 1, 2);
                }

                {
                    GtkWidget *l = gtk_label_new ("");
                    gtk_label_set_markup (GTK_LABEL(l), _("Randomize:"));
                    clonetiler_table_attach (table, l, 1.0, 1, 3);
                }
                {
                    GtkWidget *l = clonetiler_spinbox (_("Randomize the picked value by this percentage"), "rand_picked",
                                                       0, 100, "%");
                    clonetiler_table_attach (table, l, 0.0, 1, 4);
                }

                {
                    GtkWidget *l = gtk_label_new ("");
                    gtk_label_set_markup (GTK_LABEL(l), _("Invert:"));
                    clonetiler_table_attach (table, l, 1.0, 2, 1);
                }
                {
                    GtkWidget *l = clonetiler_checkbox (_("Invert the picked value"), "invert_picked");
                    clonetiler_table_attach (table, l, 0.0, 2, 2);
                }
            }

            {
                GtkWidget *frame = gtk_frame_new (_("3. Apply the value to the clones':"));
                gtk_box_pack_start (GTK_BOX (vvb), frame, FALSE, FALSE, 0);

#if GTK_CHECK_VERSION(3,0,0)
                GtkWidget *table = gtk_grid_new();
                gtk_grid_set_row_spacing(GTK_GRID(table), 4);
                gtk_grid_set_column_spacing(GTK_GRID(table), 6);
#else
                GtkWidget *table = gtk_table_new (2, 2, FALSE);
                gtk_table_set_row_spacings (GTK_TABLE (table), 4);
                gtk_table_set_col_spacings (GTK_TABLE (table), 6);
#endif
                gtk_container_add(GTK_CONTAINER(frame), table);

                {
                    GtkWidget *b  = gtk_check_button_new_with_label (_("Presence"));
                    bool old = prefs->getBool(prefs_path + "pick_to_presence", true);
                    gtk_toggle_button_set_active ((GtkToggleButton *) b, old);
                    gtk_widget_set_tooltip_text (b, _("Each clone is created with the probability determined by the picked value in that point"));
                    clonetiler_table_attach (table, b, 0.0, 1, 1);
                    g_signal_connect(G_OBJECT(b), "toggled",
                                       G_CALLBACK(clonetiler_pick_to), (gpointer) "pick_to_presence");
                }

                {
                    GtkWidget *b  = gtk_check_button_new_with_label (_("Size"));
                    bool old = prefs->getBool(prefs_path + "pick_to_size");
                    gtk_toggle_button_set_active ((GtkToggleButton *) b, old);
                    gtk_widget_set_tooltip_text (b, _("Each clone's size is determined by the picked value in that point"));
                    clonetiler_table_attach (table, b, 0.0, 2, 1);
                    g_signal_connect(G_OBJECT(b), "toggled",
                                       G_CALLBACK(clonetiler_pick_to), (gpointer) "pick_to_size");
                }

                {
                    GtkWidget *b  = gtk_check_button_new_with_label (_("Color"));
                    bool old = prefs->getBool(prefs_path + "pick_to_color", 0);
                    gtk_toggle_button_set_active ((GtkToggleButton *) b, old);
                    gtk_widget_set_tooltip_text (b, _("Each clone is painted by the picked color (the original must have unset fill or stroke)"));
                    clonetiler_table_attach (table, b, 0.0, 1, 2);
                    g_signal_connect(G_OBJECT(b), "toggled",
                                       G_CALLBACK(clonetiler_pick_to), (gpointer) "pick_to_color");
                }

                {
                    GtkWidget *b  = gtk_check_button_new_with_label (_("Opacity"));
                    bool old = prefs->getBool(prefs_path + "pick_to_opacity", 0);
                    gtk_toggle_button_set_active ((GtkToggleButton *) b, old);
                    gtk_widget_set_tooltip_text (b, _("Each clone's opacity is determined by the picked value in that point"));
                    clonetiler_table_attach (table, b, 0.0, 2, 2);
                    g_signal_connect(G_OBJECT(b), "toggled",
                                       G_CALLBACK(clonetiler_pick_to), (gpointer) "pick_to_opacity");
                }
            }
           gtk_widget_set_sensitive (vvb, prefs->getBool(prefs_path + "dotrace"));
        }
        }

        // Rows/columns, width/height
        {
#if GTK_CHECK_VERSION(3,0,0)
            GtkWidget *table = gtk_grid_new();
            gtk_grid_set_row_spacing(GTK_GRID(table), 4);
            gtk_grid_set_column_spacing(GTK_GRID(table), 6);
#else
            GtkWidget *table = gtk_table_new (2, 2, FALSE);
            gtk_table_set_row_spacings (GTK_TABLE (table), 4);
            gtk_table_set_col_spacings (GTK_TABLE (table), 6);
#endif

            gtk_container_set_border_width (GTK_CONTAINER (table), VB_MARGIN);
            gtk_box_pack_start (GTK_BOX (mainbox), table, FALSE, FALSE, 0);

            {
#if GTK_CHECK_VERSION(3,0,0)
                GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, VB_MARGIN);
                gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
                GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);
#endif
                g_object_set_data (G_OBJECT(dlg), "rowscols", (gpointer) hb);

                {
#if WITH_GTKMM_3_0
                    Glib::RefPtr<Gtk::Adjustment>a = Gtk::Adjustment::create(0.0, 1, 500, 1, 10, 0);
#else
                    Gtk::Adjustment *a = new Gtk::Adjustment (0.0, 1, 500, 1, 10, 0);
#endif
                    int value = prefs->getInt(prefs_path + "jmax", 2);
                    a->set_value (value);

#if WITH_GTKMM_3_0
                    Inkscape::UI::Widget::SpinButton *sb = new Inkscape::UI::Widget::SpinButton(a, 1.0, 0);
#else
                    Inkscape::UI::Widget::SpinButton *sb = new Inkscape::UI::Widget::SpinButton (*a, 1.0, 0);
#endif
                    sb->set_tooltip_text (_("How many rows in the tiling"));
                    sb->set_width_chars (7);
                    gtk_box_pack_start (GTK_BOX (hb), GTK_WIDGET(sb->gobj()), TRUE, TRUE, 0);

                    // TODO: C++ification
                    g_signal_connect(G_OBJECT(a->gobj()), "value_changed",
                                       G_CALLBACK(clonetiler_xy_changed), (gpointer) "jmax");
                }

                {
                    GtkWidget *l = gtk_label_new ("");
                    gtk_label_set_markup (GTK_LABEL(l), "&#215;");
                    gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
                    gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
                }

                {
#if WITH_GTKMM_3_0
                    Glib::RefPtr<Gtk::Adjustment> a = Gtk::Adjustment::create(0.0, 1, 500, 1, 10, 0);
#else
                    Gtk::Adjustment *a = new Gtk::Adjustment (0.0, 1, 500, 1, 10, 0);
#endif
                    int value = prefs->getInt(prefs_path + "imax", 2);
                    a->set_value (value);

#if WITH_GTKMM_3_0
                    Inkscape::UI::Widget::SpinButton *sb = new Inkscape::UI::Widget::SpinButton(a, 1.0, 0);
#else
                    Inkscape::UI::Widget::SpinButton *sb = new Inkscape::UI::Widget::SpinButton (*a, 1.0, 0);
#endif
                    sb->set_tooltip_text (_("How many columns in the tiling"));
                    sb->set_width_chars (7);
                    gtk_box_pack_start (GTK_BOX (hb), GTK_WIDGET(sb->gobj()), TRUE, TRUE, 0);

                    // TODO: C++ification
                    g_signal_connect(G_OBJECT(a->gobj()), "value_changed",
                                       G_CALLBACK(clonetiler_xy_changed), (gpointer) "imax");
                }

                clonetiler_table_attach (table, hb, 0.0, 1, 2);
            }

            {
#if GTK_CHECK_VERSION(3,0,0)
                GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, VB_MARGIN);
                gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
                GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);
#endif
                g_object_set_data (G_OBJECT(dlg), "widthheight", (gpointer) hb);

                // unitmenu
                unit_menu = new Inkscape::UI::Widget::UnitMenu();
                unit_menu->setUnitType(Inkscape::Util::UNIT_TYPE_LINEAR);
                unit_menu->setUnit(SP_ACTIVE_DESKTOP->getNamedView()->display_units->abbr);
                unitChangedConn = unit_menu->signal_changed().connect(sigc::mem_fun(*this, &CloneTiler::clonetiler_unit_changed));

                {
                    // Width spinbutton
#if WITH_GTKMM_3_0
                    fill_width = Gtk::Adjustment::create(0.0, -1e6, 1e6, 1.0, 10.0, 0);
#else
                    fill_width = new Gtk::Adjustment (0.0, -1e6, 1e6, 1.0, 10.0, 0);
#endif

                    double value = prefs->getDouble(prefs_path + "fillwidth", 50.0);
                    Inkscape::Util::Unit const *unit = unit_menu->getUnit();
                    gdouble const units = Inkscape::Util::Quantity::convert(value, "px", unit);
                    fill_width->set_value (units);

#if WITH_GTKMM_3_0
                    Inkscape::UI::Widget::SpinButton *e = new Inkscape::UI::Widget::SpinButton(fill_width, 1.0, 2);
#else
                    Inkscape::UI::Widget::SpinButton *e = new Inkscape::UI::Widget::SpinButton (*fill_width, 1.0, 2);
#endif
                    e->set_tooltip_text (_("Width of the rectangle to be filled"));
                    e->set_width_chars (7);
                    e->set_digits (4);
                    gtk_box_pack_start (GTK_BOX (hb), GTK_WIDGET(e->gobj()), TRUE, TRUE, 0);
                    // TODO: C++ification
            g_signal_connect(G_OBJECT(fill_width->gobj()), "value_changed",
                                       G_CALLBACK(clonetiler_fill_width_changed), unit_menu);
                }
                {
                    GtkWidget *l = gtk_label_new ("");
                    gtk_label_set_markup (GTK_LABEL(l), "&#215;");
                    gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
                    gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
                }

                {
                    // Height spinbutton
#if WITH_GTKMM_3_0
                    fill_height = Gtk::Adjustment::create(0.0, -1e6, 1e6, 1.0, 10.0, 0);
#else
                    fill_height = new Gtk::Adjustment (0.0, -1e6, 1e6, 1.0, 10.0, 0);
#endif

                    double value = prefs->getDouble(prefs_path + "fillheight", 50.0);
                    Inkscape::Util::Unit const *unit = unit_menu->getUnit();
                    gdouble const units = Inkscape::Util::Quantity::convert(value, "px", unit);
                    fill_height->set_value (units);

#if WITH_GTKMM_3_0
                    Inkscape::UI::Widget::SpinButton *e = new Inkscape::UI::Widget::SpinButton(fill_height, 1.0, 2);
#else
                    Inkscape::UI::Widget::SpinButton *e = new Inkscape::UI::Widget::SpinButton (*fill_height, 1.0, 2);
#endif
                    e->set_tooltip_text (_("Height of the rectangle to be filled"));
                    e->set_width_chars (7);
                    e->set_digits (4);
                    gtk_box_pack_start (GTK_BOX (hb), GTK_WIDGET(e->gobj()), TRUE, TRUE, 0);
                    // TODO: C++ification
            g_signal_connect(G_OBJECT(fill_height->gobj()), "value_changed",
                                       G_CALLBACK(clonetiler_fill_height_changed), unit_menu);
                }

                gtk_box_pack_start (GTK_BOX (hb), (GtkWidget*) unit_menu->gobj(), TRUE, TRUE, 0);
                clonetiler_table_attach (table, hb, 0.0, 2, 2);

            }

            // Switch
            GtkWidget* radio;
            {
                radio = gtk_radio_button_new_with_label (NULL, _("Rows, columns: "));
                gtk_widget_set_tooltip_text (radio, _("Create the specified number of rows and columns"));
                clonetiler_table_attach (table, radio, 0.0, 1, 1);
                g_signal_connect (G_OBJECT (radio), "toggled", G_CALLBACK (clonetiler_switch_to_create), (gpointer) dlg);
            }
            if (!prefs->getBool(prefs_path + "fillrect")) {
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), TRUE);
                gtk_toggle_button_toggled (GTK_TOGGLE_BUTTON (radio));
            }
            {
                radio = gtk_radio_button_new_with_label (gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio)), _("Width, height: "));
                gtk_widget_set_tooltip_text (radio, _("Fill the specified width and height with the tiling"));
                clonetiler_table_attach (table, radio, 0.0, 2, 1);
                g_signal_connect (G_OBJECT (radio), "toggled", G_CALLBACK (clonetiler_switch_to_fill), (gpointer) dlg);
            }
            if (prefs->getBool(prefs_path + "fillrect")) {
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), TRUE);
                gtk_toggle_button_toggled (GTK_TOGGLE_BUTTON (radio));
            }
        }


        // Use saved pos
        {
#if GTK_CHECK_VERSION(3,0,0)
            GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, VB_MARGIN);
            gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
            GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);
#endif
            gtk_box_pack_start (GTK_BOX (mainbox), hb, FALSE, FALSE, 0);

            GtkWidget *b  = gtk_check_button_new_with_label (_("Use saved size and position of the tile"));
            bool keepbbox = prefs->getBool(prefs_path + "keepbbox", true);
            gtk_toggle_button_set_active ((GtkToggleButton *) b, keepbbox);
            gtk_widget_set_tooltip_text (b, _("Pretend that the size and position of the tile are the same as the last time you tiled it (if any), instead of using the current size"));
            gtk_box_pack_start (GTK_BOX (hb), b, FALSE, FALSE, 0);

            g_signal_connect(G_OBJECT(b), "toggled",
                               G_CALLBACK(clonetiler_keep_bbox_toggled), NULL);
        }

        // Statusbar
        {
#if GTK_CHECK_VERSION(3,0,0)
            GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, VB_MARGIN);
            gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
            GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);
#endif
            gtk_box_pack_end (GTK_BOX (mainbox), hb, FALSE, FALSE, 0);
            GtkWidget *l = gtk_label_new("");
            g_object_set_data (G_OBJECT(dlg), "status", (gpointer) l);
            gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);
        }

        // Buttons
        {
#if GTK_CHECK_VERSION(3,0,0)
            GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, VB_MARGIN);
            gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
            GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);
#endif
            gtk_box_pack_start (GTK_BOX (mainbox), hb, FALSE, FALSE, 0);

            {
                GtkWidget *b = gtk_button_new ();
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup_with_mnemonic (GTK_LABEL(l), _(" <b>_Create</b> "));
                gtk_container_add (GTK_CONTAINER(b), l);
                gtk_widget_set_tooltip_text (b, _("Create and tile the clones of the selection"));
                g_signal_connect (G_OBJECT (b), "clicked", G_CALLBACK (clonetiler_apply), dlg);
                gtk_box_pack_end (GTK_BOX (hb), b, FALSE, FALSE, 0);
            }

            { // buttons which are enabled only when there are tiled clones
#if GTK_CHECK_VERSION(3,0,0)
                GtkWidget *sb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
                gtk_box_set_homogeneous(GTK_BOX(sb), FALSE);
#else
                GtkWidget *sb = gtk_hbox_new(FALSE, 0);
#endif
                gtk_box_pack_end (GTK_BOX (hb), sb, FALSE, FALSE, 0);
                g_object_set_data (G_OBJECT(dlg), "buttons_on_tiles", (gpointer) sb);
                {
                    // TRANSLATORS: if a group of objects are "clumped" together, then they
                    //  are unevenly spread in the given amount of space - as shown in the
                    //  diagrams on the left in the following screenshot:
                    //  http://www.inkscape.org/screenshots/gallery/inkscape-0.42-CVS-tiles-unclump.png
                    //  So unclumping is the process of spreading a number of objects out more evenly.
                    GtkWidget *b = gtk_button_new_with_mnemonic (_(" _Unclump "));
                    gtk_widget_set_tooltip_text (b, _("Spread out clones to reduce clumping; can be applied repeatedly"));
                    g_signal_connect (G_OBJECT (b), "clicked", G_CALLBACK (clonetiler_unclump), NULL);
                    gtk_box_pack_end (GTK_BOX (sb), b, FALSE, FALSE, 0);
                }

                {
                    GtkWidget *b = gtk_button_new_with_mnemonic (_(" Re_move "));
                    gtk_widget_set_tooltip_text (b, _("Remove existing tiled clones of the selected object (siblings only)"));
                    g_signal_connect (G_OBJECT (b), "clicked", G_CALLBACK (clonetiler_remove), gpointer(dlg));
                    gtk_box_pack_end (GTK_BOX (sb), b, FALSE, FALSE, 0);
                }

                // connect to global selection changed signal (so we can change desktops) and
                // external_change (so we're not fooled by undo)
                selectChangedConn = INKSCAPE.signal_selection_changed.connect(sigc::bind(sigc::ptr_fun(&CloneTiler::clonetiler_change_selection), dlg));
                externChangedConn = INKSCAPE.signal_external_change.connect   (sigc::bind(sigc::ptr_fun(&CloneTiler::clonetiler_external_change), dlg));

                g_signal_connect(G_OBJECT(dlg), "destroy", G_CALLBACK(clonetiler_disconnect_gsignal), this);

                // update now
                clonetiler_change_selection (SP_ACTIVE_DESKTOP->getSelection(), dlg);
            }

            {
                GtkWidget *b = gtk_button_new_with_mnemonic (_(" R_eset "));
                // TRANSLATORS: "change" is a noun here
                gtk_widget_set_tooltip_text (b, _("Reset all shifts, scales, rotates, opacity and color changes in the dialog to zero"));
                g_signal_connect (G_OBJECT (b), "clicked", G_CALLBACK (clonetiler_reset), dlg);
                gtk_box_pack_start (GTK_BOX (hb), b, FALSE, FALSE, 0);
            }
        }

        gtk_widget_show_all (mainbox);

    }

    show_all();

    desktopChangeConn = deskTrack.connectDesktopChanged( sigc::mem_fun(*this, &CloneTiler::setTargetDesktop) );
    deskTrack.connect(GTK_WIDGET(gobj()));

}

CloneTiler::~CloneTiler (void)
{
    //subselChangedConn.disconnect();
    //selectChangedConn.disconnect();
    //selectModifiedConn.disconnect();
    desktopChangeConn.disconnect();
    deskTrack.disconnect();
    color_changed_connection.disconnect();
}

void CloneTiler::setDesktop(SPDesktop *desktop)
{
    Panel::setDesktop(desktop);
    deskTrack.setBase(desktop);
}

void CloneTiler::setTargetDesktop(SPDesktop *desktop)
{
    if (this->desktop != desktop) {
        if (this->desktop) {
            //selectModifiedConn.disconnect();
            //subselChangedConn.disconnect();
            //selectChangedConn.disconnect();
        }
        this->desktop = desktop;
        if (desktop && desktop->selection) {
            //selectChangedConn = desktop->selection->connectChanged(sigc::hide(sigc::mem_fun(*this, &CloneTiler::clonetiler_change_selection)));
            //subselChangedConn = desktop->connectToolSubselectionChanged(sigc::hide(sigc::mem_fun(*this, &CloneTiler::clonetiler_change_selection)));
            //selectModifiedConn = desktop->selection->connectModified(sigc::hide<0>(sigc::mem_fun(*this, &CloneTiler::clonetiler_change_selection)));
        }
    }
}

void CloneTiler::on_picker_color_changed(guint rgba)
{
    static bool is_updating = false;
    if (is_updating || !SP_ACTIVE_DESKTOP)
        return;

    is_updating = true;

    gchar c[32];
    sp_svg_write_color(c, sizeof(c), rgba);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setString(prefs_path + "initial_color", c);

    is_updating = false;
}

void CloneTiler::clonetiler_change_selection(Inkscape::Selection *selection, GtkWidget *dlg)
{
    GtkWidget *buttons = GTK_WIDGET(g_object_get_data (G_OBJECT(dlg), "buttons_on_tiles"));
    GtkWidget *status = GTK_WIDGET(g_object_get_data (G_OBJECT(dlg), "status"));

    if (selection->isEmpty()) {
        gtk_widget_set_sensitive (buttons, FALSE);
        gtk_label_set_markup (GTK_LABEL(status), _("<small>Nothing selected.</small>"));
        return;
    }

    if (selection->itemList().size() > 1) {
        gtk_widget_set_sensitive (buttons, FALSE);
        gtk_label_set_markup (GTK_LABEL(status), _("<small>More than one object selected.</small>"));
        return;
    }

    guint n = clonetiler_number_of_clones(selection->singleItem());
    if (n > 0) {
        gtk_widget_set_sensitive (buttons, TRUE);
        gchar *sta = g_strdup_printf (_("<small>Object has <b>%d</b> tiled clones.</small>"), n);
        gtk_label_set_markup (GTK_LABEL(status), sta);
        g_free (sta);
    } else {
        gtk_widget_set_sensitive (buttons, FALSE);
        gtk_label_set_markup (GTK_LABEL(status), _("<small>Object has no tiled clones.</small>"));
    }
}

void CloneTiler::clonetiler_external_change(GtkWidget *dlg)
{
    clonetiler_change_selection (SP_ACTIVE_DESKTOP->getSelection(), dlg);
}

void CloneTiler::clonetiler_disconnect_gsignal(GObject *, gpointer source)
{
    g_return_if_fail(source != NULL);

    CloneTiler* dlg = reinterpret_cast<CloneTiler*>(source);
    dlg->selectChangedConn.disconnect();
    dlg->externChangedConn.disconnect();
}

Geom::Affine CloneTiler::clonetiler_get_transform(
    // symmetry group
    int type,

    // row, column
    int i, int j,

    // center, width, height of the tile
    double cx, double cy,
    double w,  double h,

    // values from the dialog:
    // Shift
    double shiftx_per_i,      double shifty_per_i,
    double shiftx_per_j,      double shifty_per_j,
    double shiftx_rand,       double shifty_rand,
    double shiftx_exp,        double shifty_exp,
    int    shiftx_alternate,  int    shifty_alternate,
    int    shiftx_cumulate,   int    shifty_cumulate,
    int    shiftx_excludew,   int    shifty_excludeh,

    // Scale
    double scalex_per_i,      double scaley_per_i,
    double scalex_per_j,      double scaley_per_j,
    double scalex_rand,       double scaley_rand,
    double scalex_exp,        double scaley_exp,
    double scalex_log,        double scaley_log,
    int    scalex_alternate,  int    scaley_alternate,
    int    scalex_cumulate,   int    scaley_cumulate,

    // Rotation
    double rotate_per_i,      double rotate_per_j,
    double rotate_rand,
    int    rotate_alternatei, int    rotate_alternatej,
    int    rotate_cumulatei,  int    rotate_cumulatej
    )
{

    // Shift (in units of tile width or height) -------------
    double delta_shifti = 0.0;
    double delta_shiftj = 0.0;

    if( shiftx_alternate ) {
        delta_shifti = (double)(i%2);
    } else {
        if( shiftx_cumulate ) {  // Should the delta shifts be cumulative (i.e. 1, 1+2, 1+2+3, ...)
            delta_shifti = (double)(i*i);
        } else {
            delta_shifti = (double)i;
        }
    }

    if( shifty_alternate ) {
        delta_shiftj = (double)(j%2);
    } else {
        if( shifty_cumulate ) {
            delta_shiftj = (double)(j*j);
        } else {
            delta_shiftj = (double)j;
        }
    }

    // Random shift, only calculate if non-zero.
    double delta_shiftx_rand = 0.0;
    double delta_shifty_rand = 0.0;
    if( shiftx_rand != 0.0 ) delta_shiftx_rand = shiftx_rand * g_random_double_range (-1, 1);
    if( shifty_rand != 0.0 ) delta_shifty_rand = shifty_rand * g_random_double_range (-1, 1);


    // Delta shift (units of tile width/height)
    double di = shiftx_per_i * delta_shifti  + shiftx_per_j * delta_shiftj + delta_shiftx_rand;
    double dj = shifty_per_i * delta_shifti  + shifty_per_j * delta_shiftj + delta_shifty_rand;

    // Shift in actual x and y, used below
    double dx = w * di;
    double dy = h * dj;

    double shifti = di;
    double shiftj = dj;

    // Include tile width and height in shift if required
    if( !shiftx_excludew ) shifti += i;
    if( !shifty_excludeh ) shiftj += j;

    // Add exponential shift if necessary
    if ( shiftx_exp != 1.0 ) shifti = pow( shifti, shiftx_exp );
    if ( shifty_exp != 1.0 ) shiftj = pow( shiftj, shifty_exp );

    // Final shift
    Geom::Affine rect_translate (Geom::Translate (w * shifti, h * shiftj));

    // Rotation (in degrees) ------------
    double delta_rotationi = 0.0;
    double delta_rotationj = 0.0;

    if( rotate_alternatei ) {
        delta_rotationi = (double)(i%2);
    } else {
        if( rotate_cumulatei ) {
            delta_rotationi = (double)(i*i + i)/2.0;
        } else {
            delta_rotationi = (double)i;
        }
    }

    if( rotate_alternatej ) {
        delta_rotationj = (double)(j%2);
    } else {
        if( rotate_cumulatej ) {
            delta_rotationj = (double)(j*j + j)/2.0;
        } else {
            delta_rotationj = (double)j;
        }
    }

    double delta_rotate_rand = 0.0;
    if( rotate_rand != 0.0 ) delta_rotate_rand = rotate_rand * 180.0 * g_random_double_range (-1, 1);

    double dr = rotate_per_i * delta_rotationi + rotate_per_j * delta_rotationj + delta_rotate_rand;

    // Scale (times the original) -----------
    double delta_scalei = 0.0;
    double delta_scalej = 0.0;

    if( scalex_alternate ) {
        delta_scalei = (double)(i%2);
    } else {
        if( scalex_cumulate ) {  // Should the delta scales be cumulative (i.e. 1, 1+2, 1+2+3, ...)
            delta_scalei = (double)(i*i + i)/2.0;
        } else {
            delta_scalei = (double)i;
        }
    }

    if( scaley_alternate ) {
        delta_scalej = (double)(j%2);
    } else {
        if( scaley_cumulate ) {
            delta_scalej = (double)(j*j + j)/2.0;
        } else {
            delta_scalej = (double)j;
        }
    }

    // Random scale, only calculate if non-zero.
    double delta_scalex_rand = 0.0;
    double delta_scaley_rand = 0.0;
    if( scalex_rand != 0.0 ) delta_scalex_rand = scalex_rand * g_random_double_range (-1, 1);
    if( scaley_rand != 0.0 ) delta_scaley_rand = scaley_rand * g_random_double_range (-1, 1);
    // But if random factors are same, scale x and y proportionally
    if( scalex_rand == scaley_rand ) delta_scalex_rand = delta_scaley_rand;

    // Total delta scale
    double scalex = 1.0 + scalex_per_i * delta_scalei  + scalex_per_j * delta_scalej + delta_scalex_rand;
    double scaley = 1.0 + scaley_per_i * delta_scalei  + scaley_per_j * delta_scalej + delta_scaley_rand;

    if( scalex < 0.0 ) scalex = 0.0;
    if( scaley < 0.0 ) scaley = 0.0;

    // Add exponential scale if necessary
    if ( scalex_exp != 1.0 ) scalex = pow( scalex, scalex_exp );
    if ( scaley_exp != 1.0 ) scaley = pow( scaley, scaley_exp );

    // Add logarithmic factor if necessary
    if ( scalex_log  > 0.0 ) scalex = pow( scalex_log, scalex - 1.0 );
    if ( scaley_log  > 0.0 ) scaley = pow( scaley_log, scaley - 1.0 );
    // Alternative using rotation angle
    //if ( scalex_log  != 1.0 ) scalex *= pow( scalex_log, M_PI*dr/180 );
    //if ( scaley_log  != 1.0 ) scaley *= pow( scaley_log, M_PI*dr/180 );


    // Calculate transformation matrices, translating back to "center of tile" (rotation center) before transforming
    Geom::Affine drot_c   = Geom::Translate(-cx, -cy) * Geom::Rotate (M_PI*dr/180)    * Geom::Translate(cx, cy);

    Geom::Affine dscale_c = Geom::Translate(-cx, -cy) * Geom::Scale (scalex, scaley)  * Geom::Translate(cx, cy);

    Geom::Affine d_s_r = dscale_c * drot_c;

    Geom::Affine rotate_180_c  = Geom::Translate(-cx, -cy) * Geom::Rotate (M_PI)      * Geom::Translate(cx, cy);

    Geom::Affine rotate_90_c   = Geom::Translate(-cx, -cy) * Geom::Rotate (-M_PI/2)   * Geom::Translate(cx, cy);
    Geom::Affine rotate_m90_c  = Geom::Translate(-cx, -cy) * Geom::Rotate ( M_PI/2)   * Geom::Translate(cx, cy);

    Geom::Affine rotate_120_c  = Geom::Translate(-cx, -cy) * Geom::Rotate (-2*M_PI/3) * Geom::Translate(cx, cy);
    Geom::Affine rotate_m120_c = Geom::Translate(-cx, -cy) * Geom::Rotate ( 2*M_PI/3) * Geom::Translate(cx, cy);

    Geom::Affine rotate_60_c   = Geom::Translate(-cx, -cy) * Geom::Rotate (-M_PI/3)   * Geom::Translate(cx, cy);
    Geom::Affine rotate_m60_c  = Geom::Translate(-cx, -cy) * Geom::Rotate ( M_PI/3)   * Geom::Translate(cx, cy);

    Geom::Affine flip_x        = Geom::Translate(-cx, -cy) * Geom::Scale (-1, 1)      * Geom::Translate(cx, cy);
    Geom::Affine flip_y        = Geom::Translate(-cx, -cy) * Geom::Scale (1, -1)      * Geom::Translate(cx, cy);


    // Create tile with required symmetry
    const double cos60 = cos(M_PI/3);
    const double sin60 = sin(M_PI/3);
    const double cos30 = cos(M_PI/6);
    const double sin30 = sin(M_PI/6);

    switch (type) {

    case TILE_P1:
        return d_s_r * rect_translate;
        break;

    case TILE_P2:
        if (i % 2 == 0) {
            return d_s_r * rect_translate;
        } else {
            return d_s_r * rotate_180_c * rect_translate;
        }
        break;

    case TILE_PM:
        if (i % 2 == 0) {
            return d_s_r * rect_translate;
        } else {
            return d_s_r * flip_x * rect_translate;
        }
        break;

    case TILE_PG:
        if (j % 2 == 0) {
            return d_s_r * rect_translate;
        } else {
            return d_s_r * flip_x * rect_translate;
        }
        break;

    case TILE_CM:
        if ((i + j) % 2 == 0) {
            return d_s_r * rect_translate;
        } else {
            return d_s_r * flip_x * rect_translate;
        }
        break;

    case TILE_PMM:
        if (j % 2 == 0) {
            if (i % 2 == 0) {
                return d_s_r * rect_translate;
            } else {
                return d_s_r * flip_x * rect_translate;
            }
        } else {
            if (i % 2 == 0) {
                return d_s_r * flip_y * rect_translate;
            } else {
                return d_s_r * flip_x * flip_y * rect_translate;
            }
        }
        break;

    case TILE_PMG:
        if (j % 2 == 0) {
            if (i % 2 == 0) {
                return d_s_r * rect_translate;
            } else {
                return d_s_r * rotate_180_c * rect_translate;
            }
        } else {
            if (i % 2 == 0) {
                return d_s_r * flip_y * rect_translate;
            } else {
                return d_s_r * rotate_180_c * flip_y * rect_translate;
            }
        }
        break;

    case TILE_PGG:
        if (j % 2 == 0) {
            if (i % 2 == 0) {
                return d_s_r * rect_translate;
            } else {
                return d_s_r * flip_y * rect_translate;
            }
        } else {
            if (i % 2 == 0) {
                return d_s_r * rotate_180_c * rect_translate;
            } else {
                return d_s_r * rotate_180_c * flip_y * rect_translate;
            }
        }
        break;

    case TILE_CMM:
        if (j % 4 == 0) {
            if (i % 2 == 0) {
                return d_s_r * rect_translate;
            } else {
                return d_s_r * flip_x * rect_translate;
            }
        } else if (j % 4 == 1) {
            if (i % 2 == 0) {
                return d_s_r * flip_y * rect_translate;
            } else {
                return d_s_r * flip_x * flip_y * rect_translate;
            }
        } else if (j % 4 == 2) {
            if (i % 2 == 1) {
                return d_s_r * rect_translate;
            } else {
                return d_s_r * flip_x * rect_translate;
            }
        } else {
            if (i % 2 == 1) {
                return d_s_r * flip_y * rect_translate;
            } else {
                return d_s_r * flip_x * flip_y * rect_translate;
            }
        }
        break;

    case TILE_P4:
    {
        Geom::Affine ori  (Geom::Translate ((w + h) * pow((i/2), shiftx_exp) + dx,  (h + w) * pow((j/2), shifty_exp) + dy));
        Geom::Affine dia1 (Geom::Translate (w/2 + h/2, -h/2 + w/2));
        Geom::Affine dia2 (Geom::Translate (-w/2 + h/2, h/2 + w/2));
        if (j % 2 == 0) {
            if (i % 2 == 0) {
                return d_s_r * ori;
            } else {
                return d_s_r * rotate_m90_c * dia1 * ori;
            }
        } else {
            if (i % 2 == 0) {
                return d_s_r * rotate_90_c * dia2 * ori;
            } else {
                return d_s_r * rotate_180_c * dia1 * dia2 * ori;
            }
        }
    }
    break;

    case TILE_P4M:
    {
        double max = MAX(w, h);
        Geom::Affine ori (Geom::Translate ((max + max) * pow((i/4), shiftx_exp) + dx,  (max + max) * pow((j/2), shifty_exp) + dy));
        Geom::Affine dia1 (Geom::Translate ( w/2 - h/2, h/2 - w/2));
        Geom::Affine dia2 (Geom::Translate (-h/2 + w/2, w/2 - h/2));
        if (j % 2 == 0) {
            if (i % 4 == 0) {
                return d_s_r * ori;
            } else if (i % 4 == 1) {
                return d_s_r * flip_y * rotate_m90_c * dia1 * ori;
            } else if (i % 4 == 2) {
                return d_s_r * rotate_m90_c * dia1 * Geom::Translate (h, 0) * ori;
            } else if (i % 4 == 3) {
                return d_s_r * flip_x * Geom::Translate (w, 0) * ori;
            }
        } else {
            if (i % 4 == 0) {
                return d_s_r * flip_y * Geom::Translate(0, h) * ori;
            } else if (i % 4 == 1) {
                return d_s_r * rotate_90_c * dia2 * Geom::Translate(0, h) * ori;
            } else if (i % 4 == 2) {
                return d_s_r * flip_y * rotate_90_c * dia2 * Geom::Translate(h, 0) * Geom::Translate(0, h) * ori;
            } else if (i % 4 == 3) {
                return d_s_r * flip_y * flip_x * Geom::Translate(w, 0) * Geom::Translate(0, h) * ori;
            }
        }
    }
    break;

    case TILE_P4G:
    {
        double max = MAX(w, h);
        Geom::Affine ori (Geom::Translate ((max + max) * pow((i/4), shiftx_exp) + dx,  (max + max) * pow(j, shifty_exp) + dy));
        Geom::Affine dia1 (Geom::Translate ( w/2 + h/2, h/2 - w/2));
        Geom::Affine dia2 (Geom::Translate (-h/2 + w/2, w/2 + h/2));
        if (((i/4) + j) % 2 == 0) {
            if (i % 4 == 0) {
                return d_s_r * ori;
            } else if (i % 4 == 1) {
                return d_s_r * rotate_m90_c * dia1 * ori;
            } else if (i % 4 == 2) {
                return d_s_r * rotate_90_c * dia2 * ori;
            } else if (i % 4 == 3) {
                return d_s_r * rotate_180_c * dia1 * dia2 * ori;
            }
        } else {
            if (i % 4 == 0) {
                return d_s_r * flip_y * Geom::Translate (0, h) * ori;
            } else if (i % 4 == 1) {
                return d_s_r * flip_y * rotate_m90_c * dia1 * Geom::Translate (-h, 0) * ori;
            } else if (i % 4 == 2) {
                return d_s_r * flip_y * rotate_90_c * dia2 * Geom::Translate (h, 0) * ori;
            } else if (i % 4 == 3) {
                return d_s_r * flip_x * Geom::Translate (w, 0) * ori;
            }
        }
    }
    break;

    case TILE_P3:
    {
        double width;
        double height;
        Geom::Affine dia1;
        Geom::Affine dia2;
        if (w > h) {
            width  = w + w * cos60;
            height = 2 * w * sin60;
            dia1 = Geom::Affine (Geom::Translate (w/2 + w/2 * cos60, -(w/2 * sin60)));
            dia2 = dia1 * Geom::Affine (Geom::Translate (0, 2 * (w/2 * sin60)));
        } else {
            width = h * cos (M_PI/6);
            height = h;
            dia1 = Geom::Affine (Geom::Translate (h/2 * cos30, -(h/2 * sin30)));
            dia2 = dia1 * Geom::Affine (Geom::Translate (0, h/2));
        }
        Geom::Affine ori (Geom::Translate (width * pow((2*(i/3) + j%2), shiftx_exp) + dx,  (height/2) * pow(j, shifty_exp) + dy));
        if (i % 3 == 0) {
            return d_s_r * ori;
        } else if (i % 3 == 1) {
            return d_s_r * rotate_m120_c * dia1 * ori;
        } else if (i % 3 == 2) {
            return d_s_r * rotate_120_c * dia2 * ori;
        }
    }
    break;

    case TILE_P31M:
    {
        Geom::Affine ori;
        Geom::Affine dia1;
        Geom::Affine dia2;
        Geom::Affine dia3;
        Geom::Affine dia4;
        if (w > h) {
            ori = Geom::Affine(Geom::Translate (w * pow((i/6) + 0.5*(j%2), shiftx_exp) + dx,  (w * cos30) * pow(j, shifty_exp) + dy));
            dia1 = Geom::Affine (Geom::Translate (0, h/2) * Geom::Translate (w/2, 0) * Geom::Translate (w/2 * cos60, -w/2 * sin60) * Geom::Translate (-h/2 * cos30, -h/2 * sin30) );
            dia2 = dia1 * Geom::Affine (Geom::Translate (h * cos30, h * sin30));
            dia3 = dia2 * Geom::Affine (Geom::Translate (0, 2 * (w/2 * sin60 - h/2 * sin30)));
            dia4 = dia3 * Geom::Affine (Geom::Translate (-h * cos30, h * sin30));
        } else {
            ori  = Geom::Affine (Geom::Translate (2*h * cos30  * pow((i/6 + 0.5*(j%2)), shiftx_exp) + dx,  (2*h - h * sin30) * pow(j, shifty_exp) + dy));
            dia1 = Geom::Affine (Geom::Translate (0, -h/2) * Geom::Translate (h/2 * cos30, h/2 * sin30));
            dia2 = dia1 * Geom::Affine (Geom::Translate (h * cos30, h * sin30));
            dia3 = dia2 * Geom::Affine (Geom::Translate (0, h/2));
            dia4 = dia3 * Geom::Affine (Geom::Translate (-h * cos30, h * sin30));
        }
        if (i % 6 == 0) {
            return d_s_r * ori;
        } else if (i % 6 == 1) {
            return d_s_r * flip_y * rotate_m120_c * dia1 * ori;
        } else if (i % 6 == 2) {
            return d_s_r * rotate_m120_c * dia2 * ori;
        } else if (i % 6 == 3) {
            return d_s_r * flip_y * rotate_120_c * dia3 * ori;
        } else if (i % 6 == 4) {
            return d_s_r * rotate_120_c * dia4 * ori;
        } else if (i % 6 == 5) {
            return d_s_r * flip_y * Geom::Translate(0, h) * ori;
        }
    }
    break;

    case TILE_P3M1:
    {
        double width;
        double height;
        Geom::Affine dia1;
        Geom::Affine dia2;
        Geom::Affine dia3;
        Geom::Affine dia4;
        if (w > h) {
            width = w + w * cos60;
            height = 2 * w * sin60;
            dia1 = Geom::Affine (Geom::Translate (0, h/2) * Geom::Translate (w/2, 0) * Geom::Translate (w/2 * cos60, -w/2 * sin60) * Geom::Translate (-h/2 * cos30, -h/2 * sin30) );
            dia2 = dia1 * Geom::Affine (Geom::Translate (h * cos30, h * sin30));
            dia3 = dia2 * Geom::Affine (Geom::Translate (0, 2 * (w/2 * sin60 - h/2 * sin30)));
            dia4 = dia3 * Geom::Affine (Geom::Translate (-h * cos30, h * sin30));
        } else {
            width = 2 * h * cos (M_PI/6);
            height = 2 * h;
            dia1 = Geom::Affine (Geom::Translate (0, -h/2) * Geom::Translate (h/2 * cos30, h/2 * sin30));
            dia2 = dia1 * Geom::Affine (Geom::Translate (h * cos30, h * sin30));
            dia3 = dia2 * Geom::Affine (Geom::Translate (0, h/2));
            dia4 = dia3 * Geom::Affine (Geom::Translate (-h * cos30, h * sin30));
        }
        Geom::Affine ori (Geom::Translate (width * pow((2*(i/6) + j%2), shiftx_exp) + dx,  (height/2) * pow(j, shifty_exp) + dy));
        if (i % 6 == 0) {
            return d_s_r * ori;
        } else if (i % 6 == 1) {
            return d_s_r * flip_y * rotate_m120_c * dia1 * ori;
        } else if (i % 6 == 2) {
            return d_s_r * rotate_m120_c * dia2 * ori;
        } else if (i % 6 == 3) {
            return d_s_r * flip_y * rotate_120_c * dia3 * ori;
        } else if (i % 6 == 4) {
            return d_s_r * rotate_120_c * dia4 * ori;
        } else if (i % 6 == 5) {
            return d_s_r * flip_y * Geom::Translate(0, h) * ori;
        }
    }
    break;

    case TILE_P6:
    {
        Geom::Affine ori;
        Geom::Affine dia1;
        Geom::Affine dia2;
        Geom::Affine dia3;
        Geom::Affine dia4;
        Geom::Affine dia5;
        if (w > h) {
            ori = Geom::Affine(Geom::Translate (w * pow((2*(i/6) + (j%2)), shiftx_exp) + dx,  (2*w * sin60) * pow(j, shifty_exp) + dy));
            dia1 = Geom::Affine (Geom::Translate (w/2 * cos60, -w/2 * sin60));
            dia2 = dia1 * Geom::Affine (Geom::Translate (w/2, 0));
            dia3 = dia2 * Geom::Affine (Geom::Translate (w/2 * cos60, w/2 * sin60));
            dia4 = dia3 * Geom::Affine (Geom::Translate (-w/2 * cos60, w/2 * sin60));
            dia5 = dia4 * Geom::Affine (Geom::Translate (-w/2, 0));
        } else {
            ori = Geom::Affine(Geom::Translate (2*h * cos30 * pow((i/6 + 0.5*(j%2)), shiftx_exp) + dx,  (h + h * sin30) * pow(j, shifty_exp) + dy));
            dia1 = Geom::Affine (Geom::Translate (-w/2, -h/2) * Geom::Translate (h/2 * cos30, -h/2 * sin30) * Geom::Translate (w/2 * cos60, w/2 * sin60));
            dia2 = dia1 * Geom::Affine (Geom::Translate (-w/2 * cos60, -w/2 * sin60) * Geom::Translate (h/2 * cos30, -h/2 * sin30) * Geom::Translate (h/2 * cos30, h/2 * sin30) * Geom::Translate (-w/2 * cos60, w/2 * sin60));
            dia3 = dia2 * Geom::Affine (Geom::Translate (w/2 * cos60, -w/2 * sin60) * Geom::Translate (h/2 * cos30, h/2 * sin30) * Geom::Translate (-w/2, h/2));
            dia4 = dia3 * dia1.inverse();
            dia5 = dia3 * dia2.inverse();
        }
        if (i % 6 == 0) {
            return d_s_r * ori;
        } else if (i % 6 == 1) {
            return d_s_r * rotate_m60_c * dia1 * ori;
        } else if (i % 6 == 2) {
            return d_s_r * rotate_m120_c * dia2 * ori;
        } else if (i % 6 == 3) {
            return d_s_r * rotate_180_c * dia3 * ori;
        } else if (i % 6 == 4) {
            return d_s_r * rotate_120_c * dia4 * ori;
        } else if (i % 6 == 5) {
            return d_s_r * rotate_60_c * dia5 * ori;
        }
    }
    break;

    case TILE_P6M:
    {

        Geom::Affine ori;
        Geom::Affine dia1, dia2, dia3, dia4, dia5, dia6, dia7, dia8, dia9, dia10;
        if (w > h) {
            ori = Geom::Affine(Geom::Translate (w * pow((2*(i/12) + (j%2)), shiftx_exp) + dx,  (2*w * sin60) * pow(j, shifty_exp) + dy));
            dia1 = Geom::Affine (Geom::Translate (w/2, h/2) * Geom::Translate (-w/2 * cos60, -w/2 * sin60) * Geom::Translate (-h/2 * cos30, h/2 * sin30));
            dia2 = dia1 * Geom::Affine (Geom::Translate (h * cos30, -h * sin30));
            dia3 = dia2 * Geom::Affine (Geom::Translate (-h/2 * cos30, h/2 * sin30) * Geom::Translate (w * cos60, 0) * Geom::Translate (-h/2 * cos30, -h/2 * sin30));
            dia4 = dia3 * Geom::Affine (Geom::Translate (h * cos30, h * sin30));
            dia5 = dia4 * Geom::Affine (Geom::Translate (-h/2 * cos30, -h/2 * sin30) * Geom::Translate (-w/2 * cos60, w/2 * sin60) * Geom::Translate (w/2, -h/2));
            dia6 = dia5 * Geom::Affine (Geom::Translate (0, h));
            dia7 = dia6 * dia1.inverse();
            dia8 = dia6 * dia2.inverse();
            dia9 = dia6 * dia3.inverse();
            dia10 = dia6 * dia4.inverse();
        } else {
            ori = Geom::Affine(Geom::Translate (4*h * cos30 * pow((i/12 + 0.5*(j%2)), shiftx_exp) + dx,  (2*h  + 2*h * sin30) * pow(j, shifty_exp) + dy));
            dia1 = Geom::Affine (Geom::Translate (-w/2, -h/2) * Geom::Translate (h/2 * cos30, -h/2 * sin30) * Geom::Translate (w/2 * cos60, w/2 * sin60));
            dia2 = dia1 * Geom::Affine (Geom::Translate (h * cos30, -h * sin30));
            dia3 = dia2 * Geom::Affine (Geom::Translate (-w/2 * cos60, -w/2 * sin60) * Geom::Translate (h * cos30, 0) * Geom::Translate (-w/2 * cos60, w/2 * sin60));
            dia4 = dia3 * Geom::Affine (Geom::Translate (h * cos30, h * sin30));
            dia5 = dia4 * Geom::Affine (Geom::Translate (w/2 * cos60, -w/2 * sin60) * Geom::Translate (h/2 * cos30, h/2 * sin30) * Geom::Translate (-w/2, h/2));
            dia6 = dia5 * Geom::Affine (Geom::Translate (0, h));
            dia7 = dia6 * dia1.inverse();
            dia8 = dia6 * dia2.inverse();
            dia9 = dia6 * dia3.inverse();
            dia10 = dia6 * dia4.inverse();
        }
        if (i % 12 == 0) {
            return d_s_r * ori;
        } else if (i % 12 == 1) {
            return d_s_r * flip_y * rotate_m60_c * dia1 * ori;
        } else if (i % 12 == 2) {
            return d_s_r * rotate_m60_c * dia2 * ori;
        } else if (i % 12 == 3) {
            return d_s_r * flip_y * rotate_m120_c * dia3 * ori;
        } else if (i % 12 == 4) {
            return d_s_r * rotate_m120_c * dia4 * ori;
        } else if (i % 12 == 5) {
            return d_s_r * flip_x * dia5 * ori;
        } else if (i % 12 == 6) {
            return d_s_r * flip_x * flip_y * dia6 * ori;
        } else if (i % 12 == 7) {
            return d_s_r * flip_y * rotate_120_c * dia7 * ori;
        } else if (i % 12 == 8) {
            return d_s_r * rotate_120_c * dia8 * ori;
        } else if (i % 12 == 9) {
            return d_s_r * flip_y * rotate_60_c * dia9 * ori;
        } else if (i % 12 == 10) {
            return d_s_r * rotate_60_c * dia10 * ori;
        } else if (i % 12 == 11) {
            return d_s_r * flip_y * Geom::Translate (0, h) * ori;
        }
    }
    break;

    default:
        break;
    }

    return Geom::identity();
}

bool CloneTiler::clonetiler_is_a_clone_of(SPObject *tile, SPObject *obj)
{
    bool result = false;
    char *id_href = NULL;

    if (obj) {
        Inkscape::XML::Node *obj_repr = obj->getRepr();
        id_href = g_strdup_printf("#%s", obj_repr->attribute("id"));
    }

    if (dynamic_cast<SPUse *>(tile) &&
        tile->getRepr()->attribute("xlink:href") &&
        (!id_href || !strcmp(id_href, tile->getRepr()->attribute("xlink:href"))) &&
        tile->getRepr()->attribute("inkscape:tiled-clone-of") &&
        (!id_href || !strcmp(id_href, tile->getRepr()->attribute("inkscape:tiled-clone-of"))))
    {
        result = true;
    } else {
        result = false;
    }
    if (id_href) {
        g_free(id_href);
        id_href = 0;
    }
    return result;
}

void CloneTiler::clonetiler_trace_hide_tiled_clones_recursively(SPObject *from)
{
    if (!trace_drawing)
        return;

    for (SPObject *o = from->firstChild(); o != NULL; o = o->next) {
        SPItem *item = dynamic_cast<SPItem *>(o);
        if (item && clonetiler_is_a_clone_of(o, NULL)) {
            item->invoke_hide(trace_visionkey); // FIXME: hide each tiled clone's original too!
        }
        clonetiler_trace_hide_tiled_clones_recursively (o);
    }
}

void CloneTiler::clonetiler_trace_setup(SPDocument *doc, gdouble zoom, SPItem *original)
{
    trace_drawing = new Inkscape::Drawing();
    /* Create ArenaItem and set transform */
    trace_visionkey = SPItem::display_key_new(1);
    trace_doc = doc;
    trace_drawing->setRoot(trace_doc->getRoot()->invoke_show(*trace_drawing, trace_visionkey, SP_ITEM_SHOW_DISPLAY));

    // hide the (current) original and any tiled clones, we only want to pick the background
    original->invoke_hide(trace_visionkey);
    clonetiler_trace_hide_tiled_clones_recursively(trace_doc->getRoot());

    trace_doc->getRoot()->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    trace_doc->ensureUpToDate();

    trace_zoom = zoom;
}

guint32 CloneTiler::clonetiler_trace_pick(Geom::Rect box)
{
    if (!trace_drawing) {
        return 0;
    }

    trace_drawing->root()->setTransform(Geom::Scale(trace_zoom));
    trace_drawing->update();

    /* Item integer bbox in points */
    Geom::IntRect ibox = (box * Geom::Scale(trace_zoom)).roundOutwards();

    /* Find visible area */
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, ibox.width(), ibox.height());
    Inkscape::DrawingContext dc(s, ibox.min());
    /* Render */
    trace_drawing->render(dc, ibox);
    double R = 0, G = 0, B = 0, A = 0;
    ink_cairo_surface_average_color(s, R, G, B, A);
    cairo_surface_destroy(s);

    return SP_RGBA32_F_COMPOSE (R, G, B, A);
}

void CloneTiler::clonetiler_trace_finish()
{
    if (trace_doc) {
        trace_doc->getRoot()->invoke_hide(trace_visionkey);
        delete trace_drawing;
        trace_doc = NULL;
        trace_drawing = NULL;
    }
}

void CloneTiler::clonetiler_unclump(GtkWidget */*widget*/, void *)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL) {
        return;
    }

    Inkscape::Selection *selection = desktop->getSelection();

    // check if something is selected
    if (selection->isEmpty() || selection->itemList().size() > 1) {
        desktop->getMessageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>one object</b> whose tiled clones to unclump."));
        return;
    }

    SPObject *obj = selection->singleItem();
    SPObject *parent = obj->parent;

    std::vector<SPItem*> to_unclump; // not including the original

    for (SPObject *child = parent->firstChild(); child != NULL; child = child->next) {
        if (clonetiler_is_a_clone_of (child, obj)) {
            to_unclump.push_back((SPItem*)child);
        }
    }

    desktop->getDocument()->ensureUpToDate();
    reverse(to_unclump.begin(),to_unclump.end());
    unclump (to_unclump);

    DocumentUndo::done(desktop->getDocument(), SP_VERB_DIALOG_CLONETILER,
                       _("Unclump tiled clones"));
}

guint CloneTiler::clonetiler_number_of_clones(SPObject *obj)
{
    SPObject *parent = obj->parent;

    guint n = 0;

    for (SPObject *child = parent->firstChild(); child != NULL; child = child->next) {
        if (clonetiler_is_a_clone_of (child, obj)) {
            n ++;
        }
    }

    return n;
}

void CloneTiler::clonetiler_remove(GtkWidget */*widget*/, GtkWidget *dlg, bool do_undo/* = true*/)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL) {
        return;
    }

    Inkscape::Selection *selection = desktop->getSelection();

    // check if something is selected
    if (selection->isEmpty() || selection->itemList().size() > 1) {
        desktop->getMessageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select <b>one object</b> whose tiled clones to remove."));
        return;
    }

    SPObject *obj = selection->singleItem();
    SPObject *parent = obj->parent;

// remove old tiling
    GSList *to_delete = NULL;
    for (SPObject *child = parent->firstChild(); child != NULL; child = child->next) {
        if (clonetiler_is_a_clone_of (child, obj)) {
            to_delete = g_slist_prepend (to_delete, child);
        }
    }
    for (GSList *i = to_delete; i; i = i->next) {
        SPObject *obj = reinterpret_cast<SPObject *>(i->data);
        g_assert(obj != NULL);
        obj->deleteObject();
    }
    g_slist_free (to_delete);

    clonetiler_change_selection (selection, dlg);

    if (do_undo) {
        DocumentUndo::done(desktop->getDocument(), SP_VERB_DIALOG_CLONETILER,
                           _("Delete tiled clones"));
    }
}

Geom::Rect CloneTiler::transform_rect(Geom::Rect const &r, Geom::Affine const &m)
{
    using Geom::X;
    using Geom::Y;
    Geom::Point const p1 = r.corner(1) * m;
    Geom::Point const p2 = r.corner(2) * m;
    Geom::Point const p3 = r.corner(3) * m;
    Geom::Point const p4 = r.corner(4) * m;
    return Geom::Rect(
        Geom::Point(
            std::min(std::min(p1[X], p2[X]), std::min(p3[X], p4[X])),
            std::min(std::min(p1[Y], p2[Y]), std::min(p3[Y], p4[Y]))),
        Geom::Point(
            std::max(std::max(p1[X], p2[X]), std::max(p3[X], p4[X])),
            std::max(std::max(p1[Y], p2[Y]), std::max(p3[Y], p4[Y]))));
}

/**
Randomizes \a val by \a rand, with 0 < val < 1 and all values (including 0, 1) having the same
probability of being displaced.
 */
double CloneTiler::randomize01(double val, double rand)
{
    double base = MIN (val - rand, 1 - 2*rand);
    if (base < 0) {
        base = 0;
    }
    val = base + g_random_double_range (0, MIN (2 * rand, 1 - base));
    return CLAMP(val, 0, 1); // this should be unnecessary with the above provisions, but just in case...
}


void CloneTiler::clonetiler_apply(GtkWidget */*widget*/, GtkWidget *dlg)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL) {
        return;
    }
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Inkscape::Selection *selection = desktop->getSelection();

    // check if something is selected
    if (selection->isEmpty()) {
        desktop->getMessageStack()->flash(Inkscape::WARNING_MESSAGE, _("Select an <b>object</b> to clone."));
        return;
    }

    // Check if more than one object is selected.
    if (selection->itemList().size() > 1) {
        desktop->getMessageStack()->flash(Inkscape::ERROR_MESSAGE, _("If you want to clone several objects, <b>group</b> them and <b>clone the group</b>."));
        return;
    }

    // set "busy" cursor
    desktop->setWaitingCursor();

    // set statusbar text
    GtkWidget *status = GTK_WIDGET(g_object_get_data (G_OBJECT(dlg), "status"));
    gtk_label_set_markup (GTK_LABEL(status), _("<small>Creating tiled clones...</small>"));
    gtk_widget_queue_draw(GTK_WIDGET(status));
    gdk_window_process_all_updates();

    SPObject *obj = selection->singleItem();
    if (!obj) {
        // Should never happen (empty selection checked above).
        std::cerr << "CloneTiler::clonetile_apply(): No object in single item selection!!!" << std::endl;
        return;
    }
    Inkscape::XML::Node *obj_repr = obj->getRepr();
    const char *id_href = g_strdup_printf("#%s", obj_repr->attribute("id"));
    SPObject *parent = obj->parent;

    clonetiler_remove (NULL, dlg, false);

    double scale_units = Inkscape::Util::Quantity::convert(1, "px", &desktop->getDocument()->getSVGUnit());

    double shiftx_per_i = 0.01 * prefs->getDoubleLimited(prefs_path + "shiftx_per_i", 0, -10000, 10000);
    double shifty_per_i = 0.01 * prefs->getDoubleLimited(prefs_path + "shifty_per_i", 0, -10000, 10000);
    double shiftx_per_j = 0.01 * prefs->getDoubleLimited(prefs_path + "shiftx_per_j", 0, -10000, 10000);
    double shifty_per_j = 0.01 * prefs->getDoubleLimited(prefs_path + "shifty_per_j", 0, -10000, 10000);
    double shiftx_rand  = 0.01 * prefs->getDoubleLimited(prefs_path + "shiftx_rand", 0, 0, 1000);
    double shifty_rand  = 0.01 * prefs->getDoubleLimited(prefs_path + "shifty_rand", 0, 0, 1000);
    double shiftx_exp   =        prefs->getDoubleLimited(prefs_path + "shiftx_exp",   1, 0, 10);
    double shifty_exp   =        prefs->getDoubleLimited(prefs_path + "shifty_exp", 1, 0, 10);
    bool   shiftx_alternate =    prefs->getBool(prefs_path + "shiftx_alternate");
    bool   shifty_alternate =    prefs->getBool(prefs_path + "shifty_alternate");
    bool   shiftx_cumulate  =    prefs->getBool(prefs_path + "shiftx_cumulate");
    bool   shifty_cumulate  =    prefs->getBool(prefs_path + "shifty_cumulate");
    bool   shiftx_excludew  =    prefs->getBool(prefs_path + "shiftx_excludew");
    bool   shifty_excludeh  =    prefs->getBool(prefs_path + "shifty_excludeh");

    double scalex_per_i = 0.01 * prefs->getDoubleLimited(prefs_path + "scalex_per_i", 0, -100, 1000);
    double scaley_per_i = 0.01 * prefs->getDoubleLimited(prefs_path + "scaley_per_i", 0, -100, 1000);
    double scalex_per_j = 0.01 * prefs->getDoubleLimited(prefs_path + "scalex_per_j", 0, -100, 1000);
    double scaley_per_j = 0.01 * prefs->getDoubleLimited(prefs_path + "scaley_per_j", 0, -100, 1000);
    double scalex_rand  = 0.01 * prefs->getDoubleLimited(prefs_path + "scalex_rand",  0, 0, 1000);
    double scaley_rand  = 0.01 * prefs->getDoubleLimited(prefs_path + "scaley_rand",  0, 0, 1000);
    double scalex_exp   =        prefs->getDoubleLimited(prefs_path + "scalex_exp",   1, 0, 10);
    double scaley_exp   =        prefs->getDoubleLimited(prefs_path + "scaley_exp",   1, 0, 10);
    double scalex_log       =    prefs->getDoubleLimited(prefs_path + "scalex_log",   0, 0, 10);
    double scaley_log       =    prefs->getDoubleLimited(prefs_path + "scaley_log",   0, 0, 10);
    bool   scalex_alternate =    prefs->getBool(prefs_path + "scalex_alternate");
    bool   scaley_alternate =    prefs->getBool(prefs_path + "scaley_alternate");
    bool   scalex_cumulate  =    prefs->getBool(prefs_path + "scalex_cumulate");
    bool   scaley_cumulate  =    prefs->getBool(prefs_path + "scaley_cumulate");

    double rotate_per_i =        prefs->getDoubleLimited(prefs_path + "rotate_per_i", 0, -180, 180);
    double rotate_per_j =        prefs->getDoubleLimited(prefs_path + "rotate_per_j", 0, -180, 180);
    double rotate_rand =  0.01 * prefs->getDoubleLimited(prefs_path + "rotate_rand", 0, 0, 100);
    bool   rotate_alternatei   = prefs->getBool(prefs_path + "rotate_alternatei");
    bool   rotate_alternatej   = prefs->getBool(prefs_path + "rotate_alternatej");
    bool   rotate_cumulatei    = prefs->getBool(prefs_path + "rotate_cumulatei");
    bool   rotate_cumulatej    = prefs->getBool(prefs_path + "rotate_cumulatej");

    double blur_per_i =   0.01 * prefs->getDoubleLimited(prefs_path + "blur_per_i", 0, 0, 100);
    double blur_per_j =   0.01 * prefs->getDoubleLimited(prefs_path + "blur_per_j", 0, 0, 100);
    bool   blur_alternatei =     prefs->getBool(prefs_path + "blur_alternatei");
    bool   blur_alternatej =     prefs->getBool(prefs_path + "blur_alternatej");
    double blur_rand =    0.01 * prefs->getDoubleLimited(prefs_path + "blur_rand", 0, 0, 100);

    double opacity_per_i = 0.01 * prefs->getDoubleLimited(prefs_path + "opacity_per_i", 0, 0, 100);
    double opacity_per_j = 0.01 * prefs->getDoubleLimited(prefs_path + "opacity_per_j", 0, 0, 100);
    bool   opacity_alternatei =   prefs->getBool(prefs_path + "opacity_alternatei");
    bool   opacity_alternatej =   prefs->getBool(prefs_path + "opacity_alternatej");
    double opacity_rand =  0.01 * prefs->getDoubleLimited(prefs_path + "opacity_rand", 0, 0, 100);

    Glib::ustring initial_color =    prefs->getString(prefs_path + "initial_color");
    double hue_per_j =        0.01 * prefs->getDoubleLimited(prefs_path + "hue_per_j", 0, -100, 100);
    double hue_per_i =        0.01 * prefs->getDoubleLimited(prefs_path + "hue_per_i", 0, -100, 100);
    double hue_rand  =        0.01 * prefs->getDoubleLimited(prefs_path + "hue_rand", 0, 0, 100);
    double saturation_per_j = 0.01 * prefs->getDoubleLimited(prefs_path + "saturation_per_j", 0, -100, 100);
    double saturation_per_i = 0.01 * prefs->getDoubleLimited(prefs_path + "saturation_per_i", 0, -100, 100);
    double saturation_rand =  0.01 * prefs->getDoubleLimited(prefs_path + "saturation_rand", 0, 0, 100);
    double lightness_per_j =  0.01 * prefs->getDoubleLimited(prefs_path + "lightness_per_j", 0, -100, 100);
    double lightness_per_i =  0.01 * prefs->getDoubleLimited(prefs_path + "lightness_per_i", 0, -100, 100);
    double lightness_rand =   0.01 * prefs->getDoubleLimited(prefs_path + "lightness_rand", 0, 0, 100);
    bool   color_alternatej = prefs->getBool(prefs_path + "color_alternatej");
    bool   color_alternatei = prefs->getBool(prefs_path + "color_alternatei");

    int    type = prefs->getInt(prefs_path + "symmetrygroup", 0);
    bool   keepbbox = prefs->getBool(prefs_path + "keepbbox", true);
    int    imax = prefs->getInt(prefs_path + "imax", 2);
    int    jmax = prefs->getInt(prefs_path + "jmax", 2);

    bool   fillrect = prefs->getBool(prefs_path + "fillrect");
    double fillwidth = scale_units*prefs->getDoubleLimited(prefs_path + "fillwidth", 50, 0, 1e6);
    double fillheight = scale_units*prefs->getDoubleLimited(prefs_path + "fillheight", 50, 0, 1e6);

    bool   dotrace = prefs->getBool(prefs_path + "dotrace");
    int    pick = prefs->getInt(prefs_path + "pick");
    bool   pick_to_presence = prefs->getBool(prefs_path + "pick_to_presence");
    bool   pick_to_size = prefs->getBool(prefs_path + "pick_to_size");
    bool   pick_to_color = prefs->getBool(prefs_path + "pick_to_color");
    bool   pick_to_opacity = prefs->getBool(prefs_path + "pick_to_opacity");
    double rand_picked = 0.01 * prefs->getDoubleLimited(prefs_path + "rand_picked", 0, 0, 100);
    bool   invert_picked = prefs->getBool(prefs_path + "invert_picked");
    double gamma_picked = prefs->getDoubleLimited(prefs_path + "gamma_picked", 0, -10, 10);

    SPItem *item = dynamic_cast<SPItem *>(obj);
    if (dotrace) {
        clonetiler_trace_setup (desktop->getDocument(), 1.0, item);
    }

    Geom::Point center;
    double w = 0;
    double h = 0;
    double x0 = 0;
    double y0 = 0;

    if (keepbbox &&
        obj_repr->attribute("inkscape:tile-w") &&
        obj_repr->attribute("inkscape:tile-h") &&
        obj_repr->attribute("inkscape:tile-x0") &&
        obj_repr->attribute("inkscape:tile-y0") &&
        obj_repr->attribute("inkscape:tile-cx") &&
        obj_repr->attribute("inkscape:tile-cy")) {

        double cx = 0;
        double cy = 0;
        sp_repr_get_double (obj_repr, "inkscape:tile-cx", &cx);
        sp_repr_get_double (obj_repr, "inkscape:tile-cy", &cy);
        center = Geom::Point (cx, cy);

        sp_repr_get_double (obj_repr, "inkscape:tile-w", &w);
        sp_repr_get_double (obj_repr, "inkscape:tile-h", &h);
        sp_repr_get_double (obj_repr, "inkscape:tile-x0", &x0);
        sp_repr_get_double (obj_repr, "inkscape:tile-y0", &y0);
    } else {
        bool prefs_bbox = prefs->getBool("/tools/bounding_box", false);
        SPItem::BBoxType bbox_type = ( !prefs_bbox ?
            SPItem::VISUAL_BBOX : SPItem::GEOMETRIC_BBOX );
        Geom::OptRect r = item->documentBounds(bbox_type);
        if (r) {
            w = scale_units*r->dimensions()[Geom::X];
            h = scale_units*r->dimensions()[Geom::Y];
            x0 = scale_units*r->min()[Geom::X];
            y0 = scale_units*r->min()[Geom::Y];
            center = scale_units*desktop->dt2doc(item->getCenter());

            sp_repr_set_svg_double(obj_repr, "inkscape:tile-cx", center[Geom::X]);
            sp_repr_set_svg_double(obj_repr, "inkscape:tile-cy", center[Geom::Y]);
            sp_repr_set_svg_double(obj_repr, "inkscape:tile-w", w);
            sp_repr_set_svg_double(obj_repr, "inkscape:tile-h", h);
            sp_repr_set_svg_double(obj_repr, "inkscape:tile-x0", x0);
            sp_repr_set_svg_double(obj_repr, "inkscape:tile-y0", y0);
        } else {
            center = Geom::Point(0, 0);
            w = h = 0;
            x0 = y0 = 0;
        }
    }

    Geom::Point cur(0, 0);
    Geom::Rect bbox_original (Geom::Point (x0, y0), Geom::Point (x0 + w, y0 + h));
    double perimeter_original = (w + h)/4;

    // The integers i and j are reserved for tile column and row.
    // The doubles x and y are used for coordinates
    for (int i = 0;
         fillrect?
             (fabs(cur[Geom::X]) < fillwidth && i < 200) // prevent "freezing" with too large fillrect, arbitrarily limit rows
             : (i < imax);
         i ++) {
        for (int j = 0;
             fillrect?
                 (fabs(cur[Geom::Y]) < fillheight && j < 200) // prevent "freezing" with too large fillrect, arbitrarily limit cols
                 : (j < jmax);
             j ++) {

            // Note: We create a clone at 0,0 too, right over the original, in case our clones are colored

            // Get transform from symmetry, shift, scale, rotation
            Geom::Affine orig_t = clonetiler_get_transform (type, i, j, center[Geom::X], center[Geom::Y], w, h,
                                                       shiftx_per_i,     shifty_per_i,
                                                       shiftx_per_j,     shifty_per_j,
                                                       shiftx_rand,      shifty_rand,
                                                       shiftx_exp,       shifty_exp,
                                                       shiftx_alternate, shifty_alternate,
                                                       shiftx_cumulate,  shifty_cumulate,
                                                       shiftx_excludew,  shifty_excludeh,
                                                       scalex_per_i,     scaley_per_i,
                                                       scalex_per_j,     scaley_per_j,
                                                       scalex_rand,      scaley_rand,
                                                       scalex_exp,       scaley_exp,
                                                       scalex_log,       scaley_log,
                                                       scalex_alternate, scaley_alternate,
                                                       scalex_cumulate,  scaley_cumulate,
                                                       rotate_per_i,     rotate_per_j,
                                                       rotate_rand,
                                                       rotate_alternatei, rotate_alternatej,
                                                       rotate_cumulatei,  rotate_cumulatej      );
            Geom::Affine parent_transform = (((SPItem*)item->parent)->i2doc_affine())*(item->document->getRoot()->c2p.inverse());
            Geom::Affine t = parent_transform*orig_t*parent_transform.inverse();
            cur = center * t - center;
            if (fillrect) {
                if ((cur[Geom::X] > fillwidth) || (cur[Geom::Y] > fillheight)) { // off limits
                    continue;
                }
            }

            gchar color_string[32]; *color_string = 0;

            // Color tab
            if (!initial_color.empty()) {
                guint32 rgba = sp_svg_read_color (initial_color.data(), 0x000000ff);
                float hsl[3];
                sp_color_rgb_to_hsl_floatv (hsl, SP_RGBA32_R_F(rgba), SP_RGBA32_G_F(rgba), SP_RGBA32_B_F(rgba));

                double eff_i = (color_alternatei? (i%2) : (i));
                double eff_j = (color_alternatej? (j%2) : (j));

                hsl[0] += hue_per_i * eff_i + hue_per_j * eff_j + hue_rand * g_random_double_range (-1, 1);
                double notused;
                hsl[0] = modf( hsl[0], &notused ); // Restrict to 0-1
                hsl[1] += saturation_per_i * eff_i + saturation_per_j * eff_j + saturation_rand * g_random_double_range (-1, 1);
                hsl[1] = CLAMP (hsl[1], 0, 1);
                hsl[2] += lightness_per_i * eff_i + lightness_per_j * eff_j + lightness_rand * g_random_double_range (-1, 1);
                hsl[2] = CLAMP (hsl[2], 0, 1);

                float rgb[3];
                sp_color_hsl_to_rgb_floatv (rgb, hsl[0], hsl[1], hsl[2]);
                sp_svg_write_color(color_string, sizeof(color_string), SP_RGBA32_F_COMPOSE(rgb[0], rgb[1], rgb[2], 1.0));
            }

            // Blur
            double blur = 0.0;
            {
            int eff_i = (blur_alternatei? (i%2) : (i));
            int eff_j = (blur_alternatej? (j%2) : (j));
            blur =  (blur_per_i * eff_i + blur_per_j * eff_j + blur_rand * g_random_double_range (-1, 1));
            blur = CLAMP (blur, 0, 1);
            }

            // Opacity
            double opacity = 1.0;
            {
            int eff_i = (opacity_alternatei? (i%2) : (i));
            int eff_j = (opacity_alternatej? (j%2) : (j));
            opacity = 1 - (opacity_per_i * eff_i + opacity_per_j * eff_j + opacity_rand * g_random_double_range (-1, 1));
            opacity = CLAMP (opacity, 0, 1);
            }

            // Trace tab
            if (dotrace) {
                Geom::Rect bbox_t = transform_rect (bbox_original, t);

                guint32 rgba = clonetiler_trace_pick (bbox_t);
                float r = SP_RGBA32_R_F(rgba);
                float g = SP_RGBA32_G_F(rgba);
                float b = SP_RGBA32_B_F(rgba);
                float a = SP_RGBA32_A_F(rgba);

                float hsl[3];
                sp_color_rgb_to_hsl_floatv (hsl, r, g, b);

                gdouble val = 0;
                switch (pick) {
                case PICK_COLOR:
                    val = 1 - hsl[2]; // inverse lightness; to match other picks where black = max
                    break;
                case PICK_OPACITY:
                    val = a;
                    break;
                case PICK_R:
                    val = r;
                    break;
                case PICK_G:
                    val = g;
                    break;
                case PICK_B:
                    val = b;
                    break;
                case PICK_H:
                    val = hsl[0];
                    break;
                case PICK_S:
                    val = hsl[1];
                    break;
                case PICK_L:
                    val = 1 - hsl[2];
                    break;
                default:
                    break;
                }

                if (rand_picked > 0) {
                    val = randomize01 (val, rand_picked);
                    r = randomize01 (r, rand_picked);
                    g = randomize01 (g, rand_picked);
                    b = randomize01 (b, rand_picked);
                }

                if (gamma_picked != 0) {
                    double power;
                    if (gamma_picked > 0)
                        power = 1/(1 + fabs(gamma_picked));
                    else
                        power = 1 + fabs(gamma_picked);

                    val = pow (val, power);
                    r = pow (r, power);
                    g = pow (g, power);
                    b = pow (b, power);
                }

                if (invert_picked) {
                    val = 1 - val;
                    r = 1 - r;
                    g = 1 - g;
                    b = 1 - b;
                }

                val = CLAMP (val, 0, 1);
                r = CLAMP (r, 0, 1);
                g = CLAMP (g, 0, 1);
                b = CLAMP (b, 0, 1);

                // recompose tweaked color
                rgba = SP_RGBA32_F_COMPOSE(r, g, b, a);

                if (pick_to_presence) {
                    if (g_random_double_range (0, 1) > val) {
                        continue; // skip!
                    }
                }
                if (pick_to_size) {
                    t = parent_transform * Geom::Translate(-center[Geom::X], -center[Geom::Y]) 
                    * Geom::Scale (val, val) * Geom::Translate(center[Geom::X], center[Geom::Y]) 
                    * parent_transform.inverse() * t;
                }
                if (pick_to_opacity) {
                    opacity *= val;
                }
                if (pick_to_color) {
                    sp_svg_write_color(color_string, sizeof(color_string), rgba);
                }
            }

            if (opacity < 1e-6) { // invisibly transparent, skip
                continue;
            }

            if (fabs(t[0]) + fabs (t[1]) + fabs(t[2]) + fabs(t[3]) < 1e-6) { // too small, skip
                continue;
            }

            // Create the clone
            Inkscape::XML::Node *clone = obj_repr->document()->createElement("svg:use");
            clone->setAttribute("x", "0");
            clone->setAttribute("y", "0");
            clone->setAttribute("inkscape:tiled-clone-of", id_href);
            clone->setAttribute("xlink:href", id_href);

            Geom::Point new_center;
            bool center_set = false;
            if (obj_repr->attribute("inkscape:transform-center-x") || obj_repr->attribute("inkscape:transform-center-y")) {
                new_center = scale_units*desktop->dt2doc(item->getCenter()) * orig_t;
                center_set = true;
            }

            gchar *affinestr=sp_svg_transform_write(t);
            clone->setAttribute("transform", affinestr);
            g_free(affinestr);

            if (opacity < 1.0) {
                sp_repr_set_css_double(clone, "opacity", opacity);
            }

            if (*color_string) {
                clone->setAttribute("fill", color_string);
                clone->setAttribute("stroke", color_string);
            }

            // add the new clone to the top of the original's parent
            parent->getRepr()->appendChild(clone);

            if (blur > 0.0) {
                SPObject *clone_object = desktop->getDocument()->getObjectByRepr(clone);
                double perimeter = perimeter_original * t.descrim();
                double radius = blur * perimeter;
                // this is necessary for all newly added clones to have correct bboxes,
                // otherwise filters won't work:
                desktop->getDocument()->ensureUpToDate();
                // it's hard to figure out exact width/height of the tile without having an object
                // that we can take bbox of; however here we only need a lower bound so that blur
                // margins are not too small, and the perimeter should work
                SPFilter *constructed = new_filter_gaussian_blur(desktop->getDocument(), radius, t.descrim(), t.expansionX(), t.expansionY(), perimeter, perimeter);
                sp_style_set_property_url (clone_object, "filter", constructed, false);
            }

            if (center_set) {
                SPObject *clone_object = desktop->getDocument()->getObjectByRepr(clone);
                SPItem *item = dynamic_cast<SPItem *>(clone_object);
                if (clone_object && item) {
                    clone_object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
                    item->setCenter(desktop->doc2dt(new_center));
                    clone_object->updateRepr();
                }
            }

            Inkscape::GC::release(clone);
        }
        cur[Geom::Y] = 0;
    }

    if (dotrace) {
        clonetiler_trace_finish ();
    }

    clonetiler_change_selection (selection, dlg);

    desktop->clearWaitingCursor();

    DocumentUndo::done(desktop->getDocument(), SP_VERB_DIALOG_CLONETILER,
                       _("Create tiled clones"));
}

GtkWidget * CloneTiler::clonetiler_new_tab(GtkWidget *nb, const gchar *label)
{
    GtkWidget *l = gtk_label_new_with_mnemonic (label);
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *vb = gtk_box_new(GTK_ORIENTATION_VERTICAL, VB_MARGIN);
    gtk_box_set_homogeneous(GTK_BOX(vb), FALSE);
#else
    GtkWidget *vb = gtk_vbox_new (FALSE, VB_MARGIN);
#endif
    gtk_container_set_border_width (GTK_CONTAINER (vb), VB_MARGIN);
    gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);
    return vb;
}

void CloneTiler::clonetiler_checkbox_toggled(GtkToggleButton *tb, gpointer *data)
{
    const gchar *attr = (const gchar *) data;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool(prefs_path + attr, gtk_toggle_button_get_active(tb));
}

GtkWidget * CloneTiler::clonetiler_checkbox(const char *tip, const char *attr)
{
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, VB_MARGIN);
    gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
    GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);
#endif

    GtkWidget *b = gtk_check_button_new ();
    gtk_widget_set_tooltip_text (b, tip);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool value = prefs->getBool(prefs_path + attr);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(b), value);

    gtk_box_pack_end (GTK_BOX (hb), b, FALSE, TRUE, 0);
    g_signal_connect ( G_OBJECT (b), "clicked",
                         G_CALLBACK (clonetiler_checkbox_toggled), (gpointer) attr);

    g_object_set_data (G_OBJECT(b), "uncheckable", GINT_TO_POINTER(TRUE));

    return hb;
}

void CloneTiler::clonetiler_value_changed(GtkAdjustment *adj, gpointer data)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    const gchar *pref = (const gchar *) data;
    prefs->setDouble(prefs_path + pref, gtk_adjustment_get_value (adj));
}

GtkWidget * CloneTiler::clonetiler_spinbox(const char *tip, const char *attr, double lower, double upper, const gchar *suffix, bool exponent/* = false*/)
{
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
    GtkWidget *hb = gtk_hbox_new(FALSE, 0);
#endif

    {
#if WITH_GTKMM_3_0
        Glib::RefPtr<Gtk::Adjustment> a;
        if (exponent) {
            a = Gtk::Adjustment::create(1.0, lower, upper, 0.01, 0.05, 0);
        } else {
            a = Gtk::Adjustment::create(0.0, lower, upper, 0.1, 0.5, 0);
        }
#else
        Gtk::Adjustment *a;
        if (exponent) {
            a = new Gtk::Adjustment (1.0, lower, upper, 0.01, 0.05, 0);
        } else {
            a = new Gtk::Adjustment (0.0, lower, upper, 0.1, 0.5, 0);
        }
#endif

        Inkscape::UI::Widget::SpinButton *sb;
#if WITH_GTKMM_3_0
        if (exponent) {
            sb = new Inkscape::UI::Widget::SpinButton(a, 0.01, 2);
        } else {
            sb = new Inkscape::UI::Widget::SpinButton(a, 0.1, 1);
	}
#else
        if (exponent) {
            sb = new Inkscape::UI::Widget::SpinButton (*a, 0.01, 2);
        } else {
            sb = new Inkscape::UI::Widget::SpinButton (*a, 0.1, 1);
        }
#endif

        sb->set_tooltip_text (tip);
        sb->set_width_chars (5);
        sb->set_digits(3);
        gtk_box_pack_start (GTK_BOX (hb), GTK_WIDGET(sb->gobj()), FALSE, FALSE, SB_MARGIN);

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        double value = prefs->getDoubleLimited(prefs_path + attr, exponent? 1.0 : 0.0, lower, upper);
        a->set_value (value);
        // TODO: C++ification
        g_signal_connect(G_OBJECT(a->gobj()), "value_changed",
                           G_CALLBACK(clonetiler_value_changed), (gpointer) attr);

        if (exponent) {
            sb->set_data ("oneable", GINT_TO_POINTER(TRUE));
        } else {
            sb->set_data ("zeroable", GINT_TO_POINTER(TRUE));
        }
    }

    {
        GtkWidget *l = gtk_label_new ("");
        gtk_label_set_markup (GTK_LABEL(l), suffix);
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0);
        gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);
    }

    return hb;
}

void CloneTiler::clonetiler_symgroup_changed(GtkComboBox *cb, gpointer /*data*/)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gint group_new = gtk_combo_box_get_active (cb);
    prefs->setInt(prefs_path + "symmetrygroup", group_new);
}

void CloneTiler::clonetiler_xy_changed(GtkAdjustment *adj, gpointer data)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    const gchar *pref = (const gchar *) data;
    prefs->setInt(prefs_path + pref, (int) floor(gtk_adjustment_get_value (adj) + 0.5));
}

void CloneTiler::clonetiler_keep_bbox_toggled(GtkToggleButton *tb, gpointer /*data*/)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool(prefs_path + "keepbbox", gtk_toggle_button_get_active(tb));
}

void CloneTiler::clonetiler_pick_to(GtkToggleButton *tb, gpointer data)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    const gchar *pref = (const gchar *) data;
    prefs->setBool(prefs_path + pref, gtk_toggle_button_get_active(tb));
}


void CloneTiler::clonetiler_reset_recursive(GtkWidget *w)
{
    if (w && G_IS_OBJECT(w)) {
        {
            int r = GPOINTER_TO_INT (g_object_get_data(G_OBJECT(w), "zeroable"));
            if (r && GTK_IS_SPIN_BUTTON(w)) { // spinbutton
                GtkAdjustment *a = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON(w));
                gtk_adjustment_set_value (a, 0);
            }
        }
        {
            int r = GPOINTER_TO_INT (g_object_get_data(G_OBJECT(w), "oneable"));
            if (r && GTK_IS_SPIN_BUTTON(w)) { // spinbutton
                GtkAdjustment *a = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON(w));
                gtk_adjustment_set_value (a, 1);
            }
        }
        {
            int r = GPOINTER_TO_INT (g_object_get_data(G_OBJECT(w), "uncheckable"));
            if (r && GTK_IS_TOGGLE_BUTTON(w)) { // checkbox
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w), FALSE);
            }
        }
    }

    if (GTK_IS_CONTAINER(w)) {
        GList *ch = gtk_container_get_children (GTK_CONTAINER(w));
        for (GList *i = ch; i != NULL; i = i->next) {
            clonetiler_reset_recursive (GTK_WIDGET(i->data));
        }
        g_list_free (ch);
    }
}

void CloneTiler::clonetiler_reset(GtkWidget */*widget*/, GtkWidget *dlg)
{
    clonetiler_reset_recursive (dlg);
}

void CloneTiler::clonetiler_table_attach(GtkWidget *table, GtkWidget *widget, float align, int row, int col)
{
    GtkWidget *a = gtk_alignment_new (align, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(a), widget);

#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(table, GTK_ALIGN_FILL);
    gtk_widget_set_valign(table, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(table), a, col, row, 1, 1);
#else
    gtk_table_attach ( GTK_TABLE (table), a, col, col + 1, row, row + 1, GTK_FILL, (GtkAttachOptions)0, 0, 0 );
#endif
}

GtkWidget * CloneTiler::clonetiler_table_x_y_rand(int values)
{
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *table = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(table), 6);
    gtk_grid_set_column_spacing(GTK_GRID(table), 8);
#else
    GtkWidget *table = gtk_table_new (values + 2, 5, FALSE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 6);
    gtk_table_set_col_spacings (GTK_TABLE (table), 8);
#endif

    gtk_container_set_border_width (GTK_CONTAINER (table), VB_MARGIN);

    {
#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
        GtkWidget *hb = gtk_hbox_new (FALSE, 0);
#endif

        GtkWidget *i = sp_icon_new (Inkscape::ICON_SIZE_DECORATION, INKSCAPE_ICON("object-rows"));
        gtk_box_pack_start (GTK_BOX (hb), i, FALSE, FALSE, 2);

        GtkWidget *l = gtk_label_new ("");
        gtk_label_set_markup (GTK_LABEL(l), _("<small>Per row:</small>"));
        gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 2);

        clonetiler_table_attach (table, hb, 0, 1, 2);
    }

    {
#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
        GtkWidget *hb = gtk_hbox_new (FALSE, 0);
#endif

        GtkWidget *i = sp_icon_new (Inkscape::ICON_SIZE_DECORATION, INKSCAPE_ICON("object-columns"));
        gtk_box_pack_start (GTK_BOX (hb), i, FALSE, FALSE, 2);

        GtkWidget *l = gtk_label_new ("");
        gtk_label_set_markup (GTK_LABEL(l), _("<small>Per column:</small>"));
        gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 2);

        clonetiler_table_attach (table, hb, 0, 1, 3);
    }

    {
        GtkWidget *l = gtk_label_new ("");
        gtk_label_set_markup (GTK_LABEL(l), _("<small>Randomize:</small>"));
        clonetiler_table_attach (table, l, 0, 1, 4);
    }

    return table;
}

void CloneTiler::clonetiler_pick_switched(GtkToggleButton */*tb*/, gpointer data)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    guint v = GPOINTER_TO_INT (data);
    prefs->setInt(prefs_path + "pick", v);
}


void CloneTiler::clonetiler_switch_to_create(GtkToggleButton * /*tb*/, GtkWidget *dlg)
{
    GtkWidget *rowscols = GTK_WIDGET(g_object_get_data (G_OBJECT(dlg), "rowscols"));
    GtkWidget *widthheight = GTK_WIDGET(g_object_get_data (G_OBJECT(dlg), "widthheight"));

    if (rowscols) {
        gtk_widget_set_sensitive (rowscols, TRUE);
    }
    if (widthheight) {
        gtk_widget_set_sensitive (widthheight, FALSE);
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool(prefs_path + "fillrect", false);
}


void CloneTiler::clonetiler_switch_to_fill(GtkToggleButton * /*tb*/, GtkWidget *dlg)
{
    GtkWidget *rowscols = GTK_WIDGET(g_object_get_data (G_OBJECT(dlg), "rowscols"));
    GtkWidget *widthheight = GTK_WIDGET(g_object_get_data (G_OBJECT(dlg), "widthheight"));

    if (rowscols) {
        gtk_widget_set_sensitive (rowscols, FALSE);
    }
    if (widthheight) {
        gtk_widget_set_sensitive (widthheight, TRUE);
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool(prefs_path + "fillrect", true);
}




void CloneTiler::clonetiler_fill_width_changed(GtkAdjustment *adj, Inkscape::UI::Widget::UnitMenu *u)
{
    gdouble const raw_dist = gtk_adjustment_get_value (adj);
    Inkscape::Util::Unit const *unit = u->getUnit();
    gdouble const pixels = Inkscape::Util::Quantity::convert(raw_dist, unit, "px");

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble(prefs_path + "fillwidth", pixels);
}

void CloneTiler::clonetiler_fill_height_changed(GtkAdjustment *adj, Inkscape::UI::Widget::UnitMenu *u)
{
    gdouble const raw_dist = gtk_adjustment_get_value (adj);
    Inkscape::Util::Unit const *unit = u->getUnit();
    gdouble const pixels = Inkscape::Util::Quantity::convert(raw_dist, unit, "px");

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble(prefs_path + "fillheight", pixels);
}

void CloneTiler::clonetiler_unit_changed()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gdouble width_pixels = prefs->getDouble(prefs_path + "fillwidth");
    gdouble height_pixels = prefs->getDouble(prefs_path + "fillheight");
    
    Inkscape::Util::Unit const *unit = unit_menu->getUnit();
    
    gdouble width_value = Inkscape::Util::Quantity::convert(width_pixels, "px", unit);
    gdouble height_value = Inkscape::Util::Quantity::convert(height_pixels, "px", unit);
    gtk_adjustment_set_value(fill_width->gobj(), width_value);
    gtk_adjustment_set_value(fill_height->gobj(), height_value);
}

void CloneTiler::clonetiler_do_pick_toggled(GtkToggleButton *tb, GtkWidget *dlg)
{
    GtkWidget *vvb = GTK_WIDGET(g_object_get_data (G_OBJECT(dlg), "dotrace"));

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool(prefs_path + "dotrace", gtk_toggle_button_get_active (tb));

    if (vvb) {
        gtk_widget_set_sensitive (vvb, gtk_toggle_button_get_active (tb));
    }
}

}
}
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
