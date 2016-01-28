/**
  @file upmf.h
  
  @brief Structures, definitions, and function prototypes for EMF+ files.

  EMF+ file Record structure derived from Microsoft's EMF+ Information pdf, releade date July 5,2012, link from
  here:
  
     http://msdn.microsoft.com/en-us/library/cc230724.aspx

  If the direct link fails the document may be found
  by searching for:  "[MS-EMFPLUS]:   Enhanced Metafile Format Plus Extensions "
  
  EMR records and structures are EMF or common with EMF+
  PMR records and structures are specific to EMF+
  
  Using PMF instead of EMF+ because "+" is a problem in symbol names.
  
  *****************************************************************************************
  * WARNING:  Microsoft's EMF+ documentation is little-endian for everything EXCEPT       *
  * bitfields,  which are big-endian.   See section 1.3.2                                 *
  * That documentation also uses 0 as the MOST significant bit, N-1 as the least.         *
  * This code is little-endian throughout, and 0 is the LEAST significant bit             *
  *****************************************************************************************
  
*/

/*
File:      upmf.h
Version:   0.0.5
Date:      26-JAN-2016
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2016 David Mathog and California Institute of Technology (Caltech)
*/

#ifndef _UPMF_
#define _UPMF_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "uemf.h"


/** \defgroup U_PMF_PMF_Misc  PMF Miscellaneous defines
  @{
*/
#define   U_PMF_DROP_ELEMENTS 1  //!< Remove leading Elements value from data.
#define   U_PMF_KEEP_ELEMENTS 0  //!< Retain leading Elements value from data.

#define   U_SEG_NEW                    1    //!<  start a new segment in the path
#define   U_SEG_OLD                    0    //!<  continue the old (current) segment in the path

#define   U_FILTER_APPLY               1    //!<  With U_PMR_DRAWIMAGEPOINTS_set, use whatever filter has been set up up.
#define   U_FILTER_IGNORE              0    //!<  With U_PMR_DRAWIMAGEPOINTS_set, ignore whatever filter has been set up up.
#define   U_OBJRECLIM              65020    //!< Longest U_PMR_OBJECT that GDI+ will process
                                            // used 9728 to test fragmenting of emitted object records

/** @} */

/** \defgroup U_PMF_DD_  PMF "standard" custom Dash Dot patterns for lines.

  U_DD_DASH, U_DD_DOT, U_DD_DASHDOT, and U_DD_DASHDOTDOT are the only ones with corresponding
    standard EMF and EMF+ dash/dot patterns.
    
  These values are used to tell U_PMF_DASHEDLINEDATA_set2() to create one of 27 custom line patterns.  
  Other custom line patterns may be created using U_PMF_DASHEDLINEDATA_set(), but this provides an easier
  way to get the same result if one of these patterns is acceptable.

  The length is divided by 2X the number of elements, so dashdash has twice as many
  dashes of half the length as just dash.

        Dot  is 1/8 of (sub)unit length
        Dash is 1/2 of (sub)unit length
        Long is 3/4 of (sub)unit length
        Example: DotDot has (sub)unit length 1/2, so each dot will be 1/16 of unit length.
  
  @{
*/
#define U_DD_Solid                     0 //!< Solid line.
#define U_DD_Dash                      1 //!< Dash line.
#define U_DD_DashDash                  2 //!< Dash Dash line.
#define U_DD_DashDashDash              3 //!< Dash Dash Dash line.
#define U_DD_DashDashDashDash          4 //!< Dash Dash Dash Dash line.
#define U_DD_Dot                       5 //!< Dot line.
#define U_DD_DotDot                    6 //!< Dot Dot line.
#define U_DD_DotDotDot                 7 //!< Dot Dot Dot line.
#define U_DD_DotDotDotDot              8 //!< Dot Dot Dot Dot line.
#define U_DD_DashDot                   9 //!< Dash Dot line.
#define U_DD_DashDashDot              10 //!< Dash Dash Dot line.
#define U_DD_DashDashDotDot           11 //!< Dash Dash Dot Dot line.
#define U_DD_DashDashDashDot          12 //!< Dash Dash Das hDot line.
#define U_DD_DashDotDot               13 //!< Dash Dot Dot line.
#define U_DD_DashDotDotDot            14 //!< Dash Dot Dot Dot line.
#define U_DD_DashDotDashDot           15 //!< Dash Dot Dash Dot line.
#define U_DD_Long                     16 //!< Long line.
#define U_DD_LongLong                 17 //!< Long Long line.
#define U_DD_LongLongLong             18 //!< Long Long Long line.
#define U_DD_LongLongLongLong         19 //!< Long Long Long Long line.
#define U_DD_LongDot                  20 //!< Long Dot line.
#define U_DD_LongLongDot              21 //!< Long Long Dot line.
#define U_DD_LongLongDotDot           22 //!< Long Long Dot Dot line.
#define U_DD_LongLongLongDot          23 //!< Long Long Long Dot line.
#define U_DD_LongDotDot               24 //!< Long Dot Dot line.
#define U_DD_LongDotDotDot            25 //!< Long Dot Dot Dot line.
#define U_DD_LongDotLongDot           26 //!< Long Dot Long Dot line.
#define U_DD_Types                    27 //!< Types

/** @} */


/** \defgroup U_PMF_PMR_Qualifiers PMF RecordType Enumeration
  EMF+ manual 2.1.1.1, Microsoft name: RecordType Enumeration
  @{
*/
#define   U_PMR_HEADER                            0x0001 //!< U_PMRHeader                  record
#define   U_PMR_ENDOFFILE                         0x0002 //!< U_PMREndOfFile               record
#define   U_PMR_COMMENT                           0x0003 //!< U_PMRComment                 record
#define   U_PMR_GETDC                             0x0004 //!< U_PMRGetDC                   record
#define   U_PMR_MULTIFORMATSTART                  0x0005 //!< U_PMRMultiFormatStart        record
#define   U_PMR_MULTIFORMATSECTION                0x0006 //!< U_PMRMultiFormatSection      record
#define   U_PMR_MULTIFORMATEND                    0x0007 //!< U_PMRMultiFormatEnd          record
#define   U_PMR_OBJECT                            0x0008 //!< U_PMRObject                  record
#define   U_PMR_CLEAR                             0x0009 //!< U_PMRClear                   record
#define   U_PMR_FILLRECTS                         0x000A //!< U_PMRFillRects               record
#define   U_PMR_DRAWRECTS                         0x000B //!< U_PMRDrawRects               record
#define   U_PMR_FILLPOLYGON                       0x000C //!< U_PMRFillPolygon             record
#define   U_PMR_DRAWLINES                         0x000D //!< U_PMRDrawLines               record
#define   U_PMR_FILLELLIPSE                       0x000E //!< U_PMRFillEllipse             record
#define   U_PMR_DRAWELLIPSE                       0x000F //!< U_PMRDrawEllipse             record
#define   U_PMR_FILLPIE                           0x0010 //!< U_PMRFillPie                 record
#define   U_PMR_DRAWPIE                           0x0011 //!< U_PMRDrawPie                 record
#define   U_PMR_DRAWARC                           0x0012 //!< U_PMRDrawArc                 record
#define   U_PMR_FILLREGION                        0x0013 //!< U_PMRFillRegion              record
#define   U_PMR_FILLPATH                          0x0014 //!< U_PMRFillPath                record
#define   U_PMR_DRAWPATH                          0x0015 //!< U_PMRDrawPath                record
#define   U_PMR_FILLCLOSEDCURVE                   0x0016 //!< U_PMRFillClosedCurve         record
#define   U_PMR_DRAWCLOSEDCURVE                   0x0017 //!< U_PMRDrawClosedCurve         record
#define   U_PMR_DRAWCURVE                         0x0018 //!< U_PMRDrawCurve               record
#define   U_PMR_DRAWBEZIERS                       0x0019 //!< U_PMRDrawBeziers             record
#define   U_PMR_DRAWIMAGE                         0x001A //!< U_PMRDrawImage               record
#define   U_PMR_DRAWIMAGEPOINTS                   0x001B //!< U_PMRDrawImagePoints         record
#define   U_PMR_DRAWSTRING                        0x001C //!< U_PMRDrawString              record
#define   U_PMR_SETRENDERINGORIGIN                0x001D //!< U_PMRSetRenderingOrigin      record
#define   U_PMR_SETANTIALIASMODE                  0x001E //!< U_PMRSetAntiAliasMode        record
#define   U_PMR_SETTEXTRENDERINGHINT              0x001F //!< U_PMRSetTextRenderingHint    record
#define   U_PMR_SETTEXTCONTRAST                   0x0020 //!< U_PMRSetTextContrast         record
#define   U_PMR_SETINTERPOLATIONMODE              0x0021 //!< U_PMRSetInterpolationMode    record
#define   U_PMR_SETPIXELOFFSETMODE                0x0022 //!< U_PMRSetPixelOffsetMode      record
#define   U_PMR_SETCOMPOSITINGMODE                0x0023 //!< U_PMRSetCompositingMode      record
#define   U_PMR_SETCOMPOSITINGQUALITY             0x0024 //!< U_PMRSetCompositingQuality   record
#define   U_PMR_SAVE                              0x0025 //!< U_PMRSave                    record
#define   U_PMR_RESTORE                           0x0026 //!< U_PMRRestore                 record
#define   U_PMR_BEGINCONTAINER                    0x0027 //!< U_PMRBeginContainer          record
#define   U_PMR_BEGINCONTAINERNOPARAMS            0x0028 //!< U_PMRBeginContainerNoParams  record
#define   U_PMR_ENDCONTAINER                      0x0029 //!< U_PMREndContainer            record
#define   U_PMR_SETWORLDTRANSFORM                 0x002A //!< U_PMRSetWorldTransform       record
#define   U_PMR_RESETWORLDTRANSFORM               0x002B //!< U_PMRResetWorldTransform     record
#define   U_PMR_MULTIPLYWORLDTRANSFORM            0x002C //!< U_PMRMultiplyWorldTransform  record
#define   U_PMR_TRANSLATEWORLDTRANSFORM           0x002D //!< U_PMRTranslateWorldTransform record
#define   U_PMR_SCALEWORLDTRANSFORM               0x002E //!< U_PMRScaleWorldTransform     record
#define   U_PMR_ROTATEWORLDTRANSFORM              0x002F //!< U_PMRRotateWorldTransform    record
#define   U_PMR_SETPAGETRANSFORM                  0x0030 //!< U_PMRSetPageTransform        record
#define   U_PMR_RESETCLIP                         0x0031 //!< U_PMRResetClip               record
#define   U_PMR_SETCLIPRECT                       0x0032 //!< U_PMRSetClipRect             record
#define   U_PMR_SETCLIPPATH                       0x0033 //!< U_PMRSetClipPath             record
#define   U_PMR_SETCLIPREGION                     0x0034 //!< U_PMRSetClipRegion           record
#define   U_PMR_OFFSETCLIP                        0x0035 //!< U_PMROffsetClip              record
#define   U_PMR_DRAWDRIVERSTRING                  0x0036 //!< U_PMRDrawDriverstring        record
#define   U_PMR_STROKEFILLPATH                    0x0037 //!< U_PMRStrokeFillPath          record
#define   U_PMR_SERIALIZABLEOBJECT                0x0038 //!< U_PMRSerializableObject      record
#define   U_PMR_SETTSGRAPHICS                     0x0039 //!< U_PMRSetTSGraphics           record
#define   U_PMR_SETTSCLIP                         0x003A //!< U_PMRSetTSClip               record
#define   U_PMR_RECFLAG                           0x4000 //!< In EMF+ files the type is one of the above + this flag
#define   U_PMR_TYPE_MASK                         0x003F //!< mask for EMF+ types
#define   U_PMR_MIN                                    1 //!< Minimum U_PMR_ value.
#define   U_PMR_MAX                                   58 //!< Maximum U_PMR_ value.

/** @} */

/** \defgroup U_PMF_PID_Values PMF Identifiers for PseudoObjects
  These are used by the *_set routines to identify types of PseudoObject. 
  Note that records are U_PMR_*_OID and other objects are U_PMF_*_OID
  The numbers are derived from the EMF+ manual sections, as in 2.2.1.3 become
  02020103.  Numbers 40000000 and up are not derived from manual setions.
  @{
*/
#define U_UNDEFINED_OID                            0x00000000 //!< Undefined PseudoObject       
#define U_PMF_BRUSH_OID                            0x02020101 //!< PMF_BRUSH PseudoObject type.                                 
#define U_PMF_CUSTOMLINECAP_OID                    0x02020102 //!< PMF_CUSTOMLINECAP PseudoObject type.                         
#define U_PMF_FONT_OID                             0x02020103 //!< PMF_FONT PseudoObject type.                                  
#define U_PMF_IMAGE_OID                            0x02020104 //!< PMF_IMAGE PseudoObject type.                                 
#define U_PMF_IMAGEATTRIBUTES_OID                  0x02020105 //!< PMF_IMAGEATTRIBUTES PseudoObject type.                       
#define U_PMF_PATH_OID                             0x02020106 //!< PMF_PATH PseudoObject type.                                  
#define U_PMF_PEN_OID                              0x02020107 //!< PMF_PEN PseudoObject type.                                   
#define U_PMF_REGION_OID                           0x02020108 //!< PMF_REGION PseudoObject type.                                
#define U_PMF_STRINGFORMAT_OID                     0x02020109 //!< PMF_STRINGFORMAT PseudoObject type.                          
#define U_PMF_ARGB_OID                             0x02020201 //!< PMF_ARGB PseudoObject type.                                  
#define U_PMF_BITMAP_OID                           0x02020202 //!< PMF_BITMAP PseudoObject type.                                
#define U_PMF_BITMAPDATA_OID                       0x02020203 //!< PMF_BITMAPDATA PseudoObject type.                            
#define U_PMF_BLENDCOLORS_OID                      0x02020204 //!< PMF_BLENDCOLORS PseudoObject type.                           
#define U_PMF_BLENDFACTORS_OID                     0x02020205 //!< PMF_BLENDFACTORS PseudoObject type.                          
#define U_PMF_BOUNDARYPATHDATA_OID                 0x02020206 //!< PMF_BOUNDARYPATHDATA PseudoObject type.                      
#define U_PMF_BOUNDARYPOINTDATA_OID                0x02020207 //!< PMF_BOUNDARYPOINTDATA PseudoObject type.                     
#define U_PMF_CHARACTERRANGE_OID                   0x02020208 //!< PMF_CHARACTERRANGE PseudoObject type.                        
#define U_PMF_COMPOUNDLINEDATA_OID                 0x02020209 //!< PMF_COMPOUNDLINEDATA PseudoObject type.                      
#define U_PMF_COMPRESSEDIMAGE_OID                  0x02020210 //!< PMF_COMPRESSEDIMAGE PseudoObject type.                      
#define U_PMF_CUSTOMENDCAPDATA_OID                 0x02020211 //!< PMF_CUSTOMENDCAPDATA PseudoObject type.                     
#define U_PMF_CUSTOMLINECAPARROWDATA_OID           0x02020212 //!< PMF_CUSTOMLINECAPARROWDATA PseudoObject type.               
#define U_PMF_CUSTOMLINECAPDATA_OID                0x02020213 //!< PMF_CUSTOMLINECAPDATA PseudoObject type.                    
#define U_PMF_CUSTOMLINECAPOPTIONALDATA_OID        0x02020214 //!< PMF_CUSTOMLINECAPOPTIONALDATA PseudoObject type.            
#define U_PMF_CUSTOMSTARTCAPDATA_OID               0x02020215 //!< PMF_CUSTOMSTARTCAPDATA PseudoObject type.                   
#define U_PMF_DASHEDLINEDATA_OID                   0x02020216 //!< PMF_DASHEDLINEDATA PseudoObject type.                       
#define U_PMF_FILLPATHOBJ_OID                      0x02020217 //!< PMF_FILLPATHOBJ PseudoObject type.                          
#define U_PMF_FOCUSSCALEDATA_OID                   0x02020218 //!< PMF_FOCUSSCALEDATA PseudoObject type.                       
#define U_PMF_GRAPHICSVERSION_OID                  0x02020219 //!< PMF_GRAPHICSVERSION PseudoObject type.                      
#define U_PMF_HATCHBRUSHDATA_OID                   0x02020220 //!< PMF_HATCHBRUSHDATA PseudoObject type.                       
#define U_PMF_INTEGER7_OID                         0x02020221 //!< PMF_INTEGER7 PseudoObject type.                             
#define U_PMF_INTEGER15_OID                        0x02020222 //!< PMF_INTEGER15 PseudoObject type.                            
#define U_PMF_LANGUAGEIDENTIFIER_OID               0x02020223 //!< PMF_LANGUAGEIDENTIFIER PseudoObject type.                   
#define U_PMF_LINEARGRADIENTBRUSHDATA_OID          0x02020224 //!< PMF_LINEARGRADIENTBRUSHDATA PseudoObject type.              
#define U_PMF_LINEARGRADIENTBRUSHOPTIONALDATA_OID  0x02020225 //!< PMF_LINEARGRADIENTBRUSHOPTIONALDATA PseudoObject type.      
#define U_PMF_LINEPATH_OID                         0x02020226 //!< PMF_LINEPATH PseudoObject type.                             
#define U_PMF_METAFILE_OID                         0x02020227 //!< PMF_METAFILE PseudoObject type.                             
#define U_PMF_PALETTE_OID                          0x02020228 //!< PMF_PALETTE PseudoObject type.                              
#define U_PMF_PATHGRADIENTBRUSHDATA_OID            0x02020229 //!< PMF_PATHGRADIENTBRUSHDATA PseudoObject type.                
#define U_PMF_PATHGRADIENTBRUSHOPTIONALDATA_OID    0x02020230 //!< PMF_PATHGRADIENTBRUSHOPTIONALDATA PseudoObject type.        
#define U_PMF_PATHPOINTTYPE_OID                    0x02020231 //!< PMF_PATHPOINTTYPE PseudoObject type.                        
#define U_PMF_PATHPOINTTYPERLE_OID                 0x02020232 //!< PMF_PATHPOINTTYPERLE PseudoObject type.                     
#define U_PMF_PENDATA_OID                          0x02020233 //!< PMF_PENDATA PseudoObject type.                              
#define U_PMF_PENOPTIONALDATA_OID                  0x02020234 //!< PMF_PENOPTIONALDATA PseudoObject type.                      
#define U_PMF_POINT_OID                            0x02020235 //!< PMF_POINT PseudoObject type.                                
#define U_PMF_POINTF_OID                           0x02020236 //!< PMF_POINTF PseudoObject type.                               
#define U_PMF_POINTR_OID                           0x02020237 //!< PMF_POINTR PseudoObject type.                               
#define U_PMF_RECT_OID                             0x02020238 //!< PMF_RECT PseudoObject type.                                 
#define U_PMF_RECTF_OID                            0x02020239 //!< PMF_RECTF PseudoObject type.                                
#define U_PMF_REGIONNODE_OID                       0x02020240 //!< PMF_REGIONNODE PseudoObject type.                           
#define U_PMF_REGIONNODECHILDNODES_OID             0x02020241 //!< PMF_REGIONNODECHILDNODES PseudoObject type.                 
#define U_PMF_REGIONNODEPATH_OID                   0x02020242 //!< PMF_REGIONNODEPATH PseudoObject type.                       
#define U_PMF_SOLIDBRUSHDATA_OID                   0x02020243 //!< PMF_SOLIDBRUSHDATA PseudoObject type.                       
#define U_PMF_STRINGFORMATDATA_OID                 0x02020244 //!< PMF_STRINGFORMATDATA PseudoObject type.                     
#define U_PMF_TEXTUREBRUSHDATA_OID                 0x02020245 //!< PMF_TEXTUREBRUSHDATA PseudoObject type.                     
#define U_PMF_TEXTUREBRUSHOPTIONALDATA_OID         0x02020246 //!< PMF_TEXTUREBRUSHOPTIONALDATA PseudoObject type.             
#define U_PMF_TRANSFORMMATRIX_OID                  0x02020247 //!< PMF_TRANSFORMMATRIX PseudoObject type.                      
#define U_PMF_IE_BLUR_OID                          0x02020301 //!< PMF_IE_BLUR PseudoObject type.                               
#define U_PMF_IE_BRIGHTNESSCONTRAST_OID            0x02020302 //!< PMF_IE_BRIGHTNESSCONTRAST PseudoObject type.                 
#define U_PMF_IE_COLORBALANCE_OID                  0x02020303 //!< PMF_IE_COLORBALANCE PseudoObject type.                       
#define U_PMF_IE_COLORCURVE_OID                    0x02020304 //!< PMF_IE_COLORCURVE PseudoObject type.                         
#define U_PMF_IE_COLORLOOKUPTABLE_OID              0x02020305 //!< PMF_IE_COLORLOOKUPTABLE PseudoObject type.                   
#define U_PMF_IE_COLORMATRIX_OID                   0x02020306 //!< PMF_IE_COLORMATRIX PseudoObject type.                        
#define U_PMF_IE_HUESATURATIONLIGHTNESS_OID        0x02020307 //!< PMF_IE_HUESATURATIONLIGHTNESS PseudoObject type.             
#define U_PMF_IE_LEVELS_OID                        0x02020308 //!< PMF_IE_LEVELS PseudoObject type.                             
#define U_PMF_IE_REDEYECORRECTION_OID              0x02020309 //!< PMF_IE_REDEYECORRECTION PseudoObject type.                   
#define U_PMF_IE_SHARPEN_OID                       0x02020310 //!< PMF_IE_SHARPEN PseudoObject type.                           
#define U_PMF_IE_TINT_OID                          0x02020311 //!< PMF_IE_TINT PseudoObject type.                              
#define U_PMR_STROKEFILLPATH_OID                   0x02010101 //!< PMR_STROKEFILLPATH PseudoObject type. (Mentioned in passing here).      
#define U_PMR_OFFSETCLIP_OID                       0x02030101 //!< PMR_OFFSETCLIP PseudoObject type.                            
#define U_PMR_RESETCLIP_OID                        0x02030102 //!< PMR_RESETCLIP PseudoObject type.                             
#define U_PMR_SETCLIPPATH_OID                      0x02030103 //!< PMR_SETCLIPPATH PseudoObject type.                           
#define U_PMR_SETCLIPRECT_OID                      0x02030104 //!< PMR_SETCLIPRECT PseudoObject type.                           
#define U_PMR_SETCLIPREGION_OID                    0x02030105 //!< PMR_SETCLIPREGION PseudoObject type.                         
#define U_PMR_COMMENT_OID                          0x02030201 //!< PMR_COMMENT PseudoObject type.                               
#define U_PMR_ENDOFFILE_OID                        0x02030301 //!< PMR_ENDOFFILE PseudoObject type.                             
#define U_PMR_GETDC_OID                            0x02030302 //!< PMR_GETDC PseudoObject type.                                 
#define U_PMR_HEADER_OID                           0x02030303 //!< PMR_HEADER PseudoObject type.                                
#define U_PMR_CLEAR_OID                            0x02030401 //!< PMR_CLEAR PseudoObject type.                                 
#define U_PMR_DRAWARC_OID                          0x02030402 //!< PMR_DRAWARC PseudoObject type.                               
#define U_PMR_DRAWBEZIERS_OID                      0x02030403 //!< PMR_DRAWBEZIERS PseudoObject type.                           
#define U_PMR_DRAWCLOSEDCURVE_OID                  0x02030404 //!< PMR_DRAWCLOSEDCURVE PseudoObject type.                       
#define U_PMR_DRAWCURVE_OID                        0x02030405 //!< PMR_DRAWCURVE PseudoObject type.                             
#define U_PMR_DRAWDRIVERSTRING_OID                 0x02030406 //!< PMR_DRAWDRIVERSTRING PseudoObject type.                      
#define U_PMR_DRAWELLIPSE_OID                      0x02030407 //!< PMR_DRAWELLIPSE PseudoObject type.                           
#define U_PMR_DRAWIMAGE_OID                        0x02030408 //!< PMR_DRAWIMAGE PseudoObject type.                             
#define U_PMR_DRAWIMAGEPOINTS_OID                  0x02030409 //!< PMR_DRAWIMAGEPOINTS PseudoObject type.                       
#define U_PMR_DRAWLINES_OID                        0x02030410 //!< PMR_DRAWLINES PseudoObject type.                            
#define U_PMR_DRAWPATH_OID                         0x02030411 //!< PMR_DRAWPATH PseudoObject type.                             
#define U_PMR_DRAWPIE_OID                          0x02030412 //!< PMR_DRAWPIE PseudoObject type.                              
#define U_PMR_DRAWRECTS_OID                        0x02030413 //!< PMR_DRAWRECTS PseudoObject type.                            
#define U_PMR_DRAWSTRING_OID                       0x02030414 //!< PMR_DRAWSTRING PseudoObject type.                           
#define U_PMR_FILLCLOSEDCURVE_OID                  0x02030415 //!< PMR_FILLCLOSEDCURVE PseudoObject type.                      
#define U_PMR_FILLELLIPSE_OID                      0x02030416 //!< PMR_FILLELLIPSE PseudoObject type.                          
#define U_PMR_FILLPATH_OID                         0x02030417 //!< PMR_FILLPATH PseudoObject type.                             
#define U_PMR_FILLPIE_OID                          0x02030418 //!< PMR_FILLPIE PseudoObject type.                              
#define U_PMR_FILLPOLYGON_OID                      0x02030419 //!< PMR_FILLPOLYGON PseudoObject type.                          
#define U_PMR_FILLRECTS_OID                        0x02030420 //!< PMR_FILLRECTS PseudoObject type.                            
#define U_PMR_FILLREGION_OID                       0x02030421 //!< PMR_FILLREGION PseudoObject type.                           
#define U_PMR_OBJECT_OID                           0x02030501 //!< PMR_OBJECT PseudoObject type.                                
#define U_PMR_SERIALIZABLEOBJECT_OID               0x02030502 //!< PMR_SERIALIZABLEOBJECT PseudoObject type.                    
#define U_PMR_SETANTIALIASMODE_OID                 0x02030601 //!< PMR_SETANTIALIASMODE PseudoObject type.                      
#define U_PMR_SETCOMPOSITINGMODE_OID               0x02030602 //!< PMR_SETCOMPOSITINGMODE PseudoObject type.                    
#define U_PMR_SETCOMPOSITINGQUALITY_OID            0x02030603 //!< PMR_SETCOMPOSITINGQUALITY PseudoObject type.                 
#define U_PMR_SETINTERPOLATIONMODE_OID             0x02030604 //!< PMR_SETINTERPOLATIONMODE PseudoObject type.                  
#define U_PMR_SETPIXELOFFSETMODE_OID               0x02030605 //!< PMR_SETPIXELOFFSETMODE PseudoObject type.                    
#define U_PMR_SETRENDERINGORIGIN_OID               0x02030606 //!< PMR_SETRENDERINGORIGIN PseudoObject type.                    
#define U_PMR_SETTEXTCONTRAST_OID                  0x02030607 //!< PMR_SETTEXTCONTRAST PseudoObject type.                       
#define U_PMR_SETTEXTRENDERINGHINT_OID             0x02030608 //!< PMR_SETTEXTRENDERINGHINT PseudoObject type.                  
#define U_PMR_BEGINCONTAINER_OID                   0x02030701 //!< PMR_BEGINCONTAINER PseudoObject type.                        
#define U_PMR_BEGINCONTAINERNOPARAMS_OID           0x02030702 //!< PMR_BEGINCONTAINERNOPARAMS PseudoObject type.                
#define U_PMR_ENDCONTAINER_OID                     0x02030703 //!< PMR_ENDCONTAINER PseudoObject type.                          
#define U_PMR_RESTORE_OID                          0x02030704 //!< PMR_RESTORE PseudoObject type.                               
#define U_PMR_SAVE_OID                             0x02030705 //!< PMR_SAVE PseudoObject type.                                  
#define U_PMR_SETTSCLIP_OID                        0x02030801 //!< PMR_SETTSCLIP PseudoObject type.                             
#define U_PMR_SETTSGRAPHICS_OID                    0x02030802 //!< PMR_SETTSGRAPHICS PseudoObject type.                         
#define U_PMR_MULTIPLYWORLDTRANSFORM_OID           0x02030901 //!< PMR_MULTIPLYWORLDTRANSFORM PseudoObject type.                
#define U_PMR_RESETWORLDTRANSFORM_OID              0x02030902 //!< PMR_RESETWORLDTRANSFORM PseudoObject type.                   
#define U_PMR_ROTATEWORLDTRANSFORM_OID             0x02030903 //!< PMR_ROTATEWORLDTRANSFORM PseudoObject type.                  
#define U_PMR_SCALEWORLDTRANSFORM_OID              0x02030904 //!< PMR_SCALEWORLDTRANSFORM PseudoObject type.                   
#define U_PMR_SETPAGETRANSFORM_OID                 0x02030905 //!< PMR_SETPAGETRANSFORM PseudoObject type.                      
#define U_PMR_SETWORLDTRANSFORM_OID                0x02030906 //!< PMR_SETWORLDTRANSFORM PseudoObject type.                     
#define U_PMR_TRANSLATEWORLDTRANSFORM_OID          0x02030907 //!< PMR_TRANSLATEWORLDTRANSFORM PseudoObject type.               
#define U_PMR_TRANSLATEWORLDTRANSFORM_OID          0x02030907 //!< PMR_TRANSLATEWORLDTRANSFORM PseudoObject type.               
#define U_PMR_CMN_HDR_OID                          0x40000000 //!< PMR_CMN_HDR PseudoObject type.
#define U_PMF_4NUM_OID                             0x40000001 //!< PMF_4NUM PseudoObject type. PseudoObject contains a 4 unsigned int in EMF+ file byte order, used in some contexts to indicate an object index number..
#define U_PMF_RAW_OID                              0x40000002 //!< PMF_RAW PseudoObject type. Raw data: no preceding elements, data has native endianness.
#define U_PMF_ARRAY_OID                            0x80000000 //!< PMF_ARRAY PseudoObject type modifier. PseudoObject contains an array of the data type revealed when this bit is cleared.
#define U_PMF_MASK_OID                             0x7FFFFFFF //!< PMF_MASK.  Select PseudoObject data type without regard to PMF_ARRAY.

/** @} */


/** \defgroup U_PMF_BDT_ PMF BitmapDataType Enumeration
  For 
  EMF+ manual 2.1.1.2, Microsoft name: BitmapDataType Enumeration (U_BDT_*)
  @{
*/
#define   U_BDT_Pixel                   0x00 //!< Data is a bitmap.
#define   U_BDT_Compressed              0x01 //!< Data is a compressed bitmap (like a PNG).
/** @} */

/** \defgroup U_PMF_BT_ PMF BrushType Enumeration
  For 
  EMF+ manual 2.1.1.3, Microsoft name: BrushType Enumeration (U_BT_*) 
  @{
*/
#define   U_BT_SolidColor               0x00 //!< Solid Color brush.
#define   U_BT_HatchFill                0x01 //!< Hatch Fill brush.
#define   U_BT_TextureFill              0x02 //!< Texture Fill brush.
#define   U_BT_PathGradient             0x03 //!< Path Gradient brush.
#define   U_BT_LinearGradient           0x04 //!< Linear Gradient brush.
/** @} */ 

/** \defgroup U_PMF_CM_ PMF CombineMode Enumeration
  For 
  EMF+ manual 2.1.1.4, Microsoft name: CombineMode Enumeration  (U_CM_*)
  @{
*/
#define   U_CM_Replace                  0x00 //!< Region becomes new region.   
#define   U_CM_Intersect                0x01 //!< Region becomes intersection of existing region and new region.
#define   U_CM_Union                    0x02 //!< Region becomes union of existing and new regions.   
#define   U_CM_XOR                      0x03 //!< Region becomes XOR of existing and new regions.      
#define   U_CM_Exclude                  0x04 //!< Region becomes part of existing region not in new region.  
#define   U_CM_Complement               0x05 //!< Region becomes part of new region not in existing region.
/** @} */

/** \defgroup U_PMF_CMS_ PMF CompositingMode Enumeration
  For 
  EMF+ manual 2.1.1.5, Microsoft name: CompositingMode Enumeration (U_CMS_* [S==Source])
  @{
*/
#define   U_CMS_Over                    0x00 //!< Source is alpha blends with destination. 
#define   U_CMS_Copy                    0x01 //!< Source over writes destination.
/** @} */

/** \defgroup U_PMF_CQ_ PMF CompositingQuality Enumeration
  For 
  EMF+ manual 2.1.1.6, Microsoft name: CompositingQuality Enumeration (U_CQ_*)
  @{
*/
#define   U_CQ_Default                  0x01 //!< Default compositing quality
#define   U_CQ_HighSpeed                0x02 //!< High Speed compositing quality
#define   U_CQ_HighQuality              0x03 //!< High Quality compositing quality
#define   U_CQ_GammaCorrected           0x04 //!< Gamma Corrected compositing quality
#define   U_CQ_AssumeLinear             0x05 //!< Assume Linear compositing quality
/** @} */

/** \defgroup U_PMF_CA_ PMF CurveAdjustments Enumeration
  For 
  EMF+ manual 2.1.1.7, Microsoft name: CurveAdjustments Enumeration (U_CA_*)
  @{
*/
#define   U_CA_Exposure                 0x00 //!< Exposure color curve adjustment
#define   U_CA_Density                  0x01 //!< Density color curve adjustment
#define   U_CA_Contrast                 0x02 //!< Contrast color curve adjustment
#define   U_CA_Highlight                0x03 //!< Highlight color curve adjustment
#define   U_CA_Shadow                   0x04 //!< Shadow color curve adjustment
#define   U_CA_Midtone                  0x05 //!< Midtone color curve adjustment
#define   U_CA_WhiteSaturation          0x06 //!< White Saturation color curve adjustment
#define   U_CA_BlackSaturation          0x07 //!< Black Saturation color curve adjustment
/** @} */

/** \defgroup U_PMF_CC_ PMF CurveChannel Enumeration
  For 
  EMF+ manual 2.1.1.8, Microsoft name: CurveChannel Enumeration (U_CC_*)   
  @{
*/
#define   U_CC_All                      0x00 //!< All color channels
#define   U_CC_Red                      0x01 //!< Red color channel
#define   U_CC_Green                    0x02 //!< Green color channel
#define   U_CC_Blue                     0x03 //!< Blue color channel
/** @} */

/** \defgroup U_PMF_CLCDT_ PMF CustomLineCapDataType Enumeration
  For 
  EMF+ manual 2.1.1.9, Microsoft name: CustomLineCapDataType Enumeration (U_CLCDT_*)
  @{
*/
#define   U_CLCDT_Default               0x00 //!< Default custom line cap
#define   U_CLCDT_AdjustableArrow       0x01 //!< Adjustable Arrow custom line cap
/** @} */

/** \defgroup U_PMF_DLCT_ PMF DashedLineCapType Enumeration
  For 
  EMF+ manual 2.1.1.10, Microsoft name: DashedLineCapType Enumeration (U_DLCT_*)
  @{
*/
#define   U_DLCT_Flat                   0x00 //!< Flat dashed line cap
#define   U_DLCT_Round                  0x02 //!< Round dashed line cap
#define   U_DLCT_Triangle               0x03 //!< Triangle dashed line cap
/** @} */

/** \defgroup U_PMF_FT_ PMF FilterType Enumeration
  For 
  EMF+ manual 2.1.1.11, Microsoft name: FilterType Enumeration (U_FT_*)
  @{
*/
#define   U_FT_None                     0x00 //!< No filtering
#define   U_FT_Point                    0x01 //!< Point filtering
#define   U_FT_Linear                   0x02 //!< Linear filtering
#define   U_FT_Triangle                 0x03 //!< Triangle filtering
#define   U_FT_Box                      0x04 //!< Box filtering
#define   U_FT_PyramidalQuad            0x06 //!< Pyramidal Quad filtering
#define   U_FT_GaussianQuad             0x07 //!< Gaussian Quad filtering
/** @} */

/** \defgroup U_PMF_GV_ PMF GraphicsVersion Enumeration
  For 
  EMF+ manual 2.1.1.12, Microsoft name: GraphicsVersion Enumeration (U_GV_*)
  @{
*/
#define   U_GV_1                        0x01 //!< 1 graphics version
#define   U_GV_1_1                      0x02 //!< 1.1 graphics version
/** @} */

/** \defgroup U_PMF_HSP_ PMF HatchStyle Enumeration
  For 
  EMF+ manual 2.1.1.13, Microsoft name: HatchStyle Enumeration   (U_HSP_* [U_HS_ already used for EMF])
  @{
*/
#define   U_HSP_Horizontal              0x00000000 //!< Horizontal             
#define   U_HSP_Vertical                0x00000001 //!< Vertical               
#define   U_HSP_ForwardDiagonal         0x00000002 //!< Forward Diagonal        
#define   U_HSP_BackwardDiagonal        0x00000003 //!< Backward Diagonal       
#define   U_HSP_LargeGrid               0x00000004 //!< Large Grid              
#define   U_HSP_DiagonalCross           0x00000005 //!< Diagonal Cross          
#define   U_HSP_05Percent               0x00000006 //!< 05 Percent             
#define   U_HSP_10Percent               0x00000007 //!< 10 Percent             
#define   U_HSP_20Percent               0x00000008 //!< 20 Percent             
#define   U_HSP_25Percent               0x00000009 //!< 25 Percent             
#define   U_HSP_30Percent               0x0000000A //!< 30 Percent             
#define   U_HSP_40Percent               0x0000000B //!< 40 Percent             
#define   U_HSP_50Percent               0x0000000C //!< 50 Percent             
#define   U_HSP_60Percent               0x0000000D //!< 60 Percent             
#define   U_HSP_70Percent               0x0000000E //!< 70 Percent             
#define   U_HSP_75Percent               0x0000000F //!< 75 Percent             
#define   U_HSP_80Percent               0x00000010 //!< 80 Percent             
#define   U_HSP_90Percent               0x00000011 //!< 90 Percent             
#define   U_HSP_LightDownwardDiagonal   0x00000012 //!< Light Downward Diagonal  
#define   U_HSP_LightUpwardDiagonal     0x00000013 //!< Light Upward Diagonal    
#define   U_HSP_DarkDownwardDiagonal    0x00000014 //!< Dark Downward Diagonal  
#define   U_HSP_DarkUpwardDiagonal      0x00000015 //!< Dark Upward Diagonal    
#define   U_HSP_WideDownwardDiagonal    0x00000016 //!< Wide Downward Diagonal   
#define   U_HSP_WideUpwardDiagonal      0x00000017 //!< Wide Upward Diagonal     
#define   U_HSP_LightVertical           0x00000018 //!< Light Vertical          
#define   U_HSP_LightHorizontal         0x00000019 //!< Light Horizontal        
#define   U_HSP_NarrowVertical          0x0000001A //!< Narrow Vertical         
#define   U_HSP_NarrowHorizontal        0x0000001B //!< Narrow Horizontal       
#define   U_HSP_DarkVertical            0x0000001C //!< Dark Vertical           
#define   U_HSP_DarkHorizontal          0x0000001D //!< Dark Horizontal         
#define   U_HSP_DashedDownwardDiagonal  0x0000001E //!< Dashed Downward Diagonal 
#define   U_HSP_DashedUpwardDiagonal    0x0000001F //!< Dashed Upward Diagonal   
#define   U_HSP_DashedHorizontal        0x00000020 //!< Dashed Horizontal       
#define   U_HSP_DashedVertical          0x00000021 //!< Dashed Vertical         
#define   U_HSP_SmallConfetti           0x00000022 //!< Small Confetti          
#define   U_HSP_LargeConfetti           0x00000023 //!< LargeC onfetti          
#define   U_HSP_ZigZag                  0x00000024 //!< Zig Zag                 
#define   U_HSP_Wave                    0x00000025 //!< Wave                   
#define   U_HSP_DiagonalBrick           0x00000026 //!< Diagonal Brick          
#define   U_HSP_HorizontalBrick         0x00000027 //!< Horizontal Brick        
#define   U_HSP_Weave                   0x00000028 //!< Weave                  
#define   U_HSP_Plaid                   0x00000029 //!< Plaid                  
#define   U_HSP_Divot                   0x0000002A //!< Divot                  
#define   U_HSP_DottedGrid              0x0000002B //!< DottedGrid             
#define   U_HSP_DottedDiamond           0x0000002C //!< DottedDiamond          
#define   U_HSP_Shingle                 0x0000002D //!< Shingle                
#define   U_HSP_Trellis                 0x0000002E //!< Trellis                
#define   U_HSP_Sphere                  0x0000002F //!< Sphere                 
#define   U_HSP_SmallGrid               0x00000030 //!< Small Grid              
#define   U_HSP_SmallCheckerBoard       0x00000031 //!< Small Checker Board      
#define   U_HSP_LargeCheckerBoard       0x00000032 //!< Large Checker Board      
#define   U_HSP_OutlinedDiamond         0x00000033 //!< Outlined Diamond        
#define   U_HSP_SolidDiamond            0x00000034 //!< Solid Diamond           
/** @} */

/** \defgroup U_PMF_HKP_ PMF HotkeyPrefix Enumeration
  For 
  EMF+ manual 2.1.1.14, Microsoft name: HotkeyPrefix Enumeration (U_HKP_*)
  @{
*/
#define   U_HKP_None                    0x00 //!< No hot key prefix
#define   U_HKP_Show                    0x01 //!< Show hot key prefix
#define   U_HKP_Hide                    0x02 //!< Hide hot key prefix
/** @} */

/** \defgroup U_PMF_IDT_ PMF ImageDataType Enumeration
  For 
  EMF+ manual 2.1.1.15, Microsoft name: ImageDataType Enumeration (U_IDT_*)
  @{
*/
#define   U_IDT_Unknown                 0x00 //!< Unknown image data type 
#define   U_IDT_Bitmap                  0x01 //!< Bitmap image data type
#define   U_IDT_Metafile                0x02 //!< Metafile image data type
/** @} */

/** \defgroup U_PMF_IM_ PMF InterpolationMode Enumeration
  For 
  EMF+ manual 2.1.1.16, Microsoft name: InterpolationMode Enumeration (U_IM_*)
  @{
*/
#define   U_IM_Default                  0x00 //!< Default interpolation mode           
#define   U_IM_LowQuality               0x01 //!< Low Quality interpolation mode
#define   U_IM_HighQuality              0x02 //!< High Quality interpolation mode
#define   U_IM_Bilinear                 0x03 //!< Bilinear interpolation mode
#define   U_IM_Bicubic                  0x04 //!< Bicubic interpolation mode
#define   U_IM_NearestNeighbor          0x05 //!< Nearest Neighbor interpolation mode
#define   U_IM_HighQualityBilinear      0x06 //!< High Quality Bilinear interpolation mode
#define   U_IM_HighQualityBicubic       0x07 //!< High Quality Bicubic interpolation mode
/** @} */

/** \defgroup U_PMF_LID_ PMF LanguageIdentifier Enumeration
  For 
  EMF+ manual 2.1.1.17, Microsoft name: LanguageIdentifier Enumeration (U_LID_*)
  @{
*/
#define   U_LID_LANG_NEUTRAL                      0x0000 //!< LANG_NEUTRAL
#define   U_LID_zh_CHS                            0x0004 //!< zh_CHS
#define   U_LID_LANG_INVARIANT                    0x007F //!< LANG_INVARIANT
#define   U_LID_LANG_NEUTRAL_USER_DEFAULT         0x0400 //!< LANG_NEUTRAL_USER_DEFAULT
#define   U_LID_ar_SA                             0x0401 //!< ar_SA
#define   U_LID_bg_BG                             0x0402 //!< bg_BG
#define   U_LID_ca_ES                             0x0403 //!< ca_ES
#define   U_LID_zh_CHT                            0x0404 //!< zh_CHT
#define   U_LID_cs_CZ                             0x0405 //!< cs_CZ
#define   U_LID_da_DK                             0x0406 //!< da_DK
#define   U_LID_de_DE                             0x0407 //!< de_DE
#define   U_LID_el_GR                             0x0408 //!< el_GR
#define   U_LID_en_US                             0x0409 //!< en_US
#define   U_LID_es_Tradnl_ES                      0x040A //!< es_Tradnl_ES
#define   U_LID_fi_FI                             0x040B //!< fi_FI
#define   U_LID_fr_FR                             0x040C //!< fr_FR
#define   U_LID_he_IL                             0x040D //!< he_IL
#define   U_LID_hu_HU                             0x040E //!< hu_HU
#define   U_LID_is_IS                             0x040F //!< is_IS
#define   U_LID_it_IT                             0x0410 //!< it_IT
#define   U_LID_ja_JA                             0x0411 //!< ja_JA
#define   U_LID_ko_KR                             0x0412 //!< ko_KR
#define   U_LID_nl_NL                             0x0413 //!< nl_NL
#define   U_LID_nb_NO                             0x0414 //!< nb_NO
#define   U_LID_pl_PL                             0x0415 //!< pl_PL
#define   U_LID_pt_BR                             0x0416 //!< pt_BR
#define   U_LID_rm_CH                             0x0417 //!< rm_CH
#define   U_LID_ro_RO                             0x0418 //!< ro_RO
#define   U_LID_ru_RU                             0x0419 //!< ru_RU
#define   U_LID_hr_HR                             0x041A //!< hr_HR
#define   U_LID_sk_SK                             0x041B //!< sk_SK
#define   U_LID_sq_AL                             0x041C //!< sq_AL
#define   U_LID_sv_SE                             0x041D //!< sv_SE
#define   U_LID_th_TH                             0x041E //!< th_TH
#define   U_LID_tr_TR                             0x041F //!< tr_TR
#define   U_LID_ur_PK                             0x0420 //!< ur_PK
#define   U_LID_id_ID                             0x0421 //!< id_ID
#define   U_LID_uk_UA                             0x0422 //!< uk_UA
#define   U_LID_be_BY                             0x0423 //!< be_BY
#define   U_LID_sl_SI                             0x0424 //!< sl_SI
#define   U_LID_et_EE                             0x0425 //!< et_EE
#define   U_LID_lv_LV                             0x0426 //!< lv_LV
#define   U_LID_lt_LT                             0x0427 //!< lt_LT
#define   U_LID_tg_TJ                             0x0428 //!< tg_TJ
#define   U_LID_fa_IR                             0x0429 //!< fa_IR
#define   U_LID_vi_VN                             0x042A //!< vi_VN
#define   U_LID_hy_AM                             0x042B //!< hy_AM
#define   U_LID_az_Latn_AZ                        0x042C //!< az_Latn_AZ
#define   U_LID_eu_ES                             0x042D //!< eu_ES
#define   U_LID_wen_DE                            0x042E //!< wen_DE
#define   U_LID_mk_MK                             0x042F //!< mk_MK
#define   U_LID_st_ZA                             0x0430 //!< st_ZA
#define   U_LID_tn_ZA                             0x0432 //!< tn_ZA
#define   U_LID_xh_ZA                             0x0434 //!< xh_ZA
#define   U_LID_zu_ZA                             0x0435 //!< zu_ZA
#define   U_LID_af_ZA                             0x0436 //!< af_ZA
#define   U_LID_ka_GE                             0x0437 //!< ka_GE
#define   U_LID_fa_FA                             0x0438 //!< fa_FA
#define   U_LID_hi_IN                             0x0439 //!< hi_IN
#define   U_LID_mt_MT                             0x043A //!< mt_MT
#define   U_LID_se_NO                             0x043B //!< se_NO
#define   U_LID_ga_GB                             0x043C //!< ga_GB
#define   U_LID_ms_MY                             0x043E //!< ms_MY
#define   U_LID_kk_KZ                             0x043F //!< kk_KZ
#define   U_LID_ky_KG                             0x0440 //!< ky_KG
#define   U_LID_sw_KE                             0x0441 //!< sw_KE
#define   U_LID_tk_TM                             0x0442 //!< tk_TM
#define   U_LID_uz_Latn_UZ                        0x0443 //!< uz_Latn_UZ
#define   U_LID_tt_Ru                             0x0444 //!< tt_Ru
#define   U_LID_bn_IN                             0x0445 //!< bn_IN
#define   U_LID_pa_IN                             0x0446 //!< pa_IN
#define   U_LID_gu_IN                             0x0447 //!< gu_IN
#define   U_LID_or_IN                             0x0448 //!< or_IN
#define   U_LID_ta_IN                             0x0449 //!< ta_IN
#define   U_LID_te_IN                             0x044A //!< te_IN
#define   U_LID_kn_IN                             0x044B //!< kn_IN
#define   U_LID_ml_IN                             0x044C //!< ml_IN
#define   U_LID_as_IN                             0x044D //!< as_IN
#define   U_LID_mr_IN                             0x044E //!< mr_IN
#define   U_LID_sa_IN                             0x044F //!< sa_IN
#define   U_LID_mn_MN                             0x0450 //!< mn_MN
#define   U_LID_bo_CN                             0x0451 //!< bo_CN
#define   U_LID_cy_GB                             0x0452 //!< cy_GB
#define   U_LID_km_KH                             0x0453 //!< km_KH
#define   U_LID_lo_LA                             0x0454 //!< lo_LA
#define   U_LID_gl_ES                             0x0456 //!< gl_ES
#define   U_LID_kok_IN                            0x0457 //!< kok_IN
#define   U_LID_sd_IN                             0x0459 //!< sd_IN
#define   U_LID_syr_SY                            0x045A //!< syr_SY
#define   U_LID_si_LK                             0x045B //!< si_LK
#define   U_LID_iu_Cans_CA                        0x045D //!< iu_Cans_CA
#define   U_LID_am_ET                             0x045E //!< am_ET
#define   U_LID_ne_NP                             0x0461 //!< ne_NP
#define   U_LID_fy_NL                             0x0462 //!< fy_NL
#define   U_LID_ps_AF                             0x0463 //!< ps_AF
#define   U_LID_fil_PH                            0x0464 //!< fil_PH
#define   U_LID_div_MV                            0x0465 //!< div_MV
#define   U_LID_ha_Latn_NG                        0x0468 //!< ha_Latn_NG
#define   U_LID_yo_NG                             0x046A //!< yo_NG
#define   U_LID_quz_BO                            0x046B //!< quz_BO
#define   U_LID_nzo_ZA                            0x046C //!< nzo_ZA
#define   U_LID_ba_RU                             0x046D //!< ba_RU
#define   U_LID_lb_LU                             0x046E //!< lb_LU
#define   U_LID_kl_GL                             0x046F //!< kl_GL
#define   U_LID_ig_NG                             0x0470 //!< ig_NG
#define   U_LID_so_SO                             0x0477 //!< so_SO
#define   U_LID_ii_CN                             0x0478 //!< ii_CN
#define   U_LID_arn_CL                            0x047A //!< arn_CL
#define   U_LID_moh_CA                            0x047C //!< moh_CA
#define   U_LID_br_FR                             0x047E //!< br_FR
#define   U_LID_ug_CN                             0x0480 //!< ug_CN
#define   U_LID_ mi_NZ                            0x0481 //!<  mi_NZ
#define   U_LID_oc_FR                             0x0482 //!< oc_FR
#define   U_LID_co_FR                             0x0483 //!< co_FR
#define   U_LID_gsw_FR                            0x0484 //!< gsw_FR
#define   U_LID_sah_RU                            0x0485 //!< sah_RU
#define   U_LID_qut_GT                            0x0486 //!< qut_GT
#define   U_LID_rw_RW                             0x0487 //!< rw_RW
#define   U_LID_wo_SN                             0x0488 //!< wo_SN
#define   U_LID_gbz_AF                            0x048C //!< gbz_AF
#define   U_LID_LANG_NEUTRAL_SYS_DEFAULT          0x0800 //!< LANG_NEUTRAL_SYS_DEFAULT
#define   U_LID_ar_IQ                             0x0801 //!< ar_IQ
#define   U_LID_zh_CN                             0x0804 //!< zh_CN
#define   U_LID_de_CH                             0x0807 //!< de_CH
#define   U_LID_en_GB                             0x0809 //!< en_GB
#define   U_LID_es_MX                             0x080A //!< es_MX
#define   U_LID_fr_BE                             0x080C //!< fr_BE
#define   U_LID_it_CH                             0x0810 //!< it_CH
#define   U_LID_ko_Johab_KR                       0x0812 //!< ko_Johab_KR
#define   U_LID_nl_BE                             0x0813 //!< nl_BE
#define   U_LID_nn_NO                             0x0814 //!< nn_NO
#define   U_LID_pt_PT                             0x0816 //!< pt_PT
#define   U_LID_sr_Latn_SP                        0x081A //!< sr_Latn_SP
#define   U_LID_sv_FI                             0x081D //!< sv_FI
#define   U_LID_ur_IN                             0x0820 //!< ur_IN
#define   U_LID_lt_C_LT                           0x0827 //!< lt_C_LT
#define   U_LID_az_Cyrl_AZ                        0x082C //!< az_Cyrl_AZ
#define   U_LID_wee_DE                            0x082E //!< wee_DE
#define   U_LID_se_SE                             0x083B //!< se_SE
#define   U_LID_ga_IE                             0x083C //!< ga_IE
#define   U_LID_ms_BN                             0x083E //!< ms_BN
#define   U_LID_uz_Cyrl_UZ                        0x0843 //!< uz_Cyrl_UZ
#define   U_LID_bn_BD                             0x0845 //!< bn_BD
#define   U_LID_mn_Mong_CN                        0x0850 //!< mn_Mong_CN
#define   U_LID_sd_PK                             0x0859 //!< sd_PK
#define   U_LID_iu_Latn_CA                        0x085D //!< iu_Latn_CA
#define   U_LID_tzm_Latn_DZ                       0x085F //!< tzm_Latn_DZ
#define   U_LID_quz_EC                            0x086B //!< quz_EC
#define   U_LID_LANG_NEUTRAL_CUSTOM_DEFAULT       0x0C00 //!< LANG_NEUTRAL_CUSTOM_DEFAULT
#define   U_LID_ar_EG                             0x0C01 //!< ar_EG
#define   U_LID_zh_HK                             0x0C04 //!< zh_HK
#define   U_LID_de_AT                             0x0C07 //!< de_AT
#define   U_LID_en_AU                             0x0C09 //!< en_AU
#define   U_LID_es_ES                             0x0C0A //!< es_ES
#define   U_LID_fr_CA                             0x0C0C //!< fr_CA
#define   U_LID_sr_Cyrl_CS                        0x0C1A //!< sr_Cyrl_CS
#define   U_LID_se_FI                             0x0C3B //!< se_FI
#define   U_LID_quz_PE                            0x0C6B //!< quz_PE
#define   U_LID_LANG_NEUTRAL_CUSTOM               0x1000 //!< LANG_NEUTRAL_CUSTOM
#define   U_LID_ar_LY                             0x1001 //!< ar_LY
#define   U_LID_zh_SG                             0x1004 //!< zh_SG
#define   U_LID_de_LU                             0x1007 //!< de_LU
#define   U_LID_en_CA                             0x1009 //!< en_CA
#define   U_LID_es_GT                             0x100A //!< es_GT
#define   U_LID_fr_CH                             0x100C //!< fr_CH
#define   U_LID_hr_BA                             0x101A //!< hr_BA
#define   U_LID_smj_NO                            0x103B //!< smj_NO
#define   U_LID_LANG_NEUTRAL_CUSTOM_DEFAULT_MUI   0x1400 //!< LANG_NEUTRAL_CUSTOM_DEFAULT_MUI
#define   U_LID_ar_DZ                             0x1401 //!< ar_DZ
#define   U_LID_zh_MO                             0x1404 //!< zh_MO
#define   U_LID_de_LI                             0x1407 //!< de_LI
#define   U_LID_en_NZ                             0x1409 //!< en_NZ
#define   U_LID_es_CR                             0x140A //!< es_CR
#define   U_LID_fr_LU                             0x140C //!< fr_LU
#define   U_LID_bs_Latn_BA                        0x141A //!< bs_Latn_BA
#define   U_LID_smj_SE                            0x143B //!< smj_SE
#define   U_LID_ar_MA                             0x1801 //!< ar_MA
#define   U_LID_en_IE                             0x1809 //!< en_IE
#define   U_LID_es_PA                             0x180A //!< es_PA
#define   U_LID_ar_MC                             0x180C //!< ar_MC
#define   U_LID_sr_Latn_BA                        0x181A //!< sr_Latn_BA
#define   U_LID_sma_NO                            0x183B //!< sma_NO
#define   U_LID_ar_TN                             0x1C01 //!< ar_TN
#define   U_LID_en_ZA                             0x1C09 //!< en_ZA
#define   U_LID_es_DO                             0x1C0A //!< es_DO
#define   U_LID_sr_Cyrl_BA                        0x1C1A //!< sr_Cyrl_BA
#define   U_LID_sma_SE                            0x1C3B //!< sma_SE
#define   U_LID_ar_OM                             0x2001 //!< ar_OM
#define   U_LID_el_2_GR                           0x2008 //!< el_2_GR
#define   U_LID_en_JM                             0x2009 //!< en_JM
#define   U_LID_es_VE                             0x200A //!< es_VE
#define   U_LID_bs_Cyrl_BA                        0x201A //!< bs_Cyrl_BA
#define   U_LID_sms_FI                            0x203B //!< sms_FI
#define   U_LID_ar_YE                             0x2401 //!< ar_YE
#define   U_LID_ar_029                            0x2409 //!< ar_029
#define   U_LID_es_CO                             0x240A //!< es_CO
#define   U_LID_smn_FI                            0x243B //!< smn_FI
#define   U_LID_ar_SY                             0x2801 //!< ar_SY
#define   U_LID_en_BZ                             0x2809 //!< en_BZ
#define   U_LID_es_PE                             0x280A //!< es_PE
#define   U_LID_ar_JO                             0x2C01 //!< ar_JO
#define   U_LID_en_TT                             0x2C09 //!< en_TT
#define   U_LID_es_AR                             0x2C0A //!< es_AR
#define   U_LID_ar_LB                             0x3001 //!< ar_LB
#define   U_LID_en_ZW                             0x3009 //!< en_ZW
#define   U_LID_es_EC                             0x300A //!< es_EC
#define   U_LID_ar_KW                             0x3401 //!< ar_KW
#define   U_LID_en_PH                             0x3409 //!< en_PH
#define   U_LID_es_CL                             0x340A //!< es_CL
#define   U_LID_ar_AE                             0x3801 //!< ar_AE
#define   U_LID_es_UY                             0x380A //!< es_UY
#define   U_LID_ar_BH                             0x3C01 //!< ar_BH
#define   U_LID_es_PY                             0x3C0A //!< es_PY
#define   U_LID_ar_QA                             0x4001 //!< ar_QA
#define   U_LID_en_IN                             0x4009 //!< en_IN
#define   U_LID_es_BO                             0x400A //!< es_BO
#define   U_LID_en_MY                             0x4409 //!< en_MY
#define   U_LID_es_SV                             0x440A //!< es_SV
#define   U_LID_en_SG                             0x4809 //!< en_SG
#define   U_LID_es_HN                             0x480A //!< es_HN
#define   U_LID_es_NI                             0x4C0A //!< es_NI
#define   U_LID_es_PR                             0x500A //!< es_PR
#define   U_LID_es_US                             0x540A //!< es_US
#define   U_LID_zh_Hant                           0x7C04 //!< zh_Hant
#define   U_LID_SEC_MASK                          0xFB00 //!< Mask for region  part of LID               
#define   U_LID_PRI_MASK                          0x03FF //!< MASK for languagepart of LID                                  
/** @} */

/** \defgroup U_PMF_LCT_ PMF LineCapType Enumeration
  For 
  EMF+ manual 2.1.1.18, Microsoft name: LineCapType Enumeration (U_LCT_*)
  @{
*/
#define   U_LCT_Flat                    0x00 //!< Flat line cap
#define   U_LCT_Square                  0x01 //!< Square line cap
#define   U_LCT_Round                   0x02 //!< Round line cap
#define   U_LCT_Triangle                0x03 //!< Triangle line cap
#define   U_LCT_NoAnchor                0x10 //!< No Anchor line cap
#define   U_LCT_SquareAnchor            0x11 //!< Square Anchor line cap
#define   U_LCT_RoundAnchor             0x12 //!< Round Anchor line cap
#define   U_LCT_DiamondAnchor           0x13 //!< Diamond Anchor line cap
#define   U_LCT_ArrowAnchor             0x14 //!< Arrow Anchor line cap
#define   U_LCT_AnchorMask              0xF0 //!< Ancho rMask line cap
#define   U_LCT_Custom                  0xFF //!< Custom line cap
/** @} */

/** \defgroup U_PMF_LJT_ PMF LineJoinType Enumeration
  For 
  EMF+ manual 2.1.1.19, Microsoft name: LineJoinType Enumeration (U_LJT_*)
  @{
*/
#define   U_LJT_Miter                   0x00 //!< Miter line join      
#define   U_LJT_Bevel                   0x01 //!< Bevel line join
#define   U_LJT_Round                   0x02 //!< Round line join
#define   U_LJT_MiterClipped            0x03 //!< Miter Clipped line join
/** @} */

/** \defgroup U_PMF_LS_ PMF LineStyle Enumeration
  For 
  EMF+ manual 2.1.1.20, Microsoft name: LineStyle Enumeration (U_LS_*)
  @{
*/
#define   U_LS_Solid                    0x00 //!< Solid line  
#define   U_LS_Dash                     0x01 //!< Dashed line  
#define   U_LS_Dot                      0x02 //!< Dotted line     
#define   U_LS_DashDot                  0x03 //!< Dash Dot line  
#define   U_LS_DashDotDot               0x04 //!< Dash Dot Dot line
#define   U_LS_Custom                   0x05 //!< Custom line  
/** @} */

/** \defgroup U_PMF_MDT_ PMF MetafileDataType Enumeration
  For 
  EMF+ manual 2.1.1.21, Microsoft name: MetafileDataType Enumeration (U_MDT_*)
  @{
*/
#define   U_MDT_Wmf                     0x01 //!< WMF metafile       
#define   U_MDT_WmfPlaceable            0x02 //!< WMF placeable metafile
#define   U_MDT_Emf                     0x03 //!< EMF metafile
#define   U_MDT_EmfPlusOnly             0x04 //!< EMF+ single mode metafile
#define   U_MDT_EmfPlusDual             0x05 //!< EMF+ dual mode metafile
/** @} */ 

/** \defgroup U_PMF_OT_ PMF ObjectType Enumeration
  For 
  EMF+ manual 2.1.1.22, Microsoft name: ObjectType Enumeration (U_OT_*)
  @{
*/
#define   U_OT_Invalid                  0x00 //!< Invalid object       
#define   U_OT_Brush                    0x01 //!< Brush object    
#define   U_OT_Pen                      0x02 //!< Pen object   
#define   U_OT_Path                     0x03 //!< Path object 
#define   U_OT_Region                   0x04 //!< Region object 
#define   U_OT_Image                    0x05 //!< Image object 
#define   U_OT_Font                     0x06 //!< Font object 
#define   U_OT_StringFormat             0x07 //!< StringFormat object 
#define   U_OT_ImageAttributes          0x08 //!< ImageAttributes object 
#define   U_OT_CustomLineCap            0x09 //!< CustomLineCap object 
/** @} */

/** \defgroup U_PMF_PPT_ PMF PathPointType Enumeration
  For 
  EMF+ manual 2.1.1.23, Microsoft name: PathPointType Enumeration (U_PPT_*)
  @{
*/
#define   U_PPT_Start                   0x00 //!< Start of path
#define   U_PPT_Line                    0x01 //!< Line path
#define   U_PPT_Bezier                  0x03 //!< Bezier path
#define   U_PPT_MASK                    0x0F //!< MASK for bits in flag
/** @} */

/** \defgroup U_PMF_PA_ PMF PenAlignment Enumeration
  For 
  EMF+ manual 2.1.1.24, Microsoft name: PenAlignment Enumeration (U_PA_*)
  @{
*/
#define   U_PA_Center                   0x00 //!< Center pen alignment
#define   U_PA_Inset                    0x01 //!< Inset pen alignment
#define   U_PA_Left                     0x02 //!< Left pen alignment
#define   U_PA_Outset                   0x03 //!< Outset pen alignment
#define   U_PA_Right                    0x04 //!< Right pen alignment
/** @} */

/** \defgroup U_PMF_PF_ PMF PixelFormat Enumeration
  For U_PMF_BITMAP PxFormat field
  EMF+ manual 2.1.1.25, Microsoft name: PixelFormat Enumeration (U_PF_*)
  
        Bitmap for this 32 bit value is:
        0-9 ignored
        10  Set: 32 bit ARGB; Clear: !32 bit ARGB
        11  Set: 16 bits/channel; Clear: !16 bits
        12  Set: colors premultiplied by alpha; Clear: !premultiplied
        13  Set: has Alpha; Clear: !has Alpha
        14  Set: Windows GDI supports; Clear: !Windows GDI supports
        15  Set: uses LUT; Clear  !uses LUT
        16-23   = total number of BITS per pixel
        24-31   = pixel format enumeration index (0->15)
  @{
*/
#define   U_PF_Undefined                0x00000000 //!<  undefined Pixel Format
#define   U_PF_1bppIndexed              0x00030101 //!<  monochrome with LUT
#define   U_PF_4bppIndexed              0x00030402 //!<  4 bit with LUT
#define   U_PF_8bppIndexed              0x00030803 //!<  8 bit with LUT
#define   U_PF_16bppGrayScale           0x00101004 //!<  16 bits grey values
#define   U_PF_16bppRGB555              0x00021005 //!<  16 bit RGB values (5,5,5,(1 ignored))
#define   U_PF_16bppRGB565              0x00021006 //!<  16 bit RGB values (5,6,5)
#define   U_PF_16bppARGB1555            0x00061007 //!<  16 bit ARGB values (1 alpha, 5,5,5 colors)
#define   U_PF_24bppRGB                 0x00021808 //!<  24 bit RGB values (8,8.8)
#define   U_PF_32bppRGB                 0x00022009 //!<  32 bit RGB value  (8,8,8,(8 ignored))
#define   U_PF_32bppARGB                0x0026200A //!<  32 bit ARGB values (8 alpha,8,8,8)
#define   U_PF_32bppPARGB               0x000E200B //!<  32 bit PARGB values (8,8,8,8, but RGB already multiplied by A)
#define   U_PF_48bppRGB                 0x0010300C //!<  48 bit RGB (16,16,16)
#define   U_PF_64bppARGB                0x0034400D //!<  64 bit ARGB (16 alpha, 16,16,16)
#define   U_PF_64bppPARGB               0x001A400E //!<  64 bit PARGB (16,16,16,16, but RGB already multiplied by A)
/** @} */

/** \defgroup U_PMF_POM_ PMF PixelOffsetMode Enumeration
  For 
  EMF+ manual 2.1.1.26, Microsoft name: PixelOffsetMode Enumeration (U_POM_*)
  @{
*/
#define   U_POM_Default                 0x00       //!<  center at {0.0,0.0}
#define   U_POM_HighSpeed               0x01       //!<  center at {0.0,0.0}
#define   U_POM_HighQuality             0x02       //!<  center at {0.5,0.5}
#define   U_POM_None                    0x03       //!<  center at {0.0,0.0}
#define   U_POM_Half                    0x04       //!<  center at {0.5,0.5}
/** @} */

/** \defgroup U_PMF_RNDT_ PMF RegionNodeDataType Enumeration
  For 
  EMF+ manual 2.1.1.27, Microsoft name: RegionNodeDataType Enumeration (U_RNDT_*)
  @{
*/
#define   U_RNDT_Kids                   0x00000000 //!< One of the next 5 is to be applied
#define   U_RNDT_And                    0x00000001 //!< AND the child nodes
#define   U_RNDT_Or                     0x00000002 //!< OR the child nodes
#define   U_RNDT_Xor                    0x00000003 //!< XOR the child nodes
#define   U_RNDT_Exclude                0x00000004 //!< Part of 1st child node not in 2nd child node
#define   U_RNDT_Complement             0x00000005 //!< Part of 2nd child node not in 1st child node
#define   U_RNDT_Rect                   0x10000000 //!< Child node is a rectangle
#define   U_RNDT_Path                   0x10000001 //!< Child node is a path
#define   U_RNDT_Empty                  0x10000002 //!< Child node is empty
#define   U_RNDT_Infinite               0x10000003 //!< Child node has infinite extent (?)
/** @} */

/** \defgroup U_PMF_SM_ PMF SmoothingMode Enumeration
  For 
  EMF+ manual 2.1.1.28, Microsoft name: SmoothingMode Enumeration (U_SM_*)
  @{
*/
#define   U_SM_Default                  0x00 //!< Default smoothing
#define   U_SM_HighSpeed                0x01 //!< High Speed smoothing
#define   U_SM_HighQuality              0x02 //!< High Quality  smoothing
#define   U_SM_None                     0x03 //!< No smoothing
#define   U_SM_AntiAlias8x4             0x04 //!< Anti Alias 8x4 smoothing
#define   U_SM_AntiAlias8x8             0x05 //!< Anti Alias 8x8 smoothing
/** @} */

/** \defgroup U_PMF_SA_ PMF StringAlignment Enumeration
  For 
  EMF+ manual 2.1.1.29, Microsoft name: StringAlignment Enumeration (U_SA_*)
  
  Note, that unlike EMF these are with respect to the bounding rectangle, not to a single point. So
  to draw centered text, for instance, U_SA_Center must be used, and the bounding rectangle must also be
  centered.
  
  For horizontal positioning of L->R text Near is all the way left in the box, Far is all the way right,
  and Center puts the center of the text in the center of the box.
  
  For vertical positioning things are a little strange.  Near is a certain distance down from the top, Far is a
  certain distance up from the bottom, and center puts the center of the text in the center of the box.  The
  "certain distance" is not specified in the EMF+ documentation. See the function U_PMR_drawstring() for an 
  implementation that places text on the baseline.
  @{
*/
#define   U_SA_Near                     0x00 //!< Position near
#define   U_SA_Center                   0x01 //!< Position center
#define   U_SA_Far                      0x02 //!< Position far
/** @} */

/** \defgroup U_PMF_SDS_ PMF StringDigitSubstitution Enumeration
  For 
  EMF+ manual 2.1.1.30, Microsoft name: StringDigitSubstitution Enumeration (U_SDS_*)
  @{
*/
#define   U_SDS_User                    0x00 //!< Digit substitution is set by implementation
#define   U_SDS_None                    0x01 //!< No Digit substitution
#define   U_SDS_National                0x02 //!< Digit substitution by official locale
#define   U_SDS_Traditional             0x03 //!< Digit substitution by traditional locale
/** @} */

/** \defgroup U_PMF_ST_ PMF StringTrimming Enumeration
  For 
  EMF+ manual 2.1.1.31, Microsoft name: StringTrimming Enumeration (U_ST_*)
  @{
*/
#define   U_ST_None                     0x00 //!< no string trimming
#define   U_ST_Character                0x01 //!< Trim at Character        
#define   U_ST_Word                     0x02 //!< Trim at Word             
#define   U_ST_EllipsisCharacter        0x03 //!< Trim at Ellipsis Character
#define   U_ST_EllipsisWord             0x04 //!< Trim at Ellipsis Word     
#define   U_ST_EllipsisPath             0x05 //!< Trim at Ellipsis Path     
/** @} */

/** \defgroup U_PMF_TRH_ PMF TextRenderingHint Enumeration
  For 
  EMF+ manual 2.1.1.32, Microsoft name: TextRenderingHint Enumeration (U_TRH_*)
  @{
*/
#define   U_TRH_SystemDefault                0x00 //!< System Default           
#define   U_TRH_SingleBitPerPixelGridFit     0x01 //!< Single Bit Per Pixel Grid Fit
#define   U_TRH_SingleBitPerPixel            0x02 //!< Single Bit Per Pixel       
#define   U_TRH_AntialiasGridFit             0x03 //!< Antialias Grid Fit        
#define   U_TRH_Antialias                    0x04 //!< Antialias               
#define   U_TRH_ClearTypeGridFit             0x05 //!< ClearType Grid Fit        
/** @} */

/** \defgroup U_PMF_UT_ PMF UnitType Enumeration
  For 
  EMF+ manual 2.1.1.33, Microsoft name: UnitType Enumeration (U_UT_*)
  @{
*/
#define   U_UT_World                    0x00 //!< World units   
#define   U_UT_Display                  0x01 //!< Display units
#define   U_UT_Pixel                    0x02 //!< Pixel units
#define   U_UT_Point                    0x03 //!< Point units
#define   U_UT_Inch                     0x04 //!< Inch units
#define   U_UT_Document                 0x05 //!< Document units
#define   U_UT_Millimeter               0x06 //!< Millimeter units
/** @} */                                        

/** \defgroup U_PMF_WM_ PMF WrapMode Enumeration
  For 
  EMF+ manual 2.1.1.34, Microsoft name: WrapMode Enumeration (U_WM_*)
  @{
*/
#define   U_WM_Tile                     0x00000000 //!< Tile
#define   U_WM_TileFlipX                0x00000001 //!< Reverse horizontally then tile
#define   U_WM_TileFlipY                0x00000002 //!< Reverse vertically then tile
#define   U_WM_TileFlipXY               0x00000003 //!< Reverse horizontally and vertically then tile
#define   U_WM_Clamp                    0x00000004 //!< Clamp pattern to the object boundary
/** @} */

/** \defgroup U_PMF_BD_ PMF BrushData Flags
  For 
  EMF+ manual 2.1.2.1, Microsoft name: BrushData Flags (U_BD_*)

  Bit flags allowed in brush object types.  Each bit indicates a type of object which is included.
  There are 5 brush types abbreviated A through E, and each uses a subset of the
  BrushData Flags, as summarized in the following table:
  
         Bits  Brush____Type                          EMF+ Manual  
         used  Abbrev.  Name                                        
         5     A        U_PMF_LINEARGRADIENTBRUSHDATA 2.2.2.24     
         6     B        U_PMF_PATHGRADIENTBRUSHDATA   2.2.2.29     
         3     C        U_PMF_TEXTUREBRUSHDATA        2.2.2.45     
         0     D        U_PMF_HATCHBRUSHDATA          2.2.2.20     
         0     E        U_PMF_SOLIDBRUSHDATA          2.2.2.45     
  @{
*/
#define   U_BD_None                     0x0000  //!< no bits set
#define   U_BD_Path                     0x0001  //!< Path, in {B}                    
#define   U_BD_Transform                0x0002  //!< Transform in {ABC}                  
#define   U_BD_PresetColors             0x0004  //!< PresetColors in {AB}                    
#define   U_BD_BlendFactorsH            0x0008  //!< BlendFactorsH in {AB}                    
#define   U_BD_BlendFactorsV            0x0010  //!< BlendFactorsV in {A} - Note, not actually implemented in GDI+.                     
#define   U_BD_NoBit                    0x0020  //!< unused bit              
#define   U_BD_FocusScales              0x0040  //!< Focus Scales in {B}                    
#define   U_BD_IsGammaCorrected         0x0080  //!< GammaCorrected in {ABC}                  
#define   U_BD_DoNotTransform           0x0100  //!< Ignore world to device transform in {C}                  
#define   U_BD_MASKA                    0x009E  //!< all bits that MAY be set in A
#define   U_BD_MASKB                    0x00CF  //!< all bits that MAY be set in B
#define   U_BD_MASKC                    0x0182  //!< all bits that MAY be set in C
/** @} */

/** \defgroup U_PMF_CLCD_ PMF CustomLineCapData Flags
  For 
  EMF+ manual 2.1.2.2, Microsoft name: CustomLineCapData Flags (U_CLCD_*)
  @{
*/
#define   U_CLCD_None                   0x00 //!< no bits set
#define   U_CLCD_FillPath               0x01 //!< Fill Path 
#define   U_CLCD_LinePath               0x02 //!< Line Path 
/** @} */

/** \defgroup U_PMF_DSO_ PMF DriverStringOptions Flags
  For 
  EMF+ manual 2.1.2.3, Microsoft name: DriverStringOptions Flags (U_DSO_*)
  @{
*/
#define   U_DSO_None                    0x00   //!< no bits set 
#define   U_DSO_CmapLookup              0x01   //!< Set: value is a Unicode character; Clear: value is an index into Glyph table in a font
#define   U_DSO_Vertical                0x02   //!< Set: draw string verically; Clear: draw horizontally
#define   U_DSO_RealizedAdvance         0x04   /**< Set: U_PMF_DRAWDRIVERSTRING Positions field specifies only position of first of Glyphs field,
                                                    with the rest calculated from font information; Clear: Positions specifies coordinates for each Glyphs member.*/
#define   U_DSO_LimitSubpixel           0x08   //!< Set: use less memory to cache anti-aliased glyphs; Clear: use more
/** @} */

/** \defgroup U_PMF_FS_ PMF FontStyle Flags
  For 
  EMF+ manual 2.1.2.4, Microsoft name: FontStyle Flags (U_FS_*)
#  @{
*/
#define   U_FS_None                     0x00 //!< no bits set
#define   U_FS_Bold                     0x01 //!< Bold     
#define   U_FS_Italic                   0x02 //!< Italic   
#define   U_FS_Underline                0x04 //!< Underline
#define   U_FS_Strikeout                0x08 //!< Strikeout
/** @} */

/** \defgroup U_PMF_PLTS_ PMF PaletteStyle Flags
  For 
  EMF+ manual 2.1.2.5, Microsoft name: PaletteStyle Flags (U_PLTS_*)
  @{
*/
#define   U_PLTS_None                   0x00 //!< no bits set
#define   U_PLTS_HasAlpha               0x01 //!< Has Alpha 
#define   U_PLTS_GrayScale              0x02 //!< Gray Scale
#define   U_PLTS_Halftone               0x04 //!< Halftone 
/** @} */

/** \defgroup U_PMF_PTP_ PMF PathPointType Flags
  For 
  EMF+ manual 2.1.2.6, Microsoft name: PathPointType Flags (U_PTP_*)
  @{
*/
#define   U_PTP_None                    0x00 //!< no bits set
#define   U_PTP_DashMode                0x10 //!< Dash Mode    
#define   U_PTP_PathMarker              0x20 //!< Path Marker  
#define   U_PTP_NoBit                   0x40 //!< unused bit       
#define   U_PTP_CloseSubpath            0x80 //!< CloseSubpath
#define   U_PTP_NotClose                0x70 //!< Everything but close
#define   U_PTP_MASK                    0xF0 //!< Everything
#define   U_PTP_SHIFT                      4 //!< offset to this bitfield
/** @} */

/** \defgroup U_PMF_PD_ PMF PenData Flags
  For 
  EMF+ manual 2.1.2.7, Microsoft name: PenData Flags (U_PD_*)

If bit is set the corresponding object must be specfied in the OptionalData field

  @{
*/
#define   U_PD_None                     0x0000 //!< no bits set        
#define   U_PD_Transform                0x0001 //!< Transform     
#define   U_PD_StartCap                 0x0002 //!< Start Cap      
#define   U_PD_EndCap                   0x0004 //!< End Cap        
#define   U_PD_Join                     0x0008 //!< Join          
#define   U_PD_MiterLimit               0x0010 //!< Miter Limit    
#define   U_PD_LineStyle                0x0020 //!< Line Style     
#define   U_PD_DLCap                    0x0040 //!< Dashed Line Cap         
#define   U_PD_DLOffset                 0x0080 //!< Dashed Line Offset      
#define   U_PD_DLData                   0x0100 //!< Dashed Line Data        
#define   U_PD_NonCenter                0x0200 //!< Alignment must be specified with optinal data     
#define   U_PD_CLData                   0x0400 //!< Compound Line Data        
#define   U_PD_CustomStartCap           0x0800 //!< Custom Start Cap
#define   U_PD_CustomEndCap             0x1000 //!< Custom End Cap  
/** @} */

/** \defgroup U_PMF_SF_ PMF StringFormat Flags
  For EmfPlusStringFormat
  EMF+ manual 2.1.2.8, Microsoft name: StringFormat Flags (U_SF_*)
  @{
*/
#define   U_SF_None                     0x00000000 //!< no bits set
#define   U_SF_DirectionRightToLeft     0x00000001 //!< text Right to Left
#define   U_SF_DirectionVertical        0x00000002 //!< text Left to Right
#define   U_SF_NoFitBlackBox            0x00000004 //!< text not restricted to layout bbox
#define   U_SF_NoBit4                   0x00000008 //!< unused bit
#define   U_SF_NoBit5                   0x00000010 //!< unused bit
#define   U_SF_DisplayFormatControl     0x00000020 //!< control codes display as "representative" glyphs
#define   U_SF_NoBit7                   0x00000040 //!< unused bit
#define   U_SF_NoBit8                   0x00000080 //!< unused bit
#define   U_SF_NoBit9                   0x00000100 //!< unused bit
#define   U_SF_NoBit10                  0x00000200 //!< unused bit
#define   U_SF_NoFontFallback           0x00000400 //!< show as missing glyph if not in font
#define   U_SF_MeasureTrailingSpaces    0x00000800 //!< trailing spaces included in line length
#define   U_SF_NoWrap                   0x00001000 //!< text does not wrap
#define   U_SF_LineLimit                0x00002000 //!< emit whole lines if not clipped
#define   U_SF_NoClip                   0x00004000 //!< text is not clipped
#define   U_SF_BypassGDI                0x80000000 //!< use implementation specific text rendering instead of GDI
/** @} */

/** \defgroup U_PMF_IE_ PMF ImageEffects Identifiers
  For 
  EMF+ manual 2.1.3.1, Microsoft name: ImageEffects Identifiers (U_IE_*)
  @{
*/
#define   U_IE_BlurEffectGuid                     "{633C80A4-1843-482B-9EF2-BE2834C5FDD4}" //!< Blur Effect
#define   U_IE_BrightnessContrastEffectGuid       "{D3A1DBE1-8EC4-4C17-9F4C-EA97AD1C343D}" //!< Brightness Contrast Effect
#define   U_IE_ColorBalanceEffectGuid             "{537E597D-251E-48DA-9664-29CA496B70F8}" //!< Color Balance Effect
#define   U_IE_ColorCurveEffectGuid               "{DD6A0022-58E4-4A67-9D9B-D48EB881A53D}" //!< Color Curve Effect
#define   U_IE_ColorLookupTableEffectGuid         "{A7CE72A9-0F7F-40D7-B3CC-D0C02D5C3212}" //!< Color Lookup Table Effect
#define   U_IE_ColorMatrixEffectGuid              "{718F2615-7933-40E3-A511-5F68FE14DD74}" //!< Color Matrix Effect
#define   U_IE_HueSaturationLightnessEffectGuid   "{8B2DD6C3-EB07-4D87-A5F0-7108E26A9C5F}" //!< Hue Saturation Lightness Effect
#define   U_IE_LevelsEffectGuid                   "{99C354EC-2A31-4F3A-8C34-17A803B33A25}" //!< Levels Effect
#define   U_IE_RedEyeCorrectionEffectGuid         "{74D29D05-69A4-4266-9549-3CC52836B632}" //!< Red Eye Correction Effect
#define   U_IE_SharpenEffectGuid                  "{63CBF3EE-C526-402C-8F71-62C540BF5142}" //!< Sharpen Effect
#define   U_IE_TintEffectGuid                     "{1077AF00-2848-4441-9489-44AD4C2D7A2C}" //!< Tint Effect
/** @} */

/** \defgroup U_PMF_IEE_ PMF ImageEffects Enumerators
  based on U_IE_
  These may be used by a parser to set up for a switch() statement.
  @{
*/
#define   U_IEE_Unknown                            0 //!< none of the following
#define   U_IEE_BlurEffectGuid                     1 //!< Blur Effect
#define   U_IEE_BrightnessContrastEffectGuid       2 //!< Brightness Contrast Effect
#define   U_IEE_ColorBalanceEffectGuid             3 //!< Color Balance Effect
#define   U_IEE_ColorCurveEffectGuid               4 //!< Color Curve Effect
#define   U_IEE_ColorLookupTableEffectGuid         5 //!< Color Lookup Table Effect
#define   U_IEE_ColorMatrixEffectGuid              6 //!< Color Matrix Effect
#define   U_IEE_HueSaturationLightnessEffectGuid   7 //!< Hue Saturation Lightness Effect
#define   U_IEE_LevelsEffectGuid                   8 //!< Levels Effect
#define   U_IEE_RedEyeCorrectionEffectGuid         9 //!< Red Eye Correction Effect
#define   U_IEE_SharpenEffectGuid                 10 //!< Sharpen Effect
#define   U_IEE_TintEffectGuid                    11 //!< Tint Effect
/** @} */

/** \defgroup U_PMF_OC_ PMF ObjectClamp Identifiers
  For U_PMF_IMAGEATTRIBUTES ObjectClamp field
  EMF+ manual 2.2.1.5, Microsoft name: ImageEffects Identifiers (U_OC_*)
  @{
*/
#define   U_OC_Rect                     0x00 //!< Clamp object to rectangle.
#define   U_OC_Bitmap                   0x01 //!< Clamp object to bitmap.
/** @} */

/** \defgroup U_PMF_PPF_ PMF PathPoint Flags
        For U_PMF_PATH Flags field
        For U_PMF_CMN_HDR Flags field
        EMF+ manual 2.2.1.6, Microsoft name: PathPoint Flags (U_PPF_*)
        For U_PMF_CMN_HDR Flags the bits are scattered all over the EMF+ manual.
        NOTE:  bitfields in manual are BIG endian and MSB 0.  
               This code reads the 16 bit flag field as LITTLE endian and uses LSB 0.
               The values shown are AFTER the data has been read into a uint16_t and the byte order set
                 appropriately.
        All of these come out of a 16 bit field.
  @{
*/
#define   U_PPF_B                    0x8000 //!< 15 Set: BrushID is an U_PFM_ARGB; Clear: is index of U_PMF_BRUSH object in EMF+ object table.
#define   U_PPF_BZ                   0x8000 //!< 15 Set: Points are on a Bezier curve; Clear: Points are on a line
#define   U_PPF_N                    0x8000 //!< 15 Set: object definition continues in next record; Clear: this is the final object definition record
#define   U_PPF_K                    0x8000 //!< 15 Set: int16_t coordinates; Clear: use U_FLOAT coordinates
#define   U_PPF_C                    0x4000 //!< 14 Set: int16_t coordinates; Clear: use U_FLOAT coordinates
#define   U_PPF_XM                   0x2000 //!< 13 Set: Post multiply matrix; Clear: Pre multiply matrix
#define   U_PPF_F                    0x2000 //!< 13 Set: winding fill; Clear: alternate fill
#define   U_PPF_E                    0x2000 //!< 13 Set: effect from previous U_PMF_SERIALIZABLEOBJECT record will be applied,; Clear: no effect applied
#define   U_PPF_R                    0x1000 //!< 12 Set: U_PMF_PathPointTypeRLE and/or U_PMF_PathPointType objects; Clear: only U_PMF_PathPointType
#define   U_PPF_P                    0x0800 //!< 11 Set: relative coordinates; Clear absolute coordinates
#define   U_PPF_D                    0x0400 //!< 10 Set: draw path closed; Clear: draw path open
#define   U_PPF_VGA                  0x0002 //!<  1 Set: Palette is VGA basic colors; Clear: Palette is ???
#define   U_PPF_PP                   0x0001 //!<  0 Set: Palette field is present; Clear: Palette field is absent
#define   U_PPF_DM                   0x0001 //!<  0 Set: Dual-mode file; Clear: EMF+ only file
#define   U_PPF_AA                   0x0001 //!<  0 Set: anti-aliasing on; Clear: anti-aliasing off
#define   U_PPF_VIDEO                0x0001 //!<  0 Set: reference device is video display; Clear: reference devis is printer
/** @} */

/** \defgroup U_PMF_FF_ PMF Masks and offsets for 16 bit flag fields
  Documenting the OBSERVED positions of fields in 16 bit flag integers
     after they have been read in Little Ended from files.

  Note, some of these are used in more than one record type, only a single reference is provided
  @{
*/

#define   U_FF_MASK_SUBLID  0x003F    //!< EMF+ manual 2.2.2.23, Microsoft name: EmfPlusLanguageIdentifier
#define   U_FF_SHFT_SUBLID  0x000A    //!< EMF+ manual 2.2.2.23, Microsoft name: EmfPlusLanguageIdentifier
#define   U_FF_MASK_PRILID  0x03FF    //!< EMF+ manual 2.2.2.23, Microsoft name: EmfPlusLanguageIdentifier
#define   U_FF_SHFT_PRILID  0x0000    //!< EMF+ manual 2.2.2.23, Microsoft name: EmfPlusLanguageIdentifier
#define   U_FF_MASK_LID     0xFFFF    //!< EMF+ manual 2.2.2.23, Microsoft name: EmfPlusLanguageIdentifier
#define   U_FF_SHFT_LID     0x0000    //!< EMF+ manual 2.2.2.23, Microsoft name: EmfPlusLanguageIdentifier
#define   U_FF_MASK_RL      0x003F    //!< EMF+ manual 2.2.2.32, Microsoft name: EmfPlusPathPointTypeRLE
#define   U_FF_SHFT_RL      0x0008    //!< EMF+ manual 2.2.2.32, Microsoft name: EmfPlusPathPointTypeRLE
#define   U_FF_MASK_PPT     0x00FF    //!< EMF+ manual 2.2.2.32, Microsoft name: EmfPlusPathPointTypeRLE
#define   U_FF_SHFT_PPT     0x0000    //!< EMF+ manual 2.2.2.32, Microsoft name: EmfPlusPathPointTypeRLE
/* the next one is used most places an object ID is specified */
#define   U_FF_MASK_OID8    0x00FF    //!< EMF+ manual 2.3.1.3, Microsoft name: EmfPlusSetClipPath
#define   U_FF_SHFT_OID8    0x0000    //!< EMF+ manual 2.3.1.3, Microsoft name: EmfPlusSetClipPath
#define   U_FF_MASK_CM4     0x000F    //!< EMF+ manual 2.3.1.3, Microsoft name: EmfPlusSetClipPath
#define   U_FF_SHFT_CM4     0x0008    //!< EMF+ manual 2.3.1.3, Microsoft name: EmfPlusSetClipPath
#define   U_FF_MASK_OT      0x003F    //!< EMF+ manual 2.3.5.1, Microsoft name: EmfPlusObject
#define   U_FF_SHFT_OT      0x0008    //!< EMF+ manual 2.3.5.1, Microsoft name: EmfPlusObject
#define   U_FF_MASK_AA      0x007F    //!< EMF+ manual 2.3.6.1, Microsoft name: EmfPlusSetAntiAliasMode
#define   U_FF_SHFT_AA      0x0001    //!< EMF+ manual 2.3.6.1, Microsoft name: EmfPlusSetAntiAliasMode
#define   U_FF_MASK_CM      0x00FF    //!< EMF+ manual 2.3.6.2, Microsoft name: EmfPlusSetCompositingMode
#define   U_FF_SHFT_CM      0x0000    //!< EMF+ manual 2.3.6.2, Microsoft name: EmfPlusSetCompositingMode
#define   U_FF_MASK_CQ      0x00FF    //!< EMF+ manual 2.3.6.3, Microsoft name: EmfPlusSetCompositingQuality
#define   U_FF_SHFT_CQ      0x0000    //!< EMF+ manual 2.3.6.3, Microsoft name: EmfPlusSetCompositingQuality
#define   U_FF_MASK_IM      0x00FF    //!< EMF+ manual 2.3.6.4, Microsoft name: EmfPlusSetInterpolationMode
#define   U_FF_SHFT_IM      0x0000    //!< EMF+ manual 2.3.6.4, Microsoft name: EmfPlusSetInterpolationMode
#define   U_FF_MASK_PxOffM  0x00FF    //!< EMF+ manual 2.3.6.5, Microsoft name: EmfPlusSetPixelOffsetMode
#define   U_FF_SHFT_PxOffM  0x0000    //!< EMF+ manual 2.3.6.5, Microsoft name: EmfPlusSetPixelOffsetMode
#define   U_FF_MASK_TGC     0x0FFF    //!< EMF+ manual 2.3.6.7, Microsoft name: EmfPlusSetTextContrast
#define   U_FF_SHFT_TGC     0x0000    //!< EMF+ manual 2.3.6.7, Microsoft name: EmfPlusSetTextContrast
#define   U_FF_MASK_TRH     0x00FF    //!< EMF+ manual 2.3.6.8, Microsoft name: EmfPlusSetTextRenderingHint
#define   U_FF_SHFT_TRH     0x0000    //!< EMF+ manual 2.3.6.8, Microsoft name: EmfPlusSetTextRenderingHint
#define   U_FF_MASK_UT      0x00FF    //!< EMF+ manual 2.3.7.1, Microsoft name: EmfPlusBeginContainer
#define   U_FF_SHFT_UT      0x0008    //!< EMF+ manual 2.3.7.1, Microsoft name: EmfPlusBeginContainer
#define   U_FF_MASK_TSC     0x7FFF    //!< EMF+ manual 2.3.8.1, Microsoft name: EmfPlusSetTSClip
#define   U_FF_SHFT_TSC     0x0000    //!< EMF+ manual 2.3.8.1, Microsoft name: EmfPlusSetTSClip
#define   U_FF_MASK_PU      0x00FF    //!< EMF+ manual 2.3.9.5, Microsoft name: EmfPlusSetPageTransform
#define   U_FF_SHFT_PU      0x0000    //!< EMF+ manual 2.3.9.5, Microsoft name: EmfPlusSetPageTransform
/** @} */


/** \defgroup U_PMF_GFVR_ PMF MetafileSignature
  For U_PMF_GRAPHICSVERSION Signature field
  EMF+ manual 2.2.2.19, Microsoft name: (none) (U_GFVR_*)
  @{
*/
#define   U_GFVR_PMF                    0x000DBC01 //!< indicates an EMF+ metafile
#define   U_GFVR_MASKHI                 0xFFFFF000 //!< mask for the signature bit field (20 bits)
#define   U_GFVR_MASKLO                 0x00000FFF //!< mask for the version   bit field (12 bits)
/** @} */

/** \defgroup U_PMF_XM_ PMF Matrix Multiplication Enumerator
  For U_PMF_RotateWorldTransform and others
  EMF+ manual 2.3.9.3, Microsoft name: (none) (U_XM_*)
  @{
*/
#define   U_XM_PostX                 1 //!< Post Multiply change to current Transformation Matrix
#define   U_XM_PreX                  0 //!< Pre  Multiply change to current Transformation Matrix
/** @} */


/* Utility objects, not defined in EMF+ spec */

/**  @brief  Used to accumulate data for objects continued over multiple records.
  see EMF+ manual 2.3.5.1
*/
typedef struct {
   char      *accum;      /**< data accumulates here  */
   uint32_t   space;      /**< bytes allocated        */
   uint32_t   used;       /**< bytes in use           */
   int        Type;       /**< ObjectType enumeration */
   int        Id;         /**< Object ID              */
} U_OBJ_ACCUM;

/**  @brief Holds EMF+ objects and records in EMF+ file format byte order.
*/
typedef struct {
   char      *Data;       /**< Buffer that hold's the PseudoObject's data */
   size_t     Size;       /**< Number of bytes allocated in Data (may be >Used if padding is present) */
   size_t     Used;       /**< Number of data bytes that are stored in Data  */
   uint32_t   Type;       /**< Type numbers are from manual section: 1.2.3.4 -> 10203040 */
} U_PSEUDO_OBJ;

/** @brief DoublePseudoObject holds pairs of PseudoObjects.  Used for constructing paths along with their types.
  The data stored in the PsuedoObjects maintains LittleEndian-ness, as expected in the final file.
  The type is U_RAW_OID, and there is no elements count at the beginning of Data
*/
typedef struct {
   uint32_t      Elements;  /**< Element count, applies to both PseudoObjects */
   U_PSEUDO_OBJ *poPoints;  /**< Points in path */
   U_PSEUDO_OBJ *poTypes;   /**< Types of points in path */
} U_DPSEUDO_OBJ;

/** @brief Serializer description records.  

An array of these are passed to U_PMF_SERIAL_set() to construct EMF+ objects from their component parts. 
The U_PMF_SERIAL_set() function should not ever be called directly by end user code.
*/
typedef struct {
    const void *Ptr;   /**< Pointer to the first byte of the data field.   
                            Each data field is an array of a basic type of Units 
                            bytes repeated Reps times */
    size_t      Units; /**< Number of bytes in each unit of each data field. */
    size_t      Reps;  /**< MNumber of repeats of Units in eah data field. */
    int         TE;    /**< (Target Endian). Only relevant for Units of 2 or 4*/
} U_SERIAL_DESC;

/** @brief FontInfoParams hold font information that is needed by U_PMR_drawstring so that it can
  place text on the baseline.  This must be extracted from the font file using
  an appropriate utility.  (See testbed_pmf.c for a table of these values for some
  common fonts.)
*/
typedef struct {
   char    *name;     /**< Font name (like "Arial")  */
   int     Ascent;    /**< in Font units (positive)  */
   int     Descent;   /**< in Font units (negative)  */
   int     LineGap;   /**< in Font units (positive)  */
   int     EmSize;    /**< Y extent of Em square, usually 2048  */
   int     yMax;      /**< in Font units (positive)  */
   int     yMin;      /**< in Font units (negative)  */
} U_FontInfoParams;



/* EMF+ objects */

/** @brief EMF+ manual 2.2.1.1, Microsoft name: EmfPlusBrush Object

variable part of object follows structure:
         uint32_t            Data[];             // one of the 5 types of Brush data (2.2.2 20, 24, 29, 43, or 45)
*/
typedef struct {
    uint32_t            Version;            //!< EmfPlusGraphicsVersion object
    uint32_t            Type;               //!< BrushType Enumeration
} U_PMF_BRUSH;

/**  @brief EMF+ manual 2.2.1.2, Microsoft name: EmfPlusCustomLineCap Object */
typedef struct {
/*@{*/
    uint32_t            Version;            //!< EmfPlusGraphicsVersion object
    uint32_t            Type;               //!< BrushType Enumeration
/* variable part of object, not part of structure
    uint32_t            Data[];             //!< one of the 2 types of Linecap data (2.2.2 12, 13)
*/
/*@}*/
} U_PMF_CUSTOMLINECAP;

/** @brief EMF+ manual 2.2.1.3, Microsoft name: EmfPlusFont Object */
typedef struct {
/*@{*/
    uint32_t            Version;            //!< EmfPlusGraphicsVersion object
    U_FLOAT             EmSize;             //!< em size in units of SizeUnit
    uint32_t            SizeUnit;           //!< UnitType enumeration
    int32_t             FSFlags;            //!< FontStyle flags
    uint32_t            Reserved;           //!< ignored
    uint32_t            Length;             //!< Number of Unicode Characters in FamilyName
/* variable part of object, not part of structure
    uint16_t            FamilyName[];       //!< Unicode (UTF-16LE) name of font family
*/
/*@}*/
} U_PMF_FONT;

/** @brief EMF+ manual 2.2.1.4, Microsoft name: EmfPlusImage Object */
typedef struct {
/*@{*/
    uint32_t            Version;            //!< EmfPlusGraphicsVersion object
    uint32_t            Type;               //!< ImageDataType Enumeration
/* variable part of object, not part of structure
    uint32_t            Data[];             //!< one of the 2 types of image data (2.2.2 2 or 27)
*/
/*@}*/
} U_PMF_IMAGE;

/** @brief EMF+ manual 2.2.2.1, Microsoft name: EmfPlusARGB Object, out of order, needed for 2.2.1.5 */
typedef struct {
/*@{*/
    uint8_t             Blue;               //!< Blue  color (0-255) 
    uint8_t             Green;              //!< Green color (0-255)
    uint8_t             Red;                //!< Red   color (0-255)
    uint8_t             Alpha;              //!< Alpha       (0-255) 
/*@}*/
} U_PMF_ARGB;

/** @brief EMF+ manual 2.2.1.5, Microsoft name: EmfPlusImageAttributes Object */
typedef struct {
 /*@{*/
   uint32_t            Version;            //!< EmfPlusGraphicsVersion object
    uint32_t            Reserved1;          //!< ignored
    uint32_t            WrapMode;           //!< WrapMode object
    U_PMF_ARGB          ClampColor;         //!< EmfPlusARGB object
    int32_t             ObjectClamp;        //!< ObjectClamp Identifiers
    uint32_t            Reserved2;          //!< ignored
/*@}*/
} U_PMF_IMAGEATTRIBUTES;

/** @brief EMF+ manual 2.2.1.6, Microsoft name: EmfPlusPath Object */
typedef struct {
/*@{*/
    uint32_t            Version;            //!< EmfPlusGraphicsVersion object
    uint32_t            Count;              //!< points and point types in this object
    uint16_t            Flags;              //!< PathPoint Flags
/* variable part of object, not part of structure
    points   array of points like:
              U_PPF_P U_PPF_C  Type
              1       x        U_PMF_POINTR
              0       1        U_PMF_POINT
              0       0        U_PMF_POINTF
    types     array of:.  
              U_PPF_R Type
              1       U_PMF_PATHPOINTTYPERLE and/or U_PMF_PATHPOINTTYPE
              0       U_PMF_PathPointType (only)
    alignment padding   up to 3 bytes
*/
/*@}*/
} U_PMF_PATH;

/** @brief EMF+ manual 2.2.1.7, Microsoft name: EmfPlusPen Object */

typedef struct {
    uint32_t            Version;            //!< EmfPlusGraphicsVersion object
    uint32_t            type;               //!< must be zero
/* variable part of object, not part of structure
    U_PMF_PENDATA    pen  
    U_PMF_BRUSH      brush
*/
} U_PMF_PEN;

/** @brief EMF+ manual 2.2.2.40, Microsoft name: EmfPlusRegionNode Object, out of order, needed for 2.2.1.8 */
typedef struct {
    uint32_t            Type;               //!< RegionNodeDataType
/* variable part of object, not part of structure, will be absent in object for some types
     data   data is a tree made up of some combination of these objects
     U_PMF_REGIONNODEPATH        2.2.2.42 terminal node 
     U_PMF_RECTF                 2.2.2.39 terminal node
     U_PMF_REGIONNODECHILDNODES  2.2.2.41 non-terminal node
*/
} U_PMF_REGIONNODE;

/** @brief EMF+ manual 2.2.1.8, Microsoft name: EmfPlusRegion Object */
typedef struct {
    uint32_t            Version;            //!< EmfPlusGraphicsVersion object
    uint32_t            Elements;           //!< Number of members in Nodes array
/* variable part of object, not part of structure, will be absent in object for some types
    U_PMF_REGIONNODE    Nodes[1];           //!< Nodes defining region
*/
} U_PMF_REGION;

/** @brief EMF+ manual 2.2.2.23, Microsoft name: EmfPlusLanguageIdentifier Object, out of order, needed for 2.2.1.9 

Bit fields are not used in structs in this implementation, these are serialized/deserialized in 
the corresponding routines.  Bitfields in the FILE (LITTLE endian here, manual uses BIG endian) are:
        int              SubLId : 6;           Example: code for USA
        int              PriLId : 10;          Example: code for English
     
This type is defined as 16 bits in the manual section, but it is only ever used as part of a 32 bit field!
*/
typedef uint32_t   U_PMF_LANGUAGEIDENTIFIER;

/** @brief EMF+ manual 2.2.1.9, Microsoft name: EmfPlusStringFormat Object */
typedef struct {
    uint32_t            Version;            //!< EmfPlusGraphicsVersion object
    uint32_t            Flags;              //!< StringFormat flags
    U_PMF_LANGUAGEIDENTIFIER
                        Language;           //!< String's Language
    uint32_t            StringAlignment;    //!< StringAlignment enumeration. 
    uint32_t            LineAlign;          //!< StringAlignment enumeration. 
    uint32_t            DigitSubstitution;  //!< StringDigitSubstitution enumeration 
    U_PMF_LANGUAGEIDENTIFIER
                        DigitLanguage;      //!< Digit's Language (overrides Language, above)
    U_FLOAT             FirstTabOffset;     //!< the number of spaces to the first tab stop. 
    int32_t             HotkeyPrefix;       //!< HotkeyPrefix enumeration
    U_FLOAT             LeadingMargin;      //!< space before starting position (text) of a string
    U_FLOAT             TrailingMargin;     //!< space after last position (text) of a string
    U_FLOAT             Tracking;           //!< horizontal space alotted per character/font specification per character
    uint32_t            Trimming;           //!< StringTrimming enumeration 
    uint32_t            TabStopCount;       //!< Number of tab stops in data field. 
    uint32_t            RangeCount;         //!< Number of U_PMF_CHARACTERRANGE objects in data field.
/* variable part of object, not part of structure.
    U_PMF_STRINGFORMATDATA  data
  
   Note that U_PMF_STRINGFORMATDATA has no struct as it is entirely variable
   and the size of the two fields in it are specified by the two preceding fields in this object type.
*/
} U_PMF_STRINGFORMAT;

/** U_PMF_ARGB EMF+ manual 2.2.2.1, Microsoft name: EmfPlusARGB Object, defined above, before 2.2.1.6*/

/** @brief EMF+ manual 2.2.2.2, Microsoft name: EmfPlusBitmap Object */
typedef struct {
    int32_t             Width;              //!< Width in pixels
    int32_t             Height;             //!< Height in pixels
    int32_t             Stride;             //!< length in bytes of 1 scan line (multiple of 4)
    uint32_t            PxFormat;           //!< PixelFormat enumeration
    uint32_t            Type;               //!< BitmapDataType enumeration (section 2.1.1.2). 
/* variable part of object, not part of structure.
    (various types)     BitmapData          //!< is either an U_PMF_BITMAPDATA or U_PMF_COMPRESSEDIMAGE object 
*/
} U_PMF_BITMAP;

/** U_PMF_BITMAPDATA EMF+ manual 2.2.2.3, Microsoft name: EmfPlusBitmapData Object
U_PMF_BITMAPDATA is an entirely variable object, there is no corresponding struct.  It consists of

   Colors    U_PMF_PALETTE object
   PixelData An array of bytes, meaning depends on fields in U_PMF_BITMAP object and the PixelFormat enumeration. 

*/

/** @brief EMF+ manual 2.2.2.4, Microsoft name: EmfPlusBlendColors Object
   For Pattern field of U_PMF_LINEARGRADIENTBRUSHOPTIONALDATA
*/
typedef struct {
    uint32_t            Elements;           //!< members in each array
/* variable part of object, not part of structure.
    U_FLOAT             Positions           //!< positions along gradient line. The first position MUST be 0.0 and the last MUST be 1.0.
    U_PMF_ARGB          Colors              //!< colors at positions on gradient line
*/
} U_PMF_BLENDCOLORS;

/** @brief EMF+ manual 2.2.2.5, Microsoft name: EmfPlusBlendFactors Object */
typedef struct {
    uint32_t            Elements;           //!< Members in each array
/* variable part of object, not part of structure.
    U_FLOAT             Positions           //!< positions along gradient line. The first position MUST be 0.0 and the last MUST be 1.0.
    U_FLOAT             Factors             //!< blending factors, 0.0->1.0 values, inclusive
*/
} U_PMF_BLENDFACTORS;

/** @brief EMF+ manual 2.2.2.6, Microsoft name: EmfPlusBoundaryPathData Object */
typedef struct {
    int32_t             Size;               //!< Bytes in Data
/* variable part of object, not part of structure.
    U_PMF_PATH          Data                //!< Boundary of the brush
*/
} U_PMF_BOUNDARYPATHDATA;

/** @brief EMF+ manual 2.2.2.7, Microsoft name: EmfPlusBoundaryPointData Object */
typedef struct {
    int32_t             Elements;           //!< Members in the array
/* variable part of object, not part of structure.
    U_PMF_POINTF        Points              //!< Boundary of the brush
*/
} U_PMF_BOUNDARYPOINTDATA;

/** @brief EMF+ manual 2.2.2.8, Microsoft name: EmfPlusCharacterRange Object */
typedef struct {
    int32_t             First;              //!< First position in range
    int32_t             Length;             //!< Range length
} U_PMF_CHARACTERRANGE;

/** @brief EMF+ manual 2.2.2.9, Microsoft name: EmfPlusCompoundLineData Object 
Compound lines are pens that draw several parallel lines at once.  The data here
alternates (sub)line width (as fraction of total width and gaps (also as fraction
of total width).
*/
typedef struct {
    int32_t             Elements;           //!< Members in the array
/* variable part of object, not part of structure.
    U_FLOAT             Data                //!< Line or gap width (0.0 <-> 1.0, fraction of total line width )
*/
} U_PMF_COMPOUNDLINEDATA;

/** @brief EMF+ manual 2.2.2.10, Microsoft name: EmfPlusCompressedImage Object 
Holds an EXIF, GIF, JFIF, PNG, or TIFF image.
  For U_PMF_BITMAP BitmapData field
  
  object has no assocated struct!
  U_PMF_COMPRESSEDIMAGE
*/

/** @brief EMF+ manual 2.2.2.11, Microsoft name: EmfPlusCustomEndCapData Object */
typedef struct {
    int32_t             Size;               //!< Bytes in Data
/* variable part of object, not part of structure.
    U_PMF_CUSTOMLINECAP Data                //!< Description of linecap
*/
} U_PMF_CUSTOMENDCAPDATA;

/** @brief EMF+ manual 2.2.2.12, Microsoft name: EmfPlusCustomLineCapArrowData Object */
typedef struct {
    U_FLOAT             Width;              //!< Arrow cap width (is multiplied by line width before draw)
    U_FLOAT             Height;             //!< Arrow cap length (is multiplied by line width before draw)
    U_FLOAT             MiddleInset;        //!< Pixels between outer edge and filled region
    uint32_t            FillState;          //!< If set, fill, otherwise, only border
    uint32_t            StartCap;           //!< LineCap enumeration (type of cap)
    uint32_t            EndCap;             //!< LineCap enumeration
    uint32_t            Join;               //!< LineJoin enumeration
    U_FLOAT             MiterLimit;         //!< Maximum (miter length / line width)
    U_FLOAT             WidthScale;         //!< Scale for U_PMF_CUSTOMLINECAP object
    U_FLOAT             FillHotSpot[2];     //!< must be 0.0, 0.0
    U_FLOAT             LineHotSpot[2];     //!< must be 0.0, 0.0
} U_PMF_CUSTOMLINECAPARROWDATA;

/** @brief EMF+ manual 2.2.2.13, Microsoft name: EmfPlusCustomLineCapData Object */
typedef struct {
    uint32_t            Flags;              //!< CustomLineCapData flags
    uint32_t            Cap;                //!< LineCap enumeration (type of cap)
    U_FLOAT             Inset;              //!< Distance line cap start -> line end
    uint32_t            StartCap;           //!< LineCap enumeration
    uint32_t            EndCap;             //!< LineCap enumeration
    uint32_t            Join;               //!< LineJoin enumeration
    U_FLOAT             MiterLimit;         //!< Maximum (miter length / line width)
    U_FLOAT             WidthScale;         //!< Scale for U_PMF_CUSTOMLINECAP object
    U_FLOAT             FillHotSpot[2];     //!< must be 0.0, 0.0
    U_FLOAT             LineHotSpot[2];     //!< must be 0.0, 0.0
/* variable part of object, not part of structure.
    U_PMF_CUSTOMLINECAPOPTIONALDATA Data    //!< meaning determined by Flags
*/
} U_PMF_CUSTOMLINECAPDATA;

/** U_PMF_CUSTOMLINECAPOPTIONALDATA EMF+ manual 2.2.2.14, Microsoft name: EmfPlusCustomLineCapOptionalData Object 
  
  object has no assocated struct!
  
  U_PMF_FILLPATHO       FillData;           //!< path to fill (optional)
  U_PMF_LINEPATH        LineData;           //!< path to stroke (optional)
*/

/** @brief EMF+ manual 2.2.2.15, Microsoft name: EmfPlusCustomStartCapData Object */
typedef struct {
    int32_t             Size;               //!< Bytes in Data
/* variable part of object, not part of structure.
    U_PMF_CUSTOMLINECAP Data                //!< Description of linecap
*/
} U_PMF_CUSTOMSTARTCAPDATA;

/** @brief EMF+ manual 2.2.2.16, Microsoft name: EmfPlusDashedLineData Object */
typedef struct {
    int32_t             Elements;           //!< Elements in Data
/* variable part of object, not part of structure.
    U_FLOAT             Data;               //!< Array of lengths of dashes and spaces
*/
} U_PMF_DASHEDLINEDATA;

/** @brief EMF+ manual 2.2.2.17, Microsoft name: EmfPlusFillPath Object
Note: U_PMF_FILLPATHOBJ is the object, U_PMF_FILLPATH is the file record
*/

typedef struct {
    int32_t             Size;               //!< Bytes in Data
/* variable part of object, not part of structure.
    U_PMF_PATH          Data;               //!< Path specification
*/
} U_PMF_FILLPATHO;

/** @brief EMF+ manual 2.2.2.18, Microsoft name: EmfPlusFocusScaleData Object
  for U_PMF_PATHGRADIENTBRUSHOPTIONALDATA data field

Used with path gradient brushes.  May be used to expand the center color
of a gradient, which would otherwise only be found at the center point.
The expanded area is the width, height X scale factor, but in no case
less than 1 pixel.

*/
typedef struct {
    uint32_t            Count;              //!< must be 2
    U_FLOAT             ScaleX;             //!< value 0.0 <-> 1.0
    U_FLOAT             ScaleY;             //!< value 0.0 <-> 1.0
} U_PMF_FOCUSSCALEDATA;

/** @brief EMF+ manual 2.2.2.19, Microsoft name: EmfPlusGraphicsVersion Object

Bit fields are not used in structs in this implementation, these are serialized/deserialized in 
the corresponding routines.  Bitfields in the FILE (LITTLE endian here, manual uses BIG endian) are:
        unsigned int       GrfVersion : 12;      GraphicsVersion enumeration
        unsigned int       Signature  : 20;      Must be U_GFVR_PMF (0xDBC01)
  @{
*/
typedef uint32_t   U_PMF_GRAPHICSVERSION; //!< EMF+ manual 2.2.2.19, Microsoft name: EmfPlusGraphicsVersion Object
/** @} */

/** @brief EMF+ manual 2.2.2.20, Microsoft name: EmfPlusHatchBrushData Object */
typedef struct {
    uint32_t            Style;              //!< HatchStyle enumeration
    U_PMF_ARGB          Foreground;         //!< Hatch pattern line color
    U_PMF_ARGB          Background;         //!< Hatch pattern bkground color
} U_PMF_HATCHBRUSHDATA;

/** \defgroup U_PMF_Int7 PMF 7 bit signed integer
   @brief EMF+ manual 2.2.2.21, Microsoft name: EmfPlusInteger7 Object 

        bit 7      U_INT7 Clear in Integer7 objects
        bits 0-6   7 bit signed integer value
  @{
*/
#define U_TEST_INT7 0x80 //!< This bit is clear in Integer7 objects.
#define U_SIGN_INT7 0x40 //!< Sign bit on an Integer7 object.
#define U_MASK_INT7 0x7F //!< xMask to retrieve integer7 bits.
/** @} */

/** \defgroup U_PMF_Int15 PMF 15 bit signed integer
   @brief EMF+ manual 2.2.2.22, Microsoft name: EmfPlusInteger15 Object 

       bit 15      U_INT15 Set in Integer15 objects
       bits 0-15   15 bit signed integer value

   This is the one data type that really does seem to be stored into the file in Big Endian order.  
   It has to be this way because the bit that determines if data is int7 or int15 must be in the first byte 
   the parser sees, and that byte is the high order byte.
  @{
*/
#define U_TEST_INT15 0x8000  //!< This bit is set in Integer15 objects.
#define U_SIGN_INT15 0x4000  //!< Sign bit on an Integer15 object.
#define U_MASK_INT15 0x7FFF  //!< Mask to retrieve integer15 bits.
/** @} */

/* EMF+ manual 2.2.2.23, Microsoft name: EmfPlusLanguageIdentifier Object, defined above, before 2.2.1.9  */

/** @brief EMF+ manual 2.2.2.39, Microsoft name: EmfPlusRectF Object, out of order, needed for 2.2.2.24  */
typedef struct {
    U_FLOAT             X;                  //!< UL X value
    U_FLOAT             Y;                  //!< UL Y value
    U_FLOAT             Width;              //!< Width
    U_FLOAT             Height;             //!< Height
} U_PMF_RECTF;

/** @brief EMF+ manual 2.2.2.24, Microsoft name: EmfPlusLinearGradientBrushData Object 
Manual says that Reserved1 and Reserved2 must be ignored.  In practice if Reserved1 is not set to StartColor
and Reserved2 is not set to EndColor, then XP Preview will not display the gradient.
*/
typedef struct {
    uint32_t            Flags;              //!< BrushData flags
    int32_t             WrapMode;           //!< WrapMode enumeration
    U_PMF_RECTF         RectF;              //!< UL=start, LR=end of gradient
    U_PMF_ARGB          StartColor;         //!< Gradient start color
    U_PMF_ARGB          EndColor;           //!< Gradient end color
    uint32_t            Reserved1;          //!< ignore
    uint32_t            Reserved2;          //!< ignore
/* variable part of object, not part of structure.
    U_PMF_LINEARGRADIENTBRUSHOPTIONALDATA data;  //!< presence and meaning depend on Flags field
*/
} U_PMF_LINEARGRADIENTBRUSHDATA;

/** @brief EMF+ manual 2.2.2.47, Microsoft name: EmfPlusTransformMatrix Object, out of order, needed for 2.2.2.25 */
typedef struct {
    U_FLOAT            m11;                 //!< Rotation matrix m11 element
    U_FLOAT            m12;                 //!< Rotation matrix m12 element
    U_FLOAT            m21;                 //!< Rotation matrix m21 element
    U_FLOAT            m22;                 //!< Rotation matrix m22 element
    U_FLOAT            dX;                  //!< Translation in X
    U_FLOAT            dY;                  //!< Translation in Y
} U_PMF_TRANSFORMMATRIX;
 
/** NOT DOCUMENTED.  Encountered in actual EmfPlusLinearGradientBrushOptionalData Object made by PowerPoint 2003.  This
    structure is needed for the next. */
typedef struct {
    U_FLOAT            m11;                 //!< Rotation matrix m11 element
    U_FLOAT            m12;                 //!< Rotation matrix m12 element
    U_FLOAT            m21;                 //!< Rotation matrix m21 element
    U_FLOAT            m22;                 //!< Rotation matrix m22 element
} U_PMF_ROTMATRIX;

/** @brief EMF+ manual 2.2.2.25, Microsoft name: EmfPlusLinearGradientBrushOptionalData Object 
  For U_PMF_LINEARGRADIENTBRUSHDATA data field 
*/
/* Entire object is variable and not part of a structure! U_PMF_LINEARGRADIENTBRUSHOPTIONALDATA
    U_PMF_ROTMATRIX             Matrix;     //!< Rotation matrix, Manuals says that this should be Transformation matrix, but last two values are missing
    (various)                   pattern;    //!< Presence and meaning depend on Flags field, see below
    
    Flag values
    U_BD_PresetColors U_BD_BlendFactorsH U_BD_BlendFactorsV    pattern(s) present?
    0                 0                  0                     none
    1                 0                  0                     U_PMF_BLENDCOLORS
    0                 1                  0                     U_PMF_BLENDFACTORS
    0                 0                  1                     U_PMF_BLENDFACTORS
    0                 1                  1                     U_PMF_BLENDFACTORS, U_PMF_BLENDFACTORS
*/

/** @brief EMF+ manual 2.2.2.26, Microsoft name: EmfPlusLinePath Object */
typedef struct {
    int32_t             Size;               //!< Bytes in Data
/* variable part of object, not part of structure.
    U_PMF_PATH          Data;               //!< Outline path
*/
} U_PMF_LINEPATH;

/** @brief EMF+ manual 2.2.2.27, Microsoft name: EmfPlusMetafile Object */
typedef struct {
    uint32_t            Type;               //!< MetaFileDatatype enumeration
    uint32_t            Size;               //!< Bytes in Data
/* variable part of object, not part of structure.
    U_PMF_IMAGE         Data;               //!< Various types of data, like an EMF metafile, WMF metafile, another EMF+ metafile
*/
} U_PMF_METAFILE;

/** @brief EMF+ manual 2.2.2.28, Microsoft name: EmfPlusPalette Object */
typedef struct {
    uint32_t            Flags;              //!< PaletteStyle flags
    uint32_t            Elements;           //!< elements in Data
/* variable part of object, not part of structure.
    U_PMF_ARGB          Data;               //!< Palette data (array of colors)
*/
} U_PMF_PALETTE;

/** @brief EMF+ manual 2.2.2.36, Microsoft name: EmfPlusPointF Object, out of order, needed for 2.2.2.29 */
typedef struct {
    U_FLOAT             X;                  //!< UL X value
    U_FLOAT             Y;                  //!< UL Y value
} U_PMF_POINTF;

/** @brief EMF+ manual 2.2.2.29, Microsoft name: EmfPlusPathGradientBrushData Object */
typedef struct {
    uint32_t            Flags;              //!< BrushData flags
    int32_t             WrapMode;           //!< WrapMode enumeration
    U_PMF_ARGB          CenterColor;        //!< Gradient center color
    U_PMF_POINTF        Center;             //!< Center coordinates
    uint32_t            Elements;           //!< Number of elements in gradient (not counting center)
/* variable part of object, not part of structure.
    U_PMF_ARGB          Gradient;           //!< Color Gradient with Elements members
    (varies)            Boundary;           //!< U_PMF_BOUNDARYPATHDATA object if BrushDataPath bit set in Flag, else U_PMF_BOUNDARYPOINTDATA object
    U_PMF_GRADIENTBRUSHOPTIONALDATA data;   //!< exact composition depends on Flags
*/
} U_PMF_PATHGRADIENTBRUSHDATA;

/** EMF+ manual 2.2.2.30, Microsoft name: EmfPlusPathGradientBrushOptionalData Object 
   for U_PMF_PATHGRADIENTNBRUSHDATA data field
*/
/* Entire thing is variable or optional.
typedef struct {
    U_PMF_TRANSFORMMATRIX       Matrix;     //!< Optional Transformation matrix
    U_PMF_BLENDCOLORS           Pattern;    //!< presence and meaning depend on Flags field
                                                 Flag values
                                                 U_BD_PresetColors U_BD_BlendFactorsH    pattern?
                                                 0                 0                     none
                                                 1                 0                     U_PMF_BLENDCOLORS
                                                 0                 1                     U_PMF_BLENDFACTORS
    U_PMF_FOCUSSSCALEDATA       data        //!< Present if U_BD_FocusScales bit set in Flags in U_PMF_PATHGRADIENTNBRUSHDATA object
} U_PMF_PATHGRADIENTBRUSHOPTIONALDATA;
*/

/** \defgroup U_PMF_PPTYPE PMF Path Point Types
    @brief EMF+ manual 2.2.2.31, Microsoft name: EmfPlusPathPointType Object 

Bitfields in the FILE (LITTLE endian here, manual uses BIG endian) are:
        bits 4-7  PathPointType flags
        bits 0-3  PathPointType enumeration
  @{
*/
typedef uint8_t   U_PMF_PATHPOINTTYPE; //!< EMF+ manual 2.2.2.31, Microsoft name: EmfPlusPathPointType Object
/** @} */

/** \defgroup U_PMF_PPTYPERLE PMF Run Length Encoded Path Point Types
    @brief EMF+ manual 2.2.2.32, Microsoft name: EmfPlusPathPointTypeRLE Object 

U_PMF_PATHPOINTTYPERLE fields specify point types in a path where the path is Run Length Encoded.
Bit fields are not used in structs in this implementation, these are serialized/deserialized in 
the corresponding routines.  Bitfields in the FILE (LITTLE endian here, manual uses BIG endian) are:
        bit     15  Set: Bezier curve; Clear: straight line
        bit     14  ignored
        bits  8-13  Run count
        bits   0-7  PathPointType enumeration
  @{
*/
typedef uint16_t   U_PMF_PATHPOINTTYPERLE; //!< EMF+ manual 2.2.2.32, Microsoft name: EmfPlusPathPointTypeRLE Object
/** @} */

/** @brief EMF+ manual 2.2.2.33, Microsoft name: EmfPlusPenData Object 

Variable part of object follows structure:
        U_PMF_PENOPTIONALDATA data;  Optional pen data, exact composition depends on Flags
*/
typedef struct {
    uint32_t            Flags;              //!< PenData flags
    uint32_t            Unit;               //!< UnitType enumeration
    U_FLOAT             Width;              //!< Width in units set by Unit
} U_PMF_PENDATA;

/** @brief EMF+ manual 2.2.2.34, Microsoft name: EmfPlusPenOptionalData Object

Every part of this object is variable or optional, there is no corresponding struct

                                                   Present if Flag        What is it
    U_PMF_TRANSFORMMATRIX    Matrix           //!< U_PD_Transform         Transformation matrix
    int32_t                  StartCap         //!< U_PD_StartCap          LineCapType enumeration
    int32_t                  EndCap           //!< U_PD_EndCap            LineCapType enumeration
    uint32_t                 Join             //!< U_PD_Join              LineJoinType enumeration
    U_FLOAT                  MiterLimit       //!< U_PD_MiterLimit        Maximum (miter length / line width)
    int32_t                  Style            //!< U_PD_LineStyle         LineStyle enumeration
    int32_t                  DLCap            //!< U_PD_DLCap             DashedLineCapType enumeration
    U_FLOAT                  DLOffset         //!< U_PD_DLOffset          Distance line start to first dash start
    U_PMF_DASHEDLINEDATA     DLData           //!< U_PD_DLData            Dash and space widths
    int32_t                  PenAlignment     //!< U_PD_NonCenter         PenAlignment enumeration
    U_PMF_COMPOUNDLINEDATA   CLData           //!< U_PD_CompoundLineData  Compount Line (parallel lines drawn instead of one)
    U_PMF_CUSTOMSTARTCAPDATA CSCapData        //!< U_PD_CustomStartCap    Custom start cap  
    U_PMF_CUSTOMENDCAPDATA   CECapData        //!< U_PD_CustomEndCap      Custom end cap
 */

/** @brief EMF+ manual 2.2.2.35, Microsoft name: EmfPlusPoint Object */
typedef struct {
    int16_t             X;                  //!< X coordinate
    int16_t             Y;                  //!< Y coordinate
} U_PMF_POINT;

/** U_PMF_POINTF EMF+ manual 2.2.2.36, Microsoft name: EmfPlusPointF Object, defined above, before 2.2.2.29 */

/** U_PMF_POINTR EMF+ manual 2.2.2.37, Microsoft name: EmfPlusPointR Object 
  For U_PMF_DRAWBEZIERS data field (optionally).
  Both parts of this object are variable, there is no corresponding struct.
  Any combination of the two allowed types of integer is valid.

    (varies)            X;                  //!< U_PMF_INTEGER7 or U_PMF_INTEGER15
    (varies)            Y;                  //!< U_PMF_INTEGER7 or U_PMF_INTEGER15

*/

/** @brief EMF+ manual 2.2.2.38, Microsoft name: EmfPlusRect Object */
typedef struct {
    int16_t            X;                  //!< UL X value
    int16_t            Y;                  //!< UL Y value
    int16_t            Width;              //!< Width
    int16_t            Height;             //!< Height
} U_PMF_RECT;

/** U_PMF_RECTF EMF+ manual 2.2.2.39, Microsoft name: EmfPlusRectF Object, defined above, before 2.2.2.24  */

/** U_PMF_REGIONNODE EMF+ manual 2.2.2.40, Microsoft name: EmfPlusRegionNode Object, defined above, before 2.2.1.8 */

/** U_PMF_REGIONNODECHILDNODES EMF+ manual 2.2.2.41, Microsoft name: EmfPlusRegionNodeChildNodes Object
  For U_PMF_REGIONNODE data field (optionally).
  Both parts of this object are variable, there is no corresponding struct.
  U_PMF_REGIONNODE      Left;               //!< Left child
  U_PMF_REGIONNODE      Right;              //!< Right child
*/

/** @brief EMF+ manual 2.2.2.42, Microsoft name: EmfPlusRegionNodePath Object */
typedef struct {
    int32_t             Size;               //!< Bytes in Data
/* variable part of object, not part of structure.
    U_PMF_PATH          Data;               //!< Boundary of region node
*/
} U_PMF_REGIONNODEPATH;

/** @brief EMF+ manual 2.2.2.43, Microsoft name: EmfPlusSolidBrushData Object 
  For U_PMF_BRUSH data field (one type of brush)
*/
typedef struct {
    U_PMF_ARGB          Color;              //!< Color of brush 
} U_PMF_SOLIDBRUSHDATA;

/** U_PMF_STRINGFORMATDATA EMF+ manual 2.2.2.44, Microsoft name: EmfPlusStringFormatData Object
  Both parts of this object are variable and optional, there is no corresponding struct
    U_FLOAT                TabStops[];      //!< Array of tabstop locations
    U_PMF_CHARACTERRANGE   CharRange[];     //!< Array of character ranges in the text
*/

/** @brief EMF+ manual 2.2.2.45, Microsoft name: EmfPlusTextureBrushData Object */
typedef struct {
    uint32_t            Flags;              //!< BrushData flags
    int32_t             WrapMode;           //!< WrapMode enumeration
/* variable part of object, not part of structure.
    U_PMF_TEXTUREBRUSHOPTIONALDATA data;    //!< Optional texture data
*/
} U_PMF_TEXTUREBRUSHDATA;

/** U_PMF_TEXTUREBRUSHOPTIONALDATA EMF+ manual 2.2.2.46, Microsoft name: EmfPlusTextureBrushOptionalData Object

Every part of this object is variable or optional, there is no corresponding struct

        U_PMF_TRANSFORMMATRIX  Matrix;          Transformation matrix, present if Flag BrushDataTransform is set.
        U_PMF_IMAGE            Image            Image that contains the texture. Present if the PMR record that includes this object still has space
                                                  for an U_PMF_IMAGE after all the other variable and optional data
                                                  within it has been accounted for.
*/

/** U_PMF_TRANSFORMMATRIX EMF+ manual 2.2.2.47, Microsoft name: EmfPlusTransformMatrix Object, defined above, before 2.2.2.25 */

/** common structure present at the beginning of all(*) EMF+ records */
typedef struct {
    uint16_t            Type;               //!< Recordtype enumeration (what this record is)
    uint16_t            Flags;              //!< Flags (meaning varies by record type)
    uint32_t            Size;               //!< Bytes in record, including this struct (will be a multiple of 4)
    uint32_t            DataSize;           //!< Bytes of data that follow, may not be a multiple of 4.
} U_PMF_CMN_HDR;

/** These are the Image Effect Objects 2.2.3.*  They specify parameters for "filters" that may be applied to bitmaps. */

/** @brief EMF+ manual 2.2.3.1, Microsoft name: BlurEffect Object */
typedef struct {
    U_FLOAT             Radius;             //!< Blur radius in pixels
    uint32_t            ExpandEdge;         //!< 1: expand bitmap by Radius; 0: bitmap size unchanged
} U_PMF_IE_BLUR;


/** @brief EMF+ manual 2.2.3.2, Microsoft name: BrightnessContrastEffect Object */
typedef struct {
    int32_t             Brightness;         //!< -255 to 255, 0 is unchanged, positive increases, negative decreases
    int32_t             Contrast;           //!< -100 to 100, 0 is unchanged, positive increases, negative decreases
} U_PMF_IE_BRIGHTNESSCONTRAST;

/** @brief EMF+ manual 2.2.3.3, Microsoft name: ColorBalanceEffect Object */
typedef struct {
    int32_t             CyanRed;            //!< -100 to 100, 0 is unchanged, positive increases Red   & decreases Cyan,    negative is opposite
    int32_t             MagentaGreen;       //!< -100 to 100, 0 is unchanged, positive increases Green & decreases Magenta, negative is opposite
    int32_t             YellowBlue;         //!< -100 to 100, 0 is unchanged, positive increases Blue  & decreases Yellow,  negative is opposite
} U_PMF_IE_COLORBALANCE;

/** @brief EMF+ manual 2.2.3.4, Microsoft name: ColorCurveEffect Object
        Adjust            Range
        Exposure          -255 to 255, 0 is unchanged
        Density           -255 to 255, 0 is unchanged
        Contrast          -100 to 100, 0 is unchanged
        Highlight         -100 to 100, 0 is unchanged
        Shadow            -100 to 100, 0 is unchanged
        WhiteSaturation   0 to 255
        BlackSaturation   0 to 255
*/
typedef struct {
    uint32_t            Adjust;             //!< CurveAdjustment enumeration
    uint32_t            Channel;            //!< CurveChannel enumeration
    int32_t             Intensity;          //!< adjustment to apply.  "Adjust" determines what field this is and range values.
} U_PMF_IE_COLORCURVE;

/** @brief EMF+ manual 2.2.3.5, Microsoft name: ColorLookupTableEffect Object */
typedef struct {
    uint8_t             BLUT[256];          //!< Blue  color lookup table
    uint8_t             GLUT[256];          //!< Green color lookup table
    uint8_t             RLUT[256];          //!< Red   color lookup table
    uint8_t             ALUT[256];          //!< Alpha color lookup table
} U_PMF_IE_COLORLOOKUPTABLE;

/** @brief EMF+ manual 2.2.3.6, Microsoft name: ColorMatrixEffect Object */
typedef struct {
    U_FLOAT             M[5][5];            //!< 5 x 5 color transformation matrix, First 4 rows are [{4 multiplier values},0.0] for R,G,B,A, last Row is [{4 color translation valuess}, 1.0]
} U_PMF_IE_COLORMATRIX;

/** @brief EMF+ manual 2.2.3.7, Microsoft name: HueSaturationLightnessEffect Object */
typedef struct {
    int32_t             Hue;                //!< -180 to 180, 0 is unchanged
    int32_t             Saturation;         //!< -100 to 100, 0 is unchanged
    int32_t             Lightness;          //!< -100 to 100, 0 is unchanged
} U_PMF_IE_HUESATURATIONLIGHTNESS;

/** @brief EMF+ manual 2.2.3.8, Microsoft name: LevelsEffect Object */
typedef struct {
    int32_t             Highlight;          //!< 0 to 100, 100 is unchanged
    int32_t             Midtone;            //!< -100 to 100, 0 is unchanged
    int32_t             Shadow;             //!< 0 to 100, 0 is unchanged
} U_PMF_IE_LEVELS;

/** @brief EMF+ manual 2.2.3.9, Microsoft name: RedEyeCorrectionEffect Object */
typedef struct {
    int32_t             Elements;           //!< Number of members in Rects
/*  variable part of object, not included in structure
    U_RECTL             Rects[];            //!< Array of rectangular area(s) to apply red eye correction
*/
} U_PMF_IE_REDEYECORRECTION;

/** @brief EMF+ manual 2.2.3.10, Microsoft name: SharpenEffect Object */
typedef struct {
    U_FLOAT             Radius;             //!< Sharpening radius in pixels
    int32_t             Sharpen;            //!< 0 to 100, 0 is unchanged
} U_PMF_IE_SHARPEN;

/** @brief EMF+ manual 2.2.3.11, Microsoft name: TintEffect Object */
typedef struct {
    int32_t             Hue;               //!< -180 to 180, [positive==clockwise] rotation in degrees starting from blue
    int32_t             Amount;            //!< -100 [add black] to 100[add white], 0 is unchanged.  Change in hue on specified axis
} U_PMF_IE_TINT;

/* **************************  EMF+ Records ******************************** */

/** @brief EMF+ manual 2.3.1.1, Microsoft name: EmfPlusOffsetClip Record,  Index 0x35 */
typedef struct {
    U_PMF_CMN_HDR       Header;             //!< Common header (ignore flags)
    U_FLOAT             dX;                 //!< horizontal translation offset to apply to clipping region
    U_FLOAT             dY;                 //!< vertical   translation offset to apply to clipping region
} U_PMF_OFFSETCLIP;

/**  @brief U_PMF_RESETCLIP EMF+ manual 2.3.1.2, Microsoft name: EmfPlusResetClip Record, Index 0x31 */
typedef struct {
    U_PMF_CMN_HDR       Header;             //!< Common header (ignore flags)
} U_PMF_RESETCLIP;

/**  @brief EMF+ manual 2.3.1.3, Microsoft name: EmfPlusSetClipPath Record, Index 0x33

flags (LITTLE endian here, manual uses BIG endian)
   bits 8-11   CombineMode enumeration
   bits 0-7    Index of an U_PMF_PATH object in the EMF+ object table (0-63, inclusive)
*/
typedef struct {
    U_PMF_CMN_HDR       Header;             //!< Common header
} U_PMF_SETCLIPPATH;

/**  @brief EMF+ manual 2.3.1.4, Microsoft name: EmfPlusSetClipRect Record, Index 0x32 

flags (LITTLE endian here, manual uses BIG endian)
   bits 8-11  CombineMode enumeration
*/
typedef struct {
    U_PMF_CMN_HDR       Header;             //!< Common header
    U_PMF_RECTF         Rect;               //!< Rectangle used with CombineMode enumeration from Header.Flags
} U_PMF_SETCLIPRECT;

/**  @brief EMF+ manual 2.3.1.5, Microsoft name: EmfPlusSetClipRegion Record, Index 0x34 

        flags (LITTLE endian here, manual uses BIG endian)
           bits 12-15  CombineMode enumeration
           bits 0-7    Index of an U_PMF_REGION object in the EMF+ object table (0-63, inclusive)
*/
typedef struct {
    U_PMF_CMN_HDR       Header;             //!< Common header
} U_PMF_SETCLIPREGION;

/**  @brief EMF+ manual 2.3.2.1, Microsoft name: EmfPlusComment Record, Index 0x03 

          variable part of record, not included in structure
            char                data[];             //!< Private data, may be anything
*/
typedef struct {
    U_PMF_CMN_HDR       Header;             //!< Common header (ignore flags and set to zero)
} U_PMF_COMMENT;

/**  @brief EMF+ manual 2.3.3.1, Microsoft name: EmfPlusEndOfFile Record, Index 0x02 */
typedef struct {
    U_PMF_CMN_HDR       Header;             //!< Common header  (ignore flags and set to zero)
} U_PMF_ENDOFFILE;

/**  @brief EMF+ manual 2.3.3.2, Microsoft name: EmfPlusGetDC Record, Index 0x04 */
typedef struct {
    U_PMF_CMN_HDR       Header;             //!< Common header  (ignore flags and set to zero)
} U_PMF_GETDC;

/**  @brief EMF+ manual 2.3.3.3, Microsoft name: EmfPlusHeader Record, Index 0x01 

        flags (LITTLE endian here, manual uses BIG endian)
           bit 0    Set indicates "dual mode" EMF+
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header  (ignore flags and set to zero)
    U_PMF_GRAPHICSVERSION Version;          //!< EmfPlusGraphicsVersion object
    uint32_t              EmfPlusFlags;     //!< Reference device 0 bit: set = video device, clear= printer.  Ignore all other bits.
    uint32_t              LogicalDpiX;      //!< Horizontal resolution reference device in DPI
    uint32_t              LogicalDpiY;      //!< Vertical   resolution reference device in DPI
} U_PMF_HEADER;

/** @brief  EMF+ manual 2.3.4.1, Microsoft name: EmfPlusClear Record, Index 0x09 */
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header  (ignore flags and set to zero)
    U_PMF_ARGB            Color;            //!< Erase everything preceding, set background ARGB color.
} U_PMF_CLEAR;

/**  @brief EMF+ manual 2.3.4.2, Microsoft name: EmfPlusDrawArc Record, Index 0x12

        flags (LITTLE endian here, manual uses BIG endian)
           bit  9      U_PPF_C Set: Rect is U_PMF_RECT; Clear: Rect is U_PMF_RECTF
           bits 0-7    Index of an U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    U_FLOAT               Start;            //!< Start angle, >=0.0, degrees clockwise from 3:00
    U_FLOAT               Sweep;            //!< Sweep angle, -360<= angle <=360, degrees clockwise from Start
/*  variable part of record, not included in structure
    U_RECT or U_RECTF     Rect;             //!< Bounding box for elliptical arc being drawn.
*/
} U_PMF_DRAWARC;

/**  @brief  EMF+ manual 2.3.4.3, Microsoft name: EmfPlusDrawBeziers Record, Index 0x19 
        flags (LITTLE endian here, manual uses BIG endian)
           bit  9      U_PPF_C Set: int16_t coordinates; Clear: U_FLOAT coordinates (IGNORE if bit 4 is set)
           bit 12      U_PPF_P Set: Coordinates are relative; Clear: they are absolute
           bits 0-7    Index of an U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
           
           bit1 bit4  Type of Data
           1    0     U_EMF_POINT
           0    0     U_EMF_POINTF
           0    1     U_EMF_POINTR
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    uint32_t              Elements;         //!< Number of members in the Points array
/*
  variable part of record, not included in structure
    (varies)              Points;           //!< Points, for type see table above
*/
} U_PMF_DRAWBEZIERS;

/** @brief EMF+ manual 2.3.4.4, Microsoft name: EmfPlusDrawClosedCurve Record, Index 0x17

        flags (LITTLE endian here, manual uses BIG endian)
           bit  9      U_PPF_C Set: int16_t coordinates; Clear: U_FLOAT coordinates (IGNORE if bit 4 is set)
           bit 12      U_PPF_P Set: Coordinates are relative; Clear: they are absolute
           bits 0-7    Index of an U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
           
           bit1 bit4  Type of Data
           1    0     U_EMF_POINT
           0    0     U_EMF_POINTF
           0    1     U_EMF_POINTR
   
Curve is a cardinal spline.
References sent by MS support:

http://alvyray.com/Memos/CG/Pixar/spline77.pdf
http://msdn.microsoft.com/en-us/library/4cf6we5y(v=vs.110).aspx

*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    U_FLOAT               Tension;          //!< Controls splines, 0 is straight line, >0 is curved
/*
  variable part of record, not included in structure
    (varies)              Points;           //!< Points, for type see table above
*/
} U_PMF_DRAWCLOSEDCURVE;

/** @brief EMF+ manual 2.3.4.5, Microsoft name: EmfPlusDrawCurve Record, Index 0x18
        flags (LITTLE endian here, manual uses BIG endian)
           bit  9      U_PPF_C Set: int16_t coordinates; Clear: U_FLOAT coordinates (IGNORE if bit 4 is set)
           bits 0-7    Index of an U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
           
           bit1 Type of Data
           1    U_EMF_POINT
           0    U_EMF_POINTF

Curve is a cardinal spline, using doubled terminator points to generate curves for the terminal segments.
References sent by MS support:

http://alvyray.com/Memos/CG/Pixar/spline77.pdf
http://msdn.microsoft.com/en-us/library/4cf6we5y(v=vs.110).aspx
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    U_FLOAT               Tension;          //!< Controls splines, 0 is straight line, >0 is curved
    uint32_t              Offset;           //!< Element in Points that is the spline's starting point
    uint32_t              NSegs;            //!< Number of segments
    uint32_t              Elements;         //!< Number of members in Points array
/*
  variable part of record, not included in structure
    (varies)              Points;           //!< Points, for type see table above
*/
} U_PMF_DRAWCURVE;

/** @brief EMF+ manual 2.3.4.6, Microsoft name: EmfPlusDrawDriverString Record, Index 0x36

        flags (LITTLE endian here, manual uses BIG endian)
.          bit 15      U_PPF_B Set: BrushID is an U_PFM_ARGB; Clear: is index of U_PMF_BRUSH object in EMF+ object table
           bits 0-7    Index of an U_PMF_FONT object in the EMF+ object table (0-63, inclusive)
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    uint32_t              BrushID;          //!< Color or index to Brush object, depends on Flags bit0
    uint32_t              DSOFlags;         //!< DriverStringOptions flags
    uint32_t              HasMatrix;        //!< If 1 record contains a TransformMatrix field, if 0 it does not.
    uint32_t              Elements;         //!< Number of members in Glyphs and Positions array
/*
  variable part of record, not included in structure
    uint16_t              Glyphs;           //!< If U_DSO_CmapLookup is set in DSOFlags this is an array 
                                                 of UTF16LE characters, otherwise, it is an array of indices into the U_PMF_FONT 
                                                 object indexed by Object_ID in flags.
    U_PMF_POINTF          Positions;        //!< Coordinates of each member of Glyphs.  U_DSO_RealizedAdvance set in DSOFlags
                                                 Relative then positions are calculated relative to the first glyph which is stored
                                                 in Positions, otherwise, all glyph positions are stored in Positions.
    U_PMF_TRANSFORMMATRIX  Matrix;          //!< Transformation to apply to Glyphs & Positions. Present if HasMatrix is 1
*/
} U_PMF_DRAWDRIVERSTRING;

/** @brief EMF+ manual 2.3.4.7, Microsoft name: EmfPlusDrawEllipse Record, Index 0x0F

        flags (LITTLE endian here, manual uses BIG endian)
           bit  9      U_PPF_C Set: Rect is U_PMF_RECT; Clear: Rect is U_PMF_RECTF
           bits 0-7    Index of an U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
/*
  variable part of record, not included in structure
    (varies)              Rect;             //!< Bounding rectangle, data type set by bit1 of Header.Flags
*/
} U_PMF_DRAWELLIPSE;

/** @brief EMF+ manual 2.3.4.8, Microsoft name: EmfPlusDrawImage Record, Index 0x1A

        flags (LITTLE endian here, manual uses BIG endian)
           bit  9      U_PPF_C Set: DstRect is U_PMF_RECT; Clear: DstRect is U_PMF_RECTF
           bits 0-7    Index of an U_PMF_Image object in the EMF+ object table (0-63, inclusive)
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    uint32_t              ImgAttrID;        //!< EmfPlusImageAttributes object 
    int32_t               SrcUnit;          //!< UnitType enumeration
    U_PMF_RECTF           SrcRect;          //!< Region of image 
/*
  variable part of record, not included in structure
    (varies)              DstRect;          //!< Destination rectangle for image.  Type controlled by bit1 of Header.Flags
*/
} U_PMF_DRAWIMAGE;

/** @brief EMF+ manual 2.3.4.9, Microsoft name: EmfPlusDrawImagePoints Record, Index 0x1B

        flags (LITTLE endian here, manual uses BIG endian)
           bit 14      U_PPF_C Set: Points is U_PMF_POINT; Clear: Points is U_PMF_POINTF
           bit 13      U_PPF_E Set: effect from previous U_PMF_SERIALIZABLEOBJECT record will be applied; Clear: no effect applied
           bit 11      U_PPF_P Set: Points has relative coordinates; Clear: Points has absolute coordinates
           bits 0-7    Index of an U_PMF_Image object in the EMF+ object table (0-63, inclusive)
           
           bit1 bit4  Type of Data
           1    0     U_EMF_POINT
           0    0     U_EMF_POINTF
           0    1     U_EMF_POINTR 
   
   WARNING!  Windows XP Preview does not show filter effects, whether or not U_PPF_E is set.   They are visible if the EMF+
   file is inserted as an image into PowerPoint.
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    uint32_t              ImgAttrID;        //!< EmfPlusImageAttributes object 
    int32_t               SrcUnit;          //!< UnitType enumeration
    U_PMF_RECTF           SrcRect;          //!< Region of image 
    uint32_t              Elements;         //!< Number of members in Points, must be 3
/*
  variable part of record, not included in structure
    (varies)              Points;           //!< 3 points of a parallelogram.  Type from bit1 and bit4 of Header.Flags, see table above
*/
} U_PMF_DRAWIMAGEPOINTS;

/** @brief EMF+ manual 2.3.4.10, Microsoft name: EmfPlusDrawLines Record, Index 0x0D

        flags (LITTLE endian here, manual uses BIG endian)
           bit  9      U_PPF_C Set: Points is U_PMF_POINT; Clear: Points is U_PMF_POINTF
           bit 10      U_PPF_D Set: close shape by connecting last point to first; Clear: leave path open
           bit 12      U_PPF_P Set: Points has relative coordinates; Clear: Points has absolute coordinates
           bits 0-7    Index of an U_PMF_Image object in the EMF+ object table (0-63, inclusive)
           
           bit1 bit4  Type of Data
           1    0     U_EMF_POINT
           0    0     U_EMF_POINTF
           0    1     U_EMF_POINTR
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    uint32_t              Elements;         //!< Number of members in Points
/*
  variable part of record, not included in structure
    (varies)              Points;           //!< Sequence of points to connect with line segments.  Type from bit1 and bit4 of Header.Flags, see table above
*/
} U_PMF_DRAWLINES;

/** @brief EMF+ manual 2.3.4.11, Microsoft name: EmfPlusDrawPath Record, Index 0x15

        flags (LITTLE endian here, manual uses BIG endian)
           bits 0-7    Index of an U_PMF_PATH object in the EMF+ object table (0-63, inclusive)
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    uint32_t              PenID;            //!< U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
} U_PMF_DRAWPATH;

/** @brief EMF+ manual 2.3.4.12, Microsoft name: EmfPlusDrawPie Record, Index 0x0D

        flags (LITTLE endian here, manual uses BIG endian)
           bit  9      U_PPF_C Set: Rect is U_PMF_RECT; Clear: Rect is U_PMF_RECTF
           bits 0-7    Index of an U_PMF_Pen object in the EMF+ object table (0-63, inclusive)
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    U_FLOAT               Start;            //!< Start angle, >=0.0, degrees clockwise from 3:00
    U_FLOAT               Sweep;            //!< Sweep angle, -360<= angle <=360, degrees clockwise from Start
/*
  variable part of record, not included in structure
    U_RECT or U_RECTF     Rect;             //!< Bounding box for elliptical pie segment being drawn.  Type from bit1 of Header.Flags, see above
*/
} U_PMF_DRAWPIE;

/** @brief EMF+ manual 2.3.4.13, Microsoft name: EmfPlusDrawRects Record, Index 0x0B

        flags (LITTLE endian here, manual uses BIG endian)
           bit  9      U_PPF_C Set: Rect is U_PMF_RECT; Clear: Rect is U_PMF_RECTF
           bits 0-7    Index of an U_PMF_Pen object in the EMF+ object table (0-63, inclusive)
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    uint32_t              Elements;         //!< Number of members in Rects
/*
  variable part of record, not included in structure
    U_RECT or U_RECTF     Rects;            //!< Array of rectangles to draw.  Type from bit1 of Header.Flags, see above
*/
} U_PMF_DRAWRECTS;

/** @brief EMF+ manual 2.3.4.14, Microsoft name: EmfPlusDrawString Record, Index 0x1C

        flags (LITTLE endian here, manual uses BIG endian)
           bit 15      U_PPF_B Set: BrushID is an U_PFM_ARGB; Clear: is index of U_PMF_BRUSH object in EMF+ object table.
           bits 0-7    Index of an U_PMF_FONT object in the EMF+ object table (0-63, inclusive)
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    uint32_t              BrushID;          //!< Color or index to Brush object, depends on Flags bit0
    uint32_t              FormatID;         //!< U_PMF_STRINGFORMAT object in EMF+ Object Table.
    uint32_t              Length;           //!< Number of characters in the string.
    U_PMF_RECTF           Rect;             //!< String's bounding box.
/*
  variable part of record, not included in structure
    uint16_t              String;           //!< Array of UFT-16LE unicode characters.
*/
} U_PMF_DRAWSTRING;

/** @brief EMF+ manual 2.3.4.15, Microsoft name: EmfPlusFillClosedCurve Record, Index 0x16

        flags (LITTLE endian here, manual uses BIG endian)
           bit 15      U_PPF_B Set: BrushID is an U_PFM_ARGB; Clear: is index of U_PMF_BRUSH object in EMF+ object table.
           bit  9      U_PPF_C Set: Points is U_PMF_POINT; Clear: Points is U_PMF_POINTF
           bit 10      U_PPF_F Set: winding fill; Clear: alternate fill
           bit 12      U_PPF_P Set: Points has relative coordinates; Clear: Points has absolute coordinates
           
           bit1 bit4  Type of Data
           1    0     U_EMF_POINT
           0    0     U_EMF_POINTF
           0    1     U_EMF_POINTR
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    uint32_t              BrushID;          //!< Color or index to Brush object, depends on Header.Flags bit0
    U_FLOAT               Tension;          //!< Controls splines, 0 is straight line, >0 is curved
    uint32_t              Elements;         //!< Number of members in Points
/*
  variable part of record, not included in structure
    (varies)              Points;           //!< Sequence of points to connect.  Type from bit1 and bit4 of Header.Flags, see table above
*/
} U_PMF_FILLCLOSEDCURVE;

/** @brief EMF+ manual 2.3.4.16, Microsoft name: EmfPlusFillEllipse Record, Index 0x0E

        flags (LITTLE endian here, manual uses BIG endian)
           bit 15      U_PPF_B Set: BrushID is an U_PFM_ARGB; Clear: is index of U_PMF_BRUSH object in EMF+ object table.
           bit  9      U_PPF_C Set: Rect is U_PMF_RECT; Clear: Rect is U_PMF_RECTF
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    uint32_t              BrushID;          //!< Color or index to Brush object, depends on Header.Flags bit0
/*
  variable part of record, not included in structure
    U_RECT or U_RECTF     Rect;             //!< Bounding box for elliptical pie segment being drawn.  Type from bit1 of Header.Flags, see above
*/
} U_PMF_FILLELLIPSE;

/** @brief EMF+ manual 2.3.4.17, Microsoft name: EmfPlusFillPath Record, Index 0x14
Note: U_PMF_FILLPATHOBJ is the object, U_PMF_FILLPATH is the file record

        flags (LITTLE endian here, manual uses BIG endian)
           bit 15      U_PPF_B Set: BrushID is an U_PFM_ARGB; Clear: is index of U_PMF_BRUSH object in EMF+ object table.
           bits 0-7    Index of an U_PMF_PATH object in the EMF+ object table (0-63, inclusive)
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    uint32_t              BrushID;          //!< Color or index to Brush object, depends on Header.Flags bit0
} U_PMF_FILLPATH;

/** @brief EMF+ manual 2.3.4.18, Microsoft name: EmfPlusFillPie Record, Index 0x10

        flags (LITTLE endian here, manual uses BIG endian)
           bit 15      U_PPF_B Set: BrushID is an U_PFM_ARGB; Clear: is index of U_PMF_BRUSH object in EMF+ object table.
           bit  9      U_PPF_C Set: Rect is U_PMF_RECT; Clear: Rect is U_PMF_RECTF
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    uint32_t              BrushID;          //!< Color or index to Brush object, depends on Header.Flags bit0
    U_FLOAT               Start;            //!< Start angle, >=0.0, degrees clockwise from 3:00
    U_FLOAT               Sweep;            //!< Sweep angle, -360<= angle <=360, degrees clockwise from Start
/*
  variable part of record, not included in structure
    U_RECT or U_RECTF     Rect;             //!< Bounding box for elliptical pie segment being filled.  Type from bit1 of Header.Flags, see above
*/
} U_PMF_FILLPIE;

/** @brief EMF+ manual 2.3.4.19, Microsoft name: EmfPlusFillPolygon Record, Index 0x0C

        flags (LITTLE endian here, manual uses BIG endian)
           bit 15      U_PPF_B Set: BrushID is an U_PFM_ARGB; Clear: is index of U_PMF_BRUSH object in EMF+ object table.
           bit  9      U_PPF_C Set: Points is U_PMF_POINT; Clear: Points is U_PMF_POINTF
           bit 12      U_PPF_P Set: Points has relative coordinates; Clear: Points has absolute coordinates
           
           bit1 bit4  Type of Data
           1    0     U_EMF_POINT
           0    0     U_EMF_POINTF
           0    1     U_EMF_POINTR
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    uint32_t              BrushID;          //!< Color or index to Brush object, depends on Header.Flags bit0
    uint32_t              Elements;         //!< Number of members in Points
/*
  variable part of record, not included in structure
    (varies)              Points;           //!< Sequence of points to connect with line segments.  Type from bit1 and bit4 of Header.Flags, see table above
*/
} U_PMF_FILLPOLYGON;

/** @brief EMF+ manual 2.3.4.20, Microsoft name: EmfPlusFillRects Record, Index 0x0A

        flags (LITTLE endian here, manual uses BIG endian)
           bit 15      U_PPF_B Set: BrushID is an U_PFM_ARGB; Clear: is index of U_PMF_BRUSH object in EMF+ object table.
           bit  9      U_PPF_C Set: Rect is U_PMF_RECT; Clear: Rect is U_PMF_RECTF
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    uint32_t              BrushID;          //!< Color or index to Brush object, depends on Header.Flags bit0
    uint32_t              Elements;         //!< Number of members in Rects
/*
  variable part of record, not included in structure
    U_RECT or U_RECTF     Rects;            //!< Array of rectangles to draw.  Type from bit1 of Header.Flags, see above
*/
} U_PMF_FILLRECTS;

/** @brief EMF+ manual 2.3.4.21, Microsoft name: EmfPlusFillRegion Record, Index 0x13

        flags (LITTLE endian here, manual uses BIG endian)
           bit 15      U_PPF_B Set: BrushID is an U_PFM_ARGB; Clear: is index of U_PMF_BRUSH object in EMF+ object table.
           bits 0-7    Index of an U_PMF_REGION object in the EMF+ object table (0-63, inclusive)
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    uint32_t              BrushID;          //!< Color or index to Brush object, depends on Header.Flags bit0
} U_PMF_FILLREGION;

/** @brief EMF+ manual 2.3.5.1, Microsoft name: EmfPlusObject Record, Index 0x13

        flags (LITTLE endian here, manual uses BIG endian)
           bit  15      U_PPF_N Set: object definition continues in next record; Clear: this is the final object definition record
           bits 8-14    Type of object being created, ObjectType enumeration
           bits 0-7    Index of an U_PMF_REGION object in the EMF+ object table (0-63, inclusive)
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
/* 
  variable part of record, not included in structure
    uint8_t               Data;             //!< Object's data.  Type from bits1-7 and bits8-15 of Header.Flags, see above
*/
} U_PMF_OBJECT;

/** @brief EMF+ manual 2.3.5.2, Microsoft name: EmfPlusSerializableObject Record, Index 0x38 

   WARNING!  Windows XP Preview does not show filter effects, whether or not U_PPF_E is set.   They are visible if the EMF+
   file is inserted as an image into PowerPoint.

*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header  (ignore flags and set to zero)
    uint8_t               GUID[16];         //!< ImageEffects identifier. Composed of Data1[4]-Data2[2]-Data3[2]-Data4[8]
                                            //!< Data1-Data3 are stored as little endian integers.  Data4 is stored big endian (like a string)
    uint32_t              Size;             //!< Bytes in Data.
/*
  variable part of record, not included in structure
    uint8_t               Data;             //!< "Serialized image effects parameter block".  One of the ImageEffects objects.
*/
} U_PMF_SERIALIZABLEOBJECT;

/** @brief EMF+ manual 2.3.6.1, Microsoft name: EmfPlusSetAntiAliasMode Record, Index 0x1E

        flags (LITTLE endian here, manual uses BIG endian)
           bits 7-1    SmoothingMode enumeration
           bit  0      U_PPF_AA Set: anti-aliasing on; Clear: anti-aliasing off
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
} U_PMF_SETANTIALIASMODE;

/** @brief EMF+ manual 2.3.6.2, Microsoft name: EmfPlusSetCompositingMode Record, Index 0x23

        flags (LITTLE endian here, manual uses BIG endian)
           bits 0-7    CompositingMode enumeration
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
} U_PMF_SETCOMPOSITINGMODE;

/** @brief EMF+ manual 2.3.6.3, Microsoft name: EmfPlusSetCompositingQuality Record, Index 0x24

        flags (LITTLE endian here, manual uses BIG endian)
           bits 0-7    CompositingQuality enumeration
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
} U_PMF_SETCOMPOSITINGQUALITY;

/** @brief EMF+ manual 2.3.6.4, Microsoft name: EmfPlusSetInterpolationMode Record, Index 0x21

        flags (LITTLE endian here, manual uses BIG endian)
           bits 0-7    InterpolationMode enumeration
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
} U_PMF_SETINTERPOLATIONMODE;

/** @brief EMF+ manual 2.3.6.5, Microsoft name: EmfPlusSetPixelOffsetMode Record, Index 0x22

        flags (LITTLE endian here, manual uses BIG endian)
           bits 0-7    PixelOffsetMode enumeration
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
} U_PMF_SETPIXELOFFSETMODE;

/** @brief EMF+ manual 2.3.6.6, Microsoft name: EmfPlusSetRenderingOrigin Record, Index 0x1D */
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header  (ignore flags and set to zero)
    int32_t               X;                //!< X coordinate of rendering origin
    int32_t               Y;                //!< Y coordinate of rendering origin
} U_PMF_SETRENDERINGORIGIN;

/** @brief EMF+ manual 2.3.6.7, Microsoft name: EmfPlusSetTextContrast Record, Index 0x20

        flags (LITTLE endian here, manual uses BIG endian)
           bits 12-0   1000 x Gamma correction value.  Range 1000-2200 = gamma 1.0-2.2
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
} U_PMF_SETTEXTCONTRAST;

/** @brief EMF+ manual 2.3.6.8, Microsoft name: EmfPlusSetTextRenderingHint Record, Index 0x1F

        flags (LITTLE endian here, manual uses BIG endian)
           bits 0-7    TextRenderingHint enumeration
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
} U_PMF_SETTEXTRENDERINGHINT;

/** @brief EMF+ manual 2.3.7.1, Microsoft name: EmfPlusBeginContainer Record, Index 0x27

        flags (LITTLE endian here, manual uses BIG endian)
           bits 8-15  UnitType enumeration
           bits 0-7   (all zero)
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    U_PMF_RECTF           DstRect;          //!< with SrcRect specifies a transformation
    U_PMF_RECTF           SrcRect;          //!< with DstRect specifies a transformation
    uint32_t              Index;            //!< Index to apply to this graphics container
} U_PMF_BEGINCONTAINER;

/** @brief EMF+ manual 2.3.7.2, Microsoft name: EmfPlusBeginContainerNoParams Record, Index 0x28 */
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header  (ignore flags and set to zero)
    uint32_t              Index;            //!< Index to apply to this graphics container
} U_PMF_BEGINCONTAINERNOPARAMS;

/** @brief EMF+ manual 2.3.7.3, Microsoft name: EmfPlusEndContainer Record, Index 0x29 */
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header  (ignore flags and set to zero)
    uint32_t              Index;            //!< Index of container being closed, from U_PMF_BEGINCONTAINERNOPARAMS or U_PMF_BEGINCONTAINER
} U_PMF_ENDCONTAINER;

/** @brief EMF+ manual 2.3.7.4, Microsoft name: EmfPlusRestore Record, Index 0x26 */
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header  (ignore flags and set to zero)
    uint32_t              Index;            //!< Index of Graphics State being restored, from U_PMF_SAVE
} U_PMF_RESTORE;

/** @brief EMF+ manual 2.3.7.5, Microsoft name: EmfPlusSave Record, Index 0x25 */
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header  (ignore flags and set to zero)
    uint32_t              Index;            //!< Index of Graphics State being saved
} U_PMF_SAVE;

/** @brief EMF+ manual 2.3.8.1, Microsoft name: EmfPlusSetTSClip Record, Index 0x3A

        flags (LITTLE endian here, manual uses BIG endian)
           bit 15      U_PPF_K Set: Rect is U_PMF_RECT; Clear: Rect is U_PMF_RECTF
           bits 14-0   Number of rectangles in Rects field
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
/*
  variable part of record, not included in structure
    U_RECT or U_RECTF     Rects;            //!< Array of rectangles to draw.  Type from bit0 of Header.Flags, see above
*/
} U_PMF_SETTSCLIP;

/** @brief EMF+ manual 2.3.8.2, Microsoft name: EmfPlusSetTSGraphics Record, Index 0x39

        flags (LITTLE endian here, manual uses BIG endian)
           bit   1     U_PPF_VGA Set: Palette is VGA basic colors; Clear: Palette is a U_PMF_PALETTE object.
           bit   0     U_PPF_PP Set: Palette field is present; Clear: Palette field is absent
*/
typedef struct {
    U_PMF_CMN_HDR         Header;              //!< Common header
    uint8_t               AntiAliasMode;       //!< SmoothingMode enumeration
    uint8_t               TextRenderHint;      //!< TextRenderingHint enumeration
    uint8_t               CompositingMode;     //!< CompositingMode enumeration
    uint8_t               CompositingQuality;  //!< CompositingQuality enumeration
    int16_t               RenderOriginX;       //!< Origin X for halftoning and dithering
    int16_t               RenderOriginY;       //!< Origin Y for halftoning and dithering
    uint16_t              TextContrast;        //!< Gamma correction, range 0 to 12
    uint8_t               FilterType;          //!< FilterType enumeraton
    uint8_t               PixelOffset;         //!< PixelOffsetMode enumeration
    U_PMF_TRANSFORMMATRIX WorldToDevice;       //!< world to device transform
/*
  optional part of record, not included in structure
    U_PMF_PALETTE         Palette;            //!< Present if bit15 of Header.Flags is set
*/
} U_PMF_SETTSGRAPHICS;

/** @brief EMF+ manual 2.3.9.1, Microsoft name: EmfPlusMultiplyWorldTransform Record, Index 0x2C

        flags (LITTLE endian here, manual uses BIG endian)
           bit 13      U_PPF_XM Set: Post multiply; Clear: Pre multiply
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    U_PMF_TRANSFORMMATRIX Matrix;           //!< Transformation matrix
} U_PMF_MULTIPLYWORLDTRANSFORM;

/** @brief EMF+ manual 2.3.9.2, Microsoft name: EmfPlusResetWorldTransform Record, Index 0x2B
Sets transformation matrix to identity matrix.
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header  (ignore flags and set to zero)
} U_PMF_RESETWORLDTRANSFORM;

/** @brief EMF+ manual 2.3.9.3, Microsoft name: EmfPlusRotateWorldTransform Record, Index 0x2F
        Construct transformation matrix from Angle:
          sin(Angle)  cos(Angle) 0
          cos(Angle) -sin(Angle) 0
        Multiply this against current world space transform, result becomes new world space transform.

        flags (LITTLE endian here, manual uses BIG endian)
           bit 13      U_PPF_XM Set: Post multiply; Clear: Pre multiply
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    U_FLOAT               Angle;            //!< Rotation angle, in degrees
} U_PMF_ROTATEWORLDTRANSFORM;

/** @brief EMF+ manual 2.3.9.4, Microsoft name: EmfPlusScaleWorldTransform Record, Index 0x2E
        Construct transformation matrix:
          Sx  0  0
          0   Sy 0
        Multiply this against current world space transform, result becomes new world space transform.

        flags (LITTLE endian here, manual uses BIG endian)
           bit 13      U_PPF_XM Set: Post multiply; Clear: Pre multiply
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    U_FLOAT               Sx;               //!< X scale factor
    U_FLOAT               Sy;               //!< Y scale factor
} U_PMF_SCALEWORLDTRANSFORM;

/** @brief EMF+ manual 2.3.9.5, Microsoft name: EmfPlusSetPageTransform Record, Index 0x30
        flags (LITTLE endian here, manual uses BIG endian)
           bits  0-7   UnitType enumeration
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    U_FLOAT               Scale;            //!< Scale factor to convert page space to device space
} U_PMF_SETPAGETRANSFORM;


/** @brief EMF+ manual 2.3.9.6, Microsoft name: EmfPlusSetWorldTransform Record, Index 0x2A */
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header  (ignore flags and set to zero)
    U_PMF_TRANSFORMMATRIX Matrix;           //!< Transformation matrix
} U_PMF_SETWORLDTRANSFORM;

/** @brief EMF+ manual 2.3.9.7, Microsoft name: EmfPlusTranslateWorldTransform Record, Index 0x2D
        Construct transformation matrix:
          1  0  Dx
          0  1  Dy
        Multiply this against current world space transform, result becomes new world space transform.

        flags (LITTLE endian here, manual uses BIG endian)
           bit 13      U_PPF_XM Set: Post multiply; Clear: Pre multiply
*/
typedef struct {
    U_PMF_CMN_HDR         Header;           //!< Common header
    U_FLOAT               Dx;               //!< X offset
    U_FLOAT               Dy;               //!< Y offset
} U_PMF_TRANSLATEWORLDTRANSFORM;

//! \cond

/* EMF+ prototypes (helper functions) */
int U_PMR_write(U_PSEUDO_OBJ *po, U_PSEUDO_OBJ *sum, EMFTRACK *et);
int U_PMR_drawline(uint32_t PenID, uint32_t PathID, U_PMF_POINTF Start, U_PMF_POINTF End, int Dashed, U_PSEUDO_OBJ *sum, EMFTRACK *et);
int U_PMR_drawstring( const char *string, int Vpos, uint32_t FontID, const U_PSEUDO_OBJ *BrushID, uint32_t FormatID,
      U_PMF_STRINGFORMAT  Sfs, const char *FontName, U_FLOAT Height, U_FontInfoParams *fip, uint32_t FontFlags,
      U_FLOAT x, U_FLOAT y, U_PSEUDO_OBJ *sum, EMFTRACK *et);
U_PMF_POINT *POINTF_To_POINT16_LE(U_PMF_POINTF *points, int count);
int U_PMF_LEN_REL715(const char *contents, int Elements);
int U_PMF_LEN_FLOATDATA(const char *contents);
int U_PMF_LEN_BYTEDATA(const char *contents);
int U_PMF_LEN_PENDATA(const char *PenData);
int U_PMF_LEN_OPTPENDATA(const char *PenData, uint32_t Flags);
char *U_PMF_CURLYGUID_set(uint8_t *GUID);
int U_PMF_KNOWNCURLYGUID_set(const char *string);
void U_PMF_MEMCPY_SRCSHIFT(void *Dst, const char **Src, size_t Size);
void U_PMF_MEMCPY_DSTSHIFT(char **Dst, const void *Src, size_t Size);
void U_PMF_REPCPY_DSTSHIFT(char **Dst, const void *Src, size_t Size, size_t Reps);
void U_PMF_PTRSAV_SHIFT(const char **Dst, const char **Src, size_t Size);
uint16_t U_PMF_HEADERFLAGS_get(const char *contents);
int U_PMF_RECORD_SIZE_get(const char *contents);
int U_PMF_CMN_HDR_get(const char **contents, U_PMF_CMN_HDR *Header);
int U_OID_To_OT(uint32_t OID);
int U_OID_To_BT(uint32_t OID);
int U_OID_To_CLCDT(uint32_t OID);
int U_OID_To_IDT(uint32_t OID);
int U_OID_To_RNDT(uint32_t OID);
uint8_t *U_OID_To_GUID(uint32_t OID);
int U_OA_append(U_OBJ_ACCUM *oa, const char *data, int size, int Type, int Id);
int U_OA_clear(U_OBJ_ACCUM *oa);
int U_OA_release(U_OBJ_ACCUM *oa);
U_PSEUDO_OBJ *U_PO_create(char *Data, size_t Size, size_t Use, uint32_t Type);
U_PSEUDO_OBJ *U_PO_append(U_PSEUDO_OBJ *po, const char *Data, size_t Size);
U_PSEUDO_OBJ *U_PO_po_append(U_PSEUDO_OBJ *po, U_PSEUDO_OBJ *src, int StripE);
int U_PO_free(U_PSEUDO_OBJ **po);
U_DPSEUDO_OBJ *U_PATH_create(int Elements, const U_PMF_POINTF *Points, uint8_t First, uint8_t Others);
int U_DPO_free(U_DPSEUDO_OBJ **dpo);
int U_DPO_clear(U_DPSEUDO_OBJ *dpo);
int U_PATH_moveto(U_DPSEUDO_OBJ *path, U_PMF_POINTF Point, uint8_t Flags);
int U_PATH_lineto(U_DPSEUDO_OBJ *path, U_PMF_POINTF Point, uint8_t Flags);
int U_PATH_closepath(U_DPSEUDO_OBJ *path);
int U_PATH_polylineto(U_DPSEUDO_OBJ *path, uint32_t Elements, const U_PMF_POINTF *Points, uint8_t Flags, uint8_t StartSeg);
int U_PATH_polybezierto(U_DPSEUDO_OBJ *path, uint32_t Elements, const U_PMF_POINTF *Points, uint8_t Flags, uint8_t StartSeg);
int U_PATH_polygon(U_DPSEUDO_OBJ *Path, uint32_t Elements, const U_PMF_POINTF *Points, uint8_t Flags);
int U_PATH_arcto(U_DPSEUDO_OBJ *Path, U_FLOAT Start, U_FLOAT Sweep, U_FLOAT Rot, U_PMF_RECTF *Rect, uint8_t Flags, int StartSeg);
U_PMF_POINTF *pointfs_transform(U_PMF_POINTF *points, int count, U_XFORM xform);
U_PMF_RECTF *rectfs_transform(U_PMF_RECTF *rects, int count, U_XFORM xform);
U_PMF_TRANSFORMMATRIX tm_for_gradrect(U_FLOAT Angle, U_FLOAT w, U_FLOAT h, U_FLOAT x, U_FLOAT y, U_FLOAT Periods);
U_PSEUDO_OBJ *U_PMR_drawfill(uint32_t PathID, uint32_t PenID, const U_PSEUDO_OBJ *BrushID);


char *U_pmr_names(unsigned int idx);

/* EMF+ prototypes (objects_set) */

U_PSEUDO_OBJ *U_PMF_BRUSH_set(uint32_t Version, const U_PSEUDO_OBJ *po);
U_PSEUDO_OBJ *U_PMF_CUSTOMLINECAP_set(uint32_t Version, const U_PSEUDO_OBJ *po);
U_PSEUDO_OBJ *U_PMF_FONT_set(uint32_t Version, U_FLOAT EmSize, uint32_t SizeUnit,
      int32_t FSFlags, uint32_t Length, const uint16_t *Font);
U_PSEUDO_OBJ *U_PMF_IMAGE_set(uint32_t Version, const U_PSEUDO_OBJ *po);
U_PSEUDO_OBJ *U_PMF_IMAGEATTRIBUTES_set(uint32_t Version, uint32_t WrapMode, uint32_t ClampColor, uint32_t ObjectClamp);
U_PSEUDO_OBJ *U_PMF_PATH_set(uint32_t Version, const U_PSEUDO_OBJ *Points, const U_PSEUDO_OBJ *Types);
U_PSEUDO_OBJ *U_PMF_PATH_set2(uint32_t Version, const U_DPSEUDO_OBJ *Path);
U_PSEUDO_OBJ *U_PMF_PATH_set3(uint32_t Version, const U_DPSEUDO_OBJ *Path);
U_PSEUDO_OBJ *U_PMF_PEN_set(uint32_t Version, const U_PSEUDO_OBJ *PenData, const U_PSEUDO_OBJ *Brush);
U_PSEUDO_OBJ *U_PMF_REGION_set(uint32_t Version, uint32_t Count, const U_PSEUDO_OBJ *Nodes);
U_PSEUDO_OBJ *U_PMF_STRINGFORMAT_set(U_PMF_STRINGFORMAT *Sfs, const U_PSEUDO_OBJ *Sfd);
U_PSEUDO_OBJ *U_PMF_4NUM_set(uint32_t BrushID);
U_PSEUDO_OBJ *U_PMF_ARGB_set(uint8_t Alpha, uint8_t Red, uint8_t Green, uint8_t Blue);
U_PSEUDO_OBJ *U_PMF_ARGBN_set(uint32_t Count, U_PMF_ARGB *Colors);
U_PMF_ARGB U_PMF_ARGBOBJ_set(uint8_t Alpha, uint8_t Red, uint8_t Green, uint8_t Blue);
U_PSEUDO_OBJ *U_PMF_BITMAP_set(const U_PMF_BITMAP *Bs, const U_PSEUDO_OBJ *Bm);
U_PSEUDO_OBJ *U_PMF_BITMAPDATA_set( const U_PSEUDO_OBJ *Ps, int cbBm, const char *Bm);
U_PSEUDO_OBJ *U_PMF_BLENDCOLORS_set(uint32_t Elements, const U_FLOAT *Positions, const U_PSEUDO_OBJ *Colors);
U_PSEUDO_OBJ *U_PMF_BLENDCOLORS_linear_set(uint32_t Elements,U_PMF_ARGB StartColor, U_PMF_ARGB EndColor);
U_PSEUDO_OBJ *U_PMF_BLENDFACTORS_set(uint32_t Elements, const U_FLOAT *Positions, const U_FLOAT *Factors);
U_PSEUDO_OBJ *U_PMF_BLENDFACTORS_linear_set(uint32_t Elements, U_FLOAT StartFactor, U_FLOAT EndFactor);
U_PSEUDO_OBJ *U_PMF_BOUNDARYPATHDATA_set(const U_PSEUDO_OBJ *Path);
U_PSEUDO_OBJ *U_PMF_BOUNDARYPOINTDATA_set(uint32_t Elements, const U_PMF_POINTF *Points);
U_PSEUDO_OBJ *U_PMF_CHARACTERRANGE_set(int32_t First, int32_t Length);
U_PSEUDO_OBJ *U_PMF_COMPOUNDLINEDATA_set(int32_t Elements, const char *Widths);
U_PSEUDO_OBJ *U_PMF_COMPRESSEDIMAGE_set(int32_t cbImage, const char *Image);
U_PSEUDO_OBJ *U_PMF_CUSTOMENDCAPDATA_set(const U_PSEUDO_OBJ *Clc);
U_PSEUDO_OBJ *U_PMF_CUSTOMLINECAPARROWDATA_set(U_FLOAT Width, U_FLOAT Height, 
      U_FLOAT MiddleInset, uint32_t FillState, uint32_t StartCap, uint32_t EndCap, uint32_t Join,
      U_FLOAT MiterLimit, U_FLOAT WidthScale);
U_PSEUDO_OBJ *U_PMF_CUSTOMLINECAPDATA_set(uint32_t Flags, uint32_t Cap, 
      U_FLOAT Inset, uint32_t StartCap, uint32_t EndCap, 
      uint32_t Join, U_FLOAT MiterLimit, U_FLOAT WidthScale, 
      const U_PSEUDO_OBJ *Clcod);
U_PSEUDO_OBJ *U_PMF_CUSTOMLINECAPOPTIONALDATA_set(const U_PSEUDO_OBJ *Fill, const U_PSEUDO_OBJ *Line);
U_PSEUDO_OBJ *U_PMF_CUSTOMSTARTCAPDATA_set(const U_PSEUDO_OBJ *Clc);
U_PSEUDO_OBJ *U_PMF_DASHEDLINEDATA_set(int32_t Elements, const U_FLOAT *Lengths);
U_PSEUDO_OBJ *U_PMF_DASHEDLINEDATA_set2(U_FLOAT Unit, int StdPat);
U_PSEUDO_OBJ *U_PMF_DASHEDLINEDATA_set3(U_FLOAT Unit, uint32_t BitPat);
U_PSEUDO_OBJ *U_PMF_FILLPATHOBJ_set(const U_PSEUDO_OBJ *Path);
U_PSEUDO_OBJ *U_PMF_FOCUSSCALEDATA_set(U_FLOAT ScaleX, U_FLOAT ScaleY);
U_PSEUDO_OBJ *U_PMF_GRAPHICSVERSION_set(int GrfVersion);
U_PMF_GRAPHICSVERSION U_PMF_GRAPHICSVERSIONOBJ_set(int GrfVersion);
U_PSEUDO_OBJ *U_PMF_HATCHBRUSHDATA_set(uint32_t Style, const U_PSEUDO_OBJ *Fg, const U_PSEUDO_OBJ *Bg);
U_PSEUDO_OBJ *U_PMF_INTEGER7_set(int value);
U_PSEUDO_OBJ *U_PMF_INTEGER15_set(int value);
U_PMF_LANGUAGEIDENTIFIER U_PMF_LANGUAGEIDENTIFIEROBJ_set(int SubLId, int PriLId);
U_PSEUDO_OBJ *U_PMF_LANGUAGEIDENTIFIER_set(U_PMF_LANGUAGEIDENTIFIER LId);
U_PSEUDO_OBJ *U_PMF_LINEARGRADIENTBRUSHDATA_set(const U_PMF_LINEARGRADIENTBRUSHDATA *Lgbd, const U_PSEUDO_OBJ *Lgbod);
U_PSEUDO_OBJ *U_PMF_LINEARGRADIENTBRUSHOPTIONALDATA_set(uint32_t *Flags, const U_PSEUDO_OBJ *Tm, const U_PSEUDO_OBJ *Bc, const U_PSEUDO_OBJ *BfH, const U_PSEUDO_OBJ *BfV);
U_PSEUDO_OBJ *U_PMF_LINEPATH_set(const U_PSEUDO_OBJ *Path);
U_PSEUDO_OBJ *U_PMF_METAFILE_set(void);
U_PSEUDO_OBJ *U_PMF_PALETTE_set(uint32_t Flags, uint32_t Elements, const U_PMF_ARGB *Pd);
U_PSEUDO_OBJ *U_PMF_PATHGRADIENTBRUSHDATA_set(uint32_t Flags, int32_t WrapMode, U_PMF_ARGB CenterColor, 
      U_PMF_POINTF Center, const U_PSEUDO_OBJ *Gradient, const U_PSEUDO_OBJ *Boundary,  const U_PSEUDO_OBJ *Data);
U_PSEUDO_OBJ *U_PMF_PATHGRADIENTBRUSHOPTIONALDATA_set(uint32_t Flags,
      const U_PSEUDO_OBJ *Tm, const U_PSEUDO_OBJ *Pd, const U_PSEUDO_OBJ *Fsd);
U_PSEUDO_OBJ *U_PMF_PATHPOINTTYPE_set(uint32_t Elements, const uint8_t *Ppt);
U_PSEUDO_OBJ *U_PMF_PATHPOINTTYPE_set2(uint32_t Elements, uint8_t Start, uint8_t Others);
U_PSEUDO_OBJ *U_PMF_PATHPOINTTYPERLE_set(uint32_t Elements, const uint8_t *Bz, const uint8_t *RL, const uint8_t *Ppte);
U_PSEUDO_OBJ *U_PMF_PENDATA_set(uint32_t Unit, U_FLOAT Width, const U_PSEUDO_OBJ *Pod);
U_PSEUDO_OBJ *U_PMF_PENOPTIONALDATA_set(uint32_t Flags, U_PSEUDO_OBJ *Tm, int32_t StartCap, int32_t EndCap, uint32_t Join,
      U_FLOAT MiterLimit, int32_t Style, int32_t DLCap, U_FLOAT DLOffset,
      U_PSEUDO_OBJ *DLData, int32_t PenAlignment, U_PSEUDO_OBJ *CmpndLineData, U_PSEUDO_OBJ *CSCapData,
      U_PSEUDO_OBJ *CECapData);
U_PSEUDO_OBJ *U_PMF_POINT_set(uint32_t Elements, const U_PMF_POINT *Coords);
U_PSEUDO_OBJ *U_PMF_POINTF_set(uint32_t Elements, const U_PMF_POINTF *Coords);
U_PSEUDO_OBJ *U_PMF_POINTR_set(uint32_t Elements, const U_PMF_POINTF *Coords);
U_PSEUDO_OBJ *U_PMF_RECT4_set(int16_t X, int16_t Y, int16_t Width, int16_t Height);
U_PSEUDO_OBJ *U_PMF_RECT_set(U_PMF_RECT *Rect);
U_PSEUDO_OBJ *U_PMF_RECTN_set(uint32_t Elements, U_PMF_RECT *Rects);
U_PSEUDO_OBJ *U_PMF_RECTF4_set(U_FLOAT X, U_FLOAT Y, U_FLOAT Width, U_FLOAT Height);
U_PSEUDO_OBJ *U_PMF_RECTF_set(U_PMF_RECTF *Rect);
U_PSEUDO_OBJ *U_PMF_RECTFN_set(uint32_t Elements, U_PMF_RECTF *Rects);
U_PSEUDO_OBJ *U_PMF_REGIONNODE_set(int32_t Type, const U_PSEUDO_OBJ *Rnd);
U_PSEUDO_OBJ *U_PMF_REGIONNODECHILDNODES_set(const U_PSEUDO_OBJ *Left, const U_PSEUDO_OBJ *Right);
U_PSEUDO_OBJ *U_PMF_REGIONNODEPATH_set(const U_PSEUDO_OBJ *Path);
U_PSEUDO_OBJ *U_PMF_SOLIDBRUSHDATA_set(const U_PSEUDO_OBJ *Color);
U_PSEUDO_OBJ *U_PMF_STRINGFORMATDATA_set(uint32_t TabStopCount, U_FLOAT *TabStops, const U_PSEUDO_OBJ *Ranges);
U_PSEUDO_OBJ *U_PMF_TEXTUREBRUSHDATA_set(uint32_t Flags, uint32_t WrapMode, const U_PSEUDO_OBJ *Tbod);
U_PSEUDO_OBJ *U_PMF_TEXTUREBRUSHOPTIONALDATA_set(const U_PSEUDO_OBJ *Tm, const U_PSEUDO_OBJ *Image);
U_PSEUDO_OBJ *U_PMF_TRANSFORMMATRIX_set(U_PMF_TRANSFORMMATRIX *Tm);
U_PSEUDO_OBJ *U_PMF_IE_BLUR_set(U_FLOAT Radius, uint32_t ExpandEdge);
U_PSEUDO_OBJ *U_PMF_IE_BRIGHTNESSCONTRAST_set(int32_t Brightness, int32_t Contrast);
U_PSEUDO_OBJ *U_PMF_IE_COLORBALANCE_set(int32_t CyanRed, int32_t MagentaGreen, int32_t YellowBlue);
U_PSEUDO_OBJ *U_PMF_IE_COLORCURVE_set(uint32_t Adjust, uint32_t Channel, int32_t Intensity);
U_PSEUDO_OBJ *U_PMF_IE_COLORLOOKUPTABLE_set(const uint8_t *BLUT, const uint8_t *GLUT, const uint8_t *RLUT, const uint8_t *ALUT);
U_PSEUDO_OBJ *U_PMF_IE_COLORMATRIX_set(const U_FLOAT *Matrix);
U_PSEUDO_OBJ *U_PMF_IE_HUESATURATIONLIGHTNESS_set(int32_t Hue, int32_t Saturation, int32_t Lightness);
U_PSEUDO_OBJ *U_PMF_IE_LEVELS_set(int32_t Highlight, int32_t Midtone, int32_t Shadow);
U_PSEUDO_OBJ *U_PMF_IE_REDEYECORRECTION_set(uint32_t Elements, const U_RECTL *rects);
U_PSEUDO_OBJ *U_PMF_IE_SHARPEN_set(U_FLOAT Radius, int32_t Sharpen);
U_PSEUDO_OBJ *U_PMF_IE_TINT_set(int32_t Hue, int32_t Amount);
U_PSEUDO_OBJ *U_PMR_SERIALIZABLEOBJECT_set(const U_PSEUDO_OBJ *Siepb);

/* EMF+ prototypes (records_set) */

U_PSEUDO_OBJ *U_PMR_OFFSETCLIP_set(U_FLOAT dX, U_FLOAT dY);
U_PSEUDO_OBJ *U_PMR_RESETCLIP_set(void);
U_PSEUDO_OBJ *U_PMR_SETCLIPPATH_set(uint32_t PathID, uint32_t CMenum);
U_PSEUDO_OBJ *U_PMR_SETCLIPRECT_set(uint32_t CMenum, const U_PSEUDO_OBJ *Rect);
U_PSEUDO_OBJ *U_PMR_SETCLIPREGION_set(uint32_t PathID, uint32_t CMenum);
U_PSEUDO_OBJ *U_PMR_COMMENT_set(size_t cbData, const void *Data);
U_PSEUDO_OBJ *U_PMR_ENDOFFILE_set(void);
U_PSEUDO_OBJ *U_PMR_GETDC_set(void);
U_PSEUDO_OBJ *U_PMR_HEADER_set(int IsDual, int IsVideo, const U_PSEUDO_OBJ *Version, 
      uint32_t LogicalDpiX, uint32_t LogicalDpiY);
U_PSEUDO_OBJ *U_PMR_CLEAR_set(const U_PSEUDO_OBJ *Color);
U_PSEUDO_OBJ *U_PMR_DRAWARC_set(uint32_t PenID, U_FLOAT Start, U_FLOAT Sweep, const U_PSEUDO_OBJ *Rect);
U_PSEUDO_OBJ *U_PMR_DRAWBEZIERS_set(uint32_t PenID, const U_PSEUDO_OBJ *Points);
U_PSEUDO_OBJ *U_PMR_DRAWCLOSEDCURVE_set(uint32_t PenID, U_FLOAT Tension, const U_PSEUDO_OBJ *Points);
U_PSEUDO_OBJ *U_PMR_DRAWCURVE_set(uint32_t PenID, U_FLOAT Tension,uint32_t Offset, uint32_t NSegs, const U_PSEUDO_OBJ *Points);
U_PSEUDO_OBJ *U_PMR_DRAWDRIVERSTRING_set(uint32_t FontID, const U_PSEUDO_OBJ *BrushID, 
      uint32_t DSOFlags, uint32_t HasMatrix, uint32_t GlyphCount,
      const uint16_t *Glyphs, const U_PSEUDO_OBJ *Points, const U_PSEUDO_OBJ *Tm);
U_PSEUDO_OBJ *U_PMR_DRAWELLIPSE_set(uint32_t PenID, const U_PSEUDO_OBJ *Rect);
U_PSEUDO_OBJ *U_PMR_DRAWIMAGE_set(uint32_t ImgID, int32_t ImgAttrID, int32_t SrcUnit, const U_PSEUDO_OBJ *SrcRect, const U_PSEUDO_OBJ *DstRect);
U_PSEUDO_OBJ *U_PMR_DRAWIMAGEPOINTS_set(uint32_t ImgID, int etype, int32_t ImgAttrID, int32_t SrcUnit, const U_PSEUDO_OBJ *SrcRect, const U_PSEUDO_OBJ *Points);
U_PSEUDO_OBJ *U_PMR_DRAWLINES_set(uint32_t PenID, int dtype, const U_PSEUDO_OBJ *Points);
U_PSEUDO_OBJ *U_PMR_DRAWPATH_set(uint32_t PathID, uint32_t PenID);
U_PSEUDO_OBJ *U_PMR_DRAWPIE_set(uint32_t PenID, U_FLOAT Start, U_FLOAT Sweep, const U_PSEUDO_OBJ *Rect);
U_PSEUDO_OBJ *U_PMR_DRAWRECTS_set(uint32_t PenID, const U_PSEUDO_OBJ *Rects);
U_PSEUDO_OBJ *U_PMR_DRAWSTRING_set(uint32_t FontID, const U_PSEUDO_OBJ *BrushID, 
      uint32_t FormatID, uint32_t Length, const U_PSEUDO_OBJ *Rect, const uint16_t *Text);
U_PSEUDO_OBJ *U_PMR_FILLCLOSEDCURVE_set(int ftype, U_FLOAT Tension, const U_PSEUDO_OBJ * BrushID, const U_PSEUDO_OBJ *Points);
U_PSEUDO_OBJ *U_PMR_FILLELLIPSE_set(const U_PSEUDO_OBJ * BrushID, const U_PSEUDO_OBJ *Rect);
U_PSEUDO_OBJ *U_PMR_FILLPATH_set(uint32_t PathID, const U_PSEUDO_OBJ * BrushID);
U_PSEUDO_OBJ *U_PMR_FILLPIE_set(U_FLOAT Start, U_FLOAT Sweep, const U_PSEUDO_OBJ *BrushID, const U_PSEUDO_OBJ *Rect);
U_PSEUDO_OBJ *U_PMR_FILLPOLYGON_set(const U_PSEUDO_OBJ *BrushID, const U_PSEUDO_OBJ *Points);
U_PSEUDO_OBJ *U_PMR_FILLRECTS_set(const U_PSEUDO_OBJ *BrushID, const U_PSEUDO_OBJ *Rects);
U_PSEUDO_OBJ *U_PMR_FILLREGION_set(uint32_t RgnID, const U_PSEUDO_OBJ *BrushID);
U_PSEUDO_OBJ *U_PMR_OBJECT_PO_set(uint32_t ObjID, U_PSEUDO_OBJ *Po);
U_PSEUDO_OBJ *U_PMR_OBJECT_set(uint32_t ObjID, int otype, int ntype, uint32_t TSize, size_t cbData, const char *Data);
U_PSEUDO_OBJ *U_PMR_SETANTIALIASMODE_set(int SMenum, int aatype);
U_PSEUDO_OBJ *U_PMR_SETCOMPOSITINGMODE_set(int CMenum);
U_PSEUDO_OBJ *U_PMR_SETCOMPOSITINGQUALITY_set(int CQenum);
U_PSEUDO_OBJ *U_PMR_SETINTERPOLATIONMODE_set(int IMenum);
U_PSEUDO_OBJ *U_PMR_SETPIXELOFFSETMODE_set(int POMenum);
U_PSEUDO_OBJ *U_PMR_SETRENDERINGORIGIN_set(int32_t X, int32_t Y);
U_PSEUDO_OBJ *U_PMR_SETTEXTCONTRAST_set(int GC);
U_PSEUDO_OBJ *U_PMR_SETTEXTRENDERINGHINT_set(int TRHenum);
U_PSEUDO_OBJ *U_PMR_BEGINCONTAINER_set(int UTenum, U_PSEUDO_OBJ *DstRect, U_PSEUDO_OBJ *SrcRect, uint32_t StackID);
U_PSEUDO_OBJ *U_PMR_BEGINCONTAINERNOPARAMS_set(int StackID);
U_PSEUDO_OBJ *U_PMR_ENDCONTAINER_set(int StackID);
U_PSEUDO_OBJ *U_PMR_RESTORE_set(int StackID);
U_PSEUDO_OBJ *U_PMR_SAVE_set(int StackID);
U_PSEUDO_OBJ *U_PMR_SETTSCLIP_set(U_PSEUDO_OBJ *Rects);
U_PSEUDO_OBJ *U_PMR_SETTSGRAPHICS_set(int vgatype, U_PMF_SETTSGRAPHICS *Tsg, U_PSEUDO_OBJ *Palette);
U_PSEUDO_OBJ *U_PMR_MULTIPLYWORLDTRANSFORM_set(int xmtype, U_PSEUDO_OBJ *Tm);
U_PSEUDO_OBJ *U_PMR_RESETWORLDTRANSFORM_set(void);
U_PSEUDO_OBJ *U_PMR_ROTATEWORLDTRANSFORM_set(int xmtype, U_FLOAT Angle);
U_PSEUDO_OBJ *U_PMR_SCALEWORLDTRANSFORM_set(int xmtype, U_FLOAT X, U_FLOAT Y);
U_PSEUDO_OBJ *U_PMR_SETPAGETRANSFORM_set(int PUenum, U_FLOAT Sale);
U_PSEUDO_OBJ *U_PMR_SETWORLDTRANSFORM_set(U_PSEUDO_OBJ *Tm);
U_PSEUDO_OBJ *U_PMR_TRANSLATEWORLDTRANSFORM_set(int xmtype, U_FLOAT Dx, U_FLOAT Dy);
U_PSEUDO_OBJ *U_PMR_STROKEFILLPATH_set(void);




/* EMF+ prototypes (objects_get) */

int U_PMF_BRUSH_get(const char *contents, uint32_t *Version, uint32_t *Type, const char **Data, const char *blimit);
int U_PMF_CUSTOMLINECAP_get(const char *contents, uint32_t *Version, uint32_t *Type, const char **Data, const char *blimit);
int U_PMF_FONT_get(const char *contents, uint32_t *Version, U_FLOAT *EmSize, uint32_t *SizeUnit, int32_t *FSFlags, uint32_t *Length, const char **Data, const char *blimit);
int U_PMF_IMAGE_get(const char *contents, uint32_t *Version, uint32_t *Type, const char **Data, const char *blimit);
int U_PMF_IMAGEATTRIBUTES_get(const char *contents, uint32_t *Version, uint32_t *WrapMode, uint32_t *ClampColor, uint32_t *ObjectClamp, const char *blimit);
int U_PMF_PATH_get(const char *contents, uint32_t *Version, uint32_t *Count, uint16_t *Flags, const char **Points, const char **Types, const char *blimit);
int U_PMF_PEN_get(const char *contents, uint32_t *Version, uint32_t *Type, const char **PenData, const char **Brush, const char *blimit);
int U_PMF_REGION_get(const char *contents, uint32_t *Version, uint32_t *Count, const char **Nodes, const char *blimit);
int U_PMF_STRINGFORMAT_get(const char *contents, U_PMF_STRINGFORMAT *Sfs, const char **Data, const char *blimit);
int U_PMF_ARGB_get(const char *contents, uint8_t *Blue, uint8_t *Green, uint8_t *Red, uint8_t *Alpha, const char *blimit);
int U_PMF_BITMAP_get(const char *contents, U_PMF_BITMAP *Bs, const char **Data, const char *blimit);
int U_PMF_BITMAPDATA_get(const char *contents, U_PMF_PALETTE *Ps, const char **Colors, const char **Data, const char *blimit);
int U_PMF_BLENDCOLORS_get(const char *contents, uint32_t *Elements, U_FLOAT **Positions, const char **Colors, const char *blimit);
int U_PMF_BLENDFACTORS_get(const char *contents, uint32_t *Elements, U_FLOAT **Positions, U_FLOAT **Factors, const char *blimit);
int U_PMF_BOUNDARYPATHDATA_get(const char *contents, int32_t *Size, const char **Data, const char *blimit);
int U_PMF_BOUNDARYPOINTDATA_get(const char *contents, int32_t *Elements, U_PMF_POINTF **Points, const char *blimit);
int U_PMF_CHARACTERRANGE_get(const char *contents, int32_t *First, int32_t *Length, const char *blimit);
int U_PMF_COMPOUNDLINEDATA_get(const char *contents, int32_t *Elements, U_FLOAT **Widths, const char *blimit);
int U_PMF_COMPRESSEDIMAGE_get(const char *contents, const char **Data, const char *blimit);
int U_PMF_CUSTOMENDCAPDATA_get(const char *contents, int32_t *Size, const char **Data, const char *blimit);
int U_PMF_CUSTOMLINECAPARROWDATA_get(const char *contents, U_PMF_CUSTOMLINECAPARROWDATA *Ccad, const char *blimit);
int U_PMF_CUSTOMLINECAPDATA_get(const char *contents, U_PMF_CUSTOMLINECAPDATA *Clcd, const char **Data, const char *blimit);
int U_PMF_CUSTOMLINECAPOPTIONALDATA_get(const char *contents, uint32_t Flags, const char **FillData, const char **LineData, const char *blimit);
int U_PMF_CUSTOMSTARTCAPDATA_get(const char *contents, int32_t *Size, const char **Data, const char *blimit);
int U_PMF_DASHEDLINEDATA_get(const char *contents, int32_t *Elements, U_FLOAT **Lengths, const char *blimit);
int U_PMF_FILLPATHOBJ_get(const char *contents, int32_t *Size, const char **Data, const char *blimit);
int U_PMF_FOCUSSCALEDATA_get(const char *contents, uint32_t *Count, U_FLOAT *ScaleX, U_FLOAT *ScaleY, const char *blimit);
int U_PMF_GRAPHICSVERSION_get(const char *contents, int *Signature, int *GrfVersion, const char *blimit);
int U_PMF_HATCHBRUSHDATA_get(const char *contents, uint32_t *Style, U_PMF_ARGB *Foreground, U_PMF_ARGB *Background, const char *blimit);
int U_PMF_INTEGER7_get(const char **contents, U_FLOAT *Value, const char *blimit);
int U_PMF_INTEGER15_get(const char **contents, U_FLOAT *Value, const char *blimit);
int U_PMF_LANGUAGEIDENTIFIER_get(U_PMF_LANGUAGEIDENTIFIER LId, int *SubLId, int *PriLId);
int U_PMF_LINEARGRADIENTBRUSHDATA_get(const char *contents, U_PMF_LINEARGRADIENTBRUSHDATA *Lgbd, const char **Data, const char *blimit);
int U_PMF_LINEARGRADIENTBRUSHOPTIONALDATA_get(const char *contents, uint32_t Flags, U_PMF_TRANSFORMMATRIX *Tm, const char **Bc, const char **BfH, const char **BfV, const char *blimit);
int U_PMF_LINEPATH_get(const char *contents, int32_t *Size, const char **Data, const char *blimit);
int U_PMF_METAFILE_get(const char *contents, uint32_t *Type, uint32_t *Size, const char **Data, const char *blimit);
int U_PMF_PALETTE_get(const char *contents, uint32_t *Flags, uint32_t *Elements, const char **Data, const char *blimit);
int U_PMF_PATHGRADIENTBRUSHDATA_get(const char *contents, U_PMF_PATHGRADIENTBRUSHDATA *Pgbd, const char **Gradient, const char **Boundary, const char **Data, const char *blimit);
int U_PMF_PATHGRADIENTBRUSHOPTIONALDATA_get(const char *contents, uint32_t Flags, U_PMF_TRANSFORMMATRIX *Matrix, const char **Pattern, const char **Data, const char *blimit);
int U_PMF_PATHPOINTTYPE_get(const char *contents, int *Flags, int *Type, const char *blimit);
int U_PMF_PATHPOINTTYPERLE_get(const char *contents, int *Bezier, int *RL, int *Ppt, const char *blimit);
int U_PMF_PENDATA_get(const char *contents, uint32_t *Flags, uint32_t *Unit, U_FLOAT *Width, const char **Data, const char *blimit);
int U_PMF_PENOPTIONALDATA_get(const char *contents, uint32_t Flags, U_PMF_TRANSFORMMATRIX *Matrix, 
  int32_t *StartCap, int32_t *EndCap, uint32_t *Join, U_FLOAT *MiterLimit, int32_t *Style, int32_t *DLCap, U_FLOAT *DLOffset, 
  const char **DLData, int32_t *Alignment, const char **CmpndLineData, const char **CSCapData, const char **CECapData, const char *blimit);
int U_PMF_POINT_get(const char **contents, U_FLOAT *X, U_FLOAT *Y, const char *blimit);
int U_PMF_POINTF_get(const char **contents, U_FLOAT *X, U_FLOAT *Y, const char *blimit);
int U_PMF_POINTR_get(const char **contents, U_FLOAT *X, U_FLOAT *Y, const char *blimit);
int U_PMF_RECT_get(const char **contents, int16_t *X, int16_t *Y, int16_t *Width, int16_t *Height, const char *blimit);
int U_PMF_RECTF_get(const char **contents, U_FLOAT *X, U_FLOAT *Y, U_FLOAT *Width, U_FLOAT *Height, const char *blimit);
int U_PMF_REGIONNODE_get(const char *contents, uint32_t *Type, const char **Data, const char *blimit);
/* There is no U_PMF_REGIONNODECHILDNODES_get, see the note in upmf.c */
int U_PMF_REGIONNODEPATH_get(const char *contents, int32_t *Size, const char **Data, const char *blimit);
int U_PMF_SOLIDBRUSHDATA_get(const char *contents, U_PMF_ARGB *Color, const char *blimit);
int U_PMF_STRINGFORMATDATA_get(const char *contents, uint32_t TabStopCount, uint32_t RangeCount, 
      const U_FLOAT **TabStops, const U_PMF_CHARACTERRANGE **CharRange, const char *blimit);
int U_PMF_TEXTUREBRUSHDATA_get(const char *contents, uint32_t *Flags, int32_t *WrapMode, const char **Data, const char *blimit);
int U_PMF_TEXTUREBRUSHOPTIONALDATA_get(const char *contents, int HasImage, U_PMF_TRANSFORMMATRIX *Matrix, const char **Image, const char *blimit);
int U_PMF_TRANSFORMMATRIX_get(const char *contents, U_PMF_TRANSFORMMATRIX *Matrix, const char *blimit);
int U_PMF_IE_BLUR_get(const char *contents, U_FLOAT *Radius, uint32_t *ExpandEdge, const char *blimit);
int U_PMF_IE_BRIGHTNESSCONTRAST_get(const char *contents, int32_t *Brightness, int32_t *Contrast, const char *blimit);
int U_PMF_IE_COLORBALANCE_get(const char *contents, int32_t *CyanRed, int32_t *MagentaGreen, int32_t *YellowBlue, const char *blimit);
int U_PMF_IE_COLORCURVE_get(const char *contents, uint32_t *Adjust, uint32_t *Channel, int32_t *Intensity, const char *blimit);
int U_PMF_IE_COLORLOOKUPTABLE_get(const char *contents, 
      const uint8_t **BLUT, const uint8_t **GLUT, const uint8_t **RLUT, const uint8_t **ALUT, const char *blimit);
int U_PMF_IE_COLORMATRIX_get(const char *contents, U_PMF_IE_COLORMATRIX *Matrix, const char *blimit);
int U_PMF_IE_HUESATURATIONLIGHTNESS_get(const char *contents, int32_t *Hue, int32_t *Saturation, int32_t *Lightness, const char *blimit);
int U_PMF_IE_LEVELS_get(const char *contents, int32_t *Highlight, int32_t *Midtone, int32_t *Shadow, const char *blimit);
int U_PMF_IE_REDEYECORRECTION_get(const char *contents, int32_t *Elements, U_RECTL **Rects, const char *blimit);
int U_PMF_IE_SHARPEN_get(const char *contents, U_FLOAT *Radius, int32_t *Sharpen, const char *blimit);
int U_PMF_IE_TINT_get(const char *contents, int32_t *Hue, int32_t *Amount, const char *blimit);

/* EMF+ prototypes (records_get) */

int U_PMR_OFFSETCLIP_get(const char *contents, U_PMF_CMN_HDR *Header, U_FLOAT *dX, U_FLOAT *dY);
int U_PMR_RESETCLIP_get(const char *contents, U_PMF_CMN_HDR *Header);
int U_PMR_SETCLIPPATH_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *PathID, int *CMenum);
int U_PMR_SETCLIPRECT_get(const char *contents, U_PMF_CMN_HDR *Header, int *CMenum, U_PMF_RECTF *Rect);
int U_PMR_SETCLIPREGION_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *PathID, int *CMenum);
int U_PMR_COMMENT_get(const char *contents, U_PMF_CMN_HDR *Header, const char **Data);
int U_PMR_ENDOFFILE_get(const char *contents, U_PMF_CMN_HDR *Header);
int U_PMR_GETDC_get(const char *contents, U_PMF_CMN_HDR *Header);
int U_PMR_HEADER_get(const char *contents, U_PMF_CMN_HDR *Header, U_PMF_GRAPHICSVERSION *Version, int *IsDual, int *IsVideo, uint32_t *LogicalDpiX, uint32_t *LogicalDpiY);
int U_PMR_CLEAR_get(const char *contents, U_PMF_CMN_HDR *Header, U_PMF_ARGB *Color);
int U_PMR_DRAWARC_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *PenID, int *ctype, U_FLOAT *Start, U_FLOAT *Sweep, U_PMF_RECTF *Rect);
int U_PMR_DRAWBEZIERS_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *PenID, int *ctype, int *RelAbs, uint32_t *Elements,  U_PMF_POINTF **Points);
int U_PMR_DRAWCLOSEDCURVE_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *PenID, int *ctype, int *RelAbs, U_FLOAT *Tension, uint32_t *Elements, U_PMF_POINTF **Points);
int U_PMR_DRAWCURVE_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *PenID, int *ctype, U_FLOAT *Tension, uint32_t *Offset, uint32_t *NSegs, uint32_t *Elements, U_PMF_POINTF **Points);
int U_PMR_DRAWDRIVERSTRING_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *FontID, int *btype, uint32_t *BrushID, uint32_t *DSOFlags, uint32_t *HasMatrix, uint32_t *Elements, uint16_t **Glyphs, U_PMF_POINTF **Points, U_PMF_TRANSFORMMATRIX **Matrix);
int U_PMR_DRAWELLIPSE_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *PenID, int *ctype, U_PMF_RECTF *Rect);
int U_PMR_DRAWIMAGE_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *ImgID, int *ctype, uint32_t *ImgAttrID, int32_t *SrcUnit, U_PMF_RECTF *SrcRect, U_PMF_RECTF *DstRect);
int U_PMR_DRAWIMAGEPOINTS_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *ImgID, int *ctype, int *etype, int *RelAbs, uint32_t *ImgAttrID, int32_t *SrcUnit, U_PMF_RECTF *SrcRect, uint32_t *Elements, U_PMF_POINTF **Points);
int U_PMR_DRAWLINES_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *PenID, int *ctype, int *dtype, int *RelAbs, uint32_t *Elements, U_PMF_POINTF **Points);
int U_PMR_DRAWPATH_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *PathID, uint32_t *PenID);
int U_PMR_DRAWPIE_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *PenID, int *ctype, U_FLOAT *Start, U_FLOAT *Sweep, U_PMF_RECTF *Rect);
int U_PMR_DRAWRECTS_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *PenID, int *ctype, uint32_t *Elements, U_PMF_RECTF **Rects);
int U_PMR_DRAWSTRING_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *FontID, int *btype, uint32_t *BrushID, uint32_t *FormatID, uint32_t *Elements, U_PMF_RECTF *Rect, uint16_t **String);
int U_PMR_FILLCLOSEDCURVE_get(const char *contents, U_PMF_CMN_HDR *Header, int *btype, int *ctype, int *ftype, int *RelAbs, uint32_t *BrushID, U_FLOAT *Tension, uint32_t *Elements, U_PMF_POINTF **Points);
int U_PMR_FILLELLIPSE_get(const char *contents, U_PMF_CMN_HDR *Header, int *btype, int *ctype, uint32_t *BrushID, U_PMF_RECTF *Rect);
int U_PMR_FILLPATH_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *PathID, int *btype, uint32_t *BrushID);
int U_PMR_FILLPIE_get(const char *contents, U_PMF_CMN_HDR *Header, int *btype, int *ctype, uint32_t *BrushID, U_FLOAT *Start, U_FLOAT *Sweep, U_PMF_RECTF *Rect);
int U_PMR_FILLPOLYGON_get(const char *contents, U_PMF_CMN_HDR *Header, int *btype, int *ctype, int *RelAbs, uint32_t *BrushID, uint32_t *Elements, U_PMF_POINTF **Points);
int U_PMR_FILLRECTS_get(const char *contents, U_PMF_CMN_HDR *Header, int *btype, int *ctype, uint32_t *BrushID, uint32_t *Elements, U_PMF_RECTF **Rects);
int U_PMR_FILLREGION_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *RgnID, int *btype, int *ctype, uint32_t *BrushID);
int U_PMR_OBJECT_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *ObjID, int *otype, int *ntype, uint32_t *TSize, const char **Data);
int U_PMR_SERIALIZABLEOBJECT_get(const char *contents, U_PMF_CMN_HDR *Header, uint8_t *GUID, uint32_t *Size, const char **Data);
int U_PMR_SETANTIALIASMODE_get(const char *contents, U_PMF_CMN_HDR *Header, int *SMenum, int *aatype);
int U_PMR_SETCOMPOSITINGMODE_get(const char *contents, U_PMF_CMN_HDR *Header, int *CMenum);
int U_PMR_SETCOMPOSITINGQUALITY_get(const char *contents, U_PMF_CMN_HDR *Header, int *CQenum);
int U_PMR_SETINTERPOLATIONMODE_get(const char *contents, U_PMF_CMN_HDR *Header, int *IMenum);
int U_PMR_SETPIXELOFFSETMODE_get(const char *contents, U_PMF_CMN_HDR *Header, int *POMenum);
int U_PMR_SETRENDERINGORIGIN_get(const char *contents, U_PMF_CMN_HDR *Header, int32_t *X, int32_t *Y);
int U_PMR_SETTEXTCONTRAST_get(const char *contents, U_PMF_CMN_HDR *Header, int *TGC);
int U_PMR_SETTEXTRENDERINGHINT_get(const char *contents, U_PMF_CMN_HDR *Header, int *TRHenum);
int U_PMR_BEGINCONTAINER_get(const char *contents, U_PMF_CMN_HDR *Header, int *UTenum, U_PMF_RECTF *DstRect, U_PMF_RECTF *SrcRect, uint32_t *StackID);
int U_PMR_BEGINCONTAINERNOPARAMS_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *StackID);
int U_PMR_ENDCONTAINER_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *StackID);
int U_PMR_RESTORE_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *StackID);
int U_PMR_SAVE_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *StackID);
int U_PMR_SETTSCLIP_get(const char *contents, U_PMF_CMN_HDR *Header, int *ctype, uint32_t *Elements, U_PMF_RECTF **Rects);
int U_PMR_SETTSGRAPHICS_get(const char *contents, U_PMF_CMN_HDR *Header, int *vgatype, int *pptype, uint8_t *AntiAliasMode, uint8_t *TextRenderHint, uint8_t *CompositingMode, uint8_t *CompositingQuality, int16_t *RenderOriginX, int16_t *RenderOriginY, uint16_t *TextContrast, uint8_t *FilterType, uint8_t *PixelOffset, U_PMF_TRANSFORMMATRIX *WorldToDevice, const char **Data);
int U_PMR_MULTIPLYWORLDTRANSFORM_get(const char *contents, U_PMF_CMN_HDR *Header, int *xmtype, U_PMF_TRANSFORMMATRIX *Matrix);
int U_PMR_RESETWORLDTRANSFORM_get(const char *contents, U_PMF_CMN_HDR *Header);
int U_PMR_ROTATEWORLDTRANSFORM_get(const char *contents, U_PMF_CMN_HDR *Header, int *xmtype, U_FLOAT *Angle);
int U_PMR_SCALEWORLDTRANSFORM_get(const char *contents, U_PMF_CMN_HDR *Header, int *xmtype, U_FLOAT *Sx, U_FLOAT *Sy);
int U_PMR_SETPAGETRANSFORM_get(const char *contents, U_PMF_CMN_HDR *Header, int *PUenum, U_FLOAT *Scale);
int U_PMR_SETWORLDTRANSFORM_get(const char *contents, U_PMF_CMN_HDR *Header, U_PMF_TRANSFORMMATRIX *Matrix);
int U_PMR_TRANSLATEWORLDTRANSFORM_get(const char *contents, U_PMF_CMN_HDR *Header, int *xmtype, U_FLOAT *Dx, U_FLOAT *Dy);
int U_PMR_STROKEFILLPATH_get(const char *contents, U_PMF_CMN_HDR *Header);
int U_PMR_MULTIFORMATSTART_get(const char *contents, U_PMF_CMN_HDR *Header);
int U_PMR_MULTIFORMATSECTION_get(const char *contents, U_PMF_CMN_HDR *Header);
int U_PMR_MULTIFORMATEND_get(const char *contents, U_PMF_CMN_HDR *Header);
//! \endcond


#ifdef __cplusplus
}
#endif

#endif /* _UPMF_ */
