/**
  @file uemf_safe.c
  
  @brief Functions for checking EMF records for memory issues.
   
  EMF records come in a variety of sizes, and some types have variable sizes.
  These functions check the record types and report if there are any issues
    that could cause a memory access problem.  All counts and offsets are examined
    and the data structure checked so that no referenced byte is outside of the
    declared size of the record.

  Many variables are initialized to zero even though they will always be set because
  some versions of gcc give spurious "may be used uninitialized" warnings otherwise.
*/

/*
File:      uemf_safe.c
Version:   0.0.5
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
#include <stddef.h> /* for offsetof() macro */
#include "uemf.h"
#include "uemf_endian.h" // for u_emf_record_sizeok

// hide almost everuything in here from Doxygen
//! \cond

/**
    \brief Test a U_EXTLOGPEN object.
    \param elp    PU_EXTLOGPEN object
    \param blimit one byte past the end of the record
*/
int extlogpen_safe(
      PU_EXTLOGPEN elp,
      const char *blimit
   ){
   int count=elp->elpNumEntries;
   if(IS_MEM_UNSAFE(&(elp->elpStyleEntry), count*4, blimit))return(0);
   return(1);
}

/**
    \brief Test a U_EMRTEXT record
    \param pemt      Pointer to a U_EMRTEXT record 
    \param record    Pointer to the start of the record which contains this U_EMRTEXT
    \param blimit    one byte past the end of the record.
*/
int emrtext_safe(
      PU_EMRTEXT  pemt,
      const char *record,
      const char *blimit
   ){
   int        off;
   uint32_t   count    = pemt->nChars;
   uint32_t   fOptions = pemt->fOptions;
   uint32_t   offDx    = 0;
   off = sizeof(U_EMRTEXT);
   if(!(fOptions & U_ETO_NO_RECT)){
       if(IS_MEM_UNSAFE(pemt, sizeof(U_RECTL), blimit))return(0);
       off+=sizeof(U_RECTL);
   }
   offDx = *(uint32_t *)((char *)pemt +off);
   if(IS_MEM_UNSAFE(pemt, off + 4, blimit))return(0);
   if(IS_MEM_UNSAFE(record, offDx + count*4, blimit))return(0);
   return(1);
}

/**
    \return 1 on success, 0 on failure
    \brief Test a U_RGNDATA object.
    \param rd  pointer to a U_RGNDATA object.
    \param cbRgnData size of the U_RGNDATA object.
*/
int rgndata_safe(
      PU_RGNDATA rd,
      int cbRgnData
   ){
   int count = rd->rdh.nCount;
   if(4*count + (int)sizeof(U_RGNDATAHEADER) > cbRgnData)return(0);
   return(1);
}


/**
    \return 1 on success, 0 on failure
    \brief Test a U_BITMAPINFO object.
    \param Bmi  pointer to a U_BITMAPINFO object.
    \param blimit    one byte past the end of the record.
*/
int bitmapinfo_safe(
      const char *Bmi,
      const char *blimit
   ){
   int       ClrUsed;
   if(IS_MEM_UNSAFE(Bmi, offsetof(U_BITMAPINFO,bmiHeader) + sizeof(U_BITMAPINFOHEADER), blimit))return(0);
   ClrUsed = get_real_color_count(Bmi + offsetof(U_BITMAPINFO,bmiHeader));
   if(ClrUsed &&  IS_MEM_UNSAFE(Bmi, offsetof(U_BITMAPINFO,bmiColors) + ClrUsed*sizeof(U_RGBQUAD), blimit))return(0);
   return(1);
}

/**
    \brief Check that the bitmap in the specified DIB is compatible with the record size
    
    \return 1 on success, 0 on failure
    \param record     EMF record that contains a DIB pixel array
    \param iUsage     DIBcolors Enumeration
    \param offBmi     offset from the start of the record to the start of the bitmapinfo structure
    \param cbBmi      declared space for the bitmapinfo structure in the record
    \param offBits    offset from the start of the record to the start of the bitmap
    \param cbBits     declared space for the bitmap in the record (amount used may be less than this)
    \param blimit     one byte past the end of the record.

    This method can only test DIBs that hold Microsoft's various bitmap types.  PNG or JPG is just a bag
    of bytes, and there is no possible way to derive from the known width and height how big it should be.
*/
int DIB_safe(
       const char      *record,
       uint32_t         iUsage,
       uint32_t         offBmi,
       uint32_t         cbBmi,
       uint32_t         offBits,
       uint32_t         cbBits,
       const char      *blimit
   ){
   int  dibparams = U_BI_UNKNOWN;       // type of image not yet determined
   const char      *px      = NULL;     // DIB pixels
   const U_RGBQUAD *ct      = NULL;     // DIB color table
   int              bs;
   int              usedbytes;

   if(!cbBmi)return(1);  // No DIB in a record where it is optional
   if(IS_MEM_UNSAFE(record, offBmi + cbBmi, blimit))return(0);
   if(!bitmapinfo_safe(record + offBmi, blimit))return(0);  // checks the number of colors
   if(cbBits && IS_MEM_UNSAFE(record, offBits + cbBits, blimit))return(0);
   if(iUsage == U_DIB_RGB_COLORS){
       uint32_t width, height, colortype, numCt, invert; // these values will be set in get_DIB_params
       // next call returns pointers and values, but allocates no memory
       dibparams = get_DIB_params(record, offBits, offBmi, &px, (const U_RGBQUAD **) &ct,
           &numCt, &width, &height, &colortype, &invert);

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
   return(1);
}


/* **********************************************************************************************
These functions contain shared code used by various U_EMR*_safe functions.  These should NEVER be called
by end user code and to further that end prototypes are NOT provided and they are hidden from Doxygen.


   These all have this form:
   
   void core1_safe(const char *record){
   
   but some do not actually use torev.
   
   
   
*********************************************************************************************** */

// all core*_safe call this, U_EMRSETMARGN_safe and some others all it directly
// numbered as core5 to be consistent with uemf.c, but must appear before the others as there is no prototype
// sizeof(U_ENHMETARECORD) bytes in the record
int core5_safe(const char *record, int minSize){
   PU_EMR pEmr = (PU_EMR)(record);
   if((int) pEmr->nSize < minSize)return(0);
   return(1);
}

// Functions with the same form starting with U_EMRPOLYBEZIER_safe
int core1_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRPOLYLINETO))return(0);
   PU_EMRPOLYLINETO pEmr = (PU_EMRPOLYLINETO) (record);
   int count=pEmr->cptl;
   const char *blimit = record + pEmr->emr.nSize;
   if(IS_MEM_UNSAFE(pEmr->aptl, count*sizeof(U_POINTL), blimit))return(0);
   return(1);
}

// Functions with the same form starting with U_EMRPOLYPOLYLINE_safe
int core2_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRPOLYPOLYLINE))return(0);
   PU_EMRPOLYPOLYLINE pEmr = (PU_EMRPOLYPOLYLINE) (record);
   int count  = pEmr->cptl;
   int nPolys = pEmr->nPolys;
   const char * blimit = record + pEmr->emr.nSize;
   if(IS_MEM_UNSAFE(pEmr->aPolyCounts, nPolys*4, blimit))return(0);
   record += sizeof(U_EMRPOLYPOLYLINE) - 4 + sizeof(uint32_t)* nPolys;
   if(IS_MEM_UNSAFE(record, count*sizeof(U_POINTL), blimit))return(0);
   return(1);
}


// Functions with the same form starting with U_EMRSETMAPMODE_safe
int core3_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRSETMAPMODE))return(0);
   return(1);
} 

// Functions taking a single U_RECT or U_RECTL, starting with U_EMRELLIPSE_safe, also U_EMRFILLPATH_safe, 
int core4_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRELLIPSE))return(0);
   return(1);
} 

// Functions with the same form starting with U_EMRPOLYBEZIER16_safe
int core6_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRPOLYBEZIER16))return(0);
   PU_EMRPOLYBEZIER16 pEmr = (PU_EMRPOLYBEZIER16) (record);
   int count=pEmr->cpts;
   const char *blimit = record + pEmr->emr.nSize;
   if(IS_MEM_UNSAFE(pEmr->apts, count*sizeof(U_POINT16), blimit))return(0);
   return(1);
} 


// Records with the same form starting with U_EMRSETWINDOWEXTEX_safe, that is, all with two uint32_t values after the emr
int core7_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRSETWINDOWEXTEX))return(0);
   return(1);
}

// For U_EMREXTTEXTOUTA and U_EMREXTTEXTOUTW, type=0 for the first one
int core8_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMREXTTEXTOUTA))return(0);
   PU_EMREXTTEXTOUTA pEmr = (PU_EMREXTTEXTOUTA) (record);
   const char *blimit = record + pEmr->emr.nSize;
   if(!emrtext_safe(&(pEmr->emrtext),record,blimit))return(0);
   return(1);
} 

// Functions that take a rect and a pair of points, starting with U_EMRARC_safe
int core9_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRARC))return(0);
   return(1);
}

// Functions with the same form starting with U_EMRPOLYPOLYLINE16_safe
int core10_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRPOLYPOLYLINE16))return(0);
   PU_EMRPOLYPOLYLINE16 pEmr = (PU_EMRPOLYPOLYLINE16) (record);
   int count = pEmr->cpts;
   int nPolys = pEmr->nPolys;
   const char *blimit = record + pEmr->emr.nSize;
   if(IS_MEM_UNSAFE(pEmr->aPolyCounts, nPolys*4, blimit))return(0);
   record += sizeof(U_EMRPOLYPOLYLINE16) - 4 + sizeof(uint32_t)* nPolys;
   if(IS_MEM_UNSAFE(record, count*sizeof(U_POINT16), blimit))return(0);
   return(1);
} 

// Functions with the same form starting with  U_EMRINVERTRGN_safe and U_EMRPAINTRGN_safe,
int core11_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRINVERTRGN))return(0);
   PU_EMRINVERTRGN pEmr = (PU_EMRINVERTRGN)(record);
   int cbRgnData = pEmr->cbRgnData;
   const char *blimit = record + pEmr->emr.nSize;
   if(IS_MEM_UNSAFE(pEmr->RgnData, cbRgnData, blimit))return(0);
   return(rgndata_safe(pEmr->RgnData, cbRgnData));
} 


// common code for U_EMRCREATEMONOBRUSH_safe and U_EMRCREATEDIBPATTERNBRUSHPT_safe,
int core12_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRCREATEMONOBRUSH))return(0);
   PU_EMRCREATEMONOBRUSH pEmr = (PU_EMRCREATEMONOBRUSH) (record);
   const char *blimit = record + pEmr->emr.nSize;
   U_OFFBMI  offBmi   = pEmr->offBmi;
   U_CBBMI   cbBmi    = pEmr->cbBmi;
   U_OFFBITS offBits  = pEmr->offBits;
   U_CBBITS  cbBits   = pEmr->cbBits;
   uint32_t  iUsage   = pEmr->iUsage;
   return(DIB_safe(record, iUsage, offBmi, cbBmi, offBits, cbBits, blimit));
}

// common code for U_EMRALPHABLEND_safe and U_EMRTRANSPARENTBLT_safe,
int core13_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRALPHABLEND))return(0);
   PU_EMRALPHABLEND pEmr = (PU_EMRALPHABLEND) (record);
   const char *blimit      = record + pEmr->emr.nSize;
   U_OFFBMISRC  offBmiSrc  = pEmr->offBmiSrc;
   U_CBBMISRC   cbBmiSrc   = pEmr->cbBmiSrc; 
   U_OFFBITSSRC offBitsSrc = pEmr->offBitsSrc;
   U_CBBITS     cbBitsSrc  = pEmr->cbBitsSrc;
   uint32_t     iUsageSrc   = pEmr->iUsageSrc;
   return(DIB_safe(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit));
}

/* **********************************************************************************************
These are the core EMR_safe functions, each converts a particular type of record.  
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
int U_EMRNOTIMPLEMENTED_safe(const char *record){
   fprintf(stderr,"EMF WARNING:  could not safety check record because that type has not been implemented!\n");
   return(core5_safe(record, sizeof(U_EMR)));
}

// U_EMRHEADER                1
int U_EMRHEADER_safe(const char *record){
   // use _MIN form so that it accepts very old EMF files
   return(core5_safe(record, U_SIZE_EMRHEADER_MIN));
}

// U_EMRPOLYBEZIER                       2
int U_EMRPOLYBEZIER_safe(const char *record){
   return(core1_safe(record));
} 

// U_EMRPOLYGON                          3
int U_EMRPOLYGON_safe(const char *record){
   return(core1_safe(record));
} 

// U_EMRPOLYLINE              4
int U_EMRPOLYLINE_safe(const char *record){
   return(core1_safe(record));
} 

// U_EMRPOLYBEZIERTO          5
int U_EMRPOLYBEZIERTO_safe(const char *record){
   return(core1_safe(record));
} 

// U_EMRPOLYLINETO            6
int U_EMRPOLYLINETO_safe(const char *record){
   return(core1_safe(record));
} 

// U_EMRPOLYPOLYLINE          7
int U_EMRPOLYPOLYLINE_safe(const char *record){
   return(core2_safe(record));
} 

// U_EMRPOLYPOLYGON           8
int U_EMRPOLYPOLYGON_safe(const char *record){
   return(core2_safe(record));
} 

// U_EMRSETWINDOWEXTEX        9
int U_EMRSETWINDOWEXTEX_safe(const char *record){
   return(core7_safe(record));
} 

// U_EMRSETWINDOWORGEX       10
int U_EMRSETWINDOWORGEX_safe(const char *record){
   return(core7_safe(record));
} 

// U_EMRSETVIEWPORTEXTEX     11
int U_EMRSETVIEWPORTEXTEX_safe(const char *record){
   return(core7_safe(record));
} 

// U_EMRSETVIEWPORTORGEX     12
int U_EMRSETVIEWPORTORGEX_safe(const char *record){
   return(core7_safe(record));
} 

// U_EMRSETBRUSHORGEX        13
int U_EMRSETBRUSHORGEX_safe(const char *record){
   return(core7_safe(record));
} 

// U_EMREOF                  14
int U_EMREOF_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMREOF))return(0);
   PU_EMREOF pEmr = (PU_EMREOF)(record);
   const char *blimit = record + pEmr->emr.nSize;
   int cbPalEntries=pEmr->cbPalEntries;
   if(cbPalEntries){
      if(IS_MEM_UNSAFE(record, pEmr->offPalEntries + 2*2, blimit))return(0);// 2 16 bit values in U_LOGPALLETE
   }
   int off = sizeof(U_EMREOF) + 4 * cbPalEntries;
   if(IS_MEM_UNSAFE(record, off + 4, blimit))return(0);
   return(1);
} 


// U_EMRSETPIXELV            15
int U_EMRSETPIXELV_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRSETPIXELV));

} 


// U_EMRSETMAPPERFLAGS       16
int U_EMRSETMAPPERFLAGS_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRSETMAPPERFLAGS));
} 


// U_EMRSETMAPMODE           17
int U_EMRSETMAPMODE_safe(const char *record){
   return(core3_safe(record));
}

// U_EMRSETBKMODE            18
int U_EMRSETBKMODE_safe(const char *record){
   return(core3_safe(record));
}

// U_EMRSETPOLYFILLMODE      19
int U_EMRSETPOLYFILLMODE_safe(const char *record){
   return(core3_safe(record));
}

// U_EMRSETROP2              20
int U_EMRSETROP2_safe(const char *record){
   return(core3_safe(record));
}

// U_EMRSETSTRETCHBLTMODE    21
int U_EMRSETSTRETCHBLTMODE_safe(const char *record){
   return(core3_safe(record));
}

// U_EMRSETTEXTALIGN         22
int U_EMRSETTEXTALIGN_safe(const char *record){
   return(core3_safe(record));
}

// U_EMRSETCOLORADJUSTMENT   23
int U_EMRSETCOLORADJUSTMENT_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRSETCOLORADJUSTMENT));
}

// U_EMRSETTEXTCOLOR         24
int U_EMRSETTEXTCOLOR_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRSETTEXTCOLOR));
}

// U_EMRSETBKCOLOR           25
int U_EMRSETBKCOLOR_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRSETBKCOLOR));
}

// U_EMROFFSETCLIPRGN        26
int U_EMROFFSETCLIPRGN_safe(const char *record){
   return(core7_safe(record));
} 

// U_EMRMOVETOEX             27
int U_EMRMOVETOEX_safe(const char *record){
   return(core7_safe(record));
} 

// U_EMRSETMETARGN           28
int U_EMRSETMETARGN_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRSETMETARGN));
}

// U_EMREXCLUDECLIPRECT      29
int U_EMREXCLUDECLIPRECT_safe(const char *record){
   return(core4_safe(record));
}

// U_EMRINTERSECTCLIPRECT    30
int U_EMRINTERSECTCLIPRECT_safe(const char *record){
   return(core4_safe(record));
}

// U_EMRSCALEVIEWPORTEXTEX   31
int U_EMRSCALEVIEWPORTEXTEX_safe(const char *record){
   return(core4_safe(record));
}

// U_EMRSCALEWINDOWEXTEX     32
int U_EMRSCALEWINDOWEXTEX_safe(const char *record){
   return(core4_safe(record));
}

// U_EMRSAVEDC               33
int U_EMRSAVEDC_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRSAVEDC));
}

// U_EMRRESTOREDC            34
int U_EMRRESTOREDC_safe(const char *record){
   return(core3_safe(record));
}

// U_EMRSETWORLDTRANSFORM    35
int U_EMRSETWORLDTRANSFORM_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRSETWORLDTRANSFORM));
} 

// U_EMRMODIFYWORLDTRANSFORM 36
int U_EMRMODIFYWORLDTRANSFORM_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRMODIFYWORLDTRANSFORM));
} 

// U_EMRSELECTOBJECT         37
int U_EMRSELECTOBJECT_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRSELECTOBJECT));
} 

// U_EMRCREATEPEN            38
int U_EMRCREATEPEN_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRCREATEPEN));
} 

// U_EMRCREATEBRUSHINDIRECT  39
int U_EMRCREATEBRUSHINDIRECT_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRCREATEBRUSHINDIRECT));
} 

// U_EMRDELETEOBJECT         40
int U_EMRDELETEOBJECT_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRDELETEOBJECT));

} 

// U_EMRANGLEARC             41
int U_EMRANGLEARC_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRANGLEARC));
} 

// U_EMRELLIPSE              42
int U_EMRELLIPSE_safe(const char *record){
   return(core4_safe(record));
}

// U_EMRRECTANGLE            43
int U_EMRRECTANGLE_safe(const char *record){
   return(core4_safe(record));
}

// U_EMRROUNDRECT            44
int U_EMRROUNDRECT_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRROUNDRECT));
}

// U_EMRARC                  45
int U_EMRARC_safe(const char *record){
   return(core9_safe(record));
}

// U_EMRCHORD                46
int U_EMRCHORD_safe(const char *record){
   return(core9_safe(record));
}

// U_EMRPIE                  47
int U_EMRPIE_safe(const char *record){
   return(core9_safe(record));
}

// U_EMRSELECTPALETTE        48
int U_EMRSELECTPALETTE_safe(const char *record){
   return(core3_safe(record));
}

// U_EMRCREATEPALETTE        49
int U_EMRCREATEPALETTE_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRCREATEPALETTE));
}

// U_EMRSETPALETTEENTRIES    50
int U_EMRSETPALETTEENTRIES_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRSETPALETTEENTRIES));
}

// U_EMRRESIZEPALETTE        51
int U_EMRRESIZEPALETTE_safe(const char *record){
   return(core7_safe(record));
} 

// U_EMRREALIZEPALETTE       52
int U_EMRREALIZEPALETTE_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRREALIZEPALETTE));
}

// U_EMREXTFLOODFILL         53
int U_EMREXTFLOODFILL_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMREXTFLOODFILL));
}

// U_EMRLINETO               54
int U_EMRLINETO_safe(const char *record){
   return(core7_safe(record));
} 

// U_EMRARCTO                55
int U_EMRARCTO_safe(const char *record){
   return(core9_safe(record));
}

// U_EMRPOLYDRAW             56
int U_EMRPOLYDRAW_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRPOLYDRAW))return(0);
   PU_EMRPOLYDRAW pEmr = (PU_EMRPOLYDRAW)(record);
   int count = pEmr->cptl;
   const char *blimit = record + pEmr->emr.nSize;
   if(IS_MEM_UNSAFE(pEmr->aptl, count*sizeof(U_POINTL), blimit))return(0);
   return(1);
}

// U_EMRSETARCDIRECTION      57
int U_EMRSETARCDIRECTION_safe(const char *record){
   return(core3_safe(record));
}

// U_EMRSETMITERLIMIT        58
int U_EMRSETMITERLIMIT_safe(const char *record){
   return(core3_safe(record));
}


// U_EMRBEGINPATH            59
int U_EMRBEGINPATH_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRBEGINPATH));
}

// U_EMRENDPATH              60
int U_EMRENDPATH_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRENDPATH));
}

// U_EMRCLOSEFIGURE          61
int U_EMRCLOSEFIGURE_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRCLOSEFIGURE));
}

// U_EMRFILLPATH             62
int U_EMRFILLPATH_safe(const char *record){
   return(core4_safe(record));
}

// U_EMRSTROKEANDFILLPATH    63
int U_EMRSTROKEANDFILLPATH_safe(const char *record){
   return(core4_safe(record));
}

// U_EMRSTROKEPATH           64
int U_EMRSTROKEPATH_safe(const char *record){
   return(core4_safe(record));
}

// U_EMRFLATTENPATH          65
int U_EMRFLATTENPATH_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRFLATTENPATH));
}

// U_EMRWIDENPATH            66
int U_EMRWIDENPATH_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRWIDENPATH));
}

// U_EMRSELECTCLIPPATH       67
int U_EMRSELECTCLIPPATH_safe(const char *record){
   return(core3_safe(record));
}

// U_EMRABORTPATH            68
int U_EMRABORTPATH_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRABORTPATH));
}

// U_EMRUNDEF69                       69
#define U_EMRUNDEF69_safe(A) U_EMRNOTIMPLEMENTED_safe(A) //!< Not implemented.

// U_EMRCOMMENT              70  Comment (any binary data, interpretation is program specific)
int U_EMRCOMMENT_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRCOMMENT))return(0);
   PU_EMRCOMMENT pEmr = (PU_EMRCOMMENT)(record);
   int cbData = pEmr->cbData;
   const char *blimit =record + pEmr->emr.nSize;
   if(IS_MEM_UNSAFE(record, cbData + sizeof(U_SIZE_EMRCOMMENT), blimit))return(0);
   return(1);
} 

// U_EMRFILLRGN              71
int U_EMRFILLRGN_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRFILLRGN))return(0);
   PU_EMRFILLRGN pEmr = (PU_EMRFILLRGN)(record);
   int cbRgnData = pEmr->cbRgnData;
   const char *blimit = record + pEmr->emr.nSize;
   if(IS_MEM_UNSAFE(pEmr->RgnData, cbRgnData, blimit))return(0);
   return(rgndata_safe(pEmr->RgnData, cbRgnData));
} 

// U_EMRFRAMERGN             72
int U_EMRFRAMERGN_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRFRAMERGN))return(0);
   PU_EMRFRAMERGN pEmr = (PU_EMRFRAMERGN)(record);
   int cbRgnData = pEmr->cbRgnData;
   const char *blimit =  record + pEmr->emr.nSize;
   if(IS_MEM_UNSAFE(pEmr->RgnData, cbRgnData, blimit))return(0);
   return(rgndata_safe(pEmr->RgnData, cbRgnData));
} 

// U_EMRINVERTRGN            73
int U_EMRINVERTRGN_safe(const char *record){
   return(core11_safe(record));
}

// U_EMRPAINTRGN             74
int U_EMRPAINTRGN_safe(const char *record){
   return(core11_safe(record));
}

// U_EMREXTSELECTCLIPRGN     75
int U_EMREXTSELECTCLIPRGN_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMREXTSELECTCLIPRGN))return(0);
   PU_EMREXTSELECTCLIPRGN pEmr = (PU_EMREXTSELECTCLIPRGN)(record);
   int cbRgnData = pEmr->cbRgnData;
   /* data size can be 0 with COPY mode, it means clear the clip region. */
   if(pEmr->iMode == U_RGN_COPY && !cbRgnData)return(1);
   const char *blimit = record + pEmr->emr.nSize;
   if(IS_MEM_UNSAFE(pEmr->RgnData, cbRgnData, blimit))return(0);
   return(rgndata_safe(pEmr->RgnData, cbRgnData));
} 

// U_EMRBITBLT               76
int U_EMRBITBLT_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRBITBLT))return(0);
   PU_EMRBITBLT pEmr = (PU_EMRBITBLT) (record);
   const char *blimit      = record + pEmr->emr.nSize;
   U_OFFBMISRC  offBmiSrc  = pEmr->offBmiSrc;
   U_CBBMISRC   cbBmiSrc   = pEmr->cbBmiSrc; 
   U_OFFBITSSRC offBitsSrc = pEmr->offBitsSrc;
   U_CBBITS     cbBitsSrc  = pEmr->cbBitsSrc;
   uint32_t     iUsageSrc  = pEmr->iUsageSrc;
   return(DIB_safe(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit));
}

// U_EMRSTRETCHBLT           77
int U_EMRSTRETCHBLT_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRSTRETCHBLT))return(0);
   PU_EMRBITBLT pEmr = (PU_EMRBITBLT) (record);
   const char *blimit      = record + pEmr->emr.nSize;
   U_OFFBMISRC  offBmiSrc  = pEmr->offBmiSrc;
   U_CBBMISRC   cbBmiSrc   = pEmr->cbBmiSrc; 
   U_OFFBITSSRC offBitsSrc = pEmr->offBitsSrc;
   U_CBBITS     cbBitsSrc  = pEmr->cbBitsSrc;
   uint32_t     iUsageSrc  = pEmr->iUsageSrc;
   return(DIB_safe(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit));
}

// U_EMRMASKBLT              78
int U_EMRMASKBLT_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRMASKBLT))return(0);
   PU_EMRMASKBLT pEmr = (PU_EMRMASKBLT) (record);
   const char *blimit       = record + pEmr->emr.nSize;
   U_OFFBMISRC  offBmiSrc   = pEmr->offBmiSrc;
   U_CBBMISRC   cbBmiSrc    = pEmr->cbBmiSrc;
   U_OFFBMIMSK  offBmiMask  = pEmr->offBmiMask;
   U_CBBMIMSK   cbBmiMask   = pEmr->cbBmiMask;
   U_OFFBITSSRC offBitsSrc  = pEmr->offBitsSrc;
   U_CBBITSSRC  cbBitsSrc   = pEmr->cbBitsSrc;
   U_OFFBITSMSK offBitsMask = pEmr->offBitsMask;
   U_CBBITSMSK  cbBitsMask  = pEmr->cbBitsMask;
   uint32_t     iUsageSrc  = pEmr->iUsageSrc;
   uint32_t     iUsageMask  = pEmr->iUsageMask;
   if(!DIB_safe(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit))return(0);
   return(DIB_safe(record, iUsageMask, offBmiMask, cbBmiMask, offBitsMask, cbBitsMask, blimit));
}

// U_EMRPLGBLT               79
int U_EMRPLGBLT_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRPLGBLT))return(0);
   PU_EMRPLGBLT pEmr = (PU_EMRPLGBLT) (record);
   const char *blimit       = record + pEmr->emr.nSize;
   U_OFFBMISRC  offBmiSrc   = pEmr->offBmiSrc;
   U_CBBMISRC   cbBmiSrc    = pEmr->cbBmiSrc;
   U_OFFBMIMSK  offBmiMask  = pEmr->offBmiMask;
   U_CBBMIMSK   cbBmiMask   = pEmr->cbBmiMask;
   U_OFFBITSSRC offBitsSrc  = pEmr->offBitsSrc;
   U_CBBITSSRC  cbBitsSrc   = pEmr->cbBitsSrc;
   U_OFFBITSMSK offBitsMask = pEmr->offBitsMask;
   U_CBBITSMSK  cbBitsMask  = pEmr->cbBitsMask;
   uint32_t     iUsageSrc  = pEmr->iUsageSrc;
   uint32_t     iUsageMask  = pEmr->iUsageMask;
   if(!DIB_safe(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit))return(0);
   return(DIB_safe(record, iUsageMask, offBmiMask, cbBmiMask, offBitsMask, cbBitsMask, blimit));
}

// U_EMRSETDIBITSTODEVICE    80
int U_EMRSETDIBITSTODEVICE_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRSETDIBITSTODEVICE))return(0);
   PU_EMRSETDIBITSTODEVICE pEmr = (PU_EMRSETDIBITSTODEVICE) (record);
   const char *blimit       = record + pEmr->emr.nSize;
   U_OFFBMISRC  offBmiSrc   = pEmr->offBmiSrc;
   U_CBBMISRC   cbBmiSrc    = pEmr->cbBmiSrc;
   U_OFFBITSSRC offBitsSrc  = pEmr->offBitsSrc;
   U_CBBITSSRC  cbBitsSrc   = pEmr->cbBitsSrc;
   uint32_t     iUsageSrc  = pEmr->iUsageSrc;
   return(DIB_safe(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit));
}

// U_EMRSTRETCHDIBITS        81
int U_EMRSTRETCHDIBITS_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRSTRETCHDIBITS))return(0);
   PU_EMRSTRETCHDIBITS pEmr = (PU_EMRSTRETCHDIBITS) (record);
   const char *blimit       = record + pEmr->emr.nSize;
   U_OFFBMISRC  offBmiSrc   = pEmr->offBmiSrc;
   U_CBBMISRC   cbBmiSrc    = pEmr->cbBmiSrc;
   U_OFFBITSSRC offBitsSrc  = pEmr->offBitsSrc;
   U_CBBITSSRC  cbBitsSrc   = pEmr->cbBitsSrc;
   uint32_t     iUsageSrc  = pEmr->iUsageSrc;
   return(DIB_safe(record, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, blimit));
}

// U_EMREXTCREATEFONTINDIRECTW    82
int U_EMREXTCREATEFONTINDIRECTW_safe(const char *record){
   /* Panose or logfont, LogFontExDv is not supported.  Test smallest to largest */
   if(core5_safe(record, U_SIZE_EMREXTCREATEFONTINDIRECTW_LOGFONT))return(1);
   return(core5_safe(record, U_SIZE_EMREXTCREATEFONTINDIRECTW_LOGFONT_PANOSE));
}

// U_EMREXTTEXTOUTA          83
int U_EMREXTTEXTOUTA_safe(const char *record){
   return(core8_safe(record));
}

// U_EMREXTTEXTOUTW          84
int U_EMREXTTEXTOUTW_safe(const char *record){
   return(core8_safe(record));
}

// U_EMRPOLYBEZIER16         85
/**
    \brief Convert a pointer to a U_EMR_POLYBEZIER16 record.
    \param record   pointer to a buffer holding the EMR record
*/
int U_EMRPOLYBEZIER16_safe(const char *record){
   return(core6_safe(record));
}

// U_EMRPOLYGON16            86
int U_EMRPOLYGON16_safe(const char *record){
   return(core6_safe(record));
}

// U_EMRPOLYLINE16           87
int U_EMRPOLYLINE16_safe(const char *record){
   return(core6_safe(record));
}

// U_EMRPOLYBEZIERTO16       88
int U_EMRPOLYBEZIERTO16_safe(const char *record){
   return(core6_safe(record));
}

// U_EMRPOLYLINETO16         89
/**
    \brief Convert a pointer to a U_EMR_POLYLINETO16 record.
    \param record   pointer to a buffer holding the EMR record
*/
int U_EMRPOLYLINETO16_safe(const char *record){
   return(core6_safe(record));
}

// U_EMRPOLYPOLYLINE16       90
int U_EMRPOLYPOLYLINE16_safe(const char *record){
   return(core10_safe(record));
}

// U_EMRPOLYPOLYGON16        91
int U_EMRPOLYPOLYGON16_safe(const char *record){
   return(core10_safe(record));
}


// U_EMRPOLYDRAW16           92
int U_EMRPOLYDRAW16_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRPOLYDRAW16))return(0);
   PU_EMRPOLYDRAW16 pEmr = (PU_EMRPOLYDRAW16)(record);
   int count = pEmr->cpts;
   const char *blimit = record + pEmr->emr.nSize;
   if(IS_MEM_UNSAFE(pEmr->apts, count*sizeof(U_POINT16), blimit))return(0);
   return(1);
}

// U_EMRCREATEMONOBRUSH      93
int U_EMRCREATEMONOBRUSH_safe(const char *record){
   return(core12_safe(record));
}

// U_EMRCREATEDIBPATTERNBRUSHPT_safe   94
int U_EMRCREATEDIBPATTERNBRUSHPT_safe(const char *record){
   return(core12_safe(record));
}


// U_EMREXTCREATEPEN         95
int U_EMREXTCREATEPEN_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMREXTCREATEPEN))return(0);
   PU_EMREXTCREATEPEN pEmr = (PU_EMREXTCREATEPEN)(record);
   const char *blimit     = record + pEmr->emr.nSize;
   U_OFFBMI     offBmi  = pEmr->offBmi;
   U_CBBMI      cbBmi   = pEmr->cbBmi;
   U_OFFBITS    offBits = pEmr->offBits;
   U_CBBITS     cbBits  = pEmr->cbBits;
   if(!DIB_safe(record, U_DIB_RGB_COLORS, offBmi, cbBmi, offBits, cbBits, blimit))return(0);
   return(extlogpen_safe((PU_EXTLOGPEN) &(pEmr->elp), blimit)); 
}

// U_EMRPOLYTEXTOUTA         96 NOT IMPLEMENTED, denigrated after Windows NT
#define U_EMRPOLYTEXTOUTA_safe(A) U_EMRNOTIMPLEMENTED_safe(A) //!< Not implemented.
// U_EMRPOLYTEXTOUTW         97 NOT IMPLEMENTED, denigrated after Windows NT
#define U_EMRPOLYTEXTOUTW_safe(A) U_EMRNOTIMPLEMENTED_safe(A) //!< Not implemented.

// U_EMRSETICMMODE           98
int U_EMRSETICMMODE_safe(const char *record){
   return(core3_safe(record));
}

// U_EMRCREATECOLORSPACE     99
int U_EMRCREATECOLORSPACE_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRCREATECOLORSPACE));
}

// U_EMRSETCOLORSPACE       100
int U_EMRSETCOLORSPACE_safe(const char *record){
   return(core3_safe(record));
}

// U_EMRDELETECOLORSPACE    101
int U_EMRDELETECOLORSPACE_safe(const char *record){
   return(core3_safe(record));
}

// U_EMRGLSRECORD           102  Not implemented
#define U_EMRGLSRECORD_safe(A) U_EMRNOTIMPLEMENTED_safe(A) //!< Not implemented.
// U_EMRGLSBOUNDEDRECORD    103  Not implemented
#define U_EMRGLSBOUNDEDRECORD_safe(A) U_EMRNOTIMPLEMENTED_safe(A) //!< Not implemented.

// U_EMRPIXELFORMAT         104
int U_EMRPIXELFORMAT_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRPIXELFORMAT));
}

// U_EMRDRAWESCAPE          105  Not implemented
#define U_EMRDRAWESCAPE_safe(A) U_EMRNOTIMPLEMENTED_safe(A) //!< Not implemented.
// U_EMREXTESCAPE           106  Not implemented
#define U_EMREXTESCAPE_safe(A) U_EMRNOTIMPLEMENTED_safe(A) //!< Not implemented.
// U_EMRUNDEF107            107  Not implemented
#define U_EMRUNDEF107_safe(A) U_EMRNOTIMPLEMENTED_safe(A) //!< Not implemented.

// U_EMRSMALLTEXTOUT        108
int U_EMRSMALLTEXTOUT_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRSMALLTEXTOUT))return(0);
   PU_EMRSMALLTEXTOUT pEmr = (PU_EMRSMALLTEXTOUT)(record);
   int roff=sizeof(U_EMRSMALLTEXTOUT);        // offset to the start of the variable fields
   int fuOptions = pEmr->fuOptions;
   int cChars = pEmr->cChars;
   const char *blimit = record + pEmr->emr.nSize;
   if(!(fuOptions & U_ETO_NO_RECT)){
      if(IS_MEM_UNSAFE(record, roff + sizeof(U_RECTL), blimit))return(0);
   }
   if(IS_MEM_UNSAFE(record, roff + sizeof(U_RECTL) + cChars, blimit))return(0);
   return(1);
}

// U_EMRFORCEUFIMAPPING     109  Not implemented
#define U_EMRFORCEUFIMAPPING_safe(A) U_EMRNOTIMPLEMENTED_safe(A) //!< Not implemented.
// U_EMRNAMEDESCAPE         110  Not implemented
#define U_EMRNAMEDESCAPE_safe(A) U_EMRNOTIMPLEMENTED_safe(A) //!< Not implemented.
// U_EMRCOLORCORRECTPALETTE 111  Not implemented
#define U_EMRCOLORCORRECTPALETTE_safe(A) U_EMRNOTIMPLEMENTED_safe(A) //!< Not implemented.
// U_EMRSETICMPROFILEA      112  Not implemented
#define U_EMRSETICMPROFILEA_safe(A) U_EMRNOTIMPLEMENTED_safe(A) //!< Not implemented.
// U_EMRSETICMPROFILEW      113  Not implemented
#define U_EMRSETICMPROFILEW_safe(A) U_EMRNOTIMPLEMENTED_safe(A) //!< Not implemented.

// U_EMRALPHABLEND          114
int U_EMRALPHABLEND_safe(const char *record){
   return(core13_safe(record));
}

// U_EMRSETLAYOUT           115
int U_EMRSETLAYOUT_safe(const char *record){
   return(core3_safe(record));
}

// U_EMRTRANSPARENTBLT      116
int U_EMRTRANSPARENTBLT_safe(const char *record){
   return(core13_safe(record));
}


// U_EMRUNDEF117            117  Not implemented
#define U_EMRUNDEF117_safe(A) U_EMRNOTIMPLEMENTED_safe(A) //!< Not implemented.
// U_EMRGRADIENTFILL        118
int U_EMRGRADIENTFILL_safe(const char *record){
   if(!core5_safe(record, U_SIZE_EMRGRADIENTFILL))return(0);
   PU_EMRGRADIENTFILL pEmr = (PU_EMRGRADIENTFILL)(record);
   int nTriVert = pEmr->nTriVert;
   int nGradObj = pEmr->nGradObj;
   int ulMode   = pEmr->ulMode;
   const char *blimit = record + pEmr->emr.nSize;
   if(IS_MEM_UNSAFE(record, nTriVert*sizeof(U_TRIVERTEX), blimit))return(0);
   record += nTriVert * sizeof(U_TRIVERTEX);
   if(nGradObj){
      if(     ulMode == U_GRADIENT_FILL_TRIANGLE){
         if(IS_MEM_UNSAFE(record, nGradObj*sizeof(U_GRADIENT3), blimit))return(0);
      }
      else if(ulMode == U_GRADIENT_FILL_RECT_H || ulMode == U_GRADIENT_FILL_RECT_V){
         if(IS_MEM_UNSAFE(record, nGradObj*sizeof(U_GRADIENT4), blimit))return(0);
      }
   }
   return(1);
}

// U_EMRSETLINKEDUFIS       119  Not implemented
#define U_EMRSETLINKEDUFIS_safe(A) U_EMRNOTIMPLEMENTED_safe(A) //!< Not implemented.
// U_EMRSETTEXTJUSTIFICATION120  Not implemented (denigrated)
#define U_EMRSETTEXTJUSTIFICATION_safe(A) U_EMRNOTIMPLEMENTED_safe(A) //!< Not implemented.
// U_EMRCOLORMATCHTOTARGETW 121  Not implemented  
#define U_EMRCOLORMATCHTOTARGETW_safe(A) U_EMRNOTIMPLEMENTED_safe(A) //!< Not implemented.

// U_EMRCREATECOLORSPACEW   122
int U_EMRCREATECOLORSPACEW_safe(const char *record){
   return(core5_safe(record, U_SIZE_EMRCREATECOLORSPACEW));
}

//! \endcond

/**
    \brief Test an EMF record in memory from Big Endian to Little Endian.
    \return 0 on failure, 1 on success
    \param record   pointer to the EMF record in memory
    
    Normally this would be called immediately after reading a record from a file
      and having called U_emf_record_sizeok().  
    It is NOT safe to call this routine without first calling U_emf_record_sizeok)()!
    If the file has been converted from one endian to another calling this routine is
      not necessary, because those routines also perform these checks.
*/
int U_emf_record_safe(const char *record){
    int rstatus=1;
    
    if(!record)return(0);  // programming error

    switch (U_EMRTYPE(record))
    {
        case U_EMR_HEADER:                  rstatus=U_EMRHEADER_safe(record);                  break;
        case U_EMR_POLYBEZIER:              rstatus=U_EMRPOLYBEZIER_safe(record);              break;
        case U_EMR_POLYGON:                 rstatus=U_EMRPOLYGON_safe(record);                 break;
        case U_EMR_POLYLINE:                rstatus=U_EMRPOLYLINE_safe(record);                break;
        case U_EMR_POLYBEZIERTO:            rstatus=U_EMRPOLYBEZIERTO_safe(record);            break;
        case U_EMR_POLYLINETO:              rstatus=U_EMRPOLYLINETO_safe(record);              break;
        case U_EMR_POLYPOLYLINE:            rstatus=U_EMRPOLYPOLYLINE_safe(record);            break;
        case U_EMR_POLYPOLYGON:             rstatus=U_EMRPOLYPOLYGON_safe(record);             break;
        case U_EMR_SETWINDOWEXTEX:          rstatus=U_EMRSETWINDOWEXTEX_safe(record);          break;
        case U_EMR_SETWINDOWORGEX:          rstatus=U_EMRSETWINDOWORGEX_safe(record);          break;
        case U_EMR_SETVIEWPORTEXTEX:        rstatus=U_EMRSETVIEWPORTEXTEX_safe(record);        break;
        case U_EMR_SETVIEWPORTORGEX:        rstatus=U_EMRSETVIEWPORTORGEX_safe(record);        break;
        case U_EMR_SETBRUSHORGEX:           rstatus=U_EMRSETBRUSHORGEX_safe(record);           break;
        case U_EMR_EOF:                     rstatus=U_EMREOF_safe(record);                     break;
        case U_EMR_SETPIXELV:               rstatus=U_EMRSETPIXELV_safe(record);               break;
        case U_EMR_SETMAPPERFLAGS:          rstatus=U_EMRSETMAPPERFLAGS_safe(record);          break;
        case U_EMR_SETMAPMODE:              rstatus=U_EMRSETMAPMODE_safe(record);              break;
        case U_EMR_SETBKMODE:               rstatus=U_EMRSETBKMODE_safe(record);               break;
        case U_EMR_SETPOLYFILLMODE:         rstatus=U_EMRSETPOLYFILLMODE_safe(record);         break;
        case U_EMR_SETROP2:                 rstatus=U_EMRSETROP2_safe(record);                 break;
        case U_EMR_SETSTRETCHBLTMODE:       rstatus=U_EMRSETSTRETCHBLTMODE_safe(record);       break;
        case U_EMR_SETTEXTALIGN:            rstatus=U_EMRSETTEXTALIGN_safe(record);            break;
        case U_EMR_SETCOLORADJUSTMENT:      rstatus=U_EMRSETCOLORADJUSTMENT_safe(record);      break;
        case U_EMR_SETTEXTCOLOR:            rstatus=U_EMRSETTEXTCOLOR_safe(record);            break;
        case U_EMR_SETBKCOLOR:              rstatus=U_EMRSETBKCOLOR_safe(record);              break;
        case U_EMR_OFFSETCLIPRGN:           rstatus=U_EMROFFSETCLIPRGN_safe(record);           break;
        case U_EMR_MOVETOEX:                rstatus=U_EMRMOVETOEX_safe(record);                break;
        case U_EMR_SETMETARGN:              rstatus=U_EMRSETMETARGN_safe(record);              break;
        case U_EMR_EXCLUDECLIPRECT:         rstatus=U_EMREXCLUDECLIPRECT_safe(record);         break;
        case U_EMR_INTERSECTCLIPRECT:       rstatus=U_EMRINTERSECTCLIPRECT_safe(record);       break;
        case U_EMR_SCALEVIEWPORTEXTEX:      rstatus=U_EMRSCALEVIEWPORTEXTEX_safe(record);      break;
        case U_EMR_SCALEWINDOWEXTEX:        rstatus=U_EMRSCALEWINDOWEXTEX_safe(record);        break;
        case U_EMR_SAVEDC:                  rstatus=U_EMRSAVEDC_safe(record);                  break;
        case U_EMR_RESTOREDC:               rstatus=U_EMRRESTOREDC_safe(record);               break;
        case U_EMR_SETWORLDTRANSFORM:       rstatus=U_EMRSETWORLDTRANSFORM_safe(record);       break;
        case U_EMR_MODIFYWORLDTRANSFORM:    rstatus=U_EMRMODIFYWORLDTRANSFORM_safe(record);    break;
        case U_EMR_SELECTOBJECT:            rstatus=U_EMRSELECTOBJECT_safe(record);            break;
        case U_EMR_CREATEPEN:               rstatus=U_EMRCREATEPEN_safe(record);               break;
        case U_EMR_CREATEBRUSHINDIRECT:     rstatus=U_EMRCREATEBRUSHINDIRECT_safe(record);     break;
        case U_EMR_DELETEOBJECT:            rstatus=U_EMRDELETEOBJECT_safe(record);            break;
        case U_EMR_ANGLEARC:                rstatus=U_EMRANGLEARC_safe(record);                break;
        case U_EMR_ELLIPSE:                 rstatus=U_EMRELLIPSE_safe(record);                 break;
        case U_EMR_RECTANGLE:               rstatus=U_EMRRECTANGLE_safe(record);               break;
        case U_EMR_ROUNDRECT:               rstatus=U_EMRROUNDRECT_safe(record);               break;
        case U_EMR_ARC:                     rstatus=U_EMRARC_safe(record);                     break;
        case U_EMR_CHORD:                   rstatus=U_EMRCHORD_safe(record);                   break;
        case U_EMR_PIE:                     rstatus=U_EMRPIE_safe(record);                     break;
        case U_EMR_SELECTPALETTE:           rstatus=U_EMRSELECTPALETTE_safe(record);           break;
        case U_EMR_CREATEPALETTE:           rstatus=U_EMRCREATEPALETTE_safe(record);           break;
        case U_EMR_SETPALETTEENTRIES:       rstatus=U_EMRSETPALETTEENTRIES_safe(record);       break;
        case U_EMR_RESIZEPALETTE:           rstatus=U_EMRRESIZEPALETTE_safe(record);           break;
        case U_EMR_REALIZEPALETTE:          rstatus=U_EMRREALIZEPALETTE_safe(record);          break;
        case U_EMR_EXTFLOODFILL:            rstatus=U_EMREXTFLOODFILL_safe(record);            break;
        case U_EMR_LINETO:                  rstatus=U_EMRLINETO_safe(record);                  break;
        case U_EMR_ARCTO:                   rstatus=U_EMRARCTO_safe(record);                   break;
        case U_EMR_POLYDRAW:                rstatus=U_EMRPOLYDRAW_safe(record);                break;
        case U_EMR_SETARCDIRECTION:         rstatus=U_EMRSETARCDIRECTION_safe(record);         break;
        case U_EMR_SETMITERLIMIT:           rstatus=U_EMRSETMITERLIMIT_safe(record);           break;
        case U_EMR_BEGINPATH:               rstatus=U_EMRBEGINPATH_safe(record);               break;
        case U_EMR_ENDPATH:                 rstatus=U_EMRENDPATH_safe(record);                 break;
        case U_EMR_CLOSEFIGURE:             rstatus=U_EMRCLOSEFIGURE_safe(record);             break;
        case U_EMR_FILLPATH:                rstatus=U_EMRFILLPATH_safe(record);                break;
        case U_EMR_STROKEANDFILLPATH:       rstatus=U_EMRSTROKEANDFILLPATH_safe(record);       break;
        case U_EMR_STROKEPATH:              rstatus=U_EMRSTROKEPATH_safe(record);              break;
        case U_EMR_FLATTENPATH:             rstatus=U_EMRFLATTENPATH_safe(record);             break;
        case U_EMR_WIDENPATH:               rstatus=U_EMRWIDENPATH_safe(record);               break;
        case U_EMR_SELECTCLIPPATH:          rstatus=U_EMRSELECTCLIPPATH_safe(record);          break;
        case U_EMR_ABORTPATH:               rstatus=U_EMRABORTPATH_safe(record);               break;
        case U_EMR_UNDEF69:                 rstatus=U_EMRUNDEF69_safe(record);                 break;
        case U_EMR_COMMENT:                 rstatus=U_EMRCOMMENT_safe(record);                 break;
        case U_EMR_FILLRGN:                 rstatus=U_EMRFILLRGN_safe(record);                 break;
        case U_EMR_FRAMERGN:                rstatus=U_EMRFRAMERGN_safe(record);                break;
        case U_EMR_INVERTRGN:               rstatus=U_EMRINVERTRGN_safe(record);               break;
        case U_EMR_PAINTRGN:                rstatus=U_EMRPAINTRGN_safe(record);                break;
        case U_EMR_EXTSELECTCLIPRGN:        rstatus=U_EMREXTSELECTCLIPRGN_safe(record);        break;
        case U_EMR_BITBLT:                  rstatus=U_EMRBITBLT_safe(record);                  break;
        case U_EMR_STRETCHBLT:              rstatus=U_EMRSTRETCHBLT_safe(record);              break;
        case U_EMR_MASKBLT:                 rstatus=U_EMRMASKBLT_safe(record);                 break;
        case U_EMR_PLGBLT:                  rstatus=U_EMRPLGBLT_safe(record);                  break;
        case U_EMR_SETDIBITSTODEVICE:       rstatus=U_EMRSETDIBITSTODEVICE_safe(record);       break;
        case U_EMR_STRETCHDIBITS:           rstatus=U_EMRSTRETCHDIBITS_safe(record);           break;
        case U_EMR_EXTCREATEFONTINDIRECTW:  rstatus=U_EMREXTCREATEFONTINDIRECTW_safe(record);  break;
        case U_EMR_EXTTEXTOUTA:             rstatus=U_EMREXTTEXTOUTA_safe(record);             break;
        case U_EMR_EXTTEXTOUTW:             rstatus=U_EMREXTTEXTOUTW_safe(record);             break;
        case U_EMR_POLYBEZIER16:            rstatus=U_EMRPOLYBEZIER16_safe(record);            break;
        case U_EMR_POLYGON16:               rstatus=U_EMRPOLYGON16_safe(record);               break;
        case U_EMR_POLYLINE16:              rstatus=U_EMRPOLYLINE16_safe(record);              break;
        case U_EMR_POLYBEZIERTO16:          rstatus=U_EMRPOLYBEZIERTO16_safe(record);          break;
        case U_EMR_POLYLINETO16:            rstatus=U_EMRPOLYLINETO16_safe(record);            break;
        case U_EMR_POLYPOLYLINE16:          rstatus=U_EMRPOLYPOLYLINE16_safe(record);          break;
        case U_EMR_POLYPOLYGON16:           rstatus=U_EMRPOLYPOLYGON16_safe(record);           break;
        case U_EMR_POLYDRAW16:              rstatus=U_EMRPOLYDRAW16_safe(record);              break;
        case U_EMR_CREATEMONOBRUSH:         rstatus=U_EMRCREATEMONOBRUSH_safe(record);         break;
        case U_EMR_CREATEDIBPATTERNBRUSHPT: rstatus=U_EMRCREATEDIBPATTERNBRUSHPT_safe(record); break;
        case U_EMR_EXTCREATEPEN:            rstatus=U_EMREXTCREATEPEN_safe(record);            break;
        case U_EMR_POLYTEXTOUTA:            rstatus=U_EMRPOLYTEXTOUTA_safe(record);            break;
        case U_EMR_POLYTEXTOUTW:            rstatus=U_EMRPOLYTEXTOUTW_safe(record);            break;
        case U_EMR_SETICMMODE:              rstatus=U_EMRSETICMMODE_safe(record);              break;
        case U_EMR_CREATECOLORSPACE:        rstatus=U_EMRCREATECOLORSPACE_safe(record);        break;
        case U_EMR_SETCOLORSPACE:           rstatus=U_EMRSETCOLORSPACE_safe(record);           break;
        case U_EMR_DELETECOLORSPACE:        rstatus=U_EMRDELETECOLORSPACE_safe(record);        break;
        case U_EMR_GLSRECORD:               rstatus=U_EMRGLSRECORD_safe(record);               break;
        case U_EMR_GLSBOUNDEDRECORD:        rstatus=U_EMRGLSBOUNDEDRECORD_safe(record);        break;
        case U_EMR_PIXELFORMAT:             rstatus=U_EMRPIXELFORMAT_safe(record);             break;
        case U_EMR_DRAWESCAPE:              rstatus=U_EMRDRAWESCAPE_safe(record);              break;
        case U_EMR_EXTESCAPE:               rstatus=U_EMREXTESCAPE_safe(record);               break;
        case U_EMR_UNDEF107:                rstatus=U_EMRUNDEF107_safe(record);                break;
        case U_EMR_SMALLTEXTOUT:            rstatus=U_EMRSMALLTEXTOUT_safe(record);            break;
        case U_EMR_FORCEUFIMAPPING:         rstatus=U_EMRFORCEUFIMAPPING_safe(record);         break;
        case U_EMR_NAMEDESCAPE:             rstatus=U_EMRNAMEDESCAPE_safe(record);             break;
        case U_EMR_COLORCORRECTPALETTE:     rstatus=U_EMRCOLORCORRECTPALETTE_safe(record);     break;
        case U_EMR_SETICMPROFILEA:          rstatus=U_EMRSETICMPROFILEA_safe(record);          break;
        case U_EMR_SETICMPROFILEW:          rstatus=U_EMRSETICMPROFILEW_safe(record);          break;
        case U_EMR_ALPHABLEND:              rstatus=U_EMRALPHABLEND_safe(record);              break;
        case U_EMR_SETLAYOUT:               rstatus=U_EMRSETLAYOUT_safe(record);               break;
        case U_EMR_TRANSPARENTBLT:          rstatus=U_EMRTRANSPARENTBLT_safe(record);          break;
        case U_EMR_UNDEF117:                rstatus=U_EMRUNDEF117_safe(record);                break;
        case U_EMR_GRADIENTFILL:            rstatus=U_EMRGRADIENTFILL_safe(record);            break;
        case U_EMR_SETLINKEDUFIS:           rstatus=U_EMRSETLINKEDUFIS_safe(record);           break;
        case U_EMR_SETTEXTJUSTIFICATION:    rstatus=U_EMRSETTEXTJUSTIFICATION_safe(record);    break;
        case U_EMR_COLORMATCHTOTARGETW:     rstatus=U_EMRCOLORMATCHTOTARGETW_safe(record);     break;
        case U_EMR_CREATECOLORSPACEW:       rstatus=U_EMRCREATECOLORSPACEW_safe(record);       break;
        default:                            rstatus=U_EMRNOTIMPLEMENTED_safe(record);          break;
    }  //end of switch
    return(rstatus);
}


#ifdef __cplusplus
}
#endif
