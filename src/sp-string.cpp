#define __SP_STRING_C__

/*
 * SVG <text> and <tspan> implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
 * fixme:
 *
 * These subcomponents should not be items, or alternately
 * we have to invent set of flags to mark, whether standard
 * attributes are applicable to given item (I even like this
 * idea somewhat - Lauris)
 *
 */



#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


#include "sp-string.h"
#include "xml/repr.h"


/*#####################################################
#  SPSTRING
#####################################################*/

static void sp_string_class_init(SPStringClass *classname);
static void sp_string_init(SPString *string);

static void sp_string_build(SPObject *object, Document *document, Inkscape::XML::Node *repr);
static void sp_string_release(SPObject *object);
static void sp_string_read_content(SPObject *object);
static void sp_string_update(SPObject *object, SPCtx *ctx, unsigned flags);

static SPObjectClass *string_parent_class;

GType
sp_string_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPStringClass),
            NULL,    /* base_init */
            NULL,    /* base_finalize */
            (GClassInitFunc) sp_string_class_init,
            NULL,    /* class_finalize */
            NULL,    /* class_data */
            sizeof(SPString),
            16,    /* n_preallocs */
            (GInstanceInitFunc) sp_string_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static(SP_TYPE_OBJECT, "SPString", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_string_class_init(SPStringClass *classname)
{
    SPObjectClass *sp_object_class;

    sp_object_class = (SPObjectClass *) classname;

    string_parent_class = (SPObjectClass*)g_type_class_ref(SP_TYPE_OBJECT);

    sp_object_class->build        = sp_string_build;
    sp_object_class->release      = sp_string_release;
    sp_object_class->read_content = sp_string_read_content;
    sp_object_class->update       = sp_string_update;
}

static void
sp_string_init(SPString *string)
{
    new (&string->string) Glib::ustring();
}

static void
sp_string_build(SPObject *object, Document *doc, Inkscape::XML::Node *repr)
{
    sp_string_read_content(object);

    if (((SPObjectClass *) string_parent_class)->build)
        ((SPObjectClass *) string_parent_class)->build(object, doc, repr);
}

static void
sp_string_release(SPObject *object)
{
    SPString *string = SP_STRING(object);

    string->string.~ustring();

    if (((SPObjectClass *) string_parent_class)->release)
        ((SPObjectClass *) string_parent_class)->release(object);
}

static void
sp_string_read_content(SPObject *object)
{
    SPString *string = SP_STRING(object);

    string->string.clear();
    gchar const *xml_string = string->repr->content();
    // see algorithms described in svg 1.1 section 10.15
    if (object->xml_space.value == SP_XML_SPACE_PRESERVE) {
        for ( ; *xml_string ; xml_string = g_utf8_next_char(xml_string) ) {
            gunichar c = g_utf8_get_char(xml_string);
            if (c == 0xa || c == 0xd || c == '\t') c = ' ';
            string->string += c;
        }
    }
    else {
        bool whitespace = false;
        for ( ; *xml_string ; xml_string = g_utf8_next_char(xml_string) ) {
            gunichar c = g_utf8_get_char(xml_string);
            if (c == 0xa || c == 0xd) continue;
            if (c == ' ' || c == '\t') whitespace = true;
            else {
                if (whitespace && (!string->string.empty() || SP_OBJECT_PREV(object) != NULL))
                    string->string += ' ';
                string->string += c;
                whitespace = false;
            }
        }
        if (whitespace && SP_OBJECT_REPR(object)->next() != NULL)   // can't use SP_OBJECT_NEXT() when the SPObject tree is still being built
            string->string += ' ';
    }
    object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_string_update(SPObject *object, SPCtx *ctx, unsigned flags)
{
    if (((SPObjectClass *) string_parent_class)->update)
        ((SPObjectClass *) string_parent_class)->update(object, ctx, flags);

    if (flags & (SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_MODIFIED_FLAG)) {
        /* Parent style or we ourselves changed, so recalculate */
        flags &= ~SP_OBJECT_USER_MODIFIED_FLAG_B; // won't be "just a transformation" anymore, we're going to recompute "x" and "y" attributes
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
