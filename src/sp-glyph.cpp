#define __SP_ANCHOR_C__

/*
 * SVG <glyph> element implementation
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
#include "sp-glyph.h"
#include "document.h"

static void sp_glyph_class_init(SPGlyphClass *fc);
static void sp_glyph_init(SPGlyph *glyph);

static void sp_glyph_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_glyph_release(SPObject *object);
static void sp_glyph_set(SPObject *object, unsigned int key, const gchar *value);
static Inkscape::XML::Node *sp_glyph_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

static SPObjectClass *parent_class;

GType sp_glyph_get_type(void)
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SPGlyphClass),
            NULL,	/* base_init */
            NULL,	/* base_finalize */
            (GClassInitFunc) sp_glyph_class_init,
            NULL,	/* class_finalize */
            NULL,	/* class_data */
            sizeof(SPGlyph),
            16,	/* n_preallocs */
            (GInstanceInitFunc) sp_glyph_init,
            NULL,	/* value_table */
        };
        type = g_type_register_static(SP_TYPE_OBJECT, "SPGlyph", &info, (GTypeFlags) 0);
    }

    return type;
}

static void sp_glyph_class_init(SPGlyphClass *gc)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) gc;

    parent_class = (SPObjectClass *) g_type_class_ref(SP_TYPE_OBJECT);

    sp_object_class->build = sp_glyph_build;
    sp_object_class->release = sp_glyph_release;
    sp_object_class->set = sp_glyph_set;
    sp_object_class->write = sp_glyph_write;
}

static void sp_glyph_init(SPGlyph *glyph)
{
//TODO: correct these values:
    glyph->unicode = "";
    glyph->glyph-name = "";
    glyph->d = "";
    glyph->orientation = "";
    glyph->arabic-form = "";
    glyph->lang = "";
    glyph->horiz-adv-x = 0;
    glyph->vert-origin-x = 0;
    glyph->vert-origin-y = 0;
    glyph->vert-adv-y = 0;
}

static void sp_glyph_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) (parent_class))->build) {
        ((SPObjectClass *) (parent_class))->build(object, document, repr);
    }

    sp_object_read_attr(object, "unicode");
    sp_object_read_attr(object, "glyph-name");
    sp_object_read_attr(object, "d");
    sp_object_read_attr(object, "orientation");
    sp_object_read_attr(object, "arabic-form");
    sp_object_read_attr(object, "lang");
    sp_object_read_attr(object, "horiz-adv-x");
    sp_object_read_attr(object, "vert-origin-x");
    sp_object_read_attr(object, "vert-origin-y");
    sp_object_read_attr(object, "vert-adv-y");
}

static void sp_glyph_release(SPObject *object)
{
    //SPGlyph *glyph = SP_GLYPH(object);

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

static void sp_glyph_set(SPObject *object, unsigned int key, const gchar *value)
{
    SPGlyph *glyph = SP_GLYPH(object);
    double number;

    switch (key) {
	case SP_ATTR_HORIZ_ORIGIN_X:
            number = helperfns_read_number(value);
            if (number != glyph->horiz_origin_x){
                glyph->horiz_origin_x = number;
g_warning("SP_ATTR_HORIZ_ORIGIN_X: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_HORIZ_ORIGIN_Y:
            number = helperfns_read_number(value);
            if (number != glyph->horiz_origin_y){
                glyph->horiz_origin_y = number;
g_warning("SP_ATTR_HORIZ_ORIGIN_Y: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_HORIZ_ADV_X:
            number = helperfns_read_number(value);
            if (number != glyph->horiz_adv_x){
                glyph->horiz_adv_x = number;
g_warning("SP_ATTR_HORIZ_ADV_X: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_VERT_ORIGIN_X:
            number = helperfns_read_number(value);
            if (number != glyph->vert_origin_x){
                glyph->vert_origin_x = number;
g_warning("SP_ATTR_VERT_ORIGIN_X: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_VERT_ORIGIN_Y:
            number = helperfns_read_number(value);
            if (number != glyph->vert_origin_y){
                glyph->vert_origin_y = number;
g_warning("SP_ATTR_VERT_ORIGIN_Y: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_VERT_ADV_Y:
            number = helperfns_read_number(value);
            if (number != glyph->vert_adv_y){
                glyph->vert_adv_y = number;
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

static Inkscape::XML::Node *sp_glyph_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    SPGlyph *glyph = SP_GLYPH(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(SP_OBJECT_DOCUMENT(object));
        repr = xml_doc->createElement("svg:glyph");
    }

    sp_repr_set_svg_double(repr, "horiz-origin-x", glyph->horiz_origin_x);
    sp_repr_set_svg_double(repr, "horiz-origin-y", glyph->horiz_origin_y);
    sp_repr_set_svg_double(repr, "horiz-adv-x", glyph->horiz_adv_x);
    sp_repr_set_svg_double(repr, "vert-origin-x", glyph->vert_origin_x);
    sp_repr_set_svg_double(repr, "vert-origin-y", glyph->vert_origin_y);
    sp_repr_set_svg_double(repr, "vert-adv-y", glyph->vert_adv_y);

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
