#include "config.h"
#ifdef ENABLE_SVG_FONTS
#define __SP_FONTFACE_C__

/*
 * SVG <font-face> element implementation
 *
 * Section 20.8.3 of the W3C SVG 1.1 spec
 * available at: 
 * http://www.w3.org/TR/SVG/fonts.html#FontFaceElement
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
#include "sp-font-face.h"
#include "document.h"
#include "helper-fns.h"

class ObjectContainer
{

public:
    ObjectContainer(double def){
        this->isset = false;
        this->default_value = def;
    }

    double get(){
        if (this->isset)
            return this->obj;
        else
            return this->default_value;
    }

    void set(double val){
        this->obj = val;
        this->isset = true;
    }

    void unset(){
        this->isset = false;
    }

private:
    double obj;
    double default_value;
    bool isset;
};

static std::vector<FontFaceStyleType> sp_read_fontFaceStyleType(gchar const *value){
    std::vector<FontFaceStyleType> v;

    if (!value){
        v.push_back(SP_FONTFACE_STYLE_ALL);
        return v;
    }
    
    if (strncmp(value, "all", 3) == 0){
        value += 3;
        while(value[0]==',' || value[0]==' ')
            value++;
        v.push_back(SP_FONTFACE_STYLE_ALL);
        return v;
    }

    while(value[0]!='\0'){
        switch(value[0]){
            case 'n':
                if (strncmp(value, "normal", 6) == 0){
                    v.push_back(SP_FONTFACE_STYLE_NORMAL);
                    value += 6;
                }
                break;
            case 'i':
                if (strncmp(value, "italic", 6) == 0){
                    v.push_back(SP_FONTFACE_STYLE_ITALIC);
                    value += 6;
                }
                break;
            case 'o':
                if (strncmp(value, "oblique", 7) == 0){
                    v.push_back(SP_FONTFACE_STYLE_OBLIQUE);
                    value += 7;
                }
                break;
        }
        while(value[0]==',' || value[0]==' ')
            value++;
    }
    return v;
}

static std::vector<FontFaceVariantType> sp_read_fontFaceVariantType(gchar const *value){
    std::vector<FontFaceVariantType> v;

    if (!value){
        v.push_back(SP_FONTFACE_VARIANT_NORMAL);
        return v;
    }

    while(value[0]!='\0'){
        switch(value[0]){
            case 'n':
                if (strncmp(value, "normal", 6) == 0){
                    v.push_back(SP_FONTFACE_VARIANT_NORMAL);
                    value += 6;
                }
                break;
            case 's':
                if (strncmp(value, "small-caps", 10) == 0){
                    v.push_back(SP_FONTFACE_VARIANT_SMALL_CAPS);
                    value += 10;
                }
                break;
        }
        while(value[0]==',' || value[0]==' ')
            value++;
    }
    return v;
}

static std::vector<FontFaceWeightType> sp_read_fontFaceWeightType(gchar const *value){
    std::vector<FontFaceWeightType> v;

    if (!value){
        v.push_back(SP_FONTFACE_WEIGHT_ALL);
        return v;
    }
    
    if (strncmp(value, "all", 3) == 0){
        value += 3;
        while(value[0]==',' || value[0]==' ')
            value++;
        v.push_back(SP_FONTFACE_WEIGHT_ALL);
        return v;
    }

    while(value[0]!='\0'){
        switch(value[0]){
            case 'n':
                if (strncmp(value, "normal", 6) == 0){
                    v.push_back(SP_FONTFACE_WEIGHT_NORMAL);
                    value += 6;
                }
                break;
            case 'b':
                if (strncmp(value, "bold", 4) == 0){
                    v.push_back(SP_FONTFACE_WEIGHT_BOLD);
                    value += 4;
                }
                break;
            case '1':
                if (strncmp(value, "100", 3) == 0){
                    v.push_back(SP_FONTFACE_WEIGHT_100);
                    value += 3;
                }
                break;
            case '2':
                if (strncmp(value, "200", 3) == 0){
                    v.push_back(SP_FONTFACE_WEIGHT_200);
                    value += 3;
                }
                break;
            case '3':
                if (strncmp(value, "300", 3) == 0){
                    v.push_back(SP_FONTFACE_WEIGHT_300);
                    value += 3;
                }
                break;
            case '4':
                if (strncmp(value, "400", 3) == 0){
                    v.push_back(SP_FONTFACE_WEIGHT_400);
                    value += 3;
                }
                break;
            case '5':
                if (strncmp(value, "500", 3) == 0){
                    v.push_back(SP_FONTFACE_WEIGHT_500);
                    value += 3;
                }
                break;
            case '6':
                if (strncmp(value, "600", 3) == 0){
                    v.push_back(SP_FONTFACE_WEIGHT_600);
                    value += 3;
                }
                break;
            case '7':
                if (strncmp(value, "700", 3) == 0){
                    v.push_back(SP_FONTFACE_WEIGHT_700);
                    value += 3;
                }
                break;
            case '8':
                if (strncmp(value, "800", 3) == 0){
                    v.push_back(SP_FONTFACE_WEIGHT_800);
                    value += 3;
                }
                break;
            case '9':
                if (strncmp(value, "900", 3) == 0){
                    v.push_back(SP_FONTFACE_WEIGHT_900);
                    value += 3;
                }
                break;
        }
        while(value[0]==',' || value[0]==' ')
            value++;
    }
    return v;
}

static std::vector<FontFaceStretchType> sp_read_fontFaceStretchType(gchar const *value){
    std::vector<FontFaceStretchType> v;

    if (!value){
        v.push_back(SP_FONTFACE_STRETCH_NORMAL);
        return v;
    }
    
    if (strncmp(value, "all", 3) == 0){
        value += 3;
        while(value[0]==',' || value[0]==' ')
            value++;
        v.push_back(SP_FONTFACE_STRETCH_ALL);
        return v;
    }

    while(value[0]!='\0'){
        switch(value[0]){
            case 'n':
                if (strncmp(value, "normal", 6) == 0){
                    v.push_back(SP_FONTFACE_STRETCH_NORMAL);
                    value += 6;
                }
                break;
            case 'u':
                if (strncmp(value, "ultra-condensed", 15) == 0){
                    v.push_back(SP_FONTFACE_STRETCH_ULTRA_CONDENSED);
                    value += 15;
                }
                if (strncmp(value, "ultra-expanded", 14) == 0){
                    v.push_back(SP_FONTFACE_STRETCH_ULTRA_EXPANDED);
                    value += 14;
                }
                break;
            case 'e':
                if (strncmp(value, "expanded", 8) == 0){
                    v.push_back(SP_FONTFACE_STRETCH_EXPANDED);
                    value += 8;
                }
                if (strncmp(value, "extra-condensed", 15) == 0){
                    v.push_back(SP_FONTFACE_STRETCH_EXTRA_CONDENSED);
                    value += 15;
                }
                if (strncmp(value, "extra-expanded", 14) == 0){
                    v.push_back(SP_FONTFACE_STRETCH_EXTRA_EXPANDED);
                    value += 14;
                }
                break;
            case 'c':
                if (strncmp(value, "condensed", 9) == 0){
                    v.push_back(SP_FONTFACE_STRETCH_CONDENSED);
                    value += 9;
                }
                break;
            case 's':
                if (strncmp(value, "semi-condensed", 14) == 0){
                    v.push_back(SP_FONTFACE_STRETCH_SEMI_CONDENSED);
                    value += 14;
                }
                if (strncmp(value, "semi-expanded", 13) == 0){
                    v.push_back(SP_FONTFACE_STRETCH_SEMI_EXPANDED);
                    value += 13;
                }
                break;
        }
        while(value[0]==',' || value[0]==' ')
            value++;
    }
    return v;
}

static void sp_fontface_class_init(SPFontFaceClass *fc);
static void sp_fontface_init(SPFontFace *font);

static void sp_fontface_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_fontface_release(SPObject *object);
static void sp_fontface_set(SPObject *object, unsigned int key, const gchar *value);
static Inkscape::XML::Node *sp_fontface_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static void sp_fontface_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref);
static void sp_fontface_remove_child(SPObject *object, Inkscape::XML::Node *child);
static void sp_fontface_update(SPObject *object, SPCtx *ctx, guint flags);

static SPObjectClass *parent_class;

GType sp_fontface_get_type(void)
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SPFontFaceClass),
            NULL,	/* base_init */
            NULL,	/* base_finalize */
            (GClassInitFunc) sp_fontface_class_init,
            NULL,	/* class_finalize */
            NULL,	/* class_data */
            sizeof(SPFontFace),
            16,	/* n_preallocs */
            (GInstanceInitFunc) sp_fontface_init,
            NULL,	/* value_table */
        };
        type = g_type_register_static(SP_TYPE_OBJECT, "SPFontFace", &info, (GTypeFlags) 0);
    }

    return type;
}

static void sp_fontface_class_init(SPFontFaceClass *fc)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) fc;

    parent_class = (SPObjectClass *) g_type_class_ref(SP_TYPE_OBJECT);

    sp_object_class->build = sp_fontface_build;
    sp_object_class->release = sp_fontface_release;
    sp_object_class->set = sp_fontface_set;
    sp_object_class->write = sp_fontface_write;
    sp_object_class->child_added = sp_fontface_child_added;
    sp_object_class->remove_child = sp_fontface_remove_child;
    sp_object_class->update = sp_fontface_update;
}

static void sp_fontface_init(SPFontFace *face)
{
    std::vector<FontFaceStyleType> style;
    style.push_back(SP_FONTFACE_STYLE_ALL);
    face->font_style = style;

    std::vector<FontFaceVariantType> variant;
    variant.push_back(SP_FONTFACE_VARIANT_NORMAL);
    face->font_variant = variant;

    std::vector<FontFaceWeightType> weight;
    weight.push_back(SP_FONTFACE_WEIGHT_ALL);
    face->font_weight = weight;

    std::vector<FontFaceStretchType> stretch;
    stretch.push_back(SP_FONTFACE_STRETCH_NORMAL);
    face->font_stretch = stretch;
/*
    face->font_family = NULL;
    //face->font_style = ;
    //face->font_variant = ;
    //face->font_weight = ;
    //face->font_stretch = ;
    face->font_size = NULL;
    //face->unicode_range = ;
    face->units_per_em = 1000;
    //face->panose_1 = ;
    face->stem_v = ;
    face->stem_h = ;
    face->slope = 0;
    face->cap_height = ;
    face->x_height = ;
    face->accent_height = ;
    face->ascent = ;
    face->descent = ;
    face->widths = NULL;
    face->bbox = NULL;
    face->ideographic = ;
    face->alphabetic = ;
    face->mathematical = ;
    face->hanging = ;
    face->v_ideographic = ;
    face->v_alphabetic = ;
    face->v_mathematical = ;
    face->v_hanging = ;
    face->underline_position = ;
    face->underline_thickness = ;
    face->strikethrough_position = ;
    face->strikethrough_thickness = ;
    face->overline_position = ;
    face->overline_thickness = ;
*/
}

static void sp_fontface_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) (parent_class))->build) {
        ((SPObjectClass *) (parent_class))->build(object, document, repr);
    }

    sp_object_read_attr(object, "font-family");
    sp_object_read_attr(object, "font-style");
    sp_object_read_attr(object, "font-variant");
    sp_object_read_attr(object, "font-weight");
    sp_object_read_attr(object, "font-stretch");
    sp_object_read_attr(object, "font-size");
    sp_object_read_attr(object, "unicode-range");
    sp_object_read_attr(object, "units-per-em");
    sp_object_read_attr(object, "panose-1");
    sp_object_read_attr(object, "stem-v");
    sp_object_read_attr(object, "stem-h");
    sp_object_read_attr(object, "slope");
    sp_object_read_attr(object, "cap-height");
    sp_object_read_attr(object, "x-height");
    sp_object_read_attr(object, "accent-height");
    sp_object_read_attr(object, "ascent");
    sp_object_read_attr(object, "descent");
    sp_object_read_attr(object, "widths");
    sp_object_read_attr(object, "bbox");
    sp_object_read_attr(object, "ideographic");
    sp_object_read_attr(object, "alphabetic");
    sp_object_read_attr(object, "mathematical");
    sp_object_read_attr(object, "ranging");
    sp_object_read_attr(object, "v-ideogaphic");
    sp_object_read_attr(object, "v-alphabetic");
    sp_object_read_attr(object, "v-mathematical");
    sp_object_read_attr(object, "v-hanging");
    sp_object_read_attr(object, "underline-position");
    sp_object_read_attr(object, "underline-thickness");
    sp_object_read_attr(object, "strikethrough-position");
    sp_object_read_attr(object, "strikethrough-thickness");
    sp_object_read_attr(object, "overline-position");
    sp_object_read_attr(object, "overline-thickness");
}

static void sp_fontface_children_modified(SPFontFace *sp_fontface)
{
}

/**
 * Callback for child_added event.
 */
static void
sp_fontface_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    SPFontFace *f = SP_FONTFACE(object);

    if (((SPObjectClass *) parent_class)->child_added)
        (* ((SPObjectClass *) parent_class)->child_added)(object, child, ref);

    sp_fontface_children_modified(f);
    object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}


/**
 * Callback for remove_child event.
 */
static void
sp_fontface_remove_child(SPObject *object, Inkscape::XML::Node *child)
{
    SPFontFace *f = SP_FONTFACE(object);

    if (((SPObjectClass *) parent_class)->remove_child)
        (* ((SPObjectClass *) parent_class)->remove_child)(object, child);

    sp_fontface_children_modified(f);
    object->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static void sp_fontface_release(SPObject *object)
{
    //SPFontFace *font = SP_FONTFACE(object);

    if (((SPObjectClass *) parent_class)->release) {
        ((SPObjectClass *) parent_class)->release(object);
    }
}

static void sp_fontface_set(SPObject *object, unsigned int key, const gchar *value)
{
    SPFontFace *face = SP_FONTFACE(object);
    double number;
    std::vector<FontFaceStyleType> style;
    std::vector<FontFaceVariantType> variant;
    std::vector<FontFaceWeightType> weight;
    std::vector<FontFaceStretchType> stretch;

    switch (key) {
	case SP_PROP_FONT_STYLE:
            style = sp_read_fontFaceStyleType(value);
            if (face->font_style.size() != style.size()){
                face->font_style = style;
g_warning("<font-face>: SP_ATTR_FONT_STYLE:");
                for (unsigned int i=0;i<style.size();i++){
                    g_warning("enum value: %d", style[i]);
                }
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            } else {
                for (unsigned int i=0;i<style.size();i++){
                    if(style[i] != face->font_style[i]){
                        face->font_style = style;
g_warning("<font-face>: SP_ATTR_FONT_STYLE:");
                        for (unsigned int i=0;i<style.size();i++){
                            g_warning("enum value: %d", style[i]);
                        }
                        object->requestModified(SP_OBJECT_MODIFIED_FLAG);
                        break;
                    }
                }
            }
            break;
	case SP_PROP_FONT_VARIANT:
            variant = sp_read_fontFaceVariantType(value);
            if (face->font_variant.size() != variant.size()){
                face->font_variant = variant;
g_warning("<font-face>: SP_ATTR_FONT_VARIANT:");
                for (unsigned int i=0;i<variant.size();i++){
                    g_warning("enum value: %d", variant[i]);
                }
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            } else {
                for (unsigned int i=0;i<variant.size();i++){
                    if(variant[i] != face->font_variant[i]){
                        face->font_variant = variant;
g_warning("<font-face>: SP_ATTR_FONT_VARIANT:");
                        for (unsigned int i=0;i<variant.size();i++){
                            g_warning("- %d", variant[i]);
                        }
                        object->requestModified(SP_OBJECT_MODIFIED_FLAG);
                        break;
                    }
                }
            }
            break;
	case SP_PROP_FONT_WEIGHT:
            weight = sp_read_fontFaceWeightType(value);
            if (face->font_weight.size() != weight.size()){
                face->font_weight = weight;
g_warning("<font-face>: SP_ATTR_FONT_WEIGHT:");
                for (unsigned int i=0;i<weight.size();i++){
                    g_warning("enum value: %d", weight[i]);
                }
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            } else {
                for (unsigned int i=0;i<weight.size();i++){
                    if(weight[i] != face->font_weight[i]){
                        face->font_weight = weight;
g_warning("<font-face>: SP_ATTR_FONT_WEIGHT:");
                        for (unsigned int i=0;i<weight.size();i++){
                            g_warning("enum value: %d", weight[i]);
                        }
                        object->requestModified(SP_OBJECT_MODIFIED_FLAG);
                        break;
                    }
                }
            }
            break;
	case SP_PROP_FONT_STRETCH:
            stretch = sp_read_fontFaceStretchType(value);
            if (face->font_stretch.size() != stretch.size()){
                face->font_stretch = stretch;
g_warning("<font-face>: SP_ATTR_FONT_STRETCH:");
                for (unsigned int i=0;i<stretch.size();i++){
                    g_warning("enum value: %d", stretch[i]);
                }
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            } else {
                for (unsigned int i=0;i<stretch.size();i++){
                    if(stretch[i] != face->font_stretch[i]){
                        face->font_stretch = stretch;
g_warning("<font-face>: SP_ATTR_FONT_STRETCH:");
                        for (unsigned int i=0;i<stretch.size();i++){
                            g_warning("enum value: %d", stretch[i]);
                        }
                        object->requestModified(SP_OBJECT_MODIFIED_FLAG);
                        break;
                    }
                }
            }
            break;
	case SP_ATTR_UNITS_PER_EM:
            number = helperfns_read_number(value);
            if (number != face->units_per_em){
                face->units_per_em = number;
g_warning("<font-face>: SP_ATTR_UNITS_PER_EM: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_STEMV:
            number = helperfns_read_number(value);
            if (number != face->stemv){
                face->stemv = number;
g_warning("<font-face>: SP_ATTR_STEMV: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_STEMH:
            number = helperfns_read_number(value);
            if (number != face->stemh){
                face->stemh = number;
g_warning("<font-face>: SP_ATTR_STEMH: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_SLOPE:
            number = helperfns_read_number(value);
            if (number != face->slope){
                face->slope = number;
g_warning("<font-face>: SP_ATTR_SLOPE: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_CAP_HEIGHT:
            number = helperfns_read_number(value);
            if (number != face->cap_height){
                face->cap_height = number;
g_warning("<font-face>: SP_ATTR_CAP_HEIGHT: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_X_HEIGHT:
            number = helperfns_read_number(value);
            if (number != face->x_height){
                face->x_height = number;
g_warning("<font-face>: SP_ATTR_X_HEIGHT: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_ACCENT_HEIGHT:
            number = helperfns_read_number(value);
            if (number != face->accent_height){
                face->accent_height = number;
g_warning("<font-face>: SP_ATTR_ACCENT_HEIGHT: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_ASCENT:
            number = helperfns_read_number(value);
            if (number != face->ascent){
                face->ascent = number;
g_warning("<font-face>: SP_ATTR_ASCENT: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_DESCENT:
            number = helperfns_read_number(value);
            if (number != face->descent){
                face->descent = number;
g_warning("<font-face>: SP_ATTR_DESCENT: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_IDEOGRAPHIC:
            number = helperfns_read_number(value);
            if (number != face->ideographic){
                face->ideographic = number;
g_warning("<font-face>: SP_ATTR_IDEOGRAPHIC: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_ALPHABETIC:
            number = helperfns_read_number(value);
            if (number != face->alphabetic){
                face->alphabetic = number;
g_warning("<font-face>: SP_ATTR_ALPHABETIC: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_MATHEMATICAL:
            number = helperfns_read_number(value);
            if (number != face->mathematical){
                face->mathematical = number;
g_warning("<font-face>: SP_ATTR_MATHEMATICAL: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_HANGING:
            number = helperfns_read_number(value);
            if (number != face->hanging){
                face->hanging = number;
g_warning("<font-face>: SP_ATTR_HANGING: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_V_IDEOGRAPHIC:
            number = helperfns_read_number(value);
            if (number != face->v_ideographic){
                face->v_ideographic = number;
g_warning("<font-face>: SP_ATTR_V_IDEOGRAPHIC: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_V_ALPHABETIC:
            number = helperfns_read_number(value);
            if (number != face->v_alphabetic){
                face->v_alphabetic = number;
g_warning("<font-face>: SP_ATTR_V_ALPHABETIC: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_V_MATHEMATICAL:
            number = helperfns_read_number(value);
            if (number != face->v_mathematical){
                face->v_mathematical = number;
g_warning("<font-face>: SP_ATTR_V_MATHEMATICAL: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_V_HANGING:
            number = helperfns_read_number(value);
            if (number != face->v_hanging){
                face->v_hanging = number;
g_warning("<font-face>: SP_ATTR_V_HANGING: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_UNDERLINE_POSITION:
            number = helperfns_read_number(value);
            if (number != face->underline_position){
                face->underline_position = number;
g_warning("<font-face>: SP_ATTR_UNDERLINE_POSITION: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_UNDERLINE_THICKNESS:
            number = helperfns_read_number(value);
            if (number != face->underline_thickness){
                face->underline_thickness = number;
g_warning("<font-face>: SP_ATTR_UNDERLINE_THICKNESS: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_STRIKETHROUGH_POSITION:
            number = helperfns_read_number(value);
            if (number != face->strikethrough_position){
                face->strikethrough_position = number;
g_warning("<font-face>: SP_ATTR_STRIKETHROUGH_POSITION: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_STRIKETHROUGH_THICKNESS:
            number = helperfns_read_number(value);
            if (number != face->strikethrough_thickness){
                face->strikethrough_thickness = number;
g_warning("<font-face>: SP_ATTR_STRIKETHROUGH_THICKNESS: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_OVERLINE_POSITION:
            number = helperfns_read_number(value);
            if (number != face->overline_position){
                face->overline_position = number;
g_warning("<font-face>: SP_ATTR_OVERLINE_POSITION: %f", number);
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;
	case SP_ATTR_OVERLINE_THICKNESS:
            number = helperfns_read_number(value);
            if (number != face->overline_thickness){
                face->overline_thickness = number;
g_warning("<font-face>: SP_ATTR_OVERLINE_THICKNESS: %f", number);
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

/**
 * Receives update notifications.
 */
static void
sp_fontface_update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG)) {
        sp_object_read_attr(object, "font-family");
        sp_object_read_attr(object, "font-style");
        sp_object_read_attr(object, "font-variant");
        sp_object_read_attr(object, "font-weight");
        sp_object_read_attr(object, "font-stretch");
        sp_object_read_attr(object, "font-size");
        sp_object_read_attr(object, "unicode-range");
        sp_object_read_attr(object, "units-per-em");
        sp_object_read_attr(object, "panose-1");
        sp_object_read_attr(object, "stemv");
        sp_object_read_attr(object, "stemh");
        sp_object_read_attr(object, "slope");
        sp_object_read_attr(object, "cap-height");
        sp_object_read_attr(object, "x-height");
        sp_object_read_attr(object, "accent-height");
        sp_object_read_attr(object, "ascent");
        sp_object_read_attr(object, "descent");
        sp_object_read_attr(object, "widths");
        sp_object_read_attr(object, "bbox");
        sp_object_read_attr(object, "ideographic");
        sp_object_read_attr(object, "alphabetic");
        sp_object_read_attr(object, "mathematical");
        sp_object_read_attr(object, "hanging");
        sp_object_read_attr(object, "v-ideographic");
        sp_object_read_attr(object, "v-alphabetic");
        sp_object_read_attr(object, "v-mathematical");
        sp_object_read_attr(object, "v-hanging");
        sp_object_read_attr(object, "underline-position");
        sp_object_read_attr(object, "underline-thickness");
        sp_object_read_attr(object, "strikethrough-position");
        sp_object_read_attr(object, "strikethrough-thickness");
        sp_object_read_attr(object, "overline-position");
        sp_object_read_attr(object, "overline-thickness");
    }

    if (((SPObjectClass *) parent_class)->update) {
        ((SPObjectClass *) parent_class)->update(object, ctx, flags);
    }
}

#define COPY_ATTR(rd,rs,key) (rd)->setAttribute((key), rs->attribute(key));

static Inkscape::XML::Node *sp_fontface_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPFontFace *face = SP_FONTFACE(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:font-face");
    }

    //TODO:
    //sp_repr_set_svg_double(repr, "font-family", face->font_family);
    //sp_repr_set_svg_double(repr, "font-style", face->font_style);
    //sp_repr_set_svg_double(repr, "font-variant", face->font_variant);
    //sp_repr_set_svg_double(repr, "font-weight", face->font_weight);
    //sp_repr_set_svg_double(repr, "font-stretch", face->font_stretch);
    //sp_repr_set_svg_double(repr, "font-size", face->font_size);
    //sp_repr_set_svg_double(repr, "unicode-range", face->unicode_range);
    sp_repr_set_svg_double(repr, "units-per-em", face->units_per_em);
    //sp_repr_set_svg_double(repr, "panose-1", face->panose_1);
    sp_repr_set_svg_double(repr, "stemv", face->stemv);
    sp_repr_set_svg_double(repr, "stemh", face->stemh);
    sp_repr_set_svg_double(repr, "slope", face->slope);
    sp_repr_set_svg_double(repr, "cap-height", face->cap_height);
    sp_repr_set_svg_double(repr, "x-height", face->x_height);
    sp_repr_set_svg_double(repr, "accent-height", face->accent_height);
    sp_repr_set_svg_double(repr, "ascent", face->ascent);
    sp_repr_set_svg_double(repr, "descent", face->descent);
    //sp_repr_set_svg_double(repr, "widths", face->widths);
    //sp_repr_set_svg_double(repr, "bbox", face->bbox);
    sp_repr_set_svg_double(repr, "ideographic", face->ideographic);
    sp_repr_set_svg_double(repr, "alphabetic", face->alphabetic);
    sp_repr_set_svg_double(repr, "mathematical", face->mathematical);
    sp_repr_set_svg_double(repr, "hanging", face->hanging);
    sp_repr_set_svg_double(repr, "v-ideographic", face->v_ideographic);
    sp_repr_set_svg_double(repr, "v-alphabetic", face->v_alphabetic);
    sp_repr_set_svg_double(repr, "v-mathematical", face->v_mathematical);
    sp_repr_set_svg_double(repr, "v-hanging", face->v_hanging);
    sp_repr_set_svg_double(repr, "underline-position", face->underline_position);
    sp_repr_set_svg_double(repr, "underline-thickness", face->underline_thickness);
    sp_repr_set_svg_double(repr, "strikethrough-position", face->strikethrough_position);
    sp_repr_set_svg_double(repr, "strikethrough-thickness", face->strikethrough_thickness);
    sp_repr_set_svg_double(repr, "overline-position", face->overline_position);
    sp_repr_set_svg_double(repr, "overline-thickness", face->overline_thickness);

    if (repr != SP_OBJECT_REPR(object)) {
        COPY_ATTR(repr, object->repr, "font-family");
        COPY_ATTR(repr, object->repr, "font-style");
        COPY_ATTR(repr, object->repr, "font-variant");
        COPY_ATTR(repr, object->repr, "font-weight");
        COPY_ATTR(repr, object->repr, "font-stretch");
        COPY_ATTR(repr, object->repr, "font-size");
        COPY_ATTR(repr, object->repr, "unicode-range");
        COPY_ATTR(repr, object->repr, "units-per-em");
        COPY_ATTR(repr, object->repr, "panose-1");
        COPY_ATTR(repr, object->repr, "stemv");
        COPY_ATTR(repr, object->repr, "stemh");
        COPY_ATTR(repr, object->repr, "slope");
        COPY_ATTR(repr, object->repr, "cap-height");
        COPY_ATTR(repr, object->repr, "x-height");
        COPY_ATTR(repr, object->repr, "accent-height");
        COPY_ATTR(repr, object->repr, "ascent");
        COPY_ATTR(repr, object->repr, "descent");
        COPY_ATTR(repr, object->repr, "widths");
        COPY_ATTR(repr, object->repr, "bbox");
        COPY_ATTR(repr, object->repr, "ideographic");
        COPY_ATTR(repr, object->repr, "alphabetic");
        COPY_ATTR(repr, object->repr, "mathematical");
        COPY_ATTR(repr, object->repr, "hanging");
        COPY_ATTR(repr, object->repr, "v-ideographic");
        COPY_ATTR(repr, object->repr, "v-alphabetic");
        COPY_ATTR(repr, object->repr, "v-mathematical");
        COPY_ATTR(repr, object->repr, "v-hanging");
        COPY_ATTR(repr, object->repr, "underline-position");
        COPY_ATTR(repr, object->repr, "underline-thickness");
        COPY_ATTR(repr, object->repr, "strikethrough-position");
        COPY_ATTR(repr, object->repr, "strikethrough-thickness");
        COPY_ATTR(repr, object->repr, "overline-position");
        COPY_ATTR(repr, object->repr, "overline-thickness");
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
