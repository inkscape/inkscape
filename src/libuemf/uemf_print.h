/**
  @file uemf_print.h
  
  @brief Prototypes for functions for printing records from EMF files.
*/

/*
File:      uemf_print.h
Version:   0.0.9
Date:      21-MAY-2015
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2015 David Mathog and California Institute of Technology (Caltech)
*/

#ifndef _UEMF_PRINT_
#define _UEMF_PRINT_

#ifdef __cplusplus
extern "C" {
#endif

//! \cond
/* prototypes for miscellaneous  */
uint32_t lu_crc32(const char *record, uint32_t Size);

/* prototypes for objects used in EMR records */
void hexbytes_print(uint8_t *buf,unsigned int num);
void colorref_print(U_COLORREF color);
void rgbquad_print(U_RGBQUAD color);
void rectl_print(U_RECTL rect);
void sizel_print(U_SIZEL sz);
void pointl_print(U_POINTL pt);
void point16_print(U_POINT16 pt);
void lcs_gamma_print(U_LCS_GAMMA lg);
void lcs_gammargb_print(U_LCS_GAMMARGB lgr);
void trivertex_print(U_TRIVERTEX tv);
void gradient3_print(U_GRADIENT3 g3);
void gradient4_print(U_GRADIENT4 g4);
void logbrush_print(U_LOGBRUSH lb);
void xform_print(U_XFORM xform);
void ciexyz_print(U_CIEXYZ ciexyz);
void ciexyztriple_print(U_CIEXYZTRIPLE cie3);
void logcolorspacea_print(U_LOGCOLORSPACEA lcsa);
void logcolorspacew_print(U_LOGCOLORSPACEW lcsa);
void panose_print(U_PANOSE panose);
void logfont_print(U_LOGFONT lf);
void logfont_panose_print(U_LOGFONT_PANOSE lfp); 
void bitmapinfoheader_print(const char *Bmih);
void bitmapinfo_print(const char *Bmi, const char *blimit);
void blend_print(U_BLEND blend);
void extlogpen_print(const PU_EXTLOGPEN elp);
void logpen_print(U_LOGPEN lp);
void logpltntry_print(U_LOGPLTNTRY lpny);
void logpalette_print(const PU_LOGPALETTE lp);
void rgndataheader_print(U_RGNDATAHEADER rdh);
void rgndata_print(const PU_RGNDATA rd, const char *blimit);
void coloradjustment_print(U_COLORADJUSTMENT ca);
void pixelformatdescriptor_print(U_PIXELFORMATDESCRIPTOR pfd);
void emrtext_print(const char *emt, const char *record, const char *blimit, int type);

/* prototypes for EMR records */
void U_EMRNOTIMPLEMENTED_print(const char *name, const char *contents, int recnum, int off);
void U_EMRHEADER_print(const char *contents);
void U_EMRPOLYBEZIER_print(const char *contents);
void U_EMRPOLYGON_print(const char *contents);
void U_EMRPOLYLINE_print(const char *contents);
void U_EMRPOLYBEZIERTO_print(const char *contents);
void U_EMRPOLYLINETO_print(const char *contents);
void U_EMRPOLYPOLYLINE_print(const char *contents);
void U_EMRPOLYPOLYGON_print(const char *contents);
void U_EMRSETWINDOWEXTEX_print(const char *contents);
void U_EMRSETWINDOWORGEX_print(const char *contents);
void U_EMRSETVIEWPORTEXTEX_print(const char *contents);
void U_EMRSETVIEWPORTORGEX_print(const char *contents);
void U_EMRSETBRUSHORGEX_print(const char *contents);
void U_EMREOF_print(const char *contents);
void U_EMRSETPIXELV_print(const char *contents);
void U_EMRSETMAPPERFLAGS_print(const char *contents);
void U_EMRSETMAPMODE_print(const char *contents);
void U_EMRSETBKMODE_print(const char *contents);
void U_EMRSETPOLYFILLMODE_print(const char *contents);
void U_EMRSETROP2_print(const char *contents);
void U_EMRSETSTRETCHBLTMODE_print(const char *contents);
void U_EMRSETTEXTALIGN_print(const char *contents);
void U_EMRSETCOLORADJUSTMENT_print(const char *contents);
void U_EMRSETTEXTCOLOR_print(const char *contents);
void U_EMRSETBKCOLOR_print(const char *contents);
void U_EMROFFSETCLIPRGN_print(const char *contents);
void U_EMRMOVETOEX_print(const char *contents);
void U_EMRSETMETARGN_print(const char *contents);
void U_EMREXCLUDECLIPRECT_print(const char *contents);
void U_EMRINTERSECTCLIPRECT_print(const char *contents);
void U_EMRSCALEVIEWPORTEXTEX_print(const char *contents);
void U_EMRSCALEWINDOWEXTEX_print(const char *contents);
void U_EMRSAVEDC_print(const char *contents);
void U_EMRRESTOREDC_print(const char *contents);
void U_EMRSETWORLDTRANSFORM_print(const char *contents);
void U_EMRMODIFYWORLDTRANSFORM_print(const char *contents);
void U_EMRSELECTOBJECT_print(const char *contents);
void U_EMRCREATEPEN_print(const char *contents);
void U_EMRCREATEBRUSHINDIRECT_print(const char *contents);
void U_EMRDELETEOBJECT_print(const char *contents);
void U_EMRANGLEARC_print(const char *contents);
void U_EMRELLIPSE_print(const char *contents);
void U_EMRRECTANGLE_print(const char *contents);
void U_EMRROUNDRECT_print(const char *contents);
void U_EMRARC_print(const char *contents);
void U_EMRCHORD_print(const char *contents);
void U_EMRPIE_print(const char *contents);
void U_EMRSELECTPALETTE_print(const char *contents);
void U_EMRCREATEPALETTE_print(const char *contents);
void U_EMRSETPALETTEENTRIES_print(const char *contents);
void U_EMRRESIZEPALETTE_print(const char *contents);
void U_EMRREALIZEPALETTE_print(const char *contents);
void U_EMREXTFLOODFILL_print(const char *contents);
void U_EMRLINETO_print(const char *contents);
void U_EMRARCTO_print(const char *contents);
void U_EMRPOLYDRAW_print(const char *contents);
void U_EMRSETARCDIRECTION_print(const char *contents);
void U_EMRSETMITERLIMIT_print(const char *contents);
void U_EMRBEGINPATH_print(const char *contents);
void U_EMRENDPATH_print(const char *contents);
void U_EMRCLOSEFIGURE_print(const char *contents);
void U_EMRFILLPATH_print(const char *contents);
void U_EMRSTROKEANDFILLPATH_print(const char *contents);
void U_EMRSTROKEPATH_print(const char *contents);
void U_EMRFLATTENPATH_print(const char *contents);
void U_EMRWIDENPATH_print(const char *contents);
void U_EMRSELECTCLIPPATH_print(const char *contents);
void U_EMRABORTPATH_print(const char *contents);
void U_EMRCOMMENT_print(const char *contents, size_t off);
void U_EMRFILLRGN_print(const char *contents);
void U_EMRFRAMERGN_print(const char *contents);
void U_EMRINVERTRGN_print(const char *contents);
void U_EMRPAINTRGN_print(const char *contents);
void U_EMREXTSELECTCLIPRGN_print(const char *contents);
void U_EMRBITBLT_print(const char *contents);
void U_EMRSTRETCHBLT_print(const char *contents);
void U_EMRMASKBLT_print(const char *contents);
void U_EMRPLGBLT_print(const char *contents);
void U_EMRSETDIBITSTODEVICE_print(const char *contents);
void U_EMRSTRETCHDIBITS_print(const char *contents);
void U_EMREXTCREATEFONTINDIRECTW_print(const char *contents);
void U_EMREXTTEXTOUTA_print(const char *contents);
void U_EMREXTTEXTOUTW_print(const char *contents);
void U_EMRPOLYBEZIER16_print(const char *contents);
void U_EMRPOLYGON16_print(const char *contents);
void U_EMRPOLYLINE16_print(const char *contents);
void U_EMRPOLYBEZIERTO16_print(const char *contents);
void U_EMRPOLYLINETO16_print(const char *contents);
void U_EMRPOLYPOLYLINE16_print(const char *contents);
void U_EMRPOLYPOLYGON16_print(const char *contents);
void U_EMRPOLYDRAW16_print(const char *contents);
void U_EMRCREATEMONOBRUSH_print(const char *contents);
void U_EMRCREATEDIBPATTERNBRUSHPT_print(const char *contents);
void U_EMREXTCREATEPEN_print(const char *contents);
void U_EMRSETICMMODE_print(const char *contents);
void U_EMRCREATECOLORSPACE_print(const char *contents);
void U_EMRSETCOLORSPACE_print(const char *contents);
void U_EMRDELETECOLORSPACE_print(const char *contents);
void U_EMRPIXELFORMAT_print(const char *contents);
void U_EMRSMALLTEXTOUT_print(const char *contents);
void U_EMRALPHABLEND_print(const char *contents);
void U_EMRSETLAYOUT_print(const char *contents);
void U_EMRTRANSPARENTBLT_print(const char *contents);
void U_EMRGRADIENTFILL_print(const char *contents);
void U_EMRCREATECOLORSPACEW_print(const char *contents);
int  U_emf_onerec_print(const char *contents, char *blimit, int recnum, int off);
//! \endcond


#ifdef __cplusplus
}
#endif

#endif /* _UEMF_PRINT_ */
