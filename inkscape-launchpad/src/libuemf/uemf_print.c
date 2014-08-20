/**
  @file uemf_print.c
  
  @brief Functions for printing EMF records
*/

/*
File:      uemf_print.c
Version:   0.0.15
Date:      17-OCT-2013
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2013 David Mathog and California Institute of Technology (Caltech)
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h> /* for offsetof() macro */
#include <string.h>
#include "uemf.h"
#include "upmf_print.h"

//! \cond
#define UNUSED(x) (void)(x)

/* one needed prototype */
void U_swap4(void *ul, unsigned int count);
//! \endcond

/** 
    \brief Print some number of hex bytes
    \param buf pointer to the first byte
    \param num number of bytes
*/
void hexbytes_print(uint8_t *buf,unsigned int num){
   for(; num; num--,buf++){
      printf("%2.2X",*buf);
   }
}

/* **********************************************************************************************
   These functions print standard objects used in the EMR records.
   The low level ones do not append EOL.
*********************************************************************************************** */



/** 
    \brief Print a U_COLORREF object.
    \param color  U_COLORREF object    
*/
void colorref_print(
      U_COLORREF color
   ){
   printf("{%u,%u,%u} ",color.Red,color.Green,color.Blue);
}


/**
    \brief Print a U_RGBQUAD object.
    \param color  U_RGBQUAD object    
*/
void rgbquad_print(
      U_RGBQUAD color
   ){
   printf("{%u,%u,%u,%u} ",color.Blue,color.Green,color.Red,color.Reserved);
}

/**
    \brief Print rect and rectl objects from Upper Left and Lower Right corner points.
    \param rect U_RECTL object
*/
void rectl_print(
      U_RECTL rect
    ){
    printf("{%d,%d,%d,%d} ",rect.left,rect.top,rect.right,rect.bottom);
}

/**
    \brief Print a U_SIZEL object.
    \param sz U_SizeL object
*/
void sizel_print(
       U_SIZEL sz
    ){
    printf("{%d,%d} ",sz.cx ,sz.cy);
} 

/**
    \brief Print a U_POINTL object
    \param pt U_POINTL object
*/
void pointl_print(
       U_POINTL pt
    ){
    printf("{%d,%d} ",pt.x ,pt.y);
} 

/**
    \brief Print a pointer to a U_POINT16 object
    \param pt pointer to a U_POINT16 object
    Warning - WMF data may contain unaligned U_POINT16, do not call
    this routine with a pointer to such data!
*/
void point16_print(
       U_POINT16 pt
    ){
    printf("{%d,%d} ",pt.x ,pt.y);
} 

/**
    \brief Print a U_LCS_GAMMA object
    \param lg U_LCS_GAMMA object
*/
void lcs_gamma_print(
      U_LCS_GAMMA lg
   ){
   uint8_t tmp;
   tmp = lg.ignoreHi; printf("ignoreHi:%u ",tmp);
   tmp = lg.intPart ; printf("intPart :%u ",tmp);
   tmp = lg.fracPart; printf("fracPart:%u ",tmp);
   tmp = lg.ignoreLo; printf("ignoreLo:%u ",tmp);
}

/**
    \brief Print a U_LCS_GAMMARGB object
    \param lgr U_LCS_GAMMARGB object
*/
void lcs_gammargb_print(
      U_LCS_GAMMARGB lgr
   ){
   printf("lcsGammaRed:");   lcs_gamma_print(lgr.lcsGammaRed  );
   printf("lcsGammaGreen:"); lcs_gamma_print(lgr.lcsGammaGreen);
   printf("lcsGammaBlue:");  lcs_gamma_print(lgr.lcsGammaBlue );
}

/**
    \brief Print a U_TRIVERTEX object.
    \param tv U_TRIVERTEX object.
*/
void trivertex_print(
      U_TRIVERTEX tv
   ){
   printf("{{%d,%d},{%u,%u,%u,%u}} ",tv.x,tv.y,tv.Red,tv.Green,tv.Blue,tv.Alpha);
}

/**
    \brief Print a U_GRADIENT3 object.
    \param g3 U_GRADIENT3 object.
*/
void gradient3_print(
      U_GRADIENT3 g3
   ){
   printf("{%u,%u,%u} ",g3.Vertex1,g3.Vertex2,g3.Vertex3);
}

/**
    \brief Print a U_GRADIENT4 object.
    \param g4 U_GRADIENT4 object.
*/
void gradient4_print(
      U_GRADIENT4 g4
   ){
   printf("{%u,%u} ",g4.UpperLeft,g4.LowerRight);
}

/**
    \brief Print a U_LOGBRUSH object.
    \param lb U_LOGBRUSH object.
*/
void logbrush_print(
      U_LOGBRUSH lb  
   ){
    printf("lbStyle:0x%8.8X ",  lb.lbStyle);
    printf("lbColor:");         colorref_print(lb.lbColor);
    printf("lbHatch:0x%8.8X ",  lb.lbHatch);
}

/**
    \brief Print a U_XFORM object.
    \param xform U_XFORM object
*/
void xform_print(
      U_XFORM xform
   ){
   printf("{%f,%f.%f,%f,%f,%f} ",xform.eM11,xform.eM12,xform.eM21,xform.eM22,xform.eDx,xform.eDy);
}

/**
  \brief Print a U_CIEXYZ object
  \param ciexyz U_CIEXYZ object
*/
void ciexyz_print(
      U_CIEXYZ ciexyz
   ){
   printf("{%d,%d.%d} ",ciexyz.ciexyzX,ciexyz.ciexyzY,ciexyz.ciexyzZ);
    
}

/**
  \brief Print a U_CIEXYZTRIPLE object
  \param cie3 U_CIEXYZTRIPLE object
*/
void ciexyztriple_print(
      U_CIEXYZTRIPLE cie3
   ){
   printf("{Red:");     ciexyz_print(cie3.ciexyzRed  );
   printf(", Green:");  ciexyz_print(cie3.ciexyzGreen);
   printf(", Blue:");   ciexyz_print(cie3.ciexyzBlue );
   printf("} ");
}
/**
    \brief Print a U_LOGCOLORSPACEA object.
    \param lcsa     U_LOGCOLORSPACEA object
*/
void logcolorspacea_print(
      U_LOGCOLORSPACEA lcsa
   ){
   printf("lcsSignature:%u ",lcsa.lcsSignature);
   printf("lcsVersion:%u ",  lcsa.lcsVersion  );
   printf("lcsSize:%u ",     lcsa.lcsSize     );
   printf("lcsCSType:%d ",    lcsa.lcsCSType   );
   printf("lcsIntent:%d ",    lcsa.lcsIntent   );
   printf("lcsEndpoints:");   ciexyztriple_print(lcsa.lcsEndpoints);
   printf("lcsGammaRGB: ");   lcs_gammargb_print(lcsa.lcsGammaRGB );
   printf("filename:%s ",     lcsa.lcsFilename );
}

/**

    \brief Print a U_LOGCOLORSPACEW object.
    \param lcsa U_LOGCOLORSPACEW object                                               
*/
void logcolorspacew_print(
      U_LOGCOLORSPACEW lcsa
   ){
   char *string;
   printf("lcsSignature:%d ",lcsa.lcsSignature);
   printf("lcsVersion:%d ",  lcsa.lcsVersion  );
   printf("lcsSize:%d ",     lcsa.lcsSize     );
   printf("lcsCSType:%d ",   lcsa.lcsCSType   );
   printf("lcsIntent:%d ",   lcsa.lcsIntent   );
   printf("lcsEndpoints:");   ciexyztriple_print(lcsa.lcsEndpoints);
   printf("lcsGammaRGB: ");   lcs_gammargb_print(lcsa.lcsGammaRGB );
   string = U_Utf16leToUtf8(lcsa.lcsFilename, U_MAX_PATH, NULL);
   printf("filename:%s ",   string );
   free(string);
}

/**
    \brief Print a U_PANOSE object.
    \param panose U_PANOSE object
*/
void panose_print(
      U_PANOSE panose
    ){
    printf("bFamilyType:%u ",     panose.bFamilyType     );
    printf("bSerifStyle:%u ",     panose.bSerifStyle     );
    printf("bWeight:%u ",         panose.bWeight         );
    printf("bProportion:%u ",     panose.bProportion     );
    printf("bContrast:%u ",       panose.bContrast       );
    printf("bStrokeVariation:%u ",panose.bStrokeVariation);
    printf("bArmStyle:%u ",       panose.bArmStyle       );
    printf("bLetterform:%u ",     panose.bLetterform     );
    printf("bMidline:%u ",        panose.bMidline        );
    printf("bXHeight:%u ",        panose.bXHeight        );
}

/**
    \brief Print a U_LOGFONT object.
    \param lf   U_LOGFONT object
*/
void logfont_print(
       U_LOGFONT lf
   ){
   char *string;
   printf("lfHeight:%d ",            lf.lfHeight        );
   printf("lfWidth:%d ",             lf.lfWidth         );
   printf("lfEscapement:%d ",        lf.lfEscapement    );
   printf("lfOrientation:%d ",       lf.lfOrientation   );
   printf("lfWeight:%d ",            lf.lfWeight        );
   printf("lfItalic:0x%2.2X ",         lf.lfItalic        );
   printf("lfUnderline:0x%2.2X ",      lf.lfUnderline     );
   printf("lfStrikeOut:0x%2.2X ",      lf.lfStrikeOut     );
   printf("lfCharSet:0x%2.2X ",        lf.lfCharSet       );
   printf("lfOutPrecision:0x%2.2X ",   lf.lfOutPrecision  );
   printf("lfClipPrecision:0x%2.2X ",  lf.lfClipPrecision );
   printf("lfQuality:0x%2.2X ",        lf.lfQuality       );
   printf("lfPitchAndFamily:0x%2.2X ", lf.lfPitchAndFamily);
     string = U_Utf16leToUtf8(lf.lfFaceName, U_LF_FACESIZE, NULL);
   printf("lfFaceName:%s ",   string );
   free(string);
}

/**
    \brief Print a U_LOGFONT_PANOSE object.
    \return U_LOGFONT_PANOSE object
*/
void logfont_panose_print(
      U_LOGFONT_PANOSE lfp
   ){    
   char *string;
   printf("elfLogFont:");       logfont_print(lfp.elfLogFont);
     string = U_Utf16leToUtf8(lfp.elfFullName, U_LF_FULLFACESIZE, NULL);
   printf("elfFullName:%s ",    string );
   free(string);
     string = U_Utf16leToUtf8(lfp.elfStyle, U_LF_FACESIZE, NULL);
   printf("elfStyle:%s ",       string );
   free(string);
   printf("elfVersion:%u "      ,lfp.elfVersion  );
   printf("elfStyleSize:%u "    ,lfp.elfStyleSize);
   printf("elfMatch:%u "        ,lfp.elfMatch    );
   printf("elfReserved:%u "     ,lfp.elfReserved );
   printf("elfVendorId:");      hexbytes_print((uint8_t *)lfp.elfVendorId,U_ELF_VENDOR_SIZE); printf(" ");
   printf("elfCulture:%u "      ,lfp.elfCulture  );
   printf("elfPanose:");        panose_print(lfp.elfPanose);
}

/**
    \brief Print a pointer to U_BITMAPINFOHEADER object.

    This may be called indirectly from WMF _print routines, where problems could occur
    if the data was passed as the struct or a pointer to the struct, as the struct may not
    be aligned in memory.

    \returns Actual number of color table entries.
    \param Bmih pointer to a U_BITMAPINFOHEADER object
*/
int bitmapinfoheader_print(
      const char *Bmih
   ){
   uint32_t  utmp4;
   int32_t   tmp4;
   int16_t   tmp2;
   int       Colors, BitCount, Width, Height, RealColors;

   /* DIB from a WMF may not be properly aligned on a 4 byte boundary, will be aligned on a 2 byte boundary */

   memcpy(&utmp4, Bmih + offsetof(U_BITMAPINFOHEADER,biSize),          4);  printf("biSize:%u "            ,utmp4 );
   memcpy(&tmp4,  Bmih + offsetof(U_BITMAPINFOHEADER,biWidth),         4);  printf("biWidth:%d "           ,tmp4  );
   Width = tmp4;
   memcpy(&tmp4,  Bmih + offsetof(U_BITMAPINFOHEADER,biHeight),        4);  printf("biHeight:%d "          ,tmp4  );
   Height = tmp4;
   memcpy(&tmp2,  Bmih + offsetof(U_BITMAPINFOHEADER,biPlanes),        2);  printf("biPlanes:%u "          ,tmp2  );
   memcpy(&tmp2,  Bmih + offsetof(U_BITMAPINFOHEADER,biBitCount),      2);  printf("biBitCount:%u "        ,tmp2  );
   BitCount = tmp2;
   memcpy(&utmp4, Bmih + offsetof(U_BITMAPINFOHEADER,biCompression),   4);  printf("biCompression:%u "     ,utmp4 );
   memcpy(&utmp4, Bmih + offsetof(U_BITMAPINFOHEADER,biSizeImage),     4);  printf("biSizeImage:%u "       ,utmp4 );
   memcpy(&tmp4,  Bmih + offsetof(U_BITMAPINFOHEADER,biXPelsPerMeter), 4);  printf("biXPelsPerMeter:%d "   ,tmp4  );
   memcpy(&tmp4,  Bmih + offsetof(U_BITMAPINFOHEADER,biYPelsPerMeter), 4);  printf("biYPelsPerMeter:%d "   ,tmp4  );
   memcpy(&utmp4, Bmih + offsetof(U_BITMAPINFOHEADER,biClrUsed),       4);  printf("biClrUsed:%u "         ,utmp4 );
   Colors = utmp4;
   memcpy(&utmp4, Bmih + offsetof(U_BITMAPINFOHEADER,biClrImportant),  4);  printf("biClrImportant:%u "    ,utmp4 );
   RealColors = get_real_color_icount(Colors, BitCount, Width, Height);
   printf("ColorEntries:%d ",RealColors);
   return(RealColors);
}


/**
    \brief Print a Pointer to a U_BITMAPINFO object.
    \param Bmi Pointer to a U_BITMAPINFO object
    This may be called from WMF _print routines, where problems could occur
    if the data was passed as the struct or a pointer to the struct, as the struct may not
    be aligned in memory.
*/
void bitmapinfo_print(
      const char *Bmi
   ){
   int       i,k;
   int       ClrUsed;
   U_RGBQUAD BmiColor;
   printf("BmiHeader: ");
   ClrUsed = bitmapinfoheader_print(Bmi + offsetof(U_BITMAPINFO,bmiHeader));
   if(ClrUsed){
     k= offsetof(U_BITMAPINFO,bmiColors);
     for(i=0; i<ClrUsed; i++, k+= sizeof(U_RGBQUAD)){
        memcpy(&BmiColor, Bmi+k, sizeof(U_RGBQUAD));
        printf("%d:",i); rgbquad_print(BmiColor);
     }
   }
}

/**
    \brief Print a U_BLEND object.
    \param blend a U_BLEND object
*/
void blend_print(
      U_BLEND blend
   ){
   printf("Operation:%u " ,blend.Operation);
   printf("Flags:%u "     ,blend.Flags    );
   printf("Global:%u "    ,blend.Global   );
   printf("Op:%u "        ,blend.Op       );
}

/**
    \brief Print a pointer to a U_EXTLOGPEN object.
    \param elp   PU_EXTLOGPEN object
*/
void extlogpen_print(
      PU_EXTLOGPEN elp
   ){
   unsigned int i;
   U_STYLEENTRY *elpStyleEntry;
   printf("elpPenStyle:0x%8.8X "   ,elp->elpPenStyle  );
   printf("elpWidth:%u "           ,elp->elpWidth     );
   printf("elpBrushStyle:0x%8.8X " ,elp->elpBrushStyle);
   printf("elpColor");              colorref_print(elp->elpColor);
   printf("elpHatch:%d "           ,elp->elpHatch     );
   printf("elpNumEntries:%u "      ,elp->elpNumEntries);
   if(elp->elpNumEntries){
     printf("elpStyleEntry:");
     elpStyleEntry = (uint32_t *) elp->elpStyleEntry;
     for(i=0;i<elp->elpNumEntries;i++){
        printf("%d:%u ",i,elpStyleEntry[i]);
     }
   }
}

/**
    \brief Print a U_LOGPEN object.
    \param lp  U_LOGPEN object
    
*/
void logpen_print(
      U_LOGPEN lp
   ){
   printf("lopnStyle:0x%8.8X "    ,lp.lopnStyle  );
   printf("lopnWidth:");      pointl_print(  lp.lopnWidth  );
   printf("lopnColor:");      colorref_print(lp.lopnColor );
} 

/**
    \brief Print a U_LOGPLTNTRY object.
    \param lpny Ignore U_LOGPLTNTRY object.
*/
void logpltntry_print(
      U_LOGPLTNTRY lpny
   ){
   printf("peReserved:%u " ,lpny.peReserved );
   printf("peRed:%u "      ,lpny.peRed      );
   printf("peGreen:%u "    ,lpny.peGreen    );
   printf("peBlue:%u "     ,lpny.peBlue     );
}

/**
    \brief Print a pointer to a U_LOGPALETTE object.
    \param lp  Pointer to a U_LOGPALETTE object.
*/
void logpalette_print(
      PU_LOGPALETTE lp
   ){
   int            i;
   PU_LOGPLTNTRY palPalEntry;
   printf("palVersion:%u ",    lp->palVersion );
   printf("palNumEntries:%u ", lp->palNumEntries );
   if(lp->palNumEntries){
     palPalEntry = (PU_LOGPLTNTRY) &(lp->palPalEntry);
     for(i=0;i<lp->palNumEntries;i++){
        printf("%d:",i); logpltntry_print(palPalEntry[i]);
     }
   }
}

/**
    \brief Print a U_RGNDATAHEADER object.
    \param rdh  U_RGNDATAHEADER object
*/
void rgndataheader_print(
      U_RGNDATAHEADER rdh
   ){
   printf("dwSize:%u ",   rdh.dwSize   );
   printf("iType:%u ",    rdh.iType    );
   printf("nCount:%u ",   rdh.nCount   );
   printf("nRgnSize:%u ", rdh.nRgnSize );
   printf("rclBounds:");  rectl_print(rdh.rclBounds  );
}

/**
    \brief Print a pointer to a U_RGNDATA object.
    \param rd  pointer to a U_RGNDATA object.
*/
void rgndata_print(
      PU_RGNDATA rd
   ){
   unsigned int i;
   PU_RECTL rects;
   printf("rdh:");     rgndataheader_print(rd->rdh );
   if(rd->rdh.nCount){
     rects = (PU_RECTL) &(rd->Buffer);
     for(i=0;i<rd->rdh.nCount;i++){
        printf("%d:",i); rectl_print(rects[i]);
     }
   }
}

/**
    \brief Print a U_COLORADJUSTMENT object.
    \param ca U_COLORADJUSTMENT object.        
*/
void coloradjustment_print(
      U_COLORADJUSTMENT ca
   ){
   printf("caSize:%u "            ,ca.caSize           );
   printf("caFlags:0x%4.4X "        ,ca.caFlags          );
   printf("caIlluminantIndex:%u " ,ca.caIlluminantIndex);
   printf("caRedGamma:%u "        ,ca.caRedGamma       );
   printf("caGreenGamma:%u "      ,ca.caGreenGamma     );
   printf("caBlueGamma:%u "       ,ca.caBlueGamma      );
   printf("caReferenceBlack:%u "  ,ca.caReferenceBlack );
   printf("caReferenceWhite:%u "  ,ca.caReferenceWhite );
   printf("caContrast:%d "         ,ca.caContrast       );
   printf("caBrightness:%d "       ,ca.caBrightness     );
   printf("caColorfulness:%d "     ,ca.caColorfulness   );
   printf("caRedGreenTint:%d "     ,ca.caRedGreenTint   );
}

/**
    \brief Print a U_PIXELFORMATDESCRIPTOR object.
    \param pfd  U_PIXELFORMATDESCRIPTOR object
*/
void pixelformatdescriptor_print(
      U_PIXELFORMATDESCRIPTOR pfd
   ){
   printf("nSize:%u "           ,pfd.nSize           );
   printf("nVersion:%u "        ,pfd.nVersion        );
   printf("dwFlags:0x%8.8X "      ,pfd.dwFlags         );
   printf("iPixelType:%u "      ,pfd.iPixelType      );
   printf("cColorBits:%u "      ,pfd.cColorBits      );
   printf("cRedBits:%u "        ,pfd.cRedBits        );
   printf("cRedShift:%u "       ,pfd.cRedShift       );
   printf("cGreenBits:%u "      ,pfd.cGreenBits      );
   printf("cGreenShift:%u "     ,pfd.cGreenShift     );
   printf("cBlueBits:%u "       ,pfd.cBlueBits       );
   printf("cBlueShift:%u "      ,pfd.cBlueShift      );
   printf("cAlphaBits:%u "      ,pfd.cAlphaBits      );
   printf("cAlphaShift:%u "     ,pfd.cAlphaShift     );
   printf("cAccumBits:%u "      ,pfd.cAccumBits      );
   printf("cAccumRedBits:%u "   ,pfd.cAccumRedBits   );
   printf("cAccumGreenBits:%u " ,pfd.cAccumGreenBits );
   printf("cAccumBlueBits:%u "  ,pfd.cAccumBlueBits  );
   printf("cAccumAlphaBits:%u " ,pfd.cAccumAlphaBits );
   printf("cDepthBits:%u "      ,pfd.cDepthBits      );
   printf("cStencilBits:%u "    ,pfd.cStencilBits    );
   printf("cAuxBuffers:%u "     ,pfd.cAuxBuffers     );
   printf("iLayerType:%u "      ,pfd.iLayerType      );
   printf("bReserved:%u "       ,pfd.bReserved       );
   printf("dwLayerMask:%u "     ,pfd.dwLayerMask     );
   printf("dwVisibleMask:%u "   ,pfd.dwVisibleMask   );
   printf("dwDamageMask:%u "    ,pfd.dwDamageMask    );
}

/**
    \brief Print a Pointer to a U_EMRTEXT record
    \param emt      Pointer to a U_EMRTEXT record 
    \param record   Pointer to the start of the record which contains this U_ERMTEXT
    \param type     0 for 8 bit character, anything else for 16 
*/
void emrtext_print(
      const char *emt,
      const char *record,
      int         type
   ){
   unsigned int i,off;
   char *string;
   PU_EMRTEXT pemt = (PU_EMRTEXT) emt;
   // constant part
   printf("ptlReference:");   pointl_print(pemt->ptlReference);
   printf("nChars:%u "       ,pemt->nChars      );
   printf("offString:%u "    ,pemt->offString   );
   if(pemt->offString){
      if(!type){
         printf("string8:<%s> ",record + pemt->offString);
      }
      else {
         string = U_Utf16leToUtf8((uint16_t *)(record + pemt->offString), pemt->nChars, NULL);
         printf("string16:<%s> ",string);
         free(string);
      }
   }
   printf("fOptions:0x%8.8X "     ,pemt->fOptions    );
   off = sizeof(U_EMRTEXT);
   if(!(pemt->fOptions & U_ETO_NO_RECT)){
      printf("rcl");   rectl_print( *((U_RECTL *)(emt+off)) );
      off += sizeof(U_RECTL);
   }
   printf("offDx:%u "        , *((U_OFFDX *)(emt+off))   ); off = *(U_OFFDX *)(emt+off);
   printf("Dx:");
   for(i=0; i<pemt->nChars; i++, off+=sizeof(uint32_t)){
      printf("%d:", *((uint32_t *)(record+off))  );
   }
}




// hide these from Doxygen
//! @cond
/* **********************************************************************************************
These functions contain shared code used by various U_EMR*_print functions.  These should NEVER be called
by end user code and to further that end prototypes are NOT provided and they are hidden from Doxygen.


   These are (mostly) ordered by U_EMR_* index number.
      
   The exceptions:
   void core3_print(const char *name, const char *label, const char *contents)
   void core7_print(const char *name, const char *field1, const char *field2, const char *contents)
   void core8_print(const char *name, const char *contents, int type)
   
   
*********************************************************************************************** */


// Functions with the same form starting with U_EMRPOLYBEZIER_print
void core1_print(const char *name, const char *contents){
   unsigned int i;
   UNUSED(name);
   PU_EMRPOLYLINETO pEmr = (PU_EMRPOLYLINETO) (contents);
   printf("   rclBounds:      ");    rectl_print(pEmr->rclBounds);    printf("\n");
   printf("   cptl:           %d\n",pEmr->cptl        );
   printf("   Points:         ");
   for(i=0;i<pEmr->cptl; i++){
      printf("[%d]:",i); pointl_print(pEmr->aptl[i]);
   }
   printf("\n");
}

// Functions with the same form starting with U_EMRPOLYPOLYLINE_print
void core2_print(const char *name, const char *contents){
   unsigned int i;
   UNUSED(name);
   PU_EMRPOLYPOLYGON pEmr = (PU_EMRPOLYPOLYGON) (contents);
   printf("   rclBounds:      ");    rectl_print(pEmr->rclBounds);    printf("\n");
   printf("   nPolys:         %d\n",pEmr->nPolys        );
   printf("   cptl:           %d\n",pEmr->cptl          );
   printf("   Counts:         ");
   for(i=0;i<pEmr->nPolys; i++){
      printf(" [%d]:%d ",i,pEmr->aPolyCounts[i] );
   }
   printf("\n");
   PU_POINTL paptl = (PU_POINTL)((char *)pEmr->aPolyCounts + sizeof(uint32_t)* pEmr->nPolys);
   printf("   Points:         ");
   for(i=0;i<pEmr->cptl; i++){
      printf(" [%d]:",i); pointl_print(paptl[i]);
   }
   printf("\n");
}


// Functions with the same form starting with U_EMRSETMAPMODE_print
void core3_print(const char *name, const char *label, const char *contents){
   UNUSED(name);
   PU_EMRSETMAPMODE pEmr   = (PU_EMRSETMAPMODE)(contents);
   if(!strcmp(label,"crColor:")){
      printf("   %-15s ",label); colorref_print(*(U_COLORREF *)&(pEmr->iMode)); printf("\n");
   }
   else if(!strcmp(label,"iMode:")){
      printf("   %-15s 0x%8.8X\n",label,pEmr->iMode     );
   }
   else {
      printf("   %-15s %d\n",label,pEmr->iMode        );
   }
} 

// Functions taking a single U_RECT or U_RECTL, starting with U_EMRELLIPSE_print, also U_EMRFILLPATH_print, 
void core4_print(const char *name, const char *contents){
   UNUSED(name);
   PU_EMRELLIPSE pEmr      = (PU_EMRELLIPSE)(   contents);
   printf("   rclBox:         ");  rectl_print(pEmr->rclBox);  printf("\n");
} 

// Functions with the same form starting with U_EMRPOLYBEZIER16_print
void core6_print(const char *name, const char *contents){
   UNUSED(name);
   unsigned int i;
   PU_EMRPOLYBEZIER16 pEmr = (PU_EMRPOLYBEZIER16) (contents);
   printf("   rclBounds:      ");    rectl_print(pEmr->rclBounds);    printf("\n");
   printf("   cpts:           %d\n",pEmr->cpts        );
   printf("   Points:         ");
   PU_POINT16 papts = (PU_POINT16)(&(pEmr->apts));
   for(i=0; i<pEmr->cpts; i++){
      printf(" [%d]:",i);  point16_print(papts[i]);
   }
   printf("\n");
} 


// Records with the same form starting with U_EMRSETWINDOWEXTEX_print
// CAREFUL, in the _set equivalents all functions with two uint32_t values are mapped here, and member names differ, consequently
//   print routines must supply the names of the two arguments.  These cannot be null.  If the second one is 
//   empty the values are printed as a pair {x,y}, otherwise each is printed with its own label on a separate line.
void core7_print(const char *name, const char *field1, const char *field2, const char *contents){
   UNUSED(name);
   PU_EMRGENERICPAIR pEmr = (PU_EMRGENERICPAIR) (contents);
   if(*field2){
      printf("   %-15s %d\n",field1,pEmr->pair.x);
      printf("   %-15s %d\n",field2,pEmr->pair.y);
   }
   else {
      printf("   %-15s {%d,%d}\n",field1,pEmr->pair.x,pEmr->pair.y);
   } 
}

// For U_EMREXTTEXTOUTA and U_EMREXTTEXTOUTW, type=0 for the first one
void core8_print(const char *name, const char *contents, int type){
   UNUSED(name);
   PU_EMREXTTEXTOUTA pEmr = (PU_EMREXTTEXTOUTA) (contents);
   printf("   iGraphicsMode:  %u\n",pEmr->iGraphicsMode );
   printf("   rclBounds:      ");    rectl_print(pEmr->rclBounds);                              printf("\n");
   printf("   exScale:        %f\n",pEmr->exScale        );
   printf("   eyScale:        %f\n",pEmr->eyScale        );
   printf("   emrtext:        ");
      emrtext_print(contents + sizeof(U_EMREXTTEXTOUTA) - sizeof(U_EMRTEXT),contents,type);
      printf("\n");
} 

// Functions that take a rect and a pair of points, starting with U_EMRARC_print
void core9_print(const char *name, const char *contents){
   UNUSED(name);
   PU_EMRARC pEmr = (PU_EMRARC) (contents);
   printf("   rclBox:         ");    rectl_print(pEmr->rclBox);    printf("\n");
   printf("   ptlStart:       ");  pointl_print(pEmr->ptlStart);   printf("\n");
   printf("   ptlEnd:         ");    pointl_print(pEmr->ptlEnd);   printf("\n");
}

// Functions with the same form starting with U_EMRPOLYPOLYLINE16_print
void core10_print(const char *name, const char *contents){
   UNUSED(name);
   unsigned int i;
   PU_EMRPOLYPOLYLINE16 pEmr = (PU_EMRPOLYPOLYLINE16) (contents);
   printf("   rclBounds:      ");    rectl_print(pEmr->rclBounds);    printf("\n");
   printf("   nPolys:         %d\n",pEmr->nPolys        );
   printf("   cpts:           %d\n",pEmr->cpts          );
   printf("   Counts:         ");
   for(i=0;i<pEmr->nPolys; i++){
      printf(" [%d]:%d ",i,pEmr->aPolyCounts[i] );
   }
   printf("\n");
   printf("   Points:         ");
   PU_POINT16 papts = (PU_POINT16)((char *)pEmr->aPolyCounts + sizeof(uint32_t)* pEmr->nPolys);
   for(i=0; i<pEmr->cpts; i++){
      printf(" [%d]:",i);  point16_print(papts[i]);
   }
   printf("\n");

} 

// Functions with the same form starting with  U_EMRINVERTRGN_print and U_EMRPAINTRGN_print,
void core11_print(const char *name, const char *contents){
   UNUSED(name);
   unsigned int i,roff;
   PU_EMRINVERTRGN pEmr = (PU_EMRINVERTRGN) (contents);
   printf("   rclBounds:      ");    rectl_print(pEmr->rclBounds);    printf("\n");
   printf("   cbRgnData:      %d\n",pEmr->cbRgnData);
   // This one is a pain since each RGNDATA may be a different size, so it isn't possible to index through them.
   roff=0;
   i=1; 
   char *prd = (char *) &(pEmr->RgnData);
   while(roff + sizeof(U_RGNDATAHEADER) < pEmr->cbRgnData){ // stop at end of the record 4*4 = header + 4*4=rect
      printf("   RegionData:%d",i);
      rgndata_print((PU_RGNDATA) (prd + roff));
      roff += (((PU_RGNDATA)prd)->rdh.dwSize + ((PU_RGNDATA)prd)->rdh.nRgnSize - 16);
      printf("\n");
   }
} 


// common code for U_EMRCREATEMONOBRUSH_print and U_EMRCREATEDIBPATTERNBRUSHPT_print,
void core12_print(const char *name, const char *contents){
   UNUSED(name);
   PU_EMRCREATEMONOBRUSH pEmr = (PU_EMRCREATEMONOBRUSH) (contents);
   printf("   ihBrush:      %u\n",pEmr->ihBrush );
   printf("   iUsage :      %u\n",pEmr->iUsage  );
   printf("   offBmi :      %u\n",pEmr->offBmi  );
   printf("   cbBmi  :      %u\n",pEmr->cbBmi   );
   if(pEmr->cbBmi){
      printf("      bitmap:");
      bitmapinfo_print(contents + pEmr->offBmi);
      printf("\n");
   }
   printf("   offBits:      %u\n",pEmr->offBits );
   printf("   cbBits :      %u\n",pEmr->cbBits  );
}

// common code for U_EMRALPHABLEND_print and U_EMRTRANSPARENTBLT_print,
void core13_print(const char *name, const char *contents){
   UNUSED(name);
   PU_EMRALPHABLEND pEmr = (PU_EMRALPHABLEND) (contents);
   printf("   rclBounds:      ");    rectl_print( pEmr->rclBounds);       printf("\n");
   printf("   Dest:           ");    pointl_print(pEmr->Dest);            printf("\n");
   printf("   cDest:          ");    pointl_print(pEmr->cDest);           printf("\n");
   printf("   Blend:          ");    blend_print(pEmr->Blend);            printf("\n");
   printf("   Src:            ");    pointl_print(pEmr->Src);             printf("\n");
   printf("   xformSrc:       ");    xform_print( pEmr->xformSrc);        printf("\n");
   printf("   crBkColorSrc:   ");    colorref_print( pEmr->crBkColorSrc); printf("\n");
   printf("   iUsageSrc:      %u\n",pEmr->iUsageSrc   );
   printf("   offBmiSrc:      %u\n",pEmr->offBmiSrc   );
   printf("   cbBmiSrc:       %u\n",pEmr->cbBmiSrc    );
   if(pEmr->cbBmiSrc){
      printf("      bitmap:");
      bitmapinfo_print(contents + pEmr->offBmiSrc);
      printf("\n");
   }
   printf("   offBitsSrc:     %u\n",pEmr->offBitsSrc  );
   printf("   cbBitsSrc:      %u\n",pEmr->cbBitsSrc   );
}
//! @endcond

/* **********************************************************************************************
These are the core EMR functions, each creates a particular type of record.  
All return these records via a char* pointer, which is NULL if the call failed.  
They are listed in order by the corresponding U_EMR_* index number.  
*********************************************************************************************** */

/**
    \brief Print a pointer to a U_EMR_whatever record which has not been implemented.
    \param name       name of this type of record
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRNOTIMPLEMENTED_print(const char *name, const char *contents){
   UNUSED(name);
   UNUSED(contents);
   printf("   Not Implemented!\n");
}

// U_EMRHEADER                1
/**
    \brief Print a pointer to a U_EMR_HEADER record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRHEADER_print(const char *contents){
   char *string;
   int  p1len;

   PU_EMRHEADER pEmr = (PU_EMRHEADER)(contents);
   printf("   rclBounds:      ");          rectl_print( pEmr->rclBounds);   printf("\n");
   printf("   rclFrame:       ");          rectl_print( pEmr->rclFrame);    printf("\n");
   printf("   dSignature:     0x%8.8X\n",  pEmr->dSignature    );
   printf("   nVersion:       0x%8.8X\n",  pEmr->nVersion      );
   printf("   nBytes:         %d\n",       pEmr->nBytes        );
   printf("   nRecords:       %d\n",       pEmr->nRecords      );
   printf("   nHandles:       %d\n",       pEmr->nHandles      );
   printf("   sReserved:      %d\n",       pEmr->sReserved     );
   printf("   nDescription:   %d\n",       pEmr->nDescription  );
   printf("   offDescription: %d\n",       pEmr->offDescription);
   if(pEmr->offDescription){
      string = U_Utf16leToUtf8((uint16_t *)((char *) pEmr + pEmr->offDescription), pEmr->nDescription, NULL);
      printf("      Desc. A:  %s\n",string);
      free(string);
      p1len = 2 + 2*wchar16len((uint16_t *)((char *) pEmr + pEmr->offDescription));
      string = U_Utf16leToUtf8((uint16_t *)((char *) pEmr + pEmr->offDescription + p1len), pEmr->nDescription, NULL);
      printf("      Desc. B:  %s\n",string);
      free(string);
   }
   printf("   nPalEntries:    %d\n",       pEmr->nPalEntries   );
   printf("   szlDevice:      {%d,%d} \n", pEmr->szlDevice.cx,pEmr->szlDevice.cy);
   printf("   szlMillimeters: {%d,%d} \n", pEmr->szlMillimeters.cx,pEmr->szlMillimeters.cy);
   if((pEmr->nDescription && (pEmr->offDescription >= 100)) || 
      (!pEmr->offDescription && pEmr->emr.nSize >= 100)
     ){
      printf("   cbPixelFormat:  %d\n",       pEmr->cbPixelFormat );
      printf("   offPixelFormat: %d\n",       pEmr->offPixelFormat);
      if(pEmr->cbPixelFormat){
         printf("      PFD:");
         pixelformatdescriptor_print( *(PU_PIXELFORMATDESCRIPTOR) (contents + pEmr->offPixelFormat));
         printf("\n");
      }
      printf("   bOpenGL:        %d\n",pEmr->bOpenGL       );
      if((pEmr->nDescription    && (pEmr->offDescription >= 108)) || 
              (pEmr->cbPixelFormat   && (pEmr->offPixelFormat >=108)) ||
              (!pEmr->offDescription && !pEmr->cbPixelFormat && pEmr->emr.nSize >= 108)
             ){
         printf("   szlMicrometers: {%d,%d} \n", pEmr->szlMicrometers.cx,pEmr->szlMicrometers.cy);
     }
   }
}

// U_EMRPOLYBEZIER                       2
/**
    \brief Print a pointer to a U_EMR_POLYBEZIER record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPOLYBEZIER_print(const char *contents){
   core1_print("U_EMRPOLYBEZIER", contents);
} 

// U_EMRPOLYGON                          3
/**
    \brief Print a pointer to a U_EMR_POLYGON record.
    \param contents   pointer to a buffer holding all EMR records
 */
void U_EMRPOLYGON_print(const char *contents){
   core1_print("U_EMRPOLYGON", contents);
} 


// U_EMRPOLYLINE              4
/**
    \brief Print a pointer to a U_EMR_POLYLINE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPOLYLINE_print(const char *contents){
   core1_print("U_EMRPOLYLINE", contents);
} 

// U_EMRPOLYBEZIERTO          5
/**
    \brief Print a pointer to a U_EMR_POLYBEZIERTO record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPOLYBEZIERTO_print(const char *contents){
   core1_print("U_EMRPOLYBEZIERTO", contents);
} 

// U_EMRPOLYLINETO            6
/**
    \brief Print a pointer to a U_EMR_POLYLINETO record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPOLYLINETO_print(const char *contents){
   core1_print("U_EMRPOLYLINETO", contents);
} 

// U_EMRPOLYPOLYLINE          7
/**
    \brief Print a pointer to a U_EMR_POLYPOLYLINE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPOLYPOLYLINE_print(const char *contents){
   core2_print("U_EMRPOLYPOLYLINE", contents);
} 

// U_EMRPOLYPOLYGON           8
/**
    \brief Print a pointer to a U_EMR_POLYPOLYGON record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPOLYPOLYGON_print(const char *contents){
   core2_print("U_EMRPOLYPOLYGON", contents);
} 

// U_EMRSETWINDOWEXTEX        9
/**
    \brief Print a pointer to a U_EMR_SETWINDOWEXTEX record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETWINDOWEXTEX_print(const char *contents){
   core7_print("U_EMRSETWINDOWEXTEX", "szlExtent:","",contents);
} 

// U_EMRSETWINDOWORGEX       10
/**
    \brief Print a pointer to a U_EMR_SETWINDOWORGEX record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETWINDOWORGEX_print(const char *contents){
   core7_print("U_EMRSETWINDOWORGEX", "ptlOrigin:","",contents);
} 

// U_EMRSETVIEWPORTEXTEX     11
/**
    \brief Print a pointer to a U_EMR_SETVIEWPORTEXTEX record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETVIEWPORTEXTEX_print(const char *contents){
   core7_print("U_EMRSETVIEWPORTEXTEX", "szlExtent:","",contents);
} 

// U_EMRSETVIEWPORTORGEX     12
/**
    \brief Print a pointer to a U_EMR_SETVIEWPORTORGEX record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETVIEWPORTORGEX_print(const char *contents){
   core7_print("U_EMRSETVIEWPORTORGEX", "ptlOrigin:","",contents);
} 

// U_EMRSETBRUSHORGEX        13
/**
    \brief Print a pointer to a U_EMR_SETBRUSHORGEX record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETBRUSHORGEX_print(const char *contents){
   core7_print("U_EMRSETBRUSHORGEX", "ptlOrigin:","",contents);
} 

// U_EMREOF                  14
/**
    \brief Print a pointer to a U_EMR_EOF record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMREOF_print(const char *contents){
   PU_EMREOF pEmr = (PU_EMREOF)(contents);
   printf("   cbPalEntries:   %u\n",      pEmr->cbPalEntries );
   printf("   offPalEntries:  %u\n",      pEmr->offPalEntries);
   if(pEmr->cbPalEntries){
     printf("      PE:");
     logpalette_print( (PU_LOGPALETTE)(contents + pEmr->offPalEntries));
     printf("\n");
   }
} 


// U_EMRSETPIXELV            15
/**
    \brief Print a pointer to a U_EMR_SETPIXELV record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETPIXELV_print(const char *contents){
   PU_EMRSETPIXELV pEmr = (PU_EMRSETPIXELV)(contents);
   printf("   ptlPixel:       ");  pointl_print(  pEmr->ptlPixel);  printf("\n");
   printf("   crColor:        ");  colorref_print(pEmr->crColor);   printf("\n");
} 


// U_EMRSETMAPPERFLAGS       16
/**
    \brief Print a pointer to a U_EMR_SETMAPPERFLAGS record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETMAPPERFLAGS_print(const char *contents){
   PU_EMRSETMAPPERFLAGS pEmr = (PU_EMRSETMAPPERFLAGS)(contents);
   printf("   dwFlags:        0x%8.8X\n",pEmr->dwFlags);
} 


// U_EMRSETMAPMODE           17
/**
    \brief Print a pointer to a U_EMR_SETMAPMODE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETMAPMODE_print(const char *contents){
   core3_print("U_EMRSETMAPMODE", "iMode:", contents);
}

// U_EMRSETBKMODE            18
/**
    \brief Print a pointer to a U_EMR_SETBKMODE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETBKMODE_print(const char *contents){
   core3_print("U_EMRSETBKMODE", "iMode:", contents);
}

// U_EMRSETPOLYFILLMODE      19
/**
    \brief Print a pointer to a U_EMR_SETPOLYFILLMODE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETPOLYFILLMODE_print(const char *contents){
   core3_print("U_EMRSETPOLYFILLMODE", "iMode:", contents);
}

// U_EMRSETROP2              20
/**
    \brief Print a pointer to a U_EMR_SETROP2 record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETROP2_print(const char *contents){
   core3_print("U_EMRSETROP2", "dwRop:", contents);
}

// U_EMRSETSTRETCHBLTMODE    21
/**
    \brief Print a pointer to a U_EMR_SETSTRETCHBLTMODE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETSTRETCHBLTMODE_print(const char *contents){
   core3_print("U_EMRSETSTRETCHBLTMODE", "iMode:", contents);
}

// U_EMRSETTEXTALIGN         22
/**
    \brief Print a pointer to a U_EMR_SETTEXTALIGN record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETTEXTALIGN_print(const char *contents){
   core3_print("U_EMRSETTEXTALIGN", "iMode:", contents);
}

// U_EMRSETCOLORADJUSTMENT   23
/**
    \brief Print a pointer to a U_EMR_SETCOLORADJUSTMENT record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETCOLORADJUSTMENT_print(const char *contents){
   PU_EMRSETCOLORADJUSTMENT pEmr = (PU_EMRSETCOLORADJUSTMENT)(contents);
   printf("   ColorAdjustment:");
   coloradjustment_print(pEmr->ColorAdjustment);
   printf("\n");
}

// U_EMRSETTEXTCOLOR         24
/**
    \brief Print a pointer to a U_EMR_SETTEXTCOLOR record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETTEXTCOLOR_print(const char *contents){
   core3_print("U_EMRSETTEXTCOLOR", "crColor:", contents);
}

// U_EMRSETBKCOLOR           25
/**
    \brief Print a pointer to a U_EMR_SETBKCOLOR record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETBKCOLOR_print(const char *contents){
   core3_print("U_EMRSETBKCOLOR", "crColor:", contents);
}

// U_EMROFFSETCLIPRGN        26
/**
    \brief Print a pointer to a U_EMR_OFFSETCLIPRGN record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMROFFSETCLIPRGN_print(const char *contents){
   core7_print("U_EMROFFSETCLIPRGN", "ptl:","",contents);
} 

// U_EMRMOVETOEX             27
/**
    \brief Print a pointer to a U_EMR_MOVETOEX record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRMOVETOEX_print(const char *contents){
   core7_print("U_EMRMOVETOEX", "ptl:","",contents);
} 

// U_EMRSETMETARGN           28
/**
    \brief Print a pointer to a U_EMR_SETMETARGN record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETMETARGN_print(const char *contents){
   UNUSED(contents);
}

// U_EMREXCLUDECLIPRECT      29
/**
    \brief Print a pointer to a U_EMR_EXCLUDECLIPRECT record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMREXCLUDECLIPRECT_print(const char *contents){
   core4_print("U_EMREXCLUDECLIPRECT", contents);
}

// U_EMRINTERSECTCLIPRECT    30
/**
    \brief Print a pointer to a U_EMR_INTERSECTCLIPRECT record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRINTERSECTCLIPRECT_print(const char *contents){
   core4_print("U_EMRINTERSECTCLIPRECT", contents);
}

// U_EMRSCALEVIEWPORTEXTEX   31
/**
    \brief Print a pointer to a U_EMR_SCALEVIEWPORTEXTEX record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSCALEVIEWPORTEXTEX_print(const char *contents){
   core4_print("U_EMRSCALEVIEWPORTEXTEX", contents);
}


// U_EMRSCALEWINDOWEXTEX     32
/**
    \brief Print a pointer to a U_EMR_SCALEWINDOWEXTEX record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSCALEWINDOWEXTEX_print(const char *contents){
   core4_print("U_EMRSCALEWINDOWEXTEX", contents);
}

// U_EMRSAVEDC               33
/**
    \brief Print a pointer to a U_EMR_SAVEDC record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSAVEDC_print(const char *contents){
   UNUSED(contents);
}

// U_EMRRESTOREDC            34
/**
    \brief Print a pointer to a U_EMR_RESTOREDC record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRRESTOREDC_print(const char *contents){
   core3_print("U_EMRRESTOREDC", "iRelative:", contents);
}

// U_EMRSETWORLDTRANSFORM    35
/**
    \brief Print a pointer to a U_EMR_SETWORLDTRANSFORM record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETWORLDTRANSFORM_print(const char *contents){
   PU_EMRSETWORLDTRANSFORM pEmr = (PU_EMRSETWORLDTRANSFORM)(contents);
   printf("   xform:");
   xform_print(pEmr->xform);
   printf("\n");
} 

// U_EMRMODIFYWORLDTRANSFORM 36
/**
    \brief Print a pointer to a U_EMR_MODIFYWORLDTRANSFORM record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRMODIFYWORLDTRANSFORM_print(const char *contents){
   PU_EMRMODIFYWORLDTRANSFORM pEmr = (PU_EMRMODIFYWORLDTRANSFORM)(contents);
   printf("   xform:");
   xform_print(pEmr->xform);
   printf("\n");
   printf("   iMode:          %u\n",      pEmr->iMode );
} 

// U_EMRSELECTOBJECT         37
/**
    \brief Print a pointer to a U_EMR_SELECTOBJECT record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSELECTOBJECT_print(const char *contents){
   PU_EMRSELECTOBJECT pEmr = (PU_EMRSELECTOBJECT)(contents);
   if(pEmr->ihObject & U_STOCK_OBJECT){
     printf("   StockObject:    0x%8.8X\n",  pEmr->ihObject );
   }
   else {
     printf("   ihObject:       %u\n",     pEmr->ihObject );
   }
} 

// U_EMRCREATEPEN            38
/**
    \brief Print a pointer to a U_EMR_CREATEPEN record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRCREATEPEN_print(const char *contents){
   PU_EMRCREATEPEN pEmr = (PU_EMRCREATEPEN)(contents);
   printf("   ihPen:          %u\n",      pEmr->ihPen );
   printf("   lopn:           ");    logpen_print(pEmr->lopn);  printf("\n");
} 

// U_EMRCREATEBRUSHINDIRECT  39
/**
    \brief Print a pointer to a U_EMR_CREATEBRUSHINDIRECT record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRCREATEBRUSHINDIRECT_print(const char *contents){
   PU_EMRCREATEBRUSHINDIRECT pEmr = (PU_EMRCREATEBRUSHINDIRECT)(contents);
   printf("   ihBrush:        %u\n",      pEmr->ihBrush );
   printf("   lb:             ");         logbrush_print(pEmr->lb);  printf("\n");
} 

// U_EMRDELETEOBJECT         40
/**
    \brief Print a pointer to a U_EMR_DELETEOBJECT record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRDELETEOBJECT_print(const char *contents){
   PU_EMRDELETEOBJECT pEmr = (PU_EMRDELETEOBJECT)(contents);
   printf("   ihObject:       %u\n",      pEmr->ihObject );
} 

// U_EMRANGLEARC             41
/**
    \brief Print a pointer to a U_EMR_ANGLEARC record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRANGLEARC_print(const char *contents){
   PU_EMRANGLEARC pEmr = (PU_EMRANGLEARC)(contents);
   printf("   ptlCenter:      "), pointl_print(pEmr->ptlCenter ); printf("\n");
   printf("   nRadius:        %u\n",      pEmr->nRadius );
   printf("   eStartAngle:    %f\n",       pEmr->eStartAngle );
   printf("   eSweepAngle:    %f\n",       pEmr->eSweepAngle );
} 

// U_EMRELLIPSE              42
/**
    \brief Print a pointer to a U_EMR_ELLIPSE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRELLIPSE_print(const char *contents){
   core4_print("U_EMRELLIPSE", contents);
}

// U_EMRRECTANGLE            43
/**
    \brief Print a pointer to a U_EMR_RECTANGLE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRRECTANGLE_print(const char *contents){
   core4_print("U_EMRRECTANGLE", contents);
}

// U_EMRROUNDRECT            44
/**
    \brief Print a pointer to a U_EMR_ROUNDRECT record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRROUNDRECT_print(const char *contents){
   PU_EMRROUNDRECT pEmr = (PU_EMRROUNDRECT)(contents);
   printf("   rclBox:         "), rectl_print(pEmr->rclBox );     printf("\n");
   printf("   szlCorner:      "), sizel_print(pEmr->szlCorner );  printf("\n");
}

// U_EMRARC                  45
/**
    \brief Print a pointer to a U_EMR_ARC record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRARC_print(const char *contents){
   core9_print("U_EMRARC", contents);
}

// U_EMRCHORD                46
/**
    \brief Print a pointer to a U_EMR_CHORD record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRCHORD_print(const char *contents){
   core9_print("U_EMRCHORD", contents);
}

// U_EMRPIE                  47
/**
    \brief Print a pointer to a U_EMR_PIE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPIE_print(const char *contents){
   core9_print("U_EMRPIE", contents);
}

// U_EMRSELECTPALETTE        48
/**
    \brief Print a pointer to a U_EMR_SELECTPALETTE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSELECTPALETTE_print(const char *contents){
   core3_print("U_EMRSELECTPALETTE", "ihPal:", contents);
}

// U_EMRCREATEPALETTE        49
/**
    \brief Print a pointer to a U_EMR_CREATEPALETTE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRCREATEPALETTE_print(const char *contents){
   PU_EMRCREATEPALETTE pEmr = (PU_EMRCREATEPALETTE)(contents);
   printf("   ihPal:          %u\n",pEmr->ihPal);
   printf("   lgpl:           "), logpalette_print( (PU_LOGPALETTE)&(pEmr->lgpl) );  printf("\n");
}

// U_EMRSETPALETTEENTRIES    50
/**
    \brief Print a pointer to a U_EMR_SETPALETTEENTRIES record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETPALETTEENTRIES_print(const char *contents){
   unsigned int i;
   PU_EMRSETPALETTEENTRIES pEmr = (PU_EMRSETPALETTEENTRIES)(contents);
   printf("   ihPal:          %u\n",pEmr->ihPal);
   printf("   iStart:         %u\n",pEmr->iStart);
   printf("   cEntries:       %u\n",pEmr->cEntries);
   if(pEmr->cEntries){
      printf("      PLTEntries:");
      PU_LOGPLTNTRY aPalEntries = (PU_LOGPLTNTRY) &(pEmr->aPalEntries);
      for(i=0; i<pEmr->cEntries; i++){
         printf("%d:",i); logpltntry_print(aPalEntries[i]);
      }
      printf("\n");
   }
}

// U_EMRRESIZEPALETTE        51
/**
    \brief Print a pointer to a U_EMR_RESIZEPALETTE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRRESIZEPALETTE_print(const char *contents){
   core7_print("U_EMRRESIZEPALETTE", "ihPal:","cEntries",contents);
} 

// U_EMRREALIZEPALETTE       52
/**
    \brief Print a pointer to a U_EMR_REALIZEPALETTE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRREALIZEPALETTE_print(const char *contents){
   UNUSED(contents);
}

// U_EMREXTFLOODFILL         53
/**
    \brief Print a pointer to a U_EMR_EXTFLOODFILL record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMREXTFLOODFILL_print(const char *contents){
   PU_EMREXTFLOODFILL pEmr = (PU_EMREXTFLOODFILL)(contents);
   printf("   ptlStart:       ");   pointl_print(pEmr->ptlStart);    printf("\n");
   printf("   crColor:        ");   colorref_print(pEmr->crColor);   printf("\n");
   printf("   iMode:          %u\n",pEmr->iMode);
}

// U_EMRLINETO               54
/**
    \brief Print a pointer to a U_EMR_LINETO record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRLINETO_print(const char *contents){
   core7_print("U_EMRLINETO", "ptl:","",contents);
} 

// U_EMRARCTO                55
/**
    \brief Print a pointer to a U_EMR_ARCTO record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRARCTO_print(const char *contents){
   core9_print("U_EMRARCTO", contents);
}

// U_EMRPOLYDRAW             56
/**
    \brief Print a pointer to a U_EMR_POLYDRAW record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPOLYDRAW_print(const char *contents){
   unsigned int i;
   PU_EMRPOLYDRAW pEmr = (PU_EMRPOLYDRAW)(contents);
   printf("   rclBounds:      ");          rectl_print( pEmr->rclBounds);   printf("\n");
   printf("   cptl:           %d\n",pEmr->cptl        );
   printf("   Points:         ");
   for(i=0;i<pEmr->cptl; i++){
      printf(" [%d]:",i);
      pointl_print(pEmr->aptl[i]);
   }
   printf("\n");
   printf("   Types:          ");
   for(i=0;i<pEmr->cptl; i++){
      printf(" [%d]:%u ",i,pEmr->abTypes[i]);
   }
   printf("\n");
}

// U_EMRSETARCDIRECTION      57
/**
    \brief Print a pointer to a U_EMR_SETARCDIRECTION record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETARCDIRECTION_print(const char *contents){
   core3_print("U_EMRSETARCDIRECTION","arcDirection:", contents);
}

// U_EMRSETMITERLIMIT        58
/**
    \brief Print a pointer to a U_EMR_SETMITERLIMIT record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETMITERLIMIT_print(const char *contents){
   core3_print("U_EMRSETMITERLIMIT", "eMiterLimit:", contents);
}


// U_EMRBEGINPATH            59
/**
    \brief Print a pointer to a U_EMR_BEGINPATH record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRBEGINPATH_print(const char *contents){
   UNUSED(contents);
}

// U_EMRENDPATH              60
/**
    \brief Print a pointer to a U_EMR_ENDPATH record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRENDPATH_print(const char *contents){
   UNUSED(contents);
}

// U_EMRCLOSEFIGURE          61
/**
    \brief Print a pointer to a U_EMR_CLOSEFIGURE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRCLOSEFIGURE_print(const char *contents){
   UNUSED(contents);
}

// U_EMRFILLPATH             62
/**
    \brief Print a pointer to a U_EMR_FILLPATH record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRFILLPATH_print(const char *contents){
   core4_print("U_EMRFILLPATH", contents);
}

// U_EMRSTROKEANDFILLPATH    63
/**
    \brief Print a pointer to a U_EMR_STROKEANDFILLPATH record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSTROKEANDFILLPATH_print(const char *contents){
   core4_print("U_EMRSTROKEANDFILLPATH", contents);
}

// U_EMRSTROKEPATH           64
/**
    \brief Print a pointer to a U_EMR_STROKEPATH record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSTROKEPATH_print(const char *contents){
   core4_print("U_EMRSTROKEPATH", contents);
}

// U_EMRFLATTENPATH          65
/**
    \brief Print a pointer to a U_EMR_FLATTENPATH record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRFLATTENPATH_print(const char *contents){
   UNUSED(contents);
}

// U_EMRWIDENPATH            66
/**
    \brief Print a pointer to a U_EMR_WIDENPATH record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRWIDENPATH_print(const char *contents){
   UNUSED(contents);
}

// U_EMRSELECTCLIPPATH       67
/**
    \brief Print a pointer to a U_EMR_SELECTCLIPPATH record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSELECTCLIPPATH_print(const char *contents){
   core3_print("U_EMRSELECTCLIPPATH", "iMode:", contents);
}

// U_EMRABORTPATH            68
/**
    \brief Print a pointer to a U_EMR_ABORTPATH record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRABORTPATH_print(const char *contents){
   UNUSED(contents);
}

// U_EMRUNDEF69                       69
#define U_EMRUNDEF69_print(A) U_EMRNOTIMPLEMENTED_print("U_EMRUNDEF69",A) //!< Not implemented.

// U_EMRCOMMENT              70  Comment (any binary data, interpretation is program specific)
/**
    \brief Print a pointer to a U_EMR_COMMENT record.
    \param contents   pointer to a location in memory holding the comment record
    \param blimit     size in bytes of the comment record
    \param off        offset in bytes to the first byte in this record

    EMF+ records, if any, are stored in EMF comment records.
*/
void U_EMRCOMMENT_print(const char *contents, const char *blimit, size_t off){
   char *string;
   char *src;
   uint32_t cIdent,cIdent2,cbData;
   size_t loff;
   int    recsize;
   static int recnum=0;

   PU_EMRCOMMENT pEmr = (PU_EMRCOMMENT)(contents);

   /* There are several different types of comments */

   cbData = pEmr->cbData;
   printf("   cbData:         %d\n",cbData        );
   src = (char *)&(pEmr->Data);  // default
   if(cbData >= 4){
      /* Since the comment is just a big bag of bytes the emf endian code cannot safely touch
         any of its payload.  This is the only record type with that limitation.  Try to determine
	 what the contents are even if more byte swapping is required. */
      cIdent = *(uint32_t *)(src);
      if(U_BYTE_SWAP){ U_swap4(&(cIdent),1); }
      if(     cIdent == U_EMR_COMMENT_PUBLIC       ){
         printf("   cIdent:  Public\n");
         PU_EMRCOMMENT_PUBLIC pEmrp = (PU_EMRCOMMENT_PUBLIC) pEmr;
	 cIdent2 = pEmrp->pcIdent;
         if(U_BYTE_SWAP){ U_swap4(&(cIdent2),1); }
         printf("   pcIdent:        0x%8.8x\n",cIdent2);
         src = (char *)&(pEmrp->Data);
         cbData -= 8;
      }
      else if(cIdent == U_EMR_COMMENT_SPOOL        ){
         printf("   cIdent:  Spool\n");
         PU_EMRCOMMENT_SPOOL pEmrs = (PU_EMRCOMMENT_SPOOL) pEmr;
	 cIdent2 = pEmrs->esrIdent;
         if(U_BYTE_SWAP){ U_swap4(&(cIdent2),1); }
         printf("   esrIdent:       0x%8.8x\n",cIdent2);
         src = (char *)&(pEmrs->Data);
         cbData -= 8;
      }
      else if(cIdent == U_EMR_COMMENT_EMFPLUSRECORD){
         printf("   cIdent:  EMF+\n");
         PU_EMRCOMMENT_EMFPLUS pEmrpl = (PU_EMRCOMMENT_EMFPLUS) pEmr;
         src = (char *)&(pEmrpl->Data);
         loff = 16;  /* Header size of the header part of an EMF+ comment record */
         while(loff < cbData + 12){  // EMF+ records may not fill the entire comment, cbData value includes cIdent, but not U_EMR or cbData
            recsize =  U_pmf_onerec_print(src, blimit, recnum, loff + off);
            if(recsize<=0)break;
            loff += recsize;
            src  += recsize;
            recnum++;
         }
         return;
      }
      else {
         printf("   cIdent:         not (Public or Spool or EMF+)\n");
      }
   }
   if(cbData){ // The data may not be printable, but try it just in case
      string = malloc(cbData + 1);
      (void)strncpy(string, src, cbData);
      string[cbData] = '\0'; // it might not be terminated - it might not even be text!
      printf("   Data:           <%s>\n",string);
      free(string);
   }
} 

// U_EMRFILLRGN              71
/**
    \brief Print a pointer to a U_EMR_FILLRGN record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRFILLRGN_print(const char *contents){
   int i,roff;
   PU_EMRFILLRGN pEmr = (PU_EMRFILLRGN)(contents);
   printf("   rclBounds:      ");    rectl_print(pEmr->rclBounds);    printf("\n");
   printf("   cbRgnData:      %u\n",pEmr->cbRgnData);
   printf("   ihBrush:        %u\n",pEmr->ihBrush);
   // This one is a pain since each RGNDATA may be a different size, so it isn't possible to index through them.
   roff=0;
   i=1; 
   char *prd = (char *) &(pEmr->RgnData);
   while(roff  + sizeof(U_RGNDATAHEADER) < pEmr->emr.nSize){ // up to the end of the record
      printf("   RegionData[%d]: ",i);   rgndata_print((PU_RGNDATA) (prd + roff));  printf("\n");
      roff += (((PU_RGNDATA)prd)->rdh.dwSize + ((PU_RGNDATA)prd)->rdh.nRgnSize - 16);
   }
} 

// U_EMRFRAMERGN             72
/**
    \brief Print a pointer to a U_EMR_FRAMERGN record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRFRAMERGN_print(const char *contents){
   int i,roff;
   PU_EMRFRAMERGN pEmr = (PU_EMRFRAMERGN)(contents);
   printf("   rclBounds:      ");    rectl_print(pEmr->rclBounds);    printf("\n");
   printf("   cbRgnData:      %u\n",pEmr->cbRgnData);
   printf("   ihBrush:        %u\n",pEmr->ihBrush);
   printf("   szlStroke:      "), sizel_print(pEmr->szlStroke );      printf("\n");
   // This one is a pain since each RGNDATA may be a different size, so it isn't possible to index through them.
   roff=0;
   i=1; 
   char *prd = (char *) &(pEmr->RgnData);
   while(roff  + sizeof(U_RGNDATAHEADER) < pEmr->emr.nSize){ // up to the end of the record
      printf("   RegionData[%d]: ",i);   rgndata_print((PU_RGNDATA) (prd + roff));  printf("\n");
      roff += (((PU_RGNDATA)prd)->rdh.dwSize + ((PU_RGNDATA)prd)->rdh.nRgnSize - 16);
   }
} 

// U_EMRINVERTRGN            73
/**
    \brief Print a pointer to a U_EMR_INVERTRGN record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRINVERTRGN_print(const char *contents){
   core11_print("U_EMRINVERTRGN", contents);
}

// U_EMRPAINTRGN             74
/**
    \brief Print a pointer to a U_EMR_PAINTRGN record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPAINTRGN_print(const char *contents){
   core11_print("U_EMRPAINTRGN", contents);
}

// U_EMREXTSELECTCLIPRGN     75
/**
    \brief Print a pointer to a U_EMR_EXTSELECTCLIPRGN record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMREXTSELECTCLIPRGN_print(const char *contents){
   int i,roff;
   PU_EMREXTSELECTCLIPRGN pEmr = (PU_EMREXTSELECTCLIPRGN) (contents);
   printf("   cbRgnData:      %u\n",pEmr->cbRgnData);
   printf("   iMode:          %u\n",pEmr->iMode);
   // This one is a pain since each RGNDATA may be a different size, so it isn't possible to index through them.
   char *prd = (char *) &(pEmr->RgnData);
   i=roff=0;
   while(roff + sizeof(U_RGNDATAHEADER) < pEmr->cbRgnData){ // stop at end of the record 4*4 = header + 4*4=rect
      printf("   RegionData[%d]: ",i++); rgndata_print((PU_RGNDATA) (prd + roff)); printf("\n");
      roff += (((PU_RGNDATA)prd)->rdh.dwSize + ((PU_RGNDATA)prd)->rdh.nRgnSize - 16);
   }
} 

// U_EMRBITBLT               76
/**
    \brief Print a pointer to a U_EMR_BITBLT record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRBITBLT_print(const char *contents){
   PU_EMRBITBLT pEmr = (PU_EMRBITBLT) (contents);
   printf("   rclBounds:      ");     rectl_print( pEmr->rclBounds);       printf("\n");
   printf("   Dest:           ");     pointl_print(pEmr->Dest);            printf("\n");
   printf("   cDest:          ");     pointl_print(pEmr->cDest);           printf("\n");
   printf("   dwRop :         0x%8.8X\n", pEmr->dwRop   );
   printf("   Src:            ");     pointl_print(pEmr->Src);             printf("\n");
   printf("   xformSrc:       ");     xform_print( pEmr->xformSrc);        printf("\n");
   printf("   crBkColorSrc:   ");     colorref_print( pEmr->crBkColorSrc); printf("\n");
   printf("   iUsageSrc:      %u\n", pEmr->iUsageSrc   );
   printf("   offBmiSrc:      %u\n", pEmr->offBmiSrc   );
   printf("   cbBmiSrc:       %u\n", pEmr->cbBmiSrc    );
   if(pEmr->cbBmiSrc){
      printf("      bitmap:      ");
      bitmapinfo_print(contents + pEmr->offBmiSrc);
      printf("\n");
   }
   printf("   offBitsSrc:     %u\n", pEmr->offBitsSrc   );
   printf("   cbBitsSrc:      %u\n", pEmr->cbBitsSrc    );
}

// U_EMRSTRETCHBLT           77
/**
    \brief Print a pointer to a U_EMR_STRETCHBLT record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSTRETCHBLT_print(const char *contents){
   PU_EMRSTRETCHBLT pEmr = (PU_EMRSTRETCHBLT) (contents);
   printf("   rclBounds:      ");     rectl_print( pEmr->rclBounds);       printf("\n");
   printf("   Dest:           ");     pointl_print(pEmr->Dest);            printf("\n");
   printf("   cDest:          ");     pointl_print(pEmr->cDest);           printf("\n");
   printf("   dwRop :         0x%8.8X\n", pEmr->dwRop   );
   printf("   Src:            ");     pointl_print(pEmr->Src);             printf("\n");
   printf("   xformSrc:       ");     xform_print( pEmr->xformSrc);        printf("\n");
   printf("   crBkColorSrc:   ");     colorref_print( pEmr->crBkColorSrc); printf("\n");
   printf("   iUsageSrc:      %u\n", pEmr->iUsageSrc   );
   printf("   offBmiSrc:      %u\n", pEmr->offBmiSrc   );
   printf("   cbBmiSrc:       %u\n", pEmr->cbBmiSrc    );
   if(pEmr->cbBmiSrc){
      printf("      bitmap:      ");
      bitmapinfo_print(contents + pEmr->offBmiSrc);
      printf("\n");
   }
   printf("   offBitsSrc:     %u\n", pEmr->offBitsSrc   );
   printf("   cbBitsSrc:      %u\n", pEmr->cbBitsSrc    );
   printf("   cSrc:           ");     pointl_print(pEmr->cSrc);            printf("\n");
}

// U_EMRMASKBLT              78
/**
    \brief Print a pointer to a U_EMR_MASKBLT record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRMASKBLT_print(const char *contents){
   PU_EMRMASKBLT pEmr = (PU_EMRMASKBLT) (contents);
   printf("   rclBounds:      ");     rectl_print( pEmr->rclBounds);       printf("\n");
   printf("   Dest:           ");     pointl_print(pEmr->Dest);            printf("\n");
   printf("   cDest:          ");     pointl_print(pEmr->cDest);           printf("\n");
   printf("   dwRop :         0x%8.8X\n",  pEmr->dwRop   );
   printf("   Src:            ");     pointl_print(pEmr->Src);             printf("\n");
   printf("   xformSrc:       ");     xform_print( pEmr->xformSrc);        printf("\n");
   printf("   crBkColorSrc:   ");     colorref_print( pEmr->crBkColorSrc); printf("\n");
   printf("   iUsageSrc:      %u\n",  pEmr->iUsageSrc   );
   printf("   offBmiSrc:      %u\n",  pEmr->offBmiSrc   );
   printf("   cbBmiSrc:       %u\n",  pEmr->cbBmiSrc    );
   if(pEmr->cbBmiSrc){
      printf("      Src bitmap:  ");
      bitmapinfo_print(contents + pEmr->offBmiSrc);
      printf("\n");
   }
   printf("   offBitsSrc:     %u\n",  pEmr->offBitsSrc   );
   printf("   cbBitsSrc:      %u\n",  pEmr->cbBitsSrc    );
   printf("   Mask:           ");     pointl_print(pEmr->Mask);            printf("\n");
   printf("   iUsageMask:     %u\n",  pEmr->iUsageMask   );
   printf("   offBmiMask:     %u\n",  pEmr->offBmiMask   );
   printf("   cbBmiMask:      %u\n",  pEmr->cbBmiMask    );
   if(pEmr->cbBmiMask){
      printf("      Mask bitmap: ");
      bitmapinfo_print(contents + pEmr->offBmiMask);
      printf("\n");
   }
   printf("   offBitsMask:    %u\n",  pEmr->offBitsMask   );
   printf("   cbBitsMask:     %u\n",  pEmr->cbBitsMask    );
}

// U_EMRPLGBLT               79
/**
    \brief Print a pointer to a U_EMR_PLGBLT record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPLGBLT_print(const char *contents){
   PU_EMRPLGBLT pEmr = (PU_EMRPLGBLT) (contents);
   printf("   rclBounds:      ");     rectl_print( pEmr->rclBounds);       printf("\n");
   printf("   aptlDst(UL):    ");     pointl_print(pEmr->aptlDst[0]);      printf("\n");
   printf("   aptlDst(UR):    ");     pointl_print(pEmr->aptlDst[1]);      printf("\n");
   printf("   aptlDst(LL):    ");     pointl_print(pEmr->aptlDst[2]);      printf("\n");
   printf("   Src:            ");     pointl_print(pEmr->Src);             printf("\n");
   printf("   cSrc:           ");     pointl_print(pEmr->cSrc);            printf("\n");
   printf("   xformSrc:       ");     xform_print( pEmr->xformSrc);        printf("\n");
   printf("   crBkColorSrc:   ");     colorref_print( pEmr->crBkColorSrc); printf("\n");
   printf("   iUsageSrc:      %u\n", pEmr->iUsageSrc   );
   printf("   offBmiSrc:      %u\n", pEmr->offBmiSrc   );
   printf("   cbBmiSrc:       %u\n", pEmr->cbBmiSrc    );
   if(pEmr->cbBmiSrc){
      printf("      Src bitmap:  ");
      bitmapinfo_print(contents + pEmr->offBmiSrc);
      printf("\n");
   }
   printf("   offBitsSrc:     %u\n", pEmr->offBitsSrc   );
   printf("   cbBitsSrc:      %u\n", pEmr->cbBitsSrc    );
   printf("   Mask:           ");    pointl_print(pEmr->Mask);            printf("\n");
   printf("   iUsageMsk:      %u\n", pEmr->iUsageMask   );
   printf("   offBmiMask:     %u\n", pEmr->offBmiMask   );
   printf("   cbBmiMask:      %u\n", pEmr->cbBmiMask    );
   if(pEmr->cbBmiMask){
      printf("      Mask bitmap: ");
      bitmapinfo_print(contents + pEmr->offBmiMask);
      printf("\n");
   }
   printf("   offBitsMask:    %u\n", pEmr->offBitsMask   );
   printf("   cbBitsMask:     %u\n", pEmr->cbBitsMask    );
}

// U_EMRSETDIBITSTODEVICE    80
/**
    \brief Print a pointer to a U_EMRSETDIBITSTODEVICE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETDIBITSTODEVICE_print(const char *contents){
   PU_EMRSETDIBITSTODEVICE pEmr = (PU_EMRSETDIBITSTODEVICE) (contents);
   printf("   rclBounds:      ");     rectl_print( pEmr->rclBounds);       printf("\n");
   printf("   Dest:           ");     pointl_print(pEmr->Dest);            printf("\n");
   printf("   Src:            ");     pointl_print(pEmr->Src);             printf("\n");
   printf("   cSrc:           ");     pointl_print(pEmr->cSrc);            printf("\n");
   printf("   offBmiSrc:      %u\n", pEmr->offBmiSrc   );
   printf("   cbBmiSrc:       %u\n", pEmr->cbBmiSrc    );
   if(pEmr->cbBmiSrc){
      printf("      Src bitmap:  ");
      bitmapinfo_print(contents + pEmr->offBmiSrc);
      printf("\n");
   }
   printf("   offBitsSrc:     %u\n", pEmr->offBitsSrc   );
   printf("   cbBitsSrc:      %u\n", pEmr->cbBitsSrc    );
   printf("   iUsageSrc:      %u\n", pEmr->iUsageSrc   );
   printf("   iStartScan:     %u\n", pEmr->iStartScan    );
   printf("   cScans :        %u\n", pEmr->cScans        );
}

// U_EMRSTRETCHDIBITS        81
/**
    \brief Print a pointer to a U_EMR_STRETCHDIBITS record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSTRETCHDIBITS_print(const char *contents){
   PU_EMRSTRETCHDIBITS pEmr = (PU_EMRSTRETCHDIBITS) (contents);
   printf("   rclBounds:      ");     rectl_print( pEmr->rclBounds);       printf("\n");
   printf("   Dest:           ");     pointl_print(pEmr->Dest);            printf("\n");
   printf("   Src:            ");     pointl_print(pEmr->Src);             printf("\n");
   printf("   cSrc:           ");     pointl_print(pEmr->cSrc);            printf("\n");
   printf("   offBmiSrc:      %u\n", pEmr->offBmiSrc   );
   printf("   cbBmiSrc:       %u\n", pEmr->cbBmiSrc    );
   if(pEmr->cbBmiSrc){
      printf("      Src bitmap:  ");
      bitmapinfo_print(contents + pEmr->offBmiSrc);
      printf("\n");
   }
   printf("   offBitsSrc:     %u\n", pEmr->offBitsSrc   );
   printf("   cbBitsSrc:      %u\n", pEmr->cbBitsSrc    );
   printf("   iUsageSrc:      %u\n", pEmr->iUsageSrc   );
   printf("   dwRop :         0x%8.8X\n", pEmr->dwRop   );
   printf("   cDest:          ");     pointl_print(pEmr->cDest);           printf("\n");
}

// U_EMREXTCREATEFONTINDIRECTW_print    82
/**
    \brief Print a pointer to a U_EMR_EXTCREATEFONTINDIRECTW record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMREXTCREATEFONTINDIRECTW_print(const char *contents){
   PU_EMREXTCREATEFONTINDIRECTW pEmr = (PU_EMREXTCREATEFONTINDIRECTW) (contents);
   printf("   ihFont:         %u\n",pEmr->ihFont );
   printf("   Font:           ");
   if(pEmr->emr.nSize == sizeof(U_EMREXTCREATEFONTINDIRECTW)){ // holds logfont_panose
      logfont_panose_print(pEmr->elfw);
   }
   else { // holds logfont
      logfont_print( *(PU_LOGFONT) &(pEmr->elfw));
   }
   printf("\n");
}

// U_EMREXTTEXTOUTA          83
/**
    \brief Print a pointer to a U_EMR_EXTTEXTOUTA record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMREXTTEXTOUTA_print(const char *contents){
   core8_print("U_EMREXTTEXTOUTA", contents, 0);
}

// U_EMREXTTEXTOUTW          84
/**
    \brief Print a pointer to a U_EMR_EXTTEXTOUTW record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMREXTTEXTOUTW_print(const char *contents){
   core8_print("U_EMREXTTEXTOUTW", contents, 1);
}

// U_EMRPOLYBEZIER16         85
/**
    \brief Print a pointer to a U_EMR_POLYBEZIER16 record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPOLYBEZIER16_print(const char *contents){
   core6_print("U_EMRPOLYBEZIER16", contents);
}

// U_EMRPOLYGON16            86
/**
    \brief Print a pointer to a U_EMR_POLYGON16 record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPOLYGON16_print(const char *contents){
   core6_print("U_EMRPOLYGON16", contents);
}

// U_EMRPOLYLINE16           87
/**
    \brief Print a pointer to a U_EMR_POLYLINE16 record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPOLYLINE16_print(const char *contents){
   core6_print("U_EMRPOLYLINE16", contents);
}

// U_EMRPOLYBEZIERTO16       88
/**
    \brief Print a pointer to a U_EMR_POLYBEZIERTO16 record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPOLYBEZIERTO16_print(const char *contents){
   core6_print("U_EMRPOLYBEZIERTO16", contents);
}

// U_EMRPOLYLINETO16         89
/**
    \brief Print a pointer to a U_EMR_POLYLINETO16 record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPOLYLINETO16_print(const char *contents){
   core6_print("U_EMRPOLYLINETO16", contents);
}

// U_EMRPOLYPOLYLINE16       90
/**
    \brief Print a pointer to a U_EMR_POLYPOLYLINE16 record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPOLYPOLYLINE16_print(const char *contents){
   core10_print("U_EMRPOLYPOLYLINE16", contents);
}

// U_EMRPOLYPOLYGON16        91
/**
    \brief Print a pointer to a U_EMR_POLYPOLYGON16 record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPOLYPOLYGON16_print(const char *contents){
   core10_print("U_EMRPOLYPOLYGON16", contents);
}


// U_EMRPOLYDRAW16           92
/**
    \brief Print a pointer to a U_EMR_POLYDRAW16 record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPOLYDRAW16_print(const char *contents){
   unsigned int i;
   PU_EMRPOLYDRAW16 pEmr = (PU_EMRPOLYDRAW16)(contents);
   printf("   rclBounds:      ");          rectl_print( pEmr->rclBounds);   printf("\n");
   printf("   cpts:           %d\n",pEmr->cpts        );
   printf("   Points:         ");
   for(i=0;i<pEmr->cpts; i++){
      printf(" [%d]:",i);
      point16_print(pEmr->apts[i]);
   }
   printf("\n");
   printf("   Types:          ");
   for(i=0;i<pEmr->cpts; i++){
      printf(" [%d]:%u ",i,pEmr->abTypes[i]);
   }
   printf("\n");
}

// U_EMRCREATEMONOBRUSH      93
/**
    \brief Print a pointer to a U_EMR_CREATEMONOBRUSH record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRCREATEMONOBRUSH_print(const char *contents){
   core12_print("U_EMRCREATEMONOBRUSH", contents);
}

// U_EMRCREATEDIBPATTERNBRUSHPT_print   94
/**
    \brief Print a pointer to a U_EMR_CREATEDIBPATTERNBRUSHPT record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRCREATEDIBPATTERNBRUSHPT_print(const char *contents){
   core12_print("U_EMRCREATEDIBPATTERNBRUSHPT", contents);
}


// U_EMREXTCREATEPEN         95
/**
    \brief Print a pointer to a U_EMR_EXTCREATEPEN record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMREXTCREATEPEN_print(const char *contents){
   PU_EMREXTCREATEPEN pEmr = (PU_EMREXTCREATEPEN)(contents);
   printf("   ihPen:          %u\n", pEmr->ihPen );
   printf("   offBmi:         %u\n", pEmr->offBmi   );
   printf("   cbBmi:          %u\n", pEmr->cbBmi    );
   if(pEmr->cbBmi){
      printf("      bitmap:      ");
      bitmapinfo_print(contents + pEmr->offBmi);
      printf("\n");
   }
   printf("   offBits:        %u\n", pEmr->offBits   );
   printf("   cbBits:         %u\n", pEmr->cbBits    );
   printf("   elp:            ");     extlogpen_print((PU_EXTLOGPEN) &(pEmr->elp));  printf("\n");
} 

// U_EMRPOLYTEXTOUTA         96 NOT IMPLEMENTED, denigrated after Windows NT
#define U_EMRPOLYTEXTOUTA_print(A) U_EMRNOTIMPLEMENTED_print("U_EMRPOLYTEXTOUTA",A) //!< Not implemented.
// U_EMRPOLYTEXTOUTW         97 NOT IMPLEMENTED, denigrated after Windows NT
#define U_EMRPOLYTEXTOUTW_print(A) U_EMRNOTIMPLEMENTED_print("U_EMRPOLYTEXTOUTW",A) //!< Not implemented.

// U_EMRSETICMMODE           98
/**
    \brief Print a pointer to a U_EMR_SETICMMODE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETICMMODE_print(const char *contents){
   core3_print("U_EMRSETICMMODE", "iMode:", contents);
}

// U_EMRCREATECOLORSPACE     99
/**
    \brief Print a pointer to a U_EMR_CREATECOLORSPACE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRCREATECOLORSPACE_print(const char *contents){
   PU_EMRCREATECOLORSPACE pEmr = (PU_EMRCREATECOLORSPACE)(contents);
   printf("   ihCS:           %u\n", pEmr->ihCS    );
   printf("   ColorSpace:     "); logcolorspacea_print(pEmr->lcs);  printf("\n");
}

// U_EMRSETCOLORSPACE       100
/**
    \brief Print a pointer to a U_EMR_SETCOLORSPACE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETCOLORSPACE_print(const char *contents){
   core3_print("U_EMRSETCOLORSPACE", "ihCS:", contents);
}

// U_EMRDELETECOLORSPACE    101
/**
    \brief Print a pointer to a U_EMR_DELETECOLORSPACE record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRDELETECOLORSPACE_print(const char *contents){
   core3_print("U_EMRDELETECOLORSPACE", "ihCS:", contents);
}

// U_EMRGLSRECORD           102  Not implemented
#define U_EMRGLSRECORD_print(A) U_EMRNOTIMPLEMENTED_print("U_EMRGLSRECORD",A) //!< Not implemented.
// U_EMRGLSBOUNDEDRECORD    103  Not implemented
#define U_EMRGLSBOUNDEDRECORD_print(A) U_EMRNOTIMPLEMENTED_print("U_EMRGLSBOUNDEDRECORD",A) //!< Not implemented.

// U_EMRPIXELFORMAT         104
/**
    \brief Print a pointer to a U_EMR_PIXELFORMAT record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRPIXELFORMAT_print(const char *contents){
   PU_EMRPIXELFORMAT pEmr = (PU_EMRPIXELFORMAT)(contents);
   printf("   Pfd:            ");  pixelformatdescriptor_print(pEmr->pfd);  printf("\n");
}

// U_EMRDRAWESCAPE          105  Not implemented
#define U_EMRDRAWESCAPE_print(A) U_EMRNOTIMPLEMENTED_print("U_EMRDRAWESCAPE",A) //!< Not implemented.
// U_EMREXTESCAPE           106  Not implemented
#define U_EMREXTESCAPE_print(A) U_EMRNOTIMPLEMENTED_print("U_EMREXTESCAPE",A) //!< Not implemented.
// U_EMRUNDEF107            107  Not implemented
#define U_EMRUNDEF107_print(A) U_EMRNOTIMPLEMENTED_print("U_EMRUNDEF107",A) //!< Not implemented.

// U_EMRSMALLTEXTOUT        108
/**
    \brief Print a pointer to a U_EMR_SMALLTEXTOUT record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSMALLTEXTOUT_print(const char *contents){
   int roff;
   char *string;
   PU_EMRSMALLTEXTOUT pEmr = (PU_EMRSMALLTEXTOUT)(contents);
   printf("   Dest:           ");         pointl_print(pEmr->Dest);            printf("\n");
   printf("   cChars:         %u\n",      pEmr->cChars          );
   printf("   fuOptions:      0x%8.8X\n", pEmr->fuOptions       );
   printf("   iGraphicsMode:  0x%8.8X\n", pEmr->iGraphicsMode   );
   printf("   exScale:        %f\n",      pEmr->exScale         );
   printf("   eyScale:        %f\n",      pEmr->eyScale         );
   roff = sizeof(U_EMRSMALLTEXTOUT);  //offset to the start of the variable fields
   if(!(pEmr->fuOptions & U_ETO_NO_RECT)){
      printf("   rclBounds:      ");      rectl_print( *(PU_RECTL) (contents + roff));       printf("\n");
      roff += sizeof(U_RECTL);
   }
   if(pEmr->fuOptions & U_ETO_SMALL_CHARS){
      printf("   Text8:          <%.*s>\n",pEmr->cChars,contents+roff);  /* May not be null terminated */
   }
   else {
      string = U_Utf16leToUtf8((uint16_t *)(contents+roff), pEmr->cChars, NULL);
      printf("   Text16:         <%s>\n",contents+roff);
      free(string);
  }
}

// U_EMRFORCEUFIMAPPING     109  Not implemented
#define U_EMRFORCEUFIMAPPING_print(A)     U_EMRNOTIMPLEMENTED_print("U_EMRFORCEUFIMAPPING",A) //!< Not implemented.
// U_EMRNAMEDESCAPE         110  Not implemented
#define U_EMRNAMEDESCAPE_print(A)         U_EMRNOTIMPLEMENTED_print("U_EMRNAMEDESCAPE",A) //!< Not implemented.
// U_EMRCOLORCORRECTPALETTE 111  Not implemented
#define U_EMRCOLORCORRECTPALETTE_print(A) U_EMRNOTIMPLEMENTED_print("U_EMRCOLORCORRECTPALETTE",A) //!< Not implemented.
// U_EMRSETICMPROFILEA      112  Not implemented
#define U_EMRSETICMPROFILEA_print(A)      U_EMRNOTIMPLEMENTED_print("U_EMRSETICMPROFILEA",A) //!< Not implemented.
// U_EMRSETICMPROFILEW      113  Not implemented
#define U_EMRSETICMPROFILEW_print(A)      U_EMRNOTIMPLEMENTED_print("U_EMRSETICMPROFILEW",A) //!< Not implemented.

// U_EMRALPHABLEND          114
/**
    \brief Print a pointer to a U_EMR_ALPHABLEND record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRALPHABLEND_print(const char *contents){
   core13_print("U_EMRALPHABLEND", contents);
}

// U_EMRSETLAYOUT           115
/**
    \brief Print a pointer to a U_EMR_SETLAYOUT record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRSETLAYOUT_print(const char *contents){
   core3_print("U_EMRSETLAYOUT", "iMode:", contents);
}

// U_EMRTRANSPARENTBLT      116
/**
    \brief Print a pointer to a U_EMR_TRANSPARENTBLT record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRTRANSPARENTBLT_print(const char *contents){
   core13_print("U_EMRTRANSPARENTBLT", contents);
}

// U_EMRUNDEF117            117  Not implemented
#define U_EMRUNDEF117_print(A)    U_EMRNOTIMPLEMENTED_print("U_EMRUNDEF117",A) //!< Not implemented.
// U_EMRGRADIENTFILL        118
/**
    \brief Print a pointer to a U_EMR_GRADIENTFILL record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRGRADIENTFILL_print(const char *contents){
   unsigned int i;
   PU_EMRGRADIENTFILL pEmr = (PU_EMRGRADIENTFILL)(contents);
   printf("   rclBounds:      ");      rectl_print( pEmr->rclBounds);   printf("\n");
   printf("   nTriVert:       %u\n",   pEmr->nTriVert   );
   printf("   nGradObj:       %u\n",   pEmr->nGradObj   );
   printf("   ulMode:         %u\n",   pEmr->ulMode     );
   contents += sizeof(U_EMRGRADIENTFILL);
   if(pEmr->nTriVert){
      printf("   TriVert:        ");
      for(i=0; i<pEmr->nTriVert; i++, contents+=sizeof(U_TRIVERTEX)){
         trivertex_print(*(PU_TRIVERTEX)(contents));
      }
      printf("\n");
   }
   if(pEmr->nGradObj){
      printf("   GradObj:        ");
      if(     pEmr->ulMode == U_GRADIENT_FILL_TRIANGLE){
         for(i=0; i<pEmr->nGradObj; i++, contents+=sizeof(U_GRADIENT3)){
            gradient3_print(*(PU_GRADIENT3)(contents));
         }
      }
      else if(pEmr->ulMode == U_GRADIENT_FILL_RECT_H || 
              pEmr->ulMode == U_GRADIENT_FILL_RECT_V){
         for(i=0; i<pEmr->nGradObj; i++, contents+=sizeof(U_GRADIENT4)){
            gradient4_print(*(PU_GRADIENT4)(contents));
         }
      }
      else { printf("invalid ulMode value!"); }
      printf("\n");
   }
}

// U_EMRSETLINKEDUFIS       119  Not implemented
#define U_EMRSETLINKEDUFIS_print(A)        U_EMRNOTIMPLEMENTED_print("U_EMR_SETLINKEDUFIS",A) //!< Not implemented.
// U_EMRSETTEXTJUSTIFICATION120  Not implemented (denigrated)
#define U_EMRSETTEXTJUSTIFICATION_print(A) U_EMRNOTIMPLEMENTED_print("U_EMR_SETTEXTJUSTIFICATION",A) //!< Not implemented.
// U_EMRCOLORMATCHTOTARGETW 121  Not implemented  
#define U_EMRCOLORMATCHTOTARGETW_print(A)  U_EMRNOTIMPLEMENTED_print("U_EMR_COLORMATCHTOTARGETW",A) //!< Not implemented.

// U_EMRCREATECOLORSPACEW   122
/**
    \brief Print a pointer to a U_EMR_CREATECOLORSPACEW record.
    \param contents   pointer to a buffer holding all EMR records
*/
void U_EMRCREATECOLORSPACEW_print(const char *contents){
   unsigned int i;
   PU_EMRCREATECOLORSPACEW pEmr = (PU_EMRCREATECOLORSPACEW)(contents);
   printf("   ihCS:           %u\n", pEmr->ihCS     );
   printf("   ColorSpace:     "); logcolorspacew_print(pEmr->lcs);  printf("\n");
   printf("   dwFlags:        0x%8.8X\n", pEmr->dwFlags  );
   printf("   cbData:         %u\n", pEmr->cbData   );
   printf("   Data(hexvalues):");
   if(pEmr->dwFlags & 1){
     for(i=0; i<pEmr->cbData; i++){
        printf("[%d]:%2.2X ",i,pEmr->Data[i]);
     }
   }
   printf("\n");
}

/**
    \brief Print any record in an emf
    \returns record length for a normal record, 0 for EMREOF, -1 for a bad record
    \param contents   pointer to a buffer holding all EMR records
    \param blimit     pointer to the byte after the last byte in the buffer holding all EMR records
    \param recnum     number of this record in contents
    \param off        offset to this record in contents
*/
int U_emf_onerec_print(const char *contents, const char *blimit, int recnum, size_t off){
    PU_ENHMETARECORD  lpEMFR  = (PU_ENHMETARECORD)(contents + off);
    unsigned int size;

    printf("%-30srecord:%5d type:%-4d offset:%8d rsize:%8d\n",U_emr_names(lpEMFR->iType),recnum,lpEMFR->iType,(int) off,lpEMFR->nSize);
    size      = lpEMFR->nSize;
    contents += off;
    
    /* Check that the record size is OK, abort if not.
       Pointer math might wrap, so check both sides of the range */
    if(size < sizeof(U_EMR)           ||
       contents + size - 1 >= blimit  || 
       contents + size - 1 < contents)return(-1);

    switch (lpEMFR->iType)
    {
        case U_EMR_HEADER:                  U_EMRHEADER_print(contents);                  break;
        case U_EMR_POLYBEZIER:              U_EMRPOLYBEZIER_print(contents);              break;
        case U_EMR_POLYGON:                 U_EMRPOLYGON_print(contents);                 break;
        case U_EMR_POLYLINE:                U_EMRPOLYLINE_print(contents);                break;
        case U_EMR_POLYBEZIERTO:            U_EMRPOLYBEZIERTO_print(contents);            break;
        case U_EMR_POLYLINETO:              U_EMRPOLYLINETO_print(contents);              break;
        case U_EMR_POLYPOLYLINE:            U_EMRPOLYPOLYLINE_print(contents);            break;
        case U_EMR_POLYPOLYGON:             U_EMRPOLYPOLYGON_print(contents);             break;
        case U_EMR_SETWINDOWEXTEX:          U_EMRSETWINDOWEXTEX_print(contents);          break;
        case U_EMR_SETWINDOWORGEX:          U_EMRSETWINDOWORGEX_print(contents);          break;
        case U_EMR_SETVIEWPORTEXTEX:        U_EMRSETVIEWPORTEXTEX_print(contents);        break;
        case U_EMR_SETVIEWPORTORGEX:        U_EMRSETVIEWPORTORGEX_print(contents);        break;
        case U_EMR_SETBRUSHORGEX:           U_EMRSETBRUSHORGEX_print(contents);           break;
        case U_EMR_EOF:                     U_EMREOF_print(contents);          size=0;    break;
        case U_EMR_SETPIXELV:               U_EMRSETPIXELV_print(contents);               break;
        case U_EMR_SETMAPPERFLAGS:          U_EMRSETMAPPERFLAGS_print(contents);          break;
        case U_EMR_SETMAPMODE:              U_EMRSETMAPMODE_print(contents);              break;
        case U_EMR_SETBKMODE:               U_EMRSETBKMODE_print(contents);               break;
        case U_EMR_SETPOLYFILLMODE:         U_EMRSETPOLYFILLMODE_print(contents);         break;
        case U_EMR_SETROP2:                 U_EMRSETROP2_print(contents);                 break;
        case U_EMR_SETSTRETCHBLTMODE:       U_EMRSETSTRETCHBLTMODE_print(contents);       break;
        case U_EMR_SETTEXTALIGN:            U_EMRSETTEXTALIGN_print(contents);            break;
        case U_EMR_SETCOLORADJUSTMENT:      U_EMRSETCOLORADJUSTMENT_print(contents);      break;
        case U_EMR_SETTEXTCOLOR:            U_EMRSETTEXTCOLOR_print(contents);            break;
        case U_EMR_SETBKCOLOR:              U_EMRSETBKCOLOR_print(contents);              break;
        case U_EMR_OFFSETCLIPRGN:           U_EMROFFSETCLIPRGN_print(contents);           break;
        case U_EMR_MOVETOEX:                U_EMRMOVETOEX_print(contents);                break;
        case U_EMR_SETMETARGN:              U_EMRSETMETARGN_print(contents);              break;
        case U_EMR_EXCLUDECLIPRECT:         U_EMREXCLUDECLIPRECT_print(contents);         break;
        case U_EMR_INTERSECTCLIPRECT:       U_EMRINTERSECTCLIPRECT_print(contents);       break;
        case U_EMR_SCALEVIEWPORTEXTEX:      U_EMRSCALEVIEWPORTEXTEX_print(contents);      break;
        case U_EMR_SCALEWINDOWEXTEX:        U_EMRSCALEWINDOWEXTEX_print(contents);        break;
        case U_EMR_SAVEDC:                  U_EMRSAVEDC_print(contents);                  break;
        case U_EMR_RESTOREDC:               U_EMRRESTOREDC_print(contents);               break;
        case U_EMR_SETWORLDTRANSFORM:       U_EMRSETWORLDTRANSFORM_print(contents);       break;
        case U_EMR_MODIFYWORLDTRANSFORM:    U_EMRMODIFYWORLDTRANSFORM_print(contents);    break;
        case U_EMR_SELECTOBJECT:            U_EMRSELECTOBJECT_print(contents);            break;
        case U_EMR_CREATEPEN:               U_EMRCREATEPEN_print(contents);               break;
        case U_EMR_CREATEBRUSHINDIRECT:     U_EMRCREATEBRUSHINDIRECT_print(contents);     break;
        case U_EMR_DELETEOBJECT:            U_EMRDELETEOBJECT_print(contents);            break;
        case U_EMR_ANGLEARC:                U_EMRANGLEARC_print(contents);                break;
        case U_EMR_ELLIPSE:                 U_EMRELLIPSE_print(contents);                 break;
        case U_EMR_RECTANGLE:               U_EMRRECTANGLE_print(contents);               break;
        case U_EMR_ROUNDRECT:               U_EMRROUNDRECT_print(contents);               break;
        case U_EMR_ARC:                     U_EMRARC_print(contents);                     break;
        case U_EMR_CHORD:                   U_EMRCHORD_print(contents);                   break;
        case U_EMR_PIE:                     U_EMRPIE_print(contents);                     break;
        case U_EMR_SELECTPALETTE:           U_EMRSELECTPALETTE_print(contents);           break;
        case U_EMR_CREATEPALETTE:           U_EMRCREATEPALETTE_print(contents);           break;
        case U_EMR_SETPALETTEENTRIES:       U_EMRSETPALETTEENTRIES_print(contents);       break;
        case U_EMR_RESIZEPALETTE:           U_EMRRESIZEPALETTE_print(contents);           break;
        case U_EMR_REALIZEPALETTE:          U_EMRREALIZEPALETTE_print(contents);          break;
        case U_EMR_EXTFLOODFILL:            U_EMREXTFLOODFILL_print(contents);            break;
        case U_EMR_LINETO:                  U_EMRLINETO_print(contents);                  break;
        case U_EMR_ARCTO:                   U_EMRARCTO_print(contents);                   break;
        case U_EMR_POLYDRAW:                U_EMRPOLYDRAW_print(contents);                break;
        case U_EMR_SETARCDIRECTION:         U_EMRSETARCDIRECTION_print(contents);         break;
        case U_EMR_SETMITERLIMIT:           U_EMRSETMITERLIMIT_print(contents);           break;
        case U_EMR_BEGINPATH:               U_EMRBEGINPATH_print(contents);               break;
        case U_EMR_ENDPATH:                 U_EMRENDPATH_print(contents);                 break;
        case U_EMR_CLOSEFIGURE:             U_EMRCLOSEFIGURE_print(contents);             break;
        case U_EMR_FILLPATH:                U_EMRFILLPATH_print(contents);                break;
        case U_EMR_STROKEANDFILLPATH:       U_EMRSTROKEANDFILLPATH_print(contents);       break;
        case U_EMR_STROKEPATH:              U_EMRSTROKEPATH_print(contents);              break;
        case U_EMR_FLATTENPATH:             U_EMRFLATTENPATH_print(contents);             break;
        case U_EMR_WIDENPATH:               U_EMRWIDENPATH_print(contents);               break;
        case U_EMR_SELECTCLIPPATH:          U_EMRSELECTCLIPPATH_print(contents);          break;
        case U_EMR_ABORTPATH:               U_EMRABORTPATH_print(contents);               break;
        case U_EMR_UNDEF69:                 U_EMRUNDEF69_print(contents);                 break;
        case U_EMR_COMMENT:                 U_EMRCOMMENT_print(contents, blimit, off);    break;
        case U_EMR_FILLRGN:                 U_EMRFILLRGN_print(contents);                 break;
        case U_EMR_FRAMERGN:                U_EMRFRAMERGN_print(contents);                break;
        case U_EMR_INVERTRGN:               U_EMRINVERTRGN_print(contents);               break;
        case U_EMR_PAINTRGN:                U_EMRPAINTRGN_print(contents);                break;
        case U_EMR_EXTSELECTCLIPRGN:        U_EMREXTSELECTCLIPRGN_print(contents);        break;
        case U_EMR_BITBLT:                  U_EMRBITBLT_print(contents);                  break;
        case U_EMR_STRETCHBLT:              U_EMRSTRETCHBLT_print(contents);              break;
        case U_EMR_MASKBLT:                 U_EMRMASKBLT_print(contents);                 break;
        case U_EMR_PLGBLT:                  U_EMRPLGBLT_print(contents);                  break;
        case U_EMR_SETDIBITSTODEVICE:       U_EMRSETDIBITSTODEVICE_print(contents);       break;
        case U_EMR_STRETCHDIBITS:           U_EMRSTRETCHDIBITS_print(contents);           break;
        case U_EMR_EXTCREATEFONTINDIRECTW:  U_EMREXTCREATEFONTINDIRECTW_print(contents);  break;
        case U_EMR_EXTTEXTOUTA:             U_EMREXTTEXTOUTA_print(contents);             break;
        case U_EMR_EXTTEXTOUTW:             U_EMREXTTEXTOUTW_print(contents);             break;
        case U_EMR_POLYBEZIER16:            U_EMRPOLYBEZIER16_print(contents);            break;
        case U_EMR_POLYGON16:               U_EMRPOLYGON16_print(contents);               break;
        case U_EMR_POLYLINE16:              U_EMRPOLYLINE16_print(contents);              break;
        case U_EMR_POLYBEZIERTO16:          U_EMRPOLYBEZIERTO16_print(contents);          break;
        case U_EMR_POLYLINETO16:            U_EMRPOLYLINETO16_print(contents);            break;
        case U_EMR_POLYPOLYLINE16:          U_EMRPOLYPOLYLINE16_print(contents);          break;
        case U_EMR_POLYPOLYGON16:           U_EMRPOLYPOLYGON16_print(contents);           break;
        case U_EMR_POLYDRAW16:              U_EMRPOLYDRAW16_print(contents);              break;
        case U_EMR_CREATEMONOBRUSH:         U_EMRCREATEMONOBRUSH_print(contents);         break;
        case U_EMR_CREATEDIBPATTERNBRUSHPT: U_EMRCREATEDIBPATTERNBRUSHPT_print(contents); break;
        case U_EMR_EXTCREATEPEN:            U_EMREXTCREATEPEN_print(contents);            break;
        case U_EMR_POLYTEXTOUTA:            U_EMRPOLYTEXTOUTA_print(contents);            break;
        case U_EMR_POLYTEXTOUTW:            U_EMRPOLYTEXTOUTW_print(contents);            break;
        case U_EMR_SETICMMODE:              U_EMRSETICMMODE_print(contents);              break;
        case U_EMR_CREATECOLORSPACE:        U_EMRCREATECOLORSPACE_print(contents);        break;
        case U_EMR_SETCOLORSPACE:           U_EMRSETCOLORSPACE_print(contents);           break;
        case U_EMR_DELETECOLORSPACE:        U_EMRDELETECOLORSPACE_print(contents);        break;
        case U_EMR_GLSRECORD:               U_EMRGLSRECORD_print(contents);               break;
        case U_EMR_GLSBOUNDEDRECORD:        U_EMRGLSBOUNDEDRECORD_print(contents);        break;
        case U_EMR_PIXELFORMAT:             U_EMRPIXELFORMAT_print(contents);             break;
        case U_EMR_DRAWESCAPE:              U_EMRDRAWESCAPE_print(contents);              break;
        case U_EMR_EXTESCAPE:               U_EMREXTESCAPE_print(contents);               break;
        case U_EMR_UNDEF107:                U_EMRUNDEF107_print(contents);                break;
        case U_EMR_SMALLTEXTOUT:            U_EMRSMALLTEXTOUT_print(contents);            break;
        case U_EMR_FORCEUFIMAPPING:         U_EMRFORCEUFIMAPPING_print(contents);         break;
        case U_EMR_NAMEDESCAPE:             U_EMRNAMEDESCAPE_print(contents);             break;
        case U_EMR_COLORCORRECTPALETTE:     U_EMRCOLORCORRECTPALETTE_print(contents);     break;
        case U_EMR_SETICMPROFILEA:          U_EMRSETICMPROFILEA_print(contents);          break;
        case U_EMR_SETICMPROFILEW:          U_EMRSETICMPROFILEW_print(contents);          break;
        case U_EMR_ALPHABLEND:              U_EMRALPHABLEND_print(contents);              break;
        case U_EMR_SETLAYOUT:               U_EMRSETLAYOUT_print(contents);               break;
        case U_EMR_TRANSPARENTBLT:          U_EMRTRANSPARENTBLT_print(contents);          break;
        case U_EMR_UNDEF117:                U_EMRUNDEF117_print(contents);                break;
        case U_EMR_GRADIENTFILL:            U_EMRGRADIENTFILL_print(contents);            break;
        case U_EMR_SETLINKEDUFIS:           U_EMRSETLINKEDUFIS_print(contents);           break;
        case U_EMR_SETTEXTJUSTIFICATION:    U_EMRSETTEXTJUSTIFICATION_print(contents);    break;
        case U_EMR_COLORMATCHTOTARGETW:     U_EMRCOLORMATCHTOTARGETW_print(contents);     break;
        case U_EMR_CREATECOLORSPACEW:       U_EMRCREATECOLORSPACEW_print(contents);       break;
        default:                            U_EMRNOTIMPLEMENTED_print("?",contents);      break;
    }  //end of switch
    return(size);
}


#ifdef __cplusplus
}
#endif
