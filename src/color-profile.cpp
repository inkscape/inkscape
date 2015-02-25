#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define noDEBUG_LCMS

#if WITH_GTKMM_3_0
# include <gdkmm/rgba.h>
#else
# include <gdkmm/color.h>
#endif

#include <glibmm/checksum.h>
#include <glib/gstdio.h>
#include <fcntl.h>
#include <glib/gi18n.h>

#ifdef DEBUG_LCMS
#include <gtk/gtk.h>
#endif // DEBUG_LCMS

#include <unistd.h>
#include <cstring>
#include <string>
#include <io/sys.h>

#ifdef WIN32
#ifndef _WIN32_WINDOWS         // Allow use of features specific to Windows 98 or later. Required for correctly including icm.h
#define _WIN32_WINDOWS 0x0410
#endif
#include <windows.h>
#endif

#if HAVE_LIBLCMS2
#  include <lcms2.h>
#elif HAVE_LIBLCMS1
#  include <lcms.h>
#endif // HAVE_LIBLCMS2

#include "xml/repr.h"
#include "color.h"
#include "color-profile.h"
#include "cms-system.h"
#include "color-profile-cms-fns.h"
#include "attributes.h"
#include "inkscape.h"
#include "document.h"
#include "preferences.h"

#include "uri.h"

#ifdef WIN32
#include <icm.h>
#endif // WIN32

#include <glibmm/convert.h>

using Inkscape::ColorProfile;
using Inkscape::ColorProfileImpl;

namespace
{
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
cmsHPROFILE getSystemProfileHandle();
cmsHPROFILE getProofProfileHandle();
void loadProfiles();
Glib::ustring getNameFromProfile(cmsHPROFILE profile);
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
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

namespace Inkscape {

class ColorProfileImpl {
public:
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    static cmsHPROFILE _sRGBProf;
    static cmsHPROFILE _NullProf;
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

    ColorProfileImpl();

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    static cmsUInt32Number _getInputFormat( cmsColorSpaceSignature space );

    static cmsHPROFILE getNULLProfile();
    static cmsHPROFILE getSRGBProfile();

    void _clearProfile();

    cmsHPROFILE _profHandle;
    cmsProfileClassSignature _profileClass;
    cmsColorSpaceSignature _profileSpace;
    cmsHTRANSFORM _transf;
    cmsHTRANSFORM _revTransf;
    cmsHTRANSFORM _gamutTransf;
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
};

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
cmsColorSpaceSignature asICColorSpaceSig(ColorSpaceSig const & sig)
{
    return ColorSpaceSigWrapper(sig);
}

cmsProfileClassSignature asICColorProfileClassSig(ColorProfileClassSig const & sig)
{
    return ColorProfileClassSigWrapper(sig);
}
#endif //  defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

} // namespace Inkscape

ColorProfileImpl::ColorProfileImpl()
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
	:
    _profHandle(0),
    _profileClass(cmsSigInputClass),
    _profileSpace(cmsSigRgbData),
    _transf(0),
    _revTransf(0),
    _gamutTransf(0)
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
{
}

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

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

#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

ColorProfile::ColorProfile() : SPObject() {
    this->impl = new ColorProfileImpl();

    this->href = 0;
    this->local = 0;
    this->name = 0;
    this->intentStr = 0;
    this->rendering_intent = Inkscape::RENDERING_INTENT_UNKNOWN;
}

ColorProfile::~ColorProfile() {
}

/**
 * Callback: free object
 */
void ColorProfile::release() {
    // Unregister ourselves
    if ( this->document ) {
        this->document->removeResource("iccprofile", this);
    }

    if ( this->href ) {
        g_free( this->href );
        this->href = 0;
    }

    if ( this->local ) {
        g_free( this->local );
        this->local = 0;
    }

    if ( this->name ) {
        g_free( this->name );
        this->name = 0;
    }

    if ( this->intentStr ) {
        g_free( this->intentStr );
        this->intentStr = 0;
    }

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    this->impl->_clearProfile();
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

    delete this->impl;
    this->impl = 0;
}

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
void ColorProfileImpl::_clearProfile()
{
    _profileSpace = cmsSigRgbData;

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
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

/**
 * Callback: set attributes from associated repr.
 */
void ColorProfile::build(SPDocument *document, Inkscape::XML::Node *repr) {
    g_assert(this->href == 0);
    g_assert(this->local == 0);
    g_assert(this->name == 0);
    g_assert(this->intentStr == 0);

    SPObject::build(document, repr);

    this->readAttr( "xlink:href" );
    this->readAttr( "local" );
    this->readAttr( "name" );
    this->readAttr( "rendering-intent" );

    // Register
    if ( document ) {
        document->addResource( "iccprofile", this );
    }
}


/**
 * Callback: set attribute.
 */
void ColorProfile::set(unsigned key, gchar const *value) {
    switch (key) {
        case SP_ATTR_XLINK_HREF:
            if ( this->href ) {
                g_free( this->href );
                this->href = 0;
            }
            if ( value ) {
                this->href = g_strdup( value );
                if ( *this->href ) {
#if HAVE_LIBLCMS1
                    cmsErrorAction( LCMS_ERROR_SHOW );
#endif
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

                    // TODO open filename and URIs properly
                    //FILE* fp = fopen_utf8name( filename, "r" );
                    //LCMSAPI cmsHPROFILE   LCMSEXPORT cmsOpenProfileFromMem(LPVOID MemPtr, cmsUInt32Number dwSize);

                    // Try to open relative
                    SPDocument *doc = this->document;
                    if (!doc) {
                        doc = SP_ACTIVE_DOCUMENT;
                        g_warning("this has no document.  using active");
                    }
                    //# 1.  Get complete URI of document
                    gchar const *docbase = doc->getURI();

                    gchar* escaped = g_uri_escape_string(this->href, "!*'();:@=+$,/?#[]", TRUE);

                    //g_message("docbase:%s\n", docbase);
                    //org::w3c::dom::URI docUri(docbase);
                    Inkscape::URI docUri("");
                    if (docbase) { // The file has already been saved
                        docUri = Inkscape::URI::from_native_filename(docbase);
                    }

                    //# 2. Get href of icc file.  we don't care if it's rel or abs
                    //org::w3c::dom::URI hrefUri(escaped);
                    Inkscape::URI hrefUri(escaped);
                    //# 3.  Resolve the href according the docBase.  This follows
                    //      the w3c specs.  All absolute and relative issues are considered
                    std::string fullpath = hrefUri.getFullPath(docUri.getFullPath(""));

                    gchar* fullname = g_uri_unescape_string(fullpath.c_str(), "");
                    this->impl->_clearProfile();
                    this->impl->_profHandle = cmsOpenProfileFromFile( fullname, "r" );
                    if ( this->impl->_profHandle ) {
                        this->impl->_profileSpace = cmsGetColorSpace( this->impl->_profHandle );
                        this->impl->_profileClass = cmsGetDeviceClass( this->impl->_profHandle );
                    }
                    DEBUG_MESSAGE( lcmsOne, "cmsOpenProfileFromFile( '%s'...) = %p", fullname, (void*)this->impl->_profHandle );
                    g_free(escaped);
                    escaped = 0;
                    g_free(fullname);
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
                }
            }
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_LOCAL:
            if ( this->local ) {
                g_free( this->local );
                this->local = 0;
            }
            this->local = g_strdup( value );
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_NAME:
            if ( this->name ) {
                g_free( this->name );
                this->name = 0;
            }
            this->name = g_strdup( value );
            DEBUG_MESSAGE( lcmsTwo, "<color-profile> name set to '%s'", this->name );
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_RENDERING_INTENT:
            if ( this->intentStr ) {
                g_free( this->intentStr );
                this->intentStr = 0;
            }
            this->intentStr = g_strdup( value );

            if ( value ) {
                if ( strcmp( value, "auto" ) == 0 ) {
                    this->rendering_intent = RENDERING_INTENT_AUTO;
                } else if ( strcmp( value, "perceptual" ) == 0 ) {
                    this->rendering_intent = RENDERING_INTENT_PERCEPTUAL;
                } else if ( strcmp( value, "relative-colorimetric" ) == 0 ) {
                    this->rendering_intent = RENDERING_INTENT_RELATIVE_COLORIMETRIC;
                } else if ( strcmp( value, "saturation" ) == 0 ) {
                    this->rendering_intent = RENDERING_INTENT_SATURATION;
                } else if ( strcmp( value, "absolute-colorimetric" ) == 0 ) {
                    this->rendering_intent = RENDERING_INTENT_ABSOLUTE_COLORIMETRIC;
                } else {
                    this->rendering_intent = RENDERING_INTENT_UNKNOWN;
                }
            } else {
                this->rendering_intent = RENDERING_INTENT_UNKNOWN;
            }

            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        default:
        	SPObject::set(key, value);
            break;
    }
}

/**
 * Callback: write attributes to associated repr.
 */
Inkscape::XML::Node* ColorProfile::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:color-profile");
    }

    if ( (flags & SP_OBJECT_WRITE_ALL) || this->href ) {
        repr->setAttribute( "xlink:href", this->href );
    }

    if ( (flags & SP_OBJECT_WRITE_ALL) || this->local ) {
        repr->setAttribute( "local", this->local );
    }

    if ( (flags & SP_OBJECT_WRITE_ALL) || this->name ) {
        repr->setAttribute( "name", this->name );
    }

    if ( (flags & SP_OBJECT_WRITE_ALL) || this->intentStr ) {
        repr->setAttribute( "rendering-intent", this->intentStr );
    }

    SPObject::write(xml_doc, repr, flags);

    return repr;
}


#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

struct MapMap {
    cmsColorSpaceSignature space;
    cmsUInt32Number inForm;
};

cmsUInt32Number ColorProfileImpl::_getInputFormat( cmsColorSpaceSignature space )
{
    MapMap possible[] = {
        {cmsSigXYZData,   TYPE_XYZ_16},
        {cmsSigLabData,   TYPE_Lab_16},
        //cmsSigLuvData
        {cmsSigYCbCrData, TYPE_YCbCr_16},
        {cmsSigYxyData,   TYPE_Yxy_16},
        {cmsSigRgbData,   TYPE_RGB_16},
        {cmsSigGrayData,  TYPE_GRAY_16},
        {cmsSigHsvData,   TYPE_HSV_16},
        {cmsSigHlsData,   TYPE_HLS_16},
        {cmsSigCmykData,  TYPE_CMYK_16},
        {cmsSigCmyData,   TYPE_CMY_16},
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

cmsHPROFILE Inkscape::CMSSystem::getHandle( SPDocument* document, guint* intent, gchar const* name )
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

Inkscape::ColorSpaceSig ColorProfile::getColorSpace() const {
    return ColorSpaceSigWrapper(impl->_profileSpace);
}

Inkscape::ColorProfileClassSig ColorProfile::getProfileClass() const {
    return ColorProfileClassSigWrapper(impl->_profileClass);
}

cmsHTRANSFORM ColorProfile::getTransfToSRGB8()
{
    if ( !impl->_transf && impl->_profHandle ) {
        int intent = getLcmsIntent(rendering_intent);
        impl->_transf = cmsCreateTransform( impl->_profHandle, ColorProfileImpl::_getInputFormat(impl->_profileSpace), ColorProfileImpl::getSRGBProfile(), TYPE_RGBA_8, intent, 0 );
    }
    return impl->_transf;
}

cmsHTRANSFORM ColorProfile::getTransfFromSRGB8()
{
    if ( !impl->_revTransf && impl->_profHandle ) {
        int intent = getLcmsIntent(rendering_intent);
        impl->_revTransf = cmsCreateTransform( ColorProfileImpl::getSRGBProfile(), TYPE_RGBA_8, impl->_profHandle, ColorProfileImpl::_getInputFormat(impl->_profileSpace), intent, 0 );
    }
    return impl->_revTransf;
}

cmsHTRANSFORM ColorProfile::getTransfGamutCheck()
{
    if ( !impl->_gamutTransf ) {
        impl->_gamutTransf = cmsCreateProofingTransform(ColorProfileImpl::getSRGBProfile(),
                                                        TYPE_BGRA_8,
                                                        ColorProfileImpl::getNULLProfile(),
                                                        TYPE_GRAY_8,
                                                        impl->_profHandle,
                                                        INTENT_RELATIVE_COLORIMETRIC,
                                                        INTENT_RELATIVE_COLORIMETRIC,
                                                        (cmsFLAGS_GAMUTCHECK | cmsFLAGS_SOFTPROOFING));
    }
    return impl->_gamutTransf;
}

bool ColorProfile::GamutCheck(SPColor color)
{
    guint32 val = color.toRGBA32(0);

#if HAVE_LIBLCMS1
    int alarm_r = 0;
    int alarm_g = 0;
    int alarm_b = 0;
    cmsGetAlarmCodes(&alarm_r, &alarm_g, &alarm_b);
    cmsSetAlarmCodes(255, 255, 255);
#elif HAVE_LIBLCMS2
    cmsUInt16Number oldAlarmCodes[cmsMAXCHANNELS] = {0};
    cmsGetAlarmCodes(oldAlarmCodes);
    cmsUInt16Number newAlarmCodes[cmsMAXCHANNELS] = {0};
    newAlarmCodes[0] = ~0;
    cmsSetAlarmCodes(newAlarmCodes);
#endif // HAVE_LIBLCMS1

    cmsUInt8Number outofgamut = 0;
    guchar check_color[4] = {
        static_cast<guchar>(SP_RGBA32_R_U(val)),
        static_cast<guchar>(SP_RGBA32_G_U(val)),
        static_cast<guchar>(SP_RGBA32_B_U(val)),
        255};
    cmsDoTransform(ColorProfile::getTransfGamutCheck(), &check_color, &outofgamut, 1);

#if HAVE_LIBLCMS1
    cmsSetAlarmCodes(alarm_r, alarm_g, alarm_b);
#elif HAVE_LIBLCMS2
    cmsSetAlarmCodes(oldAlarmCodes);
#endif // HAVE_LIBLCMS1

    return (outofgamut != 0);
}

class ProfileInfo
{
public:
    ProfileInfo( cmsHPROFILE prof, Glib::ustring const & path );

    Glib::ustring const& getName() {return _name;}
    Glib::ustring const& getPath() {return _path;}
    cmsColorSpaceSignature getSpace() {return _profileSpace;}
    cmsProfileClassSignature getClass() {return _profileClass;}

private:
    Glib::ustring _path;
    Glib::ustring _name;
    cmsColorSpaceSignature _profileSpace;
    cmsProfileClassSignature _profileClass;
};

#include <iostream>

ProfileInfo::ProfileInfo( cmsHPROFILE prof, Glib::ustring const & path ) :
    _path( path ),
    _name( getNameFromProfile(prof) ),
    _profileSpace( cmsGetColorSpace( prof ) ),
    _profileClass( cmsGetDeviceClass( prof ) )
{
}



static std::vector<ProfileInfo> knownProfiles;

std::vector<Glib::ustring> Inkscape::CMSSystem::getDisplayNames()
{
    loadProfiles();
    std::vector<Glib::ustring> result;

    for ( std::vector<ProfileInfo>::iterator it = knownProfiles.begin(); it != knownProfiles.end(); ++it ) {
        if ( it->getClass() == cmsSigDisplayClass && it->getSpace() == cmsSigRgbData ) {
            result.push_back( it->getName() );
        }
    }
    std::sort(result.begin(), result.end());

    return result;
}

std::vector<Glib::ustring> Inkscape::CMSSystem::getSoftproofNames()
{
    loadProfiles();
    std::vector<Glib::ustring> result;

    for ( std::vector<ProfileInfo>::iterator it = knownProfiles.begin(); it != knownProfiles.end(); ++it ) {
        if ( it->getClass() == cmsSigOutputClass ) {
            result.push_back( it->getName() );
        }
    }
    std::sort(result.begin(), result.end());

    return result;
}

Glib::ustring Inkscape::CMSSystem::getPathForProfile(Glib::ustring const& name)
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

void Inkscape::CMSSystem::doTransform(cmsHTRANSFORM transform, void *inBuf, void *outBuf, unsigned int size)
{
    cmsDoTransform(transform, inBuf, outBuf, size);
}

bool Inkscape::CMSSystem::isPrintColorSpace(ColorProfile const *profile)
{
    bool isPrint = false;
    if ( profile ) {
        ColorSpaceSigWrapper colorspace = profile->getColorSpace();
        isPrint = (colorspace == cmsSigCmykData) || (colorspace == cmsSigCmyData);
    }
    return isPrint;
}

gint Inkscape::CMSSystem::getChannelCount(ColorProfile const *profile)
{
    gint count = 0;
    if ( profile ) {
#if HAVE_LIBLCMS1
        count = _cmsChannelsOf( asICColorSpaceSig(profile->getColorSpace()) );
#elif HAVE_LIBLCMS2
        count = cmsChannelsOf( asICColorSpaceSig(profile->getColorSpace()) );
#endif
    }
    return count;
}

#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

std::vector<Glib::ustring> ColorProfile::getBaseProfileDirs() {
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    static bool warnSet = false;
    if (!warnSet) {
#if HAVE_LIBLCMS1
        cmsErrorAction( LCMS_ERROR_SHOW );
#endif
        warnSet = true;
    }
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    std::vector<Glib::ustring> sources;

    // first try user's local dir
    gchar* path = g_build_filename(g_get_user_data_dir(), "color", "icc", NULL);
    sources.push_back(path);
    g_free(path);

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
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
            if (isIccFile) {
                cmsHPROFILE prof = cmsOpenProfileFromFile( filepath, "r" );
                if ( prof ) {
                    cmsProfileClassSignature profClass = cmsGetDeviceClass(prof);
                    if ( profClass == cmsSigNamedColorClass ) {
                        isIccFile = false; // Ignore named color profiles for now.
                    }
                    cmsCloseProfile( prof );
                }
            }
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
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
                        sources.push_back( filepath );
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

std::vector<std::pair<Glib::ustring, Glib::ustring> > ColorProfile::getProfileFilesWithNames()
{
    std::vector<std::pair<Glib::ustring, Glib::ustring> > result;

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    std::vector<Glib::ustring> files = getProfileFiles();
    for ( std::vector<Glib::ustring>::const_iterator it = files.begin(); it != files.end(); ++it ) {
        cmsHPROFILE hProfile = cmsOpenProfileFromFile(it->c_str(), "r");
        if ( hProfile ) {
            Glib::ustring name = getNameFromProfile(hProfile);
            result.push_back( std::make_pair(*it, name) );
            cmsCloseProfile(hProfile);
        }
    }
    std::sort(result.begin(), result.end());
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

    return result;
}

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
#if HAVE_LIBLCMS1
int errorHandlerCB(int ErrorCode, const char *ErrorText)
{
    g_message("lcms: Error %d; %s", ErrorCode, ErrorText);

    return 1;
}
#elif HAVE_LIBLCMS2
void errorHandlerCB(cmsContext /*contextID*/, cmsUInt32Number errorCode, char const *errorText)
{
    g_message("lcms: Error %d", errorCode);
    g_message("                 %p", errorText);
    //g_message("lcms: Error %d; %s", errorCode, errorText);
}
#endif

namespace
{

Glib::ustring getNameFromProfile(cmsHPROFILE profile)
{
    Glib::ustring nameStr;
    if ( profile ) {
#if HAVE_LIBLCMS1
        gchar const *name = cmsTakeProductDesc(profile);
        if ( !name ) {
            name = cmsTakeProductName(profile);
        }
        if ( name && !g_utf8_validate(name, -1, NULL) ) {
            name = _("(invalid UTF-8 string)");
        }
        nameStr = (name) ? name : C_("Profile name", "None");
#elif HAVE_LIBLCMS2
    cmsUInt32Number byteLen = cmsGetProfileInfo(profile, cmsInfoDescription, "en", "US", NULL, 0);
    if (byteLen > 0) {
        // TODO investigate wchar_t and cmsGetProfileInfo()
        std::vector<char> data(byteLen);
        cmsUInt32Number readLen = cmsGetProfileInfoASCII(profile, cmsInfoDescription,
                                                         "en", "US",
                                                         data.data(), data.size());
        if (readLen < data.size()) {
            data.resize(readLen);
        }
        nameStr = Glib::ustring(data.begin(), data.end());
    }
    if (nameStr.empty() || !g_utf8_validate(nameStr.c_str(), -1, NULL)) {
        nameStr = _("(invalid UTF-8 string)");
    }
#endif
    }
    return nameStr;
}

/**
 * This function loads or refreshes data in knownProfiles.
 * Call it at the start of every call that requires this data.
 */
void loadProfiles()
{
    static bool error_handler_set = false;
    if (!error_handler_set) {
#if HAVE_LIBLCMS1
        cmsSetErrorHandler(errorHandlerCB);
#elif HAVE_LIBLCMS2
        //cmsSetLogErrorHandler(errorHandlerCB);
        //g_message("LCMS error handler set");
#endif
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

#if WITH_GTKMM_3_0
static Gdk::RGBA lastGamutColor("#808080");
#else
static Gdk::Color lastGamutColor("#808080");
#endif

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
                cmsColorSpaceSignature space = cmsGetColorSpace(theOne);
                cmsProfileClassSignature profClass = cmsGetDeviceClass(theOne);

                if ( profClass != cmsSigDisplayClass ) {
                    g_warning("Not a display profile");
                    cmsCloseProfile( theOne );
                    theOne = 0;
                } else if ( space != cmsSigRgbData ) {
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
                cmsColorSpaceSignature space = cmsGetColorSpace(theOne);
                cmsProfileClassSignature profClass = cmsGetDeviceClass(theOne);

                (void)space;
                (void)profClass;
/*
                if ( profClass != cmsSigDisplayClass ) {
                    g_warning("Not a display profile");
                    cmsCloseProfile( theOne );
                    theOne = 0;
                } else if ( space != cmsSigRgbData ) {
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

cmsHTRANSFORM Inkscape::CMSSystem::getDisplayTransform()
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

#if WITH_GTKMM_3_0
    Gdk::RGBA gamutColor( colorStr.empty() ? "#808080" : colorStr );
#else
    Gdk::Color gamutColor( colorStr.empty() ? "#808080" : colorStr );
#endif

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
            cmsUInt32Number dwFlags = cmsFLAGS_SOFTPROOFING;
            if ( gamutWarn ) {
                dwFlags |= cmsFLAGS_GAMUTCHECK;

#if WITH_GTKMM_3_0
                gushort gamutColor_r = gamutColor.get_red_u();
                gushort gamutColor_g = gamutColor.get_green_u();
                gushort gamutColor_b = gamutColor.get_blue_u();
#else
                gushort gamutColor_r = gamutColor.get_red();
                gushort gamutColor_g = gamutColor.get_green();
                gushort gamutColor_b = gamutColor.get_blue();
#endif

#if HAVE_LIBLCMS1
                cmsSetAlarmCodes(gamutColor_r >> 8, gamutColor_g >> 8, gamutColor_b >> 8);
#elif HAVE_LIBLCMS2
                cmsUInt16Number newAlarmCodes[cmsMAXCHANNELS] = {0};
                newAlarmCodes[0] = gamutColor_r;
                newAlarmCodes[1] = gamutColor_g;
                newAlarmCodes[2] = gamutColor_b;
                newAlarmCodes[3] = ~0;
                cmsSetAlarmCodes(newAlarmCodes);
#endif
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

Glib::ustring Inkscape::CMSSystem::getDisplayId( int screen, int monitor )
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

Glib::ustring Inkscape::CMSSystem::setDisplayPer( gpointer buf, guint bufLen, int screen, int monitor )
{
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

    Glib::ustring id;

    if ( buf && bufLen ) {
        gsize len = bufLen; // len is an inout parameter
        id = Glib::Checksum::compute_checksum(Glib::Checksum::CHECKSUM_MD5,
            reinterpret_cast<guchar*>(buf), len);

        // Note: if this is not a valid profile, item.hprof will be set to null.
        item.hprof = cmsOpenProfileFromMem(buf, bufLen);
    }
    item.id = id;

    return id;
}

cmsHTRANSFORM Inkscape::CMSSystem::getDisplayPer( Glib::ustring const& id )
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

#if WITH_GTKMM_3_0
                Gdk::RGBA gamutColor( colorStr.empty() ? "#808080" : colorStr );
#else
                Gdk::Color gamutColor( colorStr.empty() ? "#808080" : colorStr );
#endif

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
                        cmsUInt32Number dwFlags = cmsFLAGS_SOFTPROOFING;
                        if ( gamutWarn ) {
                            dwFlags |= cmsFLAGS_GAMUTCHECK;

#if WITH_GTKMM_3_0
                            gushort gamutColor_r = gamutColor.get_red_u();
                            gushort gamutColor_g = gamutColor.get_green_u();
                            gushort gamutColor_b = gamutColor.get_blue_u();
#else
                            gushort gamutColor_r = gamutColor.get_red();
                            gushort gamutColor_g = gamutColor.get_green();
                            gushort gamutColor_b = gamutColor.get_blue();
#endif

#if HAVE_LIBLCMS1
                            cmsSetAlarmCodes(gamutColor_r >> 8, gamutColor_g >> 8, gamutColor_b >> 8);
#elif HAVE_LIBLCMS2
                            cmsUInt16Number newAlarmCodes[cmsMAXCHANNELS] = {0};
                            newAlarmCodes[0] = gamutColor_r;
                            newAlarmCodes[1] = gamutColor_g;
                            newAlarmCodes[2] = gamutColor_b;
                            newAlarmCodes[3] = ~0;
                            cmsSetAlarmCodes(newAlarmCodes);
#endif
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



#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

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
