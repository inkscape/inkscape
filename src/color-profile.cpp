#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define noDEBUG_LCMS

#include <glib/gstdio.h>
#include <sys/fcntl.h>
#include <gdkmm/color.h>
#include <glib/gi18n.h>

#ifdef DEBUG_LCMS
#include <gtk.h>
#endif // DEBUG_LCMS

#include <cstring>
#include <string>
#include <io/sys.h>

#ifdef WIN32
#ifndef _WIN32_WINDOWS         // Allow use of features specific to Windows 98 or later. Required for correctly including icm.h
#define _WIN32_WINDOWS 0x0410
#endif
#include <windows.h>
#endif

#if ENABLE_LCMS
#include <lcms.h>
#endif // ENABLE_LCMS

#include "xml/repr.h"
#include "color.h"
#include "color-profile.h"
#include "color-profile-fns.h"
#include "attributes.h"
#include "inkscape.h"
#include "document.h"
#include "preferences.h"

#include "dom/uri.h"
#include "dom/util/digest.h"

#ifdef WIN32
#include <icm.h>
#endif // WIN32

using Inkscape::ColorProfile;
using Inkscape::ColorProfileClass;
using Inkscape::ColorProfileImpl;

namespace
{
#if ENABLE_LCMS
cmsHPROFILE getSystemProfileHandle();
cmsHPROFILE getProofProfileHandle();
void loadProfiles();
Glib::ustring getNameFromProfile(cmsHPROFILE profile);
#endif // ENABLE_LCMS
}

#ifdef DEBUG_LCMS
extern guint update_in_progress;
#define DEBUG_MESSAGE_SCISLAC(key, ...) \
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


#define DEBUG_MESSAGE(key, ...)\
{\
    g_message( __VA_ARGS__ );\
}

#else
#define DEBUG_MESSAGE_SCISLAC(key, ...)
#define DEBUG_MESSAGE(key, ...)
#endif // DEBUG_LCMS

static SPObjectClass *cprof_parent_class;


class ColorProfileImpl {
public:
    static cmsHPROFILE _sRGBProf;
    static cmsHPROFILE _NullProf;

    ColorProfileImpl();

#if ENABLE_LCMS
    static DWORD _getInputFormat( icColorSpaceSignature space );

    static cmsHPROFILE getNULLProfile();
    static cmsHPROFILE getSRGBProfile();

    void _clearProfile();

    cmsHPROFILE _profHandle;
    icProfileClassSignature _profileClass;
    icColorSpaceSignature _profileSpace;
    cmsHTRANSFORM _transf;
    cmsHTRANSFORM _revTransf;
    cmsHTRANSFORM _gamutTransf;
#endif // ENABLE_LCMS
};

ColorProfileImpl::ColorProfileImpl() :
#if ENABLE_LCMS
    _profHandle(0),
    _profileClass(icSigInputClass),
    _profileSpace(icSigRgbData),
    _transf(0),
    _revTransf(0),
    _gamutTransf(0)
#endif // ENABLE_LCMS
{
}

#if ENABLE_LCMS

cmsHPROFILE ColorProfileImpl::_sRGBProf = 0;

cmsHPROFILE ColorProfileImpl::getSRGBProfile() {
    if ( !_sRGBProf ) {
        _sRGBProf = cmsCreate_sRGBProfile();
    }
    return ColorProfileImpl::_sRGBProf;
}

cmsHPROFILE ColorProfileImpl::_NullProf = 0;

cmsHPROFILE ColorProfileImpl::getNULLProfile() {
    if ( !_NullProf ) {
        _NullProf = cmsCreateNULLProfile();
    }
    return _NullProf;
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
    cprof->impl = new ColorProfileImpl();

    cprof->href = 0;
    cprof->local = 0;
    cprof->name = 0;
    cprof->intentStr = 0;
    cprof->rendering_intent = Inkscape::RENDERING_INTENT_UNKNOWN;
}

/**
 * Callback: free object
 */
void ColorProfile::release( SPObject *object )
{
    // Unregister ourselves
    if ( object->document ) {
        object->document->removeResource("iccprofile", object);
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
    cprof->impl->_clearProfile();
#endif // ENABLE_LCMS

    delete cprof->impl;
    cprof->impl = 0;
}

#if ENABLE_LCMS
void ColorProfileImpl::_clearProfile()
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
    if ( _gamutTransf ) {
        cmsDeleteTransform( _gamutTransf );
        _gamutTransf = 0;
    }
    if ( _profHandle ) {
        cmsCloseProfile( _profHandle );
        _profHandle = 0;
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
    object->readAttr( "xlink:href" );
    object->readAttr( "local" );
    object->readAttr( "name" );
    object->readAttr( "rendering-intent" );

    // Register
    if ( document ) {
        document->addResource( "iccprofile", object );
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
                    SPDocument *doc = object->document;
                    if (!doc) {
                        doc = SP_ACTIVE_DOCUMENT;
                        g_warning("object has no document.  using active");
                    }
                    //# 1.  Get complete URI of document
                    gchar const *docbase = doc->getURI();
                    if (!docbase)
                    {
                        // Normal for files that have not yet been saved.
                        docbase = "";
                    }

                    gchar* escaped = g_uri_escape_string(cprof->href, "!*'();:@=+$,/?#[]", TRUE);

                    //g_message("docbase:%s\n", docbase);
                    org::w3c::dom::URI docUri(docbase);
                    //# 2. Get href of icc file.  we don't care if it's rel or abs
                    org::w3c::dom::URI hrefUri(escaped);
                    //# 3.  Resolve the href according the docBase.  This follows
                    //      the w3c specs.  All absolute and relative issues are considered
                    org::w3c::dom::URI cprofUri = docUri.resolve(hrefUri);
                    gchar* fullname = g_uri_unescape_string(cprofUri.getNativePath().c_str(), "");
                    cprof->impl->_clearProfile();
                    cprof->impl->_profHandle = cmsOpenProfileFromFile( fullname, "r" );
                    if ( cprof->impl->_profHandle ) {
                        cprof->impl->_profileSpace = cmsGetColorSpace( cprof->impl->_profHandle );
                        cprof->impl->_profileClass = cmsGetDeviceClass( cprof->impl->_profHandle );
                    }
                    DEBUG_MESSAGE( lcmsOne, "cmsOpenProfileFromFile( '%s'...) = %p", fullname, (void*)cprof->impl->_profHandle );
                    g_free(escaped);
                    escaped = 0;
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
            DEBUG_MESSAGE( lcmsTwo, "<color-profile> name set to '%s'", cprof->name );
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

DWORD ColorProfileImpl::_getInputFormat( icColorSpaceSignature space )
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
    const GSList * current = document->getResourceList("iccprofile");
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
        prof = COLORPROFILE(thing)->impl->_profHandle;
    }

    if ( intent ) {
        *intent = thing ? COLORPROFILE(thing)->rendering_intent : (guint)RENDERING_INTENT_UNKNOWN;
    }

    DEBUG_MESSAGE( lcmsThree, "<color-profile> queried for profile of '%s'. Returning %p with intent of %d", name, prof, (intent? *intent:0) );

    return prof;
}

icColorSpaceSignature ColorProfile::getColorSpace() const {
    return impl->_profileSpace;
}

icProfileClassSignature ColorProfile::getProfileClass() const {
    return impl->_profileClass;
}

cmsHTRANSFORM ColorProfile::getTransfToSRGB8()
{
    if ( !impl->_transf && impl->_profHandle ) {
        int intent = getLcmsIntent(rendering_intent);
        impl->_transf = cmsCreateTransform( impl->_profHandle, ColorProfileImpl::_getInputFormat(impl->_profileSpace), ColorProfileImpl::getSRGBProfile(), TYPE_BGRA_8, intent, 0 );
    }
    return impl->_transf;
}

cmsHTRANSFORM ColorProfile::getTransfFromSRGB8()
{
    if ( !impl->_revTransf && impl->_profHandle ) {
        int intent = getLcmsIntent(rendering_intent);
        impl->_revTransf = cmsCreateTransform( ColorProfileImpl::getSRGBProfile(), TYPE_BGRA_8, impl->_profHandle, ColorProfileImpl::_getInputFormat(impl->_profileSpace), intent, 0 );
    }
    return impl->_revTransf;
}

cmsHTRANSFORM ColorProfile::getTransfGamutCheck()
{
    if ( !impl->_gamutTransf ) {
        impl->_gamutTransf = cmsCreateProofingTransform(ColorProfileImpl::getSRGBProfile(), TYPE_BGRA_8, ColorProfileImpl::getNULLProfile(), TYPE_GRAY_8, impl->_profHandle, INTENT_RELATIVE_COLORIMETRIC, INTENT_RELATIVE_COLORIMETRIC, (cmsFLAGS_GAMUTCHECK|cmsFLAGS_SOFTPROOFING));
    }
    return impl->_gamutTransf;
}

bool ColorProfile::GamutCheck(SPColor color){
    BYTE outofgamut = 0;

    guint32 val = color.toRGBA32(0);
    guchar check_color[4] = {
        SP_RGBA32_R_U(val),
        SP_RGBA32_G_U(val),
        SP_RGBA32_B_U(val),
        255};

    int alarm_r, alarm_g, alarm_b;
    cmsGetAlarmCodes(&alarm_r, &alarm_g, &alarm_b);
    cmsSetAlarmCodes(255, 255, 255);
    cmsDoTransform(ColorProfile::getTransfGamutCheck(), &check_color, &outofgamut, 1);
    cmsSetAlarmCodes(alarm_r, alarm_g, alarm_b);
    return (outofgamut == 255);
}

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
    loadProfiles();
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
    loadProfiles();
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
    loadProfiles();
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

std::vector<Glib::ustring> ColorProfile::getBaseProfileDirs() {
#if ENABLE_LCMS
    static bool warnSet = false;
    if (!warnSet) {
        cmsErrorAction( LCMS_ERROR_SHOW );
        warnSet = true;
    }
#endif // ENABLE_LCMS
    std::vector<Glib::ustring> sources;

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


    const gchar* const * dataDirs = g_get_system_data_dirs();
    for ( int i = 0; dataDirs[i]; i++ ) {
        gchar* path = g_build_filename(dataDirs[i], "color", "icc", NULL);
        sources.push_back(path);
        g_free(path);
    }

    // On OS X:
    {
        bool onOSX = false;
        std::vector<Glib::ustring> possible;
        possible.push_back("/System/Library/ColorSync/Profiles");
        possible.push_back("/Library/ColorSync/Profiles");
        for ( std::vector<Glib::ustring>::const_iterator it = possible.begin(); it != possible.end(); ++it ) {
            if ( g_file_test(it->c_str(), G_FILE_TEST_EXISTS)  && g_file_test(it->c_str(), G_FILE_TEST_IS_DIR) ) {
                sources.push_back(it->c_str());
                onOSX = true;
            }
        }
        if ( onOSX ) {
            gchar* path = g_build_filename(g_get_home_dir(), "Library", "ColorSync", "Profiles", NULL);
            if ( g_file_test(path, G_FILE_TEST_EXISTS)  && g_file_test(path, G_FILE_TEST_IS_DIR) ) {
                sources.push_back(path);
            }
            g_free(path);
        }
    }

#ifdef WIN32
    wchar_t pathBuf[MAX_PATH + 1];
    pathBuf[0] = 0;
    DWORD pathSize = sizeof(pathBuf);
    g_assert(sizeof(wchar_t) == sizeof(gunichar2));
    if ( GetColorDirectoryW( NULL, pathBuf, &pathSize ) ) {
        gchar * utf8Path = g_utf16_to_utf8( (gunichar2*)(&pathBuf[0]), -1, NULL, NULL, NULL );
        if ( !g_utf8_validate(utf8Path, -1, NULL) ) {
            g_warning( "GetColorDirectoryW() resulted in invalid UTF-8" );
        } else {
            sources.push_back(utf8Path);
        }
        g_free( utf8Path );
    }
#endif // WIN32

    return sources;
}

static bool isIccFile( gchar const *filepath )
{
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
#if ENABLE_LCMS
            if (isIccFile) {
                cmsHPROFILE prof = cmsOpenProfileFromFile( filepath, "r" );
                if ( prof ) {
                    icProfileClassSignature profClass = cmsGetDeviceClass(prof);
                    if ( profClass == icSigNamedColorClass ) {
                        isIccFile = false; // Ignore named color profiles for now.
                    }
                    cmsCloseProfile( prof );
                }
            }
#endif // ENABLE_LCMS
        }
    }
    return isIccFile;
}

std::vector<Glib::ustring> ColorProfile::getProfileFiles()
{
    std::vector<Glib::ustring> files;

    std::list<Glib::ustring> sources;
    {
        std::vector<Glib::ustring> tmp = ColorProfile::getBaseProfileDirs();
        sources.insert(sources.begin(), tmp.begin(), tmp.end());
    }
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
                        if ( isIccFile( filepath ) ) {
                            files.push_back( filepath );
                        }
                    }

                    g_free(filepath);
                }
                g_dir_close(dir);
                dir = 0;
            } else {
                gchar *safeDir = Inkscape::IO::sanitizeString(it->c_str());
                g_warning(_("Color profiles directory (%s) is unavailable."), safeDir);
                g_free(safeDir);
            }
        }
    }

    return files;
}

#if ENABLE_LCMS
#endif // ENABLE_LCMS

std::vector<std::pair<Glib::ustring, Glib::ustring> > ColorProfile::getProfileFilesWithNames()
{
    std::vector<std::pair<Glib::ustring, Glib::ustring> > result;

#if ENABLE_LCMS
    std::vector<Glib::ustring> files = getProfileFiles();
    for ( std::vector<Glib::ustring>::const_iterator it = files.begin(); it != files.end(); ++it ) {
        cmsHPROFILE hProfile = cmsOpenProfileFromFile(it->c_str(), "r");
        if ( hProfile ) {
            Glib::ustring name = getNameFromProfile(hProfile);
            result.push_back( std::make_pair(*it, name) );
            cmsCloseProfile(hProfile);
        }
    }
#endif // ENABLE_LCMS

    return result;
}

#if ENABLE_LCMS
int errorHandlerCB(int ErrorCode, const char *ErrorText)
{
    g_message("lcms: Error %d; %s", ErrorCode, ErrorText);

    return 1;
}

namespace
{
Glib::ustring getNameFromProfile(cmsHPROFILE profile)
{
    gchar const *name = 0;
    if ( profile ) {
        name = cmsTakeProductDesc(profile);
        if ( !name ) {
            name = cmsTakeProductName(profile);
        }
        if ( name && !g_utf8_validate(name, -1, NULL) ) {
            name = _("(invalid UTF-8 string)");
        }
    }
    return (name) ? name : _("None");
}

/**
 * This function loads or refreshes data in knownProfiles.
 * Call it at the start of every call that requires this data.
 */
void loadProfiles()
{
    static bool error_handler_set = false;
    if (!error_handler_set) {
        cmsSetErrorHandler(errorHandlerCB);
        error_handler_set = true;
    }

    static bool profiles_searched = false;
    if ( !profiles_searched ) {
        knownProfiles.clear();
        std::vector<Glib::ustring> files = ColorProfile::getProfileFiles();

        for ( std::vector<Glib::ustring>::const_iterator it = files.begin(); it != files.end(); ++it ) {
            cmsHPROFILE prof = cmsOpenProfileFromFile( it->c_str(), "r" );
            if ( prof ) {
                ProfileInfo info( prof, Glib::filename_to_utf8( it->c_str() ) );
                cmsCloseProfile( prof );
                prof = 0;

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
        profiles_searched = true;
    }
}
} // namespace

static bool gamutWarn = false;
static Gdk::Color lastGamutColor("#808080");
static bool lastBPC = false;
#if defined(cmsFLAGS_PRESERVEBLACK)
static bool lastPreserveBlack = false;
#endif // defined(cmsFLAGS_PRESERVEBLACK)
static int lastIntent = INTENT_PERCEPTUAL;
static int lastProofIntent = INTENT_PERCEPTUAL;
static cmsHTRANSFORM transf = 0;

namespace {
cmsHPROFILE getSystemProfileHandle()
{
    static cmsHPROFILE theOne = 0;
    static Glib::ustring lastURI;

    loadProfiles();

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


cmsHPROFILE getProofProfileHandle()
{
    static cmsHPROFILE theOne = 0;
    static Glib::ustring lastURI;

    loadProfiles();

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
} // namespace

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
    cmsHPROFILE hprof = getSystemProfileHandle();
    cmsHPROFILE proofProf = hprof ? getProofProfileHandle() : 0;

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
            transf = cmsCreateProofingTransform( ColorProfileImpl::getSRGBProfile(), TYPE_BGRA_8, hprof, TYPE_BGRA_8, proofProf, intent, proofIntent, dwFlags );
        } else if ( hprof ) {
            transf = cmsCreateTransform( ColorProfileImpl::getSRGBProfile(), TYPE_BGRA_8, hprof, TYPE_BGRA_8, intent, 0 );
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
                cmsHPROFILE proofProf = item.hprof ? getProofProfileHandle() : 0;

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
                        item.transf = cmsCreateProofingTransform( ColorProfileImpl::getSRGBProfile(), TYPE_BGRA_8, item.hprof, TYPE_BGRA_8, proofProf, intent, proofIntent, dwFlags );
                    } else if ( item.hprof ) {
                        item.transf = cmsCreateTransform( ColorProfileImpl::getSRGBProfile(), TYPE_BGRA_8, item.hprof, TYPE_BGRA_8, intent, 0 );
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
