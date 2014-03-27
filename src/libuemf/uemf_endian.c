/**
  @file uemf_endian.c
  
  @brief Functions for converting EMF records between Big Endian and Little Endian byte orders.
   
  EMF files use Little Endian order.
  On a Big Endian machine the data must be converted to/from Little Endian when it is writen to/read from a file.
  On a Little Endian machine no conversion is required, but it is good to be able to test the routines on either platform.
  When "torev" is true these routines convert from the native byte order to the reverse.
  When "torev" is false these routines convert from the reverse byte order to the native.
  Routines that do not use that variable swap byte order, and the way in which they do so does not depend
  on the native byte order.

  The only function here which should be called directly is U_emf_endian(), and then,except for testing purposes, only on a BE machine.

  Many variables are initialized to zero even though they will always be set because
  some versions of gcc give spurious "may be used uninitialized" warnings otherwise.
*/

/*
File:      uemf_endian.c
Version:   0.0.16
Date:      27-MAR-2014
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2014 David Mathog and California Institute of Technology (Caltech)
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "uemf.h"
#include "uemf_endian.h"

// hide almost everuything in here from Doxygen
//! \cond

/* **********************************************************************************************
   These functions convert standard objects used in the EMR records.
*********************************************************************************************** */

void U_swap2(void *ul, unsigned int count){
   uint8_t   ctmp;
   uint8_t  *cl = (uint8_t *) ul;
   for(; count; count--,cl+=2){
      ctmp            = *cl;
      *cl             = *(cl+1);
      *(cl+1)         = ctmp;
   }
}

/* Note: U_swap4 is also used by uwmf_endian.c, in cases where the 32 bit data is not aligned on a 4 byte boundary */
void U_swap4(void *ul, unsigned int count){
   uint8_t   ctmp;
   uint8_t  *cl = (uint8_t *) ul;
   for(; count; count--,cl+=4){
      ctmp    = *(cl+0);
      *(cl+0) = *(cl+3);
      *(cl+3) = ctmp;
      ctmp    = *(cl+1);
      *(cl+1) = *(cl+2);
      *(cl+2) = ctmp;
   }
}

/** 
    U_COLORREF and U_RGBQUAD do NOT need to be swapped, they are always stored in memory in the proper order.
*/

/**
    \brief Convert rect and rectl objects from Upper Left and Lower Right corner points.
    \param rect U_RECTL object
    \param count number to convert
*/
void rectl_swap(
      PU_RECTL rect,
      unsigned int count
    ){
    U_swap4(rect,4*count);
}

/**
    \brief Convert a U_SIZEL object.
    \param sz U_SizeL object
    \param count number to convert
*/
void sizel_swap(
       PU_SIZEL sz,
       unsigned int count
    ){
    U_swap4(sz,2*count);
} 

/**
    \brief Convert a U_POINTL object
    \param pt U_POINTL object
    \param count number to convert
*/
void pointl_swap(
       PU_POINTL pt,
       unsigned int count
    ){
    U_swap4(pt,2*count);
} 

/**
    \brief Convert a U_POINT16 object
    \param pt U_POINT16 object
    \param count number to convert
*/
void point16_swap(
       PU_POINT16 pt,
       unsigned int count
    ){
    U_swap2(pt,2*count);
} 



/**
    \brief Convert a U_TRIVERTEX object.
    \param tv U_TRIVERTEX object.
    \param count number to convert
*/
void trivertex_swap(
      PU_TRIVERTEX tv,
      unsigned int count
   ){
   for(;count; count--, tv++){
      U_swap4(tv,2);          /* x,y */
      U_swap2(&(tv->Red),4);  /* Red, Green, Blue, Alpha */
   }
}

/**
    \brief Convert a U_GRADIENT3 object.
    \param tv U_GRADIENT3 object.
    \param count number to convert
*/
void gradient3_swap(
      PU_GRADIENT3 g3,
      unsigned int count
   ){
   U_swap4(g3,3*count);
}

/**
    \brief Convert a U_GRADIENT4 object.
    \param tv U_GRADIENT4 object.
    \param count number to convert
*/
void gradient4_swap(
      PU_GRADIENT4 g4,
      unsigned int count
   ){
   U_swap4(g4,2*count); //a gradient4 object has 2 int4's, NOT 4!
}

/**
    \brief Convert a U_LOGBRUSH object.
    \param lb U_LOGBRUSH object.
*/
void logbrush_swap(
      PU_LOGBRUSH lb  
   ){
   U_swap4(&(lb->lbStyle),1);               // lbStyle
   // ordered bytes:                           lbColor
   U_swap4(&(lb->lbHatch),1);               // lbHatch
}

/**
    \brief Convert a U_XFORM object.
    \param xform U_XFORM object
*/
void xform_swap(
      PU_XFORM xform
   ){
   U_swap4(xform,6);
}


/**
    \brief Convert a U_CIEXYZTRIPLE object
    \param cie3 U_CIEXYZTRIPLE object
*/
void ciexyztriple_swap(
      PU_CIEXYZTRIPLE cie3
   ){
   U_swap4(cie3,9);
}
/**
    \brief Convert a U_LOGCOLORSPACEA object.
    \param lcsa     U_LOGCOLORSPACEA object
*/
void logcolorspacea_swap(
      PU_LOGCOLORSPACEA lcsa
   ){
   U_swap4(lcsa,5);                         // lcsSignature lcsVersion lcsSize lcsCSType lcsIntent
   ciexyztriple_swap(&(lcsa->lcsEndpoints));
   // ordered bytes:                           lcsGammaRGB
   // ordered bytes:                           lcsFilename
}

/**

    \brief Convert a U_LOGCOLORSPACEW object.
    \param lcsa U_LOGCOLORSPACEW object                                               
*/
void logcolorspacew_swap(
      PU_LOGCOLORSPACEW lcsa
   ){
   U_swap4(lcsa,5);                         // lcsSignature lcsVersion lcsSize lcsCSType lcsIntent
   ciexyztriple_swap(&(lcsa->lcsEndpoints));
   // ordered bytes:                           lcsGammaRGB
   // UTF-16LE, already in order:              lcsFilename
}


/**
    \brief Convert a U_LOGFONT object.
    \param lf   U_LOGFONT object
*/
void logfont_swap(
       PU_LOGFONT lf
   ){
   U_swap4(lf,5);                           // lfHeight lfWidth lfEscapement lfOrientation lfWeight
   // ordered bytes:                           lfItalic lfUnderline lfStrikeOut lfCharSet lfOutPrecision lfClipPrecision lfQuality lfPitchAndFamily
   // UTF16-LE, already in order
}

/**
    \brief Convert a U_LOGFONT_PANOSE object.
    \return U_LOGFONT_PANOSE object
*/
void logfont_panose_swap(
      PU_LOGFONT_PANOSE lfp
   ){    
   logfont_swap(&(lfp->elfLogFont));        // elfLogFont
   // UTF-16LE, already in order:              elfFullName
   // UTF-16LE, already in order:              elfStyle
   U_swap4(&(lfp->elfVersion),4);           // elfVersion elfStyleSize elfMatch elfReserved 
   // ordered bytes:                           elfVendorId
   U_swap4(&(lfp->elfCulture),1);           // elfCulture
   // ordered bytes:                           elfPanose
}

/**
    \brief Convert a U_BITMAPINFOHEADER object.
    \param Bmi U_BITMAPINFOHEADER object 
*/
void bitmapinfoheader_swap(
      PU_BITMAPINFOHEADER Bmi
   ){
   U_swap4(Bmi,3);                          // biSize biWidth biHeight
   U_swap2(&(Bmi->biPlanes),2);             // biPlanes biBitCount
   U_swap4(&(Bmi->biCompression),6);        // biCompression biSizeImage biXPelsPerMeter biYPelsPerMeter biClrUsed biClrImportant 
}


/**
    \brief Convert a Pointer to a U_BITMAPINFO object.
    \param Bmi Pointer to a U_BITMAPINFO object
*/
void bitmapinfo_swap(
      PU_BITMAPINFO Bmi
   ){
   bitmapinfoheader_swap(&(Bmi->bmiHeader)); // bmIHeader
   // ordered bytes:                            bmiColors
}

/**
    \brief Convert a pointer to a U_EXTLOGPEN object.
    \param elp   PU_EXTLOGPEN object
*/
void extlogpen_swap(
      PU_EXTLOGPEN elp,
      int torev
   ){
   int count=0;
   U_swap4(elp,3);                          // elpPenStyle elpWidth elpBrushStyle
   // ordered bytes:                           elpColor
   if(torev){
       count    = elp->elpNumEntries;
   }
   U_swap4(&(elp->elpHatch),2);             // elpHatch elpNumEntries
   if(!torev){
       count    = elp->elpNumEntries;
   }
   U_swap4(&(elp->elpStyleEntry),count);    // elpStyleEntry[]
}

/**
    \brief Convert a U_LOGPEN object.
    \param lp  U_LOGPEN object
    
*/
void logpen_swap(
      PU_LOGPEN lp
   ){
   U_swap4(lp,1);                           // lopnStyle
   pointl_swap(&(lp->lopnWidth),1);         // lopnWidth
   // ordered bytes:                           lopnColor
} 


/**
    \brief Convert a pointer to a U_LOGPALETTE object.
    \param lp  Pointer to a U_LOGPALETTE object.
*/
void logpalette_swap(
      PU_LOGPALETTE lp
   ){
   U_swap2(lp,2);                           // palVersion palNumEntries
   // ordered bytes:                           palPalEntry[]
}

/**
    \brief Convert a U_RGNDATAHEADER object.
    \param rdh  U_RGNDATAHEADER object
*/
void rgndataheader_swap(
      PU_RGNDATAHEADER rdh
   ){
   U_swap4(rdh,4);                          // dwSize iType nCount nRgnSize
   rectl_swap(&(rdh->rclBounds),1);         // rclBounds
}

/**
    \brief Convert a pointer to a U_RGNDATA object.
    \param rgd  pointer to a U_RGNDATA object.
*/
void rgndata_swap(
      PU_RGNDATA rd
   ){
   int count = rd->rdh.nCount;
   rgndataheader_swap(&(rd->rdh));
   U_swap4(rd->Buffer,4*count);
}

/**
    \brief Convert a U_COLORADJUSTMENT object.
    \param ca U_COLORADJUSTMENT object.        
*/
void coloradjustment_swap(
      PU_COLORADJUSTMENT ca
   ){
   U_swap2(ca,12);                          // caSize caFlags caIlluminantIndex caRedGamma caGreenGamma caBlueGamma caReferenceBlack caReferenceWhite caContrast caBrightness caColorfulness caRedGreenTint   
}

/**
    \brief Convert a pointer to a U_PIXELFORMATDESCRIPTOR object.
    \param pfd pointer to a U_PIXELFORMATDESCRIPTOR object.
*/
void pixelformatdescriptor_swap(
      PU_PIXELFORMATDESCRIPTOR pfd
   ){
   U_swap2(pfd,2);                          // nSize nVersion
   U_swap4(&(pfd->dwFlags),1);              // dwFlags
   // ordered bytes:                           iPixelType cColorBits cRedBits cRedShift cGreenBits cGreenShift cBlueBits cBlueShift cAlphaBits cAlphaShift cAccumBits cAccumRedBits cAccumGreenBits cAccumBlueBits cAccumAlphaBits cDepthBits cStencilBits cAuxBuffers iLayerType bReserved      
   U_swap4(&(pfd->dwLayerMask),3);          // dwLayerMask dwVisibleMask dwDamageMask
}

/**
    \brief Convert a Pointer to a U_EMRTEXT record
    \param pemt      Pointer to a U_EMRTEXT record 
    \param record    Pointer to the start of the record which contains this U_EMRTEXT
    \param torev     1 for native to reversed, 0 for reversed to native
*/
void emrtext_swap(
      PU_EMRTEXT pemt,
      char      *record,
      int        torev
   ){
   int        off;
   uint32_t   count=0;
   uint32_t   offDx=0;
   uint32_t   fOptions=0;
   pointl_swap(&(pemt->ptlReference),1);    // ptlReference
   if(torev){
       count    = pemt->nChars;
       fOptions = pemt->fOptions;
   }
   U_swap4(&(pemt->nChars),3);              // nChars offString fOptions
   if(!torev){
       count    = pemt->nChars;
       fOptions = pemt->fOptions;
   }
   off = sizeof(U_EMRTEXT);
   if(!(fOptions & U_ETO_NO_RECT)){
       rectl_swap((PU_RECTL)((char *)pemt + off),1);  // optional rectangle
       off+=sizeof(U_RECTL);
   }
   if(torev){
      offDx = *(uint32_t *)((char *)pemt +off);
   }
   // ordered bytes OR UTF16-LE:               the string at offString
   U_swap4(((char *)pemt+off),1);           // offDx
   if(!torev){
      offDx = *(uint32_t *)((char *)pemt +off);
   }
   U_swap4((record+offDx),count);           // Dx[], offset with respect to the Record, NOT the object
}



/* **********************************************************************************************
These functions contain shared code used by various U_EMR*_swap functions.  These should NEVER be called
by end user code and to further that end prototypes are NOT provided and they are hidden from Doxygen.


   These all have this form:
   
   void core1_swap(char *record, int torev){
   
   but some do not actually use torev.
   
   
   
*********************************************************************************************** */

// all core*_swap call this, U_EMRSETMARGN_swap and some others all it directly
// numbered as core5 to be consistent with uemf.c, but must appear before the others as there is no prototype
void core5_swap(char *record, int torev){
   UNUSED_PARAMETER(torev);
   PU_ENHMETARECORD pEMR = (PU_ENHMETARECORD)(record);
   U_swap4(pEMR,2);                         // iType nSize
}

// Functions with the same form starting with U_EMRPOLYBEZIER_swap
void core1_swap(char *record, int torev){
   int count=0;
   PU_EMRPOLYLINETO pEmr = (PU_EMRPOLYLINETO) (record);
   if(torev){
      count = pEmr->cptl;
   }
   core5_swap(record, torev);
   rectl_swap(&(pEmr->rclBounds),1 );       // rclBounds
   U_swap4(&(pEmr->cptl),1);                // cptl
   if(!torev){
      count = pEmr->cptl;
   }
   pointl_swap((pEmr->aptl),count);         // aptl[]
}

// Functions with the same form starting with U_EMRPOLYPOLYLINE_swap
void core2_swap(char *record, int torev){
   int count=0;
   int nPolys=0;
   PU_EMRPOLYPOLYLINE pEmr = (PU_EMRPOLYPOLYLINE) (record);
   if(torev){
      count  = pEmr->cptl;
      nPolys = pEmr->nPolys;
   }
   core5_swap(record, torev);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->nPolys),2);              // nPolys cptl
   if(!torev){
      count  = pEmr->cptl;
      nPolys = pEmr->nPolys;
   }
   U_swap4(pEmr->aPolyCounts,nPolys);       // aPolyCounts[]
   pointl_swap((PU_POINT)(record + sizeof(U_EMRPOLYPOLYLINE) - 4 + sizeof(uint32_t)* nPolys), count); // paptl[]
}


// Functions with the same form starting with U_EMRSETMAPMODE_swap
void core3_swap(char *record, int torev){
   PU_EMRSETMAPMODE pEmr   = (PU_EMRSETMAPMODE)(record);
   core5_swap(record, torev);
   U_swap4(&(pEmr->iMode),1);               // iMode
} 

// Functions taking a single U_RECT or U_RECTL, starting with U_EMRELLIPSE_swap, also U_EMRFILLPATH_swap, 
void core4_swap(char *record, int torev){
   PU_EMRELLIPSE pEmr      = (PU_EMRELLIPSE)(   record);
   core5_swap(record, torev);
   rectl_swap(&(pEmr->rclBox),1);           // rclBox
} 

// Functions with the same form starting with U_EMRPOLYBEZIER16_swap
void core6_swap(char *record, int torev){
   int count=0;
   PU_EMRPOLYBEZIER16 pEmr = (PU_EMRPOLYBEZIER16) (record);
   if(torev){
      count = pEmr->cpts;
   }
   core5_swap(record, torev);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->cpts),1);                // cpts
   if(!torev){
      count = pEmr->cpts;
   }
   point16_swap((pEmr->apts),count);        // apts[]
} 


// Records with the same form starting with U_EMRSETWINDOWEXTEX_swap, that is, all with two uint32_t values after the emr
void core7_swap(char *record, int torev){
   PU_EMRGENERICPAIR pEmr = (PU_EMRGENERICPAIR) (record);
   core5_swap(record, torev);
   U_swap4(&(pEmr->pair),2);
}

// For U_EMREXTTEXTOUTA and U_EMREXTTEXTOUTW, type=0 for the first one
void core8_swap(char *record, int torev){
   PU_EMREXTTEXTOUTA pEmr = (PU_EMREXTTEXTOUTA) (record);
   emrtext_swap(&(pEmr->emrtext),record,torev);
   core5_swap(record, torev);
   U_swap4(&(pEmr->iGraphicsMode),1);       // iGraphicsMode
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->exScale),2);             // exScale eyScale
} 

// Functions that take a rect and a pair of points, starting with U_EMRARC_swap
void core9_swap(char *record, int torev){
   PU_EMRARC pEmr = (PU_EMRARC) (record);
   core5_swap(record, torev);
   rectl_swap(&(pEmr->rclBox),1);           // rclBox
   U_swap4(&(pEmr->ptlStart),4);            // ptlStart ptlEnd
}

// Functions with the same form starting with U_EMRPOLYPOLYLINE16_swap
void core10_swap(char *record, int torev){
   int count=0;
   int nPolys=0;
   PU_EMRPOLYPOLYLINE16 pEmr = (PU_EMRPOLYPOLYLINE16) (record);
   if(torev){
      count  = pEmr->cpts;
      nPolys = pEmr->nPolys;
   }
   core5_swap(record, torev);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->nPolys),2);              // nPolys cpts
   if(!torev){
      count  = pEmr->cpts;
      nPolys = pEmr->nPolys;
   }
   U_swap4(pEmr->aPolyCounts,nPolys);       // aPolyCounts[]
   point16_swap((PU_POINT16)(record + sizeof(U_EMRPOLYPOLYLINE16) - 4 + sizeof(uint32_t)* nPolys), count); // apts[]
} 

// Functions with the same form starting with  U_EMRINVERTRGN_swap and U_EMRPAINTRGN_swap,
void core11_swap(char *record, int torev){
   int roff=0;
   int nextroff=0;
   int limit=0;
   PU_EMRINVERTRGN pEmr = (PU_EMRINVERTRGN) (record);
   roff = 0;
   if(torev){
      limit    = pEmr->emr.nSize;
      nextroff = 0;
   }
   core5_swap(record, torev);
   if(!torev){
      limit    = pEmr->emr.nSize;
   }
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->cbRgnData),1);           // cbRgnData
   if(!torev){
      limit = pEmr->emr.nSize;   
   }
   // This one is a pain since each RGNDATA may be a different size, so it isn't possible to index through them.
   char *prd = (char *) &(pEmr->RgnData);
   while(roff + 28 < limit){                // up to the end of the record
      if(torev){
         nextroff += (((PU_RGNDATA)prd)->rdh.dwSize + ((PU_RGNDATA)prd)->rdh.nRgnSize - 16);
         rgndata_swap((PU_RGNDATA) (prd + roff));
         roff  = nextroff;
      }
      else {
         rgndata_swap((PU_RGNDATA) (prd + roff));
         roff += (((PU_RGNDATA)prd)->rdh.dwSize + ((PU_RGNDATA)prd)->rdh.nRgnSize - 16);
      }
   }
} 


// common code for U_EMRCREATEMONOBRUSH_swap and U_EMRCREATEDIBPATTERNBRUSHPT_swap,
void core12_swap(char *record, int torev){
   PU_EMRCREATEMONOBRUSH pEmr = (PU_EMRCREATEMONOBRUSH) (record);
   if(torev && pEmr->cbBmi){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmi));  // Bmi
   }
   core5_swap(record, torev);
   U_swap4(&(pEmr->ihBrush),6);             // ihBrush iUsage offBmi cbBmi offBits cbBits
   // ordered bytes:                           bitmap (including 16 bit 5bit/channel color mode, which is done bytewise).    
   if(!torev && pEmr->cbBmi){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmi));  // Bmi
   }
}

// common code for U_EMRALPHABLEND_swap and U_EMRTRANSPARENTBLT_swap,
void core13_swap(char *record, int torev){
   PU_EMRALPHABLEND pEmr = (PU_EMRALPHABLEND) (record);
   if(torev && pEmr->cbBmiSrc){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmiSrc));
   }
   core5_swap(record, torev);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   pointl_swap(&(pEmr->Dest),2);            // Dest cDest
   pointl_swap(&(pEmr->Dest),2);            // Dest cDest
   // ordered bytes:                           Blend
   pointl_swap(&(pEmr->Src),2);             // Src
   xform_swap( &(pEmr->xformSrc));          // xformSrc
   // ordered bytes:                           crBkColorSrc
   U_swap4(&(pEmr->iUsageSrc),5);           // iUsageSrc offBmiSrc cbBmiSrc offBitsSrc cbBitsSrc
   // ordered bytes:                           bitmap (including 16 bit 5bit/channel color mode, which is done bytewise).    
   if(!torev && pEmr->cbBmiSrc){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmiSrc));
   }
}

/* **********************************************************************************************
These are the core EMR functions, each creates a particular type of record.  
All return these records via a char* pointer, which is NULL if the call failed.  
They are listed in order by the corresponding U_EMR_* index number.  
*********************************************************************************************** */

/**
    All of the record level (hidden) functions have this form:
    \brief Convert a pointer to a U_EMR_whatever record which has not been implemented.
    \param record   pointer to a buffer holding the EMR record
    \param torev    1 for native to reversed, 0 for reversed to native
*/
void U_EMRNOTIMPLEMENTED_swap(char *record, int torev){
   core5_swap(record, torev);
   printf("WARNING:  could not convert data in record type that has not been implemented!\n");
}

// U_EMRHEADER                1
void U_EMRHEADER_swap(char *record, int torev){
   int nDesc,offDesc,nSize,cbPix,offPix;
   PU_EMRHEADER pEmr = (PU_EMRHEADER)(record);
   if(torev){
     nSize = pEmr->emr.nSize;
     core5_swap(record, torev);
   }
   else {
     core5_swap(record, torev);
     nSize = pEmr->emr.nSize;
   }

   rectl_swap(&(pEmr->rclBounds),2);        // rclBounds rclFrame
   U_swap4(&(pEmr->dSignature), 4);         // dSignature nVersion nBytes nRecords
   U_swap2(&(pEmr->nHandles), 2);           // nHandlessReserved
   if(torev){
      nDesc = pEmr->nDescription;
      offDesc = pEmr->offDescription;
      U_swap4(&(pEmr->nDescription), 3);       // nDescription offDescription nPalEntries 
   } 
   else {
      U_swap4(&(pEmr->nDescription), 3);       // nDescription offDescription nPalEntries 
      nDesc = pEmr->nDescription;
      offDesc = pEmr->offDescription;
   } 
   // UTF16-LE                                 Description
   sizel_swap(&(pEmr->szlDevice), 2);       // szlDevice szlMillimeters
   if((nDesc && (offDesc >= 100)) || 
      (!offDesc && nSize >= 100)
     ){
     if(torev){
        cbPix = pEmr->cbPixelFormat;
        offPix = pEmr->offPixelFormat;
        if(cbPix)pixelformatdescriptor_swap( (PU_PIXELFORMATDESCRIPTOR) (record + pEmr->offPixelFormat));
        U_swap4(&(pEmr->cbPixelFormat), 2);      // cbPixelFormat offPixelFormat 
     }
     else {
        U_swap4(&(pEmr->cbPixelFormat), 2);      // cbPixelFormat offPixelFormat 
        cbPix = pEmr->cbPixelFormat;
        offPix = pEmr->offPixelFormat;
        if(cbPix)pixelformatdescriptor_swap( (PU_PIXELFORMATDESCRIPTOR) (record + pEmr->offPixelFormat));
     }
     U_swap4(&(pEmr->bOpenGL), 1);            // bOpenGL
     if((nDesc && (offDesc >= 108)) || 
        (cbPix && (offPix >=108)) ||
        (!offDesc && !cbPix && nSize >= 108)
       ){
         sizel_swap(&(pEmr->szlMicrometers), 1);  // szlMicrometers
       }
   }
}

// U_EMRPOLYBEZIER                       2
void U_EMRPOLYBEZIER_swap(char *record, int torev){
   core1_swap(record, torev);
} 

// U_EMRPOLYGON                          3
void U_EMRPOLYGON_swap(char *record, int torev){
   core1_swap(record, torev);
} 


// U_EMRPOLYLINE              4
void U_EMRPOLYLINE_swap(char *record, int torev){
   core1_swap(record, torev);
} 

// U_EMRPOLYBEZIERTO          5
void U_EMRPOLYBEZIERTO_swap(char *record, int torev){
   core1_swap(record, torev);
} 

// U_EMRPOLYLINETO            6
void U_EMRPOLYLINETO_swap(char *record, int torev){
   core1_swap(record, torev);
} 

// U_EMRPOLYPOLYLINE          7
void U_EMRPOLYPOLYLINE_swap(char *record, int torev){
   core2_swap(record, torev);
} 

// U_EMRPOLYPOLYGON           8
void U_EMRPOLYPOLYGON_swap(char *record, int torev){
   core2_swap(record, torev);
} 

// U_EMRSETWINDOWEXTEX        9
void U_EMRSETWINDOWEXTEX_swap(char *record, int torev){
   core7_swap(record, torev);
} 

// U_EMRSETWINDOWORGEX       10
void U_EMRSETWINDOWORGEX_swap(char *record, int torev){
   core7_swap(record, torev);
} 

// U_EMRSETVIEWPORTEXTEX     11
void U_EMRSETVIEWPORTEXTEX_swap(char *record, int torev){
   core7_swap(record, torev);
} 

// U_EMRSETVIEWPORTORGEX     12
void U_EMRSETVIEWPORTORGEX_swap(char *record, int torev){
   core7_swap(record, torev);
} 

// U_EMRSETBRUSHORGEX        13
void U_EMRSETBRUSHORGEX_swap(char *record, int torev){
   core7_swap(record, torev);
} 

// U_EMREOF                  14
void U_EMREOF_swap(char *record, int torev){
   int off=0;
   int cbPalEntries=0;
   core5_swap(record, torev);
   PU_EMREOF pEmr = (PU_EMREOF)(record);
   if(torev){
      cbPalEntries = pEmr->cbPalEntries;
      if(cbPalEntries){
         logpalette_swap( (PU_LOGPALETTE)(record + pEmr->offPalEntries));
      }
   }
   U_swap4(&(pEmr->cbPalEntries),2);        // cbPalEntries offPalEntries
   if(!torev){
      cbPalEntries = pEmr->cbPalEntries;
      if(cbPalEntries){
         logpalette_swap( (PU_LOGPALETTE)(record + pEmr->offPalEntries));
      }
   }
   off = sizeof(U_EMREOF) + 4 * cbPalEntries;
   U_swap4(record + off,1);                 // nSizeLast
} 


// U_EMRSETPIXELV            15
void U_EMRSETPIXELV_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMRSETPIXELV pEmr = (PU_EMRSETPIXELV)(record);
   pointl_swap(&(pEmr->ptlPixel),1);        // ptlPixel
   // ordered bytes:                           crColor
} 


// U_EMRSETMAPPERFLAGS       16
void U_EMRSETMAPPERFLAGS_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMRSETMAPPERFLAGS pEmr = (PU_EMRSETMAPPERFLAGS)(record);
   U_swap4(&(pEmr->dwFlags),1);             // dwFlags
} 


// U_EMRSETMAPMODE           17
void U_EMRSETMAPMODE_swap(char *record, int torev){
   core3_swap(record, torev);
}

// U_EMRSETBKMODE            18
void U_EMRSETBKMODE_swap(char *record, int torev){
   core3_swap(record, torev);
}

// U_EMRSETPOLYFILLMODE      19
void U_EMRSETPOLYFILLMODE_swap(char *record, int torev){
   core3_swap(record, torev);
}

// U_EMRSETROP2              20
void U_EMRSETROP2_swap(char *record, int torev){
   core3_swap(record, torev);
}

// U_EMRSETSTRETCHBLTMODE    21
void U_EMRSETSTRETCHBLTMODE_swap(char *record, int torev){
   core3_swap(record, torev);
}

// U_EMRSETTEXTALIGN         22
void U_EMRSETTEXTALIGN_swap(char *record, int torev){
   core3_swap(record, torev);
}

// U_EMRSETCOLORADJUSTMENT   23
void U_EMRSETCOLORADJUSTMENT_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMRSETCOLORADJUSTMENT pEmr = (PU_EMRSETCOLORADJUSTMENT)(record);
   coloradjustment_swap(&(pEmr->ColorAdjustment));
}

// U_EMRSETTEXTCOLOR         24
void U_EMRSETTEXTCOLOR_swap(char *record, int torev){
   core5_swap(record, torev);
   // ordered bytes:                           crColor
}

// U_EMRSETBKCOLOR           25
void U_EMRSETBKCOLOR_swap(char *record, int torev){
   core5_swap(record, torev);
   // ordered bytes:                           crColor
}

// U_EMROFFSETCLIPRGN        26
void U_EMROFFSETCLIPRGN_swap(char *record, int torev){
   core7_swap(record, torev);
} 

// U_EMRMOVETOEX             27
void U_EMRMOVETOEX_swap(char *record, int torev){
   core7_swap(record, torev);
} 

// U_EMRSETMETARGN           28
void U_EMRSETMETARGN_swap(char *record, int torev){
   core5_swap(record, torev);
}

// U_EMREXCLUDECLIPRECT      29
void U_EMREXCLUDECLIPRECT_swap(char *record, int torev){
   core4_swap(record, torev);
}

// U_EMRINTERSECTCLIPRECT    30
void U_EMRINTERSECTCLIPRECT_swap(char *record, int torev){
   core4_swap(record, torev);
}

// U_EMRSCALEVIEWPORTEXTEX   31
void U_EMRSCALEVIEWPORTEXTEX_swap(char *record, int torev){
   core4_swap(record, torev);
}


// U_EMRSCALEWINDOWEXTEX     32
void U_EMRSCALEWINDOWEXTEX_swap(char *record, int torev){
   core4_swap(record, torev);
}

// U_EMRSAVEDC               33
void U_EMRSAVEDC_swap(char *record, int torev){
   core5_swap(record, torev);
}

// U_EMRRESTOREDC            34
void U_EMRRESTOREDC_swap(char *record, int torev){
   core3_swap(record, torev);
}

// U_EMRSETWORLDTRANSFORM    35
void U_EMRSETWORLDTRANSFORM_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMRSETWORLDTRANSFORM pEmr = (PU_EMRSETWORLDTRANSFORM)(record);
   xform_swap(&(pEmr->xform));
} 

// U_EMRMODIFYWORLDTRANSFORM 36
void U_EMRMODIFYWORLDTRANSFORM_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMRMODIFYWORLDTRANSFORM pEmr = (PU_EMRMODIFYWORLDTRANSFORM)(record);
   xform_swap(&(pEmr->xform));              // xform
   U_swap4(&(pEmr->iMode),1);               // iMode
} 

// U_EMRSELECTOBJECT         37
void U_EMRSELECTOBJECT_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMRSELECTOBJECT pEmr = (PU_EMRSELECTOBJECT)(record);
   U_swap4(&(pEmr->ihObject),1);            // ihObject
} 

// U_EMRCREATEPEN            38
void U_EMRCREATEPEN_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMRCREATEPEN pEmr = (PU_EMRCREATEPEN)(record);
   U_swap4(&(pEmr->ihPen),1);               // ihPen
   logpen_swap(&(pEmr->lopn));              // lopn
} 

// U_EMRCREATEBRUSHINDIRECT  39
void U_EMRCREATEBRUSHINDIRECT_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMRCREATEBRUSHINDIRECT pEmr = (PU_EMRCREATEBRUSHINDIRECT)(record);
   U_swap4(&(pEmr->ihBrush),1);             // ihBrush
   logbrush_swap(&(pEmr->lb));              // lb
} 

// U_EMRDELETEOBJECT         40
void U_EMRDELETEOBJECT_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMRDELETEOBJECT pEmr = (PU_EMRDELETEOBJECT)(record);
   U_swap4(&(pEmr->ihObject),1);            // ihObject
} 

// U_EMRANGLEARC             41
void U_EMRANGLEARC_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMRANGLEARC pEmr = (PU_EMRANGLEARC)(record);
   pointl_swap(&(pEmr->ptlCenter),1);       // ptlCenter
   U_swap4(&(pEmr->nRadius),3);             // nRadius eStartAngle eSweepAngle
} 

// U_EMRELLIPSE              42
void U_EMRELLIPSE_swap(char *record, int torev){
   core4_swap(record, torev);
}

// U_EMRRECTANGLE            43
void U_EMRRECTANGLE_swap(char *record, int torev){
   core4_swap(record, torev);
}

// U_EMRROUNDRECT            44
void U_EMRROUNDRECT_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMRROUNDRECT pEmr = (PU_EMRROUNDRECT)(record);
   rectl_swap(&(pEmr->rclBox),1);           // rclBox
   sizel_swap(&(pEmr->szlCorner), 1);       // szlCorner
}

// U_EMRARC                  45
void U_EMRARC_swap(char *record, int torev){
   core9_swap(record, torev);
}

// U_EMRCHORD                46
void U_EMRCHORD_swap(char *record, int torev){
   core9_swap(record, torev);
}

// U_EMRPIE                  47
void U_EMRPIE_swap(char *record, int torev){
   core9_swap(record, torev);
}

// U_EMRSELECTPALETTE        48
void U_EMRSELECTPALETTE_swap(char *record, int torev){
   core3_swap(record, torev);
}

// U_EMRCREATEPALETTE        49
void U_EMRCREATEPALETTE_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMRCREATEPALETTE pEmr = (PU_EMRCREATEPALETTE)(record);
   U_swap4(&(pEmr->ihPal),1);               // ihPal
   logpalette_swap( (PU_LOGPALETTE)&(pEmr->lgpl) ); // lgpl
}

// U_EMRSETPALETTEENTRIES    50
void U_EMRSETPALETTEENTRIES_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMRSETPALETTEENTRIES pEmr = (PU_EMRSETPALETTEENTRIES)(record);
   U_swap4(&(pEmr->ihPal),3);               // ihPal iStart cEntries
   // ordered bytes:                           aPalEntries[]
}

// U_EMRRESIZEPALETTE        51
void U_EMRRESIZEPALETTE_swap(char *record, int torev){
   core7_swap(record, torev);
} 

// U_EMRREALIZEPALETTE       52
void U_EMRREALIZEPALETTE_swap(char *record, int torev){
   core5_swap(record, torev);
}

// U_EMREXTFLOODFILL         53
void U_EMREXTFLOODFILL_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMREXTFLOODFILL pEmr = (PU_EMREXTFLOODFILL)(record);
   pointl_swap(&(pEmr->ptlStart),1);        // ptlStart
   // ordered bytes:                           crColor
   U_swap4(&(pEmr->iMode),1);               // iMode
}

// U_EMRLINETO               54
void U_EMRLINETO_swap(char *record, int torev){
   core7_swap(record, torev);
} 

// U_EMRARCTO                55
void U_EMRARCTO_swap(char *record, int torev){
   core9_swap(record, torev);
}

// U_EMRPOLYDRAW             56
void U_EMRPOLYDRAW_swap(char *record, int torev){
   int count=0;
   core5_swap(record, torev);
   PU_EMRPOLYDRAW pEmr = (PU_EMRPOLYDRAW)(record);
   
   if(torev){
      count = pEmr->cptl;
   }
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->cptl),1);                // cptl
   if(!torev){
      count = pEmr->cptl;
   }
   pointl_swap(pEmr->aptl,count);           // aptl[]
   U_swap4(pEmr->abTypes,count);            // abTypes[]
}

// U_EMRSETARCDIRECTION      57
void U_EMRSETARCDIRECTION_swap(char *record, int torev){
   core3_swap(record, torev);
}

// U_EMRSETMITERLIMIT        58
void U_EMRSETMITERLIMIT_swap(char *record, int torev){
   core3_swap(record, torev);
}


// U_EMRBEGINPATH            59
void U_EMRBEGINPATH_swap(char *record, int torev){
   core5_swap(record, torev);
}

// U_EMRENDPATH              60
void U_EMRENDPATH_swap(char *record, int torev){
   core5_swap(record, torev);
}

// U_EMRCLOSEFIGURE          61
void U_EMRCLOSEFIGURE_swap(char *record, int torev){
   core5_swap(record, torev);
}

// U_EMRFILLPATH             62
void U_EMRFILLPATH_swap(char *record, int torev){
   core4_swap(record, torev);
}

// U_EMRSTROKEANDFILLPATH    63
void U_EMRSTROKEANDFILLPATH_swap(char *record, int torev){
   core4_swap(record, torev);
}

// U_EMRSTROKEPATH           64
void U_EMRSTROKEPATH_swap(char *record, int torev){
   core4_swap(record, torev);
}

// U_EMRFLATTENPATH          65
void U_EMRFLATTENPATH_swap(char *record, int torev){
   core5_swap(record, torev);
}

// U_EMRWIDENPATH            66
void U_EMRWIDENPATH_swap(char *record, int torev){
   core5_swap(record, torev);
}

// U_EMRSELECTCLIPPATH       67
void U_EMRSELECTCLIPPATH_swap(char *record, int torev){
   core3_swap(record, torev);
}

// U_EMRABORTPATH            68
void U_EMRABORTPATH_swap(char *record, int torev){
   core5_swap(record, torev);
}

// U_EMRUNDEF69                       69
#define U_EMRUNDEF69_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.

// U_EMRCOMMENT              70  Comment (any binary data, interpretation is program specific)
void U_EMRCOMMENT_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMRCOMMENT pEmr = (PU_EMRCOMMENT)(record);
   U_swap4(&(pEmr->cbData),1);              // cbData
   // program specific data, presumably byte ordered, otherwise, not portable
} 

// U_EMRFILLRGN              71
void U_EMRFILLRGN_swap(char *record, int torev){
   int roff=0;
   int nextroff=0;
   int limit=0;
   roff=0;
   PU_EMRFILLRGN pEmr = (PU_EMRFILLRGN)(record);
   if(torev){
      limit    = pEmr->emr.nSize;
      nextroff = 0; 
   }
   core5_swap(record, torev);
   if(!torev){
      limit    = pEmr->emr.nSize;
   }
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->cbRgnData),2);           // cbRgnData ihBrush
   // This one is a pain since each RGNDATA may be a different size, so it isn't possible to index through them.
   char *prd = (char *) &(pEmr->RgnData);
   while(roff + 28 < limit){                // up to the end of the record
      if(torev){
         nextroff += (((PU_RGNDATA)prd)->rdh.dwSize + ((PU_RGNDATA)prd)->rdh.nRgnSize - 16);
         rgndata_swap((PU_RGNDATA) (prd + roff));
         roff  = nextroff;
      }
      else {
         rgndata_swap((PU_RGNDATA) (prd + roff));
         roff += (((PU_RGNDATA)prd)->rdh.dwSize + ((PU_RGNDATA)prd)->rdh.nRgnSize - 16);
      }
   }
} 

// U_EMRFRAMERGN             72
void U_EMRFRAMERGN_swap(char *record, int torev){
   int roff=0;
   int nextroff=0;
   int limit=0;
   PU_EMRFRAMERGN pEmr = (PU_EMRFRAMERGN)(record);
   roff = 0;
   if(torev){
      limit    = pEmr->emr.nSize;
      nextroff = 0; 
   }
   core5_swap(record, torev);
   if(!torev){
      limit    = pEmr->emr.nSize;
   }
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->cbRgnData),2);           // cbRgnData ihBrush
   sizel_swap(&(pEmr->szlStroke), 2);       // szlStroke
   // This one is a pain since each RGNDATA may be a different size, so it isn't possible to index through them.
   char *prd = (char *) &(pEmr->RgnData);
   while(roff + 28 < limit){                // up to the end of the record
      if(torev){
         nextroff += (((PU_RGNDATA)prd)->rdh.dwSize + ((PU_RGNDATA)prd)->rdh.nRgnSize - 16);
         rgndata_swap((PU_RGNDATA) (prd + roff));
         roff  = nextroff;
      }
      else {
         rgndata_swap((PU_RGNDATA) (prd + roff));
         roff += (((PU_RGNDATA)prd)->rdh.dwSize + ((PU_RGNDATA)prd)->rdh.nRgnSize - 16);
      }
   }
} 

// U_EMRINVERTRGN            73
void U_EMRINVERTRGN_swap(char *record, int torev){
   core11_swap(record, torev);
}

// U_EMRPAINTRGN             74
void U_EMRPAINTRGN_swap(char *record, int torev){
   core11_swap(record, torev);
}

// U_EMREXTSELECTCLIPRGN     75
void U_EMREXTSELECTCLIPRGN_swap(char *record, int torev){
   int roff=0;
   int nextroff=0;
   int limit=0;
   PU_EMREXTSELECTCLIPRGN pEmr = (PU_EMREXTSELECTCLIPRGN) (record);
   if(torev){
      limit    = pEmr->emr.nSize;
   }
   core5_swap(record, torev);
   if(!torev){
      limit    = pEmr->emr.nSize;
   }
   U_swap4(&(pEmr->cbRgnData),2);           // cbRgnData iMode
   // This one is a pain since each RGNDATA may be a different size, so it isn't possible to index through them.
   char *prd = (char *) &(pEmr->RgnData);
   nextroff = roff = 0;
   while(roff + 16 < limit){                // up to the end of the record
      if(torev){
         nextroff += (((PU_RGNDATA)prd)->rdh.dwSize + ((PU_RGNDATA)prd)->rdh.nRgnSize - 16);
         rgndata_swap((PU_RGNDATA) (prd + roff));
         roff  = nextroff;
      }
      else {
         rgndata_swap((PU_RGNDATA) (prd + roff));
         roff += (((PU_RGNDATA)prd)->rdh.dwSize + ((PU_RGNDATA)prd)->rdh.nRgnSize - 16);
      }
   }
} 

// U_EMRBITBLT               76
void U_EMRBITBLT_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMRBITBLT pEmr = (PU_EMRBITBLT) (record);
   if(torev && pEmr->cbBmiSrc){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmiSrc));
   }
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   pointl_swap(&(pEmr->Dest),2);            // Dest cDest
   U_swap4(&(pEmr->dwRop),1);               // dwRop
   pointl_swap(&(pEmr->Src),1);             // Src
   xform_swap(&(pEmr->xformSrc));           // xformSrc
   // ordered bytes:                           crBkColorSrc
   U_swap4(&(pEmr->iUsageSrc),5);           // iUsageSrc offBmiSrc cbBmiSrc offBitsSrc cbBitsSrc
   // ordered bytes:                           bitmap (including 16 bit 5bit/channel color mode, which is done bytewise).    
   if(!torev && pEmr->cbBmiSrc){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmiSrc));
   }
}

// U_EMRSTRETCHBLT           77
void U_EMRSTRETCHBLT_swap(char *record, int torev){
   PU_EMRSTRETCHBLT pEmr = (PU_EMRSTRETCHBLT) (record);
   core5_swap(record, torev);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   pointl_swap(&(pEmr->Dest),2);            // Dest cDest
   U_swap4(&(pEmr->dwRop),1);               // dwRop
   pointl_swap(&(pEmr->Src),1);             // Src
   xform_swap(&(pEmr->xformSrc));           // xformSrc
   // ordered bytes:                           crBkColorSrc
   if(torev && pEmr->cbBmiSrc){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmiSrc));
   }
   U_swap4(&(pEmr->iUsageSrc),5);           // iUsageSrc offBmiSrc cbBmiSrc offBitsSrc cbBitsSrc
   pointl_swap(&(pEmr->cSrc),1);            // cSrc
   if(!torev && pEmr->cbBmiSrc){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmiSrc));
   }
}

// U_EMRMASKBLT              78
void U_EMRMASKBLT_swap(char *record, int torev){
   PU_EMRMASKBLT pEmr = (PU_EMRMASKBLT) (record);
   core5_swap(record, torev);
   if(torev && pEmr->cbBmiSrc){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmiSrc));
   }
   if(torev && pEmr->cbBmiMask){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmiMask));
   }
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   pointl_swap(&(pEmr->Dest),2);            // Dest cDest
   U_swap4(&(pEmr->dwRop),1);               // dwRop
   pointl_swap(&(pEmr->Src),1);             // Src
   xform_swap(&(pEmr->xformSrc));           // xformSrc
   // ordered bytes:                           crBkColorSrc
   U_swap4(&(pEmr->iUsageSrc),5);           // iUsageSrc offBmiSrc cbBmiSrc offBitsSrc cbBitsSrc
   pointl_swap(&(pEmr->Mask),1);            // Mask
   U_swap4(&(pEmr->iUsageMask),5);          // iUsageMask offBmiMask cbBmiMask offBitsMask cbBitsMask
   if(!torev && pEmr->cbBmiSrc){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmiSrc));
   }
   if(!torev && pEmr->cbBmiMask){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmiMask));
   }
}

// U_EMRPLGBLT               79
void U_EMRPLGBLT_swap(char *record, int torev){
   PU_EMRPLGBLT pEmr = (PU_EMRPLGBLT) (record);
   core5_swap(record, torev);
   if(torev && pEmr->cbBmiSrc){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmiSrc));
   }
   if(torev && pEmr->cbBmiMask){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmiMask));
   }
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   pointl_swap(pEmr->aptlDst,3);            // aptlDst[]
   pointl_swap(&(pEmr->Src),2);             // Src cSrc
   xform_swap(&(pEmr->xformSrc));           // xformSrc
   // ordered bytes:                            crBkColorSrc
   U_swap4(&(pEmr->iUsageSrc),5);           // iUsageSrc offBmiSrc cbBmiSrc offBitsSrc cbBitsSrc
   pointl_swap(&(pEmr->Mask),1);            // Mask
   U_swap4(&(pEmr->iUsageMask),5);          // iUsageMask offBmiMask cbBmiMask offBitsMask cbBitsMask
   if(!torev && pEmr->cbBmiSrc){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmiSrc));
   }
   if(!torev && pEmr->cbBmiMask){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmiMask));
   }
}

// U_EMRSETDIBITSTODEVICE    80
void U_EMRSETDIBITSTODEVICE_swap(char *record, int torev){
   PU_EMRSETDIBITSTODEVICE pEmr = (PU_EMRSETDIBITSTODEVICE) (record);
   core5_swap(record, torev);
   if(torev && pEmr->cbBmiSrc){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmiSrc));
   }
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   pointl_swap(&(pEmr->Dest),1);            // Dest
   pointl_swap(&(pEmr->Src),2);             // Src cSrc
   U_swap4(&(pEmr->offBmiSrc),7);           // offBmiSrc cbBmiSrc offBitsSrc cbBitsSrc iUsageSrc iStartScan cScans
   if(!torev && pEmr->cbBmiSrc){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmiSrc));
   }
}

// U_EMRSTRETCHDIBITS        81
void U_EMRSTRETCHDIBITS_swap(char *record, int torev){
   PU_EMRSTRETCHDIBITS pEmr = (PU_EMRSTRETCHDIBITS) (record);
   core5_swap(record, torev);
   if(torev && pEmr->cbBmiSrc){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmiSrc));
   }
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   pointl_swap(&(pEmr->Dest),1);            // Dest
   pointl_swap(&(pEmr->Src),2);             // Src cSrc
   U_swap4(&(pEmr->offBmiSrc),6);           // offBmiSrc cbBmiSrc offBitsSrc cbBitsSrc iUsageSrc dwRop
   pointl_swap(&(pEmr->cDest),1);           // cDest
   if(!torev && pEmr->cbBmiSrc){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmiSrc));
   }
}

// U_EMREXTCREATEFONTINDIRECTW_swap    82
void U_EMREXTCREATEFONTINDIRECTW_swap(char *record, int torev){
   PU_EMREXTCREATEFONTINDIRECTW pEmr = (PU_EMREXTCREATEFONTINDIRECTW) (record);
   if(torev){
      if(pEmr->emr.nSize == sizeof(U_EMREXTCREATEFONTINDIRECTW)){
         logfont_panose_swap(&(pEmr->elfw));
      }
      else {
         logfont_swap( (PU_LOGFONT) &(pEmr->elfw));
      }
   }
   core5_swap(record, torev);
   if(!torev){
      if(pEmr->emr.nSize == sizeof(U_EMREXTCREATEFONTINDIRECTW)){
         logfont_panose_swap(&(pEmr->elfw));
      }
      else {
         logfont_swap( (PU_LOGFONT) &(pEmr->elfw));
      }
   }
   U_swap4(&(pEmr->ihFont),1);              // ihFont
}

// U_EMREXTTEXTOUTA          83
void U_EMREXTTEXTOUTA_swap(char *record, int torev){
   core8_swap(record, torev);
}

// U_EMREXTTEXTOUTW          84
void U_EMREXTTEXTOUTW_swap(char *record, int torev){
   core8_swap(record, torev);
}

// U_EMRPOLYBEZIER16         85
/**
    \brief Convert a pointer to a U_EMR_POLYBEZIER16 record.
    \param record   pointer to a buffer holding the EMR record
*/
void U_EMRPOLYBEZIER16_swap(char *record, int torev){
   core6_swap(record, torev);
}

// U_EMRPOLYGON16            86
void U_EMRPOLYGON16_swap(char *record, int torev){
   core6_swap(record, torev);
}

// U_EMRPOLYLINE16           87
void U_EMRPOLYLINE16_swap(char *record, int torev){
   core6_swap(record, torev);
}

// U_EMRPOLYBEZIERTO16       88
void U_EMRPOLYBEZIERTO16_swap(char *record, int torev){
   core6_swap(record, torev);
}

// U_EMRPOLYLINETO16         89
/**
    \brief Convert a pointer to a U_EMR_POLYLINETO16 record.
    \param record   pointer to a buffer holding the EMR record
*/
void U_EMRPOLYLINETO16_swap(char *record, int torev){
   core6_swap(record, torev);
}

// U_EMRPOLYPOLYLINE16       90
void U_EMRPOLYPOLYLINE16_swap(char *record, int torev){
   core10_swap(record, torev);
}

// U_EMRPOLYPOLYGON16        91
void U_EMRPOLYPOLYGON16_swap(char *record, int torev){
   core10_swap(record, torev);
}


// U_EMRPOLYDRAW16           92
void U_EMRPOLYDRAW16_swap(char *record, int torev){
   int count=0;
   core5_swap(record, torev);
   PU_EMRPOLYDRAW16 pEmr = (PU_EMRPOLYDRAW16)(record);
   if(torev){
      count = pEmr->cpts;
   }
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->cpts),1);                // cpts
   if(!torev){
      count = pEmr->cpts;
   }
   point16_swap(pEmr->apts,count);          // apts[]
   U_swap4(pEmr->abTypes,count);            // abTypes[]
}

// U_EMRCREATEMONOBRUSH      93
void U_EMRCREATEMONOBRUSH_swap(char *record, int torev){
   core12_swap(record, torev);
}

// U_EMRCREATEDIBPATTERNBRUSHPT_swap   94
void U_EMRCREATEDIBPATTERNBRUSHPT_swap(char *record, int torev){
   core12_swap(record, torev);
}


// U_EMREXTCREATEPEN         95
void U_EMREXTCREATEPEN_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMREXTCREATEPEN pEmr = (PU_EMREXTCREATEPEN)(record);
   if(torev && pEmr->cbBmi){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmi));
   }
   U_swap4(&(pEmr->ihPen),5);               // ihPen offBmi cbBmi offBits cbBits
   if(!torev && pEmr->cbBmi){
      bitmapinfo_swap((PU_BITMAPINFO)(record + pEmr->offBmi));
   }
   extlogpen_swap((PU_EXTLOGPEN) &(pEmr->elp), torev); 
}

// U_EMRPOLYTEXTOUTA         96 NOT IMPLEMENTED, denigrated after Windows NT
#define U_EMRPOLYTEXTOUTA_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.
// U_EMRPOLYTEXTOUTW         97 NOT IMPLEMENTED, denigrated after Windows NT
#define U_EMRPOLYTEXTOUTW_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.

// U_EMRSETICMMODE           98
void U_EMRSETICMMODE_swap(char *record, int torev){
   core3_swap(record, torev);
}

// U_EMRCREATECOLORSPACE     99
void U_EMRCREATECOLORSPACE_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMRCREATECOLORSPACE pEmr = (PU_EMRCREATECOLORSPACE)(record);
   U_swap4(&(pEmr->ihCS),1);                // ihCS
   logcolorspacea_swap(&(pEmr->lcs));       // lcs
}

// U_EMRSETCOLORSPACE       100
void U_EMRSETCOLORSPACE_swap(char *record, int torev){
   core3_swap(record, torev);
}

// U_EMRDELETECOLORSPACE    101
void U_EMRDELETECOLORSPACE_swap(char *record, int torev){
   core3_swap(record, torev);
}

// U_EMRGLSRECORD           102  Not implemented
#define U_EMRGLSRECORD_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.
// U_EMRGLSBOUNDEDRECORD    103  Not implemented
#define U_EMRGLSBOUNDEDRECORD_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.

// U_EMRPIXELFORMAT         104
void U_EMRPIXELFORMAT_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMRPIXELFORMAT pEmr = (PU_EMRPIXELFORMAT)(record);
   pixelformatdescriptor_swap(&(pEmr->pfd));     // pfd
}

// U_EMRDRAWESCAPE          105  Not implemented
#define U_EMRDRAWESCAPE_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.
// U_EMREXTESCAPE           106  Not implemented
#define U_EMREXTESCAPE_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.
// U_EMRUNDEF107            107  Not implemented
#define U_EMRUNDEF107_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.

// U_EMRSMALLTEXTOUT        108
void U_EMRSMALLTEXTOUT_swap(char *record, int torev){
   int roff=0;
   int fuOptions=0;
   core5_swap(record, torev);
   PU_EMRSMALLTEXTOUT pEmr = (PU_EMRSMALLTEXTOUT)(record);
   if(torev){
      fuOptions = pEmr->fuOptions;
   }
   pointl_swap(&(pEmr->Dest),1);            // Dest
   U_swap4(&(pEmr->cChars),5);              // cChars fuOptions iGraphicsMode exScale eyScale
   if(!torev){
      fuOptions = pEmr->fuOptions;
   }
   roff = sizeof(U_EMRSMALLTEXTOUT);        // offset to the start of the variable fields
   if(!(fuOptions & U_ETO_NO_RECT)){
      rectl_swap( (PU_RECTL) (record + roff),1);  // rclBounds
   }
   // ordered bytes or UTF16-LE                TextString
}

// U_EMRFORCEUFIMAPPING     109  Not implemented
#define U_EMRFORCEUFIMAPPING_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.
// U_EMRNAMEDESCAPE         110  Not implemented
#define U_EMRNAMEDESCAPE_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.
// U_EMRCOLORCORRECTPALETTE 111  Not implemented
#define U_EMRCOLORCORRECTPALETTE_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.
// U_EMRSETICMPROFILEA      112  Not implemented
#define U_EMRSETICMPROFILEA_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.
// U_EMRSETICMPROFILEW      113  Not implemented
#define U_EMRSETICMPROFILEW_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.

// U_EMRALPHABLEND          114
void U_EMRALPHABLEND_swap(char *record, int torev){
   core13_swap(record, torev);
}

// U_EMRSETLAYOUT           115
void U_EMRSETLAYOUT_swap(char *record, int torev){
   core3_swap(record, torev);
}

// U_EMRTRANSPARENTBLT      116
void U_EMRTRANSPARENTBLT_swap(char *record, int torev){
   core13_swap(record, torev);
}


// U_EMRUNDEF117            117  Not implemented
#define U_EMRUNDEF117_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.
// U_EMRGRADIENTFILL        118
void U_EMRGRADIENTFILL_swap(char *record, int torev){
   int nTriVert=0;
   int nGradObj=0;
   int ulMode=0;
   core5_swap(record, torev);
   PU_EMRGRADIENTFILL pEmr = (PU_EMRGRADIENTFILL)(record);
   if(torev){
      nTriVert = pEmr->nTriVert;
      nGradObj = pEmr->nGradObj;
      ulMode   = pEmr->ulMode;
   }
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->nTriVert),3);            // nTriVert nGradObj ulMode
   if(!torev){
      nTriVert = pEmr->nTriVert;
      nGradObj = pEmr->nGradObj;
      ulMode   = pEmr->ulMode;
   }
   record += sizeof(U_EMRGRADIENTFILL);
   if(nTriVert){
         trivertex_swap((PU_TRIVERTEX)(record),nTriVert);    // TriVert[]
   }
   record += nTriVert * sizeof(U_TRIVERTEX);
   if(nGradObj){
      if(     ulMode == U_GRADIENT_FILL_TRIANGLE){
         gradient3_swap((PU_GRADIENT3)(record), nGradObj);   // GradObj[]
      }
      else if(ulMode == U_GRADIENT_FILL_RECT_H || ulMode == U_GRADIENT_FILL_RECT_V){
         gradient4_swap((PU_GRADIENT4)(record), nGradObj);   // GradObj[]
      }
   }
}

// U_EMRSETLINKEDUFIS       119  Not implemented
#define U_EMRSETLINKEDUFIS_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.
// U_EMRSETTEXTJUSTIFICATION120  Not implemented (denigrated)
#define U_EMRSETTEXTJUSTIFICATION_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.
// U_EMRCOLORMATCHTOTARGETW 121  Not implemented  
#define U_EMRCOLORMATCHTOTARGETW_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.

// U_EMRCREATECOLORSPACEW   122
void U_EMRCREATECOLORSPACEW_swap(char *record, int torev){
   core5_swap(record, torev);
   PU_EMRCREATECOLORSPACEW pEmr = (PU_EMRCREATECOLORSPACEW)(record);
   U_swap4(&(pEmr->ihCS),1);                // ihCS
   logcolorspacew_swap(&(pEmr->lcs));       // lcs
   U_swap4(&(pEmr->dwFlags),2);             // dwFlags cbData
   // ordered bytes:                           Data
}

//! \endcond


/**
    \brief Convert an entire EMF in memory from Big Endian to Little Endian.
    \return 0 on failure, 1 on success
    \param contents   pointer to the buffer holding the entire EMF in memory
    \param length     number of bytes in the buffer
    \param torev      1 for native to reversed, 0 for reversed to native
    
    Normally this would be called immediately before writing the data to a file
    or immediately after reading the data from a file.
*/
int U_emf_endian(char *contents, size_t length, int torev){
    uint32_t  off;
    uint32_t  OK, recnum, iType;
    char     *record;
    PU_ENHMETARECORD  pEmr;

    record = contents;
    OK     = 1;
    off    = 0;
    recnum = 0;
    while(OK){
       if(record > contents + length){      // this is most likely a corrupt EMF
          return(0); 
       }

       pEmr = (PU_ENHMETARECORD)(record);

       iType = pEmr->iType;
       off   = pEmr->nSize;
       if(!torev){
          U_swap4(&iType,1);   
          U_swap4(&off,1);   
       }

       switch (iType)
       {
           case U_EMR_HEADER:                  U_EMRHEADER_swap(record, torev);                  break;
           case U_EMR_POLYBEZIER:              U_EMRPOLYBEZIER_swap(record, torev);              break;
           case U_EMR_POLYGON:                 U_EMRPOLYGON_swap(record, torev);                 break;
           case U_EMR_POLYLINE:                U_EMRPOLYLINE_swap(record, torev);                break;
           case U_EMR_POLYBEZIERTO:            U_EMRPOLYBEZIERTO_swap(record, torev);            break;
           case U_EMR_POLYLINETO:              U_EMRPOLYLINETO_swap(record, torev);              break;
           case U_EMR_POLYPOLYLINE:            U_EMRPOLYPOLYLINE_swap(record, torev);            break;
           case U_EMR_POLYPOLYGON:             U_EMRPOLYPOLYGON_swap(record, torev);             break;
           case U_EMR_SETWINDOWEXTEX:          U_EMRSETWINDOWEXTEX_swap(record, torev);          break;
           case U_EMR_SETWINDOWORGEX:          U_EMRSETWINDOWORGEX_swap(record, torev);          break;
           case U_EMR_SETVIEWPORTEXTEX:        U_EMRSETVIEWPORTEXTEX_swap(record, torev);        break;
           case U_EMR_SETVIEWPORTORGEX:        U_EMRSETVIEWPORTORGEX_swap(record, torev);        break;
           case U_EMR_SETBRUSHORGEX:           U_EMRSETBRUSHORGEX_swap(record, torev);           break;
           case U_EMR_EOF:
                                               U_EMREOF_swap(record, torev);
                                               OK = 0;    /* Exit triggered here */
                                                                                                 break;
           case U_EMR_SETPIXELV:               U_EMRSETPIXELV_swap(record, torev);               break;
           case U_EMR_SETMAPPERFLAGS:          U_EMRSETMAPPERFLAGS_swap(record, torev);          break;
           case U_EMR_SETMAPMODE:              U_EMRSETMAPMODE_swap(record, torev);              break;
           case U_EMR_SETBKMODE:               U_EMRSETBKMODE_swap(record, torev);               break;
           case U_EMR_SETPOLYFILLMODE:         U_EMRSETPOLYFILLMODE_swap(record, torev);         break;
           case U_EMR_SETROP2:                 U_EMRSETROP2_swap(record, torev);                 break;
           case U_EMR_SETSTRETCHBLTMODE:       U_EMRSETSTRETCHBLTMODE_swap(record, torev);       break;
           case U_EMR_SETTEXTALIGN:            U_EMRSETTEXTALIGN_swap(record, torev);            break;
           case U_EMR_SETCOLORADJUSTMENT:      U_EMRSETCOLORADJUSTMENT_swap(record, torev);      break;
           case U_EMR_SETTEXTCOLOR:            U_EMRSETTEXTCOLOR_swap(record, torev);            break;
           case U_EMR_SETBKCOLOR:              U_EMRSETBKCOLOR_swap(record, torev);              break;
           case U_EMR_OFFSETCLIPRGN:           U_EMROFFSETCLIPRGN_swap(record, torev);           break;
           case U_EMR_MOVETOEX:                U_EMRMOVETOEX_swap(record, torev);                break;
           case U_EMR_SETMETARGN:              U_EMRSETMETARGN_swap(record, torev);              break;
           case U_EMR_EXCLUDECLIPRECT:         U_EMREXCLUDECLIPRECT_swap(record, torev);         break;
           case U_EMR_INTERSECTCLIPRECT:       U_EMRINTERSECTCLIPRECT_swap(record, torev);       break;
           case U_EMR_SCALEVIEWPORTEXTEX:      U_EMRSCALEVIEWPORTEXTEX_swap(record, torev);      break;
           case U_EMR_SCALEWINDOWEXTEX:        U_EMRSCALEWINDOWEXTEX_swap(record, torev);        break;
           case U_EMR_SAVEDC:                  U_EMRSAVEDC_swap(record, torev);                  break;
           case U_EMR_RESTOREDC:               U_EMRRESTOREDC_swap(record, torev);               break;
           case U_EMR_SETWORLDTRANSFORM:       U_EMRSETWORLDTRANSFORM_swap(record, torev);       break;
           case U_EMR_MODIFYWORLDTRANSFORM:    U_EMRMODIFYWORLDTRANSFORM_swap(record, torev);    break;
           case U_EMR_SELECTOBJECT:            U_EMRSELECTOBJECT_swap(record, torev);            break;
           case U_EMR_CREATEPEN:               U_EMRCREATEPEN_swap(record, torev);               break;
           case U_EMR_CREATEBRUSHINDIRECT:     U_EMRCREATEBRUSHINDIRECT_swap(record, torev);     break;
           case U_EMR_DELETEOBJECT:            U_EMRDELETEOBJECT_swap(record, torev);            break;
           case U_EMR_ANGLEARC:                U_EMRANGLEARC_swap(record, torev);                break;
           case U_EMR_ELLIPSE:                 U_EMRELLIPSE_swap(record, torev);                 break;
           case U_EMR_RECTANGLE:               U_EMRRECTANGLE_swap(record, torev);               break;
           case U_EMR_ROUNDRECT:               U_EMRROUNDRECT_swap(record, torev);               break;
           case U_EMR_ARC:                     U_EMRARC_swap(record, torev);                     break;
           case U_EMR_CHORD:                   U_EMRCHORD_swap(record, torev);                   break;
           case U_EMR_PIE:                     U_EMRPIE_swap(record, torev);                     break;
           case U_EMR_SELECTPALETTE:           U_EMRSELECTPALETTE_swap(record, torev);           break;
           case U_EMR_CREATEPALETTE:           U_EMRCREATEPALETTE_swap(record, torev);           break;
           case U_EMR_SETPALETTEENTRIES:       U_EMRSETPALETTEENTRIES_swap(record, torev);       break;
           case U_EMR_RESIZEPALETTE:           U_EMRRESIZEPALETTE_swap(record, torev);           break;
           case U_EMR_REALIZEPALETTE:          U_EMRREALIZEPALETTE_swap(record, torev);          break;
           case U_EMR_EXTFLOODFILL:            U_EMREXTFLOODFILL_swap(record, torev);            break;
           case U_EMR_LINETO:                  U_EMRLINETO_swap(record, torev);                  break;
           case U_EMR_ARCTO:                   U_EMRARCTO_swap(record, torev);                   break;
           case U_EMR_POLYDRAW:                U_EMRPOLYDRAW_swap(record, torev);                break;
           case U_EMR_SETARCDIRECTION:         U_EMRSETARCDIRECTION_swap(record, torev);         break;
           case U_EMR_SETMITERLIMIT:           U_EMRSETMITERLIMIT_swap(record, torev);           break;
           case U_EMR_BEGINPATH:               U_EMRBEGINPATH_swap(record, torev);               break;
           case U_EMR_ENDPATH:                 U_EMRENDPATH_swap(record, torev);                 break;
           case U_EMR_CLOSEFIGURE:             U_EMRCLOSEFIGURE_swap(record, torev);             break;
           case U_EMR_FILLPATH:                U_EMRFILLPATH_swap(record, torev);                break;
           case U_EMR_STROKEANDFILLPATH:       U_EMRSTROKEANDFILLPATH_swap(record, torev);       break;
           case U_EMR_STROKEPATH:              U_EMRSTROKEPATH_swap(record, torev);              break;
           case U_EMR_FLATTENPATH:             U_EMRFLATTENPATH_swap(record, torev);             break;
           case U_EMR_WIDENPATH:               U_EMRWIDENPATH_swap(record, torev);               break;
           case U_EMR_SELECTCLIPPATH:          U_EMRSELECTCLIPPATH_swap(record, torev);          break;
           case U_EMR_ABORTPATH:               U_EMRABORTPATH_swap(record, torev);               break;
           case U_EMR_UNDEF69:                 U_EMRUNDEF69_swap(record, torev);                 break;
           case U_EMR_COMMENT:                 U_EMRCOMMENT_swap(record, torev);                 break;
           case U_EMR_FILLRGN:                 U_EMRFILLRGN_swap(record, torev);                 break;
           case U_EMR_FRAMERGN:                U_EMRFRAMERGN_swap(record, torev);                break;
           case U_EMR_INVERTRGN:               U_EMRINVERTRGN_swap(record, torev);               break;
           case U_EMR_PAINTRGN:                U_EMRPAINTRGN_swap(record, torev);                break;
           case U_EMR_EXTSELECTCLIPRGN:        U_EMREXTSELECTCLIPRGN_swap(record, torev);        break;
           case U_EMR_BITBLT:                  U_EMRBITBLT_swap(record, torev);                  break;
           case U_EMR_STRETCHBLT:              U_EMRSTRETCHBLT_swap(record, torev);              break;
           case U_EMR_MASKBLT:                 U_EMRMASKBLT_swap(record, torev);                 break;
           case U_EMR_PLGBLT:                  U_EMRPLGBLT_swap(record, torev);                  break;
           case U_EMR_SETDIBITSTODEVICE:       U_EMRSETDIBITSTODEVICE_swap(record, torev);       break;
           case U_EMR_STRETCHDIBITS:           U_EMRSTRETCHDIBITS_swap(record, torev);           break;
           case U_EMR_EXTCREATEFONTINDIRECTW:  U_EMREXTCREATEFONTINDIRECTW_swap(record, torev);  break;
           case U_EMR_EXTTEXTOUTA:             U_EMREXTTEXTOUTA_swap(record, torev);             break;
           case U_EMR_EXTTEXTOUTW:             U_EMREXTTEXTOUTW_swap(record, torev);             break;
           case U_EMR_POLYBEZIER16:            U_EMRPOLYBEZIER16_swap(record, torev);            break;
           case U_EMR_POLYGON16:               U_EMRPOLYGON16_swap(record, torev);               break;
           case U_EMR_POLYLINE16:              U_EMRPOLYLINE16_swap(record, torev);              break;
           case U_EMR_POLYBEZIERTO16:          U_EMRPOLYBEZIERTO16_swap(record, torev);          break;
           case U_EMR_POLYLINETO16:            U_EMRPOLYLINETO16_swap(record, torev);            break;
           case U_EMR_POLYPOLYLINE16:          U_EMRPOLYPOLYLINE16_swap(record, torev);          break;
           case U_EMR_POLYPOLYGON16:           U_EMRPOLYPOLYGON16_swap(record, torev);           break;
           case U_EMR_POLYDRAW16:              U_EMRPOLYDRAW16_swap(record, torev);              break;
           case U_EMR_CREATEMONOBRUSH:         U_EMRCREATEMONOBRUSH_swap(record, torev);         break;
           case U_EMR_CREATEDIBPATTERNBRUSHPT: U_EMRCREATEDIBPATTERNBRUSHPT_swap(record, torev); break;
           case U_EMR_EXTCREATEPEN:            U_EMREXTCREATEPEN_swap(record, torev);            break;
           case U_EMR_POLYTEXTOUTA:            U_EMRPOLYTEXTOUTA_swap(record, torev);            break;
           case U_EMR_POLYTEXTOUTW:            U_EMRPOLYTEXTOUTW_swap(record, torev);            break;
           case U_EMR_SETICMMODE:              U_EMRSETICMMODE_swap(record, torev);              break;
           case U_EMR_CREATECOLORSPACE:        U_EMRCREATECOLORSPACE_swap(record, torev);        break;
           case U_EMR_SETCOLORSPACE:           U_EMRSETCOLORSPACE_swap(record, torev);           break;
           case U_EMR_DELETECOLORSPACE:        U_EMRDELETECOLORSPACE_swap(record, torev);        break;
           case U_EMR_GLSRECORD:               U_EMRGLSRECORD_swap(record, torev);               break;
           case U_EMR_GLSBOUNDEDRECORD:        U_EMRGLSBOUNDEDRECORD_swap(record, torev);        break;
           case U_EMR_PIXELFORMAT:             U_EMRPIXELFORMAT_swap(record, torev);             break;
           case U_EMR_DRAWESCAPE:              U_EMRDRAWESCAPE_swap(record, torev);              break;
           case U_EMR_EXTESCAPE:               U_EMREXTESCAPE_swap(record, torev);               break;
           case U_EMR_UNDEF107:                U_EMRUNDEF107_swap(record, torev);                break;
           case U_EMR_SMALLTEXTOUT:            U_EMRSMALLTEXTOUT_swap(record, torev);            break;
           case U_EMR_FORCEUFIMAPPING:         U_EMRFORCEUFIMAPPING_swap(record, torev);         break;
           case U_EMR_NAMEDESCAPE:             U_EMRNAMEDESCAPE_swap(record, torev);             break;
           case U_EMR_COLORCORRECTPALETTE:     U_EMRCOLORCORRECTPALETTE_swap(record, torev);     break;
           case U_EMR_SETICMPROFILEA:          U_EMRSETICMPROFILEA_swap(record, torev);          break;
           case U_EMR_SETICMPROFILEW:          U_EMRSETICMPROFILEW_swap(record, torev);          break;
           case U_EMR_ALPHABLEND:              U_EMRALPHABLEND_swap(record, torev);              break;
           case U_EMR_SETLAYOUT:               U_EMRSETLAYOUT_swap(record, torev);               break;
           case U_EMR_TRANSPARENTBLT:          U_EMRTRANSPARENTBLT_swap(record, torev);          break;
           case U_EMR_UNDEF117:                U_EMRUNDEF117_swap(record, torev);                break;
           case U_EMR_GRADIENTFILL:            U_EMRGRADIENTFILL_swap(record, torev);            break;
           case U_EMR_SETLINKEDUFIS:           U_EMRSETLINKEDUFIS_swap(record, torev);           break;
           case U_EMR_SETTEXTJUSTIFICATION:    U_EMRSETTEXTJUSTIFICATION_swap(record, torev);    break;
           case U_EMR_COLORMATCHTOTARGETW:     U_EMRCOLORMATCHTOTARGETW_swap(record, torev);     break;
           case U_EMR_CREATECOLORSPACEW:       U_EMRCREATECOLORSPACEW_swap(record, torev);       break;
           default:                            U_EMRNOTIMPLEMENTED_swap(record, torev);          break;
       }  //end of switch
       record += off;
       recnum++;
    }  //end of while

    return(1);
}

#ifdef __cplusplus
}
#endif
