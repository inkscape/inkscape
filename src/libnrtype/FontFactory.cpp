/*
 *  FontFactory.cpp
 *  testICU
 *
 *   Authors:
 *     fred
 *     bulia byak <buliabyak@users.sf.net>
 *
 */

#include "FontFactory.h"
#include <libnrtype/font-instance.h>


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm/i18n.h> // _()

/* Freetype2 */
# include <pango/pangoft2.h>


// need to avoid using the size field
size_t font_descr_hash::operator()( PangoFontDescription *const &x) const {
    int h = 0;
    h *= 1128467;
    char const *theF = pango_font_description_get_family(x);
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
bool  font_descr_equal::operator()( PangoFontDescription *const&a, PangoFontDescription *const &b) {
    //if ( pango_font_description_equal(a,b) ) return true;
    char const *fa = pango_font_description_get_family(a);
    char const *fb = pango_font_description_get_family(b);
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
static int
style_name_compare(void const *aa, void const *bb)
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
        free(list->names[i]);
    if ( list->names ) nr_free(list->names);
}

static void font_factory_style_list_destructor(NRStyleList *list) 
{
    for (unsigned int i = 0; i < list->length; i++) {
        free((void *) (list->records)[i].name);
        free((void *) (list->records)[i].descr);
    }
    if ( list->records ) nr_free(list->records);
}

/**
 * On Win32 performs a stricmp(a,b), otherwise does a strcasecmp(a,b)
 */
static int
family_name_compare(void const *a, void const *b)
{
#ifndef WIN32
    return strcasecmp((*((char const **) a)), (*((char const **) b)));
#else
    return stricmp((*((char const **) a)), (*((char const **) b)));
#endif
}

void noop(...) {}
//#define PANGO_DEBUG g_print
#define PANGO_DEBUG noop



///////////////////// FontFactory
#ifndef USE_PANGO_WIN32
// the substitute function to tell fontconfig to enforce outline fonts
void FactorySubstituteFunc(FcPattern *pattern,gpointer /*data*/)
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

font_factory::font_factory(void)
{
    fontSize = 512;
    nbEnt = 0;
    maxEnt = 32;
    ents = (font_entry*)malloc(maxEnt*sizeof(font_entry));

#ifdef USE_PANGO_WIN32
    hScreenDC = pango_win32_get_dc();
    fontServer = pango_win32_font_map_for_display();
    fontContext = pango_win32_get_context();
    pangoFontCache = pango_win32_font_map_get_font_cache(fontServer);
#else
    fontServer = pango_ft2_font_map_new();
    pango_ft2_font_map_set_resolution((PangoFT2FontMap*)fontServer, 72, 72);
    fontContext = pango_ft2_font_map_create_context((PangoFT2FontMap*)fontServer);
    pango_ft2_font_map_set_default_substitute((PangoFT2FontMap*)fontServer,FactorySubstituteFunc,this,NULL);
#endif
}

font_factory::~font_factory(void)
{
    for (int i = 0;i < nbEnt;i++) ents[i].f->Unref();
    if ( ents ) free(ents);

    g_object_unref(fontServer);
#ifdef USE_PANGO_WIN32
    pango_win32_shutdown_display();
#else
    //pango_ft2_shutdown_display();
#endif
    //g_object_unref(fontContext);
}

font_instance *font_factory::FaceFromDescr(char const *family, char const *style)
{
    PangoFontDescription *temp_descr = pango_font_description_from_string(style);
    pango_font_description_set_family(temp_descr,family);
    font_instance *res = Face(temp_descr);
    pango_font_description_free(temp_descr);
    return res;
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
	
    if ( loadedFaces.find(descr) == loadedFaces.end() ) {
        // not yet loaded
        PangoFont *nFace = NULL;

        // workaround for bug #1025565.
        // fonts without families blow up Pango.
        if (pango_font_description_get_family(descr) != NULL) {
            nFace = pango_font_map_load_font(fontServer,fontContext,descr);
        }
        else {
            g_warning(_("Ignoring font without family that will crash Pango"));
        }

        if ( nFace ) {
            // duplicate FcPattern, the hard way
            res = new font_instance();
            // store the descr of the font we asked for, since this is the key where we intend to put the font_instance at
            // in the hash_map.  the descr of the returned pangofont may differ from what was asked, so we don't know (at this 
            // point) whether loadedFaces[that_descr] is free or not (and overwriting an entry will bring deallocation problems)
            res->descr = pango_font_description_copy(descr);
            res->daddy = this;
            res->InstallFace(nFace);
            if ( res->pFont == NULL ) {
                // failed to install face -> bitmap font
                // printf("face failed\n");
                res->daddy = NULL;
                delete res;
                res = NULL;
                if ( canFail ) {
                    char *tc = pango_font_description_to_string(descr);
                    PANGO_DEBUG("falling back from %s to Sans because InstallFace failed\n",tc);
                    free(tc);
                    pango_font_description_set_family(descr,"Sans");
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
                PANGO_DEBUG("falling back to Sans\n");
                pango_font_description_set_family(descr,"Sans");
                res = Face(descr,false);
            }
        }
    } else {
        // already here
        res = loadedFaces[descr];
        res->Ref();
        AddInCache(res);
    }
    res->InitTheFace();
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
	
    if ( apos.weight <= NR_POS_WEIGHT_ULTRA_LIGHT ) {
        pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_ULTRALIGHT);
    } else if ( apos.weight <= NR_POS_WEIGHT_LIGHT ) {
        pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_LIGHT);
    } else if ( apos.weight <= NR_POS_WEIGHT_NORMAL ) {
        pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_NORMAL);
    } else if ( apos.weight <= NR_POS_WEIGHT_BOLD ) {
        pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_BOLD);
    } else if ( apos.weight <= NR_POS_WEIGHT_ULTRA_BOLD ) {
        pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_ULTRABOLD);
    } else {
        pango_font_description_set_weight(temp_descr, PANGO_WEIGHT_HEAVY);
    }
	
    if ( apos.stretch <= NR_POS_STRETCH_ULTRA_CONDENSED ) {
        pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_EXTRA_CONDENSED);
    } else if ( apos.stretch <= NR_POS_STRETCH_CONDENSED ) {
        pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_CONDENSED);
    } else if ( apos.stretch <= NR_POS_STRETCH_SEMI_CONDENSED ) {
        pango_font_description_set_stretch(temp_descr, PANGO_STRETCH_SEMI_CONDENSED);
    } else if ( apos.stretch <= NR_POS_WEIGHT_NORMAL ) {
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
    if ( who == NULL ) return;
    if ( loadedFaces.find(who->descr) == loadedFaces.end() ) {
        // not found
        char *tc = pango_font_description_to_string(who->descr);
        g_warning("unrefFace %p=%s: failed\n",who,tc);
        free(tc);
    } else {
        loadedFaces.erase(loadedFaces.find(who->descr));
        //			printf("unrefFace %p: success\n",who);
    }
}

NRNameList *font_factory::Families(NRNameList *flist)
{
    PangoFontFamily**  fams = NULL;
    int                nbFam = 0;
    pango_font_map_list_families(fontServer, &fams, &nbFam);
	
    PANGO_DEBUG("got %d families\n", nbFam);
	
    flist->length = nbFam;
    flist->names = (guchar **)malloc(nbFam*sizeof(guchar*));
    flist->destructor = font_factory_name_list_destructor;
	
    for (int i = 0;i < nbFam;i++) {
// Note: on Windows, pango_font_family_get_name always returns lowercase name.
// As a result the list of fonts in the dialog is lowercase.
// We could work around by loading the font and taking pango_font_description_get_family from its descr (that gives correct case),
// but this is slow, and it's better to fix Pango instead.
        flist->names[i]=(guchar*)strdup(pango_font_family_get_name(fams[i]));
    }
	
    qsort(flist->names, nbFam, sizeof(guchar *), family_name_compare);
	
    g_free(fams);
	
    return flist;
}

NRStyleList *font_factory::Styles(gchar const *family, NRStyleList *slist)
{
    PangoFontFamily *theFam = NULL;
	
    // search available families
    {
        PangoFontFamily**  fams = NULL;
        int                nbFam = 0;
        pango_font_map_list_families(fontServer, &fams, &nbFam);
		
        for (int i = 0;i < nbFam;i++) {
            char const *fname = pango_font_family_get_name(fams[i]);
            if ( fname && strcmp(family,fname) == 0 ) {
                theFam = fams[i];
                break;
            }
        }
		
        g_free(fams);
    }
	
    // nothing found
    if ( theFam == NULL ) {
        slist->length = 0;
        slist->records = NULL;
        slist->destructor = NULL;
        return slist;
    }
	
    // search faces in the found family
    PangoFontFace**  faces = NULL;
    int nFaces = 0;
    pango_font_family_list_faces(theFam, &faces, &nFaces);
	
    slist->records = (NRStyleRecord *) malloc(nFaces * sizeof(NRStyleRecord));
    slist->destructor = font_factory_style_list_destructor;
	
    int nr = 0;
    for (int i = 0; i < nFaces; i++) {
		
        // no unnamed faces
        if (pango_font_face_get_face_name(faces[i]) == NULL)
            continue;
        PangoFontDescription *nd = pango_font_face_describe(faces[i]);
        if (nd == NULL)
            continue;
        char const *descr = pango_font_description_to_string(nd);
        if (descr == NULL) {
            pango_font_description_free(nd);
            continue;
        }
		
        char const *name = g_strdup(pango_font_face_get_face_name(faces[i]));
        pango_font_description_free(nd);
		
        slist->records[nr].name = name;
        slist->records[nr].descr = descr;
        nr ++;
    }
	
    slist->length = nr;
	
    qsort(slist->records, slist->length, sizeof(NRStyleRecord), style_record_compare);
    /* effic: Consider doing strdown and all the is_italic etc. tests once off and store the
     * results in a table, rather than having the sort invoke multiple is_italic tests per
     * record.
     */
	
    g_free(faces);
	
    return slist;
}

void font_factory::AddInCache(font_instance *who)
{
    if ( who == NULL ) return;
    for (int i = 0;i < nbEnt;i++) ents[i].age *= 0.9;
    for (int i = 0;i < nbEnt;i++) {
        if ( ents[i].f == who ) {
            //			printf("present\n");
            ents[i].age += 1.0;
            return;
        }
    }
    if ( nbEnt > maxEnt ) {
        printf("cache sur-plein?\n");
        return;
    }
    who->Ref();
    if ( nbEnt == maxEnt ) {
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
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
