/*
 * This is where the implementation of the DBus based document API lives.
 * All the methods in here (except in the helper section) are 
 * designed to be called remotly via DBus. application-interface.cpp
 * has the methods used to connect to the bus and get a document instance.
 *
 * Documentation for these methods is in document-interface.xml
 * which is the "gold standard" as to how the interface should work.
 *
 * Authors:
 *   Soren Berg <Glimmer07@gmail.com>
 *
 * Copyright (C) 2009 Soren Berg
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "document-interface.h"
#include <string.h>

#include "verbs.h"
#include "helper/action.h" //sp_action_perform

#include "inkscape.h" //inkscape_find_desktop_by_dkey, activate desktops

#include "desktop-handles.h" //sp_desktop_document()
#include "xml/repr.h" //sp_repr_document_new

#include "sp-object.h"

#include "document.h" // sp_document_repr_doc

#include "desktop-style.h" //sp_desktop_get_style

#include "selection.h" //selection struct
#include "selection-chemistry.h"// lots of selection functions

#include "sp-ellipse.h"

#include "layer-fns.h" //LPOS_BELOW

#include "style.h" //style_write

#include "file.h" //IO

#include "extension/system.h" //IO

#include "extension/output.h" //IO

#include "print.h" //IO

#include "live_effects/parameter/text.h" //text
#include "display/canvas-text.h" //text

//#include "2geom/svg-path-parser.h" //get_node_coordinates

/****************************************************************************
     HELPER / SHORTCUT FUNCTIONS
****************************************************************************/

/* 
 * This function or the one below it translates the user input for an object
 * into Inkscapes internal representation.  It is called by almost every
 * method so it should be as fast as possible.
 *
 * (eg turns "rect2234" to an SPObject or Inkscape::XML::Node)
 *
 * If the internal representation changes (No more 'id' attributes) this is the
 * place to adjust things.
 */
Inkscape::XML::Node *
get_repr_by_name (SPDesktop *desk, gchar *name, GError **error)
{
    /* ALTERNATIVE (is this faster if only repr is needed?)
    Inkscape::XML::Node *node = sp_repr_lookup_name((doc->root)->repr, name);
    */
    Inkscape::XML::Node * node = sp_desktop_document(desk)->getObjectById(name)->repr;
    if (!node)
    {
        g_set_error(error, INKSCAPE_ERROR, INKSCAPE_ERROR_OBJECT, "Object '%s' not found in document.", name);
        return NULL;
    }
    return node;
}

/* 
 * See comment for get_repr_by_name, above.
 */
SPObject *
get_object_by_name (SPDesktop *desk, gchar *name, GError **error)
{
    SPObject * obj = sp_desktop_document(desk)->getObjectById(name);
    if (!obj)
    {
        g_set_error(error, INKSCAPE_ERROR, INKSCAPE_ERROR_OBJECT, "Object '%s' not found in document.", name);
        return NULL;
    }
    return obj;
}

/*
 * Tests for NULL strings and throws an appropriate error.
 * Every method that takes a string parameter (other than the 
 * name of an object, that's tested seperatly) should call this.
 */
gboolean
dbus_check_string (gchar *string, GError ** error, const gchar * errorstr)
{
    if (string == NULL)
    {
        g_set_error(error, INKSCAPE_ERROR, INKSCAPE_ERROR_OTHER, "%s", errorstr);
        return FALSE;
    }
    return TRUE;
}

/* 
 * This is used to return object values to the user
 */
const gchar *
get_name_from_object (SPObject * obj)
{
    return obj->repr->attribute("id"); 
}

/*
 * Some verbs (cut, paste) only work on the active layer.
 * This makes sure that the document that is about to recive a command is active.
 */
void
desktop_ensure_active (SPDesktop* desk) {
    if (desk != SP_ACTIVE_DESKTOP)
        inkscape_activate_desktop (desk);
    return;
}

gdouble
selection_get_center_x (Inkscape::Selection *sel){
    NRRect *box = g_new(NRRect, 1);;
    box = sel->boundsInDocument(box);
    return box->x0 + ((box->x1 - box->x0)/2);
}

gdouble
selection_get_center_y (Inkscape::Selection *sel){
    NRRect *box = g_new(NRRect, 1);;
    box = sel->boundsInDocument(box);
    return box->y0 + ((box->y1 - box->y0)/2);
}

/* 
 * This function is used along with selection_restore to
 * take advantage of functionality provided by a selection
 * for a single object.
 *
 * It saves the current selection and sets the selection to 
 * the object specified.  Any selection verb can be used on the
 * object and then selection_restore is called, restoring the 
 * original selection.
 *
 * This should be mostly transparent to the user who need never
 * know we never bothered to implement it seperatly.  Although
 * they might see the selection box flicker if used in a loop.
 */
const GSList *
selection_swap(SPDesktop *desk, gchar *name, GError **error)
{
    Inkscape::Selection *sel = sp_desktop_selection(desk);
    const GSList *oldsel = g_slist_copy((GSList *)sel->list());
    
    sel->set(get_object_by_name(desk, name, error));
    return oldsel;
}

/*
 * See selection_swap, above
 */
void
selection_restore(SPDesktop *desk, const GSList * oldsel)
{
    Inkscape::Selection *sel = sp_desktop_selection(desk);
    sel->setList(oldsel);
}

/*
 * Shortcut for creating a Node.
 */
Inkscape::XML::Node *
dbus_create_node (SPDesktop *desk, const gchar *type)
{
    SPDocument * doc = sp_desktop_document (desk);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

    return xml_doc->createElement(type);
}

/*
 * Called by the shape creation functions.  Gets the default style for the doc
 * or sets it arbitrarily if none.
 *
 * There is probably a better way to do this (use the shape tools default styles)
 * but I'm not sure how.
 */
gchar *
finish_create_shape (DocumentInterface *object, GError **error, Inkscape::XML::Node *newNode, gchar *desc)
{

    SPCSSAttr *style = sp_desktop_get_style(object->desk, TRUE);
    
    if (style) {
        newNode->setAttribute("style", sp_repr_css_write_string(style), TRUE);
    }
    else {
        newNode->setAttribute("style", "fill:#0000ff;fill-opacity:1;stroke:#c900b9;stroke-width:0;stroke-miterlimit:0;stroke-opacity:1;stroke-dasharray:none", TRUE);
    }

    object->desk->currentLayer()->appendChildRepr(newNode);
    object->desk->currentLayer()->updateRepr();

    if (object->updates)
        sp_document_done(sp_desktop_document(object->desk), 0, (gchar *)desc);
    //else
        //document_interface_pause_updates(object, error);

    return strdup(newNode->attribute("id"));
}

/*
 * This is the code used internally to call all the verbs.
 *
 * It handles error reporting and update pausing (which needs some work.)
 * This is a good place to improve efficiency as it is called a lot.
 *
 * document_interface_call_verb is similar but is called by the user.
 */
gboolean
dbus_call_verb (DocumentInterface *object, int verbid, GError **error)
{    
    SPDesktop *desk2 = object->desk;
    desktop_ensure_active (desk2);
    
    if ( desk2 ) {
        Inkscape::Verb *verb = Inkscape::Verb::get( verbid );
        if ( verb ) {
            SPAction *action = verb->get_action(desk2);
            if ( action ) {
                //if (!object->updates)
                    //document_interface_pause_updates (object, error);
                sp_action_perform( action, NULL );
                if (object->updates)
                    sp_document_done(sp_desktop_document(desk2), verb->get_code(), g_strdup(verb->get_tip()));
                //if (!object->updates)
                    //document_interface_pause_updates (object, error);
                return TRUE;
            }
        }
    }
    g_set_error(error, INKSCAPE_ERROR, INKSCAPE_ERROR_VERB, "Verb failed to execute");
    return FALSE;
}

/****************************************************************************
     DOCUMENT INTERFACE CLASS STUFF
****************************************************************************/

G_DEFINE_TYPE(DocumentInterface, document_interface, G_TYPE_OBJECT)

static void
document_interface_finalize (GObject *object)
{
        G_OBJECT_CLASS (document_interface_parent_class)->finalize (object);
}


static void
document_interface_class_init (DocumentInterfaceClass *klass)
{
        GObjectClass *object_class;
        object_class = G_OBJECT_CLASS (klass);
        object_class->finalize = document_interface_finalize;
}

static void
document_interface_init (DocumentInterface *object)
{
	object->desk = NULL;
}


DocumentInterface *
document_interface_new (void)
{
        return (DocumentInterface*)g_object_new (TYPE_DOCUMENT_INTERFACE, NULL);
}

/* 
 * Error stuff...
 *
 * To add a new error type, edit here and in the .h InkscapeError enum.
 */
GQuark
inkscape_error_quark (void)
{
  static GQuark quark = 0;
  if (!quark)
    quark = g_quark_from_static_string ("inkscape_error");

  return quark;
}

#define ENUM_ENTRY(NAME, DESC) { NAME, "" #NAME "", DESC }

GType
inkscape_error_get_type (void)
{
	static GType etype = 0;

	if (etype == 0)
	{
		static const GEnumValue values[] =
		{

			ENUM_ENTRY (INKSCAPE_ERROR_SELECTION, "Incompatible_Selection"),
			ENUM_ENTRY (INKSCAPE_ERROR_OBJECT, "Incompatible_Object"),
			ENUM_ENTRY (INKSCAPE_ERROR_VERB, "Failed_Verb"),
			ENUM_ENTRY (INKSCAPE_ERROR_OTHER, "Generic_Error"),
			{ 0, 0, 0 }
		};

		etype = g_enum_register_static ("InkscapeError", values);
	}

	return etype;
}

/****************************************************************************
     MISC FUNCTIONS
****************************************************************************/

gboolean
document_interface_delete_all (DocumentInterface *object, GError **error)
{
    sp_edit_clear_all (object->desk);
    return TRUE;
}

gboolean
document_interface_call_verb (DocumentInterface *object, gchar *verbid, GError **error)
{
    SPDesktop *desk2 = object->desk;
    desktop_ensure_active (object->desk);
    if ( desk2 ) {
        Inkscape::Verb *verb = Inkscape::Verb::getbyid( verbid );
        if ( verb ) {
            SPAction *action = verb->get_action(desk2);
            if ( action ) {
                sp_action_perform( action, NULL );
                if (object->updates) {
                    sp_document_done(sp_desktop_document(desk2), verb->get_code(), g_strdup(verb->get_tip()));
                }
            }
        }
    }
    g_set_error(error, INKSCAPE_ERROR, INKSCAPE_ERROR_VERB, "Verb '%s' failed to execute or was not found.", verbid);
    return FALSE;
}


/****************************************************************************
     CREATION FUNCTIONS
****************************************************************************/

gchar* 
document_interface_rectangle (DocumentInterface *object, int x, int y, 
                              int width, int height, GError **error)
{


    Inkscape::XML::Node *newNode = dbus_create_node(object->desk, "svg:rect");
    sp_repr_set_int(newNode, "x", x);  //could also use newNode->setAttribute()
    sp_repr_set_int(newNode, "y", y);
    sp_repr_set_int(newNode, "width", width);
    sp_repr_set_int(newNode, "height", height);
    return finish_create_shape (object, error, newNode, (gchar *)"create rectangle");
}

gchar*
document_interface_ellipse_center (DocumentInterface *object, int cx, int cy, 
                                   int rx, int ry, GError **error)
{
    Inkscape::XML::Node *newNode = dbus_create_node(object->desk, "svg:path");
    newNode->setAttribute("sodipodi:type", "arc");
    sp_repr_set_int(newNode, "sodipodi:cx", cx);
    sp_repr_set_int(newNode, "sodipodi:cy", cy);
    sp_repr_set_int(newNode, "sodipodi:rx", rx);
    sp_repr_set_int(newNode, "sodipodi:ry", ry);
    return finish_create_shape (object, error, newNode, (gchar *)"create circle");
}

gchar* 
document_interface_polygon (DocumentInterface *object, int cx, int cy, 
                            int radius, int rotation, int sides, 
                            GError **error)
{
    gdouble rot = ((rotation / 180.0) * 3.14159265) - ( 3.14159265 / 2.0);
    Inkscape::XML::Node *newNode = dbus_create_node(object->desk, "svg:path");
    newNode->setAttribute("inkscape:flatsided", "true");
    newNode->setAttribute("sodipodi:type", "star");
    sp_repr_set_int(newNode, "sodipodi:cx", cx);
    sp_repr_set_int(newNode, "sodipodi:cy", cy);
    sp_repr_set_int(newNode, "sodipodi:r1", radius);
    sp_repr_set_int(newNode, "sodipodi:r2", radius);
    sp_repr_set_int(newNode, "sodipodi:sides", sides);
    sp_repr_set_int(newNode, "inkscape:randomized", 0);
    sp_repr_set_svg_double(newNode, "sodipodi:arg1", rot);
    sp_repr_set_svg_double(newNode, "sodipodi:arg2", rot);
    sp_repr_set_svg_double(newNode, "inkscape:rounded", 0);

    return finish_create_shape (object, error, newNode, (gchar *)"create polygon");
}

gchar* 
document_interface_star (DocumentInterface *object, int cx, int cy, 
                         int r1, int r2, int sides, gdouble rounded,
                         gdouble arg1, gdouble arg2, GError **error)
{
    Inkscape::XML::Node *newNode = dbus_create_node(object->desk, "svg:path");
    newNode->setAttribute("inkscape:flatsided", "false");
    newNode->setAttribute("sodipodi:type", "star");
    sp_repr_set_int(newNode, "sodipodi:cx", cx);
    sp_repr_set_int(newNode, "sodipodi:cy", cy);
    sp_repr_set_int(newNode, "sodipodi:r1", r1);
    sp_repr_set_int(newNode, "sodipodi:r2", r2);
    sp_repr_set_int(newNode, "sodipodi:sides", sides);
    sp_repr_set_int(newNode, "inkscape:randomized", 0);
    sp_repr_set_svg_double(newNode, "sodipodi:arg1", arg1);
    sp_repr_set_svg_double(newNode, "sodipodi:arg2", arg2);
    sp_repr_set_svg_double(newNode, "inkscape:rounded", rounded);

    return finish_create_shape (object, error, newNode, (gchar *)"create star");
}

gchar* 
document_interface_ellipse (DocumentInterface *object, int x, int y, 
                            int width, int height, GError **error)
{
    int rx = width/2;
    int ry = height/2;
    return document_interface_ellipse_center (object, x+rx, y+ry, rx, ry, error);
}

gchar* 
document_interface_line (DocumentInterface *object, int x, int y, 
                              int x2, int y2, GError **error)
{
    Inkscape::XML::Node *newNode = dbus_create_node(object->desk, "svg:path");
    std::stringstream out;
    // Not sure why this works.
	out << "m " << x << "," << y << " " << x2 - x << "," << y2 - y;
    newNode->setAttribute("d", out.str().c_str());
    return finish_create_shape (object, error, newNode, (gchar *)"create line");
}

gchar* 
document_interface_spiral (DocumentInterface *object, int cx, int cy, 
                           int r, int revolutions, GError **error)
{
    Inkscape::XML::Node *newNode = dbus_create_node(object->desk, "svg:path");
    newNode->setAttribute("sodipodi:type", "spiral");
    sp_repr_set_int(newNode, "sodipodi:cx", cx);
    sp_repr_set_int(newNode, "sodipodi:cy", cy);
    sp_repr_set_int(newNode, "sodipodi:radius", r);
    sp_repr_set_int(newNode, "sodipodi:revolution", revolutions);
    sp_repr_set_int(newNode, "sodipodi:t0", 0);
    sp_repr_set_int(newNode, "sodipodi:argument", 0);
    sp_repr_set_int(newNode, "sodipodi:expansion", 1);
    gchar * retval = finish_create_shape (object, error, newNode, (gchar *)"create spiral");
    //Makes sure there is no fill for spirals by default.
    gchar* newString = g_strconcat(newNode->attribute("style"), ";fill:none", NULL);
    newNode->setAttribute("style", newString);
    g_free(newString);
    return retval;
}

gboolean
document_interface_text (DocumentInterface *object, int x, int y, gchar *text, GError **error)
{
    //FIXME: Not selectable (aka broken).  Needs to be rewritten completely.

    SPDesktop *desktop = object->desk;
    SPCanvasText * canvas_text = (SPCanvasText *) sp_canvastext_new(sp_desktop_tempgroup(desktop), desktop, Geom::Point(0,0), "");
    sp_canvastext_set_text (canvas_text, text);
    sp_canvastext_set_coords (canvas_text, x, y);

    return TRUE;
}

gchar *
document_interface_image (DocumentInterface *object, int x, int y, gchar *filename, GError **error)
{
    gchar * uri = g_filename_to_uri (filename, FALSE, error);
    if (!uri)
        return FALSE;
    
    Inkscape::XML::Node *newNode = dbus_create_node(object->desk, "svg:image");
    sp_repr_set_int(newNode, "x", x);
    sp_repr_set_int(newNode, "y", y);
    newNode->setAttribute("xlink:href", uri);
    
    object->desk->currentLayer()->appendChildRepr(newNode);
    object->desk->currentLayer()->updateRepr();

    if (object->updates)
        sp_document_done(sp_desktop_document(object->desk), 0, "Imported bitmap.");

    //g_free(uri);
    return strdup(newNode->attribute("id"));
}

gchar* 
document_interface_node (DocumentInterface *object, gchar *type, GError **error)
{
    SPDocument * doc = sp_desktop_document (object->desk);
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

    Inkscape::XML::Node *newNode =  xml_doc->createElement(type);

    object->desk->currentLayer()->appendChildRepr(newNode);
    object->desk->currentLayer()->updateRepr();

    if (object->updates)
        sp_document_done(sp_desktop_document(object->desk), 0, (gchar *)"created empty node");
    //else
        //document_interface_pause_updates(object, error);

    return strdup(newNode->attribute("id"));
}

/****************************************************************************
     ENVIORNMENT FUNCTIONS
****************************************************************************/
gdouble
document_interface_document_get_width (DocumentInterface *object)
{
    return sp_document_width(sp_desktop_document(object->desk));
}

gdouble
document_interface_document_get_height (DocumentInterface *object)
{
    return sp_document_height(sp_desktop_document(object->desk));
}

gchar *
document_interface_document_get_css (DocumentInterface *object, GError **error)
{
    SPCSSAttr *current = (object->desk)->current;
    return sp_repr_css_write_string(current);
}

gboolean 
document_interface_document_merge_css (DocumentInterface *object,
                                       gchar *stylestring, GError **error)
{
    SPCSSAttr * style = sp_repr_css_attr_new();
    sp_repr_css_attr_add_from_string (style, stylestring);
    sp_desktop_set_style (object->desk, style);
    return TRUE;
}

gboolean 
document_interface_document_set_css (DocumentInterface *object,
                                     gchar *stylestring, GError **error)
{
    SPCSSAttr * style = sp_repr_css_attr_new();
    sp_repr_css_attr_add_from_string (style, stylestring);
    //Memory leak?
    object->desk->current = style;
    return TRUE;
}

gboolean 
document_interface_document_resize_to_fit_selection (DocumentInterface *object,
                                                     GError **error)
{
    return dbus_call_verb (object, SP_VERB_FIT_CANVAS_TO_SELECTION, error);
    return TRUE;
}

/****************************************************************************
     OBJECT FUNCTIONS
****************************************************************************/

gboolean
document_interface_set_attribute (DocumentInterface *object, char *shape, 
                                  char *attribute, char *newval, GError **error)
{
    Inkscape::XML::Node *newNode = get_repr_by_name(object->desk, shape, error);

    /* ALTERNATIVE (is this faster?)
    Inkscape::XML::Node *newnode = sp_repr_lookup_name((doc->root)->repr, name);
    */
    if (!dbus_check_string(newval, error, "New value string was empty."))
        return FALSE;
        
    if (!newNode)
        return FALSE;
        
    newNode->setAttribute(attribute, newval, TRUE);
    return TRUE;
}

gboolean 
document_interface_set_int_attribute (DocumentInterface *object, 
                                      char *shape, char *attribute, 
                                      int newval, GError **error)
{
    Inkscape::XML::Node *newNode = get_repr_by_name (object->desk, shape, error);
    if (!newNode)
        return FALSE;
        
    sp_repr_set_int (newNode, attribute, newval);
    return TRUE;
}


gboolean
document_interface_set_double_attribute (DocumentInterface *object, 
                                         char *shape, char *attribute, 
                                         double newval, GError **error)
{
    Inkscape::XML::Node *newNode = get_repr_by_name (object->desk, shape, error);
    
    if (!dbus_check_string (attribute, error, "New value string was empty."))
        return FALSE;
    if (!newNode)
        return FALSE;
    
    sp_repr_set_svg_double (newNode, attribute, newval);
    return TRUE;
}

gchar *
document_interface_get_attribute (DocumentInterface *object, char *shape, 
                                  char *attribute, GError **error)
{
    Inkscape::XML::Node *newNode = get_repr_by_name(object->desk, shape, error);

    if (!dbus_check_string (attribute, error, "Attribute name empty."))
        return NULL;
    if (!newNode)
        return NULL;
        
    return g_strdup(newNode->attribute(attribute));
}

gboolean
document_interface_move (DocumentInterface *object, gchar *name, gdouble x, 
                         gdouble y, GError **error)
{
    const GSList *oldsel = selection_swap(object->desk, name, error);
    if (!oldsel)
        return FALSE;
    sp_selection_move (object->desk, x, 0 - y);
    selection_restore(object->desk, oldsel);
    return TRUE;
}

gboolean
document_interface_move_to (DocumentInterface *object, gchar *name, gdouble x, 
                         gdouble y, GError **error)
{
    const GSList *oldsel = selection_swap(object->desk, name, error);
    if (!oldsel)
        return FALSE;
    Inkscape::Selection * sel = sp_desktop_selection(object->desk);
    sp_selection_move (object->desk, x - selection_get_center_x(sel),
                                     0 - (y - selection_get_center_y(sel)));
    selection_restore(object->desk, oldsel);
    return TRUE;
}

gboolean
document_interface_object_to_path (DocumentInterface *object, 
                                   char *shape, GError **error)
{
    const GSList *oldsel = selection_swap(object->desk, shape, error);
    if (!oldsel)
        return FALSE;
    dbus_call_verb (object, SP_VERB_OBJECT_TO_CURVE, error);
    selection_restore(object->desk, oldsel);
    return TRUE;
}

gchar *
document_interface_get_path (DocumentInterface *object, char *pathname, GError **error)
{
    Inkscape::XML::Node *node = get_repr_by_name(object->desk, pathname, error);
    
    if (!node)
        return NULL;
        
    if (node->attribute("d") == NULL)
    {
        g_set_error(error, INKSCAPE_ERROR, INKSCAPE_ERROR_OBJECT, "Object is not a path.");
        return NULL;
    }
    return strdup(node->attribute("d"));
}

gboolean 
document_interface_transform (DocumentInterface *object, gchar *shape,
                              gchar *transformstr, GError **error)
{
    //FIXME: This should merge transformations.
    gchar trans[] = "transform";
    document_interface_set_attribute (object, shape, trans, transformstr, error);
    return TRUE;
}

gchar *
document_interface_get_css (DocumentInterface *object, gchar *shape,
                            GError **error)
{
    gchar style[] = "style";
    return document_interface_get_attribute (object, shape, style, error);
}

gboolean 
document_interface_modify_css (DocumentInterface *object, gchar *shape,
                               gchar *cssattrb, gchar *newval, GError **error)
{
    // Doesn't like non-variable strings for some reason.
    gchar style[] = "style";
    Inkscape::XML::Node *node = get_repr_by_name(object->desk, shape, error);
    
    if (!dbus_check_string (cssattrb, error, "Attribute string empty."))
        return FALSE;
    if (!node)
        return FALSE;
        
    SPCSSAttr * oldstyle = sp_repr_css_attr (node, style);
    sp_repr_css_set_property(oldstyle, cssattrb, newval);
    node->setAttribute (style, sp_repr_css_write_string (oldstyle), TRUE);
    return TRUE;
}

gboolean 
document_interface_merge_css (DocumentInterface *object, gchar *shape,
                               gchar *stylestring, GError **error)
{
    gchar style[] = "style";
    
    Inkscape::XML::Node *node = get_repr_by_name(object->desk, shape, error);
    
    if (!dbus_check_string (stylestring, error, "Style string empty."))
        return FALSE;
    if (!node)
        return FALSE;
        
    SPCSSAttr * newstyle = sp_repr_css_attr_new();
    sp_repr_css_attr_add_from_string (newstyle, stylestring);

    SPCSSAttr * oldstyle = sp_repr_css_attr (node, style);

    sp_repr_css_merge(oldstyle, newstyle);
    node->setAttribute (style, sp_repr_css_write_string (oldstyle), TRUE);
    return TRUE;
}

gboolean 
document_interface_set_color (DocumentInterface *object, gchar *shape,
                              int r, int g, int b, gboolean fill, GError **error)
{
    gchar style[15];
    if (r<0 || r>255 || g<0 || g>255 || b<0 || b>255)
    {
        g_set_error(error, INKSCAPE_ERROR, INKSCAPE_ERROR_OTHER, "Given (%d,%d,%d).  All values must be between 0-255 inclusive.", r, g, b);
        return FALSE;
    }
    
    if (fill)
        snprintf(style, 15, "fill:#%.2x%.2x%.2x", r, g, b);
    else
        snprintf(style, 15, "stroke:#%.2x%.2x%.2x", r, g, b);
    
    if (strcmp(shape, "document") == 0)
        return document_interface_document_merge_css (object, style, error);
    
    return document_interface_merge_css (object, shape, style, error);
}

gboolean 
document_interface_move_to_layer (DocumentInterface *object, gchar *shape, 
                              gchar *layerstr, GError **error)
{
    const GSList *oldsel = selection_swap(object->desk, shape, error);
    if (!oldsel)
        return FALSE;
        
    document_interface_selection_move_to_layer(object, layerstr, error);
    selection_restore(object->desk, oldsel);
    return TRUE;
}

GArray *
document_interface_get_node_coordinates (DocumentInterface *object, gchar *shape)
{
    //FIXME: Needs lot's of work.
/*
    Inkscape::XML::Node *shapenode = get_repr_by_name (object->desk, shape, error);
    if (shapenode == NULL || shapenode->attribute("d") == NULL) {
        return FALSE;
    }
    char * path = strdup(shapenode->attribute("d"));
    printf("PATH: %s\n", path);
    
    Geom::parse_svg_path (path);
    return NULL;
    */
    return NULL;
}


/****************************************************************************
     FILE I/O FUNCTIONS
****************************************************************************/

gboolean 
document_interface_save (DocumentInterface *object, GError **error)
{
    SPDocument * doc = sp_desktop_document(object->desk);
    printf("1:  %s\n2:  %s\n3:  %s\n", doc->uri, doc->base, doc->name);
    if (doc->uri)
        return document_interface_save_as (object, doc->uri, error);
    return FALSE;
}

gboolean 
document_interface_load (DocumentInterface *object, 
                        gchar *filename, GError **error)
{
    desktop_ensure_active (object->desk);
    const Glib::ustring file(filename);
    sp_file_open(file, NULL, TRUE, TRUE);
    if (object->updates)
        sp_document_done(sp_desktop_document(object->desk), SP_VERB_FILE_OPEN, "Opened File");
    return TRUE;
}

gboolean 
document_interface_save_as (DocumentInterface *object, 
                           gchar *filename, GError **error)
{
    SPDocument * doc = sp_desktop_document(object->desk);
    #ifdef WITH_GNOME_VFS
    const Glib::ustring file(filename);
    return file_save_remote(doc, file, NULL, TRUE, TRUE);
    #endif
    if (!doc || strlen(filename)<1) //Safety check
        return false;

    try {
        Inkscape::Extension::save(NULL, doc, filename,
                 false, false, true, Inkscape::Extension::FILE_SAVE_METHOD_SAVE_AS);
    } catch (...) {
        //SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::ERROR_MESSAGE, _("Document not saved."));
        return false;
    }

    //SP_ACTIVE_DESKTOP->event_log->rememberFileSave();
    //SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::NORMAL_MESSAGE, "Document saved.");
    return true;
}

gboolean
document_interface_mark_as_unmodified (DocumentInterface *object, GError **error)
{
    SPDocument * doc = sp_desktop_document(object->desk);
    if (doc)
        doc->modified_since_save = FALSE;
    return TRUE;
}

/*
gboolean 
document_interface_print_to_file (DocumentInterface *object, GError **error)
{
    SPDocument * doc = sp_desktop_document(object->desk);
    sp_print_document_to_file (doc, g_strdup("/home/soren/test.pdf"));
                               
    return TRUE;
}
*/
/****************************************************************************
     PROGRAM CONTROL FUNCTIONS
****************************************************************************/

gboolean
document_interface_close (DocumentInterface *object, GError **error)
{
    return dbus_call_verb (object, SP_VERB_FILE_CLOSE_VIEW, error);
}

gboolean
document_interface_exit (DocumentInterface *object, GError **error)
{
    return dbus_call_verb (object, SP_VERB_FILE_QUIT, error);
}

gboolean
document_interface_undo (DocumentInterface *object, GError **error)
{
    return dbus_call_verb (object, SP_VERB_EDIT_UNDO, error);
}

gboolean
document_interface_redo (DocumentInterface *object, GError **error)
{
    return dbus_call_verb (object, SP_VERB_EDIT_REDO, error);
}



/****************************************************************************
     UPDATE FUNCTIONS 
     FIXME: This would work better by adding a flag to SPDesktop to prevent
     updating but that would be very intrusive so for now there is a workaround.
     Need to make sure it plays well with verbs because they are used so much.
****************************************************************************/

void
document_interface_pause_updates (DocumentInterface *object, GError **error)
{
    object->updates = FALSE;
    object->desk->canvas->drawing_disabled = 1;
    //object->desk->canvas->need_redraw = 0;
    //object->desk->canvas->need_repick = 0;
    //sp_desktop_document(object->desk)->root->uflags = FALSE;
    //sp_desktop_document(object->desk)->root->mflags = FALSE;
}

void
document_interface_resume_updates (DocumentInterface *object, GError **error)
{
    object->updates = TRUE;
    object->desk->canvas->drawing_disabled = 0;
    //object->desk->canvas->need_redraw = 1;
    //object->desk->canvas->need_repick = 1;
    //sp_desktop_document(object->desk)->root->uflags = TRUE;
    //sp_desktop_document(object->desk)->root->mflags = TRUE;
    //sp_desktop_document(object->desk)->_updateDocument();
    //FIXME: use better verb than rect.
    sp_document_done(sp_desktop_document(object->desk), SP_VERB_CONTEXT_RECT, "Multiple actions");
}

void
document_interface_update (DocumentInterface *object, GError **error)
{
    sp_desktop_document(object->desk)->root->uflags = TRUE;
    sp_desktop_document(object->desk)->root->mflags = TRUE;
    object->desk->enableInteraction();
    sp_desktop_document(object->desk)->_updateDocument();
    object->desk->disableInteraction();
    sp_desktop_document(object->desk)->root->uflags = FALSE;
    sp_desktop_document(object->desk)->root->mflags = FALSE;
    //sp_document_done(sp_desktop_document(object->desk), SP_VERB_CONTEXT_RECT, "Multiple actions");
}

/****************************************************************************
     SELECTION FUNCTIONS FIXME: use call_verb where appropriate (once update system is tested.)
****************************************************************************/

gboolean
document_interface_selection_get (DocumentInterface *object, char ***out, GError **error)
{
    Inkscape::Selection * sel = sp_desktop_selection(object->desk);
    GSList const *oldsel = sel->list();

    int size = g_slist_length((GSList *) oldsel);

    *out = g_new0 (char *, size + 1);

    int i = 0;
    for (GSList const *iter = oldsel; iter != NULL; iter = iter->next) {
        (*out)[i] = g_strdup(SP_OBJECT(iter->data)->repr->attribute("id"));
        i++;
    }
    (*out)[i] = NULL;

    return TRUE;
}

gboolean
document_interface_selection_add (DocumentInterface *object, char *name, GError **error)
{
    SPObject * obj = get_object_by_name(object->desk, name, error);
    if (!obj)
        return FALSE;
    
    Inkscape::Selection *selection = sp_desktop_selection(object->desk);

    selection->add(obj);
    return TRUE;
}

gboolean
document_interface_selection_add_list (DocumentInterface *object, 
                                       char **names, GError **error)
{
    int i;
    for (i=0;names[i] != NULL;i++) {
        document_interface_selection_add(object, names[i], error);       
    }
    return TRUE;
}

gboolean
document_interface_selection_set (DocumentInterface *object, char *name, GError **error)
{
    SPDocument * doc = sp_desktop_document (object->desk);
    Inkscape::Selection *selection = sp_desktop_selection(object->desk);
    selection->set(doc->getObjectById(name));
    return TRUE;
}

gboolean
document_interface_selection_set_list (DocumentInterface *object, 
                                       gchar **names, GError **error)
{
    sp_desktop_selection(object->desk)->clear();
    int i;
    for (i=0;names[i] != NULL;i++) {
        document_interface_selection_add(object, names[i], error);       
    }
    return TRUE;
}

gboolean
document_interface_selection_rotate (DocumentInterface *object, int angle, GError **error)
{
    Inkscape::Selection *selection = sp_desktop_selection(object->desk);
    sp_selection_rotate(selection, angle);
    return TRUE;
}

gboolean
document_interface_selection_delete (DocumentInterface *object, GError **error)
{
    //sp_selection_delete (object->desk);
    return dbus_call_verb (object, SP_VERB_EDIT_DELETE, error);
}

gboolean
document_interface_selection_clear (DocumentInterface *object, GError **error)
{
    sp_desktop_selection(object->desk)->clear();
    return TRUE;
}

gboolean
document_interface_select_all (DocumentInterface *object, GError **error)
{
    //sp_edit_select_all (object->desk);
    return dbus_call_verb (object, SP_VERB_EDIT_SELECT_ALL, error);
}

gboolean
document_interface_select_all_in_all_layers(DocumentInterface *object, 
                                            GError **error)
{
    //sp_edit_select_all_in_all_layers (object->desk);
    return dbus_call_verb (object, SP_VERB_EDIT_SELECT_ALL_IN_ALL_LAYERS, error);
}

gboolean
document_interface_selection_box (DocumentInterface *object, int x, int y,
                                  int x2, int y2, gboolean replace, 
                                  GError **error)
{
    //FIXME: implement.
    return FALSE;
}

gboolean
document_interface_selection_invert (DocumentInterface *object, GError **error)
{
    //sp_edit_invert (object->desk);
    return dbus_call_verb (object, SP_VERB_EDIT_INVERT, error);
}

gboolean
document_interface_selection_group (DocumentInterface *object, GError **error)
{
    //sp_selection_group (object->desk);
    return dbus_call_verb (object, SP_VERB_SELECTION_GROUP, error);
}
gboolean
document_interface_selection_ungroup (DocumentInterface *object, GError **error)
{
    //sp_selection_ungroup (object->desk);
    return dbus_call_verb (object, SP_VERB_SELECTION_UNGROUP, error);
}
 
gboolean
document_interface_selection_cut (DocumentInterface *object, GError **error)
{
    //desktop_ensure_active (object->desk);
    //sp_selection_cut (object->desk);
    return dbus_call_verb (object, SP_VERB_EDIT_CUT, error);
}

gboolean
document_interface_selection_copy (DocumentInterface *object, GError **error)
{
    //desktop_ensure_active (object->desk);
    //sp_selection_copy ();
    return dbus_call_verb (object, SP_VERB_EDIT_COPY, error);
}
/*
gboolean
document_interface_selection_paste (DocumentInterface *object, GError **error)
{
    desktop_ensure_active (object->desk);
                    if (!object->updates)
                    document_interface_pause_updates (object, error);
    sp_selection_paste (object->desk, TRUE);
                    if (!object->updates)
                    document_interface_pause_updates (object, error);
    return TRUE;
    //return dbus_call_verb (object, SP_VERB_EDIT_PASTE, error);
}
*/
gboolean
document_interface_selection_paste (DocumentInterface *object, GError **error)
{
    return dbus_call_verb (object, SP_VERB_EDIT_PASTE, error);
}

gboolean
document_interface_selection_scale (DocumentInterface *object, gdouble grow, GError **error)
{
    Inkscape::Selection *selection = sp_desktop_selection(object->desk);
    if (!selection)
    {
        return FALSE;
    }     
    sp_selection_scale (selection, grow);
    return TRUE;
}

gboolean
document_interface_selection_move (DocumentInterface *object, gdouble x, gdouble y, GError **error)
{
    sp_selection_move (object->desk, x, 0 - y); //switching coordinate systems.
    return TRUE;
}

gboolean
document_interface_selection_move_to (DocumentInterface *object, gdouble x, gdouble y, GError **error)
{
    Inkscape::Selection * sel = sp_desktop_selection(object->desk);

    Geom::OptRect sel_bbox = sel->bounds();
    if (sel_bbox) {
        Geom::Point m( x - selection_get_center_x(sel) , 0 - (y - selection_get_center_y(sel)) );
        sp_selection_move_relative(sel, m, true);
    }
    return TRUE;
}

//FIXME: does not paste in new layer.
// This needs to use lower level cut_impl and paste_impl (messy)
// See the built-in sp_selection_to_next_layer and duplicate.
gboolean 
document_interface_selection_move_to_layer (DocumentInterface *object,
                                            gchar *layerstr, GError **error)
{
    SPDesktop * dt = object->desk;

    Inkscape::Selection *selection = sp_desktop_selection(dt);

    // check if something is selected
    if (selection->isEmpty())
        return FALSE;

    SPObject *next = get_object_by_name(object->desk, layerstr, error);
    
    if (!next)
        return FALSE;

    if (strcmp("layer", (next->repr)->attribute("inkscape:groupmode")) == 0) {

        sp_selection_cut(dt);

        dt->setCurrentLayer(next);

        sp_selection_paste(dt, TRUE);
        }
    return TRUE;
}

GArray *
document_interface_selection_get_center (DocumentInterface *object)
{
    Inkscape::Selection * sel = sp_desktop_selection(object->desk);

    if (sel) 
    {
        gdouble x = selection_get_center_x(sel);
        gdouble y = selection_get_center_y(sel);
        GArray * intArr = g_array_new (TRUE, TRUE, sizeof(double));

        g_array_append_val (intArr, x);
        g_array_append_val (intArr, y);
        return intArr;
    }

    return NULL;
}

gboolean 
document_interface_selection_to_path (DocumentInterface *object, GError **error)
{
    return dbus_call_verb (object, SP_VERB_OBJECT_TO_CURVE, error);    
}


gchar *
document_interface_selection_combine (DocumentInterface *object, gchar *cmd,
                                      GError **error)
{
    if (strcmp(cmd, "union") == 0)
        dbus_call_verb (object, SP_VERB_SELECTION_UNION, error);
    else if (strcmp(cmd, "intersection") == 0)
        dbus_call_verb (object, SP_VERB_SELECTION_INTERSECT, error);
    else if (strcmp(cmd, "difference") == 0)
        dbus_call_verb (object, SP_VERB_SELECTION_DIFF, error);
    else if (strcmp(cmd, "exclusion") == 0)
        dbus_call_verb (object, SP_VERB_SELECTION_SYMDIFF, error);
    else
        return NULL;

    if (sp_desktop_selection(object->desk)->singleRepr() != NULL)
        return g_strdup((sp_desktop_selection(object->desk)->singleRepr())->attribute("id"));
    return NULL;
}

gboolean
document_interface_selection_divide (DocumentInterface *object, char ***out, GError **error)
{
    dbus_call_verb (object, SP_VERB_SELECTION_CUT, error);

    return document_interface_selection_get (object, out, error);
}

gboolean
document_interface_selection_change_level (DocumentInterface *object, gchar *cmd,
                                      GError **error)
{
    if (strcmp(cmd, "raise") == 0)
        return dbus_call_verb (object, SP_VERB_SELECTION_RAISE, error);
    if (strcmp(cmd, "lower") == 0)
        return dbus_call_verb (object, SP_VERB_SELECTION_LOWER, error);
    if ((strcmp(cmd, "to_top") == 0) || (strcmp(cmd, "to_front") == 0))
        return dbus_call_verb (object, SP_VERB_SELECTION_TO_FRONT, error);
    if ((strcmp(cmd, "to_bottom") == 0) || (strcmp(cmd, "to_back") == 0))
        return dbus_call_verb (object, SP_VERB_SELECTION_TO_BACK, error);
    return TRUE;
}

/****************************************************************************
     LAYER FUNCTIONS
****************************************************************************/

gchar *
document_interface_layer_new (DocumentInterface *object, GError **error)
{
    SPDesktop * dt = object->desk;
    SPObject *new_layer = Inkscape::create_layer(dt->currentRoot(), dt->currentLayer(), Inkscape::LPOS_BELOW);
    dt->setCurrentLayer(new_layer);
    return g_strdup(get_name_from_object (new_layer));
}

gboolean 
document_interface_layer_set (DocumentInterface *object,
                              gchar *layerstr, GError **error)
{
    SPObject * obj = get_object_by_name (object->desk, layerstr, error);
    
    if (!obj)
        return FALSE;
        
    object->desk->setCurrentLayer (obj);
    return TRUE;
}

gchar **
document_interface_layer_get_all (DocumentInterface *object)
{
    //FIXME: implement.
    return NULL;
}

gboolean 
document_interface_layer_change_level (DocumentInterface *object,
                                       gchar *cmd, GError **error)
{
    if (strcmp(cmd, "raise") == 0)
        return dbus_call_verb (object, SP_VERB_LAYER_RAISE, error);
    if (strcmp(cmd, "lower") == 0)
        return dbus_call_verb (object, SP_VERB_LAYER_LOWER, error);
    if ((strcmp(cmd, "to_top") == 0) || (strcmp(cmd, "to_front") == 0))
        return dbus_call_verb (object, SP_VERB_LAYER_TO_TOP, error);
    if ((strcmp(cmd, "to_bottom") == 0) || (strcmp(cmd, "to_back") == 0))
        return dbus_call_verb (object, SP_VERB_LAYER_TO_BOTTOM, error);
    return TRUE;
}

gboolean 
document_interface_layer_next (DocumentInterface *object, GError **error)
{
    return dbus_call_verb (object, SP_VERB_LAYER_NEXT, error);
}

gboolean 
document_interface_layer_previous (DocumentInterface *object, GError **error)
{
    return dbus_call_verb (object, SP_VERB_LAYER_PREV, error);
}






