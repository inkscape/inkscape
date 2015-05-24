/**
  @file upmf_print.h
  
  @brief Prototypes for functions for printing records from EMF files.
*/

/*
File:      upmf_print.h
Version:   0.0.5
Date:      28-APR-2015
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2015 David Mathog and California Institute of Technology (Caltech)
*/

#ifndef _UPMF_PRINT_
#define _UPMF_PRINT_

#ifdef __cplusplus
extern "C" {
#endif

#include "upmf.h" /* includes uemf.h */

/* prototypes for simple types and enums used in PMR records */
int U_PMF_CMN_HDR_print(const char *contents, U_PMF_CMN_HDR Header, int precnum, int off);
int U_PMF_UINT8_ARRAY_print(const char *Start, const uint8_t *Array, int Elements, char *End);
int U_PMF_BRUSHTYPEENUMERATION_print(int otype);
int U_PMF_HATCHSTYLEENUMERATION_print(int hstype);
int U_PMF_OBJECTTYPEENUMERATION_print(int otype);
int U_PMF_PATHPOINTTYPE_ENUM_print(int Type);
int U_PMF_PX_FMT_ENUM_print(int pfe);
int U_PMF_NODETYPE_print(int Type);

/* prototypes for objects used in PMR records */
int U_PMF_BRUSH_print(const char *contents, const char *blimit);
int U_PMF_CUSTOMLINECAP_print(const char *contents, const char *Which, const char *blimit);
int U_PMF_FONT_print(const char *contents, const char *blimit);
int U_PMF_IMAGE_print(const char *contents, const char *blimit);
int U_PMF_IMAGEATTRIBUTES_print(const char *contents, const char *blimit);
int U_PMF_PATH_print(const char *contents, const char *blimit);
int U_PMF_PEN_print(const char *contents, const char *blimit);
int U_PMF_REGION_print(const char *contents, const char *blimit);
int U_PMF_STRINGFORMAT_print(const char *contents, const char *blimit);
int U_PMF_ARGB_print(const char *contents);
int U_PMF_BITMAP_print(const char *contents, const char *blimit);
int U_PMF_BITMAPDATA_print(const char *contents, const char *blimit);
int U_PMF_BLENDCOLORS_print(const char *contents, const char *blimit);
int U_PMF_BLENDFACTORS_print(const char *contents, const char *type, const char *blimit);
int U_PMF_BOUNDARYPATHDATA_print(const char *contents, const char *blimit);
int U_PMF_BOUNDARYPOINTDATA_print(const char *contents, const char *blimit);
int U_PMF_CHARACTERRANGE_print(const char *contents, const char *blimit);
int U_PMF_COMPOUNDLINEDATA_print(const char *contents, const char *blimit);
int U_PMF_COMPRESSEDIMAGE_print(const char *contents, const char *blimit);
int U_PMF_CUSTOMENDCAPDATA_print(const char *contents, const char *blimit);
int U_PMF_CUSTOMLINECAPARROWDATA_print(const char *contents, const char *blimit);
int U_PMF_CUSTOMLINECAPDATA_print(const char *contents, const char *blimit);
int U_PMF_CUSTOMLINECAPOPTIONALDATA_print(const char *contents, uint32_t Flags, const char *blimit);
int U_PMF_CUSTOMSTARTCAPDATA_print(const char *contents, const char *blimit);
int U_PMF_DASHEDLINEDATA_print(const char *contents, const char *blimit);
int U_PMF_FILLPATHOBJ_print(const char *contents, const char *blimit);
int U_PMF_FOCUSSCALEDATA_print(const char *contents, const char *blimit);
int U_PMF_GRAPHICSVERSION_memsafe_print(const char *contents);
int U_PMF_GRAPHICSVERSION_print(const char *contents, const char *blimit);
int U_PMF_HATCHBRUSHDATA_print(const char *contents, const char *blimit);
int U_PMF_LANGUAGEIDENTIFIER_print(U_PMF_LANGUAGEIDENTIFIER LId);
int U_PMF_LINEARGRADIENTBRUSHDATA_print(const char *contents, const char *blimit);
int U_PMF_LINEARGRADIENTBRUSHOPTIONALDATA_print(const char *contents, int BDFlag, const char *blimit);
int U_PMF_LINEPATH_print(const char *contents, const char *blimit);
int U_PMF_METAFILE_print(const char *contents, const char *blimit);
int U_PMF_PALETTE_print(const char *contents, const char *blimit);
int U_PMF_PATHGRADIENTBRUSHDATA_print(const char *contents, const char *blimit);
int U_PMF_PATHGRADIENTBRUSHOPTIONALDATA_print(const char *contents, int BDFlag, const char *blimit);
int U_PMF_PATHPOINTTYPE_print(const char *contents, const char *blimit);
int U_PMF_PATHPOINTTYPERLE_print(const char *contents, const char *blimit);
int U_PMF_PENDATA_print(const char *contents, const char *blimit);
int U_PMF_PENOPTIONALDATA_print(const char *contents, int Flags, const char *blimit);
int U_PMF_POINT_print(const char **contents, const char *blimit);
int U_PMF_POINTF_print(const char **contents, const char *blimit);
int U_PMF_POINTR_print(const char **contents, U_FLOAT *Xpos, U_FLOAT *Ypos, const char *blimit);
int U_PMF_POINT_S_print(U_PMF_POINT *Point);
int U_PMF_POINTF_S_print(U_PMF_POINTF *Point);
int U_PMF_RECT_print(const char **contents, const char *blimit);
int U_PMF_RECTF_print(const char **contents, const char *blimit);
int U_PMF_RECT_S_print(U_PMF_RECT *Rect);
int U_PMF_RECTF_S_print(U_PMF_RECTF *Rect);
int U_PMF_REGIONNODE_print(const char *contents, int Level, const char *blimit);
int U_PMF_REGIONNODECHILDNODES_print(const char *contents, int Level, const char *blimit);
int U_PMF_REGIONNODEPATH_print(const char *contents, const char *blimit);
int U_PMF_SOLIDBRUSHDATA_print(const char *contents, const char *blimit);
int U_PMF_STRINGFORMATDATA_print(const char *contents, uint32_t TabStopCount, uint32_t RangeCount, const char *blimit);
int U_PMF_TEXTUREBRUSHDATA_print(const char *contents, const char *blimit);
int U_PMF_TEXTUREBRUSHOPTIONALDATA_print(const char *contents, int HasMatrix, int HasImage, const char *blimit);
int U_PMF_TRANSFORMMATRIX_print(const char *contents, const char *blimit);
int U_PMF_TRANSFORMMATRIX2_print(U_PMF_TRANSFORMMATRIX *Matrix);
int U_PMF_ROTMATRIX2_print(U_PMF_ROTMATRIX *Matrix);
int U_PMF_IE_BLUR_print(const char *contents, const char *blimit);
int U_PMF_IE_BRIGHTNESSCONTRAST_print(const char *contents, const char *blimit);
int U_PMF_IE_COLORBALANCE_print(const char *contents, const char *blimit);
int U_PMF_IE_COLORCURVE_print(const char *contents, const char *blimit);
int U_PMF_IE_COLORLOOKUPTABLE_print(const char *contents, const char *blimit);
int U_PMF_IE_COLORMATRIX_print(const char *contents, const char *blimit);
int U_PMF_IE_HUESATURATIONLIGHTNESS_print(const char *contents, const char *blimit);
int U_PMF_IE_LEVELS_print(const char *contents, const char *blimit);
int U_PMF_IE_REDEYECORRECTION_print(const char *contents, const char *blimit);
int U_PMF_IE_SHARPEN_print(const char *contents, const char *blimit);
int U_PMF_IE_TINT_print(const char *contents, const char *blimit);

/* prototypes for PMR records */
int U_PMR_OFFSETCLIP_print(const char *contents);
int U_PMR_RESETCLIP_print(const char *contents);
int U_PMR_SETCLIPPATH_print(const char *contents);
int U_PMR_SETCLIPRECT_print(const char *contents);
int U_PMR_SETCLIPREGION_print(const char *contents);
int U_PMR_COMMENT_print(const char *contents);
int U_PMR_ENDOFFILE_print(const char *contents);
int U_PMR_GETDC_print(const char *contents);
int U_PMR_HEADER_print(const char *contents);
int U_PMR_CLEAR_print(const char *contents);
int U_PMR_DRAWARC_print(const char *contents);
int U_PMR_DRAWBEZIERS_print(const char *contents);
int U_PMR_DRAWCLOSEDCURVE_print(const char *contents);
int U_PMR_DRAWCURVE_print(const char *contents);
int U_PMR_DRAWDRIVERSTRING_print(const char *contents);
int U_PMR_DRAWELLIPSE_print(const char *contents);
int U_PMR_DRAWIMAGE_print(const char *contents);
int U_PMR_DRAWIMAGEPOINTS_print(const char *contents);
int U_PMR_DRAWLINES_print(const char *contents);
int U_PMR_DRAWPATH_print(const char *contents);
int U_PMR_DRAWPIE_print(const char *contents);
int U_PMR_DRAWRECTS_print(const char *contents);
int U_PMR_DRAWSTRING_print(const char *contents);
int U_PMR_FILLCLOSEDCURVE_print(const char *contents);
int U_PMR_FILLELLIPSE_print(const char *contents);
int U_PMR_FILLPATH_print(const char *contents);
int U_PMR_FILLPIE_print(const char *contents);
int U_PMR_FILLPOLYGON_print(const char *contents);
int U_PMR_FILLRECTS_print(const char *contents);
int U_PMR_FILLREGION_print(const char *contents);
int U_PMR_OBJECT_print(const char *contents, const char *blimit, U_OBJ_ACCUM *ObjCont, int term);
int U_PMR_SERIALIZABLEOBJECT_print(const char *contents);
int U_PMR_SETANTIALIASMODE_print(const char *contents);
int U_PMR_SETCOMPOSITINGMODE_print(const char *contents);
int U_PMR_SETCOMPOSITINGQUALITY_print(const char *contents);
int U_PMR_SETINTERPOLATIONMODE_print(const char *contents);
int U_PMR_SETPIXELOFFSETMODE_print(const char *contents);
int U_PMR_SETRENDERINGORIGIN_print(const char *contents);
int U_PMR_SETTEXTCONTRAST_print(const char *contents);
int U_PMR_SETTEXTRENDERINGHINT_print(const char *contents);
int U_PMR_BEGINCONTAINER_print(const char *contents);
int U_PMR_BEGINCONTAINERNOPARAMS_print(const char *contents);
int U_PMR_ENDCONTAINER_print(const char *contents);
int U_PMR_RESTORE_print(const char *contents);
int U_PMR_SAVE_print(const char *contents);
int U_PMR_SETTSCLIP_print(const char *contents);
int U_PMR_SETTSGRAPHICS_print(const char *contents);
int U_PMR_MULTIPLYWORLDTRANSFORM_print(const char *contents);
int U_PMR_RESETWORLDTRANSFORM_print(const char *contents);
int U_PMR_ROTATEWORLDTRANSFORM_print(const char *contents);
int U_PMR_SCALEWORLDTRANSFORM_print(const char *contents);
int U_PMR_SETPAGETRANSFORM_print(const char *contents);
int U_PMR_SETWORLDTRANSFORM_print(const char *contents);
int U_PMR_TRANSLATEWORLDTRANSFORM_print(const char *contents);
int U_PMR_STROKEFILLPATH_print(const char *contents);      /* not documented */
int U_PMR_MULTIFORMATSTART_print(const char *contents);    /* last of reserved but not used */
int U_PMR_MULTIFORMATSECTION_print(const char *contents);  /* last of reserved but not used */
int U_PMR_MULTIFORMATEND_print(const char *contents);      /* last of reserved but not used */

int U_pmf_onerec_print(const char *contents, const char *blimit, int recnum, int off);







#ifdef __cplusplus
}
#endif

#endif /* _UPMF_PRINT_ */
