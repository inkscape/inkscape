/**
 * @file
 * Combobox for selecting dash patterns - implementation.
 */
/* Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "stroke-marker-selector.h"

#include <cstring>
#include <string>
#include <glibmm/i18n.h>
#include <2geom/coord.h>


#include "style.h"
#include "ui/dialog-events.h"

#include "desktop-style.h"
#include "preferences.h"
#include "path-prefix.h"
#include "io/sys.h"
#include "sp-marker.h"
#include "sp-defs.h"
#include "sp-root.h"
#include "ui/cache/svg_preview_cache.h"
#include "helper/stock-items.h"
#include "gradient-vector.h"

#include <gtkmm/icontheme.h>
#include <gtkmm/adjustment.h>
#include "ui/widget/spinbutton.h"
#include "stroke-style.h"
#include "gradient-chemistry.h"

static Inkscape::UI::Cache::SvgPreview svg_preview_cache;

MarkerComboBox::MarkerComboBox(gchar const *id, int l) :
            Gtk::ComboBox(),
            combo_id(id),
            loc(l),
            updating(false),
            markerCount(0)
{

    marker_store = Gtk::ListStore::create(marker_columns);
    set_model(marker_store);
    pack_start(image_renderer, false);
    set_cell_data_func(image_renderer, sigc::mem_fun(*this, &MarkerComboBox::prepareImageRenderer));
    gtk_combo_box_set_row_separator_func(GTK_COMBO_BOX(gobj()), MarkerComboBox::separator_cb, NULL, NULL);

    empty_image = new Gtk::Image( Glib::wrap(
        sp_pixbuf_new( Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_ICON("no-marker") ) ) );

    sandbox = ink_markers_preview_doc ();
    desktop = SP_ACTIVE_DESKTOP;
    doc = desktop->getDocument();

    modified_connection = doc->getDefs()->connectModified( sigc::hide(sigc::hide(sigc::bind(sigc::ptr_fun(&MarkerComboBox::handleDefsModified), this))) );

    init_combo();

    show();
}

MarkerComboBox::~MarkerComboBox() {
    delete combo_id;
    delete sandbox;

    if (doc) {
        modified_connection.disconnect();
    }
}

void MarkerComboBox::setDesktop(SPDesktop *desktop)
{
    if (this->desktop != desktop) {

        if (doc) {
            modified_connection.disconnect();
        }

        this->desktop = desktop;
        doc = desktop->getDocument();

        if (doc) {
            modified_connection = doc->getDefs()->connectModified( sigc::hide(sigc::hide(sigc::bind(sigc::ptr_fun(&MarkerComboBox::handleDefsModified), this))) );
        }

        refreshHistory();
    }
}

void
MarkerComboBox::handleDefsModified(MarkerComboBox *self)
{
    self->refreshHistory();
}

void
MarkerComboBox::refreshHistory()
{
    if (updating)
        return;

    updating = true;

    GSList *ml = get_marker_list(doc);

    /*
     * Seems to be no way to get notified of changes just to markers,
     * so listen to changes in all defs and check if the number of markers has changed here
     * to avoid unnecessary refreshes when things like gradients change
    */
    if (markerCount != g_slist_length(ml)) {
        const char *active = get_active()->get_value(marker_columns.marker);
        sp_marker_list_from_doc(doc, true);
        set_selected(active);
        markerCount = g_slist_length(ml);
    }

    g_slist_free (ml);

    updating = false;
}

/**
 * Init the combobox widget to display markers from markers.svg
 */
void
MarkerComboBox::init_combo()
{
    if (updating)
        return;

    const gchar *active = NULL;
    if (get_active()) {
        active = get_active()->get_value(marker_columns.marker);
    }

    if (!doc) {
        Gtk::TreeModel::Row row = *(marker_store->append());
        row[marker_columns.label] = _("No document selected");
        row[marker_columns.marker] = g_strdup("None");
        row[marker_columns.image] = NULL;
        row[marker_columns.stock] = false;
        row[marker_columns.history] = false;
        row[marker_columns.separator] = false;
        set_sensitive(false);
        set_current(NULL);
        return;
    }

    static SPDocument *markers_doc = NULL;

    // add separator
    Gtk::TreeModel::Row row_sep = *(marker_store->append());
    row_sep[marker_columns.label] = "Separator";
    row_sep[marker_columns.marker] = g_strdup("None");
    row_sep[marker_columns.image] = NULL;
    row_sep[marker_columns.stock] = false;
    row_sep[marker_columns.history] = false;
    row_sep[marker_columns.separator] = true;

    // load markers from the current doc
    sp_marker_list_from_doc(doc, true);

    // find and load markers.svg
    if (markers_doc == NULL) {
        char *markers_source = g_build_filename(INKSCAPE_MARKERSDIR, "markers.svg", NULL);
        if (Inkscape::IO::file_test(markers_source, G_FILE_TEST_IS_REGULAR)) {
            markers_doc = SPDocument::createNewDoc(markers_source, FALSE);
        }
        g_free(markers_source);
    }

    // load markers from markers.svg
    if (markers_doc) {
        doc->ensureUpToDate();
        sp_marker_list_from_doc(markers_doc, false);
    }

    set_sensitive(true);

    /* Set history */
    set_selected(active);

}

/**
 * Sets the current marker in the marker combobox.
 */
void MarkerComboBox::set_current(SPObject *marker)
{
    updating = true;

    if (marker != NULL) {
        gchar *markname = g_strdup(marker->getRepr()->attribute("id"));
        set_selected(markname);
        g_free (markname);
    }
    else {
        set_selected(NULL);
    }

    updating = false;

}
/**
 * Return a uri string representing the current selected marker used for setting the marker style in the document
 */
const gchar * MarkerComboBox::get_active_marker_uri()
{
    /* Get Marker */
    const gchar *markid = get_active()->get_value(marker_columns.marker);
    if (!markid)
    {
        return NULL;
    }

    gchar const *marker = "";
    if (strcmp(markid, "none")) {
        bool stockid = get_active()->get_value(marker_columns.stock);

        gchar *markurn;
        if (stockid)
        {
            markurn = g_strconcat("urn:inkscape:marker:",markid,NULL);
        }
        else
        {
            markurn = g_strdup(markid);
        }
        SPObject *mark = get_stock_item(markurn, stockid);
        g_free(markurn);
        if (mark) {
            Inkscape::XML::Node *repr = mark->getRepr();
            marker = g_strconcat("url(#", repr->attribute("id"), ")", NULL);
        }
    } else {
        marker = g_strdup(markid);
    }

    return marker;
}

void MarkerComboBox::set_active_history() {

    const gchar *markid = get_active()->get_value(marker_columns.marker);

    // If forked from a stockid, add the stockid
    SPObject const *marker = doc->getObjectById(markid);
    if (marker && marker->getRepr()->attribute("inkscape:stockid")) {
        markid = marker->getRepr()->attribute("inkscape:stockid");
    }

    set_selected(markid);
}


void MarkerComboBox::set_selected(const gchar *name, gboolean retry/*=true*/) {

    if (!name) {
        set_active(0);
        return;
    }

    for(Gtk::TreeIter iter = marker_store->children().begin();
        iter != marker_store->children().end(); ++iter) {
            Gtk::TreeModel::Row row = (*iter);
            if (row[marker_columns.marker] &&
                    !strcmp(row[marker_columns.marker], name)) {
                set_active(iter);
                return;
            }
    }

    // Didn't find it in the list, try refreshing from the doc
    if (retry) {
        sp_marker_list_from_doc(doc, true);
        set_selected(name, false);
    }
}


/**
 * Pick up all markers from source, except those that are in
 * current_doc (if non-NULL), and add items to the combo.
 */
void MarkerComboBox::sp_marker_list_from_doc(SPDocument *source, gboolean history)
{
    GSList *ml = get_marker_list(source);
    GSList *clean_ml = NULL;

    for (; ml != NULL; ml = ml->next) {
        if (!SP_IS_MARKER(ml->data))
            continue;

        // Add to the list of markers we really do wish to show
        clean_ml = g_slist_prepend (clean_ml, ml->data);
    }

    remove_markers(history); // Seem to need to remove 2x
    remove_markers(history);
    add_markers(clean_ml, source, history);

    g_slist_free (ml);
    g_slist_free (clean_ml);

}

/**
 *  Returns a list of markers in the defs of the given source document as a GSList object
 *  Returns NULL if there are no markers in the document.
 */
GSList *MarkerComboBox::get_marker_list (SPDocument *source)
{
    if (source == NULL)
        return NULL;

    GSList *ml   = NULL;
    SPDefs *defs = source->getDefs();
    if (!defs) {
        return NULL;
    }

    for ( SPObject *child = defs->firstChild(); child; child = child->getNext() )
    {
        if (SP_IS_MARKER(child)) {
            ml = g_slist_prepend (ml, child);
        }
    }
    return ml;
}

/**
 * Remove history or non-history markers from the combo
 */
void MarkerComboBox::remove_markers (gboolean history)
{
    // Having the model set causes assertions when erasing rows, temporarily disconnect
    unset_model();
    for(Gtk::TreeIter iter = marker_store->children().begin();
        iter != marker_store->children().end(); ++iter) {
            Gtk::TreeModel::Row row = (*iter);
            if (row[marker_columns.history] == history && row[marker_columns.separator] == false) {
                marker_store->erase(iter);
                iter = marker_store->children().begin();
            }
    }

    set_model(marker_store);
}

/**
 * Adds markers in marker_list to the combo
 */
void MarkerComboBox::add_markers (GSList *marker_list, SPDocument *source, gboolean history)
{
    // Do this here, outside of loop, to speed up preview generation:
    Inkscape::Drawing drawing;
    unsigned const visionkey = SPItem::display_key_new(1);
    drawing.setRoot(sandbox->getRoot()->invoke_show(drawing, visionkey, SP_ITEM_SHOW_DISPLAY));
    // Find the separator,
    Gtk::TreeIter sep_iter;
    for(Gtk::TreeIter iter = marker_store->children().begin();
        iter != marker_store->children().end(); ++iter) {
            Gtk::TreeModel::Row row = (*iter);
            if (row[marker_columns.separator]) {
                sep_iter = iter;
            }
    }

    if (history) {
        // add "None"
        Gtk::TreeModel::Row row = *(marker_store->prepend());
        row[marker_columns.label] = C_("Marker", "None");
        row[marker_columns.stock] = false;
        row[marker_columns.marker] = g_strdup("None");
        row[marker_columns.image] = NULL;
        row[marker_columns.history] = true;
        row[marker_columns.separator] = false;
    }

    for (; marker_list != NULL; marker_list = marker_list->next) {

        Inkscape::XML::Node *repr = reinterpret_cast<SPItem *>(marker_list->data)->getRepr();
        gchar const *markid = repr->attribute("inkscape:stockid") ? repr->attribute("inkscape:stockid") : repr->attribute("id");

        // generate preview
        Gtk::Image *prv = create_marker_image (22, repr->attribute("id"), source, drawing, visionkey);
        prv->show();

        // Add history before separator, others after
        Gtk::TreeModel::Row row;
        if (history)
            row = *(marker_store->insert(sep_iter));
        else
            row = *(marker_store->append());

        row[marker_columns.label] = gr_ellipsize_text(markid, 20);
        // Non "stock" markers can also have "inkscape:stockid" (when using extension ColorMarkers),
        // So use !is_history instead to determine is it is "stock" (ie in the markers.svg file)
        row[marker_columns.stock] = !history;
        row[marker_columns.marker] = repr->attribute("id");
        row[marker_columns.image] = prv;
        row[marker_columns.history] = history;
        row[marker_columns.separator] = false;

    }

    sandbox->getRoot()->invoke_hide(visionkey);
}

/*
 * Remove from the cache and recreate a marker image
 */
void
MarkerComboBox::update_marker_image(gchar const *mname)
{
    gchar *cache_name = g_strconcat(combo_id, mname, NULL);
    Glib::ustring key = svg_preview_cache.cache_key(doc->getURI(), cache_name, 22);
    g_free (cache_name);
    svg_preview_cache.remove_preview_from_cache(key);

    Inkscape::Drawing drawing;
    unsigned const visionkey = SPItem::display_key_new(1);
    drawing.setRoot(sandbox->getRoot()->invoke_show(drawing, visionkey, SP_ITEM_SHOW_DISPLAY));
    Gtk::Image *prv = create_marker_image(22, mname, doc, drawing, visionkey);
    if (prv) {
        prv->show();
    }
    sandbox->getRoot()->invoke_hide(visionkey);

    for(Gtk::TreeIter iter = marker_store->children().begin();
        iter != marker_store->children().end(); ++iter) {
            Gtk::TreeModel::Row row = (*iter);
            if (row[marker_columns.marker] && row[marker_columns.history] &&
                    !strcmp(row[marker_columns.marker], mname)) {
                row[marker_columns.image] = prv;
                return;
            }
    }

}
/**
 * Creates a copy of the marker named mname, determines its visible and renderable
 * area in the bounding box, and then renders it.  This allows us to fill in
 * preview images of each marker in the marker combobox.
 */
Gtk::Image *
MarkerComboBox::create_marker_image(unsigned psize, gchar const *mname,
                   SPDocument *source,  Inkscape::Drawing &drawing, unsigned /*visionkey*/)
{
    // Retrieve the marker named 'mname' from the source SVG document
    SPObject const *marker = source->getObjectById(mname);
    if (marker == NULL) {
        return NULL;
    }

    // Create a copy repr of the marker with id="sample"
    Inkscape::XML::Document *xml_doc = sandbox->getReprDoc();
    Inkscape::XML::Node *mrepr = marker->getRepr()->duplicate(xml_doc);
    mrepr->setAttribute("id", "sample");

    // Replace the old sample in the sandbox by the new one
    Inkscape::XML::Node *defsrepr = sandbox->getObjectById("defs")->getRepr();
    SPObject *oldmarker = sandbox->getObjectById("sample");
    if (oldmarker) {
        oldmarker->deleteObject(false);
    }

    // TODO - This causes a SIGTRAP on windows
    defsrepr->appendChild(mrepr);

    Inkscape::GC::release(mrepr);

    // If the marker color is a url link to a pattern or gradient copy that too
    SPObject *mk = source->getObjectById(mname);
    SPCSSAttr *css_marker = sp_css_attr_from_object(mk->firstChild(), SP_STYLE_FLAG_ALWAYS);
    //const char *mfill = sp_repr_css_property(css_marker, "fill", "none");
    const char *mstroke = sp_repr_css_property(css_marker, "fill", "none");

    if (!strncmp (mstroke, "url(", 4)) {
        SPObject *linkObj = getMarkerObj(mstroke, source);
        if (linkObj) {
            Inkscape::XML::Node *grepr = linkObj->getRepr()->duplicate(xml_doc);
            SPObject *oldmarker = sandbox->getObjectById(linkObj->getId());
            if (oldmarker) {
                oldmarker->deleteObject(false);
            }
            defsrepr->appendChild(grepr);
            Inkscape::GC::release(grepr);

            if (SP_IS_GRADIENT(linkObj)) {
                SPGradient *vector = sp_gradient_get_forked_vector_if_necessary (SP_GRADIENT(linkObj), false);
                if (vector) {
                    Inkscape::XML::Node *grepr = vector->getRepr()->duplicate(xml_doc);
                    SPObject *oldmarker = sandbox->getObjectById(vector->getId());
                    if (oldmarker) {
                        oldmarker->deleteObject(false);
                    }
                    defsrepr->appendChild(grepr);
                    Inkscape::GC::release(grepr);
                }
            }
        }
    }

// Uncomment this to get the sandbox documents saved (useful for debugging)
    //FILE *fp = fopen (g_strconcat(combo_id, mname, ".svg", NULL), "w");
    //sp_repr_save_stream(sandbox->getReprDoc(), fp);
    //fclose (fp);

    // object to render; note that the id is the same as that of the combo we're building
    SPObject *object = sandbox->getObjectById(combo_id);
    sandbox->getRoot()->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    sandbox->ensureUpToDate();

    if (object == NULL || !SP_IS_ITEM(object)) {
        return NULL; // sandbox broken?
    }

    SPItem *item = SP_ITEM(object);
    // Find object's bbox in document
    Geom::OptRect dbox = item->documentVisualBounds();

    if (!dbox) {
        return NULL;
    }

    /* Update to renderable state */
    gchar *cache_name = g_strconcat(combo_id, mname, NULL);
    Glib::ustring key = svg_preview_cache.cache_key(source->getURI(), cache_name, psize);
    g_free (cache_name);
    GdkPixbuf *pixbuf = svg_preview_cache.get_preview_from_cache(key); // no ref created

    if (!pixbuf) {
        pixbuf = render_pixbuf(drawing, 0.8, *dbox, psize);
        svg_preview_cache.set_preview_in_cache(key, pixbuf);
        g_object_unref(pixbuf); // reference is held by svg_preview_cache
    }

    // Create widget
    Gtk::Image *pb = Glib::wrap(GTK_IMAGE(gtk_image_new_from_pixbuf(pixbuf)));
    return pb;
}

void MarkerComboBox::prepareImageRenderer( Gtk::TreeModel::const_iterator const &row ) {

    Gtk::Image *image = (*row)[marker_columns.image];
    if (image)
        image_renderer.property_pixbuf() = image->get_pixbuf();
    else
        image_renderer.property_pixbuf() = empty_image->get_pixbuf();
}

gboolean MarkerComboBox::separator_cb (GtkTreeModel *model, GtkTreeIter *iter, gpointer /*data*/) {

    gboolean sep = FALSE;
    gtk_tree_model_get(model, iter, 4, &sep, -1);
    return sep;
}

/**
 * Returns a new document containing default start, mid, and end markers.
 */
SPDocument *MarkerComboBox::ink_markers_preview_doc ()
{
gchar const *buffer = "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\" xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" id=\"MarkerSample\">"
"  <defs id=\"defs\" />"

"  <g id=\"marker-start\">"
"    <path style=\"fill:gray;stroke:darkgray;stroke-width:1.7;marker-start:url(#sample);marker-mid:none;marker-end:none\""
"       d=\"M 12.5,13 L 25,13\" id=\"path1\" />"
"    <rect style=\"fill:none;stroke:none\" id=\"rect2\""
"       width=\"25\" height=\"25\" x=\"0\" y=\"0\" />"
"  </g>"

"  <g id=\"marker-mid\">"
"    <path style=\"fill:gray;stroke:darkgray;stroke-width:1.7;marker-start:none;marker-mid:url(#sample);marker-end:none\""
"       d=\"M 0,113 L 12.5,113 L 25,113\" id=\"path11\" />"
"    <rect style=\"fill:none;stroke:none\" id=\"rect22\""
"       width=\"25\" height=\"25\" x=\"0\" y=\"100\" />"
"  </g>"

"  <g id=\"marker-end\">"
"    <path style=\"fill:gray;stroke:darkgray;stroke-width:1.7;marker-start:none;marker-mid:none;marker-end:url(#sample)\""
"       d=\"M 0,213 L 12.5,213\" id=\"path111\" />"
"    <rect style=\"fill:none;stroke:none\" id=\"rect222\""
"       width=\"25\" height=\"25\" x=\"0\" y=\"200\" />"
"  </g>"

"</svg>";

    return SPDocument::createNewDocFromMem (buffer, strlen(buffer), FALSE);
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
