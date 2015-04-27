/**
  @file uwmf_print.h
  
  @brief Prototypes for functions for printing records from WMF files.
*/

/*
File:      uwmf_print.h
Version:   0.0.2
Date:      14-FEB-2013
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2012 David Mathog and California Institute of Technology (Caltech)
*/

#ifndef _UWMF_PRINT_
#define _UWMF_PRINT_

#ifdef __cplusplus
extern "C" {
#endif
#include "uwmf.h"
#include "uemf_print.h"

//! \cond
/* prototypes for objects used in WMR records (other than those defined in uemf_print.h) */
void brush_print(U_BRUSH b);
void font_print(const char *font);
void pltntry_print(U_PLTNTRY pny);
void palette_print(const U_PALETTE *p, const char *PalEntries);
void pen_print(U_PEN p);
void rect16_ltrb_print(U_RECT16 rect);
void rect16_brtl_print(U_RECT16 rect);
void region_print(const char *region);
void bitmap16_print(U_BITMAP16 b);
void bitmapcoreheader_print(U_BITMAPCOREHEADER ch);
void logbrushw_print(U_WLOGBRUSH lb);
void polypolygon_print(uint16_t  nPolys, const uint16_t *aPolyCounts, const char *Points);
void scan_print(U_SCAN sc);
void dibheader_print(const char *dh, const char *blimit);

/* prototypes for WMF records */
int  wmfheader_print(const char *contents, const char *blimit);
void U_WMRNOTIMPLEMENTED_print(const char *contents);
int  U_wmf_onerec_print(const char *contents, const char *blimit, int recnum,  size_t  off);
//! \endcond

#ifdef __cplusplus
}
#endif

#endif /* _UWMF_PRINT_ */
