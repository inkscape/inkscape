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

/**
 * A wrapper for strcasestr that also provides an implementation for Win32.
 */
static bool
ink_strstr(char const *haystack, char const *pneedle)
{
    // windows has no strcasestr implementation, so here is ours...
    // stolen from nmap
    /* FIXME: This is broken for e.g. ink_strstr("aab", "ab").  Report to nmap.
     *
     * Also, suggest use of g_ascii_todown instead of buffer stuff, and g_ascii_tolower instead
     * of tolower.  Given that haystack is a font name (i.e. fairly short), it should be ok to
     * do g_ascii_strdown on both haystack and pneedle, and do normal strstr.
     *
     * Rather than fixing in inkscape, consider getting rid of this routine, instead using
     * strdown and plain strstr at caller.  We have control over the needle values, so we can
     * modify the callers rather than calling strdown there.
     */
    char buf[512];
    register char const *p;
    char *needle, *q, *foundto;
    if (!*pneedle) return true;
    if (!haystack) return false;

    needle = buf;
    p = pneedle; q = needle;
    while ((*q++ = tolower(*p++)))
        ;
    p = haystack - 1; foundto = needle;
    while (*++p) {
        if (tolower(*p) == *foundto) {
            if (!*++foundto) {
                /* Yeah, we found it */
                return true;
            }
        } else foundto = needle;
    }
    return false;
}

/**
 * Regular fonts are 'Regular', 'Roman', 'Normal', or 'Plain'
 */
// FIXME: make this UTF8, add non-English style names
static bool
is_regular(char const *s)
{
    if (ink_strstr(s, "Regular")) return true;
    if (ink_strstr(s, "Roman")) return true;
    if (ink_strstr(s, "Normal")) return true;
    if (ink_strstr(s, "Plain")) return true;
    return false;
}

/**
 * Non-bold fonts are 'Medium' or 'Book'
 */
static bool
is_nonbold(char const *s)
{
    if (ink_strstr(s, "Medium")) return true;
    if (ink_strstr(s, "Book")) return true;
    return false;
}

/**
 * Italic fonts are 'Italic', 'Oblique', or 'Slanted'
 */
static bool
is_italic(char const *s)
{
    if (ink_strstr(s, "Italic")) return true;
    if (ink_strstr(s, "Oblique")) return true;
    if (ink_strstr(s, "Slanted")) return true;
    return false;
}

/**
 * Bold fonts are 'Bold'
 */
static bool
is_bold(char const *s)
{
    if (ink_strstr(s, "Bold")) return true;
    return false;
}

/**
 * Caps fonts are 'Caps'
 */
static bool
is_caps(char const *s)
{
    if (ink_strstr(s, "Caps")) return true;
    return false;
}

#if 0 /* FIXME: These are all unused.  Please delete them or use them (presumably in
* style_name_compare). */
/**
 * Monospaced fonts are 'Mono'
 */
static bool
is_mono(char const *s)
{
    if (ink_strstr(s, "Mono")) return true;
    return false;
}

/**
 * Rounded fonts are 'Round'
 */
static bool
is_round(char const *s)
{
    if (ink_strstr(s, "Round")) return true;
    return false;
}

/**
 * Outline fonts are 'Outline'
 */
static bool
is_outline(char const *s)
{
    if (ink_strstr(s, "Outline")) return true;
    return false;
}

/**
 * Swash fonts are 'Swash'
 */
static bool
is_swash(char const *s)
{
    if (ink_strstr(s, "Swash")) return true;
    return false;
}
#endif

/**
 * Determines if two style names match.  This allows us to match
 * based on the type of style rather than simply doing string matching,
 * because for instance 'Plain' and 'Normal' mean the same thing.
 *
 * Q:  Shouldn't this include the other tests such as is_outline, etc.?
 * Q:  Is there a problem with strcasecmp on Win32?  Should it use stricmp?
 */
int
style_name_compare(char const *aa, char const *bb)
{
    char const *a = (char const *) aa;
    char const *b = (char const *) bb;

    if (is_regular(a) && !is_regular(b)) return -1;
    if (is_regular(b) && !is_regular(a)) return 1;

    if (is_bold(a) && !is_bold(b)) return 1;
    if (is_bold(b) && !is_bold(a)) return -1;

    if (is_italic(a) && !is_italic(b)) return 1;
    if (is_italic(b) && !is_italic(a)) return -1;

    if (is_nonbold(a) && !is_nonbold(b)) return 1;
    if (is_nonbold(b) && !is_nonbold(a)) return -1;

    if (is_caps(a) && !is_caps(b)) return 1;
    if (is_caps(b) && !is_caps(a)) return -1;

    return strcasecmp(a, b);
}

/*
 defined but not used:

static int
style_record_compare(void const *aa, void const *bb)
{
    NRStyleRecord const *a = (NRStyleRecord const *) aa;
    NRStyleRecord const *b = (NRStyleRecord const *) bb;

    return (style_name_compare(a->name, b->name));
}

static void font_factory_name_list_destructor(NRNameList *list)
{
    for (unsigned int i = 0; i < list->length; i++)
        g_free(list->names[i]);
    if ( list->names ) g_free(list->names);
}

static void font_factory_style_list_destructor(NRStyleList *list)
{
    for (unsigned int i = 0; i < list->length; i++) {
        g_free((void *) (list->records)[i].name);
        g_free((void *) (list->records)[i].descr);
    }
    if ( list->records ) g_free(list->records);
}
*/

/**
 * On Win32 performs a stricmp(a,b), otherwise does a strcasecmp(a,b)
 */
int
family_name_compare(char const *a, char const *b)
{
#ifndef WIN32
    return strcasecmp((*((char const **) a)), (*((char const **) b)));
#else
    return stricmp((*((char const **) a)), (*((char const **) b)));
#endif
}

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

static bool StyleNameCompareInternal(Glib::ustring style1, Glib::ustring style2)
{
    return (style_name_compare(style1.c_str(), style2.c_str()) < 0);
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

                if (pango_font_face_get_face_name(faces[currentFace]) == NULL) {
                    continue;
                }

                PangoFontDescription *faceDescr = pango_font_face_describe(faces[currentFace]);
                if (faceDescr) {
                    Glib::ustring familyUIName = GetUIFamilyString(faceDescr);
                    Glib::ustring styleUIName = GetUIStyleString(faceDescr);

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

                    if (!familyUIName.empty() && !styleUIName.empty()) {

                        // Find the right place to put the style information, adding
                        // a map entry for the family name if it doesn't yet exist

                        FamilyToStylesMap::iterator iter = map->find(familyUIName);

                        // Insert new family
                        if (iter == map->end()) {
                            map->insert(std::make_pair(familyUIName, std::list<Glib::ustring>()));
                        }

                        // Insert into the style list and save the info in the reference maps
                        // only if the style does not yet exist

                        bool exists = false;
                        std::list<Glib::ustring> &styleList = (*map)[familyUIName];

                        for (std::list<Glib::ustring>::iterator it=styleList.begin();
                                 it != styleList.end();
                                 ++it) {
                            if (*it == styleUIName) {
                                exists = true;
                                break;
                            }
                        }

                        if (!exists) {
                            styleList.push_back(styleUIName);

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

            font = Face(style->font_family.value, font_style_to_pos(*style));

            // That was a hatchet job... so we need to check if this font exists!!
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

font_instance *font_factory::Face(char const *family, NRTypePosDef apos)
{
    PangoFontDescription *temp_descr = pango_font_description_new();

    pango_font_description_set_family(temp_descr, family);

    if ( apos.variant == NR_POS_VARIANT_SMALLCAPS ) {
        pango_font_description_set_variant(temp_descr, PANGO_VARIANT_SMALL_CAPS);
    } else {
        pango_font_description_set_variant(temp_descr, PANGO_VARIANT_NORMAL);
    }

    if ( apos.italic ) {
        pango_font_description_set_style(temp_descr, PANGO_STYLE_ITALIC);
    } else if ( apos.oblique ) {
        pango_font_description_set_style(temp_descr, PANGO_STYLE_OBLIQUE);
    } else {
        pango_font_description_set_style(temp_descr, PANGO_STYLE_NORMAL);
    }

    if ( apos.weight <= NR_POS_WEIGHT_THIN ) {
        pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_THIN);
    } else if ( apos.weight <= NR_POS_WEIGHT_ULTRA_LIGHT ) {
        pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_ULTRALIGHT);
    } else if ( apos.weight <= NR_POS_WEIGHT_LIGHT ) {
        pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_LIGHT);
    } else if ( apos.weight <= NR_POS_WEIGHT_BOOK ) {
        pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_BOOK);
    } else if ( apos.weight <= NR_POS_WEIGHT_NORMAL ) {
        pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_NORMAL);
    } else if ( apos.weight <= NR_POS_WEIGHT_MEDIUM ) {
        pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_MEDIUM);
    } else if ( apos.weight <= NR_POS_WEIGHT_SEMIBOLD ) {
        pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_SEMIBOLD);
    } else if ( apos.weight <= NR_POS_WEIGHT_BOLD ) {
        pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_BOLD);
    } else if ( apos.weight <= NR_POS_WEIGHT_ULTRA_BOLD ) {
        pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_ULTRABOLD);
    } else {
        pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_HEAVY);
    }
    // PANGO_WIEGHT_ULTRAHEAVY not used (not CSS2)

    if ( apos.stretch <= NR_POS_STRETCH_ULTRA_CONDENSED ) {
        pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_EXTRA_CONDENSED);
    } else if ( apos.stretch <= NR_POS_STRETCH_CONDENSED ) {
        pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_CONDENSED);
    } else if ( apos.stretch <= NR_POS_STRETCH_SEMI_CONDENSED ) {
        pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_SEMI_CONDENSED);
    } else if ( apos.stretch <= NR_POS_STRETCH_NORMAL ) {
        pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_NORMAL);
    } else if ( apos.stretch <= NR_POS_STRETCH_SEMI_EXPANDED ) {
        pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_SEMI_EXPANDED);
    } else if ( apos.stretch <= NR_POS_STRETCH_EXPANDED ) {
        pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_EXPANDED);
    } else {
        pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_EXTRA_EXPANDED);
    }

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
