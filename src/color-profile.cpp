

#include "xml/repr.h"
#include "color-profile.h"
#include "color-profile-fns.h"
#include "attributes.h"
#include "inkscape.h"
#include "document.h"
#include "prefs-utils.h"

#include "dom/uri.h"

//#define DEBUG_LCMS

#include <glib/gstdio.h>
#include <sys/fcntl.h>

#ifdef DEBUG_LCMS
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

static cmsHPROFILE colorprofile_get_system_profile_handle();
static cmsHPROFILE colorprofile_get_proof_profile_handle();
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
                    gchar const *docbase = SP_DOCUMENT_URI( doc );
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


static SPObject* bruteFind( SPObject* curr, gchar const* name )
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

cmsHPROFILE Inkscape::colorprofile_get_handle( SPDocument* document, guint* intent, gchar const* name )
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


#include <io/sys.h>

class ProfileInfo
{
public:
    ProfileInfo( cmsHPROFILE, Glib::ustring const & path );

    Glib::ustring const& getName() {return _name;}
    Glib::ustring const& getPath() {return _path;}
    icColorSpaceSignature getSpace() {return _profileSpace;}
    icProfileClassSignature getClass() {return _profileClass;}

private:
    Glib::ustring _path;
    Glib::ustring _name;
    icColorSpaceSignature _profileSpace;
    icProfileClassSignature _profileClass;
};


ProfileInfo::ProfileInfo( cmsHPROFILE prof, Glib::ustring const & path )
{
    _path = path;
    _name = cmsTakeProductName(prof);
    _profileSpace = cmsGetColorSpace( prof );
    _profileClass = cmsGetDeviceClass( prof );
}



static std::vector<ProfileInfo> knownProfiles;

std::vector<Glib::ustring> Inkscape::colorprofile_get_display_names()
{
    std::vector<Glib::ustring> result;

    for ( std::vector<ProfileInfo>::iterator it = knownProfiles.begin(); it != knownProfiles.end(); ++it ) {
        if ( it->getClass() == icSigDisplayClass && it->getSpace() == icSigRgbData ) {
            result.push_back( it->getName() );
        }
    }

    return result;
}

std::vector<Glib::ustring> Inkscape::colorprofile_get_softproof_names()
{
    std::vector<Glib::ustring> result;

    for ( std::vector<ProfileInfo>::iterator it = knownProfiles.begin(); it != knownProfiles.end(); ++it ) {
        if ( it->getClass() == icSigOutputClass ) {
            result.push_back( it->getName() );
        }
    }

    return result;
}

Glib::ustring Inkscape::get_path_for_profile(Glib::ustring const& name)
{
    Glib::ustring result;

    for ( std::vector<ProfileInfo>::iterator it = knownProfiles.begin(); it != knownProfiles.end(); ++it ) {
        if ( name == it->getName() ) {
            result = it->getPath();
            break;
        }
    }

    return result;
}

static void findThings() {
    std::list<gchar *> sources;

    gchar* base = profile_path("XXX");
    {
        gchar* base2 = g_path_get_dirname(base);
        g_free(base);
        base = base2;
        base2 = g_path_get_dirname(base);
        g_free(base);
        base = base2;
    }

    // first try user's local dir
    sources.push_back( g_build_filename(g_get_user_data_dir(), "color", "icc", NULL) );
    sources.push_back( g_build_filename(base, ".color", "icc", NULL) ); // OpenICC recommends to deprecate this

    const gchar* const * dataDirs = g_get_system_data_dirs();
    for ( int i = 0; dataDirs[i]; i++ ) {
        sources.push_back(g_build_filename(dataDirs[i], "color", "icc", NULL));
    }

    while (!sources.empty()) {
        gchar *dirname = sources.front();
        if ( g_file_test( dirname, G_FILE_TEST_EXISTS ) && g_file_test( dirname, G_FILE_TEST_IS_DIR ) ) {
            GError *err = 0;
            GDir *dir = g_dir_open(dirname, 0, &err);

            if (dir) {
                for (gchar const *file = g_dir_read_name(dir); file != NULL; file = g_dir_read_name(dir)) {
                    gchar *filepath = g_build_filename(dirname, file, NULL);


                    if ( g_file_test( filepath, G_FILE_TEST_IS_DIR ) ) {
                        sources.push_back(g_strdup(filepath));
                    } else {
                        bool isIccFile = false;
                        struct stat st;
                        if ( g_stat(filepath, &st) == 0 && (st.st_size > 128) ) {
                            //0-3 == size
                            //36-39 == 'acsp' 0x61637370
                            int fd = g_open( filepath, O_RDONLY, S_IRWXU);
                            if ( fd != -1 ) {
                                guchar scratch[40] = {0};
                                size_t len = sizeof(scratch);

                                //size_t left = 40;
                                ssize_t got = read(fd, scratch, len);
                                if ( got != -1 ) {
                                    size_t calcSize = (scratch[0] << 24) | (scratch[1] << 16) | (scratch[2] << 8) | scratch[3];
                                    if ( calcSize > 128 && calcSize <= st.st_size ) {
                                        isIccFile = (scratch[36] == 'a') && (scratch[37] == 'c') && (scratch[38] == 's') && (scratch[39] == 'p');
                                    }
                                }

                                close(fd);
                            }
                        }

                        if ( isIccFile ) {
                            cmsHPROFILE prof = cmsOpenProfileFromFile( filepath, "r" );
                            if ( prof ) {
                                ProfileInfo info( prof, Glib::filename_to_utf8( filepath ) );
                                cmsCloseProfile( prof );

                                bool sameName = false;
                                for ( std::vector<ProfileInfo>::iterator it = knownProfiles.begin(); it != knownProfiles.end(); ++it ) {
                                    if ( it->getName() == info.getName() ) {
                                        sameName = true;
                                        break;
                                    }
                                }

                                if ( !sameName ) {
                                    knownProfiles.push_back(info);
                                }
                            }
                        }
                    }

                    g_free(filepath);
                }
            }
        }

        // toss the dirname
        g_free(dirname);
        sources.pop_front();
    }
}

int errorHandlerCB(int ErrorCode, const char *ErrorText)
{
    g_message("lcms: Error %d; %s", ErrorCode, ErrorText);

    return 1;
}

static bool gamutWarn = false;
static int lastIntent = INTENT_PERCEPTUAL;
static int lastProofIntent = INTENT_PERCEPTUAL;
static cmsHTRANSFORM transf = 0;
static cmsHPROFILE srcprof = 0;

cmsHPROFILE Inkscape::colorprofile_get_system_profile_handle()
{
    static cmsHPROFILE theOne = 0;
    static std::string lastURI;

    static bool init = false;
    if ( !init ) {
        cmsSetErrorHandler(errorHandlerCB);

        findThings();
        init = true;
    }

    long long int which = prefs_get_int_attribute_limited( "options.displayprofile", "enable", 0, 0, 1 );
    gchar const * uri = prefs_get_string_attribute("options.displayprofile", "uri");

    if ( which && uri && *uri ) {
        if ( lastURI != std::string(uri) ) {
            lastURI.clear();
            if ( theOne ) {
                cmsCloseProfile( theOne );
            }
            if ( transf ) {
                cmsDeleteTransform( transf );
                transf = 0;
            }
            theOne = cmsOpenProfileFromFile( uri, "r" );
            if ( theOne ) {
                // a display profile must have the proper stuff
                icColorSpaceSignature space = cmsGetColorSpace(theOne);
                icProfileClassSignature profClass = cmsGetDeviceClass(theOne);

                if ( profClass != icSigDisplayClass ) {
                    g_warning("Not a display profile");
                    cmsCloseProfile( theOne );
                    theOne = 0;
                } else if ( space != icSigRgbData ) {
                    g_warning("Not an RGB profile");
                    cmsCloseProfile( theOne );
                    theOne = 0;
                } else {
                    lastURI = uri;
                }
            }
        }
    } else if ( theOne ) {
        cmsCloseProfile( theOne );
        theOne = 0;
        lastURI.clear();
        if ( transf ) {
            cmsDeleteTransform( transf );
            transf = 0;
        }
    }

    return theOne;
}


cmsHPROFILE Inkscape::colorprofile_get_proof_profile_handle()
{
    static cmsHPROFILE theOne = 0;
    static std::string lastURI;

    static bool init = false;
    if ( !init ) {
        cmsSetErrorHandler(errorHandlerCB);

        findThings();
        init = true;
    }

    long long int which = prefs_get_int_attribute_limited( "options.softproof", "enable", 0, 0, 1 );
    gchar const * uri = prefs_get_string_attribute("options.softproof", "uri");

    if ( which && uri && *uri ) {
        if ( lastURI != std::string(uri) ) {
            lastURI.clear();
            if ( theOne ) {
                cmsCloseProfile( theOne );
            }
            if ( transf ) {
                cmsDeleteTransform( transf );
                transf = 0;
            }
            theOne = cmsOpenProfileFromFile( uri, "r" );
            if ( theOne ) {
                // a display profile must have the proper stuff
                icColorSpaceSignature space = cmsGetColorSpace(theOne);
                icProfileClassSignature profClass = cmsGetDeviceClass(theOne);

/*
                if ( profClass != icSigDisplayClass ) {
                    g_warning("Not a display profile");
                    cmsCloseProfile( theOne );
                    theOne = 0;
                } else if ( space != icSigRgbData ) {
                    g_warning("Not an RGB profile");
                    cmsCloseProfile( theOne );
                    theOne = 0;
                } else {
*/
                    lastURI = uri;
/*
                }
*/
            }
        }
    } else if ( theOne ) {
        cmsCloseProfile( theOne );
        theOne = 0;
        lastURI.clear();
        if ( transf ) {
            cmsDeleteTransform( transf );
            transf = 0;
        }
    }

    return theOne;
}

cmsHTRANSFORM Inkscape::colorprofile_get_display_transform()
{
    bool warn = prefs_get_int_attribute_limited( "options.softproof", "gamutwarn", 0, 0, 1 );
    int intent = prefs_get_int_attribute_limited( "options.displayprofile", "intent", 0, 0, 3 );
    int proofIntent = prefs_get_int_attribute_limited( "options.softproof", "intent", 0, 0, 3 );

    if ( (warn != gamutWarn) || (lastIntent != intent) || (lastProofIntent != proofIntent)) {
        gamutWarn = warn;
        if ( transf ) {
            cmsDeleteTransform(transf);
            transf = 0;
        }
        lastIntent = intent;
        lastProofIntent = proofIntent;
    }

    // Fecth these now, as they might clear the transform as a side effect.
    cmsHPROFILE hprof = Inkscape::colorprofile_get_system_profile_handle();
    cmsHPROFILE proofProf = hprof ? Inkscape::colorprofile_get_proof_profile_handle() : 0;

    if ( !transf ) {
        if ( !srcprof ) {
            srcprof = cmsCreate_sRGBProfile();
        }
        if ( hprof && proofProf ) {
            DWORD dwFlags = cmsFLAGS_SOFTPROOFING;
            if ( gamutWarn ) {
                dwFlags |= cmsFLAGS_GAMUTCHECK;
            }
            cmsSetAlarmCodes(0, 255, 0);
            transf = cmsCreateProofingTransform( srcprof, TYPE_RGB_8, hprof, TYPE_RGB_8, proofProf, intent, proofIntent, dwFlags );
        } else if ( hprof ) {
            transf = cmsCreateTransform( srcprof, TYPE_RGB_8, hprof, TYPE_RGB_8, intent, 0 );
        }
    }

    return transf;
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
