#define __CONTEXT_MENU_C__

/*
 * Unser-interface related object extension
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "context-menu.h"
#include "../xml/repr.h"
#include "desktop.h"
#include "document.h"
#include "message-stack.h"
#include "preferences.h"
#include "ui/dialog/dialog-manager.h"

static void sp_object_type_menu(GType type, SPObject *object, SPDesktop *desktop, GtkMenu *menu);

/* Append object-specific part to context menu */

void
sp_object_menu(SPObject *object, SPDesktop *desktop, GtkMenu *menu)
{
    GObjectClass *klass;
    klass = G_OBJECT_GET_CLASS(object);
    while (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_OBJECT)) {
        GType type;
        type = G_TYPE_FROM_CLASS(klass);
        sp_object_type_menu(type, object, desktop, menu);
        klass = (GObjectClass*)g_type_class_peek_parent(klass);
    }
}

/* Implementation */

#include <gtk/gtkmenuitem.h>

#include <glibmm/i18n.h>

#include "sp-anchor.h"
#include "sp-image.h"

#include "document.h"
#include "desktop-handles.h"
#include "selection.h"
#include "selection-chemistry.h"
#include "dialogs/item-properties.h"
#include "dialogs/object-attributes.h"

#include "sp-path.h"
#include "sp-clippath.h"
#include "sp-mask.h"


static void sp_item_menu(SPObject *object, SPDesktop *desktop, GtkMenu *menu);
static void sp_group_menu(SPObject *object, SPDesktop *desktop, GtkMenu *menu);
static void sp_anchor_menu(SPObject *object, SPDesktop *desktop, GtkMenu *menu);
static void sp_image_menu(SPObject *object, SPDesktop *desktop, GtkMenu *menu);
static void sp_shape_menu(SPObject *object, SPDesktop *desktop, GtkMenu *menu);

static void
sp_object_type_menu(GType type, SPObject *object, SPDesktop *desktop, GtkMenu *menu)
{
    static GHashTable *t2m = NULL;
    void (* handler)(SPObject *object, SPDesktop *desktop, GtkMenu *menu);
    if (!t2m) {
        t2m = g_hash_table_new(NULL, NULL);
        g_hash_table_insert(t2m, GUINT_TO_POINTER(SP_TYPE_ITEM), (void*)sp_item_menu);
        g_hash_table_insert(t2m, GUINT_TO_POINTER(SP_TYPE_GROUP), (void*)sp_group_menu);
        g_hash_table_insert(t2m, GUINT_TO_POINTER(SP_TYPE_ANCHOR), (void*)sp_anchor_menu);
        g_hash_table_insert(t2m, GUINT_TO_POINTER(SP_TYPE_IMAGE), (void*)sp_image_menu);
        g_hash_table_insert(t2m, GUINT_TO_POINTER(SP_TYPE_SHAPE), (void*)sp_shape_menu);
    }
    handler = (void (*)(SPObject*, SPDesktop*, GtkMenu*))g_hash_table_lookup(t2m, GUINT_TO_POINTER(type));
    if (handler) handler(object, desktop, menu);
}

/* SPItem */

static void sp_item_properties(GtkMenuItem *menuitem, SPItem *item);
static void sp_item_select_this(GtkMenuItem *menuitem, SPItem *item);
static void sp_item_create_link(GtkMenuItem *menuitem, SPItem *item);
static void sp_set_mask(GtkMenuItem *menuitem, SPItem *item);
static void sp_release_mask(GtkMenuItem *menuitem, SPItem *item);
static void sp_set_clip(GtkMenuItem *menuitem, SPItem *item);
static void sp_release_clip(GtkMenuItem *menuitem, SPItem *item);
/* Generate context menu item section */

static void
sp_item_menu(SPObject *object, SPDesktop *desktop, GtkMenu *m)
{
    SPItem *item;
    GtkWidget *w;

    item = (SPItem *) object;

    /* Item dialog */
    w = gtk_menu_item_new_with_mnemonic(_("Object _Properties"));
    gtk_object_set_data(GTK_OBJECT(w), "desktop", desktop);
    gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(sp_item_properties), item);
    gtk_widget_show(w);
    gtk_menu_append(GTK_MENU(m), w);
    /* Separator */
    w = gtk_menu_item_new();
    gtk_widget_show(w);
    gtk_menu_append(GTK_MENU(m), w);
    /* Select item */
    w = gtk_menu_item_new_with_mnemonic(_("_Select This"));
    if (sp_desktop_selection(desktop)->includes(item)) {
        gtk_widget_set_sensitive(w, FALSE);
    } else {
        gtk_object_set_data(GTK_OBJECT(w), "desktop", desktop);
        gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(sp_item_select_this), item);
    }
    gtk_widget_show(w);
    gtk_menu_append(GTK_MENU(m), w);
    /* Create link */
    w = gtk_menu_item_new_with_mnemonic(_("_Create Link"));
    gtk_object_set_data(GTK_OBJECT(w), "desktop", desktop);
    gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(sp_item_create_link), item);
    gtk_widget_set_sensitive(w, !SP_IS_ANCHOR(item));
    gtk_widget_show(w);
    gtk_menu_append(GTK_MENU(m), w);
    /* Set mask */
    w = gtk_menu_item_new_with_mnemonic(_("Set Mask"));
    gtk_object_set_data(GTK_OBJECT(w), "desktop", desktop);
    gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(sp_set_mask), item);
    if ((item && item->mask_ref && item->mask_ref->getObject()) || (item->clip_ref && item->clip_ref->getObject())) {
        gtk_widget_set_sensitive(w, FALSE);
    } else {
        gtk_widget_set_sensitive(w, TRUE);
    }
    gtk_widget_show(w);
    gtk_menu_append(GTK_MENU(m), w);
    /* Release mask */
    w = gtk_menu_item_new_with_mnemonic(_("Release Mask"));
    gtk_object_set_data(GTK_OBJECT(w), "desktop", desktop);
    gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(sp_release_mask), item);
    if (item && item->mask_ref && item->mask_ref->getObject()) {
        gtk_widget_set_sensitive(w, TRUE);
    } else {
        gtk_widget_set_sensitive(w, FALSE);
    }
    gtk_widget_show(w);
    gtk_menu_append(GTK_MENU(m), w);
    /* Set Clip */
    w = gtk_menu_item_new_with_mnemonic(_("Set Clip"));
    gtk_object_set_data(GTK_OBJECT(w), "desktop", desktop);
    gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(sp_set_clip), item);
    if ((item && item->mask_ref && item->mask_ref->getObject()) || (item->clip_ref && item->clip_ref->getObject())) {
        gtk_widget_set_sensitive(w, FALSE);
    } else {
        gtk_widget_set_sensitive(w, TRUE);
    }
    gtk_widget_show(w);
    gtk_menu_append(GTK_MENU(m), w);
    /* Release Clip */
    w = gtk_menu_item_new_with_mnemonic(_("Release Clip"));
    gtk_object_set_data(GTK_OBJECT(w), "desktop", desktop);
    gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(sp_release_clip), item);
    if (item && item->clip_ref && item->clip_ref->getObject()) {
        gtk_widget_set_sensitive(w, TRUE);
    } else {
        gtk_widget_set_sensitive(w, FALSE);
    }
    gtk_widget_show(w);
    gtk_menu_append(GTK_MENU(m), w);

}

static void
sp_item_properties(GtkMenuItem *menuitem, SPItem *item)
{
    SPDesktop *desktop;

    g_assert(SP_IS_ITEM(item));

    desktop = (SPDesktop*)gtk_object_get_data(GTK_OBJECT(menuitem), "desktop");
    g_return_if_fail(desktop != NULL);

    sp_desktop_selection(desktop)->set(item);

    sp_item_dialog();
}


static void
sp_set_mask(GtkMenuItem *menuitem, SPItem *item)
{
    SPDesktop *desktop;

    g_assert(SP_IS_ITEM(item));

    desktop = (SPDesktop*)gtk_object_get_data(GTK_OBJECT(menuitem), "desktop");
    g_return_if_fail(desktop != NULL);

	sp_selection_set_mask(desktop, false, false);
}


static void
sp_release_mask(GtkMenuItem *menuitem, SPItem *item)
{
    SPDesktop *desktop;

    g_assert(SP_IS_ITEM(item));

    desktop = (SPDesktop*)gtk_object_get_data(GTK_OBJECT(menuitem), "desktop");
    g_return_if_fail(desktop != NULL);

    sp_selection_unset_mask(desktop, false);
}


static void
sp_set_clip(GtkMenuItem *menuitem, SPItem *item)
{
    SPDesktop *desktop;

    g_assert(SP_IS_ITEM(item));

    desktop = (SPDesktop*)gtk_object_get_data(GTK_OBJECT(menuitem), "desktop");
    g_return_if_fail(desktop != NULL);

	sp_selection_set_mask(desktop, true, false);
}


static void
sp_release_clip(GtkMenuItem *menuitem, SPItem *item)
{
    SPDesktop *desktop;

    g_assert(SP_IS_ITEM(item));

    desktop = (SPDesktop*)gtk_object_get_data(GTK_OBJECT(menuitem), "desktop");
    g_return_if_fail(desktop != NULL);

    sp_selection_unset_mask(desktop, true);
}


static void
sp_item_select_this(GtkMenuItem *menuitem, SPItem *item)
{
    SPDesktop *desktop;

    g_assert(SP_IS_ITEM(item));

    desktop = (SPDesktop*)gtk_object_get_data(GTK_OBJECT(menuitem), "desktop");
    g_return_if_fail(desktop != NULL);

    sp_desktop_selection(desktop)->set(item);
}

static void
sp_item_create_link(GtkMenuItem *menuitem, SPItem *item)
{
    g_assert(SP_IS_ITEM(item));
    g_assert(!SP_IS_ANCHOR(item));

    SPDesktop *desktop = (SPDesktop*)gtk_object_get_data(GTK_OBJECT(menuitem), "desktop");
    g_return_if_fail(desktop != NULL);

    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());
    Inkscape::XML::Node *repr = xml_doc->createElement("svg:a");
    SP_OBJECT_REPR(SP_OBJECT_PARENT(item))->addChild(repr, SP_OBJECT_REPR(item));
    SPObject *object = SP_OBJECT_DOCUMENT(item)->getObjectByRepr(repr);
    g_return_if_fail(SP_IS_ANCHOR(object));

    const char *id = SP_OBJECT_REPR(item)->attribute("id");
    Inkscape::XML::Node *child = SP_OBJECT_REPR(item)->duplicate(xml_doc);
    SP_OBJECT(item)->deleteObject(false);
    repr->addChild(child, NULL);
    child->setAttribute("id", id);

    Inkscape::GC::release(repr);
    Inkscape::GC::release(child);

    sp_document_done(SP_OBJECT_DOCUMENT(object), SP_VERB_NONE,
                     _("Create link"));

    sp_object_attributes_dialog(object, "SPAnchor");

    sp_desktop_selection(desktop)->set(SP_ITEM(object));
}

/* SPGroup */

static void sp_item_group_ungroup_activate(GtkMenuItem *menuitem, SPGroup *group);

static void
sp_group_menu(SPObject *object, SPDesktop *desktop, GtkMenu *menu)
{
    SPItem *item=SP_ITEM(object);
    GtkWidget *w;

    /* "Ungroup" */
    w = gtk_menu_item_new_with_mnemonic(_("_Ungroup"));
    gtk_object_set_data(GTK_OBJECT(w), "desktop", desktop);
    gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(sp_item_group_ungroup_activate), item);
    gtk_widget_show(w);
    gtk_menu_append(GTK_MENU(menu), w);
}

static void
sp_item_group_ungroup_activate(GtkMenuItem *menuitem, SPGroup *group)
{
    SPDesktop *desktop;
    GSList *children;

    g_assert(SP_IS_GROUP(group));

    desktop = (SPDesktop*)gtk_object_get_data(GTK_OBJECT(menuitem), "desktop");
    g_return_if_fail(desktop != NULL);

    children = NULL;
    sp_item_group_ungroup(group, &children);

    sp_desktop_selection(desktop)->setList(children);
    g_slist_free(children);
}

/* SPAnchor */

static void sp_anchor_link_properties(GtkMenuItem *menuitem, SPAnchor *anchor);
static void sp_anchor_link_follow(GtkMenuItem *menuitem, SPAnchor *anchor);
static void sp_anchor_link_remove(GtkMenuItem *menuitem, SPAnchor *anchor);

static void
sp_anchor_menu(SPObject *object, SPDesktop *desktop, GtkMenu *m)
{
    SPItem *item;
    GtkWidget *w;

    item = (SPItem *) object;

    /* Link dialog */
    w = gtk_menu_item_new_with_mnemonic(_("Link _Properties"));
    gtk_object_set_data(GTK_OBJECT(w), "desktop", desktop);
    gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(sp_anchor_link_properties), item);
    gtk_widget_show(w);
    gtk_menu_append(GTK_MENU(m), w);
    /* Select item */
    w = gtk_menu_item_new_with_mnemonic(_("_Follow Link"));
    gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(sp_anchor_link_follow), item);
    gtk_widget_show(w);
    gtk_menu_append(GTK_MENU(m), w);
    /* Reset transformations */
    w = gtk_menu_item_new_with_mnemonic(_("_Remove Link"));
    gtk_object_set_data(GTK_OBJECT(w), "desktop", desktop);
    gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(sp_anchor_link_remove), item);
    gtk_widget_show(w);
    gtk_menu_append(GTK_MENU(m), w);
}

static void
sp_anchor_link_properties(GtkMenuItem */*menuitem*/, SPAnchor *anchor)
{
    sp_object_attributes_dialog(SP_OBJECT(anchor), "Link");
}

static void
sp_anchor_link_follow(GtkMenuItem */*menuitem*/, SPAnchor *anchor)
{
    g_return_if_fail(anchor != NULL);
    g_return_if_fail(SP_IS_ANCHOR(anchor));

    /* shell out to an external browser here */
}

static void
sp_anchor_link_remove(GtkMenuItem */*menuitem*/, SPAnchor *anchor)
{
    GSList *children;

    g_return_if_fail(anchor != NULL);
    g_return_if_fail(SP_IS_ANCHOR(anchor));

    children = NULL;
    sp_item_group_ungroup(SP_GROUP(anchor), &children);

    g_slist_free(children);
}

/* Image */

static void sp_image_image_properties(GtkMenuItem *menuitem, SPAnchor *anchor);
static void sp_image_image_edit(GtkMenuItem *menuitem, SPAnchor *anchor);

static void
sp_image_menu(SPObject *object, SPDesktop *desktop, GtkMenu *m)
{
    SPItem *item = SP_ITEM(object);
    GtkWidget *w;

    /* Link dialog */
    w = gtk_menu_item_new_with_mnemonic(_("Image _Properties"));
    gtk_object_set_data(GTK_OBJECT(w), "desktop", desktop);
    gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(sp_image_image_properties), item);
    gtk_widget_show(w);
    gtk_menu_append(GTK_MENU(m), w);

    w = gtk_menu_item_new_with_mnemonic(_("Edit Externally..."));
    gtk_object_set_data(GTK_OBJECT(w), "desktop", desktop);
    gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(sp_image_image_edit), item);
    gtk_widget_show(w);
    gtk_menu_append(GTK_MENU(m), w);
    Inkscape::XML::Node *ir = SP_OBJECT_REPR(object);
    const gchar *href = ir->attribute("xlink:href");
    if ( (!href) || ((strncmp(href, "data:", 5) == 0)) ) {
        gtk_widget_set_sensitive( w, FALSE );
    }
}

static void
sp_image_image_properties(GtkMenuItem */*menuitem*/, SPAnchor *anchor)
{
    sp_object_attributes_dialog(SP_OBJECT(anchor), "Image");
}

static gchar* getImageEditorName() {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gchar* value = 0;
    Glib::ustring choices = prefs->getString("/options/bitmapeditor/choices");
    if (!choices.empty()) {
        gchar** splits = g_strsplit(choices.data(), ",", 0);
        gint numIems = g_strv_length(splits);

        int setting = prefs->getIntLimited("/options/bitmapeditor/value", 0, 0, numIems);
        value = g_strdup(splits[setting]);

        g_strfreev(splits);
    }

    if (!value) {
        value = g_strdup("gimp");
    }
    return value;
}

static void sp_image_image_edit(GtkMenuItem *menuitem, SPAnchor *anchor)
{
    SPObject* obj = SP_OBJECT(anchor);
    Inkscape::XML::Node *ir = SP_OBJECT_REPR(obj);
    const gchar *href = ir->attribute("xlink:href");

    GError* errThing = 0;
    gchar* editorBin = getImageEditorName();
    gchar const* args[] = {editorBin, href, 0};
    g_spawn_async(0, // working dir
                  const_cast<gchar **>(args),
                  0, //envp
                  G_SPAWN_SEARCH_PATH,
                  0, // child_setup
                  0, // user_data
                  0, //GPid *child_pid
                  &errThing);
    if ( errThing ) {
        g_warning("Problem launching editor (%d). %s", errThing->code, errThing->message);
        SPDesktop *desktop = (SPDesktop*)gtk_object_get_data(GTK_OBJECT(menuitem), "desktop");
        desktop->messageStack()->flash(Inkscape::ERROR_MESSAGE, errThing->message);
        g_error_free(errThing);
        errThing = 0;
    }
    g_free(editorBin);
}

/* SPShape */

static void
sp_shape_fill_settings(GtkMenuItem *menuitem, SPItem *item)
{
    SPDesktop *desktop;

    g_assert(SP_IS_ITEM(item));

    desktop = (SPDesktop*)gtk_object_get_data(GTK_OBJECT(menuitem), "desktop");
    g_return_if_fail(desktop != NULL);

    if (sp_desktop_selection(desktop)->isEmpty()) {
        sp_desktop_selection(desktop)->set(item);
    }

    desktop->_dlg_mgr->showDialog("FillAndStroke");
}

static void
sp_shape_menu(SPObject *object, SPDesktop *desktop, GtkMenu *m)
{
    SPItem *item;
    GtkWidget *w;

    item = (SPItem *) object;

    /* Item dialog */
    w = gtk_menu_item_new_with_mnemonic(_("_Fill and Stroke"));
    gtk_object_set_data(GTK_OBJECT(w), "desktop", desktop);
    gtk_signal_connect(GTK_OBJECT(w), "activate", GTK_SIGNAL_FUNC(sp_shape_fill_settings), item);
    gtk_widget_show(w);
    gtk_menu_append(GTK_MENU(m), w);
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
