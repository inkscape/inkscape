/**
 * @file
 * Generic properties editor.
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 1999-2011 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>
#include <string>
#include <cstring>
#include <stddef.h>
#include <sigc++/connection.h>
#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>

#include "helper/window.h"
#include "macros.h"
#include "sp-anchor.h"
#include "widgets/sp-attribute-widget.h"
#include "../xml/repr.h"

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


static void sp_object_attr_show_dialog ( SPObject *object,
                             const SPAttrDesc *desc,
                             const gchar *tag )
{
    int len;
	Gtk::Window *window;
    SPAttributeTable* t;
    Glib::ustring title;
    std::vector<Glib::ustring> labels;
    std::vector<Glib::ustring> attrs;

    if (!strcmp (tag, "Link")) {
        title = _("Link Properties");
    } else if (!strcmp (tag, "Image")) {
        title = _("Image Properties");
    } else {
        title = Glib::ustring::compose(_("%1 Properties"), tag);
    }

    len = 0;
    while (desc[len].label)
    {
        labels.push_back(desc[len].label);
        attrs.push_back (desc[len].attribute);
        len += 1;
    }
    
    window = Inkscape::UI::window_new (title.c_str(), true);
    t = new SPAttributeTable (object, labels, attrs, (GtkWidget*)window->gobj());
    t->show();
    window->show();
} // end of sp_object_attr_show_dialog()


void sp_object_attributes_dialog (SPObject *object, const gchar *tag)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (SP_IS_OBJECT (object));
    g_return_if_fail (tag != NULL);

    if (!strcmp (tag, "Link")) {
        sp_object_attr_show_dialog (object, anchor_desc, tag);
    } else if (!strcmp (tag, "Image")) {
        Inkscape::XML::Node *ir = object->getRepr();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
