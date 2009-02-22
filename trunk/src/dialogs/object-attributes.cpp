/** @file
 * @brief  Generic properties editor
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>
#include <string>
#include <cstring>

#include "helper/window.h"
#include "macros.h"
#include "sp-anchor.h"
#include "sp-attribute-widget.h"
#include "../xml/repr.h"

#include <sigc++/connection.h>
#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>

struct SPAttrDesc {
    gchar const *label;
    gchar const *attribute;
};

static const SPAttrDesc anchor_desc[] = {
    { N_("Href:"), "xlink:href"},
    { N_("Target:"), "target"},
    { N_("Type:"), "xlink:type"},
    // TRANSLATORS: for info, see http://www.w3.org/TR/2000/CR-SVG-20000802/linking.html#AElementXLinkRoleAttribute
    // Identifies the type of the related resource with an absolute URI
    { N_("Role:"), "xlink:role"},
    // TRANSLATORS: for info, see http://www.w3.org/TR/2000/CR-SVG-20000802/linking.html#AElementXLinkArcRoleAttribute
    // For situations where the nature/role alone isn't enough, this offers an additional URI defining the purpose of the link.
    { N_("Arcrole:"), "xlink:arcrole"},
    // TRANSLATORS: for info, see http://www.w3.org/TR/2000/CR-SVG-20000802/linking.html#AElementXLinkTitleAttribute
    { N_("Title:"), "xlink:title"},
    { N_("Show:"), "xlink:show"},
    // TRANSLATORS: for info, see http://www.w3.org/TR/2000/CR-SVG-20000802/linking.html#AElementXLinkActuateAttribute
    { N_("Actuate:"), "xlink:actuate"},
    { NULL, NULL}
};

static const SPAttrDesc image_desc[] = {
    { N_("URL:"), "xlink:href"},
    { N_("X:"), "x"},
    { N_("Y:"), "y"},
    { N_("Width:"), "width"},
    { N_("Height:"), "height"},
    { NULL, NULL}
};

static const SPAttrDesc image_nohref_desc[] = {
    { N_("X:"), "x"},
    { N_("Y:"), "y"},
    { N_("Width:"), "width"},
    { N_("Height:"), "height"},
    { NULL, NULL}
};


static void
object_released( SPObject */*object*/, GtkWidget *widget )
{
    gtk_widget_destroy (widget);
}



static void
window_destroyed( GtkObject *window, GtkObject */*object*/ )
{
    sigc::connection *release_connection = (sigc::connection *)g_object_get_data(G_OBJECT(window), "release_connection");
    release_connection->disconnect();
    delete release_connection;
}



static void
sp_object_attr_show_dialog ( SPObject *object,
                             const SPAttrDesc *desc,
                             const gchar *tag )
{
    const gchar **labels, **attrs;
    gint len, i;
    gchar *title;
    GtkWidget *w, *t;

    len = 0;
    while (desc[len].label) len += 1;

    labels = (const gchar **)alloca (len * sizeof (char *));
    attrs = (const gchar **)alloca (len * sizeof (char *));

    for (i = 0; i < len; i++) {
        labels[i] = desc[i].label;
        attrs[i] = desc[i].attribute;
    }

    title = g_strdup_printf (_("%s Properties"), tag);
    w = sp_window_new (title, TRUE);
    g_free (title);

    t = sp_attribute_table_new (object, len, labels, attrs);
    gtk_widget_show (t);
    gtk_container_add (GTK_CONTAINER (w), t);

    g_signal_connect ( G_OBJECT (w), "destroy",
                       G_CALLBACK (window_destroyed), object );

    sigc::connection *release_connection = new sigc::connection();
    *release_connection = object->connectRelease(sigc::bind<1>(sigc::ptr_fun(&object_released), w));
    g_object_set_data(G_OBJECT(w), "release_connection", release_connection);

    gtk_widget_show (w);

} // end of sp_object_attr_show_dialog()



void
sp_object_attributes_dialog (SPObject *object, const gchar *tag)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (SP_IS_OBJECT (object));
    g_return_if_fail (tag != NULL);

    if (!strcmp (tag, "Link")) {
        sp_object_attr_show_dialog (object, anchor_desc, tag);
    } else if (!strcmp (tag, "Image")) {
        Inkscape::XML::Node *ir = SP_OBJECT_REPR(object);
        const gchar *href = ir->attribute("xlink:href");
        if ( (!href) || ((strncmp(href, "data:", 5) == 0)) ) {
            sp_object_attr_show_dialog (object, image_nohref_desc, tag);
        } else {
            sp_object_attr_show_dialog (object, image_desc, tag);
        }
    } 
} // end of sp_object_attributes_dialog()

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
