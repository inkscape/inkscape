/**
  @file text_reassemble.h  libTERE headers.

See text_reassemble.c for notes

File:      text_reassemble.h
Version:   0.0.13
Date:      06-FEB-2014
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2014 David Mathog and California Institute of Technology (Caltech)
*/

#ifndef _TEXT_REASSEMBLE_
#define _TEXT_REASSEMBLE_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <ctype.h>
#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include <iconv.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

/** \cond */
#define TEREMIN(A,B) (A < B ? A : B)
#define TEREMAX(A,B) (A > B ? A : B)

#ifndef M_PI
# define M_PI		3.14159265358979323846	/* pi */
#endif 
#define ALLOCINFO_CHUNK 32
#define ALLOCOUT_CHUNK  8192
#define TRPRINT trinfo_append_out
/** \endcond */

/** \defgroup color background options
  Text is underwritten with the background color not at all, 
  by reassembled line, or by full assembly .
  @{
*/
#define BKCLR_NONE 0x00     /**< text is not underwritten with background color (default) */
#define BKCLR_FRAG 0x01     /**< each fragment of text is underwritten with background color  */
#define BKCLR_LINE 0x02     /**< each line of text is underwritten with background color  */
#define BKCLR_ALL  0x03     /**< entire assembly is underwritten with background color    */
/** @} */

/** \defgroup decoration options
  One of these values may be present in the decoration field. 
  Unused bits may be used by end user code.
  These values are SVG specific.  Other applications could use the text
  decoration field for a different set of bits, so long as it provided its own
  output function.
  @{
*/
#define TXTDECOR_NONE     0x000  /**< text is not decorated (default) */
#define TXTDECOR_UNDER    0x001  /**< underlined                      */
#define TXTDECOR_OVER     0x002  /**< overlined                       */
#define TXTDECOR_BLINK    0x004  /**< blinking text                   */
#define TXTDECOR_STRIKE   0x008  /**< strike through                  */
#define TXTDECOR_TMASK    0x00F  /**< Mask for selecting bits above   */

#define TXTDECOR_SOLID    0x000  /**< draw as single solid line       */
#define TXTDECOR_DOUBLE   0x010  /**< draw as double solid line       */
#define TXTDECOR_DOTTED   0x020  /**< draw as single dotted line      */
#define TXTDECOR_DASHED   0x040  /**< draw as single dashed line      */
#define TXTDECOR_WAVY     0x080  /**< draw as single wavy line        */
#define TXTDECOR_LMASK    0x0F0  /**< Mask for selecting these bits   */

#define TXTDECOR_CLRSET   0x100  /**< decoration has its own color    */

/** @} */





/** \defgroup text alignment types
  Location of text's {X,Y} coordinate on bounding rectangle.
  Values are compatible with Fontconfig.
  @{
*/
#define ALILEFT    0x01     /**< text object horizontal alignment = left     */
#define ALICENTER  0x02     /**< text object horizontal alignment = center   */
#define ALIRIGHT   0x04     /**< text object horizontal alignment = right    */
#define ALIHORI    0x07     /**< text object horizontal alignment mask       */
#define ALITOP     0x08     /**< text object vertical   alignment = top      */
#define ALIBASE    0x10     /**< text object vertical   alignment = baseline */
#define ALIBOT     0x20     /**< text object vertical   alignment = bottom   */
#define ALIVERT    0x38     /**< text object vertical   alignment mask       */
/** @} */

/** \defgroup language direction types
  @{
*/
#define LDIR_LR    0x00     /**< left to right */ 
#define LDIR_RL    0x01     /**< right to left */ 
#define LDIR_TB    0x02     /**< top to bottom */ 
/** @} */

/** \defgroup special processing flags
  @{
*/
#define TR_EMFBOT  0x01     /**< use an approximation compatible with EMF file's "BOTTOM" text orientation, which is not the "bottom" for Freetype fonts */ 
/** @} */

/** \enum tr_classes
classification of complexes
  @{
*/
enum tr_classes {
      TR_TEXT,              /**< simple text object                                                   */
      TR_LINE,              /**< linear assembly of TR_TEXTs                                          */
      TR_PARA_UJ,           /**< sequential assembly of TR_LINEs and TR_TEXTs into a paragraph -      
                                     unknown justification properties                                 */
      TR_PARA_LJ,           /**< ditto, left justified                                                */
      TR_PARA_CJ,           /**< ditto, center justified                                              */
      TR_PARA_RJ            /**< ditto, right justified                                               */
   };
/** @} */

/**
   \brief alt font entries.
*/
typedef struct {
   uint32_t    fi_idx;      /**< index into FT_INFO fonts, for fonts added for missing glyphs         */
   uint32_t    weight;      /**< integer weight for alt fonts, kept sorted into descending order      */
} ALT_SPECS;

/**
   \brief Information for a font instance.
*/
typedef struct {
   FcFontSet  *fontset;     /**< all matching fonts (for fallback on missing glyphs)                  */
   ALT_SPECS  *alts;        /**< index into FT_INFO fonts, for fonts added for missing glyphs         */
   uint32_t    space;       /**< alts storage slots allocated                                         */
   uint32_t    used;        /**< alts storage slots in use                                            */
   FT_Face     face;        /**< font face structures (FT_FACE is a pointer!)                         */
   uint8_t    *file;        /**< pointer to font paths to files                                       */
   uint8_t    *fontspec;    /**< pointer to a font specification (name:italics, etc.)                 */
   FcPattern  *fpat;        /**< current font, must hang onto this or faces operations break          */
   double      spcadv;      /**< advance equal to a space, in points at font's face size              */
   double      fsize;       /**< font's face size in points                                           */
} FNT_SPECS;

/**
   \brief Information for all font instances.
*/
typedef struct {
   FT_Library  library;     /**< Fontconfig handle                                                    */
   FNT_SPECS  *fonts;       /**< Array of fontinfo structures                                         */
   uint32_t    space;       /**< storage slots allocated                                              */
   uint32_t    used;        /**< storage slots in use                                                 */
} FT_INFO;

typedef struct {
    uint8_t             Red;                //!< Red   color (0-255)
    uint8_t             Green;              //!< Green color (0-255)
    uint8_t             Blue;               //!< Blue  color (0-255) 
    uint8_t             Reserved;           //!< Not used
} TRCOLORREF;

/**
   \brief Information for a single text object
*/
typedef struct {
   uint8_t    *string;      /**< UTF-8 text                                                           */
   double      ori;         /**< Orientation, angle of characters with respect to baseline in degrees */
   double      fs;          /**< font size of text                                                    */
   double      x;           /**< x coordinate, relative to TR_INFO x,y, in points                     */
   double      y;           /**< y coordinate, relative to TR_INFO x,y, in points                     */
   double      xkern;       /**< x kern relative to preceding text chunk in complex (if any)          */
   double      ykern;       /**< y kern relative to preceding text chunk in complex (if any)          */
   double      boff;        /**< Y LL corner - boff finds baseline                                    */
   double      vadvance;    /**< Line spacing typically 1.25 or 1.2,  only set on the first text
                                 element in a complex                                                 */
   TRCOLORREF  color;       /**< RGB                                                                  */
   int         taln;        /**< text alignment with respect to x,y                                   */
   int         ldir;        /**< language diretion LDIR_*                                             */
   int         italics;     /**< italics, as in FontConfig                                            */
   int         weight;      /**< weight, as in FontConfig                                             */
   int         condensed;   /**< condensed, as in FontConfig                                          */
   int         decoration;  /**< text decorations, ignored during assembly, used during output        */
   int         spaces;      /**< count of spaces converted from wide kerning (1 or 2)                 */
   TRCOLORREF  decColor;    /**< text decoration color, ignored during assembly, used during output   */
   int         co;          /**< condensed override, if set Font name included narrow                 */
   int         rt_tidx;     /**< index of rectangle that contains it                                  */
   int         fi_idx;      /**< index of the font it uses                                            */
} TCHUNK_SPECS;

/**
   \brief Information for all text objects.
   Coordinates here are INTERNAL, after offset/rotate using values in TR_INFO.
*/
typedef struct {
   TCHUNK_SPECS *chunks;     /**< text chunks                                                         */
   uint32_t      space;      /**< storage slots allocated                                             */
   uint32_t      used;       /**< storage slots in use                                                */
} TP_INFO;

/**
   \brief Information for a single bounding rectangle.
   Coordinates here are INTERNAL, after offset/rotate using values in TR_INFO.
*/
typedef struct {
   double      xll;           /**< x rectangle lower left corner                                      */
   double      yll;           /**< y "                                                                */
   double      xur;           /**< x upper right corner                                               */
   double      yur;           /**< y "                                                                */
   double      xbearing;      /**< x bearing of the leftmost character                                */
} BRECT_SPECS;

/**
   \brief Information for all bounding rectangles.
*/
typedef struct {
   BRECT_SPECS *rects;         /**< bounding rectangles                                               */
   uint32_t     space;         /**< storage slots allocated                                           */
   uint32_t     used;          /**< storage slots in use                                              */
} BR_INFO;

/**
   \brief List of all members of a single complex.
*/
typedef struct {
   int        *members;        /**< array of immediate children (for TR_PARA_* these are indicies 
                                    for TR_TEXT or TR_LINE complexes also in cxi. For TR_TEXT
                                    and TR_LINE these are indices to the actual text in tpi.)         */
   uint32_t    space;          /**< storage slots allocated                                           */
   uint32_t    used;           /**< storage slots in use                                              */
} CHILD_SPECS;

/**
   \brief Information for a single complex. 
*/
typedef struct {
   int               rt_cidx;  /**< index of rectangle that contains all members                      */
   enum tr_classes   type;     /**< classification of the complex                                     */
   CHILD_SPECS       kids;     /**< immediate child nodes of this complex, for type TR_TEXT the
                                    idx refers to the tpi data. otherwise, cxi data                   */
} CX_SPECS;

/**
   \brief Information for all complexes.
*/
typedef struct {
   CX_SPECS   *cx;             /**< complexes                                                         */
   uint32_t    space;          /**< storage slots allocated                                           */
   uint32_t    used;           /**< storage slots in use                                              */
   uint32_t    phase1;         /**< Number of complexes (lines + text fragments) entered in phase 1   */
   uint32_t    lines;          /**< Number of lines in phase 1                                        */
   uint32_t    paras;          /**< Number of complexes (paras) entered in phase 2                    */
} CX_INFO;

/**
   \brief Information for the entire text reassembly system.
*/
typedef struct {
   FT_INFO    *fti;            /**< Font info storage                                                 */
   TP_INFO    *tpi;            /**< Text Info/Position Info storage                                   */
   BR_INFO    *bri;            /**< Bounding Rectangle Info storage                                   */
   CX_INFO    *cxi;            /**< Complex Info storage                                              */
   uint8_t    *out;            /**< buffer to hold formatted output                                   */
   double      qe;             /**< quantization error in points.                                     */
   double      esc;            /**< escapement angle in DEGREES                                       */
   double      x;              /**< x coordinate of first text object, in points                      */
   double      y;              /**< y coordinate of first text object, in points                      */
   int         dirty;          /**< 1 if text records are loaded                                      */ 
   int         use_kern;       /**< 1 if kerning is used, 0 if not                                    */
   int         load_flags;     /**< FT_LOAD_NO_SCALE or FT_LOAD_TARGET_NORMAL                         */
   int         kern_mode;      /**< FT_KERNING_DEFAULT, FT_KERNING_UNFITTED, or FT_KERNING_UNSCALED   */
   uint32_t    outspace;       /**< storage in output buffer  allocated                               */
   uint32_t    outused;        /**< storage in output buffer in use                                   */
   int         usebk;          /**< On output write the background color under the text               */
   TRCOLORREF  bkcolor;        /**< RGB background color                                              */
} TR_INFO;

/* padding added to rectangles before overlap test */
/**
   \brief Information for one padding record.  (Padding is added to bounding rectangles before overlap tests.)
*/
typedef struct {
   double up;                  /**< to top                                                            */
   double down;                /**< to bottom                                                         */
   double left;                /**< to left                                                           */
   double right;               /**< to right                                                          */
} RT_PAD;

/** \cond  */
/*
   iconv() has a funny cast on some older systems, on most recent ones
   it is just char **.  This tries to work around the issue.  If you build this
   on another funky system this code may need to be modified, or define ICONV_CAST
   on the compile line(but it may be tricky).
*/
#ifdef SOL8
#define ICONV_CAST (const char **)
#endif  //SOL8
#if !defined(ICONV_CAST)
#define ICONV_CAST (char **)
#endif  //ICONV_CAST
/** \endcond */

/* Prototypes */
int           TR_findcasesub(const char *string, const char *sub);
char         *TR_construct_fontspec(const TCHUNK_SPECS *tsp, const char *fontname);
char         *TR_reconstruct_fontspec(const char *fontspec, const  char *fontname);
int           TR_find_alternate_font(FT_INFO *fti, FNT_SPECS **efsp, uint32_t wc);
int           TR_getadvance(FT_INFO *fti, FNT_SPECS *fsp, uint32_t wc, uint32_t pc, int load_flags, int kern_mode, int *ymin, int *ymax);
int           TR_getkern2(FNT_SPECS *fsp, uint32_t wc, uint32_t pc, int kern_mode);
int           TR_kern_gap(FNT_SPECS *fsp, TCHUNK_SPECS *tsp, TCHUNK_SPECS *ptsp, int kern_mode);
void          TR_rt_pad_set(RT_PAD *rt_pad, double up, double down, double left, double right);
double        TR_baseline(TR_INFO *tri, int src, double *AscMax, double *DscMax);
int           TR_check_set_vadvance(TR_INFO *tri, int src, int lines);
int           TR_layout_analyze(TR_INFO *tri);
void          TR_layout_2_svg(TR_INFO *tri);
int           TR_weight_FC_to_SVG(int weight);

FT_INFO      *ftinfo_init(void);
int           ftinfo_make_insertable(FT_INFO *fti);
int           ftinfo_insert(FT_INFO *fti, FNT_SPECS *fsp);
FT_INFO      *ftinfo_release(FT_INFO *fti);
FT_INFO      *ftinfo_clear(FT_INFO *fti);
int           ftinfo_find_loaded_by_spec(const FT_INFO *fti, const uint8_t *fname);
int           ftinfo_find_loaded_by_src(const FT_INFO *fti, const uint8_t *filename);
int           ftinfo_load_fontname(FT_INFO *fti, const char *fontspec);
void          ftinfo_dump(const FT_INFO *fti);

int           fsp_alts_make_insertable(FNT_SPECS *fsp);
int           fsp_alts_insert(FNT_SPECS *fsp, uint32_t fi_idx);
int           fsp_alts_weight(FNT_SPECS *fsp, uint32_t a_idx);

int           csp_make_insertable(CHILD_SPECS *csp);
int           csp_insert(CHILD_SPECS *csp, int src);
int           csp_merge(CHILD_SPECS *dst, CHILD_SPECS *src);
void          csp_release(CHILD_SPECS *csp);
void          csp_clear(CHILD_SPECS *csp);

CX_INFO      *cxinfo_init(void);
int           cxinfo_make_insertable(CX_INFO *cxi);
int           cxinfo_insert(CX_INFO *cxi, int src, int src_rt_idx, enum tr_classes type);
int           cxinfo_append(CX_INFO *cxi, int src, enum tr_classes type);
int           cxinfo_merge(CX_INFO *cxi,  int dst, int src, enum tr_classes type);
int           cxinfo_trim(CX_INFO *cxi);
CX_INFO      *cxinfo_release(CX_INFO *cxi);
void          cxinfo_dump(const TR_INFO *tri);

TP_INFO      *tpinfo_init(void);
int           tpinfo_make_insertable(TP_INFO *tpi);
int           tpinfo_insert(TP_INFO *tpi, const TCHUNK_SPECS *tsp);
TP_INFO      *tpinfo_release(TP_INFO *tpi);

BR_INFO      *brinfo_init(void);
int           brinfo_make_insertable(BR_INFO *bri);
int           brinfo_insert(BR_INFO *bri, const BRECT_SPECS *element);
int           brinfo_merge(BR_INFO *bri, int dst, int src);
enum tr_classes 
              brinfo_pp_alignment(const BR_INFO *bri, int dst, int src, double slop, enum tr_classes type);
int           brinfo_overlap(const BR_INFO *bri, int dst, int src, RT_PAD *rp_dst, RT_PAD *rp_src);
BR_INFO      *brinfo_release(BR_INFO *bri);

TR_INFO      *trinfo_init(TR_INFO *tri);
TR_INFO      *trinfo_release(TR_INFO *tri);
TR_INFO      *trinfo_release_except_FC(TR_INFO *tri);
TR_INFO      *trinfo_clear(TR_INFO *tri);
int           trinfo_load_qe(TR_INFO *tri, double qe);
int           trinfo_load_bk(TR_INFO *tri, int usebk, TRCOLORREF bkcolor);
int           trinfo_load_ft_opts(TR_INFO *tri, int use_kern, int load_flags, int kern_mode);
int           trinfo_load_textrec(TR_INFO *tri, const TCHUNK_SPECS *tsp, double escapement, int flags);  
int           trinfo_check_bk(TR_INFO *tri, int usebk, TRCOLORREF bkcolor);
int           trinfo_append_out(TR_INFO *tri, const char *src);

int           is_mn_unicode(int test);


#ifdef __cplusplus
}
#endif
#endif /* _TEXT_REASSEMBLE_ */
