/**
  @file uemf.h Structures and functions prototypes for EMF files.

  EMF file Record structure information has been derived from Mingw, Wine, and libEMF header files, and from
  Microsoft's EMF Information pdf, release date March 28,2012, link from here:
  
     http://msdn2.microsoft.com/en-us/library/cc230514.aspx
  
  If the direct link fails the document may be found
  by searching for: "[MS-EMF]: Enhanced Metafile Format"
  
*/

/*
File:      uemf.h
Version:   0.0.19
Date:      20-FEB-2013
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2013 David Mathog and California Institute of Technology (Caltech)
*/

#ifndef _UEMF_
#define _UEMF_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "uemf_utf.h"
#include "uemf_endian.h"



//  ***********************************************************************************
//  defines not placed yet

#define U_PAN_CULTURE_LATIN              0

#define U_SYSPAL_ERROR    0
#define U_SYSPAL_STATIC   1
#define U_SYSPAL_NOSTATIC 2

#define U_ELF_VENDOR_SIZE 4


//  ***********************************************************************************
//  Value enumerations and other predefined constants, alphabetical order by group


/** \defgroup Font_struct_widths Font name and style widths in characters
  For U_LOGFONT and U_LOGFONT_PANOSE, 
  @{
*/
#define U_LF_FACESIZE     32    //!< U_LOGFONT lfFaceName and U_LOGFONT_PANOSE elfStyle fields maximum width
#define U_LF_FULLFACESIZE 64    //!< U_LOGFONT_PANOSE elfFullName field maximum width
/** @} */

/** \defgroup U_EMR_Qualifiers RecordType Enumeration
  (RecordType Enumeration)
  For U_EMR iType field
  @{
*/
#define U_EMR_HEADER                   1 //!< U_EMRHEADER                  record
#define U_EMR_POLYBEZIER               2 //!< U_EMRPOLYBEZIER              record
#define U_EMR_POLYGON                  3 //!< U_EMRPOLYGON                 record
#define U_EMR_POLYLINE                 4 //!< U_EMRPOLYLINE                record
#define U_EMR_POLYBEZIERTO             5 //!< U_EMRPOLYBEZIERTO            record
#define U_EMR_POLYLINETO               6 //!< U_EMRPOLYLINETO              record
#define U_EMR_POLYPOLYLINE             7 //!< U_EMRPOLYPOLYLINE            record
#define U_EMR_POLYPOLYGON              8 //!< U_EMRPOLYPOLYGON             record
#define U_EMR_SETWINDOWEXTEX           9 //!< U_EMRSETWINDOWEXTEX          record
#define U_EMR_SETWINDOWORGEX          10 //!< U_EMRSETWINDOWORGEX          record
#define U_EMR_SETVIEWPORTEXTEX        11 //!< U_EMRSETVIEWPORTEXTEX        record
#define U_EMR_SETVIEWPORTORGEX        12 //!< U_EMRSETVIEWPORTORGEX        record
#define U_EMR_SETBRUSHORGEX           13 //!< U_EMRSETBRUSHORGEX           record
#define U_EMR_EOF                     14 //!< U_EMREOF                     record
#define U_EMR_SETPIXELV               15 //!< U_EMRSETPIXELV               record
#define U_EMR_SETMAPPERFLAGS          16 //!< U_EMRSETMAPPERFLAGS          record
#define U_EMR_SETMAPMODE              17 //!< U_EMRSETMAPMODE              record
#define U_EMR_SETBKMODE               18 //!< U_EMRSETBKMODE               record
#define U_EMR_SETPOLYFILLMODE         19 //!< U_EMRSETPOLYFILLMODE         record
#define U_EMR_SETROP2                 20 //!< U_EMRSETROP2                 record
#define U_EMR_SETSTRETCHBLTMODE       21 //!< U_EMRSETSTRETCHBLTMODE       record
#define U_EMR_SETTEXTALIGN            22 //!< U_EMRSETTEXTALIGN            record
#define U_EMR_SETCOLORADJUSTMENT      23 //!< U_EMRSETCOLORADJUSTMENT      record
#define U_EMR_SETTEXTCOLOR            24 //!< U_EMRSETTEXTCOLOR            record
#define U_EMR_SETBKCOLOR              25 //!< U_EMRSETBKCOLOR              record
#define U_EMR_OFFSETCLIPRGN           26 //!< U_EMROFFSETCLIPRGN           record
#define U_EMR_MOVETOEX                27 //!< U_EMRMOVETOEX                record
#define U_EMR_SETMETARGN              28 //!< U_EMRSETMETARGN              record
#define U_EMR_EXCLUDECLIPRECT         29 //!< U_EMREXCLUDECLIPRECT         record
#define U_EMR_INTERSECTCLIPRECT       30 //!< U_EMRINTERSECTCLIPRECT       record
#define U_EMR_SCALEVIEWPORTEXTEX      31 //!< U_EMRSCALEVIEWPORTEXTEX      record
#define U_EMR_SCALEWINDOWEXTEX        32 //!< U_EMRSCALEWINDOWEXTEX        record
#define U_EMR_SAVEDC                  33 //!< U_EMRSAVEDC                  record
#define U_EMR_RESTOREDC               34 //!< U_EMRRESTOREDC               record
#define U_EMR_SETWORLDTRANSFORM       35 //!< U_EMRSETWORLDTRANSFORM       record
#define U_EMR_MODIFYWORLDTRANSFORM    36 //!< U_EMRMODIFYWORLDTRANSFORM    record
#define U_EMR_SELECTOBJECT            37 //!< U_EMRSELECTOBJECT            record
#define U_EMR_CREATEPEN               38 //!< U_EMRCREATEPEN               record
#define U_EMR_CREATEBRUSHINDIRECT     39 //!< U_EMRCREATEBRUSHINDIRECT     record
#define U_EMR_DELETEOBJECT            40 //!< U_EMRDELETEOBJECT            record
#define U_EMR_ANGLEARC                41 //!< U_EMRANGLEARC                record
#define U_EMR_ELLIPSE                 42 //!< U_EMRELLIPSE                 record
#define U_EMR_RECTANGLE               43 //!< U_EMRRECTANGLE               record
#define U_EMR_ROUNDRECT               44 //!< U_EMRROUNDRECT               record
#define U_EMR_ARC                     45 //!< U_EMRARC                     record
#define U_EMR_CHORD                   46 //!< U_EMRCHORD                   record
#define U_EMR_PIE                     47 //!< U_EMRPIE                     record
#define U_EMR_SELECTPALETTE           48 //!< U_EMRSELECTPALETTE           record
#define U_EMR_CREATEPALETTE           49 //!< U_EMRCREATEPALETTE           record
#define U_EMR_SETPALETTEENTRIES       50 //!< U_EMRSETPALETTEENTRIES       record
#define U_EMR_RESIZEPALETTE           51 //!< U_EMRRESIZEPALETTE           record
#define U_EMR_REALIZEPALETTE          52 //!< U_EMRREALIZEPALETTE          record
#define U_EMR_EXTFLOODFILL            53 //!< U_EMREXTFLOODFILL            record
#define U_EMR_LINETO                  54 //!< U_EMRLINETO                  record
#define U_EMR_ARCTO                   55 //!< U_EMRARCTO                   record
#define U_EMR_POLYDRAW                56 //!< U_EMRPOLYDRAW                record
#define U_EMR_SETARCDIRECTION         57 //!< U_EMRSETARCDIRECTION         record
#define U_EMR_SETMITERLIMIT           58 //!< U_EMRSETMITERLIMIT           record
#define U_EMR_BEGINPATH               59 //!< U_EMRBEGINPATH               record
#define U_EMR_ENDPATH                 60 //!< U_EMRENDPATH                 record
#define U_EMR_CLOSEFIGURE             61 //!< U_EMRCLOSEFIGURE             record
#define U_EMR_FILLPATH                62 //!< U_EMRFILLPATH                record
#define U_EMR_STROKEANDFILLPATH       63 //!< U_EMRSTROKEANDFILLPATH       record
#define U_EMR_STROKEPATH              64 //!< U_EMRSTROKEPATH              record
#define U_EMR_FLATTENPATH             65 //!< U_EMRFLATTENPATH             record
#define U_EMR_WIDENPATH               66 //!< U_EMRWIDENPATH               record
#define U_EMR_SELECTCLIPPATH          67 //!< U_EMRSELECTCLIPPATH          record
#define U_EMR_ABORTPATH               68 //!< U_EMRABORTPATH               record
#define U_EMR_UNDEF69                 69 //!< U_EMRUNDEF69                 record
#define U_EMR_COMMENT                 70 //!< U_EMRCOMMENT                 record
#define U_EMR_FILLRGN                 71 //!< U_EMRFILLRGN                 record
#define U_EMR_FRAMERGN                72 //!< U_EMRFRAMERGN                record
#define U_EMR_INVERTRGN               73 //!< U_EMRINVERTRGN               record
#define U_EMR_PAINTRGN                74 //!< U_EMRPAINTRGN                record
#define U_EMR_EXTSELECTCLIPRGN        75 //!< U_EMREXTSELECTCLIPRGN        record
#define U_EMR_BITBLT                  76 //!< U_EMRBITBLT                  record
#define U_EMR_STRETCHBLT              77 //!< U_EMRSTRETCHBLT              record
#define U_EMR_MASKBLT                 78 //!< U_EMRMASKBLT                 record
#define U_EMR_PLGBLT                  79 //!< U_EMRPLGBLT                  record
#define U_EMR_SETDIBITSTODEVICE       80 //!< U_EMRSETDIBITSTODEVICE       record
#define U_EMR_STRETCHDIBITS           81 //!< U_EMRSTRETCHDIBITS           record
#define U_EMR_EXTCREATEFONTINDIRECTW  82 //!< U_EMREXTCREATEFONTINDIRECTW  record
#define U_EMR_EXTTEXTOUTA             83 //!< U_EMREXTTEXTOUTA             record
#define U_EMR_EXTTEXTOUTW             84 //!< U_EMREXTTEXTOUTW             record
#define U_EMR_POLYBEZIER16            85 //!< U_EMRPOLYBEZIER16            record
#define U_EMR_POLYGON16               86 //!< U_EMRPOLYGON16               record
#define U_EMR_POLYLINE16              87 //!< U_EMRPOLYLINE16              record
#define U_EMR_POLYBEZIERTO16          88 //!< U_EMRPOLYBEZIERTO16          record
#define U_EMR_POLYLINETO16            89 //!< U_EMRPOLYLINETO16            record
#define U_EMR_POLYPOLYLINE16          90 //!< U_EMRPOLYPOLYLINE16          record
#define U_EMR_POLYPOLYGON16           91 //!< U_EMRPOLYPOLYGON16           record
#define U_EMR_POLYDRAW16              92 //!< U_EMRPOLYDRAW16              record
#define U_EMR_CREATEMONOBRUSH         93 //!< U_EMRCREATEMONOBRUSH         record
#define U_EMR_CREATEDIBPATTERNBRUSHPT 94 //!< U_EMRCREATEDIBPATTERNBRUSHPT record
#define U_EMR_EXTCREATEPEN            95 //!< U_EMREXTCREATEPEN            record
#define U_EMR_POLYTEXTOUTA            96 //!< U_EMRPOLYTEXTOUTA            record
#define U_EMR_POLYTEXTOUTW            97 //!< U_EMRPOLYTEXTOUTW            record
#define U_EMR_SETICMMODE              98 //!< U_EMRSETICMMODE              record
#define U_EMR_CREATECOLORSPACE        99 //!< U_EMRCREATECOLORSPACE        record
#define U_EMR_SETCOLORSPACE          100 //!< U_EMRSETCOLORSPACE           record
#define U_EMR_DELETECOLORSPACE       101 //!< U_EMRDELETECOLORSPACE        record
#define U_EMR_GLSRECORD              102 //!< U_EMRGLSRECORD               record
#define U_EMR_GLSBOUNDEDRECORD       103 //!< U_EMRGLSBOUNDEDRECORD        record
#define U_EMR_PIXELFORMAT            104 //!< U_EMRPIXELFORMAT             record
#define U_EMR_DRAWESCAPE             105 //!< U_EMRDRAWESCAPE              record
#define U_EMR_EXTESCAPE              106 //!< U_EMREXTESCAPE               record
#define U_EMR_UNDEF107               107 //!< U_EMRUNDEF107                record
#define U_EMR_SMALLTEXTOUT           108 //!< U_EMRSMALLTEXTOUT            record
#define U_EMR_FORCEUFIMAPPING        109 //!< U_EMRFORCEUFIMAPPING         record
#define U_EMR_NAMEDESCAPE            110 //!< U_EMRNAMEDESCAPE             record
#define U_EMR_COLORCORRECTPALETTE    111 //!< U_EMRCOLORCORRECTPALETTE     record
#define U_EMR_SETICMPROFILEA         112 //!< U_EMRSETICMPROFILEA          record
#define U_EMR_SETICMPROFILEW         113 //!< U_EMRSETICMPROFILEW          record
#define U_EMR_ALPHABLEND             114 //!< U_EMRALPHABLEND              record
#define U_EMR_SETLAYOUT              115 //!< U_EMRSETLAYOUT               record
#define U_EMR_TRANSPARENTBLT         116 //!< U_EMRTRANSPARENTBLT          record
#define U_EMR_UNDEF117               117 //!< U_EMRUNDEF117                record
#define U_EMR_GRADIENTFILL           118 //!< U_EMRGRADIENTFILL            record
#define U_EMR_SETLINKEDUFIS          119 //!< U_EMRSETLINKEDUFIS           record
#define U_EMR_SETTEXTJUSTIFICATION   120 //!< U_EMRSETTEXTJUSTIFICATION    record
#define U_EMR_COLORMATCHTOTARGETW    121 //!< U_EMRCOLORMATCHTOTARGETW     record
#define U_EMR_CREATECOLORSPACEW      122 //!< U_EMRCREATECOLORSPACEW       record

#define U_EMR_MIN 1                      //!< Minimum U_EMR_ value.
#define U_EMR_MAX 122                    //!< Maximum U_EMR_ value.  Not much beyond 104 is implemented

#define U_EMR_INVALID         0xFFFFFFFF //!< Not any valid U_EMF_ value
/** @} */

/** \defgroup U_DRAW_PROPERTIES draw properties
  Used in emr_properties() and wmr_properties.  These are the bit definitions.
  @{
*/
#define U_DRAW_NOTEMPTY   0x001           //!< Path has at least a MOVETO in it
#define U_DRAW_VISIBLE    0x002           //!< Path has at least a LINE in it
#define U_DRAW_CLOSED     0x004           //!< Path has been closed
#define U_DRAW_ONLYTO     0x008           //!< Path so far contains only *TO operations
#define U_DRAW_FORCE      0x010           //!< Path MUST be drawn
#define U_DRAW_ALTERS     0x020           //!< Alters draw parameters (pen, brush, coordinates...)
#define U_DRAW_PATH       0x040           //!< An explicit path is being used (with a BEGIN and END)
#define U_DRAW_TEXT       0x080           //!< Current record forces all pending text to be drawn first.
#define U_DRAW_OBJECT     0x100           //!< Creates an Object (only used in WMF)
#define U_DRAW_NOFILL     0x200           //!< Object is not fillable (lines and arc, only used in WMF)

/** @} */
/** \defgroup U_EMRSETARCDIRECTION_Qualifiers  ArcDirection Enumeration
  For U_EMRSETARCDIRECTION iArcDirection field 
  @{
*/
#define U_AD_COUNTERCLOCKWISE 1
#define U_AD_CLOCKWISE        2
/** @} */

/** \defgroup U_PANOSE_bArmStyle_Qualifiers ArmStyle Enumeration
  For U_PANOSE bArmStyle field
  @{
*/
#define U_PAN_STRAIGHT_ARMS_HORZ         2
#define U_PAN_STRAIGHT_ARMS_WEDGE        3
#define U_PAN_STRAIGHT_ARMS_VERT         4
#define U_PAN_STRAIGHT_ARMS_SINGLE_SERIF 5
#define U_PAN_STRAIGHT_ARMS_DOUBLE_SERIF 6
#define U_PAN_BENT_ARMS_HORZ             7
#define U_PAN_BENT_ARMS_WEDGE            8
#define U_PAN_BENT_ARMS_VERT             9
#define U_PAN_BENT_ARMS_SINGLE_SERIF     10
#define U_PAN_BENT_ARMS_DOUBLE_SERIF     11
/** @} */

/** \defgroup U_EMRSETBKMODE_iMode_Qualifiers BackgroundMode enumeration
  For U_EMRSETBKMODE iMode field
  @{
*/
#define U_TRANSPARENT  1
#define U_OPAQUE       2
/** @} */

/** \defgroup U_BITMAPINFOHEADER_biBitCount_Qualifiers BitCount Enumeration
  For U_BITMAPINFOHEADER biBitCount field.
  @{
*/
#define U_BCBM_EXPLICIT   0  //!< Derived from JPG or PNG compressed image or ?
#define U_BCBM_MONOCHROME 1  //!< 2 colors.    bmiColors array has two entries
#define U_BCBM_COLOR4     4  //!< 2^4 colors.  bmiColors array has 16 entries
#define U_BCBM_COLOR8     8  //!< 2^8 colors.  bmiColors array has 256 entries
#define U_BCBM_COLOR16   16  //!< 2^16 colors. bmiColors is not used. Pixels are 5 bits B,G,R with 1 unused bit
#define U_BCBM_COLOR24   24  //!< 2^24 colors. bmiColors is not used. Pixels are U_RGBTRIPLE.
#define U_BCBM_COLOR32   32  //!< 2^32 colors. bmiColors is not used. Pixels are U_RGBQUAD.
/** @} */

/** \defgroup U_BITMAPINFOHEADER_biCompression_Qualifiers BI_Compression Enumeration
  For U_BITMAPINFOHEADER biCompression field
  @{
*/
#define U_BI_RGB           0  //!< This is the only one supported by UEMF at present
#define U_BI_RLE8          1
#define U_BI_RLE4          2
#define U_BI_BITFIELDS     3
#define U_BI_JPEG          4
#define U_BI_PNG           5
/** @} */

/** \defgroup U_COLORADJUSTMENT_caFlags_Qualifiers ColorAdjustment Enumeration
  For U_COLORADJUSTMENT caFlags field
  @{
*/
#define U_CA_NEGATIVE          0x0001
#define U_CA_LOG_FILTER        0x0002
/** @} */

/** \defgroup U_EMRCOLORMATCHTOTARGETW_dwFlags_Qualifiers ColorMatchToTarget Enumeration
  For U_EMRCOLORMATCHTOTARGETW dwFlags field
  @{
*/
#define U_COLORMATCHTOTARGET_NOTEMBEDDED       0
#define U_COLORMATCHTOTARGET_EMBEDDED          1
/** @} */

/** \defgroup U_EMRCOLORMATCHTOTARGETW_dwAction_Qualifiers ColorSpace Enumeration
  For U_EMRCOLORMATCHTOTARGETW dwAction field 
  @{
*/
#define U_CS_ENABLE           1
#define U_CS_DISABLE          2
#define U_CS_DELETE_TRANSFORM  3
/** @} */

/** \defgroup U_PANOSE_bContrast_Qualifiers Contrast Enumeration
  For U_PANOSE bContrast field
  @{
*/
#define U_PAN_CONTRAST_NONE              2
#define U_PAN_CONTRAST_VERY_LOW          3
#define U_PAN_CONTRAST_LOW               4
#define U_PAN_CONTRAST_MEDIUM_LOW        5
#define U_PAN_CONTRAST_MEDIUM            6
#define U_PAN_CONTRAST_MEDIUM_HIGH       7
#define U_PAN_CONTRAST_HIGH              8
#define U_PAN_CONTRAST_VERY_HIGH         9
/** @} */

/** \defgroup U_DIBITS_iUsageSrc_Qualifiers DIBColors Enumeration
  For U_EMRSETDIBITSTODEIVCE and U_EMRSTRETCHDIBITS iUsageSrc fields.
  @{
*/
#define U_DIB_RGB_COLORS   0
#define U_DIB_PAL_COLORS   1
/** @} */

/** \defgroup U_EMRCOMMENT_TYPES Comment record types
  For U_EMRCOMMENT_* cIdent fields
  @{
*/
#define U_EMR_COMMENT_PUBLIC            0x43494447
#define U_EMR_COMMENT_SPOOL             0x00000000
#define U_EMR_COMMENT_SPOOLFONTDEF      0x544F4E46
#define U_EMR_COMMENT_EMFPLUSRECORD     0x2B464D45
/** @} */

/** \defgroup U_EMR_COMMENT_PUBLIC EMRComment Enumeration
  For U_EMRCOMMENT_PUBLIC pcIdent fields
  @{
*/
#define U_EMR_COMMENT_WINDOWS_METAFILE  0x80000001
#define U_EMR_COMMENT_BEGINGROUP        0x00000002
#define U_EMR_COMMENT_ENDGROUP          0x00000003
#define U_EMR_COMMENT_MULTIFORMATS      0x40000004
#define U_EMR_COMMENT_UNICODE_STRING    0x00000040
#define U_EMR_COMMENT_UNICODE_END       0x00000080
/** @} */

/** \defgroup U_EMRTEXT_foptions_Qualifiers ExtTextOutOptions Enumeration
  For U_EMRTEXT foptions field
  @{
*/
#define U_ETO_NONE              0x00000000
#define U_ETO_GRAYED            0x00000001
#define U_ETO_OPAQUE            0x00000002
#define U_ETO_CLIPPED           0x00000004
#define U_ETO_GLYPH_INDEX       0x00000010
#define U_ETO_RTLREADING        0x00000080
#define U_ETO_NO_RECT           0x00000100
#define U_ETO_SMALL_CHARS       0x00000200  // For EMRSMALLTEXTOUT ONLY, does not affect EMRTEXTOUTA or EMRTEXTOUTW
#define U_ETO_NUMERICSLOCAL     0x00000400
#define U_ETO_NUMERICSLATIN     0x00000800
#define U_ETO_IGNORELANGUAGE    0x00001000
#define U_ETO_PDY               0x00002000
#define U_ETO_REVERSE_INDEX_MAP 0x00010000
/** @} */

/** \defgroup U_PANOSE_bFamilyType_Qualifiers FamilyType Enumeration
  For U_PANOSE bFamilyType field
  @{
*/
#define U_PAN_FAMILY_TEXT_DISPLAY        2
#define U_PAN_FAMILY_SCRIPT              3
#define U_PAN_FAMILY_DECORATIVE          4
#define U_PAN_FAMILY_PICTORIAL           5
/** @} */

/** \defgroup U_EMREXTFLOODFILL_iMode_Qualifiers FloodFill Enumeration
  For U_EMREXTFLOODFILL iMode field
  @{
*/
#define U_FLOODFILLBORDER   0x00000000  /* Color specified must be the same as the border - brush fill stops at this color */
#define U_FLOODFILLSURFACE  0x00000001  /* Color specified must be different from the border - brush fills only this color  */
/** @} */

/** \defgroup U_DESIGNVECTOR_Signature_Qualifiers Signature Enumeration
  For U_DESIGNVECTOR Signature field
  @{
*/
#define U_ENHMETA_SIGNATURE 0x464D4520      //!< also for U_EMRHEADER dSignature field
#define U_EPS_SIGNATURE     0x46535045
/** @} */

/** \defgroup U_EMRGRADIENTFILL_ulMode_Qualifiers GradientFill Enumeration
  For U_EMRGRADIENTFILL ulMode field
  @{
*/
#define U_GRADIENT_FILL_RECT_H     0x00000000
#define U_GRADIENT_FILL_RECT_V     0x00000001
#define U_GRADIENT_FILL_TRIANGLE   0x00000002
/** @} */

/** \defgroup U_EMREXTTEXTOUT_iGraphicsMode_Qualifiers GraphicsMode Enumeration
  For U_EMREXTTEXTOUTA/U_EMREXTTEXTOUTW and all other iGraphicsMode fields
  @{
*/
#define U_GM_COMPATIBLE     1
#define U_GM_ADVANCED       2
#define U_GM_LAST           2
/** @} */

/** \defgroup U_LOGBRUSH_lbHatch_Qualifiers HatchStyle Enumeration
  For U_LOGBRUSH lbHatch field
  @{
*/
#define U_HS_HORIZONTAL       0
#define U_HS_VERTICAL         1
#define U_HS_FDIAGONAL        2
#define U_HS_BDIAGONAL        3
#define U_HS_CROSS            4
#define U_HS_DIAGCROSS        5
#define U_HS_SOLIDCLR         6
#define U_HS_DITHEREDCLR      7
#define U_HS_SOLIDTEXTCLR     8
#define U_HS_DITHEREDTEXTCLR  9
#define U_HS_SOLIDBKCLR      10
#define U_HS_DITHEREDBKCLR   11
/** @} */

/** \defgroup U_EMRSETICMMODE_iMode_Qualifiers ICMMode Enumeration
  For EMF U_EMR_SETICMMODE iMode field
  @{
*/
#define U_ICM_OFF   1
#define U_ICM_ON    2
#define U_ICM_QUERY 3
/** @} */

/** \defgroup U_COLORADJUSTMENT_caIlluminantIndex_Qualifiers Illuminant Enumeration
  For U_COLORADJUSTMENT caIlluminantIndex field
  @{
*/
#define U_ILLUMINANT_DEVICE_DEFAULT 0
#define U_ILLUMINANT_A              1
#define U_ILLUMINANT_B              2
#define U_ILLUMINANT_C              3
#define U_ILLUMINANT_D50            4
#define U_ILLUMINANT_D55            5
#define U_ILLUMINANT_D65            6
#define U_ILLUMINANT_D75            7
#define U_ILLUMINANT_F2             8
#define U_ILLUMINANT_MAX_INDEX      ILLUMINANT_F2
#define U_ILLUMINANT_TUNGSTEN       ILLUMINANT_A
#define U_ILLUMINANT_DAYLIGHT       ILLUMINANT_C 
#define U_ILLUMINANT_FLUORESCENT    ILLUMINANT_F2
#define U_ILLUMINANT_NTSC           ILLUMINANT_C
/** @} */

/** \defgroup U_LOGBRUSH_lbStyle_Qualifiers LB_Style Enumeration
  For U_LOGBRUSH lbStyle field
  @{
*/
#define U_BS_SOLID         0
#define U_BS_NULL          1
#define U_BS_HOLLOW        1
#define U_BS_HATCHED       2
#define U_BS_PATTERN       3
#define U_BS_INDEXED       4
#define U_BS_DIBPATTERN    5
#define U_BS_DIBPATTERNPT  6
#define U_BS_PATTERN8X8    7
#define U_BS_DIBPATTERN8X8 8
#define U_BS_MONOPATTERN   9
/** @} */

/** \defgroup _LOGCOLORSPACE_lcsCSType_Qualifiers LCS_CSType Enumeration
  For U_LOGCOLORSPACEA/U_LOGCOLORSPACEW lcsCSType field
  @{
*/
#define U_LCS_CALIBRATED_RGB 0x00000000L
#define U_LCS_DEVICE_RGB     0x00000001L
#define U_LCS_DEVICE_CMYK    0x00000002L
/** @} */

/** \defgroup U_LOGCOLORSPACE_lcsIntent_Qualifiers LCS_Intent Enumeration
  For U_LOGCOLORSPACEA/U_LOGCOLORSPACEW lcsIntent field 
  @{
*/
#define U_LCS_GM_BUSINESS         0x00000001L
#define U_LCS_GM_GRAPHICS         0x00000002L
#define U_LCS_GM_IMAGES           0x00000004L
#define U_LCS_GM_ABS_COLORIMETRIC 0x00000008L
/** @} */

/** \defgroup U_PANOSE_bLetterForm_Qualifiers Letterform Enumeration
  For U_PANOSE bLetterForm field
  @{
*/
#define U_PAN_LETT_NORMAL_COMPACT     2
#define U_PAN_LETT_NORMAL_WEIGHTED    3
#define U_PAN_LETT_NORMAL_BOXED       4
#define U_PAN_LETT_NORMAL_FLATTENED   5
#define U_PAN_LETT_NORMAL_ROUNDED     6
#define U_PAN_LETT_NORMAL_OFF_CENTER  7
#define U_PAN_LETT_NORMAL_SQUARE      8
#define U_PAN_LETT_OBLIQUE_COMPACT    9
#define U_PAN_LETT_OBLIQUE_WEIGHTED   10
#define U_PAN_LETT_OBLIQUE_BOXED      11
#define U_PAN_LETT_OBLIQUE_FLATTENED  12
#define U_PAN_LETT_OBLIQUE_ROUNDED    13
#define U_PAN_LETT_OBLIQUE_OFF_CENTER 14
#define U_PAN_LETT_OBLIQUE_SQUARE     15
/** @} */

/** \defgroup U_LOGFONT_lfWeight_Qualifiers LF_Weight Enumeration
  For U_LOGFONT lfWeight field
  @{
*/
#define U_FW_DONTCARE     0
#define U_FW_THIN       100
#define U_FW_EXTRALIGHT 200
#define U_FW_ULTRALIGHT 200
#define U_FW_LIGHT      300
#define U_FW_NORMAL     400
#define U_FW_REGULAR    400
#define U_FW_MEDIUM     500
#define U_FW_SEMIBOLD   600
#define U_FW_DEMIBOLD   600
#define U_FW_BOLD       700
#define U_FW_EXTRABOLD  800
#define U_FW_ULTRABOLD  800
#define U_FW_HEAVY      900
#define U_FW_BLACK      900
/** @} */

/** \defgroup U_LOGFONT_lfItalic_Qualifiers LF_Italic Enumeration
  For U_LOGFONT lfItalic field
  @{
*/
#define U_FW_NOITALIC     0
#define U_FW_ITALIC       1
/** @} */

/** \defgroup U_LOGFONT_lfunderline_Qualifiers LF_Underline Enumeration
  For U_LOGFONT lfunderline field
  @{
*/
#define U_FW_NOUNDERLINE  0
#define U_FW_UNDERLINE    1
/** @} */

/** \defgroup U_LOGFONT_lfStrikeOut_Qualifiers LF_StrikeOut Enumeration
  For U_LOGFONT lfStrikeOut field
  @{
*/
#define U_FW_NOSTRIKEOUT  0
#define U_FW_STRIKEOUT    1
/** @} */

/** \defgroup U_LOGFONT_lfCharSet_Qualifiers LF_CharSet Enumeration
  For U_LOGFONT lfCharSet field
  @{
*/
#define U_ANSI_CHARSET          (uint8_t)0   /* CP1252, ansi-0, iso8859-{1,15} */
#define U_DEFAULT_CHARSET       (uint8_t)1
#define U_SYMBOL_CHARSET        (uint8_t)2
#define U_SHIFTJIS_CHARSET      (uint8_t)128 /* CP932 */
#define U_HANGEUL_CHARSET       (uint8_t)129 /* CP949, ksc5601.1987-0 */
#define U_HANGUL_CHARSET        U_HANGEUL_CHARSET
#define U_GB2312_CHARSET        (uint8_t)134 /* CP936, gb2312.1980-0 */
#define U_CHINESEBIG5_CHARSET   (uint8_t)136 /* CP950, big5.et-0 */
#define U_GREEK_CHARSET         (uint8_t)161    /* CP1253 */
#define U_TURKISH_CHARSET       (uint8_t)162    /* CP1254, -iso8859-9 */
#define U_HEBREW_CHARSET        (uint8_t)177    /* CP1255, -iso8859-8 */
#define U_ARABIC_CHARSET        (uint8_t)178    /* CP1256, -iso8859-6 */
#define U_BALTIC_CHARSET        (uint8_t)186    /* CP1257, -iso8859-13 */
#define U_RUSSIAN_CHARSET       (uint8_t)204    /* CP1251, -iso8859-5 */
#define U_EE_CHARSET            (uint8_t)238    /* CP1250, -iso8859-2 */
#define U_EASTEUROPE_CHARSET    U_EE_CHARSET
#define U_THAI_CHARSET          (uint8_t)222 /* CP874, iso8859-11, tis620 */
#define U_JOHAB_CHARSET         (uint8_t)130 /* korean (johab) CP1361 */
#define U_MAC_CHARSET           (uint8_t)77
#define U_OEM_CHARSET           (uint8_t)255
/* I don't know if the values of *_CHARSET macros are defined in Windows
 * or if we can choose them as we want. -- srtxg
 */
#define U_VISCII_CHARSET        (uint8_t)240 /* viscii1.1-1 */
#define U_TCVN_CHARSET          (uint8_t)241 /* tcvn-0 */
#define U_KOI8_CHARSET          (uint8_t)242 /* koi8-{r,u,ru} */
#define U_ISO3_CHARSET          (uint8_t)243 /* iso8859-3 */
#define U_ISO4_CHARSET          (uint8_t)244 /* iso8859-4 */
#define U_ISO10_CHARSET         (uint8_t)245 /* iso8859-10 */
#define U_CELTIC_CHARSET        (uint8_t)246 /* iso8859-14 */
/** @} */

/** \defgroup U_LOGFONT_lfOutPrecision_Qualifiers LF_OutPrecision Enumeration
  For U_LOGFONT lfOutPrecision field
  @{
*/
#define U_OUT_DEFAULT_PRECIS   0
#define U_OUT_STRING_PRECIS    1
#define U_OUT_CHARACTER_PRECIS 2
#define U_OUT_STROKE_PRECIS    3
#define U_OUT_TT_PRECIS        4
#define U_OUT_DEVICE_PRECIS    5
#define U_OUT_RASTER_PRECIS    6
#define U_OUT_TT_ONLY_PRECIS   7
#define U_OUT_OUTLINE_PRECIS   8
/** @} */

/** \defgroup U_LOGFONT_lfClipPrecision_Qualifiers LF_ClipPrecision Enumeration
  For U_LOGFONT lfClipPrecision field
  @{
*/
#define U_CLIP_DEFAULT_PRECIS   0x00
#define U_CLIP_CHARACTER_PRECIS 0x01
#define U_CLIP_STROKE_PRECIS    0x02
#define U_CLIP_MASK             0x0F
#define U_CLIP_LH_ANGLES        0x10
#define U_CLIP_TT_ALWAYS        0x20
#define U_CLIP_EMBEDDED         0x80
/** @} */

/** \defgroup U_LOGFONT_lfQuality_Qualifiers LF_Quality Enumeration
  For For U_LOGFONT lfQuality field
  @{
*/
#define U_DEFAULT_QUALITY        0
#define U_DRAFT_QUALITY          1
#define U_PROOF_QUALITY          2
#define U_NONANTIALIASED_QUALITY 3
#define U_ANTIALIASED_QUALITY    4
/** @} */

/** \defgroup U_LOGFONT_lfPitchAndFamily_Qualifiers LF_PitchAndFamily Enumeration
  For U_LOGFONT lfPitchAndFamily field
  @{
*/
#define U_DEFAULT_PITCH  0x00
#define U_FIXED_PITCH    0x01
#define U_VARIABLE_PITCH 0x02
#define U_MONO_FONT      0x08
#define U_FF_DONTCARE    0x00
#define U_FF_ROMAN       0x10
#define U_FF_SWISS       0x20
#define U_FF_MODERN      0x30
#define U_FF_SCRIPT      0x40
#define U_FF_DECORATIVE  0x50
/** @} */

/** \defgroup U_EMRSETMAPMODE_iMode_Qualifiers MapMode Enumeration
  For U_EMRSETMAPMODE iMode field
  @{
*/
#define U_MM_TEXT           1
#define U_MM_LOMETRIC       2
#define U_MM_HIMETRIC       3
#define U_MM_LOENGLISH      4
#define U_MM_HIENGLISH      5
#define U_MM_TWIPS          6
#define U_MM_ISOTROPIC      7
#define U_MM_ANISOTROPIC    8
#define U_MM_MIN            U_MM_TEXT
#define U_MM_MAX            U_MM_ANISOTROPIC
#define U_MM_MAX_FIXEDSCALE U_MM_TWIPS
/** @} */


/** \defgroup U_PANOSE_bMidline_Qualifiers MidLine Enumeration
  For U_PANOSE bMidline field
  @{
*/
#define U_PAN_MIDLINE_STANDARD_TRIMMED 2
#define U_PAN_MIDLINE_STANDARD_POINTED 3
#define U_PAN_MIDLINE_STANDARD_SERIFED 4
#define U_PAN_MIDLINE_HIGH_TRIMMED     5
#define U_PAN_MIDLINE_HIGH_POINTED     6
#define U_PAN_MIDLINE_HIGH_SERIFED     7
#define U_PAN_MIDLINE_CONSTANT_TRIMMED 8
#define U_PAN_MIDLINE_CONSTANT_POINTED 9
#define U_PAN_MIDLINE_CONSTANT_SERIFED 10
#define U_PAN_MIDLINE_LOW_TRIMMED      11
#define U_PAN_MIDLINE_LOW_POINTED      12
#define U_PAN_MIDLINE_LOW_SERIFED      13
/** @} */

/** \defgroup U_EMRSETLAYOUT_iMode_Qualifiers Mirroring Enumeration
  For U_EMRSETLAYOUT iMode field
  @{
*/
#define U_LAYOUT_LTR                         0x00000000
#define U_LAYOUT_RTL                         0x00000001
#define U_LAYOUT_BITMAPORIENTATIONPRESERVED  0x00000008
#define U_NOMIRRORBITMAP                     0x80000000
/** @} */

/** \defgroup U_EMRMODIFYWORLDTRANSFORM_iMode_Qualifiers ModifyWorldTransformMode Enumeration
  For U_EMRMODIFYWORLDTRANSFORM iMode
  @{
*/
#define U_MWT_IDENTITY      1
#define U_MWT_LEFTMULTIPLY  2
#define U_MWT_RIGHTMULTIPLY 3
#define U_MWT_MIN           U_MWT_IDENTITY
#define U_MWT_MAX           U_MWT_RIGHTMULTIPLY      
/** @} */

/** \defgroup U_PANOSE_common_Qualifiers PanoseCommon Enumeration
  Used by all PAN_* enumerations, but only defined once here.  
  See also U_PAN_ALL1 after the U_PANOSE structure
  @{
*/
#define U_PAN_ANY     0
#define U_PAN_NO_FIT  1
/** @} */

/** \defgroup U_PANOSE_index PanoseIndex Enumeration
  Fositions of each field in U_PANOSE structure.
  @{
*/
#define U_PANOSE_COUNT              10
#define U_PANOSE_FAMILYTYPE_INDEX   0
#define U_PAN_SERIFSTYLE_INDEX      1
#define U_PAN_WEIGHT_INDEX          2
#define U_PAN_PROPORTION_INDEX      3
#define U_PAN_CONTRAST_INDEX        4
#define U_PAN_STROKEVARIATION_INDEX 5
#define U_PAN_ARMSTYLE_INDEX        6
#define U_PAN_LETTERFORM_INDEX      7
#define U_PAN_MIDLINE_INDEX         8
#define U_PAN_XHEIGHT_INDEX         9
/** @} */

/** \defgroup U_*LOGPEN_elpPenStyle_Qualifiers PenStyle Enumeration
  For U_LOGPEN lopnStyle and U_EXTLOGPEN elpPenStyle fields
  @{
*/
#define U_PS_SOLID         0x00000000
#define U_PS_DASH          0x00000001  //!< This only works when NO other U_PS is set.  Line width is minimum no matter what pen is set to.
#define U_PS_DOT           0x00000002  //!< This only works when NO other U_PS is set.  Line width is minimum no matter what pen is set to.
#define U_PS_DASHDOT       0x00000003  //!< This only works when NO other U_PS is set.  Line width is minimum no matter what pen is set to.
#define U_PS_DASHDOTDOT    0x00000004  //!< This only works when NO other U_PS is set.  Line width is minimum no matter what pen is set to.
#define U_PS_NULL          0x00000005
#define U_PS_INSIDEFRAME   0x00000006
#define U_PS_USERSTYLE     0x00000007
#define U_PS_ALTERNATE     0x00000008
#define U_PS_STYLE_MASK    0x0000000f

#define U_PS_ENDCAP_ROUND  0x00000000  //!< These are only with U_PS_GEOMETRIC
#define U_PS_ENDCAP_SQUARE 0x00000100
#define U_PS_ENDCAP_FLAT   0x00000200
#define U_PS_ENDCAP_MASK   0x00000f00

#define U_PS_JOIN_ROUND    0x00000000  //!< These are only with U_PS_GEOMETRIC
#define U_PS_JOIN_BEVEL    0x00001000
#define U_PS_JOIN_MITER    0x00002000
#define U_PS_JOIN_MASK     0x0000f000

#define U_PS_COSMETIC      0x00000000  //!< width may only be 1 pixel.  (If set higher it is still drawn as 1).
#define U_PS_GEOMETRIC     0x00010000  //!< width may be >1 pixel, but style may only be U_PS_SOLID or U_PS_NULL.
#define U_PS_TYPE_MASK     0x000f0000
/** @} */

/** \defgroup U_PIXELFORMATDESCRIPTOR_dwFlags_Qualifiers PFD_dwFlags Enumeration
  For U_PIXELFORMATDESCRIPTOR dwFlags field 
  @{
*/
#define U_PFD_DOUBLEBUFFER          0x00000001
#define U_PFD_STEREO                0x00000002
#define U_PFD_DRAW_TO_WINDOW        0x00000004
#define U_PFD_DRAW_TO_BITMAP        0x00000008
#define U_PFD_SUPPORT_GDI           0x00000010
#define U_PFD_SUPPORT_OPENGL        0x00000020
#define U_PFD_GENERIC_FORMAT        0x00000040
#define U_PFD_NEED_PALETTE          0x00000080
#define U_PFD_NEED_SYSTEM_PALETTE   0x00000100
#define U_PFD_SWAP_EXCHANGE         0x00000200
#define U_PFD_SWAP_COPY             0x00000400
#define U_PFD_SWAP_LAYER_BUFFERS    0x00000800
#define U_PFD_GENERIC_ACCELERATED   0x00001000
/** @} */

/** \defgroup U_PIXELFORMATDESCRIPTOR_iLayerType_Qualifiers PFD_iLayerType Enumeration
  For U_PIXELFORMATDESCRIPTOR iLayerType field
  @{
*/
#define U_PFD_MAIN_PLANE       0
#define U_PFD_OVERLAY_PLANE    1
#define U_PFD_UNDERLAY_PLANE   (-1)
/** @} */

/** \defgroup U_PIXELFORMATDESCRIPTOR_iPixelType_Qualifiers PFD_iPixelType Enumeration
  For U_PIXELFORMATDESCRIPTOR iPixelType field
  @{
*/
#define U_PFD_TYPE_RGBA        0
#define U_PFD_TYPE_COLORINDEX  1
/** @} */

/** \defgroup U_EMRPOLY_iMode_Qualifiers Point Enumeration
  For U_EMRPOLYDRAW and U_EMRPOLAYDRAW16 abTypes fields.
 @{
*/
#define U_PT_CLOSEFIGURE          0x0001
#define U_PT_LINETO               0x0002
#define U_PT_BEZIERTO             0x0004
#define U_PT_MOVETO               0x0006
/** @} */

/** \defgroup U_EMRSETPOLYFILLMODE_iMode_Qualifiers PolygonFillMode Enumeration
  For U_EMRSETPOLYFILLMODE iMode field
  @{
*/
#define U_ALTERNATE         1
#define U_WINDING           2
#define U_POLYFILL_LAST     2
/** @} */

/** \defgroup U_BITMAPV5HEADER_bV5CSType_Qualifiers Profile Enumeration
  For U_BITMAPV5HEADER bV5CSType field
  @{
*/
#define U_PROFILE_LINKED   'LINK'
#define U_PROFILE_EMBEDDED 'MBED'
/** @} */

/** \defgroup U_PANOSE_bProportion_Qualifiers Proportion Enumeration
  For U_PANOSE bProportion field
  @{
*/
#define U_PAN_PROP_OLD_STYLE      2
#define U_PAN_PROP_MODERN         3
#define U_PAN_PROP_EVEN_WIDTH     4
#define U_PAN_PROP_EXPANDED       5
#define U_PAN_PROP_CONDENSED      6
#define U_PAN_PROP_VERY_EXPANDED  7
#define U_PAN_PROP_VERY_CONDENSED 8
#define U_PAN_PROP_MONOSPACED     9
/** @} */

/** \defgroup U_EMR_dwROP_Qualifiers Ternary Raster Operation enumeration
  For U_EMR* dwROP fields.
  
  These codes specify:
    1.  an order of operands (composed of various orders and combinations of: Dest, Src, Pen)
        (There are 3, hence "Ternary Raster Operation")
    2.  an order of operators to apply to the operands (composed of Not, Xor, Or, And)
  Only a few of the more common operations are provided here.
  When the Operation does not use a Src operand the corresponding source bitmap may be
     omitted from the record.
  
  For more details see:
      http://wiki.winehq.org/TernaryRasterOps
  @{
*/
#define U_SRCCOPY     0xcc0020
#define U_SRCPAINT    0xee0086
#define U_SRCAND      0x8800c6
#define U_SRCINVERT   0x660046
#define U_SRCERASE    0x440328
#define U_NOTSRCCOPY  0x330008
#define U_NOTSRCERASE 0x1100a6
#define U_MERGECOPY   0xc000ca
#define U_MERGEPAINT  0xbb0226
#define U_PATCOPY     0xf00021
#define U_PATPAINT    0xfb0a09
#define U_PATINVERT   0x5a0049
#define U_DSTINVERT   0x550009
#define U_BLACKNESS   0x000042
#define U_WHITENESS   0xff0062
/** @} */

/** \defgroup U_EMRSETROP2_iMode_Qualifiers Binary Raster Operation Enumeration
  For U_EMRSETROP2 iMode field
  
  These codes specify:
    1.  an order of operands (composed of various orders and combinations of: Dest, Pen)
        (There are 2, hence "Binary Raster Operation")
    2.  an order of operators to apply to the operands (composed of Not, Xor, Or, And)
  Only a few of the more common operations are provided here.

  The default is U_R2_COPYPEN.  If this value is changed to something else all subsequenty
  draw operations will use the altered logic.  For instance, if it is set to U_R2_BLACK and
  a red rectangle is drawn it will appear as a black rectangle.
  
  @{
*/
#define U_R2_BLACK        1
#define U_R2_NOTMERGEPEN  2
#define U_R2_MASKNOTPEN   3
#define U_R2_NOTCOPYPEN   4
#define U_R2_MASKPENNOT   5
#define U_R2_NOT          6
#define U_R2_XORPEN       7
#define U_R2_NOTMASKPEN   8
#define U_R2_MASKPEN      9
#define U_R2_NOTXORPEN   10
#define U_R2_NOP         11
#define U_R2_MERGENOTPEN 12
#define U_R2_COPYPEN     13
#define U_R2_MERGEPENNOT 14
#define U_R2_MERGEPEN    15
#define U_R2_WHITE       16
#define U_R2_LAST        16
/** @} */

/** \defgroup U_EMRSELECTCLIP_iMode_Qualifiers RegionMode Enumeration
  For U_EMRSELECTCLIPPATH and U_EMREXTSELECTCLIPRGN iMode field
  @{
*/
#define U_RGN_AND  1
#define U_RGN_OR   2
#define U_RGN_XOR  3
#define U_RGN_DIFF 4
#define U_RGN_COPY 5
#define U_RGN_MIN  U_RGN_AND
#define U_RGN_MAX  U_RGN_COPY
/** @} */

/** \defgroup U_PANOSE_bSerifStyle_Qualifiers SerifType Enumeration
  For U_PANOSE bSerifStyle field
  @{
*/
#define U_PAN_SERIF_COVE                2
#define U_PAN_SERIF_OBTUSE_COVE         3
#define U_PAN_SERIF_SQUARE_COVE         4
#define U_PAN_SERIF_OBTUSE_SQUARE_COVE  5
#define U_PAN_SERIF_SQUARE              6
#define U_PAN_SERIF_THIN                7
#define U_PAN_SERIF_BONE                8
#define U_PAN_SERIF_EXAGGERATED         9
#define U_PAN_SERIF_TRIANGLE           10
#define U_PAN_SERIF_NORMAL_SANS        11
#define U_PAN_SERIF_OBTUSE_SANS        12
#define U_PAN_SERIF_PERP_SANS          13
#define U_PAN_SERIF_FLARED             14
#define U_PAN_SERIF_ROUNDED            15
/** @} */

/** \defgroup U_EMRSELECTOBJECT_ihObject_Qualifiers StockObject Enumeration
  For U_EMRSELECTOBJECT ihObject field.
  @{
*/
#define U_STOCK_OBJECT          0x80000000
#define U_WHITE_BRUSH           0x80000000
#define U_LTGRAY_BRUSH          0x80000001
#define U_GRAY_BRUSH            0x80000002
#define U_DKGRAY_BRUSH          0x80000003
#define U_BLACK_BRUSH           0x80000004
#define U_NULL_BRUSH            0x80000005
#define U_HOLLOW_BRUSH          0x80000005
#define U_WHITE_PEN             0x80000006
#define U_BLACK_PEN             0x80000007
#define U_NULL_PEN              0x80000008
#define U_OEM_FIXED_FONT        0x8000000A
#define U_ANSI_FIXED_FONT       0x8000000B
#define U_ANSI_VAR_FONT         0x8000000C
#define U_SYSTEM_FONT           0x8000000D
#define U_DEVICE_DEFAULT_FONT   0x8000000E
#define U_DEFAULT_PALETTE       0x8000000F
#define U_SYSTEM_FIXED_FONT     0x80000010
#define U_DEFAULT_GUI_FONT      0x80000011
#define U_STOCK_LAST            0x80000011
/** @} */

/** \defgroup U_EMRSETSTRETCHBLTMODE_iMode_Qualifiers StretchMode Enumeration
  For EMF U_EMRSETSTRETCHBLTMODE iMode field
  @{
*/
#define U_BLACKONWHITE         1
#define U_WHITEONBLACK         2
#define U_COLORONCOLOR         3
#define U_HALFTONE             4
#define U_MAXSTRETCHBLTMODE    4
#define U_STRETCH_ANDSCANS     1
#define U_STRETCH_ORSCANS      2
#define U_STRETCH_DELETESCANS  3
#define U_STRETCH_HALFTONE     4
/** @} */

/** \defgroup U_PANOSE_bStrokeVariation_Qualifiers StrokeVariation Enumeration
  For U_PANOSE bStrokeVariation field
  @{
*/
#define U_PAN_STROKE_GRADUAL_DIAG 2
#define U_PAN_STROKE_GRADUAL_TRAN 3
#define U_PAN_STROKE_GRADUAL_VERT 4
#define U_PAN_STROKE_GRADUAL_HORZ 5
#define U_PAN_STROKE_RAPID_VERT   6
#define U_PAN_STROKE_RAPID_HORZ   7
#define U_PAN_STROKE_INSTANT_VERT 8
/** @} */

/** \defgroup U_EMRSETTEXTALIGN_iMode_Qualifiers TextAlignment Enumeration
  For U_EMRSETTEXTALIGN iMode field
  
  Recall that EMF coordinates have UL  closest to {0,0}, LR is below and to the right of UL and so has LARGER
  {x,y} coordinates.  In the following "TOP" is on the horizontal line defined by LR, as it has larger y coordinates,
  which when viewing the EMF file, would actually be on the BOTTOM of the bounding rectangle.  Similarly, left and right
  are reversed.
  
  Microsoft documentation (WMF manual, section 2.1.2.3) says that the text starts on certain edges of the bounding rectangle.
  That is apparently not true, whether the bounding rectangle is {0,0,-1,-1}, which is effectively no bounding rectangle,
  or if a valid bounding rectangle is specified.  In all cases the text (in Windows XP Preview) starts, has center at, or ends 
  at the center point.  Vertical offsets seem to be defined analogously, but with respect to the height of the font.  The bounding
  rectangle defined for the U_EMRTEXT record appears to be ignored.

  Microsoft documentation (EMF manual,section 2.2.5) says that the same rectangle is used for "clipping or opaquing" by ExtTextOutA/W.
  That does not seem to occur either.

  @{
*/
//  Horizontal text flags
#define U_TA_DEFAULT    0x00                // default alignment
#define U_TA_NOUPDATECP 0x00                // Reference point does not move
#define U_TA_UPDATECP   0x01                // Reference point moves to end of next text drawn.
#define U_TA_LEFT       0x00                // Reference point is on left edge of bounding rectangle
#define U_TA_RIGHT      0x02                // Reference point is on right edge of bounding rectangle
#define U_TA_CENTER     0x06                // Reference point is on center vertical line of bounding rectangle
#define U_TA_TOP        0x00                // Reference point is on top edge of bounding rectangle
#define U_TA_BOTTOM     0x08                // Reference point is on bottom edge of bounding rectangle
#define U_TA_BASEBIT    0x10                // Reference point is on baseline of text if this bit is set, for 0x10 <-> 0x18
#define U_TA_BASELINE   0x18                // Reference point is on baseline of text
#define U_TA_RTLREADING 0x100               // Set for Right to Left languages like Hebrew and Arabic
#define U_TA_MASK       U_TA_BASELINE+U_TA_CENTER+U_TA_UPDATECP+U_TA_RTLREADING
//  Vertical text flags
#define U_VTA_BASELINE  U_TA_BASELINE       // for vertical text
#define U_VTA_LEFT      U_TA_BOTTOM
#define U_VTA_RIGHT     U_TA_TOP
#define U_VTA_CENTER    U_TA_CENTER
#define U_VTA_BOTTOM    U_TA_RIGHT
#define U_VTA_TOP       U_TA_LEFT
/** @} */

/** \defgroup U_PANOSE_bWeight_Qualifiers Weight Enumeration
  For U_PANOSE bWeight field
  @{
*/
#define U_PAN_WEIGHT_VERY_LIGHT 2
#define U_PAN_WEIGHT_LIGHT      3
#define U_PAN_WEIGHT_THIN       4
#define U_PAN_WEIGHT_BOOK       5
#define U_PAN_WEIGHT_MEDIUM     6
#define U_PAN_WEIGHT_DEMI       7
#define U_PAN_WEIGHT_BOLD       8
#define U_PAN_WEIGHT_HEAVY      9
#define U_PAN_WEIGHT_BLACK      10
#define U_PAN_WEIGHT_NORD       11
/** @} */

/** \defgroup U_PANOSE_bXHeight_Qualifiers XHeight Enumeration
  For U_PANOSE bXHeight field
  @{
*/
#define U_PAN_XHEIGHT_CONSTANT_SMALL    2
#define U_PAN_XHEIGHT_CONSTANT_STANDARD 3
#define U_PAN_XHEIGHT_CONSTANT_LARGE    4
#define U_PAN_XHEIGHT_DUCKING_SMALL     5
#define U_PAN_XHEIGHT_DUCKING_STANDARD  6
#define U_PAN_XHEIGHT_DUCKING_LARGE     7
/** @} */

/** \defgroup U_BLEND_Op_Qualifiers Blend Enumeration
  For U_BLEND Op field
  @{
*/
#define U_AC_SRC_GLOBAL 0
#define U_AC_SRC_CONST  0
#define U_AC_SRC_ALPHA  1
/** @} */


//  ***************************************************************************
/** \defgroup Miscellaneous_values Miscellaneous Values
  @{
*/
#define U_NONE                        0               //!< Generic for nothing selected for all flag fields
#define U_PI                          3.14159265358979323846	//!< pi
#define U_READ                        1
#define U_WRITE                       0
#define U_ENHMETA_VERSION             0x00010000      //!< U_EMRHEADER nVersion field
#define U_DV_SGNTR                    0x08007664      //!< For U_DESIGNVECTOR Signature field
#define U_LP_VERSION                  0x0300          //!< For U_LOGPALETTE palVersion field
#define U_RDH_RECTANGLES              1               //!< For U_RGNDATAHEADER iType field
#define U_RDH_OBJSIZE                 0x20            //!< For U_RGNDATAHEADER dwSIze field
#define U_RGB_GAMMA_MIN               (uint16_t)02500 //!< For U_COLORADJUSTMENT ca[Red|Green|Blue]Gamma fields
#define U_RGB_GAMMA_MAX               (uint16_t)65000 //!< For U_COLORADJUSTMENT ca[Red|Green|Blue]Gamma fields
#define U_REFERENCE_WHITE_MIN         (uint16_t)6000  //!< For U_COLORADJUSTMENT caReferenceWhite field
#define U_REFERENCE_WHITE_MAX         (uint16_t)10000 //!< For U_COLORADJUSTMENT caReferenceWhite field
#define U_REFERENCE_BLACK_MIN         (uint16_t)0     //!< For U_COLORADJUSTMENT caReferenceBlack field
#define U_REFERENCE_BLACK_MAX         (uint16_t)4000  //!< For U_COLORADJUSTMENT caReferenceBlack field
#define U_COLOR_ADJ_MIN               ((int16_t)-100) //!< For U_COLORADJUSTMENT ca[Contrast|Brightness|Colorfulness|RedGreenTint] fields
#define U_COLOR_ADJ_MAX               (int16_t) 100   //!< For U_COLORADJUSTMENT ca[Contrast|Brightness|Colorfulness|RedGreenTint] fields
#define U_MAX_PATH                    1024            //!< longest path name for a file
#define U_LCS_SIGNATURE               0x50534F43      //!< logColorSpace Signature
#define U_LCS_VERSION                 0x400           //!< logColorSpace Version
#define U_REC_FREE                    1               //!< used with emf_append
#define U_REC_KEEP                    0               //!< used with emf_append
/** Solaris 8 has problems with round/roundf, just use this everywhere  */
#define U_ROUND(A)  ( (A) > 0 ? floor((A)+0.5) : ( (A) < 0 ? -floor(-(A)+0.5) : (A) ) )

/** @} */

//  ***************************************************************************
//  Macros

/** \defgroup Common_macros Common Macros
  @{
*/
//  Color U_BGR(A), byte order lo to hi: {B,G,R,A} corresponding to U_RGBQUAD.  Set/Get Macros.  
//  These are used in EMF structures and the byte order must be the same in memory or on disk.
#define U_BGR(r,g,b)           (U_RGBQUAD){b,g,r,0}             //!<  Set any BGR  color with an {r,g,b} triplet
#define U_BGRA(r,g,b,a)        (U_RGBQUAD){b,g,r,a}             //!<  Set any BGRA color with an {r,g,b,a} quad
#define U_WHITE                U_BGR(255,255,255)               //!<  Set BGR white.
#define U_BLACK                U_BGR(0,0,0)                     //!<  Set BGR black.
#define U_BGRAGetR(rgb)        (rgb.Red     )                   //!<  Color BGR Get Red Macro.
#define U_BGRAGetG(rgb)        (rgb.Green   )                   //!<  Color BGR Get Green Macro.
#define U_BGRAGetB(rgb)        (rgb.Blue    )                   //!<  Color BGR Get Blue Macro.
#define U_BGRAGetA(rgb)        (rgb.Reserved)                   //!<  Color BGR Get A/reserved Macro.

#define U_PALETTERGB(r,g,b)    U_RGB(r,g,b,0x02))               //!<  Set any Palette RGB color.
#define U_PALETTEINDEX(i)     ((U_COLORREF)(0x01000000 | (uint16_t)(i)))\
                                                                //!<  Get RGB from Palette by index.

//  Color U_RGB(A), byte order lo to hi: {R,G,B,A} corresponding to U_COLORREF. Set/Get Macros.
//  These are used in EMF structures and the byte order must be the same in memory or on disk.
//  These MAY be used in PNG and other libraries if these enforce byte order in memory,otherwise
//  U_swap4 may need to also be employed.
#define U_RGB(r,g,b)         (U_COLORREF){r,g,b,0}              //!<  Set any RGB  color with an {r,g,b} triplet
#define U_RGBA(r,g,b,a)      (U_COLORREF){r,g,b,a}              //!<  Set any RGBA color with an {r,g,b,a} quad
#define U_RGBAGetR(rgb)     ((U_COLORREF)rgb).Red               //!<  Color RGB Get Red Macro.
#define U_RGBAGetG(rgb)     ((U_COLORREF)rgb).Green             //!<  Color RGB Get Green Macro.
#define U_RGBAGetB(rgb)     ((U_COLORREF)rgb).Blue              //!<  Color RGB Get Blue Macro.
#define U_RGBAGetA(rgb)     ((U_COLORREF)rgb).Reserved          //!<  Color RGBA Get A/reserved Macro.

//   color type conversions
#define U_RGB2BGR(rgb)        (U_RGBQUAD){ U_RGBAGetB(rgb),U_RGBAGetG(rgb),U_RGBAGetR(rgb),0}   //!<  Set any BGR color from an RGB color
#define U_BGR2RGB(rgb)        (U_COLORREF){U_BGRAGetR(rgb),U_BGRAGetG(rgb),U_BGRAGetB(rgb),0}   //!<  Set any RGB color from an BGR color
#define U_RGBA2BGRA(rgb)      (U_RGBQUAD){ U_RGBAGetB(rgb),U_RGBAGetG(rgb),U_RGBAGetR(rgb),U_RGBAGetA(rgb)}   //!<  Set any BGRA color from an RGBA color
#define U_BGRA2RGBA(rgb)      (U_COLORREF){U_BGRAGetR(rgb),U_BGRAGetG(rgb),U_BGRAGetB(rgb),U_BGRAGetA(rgb)}   //!<  Set any RGBA color from an BGRA color

//  Color CMYK Get/Set Macros
#define U_CMYK(c,m,y,k)\
  ((COLOREF)((((uint8_t)(k)|((uint16_t)((uint8_t)(y))<<8))|(((uint32_t)(uint8_t)(m))<<16))|(((uint32_t)(uint8_t)(c))<<24))) \
                                                                //!<   Color CMYK Set Macro.
#define U_GetKValue(cmyk)     ((uint8_t)  (cmyk) )              //!<  Color CMYK Get K Macro.
#define U_GetYValue(cmyk)     ((uint8_t) ((cymk) >> 8))         //!<  Color CMYK Get Y Macro.
#define U_GetMValue(cmyk)     ((uint8_t) ((cymk) >> 16))        //!<  Color CMYK Get M Macro.
#define U_GetCValue(cmyk)     ((uint8_t) ((cymk) >> 24))        //!<  Color CMYK Get C Macro.

// Other macros
#define U_Gamma(A) (A < U_RGB_GAMMA_MIN ? U_RGB_GAMMA_MIN : (A > U_RGB_GAMMA_MAX ? U_RGB_GAMMA_MAX: A)) \
                                                                //!<  Gamma set Macro (enforce range).
#define U_PM(A,B)     ((A)<-(B)?-(B):((A)>(B)?(B):(A)))         //!<  Plus/Minus Range Macro (B must be postitive!).
#define U_MNMX(A,B,C) ((A)<(B)?(B):((A)>(C)?(C):(A)))           //!<  Min/Max Range Macro (B <= A <= C).
#define U_MIN(A,B)    ((A)>(B)?(B):(A))                         //!<  Minimum of A,B
#define U_MAX(A,B)    ((A)>(B)?(A):(B))                         //!<  Maximum of A,B

//  basic EMR macros.
#define U_EMRTYPE(A) (((PU_EMR)A)->iType)                       //!<  Get iType from U_EMR* record
#define U_EMRSIZE(A) (((PU_EMR)A)->nSize)                       //!<  Get nSize from U_EMR* record

// Utility macros
#define UP4(A) (4 * ((A + 3 ) / 4))                             //!< Round up to nearest multiple of 4

/** @} */

typedef float      U_FLOAT;

typedef uint32_t   U_CBBITS;                // Describes byte count of TYPE
typedef uint32_t   U_CBBITSMSK;
typedef uint32_t   U_CBBITSSRC;
typedef uint32_t   U_CBBMI;
typedef uint32_t   U_CBBMIMSK;
typedef uint32_t   U_CBBMISRC;
typedef uint32_t   U_CBDATA;
typedef uint32_t   U_CBNAME;
typedef uint32_t   U_CBPLENTRIES;
typedef uint32_t   U_CBPXLFMT;
typedef uint32_t   U_CBRGNDATA;
typedef uint32_t   U_CBSTR;                 // bytes in an 8 or 16 bit string

typedef uint32_t   U_OFFBITS;               // Describes byte offset to TYPE, always measured from the start of the RECORD (not the struct)
typedef uint32_t   U_OFFBITSMSK;
typedef uint32_t   U_OFFBITSSRC;
typedef uint32_t   U_OFFBMI;
typedef uint32_t   U_OFFBMIMSK;
typedef uint32_t   U_OFFBMISRC;
typedef uint32_t   U_OFFDATA;
typedef uint32_t   U_OFFDESC;
typedef uint32_t   U_OFFDX;
typedef uint32_t   U_OFFPLENTRIES;
typedef uint32_t   U_OFFPXLFMT;
typedef uint32_t   U_OFFSTR;                // String of either 8 or 16 bit characters
typedef uint8_t    U_DATA;                  // any binary sort of data, not otherwise classified.

//  "Types" For array components in structures, where not otherwise defined as a structure
typedef uint32_t   U_FNTAXES;               // Font Axes For U_DESIGNVECTOR 
typedef uint32_t   U_STYLEENTRY;            // StyleEntry For U_EXTLOGPEN
typedef uint32_t   U_POLYCOUNTS;            // aPolyCounts For U_EMRPOLYPOLYLINE etc.

//  "Counts" for array components in structures
typedef uint32_t   U_NUM_FNTAXES;           // Number of U_FNTAXES
typedef uint32_t   U_NUM_LOGPLTNTRY;        // Number of U_LOGPLTENTRY
typedef uint32_t   U_NUM_RECTL;             // Number of U_RECTL
typedef uint32_t   U_NUM_POINTL;            // Number of U_POINTL
typedef uint32_t   U_NUM_POINT16;           // Number of U_POINT16
typedef uint32_t   U_NUM_STYLEENTRY;        // Number of U_STYLEENTRY
typedef uint32_t   U_NUM_POLYCOUNTS;        // Number of U_POLYCOUNTS
typedef uint32_t   U_NUM_EMRTEXT;           // Number of U_EMRTEXT
typedef uint32_t   U_NUM_STR;               // Number of 8 or 16 bit characters in string
typedef uint32_t   U_NUM_TRIVERTEX;         // Number of U_TRIVERTEX
typedef uint32_t   U_NUM_GRADOBJ;           // Number of U_GRADIENT4 OR U_GRADIENT3 (determined at run time)
typedef uint32_t   U_NUM_RGBQUAD;           // Number of U_RGBQUAD (in bmciColors in U_BITMAPCOREINFO)


/**
  \brief Pair of values indicating x and y sizes.
  Microsoft name: SIZE Object
  Microsoft name: SIZEL Object
*/
typedef struct {
    int32_t cx;                             //!< X size
    int32_t cy;                             //!< Y size
} U_SIZE, U_SIZEL, *PU_SIZE, *PU_SIZEL;

/**
  \brief Used for any generic pair of floats
  Microsoft name: (none)
*/
typedef struct {
    float x;                              //!< X value
    float y;                              //!< Y value
} U_PAIRF, *PU_PAIRF;

/**
  \brief Used for any generic pair of uint32_t
  Microsoft name: POINT Object
*/
typedef struct {
    int32_t x;                              //!< X value
    int32_t y;                              //!< Y value
} U_PAIR, *PU_PAIR, U_POINT, *PU_POINT, U_POINTL, *PU_POINTL;

/**
  \brief Point type for 16 bit EMR drawing functions.
  Microsoft name: POINTS Object
  Microsoft name: POINTS16 Object
*/
typedef struct {
    int16_t x;                              //!< X size (16 bit)
    int16_t y;                              //!< Y size (16 bit)
} U_POINT16, *PU_POINT16;

/**
   \brief Coordinates of the upper left, lower right corner.
   Note that the coordinate system is 0,0 in the upper left corner
   of the screen an N,M in the lower right corner.
   Microsoft name: RECT Object
*/
typedef struct {
    int32_t  left;                          //!< left coordinate
    int32_t  top;                           //!< top coordinate
    int32_t  right;                         //!< right coordinate
    int32_t  bottom;                        //!< bottom coordinate
} U_RECT, *PU_RECT,
  U_RECTL, *PU_RECTL;

#define U_RCL_DEF (U_RECTL){0,0,-1,-1}  //!< Use this when no bounds are needed. 

/* ************************************************************
    EMF structures OTHER than those corresponding to complete U_EMR_* records
   ************************************************************ */

/**
  \brief For U_BITMAPINFO bmiColors field
  NOTE that the color order is BGR, even though the name is RGB!
  Microsoft name: RGBQUAD Object
*/
typedef struct {
    uint8_t             Blue;               //!< Blue  color (0-255) 
    uint8_t             Green;              //!< Green color (0-255)
    uint8_t             Red;                //!< Red   color (0-255)
    uint8_t             Reserved;           //!< Not used
} U_RGBQUAD, *PU_RGBQUAD;

/**
  \brief For U_BITMAPINFO crColor field
  NOTE that the color order is RGB reserved, flipped around from the preceding.
  Microsoft name: COLORREF Object
*/
typedef struct {
    uint8_t             Red;                //!< Red   color (0-255)
    uint8_t             Green;              //!< Green color (0-255)
    uint8_t             Blue;               //!< Blue  color (0-255) 
    uint8_t             Reserved;           //!< Not used
} U_COLORREF, *PU_COLORREF;

/**
   \brief For U_POINT28_4 x and y fields.
   Microsoft name: BitFIX28_4 Object.
*/
typedef struct {
    signed              IntValue :28;       //!< Signed integral bit field
    unsigned            FracValue :4;       //!< Unsigned integral bit field
} U_BITFIX28_4, *PU_BITFIX28_4;

/**
   \brief For U_LCS_GAMMARGB lcsGamma* fields
   Microsoft name:(unknown) Object
*/
typedef struct {
    unsigned             ignoreHi :8;       //!< not used
    unsigned             intPart  :8;       //!< integer part
    unsigned             fracPart :8;       //!< fraction part
    unsigned             ignoreLo :8;       //!< not used
} U_LCS_GAMMA, *PU_LCS_GAMMA;

/** 
  \brief For U_LOGCOLORSPACEA and U_LOGCOLORSPACEW lcsGammaRGB field
  Microsoft name:(unknown) Object
*/
typedef struct {
    U_LCS_GAMMA         lcsGammaRed;        //!< Red   Gamma
    U_LCS_GAMMA         lcsGammaGreen;      //!< Green Gamma
    U_LCS_GAMMA         lcsGammaBlue;       //!< Blue  Gamma
} U_LCS_GAMMARGB, *PU_LCS_GAMMARGB;

/** 
  \brief For U_EMRSETOLORADJUSTMENT ColorAdjustment field
  Note, range constants are: RGB_GAMMA_[MIN|MAX],REFERENCE_[WHITE|BLACK]_[MIN|MAX],COLOR_ADJ_[MIN|MAX]
  Microsoft name: ColorAdjustment Object
*/
typedef struct {
    uint16_t            caSize;             //!< Size of this structure in bytes
    uint16_t            caFlags;            //!< ColorAdjustment Enumeration
    uint16_t            caIlluminantIndex;  //!< Illuminant Enumeration
    uint16_t            caRedGamma;         //!< Red   Gamma correction (range:2500:65000, 10000 is no correction)
    uint16_t            caGreenGamma;       //!< Green Gamma correction (range:2500:65000, 10000 is no correction)
    uint16_t            caBlueGamma;        //!< Blue  Gamma correction (range:2500:65000, 10000 is no correction)
    uint16_t            caReferenceBlack;   //!< Values less than this are black (range:0:4000)
    uint16_t            caReferenceWhite;   //!< Values more than this are white (range:6000:10000)
    int16_t             caContrast;         //!< Contrast     adjustment (range:-100:100, 0 is no correction)
    int16_t             caBrightness;       //!< Brightness   adjustment (range:-100:100, 0 is no correction)
    int16_t             caColorfulness;     //!< Colorfulness adjustment (range:-100:100, 0 is no correction)
    int16_t             caRedGreenTint;     //!< Tine         adjustment (range:-100:100, 0 is no correction)
} U_COLORADJUSTMENT, *PU_COLORADJUSTMENT;

/**
  \brief For ? (not implemented yet)
  Microsoft name: DesignVector Object
*/
typedef struct {
    uint32_t            Signature;          //!< Must be 0x08007664 (AKA: DV_SGNTR)
    U_NUM_FNTAXES       NumAxes;            //!< Number of elements in Values, 0-16
    U_FNTAXES           Values[1];          //!< Optional. Array of font axes for opentype font
} U_DESIGNVECTOR,*PU_DESIGNVECTOR; 

/**
  \brief For U_EMR_COMMENT_MULTIFORMATS record, where an array of these is used
  Microsoft name: EmrFormat Object
*/
typedef struct {
    uint32_t            signature;          //!< FormatSignature Enumeration
    uint32_t            nVersion;           //!< Must be 1 if signature is EPS, else ignored
    U_CBDATA            cbData;             //!< Data size in bytes
    U_OFFDATA           offData;            //!< Offset in bytes to the Data from the start of the RECORD
} U_EMRFORMAT, *PU_EMRFORMAT;

/**
  \brief For U_EMR[POLY]EXTTEXTOUT[A|W] emrtext field
  Differs from implementation in Mingw and Wine in that the core struct has a fixed size.  
  Optional and movable components must be handled with offsets.
  Microsoft name: EmrText Object
  Following invariant core there may/must be:
  U_RECTL             rcl;                (Optional, absent when fOptions & U_ETO_NO_RECT) grayed/clipping/opaque rectangle

  U_OFFDX             offDx;              (required) but position isn't static.  Offset in bytes to the character spacing array measured
                                          from the start of the RECORD, NOT from the start of this structure.
  
  The order of the next two may be reversed, they are found from their offsets.
  
  char                string              (required) String buffer holding nChars (padded to a multiple of 4 bytes in length).

  uint32_t            Dx[1]               (required) character spacing, array with one entry per glyph.
  
*/
typedef struct {
    U_POINTL            ptlReference;       //!< String start coordinates
    U_NUM_STR           nChars;             //!< Number of characters in the string
    U_OFFSTR            offString;          //!< Offset in bytes to the string from the start of the RECORD
    uint32_t            fOptions;           //!< ExtTextOutOptions Enumeration
} U_EMRTEXT, *PU_EMRTEXT;

/**
  \brief For U_EPS_DATA Points field
  Microsoft name: Point28_4 Object
*/
typedef struct {
    U_BITFIX28_4        x;                  //!< X coordinate
    U_BITFIX28_4        y;                  //!< Y coordinate
} U_POINT28_4, *PU_POINT28_4;

/**
  \brief For embedding EPS in EMF via U_EMRFORMAT offData array in U_EMR_COMMENT_MULTIFORMATS
  Microsoft name: EpsData Object
*/
typedef struct {
    uint32_t            sizeData;           //!< Size in bytes of this object
    uint32_t            version;            //!< Must be 1
    U_POINT28_4         Points[3];          //!< Defines parallelogram, UL, UR, LL corners, LR is derived.
    U_RECTL             PostScriptData;     //!< Record may include optional clipping/opaque rectangle
} U_EPS_DATA, *PU_EPS_DATA;

/**
  \brief For GRADIENT_[TRIANGLE|U_RECT]
  Microsoft name: TriVertex Object
*/
typedef struct {
    int32_t             x;                  //!< X coord
    int32_t             y;                  //!< Y coord
    uint16_t            Red;                //!< Red   component
    uint16_t            Green;              //!< Green component
    uint16_t            Blue;               //!< Bule  component
    uint16_t            Alpha;              //!< Alpha Transparency
} U_TRIVERTEX, *PU_TRIVERTEX;


/**
  \brief For U_EMRGRADIENTFILL GradObj field

  Gradient object notes.  The next two structures are used to define the shape with reference to an existing array
  of points stored in an array of TriVertex objects in the U_EMRGRADIENTFILL record.  The tricky part
  is that these two structures are different sizes.  In some implementations (MingW) the array is cast to uint32_t
  and basically the cast is then ignored.  For libUEMF we leave this out of the structure entirely and get to it with offsets.

  Microsoft name: GradientTriangle Object
*/
typedef struct {
    uint32_t            Vertex1;            //!< Index of Vertex1 in an array of U_TRIVERTEX objects
    uint32_t            Vertex2;            //!< Index of Vertex2 in an array of U_TRIVERTEX objects
    uint32_t            Vertex3;            //!< Index of Vertex3 in an array of U_TRIVERTEX objects
} U_GRADIENT3, *PU_GRADIENT3;

/**
  \brief For U_EMRGRADIENTFILL GradObj field
  Microsoft name: GradientRectangle Object
*/
typedef struct {
    uint32_t            UpperLeft;          //!< Index of UL corner in an array of U_TRIVERTEX objects
    uint32_t            LowerRight;         //!< Index of LR corner in an array of U_TRIVERTEX objects
} U_GRADIENT4, *PU_GRADIENT4; 

/**
  \brief For U_EMRCREATEBRUSHINDIRECT lb field
  Microsoft name: LogBrushEx Object
*/
typedef struct {                            //!< In MS documentation this is LogBrushEx Object
    uint32_t            lbStyle;            //!< LB_Style Enumeration
    U_COLORREF          lbColor;            //!< Brush color
    uint32_t            lbHatch;            //!< HatchStyle Enumeration
} U_LOGBRUSH, *PU_LOGBRUSH;
typedef U_LOGBRUSH U_PATTERN, *PU_PATTERN;

/**
  \brief For U_LOGFONT_PANOSE elfLogFont field
  Microsoft name: LogFont Object
*/
typedef struct {
    int32_t             lfHeight;           //!< Height in Logical units
    int32_t             lfWidth;            //!< Average Width in Logical units
    int32_t             lfEscapement;       //!< Angle in 0.1 degrees betweem escapement vector and X axis
    int32_t             lfOrientation;      //!< Angle in 0.1 degrees between baseline and X axis
    int32_t             lfWeight;           //!< LF_Weight Enumeration
    uint8_t             lfItalic;           //!< LF_Italic Enumeration
    uint8_t             lfUnderline;        //!< LF_Underline Enumeration
    uint8_t             lfStrikeOut;        //!< LF_StrikeOut Enumeration
    uint8_t             lfCharSet;          //!< LF_CharSet Enumeration
    uint8_t             lfOutPrecision;     //!< LF_OutPrecision Enumeration
    uint8_t             lfClipPrecision;    //!< LF_ClipPrecision Enumeration
    uint8_t             lfQuality;          //!< LF_Quality Enumeration
    uint8_t             lfPitchAndFamily;   //!< LF_PitchAndFamily Enumeration
    uint16_t            lfFaceName[U_LF_FACESIZE]; //!< Name of font.  If <U_LF_FACESIZE chars must be null terminated
} U_LOGFONT, *PU_LOGFONT;

/**
  \brief For U_LOGFONT_PANOSE elfPanose field
  Microsoft name: Panose Object
*/
typedef struct {
    uint8_t             bFamilyType;        //!< FamilyType Enumeration
    uint8_t             bSerifStyle;        //!< SerifType Enumeration
    uint8_t             bWeight;            //!< Weight Enumeration
    uint8_t             bProportion;        //!< Proportion Enumeration
    uint8_t             bContrast;          //!< Contrast Enumeration
    uint8_t             bStrokeVariation;   //!< StrokeVariation Enumeration
    uint8_t             bArmStyle;          //!< ArmStyle Enumeration
    uint8_t             bLetterform;        //!< Letterform Enumeration
    uint8_t             bMidline;           //!< Midline Enumeration
    uint8_t             bXHeight;           //!< XHeight Enumeration
} U_PANOSE, *PU_PANOSE;

#define    U_PAN_ALL0  (U_PANOSE){0,0,0,0,0,0,0,0,0,0}  // all U_PAN_ANY, have not seen this in an EMF file
#define    U_PAN_ALL1  (U_PANOSE){1,1,1,1,1,1,1,1,1,1}  // all U_PAN_NO_FIT, this is what createfont() would have made

// Microsoft name: LogFontEx Object (not implemented)
// Microsoft name: LogFontExDv Object (not implemented)

/**
  \brief For U_EMREXTCREATEFONTINDIRECTW elfw field
  Microsoft name: LogFont_Panose Object
*/
typedef struct {
    U_LOGFONT           elfLogFont;                     //!< Basic font attributes
    uint16_t            elfFullName[U_LF_FULLFACESIZE]; //!< Font full name
    uint16_t            elfStyle[U_LF_FACESIZE];        //!< Font style (if <U_LF_FACESIZE characters, null terminate string)
    uint32_t            elfVersion;                     //!< Ignore
    uint32_t            elfStyleSize;                   //!< Font hinting starting at this point size, if 0, starts at Height
    uint32_t            elfMatch;                       //!< Ignore
    uint32_t            elfReserved;                    //!< Must be 0, Ignore
    uint8_t             elfVendorId[U_ELF_VENDOR_SIZE]; //!< Ignore
    uint32_t            elfCulture;                     //!< Must be 0, Ignore
    U_PANOSE            elfPanose;                      //!< Panose Object. If all zero, it is ignored.
    uint16_t            elfPadding;                     //!< Ignore
} U_LOGFONT_PANOSE, *PU_LOGFONT_PANOSE;

/**
  \brief  For U_LOGPALETTE palPalEntry field(s)
  Microsoft name: LogPaletteEntry Object
*/
typedef struct {
    uint8_t             peReserved;         //!< Ignore
    uint8_t             peRed;              //!< Palette entry Red Intensity
    uint8_t             peGreen;            //!< Palette entry Green Intensity
    uint8_t             peBlue;             //!< Palette entry Blue Intensity
} U_LOGPLTNTRY, *PU_LOGPLTNTRY;

/**
  \brief  For U_EMRCREATEPALETTE lgpl field
  Microsoft name: LogPalette Object
*/
typedef struct { 
    uint16_t            palVersion;         //!< Must be  0x0300 (AKA: U_LP_VERSION)
    uint16_t            palNumEntries;      //!< Number of U_LOGPLTNTRY objects
    U_LOGPLTNTRY        palPalEntry[1];     //!< PC_Entry Enumeration
} U_LOGPALETTE, *PU_LOGPALETTE;

/**
  \brief For U_EMRCREATEPEN lopn field
  Microsoft name: LogPen Object
*/
typedef struct {
    uint32_t            lopnStyle;          //!< PenStyle Enumeration
    U_POINT             lopnWidth;          //!< Width of pen set by X, Y is ignored
    U_COLORREF          lopnColor;          //!< Pen color value
} U_LOGPEN, *PU_LOGPEN;

// Microsoft name: LogPenEx Object (not implemented)

/**
  \brief  For U_EMRPIXELFORMAT pfd field
  Microsoft name: PixelFormatDescriptor Object
*/
typedef struct {
    uint16_t            nSize;              //!< Structure size in bytes
    uint16_t            nVersion;           //!< must be 1
    uint32_t            dwFlags;            //!< PFD_dwFlags Enumeration
    uint8_t             iPixelType;         //!< PFD_iPixelType Enumeration
    uint8_t             cColorBits;         //!< RGBA: total bits per pixel
    uint8_t             cRedBits;           //!< Red   bits per pixel
    uint8_t             cRedShift;          //!< Red   shift to data bits
    uint8_t             cGreenBits;         //!< Green bits per pixel
    uint8_t             cGreenShift;        //!< Green shift to data bits
    uint8_t             cBlueBits;          //!< Blue  bits per pixel
    uint8_t             cBlueShift;         //!< Blue  shift to data bits
    uint8_t             cAlphaBits;         //!< Alpha bits per pixel
    uint8_t             cAlphaShift;        //!< Alpha shift to data bits
    uint8_t             cAccumBits;         //!< Accumulator buffer, total bitplanes
    uint8_t             cAccumRedBits;      //!< Red   accumulator buffer bitplanes
    uint8_t             cAccumGreenBits;    //!< Green accumulator buffer bitplanes
    uint8_t             cAccumBlueBits;     //!< Blue  accumulator buffer bitplanes
    uint8_t             cAccumAlphaBits;    //!< Alpha accumulator buffer bitplanes
    uint8_t             cDepthBits;         //!< Depth of Z-buffer
    uint8_t             cStencilBits;       //!< Depth of stencil buffer
    uint8_t             cAuxBuffers;        //!< Depth of auxilliary buffers (not supported)
    uint8_t             iLayerType;         //!< PFD_iLayerType Enumeration, may be ignored
    uint8_t             bReserved;          //!< Bits 0:3/4:7 are number of Overlay/Underlay planes 
    uint32_t            dwLayerMask;        //!< may be ignored
    uint32_t            dwVisibleMask;      //!< color or index of underlay plane
    uint32_t            dwDamageMask;       //!< may be ignored
} U_PIXELFORMATDESCRIPTOR, *PU_PIXELFORMATDESCRIPTOR;

/**
  \brief  For U_RGNDATA rdb field
  Microsoft name: RegionDataHeader Object (RGNDATAHEADER)
*/
typedef struct {
    uint32_t            dwSize;             //!< Size in bytes, must be 0x20 (AKA: U_RDH_OBJSIZE)
    uint32_t            iType;              //!< Must be 1 (AKA: U_RDH_RECTANGLES)
    U_NUM_RECTL         nCount;             //!< Number of rectangles in region
    uint32_t            nRgnSize;           //!< Size in bytes of rectangle buffer
    U_RECTL             rclBounds;          //!< Region bounds
} U_RGNDATAHEADER,*PU_RGNDATAHEADER;

/**
  \brief For U_EMRFILLRGN RgnData field(s)
  Microsoft name: RegionData Object
*/
typedef struct {
    U_RGNDATAHEADER     rdh;                //!< Data description
    U_RECTL             Buffer[1];          //!< Array of U_RECTL elements
} U_RGNDATA,*PU_RGNDATA;

// Microsoft name: UniversalFontId Object (not implemented)

/**
  \brief  For U_EMR[FILLRGN|STRETCHBLT|MASKBLT|PLGBLT] xformSrc field
  Microsoft name: Xform Object
*/
typedef struct {
    U_FLOAT             eM11;               //!< Matrix element M11
    U_FLOAT             eM12;               //!< Matrix element M12
    U_FLOAT             eM21;               //!< Matrix element M21
    U_FLOAT             eM22;               //!< Matrix element M22
    U_FLOAT             eDx;                //!< X offset in logical units
    U_FLOAT             eDy;                //!< Y offset in logical units
} U_XFORM , *PU_XFORM;

/**
  \brief For U_CIEXYZTRIPLE (all) fields
  Microsoft name: CIEXYZ Object
*/
typedef struct {
    int32_t             ciexyzX;            //!< CIE color space X component
    int32_t             ciexyzY;            //!< CIE color space Y component
    int32_t             ciexyzZ;            //!< CIE color space Z component
} U_CIEXYZ, *PU_CIEXYZ;

/**
  \brief For U_LOGCOLORSPACEA and U_LOGCOLORSPACEW lcsEndpints field
  defines a CIE colorspace
  Microsoft name: CIEXYZTRIPLE Object
*/
typedef struct {
    U_CIEXYZ            ciexyzRed;          //!< CIE XYZ coord of red   endpoint of colorspace
    U_CIEXYZ            ciexyzGreen;        //!< CIE XYZ coord of green endpoint of colorspace
    U_CIEXYZ            ciexyzBlue;         //!< CIE XYZ coord of blue  endpoint of colorspace
} U_CIEXYZTRIPLE, *PU_CIEXYZTRIPLE;

/**
  \brief For U_EMRCREATECOLORSPACE lcs field
  Microsoft name: LOGCOLORSPACEA Object
*/
typedef struct {
    uint32_t            lcsSignature;       //!< must be U_LCS_SIGNATURE
    uint32_t            lcsVersion;         //!< must be U_LCS_VERSION
    uint32_t            lcsSize;            //!< Size in bytes of this structure
    int32_t             lcsCSType;          //!< LCS_CSType Enumeration
    int32_t             lcsIntent;          //!< LCS_Intent Enumeration
    U_CIEXYZTRIPLE      lcsEndpoints;       //!< CIE XYZ color space endpoints
    U_LCS_GAMMARGB      lcsGammaRGB;        //!< Gamma For RGB
    char                lcsFilename[U_MAX_PATH];  //!< Names an external color profile file, otherwise empty string
} U_LOGCOLORSPACEA, *PU_LOGCOLORSPACEA;

/**
  \brief For U_EMRCREATECOLORSPACEW lcs field
  Microsoft name: LOGCOLORSPACEW Object
*/
typedef struct {
    uint32_t            lcsSignature;       //!< must be U_LCS_SIGNATURE
    uint32_t            lcsVersion;         //!< must be U_LCS_VERSION
    uint32_t            lcsSize;            //!< Size in bytes of this structure
    int32_t             lcsCSType;          //!< lcsCSType Enumeration
    int32_t             lcsIntent;          //!< lcsIntent Enumeration
    U_CIEXYZTRIPLE      lcsEndpoints;       //!< CIE XYZ color space endpoints
    U_LCS_GAMMARGB      lcsGammaRGB;        //!< Gamma For RGB
    uint16_t            lcsFilename[U_MAX_PATH];  //!< Could name an external color profile file, otherwise empty string
} U_LOGCOLORSPACEW, *PU_LOGCOLORSPACEW;

/**
  \brief For U_EMREXTCREATEPEN lopn field
  Microsoft name: EXTLOGPEN Object
*/
typedef struct {
    uint32_t            elpPenStyle;        //!< PenStyle Enumeration
    uint32_t            elpWidth;           //!< Width in logical units (elpPenStyle & U_PS_GEOMETRIC) or 1 (pixel)
    uint32_t            elpBrushStyle;      //!< LB_Style Enumeration
    U_COLORREF          elpColor;           //!< Pen color
    uint32_t            elpHatch;           //!< HatchStyle Enumeration
    U_NUM_STYLEENTRY    elpNumEntries;      //!< Count of StyleEntry array
    U_STYLEENTRY        elpStyleEntry[1];   //!< Array of StyleEntry (For user specified dot/dash patterns)
} U_EXTLOGPEN, *PU_EXTLOGPEN; 

/**
  \brief For U_BITMAPINFO bmiHeader field
  Microsoft name: BITMAPINFOHEADER Object
*/
typedef struct {
    uint32_t            biSize;             //!< Structure size in bytes
    int32_t             biWidth;            //!< Bitmap width in pixels
    int32_t             biHeight;           //!< Bitmap height in pixels, may be negative.
                                            //!< abs(biHeight) is bitmap height
                                            //!< bitmap may appear in two orientations:
                                            //!<   biHeight > 0 origin is LL corner, may be compressed, this is height after decompression.
                                            //!<   biHeight < 0 origin is UL corner, may not be compressed
    uint16_t            biPlanes;           //!< Planes (must be 1)
    uint16_t            biBitCount;         //!< BitCount Enumeration (determines number of RBG colors)
    uint32_t            biCompression;      //!< BI_Compression Enumeration
    uint32_t            biSizeImage;        //!< Image size in bytes or 0 = "default size (calculated from geometry?)"
    int32_t             biXPelsPerMeter;    //!< X Resolution in pixels/meter
    int32_t             biYPelsPerMeter;    //!< Y Resolution in pixels/meter
    U_NUM_RGBQUAD       biClrUsed;          //!< Number of bmciColors in U_BITMAPINFO/U_BITMAPCOREINFO that are used by the bitmap
    uint32_t            biClrImportant;     //!< Number of bmciColors needed (0 means all).
} U_BITMAPINFOHEADER, *PU_BITMAPINFOHEADER;

/**
  \brief For U_EMR_* OffBmi* fields
  Description of a Bitmap which in some cases is a Device Independent Bitmap (DIB)
  Microsoft name: BITMAPINFO Object
*/
typedef struct {
    U_BITMAPINFOHEADER  bmiHeader;          //!< Geometry and pixel properties
    U_RGBQUAD           bmiColors[1];       //!< Color table.  24 bit images do not use color table values.
} U_BITMAPINFO, *PU_BITMAPINFO;

/**
  \brief U_EMRALPHABLEND Blend field
*/
typedef struct {
  uint8_t Operation;  //!< Must be 0
  uint8_t Flags;      //!< Must be 0
  uint8_t Global;     //!< Alpha for whole thing if Op is U_AC_SRC_GLOBAL (AKA U_AC_SRC_GLOBAL)
  uint8_t Op;         //!< Blend Enumeration
} U_BLEND, *PU_BLEND;
#

/** 
    General form of an EMF record.
    Microsoft name: ENHMETARECORD Object
    For generic cast of other U_EMR_* records
*/
typedef struct {
    uint32_t            iType;              //!< Type of EMR record
    uint32_t            nSize;              //!< Size of entire record in bytes (multiple of 4).
    uint32_t            dParm[1];           //!< Data in record
} U_ENHMETARECORD, *PU_ENHMETARECORD; 

/** First two fields of all EMF records
    For accessing iType and nSize files in all U_EMR* records
    Microsoft name: EMR Object
*/
typedef struct {
    uint32_t            iType;              //!< Type of EMR record
    uint32_t            nSize;              //!< Size of entire record in bytes (multiple of 4).
} U_EMR, *PU_EMR;

typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_PAIR              pair;               //!< pair of 32 bit values
} U_EMRGENERICPAIR,   *PU_EMRGENERICPAIR;




// ***********************************************************************************
// The following have U_EMR_# records

/* Index  1 */
/** 
   \brief The firstU_ENHMETARECORD record in the metafile.  
   
   Microsoft names instead: Header, HeaderExtension1, and HeaderExtension2 objects.  These are
   used nowhere else, so they are combined here, along with the first two fields which were not listed in the Header.
   
   Note also that three fields in this file (nBytes, nRecords, nHandles) must be (re)set after the entire EMF
   is constructed, since typically they are not known until then.  bOpenGL may or may not be knowable when this
   header is written.
   
   Note also that rclBounds and rclFrame are supposed to be the region bounding the drawn content within the 
   EMF.  This is generally smaller than the size from szlDevice.  However, since libUEMF does not actually draw
   anything it has no way of knowing what these values are.  Instead when it creates a header it sets these to 
   match the szl* fields.
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR  
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units 
    U_RECTL             rclFrame;           //!< Bounding rectangle in 0.01 mm units
    uint32_t            dSignature;         //!< FormatSignature Enumeration  (must be U_ENHMETA_SIGNATURE)  
    uint32_t            nVersion;           //!< Must be U_ENHMETA_VERSION (0x00010000)
    uint32_t            nBytes;             //!< Length in bytes of the Metafile  
    uint32_t            nRecords;           //!< Records in the Metafile  
    uint16_t            nHandles;           //!< Number of graphics objects used in the Metafile   
    uint16_t            sReserved;          //!< Must be 0   
    uint32_t            nDescription;       //!< Characters in the Description field, 0 if no description  
    uint32_t            offDescription;     //!< Offset in bytes to Description field 
    uint32_t            nPalEntries;        //!< Number of Palette entries (in U_EMR_EOF record). 
    U_SIZEL             szlDevice;          //!< Reference device size in pixels  
    U_SIZEL             szlMillimeters;     //!< Reference device size in 0.01 mm
    /** Fields for winver >= win95 */
    U_CBPXLFMT          cbPixelFormat;      //!< Size in bytes of PixelFormatDescriptor, 0 if no PFD
    U_OFFPXLFMT         offPixelFormat;     //!< Offset in bytes to PixelFormatDescriptor from the start of the RECORD, 0 if no PFD
    uint32_t            bOpenGL;            //!< nonZero if OpenGL commands are included
    /** Fields for winver >= win98 */ 
    U_SIZEL             szlMicrometers;     //!< Size of the display device in micrometer
                                            //!< Record may include optional Description, UTF-16BE string
                                            //!< Record may include optional PxlFmtDescriptor, U_PIXELFORMATDESCRIPTOR
} U_EMRHEADER, *PU_EMRHEADER; 

/* Index  2,3,4,5,6*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< bounding rectangle in device units
    U_NUM_POINTL        cptl;               //!< Number of points to draw
    U_POINTL            aptl[1];            //!< array of points
} U_EMRPOLYBEZIER,   *PU_EMRPOLYBEZIER,
  U_EMRPOLYGON,      *PU_EMRPOLYGON,
  U_EMRPOLYLINE,     *PU_EMRPOLYLINE,
  U_EMRPOLYBEZIERTO, *PU_EMRPOLYBEZIERTO,
  U_EMRPOLYLINETO,   *PU_EMRPOLYLINETO;

/* Index  7,8 */
typedef struct {
    U_EMR               emr;                //!< U_EMR     
    U_RECTL             rclBounds;          //!< bounding rectangle in device units
    U_NUM_POLYCOUNTS    nPolys;             //!< Number of elements in aPolyCounts
    U_NUM_POINTL        cptl;               //!< Total number of points (over all poly)
    U_POLYCOUNTS        aPolyCounts[1];     //!< Number of points in each poly (sequential)
//  This will appear somewhere but is not really part of the core structure.
//    U_POINTL            aptl[1];            //!< array of points
} U_EMRPOLYPOLYLINE, *PU_EMRPOLYPOLYLINE,
  U_EMRPOLYPOLYGON,  *PU_EMRPOLYPOLYGON;

/* Index  9,11 (numbers interleave with next one) */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_SIZEL             szlExtent;          //!< H & V extent in logical units
} U_EMRSETWINDOWEXTEX,   *PU_EMRSETWINDOWEXTEX,
  U_EMRSETVIEWPORTEXTEX, *PU_EMRSETVIEWPORTEXTEX;

/* Index  10,12,13 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_POINTL            ptlOrigin;          //!< H & V origin in logical units
} U_EMRSETWINDOWORGEX,   *PU_EMRSETWINDOWORGEX,
  U_EMRSETVIEWPORTORGEX, *PU_EMRSETVIEWPORTORGEX,
  U_EMRSETBRUSHORGEX,    *PU_EMRSETBRUSHORGEX;

/* Index  14 
*/
/**
This is a very odd structure because the nSizeLast follows an optional variable size field.  Consequently
even though nSizeLast has a name it cannot actually be accessed by it!  Following the core appear these fields:

 U_LOGPLTNTRY        PalEntries[1];      Record may include optional array of PalEntries

     uint32_t            nSizeLast;      Mandatory, but position isn't fixed. Must have same value as emr.nSize in header record
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_CBPLENTRIES       cbPalEntries;       //!< Number of palette entries
    U_OFFPLENTRIES      offPalEntries;      //!< Offset in bytes to array of palette entries
} U_EMREOF, *PU_EMREOF;

/* Index  15 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_POINTL            ptlPixel;           //!< Pixel coordinates (logical)
    U_COLORREF          crColor;            //!< Pixel color
} U_EMRSETPIXELV, *PU_EMRSETPIXELV;

/* Index  16 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            dwFlags;            //!< must be 1
} U_EMRSETMAPPERFLAGS, *PU_EMRSETMAPPERFLAGS;

/* Index  17,18,19,20,21,22,67,98,115 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            iMode;              //!< enumeration varies with type
} U_EMRSETMAPMODE,        *PU_EMRSETMAPMODE,         //!< MapMode enumeration
  U_EMRSETBKMODE,         *PU_EMRSETBKMODE,          //!< BackgroundMode Enumeration
  U_EMRSETPOLYFILLMODE,   *PU_EMRSETPOLYFILLMODE,    //!< PolygonFillMode Enumeration
  U_EMRSETROP2,           *PU_EMRSETROP2,            //!< Binary Raster Operation Enumeration
  U_EMRSETSTRETCHBLTMODE, *PU_EMRSETSTRETCHBLTMODE,  //!< StretchMode Enumeration
  U_EMRSETTEXTALIGN,      *PU_EMRSETTEXTALIGN,       //!< TextAlignment enumeration
  U_EMRSELECTCLIPPATH,    *PU_EMRSELECTCLIPPATH,     //!< RegionMode Enumeration
  U_EMRSETICMMODE,        *PU_EMRSETICMMODE,         //!< ICMMode Enumeration
  U_EMRSETLAYOUT,         *PU_EMRSETLAYOUT;          //!< Mirroring Enumeration

/* Index  23 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_COLORADJUSTMENT   ColorAdjustment;    //!< Color Adjustment
} U_EMRSETCOLORADJUSTMENT, *PU_EMRSETCOLORADJUSTMENT;

/* Index  24, 25 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_COLORREF          crColor;            //!< Color
} U_EMRSETTEXTCOLOR, *PU_EMRSETTEXTCOLOR,
  U_EMRSETBKCOLOR,   *PU_EMRSETBKCOLOR;

/* Index  26 */
typedef struct {
    U_EMR               emr;                //!< U_EMR           
    U_POINTL            ptlOffset;          //!< Clipping region 
} U_EMROFFSETCLIPRGN, *PU_EMROFFSETCLIPRGN;

/* Index  27, 54 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_POINTL            ptl;                //!< Point coordinates
} U_EMRMOVETOEX, *PU_EMRMOVETOEX,
  U_EMRLINETO,   *PU_EMRLINETO;

/* Index  28,33,52,59,60,61,65,66,68 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
}
  U_EMRSETMETARGN,     *PU_EMRSETMETARGN,
  U_EMRSAVEDC,         *PU_EMRSAVEDC,
  U_EMRREALIZEPALETTE, *PU_EMRREALIZEPALETTE,
  U_EMRBEGINPATH,      *PU_EMRBEGINPATH,
  U_EMRENDPATH,        *PU_EMRENDPATH,
  U_EMRCLOSEFIGURE,    *PU_EMRCLOSEFIGURE,
  U_EMRFLATTENPATH,    *PU_EMRFLATTENPATH,
  U_EMRWIDENPATH,      *PU_EMRWIDENPATH,
  U_EMRABORTPATH,      *PU_EMRABORTPATH;

/* Index  29,30 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclClip;            //!< Clipping Region
} U_EMREXCLUDECLIPRECT,   *PU_EMREXCLUDECLIPRECT,
  U_EMRINTERSECTCLIPRECT, *PU_EMRINTERSECTCLIPRECT;

/* Index  31,32 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    int32_t             xNum;               //!< Horizontal multiplier (!=0)
    int32_t             xDenom;             //!< Horizontal divisor    (!=0) 
    int32_t             yNum;               //!< Vertical   multiplier (!=0)
    int32_t             yDenom;             //!< Vertical   divisor    (!=0)
} U_EMRSCALEVIEWPORTEXTEX, *PU_EMRSCALEVIEWPORTEXTEX,
  U_EMRSCALEWINDOWEXTEX,   *PU_EMRSCALEWINDOWEXTEX;

/* Index  33  (see 28) */

/* Index  34 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    int32_t             iRelative;          //!< DC to restore. -1 is preceding
} U_EMRRESTOREDC, *PU_EMRRESTOREDC;

/* Index  35 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_XFORM             xform;              //!< Transform
} U_EMRSETWORLDTRANSFORM, *PU_EMRSETWORLDTRANSFORM;

/* Index  36 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_XFORM             xform;              //!< Transform
    uint32_t            iMode;              //!< ModifyWorldTransformMode Enumeration
} U_EMRMODIFYWORLDTRANSFORM, *PU_EMRMODIFYWORLDTRANSFORM;

/* Index  37,40 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihObject;           //!< Number of a stock or created object
} U_EMRDELETEOBJECT, *PU_EMRDELETEOBJECT,
  U_EMRSELECTOBJECT, *PU_EMRSELECTOBJECT;

/* Index  38 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihPen;              //!< Index to place object in EMF object table (this entry must not yet exist)
    U_LOGPEN            lopn;               //!< Pen properties
} U_EMRCREATEPEN, *PU_EMRCREATEPEN;

/* Index  39 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihBrush;            //!< Index to place object in EMF object table (this entry must not yet exist)
    U_LOGBRUSH          lb;                 //!< Brush properties
} U_EMRCREATEBRUSHINDIRECT, *PU_EMRCREATEBRUSHINDIRECT;

/* Index  40 see 37 */

/* Index  41 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_POINTL            ptlCenter;          //!< Center in logical units
    uint32_t            nRadius;            //!< Radius in logical units
    U_FLOAT             eStartAngle;        //!< Starting angle in degrees (counter clockwise from x axis)
    U_FLOAT             eSweepAngle;        //!< Sweep angle in degrees
} U_EMRANGLEARC, *PU_EMRANGLEARC;

/* Index  42,43 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBox;             //!< bounding rectangle in logical units
} U_EMRELLIPSE,   *PU_EMRELLIPSE,
  U_EMRRECTANGLE, *PU_EMRRECTANGLE;

/* Index  44 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBox;             //!< bounding rectangle in logical units
    U_SIZEL             szlCorner;          //!< W & H in logical units of ellipse used to round corner
} U_EMRROUNDRECT, *PU_EMRROUNDRECT;

/* Index  45, 46 ,47, 55  */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBox;             //!< bounding rectangle in logical units
    U_POINTL            ptlStart;           //!< Start point in logical units
    U_POINTL            ptlEnd;             //!< End point in logical units
} U_EMRARC,   *PU_EMRARC,
  U_EMRCHORD, *PU_EMRCHORD,
  U_EMRPIE,   *PU_EMRPIE,
  U_EMRARCTO, *PU_EMRARCTO;

/* Index  48 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihPal;              //!< Index of a Palette object in the EMF object table
} U_EMRSELECTPALETTE, *PU_EMRSELECTPALETTE;

/* Index  49 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihPal;              //!< Index to place object in EMF object table (this entry must not yet exist)
    U_LOGPALETTE        lgpl;               //!< Palette properties
} U_EMRCREATEPALETTE, *PU_EMRCREATEPALETTE;

/* Index  50 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihPal;              //!< Index of a Palette object in the EMF object table
    uint32_t            iStart;             //!< First Palette entry in selected object to set
    U_NUM_LOGPLTNTRY    cEntries;           //!< Number of Palette entries in selected object to set
    U_LOGPLTNTRY        aPalEntries[1];     //!< Values to set with
} U_EMRSETPALETTEENTRIES, *PU_EMRSETPALETTEENTRIES;

/* Index  51 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihPal;              //!< Index of a Palette object in the EMF object table
    uint32_t            cEntries;           //!< Number to expand or truncate the Palette entry list to.
} U_EMRRESIZEPALETTE, *PU_EMRRESIZEPALETTE;

/* Index  52  (see 28) */

/* Index  53 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_POINTL            ptlStart;           //!< Start point in logical units
    U_COLORREF          crColor;            //!< Color to fill with
    uint32_t            iMode;              //!< FloodFill Enumeration
} U_EMREXTFLOODFILL, *PU_EMREXTFLOODFILL;

/* Index  54  (see 27) */

/* Index  55 (see 45) */

/* Index  56 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_NUM_POINTL        cptl;               //!< Number of U_POINTL objects
    U_POINTL            aptl[1];            //!< Array of U_POINTL objects
    uint8_t             abTypes[1];         //!< Array of Point Enumeration 
} U_EMRPOLYDRAW, *PU_EMRPOLYDRAW; 

/* Index  57 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            iArcDirection;      //!< ArcDirection Enumeration
} U_EMRSETARCDIRECTION, *PU_EMRSETARCDIRECTION;

/* Index  58
IMPORTANT!!!!  The Microsoft structure uses a float for the miterlimit but the EMF file record
uses an unsigned int.  The latter form is used in this structure.
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            eMiterLimit;        //!< Miter limit (max value of mitered length / line width) 
} U_EMRSETMITERLIMIT, *PU_EMRSETMITERLIMIT;

/* Index  59,60,61  (see 28) */

/* Index  62,63,64 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
} U_EMRFILLPATH,          *PU_EMRFILLPATH,
  U_EMRSTROKEANDFILLPATH, *PU_EMRSTROKEANDFILLPATH,
  U_EMRSTROKEPATH,        *PU_EMRSTROKEPATH;

/* Index  65,66  (see 28) */
/* Index  67  (see 17) */
/* Index  68  (see 28) */
/* Index  69  (not a defined U_EMR record type ) */

/* Index  70 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_CBDATA            cbData;             //!< Number of bytes in comment
    uint8_t             Data[1];            //!< Comment (any binary data, interpretation is program specific)
} U_EMRCOMMENT, *PU_EMRCOMMENT;             //!< AKA GDICOMMENT

/* variant comment types */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_CBDATA            cbData;             //!< Number of bytes in comment
    uint32_t            cIdent;             //!< Comment identifier, must be U_EMR_COMMENT_EMFPLUSRECORD
    uint8_t             Data[1];            //!< EMF Plus record
} U_EMRCOMMENT_EMFPLUS, *PU_EMRCOMMENT_EMFPLUS;    //!< EMF Plus comment

typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_CBDATA            cbData;             //!< Number of bytes in comment
    uint32_t            cIdent;             //!< Comment identifier, must be U_EMR_COMMENT_SPOOL
    uint32_t            esrIdent;           //!< EMFSpoolRecordIdentifier, may be  U_EMR_COMMENT_SPOOLFONTDEF
    uint8_t             Data[1];            //!< EMF Spool records
} U_EMRCOMMENT_SPOOL, *PU_EMRCOMMENT_SPOOL;    //!< EMF Spool comment

typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_CBDATA            cbData;             //!< Number of bytes in comment
    uint32_t            cIdent;             //!< Comment identifier, must be U_EMR_COMMENT_PUBLIC
    uint32_t            pcIdent;            //!< Public Comment Identifier, from EMRComment Enumeration
    uint8_t             Data[1];            //!< Public comment data
} U_EMRCOMMENT_PUBLIC, *PU_EMRCOMMENT_PUBLIC;    //!< EMF Public comment

/* Index  71 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_CBRGNDATA         cbRgnData;          //!< Size in bytes of Region data
    uint32_t            ihBrush;            //!< Index of a Brush object in the EMF object table
    U_RGNDATA           RgnData[1];         //!< Variable size U_RGNDATA structure
} U_EMRFILLRGN, *PU_EMRFILLRGN;

/* Index  72 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_CBRGNDATA         cbRgnData;          //!< Size in bytes of Region data
    uint32_t            ihBrush;            //!< Index of a Brush object in the EMF object table
    U_SIZEL             szlStroke;          //!< W & H of Brush stroke
    U_RGNDATA           RgnData[1];         //!< Variable size U_RGNDATA structure
} U_EMRFRAMERGN, *PU_EMRFRAMERGN;

/* Index  73,74 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_CBRGNDATA         cbRgnData;          //!< Size in bytes of Region data
    U_RGNDATA           RgnData[1];         //!< Variable size U_RGNDATA structure
} U_EMRINVERTRGN, *PU_EMRINVERTRGN,
  U_EMRPAINTRGN,  *PU_EMRPAINTRGN;

/* Index  75 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_CBRGNDATA         cbRgnData;          //!< Size in bytes of Region data 
    uint32_t            iMode;              //!< RegionMode Enumeration       
    U_RGNDATA           RgnData[1];         //!< Variable size U_RGNDATA structure          
} U_EMREXTSELECTCLIPRGN, *PU_EMREXTSELECTCLIPRGN;

/* Index  76 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_POINTL            Dest;               //!< Destination UL corner in logical units
    U_POINTL            cDest;              //!< Destination width in logical units
    uint32_t            dwRop;              //!< Ternary Raster Operation enumeration
    U_POINTL            Src;                //!< Source retangle UL corner in logical units
    U_XFORM             xformSrc;           //!< Source bitmap transform (world to page coordinates)
    U_COLORREF          crBkColorSrc;       //!< Source bitmap background color
    uint32_t            iUsageSrc;          //!< DIBcolors Enumeration
    U_OFFBMISRC         offBmiSrc;          //!< Offset in bytes to U_BITMAPINFO (within bitmapbuffer)
    U_CBBMISRC          cbBmiSrc;           //!< Size   in bytes of U_BITMAPINFO
    U_OFFBITSSRC        offBitsSrc;         //!< Offset in bytes to the bitmap (within bitmapbuffer)
    U_CBBITS            cbBitsSrc;          //!< Size in bytes of bitmap
                                            //!< Record may include optional bitmapbuffer
} U_EMRBITBLT, *PU_EMRBITBLT;

/* Index  77 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_POINTL            Dest;               //!< Destination UL corner in logical units
    U_POINTL            cDest;              //!< Destination width in logical units
    uint32_t            dwRop;              //!< Ternary Raster Operation enumeration
    U_POINTL            Src;                //!< Source UL corner in logical units
    U_XFORM             xformSrc;           //!< Transform to apply to source
    U_COLORREF          crBkColorSrc;       //!< Background color
    uint32_t            iUsageSrc;          //!< DIBcolors Enumeration
    U_OFFBMISRC         offBmiSrc;          //!< Offset in bytes to U_BITMAPINFO (within bitmapbuffer)
    U_CBBMISRC          cbBmiSrc;           //!< Size   in bytes of U_BITMAPINFO
    U_OFFBITSSRC        offBitsSrc;         //!< Offset in bytes to the bitmap (within bitmapbuffer)
    U_CBBITS            cbBitsSrc;          //!< Size in bytes of bitmap
    U_POINTL            cSrc;               //!< Src W & H in logical units
                                            //!< Record may include optional bitmapbuffer
} U_EMRSTRETCHBLT, *PU_EMRSTRETCHBLT;

/* Index  78 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_POINTL            Dest;               //!< Destination UL corner in logical units
    U_POINTL            cDest;              //!< Destination width in logical units
    uint32_t            dwRop;              //!< Ternary Raster Operation enumeration
    U_POINTL            Src;                //!< Source UL corner in logical units
    U_XFORM             xformSrc;           //!< Transform to apply to source
    U_COLORREF          crBkColorSrc;       //!< Background color
    uint32_t            iUsageSrc;          //!< DIBcolors Enumeration
    U_OFFBMISRC         offBmiSrc;          //!< Offset in bytes to U_BITMAPINFO (within srcbitmapbuffer)
    U_CBBMISRC          cbBmiSrc;           //!< Size   in bytes of U_BITMAPINFO
    U_OFFBITSSRC        offBitsSrc;         //!< Offset in bytes to the src bitmap (within srcbitmapbuffer)
    U_CBBITS            cbBitsSrc;          //!< Size in bytes of src bitmap
    U_POINTL            Mask;               //!< Mask UL corner in logical units
    uint32_t            iUsageMask;         //!< DIBcolors Enumeration
    U_OFFBMIMSK         offBmiMask;         //!< Offset in bytes to U_BITMAPINFO (within maskbitmapbuffer)
    U_CBBMIMSK          cbBmiMask;          //!< Size   in bytes of U_BITMAPINFO
    U_OFFBITSMSK        offBitsMask;        //!< Offset in bytes to the mask bitmap (within maskbitmapbuffer)
    U_CBBITSMSK         cbBitsMask;         //!< Size in bytes of bitmap
                                            //!< Record may include optional Source and mask bitmapbuffers
} U_EMRMASKBLT, *PU_EMRMASKBLT;

/* Index  79 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_POINTL            aptlDst[3];         //!< Defines parallelogram, UL, UR, LL corners, LR is derived.
    U_POINTL            Src;                //!< Source UL corner in logical units
    U_POINTL            cSrc;               //!< Src W & H in logical units
    U_XFORM             xformSrc;           //!< Transform to apply to source
    U_COLORREF          crBkColorSrc;       //!< Background color
    uint32_t            iUsageSrc;          //!< DIBcolors Enumeration
    U_OFFBMISRC         offBmiSrc;          //!< Offset in bytes to U_BITMAPINFO (within srcbitmapbuffer)
    U_CBBMISRC          cbBmiSrc;           //!< Size   in bytes of U_BITMAPINFO
    U_OFFBITSSRC        offBitsSrc;         //!< Offset in bytes to the src bitmap (within srcbitmapbuffer)
    U_CBBITS            cbBitsSrc;          //!< Size in bytes of src bitmap
    U_POINTL            Mask;               //!< Mask UL corner in logical units
    uint32_t            iUsageMask;         //!< DIBcolors Enumeration
    U_OFFBMIMSK         offBmiMask;         //!< Offset in bytes to U_BITMAPINFO (within maskbitmapbuffer)
    U_CBBMIMSK          cbBmiMask;          //!< Size   in bytes of U_BITMAPINFO
    U_OFFBITSMSK        offBitsMask;        //!< Offset in bytes to the mask bitmap (within maskbitmapbuffer)
    U_CBBITSMSK         cbBitsMask;         //!< Size in bytes of bitmap
                                            //!< Record may include optional Source and mask bitmapbuffers
} U_EMRPLGBLT, *PU_EMRPLGBLT;

/* Index  80 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_POINTL            Dest;               //!< Destination UL corner in logical units
    U_POINTL            Src;                //!< Source LL corner in logical units
    U_POINTL            cSrc;               //!< Src W & H in logical units
    U_OFFBMISRC         offBmiSrc;          //!< Offset in bytes to U_BITMAPINFO (within bitmapbuffer)
    U_CBBMISRC          cbBmiSrc;           //!< Size   in bytes of U_BITMAPINFO
    U_OFFBITSSRC        offBitsSrc;         //!< Offset in bytes to bitmap
    U_CBBITS            cbBitsSrc;          //!< Size   in bytes of bitmap
    uint32_t            iUsageSrc;          //!< DIBColors Enumeration
    uint32_t            iStartScan;         //!< First scan line
    uint32_t            cScans;             //!< Number of scan lines
                                            //!< Record may includes optional bitmapbuffer
} U_EMRSETDIBITSTODEVICE, *PU_EMRSETDIBITSTODEVICE;

/* Index  81 */
typedef struct {
    U_EMR               emr;                //!< U_EMR     
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_POINTL            Dest;               //!< Destination UL corner in logical units
    U_POINTL            Src;                //!< Source UL corner in logical units
    U_POINTL            cSrc;               //!< Source W & H in logical units
    U_OFFBMISRC         offBmiSrc;          //!< Offset in bytes to U_BITMAPINFO (within bitmapbuffer)
    U_CBBMISRC          cbBmiSrc;           //!< Size   in bytes of U_BITMAPINFO
    U_OFFBITSSRC        offBitsSrc;         //!< Offset in bytes to bitmap
    U_CBBITS            cbBitsSrc;          //!< Size   in bytes of bitmap
    uint32_t            iUsageSrc;          //!< DIBColors Enumeration
    uint32_t            dwRop;              //!< Ternary Raster Operation enumeration
    U_POINTL            cDest;              //!< Destination W & H in logical units
                                            //!< Record may includes optional bitmapbuffer
} U_EMRSTRETCHDIBITS, *PU_EMRSTRETCHDIBITS;

/* Index  82 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihFont;             //!< Index of the font in the EMF object table
    U_LOGFONT_PANOSE    elfw;               //!< Font parameters, either U_LOGFONT or U_LOGFONT_PANOSE, the latter is bigger so use that type here
} U_EMREXTCREATEFONTINDIRECTW, *PU_EMREXTCREATEFONTINDIRECTW;

/* Index  83,84 */
/**
Variable and optional fields may follow core structure in record:

 U_RECTL             rcl;                absent when fOptions & U_ETO_NO_RECT) grayed/clipping/opaque rectangle

 U_OFFDX             offDx;              (required) Offset in bytes to the character spacing array from the start of the RECORD

 uint32_t             Dx                 (optional) character spacing array (Required, but position is not static.)  
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    uint32_t            iGraphicsMode;      //!< GraphicsMode Enumeration
    U_FLOAT             exScale;            //!< scale to 0.01 mm units ( only if iGraphicsMode & U_GM_COMPATIBLE)
    U_FLOAT             eyScale;            //!< scale to 0.01 mm units ( only if iGraphicsMode & U_GM_COMPATIBLE)
    U_EMRTEXT           emrtext;            //!< Text parameters
// U_RECTL             rcl;                absent when fOptions & U_ETO_NO_RECT) grayed/clipping/opaque rectangle
// U_OFFDX             offDx;              (required) but position isn't static.  Offset in bytes to the character spacing array from the start of the RECORD
//                                         Record may include optional Dx, character spacing, array of uint32_t
} U_EMREXTTEXTOUTA, *PU_EMREXTTEXTOUTA,
  U_EMREXTTEXTOUTW, *PU_EMREXTTEXTOUTW;

/* Index  85,86,87,88,89 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_NUM_POINT16       cpts;               //!< Number of POINT16 in array
    U_POINT16           apts[1];            //!< Array of POINT16
} U_EMRPOLYBEZIER16,*PU_EMRPOLYBEZIER16,
  U_EMRPOLYGON16,*PU_EMRPOLYGON16,
  U_EMRPOLYLINE16,*PU_EMRPOLYLINE16,
  U_EMRPOLYBEZIERTO16,*PU_EMRPOLYBEZIERTO16,
  U_EMRPOLYLINETO16,*PU_EMRPOLYLINETO16;

/* Index  90,91 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_NUM_POLYCOUNTS    nPolys;             //!< Number of elements in aPolyCounts
    U_NUM_POINT16       cpts;               //!< Total number of points (over all poly)
    U_POLYCOUNTS        aPolyCounts[1];     //!< Number of points in each poly (sequential)
//  This will appear somewhere but is not really part of the core structure.
//    U_POINT16           apts[1];            //!< array of point16
} U_EMRPOLYPOLYLINE16,*PU_EMRPOLYPOLYLINE16,
  U_EMRPOLYPOLYGON16,*PU_EMRPOLYPOLYGON16;

/* Index  92 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_NUM_POINT16       cpts;               //!< Total number of points (over all poly)
    U_POINT16           apts[1];            //!< array of points
    uint8_t             abTypes[1];         //!< Array of Point Enumeration
} U_EMRPOLYDRAW16,*PU_EMRPOLYDRAW16;

/* Index  93 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihBrush;            //!< Index to place object in EMF object table (this entry must not yet exist)
    uint32_t            iUsage;             //!< DIBcolors Enumeration
    U_OFFBMI            offBmi;             //!< Offset in bytes to U_BITMAPINFO (within DIBbitmapbuffer)
    U_CBBMI             cbBmi;              //!< Size   in bytes of U_BITMAPINFO
    U_OFFBITS           offBits;            //!< Offset in bytes to the DIB bitmap data (within DIBbitmapbuffer
    U_CBBITS            cbBits;             //!< Size in bytes of DIB bitmap
                                            //!< Record may include optional DIB bitmapbuffer
} U_EMRCREATEMONOBRUSH, *PU_EMRCREATEMONOBRUSH;

/* Index  94 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihBrush;            //!< Index to place object in EMF object table (this entry must not yet exist)
    uint32_t            iUsage;             //!< DIBcolors Enumeration
    U_OFFBMI            offBmi;             //!< Offset in bytes to U_BITMAPINFO (within DIB bitmapbuffer)
    U_CBBMI             cbBmi;              //!< Size   in bytes of U_BITMAPINFO
    U_OFFBITS           offBits;            //!< Offset in bytes to the DIB bitmap data (within DIB bitmapbuffer
    U_CBBITS            cbBits;             //!< Size in bytes of DIB bitmap
                                            //!< Record may include optional DIB bitmapbuffer
} U_EMRCREATEDIBPATTERNBRUSHPT, *PU_EMRCREATEDIBPATTERNBRUSHPT;

/* Index  95 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihPen;              //!< Index to place object in EMF object table (this entry must not yet exist)
    U_OFFBMI            offBmi;             //!< Offset in bytes to U_BITMAPINFO (within DIB bitmapbuffer)
    U_CBBMI             cbBmi;              //!< Size   in bytes of U_BITMAPINFO
    U_OFFBITS           offBits;            //!< Offset in bytes to the DIB bitmap data (within DIB bitmapbuffer
    U_CBBITS            cbBits;             //!< Size in bytes of DIB bitmap
    U_EXTLOGPEN         elp;                //!< Pen parameters (Size is Variable!!!!)
                                            //!< Record may include optional DIB bitmap
} U_EMREXTCREATEPEN, *PU_EMREXTCREATEPEN;

/* Index  96.97 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    uint32_t            iGraphicsMode;      //!< GraphicsMode Enumeration
    U_FLOAT             exScale;            //!< scale to 0.01 mm units ( only if iGraphicsMode & U_GM_COMPATIBLE)
    U_FLOAT             eyScale;            //!< scale to 0.01 mm units ( only if iGraphicsMode & U_GM_COMPATIBLE)
    U_NUM_EMRTEXT       cStrings;           //!< Number of U_EMRTEXT in array
    U_EMRTEXT           emrtext[1];         //!< Text parameters
} U_EMRPOLYTEXTOUTA, *PU_EMRPOLYTEXTOUTA,
  U_EMRPOLYTEXTOUTW, *PU_EMRPOLYTEXTOUTW;

/* Index  98  (see 17) */

/* Index  99 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihCS;               //!< Index to place object in EMF object table (this entry must not yet exist)
    U_LOGCOLORSPACEA    lcs;                //!< ColorSpace parameters
} U_EMRCREATECOLORSPACE, *PU_EMRCREATECOLORSPACE;

/* Index  100,101 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihCS;               //!< Index of object in EMF object table
} U_EMRDELETECOLORSPACE, *PU_EMRDELETECOLORSPACE,
  U_EMRSETCOLORSPACE,    *PU_EMRSETCOLORSPACE;

/* Index  102 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_CBDATA            cbData;             //!< Size of OpenGL data in bytes
    U_DATA              Data[1];            //!< OpenGL data
} U_EMRGLSRECORD, *PU_EMRGLSRECORD;

/* Index  103 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_CBDATA            cbData;             //!< Size of OpenGL data in bytes
    U_DATA              Data[1];            //!< OpenGL data
} U_EMRGLSBOUNDEDRECORD, *PU_EMRGLSBOUNDEDRECORD; 

/* Index  104 */
typedef struct {
    U_EMR                   emr;            //!< U_EMR
    U_PIXELFORMATDESCRIPTOR pfd;            //!< PixelFormatDescriptor
} U_EMRPIXELFORMAT, *PU_EMRPIXELFORMAT;

/* Index  105 */
typedef struct {
    U_EMR               emr;                //!< U_EMR 
    U_CBDATA            cjIn;               //!< Number of bytes to send to printer driver
    U_DATA              Data[1];            //!< Data to send
} U_EMRDRAWESCAPE, *PU_EMRDRAWESCAPE;

/* Index  106 */
typedef struct {
    U_EMR               emr;                //!< U_EMR 
    U_CBDATA            cjIn;               //!< Number of bytes to send to printer driver
    U_DATA              Data[1];            //!< Data to send
} U_EMREXTESCAPE, *PU_EMREXTESCAPE;

/* Index  107 (not implemented ) */

/* Index  108 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_POINTL            Dest;               //!< Where to draw the text
    U_NUM_STR           cChars;             //!< Characters in TextString (not null terminated)
    uint32_t            fuOptions;          //!< ExtTextOutOptions Enumeration
    uint32_t            iGraphicsMode;      //!< GraphicsMode Enumeration
    U_FLOAT             exScale;            //!< scale on X axis
    U_FLOAT             eyScale;            //!< scale on Y axis
//!< the tail end of this record is variable.
//!<    U_RECTL             rclBounds;           Record may include optional Bounding rectangle (absent when: fuOPtions & ETO_NO_U_RECT)
//!<    uint32_t            TextString;          text to output (fuOptions & ETO_SMALL_CHARS ? 8 bit : 16 bit)
} U_EMRSMALLTEXTOUT, *PU_EMRSMALLTEXTOUT;

/* Index  109 (not implemented ) */

/* Index  110 */
typedef struct {
    U_EMR               emr;                //!< U_EMR       
    U_CBDATA            cbDriver;           //!< Number of bytes in driver name (note, BYTES, not CHARACTERS)
    U_CBDATA            cbData;             //!< Number of bytes in data
    uint16_t            Driver[1];          //!< Driver name in uint16_t characters, null terminated
    uint8_t             Data[1];            //!< Data for printer driver
} U_EMRNAMEDESCAPE, *PU_EMRNAMEDESCAPE;

/* Index  111-113 (not implemented ) */

/* Index 114 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_POINTL            Dest;               //!< Destination UL corner in logical units
    U_POINTL            cDest;              //!< Destination W & H in logical units
    U_BLEND             Blend;              //!< Blend Function
    U_POINTL            Src;                //!< Source UL corner in logical units
    U_XFORM             xformSrc;           //!< Transform to apply to source
    U_COLORREF          crBkColorSrc;       //!< Background color
    uint32_t            iUsageSrc;          //!< DIBcolors Enumeration
    U_OFFBMISRC         offBmiSrc;          //!< Offset in bytes to U_BITMAPINFO (within bitmapbuffer)
    U_CBBMISRC          cbBmiSrc;           //!< Size   in bytes of U_BITMAPINFO
    U_OFFBITSSRC        offBitsSrc;         //!< Offset in bytes to the bitmap (within bitmapbuffer)
    U_CBBITS            cbBitsSrc;          //!< Size in bytes of bitmap
    U_POINTL            cSrc;               //!< Source W & H in logical units
                                            //!< Record may include optional DIB bitmap
} U_EMRALPHABLEND, *PU_EMRALPHABLEND;

/* Index  115  (see 17) */

/* Index  116 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_POINTL            Dest;               //!< Destination UL corner in logical units
    U_POINTL            cDest;              //!< Destination W & H in logical units
    uint32_t            TColor;             //!< Bitmap color to be treated as transparent
    U_POINTL            Src;                //!< Source UL corner in logical units
    U_XFORM             xformSrc;           //!< Transform to apply to source
    U_COLORREF          crBkColorSrc;       //!< Background color
    uint32_t            iUsageSrc;          //!< DIBcolors Enumeration
    U_OFFBMISRC         offBmiSrc;          //!< Offset in bytes to U_BITMAPINFO (within bitmapbuffer)
    U_CBBMISRC          cbBmiSrc;           //!< Size   in bytes of U_BITMAPINFO
    U_OFFBITSSRC        offBitsSrc;         //!< Offset in bytes to the bitmap (within bitmapbuffer)
    U_CBBITS            cbBitsSrc;          //!< Size in bytes of bitmap
    U_POINTL            cSrc;               //!< Source W & H in logical units
                                            //!< Record may includes optional bitmapbuffer
} U_EMRTRANSPARENTBLT, *PU_EMRTRANSPARENTBLT;

/* Index  117 (not a defined U_EMR record type ) */

/* Index  118 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_NUM_TRIVERTEX     nTriVert;           //!< Number of TriVertex objects
    U_NUM_GRADOBJ       nGradObj;           //!< Number of gradient triangle/rectangle objects
    uint32_t            ulMode;             //!< Gradientfill Enumeration (determines Triangle/Rectangle)
//parts that are required but which are not included in the core structure
//    U_TRIVERTEX         TriVert[1];          Array of TriVertex objects
//    uint32_t            GradObj[1];          Array of gradient objects (each has 2 or 3 indices into TriVert array) 
} U_EMRGRADIENTFILL, *PU_EMRGRADIENTFILL;

/* Index  119,120 (not implemented ) */

/* Index  121 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            dwAction;           //!< ColorSpace Enumeration
    uint32_t            dwFlags;            //!< ColorMatchToTarget Enumeration
    U_CBNAME            cbName;             //!< Number of bytes in in UTF16 name of the color profile
    U_CBDATA            cbData;             //!< Number of bytes of the target profile
    uint8_t             Data[1];            //!< Data of size cbName+cbData: Name in UTF16 then color profile data 
} U_EMRCOLORMATCHTOTARGETW, *PU_EMRCOLORMATCHTOTARGETW;

/* Index  122 */
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihCS;               //!< Index of the logical color space object in the EMF object table 
    U_LOGCOLORSPACEW    lcs;                //!< Description of the color profile
    uint32_t            dwFlags;            //!< If low bit set Data is present
    U_CBDATA            cbData;             //!< Number of bytes of theData field.
    uint8_t             Data[1];            //!< (Optional, dwFlags & 1) color profile data 
} U_EMRCREATECOLORSPACEW, *PU_EMRCREATECOLORSPACEW;

// ************************************************************************************************
// Utility function structures

typedef struct {
    FILE               *fp;                 //!< Open file
    size_t              allocated;          //!< Size of the buffer
    size_t              used;               //!< Amount consumed
    uint32_t            records;            //!< Number of records already contained
    uint16_t            ignore;             //!< size padding,not used
    uint32_t            PalEntries;         //!< Number of PalEntries (set from U_EMREOF)
    uint32_t            chunk;              //!< Number of bytes to add when more space is needed
    char               *buf;                //!< Buffer for constructing the EMF in memory 
} EMFTRACK;

/**
  The various create functions need a place to put their handles, these are stored in the table below.
  We don't actually do anything much with these handles, that is up to whatever program finally plays back the EMF, but
  we do need to keep track of the numbers so that they are not accidentally reused.  This structure is used for that,
  and all *_set functions that touch a handle reference it.
  
  Stock objects are not used in this limited model, so libUEMF cannot detect if a handle is still in use.  Nor can it
  tell when a handle has been deselected (by selecting another handle for the same type of graphic object, and thus
  made deleteable).  End user code must keep track of this for itself.
*/
typedef struct {
    uint32_t           *table;              //!< Array Buffer for constructing the EMF in memory 
    uint32_t           *stack;              //!< handles are either on the stack or in the table
    size_t              allocated;          //!< Slots in the buffer
    size_t              chunk;              //!< Number to add if a realloc is required
    uint32_t            sptr;               //!< Pointer to next available handle in the stack
    uint32_t            top;                //!< Highest slot occupied (currently)
    uint32_t            peak;               //!< Highest slot occupied (ever)
} EMFHANDLES;

/**
  2 x 2 matrix, used by xform_alt_set() function.
*/
typedef struct {
    double M11;                             //!< Matrix element 1,1
    double M12;                             //!< Matrix element 1,2
    double M21;                             //!< Matrix element 2,1
    double M22;                             //!< Matrix element 2,2
} U_MAT2X2, *PU_MAT2X2;

// ************************************************************************************************
// Prototypes

int  memprobe(const void *buf, size_t size);
void wchar8show(const char *src);
void wchar16show(const uint16_t *src);
void wchar32show(const uint32_t *src);
void wchartshow(const wchar_t *src);
void dumpeht(char *string, unsigned int *handle, EMFHANDLES *eht);


char     *U_emr_names(unsigned int idx);
uint32_t *dx_set(int32_t height,  uint32_t weight, uint32_t members);
uint32_t  emr_properties(uint32_t type);
int       emr_arc_points(PU_ENHMETARECORD record, int *f1, int f2, PU_PAIRF center, PU_PAIRF start, PU_PAIRF end, PU_PAIRF size);
int       emr_arc_points_common(PU_RECTL rclBox,  PU_POINTL ArcStart, PU_POINTL ArcEnd,
               int *f1, int f2, PU_PAIRF center, PU_PAIRF start, PU_PAIRF end, PU_PAIRF size);
int       get_real_color_count(const char *Bmih);
int       get_real_color_icount(int Colors, int BitCount, int Width, int Height);
int       RGBA_to_DIB(char **px, uint32_t *cbPx, PU_RGBQUAD *ct, int *numCt, 
               const char *rgba_px, int w, int h, int stride, uint32_t colortype, int use_ct, int invert);
int       get_DIB_params( void *pEmr, uint32_t offBitsSrc, uint32_t offBmiSrc, 
               const char **px, const U_RGBQUAD **ct, uint32_t *numCt, 
               uint32_t *width, uint32_t *height, uint32_t *colortype, uint32_t *invert );
int       DIB_to_RGBA(const char *px, const U_RGBQUAD *ct, int numCt,
               char **rgba_px, int w, int h, uint32_t colortype, int use_ct, int invert);
char     *RGBA_to_RGBA(char *rgba_px, int w, int h, int sl, int st, int *ew, int *eh);

int   device_size(const int xmm, const int ymm, const float dpmm, U_SIZEL *szlDev, U_SIZEL *szlMm);
int   drawing_size(const int xmm, const int yum, const float dpmm, U_RECTL *rclBounds, U_RECTL *rclFrame);

int   emf_start(const char *name, const uint32_t initsize, const uint32_t chunksize, EMFTRACK **et);
int   emf_finish(EMFTRACK *et, EMFHANDLES *eht);
int   emf_free(EMFTRACK **et);
int   emf_append(U_ENHMETARECORD *rec, EMFTRACK *et, int freerec);
int   emf_readdata(const char *filename, char **contents, size_t *length);   
FILE *emf_fopen(const char *filename, const int mode);


/* use these instead*/
int   emf_htable_create(uint32_t initsize, uint32_t chunksize, EMFHANDLES **eht);
int   emf_htable_delete(uint32_t *ih, EMFHANDLES *eht);
int   emf_htable_insert(uint32_t *ih, EMFHANDLES *eht);
int   emf_htable_free(EMFHANDLES **eht);
/* Deprecated forms */
#define   htable_create  emf_htable_create
#define   htable_delete  emf_htable_delete
#define   htable_insert  emf_htable_insert
#define   htable_free    emf_htable_free

U_RECTL          rectl_set(U_POINTL ul, U_POINTL lr);
U_SIZEL          sizel_set(int32_t  x,  int32_t  y);
U_POINTL         point32_set(int32_t  x,  int32_t  y);
#define point_set point32_set
#define pointl_set point32_set
U_POINT16        point16_set(int16_t  x,  int16_t  y);
U_PANOSE         panose_set(uint8_t bFamilyType, uint8_t bSerifStyle,      uint8_t bWeight,   uint8_t bProportion,
                      uint8_t bContrast,   uint8_t bStrokeVariation, uint8_t bArmStyle, uint8_t bLetterform, 
                      uint8_t bMidline,    uint8_t bXHeight );
U_COLORREF       colorref_set(uint8_t red, uint8_t green, uint8_t blue);
U_LOGBRUSH       logbrush_set(uint32_t lbStyle, U_COLORREF lbColor, int32_t lbHatch);
U_XFORM          xform_set(U_FLOAT eM11, U_FLOAT eM12, U_FLOAT eM21, U_FLOAT eM22, U_FLOAT eDx, U_FLOAT eDy);
U_XFORM          xform_alt_set(U_FLOAT scale, U_FLOAT ratio, U_FLOAT rot, U_FLOAT axisrot, U_FLOAT eDx, U_FLOAT eDy);
U_LOGPEN         logpen_set( uint32_t lopnStyle,  U_POINT lopnWidth, U_COLORREF lopnColor );
PU_EXTLOGPEN     extlogpen_set(uint32_t elpPenStyle, uint32_t elpWidth, uint32_t elpBrushStyle,
                     U_COLORREF elpColor, int32_t elpHatch, U_NUM_STYLEENTRY elpNumEntries, U_STYLEENTRY *elpStyleEntry );
U_LOGFONT_PANOSE logfont_panose_set(U_LOGFONT elfLogFont, uint16_t *elfFullName, 
                     uint16_t *elfStyle, uint32_t elfStyleSize, U_PANOSE elfPanose);
U_LOGFONT        logfont_set( int32_t lfHeight, int32_t lfWidth, int32_t lfEscapement, int32_t lfOrientation, 
                     int32_t lfWeight, uint8_t lfItalic, uint8_t lfUnderline, uint8_t lfStrikeOut,
                     uint8_t lfCharSet, uint8_t lfOutPrecision, uint8_t lfClipPrecision, 
                     uint8_t lfQuality, uint8_t lfPitchAndFamily, uint16_t *lfFaceName);
char            *emrtext_set(U_POINTL ptlReference, U_NUM_STR NumString, uint32_t cbChar, void *String, uint32_t fOptions, U_RECTL rcl, uint32_t *Dx);
U_LOGPLTNTRY     logpltntry_set(uint8_t peReserved,uint8_t peRed,uint8_t peGreen,uint8_t peBlue);
PU_LOGPALETTE    logpalette_set(U_NUM_LOGPLTNTRY palNumEntries,PU_LOGPLTNTRY *palPalEntry);
U_RGNDATAHEADER  rgndataheader_set( U_NUM_RECTL nCount,  U_RECTL rcBound);
PU_RGNDATA       rgndata_set( U_RGNDATAHEADER rdh, PU_RECTL Buffer);
U_BITMAPINFOHEADER bitmapinfoheader_set(int32_t biWidth, int32_t biHeight, 
                     uint16_t biPlanes, uint16_t biBitCount, uint32_t biCompression, 
                     uint32_t biSizeImage, int32_t biXPelsPerMeter, 
                     int32_t biYPelsPerMeter, U_NUM_RGBQUAD biClrUsed, uint32_t biClrImportant);
PU_BITMAPINFO    bitmapinfo_set(U_BITMAPINFOHEADER BmiHeader, PU_RGBQUAD BmiColors);
U_LOGCOLORSPACEA logcolorspacea_set(int32_t lcsCSType, int32_t lcsIntent,
                     U_CIEXYZTRIPLE lcsEndpoints, U_LCS_GAMMARGB lcsGammaRGB, char *lcsFilename);
U_LOGCOLORSPACEW logcolorspacew_set(int32_t lcsCSType, int32_t lcsIntent,
                     U_CIEXYZTRIPLE lcsEndpoints, U_LCS_GAMMARGB lcsGammaRGB, uint16_t *lcsFilename);
U_COLORADJUSTMENT coloradjustment_set(uint16_t Size, uint16_t Flags, uint16_t IlluminantIndex,
                     uint16_t RedGamma, uint16_t GreenGamma, uint16_t BlueGamma, 
                     uint16_t ReferenceBlack, uint16_t ReferenceWhite,
                     int16_t Contrast, int16_t Brightness, int16_t Colorfulness, int16_t RedGreenTint);
U_PIXELFORMATDESCRIPTOR pixelformatdescriptor_set( uint32_t dwFlags, uint8_t iPixelType, uint8_t cColorBits,
                     uint8_t cRedBits, uint8_t cRedShift,
                     uint8_t cGreenBits, uint8_t cGreenShift,
                     uint8_t cBlueBits, uint8_t cBlueShift,
                     uint8_t cAlphaBits, uint8_t cAlphaShift, 
                     uint8_t cAccumBits, uint8_t cAccumRedBits, uint8_t cAccumGreenBits, uint8_t cAccumBlueBits,
                     uint8_t cAccumAlphaBits, uint8_t cDepthBits, uint8_t cStencilBits,
                     uint8_t cAuxBuffers, uint8_t iLayerType, uint8_t bReserved, uint32_t dwLayerMask,
                     uint32_t dwVisibleMask, uint32_t dwDamageMask);
  
PU_POINT     points_transform(PU_POINT points, int count, U_XFORM xform);
PU_POINT16   point16_transform(PU_POINT16 points, int count, U_XFORM xform);
PU_TRIVERTEX trivertex_transform(PU_TRIVERTEX tv, int count, U_XFORM xform);
PU_POINT     point16_to_point(PU_POINT16 points, int count);
PU_POINT16   point_to_point16(PU_POINT   points, int count);

U_RECT findbounds(uint32_t count, PU_POINT pts, uint32_t width);
U_RECT findbounds16(uint32_t count, PU_POINT16 pts, uint32_t width);
char *emr_dup(const char *emr);

char *textcomment_set(const char *string);

// These generate the handle and then call the underlying function
char *deleteobject_set(uint32_t *ihObject, EMFHANDLES *eht); 
char *selectobject_set(uint32_t ihObject, EMFHANDLES *eht);
char *createpen_set(uint32_t *ihPen, EMFHANDLES *eht, U_LOGPEN lopn );
char *extcreatepen_set(uint32_t *ihPen, EMFHANDLES *eht,
                    PU_BITMAPINFO Bmi, const uint32_t cbPx, char *Px, PU_EXTLOGPEN elp);
char *createbrushindirect_set(uint32_t *ihBrush, EMFHANDLES *eht, U_LOGBRUSH lb );
char *createdibpatternbrushpt_set(uint32_t *ihBrush, EMFHANDLES *eht, uint32_t iUsage, 
                    PU_BITMAPINFO Bmi, const uint32_t cbPx, const char *Px);
char *createmonobrush_set(uint32_t *ihBrush, EMFHANDLES *eht, uint32_t iUsage, 
                    PU_BITMAPINFO Bmi, const uint32_t cbPx, const char *Px);
char *extcreatefontindirectw_set(uint32_t *ihFont, EMFHANDLES *eht, const char *elf, const char *elfw);
char *createpalette_set(uint32_t *ihPal, EMFHANDLES *eht,  U_LOGPALETTE lgpl);
char *setpaletteentries_set(uint32_t *ihPal, EMFHANDLES *eht, uint32_t iStart, U_NUM_LOGPLTNTRY cEntries, PU_LOGPLTNTRY aPalEntries);
char *fillrgn_set(uint32_t *ihBrush, EMFHANDLES *eht, U_RECTL rclBounds,PU_RGNDATA RgnData);
char *framergn_set(uint32_t *ihBrush, EMFHANDLES *eht, U_RECTL rclBounds, U_SIZEL szlStroke, PU_RGNDATA RgnData);
char *createcolorspace_set(uint32_t *ihCS, EMFHANDLES *eht, U_LOGCOLORSPACEA lcs);
char *createcolorspacew_set(uint32_t *ihCS, EMFHANDLES *eht, U_LOGCOLORSPACEW lcs, uint32_t dwFlags, U_CBDATA cbData, uint8_t *Data);

char *U_EMRHEADER_set( const U_RECTL rclBounds,  const U_RECTL rclFrame,  U_PIXELFORMATDESCRIPTOR* const pfmtDesc,
    U_CBSTR nDesc, uint16_t* const Description, const U_SIZEL szlDevice, const U_SIZEL szlMillimeters,
    const uint32_t bOpenGL);
char *U_EMRPOLYBEZIER_set(  const U_RECTL rclBounds, const uint32_t count, const U_POINTL *points);
char *U_EMRPOLYGON_set(     const U_RECTL rclBounds, const uint32_t count, const U_POINTL *points);
char *U_EMRPOLYLINE_set(    const U_RECTL rclBounds, const uint32_t count, const U_POINTL *points);
char *U_EMRPOLYBEZIERTO_set(const U_RECTL rclBounds, const uint32_t count, const U_POINTL *points);
char *U_EMRPOLYLINETO_set(  const U_RECTL rclBounds, const uint32_t count, const U_POINTL *points);

char *U_EMRPOLYPOLYLINE_set(const U_RECTL rclBounds, const uint32_t nPolys, const uint32_t *aPolyCounts,
                          const uint32_t cptl, const U_POINTL *points);
char *U_EMRPOLYPOLYGON_set(const U_RECTL rclBounds, const uint32_t nPolys, const uint32_t *aPolyCounts,
                          const uint32_t cptl, const U_POINTL *points);
char *U_EMRSETWINDOWEXTEX_set(const U_SIZEL szlExtent);
char *U_EMRSETWINDOWORGEX_set(const U_POINTL ptlOrigin);
char *U_EMRSETVIEWPORTEXTEX_set(const U_SIZEL szlExtent);
char *U_EMRSETVIEWPORTORGEX_set(const U_POINTL ptlOrigin);
char *U_EMRSETBRUSHORGEX_set(const U_POINTL ptlOrigin);
char *U_EMREOF_set(const U_CBPLENTRIES cbPalEntries, const PU_LOGPLTNTRY PalEntries, EMFTRACK *et);
char *U_EMRSETPIXELV_set(const U_POINTL ptlPixel, const U_COLORREF crColor);
char *U_EMRSETMAPPERFLAGS_set(void);
char *U_EMRSETMAPMODE_set(const uint32_t iMode);
char *U_EMRSETBKMODE_set(const uint32_t iMode);
char *U_EMRSETPOLYFILLMODE_set(const uint32_t iMode);
char *U_EMRSETROP2_set(const uint32_t iMode);
char *U_EMRSETSTRETCHBLTMODE_set(const uint32_t iMode);
char *U_EMRSETTEXTALIGN_set(const uint32_t iMode);
char *U_EMRSETCOLORADJUSTMENT_set(const U_COLORADJUSTMENT ColorAdjustment);
char *U_EMRSETTEXTCOLOR_set(const U_COLORREF crColor);
char *U_EMRSETBKCOLOR_set(const U_COLORREF crColor);
char *U_EMROFFSETCLIPRGN_set(const U_POINTL ptl);
char *U_EMRMOVETOEX_set(const U_POINTL ptl);
char *U_EMRSETMETARGN_set(void);
char *U_EMREXCLUDECLIPRECT_set(const U_RECTL rclClip);
char *U_EMRINTERSECTCLIPRECT_set(const U_RECTL rclClip);
char *U_EMRSCALEVIEWPORTEXTEX_set(const int32_t xNum, const int32_t xDenom, const int32_t yNum, const int32_t yDenom);
char *U_EMRSCALEWINDOWEXTEX_set(const int32_t xNum, const int32_t xDenom,  const int32_t yNum, const int32_t yDenom);
char *U_EMRSAVEDC_set(void);      
char *U_EMRRESTOREDC_set(const int32_t iRelative);
char *U_EMRSETWORLDTRANSFORM_set(const U_XFORM xform);
char *U_EMRMODIFYWORLDTRANSFORM_set(const U_XFORM xform, const uint32_t iMode);
char *U_EMRSELECTOBJECT_set(const uint32_t ihObject);  // better to call selectobject_set()
char *U_EMRCREATEPEN_set(const uint32_t ihPen, const U_LOGPEN lopn );
char *U_EMRCREATEBRUSHINDIRECT_set(const uint32_t ihBrush, const U_LOGBRUSH lb);
char *U_EMRDELETEOBJECT_set(const uint32_t ihObject); // better to call deleteobject_set()
char *U_EMRANGLEARC_set(const U_POINTL ptlCenter, const uint32_t nRadius, const U_FLOAT eStartAngle, const U_FLOAT eSweepAngle);
char *U_EMRELLIPSE_set(const U_RECTL rclBox);
char *U_EMRRECTANGLE_set(const U_RECTL rclBox);
char *U_EMRROUNDRECT_set(const U_RECTL rclBox, const U_SIZEL szlCorner);
char *U_EMRARC_set(const U_RECTL rclBox,  const U_POINTL ptlStart, const U_POINTL ptlEnd);
char *U_EMRCHORD_set(const U_RECTL rclBox, const U_POINTL ptlStart, const U_POINTL ptlEnd);
char *U_EMRPIE_set(const U_RECTL rclBox, const U_POINTL ptlStart, const U_POINTL ptlEnd);
char *U_EMRSELECTPALETTE_set(const uint32_t ihPal);
char *U_EMRCREATEPALETTE_set(const uint32_t ihPal, const U_LOGPALETTE lgpl);
char *U_EMRSETPALETTEENTRIES_set(const uint32_t ihPal, const uint32_t iStart, const U_NUM_LOGPLTNTRY cEntries, const PU_LOGPLTNTRY aPalEntries);
char *U_EMRRESIZEPALETTE_set(const uint32_t ihPal, const uint32_t cEntries);
char *U_EMRREALIZEPALETTE_set(void);
char *U_EMREXTFLOODFILL_set(const U_POINTL ptlStart, const U_COLORREF crColor, const uint32_t iMode);
char *U_EMRLINETO_set(const U_POINTL ptl);
char *U_EMRARCTO_set(const U_RECTL rclBox, const U_POINTL ptlStart, const U_POINTL ptlEnd);
char *U_EMRPOLYDRAW_set(const U_RECTL rclBounds,const U_NUM_POINTL cptl,const U_POINTL *aptl,const uint8_t *abTypes);
char *U_EMRSETARCDIRECTION_set(const uint32_t iArcDirection);
char *U_EMRSETMITERLIMIT_set(const uint32_t eMiterLimit);
char *U_EMRBEGINPATH_set(void); 
char *U_EMRENDPATH_set(void);  
char *U_EMRCLOSEFIGURE_set(void);
char *U_EMRFILLPATH_set(const U_RECTL rclBox);
char *U_EMRSTROKEANDFILLPATH_set(const U_RECTL rclBox);
char *U_EMRSTROKEPATH_set(const U_RECTL rclBox);
char *U_EMRFLATTENPATH_set(void); 
char *U_EMRWIDENPATH_set(void);
char *U_EMRSELECTCLIPPATH_set(const uint32_t iMode);
char *U_EMRABORTPATH_set(void);
// EMR_ENDEF69
char *U_EMRCOMMENT_set(const U_CBDATA cbData, const char *Data);
char *U_EMRFILLRGN_set(const U_RECTL rclBounds, const uint32_t ihBrush, const PU_RGNDATA RgnData);
char *U_EMRFRAMERGN_set(const U_RECTL rclBounds, const uint32_t ihBrush, const U_SIZEL szlStroke, const PU_RGNDATA RgnData);
char *U_EMRINVERTRGN_set(const PU_RGNDATA RgnData);
char *U_EMRPAINTRGN_set(const PU_RGNDATA RgnData);
char *U_EMREXTSELECTCLIPRGN_set(const uint32_t iMode, const PU_RGNDATA RgnData);
char *U_EMRBITBLT_set(const U_RECTL rclBounds, const U_POINTL Dest, const U_POINTL cDest,
                    const U_POINTL Src, const U_XFORM xformSrc, const U_COLORREF crBkColorSrc,
                    const uint32_t iUsageSrc, const uint32_t dwRop, 
                    const PU_BITMAPINFO Bmi, const uint32_t cbPx, char *Px);
char *U_EMRSTRETCHBLT_set(U_RECTL rclBounds, U_POINTL Dest, U_POINTL cDest,
                    const U_POINTL Src, const U_POINTL cSrc, const U_XFORM xformSrc, const U_COLORREF crBkColorSrc, const uint32_t iUsageSrc, 
                    const uint32_t dwRop,
                    const PU_BITMAPINFO Bmi, const uint32_t cbPx, char *Px);
char *U_EMRMASKBLT_set(U_RECTL rclBounds, U_POINTL Dest, U_POINTL cDest,  
                    const U_POINTL Src, const U_XFORM xformSrc, const U_COLORREF crBkColorSrc, const uint32_t iUsageSrc, 
                    const U_POINTL Mask, const uint32_t iUsageMask,
                    const uint32_t dwRop,
                    const PU_BITMAPINFO Bmi,  const uint32_t cbPx,  char *Px,
                    const PU_BITMAPINFO BmiMsk, const uint32_t cbMsk, char *Msk);
char *U_EMRPLGBLT_set(const U_RECTL rclBounds, const PU_POINTL aptlDst, 
                    const U_POINTL Src, const U_POINTL cSrc, const U_XFORM xformSrc, const U_COLORREF crBkColorSrc, const uint32_t iUsageSrc, 
                    const U_POINTL Mask, const uint32_t iUsageMask, 
                    const PU_BITMAPINFO Bmi,  const uint32_t cbPx,  char *Px,
                    const PU_BITMAPINFO BmiMsk, const uint32_t cbMsk, char *Msk);
char *U_EMRSETDIBITSTODEVICE_set(const U_RECTL rclBounds, const U_POINTL Dest,
                    const U_POINTL Src, const U_POINTL cSrc, const uint32_t iUsageSrc, 
                    const uint32_t iStartScan, const uint32_t cScans,
                    const PU_BITMAPINFO Bmi, const uint32_t cbPx, char *Px);
char *U_EMRSTRETCHDIBITS_set(const U_RECTL rclBounds, const U_POINTL Dest, const U_POINTL cDest,
                    const U_POINTL Src, const U_POINTL cSrc, const uint32_t iUsageSrc, 
                    const uint32_t dwRop,
                    const PU_BITMAPINFO Bmi, const uint32_t cbPx, char *Px);
char *U_EMREXTCREATEFONTINDIRECTW_set( uint32_t ihFont, const char *elf, const char *elfw);
char *U_EMREXTTEXTOUTA_set(U_RECTL rclBounds, uint32_t iGraphicsMode, U_FLOAT exScale, U_FLOAT eyScale, PU_EMRTEXT emrtext);
char *U_EMREXTTEXTOUTW_set(U_RECTL rclBounds, uint32_t iGraphicsMode, U_FLOAT exScale, U_FLOAT eyScale, PU_EMRTEXT emrtext);
char *U_EMRPOLYBEZIER16_set(const U_RECTL rclBounds, const uint32_t cpts, const U_POINT16 *points);
char *U_EMRPOLYGON16_set(const U_RECTL rclBounds, const uint32_t cpts, const U_POINT16 *points);
char *U_EMRPOLYLINE16_set(const U_RECTL rclBounds, const uint32_t cpts, const U_POINT16 *points);
char *U_EMRPOLYBEZIERTO16_set(const U_RECTL rclBounds, const uint32_t cpts, const U_POINT16 *points);
char *U_EMRPOLYLINETO16_set(const U_RECTL rclBounds, const uint32_t cpts, const U_POINT16 *points);
char *U_EMRPOLYPOLYLINE16_set(const U_RECTL rclBounds, const uint32_t nPolys, const uint32_t *aPolyCounts,const uint32_t cpts, const U_POINT16 *points);
char *U_EMRPOLYPOLYGON16_set(const U_RECTL rclBounds, const uint32_t nPolys, const uint32_t *aPolyCounts,const uint32_t cpts, const U_POINT16 *points);
char *U_EMRPOLYDRAW16_set(const U_RECTL rclBounds,const U_NUM_POINT16 cpts, const U_POINT16 *aptl, const uint8_t *abTypes);
char *U_EMRCREATEMONOBRUSH_set(const uint32_t ihBrush, const uint32_t iUsage, 
                    const PU_BITMAPINFO Bmi, const uint32_t cbPx, const char *Px);
char *U_EMRCREATEDIBPATTERNBRUSHPT_set(const uint32_t ihBrush, const uint32_t iUsage,
                    const PU_BITMAPINFO Bmi, const uint32_t cbPx, const char *Px);
char *U_EMREXTCREATEPEN_set(const uint32_t ihPen, const PU_BITMAPINFO Bmi, const uint32_t cbPx, char *Px, const PU_EXTLOGPEN elp );
// U_EMRPOLYTEXTOUTA_set 96 NOT IMPLEMENTED, denigrated after Windows NT
// U_EMRPOLYTEXTOUTW_set 97 NOT IMPLEMENTED, denigrated after Windows NT
char *U_EMRSETICMMODE_set(const uint32_t iMode);
char *U_EMRCREATECOLORSPACE_set(const uint32_t ihCS, const U_LOGCOLORSPACEA lcs);
char *U_EMRSETCOLORSPACE_set(const uint32_t ihCS );
char *U_EMRDELETECOLORSPACE_set(const uint32_t ihCS);
// U_EMRGLSRECORD_set                102  Not implemented
// U_EMRGLSBOUNDEDRECORD_set         103  Not implemented
char *U_EMRPIXELFORMAT_set(const U_PIXELFORMATDESCRIPTOR pfd);
char *U_EMRSMALLTEXTOUT_set(const U_POINTL Dest, const U_NUM_STR cChars, const uint32_t fuOptions, const uint32_t iGraphicsMode, 
                    const U_FLOAT exScale, const U_FLOAT eyScale, const U_RECTL rclBounds, const char *TextString);
// U_EMRDRAWESCAPE_set               105  Not implemented
// U_EMREXTESCAPE_set                106  Not implemented
// U_EMRUNDEF107_set                 107  Not implemented
// U_EMRSMALLTEXTOUT_set             108
// U_EMRFORCEUFIMAPPING_set          109  Not implemented
// U_EMRNAMEDESCAPE_set              110  Not implemented
// U_EMRCOLORCORRECTPALETTE_set      111  Not implemented
// U_EMRSETICMPROFILEA_set           112  Not implemented
// U_EMRSETICMPROFILEW_set           113  Not implemented
char *U_EMRALPHABLEND_set(const U_RECTL rclBounds, const U_POINTL Dest, const U_POINTL cDest, 
                    const U_POINTL Src, const U_POINTL cSrc, const U_XFORM xformSrc,
                    const U_COLORREF crBkColorSrc, const uint32_t iUsageSrc, 
                    const U_BLEND Blend,
                    const PU_BITMAPINFO Bmi, const uint32_t cbPx, char *Px);
char *U_EMRSETLAYOUT_set(const uint32_t iMode);
char *U_EMRTRANSPARENTBLT_set(const U_RECTL rclBounds, const U_POINTL Dest, const U_POINTL cDest, 
                    const U_POINTL Src, const U_POINTL cSrc, const U_XFORM xformSrc, 
                    const U_COLORREF crBkColorSrc, const uint32_t iUsageSrc, const uint32_t TColor,
                    const PU_BITMAPINFO Bmi, const uint32_t cbPx, char *Px);
// U_EMRUNDEF117_set                 117  Not implemented
char *U_EMRGRADIENTFILL_set(const U_RECTL rclBounds, const U_NUM_TRIVERTEX nTriVert, const U_NUM_GRADOBJ nGradObj, 
                    const uint32_t ulMode, const PU_TRIVERTEX TriVert, const uint32_t *GradObj );
// U_EMRSETLINKEDUFIS_set            119  Not implemented
// U_EMRSETTEXTJUSTIFICATION_set     120  Not implemented (denigrated)
// U_EMRCOLORMATCHTOTARGETW_set      121  Not implemented  
char *U_EMRCREATECOLORSPACEW_set(const uint32_t ihCS, const U_LOGCOLORSPACEW lcs, const uint32_t dwFlags,
                    const U_CBDATA cbData, const uint8_t *Data);

#ifdef __cplusplus
}
#endif

#endif /* _UEMF_ */
