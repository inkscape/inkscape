/*
 *  FontFactory.cpp
 *  testICU
 *
 *   Authors:
 *     fred
 *     bulia byak <buliabyak@users.sf.net>
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef PANGO_ENABLE_ENGINE
#define PANGO_ENABLE_ENGINE
#endif

#include <glibmm/i18n.h>
#include <pango/pangoft2.h>
#include "libnrtype/FontFactory.h"
#include "libnrtype/font-instance.h"
#include "util/unordered-containers.h"

typedef INK_UNORDERED_MAP<PangoFontDescription*, font_instance*, font_descr_hash, font_descr_equal> FaceMapType;

// need to avoid using the size field
size_t font_descr_hash::operator()( PangoFontDescription *const &x) const {
    int h = 0;
    char const *theF = sp_font_description_get_family(x);
    h += (theF)?g_str_hash(theF):0;
    h *= 1128467;
    h += (int)pango_font_description_get_style(x);
    h *= 1128467;
    h += (int)pango_font_description_get_variant(x);
    h *= 1128467;
    h += (int)pango_font_description_get_weight(x);
    h *= 1128467;
    h += (int)pango_font_description_get_stretch(x);
    return h;
}
bool  font_descr_equal::operator()( PangoFontDescription *const&a, PangoFontDescription *const &b) const {
    //if ( pango_font_description_equal(a,b) ) return true;
    char const *fa = sp_font_description_get_family(a);
    char const *fb = sp_font_description_get_family(b);
    if ( ( fa && fb == NULL ) || ( fb && fa == NULL ) ) return false;
    if ( fa && fb && strcmp(fa,fb) != 0 ) return false;
    if ( pango_font_description_get_style(a) != pango_font_description_get_style(b) ) return false;
    if ( pango_font_description_get_variant(a) != pango_font_description_get_variant(b) ) return false;
    if ( pango_font_description_get_weight(a) != pango_font_description_get_weight(b) ) return false;
    if ( pango_font_description_get_stretch(a) != pango_font_description_get_stretch(b) ) return false;
    return true;
}

/////////////////// helper functions

static void noop(...) {}
//#define PANGO_DEBUG g_print
#define PANGO_DEBUG noop


///////////////////// FontFactory
#ifndef USE_PANGO_WIN32
// the substitute function to tell fontconfig to enforce outline fonts
static void FactorySubstituteFunc(FcPattern *pattern,gpointer /*data*/)
{
    FcPatternAddBool(pattern, "FC_OUTLINE",FcTrue);
    //char *fam = NULL;
    //FcPatternGetString(pattern, "FC_FAMILY",0, &fam);
    //printf("subst_f on %s\n",fam);
}
#endif


font_factory *font_factory::lUsine = NULL;

font_factory *font_factory::Default(void)
{
    if ( lUsine == NULL ) lUsine = new font_factory;
    return lUsine;
}

font_factory::font_factory(void) :
    nbEnt(0), // Note: this "ents" cache only keeps fonts from being unreffed, does not speed up access
    maxEnt(32),
    ents(static_cast<font_entry*>(g_malloc(maxEnt*sizeof(font_entry)))),

#ifdef USE_PANGO_WIN32
    fontServer(pango_win32_font_map_for_display()),
    fontContext(pango_win32_get_context()),
    pangoFontCache(pango_win32_font_map_get_font_cache(fontServer)),
    hScreenDC(pango_win32_get_dc()),
#else
    fontServer(pango_ft2_font_map_new()),
    fontContext(0),
#endif
    fontSize(512),
    loadedPtr(new FaceMapType())
{
    // std::cout << pango_version_string() << std::endl;
#ifdef USE_PANGO_WIN32
#else
    pango_ft2_font_map_set_resolution(PANGO_FT2_FONT_MAP(fontServer),
                                      72, 72);
    
    fontContext = pango_font_map_create_context(fontServer);

    pango_ft2_font_map_set_default_substitute(PANGO_FT2_FONT_MAP(fontServer),
                                              FactorySubstituteFunc,
                                              this,
                                              NULL);
#endif
}

font_factory::~font_factory(void)
{
    for (int i = 0;i < nbEnt;i++) ents[i].f->Unref();
    if ( ents ) g_free(ents);

    g_object_unref(fontServer);
#ifdef USE_PANGO_WIN32
    pango_win32_shutdown_display();
#else
    //pango_ft2_shutdown_display();
#endif
    //g_object_unref(fontContext);

    if (loadedPtr) {
        FaceMapType* tmp = static_cast<FaceMapType*>(loadedPtr);
        delete tmp;
        loadedPtr = 0;
    }

    // Delete the pango font pointers in the string to instance map
    PangoStringToDescrMap::iterator it = fontInstanceMap.begin();
    while (it != fontInstanceMap.end()) {
        pango_font_description_free((*it).second);
        ++it;
    }
}


Glib::ustring font_factory::ConstructFontSpecification(PangoFontDescription *font)
{
    Glib::ustring pangoString;

    g_assert(font);

    if (font) {
        // Once the format for the font specification is decided, it must be
        // kept.. if it is absolutely necessary to change it, the attribute
        // it is written to needs to have a new version so the legacy files
        // can be read.

        PangoFontDescription *copy = pango_font_description_copy(font);

        pango_font_description_unset_fields (copy, PANGO_FONT_MASK_SIZE);
        char * copyAsString = pango_font_description_to_string(copy);
        pangoString = copyAsString;
        g_free(copyAsString);
        copyAsString = 0;

        pango_font_description_free(copy);

    }

    return pangoString;
}

Glib::ustring font_factory::ConstructFontSpecification(font_instance *font)
{
    Glib::ustring pangoString;

    g_assert(font);

    if (font) {
        pangoString = ConstructFontSpecification(font->descr);
    }

    return pangoString;
}

/*
 * Wrap calls to pango_font_description_get_family
 * and replace some of the pango font names with generic css names
 * http://www.w3.org/TR/2008/REC-CSS2-20080411/fonts.html#generic-font-families
 *
 * This function should be called in place of pango_font_description_get_family()
 */
const char *sp_font_description_get_family(PangoFontDescription const *fontDescr) {

    static std::map<Glib::ustring, Glib::ustring> fontNameMap;
    std::map<Glib::ustring, Glib::ustring>::iterator it;

    if (fontNameMap.empty()) {
        fontNameMap.insert(std::make_pair("Sans", "sans-serif"));
        fontNameMap.insert(std::make_pair("Serif", "serif"));
        fontNameMap.insert(std::make_pair("Monospace", "monospace"));
    }

    const char *pangoFamily = pango_font_description_get_family(fontDescr);

    if (pangoFamily && ((it = fontNameMap.find(pangoFamily)) != fontNameMap.end())) {
        return ((Glib::ustring)it->second).c_str();
    }

    return pangoFamily;
}

Glib::ustring font_factory::GetUIFamilyString(PangoFontDescription const *fontDescr)
{
    Glib::ustring family;

    g_assert(fontDescr);

    if (fontDescr) {
        // For now, keep it as family name taken from pango
        const char *pangoFamily = sp_font_description_get_family(fontDescr);

        if( pangoFamily ) {
            family = pangoFamily;
        }
    }

    return family;
}

Glib::ustring font_factory::GetUIStyleString(PangoFontDescription const *fontDescr)
{
    Glib::ustring style;

    g_assert(fontDescr);

    if (fontDescr) {
        PangoFontDescription *fontDescrCopy = pango_font_description_copy(fontDescr);

        pango_font_description_unset_fields(fontDescrCopy, PANGO_FONT_MASK_FAMILY);
        pango_font_description_unset_fields(fontDescrCopy, PANGO_FONT_MASK_SIZE);

        // For now, keep it as style name taken from pango
        char *fontDescrAsString = pango_font_description_to_string(fontDescrCopy);
        style = fontDescrAsString;

        g_free(fontDescrAsString);
        fontDescrAsString = 0;
        pango_font_description_free(fontDescrCopy);
    }

    return style;
}

/**
    Replace font family leaving style alone (if possible).
    @param fontSpec the given font
    @param newFamily
    @return the changed fontspec, if the property can not be set return an empty string
    The routine first searches for an exact match.
    If no exact match found, calls FontSpecificationBestMatch().
*/
Glib::ustring font_factory::ReplaceFontSpecificationFamily(const Glib::ustring & fontSpec, const Glib::ustring & newFamily)
{
    Glib::ustring newFontSpec;

    // Although we are using the string from pango_font_description_to_string for the
    // font specification, we definitely cannot just set the new family in the
    // PangoFontDescription structure and ask for a new string.  This is because
    // what constitutes a "family" in our own UI may be different from how Pango
    // sees it.

    // Find the PangoFontDescription associated with the old font specification string.
    PangoStringToDescrMap::iterator it = fontInstanceMap.find(fontSpec);


    if (it != fontInstanceMap.end()) {
        // Description found!

        // Make copy
        PangoFontDescription *descr = pango_font_description_copy((*it).second);

        // Grab the old UI Family string from the descr
        Glib::ustring uiFamily = GetUIFamilyString(descr);

        // Replace the UI Family name with the new family name
        std::size_t found = fontSpec.find(uiFamily);
        if (found != Glib::ustring::npos) {

            // Add comma to end of newFamily... commas at end don't hurt but are
            // required if the last part of a family name is a valid font style
            // (e.g. "Arial Black").
            Glib::ustring newFamilyComma = newFamily;
            if( *newFamilyComma.rbegin() != ',' ) {
                newFamilyComma += ",";
            }
            newFontSpec = fontSpec;
            newFontSpec.erase(found, uiFamily.size());
            newFontSpec.insert(found, newFamilyComma);

            // If the new font specification does not exist in the reference maps,
            // search for the next best match for the faces in that style
            it = fontInstanceMap.find(newFontSpec);
            if (it == fontInstanceMap.end()) {

                // Search for best match, empty string returned if not found.
                newFontSpec = FontSpecificationBestMatch( newFontSpec );

            }
        }

        pango_font_description_free(descr);
    }

    return newFontSpec;
}

/**
    Apply style property to the given font
    @param fontSpec the given font
    @param turnOn true to set italic style
    @return the changed fontspec, if the property can not be set return an empty string
    The routine first searches for an exact match to "FontFamily Italic" or
    "Font Family Oblique" (turnOn is true) or "FontFamily" (turnOn is false).
    If no exact match found, calls FontSpecificationBestMatch().
*/
Glib::ustring font_factory::FontSpecificationSetItalic(const Glib::ustring & fontSpec, bool turnOn)
{
    Glib::ustring newFontSpec;

    // Find the PangoFontDescription associated with the font specification string.
    PangoStringToDescrMap::iterator it = fontInstanceMap.find(fontSpec);

    if (it != fontInstanceMap.end()) {
        // Description found!

        // Make copy.
        PangoFontDescription *descr = pango_font_description_copy((*it).second);

        PangoStyle style;
        if (turnOn) {
            // First try Oblique, we'll try Italic later
            style = PANGO_STYLE_OBLIQUE;
        } else {
            style = PANGO_STYLE_NORMAL;
        }

        pango_font_description_set_style(descr, style);

        newFontSpec = ConstructFontSpecification(descr);

        bool exactMatchFound = true;
        if (fontInstanceMap.find(newFontSpec) == fontInstanceMap.end()) {

            exactMatchFound = false;
            if (turnOn) {
                // Next try Italic
                style = PANGO_STYLE_ITALIC;
                pango_font_description_set_style(descr, style);

                exactMatchFound = true;
                if (fontInstanceMap.find(newFontSpec) == fontInstanceMap.end()) {
                    exactMatchFound = false;
                }
            }
        }

        // Search for best match, empty string returned if not found.
        if( !exactMatchFound ) {
           newFontSpec = FontSpecificationBestMatch( newFontSpec );
        }

        pango_font_description_free(descr);
    }

    return newFontSpec; // Empty if not found.
}

/**
    Apply weight property to the given font
    @param fontSpec the given font
    @param turnOn true to set bold
    @return the changed fontspec, if the property can not be set return an empty string
    This routine first searches for an exact match, if none found
    it calls FontSpecificationBestMatch().
*/
Glib::ustring font_factory::FontSpecificationSetBold(const Glib::ustring & fontSpec, bool turnOn)
{
    Glib::ustring newFontSpec;

    // Find the PangoFontDescription associated with the font specification string.
    PangoStringToDescrMap::iterator it = fontInstanceMap.find(fontSpec);

    if (it != fontInstanceMap.end()) {
        // Description found!

        // Make copy.
        PangoFontDescription *descr = pango_font_description_copy((*it).second);


        PangoWeight weight;
        if (turnOn) {
            weight = PANGO_WEIGHT_BOLD;
        } else {
            weight = PANGO_WEIGHT_NORMAL;
        }

        pango_font_description_set_weight(descr, weight);

        newFontSpec = ConstructFontSpecification(descr);

        if (fontInstanceMap.find(newFontSpec) == fontInstanceMap.end()) {
            // Search for best match, empty string returned if not found.
            newFontSpec = FontSpecificationBestMatch( newFontSpec );
        }

        pango_font_description_free(descr);
    }

    return newFontSpec; // Empty if not found.
}

/**
    Use pango_font_description_better_match() to find best font match.
    This handles cases like Century Schoolbook L where the "normal"
    font is Century Schoolbook L Medium so just removing Italic
    from the font name doesn't yield the correct name.
    @param fontSpec the given font
    @return the changed fontspec, if the property can not be set return an empty string
*/
// http://library.gnome.org/devel/pango/1.28/pango-Fonts.html#pango-font-description-better-match
Glib::ustring font_factory::FontSpecificationBestMatch(const Glib::ustring & fontSpec )
{

    Glib::ustring newFontSpec;

    // Look for exact match
    PangoStringToDescrMap::iterator it = fontInstanceMap.find(fontSpec);

    // If there is no exact match, look for the best match.
    if (it != fontInstanceMap.end()) {

        newFontSpec = fontSpec;

    } else {

        PangoFontDescription *fontDescr = pango_font_description_from_string(fontSpec.c_str());
        PangoFontDescription *bestMatchDescr = NULL;

        // Grab the UI Family string from the descr
        Glib::ustring family = GetUIFamilyString(fontDescr);
        Glib::ustring bestMatchDescription;

        bool setFirstFamilyMatch = false;
        for (it = fontInstanceMap.begin(); it != fontInstanceMap.end(); ++it) {

            Glib::ustring currentFontSpec = (*it).first;
            Glib::ustring currentFamily = GetUIFamilyString((*it).second);

            // Save some time by only looking at the right family.
            // Must use family name rather than fontSpec
            //   (otherwise DejaVu Sans matches DejaVu Sans Mono).
            if (currentFamily == family) {
                if (!setFirstFamilyMatch) {
                    // This ensures that the closest match is at least within the correct
                    // family rather than the first font in the list
                    bestMatchDescr = pango_font_description_copy((*it).second);
                    bestMatchDescription = currentFontSpec;
                    setFirstFamilyMatch = true;
                } else {
                    // Get the font description that corresponds, and
                    // then see if we've found a better match
                    PangoFontDescription *possibleMatch = pango_font_description_copy((*it).second);

                    if (pango_font_description_better_match(
                            fontDescr, bestMatchDescr, possibleMatch)) {

                        pango_font_description_free(bestMatchDescr);
                        bestMatchDescr = possibleMatch;
                        bestMatchDescription = currentFontSpec;
                    } else {
                        pango_font_description_free(possibleMatch);
                    }
                }
            }
        } // for

        newFontSpec = bestMatchDescription; // If NULL, then no match found

        pango_font_description_free(fontDescr);
        pango_font_description_free(bestMatchDescr);

    }

    return newFontSpec;
}

/////

// Calculate a Style "value" based on CSS values for ordering styles.
static int StyleNameValue( const Glib::ustring &style )
{

    PangoFontDescription *pfd = pango_font_description_from_string ( style.c_str() );
    int value =
        pango_font_description_get_weight ( pfd ) * 1000000 +
        pango_font_description_get_style  ( pfd ) *   10000 +
        pango_font_description_get_stretch( pfd ) *     100 +
        pango_font_description_get_variant( pfd );
    pango_font_description_free ( pfd );
    return value;
}

// Determines order in which styles are presented (sorted by CSS style values)
static bool StyleNameCompareInternal(const StyleNames &style1, const StyleNames &style2)
{
    return( StyleNameValue( style1.CssName ) < StyleNameValue( style2.CssName ) );
}

void font_factory::GetUIFamiliesAndStyles(FamilyToStylesMap *map)
{
    g_assert(map);

    if (map) {

        // Gather the family names as listed by Pango
        PangoFontFamily**  families = NULL;
        int numFamilies = 0;
        pango_font_map_list_families(fontServer, &families, &numFamilies);

        for (int currentFamily=0; currentFamily < numFamilies; currentFamily++) {

            // Gather the styles for this family
            PangoFontFace** faces = NULL;
            int numFaces = 0;
            pango_font_family_list_faces(families[currentFamily], &faces, &numFaces);

            for (int currentFace=0; currentFace < numFaces; currentFace++) {

                // If the face has a name, describe it, and then use the
                // description to get the UI family and face strings

                const gchar* displayName = pango_font_face_get_face_name(faces[currentFace]);
                if (displayName == NULL) {
                    continue;
                }

                PangoFontDescription *faceDescr = pango_font_face_describe(faces[currentFace]);
                if (faceDescr) {
                    Glib::ustring familyUIName = GetUIFamilyString(faceDescr);
                    Glib::ustring styleUIName = GetUIStyleString(faceDescr);
                    // std::cout << familyUIName << " " << styleUIName << " (" << displayName << ")" << std::endl;

                    // Disable synthesized (faux) font faces except for CSS generic faces
                    if (pango_font_face_is_synthesized(faces[currentFace]) ) {
                        if( familyUIName.compare( "sans-serif" ) != 0 &&
                            familyUIName.compare( "serif"      ) != 0 &&
                            familyUIName.compare( "monospace"  ) != 0 &&
                            familyUIName.compare( "fantasy"    ) != 0 &&
                            familyUIName.compare( "cursive"    ) != 0 ) {
                            //std::cout << "faux: " << familyUIName << "  |  " << styleUIName << std::endl;
                            continue;
                        }
                    } 

                    // Pango breaks the 1 to 1 mapping between Pango weights and CSS weights by
                    // adding Semi-Light (as of 1.36.7), Book (as of 1.24), and Ultra-Heavy (as of
                    // 1.24). We need to map these weights to CSS weights. Book and Ultra-Heavy
                    // are rarely used. Semi-Light (350) is problematic as it is halfway between
                    // Light (300) and Normal (400) and if care is not taken it is converted to
                    // Normal, rather than Light.
                    //
                    // Note: The ultimate solution to handling various weight in the same
                    // font family is to support the @font rules from CSS.
                    //
                    // Additional notes, helpful for debugging:
                    //   Pango's FC backend:
                    //     Weights defined in fontconfig/fontconfig.h
                    //     String equivalents in src/fcfreetype.c
                    //     Weight set from os2->usWeightClass
                    //   Use Fontforge: Element->Font Info...->OS/2->Misc->Weight Class to check font weight
                    size_t f = styleUIName.find( "Book" );
                    if( f != Glib::ustring::npos ) {
                        styleUIName.replace( f, 4, "Normal" );
                    }
                    f = styleUIName.find( "Semi-Light" );
                    if( f != Glib::ustring::npos ) {
                        styleUIName.replace( f, 10, "Light" );
                    }
                    f = styleUIName.find( "Ultra-Heavy" );
                    if( f != Glib::ustring::npos ) {
                        styleUIName.replace( f, 11, "Heavy" );
                    }

                    if (!familyUIName.empty() && !styleUIName.empty()) {

                        // Find the right place to put the style information, adding
                        // a map entry for the family name if it doesn't yet exist

                        FamilyToStylesMap::iterator iter = map->find(familyUIName);

                        // Insert new family
                        if (iter == map->end()) {
                            map->insert(std::make_pair(familyUIName, std::list<StyleNames>()));
                        }

                        // Insert into the style list and save the info in the reference maps
                        // only if the style does not yet exist

                        bool exists = false;
                        std::list<StyleNames> &styleList = (*map)[familyUIName];

                        for (std::list<StyleNames>::iterator it=styleList.begin();
                                 it != styleList.end();
                                 ++it) {
                            if ( (*it).CssName == styleUIName) {
                                exists = true;
                                std::cerr << "Warning: Font face with same CSS values already added: " << familyUIName << " " << styleUIName << " (" << (*it).DisplayName << ", " << displayName << ")" << std::endl;
                                break;
                            }
                        }

                        if (!exists) {
                            styleList.push_back( StyleNames(styleUIName,displayName) );

                            // Add the string info needed in the reference maps
                            fontStringMap.insert(
                                    std::make_pair(
                                            Glib::ustring(familyUIName) + Glib::ustring(styleUIName),
                                            ConstructFontSpecification(faceDescr)));
                            fontInstanceMap.insert(
                                    std::make_pair(ConstructFontSpecification(faceDescr), faceDescr));

                        } else {
                            pango_font_description_free(faceDescr);
                        }
                    } else {
                        pango_font_description_free(faceDescr);
                    }
                }
            }
            g_free(faces);
            faces = 0;
        }
        g_free(families);
        families = 0;

        // Sort the style lists
        for (FamilyToStylesMap::iterator iter = map->begin() ; iter != map->end(); ++iter) {
            (*iter).second.sort(StyleNameCompareInternal);
        }
    }
}

font_instance* font_factory::FaceFromStyle(SPStyle const *style)
{
    font_instance *font = NULL;

    g_assert(style);

    if (style) {

        //  First try to use the font specification if it is set
        if (style->font_specification.set
            && style->font_specification.value
            && *style->font_specification.value) {

            font = FaceFromFontSpecification(style->font_specification.value);
        }

        // If that failed, try using the CSS information in the style
        if (!font) {

            PangoFontDescription *temp_descr = pango_font_description_new();

            pango_font_description_set_family(temp_descr, style->font_family.value);

            // This duplicates Layout::EnumConversionItem... perhaps we can share code?
            switch ( style->font_style.computed ) {
                case SP_CSS_FONT_STYLE_ITALIC:
                    pango_font_description_set_style(temp_descr, PANGO_STYLE_ITALIC);
                    break;

                case SP_CSS_FONT_STYLE_OBLIQUE:
                    pango_font_description_set_style(temp_descr, PANGO_STYLE_OBLIQUE);
                    break;

                case SP_CSS_FONT_STYLE_NORMAL:
                default:
                    pango_font_description_set_style(temp_descr, PANGO_STYLE_NORMAL);
                    break;
            }

            switch( style->font_weight.computed ) {
                case SP_CSS_FONT_WEIGHT_100:
                    pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_THIN);
                    break;

                case SP_CSS_FONT_WEIGHT_200:
                    pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_ULTRALIGHT);
                    break;

                case SP_CSS_FONT_WEIGHT_300:
                    pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_LIGHT);
                    break;

                case SP_CSS_FONT_WEIGHT_400:
                case SP_CSS_FONT_WEIGHT_NORMAL:
                    pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_NORMAL);
                    break;

                case SP_CSS_FONT_WEIGHT_500:
                    pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_MEDIUM);
                    break;

                case SP_CSS_FONT_WEIGHT_600:
                    pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_SEMIBOLD);
                    break;

                case SP_CSS_FONT_WEIGHT_700:
                case SP_CSS_FONT_WEIGHT_BOLD:
                    pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_BOLD);
                    break;

                case SP_CSS_FONT_WEIGHT_800:
                    pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_ULTRABOLD);
                    break;

                case SP_CSS_FONT_WEIGHT_900:
                    pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_HEAVY);
                    break;

                case SP_CSS_FONT_WEIGHT_LIGHTER:
                case SP_CSS_FONT_WEIGHT_BOLDER:
                default:
                    g_warning("FaceFromStyle: Unrecognized font_weight.computed value");
                    pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_NORMAL);
                    break;
            }
            // PANGO_WIEGHT_ULTRAHEAVY not used (not CSS2)

            switch (style->font_stretch.computed) {
                case SP_CSS_FONT_STRETCH_ULTRA_CONDENSED:
                    pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_ULTRA_CONDENSED);
                    break;

                case SP_CSS_FONT_STRETCH_EXTRA_CONDENSED:
                    pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_EXTRA_CONDENSED);
                    break;

                case SP_CSS_FONT_STRETCH_CONDENSED:
                    pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_CONDENSED);
                    break;

                case SP_CSS_FONT_STRETCH_SEMI_CONDENSED:
                    pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_SEMI_CONDENSED);
                    break;

                case SP_CSS_FONT_STRETCH_NORMAL:
                    pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_NORMAL);
                    break;

                case SP_CSS_FONT_STRETCH_SEMI_EXPANDED:
                    pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_SEMI_EXPANDED);
                    break;

                case SP_CSS_FONT_STRETCH_EXPANDED:
                    pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_EXPANDED);
                    break;

                case SP_CSS_FONT_STRETCH_EXTRA_EXPANDED:
                    pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_EXTRA_EXPANDED);
                    break;

                case SP_CSS_FONT_STRETCH_ULTRA_EXPANDED:
                    pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_ULTRA_EXPANDED);

                case SP_CSS_FONT_STRETCH_WIDER:
                case SP_CSS_FONT_STRETCH_NARROWER:
                default:
                    g_warning("FaceFromStyle: Unrecognized font_stretch.computed value");
                    pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_NORMAL);
                    break;
            }

            switch ( style->font_variant.computed ) {
                case SP_CSS_FONT_VARIANT_SMALL_CAPS:
                    pango_font_description_set_variant(temp_descr, PANGO_VARIANT_SMALL_CAPS);
                    break;

                case SP_CSS_FONT_VARIANT_NORMAL:
                default:
                    pango_font_description_set_variant(temp_descr, PANGO_VARIANT_NORMAL);
                    break;
            }

            font = Face(temp_descr);
            pango_font_description_free(temp_descr);

            // We now find closest match to this font:
            Glib::ustring fontSpec = font_factory::Default()->ConstructFontSpecification(font);
            Glib::ustring newFontSpec = FontSpecificationBestMatch( fontSpec );
            if( fontSpec != newFontSpec ) {
                font->Unref();
                font = FaceFromFontSpecification( newFontSpec.c_str() );
            }
        }
    }

    return font;
}

font_instance *font_factory::FaceFromDescr(char const *family, char const *style)
{
    PangoFontDescription *temp_descr = pango_font_description_from_string(style);
    pango_font_description_set_family(temp_descr,family);
    font_instance *res = Face(temp_descr);
    pango_font_description_free(temp_descr);
    return res;
}

font_instance* font_factory::FaceFromUIStrings(char const *uiFamily, char const *uiStyle)
{
    font_instance *fontInstance = NULL;

    g_assert(uiFamily && uiStyle);
    if (uiFamily && uiStyle) {

        // If font list, take only first font in list
        gchar** tokens = g_strsplit( uiFamily, ",", 0 );
        g_strstrip( tokens[0] );

        Glib::ustring uiString = Glib::ustring(tokens[0]) + Glib::ustring(uiStyle);

        g_strfreev( tokens );

        UIStringToPangoStringMap::iterator uiToPangoIter = fontStringMap.find(uiString);

        if (uiToPangoIter != fontStringMap.end ()) {
            PangoStringToDescrMap::iterator pangoToDescrIter = fontInstanceMap.find((*uiToPangoIter).second);
            if (pangoToDescrIter != fontInstanceMap.end()) {
                // We found the pango description - now we can make a font_instance
                PangoFontDescription *tempDescr = pango_font_description_copy((*pangoToDescrIter).second);
                fontInstance = Face(tempDescr);
                pango_font_description_free(tempDescr);
            }
        }
    }

    return fontInstance;
}

font_instance* font_factory::FaceFromPangoString(char const *pangoString)
{
    font_instance *fontInstance = NULL;

    g_assert(pangoString);

    if (pangoString) {
        PangoFontDescription *descr = NULL;

        // First attempt to find the font specification in the reference map
        PangoStringToDescrMap::iterator it = fontInstanceMap.find(Glib::ustring(pangoString));
        if (it != fontInstanceMap.end()) {
            descr = pango_font_description_copy((*it).second);
        }

        // Or create a font description from the string - this may fail or
        // produce unexpected results if the string does not have a good format
        if (!descr) {
            descr = pango_font_description_from_string(pangoString);
        }

        if (descr && (sp_font_description_get_family(descr) != NULL)) {
            fontInstance = Face(descr);
        }

        if (descr) {
            pango_font_description_free(descr);
        }
    }

    return fontInstance;
}

font_instance* font_factory::FaceFromFontSpecification(char const *fontSpecification)
{
    font_instance *font = NULL;

    g_assert(fontSpecification);

    if (fontSpecification) {
        // How the string is used to reconstruct a font depends on how it
        // was constructed in ConstructFontSpecification.  As it stands,
        // the font specification is a pango-created string
        font = FaceFromPangoString(fontSpecification);
    }

    return font;
}



font_instance *font_factory::Face(PangoFontDescription *descr, bool canFail)
{
#ifdef USE_PANGO_WIN32
    // damn Pango fudges the size, so we need to unfudge. See source of pango_win32_font_map_init()
    pango_font_description_set_size(descr, (int) (fontSize*PANGO_SCALE*72/GetDeviceCaps(pango_win32_get_dc(),LOGPIXELSY))); // mandatory huge size (hinting workaround)
#else
    pango_font_description_set_size(descr, (int) (fontSize*PANGO_SCALE)); // mandatory huge size (hinting workaround)
#endif

    font_instance *res = NULL;

    FaceMapType& loadedFaces = *static_cast<FaceMapType*>(loadedPtr);
    if ( loadedFaces.find(descr) == loadedFaces.end() ) {
        // not yet loaded
        PangoFont *nFace = NULL;

        // workaround for bug #1025565.
        // fonts without families blow up Pango.
        if (sp_font_description_get_family(descr) != NULL) {
            nFace = pango_font_map_load_font(fontServer,fontContext,descr);
        }
        else {
            g_warning("%s", _("Ignoring font without family that will crash Pango"));
        }

        if ( nFace ) {
            // duplicate FcPattern, the hard way
            res = new font_instance();
            // store the descr of the font we asked for, since this is the key where we intend
            // to put the font_instance at in the unordered_map.  the descr of the returned
            // pangofont may differ from what was asked, so we don't know (at this
            // point) whether loadedFaces[that_descr] is free or not (and overwriting
            // an entry will bring deallocation problems)
            res->descr = pango_font_description_copy(descr);
            res->parent = this;
            res->InstallFace(nFace);
            if ( res->pFont == NULL ) {
                // failed to install face -> bitmap font
                // printf("face failed\n");
                res->parent = NULL;
                delete res;
                res = NULL;
                if ( canFail ) {
                    char *tc = pango_font_description_to_string(descr);
                    PANGO_DEBUG("falling back from %s to 'sans-serif' because InstallFace failed\n",tc);
                    g_free(tc);
                    pango_font_description_set_family(descr,"sans-serif");
                    res = Face(descr,false);
                }
            } else {
                loadedFaces[res->descr]=res;
                res->Ref();
                AddInCache(res);
            }
        } else {
            // no match
            if ( canFail ) {
                PANGO_DEBUG("falling back to 'sans-serif'\n");
                descr = pango_font_description_new();
                pango_font_description_set_family(descr,"sans-serif");
                res = Face(descr,false);
                pango_font_description_free(descr);
            }
        }
    } else {
        // already here
        res = loadedFaces[descr];
        res->Ref();
        AddInCache(res);
    }
    if (res) {
        res->InitTheFace();
    }
    return res;
}

font_instance *font_factory::Face(char const *family, int variant, int style, int weight, int stretch, int /*size*/, int /*spacing*/)
{
    PangoFontDescription *temp_descr = pango_font_description_new();
    pango_font_description_set_family(temp_descr,family);
    pango_font_description_set_weight(temp_descr,(PangoWeight)weight);
    pango_font_description_set_stretch(temp_descr,(PangoStretch)stretch);
    pango_font_description_set_style(temp_descr,(PangoStyle)style);
    pango_font_description_set_variant(temp_descr,(PangoVariant)variant);
    font_instance *res = Face(temp_descr);
    pango_font_description_free(temp_descr);
    return res;
}

void font_factory::UnrefFace(font_instance *who)
{
    if ( who ) {
        FaceMapType& loadedFaces = *static_cast<FaceMapType*>(loadedPtr);

        if ( loadedFaces.find(who->descr) == loadedFaces.end() ) {
            // not found
            char *tc = pango_font_description_to_string(who->descr);
            g_warning("unrefFace %p=%s: failed\n",who,tc);
            g_free(tc);
        } else {
            loadedFaces.erase(loadedFaces.find(who->descr));
            //            printf("unrefFace %p: success\n",who);
        }
    }
}

void font_factory::AddInCache(font_instance *who)
{
    if ( who == NULL ) return;
    for (int i = 0;i < nbEnt;i++) ents[i].age *= 0.9;
    for (int i = 0;i < nbEnt;i++) {
        if ( ents[i].f == who ) {
            //            printf("present\n");
            ents[i].age += 1.0;
            return;
        }
    }
    if ( nbEnt > maxEnt ) {
        printf("cache sur-plein?\n");
        return;
    }
    who->Ref();
    if ( nbEnt == maxEnt ) { // cache is filled, unref the oldest-accessed font in it
        int    bi = 0;
        double ba = ents[bi].age;
        for (int i = 1;i < nbEnt;i++) {
            if ( ents[i].age < ba ) {
                bi = i;
                ba = ents[bi].age;
            }
        }
        ents[bi].f->Unref();
        ents[bi]=ents[--nbEnt];
    }
    ents[nbEnt].f = who;
    ents[nbEnt].age = 1.0;
    nbEnt++;
}

/*
        {
            std::cout << " Printing out fontInstanceMap: " << std::endl;
            PangoStringToDescrMap::iterator it = fontInstanceMap.begin();
            while (it != fontInstanceMap.end()) {

                PangoFontDescription *descr = pango_font_description_copy((*it).second);

                // Grab the UI Family string from the descr
                Glib::ustring uiFamily = GetUIFamilyString(descr);
                Glib::ustring uiStyle  = GetUIStyleString(descr);
                std::cout << "     " << uiFamily << "  " << uiStyle << std::endl;

                it++;
            }
        }
*/

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
