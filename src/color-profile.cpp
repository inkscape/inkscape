

#include "xml/repr.h"
#include "color-profile.h"
#include "color-profile-fns.h"
#include "attributes.h"
#include "inkscape.h"
#include "document.h"

#include "dom/uri.h"

//#define DEBUG_LCMS

#ifdef DEBUG_LCMS
#include "prefs-utils.h"
#include <gtk/gtkmessagedialog.h>
#endif // DEBUG_LCMS

using Inkscape::ColorProfile;
using Inkscape::ColorProfileClass;

namespace Inkscape
{
static void colorprofile_class_init( ColorProfileClass *klass );
static void colorprofile_init( ColorProfile *cprof );

static void colorprofile_release( SPObject *object );
static void colorprofile_build( SPObject *object, SPDocument *document, Inkscape::XML::Node *repr );
static void colorprofile_set( SPObject *object, unsigned key, gchar const *value );
static Inkscape::XML::Node *colorprofile_write( SPObject *object, Inkscape::XML::Node *repr, guint flags );
}

#ifdef DEBUG_LCMS
extern guint update_in_progress;
#define DEBUG_MESSAGE(key, ...) \
{\
    gint dump = prefs_get_int_attribute_limited("options.scislac", #key, 0, 0, 1);\
    gint dumpD = prefs_get_int_attribute_limited("options.scislac", #key"D", 0, 0, 1);\
    gint dumpD2 = prefs_get_int_attribute_limited("options.scislac", #key"D2", 0, 0, 1);\
    dumpD &= ( (update_in_progress == 0) || dumpD2 );\
    if ( dump )\
    {\
        g_message( __VA_ARGS__ );\
\
    }\
    if ( dumpD )\
    {\
        GtkWidget *dialog = gtk_message_dialog_new(NULL,\
                                                   GTK_DIALOG_DESTROY_WITH_PARENT, \
                                                   GTK_MESSAGE_INFO,    \
                                                   GTK_BUTTONS_OK,      \
                                                   __VA_ARGS__          \
                                                   );\
        g_signal_connect_swapped(dialog, "response",\
                                 G_CALLBACK(gtk_widget_destroy),        \
                                 dialog);                               \
        gtk_widget_show_all( dialog );\
    }\
}
#endif // DEBUG_LCMS

static SPObject *cprof_parent_class;

/**
 * Register ColorProfile class and return its type.
 */
GType Inkscape::colorprofile_get_type()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(ColorProfileClass),
            NULL, NULL,
            (GClassInitFunc) colorprofile_class_init,
            NULL, NULL,
            sizeof(ColorProfile),
            16,
            (GInstanceInitFunc) colorprofile_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static( SP_TYPE_OBJECT, "ColorProfile", &info, static_cast<GTypeFlags>(0) );
    }
    return type;
}

/**
 * ColorProfile vtable initialization.
 */
static void Inkscape::colorprofile_class_init( ColorProfileClass *klass )
{
    SPObjectClass *sp_object_class = reinterpret_cast<SPObjectClass *>(klass);

    cprof_parent_class = static_cast<SPObject*>(g_type_class_ref(SP_TYPE_OBJECT));

    sp_object_class->release = colorprofile_release;
    sp_object_class->build = colorprofile_build;
    sp_object_class->set = colorprofile_set;
    sp_object_class->write = colorprofile_write;
}

/**
 * Callback for ColorProfile object initialization.
 */
static void Inkscape::colorprofile_init( ColorProfile *cprof )
{
    cprof->href = 0;
    cprof->local = 0;
    cprof->name = 0;
    cprof->intentStr = 0;
    cprof->rendering_intent = Inkscape::RENDERING_INTENT_UNKNOWN;
#if ENABLE_LCMS
    cprof->profHandle = 0;
#endif // ENABLE_LCMS
}

/**
 * Callback: free object
 */
static void Inkscape::colorprofile_release( SPObject *object )
{
    ColorProfile *cprof = COLORPROFILE(object);
    if ( cprof->href ) {
        g_free( cprof->href );
        cprof->href = 0;
    }

    if ( cprof->local ) {
        g_free( cprof->local );
        cprof->local = 0;
    }

    if ( cprof->name ) {
        g_free( cprof->name );
        cprof->name = 0;
    }

    if ( cprof->intentStr ) {
        g_free( cprof->intentStr );
        cprof->intentStr = 0;
    }

#if ENABLE_LCMS
    if ( cprof->profHandle ) {
        cmsCloseProfile( cprof->profHandle );
        cprof->profHandle = 0;
    }
#endif // ENABLE_LCMS
}

/**
 * Callback: set attributes from associated repr.
 */
static void Inkscape::colorprofile_build( SPObject *object, SPDocument *document, Inkscape::XML::Node *repr )
{
    ColorProfile *cprof = COLORPROFILE(object);
    g_assert(cprof->href == 0);
    g_assert(cprof->local == 0);
    g_assert(cprof->name == 0);
    g_assert(cprof->intentStr == 0);

    if (((SPObjectClass *) cprof_parent_class)->build) {
        (* ((SPObjectClass *) cprof_parent_class)->build)(object, document, repr);
    }
    sp_object_read_attr( object, "xlink:href" );
    sp_object_read_attr( object, "local" );
    sp_object_read_attr( object, "name" );
    sp_object_read_attr( object, "rendering-intent" );
}

/**
 * Callback: set attribute.
 */
static void Inkscape::colorprofile_set( SPObject *object, unsigned key, gchar const *value )
{
    ColorProfile *cprof = COLORPROFILE(object);

    switch (key) {
        case SP_ATTR_XLINK_HREF:
            if ( cprof->href ) {
                g_free( cprof->href );
                cprof->href = 0;
            }
            if ( value ) {
                cprof->href = g_strdup( value );
                if ( *cprof->href ) {
#if ENABLE_LCMS
                    cmsErrorAction( LCMS_ERROR_SHOW );

                    // TODO open filename and URIs properly
                    //FILE* fp = fopen_utf8name( filename, "r" );
                    //LCMSAPI cmsHPROFILE   LCMSEXPORT cmsOpenProfileFromMem(LPVOID MemPtr, DWORD dwSize);

                    // Try to open relative
                    SPDocument *doc = SP_OBJECT_DOCUMENT(object);
                    if (!doc) {
                        doc = SP_ACTIVE_DOCUMENT;
                        g_warning("object has no document.  using active");
                    }
                    //# 1.  Get complete URI of document
                    gchar* docbase = SP_DOCUMENT_URI( doc );
                    if (!docbase)
                        {
                        g_warning("null docbase");
                        docbase = "";
                        }
                    //g_message("docbase:%s\n", docbase);
                    org::w3c::dom::URI docUri(docbase);
                    //# 2. Get href of icc file.  we don't care if it's rel or abs
                    org::w3c::dom::URI hrefUri(cprof->href);
                    //# 3.  Resolve the href according the docBase.  This follows
                    //      the w3c specs.  All absolute and relative issues are considered
                    org::w3c::dom::URI cprofUri = docUri.resolve(hrefUri);
                    gchar* fullname = (gchar *)cprofUri.getNativePath().c_str();
                    cprof->profHandle = cmsOpenProfileFromFile( fullname, "r" );
#ifdef DEBUG_LCMS
                    DEBUG_MESSAGE( lcmsOne, "cmsOpenProfileFromFile( '%s'...) = %p", fullname, (void*)cprof->profHandle );
#endif // DEBUG_LCMS

#endif // ENABLE_LCMS
                }
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_LOCAL:
            if ( cprof->local ) {
                g_free( cprof->local );
                cprof->local = 0;
            }
            cprof->local = g_strdup( value );
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_NAME:
            if ( cprof->name ) {
                g_free( cprof->name );
                cprof->name = 0;
            }
            cprof->name = g_strdup( value );
#ifdef DEBUG_LCMS
            DEBUG_MESSAGE( lcmsTwo, "<color-profile> name set to '%s'", cprof->name );
#endif // DEBUG_LCMS
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_RENDERING_INTENT:
            if ( cprof->intentStr ) {
                g_free( cprof->intentStr );
                cprof->intentStr = 0;
            }
            cprof->intentStr = g_strdup( value );

            if ( value ) {
                if ( strcmp( value, "auto" ) == 0 ) {
                    cprof->rendering_intent = RENDERING_INTENT_AUTO;
                } else if ( strcmp( value, "perceptual" ) == 0 ) {
                    cprof->rendering_intent = RENDERING_INTENT_PERCEPTUAL;
                } else if ( strcmp( value, "relative-colorimetric" ) == 0 ) {
                    cprof->rendering_intent = RENDERING_INTENT_RELATIVE_COLORIMETRIC;
                } else if ( strcmp( value, "saturation" ) == 0 ) {
                    cprof->rendering_intent = RENDERING_INTENT_SATURATION;
                } else if ( strcmp( value, "absolute-colorimetric" ) == 0 ) {
                    cprof->rendering_intent = RENDERING_INTENT_ABSOLUTE_COLORIMETRIC;
                } else {
                    cprof->rendering_intent = RENDERING_INTENT_UNKNOWN;
                }
            } else {
                cprof->rendering_intent = RENDERING_INTENT_UNKNOWN;
            }

            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        default:
            if (((SPObjectClass *) cprof_parent_class)->set) {
                (* ((SPObjectClass *) cprof_parent_class)->set)(object, key, value);
            }
            break;
    }

}

/**
 * Callback: write attributes to associated repr.
 */
static Inkscape::XML::Node* Inkscape::colorprofile_write( SPObject *object, Inkscape::XML::Node *repr, guint flags )
{
    ColorProfile *cprof = COLORPROFILE(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(SP_OBJECT_DOCUMENT(object));
        repr = xml_doc->createElement("svg:color-profile");
    }

    if ( (flags & SP_OBJECT_WRITE_ALL) || cprof->href ) {
        repr->setAttribute( "xlink:href", cprof->href );
    }

    if ( (flags & SP_OBJECT_WRITE_ALL) || cprof->local ) {
        repr->setAttribute( "local", cprof->local );
    }

    if ( (flags & SP_OBJECT_WRITE_ALL) || cprof->name ) {
        repr->setAttribute( "name", cprof->name );
    }

    if ( (flags & SP_OBJECT_WRITE_ALL) || cprof->intentStr ) {
        repr->setAttribute( "rendering-intent", cprof->intentStr );
    }

    if (((SPObjectClass *) cprof_parent_class)->write) {
        (* ((SPObjectClass *) cprof_parent_class)->write)(object, repr, flags);
    }

    return repr;
}


#if ENABLE_LCMS


static SPObject* bruteFind( SPObject* curr, gchar* const name )
{
    SPObject* result = 0;

    if ( curr ) {
        if ( IS_COLORPROFILE(curr) ) {
            ColorProfile* prof = COLORPROFILE(curr);
            if ( prof ) {
                if ( prof->name && (strcmp(prof->name, name) == 0) ) {
                    result = curr;
                }
            }
        } else {
            if ( curr->hasChildren() ) {
                SPObject* child = curr->firstChild();
                while ( child && !result ) {
                    result = bruteFind( child, name );
                    if ( !result ) {
                        child = child->next;
                    }
                };
            }
        }
    }

    return result;
}

cmsHPROFILE Inkscape::colorprofile_get_handle( SPDocument* document, guint* intent, gchar* const name )
{
    cmsHPROFILE prof = 0;

    SPObject* root = SP_DOCUMENT_ROOT(document);
    SPObject* thing = bruteFind( root, name );
    if ( thing ) {
        prof = COLORPROFILE(thing)->profHandle;
    }

    if ( intent ) {
        *intent = thing ? COLORPROFILE(thing)->rendering_intent : (guint)RENDERING_INTENT_UNKNOWN;
    }

#ifdef DEBUG_LCMS
    DEBUG_MESSAGE( lcmsThree, "<color-profile> queried for profile of '%s'. Returning %p with intent of %d", name, prof, (intent? *intent:0) );
#endif // DEBUG_LCMS

    return prof;
}
#endif // ENABLE_LCMS

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
