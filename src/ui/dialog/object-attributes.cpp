/** @file
 * Generic object attribute editor
 */
/* 
 * Inkscape, an Open Source vector graphics editor
 * Copyright (C) 2012 Kris De Gussem <Kris.DeGussem@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "ui/dialog/dialog-manager.h"
#include "desktop.h"

#include "macros.h"
#include "sp-anchor.h"
#include "sp-image.h"
#include "verbs.h"
#include "xml/repr.h"
#include "ui/dialog/object-attributes.h"
#include "widgets/sp-attribute-widget.h"
#include "inkscape.h"
#include "selection.h"
#include <glibmm/i18n.h>

namespace Inkscape {
namespace UI {
namespace Dialog {

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
    { N_("Image Rendering:"), "image-rendering"},
    { NULL, NULL}
};

static const SPAttrDesc image_nohref_desc[] = {
    { N_("X:"), "x"},
    { N_("Y:"), "y"},
    { N_("Width:"), "width"},
    { N_("Height:"), "height"},
    { NULL, NULL}
};

ObjectAttributes::ObjectAttributes (void) :
    UI::Widget::Panel ("", "/dialogs/objectattr/", SP_VERB_DIALOG_ATTR),
    blocked (false),
    CurrentItem(NULL),
    attrTable(Gtk::manage(new SPAttributeTable())),
    desktop(NULL),
    deskTrack(),
    selectChangedConn(),
    subselChangedConn(),
    selectModifiedConn()
{
    attrTable->show();
    widget_setup();
    
    desktopChangeConn = deskTrack.connectDesktopChanged( sigc::mem_fun(*this, &ObjectAttributes::setTargetDesktop) );
    deskTrack.connect(GTK_WIDGET(gobj()));
}

ObjectAttributes::~ObjectAttributes (void)
{
    selectModifiedConn.disconnect();
    subselChangedConn.disconnect();
    selectChangedConn.disconnect();
    desktopChangeConn.disconnect();
    deskTrack.disconnect();
}

void ObjectAttributes::widget_setup (void)
{
    if (blocked)
    {
        return;
    }
    
    Inkscape::Selection *selection = SP_ACTIVE_DESKTOP->getSelection();
    SPItem *item = selection->singleItem();
    if (!item)
    {
        set_sensitive (false);
        CurrentItem = NULL;
        //no selection anymore or multiple objects selected, means that we need
        //to close the connections to the previously selected object
        return;
    }
    
    blocked = true;

    // CPPIFY
    SPObject *obj = item; //to get the selected item
//    GObjectClass *klass = G_OBJECT_GET_CLASS(obj); //to deduce the object's type
//    GType type = G_TYPE_FROM_CLASS(klass);
    const SPAttrDesc *desc;
    
//    if (type == SP_TYPE_ANCHOR)
    if (SP_IS_ANCHOR(item))
    {
        desc = anchor_desc;
    }
//    else if (type == SP_TYPE_IMAGE)
    else if (SP_IS_IMAGE(item))
    {
        Inkscape::XML::Node *ir = obj->getRepr();
        const gchar *href = ir->attribute("xlink:href");
        if ( (!href) || ((strncmp(href, "data:", 5) == 0)) )
        {
            desc = image_nohref_desc;
        }
        else
        {
            desc = image_desc;
        }
    }
    else
    {
        blocked = false;
        set_sensitive (false);
        return;
    }
    
    std::vector<Glib::ustring> labels;
    std::vector<Glib::ustring> attrs;
    if (CurrentItem != item)
    {
        int len = 0;
        while (desc[len].label)
        {
            labels.push_back(desc[len].label);
            attrs.push_back (desc[len].attribute);
            len += 1;
        }
        attrTable->set_object(obj, labels, attrs, (GtkWidget*)gobj());
        CurrentItem = item;
    }
    else
    {
        attrTable->change_object(obj);
    }
    
    set_sensitive (true);
    show_all();
    blocked = false;
}

void ObjectAttributes::setTargetDesktop(SPDesktop *desktop)
{
    if (this->desktop != desktop) {
        if (this->desktop) {
            selectModifiedConn.disconnect();
            subselChangedConn.disconnect();
            selectChangedConn.disconnect();
        }
        this->desktop = desktop;
        if (desktop && desktop->selection) {
            selectChangedConn = desktop->selection->connectChanged(sigc::hide(sigc::mem_fun(*this, &ObjectAttributes::widget_setup)));
            subselChangedConn = desktop->connectToolSubselectionChanged(sigc::hide(sigc::mem_fun(*this, &ObjectAttributes::widget_setup)));

            // Must check flags, so can't call widget_setup() directly.
            selectModifiedConn = desktop->selection->connectModified(sigc::hide<0>(sigc::mem_fun(*this, &ObjectAttributes::selectionModifiedCB)));
        }
        widget_setup();
    }
}

void ObjectAttributes::selectionModifiedCB( guint flags )
{
    if (flags & ( SP_OBJECT_MODIFIED_FLAG |
                   SP_OBJECT_PARENT_MODIFIED_FLAG |
                   SP_OBJECT_STYLE_MODIFIED_FLAG) ) {
        attrTable->reread_properties();
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
