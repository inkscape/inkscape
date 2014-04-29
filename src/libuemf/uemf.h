/**
  @file uemf.h
  
  @brief Structures, definitions, and function prototypes for EMF files.

  EMF file Record structure information has been derived from Mingw, Wine, and libEMF header files, and from
  Microsoft's EMF Information pdf, release date March 28,2012, link from here:
  
     http://msdn2.microsoft.com/en-us/library/cc230514.aspx
  
  If the direct link fails the document may be found
  by searching for: "[MS-EMF]: Enhanced Metafile Format"
  
*/

/** \mainpage libUEMF overview
\section ov Overview
Microsoft's WMF, EMF, and EMF+ metafile types are supported.  In each case functions are provided for reading, constructing, writing, and printing 
metafile records.  The methods used to do that differ somewhat between metafiles, and the simplest
way to get started is to have a look at the example programs provided.  The WMF, EMF, and EMF+ structs and functions are 
marked with U_WMF, U_EMF, U_PMF and U_WMR, U_EMR, and U_PMR prefixes.  (PMF because "+" is a reserved character
in many contexts, so U_EMF+NAME would be a problem.)  Please be aware that normally both EMF and EMF+ files have the ".emf"
file extension, and that it is very common for such files to contain both an EMF and an EMF+ representation of the 
drawing.

\section example_sec Example Programs
testbed_emf.c  Creates an  EMF file test_libuemf.emf.\n
testbed_wmf.c  Creates a WMF file test_libuemf.wmf.\n
testbed_pmf.c  Creates an EMF+ file test_libuemf_p.emf.\n
reademf.c  Reads an EMF or EMF+ file and emits a text summary of its records.\n
readwmf.c  Reads a WMF file and emits a text summary of its records.\n
emf-inout.cpp.example Example code from Inkscape to convert graphics from EMF to SVG.\n
emf-print.cpp.example Example code from Inkscape to print a drawing to EMF.\n 
wmf-inout.cpp.example Example code from Inkscape to convert graphics from WMF to SVG.\n
wmf-print.cpp.example Example code from Inkscape to print a drawing to WMF.

\section doxy_limits Documentation issues
There are currently some unresolved issues with Doxygen that result in some structs
not being "defined".  This comes up when several different types of structs have the same
layout.  When this occurs the first one listed on the "typedef struct" is defined but all the
others will only be shown under "typedef struct" referring to the first one.  This is why
clicking on U_RECTL in a function parameter jumps to a typedef struct page,  why U_RECTL is shown
as plain text here, but U_RECT is shown as a link here, and clicking on it jumps directly
to its structure definition.

An additional issue is that the Enumeration names used in WMF are different from those
used in EMF, even when the values are either identical or differ only slightly, and no method 
has been found yet to link one to the other in Doxygen.  At present the only way to look up 
these WMF enumerations is by referencing the following table:

        EMF                                        WMF                                              WMF Manual
        EMF Binary Raster Operation Enumeration    BinaryRasterOperation Enumeration                2.1.1.2
        EMF Bitcount Enumeration                   BitCount Enumeration                             2.1.1.3
        EMF LB_Style Enumeration                   BrushStyle Enumeration                           2.1.1.4
        EMF LF_CharSet Enumeration                 CharacterSet Enumeration                         2.1.1.5
        EMF DIBColors Enumeration                  ColorUsage Enumeration  [has 1 extra value]      2.1.1.6
        EMF BI_Compression Enumeration             Compression Enumeration [has 3 extra values]     2.1.1.7
        -                                          FamilyFont Enumeration                           2.1.1.8
        EMF FloodFill Enumeration                  FloodFill Enumeration                            2.1.1.9
        EMF LF_Quality Enumeration                 FontQuality Enumeration                          2.1.1.10
        EMF LCS_Intent Enumeration                 GamutMappingIntent Enumeration                   2.1.1.11
        EMF HatchStyle Enumeration                 HatchStyle Enumeration                           2.1.1.12
        EMF Mirroring Enumeration                  LayoutEnumeration                                2.1.1.13
        -                                          LogicalColorSpace Enumeration                    2.1.1.14
        EMF Profile Enumeration                    LogicalColorSpaceV5 Enumeration                  2.1.1.15
        EMF MapMode Enumeration                    MapModeEnumeration                               2.1.1.16
        -                                          MetaFilesEscape Enumeration                      2.1.1.17
        -                                          MetafileType Enumeration                         2.1.1.18
        -                                          MetafileVersion Enumeration                      2.1.1.19
        EMF BackgroundMode Enumeration             MixModeEnumeration                               2.1.1.20
        EMF LF_OutPrecision Enumeration            OutPrecision Enumeration                         2.1.1.21
        -                                          PaletteEntryFlag Enumeration                     2.1.1.22
        EMF PenStyle Enumeration                   PenStyle Enumeration [not values >0xFFFF]        2.1.1.23
        -                                          PitchFont Enumeration                            2.1.1.24
        EMF PolygonFillMode Enumeration            PolyFillMode Enumeration [first 2 only]          2.1.1.25
        -                                          PostScriptCap Enumeration                        2.1.1.26
        -                                          PostScriptClipping Enumeration                   2.1.1.27
        -                                          PostFeatureSetting Enumeration                   2.1.1.28
        -                                          PostScrioptJoin Enumeration                      2.1.1.29
        EMF StretchMode Enumeration                StretchMode Enumeration                          2.1.1.30
        EMF Ternary Raster Operation Enumeration   TernaryRasterOperation  Enumeration              2.1.1.31
        EMF LF_ClipPrecision Enumeration           ClipPrecision Flags                              2.1.2.1
        EMF ExtTextOutOptions Enumeration          ExtTextOutOptions Flags [subset]                 2.1.2.2
        EMF TextAlignment Enumeration              TextAlignment Enumeration                        2.1.2.3
        EMF TextAlignment Enumeration              VertialTextAlignment Enumeration                 2.1.2.4
        EMF LF_PitchAndFamily Enumeration          PitchAndFamily Enumerations                      2.2.2.14
        
\section refs Reference documentation

        Manual   Date        Link
        EMF      3/28/2012   http://msdn2.microsoft.com/en-us/library/cc230514.aspx
        EMF+     7/5/2012    http://msdn.microsoft.com/en-us/library/cc230724.aspx
        WMF      7/5/2012    http://msdn2.microsoft.com/en-us/library/cc250370.aspx 
*/

/*
File:      uemf.h
Version:   0.0.27
Date:      28-MAR-2014
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2014 David Mathog and California Institute of Technology (Caltech)
*/

#ifndef _UEMF_
#define _UEMF_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "uemf_utf.h"
#include "uemf_endian.h"


/** \cond */
//  ***********************************************************************************
//  defines not placed yet

#define U_PAN_CULTURE_LATIN              0

#define U_SYSPAL_ERROR    0
#define U_SYSPAL_STATIC   1
#define U_SYSPAL_NOSTATIC 2

#define U_ELF_VENDOR_SIZE 4

#define UNUSED_PARAMETER(x) (void)(x)
/** \endcond */

//  ***************************************************************************
/** \defgroup U_EMF_Miscellaneous_values EMF Miscellaneous Values
  @{
*/
#define U_NONE                        0               //!< Generic for nothing selected for all flag fields
#define U_PI                          3.14159265358979323846	//!< pi
#define U_READ                        1               //!< open file as "rb"
#define U_WRITE                       0               //!< open file as "wb"
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
#define U_REC_FREE                    1               //!< use with emf_append
#define U_REC_KEEP                    0               //!< use with emf_append
#define U_ROW_ORDER_INVERT            1               //!< For RGBA_to_DIB, invert row order in DIB relative to pixel array
#define U_ROW_ORDER_SAME              0               //!< For RGBA_to_DIB, same   row order in DIB as in pixel array
#define U_CT_NO                       0               //!< For RGBA_to_DIB, do not use color table
#define U_CT_BGRA                     1               //!< For RGBA_to_DIB, use color table (16 bits or less only) BGRA colors, compatible with EMF+ ARGB
#define U_CT_ARGB                     1               //!< For RGBA_to_DIB, use color table (16 bits or less only) BGRA colors, compatible with EMF+ ARGB
#define U_EMR_COMMENT_SPOOLFONTDEF    0x544F4E46      //!< For U_EMRCOMMENT record that is U_EMR_COMMENT_SPOOL, comment holds font definition informtion.
/** Solaris 8 has problems with round/roundf, just use this everywhere  */
#define U_ROUND(A)  ( (A) > 0 ? floor((A)+0.5) : ( (A) < 0 ? -floor(-(A)+0.5) : (A) ) )

/** @} */


typedef float      U_FLOAT;                 //!< 32 bit float

typedef uint32_t   U_CBBITS;                //!< Count of Bytes in object at corresponding U_OFF*
typedef uint32_t   U_CBBITSMSK;             //!< Count of Bytes in object at corresponding U_OFF*
typedef uint32_t   U_CBBITSSRC;             //!< Count of Bytes in object at corresponding U_OFF*
typedef uint32_t   U_CBBMI;                 //!< Count of Bytes in object at corresponding U_OFF*
typedef uint32_t   U_CBBMIMSK;              //!< Count of Bytes in object at corresponding U_OFF*
typedef uint32_t   U_CBBMISRC;              //!< Count of Bytes in object at corresponding U_OFF*
typedef uint32_t   U_CBDATA;                //!< Count of Bytes in object at corresponding U_OFF*
typedef uint32_t   U_CBNAME;                //!< Count of Bytes in object at corresponding U_OFF*
typedef uint32_t   U_CBPLENTRIES;           //!< Count of Bytes in object at corresponding U_OFF*
typedef uint32_t   U_CBPXLFMT;              //!< Count of Bytes in object at corresponding U_OFF*
typedef uint32_t   U_CBRGNDATA;             //!< Count of Bytes in object at corresponding U_OFF*
typedef uint32_t   U_CBSTR;                 //!< Count of Bytes in an 8 or 16 bit string

typedef uint32_t   U_OFFBITS;               //!< Byte offset to TYPE, always measured from the start of the RECORD (not the struct)
typedef uint32_t   U_OFFBITSMSK;            //!< Byte offset to TYPE, always measured from the start of the RECORD (not the struct)
typedef uint32_t   U_OFFBITSSRC;            //!< Byte offset to TYPE, always measured from the start of the RECORD (not the struct)
typedef uint32_t   U_OFFBMI;                //!< Byte offset to TYPE, always measured from the start of the RECORD (not the struct)
typedef uint32_t   U_OFFBMIMSK;             //!< Byte offset to TYPE, always measured from the start of the RECORD (not the struct)
typedef uint32_t   U_OFFBMISRC;             //!< Byte offset to TYPE, always measured from the start of the RECORD (not the struct)
typedef uint32_t   U_OFFDATA;               //!< Byte offset to TYPE, always measured from the start of the RECORD (not the struct)
typedef uint32_t   U_OFFDESC;               //!< Byte offset to TYPE, always measured from the start of the RECORD (not the struct)
typedef uint32_t   U_OFFDX;                 //!< Byte offset to TYPE, always measured from the start of the RECORD (not the struct)
typedef uint32_t   U_OFFPLENTRIES;          //!< Byte offset to TYPE, always measured from the start of the RECORD (not the struct)
typedef uint32_t   U_OFFPXLFMT;             //!< Byte offset to TYPE, always measured from the start of the RECORD (not the struct)
typedef uint32_t   U_OFFSTR;                //!< Byte offset to string of either 8 or 16 bit characters
typedef uint8_t    U_DATA;                  //!< any binary sort of data, not otherwise classified.

//  "Types" For array components in structures, where not otherwise defined as a structure
typedef uint32_t   U_FNTAXES;               //!< Font Axes For U_DESIGNVECTOR 
typedef uint32_t   U_STYLEENTRY;            //!< StyleEntry For U_EXTLOGPEN
typedef uint32_t   U_POLYCOUNTS;            //!< aPolyCounts For U_EMRPOLYPOLYLINE etc.

//  "Counts" for array components in structures
typedef uint32_t   U_NUM_FNTAXES;           //!< Number of U_FNTAXES
typedef uint32_t   U_NUM_LOGPLTNTRY;        //!< Number of U_LOGPLTENTRY
typedef uint32_t   U_NUM_RECTL;             //!< Number of U_RECTL
typedef uint32_t   U_NUM_POINTL;            //!< Number of U_POINTL
typedef uint32_t   U_NUM_POINT16;           //!< Number of U_POINT16
typedef uint32_t   U_NUM_STYLEENTRY;        //!< Number of U_STYLEENTRY
typedef uint32_t   U_NUM_POLYCOUNTS;        //!< Number of U_POLYCOUNTS
typedef uint32_t   U_NUM_EMRTEXT;           //!< Number of U_EMRTEXT
typedef uint32_t   U_NUM_STR;               //!< Number of 8 or 16 bit characters in string
typedef uint32_t   U_NUM_TRIVERTEX;         //!< Number of U_TRIVERTEX
typedef uint32_t   U_NUM_GRADOBJ;           //!< Number of U_GRADIENT4 OR U_GRADIENT3 (determined at run time)
typedef uint32_t   U_NUM_RGBQUAD;           //!< Number of U_RGBQUAD (in bmciColors in U_BITMAPCOREINFO)



/* ************************ WMF pieces used in EMF or EMF+ ****************************** */

/** \defgroup U_EMF_EMRSETROP2_iMode_Qualifiers EMF Binary Raster Operation Enumeration

  For U_EMRSETROP2 iMode field
  Microsoft name: Binary Raster Operation Enumeration
  WMF manual 2.1.1.2
  
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
#define U_R2_BLACK        1 //!< BLACK      
#define U_R2_NOTMERGEPEN  2 //!< NOTMERGEPEN
#define U_R2_MASKNOTPEN   3 //!< MASKNOTPEN 
#define U_R2_NOTCOPYPEN   4 //!< NOTCOPYPEN 
#define U_R2_MASKPENNOT   5 //!< MASKPENNOT 
#define U_R2_NOT          6 //!< NOT        
#define U_R2_XORPEN       7 //!< XORPEN     
#define U_R2_NOTMASKPEN   8 //!< NOTMASKPEN 
#define U_R2_MASKPEN      9 //!< MASKPEN    
#define U_R2_NOTXORPEN   10 //!< NOTXORPEN  
#define U_R2_NOP         11 //!< NOP        
#define U_R2_MERGENOTPEN 12 //!< MERGENOTPEN
#define U_R2_COPYPEN     13 //!< COPYPEN    
#define U_R2_MERGEPENNOT 14 //!< MERGEPENNOT
#define U_R2_MERGEPEN    15 //!< MERGEPEN   
#define U_R2_WHITE       16 //!< WHITE      
#define U_R2_LAST        16 //!< LAST       
/** @} */

/** \defgroup U_EMF_BITMAPINFOHEADER_biBitCount_Qualifiers EMF BitCount Enumeration
  For U_BITMAPINFOHEADER biBitCount field.
  Microsoft name: Bitcount Enumeration
  WMF manual 2.1.1.3
    @{
*/
#define U_BCBM_EXPLICIT   0  //!< Derived from JPG or PNG compressed image or ?
#define U_BCBM_MONOCHROME 1  //!< 2 colors.    bmiColors array has two entries
#define U_BCBM_COLOR4     4  //!< 2^4 colors.  bmiColors array has 16 entries
#define U_BCBM_COLOR8     8  //!< 2^8 colors.  bmiColors array has 256 entries
#define U_BCBM_COLOR16   16  //!< 2^16 colors. bmiColors is not used. Pixels are 5 bits B,G,R with 1 unused bit
#define U_BCBM_COLOR24   24  //!< 2^24 colors. bmiColors is not used. Pixels are U_RGBTRIPLE.
#define U_BCBM_COLOR32   32  //!< 2^32 colors. bmiColors is not used. Pixels are U_RGBQUAD.  Also use for EMF+ ARGB
/** @} */

/** \defgroup U_EMF_BITMAPINFOHEADER_biCompression_Qualifiers EMF BI_Compression Enumeration
  For U_BITMAPINFOHEADER biCompression field
  Microsoft name: Compression Enumeration
  WMF manual 2.1.1.7
  @{
*/
#define U_BI_UNKNOWN      -1  //!< not defined in EMF standard, not to be used in EMF files
#define U_BI_RGB           0  //!< Supported by libUEMF
#define U_BI_RLE8          1  //!< NOT supported by libUEMF
#define U_BI_RLE4          2  //!< NOT supported by libUEMF
#define U_BI_BITFIELDS     3  //!< Supported by libUEMF
#define U_BI_JPEG          4  //!< Supported by libUEMF
#define U_BI_PNG           5  //!< Supported by libUEMF
/** @} */

/** \defgroup U_EMF_LOGCOLORSPACE_lcsIntent_Qualifiers EMF LCS_Intent Enumeration
  For U_LOGCOLORSPACEA/U_LOGCOLORSPACEW lcsIntent field 
  Microsoft name: LCS_Intent Enumeration
  WMF manual 2.1.1.11
  @{
*/
#define U_LCS_GM_BUSINESS         0x00000001L //!<  BUSINESS        
#define U_LCS_GM_GRAPHICS         0x00000002L //!<  GRAPHICS        
#define U_LCS_GM_IMAGES           0x00000004L //!<  IMAGES          
#define U_LCS_GM_ABS_COLORIMETRIC 0x00000008L //!<  ABS_COLORIMETRIC
/** @} */

/** \defgroup U_EMF_LOGCOLORSPACE_lcsCSType_Qualifiers EMF LCS_CSType Enumeration
  For U_LOGCOLORSPACEA/U_LOGCOLORSPACEW lcsCSType field
  Microsoft name: LCS_CSType Enumeration
  WMF manual 2.1.1.14
  @{
*/
#define U_LCS_CALIBRATED_RGB 0x00000000L //!< CALIBRATED_RGB
#define U_LCS_DEVICE_RGB     0x00000001L //!< DEVICE_RGB    
#define U_LCS_DEVICE_CMYK    0x00000002L //!< DEVICE_CMYK   
/** @} */

/** \defgroup U_EMF_EMR_dwROP_Qualifiers EMF Ternary Raster Operation enumeration

  For U_EMR* dwROP fields.
  Microsoft name: Ternary Raster Operation enumeration
  WMF manual 2.1.1.31
  
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
#define U_SRCCOPY        0x00cc0020 //!< SRCCOPY    
#define U_SRCPAINT       0x00ee0086 //!< SRCPAINT   
#define U_SRCAND         0x008800c6 //!< SRCAND     
#define U_SRCINVERT      0x00660046 //!< SRCINVERT  
#define U_SRCERASE       0x00440328 //!< SRCERASE   
#define U_NOTSRCCOPY     0x00330008 //!< NOTSRCCOPY 
#define U_NOTSRCERASE    0x001100a6 //!< NOTSRCERASE
#define U_MERGECOPY      0x00c000ca //!< MERGECOPY  
#define U_MERGEPAINT     0x00bb0226 //!< MERGEPAINT 
#define U_PATCOPY        0x00f00021 //!< PATCOPY    
#define U_PATPAINT       0x00fb0a09 //!< PATPAINT   
#define U_PATINVERT      0x005a0049 //!< PATINVERT  
#define U_DSTINVERT      0x00550009 //!< DSTINVERT  
#define U_BLACKNESS      0x00000042 //!< BLACKNESS  
#define U_WHITENESS      0x00ff0062 //!< WHITENESS  
#define U_NOOP           0x00aa0029 //!< Many GDI programs end with a bitblt with this ROP == "D".  Seems to work like flush()
#define U_NOMIRRORBITMAP 0x80000000 //!< If bit set, disable horizontal reflection of bitmap.
/** @} */

/** \defgroup U_EMF_EMRSETTEXTALIGN_iMode_Qualifiers EMF TextAlignment Enumeration
  For U_EMRSETTEXTALIGN iMode field
  Microsoft name: TextAlignment Enumeration
  WMF Manual 2.1.2.3
  WMF Manual 2.1.2.4
  
  Recall that EMF coordinates have UL  closest to {0,0}, LR is below and to the right of UL and so has LARGER
  {x,y} coordinates.  In the following "TOP" is on the horizontal line defined by LR, as it has larger y coordinates,
  which when viewing the EMF file, would actually be on the BOTTOM of the bounding rectangle.  Similarly, left and right
  are reversed.
  
  Microsoft documentation (WMF  manual, section 2.1.2.3) says that the text starts on certain edges of the bounding rectangle.
  That is apparently not true, whether the bounding rectangle is {0,0,-1,-1}, which is effectively no bounding rectangle,
  or if a valid bounding rectangle is specified.  In all cases the text (in Windows XP Preview) starts, has center at, or ends 
  at the center point.  Vertical offsets seem to be defined analogously, but with respect to the height of the font.  The bounding
  rectangle defined for the U_EMRTEXT record appears to be ignored.

  Microsoft documentation (EMF  manual,section 2.2.5) says that the same rectangle is used for "clipping or opaquing" by ExtTextOutA/W.
  That does not seem to occur either.

  @{
*/
//  Horizontal text flags
#define U_TA_DEFAULT    0x00                //!< default alignment
#define U_TA_NOUPDATECP 0x00                //!< Reference point does not move
#define U_TA_UPDATECP   0x01                //!< Reference point moves to end of next text drawn.
#define U_TA_LEFT       0x00                //!< Reference point is on left edge of bounding rectangle
#define U_TA_RIGHT      0x02                //!< Reference point is on right edge of bounding rectangle
#define U_TA_CENTER     0x06                //!< Reference point is on center vertical line of bounding rectangle
#define U_TA_TOP        0x00                //!< Reference point is on top edge of bounding rectangle
#define U_TA_BOTTOM     0x08                //!< Reference point is on bottom edge of bounding rectangle
#define U_TA_BASEBIT    0x10                //!< Reference point is on baseline of text if this bit is set, for 0x10 <-> 0x18
#define U_TA_BASELINE   0x18                //!< Reference point is on baseline of text
#define U_TA_RTLREADING 0x100               //!< Set for Right to Left languages like Hebrew and Arabic
#define U_TA_MASK       U_TA_BASELINE+U_TA_CENTER+U_TA_UPDATECP+U_TA_RTLREADING //!< Mask for these bits
//  Vertical text flags
#define U_VTA_BASELINE  U_TA_BASELINE       //!< same meaning, but for vertical text
#define U_VTA_LEFT      U_TA_BOTTOM         //!< same meaning, but for vertical text
#define U_VTA_RIGHT     U_TA_TOP            //!< same meaning, but for vertical text
#define U_VTA_CENTER    U_TA_CENTER         //!< same meaning, but for vertical text
#define U_VTA_BOTTOM    U_TA_RIGHT          //!< same meaning, but for vertical text
#define U_VTA_TOP       U_TA_LEFT           //!< same meaning, but for vertical text
/** @} */

/** WMF manual 2.2.2.3
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
} U_BITMAPINFOHEADER,
  *PU_BITMAPINFOHEADER;                     //!< WMF manual 2.2.2.3

/** WMF manual 2.2.2.6
  \brief For U_CIEXYZTRIPLE (all) fields

  Microsoft name: CIEXYZ Object
*/
typedef struct {
    int32_t             ciexyzX;            //!< CIE color space X component
    int32_t             ciexyzY;            //!< CIE color space Y component
    int32_t             ciexyzZ;            //!< CIE color space Z component
} U_CIEXYZ,
  *PU_CIEXYZ;                               //!< WMF manual 2.2.2.6

/** WMF manual 2.2.2.7
  \brief For U_LOGCOLORSPACEA and U_LOGCOLORSPACEW lcsEndpints field

  defines a CIE colorspace.
  Microsoft name: CIEXYZTRIPLE Object
  
*/
typedef struct {
    U_CIEXYZ            ciexyzRed;          //!< CIE XYZ coord of red   endpoint of colorspace
    U_CIEXYZ            ciexyzGreen;        //!< CIE XYZ coord of green endpoint of colorspace
    U_CIEXYZ            ciexyzBlue;         //!< CIE XYZ coord of blue  endpoint of colorspace
} U_CIEXYZTRIPLE,
  *PU_CIEXYZTRIPLE;                         //!< WMF manual 2.2.2.7

/** WMF manual 2.2.2.8
  \brief For U_BITMAPINFO crColor field

  NOTE that the color order is RGB reserved, flipped around from the preceding.
  Microsoft name: COLORREF Object
*/
typedef struct {
    uint8_t             Red;                //!< Red   color (0-255)
    uint8_t             Green;              //!< Green color (0-255)
    uint8_t             Blue;               //!< Blue  color (0-255) 
    uint8_t             Reserved;           //!< Not used
} U_COLORREF,
  *PU_COLORREF;                             //!< WMF manual 2.2.2.8

/** WMF manual 2.2.2.11
  \brief For U_LCS_GAMMARGB lcsGamma* fields

  Microsoft name:(unknown)
*/
typedef struct {
    unsigned             ignoreHi :8;       //!< not used
    unsigned             intPart  :8;       //!< integer part
    unsigned             fracPart :8;       //!< fraction part
    unsigned             ignoreLo :8;       //!< not used
} U_LCS_GAMMA,
  *PU_LCS_GAMMA;                            //!< WMF manual 2.2.2.11

/** WMF manual 2.2.2.11
  \brief For U_LOGCOLORSPACEA and U_LOGCOLORSPACEW lcsGammaRGB field

  Microsoft name:(unknown)
*/
typedef struct {
    U_LCS_GAMMA         lcsGammaRed;        //!< Red   Gamma
    U_LCS_GAMMA         lcsGammaGreen;      //!< Green Gamma
    U_LCS_GAMMA         lcsGammaBlue;       //!< Blue  Gamma
} U_LCS_GAMMARGB,
  *PU_LCS_GAMMARGB;                         //!< WMF manual 2.2.2.11

/** WMF manual 2.2.2.11
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
} U_LOGCOLORSPACEA,
  *PU_LOGCOLORSPACEA;                       //!< WMF manual 2.2.2.11

/** WMF manual 2.2.2.12
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
} U_LOGCOLORSPACEW,
  *PU_LOGCOLORSPACEW;                       //!< WMF manual 2.2.2.12

/**  WMF manual 2.2.2.15
  \brief Used for any generic pair of uint32_t

  Microsoft name: POINTL Object
*/
typedef struct {
    int32_t x;                              //!< X value
    int32_t y;                              //!< Y value
} U_PAIR, 
  U_POINT,                                  //!< WMF manual 2.2.2.15
  U_POINTL,                                 //!< WMF manual 2.2.2.15
  *PU_PAIR,                                 //!< WMF manual 2.2.2.15
  *PU_POINT,                                //!< WMF manual 2.2.2.15
  *PU_POINTL;                               //!< WMF manual 2.2.2.15


/** WMF manual 2.2.2.16
  \brief Point type for 16 bit EMR drawing functions.

  Microsoft name: POINTS Object.
  Microsoft name: POINTS16 Object.
*/
typedef struct {
    int16_t x;                              //!< X size (16 bit)
    int16_t y;                              //!< Y size (16 bit)
} U_POINT16,
  *PU_POINT16;                              //!< WMF manual 2.2.2.16

/** WMF manual 2.2.2.19
  \brief Coordinates of the upper left, lower right corner.

  Note that the coordinate system is 0,0 in the upper left corner
  of the screen an N,M in the lower right corner.
  Microsoft name: RECTL Object
*/
typedef struct {
    int32_t  left;                          //!< left coordinate
    int32_t  top;                           //!< top coordinate
    int32_t  right;                         //!< right coordinate
    int32_t  bottom;                        //!< bottom coordinate
} U_RECT, 
  U_RECTL,                                  //!< WMF manual 2.2.2.19
  *PU_RECT,                                 //!< WMF manual 2.2.2.19
  *PU_RECTL;                                //!< WMF manual 2.2.2.19

/** WMF manual 2.2.2.20
  \brief For U_BITMAPINFO bmiColors field

  NOTE that the color order is BGR, even though the name is RGB!
  Microsoft name: RGBQUAD Object
*/
typedef struct {
    uint8_t             Blue;               //!< Blue  color (0-255) 
    uint8_t             Green;              //!< Green color (0-255)
    uint8_t             Red;                //!< Red   color (0-255)
    uint8_t             Reserved;           //!< Not used
} U_RGBQUAD,
  *PU_RGBQUAD;                              //!< WMF manual 2.2.2.20

#define U_RCL_DEF (U_RECTL){0,0,-1,-1}  //!< Use this when no bounds are needed. 

/** WMF manual 2.2.2.22
  \brief Pair of values indicating x and y sizes.

  Microsoft name: SIZE Object
  Microsoft name: SIZEL Object
*/
typedef struct {
    int32_t cx;                             //!< X size
    int32_t cy;                             //!< Y size
} U_SIZE, 
  U_SIZEL,                                  //!< WMF manual 2.2.2.22
  *PU_SIZE,                                 //!< WMF manual 2.2.2.22
  *PU_SIZEL;                                //!< WMF manual 2.2.2.22



/* ************************ EMF or common to EMF and EMF+ ****************************** */

//  ***********************************************************************************
//  Value enumerations and other predefined constants, alphabetical order by group


/** \defgroup U_EMF_FONT_STRUCT_WIDTHS EMF Font name and style widths in characters
  For U_LOGFONT and U_LOGFONT_PANOSE, 
  @{
*/
#define U_LF_FACESIZE     32    //!< U_LOGFONT lfFaceName and U_LOGFONT_PANOSE elfStyle fields maximum width
#define U_LF_FULLFACESIZE 64    //!< U_LOGFONT_PANOSE elfFullName field maximum width
/** @} */

/** \defgroup U_EMF_EMR_Qualifiers EMF RecordType Enumeration
  (RecordType Enumeration, EMF manual 2.1.1 )
  For U_EMR iType field
  EMF manual 2.1.1
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


/** \defgroup U_EMF_DRAW_PROPERTIES EMF draw properties
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
/** \defgroup U_EMF_EMRSETARCDIRECTION_Qualifiers  EMF ArcDirection Enumeration
  For U_EMRSETARCDIRECTION iArcDirection field 
  Microsoft name: ArcDirection Enumeration
  EMF manual 2.1.2
  @{
*/
#define U_AD_COUNTERCLOCKWISE 1 //!< Draw arc counterclockwise.
#define U_AD_CLOCKWISE        2 //!< Draw arc clockwise.
/** @} */

/** \defgroup U_EMF_PANOSE_bArmStyle_Qualifiers EMF ArmStyle Enumeration
  For U_PANOSE bArmStyle field
  Microsoft name: ArmStyle Enumeration
  EMF manual 2.1.3
  @{
*/
#define U_PAN_STRAIGHT_ARMS_HORZ          2 //!< straight arms horizontal
#define U_PAN_STRAIGHT_ARMS_WEDGE         3 //!< straight arms wedge
#define U_PAN_STRAIGHT_ARMS_VERT          4 //!< straight arms vertical
#define U_PAN_STRAIGHT_ARMS_SINGLE_SERIF  5 //!< straight arms singleserif
#define U_PAN_STRAIGHT_ARMS_DOUBLE_SERIF  6 //!< straight arms doubleserif
#define U_PAN_BENT_ARMS_HORZ              7 //!< bent arms horizontal
#define U_PAN_BENT_ARMS_WEDGE             8 //!< bent arms wedge
#define U_PAN_BENT_ARMS_VERT              9 //!< bent arms vertical
#define U_PAN_BENT_ARMS_SINGLE_SERIF     10 //!< bent arms singleserif
#define U_PAN_BENT_ARMS_DOUBLE_SERIF     11 //!< bent arms doubleserif
/** @} */

/** \defgroup U_EMF_EMRSETBKMODE_iMode_Qualifiers EMF BackgroundMode enumeration
  For U_EMRSETBKMODE iMode field
  Microsoft name: BackgroundMode enumeration
  EMF manual 2.1.4
  @{
*/
#define U_TRANSPARENT  1 //!< Transparent background mode
#define U_OPAQUE       2 //!< Opaque background mode
/** @} */

/** \defgroup U_EMF_COLORADJUSTMENT_caFlags_Qualifiers EMF ColorAdjustment Enumeration
  For U_COLORADJUSTMENT caFlags field
  Microsoft name: ColorAdjustment Enumeration
  EMF manual 2.1.5
  @{
*/
#define U_CA_NEGATIVE          0x0001 //!< display Negative of image
#define U_CA_LOG_FILTER        0x0002 //!< display Logarithmi filter of image
/** @} */

/** \defgroup U_EMF_EMRCOLORMATCHTOTARGETW_dwFlags_Qualifiers EMF ColorMatchToTarget Enumeration
  For U_EMRCOLORMATCHTOTARGETW dwFlags field
  Microsoft name: ColorMatchToTarget Enumeration
  EMF manual 2.1.6
  @{
*/
#define U_COLORMATCHTOTARGET_NOTEMBEDDED       0 //!< Color match profile is not embedded in metafile
#define U_COLORMATCHTOTARGET_EMBEDDED          1 //!< Color match profile is  embedded in metafile
/** @} */

/** \defgroup U_EMF_EMRCOLORMATCHTOTARGETW_dwAction_Qualifiers EMF ColorSpace Enumeration
  For U_EMRCOLORMATCHTOTARGETW dwAction field 
  Microsoft name: ColorSpace Enumeration
  EMF manual 2.1.7
  @{
*/
#define U_CS_ENABLE            1 //!< Enable color proofing.
#define U_CS_DISABLE           2 //!< Disable color proofing.
#define U_CS_DELETE_TRANSFORM  3 //!< Disable proofing and delete color transform.
/** @} */

/** \defgroup U_EMF_PANOSE_common_Qualifiers EMF PanoseCommon Enumeration
  Used by all PAN_* enumerations, but only defined once here.  
  See also U_PAN_ALL1 after the U_PANOSE structure
  @{
*/
#define U_PAN_ANY     0 //!< Any (for any type of Panose enumeration)
#define U_PAN_NO_FIT  1 //!< No fit (for any type of Panose enumeration)
/** @} */

/** \defgroup U_EMF_PANOSE_bContrast_Qualifiers EMF Contrast Enumeration
  For U_PANOSE bContrast field
  Microsoft name: Contrast Enumeration
  EMF manual 2.1.8
  @{
*/
#define U_PAN_CONTRAST_NONE              2 //!< None
#define U_PAN_CONTRAST_VERY_LOW          3 //!< Very low
#define U_PAN_CONTRAST_LOW               4 //!< Low
#define U_PAN_CONTRAST_MEDIUM_LOW        5 //!< Medium low
#define U_PAN_CONTRAST_MEDIUM            6 //!< Medium
#define U_PAN_CONTRAST_MEDIUM_HIGH       7 //!< Medium high
#define U_PAN_CONTRAST_HIGH              8 //!< High
#define U_PAN_CONTRAST_VERY_HIGH         9 //!< Very high
/** @} */

/** \defgroup U_EMF_DIBITS_iUsageSrc_Qualifiers EMF DIBColors Enumeration
  For U_EMRSETDIBITSTODEIVCE and U_EMRSTRETCHDIBITS iUsageSrc fields.
  Microsoft name: DIBColors Enumeration
  EMF manual 2.1.9
  @{
*/
#define U_DIB_RGB_COLORS   0 //!< color table contains colors
#define U_DIB_PAL_COLORS   1 //!< color table contains 16 bit indices into logical palette
#define U_DIB_PAL_INDICES  2 //!< no color table, pixel values are indices into logical palette
/** @} */

/** \defgroup U_EMF_EMR_COMMENT_PUBLIC EMF EMRComment Enumeration
  For U_EMRCOMMENT_PUBLIC pcIdent fields
  Microsoft name: EMRComment Enumeration
  EMF manual 2.1.10
  @{
*/
#define U_EMR_COMMENT_WINDOWS_METAFILE  0x80000001 //!< Comment contains WMF
#define U_EMR_COMMENT_BEGINGROUP        0x00000002 //!< Comment begins group of EMF records
#define U_EMR_COMMENT_ENDGROUP          0x00000003 //!< Comment ends group of EMF records
#define U_EMR_COMMENT_MULTIFORMATS      0x40000004 //!< Comment contains some other representation of drawing
#define U_EMR_COMMENT_UNICODE_STRING    0x00000040 //!< Reserved
#define U_EMR_COMMENT_UNICODE_END       0x00000080 //!< Reserved
/** @} */

/** \defgroup U_EMF_EMRTEXT_foptions_Qualifiers EMF ExtTextOutOptions Enumeration
  For U_EMRTEXT foptions field
  Microsoft name: ExtTextOutOptions Enumeration
  EMF manual 2.1.11
  @{
*/
#define U_ETO_NONE              0x00000000 //!< None
#define U_ETO_GRAYED            0x00000001 //!< Grayed
#define U_ETO_OPAQUE            0x00000002 //!< Fill rectangle with background color.
#define U_ETO_CLIPPED           0x00000004 //!< Clip text to rectangle.
#define U_ETO_GLYPH_INDEX       0x00000010 //!< Characters are glyph indices for the font.
#define U_ETO_RTLREADING        0x00000080 //!< Right to left text.
#define U_ETO_NO_RECT           0x00000100 //!< No bounding rectangle is specified.
#define U_ETO_SMALL_CHARS       0x00000200 //!< 8 bit characters instead of 16 bit.  For EMRSMALLTEXTOUT ONLY, does not affect EMRTEXTOUTA or EMRTEXTOUTW
#define U_ETO_NUMERICSLOCAL     0x00000400 //!< Show numbers for the current locale.
#define U_ETO_NUMERICSLATIN     0x00000800 //!< Show numbers using European digits.
#define U_ETO_IGNORELANGUAGE    0x00001000 //!< Process Right to Left languages exactly as specified in the metafile.
#define U_ETO_PDY               0x00002000 //!< Both horizontal and vertical displacements are provided.
#define U_ETO_REVERSE_INDEX_MAP 0x00010000 //!< Reverse_index_map
/** @} */

/** \defgroup U_EMF_PANOSE_bFamilyType_Qualifiers EMF FamilyType Enumeration
  For U_PANOSE bFamilyType field
  Microsoft name: FamilyType Enumeration
  EMF manual 2.1.12
  @{
*/
#define U_PAN_FAMILY_TEXT_DISPLAY        2 //!< Text display
#define U_PAN_FAMILY_SCRIPT              3 //!< Script
#define U_PAN_FAMILY_DECORATIVE          4 //!< Decorative
#define U_PAN_FAMILY_PICTORIAL           5 //!< Pictorial
/** @} */

/** \defgroup U_EMF_EMREXTFLOODFILL_iMode_Qualifiers EMF FloodFill Enumeration
  For U_EMREXTFLOODFILL iMode field
  Microsoft name: FloodFill Enumeration
  EMF manual 2.1.13
  @{
*/
#define U_FLOODFILLBORDER   0x00000000 //!< Color specified must be the same as the border - brush fill stops at this color
#define U_FLOODFILLSURFACE  0x00000001 //!< Color specified must be different from the border - brush fills only this color
/** @} */

/** \defgroup U_EMF_DESIGNVECTOR_Signature_Qualifiers EMF Signature Enumeration
  For U_DESIGNVECTOR Signature field
  Microsoft name: Signature Enumeration
  EMF manual 2.1.14
  @{
*/
#define U_ENHMETA_SIGNATURE 0x464D4520  //!< "EMF" signature also for U_EMRHEADER dSignature field.
#define U_EPS_SIGNATURE     0x46535045  //!< "FSPE" signature, indicates encapsulated postscript. 
/** @} */

/** \defgroup U_EMF_EMRGRADIENTFILL_ulMode_Qualifiers EMF GradientFill Enumeration
  For U_EMRGRADIENTFILL ulMode field
  Microsoft name: GradientFill Enumeration
  EMF manual 2.1.15
  @{
*/
#define U_GRADIENT_FILL_RECT_H     0x00000000 //!< Gradient is left to right.
#define U_GRADIENT_FILL_RECT_V     0x00000001 //!< Grident is top to bottom.
#define U_GRADIENT_FILL_TRIANGLE   0x00000002 //!< Gradient is between 3 vertices of a triangle.
/** @} */

/** \defgroup U_EMF_EMREXTTEXTOUT_iGraphicsMode_Qualifiers EMF GraphicsMode Enumeration
  For U_EMREXTTEXTOUTA/U_EMREXTTEXTOUTW and all other iGraphicsMode fields
  Microsoft name: GraphicsMode Enumeration
  EMF manual 2.1.16
  @{
*/
#define U_GM_COMPATIBLE     1 //!< TrueType text ignores world to device transform except for Scale.  Arcs ignore transform
#define U_GM_ADVANCED       2 //!< TrueType text and Arcs must conform to all of world to device transform.
#define U_GM_LAST           2 //!< Number of GraphicsMode Enumeration entries.
/** @} */

/** \defgroup U_EMF_LOGBRUSH_lbHatch_Qualifiers EMF HatchStyle Enumeration
  For U_LOGBRUSH lbHatch field
  Microsoft name: HatchStyle Enumeration
  EMF manual 2.1.17
  @{
*/
#define U_HS_HORIZONTAL       0 //!< Horizontal.
#define U_HS_VERTICAL         1 //!< Vertical.
#define U_HS_FDIAGONAL        2 //!< Forward diagonal.
#define U_HS_BDIAGONAL        3 //!< Back diagonal.
#define U_HS_CROSS            4 //!< Cross.
#define U_HS_DIAGCROSS        5 //!< Diagonal cross.
#define U_HS_SOLIDCLR         6 //!< Solid color.
#define U_HS_DITHEREDCLR      7 //!< Dithered color.
#define U_HS_SOLIDTEXTCLR     8 //!< Solid text color.
#define U_HS_DITHEREDTEXTCLR  9 //!< Dithered text color.
#define U_HS_SOLIDBKCLR      10 //!< Solid background color.
#define U_HS_DITHEREDBKCLR   11 //!< Dithered background color.
/** @} */

/** \defgroup U_EMF_EMRSETICMMODE_iMode_Qualifiers EMF ICMMode Enumeration
  For EMF U_EMR_SETICMMODE iMode field
  Microsoft name: ICMMode Enumeration
  EMF manual 2.1.18
  @{
*/
#define U_ICM_OFF   1 //!< Off
#define U_ICM_ON    2 //!< On
#define U_ICM_QUERY 3 //!< Query
/** @} */

/** \defgroup U_EMF_COLORADJUSTMENT_caIlluminantIndex_Qualifiers EMF Illuminant Enumeration
  For U_COLORADJUSTMENT caIlluminantIndex field
  Microsoft name: Illuminant Enumeration
  EMF manual 2.1.19
  @{
*/
#define U_ILLUMINANT_DEVICE_DEFAULT 0                //!< Device default
#define U_ILLUMINANT_A              1                //!< A
#define U_ILLUMINANT_B              2                //!< B
#define U_ILLUMINANT_C              3                //!< C
#define U_ILLUMINANT_D50            4                //!< D50
#define U_ILLUMINANT_D55            5                //!< D55
#define U_ILLUMINANT_D65            6                //!< D65
#define U_ILLUMINANT_D75            7                //!< D75
#define U_ILLUMINANT_F2             8                //!< F2
#define U_ILLUMINANT_MAX_INDEX      ILLUMINANT_F2    //!< Max index
#define U_ILLUMINANT_TUNGSTEN       ILLUMINANT_A     //!< Tungsten
#define U_ILLUMINANT_DAYLIGHT       ILLUMINANT_C     //!< Daylight
#define U_ILLUMINANT_FLUORESCENT    ILLUMINANT_F2    //!< Fluorescent
#define U_ILLUMINANT_NTSC           ILLUMINANT_C     //!< NTSC
/** @} */

/** \defgroup U_EMF_PANOSE_bLetterForm_Qualifiers EMF Letterform Enumeration
  For U_PANOSE bLetterForm field
  Microsoft name: Letterform Enumeration
  EMF manual 2.1.20
  @{
*/
#define U_PAN_LETT_NORMAL_COMPACT      2  //!< Normal compact
#define U_PAN_LETT_NORMAL_WEIGHTED     3  //!< Normal weighted
#define U_PAN_LETT_NORMAL_BOXED        4  //!< Normal boxed
#define U_PAN_LETT_NORMAL_FLATTENED    5  //!< Normal flattened
#define U_PAN_LETT_NORMAL_ROUNDED      6  //!< Normal rounded
#define U_PAN_LETT_NORMAL_OFF_CENTER   7  //!< Normal off center
#define U_PAN_LETT_NORMAL_SQUARE       8  //!< Normal square
#define U_PAN_LETT_OBLIQUE_COMPACT     9  //!< Oblique compact
#define U_PAN_LETT_OBLIQUE_WEIGHTED   10  //!< Oblique weighted
#define U_PAN_LETT_OBLIQUE_BOXED      11  //!< Oblique boxed
#define U_PAN_LETT_OBLIQUE_FLATTENED  12  //!< Oblique flattened
#define U_PAN_LETT_OBLIQUE_ROUNDED    13  //!< Oblique rounded
#define U_PAN_LETT_OBLIQUE_OFF_CENTER 14  //!< Oblique off center
#define U_PAN_LETT_OBLIQUE_SQUARE     15  //!< Oblique square
/** @} */

/** \defgroup U_EMF_EMRSETMAPMODE_iMode_Qualifiers EMF MapMode Enumeration
  For U_EMRSETMAPMODE iMode field
  Microsoft name: MapMode Enumeration
  EMF manual 2.1.21
  @{
*/
#define U_MM_TEXT           1                 //!< Text
#define U_MM_LOMETRIC       2                 //!< Low metric
#define U_MM_HIMETRIC       3                 //!< Hig hmetric
#define U_MM_LOENGLISH      4                 //!< Low English
#define U_MM_HIENGLISH      5                 //!< High English
#define U_MM_TWIPS          6                 //!< Twips
#define U_MM_ISOTROPIC      7                 //!< Isotropic
#define U_MM_ANISOTROPIC    8                 //!< Anisotropic
#define U_MM_MIN            U_MM_TEXT         //!< smallest enumeration
#define U_MM_MAX            U_MM_ANISOTROPIC  //!< largest enumeration
#define U_MM_MAX_FIXEDSCALE U_MM_TWIPS        //!< alternate definition
/** @} */

/** \defgroup U_EMF_MF_version EMF MetafileVersion Enumeration 
  For U_EMR_COMMENTS_METAFILE version field
  Microsoft name: MetafileVersion Enumeration 
  EMF manual 2.1.22
  @{
*/
#define U_ENHMETA_VERSION             0x00010000      //!< U_EMRHEADER nVersion field
/** @} */

/** \defgroup U_EMF_PANOSE_bMidline_Qualifiers EMF MidLine Enumeration
  For U_PANOSE bMidline field
  Microsoft name: MidLine Enumeration
  EMF manual 2.1.23
  @{
*/
#define U_PAN_MIDLINE_STANDARD_TRIMMED  2  //!< Midline standard trimmed
#define U_PAN_MIDLINE_STANDARD_POINTED  3  //!< Midline standard pointed
#define U_PAN_MIDLINE_STANDARD_SERIFED  4  //!< Midline standard serifed
#define U_PAN_MIDLINE_HIGH_TRIMMED      5  //!< Midline high trimmed
#define U_PAN_MIDLINE_HIGH_POINTED      6  //!< Midline high pointed
#define U_PAN_MIDLINE_HIGH_SERIFED      7  //!< Midline high serifed
#define U_PAN_MIDLINE_CONSTANT_TRIMMED  8  //!< Midline constant trimmed
#define U_PAN_MIDLINE_CONSTANT_POINTED  9  //!< Midline constant pointed
#define U_PAN_MIDLINE_CONSTANT_SERIFED 10  //!< Midline constant serifed
#define U_PAN_MIDLINE_LOW_TRIMMED      11  //!< Midline low trimmed
#define U_PAN_MIDLINE_LOW_POINTED      12  //!< Midline low pointed
#define U_PAN_MIDLINE_LOW_SERIFED      13  //!< Midline low serifed
/** @} */

/** \defgroup U_EMF_EMRMODIFYWORLDTRANSFORM_iMode_Qualifiers EMF ModifyWorldTransformMode Enumeration
  For U_EMRMODIFYWORLDTRANSFORM iMode
  Microsoft name: ModifyWorldTransformMode Enumeration
  EMF manual 2.1.24
  @{
*/
#define U_MWT_IDENTITY      1                   //!< Transform is identity.
#define U_MWT_LEFTMULTIPLY  2                   //!< Left multiply transform.
#define U_MWT_RIGHTMULTIPLY 3                   //!< Right multiply transform.
#define U_MWT_MIN           U_MWT_IDENTITY      //!< smallest enumeration.
#define U_MWT_MAX           U_MWT_RIGHTMULTIPLY //!< largest enumeration.
/** @} */

/** \defgroup U_EMF_LOGPEN_elpPenStyle_Qualifiers EMF PenStyle Enumeration
  For U_LOGPEN lopnStyle and U_EXTLOGPEN elpPenStyle fields
  Microsoft name: PenStyle Enumeration
  EMF manual 2.1.25
  @{
*/
#define U_PS_SOLID         0x00000000  //!< Solid line.
#define U_PS_DASH          0x00000001  //!< Dashed line. This only works when NO other U_PS is set.  Line width is minimum no matter what pen is set to.
#define U_PS_DOT           0x00000002  //!< Dotted line. This only works when NO other U_PS is set.  Line width is minimum no matter what pen is set to.
#define U_PS_DASHDOT       0x00000003  //!< Dash-Dot line. This only works when NO other U_PS is set.  Line width is minimum no matter what pen is set to.
#define U_PS_DASHDOTDOT    0x00000004  //!< Dash-Dot-Dot line. This only works when NO other U_PS is set.  Line width is minimum no matter what pen is set to.
#define U_PS_NULL          0x00000005  //!< Invisible line.
#define U_PS_INSIDEFRAME   0x00000006  //!< Draw line around drawing, then shrink drawing to fit within line taking its width into account.
#define U_PS_USERSTYLE     0x00000007  //!< User defined.
#define U_PS_ALTERNATE     0x00000008  //!< Every other pixel is drawn.
#define U_PS_STYLE_MASK    0x0000000f  //!< Mask to select just the preceding line type fields.

#define U_PS_ENDCAP_ROUND  0x00000000  //!< Round end cap.  Only with U_PS_GEOMETRIC
#define U_PS_ENDCAP_SQUARE 0x00000100  //!< Square end cap.  Only with U_PS_GEOMETRIC
#define U_PS_ENDCAP_FLAT   0x00000200  //!< Flat end cap.  Only with U_PS_GEOMETRIC
#define U_PS_ENDCAP_MASK   0x00000f00  //!< Mask to select just the preceding ENDCAP fields.

#define U_PS_JOIN_ROUND    0x00000000  //!< Rounded join.  Only with U_PS_GEOMETRIC
#define U_PS_JOIN_BEVEL    0x00001000  //!< Beveled join.  Only with U_PS_GEOMETRIC
#define U_PS_JOIN_MITER    0x00002000  //!< Mitered join.  Only with U_PS_GEOMETRIC
#define U_PS_JOIN_MASK     0x0000f000  //!< Mask to select just the preceding JOIN fields.

#define U_PS_COSMETIC      0x00000000  //!< width may only be 1 pixel.  (If set higher it is still drawn as 1).
#define U_PS_GEOMETRIC     0x00010000  //!< width may be >1 pixel, but style may only be U_PS_SOLID or U_PS_NULL.
#define U_PS_TYPE_MASK     0x000f0000  //!< Mask to select just the preceding TYPE fields. 
/** @} */
/** \defgroup U_EMF_EMRPOLY_iMode_Qualifiers EMF Point Enumeration
  For U_EMRPOLYDRAW and U_EMRPOLAYDRAW16 abTypes fields.
  Microsoft name: Point Enumeration
  EMF manual 2.1.26
 @{
*/
#define U_PT_CLOSEFIGURE          0x0001 //!< Close figure
#define U_PT_LINETO               0x0002 //!< Line to
#define U_PT_BEZIERTO             0x0004 //!< Bezier to
#define U_PT_MOVETO               0x0006 //!< Move to
/** @} */

/** \defgroup U_EMF_EMRSETPOLYFILLMODE_iMode_Qualifiers EMF PolygonFillMode Enumeration
  For U_EMRSETPOLYFILLMODE iMode field
  Microsoft name: PolygonFillMode Enumeration
  EMF manual 2.1.27
  @{
*/
#define U_ALTERNATE         1 //!< Alternate
#define U_WINDING           2 //!< Winding
#define U_POLYFILL_LAST     2 //!< Polyfill last
/** @} */

/** \defgroup U_EMF_PANOSE_bProportion_Qualifiers EMF Proportion Enumeration
  For U_PANOSE bProportion field
  Microsoft name: Proportion Enumeration
  EMF manual 2.1.28
  @{
*/
#define U_PAN_PROP_OLD_STYLE      2 //!< Old style
#define U_PAN_PROP_MODERN         3 //!< Modern
#define U_PAN_PROP_EVEN_WIDTH     4 //!< Even width
#define U_PAN_PROP_EXPANDED       5 //!< Expanded
#define U_PAN_PROP_CONDENSED      6 //!< Condensed
#define U_PAN_PROP_VERY_EXPANDED  7 //!< Very expanded
#define U_PAN_PROP_VERY_CONDENSED 8 //!< Very condensed
#define U_PAN_PROP_MONOSPACED     9 //!< Monospaced
/** @} */

/** \defgroup U_EMF_EMRSELECTCLIP_iMode_Qualifiers EMF RegionMode Enumeration
  For U_EMRSELECTCLIPPATH and U_EMREXTSELECTCLIPRGN iMode field
  Microsoft name: RegionMode Enumeration
  EMF manual 2.1.29
  @{
*/
#define U_RGN_NONE 0          //!< not part of EMF standard, may be used by others
#define U_RGN_AND  1          //!< Region becomes intersection of existing region and new region.
#define U_RGN_OR   2          //!< Region becomes union of existing region and new region.
#define U_RGN_XOR  3          //!< Region becomes XOR of existing and new regions.
#define U_RGN_DIFF 4          //!< Region becomes part of existing region not in new region.
#define U_RGN_COPY 5          //!< Region becomes new region.   
#define U_RGN_MIN  U_RGN_AND  //!< smallest enumeration.
#define U_RGN_MAX  U_RGN_COPY //!< largest enumeration.
/** @} */

/** \defgroup U_EMF_PANOSE_bSerifStyle_Qualifiers EMF SerifType Enumeration
  For U_PANOSE bSerifStyle field
  Microsoft name: SerifType Enumeration
  EMF manual 2.1.30
  @{
*/
#define U_PAN_SERIF_COVE                2 //!< Serif cove
#define U_PAN_SERIF_OBTUSE_COVE         3 //!< Serif obtuse cove
#define U_PAN_SERIF_SQUARE_COVE         4 //!< Serif square cove
#define U_PAN_SERIF_OBTUSE_SQUARE_COVE  5 //!< Serif obtuse square cove
#define U_PAN_SERIF_SQUARE              6 //!< Serif square
#define U_PAN_SERIF_THIN                7 //!< Serif thin
#define U_PAN_SERIF_BONE                8 //!< Serif bone
#define U_PAN_SERIF_EXAGGERATED         9 //!< Serif exaggerated
#define U_PAN_SERIF_TRIANGLE           10 //!< Serif triangle
#define U_PAN_SERIF_NORMAL_SANS        11 //!< Serif normal sans
#define U_PAN_SERIF_OBTUSE_SANS        12 //!< Serif obtuse sans
#define U_PAN_SERIF_PERP_SANS          13 //!< Serif perp sans
#define U_PAN_SERIF_FLARED             14 //!< Serif flared
#define U_PAN_SERIF_ROUNDED            15 //!< Serif rounded           
/** @} */

/** \defgroup U_EMF_EMRSELECTOBJECT_ihObject_Qualifiers EMF StockObject Enumeration
  For U_EMRSELECTOBJECT ihObject field.
  Microsoft name: StockObject Enumeration
  EMF manual 2.1.31
  @{
*/
#define U_STOCK_OBJECT          0x80000000 //!< Stock object
#define U_WHITE_BRUSH           0x80000000 //!< White brush
#define U_LTGRAY_BRUSH          0x80000001 //!< Ltgray brush
#define U_GRAY_BRUSH            0x80000002 //!< Gray brush
#define U_DKGRAY_BRUSH          0x80000003 //!< Dkgray brush
#define U_BLACK_BRUSH           0x80000004 //!< Black brush
#define U_NULL_BRUSH            0x80000005 //!< Null brush
#define U_HOLLOW_BRUSH          0x80000005 //!< Hollow brush
#define U_WHITE_PEN             0x80000006 //!< White pen
#define U_BLACK_PEN             0x80000007 //!< Black pen
#define U_NULL_PEN              0x80000008 //!< Null pen
#define U_OEM_FIXED_FONT        0x8000000A //!< Oem fixed font
#define U_ANSI_FIXED_FONT       0x8000000B //!< Ansi fixed font
#define U_ANSI_VAR_FONT         0x8000000C //!< Ansi var font
#define U_SYSTEM_FONT           0x8000000D //!< System font
#define U_DEVICE_DEFAULT_FONT   0x8000000E //!< Device default font
#define U_DEFAULT_PALETTE       0x8000000F //!< Default palette
#define U_SYSTEM_FIXED_FONT     0x80000010 //!< System fixed font
#define U_DEFAULT_GUI_FONT      0x80000011 //!< Default GUI font
#define U_STOCK_LAST            0x80000011 //!< Stock last
/** @} */

/** \defgroup U_EMF_EMRSETSTRETCHBLTMODE_iMode_Qualifiers EMF StretchMode Enumeration
  For EMF U_EMRSETSTRETCHBLTMODE iMode field
  Microsoft name: StretchMode Enumeration
  EMF manual 2.1.32 and footnote 52 on page 297
  @{
*/
#define U_BLACKONWHITE         1 //!< AND the destination and source pixels.
#define U_WHITEONBLACK         2 //!< OR the destination and source pixels.
#define U_COLORONCOLOR         3 //!< Replace the destination pixels with the source pixels.                                     
#define U_HALFTONE             4 //!< Replace a block of destination pixels with a half-tone representation of the source pixel. 
#define U_MAXSTRETCHBLTMODE    4 //!< largest enumeration.
#define U_STRETCH_ANDSCANS     1 //!< AND the destination and source pixels.
#define U_STRETCH_ORSCANS      2 //!< OR the destination and source pixels.
#define U_STRETCH_DELETESCANS  3 //!< Replace the destination pixels with the source pixels.
#define U_STRETCH_HALFTONE     4 //!< Replace a block of destination pixels with a half-tone representation of the source pixel.
/** @} */

/** \defgroup U_EMF_PANOSE_bStrokeVariation_Qualifiers EMF StrokeVariation Enumeration
  For U_PANOSE bStrokeVariation field
  Microsoft name: StrokeVariation Enumeration
  EMF manual 2.1.33
  @{
*/
#define U_PAN_STROKE_GRADUAL_DIAG 2 //!< Gradual diagonal.
#define U_PAN_STROKE_GRADUAL_TRAN 3 //!< Gradual transitional.
#define U_PAN_STROKE_GRADUAL_VERT 4 //!< Gradual vertical.
#define U_PAN_STROKE_GRADUAL_HORZ 5 //!< Gradual horizontal.
#define U_PAN_STROKE_RAPID_VERT   6 //!< Rapid vertical.
#define U_PAN_STROKE_RAPID_HORZ   7 //!< Rapid horizontal.
#define U_PAN_STROKE_INSTANT_VERT 8 //!< Instant vertical.
/** @} */

/** \defgroup U_EMF_PANOSE_bWeight_Qualifiers EMF Weight Enumeration
  For U_PANOSE bWeight field
  EMF manual 2.1.34
  @{
*/
#define U_PAN_WEIGHT_VERY_LIGHT  2 //!< Very light
#define U_PAN_WEIGHT_LIGHT       3 //!< Light
#define U_PAN_WEIGHT_THIN        4 //!< Thin
#define U_PAN_WEIGHT_BOOK        5 //!< Book
#define U_PAN_WEIGHT_MEDIUM      6 //!< Medium
#define U_PAN_WEIGHT_DEMI        7 //!< Demi
#define U_PAN_WEIGHT_BOLD        8 //!< Bold
#define U_PAN_WEIGHT_HEAVY       9 //!< Heavy
#define U_PAN_WEIGHT_BLACK      10 //!< Black
#define U_PAN_WEIGHT_NORD       11 //!< Nord
/** @} */

/** \defgroup U_EMF_PANOSE_bXHeight_Qualifiers EMF XHeight Enumeration
  For U_PANOSE bXHeight field
  EMF manual 2.1.35
  @{
*/
#define U_PAN_XHEIGHT_CONSTANT_SMALL    2 //!< Constant small
#define U_PAN_XHEIGHT_CONSTANT_STANDARD 3 //!< Constant standard
#define U_PAN_XHEIGHT_CONSTANT_LARGE    4 //!< Constant large
#define U_PAN_XHEIGHT_DUCKING_SMALL     5 //!< Ducking small
#define U_PAN_XHEIGHT_DUCKING_STANDARD  6 //!< Ducking standard
#define U_PAN_XHEIGHT_DUCKING_LARGE     7 //!< Ducking large
/** @} */

/** \defgroup U_EMF_LOGFONT_lfWeight_Qualifiers EMF LF_Weight Enumeration
  For U_LOGFONT lfWeight field
  EMF manual 2.2.13, footnote 61 (on page 297)
  @{
*/
#define U_FW_DONTCARE     0 //!< Don't care
#define U_FW_THIN       100 //!< Thin
#define U_FW_EXTRALIGHT 200 //!< Extra light
#define U_FW_ULTRALIGHT 200 //!< Ultra light
#define U_FW_LIGHT      300 //!< Light
#define U_FW_NORMAL     400 //!< Normal
#define U_FW_REGULAR    400 //!< Regular
#define U_FW_MEDIUM     500 //!< Medium
#define U_FW_SEMIBOLD   600 //!< Semibold
#define U_FW_DEMIBOLD   600 //!< Demibold
#define U_FW_BOLD       700 //!< Bold
#define U_FW_EXTRABOLD  800 //!< Extrabold
#define U_FW_ULTRABOLD  800 //!< Ultrabold
#define U_FW_HEAVY      900 //!< Heavy
#define U_FW_BLACK      900 //!< Black
/** @} */

/** \defgroup U_EMF_LOGFONT_lfItalic_Qualifiers EMF LF_Italic Enumeration
  For U_LOGFONT lfItalic field
  Microsoft name: LF_Italic Enumeration
  EMF manual 2.2.13
  @{
*/
#define U_FW_NOITALIC     0 //!< Do not use italics.
#define U_FW_ITALIC       1 //!< Use italics.
/** @} */

/** \defgroup U_EMF_LOGFONT_lfunderline_Qualifiers EMF LF_Underline Enumeration
  For U_LOGFONT lfunderline field
  Microsoft name: LF_Underline Enumeration
  EMF manual 2.2.13
  @{
*/
#define U_FW_NOUNDERLINE  0 //!< Do not use underline.
#define U_FW_UNDERLINE    1 //!< Use underline.
/** @} */

/** \defgroup U_EMF_LOGFONT_lfStrikeOut_Qualifiers EMF LF_StrikeOut Enumeration
  For U_LOGFONT lfStrikeOut field
  EMF manual 2.2.13
  @{
*/
#define U_FW_NOSTRIKEOUT  0 //!< Do not use strikeout.
#define U_FW_STRIKEOUT    1 //!< Use strikeout.
/** @} */

/** \defgroup U_EMF_LOGFONT_lfCharSet_Qualifiers EMF LF_CharSet Enumeration
  For U_LOGFONT lfCharSet field
  EMF manual 2.2.13 & WMF manual 2.1.15
  @{
*/
#define U_ANSI_CHARSET          (uint8_t)0         //!< CP1252, ansi-0, iso8859-{1,15} 
#define U_DEFAULT_CHARSET       (uint8_t)1         //!< Default character set.
#define U_SYMBOL_CHARSET        (uint8_t)2         //!< Symbol character set.
#define U_SHIFTJIS_CHARSET      (uint8_t)128       //!< CP932 
#define U_HANGEUL_CHARSET       (uint8_t)129       //!< CP949, ksc5601.1987-0 
#define U_HANGUL_CHARSET        U_HANGEUL_CHARSET  //!< CP949, ksc5601.1987-0 
#define U_GB2312_CHARSET        (uint8_t)134       //!< CP936, gb2312.1980-0 
#define U_CHINESEBIG5_CHARSET   (uint8_t)136       //!< CP950, big5.et-0 
#define U_GREEK_CHARSET         (uint8_t)161       //!< CP1253 
#define U_TURKISH_CHARSET       (uint8_t)162       //!< CP1254, -iso8859-9 
#define U_HEBREW_CHARSET        (uint8_t)177       //!< CP1255, -iso8859-8 
#define U_ARABIC_CHARSET        (uint8_t)178       //!< CP1256, -iso8859-6 
#define U_BALTIC_CHARSET        (uint8_t)186       //!< CP1257, -iso8859-13 
#define U_RUSSIAN_CHARSET       (uint8_t)204       //!< CP1251, -iso8859-5 
#define U_EE_CHARSET            (uint8_t)238       //!< CP1250, -iso8859-2 
#define U_EASTEUROPE_CHARSET    U_EE_CHARSET       //!< CP1250, -iso8859-2
#define U_THAI_CHARSET          (uint8_t)222       //!< CP874, iso8859-11, tis620 
#define U_JOHAB_CHARSET         (uint8_t)130       //!< korean (johab) CP1361 
#define U_MAC_CHARSET           (uint8_t)77        //!< Macintosh character set.
#define U_OEM_CHARSET           (uint8_t)255       //!< OEM character set.
#define U_VISCII_CHARSET        (uint8_t)240       //!< viscii1.1-1 
#define U_TCVN_CHARSET          (uint8_t)241       //!< tcvn-0 
#define U_KOI8_CHARSET          (uint8_t)242       //!< koi8-{r,u,ru} 
#define U_ISO3_CHARSET          (uint8_t)243       //!< iso8859-3 
#define U_ISO4_CHARSET          (uint8_t)244       //!< iso8859-4 
#define U_ISO10_CHARSET         (uint8_t)245       //!< iso8859-10 
#define U_CELTIC_CHARSET        (uint8_t)246       //!< iso8859-14 
/** @} */

/** \defgroup U_EMF_LOGFONT_lfOutPrecision_Qualifiers EMF LF_OutPrecision Enumeration
  For U_LOGFONT lfOutPrecision field
  EMF manual 2.2.13 & WMF manual 2.1.1.21
  @{
*/
#define U_OUT_DEFAULT_PRECIS   0  //!< Default precision
#define U_OUT_STRING_PRECIS    1  //!< String precision
#define U_OUT_CHARACTER_PRECIS 2  //!< Character precision
#define U_OUT_STROKE_PRECIS    3  //!< Stroke precision
#define U_OUT_TT_PRECIS        4  //!< Tt precision
#define U_OUT_DEVICE_PRECIS    5  //!< Device precision
#define U_OUT_RASTER_PRECIS    6  //!< Raster precision
#define U_OUT_TT_ONLY_PRECIS   7  //!< Tt_only precision
#define U_OUT_OUTLINE_PRECIS   8  //!< Outline precision
/** @} */

/** \defgroup U_EMF_LOGFONT_lfClipPrecision_Qualifiers EMF LF_ClipPrecision Enumeration
  For U_LOGFONT lfClipPrecision field
  EMF manual 2.2.13 & WMF manual 2.1.2.1
  @{
*/
#define U_CLIP_DEFAULT_PRECIS   0x00 //!< Use default clipping precision.
#define U_CLIP_CHARACTER_PRECIS 0x01 //!< Use character clipping precision
#define U_CLIP_STROKE_PRECIS    0x02 //!< (Source documentation is vague about what this means.)
#define U_CLIP_MASK             0x0F //!< MASK for bits in preceding.
#define U_CLIP_LH_ANGLES        0x10 //!< Set: font rotation by coordinate system, Clear: device fonts (only) rotate counterclockwise.
#define U_CLIP_TT_ALWAYS        0x20 //!< Reserved.
#define U_CLIP_EMBEDDED         0x80 //!< Font embedding is required.  (Method for doing so is not documented in EMF or WMF.)
/** @} */

/** \defgroup U_EMF_LOGFONT_lfQuality_Qualifiers EMF LF_Quality Enumeration
  For For U_LOGFONT lfQuality field
  EMF manual 2.2.13 & WMF manual 2.1.1.10
  @{
*/
#define U_DEFAULT_QUALITY        0 //!< Default quality
#define U_DRAFT_QUALITY          1 //!< Draft quality
#define U_PROOF_QUALITY          2 //!< Proof quality
#define U_NONANTIALIASED_QUALITY 3 //!< Nonantialiased quality
#define U_ANTIALIASED_QUALITY    4 //!< Antialiased quality
/** @} */

/** \defgroup U_EMF_LOGFONT_lfPitchAndFamily_Qualifiers EMF LF_PitchAndFamily Enumeration
  For U_LOGFONT lfPitchAndFamily field
  EMF manual 2.2.13 & WMF manual 2.2.2.14
  @{
*/
#define U_DEFAULT_PITCH  0x00 //!< Default pitch
#define U_FIXED_PITCH    0x01 //!< Fixed pitch
#define U_VARIABLE_PITCH 0x02 //!< Variable pitch
#define U_MONO_FONT      0x08 //!< Mono font
#define U_FF_DONTCARE    0x00 //!< Font family don't care
#define U_FF_ROMAN       0x10 //!< Font family Roman
#define U_FF_SWISS       0x20 //!< Font family Swiss
#define U_FF_MODERN      0x30 //!< Font family Modern
#define U_FF_SCRIPT      0x40 //!< Font family Script
#define U_FF_DECORATIVE  0x50 //!< Font family Decorative
/** @} */

/** \defgroup U_EMF_LOGBRUSH_lbStyle_Qualifiers EMF LB_Style Enumeration
  For U_LOGBRUSH lbStyle field
  EMF manual 2.2.20
  @{
*/
#define U_BS_SOLID         0 //!< Solid brush.
#define U_BS_NULL          1 //!< Null brush.
#define U_BS_HOLLOW        1 //!< Hollow brush.
#define U_BS_HATCHED       2 //!< Hatched brush.
#define U_BS_PATTERN       3 //!< Pattern brush.
#define U_BS_INDEXED       4 //!< Indexed brush.
#define U_BS_DIBPATTERN    5 //!< Dibpattern brush.
#define U_BS_DIBPATTERNPT  6 //!< Dibpatternpt brush.
#define U_BS_PATTERN8X8    7 //!< Pattern 8x8 brush.
#define U_BS_DIBPATTERN8X8 8 //!< Dibpattern 8x8 brush.
#define U_BS_MONOPATTERN   9 //!< Monopattern brush.
/** @} */

/** \defgroup U_EMF_PANOSE_index EMF PanoseIndex Enumeration
  Fositions of each field in U_PANOSE structure.
  Microsoft name: (none)
  EMF manual 2.2.21
  @{
*/
#define U_PAN_FAMILYTYPE_INDEX       0 //!< Familytype index
#define U_PAN_SERIFSTYLE_INDEX       1 //!< Serifstyle index
#define U_PAN_WEIGHT_INDEX           2 //!< Weight index
#define U_PAN_PROPORTION_INDEX       3 //!< Proportion index
#define U_PAN_CONTRAST_INDEX         4 //!< Contrast index
#define U_PAN_STROKEVARIATION_INDEX  5 //!< Strokevariation index
#define U_PAN_ARMSTYLE_INDEX         6 //!< Armstyle index
#define U_PAN_LETTERFORM_INDEX       7 //!< Letterform index
#define U_PAN_MIDLINE_INDEX          8 //!< Midline index
#define U_PAN_XHEIGHT_INDEX          9 //!< Xheight index
#define U_PAN_COUNT                 10 //!< Count
/** @} */


/** \defgroup U_EMF_PIXELFORMATDESCRIPTOR_iLayerType_Qualifiers EMF PFD_iLayerType Enumeration
  For U_PIXELFORMATDESCRIPTOR iLayerType field
  Microsoft name: (none)
  EMF manual 2.2.22
  @{
*/
#define U_PFD_MAIN_PLANE       0     //!< Main plane
#define U_PFD_OVERLAY_PLANE    1     //!< Overlay plane
#define U_PFD_UNDERLAY_PLANE   (-1)  //!< Underlay plane
/** @} */

/** \defgroup U_EMF_PIXELFORMATDESCRIPTOR_iPixelType_Qualifiers EMF PFD_iPixelType Enumeration
  For U_PIXELFORMATDESCRIPTOR iPixelType field
  Microsoft name: (none)
  EMF manual 2.2.22
  @{
*/
#define U_PFD_TYPE_RGBA        0 //!< Pixel contains an RGBA value.      
#define U_PFD_TYPE_COLORINDEX  1 //!< Pixel contains an index into the color table.
/** @} */

/** \defgroup U_EMF_PIXELFORMATDESCRIPTOR_dwFlags_Qualifiers EMF PFD_dwFlags Enumeration
  For U_PIXELFORMATDESCRIPTOR dwFlags field 
  EMF manual 2.2.22
  @{
*/
#define U_PFD_DOUBLEBUFFER          0x00000001 //!< Doublebuffer
#define U_PFD_STEREO                0x00000002 //!< Stereo
#define U_PFD_DRAW_TO_WINDOW        0x00000004 //!< Draw to window
#define U_PFD_DRAW_TO_BITMAP        0x00000008 //!< Draw to bitmap
#define U_PFD_SUPPORT_GDI           0x00000010 //!< Support gdi
#define U_PFD_SUPPORT_OPENGL        0x00000020 //!< Support opengl
#define U_PFD_GENERIC_FORMAT        0x00000040 //!< Generic format
#define U_PFD_NEED_PALETTE          0x00000080 //!< Need palette
#define U_PFD_NEED_SYSTEM_PALETTE   0x00000100 //!< Need system palette
#define U_PFD_SWAP_EXCHANGE         0x00000200 //!< Swap exchange
#define U_PFD_SWAP_COPY             0x00000400 //!< Swap copy
#define U_PFD_SWAP_LAYER_BUFFERS    0x00000800 //!< Swap layer buffers
#define U_PFD_GENERIC_ACCELERATED   0x00001000 //!< Generic accelerated
/** @} */

/** \defgroup U_EMF_EMRCOMMENT_TYPES EMF Comment record types
  For U_EMRCOMMENT_* cIdent fields
  EMF manual 2.3.3
  @{
*/
#define U_EMR_COMMENT_PUBLIC            0x43494447 //!< Public comment.
#define U_EMR_COMMENT_SPOOL             0x00000000 //!< Spool comment.
#define U_EMR_COMMENT_EMFPLUSRECORD     0x2B464D45 //!< EMF+ record comment.
/** @} */

/** \defgroup U_EMF_EMRSETLAYOUT_iMode_Qualifiers EMF Mirroring Enumeration
  For U_EMRSETLAYOUT iMode field
  EMF manual 2.3.11.17
  @{
*/
#define U_LAYOUT_LTR                         0x00000000 //!< Left to right lsyout.
#define U_LAYOUT_RTL                         0x00000001 //!< Right to left layout.
#define U_LAYOUT_BITMAPORIENTATIONPRESERVED  0x00000008 //!< Do not flip bitmaps if layout is right to left.
/** @} */


/** \defgroup U_EMF_BLEND_Op_Qualifiers EMF Blend Enumeration
  For U_BLEND Op field
  @{
*/
#define U_AC_SRC_GLOBAL 0 //!< Global
#define U_AC_SRC_CONST  0 //!< Const
#define U_AC_SRC_ALPHA  1 //!< Alpha
/** @} */


//  ***************************************************************************
//  Macros

/** \defgroup U_EMF_Common_macros EMF Common Macros
  @{
*/
//  Note, many of these were originally defined using C99 (type){val,val,val} format, but that turned out to
//  have incompatibilities with C++, so use functions as the basis to avoid this.

//  Set/Get for U_RGBQUAD Colors, which are stored in byte order: {B,G,R,A}.  
//  These are used in EMF structures and the byte order must be the same in memory or on disk.
#define U_BGR(r,g,b)         rgbquad_set(r, g, b, 0)        //!<  Set any BGR  color with an {r,g,b} triplet
#define U_BGRA(r,g,b,a)      rgbquad_set(r, g, b, a)        //!<  Set any BGRA color with an {r,g,b,a} quad
#define U_WHITE              U_BGR(255,255,255)             //!<  Set BGR white.
#define U_BLACK              U_BGR(0,0,0)                   //!<  Set BGR black.
#define U_BGRAGetR(rgb)      (rgb.Red     )                 //!<  Color BGR Red.
#define U_BGRAGetG(rgb)      (rgb.Green   )                 //!<  Color BGR Green.
#define U_BGRAGetB(rgb)      (rgb.Blue    )                 //!<  Color BGR Blue.
#define U_BGRAGetA(rgb)      (rgb.Reserved)                 //!<  Color BGRA A/reserved


//  Set/Get for U_COLORREF Color, which are stored in byte order:: {R,G,B,A}.
//  These are used in EMF structures and the byte order must be the same in memory or on disk.
//  These MAY be used in PNG and other libraries if these enforce byte order in memory, otherwise
//  U_swap4 may need to also be employed.
//

#define colorref_set         colorref3_set                  //!<  Most frequent usage is 3 colors, so set the unqualified one to that
#define U_RGB(r,g,b)         colorref3_set(r, g, b)         //!<  Set any RGB  color with an {r,g,b} triplet
#define U_RGBA(r,g,b,a)      colorref4_set(r, g, b,  a)     //!<  Set any RGBA color with an {r,g,b,a} quad
#define U_RGBAGetR(rgb)      (rgb.Red     )                 //!<  Color RGB Red.
#define U_RGBAGetG(rgb)      (rgb.Green   )                 //!<  Color RGB Green
#define U_RGBAGetB(rgb)      (rgb.Blue    )                 //!<  Color RGB Blue
#define U_RGBAGetA(rgb)      (rgb.Reserved)                 //!<  Color RGBA A/reserved

//   color type conversions
#define U_RGB2BGR(rgb)        U_BGR(U_RGBAGetR(rgb),U_RGBAGetG(rgb),U_RGBAGetB(rgb))                    //!<  Set any BGR color from an RGB color
#define U_BGR2RGB(rgb)        U_RGB(U_BGRAGetR(rgb),U_BGRAGetG(rgb),U_BGRAGetB(rgb))                    //!<  Set any RGB color from an BGR color
#define U_RGBA2BGRA(rgb)      U_BGRA(U_RGBAGetR(rgb),U_RGBAGetG(rgb),U_RGBAGetB(rgb),U_RGBAGetA(rgb)}   //!<  Set any BGRA color from an RGBA color
#define U_BGRA2RGBA(rgb)      U_RGBA(U_BGRAGetR(rgb),U_BGRAGetG(rgb),U_BGRAGetB(rgb),U_BGRAGetA(rgb)}   //!<  Set any RGBA color from an BGRA color

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

/** Any generic pair of floats.

  Microsoft name: (none)
*/
typedef struct {
    float x;                                //!< X value
    float y;                                //!< Y value
} U_PAIRF,
  *PU_PAIRF;                                //!< Any generic pair of floats. Microsoft name: (none)


/* ************************************************************
    EMF structures OTHER than those corresponding to complete U_EMR_* records
   ************************************************************ */

/**  
  \brief For U_POINT28_4 x and y fields.

  EMF manual 2.2.1, Microsoft name: BitFIX28_4 Object.
*/
typedef struct {
    signed              IntValue :28;       //!< Signed integral bit field
    unsigned            FracValue :4;       //!< Unsigned integral bit field
} U_BITFIX28_4,
  *PU_BITFIX28_4;                           //!< EMF manual 2.2.1

/** 
  \brief For U_EMRSETOLORADJUSTMENT ColorAdjustment field

  EMF manual 2.2.2, Microsoft name: ColorAdjustment Object

  Note, range constants are: RGB_GAMMA_[MIN|MAX],REFERENCE_[WHITE|BLACK]_[MIN|MAX],COLOR_ADJ_[MIN|MAX]
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
} U_COLORADJUSTMENT,
  *PU_COLORADJUSTMENT;                      //!< EMF manual 2.2.2

/** EMF manual 2.2.3
  \brief For ? (not implemented yet)

  Microsoft name: DesignVector Object
*/
typedef struct {
    uint32_t            Signature;          //!< Must be 0x08007664 (AKA: DV_SGNTR)
    U_NUM_FNTAXES       NumAxes;            //!< Number of elements in Values, 0-16
    U_FNTAXES           Values[1];          //!< Optional. Array of font axes for opentype font
} U_DESIGNVECTOR,
  *PU_DESIGNVECTOR;                         //!< EMF manual 2.2.3

/** 
  \brief For U_EMR_COMMENT_MULTIFORMATS record, where an array of these is used

  EMF manual 2.2.4, Microsoft name: EmrFormat Object
*/
typedef struct {
    uint32_t            signature;          //!< FormatSignature Enumeration
    uint32_t            nVersion;           //!< Must be 1 if signature is EPS, else ignored
    U_CBDATA            cbData;             //!< Data size in bytes
    U_OFFDATA           offData;            //!< Offset in bytes to the Data from the start of the RECORD
} U_EMRFORMAT,
  *PU_EMRFORMAT;                            //!< EMF manual 2.2.4

/** 

  \brief For U_EMR[POLY]EXTTEXTOUT[A|W] emrtext field
  
  EMF manual 2.2.5, Microsoft name: EmrText Object

  Differs from implementation in Mingw and Wine in that the core struct has a fixed size.  
  Optional and movable components must be handled with offsets.
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
} U_EMRTEXT,
  *PU_EMRTEXT;                              //!< EMF manual 2.2.5

/** 
  \brief For U_EPS_DATA Points field

  EMF manual 2.2.23, Microsoft name: Point28_4 Object, out of order, needed for 2.2.6
*/
typedef struct {
    U_BITFIX28_4        x;                  //!< X coordinate
    U_BITFIX28_4        y;                  //!< Y coordinate
} U_POINT28_4,
  *PU_POINT28_4;                            //!< EMF manual 2.2.23

/** 
  \brief For embedding EPS in EMF via U_EMRFORMAT offData array in U_EMR_COMMENT_MULTIFORMATS
  
  EMF manual 2.2.6, Microsoft name: EpsData Object
*/
typedef struct {
    uint32_t            sizeData;           //!< Size in bytes of this object
    uint32_t            version;            //!< Must be 1
    U_POINT28_4         Points[3];          //!< Defines parallelogram, UL, UR, LL corners, LR is derived.
    U_RECTL             PostScriptData;     //!< Record may include optional clipping/opaque rectangle
} U_EPS_DATA,
  *PU_EPS_DATA;                             //!< EMF manual 2.2.6

/**  
  \brief For GRADIENT_[TRIANGLE|U_RECT]

  EMF manual 2.2.26, Microsoft name: TriVertex Object, out of order, needed for 2.2.7
*/
typedef struct {
    int32_t             x;                  //!< X coord
    int32_t             y;                  //!< Y coord
    uint16_t            Red;                //!< Red   component
    uint16_t            Green;              //!< Green component
    uint16_t            Blue;               //!< Bule  component
    uint16_t            Alpha;              //!< Alpha Transparency
} U_TRIVERTEX,
  *PU_TRIVERTEX;                            //!< EMF manual 2.2.26

/**
  \brief For U_EMRGRADIENTFILL GradObj field

  EMF manual 2.2.7, Microsoft name: GradientRectangle Object
*/
typedef struct {
    uint32_t            UpperLeft;          //!< Index of UL corner in an array of U_TRIVERTEX objects
    uint32_t            LowerRight;         //!< Index of LR corner in an array of U_TRIVERTEX objects
} U_GRADIENT4,
  *PU_GRADIENT4;                            //!< EMF manual 2.2.7

/**
  \brief For U_EMRGRADIENTFILL GradObj field

  EMF manual 2.2.8, Microsoft name: GradientTriangle Object

  Gradient object notes.  The next two structures are used to define the shape with reference to an existing array
  of points stored in an array of TriVertex objects in the U_EMRGRADIENTFILL record.  The tricky part
  is that these two structures are different sizes.  In some implementations (MingW) the array is cast to uint32_t
  and basically the cast is then ignored.  For libUEMF we leave this out of the structure entirely and get to it with offsets.
*/
typedef struct {
    uint32_t            Vertex1;            //!< Index of Vertex1 in an array of U_TRIVERTEX objects
    uint32_t            Vertex2;            //!< Index of Vertex2 in an array of U_TRIVERTEX objects
    uint32_t            Vertex3;            //!< Index of Vertex3 in an array of U_TRIVERTEX objects
} U_GRADIENT3,
  *PU_GRADIENT3;                            //!< EMF manual 2.2.8

//Microsoft name: Header object, EMF manual 2.2.9  defined below with record structs
//Microsoft name: HeaderExtension1 object, EMF manual 2.2.10 defined below with record structs
//Microsoft name: HeaderExtension1 object, EMF manual 2.2.11 defined below with record structs

/**
  \brief For U_EMRCREATEBRUSHINDIRECT lb field

  EMF manual 2.2.12, Microsoft name: LogBrushEx Object
*/
typedef struct {                            //!< In MS documentation this is LogBrushEx Object
    uint32_t            lbStyle;            //!< LB_Style Enumeration
    U_COLORREF          lbColor;            //!< Brush color
    uint32_t            lbHatch;            //!< HatchStyle Enumeration
} U_LOGBRUSH,
  U_PATTERN,                                //!< EMF manual 2.2.12
  *PU_LOGBRUSH,                             //!< EMF manual 2.2.12
  *PU_PATTERN;                              //!< EMF manual 2.2.12

/**
  \brief For U_LOGFONT_PANOSE elfLogFont field

  EMF manual 2.2.13, Microsoft name: LogFont Object
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
} U_LOGFONT,
  *PU_LOGFONT;                              //!< EMF manual 2.2.13

// Microsoft name: LogFontEx Object (not implemented)   EMF manual 2.2.14
// Microsoft name: LogFontExDv Object (not implemented) EMF manual 2.2.15

/**
  \brief For U_LOGFONT_PANOSE elfPanose field

  EMF manual 2.2.21, Microsoft name: Panose Object
*/
//   out of order, needed for 2.2.16
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
} U_PANOSE,
  *PU_PANOSE;                               //!< EMF manual 2.2.21

#define    U_PAN_ALL0  (U_PANOSE){0,0,0,0,0,0,0,0,0,0}  //!<  all U_PAN_ANY, have not seen this in an EMF file
#define    U_PAN_ALL1  (U_PANOSE){1,1,1,1,1,1,1,1,1,1}  //!<  all U_PAN_NO_FIT, this is what createfont() would have made

/**
  \brief For U_EMREXTCREATEFONTINDIRECTW elfw field

  EMF manual 2.2.16, Microsoft name: LogFontPanose Object
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
} U_LOGFONT_PANOSE,
  *PU_LOGFONT_PANOSE;                                   //!< EMF manual 2.2.16

/**
  \brief  For U_LOGPALETTE palPalEntry field(s)

  EMF manual 2.2.18, Microsoft name: LogPaletteEntry Object
*/
//  out of order, needed for 2.2.17
typedef struct {
    uint8_t             peReserved;         //!< Ignore
    uint8_t             peRed;              //!< Palette entry Red Intensity
    uint8_t             peGreen;            //!< Palette entry Green Intensity
    uint8_t             peBlue;             //!< Palette entry Blue Intensity
} U_LOGPLTNTRY,
  *PU_LOGPLTNTRY;                           //!< EMF manual 2.2.18

/**
  \brief  For U_EMRCREATEPALETTE lgpl field

  EMF manual 2.2.17, Microsoft name: LogPalette Object
  
*/
typedef struct { 
    uint16_t            palVersion;         //!< Must be  0x0300 (AKA: U_LP_VERSION)
    uint16_t            palNumEntries;      //!< Number of U_LOGPLTNTRY objects
    U_LOGPLTNTRY        palPalEntry[1];     //!< PC_Entry Enumeration
} U_LOGPALETTE,
  *PU_LOGPALETTE;                           //!< EMF manual 2.2.17

// Microsoft name: LogPaletteEntry Object, EMF manual 2.2.18, defined above, before 2.2.17

/**
  \brief For U_EMRCREATEPEN lopn field

  EMF manual 2.2.19, Microsoft name: LogPen Object
*/
typedef struct {
    uint32_t            lopnStyle;          //!< PenStyle Enumeration
    U_POINT             lopnWidth;          //!< Width of pen set by X, Y is ignored
    U_COLORREF          lopnColor;          //!< Pen color value
} U_LOGPEN,
  *PU_LOGPEN;                               //!< EMF manual 2.2.19

// Microsoft name: LogPenEx Object (not implemented)  EMF manual 2.2.20
// Microsoft name: Panose Object, EMF manual 2.2.21, defined above, before 2.2.16

/**
  \brief  For U_EMRPIXELFORMAT pfd field

  EMF manual 2.2.22, Microsoft name: PixelFormatDescriptor Object
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
} U_PIXELFORMATDESCRIPTOR,
  *PU_PIXELFORMATDESCRIPTOR;                //!< EMF manual 2.2.22

// Microsoft name: Point28_4 Object.  EMF manual 2.2.23, defined above, before 2.2.6

/**
  \brief  For U_RGNDATA rdb field

  EMF manual 2.2.25, Microsoft name: RegionDataHeader Object (RGNDATAHEADER)
*/
// out of order, needed for 2.2.24
typedef struct {
    uint32_t            dwSize;             //!< Size in bytes, must be 0x20 (AKA: U_RDH_OBJSIZE)
    uint32_t            iType;              //!< Must be 1 (AKA: U_RDH_RECTANGLES)
    U_NUM_RECTL         nCount;             //!< Number of rectangles in region
    uint32_t            nRgnSize;           //!< Size in bytes of rectangle buffer
    U_RECTL             rclBounds;          //!< Region bounds
} U_RGNDATAHEADER,
  *PU_RGNDATAHEADER;                        //!< EMF manual 2.2.25

/**
  \brief For U_EMRFILLRGN RgnData field(s)

  EMF manual 2.2.24, Microsoft name: RegionData Object
  
*/
typedef struct {
    U_RGNDATAHEADER     rdh;                //!< Data description
    U_RECTL             Buffer[1];          //!< Array of U_RECTL elements
} U_RGNDATA,
  *PU_RGNDATA;                              //!< EMF manual 2.2.24

// Microsoft name: RegionDataHeader Object.  EMF manual 2.2.25, defined above, before 2.2.24
// Microsoft name: TriVertex Object.  EMF manual 2.2.26, defined above, before 2.2.7
// Microsoft name: UniversalFontId Object.  EMF manual 2.2.27 (not implemented)

/**
  \brief  For U_EMR[FILLRGN|STRETCHBLT|MASKBLT|PLGBLT] xformSrc field

  EMF manual 2.2.28, Microsoft name: Xform Object
*/
typedef struct {
    U_FLOAT             eM11;               //!< Matrix element M11
    U_FLOAT             eM12;               //!< Matrix element M12
    U_FLOAT             eM21;               //!< Matrix element M21
    U_FLOAT             eM22;               //!< Matrix element M22
    U_FLOAT             eDx;                //!< X offset in logical units
    U_FLOAT             eDy;                //!< Y offset in logical units
} U_XFORM ,
  *PU_XFORM;                                //!< EMF manual 2.2.28
 
/**
  \brief For U_EMREXTCREATEPEN lopn field

  EMF manual 2.2.20, Microsoft name: LogPenEx Object
*/
typedef struct {
    uint32_t            elpPenStyle;        //!< PenStyle Enumeration
    uint32_t            elpWidth;           //!< Width in logical units (elpPenStyle & U_PS_GEOMETRIC) or 1 (pixel)
    uint32_t            elpBrushStyle;      //!< LB_Style Enumeration
    U_COLORREF          elpColor;           //!< Pen color
    uint32_t            elpHatch;           //!< HatchStyle Enumeration
    U_NUM_STYLEENTRY    elpNumEntries;      //!< Count of StyleEntry array
    U_STYLEENTRY        elpStyleEntry[1];   //!< Array of StyleEntry (For user specified dot/dash patterns)
} U_EXTLOGPEN,
  *PU_EXTLOGPEN;                            //!< EMF manual 2.2.20

/**
  \brief For U_EMR_* OffBmi* fields

  WMF Manual 2.2.2.9, Microsoft name: (none).

  Description of a Bitmap which in some cases is a Device Independent Bitmap (DIB)
*/
typedef struct {
    U_BITMAPINFOHEADER  bmiHeader;          //!< Geometry and pixel properties
    U_RGBQUAD           bmiColors[1];       //!< Color table.  24 bit images do not use color table values.
} U_BITMAPINFO,
  *PU_BITMAPINFO;                           //!< WMF Manual 2.2.2.9

/**
  \brief U_EMRALPHABLEND Blend field
  
  EMF Manual 2.3.1.1, Microsoft name: BLENDFUNCTION field of EMR_ALPHABLEND record.
*/
typedef struct {
  uint8_t Operation;  //!< Must be 0
  uint8_t Flags;      //!< Must be 0
  uint8_t Global;     //!< Alpha for whole thing if Op is U_AC_SRC_GLOBAL (AKA U_AC_SRC_GLOBAL)
  uint8_t Op;         //!< Blend Enumeration
} U_BLEND,
  *PU_BLEND;          //!< EMF Manual 2.3.1.1
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
} U_ENHMETARECORD,
  *PU_ENHMETARECORD;                        //!< General form of an EMF record.

/** First two fields of all EMF records,
    First two fields of all EMF+ records (1 or more within an EMF comment)
    For accessing iType and nSize files in all U_EMR* records
    Microsoft name: EMR Object
*/
typedef struct {
    uint32_t            iType;              //!< Type of EMR record
    uint32_t            nSize;              //!< Size of entire record in bytes (multiple of 4).
} U_EMR,
  *PU_EMR;                                  //!< First two fields of all EMF records,

/** Generic EMR record with two 32 bit values.
    Microsoft name: (none)
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_PAIR              pair;               //!< pair of 32 bit values
} U_EMRGENERICPAIR,   
  *PU_EMRGENERICPAIR;                       //!< Generic EMR record with two 32 bit values. Microsoft name: (none)




// ***********************************************************************************
// The following have U_EMR_# records.  They are ordered by their record index, not by EMF manual position.

/* Index  1 */
/** 
  \brief The first U_ENHMETARECORD record in the metafile.

  EMF manual 2.2.9, Microsoft name: Header object, HeaderExtension1 object, HeaderExtension2 object
  
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
    uint32_t            offDescription;     //!< Offset in bytes to optional UTF-16BE string Description field 
    uint32_t            nPalEntries;        //!< Number of Palette entries (in U_EMR_EOF record). 
    U_SIZEL             szlDevice;          //!< Reference device size in pixels  
    U_SIZEL             szlMillimeters;     //!< Reference device size in 0.01 mm
    /** Fields for winver >= win95 */
    U_CBPXLFMT          cbPixelFormat;      //!< Size in bytes of PixelFormatDescriptor, 0 if no PFD
    U_OFFPXLFMT         offPixelFormat;     //!< Offset in bytes to optional PixelFormatDescriptor from the start of the RECORD, 0 if no PFD
    uint32_t            bOpenGL;            //!< nonZero if OpenGL commands are included
    /** Fields for winver >= win98 */ 
    U_SIZEL             szlMicrometers;     //!< Size of the display device in micrometer
} U_EMRHEADER,
  *PU_EMRHEADER;                            //!< EMF manual 2.2.9

/* Index  2,3,4,5,6  */
/** EMF manual 2.3.5.16
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< bounding rectangle in device units
    U_NUM_POINTL        cptl;               //!< Number of points to draw
    U_POINTL            aptl[1];            //!< array of points
} U_EMRPOLYBEZIER, 
  U_EMRPOLYGON,                             //!< EMF manual 2.3.5.22 
  U_EMRPOLYLINE,                            //!< EMF manual 2.3.5.24 
  U_EMRPOLYBEZIERTO,                        //!< EMF manual 2.3.5.18
  U_EMRPOLYLINETO,                          //!< EMF manual 2.3.5.26 
 *PU_EMRPOLYBEZIER,                         //!< EMF manual 2.3.5.16
 *PU_EMRPOLYGON,                            //!< EMF manual 2.3.5.22
 *PU_EMRPOLYLINE,                           //!< EMF manual 2.3.5.24
 *PU_EMRPOLYBEZIERTO,                       //!< EMF manual 2.3.5.18
 *PU_EMRPOLYLINETO;                         //!< EMF manual 2.3.5.26

/* Index  7,8 */
/** EMF manual 2.3.5.30

  After this struct the record also contains an array of points:\n
  U_POINTL            aptl[1];
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR     
    U_RECTL             rclBounds;          //!< bounding rectangle in device units
    U_NUM_POLYCOUNTS    nPolys;             //!< Number of elements in aPolyCounts
    U_NUM_POINTL        cptl;               //!< Total number of points (over all poly)
    U_POLYCOUNTS        aPolyCounts[1];     //!< Number of points in each poly (sequential)
} U_EMRPOLYPOLYLINE, 
  U_EMRPOLYPOLYGON,                         //!< EMF manual 2.3.5.28 
  *PU_EMRPOLYPOLYLINE,                      //!< EMF manual 2.3.5.30 
  *PU_EMRPOLYPOLYGON;                       //!< EMF manual 2.3.5.28 

/* Index  9,11 (numbers interleave with next one) */
/** EMF manual 2.3.11.30
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_SIZEL             szlExtent;          //!< H & V extent in logical units
} U_EMRSETWINDOWEXTEX,    
  U_EMRSETVIEWPORTEXTEX,                    //!< EMF manual manual 2.3.11.28
  *PU_EMRSETWINDOWEXTEX,                    //!< EMF manual manual 2.3.11.30
  *PU_EMRSETVIEWPORTEXTEX;                  //!< EMF manual manual 2.3.11.28

/* Index  10,12,13 */
/** EMF manual 2.3.11.31
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_POINTL            ptlOrigin;          //!< H & V origin in logical units
} U_EMRSETWINDOWORGEX,
  U_EMRSETVIEWPORTORGEX,                    //!< EMF manual 2.3.11.29
  U_EMRSETBRUSHORGEX,                       //!< EMF manual 2.3.11.12
  *PU_EMRSETWINDOWORGEX,                    //!< EMF manual 2.3.11.31
  *PU_EMRSETVIEWPORTORGEX,                  //!< EMF manual 2.3.11.29
  *PU_EMRSETBRUSHORGEX;                     //!< EMF manual 2.3.11.12


/* Index  14 */
/** EMF manual 3.2.4.1  

This is a very odd structure because the nSizeLast follows an optional variable size field.  Consequently
even though nSizeLast has a name it cannot actually be accessed by it!  Following the core structure
appear these fields:\n

 U_LOGPLTNTRY        PalEntries[1];      Record may include optional array of PalEntries

     uint32_t            nSizeLast;      Mandatory, but position isn't fixed. Must have same value as emr.nSize in header record

*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_CBPLENTRIES       cbPalEntries;       //!< Number of palette entries
    U_OFFPLENTRIES      offPalEntries;      //!< Offset in bytes to array of palette entries
} U_EMREOF, 
  *PU_EMREOF;                               //!< EMF manual 3.2.4.1

/* Index  15 */
/** EMF manual 2.3.5.36  
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_POINTL            ptlPixel;           //!< Pixel coordinates (logical)
    U_COLORREF          crColor;            //!< Pixel color
} U_EMRSETPIXELV, 
 *PU_EMRSETPIXELV;                          //!< EMF manual 2.3.5.36

/* Index  16 */
/** EMF manual 2.3.11.20
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            dwFlags;            //!< must be 1
} U_EMRSETMAPPERFLAGS,
  *PU_EMRSETMAPPERFLAGS;                    //!< EMF manual 2.3.11.20

/* Index  17,18,19,20,21,22,67,98,115 
*/
/** EMF manual 2.3.11.19 MapMode enumeration
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            iMode;              //!< enumeration varies with type
} U_EMRSETMAPMODE,
  U_EMRSETBKMODE,                           //!< EMF manual 2.3.11.11 BackgroundMode Enumeration
  U_EMRSETPOLYFILLMODE,                     //!< EMF manual 2.3.11.22 PolygonFillMode Enumeration
  U_EMRSETROP2,                             //!< EMF manual 2.3.11.23 Binary Raster Operation Enumeration
  U_EMRSETSTRETCHBLTMODE,                   //!< EMF manual 2.3.11.24 StretchMode Enumeration
  U_EMRSETTEXTALIGN,                        //!< EMF manual 2.3.11.25 TextAlignment enumeration
  U_EMRSELECTCLIPPATH,                      //!< EMF manual 2.3.2.5   RegionMode Enumeration
  U_EMRSETICMMODE,                          //!< EMF manual 2.3.11.14 ICMMode Enumeration
  U_EMRSETLAYOUT,                           //!< EMF manual 2.3.11.17 Mirroring Enumeration
  *PU_EMRSETMAPMODE,                        //!< EMF manual 2.3.11.19 MapMode enumeration
  *PU_EMRSETBKMODE,                         //!< EMF manual 2.3.11.11 BackgroundMode Enumeration
  *PU_EMRSETPOLYFILLMODE,                   //!< EMF manual 2.3.11.22 PolygonFillMode Enumeration
  *PU_EMRSETROP2,                           //!< EMF manual 2.3.11.23 Binary Raster Operation Enumeration
  *PU_EMRSETSTRETCHBLTMODE,                 //!< EMF manual 2.3.11.24 StretchMode Enumeration
  *PU_EMRSETTEXTALIGN,                      //!< EMF manual 2.3.11.25 TextAlignment enumeration
  *PU_EMRSELECTCLIPPATH,                    //!< EMF manual 2.3.2.5   RegionMode Enumeration
  *PU_EMRSETICMMODE,                        //!< EMF manual 2.3.11.14 ICMMode Enumeration
  *PU_EMRSETLAYOUT;                         //!< EMF manual 2.3.11.17 Mirroring Enumeration

/* Index  23 */
/** EMF manual 2.3.11.13         
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_COLORADJUSTMENT   ColorAdjustment;    //!< Color Adjustment
} U_EMRSETCOLORADJUSTMENT, 
  *PU_EMRSETCOLORADJUSTMENT;                //!< EMF manual 2.3.11.13

/* Index  24, 25 */
/** EMF manual 2.3.11.26         
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_COLORREF          crColor;            //!< Color
} U_EMRSETTEXTCOLOR,
  U_EMRSETBKCOLOR,                          //!< EMF manual 2.3.11.10
  *PU_EMRSETTEXTCOLOR,                      //!< EMF manual 2.3.11.26
  *PU_EMRSETBKCOLOR;                        //!< EMF manual 2.3.11.10

/* Index  26 */
/** EMF manual 2.3.2.4           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR           
    U_POINTL            ptlOffset;          //!< Clipping region 
} U_EMROFFSETCLIPRGN, 
  *PU_EMROFFSETCLIPRGN;                     //!< EMF manual 2.3.2.4

/* Index  27, 54 */
/** 
EMF manual 2.3.11.4          
EMF manual 2.3.5.13          
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_POINTL            ptl;                //!< Point coordinates
} U_EMRMOVETOEX,
  U_EMRLINETO,                              //!< EMF manual 2.3.5.13
  *PU_EMRMOVETOEX,                          //!< EMF manual 2.3.11.4 
  *PU_EMRLINETO;                            //!< EMF manual 2.3.5.13 

/* Index  28,33,52,59,60,61,65,66,68 */
/** EMF manual 2.3.2
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
}
  U_EMRSETMETARGN,    
  U_EMRSAVEDC,                              //!< EMF manual 2.3.11 
  U_EMRREALIZEPALETTE,                      //!< EMF manual 2.3.10
  U_EMRBEGINPATH,                           //!< EMF manual 2.3.10
  U_EMRENDPATH,                             //!< EMF manual 2.3.10
  U_EMRCLOSEFIGURE,                         //!< EMF manual 2.3.10
  U_EMRFLATTENPATH,                         //!< EMF manual 2.3.10
  U_EMRWIDENPATH,                           //!< EMF manual 2.3.10
  U_EMRABORTPATH,                           //!< EMF manual 2.3.10
  *PU_EMRSETMETARGN,                        //!< EMF manual 2.3.2
  *PU_EMRSAVEDC,                            //!< EMF manual 2.3.11
  *PU_EMRREALIZEPALETTE,                    //!< EMF manual 2.3.10
  *PU_EMRBEGINPATH,                         //!< EMF manual 2.3.10
  *PU_EMRENDPATH,                           //!< EMF manual 2.3.10
  *PU_EMRCLOSEFIGURE,                       //!< EMF manual 2.3.10
  *PU_EMRFLATTENPATH,                       //!< EMF manual 2.3.10
  *PU_EMRWIDENPATH,                         //!< EMF manual 2.3.10
  *PU_EMRABORTPATH;                         //!< EMF manual 2.3.10

/* Index  29,30  */
/** EMF manual 2.3.2.1           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclClip;            //!< Clipping Region
} U_EMREXCLUDECLIPRECT,  
  U_EMRINTERSECTCLIPRECT,                   //!< EMF manual 2.3.2.3
  *PU_EMREXCLUDECLIPRECT,                   //!< EMF manual 2.3.2.1
  *PU_EMRINTERSECTCLIPRECT;                 //!< EMF manual 2.3.2.3

/* Index  31,32 */
/** EMF manual 2.3.11.7          
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    int32_t             xNum;               //!< Horizontal multiplier (!=0)
    int32_t             xDenom;             //!< Horizontal divisor    (!=0) 
    int32_t             yNum;               //!< Vertical   multiplier (!=0)
    int32_t             yDenom;             //!< Vertical   divisor    (!=0)
} U_EMRSCALEVIEWPORTEXTEX,
  U_EMRSCALEWINDOWEXTEX,                    //!< EMF manual 2.3.11.8
  *PU_EMRSCALEVIEWPORTEXTEX,                //!< EMF manual 2.3.11.7
  *PU_EMRSCALEWINDOWEXTEX;                  //!< EMF manual 2.3.11.8 

/* Index  33  (see 28) */

/* Index  34  */
/** EMF manual 2.3.11.6          
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    int32_t             iRelative;          //!< DC to restore. -1 is preceding
} U_EMRRESTOREDC, 
  *PU_EMRRESTOREDC;                         //!< EMF manual 2.3.11.6

/* Index  35 */
/** EMF manual 2.3.12.2          
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_XFORM             xform;              //!< Transform
} U_EMRSETWORLDTRANSFORM, 
  *PU_EMRSETWORLDTRANSFORM;                 //!< EMF manual 2.3.12.2

/* Index  36 */
/** EMF manual 2.3.12.1          
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_XFORM             xform;              //!< Transform
    uint32_t            iMode;              //!< ModifyWorldTransformMode Enumeration
} U_EMRMODIFYWORLDTRANSFORM,
  *PU_EMRMODIFYWORLDTRANSFORM;              //!< EMF manual 2.3.12.1

/* Index  37,40 */
/** EMF manual 2.3.8.3           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihObject;           //!< Number of a stock or created object
} U_EMRDELETEOBJECT,
  U_EMRSELECTOBJECT,                        //!< EMF manual 2.3.8.5
  *PU_EMRDELETEOBJECT,                      //!< EMF manual 2.3.8.3
  *PU_EMRSELECTOBJECT;                      //!< EMF manual 2.3.8.5

/* Index  38 */
/** EMF manual 2.3.7.7           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihPen;              //!< Index to place object in EMF object table (this entry must not yet exist)
    U_LOGPEN            lopn;               //!< Pen properties
} U_EMRCREATEPEN, 
  *PU_EMRCREATEPEN;                         //!< EMF manual 2.3.7.7

/* Index  39  */
/** EMF manual 2.3.7.1           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihBrush;            //!< Index to place object in EMF object table (this entry must not yet exist)
    U_LOGBRUSH          lb;                 //!< Brush properties
} U_EMRCREATEBRUSHINDIRECT, 
  *PU_EMRCREATEBRUSHINDIRECT;               //!< EMF manual 2.3.7.1           

/* Index  40 see 37 */

/* Index  41  */
/** EMF manual 2.3.5.1           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_POINTL            ptlCenter;          //!< Center in logical units
    uint32_t            nRadius;            //!< Radius in logical units
    U_FLOAT             eStartAngle;        //!< Starting angle in degrees (counter clockwise from x axis)
    U_FLOAT             eSweepAngle;        //!< Sweep angle in degrees
} U_EMRANGLEARC, 
  *PU_EMRANGLEARC;                          //!< EMF manual 2.3.5.1           

/* Index  42,43  */
/** EMF manual 2.3.5.5           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBox;             //!< bounding rectangle in logical units
} U_EMRELLIPSE,  
  U_EMRRECTANGLE,                           //!< EMF manual 2.3.5.5           
  *PU_EMRELLIPSE,                           //!< EMF manual 2.3.5.5 
  *PU_EMRRECTANGLE;                         //!< EMF manual 2.3.5.34

/* Index  44  */
/** EMF manual 2.3.5.35          
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBox;             //!< bounding rectangle in logical units
    U_SIZEL             szlCorner;          //!< W & H in logical units of ellipse used to round corner
} U_EMRROUNDRECT, 
  *PU_EMRROUNDRECT;                         //!< EMF manual 2.3.5.35          

/* Index  45, 46 ,47, 55 */
/** EMF manual 2.3.5.2           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBox;             //!< bounding rectangle in logical units
    U_POINTL            ptlStart;           //!< Start point in logical units
    U_POINTL            ptlEnd;             //!< End point in logical units
} U_EMRARC,  
  U_EMRCHORD,                               //!< EMF manual 2.3.5.4 
  U_EMRPIE,                                 //!< EMF manual 2.3.5.15
  U_EMRARCTO,                               //!< EMF manual 2.3.5.3 
  *PU_EMRARC,                               //!< EMF manual 2.3.5.2 
  *PU_EMRCHORD,                             //!< EMF manual 2.3.5.4 
  *PU_EMRPIE,                               //!< EMF manual 2.3.5.15
  *PU_EMRARCTO;                             //!< EMF manual 2.3.5.3 

/* Index  48  */
/** EMF manual 2.3.8.6           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihPal;              //!< Index of a Palette object in the EMF object table
} U_EMRSELECTPALETTE, 
  *PU_EMRSELECTPALETTE;                     //!< EMF manual 2.3.8.6           

/* Index  49  */
/** EMF manual 2.3.7.6           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihPal;              //!< Index to place object in EMF object table (this entry must not yet exist)
    U_LOGPALETTE        lgpl;               //!< Palette properties
} U_EMRCREATEPALETTE,
  *PU_EMRCREATEPALETTE;                     //!< EMF manual 2.3.7.6           

/* Index  50 */
/** EMF manual 2.3.8.8           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihPal;              //!< Index of a Palette object in the EMF object table
    uint32_t            iStart;             //!< First Palette entry in selected object to set
    U_NUM_LOGPLTNTRY    cEntries;           //!< Number of Palette entries in selected object to set
    U_LOGPLTNTRY        aPalEntries[1];     //!< Values to set with
} U_EMRSETPALETTEENTRIES,
  *PU_EMRSETPALETTEENTRIES;                 //!< EMF manual 2.3.8.8           

/* Index  51 */
/** EMF manual 2.3.8.4           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihPal;              //!< Index of a Palette object in the EMF object table
    uint32_t            cEntries;           //!< Number to expand or truncate the Palette entry list to.
} U_EMRRESIZEPALETTE,
  *PU_EMRRESIZEPALETTE;                     //!< EMF manual 2.3.8.4           

/* Index  52  (see 28) */

/* Index  53 */
/** EMF manual 2.3.5.6           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_POINTL            ptlStart;           //!< Start point in logical units
    U_COLORREF          crColor;            //!< Color to fill with
    uint32_t            iMode;              //!< FloodFill Enumeration
} U_EMREXTFLOODFILL,
  *PU_EMREXTFLOODFILL;                      //!< EMF manual 2.3.5.6       

/* Index  54  (see 27) */

/* Index  55 (see 45) */

/* Index  56  */
/** EMF manual 2.3.5.20          
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_NUM_POINTL        cptl;               //!< Number of U_POINTL objects
    U_POINTL            aptl[1];            //!< Array of U_POINTL objects
    uint8_t             abTypes[1];         //!< Array of Point Enumeration 
} U_EMRPOLYDRAW, 
  *PU_EMRPOLYDRAW;                          //!< EMF manual 2.3.5.20          

/* Index  57  */
/** EMF manual 2.3.11.9          
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            iArcDirection;      //!< ArcDirection Enumeration
} U_EMRSETARCDIRECTION,
  *PU_EMRSETARCDIRECTION;                   //!< EMF manual 2.3.11.9

/* Index  58 */
/** EMF manual 2.3.11.21         

IMPORTANT!!!!  The Microsoft structure uses a float for the miterlimit but the EMF file record
uses an unsigned int.  The latter form is used in this structure.
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            eMiterLimit;        //!< Miter limit (max value of mitered length / line width) 
} U_EMRSETMITERLIMIT,
  *PU_EMRSETMITERLIMIT;                     //!< EMF manual 2.3.11.21         

/* Index  59,60,61  (see 28) */

/* Index  62,63,64  */
/** EMF manual 2.3.5.9           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
} U_EMRFILLPATH,
  U_EMRSTROKEANDFILLPATH,                   //!< EMF manual 2.3.5.38
  U_EMRSTROKEPATH,                          //!< EMF manual 2.3.5.39
  *PU_EMRFILLPATH,                          //!< EMF manual 2.3.5.9 
  *PU_EMRSTROKEANDFILLPATH,                 //!< EMF manual 2.3.5.38
  *PU_EMRSTROKEPATH;                        //!< EMF manual 2.3.5.39

/* Index  65,66  (see 28) */
/* Index  67  (see 17) */
/* Index  68  (see 28) */
/* Index  69  (not a defined U_EMR record type ) */


/* Index  70  */
/** EMF manual 2.3.3.1
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_CBDATA            cbData;             //!< Number of bytes in comment
    uint8_t             Data[1];            //!< Comment (any binary data, interpretation is program specific)
} U_EMRCOMMENT,
  *PU_EMRCOMMENT;                           //!< EMF manual 2.3.3.1, AKA GDICOMMENT

/* variant comment types */
/** EMF manual 2.3.3.2 
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_CBDATA            cbData;             //!< Number of bytes in comment
    uint32_t            cIdent;             //!< Comment identifier, must be U_EMR_COMMENT_EMFPLUSRECORD
    uint8_t             Data[1];            //!< EMF Plus record
} U_EMRCOMMENT_EMFPLUS,
  *PU_EMRCOMMENT_EMFPLUS;                   //!< EMF manual 2.3.3.2, EMF Plus comment

/** EMF manual 2.3.3.3 
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_CBDATA            cbData;             //!< Number of bytes in comment
    uint32_t            cIdent;             //!< Comment identifier, must be U_EMR_COMMENT_SPOOL
    uint32_t            esrIdent;           //!< EMFSpoolRecordIdentifier, may be  U_EMR_COMMENT_SPOOLFONTDEF
    uint8_t             Data[1];            //!< EMF Spool records
} U_EMRCOMMENT_SPOOL,
  *PU_EMRCOMMENT_SPOOL;                     //!< EMF manual 2.3.3.3, EMF Spool comment

/** EMF manual 2.3.3.4 
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_CBDATA            cbData;             //!< Number of bytes in comment
    uint32_t            cIdent;             //!< Comment identifier, must be U_EMR_COMMENT_PUBLIC
    uint32_t            pcIdent;            //!< Public Comment Identifier, from EMRComment Enumeration
    uint8_t             Data[1];            //!< Public comment data
} U_EMRCOMMENT_PUBLIC,
  *PU_EMRCOMMENT_PUBLIC;                    //!< EMF manual 2.3.3.4, EMF Public comment

/* Index  71  */
/** EMF manual 2.3.5.10          
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_CBRGNDATA         cbRgnData;          //!< Size in bytes of Region data
    uint32_t            ihBrush;            //!< Index of a Brush object in the EMF object table
    U_RGNDATA           RgnData[1];         //!< Variable size U_RGNDATA structure
} U_EMRFILLRGN,
  *PU_EMRFILLRGN;                           //!< EMF manual 2.3.5.10          

/* Index  72 */
/** EMF manual 2.3.5.11          
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_CBRGNDATA         cbRgnData;          //!< Size in bytes of Region data
    uint32_t            ihBrush;            //!< Index of a Brush object in the EMF object table
    U_SIZEL             szlStroke;          //!< W & H of Brush stroke
    U_RGNDATA           RgnData[1];         //!< Variable size U_RGNDATA structure
} U_EMRFRAMERGN,
  *PU_EMRFRAMERGN;                          //!< EMF manual 2.3.5.11          

/* Index  73,74 */
/** EMF manual 2.3.11.3          
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_CBRGNDATA         cbRgnData;          //!< Size in bytes of Region data
    U_RGNDATA           RgnData[1];         //!< Variable size U_RGNDATA structure
} U_EMRINVERTRGN,
  U_EMRPAINTRGN,                            //!< EMF manual 2.3.5.14
  *PU_EMRINVERTRGN,                         //!< EMF manual 2.3.11.3
  *PU_EMRPAINTRGN;                          //!< EMF manual 2.3.5.14

/* Index  75 */
/** EMF manual 2.3.2.2           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_CBRGNDATA         cbRgnData;          //!< Size in bytes of Region data 
    uint32_t            iMode;              //!< RegionMode Enumeration       
    U_RGNDATA           RgnData[1];         //!< Variable size U_RGNDATA structure          
} U_EMREXTSELECTCLIPRGN,
  *PU_EMREXTSELECTCLIPRGN;                  //!< EMF manual 2.3.2.2

/* Index  76 */
/** EMF manual 2.3.1.2           
*/
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
} U_EMRBITBLT,
  *PU_EMRBITBLT;                            //!< EMF manual 2.3.1.2

/* Index  77 */
/** EMF manual 2.3.1.6           
*/
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
} U_EMRSTRETCHBLT,
  *PU_EMRSTRETCHBLT;                        //!< EMF manual 2.3.1.6

/* Index  78  */
/** EMF manual 2.3.1.3           
*/
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
} U_EMRMASKBLT,
  *PU_EMRMASKBLT;                           //!< EMF manual 2.3.1.3

/* Index  79 */
/** EMF manual 2.3.1.4           
*/
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
} U_EMRPLGBLT,
  *PU_EMRPLGBLT;                            //!< EMF manual 2.3.1.4

/* Index  80 */
/** EMF manual 2.3.1.5           
*/
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
} U_EMRSETDIBITSTODEVICE,
  *PU_EMRSETDIBITSTODEVICE;                 //!< EMF manual 2.3.1.5

/* Index  81 */
/** EMF manual 2.3.1.7           
*/
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
} U_EMRSTRETCHDIBITS,
  *PU_EMRSTRETCHDIBITS;                     //!< EMF manual 2.3.1.7

/* Index  82 */
/** EMF manual 2.3.7.8           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihFont;             //!< Index of the font in the EMF object table
    U_LOGFONT_PANOSE    elfw;               //!< Font parameters, either U_LOGFONT or U_LOGFONT_PANOSE, the latter is bigger so use that type here
} U_EMREXTCREATEFONTINDIRECTW,
  *PU_EMREXTCREATEFONTINDIRECTW;            //!< EMF manual 2.3.7.8

/* Index  83,84  */
/** EMF manual 2.3.5.7           

Variable and optional fields may follow core structure in record:\n

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
} U_EMREXTTEXTOUTA,
  U_EMREXTTEXTOUTW,                         //!< EMF manual 2.3.5.8
 *PU_EMREXTTEXTOUTA,                        //!< EMF manual 2.3.5.7
 *PU_EMREXTTEXTOUTW;                        //!< EMF manual 2.3.5.8

/* Index  85,86,87,88,89 */
/** EMF manual 2.3.5.17          
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_NUM_POINT16       cpts;               //!< Number of POINT16 in array
    U_POINT16           apts[1];            //!< Array of POINT16
} U_EMRPOLYBEZIER16,  
  U_EMRPOLYGON16,                           //!< EMF manual 2.3.5.23
  U_EMRPOLYLINE16,                          //!< EMF manual 2.3.5.25
  U_EMRPOLYBEZIERTO16,                      //!< EMF manual 2.3.5.19
  U_EMRPOLYLINETO16,                        //!< EMF manual 2.3.5.27
  *PU_EMRPOLYBEZIER16,                      //!< EMF manual 2.3.5.17
  *PU_EMRPOLYGON16,                         //!< EMF manual 2.3.5.23
  *PU_EMRPOLYLINE16,                        //!< EMF manual 2.3.5.25
  *PU_EMRPOLYBEZIERTO16,                    //!< EMF manual 2.3.5.19
  *PU_EMRPOLYLINETO16;                      //!< EMF manual 2.3.5.27
                                                 
/* Index  90,91 */
/** EMF manual 2.3.5.31          
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_NUM_POLYCOUNTS    nPolys;             //!< Number of elements in aPolyCounts
    U_NUM_POINT16       cpts;               //!< Total number of points (over all poly)
    U_POLYCOUNTS        aPolyCounts[1];     //!< Number of points in each poly (sequential)
//  This will appear somewhere but is not really part of the core structure.
//    U_POINT16           apts[1];          //!< array of point16
} U_EMRPOLYPOLYLINE16,
  U_EMRPOLYPOLYGON16,                       //!< EMF manual 2.3.5.29
  *PU_EMRPOLYPOLYLINE16,                    //!< EMF manual 2.3.5.31
  *PU_EMRPOLYPOLYGON16;                     //!< EMF manual 2.3.5.29

/* Index  92 */
/** EMF manual 2.3.5.21          
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_NUM_POINT16       cpts;               //!< Total number of points (over all poly)
    U_POINT16           apts[1];            //!< array of points
    uint8_t             abTypes[1];         //!< Array of Point Enumeration
} U_EMRPOLYDRAW16,
  *PU_EMRPOLYDRAW16;                        //!< EMF manual 2.3.5.21

/* Index  93  */
/** EMF manual 2.3.7.5           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihBrush;            //!< Index to place object in EMF object table (this entry must not yet exist)
    uint32_t            iUsage;             //!< DIBcolors Enumeration
    U_OFFBMI            offBmi;             //!< Offset in bytes to U_BITMAPINFO (within DIBbitmapbuffer)
    U_CBBMI             cbBmi;              //!< Size   in bytes of U_BITMAPINFO
    U_OFFBITS           offBits;            //!< Offset in bytes to the DIB bitmap data (within DIBbitmapbuffer
    U_CBBITS            cbBits;             //!< Size in bytes of DIB bitmap
                                            //!< Record may include optional DIB bitmapbuffer
} U_EMRCREATEMONOBRUSH,
  *PU_EMRCREATEMONOBRUSH;                   //!< EMF manual 2.3.7.5

/* Index  94  */
/** EMF manual 2.3.7.4           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihBrush;            //!< Index to place object in EMF object table (this entry must not yet exist)
    uint32_t            iUsage;             //!< DIBcolors Enumeration
    U_OFFBMI            offBmi;             //!< Offset in bytes to U_BITMAPINFO (within DIB bitmapbuffer)
    U_CBBMI             cbBmi;              //!< Size   in bytes of U_BITMAPINFO
    U_OFFBITS           offBits;            //!< Offset in bytes to the DIB bitmap data (within DIB bitmapbuffer
    U_CBBITS            cbBits;             //!< Size in bytes of DIB bitmap
                                            //!< Record may include optional DIB bitmapbuffer
} U_EMRCREATEDIBPATTERNBRUSHPT,
  *PU_EMRCREATEDIBPATTERNBRUSHPT;           //!< EMF manual 2.3.7.4

/* Index  95 */
/** EMF manual 2.3.7.9           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihPen;              //!< Index to place object in EMF object table (this entry must not yet exist)
    U_OFFBMI            offBmi;             //!< Offset in bytes to U_BITMAPINFO (within DIB bitmapbuffer)
    U_CBBMI             cbBmi;              //!< Size   in bytes of U_BITMAPINFO
    U_OFFBITS           offBits;            //!< Offset in bytes to the DIB bitmap data (within DIB bitmapbuffer
    U_CBBITS            cbBits;             //!< Size in bytes of DIB bitmap
    U_EXTLOGPEN         elp;                //!< Pen parameters (Size is Variable!!!!)
                                            //!< Record may include optional DIB bitmap
} U_EMREXTCREATEPEN,
  *PU_EMREXTCREATEPEN;                      //!< EMF manual 2.3.7.9

/* Index  96.97  */
/** EMF manual 2.3.5.32          
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    uint32_t            iGraphicsMode;      //!< GraphicsMode Enumeration
    U_FLOAT             exScale;            //!< scale to 0.01 mm units ( only if iGraphicsMode & U_GM_COMPATIBLE)
    U_FLOAT             eyScale;            //!< scale to 0.01 mm units ( only if iGraphicsMode & U_GM_COMPATIBLE)
    U_NUM_EMRTEXT       cStrings;           //!< Number of U_EMRTEXT in array
    U_EMRTEXT           emrtext[1];         //!< Text parameters
} U_EMRPOLYTEXTOUTA,
  U_EMRPOLYTEXTOUTW,                        //!< EMF manual 2.3.5.33
  *PU_EMRPOLYTEXTOUTA,                      //!< EMF manual 2.3.5.32
  *PU_EMRPOLYTEXTOUTW;                      //!< EMF manual 2.3.5.33

/* Index  98  (see 17) */

/* Index  99 */
/** EMF manual 2.3.7.2           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihCS;               //!< Index to place object in EMF object table (this entry must not yet exist)
    U_LOGCOLORSPACEA    lcs;                //!< ColorSpace parameters
} U_EMRCREATECOLORSPACE,
  *PU_EMRCREATECOLORSPACE;                  //!< EMF manual 2.3.7.2           

/* Index  100,101 */
/** EMF manual 2.3.8.2           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihCS;               //!< Index of object in EMF object table
} U_EMRDELETECOLORSPACE,
  U_EMRSETCOLORSPACE,                       //!< EMF manual 2.3.8.7
  *PU_EMRDELETECOLORSPACE,                  //!< EMF manual 2.3.8.2
  *PU_EMRSETCOLORSPACE;                     //!< EMF manual 2.3.8.7

/* Index  102 */
/** EMF manual 2.3.9.2           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_CBDATA            cbData;             //!< Size of OpenGL data in bytes
    U_DATA              Data[1];            //!< OpenGL data
} U_EMRGLSRECORD,
  *PU_EMRGLSRECORD;                         //!< EMF manual 2.3.9.2           

/* Index  103 */
/** EMF manual 2.3.9.1           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_CBDATA            cbData;             //!< Size of OpenGL data in bytes
    U_DATA              Data[1];            //!< OpenGL data
} U_EMRGLSBOUNDEDRECORD,
  *PU_EMRGLSBOUNDEDRECORD;                  //!<  EMF manual 2.3.9.1

/* Index  104 */
/** EMF manual 2.3.11.5          
*/
typedef struct {
    U_EMR                   emr;            //!< U_EMR
    U_PIXELFORMATDESCRIPTOR pfd;            //!< PixelFormatDescriptor
} U_EMRPIXELFORMAT,
  *PU_EMRPIXELFORMAT;                       //!< EMF manual 2.3.11.5

/* Index  105 */
/** EMF manual 2.3.6.1           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR 
    U_CBDATA            cjIn;               //!< Number of bytes to send to printer driver
    U_DATA              Data[1];            //!< Data to send
} U_EMRDRAWESCAPE,
  *PU_EMRDRAWESCAPE;                        //!< EMF manual 2.3.6.1

/* Index  106 */
/** EMF manual 2.3.6.2           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR 
    U_CBDATA            cjIn;               //!< Number of bytes to send to printer driver
    U_DATA              Data[1];            //!< Data to send
} U_EMREXTESCAPE,
  *PU_EMREXTESCAPE;                         //!< EMF manual 2.3.6.2

/* Index  107 (not implemented ) */

/* Index  108 */
/** EMF manual 2.3.5.37          
*/
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
} U_EMRSMALLTEXTOUT,
  *PU_EMRSMALLTEXTOUT;                      //!< EMF manual 2.3.5.37

/* Index  109 (not implemented )
EMF manual 2.3.11.2          
*/

/* Index  110 */
/** EMF manual 2.3.6.3           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR       
    U_CBDATA            cbDriver;           //!< Number of bytes in driver name (note, BYTES, not CHARACTERS)
    U_CBDATA            cbData;             //!< Number of bytes in data
    uint16_t            Driver[1];          //!< Driver name in uint16_t characters, null terminated
    uint8_t             Data[1];            //!< Data for printer driver
} U_EMRNAMEDESCAPE,
  *PU_EMRNAMEDESCAPE;                       //!< EMF manual 2.3.6.3

/* Index  111-113 (not implemented ) 
  EMF manual 2.3.8.1           
  EMF manual 2.3.11.15
  EMF manual 2.3.11.16
*/

/* Index 114 */
/** EMF manual 2.3.1.1           
*/
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
} U_EMRALPHABLEND,
  *PU_EMRALPHABLEND;                        //!< EMF manual 2.3.1.1

/* Index  115  (see 17) */

/* Index  116  */
/** EMF manual 2.3.1.8           
*/
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
} U_EMRTRANSPARENTBLT,
  *PU_EMRTRANSPARENTBLT;                    //!< EMF manual 2.3.1.8

/* Index  117 (not a defined U_EMR record type ) */

/* Index  118 */
/** EMF manual 2.3.5.12          
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    U_RECTL             rclBounds;          //!< Bounding rectangle in device units
    U_NUM_TRIVERTEX     nTriVert;           //!< Number of TriVertex objects
    U_NUM_GRADOBJ       nGradObj;           //!< Number of gradient triangle/rectangle objects
    uint32_t            ulMode;             //!< Gradientfill Enumeration (determines Triangle/Rectangle)
//parts that are required but which are not included in the core structure
//    U_TRIVERTEX         TriVert[1];          Array of TriVertex objects
//    uint32_t            GradObj[1];          Array of gradient objects (each has 2 or 3 indices into TriVert array) 
} U_EMRGRADIENTFILL,
  *PU_EMRGRADIENTFILL;                      //!< EMF manual 2.3.5.12

/* Index  119,120 (not implemented )
EMF manual 2.3.11.18         
EMF manual 2.3.11.27         
*/

/* Index  121 */
/** EMF manual 2.3.11.1          
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            dwAction;           //!< ColorSpace Enumeration
    uint32_t            dwFlags;            //!< ColorMatchToTarget Enumeration
    U_CBNAME            cbName;             //!< Number of bytes in UTF16 name of the color profile
    U_CBDATA            cbData;             //!< Number of bytes of the target profile
    uint8_t             Data[1];            //!< Data of size cbName+cbData: Name in UTF16 then color profile data 
} U_EMRCOLORMATCHTOTARGETW,
  *PU_EMRCOLORMATCHTOTARGETW;               //!< EMF manual 2.3.11.1 

/* Index  122 */
/** EMF manual 2.3.7.3           
*/
typedef struct {
    U_EMR               emr;                //!< U_EMR
    uint32_t            ihCS;               //!< Index of the logical color space object in the EMF object table 
    U_LOGCOLORSPACEW    lcs;                //!< Description of the color profile
    uint32_t            dwFlags;            //!< If low bit set Data is present
    U_CBDATA            cbData;             //!< Number of bytes of theData field.
    uint8_t             Data[1];            //!< (Optional, dwFlags & 1) color profile data 
} U_EMRCREATECOLORSPACEW,
  *PU_EMRCREATECOLORSPACEW;                 //!< EMF manual 2.3.7.3

// ************************************************************************************************
// Utility function structures

/**
  Storage for keeping track of properties of the growing EMF file as records are added.
*/
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
} U_MAT2X2,
  *PU_MAT2X2;                               //!< 2 x 2 matrix, used by xform_alt_set() function.

// ************************************************************************************************
// Prototypes

//! \cond
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
U_COLORREF       colorref3_set(uint8_t red, uint8_t green, uint8_t blue);
U_COLORREF       colorref4_set(uint8_t red, uint8_t green, uint8_t blue, uint8_t reserved);
U_RGBQUAD        rgbquad_set(uint8_t red,uint8_t green, uint8_t blue,  uint8_t reserved);
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

//! \endcond

#ifdef __cplusplus
}
#endif

#endif /* _UEMF_ */
