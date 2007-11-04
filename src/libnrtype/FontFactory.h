/*
 *  FontFactory.h
 *  testICU
 *
 */

#ifndef my_font_factory
#define my_font_factory

#include <functional>
#include <algorithm>
#include <ext/hash_map>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#ifdef _WIN32
#define USE_PANGO_WIN32
#endif

#include <pango/pango.h>
#include "nr-type-primitives.h"
#include "nr-type-pos-def.h"
#include <libnrtype/nrtype-forward.h>

/* Freetype */
#ifdef USE_PANGO_WIN32
#include <pango/pangowin32.h>
#else
#include <pango/pangoft2.h>
#include <freetype/freetype.h>
#endif

// the font_factory keeps a hashmap of all the loaded font_instances, and uses the PangoFontDescription
// as index (nota: since pango already does that, using the PangoFont could work too)
struct font_descr_hash : public std::unary_function<PangoFontDescription*,size_t> {
    size_t operator()(PangoFontDescription *const &x) const;
};
struct font_descr_equal : public std::binary_function<PangoFontDescription*, PangoFontDescription*, bool> {
    bool operator()(PangoFontDescription *const &a, PangoFontDescription *const &b);
};

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

    __gnu_cxx::hash_map<PangoFontDescription*, font_instance*, font_descr_hash, font_descr_equal> loadedFaces;

    font_factory();
    virtual ~font_factory();

    /// Returns the default font_factory.
    static font_factory*  Default();

    // Various functions to get a font_instance from different descriptions.
    font_instance*        FaceFromDescr(char const *family, char const *style);
    font_instance*        Face(PangoFontDescription *descr, bool canFail=true);
    font_instance*        Face(char const *family,
                               int variant=PANGO_VARIANT_NORMAL, int style=PANGO_STYLE_NORMAL,
                               int weight=PANGO_WEIGHT_NORMAL, int stretch=PANGO_STRETCH_NORMAL,
                               int size=10, int spacing=0);
    font_instance*        Face(char const *family, NRTypePosDef apos);

    /// Semi-private: tells the font_factory taht the font_instance 'who' has died and should be removed from loadedFaces
    void                  UnrefFace(font_instance* who);

    // Queries for the font-selector.
    NRNameList*           Families(NRNameList *flist);
    NRStyleList*           Styles(const gchar *family, NRStyleList *slist);

    // internal
    void                  AddInCache(font_instance *who);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
