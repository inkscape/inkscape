/**
  @file uemf.c
  
  @brief Functions for manipulating EMF files and structures.

  [U_EMR]_set all take data and return a pointer to memory holding the constructed record.  
  The size of that record is also returned in recsize.
  It is also in the second int32 in the record, but may have been byte swapped and so not usable.
  If something goes wrong a NULL pointer is returned and recsize is set to 0.
  
  Compile with "U_VALGRIND" defined defined to enable code which lets valgrind check each record for
  uninitialized data.
  
  Compile with "SOL8" defined for Solaris 8 or 9 (Sparc).
*/

/*
File:      uemf.c
Version:   0.0.31
Date:      26-JAN-2016
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2016 David Mathog and California Institute of Technology (Caltech)
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iconv.h>
#include <wchar.h>
#include <errno.h>
#include <string.h>
#include <limits.h> // for INT_MAX, INT_MIN
#include <math.h>   // for U_ROUND()
#include <stddef.h> /* for offsetof() macro */
#if 0
#include <windef.h>    //Not actually used, looking for collisions
#include <winnt.h>    //Not actually used, looking for collisions
#include <wingdi.h>   //Not actually used, looking for collisions
#endif
#include "uemf.h"

//! \cond
/* one prototype from uemf_endian.  Put it here because end user should never need to see it, so
not in uemf.h or uemf_endian.h */
void U_swap2(void *ul, unsigned int count);
//! \endcond

/**
    \brief Look up the name of the EMR record by type.  Returns U_EMR_INVALID if out of range.
        
    \return name of the EMR record, "U_EMR_INVALID" if out of range.
    \param idx  EMR record type. 
    
*/
char *U_emr_names(unsigned int idx){
   if(idx<U_EMR_MIN || idx > U_EMR_MAX){ idx = 0; }
   static char *U_EMR_NAMES[U_EMR_MAX+1]={
      "U_EMR_INVALID",          
      "U_EMR_HEADER",          
      "U_EMR_POLYBEZIER",
      "U_EMR_POLYGON",
      "U_EMR_POLYLINE",
      "U_EMR_POLYBEZIERTO",
      "U_EMR_POLYLINETO",
      "U_EMR_POLYPOLYLINE",
      "U_EMR_POLYPOLYGON",
      "U_EMR_SETWINDOWEXTEX",
      "U_EMR_SETWINDOWORGEX",
      "U_EMR_SETVIEWPORTEXTEX",
      "U_EMR_SETVIEWPORTORGEX",
      "U_EMR_SETBRUSHORGEX",
      "U_EMR_EOF",
      "U_EMR_SETPIXELV",
      "U_EMR_SETMAPPERFLAGS",
      "U_EMR_SETMAPMODE",
      "U_EMR_SETBKMODE",
      "U_EMR_SETPOLYFILLMODE",
      "U_EMR_SETROP2",
      "U_EMR_SETSTRETCHBLTMODE",
      "U_EMR_SETTEXTALIGN",
      "U_EMR_SETCOLORADJUSTMENT",
      "U_EMR_SETTEXTCOLOR",
      "U_EMR_SETBKCOLOR",
      "U_EMR_OFFSETCLIPRGN",
      "U_EMR_MOVETOEX",
      "U_EMR_SETMETARGN",
      "U_EMR_EXCLUDECLIPRECT",
      "U_EMR_INTERSECTCLIPRECT",
      "U_EMR_SCALEVIEWPORTEXTEX",
      "U_EMR_SCALEWINDOWEXTEX",
      "U_EMR_SAVEDC",
      "U_EMR_RESTOREDC",
      "U_EMR_SETWORLDTRANSFORM",
      "U_EMR_MODIFYWORLDTRANSFORM",
      "U_EMR_SELECTOBJECT",
      "U_EMR_CREATEPEN",
      "U_EMR_CREATEBRUSHINDIRECT",
      "U_EMR_DELETEOBJECT",
      "U_EMR_ANGLEARC",
      "U_EMR_ELLIPSE",
      "U_EMR_RECTANGLE",
      "U_EMR_ROUNDRECT",
      "U_EMR_ARC",
      "U_EMR_CHORD",
      "U_EMR_PIE",
      "U_EMR_SELECTPALETTE",
      "U_EMR_CREATEPALETTE",
      "U_EMR_SETPALETTEENTRIES",
      "U_EMR_RESIZEPALETTE",
      "U_EMR_REALIZEPALETTE",
      "U_EMR_EXTFLOODFILL",
      "U_EMR_LINETO",
      "U_EMR_ARCTO",
      "U_EMR_POLYDRAW",
      "U_EMR_SETARCDIRECTION",
      "U_EMR_SETMITERLIMIT",
      "U_EMR_BEGINPATH",
      "U_EMR_ENDPATH",
      "U_EMR_CLOSEFIGURE",
      "U_EMR_FILLPATH",
      "U_EMR_STROKEANDFILLPATH",
      "U_EMR_STROKEPATH",
      "U_EMR_FLATTENPATH",
      "U_EMR_WIDENPATH",
      "U_EMR_SELECTCLIPPATH",
      "U_EMR_ABORTPATH",
      "U_EMR_UNDEF69",
      "U_EMR_COMMENT",
      "U_EMR_FILLRGN",
      "U_EMR_FRAMERGN",
      "U_EMR_INVERTRGN",
      "U_EMR_PAINTRGN",
      "U_EMR_EXTSELECTCLIPRGN",
      "U_EMR_BITBLT",
      "U_EMR_STRETCHBLT",
      "U_EMR_MASKBLT",
      "U_EMR_PLGBLT",
      "U_EMR_SETDIBITSTODEVICE",
      "U_EMR_STRETCHDIBITS",
      "U_EMR_EXTCREATEFONTINDIRECTW",
      "U_EMR_EXTTEXTOUTA",
      "U_EMR_EXTTEXTOUTW",
      "U_EMR_POLYBEZIER16",
      "U_EMR_POLYGON16",
      "U_EMR_POLYLINE16",
      "U_EMR_POLYBEZIERTO16",
      "U_EMR_POLYLINETO16",
      "U_EMR_POLYPOLYLINE16",
      "U_EMR_POLYPOLYGON16",
      "U_EMR_POLYDRAW16",
      "U_EMR_CREATEMONOBRUSH",
      "U_EMR_CREATEDIBPATTERNBRUSHPT",
      "U_EMR_EXTCREATEPEN",
      "U_EMR_POLYTEXTOUTA",
      "U_EMR_POLYTEXTOUTW",
      "U_EMR_SETICMMODE",
      "U_EMR_CREATECOLORSPACE",
      "U_EMR_SETCOLORSPACE",
      "U_EMR_DELETECOLORSPACE",
      "U_EMR_GLSRECORD",
      "U_EMR_GLSBOUNDEDRECORD",
      "U_EMR_PIXELFORMAT",
      "U_EMR_DRAWESCAPE",
      "U_EMR_EXTESCAPE",
      "U_EMR_UNDEF107",
      "U_EMR_SMALLTEXTOUT",
      "U_EMR_FORCEUFIMAPPING",
      "U_EMR_NAMEDESCAPE",
      "U_EMR_COLORCORRECTPALETTE",
      "U_EMR_SETICMPROFILEA",
      "U_EMR_SETICMPROFILEW",
      "U_EMR_ALPHABLEND",
      "U_EMR_SETLAYOUT",
      "U_EMR_TRANSPARENTBLT",
      "U_EMR_UNDEF117",
      "U_EMR_GRADIENTFILL",
      "U_EMR_SETLINKEDUFIS",
      "U_EMR_SETTEXTJUSTIFICATION",
      "U_EMR_COLORMATCHTOTARGETW",
      "U_EMR_CREATECOLORSPACEW"
   };
   return(U_EMR_NAMES[idx]);
}



/* **********************************************************************************************
These definitions are for code pieces that are used many times in the following implementation.  These
definitions are not needed in end user code, so they are here rather than in uemf.h.
*********************************************************************************************** */

//! @cond

// this one may also be used A=Msk,B=MskBmi and F=cbMsk 
#define SET_CB_FROM_PXBMI(A,B,C,D,E,F)    /* A=Px, B=Bmi, C=cbImage, D=cbImage4, E=cbBmi, F=cbPx */ \
   if(A){\
     if(!B)return(NULL);  /* size is derived from U_BITMAPINFO, but NOT from its size field, go figure*/ \
     C = F;\
     D = UP4(C);          /*  pixel array might not be a multiples of 4 bytes*/ \
     E    = sizeof(U_BITMAPINFOHEADER) +  4 * get_real_color_count((const char *) &(B->bmiHeader));  /*  bmiheader + colortable*/ \
   }\
   else { C = 0; D = 0; E=0; }

// variable "off" must be declared in the function

#define APPEND_PXBMISRC(A,B,C,D,E,F,G) /* A=record, B=U_EMR,C=cbBmi, D=Bmi, E=Px, F=cbImage, G=cbImage4 */ \
      if(C){\
         memcpy(A + off, D, C);\
         ((B *) A)->offBmiSrc  = off;\
         ((B *) A)->cbBmiSrc   = C;\
         off += C;\
         memcpy(A + off, E, F);\
         ((B *) A)->offBitsSrc = off;\
         ((B *) A)->cbBitsSrc  = F;\
         if(G - F){  \
            off += F;\
            memset(A + off, 0, G - F); \
         }\
      }\
      else {\
         ((B *) A)->offBmiSrc  = 0;\
         ((B *) A)->cbBmiSrc   = 0;\
         ((B *) A)->offBitsSrc = 0;\
         ((B *) A)->cbBitsSrc  = 0;\
      }

// variable "off" must be declared in the function

#define APPEND_MSKBMISRC(A,B,C,D,E,F,G) /* A=record, B=U_EMR*,C=cbMskBmi, D=MskBmi, E=Msk, F=cbMskImage, G=cbMskImage4 */ \
      if(C){\
         memcpy(A + off, D, C);\
         ((B *) A)->offBmiMask   = off;\
         ((B *) A)->cbBmiMask    = C;\
         off += C;\
         memcpy(A + off, Msk, F);\
         ((B *) A)->offBitsMask  = off;\
         ((B *) A)->cbBitsMask   = F;\
         if(G - F){  memset(A + off, 0, G - F); }\
      }\
      else {\
         ((B *) A)->offBmiMask   = 0;\
         ((B *) A)->cbBmiMask    = 0;\
         ((B *) A)->offBitsMask  = 0;\
         ((B *) A)->cbBitsMask   = 0;\
      }
      
//! @endcond

/* **********************************************************************************************
These functions are used for development and debugging and should be be includied in production code.
*********************************************************************************************** */

/**
    \brief Debugging utility, used with valgrind to find uninitialized values.  Not for use in production code.
    \param buf memory area to examine !
    \param size length in bytes of buf!
*/
int memprobe(
      const void   *buf,
      size_t        size
   ){
   int sum=0;
   char *ptr=(char *)buf;
   for(;size;size--,ptr++){ sum += *ptr; }  // read all bytes, trigger valgrind warning if any uninitialized
   return(sum);
}

/**
    \brief Dump an EMFHANDLES structure.  Not for use in production code.
    \param string  Text to output before dumping eht structure
    \param handle  Handle
    \param eht     EMFHANDLES structure to dump
*/
void dumpeht(
     char         *string, 
     unsigned int *handle,
     EMFHANDLES   *eht
  ){
  uint32_t i;
  printf("%s\n",string);
  printf("sptr: %d peak: %d top: %d\n",eht->sptr,eht->peak,eht->top);
  if(handle){
    printf("handle: %d \n",*handle);
  }
  for(i=0;i<=5;i++){
     printf("table[%d]: %d\n",i,eht->table[i]);
  }
  for(i=1;i<=5;i++){
     printf("stack[%d]: %d\n",i,eht->stack[i]);
  }
}

/* **********************************************************************************************
These functions are used for Image conversions and other
utility operations.  Character type conversions are in uemf_utf.c
*********************************************************************************************** */

/**
    \brief Make up an approximate dx array to pass to emrtext_set(), based on character height and weight.
    
    Take abs. value of character height, get width by multiplying by 0.6, and correct weight
    approximately, with formula (measured on screen for one text line of Arial).
    Caller is responsible for free() on the returned pointer.
    
    \return pointer to dx array
    \param height  character height (absolute value will be used)
    \param weight  LF_Weight Enumeration (character weight) 
    \param members Number of entries to put into dx
    
*/
uint32_t *dx_set(
      int32_t  height,
      uint32_t weight,
      uint32_t members
   ){
   uint32_t i, width, *dx;
   dx = (uint32_t *) malloc(members * sizeof(uint32_t));
   if(dx){
       if(U_FW_DONTCARE == weight)weight=U_FW_NORMAL;
       width = (uint32_t) U_ROUND(((float) (height > 0 ? height : -height)) * 0.6 * (0.00024*(float) weight + 0.904));
       for ( i = 0; i < members; i++ ){ dx[i] = width; }
   }
   return(dx);
}

/**
    \brief Look up the properties (a bit map) of a type of EMR record.
          Bits that may be set are defined in "Draw Properties" in uemf.h, they are U_DRAW_NOTEMPTY, etc..
        
    \return bitmap of EMR record properties, or U_EMR_INVALID on error or release of all memory
    \param type EMR record type.  If U_EMR_INVALID release memory. (There is no U_EMR_INVALID EMR record type)
    
*/
uint32_t emr_properties(uint32_t type){
   static uint32_t *table=NULL;
   uint32_t result = U_EMR_INVALID;  // initialized to indicate an error (on a lookup) or nothing (on a memory release) 
   if(type == U_EMR_INVALID){
      if(table)free(table);
      table=NULL;
   }
   else if(type>=1 && type<U_EMR_MAX){
      if(!table){
         table = (uint32_t *) malloc(sizeof(uint32_t)*(1 + U_EMR_MAX));
         if(!table)return(result); 
   //                                                               0x80 0x40 0x20 0x10 0x08 0x04 0x02 0x01
   //                 Path properties (U_DRAW_*)                    TEXT      ALTERS    ONLYTO    VISIBLE   
   //                                                                    PATH      FORCE     CLOSED    NOTEMPTY
         table[  0] = 0x00;     //   Does not map to any EMR record
         table[  1] = 0x80;     //   U_EMRHEADER                    1    0    0    0    0    0    0    0 
         table[  2] = 0x83;     //   U_EMRPOLYBEZIER                1    0    0    0    0    0    1    1
         table[  3] = 0x87;     //   U_EMRPOLYGON                   1    0    0    0    0    1    1    1
         table[  4] = 0x83;     //   U_EMRPOLYLINE                  1    0    0    0    0    0    1    1
         table[  5] = 0x8B;     //   U_EMRPOLYBEZIERTO              1    0    0    0    1    0    1    1
         table[  6] = 0x8B;     //   U_EMRPOLYLINETO                1    0    0    0    1    0    1    1
         table[  7] = 0x83;     //   U_EMRPOLYPOLYLINE              1    0    0    0    0    0    1    1
         table[  8] = 0x87;     //   U_EMRPOLYPOLYGON               1    0    0    0    0    1    1    1
         table[  9] = 0xA0;     //   U_EMRSETWINDOWEXTEX            1    0    1    0    0    0    0    0
         table[ 10] = 0xA0;     //   U_EMRSETWINDOWORGEX            1    0    1    0    0    0    0    0
         table[ 11] = 0xA0;     //   U_EMRSETVIEWPORTEXTEX          1    0    1    0    0    0    0    0
         table[ 12] = 0xA0;     //   U_EMRSETVIEWPORTORGEX          1    0    1    0    0    0    0    0
         table[ 13] = 0xA0;     //   U_EMRSETBRUSHORGEX             1    0    1    0    0    0    0    0
         table[ 14] = 0x82;     //   U_EMREOF                       1    0    1    0    0    0    0    0  Force out any pending draw
         table[ 15] = 0x82;     //   U_EMRSETPIXELV                 1    0    0    0    0    0    1    0
         table[ 16] = 0xA0;     //   U_EMRSETMAPPERFLAGS            1    0    1    0    0    0    0    0
         table[ 17] = 0xA0;     //   U_EMRSETMAPMODE                1    0    1    0    0    0    0    0
         table[ 18] = 0x20;     //   U_EMRSETBKMODE                 0    0    1    0    0    0    0    0
         table[ 19] = 0xA0;     //   U_EMRSETPOLYFILLMODE           1    0    1    0    0    0    0    0
         table[ 20] = 0xA0;     //   U_EMRSETROP2                   1    0    1    0    0    0    0    0
         table[ 21] = 0xA0;     //   U_EMRSETSTRETCHBLTMODE         1    0    1    0    0    0    0    0
         table[ 22] = 0x20;     //   U_EMRSETTEXTALIGN              0    0    1    0    0    0    0    0
         table[ 23] = 0xA0;     //   U_EMRSETCOLORADJUSTMENT        1    0    1    0    0    0    0    0
         table[ 24] = 0x20;     //   U_EMRSETTEXTCOLOR              0    0    1    0    0    0    0    0
         table[ 25] = 0x20;     //   U_EMRSETBKCOLOR                0    0    1    0    0    0    0    0
         table[ 26] = 0xA0;     //   U_EMROFFSETCLIPRGN             1    0    1    0    0    0    0    0
         table[ 27] = 0x89;     //   U_EMRMOVETOEX                  1    0    0    0    1    0    0    1
         table[ 28] = 0xA0;     //   U_EMRSETMETARGN                1    0    1    0    0    0    0    0
         table[ 29] = 0xA0;     //   U_EMREXCLUDECLIPRECT           1    0    1    0    0    0    0    0
         table[ 30] = 0xA0;     //   U_EMRINTERSECTCLIPRECT         1    0    1    0    0    0    0    0
         table[ 31] = 0xA0;     //   U_EMRSCALEVIEWPORTEXTEX        1    0    1    0    0    0    0    0
         table[ 32] = 0xA0;     //   U_EMRSCALEWINDOWEXTEX          1    0    1    0    0    0    0    0
         table[ 33] = 0xA0;     //   U_EMRSAVEDC                    1    0    1    0    0    0    0    0
         table[ 34] = 0xA0;     //   U_EMRRESTOREDC                 1    0    1    0    0    0    0    0
         table[ 35] = 0xA0;     //   U_EMRSETWORLDTRANSFORM         1    0    1    0    0    0    0    0
         table[ 36] = 0xA0;     //   U_EMRMODIFYWORLDTRANSFORM      1    0    1    0    0    0    0    0
         table[ 37] = 0x20;     //   U_EMRSELECTOBJECT              0    0    1    0    0    0    0    0
         table[ 38] = 0x20;     //   U_EMRCREATEPEN                 0    0    1    0    0    0    0    0
         table[ 39] = 0x20;     //   U_EMRCREATEBRUSHINDIRECT       0    0    1    0    0    0    0    0
         table[ 40] = 0x20;     //   U_EMRDELETEOBJECT              0    0    1    0    0    0    0    0
         table[ 41] = 0x83;     //   U_EMRANGLEARC                  1    0    0    0    0    0    1    1
         table[ 42] = 0x87;     //   U_EMRELLIPSE                   1    0    0    0    0    1    1    1
         table[ 43] = 0x87;     //   U_EMRRECTANGLE                 1    0    0    0    0    1    1    1
         table[ 44] = 0x87;     //   U_EMRROUNDRECT                 1    0    0    0    0    1    1    1
         table[ 45] = 0x83;     //   U_EMRARC                       1    0    0    0    0    0    1    1
         table[ 46] = 0x87;     //   U_EMRCHORD                     1    0    0    0    0    1    1    1
         table[ 47] = 0x87;     //   U_EMRPIE                       1    0    0    0    0    1    1    1
         table[ 48] = 0xA0;     //   U_EMRSELECTPALETTE             1    0    1    0    0    0    0    0
         table[ 49] = 0xA0;     //   U_EMRCREATEPALETTE             1    0    1    0    0    0    0    0
         table[ 50] = 0xA0;     //   U_EMRSETPALETTEENTRIES         1    0    1    0    0    0    0    0
         table[ 51] = 0xA0;     //   U_EMRRESIZEPALETTE             1    0    1    0    0    0    0    0
         table[ 52] = 0xA0;     //   U_EMRREALIZEPALETTE            1    0    1    0    0    0    0    0
         table[ 53] = 0x82;     //   U_EMREXTFLOODFILL              1    0    0    0    0    0    1    0
         table[ 54] = 0x8B;     //   U_EMRLINETO                    1    0    0    0    1    0    1    1
         table[ 55] = 0x8B;     //   U_EMRARCTO                     1    0    0    0    1    0    1    1
         table[ 56] = 0x83;     //   U_EMRPOLYDRAW                  1    0    0    0    0    0    1    1
         table[ 57] = 0xA0;     //   U_EMRSETARCDIRECTION           1    0    1    0    0    0    0    0
         table[ 58] = 0xA0;     //   U_EMRSETMITERLIMIT             1    0    1    0    0    0    0    0
         table[ 59] = 0xE0;     //   U_EMRBEGINPATH                 1    1    1    0    0    0    0    0
         table[ 60] = 0x80;     //   U_EMRENDPATH                   1    0    0    0    0    0    0    0
         table[ 61] = 0x84;     //   U_EMRCLOSEFIGURE               1    0    0    0    0    1    0    0
         table[ 62] = 0x94;     //   U_EMRFILLPATH                  1    0    0    1    0    1    0    0
         table[ 63] = 0x94;     //   U_EMRSTROKEANDFILLPATH         1    0    0    1    0    1    0    0
         table[ 64] = 0x90;     //   U_EMRSTROKEPATH                1    0    0    1    0    0    0    0
         table[ 65] = 0xA0;     //   U_EMRFLATTENPATH               1    0    1    0    0    0    0    0
         table[ 66] = 0xA0;     //   U_EMRWIDENPATH                 1    0    1    0    0    0    0    0
         table[ 67] = 0x80;     //   U_EMRSELECTCLIPPATH            1    0    0    0    0    0    0    0  consumes the path, draws nothing
         table[ 68] = 0xA0;     //   U_EMRABORTPATH                 1    0    1    0    0    0    0    0
         table[ 69] = 0xA0;     //   U_EMRUNDEF69                   1    0    1    0    0    0    0    0
         table[ 70] = 0x00;     //   U_EMRCOMMENT                   0    0    0    0    0    0    0    0
         table[ 71] = 0x82;     //   U_EMRFILLRGN                   1    0    0    0    0    0    1    0
         table[ 72] = 0x82;     //   U_EMRFRAMERGN                  1    0    0    0    0    0    1    0
         table[ 73] = 0x82;     //   U_EMRINVERTRGN                 1    0    0    0    0    0    1    0
         table[ 74] = 0x82;     //   U_EMRPAINTRGN                  1    0    0    0    0    0    1    0
         table[ 75] = 0xA0;     //   U_EMREXTSELECTCLIPRGN          1    0    1    0    0    0    0    0
         table[ 76] = 0x82;     //   U_EMRBITBLT                    1    0    0    0    0    0    1    0
         table[ 77] = 0x82;     //   U_EMRSTRETCHBLT                1    0    0    0    0    0    1    0
         table[ 78] = 0x82;     //   U_EMRMASKBLT                   1    0    0    0    0    0    1    0
         table[ 79] = 0x82;     //   U_EMRPLGBLT                    1    0    0    0    0    0    1    0
         table[ 80] = 0xA0;     //   U_EMRSETDIBITSTODEVICE         1    0    1    0    0    0    0    0
         table[ 81] = 0xA0;     //   U_EMRSTRETCHDIBITS             1    0    1    0    0    0    0    0
         table[ 82] = 0x20;     //   U_EMREXTCREATEFONTINDIRECTW    0    0    1    0    0    0    0    0
         table[ 83] = 0x02;     //   U_EMREXTTEXTOUTA               0    0    0    0    0    0    1    0
         table[ 84] = 0x02;     //   U_EMREXTTEXTOUTW               0    0    0    0    0    0    1    0
         table[ 85] = 0x83;     //   U_EMRPOLYBEZIER16              1    0    0    0    0    0    1    1
         table[ 86] = 0x83;     //   U_EMRPOLYGON16                 1    0    0    0    0    0    1    1
         table[ 87] = 0x83;     //   U_EMRPOLYLINE16                1    0    0    0    0    0    1    1
         table[ 88] = 0x8B;     //   U_EMRPOLYBEZIERTO16            1    0    0    0    1    0    1    1
         table[ 89] = 0x8B;     //   U_EMRPOLYLINETO16              1    0    0    0    1    0    1    1
         table[ 90] = 0x83;     //   U_EMRPOLYPOLYLINE16            1    0    0    0    0    0    1    1
         table[ 91] = 0x87;     //   U_EMRPOLYPOLYGON16             1    0    0    0    0    1    1    1
         table[ 92] = 0x83;     //   U_EMRPOLYDRAW16                1    0    0    0    0    0    1    1
         table[ 93] = 0x80;     //   U_EMRCREATEMONOBRUSH           1    0    0    0    0    0    0    0  Not selected yet, so no change in drawing conditions
         table[ 94] = 0x80;     //   U_EMRCREATEDIBPATTERNBRUSHPT   1    0    0    0    0    0    0    0  "
         table[ 95] = 0x00;     //   U_EMREXTCREATEPEN              0    0    0    0    0    0    0    0  "
         table[ 96] = 0x02;     //   U_EMRPOLYTEXTOUTA              0    0    0    0    0    0    1    0
         table[ 97] = 0x02;     //   U_EMRPOLYTEXTOUTW              0    0    0    0    0    0    1    0
         table[ 98] = 0xA0;     //   U_EMRSETICMMODE                1    0    1    0    0    0    0    0
         table[ 99] = 0xA0;     //   U_EMRCREATECOLORSPACE          1    0    1    0    0    0    0    0
         table[100] = 0xA0;     //   U_EMRSETCOLORSPACE             1    0    1    0    0    0    0    0
         table[101] = 0xA0;     //   U_EMRDELETECOLORSPACE          1    0    1    0    0    0    0    0
         table[102] = 0xA0;     //   U_EMRGLSRECORD                 1    0    1    0    0    0    0    0
         table[103] = 0xA0;     //   U_EMRGLSBOUNDEDRECORD          1    0    1    0    0    0    0    0
         table[104] = 0xA0;     //   U_EMRPIXELFORMAT               1    0    1    0    0    0    0    0
         table[105] = 0xA0;     //   U_EMRDRAWESCAPE                1    0    1    0    0    0    0    0
         table[106] = 0xA0;     //   U_EMREXTESCAPE                 1    0    1    0    0    0    0    0
         table[107] = 0xA0;     //   U_EMRUNDEF107                  1    0    1    0    0    0    0    0
         table[108] = 0x02;     //   U_EMRSMALLTEXTOUT              0    0    0    0    0    0    1    0
         table[109] = 0xA0;     //   U_EMRFORCEUFIMAPPING           1    0    1    0    0    0    0    0
         table[110] = 0xA0;     //   U_EMRNAMEDESCAPE               1    0    1    0    0    0    0    0
         table[111] = 0xA0;     //   U_EMRCOLORCORRECTPALETTE       1    0    1    0    0    0    0    0
         table[112] = 0xA0;     //   U_EMRSETICMPROFILEA            1    0    1    0    0    0    0    0
         table[113] = 0xA0;     //   U_EMRSETICMPROFILEW            1    0    1    0    0    0    0    0
         table[114] = 0x82;     //   U_EMRALPHABLEND                1    0    0    0    0    0    1    0
         table[115] = 0xA0;     //   U_EMRSETLAYOUT                 1    0    1    0    0    0    0    0
         table[116] = 0x82;     //   U_EMRTRANSPARENTBLT            1    0    0    0    0    0    1    0
         table[117] = 0xA0;     //   U_EMRUNDEF117                  1    0    1    0    0    0    0    0
         table[118] = 0x82;     //   U_EMRGRADIENTFILL              1    0    1    0    0    0    1    0
         table[119] = 0xA0;     //   U_EMRSETLINKEDUFIS             1    0    1    0    0    0    0    0
         table[120] = 0x20;     //   U_EMRSETTEXTJUSTIFICATION      0    0    1    0    0    0    0    0
         table[121] = 0xA0;     //   U_EMRCOLORMATCHTOTARGETW       1    0    1    0    0    0    0    0
         table[122] = 0xA0;     //   U_EMRCREATECOLORSPACEW         1    0    1    0    0    0    0    0
      }
      result = table[type];
   }
   return(result);
}

/**
    \brief Derive from bounding rect, start and end radials, for arc, chord, or pie, the center, start, and end points, and the bounding rectangle.
    
    \return 0 on success, other values on errors.
    \param rclBox     bounding rectangle
    \param ArcStart   start of arc
    \param ArcEnd     end of arc
    \param f1         1 if rotation angle >= 180, else 0
    \param f2         Rotation direction, 1 if counter clockwise, else 0
    \param center     Center coordinates
    \param start      Start coordinates (point on the ellipse defined by rect)
    \param end        End coordinates (point on the ellipse defined by rect)
    \param size       W,H of the x,y axes of the bounding rectangle.
*/
int emr_arc_points_common(
       PU_RECTL          rclBox,
       PU_POINTL         ArcStart,
       PU_POINTL         ArcEnd,
       int              *f1,
       int               f2,
       PU_PAIRF          center,
       PU_PAIRF          start,
       PU_PAIRF          end,
       PU_PAIRF          size
    ){
    U_PAIRF estart;     // EMF start position, defines a radial
    U_PAIRF eend;       // EMF end   position, defines a radial
    U_PAIRF vec_estart; // define a unit vector from the center to estart
    U_PAIRF vec_eend;   // define a unit vector from the center to eend
    U_PAIRF radii;      // x,y radii of ellipse 
    U_PAIRF ratio;      // intermediate value
    float scale, cross;
    center->x   = ((float)(rclBox->left   + rclBox->right ))/2.0;
    center->y   = ((float)(rclBox->top    + rclBox->bottom))/2.0;
    size->x     =  (float)(rclBox->right  - rclBox->left );
    size->y     =  (float)(rclBox->bottom - rclBox->top  );
    estart.x    =  (float)(ArcStart->x);
    estart.y    =  (float)(ArcStart->y);
    eend.x      =  (float)(ArcEnd->x);
    eend.y      =  (float)(ArcEnd->y);
    radii.x     =  size->x/2.0;
    radii.y     =  size->y/2.0;

    vec_estart.x  = (estart.x - center->x); // initial vector, not unit length
    vec_estart.y  = (estart.y - center->y);
    scale         = sqrt(vec_estart.x*vec_estart.x + vec_estart.y*vec_estart.y);
    if(!scale)return(1);                    // bogus record, has start at center
    vec_estart.x /= scale;                  // now a unit vector
    vec_estart.y /= scale;

    vec_eend.x    = (eend.x - center->x);   // initial vector, not unit length
    vec_eend.y    = (eend.y - center->y);
    scale         = sqrt(vec_eend.x*vec_eend.x + vec_eend.y*vec_eend.y);
    if(!scale)return(2);                    // bogus record, has end at center
    vec_eend.x   /= scale;                  // now a unit vector
    vec_eend.y   /= scale;

    
    // Find the intersection of the vectors with the ellipse.  With no loss of generality
    // we can translate the ellipse to the origin, then we just need to find tu (t a factor, u the unit vector)
    // that also satisfies (x/Rx)^2 + (y/Ry)^2 = 1.  x is t*(ux), y is t*(uy), where ux,uy are the x,y components
    // of the unit vector.  Substituting gives:
    // (t*(ux)/Rx)^2 + (t*(uy)/Ry)^2 = 1
    // t^2 = 1/(  (ux/Rx)^2 + (uy/Ry)^2 )
    // t = sqrt(1/(  (ux/Rx)^2 + (uy/Ry)^2 ))

    ratio.x  = vec_estart.x/radii.x;
    ratio.y  = vec_estart.y/radii.y;
    ratio.x *= ratio.x;                     // we only use the square
    ratio.y *= ratio.y;
    scale    = 1.0/sqrt(ratio.x + ratio.y);
    start->x = center->x + scale * vec_estart.x;
    start->y = center->y + scale * vec_estart.y;

    ratio.x  = vec_eend.x/radii.x;
    ratio.y  = vec_eend.y/radii.y;
    ratio.x *= ratio.x;                     // we only use the square
    ratio.y *= ratio.y;
    scale    = 1.0/sqrt(ratio.x + ratio.y);
    end->x   = center->x + scale * vec_eend.x;
    end->y   = center->y + scale * vec_eend.y;
    
    //lastly figure out if the swept angle is >180 degrees or not, based on the direction of rotation
    //and the two unit vectors.
    
    cross = vec_estart.x * vec_eend.y - vec_estart.y * vec_eend.x;
    if(!f2){  // counter clockwise rotation
      if(cross >=0){ *f1 = 1; }
      else {         *f1 = 0; }
    }
    else {
      if(cross >=0){ *f1 = 0; }
      else {         *f1 = 1; }
    }
    
    
    return(0);
}

/**
    \brief Derive from an EMF arc, chord, or pie the center, start, and end points, and the bounding rectangle.
    
    \return 0 on success, other values on errors.
    \param record     U_EMRPIE, U_EMRCHORD, or _EMRARC record
    \param f1         1 if rotation angle >= 180, else 0
    \param f2         Rotation direction, 1 if counter clockwise, else 0
    \param center     Center coordinates
    \param start      Start coordinates (point on the ellipse defined by rect)
    \param end        End coordinates (point on the ellipse defined by rect)
    \param size       W,H of the x,y axes of the bounding rectangle.
*/
int emr_arc_points(
       PU_ENHMETARECORD  record,
       int              *f1,
       int               f2,
       PU_PAIRF          center,
       PU_PAIRF          start,
       PU_PAIRF          end,
       PU_PAIRF          size
    ){
    PU_EMRARC pEmr = (PU_EMRARC) (record);
    return emr_arc_points_common(&(pEmr->rclBox), &(pEmr->ptlStart), &(pEmr->ptlEnd), f1, f2, center, start, end, size );
}

/**
    \brief Convert a U_RGBA 32 bit pixmap to one of many different types of DIB pixmaps.
    
    Conversions to formats using color tables assume that the color table can hold every color
    in the input image.  If that assumption is false then the conversion will fail.  Conversion
    from 8 bit color to N bit colors (N<8) do so by shifting the appropriate number of bits. 
    
    \return 0 on success, other values on errors.
    \param px         DIB pixel array
    \param cbPx       DIB pixel array size in bytes
    \param ct         DIB color table
    \param numCt      DIB color table number of entries
    \param rgba_px    U_RGBA pixel array (32 bits)
    \param w          Width of pixel array
    \param h          Height of pixel array
    \param stride     Row stride of input pixel array in bytes
    \param colortype  DIB BitCount Enumeration
    \param use_ct     If true use color table (only for 1-16 bit DIBs).
    \param invert     If DIB rows are in opposite order from RGBA rows
*/
int RGBA_to_DIB(
       char      **px,
       uint32_t   *cbPx,
       PU_RGBQUAD *ct,
       int        *numCt,
       const char *rgba_px,
       int         w,
       int         h,
       int         stride,
       uint32_t    colortype,
       int         use_ct,
       int         invert
   ){
   int          bs;
   int          pad;
   int          i,j,k;
   int          istart, iend, iinc;
   uint8_t      r,g,b,a,tmp8;
   char        *pxptr;
   const char  *rptr;
   int          found;
   int          usedbytes;
   U_RGBQUAD    color;
   PU_RGBQUAD   lct;
   int32_t      index;
   
   *px=NULL;
   *ct=NULL;
   *numCt=0;
   *cbPx=0;
   // sanity checking
   if(!w || !h || !stride || !colortype || !rgba_px)return(1);
   if(use_ct && colortype  >= U_BCBM_COLOR16)return(2);  //color tables not used above 16 bit pixels
   if(!use_ct && colortype < U_BCBM_COLOR16)return(3);   //color tables mandatory for < 16 bit

   bs = colortype/8;
   if(bs<1){
      usedbytes = (w*colortype + 7)/8;      // width of line in fully and partially occupied bytes
   }
   else {
      usedbytes = w*bs;
   }
   pad = UP4(usedbytes) - usedbytes;        // DIB rows must be aligned on 4 byte boundaries, they are padded at the end to accomplish this.;
   *cbPx = h * (usedbytes + pad);           // Rows must start on a 4 byte boundary!
   *px = (char *) malloc(*cbPx);  
   if(!px)return(4);
   if(use_ct){
       *numCt = 1<< colortype;
       if(*numCt >w*h)*numCt=w*h;
       lct = (PU_RGBQUAD) malloc(*numCt * sizeof(U_RGBQUAD));
       if(!lct)return(5);
       *ct = lct;
   }
   
   if(invert){
     istart = h-1;
     iend   = -1;
     iinc   = -1;
   }
   else {
     istart = 0;
     iend   = h;
     iinc   = 1;
   }

   found = 0;
   tmp8  = 0;
   pxptr = *px;
   for(i=istart; i!=iend; i+=iinc){
      rptr= rgba_px + i*stride;
      for(j=0; j<w; j++){
          r = *rptr++;
          g = *rptr++;
          b = *rptr++;
          a = *rptr++;
          if(use_ct){
             color = U_BGRA(r,g,b,a); // color has order in memory: b,g,r,a, same as EMF+ ARGB
             index = -1;
             for(lct = *ct, k=0; k<found; k++,lct++){  // Is this color in the table (VERY inefficient if there are a lot of colors!!!)
                if(*(uint32_t *)lct != *(uint32_t *) &color)continue;
                index =k;
                break;
             }
             if(index==-1){  // add a color
                found++;
                if(found > *numCt){  // More colors found than are supported by the color table
                   free(*ct);
                   free(*px);
                   *numCt=0;
                   *cbPx=0;
                   return(6);
                }
                index = found - 1;
                *lct = color;
             }
             switch(colortype){
                case U_BCBM_MONOCHROME: // 2 colors.    bmiColors array has two entries
                    tmp8 = tmp8 >> 1;      // This seems wrong, as it fills from the top of each byte.  But it works.
                    tmp8 |= index << 7;
                    if(!((j+1) % 8)){
                       *pxptr++ = tmp8;
                       tmp8     = 0;
                    }
                    break;           
                case U_BCBM_COLOR4:     // 2^4 colors.  bmiColors array has 16 entries                 
                    tmp8 = tmp8 << 4;
                    tmp8 |= index;
                    if(!((j+1) % 2)){
                       *pxptr++ = tmp8;
                       tmp8     = 0;
                    }
                    break;           
                case U_BCBM_COLOR8:     // 2^8 colors.  bmiColors array has 256 entries 
                    tmp8     = index;               
                    *pxptr++ = tmp8;
                    break;           
                case U_BCBM_COLOR16:    // 2^16 colors. (Several different color methods))
                case U_BCBM_COLOR24:    // 2^24 colors. bmiColors is not used. Pixels are U_RGBTRIPLE.
                case U_BCBM_COLOR32:    // 2^32 colors. bmiColors is not used. Pixels are U_RGBQUAD.
                case U_BCBM_EXPLICIT:   // Derinved from JPG or PNG compressed image or ?   
                default:
                    return(7);            // This should not be possible, but might happen with memory corruption  
             }
          }
          else {
             switch(colortype){
                case U_BCBM_COLOR16:        // 2^16 colors. (Several different color methods)) 
                   b /= 8; g /= 8; r /= 8;
                   // Do it in this way so that the bytes are always stored Little Endian
                   tmp8  = b;
                   tmp8 |= g<<5;            // least significant 3 bits of green
                   *pxptr++ = tmp8;
                   tmp8  = g>>3;            // most  significant 2 bits of green (there are only 5 bits of data)
                   tmp8 |= r<<2;
                   *pxptr++ = tmp8;
                   break;           
                case U_BCBM_COLOR24:        // 2^24 colors. bmiColors is not used. Pixels are U_RGBTRIPLE. 
                   *pxptr++ = b;
                   *pxptr++ = g;
                   *pxptr++ = r;
                   break;          
                case U_BCBM_COLOR32:        // 2^32 colors. bmiColors is not used. Pixels are U_RGBQUAD.
                   *pxptr++ = b;
                   *pxptr++ = g;
                   *pxptr++ = r;
                   *pxptr++ = a;
                   break;           
                case U_BCBM_MONOCHROME:     // 2 colors.    bmiColors array has two entries                
                case U_BCBM_COLOR4:         // 2^4 colors.  bmiColors array has 16 entries                 
                case U_BCBM_COLOR8:         // 2^8 colors.  bmiColors array has 256 entries                
                case U_BCBM_EXPLICIT:       // Derinved from JPG or PNG compressed image or ?   
                default:
                  return(7);                // This should not be possible, but might happen with memory corruption  
             }
          }
      }
      if( use_ct && colortype == U_BCBM_MONOCHROME && (j % 8) ){
         *pxptr++ = tmp8;                   // Write last few indices
         tmp8 = 0;
      }
      if( use_ct && colortype == U_BCBM_COLOR4     && (j % 2) ){
         *pxptr++ = tmp8;                   // Write last few indices
         tmp8 = 0;
      }
      if(pad){
         memset(pxptr,0,pad);               // not strictly necessary, but set all bytes so that we can find important unset ones with valgrind
         pxptr += pad;
      }
   } 
   return(0);
}

/**
    \brief Get the actual number of colors in the color table from the BitMapInfoHeader.  
    \return Number of entries in the color table.
    \param Bmih  char * pointer to the U_BITMAPINFOHEADER
    
    BitmapInfoHeader may list 0 for some types which implies the maximum value.
    If the image is big enough, that is set by the bit count, as in 256 for an 8
    bit image.  
    If the image is smaller it is set by width * height.
    Note, this may be called by WMF code, so it is not safe to assume the data is aligned.
*/
int get_real_color_count(
       const char *Bmih
   ){
   int Colors, BitCount, Width, Height;
   uint32_t  utmp4;
   uint16_t  utmp2;
   int32_t   tmp4;
   char     *cBmih = (char *) Bmih;
   memcpy(&utmp4, cBmih + offsetof(U_BITMAPINFOHEADER,biClrUsed),  4);  Colors   = utmp4;   
   memcpy(&utmp2, cBmih + offsetof(U_BITMAPINFOHEADER,biBitCount), 2);  BitCount = utmp2;   
   memcpy(&tmp4,  cBmih + offsetof(U_BITMAPINFOHEADER,biWidth),    4);  Width    = tmp4;   
   memcpy(&tmp4,  cBmih + offsetof(U_BITMAPINFOHEADER,biHeight),   4);  Height   = tmp4;   
   return(get_real_color_icount(Colors, BitCount, Width, Height));
}

/**
    \brief Get the actual number of colors in the color table from the ClrUsed, BitCount, Width, and Height.  
    \return Number of entries in the color table.
    \param Colors        Number of colors in the table.
    \param BitCount      BitCount Enumeration
    \param Width         bitmap width
    \param Height        bitmap height
*/
int get_real_color_icount(
       int Colors,
       int BitCount,
       int Width,
       int Height
   ){
   int area = Width * Height;
   if(area < 0){ area = -area; } /* Height might be negative */
   if(Colors == 0){
         if(     BitCount == U_BCBM_MONOCHROME){ Colors = 2;   }                                                                                          
         else if(BitCount == U_BCBM_COLOR4    ){ Colors = 16;  }                                                                                          
         else if(BitCount == U_BCBM_COLOR8    ){ Colors = 256; } 
         if(Colors > area){  Colors = area; }
   }
   return(Colors);
}


/**
    \brief Get the DIB parameters from the BMI of the record for use by DBI_to_RGBA()
    
    \return BI_Compression Enumeration.  For anything other than U_BI_RGB values other than px may not be valid.
    \param record      pointer to EMR record that has a U_BITMAPINFO and bitmap
    \param offBitsSrc  Offset to the bitmap
    \param offBmiSrc   Offset to the U_BITMAPINFO
    \param px          pointer to DIB pixel array in pEmr
    \param ct          pointer to DIB color table in pEmr
    \param numCt       DIB color table number of entries, for PNG or JPG returns the number of bytes in the image
    \param width       Width of pixel array
    \param height      Height of pixel array (always returned as a positive number)
    \param colortype   DIB BitCount Enumeration
    \param invert      If DIB rows are in opposite order from RGBA rows
*/
int get_DIB_params(
       const char       *record,
       uint32_t          offBitsSrc,
       uint32_t          offBmiSrc,
       const char      **px,
       const U_RGBQUAD **ct,
       uint32_t         *numCt,
       uint32_t         *width,
       uint32_t         *height,
       uint32_t         *colortype,
       uint32_t         *invert
   ){
   uint32_t bic;
   PU_BITMAPINFO Bmi = (PU_BITMAPINFO)(record + offBmiSrc);
   PU_BITMAPINFOHEADER Bmih = &(Bmi->bmiHeader);
   /* if biCompression is not U_BI_RGB some or all of the following might not hold real values */
   bic        = Bmih->biCompression;
   *width     = Bmih->biWidth;
   *colortype = Bmih->biBitCount;
   if(Bmih->biHeight < 0){
      *height = -Bmih->biHeight;
      *invert = 1;
   }
   else {
      *height = Bmih->biHeight;
      *invert = 0;
   }
   if(bic == U_BI_RGB){
      *numCt     = get_real_color_count((const char *) Bmih);
      if( numCt){ *ct = (PU_RGBQUAD) ((char *)Bmi + sizeof(U_BITMAPINFOHEADER)); }
      else {      *ct = NULL;                                                    }                                                                                       
   }
   else if(bic == U_BI_BITFIELDS){ /* to date only encountered once, for 32 bit, from PPT*/
      *numCt     = 0;
      *ct        = NULL;
      bic        = U_BI_RGB;  /* there seems to be no difference, at least for the 32 bit images */
   }
   else {
      *numCt     = Bmih->biSizeImage;
      *ct        = NULL;
   }
   *px = record + offBitsSrc;
   return(bic);
}

/**
    \brief Convert one of many different types of DIB pixmaps to an RGBA 32 bit pixmap.
    
    \return 0 on success, other values on errors.
    \param px         DIB pixel array
    \param ct         DIB color table
    \param numCt      DIB color table number of entries
    \param rgba_px    U_RGBA pixel array (32 bits), created by this routine, caller must free.
    \param w          Width of pixel array in the record
    \param h          Height of pixel array in the record
    \param colortype  DIB BitCount Enumeration
    \param use_ct     Kept for symmetry with RGBA_to_DIB, should be set to numCt
    \param invert     If DIB rows are in opposite order from RGBA rows
*/
int DIB_to_RGBA(
       const char      *px,
       const U_RGBQUAD *ct,
       int              numCt,
       char           **rgba_px,
       int              w,
       int              h,
       uint32_t         colortype,
       int              use_ct,
       int              invert
   ){
   uint32_t     cbRgba_px;
   int          stride;
   int          bs;
   int          pad;
   int          i,j;
   int          istart, iend, iinc;
   uint8_t      r,g,b,a,tmp8;
   const char  *pxptr;
   char        *rptr;
   int          usedbytes;
   U_RGBQUAD    color;
   int32_t      index;
   
   // sanity checking
   if(!w || !h || !colortype || !px)return(1);
   if(use_ct && colortype  >= U_BCBM_COLOR16)return(2);  //color tables not used above 16 bit pixels
   if(!use_ct && colortype < U_BCBM_COLOR16)return(3);   //color tables mandatory for < 16 bit
   if(use_ct && !numCt)return(4);                        //color table not adequately described

   stride    = w * 4;
   cbRgba_px = stride * h;
   bs = colortype/8;
   if(bs<1){
      usedbytes = (w*colortype + 7)/8;      // width of line in fully and partially occupied bytes
   }
   else {
      usedbytes = w*bs;
   }
   pad = UP4(usedbytes) - usedbytes;        // DIB rows must be aligned on 4 byte boundaries, they are padded at the end to accomplish this.;
   *rgba_px = (char *) malloc(cbRgba_px);
   if(!rgba_px)return(4);
   
   if(invert){
     istart = h-1;
     iend   = -1;
     iinc   = -1;
   }
   else {
     istart = 0;
     iend   = h;
     iinc   = 1;
   }

   pxptr = px;
   tmp8  = 0;  // silences a compiler warning, tmp8 always sets when j=0, so never used uninitialized
   for(i=istart; i!=iend; i+=iinc){
      rptr= *rgba_px + i*stride;
      for(j=0; j<w; j++){
          if(use_ct){
             switch(colortype){
                case U_BCBM_MONOCHROME: // 2 colors.    bmiColors array has two entries
                    if(!(j % 8)){ tmp8 = *pxptr++; }
                    index = 0x80 & tmp8;      // This seems wrong, as lowest position is top bit, but it works.
                    index = index >> 7;
                    tmp8 = tmp8 << 1; 
                    break;           
                case U_BCBM_COLOR4:     // 2^4 colors.  bmiColors array has 16 entries                 
                    if(!(j % 2)){  tmp8 = *pxptr++; }
                    index = 0xF0 & tmp8;
                    index = index >> 4;
                    tmp8  = tmp8  << 4;
                    break;           
                case U_BCBM_COLOR8:     // 2^8 colors.  bmiColors array has 256 entries 
                    index    = (uint8_t) *pxptr++;;
                    break;           
                case U_BCBM_COLOR16:    // 2^16 colors. (Several different color methods))
                case U_BCBM_COLOR24:    // 2^24 colors. bmiColors is not used. Pixels are U_RGBTRIPLE.
                case U_BCBM_COLOR32:    // 2^32 colors. bmiColors is not used. Pixels are U_RGBQUAD.
                case U_BCBM_EXPLICIT:   // Derinved from JPG or PNG compressed image or ?   
                default:
                    return(7);            // This should not be possible, but might happen with memory corruption  
             }
             color = ct[index];
             b = U_BGRAGetB(color);
             g = U_BGRAGetG(color);
             r = U_BGRAGetR(color);
             a = U_BGRAGetA(color);
          }
          else {
             switch(colortype){
                case U_BCBM_COLOR16:    // 2^16 colors. (Several different color methods)) 
                   // Do it in this way because the bytes are always stored Little Endian
                   tmp8  = *pxptr++;
                   b = (0x1F & tmp8) <<3;       // 5 bits of b into the top 5 of 8
                   g = tmp8 >> 5;               //  least significant 3 bits of green
                   tmp8  = *pxptr++;
                   r = (0x7C & tmp8) << 1;      // 5 bits of r into the top 5 of 8
                   g |= (0x3 & tmp8) << 3;      // most  significant 2 bits of green (there are only 5 bits of data)
                   g = g << 3;                  //  restore intensity (have lost 3 bits of accuracy)
                   a = 0;
                   break;           
                case U_BCBM_COLOR24:    // 2^24 colors. bmiColors is not used. Pixels are U_RGBTRIPLE. 
                   b = *pxptr++;
                   g = *pxptr++;
                   r = *pxptr++;
                   a = 0;
                   break;          
                case U_BCBM_COLOR32:    // 2^32 colors. bmiColors is not used. Pixels are U_RGBQUAD.
                   b = *pxptr++;
                   g = *pxptr++;
                   r = *pxptr++;
                   a = *pxptr++;
                   break;           
                case U_BCBM_MONOCHROME: // 2 colors.    bmiColors array has two entries                
                case U_BCBM_COLOR4:     // 2^4 colors.  bmiColors array has 16 entries                 
                case U_BCBM_COLOR8:     // 2^8 colors.  bmiColors array has 256 entries                
                case U_BCBM_EXPLICIT:   // Derinved from JPG or PNG compressed image or ?   
                default:
                  return(7);            // This should not be possible, but might happen with memory corruption  
             }
          }
          *rptr++ = r;
          *rptr++ = g;
          *rptr++ = b;
          *rptr++ = a;
      }
      for(j=0; j<pad; j++){ pxptr++; }  // DIB rows are all 4 byte aligned
   } 
   return(0);
}

/**
    \brief Extract a subset of an RGBA bitmap array.
    Frees the incoming bitmap array IF a subset is extracted, otherwise it is left alone.
    If the entire array is extracted it just returns the incoming pointer.
    If the subset requested is partially outside of the bitmap the region is clipped to the
      bitmap boundaries and extracted.  This seems to be a (very) grey area in EMF files, and
      even different Microsoft applications do not always do the same thing.  For instance,
      XP Preview gives some different images for EMR_BITBLT records than does the "import image"
      (but not unpacked) view in PowerPoint. Since all of these states are probably best viewed
      as undefined or errors we can only try to do something reasonable and not blow up when
      encountering one.
    
    \return Pointer to the sub array on success, NULL otherwise.
    \param rgba_px    U_RGBA pixel array (32 bits), created by this routine, caller must free.
    \param w          Width of pixel array in the record
    \param h          Height of pixel array in the record
    \param sl         start left position in the pixel array in the record to start extracting
    \param st         start top  position in the pixel array in the record to start extracting
    \param eew        Width of pixel array to extract
    \param eeh        Height of pixel array to extract
*/
char *RGBA_to_RGBA(
       char        *rgba_px,
       int          w,
       int          h,
       int          sl,
       int          st,
       int          *eew,
       int          *eeh
   ){
   int          i;
   char        *sub;
   char        *sptr;
   int          ew = *eew;
   int          eh = *eeh;
   
   // sanity checking
   if(w<=0 || h<=0 || ew<=0 || eh<=0 || !rgba_px)return(NULL);

   if(sl>w || st >h)return(NULL);  // This is hopeless, the start point is outside of the array.
   if(sl<0){
      if(sl+ew<=0)return(NULL);    // This is hopeless, the start point is outside of the array.
      ew += sl;
      sl = 0;
   }
   if(st<0){
      if(st+eh<=0)return(NULL);    // This is hopeless, the start point is outside of the array.
      eh += st;
      st = 0;
   }
   if(sl+ew > w)ew=w-sl;
   if(st+eh > h)eh=h-st;
   if(!sl && !st && (ew == w) && (eh == h)){
      sub = rgba_px;
   }
   else {
      sptr = sub = malloc(ew*eh*4);
      if(!sub)return(NULL);
      for(i=st; i<st+eh; i++){
         memcpy(sptr,rgba_px + i*w*4 + sl*4,4*ew);
         sptr += 4*ew;
      }
      free(rgba_px);
   }
   *eeh = eh;
   *eew = ew;
   return(sub);
 }


/* **********************************************************************************************
These functions are for setting up, appending to, and then tearing down an EMF structure, including
writing the final data structure out to a file.
*********************************************************************************************** */

/**
    \brief Duplicate an EMR record.
    \param emr record to duplicate
*/
char *emr_dup(
      const char *emr
   ){
   char *dup;
   int   irecsize;

   if(!emr)return(NULL);
   irecsize = ((PU_EMR)emr)->nSize;
   dup=malloc(irecsize);
   if(dup){ memcpy(dup,emr,irecsize); }
   return(dup);
}
    

/**
    \brief Start constructing an emf in memory. Supply the file name and initial size.
    \return 0 for success, >=0 for failure.
    \param name  EMF filename (will be opened)
    \param initsize Initialize EMF in memory to hold this many bytes
    \param chunksize When needed increase EMF in memory by this number of bytes
    \param et EMF in memory
    
    
*/
int  emf_start(
      const char       *name,
      const uint32_t   initsize,
      const uint32_t   chunksize,
      EMFTRACK       **et
   ){
   FILE *fp;
   EMFTRACK *etl=NULL;

   if(initsize < 1)return(1);
   if(chunksize < 1)return(2);
   if(!name)return(3);
   etl = (EMFTRACK *) malloc(sizeof(EMFTRACK));
   if(!etl)return(4);
   etl->buf = malloc(initsize);  // no need to zero the memory
   if(!etl->buf){
      free(etl);
      return(5);
   }
   fp=emf_fopen(name,U_WRITE);
   if(!fp){
      free(etl->buf);
      free(etl);
      return(6);
   }
   etl->fp         =  fp;    
   etl->allocated  =  initsize;
   etl->used       =  0;
   etl->records    =  0;
   etl->PalEntries =  0;
   etl->chunk      =  chunksize;
   *et=etl;
   return(0);
}

/**
    \brief  Finalize the emf in memory and write it to the file.
    \return 0 on success, >=1 on failure
    \param et EMF in memory
    \param eht  EMF handle table (peak handle number needed)
*/
int  emf_finish(
      EMFTRACK   *et,
      EMFHANDLES *eht
   ){
   U_EMRHEADER *record;

   if(!et->fp)return(1);   // This could happen if something stomps on memory, otherwise should be caught in emf_start

   // Set the header fields which were unknown up until this point
  
   record = (U_EMRHEADER *)et->buf;
   record->nBytes       = et->used;
   record->nRecords     = et->records;
   record->nHandles     = eht->peak + 1;
   record->nPalEntries  = et->PalEntries;
  
#if U_BYTE_SWAP
    //This is a Big Endian machine, EMF data must be  Little Endian
    U_emf_endian(et->buf,et->used,1); 
#endif

   if(1 != fwrite(et->buf,et->used,1,et->fp))return(2);
   (void) fclose(et->fp);
   et->fp=NULL;
   return(0);
}

/**
    \brief Release memory for an emf structure in memory. Call this after emf_finish().
    \return 0 on success, >=1 on failure
    \param et EMF in memory
*/
int emf_free(
      EMFTRACK **et
   ){    
   EMFTRACK *etl;
   if(!et)return(1);
   etl=*et;
   if(!etl)return(2);
   free(etl->buf);
   free(etl);
   *et=NULL;
   return(0);
}

/**
    \brief wrapper for fopen, works on any platform
    \return 0 on success, >=1 on failure
    \param filename file to open (either ASCII or UTF-8)
    \param mode     U_READ or U_WRITE (these map to  "rb" and "wb")
*/
FILE *emf_fopen(
      const char *filename,
      const int mode
   ){    
   FILE *fp = NULL;
#ifdef WIN32
   uint16_t *fn16;
   uint16_t *md16;
   if(mode == U_READ){ md16 = U_Utf8ToUtf16le("rb", 0, NULL); }
   else {              md16 = U_Utf8ToUtf16le("wb", 0, NULL); }
   fn16 = U_Utf8ToUtf16le(filename, 0, NULL);
   fp = _wfopen(fn16,md16);
   free(fn16);
   free(md16);
#else
   if(mode == U_READ){ fp = fopen(filename,"rb"); }
   else {              fp = fopen(filename,"wb"); }
#endif
   return(fp);
}

/**
    \brief Retrieve contents of an EMF file by name.
    \return 0 on success, >=1 on failure
    \param filename Name of file to open, including the path
    \param contents Contents of the file.  Buffer must be free()'d by caller.
    \param length   Number of bytes in Contents
*/
int emf_readdata(
      const char   *filename,
      char        **contents,
      size_t       *length
   ){    
   FILE     *fp;
   int       status=0;

   *contents=NULL;
   fp=emf_fopen(filename,U_READ);
   if(!fp){ status = 1; }
   else {
      // read the entire file into memory
      fseek(fp, 0, SEEK_END); // move to end
      *length = ftell(fp);
      rewind(fp);
      *contents = (char *) malloc(*length);
      if(!*contents){ 
         status = 2; 
      }
      else {
         size_t inbytes = fread(*contents,*length,1,fp);
         if(inbytes != 1){
            free(*contents);
            status = 3;
         }
         else {
#if U_BYTE_SWAP
            //This is a Big Endian machine, EMF data is Little Endian
            U_emf_endian(*contents,*length,0);  // LE to BE
#endif
          }
      }
      fclose(fp);
   }
    return(status);
}


/**
    \brief Append an EMF record to an emf in memory. This may reallocate buf memory.
    \return 0 for success, >=1 for failure.
    \param rec     Record to append to EMF in memory
    \param et      EMF in memory
    \param freerec If true, free rec after append    
*/
int  emf_append(
      U_ENHMETARECORD *rec,
      EMFTRACK        *et,
      int              freerec
   ){
   size_t deficit;
   
#ifdef U_VALGRIND
   printf("\nbefore \n");
   printf(" probe %d\n",memprobe(rec, U_EMRSIZE(rec)));
   printf("after \n");
#endif
   if(!rec)return(1);
   if(!et)return(2);
   if(rec->nSize + et->used > et->allocated){
      deficit = rec->nSize + et->used - et->allocated;
      if(deficit < et->chunk)deficit = et->chunk;
      et->allocated += deficit;
      et->buf = realloc(et->buf,et->allocated);
      if(!et->buf)return(3);
   }
   memcpy(et->buf + et->used, rec, rec->nSize);
   et->used += rec->nSize;
   et->records++;
   if(rec->iType == U_EMR_EOF){ et->PalEntries = ((U_EMREOF *)rec)->cbPalEntries; }
   if(freerec){ free(rec); }
   return(0);
}

/**
    \brief Create a handle table. Entries filled with 0 are empty, entries >0 hold a handle.
    \return 0 for success, >=1 for failure.
    \param initsize Initialize with space for this number of handles
    \param chunksize When needed increase space by this number of handles
    \param eht EMF handle table    
*/
int emf_htable_create(
      uint32_t     initsize,
      uint32_t     chunksize,
      EMFHANDLES **eht
   ){
   EMFHANDLES *ehtl;
   unsigned int i;
   
   if(initsize<1)return(1);
   if(chunksize<1)return(2);
   ehtl = (EMFHANDLES *) malloc(sizeof(EMFHANDLES));
   if(!ehtl)return(3);
   ehtl->table = malloc(initsize * sizeof(uint32_t));
   if(!ehtl->table){
      free(ehtl);
      return(4);
   }
   ehtl->stack = malloc(initsize * sizeof(uint32_t));
   if(!ehtl->stack){
      free(ehtl->table);
      free(ehtl);
      return(5);
   }
   memset(ehtl->table , 0, initsize * sizeof(uint32_t));  // zero all slots in the table
   for(i=1; i<initsize; i++){ehtl->stack[i]=i;}             // preset the stack
   ehtl->allocated = initsize;
   ehtl->chunk     = chunksize;
   ehtl->table[0]  = 0;         // This slot isn't actually ever used
   ehtl->stack[0]  = 0;         // This stack position isn't actually ever used
   ehtl->peak      = 1;
   ehtl->sptr      = 1;
   ehtl->top       = 0;
   *eht            = ehtl;
   return(0);
}

/**
    \brief Delete an entry from the handle table. Move it back onto the stack. The specified slot is filled with a 0.
    \return 0 for success, >=1 for failure.
    \param ih  handle
    \param eht EMF handle table
    
*/
int emf_htable_delete(
      uint32_t    *ih,
      EMFHANDLES  *eht
   ){
   if(!eht)return(1);
   if(!eht->table)return(2);
   if(!eht->stack)return(3);
   if(*ih < 1)return(4);           // invalid handle
   if(!eht->table[*ih])return(5);  // requested table position was not in use
   eht->table[*ih]=0;              // remove handle from table
   while(eht->top>0 && !eht->table[eht->top]){  // adjust top
     eht->top--;
   }
   eht->sptr--;                   // adjust stack
   eht->stack[eht->sptr]=*ih;      // place handle on stack
   *ih=0;                         // invalidate handle variable, so a second delete will of it is not possible
   return(0);
}

/**
    \brief Returns the index of the first free slot.  
    Call realloc() if needed.  The slot is set to handle (indicates occupied) and the peak value is adjusted.
    \return 0 for success, >=1 for failure.
    \param ih  handle
    \param eht EMF handle table
*/
int emf_htable_insert(
      uint32_t   *ih,
      EMFHANDLES *eht
   ){
   unsigned int i;
   size_t newsize;

   if(!eht)return(1);
   if(!eht->table)return(2);
   if(!eht->stack)return(3);
   if(!ih)return(4);
   if(eht->sptr >= eht->allocated - 1){  // need to reallocate
     newsize=eht->allocated + eht->chunk;
     eht->table = realloc(eht->table,newsize * sizeof(uint32_t));
     if(!eht->table)return(5);
     memset(&eht->table[eht->allocated] , 0, eht->chunk * sizeof(uint32_t));  // zero all NEW slots in the table
 
     eht->stack = realloc(eht->stack,newsize * sizeof(uint32_t));
     if(!eht->stack)return(6);
     for(i=eht->allocated; i<newsize;i++){ eht->stack[i] = i; }  // init all NEW slots in the stack
     eht->allocated = newsize;
   }
   *ih = eht->stack[eht->sptr];     // handle that is inserted
   if(eht->table[*ih])return(7);
   eht->table[*ih] = *ih;           // handle goes into preexisting (but zero) slot in table
   eht->stack[eht->sptr] = 0;
   if(*ih > eht->top){        eht->top = *ih;  }
   if(eht->sptr > eht->peak){ eht->peak = eht->sptr; }
   eht->sptr++;         // next available handle
   return(0);
}

/**
    \brief Free all memory in an htable.  Sets the pointer to NULL.
    \return 0 for success, >=1 for failure.
    \param eht  EMF handle table
*/
int emf_htable_free(
      EMFHANDLES **eht
   ){
   EMFHANDLES *ehtl;
   if(!eht)return(1);
   ehtl = *eht;
   if(!ehtl)return(2);
   if(!ehtl->table)return(3);
   if(!ehtl->stack)return(4);
   free(ehtl->table);
   free(ehtl->stack);
   free(ehtl);
   *eht=NULL;
   return(0);
}

/* **********************************************************************************************
These functions create standard structures used in the EMR records.
*********************************************************************************************** */


/**
    \brief Set up fields for an EMR_HEADER from the physical device's width and height in mm and dots per millimeter.
    Typically this is something like 216,279,47.244 (Letter paper, 1200 DPI = 47.244 DPmm)
    \return 0 for success, >=1 for failure.
    \param xmm     Device  width in millimeters
    \param ymm     Device height in millimeters
    \param dpmm    Dots per millimeter
    \param szlDev  Device size structure in pixels
    \param szlMm   Device size structure in mm
*/
int device_size(
      const int    xmm,
      const int    ymm,
      const float  dpmm,
      U_SIZEL     *szlDev,
      U_SIZEL     *szlMm
   ){
   if(xmm < 0 || ymm < 0 || dpmm < 0)return(1);
   szlDev->cx          =  U_ROUND((float) xmm * dpmm);
   szlDev->cy          =  U_ROUND((float) ymm * dpmm);;
   szlMm->cx           =  xmm;
   szlMm->cy           =  ymm;
   return(0);
}

/**
    \brief Set up fields for an EMR_HEADER for drawing by physical size in mm and dots per millimeter.
    Technically rclBounds is supposed to be the extent of the drawing within the EMF, but libUEMF has no way
    of knowing this since it never actually draws anything.  Instead this is set to the full drawing size.
    Coordinates are inclusive inclusive, so 297 -> 0,29699.
    \return 0 for success, >=1 for failure.
    \param xmm        Drawing  width in millimeters
    \param ymm        Drawing height in millimeters
    \param dpmm       Dots per millimeter
    \param rclBounds  Drawing size structure in pixels
    \param rclFrame   Drawing size structure in mm
*/
int drawing_size(
      const int    xmm,
      const int    ymm,
      const float  dpmm,
      U_RECTL     *rclBounds,
      U_RECTL     *rclFrame
   ){
   if(xmm < 0 || ymm < 0 || dpmm < 0)return(1);
   rclBounds->left     =  0;
   rclBounds->top      =  0;
   rclBounds->right    =  U_ROUND((float) xmm  * dpmm) - 1;  // because coordinate system is 0,0 in upper left, N,M in lower right
   rclBounds->bottom   =  U_ROUND((float) ymm  * dpmm) - 1;
   rclFrame->left      =  0;       
   rclFrame->top       =  0; 
   rclFrame->right     =  U_ROUND((float) xmm * 100.) - 1; 
   rclFrame->bottom    =  U_ROUND((float) ymm * 100.) - 1;         
   return(0);
}

/** 
    \brief Set a U_COLORREF value from separate R,G,B values.
    \param red    Red   component
    \param green  Green component
    \param blue   Blue  component
    
*/
U_COLORREF colorref3_set(
      uint8_t red,
      uint8_t green,
      uint8_t blue
   ){
   U_COLORREF cr = (U_COLORREF){red , green, blue, 0};
   return(cr);
}

/** 
    \brief Set a U_COLORREF value from separate R,G,B, and Reserved values.
    \param red        Red      component
    \param green      Green    component
    \param blue       Blue     component
    \param Reserved   Reserved component
    
*/
U_COLORREF colorref4_set(
      uint8_t red,
      uint8_t green,
      uint8_t blue,
      uint8_t Reserved
   ){
   U_COLORREF cr = (U_COLORREF){red , green, blue, Reserved};
   return(cr);
}

/** 
    \brief Set a U_RGBQUAD value from separate R,G,B, Reserved values.
    \param red       Red      component
    \param green     Green    component
    \param blue      Blue     component
    \param reserved  Reserved component
    
*/
U_RGBQUAD rgbquad_set(
      uint8_t red,
      uint8_t green,
      uint8_t blue,
      uint8_t reserved
   ){
   U_RGBQUAD cr = (U_RGBQUAD){blue , green, red, reserved};
   return(cr);
}

/**
    \brief Set rect and rectl objects from Upper Left and Lower Right corner points.
    \param ul upper left corner of rectangle
    \param lr lower right corner of rectangle
*/
U_RECTL rectl_set(
      U_POINTL ul,
      U_POINTL lr
    ){
    U_RECTL rct;
    rct.left     =   ul.x;
    rct.top      =   ul.y;
    rct.right    =   lr.x;
    rct.bottom   =   lr.y;
    return(rct);
}

/**
    \brief Set rect and rectl objects from Upper Left and Lower Right corner points.
    \param array array of rectangles
    \param index  array entry to fill, numbered from 0
    \param ul upper left corner of rectangle
    \param lr lower right corner of rectangle
*/
void rectli_set(
      PU_RECTL array,
      int index,
      U_POINTL ul,
      U_POINTL lr
    ){
    PU_RECTL rct = &(array[index]);
    rct->left     =   ul.x;
    rct->top      =   ul.y;
    rct->right    =   lr.x;
    rct->bottom   =   lr.y;
}

/**
    \brief Set sizel objects with X,Y values.
    \param x X coordinate 
    \param y Y coordinate
*/
U_SIZEL sizel_set(
       int32_t  x,
       int32_t  y
    ){
    U_SIZEL sz;
    sz.cx = x;
    sz.cy = y;
    return(sz);
} 

/**
    \brief Set pointl objects with X,Y values.
    \param x X coordinate 
    \param y Y coordinate
*/
U_POINTL point32_set(
       int32_t  x,   
       int32_t  y
    ){
    U_POINTL pt;
    pt.x = x;
    pt.y = y;
    return(pt);
} 

/**
    \brief Set point16 objects with 16 bit X,Y values.
    \param x X coordinate 
    \param y Y coordinate
*/
U_POINT16 point16_set(
       int16_t  x,
       int16_t  y
    ){
    U_POINT16 pt;
    pt.x = x;
    pt.y = y;
    return(pt);
} 

/**
    \brief Find the bounding rectangle from a polyline of a given width.
    \param count  number of points in the polyline
    \param pts    the polyline
    \param width  width of drawn line
    
*/
U_RECT findbounds(
      uint32_t count,
      PU_POINT pts,
      uint32_t width
   ){
   U_RECT rect={INT32_MAX, INT32_MAX, INT32_MIN, INT32_MIN };
   unsigned int i;

   for(i=0; i<count;i++,pts++){
       if ( pts->x < rect.left )   rect.left   = pts->x;
       if ( pts->x > rect.right )  rect.right  = pts->x;
       if ( pts->y < rect.top )    rect.top    = pts->y;
       if ( pts->y > rect.bottom ) rect.bottom = pts->y;
   }
   if(width > 0){
     rect.left   -= width;
     rect.right  += width;
     rect.top    += width;
     rect.bottom -= width;
   }
   return(rect);
}

/**
    \brief Find the bounding rectangle from a polyline of a given width.
    \param count  number of points in the polyline
    \param pts    the polyline
    \param width  width of drawn line
    
*/
U_RECT findbounds16(
      uint32_t count,
      PU_POINT16 pts,
      uint32_t width
   ){
   U_RECT rect={INT16_MAX, INT16_MAX, INT16_MIN, INT16_MIN };
   unsigned int i;

   for(i=0; i<count;i++,pts++){
       if ( pts->x < rect.left )   rect.left   = pts->x;
       if ( pts->x > rect.right )  rect.right  = pts->x;
       if ( pts->y < rect.top )    rect.top    = pts->y;
       if ( pts->y > rect.bottom ) rect.bottom = pts->y;
   }
   if(width > 0){
     rect.left   -= width;
     rect.right  += width;
     rect.top    += width;
     rect.bottom -= width;
   }
   return(rect);
}
/**
    \brief Construct a U_LOGBRUSH structure.
    \return U_LOGBRUSH structure
    \param lbStyle   LB_Style Enumeration
    \param lbColor   Brush color
    \param lbHatch   HatchStyle Enumertaion
*/
U_LOGBRUSH logbrush_set(
      uint32_t    lbStyle,
      U_COLORREF  lbColor,
      int32_t     lbHatch
   ){
   U_LOGBRUSH lb;
   lb.lbStyle = lbStyle;
   lb.lbColor = lbColor;
   lb.lbHatch = lbHatch;
   return(lb);
}

/**
    \brief Construct a U_XFORM structure.
    \return U_XFORM structure
    \param eM11 Rotation Matrix element
    \param eM12 Rotation Matrix element
    \param eM21 Rotation Matrix element
    \param eM22 Rotation Matrix element
    \param eDx  Translation element 
    \param eDy  Translation element 
*/
U_XFORM xform_set(
      U_FLOAT eM11,
      U_FLOAT eM12,
      U_FLOAT eM21,
      U_FLOAT eM22,
      U_FLOAT eDx,
      U_FLOAT eDy
   ){
   U_XFORM xform;
   xform.eM11 =  eM11;
   xform.eM12 =  eM12;
   xform.eM21 =  eM21;
   xform.eM22 =  eM22;
   xform.eDx  =  eDx;
   xform.eDy  =  eDy;
   return(xform);
}

/**
    \brief Construct a U_XFORM structure.
    \return U_XFORM structure
    \param scale     Scale factor
    \param ratio     Ratio of minor axis/major axis
    \param rot       Rotation angle in degrees, positive is counter clockwise from the x axis.
    \param axisrot   Angle in degrees defining the major axis before rotation, positive is counter clockwise from the x axis.
    \param eDx       Translation element 
    \param eDy       Translation element 
    
    Operation is:
      1  Conformal map of points based on scale, axis rotation, and axis ratio,
      2. Apply rotation
      3. Apply offset
*/
U_XFORM xform_alt_set(
      U_FLOAT scale,
      U_FLOAT ratio,
      U_FLOAT rot,
      U_FLOAT axisrot,
      U_FLOAT eDx,
      U_FLOAT eDy
   ){
   U_XFORM xform;
   U_MAT2X2 mat1, mat2;
   // angles are in degrees, must be in radians
   rot     *=  (2.0 * U_PI)/360.0;
   axisrot *= -(2.0 * U_PI)/360.0;
   mat1.M11 = cos(rot);  // set up the rotation matrix
   mat1.M12 = -sin(rot);
   mat1.M21 = sin(rot);
   mat1.M22 = cos(rot);
   if(ratio!=1.0){  // set scale/ellipticity matrix
      mat2.M11 =            scale*( cos(axisrot)*cos(axisrot) + ratio*sin(axisrot)*sin(axisrot) );
      mat2.M12 = mat2.M21 = scale*( sin(axisrot)*cos(axisrot) * (1.0 - ratio)           );
      mat2.M22 =            scale*( sin(axisrot)*sin(axisrot) + ratio*cos(axisrot)*cos(axisrot) );
   }
   else { // when the ratio is 1.0 then the major axis angle is ignored and only scale matters
      mat2.M11 = scale;
      mat2.M12 = 0.0;
      mat2.M21 = 0.0;
      mat2.M22 = scale;
   }
   xform.eM11 =  mat2.M11 * mat1.M11 + mat2.M12 * mat1.M21;
   xform.eM12 =  mat2.M11 * mat1.M12 + mat2.M12 * mat1.M22;;
   xform.eM21 =  mat2.M21 * mat1.M11 + mat2.M22 * mat1.M21;
   xform.eM22 =  mat2.M21 * mat1.M12 + mat2.M22 * mat1.M22;
   xform.eDx  =  eDx;
   xform.eDy  =  eDy;
   return(xform);
}


/**
    \brief Construct a U_LOGCOLORSPACEA structure.
    \return U_LOGCOLORSPACEA structure
    \param lcsCSType     LCS_CSType Enumeration
    \param lcsIntent     LCS_Intent Enumeration
    \param lcsEndpoints  CIE XYZ color space endpoints
    \param lcsGammaRGB   Gamma For RGB
    \param lcsFilename   Could name an external color profile file, otherwise empty string
*/
U_LOGCOLORSPACEA logcolorspacea_set(
      int32_t             lcsCSType,
      int32_t             lcsIntent,
      U_CIEXYZTRIPLE      lcsEndpoints,
      U_LCS_GAMMARGB      lcsGammaRGB,
      char                *lcsFilename
   ){
   U_LOGCOLORSPACEA lcsa;
   lcsa.lcsSignature  =    U_LCS_SIGNATURE;
   lcsa.lcsVersion    =    U_LCS_SIGNATURE;  
   lcsa.lcsSize       =    sizeof(U_LOGCOLORSPACEA);     
   lcsa.lcsCSType     =    lcsCSType;   
   lcsa.lcsIntent     =    lcsIntent;   
   lcsa.lcsEndpoints  =    lcsEndpoints;
   lcsa.lcsGammaRGB   =    lcsGammaRGB; 
   strncpy(lcsa.lcsFilename,lcsFilename,U_MAX_PATH);
   lcsa.lcsFilename[U_MAX_PATH-1] = '\0';
   return(lcsa);
}

/**

    \brief Construct a U_LOGCOLORSPACEW structure.
    \return U_LOGCOLORSPACEW structure
    \param lcsCSType     LCS_CSType Enumeration                                               
    \param lcsIntent     LCS_Intent Enumeration                                               
    \param lcsEndpoints  CIE XYZ color space endpoints                                        
    \param lcsGammaRGB   Gamma For RGB                                                        
    \param lcsFilename   Could name an external color profile file, otherwise empty string    
*/
U_LOGCOLORSPACEW logcolorspacew_set(
    int32_t             lcsCSType,
    int32_t             lcsIntent,
    U_CIEXYZTRIPLE      lcsEndpoints,
    U_LCS_GAMMARGB      lcsGammaRGB,
    uint16_t            *lcsFilename
    ){
    U_LOGCOLORSPACEW lcsa;
    lcsa.lcsSignature  =    U_LCS_SIGNATURE;
    lcsa.lcsVersion    =    U_LCS_SIGNATURE;  
    lcsa.lcsSize       =    sizeof(U_LOGCOLORSPACEW);     
    lcsa.lcsCSType     =    lcsCSType;   
    lcsa.lcsIntent     =    lcsIntent;   
    lcsa.lcsEndpoints  =    lcsEndpoints;
    lcsa.lcsGammaRGB   =    lcsGammaRGB; 
    wchar16strncpypad(lcsa.lcsFilename,lcsFilename,U_MAX_PATH);
    lcsa.lcsFilename[U_MAX_PATH-1] = '\0';
    return(lcsa);
}

/**

    \brief Construct a U_PANOSE structure.
    \return U_PANOSE structure
    \param bFamilyType      FamilyType Enumeration
    \param bSerifStyle      SerifType Enumeration
    \param bWeight          Weight Enumeration
    \param bProportion      Proportion Enumeration
    \param bContrast        Contrast Enumeration
    \param bStrokeVariation StrokeVariation Enumeration
    \param bArmStyle        ArmStyle Enumeration
    \param bLetterform      Letterform Enumeration
    \param bMidline         Midline Enumeration
    \param bXHeight         XHeight Enumeration
*/
U_PANOSE panose_set(
      uint8_t bFamilyType,
      uint8_t bSerifStyle,
      uint8_t bWeight,
      uint8_t bProportion,
      uint8_t bContrast,
      uint8_t bStrokeVariation,
      uint8_t bArmStyle,
      uint8_t bLetterform,
      uint8_t bMidline,
      uint8_t bXHeight
    ){
    U_PANOSE panose;
    panose.bFamilyType       = bFamilyType;      
    panose.bSerifStyle       = bSerifStyle;      
    panose.bWeight           = bWeight;          
    panose.bProportion       = bProportion;      
    panose.bContrast         = bContrast;        
    panose.bStrokeVariation  = bStrokeVariation; 
    panose.bArmStyle         = bArmStyle;        
    panose.bLetterform       = bLetterform;      
    panose.bMidline          = bMidline;         
    panose.bXHeight          = bXHeight;
    return(panose);        
}

/**
    \brief Construct a U_LOGFONT structure.
    \return U_LOGFONT structure
    \param lfHeight         Height in Logical units
    \param lfWidth          Average Width in Logical units
    \param lfEscapement     Angle in 0.1 degrees betweem escapement vector and X axis
    \param lfOrientation    Angle in 0.1 degrees between baseline and X axis
    \param lfWeight         LF_Weight Enumeration
    \param lfItalic         Italics:   0 or 1
    \param lfUnderline      Underline: 0 or 1
    \param lfStrikeOut      Strikeout: 0 or 1
    \param lfCharSet        LF_CharSet Enumeration
    \param lfOutPrecision   LF_OutPrecision Enumeration
    \param lfClipPrecision  LF_ClipPrecision Enumeration
    \param lfQuality        LF_Quality Enumeration
    \param lfPitchAndFamily LF_PitchAndFamily Enumeration
    \param lfFaceName       Name of font. truncates at U_LF_FACESIZE, smaller must be null terminated

*/
U_LOGFONT logfont_set(
    int32_t    lfHeight,
    int32_t    lfWidth,
    int32_t    lfEscapement,
    int32_t    lfOrientation,
    int32_t    lfWeight,
    uint8_t    lfItalic,
    uint8_t    lfUnderline,
    uint8_t    lfStrikeOut,
    uint8_t    lfCharSet,
    uint8_t    lfOutPrecision,
    uint8_t    lfClipPrecision,
    uint8_t    lfQuality,
    uint8_t    lfPitchAndFamily,
    uint16_t  *lfFaceName
    ){
    U_LOGFONT lf;
    lf.lfHeight                  = lfHeight;          
    lf.lfWidth                   = lfWidth;           
    lf.lfEscapement              = lfEscapement;      
    lf.lfOrientation             = lfOrientation;     
    lf.lfWeight                  = lfWeight;          
    lf.lfItalic                  = lfItalic;          
    lf.lfUnderline               = lfUnderline;       
    lf.lfStrikeOut               = lfStrikeOut;       
    lf.lfCharSet                 = lfCharSet;         
    lf.lfOutPrecision            = lfOutPrecision;    
    lf.lfClipPrecision           = lfClipPrecision;   
    lf.lfQuality                 = lfQuality;         
    lf.lfPitchAndFamily          = lfPitchAndFamily;  
    wchar16strncpypad(lf.lfFaceName, lfFaceName, U_LF_FACESIZE); // pad this one as the intial structure was not set to zero
    lf.lfFaceName[U_LF_FACESIZE-1] = '\0';
    return(lf);
}


/**
    \brief Construct a U_LOGFONT_PANOSE structure.
    \return U_LOGFONT_PANOSE structure
    \param elfLogFont    Basic font attributes
    \param elfFullName   Font full name, truncates at U_LF_FULLFACESIZE, smaller must be null terminated
    \param elfStyle      Font style, truncates at U_LF_FULLFACESIZE, smaller must be null terminated
    \param elfStyleSize  Font hinting starting at this point size, if 0, starts at Height
    \param elfPanose     Panose Object. If all zero, it is ignored.
*/
U_LOGFONT_PANOSE logfont_panose_set(
      U_LOGFONT   elfLogFont,
      uint16_t   *elfFullName,
      uint16_t   *elfStyle,
      uint32_t    elfStyleSize,
      U_PANOSE    elfPanose
    ){
   U_LOGFONT_PANOSE lfp;
   memset(&lfp,0,sizeof(U_LOGFONT_PANOSE)); // all fields zero unless needed.  Many should be ignored or must be 0.
   wchar16strncpy(lfp.elfFullName, elfFullName, U_LF_FULLFACESIZE);
   lfp.elfFullName[U_LF_FULLFACESIZE-1] = '\0';
   wchar16strncpy(lfp.elfStyle,    elfStyle,    U_LF_FACESIZE);
   lfp.elfStyle[U_LF_FACESIZE-1] = '\0';
   lfp.elfLogFont    =   elfLogFont;
   lfp.elfStyleSize  =   elfStyleSize;
   lfp.elfPanose     =   elfPanose;
   return(lfp);
}

/**
    \brief Construct a U_BITMAPINFOHEADER structure.
    \return U_BITMAPINFOHEADER structure
    \param biWidth         Bitmap width in pixels
    \param biHeight        Bitmap height in pixels
    \param biPlanes        Planes (must be 1)
    \param biBitCount      BitCount Enumeration
    \param biCompression   BI_Compression Enumeration
    \param biSizeImage     Size in bytes of image
    \param biXPelsPerMeter X Resolution in pixels/meter
    \param biYPelsPerMeter Y Resolution in pixels/meter
    \param biClrUsed       Number of bmciColors in U_BITMAPCOREINFO
    \param biClrImportant  Number of bmciColors needed (0 means all). 
*/
U_BITMAPINFOHEADER bitmapinfoheader_set(
      int32_t       biWidth,
      int32_t       biHeight,
      uint16_t      biPlanes,
      uint16_t      biBitCount,
      uint32_t      biCompression,
      uint32_t      biSizeImage,
      int32_t       biXPelsPerMeter,
      int32_t       biYPelsPerMeter,
      U_NUM_RGBQUAD biClrUsed,
      uint32_t      biClrImportant
    ){
    U_BITMAPINFOHEADER Bmi;
    Bmi.biSize          = sizeof(U_BITMAPINFOHEADER);         
    Bmi.biWidth         = biWidth;        
    Bmi.biHeight        = biHeight;       
    Bmi.biPlanes        = biPlanes;       
    Bmi.biBitCount      = biBitCount;     
    Bmi.biCompression   = biCompression;  
    Bmi.biSizeImage     = biSizeImage;    
    Bmi.biXPelsPerMeter = biXPelsPerMeter;
    Bmi.biYPelsPerMeter = biYPelsPerMeter;
    Bmi.biClrUsed       = biClrUsed;      
    Bmi.biClrImportant  = biClrImportant;
    return(Bmi); 
}


/**
    \brief Allocate and construct a U_BITMAPINFO structure.
    \return Pointer to a U_BITMAPINFO structure
    \param BmiHeader   Geometry and pixel properties
    \param BmiColors   Color table (must be NULL for some  values of BmiHeader->biBitCount)
*/
PU_BITMAPINFO bitmapinfo_set(
      U_BITMAPINFOHEADER  BmiHeader,
      PU_RGBQUAD          BmiColors
   ){
   char *record;
   int   irecsize;
   int   cbColors, cbColors4, off;

   cbColors  = 4*get_real_color_count((char *) &BmiHeader);
   cbColors4 = UP4(cbColors);
   irecsize  = sizeof(U_BITMAPINFOHEADER) + cbColors4;
   record    = malloc(irecsize);
   if(record){
     memcpy(record, &BmiHeader, sizeof(U_BITMAPINFOHEADER));
     if(cbColors){
        off = sizeof(U_BITMAPINFOHEADER);
        memcpy(record + off, BmiColors, cbColors);
        off += cbColors;
        if(cbColors4 - cbColors){  memset(record + off, 0, cbColors4 - cbColors); }
     }
   }
   return((PU_BITMAPINFO) record);
}

/**
    \brief Allocate and construct a U_EXTLOGPEN structure.
    \return pointer to U_EXTLOGPEN structure, or NULL on error
    \param elpPenStyle   PenStyle Enumeration
    \param elpWidth      Width in logical units (elpPenStyle & U_PS_GEOMETRIC) or 1 (pixel)
    \param elpBrushStyle LB_Style Enumeration
    \param elpColor      Pen color
    \param elpHatch      HatchStyle Enumeration
    \param elpNumEntries Count of StyleEntry array
    \param elpStyleEntry Array of StyleEntry (For user specified dot/dash patterns)
*/
PU_EXTLOGPEN extlogpen_set(
      uint32_t            elpPenStyle,
      uint32_t            elpWidth,
      uint32_t            elpBrushStyle,
      U_COLORREF          elpColor,
      int32_t             elpHatch,
      U_NUM_STYLEENTRY    elpNumEntries,
      U_STYLEENTRY       *elpStyleEntry
   ){
   int irecsize,szSyleArray;
   char *record;
   
   if(elpNumEntries){
     if(!elpStyleEntry)return(NULL);
     szSyleArray   = elpNumEntries * sizeof(U_STYLEENTRY);
     irecsize = sizeof(U_EXTLOGPEN) + szSyleArray - sizeof(U_STYLEENTRY); // first one is in the record
   }
   else {
     szSyleArray   = 0;
     irecsize = sizeof(U_EXTLOGPEN);
   }
   record   = malloc(irecsize);
   if(record){
     ((PU_EXTLOGPEN) record)->elpPenStyle    = elpPenStyle;                             
     ((PU_EXTLOGPEN) record)->elpWidth       = elpWidth;                                
     ((PU_EXTLOGPEN) record)->elpBrushStyle  = elpBrushStyle;                           
     ((PU_EXTLOGPEN) record)->elpColor       = elpColor;                                
     ((PU_EXTLOGPEN) record)->elpHatch       = elpHatch;                                
     ((PU_EXTLOGPEN) record)->elpNumEntries  = elpNumEntries;                           
     if(elpNumEntries){ memcpy(((PU_EXTLOGPEN) record)->elpStyleEntry,elpStyleEntry,szSyleArray);   }
     else {             memset(((PU_EXTLOGPEN) record)->elpStyleEntry,0,sizeof(U_STYLEENTRY)); }  // not used, but this stops valgrind warnings
   }
   return((PU_EXTLOGPEN) record);
}

/**
    \brief Construct a U_LOGPEN structure.
    \return U_LOGPEN structure
    \param lopnStyle PenStyle Enumeration
    \param lopnWidth Width of pen set by X, Y is ignored
    \param lopnColor Pen color value
    
*/
U_LOGPEN logpen_set(
      uint32_t     lopnStyle,
      U_POINT      lopnWidth,
      U_COLORREF   lopnColor
   ){
   U_LOGPEN lp;
   lp.lopnStyle = lopnStyle;
   lp.lopnWidth = lopnWidth;
   lp.lopnColor = lopnColor;
   return(lp);
} 

/**
    \brief Construct a U_LOGPLTNTRY structure.
    \return U_LOGPLTNTRY structure
    \param peReserved Ignore
    \param peRed      Palette entry Red Intensity
    \param peGreen    Palette entry Green Intensity
    \param peBlue     Palette entry Blue Intensity
*/
U_LOGPLTNTRY logpltntry_set(
      uint8_t    peReserved,
      uint8_t    peRed,
      uint8_t    peGreen,
      uint8_t    peBlue
   ){
   U_LOGPLTNTRY lpny;
   lpny.peReserved = peReserved;
   lpny.peRed      = peRed;     
   lpny.peGreen    = peGreen;   
   lpny.peBlue     = peBlue;
   return(lpny);     
}

/**
    \brief Allocate and construct a U_LOGPALETTE structure.
    \return pointer to U_LOGPALETTE structure, or NULL on error.
    \param palNumEntries Number of U_LOGPLTNTRY objects
    \param palPalEntry   array, PC_Entry Enumeration
*/
PU_LOGPALETTE logpalette_set(
      U_NUM_LOGPLTNTRY    palNumEntries,
      PU_LOGPLTNTRY      *palPalEntry
   ){
   PU_LOGPALETTE record;
   int cbPalArray,irecsize;
   
   if(palNumEntries == 0 || !palPalEntry)return(NULL);
   cbPalArray = palNumEntries * sizeof(U_LOGPLTNTRY);
   irecsize = sizeof(U_LOGPALETTE) + cbPalArray - sizeof(U_LOGPLTNTRY);
   record = (PU_LOGPALETTE) malloc(irecsize);
   if(irecsize){
      record->palVersion    = U_LP_VERSION;
      record->palNumEntries = palNumEntries;
      memcpy(record->palPalEntry,palPalEntry,cbPalArray);
   }
   return(record);
}

/**
    \brief Construct a U_RGNDATAHEADER structure.
    \return U_RGNDATAHEADER structure
    \param nCount  Number of rectangles in region
    \param rclBounds Region bounds
*/
U_RGNDATAHEADER rgndataheader_set(
      U_NUM_RECTL         nCount,
      U_RECTL             rclBounds
   ){
   U_RGNDATAHEADER rdh;
   rdh.dwSize    = U_RDH_OBJSIZE;  
   rdh.iType     = U_RDH_RECTANGLES;   
   rdh.nCount    = nCount;  
   rdh.nRgnSize  = nCount * sizeof(U_RECTL); // Size in bytes of rectangle array
   rdh.rclBounds = rclBounds; 
   return(rdh);
}

/**
    \brief Allocate and construct a U_RGNDATA structure.
    \return pointer to U_RGNDATA structure, or NULL on error.
    \param rdh    Data description
    \param Buffer Array of U_RECTL elements
*/
PU_RGNDATA rgndata_set(
      U_RGNDATAHEADER     rdh,
      PU_RECTL            Buffer
   ){
   char *record;
   int irecsize;
   int szRgnArray,off;
   
   if(!Buffer || !rdh.nCount || !rdh.nRgnSize)return(NULL);
   szRgnArray = rdh.nRgnSize;             // size of the U_RECTL array
   irecsize = sizeof(U_RGNDATA) + szRgnArray - sizeof(U_RECTL);  // core + array - overlap
   record    = malloc(irecsize);
   if(record){
      memcpy(record, &rdh, sizeof(U_RGNDATAHEADER));
      off =  sizeof(U_RGNDATAHEADER);
      memcpy(record + off, Buffer, szRgnArray);   
   }
   return((PU_RGNDATA) record);
}

/**
    \brief Construct a U_COLORADJUSTMENT structure.
    \return U_COLORADJUSTMENT structure
    \param Size            Size of this structure in bytes                                      
    \param Flags           ColorAdjustment Enumeration                                          
    \param IlluminantIndex Illuminant Enumeration                                               
    \param RedGamma        Red   Gamma correction (range:2500:65000, 10000 is no correction)    
    \param GreenGamma      Green Gamma correction (range:2500:65000, 10000 is no correction)    
    \param BlueGamma       Blue  Gamma correction (range:2500:65000, 10000 is no correction)    
    \param ReferenceBlack  Values less than this are black (range:0:4000)                       
    \param ReferenceWhite  Values more than this are white (range:6000:10000)                   
    \param Contrast        Contrast     adjustment (range:-100:100, 0 is no correction)         
    \param Brightness      Brightness   adjustment (range:-100:100, 0 is no correction)         
    \param Colorfulness    Colorfulness adjustment (range:-100:100, 0 is no correction)         
    \param RedGreenTint    Tine         adjustment (range:-100:100, 0 is no correction)         
*/
U_COLORADJUSTMENT coloradjustment_set(
      uint16_t            Size,
      uint16_t            Flags,
      uint16_t            IlluminantIndex,
      uint16_t            RedGamma,
      uint16_t            GreenGamma,
      uint16_t            BlueGamma,
      uint16_t            ReferenceBlack,
      uint16_t            ReferenceWhite,
      int16_t             Contrast,
      int16_t             Brightness,
      int16_t             Colorfulness,
      int16_t             RedGreenTint
   ){
   U_COLORADJUSTMENT ca;
   ca.caSize            = Size;                     
   ca.caFlags           = Flags;          
   ca.caIlluminantIndex = IlluminantIndex;
   ca.caRedGamma        = U_MNMX(RedGamma,       U_RGB_GAMMA_MIN,       U_RGB_GAMMA_MAX);
   ca.caGreenGamma      = U_MNMX(GreenGamma,     U_RGB_GAMMA_MIN,       U_RGB_GAMMA_MAX);
   ca.caBlueGamma       = U_MNMX(BlueGamma,      U_RGB_GAMMA_MIN,       U_RGB_GAMMA_MAX);
   // Next one is different to eliminate compiler warning -  U_R_B_MIN is 0 and unsigned
   ca.caReferenceBlack  = U_MAX( ReferenceBlack, U_REFERENCE_BLACK_MAX);
   ca.caReferenceWhite  = U_MNMX(ReferenceWhite, U_REFERENCE_WHITE_MIN, U_REFERENCE_WHITE_MAX); 
   ca.caContrast        = U_MNMX(Contrast,       U_COLOR_ADJ_MIN,       U_COLOR_ADJ_MAX); 
   ca.caBrightness      = U_MNMX(Brightness,     U_COLOR_ADJ_MIN,       U_COLOR_ADJ_MAX); 
   ca.caColorfulness    = U_MNMX(Colorfulness,   U_COLOR_ADJ_MIN,       U_COLOR_ADJ_MAX); 
   ca.caRedGreenTint    = U_MNMX(RedGreenTint,   U_COLOR_ADJ_MIN,       U_COLOR_ADJ_MAX);
   return(ca);
}

/**
    \brief Construct a U_PIXELFORMATDESCRIPTOR structure.
    \return U_PIXELFORMATDESCRIPTOR structure
    \param dwFlags         PFD_dwFlags Enumeration
    \param iPixelType      PFD_iPixelType Enumeration
    \param cColorBits      RGBA: total bits per pixel
    \param cRedBits        Red   bits per pixel
    \param cRedShift       Red   shift to data bits
    \param cGreenBits      Green bits per pixel
    \param cGreenShift     Green shift to data bits
    \param cBlueBits       Blue  bits per pixel
    \param cBlueShift      Blue  shift to data bits
    \param cAlphaBits      Alpha bits per pixel
    \param cAlphaShift     Alpha shift to data bits
    \param cAccumBits      Accumulator buffer, total bitplanes
    \param cAccumRedBits   Red   accumulator buffer bitplanes
    \param cAccumGreenBits Green accumulator buffer bitplanes
    \param cAccumBlueBits  Blue  accumulator buffer bitplanes
    \param cAccumAlphaBits Alpha accumulator buffer bitplanes
    \param cDepthBits      Depth of Z-buffer
    \param cStencilBits    Depth of stencil buffer
    \param cAuxBuffers     Depth of auxilliary buffers (not supported)
    \param iLayerType      PFD_iLayerType Enumeration, may be ignored
    \param bReserved       Bits 0:3/4:7 are number of Overlay/Underlay planes 
    \param dwLayerMask     may be ignored
    \param dwVisibleMask   color or index of underlay plane
    \param dwDamageMask    may be ignored
*/
U_PIXELFORMATDESCRIPTOR pixelformatdescriptor_set(
      uint32_t   dwFlags,
      uint8_t    iPixelType,
      uint8_t    cColorBits,
      uint8_t    cRedBits,
      uint8_t    cRedShift,
      uint8_t    cGreenBits,
      uint8_t    cGreenShift,
      uint8_t    cBlueBits,
      uint8_t    cBlueShift,
      uint8_t    cAlphaBits,
      uint8_t    cAlphaShift,
      uint8_t    cAccumBits,
      uint8_t    cAccumRedBits,
      uint8_t    cAccumGreenBits,
      uint8_t    cAccumBlueBits,
      uint8_t    cAccumAlphaBits,
      uint8_t    cDepthBits,
      uint8_t    cStencilBits,
      uint8_t    cAuxBuffers,
      uint8_t    iLayerType,
      uint8_t    bReserved,
      uint32_t   dwLayerMask,
      uint32_t   dwVisibleMask,
      uint32_t   dwDamageMask
   ){
   U_PIXELFORMATDESCRIPTOR pfd;
   pfd.nSize           = sizeof(U_PIXELFORMATDESCRIPTOR);                  
   pfd.nVersion        = 1;        
   pfd.dwFlags         = dwFlags;         
   pfd.iPixelType      = iPixelType;      
   pfd.cColorBits      = cColorBits;      
   pfd.cRedBits        = cRedBits;        
   pfd.cRedShift       = cRedShift;       
   pfd.cGreenBits      = cGreenBits;      
   pfd.cGreenShift     = cGreenShift;     
   pfd.cBlueBits       = cBlueBits;       
   pfd.cBlueShift      = cBlueShift;      
   pfd.cAlphaBits      = cAlphaBits;      
   pfd.cAlphaShift     = cAlphaShift;     
   pfd.cAccumBits      = cAccumBits;      
   pfd.cAccumRedBits   = cAccumRedBits;   
   pfd.cAccumGreenBits = cAccumGreenBits; 
   pfd.cAccumBlueBits  = cAccumBlueBits;  
   pfd.cAccumAlphaBits = cAccumAlphaBits; 
   pfd.cDepthBits      = cDepthBits;      
   pfd.cStencilBits    = cStencilBits;    
   pfd.cAuxBuffers     = cAuxBuffers;     
   pfd.iLayerType      = iLayerType;      
   pfd.bReserved       = bReserved;       
   pfd.dwLayerMask     = dwLayerMask;     
   pfd.dwVisibleMask   = dwVisibleMask;   
   pfd.dwDamageMask    = dwDamageMask;
   return(pfd);    
}

/**
    \brief Allocate and create a U_EMRTEXT structure followed by its variable pieces via a char* pointer.
    Dx cannot be NULL, if the calling program has no appropriate values call dx_set() first. 
    \return char* pointer to U_EMRTEXT structure followed by its variable pieces, or NULL on error
    \param ptlReference String start coordinates
    \param NumString    Number of characters in string, does NOT include a terminator
    \param cbChar       Number of bytes per character
    \param String       String to write
    \param fOptions     ExtTextOutOptions Enumeration
    \param rcl          (Optional, when fOptions & 7) grayed/clipping/opaque rectangle
    \param Dx           Character spacing array from the start of the RECORD   
*/
char *emrtext_set(
      U_POINTL         ptlReference,
      U_NUM_STR        NumString,
      uint32_t         cbChar,
      void            *String,
      uint32_t         fOptions,
      U_RECTL          rcl,
      uint32_t        *Dx
    ){
    int   irecsize,cbDxArray,cbString4,cbString,off;
    char *record;
    uint32_t *loffDx;
    
    if(!String)return(NULL);
    if(!Dx)return(NULL);
    cbString = cbChar * NumString;                   // size of the string in bytes
    cbString4 = UP4(cbString);                       // size of the string buffer
    cbDxArray = sizeof(uint32_t)*NumString;          // size of Dx array storage
    if(fOptions & U_ETO_PDY)cbDxArray += cbDxArray;  // of the Dx buffer, here do both X and Y coordinates
    irecsize = sizeof(U_EMRTEXT) + sizeof(uint32_t) + cbString4 + cbDxArray;    // core structure  + offDx + string buf + dx buf
    if(!(fOptions & U_ETO_NO_RECT)){ irecsize += sizeof(U_RECTL); } // plus variable U_RECTL, when it is present
    record   = malloc(irecsize);
    if(record){
      ((PU_EMRTEXT)record)->ptlReference    = ptlReference;
      ((PU_EMRTEXT)record)->nChars          = NumString;
      // pick up ((PU_EMRTEXT)record)->offString later
      ((PU_EMRTEXT)record)->fOptions        = fOptions;
      off = sizeof(U_EMRTEXT);              // location where variable pieces will start to be written                
      if(!(fOptions & U_ETO_NO_RECT)){      // variable field, may or may not be present 
         memcpy(record + off,&rcl, sizeof(U_RECTL));
         off += sizeof(U_RECTL);
      }
      loffDx = (uint32_t *)(record + off);                  // offDx will go here, but we do not know with what value yet
      off += sizeof(uint32_t);
      memcpy(record + off,String,cbString);                 // copy the string data to its buffer
      ((PU_EMRTEXT)record)->offString       = off;          // now save offset in the structure
      off += cbString;
      if(cbString < cbString4){ 
        memset(record+off,0,cbString4-cbString); // keeps valgrind happy (initialize padding after string)
        off += cbString4-cbString;
      }
      memcpy(record + off, Dx, cbDxArray);                  // copy the Dx data to its buffer
      *loffDx = off;                                        // now save offDx to the structure
    }
    return(record);
}



/* **********************************************************************************************
These functions are simpler or more convenient ways to generate the specified types of EMR records.  
Each should be called in preference to the underlying "base" EMR function.
*********************************************************************************************** */


/**
    \brief Allocate and construct a U_EMRCOMMENT structure with a UTF8 string.
    A U_EMRCOMMENT contains application specific data, and that may include contain null characters. This function may be used when the
    comment only incluces UT8 text.
    \return pointer to U_EMRCOMMENT structure, or NULL on error.
    \param string  UTF8 string to store in the comment


*/
char *textcomment_set(
      const char *string
   ){
   if(!string)return(NULL);
   return(U_EMRCOMMENT_set(1 + strlen(string),string));
}

/**
    \brief Allocate and construct a U_EMRDELETEOBJECT structure and also delete the requested object from the table.
    Use this function instead of calling U_EMRDELETEOBJECT_set() directly.
    \return pointer to U_EMRDELETEOBJECT structure, or NULL on error.
    \param ihObject  Pointer to handle to delete.  This value is set to 0 if the function succeeds.
    \param eht       EMF handle table
    
    Note that calling this function should always be conditional on the specifed object being defined.  It is easy to
    write a program where deleteobject_set() is called in a sequence where, at the time, we know that ihObject is defined.
    Then a later modification, possibly quite far away in the code, causes it to be undefined.  That distant change will
    result in a failure when this function reutrns.  That problem cannot be handled here because the only  values which
    may be returned are a valid U_EMRDELETEOBJECT record or a NULL, and other errors could result in the NULL.  
    So the object must be checked before the call.
*/
char *deleteobject_set(
      uint32_t    *ihObject,
      EMFHANDLES  *eht
   ){
   uint32_t saveObject=*ihObject;
   if(emf_htable_delete(ihObject,eht))return(NULL);  // invalid handle or other problem, cannot be deleted
   return(U_EMRDELETEOBJECT_set(saveObject));
}

/**
    \brief Allocate and construct a U_EMRSELECTOBJECT structure, checks that the handle specified is one that can actually be selected.
    Use this function instead of calling U_EMRSELECTOBJECT_set() directly.
    \return pointer to U_EMRSELECTOBJECT structure, or NULL on error.
    \param ihObject  handle to select
    \param eht       EMF handle table
*/
char *selectobject_set(
      uint32_t    ihObject,
      EMFHANDLES *eht
   ){
   if(!(U_STOCK_OBJECT & ihObject)){        // not a stock object, those go straight through
     if(ihObject > eht->top)return(NULL);   // handle this high is not in the table
     if(!eht->table[ihObject])return(NULL); // handle is not in the table, so not active, so cannot be selected
   }
   return(U_EMRSELECTOBJECT_set(ihObject));
}

/**
    \brief Allocate and construct a U_EMREXTCREATEPEN structure, create a handle and return it.
    Use this function instead of calling U_EMREXTCREATEPEN_set() directly.
    \return pointer to U_EMREXTCREATEPEN structure, or NULL on error.
    \param ihPen handle to be used by new object
    \param eht   EMF handle table 
    \param Bmi   bitmapbuffer
    \param cbPx   Size in bytes of pixel array (row stride * height, there may be some padding at the end of each row)
    \param Px     pixel array (NULL if cbPx == 0)
    \param elp   Pen parameters (Size is Variable!!!!)
*/
char *extcreatepen_set(
      uint32_t        *ihPen,
      EMFHANDLES      *eht,
      PU_BITMAPINFO    Bmi,
      const uint32_t   cbPx,
      char            *Px,
      PU_EXTLOGPEN     elp
   ){
   if(emf_htable_insert(ihPen, eht))return(NULL);
   return(U_EMREXTCREATEPEN_set(*ihPen, Bmi, cbPx, Px, elp ));
}

/**
    \brief Allocate and construct a U_EMRCREATEPEN structure, create a handle and returns it
    Use this function instead of calling U_EMRCREATEPEN_set() directly.
    \return pointer to U_EMRCREATEPEN structure, or NULL on error.
    \param ihPen handle to be used by new object
    \param eht   EMF handle table 
    \param lopn  Pen parameters
*/
char *createpen_set(
      uint32_t   *ihPen,
      EMFHANDLES *eht,
      U_LOGPEN    lopn
   ){
   if(emf_htable_insert(ihPen, eht))return(NULL);
   return(U_EMRCREATEPEN_set(*ihPen, lopn));
}

/**
    \brief Allocate and construct a U_EMRCREATEBRUSHINDIRECT structure, create a handle and returns it
    Use this function instead of calling U_EMRCREATEBRUSHINDIRECT_set() directly.
    \return pointer to U_EMRCREATEBRUSHINDIRECT structure, or NULL on error.
    \param ihBrush handle to be used by new object 
    \param eht     EMF handle table 
    \param lb      Brush parameters  
*/
char *createbrushindirect_set(
      uint32_t    *ihBrush,
      EMFHANDLES  *eht,
      U_LOGBRUSH   lb
   ){
   if(emf_htable_insert(ihBrush, eht))return(NULL);
   return(U_EMRCREATEBRUSHINDIRECT_set(*ihBrush, lb));
}

/**
    \brief Allocate and construct a U_EMRCREATEDIBPATTERNBRUSHPT_set structure, create a handle and returns it
    Use this function instead of calling U_EMRCREATEDIBPATTERNBRUSHPT_set() directly.
    \return pointer to U_EMRCREATEDIBPATTERNBRUSHPT_set structure, or NULL on error.
    \param ihBrush handle to be used by new object 
    \param eht     EMF handle table 
    \param iUsage  DIBColors enumeration
    \param Bmi     Bitmap info
    \param cbPx    Size in bytes of pixel array (row stride * height, there may be some padding at the end of each row)
    \param Px      (Optional) bitmapbuffer (pixel array section )
*/
char *createdibpatternbrushpt_set(
      uint32_t            *ihBrush,
      EMFHANDLES          *eht,
      const uint32_t       iUsage, 
      PU_BITMAPINFO        Bmi,
      const uint32_t       cbPx,
      const char          *Px
      
   ){
   if(emf_htable_insert(ihBrush, eht))return(NULL);
   return(U_EMRCREATEDIBPATTERNBRUSHPT_set(*ihBrush, iUsage, Bmi, cbPx, Px));
}

/**
    \brief Allocate and construct a U_EMRCREATEMONOBRUSH_set structure, create a handle and returns it
    Use this function instead of calling U_EMRCREATEMONOBRUSH_set() directly.
    \return pointer to U_EMRCREATEMONOBRUSH_set structure, or NULL on error.
    \param ihBrush handle to be used by new object 
    \param eht     EMF handle table 
    \param iUsage  DIBColors enumeration
    \param Bmi     Bitmap info
    \param cbPx    Size in bytes of pixel array (row stride * height, there may be some padding at the end of each row)
    \param Px      (Optional) bitmapbuffer (pixel array section )
*/
char *createmonobrush_set(
      uint32_t            *ihBrush,
      EMFHANDLES          *eht,
      const uint32_t       iUsage, 
      PU_BITMAPINFO        Bmi,
      const uint32_t       cbPx,
      const char          *Px
      
   ){
   if(emf_htable_insert(ihBrush, eht))return(NULL);
   return(U_EMRCREATEMONOBRUSH_set(*ihBrush, iUsage, Bmi, cbPx, Px));
}


/**
    \brief Allocate and construct a U_EMRCREATECOLORSPACE structure, create a handle and returns it
    Use this function instead of calling U_EMRCREATECOLORSPACE_set() directly.
    \return pointer to U_EMRCREATECOLORSPACE structure, or NULL on error.
    \param ihCS  ColorSpace handle, will be created and returned
    \param eht   Pointer to structure holding all EMF handles
    \param lcs    ColorSpace parameters
*/
char *createcolorspace_set(
      uint32_t          *ihCS,
      EMFHANDLES         *eht,
      U_LOGCOLORSPACEA    lcs
   ){
   if(emf_htable_insert(ihCS, eht))return(NULL);
   return(U_EMRCREATECOLORSPACE_set(*ihCS,lcs));
}

/**
    \brief Allocate and construct a U_EMRCREATECOLORSPACEW structure, create a handle and returns it
    Use this function instead of calling U_EMRCREATECOLORSPACEW_set() directly.
    \return pointer to U_EMRCREATECOLORSPACEW structure, or NULL on error.
    \param ihCS    ColorSpace handle, will be created and returned
    \param eht     Pointer to structure holding all EMF handles
    \param lcs     ColorSpace parameters
    \param dwFlags If low bit set Data is present
    \param cbData  Number of bytes of theData field.
    \param Data    (Optional, dwFlags & 1) color profile data 
*/
char *createcolorspacew_set(
      uint32_t           *ihCS,
      EMFHANDLES         *eht,
      U_LOGCOLORSPACEW    lcs,
      uint32_t            dwFlags,
      U_CBDATA            cbData,
      uint8_t            *Data
   ){
   if(emf_htable_insert(ihCS, eht))return(NULL);
   return(U_EMRCREATECOLORSPACEW_set(*ihCS, lcs, dwFlags, cbData, Data));
}

/**
    \brief Allocate and construct a U_EMREXTCREATEFONTINDIRECTW structure, create a handle and returns it
    Use this function instead of calling U_EMREXTCREATEFONTINDIRECTW_set() directly.
    \return pointer to U_EMREXTCREATEFONTINDIRECTW structure, or NULL on error.
    \param ihFont  Font handle, will be created and returned
    \param eht     Pointer to structure holding all EMF handles
    \param elf     Pointer to Font parameters asPU_LOGFONT
    \param elfw    Pointer to Font parameters as U_LOGFONT_PANOSE
*/
char *extcreatefontindirectw_set(
      uint32_t   *ihFont,
      EMFHANDLES *eht,
      const char *elf,
      const char *elfw
   ){
   if(emf_htable_insert(ihFont, eht))return(NULL);
   return(U_EMREXTCREATEFONTINDIRECTW_set(*ihFont, elf, elfw));
}

/**
    \brief Allocate and construct a U_EMRCREATEPALETTE structure, create a handle and returns it
    Use this function instead of calling U_EMRCREATEPALETTE_set() directly.
    \return pointer to U_EMRCREATEPALETTE structure, or NULL on error.
    \param ihPal  Palette handle, will be created and returned
    \param eht    Pointer to structure holding all EMF handles
    \param lgpl   PaletteFont parameters
*/
char *createpalette_set(
      uint32_t     *ihPal,
      EMFHANDLES   *eht,
      U_LOGPALETTE  lgpl
   ){
  if(emf_htable_insert(ihPal, eht))return(NULL);
   return(U_EMRCREATEPALETTE_set(*ihPal, lgpl));
}

/**
    \brief Allocate and construct a U_EMRSETPALETTEENTRIES structure, create a handle and returns it
    Use this function instead of calling U_EMRSETPALETTEENTRIES_set() directly.
    \return pointer to U_EMRSETPALETTEENTRIES structure, or NULL on error.
    \param ihPal       Palette handle, will be created and returned
    \param eht         Pointer to structure holding all EMF handles
    \param iStart      First Palette entry in selected object to set
    \param cEntries    Number of Palette entries in selected object to set
    \param aPalEntries Values to set with
*/
char *setpaletteentries_set(
      uint32_t               *ihPal,
      EMFHANDLES             *eht,
      const uint32_t         iStart,
      const U_NUM_LOGPLTNTRY cEntries,
      const PU_LOGPLTNTRY    aPalEntries
   ){
   if(emf_htable_insert(ihPal, eht))return(NULL);
   return(U_EMRSETPALETTEENTRIES_set(*ihPal, iStart, cEntries, aPalEntries));
}

/**
    \brief Allocate and construct a U_EMRFILLRGN structure, create a handle and returns it
    Use this function instead of calling U_EMRFILLRGN_set() directly.
    \return pointer to U_EMRFILLRGN structure, or NULL on error.
    \param ihBrush   Brush handle, will be created and returned
    \param eht       Pointer to structure holding all EMF handles
    \param rclBounds Bounding rectangle in device units
    \param RgnData   Pointer to a U_RGNDATA structure
*/
char *fillrgn_set(
      uint32_t         *ihBrush,
      EMFHANDLES       *eht,
      const U_RECTL     rclBounds,
      const PU_RGNDATA  RgnData
   ){
   if(emf_htable_insert(ihBrush, eht))return(NULL);
   return(U_EMRFILLRGN_set(rclBounds, *ihBrush, RgnData));
}

/**
    \brief Allocate and construct a U_EMRFRAMERGN structure, create a handle and returns it
    Use this function instead of calling U_EMRFRAMERGN_set() directly.
    \return pointer to U_EMRFRAMERGN structure, or NULL on error.
    \param ihBrush   Brush handle, will be created and returned
    \param eht       Pointer to structure holding all EMF handles
    \param rclBounds Bounding rectangle in device units
    \param szlStroke W & H of Brush stroke
    \param RgnData   Pointer to a U_RGNDATA structure
*/
char *framergn_set(
      uint32_t         *ihBrush,
      EMFHANDLES       *eht,
      const U_RECTL     rclBounds,
      const U_SIZEL     szlStroke,
      const PU_RGNDATA  RgnData
   ){
   if(emf_htable_insert(ihBrush, eht))return(NULL);
   return(U_EMRFRAMERGN_set(rclBounds, *ihBrush, szlStroke, RgnData));
}

/**
    \brief Allocate and construct an array of U_POINT objects which has been subjected to a U_XFORM
    \returns pointer to an array of U_POINT structures.
    \param points  pointer to the source U_POINT structures
    \param count   number of members in points
    \param xform   U_XFORM to apply
    
    May also be used to modify U_RECT by doubling the count and casting the pointer.
*/
PU_POINT points_transform(PU_POINT points, int count, U_XFORM xform){
   PU_POINT newpts;
   int i;
   float x,y;
   newpts = (PU_POINT) malloc(count * sizeof(U_POINT));
   for(i=0; i<count; i++){
      x = (float) points[i].x;
      y = (float) points[i].y;
      newpts[i].x = U_ROUND(x * xform.eM11 + y * xform.eM21 + xform.eDx);
      newpts[i].y = U_ROUND(x * xform.eM12 + y * xform.eM22 + xform.eDy);
   }
   return(newpts);
}

/**
    \brief Allocate and construct an array of U_POINT16 objects  which has been subjected to a U_XFORM
    \returns pointer to an array of U_POINT16 structures.
    \param points  pointer to the source U_POINT16 structures
    \param count   number of members in points
    \param xform   U_XFORM to apply
    
    Transformed src points {x0,y0} appear at {x0*xscale + x, y0*yscale + y}
*/
PU_POINT16 point16_transform(PU_POINT16 points, int count, U_XFORM xform){
   PU_POINT16 newpts;
   int i;
   float x,y;
   newpts = (PU_POINT16) malloc(count * sizeof(U_POINT16));
   for(i=0; i<count; i++){
      x = (float) points[i].x;
      y = (float) points[i].y;
      newpts[i].x = U_ROUND(x * xform.eM11 + y * xform.eM21 + xform.eDx);
      newpts[i].y = U_ROUND(x * xform.eM12 + y * xform.eM22 + xform.eDy);
   }
   return(newpts);
}

/**
    \brief Allocate and construct an array of U_TRIVERTEX objects  which has been subjected to a U_XFORM
    \returns pointer to an array of U_TRIVERTEX structures.
    \param tv  pointer to the source U_TRIVERTEX structures
    \param count   number of members in points
    \param xform   U_XFORM to apply
    
    Transformed Trivertex points {x0,y0} appear at {x0*xscale + x, y0*yscale + y}
*/
PU_TRIVERTEX trivertex_transform(PU_TRIVERTEX tv, int count, U_XFORM xform){
   PU_TRIVERTEX newtvs;
   int i;
   float x,y;
   newtvs = (PU_TRIVERTEX) malloc(count * sizeof(U_TRIVERTEX));
   for(i=0; i<count; i++){
      x = (float) tv[i].x;
      y = (float) tv[i].y;
      newtvs[i]   = tv[i];
      newtvs[i].x = U_ROUND(x * xform.eM11 + y * xform.eM21 + xform.eDx);
      newtvs[i].y = U_ROUND(x * xform.eM12 + y * xform.eM22 + xform.eDy);
   }
   return(newtvs);
}

/**
    \brief Allocate and construct an array of U_POINT objects from a set of U_POINT16 objects
    \returns pointer to an array of U_POINT structures.
    \param points  pointer to the source U_POINT16 structures
    \param count   number of members in points
    
*/
PU_POINT point16_to_point(PU_POINT16 points, int count){
   PU_POINT newpts;
   int i;
   newpts = (PU_POINT) malloc(count * sizeof(U_POINT));
   for(i=0; i<count; i++){
      newpts[i].x = points[i].x;
      newpts[i].y = points[i].y;
   }
   return(newpts);
}

/**
    \brief Allocate and construct an array of U_POINT16 objects from a set of U_POINT objects
    \returns pointer to an array of U_POINT16 structures.
    \param points  pointer to the source U_POINT structures
    \param count   number of members in points
    
    If a coordinate is out of range it saturates at boundary.
*/
PU_POINT16 point_to_point16(PU_POINT points, int count){
   PU_POINT16 newpts;
   int i;
   newpts = (PU_POINT16) malloc(count * sizeof(U_POINT16));
   for(i=0; i<count; i++){
      newpts[i].x = U_MNMX(points[i].x, INT16_MIN, INT16_MAX);
      newpts[i].y = U_MNMX(points[i].y, INT16_MIN, INT16_MAX);
   }
   return(newpts);
}

// hide these from Doxygen
//! @cond
/* **********************************************************************************************
These functions contain shared code used by various U_EMR*_set functions.  These should NEVER be called
by end user code and to further that end prototypes are NOT provided and they are hidden from Doxygen.


   These are (mostly) ordered by U_EMR_* index number.
   For all _set functions the caller must eventually call free() on the returned pointer.
   
    CORE1(uint32_t iType, U_RECTL rclBounds, const uint32_t cptl, const U_POINTL *points){
    CORE2(uint32_t iType, U_RECTL rclBounds, const uint32_t nPolys, const uint32_t *aPolyCounts,const uint32_t cptl, const U_POINTL *points){
    CORE3(uint32_t iType, uint32_t iMode){       (generic 1 uint)
    CORE4(uint32_t iType, U_RECTL rclBox){
    CORE5(uint32_t iType){                       (generic noargs)
    CORE6(uint32_t iType, U_RECTL rclBounds, const uint32_t cpts, const U_POINT16 *points){  (16bit form of CORE1)
    CORE7(uint32_t iType, U_PAIR pair){
    CORE8(uint32_t iType, U_RECTL rclBounds, uint32_t iGraphicsMode, U_FLOAT exScale, U_FLOAT eyScale, PU_EMRTEXT emrtext){ 
    CORE9(uint32_t iType, U_RECTL rclBox, U_POINTL ptlStart, U_POINTL ptlEnd){
    CORE10(uint32_t iType, U_RECTL rclBounds, const uint32_t nPolys, const uint32_t *aPolyCounts,const uint32_t cpts, const U_POINT16 *points){ (16bit form of CORE2)
    CORE11(uint32_t iType, PU_RGNDATA RgnData){
    CORE12(uint32_t iType, uint32_t ihBrush, uint32_t iUsage, PU_BITMAPINFO Bmi){
    CORE13(uint32_t iType, U_RECTL rclBounds, U_POINTL Dest, U_POINTL cDest, 
              U_POINTL Src, U_POINTL cSrc, U_XFORM xformSrc, U_COLORREF crBkColorSrc, uint32_t iUsageSrc, 
              uint32_t Data, PU_BITMAPINFO Bmi);
*********************************************************************************************** */


// Functions with the same form starting with U_EMRPOLYBEZIER_set
char *U_EMR_CORE1_set(uint32_t iType, U_RECTL rclBounds, const uint32_t cptl, const U_POINTL *points){
   char *record;
   int   cbPoints;
   int   irecsize;

   cbPoints    = sizeof(U_POINTL)*cptl;
   irecsize = sizeof(U_EMRPOLYBEZIER) + cbPoints - sizeof(U_POINTL); // First instance is in struct
   record    = malloc(irecsize);
   if(record){
      ((PU_EMR)           record)->iType     = iType;
      ((PU_EMR)           record)->nSize     = irecsize;
      ((PU_EMRPOLYBEZIER) record)->rclBounds = rclBounds;
      ((PU_EMRPOLYBEZIER) record)->cptl      = cptl;
      memcpy(((PU_EMRPOLYBEZIER) record)->aptl,points,cbPoints);
   }
   return(record);
} 

// Functions with the same form starting with U_EMR_POLYPOLYLINE
char *U_EMR_CORE2_set(uint32_t iType, U_RECTL rclBounds, const uint32_t nPolys, const uint32_t *aPolyCounts,const uint32_t cptl, const U_POINTL *points){
   char *record;
   int   cbPolys,cbPoints,off;
   int   irecsize;

   cbPoints    = sizeof(U_POINTL)*cptl;
   cbPolys    = sizeof(uint32_t)*nPolys;
   irecsize = sizeof(U_EMRPOLYPOLYLINE) + cbPoints + cbPolys - sizeof(uint32_t); // First instance of each is in struct
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)             record)->iType     = iType;
      ((PU_EMR)             record)->nSize     = irecsize;
      ((PU_EMRPOLYPOLYLINE) record)->rclBounds = rclBounds;
      ((PU_EMRPOLYPOLYLINE) record)->nPolys    = nPolys;
      ((PU_EMRPOLYPOLYLINE) record)->cptl      = cptl;
      memcpy(((PU_EMRPOLYPOLYLINE) record)->aPolyCounts,aPolyCounts,cbPolys); 
      off = sizeof(U_EMRPOLYPOLYLINE) - 4 + cbPolys;
      memcpy(record + off,points,cbPoints);   
   }
   return(record);
} 

// Functions with the same form starting with U_EMR_SETMAPMODE_set
char *U_EMR_CORE3_set(uint32_t iType, uint32_t iMode){
   char *record;
   int   irecsize;

   irecsize = sizeof(U_EMRSETMAPMODE);
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)          record)->iType     = iType;
      ((PU_EMR)          record)->nSize     = irecsize;
      ((PU_EMRSETMAPMODE)record)->iMode     = iMode;
   }
   return(record);
} 

// Functions taking a single U_RECT or U_RECTL, starting with U_EMRELLIPSE_set, also U_EMRFILLPATH, 
char *U_EMR_CORE4_set(uint32_t iType, U_RECTL rclBox){
   char *record;
   int   irecsize;

   irecsize = sizeof(U_EMRELLIPSE);
   record   = malloc(irecsize);
   memset(record,0,irecsize);
   if(record){
      ((PU_EMR)       record)->iType  = iType;
      ((PU_EMR)       record)->nSize  = irecsize;
      ((PU_EMRELLIPSE)record)->rclBox = rclBox;  // bounding rectangle in logical units
   }
   return(record);
} 

// Functions with the same form starting with U_EMRSETMETARGN_set
char *U_EMR_CORE5_set(uint32_t iType){
   char *record;
   int   irecsize = 8;

   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)       record)->iType  = iType;
      ((PU_EMR)       record)->nSize  = irecsize;
   }
   return(record);
}

// Functions with the same form starting with U_EMRPOLYBEZIER16_set
char *U_EMR_CORE6_set(uint32_t iType, U_RECTL rclBounds, const uint32_t cpts, const U_POINT16 *points){
   char *record;
   int   cbPoints,cbPoints4,off;
   int   irecsize;

   cbPoints   = sizeof(U_POINT16)*cpts;
   cbPoints4   = UP4(cbPoints);
   off      = sizeof(U_EMR) + sizeof(U_RECTL) + sizeof(U_NUM_POINT16); // offset to the start of the variable region
   irecsize = off + cbPoints4; // First instance is in struct
   record    = malloc(irecsize);
   if(record){
      ((PU_EMR)             record)->iType     = iType;
      ((PU_EMR)             record)->nSize     = irecsize;
      ((PU_EMRPOLYBEZIER16) record)->rclBounds = rclBounds;
      ((PU_EMRPOLYBEZIER16) record)->cpts      = cpts;
      memcpy(record + off, points, cbPoints);
      if(cbPoints < cbPoints4){
         off += cbPoints;
         memset(record + off, 0, cbPoints4 - cbPoints);
      }
   }
   return(record);
} 


// Functions that take a single struct argument which contains two uint32_t, starting with U_EMRSETWINDOWEXTEX_set
// these all pass two 32 bit ints and are cast by the caller to U_PAIR
char *U_EMR_CORE7_set(uint32_t iType, U_PAIR pair){
   char *record;
   int   irecsize = sizeof(U_EMRGENERICPAIR);

   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)           record)->iType   = iType;
      ((PU_EMR)           record)->nSize   = irecsize;
      ((PU_EMRGENERICPAIR)record)->pair    = pair;
   }
   return(record);
}

// For U_EMREXTTEXTOUTA and U_EMREXTTEXTOUTW
char *U_EMR_CORE8_set(
       uint32_t            iType,
       U_RECTL             rclBounds,          // Bounding rectangle in device units
       uint32_t            iGraphicsMode,      // Graphics mode Enumeration
       U_FLOAT             exScale,            // scale to 0.01 mm units ( only if iGraphicsMode & GM_COMPATIBLE)
       U_FLOAT             eyScale,            // scale to 0.01 mm units ( only if iGraphicsMode & GM_COMPATIBLE)
       PU_EMRTEXT          emrtext             // Text parameters
   ){
   char *record;
   int   irecsize,cbString,cbString4,cbDx,cbEmrtext,cbEmrtextAll;
   uint32_t *loffDx;
   int  csize;
   
   if(     iType == U_EMR_EXTTEXTOUTA){ csize = 1; } // how many bytes per character
   else if(iType == U_EMR_EXTTEXTOUTW){ csize = 2; }
   else { return(NULL); }
 
   cbString = csize * emrtext->nChars;
   cbString4 = UP4(cbString);                                   // size of the string buffer
   cbEmrtext = sizeof(U_EMRTEXT);                             // size of the constant part of the U_EMRTEXT structure
   if(!(emrtext->fOptions & U_ETO_NO_RECT)){ cbEmrtext += sizeof(U_RECTL); } // plus the variable U_RECTL, when it is present
   cbDx = emrtext->nChars * sizeof(int32_t);             // size of Dx buffer
   if(emrtext->fOptions & U_ETO_PDY)cbDx += cbDx;      // size of Dx buffer when both x and y offsets are used
   cbEmrtextAll = cbEmrtext + sizeof(uint32_t) + cbString4 + cbDx;   // structure (+- rect) + offDx + string buf + dx buf + offDx
   
   /* adjust offset fields in emrtext to match the EMRTEXTOUT* field, currently they match EMRTEXT.
      This works because the variable pieces have all been moved outside of the U_EMRTEXT and U_EMRTEXTOUTA strutures.
   */
   ((PU_EMRTEXT)emrtext)->offString  += sizeof(U_EMREXTTEXTOUTA) - sizeof(U_EMRTEXT);   // adjust offString
    loffDx = (uint32_t *)((char *)emrtext + cbEmrtext);
    *loffDx += sizeof(U_EMREXTTEXTOUTA) - sizeof(U_EMRTEXT);

   // final record size is: U_EMREXTTEXTOUTA (includes constant part of U_EMRTEXT) + U_RECTL (if present) + offDx + dx buffer + string buffer
   irecsize = sizeof(U_EMREXTTEXTOUTA) + cbEmrtextAll - sizeof(U_EMRTEXT); // do not count core emrtext strcture twice
   record    = malloc(irecsize);
   if(record){
      ((PU_EMR)            record)->iType         = iType;
      ((PU_EMR)            record)->nSize         = irecsize;
      ((PU_EMREXTTEXTOUTA) record)->iGraphicsMode = iGraphicsMode;
      ((PU_EMREXTTEXTOUTA) record)->rclBounds     = rclBounds;
      ((PU_EMREXTTEXTOUTA) record)->exScale       = exScale;
      ((PU_EMREXTTEXTOUTA) record)->eyScale       = eyScale;
      // copy the adjusted U_EMRTEXT into the emrtext part of the full record..
      memcpy(&(((PU_EMREXTTEXTOUTA) record)->emrtext), emrtext, cbEmrtextAll);
   }
   return(record);
} 

// Functions that take a rect and a pair of points, starting with U_EMRARC_set
char *U_EMR_CORE9_set(uint32_t iType, U_RECTL rclBox, U_POINTL ptlStart, U_POINTL ptlEnd){
   char *record;
   int   irecsize = sizeof(U_EMRARC);

   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)           record)->iType    = iType;
      ((PU_EMR)           record)->nSize    = irecsize;
      ((PU_EMRARC)        record)->rclBox   = rclBox;
      ((PU_EMRARC)        record)->ptlStart = ptlStart;
      ((PU_EMRARC)        record)->ptlEnd   = ptlEnd;
   }
   return(record);
}

// Functions with the same form starting with U_EMR_POLYPOLYLINE16
char *U_EMR_CORE10_set(uint32_t iType, U_RECTL rclBounds, const uint32_t nPolys, const uint32_t *aPolyCounts,const uint32_t cpts, const U_POINT16 *points){
   char *record;
   int   cbPoints,cbPolys,off;
   int   irecsize;

   cbPolys  = sizeof(uint32_t)*nPolys;
   cbPoints = sizeof(U_POINT16)*cpts;
   irecsize = sizeof(U_EMRPOLYPOLYLINE16) + cbPoints + cbPolys - sizeof(uint32_t); // First instance of each is in struct
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)               record)->iType     = iType;
      ((PU_EMR)               record)->nSize     = irecsize;
      ((PU_EMRPOLYPOLYLINE16) record)->rclBounds = rclBounds;
      ((PU_EMRPOLYPOLYLINE16) record)->nPolys    = nPolys;
      ((PU_EMRPOLYPOLYLINE16) record)->cpts      = cpts;
      memcpy(((PU_EMRPOLYPOLYLINE16) record)->aPolyCounts,aPolyCounts,cbPolys); 
      off = sizeof(U_EMRPOLYPOLYLINE16) - 4 + cbPolys;
      memcpy(record + off,points,cbPoints);   
   }
   return(record);
} 

// common code for U_EMRINVERTRGN and U_EMRPAINTRGN,
char *U_EMR_CORE11_set(uint32_t iType, PU_RGNDATA RgnData){
   char *record;
   int   irecsize;
   int   cbRgns,cbRgns4,rds,rds4,off;

   if(!RgnData)return(NULL);
   cbRgns   = ((PU_RGNDATAHEADER) RgnData)->nRgnSize;
   cbRgns4  = UP4(cbRgns);
   rds      = sizeof(U_RGNDATAHEADER) + cbRgns;
   rds4     = UP4(rds);
   irecsize = sizeof(U_EMRINVERTRGN) - sizeof(U_RECTL) + cbRgns4;  // core + array - overlap of one rectL
   record    = malloc(irecsize);
   if(record){
      ((PU_EMR)           record)->iType     = iType;
      ((PU_EMR)           record)->nSize     = irecsize;
      ((PU_EMRINVERTRGN)  record)->rclBounds = ((PU_RGNDATAHEADER) RgnData)->rclBounds;
      ((PU_EMRINVERTRGN)  record)->cbRgnData = rds;
      off = sizeof(U_EMRINVERTRGN) - sizeof(U_RGNDATA);
      memcpy(record + off, RgnData, rds);
      off += rds;
      if(rds < rds4){ memset(record + off,0, rds4 - rds); } // clear any unused bytes
   }
   return(record);
} 


// common code for U_EMRCREATEMONOBRUSH_set and U_EMRCREATEDIBPATTERNBRUSHPT_set,
char *U_EMR_CORE12_set(
       uint32_t            iType,
       uint32_t            ihBrush,            // Index to place object in EMF object table (this entry must not yet exist)
       uint32_t            iUsage,             // DIBcolors Enumeration
       PU_BITMAPINFO       Bmi,                // (Optional) bitmapbuffer (U_BITMAPINFO + pixel array)
       const uint32_t      cbPx,               // Size in bytes of pixel array (row stride * height, there may be some padding at the end of each row)
       const char         *Px                  // (Optional) bitmapbuffer (pixel array section )
   ){
   char *record;
   int   irecsize;
   int   cbImage,cbImage4,cbBmi,off;

   SET_CB_FROM_PXBMI(Px,Bmi,cbImage,cbImage4,cbBmi,cbPx);
   
   irecsize = sizeof(U_EMRCREATEMONOBRUSH) + cbBmi + cbImage4;
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)                      record)->iType   = iType;                        
      ((PU_EMR)                      record)->nSize   = irecsize;                     
      ((PU_EMRCREATEMONOBRUSH)       record)->ihBrush = ihBrush;                      
      ((PU_EMRCREATEMONOBRUSH)       record)->iUsage  = iUsage;                       
      if(cbBmi){
         off = sizeof(U_EMRCREATEMONOBRUSH);
         memcpy(record + off, Bmi, cbBmi);
         ((PU_EMRCREATEMONOBRUSH)    record)->offBmi  = off;                          
         ((PU_EMRCREATEMONOBRUSH)    record)->cbBmi   = cbBmi;   
         off += cbBmi;
         memcpy(record + off, Px, cbPx);
         ((PU_EMRCREATEMONOBRUSH)    record)->offBits = off;                        
         ((PU_EMRCREATEMONOBRUSH)    record)->cbBits  = cbImage;
      }
      else {
         ((PU_EMRCREATEMONOBRUSH)    record)->offBmi  = 0;                            
         ((PU_EMRCREATEMONOBRUSH)    record)->cbBmi   = 0;                            
         ((PU_EMRCREATEMONOBRUSH)    record)->offBits = 0;                            
         ((PU_EMRCREATEMONOBRUSH)    record)->cbBits  = 0;                            
      }
   }
   return(record);
}

// common code for U_EMRBLEND_set and U_EMRTRANSPARENTBLT_set,
char *U_EMR_CORE13_set(
      uint32_t             iType,
      U_RECTL              rclBounds,       // Bounding rectangle in device units
      U_POINTL             Dest,            // Destination UL corner in logical units
      U_POINTL             cDest,           // Destination width in logical units
      U_POINTL             Src,             // Source UL corner in logical units
      U_POINTL             cSrc,            // Src W & H in logical units
      U_XFORM              xformSrc,        // Transform to apply to source
      U_COLORREF           crBkColorSrc,    // Background color
      uint32_t             iUsageSrc,       // DIBcolors Enumeration
      uint32_t             Data,            // The meaning and type of this field varies, but it is always 4 bytes
      const PU_BITMAPINFO  Bmi,             // (Optional) bitmapbuffer (U_BITMAPINFO section)
      const uint32_t       cbPx,            // Size in bytes of pixel array (row stride * height, there may be some padding at the end of each row)
      char                *Px               // (Optional) bitmapbuffer (pixel array section )
   ){
   char *record;
   int   irecsize;
   int   cbImage,cbImage4,cbBmi,off;

   SET_CB_FROM_PXBMI(Px,Bmi,cbImage,cbImage4,cbBmi,cbPx);
   
   irecsize = sizeof(U_EMRALPHABLEND) + cbBmi + cbImage4;
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)                 record)->iType        = iType;
      ((PU_EMR)                 record)->nSize        = irecsize;
      ((PU_EMRALPHABLEND)       record)->rclBounds    = rclBounds;
      ((PU_EMRALPHABLEND)       record)->Dest         = Dest;
      ((PU_EMRALPHABLEND)       record)->cDest        = cDest;
      ((PU_EMRALPHABLEND)       record)->Blend        =  *((PU_BLEND)&Data);
      ((PU_EMRALPHABLEND)       record)->Src          = Src;
      ((PU_EMRALPHABLEND)       record)->xformSrc     = xformSrc;
      ((PU_EMRALPHABLEND)       record)->crBkColorSrc = crBkColorSrc;
      ((PU_EMRALPHABLEND)       record)->iUsageSrc    = iUsageSrc;
      off = sizeof(U_EMRALPHABLEND);
      APPEND_PXBMISRC(record, U_EMRALPHABLEND, cbBmi, Bmi, Px, cbImage, cbImage4);
      ((PU_EMRALPHABLEND)       record)->cSrc         = cSrc;
   }
   return(record);
}
//! @endcond

/* **********************************************************************************************
These are the core EMR functions, each creates a particular type of record.  
All return these records via a char* pointer, which is NULL if the call failed.  
They are listed in order by the corresponding U_EMR_* index number.  
*********************************************************************************************** */

// U_EMRHEADER_set                     1

/**
    \brief Allocate and construct a U_EMRHEADER record.
    \return pointer to U_EMRHEADER record, or NULL on error.
    \param rclBounds       Bounding rectangle in device units 
    \param rclFrame        Bounding rectangle in 0.01 mm units
    \param pfmtDesc        Pointer to a PixelFormatDescriptor
    \param nDesc           number of characters in Description, will include first three '\0'
    \param Description     Description, formatted like: text1\0text2\0\0
    \param szlDevice       Reference device size in pixels  
    \param szlMillimeters  Reference device size in 0.01 mm
    \param bOpenGL         nonZero if OpenGL commands are included
*/
char *U_EMRHEADER_set( 
      const U_RECTL                  rclBounds,
      const U_RECTL                  rclFrame,
      U_PIXELFORMATDESCRIPTOR* const pfmtDesc,
      U_CBSTR                        nDesc,
      uint16_t* const                Description,
      const U_SIZEL                  szlDevice,
      const U_SIZEL                  szlMillimeters,
      const uint32_t                 bOpenGL
   ){

   char *record; 
   int cbPFD,cbDesc,cbDesc4;
   uint32_t off;
   int irecsize;

   if(pfmtDesc){    cbPFD = sizeof(U_PIXELFORMATDESCRIPTOR); }
   else  {          cbPFD = 0;                               }
   if(Description){ cbDesc = 2*nDesc;                         } // also copy the terminator.  Size is in bytes
   else  {          cbDesc = 0;                               }
   cbDesc4  = UP4(cbDesc);
   irecsize = sizeof(U_EMRHEADER) + cbPFD + cbDesc4;
   record   = malloc(irecsize);
   if(record){
      off = sizeof(U_EMRHEADER);
      ((PU_EMR)           record)->iType             = U_EMR_HEADER;
      ((PU_EMR)           record)->nSize             = irecsize;
      ((PU_EMRHEADER)     record)->rclBounds         = rclBounds;
      ((PU_EMRHEADER)     record)->rclFrame          = rclFrame;
      ((PU_EMRHEADER)     record)->dSignature        = U_ENHMETA_SIGNATURE;
      ((PU_EMRHEADER)     record)->nVersion          = U_ENHMETA_VERSION;  
      ((PU_EMRHEADER)     record)->nBytes            = 0;  // Not known yet
      ((PU_EMRHEADER)     record)->nRecords          = 0;  // Not known yet
      ((PU_EMRHEADER)     record)->nHandles          = 0;  // Not known yet
      ((PU_EMRHEADER)     record)->sReserved         = 0;  // Must be 0
      ((PU_EMRHEADER)     record)->nDescription      = nDesc;
      ((PU_EMRHEADER)     record)->offDescription    = 0;  // may change below  
      ((PU_EMRHEADER)     record)->nPalEntries       = 0;  // Not known yet
      ((PU_EMRHEADER)     record)->szlDevice         = szlDevice;
      ((PU_EMRHEADER)     record)->szlMillimeters    = szlMillimeters;
      ((PU_EMRHEADER)     record)->cbPixelFormat     = cbPFD;
      ((PU_EMRHEADER)     record)->offPixelFormat    = 0;  // may change below 
      ((PU_EMRHEADER)     record)->bOpenGL           = bOpenGL;
      ((PU_EMRHEADER)     record)->szlMicrometers.cx = szlMillimeters.cx*1000;   
      ((PU_EMRHEADER)     record)->szlMicrometers.cy = szlMillimeters.cy*1000;   
      if(cbDesc4){
          ((PU_EMRHEADER) record)->offDescription    = off;
          memcpy(record + off, Description, cbDesc);
          off += cbDesc;
          if(cbDesc < cbDesc4)memset(record + off,0,cbDesc4-cbDesc);  // clear any unused bytes
          off += cbDesc4 - cbDesc;
      }
      if(cbPFD){
         ((PU_EMRHEADER)  record)->offPixelFormat   = off;
         memcpy(record+off,pfmtDesc,cbPFD);    
      }
   }
   return(record);
}

// U_EMRPOLYBEZIER_set                 2
/**
    \brief Allocate and construct a U_EMR_POLYBEZIER record.
    \return pointer to U_EMR_POLYBEZIER record, or NULL on error.
    \param rclBounds   bounding rectangle in device units
    \param cptl        Number of points to draw
    \param points      array of points
*/
char *U_EMRPOLYBEZIER_set(
      const U_RECTL   rclBounds,
      const uint32_t  cptl,
      const U_POINTL *points
   ){
   return(U_EMR_CORE1_set(U_EMR_POLYBEZIER, rclBounds, cptl, points));
} 

// U_EMRPOLYGON_set                    3
/**
    \brief Allocate and construct a U_EMR_POLYGON record.
    \return pointer to U_EMR_POLYGON record, or NULL on error.
    \param rclBounds   bounding rectangle in device units
    \param cptl        Number of points to draw
    \param points      array of points
*/
char *U_EMRPOLYGON_set(
      const U_RECTL   rclBounds,
      const uint32_t  cptl,
      const U_POINTL *points
   ){
   return(U_EMR_CORE1_set(U_EMR_POLYGON, rclBounds, cptl, points));
} 

// U_EMRPOLYLINE_set                   4
/**
    \brief Allocate and construct a U_EMR_POLYLINE record.
    \return pointer to U_EMR_POLYLINE record, or NULL on error.
    \param rclBounds   bounding rectangle in device units
    \param cptl        Number of points to draw
    \param points      array of points
*/
char *U_EMRPOLYLINE_set(
      const U_RECTL   rclBounds,
      const uint32_t  cptl,
      const U_POINTL *points
   ){
   return(U_EMR_CORE1_set(U_EMR_POLYLINE, rclBounds, cptl, points));
} 

// U_EMRPOLYBEZIERTO_set               5
/**
    \brief Allocate and construct a U_EMR_POLYBEZIERTO record.
    \return pointer to U_EMR_POLYBEZIERTO record, or NULL on error.
    \param rclBounds   bounding rectangle in device units
    \param cptl        Number of points to draw
    \param points      array of points
*/
char *U_EMRPOLYBEZIERTO_set(
      const U_RECTL   rclBounds,
      const uint32_t  cptl,
      const U_POINTL *points
   ){
   return(U_EMR_CORE1_set(U_EMR_POLYBEZIERTO, rclBounds, cptl, points));
} 

// U_EMRPOLYLINETO_set                 6
/**
    \brief Allocate and construct a U_EMR_POLYLINETO record.
    \return pointer to U_EMR_POLYLINETO record, or NULL on error.
    \param rclBounds   bounding rectangle in device units
    \param cptl        Number of points to draw
    \param points      array of points
*/
char *U_EMRPOLYLINETO_set(
      const U_RECTL   rclBounds,
      const uint32_t  cptl,
      const U_POINTL *points
   ){
   return(U_EMR_CORE1_set(U_EMR_POLYLINETO, rclBounds, cptl, points));
} 

// U_EMRPOLYPOLYLINE_set               7
/**
    \brief Allocate and construct a U_EMR_POLYPOLYLINE record.
    \return pointer to U_EMR_POLYPOLYLINE record, or NULL on error.
    \param rclBounds    bounding rectangle in device units
    \param nPolys       Number of elements in aPolyCounts
    \param aPolyCounts  Number of points in each poly (sequential)
    \param cptl         Total number of points (over all poly)
    \param points       array of points
*/
char *U_EMRPOLYPOLYLINE_set(
      const U_RECTL   rclBounds,
      const uint32_t  nPolys,
      const uint32_t *aPolyCounts,
      const uint32_t  cptl,
      const U_POINTL *points
   ){
   return(U_EMR_CORE2_set(U_EMR_POLYPOLYLINE, rclBounds, nPolys, aPolyCounts,cptl, points));
}

// U_EMRPOLYPOLYGON_set                8
/**
    \brief Allocate and construct a U_EMR_POLYPOLYGON record.
    \return pointer to U_EMR_POLYPOLYGON record, or NULL on error.
    \param rclBounds    bounding rectangle in device units
    \param nPolys       Number of elements in aPolyCounts
    \param aPolyCounts  Number of points in each poly (sequential)
    \param cptl         Total number of points (over all poly)
    \param points       array of points
*/
char *U_EMRPOLYPOLYGON_set(
      const U_RECTL   rclBounds,
      const uint32_t  nPolys,
      const uint32_t *aPolyCounts,
      const uint32_t  cptl,
      const U_POINTL *points
   ){
   return(U_EMR_CORE2_set(U_EMR_POLYPOLYGON, rclBounds, nPolys, aPolyCounts,cptl, points));
}

// U_EMRSETWINDOWEXTEX_set             9
/** 
    \brief Allocate and construct a U_EMR_SETWINDOWEXTEX record.
    \return pointer to U_EMR_SETWINDOWEXTEX record, or NULL on error.
    \param szlExtent H & V extent in logical units
*/
char *U_EMRSETWINDOWEXTEX_set(
      const U_SIZEL szlExtent 
   ){
   U_PAIR temp;
   temp.x = szlExtent.cx;
   temp.y = szlExtent.cy;
   return(U_EMR_CORE7_set(U_EMR_SETWINDOWEXTEX, temp)); 
}

// U_EMRSETWINDOWORGEX_set            10
/**
    \brief Allocate and construct a U_EMR_SETWINDOWORGEX record.
    \return pointer to U_EMR_SETWINDOWORGEX record, or NULL on error.
    \param ptlOrigin H & V origin in logical units
*/
char *U_EMRSETWINDOWORGEX_set(
      const U_POINTL ptlOrigin
   ){
   return(U_EMR_CORE7_set(U_EMR_SETWINDOWORGEX, ptlOrigin)); // U_PAIR and U_POINTL are the same thing
}

// U_EMRSETVIEWPORTEXTEX_set          11
/**
    \brief Allocate and construct a U_EMR_SETVIEWPORTEXTEX record.
    \return pointer to U_EMR_SETVIEWPORTEXTEX record, or NULL on error.
    \param szlExtent  H & V extent in logical units
*/
char *U_EMRSETVIEWPORTEXTEX_set(
      const U_SIZEL szlExtent
   ){
   U_PAIR temp;
   temp.x = szlExtent.cx;
   temp.y = szlExtent.cy;
   return(U_EMR_CORE7_set(U_EMR_SETVIEWPORTEXTEX, temp)); 
}

// U_EMRSETVIEWPORTORGEX_set          12
/**
    \brief Allocate and construct a U_EMR_SETVIEWPORTORGEX record.
    \return pointer to U_EMR_SETVIEWPORTORGEX record, or NULL on error.
    \param ptlOrigin  H & V origin in logical units
*/
char *U_EMRSETVIEWPORTORGEX_set(
      const U_POINTL ptlOrigin
   ){
   return(U_EMR_CORE7_set(U_EMR_SETVIEWPORTORGEX, ptlOrigin));  // U_PAIR and U_POINTL are the same thing
}

// U_EMRSETBRUSHORGEX_set             13
/**
    \brief Allocate and construct a U_EMR_SETBRUSHORGEX record.
    \return pointer to U_EMR_SETBRUSHORGEX record, or NULL on error.
    \param ptlOrigin   H & V origin in logical units
*/
char *U_EMRSETBRUSHORGEX_set(
      const U_POINTL ptlOrigin
   ){
   return(U_EMR_CORE7_set(U_EMR_SETBRUSHORGEX, *((PU_PAIR) & ptlOrigin))); 
}

// U_EMREOF_set                       14
/**
    \brief Allocate and construct a U_EMR_EOF record.
    \return pointer to U_EMR_EOF record, or NULL on error.
    \param cbPalEntries  Number of palette entries
    \param PalEntries    (optional) array of PalEntries
    \param et            tracking information, needed for nSizeLast calculation
*/
char *U_EMREOF_set(
      const U_CBPLENTRIES  cbPalEntries,
      const PU_LOGPLTNTRY  PalEntries,
      EMFTRACK            *et
   ){
   char *record;
   char *ptr;
   int  irecsize;
   int  cbPals;   // space allocated for Palette Entries
   uint32_t off;

   if(cbPalEntries && !PalEntries)return(NULL);
   if(!et)return(NULL);
   cbPals     = cbPalEntries * sizeof(U_LOGPLTNTRY);
   irecsize  = sizeof(U_EMREOF) + cbPals + sizeof(uint32_t);  //invariant core, variable palette, palette byte count
   record    = malloc(irecsize);
   if(record){
      ((PU_EMR)         record)->iType         = U_EMR_EOF;
      ((PU_EMR)         record)->nSize         = irecsize;
      ((PU_EMREOF)      record)->cbPalEntries  = cbPalEntries;
      ((PU_EMREOF)      record)->offPalEntries = 0;  // May be changed below
      off = sizeof(U_EMREOF);         //start of the variable region
      if(cbPals){
         ((PU_EMREOF)   record)->offPalEntries = off;
         memcpy(record+off,PalEntries,cbPals);
         off += cbPals;
      }
      ptr = record + off;
      *(uint32_t *)ptr = irecsize + et->used;  // EMREOF nSizeLast field, not at a fixed position, cannot be accessed by field name
   }
   et->PalEntries = cbPalEntries;
   return(record);
} 


// U_EMRSETPIXELV_set                 15
/**
    \brief Allocate and construct a U_EMR_SETPIXELV record.
    \return pointer to U_EMR_SETPIXELV record, or NULL on error.
    \param ptlPixel Pixel coordinates (logical)
    \param crColor  Pixel color
*/
char *U_EMRSETPIXELV_set(
      const U_POINTL    ptlPixel,
      const U_COLORREF  crColor
   ){
   char *record;
   int  irecsize;

   irecsize = sizeof(U_EMRSETPIXELV);
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)         record)->iType    = U_EMR_SETPIXELV;
      ((PU_EMR)         record)->nSize    = irecsize;
      ((PU_EMRSETPIXELV)record)->ptlPixel = ptlPixel;
      ((PU_EMRSETPIXELV)record)->crColor  = crColor;
   }
   return(record);
} 


// U_EMRSETMAPPERFLAGS_set            16
/**
    \brief Allocate and construct a U_EMR_SETMAPPERFLAGS record.
    \return pointer to U_EMR_SETMAPPERFLAGS record, or NULL on error.
*/
char *U_EMRSETMAPPERFLAGS_set(void){
   char *record;
   int  irecsize;

   irecsize = sizeof(U_EMRSETMAPPERFLAGS);
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)              record)->iType   = U_EMR_SETMAPPERFLAGS;
      ((PU_EMR)              record)->nSize   = irecsize;
      ((PU_EMRSETMAPPERFLAGS)record)->dwFlags = 1;
   }
   return(record);
} 

// U_EMRSETMAPMODE_set                17
/**
    \brief Allocate and construct a U_EMR_SETMAPMODE record.
    \return pointer to U_EMR_SETMAPMODE record, or NULL on error.
    \param iMode  MapMode Enumeration
*/
char *U_EMRSETMAPMODE_set(
      const uint32_t iMode
   ){
   return(U_EMR_CORE3_set(U_EMR_SETMAPMODE, iMode));
}

// U_EMRSETBKMODE_set                 18
/**
    \brief Allocate and construct a U_EMR_SETBKMODE record.
    \return pointer to U_EMR_SETBKMODE record, or NULL on error.
    \param iMode BackgroundMode Enumeration
*/
char *U_EMRSETBKMODE_set( 
      const uint32_t iMode
   ){
  return(U_EMR_CORE3_set(U_EMR_SETBKMODE, iMode));
}

// U_EMRSETPOLYFILLMODE_set           19
/**
    \brief Allocate and construct a U_EMR_SETPOLYFILLMODE record.
    \return pointer to U_EMR_SETPOLYFILLMODE record, or NULL on error.
    \param iMode  PolygonFillMode Enumeration
*/
char *U_EMRSETPOLYFILLMODE_set(
      const uint32_t iMode
   ){
   return(U_EMR_CORE3_set(U_EMR_SETPOLYFILLMODE, iMode));
}

// U_EMRSETROP2_set                   20
/**
    \brief Allocate and construct a U_EMR_SETROP2 record.
    \return pointer to U_EMR_SETROP2 record, or NULL on error.
    \param iMode  RasterOperation2 Enumeration
*/
char *U_EMRSETROP2_set(
      const uint32_t iMode 
   ){
   return(U_EMR_CORE3_set(U_EMR_SETROP2, iMode));
}

// U_EMRSETSTRETCHBLTMODE_set         21
/**
    \brief Allocate and construct a U_EMR_SETSTRETCHBLTMODE record.
    \return pointer to U_EMR_SETSTRETCHBLTMODE record, or NULL on error.
    \param iMode  StretchMode Enumeration
*/
char *U_EMRSETSTRETCHBLTMODE_set(
      const uint32_t iMode
   ){
   return(U_EMR_CORE3_set(U_EMR_SETSTRETCHBLTMODE, iMode));
}

// U_EMRSETTEXTALIGN_set              22
/**
    \brief Allocate and construct a U_EMR_SETTEXTALIGN record.
    \return pointer to U_EMR_SETTEXTALIGN record, or NULL on error.
    \param iMode  TextAlignment Enumeration
*/
char *U_EMRSETTEXTALIGN_set(
      const uint32_t iMode 
   ){
   return(U_EMR_CORE3_set(U_EMR_SETTEXTALIGN, iMode));
}

// U_EMRSETCOLORADJUSTMENT_set        23
/**
    \brief Allocate and construct a U_EMR_SETCOLORADJUSTMENT record.
    \return pointer to U_EMR_SETCOLORADJUSTMENT record, or NULL on error.
    \param ColorAdjustment Color Adjustment
*/
char *U_EMRSETCOLORADJUSTMENT_set(
      const U_COLORADJUSTMENT ColorAdjustment
   ){
   char *record;
   int  irecsize;

   irecsize = sizeof(U_EMRSETCOLORADJUSTMENT);
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)                   record)->iType           = U_EMR_SETCOLORADJUSTMENT;
      ((PU_EMR)                   record)->nSize           = irecsize;
      ((PU_EMRSETCOLORADJUSTMENT) record)->ColorAdjustment = ColorAdjustment;
   }
   return(record);
}

// U_EMRSETTEXTCOLOR_set              24
/**
    \brief Allocate and construct a U_EMR_SETTEXTCOLOR record.
    \return pointer to U_EMR_SETTEXTCOLOR record, or NULL on error.
    \param crColor  Text Color
*/
char *U_EMRSETTEXTCOLOR_set(
      const U_COLORREF crColor
  ){
  return(U_EMR_CORE3_set(U_EMR_SETTEXTCOLOR, *(uint32_t *) &crColor));
}

// U_EMRSETBKCOLOR_set                25
/**
    \brief Allocate and construct a U_EMR_SETBKCOLOR record.
    \return pointer to U_EMR_SETBKCOLOR record, or NULL on error.
    \param crColor Background Color
*/
char *U_EMRSETBKCOLOR_set(
      const U_COLORREF crColor
  ){
  return(U_EMR_CORE3_set(U_EMR_SETBKCOLOR, *(uint32_t *) &crColor));
}

// U_EMROFFSETCLIPRGN_set             26
/**
    \brief Allocate and construct a U_EMR_OFFSETCLIPRGN record.
    \return pointer to U_EMR_OFFSETCLIPRGN record, or NULL on error.
    \param ptl Clipping region 
*/
char *U_EMROFFSETCLIPRGN_set(
      const U_POINTL ptl
   ){
  return(U_EMR_CORE7_set(U_EMR_OFFSETCLIPRGN, ptl));
}

// U_EMRMOVETOEX_set                  27
/** 
    \brief Allocate and construct a U_EMR_MOVETOEX record.
    \return pointer to U_EMR_MOVETOEX record, or NULL on error.
    \param ptl Point coordinates
*/
char *U_EMRMOVETOEX_set(
      const U_POINTL ptl
   ){
   return(U_EMR_CORE7_set(U_EMR_MOVETOEX, ptl));
}

// U_EMRSETMETARGN_set                28
/**
    \brief Allocate and construct a U_EMR_SETMETARGN record.
    \return pointer to U_EMR_SETMETARGN record, or NULL on error.
*/
char *U_EMRSETMETARGN_set(void){
  return(U_EMR_CORE5_set(U_EMR_SETMETARGN));
}

// U_EMREXCLUDECLIPRECT_set           29
/**
    \brief Allocate and construct a U_EMR_EXCLUDECLIPRECT record.
    \return pointer to U_EMR_EXCLUDECLIPRECT record, or NULL on error.
    \param rclClip Clipping Region
*/
char *U_EMREXCLUDECLIPRECT_set(
      const U_RECTL rclClip
    ){
    return(U_EMR_CORE4_set(U_EMR_EXCLUDECLIPRECT,rclClip));
} 

// U_EMRINTERSECTCLIPRECT_set         30
/**
    \brief Allocate and construct a U_EMR_INTERSECTCLIPRECT record.
    \return pointer to U_EMR_INTERSECTCLIPRECT record, or NULL on error.
    \param rclClip Clipping Region
*/
char *U_EMRINTERSECTCLIPRECT_set(
      const U_RECTL rclClip
    ){
    return(U_EMR_CORE4_set(U_EMR_INTERSECTCLIPRECT,rclClip));
} 

// U_EMRSCALEVIEWPORTEXTEX_set        31
/** 
    \brief Allocate and construct a U_EMR_SCALEVIEWPORTEXTEX record.
    \return pointer to U_EMR_SCALEVIEWPORTEXTEX record, or NULL on error.
    \param xNum   Horizontal multiplier (!=0)
    \param xDenom Horizontal divisor    (!=0)
    \param yNum   Vertical   multiplier (!=0)
    \param yDenom Vertical   divisor    (!=0)
*/
char *U_EMRSCALEVIEWPORTEXTEX_set(
    const int32_t  xNum,
    const int32_t  xDenom,
    const int32_t  yNum,
    const int32_t  yDenom
  ){
  return(U_EMR_CORE4_set(U_EMR_SCALEVIEWPORTEXTEX,(U_RECTL){xNum,xDenom,yNum,yDenom}));
}


// U_EMRSCALEWINDOWEXTEX_set          32
/** 
    \brief Allocate and construct a U_EMR_SCALEWINDOWEXTEX record.
    \return pointer to U_EMR_SCALEWINDOWEXTEX record, or NULL on error.
    \param xNum   Horizontal multiplier (!=0)
    \param xDenom Horizontal divisor    (!=0)
    \param yNum   Vertical   multiplier (!=0)
    \param yDenom Vertical   divisor    (!=0)
*/
char *U_EMRSCALEWINDOWEXTEX_set(
    const int32_t  xNum,
    const int32_t  xDenom,
    const int32_t  yNum,
    const int32_t  yDenom
  ){
  return(U_EMR_CORE4_set(U_EMR_SCALEWINDOWEXTEX,(U_RECTL){xNum,xDenom,yNum,yDenom}));
}

// U_EMRSAVEDC_set                    33
/**
    \brief Allocate and construct a U_EMR_SAVEDC record.
    \return pointer to U_EMR_SAVEDC record, or NULL on error.
*/
char *U_EMRSAVEDC_set(void){
  return(U_EMR_CORE5_set(U_EMR_SAVEDC));
}

// U_EMRRESTOREDC_set                 34
/**
    \brief Allocate and construct a U_EMR_RESTOREDC record.
    \return pointer to U_EMR_RESTOREDC record, or NULL on error.
    \param iRelative DC to restore. -1 is preceding
*/
char *U_EMRRESTOREDC_set(
    const int32_t iRelative
  ){
  return(U_EMR_CORE3_set(U_EMR_RESTOREDC, (uint32_t) iRelative));
}

// U_EMRSETWORLDTRANSFORM_set         35
/**
    \brief Allocate and construct a U_EMR_SETWORLDTRANSFORM record.
    \return pointer to U_EMR_SETWORLDTRANSFORM record, or NULL on error.
    \param xform Transform to use
*/
char *U_EMRSETWORLDTRANSFORM_set(
      const U_XFORM xform
   ){
   char *record;
   int  irecsize;

   irecsize = sizeof(U_EMRSETWORLDTRANSFORM);
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)                  record)->iType = U_EMR_SETWORLDTRANSFORM;
      ((PU_EMR)                  record)->nSize = irecsize;
      ((PU_EMRSETWORLDTRANSFORM) record)->xform = xform;
   }
   return(record);
} 

// U_EMRMODIFYWORLDTRANSFORM_set      36
/**
    \brief Allocate and construct a U_EMR_MODIFYWORLDTRANSFORM record.
    \return pointer to U_EMR_MODIFYWORLDTRANSFORM record, or NULL on error.
    \param xform Transform to use
    \param iMode ModifyWorldTransformMode Enumeration
*/
char *U_EMRMODIFYWORLDTRANSFORM_set(
      const U_XFORM  xform,
      const uint32_t iMode
   ){
   char *record;
   int irecsize;

   irecsize = sizeof(U_EMRMODIFYWORLDTRANSFORM);
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)                     record)->iType = U_EMR_MODIFYWORLDTRANSFORM;
      ((PU_EMR)                     record)->nSize = irecsize;
      ((PU_EMRMODIFYWORLDTRANSFORM) record)->xform = xform;
      ((PU_EMRMODIFYWORLDTRANSFORM) record)->iMode = iMode;
   }
   return(record);
} 

// U_EMRSELECTOBJECT_set              37
/**
    \brief Allocate and construct a U_EMR_SELECTOBJECT record.
    Use selectobject_set() instead of calling this function directly.
    \return pointer to U_EMR_SELECTOBJECT record, or NULL on error.
    \param ihObject Number of a stock or created object
*/
char *U_EMRSELECTOBJECT_set(
      const uint32_t ihObject 
   ){
   char *record;
   int  irecsize;

   irecsize = sizeof(U_EMRSELECTOBJECT);
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)             record)->iType    = U_EMR_SELECTOBJECT;
      ((PU_EMR)             record)->nSize    = irecsize;
      ((PU_EMRSELECTOBJECT) record)->ihObject = ihObject;  // Index of object to SELECT
   }
   return(record);
} 

// U_EMRCREATEPEN_set                 38
/**
    \brief Allocate and construct a U_EMR_CREATEPEN record.
    Use createpen_set() instead of calling this function directly.
    \return pointer to U_EMR_CREATEPEN record, or NULL on error.
    \param ihPen Handle of created pen
    \param lopn  U_LOGPEN structure describing this pen
*/
char *U_EMRCREATEPEN_set(
      const uint32_t ihPen,
      const U_LOGPEN lopn
   ){
   char *record;
   int irecsize=sizeof(U_EMRCREATEPEN);

   record   = malloc(irecsize);    
   if(record){
      ((PU_EMR)          record)->iType = U_EMR_CREATEPEN;
      ((PU_EMR)          record)->nSize = irecsize;
      ((PU_EMRCREATEPEN) record)->ihPen = ihPen;
      ((PU_EMRCREATEPEN) record)->lopn  = lopn;
   }
   return(record);
}

// U_EMRCREATEBRUSHINDIRECT_set       39
/**
    \brief Allocate and construct a U_EMR_CREATEBRUSHINDIRECT record.
    Use createbrushindirect_set() instead of calling this function directly.
    \return pointer to U_EMR_CREATEBRUSHINDIRECT record, or NULL on error.
    \param ihBrush Index to place object in EMF object table (this entry must not yet exist) 
    \param lb      Brush properties                                                          
*/
char *U_EMRCREATEBRUSHINDIRECT_set(
      const uint32_t   ihBrush,
      const U_LOGBRUSH lb
   ){
   char *record;
   int  irecsize;

   irecsize = sizeof(U_EMRCREATEBRUSHINDIRECT);
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)                    record)->iType   = U_EMR_CREATEBRUSHINDIRECT;
      ((PU_EMR)                    record)->nSize   = irecsize;
      ((PU_EMRCREATEBRUSHINDIRECT) record)->ihBrush = ihBrush;  // Index to place object in EMF object table (this entry must not yet exist)
      ((PU_EMRCREATEBRUSHINDIRECT) record)->lb      = lb;
   }
   return(record);
} 

// U_EMRDELETEOBJECT_set              40
/** 
    \brief Allocate and construct a U_EMR_DELETEOBJECT record.
    Use deleteobject_set() instead of calling this function directly.
    \return pointer to U_EMR_DELETEOBJECT record, or NULL on error.
    \param ihObject Number of a stock or created object
*/
char *U_EMRDELETEOBJECT_set(
      const uint32_t ihObject
   ){
   char *record;
   int  irecsize;

   irecsize = sizeof(U_EMRDELETEOBJECT);
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)             record)->iType    = U_EMR_DELETEOBJECT;
      ((PU_EMR)             record)->nSize    = irecsize;
      ((PU_EMRDELETEOBJECT) record)->ihObject = ihObject;  // Index of object to DELETE
   }
   return(record);
} 

// U_EMRANGLEARC_set                  41
/**
    \brief Allocate and construct a U_EMR_ANGLEARC record.
    \return pointer to U_EMR_ANGLEARC record, or NULL on error.
    \param ptlCenter   Center in logical units
    \param nRadius     Radius in logical units
    \param eStartAngle Starting angle in degrees (counter clockwise from x axis)
    \param eSweepAngle Sweep angle in degrees
*/
char *U_EMRANGLEARC_set(
      const U_POINTL  ptlCenter,
      const uint32_t  nRadius,
      const U_FLOAT   eStartAngle,
      const U_FLOAT   eSweepAngle
   ){
   char *record;
   int  irecsize;

   irecsize = sizeof(U_EMRANGLEARC);
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)         record)->iType       = U_EMR_ANGLEARC;
      ((PU_EMR)         record)->nSize       = irecsize;
      ((PU_EMRANGLEARC) record)->ptlCenter   = ptlCenter;   
      ((PU_EMRANGLEARC) record)->nRadius     = nRadius;     
      ((PU_EMRANGLEARC) record)->eStartAngle = eStartAngle; 
      ((PU_EMRANGLEARC) record)->eSweepAngle = eSweepAngle; 
   }
   return(record);
} 

// U_EMRELLIPSE_set                   42
/**
    \brief Allocate and construct a U_EMR_ELLIPSE record.
    \return pointer to U_EMR_ELLIPSE record, or NULL on error.
    \param rclBox bounding rectangle in logical units
*/
char *U_EMRELLIPSE_set(
      const U_RECTL rclBox
   ){
   return(U_EMR_CORE4_set(U_EMR_ELLIPSE,rclBox));
} 

// U_EMRRECTANGLE_set                 43
/**
    \brief Allocate and construct a U_EMR_RECTANGLE record.
    \return pointer to U_EMR_RECTANGLE record, or NULL on error.
    \param rclBox bounding rectangle in logical units
*/
char *U_EMRRECTANGLE_set(
      const U_RECTL rclBox
   ){
   return(U_EMR_CORE4_set(U_EMR_RECTANGLE,rclBox));
} 

// U_EMRROUNDRECT_set                 44
/**
    \brief Allocate and construct a U_EMR_ROUNDRECT record.
    \return pointer to U_EMR_ROUNDRECT record, or NULL on error.
    \param rclBox    bounding rectangle in logical units
    \param szlCorner W & H in logical units of ellipse used to round corner
*/
char *U_EMRROUNDRECT_set(
      const U_RECTL rclBox,
      const U_SIZEL szlCorner
   ){
   char *record;
   int  irecsize;

   irecsize = sizeof(U_EMRROUNDRECT);
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)          record)->iType       = U_EMR_ROUNDRECT;
      ((PU_EMR)          record)->nSize       = irecsize;
      ((PU_EMRROUNDRECT) record)->rclBox      = rclBox;  
      ((PU_EMRROUNDRECT) record)->szlCorner   = szlCorner;
   }
   return(record);
} 

// U_EMRARC_set                       45
/**
    \brief Allocate and construct a U_EMR_ARC record.
    \return pointer to U_EMR_ARC record, or NULL on error.
    \param rclBox   bounding rectangle in logical units
    \param ptlStart Start point in logical units
    \param ptlEnd   End point in logical units
*/
char *U_EMRARC_set(
      const U_RECTL  rclBox,
      const U_POINTL ptlStart,
      const U_POINTL ptlEnd
   ){
   return(U_EMR_CORE9_set(U_EMR_ARC,rclBox, ptlStart, ptlEnd));
}

// U_EMRCHORD_set                     46
/**
    \brief Allocate and construct a U_EMR_CHORD record.
    \return pointer to U_EMR_CHORD record, or NULL on error.
    \param rclBox   bounding rectangle in logical units
    \param ptlStart Start point in logical units
    \param ptlEnd   End point in logical units
*/
char *U_EMRCHORD_set(
      const U_RECTL  rclBox,
      const U_POINTL ptlStart,
      const U_POINTL ptlEnd
   ){
   return(U_EMR_CORE9_set(U_EMR_CHORD,rclBox, ptlStart, ptlEnd));
}

// U_EMRPIE_set                       47
/**
    \brief Allocate and construct a U_EMR_PIE record.
    \return pointer to U_EMR_PIE record, or NULL on error.
    \param rclBox   bounding rectangle in logical units
    \param ptlStart Start point in logical units
    \param ptlEnd   End point in logical units
*/
char *U_EMRPIE_set(
      const U_RECTL  rclBox,
      const U_POINTL ptlStart,
      const U_POINTL ptlEnd
   ){
   return(U_EMR_CORE9_set(U_EMR_PIE,rclBox, ptlStart, ptlEnd));
}

// U_EMRSELECTPALETTE_set             48
/**
    \brief Allocate and construct a U_EMR_SELECTPALETTE record.
    \return pointer to U_EMR_SELECTPALETTE record, or NULL on error.
    \param ihPal Index of a Palette object in the EMF object table
*/
char *U_EMRSELECTPALETTE_set(
      const uint32_t ihPal
   ){
   return(U_EMR_CORE3_set(U_EMR_SELECTPALETTE, ihPal));
}

// U_EMRCREATEPALETTE_set             49
/**
    \brief Allocate and construct a U_EMR_CREATEPALETTE record.
    Use createpalette_set() instead of calling this function directly.
    \return pointer to U_EMR_CREATEPALETTE record, or NULL on error.
    \param ihPal Index to place object in EMF object table (this entry must not yet exist)
    \param lgpl   Palette properties
*/
char *U_EMRCREATEPALETTE_set(
      const uint32_t     ihPal,
      const U_LOGPALETTE lgpl
   ){
   char *record;
   int  irecsize;

   irecsize = sizeof(U_EMRCREATEPALETTE);
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)              record)->iType   = U_EMR_CREATEPALETTE;
      ((PU_EMR)              record)->nSize   = irecsize;
      ((PU_EMRCREATEPALETTE) record)->ihPal   = ihPal;
      ((PU_EMRCREATEPALETTE) record)->lgpl    = lgpl;
   }
   return(record);
}

// U_EMRSETPALETTEENTRIES_set         50
/**
    \brief Allocate and construct a U_EMR_SETPALETTEENTRIES record.
    Use setpaletteentries_set() instead of calling this function directly.
    \return pointer to U_EMR_SETPALETTEENTRIES record, or NULL on error.
    \param ihPal        Index of a Palette object in the EMF object table
    \param iStart       First Palette entry in selected object to set
    \param cEntries     Number of Palette entries in selected object to set
    \param aPalEntries  Values to set with
*/
char *U_EMRSETPALETTEENTRIES_set(
      const uint32_t         ihPal,
      const uint32_t         iStart,
      const U_NUM_LOGPLTNTRY cEntries,
      const PU_LOGPLTNTRY    aPalEntries
   ){
   char *record;
   int  irecsize;
   int  cbPals;

   if(!aPalEntries)return(NULL);
   cbPals    = cEntries * sizeof(U_LOGPLTNTRY);
   irecsize = sizeof(U_EMRSETPALETTEENTRIES) + cbPals - sizeof(U_LOGPLTNTRY);
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)                  record)->iType      = U_EMR_SETPALETTEENTRIES;
      ((PU_EMR)                  record)->nSize      = irecsize;
      ((PU_EMRSETPALETTEENTRIES) record)->ihPal      = ihPal;
      ((PU_EMRSETPALETTEENTRIES) record)->iStart     = iStart;
      ((PU_EMRSETPALETTEENTRIES) record)->cEntries   = cEntries;
      memcpy(((PU_EMRSETPALETTEENTRIES) record)->aPalEntries, aPalEntries,cbPals);
   }
   return(record);
}

// U_EMRRESIZEPALETTE_set             51
/**
    \brief Allocate and construct a U_EMR_RESIZEPALETTE record.
    \return pointer to U_EMR_RESIZEPALETTE record, or NULL on error.
    \param ihPal    Index of a Palette object in the EMF object table
    \param cEntries Number to expand or truncate the Palette entry list to
*/
char *U_EMRRESIZEPALETTE_set(
      const uint32_t ihPal,
      const uint32_t cEntries
   ){
   return(U_EMR_CORE7_set(U_EMR_RESIZEPALETTE, (U_PAIR){ihPal,cEntries}));
}

// U_EMRREALIZEPALETTE_set            52
/**
    \brief Allocate and construct a U_EMR_REALIZEPALETTE record.
    \return pointer to U_EMR_REALIZEPALETTE record, or NULL on error.
*/
char *U_EMRREALIZEPALETTE_set(void){
  return(U_EMR_CORE5_set(U_EMR_REALIZEPALETTE));
}

// U_EMREXTFLOODFILL_set              53
/**
    \brief Allocate and construct a U_EMR_EXTFLOODFILL record.
    \return pointer to U_EMR_EXTFLOODFILL record, or NULL on error.
    \param ptlStart Start point in logical units
    \param crColor  Color to fill with
    \param iMode    FloodFill Enumeration
*/
char *U_EMREXTFLOODFILL_set(
      const U_POINTL   ptlStart,
      const U_COLORREF crColor,
      const uint32_t   iMode
   ){
   char *record;
   int  irecsize;

   irecsize = sizeof(U_EMREXTFLOODFILL);
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)             record)->iType    = U_EMR_EXTFLOODFILL; 
      ((PU_EMR)             record)->nSize    = irecsize;           
      ((PU_EMREXTFLOODFILL) record)->ptlStart = ptlStart;           
      ((PU_EMREXTFLOODFILL) record)->crColor  = crColor;            
      ((PU_EMREXTFLOODFILL) record)->iMode    = iMode;              
   }
   return(record);
}

// U_EMRLINETO_set                    54
/**
    \brief Allocate and construct a U_EMR_LINETO record.
    \return pointer to U_EMR_LINETO record, or NULL on error.
    \param ptl Point coordinates
*/
char *U_EMRLINETO_set(
      const U_POINTL ptl
   ){
   return(U_EMR_CORE7_set(U_EMR_LINETO, ptl));
}

// U_EMRARCTO_set                     55
/**
    \brief Allocate and construct a U_EMR_ARCTO record.
    \return pointer to U_EMR_ARCTO record, or NULL on error.
    \param rclBox   bounding rectangle in logical units
    \param ptlStart Start point in logical units
    \param ptlEnd   End point in logical units
    
    Note that the draw begins with a line from the current point to ptlStart, which is
    not indicated in the Microsoft EMF documentation for this record.
*/
char *U_EMRARCTO_set(
      U_RECTL             rclBox,
      U_POINTL            ptlStart,
      U_POINTL            ptlEnd
   ){
   return(U_EMR_CORE9_set(U_EMR_ARCTO,rclBox, ptlStart, ptlEnd));
}

// U_EMRPOLYDRAW_set                  56
/**
    \brief Allocate and construct a U_EMR_POLYDRAW record.
    \return pointer to U_EMR_POLYDRAW record, or NULL on error.
    \param rclBounds Bounding rectangle in device units
    \param cptl      Number of U_POINTL objects
    \param aptl      Array of U_POINTL objects
    \param abTypes   Array of Point Enumeration 
*/
char *U_EMRPOLYDRAW_set(
      const U_RECTL       rclBounds,
      const U_NUM_POINTL  cptl,
      const U_POINTL     *aptl,
      const uint8_t      *abTypes
   ){
   char *record;
   int  irecsize;
   int  cbPoints, cbAbTypes, cbAbTypes4, off;

   if(!cptl || !aptl || !abTypes)return(NULL);
   cbPoints    = cptl * sizeof(U_POINTL);    // space for aptl
   cbAbTypes    = cptl;                       // number of abTypes (same array size, 1 byte each)
   cbAbTypes4    = UP4(cbAbTypes);                // space for abTypes
   irecsize = sizeof(U_EMRPOLYDRAW) + cbPoints + cbAbTypes4 - sizeof(U_POINTL) - 1;
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)         record)->iType       = U_EMR_POLYDRAW;
      ((PU_EMR)         record)->nSize       = irecsize;
      ((PU_EMRPOLYDRAW) record)->rclBounds   = rclBounds;
      ((PU_EMRPOLYDRAW) record)->cptl        = cptl;
      off = sizeof(U_EMR) + sizeof(U_RECTL) + sizeof(uint32_t);  // offset to first variable part
      memcpy(record+off,aptl,cbPoints);
      off += cbPoints;
      memcpy(record+off,abTypes,cbAbTypes);
      off += cbAbTypes;
      if(cbAbTypes4 > cbAbTypes){ memset(record+off,0,cbAbTypes4-cbAbTypes); } // keeps valgrind happy (initialize padding after byte array)
   }
   return(record);
}   

// U_EMRSETARCDIRECTION_set           57
/**
    \brief Allocate and construct a U_EMR_SETARCDIRECTION record.
    \return pointer to U_EMR_SETARCDIRECTION record, or NULL on error.
    \param iArcDirection ArcDirection Enumeration
*/
char *U_EMRSETARCDIRECTION_set(
      const uint32_t iArcDirection
   ){
   return(U_EMR_CORE3_set(U_EMR_SETARCDIRECTION, iArcDirection));
}

// U_EMRSETMITERLIMIT_set             58
/**
    \brief Allocate and construct a U_EMR_SETMITERLIMIT record.
    \return pointer to U_EMR_SETMITERLIMIT record, or NULL on error.
    \param eMiterLimit MapMode Enumeration
*/
char *U_EMRSETMITERLIMIT_set(
      const uint32_t eMiterLimit
   ){
   return(U_EMR_CORE3_set(U_EMR_SETMITERLIMIT, eMiterLimit));
}


// U_EMRBEGINPATH_set                 59
/**
    \brief Allocate and construct a U_EMR_BEGINPATH record.
    \return pointer to U_EMR_BEGINPATH record, or NULL on error.
*/
char *U_EMRBEGINPATH_set(void){
   return(U_EMR_CORE5_set(U_EMR_BEGINPATH));
}

// U_EMRENDPATH_set                   60
/**
    \brief Allocate and construct a U_EMR_ENDPATH record.
    \return pointer to U_EMR_ENDPATH record, or NULL on error.
*/
char *U_EMRENDPATH_set(void){
   return(U_EMR_CORE5_set(U_EMR_ENDPATH));
}

// U_EMRCLOSEFIGURE_set               61
/**
    \brief Allocate and construct a U_EMR_CLOSEFIGURE record.
    \return pointer to U_EMR_CLOSEFIGURE record, or NULL on error.
*/
char *U_EMRCLOSEFIGURE_set(void){
   return(U_EMR_CORE5_set(U_EMR_CLOSEFIGURE));
}

// U_EMRFILLPATH_set                  62
/**
    \brief Allocate and construct a U_EMR_FILLPATH record.
    \return pointer to U_EMR_FILLPATH record, or NULL on error.
    \param rclBox Bounding rectangle in device units
    
    U_EMR_FILLPATH closes the open figure before filling.
*/
char *U_EMRFILLPATH_set(
      const U_RECTL rclBox
   ){
   return(U_EMR_CORE4_set(U_EMR_FILLPATH,rclBox));
} 

// U_EMRSTROKEANDFILLPATH_set         63
/**
    \brief Allocate and construct a U_EMR_STROKEANDFILLPATH record.
    \return pointer to U_EMR_STROKEANDFILLPATH record, or NULL on error.
    \param rclBox Bounding rectangle in device units

    U_EMR_STROKEANDFILLPATH closes the open figure before filling and stroking.
    There appears to be no way to fill an open path while stroking it, as any one
    of U_EMRFILLPATH, U_EMRSTROKEPATH, or U_EMRSTROKEANDFILEPATH will "use up" the path,
*/
char *U_EMRSTROKEANDFILLPATH_set(
      const U_RECTL rclBox
   ){
   return(U_EMR_CORE4_set(U_EMR_STROKEANDFILLPATH,rclBox));
} 

// U_EMRSTROKEPATH_set                64
/**
    \brief Allocate and construct a U_EMR_STROKEPATH record.
    \return pointer to U_EMR_STROKEPATH record, or NULL on error.
    \param rclBox Bounding rectangle in device units

    U_EMR_STROKEPATH does NOT close the open figure before stroking it.
*/
char *U_EMRSTROKEPATH_set(
      const U_RECTL rclBox
   ){
   return(U_EMR_CORE4_set(U_EMR_STROKEPATH,rclBox));
} 

// U_EMRFLATTENPATH_set               65
/**
    \brief Allocate and construct a U_EMR_FLATTENPATH record.
    \return pointer to U_EMR_FLATTENPATH record, or NULL on error.
*/
char *U_EMRFLATTENPATH_set(void){
   return(U_EMR_CORE5_set(U_EMR_FLATTENPATH));
}

// U_EMRWIDENPATH_set                 66
/**
    \brief Allocate and construct a U_EMR_WIDENPATH record.
    \return pointer to U_EMR_WIDENPATH record, or NULL on error.
*/
char *U_EMRWIDENPATH_set(void){
   return(U_EMR_CORE5_set(U_EMR_WIDENPATH));
}

// U_EMRSELECTCLIPPATH_set            67
/**
    \brief Allocate and construct a U_EMR_SELECTCLIPPATH record.
    \return pointer to U_EMR_SELECTCLIPPATH record, or NULL on error.
    \param iMode RegionMode Enumeration
*/
char *U_EMRSELECTCLIPPATH_set(
      const uint32_t iMode
   ){ 
   return(U_EMR_CORE3_set(U_EMR_SELECTCLIPPATH, iMode));
}

// U_EMRABORTPATH_set                 68
/**
    \brief Allocate and construct a U_EMR_ABORTPATH record.
    \return pointer to U_EMR_ABORTPATH record, or NULL on error.
*/
char *U_EMRABORTPATH_set(void){
   return(U_EMR_CORE5_set(U_EMR_ABORTPATH));
}

// U_EMRUNDEF69                       69

// U_EMRCOMMENT_set                   70  Comment (any binary data, interpretation is program specific)
/**
    \brief Allocate and construct a U_EMR_COMMENT record.
    \return pointer to U_EMR_COMMENT record, or NULL on error.
    \param cbData Number of bytes in comment
    \param Data   Comment (any binary data, interpretation is program specific)
*/
char *U_EMRCOMMENT_set(
      const U_CBDATA cbData,
      const char    *Data
   ){
   char *record;
   unsigned int   cbData4;
   int   irecsize;

   cbData4    = UP4(cbData);
   irecsize = sizeof(U_EMR) + sizeof(U_CBDATA) + cbData4;
   record    = malloc(irecsize);
   if(record){
      ((PU_EMR)           record)->iType   = U_EMR_COMMENT;
      ((PU_EMR)           record)->nSize   = irecsize;
      ((PU_EMRCOMMENT)    record)->cbData  = cbData;
      memcpy(record + irecsize - cbData4,Data,cbData);
      if(cbData4 > cbData)memset(record + irecsize - cbData4 + cbData,0,cbData4-cbData);  // clear any unused bytes
   }
   return(record);
} 

// U_EMRFILLRGN_set                   71
/**
    \brief Allocate and construct a U_EMR_FILLRGN record.
    Use fillrgn_set() instead of calling this function directly.
    \return pointer to U_EMR_FILLRGN record, or NULL on error.
    \param rclBounds  Bounding rectangle in device units
    \param ihBrush    Index of a Brush object in the EMF object table
    \param RgnData    Pointer to a U_RGNDATA structure
*/
char *U_EMRFILLRGN_set(
      const U_RECTL     rclBounds,
      const uint32_t    ihBrush,
      const PU_RGNDATA  RgnData
   ){
   char *record;
   int   irecsize;
   int   cbRgns,cbRgns4,rds,rds4,off;

   if(!RgnData)return(NULL);
   cbRgns   = ((PU_RGNDATAHEADER) RgnData)->nRgnSize;
   cbRgns4  = UP4(cbRgns);
   rds      = sizeof(U_RGNDATAHEADER) + cbRgns;
   rds4     = UP4(rds);
   irecsize = sizeof(U_EMRFILLRGN) - sizeof(U_RECTL) + cbRgns4;  // core + array - overlap of one rectL
   record    = malloc(irecsize);
   if(record){
      ((PU_EMR)           record)->iType     = U_EMR_FILLRGN;
      ((PU_EMR)           record)->nSize     = irecsize;
      ((PU_EMRFILLRGN)    record)->rclBounds = rclBounds;
      ((PU_EMRFILLRGN)    record)->cbRgnData = rds;
      ((PU_EMRFILLRGN)    record)->ihBrush   = ihBrush;
      off = sizeof(U_EMRFILLRGN) - sizeof(U_RGNDATA);
      memcpy(record + off, RgnData, rds);
      off += rds;
      if(rds < rds4){ memset(record + off,0, rds4 - rds); } // clear any unused bytes
   }
   return(record);
} 

// U_EMRFRAMERGN_set                  72
/**
    \brief Allocate and construct a U_EMR_FRAMERGN record.
    Use framegrn_set() instead of calling this function directly.
    \return pointer to U_EMR_FRAMERGN record, or NULL on error.
    \param rclBounds  Bounding rectangle in device units
    \param ihBrush    Index of a Brush object in the EMF object table
    \param szlStroke  W & H of Brush stroke
    \param RgnData    Pointer to a U_RGNDATA structure
*/
char *U_EMRFRAMERGN_set(
      const U_RECTL     rclBounds,
      const uint32_t    ihBrush,
      const U_SIZEL     szlStroke,
      const PU_RGNDATA  RgnData
   ){
   char *record;
   int   irecsize;
   int   cbRgns,cbRgns4,rds,rds4,off;

   if(!RgnData)return(NULL);
   cbRgns   = ((PU_RGNDATAHEADER) RgnData)->nRgnSize;
   cbRgns4  = UP4(cbRgns);
   rds      = sizeof(U_RGNDATAHEADER) + cbRgns;
   rds4     = UP4(rds);
   irecsize = sizeof(U_EMRFRAMERGN) - sizeof(U_RECTL) + cbRgns4;  // core + array - overlap of one rectL
   record    = malloc(irecsize);
   if(record){
      ((PU_EMR)           record)->iType     = U_EMR_FRAMERGN;
      ((PU_EMR)           record)->nSize     = irecsize;
      ((PU_EMRFRAMERGN)   record)->rclBounds = rclBounds;
      ((PU_EMRFRAMERGN)   record)->cbRgnData = rds;
      ((PU_EMRFRAMERGN)   record)->ihBrush   = ihBrush;
      ((PU_EMRFRAMERGN)   record)->szlStroke = szlStroke;
      off = sizeof(U_EMRFRAMERGN) - sizeof(U_RGNDATA);
      memcpy(record + off, RgnData, rds);
      off += rds;
      if(rds < rds4){ memset(record + off,0, rds4 - rds); } // clear any unused bytes
   }
   return(record);
} 

// U_EMRINVERTRGN_set                 73
/**
    \brief Allocate and construct a U_EMR_INVERTRGN record.
    \return pointer to U_EMR_INVERTRGN record, or NULL on error.
    \param RgnData Variable size U_RGNDATA structure
*/
char *U_EMRINVERTRGN_set(
      const PU_RGNDATA RgnData
   ){
   return(U_EMR_CORE11_set(U_EMR_INVERTRGN, RgnData));
} 

// U_EMRPAINTRGN_set                  74
/**
    \brief Allocate and construct a U_EMR_PAINTRGN record.
    \return pointer to U_EMR_PAINTRGN record, or NULL on error.
    \param RgnData Variable size U_RGNDATA structure
*/
char *U_EMRPAINTRGN_set(
      const PU_RGNDATA RgnData
   ){
   return(U_EMR_CORE11_set(U_EMR_PAINTRGN, RgnData));
} 

// U_EMREXTSELECTCLIPRGN_set          75
/**
    \brief Allocate and construct a U_EMR_EXTSELECTCLIPRGN record.
    \return pointer to U_EMR_EXTSELECTCLIPRGN or NULL on error.
    \param iMode   RegionMode Enumeration       
    \param RgnData Variable size U_RGNDATA structure
*/
char *U_EMREXTSELECTCLIPRGN_set(
      const uint32_t    iMode,
      const PU_RGNDATA  RgnData
   ){
   char *record;
   int   irecsize;
   int   cbRgns,cbRgns4,rds,rds4,off;

   if(!RgnData)return(NULL);
   cbRgns   = ((PU_RGNDATAHEADER) RgnData)->nRgnSize;
   cbRgns4  = UP4(cbRgns);
   rds      = sizeof(U_RGNDATAHEADER) + cbRgns;
   rds4     = UP4(rds);
   irecsize = sizeof(U_EMREXTSELECTCLIPRGN) - sizeof(U_RECTL) + cbRgns4;  // core + array - overlap of one rectL
   record    = malloc(irecsize);
   if(record){
      ((PU_EMR)                  record)->iType     = U_EMR_EXTSELECTCLIPRGN;
      ((PU_EMR)                  record)->nSize     = irecsize;
      ((PU_EMREXTSELECTCLIPRGN)  record)->cbRgnData = rds;
      ((PU_EMREXTSELECTCLIPRGN)  record)->iMode     = iMode;
      off = sizeof(U_EMREXTSELECTCLIPRGN) - sizeof(U_RGNDATA);
      memcpy(record + off, RgnData, rds);
      off += rds;
      if(rds < rds4){ memset(record + off,0, rds4 - rds); } // clear any unused bytes
   }
   return(record);
} 

// U_EMRBITBLT_set                    76
/**
    \brief Allocate and construct a U_EMR_BITBLT record.
    \return pointer to U_EMR_BITBLT record, or NULL on error.
    \param rclBounds    Bounding rectangle in device units
    \param Dest         Destination UL corner in logical units
    \param cDest        Destination width in logical units
    \param Src          Source rectangle UL corner in logical units
    \param xformSrc     Source bitmap transform (world to page coordinates)
    \param crBkColorSrc Source bitmap background color
    \param iUsageSrc    DIBcolors Enumeration
    \param dwRop        Ternary Raster Operation enumeration
    \param Bmi          (Optional) bitmapbuffer (U_BITMAPINFO section)
    \param cbPx         Size in bytes of pixel array (row stride * height, there may be some padding at the end of each row)
    \param Px           (Optional) bitmapbuffer (pixel array section )
*/
char *U_EMRBITBLT_set(
      const U_RECTL        rclBounds,
      const U_POINTL       Dest,
      const U_POINTL       cDest,
      const U_POINTL       Src,
      const U_XFORM        xformSrc,
      const U_COLORREF     crBkColorSrc,
      const uint32_t       iUsageSrc,
      const uint32_t       dwRop,
      const PU_BITMAPINFO  Bmi,
      const uint32_t       cbPx,
      char                *Px
   ){
   char *record;
   int   irecsize;
   int   cbImage,cbImage4,cbBmi,off;

   SET_CB_FROM_PXBMI(Px,Bmi,cbImage,cbImage4,cbBmi,cbPx);
   irecsize = sizeof(U_EMRBITBLT) + cbBmi + cbImage4;
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)             record)->iType        = U_EMR_BITBLT;
      ((PU_EMR)             record)->nSize        = irecsize;
      ((PU_EMRBITBLT)       record)->rclBounds    = rclBounds;
      ((PU_EMRBITBLT)       record)->Dest         = Dest;
      ((PU_EMRBITBLT)       record)->cDest        = cDest;
      ((PU_EMRBITBLT)       record)->dwRop        = dwRop;
      ((PU_EMRBITBLT)       record)->Src          = Src;
      ((PU_EMRBITBLT)       record)->xformSrc     = xformSrc;
      ((PU_EMRBITBLT)       record)->crBkColorSrc = crBkColorSrc;
      ((PU_EMRBITBLT)       record)->iUsageSrc    = iUsageSrc;
      off = sizeof(U_EMRBITBLT);
      APPEND_PXBMISRC(record, U_EMRBITBLT, cbBmi, Bmi, Px, cbImage, cbImage4);
   }
   return(record);
}

// U_EMRSTRETCHBLT_set                77
/**
    \brief Allocate and construct a U_EMR_STRETCHBLT record.
    \return pointer to U_EMR_STRETCHBLT record, or NULL on error.
    \param rclBounds    Bounding rectangle in device units
    \param Dest         Destination UL corner in logical units
    \param cDest        Destination width in logical units
    \param Src          Source UL corner in logical units
    \param cSrc         Src W & H in logical units
    \param xformSrc     Transform to apply to source
    \param crBkColorSrc Background color
    \param iUsageSrc    DIBcolors Enumeration
    \param dwRop        Ternary Raster Operation enumeration
    \param Bmi          (Optional) bitmapbuffer (U_BITMAPINFO section)
    \param cbPx         Size in bytes of pixel array (row stride * height, there may be some padding at the end of each row)
    \param Px           (Optional) bitmapbuffer (pixel array section )
*/
char *U_EMRSTRETCHBLT_set(
      const U_RECTL        rclBounds,
      const U_POINTL       Dest,
      const U_POINTL       cDest,
      const U_POINTL       Src,
      const U_POINTL       cSrc,
      const U_XFORM        xformSrc,
      const U_COLORREF     crBkColorSrc,
      const uint32_t       iUsageSrc,
      const uint32_t       dwRop,
      const PU_BITMAPINFO  Bmi,
      const uint32_t       cbPx,
      char                *Px
   ){
   char *record;
   int   irecsize;
   int   cbImage,cbImage4,cbBmi,off;

   SET_CB_FROM_PXBMI(Px,Bmi,cbImage,cbImage4,cbBmi,cbPx);
   
   irecsize = sizeof(U_EMRSTRETCHBLT) + cbBmi + cbImage4;
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)                 record)->iType        = U_EMR_STRETCHBLT;
      ((PU_EMR)                 record)->nSize        = irecsize;
      ((PU_EMRSTRETCHBLT)       record)->rclBounds    = rclBounds;
      ((PU_EMRSTRETCHBLT)       record)->Dest         = Dest;
      ((PU_EMRSTRETCHBLT)       record)->cDest        = cDest;
      ((PU_EMRSTRETCHBLT)       record)->dwRop        = dwRop;
      ((PU_EMRSTRETCHBLT)       record)->Src          = Src;
      ((PU_EMRSTRETCHBLT)       record)->xformSrc     = xformSrc;
      ((PU_EMRSTRETCHBLT)       record)->crBkColorSrc = crBkColorSrc;
      ((PU_EMRSTRETCHBLT)       record)->iUsageSrc    = iUsageSrc;
      off = sizeof(U_EMRSTRETCHBLT);
      APPEND_PXBMISRC(record, U_EMRSTRETCHBLT, cbBmi, Bmi, Px, cbImage, cbImage4);
      ((PU_EMRSTRETCHBLT)       record)->cSrc         = cSrc;
   }
   return(record);
}

// U_EMRMASKBLT_set                   78
/**
    \brief Allocate and construct a U_EMR_MASKBLT record.
    \return pointer to U_EMR_MASKBLT record, or NULL on error.
    \param rclBounds    Bounding rectangle in device units
    \param Dest         Destination UL corner in logical units
    \param cDest        Destination width in logical units
    \param Src          Source UL corner in logical units
    \param xformSrc     Transform to apply to source
    \param crBkColorSrc Background color
    \param iUsageSrc    DIBcolors Enumeration
    \param Mask         Mask UL corner in logical units
    \param iUsageMask   DIBcolors Enumeration
    \param dwRop        Ternary Raster Operation enumeration
    \param Bmi          (Optional) bitmapbuffer (U_BITMAPINFO section)
    \param cbPx         Size in bytes of pixel array (row stride * height, there may be some padding at the end of each row)
    \param Px           (Optional) bitmapbuffer (pixel array section )
    \param MskBmi       (Optional) bitmapbuffer (U_BITMAPINFO section)
    \param cbMsk        Size in bytes of mask array (row stride * height, there may be some padding at the end of each row)
    \param Msk          (Optional) bitmapbuffer (mask section )
*/
char *U_EMRMASKBLT_set(
      const U_RECTL        rclBounds,
      const U_POINTL       Dest,
      const U_POINTL       cDest,
      const U_POINTL       Src,
      const U_XFORM        xformSrc,
      const U_COLORREF     crBkColorSrc,
      const uint32_t       iUsageSrc,
      const U_POINTL       Mask,
      const uint32_t       iUsageMask,
      const uint32_t       dwRop,
      const PU_BITMAPINFO  Bmi,
      const uint32_t       cbPx,
      char                *Px,
      const PU_BITMAPINFO  MskBmi,
      const uint32_t       cbMsk,
      char                *Msk
   ){
   char *record;
   int   irecsize;
   int   cbImage,cbImage4,cbBmi,cbMskImage,cbMskImage4,cbMskBmi,off;

   SET_CB_FROM_PXBMI(Px,Bmi,cbImage,cbImage4,cbBmi,cbPx);
   SET_CB_FROM_PXBMI(Msk,MskBmi,cbMskImage,cbMskImage4,cbMskBmi,cbMsk);

   irecsize = sizeof(U_EMRMASKBLT) + cbBmi + cbImage4 + cbMskBmi + cbMskImage4;
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)              record)->iType        = U_EMR_MASKBLT;
      ((PU_EMR)              record)->nSize        = irecsize;
      ((PU_EMRMASKBLT)       record)->rclBounds    = rclBounds;
      ((PU_EMRMASKBLT)       record)->Dest         = Dest;
      ((PU_EMRMASKBLT)       record)->cDest        = cDest;
      ((PU_EMRMASKBLT)       record)->dwRop        = dwRop;
      ((PU_EMRMASKBLT)       record)->Src          = Src;
      ((PU_EMRMASKBLT)       record)->xformSrc     = xformSrc;
      ((PU_EMRMASKBLT)       record)->crBkColorSrc = crBkColorSrc;
      ((PU_EMRMASKBLT)       record)->iUsageSrc    = iUsageSrc;
      ((PU_EMRMASKBLT)       record)->Mask         = Mask;
      ((PU_EMRMASKBLT)       record)->iUsageMask   = iUsageMask;
      off = sizeof(U_EMRMASKBLT);
      APPEND_PXBMISRC(record, U_EMRMASKBLT, cbBmi, Bmi, Px, cbImage, cbImage4);
      APPEND_MSKBMISRC(record, U_EMRMASKBLT, cbMskBmi, MskBmi, Msk, cbMskImage, cbMskImage4);
   }
   return(record);
}

// U_EMRPLGBLT_set                    79

/**
    \brief Allocate and construct a U_EMRPLGBLT record.
    \return U_EMRPLGBLT record.
    \param rclBounds    Bounding rectangle in device units
    \param aptlDst      Defines parallelogram, UL, UR, LL corners, LR is derived (3 points)
    \param Src          Source UL corner in logical units
    \param cSrc         Source width in logical units
    \param xformSrc     Transform to apply to source
    \param crBkColorSrc Background color
    \param iUsageSrc    DIBcolors Enumeration
    \param Mask         Mask UL corner in logical units
    \param iUsageMask   DIBcolors Enumeration
    \param Bmi          (Optional) bitmapbuffer (U_BITMAPINFO section)
    \param cbPx         Size in bytes of pixel array (row stride * height, there may be some padding at the end of each row)
    \param Px           (Optional) bitmapbuffer (pixel array section )
    \param MskBmi       (Optional) bitmapbuffer (U_BITMAPINFO section)
    \param cbMsk        Size in bytes of mask array (row stride * height, there may be some padding at the end of each row)
    \param Msk          (Optional) bitmapbuffer (mask section )
*/
char *U_EMRPLGBLT_set(
      const U_RECTL       rclBounds,
      const PU_POINTL     aptlDst,
      const U_POINTL      Src,
      const U_POINTL      cSrc,
      const U_XFORM       xformSrc,
      const U_COLORREF    crBkColorSrc,
      const uint32_t      iUsageSrc,
      const U_POINTL      Mask,
      const uint32_t      iUsageMask,
      const PU_BITMAPINFO Bmi,
      const uint32_t      cbPx,
      char               *Px,
      const PU_BITMAPINFO MskBmi,
      const uint32_t      cbMsk,
      char               *Msk
   ){
   char *record;
   int   irecsize;
   int   cbImage,cbImage4,cbBmi,cbMskImage,cbMskImage4,cbMskBmi,off;

   SET_CB_FROM_PXBMI(Px,Bmi,cbImage,cbImage4,cbBmi,cbPx);
   SET_CB_FROM_PXBMI(Msk,MskBmi,cbMskImage,cbMskImage4,cbMskBmi,cbMsk);

   irecsize = sizeof(U_EMRPLGBLT) + cbBmi + cbImage4 + cbMskBmi + cbMskImage4;
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)             record)->iType        = U_EMR_PLGBLT;
      ((PU_EMR)             record)->nSize        = irecsize;
      ((PU_EMRPLGBLT)       record)->rclBounds    = rclBounds;
      memcpy(((PU_EMRPLGBLT)  record)->aptlDst,aptlDst,3*sizeof(U_POINTL));
      ((PU_EMRPLGBLT)       record)->Src          = Src;
      ((PU_EMRPLGBLT)       record)->cSrc         = cSrc;
      ((PU_EMRPLGBLT)       record)->xformSrc     = xformSrc;
      ((PU_EMRPLGBLT)       record)->crBkColorSrc = crBkColorSrc;
      ((PU_EMRPLGBLT)       record)->iUsageSrc    = iUsageSrc;
      ((PU_EMRPLGBLT)       record)->Mask         = Mask;
      ((PU_EMRPLGBLT)       record)->iUsageMask   = iUsageMask;
      off = sizeof(U_EMRPLGBLT);
      APPEND_PXBMISRC(record, U_EMRPLGBLT, cbBmi, Bmi, Px, cbImage, cbImage4);
      APPEND_MSKBMISRC(record, U_EMRPLGBLT, cbMskBmi, MskBmi, Msk, cbMskImage, cbMskImage4);
   }
   return(record);
}

// U_EMRSETDIBITSTODEVICE_set         80
/**
    \brief Allocate and construct a U_EMR_SETDIBITSTODEVICE record.
    \return pointer to U_EMR_SETDIBITSTODEVICE record, or NULL on error.
    \param rclBounds  Bounding rectangle in device units
    \param Dest       Destination UL corner in logical units
    \param Src        Source UL corner in logical units
    \param cSrc       Source W & H in logical units
    \param iUsageSrc  DIBColors Enumeration
    \param iStartScan First scan line
    \param cScans     Number of scan lines
    \param Bmi        (Optional) bitmapbuffer (U_BITMAPINFO section)
    \param cbPx       Size in bytes of pixel array (row stride * height, there may be some padding at the end of each row)
    \param Px         (Optional) bitmapbuffer (pixel array section )
*/
char *U_EMRSETDIBITSTODEVICE_set(
      const U_RECTL        rclBounds,
      const U_POINTL       Dest,
      const U_POINTL       Src,
      const U_POINTL       cSrc,
      const uint32_t       iUsageSrc,
      const uint32_t       iStartScan,
      const uint32_t       cScans,
      const PU_BITMAPINFO  Bmi,
      const uint32_t       cbPx,
      char                *Px
   ){
   char *record;
   int   irecsize;
   int   cbImage,cbImage4,cbBmi,off;

   SET_CB_FROM_PXBMI(Px,Bmi,cbImage,cbImage4,cbBmi,cbPx);
  
   irecsize = sizeof(U_EMRSETDIBITSTODEVICE) + cbBmi + cbImage4;
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)                       record)->iType      = U_EMR_SETDIBITSTODEVICE;
      ((PU_EMR)                       record)->nSize      = irecsize;
      ((PU_EMRSETDIBITSTODEVICE)      record)->rclBounds  = rclBounds;
      ((PU_EMRSETDIBITSTODEVICE)      record)->Dest       = Dest;
      ((PU_EMRSETDIBITSTODEVICE)      record)->Src        = Src;
      ((PU_EMRSETDIBITSTODEVICE)      record)->cSrc       = cSrc;
      ((PU_EMRSETDIBITSTODEVICE)      record)->iUsageSrc  = iUsageSrc;
      ((PU_EMRSETDIBITSTODEVICE)      record)->iStartScan = iStartScan;
      ((PU_EMRSETDIBITSTODEVICE)      record)->cScans     = cScans;
      off = sizeof(U_EMRSETDIBITSTODEVICE);
      APPEND_PXBMISRC(record, U_EMRSETDIBITSTODEVICE, cbBmi, Bmi, Px, cbImage, cbImage4);
   }
   return(record);
}

// U_EMRSTRETCHDIBITS_set             81
/**
    \brief Allocate and construct a U_EMR_EMRSTRETCHDIBITS record.
    \return pointer to U_EMR_EMRSTRETCHDIBITS record, or NULL on error.
    \param rclBounds Bounding rectangle in device units
    \param Dest      Destination UL corner in logical units
    \param cDest     Destination W & H in logical units
    \param Src       Source UL corner in logical units
    \param cSrc      Source W & H in logical units
    \param iUsageSrc DIBColors Enumeration
    \param dwRop     RasterOPeration Enumeration
    \param Bmi       (Optional) bitmapbuffer (U_BITMAPINFO section)
    \param cbPx      Size in bytes of pixel array (row STRIDE * height, there may be some padding at the end of each row)
    \param Px        (Optional) bitmapbuffer (pixel array section )
*/
char *U_EMRSTRETCHDIBITS_set(
      const U_RECTL        rclBounds,
      const U_POINTL       Dest,
      const U_POINTL       cDest,
      const U_POINTL       Src,
      const U_POINTL       cSrc,
      const uint32_t       iUsageSrc,
      const uint32_t       dwRop,
      const PU_BITMAPINFO  Bmi,
      const uint32_t       cbPx,
      char                *Px
   ){
   char *record;
   int   irecsize;
   int   cbImage,cbImage4,cbBmi,off;

   SET_CB_FROM_PXBMI(Px,Bmi,cbImage,cbImage4,cbBmi,cbPx);
   
   irecsize = sizeof(U_EMRSTRETCHDIBITS) + cbBmi + cbImage4;
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)                    record)->iType      = U_EMR_STRETCHDIBITS;
      ((PU_EMR)                    record)->nSize      = irecsize;
      ((PU_EMRSTRETCHDIBITS)       record)->rclBounds  = rclBounds;
      ((PU_EMRSTRETCHDIBITS)       record)->Dest       = Dest;
      ((PU_EMRSTRETCHDIBITS)       record)->Src        = Src;
      ((PU_EMRSTRETCHDIBITS)       record)->cSrc       = cSrc;
      ((PU_EMRSTRETCHDIBITS)       record)->iUsageSrc  = iUsageSrc;
      ((PU_EMRSTRETCHDIBITS)       record)->dwRop      = dwRop;
      ((PU_EMRSTRETCHDIBITS)       record)->cDest      = cDest;
      off = sizeof(U_EMRSTRETCHDIBITS);
      APPEND_PXBMISRC(record, U_EMRSTRETCHDIBITS, cbBmi, Bmi, Px, cbImage, cbImage4);
   }
   return(record);
}

// U_EMREXTCREATEFONTINDIRECTW_set    82
/**
    \brief Allocate and construct a U_EMR_EXTCREATEFONTINDIRECTW record.
    Use extcreatefontindirectw_set() instead of calling this function directly.
    \return pointer to U_EMR_EXTCREATEFONTINDIRECTW record, or NULL on error.
    \param ihFont Index of the font in the EMF object table
    \param elf    Font parameters as U_LOGFONT
    \param elfw   Font parameters as U_LOGFONT_PANOSE
*/
char *U_EMREXTCREATEFONTINDIRECTW_set(
      const uint32_t               ihFont,
      const char *                 elf,
      const char *                 elfw
   ){
   char *record;
   const char *cptr;
   int   irecsize;
   int   cbLf,off;

   if((elf && elfw) || (!elf && !elfw))return(NULL);  // ONE only must be passed
   if(elf){ cbLf = sizeof(U_LOGFONT);        cptr = elf;  }
   else {   cbLf = sizeof(U_LOGFONT_PANOSE); cptr = elfw; }

   irecsize = sizeof(U_EMR) + sizeof(uint32_t) + cbLf;
   record    = malloc(irecsize);
   if(record){
      ((PU_EMR)                       record)->iType  = U_EMR_EXTCREATEFONTINDIRECTW;
      ((PU_EMR)                       record)->nSize  = irecsize;
      ((PU_EMREXTCREATEFONTINDIRECTW) record)->ihFont = ihFont;
      off = sizeof(U_EMR) + sizeof(uint32_t);
      memcpy(record + off, cptr, cbLf);    // No need to add padding for either structure
   }
   return(record);
}

// U_EMREXTTEXTOUTA_set               83
/**
    \brief Allocate and construct a U_EMR_EXTTEXTOUTA record.
    \return pointer to U_EMR_EXTTEXTOUTA record, or NULL on error.
    \param rclBounds     Bounding rectangle in device units
    \param iGraphicsMode Graphics mode Enumeration
    \param exScale       scale to 0.01 mm units ( only if iGraphicsMode & GM_COMPATIBLE)
    \param eyScale       scale to 0.01 mm units ( only if iGraphicsMode & GM_COMPATIBLE)
    \param emrtext       Text parameters
*/
char *U_EMREXTTEXTOUTA_set(
      const U_RECTL     rclBounds,
      const uint32_t    iGraphicsMode,
      const U_FLOAT     exScale,
      const U_FLOAT     eyScale,
      const PU_EMRTEXT  emrtext
   ){
   return(U_EMR_CORE8_set(U_EMR_EXTTEXTOUTA,rclBounds, iGraphicsMode, exScale, eyScale,emrtext));
}

// U_EMREXTTEXTOUTW_set               84
/**
    \brief Allocate and construct a U_EMR_EXTTEXTOUTW record.
    \return pointer to U_EMR_EXTTEXTOUTW record, or NULL on error.
    \param rclBounds     Bounding rectangle in device units
    \param iGraphicsMode Graphics mode Enumeration
    \param exScale       scale to 0.01 mm units ( only if iGraphicsMode & GM_COMPATIBLE)
    \param eyScale       scale to 0.01 mm units ( only if iGraphicsMode & GM_COMPATIBLE)
    \param emrtext       Text parameters
*/
char *U_EMREXTTEXTOUTW_set(
      const U_RECTL    rclBounds,
      const uint32_t   iGraphicsMode,
      const U_FLOAT    exScale,
      const U_FLOAT    eyScale,
      const PU_EMRTEXT emrtext
   ){
   return(U_EMR_CORE8_set(U_EMR_EXTTEXTOUTW,rclBounds, iGraphicsMode, exScale, eyScale,emrtext));
}

// U_EMRPOLYBEZIER16_set              85
/**
    \brief Allocate and construct a U_EMR_POLYBEZIER16 record.
    \return pointer to U_EMR_POLYBEZIER16 record, or NULL on error.
    \param rclBounds Bounding rectangle in device units
    \param cpts      Number of POINT16 in array
    \param points    Array of POINT16
*/
char *U_EMRPOLYBEZIER16_set(
      const U_RECTL    rclBounds,
      const uint32_t   cpts,
      const U_POINT16 *points
   ){
   return(U_EMR_CORE6_set(U_EMR_POLYBEZIER16, rclBounds, cpts, points));
} 

// U_EMRPOLYGON16_set                 86
/**
    \brief Allocate and construct a U_EMR_POLYGON16 record.
    \return pointer to U_EMR_POLYGON16 record, or NULL on error.
    \param rclBounds Bounding rectangle in device units
    \param cpts      Number of POINT16 in array
    \param points    Array of POINT16
*/
char *U_EMRPOLYGON16_set(
      const U_RECTL    rclBounds,
      const uint32_t   cpts,
      const U_POINT16 *points
   ){
   return(U_EMR_CORE6_set(U_EMR_POLYGON16, rclBounds, cpts, points));
} 

// U_EMRPOLYLINE16_set                87
/**
    \brief Allocate and construct a U_EMR_POLYLINE16 record.
    \return pointer to U_EMR_POLYLINE16 record, or NULL on error.
    \param rclBounds Bounding rectangle in device units
    \param cpts      Number of POINT16 in array
    \param points    Array of POINT16
*/
char *U_EMRPOLYLINE16_set(
      const U_RECTL    rclBounds,
      const uint32_t   cpts,
      const U_POINT16 *points
   ){
   return(U_EMR_CORE6_set(U_EMR_POLYLINE16, rclBounds, cpts, points));
} 

// U_EMRPOLYBEZIERTO16_set            88
/**
    \brief Allocate and construct a U_EMR_POLYBEZIERTO record.
    \return pointer to U_EMR_POLYBEZIERTO record, or NULL on error.
    \param rclBounds Bounding rectangle in device units
    \param cpts      Number of POINT16 in array
    \param points    Array of POINT16
*/
char *U_EMRPOLYBEZIERTO16_set(
      const U_RECTL    rclBounds,
      const uint32_t   cpts,
      const U_POINT16 *points
   ){
   return(U_EMR_CORE6_set(U_EMR_POLYBEZIERTO16, rclBounds, cpts, points));
} 

// U_EMRPOLYLINETO16_set              89
/**
    \brief Allocate and construct a U_EMR_POLYLINETO record.
    \return pointer to U_EMR_POLYLINETO record, or NULL on error.
    \param rclBounds Bounding rectangle in device units
    \param cpts      Number of POINT16 in array
    \param points    Array of POINT16
*/
char *U_EMRPOLYLINETO16_set(
      const U_RECTL    rclBounds,
      const uint32_t   cpts,
      const U_POINT16 *points
   ){
   return(U_EMR_CORE6_set(U_EMR_POLYLINETO16, rclBounds, cpts, points));
} 

// U_EMRPOLYPOLYLINE16_set            90
/**
    \brief Allocate and construct a U_EMR_POLYPOLYLINE16 record.
    \return pointer to U_EMR_POLYPOLYLINE16 record, or NULL on error.
    \param rclBounds   Bounding rectangle in device units
    \param nPolys      Number of elements in aPolyCounts
    \param aPolyCounts Number of points in each poly (sequential)
    \param cpts        Number of POINT16 in array
    \param points      Array of POINT16
*/
char *U_EMRPOLYPOLYLINE16_set(
      const U_RECTL    rclBounds,
      const uint32_t   nPolys,
      const uint32_t  *aPolyCounts,
      const uint32_t   cpts,
      const U_POINT16 *points
   ){
   return(U_EMR_CORE10_set(U_EMR_POLYPOLYLINE16, rclBounds, nPolys, aPolyCounts,cpts, points));
}

// U_EMRPOLYPOLYGON16_set             91
/**
    \brief Allocate and construct a U_EMR_POLYPOLYGON16 record.
    \return pointer to U_EMR_POLYPOLYGON16 record, or NULL on error.
    \param rclBounds   Bounding rectangle in device units
    \param nPolys      Number of elements in aPolyCounts
    \param aPolyCounts Number of points in each poly (sequential)
    \param cpts        Number of POINT16 in array
    \param points      Array of POINT16
*/
char *U_EMRPOLYPOLYGON16_set(
      const U_RECTL    rclBounds,
      const uint32_t   nPolys,
      const uint32_t  *aPolyCounts,
      const uint32_t   cpts,
      const U_POINT16 *points
   ){
   return(U_EMR_CORE10_set(U_EMR_POLYPOLYGON16, rclBounds, nPolys, aPolyCounts,cpts, points));
}


// U_EMRPOLYDRAW16_set                92
/**
    \brief Allocate and construct a U_EMR_POLYDRAW16 record.
    \return pointer to U_EMR_POLYDRAW16 record, or NULL on error.
    \param rclBounds Bounding rectangle in device units 
    \param cpts      Number of U_POINTL objects         
    \param aptl      Array of U_POINTL objects          
    \param abTypes   Array of Point Enumeration         
*/
char *U_EMRPOLYDRAW16_set(
     const U_RECTL        rclBounds,
     const U_NUM_POINT16  cpts,
     const U_POINT16     *aptl,
     const uint8_t       *abTypes
   ){
   char *record;
   int  irecsize;
   int  cbPoints, cbAbTypes, cbAbTypes4, off;

   if(!cpts || !aptl || !abTypes)return(NULL);
   cbPoints    = cpts * sizeof(U_POINT16);  // space for aptl
   cbAbTypes    = cpts;                     // number of abTypes (same array size, 1 byte each)
   cbAbTypes4    = UP4(cbAbTypes);          // space for abTypes
   irecsize = sizeof(U_EMRPOLYDRAW16) + cbPoints + cbAbTypes4 - sizeof(U_POINT16) - 1;
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)           record)->iType      = U_EMR_POLYDRAW16; 
      ((PU_EMR)           record)->nSize      = irecsize;         
      ((PU_EMRPOLYDRAW16) record)->rclBounds  = rclBounds;        
      ((PU_EMRPOLYDRAW16) record)->cpts       = cpts;             
      off = sizeof(U_EMR) + sizeof(U_RECTL) + sizeof(uint32_t);  // offset to first variable part
      memcpy(record+off,aptl,cbPoints);
      off += cbPoints;
      memcpy(record+off,abTypes,cbAbTypes);
      off += cbAbTypes;
      if(cbAbTypes4 > cbAbTypes){ memset(record+off,0,cbAbTypes4-cbAbTypes); } // keeps valgrind happy (initialize padding after byte array)
   }
   return(record);
}  

// U_EMRCREATEMONOBRUSH_set           93
/**
    \brief Allocate and construct a U_EMR_CREATEMONOBRUSH record.
    \return pointer to U_EMR_CREATEMONOBRUSH record, or NULL on error.
    \param ihBrush Index to place object in EMF object table (this entry must not yet exist) 
    \param iUsage  DIBcolors Enumeration                                                     
    \param Bmi     (Optional) bitmapbuffer (U_BITMAPINFO + pixel array)                      
    \param cbPx    Size in bytes of pixel array (row stride * height, there may be some padding at the end of each row)
    \param Px      (Optional) bitmapbuffer (pixel array section )
*/
char *U_EMRCREATEMONOBRUSH_set(
      const uint32_t            ihBrush,
      const uint32_t            iUsage,
      const PU_BITMAPINFO       Bmi,
      const uint32_t            cbPx,
      const char               *Px
   ){
   return(U_EMR_CORE12_set(U_EMR_CREATEMONOBRUSH,ihBrush,iUsage,Bmi,cbPx,Px));
}

// U_EMRCREATEDIBPATTERNBRUSHPT_set   94
/**
    \brief Allocate and construct a U_EMR_CREATEDIBPATTERNBRUSHPT record.
    Use createdibpatternbrushpt_set() instead of calling this function directly.
    \return pointer to U_EMR_CREATEDIBPATTERNBRUSHPT record, or NULL on error.
    \param ihBrush Index to place object in EMF object table (this entry must not yet exist)
    \param iUsage  DIBcolors Enumeration
    \param Bmi     (Optional) bitmapbuffer (U_BITMAPINFO + pixel array)
    \param cbPx    Size in bytes of pixel array (row stride * height, there may be some padding at the end of each row)
    \param Px      (Optional) bitmapbuffer (pixel array section )
*/
char *U_EMRCREATEDIBPATTERNBRUSHPT_set(
      const uint32_t            ihBrush,
      const uint32_t            iUsage,
      const PU_BITMAPINFO       Bmi,
      const uint32_t            cbPx,
      const char               *Px
   ){
    return(U_EMR_CORE12_set(U_EMR_CREATEDIBPATTERNBRUSHPT,ihBrush,iUsage,Bmi,cbPx,Px));
}


// U_EMREXTCREATEPEN_set              95
/**
    \brief Allocate and construct a U_EMR_EXTCREATEPEN record.
    Use extcreatepen_set() instead of calling this function directly.
    \return pointer to U_EMR_EXTCREATEPEN record, or NULL on error.
    \param ihPen ihPen Index to place object in EMF object table (this entry must not yet exist)
    \param Bmi   Bmi   bitmapbuffer
    \param cbPx  cbPx  Size in bytes of pixel array (row stride * height, there may be some padding at the end of each row)
    \param Px    Px    pixel array (NULL if cbPx == 0)
    \param elp   elp   Pen parameters (Size is Variable!!!!)
*/
char *U_EMREXTCREATEPEN_set(
      const uint32_t      ihPen,
      const PU_BITMAPINFO Bmi,
      const uint32_t      cbPx,
      char               *Px,
      const PU_EXTLOGPEN  elp
   ){
   char *record;
   int   cbImage,cbImage4,cbBmi,off;
   int   irecsize,cbStyleArray,cbElp;
   
   if(!elp)return(NULL);

   SET_CB_FROM_PXBMI(Px,Bmi,cbImage,cbImage4,cbBmi,cbPx);

   cbStyleArray   = elp->elpNumEntries * sizeof(U_STYLEENTRY);                         // space actually used by penstyle entries
   // EXTLOGPEN is already included in EMREXTCREATEPEN, including the possibly unused first penstyle entry
   if(cbStyleArray){
      cbElp   = sizeof(U_EXTLOGPEN) + cbStyleArray - sizeof(U_STYLEENTRY);  // space actually used by elp
      irecsize = sizeof(U_EMREXTCREATEPEN) + cbBmi + cbImage4 + cbStyleArray - sizeof(U_STYLEENTRY);
   }
   else {
      cbElp   = sizeof(U_EXTLOGPEN);                                  //    first U_STYLEENTRY is present but unused
      irecsize = sizeof(U_EMREXTCREATEPEN) + cbBmi + cbImage4;
   }
   record   = malloc(irecsize);

   if(record){
      ((PU_EMR)                 record)->iType      = U_EMR_EXTCREATEPEN;
      ((PU_EMR)                 record)->nSize      = irecsize;
      ((PU_EMREXTCREATEPEN)     record)->ihPen      = ihPen;
      memcpy(&(((PU_EMREXTCREATEPEN) record)->elp),elp,cbElp);
      if(cbStyleArray){
         off = sizeof(U_EMREXTCREATEPEN) + cbStyleArray - sizeof(U_STYLEENTRY);
      }
      else {
         off = sizeof(U_EMREXTCREATEPEN);
      }
      // Cannot use APPEND_PXBMISRC here because there is no "Src" in the field names 
      if(cbBmi){
         memcpy(record + off, Bmi, cbBmi);
         ((PU_EMREXTCREATEPEN) record)->offBmi     = off;
         ((PU_EMREXTCREATEPEN) record)->cbBmi      = cbBmi;
         off += cbBmi; 
         memcpy(record + off, Px, cbImage);
         ((PU_EMREXTCREATEPEN) record)->offBits    = off;
         ((PU_EMREXTCREATEPEN) record)->cbBits     = cbImage;
         off += cbImage;
         if(cbImage4 - cbImage){  memset(record + off, 0, cbImage4 - cbImage); }
      }
      else {
         ((PU_EMREXTCREATEPEN) record)->cbBmi      = 0;
         ((PU_EMREXTCREATEPEN) record)->offBmi     = 0;
         ((PU_EMREXTCREATEPEN) record)->cbBits     = 0;
         ((PU_EMREXTCREATEPEN) record)->offBits    = 0;
      }
   }
   return(record);
}

// U_EMRPOLYTEXTOUTA_set              96 NOT IMPLEMENTED, denigrated after Windows NT
// U_EMRPOLYTEXTOUTW_set              97 NOT IMPLEMENTED, denigrated after Windows NT

// U_EMRSETICMMODE_set                98
/**
    \brief Allocate and construct a U_EMR_SETICMMODE record.
    \return pointer to U_EMR_SETICMMODE record, or NULL on error.
    \param iMode ICMMode Enumeration
*/
char *U_EMRSETICMMODE_set(
      const uint32_t iMode
   ){ 
   return(U_EMR_CORE3_set(U_EMR_SETICMMODE, iMode));
}

// U_EMRCREATECOLORSPACE_set          99
/**
    \brief Allocate and construct a U_EMR_CREATECOLORSPACE record.
    Use createcolorspace_set() instead of calling this function directly.
    \return pointer to U_EMR_CREATECOLORSPACE record, or NULL on error.
    \param ihCS Index to place object in EMF object table (this entry must not yet exist)
    \param lcs  ColorSpace parameters
*/
char *U_EMRCREATECOLORSPACE_set(
      const uint32_t            ihCS,
      const U_LOGCOLORSPACEA    lcs
   ){
   char *record;
   int   irecsize;

   irecsize = sizeof(U_EMRCREATECOLORSPACE);
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)                 record)->iType = U_EMR_CREATECOLORSPACE;
      ((PU_EMR)                 record)->nSize = irecsize;
      ((PU_EMRCREATECOLORSPACE) record)->ihCS  = ihCS;
      ((PU_EMRCREATECOLORSPACE) record)->lcs   = lcs;
   }
   return(record);
} 

// U_EMRSETCOLORSPACE_set            100
/**
    \brief Allocate and construct a U_EMR_SETCOLORSPACE record.
    \return pointer to U_EMR_SETCOLORSPACE record, or NULL on error.
    \param ihCS Index of object in EMF object table
*/
char *U_EMRSETCOLORSPACE_set(
      const uint32_t             ihCS
   ){
   return(U_EMR_CORE3_set(U_EMR_SETCOLORSPACE, ihCS));
}

// U_EMRDELETECOLORSPACE_set         101
/** 
    \brief Allocate and construct a U_EMR_DELETECOLORSPACE record.
    \return pointer to U_EMR_DELETECOLORSPACE record, or NULL on error.
    \param ihCS Index of object in EMF object table
*/
char *U_EMRDELETECOLORSPACE_set(
      const uint32_t             ihCS
   ){
   return(U_EMR_CORE3_set(U_EMR_DELETECOLORSPACE, ihCS));
}

// U_EMRGLSRECORD_set                102  Not implemented
// U_EMRGLSBOUNDEDRECORD_set         103  Not implemented
// U_EMRPIXELFORMAT_set              104
/**
    \brief Allocate and construct a U_EMR_PIXELFORMAT record.
    \return pointer to U_EMR_PIXELFORMAT record, or NULL on error.
    \param pfd PixelFormatDescriptor
*/
char *U_EMRPIXELFORMAT_set(
      const U_PIXELFORMATDESCRIPTOR pfd
   ){
   char *record;
   int   irecsize;

   irecsize = sizeof(U_EMRPIXELFORMAT);
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)            record)->iType   = U_EMR_PIXELFORMAT;
      ((PU_EMR)            record)->nSize   = irecsize;
      ((PU_EMRPIXELFORMAT) record)->pfd     = pfd;
   }
   return(record);
} 
// U_EMRDRAWESCAPE_set               105  Not implemented
// U_EMREXTESCAPE_set                106  Not implemented
// U_EMRUNDEF107_set                 107  Not implemented

// U_EMRSMALLTEXTOUT_set             108
/**
    \brief Allocate and construct a U_EMR_SMALLTEXTOUT record.
    \return pointer to U_EMR_SMALLTEXTOUT record, or NULL on error.
    \param Dest          Where to draw the text
    \param cChars        Characters in TextString (not null terminated)
    \param fuOptions     ExtTextOutOptions Enumeration
    \param iGraphicsMode GraphicsMode Enumeration
    \param exScale       scale on X axis
    \param eyScale       scale on Y axis
    \param rclBounds     OPTIONAL Bounding rectangle (absent when: fuOPtions & ETO_NO_U_RECT)
    \param TextString    text to output (fuOptions & ETO_SMALL_CHARS ? 8 bit : 16 bit)
*/
char *U_EMRSMALLTEXTOUT_set(
      const U_POINTL   Dest,
      const U_NUM_STR  cChars,
      const uint32_t   fuOptions,
      const uint32_t   iGraphicsMode,
      const U_FLOAT    exScale,
      const U_FLOAT    eyScale,
      const U_RECTL    rclBounds,
      const char      *TextString
   ){
   char *record;
   int   irecsize,cbString,cbString4,cbRectl,off;
   int   csize;
   
   if( fuOptions & U_ETO_SMALL_CHARS ){  csize = 1; }         // how many bytes per character
   else {                                csize = 2; }
   cbString = csize * cChars;                                   // filled contents of the string buffer
   cbString4 = UP4(cbString);                                      // size of the variable string buffer
   if(fuOptions & U_ETO_NO_RECT){ cbRectl = 0;               } // size of the optional U_RECTL field
   else {                         cbRectl = sizeof(U_RECTL); }
   
   irecsize = sizeof(U_EMRSMALLTEXTOUT) + cbString4 + cbRectl;
   record    = malloc(irecsize);
   if(record){
      ((PU_EMR)             record)->iType         = U_EMR_SMALLTEXTOUT;
      ((PU_EMR)             record)->nSize         = irecsize;
      ((PU_EMRSMALLTEXTOUT) record)->Dest          = Dest;
      ((PU_EMRSMALLTEXTOUT) record)->cChars        = cChars;
      ((PU_EMRSMALLTEXTOUT) record)->fuOptions     = fuOptions;
      ((PU_EMRSMALLTEXTOUT) record)->iGraphicsMode = iGraphicsMode;
      ((PU_EMRSMALLTEXTOUT) record)->exScale       = exScale;
      ((PU_EMRSMALLTEXTOUT) record)->eyScale       = eyScale;
      off = sizeof(U_EMRSMALLTEXTOUT);  //offset to the start of the variable fields
      if(cbRectl){
         memcpy(record + off, &rclBounds, cbRectl);
         off += cbRectl;
      }
      memcpy(record + off, TextString, cbString);
      if(cbString < cbString4){
         off += cbString;
         memset(record + off, 0, cbString4 - cbString);
      }
   }
   return(record);
} 

// U_EMRFORCEUFIMAPPING_set          109  Not implemented
// U_EMRNAMEDESCAPE_set              110  Not implemented
// U_EMRCOLORCORRECTPALETTE_set      111  Not implemented
// U_EMRSETICMPROFILEA_set           112  Not implemented
// U_EMRSETICMPROFILEW_set           113  Not implemented

// U_EMRALPHABLEND_set               114
/**
    \brief Allocate and construct a U_EMR_ALPHABLEND record.
    \return pointer to U_EMR_ALPHABLEND record, or NULL on error.
    \param rclBounds    Bounding rectangle in device units
    \param Dest         Destination UL corner in logical units
    \param cDest        Destination width in logical units
    \param Src          Source UL corner in logical units
    \param cSrc         Src W & H in logical units
    \param xformSrc     Transform to apply to source
    \param crBkColorSrc Background color
    \param iUsageSrc    DIBcolors Enumeration
    \param Blend        Blend function
    \param Bmi          (Optional) bitmapbuffer (U_BITMAPINFO section)
    \param cbPx         Size in bytes of pixel array (row stride * height, there may be some padding at the end of each row)
    \param Px           (Optional) bitmapbuffer (pixel array section )
*/
char *U_EMRALPHABLEND_set(
      const U_RECTL       rclBounds,
      const U_POINTL      Dest,
      const U_POINTL      cDest,
      const U_POINTL      Src,
      const U_POINTL      cSrc,
      const U_XFORM       xformSrc,
      const U_COLORREF    crBkColorSrc,
      const uint32_t      iUsageSrc,
      const U_BLEND       Blend,
      const PU_BITMAPINFO Bmi,
      const uint32_t      cbPx,
      char               *Px
   ){
   return(U_EMR_CORE13_set(U_EMR_ALPHABLEND,rclBounds,Dest,cDest,Src,cSrc,xformSrc,crBkColorSrc,iUsageSrc,*((uint32_t *) &Blend),Bmi,cbPx,Px));
}

// U_EMRSETLAYOUT_set                115
/**
    \brief Allocate and construct a U_EMR_SETLAYOUT record.
    \return pointer to U_EMR_SETLAYOUT record, or NULL on error.
    \param iMode Mirroring Enumeration
*/
char *U_EMRSETLAYOUT_set(uint32_t iMode){
  return(U_EMR_CORE3_set(U_EMR_SETLAYOUT, iMode));
}

// U_EMRTRANSPARENTBLT_set           116
/**
    \brief Allocate and construct a U_EMR_TRANSPARENTBLT record.
    \return pointer to U_EMR_TRANSPARENTBLT record, or NULL on error.
    \param rclBounds    Bounding rectangle in device units
    \param Dest         Destination UL corner in logical units
    \param cDest        Destination width in logical units
    \param Src          Source UL corner in logical units
    \param cSrc         Src W & H in logical units
    \param xformSrc     Transform to apply to source
    \param crBkColorSrc Background color
    \param iUsageSrc    DIBcolors Enumeration
    \param TColor       Bitmap color to be treated as transparent
    \param Bmi          (Optional) bitmapbuffer (U_BITMAPINFO section)
    \param cbPx         Size in bytes of pixel array (row stride * height, there may be some padding at the end of each row)
    \param Px           (Optional) bitmapbuffer (pixel array section )
*/
char *U_EMRTRANSPARENTBLT_set(
      const U_RECTL       rclBounds,
      const U_POINTL      Dest,
      const U_POINTL      cDest,
      const U_POINTL      Src,
      const U_POINTL      cSrc,
      const U_XFORM       xformSrc,
      const U_COLORREF    crBkColorSrc,
      const uint32_t      iUsageSrc,
      const uint32_t      TColor,
      const PU_BITMAPINFO Bmi,
      const uint32_t      cbPx,
      char               *Px
   ){
   return(U_EMR_CORE13_set(U_EMR_TRANSPARENTBLT,rclBounds,Dest,cDest,Src,cSrc,xformSrc,crBkColorSrc,iUsageSrc,TColor,Bmi,cbPx,Px));
}
// U_EMRUNDEF117_set                 117  Not implemented
// U_EMRGRADIENTFILL_set             118
/**
    \brief Allocate and construct a U_EMR_TRANSPARENTBLT record.
    \return pointer to U_EMR_TRANSPARENTBLT record, or NULL on error.
    \param rclBounds    Bounding rectangle in device units
    \param nTriVert     Number of TriVertex objects in TriVert
    \param nGradObj     Number of gradient triangle/rectangle objects
    \param ulMode       Gradientfill Enumeration (determines Triangle/Rectangle)
    \param TriVert      Array of TriVertex objects
    \param GradObj      Array of gradient objects (each has 2 [rect] or 3 [triangle] indices into TriVert array) 

There is an MS documentation or library problem for this record, as the size of the GradObj must always be set
as if it was an array of U_GRADIENT3 objects for both rect and triangle.  For horizontal and vertical gradients
this means that there will be unused bytes at the end of the record.  This is not what the documentation says,
but it is how MS's libraries work.

*/
char *U_EMRGRADIENTFILL_set(
      const U_RECTL             rclBounds,
      const U_NUM_TRIVERTEX     nTriVert,
      const U_NUM_GRADOBJ       nGradObj,
      const uint32_t            ulMode,
      const PU_TRIVERTEX        TriVert,
      const uint32_t           *GradObj
   ){
   char *record;
   unsigned int   cbTriVert,cbGradObj,off;
   unsigned int   cbGradObjAlloc; /* larger than cbGradObj, because of problem described above */
   int   irecsize;

   cbTriVert = sizeof(U_TRIVERTEX) * nTriVert;  // all of the cb's will be a multiple of 4 bytes
   if(     ulMode == U_GRADIENT_FILL_TRIANGLE){ cbGradObj = sizeof(U_GRADIENT3) * nGradObj; }
   else if(ulMode == U_GRADIENT_FILL_RECT_H || 
           ulMode == U_GRADIENT_FILL_RECT_V){   cbGradObj = sizeof(U_GRADIENT4) * nGradObj; }
   else {                                       return(NULL);                               }
   cbGradObjAlloc = sizeof(U_GRADIENT3) * nGradObj; 

   irecsize = sizeof(U_EMRGRADIENTFILL) + cbTriVert + cbGradObjAlloc;
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)             record)->iType      = U_EMR_GRADIENTFILL;
      ((PU_EMR)             record)->nSize      = irecsize;
      ((PU_EMRGRADIENTFILL) record)->rclBounds  = rclBounds;
      ((PU_EMRGRADIENTFILL) record)->nTriVert   = nTriVert;
      ((PU_EMRGRADIENTFILL) record)->nGradObj   = nGradObj;
      ((PU_EMRGRADIENTFILL) record)->ulMode     = ulMode;
      off = sizeof(U_EMRGRADIENTFILL); // offset to TriVert field
      memcpy(record + off, TriVert, cbTriVert);
      off += cbTriVert;
      memcpy(record + off, GradObj, cbGradObj);
      off += cbGradObj;
      if(cbGradObjAlloc > cbGradObj){
         memset(record+off,0,cbGradObjAlloc - cbGradObj);
      }
   }
   return(record);
}

// U_EMRSETLINKEDUFIS_set            119  Not implemented
// U_EMRSETTEXTJUSTIFICATION_set     120  Not implemented (denigrated)
// U_EMRCOLORMATCHTOTARGETW_set      121  Not implemented  

// U_EMRCREATECOLORSPACEW_set        122
/**
    \brief Allocate and construct a U_EMR_CREATECOLORSPACEW record.
    Use createcolorspacew_set() instead of calling this function directly.
    \return pointer to U_EMR_CREATECOLORSPACEW record, or NULL on error.
    \param ihCS     Index to place object in EMF object table (this entry must not yet exist)
    \param lcs      ColorSpace parameters
    \param dwFlags  If low bit set Data is present
    \param cbData   Number of bytes of theData field.
    \param Data     (Optional, dwFlags & 1) color profile data 
*/
char *U_EMRCREATECOLORSPACEW_set(
      const uint32_t          ihCS,
      const U_LOGCOLORSPACEW  lcs,
      const uint32_t          dwFlags,
      const U_CBDATA          cbData,
      const uint8_t          *Data
   ){
   char *record;
   unsigned int   cbData4,off;
   int   irecsize;

   cbData4 = UP4(cbData);                     // buffer to hold Data
   irecsize = sizeof(U_EMRCREATECOLORSPACEW) + cbData4;
   record   = malloc(irecsize);
   if(record){
      ((PU_EMR)                  record)->iType   = U_EMR_CREATECOLORSPACEW;
      ((PU_EMR)                  record)->nSize   = irecsize;
      ((PU_EMRCREATECOLORSPACEW) record)->ihCS    = ihCS;
      ((PU_EMRCREATECOLORSPACEW) record)->lcs     = lcs;
      ((PU_EMRCREATECOLORSPACEW) record)->dwFlags = dwFlags;
      ((PU_EMRCREATECOLORSPACEW) record)->cbData  = cbData;
      off = sizeof(U_EMR) + sizeof(uint32_t) + sizeof(U_LOGCOLORSPACEW) + sizeof(uint32_t) + sizeof(U_CBDATA); // offset to Data field
      memcpy(record + off, Data, cbData);
      if(cbData < cbData4){
        off += cbData;
        memset(record + off,0,cbData4-cbData);
      }
   }
   return(record);
} 


#ifdef __cplusplus
}
#endif
