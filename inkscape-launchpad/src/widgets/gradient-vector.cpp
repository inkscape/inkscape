/*
 * Gradient vector selection widget
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   MenTaLguY <mental@rydia.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2004 Monash University
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2006 MenTaLguY
 * Copyright (C) 2010 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm.h>
#include "gradient-vector.h"
#include "ui/widget/color-preview.h"
#include "verbs.h"
#include <gtk/gtk.h>
#include "macros.h"
#include <glibmm/i18n.h>
#include <set>
#include "../widgets/gradient-image.h"
#include "../inkscape.h"
#include "../document-private.h"
#include "../gradient-chemistry.h"
#include "../helper/window.h"
#include "io/resource.h"

#include "xml/repr.h"

#include "../preferences.h"
#include "svg/css-ostringstream.h"
#include "sp-stop.h"
#include "selection-chemistry.h"
#include "style.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "desktop.h"
#include "layer-manager.h"

#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>
#include "document-undo.h"

#include "ui/dialog-events.h"
#include "ui/selected-color.h"
#include "ui/widget/color-notebook.h"

using Inkscape::DocumentUndo;
using Inkscape::UI::SelectedColor;

enum {
    VECTOR_SET,
    LAST_SIGNAL
};

#if GTK_CHECK_VERSION(3,0,0)
static void sp_gradient_vector_selector_destroy(GtkWidget *object);
#else
static void sp_gradient_vector_selector_destroy(GtkObject *object);
#endif

static void sp_gvs_gradient_release(SPObject *obj, SPGradientVectorSelector *gvs);
static void sp_gvs_defs_release(SPObject *defs, SPGradientVectorSelector *gvs);
static void sp_gvs_defs_modified(SPObject *defs, guint flags, SPGradientVectorSelector *gvs);

static void sp_gvs_rebuild_gui_full(SPGradientVectorSelector *gvs);
static SPStop *get_selected_stop( GtkWidget *vb);
void gr_get_usage_counts(SPDocument *doc, std::map<SPGradient *, gint> *mapUsageCount );
unsigned long sp_gradient_to_hhssll(SPGradient *gr);

static guint signals[LAST_SIGNAL] = {0};

// TODO FIXME kill these globals!!!
static GtkWidget *dlg = NULL;
static win_data wd;
static gint x = -1000, y = -1000, w = 0, h = 0; // impossible original values to make sure they are read from prefs
static Glib::ustring const prefs_path = "/dialogs/gradienteditor/";

#if GTK_CHECK_VERSION(3,0,0)
G_DEFINE_TYPE(SPGradientVectorSelector, sp_gradient_vector_selector, GTK_TYPE_BOX);
#else
G_DEFINE_TYPE(SPGradientVectorSelector, sp_gradient_vector_selector, GTK_TYPE_VBOX);
#endif

static void sp_gradient_vector_selector_class_init(SPGradientVectorSelectorClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    signals[VECTOR_SET] = g_signal_new( "vector_set",
                                        G_TYPE_FROM_CLASS(gobject_class),
                                        G_SIGNAL_RUN_LAST,
                                        G_STRUCT_OFFSET(SPGradientVectorSelectorClass, vector_set),
                                        NULL, NULL,
                                        g_cclosure_marshal_VOID__POINTER,
                                        G_TYPE_NONE, 1,
                                        G_TYPE_POINTER);

#if GTK_CHECK_VERSION(3,0,0)
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->destroy = sp_gradient_vector_selector_destroy;
#else
    GtkObjectClass *object_class = GTK_OBJECT_CLASS(klass);
    object_class->destroy = sp_gradient_vector_selector_destroy;
#endif
}

static void sp_gradient_vector_selector_init(SPGradientVectorSelector *gvs)
{
#if GTK_CHECK_VERSION(3,0,0)
    gtk_orientable_set_orientation(GTK_ORIENTABLE(gvs), GTK_ORIENTATION_VERTICAL);
#endif

    gvs->idlabel = TRUE;

    gvs->swatched = false;

    gvs->doc = NULL;
    gvs->gr = NULL;

    new (&gvs->gradient_release_connection) sigc::connection();
    new (&gvs->defs_release_connection) sigc::connection();
    new (&gvs->defs_modified_connection) sigc::connection();

    gvs->columns = new SPGradientSelector::ModelColumns();
    gvs->store = Gtk::ListStore::create(*gvs->columns);
    new (&gvs->tree_select_connection) sigc::connection();

}

#if GTK_CHECK_VERSION(3,0,0)
static void sp_gradient_vector_selector_destroy(GtkWidget *object)
#else
static void sp_gradient_vector_selector_destroy(GtkObject *object)
#endif
{
    SPGradientVectorSelector *gvs = SP_GRADIENT_VECTOR_SELECTOR(object);

    if (gvs->gr) {
        gvs->gradient_release_connection.disconnect();
        gvs->tree_select_connection.disconnect();
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
    gvs->tree_select_connection.~connection();

#if GTK_CHECK_VERSION(3,0,0)
    if ((GTK_WIDGET_CLASS(sp_gradient_vector_selector_parent_class))->destroy) {
        (GTK_WIDGET_CLASS(sp_gradient_vector_selector_parent_class))->destroy(object);
    }
#else
    if ((GTK_OBJECT_CLASS(sp_gradient_vector_selector_parent_class))->destroy) {
        (GTK_OBJECT_CLASS(sp_gradient_vector_selector_parent_class))->destroy(object);
    }
#endif
}

GtkWidget *sp_gradient_vector_selector_new(SPDocument *doc, SPGradient *gr)
{
    GtkWidget *gvs;

    g_return_val_if_fail(!gr || SP_IS_GRADIENT(gr), NULL);
    g_return_val_if_fail(!gr || (gr->document == doc), NULL);

    gvs = static_cast<GtkWidget*>(g_object_new(SP_TYPE_GRADIENT_VECTOR_SELECTOR, NULL));

    if (doc) {
        sp_gradient_vector_selector_set_gradient(SP_GRADIENT_VECTOR_SELECTOR(gvs), doc, gr);
    } else {
        sp_gvs_rebuild_gui_full(SP_GRADIENT_VECTOR_SELECTOR(gvs));
    }

    return gvs;
}

void sp_gradient_vector_selector_set_gradient(SPGradientVectorSelector *gvs, SPDocument *doc, SPGradient *gr)
{
//     g_message("sp_gradient_vector_selector_set_gradient(%p, %p, %p) [%s] %d %d", gvs, doc, gr,
//               (gr ? gr->getId():"N/A"),
//               (gr ? gr->isSwatch() : -1),
//               (gr ? gr->isSolid() : -1));
    static gboolean suppress = FALSE;

    g_return_if_fail(gvs != NULL);
    g_return_if_fail(SP_IS_GRADIENT_VECTOR_SELECTOR(gvs));
    g_return_if_fail(!gr || (doc != NULL));
    g_return_if_fail(!gr || SP_IS_GRADIENT(gr));
    g_return_if_fail(!gr || (gr->document == doc));
    g_return_if_fail(!gr || gr->hasStops());

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

        // Connect signals
        if (doc) {
            gvs->defs_release_connection = doc->getDefs()->connectRelease(sigc::bind<1>(sigc::ptr_fun(&sp_gvs_defs_release), gvs));
            gvs->defs_modified_connection = doc->getDefs()->connectModified(sigc::bind<2>(sigc::ptr_fun(&sp_gvs_defs_modified), gvs));
        }
        if (gr) {
            gvs->gradient_release_connection = gr->connectRelease(sigc::bind<1>(sigc::ptr_fun(&sp_gvs_gradient_release), gvs));
        }
        gvs->doc = doc;
        gvs->gr = gr;
        sp_gvs_rebuild_gui_full(gvs);
        if (!suppress) g_signal_emit(G_OBJECT(gvs), signals[VECTOR_SET], 0, gr);
    } else if (gr != gvs->gr) {
        // Harder case - keep document, rebuild list and stuff
        // fixme: (Lauris)
        suppress = TRUE;
        sp_gradient_vector_selector_set_gradient(gvs, NULL, NULL);
        sp_gradient_vector_selector_set_gradient(gvs, doc, gr);
        suppress = FALSE;
        g_signal_emit(G_OBJECT(gvs), signals[VECTOR_SET], 0, gr);
    }
    /* The case of setting NULL -> NULL is not very interesting */
}

SPDocument *sp_gradient_vector_selector_get_document(SPGradientVectorSelector *gvs)
{
    g_return_val_if_fail(gvs != NULL, NULL);
    g_return_val_if_fail(SP_IS_GRADIENT_VECTOR_SELECTOR(gvs), NULL);

    return gvs->doc;
}

SPGradient *sp_gradient_vector_selector_get_gradient(SPGradientVectorSelector *gvs)
{
    g_return_val_if_fail(gvs != NULL, NULL);
    g_return_val_if_fail(SP_IS_GRADIENT_VECTOR_SELECTOR(gvs), NULL);

    return gvs->gr;
}

Glib::ustring gr_prepare_label (SPObject *obj)
{
    const gchar *id = obj->label() ? obj->label() : obj->getId();
    if (!id) {
        id = obj->getRepr()->name();
    }

    if (strlen(id) > 14 && (!strncmp (id, "linearGradient", 14) || !strncmp (id, "radialGradient", 14)))
        return gr_ellipsize_text (g_strdup_printf ("%s", id+14), 35);
    return gr_ellipsize_text (id, 35);
}

/*
 * Ellipse text if longer than maxlen, "50% start text + ... + ~50% end text"
 * Text should be > length 8 or just return the original text
 */
Glib::ustring gr_ellipsize_text(Glib::ustring const &src, size_t maxlen)
{
    if (src.length() > maxlen && maxlen > 8) {
        size_t p1 = (size_t) maxlen / 2;
        size_t p2 = (size_t) src.length() - (maxlen - p1 - 1);
        return src.substr(0, p1) + "…" + src.substr(p2);
    }
    return src;
}

static void sp_gvs_rebuild_gui_full(SPGradientVectorSelector *gvs)
{

    gvs->tree_select_connection.block();

    /* Clear old list, if there is any */
    gvs->store->clear();

    /* Pick up all gradients with vectors */
    GSList *gl = NULL;
    if (gvs->gr) {
        std::vector<SPObject *> gradients = gvs->gr->document->getResourceList("gradient");
        for (std::vector<SPObject *>::const_iterator it = gradients.begin(); it != gradients.end(); ++it) {
            SPGradient* grad = SP_GRADIENT(*it);
            if ( grad->hasStops() && (grad->isSwatch() == gvs->swatched) ) {
                gl = g_slist_prepend(gl, *it);
            }
        }
    }
    gl = g_slist_reverse(gl);

    /* Get usage count of all the gradients */
    std::map<SPGradient *, gint> usageCount;
    gr_get_usage_counts(gvs->doc, &usageCount);

    if (!gvs->doc) {
        Gtk::TreeModel::Row row = *(gvs->store->append());
        row[gvs->columns->name] = _("No document selected");

    } else if (!gl) {
        Gtk::TreeModel::Row row = *(gvs->store->append());
        row[gvs->columns->name] = _("No gradients in document");

    } else if (!gvs->gr) {
        Gtk::TreeModel::Row row = *(gvs->store->append());
        row[gvs->columns->name] =  _("No gradient selected");

    } else {
        while (gl) {
            SPGradient *gr;
            gr = SP_GRADIENT(gl->data);
            gl = g_slist_remove(gl, gr);

            unsigned long hhssll = sp_gradient_to_hhssll(gr);
            GdkPixbuf *pixb = sp_gradient_to_pixbuf (gr, 64, 18);
            Glib::ustring label = gr_prepare_label(gr);

            Gtk::TreeModel::Row row = *(gvs->store->append());
            row[gvs->columns->name] = label.c_str();
            row[gvs->columns->color] = hhssll;
            row[gvs->columns->refcount] = usageCount[gr];
            row[gvs->columns->data] = gr;
            row[gvs->columns->pixbuf] = Glib::wrap(pixb);

        }
    }

    gvs->tree_select_connection.unblock();

}

/*
 *  Return a "HHSSLL" version of the first stop color so we can sort by it
 */
unsigned long sp_gradient_to_hhssll(SPGradient *gr)
{
    SPStop *stop = gr->getFirstStop();
    unsigned long rgba = stop->get_rgba32();
    float hsl[3];
    sp_color_rgb_to_hsl_floatv (hsl, SP_RGBA32_R_F(rgba), SP_RGBA32_G_F(rgba), SP_RGBA32_B_F(rgba));

    return ((int)(hsl[0]*100 * 10000)) + ((int)(hsl[1]*100 * 100)) + ((int)(hsl[2]*100 * 1));
}

static GSList *get_all_doc_items(GSList *list, SPObject *from, bool onlyvisible, bool onlysensitive, bool ingroups, GSList const *exclude)
{
    for ( SPObject *child = from->firstChild() ; child; child = child->getNext() ) {
        if (SP_IS_ITEM(child)) {
            list = g_slist_prepend(list, SP_ITEM(child));
        }

        if (ingroups || SP_IS_ITEM(child)) {
            list = get_all_doc_items(list, child, onlyvisible, onlysensitive, ingroups, exclude);
        }
    }

    return list;
}

/*
 * Return a SPItem's gradient
 */
static SPGradient * gr_item_get_gradient(SPItem *item, gboolean fillorstroke)
{
    SPIPaint *item_paint = (fillorstroke) ? &(item->style->fill) : &(item->style->stroke);
    if (item_paint->isPaintserver()) {

        SPPaintServer *item_server = (fillorstroke) ?
                item->style->getFillPaintServer() : item->style->getStrokePaintServer();

        if (SP_IS_LINEARGRADIENT(item_server) || SP_IS_RADIALGRADIENT(item_server) ||
                (SP_IS_GRADIENT(item_server) && SP_GRADIENT(item_server)->getVector()->isSwatch()))  {

            return SP_GRADIENT(item_server)->getVector();
        }
    }

    return NULL;
}

/*
 * Map each gradient to its usage count for both fill and stroke styles
 */
void gr_get_usage_counts(SPDocument *doc, std::map<SPGradient *, gint> *mapUsageCount )
{
    if (!doc)
        return;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool onlyvisible = prefs->getBool("/options/kbselection/onlyvisible", true);
    bool onlysensitive = prefs->getBool("/options/kbselection/onlysensitive", true);
    bool ingroups = TRUE;

    GSList *all_list = get_all_doc_items(NULL, doc->getRoot(), onlyvisible, onlysensitive, ingroups, NULL);

    for (GSList *i = all_list; i != NULL; i = i->next) {
        SPItem *item = SP_ITEM(i->data);
        if (!item->getId())
            continue;
        SPGradient *gr = NULL;
        gr = gr_item_get_gradient(item, true); // fill
        if (gr) {
            mapUsageCount->count(gr) > 0 ? (*mapUsageCount)[gr] += 1 : (*mapUsageCount)[gr] = 1;
        }
        gr = gr_item_get_gradient(item, false); // stroke
        if (gr) {
            mapUsageCount->count(gr) > 0 ? (*mapUsageCount)[gr] += 1 : (*mapUsageCount)[gr] = 1;
        }
    }
}

static void sp_gvs_gradient_release(SPObject */*obj*/, SPGradientVectorSelector *gvs)
{
    /* Disconnect gradient */
    if (gvs->gr) {
        gvs->gradient_release_connection.disconnect();
        gvs->gr = NULL;
    }

    /* Rebuild GUI */
    sp_gvs_rebuild_gui_full(gvs);
}

static void sp_gvs_defs_release(SPObject */*defs*/, SPGradientVectorSelector *gvs)
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
    sp_gvs_rebuild_gui_full(gvs);
}

static void sp_gvs_defs_modified(SPObject */*defs*/, guint /*flags*/, SPGradientVectorSelector *gvs)
{
    /* fixme: We probably have to check some flags here (Lauris) */
    sp_gvs_rebuild_gui_full(gvs);
}

void SPGradientVectorSelector::setSwatched()
{
    swatched = true;
    sp_gvs_rebuild_gui_full(this);
}

/*##################################################################
  ###                 Vector Editing Widget
  ##################################################################*/

#include "widgets/widget-sizes.h"
#include "xml/node-event-vector.h"
#include "svg/svg-color.h"
#include "ui/widget/color-notebook.h"

#define PAD 4

static GtkWidget *sp_gradient_vector_widget_new(SPGradient *gradient, SPStop *stop);

static void sp_gradient_vector_widget_load_gradient(GtkWidget *widget, SPGradient *gradient);
static gint sp_gradient_vector_dialog_delete(GtkWidget *widget, GdkEvent *event, GtkWidget *dialog);

#if GTK_CHECK_VERSION(3,0,0)
static void sp_gradient_vector_dialog_destroy(GtkWidget *object, gpointer data);
static void sp_gradient_vector_widget_destroy(GtkWidget *object, gpointer data);
#else
static void sp_gradient_vector_dialog_destroy(GtkObject *object, gpointer data);
static void sp_gradient_vector_widget_destroy(GtkObject *object, gpointer data);
#endif

static void sp_gradient_vector_gradient_release(SPObject *obj, GtkWidget *widget);
static void sp_gradient_vector_gradient_modified(SPObject *obj, guint flags, GtkWidget *widget);
static void sp_gradient_vector_color_dragged(Inkscape::UI::SelectedColor *selected_color, GObject *object);
static void sp_gradient_vector_color_changed(Inkscape::UI::SelectedColor *selected_color, GObject *object);
static void update_stop_list( GtkWidget *vb, SPGradient *gradient, SPStop *new_stop);

static gboolean blocked = FALSE;

static void grad_edit_dia_stop_added_or_removed(Inkscape::XML::Node */*repr*/, Inkscape::XML::Node */*child*/, Inkscape::XML::Node */*ref*/, gpointer data)
{
    GtkWidget *vb = GTK_WIDGET(data);
    SPGradient *gradient = static_cast<SPGradient *>(g_object_get_data(G_OBJECT(vb), "gradient"));
    update_stop_list(vb, gradient, NULL);
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

static void verify_grad(SPGradient *gradient)
{
    int i = 0;
    SPStop *stop = NULL;
    /* count stops */
    for ( SPObject *ochild = gradient->firstChild() ; ochild ; ochild = ochild->getNext() ) {
        if (SP_IS_STOP(ochild)) {
            i++;
            stop = SP_STOP(ochild);
        }
    }

    Inkscape::XML::Document *xml_doc;
    xml_doc = gradient->getRepr()->document();

    if (i < 1) {
        Inkscape::CSSOStringStream os;
        os << "stop-color: #000000;stop-opacity:" << 1.0 << ";";

        Inkscape::XML::Node *child;

        child = xml_doc->createElement("svg:stop");
        sp_repr_set_css_double(child, "offset", 0.0);
        child->setAttribute("style", os.str().c_str());
        gradient->getRepr()->addChild(child, NULL);
        Inkscape::GC::release(child);

        child = xml_doc->createElement("svg:stop");
        sp_repr_set_css_double(child, "offset", 1.0);
        child->setAttribute("style", os.str().c_str());
        gradient->getRepr()->addChild(child, NULL);
        Inkscape::GC::release(child);
        return;
    }
    if (i < 2) {
        sp_repr_set_css_double(stop->getRepr(), "offset", 0.0);
        Inkscape::XML::Node *child = stop->getRepr()->duplicate(gradient->getRepr()->document());
        sp_repr_set_css_double(child, "offset", 1.0);
        gradient->getRepr()->addChild(child, stop->getRepr());
        Inkscape::GC::release(child);
    }
}

static void select_stop_in_list( GtkWidget *vb, SPGradient *gradient, SPStop *new_stop)
{
    GtkWidget *combo_box = static_cast<GtkWidget *>(g_object_get_data(G_OBJECT(vb), "combo_box"));

    int i = 0;
    for ( SPObject *ochild = gradient->firstChild() ; ochild ; ochild = ochild->getNext() ) {
        if (SP_IS_STOP(ochild)) {
            if (ochild == new_stop) {
                gtk_combo_box_set_active (GTK_COMBO_BOX(combo_box) , i);
                break;
            }
            i++;
        }
    }
}

static void update_stop_list( GtkWidget *vb, SPGradient *gradient, SPStop *new_stop)
{

    if (!SP_IS_GRADIENT(gradient)) {
        return;
    }

    blocked = TRUE;

    /* Clear old list, if there is any */
    GtkWidget *combo_box = static_cast<GtkWidget *>(g_object_get_data(G_OBJECT(vb), "combo_box"));
    if (!combo_box) {
        return;
    }
    GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(combo_box)));
    if (!store) {
        return;
    }
    gtk_list_store_clear(store);
    GtkTreeIter iter;

    /* Populate the combobox store */
    GSList *sl = NULL;
    if ( gradient->hasStops() ) {
        for ( SPObject *ochild = gradient->firstChild() ; ochild ; ochild = ochild->getNext() ) {
            if (SP_IS_STOP(ochild)) {
                sl = g_slist_append(sl, ochild);
            }
        }
    }
    if (!sl) {
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, NULL, 1, _("No stops in gradient"), 2, NULL, -1);
        gtk_widget_set_sensitive (combo_box, FALSE);

    } else {

        for (; sl != NULL; sl = sl->next){
            if (SP_IS_STOP(sl->data)){
                SPStop *stop = SP_STOP(sl->data);
                Inkscape::XML::Node *repr = reinterpret_cast<SPItem *>(sl->data)->getRepr();
                Inkscape::UI::Widget::ColorPreview *cpv = Gtk::manage(new Inkscape::UI::Widget::ColorPreview(stop->get_rgba32()));
                GdkPixbuf *pb = cpv->toPixbuf(64, 16);

                gtk_list_store_append (store, &iter);
                gtk_list_store_set (store, &iter, 0, pb, 1, repr->attribute("id"), 2, stop, -1);
                gtk_widget_set_sensitive (combo_box, FALSE);
            }
        }

        gtk_widget_set_sensitive(combo_box, TRUE);
    }

    /* Set history */
    if (new_stop == NULL) {
        gtk_combo_box_set_active (GTK_COMBO_BOX(combo_box) , 0);
    } else {
        select_stop_in_list(vb, gradient, new_stop);
    }

    blocked = FALSE;
}


// user selected existing stop from list
static void sp_grad_edit_combo_box_changed (GtkComboBox * /*widget*/, GtkWidget *tbl)
{
    SPStop *stop = get_selected_stop(tbl);
    if (!stop) {
        return;
    }

    blocked = TRUE;

    SelectedColor *csel = static_cast<SelectedColor*>(g_object_get_data(G_OBJECT(tbl), "cselector"));
    // set its color, from the stored array
    g_object_set_data(G_OBJECT(tbl), "updating_color", reinterpret_cast<void*>(1));
    csel->setColorAlpha(stop->getEffectiveColor(), stop->opacity);
    g_object_set_data(G_OBJECT(tbl), "updating_color", reinterpret_cast<void*>(0));
    GtkWidget *offspin = GTK_WIDGET(g_object_get_data(G_OBJECT(tbl), "offspn"));
    GtkWidget *offslide =GTK_WIDGET(g_object_get_data(G_OBJECT(tbl), "offslide"));

    GtkAdjustment *adj = static_cast<GtkAdjustment*>(g_object_get_data(G_OBJECT(tbl), "offset"));

    bool isEndStop = false;

    SPStop *prev = NULL;
    prev = stop->getPrevStop();
    if (prev != NULL )  {
        gtk_adjustment_set_lower (adj, prev->offset);
    } else {
        isEndStop = true;
        gtk_adjustment_set_lower (adj, 0);
    }

    SPStop *next = NULL;
    next = stop->getNextStop();
    if (next != NULL ) {
        gtk_adjustment_set_upper (adj, next->offset);
    } else {
        isEndStop = true;
        gtk_adjustment_set_upper (adj, 1.0);
    }

    //fixme: does this work on all possible input gradients?
    if (!isEndStop) {
        gtk_widget_set_sensitive(offslide, TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(offspin), TRUE);
    } else {
        gtk_widget_set_sensitive(offslide, FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(offspin), FALSE);
    }

    gtk_adjustment_set_value(adj, stop->offset);

    gtk_adjustment_changed(adj);

    blocked = FALSE;
}

static SPStop *get_selected_stop( GtkWidget *vb)
{
    SPStop *stop = NULL;
    GtkWidget *combo_box = static_cast<GtkWidget *>(g_object_get_data(G_OBJECT(vb), "combo_box"));
    if (combo_box) {
        GtkTreeIter  iter;
        if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX(combo_box), &iter)) {
            GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(combo_box)));
            gtk_tree_model_get (GTK_TREE_MODEL(store), &iter, 2, &stop, -1);
        }
    }
    return stop;
}

static void offadjustmentChanged( GtkAdjustment *adjustment, GtkWidget *vb)
{
    if (!blocked) {
        blocked = TRUE;

        SPStop *stop = get_selected_stop(vb);
        if (stop) {
            stop->offset = gtk_adjustment_get_value (adjustment);
            sp_repr_set_css_double(stop->getRepr(), "offset", stop->offset);

            DocumentUndo::maybeDone(stop->document, "gradient:stop:offset", SP_VERB_CONTEXT_GRADIENT,
                                    _("Change gradient stop offset"));

        }

        blocked = FALSE;
    }
}

guint32 sp_average_color(guint32 c1, guint32 c2, gdouble p/* = 0.5*/)
{
    guint32 r = (guint32) (SP_RGBA32_R_U(c1) * p + SP_RGBA32_R_U(c2) * (1 - p));
    guint32 g = (guint32) (SP_RGBA32_G_U(c1) * p + SP_RGBA32_G_U(c2) * (1 - p));
    guint32 b = (guint32) (SP_RGBA32_B_U(c1) * p + SP_RGBA32_B_U(c2) * (1 - p));
    guint32 a = (guint32) (SP_RGBA32_A_U(c1) * p + SP_RGBA32_A_U(c2) * (1 - p));

    return SP_RGBA32_U_COMPOSE(r, g, b, a);
}


static void sp_grd_ed_add_stop(GtkWidget */*widget*/,  GtkWidget *vb)
{
    SPGradient *gradient = static_cast<SPGradient *>(g_object_get_data(G_OBJECT(vb), "gradient"));
    verify_grad(gradient);

    SPStop *stop = get_selected_stop(vb);
    if (!stop) {
        return;
    }

    Inkscape::XML::Node *new_stop_repr = NULL;

    SPStop *next = stop->getNextStop();

    if (next == NULL) {
        SPStop *prev = stop->getPrevStop();
        if (prev != NULL) {
            next = stop;
            stop = prev;
        }
    }

    if (next != NULL) {
        new_stop_repr = stop->getRepr()->duplicate(gradient->getRepr()->document());
        gradient->getRepr()->addChild(new_stop_repr, stop->getRepr());
    } else {
        next = stop;
        new_stop_repr = stop->getPrevStop()->getRepr()->duplicate(gradient->getRepr()->document());
        gradient->getRepr()->addChild(new_stop_repr, stop->getPrevStop()->getRepr());
    }

    SPStop *newstop = reinterpret_cast<SPStop *>(gradient->document->getObjectByRepr(new_stop_repr));

    newstop->offset = (stop->offset + next->offset) * 0.5 ;

    guint32 const c1 = stop->get_rgba32();
    guint32 const c2 = next->get_rgba32();
    guint32 cnew = sp_average_color(c1, c2);

    Inkscape::CSSOStringStream os;
    gchar c[64];
    sp_svg_write_color(c, sizeof(c), cnew);
    gdouble opacity = static_cast<gdouble>(SP_RGBA32_A_F(cnew));
    os << "stop-color:" << c << ";stop-opacity:" << opacity <<";";
    newstop->getRepr()->setAttribute("style", os.str().c_str());
    sp_repr_set_css_double( newstop->getRepr(), "offset", (double)newstop->offset);

    sp_gradient_vector_widget_load_gradient(vb, gradient);
    Inkscape::GC::release(new_stop_repr);
    update_stop_list(GTK_WIDGET(vb), gradient, newstop);
    GtkWidget *offspin = GTK_WIDGET(g_object_get_data(G_OBJECT(vb), "offspn"));
    GtkWidget *offslide =GTK_WIDGET(g_object_get_data(G_OBJECT(vb), "offslide"));
    gtk_widget_set_sensitive(offslide, TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(offspin), TRUE);
    DocumentUndo::done(gradient->document, SP_VERB_CONTEXT_GRADIENT,
                       _("Add gradient stop"));
}

static void sp_grd_ed_del_stop(GtkWidget */*widget*/,  GtkWidget *vb)
{
    SPGradient *gradient = static_cast<SPGradient *>(g_object_get_data(G_OBJECT(vb), "gradient"));

    SPStop *stop = get_selected_stop(vb);
    if (!stop) {
        return;
    }

    if (gradient->vector.stops.size() > 2) { // 2 is the minimum

        // if we delete first or last stop, move the next/previous to the edge
        if (stop->offset == 0) {
            SPStop *next = stop->getNextStop();
            if (next) {
                next->offset = 0;
                sp_repr_set_css_double(next->getRepr(), "offset", 0);
            }
        } else if (stop->offset == 1) {
            SPStop *prev = stop->getPrevStop();
            if (prev) {
                prev->offset = 1;
                sp_repr_set_css_double(prev->getRepr(), "offset", 1);
            }
        }

        gradient->getRepr()->removeChild(stop->getRepr());
        sp_gradient_vector_widget_load_gradient(vb, gradient);
        update_stop_list(GTK_WIDGET(vb), gradient, NULL);
        DocumentUndo::done(gradient->document, SP_VERB_CONTEXT_GRADIENT,
                           _("Delete gradient stop"));
    }

}

static GtkWidget * sp_gradient_vector_widget_new(SPGradient *gradient, SPStop *select_stop)
{
    using Inkscape::UI::Widget::ColorNotebook;

    GtkWidget *vb, *w, *f;

    g_return_val_if_fail(gradient != NULL, NULL);
    g_return_val_if_fail(SP_IS_GRADIENT(gradient), NULL);

#if GTK_CHECK_VERSION(3,0,0)
    vb = gtk_box_new(GTK_ORIENTATION_VERTICAL, PAD);
    gtk_box_set_homogeneous(GTK_BOX(vb), FALSE);
#else
    vb = gtk_vbox_new(FALSE, PAD);
#endif
    g_signal_connect(G_OBJECT(vb), "destroy", G_CALLBACK(sp_gradient_vector_widget_destroy), NULL);

    w = sp_gradient_image_new(gradient);
    g_object_set_data(G_OBJECT(vb), "preview", w);
    gtk_widget_show(w);
    gtk_box_pack_start(GTK_BOX(vb), w, TRUE, TRUE, PAD);

    sp_repr_add_listener(gradient->getRepr(), &grad_edit_dia_repr_events, vb);

    /* ComboBox of stops with 3 columns,
     * The color preview, the label and a pointer to the SPStop
     */
    GtkListStore *store = gtk_list_store_new (3, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_POINTER);
    GtkWidget *combo_box = gtk_combo_box_new_with_model (GTK_TREE_MODEL (store));

    GtkCellRenderer *renderer = gtk_cell_renderer_pixbuf_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), renderer, FALSE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), renderer, "pixbuf", 0,  NULL);
    gtk_cell_renderer_set_padding(renderer, 5, 0);

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), renderer, "text", 1, NULL);
    gtk_widget_show(combo_box);
    gtk_box_pack_start(GTK_BOX(vb), combo_box, FALSE, FALSE, 0);
    g_object_set_data(G_OBJECT(vb), "combo_box", combo_box);

    update_stop_list(GTK_WIDGET(vb), gradient, NULL);

    g_signal_connect(G_OBJECT(combo_box), "changed", G_CALLBACK(sp_grad_edit_combo_box_changed), vb);

    /* Add and Remove buttons */
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
    gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
    GtkWidget *hb = gtk_hbox_new(FALSE, 1);
#endif
    // TRANSLATORS: "Stop" means: a "phase" of a gradient
    GtkWidget *b = gtk_button_new_with_label(_("Add stop"));
    gtk_widget_show(b);
    gtk_container_add(GTK_CONTAINER(hb), b);
    gtk_widget_set_tooltip_text(b, _("Add another control stop to gradient"));
    g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK(sp_grd_ed_add_stop), vb);
    b = gtk_button_new_with_label(_("Delete stop"));
    gtk_widget_show(b);
    gtk_container_add(GTK_CONTAINER(hb), b);
    gtk_widget_set_tooltip_text(b, _("Delete current control stop from gradient"));
    g_signal_connect(G_OBJECT(b), "clicked", G_CALLBACK(sp_grd_ed_del_stop), vb);

    gtk_widget_show(hb);
    gtk_box_pack_start(GTK_BOX(vb),hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);

    /*  Offset Slider and stuff   */
#if GTK_CHECK_VERSION(3,0,0)
    hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_set_homogeneous(GTK_BOX(hb), FALSE);
#else
    hb = gtk_hbox_new(FALSE, 0);
#endif

    /* Label */
    GtkWidget *l = gtk_label_new(C_("Gradient","Offset:"));

#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(l, GTK_ALIGN_END);
#else
    gtk_misc_set_alignment(GTK_MISC(l), 1.0, 0.5);
#endif

    gtk_box_pack_start(GTK_BOX(hb),l, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
    gtk_widget_show(l);

    /* Adjustment */
    GtkAdjustment *Offset_adj = NULL;
    Offset_adj= GTK_ADJUSTMENT(gtk_adjustment_new(0.0, 0.0, 1.0, 0.01, 0.01, 0.0));
    g_object_set_data(G_OBJECT(vb), "offset", Offset_adj);

    SPStop *stop = get_selected_stop(vb);
    if (!stop) {
        return NULL;
    }

    gtk_adjustment_set_value(Offset_adj, stop->offset);

    /* Slider */
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *slider = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, Offset_adj);
#else
    GtkWidget *slider = gtk_hscale_new(Offset_adj);
#endif
    gtk_scale_set_draw_value( GTK_SCALE(slider), FALSE );
    gtk_widget_show(slider);
    gtk_box_pack_start(GTK_BOX(hb),slider, TRUE, TRUE, AUX_BETWEEN_BUTTON_GROUPS);
    g_object_set_data(G_OBJECT(vb), "offslide", slider);

    /* Spinbutton */
    GtkWidget *sbtn = gtk_spin_button_new(GTK_ADJUSTMENT(Offset_adj), 0.01, 2);
    sp_dialog_defocus_on_enter(sbtn);
    gtk_widget_show(sbtn);
    gtk_box_pack_start(GTK_BOX(hb),sbtn, FALSE, TRUE, AUX_BETWEEN_BUTTON_GROUPS);
    g_object_set_data(G_OBJECT(vb), "offspn", sbtn);

    if (stop->offset>0 && stop->offset<1) {
        gtk_widget_set_sensitive(slider, TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(sbtn), TRUE);
    } else {
        gtk_widget_set_sensitive(slider, FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(sbtn), FALSE);
    }


    /* Signals */
    g_signal_connect(G_OBJECT(Offset_adj), "value_changed",
                        G_CALLBACK(offadjustmentChanged), vb);

    // g_signal_connect(G_OBJECT(slider), "changed",  G_CALLBACK(offsliderChanged), vb);
    gtk_widget_show(hb);
    gtk_box_pack_start(GTK_BOX(vb), hb, FALSE, FALSE, PAD);

    // TRANSLATORS: "Stop" means: a "phase" of a gradient
    f = gtk_frame_new(_("Stop Color"));
    gtk_widget_show(f);
    gtk_box_pack_start(GTK_BOX(vb), f, TRUE, TRUE, PAD);

    Inkscape::UI::SelectedColor *selected_color = new Inkscape::UI::SelectedColor;
    g_object_set_data(G_OBJECT(vb), "cselector", selected_color);
    g_object_set_data(G_OBJECT(vb), "updating_color", reinterpret_cast<void*>(0));
    selected_color->signal_dragged.connect(sigc::bind(sigc::ptr_fun(&sp_gradient_vector_color_dragged), selected_color, G_OBJECT(vb)));
    selected_color->signal_dragged.connect(sigc::bind(sigc::ptr_fun(&sp_gradient_vector_color_changed), selected_color, G_OBJECT(vb)));

    Gtk::Widget *color_selector = Gtk::manage(new ColorNotebook(*selected_color));
    color_selector->show();
    gtk_container_add(GTK_CONTAINER(f), color_selector->gobj());

    /*
    gtk_widget_show(csel);
    gtk_container_add(GTK_CONTAINER(f), csel);
    g_signal_connect(G_OBJECT(csel), "dragged", G_CALLBACK(sp_gradient_vector_color_dragged), vb);
    g_signal_connect(G_OBJECT(csel), "changed", G_CALLBACK(sp_gradient_vector_color_changed), vb);
    */

    gtk_widget_show(vb);

    sp_gradient_vector_widget_load_gradient(vb, gradient);

    if (select_stop) {
        select_stop_in_list(GTK_WIDGET(vb), gradient, select_stop);
    }

    return vb;
}



GtkWidget * sp_gradient_vector_editor_new(SPGradient *gradient, SPStop *stop)
{
    if (dlg == NULL) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();

        dlg = sp_window_new(_("Gradient editor"), TRUE);
        if (x == -1000 || y == -1000) {
            x = prefs->getInt(prefs_path + "x", -1000);
            y = prefs->getInt(prefs_path + "y", -1000);
        }
        if (w ==0 || h == 0) {
            w = prefs->getInt(prefs_path + "w", 0);
            h = prefs->getInt(prefs_path + "h", 0);
        }

        if (x<0) {
            x=0;
        }
        if (y<0) {
            y=0;
        }

        if (x != 0 || y != 0) {
            gtk_window_move(reinterpret_cast<GtkWindow *>(dlg), x, y);
        } else {
            gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
        }
        if (w && h) {
            gtk_window_resize(reinterpret_cast<GtkWindow *>(dlg), w, h);
        }
        sp_transientize(dlg);
        wd.win = dlg;
        wd.stop = 0;

        GObject *obj = G_OBJECT(dlg);
        sigc::connection *conn = NULL;

        conn = new sigc::connection(INKSCAPE.signal_activate_desktop.connect(sigc::bind(sigc::ptr_fun(&sp_transientize_callback), &wd)));
        g_object_set_data(obj, "desktop-activate-connection", conn);

        g_signal_connect(obj, "event", G_CALLBACK(sp_dialog_event_handler), dlg);
        g_signal_connect(obj, "destroy", G_CALLBACK(sp_gradient_vector_dialog_destroy), dlg);
        g_signal_connect(obj, "delete_event", G_CALLBACK(sp_gradient_vector_dialog_delete), dlg);

        conn = new sigc::connection(INKSCAPE.signal_shut_down.connect(
            sigc::hide_return(
            sigc::bind(sigc::ptr_fun(&sp_gradient_vector_dialog_delete), (GtkWidget *) NULL, (GdkEvent *) NULL, (GtkWidget *) NULL)
        )));
        g_object_set_data(obj, "shutdown-connection", conn);

        conn = new sigc::connection(INKSCAPE.signal_dialogs_hide.connect(sigc::bind(sigc::ptr_fun(&gtk_widget_hide), dlg)));
        g_object_set_data(obj, "dialog-hide-connection", conn);

        conn = new sigc::connection(INKSCAPE.signal_dialogs_unhide.connect(sigc::bind(sigc::ptr_fun(&gtk_widget_show), dlg)));
        g_object_set_data(obj, "dialog-unhide-connection", conn);

        gtk_container_set_border_width(GTK_CONTAINER(dlg), PAD);

        GtkWidget *wid = static_cast<GtkWidget*>(sp_gradient_vector_widget_new(gradient, stop));
        g_object_set_data(G_OBJECT(dlg), "gradient-vector-widget", wid);
        /* Connect signals */
        gtk_widget_show(wid);
        gtk_container_add(GTK_CONTAINER(dlg), wid);
    } else {
        // FIXME: temp fix for 0.38
        // Simply load_gradient into the editor does not work for multi-stop gradients,
        // as the stop list and other widgets are in a wrong state and crash readily.
        // Instead we just delete the window (by sending the delete signal)
        // and call sp_gradient_vector_editor_new again, so it creates the window anew.

        GdkEventAny event;
        GtkWidget *widget = static_cast<GtkWidget *>(dlg);
        event.type = GDK_DELETE;
        event.window = gtk_widget_get_window (widget);
        event.send_event = TRUE;
        g_object_ref(G_OBJECT(event.window));
        gtk_main_do_event(reinterpret_cast<GdkEvent*>(&event));
        g_object_unref(G_OBJECT(event.window));

        g_assert(dlg == NULL);
        sp_gradient_vector_editor_new(gradient, stop);
    }

    return dlg;
}

static void sp_gradient_vector_widget_load_gradient(GtkWidget *widget, SPGradient *gradient)
{
    blocked = TRUE;

    SPGradient *old;

    old = static_cast<SPGradient*>(g_object_get_data(G_OBJECT(widget), "gradient"));

    if (old != gradient) {
        sigc::connection *release_connection;
        sigc::connection *modified_connection;

        release_connection = static_cast<sigc::connection *>(g_object_get_data(G_OBJECT(widget), "gradient_release_connection"));
        modified_connection = static_cast<sigc::connection *>(g_object_get_data(G_OBJECT(widget), "gradient_modified_connection"));

        if (old) {
            g_assert( release_connection != NULL );
            g_assert( modified_connection != NULL );
            release_connection->disconnect();
            modified_connection->disconnect();
            sp_signal_disconnect_by_data(old, widget);
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

    g_object_set_data(G_OBJECT(widget), "gradient", gradient);

    if (gradient) {
        gtk_widget_set_sensitive(widget, TRUE);

        gradient->ensureVector();

        SPStop *stop = get_selected_stop(widget);
        if (!stop) {
            return;
        }

        // get the color selector
        SelectedColor *csel =  static_cast<SelectedColor*>(g_object_get_data(G_OBJECT(widget), "cselector"));

        g_object_set_data(G_OBJECT(widget), "updating_color", reinterpret_cast<void*>(1));
        csel->setColorAlpha(stop->getEffectiveColor(), stop->opacity);
        g_object_set_data(G_OBJECT(widget), "updating_color", reinterpret_cast<void*>(0));

        /* Fill preview */
        GtkWidget *w = static_cast<GtkWidget *>(g_object_get_data(G_OBJECT(widget), "preview"));
        sp_gradient_image_set_gradient(SP_GRADIENT_IMAGE(w), gradient);

        update_stop_list(GTK_WIDGET(widget), gradient, NULL);

        // Once the user edits a gradient, it stops being auto-collectable
        if (gradient->getRepr()->attribute("inkscape:collect")) {
            SPDocument *document = gradient->document;
            bool saved = DocumentUndo::getUndoSensitive(document);
            DocumentUndo::setUndoSensitive(document, false);
            gradient->getRepr()->setAttribute("inkscape:collect", NULL);
            DocumentUndo::setUndoSensitive(document, saved);
        }
    } else { // no gradient, disable everything
        gtk_widget_set_sensitive(widget, FALSE);
    }

    blocked = FALSE;
}

#if GTK_CHECK_VERSION(3,0,0)
static void sp_gradient_vector_dialog_destroy(GtkWidget * /*object*/, gpointer /*data*/)
#else
static void sp_gradient_vector_dialog_destroy(GtkObject * /*object*/, gpointer /*data*/)
#endif
{
    GObject *obj = G_OBJECT(dlg);
    assert(obj != NULL);

    sigc::connection *conn = static_cast<sigc::connection *>(g_object_get_data(obj, "desktop-activate-connection"));
    assert(conn != NULL);
    conn->disconnect();
    delete conn;

    conn = static_cast<sigc::connection *>(g_object_get_data(obj, "shutdown-connection"));
    assert(conn != NULL);
    conn->disconnect();
    delete conn;

    conn = static_cast<sigc::connection *>(g_object_get_data(obj, "dialog-hide-connection"));
    assert(conn != NULL);
    conn->disconnect();
    delete conn;

    conn = static_cast<sigc::connection *>(g_object_get_data(obj, "dialog-unhide-connection"));
    assert(conn != NULL);
    conn->disconnect();
    delete conn;

    wd.win = dlg = NULL;
    wd.stop = 0;
}

static gboolean sp_gradient_vector_dialog_delete(GtkWidget */*widget*/, GdkEvent */*event*/, GtkWidget */*dialog*/)
{
    gtk_window_get_position(GTK_WINDOW(dlg), &x, &y);
    gtk_window_get_size(GTK_WINDOW(dlg), &w, &h);

    if (x<0) {
        x=0;
    }
    if (y<0) {
        y=0;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt(prefs_path + "x", x);
    prefs->setInt(prefs_path + "y", y);
    prefs->setInt(prefs_path + "w", w);
    prefs->setInt(prefs_path + "h", h);

    return FALSE; // which means, go ahead and destroy it
}

/* Widget destroy handler */
#if GTK_CHECK_VERSION(3,0,0)
static void sp_gradient_vector_widget_destroy(GtkWidget *object, gpointer /*data*/)
#else
static void sp_gradient_vector_widget_destroy(GtkObject *object, gpointer /*data*/)
#endif
{
    SPObject *gradient = SP_OBJECT(g_object_get_data(G_OBJECT(object), "gradient"));

    sigc::connection *release_connection = static_cast<sigc::connection *>(g_object_get_data(G_OBJECT(object), "gradient_release_connection"));
    sigc::connection *modified_connection = static_cast<sigc::connection *>(g_object_get_data(G_OBJECT(object), "gradient_modified_connection"));

    if (gradient) {
        g_assert( release_connection != NULL );
        g_assert( modified_connection != NULL );
        release_connection->disconnect();
        modified_connection->disconnect();
        sp_signal_disconnect_by_data(gradient, object);

        if (gradient->getRepr()) {
            sp_repr_remove_listener_by_data(gradient->getRepr(), object);
        }
    }

    SelectedColor *selected_color = static_cast<SelectedColor *>(g_object_get_data(G_OBJECT(object), "cselector"));
    if (selected_color) {
        delete selected_color;
        g_object_set_data(G_OBJECT(object), "cselector", NULL);
    }
}

static void sp_gradient_vector_gradient_release(SPObject */*object*/, GtkWidget *widget)
{
    sp_gradient_vector_widget_load_gradient(widget, NULL);
}

static void sp_gradient_vector_gradient_modified(SPObject *object, guint /*flags*/, GtkWidget *widget)
{
    SPGradient *gradient=SP_GRADIENT(object);
    if (!blocked) {
        blocked = TRUE;
        sp_gradient_vector_widget_load_gradient(widget, gradient);
        blocked = FALSE;
    }
}

static void sp_gradient_vector_color_dragged(Inkscape::UI::SelectedColor *selected_color, GObject *object)
{
    SPGradient *gradient, *ngr;

    if (blocked) {
        return;
    }

    gradient = static_cast<SPGradient*>(g_object_get_data(G_OBJECT(object), "gradient"));
    if (!gradient) {
        return;
    }

    blocked = TRUE;

    ngr = sp_gradient_ensure_vector_normalized(gradient);
    if (ngr != gradient) {
        /* Our master gradient has changed */
        sp_gradient_vector_widget_load_gradient(GTK_WIDGET(object), ngr);
    }

    ngr->ensureVector();

    SPStop *stop = get_selected_stop(GTK_WIDGET(object));
    if (!stop) {
        return;
    }

    selected_color->colorAlpha(stop->specified_color, stop->opacity);
    stop->currentColor = false;

    blocked = FALSE;
}

static void sp_gradient_vector_color_changed(Inkscape::UI::SelectedColor *selected_color, GObject *object)
{
    (void)selected_color;

    void* updating_color = g_object_get_data(G_OBJECT(object), "updating_color");
    if (updating_color) {
        return;
    }

    if (blocked) {
        return;
    }

    SPGradient *gradient = static_cast<SPGradient*>(g_object_get_data(G_OBJECT(object), "gradient"));
    if (!gradient) {
        return;
    }

    blocked = TRUE;

    SPGradient *ngr = sp_gradient_ensure_vector_normalized(gradient);
    if (ngr != gradient) {
        /* Our master gradient has changed */
        sp_gradient_vector_widget_load_gradient(GTK_WIDGET(object), ngr);
    }

    ngr->ensureVector();

    /* Set start parameters */
    /* We rely on normalized vector, i.e. stops HAVE to exist */
    g_return_if_fail(ngr->getFirstStop() != NULL);

    SPStop *stop = get_selected_stop(GTK_WIDGET(object));
    if (!stop) {
        return;
    }

    SelectedColor *csel = static_cast<SelectedColor *>(g_object_get_data(G_OBJECT(object), "cselector"));
    SPColor color;
    float alpha = 0;
    csel->colorAlpha(color, alpha);

    sp_repr_set_css_double(stop->getRepr(), "offset", stop->offset);
    Inkscape::CSSOStringStream os;
    os << "stop-color:" << color.toString() << ";stop-opacity:" << static_cast<gdouble>(alpha) <<";";
    stop->getRepr()->setAttribute("style", os.str().c_str());
    // g_snprintf(c, 256, "stop-color:#%06x;stop-opacity:%g;", rgb >> 8, static_cast<gdouble>(alpha));
    //stop->getRepr()->setAttribute("style", c);

    DocumentUndo::done(ngr->document, SP_VERB_CONTEXT_GRADIENT,
                       _("Change gradient stop color"));

    blocked = FALSE;

    // Set the color in the selected stop after change
    GtkWidget *combo_box = static_cast<GtkWidget *>(g_object_get_data(G_OBJECT(object), "combo_box"));
    if (combo_box) {
        GtkTreeIter  iter;
        if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX(combo_box), &iter)) {
            GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(combo_box)));

            Inkscape::UI::Widget::ColorPreview *cp = Gtk::manage(new Inkscape::UI::Widget::ColorPreview(stop->get_rgba32()));
            GdkPixbuf *pb = cp->toPixbuf(64, 16);

            gtk_list_store_set (store, &iter, 0, pb, /*1, repr->attribute("id"),*/ 2, stop, -1);
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
