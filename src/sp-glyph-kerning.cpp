#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef ENABLE_SVG_FONTS
#define __SP_ANCHOR_C__

/*
 * SVG <hkern> and <vkern> elements implementation
 * W3C SVG 1.1 spec, page 476, section 20.7
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
#include "sp-glyph-kerning.h"

#include "document.h"
#include <string>

static void sp_glyph_kerning_class_init(SPGlyphKerningClass *gc);
static void sp_glyph_kerning_init(SPGlyphKerning *glyph);

static void sp_glyph_kerning_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_glyph_kerning_release(SPObject *object);
static void sp_glyph_kerning_set(SPObject *object, unsigned int key, const gchar *value);
static Inkscape::XML::Node *sp_glyph_kerning_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_glyph_kerning_update(SPObject *object, SPCtx *ctx, guint flags);

static SPObjectClass *parent_class;

GType sp_glyph_kerning_h_get_type(void)
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SPGlyphKerningClass),
            NULL,	/* base_init */
            NULL,	/* base_finalize */
            (GClassInitFunc) sp_glyph_kerning_class_init,
            NULL,	/* class_finalize */
            NULL,	/* class_data */
            sizeof(SPHkern),
            16,	/* n_preallocs */
            (GInstanceInitFunc) sp_glyph_kerning_init,
            NULL,	/* value_table */
        };
        type = g_type_register_static(SP_TYPE_OBJECT, "SPHkern", &info, (GTypeFlags) 0);
    }

    return type;
}

GType sp_glyph_kerning_v_get_type(void)
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SPGlyphKerningClass),
            NULL,	/* base_init */
            NULL,	/* base_finalize */
            (GClassInitFunc) sp_glyph_kerning_class_init,
            NULL,	/* class_finalize */
            NULL,	/* class_data */
            sizeof(SPVkern),
            16,	/* n_preallocs */
            (GInstanceInitFunc) sp_glyph_kerning_init,
            NULL,	/* value_table */
        };
        type = g_type_register_static(SP_TYPE_OBJECT, "SPVkern", &info, (GTypeFlags) 0);
    }

    return type;
}

static void sp_glyph_kerning_class_init(SPGlyphKerningClass *gc)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) gc;

    parent_class = (SPObjectClass*)g_type_class_peek_parent(gc);

    sp_object_class->build = sp_glyph_kerning_build;
    sp_object_class->release = sp_glyph_kerning_release;
    sp_object_class->set = sp_glyph_kerning_set;
    sp_object_class->write = sp_glyph_kerning_write;
    sp_object_class->update = sp_glyph_kerning_update;
}

static void sp_glyph_kerning_init(SPGlyphKerning *glyph)
{
//TODO: correct these values:
    glyph->u1 = NULL;
    glyph->g1 = NULL;
    glyph->u2 = NULL;
    glyph->g2 = NULL;
    glyph->k = 0;
}

static void sp_glyph_kerning_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) (parent_class))->build) {
        ((SPObjectClass *) (parent_class))->build(object, document, repr);
    }

    sp_object_read_attr(object, "u1");
    sp_object_read_attr(object, "g1");
    sp_object_read_attr(object, "u2");
    sp_object_read_attr(object, "g2");
    sp_object_read_attr(object, "k");
}

static void sp_glyph_kerning_release(SPObject *object)
{
    //SPGlyphKerning *glyph = SP_GLYPH_KERNING(object);

    if (((SPObjectClass *) parent_class)->release) {
        ((SPObjectClass *) parent_class)->release(object);
    }
}

GlyphNames::GlyphNames(const gchar* value){
        if (value) this->names = strdup(value);
}

GlyphNames::~GlyphNames(){
    if (this->names) g_free(this->names);
}

bool GlyphNames::contains(const char* name){
    if (!(this->names) || !name) return false;
    std::istringstream is(this->names);
    std::string str;
    std::string s(name);
    while (is >> str){
        if (str == s) return true;
    }
    return false;
}

static void sp_glyph_kerning_set(SPObject *object, unsigned int key, const gchar *value)
{
    SPGlyphKerning * glyphkern = (SPGlyphKerning*) object; //even if it is a VKern this will work. I did it this way just to avoind warnings.

    switch (key) {
        case SP_ATTR_U1:
        {
            if (glyphkern->u1) {
                delete glyphkern->u1;
            }
            glyphkern->u1 = new UnicodeRange(value);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }
        case SP_ATTR_U2:
        {
            if (glyphkern->u2) {
                delete glyphkern->u2;
            }
            glyphkern->u2 = new UnicodeRange(value);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }
        case SP_ATTR_G1:
        {
            if (glyphkern->g1) {
                delete glyphkern->g1;
            }
            glyphkern->g1 = new GlyphNames(value);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }
        case SP_ATTR_G2:
        {
            if (glyphkern->g2) {
                delete glyphkern->g2;
            }
            glyphkern->g2 = new GlyphNames(value);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
             break;
        }
        case SP_ATTR_K:
        {
            double number = value ? g_ascii_strtod(value, 0) : 0;
            if (number != glyphkern->k){
                glyphkern->k = number;
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
sp_glyph_kerning_update(SPObject *object, SPCtx *ctx, guint flags)
{
    SPGlyphKerning *glyph = (SPGlyphKerning *)object;
    (void)glyph;

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        /* do something to trigger redisplay, updates? */
            sp_object_read_attr(object, "u1");
            sp_object_read_attr(object, "u2");
            sp_object_read_attr(object, "g2");
            sp_object_read_attr(object, "k");
    }

    if (((SPObjectClass *) parent_class)->update) {
        ((SPObjectClass *) parent_class)->update(object, ctx, flags);
    }
}

#define COPY_ATTR(rd,rs,key) (rd)->setAttribute((key), rs->attribute(key));

static Inkscape::XML::Node *sp_glyph_kerning_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
//    SPGlyphKerning *glyph = SP_GLYPH_KERNING(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:glyphkerning");//fix this!
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
        COPY_ATTR(repr, object->repr, "u1");
        COPY_ATTR(repr, object->repr, "g1");
        COPY_ATTR(repr, object->repr, "u2");
        COPY_ATTR(repr, object->repr, "g2");
        COPY_ATTR(repr, object->repr, "k");
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
