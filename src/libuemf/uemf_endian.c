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
Version:   0.0.21
Date:      23-APR-2015
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2015 David Mathog and California Institute of Technology (Caltech)
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
      const char *Bmi
   ){
   PU_BITMAPINFO pBmi = (PU_BITMAPINFO)Bmi;
   bitmapinfoheader_swap(&(pBmi->bmiHeader)); // bmIHeader
   // ordered bytes:                            bmiColors
}

/**
    \brief Swap the ordered bytes in a DIB and verify that the sizes are OK
    
    \return 1 on success, 0 on failure
    \param record     EMF record that contains a DIB pixel array
    \param iUsage     DIBcolors Enumeration
    \param offBmi     offset from the start of the record to the start of the bitmapinfo structure
    \param cbBmi      declared space for the bitmapinfo structure in the record
    \param offBits    offset from the start of the record to the start of the bitmap
    \param cbBits     declared space for the bitmap in the record (amount used may be less than this)
    \param blimit     one byte past the end of the record.
    \param torev      1 for native to reversed, 0 for reversed to native

    This method can only test DIBs that hold Microsoft's various bitmap types.  PNG or JPG is just a bag
    of bytes, and there is no possible way to derive from the known width and height how big it should be.
*/
int DIB_swap(
       const char      *record,
       uint32_t         iUsage,
       uint32_t         offBmi,
       uint32_t         cbBmi,
       uint32_t         offBits,
       uint32_t         cbBits,
       const char      *blimit,
       int              torev
   ){
   int  dibparams = U_BI_UNKNOWN;       // type of image not yet determined
   const char      *px      = NULL;     // DIB pixels
   const U_RGBQUAD *ct      = NULL;     // DIB color table
   int              bs;
   int              usedbytes;

   if(!cbBmi)return(1);  // No DIB in a record where it is optional
   if(IS_MEM_UNSAFE(record, offBmi + cbBmi, blimit))return(0);
   if(cbBits && IS_MEM_UNSAFE(record, offBits + cbBits, blimit))return(0);
   if(iUsage == U_DIB_RGB_COLORS){
       uint32_t width, height, colortype, numCt, invert; // these values will be set in get_DIB_params
       // next call returns pointers and values, but allocates no memory
       if(torev){
           dibparams = get_DIB_params(record, offBits, offBmi, &px, (const U_RGBQUAD **) &ct,
           &numCt, &width, &height, &colortype, &invert);
       }
       bitmapinfo_swap(record + offBmi);  // byte ordered fields in bitmapinfo
       if(!torev){
           dibparams = get_DIB_params(record, offBits, offBmi, &px, (const U_RGBQUAD **) &ct,
           &numCt, &width, &height, &colortype, &invert);
       }

       // sanity checking
       if(numCt && colortype  >= U_BCBM_COLOR16)return(0);  //color tables not used above 16 bit pixels
       if(!numCt && colortype < U_BCBM_COLOR16)return(0);   //color tables mandatory for < 16 bit
 
       if(dibparams ==U_BI_RGB){  
           // this is the only DIB type where we can calculate how big it should be when stored in the EMF file
           bs = colortype/8;
           if(bs<1){
              usedbytes = (width*colortype + 7)/8;      // width of line in fully and partially occupied bytes
           }
           else {
              usedbytes = width*bs;
           }
           if(IS_MEM_UNSAFE(record+offBits, usedbytes, blimit))return(0);
       }
   }
   else {
      bitmapinfo_swap(record + offBmi);
   }
   return(1);
}

/**
    \brief Convert a pointer to a U_EXTLOGPEN object.
    \param elp    PU_EXTLOGPEN object
    \param blimit one byte past the end of the record
*/
int extlogpen_swap(
      PU_EXTLOGPEN elp,
      const char *blimit,
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
   if(IS_MEM_UNSAFE(&(elp->elpStyleEntry), count*4, blimit))return(0);
   U_swap4(&(elp->elpStyleEntry),count);    // elpStyleEntry[]
   return(1);
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
    \return 1 on success, 0 on failure
    \brief Convert a pointer to a U_RGNDATA object.
    \param rd  pointer to a U_RGNDATA object.
    \param cbRgnData size of the U_RGNDATA object.
*/
int rgndata_swap(
      PU_RGNDATA rd,
      int cbRgnData,
      int torev
   ){
   int count = 0;
   if(torev){
      count = rd->rdh.nCount;
   }
   rgndataheader_swap(&(rd->rdh));
   if(!torev){
      count = rd->rdh.nCount;
   }
   if(4*count + (int)sizeof(U_RGNDATAHEADER) > cbRgnData)return(0);
   U_swap4(rd->Buffer,4*count);
   return(1);
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
    \param blimit    one byte past the end of the record.
    \param torev     1 for native to reversed, 0 for reversed to native
*/
int emrtext_swap(
      PU_EMRTEXT  pemt,
      char       *record,
      const char *blimit,
      int         torev
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
       if(IS_MEM_UNSAFE(pemt, sizeof(U_RECTL), blimit))return(0);
       rectl_swap((PU_RECTL)((char *)pemt + off),1);  // optional rectangle
       off+=sizeof(U_RECTL);
   }
   if(torev){
      offDx = *(uint32_t *)((char *)pemt +off);
   }
   // ordered bytes OR UTF16-LE:               the string at offString
   if(IS_MEM_UNSAFE(pemt, off + 4, blimit))return(0);
   U_swap4(((char *)pemt+off),1);           // offDx
   if(!torev){
      offDx = *(uint32_t *)((char *)pemt +off);
   }
   if(IS_MEM_UNSAFE(record, count*4, blimit))return(0);
   U_swap4((record + offDx),count);           // Dx[], offset with respect to the Record, NOT the object
   return(1);
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
int core5_swap(char *record, int torev){
   UNUSED_PARAMETER(torev);
   if(!record)return(0);
   PU_EMR pEmr = (PU_EMR)(record);
   U_swap4(pEmr,2);                         // iType nSize
   return(1);
}

// Functions with the same form starting with U_EMRPOLYBEZIER_swap
int core1_swap(char *record, int torev){
   int count=0;
   const char *blimit = NULL;
   PU_EMRPOLYLINETO pEmr = (PU_EMRPOLYLINETO) (record);
   if(torev){
      count = pEmr->cptl;
      blimit = record + pEmr->emr.nSize;
   }
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBounds),1 );       // rclBounds
   U_swap4(&(pEmr->cptl),1);                // cptl
   if(!torev){
      count = pEmr->cptl;
      blimit = record + pEmr->emr.nSize;
   }
   if(IS_MEM_UNSAFE((pEmr->aptl), count*sizeof(U_POINTL), blimit))return(0);
   pointl_swap((pEmr->aptl),count);         // aptl[]
   return(1);
}

// Functions with the same form starting with U_EMRPOLYPOLYLINE_swap
int core2_swap(char *record, int torev){
   int count=0;
   int nPolys=0;
   const char *blimit = NULL;
   PU_EMRPOLYPOLYLINE pEmr = (PU_EMRPOLYPOLYLINE) (record);
   if(torev){
      count  = pEmr->cptl;
      nPolys = pEmr->nPolys;
      blimit = record + pEmr->emr.nSize;
   }
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->nPolys),2);              // nPolys cptl
   if(!torev){
      count  = pEmr->cptl;
      nPolys = pEmr->nPolys;
      blimit = record + pEmr->emr.nSize;
   }
   if(IS_MEM_UNSAFE((pEmr->aPolyCounts), nPolys*4, blimit))return(0);
   U_swap4(pEmr->aPolyCounts,nPolys);       // aPolyCounts[]
   record += sizeof(U_EMRPOLYPOLYLINE) - 4 + sizeof(uint32_t)* nPolys;
   if(IS_MEM_UNSAFE(record, count*sizeof(U_POINTL), blimit))return(0);
   pointl_swap((PU_POINT)(record), count); // paptl[]
   return(1);
}


// Functions with the same form starting with U_EMRSETMAPMODE_swap
int core3_swap(char *record, int torev){
   PU_EMRSETMAPMODE pEmr   = (PU_EMRSETMAPMODE)(record);
   if(!core5_swap(record, torev))return(0);
   U_swap4(&(pEmr->iMode),1);               // iMode
   return(1);
} 

// Functions taking a single U_RECT or U_RECTL, starting with U_EMRELLIPSE_swap, also U_EMRFILLPATH_swap, 
int core4_swap(char *record, int torev){
   PU_EMRELLIPSE pEmr      = (PU_EMRELLIPSE)(   record);
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBox),1);           // rclBox
   return(1);
} 

// Functions with the same form starting with U_EMRPOLYBEZIER16_swap
int core6_swap(char *record, int torev){
   int count=0;
   const char *blimit = NULL;
   PU_EMRPOLYBEZIER16 pEmr = (PU_EMRPOLYBEZIER16) (record);
   if(torev){
      count = pEmr->cpts;
      blimit = record + pEmr->emr.nSize;
   }
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->cpts),1);                // cpts
   if(!torev){
      count = pEmr->cpts;
      blimit = record + pEmr->emr.nSize;
   }
   if(IS_MEM_UNSAFE((pEmr->apts), count*sizeof(U_POINT16), blimit))return(0);
   point16_swap((pEmr->apts),count);        // apts[]
   return(1);
} 


// Records with the same form starting with U_EMRSETWINDOWEXTEX_swap, that is, all with two uint32_t values after the emr
int core7_swap(char *record, int torev){
   PU_EMRGENERICPAIR pEmr = (PU_EMRGENERICPAIR) (record);
   if(!core5_swap(record, torev))return(0);
   U_swap4(&(pEmr->pair),2);
   return(1);
}

// For U_EMREXTTEXTOUTA and U_EMREXTTEXTOUTW, type=0 for the first one
int core8_swap(char *record, int torev){
   const char *blimit = NULL;
   PU_EMREXTTEXTOUTA pEmr = (PU_EMREXTTEXTOUTA) (record);
   if(torev){
      blimit = record + pEmr->emr.nSize;
   }
   if(!core5_swap(record, torev))return(0);
   U_swap4(&(pEmr->iGraphicsMode),1);       // iGraphicsMode
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->exScale),2);             // exScale eyScale
   if(!torev){
      blimit = record + pEmr->emr.nSize;
   }
   if(!emrtext_swap(&(pEmr->emrtext),record,blimit,torev))return(0);
   return(1);
} 

// Functions that take a rect and a pair of points, starting with U_EMRARC_swap
int core9_swap(char *record, int torev){
   PU_EMRARC pEmr = (PU_EMRARC) (record);
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBox),1);           // rclBox
   U_swap4(&(pEmr->ptlStart),4);            // ptlStart ptlEnd
   return(1);
}

// Functions with the same form starting with U_EMRPOLYPOLYLINE16_swap
int core10_swap(char *record, int torev){
   int count=0;
   int nPolys=0;
   const char *blimit = NULL;
   PU_EMRPOLYPOLYLINE16 pEmr = (PU_EMRPOLYPOLYLINE16) (record);
   if(torev){
      count  = pEmr->cpts;
      nPolys = pEmr->nPolys;
      blimit = record + pEmr->emr.nSize;
   }
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->nPolys),2);              // nPolys cpts
   if(!torev){
      count  = pEmr->cpts;
      nPolys = pEmr->nPolys;
      blimit = record + pEmr->emr.nSize;
   }
   if(IS_MEM_UNSAFE((pEmr->aPolyCounts), nPolys*4, blimit))return(0);
   U_swap4(pEmr->aPolyCounts,nPolys);       // aPolyCounts[]
   record += sizeof(U_EMRPOLYPOLYLINE16) - 4 + sizeof(uint32_t)* nPolys;
   if(IS_MEM_UNSAFE(record, count*sizeof(U_POINT16), blimit))return(0);
   point16_swap((PU_POINT16)(record), count); // apts[]
   return(1);
} 

// Functions with the same form starting with  U_EMRINVERTRGN_swap and U_EMRPAINTRGN_swap,
int core11_swap(char *record, int torev){
   int cbRgnData=0;
   const char *blimit = NULL;
   PU_EMRINVERTRGN pEmr = (PU_EMRINVERTRGN)(record);
   if(torev){
      cbRgnData= pEmr->cbRgnData;
      blimit = record + pEmr->emr.nSize;
   }
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->cbRgnData),1);           // cbRgnData
   if(!torev){
      cbRgnData= pEmr->cbRgnData;
      blimit = record + pEmr->emr.nSize;
   }
   if(IS_MEM_UNSAFE(pEmr->RgnData, cbRgnData, blimit))return(0);
   return(rgndata_swap(pEmr->RgnData, cbRgnData, torev));
} 


// common code for U_EMRCREATEMONOBRUSH_swap and U_EMRCREATEDIBPATTERNBRUSHPT_swap,
int core12_swap(char *record, int torev){
   const char *blimit = NULL;
   U_OFFBMI  offBmi  = 0;
   U_CBBMI   cbBmi   = 0; 
   U_OFFBITS offBits = 0;
   U_CBBITS  cbBits  = 0;
   uint32_t  iUsage  = 0;
   PU_EMRCREATEMONOBRUSH pEmr = (PU_EMRCREATEMONOBRUSH) (record);
   if(torev){
      offBmi  = pEmr->offBmi;
      cbBmi   = pEmr->cbBmi;
      offBits = pEmr->offBits;
      cbBits  = pEmr->cbBits;
      iUsage  = pEmr->iUsage;
      blimit  = record + pEmr->emr.nSize;
      if(!DIB_swap(record, iUsage, offBmi, cbBmi, offBits, cbBits, blimit, torev))return(0);
   }
   if(!core5_swap(record, torev))return(0);
   U_swap4(&(pEmr->ihBrush),6);             // ihBrush iUsage offBmi cbBmi offBits cbBits
   if(!torev){
      offBmi  = pEmr->offBmi;
      cbBmi   = pEmr->cbBmi;
      offBits = pEmr->offBits;
      cbBits  = pEmr->cbBits;
      iUsage  = pEmr->iUsage;
      blimit  = record + pEmr->emr.nSize;
      if(!DIB_swap(record, iUsage, offBmi, cbBmi, offBits, cbBits, blimit, torev))return(0);
   }
   // ordered bytes:                           bitmap (including 16 bit 5bit/channel color mode, which is done bytewise).    
   return(1);
}

// common code for U_EMRALPHABLEND_swap and U_EMRTRANSPARENTBLT_swap,
int core13_swap(char *record, int torev){
   const char *blimit = NULL;
   U_OFFBMISRC  offBmiSrc  = 0;
   U_CBBMISRC   cbBmiSrc   = 0; 
   U_OFFBITSSRC offBitsSrc = 0;
   U_CBBITSSRC  cbBitsSrc  = 0;
   uint32_t     iUsageSrc  = 0;
   PU_EMRALPHABLEND pEmr = (PU_EMRALPHABLEND) (record);
   if(torev){
      offBmiSrc  = pEmr->offBmiSrc;
      cbBmiSrc   = pEmr->cbBmiSrc;
      offBitsSrc = pEmr->offBitsSrc;
      cbBitsSrc  = pEmr->cbBitsSrc;
      iUsageSrc  = pEmr->iUsageSrc;
      blimit     = record + pEmr->emr.nSize;
      if(!DIB_swap(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit, torev))return(0);
   }
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   pointl_swap(&(pEmr->Dest),2);            // Dest cDest
   pointl_swap(&(pEmr->Dest),2);            // Dest cDest
   // ordered bytes:                           Blend
   pointl_swap(&(pEmr->Src),2);             // Src
   xform_swap( &(pEmr->xformSrc));          // xformSrc
   // ordered bytes:                           crBkColorSrc
   U_swap4(&(pEmr->iUsageSrc),5);           // iUsageSrc offBmiSrc cbBmiSrc offBitsSrc cbBitsSrc
   if(!torev){
      offBmiSrc  = pEmr->offBmiSrc;
      cbBmiSrc   = pEmr->cbBmiSrc;
      offBitsSrc = pEmr->offBitsSrc;
      cbBitsSrc  = pEmr->cbBitsSrc;
      iUsageSrc  = pEmr->iUsageSrc;
      blimit     = record + pEmr->emr.nSize;
      if(!DIB_swap(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit, torev))return(0);
   }
   // ordered bytes:                           bitmap (including 16 bit 5bit/channel color mode, which is done bytewise).    
   return(1);
}

/* **********************************************************************************************
These are the core EMR_swap functions, each converts a particular type of record.  
All operate in place on the chunk of memory holding that record.
Some of these have offsets or counts which, if corrupt or evil would result in access outside
  the record.  These cases return a status value of 0 if that happens, 1 on success.  Other
  records which do not have these issues do not return a status value.
They are listed in order by the corresponding U_EMR_* index number.  
*********************************************************************************************** */

/**
    All of the record level (hidden) functions have this form:
    \brief Convert a pointer to a U_EMR_whatever record which has not been implemented.
    \param record   pointer to a buffer holding the EMR record
    \param torev    1 for native to reversed, 0 for reversed to native
*/
int U_EMRNOTIMPLEMENTED_swap(char *record, int torev){
   fprintf(stderr,"EMF WARNING:  could not swap data bytes on record because that type has not been implemented!\n");
   return(core5_swap(record, torev));
}

// U_EMRHEADER                1
int U_EMRHEADER_swap(char *record, int torev){
   int nDesc,offDesc,nSize,cbPix,offPix;
   nDesc = offDesc = nSize = cbPix = offPix = 0;
   PU_EMRHEADER pEmr = (PU_EMRHEADER)(record);
   if(torev){
     nSize = pEmr->emr.nSize;
     nDesc = pEmr->nDescription;
     offDesc = pEmr->offDescription;
   }
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBounds),2);        // rclBounds rclFrame
   U_swap4(&(pEmr->dSignature), 4);         // dSignature nVersion nBytes nRecords
   U_swap2(&(pEmr->nHandles), 2);           // nHandlessReserved
   U_swap4(&(pEmr->nDescription), 3);       // nDescription offDescription nPalEntries 
   if(!torev){
     nSize = pEmr->emr.nSize;
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
     }
     U_swap4(&(pEmr->cbPixelFormat), 2);      // cbPixelFormat offPixelFormat 
     U_swap4(&(pEmr->bOpenGL), 1);            // bOpenGL
     if(!torev){
        cbPix = pEmr->cbPixelFormat;
        offPix = pEmr->offPixelFormat;
     }
     if(cbPix)pixelformatdescriptor_swap( (PU_PIXELFORMATDESCRIPTOR) (record + pEmr->offPixelFormat));
     if((nDesc && (offDesc >= 108)) || 
        (cbPix && (offPix >=108)) ||
        (!offDesc && !cbPix && nSize >= 108)
       ){
         sizel_swap(&(pEmr->szlMicrometers), 1);  // szlMicrometers
       }
   }
   return(1);
}

// U_EMRPOLYBEZIER                       2
int U_EMRPOLYBEZIER_swap(char *record, int torev){
   return(core1_swap(record, torev));
} 

// U_EMRPOLYGON                          3
int U_EMRPOLYGON_swap(char *record, int torev){
   return(core1_swap(record, torev));
} 


// U_EMRPOLYLINE              4
int U_EMRPOLYLINE_swap(char *record, int torev){
   return(core1_swap(record, torev));
} 

// U_EMRPOLYBEZIERTO          5
int U_EMRPOLYBEZIERTO_swap(char *record, int torev){
   return(core1_swap(record, torev));
} 

// U_EMRPOLYLINETO            6
int U_EMRPOLYLINETO_swap(char *record, int torev){
   return(core1_swap(record, torev));
} 

// U_EMRPOLYPOLYLINE          7
int U_EMRPOLYPOLYLINE_swap(char *record, int torev){
   return(core2_swap(record, torev));
} 

// U_EMRPOLYPOLYGON           8
int U_EMRPOLYPOLYGON_swap(char *record, int torev){
   return(core2_swap(record, torev));
} 

// U_EMRSETWINDOWEXTEX        9
int U_EMRSETWINDOWEXTEX_swap(char *record, int torev){
   return(core7_swap(record, torev));
} 

// U_EMRSETWINDOWORGEX       10
int U_EMRSETWINDOWORGEX_swap(char *record, int torev){
   return(core7_swap(record, torev));
} 

// U_EMRSETVIEWPORTEXTEX     11
int U_EMRSETVIEWPORTEXTEX_swap(char *record, int torev){
   return(core7_swap(record, torev));
} 

// U_EMRSETVIEWPORTORGEX     12
int U_EMRSETVIEWPORTORGEX_swap(char *record, int torev){
   return(core7_swap(record, torev));
} 

// U_EMRSETBRUSHORGEX        13
int U_EMRSETBRUSHORGEX_swap(char *record, int torev){
   return(core7_swap(record, torev));
} 

// U_EMREOF                  14
int U_EMREOF_swap(char *record, int torev){
   int off=0;
   int cbPalEntries=0;
   const char *blimit = NULL;
   PU_EMREOF pEmr = (PU_EMREOF)(record);
   if(torev){
      blimit = record + pEmr->emr.nSize;
      cbPalEntries = pEmr->cbPalEntries;
   }
   if(!core5_swap(record, torev))return(0);
   U_swap4(&(pEmr->cbPalEntries),2);        // cbPalEntries offPalEntries
   if(!torev){
      blimit = record + pEmr->emr.nSize;
      cbPalEntries = pEmr->cbPalEntries;
   }
   if(cbPalEntries){
      if(IS_MEM_UNSAFE(record, pEmr->offPalEntries + 2*2, blimit))return(0); // 2 16 bit values in U_LOGPALLETE
      logpalette_swap( (PU_LOGPALETTE)(record + pEmr->offPalEntries));
      // U_LOGPLTNTRY values in pallette are ordered data
   }
   off = sizeof(U_EMREOF) + 4 * cbPalEntries;
   if(IS_MEM_UNSAFE(record, off + 4, blimit))return(0);
   U_swap4(record + off,1);                 // nSizeLast
   return(1);
} 


// U_EMRSETPIXELV            15
int U_EMRSETPIXELV_swap(char *record, int torev){
   if(!core5_swap(record, torev))return(0);
   PU_EMRSETPIXELV pEmr = (PU_EMRSETPIXELV)(record);
   pointl_swap(&(pEmr->ptlPixel),1);        // ptlPixel
   // ordered bytes:                           crColor
   return(1);
} 


// U_EMRSETMAPPERFLAGS       16
int U_EMRSETMAPPERFLAGS_swap(char *record, int torev){
   if(!core5_swap(record, torev))return(0);
   PU_EMRSETMAPPERFLAGS pEmr = (PU_EMRSETMAPPERFLAGS)(record);
   U_swap4(&(pEmr->dwFlags),1);             // dwFlags
   return(1);
} 


// U_EMRSETMAPMODE           17
int U_EMRSETMAPMODE_swap(char *record, int torev){
   return(core3_swap(record, torev));
}

// U_EMRSETBKMODE            18
int U_EMRSETBKMODE_swap(char *record, int torev){
   return(core3_swap(record, torev));
}

// U_EMRSETPOLYFILLMODE      19
int U_EMRSETPOLYFILLMODE_swap(char *record, int torev){
   return(core3_swap(record, torev));
}

// U_EMRSETROP2              20
int U_EMRSETROP2_swap(char *record, int torev){
   return(core3_swap(record, torev));
}

// U_EMRSETSTRETCHBLTMODE    21
int U_EMRSETSTRETCHBLTMODE_swap(char *record, int torev){
   return(core3_swap(record, torev));
}

// U_EMRSETTEXTALIGN         22
int U_EMRSETTEXTALIGN_swap(char *record, int torev){
   return(core3_swap(record, torev));
}

// U_EMRSETCOLORADJUSTMENT   23
int U_EMRSETCOLORADJUSTMENT_swap(char *record, int torev){
   if(!core5_swap(record, torev))return(0);
   PU_EMRSETCOLORADJUSTMENT pEmr = (PU_EMRSETCOLORADJUSTMENT)(record);
   coloradjustment_swap(&(pEmr->ColorAdjustment));
   return(1);
}

// U_EMRSETTEXTCOLOR         24
int U_EMRSETTEXTCOLOR_swap(char *record, int torev){
   if(!core5_swap(record, torev))return(0);
   // ordered bytes:                           crColor
   return(1);
}

// U_EMRSETBKCOLOR           25
int U_EMRSETBKCOLOR_swap(char *record, int torev){
   if(!core5_swap(record, torev))return(0);
   // ordered bytes:                           crColor
   return(1);
}

// U_EMROFFSETCLIPRGN        26
int U_EMROFFSETCLIPRGN_swap(char *record, int torev){
   return(core7_swap(record, torev));
} 

// U_EMRMOVETOEX             27
int U_EMRMOVETOEX_swap(char *record, int torev){
   return(core7_swap(record, torev));
} 

// U_EMRSETMETARGN           28
int U_EMRSETMETARGN_swap(char *record, int torev){
   return(core5_swap(record, torev));
}

// U_EMREXCLUDECLIPRECT      29
int U_EMREXCLUDECLIPRECT_swap(char *record, int torev){
   return(core4_swap(record, torev));
}

// U_EMRINTERSECTCLIPRECT    30
int U_EMRINTERSECTCLIPRECT_swap(char *record, int torev){
   return(core4_swap(record, torev));
}

// U_EMRSCALEVIEWPORTEXTEX   31
int U_EMRSCALEVIEWPORTEXTEX_swap(char *record, int torev){
   return(core4_swap(record, torev));
}

// U_EMRSCALEWINDOWEXTEX     32
int U_EMRSCALEWINDOWEXTEX_swap(char *record, int torev){
   return(core4_swap(record, torev));
}

// U_EMRSAVEDC               33
int U_EMRSAVEDC_swap(char *record, int torev){
   return(core5_swap(record, torev));
}

// U_EMRRESTOREDC            34
int U_EMRRESTOREDC_swap(char *record, int torev){
   return(core3_swap(record, torev));
}

// U_EMRSETWORLDTRANSFORM    35
int U_EMRSETWORLDTRANSFORM_swap(char *record, int torev){
   if(!core5_swap(record, torev))return(0);
   PU_EMRSETWORLDTRANSFORM pEmr = (PU_EMRSETWORLDTRANSFORM)(record);
   xform_swap(&(pEmr->xform));
   return(1);
} 

// U_EMRMODIFYWORLDTRANSFORM 36
int U_EMRMODIFYWORLDTRANSFORM_swap(char *record, int torev){
   if(!core5_swap(record, torev))return(0);
   PU_EMRMODIFYWORLDTRANSFORM pEmr = (PU_EMRMODIFYWORLDTRANSFORM)(record);
   xform_swap(&(pEmr->xform));              // xform
   U_swap4(&(pEmr->iMode),1);               // iMode
   return(1);
} 

// U_EMRSELECTOBJECT         37
int U_EMRSELECTOBJECT_swap(char *record, int torev){
   if(!core5_swap(record, torev))return(0);
   PU_EMRSELECTOBJECT pEmr = (PU_EMRSELECTOBJECT)(record);
   U_swap4(&(pEmr->ihObject),1);            // ihObject
   return(1);
} 

// U_EMRCREATEPEN            38
int U_EMRCREATEPEN_swap(char *record, int torev){
   if(!core5_swap(record, torev))return(0);
   PU_EMRCREATEPEN pEmr = (PU_EMRCREATEPEN)(record);
   U_swap4(&(pEmr->ihPen),1);               // ihPen
   logpen_swap(&(pEmr->lopn));              // lopn
   return(1);
} 

// U_EMRCREATEBRUSHINDIRECT  39
int U_EMRCREATEBRUSHINDIRECT_swap(char *record, int torev){
   if(!core5_swap(record, torev))return(0);
   PU_EMRCREATEBRUSHINDIRECT pEmr = (PU_EMRCREATEBRUSHINDIRECT)(record);
   U_swap4(&(pEmr->ihBrush),1);             // ihBrush
   logbrush_swap(&(pEmr->lb));              // lb
   return(1);
} 

// U_EMRDELETEOBJECT         40
int U_EMRDELETEOBJECT_swap(char *record, int torev){
   if(!core5_swap(record, torev))return(0);
   PU_EMRDELETEOBJECT pEmr = (PU_EMRDELETEOBJECT)(record);
   U_swap4(&(pEmr->ihObject),1);            // ihObject
   return(1);
} 

// U_EMRANGLEARC             41
int U_EMRANGLEARC_swap(char *record, int torev){
   if(!core5_swap(record, torev))return(0);
   PU_EMRANGLEARC pEmr = (PU_EMRANGLEARC)(record);
   pointl_swap(&(pEmr->ptlCenter),1);       // ptlCenter
   U_swap4(&(pEmr->nRadius),3);             // nRadius eStartAngle eSweepAngle
   return(1);
} 

// U_EMRELLIPSE              42
int U_EMRELLIPSE_swap(char *record, int torev){
   return(core4_swap(record, torev));
}

// U_EMRRECTANGLE            43
int U_EMRRECTANGLE_swap(char *record, int torev){
   return(core4_swap(record, torev));
}

// U_EMRROUNDRECT            44
int U_EMRROUNDRECT_swap(char *record, int torev){
   if(!core5_swap(record, torev))return(0);
   PU_EMRROUNDRECT pEmr = (PU_EMRROUNDRECT)(record);
   rectl_swap(&(pEmr->rclBox),1);           // rclBox
   sizel_swap(&(pEmr->szlCorner), 1);       // szlCorner
   return(1);
}

// U_EMRARC                  45
int U_EMRARC_swap(char *record, int torev){
   return(core9_swap(record, torev));
}

// U_EMRCHORD                46
int U_EMRCHORD_swap(char *record, int torev){
   return(core9_swap(record, torev));
}

// U_EMRPIE                  47
int U_EMRPIE_swap(char *record, int torev){
   return(core9_swap(record, torev));
}

// U_EMRSELECTPALETTE        48
int U_EMRSELECTPALETTE_swap(char *record, int torev){
   return(core3_swap(record, torev));
}

// U_EMRCREATEPALETTE        49
int U_EMRCREATEPALETTE_swap(char *record, int torev){
   if(!core5_swap(record, torev))return(0);
   PU_EMRCREATEPALETTE pEmr = (PU_EMRCREATEPALETTE)(record);
   U_swap4(&(pEmr->ihPal),1);               // ihPal
   logpalette_swap( (PU_LOGPALETTE)&(pEmr->lgpl) ); // lgpl
   return(1);
}

// U_EMRSETPALETTEENTRIES    50
int U_EMRSETPALETTEENTRIES_swap(char *record, int torev){
   if(!core5_swap(record, torev))return(0);
   PU_EMRSETPALETTEENTRIES pEmr = (PU_EMRSETPALETTEENTRIES)(record);
   U_swap4(&(pEmr->ihPal),3);               // ihPal iStart cEntries
   // ordered bytes:                           aPalEntries[]
   return(1);
}

// U_EMRRESIZEPALETTE        51
int U_EMRRESIZEPALETTE_swap(char *record, int torev){
   return(core7_swap(record, torev));
} 

// U_EMRREALIZEPALETTE       52
int U_EMRREALIZEPALETTE_swap(char *record, int torev){
   return(core5_swap(record, torev));
}

// U_EMREXTFLOODFILL         53
int U_EMREXTFLOODFILL_swap(char *record, int torev){
   if(!core5_swap(record, torev))return(0);
   PU_EMREXTFLOODFILL pEmr = (PU_EMREXTFLOODFILL)(record);
   pointl_swap(&(pEmr->ptlStart),1);        // ptlStart
   // ordered bytes:                           crColor
   U_swap4(&(pEmr->iMode),1);               // iMode
   return(1);
}

// U_EMRLINETO               54
int U_EMRLINETO_swap(char *record, int torev){
   return(core7_swap(record, torev));
} 

// U_EMRARCTO                55
int U_EMRARCTO_swap(char *record, int torev){
   return(core9_swap(record, torev));
}

// U_EMRPOLYDRAW             56
int U_EMRPOLYDRAW_swap(char *record, int torev){
   int count=0;
   const char *blimit = NULL;
   PU_EMRPOLYDRAW pEmr = (PU_EMRPOLYDRAW)(record);
   
   if(torev){
      count = pEmr->cptl;
      blimit = record + pEmr->emr.nSize;
   }
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->cptl),1);                // cptl
   if(!torev){
      count = pEmr->cptl;
      blimit = record + pEmr->emr.nSize;
   }
   if(IS_MEM_UNSAFE((pEmr->aptl), count*sizeof(U_POINTL), blimit))return(0);
   pointl_swap(pEmr->aptl,count);           // aptl[]
   // single byte data                         abTypes
   return(1);
}

// U_EMRSETARCDIRECTION      57
int U_EMRSETARCDIRECTION_swap(char *record, int torev){
   return(core3_swap(record, torev));
}

// U_EMRSETMITERLIMIT        58
int U_EMRSETMITERLIMIT_swap(char *record, int torev){
   return(core3_swap(record, torev));
}


// U_EMRBEGINPATH            59
int U_EMRBEGINPATH_swap(char *record, int torev){
   return(core5_swap(record, torev));
}

// U_EMRENDPATH              60
int U_EMRENDPATH_swap(char *record, int torev){
   return(core5_swap(record, torev));
}

// U_EMRCLOSEFIGURE          61
int U_EMRCLOSEFIGURE_swap(char *record, int torev){
   return(core5_swap(record, torev));
}

// U_EMRFILLPATH             62
int U_EMRFILLPATH_swap(char *record, int torev){
   return(core4_swap(record, torev));
}

// U_EMRSTROKEANDFILLPATH    63
int U_EMRSTROKEANDFILLPATH_swap(char *record, int torev){
   return(core4_swap(record, torev));
}

// U_EMRSTROKEPATH           64
int U_EMRSTROKEPATH_swap(char *record, int torev){
   return(core4_swap(record, torev));
}

// U_EMRFLATTENPATH          65
int U_EMRFLATTENPATH_swap(char *record, int torev){
   return(core5_swap(record, torev));
}

// U_EMRWIDENPATH            66
int U_EMRWIDENPATH_swap(char *record, int torev){
   return(core5_swap(record, torev));
}

// U_EMRSELECTCLIPPATH       67
int U_EMRSELECTCLIPPATH_swap(char *record, int torev){
   return(core3_swap(record, torev));
}

// U_EMRABORTPATH            68
int U_EMRABORTPATH_swap(char *record, int torev){
   return(core5_swap(record, torev));
}

// U_EMRUNDEF69                       69
#define U_EMRUNDEF69_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.

// U_EMRCOMMENT              70  Comment (any binary data, interpretation is program specific)
int U_EMRCOMMENT_swap(char *record, int torev){
   int cbData = 0;
   const char *blimit = NULL;
   PU_EMRCOMMENT pEmr = (PU_EMRCOMMENT)(record);
   if(torev){
      cbData = pEmr->cbData;
      blimit = record + pEmr->emr.nSize;
   }
   if(!core5_swap(record, torev))return(0);
   U_swap4(&(pEmr->cbData),1);              // cbData
   if(!torev){
      cbData = pEmr->cbData;
      blimit = record + pEmr->emr.nSize;
   }
   if(IS_MEM_UNSAFE(record, cbData + sizeof(U_SIZE_EMRCOMMENT), blimit))return(0);
   // program specific data, presumably byte ordered, otherwise, not portable
   return(1);
} 

// U_EMRFILLRGN              71
int U_EMRFILLRGN_swap(char *record, int torev){
   int cbRgnData=0;
   const char *blimit = NULL;
   PU_EMRFILLRGN pEmr = (PU_EMRFILLRGN)(record);
   if(torev){
      cbRgnData= pEmr->cbRgnData;
      blimit = record + pEmr->emr.nSize;
   }
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->cbRgnData),2);           // cbRgnData ihBrush
   if(!torev){
      cbRgnData= pEmr->cbRgnData;
      blimit = record + pEmr->emr.nSize;
   }
   if(IS_MEM_UNSAFE(pEmr->RgnData, cbRgnData, blimit))return(0);
   return(rgndata_swap(pEmr->RgnData, cbRgnData, torev));
} 

// U_EMRFRAMERGN             72
int U_EMRFRAMERGN_swap(char *record, int torev){
   int cbRgnData=0;
   const char *blimit = NULL;
   PU_EMRFRAMERGN pEmr = (PU_EMRFRAMERGN)(record);
   if(torev){
      cbRgnData= pEmr->cbRgnData;
      blimit = record + pEmr->emr.nSize;
   }
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->cbRgnData),2);           // cbRgnData ihBrush
   sizel_swap(&(pEmr->szlStroke), 1);       // szlStroke
   if(!torev){
      cbRgnData= pEmr->cbRgnData;
      blimit = record + pEmr->emr.nSize;
   }
   if(IS_MEM_UNSAFE(pEmr->RgnData, cbRgnData, blimit))return(0);
   return(rgndata_swap(pEmr->RgnData, cbRgnData, torev));
} 

// U_EMRINVERTRGN            73
int U_EMRINVERTRGN_swap(char *record, int torev){
   return(core11_swap(record, torev));
}

// U_EMRPAINTRGN             74
int U_EMRPAINTRGN_swap(char *record, int torev){
   return(core11_swap(record, torev));
}

// U_EMREXTSELECTCLIPRGN     75
int U_EMREXTSELECTCLIPRGN_swap(char *record, int torev){
   int cbRgnData=0;
   const char *blimit = NULL;
   PU_EMREXTSELECTCLIPRGN pEmr = (PU_EMREXTSELECTCLIPRGN)(record);
   if(torev){
      cbRgnData= pEmr->cbRgnData;
      blimit = record + pEmr->emr.nSize;
   }
   if(!core5_swap(record, torev))return(0);
   U_swap4(&(pEmr->cbRgnData),2);           // cbRgnData iMode
   if(!torev){
      cbRgnData= pEmr->cbRgnData;
      blimit = record + pEmr->emr.nSize;
   }
   if(IS_MEM_UNSAFE(pEmr->RgnData, cbRgnData, blimit))return(0);
   return(rgndata_swap(pEmr->RgnData, cbRgnData, torev));
} 

// U_EMRBITBLT               76
int U_EMRBITBLT_swap(char *record, int torev){
   const char *blimit = NULL;
   U_OFFBMISRC  offBmiSrc  = 0;
   U_CBBMISRC   cbBmiSrc   = 0; 
   U_OFFBITSSRC offBitsSrc = 0;
   U_CBBITSSRC  cbBitsSrc  = 0;
   uint32_t     iUsageSrc  = 0;
   PU_EMRBITBLT pEmr = (PU_EMRBITBLT) (record);
   if(torev){
      offBmiSrc  = pEmr->offBmiSrc;
      cbBmiSrc   = pEmr->cbBmiSrc;
      offBitsSrc = pEmr->offBitsSrc;
      cbBitsSrc  = pEmr->cbBitsSrc;
      iUsageSrc  = pEmr->iUsageSrc;
      blimit     = record + pEmr->emr.nSize;
      if(!DIB_swap(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit, torev))return(0);
   }
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   pointl_swap(&(pEmr->Dest),2);            // Dest cDest
   U_swap4(&(pEmr->dwRop),1);               // dwRop
   pointl_swap(&(pEmr->Src),1);             // Src
   xform_swap(&(pEmr->xformSrc));           // xformSrc
   // ordered bytes:                           crBkColorSrc
   U_swap4(&(pEmr->iUsageSrc),5);           // iUsageSrc offBmiSrc cbBmiSrc offBitsSrc cbBitsSrc
   if(!torev){
      offBmiSrc  = pEmr->offBmiSrc;
      cbBmiSrc   = pEmr->cbBmiSrc;
      offBitsSrc = pEmr->offBitsSrc;
      cbBitsSrc  = pEmr->cbBitsSrc;
      iUsageSrc  = pEmr->iUsageSrc;
      blimit     = record + pEmr->emr.nSize;
      if(!DIB_swap(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit, torev))return(0);
   }
   // ordered bytes:                           bitmap (including 16 bit 5bit/channel color mode, which is done bytewise).    
   return(1);
}

// U_EMRSTRETCHBLT           77
int U_EMRSTRETCHBLT_swap(char *record, int torev){
   const char *blimit = NULL;
   U_OFFBMISRC  offBmiSrc  = 0;
   U_CBBMISRC   cbBmiSrc   = 0; 
   U_OFFBITSSRC offBitsSrc = 0;
   U_CBBITSSRC  cbBitsSrc  = 0;
   uint32_t     iUsageSrc  = 0;
   PU_EMRSTRETCHBLT pEmr = (PU_EMRSTRETCHBLT) (record);
   if(torev){
      offBmiSrc  = pEmr->offBmiSrc;
      cbBmiSrc   = pEmr->cbBmiSrc;
      offBitsSrc = pEmr->offBitsSrc;
      cbBitsSrc  = pEmr->cbBitsSrc;
      iUsageSrc  = pEmr->iUsageSrc;
      blimit     = record + pEmr->emr.nSize;
      if(!DIB_swap(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit, torev))return(0);
   }
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   pointl_swap(&(pEmr->Dest),2);            // Dest cDest
   U_swap4(&(pEmr->dwRop),1);               // dwRop
   pointl_swap(&(pEmr->Src),1);             // Src
   xform_swap(&(pEmr->xformSrc));           // xformSrc
   // ordered bytes:                           crBkColorSrc
   U_swap4(&(pEmr->iUsageSrc),5);           // iUsageSrc offBmiSrc cbBmiSrc offBitsSrc cbBitsSrc
   pointl_swap(&(pEmr->cSrc),1);            // cSrc
   if(!torev){
      offBmiSrc  = pEmr->offBmiSrc;
      cbBmiSrc   = pEmr->cbBmiSrc;
      offBitsSrc = pEmr->offBitsSrc;
      cbBitsSrc  = pEmr->cbBitsSrc;
      iUsageSrc  = pEmr->iUsageSrc;
      blimit     = record + pEmr->emr.nSize;
      if(!DIB_swap(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit, torev))return(0);
   }
   // ordered bytes:                           bitmap (including 16 bit 5bit/channel color mode, which is done bytewise).    
   return(1);
}

// U_EMRMASKBLT              78
int U_EMRMASKBLT_swap(char *record, int torev){
   const char *blimit = NULL;
   U_OFFBMISRC  offBmiSrc   = 0;
   U_CBBMISRC   cbBmiSrc    = 0; 
   U_OFFBITSSRC offBitsSrc  = 0;
   U_CBBITSSRC  cbBitsSrc   = 0;
   uint32_t     iUsageSrc   = 0;
   U_OFFBMIMSK  offBmiMask  = 0;
   U_CBBMIMSK   cbBmiMask   = 0; 
   U_OFFBITSMSK offBitsMask = 0;
   U_CBBITSMSK  cbBitsMask  = 0;
   uint32_t     iUsageMask  = 0;
   PU_EMRMASKBLT pEmr = (PU_EMRMASKBLT) (record);
   if(torev){
      offBmiSrc   = pEmr->offBmiSrc;
      cbBmiSrc    = pEmr->cbBmiSrc;
      offBitsSrc  = pEmr->offBitsSrc;
      cbBitsSrc   = pEmr->cbBitsSrc;
      iUsageSrc   = pEmr->iUsageSrc;
      offBmiMask  = pEmr->offBmiMask;
      cbBmiMask   = pEmr->cbBmiMask;
      offBitsMask = pEmr->offBitsMask;
      cbBitsMask  = pEmr->cbBitsMask;
      iUsageMask  = pEmr->iUsageMask;
      blimit      = record + pEmr->emr.nSize;
      if(!DIB_swap(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit, torev))return(0);
      if(!DIB_swap(record, iUsageMask, offBmiMask, cbBmiMask, offBitsMask, cbBitsMask, blimit, torev))return(0);
   }
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   pointl_swap(&(pEmr->Dest),2);            // Dest cDest
   U_swap4(&(pEmr->dwRop),1);               // dwRop
   pointl_swap(&(pEmr->Src),1);             // Src
   xform_swap(&(pEmr->xformSrc));           // xformSrc
   // ordered bytes:                           crBkColorSrc
   U_swap4(&(pEmr->iUsageSrc),5);           // iUsageSrc offBmiSrc cbBmiSrc offBitsSrc cbBitsSrc
   pointl_swap(&(pEmr->Mask),1);            // Mask
   U_swap4(&(pEmr->iUsageMask),5);          // iUsageMask offBmiMask cbBmiMask offBitsMask cbBitsMask
   if(!torev){
      offBmiSrc   = pEmr->offBmiSrc;
      cbBmiSrc    = pEmr->cbBmiSrc;
      offBitsSrc  = pEmr->offBitsSrc;
      cbBitsSrc   = pEmr->cbBitsSrc;
      iUsageSrc   = pEmr->iUsageSrc;
      offBmiMask  = pEmr->offBmiMask;
      cbBmiMask   = pEmr->cbBmiMask;
      offBitsMask = pEmr->offBitsMask;
      cbBitsMask  = pEmr->cbBitsMask;
      iUsageMask  = pEmr->iUsageMask;
      blimit      = record + pEmr->emr.nSize;
      if(!DIB_swap(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit, torev))return(0);
      if(!DIB_swap(record, iUsageMask, offBmiMask, cbBmiMask, offBitsMask, cbBitsMask, blimit, torev))return(0);
   }
   return(1);
}

// U_EMRPLGBLT               79
int U_EMRPLGBLT_swap(char *record, int torev){
   const char *blimit = NULL;
   U_OFFBMISRC  offBmiSrc   = 0;
   U_CBBMISRC   cbBmiSrc    = 0; 
   U_OFFBITSSRC offBitsSrc  = 0;
   U_CBBITSSRC  cbBitsSrc   = 0;
   uint32_t     iUsageSrc   = 0;
   U_OFFBMIMSK  offBmiMask  = 0;
   U_CBBMIMSK   cbBmiMask   = 0; 
   U_OFFBITSMSK offBitsMask = 0;
   U_CBBITSMSK  cbBitsMask  = 0;
   uint32_t     iUsageMask  = 0;
   PU_EMRPLGBLT pEmr = (PU_EMRPLGBLT) (record);
   if(torev){
      offBmiSrc   = pEmr->offBmiSrc;
      cbBmiSrc    = pEmr->cbBmiSrc;
      offBitsSrc  = pEmr->offBitsSrc;
      cbBitsSrc   = pEmr->cbBitsSrc;
      iUsageSrc   = pEmr->iUsageSrc;
      offBmiMask  = pEmr->offBmiMask;
      cbBmiMask   = pEmr->cbBmiMask;
      offBitsMask = pEmr->offBitsMask;
      cbBitsMask  = pEmr->cbBitsMask;
      iUsageMask  = pEmr->iUsageMask;
      blimit      = record + pEmr->emr.nSize;
      if(!DIB_swap(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit, torev))return(0);
      if(!DIB_swap(record, iUsageMask, offBmiMask, cbBmiMask, offBitsMask, cbBitsMask, blimit, torev))return(0);
   }
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   pointl_swap(pEmr->aptlDst,3);            // aptlDst[]
   pointl_swap(&(pEmr->Src),2);             // Src cSrc
   xform_swap(&(pEmr->xformSrc));           // xformSrc
   // ordered bytes:                            crBkColorSrc
   U_swap4(&(pEmr->iUsageSrc),5);           // iUsageSrc offBmiSrc cbBmiSrc offBitsSrc cbBitsSrc
   pointl_swap(&(pEmr->Mask),1);            // Mask
   U_swap4(&(pEmr->iUsageMask),5);          // iUsageMask offBmiMask cbBmiMask offBitsMask cbBitsMask
   if(!torev){
      offBmiSrc   = pEmr->offBmiSrc;
      cbBmiSrc    = pEmr->cbBmiSrc;
      offBitsSrc  = pEmr->offBitsSrc;
      cbBitsSrc   = pEmr->cbBitsSrc;
      iUsageSrc   = pEmr->iUsageSrc;
      offBmiMask  = pEmr->offBmiMask;
      cbBmiMask   = pEmr->cbBmiMask;
      offBitsMask = pEmr->offBitsMask;
      cbBitsMask  = pEmr->cbBitsMask;
      iUsageMask  = pEmr->iUsageMask;
      blimit      = record + pEmr->emr.nSize;
      if(!DIB_swap(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit, torev))return(0);
      if(!DIB_swap(record, iUsageMask, offBmiMask, cbBmiMask, offBitsMask, cbBitsMask, blimit, torev))return(0);
   }
   return(1);
}

// U_EMRSETDIBITSTODEVICE    80
int U_EMRSETDIBITSTODEVICE_swap(char *record, int torev){
   const char *blimit = NULL;
   U_OFFBMISRC  offBmiSrc   = 0;
   U_CBBMISRC   cbBmiSrc    = 0; 
   U_OFFBITSSRC offBitsSrc  = 0;
   U_CBBITSSRC  cbBitsSrc   = 0;
   uint32_t     iUsageSrc   = 0;
   PU_EMRSETDIBITSTODEVICE pEmr = (PU_EMRSETDIBITSTODEVICE) (record);
   if(torev){
      offBmiSrc   = pEmr->offBmiSrc;
      cbBmiSrc    = pEmr->cbBmiSrc;
      offBitsSrc  = pEmr->offBitsSrc;
      cbBitsSrc   = pEmr->cbBitsSrc;
      iUsageSrc   = pEmr->iUsageSrc;
      blimit      = record + pEmr->emr.nSize;
      if(!DIB_swap(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit, torev))return(0);
   }
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   pointl_swap(&(pEmr->Dest),1);            // Dest
   pointl_swap(&(pEmr->Src),2);             // Src cSrc
   U_swap4(&(pEmr->offBmiSrc),7);           // offBmiSrc cbBmiSrc offBitsSrc cbBitsSrc iUsageSrc iStartScan cScans
   if(!torev){
      offBmiSrc   = pEmr->offBmiSrc;
      cbBmiSrc    = pEmr->cbBmiSrc;
      offBitsSrc  = pEmr->offBitsSrc;
      cbBitsSrc   = pEmr->cbBitsSrc;
      iUsageSrc   = pEmr->iUsageSrc;
      blimit      = record + pEmr->emr.nSize;
      if(!DIB_swap(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit, torev))return(0);
   }
   return(1);
}

// U_EMRSTRETCHDIBITS        81
int U_EMRSTRETCHDIBITS_swap(char *record, int torev){
   const char *blimit = NULL;
   U_OFFBMISRC  offBmiSrc   = 0;
   U_CBBMISRC   cbBmiSrc    = 0; 
   U_OFFBITSSRC offBitsSrc  = 0;
   U_CBBITSSRC  cbBitsSrc   = 0;
   uint32_t     iUsageSrc   = 0;
   PU_EMRSTRETCHDIBITS pEmr = (PU_EMRSTRETCHDIBITS) (record);
   if(torev){
      offBmiSrc   = pEmr->offBmiSrc;
      cbBmiSrc    = pEmr->cbBmiSrc;
      offBitsSrc  = pEmr->offBitsSrc;
      cbBitsSrc   = pEmr->cbBitsSrc;
      iUsageSrc   = pEmr->iUsageSrc;
      blimit      = record + pEmr->emr.nSize;
      if(!DIB_swap(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit, torev))return(0);
   }
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   pointl_swap(&(pEmr->Dest),1);            // Dest
   pointl_swap(&(pEmr->Src),2);             // Src cSrc
   U_swap4(&(pEmr->offBmiSrc),6);           // offBmiSrc cbBmiSrc offBitsSrc cbBitsSrc iUsageSrc dwRop
   pointl_swap(&(pEmr->cDest),1);           // cDest
   if(!torev){
      offBmiSrc   = pEmr->offBmiSrc;
      cbBmiSrc    = pEmr->cbBmiSrc;
      offBitsSrc  = pEmr->offBitsSrc;
      cbBitsSrc   = pEmr->cbBitsSrc;
      iUsageSrc   = pEmr->iUsageSrc;
      blimit      = record + pEmr->emr.nSize;
      if(!DIB_swap(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit, torev))return(0);
   }
   return(1);
}

// U_EMREXTCREATEFONTINDIRECTW    82
int U_EMREXTCREATEFONTINDIRECTW_swap(char *record, int torev){
   int nSize = 0;
   PU_EMREXTCREATEFONTINDIRECTW pEmr = (PU_EMREXTCREATEFONTINDIRECTW) (record);
   if(torev){
      nSize = pEmr->emr.nSize;
   }
   if(!core5_swap(record, torev))return(0);
   U_swap4(&(pEmr->ihFont),1);              // ihFont
   if(!torev){
      nSize = pEmr->emr.nSize;
   }
   if(nSize == U_SIZE_EMREXTCREATEFONTINDIRECTW_LOGFONT_PANOSE){
      logfont_panose_swap(&(pEmr->elfw));
   }
   else { // logfont or logfontExDv (which starts with logfont, which can be swapped, and the rest is already in byte order
      logfont_swap( (PU_LOGFONT) &(pEmr->elfw));
   }
   return(1);
}

// U_EMREXTTEXTOUTA          83
int U_EMREXTTEXTOUTA_swap(char *record, int torev){
   return(core8_swap(record, torev));
}

// U_EMREXTTEXTOUTW          84
int U_EMREXTTEXTOUTW_swap(char *record, int torev){
   return(core8_swap(record, torev));
}

// U_EMRPOLYBEZIER16         85
/**
    \brief Convert a pointer to a U_EMR_POLYBEZIER16 record.
    \param record   pointer to a buffer holding the EMR record
*/
int U_EMRPOLYBEZIER16_swap(char *record, int torev){
   return(core6_swap(record, torev));
}

// U_EMRPOLYGON16            86
int U_EMRPOLYGON16_swap(char *record, int torev){
   return(core6_swap(record, torev));
}

// U_EMRPOLYLINE16           87
int U_EMRPOLYLINE16_swap(char *record, int torev){
   return(core6_swap(record, torev));
}

// U_EMRPOLYBEZIERTO16       88
int U_EMRPOLYBEZIERTO16_swap(char *record, int torev){
   return(core6_swap(record, torev));
}

// U_EMRPOLYLINETO16         89
/**
    \brief Convert a pointer to a U_EMR_POLYLINETO16 record.
    \param record   pointer to a buffer holding the EMR record
*/
int U_EMRPOLYLINETO16_swap(char *record, int torev){
   return(core6_swap(record, torev));
}

// U_EMRPOLYPOLYLINE16       90
int U_EMRPOLYPOLYLINE16_swap(char *record, int torev){
   return(core10_swap(record, torev));
}

// U_EMRPOLYPOLYGON16        91
int U_EMRPOLYPOLYGON16_swap(char *record, int torev){
   return(core10_swap(record, torev));
}


// U_EMRPOLYDRAW16           92
int U_EMRPOLYDRAW16_swap(char *record, int torev){
   int count=0;
   const char *blimit = NULL;
   PU_EMRPOLYDRAW16 pEmr = (PU_EMRPOLYDRAW16)(record);
   if(torev){
      count = pEmr->cpts;
      blimit = record + pEmr->emr.nSize;
   }
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->cpts),1);                // cpts
   if(!torev){
      count = pEmr->cpts;
      blimit = record + pEmr->emr.nSize;
   }
   if(IS_MEM_UNSAFE((pEmr->apts), count*sizeof(U_POINT16), blimit))return(0);
   point16_swap(pEmr->apts,count);          // apts[]
   // single byte data                         abTypes
   return(1);
}

// U_EMRCREATEMONOBRUSH      93
int U_EMRCREATEMONOBRUSH_swap(char *record, int torev){
   return(core12_swap(record, torev));
}

// U_EMRCREATEDIBPATTERNBRUSHPT_swap   94
int U_EMRCREATEDIBPATTERNBRUSHPT_swap(char *record, int torev){
   return(core12_swap(record, torev));
}


// U_EMREXTCREATEPEN         95
int U_EMREXTCREATEPEN_swap(char *record, int torev){
   const char *blimit = NULL;
   U_OFFBMI  offBmi  = 0;
   U_CBBMI   cbBmi   = 0; 
   U_OFFBITS offBits = 0;
   U_CBBITS  cbBits  = 0;
   PU_EMREXTCREATEPEN pEmr = (PU_EMREXTCREATEPEN)(record);
   if(torev){
      offBmi  = pEmr->offBmi;
      cbBmi   = pEmr->cbBmi;
      offBits = pEmr->offBits;
      cbBits  = pEmr->cbBits;
      blimit  = record + pEmr->emr.nSize;
      if(!DIB_swap(record, U_DIB_RGB_COLORS, offBmi, cbBmi, offBits, cbBits, blimit, torev))return(0);
   }
   if(!core5_swap(record, torev))return(0);
   U_swap4(&(pEmr->ihPen),5);               // ihPen offBmi cbBmi offBits cbBits
   if(!torev){
      offBmi  = pEmr->offBmi;
      cbBmi   = pEmr->cbBmi;
      offBits = pEmr->offBits;
      cbBits  = pEmr->cbBits;
      blimit  = record + pEmr->emr.nSize;
      if(!DIB_swap(record, U_DIB_RGB_COLORS, offBmi, cbBmi, offBits, cbBits, blimit, torev))return(0);
   }
   return(extlogpen_swap((PU_EXTLOGPEN) &(pEmr->elp), blimit, torev)); 
}

// U_EMRPOLYTEXTOUTA         96 NOT IMPLEMENTED, denigrated after Windows NT
#define U_EMRPOLYTEXTOUTA_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.
// U_EMRPOLYTEXTOUTW         97 NOT IMPLEMENTED, denigrated after Windows NT
#define U_EMRPOLYTEXTOUTW_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.

// U_EMRSETICMMODE           98
int U_EMRSETICMMODE_swap(char *record, int torev){
   return(core3_swap(record, torev));
}

// U_EMRCREATECOLORSPACE     99
int U_EMRCREATECOLORSPACE_swap(char *record, int torev){
   if(!core5_swap(record, torev))return(0);
   PU_EMRCREATECOLORSPACE pEmr = (PU_EMRCREATECOLORSPACE)(record);
   U_swap4(&(pEmr->ihCS),1);                // ihCS
   logcolorspacea_swap(&(pEmr->lcs));       // lcs
   return(1);
}

// U_EMRSETCOLORSPACE       100
int U_EMRSETCOLORSPACE_swap(char *record, int torev){
   return(core3_swap(record, torev));
}

// U_EMRDELETECOLORSPACE    101
int U_EMRDELETECOLORSPACE_swap(char *record, int torev){
   return(core3_swap(record, torev));
}

// U_EMRGLSRECORD           102  Not implemented
#define U_EMRGLSRECORD_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.
// U_EMRGLSBOUNDEDRECORD    103  Not implemented
#define U_EMRGLSBOUNDEDRECORD_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.

// U_EMRPIXELFORMAT         104
int U_EMRPIXELFORMAT_swap(char *record, int torev){
   if(!core5_swap(record, torev))return(0);
   PU_EMRPIXELFORMAT pEmr = (PU_EMRPIXELFORMAT)(record);
   pixelformatdescriptor_swap(&(pEmr->pfd));     // pfd
   return(1);
}

// U_EMRDRAWESCAPE          105  Not implemented
#define U_EMRDRAWESCAPE_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.
// U_EMREXTESCAPE           106  Not implemented
#define U_EMREXTESCAPE_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.
// U_EMRUNDEF107            107  Not implemented
#define U_EMRUNDEF107_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.

// U_EMRSMALLTEXTOUT        108
int U_EMRSMALLTEXTOUT_swap(char *record, int torev){
   int roff=sizeof(U_EMRSMALLTEXTOUT);        // offset to the start of the variable fields
   int fuOptions = 0;
   int cChars = 0;
   const char *blimit = NULL;
   PU_EMRSMALLTEXTOUT pEmr = (PU_EMRSMALLTEXTOUT)(record);
   if(torev){
      fuOptions = pEmr->fuOptions;
      cChars    = pEmr->cChars;
      blimit = record + pEmr->emr.nSize;
   }
   if(!core5_swap(record, torev))return(0);
   pointl_swap(&(pEmr->Dest),1);            // Dest
   U_swap4(&(pEmr->cChars),5);              // cChars fuOptions iGraphicsMode exScale eyScale
   if(!torev){
      fuOptions = pEmr->fuOptions;
      cChars    = pEmr->cChars;
      blimit = record + pEmr->emr.nSize;
   }
   if(!(fuOptions & U_ETO_NO_RECT)){
      if(IS_MEM_UNSAFE(record, roff + sizeof(U_RECTL), blimit))return(0);
      rectl_swap( (PU_RECTL) (record + roff),1);  // rclBounds
   }
   if(IS_MEM_UNSAFE(record, roff + sizeof(U_RECTL) + cChars, blimit))return(0);
   // ordered bytes or UTF16-LE                TextString
   return(1);
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
int U_EMRALPHABLEND_swap(char *record, int torev){
   return(core13_swap(record, torev));
}

// U_EMRSETLAYOUT           115
int U_EMRSETLAYOUT_swap(char *record, int torev){
   return(core3_swap(record, torev));
}

// U_EMRTRANSPARENTBLT      116
int U_EMRTRANSPARENTBLT_swap(char *record, int torev){
   return(core13_swap(record, torev));
}


// U_EMRUNDEF117            117  Not implemented
#define U_EMRUNDEF117_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.
// U_EMRGRADIENTFILL        118
int U_EMRGRADIENTFILL_swap(char *record, int torev){
   int nTriVert=0;
   int nGradObj=0;
   int ulMode=0;
   const char *blimit = NULL;
   PU_EMRGRADIENTFILL pEmr = (PU_EMRGRADIENTFILL)(record);
   if(torev){
      nTriVert = pEmr->nTriVert;
      nGradObj = pEmr->nGradObj;
      ulMode   = pEmr->ulMode;
      blimit = record + pEmr->emr.nSize;
   }
   if(!core5_swap(record, torev))return(0);
   rectl_swap(&(pEmr->rclBounds),1);        // rclBounds
   U_swap4(&(pEmr->nTriVert),3);            // nTriVert nGradObj ulMode
   if(!torev){
      nTriVert = pEmr->nTriVert;
      nGradObj = pEmr->nGradObj;
      ulMode   = pEmr->ulMode;
      blimit = record + pEmr->emr.nSize;
   }
   record += sizeof(U_EMRGRADIENTFILL);
   if(IS_MEM_UNSAFE(record, nTriVert*sizeof(U_TRIVERTEX), blimit))return(0);
   if(nTriVert){
         trivertex_swap((PU_TRIVERTEX)(record),nTriVert);    // TriVert[]
   }
   record += nTriVert * sizeof(U_TRIVERTEX);
   if(nGradObj){
      if(     ulMode == U_GRADIENT_FILL_TRIANGLE){
         if(IS_MEM_UNSAFE(record, nGradObj*sizeof(U_GRADIENT3), blimit))return(0);
         gradient3_swap((PU_GRADIENT3)(record), nGradObj);   // GradObj[]
      }
      else if(ulMode == U_GRADIENT_FILL_RECT_H || ulMode == U_GRADIENT_FILL_RECT_V){
         if(IS_MEM_UNSAFE(record, nGradObj*sizeof(U_GRADIENT4), blimit))return(0);
         gradient4_swap((PU_GRADIENT4)(record), nGradObj);   // GradObj[]
      }
   }
   return(1);
}

// U_EMRSETLINKEDUFIS       119  Not implemented
#define U_EMRSETLINKEDUFIS_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.
// U_EMRSETTEXTJUSTIFICATION120  Not implemented (denigrated)
#define U_EMRSETTEXTJUSTIFICATION_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.
// U_EMRCOLORMATCHTOTARGETW 121  Not implemented  
#define U_EMRCOLORMATCHTOTARGETW_swap(A,B) U_EMRNOTIMPLEMENTED_swap(A,B) //!< Not implemented.

// U_EMRCREATECOLORSPACEW   122
int U_EMRCREATECOLORSPACEW_swap(char *record, int torev){
   PU_EMRCREATECOLORSPACEW pEmr = (PU_EMRCREATECOLORSPACEW)(record);
   if(!core5_swap(record, torev))return(0);
   U_swap4(&(pEmr->ihCS),1);                // ihCS
   logcolorspacew_swap(&(pEmr->lcs));       // lcs
   U_swap4(&(pEmr->dwFlags),2);             // dwFlags cbData
   // ordered bytes:                           Data
   return(1);
}

//! \endcond

/**
    \brief Checks the declared size of a record for consistency
    \return 0 on failure, 1 on success
    \param record   pointer to the start of the EMF record in memory
    \param blimit   pointer to one byte after the EMF record in memory
    \param nSize    number of bytes in the record, read from the record
    \param iType    type of the record, read from the record
    \param torev    1 for native to reversed, 0 for reversed to native
    
    Normally this would be called immediately after reading the data from a file.
    Verifies that the declared size is consistent with this type of record.
*/
int U_emf_record_sizeok(const char *record, const char *blimit, uint32_t  *nSize, uint32_t  *iType, int torev){
   uint32_t rsize=0;
   if(!nSize || !iType)return(0); // programming error

   /* Check that COMMON data in record can be touched without an access violation.  If it cannot be
       this is either a corrupt EMF or one engineered to cause a buffer overflow.  Pointer math
       could wrap so check both sides of the range, and fail any indications of such a wrap.
   */
   if(IS_MEM_UNSAFE(record, sizeof(U_EMR), blimit))return(0); 

   PU_ENHMETARECORD pEmr = (PU_ENHMETARECORD)(record);
   *iType = pEmr->iType;
   *nSize = pEmr->nSize;
   if(!torev){
      U_swap4(iType,1);   
      U_swap4(nSize,1);   
   }

   /* Check that the FULL record size is OK, abort if not.  */
   if(IS_MEM_UNSAFE(record, *nSize, blimit))return(0);
   
   switch (*iType)
   {
       // next line, ancient EMF files used a smaller header, to be safe, test for that
       case U_EMR_HEADER:                  rsize = U_SIZE_EMRHEADER_MIN;              break;
       case U_EMR_POLYBEZIER:              rsize = U_SIZE_EMRPOLYBEZIER;              break;
       case U_EMR_POLYGON:                 rsize = U_SIZE_EMRPOLYGON;                 break;
       case U_EMR_POLYLINE:                rsize = U_SIZE_EMRPOLYLINE;                break;
       case U_EMR_POLYBEZIERTO:            rsize = U_SIZE_EMRPOLYBEZIERTO;            break;
       case U_EMR_POLYLINETO:              rsize = U_SIZE_EMRPOLYLINETO;              break;
       case U_EMR_POLYPOLYLINE:            rsize = U_SIZE_EMRPOLYPOLYLINE;            break;
       case U_EMR_POLYPOLYGON:             rsize = U_SIZE_EMRPOLYPOLYGON;             break;
       case U_EMR_SETWINDOWEXTEX:          rsize = U_SIZE_EMRSETWINDOWEXTEX;          break;
       case U_EMR_SETWINDOWORGEX:          rsize = U_SIZE_EMRSETWINDOWORGEX;          break;
       case U_EMR_SETVIEWPORTEXTEX:        rsize = U_SIZE_EMRSETVIEWPORTEXTEX;        break;
       case U_EMR_SETVIEWPORTORGEX:        rsize = U_SIZE_EMRSETVIEWPORTORGEX;        break;
       case U_EMR_SETBRUSHORGEX:           rsize = U_SIZE_EMRSETBRUSHORGEX;           break;
       case U_EMR_EOF:                     rsize = U_SIZE_EMREOF;                     break;
       case U_EMR_SETPIXELV:               rsize = U_SIZE_EMRSETPIXELV;               break;
       case U_EMR_SETMAPPERFLAGS:          rsize = U_SIZE_EMRSETMAPPERFLAGS;          break;
       case U_EMR_SETMAPMODE:              rsize = U_SIZE_EMRSETMAPMODE;              break;
       case U_EMR_SETBKMODE:               rsize = U_SIZE_EMRSETBKMODE;               break;
       case U_EMR_SETPOLYFILLMODE:         rsize = U_SIZE_EMRSETPOLYFILLMODE;         break;
       case U_EMR_SETROP2:                 rsize = U_SIZE_EMRSETROP2;                 break;
       case U_EMR_SETSTRETCHBLTMODE:       rsize = U_SIZE_EMRSETSTRETCHBLTMODE;       break;
       case U_EMR_SETTEXTALIGN:            rsize = U_SIZE_EMRSETTEXTALIGN;            break;
       case U_EMR_SETCOLORADJUSTMENT:      rsize = U_SIZE_EMRSETCOLORADJUSTMENT;      break;
       case U_EMR_SETTEXTCOLOR:            rsize = U_SIZE_EMRSETTEXTCOLOR;            break;
       case U_EMR_SETBKCOLOR:              rsize = U_SIZE_EMRSETBKCOLOR;              break;
       case U_EMR_OFFSETCLIPRGN:           rsize = U_SIZE_EMROFFSETCLIPRGN;           break;
       case U_EMR_MOVETOEX:                rsize = U_SIZE_EMRMOVETOEX;                break;
       case U_EMR_SETMETARGN:              rsize = U_SIZE_EMRSETMETARGN;              break;
       case U_EMR_EXCLUDECLIPRECT:         rsize = U_SIZE_EMREXCLUDECLIPRECT;         break;
       case U_EMR_INTERSECTCLIPRECT:       rsize = U_SIZE_EMRINTERSECTCLIPRECT;       break;
       case U_EMR_SCALEVIEWPORTEXTEX:      rsize = U_SIZE_EMRSCALEVIEWPORTEXTEX;      break;
       case U_EMR_SCALEWINDOWEXTEX:        rsize = U_SIZE_EMRSCALEWINDOWEXTEX;        break;
       case U_EMR_SAVEDC:                  rsize = U_SIZE_EMRSAVEDC;                  break;
       case U_EMR_RESTOREDC:               rsize = U_SIZE_EMRRESTOREDC;               break;
       case U_EMR_SETWORLDTRANSFORM:       rsize = U_SIZE_EMRSETWORLDTRANSFORM;       break;
       case U_EMR_MODIFYWORLDTRANSFORM:    rsize = U_SIZE_EMRMODIFYWORLDTRANSFORM;    break;
       case U_EMR_SELECTOBJECT:            rsize = U_SIZE_EMRSELECTOBJECT;            break;
       case U_EMR_CREATEPEN:               rsize = U_SIZE_EMRCREATEPEN;               break;
       case U_EMR_CREATEBRUSHINDIRECT:     rsize = U_SIZE_EMRCREATEBRUSHINDIRECT;     break;
       case U_EMR_DELETEOBJECT:            rsize = U_SIZE_EMRDELETEOBJECT;            break;
       case U_EMR_ANGLEARC:                rsize = U_SIZE_EMRANGLEARC;                break;
       case U_EMR_ELLIPSE:                 rsize = U_SIZE_EMRELLIPSE;                 break;
       case U_EMR_RECTANGLE:               rsize = U_SIZE_EMRRECTANGLE;               break;
       case U_EMR_ROUNDRECT:               rsize = U_SIZE_EMRROUNDRECT;               break;
       case U_EMR_ARC:                     rsize = U_SIZE_EMRARC;                     break;
       case U_EMR_CHORD:                   rsize = U_SIZE_EMRCHORD;                   break;
       case U_EMR_PIE:                     rsize = U_SIZE_EMRPIE;                     break;
       case U_EMR_SELECTPALETTE:           rsize = U_SIZE_EMRSELECTPALETTE;           break;
       case U_EMR_CREATEPALETTE:           rsize = U_SIZE_EMRCREATEPALETTE;           break;
       case U_EMR_SETPALETTEENTRIES:       rsize = U_SIZE_EMRSETPALETTEENTRIES;       break;
       case U_EMR_RESIZEPALETTE:           rsize = U_SIZE_EMRRESIZEPALETTE;           break;
       case U_EMR_REALIZEPALETTE:          rsize = U_SIZE_EMRREALIZEPALETTE;          break;
       case U_EMR_EXTFLOODFILL:            rsize = U_SIZE_EMREXTFLOODFILL;            break;
       case U_EMR_LINETO:                  rsize = U_SIZE_EMRLINETO;                  break;
       case U_EMR_ARCTO:                   rsize = U_SIZE_EMRARCTO;                   break;
       case U_EMR_POLYDRAW:                rsize = U_SIZE_EMRPOLYDRAW;                break;
       case U_EMR_SETARCDIRECTION:         rsize = U_SIZE_EMRSETARCDIRECTION;         break;
       case U_EMR_SETMITERLIMIT:           rsize = U_SIZE_EMRSETMITERLIMIT;           break;
       case U_EMR_BEGINPATH:               rsize = U_SIZE_EMRBEGINPATH;               break;
       case U_EMR_ENDPATH:                 rsize = U_SIZE_EMRENDPATH;                 break;
       case U_EMR_CLOSEFIGURE:             rsize = U_SIZE_EMRCLOSEFIGURE;             break;
       case U_EMR_FILLPATH:                rsize = U_SIZE_EMRFILLPATH;                break;
       case U_EMR_STROKEANDFILLPATH:       rsize = U_SIZE_EMRSTROKEANDFILLPATH;       break;
       case U_EMR_STROKEPATH:              rsize = U_SIZE_EMRSTROKEPATH;              break;
       case U_EMR_FLATTENPATH:             rsize = U_SIZE_EMRFLATTENPATH;             break;
       case U_EMR_WIDENPATH:               rsize = U_SIZE_EMRWIDENPATH;               break;
       case U_EMR_SELECTCLIPPATH:          rsize = U_SIZE_EMRSELECTCLIPPATH;          break;
       case U_EMR_ABORTPATH:               rsize = U_SIZE_EMRABORTPATH;               break;
       case U_EMR_UNDEF69:                 rsize = U_SIZE_EMRUNDEFINED;               break;
       case U_EMR_COMMENT:                 rsize = U_SIZE_EMRCOMMENT;                 break;
       case U_EMR_FILLRGN:                 rsize = U_SIZE_EMRFILLRGN;                 break;
       case U_EMR_FRAMERGN:                rsize = U_SIZE_EMRFRAMERGN;                break;
       case U_EMR_INVERTRGN:               rsize = U_SIZE_EMRINVERTRGN;               break;
       case U_EMR_PAINTRGN:                rsize = U_SIZE_EMRPAINTRGN;                break;
       case U_EMR_EXTSELECTCLIPRGN:        rsize = U_SIZE_EMREXTSELECTCLIPRGN;        break;
       case U_EMR_BITBLT:                  rsize = U_SIZE_EMRBITBLT;                  break;
       case U_EMR_STRETCHBLT:              rsize = U_SIZE_EMRSTRETCHBLT;              break;
       case U_EMR_MASKBLT:                 rsize = U_SIZE_EMRMASKBLT;                 break;
       case U_EMR_PLGBLT:                  rsize = U_SIZE_EMRPLGBLT;                  break;
       case U_EMR_SETDIBITSTODEVICE:       rsize = U_SIZE_EMRSETDIBITSTODEVICE;       break;
       case U_EMR_STRETCHDIBITS:           rsize = U_SIZE_EMRSTRETCHDIBITS;           break;
       case U_EMR_EXTCREATEFONTINDIRECTW:  rsize = U_SIZE_EMREXTCREATEFONTINDIRECTW_LOGFONT;  break;
       case U_EMR_EXTTEXTOUTA:             rsize = U_SIZE_EMREXTTEXTOUTA;             break;
       case U_EMR_EXTTEXTOUTW:             rsize = U_SIZE_EMREXTTEXTOUTW;             break;
       case U_EMR_POLYBEZIER16:            rsize = U_SIZE_EMRPOLYBEZIER16;            break;
       case U_EMR_POLYGON16:               rsize = U_SIZE_EMRPOLYGON16;               break;
       case U_EMR_POLYLINE16:              rsize = U_SIZE_EMRPOLYLINE16;              break;
       case U_EMR_POLYBEZIERTO16:          rsize = U_SIZE_EMRPOLYBEZIERTO16;          break;
       case U_EMR_POLYLINETO16:            rsize = U_SIZE_EMRPOLYLINETO16;            break;
       case U_EMR_POLYPOLYLINE16:          rsize = U_SIZE_EMRPOLYPOLYLINE16;          break;
       case U_EMR_POLYPOLYGON16:           rsize = U_SIZE_EMRPOLYPOLYGON16;           break;
       case U_EMR_POLYDRAW16:              rsize = U_SIZE_EMRPOLYDRAW16;              break;
       case U_EMR_CREATEMONOBRUSH:         rsize = U_SIZE_EMRCREATEMONOBRUSH;         break;
       case U_EMR_CREATEDIBPATTERNBRUSHPT: rsize = U_SIZE_EMRCREATEDIBPATTERNBRUSHPT; break;
       case U_EMR_EXTCREATEPEN:            rsize = U_SIZE_EMREXTCREATEPEN;            break;
       case U_EMR_POLYTEXTOUTA:            rsize = U_SIZE_EMRPOLYTEXTOUTA;            break;
       case U_EMR_POLYTEXTOUTW:            rsize = U_SIZE_EMRPOLYTEXTOUTW;            break;
       case U_EMR_SETICMMODE:              rsize = U_SIZE_EMRSETICMMODE;              break;
       case U_EMR_CREATECOLORSPACE:        rsize = U_SIZE_EMRCREATECOLORSPACE;        break;
       case U_EMR_SETCOLORSPACE:           rsize = U_SIZE_EMRSETCOLORSPACE;           break;
       case U_EMR_DELETECOLORSPACE:        rsize = U_SIZE_EMRDELETECOLORSPACE;        break;
       case U_EMR_GLSRECORD:               rsize = U_SIZE_EMRGLSRECORD;               break;
       case U_EMR_GLSBOUNDEDRECORD:        rsize = U_SIZE_EMRGLSBOUNDEDRECORD;        break;
       case U_EMR_PIXELFORMAT:             rsize = U_SIZE_EMRPIXELFORMAT;             break;
       case U_EMR_DRAWESCAPE:              rsize = U_SIZE_EMRDRAWESCAPE;              break;
       case U_EMR_EXTESCAPE:               rsize = U_SIZE_EMREXTESCAPE;               break;
       case U_EMR_UNDEF107:                rsize = U_SIZE_EMRUNDEFINED;               break;
       case U_EMR_SMALLTEXTOUT:            rsize = U_SIZE_EMRSMALLTEXTOUT;            break;
//       case U_EMR_FORCEUFIMAPPING:         rsize = U_SIZE_EMRFORCEUFIMAPPING;         break;
       case U_EMR_NAMEDESCAPE:             rsize = U_SIZE_EMRNAMEDESCAPE;             break;
//       case U_EMR_COLORCORRECTPALETTE:     rsize = U_SIZE_EMRCOLORCORRECTPALETTE;     break;
//       case U_EMR_SETICMPROFILEA:          rsize = U_SIZE_EMRSETICMPROFILEA;          break;
//       case U_EMR_SETICMPROFILEW:          rsize = U_SIZE_EMRSETICMPROFILEW;          break;
       case U_EMR_ALPHABLEND:              rsize = U_SIZE_EMRALPHABLEND;              break;
       case U_EMR_SETLAYOUT:               rsize = U_SIZE_EMRSETLAYOUT;               break;
       case U_EMR_TRANSPARENTBLT:          rsize = U_SIZE_EMRTRANSPARENTBLT;          break;
       case U_EMR_UNDEF117:                rsize = U_SIZE_EMRUNDEFINED;               break;
       case U_EMR_GRADIENTFILL:            rsize = U_SIZE_EMRGRADIENTFILL;            break;
//       case U_EMR_SETLINKEDUFIS:           rsize = U_SIZE_EMRSETLINKEDUFIS;           break;
//       case U_EMR_SETTEXTJUSTIFICATION:    rsize = U_SIZE_EMRSETTEXTJUSTIFICATION;    break;
       case U_EMR_COLORMATCHTOTARGETW:     rsize = U_SIZE_EMRCOLORMATCHTOTARGETW;     break;
       case U_EMR_CREATECOLORSPACEW:       rsize = U_SIZE_EMRCREATECOLORSPACEW;       break;
       default:                            rsize = U_SIZE_EMRNOTIMPLEMENTED;          break;
   }  //end of switch
   //  record's declared size must be more than the minimum size for this type of record
   if(*nSize < rsize){
      return(0);
   }
   return(1);
}


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
    const char *blimit = contents + length;  /* should never wrap since it describes a structure in memory */
    int rstatus=1;

    record = contents;
    OK     = 1;
    off    = 0;
    recnum = 0;
    while(OK){

       if(!U_emf_record_sizeok(record, blimit, &off, &iType, torev)){
          return(0);
       }

       switch (iType)
       {
           case U_EMR_HEADER:                  rstatus=U_EMRHEADER_swap(record, torev);                  break;
           case U_EMR_POLYBEZIER:              rstatus=U_EMRPOLYBEZIER_swap(record, torev);              break;
           case U_EMR_POLYGON:                 rstatus=U_EMRPOLYGON_swap(record, torev);                 break;
           case U_EMR_POLYLINE:                rstatus=U_EMRPOLYLINE_swap(record, torev);                break;
           case U_EMR_POLYBEZIERTO:            rstatus=U_EMRPOLYBEZIERTO_swap(record, torev);            break;
           case U_EMR_POLYLINETO:              rstatus=U_EMRPOLYLINETO_swap(record, torev);              break;
           case U_EMR_POLYPOLYLINE:            rstatus=U_EMRPOLYPOLYLINE_swap(record, torev);            break;
           case U_EMR_POLYPOLYGON:             rstatus=U_EMRPOLYPOLYGON_swap(record, torev);             break;
           case U_EMR_SETWINDOWEXTEX:          rstatus=U_EMRSETWINDOWEXTEX_swap(record, torev);          break;
           case U_EMR_SETWINDOWORGEX:          rstatus=U_EMRSETWINDOWORGEX_swap(record, torev);          break;
           case U_EMR_SETVIEWPORTEXTEX:        rstatus=U_EMRSETVIEWPORTEXTEX_swap(record, torev);        break;
           case U_EMR_SETVIEWPORTORGEX:        rstatus=U_EMRSETVIEWPORTORGEX_swap(record, torev);        break;
           case U_EMR_SETBRUSHORGEX:           rstatus=U_EMRSETBRUSHORGEX_swap(record, torev);           break;
           case U_EMR_EOF:
                                               rstatus=U_EMREOF_swap(record, torev);
                                               OK = 0;    /* Exit triggered here */
                                                                                                 break;
           case U_EMR_SETPIXELV:               rstatus=U_EMRSETPIXELV_swap(record, torev);               break;
           case U_EMR_SETMAPPERFLAGS:          rstatus=U_EMRSETMAPPERFLAGS_swap(record, torev);          break;
           case U_EMR_SETMAPMODE:              rstatus=U_EMRSETMAPMODE_swap(record, torev);              break;
           case U_EMR_SETBKMODE:               rstatus=U_EMRSETBKMODE_swap(record, torev);               break;
           case U_EMR_SETPOLYFILLMODE:         rstatus=U_EMRSETPOLYFILLMODE_swap(record, torev);         break;
           case U_EMR_SETROP2:                 rstatus=U_EMRSETROP2_swap(record, torev);                 break;
           case U_EMR_SETSTRETCHBLTMODE:       rstatus=U_EMRSETSTRETCHBLTMODE_swap(record, torev);       break;
           case U_EMR_SETTEXTALIGN:            rstatus=U_EMRSETTEXTALIGN_swap(record, torev);            break;
           case U_EMR_SETCOLORADJUSTMENT:      rstatus=U_EMRSETCOLORADJUSTMENT_swap(record, torev);      break;
           case U_EMR_SETTEXTCOLOR:            rstatus=U_EMRSETTEXTCOLOR_swap(record, torev);            break;
           case U_EMR_SETBKCOLOR:              rstatus=U_EMRSETBKCOLOR_swap(record, torev);              break;
           case U_EMR_OFFSETCLIPRGN:           rstatus=U_EMROFFSETCLIPRGN_swap(record, torev);           break;
           case U_EMR_MOVETOEX:                rstatus=U_EMRMOVETOEX_swap(record, torev);                break;
           case U_EMR_SETMETARGN:              rstatus=U_EMRSETMETARGN_swap(record, torev);              break;
           case U_EMR_EXCLUDECLIPRECT:         rstatus=U_EMREXCLUDECLIPRECT_swap(record, torev);         break;
           case U_EMR_INTERSECTCLIPRECT:       rstatus=U_EMRINTERSECTCLIPRECT_swap(record, torev);       break;
           case U_EMR_SCALEVIEWPORTEXTEX:      rstatus=U_EMRSCALEVIEWPORTEXTEX_swap(record, torev);      break;
           case U_EMR_SCALEWINDOWEXTEX:        rstatus=U_EMRSCALEWINDOWEXTEX_swap(record, torev);        break;
           case U_EMR_SAVEDC:                  rstatus=U_EMRSAVEDC_swap(record, torev);                  break;
           case U_EMR_RESTOREDC:               rstatus=U_EMRRESTOREDC_swap(record, torev);               break;
           case U_EMR_SETWORLDTRANSFORM:       rstatus=U_EMRSETWORLDTRANSFORM_swap(record, torev);       break;
           case U_EMR_MODIFYWORLDTRANSFORM:    rstatus=U_EMRMODIFYWORLDTRANSFORM_swap(record, torev);    break;
           case U_EMR_SELECTOBJECT:            rstatus=U_EMRSELECTOBJECT_swap(record, torev);            break;
           case U_EMR_CREATEPEN:               rstatus=U_EMRCREATEPEN_swap(record, torev);               break;
           case U_EMR_CREATEBRUSHINDIRECT:     rstatus=U_EMRCREATEBRUSHINDIRECT_swap(record, torev);     break;
           case U_EMR_DELETEOBJECT:            rstatus=U_EMRDELETEOBJECT_swap(record, torev);            break;
           case U_EMR_ANGLEARC:                rstatus=U_EMRANGLEARC_swap(record, torev);                break;
           case U_EMR_ELLIPSE:                 rstatus=U_EMRELLIPSE_swap(record, torev);                 break;
           case U_EMR_RECTANGLE:               rstatus=U_EMRRECTANGLE_swap(record, torev);               break;
           case U_EMR_ROUNDRECT:               rstatus=U_EMRROUNDRECT_swap(record, torev);               break;
           case U_EMR_ARC:                     rstatus=U_EMRARC_swap(record, torev);                     break;
           case U_EMR_CHORD:                   rstatus=U_EMRCHORD_swap(record, torev);                   break;
           case U_EMR_PIE:                     rstatus=U_EMRPIE_swap(record, torev);                     break;
           case U_EMR_SELECTPALETTE:           rstatus=U_EMRSELECTPALETTE_swap(record, torev);           break;
           case U_EMR_CREATEPALETTE:           rstatus=U_EMRCREATEPALETTE_swap(record, torev);           break;
           case U_EMR_SETPALETTEENTRIES:       rstatus=U_EMRSETPALETTEENTRIES_swap(record, torev);       break;
           case U_EMR_RESIZEPALETTE:           rstatus=U_EMRRESIZEPALETTE_swap(record, torev);           break;
           case U_EMR_REALIZEPALETTE:          rstatus=U_EMRREALIZEPALETTE_swap(record, torev);          break;
           case U_EMR_EXTFLOODFILL:            rstatus=U_EMREXTFLOODFILL_swap(record, torev);            break;
           case U_EMR_LINETO:                  rstatus=U_EMRLINETO_swap(record, torev);                  break;
           case U_EMR_ARCTO:                   rstatus=U_EMRARCTO_swap(record, torev);                   break;
           case U_EMR_POLYDRAW:                rstatus=U_EMRPOLYDRAW_swap(record, torev);                break;
           case U_EMR_SETARCDIRECTION:         rstatus=U_EMRSETARCDIRECTION_swap(record, torev);         break;
           case U_EMR_SETMITERLIMIT:           rstatus=U_EMRSETMITERLIMIT_swap(record, torev);           break;
           case U_EMR_BEGINPATH:               rstatus=U_EMRBEGINPATH_swap(record, torev);               break;
           case U_EMR_ENDPATH:                 rstatus=U_EMRENDPATH_swap(record, torev);                 break;
           case U_EMR_CLOSEFIGURE:             rstatus=U_EMRCLOSEFIGURE_swap(record, torev);             break;
           case U_EMR_FILLPATH:                rstatus=U_EMRFILLPATH_swap(record, torev);                break;
           case U_EMR_STROKEANDFILLPATH:       rstatus=U_EMRSTROKEANDFILLPATH_swap(record, torev);       break;
           case U_EMR_STROKEPATH:              rstatus=U_EMRSTROKEPATH_swap(record, torev);              break;
           case U_EMR_FLATTENPATH:             rstatus=U_EMRFLATTENPATH_swap(record, torev);             break;
           case U_EMR_WIDENPATH:               rstatus=U_EMRWIDENPATH_swap(record, torev);               break;
           case U_EMR_SELECTCLIPPATH:          rstatus=U_EMRSELECTCLIPPATH_swap(record, torev);          break;
           case U_EMR_ABORTPATH:               rstatus=U_EMRABORTPATH_swap(record, torev);               break;
           case U_EMR_UNDEF69:                 rstatus=U_EMRUNDEF69_swap(record, torev);                 break;
           case U_EMR_COMMENT:                 rstatus=U_EMRCOMMENT_swap(record, torev);                 break;
           case U_EMR_FILLRGN:                 rstatus=U_EMRFILLRGN_swap(record, torev);         break;
           case U_EMR_FRAMERGN:                rstatus=U_EMRFRAMERGN_swap(record, torev);        break;
           case U_EMR_INVERTRGN:               rstatus=U_EMRINVERTRGN_swap(record, torev);       break;
           case U_EMR_PAINTRGN:                rstatus=U_EMRPAINTRGN_swap(record, torev);        break;
           case U_EMR_EXTSELECTCLIPRGN:        rstatus=U_EMREXTSELECTCLIPRGN_swap(record, torev);break;
           case U_EMR_BITBLT:                  rstatus=U_EMRBITBLT_swap(record, torev);                  break;
           case U_EMR_STRETCHBLT:              rstatus=U_EMRSTRETCHBLT_swap(record, torev);              break;
           case U_EMR_MASKBLT:                 rstatus=U_EMRMASKBLT_swap(record, torev);                 break;
           case U_EMR_PLGBLT:                  rstatus=U_EMRPLGBLT_swap(record, torev);                  break;
           case U_EMR_SETDIBITSTODEVICE:       rstatus=U_EMRSETDIBITSTODEVICE_swap(record, torev);       break;
           case U_EMR_STRETCHDIBITS:           rstatus=U_EMRSTRETCHDIBITS_swap(record, torev);           break;
           case U_EMR_EXTCREATEFONTINDIRECTW:  rstatus=U_EMREXTCREATEFONTINDIRECTW_swap(record, torev);  break;
           case U_EMR_EXTTEXTOUTA:             rstatus=U_EMREXTTEXTOUTA_swap(record, torev);             break;
           case U_EMR_EXTTEXTOUTW:             rstatus=U_EMREXTTEXTOUTW_swap(record, torev);             break;
           case U_EMR_POLYBEZIER16:            rstatus=U_EMRPOLYBEZIER16_swap(record, torev);            break;
           case U_EMR_POLYGON16:               rstatus=U_EMRPOLYGON16_swap(record, torev);               break;
           case U_EMR_POLYLINE16:              rstatus=U_EMRPOLYLINE16_swap(record, torev);              break;
           case U_EMR_POLYBEZIERTO16:          rstatus=U_EMRPOLYBEZIERTO16_swap(record, torev);          break;
           case U_EMR_POLYLINETO16:            rstatus=U_EMRPOLYLINETO16_swap(record, torev);            break;
           case U_EMR_POLYPOLYLINE16:          rstatus=U_EMRPOLYPOLYLINE16_swap(record, torev);          break;
           case U_EMR_POLYPOLYGON16:           rstatus=U_EMRPOLYPOLYGON16_swap(record, torev);           break;
           case U_EMR_POLYDRAW16:              rstatus=U_EMRPOLYDRAW16_swap(record, torev);              break;
           case U_EMR_CREATEMONOBRUSH:         rstatus=U_EMRCREATEMONOBRUSH_swap(record, torev);         break;
           case U_EMR_CREATEDIBPATTERNBRUSHPT: rstatus=U_EMRCREATEDIBPATTERNBRUSHPT_swap(record, torev); break;
           case U_EMR_EXTCREATEPEN:            rstatus=U_EMREXTCREATEPEN_swap(record, torev);            break;
           case U_EMR_POLYTEXTOUTA:            rstatus=U_EMRPOLYTEXTOUTA_swap(record, torev);            break;
           case U_EMR_POLYTEXTOUTW:            rstatus=U_EMRPOLYTEXTOUTW_swap(record, torev);            break;
           case U_EMR_SETICMMODE:              rstatus=U_EMRSETICMMODE_swap(record, torev);              break;
           case U_EMR_CREATECOLORSPACE:        rstatus=U_EMRCREATECOLORSPACE_swap(record, torev);        break;
           case U_EMR_SETCOLORSPACE:           rstatus=U_EMRSETCOLORSPACE_swap(record, torev);           break;
           case U_EMR_DELETECOLORSPACE:        rstatus=U_EMRDELETECOLORSPACE_swap(record, torev);        break;
           case U_EMR_GLSRECORD:               rstatus=U_EMRGLSRECORD_swap(record, torev);               break;
           case U_EMR_GLSBOUNDEDRECORD:        rstatus=U_EMRGLSBOUNDEDRECORD_swap(record, torev);        break;
           case U_EMR_PIXELFORMAT:             rstatus=U_EMRPIXELFORMAT_swap(record, torev);             break;
           case U_EMR_DRAWESCAPE:              rstatus=U_EMRDRAWESCAPE_swap(record, torev);              break;
           case U_EMR_EXTESCAPE:               rstatus=U_EMREXTESCAPE_swap(record, torev);               break;
           case U_EMR_UNDEF107:                rstatus=U_EMRUNDEF107_swap(record, torev);                break;
           case U_EMR_SMALLTEXTOUT:            rstatus=U_EMRSMALLTEXTOUT_swap(record, torev);            break;
           case U_EMR_FORCEUFIMAPPING:         rstatus=U_EMRFORCEUFIMAPPING_swap(record, torev);         break;
           case U_EMR_NAMEDESCAPE:             rstatus=U_EMRNAMEDESCAPE_swap(record, torev);             break;
           case U_EMR_COLORCORRECTPALETTE:     rstatus=U_EMRCOLORCORRECTPALETTE_swap(record, torev);     break;
           case U_EMR_SETICMPROFILEA:          rstatus=U_EMRSETICMPROFILEA_swap(record, torev);          break;
           case U_EMR_SETICMPROFILEW:          rstatus=U_EMRSETICMPROFILEW_swap(record, torev);          break;
           case U_EMR_ALPHABLEND:              rstatus=U_EMRALPHABLEND_swap(record, torev);              break;
           case U_EMR_SETLAYOUT:               rstatus=U_EMRSETLAYOUT_swap(record, torev);               break;
           case U_EMR_TRANSPARENTBLT:          rstatus=U_EMRTRANSPARENTBLT_swap(record, torev);          break;
           case U_EMR_UNDEF117:                rstatus=U_EMRUNDEF117_swap(record, torev);                break;
           case U_EMR_GRADIENTFILL:            rstatus=U_EMRGRADIENTFILL_swap(record, torev);            break;
           case U_EMR_SETLINKEDUFIS:           rstatus=U_EMRSETLINKEDUFIS_swap(record, torev);           break;
           case U_EMR_SETTEXTJUSTIFICATION:    rstatus=U_EMRSETTEXTJUSTIFICATION_swap(record, torev);    break;
           case U_EMR_COLORMATCHTOTARGETW:     rstatus=U_EMRCOLORMATCHTOTARGETW_swap(record, torev);     break;
           case U_EMR_CREATECOLORSPACEW:       rstatus=U_EMRCREATECOLORSPACEW_swap(record, torev);       break;
           default:                            rstatus=U_EMRNOTIMPLEMENTED_swap(record, torev);          break;
       }  //end of switch
       if(!rstatus){
          return(rstatus);
       }
       record += off;
       recnum++;
    }  //end of while

    return(1);
}

#ifdef __cplusplus
}
#endif
