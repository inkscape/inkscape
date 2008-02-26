#define __SP_ANCHOR_C__

/*
 * SVG <font> element implementation
 *
 * Author:
 *   Felipe C. da S. Sanches <felipe.sanches@gmail.com>
 *
 * Copyright (C) 2008, Felipe C. da S. Sanches
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "xml/repr.h"
#include "attributes.h"
#include "sp-font.h"
#include "document.h"

static void sp_font_class_init(SPFontClass *fc);
static void sp_font_init(SPFont *font);

static void sp_font_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_font_release(SPObject *object);
static void sp_font_set(SPObject *object, unsigned int key, const gchar *value);
static Inkscape::XML::Node *sp_font_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

static gchar *sp_font_description(SPItem *item);

static SPObjectClass *parent_class;

GType sp_font_get_type(void)
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SPFontClass),
            NULL,	/* base_init */
            NULL,	/* base_finalize */
            (GClassInitFunc) sp_font_class_init,
            NULL,	/* class_finalize */
            NULL,	/* class_data */
            sizeof(SPFont),
            16,	/* n_preallocs */
            (GInstanceInitFunc) sp_font_init,
            NULL,	/* value_table */
        };
        type = g_type_register_static(SP_TYPE_OBJECT, "SPFont", &info, (GTypeFlags) 0);
    }

    return type;
}

static void sp_font_class_init(SPFontClass *fc)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) fc;

    parent_class = (SPObjectClass *) g_type_class_ref(SP_TYPE_OBJECT);

    sp_object_class->build = sp_font_build;
    sp_object_class->release = sp_font_release;
    sp_object_class->set = sp_font_set;
    sp_object_class->write = sp_font_write;
}

static void sp_font_init(SPFont *font)
{
    font->horiz_origin_x = 0;
    font->horiz_origin_y = 0;
    font->horiz_adv_x = 0;
//I think we should have extra stuff here and in the set method in order to set default value as specified at http://www.w3.org/TR/SVG/fonts.html
    font->vert_origin_x = 0;
    font->vert_origin_y = 0;
    font->vert_adv_y = 0;
}

static void sp_font_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) (parent_class))->build) {
        ((SPObjectClass *) (parent_class))->build(object, document, repr);
    }

    sp_object_read_attr(object, "horiz-origin-x");
    sp_object_read_attr(object, "horiz-origin-y");
    sp_object_read_attr(object, "horiz-adv-x");
    sp_object_read_attr(object, "vert-origin-x");
    sp_object_read_attr(object, "vert-origin-y");
    sp_object_read_attr(object, "vert-adv-y");
}

static void sp_font_release(SPObject *object)
{
    //SPFont *font = SP_FONT(object);

    if (((SPObjectClass *) parent_class)->release) {
        ((SPObjectClass *) parent_class)->release(object);
    }
}

//ToDo: use helper-fns.h
inline double helperfns_read_number(gchar const *value) {
    if (!value) return 0;
    char *end;
    double ret = g_ascii_strtod(value, &end);
    if (*end) {
        g_warning("Unable to convert \"%s\" to number", value);
        // We could leave this out, too. If strtod can't convert
        // anything, it will return zero.
        ret = 0;
    }
    return ret;
}

static void sp_font_set(SPObject *object, unsigned int key, const gchar *value)
{
    SPFont *font = SP_FONT(object);
    double number;

    switch (key) {
	case SP_ATTR_HORIZ_ORIGIN_X:
            number = helperfns_read_number(value);
            if (number != font->horiz_origin_x){
                font->horiz_origin_x = number;
g_warning("SP_ATTR_HORIZ_ORIGIN_X: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_HORIZ_ORIGIN_Y:
            number = helperfns_read_number(value);
            if (number != font->horiz_origin_y){
                font->horiz_origin_y = number;
g_warning("SP_ATTR_HORIZ_ORIGIN_Y: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_HORIZ_ADV_X:
            number = helperfns_read_number(value);
            if (number != font->horiz_adv_x){
                font->horiz_adv_x = number;
g_warning("SP_ATTR_HORIZ_ADV_X: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_VERT_ORIGIN_X:
            number = helperfns_read_number(value);
            if (number != font->vert_origin_x){
                font->vert_origin_x = number;
g_warning("SP_ATTR_VERT_ORIGIN_X: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_VERT_ORIGIN_Y:
            number = helperfns_read_number(value);
            if (number != font->vert_origin_y){
                font->vert_origin_y = number;
g_warning("SP_ATTR_VERT_ORIGIN_Y: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_VERT_ADV_Y:
            number = helperfns_read_number(value);
            if (number != font->vert_adv_y){
                font->vert_adv_y = number;
g_warning("SP_ATTR_VERT_ADV_Y: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	default:
            if (((SPObjectClass *) (parent_class))->set) {
                ((SPObjectClass *) (parent_class))->set(object, key, value);
            }
            break;
    }
}

#define COPY_ATTR(rd,rs,key) (rd)->setAttribute((key), rs->attribute(key));

static Inkscape::XML::Node *sp_font_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    SPFont *font = SP_FONT(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(SP_OBJECT_DOCUMENT(object));
        repr = xml_doc->createElement("svg:font");
    }

    sp_repr_set_svg_double(repr, "horiz-origin-x", font->horiz_origin_x);
    sp_repr_set_svg_double(repr, "horiz-origin-y", font->horiz_origin_y);
    sp_repr_set_svg_double(repr, "horiz-adv-x", font->horiz_adv_x);
    sp_repr_set_svg_double(repr, "vert-origin-x", font->vert_origin_x);
    sp_repr_set_svg_double(repr, "vert-origin-y", font->vert_origin_y);
    sp_repr_set_svg_double(repr, "vert-adv-y", font->vert_adv_y);

    if (repr != SP_OBJECT_REPR(object)) {
        COPY_ATTR(repr, object->repr, "horiz-origin-x");
        COPY_ATTR(repr, object->repr, "horiz-origin-y");
        COPY_ATTR(repr, object->repr, "horiz-adv-x");
        COPY_ATTR(repr, object->repr, "vert-origin-x");
        COPY_ATTR(repr, object->repr, "vert-origin-y");
        COPY_ATTR(repr, object->repr, "vert-adv-y");
    }

    if (((SPObjectClass *) (parent_class))->write) {
        ((SPObjectClass *) (parent_class))->write(object, repr, flags);
    }

    return repr;
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
