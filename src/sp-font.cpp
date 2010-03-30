#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef ENABLE_SVG_FONTS

/*
 * SVG <font> element implementation
 *
 * Author:
 *   Felipe C. da S. Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2008, Felipe C. da S. Sanches
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "xml/repr.h"
#include "attributes.h"
#include "sp-font.h"
#include "sp-glyph.h"
#include "sp-missing-glyph.h"
#include "document.h"

#include "display/nr-svgfonts.h"

static void sp_font_class_init(SPFontClass *fc);
static void sp_font_init(SPFont *font);

static void sp_font_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_font_release(SPObject *object);
static void sp_font_set(SPObject *object, unsigned int key, const gchar *value);
static Inkscape::XML::Node *sp_font_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static void sp_font_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref);
static void sp_font_remove_child(SPObject *object, Inkscape::XML::Node *child);
static void sp_font_update(SPObject *object, SPCtx *ctx, guint flags);

// static gchar *sp_font_description(SPItem *item);

static SPObjectClass *parent_class;

GType sp_font_get_type(void)
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SPFontClass),
            NULL,       /* base_init */
            NULL,       /* base_finalize */
            (GClassInitFunc) sp_font_class_init,
            NULL,       /* class_finalize */
            NULL,       /* class_data */
            sizeof(SPFont),
            16, /* n_preallocs */
            (GInstanceInitFunc) sp_font_init,
            NULL,       /* value_table */
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
    sp_object_class->child_added = sp_font_child_added;
    sp_object_class->remove_child = sp_font_remove_child;
    sp_object_class->update = sp_font_update;
}

//I think we should have extra stuff here and in the set method in order to set default value as specified at http://www.w3.org/TR/SVG/fonts.html

// TODO determine better values and/or make these dynamic:
double FNT_DEFAULT_ADV = 90; // TODO determine proper default
double FNT_DEFAULT_ASCENT = 90; // TODO determine proper default
double FNT_UNITS_PER_EM = 90; // TODO determine proper default

static void sp_font_init(SPFont *font)
{
    font->horiz_origin_x = 0;
    font->horiz_origin_y = 0;
    font->horiz_adv_x = FNT_DEFAULT_ADV;
    font->vert_origin_x = FNT_DEFAULT_ADV / 2.0;
    font->vert_origin_y = FNT_DEFAULT_ASCENT;
    font->vert_adv_y = FNT_UNITS_PER_EM;
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

    sp_document_add_resource(document, "font", object);
}


static void sp_font_children_modified(SPFont */*sp_font*/)
{
}

/**
 * Callback for child_added event.
 */
static void
sp_font_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    SPFont *f = SP_FONT(object);

    if (((SPObjectClass *) parent_class)->child_added)
        (* ((SPObjectClass *) parent_class)->child_added)(object, child, ref);

    sp_font_children_modified(f);
    object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}


/**
 * Callback for remove_child event.
 */
static void
sp_font_remove_child(SPObject *object, Inkscape::XML::Node *child)
{
    SPFont *f = SP_FONT(object);

    if (((SPObjectClass *) parent_class)->remove_child)
        (* ((SPObjectClass *) parent_class)->remove_child)(object, child);

    sp_font_children_modified(f);
    object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static void sp_font_release(SPObject *object)
{
    //SPFont *font = SP_FONT(object);
    sp_document_remove_resource(SP_OBJECT_DOCUMENT(object), "font", object);

    if (((SPObjectClass *) parent_class)->release) {
        ((SPObjectClass *) parent_class)->release(object);
    }
}

static void sp_font_set(SPObject *object, unsigned int key, const gchar *value)
{
    SPFont *font = SP_FONT(object);

    // TODO these are floating point, so some epsilon comparison would be good
    switch (key) {
        case SP_ATTR_HORIZ_ORIGIN_X:
        {
            double number = value ? g_ascii_strtod(value, 0) : 0;
            if (number != font->horiz_origin_x){
                font->horiz_origin_x = number;
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        case SP_ATTR_HORIZ_ORIGIN_Y:
        {
            double number = value ? g_ascii_strtod(value, 0) : 0;
            if (number != font->horiz_origin_y){
                font->horiz_origin_y = number;
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        case SP_ATTR_HORIZ_ADV_X:
        {
            double number = value ? g_ascii_strtod(value, 0) : FNT_DEFAULT_ADV;
            if (number != font->horiz_adv_x){
                font->horiz_adv_x = number;
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        case SP_ATTR_VERT_ORIGIN_X:
        {
            double number = value ? g_ascii_strtod(value, 0) : FNT_DEFAULT_ADV / 2.0;
            if (number != font->vert_origin_x){
                font->vert_origin_x = number;
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        case SP_ATTR_VERT_ORIGIN_Y:
        {
            double number = value ? g_ascii_strtod(value, 0) : FNT_DEFAULT_ASCENT;
            if (number != font->vert_origin_y){
                font->vert_origin_y = number;
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        case SP_ATTR_VERT_ADV_Y:
        {
            double number = value ? g_ascii_strtod(value, 0) : FNT_UNITS_PER_EM;
            if (number != font->vert_adv_y){
                font->vert_adv_y = number;
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        default:
            if (((SPObjectClass *) (parent_class))->set) {
                ((SPObjectClass *) (parent_class))->set(object, key, value);
            }
            break;
    }
}

/**
 * Receives update notifications.
 */
static void
sp_font_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG)) {
        sp_object_read_attr(object, "horiz-origin-x");
        sp_object_read_attr(object, "horiz-origin-y");
        sp_object_read_attr(object, "horiz-adv-x");
        sp_object_read_attr(object, "vert-origin-x");
        sp_object_read_attr(object, "vert-origin-y");
        sp_object_read_attr(object, "vert-adv-y");
    }

    if (((SPObjectClass *) parent_class)->update) {
        ((SPObjectClass *) parent_class)->update(object, ctx, flags);
    }
}

#define COPY_ATTR(rd,rs,key) (rd)->setAttribute((key), rs->attribute(key));

static Inkscape::XML::Node *sp_font_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPFont *font = SP_FONT(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
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
