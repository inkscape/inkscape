/**
  @file uwmf_endian.c
  
  @brief Functions for converting WMF records between Big Endian and Little Endian byte orders.
*/

/*
File:      uwmf_endian.c
Version:   0.1.5
Date:      28-MAY-2015
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2015 David Mathog and California Institute of Technology (Caltech)
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h> /* for offsetof() */
#include <string.h>
#include "uwmf.h"
#include "uwmf_endian.h"

// hide almost everything in this file from Doxygen
//! \cond
/* Prototypes for functions used here and defined in uemf_endian.c, but which are not supposed
to be used in end user code. */

void U_swap2(void *ul, unsigned int count);
void U_swap4(void *ul, unsigned int count);
void bitmapinfo_swap(char *Bmi);

/* **********************************************************************************************
   These functions Swap standard objects used in the WMR records.
   The low level ones do not append EOL.
*********************************************************************************************** */

/**
    \brief Swap U_BITMAP16 object
    \param b U_BITMAP16 object
*/
void bitmap16_swap(
      char *b
    ){
    U_swap2(b,4); /* Type, Width, Height, WidthBytes */
    /* Planes and BitsPixel are bytes, so no swap needed */
    /* Bits[] pixel data should already be in order */
}

/**
    \brief Swap a U_BRUSH object.
    \param b U_BRUSH object.
  style              bColor               bHatch
  U_BS_SOLID           ColorRef Object      Not used (bytes present???)
  U_BS_NULL            ignored              ignored  (bytes present???).
  U_BS_PATTERN         ignored              Bitmap16 object holding patern
  U_BS_DIBPATTERNPT    ColorUsage Enum      DIB object
  U_BS_HATCHED         ColorRef Object      HatchStyle Enumeration
*/
void brush_swap(
      char *b,
      int torev
   ){
   int Style;
   if(torev){   Style = *(uint16_t *)(b + offsetof(U_BRUSH,Style)); }
   U_swap2(b + offsetof(U_BRUSH,Style),1);
   if(!torev){  Style = *(uint16_t *)(b + offsetof(U_BRUSH,Style)); }
   /* Color is already in the right order */
   switch(Style){
      case U_BS_SOLID:
         /* no/ignored data field */
   	 break;
      case U_BS_NULL:
         /* no/ignored data field */
   	 break;
      case U_BS_PATTERN:
   	 bitmap16_swap(b + offsetof(U_BRUSH,Data));
   	 break;
      case U_BS_DIBPATTERNPT:
   	 bitmapinfo_swap(b + offsetof(U_BRUSH,Data));
   	 break;
      case U_BS_HATCHED:
         /* no/ignored data field */
   	 break;
   }
}

/**
    \brief Swap a U_FONT object from pointer.
    \param lf   U_FONT object
*/
void font_swap(
       char *f
   ){
   U_swap2(f + offsetof(U_FONT,Height),5); /*Height, Width, Escapement, Orientation, Weight */
   /* Other fields are single bytes */
}

/**
    \brief Swap a pointer to a U_PALETTE object.
    \param lp  Pointer to a U_PALETTE object.
*/
void palette_swap(
      char *p
   ){
   U_swap2(p + offsetof(U_PALETTE,Start),2); /* Start, NumEntries*/
   /* PalEntries[1] is byte ordered, so no need to swap */
}

/**
    \brief Swap a  U_PEN object.
    \param p   U_PEN object
*/
void pen_swap(
      char *p
   ){
   U_swap2(p + offsetof(U_PEN,Style),3);             /* Style,Widthw[0],Widthw[1] */
   /* Color already in order  */
}

/* there are no 
  void rect16_ltrb_swap()
  void rect16_brtl_swap()
because rectangles are swapped using U_swap2 as an array of 4 int16 values.
*/


/**
    \brief Swap U_REGION object
    \param rect U_REGION object
    \param torev
    PARTIAL IMPLEMENTATION
*/
void region_swap(
      char *reg,
      int   torev
    ){
    int Size;
    if(torev){  Size = *(int16_t *)(reg + offsetof(U_REGION,Size)); }
    U_swap2(reg,10); /* ignore1 through sRrect*/
    if(!torev){ Size = *(int16_t *)(reg + offsetof(U_REGION,Size)); }
    U_swap2(reg + U_SIZE_REGION, (Size - U_SIZE_REGION)/2); /* aScans */
}


/**
    \brief Swap U_BITMAPCOREHEADER object
    \param ch U_BITMAPCOREHEADER object
*/
void bitmapcoreheader_swap(
      char *ch
    ){
    U_swap4(ch + offsetof(U_BITMAPCOREHEADER,Size_4), 1);  /* Size_4, may not be aligned  */
    U_swap2(ch + offsetof(U_BITMAPCOREHEADER,Width),4);    /* Width, Height, Planes, BitCount, */
}

/**   LogBrushW Object                      WMF PDF 2.2.2.10
     \brief Swap a U_WLOGBRUSH object.
     \param lb U_WLOGBRUSH object.

  style                Color                Hatch
  U_BS_SOLID           ColorRef Object      Not used (bytes present???)
  U_BS_NULL            ignored              ignored  (bytes present???).
  U_BS_PATTERN         ignored              not used     (Action is not strictly defined)
  U_BS_DIBPATTERN      ignored              not used     (Action is not strictly defined)
  U_BS_DIBPATTERNPT    ignored              not used     (Action is not strictly defined)
  U_BS_HATCHED         ColorRef Object      HatchStyle Enumeration
*/
void wlogbrush_swap(
      char *lb  
   ){
    U_swap2(lb + offsetof(U_WLOGBRUSH,Style),1);
    /* Color is already in order */
    U_swap2(lb + offsetof(U_WLOGBRUSH,Hatch),1);
}

/**
    \brief Swap U_POLYPOLY object from pointer
    \param pp PU_POLYPOLY object
*/
void polypolygon_swap(
     char *pp,
     int torev
   ){
   int i,totPoints;
   uint16_t  nPolys;
   uint16_t *aPolyCounts;
   if(torev){  nPolys = *(uint16_t *)(pp + offsetof(U_POLYPOLYGON, nPolys)); }
   U_swap2(pp + offsetof(U_POLYPOLYGON, nPolys),1);
   if(!torev){ nPolys = *(uint16_t *)(pp + offsetof(U_POLYPOLYGON, nPolys)); }
   aPolyCounts =  (uint16_t *)(pp + offsetof(U_POLYPOLYGON, aPolyCounts));
   if(torev){
      for(totPoints=0,i=0;i<nPolys; i++){ totPoints += aPolyCounts[i]; }
   }
   U_swap2(aPolyCounts,nPolys);
   if(!torev){
      for(totPoints=0,i=0;i<nPolys; i++){ totPoints += aPolyCounts[i]; }
   }
   U_swap2(&(aPolyCounts[nPolys]),2*totPoints);  /* 2 coords/ point */
}

/**
    \brief Swap U_SCAN object
    \param pp U_SCAN object
*/
void scan_swap(
      char *sc,
      int torev
   ){
   int count;
   if(torev){  count = *(uint16_t *)sc;  }
   U_swap2(sc,3); /*count, top, bottom */
   if(!torev){ count = *(uint16_t *)sc;  }
   U_swap2(sc + offsetof(U_SCAN,ScanLines),count);
}

/**
    \brief Swap a summary of a DIB header
    A DIB header in an WMF may be either a BitmapCoreHeader or BitmapInfoHeader.
    \param dh void pointer to DIB header
*/
void dibheader_swap(
      char *dh,
      int torev
   ){
   int Size;
   memcpy(&Size, dh, 4); /* may not be aligned */
   if(!torev)U_swap4(&Size,1);
   if(Size == 0xC){
       bitmapcoreheader_swap(dh);
   }
   else {
       bitmapinfo_swap(dh);
   }
}

/**
    \brief Swap WMF header object
    \param head uint8_t pointer to header
    \returns size of entire header structure
    
    If the header is preceded by a placeable struture, Swap that as well.
*/
int wmfheader_swap(
      char *contents,
      int torev
    ){
    uint32_t Key,Size16w;
    int size=0;
    Key=*(uint32_t *)(contents + offsetof(U_WMRPLACEABLE,Key));
    if(!torev)U_swap4(&Key,1);
    if(Key == 0x9AC6CDD7){
       U_swap4(contents + offsetof(U_WMRPLACEABLE,Key     ),1);
       U_swap2(contents + offsetof(U_WMRPLACEABLE,HWmf    ),1);
       U_swap2(contents + offsetof(U_WMRPLACEABLE,Dst     ),4);
       U_swap2(contents + offsetof(U_WMRPLACEABLE,Inch    ),1);
       U_swap4(contents + offsetof(U_WMRPLACEABLE,Reserved),1);
       U_swap2(contents + offsetof(U_WMRPLACEABLE,Checksum),1);
       contents += U_SIZE_WMRPLACEABLE;
       size     += U_SIZE_WMRPLACEABLE;
    }
    if(torev){  Size16w = *(uint16_t *)(contents + offsetof(U_WMRHEADER,Size16w)); }
    U_swap2(contents + offsetof(U_WMRHEADER,Size16w),2);/* Size16w, Version   */
    if(!torev){ Size16w = *(uint16_t *)(contents + offsetof(U_WMRHEADER,Size16w)); }
    U_swap4(contents + offsetof(U_WMRHEADER,Sizew   ),1);/* Sizew    */
    U_swap2(contents + offsetof(U_WMRHEADER,nObjects),1);/* nObjects */
    U_swap4(contents + offsetof(U_WMRHEADER,maxSize ),1);/* maxSize  */
    U_swap2(contents + offsetof(U_WMRHEADER,nMembers),1);/* nMembers */
    size += 2*Size16w;
    return(size);
}



/* **********************************************************************************************
These functions contain shared code used by various U_WMR*_Swap functions.  These should NEVER be called
by end user code and to further that end prototypes are NOT provided and they are hidden from Doxygen.   
*********************************************************************************************** */

/* Size16 EVERY record type should call this, directly or indirectly*/
void U_WMRCORE_SIZE16_swap(char *record, int torev){
   UNUSED_PARAMETER(torev);
   U_swap4(record, 1); /* Size16_4 is at offset 0 in U_METARECORD */
}


/* Size16, move to data, Single 32bit followed by array of N16 U_POINT16 */
void U_WMRCORE_U32_N16_swap(char *record, int N16, int torev){
   int off=U_SIZE_METARECORD;
   U_WMRCORE_SIZE16_swap(record, torev);
   U_swap4(record + off,   1);  off+=4;
   U_swap2(record + off, N16);
}

/* Single 16bit nPoints followed by array of nPoints U_POINT16 */
void U_WMRCORE_U16_N16_swap(char *record, int torev){
   int nPoints;
   U_WMRCORE_SIZE16_swap(record, torev);
   if(torev){  nPoints = *(uint16_t *)(record + offsetof(U_WMRPOLYGON,nPoints)); }
   U_swap2(record + offsetof(U_WMRPOLYGON,nPoints), 1);
   if(!torev){ nPoints = *(uint16_t *)(record + offsetof(U_WMRPOLYGON,nPoints)); }
   U_swap2(record + offsetof(U_WMRPOLYGON,aPoints), 2*nPoints);
}

/* all records that specify palette objects */
void U_WMRCORE_PALETTE_swap(char *record, int torev){
   UNUSED_PARAMETER(torev);
   U_WMRCORE_SIZE16_swap(record, torev);
   palette_swap(record + offsetof(U_WMRANIMATEPALETTE,Palette));
}

/* all records that have N int16 values, unconditionally swapped  */
void U_WMRCORE_N16_swap(char *record, int N16, int torev){
   U_WMRCORE_SIZE16_swap(record, torev);
   U_swap2(record+U_SIZE_METARECORD, N16);
}


/* like floodfill  */
void U_WMRCORE_U16_CR_2U16_swap(char *record, int torev){
   int off = U_SIZE_METARECORD;
   U_WMRCORE_SIZE16_swap(record, torev);
   U_swap2(record+off,   1);   off += 2 + sizeof(U_COLORREF);
   U_swap2(record+off,   2);
}

/* **********************************************************************************************
These are the core WMR functions, each creates a particular type of record.  
All return these records via a char* pointer, which is NULL if the call failed.  
They are listed in order by the corresponding U_EMR_* index number.  
*********************************************************************************************** */

/**
    \brief Swap a pointer to a U_WMR_whatever record which has not been implemented.
    \param name       name of this type of record
    \param contents   pointer to a buffer holding all EMR records
    \param recnum     number of this record in contents
    \param off        offset to this record in contents
*/
void U_WMRNOTIMPLEMENTED_swap(char *record, int torev){
   U_WMRCORE_SIZE16_swap(record, torev);
}

void U_WMREOF_swap(char *record, int torev){
   U_WMRCORE_SIZE16_swap(record, torev);
}

void U_WMRSETBKCOLOR_swap(char *record, int torev){
   U_WMRCORE_SIZE16_swap(record, torev);
}

void U_WMRSETBKMODE_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,1,torev);
}

void U_WMRSETMAPMODE_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,1,torev);
}

void U_WMRSETROP2_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,1,torev);
}

void U_WMRSETRELABS_swap(char *record, int torev){
   U_WMRCORE_SIZE16_swap(record, torev);
}

void U_WMRSETPOLYFILLMODE_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,1,torev);
}

void U_WMRSETSTRETCHBLTMODE_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,1,torev);
}

void U_WMRSETTEXTCHAREXTRA_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,1,torev);
}

void U_WMRSETTEXTCOLOR_swap(char *record, int torev){
   U_WMRCORE_SIZE16_swap(record, torev);
}

void U_WMRSETTEXTJUSTIFICATION_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,2,torev);
}

void U_WMRSETWINDOWORG_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,2,torev);
}

void U_WMRSETWINDOWEXT_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,2,torev);
}

void U_WMRSETVIEWPORTORG_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,2,torev);
}

void U_WMRSETVIEWPORTEXT_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,2,torev);
}

void U_WMROFFSETWINDOWORG_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,2,torev);
}

void U_WMRSCALEWINDOWEXT_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,4,torev);
}

void U_WMROFFSETVIEWPORTORG_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,2,torev);
}

void U_WMRSCALEVIEWPORTEXT_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,4,torev);
}

void U_WMRLINETO_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,2,torev);
}

void U_WMRMOVETO_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,2,torev);
}

void U_WMREXCLUDECLIPRECT_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,4,torev);
}

void U_WMRINTERSECTCLIPRECT_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,4,torev);
}

void U_WMRARC_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record, 8, torev);
}

void U_WMRELLIPSE_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,4,torev);
}

void U_WMRFLOODFILL_swap(char *record, int torev){
   U_WMRCORE_U16_CR_2U16_swap(record, torev);
}

void U_WMRPIE_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record, 8, torev);
}

void U_WMRRECTANGLE_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,4,torev);
}

void U_WMRROUNDRECT_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,6,torev);
}

void U_WMRPATBLT_swap(char *record, int torev){
    U_WMRCORE_U32_N16_swap(record,4,torev);
}

void U_WMRSAVEDC_swap(char *record, int torev){
   U_WMRCORE_SIZE16_swap(record, torev);
}

void U_WMRSETPIXEL_swap(char *record, int torev){
   int off = U_SIZE_METARECORD + sizeof(U_COLORREF);
   U_WMRCORE_SIZE16_swap(record, torev);
   U_swap2(record+off,   2);
}

void U_WMROFFSETCLIPRGN_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,2,torev);
}

void U_WMRTEXTOUT_swap(char *record, int torev){
   int  L2;
   if(torev){  L2 =  *(int16_t *)(record + U_SIZE_METARECORD); } /* Length field */
   U_WMRCORE_N16_swap(record,1,torev); /* Length */
   if(!torev){ L2 =  *(int16_t *)(record + U_SIZE_METARECORD); } /* Length field */
   /* string is in bytes, do not swap that */
   U_swap2(record + U_SIZE_WMRTEXTOUT + L2, 2); /* y,x */
}

void U_WMRBITBLT_swap(char *record, int torev){
   uint32_t Size16;
   uint8_t xb;
   memcpy(&Size16, record + offsetof(U_METARECORD,Size16_4), 4);
   if(!torev)U_swap4(&Size16,1);
   xb     = *(uint8_t *)(record + offsetof(U_METARECORD,xb));
   if(U_TEST_NOPX2(Size16,xb)){ /* no bitmap */
      U_WMRCORE_U32_N16_swap(record,7,torev);
   }
   else { /* yes bitmap */
      U_WMRCORE_U32_N16_swap(record,6,torev);
      bitmap16_swap(record + offsetof(U_WMRBITBLT_PX,bitmap));
   }
}

void U_WMRSTRETCHBLT_swap(char *record, int torev){
   uint32_t Size16;
   uint8_t xb;
   memcpy(&Size16, record + offsetof(U_METARECORD,Size16_4), 4);
   if(!torev)U_swap4(&Size16,1);
   xb     = *(uint8_t *)(record + offsetof(U_METARECORD,xb));
   if(U_TEST_NOPX2(Size16,xb)){ /* no bitmap */
      U_WMRCORE_U32_N16_swap(record,9,torev);
   }
   else { /* yes bitmap */
      U_WMRCORE_U32_N16_swap(record,8,torev);
      bitmap16_swap(record + offsetof(U_WMRSTRETCHBLT_PX,bitmap));
   }
}

void U_WMRPOLYGON_swap(char *record, int torev){
   U_WMRCORE_U16_N16_swap(record, torev);
}

void U_WMRPOLYLINE_swap(char *record, int torev){
   U_WMRCORE_U16_N16_swap(record, torev);
}

void U_WMRESCAPE_swap(char *record, int torev){
   uint16_t  eFunc;
 
   if(torev){  eFunc = *(uint16_t *)(record + offsetof(U_WMRESCAPE,eFunc)); }
   U_WMRCORE_N16_swap(record,2,torev);
   if(!torev){ eFunc = *(uint16_t *)(record + offsetof(U_WMRESCAPE,eFunc)); }
   /* Handle data swapping for three types only, anything else end user code must handle */
   if((eFunc == U_MFE_SETLINECAP) || (eFunc == U_MFE_SETLINEJOIN) || (eFunc == U_MFE_SETMITERLIMIT)){
      U_swap4(record + offsetof(U_WMRESCAPE, Data),1);
   }
}

void U_WMRRESTOREDC_swap(char *record, int torev){
   U_WMRCORE_SIZE16_swap(record, torev);
}

void U_WMRFILLREGION_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,2,torev);
}

void U_WMRFRAMEREGION_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,4,torev);
}

void U_WMRINVERTREGION_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,1,torev);
}

void U_WMRPAINTREGION_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,1,torev);
}

void U_WMRSELECTCLIPREGION_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,1,torev);
}

void U_WMRSELECTOBJECT_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,1,torev);
}

void U_WMRSETTEXTALIGN_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,1,torev);
}

void U_WMRDRAWTEXT_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRCHORD_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record, 8, torev);
}

void U_WMRSETMAPPERFLAGS_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,1,torev);
}

void U_WMREXTTEXTOUT_swap(char *record, int torev){
   int off,Length,Len2,Opts;
   U_swap4(record + offsetof(U_WMREXTTEXTOUT,Size16_4),1);
   if(torev){ 
      Length = *(int16_t *)( record + offsetof(U_WMREXTTEXTOUT,Length));
      Opts   = *(uint16_t *)(record + offsetof(U_WMREXTTEXTOUT,Opts));
   }
   U_swap2(record + offsetof(U_WMREXTTEXTOUT,y),    4);       /* y,x,Length,Opts*/
   if(!torev){ 
      Length = *(int16_t *)( record + offsetof(U_WMREXTTEXTOUT,Length));
      Opts   = *(uint16_t *)(record + offsetof(U_WMREXTTEXTOUT,Opts));
   }
   off = U_SIZE_WMREXTTEXTOUT;
   if(Opts & (U_ETO_OPAQUE | U_ETO_CLIPPED)){
      U_swap2(record + off,4); off += 8;
   }
   Len2 = (Length & 1 ? Length + 1 : Length);
   off += Len2;  /* no need to swap string, it is a byte array */
   U_swap2(record+off,Length);  /* swap the dx array */
}

void U_WMRSETDIBTODEV_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record, 9, torev);
   dibheader_swap(record + offsetof(U_WMRSETDIBTODEV,dib), torev);
}

void U_WMRSELECTPALETTE_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,1,torev);
}

void U_WMRREALIZEPALETTE_swap(char *record, int torev){
   U_WMRCORE_SIZE16_swap(record, torev);
}

void U_WMRANIMATEPALETTE_swap(char *record, int torev){
   U_WMRCORE_PALETTE_swap(record, torev);
}

void U_WMRSETPALENTRIES_swap(char *record, int torev){
   U_WMRCORE_PALETTE_swap(record, torev);
}

void U_WMRPOLYPOLYGON_swap(char *record, int torev){
   U_WMRCORE_SIZE16_swap(record, torev);
   polypolygon_swap(record + offsetof(U_WMRPOLYPOLYGON,PPolygon), torev);
}

void U_WMRRESIZEPALETTE_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,1,torev);
}

void U_WMR3A_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR3B_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR3C_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR3D_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR3E_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR3F_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRDIBBITBLT_swap(char *record, int torev){
   uint32_t Size16;
   uint8_t xb;
   memcpy(&Size16, record + offsetof(U_METARECORD,Size16_4), 4);
   if(!torev)U_swap4(&Size16,1);
   xb     = *(uint8_t *)(record + offsetof(U_METARECORD,xb));
   if(U_TEST_NOPX2(Size16,xb)){ /* no bitmap */
      U_WMRCORE_U32_N16_swap(record,7,torev);
   }
   else { /* yes bitmap */
      U_WMRCORE_U32_N16_swap(record,6,torev);
      dibheader_swap(record + offsetof(U_WMRDIBBITBLT_PX,dib), torev);
   }
}

void U_WMRDIBSTRETCHBLT_swap(char *record, int torev){
   uint32_t Size16;
   uint8_t xb;
   memcpy(&Size16, record + offsetof(U_METARECORD,Size16_4), 4);
   if(!torev)U_swap4(&Size16,1);
   xb     = *(uint8_t *)(record + offsetof(U_METARECORD,xb));
   if(U_TEST_NOPX2(Size16,xb)){ /* no bitmap */
      U_WMRCORE_U32_N16_swap(record,9,torev);
   }
   else { /* yes bitmap */
      U_WMRCORE_U32_N16_swap(record,8,torev);
      dibheader_swap(record + offsetof(U_WMRDIBSTRETCHBLT_PX,dib), torev);
   }
}

void U_WMRDIBCREATEPATTERNBRUSH_swap(char *record, int torev){
   int Style;
   if(torev){ Style  = *(uint16_t *)(record + offsetof(U_WMRDIBCREATEPATTERNBRUSH,Style)); }
   U_WMRCORE_N16_swap(record,2,torev); /* Style and cUsage */
   if(!torev){ Style  = *(uint16_t *)(record + offsetof(U_WMRDIBCREATEPATTERNBRUSH,Style));  }
   if(Style == U_BS_PATTERN){
      bitmap16_swap(record + offsetof(U_WMRDIBCREATEPATTERNBRUSH,Src));
   }
   else {
      dibheader_swap(record + offsetof(U_WMRDIBCREATEPATTERNBRUSH,Src), torev);
   }
}

void U_WMRSTRETCHDIB_swap(char *record, int torev){
   UNUSED_PARAMETER(torev);
   U_WMRCORE_U32_N16_swap(record,9,torev);
   dibheader_swap(record + offsetof(U_WMRSTRETCHDIB,dib), torev);
}

void U_WMR44_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR45_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR46_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR47_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMREXTFLOODFILL_swap(char *record, int torev){
   U_WMRCORE_U16_CR_2U16_swap(record, torev);
}

void U_WMR49_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR4A_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR4B_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR4C_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR4D_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR4E_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR4F_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR50_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR51_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR52_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR53_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR54_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR55_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR56_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR57_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR58_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR59_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR5A_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR5B_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR5C_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR5D_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR5E_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR5F_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR60_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR61_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR62_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR63_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR64_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR65_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR66_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR67_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR68_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR69_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR6A_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR6B_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR6C_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR6D_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR6E_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR6F_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR70_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR71_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR72_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR73_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR74_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR75_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR76_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR77_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR78_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR79_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR7A_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR7B_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR7C_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR7D_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR7E_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR7F_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR80_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR81_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR82_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR83_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR84_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR85_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR86_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR87_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR88_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR89_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR8A_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR8B_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR8C_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR8D_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR8E_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR8F_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR90_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR91_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR92_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR93_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR94_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR95_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR96_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR97_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR98_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR99_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR9A_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR9B_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR9C_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR9D_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR9E_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMR9F_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRA0_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRA1_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRA2_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRA3_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRA4_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRA5_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRA6_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRA7_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRA8_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRA9_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRAA_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRAB_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRAC_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRAD_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRAE_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRAF_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRB0_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRB1_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRB2_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRB3_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRB4_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRB5_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRB6_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRB7_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRB8_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRB9_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRBA_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRBB_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRBC_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRBD_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRBE_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRBF_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRC0_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRC1_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRC2_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRC3_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRC4_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRC5_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRC6_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRC7_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRC8_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRC9_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRCA_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRCB_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRCC_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRCD_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRCE_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRCF_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRD0_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRD1_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRD2_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRD3_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRD4_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRD5_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRD6_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRD7_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRD8_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRD9_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRDA_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRDB_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRDC_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRDD_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRDE_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRDF_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRE0_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRE1_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRE2_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRE3_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRE4_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRE5_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRE6_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRE7_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRE8_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRE9_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMREA_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMREB_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMREC_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRED_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMREE_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMREF_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRDELETEOBJECT_swap(char *record, int torev){
   U_WMRCORE_N16_swap(record,1,torev);
}

void U_WMRF1_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRF2_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRF3_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRF4_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRF5_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRF6_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRCREATEPALETTE_swap(char *record, int torev){
   U_WMRCORE_PALETTE_swap(record, torev);
}

void U_WMRF8_swap(char *record, int torev){
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRCREATEPATTERNBRUSH_swap(char *record, int torev){
   U_WMRCORE_SIZE16_swap(record, torev);
   bitmap16_swap(record + U_SIZE_METARECORD);
   /* pattern array byte order already correct? */
}

void U_WMRCREATEPENINDIRECT_swap(char *record, int torev){
   U_WMRCORE_SIZE16_swap(record, torev);
   pen_swap(record + offsetof(U_WMRCREATEPENINDIRECT,pen));
}

void U_WMRCREATEFONTINDIRECT_swap(char *record, int torev){
   U_WMRCORE_SIZE16_swap(record, torev);
   font_swap(record + offsetof(U_WMRCREATEFONTINDIRECT,font));
}

void U_WMRCREATEBRUSHINDIRECT_swap(char *record, int torev){
   U_WMRCORE_SIZE16_swap(record, torev);
   wlogbrush_swap(record + offsetof(U_WMRCREATEBRUSHINDIRECT,brush));
}

void U_WMRCREATEBITMAPINDIRECT_swap(char *record, int torev){ /* in Wine, not in WMF PDF */
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRCREATEBITMAP_swap(char *record, int torev){ /* in Wine, not in WMF PDF */
   U_WMRNOTIMPLEMENTED_swap(record, torev);
}

void U_WMRCREATEREGION_swap(char *record, int torev){
   U_WMRCORE_SIZE16_swap(record, torev);
   region_swap(record + offsetof(U_WMRCREATEREGION,region), torev);
}
//! \endcond

/**
    \brief Convert an entire WMF in memory from Big Endian to Little Endian (or vice versa).
    \return 0 on failure, 1 on success
    \param contents   pointer to the buffer holding the entire EMF in memory
    \param length     number of bytes in the buffer
    \param torev      1 for native to reversed, 0 for reversed to native
    \param onerec     1 if this is operating on a single record instead of an entire file
    
    Normally this would be called immediately before writing the data to a file
    or immediately after reading the data from a file.
*/
int U_wmf_endian(char *contents, size_t length, int torev, int onerec){
    size_t    off=0;
    uint32_t  OK, Size16;
    uint8_t   iType;
    char     *record;
    int       recnum;
    int       offset=0;

    record  = contents;
    if(!onerec){
       off     = wmfheader_swap(record,torev); fflush(stdout); /* WMF header is not a normal record, handle it separately */
       record += off;
       offset  = off;
    }
    OK      = 1;
    recnum  = 1;  /* used when debugging */

    while(OK){
       if(record > contents + length){ return(0); } // this is most likely a corrupt WMF

       memcpy(&Size16, record + offsetof(U_METARECORD,Size16_4), 4);  /* This may not be aligned */
       if(!torev){ U_swap4(&Size16,1); }
       iType  = *(uint8_t *)(record + offsetof(U_METARECORD,iType));

//printf("DEBUG U_wmf_endian before switch record:%d offset:%d type:%d name:%s Size16:%d\n",recnum,offset,iType,U_wmr_names(iType),Size16);fflush(stdout);
       switch (iType)
       {
	  case  U_WMR_EOF:                    U_WMREOF_swap(record, torev);     OK=0;            break;
	  case  U_WMR_SETBKCOLOR:             U_WMRSETBKCOLOR_swap(record, torev);               break;
	  case  U_WMR_SETBKMODE:              U_WMRSETBKMODE_swap(record, torev);                break;
	  case  U_WMR_SETMAPMODE:             U_WMRSETMAPMODE_swap(record, torev);               break;
	  case  U_WMR_SETROP2:                U_WMRSETROP2_swap(record, torev);                  break;
	  case  U_WMR_SETRELABS:              U_WMRSETRELABS_swap(record, torev);                break;
	  case  U_WMR_SETPOLYFILLMODE:        U_WMRSETPOLYFILLMODE_swap(record, torev);          break;
	  case  U_WMR_SETSTRETCHBLTMODE:      U_WMRSETSTRETCHBLTMODE_swap(record, torev);        break;
	  case  U_WMR_SETTEXTCHAREXTRA:       U_WMRSETTEXTCHAREXTRA_swap(record, torev);         break;
	  case  U_WMR_SETTEXTCOLOR:           U_WMRSETTEXTCOLOR_swap(record, torev);             break;
	  case  U_WMR_SETTEXTJUSTIFICATION:   U_WMRSETTEXTJUSTIFICATION_swap(record, torev);     break;
	  case  U_WMR_SETWINDOWORG:           U_WMRSETWINDOWORG_swap(record, torev);             break;
	  case  U_WMR_SETWINDOWEXT:           U_WMRSETWINDOWEXT_swap(record, torev);             break;
	  case  U_WMR_SETVIEWPORTORG:         U_WMRSETVIEWPORTORG_swap(record, torev);           break;
	  case  U_WMR_SETVIEWPORTEXT:         U_WMRSETVIEWPORTEXT_swap(record, torev);           break;
	  case  U_WMR_OFFSETWINDOWORG:        U_WMROFFSETWINDOWORG_swap(record, torev);          break;
	  case  U_WMR_SCALEWINDOWEXT:         U_WMRSCALEWINDOWEXT_swap(record, torev);           break;
	  case  U_WMR_OFFSETVIEWPORTORG:      U_WMROFFSETVIEWPORTORG_swap(record, torev);        break;
	  case  U_WMR_SCALEVIEWPORTEXT:       U_WMRSCALEVIEWPORTEXT_swap(record, torev);         break;
	  case  U_WMR_LINETO:                 U_WMRLINETO_swap(record, torev);                   break;
	  case  U_WMR_MOVETO:                 U_WMRMOVETO_swap(record, torev);                   break;
	  case  U_WMR_EXCLUDECLIPRECT:        U_WMREXCLUDECLIPRECT_swap(record, torev);          break;
	  case  U_WMR_INTERSECTCLIPRECT:      U_WMRINTERSECTCLIPRECT_swap(record, torev);        break;
	  case  U_WMR_ARC:                    U_WMRARC_swap(record, torev);                      break;
	  case  U_WMR_ELLIPSE:                U_WMRELLIPSE_swap(record, torev);                  break;
	  case  U_WMR_FLOODFILL:              U_WMRFLOODFILL_swap(record, torev);                break;
	  case  U_WMR_PIE:                    U_WMRPIE_swap(record, torev);                      break;
	  case  U_WMR_RECTANGLE:              U_WMRRECTANGLE_swap(record, torev);                break;
	  case  U_WMR_ROUNDRECT:              U_WMRROUNDRECT_swap(record, torev);                break;
	  case  U_WMR_PATBLT:                 U_WMRPATBLT_swap(record, torev);                   break;
	  case  U_WMR_SAVEDC:                 U_WMRSAVEDC_swap(record, torev);                   break;
	  case  U_WMR_SETPIXEL:               U_WMRSETPIXEL_swap(record, torev);                 break;
	  case  U_WMR_OFFSETCLIPRGN:          U_WMROFFSETCLIPRGN_swap(record, torev);            break;
	  case  U_WMR_TEXTOUT:                U_WMRTEXTOUT_swap(record, torev);                  break;
	  case  U_WMR_BITBLT:                 U_WMRBITBLT_swap(record, torev);                   break;
	  case  U_WMR_STRETCHBLT:             U_WMRSTRETCHBLT_swap(record, torev);               break;
	  case  U_WMR_POLYGON:                U_WMRPOLYGON_swap(record, torev);                  break;
	  case  U_WMR_POLYLINE:               U_WMRPOLYLINE_swap(record, torev);                 break;
	  case  U_WMR_ESCAPE:                 U_WMRESCAPE_swap(record, torev);                   break;
	  case  U_WMR_RESTOREDC:              U_WMRRESTOREDC_swap(record, torev);                break;
	  case  U_WMR_FILLREGION:             U_WMRFILLREGION_swap(record, torev);               break;
	  case  U_WMR_FRAMEREGION:            U_WMRFRAMEREGION_swap(record, torev);              break;
	  case  U_WMR_INVERTREGION:           U_WMRINVERTREGION_swap(record, torev);             break;
	  case  U_WMR_PAINTREGION:            U_WMRPAINTREGION_swap(record, torev);              break;
	  case  U_WMR_SELECTCLIPREGION:       U_WMRSELECTCLIPREGION_swap(record, torev);         break;
	  case  U_WMR_SELECTOBJECT:           U_WMRSELECTOBJECT_swap(record, torev);             break;
	  case  U_WMR_SETTEXTALIGN:           U_WMRSETTEXTALIGN_swap(record, torev);             break;
	  case  U_WMR_DRAWTEXT:               U_WMRDRAWTEXT_swap(record, torev);                 break;
	  case  U_WMR_CHORD:                  U_WMRCHORD_swap(record, torev);                    break;
	  case  U_WMR_SETMAPPERFLAGS:         U_WMRSETMAPPERFLAGS_swap(record, torev);           break;
	  case  U_WMR_EXTTEXTOUT:             U_WMREXTTEXTOUT_swap(record, torev);               break;
	  case  U_WMR_SETDIBTODEV:            U_WMRSETDIBTODEV_swap(record, torev);              break;
	  case  U_WMR_SELECTPALETTE:          U_WMRSELECTPALETTE_swap(record, torev);            break;
	  case  U_WMR_REALIZEPALETTE:         U_WMRREALIZEPALETTE_swap(record, torev);           break;
	  case  U_WMR_ANIMATEPALETTE:         U_WMRANIMATEPALETTE_swap(record, torev);           break;
	  case  U_WMR_SETPALENTRIES:          U_WMRSETPALENTRIES_swap(record, torev);            break;
	  case  U_WMR_POLYPOLYGON:            U_WMRPOLYPOLYGON_swap(record, torev);              break;
	  case  U_WMR_RESIZEPALETTE:          U_WMRRESIZEPALETTE_swap(record, torev);            break;
	  case  U_WMR_3A:                     U_WMR3A_swap(record, torev);                       break;
	  case  U_WMR_3B:                     U_WMR3B_swap(record, torev);                       break;
	  case  U_WMR_3C:                     U_WMR3C_swap(record, torev);                       break;
	  case  U_WMR_3D:                     U_WMR3D_swap(record, torev);                       break;
	  case  U_WMR_3E:                     U_WMR3E_swap(record, torev);                       break;
	  case  U_WMR_3F:                     U_WMR3F_swap(record, torev);                       break;
	  case  U_WMR_DIBBITBLT:              U_WMRDIBBITBLT_swap(record, torev);                break;
	  case  U_WMR_DIBSTRETCHBLT:          U_WMRDIBSTRETCHBLT_swap(record, torev);            break;
	  case  U_WMR_DIBCREATEPATTERNBRUSH:  U_WMRDIBCREATEPATTERNBRUSH_swap(record, torev);    break;
	  case  U_WMR_STRETCHDIB:             U_WMRSTRETCHDIB_swap(record, torev);               break;
	  case  U_WMR_44:                     U_WMR44_swap(record, torev);                       break;
	  case  U_WMR_45:                     U_WMR45_swap(record, torev);                       break;
	  case  U_WMR_46:                     U_WMR46_swap(record, torev);                       break;
	  case  U_WMR_47:                     U_WMR47_swap(record, torev);                       break;
	  case  U_WMR_EXTFLOODFILL:           U_WMREXTFLOODFILL_swap(record, torev);             break;
	  case  U_WMR_49:                     U_WMR49_swap(record, torev);                       break;
	  case  U_WMR_4A:                     U_WMR4A_swap(record, torev);                       break;
	  case  U_WMR_4B:                     U_WMR4B_swap(record, torev);                       break;
	  case  U_WMR_4C:                     U_WMR4C_swap(record, torev);                       break;
	  case  U_WMR_4D:                     U_WMR4D_swap(record, torev);                       break;
	  case  U_WMR_4E:                     U_WMR4E_swap(record, torev);                       break;
	  case  U_WMR_4F:                     U_WMR4F_swap(record, torev);                       break;
	  case  U_WMR_50:                     U_WMR50_swap(record, torev);                       break;
	  case  U_WMR_51:                     U_WMR51_swap(record, torev);                       break;
	  case  U_WMR_52:                     U_WMR52_swap(record, torev);                       break;
	  case  U_WMR_53:                     U_WMR53_swap(record, torev);                       break;
	  case  U_WMR_54:                     U_WMR54_swap(record, torev);                       break;
	  case  U_WMR_55:                     U_WMR55_swap(record, torev);                       break;
	  case  U_WMR_56:                     U_WMR56_swap(record, torev);                       break;
	  case  U_WMR_57:                     U_WMR57_swap(record, torev);                       break;
	  case  U_WMR_58:                     U_WMR58_swap(record, torev);                       break;
	  case  U_WMR_59:                     U_WMR59_swap(record, torev);                       break;
	  case  U_WMR_5A:                     U_WMR5A_swap(record, torev);                       break;
	  case  U_WMR_5B:                     U_WMR5B_swap(record, torev);                       break;
	  case  U_WMR_5C:                     U_WMR5C_swap(record, torev);                       break;
	  case  U_WMR_5D:                     U_WMR5D_swap(record, torev);                       break;
	  case  U_WMR_5E:                     U_WMR5E_swap(record, torev);                       break;
	  case  U_WMR_5F:                     U_WMR5F_swap(record, torev);                       break;
	  case  U_WMR_60:                     U_WMR60_swap(record, torev);                       break;
	  case  U_WMR_61:                     U_WMR61_swap(record, torev);                       break;
	  case  U_WMR_62:                     U_WMR62_swap(record, torev);                       break;
	  case  U_WMR_63:                     U_WMR63_swap(record, torev);                       break;
	  case  U_WMR_64:                     U_WMR64_swap(record, torev);                       break;
	  case  U_WMR_65:                     U_WMR65_swap(record, torev);                       break;
	  case  U_WMR_66:                     U_WMR66_swap(record, torev);                       break;
	  case  U_WMR_67:                     U_WMR67_swap(record, torev);                       break;
	  case  U_WMR_68:                     U_WMR68_swap(record, torev);                       break;
	  case  U_WMR_69:                     U_WMR69_swap(record, torev);                       break;
	  case  U_WMR_6A:                     U_WMR6A_swap(record, torev);                       break;
	  case  U_WMR_6B:                     U_WMR6B_swap(record, torev);                       break;
	  case  U_WMR_6C:                     U_WMR6C_swap(record, torev);                       break;
	  case  U_WMR_6D:                     U_WMR6D_swap(record, torev);                       break;
	  case  U_WMR_6E:                     U_WMR6E_swap(record, torev);                       break;
	  case  U_WMR_6F:                     U_WMR6F_swap(record, torev);                       break;
	  case  U_WMR_70:                     U_WMR70_swap(record, torev);                       break;
	  case  U_WMR_71:                     U_WMR71_swap(record, torev);                       break;
	  case  U_WMR_72:                     U_WMR72_swap(record, torev);                       break;
	  case  U_WMR_73:                     U_WMR73_swap(record, torev);                       break;
	  case  U_WMR_74:                     U_WMR74_swap(record, torev);                       break;
	  case  U_WMR_75:                     U_WMR75_swap(record, torev);                       break;
	  case  U_WMR_76:                     U_WMR76_swap(record, torev);                       break;
	  case  U_WMR_77:                     U_WMR77_swap(record, torev);                       break;
	  case  U_WMR_78:                     U_WMR78_swap(record, torev);                       break;
	  case  U_WMR_79:                     U_WMR79_swap(record, torev);                       break;
	  case  U_WMR_7A:                     U_WMR7A_swap(record, torev);                       break;
	  case  U_WMR_7B:                     U_WMR7B_swap(record, torev);                       break;
	  case  U_WMR_7C:                     U_WMR7C_swap(record, torev);                       break;
	  case  U_WMR_7D:                     U_WMR7D_swap(record, torev);                       break;
	  case  U_WMR_7E:                     U_WMR7E_swap(record, torev);                       break;
	  case  U_WMR_7F:                     U_WMR7F_swap(record, torev);                       break;
	  case  U_WMR_80:                     U_WMR80_swap(record, torev);                       break;
	  case  U_WMR_81:                     U_WMR81_swap(record, torev);                       break;
	  case  U_WMR_82:                     U_WMR82_swap(record, torev);                       break;
	  case  U_WMR_83:                     U_WMR83_swap(record, torev);                       break;
	  case  U_WMR_84:                     U_WMR84_swap(record, torev);                       break;
	  case  U_WMR_85:                     U_WMR85_swap(record, torev);                       break;
	  case  U_WMR_86:                     U_WMR86_swap(record, torev);                       break;
	  case  U_WMR_87:                     U_WMR87_swap(record, torev);                       break;
	  case  U_WMR_88:                     U_WMR88_swap(record, torev);                       break;
	  case  U_WMR_89:                     U_WMR89_swap(record, torev);                       break;
	  case  U_WMR_8A:                     U_WMR8A_swap(record, torev);                       break;
	  case  U_WMR_8B:                     U_WMR8B_swap(record, torev);                       break;
	  case  U_WMR_8C:                     U_WMR8C_swap(record, torev);                       break;
	  case  U_WMR_8D:                     U_WMR8D_swap(record, torev);                       break;
	  case  U_WMR_8E:                     U_WMR8E_swap(record, torev);                       break;
	  case  U_WMR_8F:                     U_WMR8F_swap(record, torev);                       break;
	  case  U_WMR_90:                     U_WMR90_swap(record, torev);                       break;
	  case  U_WMR_91:                     U_WMR91_swap(record, torev);                       break;
	  case  U_WMR_92:                     U_WMR92_swap(record, torev);                       break;
	  case  U_WMR_93:                     U_WMR93_swap(record, torev);                       break;
	  case  U_WMR_94:                     U_WMR94_swap(record, torev);                       break;
	  case  U_WMR_95:                     U_WMR95_swap(record, torev);                       break;
	  case  U_WMR_96:                     U_WMR96_swap(record, torev);                       break;
	  case  U_WMR_97:                     U_WMR97_swap(record, torev);                       break;
	  case  U_WMR_98:                     U_WMR98_swap(record, torev);                       break;
	  case  U_WMR_99:                     U_WMR99_swap(record, torev);                       break;
	  case  U_WMR_9A:                     U_WMR9A_swap(record, torev);                       break;
	  case  U_WMR_9B:                     U_WMR9B_swap(record, torev);                       break;
	  case  U_WMR_9C:                     U_WMR9C_swap(record, torev);                       break;
	  case  U_WMR_9D:                     U_WMR9D_swap(record, torev);                       break;
	  case  U_WMR_9E:                     U_WMR9E_swap(record, torev);                       break;
	  case  U_WMR_9F:                     U_WMR9F_swap(record, torev);                       break;
	  case  U_WMR_A0:                     U_WMRA0_swap(record, torev);                       break;
	  case  U_WMR_A1:                     U_WMRA1_swap(record, torev);                       break;
	  case  U_WMR_A2:                     U_WMRA2_swap(record, torev);                       break;
	  case  U_WMR_A3:                     U_WMRA3_swap(record, torev);                       break;
	  case  U_WMR_A4:                     U_WMRA4_swap(record, torev);                       break;
	  case  U_WMR_A5:                     U_WMRA5_swap(record, torev);                       break;
	  case  U_WMR_A6:                     U_WMRA6_swap(record, torev);                       break;
	  case  U_WMR_A7:                     U_WMRA7_swap(record, torev);                       break;
	  case  U_WMR_A8:                     U_WMRA8_swap(record, torev);                       break;
	  case  U_WMR_A9:                     U_WMRA9_swap(record, torev);                       break;
	  case  U_WMR_AA:                     U_WMRAA_swap(record, torev);                       break;
	  case  U_WMR_AB:                     U_WMRAB_swap(record, torev);                       break;
	  case  U_WMR_AC:                     U_WMRAC_swap(record, torev);                       break;
	  case  U_WMR_AD:                     U_WMRAD_swap(record, torev);                       break;
	  case  U_WMR_AE:                     U_WMRAE_swap(record, torev);                       break;
	  case  U_WMR_AF:                     U_WMRAF_swap(record, torev);                       break;
	  case  U_WMR_B0:                     U_WMRB0_swap(record, torev);                       break;
	  case  U_WMR_B1:                     U_WMRB1_swap(record, torev);                       break;
	  case  U_WMR_B2:                     U_WMRB2_swap(record, torev);                       break;
	  case  U_WMR_B3:                     U_WMRB3_swap(record, torev);                       break;
	  case  U_WMR_B4:                     U_WMRB4_swap(record, torev);                       break;
	  case  U_WMR_B5:                     U_WMRB5_swap(record, torev);                       break;
	  case  U_WMR_B6:                     U_WMRB6_swap(record, torev);                       break;
	  case  U_WMR_B7:                     U_WMRB7_swap(record, torev);                       break;
	  case  U_WMR_B8:                     U_WMRB8_swap(record, torev);                       break;
	  case  U_WMR_B9:                     U_WMRB9_swap(record, torev);                       break;
	  case  U_WMR_BA:                     U_WMRBA_swap(record, torev);                       break;
	  case  U_WMR_BB:                     U_WMRBB_swap(record, torev);                       break;
	  case  U_WMR_BC:                     U_WMRBC_swap(record, torev);                       break;
	  case  U_WMR_BD:                     U_WMRBD_swap(record, torev);                       break;
	  case  U_WMR_BE:                     U_WMRBE_swap(record, torev);                       break;
	  case  U_WMR_BF:                     U_WMRBF_swap(record, torev);                       break;
	  case  U_WMR_C0:                     U_WMRC0_swap(record, torev);                       break;
	  case  U_WMR_C1:                     U_WMRC1_swap(record, torev);                       break;
	  case  U_WMR_C2:                     U_WMRC2_swap(record, torev);                       break;
	  case  U_WMR_C3:                     U_WMRC3_swap(record, torev);                       break;
	  case  U_WMR_C4:                     U_WMRC4_swap(record, torev);                       break;
	  case  U_WMR_C5:                     U_WMRC5_swap(record, torev);                       break;
	  case  U_WMR_C6:                     U_WMRC6_swap(record, torev);                       break;
	  case  U_WMR_C7:                     U_WMRC7_swap(record, torev);                       break;
	  case  U_WMR_C8:                     U_WMRC8_swap(record, torev);                       break;
	  case  U_WMR_C9:                     U_WMRC9_swap(record, torev);                       break;
	  case  U_WMR_CA:                     U_WMRCA_swap(record, torev);                       break;
	  case  U_WMR_CB:                     U_WMRCB_swap(record, torev);                       break;
	  case  U_WMR_CC:                     U_WMRCC_swap(record, torev);                       break;
	  case  U_WMR_CD:                     U_WMRCD_swap(record, torev);                       break;
	  case  U_WMR_CE:                     U_WMRCE_swap(record, torev);                       break;
	  case  U_WMR_CF:                     U_WMRCF_swap(record, torev);                       break;
	  case  U_WMR_D0:                     U_WMRD0_swap(record, torev);                       break;
	  case  U_WMR_D1:                     U_WMRD1_swap(record, torev);                       break;
	  case  U_WMR_D2:                     U_WMRD2_swap(record, torev);                       break;
	  case  U_WMR_D3:                     U_WMRD3_swap(record, torev);                       break;
	  case  U_WMR_D4:                     U_WMRD4_swap(record, torev);                       break;
	  case  U_WMR_D5:                     U_WMRD5_swap(record, torev);                       break;
	  case  U_WMR_D6:                     U_WMRD6_swap(record, torev);                       break;
	  case  U_WMR_D7:                     U_WMRD7_swap(record, torev);                       break;
	  case  U_WMR_D8:                     U_WMRD8_swap(record, torev);                       break;
	  case  U_WMR_D9:                     U_WMRD9_swap(record, torev);                       break;
	  case  U_WMR_DA:                     U_WMRDA_swap(record, torev);                       break;
	  case  U_WMR_DB:                     U_WMRDB_swap(record, torev);                       break;
	  case  U_WMR_DC:                     U_WMRDC_swap(record, torev);                       break;
	  case  U_WMR_DD:                     U_WMRDD_swap(record, torev);                       break;
	  case  U_WMR_DE:                     U_WMRDE_swap(record, torev);                       break;
	  case  U_WMR_DF:                     U_WMRDF_swap(record, torev);                       break;
	  case  U_WMR_E0:                     U_WMRE0_swap(record, torev);                       break;
	  case  U_WMR_E1:                     U_WMRE1_swap(record, torev);                       break;
	  case  U_WMR_E2:                     U_WMRE2_swap(record, torev);                       break;
	  case  U_WMR_E3:                     U_WMRE3_swap(record, torev);                       break;
	  case  U_WMR_E4:                     U_WMRE4_swap(record, torev);                       break;
	  case  U_WMR_E5:                     U_WMRE5_swap(record, torev);                       break;
	  case  U_WMR_E6:                     U_WMRE6_swap(record, torev);                       break;
	  case  U_WMR_E7:                     U_WMRE7_swap(record, torev);                       break;
	  case  U_WMR_E8:                     U_WMRE8_swap(record, torev);                       break;
	  case  U_WMR_E9:                     U_WMRE9_swap(record, torev);                       break;
	  case  U_WMR_EA:                     U_WMREA_swap(record, torev);                       break;
	  case  U_WMR_EB:                     U_WMREB_swap(record, torev);                       break;
	  case  U_WMR_EC:                     U_WMREC_swap(record, torev);                       break;
	  case  U_WMR_ED:                     U_WMRED_swap(record, torev);                       break;
	  case  U_WMR_EE:                     U_WMREE_swap(record, torev);                       break;
	  case  U_WMR_EF:                     U_WMREF_swap(record, torev);                       break;
	  case  U_WMR_DELETEOBJECT:           U_WMRDELETEOBJECT_swap(record, torev);             break;
	  case  U_WMR_F1:                     U_WMRF1_swap(record, torev);                       break;
	  case  U_WMR_F2:                     U_WMRF2_swap(record, torev);                       break;
	  case  U_WMR_F3:                     U_WMRF3_swap(record, torev);                       break;
	  case  U_WMR_F4:                     U_WMRF4_swap(record, torev);                       break;
	  case  U_WMR_F5:                     U_WMRF5_swap(record, torev);                       break;
	  case  U_WMR_F6:                     U_WMRF6_swap(record, torev);                       break;
	  case  U_WMR_CREATEPALETTE:          U_WMRCREATEPALETTE_swap(record, torev);            break;
	  case  U_WMR_F8:                     U_WMRF8_swap(record, torev);                       break;
	  case  U_WMR_CREATEPATTERNBRUSH:     U_WMRCREATEPATTERNBRUSH_swap(record, torev);       break;
	  case  U_WMR_CREATEPENINDIRECT:      U_WMRCREATEPENINDIRECT_swap(record, torev);        break;
	  case  U_WMR_CREATEFONTINDIRECT:     U_WMRCREATEFONTINDIRECT_swap(record, torev);       break;
	  case  U_WMR_CREATEBRUSHINDIRECT:    U_WMRCREATEBRUSHINDIRECT_swap(record, torev);      break;
	  case  U_WMR_CREATEBITMAPINDIRECT:   U_WMRCREATEBITMAPINDIRECT_swap(record, torev);     break;
	  case  U_WMR_CREATEBITMAP:           U_WMRCREATEBITMAP_swap(record, torev);             break;
	  case  U_WMR_CREATEREGION:           U_WMRCREATEREGION_swap(record, torev);             break;
	  default:                            U_WMRNOTIMPLEMENTED_swap(record, torev);           break;
       }  //end of switch
       if(onerec)break;
       record += 2*Size16;
       offset += 2*Size16;
       recnum++;
    }  //end of while
    return(1);
}


#ifdef __cplusplus
}
#endif
