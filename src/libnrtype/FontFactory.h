/*
 *  FontFactory.h
 *  testICU
 *
 */

#ifndef my_font_factory
#define my_font_factory

//#include <glibmm/ustring.h>

#include <functional>
#include <algorithm>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#ifdef _WIN32
//#define USE_PANGO_WIN32 // disable for Bug 165665
#endif

#include <pango/pango.h>
#include "nr-type-primitives.h"
#include "style.h"

/* Freetype */
#ifdef USE_PANGO_WIN32
#include <pango/pangowin32.h>
#else
#include <pango/pangoft2.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#endif


class font_instance;

namespace Glib
{
    class ustring;
}

// the font_factory keeps a hashmap of all the loaded font_instances, and uses the PangoFontDescription
// as index (nota: since pango already does that, using the PangoFont could work too)
struct font_descr_hash : public std::unary_function<PangoFontDescription*,size_t> {
    size_t operator()(PangoFontDescription *const &x) const;
};
struct font_descr_equal : public std::binary_function<PangoFontDescription*, PangoFontDescription*, bool> {
    bool operator()(PangoFontDescription *const &a, PangoFontDescription *const &b) const;
};

// Wraps calls to pango_font_description_get_family with some name substitution
const char *sp_font_description_get_family(PangoFontDescription const *fontDescr);

// Class for style strings: both CSS and as suggested by font.
class StyleNames {

public:
    StyleNames() {};
    StyleNames( Glib::ustring name ) :
        CssName( name ), DisplayName( name ) {};
    StyleNames( Glib::ustring cssname, Glib::ustring displayname ) :
        CssName( cssname ), DisplayName( displayname ) {};

public:
    Glib::ustring CssName;     // Style as Pango/CSS would write it.
    Glib::ustring DisplayName; // Style as Font designer named it.
};

// Map type for gathering UI family and style names
// typedef std::map<Glib::ustring, std::list<StyleNames> > FamilyToStylesMap;

class font_factory {
public:
    static font_factory *lUsine; /**< The default font_factory; i cannot think of why we would
                                  *   need more than one.
                                  *
                                  *   ("l'usine" is french for "the factory".)
                                  */

    /** A little cache for fonts, so that you don't loose your time looking up fonts in the font list
     *  each font in the cache is refcounted once (and deref'd when removed from the cache). */
    struct font_entry {
        font_instance *f;
        double age;
    };
    int nbEnt;   ///< Number of entries.
    int maxEnt;  ///< Cache size.
    font_entry *ents;

    // Pango data.  Backend-specific structures are cast to these opaque types.
    PangoFontMap *fontServer;
    PangoContext *fontContext;
#ifdef USE_PANGO_WIN32
    PangoWin32FontCache *pangoFontCache;
    HDC hScreenDC;
#endif
    double fontSize; /**< The huge fontsize used as workaround for hinting.
                      *   Different between freetype and win32. */

    font_factory();
    virtual ~font_factory();

    /// Returns the default font_factory.
    static font_factory*  Default();

    /// Constructs a pango string for use with the fontStringMap (see below)
    Glib::ustring         ConstructFontSpecification(PangoFontDescription *font);
    Glib::ustring         ConstructFontSpecification(font_instance *font);

    /// Returns strings to be used in the UI for family and face (or "style" as the column is labeled)
    Glib::ustring         GetUIFamilyString(PangoFontDescription const *fontDescr);
    Glib::ustring         GetUIStyleString(PangoFontDescription const *fontDescr);

    // Helpfully inserts all font families into the provided vector
    void                  GetUIFamilies(std::vector<PangoFontFamily *>& out);
    // Retrieves style information about a family in a newly allocated GList.
    GList*                GetUIStyles(PangoFontFamily * in);

    /// Retrieve a font_instance from a style object, first trying to use the font-specification, the CSS information
    font_instance*        FaceFromStyle(SPStyle const *style);

    // Various functions to get a font_instance from different descriptions.
    font_instance*        FaceFromDescr(char const *family, char const *style);
    font_instance*        FaceFromUIStrings(char const *uiFamily, char const *uiStyle);
    font_instance*        FaceFromPangoString(char const *pangoString);
    font_instance*        FaceFromFontSpecification(char const *fontSpecification);
    font_instance*        Face(PangoFontDescription *descr, bool canFail=true);
    font_instance*        Face(char const *family,
                               int variant=PANGO_VARIANT_NORMAL, int style=PANGO_STYLE_NORMAL,
                               int weight=PANGO_WEIGHT_NORMAL, int stretch=PANGO_STRETCH_NORMAL,
                               int size=10, int spacing=0);

    /// Semi-private: tells the font_factory taht the font_instance 'who' has died and should be removed from loadedFaces
    void                  UnrefFace(font_instance* who);

    // internal
    void                  AddInCache(font_instance *who);

private:
    void*                 loadedPtr;


    // The following two commented out maps were an attempt to allow Inkscape to use font faces
    // that could not be distinguished by CSS values alone. In practice, they never were that
    // useful as PangoFontDescription, which is used throughout our code, cannot distinguish
    // between faces anymore than raw CSS values (with the exception of two additional weight
    // values).
    //
    // During various works, for example to handle font-family lists and fonts that are not
    // installed on the system, the code has become less reliant on these maps. And in the work to
    // catch style information to speed up start up times, the maps were not being filled.
    // I've removed all code that used these maps as of Oct 2014 in the experimental branch.
    // The commented out maps are left here as a reminder of the path that was attempted.
    //
    // One possible method to keep track of font faces would be to use the 'display name', keeping
    // pointers to the appropriate PangoFontFace. The font_factory loadedFaces map indexing would
    // have to be changed to incorporate 'display name' (InkscapeFontDescription?).


    // These two maps are used for translating between what's in the UI and a pango
    // font description.  This is necessary because Pango cannot always
    // reproduce these structures from the names it gave us in the first place.

    // Key: A string produced by font_factory::ConstructFontSpecification
    // Value: The associated PangoFontDescription
    // typedef std::map<Glib::ustring, PangoFontDescription *> PangoStringToDescrMap;
    // PangoStringToDescrMap fontInstanceMap;

    // Key: Family name in UI + Style name in UI
    // Value: The associated string that should be produced with font_factory::ConstructFontSpecification
    // typedef std::map<Glib::ustring, Glib::ustring> UIStringToPangoStringMap;
    // UIStringToPangoStringMap fontStringMap;
};


#endif /* my_font_factory */


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
