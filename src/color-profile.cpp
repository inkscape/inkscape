

#include "xml/repr.h"
#include "color-profile.h"
#include "color-profile-fns.h"
#include "attributes.h"
#include "document.h"

using Inkscape::ColorProfile;
using Inkscape::ColorProfileClass;

static void colorprofile_class_init( ColorProfileClass *klass );
static void colorprofile_init( ColorProfile *cprof );

static void colorprofile_release( SPObject *object );
static void colorprofile_build( SPObject *object, SPDocument *document, Inkscape::XML::Node *repr );
static void colorprofile_set( SPObject *object, unsigned key, gchar const *value );
static Inkscape::XML::Node *colorprofile_write( SPObject *object, Inkscape::XML::Node *repr, guint flags );

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
static void colorprofile_class_init( ColorProfileClass *klass )
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
static void colorprofile_init( ColorProfile *cprof )
{
    cprof->href = 0;
    cprof->local = 0;
    cprof->name = 0;
    cprof->rendering_intent = 0;
#if ENABLE_LCMS
    cprof->profHandle = 0;
#endif // ENABLE_LCMS
}

/**
 * Callback: free object
 */
static void colorprofile_release( SPObject *object )
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
static void colorprofile_build( SPObject *object, SPDocument *document, Inkscape::XML::Node *repr )
{
    ColorProfile *cprof = COLORPROFILE(object);
    g_assert(cprof->href == 0);
    g_assert(cprof->local == 0);
    g_assert(cprof->name == 0);

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
static void colorprofile_set( SPObject *object, unsigned key, gchar const *value )
{
    ColorProfile *cprof = COLORPROFILE(object);

    switch (key) {
        case SP_ATTR_XLINK_HREF:
            if ( value ) {
                cprof->href = g_strdup( value );
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
                if ( *cprof->href ) {
#if ENABLE_LCMS
                    cmsErrorAction( LCMS_ERROR_SHOW );

                    // TODO open filename and URIs properly
                    //FILE* fp = fopen_utf8name( filename, "r" );
                    //LCMSAPI cmsHPROFILE   LCMSEXPORT cmsOpenProfileFromMem(LPVOID MemPtr, DWORD dwSize);

                    if ( !g_path_is_absolute(cprof->href) ) {
                        // Try to open relative
                        gchar* docbase = SP_DOCUMENT_BASE( SP_OBJECT_DOCUMENT(object) );
			gchar* fullname = g_build_filename( docbase ? docbase : ".", cprof->href, NULL );

                        cprof->profHandle = cmsOpenProfileFromFile( fullname, "r" );

			g_free (fullname);
                    } else {
                        cprof->profHandle = cmsOpenProfileFromFile( cprof->href, "r" );
                    }

#endif // ENABLE_LCMS
                }
            }
            break;

        case SP_ATTR_LOCAL:
            if ( value ) {
                cprof->local = g_strdup( value );
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;

        case SP_ATTR_NAME:
            if ( value ) {
                cprof->name = g_strdup( value );
                object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;

        case SP_ATTR_RENDERING_INTENT:
            if ( value ) {
// auto | perceptual | relative-colorimetric | saturation | absolute-colorimetric
                //cprof->name = g_strdup( value );
                //object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
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
static Inkscape::XML::Node* colorprofile_write( SPObject *object, Inkscape::XML::Node *repr, guint flags )
{
    ColorProfile *cprof = COLORPROFILE(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = sp_repr_new("svg:color-profile");
    }

    if ( (flags & SP_OBJECT_WRITE_ALL) || cprof->href ) {
        repr->setAttribute( "xlink:href", cprof->name );
    }

    if ( (flags & SP_OBJECT_WRITE_ALL) || cprof->href ) {
        repr->setAttribute( "local", cprof->name );
    }

    if ( (flags & SP_OBJECT_WRITE_ALL) || cprof->href ) {
        repr->setAttribute( "name", cprof->name );
    }

    if ( (flags & SP_OBJECT_WRITE_ALL) || cprof->href ) {
//        repr->setAttribute( "rendering-intent", cprof->name );
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

cmsHPROFILE Inkscape::colorprofile_get_handle( SPDocument* document, gchar* const name )
{
    cmsHPROFILE prof = 0;

    SPObject* root = SP_DOCUMENT_ROOT(document);
    SPObject* thing = bruteFind( root, name );
    if ( thing ) {
        prof = COLORPROFILE(thing)->profHandle;
    }

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
