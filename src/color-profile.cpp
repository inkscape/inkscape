

//#define DEBUG_LCMS

#include <glib/gstdio.h>
#include <sys/fcntl.h>
#include <gdkmm/color.h>

#ifdef DEBUG_LCMS
#include <gtk/gtkmessagedialog.h>
#endif // DEBUG_LCMS

#include <cstring>
#include <string>
#include "xml/repr.h"
#include "color-profile.h"
#include "color-profile-fns.h"
#include "attributes.h"
#include "inkscape.h"
#include "document.h"
#include "preferences.h"

#include "dom/uri.h"
#include "dom/util/digest.h"

using Inkscape::ColorProfile;
using Inkscape::ColorProfileClass;

namespace Inkscape
{
#if ENABLE_LCMS
static cmsHPROFILE colorprofile_get_system_profile_handle();
static cmsHPROFILE colorprofile_get_proof_profile_handle();
#endif // ENABLE_LCMS
}

#ifdef DEBUG_LCMS
extern guint update_in_progress;
#define DEBUG_MESSAGE(key, ...) \
{\
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();\
    bool dump = prefs->getBool(Glib::ustring("/options/scislac/") + #key);\
    bool dumpD = prefs->getBool(Glib::ustring("/options/scislac/") + #key"D");\
    bool dumpD2 = prefs->getBool(Glib::ustring("/options/scislac/") + #key"D2");\
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

static SPObjectClass *cprof_parent_class;

#if ENABLE_LCMS

cmsHPROFILE ColorProfile::_sRGBProf = 0;

cmsHPROFILE ColorProfile::getSRGBProfile() {
    if ( !_sRGBProf ) {
        _sRGBProf = cmsCreate_sRGBProfile();
    }
    return _sRGBProf;
}

#endif // ENABLE_LCMS

/**
 * Register ColorProfile class and return its type.
 */
GType Inkscape::colorprofile_get_type()
{
    return ColorProfile::getType();
}

GType ColorProfile::getType()
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(ColorProfileClass),
            NULL, NULL,
            (GClassInitFunc) ColorProfile::classInit,
            NULL, NULL,
            sizeof(ColorProfile),
            16,
            (GInstanceInitFunc) ColorProfile::init,
            NULL,   /* value_table */
        };
        type = g_type_register_static( SP_TYPE_OBJECT, "ColorProfile", &info, static_cast<GTypeFlags>(0) );
    }
    return type;
}

/**
 * ColorProfile vtable initialization.
 */
void ColorProfile::classInit( ColorProfileClass *klass )
{
    SPObjectClass *sp_object_class = reinterpret_cast<SPObjectClass *>(klass);

    cprof_parent_class = static_cast<SPObjectClass*>(g_type_class_ref(SP_TYPE_OBJECT));

    sp_object_class->release = ColorProfile::release;
    sp_object_class->build = ColorProfile::build;
    sp_object_class->set = ColorProfile::set;
    sp_object_class->write = ColorProfile::write;
}

/**
 * Callback for ColorProfile object initialization.
 */
void ColorProfile::init( ColorProfile *cprof )
{
    cprof->href = 0;
    cprof->local = 0;
    cprof->name = 0;
    cprof->intentStr = 0;
    cprof->rendering_intent = Inkscape::RENDERING_INTENT_UNKNOWN;
#if ENABLE_LCMS
    cprof->profHandle = 0;
    cprof->_profileClass = icSigInputClass;
    cprof->_profileSpace = icSigRgbData;
    cprof->_transf = 0;
    cprof->_revTransf = 0;
#endif // ENABLE_LCMS
}

/**
 * Callback: free object
 */
void ColorProfile::release( SPObject *object )
{
    // Unregister ourselves
    SPDocument* document = SP_OBJECT_DOCUMENT(object);
    if ( document ) {
        sp_document_remove_resource (SP_OBJECT_DOCUMENT (object), "iccprofile", SP_OBJECT (object));
    }

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
    cprof->_clearProfile();
#endif // ENABLE_LCMS
}

#if ENABLE_LCMS
void ColorProfile::_clearProfile()
{
    _profileSpace = icSigRgbData;

    if ( _transf ) {
        cmsDeleteTransform( _transf );
        _transf = 0;
    }
    if ( _revTransf ) {
        cmsDeleteTransform( _revTransf );
        _revTransf = 0;
    }
    if ( profHandle ) {
        cmsCloseProfile( profHandle );
        profHandle = 0;
    }
}
#endif // ENABLE_LCMS

/**
 * Callback: set attributes from associated repr.
 */
void ColorProfile::build( SPObject *object, SPDocument *document, Inkscape::XML::Node *repr )
{
    ColorProfile *cprof = COLORPROFILE(object);
    g_assert(cprof->href == 0);
    g_assert(cprof->local == 0);
    g_assert(cprof->name == 0);
    g_assert(cprof->intentStr == 0);

    if (cprof_parent_class->build) {
        (* cprof_parent_class->build)(object, document, repr);
    }
    sp_object_read_attr( object, "xlink:href" );
    sp_object_read_attr( object, "local" );
    sp_object_read_attr( object, "name" );
    sp_object_read_attr( object, "rendering-intent" );

    // Register
    if ( document ) {
        sp_document_add_resource( document, "iccprofile", object );
    }
}

/**
 * Callback: set attribute.
 */
void ColorProfile::set( SPObject *object, unsigned key, gchar const *value )
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
                        // Normal for files that have not yet been saved.
                        docbase = "";
                    }
                    //g_message("docbase:%s\n", docbase);
                    org::w3c::dom::URI docUri(docbase);
                    //# 2. Get href of icc file.  we don't care if it's rel or abs
                    org::w3c::dom::URI hrefUri(cprof->href);
                    //# 3.  Resolve the href according the docBase.  This follows
                    //      the w3c specs.  All absolute and relative issues are considered
                    org::w3c::dom::URI cprofUri = docUri.resolve(hrefUri);
                    gchar* fullname = g_strdup((gchar *)cprofUri.getNativePath().c_str());
                    cprof->_clearProfile();
                    cprof->profHandle = cmsOpenProfileFromFile( fullname, "r" );
                    if ( cprof->profHandle ) {
                        cprof->_profileSpace = cmsGetColorSpace( cprof->profHandle );
                        cprof->_profileClass = cmsGetDeviceClass( cprof->profHandle );
                    }
#ifdef DEBUG_LCMS
                    DEBUG_MESSAGE( lcmsOne, "cmsOpenProfileFromFile( '%s'...) = %p", fullname, (void*)cprof->profHandle );
#endif // DEBUG_LCMS
                    g_free(fullname);
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
            if (cprof_parent_class->set) {
                (* cprof_parent_class->set)(object, key, value);
            }
            break;
    }

}

/**
 * Callback: write attributes to associated repr.
 */
Inkscape::XML::Node* ColorProfile::write( SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags )
{
    ColorProfile *cprof = COLORPROFILE(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
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

    if (cprof_parent_class->write) {
        (* cprof_parent_class->write)(object, xml_doc, repr, flags);
    }

    return repr;
}


#if ENABLE_LCMS

struct MapMap {
    icColorSpaceSignature space;
    DWORD inForm;
};

DWORD ColorProfile::_getInputFormat( icColorSpaceSignature space )
{
    MapMap possible[] = {
        {icSigXYZData,   TYPE_XYZ_16},
        {icSigLabData,   TYPE_Lab_16},
        //icSigLuvData
        {icSigYCbCrData, TYPE_YCbCr_16},
        {icSigYxyData,   TYPE_Yxy_16},
        {icSigRgbData,   TYPE_RGB_16},
        {icSigGrayData,  TYPE_GRAY_16},
        {icSigHsvData,   TYPE_HSV_16},
        {icSigHlsData,   TYPE_HLS_16},
        {icSigCmykData,  TYPE_CMYK_16},
        {icSigCmyData,   TYPE_CMY_16},
    };

    int index = 0;
    for ( guint i = 0; i < G_N_ELEMENTS(possible); i++ ) {
        if ( possible[i].space == space ) {
            index = i;
            break;
        }
    }

    return possible[index].inForm;
}

static int getLcmsIntent( guint svgIntent )
{
    int intent = INTENT_PERCEPTUAL;
    switch ( svgIntent ) {
        case Inkscape::RENDERING_INTENT_RELATIVE_COLORIMETRIC:
            intent = INTENT_RELATIVE_COLORIMETRIC;
            break;
        case Inkscape::RENDERING_INTENT_SATURATION:
            intent = INTENT_SATURATION;
            break;
        case Inkscape::RENDERING_INTENT_ABSOLUTE_COLORIMETRIC:
            intent = INTENT_ABSOLUTE_COLORIMETRIC;
            break;
        case Inkscape::RENDERING_INTENT_PERCEPTUAL:
        case Inkscape::RENDERING_INTENT_UNKNOWN:
        case Inkscape::RENDERING_INTENT_AUTO:
        default:
            intent = INTENT_PERCEPTUAL;
    }
    return intent;
}

static SPObject* bruteFind( SPDocument* document, gchar const* name )
{
    SPObject* result = 0;
    const GSList * current = sp_document_get_resource_list(document, "iccprofile");
    while ( current && !result ) {
        if ( IS_COLORPROFILE(current->data) ) {
            ColorProfile* prof = COLORPROFILE(current->data);
            if ( prof ) {
                if ( prof->name && (strcmp(prof->name, name) == 0) ) {
                    result = SP_OBJECT(current->data);
                    break;
                }
            }
        }
        current = g_slist_next(current);
    }

    return result;
}

cmsHPROFILE Inkscape::colorprofile_get_handle( SPDocument* document, guint* intent, gchar const* name )
{
    cmsHPROFILE prof = 0;

    SPObject* thing = bruteFind( document, name );
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

cmsHTRANSFORM ColorProfile::getTransfToSRGB8()
{
    if ( !_transf ) {
        int intent = getLcmsIntent(rendering_intent);
        _transf = cmsCreateTransform( profHandle, _getInputFormat(_profileSpace), getSRGBProfile(), TYPE_RGBA_8, intent, 0 );
    }
    return _transf;
}

cmsHTRANSFORM ColorProfile::getTransfFromSRGB8()
{
    if ( !_revTransf ) {
        int intent = getLcmsIntent(rendering_intent);
        _revTransf = cmsCreateTransform( getSRGBProfile(), TYPE_RGBA_8, profHandle, _getInputFormat(_profileSpace), intent, 0 );
    }
    return _revTransf;
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
    _name = cmsTakeProductDesc(prof);
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
#endif // ENABLE_LCMS

std::list<Glib::ustring> ColorProfile::getProfileDirs() {
    std::list<Glib::ustring> sources;

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

    return sources;
}

#if ENABLE_LCMS
static void findThings() {
    std::list<Glib::ustring> sources = ColorProfile::getProfileDirs();

    for ( std::list<Glib::ustring>::const_iterator it = sources.begin(); it != sources.end(); ++it ) {
        if ( g_file_test( it->c_str(), G_FILE_TEST_EXISTS ) && g_file_test( it->c_str(), G_FILE_TEST_IS_DIR ) ) {
            GError *err = 0;
            GDir *dir = g_dir_open(it->c_str(), 0, &err);

            if (dir) {
                for (gchar const *file = g_dir_read_name(dir); file != NULL; file = g_dir_read_name(dir)) {
                    gchar *filepath = g_build_filename(it->c_str(), file, NULL);


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
                                    if ( calcSize > 128 && calcSize <= static_cast<size_t>(st.st_size) ) {
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
    }
}

int errorHandlerCB(int ErrorCode, const char *ErrorText)
{
    g_message("lcms: Error %d; %s", ErrorCode, ErrorText);

    return 1;
}

static bool gamutWarn = false;
static Gdk::Color lastGamutColor("#808080");
static bool lastBPC = false;
#if defined(cmsFLAGS_PRESERVEBLACK)
static bool lastPreserveBlack = false;
#endif // defined(cmsFLAGS_PRESERVEBLACK)
static int lastIntent = INTENT_PERCEPTUAL;
static int lastProofIntent = INTENT_PERCEPTUAL;
static cmsHTRANSFORM transf = 0;

cmsHPROFILE Inkscape::colorprofile_get_system_profile_handle()
{
    static cmsHPROFILE theOne = 0;
    static Glib::ustring lastURI;

    static bool init = false;
    if ( !init ) {
        cmsSetErrorHandler(errorHandlerCB);

        findThings();
        init = true;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Glib::ustring uri = prefs->getString("/options/displayprofile/uri");

    if ( !uri.empty() ) {
        if ( uri != lastURI ) {
            lastURI.clear();
            if ( theOne ) {
                cmsCloseProfile( theOne );
            }
            if ( transf ) {
                cmsDeleteTransform( transf );
                transf = 0;
            }
            theOne = cmsOpenProfileFromFile( uri.data(), "r" );
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
    static Glib::ustring lastURI;

    static bool init = false;
    if ( !init ) {
        cmsSetErrorHandler(errorHandlerCB);

        findThings();
        init = true;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool which = prefs->getBool( "/options/softproof/enable");
    Glib::ustring uri = prefs->getString("/options/softproof/uri");

    if ( which && !uri.empty() ) {
        if ( lastURI != uri ) {
            lastURI.clear();
            if ( theOne ) {
                cmsCloseProfile( theOne );
            }
            if ( transf ) {
                cmsDeleteTransform( transf );
                transf = 0;
            }
            theOne = cmsOpenProfileFromFile( uri.data(), "r" );
            if ( theOne ) {
                // a display profile must have the proper stuff
                icColorSpaceSignature space = cmsGetColorSpace(theOne);
                icProfileClassSignature profClass = cmsGetDeviceClass(theOne);

                (void)space;
                (void)profClass;
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

static void free_transforms();

cmsHTRANSFORM Inkscape::colorprofile_get_display_transform()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool fromDisplay = prefs->getBool( "/options/displayprofile/from_display");
    if ( fromDisplay ) {
        if ( transf ) {
            cmsDeleteTransform(transf);
            transf = 0;
        }
        return 0;
    }

    bool warn = prefs->getBool( "/options/softproof/gamutwarn");
    int intent = prefs->getIntLimited( "/options/displayprofile/intent", 0, 0, 3 );
    int proofIntent = prefs->getIntLimited( "/options/softproof/intent", 0, 0, 3 );
    bool bpc = prefs->getBool( "/options/softproof/bpc");
#if defined(cmsFLAGS_PRESERVEBLACK)
    bool preserveBlack = prefs->getBool( "/options/softproof/preserveblack");
#endif //defined(cmsFLAGS_PRESERVEBLACK)
    Glib::ustring colorStr = prefs->getString("/options/softproof/gamutcolor");
    Gdk::Color gamutColor( colorStr.empty() ? "#808080" : colorStr );

    if ( (warn != gamutWarn)
         || (lastIntent != intent)
         || (lastProofIntent != proofIntent)
         || (bpc != lastBPC)
#if defined(cmsFLAGS_PRESERVEBLACK)
         || (preserveBlack != lastPreserveBlack)
#endif // defined(cmsFLAGS_PRESERVEBLACK)
         || (gamutColor != lastGamutColor)
        ) {
        gamutWarn = warn;
        free_transforms();
        lastIntent = intent;
        lastProofIntent = proofIntent;
        lastBPC = bpc;
#if defined(cmsFLAGS_PRESERVEBLACK)
        lastPreserveBlack = preserveBlack;
#endif // defined(cmsFLAGS_PRESERVEBLACK)
        lastGamutColor = gamutColor;
    }

    // Fetch these now, as they might clear the transform as a side effect.
    cmsHPROFILE hprof = Inkscape::colorprofile_get_system_profile_handle();
    cmsHPROFILE proofProf = hprof ? Inkscape::colorprofile_get_proof_profile_handle() : 0;

    if ( !transf ) {
        if ( hprof && proofProf ) {
            DWORD dwFlags = cmsFLAGS_SOFTPROOFING;
            if ( gamutWarn ) {
                dwFlags |= cmsFLAGS_GAMUTCHECK;
                cmsSetAlarmCodes(gamutColor.get_red() >> 8, gamutColor.get_green() >> 8, gamutColor.get_blue() >> 8);
            }
            if ( bpc ) {
                dwFlags |= cmsFLAGS_BLACKPOINTCOMPENSATION;
            }
#if defined(cmsFLAGS_PRESERVEBLACK)
            if ( preserveBlack ) {
                dwFlags |= cmsFLAGS_PRESERVEBLACK;
            }
#endif // defined(cmsFLAGS_PRESERVEBLACK)
            transf = cmsCreateProofingTransform( ColorProfile::getSRGBProfile(), TYPE_RGBA_8, hprof, TYPE_RGBA_8, proofProf, intent, proofIntent, dwFlags );
        } else if ( hprof ) {
            transf = cmsCreateTransform( ColorProfile::getSRGBProfile(), TYPE_RGBA_8, hprof, TYPE_RGBA_8, intent, 0 );
        }
    }

    return transf;
}


class MemProfile {
public:
    MemProfile();
    ~MemProfile();

    std::string id;
    cmsHPROFILE hprof;
    cmsHTRANSFORM transf;
};

MemProfile::MemProfile() :
    id(),
    hprof(0),
    transf(0)
{
}

MemProfile::~MemProfile()
{
}

static std::vector< std::vector<MemProfile> > perMonitorProfiles;

void free_transforms()
{
    if ( transf ) {
        cmsDeleteTransform(transf);
        transf = 0;
    }

    for ( std::vector< std::vector<MemProfile> >::iterator it = perMonitorProfiles.begin(); it != perMonitorProfiles.end(); ++it ) {
        for ( std::vector<MemProfile>::iterator it2 = it->begin(); it2 != it->end(); ++it2 ) {
            if ( it2->transf ) {
                cmsDeleteTransform(it2->transf);
                it2->transf = 0;
            }
        }
    }
}

Glib::ustring Inkscape::colorprofile_get_display_id( int screen, int monitor )
{
    Glib::ustring id;

    if ( screen >= 0 && screen < static_cast<int>(perMonitorProfiles.size()) ) {
        std::vector<MemProfile>& row = perMonitorProfiles[screen];
        if ( monitor >= 0 && monitor < static_cast<int>(row.size()) ) {
            MemProfile& item = row[monitor];
            id = item.id;
        }
    }

    return id;
}

Glib::ustring Inkscape::colorprofile_set_display_per( gpointer buf, guint bufLen, int screen, int monitor )
{
    Glib::ustring id;

    while ( static_cast<int>(perMonitorProfiles.size()) <= screen ) {
        std::vector<MemProfile> tmp;
        perMonitorProfiles.push_back(tmp);
    }
    std::vector<MemProfile>& row = perMonitorProfiles[screen];
    while ( static_cast<int>(row.size()) <= monitor ) {
        MemProfile tmp;
        row.push_back(tmp);
    }
    MemProfile& item = row[monitor];

    if ( item.hprof ) {
        cmsCloseProfile( item.hprof );
        item.hprof = 0;
    }
    id.clear();

    if ( buf && bufLen ) {
        id = Digest::hashHex(Digest::HASH_MD5,
                   reinterpret_cast<unsigned char*>(buf), bufLen);

        // Note: if this is not a valid profile, item.hprof will be set to null.
        item.hprof = cmsOpenProfileFromMem(buf, bufLen);
    }
    item.id = id;

    return id;
}

cmsHTRANSFORM Inkscape::colorprofile_get_display_per( Glib::ustring const& id )
{
    cmsHTRANSFORM result = 0;
    if ( id.empty() ) {
        return 0;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool found = false;
    for ( std::vector< std::vector<MemProfile> >::iterator it = perMonitorProfiles.begin(); it != perMonitorProfiles.end() && !found; ++it ) {
        for ( std::vector<MemProfile>::iterator it2 = it->begin(); it2 != it->end() && !found; ++it2 ) {
            if ( id == it2->id ) {
                MemProfile& item = *it2;

                bool warn = prefs->getBool( "/options/softproof/gamutwarn");
                int intent = prefs->getIntLimited( "/options/displayprofile/intent", 0, 0, 3 );
                int proofIntent = prefs->getIntLimited( "/options/softproof/intent", 0, 0, 3 );
                bool bpc = prefs->getBool( "/options/softproof/bpc");
#if defined(cmsFLAGS_PRESERVEBLACK)
                bool preserveBlack = prefs->getBool( "/options/softproof/preserveblack");
#endif //defined(cmsFLAGS_PRESERVEBLACK)
                Glib::ustring colorStr = prefs->getString("/options/softproof/gamutcolor");
                Gdk::Color gamutColor( colorStr.empty() ? "#808080" : colorStr );

                if ( (warn != gamutWarn)
                     || (lastIntent != intent)
                     || (lastProofIntent != proofIntent)
                     || (bpc != lastBPC)
#if defined(cmsFLAGS_PRESERVEBLACK)
                     || (preserveBlack != lastPreserveBlack)
#endif // defined(cmsFLAGS_PRESERVEBLACK)
                     || (gamutColor != lastGamutColor)
                    ) {
                    gamutWarn = warn;
                    free_transforms();
                    lastIntent = intent;
                    lastProofIntent = proofIntent;
                    lastBPC = bpc;
#if defined(cmsFLAGS_PRESERVEBLACK)
                    lastPreserveBlack = preserveBlack;
#endif // defined(cmsFLAGS_PRESERVEBLACK)
                    lastGamutColor = gamutColor;
                }

                // Fetch these now, as they might clear the transform as a side effect.
                cmsHPROFILE proofProf = item.hprof ? Inkscape::colorprofile_get_proof_profile_handle() : 0;

                if ( !item.transf ) {
                    if ( item.hprof && proofProf ) {
                        DWORD dwFlags = cmsFLAGS_SOFTPROOFING;
                        if ( gamutWarn ) {
                            dwFlags |= cmsFLAGS_GAMUTCHECK;
                            cmsSetAlarmCodes(gamutColor.get_red() >> 8, gamutColor.get_green() >> 8, gamutColor.get_blue() >> 8);
                        }
                        if ( bpc ) {
                            dwFlags |= cmsFLAGS_BLACKPOINTCOMPENSATION;
                        }
#if defined(cmsFLAGS_PRESERVEBLACK)
                        if ( preserveBlack ) {
                            dwFlags |= cmsFLAGS_PRESERVEBLACK;
                        }
#endif // defined(cmsFLAGS_PRESERVEBLACK)
                        item.transf = cmsCreateProofingTransform( ColorProfile::getSRGBProfile(), TYPE_RGBA_8, item.hprof, TYPE_RGBA_8, proofProf, intent, proofIntent, dwFlags );
                    } else if ( item.hprof ) {
                        item.transf = cmsCreateTransform( ColorProfile::getSRGBProfile(), TYPE_RGBA_8, item.hprof, TYPE_RGBA_8, intent, 0 );
                    }
                }

                result = item.transf;
                found = true;
            }
        }
    }

    return result;
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
