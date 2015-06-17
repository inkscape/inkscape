/**
  @file uwmf_print.c
  
  @brief Functions for printing WMF records
*/

/*
File:      uwmf_print.c
Version:   0.0.6
Date:      21-MAY-2015
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2015 David Mathog and California Institute of Technology (Caltech)
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h> /* for offsetof() macro */
#include <string.h>
#include "uwmf_print.h"

//! \cond

#define UNUSED(x) (void)(x)

/* **********************************************************************************************
   These functions print standard objects used in the WMR records.
   The low level ones do not append EOL.
*********************************************************************************************** */

/* many of these are implemented in uemf_print.c and not replicated here */



/**
    \brief Print a U_BRUSH object.
    \param b U_BRUSH object.
  style              bColor               bHatch
  U_BS_SOLID           ColorRef Object      Not used (bytes present???)
  U_BS_NULL            ignored              ignored  (bytes present???).
  U_BS_PATTERN         ignored              Bitmap16 object holding patern
  U_BS_DIBPATTERNPT    ColorUsage Enum      DIB object
  U_BS_HATCHED         ColorRef Object      HatchStyle Enumeration
*/
void brush_print(
      U_BRUSH b  
   ){
    uint16_t hatch;
    U_COLORREF Color;
    switch(b.Style){
       case U_BS_SOLID:
          memcpy(&Color, &b.Color, sizeof(U_COLORREF));
          printf("Color:");         colorref_print(Color);
          break;
       case U_BS_NULL:
          printf("Null");
          break;
       case U_BS_PATTERN:
          printf("Pattern:(not shown)");
          break;
       case U_BS_DIBPATTERNPT:
          printf("DIBPattern:(not shown)");
          break;
       case U_BS_HATCHED:
          memcpy(&hatch, b.Data, 2);
          printf("Hatch:0x%4.4X ", hatch);
          break;
    }
}

/**
    \brief Print a U_FONT object from a char *pointer.  
    The U_FONT struct object may not be properly aligned, but all of the fields within it will
    OK for alignment.
    \param font U_FONT object (as a char * pointer)
*/
void font_print(
       const char *font
   ){
   printf("Height:%d ",              *(int16_t *)(font + offsetof(U_FONT,Height        )));
   printf("Width:%d ",               *(int16_t *)(font + offsetof(U_FONT,Width         )));
   printf("Escapement:%d ",          *(int16_t *)(font + offsetof(U_FONT,Escapement    )));
   printf("Orientation:%d ",         *(int16_t *)(font + offsetof(U_FONT,Orientation   )));
   printf("Weight:%d ",              *(int16_t *)(font + offsetof(U_FONT,Weight        )));
   printf("Italic:0x%2.2X ",         *(uint8_t *)(font + offsetof(U_FONT,Italic        )));
   printf("Underline:0x%2.2X ",      *(uint8_t *)(font + offsetof(U_FONT,Underline     )));
   printf("StrikeOut:0x%2.2X ",      *(uint8_t *)(font + offsetof(U_FONT,StrikeOut     )));
   printf("CharSet:0x%2.2X ",        *(uint8_t *)(font + offsetof(U_FONT,CharSet       )));
   printf("OutPrecision:0x%2.2X ",   *(uint8_t *)(font + offsetof(U_FONT,OutPrecision  )));
   printf("ClipPrecision:0x%2.2X ",  *(uint8_t *)(font + offsetof(U_FONT,ClipPrecision )));
   printf("Quality:0x%2.2X ",        *(uint8_t *)(font + offsetof(U_FONT,Quality       )));
   printf("PitchAndFamily:0x%2.2X ", *(uint8_t *)(font + offsetof(U_FONT,PitchAndFamily)));
   printf("FaceName:%s ",                        (font + offsetof(U_FONT,FaceName      )));
}


/**
    \brief Print a U_PLTNTRY object.
    \param pny U_PLTNTRY object.
*/
void pltntry_print(
      U_PLTNTRY pny
   ){
   printf("Value:%u ", pny.Value);
   printf("Red:%u ",   pny.Red  );
   printf("Green:%u ", pny.Green);
   printf("Blue:%u ",  pny.Blue );
}


/**
    \brief Print a pointer to a U_PALETTE object.
    \param p          Pointer to a U_PALETTE object
    \param PalEntries Array of Palette Entries
*/
void palette_print(
      const U_PALETTE  *p,
      const char       *PalEntries
   ){
   int            i;
   U_PLTNTRY pny;

   printf("Start:%X ", p->Start );
   printf("NumEntries:%u ",p->NumEntries );
   if(p->NumEntries && PalEntries){
     for(i=0; i < p->NumEntries; i++, PalEntries += sizeof(U_PLTNTRY)){
        memcpy(&pny, PalEntries, sizeof(U_PLTNTRY));
        printf("%d:",i); pltntry_print(pny);
     }
   }
}

/**
    \brief Print a  U_PEN object.
    \param p   U_PEN object
   uint16_t            Style;              //!< PenStyle Enumeration
    uint16_t            Width;              //!< Pen Width in object dimensions
    uint16_t            unused;             //!< unused
    union {
      U_COLORREF        Color;              //!< Pen color (NOT aligned on 4n byte boundary!)
      uint16_t          Colorw[2];          //!< reassemble/store the Color value using these, NOT Color.
    };
*/
void pen_print(
      U_PEN p
   ){
   U_COLORREF Color;
   printf("Style:0x%8.8X "      ,p.Style  );
   printf("Width:%u "           ,p.Widthw[0]  );
   memcpy(&Color, &p.Color, sizeof(U_COLORREF));
   printf("Color");              colorref_print(Color);
}

/**
    \brief Print U_RECT16 object
    Prints in order left, top, right, bottom
    \param rect U_RECT16 object
*/
void rect16_ltrb_print(
      U_RECT16 rect
    ){
    printf("LTRB{%d,%d,%d,%d} ",rect.left,rect.top,rect.right,rect.bottom);
}

/**
    \brief Print U_RECT16 object
    Some WMF rects use the order bottom, right, top, left.  These are passed in using
    the same structure as for a normal U_RECT16 so:
       position holds
       left     bottom
       top      right
       right    top
       bottom   left
    This is used by WMR_RECTANGLE and many others.
    \param rect U_RECT16 object
*/
void rect16_brtl_print(
      U_RECT16 rect
    ){
    printf("BRTL{%d,%d,%d,%d} ",rect.bottom,rect.right,rect.top,rect.left);
}



/**
    \brief Print U_REGION object from a char * pointer.
    \param region U_REGION object
*/
void region_print(
      const char *region
    ){
    U_RECT16 rect16;
    printf("Type:%d ",  *(uint16_t *)(region + offsetof(U_REGION,Type  )));
    printf("Size:%d ",  *( int16_t *)(region + offsetof(U_REGION,Size  )));
    printf("sCount:%d ",*( int16_t *)(region + offsetof(U_REGION,sCount)));
    printf("sMax:%d ",  *( int16_t *)(region + offsetof(U_REGION,sMax  )));
    memcpy(&rect16,                  (region + offsetof(U_REGION,sRect )),  sizeof(U_RECT16));
    printf("sRect: ");  rect16_ltrb_print(rect16);
}


/**
    \brief Print U_BITMAP16 object
    \param b U_BITMAP16 object
*/
void bitmap16_print(
      U_BITMAP16 b
    ){
    printf("Type:%d ",        b.Type      );
    printf("Width:%d ",       b.Width     );
    printf("Height:%d ",      b.Height    );
    printf("WidthBytes:%d ",  b.WidthBytes);
    printf("Planes:%d ",      b.Planes    );
    printf("BitsPixel:%d ",   b.BitsPixel );
    printf("BitsBytes:%d ",   (((b.Width * b.BitsPixel + 15) >> 4) << 1) * b.Height );
}

/**
    \brief Print U_BITMAPCOREHEADER object
    \param ch U_BITMAPCOREHEADER object
*/
void bitmapcoreheader_print(
      U_BITMAPCOREHEADER ch
    ){
    uint32_t Size;
    memcpy(&Size, &(ch.Size_4), 4);  /* will be aligned, but is in two pieces */
    printf("Size:%d ",     Size);
    printf("Width:%d ",    ch.Width);
    printf("Height:%d ",   ch.Height);
    printf("Planes:%d ",   ch.Planes);
    printf("BitCount:%d ", ch.BitCount);
}

/**   LogBrushW Object                      WMF PDF 2.2.2.10
     \brief Print a U_LOGBRUSHW object.
     \param lb U_LOGBRUSHW object.

  style                Color                Hatch
  U_BS_SOLID           ColorRef Object      Not used (bytes present???)
  U_BS_NULL            ignored              ignored  (bytes present???).
  U_BS_PATTERN         ignored              not used     (Action is not strictly defined)
  U_BS_DIBPATTERN      ignored              not used     (Action is not strictly defined)
  U_BS_DIBPATTERNPT    ignored              not used     (Action is not strictly defined)
  U_BS_HATCHED         ColorRef Object      HatchStyle Enumeration
*/
void wlogbrush_print(
      const char *lb  
   ){
    U_COLORREF Color;
    uint16_t Style = *(uint16_t *)(lb + offsetof(U_WLOGBRUSH,Style));
    uint16_t Hatch = *(uint16_t *)(lb + offsetof(U_WLOGBRUSH,Hatch));
    memcpy(&Color, lb + offsetof(U_WLOGBRUSH,Color), sizeof(U_COLORREF));
    printf("Style:0x%4.4X ",Style);
    switch(Style){
       case U_BS_SOLID:
          printf("Color:");         colorref_print(Color);
          break;
       case U_BS_NULL:
          printf("Null");
          break;
       case U_BS_PATTERN:
          printf("Pattern:(not implemented)");
          break;
       case U_BS_DIBPATTERN:
          printf("DIBPattern:(not implemented)");
          break;
       case U_BS_DIBPATTERNPT:
          printf("DIBPatternPt:(not implemented)");
          break;
       case U_BS_HATCHED:
          printf("Color:");         colorref_print(Color);
          printf("Hatch:0x%4.4X ",  Hatch);
          break;
    }
}


/**
    \brief Print U_POLYPOLY object from pointer
    \param  nPolys       Number of elements in aPolyCounts
    \param  aPolyCounts  Number of points in each poly (sequential)
    \param  Points       pointer to array of U_POINT16 in memory.  Probably not aligned.
*/
void polypolygon_print(
     uint16_t        nPolys,
     const uint16_t *aPolyCounts,
     const char     *Points
   ){
   int i,j;
   U_POINT16 pt;
   for(i=0; i<nPolys; i++, aPolyCounts++){
      printf("  Polygon[%d]: ",i);
      for(j=0; j < *aPolyCounts; j++, Points += sizeof(U_POINT16)){
         memcpy(&pt, Points, sizeof(U_POINT16)); /* may not be aligned */
         point16_print(pt);
      }
   }
}

/**
    \brief Print U_SCAN object
    \param sc U_SCAN object
*/
void scan_print(
      U_SCAN sc
    ){
    printf("Count:%d ",     sc.count);
    printf("Top:%d ",       sc.top);
    printf("Bottom:%d ",    sc.bottom);
    printf("data:(not shown)");
}

/**
    \brief Print a summary of a DIB header
    \param dh void pointer to DIB header
    A DIB header in an WMF may be either a BitmapCoreHeader or BitmapInfoHeader.
*/
void dibheader_print(const char *dh, const char *blimit){
   uint32_t Size;
   memcpy(&Size, dh, 4); /* may not be aligned */
   if(Size == 0xC ){
       printf("(BitmapCoreHeader) ");
       U_BITMAPCOREHEADER bmch;
       memcpy(&bmch, dh, sizeof(U_BITMAPCOREHEADER)); /* may not be aligned */
       bitmapcoreheader_print(bmch);
   }
   else {
       printf(" (BitmapInfoHeader) ");
       bitmapinfo_print(dh, blimit); /* may not be aligned, called routine must handle it */
   }
}

/**
    \brief Print WMF header object
    \returns size of entire header structure
    \param contents pointer to the first byte in the buffer holding the entire WMF file in memory
    \param blimit   pointer to the byte after the last byte in contents
    
    If the header is preceded by a placeable struture, print that as well.
*/
int wmfheader_print(
       const char *contents,
       const char *blimit
    ){
    U_WMRPLACEABLE  Placeable;
    U_WMRHEADER     Header;
    int size = wmfheader_get(contents, blimit, &Placeable, &Header);
    uint32_t utmp4;
    U_RECT16 rect16;
    uint32_t Key;
    memcpy(&Key, contents + offsetof(U_WMRPLACEABLE,Key), 4);
    if(Placeable.Key == 0x9AC6CDD7){
       printf("WMF, Placeable: ");
       printf("HWmf:%u ",      Placeable.HWmf);
       memcpy(&rect16,       &(Placeable.Dst),  sizeof(U_RECT16));
       printf("Box:"); rect16_ltrb_print(rect16);
       printf("Inch:%u ",      Placeable.Inch);
       printf("Checksum:%d ",  Placeable.Checksum);
       printf("Calculated_Checksum:%d\n",U_16_checksum((int16_t *)contents,10));
    }
    else {
       printf("WMF, Not Placeable\n");
    }
    printf("   RecType:%d\n",                     Header.iType);
    printf("   16bit words in record:%d\n",       Header.Size16w); 
    printf("   Version:%d\n",                     Header.version);
    memcpy(&utmp4,                              &(Header.Sizew),4);
    printf("   16bit words in file:%d\n",utmp4);
    printf("   Objects:%d\n",                     Header.nObjects);
    memcpy(&utmp4,                              &(Header.maxSize),4);
    printf("   Largest Record:%d\n", utmp4);
    printf("   nMembers:%d\n",                    Header.nMembers);

    return(size);
}



/* **********************************************************************************************
These functions contain shared code used by various U_WMR*_print functions.  These should NEVER be called
by end user code and to further that end prototypes are NOT provided and they are hidden from Doxygen.   
*********************************************************************************************** */


void wcore_points_print(uint16_t nPoints, const char *aPoints){
   int i;
   U_POINT16 pt;
   printf("  Points: ");
   for(i=0;i<nPoints; i++){ 
      memcpy(&pt, aPoints + i*4, sizeof(U_POINT16));  /* aPoints U_POINT16 structure may not be aligned, so copy it out */
      printf("[%d]:",i); point16_print(pt);
   }
   printf("\n");
}



/* **********************************************************************************************
These are the core WMR functions, each creates a particular type of record.  
All return these records via a char* pointer, which is NULL if the call failed.  
They are listed in order by the corresponding U_WMR_* index number.  
*********************************************************************************************** */

/**
    \brief Print a pointer to a U_WMR_whatever record which has not been implemented.
    \param contents   pointer to a buffer holding a WMR record
*/
void U_WMRNOTIMPLEMENTED_print(const char *contents){
   UNUSED(contents);
   printf("   Not Implemented!\n");
}

void U_WMREOF_print(const char *contents){
   UNUSED(contents);
}

void U_WMRSETBKCOLOR_print(const char *contents){
   U_COLORREF  Color;
   int size = U_WMRSETBKCOLOR_get(contents, &Color);
   if(size>0){
      printf("   %-15s ","Color:");  colorref_print(Color);   printf("\n");
   }
}

void U_WMRSETBKMODE_print(const char *contents){
   uint16_t iMode;
   int size = U_WMRSETBKMODE_get(contents, &iMode);
   if(size>0){
      printf("   %-15s 0x%4.4X\n","iMode:", iMode);
   }
}

void U_WMRSETMAPMODE_print(const char *contents){
   uint16_t iMode;
   int size = U_WMRSETMAPMODE_get(contents, &iMode);
   if(size>0){
      printf("   %-15s 0x%4.4X\n","iMode:", iMode);
   }
}

void U_WMRSETROP2_print(const char *contents){
   uint16_t iMode;
   int size = U_WMRSETROP2_get(contents, &iMode);
   if(size>0){
      printf("   %-15s 0x%4.4X\n","iMode:", iMode);
   }
}

void U_WMRSETRELABS_print(const char *contents){
   UNUSED(contents);
   /* This record type has only the common 6 bytes, so nothing (else) to print */
}

void U_WMRSETPOLYFILLMODE_print(const char *contents){
   uint16_t iMode;
   int size = U_WMRSETPOLYFILLMODE_get(contents, &iMode);
   if(size>0){
      printf("   %-15s 0x%4.4X\n","iMode:", iMode);
   }
}

void U_WMRSETSTRETCHBLTMODE_print(const char *contents){
   uint16_t iMode;
   int size = U_WMRSETSTRETCHBLTMODE_get(contents, &iMode);
   if(size>0){
      printf("   %-15s 0x%4.4X\n","iMode:", iMode);
   }
}

void U_WMRSETTEXTCHAREXTRA_print(const char *contents){
   uint16_t iMode;
   int size = U_WMRSETTEXTCHAREXTRA_get(contents, &iMode);
   if(size>0){
      printf("   %-15s 0x%4.4X\n","iMode:", iMode);
   }
}

void U_WMRSETTEXTCOLOR_print(const char *contents){
   U_COLORREF  Color;
   int size = U_WMRSETTEXTCOLOR_get(contents, &Color);
   if(size>0){
      printf("   %-15s ","Color:");  colorref_print(Color);   printf("\n");
   }
}

void U_WMRSETTEXTJUSTIFICATION_print(const char *contents){
   uint16_t Count;
   uint16_t Extra;
   int      size = U_WMRSETTEXTJUSTIFICATION_get(contents, &Count, &Extra);
   if(size){
      printf("   %-15s %d\n","Count", Count);
      printf("   %-15s %d\n","Extra", Extra);
   }
}

void U_WMRSETWINDOWORG_print(const char *contents){
   U_POINT16 coord;
   int       size = U_WMRSETWINDOWORG_get(contents, &coord);
   if(size){
      printf("   %-15s {%d,%d}\n","X,Y",coord.x, coord.y);
   }
}

void U_WMRSETWINDOWEXT_print(const char *contents){
   U_POINT16 coord;
   int       size = U_WMRSETWINDOWEXT_get(contents, &coord);
   if(size){
      printf("   %-15s {%d,%d}\n","W,H",coord.x, coord.y);
   }
}

void U_WMRSETVIEWPORTORG_print(const char *contents){
   U_POINT16 coord;
   int       size = U_WMRSETVIEWPORTORG_get(contents, &coord);
   if(size){
      printf("   %-15s {%d,%d}\n","X,Y",coord.x, coord.y);
   }
}

void U_WMRSETVIEWPORTEXT_print(const char *contents){
   U_POINT16 coord;
   int       size = U_WMRSETVIEWPORTEXT_get(contents, &coord);
   if(size){
      printf("   %-15s {%d,%d}\n","W,H",coord.x, coord.y);
   }
}

void U_WMROFFSETWINDOWORG_print(const char *contents){
   U_POINT16 coord;
   int       size = U_WMROFFSETWINDOWORG_get(contents, &coord);
   if(size){
      printf("   %-15s {%d,%d}\n","X,Y",coord.x, coord.y);
   }
}

void U_WMRSCALEWINDOWEXT_print(const char *contents){
   U_POINT16 Denom, Num;
   int size = U_WMRSCALEWINDOWEXT_get(contents, &Denom, &Num);
   if(size > 0){
      printf("   yDenom:%d\n",  Denom.y);
      printf("   yNum:%d\n",    Num.y  );
      printf("   xDenom:%d\n",  Denom.x);
      printf("   xNum:%d\n",    Num.x  );
   }
}

void U_WMROFFSETVIEWPORTORG_print(const char *contents){
   U_POINT16 coord;
   int       size = U_WMROFFSETVIEWPORTORG_get(contents, &coord);
   if(size){
      printf("   %-15s {%d,%d}\n","X,Y",coord.x, coord.y);
   }
}

void U_WMRSCALEVIEWPORTEXT_print(const char *contents){
   U_POINT16 Denom, Num;
   int       size = U_WMRSCALEVIEWPORTEXT_get(contents, &Denom, &Num);
   if(size > 0){
      printf("   yDenom:%d\n",  Denom.y);
      printf("   yNum:%d\n",    Num.y  );
      printf("   xDenom:%d\n",  Denom.x);
      printf("   xNum:%d\n",    Num.x  );
   }
}

void U_WMRLINETO_print(const char *contents){
   U_POINT16 coord;
   int       size = U_WMRLINETO_get(contents, &coord);
   if(size){
      printf("   %-15s {%d,%d}\n","X,Y",coord.x, coord.y);
   }
}

void U_WMRMOVETO_print(const char *contents){
   U_POINT16 coord;
   int       size = U_WMRMOVETO_get(contents, &coord);
   if(size > 0){
      printf("   %-15s {%d,%d}\n","X,Y",coord.x, coord.y);
   }
}

void U_WMREXCLUDECLIPRECT_print(const char *contents){
   U_RECT16 rect16;
   int      size = U_WMREXCLUDECLIPRECT_get(contents, &rect16);
   if(size > 0){
      printf("   Rect:"); 
      rect16_ltrb_print(rect16);
      printf("\n");
   }
}

void U_WMRINTERSECTCLIPRECT_print(const char *contents){
   U_RECT16 rect16;
   int      size = U_WMRINTERSECTCLIPRECT_get(contents, &rect16);
   if(size > 0){
      printf("   Rect:"); 
      rect16_ltrb_print(rect16);
      printf("\n");
   }
}

void U_WMRARC_print(const char *contents){
   U_POINT16 StartArc, EndArc;
   U_RECT16  rect16;
   int       size = U_WMRARC_get(contents, &StartArc, &EndArc, &rect16);
   if(size > 0){
       printf("   yRadial2:%d\n",  EndArc.y);
       printf("   xRadial2:%d\n",  EndArc.x);
       printf("   yRadial1:%d\n",  StartArc.y);
       printf("   xRadial1:%d\n",  StartArc.x);
       printf("   Rect:");    rect16_ltrb_print(rect16);    printf("\n");
   }
}

void U_WMRELLIPSE_print(const char *contents){
   U_RECT16 rect16;
   int      size = U_WMRELLIPSE_get(contents, &rect16);
   if(size > 0){
      printf("   Rect:");
      rect16_ltrb_print(rect16);
      printf("\n");
   }
}

void U_WMRFLOODFILL_print(const char *contents){
   uint16_t   Mode;
   U_COLORREF Color;
   U_POINT16  coord;
   int        size = U_WMRFLOODFILL_get(contents, &Mode, &Color, &coord);
   if(size > 0){
      printf("   Mode 0x%4.4X\n",  Mode);
      printf("   Color:");         colorref_print(Color);   printf("\n");
      printf("   X,Y {%d,%d}\n",   coord.x, coord.y);
   }
}

void U_WMRPIE_print(const char *contents){
   U_POINT16 StartArc, EndArc;
   U_RECT16  rect16;
   int       size = U_WMRPIE_get(contents, &StartArc, &EndArc, &rect16);
   if(size > 0){
       printf("   yRadial2:%d\n",  EndArc.y);
       printf("   xRadial2:%d\n",  EndArc.x);
       printf("   yRadial1:%d\n",  StartArc.y);
       printf("   xRadial1:%d\n",  StartArc.x);
       printf("   Rect:");    rect16_ltrb_print(rect16);    printf("\n");
   }
}

void U_WMRRECTANGLE_print(const char *contents){
   U_RECT16 rect16;
   int      size = U_WMRRECTANGLE_get(contents, &rect16);
   if(size > 0){
      printf("   Rect:");
      rect16_ltrb_print(rect16);
      printf("\n");
   }
}

void U_WMRROUNDRECT_print(const char *contents){
   int16_t  Height, Width;
   U_RECT16 rect16;
   int      size = U_WMRROUNDRECT_get(contents, &Width, &Height, &rect16);
   if(size > 0){
      printf("   Width:%d\n",  Width);
      printf("   Height:%d\n", Height);
      printf("   Rect:");
      rect16_ltrb_print(rect16);
      printf("\n");
   }
}

void U_WMRPATBLT_print(const char *contents){
   uint32_t   dwRop3;
   U_POINT16  Dst;
   U_POINT16  cwh;
   int        size = U_WMRPATBLT_get(contents, &Dst, &cwh, &dwRop3);
   if(size > 0){
      printf("    Rop3:%8.8X\n",      dwRop3 );
      printf("    W,H:%d,%d\n",       cwh.x, cwh.y );
      printf("    Dst X,Y:{%d,%d}\n", Dst.x,  Dst.y );
   }
}

void U_WMRSAVEDC_print(const char *contents){
   UNUSED(contents);
   /* This record type has only the common 6 bytes, so nothing (else) to print */
}

void U_WMRSETPIXEL_print(const char *contents){
   U_COLORREF Color;
   U_POINT16  coord;
   int        size = U_WMRSETPIXEL_get(contents, &Color, &coord);
   if(size > 0){
      printf("   Color:");         colorref_print(Color);   printf("\n");
      printf("   X,Y {%d,%d}\n",   coord.x, coord.y);
   }
}

void U_WMROFFSETCLIPRGN_print(const char *contents){
   U_POINT16 coord;
   int    size = U_WMROFFSETCLIPRGN_get(contents, &coord);
   if(size > 0){
      printf("   %-15s {%d,%d}\n","X,Y",coord.x, coord.y);
   }
}

void U_WMRTEXTOUT_print(const char *contents){
   int16_t         Length;
   const char     *string;
   U_POINT16       Dst;
   int       size = U_WMRTEXTOUT_get(contents, &Dst, &Length, &string);
   if(size > 0){
      printf("   X,Y:{%d,%d}\n", Dst.x,Dst.y);
      printf("   Length:%d\n", Length);
      printf("   String:<%.*s>\n", Length, string); /* May not be null terminated */
   }
}

void U_WMRBITBLT_print(const char *contents){
   uint32_t         dwRop3;
   U_POINT16        Dst, Src, cwh;
   U_BITMAP16       Bm16;
   const char      *px;
   int              size = U_WMRBITBLT_get(contents, &Dst, &cwh, &Src, &dwRop3, &Bm16, &px);
   if(size > 0){
      printf("    Rop3:%8.8X\n",      dwRop3 );
      printf("    Src X,Y:{%d,%d}\n", Src.x,  Src.y);
      printf("    W,H:%d,%d\n",       cwh.x,  cwh.y);
      printf("    Dst X,Y:{%d,%d}\n", Dst.x,  Dst.y);
      if(px){ printf("    Bitmap16:");   bitmap16_print(Bm16);  printf("\n"); }
      else {  printf("    Bitmap16: none\n");                                 }
   }
}

void U_WMRSTRETCHBLT_print(const char *contents){
   uint32_t         dwRop3;
   U_POINT16        Dst, Src, cDst, cSrc;  
   U_BITMAP16       Bm16;
   const char      *px;
   int              size = U_WMRSTRETCHBLT_get(contents, &Dst, &cDst, &Src, &cSrc, &dwRop3, &Bm16, &px);
   if(size > 0){
      printf("    Rop3:%8.8X\n",      dwRop3 );
      printf("    Src W,H:%d,%d\n",   cSrc.x,  cSrc.y);
      printf("    Src X,Y:{%d,%d}\n", Src.x,   Src.y );
      printf("    Dst W,H:%d,%d\n",   cDst.x,  cDst.y);
      printf("    Dst X,Y:{%d,%d}\n", Dst.x,   Dst.y );
      if(px){ printf("    Bitmap16:");   bitmap16_print(Bm16);  printf("\n"); }
      else {  printf("    Bitmap16: none\n");                                 }
   }
}

void U_WMRPOLYGON_print(const char *contents){
   uint16_t        Length;
   const char     *Data;
   int             size = U_WMRPOLYGON_get(contents, &Length, &Data);
   if(size > 0){
      wcore_points_print(Length, Data);
   }
}

void U_WMRPOLYLINE_print(const char *contents){
   uint16_t        Length;
   const char     *Data;
   int             size = U_WMRPOLYLINE_get(contents, &Length, &Data);
   if(size > 0){
      wcore_points_print(Length, Data);
   }
}

void U_WMRESCAPE_print(const char *contents){
   uint32_t        utmp4;
   uint16_t        Escape;
   uint16_t        Length;
   const char     *Data;
   int             size = U_WMRESCAPE_get(contents, &Escape, &Length, &Data);
   if(size > 0){
      printf("   EscType:%s\n",U_wmr_escnames(Escape));
      printf("   nBytes:%d\n",Length);
      if((Escape == U_MFE_SETLINECAP) || (Escape == U_MFE_SETLINEJOIN) || (Escape == U_MFE_SETMITERLIMIT)){
         memcpy(&utmp4, Data ,4);
         printf("   Data:%d\n", utmp4);
      }
      else {
         printf("   Data: (not shown)\n");
      }
   }
}

void U_WMRRESTOREDC_print(const char *contents){
   UNUSED(contents);
   /* This record type has only the common 6 bytes, so nothing (else) to print */
}

void U_WMRFILLREGION_print(const char *contents){
   uint16_t Region;
   uint16_t Brush;
   int    size = U_WMRFILLREGION_get(contents, &Region, &Brush);
   if(size > 0){
      printf("   %-15s %d\n","Region", Region);
      printf("   %-15s %d\n","Brush", Brush);
   }
}

void U_WMRFRAMEREGION_print(const char *contents){
   uint16_t Region;
   uint16_t Brush;
   int16_t  Height;
   int16_t  Width;
   int      size = U_WMRFRAMEREGION_get(contents, &Region, &Brush, &Height, &Width);
   if(size > 0){
      printf("   Region:%d\n",Region);
      printf("   Brush:%d\n", Brush );
      printf("   Height:%d\n",Height);
      printf("   Width:%d\n", Width );
   }
}

void U_WMRINVERTREGION_print(const char *contents){
   uint16_t Region;
   int      size = U_WMRSETTEXTALIGN_get(contents, &Region);
   if(size > 0){
      printf("   %-15s %d\n","Region:", Region);
   }
}

void U_WMRPAINTREGION_print(const char *contents){
   uint16_t Region;
   int      size = U_WMRPAINTREGION_get(contents, &Region);
   if(size>0){
      printf("   %-15s %d\n","Region:", Region);
   }
}

void U_WMRSELECTCLIPREGION_print(const char *contents){
   uint16_t Region;
   int      size = U_WMRSELECTCLIPREGION_get(contents, &Region);
   if(size>0){
      printf("   %-15s %d\n","Region:", Region);
   }
}

void U_WMRSELECTOBJECT_print(const char *contents){
   uint16_t Object;
   int      size = U_WMRSELECTOBJECT_get(contents, &Object);
   if(size>0){
      printf("   %-15s %d\n","Object:", Object);
   }
}

void U_WMRSETTEXTALIGN_print(const char *contents){
   uint16_t iMode;
   int      size = U_WMRSETTEXTALIGN_get(contents, &iMode);
   if(size>0){
      printf("   %-15s 0x%4.4X\n","iMode:", iMode);
   }
}

#define U_WMRDRAWTEXT_print     U_WMRNOTIMPLEMENTED_print

void U_WMRCHORD_print(const char *contents){
   U_POINT16 StartArc, EndArc;
   U_RECT16  rect16;
   int       size = U_WMRCHORD_get(contents, &StartArc, &EndArc, &rect16);
   if(size > 0){
       printf("   yRadial2:%d\n",  EndArc.y);
       printf("   xRadial2:%d\n",  EndArc.x);
       printf("   yRadial1:%d\n",  StartArc.y);
       printf("   xRadial1:%d\n",  StartArc.x);
       printf("   Rect:");    rect16_ltrb_print(rect16);    printf("\n");
   }
}

void U_WMRSETMAPPERFLAGS_print(const char *contents){
   uint32_t Flags4;
   int      size = U_WMRSETMAPPERFLAGS_get(contents, &Flags4);
   if(size > 0){
      printf("   %-15s 0x%8.8X\n","Flags4:", Flags4);
   }
}

void U_WMREXTTEXTOUT_print(const char *contents){
   U_RECT16          rect16;
   U_POINT16         Dst;
   int16_t           Length;
   uint16_t          Opts;
   const int16_t    *dx;
   const char       *string;
   int               i;
   int               size  = U_WMREXTTEXTOUT_get(contents, &Dst, &Length, &Opts, &string, &dx, &rect16);
   if(size > 0){
      printf("   X,Y:{%d,%d}\n",      Dst.x, Dst.y);
      printf("   Length:%d\n",        Length      );
      printf("   Opts:%4.4X\n",       Opts        );
      if(Opts & (U_ETO_OPAQUE | U_ETO_CLIPPED)){
         printf("   Rect:");   rect16_ltrb_print(rect16); printf("\n");
      }
      printf("   String:<%.*s>\n",Length, string);
      printf("   Dx:");
      for(i=0; i<Length; i++,dx++){  printf("%d:", *dx  ); }
      printf("\n");
   }
}

void U_WMRSETDIBTODEV_print(const char *contents){
   uint16_t           cUsage;
   uint16_t           ScanCount;
   uint16_t           StartScan;
   U_POINT16          Dst;
   U_POINT16          cwh;
   U_POINT16          Src;
   const char        *dib;
   int                size = U_WMRSETDIBTODEV_get(contents, &Dst, &cwh, &Src, &cUsage, &ScanCount, &StartScan, &dib);
   if(size > 0){
      printf("    cUsage:%d\n",       cUsage        );
      printf("    ScanCount:%d\n",    ScanCount     );
      printf("    StartScan:%d\n",    StartScan     );
      printf("    Src X,Y:{%d,%d}\n", Src.x,  Src.y );
      printf("    W,H:%d,%d\n",       cwh.x,  cwh.y );
      printf("    Dst X,Y:{%d,%d}\n", Dst.x,  Dst.y );
      printf("    DIB:");   dibheader_print(dib, dib+size);   printf("\n");
   }
}

void U_WMRSELECTPALETTE_print(const char *contents){
   uint16_t Palette;
   int      size = U_WMRSELECTPALETTE_get(contents, &Palette);
   if(size > 0){
      printf("   %-15s %d\n","Palette:", Palette);
   }
}

void U_WMRREALIZEPALETTE_print(const char *contents){
   UNUSED(contents);
   /* This record type has only the common 6 bytes, so nothing (else) to print */
}

void U_WMRANIMATEPALETTE_print(const char *contents){
   U_PALETTE       Palette;
   const char     *PalEntries;
   int             size = U_WMRANIMATEPALETTE_get(contents, &Palette, &PalEntries);
   if(size > 0){
      printf("   Palette:");  palette_print(&Palette, PalEntries);  printf("\n");
       
   }
}

void U_WMRSETPALENTRIES_print(const char *contents){
   U_PALETTE       Palette;
   const char     *PalEntries;
   int             size = U_WMRSETPALENTRIES_get(contents, &Palette, &PalEntries);
   if(size > 0){
      printf("   Palette:");  palette_print(&Palette, PalEntries);  printf("\n");
   }
}

void U_WMRPOLYPOLYGON_print(const char *contents){
   uint16_t        nPolys;
   const uint16_t *aPolyCounts;
   const char     *Points;
   int             size = U_WMRPOLYPOLYGON_get(contents, &nPolys, &aPolyCounts, &Points);
   if(size > 0){
      printf("   Polygons:"); polypolygon_print(nPolys, aPolyCounts, Points); printf("\n");
   }
}

void U_WMRRESIZEPALETTE_print(const char *contents){
   uint16_t Palette;
   int size = U_WMRSELECTCLIPREGION_get(contents, &Palette);
   if(size>0){
      printf("   %-15s %d\n","Palette:", Palette);
   }
}

#define U_WMR3A_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR3B_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR3C_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR3D_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR3E_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR3F_print    U_WMRNOTIMPLEMENTED_print

void U_WMRDIBBITBLT_print(const char *contents){
   U_POINT16        Dst, cwh, Src;
   uint32_t         dwRop3;
   const char      *dib;
   int              size = U_WMRDIBBITBLT_get(contents, &Dst, &cwh, &Src, &dwRop3, &dib);
   if(size > 0){
      printf("    Rop3:%8.8X\n",      dwRop3 );
      printf("    Src X,Y:{%d,%d}\n", Src.x, Src.x );
      printf("    W,H:%d,%d\n",       cwh.x, cwh.y );
      printf("    Dst X,Y:{%d,%d}\n", Dst.x, Dst.y );
      if(dib){  printf("    DIB:");   dibheader_print(dib, dib+size);  printf("\n"); }
      else {    printf("    DIB: none\n");                                 }
   }
}

void U_WMRDIBSTRETCHBLT_print(const char *contents){
   U_POINT16        Dst, cDst, Src, cSrc;
   uint32_t         dwRop3;
   const char      *dib;
   int              size = U_WMRDIBSTRETCHBLT_get(contents, &Dst, &cDst, &Src, &cSrc, &dwRop3, &dib);
   if(size > 0){
      printf("    Rop3:%8.8X\n",      dwRop3 );
      printf("    Src W,H:%d,%d\n",   cSrc.x, cSrc.y );
      printf("    Src X,Y:{%d,%d}\n", Src.x,  Src.x );
      printf("    Dst W,H:%d,%d\n",   cDst.x, cDst.y );
      printf("    Dst X,Y:{%d,%d}\n", Dst.x,  Dst.y );
      if(dib){  printf("    DIB:");   dibheader_print(dib, dib+size);  printf("\n"); }
      else {    printf("    DIB: none\n");                                 }
   }
}

void U_WMRDIBCREATEPATTERNBRUSH_print(const char *contents){
   uint16_t        Style, cUsage;
   const char     *TBm16;
   const char     *dib;
   int             size = U_WMRDIBCREATEPATTERNBRUSH_get(contents, &Style, &cUsage, &TBm16, &dib);
   if(size > 0){
      U_BITMAP16 Bm16;
      printf("   Style:%d\n",   Style );
      printf("   cUsage:%d\n",  cUsage);
      if(TBm16){
         memcpy(&Bm16, TBm16, U_SIZE_BITMAP16);
         printf("   Src:Bitmap16:");  bitmap16_print(Bm16); printf("\n");
      }
      else { /* from DIB */
         printf("   Src:DIB:");       dibheader_print(dib, dib+size); printf("\n");
      }
   }
}

void U_WMRSTRETCHDIB_print(const char *contents){
   U_POINT16        Dst, cDst, Src, cSrc;
   uint32_t         dwRop3;
   uint16_t         cUsage;
   const char      *dib;
   int              size = U_WMRSTRETCHDIB_get(contents, &Dst, &cDst, &Src, &cSrc, &cUsage, &dwRop3, &dib);
   if(size > 0){
      printf("    Rop3:%8.8X\n",      dwRop3 );
      printf("    cUsage:%d\n",       cUsage );
      printf("    Src W,H:%d,%d\n",   cSrc.x, cSrc.y );
      printf("    Src X,Y:{%d,%d}\n", Src.x,  Src.x );
      printf("    Dst W,H:%d,%d\n",   cDst.x, cDst.y );
      printf("    Dst X,Y:{%d,%d}\n", Dst.x,  Dst.y );
      if(dib){  printf("    DIB:");   dibheader_print(dib, dib+size);  printf("\n"); }
      else {    printf("    DIB: none\n");                                 }
   }
}

#define U_WMR44_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR45_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR46_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR47_print    U_WMRNOTIMPLEMENTED_print

void U_WMREXTFLOODFILL_print(const char *contents){
   uint16_t   Mode;
   U_COLORREF Color;
   U_POINT16  coord;
   int        size = U_WMREXTFLOODFILL_get(contents, &Mode, &Color, &coord);
   if(size > 0){
      printf("   Mode 0x%4.4X\n",  Mode);
      printf("   Color:");         colorref_print(Color);   printf("\n");
      printf("   X,Y {%d,%d}\n",   coord.x, coord.y);
   }
}

#define U_WMR49_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR4A_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR4B_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR4C_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR4D_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR4E_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR4F_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR50_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR51_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR52_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR53_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR54_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR55_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR56_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR57_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR58_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR59_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR5A_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR5B_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR5C_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR5D_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR5E_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR5F_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR60_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR61_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR62_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR63_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR64_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR65_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR66_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR67_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR68_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR69_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR6A_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR6B_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR6C_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR6D_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR6E_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR6F_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR70_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR71_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR72_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR73_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR74_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR75_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR76_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR77_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR78_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR79_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR7A_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR7B_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR7C_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR7D_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR7E_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR7F_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR80_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR81_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR82_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR83_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR84_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR85_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR86_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR87_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR88_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR89_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR8A_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR8B_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR8C_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR8D_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR8E_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR8F_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR90_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR91_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR92_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR93_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR94_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR95_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR96_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR97_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR98_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR99_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR9A_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR9B_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR9C_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR9D_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR9E_print    U_WMRNOTIMPLEMENTED_print
#define U_WMR9F_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRA0_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRA1_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRA2_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRA3_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRA4_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRA5_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRA6_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRA7_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRA8_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRA9_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRAA_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRAB_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRAC_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRAD_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRAE_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRAF_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRB0_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRB1_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRB2_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRB3_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRB4_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRB5_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRB6_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRB7_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRB8_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRB9_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRBA_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRBB_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRBC_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRBD_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRBE_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRBF_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRC0_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRC1_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRC2_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRC3_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRC4_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRC5_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRC6_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRC7_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRC8_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRC9_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRCA_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRCB_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRCC_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRCD_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRCE_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRCF_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRD0_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRD1_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRD2_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRD3_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRD4_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRD5_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRD6_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRD7_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRD8_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRD9_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRDA_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRDB_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRDC_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRDD_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRDE_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRDF_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRE0_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRE1_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRE2_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRE3_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRE4_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRE5_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRE6_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRE7_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRE8_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRE9_print    U_WMRNOTIMPLEMENTED_print
#define U_WMREA_print    U_WMRNOTIMPLEMENTED_print
#define U_WMREB_print    U_WMRNOTIMPLEMENTED_print
#define U_WMREC_print    U_WMRNOTIMPLEMENTED_print
#define U_WMRED_print    U_WMRNOTIMPLEMENTED_print
#define U_WMREE_print    U_WMRNOTIMPLEMENTED_print
#define U_WMREF_print    U_WMRNOTIMPLEMENTED_print

void U_WMRDELETEOBJECT_print(const char *contents){
   uint16_t Object;
   int size = U_WMRDELETEOBJECT_get(contents, &Object);
   if(size>0){
      printf("   %-15s %d\n","Object:", Object);
   }
}

#define  U_WMRF1_print    U_WMRNOTIMPLEMENTED_print
#define  U_WMRF2_print    U_WMRNOTIMPLEMENTED_print
#define  U_WMRF3_print    U_WMRNOTIMPLEMENTED_print
#define  U_WMRF4_print    U_WMRNOTIMPLEMENTED_print
#define  U_WMRF5_print    U_WMRNOTIMPLEMENTED_print
#define  U_WMRF6_print    U_WMRNOTIMPLEMENTED_print

void U_WMRCREATEPALETTE_print(const char *contents){
   U_PALETTE       Palette;
   const char     *PalEntries;
   int             size = U_WMRCREATEPALETTE_get(contents, &Palette, &PalEntries);
   if(size > 0){
      printf("   Palette:");  palette_print(&Palette, PalEntries);  printf("\n");
       
   }
}

#define  U_WMRF8_print   U_WMRNOTIMPLEMENTED_print

void U_WMRCREATEPATTERNBRUSH_print(const char *contents){
   U_BITMAP16       Bm16;
   int              pasize;
   int              i;
   const char      *Pattern;

   int size = U_WMRCREATEPATTERNBRUSH_get(contents, &Bm16, &pasize, &Pattern);
   if(size > 0){
      /* BM16 is truncated, but bitmap16_print does not get into the part that was omitted */
      printf("   BitMap16: ");  bitmap16_print(Bm16); printf("\n");
      printf("   Pattern: ");
      for(i=0;i<pasize;i++){
         printf("%2.2X ",Pattern[i]);
      }
      printf("\n");
   }
}

void U_WMRCREATEPENINDIRECT_print(const char *contents){
   U_PEN pen;
   int   size = U_WMRCREATEPENINDIRECT_get(contents, &pen);
   if(size > 0){
      printf("   Pen:"); pen_print(pen); printf("\n");
   }
}

void U_WMRCREATEFONTINDIRECT_print(const char *contents){
   const char *font; /* Note, because of possible struct alignment issue have to use char * to reference the data */
   int         size = U_WMRCREATEFONTINDIRECT_get(contents, &font);
   if(size > 0){
      printf("   Font:"); 
      font_print(font);
      printf("\n");
   }
}

void U_WMRCREATEBRUSHINDIRECT_print(const char *contents){
   const char *brush; /* Note, because of possible struct alignment issue have to use char * to reference the data */
   int         size = U_WMRCREATEBRUSHINDIRECT_get(contents, &brush);
   if(size > 0){
      printf("   Brush:"); 
      wlogbrush_print(brush);
      printf("\n");
   }
}

void U_WMRCREATEBITMAPINDIRECT_print(const char *contents){ /* in Wine, not in WMF PDF */
   U_WMRNOTIMPLEMENTED_print(contents);
}

void U_WMRCREATEBITMAP_print(const char *contents){ /* in Wine, not in WMF PDF */
   U_WMRNOTIMPLEMENTED_print(contents);
}

void U_WMRCREATEREGION_print(const char *contents){
   const char *region; /* Note, because of possible struct alignment issue have to use char * to reference the data */
   int         size = U_WMRCREATEBRUSHINDIRECT_get(contents, &region);
   if(size > 0){
      printf("   Brush:"); 
      printf("   Region: ");  region_print(region); printf("\n");
   }
}

//! \endcond

/**
    \brief Print any record in a wmf
    \returns record length for a normal record, 0 for WMREOF, -1 for a bad record
    \param contents   pointer to a buffer holding all WMR records
    \param blimit     one byte past the last WMF record in memory.
    \param recnum     number of this record in contents
    \param off        offset to this record in contents
*/
int U_wmf_onerec_print(const char *contents, const char *blimit, int recnum, size_t off){
      

    uint8_t   iType;
    size_t    size;

    contents += off;

    /* Check that the record size is OK, abort if not.
       Pointer math might wrap, so check both sides of the range */
    size = U_WMRRECSAFE_get(contents, blimit);
    if(!size)return(-1);

    iType     = *(uint8_t *)(contents + offsetof(U_METARECORD, iType )  );

    uint32_t crc;   
#if U_BYTE_SWAP
    //This is a Big Endian machine, WMF crc values must be calculated on Little Endian form
    char *swapbuf=malloc(size);
    if(!swapbuf)return(-1);
    memcpy(swapbuf,contents,size);
    U_wmf_endian(swapbuf,size,1,1);  // BE to LE
    crc=lu_crc32(swapbuf,size);
    free(swapbuf);
#else 
    crc=lu_crc32(contents,size);
#endif
    printf("%-30srecord:%5d type:%-4u offset:%8d rsize:%8u crc32:%8.8X\n",
      U_wmr_names(iType), recnum, iType, (int) off, (int) size, crc);

    switch (iType)
    {
       case  U_WMR_EOF:                    U_WMREOF_print(contents);     size=0;          break;
       case  U_WMR_SETBKCOLOR:             U_WMRSETBKCOLOR_print(contents);               break;
       case  U_WMR_SETBKMODE:              U_WMRSETBKMODE_print(contents);                break;
       case  U_WMR_SETMAPMODE:             U_WMRSETMAPMODE_print(contents);               break;
       case  U_WMR_SETROP2:                U_WMRSETROP2_print(contents);                  break;
       case  U_WMR_SETRELABS:              U_WMRSETRELABS_print(contents);                break;
       case  U_WMR_SETPOLYFILLMODE:        U_WMRSETPOLYFILLMODE_print(contents);          break;
       case  U_WMR_SETSTRETCHBLTMODE:      U_WMRSETSTRETCHBLTMODE_print(contents);        break;
       case  U_WMR_SETTEXTCHAREXTRA:       U_WMRSETTEXTCHAREXTRA_print(contents);         break;
       case  U_WMR_SETTEXTCOLOR:           U_WMRSETTEXTCOLOR_print(contents);             break;
       case  U_WMR_SETTEXTJUSTIFICATION:   U_WMRSETTEXTJUSTIFICATION_print(contents);     break;
       case  U_WMR_SETWINDOWORG:           U_WMRSETWINDOWORG_print(contents);             break;
       case  U_WMR_SETWINDOWEXT:           U_WMRSETWINDOWEXT_print(contents);             break;
       case  U_WMR_SETVIEWPORTORG:         U_WMRSETVIEWPORTORG_print(contents);           break;
       case  U_WMR_SETVIEWPORTEXT:         U_WMRSETVIEWPORTEXT_print(contents);           break;
       case  U_WMR_OFFSETWINDOWORG:        U_WMROFFSETWINDOWORG_print(contents);          break;
       case  U_WMR_SCALEWINDOWEXT:         U_WMRSCALEWINDOWEXT_print(contents);           break;
       case  U_WMR_OFFSETVIEWPORTORG:      U_WMROFFSETVIEWPORTORG_print(contents);        break;
       case  U_WMR_SCALEVIEWPORTEXT:       U_WMRSCALEVIEWPORTEXT_print(contents);         break;
       case  U_WMR_LINETO:                 U_WMRLINETO_print(contents);                   break;
       case  U_WMR_MOVETO:                 U_WMRMOVETO_print(contents);                   break;
       case  U_WMR_EXCLUDECLIPRECT:        U_WMREXCLUDECLIPRECT_print(contents);          break;
       case  U_WMR_INTERSECTCLIPRECT:      U_WMRINTERSECTCLIPRECT_print(contents);        break;
       case  U_WMR_ARC:                    U_WMRARC_print(contents);                      break;
       case  U_WMR_ELLIPSE:                U_WMRELLIPSE_print(contents);                  break;
       case  U_WMR_FLOODFILL:              U_WMRFLOODFILL_print(contents);                break;
       case  U_WMR_PIE:                    U_WMRPIE_print(contents);                      break;
       case  U_WMR_RECTANGLE:              U_WMRRECTANGLE_print(contents);                break;
       case  U_WMR_ROUNDRECT:              U_WMRROUNDRECT_print(contents);                break;
       case  U_WMR_PATBLT:                 U_WMRPATBLT_print(contents);                   break;
       case  U_WMR_SAVEDC:                 U_WMRSAVEDC_print(contents);                   break;
       case  U_WMR_SETPIXEL:               U_WMRSETPIXEL_print(contents);                 break;
       case  U_WMR_OFFSETCLIPRGN:          U_WMROFFSETCLIPRGN_print(contents);            break;
       case  U_WMR_TEXTOUT:                U_WMRTEXTOUT_print(contents);                  break;
       case  U_WMR_BITBLT:                 U_WMRBITBLT_print(contents);                   break;
       case  U_WMR_STRETCHBLT:             U_WMRSTRETCHBLT_print(contents);               break;
       case  U_WMR_POLYGON:                U_WMRPOLYGON_print(contents);                  break;
       case  U_WMR_POLYLINE:               U_WMRPOLYLINE_print(contents);                 break;
       case  U_WMR_ESCAPE:                 U_WMRESCAPE_print(contents);                   break;
       case  U_WMR_RESTOREDC:              U_WMRRESTOREDC_print(contents);                break;
       case  U_WMR_FILLREGION:             U_WMRFILLREGION_print(contents);               break;
       case  U_WMR_FRAMEREGION:            U_WMRFRAMEREGION_print(contents);              break;
       case  U_WMR_INVERTREGION:           U_WMRINVERTREGION_print(contents);             break;
       case  U_WMR_PAINTREGION:            U_WMRPAINTREGION_print(contents);              break;
       case  U_WMR_SELECTCLIPREGION:       U_WMRSELECTCLIPREGION_print(contents);         break;
       case  U_WMR_SELECTOBJECT:           U_WMRSELECTOBJECT_print(contents);             break;
       case  U_WMR_SETTEXTALIGN:           U_WMRSETTEXTALIGN_print(contents);             break;
       case  U_WMR_DRAWTEXT:               U_WMRDRAWTEXT_print(contents);                 break;
       case  U_WMR_CHORD:                  U_WMRCHORD_print(contents);                    break;
       case  U_WMR_SETMAPPERFLAGS:         U_WMRSETMAPPERFLAGS_print(contents);           break;
       case  U_WMR_EXTTEXTOUT:             U_WMREXTTEXTOUT_print(contents);               break;
       case  U_WMR_SETDIBTODEV:            U_WMRSETDIBTODEV_print(contents);              break;
       case  U_WMR_SELECTPALETTE:          U_WMRSELECTPALETTE_print(contents);            break;
       case  U_WMR_REALIZEPALETTE:         U_WMRREALIZEPALETTE_print(contents);           break;
       case  U_WMR_ANIMATEPALETTE:         U_WMRANIMATEPALETTE_print(contents);           break;
       case  U_WMR_SETPALENTRIES:          U_WMRSETPALENTRIES_print(contents);            break;
       case  U_WMR_POLYPOLYGON:            U_WMRPOLYPOLYGON_print(contents);              break;
       case  U_WMR_RESIZEPALETTE:          U_WMRRESIZEPALETTE_print(contents);            break;
       case  U_WMR_3A:                     U_WMR3A_print(contents);                       break;
       case  U_WMR_3B:                     U_WMR3B_print(contents);                       break;
       case  U_WMR_3C:                     U_WMR3C_print(contents);                       break;
       case  U_WMR_3D:                     U_WMR3D_print(contents);                       break;
       case  U_WMR_3E:                     U_WMR3E_print(contents);                       break;
       case  U_WMR_3F:                     U_WMR3F_print(contents);                       break;
       case  U_WMR_DIBBITBLT:              U_WMRDIBBITBLT_print(contents);                break;
       case  U_WMR_DIBSTRETCHBLT:          U_WMRDIBSTRETCHBLT_print(contents);            break;
       case  U_WMR_DIBCREATEPATTERNBRUSH:  U_WMRDIBCREATEPATTERNBRUSH_print(contents);    break;
       case  U_WMR_STRETCHDIB:             U_WMRSTRETCHDIB_print(contents);               break;
       case  U_WMR_44:                     U_WMR44_print(contents);                       break;
       case  U_WMR_45:                     U_WMR45_print(contents);                       break;
       case  U_WMR_46:                     U_WMR46_print(contents);                       break;
       case  U_WMR_47:                     U_WMR47_print(contents);                       break;
       case  U_WMR_EXTFLOODFILL:           U_WMREXTFLOODFILL_print(contents);             break;
       case  U_WMR_49:                     U_WMR49_print(contents);                       break;
       case  U_WMR_4A:                     U_WMR4A_print(contents);                       break;
       case  U_WMR_4B:                     U_WMR4B_print(contents);                       break;
       case  U_WMR_4C:                     U_WMR4C_print(contents);                       break;
       case  U_WMR_4D:                     U_WMR4D_print(contents);                       break;
       case  U_WMR_4E:                     U_WMR4E_print(contents);                       break;
       case  U_WMR_4F:                     U_WMR4F_print(contents);                       break;
       case  U_WMR_50:                     U_WMR50_print(contents);                       break;
       case  U_WMR_51:                     U_WMR51_print(contents);                       break;
       case  U_WMR_52:                     U_WMR52_print(contents);                       break;
       case  U_WMR_53:                     U_WMR53_print(contents);                       break;
       case  U_WMR_54:                     U_WMR54_print(contents);                       break;
       case  U_WMR_55:                     U_WMR55_print(contents);                       break;
       case  U_WMR_56:                     U_WMR56_print(contents);                       break;
       case  U_WMR_57:                     U_WMR57_print(contents);                       break;
       case  U_WMR_58:                     U_WMR58_print(contents);                       break;
       case  U_WMR_59:                     U_WMR59_print(contents);                       break;
       case  U_WMR_5A:                     U_WMR5A_print(contents);                       break;
       case  U_WMR_5B:                     U_WMR5B_print(contents);                       break;
       case  U_WMR_5C:                     U_WMR5C_print(contents);                       break;
       case  U_WMR_5D:                     U_WMR5D_print(contents);                       break;
       case  U_WMR_5E:                     U_WMR5E_print(contents);                       break;
       case  U_WMR_5F:                     U_WMR5F_print(contents);                       break;
       case  U_WMR_60:                     U_WMR60_print(contents);                       break;
       case  U_WMR_61:                     U_WMR61_print(contents);                       break;
       case  U_WMR_62:                     U_WMR62_print(contents);                       break;
       case  U_WMR_63:                     U_WMR63_print(contents);                       break;
       case  U_WMR_64:                     U_WMR64_print(contents);                       break;
       case  U_WMR_65:                     U_WMR65_print(contents);                       break;
       case  U_WMR_66:                     U_WMR66_print(contents);                       break;
       case  U_WMR_67:                     U_WMR67_print(contents);                       break;
       case  U_WMR_68:                     U_WMR68_print(contents);                       break;
       case  U_WMR_69:                     U_WMR69_print(contents);                       break;
       case  U_WMR_6A:                     U_WMR6A_print(contents);                       break;
       case  U_WMR_6B:                     U_WMR6B_print(contents);                       break;
       case  U_WMR_6C:                     U_WMR6C_print(contents);                       break;
       case  U_WMR_6D:                     U_WMR6D_print(contents);                       break;
       case  U_WMR_6E:                     U_WMR6E_print(contents);                       break;
       case  U_WMR_6F:                     U_WMR6F_print(contents);                       break;
       case  U_WMR_70:                     U_WMR70_print(contents);                       break;
       case  U_WMR_71:                     U_WMR71_print(contents);                       break;
       case  U_WMR_72:                     U_WMR72_print(contents);                       break;
       case  U_WMR_73:                     U_WMR73_print(contents);                       break;
       case  U_WMR_74:                     U_WMR74_print(contents);                       break;
       case  U_WMR_75:                     U_WMR75_print(contents);                       break;
       case  U_WMR_76:                     U_WMR76_print(contents);                       break;
       case  U_WMR_77:                     U_WMR77_print(contents);                       break;
       case  U_WMR_78:                     U_WMR78_print(contents);                       break;
       case  U_WMR_79:                     U_WMR79_print(contents);                       break;
       case  U_WMR_7A:                     U_WMR7A_print(contents);                       break;
       case  U_WMR_7B:                     U_WMR7B_print(contents);                       break;
       case  U_WMR_7C:                     U_WMR7C_print(contents);                       break;
       case  U_WMR_7D:                     U_WMR7D_print(contents);                       break;
       case  U_WMR_7E:                     U_WMR7E_print(contents);                       break;
       case  U_WMR_7F:                     U_WMR7F_print(contents);                       break;
       case  U_WMR_80:                     U_WMR80_print(contents);                       break;
       case  U_WMR_81:                     U_WMR81_print(contents);                       break;
       case  U_WMR_82:                     U_WMR82_print(contents);                       break;
       case  U_WMR_83:                     U_WMR83_print(contents);                       break;
       case  U_WMR_84:                     U_WMR84_print(contents);                       break;
       case  U_WMR_85:                     U_WMR85_print(contents);                       break;
       case  U_WMR_86:                     U_WMR86_print(contents);                       break;
       case  U_WMR_87:                     U_WMR87_print(contents);                       break;
       case  U_WMR_88:                     U_WMR88_print(contents);                       break;
       case  U_WMR_89:                     U_WMR89_print(contents);                       break;
       case  U_WMR_8A:                     U_WMR8A_print(contents);                       break;
       case  U_WMR_8B:                     U_WMR8B_print(contents);                       break;
       case  U_WMR_8C:                     U_WMR8C_print(contents);                       break;
       case  U_WMR_8D:                     U_WMR8D_print(contents);                       break;
       case  U_WMR_8E:                     U_WMR8E_print(contents);                       break;
       case  U_WMR_8F:                     U_WMR8F_print(contents);                       break;
       case  U_WMR_90:                     U_WMR90_print(contents);                       break;
       case  U_WMR_91:                     U_WMR91_print(contents);                       break;
       case  U_WMR_92:                     U_WMR92_print(contents);                       break;
       case  U_WMR_93:                     U_WMR93_print(contents);                       break;
       case  U_WMR_94:                     U_WMR94_print(contents);                       break;
       case  U_WMR_95:                     U_WMR95_print(contents);                       break;
       case  U_WMR_96:                     U_WMR96_print(contents);                       break;
       case  U_WMR_97:                     U_WMR97_print(contents);                       break;
       case  U_WMR_98:                     U_WMR98_print(contents);                       break;
       case  U_WMR_99:                     U_WMR99_print(contents);                       break;
       case  U_WMR_9A:                     U_WMR9A_print(contents);                       break;
       case  U_WMR_9B:                     U_WMR9B_print(contents);                       break;
       case  U_WMR_9C:                     U_WMR9C_print(contents);                       break;
       case  U_WMR_9D:                     U_WMR9D_print(contents);                       break;
       case  U_WMR_9E:                     U_WMR9E_print(contents);                       break;
       case  U_WMR_9F:                     U_WMR9F_print(contents);                       break;
       case  U_WMR_A0:                     U_WMRA0_print(contents);                       break;
       case  U_WMR_A1:                     U_WMRA1_print(contents);                       break;
       case  U_WMR_A2:                     U_WMRA2_print(contents);                       break;
       case  U_WMR_A3:                     U_WMRA3_print(contents);                       break;
       case  U_WMR_A4:                     U_WMRA4_print(contents);                       break;
       case  U_WMR_A5:                     U_WMRA5_print(contents);                       break;
       case  U_WMR_A6:                     U_WMRA6_print(contents);                       break;
       case  U_WMR_A7:                     U_WMRA7_print(contents);                       break;
       case  U_WMR_A8:                     U_WMRA8_print(contents);                       break;
       case  U_WMR_A9:                     U_WMRA9_print(contents);                       break;
       case  U_WMR_AA:                     U_WMRAA_print(contents);                       break;
       case  U_WMR_AB:                     U_WMRAB_print(contents);                       break;
       case  U_WMR_AC:                     U_WMRAC_print(contents);                       break;
       case  U_WMR_AD:                     U_WMRAD_print(contents);                       break;
       case  U_WMR_AE:                     U_WMRAE_print(contents);                       break;
       case  U_WMR_AF:                     U_WMRAF_print(contents);                       break;
       case  U_WMR_B0:                     U_WMRB0_print(contents);                       break;
       case  U_WMR_B1:                     U_WMRB1_print(contents);                       break;
       case  U_WMR_B2:                     U_WMRB2_print(contents);                       break;
       case  U_WMR_B3:                     U_WMRB3_print(contents);                       break;
       case  U_WMR_B4:                     U_WMRB4_print(contents);                       break;
       case  U_WMR_B5:                     U_WMRB5_print(contents);                       break;
       case  U_WMR_B6:                     U_WMRB6_print(contents);                       break;
       case  U_WMR_B7:                     U_WMRB7_print(contents);                       break;
       case  U_WMR_B8:                     U_WMRB8_print(contents);                       break;
       case  U_WMR_B9:                     U_WMRB9_print(contents);                       break;
       case  U_WMR_BA:                     U_WMRBA_print(contents);                       break;
       case  U_WMR_BB:                     U_WMRBB_print(contents);                       break;
       case  U_WMR_BC:                     U_WMRBC_print(contents);                       break;
       case  U_WMR_BD:                     U_WMRBD_print(contents);                       break;
       case  U_WMR_BE:                     U_WMRBE_print(contents);                       break;
       case  U_WMR_BF:                     U_WMRBF_print(contents);                       break;
       case  U_WMR_C0:                     U_WMRC0_print(contents);                       break;
       case  U_WMR_C1:                     U_WMRC1_print(contents);                       break;
       case  U_WMR_C2:                     U_WMRC2_print(contents);                       break;
       case  U_WMR_C3:                     U_WMRC3_print(contents);                       break;
       case  U_WMR_C4:                     U_WMRC4_print(contents);                       break;
       case  U_WMR_C5:                     U_WMRC5_print(contents);                       break;
       case  U_WMR_C6:                     U_WMRC6_print(contents);                       break;
       case  U_WMR_C7:                     U_WMRC7_print(contents);                       break;
       case  U_WMR_C8:                     U_WMRC8_print(contents);                       break;
       case  U_WMR_C9:                     U_WMRC9_print(contents);                       break;
       case  U_WMR_CA:                     U_WMRCA_print(contents);                       break;
       case  U_WMR_CB:                     U_WMRCB_print(contents);                       break;
       case  U_WMR_CC:                     U_WMRCC_print(contents);                       break;
       case  U_WMR_CD:                     U_WMRCD_print(contents);                       break;
       case  U_WMR_CE:                     U_WMRCE_print(contents);                       break;
       case  U_WMR_CF:                     U_WMRCF_print(contents);                       break;
       case  U_WMR_D0:                     U_WMRD0_print(contents);                       break;
       case  U_WMR_D1:                     U_WMRD1_print(contents);                       break;
       case  U_WMR_D2:                     U_WMRD2_print(contents);                       break;
       case  U_WMR_D3:                     U_WMRD3_print(contents);                       break;
       case  U_WMR_D4:                     U_WMRD4_print(contents);                       break;
       case  U_WMR_D5:                     U_WMRD5_print(contents);                       break;
       case  U_WMR_D6:                     U_WMRD6_print(contents);                       break;
       case  U_WMR_D7:                     U_WMRD7_print(contents);                       break;
       case  U_WMR_D8:                     U_WMRD8_print(contents);                       break;
       case  U_WMR_D9:                     U_WMRD9_print(contents);                       break;
       case  U_WMR_DA:                     U_WMRDA_print(contents);                       break;
       case  U_WMR_DB:                     U_WMRDB_print(contents);                       break;
       case  U_WMR_DC:                     U_WMRDC_print(contents);                       break;
       case  U_WMR_DD:                     U_WMRDD_print(contents);                       break;
       case  U_WMR_DE:                     U_WMRDE_print(contents);                       break;
       case  U_WMR_DF:                     U_WMRDF_print(contents);                       break;
       case  U_WMR_E0:                     U_WMRE0_print(contents);                       break;
       case  U_WMR_E1:                     U_WMRE1_print(contents);                       break;
       case  U_WMR_E2:                     U_WMRE2_print(contents);                       break;
       case  U_WMR_E3:                     U_WMRE3_print(contents);                       break;
       case  U_WMR_E4:                     U_WMRE4_print(contents);                       break;
       case  U_WMR_E5:                     U_WMRE5_print(contents);                       break;
       case  U_WMR_E6:                     U_WMRE6_print(contents);                       break;
       case  U_WMR_E7:                     U_WMRE7_print(contents);                       break;
       case  U_WMR_E8:                     U_WMRE8_print(contents);                       break;
       case  U_WMR_E9:                     U_WMRE9_print(contents);                       break;
       case  U_WMR_EA:                     U_WMREA_print(contents);                       break;
       case  U_WMR_EB:                     U_WMREB_print(contents);                       break;
       case  U_WMR_EC:                     U_WMREC_print(contents);                       break;
       case  U_WMR_ED:                     U_WMRED_print(contents);                       break;
       case  U_WMR_EE:                     U_WMREE_print(contents);                       break;
       case  U_WMR_EF:                     U_WMREF_print(contents);                       break;
       case  U_WMR_DELETEOBJECT:           U_WMRDELETEOBJECT_print(contents);             break;
       case  U_WMR_F1:                     U_WMRF1_print(contents);                       break;
       case  U_WMR_F2:                     U_WMRF2_print(contents);                       break;
       case  U_WMR_F3:                     U_WMRF3_print(contents);                       break;
       case  U_WMR_F4:                     U_WMRF4_print(contents);                       break;
       case  U_WMR_F5:                     U_WMRF5_print(contents);                       break;
       case  U_WMR_F6:                     U_WMRF6_print(contents);                       break;
       case  U_WMR_CREATEPALETTE:          U_WMRCREATEPALETTE_print(contents);            break;
       case  U_WMR_F8:                     U_WMRF8_print(contents);                       break;
       case  U_WMR_CREATEPATTERNBRUSH:     U_WMRCREATEPATTERNBRUSH_print(contents);       break;
       case  U_WMR_CREATEPENINDIRECT:      U_WMRCREATEPENINDIRECT_print(contents);        break;
       case  U_WMR_CREATEFONTINDIRECT:     U_WMRCREATEFONTINDIRECT_print(contents);       break;
       case  U_WMR_CREATEBRUSHINDIRECT:    U_WMRCREATEBRUSHINDIRECT_print(contents);      break;
       case  U_WMR_CREATEBITMAPINDIRECT:   U_WMRCREATEBITMAPINDIRECT_print(contents);     break;
       case  U_WMR_CREATEBITMAP:           U_WMRCREATEBITMAP_print(contents);             break;
       case  U_WMR_CREATEREGION:           U_WMRCREATEREGION_print(contents);             break;
       default:                            U_WMRNOTIMPLEMENTED_print(contents);           break;
    }  //end of switch
    return(size);
}


#ifdef __cplusplus
}
#endif
