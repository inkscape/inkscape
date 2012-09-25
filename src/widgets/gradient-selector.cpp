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
#include "inkscape.h"
#include "verbs.h"
#include "helper/action.h"
#include "preferences.h"

#include <glibmm/i18n.h>
#include <xml/repr.h>

#include "gradient-vector.h"
#include "gradient-selector.h"
#include "paint-selector.h"
#include "style.h"
#include "id-clash.h"

enum {
    GRABBED,
    DRAGGED,
    RELEASED,
    CHANGED,
    LAST_SIGNAL
};


static void sp_gradient_selector_class_init (SPGradientSelectorClass *klass);
static void sp_gradient_selector_init (SPGradientSelector *selector);
static void sp_gradient_selector_dispose(GObject *object);

/* Signal handlers */
static void sp_gradient_selector_vector_set (SPGradientVectorSelector *gvs, SPGradient *gr, SPGradientSelector *sel);
static void sp_gradient_selector_edit_vector_clicked (GtkWidget *w, SPGradientSelector *sel);
static void sp_gradient_selector_add_vector_clicked (GtkWidget *w, SPGradientSelector *sel);

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

static void sp_gradient_selector_class_init(SPGradientSelectorClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;

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

    object_class->dispose = sp_gradient_selector_dispose;
}

static void sp_gradient_selector_init(SPGradientSelector *sel)
{
    sel->safelyInit = true;
    sel->blocked = false;
    new (&sel->nonsolid) std::vector<GtkWidget*>();

    sel->mode = SPGradientSelector::MODE_LINEAR;

    sel->gradientUnits = SP_GRADIENT_UNITS_USERSPACEONUSE;
    sel->gradientSpread = SP_GRADIENT_SPREAD_PAD;

    /* Vectors */
    sel->vectors = sp_gradient_vector_selector_new (NULL, NULL);
    SPGradientVectorSelector *gvs = SP_GRADIENT_VECTOR_SELECTOR(sel->vectors);
    sel->store = gvs->store;
    sel->columns = gvs->columns;

    sel->treeview = Gtk::manage(new Gtk::TreeView());
    sel->treeview->set_model(gvs->store);
    sel->treeview->set_headers_clickable (true);
    sel->treeview->set_search_column(1);
    sel->icon_renderer = Gtk::manage(new Gtk::CellRendererPixbuf());
    sel->text_renderer = Gtk::manage(new Gtk::CellRendererText());

    sel->treeview->append_column("Gradient", *sel->icon_renderer);
    Gtk::TreeView::Column* icon_column = sel->treeview->get_column(0);
    icon_column->add_attribute(sel->icon_renderer->property_pixbuf(), sel->columns->pixbuf);
    icon_column->set_sort_column(sel->columns->color);
    icon_column->set_clickable(true);

    sel->treeview->append_column("Name", *sel->text_renderer);
    Gtk::TreeView::Column* name_column = sel->treeview->get_column(1);
    sel->text_renderer->property_editable() = true;
    name_column->add_attribute(sel->text_renderer->property_text(), sel->columns->name);
    name_column->set_min_width(180);
    name_column->set_clickable(true);
    name_column->set_resizable(true);

    sel->treeview->append_column("#", sel->columns->refcount);
    Gtk::TreeView::Column* count_column = sel->treeview->get_column(2);
    count_column->set_clickable(true);
    count_column->set_resizable(true);

    sel->treeview->show();

    icon_column->signal_clicked().connect( sigc::mem_fun(*sel, &SPGradientSelector::onTreeColorColClick) );
    name_column->signal_clicked().connect( sigc::mem_fun(*sel, &SPGradientSelector::onTreeNameColClick) );
    count_column->signal_clicked().connect( sigc::mem_fun(*sel, &SPGradientSelector::onTreeCountColClick) );

    gvs->tree_select_connection = sel->treeview->get_selection()->signal_changed().connect( sigc::mem_fun(*sel, &SPGradientSelector::onTreeSelection) );
    sel->text_renderer->signal_edited().connect( sigc::mem_fun(*sel, &SPGradientSelector::onTreeEdited) );

    sel->scrolled_window = Gtk::manage(new Gtk::ScrolledWindow());
    sel->scrolled_window->add(*sel->treeview);
    sel->scrolled_window->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    sel->scrolled_window->set_shadow_type(Gtk::SHADOW_IN);
    sel->scrolled_window->set_size_request(0, 150);
    sel->scrolled_window->show();

    gtk_box_pack_start (GTK_BOX (sel), GTK_WIDGET(sel->scrolled_window->gobj()), TRUE, TRUE, 4);


    /* Create box for buttons */
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
    GtkWidget *hb = gtk_hbox_new( FALSE, 2 );
#endif
    sel->nonsolid.push_back(hb);
    gtk_box_pack_start( GTK_BOX(sel), hb, FALSE, FALSE, 0 );

    sel->add = gtk_button_new ();
    gtk_button_set_image((GtkButton*)sel->add , gtk_image_new_from_stock ( GTK_STOCK_ADD, GTK_ICON_SIZE_SMALL_TOOLBAR ) );

    sel->nonsolid.push_back(sel->add);
    gtk_box_pack_start (GTK_BOX (hb), sel->add, FALSE, FALSE, 0);

    g_signal_connect (G_OBJECT (sel->add), "clicked", G_CALLBACK (sp_gradient_selector_add_vector_clicked), sel);
    gtk_widget_set_sensitive (sel->add, FALSE);
    gtk_button_set_relief(GTK_BUTTON(sel->add), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text( sel->add, _("Create a duplicate gradient"));

    sel->edit = gtk_button_new ();
    gtk_button_set_image((GtkButton*)sel->edit , gtk_image_new_from_stock ( GTK_STOCK_EDIT, GTK_ICON_SIZE_SMALL_TOOLBAR ) );

    sel->nonsolid.push_back(sel->edit);
    gtk_box_pack_start (GTK_BOX (hb), sel->edit, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (sel->edit), "clicked", G_CALLBACK (sp_gradient_selector_edit_vector_clicked), sel);
    gtk_widget_set_sensitive (sel->edit, FALSE);
    gtk_button_set_relief(GTK_BUTTON(sel->edit), GTK_RELIEF_NONE);
    gtk_widget_set_tooltip_text( sel->edit, _("Edit gradient"));

    gtk_widget_show_all(hb);


}

static void sp_gradient_selector_dispose(GObject *object)
{
    SPGradientSelector *sel = SP_GRADIENT_SELECTOR( object );

    if ( sel->safelyInit ) {
        sel->safelyInit = false;
        using std::vector;
        sel->nonsolid.~vector<GtkWidget*>();
    }

    if (sel->icon_renderer) {
        delete sel->icon_renderer;
        sel->icon_renderer = NULL;
    }
    if (sel->text_renderer) {
        delete sel->text_renderer;
        sel->text_renderer = NULL;
    }

    if (((GObjectClass *) (parent_class))->dispose) {
        (* ((GObjectClass *) (parent_class))->dispose) (object);
    }
}

void SPGradientSelector::setSpread(SPGradientSpread spread)
{
    gradientSpread = spread;
    //gtk_combo_box_set_active (GTK_COMBO_BOX(this->spread), gradientSpread);
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

SPGradientUnits SPGradientSelector::getUnits()
{
    return gradientUnits;
}

SPGradientSpread SPGradientSelector::getSpread()
{
    return gradientSpread;
}

void SPGradientSelector::onTreeEdited( const Glib::ustring& path_string, const Glib::ustring& new_text)
{
    Gtk::TreePath path(path_string);
    Gtk::TreeModel::iterator iter = store->get_iter(path);

    if( iter )
    {
        Gtk::TreeModel::Row row = *iter;
        if ( row ) {
            SPObject* obj = row[columns->data];
            if ( obj ) {
                if (!new_text.empty() && new_text != row[columns->name]) {
                  rename_id(obj, new_text );
                }
                row[columns->name] = gr_prepare_label(obj);
            }
        }
    }
}

void SPGradientSelector::onTreeColorColClick() {
    Gtk::TreeView::Column* column = treeview->get_column(0);
    column->set_sort_column(columns->color);
}

void SPGradientSelector::onTreeNameColClick() {
    Gtk::TreeView::Column* column = treeview->get_column(1);
    column->set_sort_column(columns->name);
}


void SPGradientSelector::onTreeCountColClick() {
    Gtk::TreeView::Column* column = treeview->get_column(2);
    column->set_sort_column(columns->refcount);
}


void SPGradientSelector::onTreeSelection()
{
    if (!treeview) {
        return;
    }

    if (blocked) {
        return;
    }

    const Glib::RefPtr<Gtk::TreeSelection> sel = treeview->get_selection();
    if (!sel) {
        return;
    }

    SPGradient *obj = NULL;
    /* Single selection */
    Gtk::TreeModel::iterator iter = sel->get_selected();
    if ( iter ) {
        Gtk::TreeModel::Row row = *iter;
        obj = row[columns->data];
    }

    if (obj) {
        sp_gradient_selector_vector_set (NULL, (SPGradient*)obj, this);
    }
}

bool SPGradientSelector::_checkForSelected(const Gtk::TreePath &path, const Gtk::TreeIter& iter, SPGradient *vector)
{
    bool found = false;

    Gtk::TreeModel::Row row = *iter;
    if ( vector == row[columns->data] )
    {
        treeview->scroll_to_row(path, 0.5);
        Glib::RefPtr<Gtk::TreeSelection> select = treeview->get_selection();
        select->select(iter);
        found = true;
    }

    return found;
}

void SPGradientSelector::selectGradientInTree(SPGradient *vector)
{
    store->foreach( sigc::bind<SPGradient*>(sigc::mem_fun(*this, &SPGradientSelector::_checkForSelected), vector) );
}

void SPGradientSelector::setVector(SPDocument *doc, SPGradient *vector)
{
    g_return_if_fail(!vector || SP_IS_GRADIENT(vector));
    g_return_if_fail(!vector || (vector->document == doc));

    if (vector && !vector->hasStops()) {
        return;
    }

    sp_gradient_vector_selector_set_gradient(SP_GRADIENT_VECTOR_SELECTOR(vectors), doc, vector);

    selectGradientInTree(vector);

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
sp_gradient_selector_vector_set (SPGradientVectorSelector *gvs, SPGradient *gr, SPGradientSelector *sel)
{

    if (!sel->blocked) {
        sel->blocked = TRUE;
        gr = sp_gradient_ensure_vector_normalized (gr);
        sel->setVector((gr) ? gr->document : 0, gr);
        g_signal_emit (G_OBJECT (sel), signals[CHANGED], 0, gr);
        sel->blocked = FALSE;

    }
}

static void
sp_gradient_selector_edit_vector_clicked (GtkWidget */*w*/, SPGradientSelector *sel)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/dialogs/gradienteditor/showlegacy", false)) {
        // Legacy gradient dialog
        GtkWidget *dialog;
        dialog = sp_gradient_vector_editor_new (SP_GRADIENT_VECTOR_SELECTOR (sel->vectors)->gr);
        gtk_widget_show (dialog);
    } else {
        // Invoke the gradient tool
        Inkscape::Verb *verb = Inkscape::Verb::get( SP_VERB_CONTEXT_GRADIENT );
        if ( verb ) {
            SPAction *action = verb->get_action( ( Inkscape::UI::View::View * ) SP_ACTIVE_DESKTOP);
            if ( action ) {
                sp_action_perform( action, NULL );
            }
        }
    }
}

static void
sp_gradient_selector_add_vector_clicked (GtkWidget */*w*/, SPGradientSelector *sel)
{
    SPDocument *doc = sp_gradient_vector_selector_get_document (SP_GRADIENT_VECTOR_SELECTOR (sel->vectors));

    if (!doc)
        return;

    SPGradient *gr = sp_gradient_vector_selector_get_gradient( SP_GRADIENT_VECTOR_SELECTOR (sel->vectors));
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

    Glib::ustring old_id = gr->getId();

    gr = (SPGradient *) doc->getObjectByRepr(repr);

    // Rename the new gradients id to be similar to the cloned gradients
    rename_id(gr, old_id);

    sp_gradient_vector_selector_set_gradient( SP_GRADIENT_VECTOR_SELECTOR (sel->vectors), doc, gr);

    sel->selectGradientInTree(gr);

    Inkscape::GC::release(repr);
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
