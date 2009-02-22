#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef ENABLE_SVG_FONTS
#define __SP_MISSING_GLYPH_C__

/*
 * SVG <missing-glyph> element implementation
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
#include "sp-missing-glyph.h"
#include "document.h"
#include "helper-fns.h"

static void sp_missing_glyph_class_init(SPMissingGlyphClass *gc);
static void sp_missing_glyph_init(SPMissingGlyph *glyph);

static void sp_missing_glyph_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_missing_glyph_release(SPObject *object);
static void sp_missing_glyph_set(SPObject *object, unsigned int key, const gchar *value);
static Inkscape::XML::Node *sp_missing_glyph_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static SPObjectClass *parent_class;

GType sp_missing_glyph_get_type(void)
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SPMissingGlyphClass),
            NULL,	/* base_init */
            NULL,	/* base_finalize */
            (GClassInitFunc) sp_missing_glyph_class_init,
            NULL,	/* class_finalize */
            NULL,	/* class_data */
            sizeof(SPMissingGlyph),
            16,	/* n_preallocs */
            (GInstanceInitFunc) sp_missing_glyph_init,
            NULL,	/* value_table */
        };
        type = g_type_register_static(SP_TYPE_OBJECT, "SPMissingGlyph", &info, (GTypeFlags) 0);
    }

    return type;
}

static void sp_missing_glyph_class_init(SPMissingGlyphClass *gc)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) gc;

    parent_class = (SPObjectClass*)g_type_class_peek_parent(gc);

    sp_object_class->build = sp_missing_glyph_build;
    sp_object_class->release = sp_missing_glyph_release;
    sp_object_class->set = sp_missing_glyph_set;
    sp_object_class->write = sp_missing_glyph_write;
}

static void sp_missing_glyph_init(SPMissingGlyph *glyph)
{
//TODO: correct these values:
    glyph->d = NULL;
    glyph->horiz_adv_x = 0;
    glyph->vert_origin_x = 0;
    glyph->vert_origin_y = 0;
    glyph->vert_adv_y = 0;
}

static void sp_missing_glyph_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) (parent_class))->build) {
        ((SPObjectClass *) (parent_class))->build(object, document, repr);
    }

    sp_object_read_attr(object, "d");
    sp_object_read_attr(object, "horiz-adv-x");
    sp_object_read_attr(object, "vert-origin-x");
    sp_object_read_attr(object, "vert-origin-y");
    sp_object_read_attr(object, "vert-adv-y");
}

static void sp_missing_glyph_release(SPObject *object)
{
    //SPMissingGlyph *glyph = SP_MISSING_GLYPH(object);

    if (((SPObjectClass *) parent_class)->release) {
        ((SPObjectClass *) parent_class)->release(object);
    }
}

static void sp_missing_glyph_set(SPObject *object, unsigned int key, const gchar *value)
{
    SPMissingGlyph *glyph = SP_MISSING_GLYPH(object);
    double number;

    switch (key) {
        case SP_ATTR_D:
            if (glyph->d) g_free(glyph->d);
            glyph->d = g_strdup(value);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
	case SP_ATTR_HORIZ_ADV_X:
            number = helperfns_read_number(value);
            if (number != glyph->horiz_adv_x){
                glyph->horiz_adv_x = number;
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_VERT_ORIGIN_X:
            number = helperfns_read_number(value);
            if (number != glyph->vert_origin_x){
                glyph->vert_origin_x = number;
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_VERT_ORIGIN_Y:
            number = helperfns_read_number(value);
            if (number != glyph->vert_origin_y){
                glyph->vert_origin_y = number;
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_VERT_ADV_Y:
            number = helperfns_read_number(value);
            if (number != glyph->vert_adv_y){
                glyph->vert_adv_y = number;
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

static Inkscape::XML::Node *sp_missing_glyph_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
//    SPMissingGlyph *glyph = SP_MISSING_GLYPH(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:glyph");
    }

/* I am commenting out this part because I am not certain how does it work. I will have to study it later. Juca
    repr->setAttribute("d", glyph->d);
    sp_repr_set_svg_double(repr, "horiz-adv-x", glyph->horiz_adv_x);
    sp_repr_set_svg_double(repr, "vert-origin-x", glyph->vert_origin_x);
    sp_repr_set_svg_double(repr, "vert-origin-y", glyph->vert_origin_y);
    sp_repr_set_svg_double(repr, "vert-adv-y", glyph->vert_adv_y);
*/
    if (repr != SP_OBJECT_REPR(object)) {
        COPY_ATTR(repr, object->repr, "d");
        COPY_ATTR(repr, object->repr, "horiz-adv-x");
        COPY_ATTR(repr, object->repr, "vert-origin-x");
        COPY_ATTR(repr, object->repr, "vert-origin-y");
        COPY_ATTR(repr, object->repr, "vert-adv-y");
    }

    if (((SPObjectClass *) (parent_class))->write) {
        ((SPObjectClass *) (parent_class))->write(object, xml_doc, repr, flags);
    }

    return repr;
}
#endif //#ifdef ENABLE_SVG_FONTS
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
