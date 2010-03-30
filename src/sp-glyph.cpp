#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef ENABLE_SVG_FONTS
#define __SP_GLYPH_C__

/*
 * SVG <glyph> element implementation
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
#include "sp-glyph.h"
#include "document.h"

static void sp_glyph_class_init(SPGlyphClass *gc);
static void sp_glyph_init(SPGlyph *glyph);

static void sp_glyph_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_glyph_release(SPObject *object);
static void sp_glyph_set(SPObject *object, unsigned int key, const gchar *value);
static Inkscape::XML::Node *sp_glyph_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_glyph_update(SPObject *object, SPCtx *ctx, guint flags);

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

    parent_class = (SPObjectClass*)g_type_class_peek_parent(gc);

    sp_object_class->build = sp_glyph_build;
    sp_object_class->release = sp_glyph_release;
    sp_object_class->set = sp_glyph_set;
    sp_object_class->write = sp_glyph_write;
    sp_object_class->update = sp_glyph_update;
}

static void sp_glyph_init(SPGlyph *glyph)
{
//TODO: correct these values:

    new (&glyph->unicode) Glib::ustring();
    new (&glyph->glyph_name) Glib::ustring();
    glyph->d = NULL;
    glyph->orientation = GLYPH_ORIENTATION_BOTH;
    glyph->arabic_form = GLYPH_ARABIC_FORM_INITIAL;
    glyph->lang = NULL;
    glyph->horiz_adv_x = 0;
    glyph->vert_origin_x = 0;
    glyph->vert_origin_y = 0;
    glyph->vert_adv_y = 0;
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

static glyphArabicForm sp_glyph_read_arabic_form(gchar const *value){
    if (!value) return GLYPH_ARABIC_FORM_INITIAL; //TODO: verify which is the default default (for me, the spec is not clear)
    switch(value[0]){
        case 'i':
            if (strncmp(value, "initial", 7) == 0) return GLYPH_ARABIC_FORM_INITIAL;
            if (strncmp(value, "isolated", 8) == 0) return GLYPH_ARABIC_FORM_ISOLATED;
            break;
        case 'm':
            if (strncmp(value, "medial", 6) == 0) return GLYPH_ARABIC_FORM_MEDIAL;
            break;
        case 't':
            if (strncmp(value, "terminal", 8) == 0) return GLYPH_ARABIC_FORM_TERMINAL;
            break;
    }
    return GLYPH_ARABIC_FORM_INITIAL; //TODO: VERIFY DEFAULT!
}

static glyphOrientation sp_glyph_read_orientation(gchar const *value){
    if (!value) return GLYPH_ORIENTATION_BOTH;
    switch(value[0]){
        case 'h':
            return GLYPH_ORIENTATION_HORIZONTAL;
            break;
        case 'v':
            return GLYPH_ORIENTATION_VERTICAL;
            break;
    }
//ERROR? TODO: VERIFY PROPER ERROR HANDLING
    return GLYPH_ORIENTATION_BOTH;
}

static void sp_glyph_set(SPObject *object, unsigned int key, const gchar *value)
{
    SPGlyph *glyph = SP_GLYPH(object);

    switch (key) {
        case SP_ATTR_UNICODE:
        {
            glyph->unicode.clear();
            if (value) glyph->unicode.append(value);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }
        case SP_ATTR_GLYPH_NAME:
        {
            glyph->glyph_name.clear();
            if (value) glyph->glyph_name.append(value);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }
        case SP_ATTR_D:
        {
            if (glyph->d) g_free(glyph->d);
            glyph->d = g_strdup(value);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }
        case SP_ATTR_ORIENTATION:
        {
            glyphOrientation orient = sp_glyph_read_orientation(value);
            if (glyph->orientation != orient){
                glyph->orientation = orient;
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        case SP_ATTR_ARABIC_FORM:
        {
            glyphArabicForm form = sp_glyph_read_arabic_form(value);
            if (glyph->arabic_form != form){
                glyph->arabic_form = form;
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        case SP_ATTR_LANG:
        {
            if (glyph->lang) g_free(glyph->lang);
            glyph->lang = g_strdup(value);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }
        case SP_ATTR_HORIZ_ADV_X:
        {
            double number = value ? g_ascii_strtod(value, 0) : 0;
            if (number != glyph->horiz_adv_x){
                glyph->horiz_adv_x = number;
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        case SP_ATTR_VERT_ORIGIN_X:
        {
            double number = value ? g_ascii_strtod(value, 0) : 0;
            if (number != glyph->vert_origin_x){
                glyph->vert_origin_x = number;
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        case SP_ATTR_VERT_ORIGIN_Y:
        {
            double number = value ? g_ascii_strtod(value, 0) : 0;
            if (number != glyph->vert_origin_y){
                glyph->vert_origin_y = number;
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        case SP_ATTR_VERT_ADV_Y:
        {
            double number = value ? g_ascii_strtod(value, 0) : 0;
            if (number != glyph->vert_adv_y){
                glyph->vert_adv_y = number;
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
        }
        default:
        {
            if (((SPObjectClass *) (parent_class))->set) {
                ((SPObjectClass *) (parent_class))->set(object, key, value);
            }
            break;
        }
    }
}

/**
 *  * Receives update notifications.
 *   */
static void
sp_glyph_update(SPObject *object, SPCtx *ctx, guint flags)
{
    SPGlyph *glyph = SP_GLYPH(object);
    (void)glyph;

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        /* do something to trigger redisplay, updates? */
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

    if (((SPObjectClass *) parent_class)->update) {
        ((SPObjectClass *) parent_class)->update(object, ctx, flags);
    }
}

#define COPY_ATTR(rd,rs,key) (rd)->setAttribute((key), rs->attribute(key));

static Inkscape::XML::Node *sp_glyph_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
//    SPGlyph *glyph = SP_GLYPH(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:glyph");
    }

/* I am commenting out this part because I am not certain how does it work. I will have to study it later. Juca
    repr->setAttribute("unicode", glyph->unicode);
    repr->setAttribute("glyph-name", glyph->glyph_name);
    repr->setAttribute("d", glyph->d);
    sp_repr_set_svg_double(repr, "orientation", (double) glyph->orientation);
    sp_repr_set_svg_double(repr, "arabic-form", (double) glyph->arabic_form);
    repr->setAttribute("lang", glyph->lang);
    sp_repr_set_svg_double(repr, "horiz-adv-x", glyph->horiz_adv_x);
    sp_repr_set_svg_double(repr, "vert-origin-x", glyph->vert_origin_x);
    sp_repr_set_svg_double(repr, "vert-origin-y", glyph->vert_origin_y);
    sp_repr_set_svg_double(repr, "vert-adv-y", glyph->vert_adv_y);
*/
    if (repr != SP_OBJECT_REPR(object)) {
        COPY_ATTR(repr, object->repr, "unicode");
        COPY_ATTR(repr, object->repr, "glyph-name");
        COPY_ATTR(repr, object->repr, "d");
        COPY_ATTR(repr, object->repr, "orientation");
        COPY_ATTR(repr, object->repr, "arabic-form");
        COPY_ATTR(repr, object->repr, "lang");
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
