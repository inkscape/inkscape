/**
  @file upmf.c
  
  @brief Functions for manipulating EMF+ files and structures.

  EMF+ is much more object based than is EMF or WMF, so the U_PMR_*_set and most U_PMF_*_set functions
  return a pointer to a PseudoObject.  PseudoObjects are structs that contain a data field to hold the
  object in EMF+ file byte order, size information, and some type information.  This is sufficient to allow
  complex records to be built up from the various sorts of nested objects which they normally contain.
  If something goes wrong a NULL pointer is returned and recsize is set to 0.
  
  EMF+ does not use a separate set of endian functions, _get and _set routines convert from/to
  the EMF+ file byte order on the fly.
    
        WARNING:  Microsoft's EMF+ documentation is little-endian for everything EXCEPT
        bitfields,  which are big-endian.   See EMF+ manual section 1.3.2              
        That documentation also uses 0 as the MOST significant bit, N-1 as the least.  
        This code is little-endian throughout, and 0 is the LEAST significant bit      
  
*/

/*
File:      upmf.c
Version:   0.0.12
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
#include <math.h>   // for sin, cos, tan2, use U_ROUND() instead of roundf() 
#include <stddef.h> /* for offsetof() macro */
#if 0
#include <windef.h>    //Not actually used, looking for collisions
#include <winnt.h>    //Not actually used, looking for collisions
#include <wingdi.h>   //Not actually used, looking for collisions
#endif
#include "upmf.h"         // includes uemf.h
#include "uemf_endian.h"  // for U_swap* functions

//! \cond

/// things Doxygen should not process
/* remove this after debugging is completed */
void dumphex(uint8_t *buf,unsigned int num){
   for(; num; num--,buf++){
      printf("%2.2X",*buf);
   }
}


/* Prototypes for functions used here and defined in uemf_endian.c, but which are not supposed
to be used in end user code. */

void U_swap2(void *ul, unsigned int count);
void U_swap4(void *ul, unsigned int count);
//! \endcond

/**
    \brief  Utility function for writing one or more EMF+ records in a PseudoObject to the EMF output file
    \return 1 on success, 0 on error.
    \param  po          U_PSEUDO_OBJ to write, it is deleted after it is written
    \param  sum         U_PSEUDO_OBJ to use for scratch space
    \param  et          EMFTRACK used to write records to EMF file
*/
int U_PMR_write(U_PSEUDO_OBJ *po, U_PSEUDO_OBJ *sum, EMFTRACK *et){
   char *rec;
   int status = 0;
   sum->Used = 0;                                          /* clean it out, retaining allocated memory         */
   sum       = U_PO_append(sum, "EMF+", 4);                /* indicates that this comment holds an EMF+ record */
   if(!sum)goto end;
   sum       = U_PO_append(sum,  po->Data, po->Used);      /* the EMF+ record itself                           */
   if(!sum)goto end;
   U_PO_free(&po);                                         /* delete the PseudoObject                          */
   rec       = U_EMRCOMMENT_set(sum->Used, sum->Data);     /* stuff it into the EMF comment                    */
   if(!emf_append((PU_ENHMETARECORD)rec, et, 1))goto end;  /* write it to the EMF file, delete the record, check status */
   status = 1;
end:
   return(status);
}

/**
    \brief  Utility function to draw a line.
    \return 1 on success, 0 on error.
    \param  PenID       Index of U_PMF_PEN object to use in the EMF+ object table (0-63, inclusive)
    \param  PathID      Index of U_PMF_PATH object to use in the EMF+ object table (0-63, inclusive)
    \param  Start       U_PMF_POINTF coordinates of start of line.
    \param  End         U_PMF_POINTF coordinates of end of line.
    \param  Dashed      Set if the line is dashed, clear if it is not.
    \param  sum         PseudoObject used for scratch space
    \param  et          EMFTRACK used to write records to EMF file
   
*/
int U_PMR_drawline(uint32_t PenID, uint32_t PathID, U_PMF_POINTF Start, U_PMF_POINTF End, int Dashed, U_PSEUDO_OBJ *sum, EMFTRACK *et){
   U_DPSEUDO_OBJ *dpath;
   U_PSEUDO_OBJ *poPath;
   U_PSEUDO_OBJ *po;
   int           status=0;
   int           PTP_value = ( Dashed ? U_PTP_DashMode : U_PTP_None);
   dpath =  U_PATH_create(0, NULL, 0, 0); /* create an empty path*/
   if(dpath){
      if(U_PATH_moveto(dpath, Start, PTP_value) && U_PATH_lineto(dpath, End, PTP_value)){
         poPath = U_PMF_PATH_set2(U_PMF_GRAPHICSVERSIONOBJ_set(2), dpath);
         if(poPath){
            po = U_PMR_OBJECT_PO_set(PathID, poPath);
            U_PO_free(&poPath);
            if(po){
               U_PMR_write(po, sum, et);
               po = U_PMR_DRAWPATH_set(PathID, PenID);
               if(po){
                  U_PMR_write(po, sum, et);
                  status = 1;
               }
            }
         }
      }
      U_DPO_free(&dpath);
   }
   return(status);
}

/**
    \brief  Utility function for drawing strings onto the baseline in one call.
    \return 1 on success, 0 on error.
    \param  string      Text to draw in UTF-8 format
    \param  Vpos        StringAlignment Enumeration.  Always drawn on baseline, but using one of these three modes.
    \param  FontID      Index of U_PMF_FONT object to use in the EMF+ object table (0-63, inclusive)
    \param  BrushID     U_PSEUDO_OBJ containing a U_PMF_ARGB or a U_PMF_4NUM. Color or U_PMF_BRUSH object in the EMF+ object table (0-63, inclusive)
    \param  FormatID    index of U_PMF_STRINGFORMAT object to use in the EMF+ Object Table.
    \param  Sfs         StringFormat structure. Ignored values: StringAlignment, LineAlign, Flags
    \param  FontName    Name of font to draw with
    \param  Height      Height of font in pixels (positive)
    \param  fip         U_FontInfoParams (ascent, descent, and so forth)
    \param  FontFlags   FontStyle Flags
    \param  x           X position in pixels of left side of EM box of first character
    \param  y           Y position in pixels of baseline of first character
    \param  sum         PseudoObject used for scratch space
    \param  et          EMFTRACK used to write records to EMF file
   
    EMF+ manual 2.3.4.14, Microsoft name: EmfPlusDrawString Record, Index 0x1C
    
    For most fonts Ascent and Descent are used to adjust the bounding box to properly position the
    baseline.  Some fonts, like Verdana, are strange and they position the baseline on the bottom of
    the bounding box if that box has the same height as the font.  For those fonts specify 0.0 for
    both Ascent and Descent.
*/
int U_PMR_drawstring( const char *string, int Vpos, uint32_t FontID, const U_PSEUDO_OBJ *BrushID, uint32_t FormatID,
      U_PMF_STRINGFORMAT  Sfs, const char *FontName, U_FLOAT Height, U_FontInfoParams *fip, uint32_t FontFlags,
      U_FLOAT x, U_FLOAT y, U_PSEUDO_OBJ *sum, EMFTRACK *et){
   U_PSEUDO_OBJ  *po;
   U_PSEUDO_OBJ  *poSF;
   U_PSEUDO_OBJ  *poFont;
   U_PSEUDO_OBJ  *poRect;
   U_FLOAT        rx,ry,rw,rh,rd;
   uint16_t      *UFontName;
   uint16_t      *Text16;
   int            slen;
   int            status = 0;
   double         aval, dval;
   
   Sfs.Flags           = U_SF_NoFitBlackBox + U_SF_NoClip;
   
   if(Vpos < U_SA_Near || Vpos > U_SA_Far)return(0);
   Sfs.StringAlignment = U_SA_Near; //  Horizontal
   Sfs.LineAlign       = Vpos; //  Vertical

   UFontName = U_Utf8ToUtf16le(FontName, 0, NULL);
   slen = strlen(FontName);
   poFont = U_PMF_FONT_set(U_PMF_GRAPHICSVERSIONOBJ_set(2), Height, U_UT_World, FontFlags, slen, UFontName);
   if(poFont){
      po = U_PMR_OBJECT_PO_set(FontID, poFont); /* font to use */
      if(po){
         U_PMR_write(po, sum, et);
         
         poSF = U_PMF_STRINGFORMAT_set(&Sfs, NULL);
         if(poSF){
            po = U_PMR_OBJECT_PO_set(FormatID, poSF);
            U_PO_free(&poSF);
            if(po){
               U_PMR_write(po, sum, et);

               rw = 4*Height*slen; /* This could probably be any value */
               rh = Height;
               rx = x;
               if(fip->LineGap > -fip->Descent){ aval = fip->yMax;   } // sylfaen, palatino
               else {                            aval = fip->Ascent; } // others
               if(fip->LineGap && (fip->LineGap < -fip->Descent)){ dval =  ((double) (fip->Descent - fip->LineGap)) / ((double) fip->EmSize); } //shruti and some others
               else {                                              dval =  ((double)  fip->Descent                ) / ((double) fip->EmSize); }
               switch(Vpos){
                  case U_SA_Near:  
                     rd = Height * aval / ((double) fip->EmSize);
                     break;
                  case U_SA_Center:
                     rd = 0.5 * (  Height * aval / ((double) fip->EmSize) +  Height * ( 1.0 +  dval));
                     break;
                  case U_SA_Far:
                     rd = Height * ( 1.0 +  dval);
                     break;
               }
               ry = y - rd;   /* draw from upper left corner, which is shifted to put baseline on y */
               poRect = U_PMF_RECTF4_set(rx, ry, rw, rh); 
#if 0
(void) U_PMR_drawline(OBJ_PEN_BLACK_1,OBJ_PATH_1, (U_PMF_POINTF){x-100, ry}, (U_PMF_POINTF){x+100, ry},  0, sum, et);
(void) U_PMR_drawline(OBJ_PEN_BLACK_1,OBJ_PATH_1, (U_PMF_POINTF){x-100, ry+rh}, (U_PMF_POINTF){x+100, ry+rh},  0, sum, et);
(void) U_PMR_drawline(OBJ_PEN_BLACK_1,OBJ_PATH_1, (U_PMF_POINTF){x-100, ry}, (U_PMF_POINTF){x-100, ry + Height * (double) fip->Ascent / ((double) fip->EmSize)},  0, sum, et);
(void) U_PMR_drawline(OBJ_PEN_BLACK_1,OBJ_PATH_1, (U_PMF_POINTF){x- 90, ry}, (U_PMF_POINTF){x- 90, ry - Height * (double) fip->Descent / ((double) fip->EmSize)},  0, sum, et);
(void) U_PMR_drawline(OBJ_PEN_BLACK_1,OBJ_PATH_1, (U_PMF_POINTF){x- 80, ry}, (U_PMF_POINTF){x- 80, ry + Height * (double) fip->yMax / ((double) fip->EmSize)},  0, sum, et);
(void) U_PMR_drawline(OBJ_PEN_BLACK_1,OBJ_PATH_1, (U_PMF_POINTF){x- 70, ry}, (U_PMF_POINTF){x- 70, ry - Height * (double) fip->yMin / ((double) fip->EmSize)},  0, sum, et);
(void) U_PMR_drawline(OBJ_PEN_BLACK_1,OBJ_PATH_1, (U_PMF_POINTF){x- 60, ry}, (U_PMF_POINTF){x- 60, ry + Height * (double) fip->LineGap / ((double) fip->EmSize)},  0, sum, et);
(void) U_PMR_drawline(OBJ_PEN_BLACK_1,OBJ_PATH_1, (U_PMF_POINTF){x- 50, ry}, (U_PMF_POINTF){x- 50, ry + Height * ( 1.0 -  (((double) (fip->LineGap - fip->Descent)) / ((double) fip->EmSize)) )},  0, sum, et);
#endif

               Text16 = U_Utf8ToUtf16le(string, 0, NULL);
               slen = strlen(string);
               po = U_PMR_DRAWSTRING_set(FontID, BrushID, FormatID, slen, poRect, Text16);
               if(po){
                  U_PMR_write(po, sum, et);
                  status = 1;                                             /* Success!!!                                       */
               }
               U_PO_free(&poRect);
               free(Text16);
            }
         }
      }
      U_PO_free(&poFont);
   }
   free(UFontName);
   return(status);
}

/**
    \brief Allocate and construct an array of U_POINT16 objects from a set of U_PMF_POINTF objects, endianness in and out is LE
    \returns pointer to an array of U_POINT16 structures.
    \param points  pointer to the source U_POINT structures
    \param count   number of members in points

    If a coordinate is out of range it saturates at boundary.
*/
U_PMF_POINT *POINTF_To_POINT16_LE(U_PMF_POINTF *points, int count){
   U_PMF_POINT *newpts;
   U_PMF_POINTF ptfl;
   int i;
   newpts = (U_PMF_POINT *) malloc(count * sizeof(U_PMF_POINT));

   for(i=0; i<count; i++){
      memcpy(&ptfl, &(points[i]), 8);
      if(U_BYTE_SWAP){  U_swap4(&ptfl,2); } /* on BE platforms swap going in and coming out */
      newpts[i].X = U_MNMX(ptfl.X, INT16_MIN, INT16_MAX);
      newpts[i].Y = U_MNMX(ptfl.Y, INT16_MIN, INT16_MAX);
      if(U_BYTE_SWAP){  U_swap2(&(newpts[i]),2); }
   }
   return(newpts);
}

/**
    \brief Look up the name of the EMR+ record by type.  Returns U_EMR_INVALID if out of range.
        
    \return name of the PMR record, "U_EMR_INVALID" if out of range.
    \param idx  PMR record type WITHOUT the U_PMR_RECFLAG bit. 
    
*/
char *U_pmr_names(unsigned int idx){
   if(idx<U_PMR_MIN || idx > U_PMR_MAX){ idx = 0; }
   static char *U_PMR_NAMES[U_PMR_MAX+1]={
      "U_PMR_INVALID",          
      "U_PMR_Header",
      "U_PMR_EndOfFile",
      "U_PMR_Comment",
      "U_PMR_GetDC",
      "U_PMR_MultiFormatStart",
      "U_PMR_MultiFormatSection",
      "U_PMR_MultiFormatEnd",
      "U_PMR_Object",
      "U_PMR_Clear",
      "U_PMR_FillRects",
      "U_PMR_DrawRects",
      "U_PMR_FillPolygon",
      "U_PMR_DrawLines",
      "U_PMR_FillEllipse",
      "U_PMR_DrawEllipse",
      "U_PMR_FillPie",
      "U_PMR_DrawPie",
      "U_PMR_DrawArc",
      "U_PMR_FillRegion",
      "U_PMR_FillPath",
      "U_PMR_DrawPath",
      "U_PMR_FillClosedCurve",
      "U_PMR_DrawClosedCurve",
      "U_PMR_DrawCurve",
      "U_PMR_DrawBeziers",
      "U_PMR_DrawImage",
      "U_PMR_DrawImagePoints",
      "U_PMR_DrawString",
      "U_PMR_SetRenderingOrigin",
      "U_PMR_SetAntiAliasMode",
      "U_PMR_SetTextRenderingHint",
      "U_PMR_SetTextContrast",
      "U_PMR_SetInterpolationMode",
      "U_PMR_SetPixelOffsetMode",
      "U_PMR_SetCompositingMode",
      "U_PMR_SetCompositingQuality",
      "U_PMR_Save",
      "U_PMR_Restore",
      "U_PMR_BeginContainer",
      "U_PMR_BeginContainerNoParams",
      "U_PMR_EndContainer",
      "U_PMR_SetWorldTransform",
      "U_PMR_ResetWorldTransform",
      "U_PMR_MultiplyWorldTransform",
      "U_PMR_TranslateWorldTransform",
      "U_PMR_ScaleWorldTransform",
      "U_PMR_RotateWorldTransform",
      "U_PMR_SetPageTransform",
      "U_PMR_ResetClip",
      "U_PMR_SetClipRect",
      "U_PMR_SetClipPath",
      "U_PMR_SetClipRegion",
      "U_PMR_OffsetClip",
      "U_PMR_DrawDriverstring",
      "U_PMR_StrokeFillPath",
      "U_PMR_SerializableObject",
      "U_PMR_SetTSGraphics",
      "U_PMR_SetTSClip"
   };
   return(U_PMR_NAMES[idx]);
}

/**
    \brief Convert from PseudoObject OID to ObjectType enumeration.
    \returns OT value on success, 0 if no match
    \param OID   PseudoObject OID (based on EMF+ manual chapter number. )

    Only OTs that may be stored in the EMF+ object table are supported.
*/

int U_OID_To_OT(uint32_t OID){
   int otype;
   if(     OID==U_PMF_BRUSH_OID          ){ otype = U_OT_Brush;           }                    
   else if(OID==U_PMF_PEN_OID            ){ otype = U_OT_Pen;             }         
   else if(OID==U_PMF_PATH_OID           ){ otype = U_OT_Path;            }               
   else if(OID==U_PMF_REGION_OID         ){ otype = U_OT_Region;          }               
   else if(OID==U_PMF_IMAGE_OID          ){ otype = U_OT_Image;           }               
   else if(OID==U_PMF_FONT_OID           ){ otype = U_OT_Font;            }               
   else if(OID==U_PMF_STRINGFORMAT_OID   ){ otype = U_OT_StringFormat;    }               
   else if(OID==U_PMF_IMAGEATTRIBUTES_OID){ otype = U_OT_ImageAttributes; }               
   else if(OID==U_PMF_CUSTOMLINECAP_OID  ){ otype = U_OT_CustomLineCap;   } 
   else {                                   otype = U_OT_Invalid;         }          
   return(otype);
}

/**
    \brief Convert from PseudoObject OID to BrushType enumeration.
    \returns BT value on success, -1 if no match
    \param OID   PseudoObject OID (based on EMF+ manual chapter number. )

    Only OIDs that map to BT's are supported.
*/

int U_OID_To_BT(uint32_t OID){
   int otype;
   if(     OID==U_PMF_HATCHBRUSHDATA_OID          ){ otype = U_BT_HatchFill;      }                    
   else if(OID==U_PMF_LINEARGRADIENTBRUSHDATA_OID ){ otype = U_BT_LinearGradient; }               
   else if(OID==U_PMF_PATHGRADIENTBRUSHDATA_OID   ){ otype = U_BT_PathGradient;   }               
   else if(OID==U_PMF_SOLIDBRUSHDATA_OID          ){ otype = U_BT_SolidColor;     }               
   else if(OID==U_PMF_TEXTUREBRUSHDATA_OID        ){ otype = U_BT_TextureFill;    }               
   else {                                            otype = -1;                  }          
   return(otype);
}

/**
    \brief Convert from PseudoObject OID to CustomLineCapDataType Enumeration.
    \returns BT value on success, -1 if no match
    \param OID   PseudoObject OID (based on EMF+ manual chapter number. )

    Only OIDs that map to CLCDT's are supported.
*/

int U_OID_To_CLCDT(uint32_t OID){
   int otype;
   if(     OID==U_PMF_CUSTOMLINECAPDATA_OID       ){ otype = U_CLCDT_Default;         }                    
   else if(OID==U_PMF_CUSTOMLINECAPARROWDATA_OID ){  otype = U_CLCDT_AdjustableArrow; }               
   else {                                            otype = -1;                      }          
   return(otype);
}

/**
    \brief Convert from PseudoObject OID to ImageDataType Enumeration.
    \returns BT value on success, -1 if no match
    \param OID   PseudoObject OID (based on EMF+ manual chapter number. )

    Only OIDs that map to IDT's are supported.
*/

int U_OID_To_IDT(uint32_t OID){
   int otype;
   if(     OID==U_PMF_BITMAP_OID   ){ otype = U_IDT_Bitmap;   }                    
   else if(OID==U_PMF_METAFILE_OID ){ otype = U_IDT_Metafile; }               
   else {                             otype = -1;             }          
   return(otype);
}

/**
    \brief Convert from PseudoObject OID to RegionNodeDataType Enumeration.
    \returns BT value on success, -1 if no match
    \param OID   PseudoObject OID (based on EMF+ manual chapter number. )

    Only OIDs that map to RNDT's are supported.
*/

int U_OID_To_RNDT(uint32_t OID){
   int otype;
   if(     OID==U_PMF_REGIONNODECHILDNODES_OID   ){ otype = U_RNDT_Kids; }  /* there are 5 types, which must be specified separately */         
   else if(OID==U_PMF_RECTF_OID ){                  otype = U_RNDT_Rect; }               
   else if(OID==U_PMF_REGIONNODEPATH_OID ){         otype = U_RNDT_Path; }               
   else {                                           otype = -1;          }          
   return(otype);
}

/**
    \brief Append data to an U_OBJ_ACCUM structure.
    \param oa    pointer to the U_OBJ_ACCUM structure
    \param data  data to add
    \param size  bytes in data
    \param Type  object type
    \param Id    Object ID
    \returns 0 on success, !0 on error.  -1 on Type change, -2 on Id change
    
    Safe to test for Id, Type changes by calling with size=0.
*/
int U_OA_append(U_OBJ_ACCUM *oa, const char *data, int size, int Type, int Id){
   int tail;
   if(!oa)return(2);
   if(oa->used){
      if(oa->Type != Type)return(-1);
      if(oa->Id   != Id)return(-2);
   }
   tail = oa->used;
   if(oa->used + size >= oa->space){
      oa->space += size;
      char *newaccum = (char *) realloc(oa->accum, oa->space);
      if(!newaccum){
         oa->space -= size; /* put it back the way it was */
         return(1);
      }
      oa->accum = newaccum;
   }
   memcpy(oa->accum + tail,data,size);
   oa->used += size;
   oa->Type  = Type;
   oa->Id    = Id;
   return(0);
}


/**
    \brief Clear an U_OBJ_ACCUM structure.  Accumulated storage is retained.
    \param oa    pointer to the U_OBJ_ACCUM structure
    \returns 0 on success, !0 on error.
*/
int U_OA_clear(U_OBJ_ACCUM *oa){
   if(!oa)return(2);
   oa->used=0;
   /* Type and Id may be ignored as they are reset on the first append */
   return(0);
}

/**
    \brief Release an U_OBJ_ACCUM structure.  Accumulated storage is free'd.
    \param oa    pointer to the U_OBJ_ACCUM structure
    \returns 0 on success, !0 on error.
*/
int U_OA_release(U_OBJ_ACCUM *oa){
   if(!oa)return(2);
   oa->used=0;
   oa->space = 0;
   if(oa->accum)free(oa->accum);
   oa->accum=NULL;
   return(0);
}

/**
    \brief Create and set an U_PSEUDO_OBJ
    \returns pointer to the U_PSEUDO_OBJ, NULL on error
    \param Data  Data to copy into the PseudoObject's data. If NULL, space is allocated, but is cleared instead of filled.
    \param Size  Number of bytes to allocate for Data (may be >Use if padding is present) 
    \param Use   Number of data bytes in Data (whether or not Data is actually copied) 
    \param Type  Type numbers are from manual section: 1.2.3.47 -> 0x01020347 
    
    If Data is NULL and Size is 0 an empty PseudoObject is created.  One byte of storage
       is allocated for Data, Size is set to 1, and Used to 0.

    If Data is NULL and Size is !0 a zero filled PseudoObject is created.

    If Data is !Null and Size is !0 a data filled PseudoObject is created.
*/
U_PSEUDO_OBJ *U_PO_create(char *Data, size_t Size, size_t Use, uint32_t Type){
   if(Use>Size)return(NULL);
   size_t tSize = (Size ? Size : 1);
   U_PSEUDO_OBJ *po = (U_PSEUDO_OBJ *)malloc(sizeof(U_PSEUDO_OBJ));
   if(po){
      po->Data = malloc(tSize);
      if(po->Data){
         po->Size = tSize;
         po->Used = Use;
         po->Type = Type;
         if(Data){ memcpy(po->Data, Data, Use);   } /* if Use < Size uninitialized bytes will be present! */
         else {    memset(po->Data, 0,    tSize); }
      }
      else {
         free(po);
         po=NULL;
      }
   }
   return(po);
}

/**
    \brief Append data to a U_PSEUDO_OBJ object and return it
    \returns pointer to the U_PSEUDO_OBJ object, NULL on error
    \param po    PseudoObject to append to.  Cannot be NULL.
    \param Data  Data to copy into the PseudoObject's data. If NULL, space is allocated (if necessary) and cleared instead of filled.
    \param Size  Number of data bytes in Data
*/
U_PSEUDO_OBJ *U_PO_append(U_PSEUDO_OBJ *po, const char *Data, size_t Size){
   /* po cannot be NULL,as in U_PO_po_append(), because there would be no way to determine the TYPE of the resulting PO */
   if(po){
      if(!po->Data || po->Used + Size > po->Size){
         po->Size = po->Used + Size;
         char *newData = realloc(po->Data, po->Size);
         if(!newData){
            po->Size -= Size; /* put it back the way it was*/
            po=NULL;          /* skip the rest of the actions, does not affect po in caller */
         }
         else {
            po->Data = newData;
         }
      }
      if(po){ /* po->Data ready to append new data */
         if(Data){ memcpy(po->Data + po->Used, Data, Size); }
         else {    memset(po->Data + po->Used, 0,    Size); }
         po->Used += Size;
      }
   }
   return(po);
}

/**
    \brief Append data to a U_PSEUDO_OBJ object and return it
    \returns pointer to the U_PSEUDO_OBJ object, NULL on error
    \param po     PseudoObject to append to.  May be NULL.
    \param Src    PseudoObject to append.
    \param StripE Set: leading Elements in Src->Data is not copied, Clear: it is copied.
*/
U_PSEUDO_OBJ *U_PO_po_append(U_PSEUDO_OBJ *po, U_PSEUDO_OBJ *Src, int StripE){
   if(!Src){ return(NULL); }
   if((StripE && (Src->Used == 4)) || !Src->Used){ return(po); } /* appending nothing is not an error */
   char   *Data = Src->Data;
   size_t  Size = Src->Used;  /* append only what is used */
   U_PSEUDO_OBJ *ipo = po;
   if(StripE){ Size -= 4; }
   if(!ipo){
      ipo = U_PO_create(NULL, 0, 0, Src->Type); /* create an empty pseudoobject */
   }
   if(ipo){
      if(!ipo->Data || ipo->Used + Size > ipo->Size){
         ipo->Size = ipo->Used + Size;
         char *newData = realloc(ipo->Data, ipo->Size);
         if(!newData){
            if(ipo != po)U_PO_free(&ipo);
         }
         else {
            ipo->Data = newData;
         }
      }
      if(ipo){
         if(Data){
            if(StripE){  memcpy(ipo->Data + ipo->Used, Data + 4, Size); } /* Size is already 4 less, skip the leading Elements value */
            else {       memcpy(ipo->Data + ipo->Used, Data,     Size); } /* copy everything */
         }
         else {          memset(ipo->Data + ipo->Used, 0,        Size); } /* set everything  */
         ipo->Used += Size;
      }
   }
   return(ipo);
}

/**
    \brief Free an U_PSEUDO_OBJ structure.  All associated memory is released.
    \param po   Address of a pointer to the U_PSEUDO_OBJ structure, Pointer is set to NULL.
    \returns 1 on success, 0 on error.
*/
int U_PO_free(U_PSEUDO_OBJ **po){
   if(!po)return(0);
   if(!*po)return(1);
   if((*po)->Data)free((*po)->Data);
   free(*po);
   *po=NULL;
   return(1);
}

/** \brief create a PseudoObject with data in the correct byte order for an EMF+ file.
    \returns The PseudoObject on success, NULL on error.

    \param Type  the type of the PseudoObject that is created.  Allowed values are in U_PID_Values.
    \param List  an array of U_SERIAL_DESC structures containing the data to store.

      The U_PMF_SERIAL_set() function should not ever be called directly by end user code.
    
   Each U_SERIAL_DESC element in List consists of Data fields and a description of that data.  List is terminated
     by the first U_SERIAL_DESC element having a TE value of U_XX.
   
   Data fields: an array of a basic type of Units bytes repeated Reps times with the target byte order
      described in TE. 
   
   Ptrs:  Address of the first byte of the data fields.

   Units: Number of bytes of in each data field unit

   Reps:  Number of repeats of the unit in data fields.
           If a Ptr is NULL, and Units*Reps is not zero, then Units*Reps 0x00 bytes are stored.
           If a Ptr is NULL, and Units*Reps is zero, this U_SERIAL_DESC is ignored.
           if a Ptr is NOT NULL, and Units * Reps is not zero, then the data is stored in the indicated byte order.
           If a Ptr is NOT NULL, and Units or Reps is zero an error is signaled.

   TE:   (Target Endian) the byte order in which to store each unit of a data field as defined in U_Endian.
         Byte swapping is only enabled when Units is 2 or 4.  In addition to the byte order values U_XE, U_LE,
         and U_BE, and the array terminator U_XX, the value may also be U_RP.  U_RP means there is only a 
         single unit in the data fields, but it is to be copied to the target Reps times.  That is, the data
         was passed in with a form of run length encoding.

   Creates an empty PseudoObject if all pointers are NULL and all sizes are zero.
 */
U_PSEUDO_OBJ *U_PMF_SERIAL_set(uint32_t Type, const U_SERIAL_DESC *List){
   U_PSEUDO_OBJ        *po=NULL;
   size_t               Total=0;
   size_t               FSize;
   char                *cptr;
   char                *hptr;
   const U_SERIAL_DESC *lptr;
   if(!List)return(NULL);
   for(lptr=List; lptr->TE != U_XX; lptr++){
      if(!(lptr->Units * lptr->Reps) && lptr->Ptr)return(po);
      Total += lptr->Units * lptr->Reps;
   }
   po = U_PO_create(NULL, Total, Total, Type);
   if(po){
      cptr = po->Data;
      for(lptr=List; lptr->TE != U_XX; lptr++){
         FSize = lptr->Units * lptr->Reps;
         if(FSize){ /* Ptr is not NULL, that would have been detected already */
            hptr = cptr;
            if(lptr->TE & U_RP){  U_PMF_REPCPY_DSTSHIFT(&cptr, lptr->Ptr, lptr->Units, lptr->Reps); }
            else {                U_PMF_MEMCPY_DSTSHIFT(&cptr, lptr->Ptr, FSize);                   }
            if(((lptr->TE & U_LE) && U_IS_BE) || ((lptr->TE & U_BE) && U_IS_LE)){
               if(lptr->Units==2){      U_swap2(hptr,lptr->Reps); }
               else if(lptr->Units==4){ U_swap4(hptr,lptr->Reps); }
            }
         }
      }
   }
   return(po);
}

/**
    \brief Create U_DPSEUDO_OBJ's for the Points and Types of a path
    \param Elements Number of elements in Points.  May be zero, which creates an empty path.
    \param Points   Array of U_PMF_POINTF values.
    \param First    Apply to first point,      unsigned byte, lower 4 bits hold the PathPointType flag upper 4 bits hold the PathPointType enumeration. Must have U_PPT_Start set.
    \param Others   Apply to all other points, unsigned byte, lower 4 bits hold the PathPointType flag upper 4 bits hold the PathPointType enumeration. Must have U_PPT_Line or U_PPT_Bezier set.
    \returns pointer to the DU_PSEUDO_OBJ object, NULL on error
*/
U_DPSEUDO_OBJ *U_PATH_create(int Elements, const U_PMF_POINTF *Points, uint8_t First, uint8_t Others){
   if(Elements){
      if(!Points){                               return(NULL); }
      if( (First & U_PPT_MASK) != U_PPT_Start ){ return(NULL); }
      if(!(Others & U_PPT_Bezier)){              return(NULL); } /* will pass if either line or bezier is set */
   }
    
   U_DPSEUDO_OBJ *Path = (U_DPSEUDO_OBJ *)calloc(sizeof(U_DPSEUDO_OBJ),1); /* make poTypes and poPoints NULL */
   const U_SERIAL_DESC List[] = { {NULL,0,0,U_XX} };
   if(Path){
      Path->Elements = Elements;
      Path->poPoints = U_PMF_SERIAL_set(U_PMF_RAW_OID, List);  /* Empty PO to hold points as raw data */
      if(!Elements){
         Path->poTypes  = U_PMF_SERIAL_set(U_PMF_RAW_OID, List);  /* Empty PO to hold types as raw data */
      }
      else {
         Path->poPoints = U_PO_append(Path->poPoints, (char *)Points, Elements*sizeof(U_PMF_POINTF));
         if(Path->poPoints){
            U_PSEUDO_OBJ *tpo = U_PMF_PATHPOINTTYPE_set2(Elements, First | U_PPT_Start, Others);  /* PO holding types, has leading Elements value */
            Path->poTypes = U_PO_po_append(NULL, tpo, U_PMF_DROP_ELEMENTS); /* remove the leading Elements value*/
            U_PO_free(&tpo);
            if(!Path->poTypes){ U_PO_free(&Path->poPoints); }
         }
         if(!Path->poPoints){ U_DPO_free(&Path); }
      }
   }
   return(Path);
}

/**
    \brief Free U_DPSEUDO_OBJ's
    \returns 1 on success, 0 on error.
*/
int U_DPO_free(U_DPSEUDO_OBJ **dpo){
   if(!dpo){  return(0); }
   if(!*dpo){ return(1); }
   U_DPSEUDO_OBJ *kpo = *dpo;
   if(kpo->poPoints){ U_PO_free(&kpo->poPoints); }
   if(kpo->poTypes){  U_PO_free(&kpo->poTypes);  }
   free(*dpo);
   *dpo=NULL;
   return(1);
}

/**
    \brief Clear U_DPSEUDO_OBJ's.  Memory is retained, Elements and Used values are set to 0.
    \returns 1 on success, 0 on error.
    
    It is much more efficient to clear a DPO and reuse it than to free that DPO and create another.
*/
int U_DPO_clear(U_DPSEUDO_OBJ *dpo){
   if(!dpo){  return(0); }
   if(dpo->poPoints){ dpo->poPoints->Used = 0; }
   if(dpo->poTypes){  dpo->poTypes->Used  = 0; }
   dpo->Elements = 0;
   return(1);
}

/**
    \brief Append a "moveto" point to a path
    \param Path   Address of a DoublePseudoObject holding the path to append to.
    \param Point  Point to move to.
    \param Flags  Flags may be (U_PTP_None, U_PTP_DashMode, U_PTP_PathMarker, U_PTP_NoBit, U_PTP_CloseSubpath)
    \returns 1 on success, 0 on error.
*/
int U_PATH_moveto(U_DPSEUDO_OBJ *Path, U_PMF_POINTF Point, uint8_t Flags){
   if(!Path){ return(0); }
   U_PSEUDO_OBJ *tpo;
   U_PSEUDO_OBJ *tpo2;
   uint8_t Type = (Flags & U_PTP_NotClose) | U_PPT_Start;

   tpo = U_PMF_POINTF_set(1, &Point);
   if(!tpo){ return(0); }
   tpo2 = U_PO_po_append(Path->poPoints, tpo, U_PMF_DROP_ELEMENTS);
   U_PO_free(&tpo);
   if(!tpo2)return(0);
   Path->poPoints = tpo2;
   

   tpo  = U_PMF_PATHPOINTTYPE_set(1, &Type);
   if(!tpo){ return(0); }
   tpo2= U_PO_po_append(Path->poTypes, tpo, U_PMF_DROP_ELEMENTS);
   U_PO_free(&tpo);
   if(!tpo2)return(0);
   Path->poTypes = tpo2;

   Path->Elements++;
   return(1);
}

/**
    \brief Append a "lineto" point to a path
    \param Path    Address of a DoublePseudoObject holding the path to append to.
    \param Point   U_PMF_POINTF point to draw to.
    \param Flags  Flags may be (U_PTP_None, U_PTP_DashMode, U_PTP_PathMarker, U_PTP_NoBit, U_PTP_CloseSubpath)
    \returns 1 on success, 0 on error.
*/
int U_PATH_lineto(U_DPSEUDO_OBJ *Path,  U_PMF_POINTF Point, uint8_t Flags){
   if(!Path || !Path->Elements){ return(0); }  /* must be at least one point to extend from */
   if(Path->poTypes->Data[Path->Elements - 1] & U_PTP_CloseSubpath){ return(0); }  /* cannot extend a closed subpath */
   U_PSEUDO_OBJ *tpo;
   U_PSEUDO_OBJ *tpo2;
   uint8_t Type = (Flags & U_PTP_NotClose) | U_PPT_Line;
   tpo = U_PMF_POINTF_set(1, &Point);
   if(!tpo){ return(0); }
   tpo2 = U_PO_po_append(Path->poPoints, tpo, U_PMF_DROP_ELEMENTS);
   U_PO_free(&tpo);
   if(!tpo2)return(0);
   Path->poPoints  = tpo2;
   

   tpo  = U_PMF_PATHPOINTTYPE_set(1, &Type);
   if(!tpo){ return(0); }
   tpo2 = U_PO_po_append(Path->poTypes, tpo, U_PMF_DROP_ELEMENTS);
   U_PO_free(&tpo);
   if(!tpo2)return(0);
   Path->poTypes  = tpo2;

   Path->Elements++;
   return(1);
}

/**
    \brief Set the closepath bit in the last point
    \param Path    Address of a DoublePseudoObject holding the path to act upon.
    \returns 1 on success, 0 on error.
*/
int U_PATH_closepath(U_DPSEUDO_OBJ *Path){
   if(!Path || !Path->poTypes){ return(0); }
   uint32_t Elements = Path->Elements;
   uint8_t  *Type    = (uint8_t *)(Path->poTypes->Data) + Elements - 1;
   if(*Type & U_PPT_Start){ return(0); } /* single point closed path makes no sense */
   *Type =  *Type | U_PTP_CloseSubpath;
   return(1);
}

/**
    \brief Append a "polylineto" set of point to a path
    \param Path      Address of a DoublePseudoObject holding the path to append to.
    \param Elements  number of Points and Flags
    \param Points    Line points.
    \param Flags     Flags (U_PTP_None, U_PTP_DashMode, U_PTP_PathMarker, U_PTP_NoBit, but NOT U_PTP_CloseSubpath)
    \param StartSeg  If set, use U_PPT_Start PathPointType enumeration for first point, otherwise use U_PPT_Line.
    \returns 1 on success, 0 on error.
*/
int U_PATH_polylineto(U_DPSEUDO_OBJ *Path, uint32_t Elements, const U_PMF_POINTF *Points, uint8_t Flags, uint8_t StartSeg){
   if(!Path || !Points){ return(0); }
   if(!Elements){ return(1); } /* harmless - do nothing */
   U_PSEUDO_OBJ *tpo;
   U_PSEUDO_OBJ *tpo2;
   uint8_t First, Others;

   tpo = U_PMF_POINTF_set(Elements, Points);
   tpo2 = U_PO_po_append(Path->poPoints, tpo, U_PMF_DROP_ELEMENTS);
   U_PO_free(&tpo);
   if(!tpo2)return(0);
   Path->poPoints = tpo2;
   
   if(StartSeg){ First  = (Flags & U_PTP_NotClose) | U_PPT_Start; }
   else {        First  = (Flags & U_PTP_NotClose) | U_PPT_Line;  }
                 Others = (Flags & U_PTP_NotClose) | U_PPT_Line;  
   tpo  = U_PMF_PATHPOINTTYPE_set2(Elements, First, Others);
   if(!tpo){ return(0); }
   tpo2 = U_PO_po_append(Path->poTypes, tpo, U_PMF_DROP_ELEMENTS);
   U_PO_free(&tpo);
   if(!tpo2)return(0);
   Path->poTypes = tpo2;

   Path->Elements += Elements;
   return(1);
}


/**
    \brief Append a "polybezierto" set of point to a path
    \param Path      Address of a DoublePseudoObject holding the path to append to.
    \param Elements  number of Points
    \param Points    Bezier points.  Optional starting point, then N sets of 3, example: [P1] (Q12A Q12B P2) (Q23A Q23B P3).
    \param Flags     Flags (U_PTP_None, U_PTP_DashMode, U_PTP_PathMarker, U_PTP_NoBit, but NOT U_PTP_CloseSubpath)
    \param StartSeg  If set, use U_PPT_Start PathPointType enumeration for first point, otherwise use U_PPT_Bezier.
    \returns 1 on success, 0 on error.
    
    If Start is set   Elements must be 1 + multiple of 3. Ie, P1 Q12A Q12B P2 Q23A Q23B P3

    If Start is clear Elements must be a multiple of 3.   Ie, (P1, already in path) Q12A Q12B P2 Q23A Q23B P3
*/
int U_PATH_polybezierto(U_DPSEUDO_OBJ *Path, uint32_t Elements, const U_PMF_POINTF *Points, uint8_t Flags, uint8_t StartSeg){
   if(!Path || !Points){                  return(0); }
   if(!Elements){
      if(StartSeg){                       return(0); }    /* cannot have both a NEW segment and ZERO points */
      else{                               return(1); }    /* harmless - do nothing */
   }
   if(StartSeg  && ((Elements - 1) % 3)){ return(0); }    /* new segment    must be 1 + N*3 points */
   if(!StartSeg && (Elements % 3)){       return(0); }    /* extend segment must be     N*3 points */
   U_PSEUDO_OBJ *tpo;
   U_PSEUDO_OBJ *tpo2;
   uint8_t First, Others;

   tpo = U_PMF_POINTF_set(Elements, Points);
   tpo2 = U_PO_po_append(Path->poPoints, tpo, U_PMF_DROP_ELEMENTS);
   U_PO_free(&tpo);
   if(!tpo2)return(0);
   Path->poPoints = tpo2;
 
   if(StartSeg){ First  = (Flags & U_PTP_NotClose) | U_PPT_Start;  }
   else {        First  = (Flags & U_PTP_NotClose) | U_PPT_Bezier; }
                 Others = (Flags & U_PTP_NotClose) | U_PPT_Bezier;
   tpo  = U_PMF_PATHPOINTTYPE_set2(Elements, First, Others);
   if(!tpo){ return(0); }
   tpo2 = U_PO_po_append(Path->poTypes, tpo, U_PMF_DROP_ELEMENTS);
   U_PO_free(&tpo);
   if(!tpo2)return(0);
   Path->poTypes = tpo2;

   Path->Elements += Elements;
   return(1);
}

/**
    \brief Append a "polygon" set of points to a path.
    \param Path      Address of a DoublePseudoObject holding the path to append to.
    \param Elements  number of Points and Flags
    \param Points    Line points.
    \param Flags     Flags (U_PTP_None, U_PTP_DashMode, U_PTP_PathMarker, U_PTP_NoBit, but NOT U_PTP_CloseSubpath)
    \returns 1 on success, 0 on error.
*/
int U_PATH_polygon(U_DPSEUDO_OBJ *Path, uint32_t Elements, const U_PMF_POINTF *Points, uint8_t Flags){
   int status = U_PATH_polylineto(Path, Elements, Points, Flags, U_SEG_NEW);
   if(status){
       status = U_PATH_closepath(Path);
   }
   return(status);
}

//! \cond
// These are not exposed in the API
/* Parameterized Ellipse coordinates */
U_PMF_POINTF U_eparam(U_FLOAT a, U_FLOAT b, U_PMF_POINTF *center, double Ang, double Theta){
   U_PMF_POINTF point;
   point.X = center->X + a*cos(Theta)*cos(Ang) - b*sin(Theta)*sin(Ang);
   point.Y = center->Y + a*sin(Theta)*cos(Ang) + b*cos(Theta)*sin(Ang);
   return(point);
}

/* Parameterized Ellipse derivative  */
U_PMF_POINTF U_eparam2(U_FLOAT a, U_FLOAT b, double Ang, double Theta){
   U_PMF_POINTF point;
   point.X = -a*cos(Theta)*sin(Ang) - b*sin(Theta)*cos(Ang);
   point.Y = -a*sin(Theta)*sin(Ang) + b*cos(Theta)*cos(Ang);
   return(point);
}

double U_aparam(double Ang1, double Ang2){
   double Alpha;
   double t2;
   t2 = tan((Ang2 - Ang1)/2.0);
   t2 *= t2;
   Alpha = sin(Ang2 - Ang1) * (sqrt(4 + 3*t2) -1.0)/3.0;
   return(Alpha);
}

/* Parameterized Bezier point Q1 or Q2  derivative  */
U_PMF_POINTF U_qparam(double Alpha, double a, double b, U_PMF_POINTF *Point, double Ang, double Theta, int mode){
   U_PMF_POINTF Q, Prime;
   Prime = U_eparam2(a,b,Ang,Theta);
   if(mode==1){ /* Q1, anything else is Q2*/
      Q.X = Point->X + Alpha * Prime.X;
      Q.Y = Point->Y + Alpha * Prime.Y;
   }
   else {
      Q.X = Point->X - Alpha * Prime.X;
      Q.Y = Point->Y - Alpha * Prime.Y;
   }
   return(Q);
}
//! \endcond

/**
    \brief Append an "arcto" set of points to a path (Bezier points are calculated, and these are appended
    \param Path      Address of a pointer to the U_PSEUDO_OBJ that holds points
    \param Start     Start angle, >=0.0, degrees clockwise from 3:00
    \param Sweep     Sweep angle, -360<= angle <=360, degrees clockwise from Start
    \param Rot       Rotation angle to apply to coordinate system (Start and Rect), positive is degrees clockwise
    \param Rect      U_PMF_RECTF that defines the bounding rectangle.
    \param Flags     Flags (U_PTP_None, U_PTP_DashMode, U_PTP_PathMarker, U_PTP_NoBit, but NOT U_PTP_CloseSubpath)
    \param StartSeg  If set, the arc starts a new segment, if clear, continue the existing segment.  Starting a new segment does not automatically apply U_PATH_closepath to the existing path.
    \returns 1 on success, 0 on error.
    
    Based on Luc Maisonobe's work, http://www.spaceroots.org/documents/ellipse/
*/
int U_PATH_arcto(U_DPSEUDO_OBJ *Path, U_FLOAT Start, U_FLOAT Sweep, U_FLOAT Rot, U_PMF_RECTF *Rect, uint8_t Flags, int StartSeg){
   U_PMF_POINTF Bz[3];
   U_PMF_POINTF Center;
   U_PMF_POINTF P1,P2;
   double a, b;
   int done  = 0;
   int fpoint = 0;
   double L1, L2;     /* These are degrees CounterClockwise from 3:00 */
   double Ang1, Ang2; /* These are parametric angles, derived from L1, L2*/
   double Alpha;      /* Dimensionless number used for Q1, Q2 */
   double Theta;      /* Rot in radians */
   double Slop;       /* do not let rounding errors cause spurious end points */
   if(!Path){   return(0); }
   if((Sweep > 360.0) || (Sweep < -360.0)){ return(0); }
   /* Start should be between 0 and 360 degrees, but it does not really matter because sin, and cos will accept anything */
   /* the sign on Sweep and Start is correct bacause LM's derivation has y positive up, but GDI+ has y positive down. */
   a  = Rect->Width/2.0;
   b  = Rect->Height/2.0;
   if(!a || !b){ return(0); }
   Center.X = Rect->X + a;
   Center.Y = Rect->Y + b;
   /* convert to radians */
   Start = (2.0 * U_PI * Start)/360.0;
   Sweep = (2.0 * U_PI * Sweep)/360.0;
   Theta = (2.0 * U_PI * Rot  )/360.0;
   Slop  = Sweep/100000.0;
   L1 = Start;

   while(!done){
      if(Sweep < 0){
         L2 = L1 - U_PI/2.0;
         if(L2 <= Sweep + Start - Slop){ L2 = Sweep + Start; done = 1; }
         else {done = 0; }
      }
      else {
         L2 = L1 + U_PI/2.0;
         if(L2 >= Sweep + Start + Slop){ L2 = Sweep + Start; done = 1; }
         else {done = 0; }
      }
      Ang1 = atan2(sin(L1)/b, cos(L1)/a);
      Ang2 = atan2(sin(L2)/b, cos(L2)/a);
      Alpha = U_aparam(Ang1, Ang2);
      P1    = U_eparam(a, b, &Center, Ang1, Theta);        /* P1 */
      P2    = U_eparam(a, b, &Center, Ang2, Theta);        /* P2 */
      Bz[0] = U_qparam(Alpha, a, b, &P1, Ang1, Theta, 1);  /* Q1 */
      Bz[1] = U_qparam(Alpha, a, b, &P2, Ang2, Theta, 2);  /* Q2 */
      Bz[2] = P2;
      if(!fpoint){
         if(StartSeg){ U_PATH_moveto(Path, P1, Flags); }
         else {        U_PATH_lineto(Path, P1, Flags); }
         fpoint = 1;
      }
      U_PATH_polybezierto(Path, 3, Bz, Flags, U_SEG_OLD );
      L1 = L2;
   }
   return(1);
}

/**
    \brief Allocate and construct an array of U_PMF_POINTF objects which have been subjected to a U_XFORM
    \returns pointer to an array of U_PMF_POINTF structures.
    \param points  pointer to the source U_PMF_POINTF structures
    \param count   number of members in points
    \param xform   U_XFORM to apply
    
*/
U_PMF_POINTF *pointfs_transform(U_PMF_POINTF *points, int count, U_XFORM xform){
   U_PMF_POINTF *newpts=NULL;;
   int i;
   float X,Y;
   newpts = (U_PMF_POINTF *) malloc(count * sizeof(U_PMF_POINTF));
   if(newpts){
      for(i=0; i<count; i++){
         X = points[i].X;
         Y = points[i].Y;
         newpts[i].X = U_ROUND(X * xform.eM11 + Y * xform.eM21 + xform.eDx);
         newpts[i].Y = U_ROUND(X * xform.eM12 + Y * xform.eM22 + xform.eDy);
      }
   }
   return(newpts);
}

/**
    \brief Allocate and construct an array of U_PMF_RECTF objects which have been subjected to a U_XFORM
    \returns pointer to an array of U_PMF_RECTF structures.
    \param Rects   pointer to the source U_PMF_RECTF structures
    \param Count   number of members in Rects
    \param Xform   U_XFORM to apply.  Rotation is ignored, only translation is applied.
    
*/
U_PMF_RECTF *rectfs_transform(U_PMF_RECTF *Rects, int Count, U_XFORM Xform){
   U_PMF_RECTF *newRects;
   int i;
   newRects = (U_PMF_RECTF *) malloc(Count * sizeof(U_PMF_RECTF));
   if(newRects){
      for(i=0; i<Count; i++){
         newRects[i].X      = U_ROUND(Rects[i].X + Xform.eDx);
         newRects[i].Y      = U_ROUND(Rects[i].Y + Xform.eDy);
         newRects[i].Width  = U_ROUND(Rects[i].Width);
         newRects[i].Height = U_ROUND(Rects[i].Height);
      }
   }
   return(newRects);
}

/**
    \brief  Utility function calculate the transformation matrix needed to make a gradient run precisely corner to corner of a rectangle
    \param  Angle   Rotation in degrees clockwise of the gradient. 0 is horizontal gradient.
    \param  w       Width of the rectangle
    \param  h       Height of the rectangle
    \param  x       X coordinate of upper left corner of rectangle
    \param  y       Y coordinate of upper left corner of rectangle
    \param  Periods Periods of gradient corner to corner.  1.0 is one, corner to corner.
    \return Transformation matrix.  All values are zero if Periods, w, or h are less than or equal to zero.
*/
U_PMF_TRANSFORMMATRIX tm_for_gradrect(U_FLOAT Angle, U_FLOAT w, U_FLOAT h, U_FLOAT x, U_FLOAT y, U_FLOAT Periods){
//! \cond
#define CLOSE_TO_IS_REALLY_ZERO(A) ((A) > 1.0e-10 || (A) < -1.0e-10 ? (A) : 0.0) //! \hideinitializer 
//! \endcond
   U_PMF_TRANSFORMMATRIX tm;
   double dang = Angle * 2*U_PI /360.0;
   double scale;
   double cd,sd;
   if((Periods <=0.0) || (w <= 0.0) || (h <= 0.0)){
      tm.m11 = tm.m12 = tm.m21 = tm.m22 = tm.dX = tm.dY = 0.0;
   }
   else {
      /*
        scale is gradient period divided by w
        The scale value sets the gradient period to match exactly with the inscribed (w,h) rectangle
          in the direction specified by the angle.
        The numberator of scale is the max of the four dot product values of the rotated X basis unit vector with (w,h),
          with each of the vectors {w,h}, {-w,h}, {-w,-h}, {w,h}.  Those vectors run from each corner in turn
          to the opposite corner.  The one most parallel to the rotated unit vector will have both terms positive.
          
        Trig results like cos(pi/2) are not stable between platforms due to minor differences in the
          implementation. Detect these and make them zero, which then allows binary comparison of output files.
          Otherwise the binary comparisons between platforms would fail because of a bunch of insignificant digits.
      */
      cd = CLOSE_TO_IS_REALLY_ZERO(cos(dang));
      sd = CLOSE_TO_IS_REALLY_ZERO(sin(dang));
      scale = (w*fabs(cd) + h*fabs(sd))/(w*Periods);
      tm.m11 =  scale * cd;
      tm.m12 = -scale * sd;                
      tm.m21 =  scale * sd;                
      tm.m22 =  scale * cd; 
      /* offset is to one corner of the square, depending on which quadrant the rotation selects. */
      if(cos(dang)>=0){
         tm.dX = x;
         if(sin(dang)>=0){tm.dY = y + h;  } // LL corner
         else {           tm.dY = y;      } // UL corner
      }
      else {
         tm.dX = x + w;
         if(sin(dang)>=0){ tm.dY = y + h; } // LR corner
         else {            tm.dY = y;     } // UR corner
      }
   }
   return tm; 
#undef CLOSE_TO_IS_REALLY_ZERO
}
/**
    \brief Create a U_PSEUDO_OBJ containing a U_PMR_FILLPATH and U_PMR_DRAWPATH records.
    \returns pointer to U_PSEUDO_OBJ or NULL on error.
    \param  PathID      U_PMF_PATH object in the EMF+ object table (0-63, inclusive)
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
    \param  BrushID     U_PSEUDO_OBJ containing a U_PMF_ARGB or a U_PMF_4NUM. Color or U_PMF_BRUSH object in the EMF+ object table (0-63, inclusive)
*/
U_PSEUDO_OBJ *U_PMR_drawfill(uint32_t PathID, uint32_t PenID, const U_PSEUDO_OBJ *BrushID){
   U_PSEUDO_OBJ *po = U_PMR_FILLPATH_set(PathID, BrushID);
   if(po){
      U_PSEUDO_OBJ *tpo = U_PMR_DRAWPATH_set(PathID, PenID);
      po = U_PO_po_append(po, tpo, U_PMF_KEEP_ELEMENTS);
      U_PO_free(&tpo);
   }
   return(po);
}



/**
   \brief Extract a single data field from a source.
   \returns 1 on success, 0 on error.
   \param Src   where the data is coming from.  It is incremented by the number of bytes retrieved.
   \param Dst   where the data will be stored.  This must either be NULL (in which case the Src
           is advanced and nothing is stored, or it must be allocated to Reps * Units bytes!!!!
   \param Units number of bytes in each unit of the data field
   \param Reps  number of repeats of units in the data field.
           If a Ptr is NULL, then Units*Reps 0 bytes are stored.
           If a Ptr is NOT NULL, and Units or Reps, is zero an error is signaled.
           If a Ptr is NULL and Units*Reps is 0, nothing happens.
   \param SE    logical (Source Endian). Only relevant for Sizes of 2 or 4 
         Indicates when Bytes may need to be rearranged when they are retrieved.
         U_XE no change   (this is used when the data has already been set to the proper orientation or it is not known)
         U_LE source is Little Endian
         U_BE source is Big Endian.
         U_XX error
    
*/
int U_PMF_SERIAL_get(const char **Src, void *Dst, size_t Units, size_t Reps, int SE){
   if(!Src || !*Src || SE == U_XX){ return(0); }
   U_PMF_MEMCPY_SRCSHIFT(Dst, Src, Units * Reps);
   if(!Dst){                  return(1); } /* "fake" get, no data was retrieved, so we are done */
   if(SE == U_XE){            return(1); }
   if(SE == U_LE && U_IS_LE){ return(1); }
   if(SE == U_BE && U_IS_BE){ return(1); }
   /* need to swap */
   if(     Units==2){ U_swap2(Dst,Reps); }
   else if(Units==4){ U_swap4(Dst,Reps); }
   return(1);
}

/**
      \brief Conditionally extract an array of data from a source, allocating space to hold it.
      \returns 1 on success, 0 on error.
      \param Src   where the data is coming from.  It is incremented by the number of bytes retrieved.
      \param Dst   Caller must free.  Where the pointer to the data will be stored.  Reps * Units bytes will be allocated,
      \param Units number of bytes in each unit of the data field
      \param Reps  number of repeats of units in the data field.
           If a Ptr is NULL, then Units*Reps 0 bytes are stored.
           If a Ptr is NOT NULL, and Units or Reps, is zero an error is signaled.
           If a Ptr is NULL and Units*Reps is 0, nothing happens.
      \param SE    logical (Source Endian). Only relevant for Sizes of 2 or 4 
         Indicates when Bytes may need to be rearranged when they are retrieved.
         U_XE no change   (this is used when the data has already been set to the proper orientation or it is not known)
         U_LE source is Little Endian
         U_BE source is Big Endian.
         U_XX error
      \param Cond  Store the data into *Dst if true, set *Dst to NULL otherwise.
    
*/
int U_PMF_SERIAL_array_copy_get(const char **Src, void **Dst, size_t Units, size_t Reps, int SE, int Cond){
   if(!Src || !*Src || !Dst || SE == U_XX){ return(0); }
   if(!Cond){
      *Src += Units * Reps;
      *Dst = NULL;
      return(1);
   }
   *Dst = malloc(Units * Reps);
   if(!*Dst){                 return(1); } /* "fake" get, no data was retrieved, so we are done */
   U_PMF_MEMCPY_SRCSHIFT(*Dst, Src, Units * Reps);
   if(SE == U_XE){            return(1); }
   if(SE == U_LE && U_IS_LE){ return(1); }
   if(SE == U_BE && U_IS_BE){ return(1); }
   /* need to swap */
   if(     Units==2){ U_swap2(*Dst,Reps); }
   else if(Units==4){ U_swap4(*Dst,Reps); }
   return(1);
}



/**
    \brief Calculate the length in bytes of a relative path object composed of U_PMF_INTEGER7 and U_PMF_INTER15 values
    \return >=0 length == success, <0  error
    \param  contents   Start of a relative path consisting of int7 and int15 X,Y pairs.
    \param  Elements   number of relative X,Y pairs in the object
*/
int U_PMF_LEN_REL715(const char *contents, int Elements){
   int length=0;
   Elements *= 2; /* N pairs = 2N values */
   for( ; Elements; Elements--){
      /* X or Y value */
      if(*contents & U_TEST_INT7){ contents +=2; length +=2; } //int15
      else {                       contents +=1; length +=1; } //int7
   }
   return(length);
}

/**
    \brief Calculate the length in bytes of objects which are a 4 byte Count followed by Count * float bytes
    \return >=0 length == success, <0  error
    Object types whose size may be derived with this function are:
       U_PMF_COMPOUNDLINEDATA
       U_PMF_DASHEDLINEDATA    
*/
int U_PMF_LEN_FLOATDATA(const char *contents){
   int Size;
   U_PMF_SERIAL_get(&contents, &Size, 4, 1, U_LE);
   Size = 4*Size + 4;
   return(Size);
}

/**
    \brief Calculate the length in bytes of objects which are a 4 byte count followed by count bytes
    \return >=0 length == success, <0  error
    Object types whose size may be derived with this function are:
       U_PMF_BOUNDARYPATHDATA
       U_PMF_BOUNDARYPOINTDATA
       U_PMF_CUSTOMSTARTCAPDATA
       U_PMF_PATH
       U_PMF_LINEPATH
       U_PMF_REGIONNODEPATH
*/
int U_PMF_LEN_BYTEDATA(const char *contents){
   int Size;
   U_PMF_SERIAL_get(&contents, &Size, 4, 1, U_LE);
   Size += 4;
   return(Size);
}

/**
    \brief Create a string containing the curly bracket form of the 16 byte GUID value
    \return number of bytes in record, 0 on error
    \param  GUID   pointer to the 16 unsigned bytes
    \return string in curly bracket form.
    http://msdn.microsoft.com/en-us/library/cc230316.aspx

    Text form is Data1-Data2-Data3-Data4, the first 3 are stored as little endian integers, the last as a string (big endian).
*/
char *U_PMF_CURLYGUID_set(uint8_t *GUID){
   char *string=malloc(64);
   if(string){
      sprintf(string,"{%2.2X%2.2X%2.2X%2.2X-%2.2X%2.2X-%2.2X%2.2X-%2.2X%2.2X-%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X}",         
         GUID[3],GUID[2],GUID[1],GUID[0],
         GUID[5],GUID[4],
         GUID[7],GUID[6],
         GUID[8],GUID[9],
         GUID[10],GUID[11],GUID[12],GUID[13],GUID[14],GUID[15]
      );
   }
   return(string);
}

/**
    \brief Identify a known curly GUID
    \param  string   Curly GUID form.
    \return EmageEffects Enumerator 

    EMF+ manual 2.1.3.1, Microsoft name: ImageEffects Identifier
*/
int U_PMF_KNOWNCURLYGUID_set(const char *string){
   int status; 
   if(     !strcmp(string,"{633C80A4-1843-482B-9EF2-BE2834C5FDD4}")){ status = U_IEE_BlurEffectGuid;                  }                                                     
   else if(!strcmp(string,"{D3A1DBE1-8EC4-4C17-9F4C-EA97AD1C343D}")){ status = U_IEE_BrightnessContrastEffectGuid;    }
   else if(!strcmp(string,"{537E597D-251E-48DA-9664-29CA496B70F8}")){ status = U_IEE_ColorBalanceEffectGuid;          }
   else if(!strcmp(string,"{DD6A0022-58E4-4A67-9D9B-D48EB881A53D}")){ status = U_IEE_ColorCurveEffectGuid;            }
   else if(!strcmp(string,"{A7CE72A9-0F7F-40D7-B3CC-D0C02D5C3212}")){ status = U_IEE_ColorLookupTableEffectGuid;      }
   else if(!strcmp(string,"{718F2615-7933-40E3-A511-5F68FE14DD74}")){ status = U_IEE_ColorMatrixEffectGuid;           }
   else if(!strcmp(string,"{8B2DD6C3-EB07-4D87-A5F0-7108E26A9C5F}")){ status = U_IEE_HueSaturationLightnessEffectGuid;}
   else if(!strcmp(string,"{99C354EC-2A31-4F3A-8C34-17A803B33A25}")){ status = U_IEE_LevelsEffectGuid;                }
   else if(!strcmp(string,"{74D29D05-69A4-4266-9549-3CC52836B632}")){ status = U_IEE_RedEyeCorrectionEffectGuid;      }
   else if(!strcmp(string,"{63CBF3EE-C526-402C-8F71-62C540BF5142}")){ status = U_IEE_SharpenEffectGuid;               }
   else if(!strcmp(string,"{1077AF00-2848-4441-9489-44AD4C2D7A2C}")){ status = U_IEE_TintEffectGuid;                  }
   else {                                                             status = U_IEE_Unknown;                         }
   return(status);
}

/** \brief Load a GUID from text format into EMF+ file binary format.
    \param  string  Curly GUID as text, minus the barckets and dashes.
    \return GUID in EMF+ file binary format.


This accepts a string that is 16 bytes long = 32 characters hex (no dash spaces or brackets) as text.  
Text form is; Data1|Data2|Data3|Data4, first three are stored as little endian integers of 4,2,2 bytes, respectively,
last is stored like a string (big endian), after conversion from text hex to binary.

This function is not normally called by end user code.
*/
uint8_t *U_LOAD_GUID(char *string){
   uint32_t Data1,tData2,tData3,tByte;
   uint16_t Data2,Data3;
   char    *Data4 = string + 16;
   uint8_t *hold  = malloc(16);
   char    *lf    = (char *) hold;
   int      i;
   if(hold){
      Data1=tData2=tData3=0;               
      if(3 != sscanf(string +  0,"%8X",&Data1)  + 
              sscanf(string +  8,"%4X",&tData2) + 
              sscanf(string + 12,"%4X",&tData3)){
         free(hold);
         hold = NULL;
         goto bye;
      }
      Data2=tData2;
      Data3=tData3;
      U_PMF_MEMCPY_DSTSHIFT(&lf, &Data1, 4);
      U_PMF_MEMCPY_DSTSHIFT(&lf, &Data2, 2);
      U_PMF_MEMCPY_DSTSHIFT(&lf, &Data3, 2);
      if(U_IS_BE){ /* these fields are stored little endian */
         U_swap4(hold,1);
         U_swap2(hold+4,2);
      }
      /* remainder is converted byte by byte and stored in that order */
      for(i=0;i<8;i++,Data4+=2,lf++){
         if(1 != sscanf(Data4,"%2X",&tByte)){
            free(hold);
            hold = NULL;
            goto bye;
         }
         *lf=tByte;
      }
   }
bye:
   return(hold);
}

/**
    \brief Generate the 16 byte form from OID of the ImageEffects Identifier
    \param  OID  OID of the ImageEffects Identifier
    \return pointer to 16 byte buffer holding the long GUID binary form, or NULL on error.

    EMF+ manual 2.1.3.1, Microsoft name: ImageEffects Identifier
*/
uint8_t *U_OID_To_GUID(uint32_t OID){
   uint8_t *lf = NULL;
   if(     OID == U_PMF_IE_BLUR_OID                  ){ lf = U_LOAD_GUID("633C80A41843482B9EF2BE2834C5FDD4"); } 
   else if(OID == U_PMF_IE_BRIGHTNESSCONTRAST_OID    ){ lf = U_LOAD_GUID("D3A1DBE18EC44C179F4CEA97AD1C343D"); } 
   else if(OID == U_PMF_IE_COLORBALANCE_OID          ){ lf = U_LOAD_GUID("537E597D251E48DA966429CA496B70F8"); } 
   else if(OID == U_PMF_IE_COLORCURVE_OID            ){ lf = U_LOAD_GUID("DD6A002258E44A679D9BD48EB881A53D"); } 
   else if(OID == U_PMF_IE_COLORLOOKUPTABLE_OID      ){ lf = U_LOAD_GUID("A7CE72A90F7F40D7B3CCD0C02D5C3212"); } 
   else if(OID == U_PMF_IE_COLORMATRIX_OID           ){ lf = U_LOAD_GUID("718F2615793340E3A5115F68FE14DD74"); } 
   else if(OID == U_PMF_IE_HUESATURATIONLIGHTNESS_OID){ lf = U_LOAD_GUID("8B2DD6C3EB074D87A5F07108E26A9C5F"); } 
   else if(OID == U_PMF_IE_LEVELS_OID                ){ lf = U_LOAD_GUID("99C354EC2A314F3A8C3417A803B33A25"); } 
   else if(OID == U_PMF_IE_REDEYECORRECTION_OID      ){ lf = U_LOAD_GUID("74D29D0569A4426695493CC52836B632"); } 
   else if(OID == U_PMF_IE_SHARPEN_OID               ){ lf = U_LOAD_GUID("63CBF3EEC526402C8F7162C540BF5142"); } 
   else if(OID == U_PMF_IE_TINT_OID                  ){ lf = U_LOAD_GUID("1077AF0028484441948944AD4C2D7A2C"); }
   return(lf);
}

/**
    \brief copy data and shift source pointer by the amount of data moved
    \param Dst Destination in memory
    \param Src Source in memory
    \param Size Number of bytes to move
*/
void U_PMF_MEMCPY_SRCSHIFT(void *Dst, const char **Src, size_t Size){
   if(Dst)memcpy(Dst, *Src, Size);
   *Src += Size;
}

/**
    \brief copy data and shift destination pointer by the amount of data moved
    \param Dst Destination in memory (this must not be NULL)
    \param Src Source in memory (if this is NULL, fill with that many zero bytes instead)
    \param Size Number of bytes to move
*/
void U_PMF_MEMCPY_DSTSHIFT(char **Dst, const void *Src, size_t Size){
   if(Src){ memcpy(*Dst, Src, Size); }
   else {   memset(*Dst, 0,   Size); }
   *Dst += Size;
}

/**
    \brief Copy the single instance at Src repeatedly to Dst.
    \param Dst   Destination in memory
    \param Src   Source in memory (if this is NULL, fill with that many zero bytes instead)
    \param Size  number of bytes in single instance that is template.
    \param Reps  Number of instances of the template to opy
*/
void U_PMF_REPCPY_DSTSHIFT(char **Dst, const void *Src, size_t Size, size_t Reps){
   for(;Reps;Reps--){
      if(Src){ memcpy(*Dst, Src, Size); }
      else {   memset(*Dst, 0,   Size); }
      *Dst += Size;
   }
}

/**
    \brief save pointer to data and shift source pointer by the amount of data moved
    \param Dst Destination in memory
    \param Src Source in memory or NULL. If NULL Dst is set to NULL.
    \param Size Number of bytes to move
*/
void U_PMF_PTRSAV_SHIFT(const char **Dst, const char **Src, size_t Size){
   if(*Src){
      if(Dst)*Dst = *Src;
      *Src += Size;
   }
   else {
      if(Dst)*Dst = NULL;
   }
}

/**
    \brief save pointer to data and shift source pointer by the amount of data moved
    \return 1 on sucess, 0 on error
    \param Dst Destination in memory
    \param Src Source in memory or NULL. If NULL Dst is set to NULL.
    \param Doit Assign if true, otherwise, set to NULL
*/
int U_PMF_PTRSAV_COND(const char **Dst, const char *Src, int Doit){
   if(!Dst){ return(0); }
   if(Src && Doit){ *Dst = Src;  }
   else {           *Dst = NULL; }
   return(1);
}
/*

   =====================================================================================
   start of U_PMF_*_get() functions 

*/

/**
    \brief Get the 16 bit unsigned Flags field from a header.
    \param  contents   Record from which to extract data, will be incremented by header size.
    \return Flags field

    In many records the only value needed from the header is Flags.  Rather than mapping
    the entire Header and returning it, in these instances this function may be called to
    just get this one value.
*/
uint16_t U_PMF_HEADERFLAGS_get(const char *contents){
   uint16_t Flags;
   const char *cptr = contents + offsetof(U_PMF_CMN_HDR,Flags);
   U_PMF_SERIAL_get(&cptr, &Flags,  2, 1, U_LE); /* EMF+ manual documents it as BE, but this code implements it as LE*/
   return(Flags);
}

/**
    \brief Retrieve whichever header fields are requested.  NULL pointers do not retrieve.
    \param  contents   Record from which to extract data, will be incremented by header size.
    \param  Type       Record type
    \param  Flags      Record flags
    \param  Size       Records size
    \param  Datasize   Data size
    \return 1 on success, 0 on failure.
*/
int U_PMF_HEADERFIELDS_get(const char *contents,
   uint16_t *Type, uint16_t *Flags, uint32_t *Size, uint32_t *Datasize){
   if(!contents){ return(0); }
   U_PMF_SERIAL_get(&contents, Type,     2, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Flags,    2, 1, U_LE); /* EMF+ manual documents it as BE, but this code implements it as LE*/
   U_PMF_SERIAL_get(&contents, Size,     4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Datasize, 4, 1, U_LE);
   return(1);
}

/**
    \brief Get the entire EMF+ record header.
    \param  contents   Record from which to extract data, will be offset by header size.
    \param  Header     Location to store data (may be NULL)
    \returns 1
    If Header is Null, nothing is stored but contents is still offset.
*/
int U_PMF_CMN_HDR_get(const char **contents, U_PMF_CMN_HDR *Header){
   if(!contents || !*contents){ return(0); }
   if(Header){
      U_PMF_SERIAL_get(contents, &(Header->Type),     2, 1, U_LE);
      U_PMF_SERIAL_get(contents, &(Header->Flags),    2, 1, U_LE); /* EMF+ manual documents it as BE, but this code implements it as LE*/
      U_PMF_SERIAL_get(contents, &(Header->Size),     4, 1, U_LE);
      U_PMF_SERIAL_get(contents, &(Header->DataSize), 4, 1, U_LE);
   }
   else {
      *contents += sizeof(U_PMF_CMN_HDR);
   }
   return(1);
}

/**
    \brief return the size in bytes of the EMF+ record
    \param  contents   Record from which to extract data, will not be modified.
    \returns size, or 0 if contents is Null
*/
int U_PMF_RECORD_SIZE_get(const char *contents){
   if(!contents){ return(0); }
   int Size;
   const char *from = contents + 4;
   U_PMF_SERIAL_get(&from, &Size, 4, 1, U_LE);
   return(Size);
}

/**
    \brief Return the size of a PenData object from an EMF+ record.
    \param  PenData   Address in memory where the PenData object starts.
    \returns size of the object in bytes
*/
int U_PMF_LEN_PENDATA(const char *PenData){
   uint32_t Flags;
   int length=12;  /* Flags, Unit, Width */
   U_PMF_SERIAL_get(&PenData, &Flags,  4, 1, U_LE);
   PenData += 8;  /* skip Unit and Width */
   length  += U_PMF_LEN_OPTPENDATA(PenData, Flags);
   return(length);
}

/**
    \brief Return the size of an OptPenData object from an EMF+ record.
    \param  PenData   Address in memory where the PenData object starts.
    \param  Flags     PenData Flags that indicate which fields are present.
    \returns size of the object in bytes
*/
int U_PMF_LEN_OPTPENDATA(const char *PenData, uint32_t Flags){
   int length=0;
   if(Flags & U_PD_Transform){       length += sizeof(U_PMF_TRANSFORMMATRIX);           }
   if(Flags & U_PD_StartCap){        length += sizeof(int32_t);                         }
   if(Flags & U_PD_EndCap){          length += sizeof(int32_t);                         }
   if(Flags & U_PD_Join){            length += sizeof(uint32_t);                        }
   if(Flags & U_PD_MiterLimit){      length += sizeof(U_FLOAT);                         }
   if(Flags & U_PD_LineStyle){       length += sizeof(int32_t);                         }
   if(Flags & U_PD_DLCap){           length += sizeof(int32_t);                         }
   if(Flags & U_PD_DLOffset){        length += sizeof(int32_t);                         }
   if(Flags & U_PD_DLData){          length += U_PMF_LEN_FLOATDATA(PenData + length);   }
   if(Flags & U_PD_NonCenter){       length += sizeof(int32_t);                         }
   if(Flags & U_PD_CLData){          length += U_PMF_LEN_FLOATDATA(PenData + length);   }
   if(Flags & U_PD_CustomStartCap){  length += U_PMF_LEN_BYTEDATA(PenData + length);    }
   if(Flags & U_PD_CustomEndCap){    length += U_PMF_LEN_BYTEDATA(PenData + length);    }
   return(length);
}

/**
    \brief  Create and set a U_PMF_BRUSH PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Version    EmfPlusGraphicsVersion object
    \param  Bd         U_PSEUDO_OBJ containing one of the 5 types of Brush data

    EMF+ manual 2.2.1.1, Microsoft name: EmfPlusBrush Object
*/
U_PSEUDO_OBJ *U_PMF_BRUSH_set(uint32_t Version, const U_PSEUDO_OBJ *Bd){
   if(!Bd){ return(NULL); }
   int32_t Type = U_OID_To_BT(Bd->Type);
   if(Type < 0){ return(NULL); }
   const U_SERIAL_DESC List[] = {
      {&Version, 4,        1, U_LE},
      {&Type,    4,        1, U_LE},
      {Bd->Data, Bd->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_BRUSH_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_CUSTOMLINECAP PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Version    EmfPlusGraphicsVersion object
    \param  Ld         U_PSEUDO_OBJ containing one of the 2 types of Linecap data

    EMF+ manual 2.2.1.2, Microsoft name: EmfPlusCustomLineCap Object
*/
U_PSEUDO_OBJ *U_PMF_CUSTOMLINECAP_set(uint32_t Version, const U_PSEUDO_OBJ *Ld){
   if(!Ld){ return(NULL); }
   int32_t Type = U_OID_To_CLCDT(Ld->Type);
   if(Type<0){ return(NULL); }
   const U_SERIAL_DESC List[] = {
      {&Version, 4,        1, U_LE},
      {&Type,    4,        1, U_LE},
      {Ld->Data, Ld->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_CUSTOMLINECAP_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_FONT PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Version    EmfPlusGraphicsVersion object
    \param  EmSize     em size in units of SizeUnit
    \param  SizeUnit   UnitType enumeration
    \param  FSFlags    FontStyle flags
    \param  Length     Number of Unicode Characters in FamilyName
    \param  Font       Unicode (UTF-16LE) fontname

    EMF+ manual 2.2.1.3, Microsoft name: EmfPlusFont Object
*/
U_PSEUDO_OBJ *U_PMF_FONT_set(uint32_t Version, U_FLOAT EmSize, uint32_t SizeUnit,
      int32_t FSFlags, uint32_t Length, const uint16_t *Font){
   uint32_t cbFont = 2 * wchar16len(Font); /* this need not be 2*Length parameter */
   uint32_t pad = (0x3 & cbFont ? 2 : 0);
   const U_SERIAL_DESC List[] = {
      {&Version, 4,      1,             U_LE},
      {&EmSize,  4,      1,             U_LE},
      {&SizeUnit,4,      1,             U_LE},
      {&FSFlags, 4,      1,             U_LE},
      {NULL,     4,      1,             U_LE},  /* NULL is for Reserved field */
      {&Length,  4,      1,             U_LE},
      {Font,     cbFont, 1,             U_LE},
      {NULL,     pad,    (pad ? 1 : 0), (pad ? U_XE : U_XX)}, /* Entire record must be a multiple of 4 */
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_FONT_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_IMAGE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Version    EmfPlusGraphicsVersion object
    \param  Id         U_PSEUDO_OBJ containing one of the 2 types of image data

    EMF+ manual 2.2.1.4, Microsoft name: EmfPlusImage Object
*/
U_PSEUDO_OBJ *U_PMF_IMAGE_set(uint32_t Version, const U_PSEUDO_OBJ *Id){
   if(!Id){ return(NULL); }
   int32_t Type = U_OID_To_IDT(Id->Type);
   if(Type<0){ return(NULL);}
   const U_SERIAL_DESC List[] = {
      {&Version, 4,        1, U_LE},
      {&Type,    4,        1, U_LE},
      {Id->Data, Id->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_IMAGE_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_IMAGEATTRIBUTES PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Version     EmfPlusGraphicsVersion object
    \param  WrapMode    WrapMode object
    \param  ClampColor  EmfPlusARGB object
    \param  ObjectClamp ObjectClamp Identifiers

    EMF+ manual 2.2.1.5, Microsoft name: EmfPlusImageAttributes Object
*/
U_PSEUDO_OBJ *U_PMF_IMAGEATTRIBUTES_set(uint32_t Version, uint32_t WrapMode, uint32_t ClampColor, uint32_t ObjectClamp){
   uint32_t Reserved=0;
   const U_SERIAL_DESC List[] = {
      {&Version,     4, 1, U_LE},
      {&Reserved,    4, 1, U_LE},
      {&WrapMode,    4, 1, U_LE},
      {&ClampColor,  4, 1, U_LE},
      {&ObjectClamp, 4, 1, U_LE},
      {&Reserved,    4, 1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_IMAGEATTRIBUTES_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_PATH PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Version    EmfPlusGraphicsVersion object
    \param  Points     U_PSEUDO_OBJ containing array of points (of type PMFPointR, PMFPoint, or PMFPointF, determined by U_PPF_P and U_PPF_C bits in Flags)
    \param  Types      U_PSEUDO_OBJ containing array of types (U_PMF_PATHPOINTTYPE or U_PMF_PATHPOINTTYPERLE, determined by  U_PPF_R big in Flags)

    EMF+ manual 2.2.1.6, Microsoft name: EmfPlusPath Object
*/
U_PSEUDO_OBJ *U_PMF_PATH_set(uint32_t Version, const U_PSEUDO_OBJ *Points, const U_PSEUDO_OBJ *Types){
   int ctype, RelAbs, rtype;
   int pad;
   if(Points){
      if(     Points->Type == U_PMF_POINTR_OID){                               RelAbs = 1; ctype = 0; }
      else if(Points->Type == (U_PMF_POINT_OID  | U_PMF_ARRAY_OID)){           RelAbs = 0; ctype = 1; }
      else if(Points->Type == (U_PMF_POINTF_OID | U_PMF_ARRAY_OID)){           RelAbs = 0; ctype = 0; }
      else {                                                                   return(NULL);          }
   }
   else {                                                                      return(NULL);          }
   if(Types){
      if(     Types->Type  == (U_PMF_PATHPOINTTYPERLE_OID | U_PMF_ARRAY_OID)){ rtype = 1;             }
      else if(Types->Type  == (U_PMF_PATHPOINTTYPE_OID | U_PMF_ARRAY_OID)){    rtype = 0;             }
      else {                                                                   return(NULL);          }
   }
   else {                                                                      return(NULL);          }
   uint16_t Flags = (rtype ? U_PPF_R : 0) | (ctype ? U_PPF_C : 0)| (RelAbs ? U_PPF_P : 0);
   pad = (0x3 & (Points->Used + Types->Used));
   if(pad){ pad = 4 - pad; }
   const U_SERIAL_DESC List[] = {
      {&Version,         4,                1,             U_LE               },
      {Points->Data,     4,                1,             U_XE               }, /* Elements from Points */
      {&Flags,           2,                1,             U_LE               },
      {NULL,             2,                1,             U_LE               }, /* Reserved field */
      {Points->Data + 4, Points->Used - 4, 1,             U_XE               }, /* omit Points Elements */
      {Types->Data +4,   Types->Used - 4,  1,             U_XE               }, /* omit Types  Elements */
      {NULL,             pad,              (pad ? 1 : 0), (pad ? U_XE : U_XX)}, /* if no padding is needed the Units will be zero and nothing will happen */
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_PATH_OID, List);
   return(po);
}


/**
    \brief  Create and set a U_PMF_PATH PseudoObject that uses U_PMF_POINTF coordinates
    \return Pointer to PseudoObject, NULL on error
    \param  Version    EmfPlusGraphicsVersion object
    \param  Path       U_DPSEUDO_OBJ containing a path.

    EMF+ manual 2.2.1.6, Microsoft name: EmfPlusPath Object
*/
U_PSEUDO_OBJ *U_PMF_PATH_set2(uint32_t Version, const U_DPSEUDO_OBJ *Path){
   if(!Path || !Path->Elements){ return(NULL); }
   uint16_t Flags = 0;
   int pad = (0x3 & Path->Elements);
   if(pad){ pad = 4 - pad; }
   const U_SERIAL_DESC List[] = {
      {&Version,               4,                1,               U_LE               },
      {&Path->Elements,        4,                1,               U_LE               },
      {&Flags,                 2,                1,               U_LE               },
      {NULL,                   2,                1,               U_LE               }, /*  Reserved field */
      {Path->poPoints->Data,   4,                2*Path->Elements,U_XE               }, /*  raw OID, so no leading Elements to omit */
      {Path->poTypes->Data,    1,                Path->Elements,  U_XE               }, /*  raw OID, so no leading Elements to omit */
      {NULL,                   pad,              (pad ? 1 : 0),   (pad ? U_XE : U_XX)}, /* if no padding is needed the Units will be zero and nothing will happen */
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_PATH_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_PATH PseudoObject that uses U_PMF_POINT (int 16) coordinates
    \return Pointer to PseudoObject, NULL on error
    \param  Version    EmfPlusGraphicsVersion object
    \param  Path       U_DPSEUDO_OBJ containing a path.

    EMF+ manual 2.2.1.6, Microsoft name: EmfPlusPath Object
*/
U_PSEUDO_OBJ *U_PMF_PATH_set3(uint32_t Version, const U_DPSEUDO_OBJ *Path){
   if(!Path || !Path->Elements){return(NULL);  }
   uint16_t Flags = U_PPF_C;
   int pad = (0x3 & Path->Elements);
   if(pad){ pad = 4 - pad; }
   U_PMF_POINT *Points16 = POINTF_To_POINT16_LE((U_PMF_POINTF *)Path->poPoints->Data, Path->Elements);
   if(!Points16){ return(NULL); }
   const U_SERIAL_DESC List[] = {
      {&Version,               4,                1,               U_LE               },
      {&Path->Elements,        4,                1,               U_LE               },
      {&Flags,                 2,                1,               U_LE               },
      {NULL,                   2,                1,               U_LE               }, /*  Reserved field */
      {Points16,               2,                2*Path->Elements,U_XE               }, /*  raw data, so no leading Elements to omit */
      {Path->poTypes->Data,    1,                Path->Elements,  U_XE               }, /*  raw data, so no leading Elements to omit */
      {NULL,                   pad,              (pad ? 1 : 0),   (pad ? U_XE : U_XX)}, /* if no padding is needed the Units will be zero and nothing will happen */
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_PATH_OID, List);
   free(Points16);
   return(po);
}

/**
    \brief  Create and set a U_PMF_PEN PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Version    EmfPlusGraphicsVersion object
    \param  PenData    U_PSEUDO_OBJ containing U_PMF_PENDATA object
    \param  Brush      U_PSEUDO_OBJ containing U_PMF_BRUSH object 

    EMF+ manual 2.2.1.7, Microsoft name: EmfPlusPen Object
*/
U_PSEUDO_OBJ *U_PMF_PEN_set(uint32_t Version, const U_PSEUDO_OBJ *PenData, const U_PSEUDO_OBJ *Brush){
   if(!PenData   || (PenData->Type    != U_PMF_PENDATA_OID)){ return(NULL); }
   if(!Brush     || (Brush->Type      != U_PMF_BRUSH_OID)  ){ return(NULL); }
   const U_SERIAL_DESC List[] = {
      {&Version,        4,             1, U_LE},
      {NULL,            4,             1, U_LE},
      {PenData->Data,   PenData->Used, 1, U_XE},
      {Brush->Data,     Brush->Used,   1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_PEN_OID, List);
   return(po);
}


/**
    \brief  Create and set a U_PMF_REGION PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Version    EmfPlusGraphicsVersion object
    \param  Count      Number of CHILD nodes.  This is one less than the total number of U_PMF_REGIONNODE objects in Nodes.
    \param  Nodes      U_PSEUDO_OBJ containing U_PMF_REGIONNODE object (Nodes defining region, may be a single element or a binary tree) 

    EMF+ manual 2.2.1.8, Microsoft name: EmfPlusRegion Object
*/
U_PSEUDO_OBJ *U_PMF_REGION_set(uint32_t Version, uint32_t Count, const U_PSEUDO_OBJ *Nodes){
   if(!Nodes || Nodes->Type  != U_PMF_REGIONNODE_OID)return(NULL);
   const U_SERIAL_DESC List[] = {
      {&Version,    4,           1, U_LE},
      {&Count,      4,           1, U_LE},
      {Nodes->Data, Nodes->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_REGION_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_STRINGFORMAT PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Sfs       pointer to U_PMF_STRINGFORMAT structure, with no variable part
    \param  Sfd       (optional) U_PSEUDO_OBJ containing U_PMF_STRINGFORMATDATA object

    EMF+ manual 2.2.1.9, Microsoft name: EmfPlusStringFormat Object
*/
U_PSEUDO_OBJ *U_PMF_STRINGFORMAT_set(U_PMF_STRINGFORMAT *Sfs, const U_PSEUDO_OBJ *Sfd){
   if(!Sfs){ return(NULL); }
   if(Sfd){
     if((!Sfs->TabStopCount && !Sfs->RangeCount) || (Sfd->Type  != U_PMF_STRINGFORMATDATA_OID))return(NULL);
   }
   else {
     if(Sfs->TabStopCount || Sfs->RangeCount)return(NULL);
   }
   const U_SERIAL_DESC List[] = {
      {&Sfs->Version,            4,                     1, U_LE},
      {&Sfs->Flags,              4,                     1, U_LE},
      {&Sfs->Language,           4,                     1, U_LE},
      {&Sfs->StringAlignment,    4,                     1, U_LE},
      {&Sfs->LineAlign,          4,                     1, U_LE},
      {&Sfs->DigitSubstitution,  4,                     1, U_LE},
      {&Sfs->DigitLanguage,      4,                     1, U_LE},
      {&Sfs->FirstTabOffset,     4,                     1, U_LE},
      {&Sfs->HotkeyPrefix,       4,                     1, U_LE},
      {&Sfs->LeadingMargin,      4,                     1, U_LE},
      {&Sfs->TrailingMargin,     4,                     1, U_LE},
      {&Sfs->Tracking,           4,                     1, U_LE},
      {&Sfs->Trimming,           4,                     1, U_LE},
      {&Sfs->TabStopCount,       4,                     1, U_LE},
      {&Sfs->RangeCount,         4,                     1, U_LE},
      {(Sfd ? Sfd->Data : NULL), (Sfd ? Sfd->Used : 0), 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_STRINGFORMAT_OID, List);
   return(po);
}

/**
    \brief  Create and set a PMF_4NUM PseudoObject (used for BrushID's)
    \return Pointer to PseudoObject, NULL on error
    \param  BrushID     U_PMF_BRUSH object in the EMF+ object table (0-63, inclusive)
   
*/
U_PSEUDO_OBJ *U_PMF_4NUM_set(uint32_t BrushID){
   if(BrushID>63){ return(NULL); }
   const U_SERIAL_DESC List[] = {
      {&BrushID, 4, 1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_4NUM_OID, List);
   return(po);
}

/**
    \brief Create and set a U_PMF_ARGB PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Alpha      Alpha       (0-255)
    \param  Red        Red   color (0-255)
    \param  Green      Green color (0-255)
    \param  Blue       Blue  color (0-255)

    EMF+ manual 2.2.2.1, Microsoft name: EmfPlusARGB Object
*/
U_PSEUDO_OBJ *U_PMF_ARGB_set(uint8_t Alpha, uint8_t Red, uint8_t Green, uint8_t Blue){
   const U_SERIAL_DESC List[] = {
      {&Blue,  1, 1, U_XE},
      {&Green, 1, 1, U_XE},
      {&Red,   1, 1, U_XE},
      {&Alpha, 1, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_ARGB_OID, List);
   return(po);
}

/**
    \brief Create and set an Array of U_PMF_ARGB valus in a PseudoObject
    \return Pointer to PseudoObject containing the count, followed by the array of colors, NULL on error
    \param  Count      Number of entries in Colors
    \param  Colors     Array of ARGB values

    EMF+ manual 2.2.2.1, Microsoft name: EmfPlusARGB Object
*/
U_PSEUDO_OBJ *U_PMF_ARGBN_set(uint32_t Count, U_PMF_ARGB *Colors){
   const U_SERIAL_DESC List[] = {
      {&Count, 4, 1,     U_LE},
      {Colors, 4, Count, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_ARGB_OID | U_PMF_ARRAY_OID, List);
   return(po);
}

/**
    \brief Set a U_PMF_ARGB object
    \return Object
    \param  Alpha      Alpha       (0-255)
    \param  Red        Red   color (0-255)
    \param  Green      Green color (0-255)
    \param  Blue       Blue  color (0-255)

    EMF+ manual 2.2.2.1, Microsoft name: EmfPlusARGB Object
*/
U_PMF_ARGB U_PMF_ARGBOBJ_set(uint8_t Alpha, uint8_t Red, uint8_t Green, uint8_t Blue){
   U_PMF_ARGB argb;
   char *ptr = (char *) &argb;
   U_PMF_MEMCPY_DSTSHIFT(&ptr, &Blue,  1);
   U_PMF_MEMCPY_DSTSHIFT(&ptr, &Green, 1);
   U_PMF_MEMCPY_DSTSHIFT(&ptr, &Red,   1);
   U_PMF_MEMCPY_DSTSHIFT(&ptr, &Alpha, 1);
   return(argb);
}

/**
    \brief  Create and set a U_PMF_BITMAP PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Bs       pointer to U_PMF_BITMAP structure, with no variable part
    \param  Bm       U_PSEUDO_OBJ containing an U_PMF_BITMAPDATA or U_PMF_COMPRESSEDIMAGE object 

    EMF+ manual 2.2.2.2, Microsoft name: EmfPlusBitmap Object
*/
U_PSEUDO_OBJ *U_PMF_BITMAP_set(const U_PMF_BITMAP *Bs, const U_PSEUDO_OBJ *Bm){
   if(!Bs)return(NULL);
   if(Bm->Type  != U_PMF_BITMAPDATA_OID &&
      Bm->Type  != U_PMF_COMPRESSEDIMAGE_OID )return(NULL);
   uint32_t Pad = UP4(Bm->Used) - Bm->Used;  /* undocumented padding, must be present for at least PNG */
   const U_SERIAL_DESC List[] = {
      {Bs,       4,               5,             U_LE},
      {Bm->Data, Bm->Used,        1,             U_XE},
      {NULL,     (Pad ? Pad : 0), (Pad ? 1 : 0), U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_BITMAP_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_BITMAPDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Ps     (optional) U_PSEUDO_OBJ containing a U_PMF_PALETTE structure
    \param  cbBm   Bytes in Bm
    \param  Bm     An array of bytes, meaning depends on fields in U_PMF_BITMAP object and the PixelFormat enumeration.

    EMF+ manual 2.2.2.3, Microsoft name: EmfPlusBitmapData Object
*/
U_PSEUDO_OBJ *U_PMF_BITMAPDATA_set( const U_PSEUDO_OBJ *Ps, int cbBm, const char *Bm){
   if(Ps && (Ps->Type  != U_PMF_PALETTE_OID))return(NULL);
   if(!Bm && cbBm)return(NULL);
   const U_SERIAL_DESC List[] = {
      {(Ps ? Ps->Data : NULL), (Ps  ? Ps->Used : 0), (Ps ? 1 : 0), U_LE},
      {Bm,                     cbBm,                 1,            U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_BITMAPDATA_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_BLENDCOLORS PseudoObject
    \return Pointer to PseudoObject, NULL on Positions and Colors
    \param  Elements   number of elements in Positions, must agree with the number of Colors.
    \param  Positions  positions along gradient line. The first position MUST be 0.0 and the last MUST be 1.0.
    \param  Colors     U_PSEUDO_OBJ containing an array of U_PMF_ARGB objects: object colors at positions on gradient line

    EMF+ manual 2.2.2.4, Microsoft name: EmfPlusBlendColors Object
*/
U_PSEUDO_OBJ *U_PMF_BLENDCOLORS_set(uint32_t Elements, const U_FLOAT *Positions, const U_PSEUDO_OBJ *Colors){
   if(!Colors || !Positions || Colors->Type != (U_PMF_ARGB_OID | U_PMF_ARRAY_OID)){ return(NULL); }
   uint32_t CElements = (Colors->Used - 4)/4;
   if(CElements != Elements){ return(NULL); }
   const U_SERIAL_DESC List[] = {
      {&CElements,       4,                1,          U_LE},
      {Positions,        4,                CElements,  U_LE},
      {Colors->Data + 4, Colors->Used - 4, 1,          U_XE}, /* omit Elements part of this PseudoObject */
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_BLENDCOLORS_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_BLENDCOLORS PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Elements      members in osition, inluding Start and End (0.0 - 1.0)
    \param  StartColor    Start Color (U_PMF_ARGB)
    \param  EndColor      End Color (U_PMF_ARGB)

    EMF+ manual 2.2.2.5, Microsoft name: EmfPlusBlendFactors Object

    
    Positions always start at 0.0 and always end at 1.0.  It is not well documented but other
    start and end values generally do not work.
*/
U_PSEUDO_OBJ *U_PMF_BLENDCOLORS_linear_set(uint32_t Elements, U_PMF_ARGB StartColor, U_PMF_ARGB EndColor){
   double dP,dR,dG,dB,dA,P,R,G,B,A;
   U_FLOAT  StartPos = 0.0;
   U_FLOAT  EndPos = 1.0;
   U_FLOAT *Positions;
   U_FLOAT *pP;
   U_PMF_ARGB *Colors;
   U_PMF_ARGB *pC;
   unsigned int i;
   if(Elements <= 2 ){ return(NULL); }
   pP = Positions = (U_FLOAT *)malloc(Elements *sizeof(U_FLOAT));
   if(!Positions){ return(NULL); }
   pC = Colors    = (U_PMF_ARGB *)malloc(Elements *sizeof(U_PMF_ARGB));
   if(!Colors){
      free(Positions); 
      return(NULL);
   }
   dP = (EndPos    - StartPos   )/(float)(Elements - 1);
   dB = ((double)EndColor.Blue  - (double)StartColor.Blue )/(double)(Elements - 1);
   dG = ((double)EndColor.Green - (double)StartColor.Green)/(double)(Elements - 1);
   dR = ((double)EndColor.Red   - (double)StartColor.Red  )/(double)(Elements - 1);
   dA = ((double)EndColor.Alpha - (double)StartColor.Alpha)/(double)(Elements - 1);
   P  = StartPos;
   B  = StartColor.Blue; 
   G  = StartColor.Green; 
   R  = StartColor.Red; 
   A  = StartColor.Alpha; 
   for(i=0;i<Elements;i++,pC++,pP++){ /* hopefully the rounding errors are not a problem, used doubles to minimize that */
      *pP  = P;  
      P   += dP;
      *pC  = (U_PMF_ARGB){B,G,R,A};
      B   += dB;
      G   += dG;
      R   += dR;
      A   += dA;
   }
   U_PSEUDO_OBJ *poColors = U_PMF_ARGBN_set(Elements, Colors);
   U_PSEUDO_OBJ *po = U_PMF_BLENDCOLORS_set(Elements, Positions, poColors);
   U_PO_free(&poColors);
   free(Positions);
   free(Colors);
   return(po);
}


/**
    \brief  Create and set a U_PMF_BLENDFACTORS PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Elements   members in each array
    \param  Positions  positions along gradient line.  The first position MUST be 0.0 and the last MUST be 1.0.
    \param  Factors    blending factors, 0.0->1.0 values, inclusive

    EMF+ manual 2.2.2.5, Microsoft name: EmfPlusBlendFactors Object
*/
U_PSEUDO_OBJ *U_PMF_BLENDFACTORS_set(uint32_t Elements, const U_FLOAT *Positions, const U_FLOAT *Factors){
   if(!Positions || !Factors)return(NULL);
   const U_SERIAL_DESC List[] = {
      {&Elements, 4, 1,        U_LE},
      {Positions, 4, Elements, U_LE},
      {Factors,   4, Elements, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_BLENDFACTORS_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_BLENDFACTORS PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Elements      members in osition, inluding Start and End (0.0 - 1.0)
    \param  StartFactor   Start Factor (0.0 - 1.0)
    \param  EndFactor     End Factor (0.0 - 1.0)

    EMF+ manual 2.2.2.5, Microsoft name: EmfPlusBlendFactors Object

    
    Positions always start at 0.0 and always end at 1.0.  It is not well documented but other
    start and end values generally do not work.
*/
U_PSEUDO_OBJ *U_PMF_BLENDFACTORS_linear_set(uint32_t Elements, U_FLOAT StartFactor, U_FLOAT EndFactor){
   double dP,dF,P,F;
   U_FLOAT  StartPos = 0.0;
   U_FLOAT  EndPos = 1.0;
   U_FLOAT *Positions;
   U_FLOAT *Factors;
   U_FLOAT *pP;
   U_FLOAT *pF;
   unsigned int i;
   if(Elements <= 2 ){ return(NULL); }
   pP = Positions = (U_FLOAT *)malloc(Elements *sizeof(U_FLOAT));
   if(!Positions){ return(NULL); }
   pF = Factors   = (U_FLOAT *)malloc(Elements *sizeof(U_FLOAT));
   if(!Factors){
      free(Positions); 
      return(NULL);
   }
   dP = (EndPos    - StartPos   )/(float)(Elements - 1);
   dF = (EndFactor - StartFactor)/(float)(Elements - 1);
   P  = StartPos;
   F  = StartFactor;
   for(i=0;i<Elements;i++){ /* hopefully the rounding errors are not a problem, used doubles to minimize that */
      *pP++ = P;  P += dP;
      *pF++ = F;  F += dF;
   }
   U_PSEUDO_OBJ *po = U_PMF_BLENDFACTORS_set(Elements, Positions, Factors);
   free(Positions);
   free(Factors);
   return(po);
}

/**
    \brief  Create and set a U_PMF_BOUNDARYPATHDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Path       U_PSEUDO_OBJ containing U_PMF_PATH object

    EMF+ manual 2.2.2.6, Microsoft name: EmfPlusBoundaryPathData Object
*/
U_PSEUDO_OBJ *U_PMF_BOUNDARYPATHDATA_set(const U_PSEUDO_OBJ *Path){
   if(!Path || Path->Type  != U_PMF_PATH_OID)return(NULL);
   /* PO Used is size_t, might be 8 bytes, value in record must be 4 bytes */
   uint32_t Used = Path->Used;
   const U_SERIAL_DESC List[] = {
      {&Used,      4,          1, U_LE},
      {Path->Data, Path->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_BOUNDARYPATHDATA_OID, List);
   return(po);
}


/**
    \brief  Create and set a U_PMF_BOUNDARYPOINTDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Elements   members in each array
    \param  Points     array of U_PMF_POINTF

    EMF+ manual 2.2.2.7, Microsoft name: EmfPlusBoundaryPointData Object
*/
U_PSEUDO_OBJ *U_PMF_BOUNDARYPOINTDATA_set(uint32_t Elements, const U_PMF_POINTF *Points){
   if(!Points)return(NULL);
   const U_SERIAL_DESC List[] = {
      {&Elements, 4, 1,         U_LE},
      {Points,    4, 2*Elements,U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_BOUNDARYPOINTDATA_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_CHARACTERRANGE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  First      First position in range
    \param  Length     Range length

    EMF+ manual 2.2.2.8, Microsoft name: EmfPlusCharacterRange Object
*/
U_PSEUDO_OBJ *U_PMF_CHARACTERRANGE_set(int32_t First, int32_t Length){
   const U_SERIAL_DESC List[] = {
      {&First,  4, 1, U_LE},
      {&Length, 4, 1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_CHARACTERRANGE_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_COMPOUNDLINEDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Elements   Members in Widths
    \param  Widths     Array of U_FLOAT Line or gap widths (0.0 <-> 1.0, fraction of total line width )

    EMF+ manual 2.2.2.9, Microsoft name: EmfPlusCompoundLineData Object
*/
U_PSEUDO_OBJ *U_PMF_COMPOUNDLINEDATA_set(int32_t Elements, const char *Widths){
   if(!Widths)return(NULL);
   const U_SERIAL_DESC List[] = {
      {&Elements ,4, 1,       U_LE},
      {Widths,    4, Elements,U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_COMPOUNDLINEDATA_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_COMPRESSEDIMAGE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  cbImage     Bytes in Image
    \param  Image       Stored image in one of the supported formats (GIF, PNG, etc.).

    EMF+ manual 2.2.2.10, Microsoft name: EmfPlusCompressedImage Object
*/
U_PSEUDO_OBJ *U_PMF_COMPRESSEDIMAGE_set(int32_t cbImage, const char *Image){
   if(!cbImage || !Image)return(NULL);
   const U_SERIAL_DESC List[] = {
      {Image,    cbImage, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_COMPRESSEDIMAGE_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_CUSTOMENDCAPDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Clc       U_PSEUDO_OBJ containing a U_PMF_CUSTOMLINECAP object

    EMF+ manual 2.2.2.11, Microsoft name: EmfPlusCustomEndCapData Object
*/
U_PSEUDO_OBJ *U_PMF_CUSTOMENDCAPDATA_set(const U_PSEUDO_OBJ *Clc){
   if(!Clc || Clc->Type  != U_PMF_CUSTOMLINECAP_OID)return(NULL);
   /* PO Used is size_t, might be 8 bytes, value in record must be 4 bytes */
   uint32_t Used = Clc->Used;
   const U_SERIAL_DESC List[] = {
      {&Used,      4,         1, U_LE},
      {Clc->Data,  Clc->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_CUSTOMENDCAPDATA_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_CUSTOMLINECAPARROWDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Width       Arrow cap width (is multiplied by line width before draw)
    \param  Height      Arrow cap length (is multiplied by line width before draw)
    \param  MiddleInset Pixels between outer edge and filled region
    \param  FillState   If set, fill, otherwise, only border
    \param  StartCap    LineCap enumeration (type of cap)
    \param  EndCap      LineCap enumeration
    \param  Join        LineJoin enumeration
    \param  MiterLimit  Maximum (miter length / line width)
    \param  WidthScale  Scale for U_PMF_CUSTOMLINECAP object

    EMF+ manual 2.2.2.12, Microsoft name: EmfPlusCustomLineCapArrowData Object
*/
U_PSEUDO_OBJ *U_PMF_CUSTOMLINECAPARROWDATA_set(U_FLOAT Width, U_FLOAT Height, 
      U_FLOAT MiddleInset, uint32_t FillState, uint32_t StartCap, uint32_t EndCap, uint32_t Join,
      U_FLOAT MiterLimit, U_FLOAT WidthScale
   ){
   const U_SERIAL_DESC List[] = {
      {&Width,       4, 1, U_LE},
      {&Height      ,4, 1, U_LE},
      {&MiddleInset, 4, 1, U_LE},
      {&FillState,   4, 1, U_LE},
      {&StartCap,    4, 1, U_LE},
      {&EndCap,      4, 1, U_LE},
      {&Join,        4, 1, U_LE},
      {&MiterLimit,  4, 1, U_LE},
      {&WidthScale,  4, 1, U_LE},
      {NULL,         8, 2, U_LE}, /* FillHotSpots and LineHotSpots */
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_CUSTOMLINECAPARROWDATA_OID, List);
   return(po);
}


/**
    \brief  Create and set a U_PMF_CUSTOMLINECAPDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Flags      CustomLineCapData flags
    \param  Cap        LineCap enumeration (type of cap)
    \param  Inset      Distance line cap start -> line end
    \param  StartCap   LineCap enumeration
    \param  EndCap     LineCap enumeration
    \param  Join       LineJoin enumeration
    \param  MiterLimit Maximum (miter length / line width)
    \param  WidthScale Scale for U_PMF_CUSTOMLINECAP object
    \param  Clcod      U_PSEUDO_OBJ containing a U_PMF_CUSTOMLINECAPOPTIONALDATA object

    EMF+ manual 2.2.2.13, Microsoft name: EmfPlusCustomLineCapData Object
*/
U_PSEUDO_OBJ *U_PMF_CUSTOMLINECAPDATA_set(uint32_t Flags, uint32_t Cap, 
      U_FLOAT Inset, uint32_t StartCap, uint32_t EndCap, 
      uint32_t Join, U_FLOAT MiterLimit, U_FLOAT WidthScale, 
      const U_PSEUDO_OBJ *Clcod
   ){
   if(!Clcod || Clcod->Type  != U_PMF_CUSTOMLINECAPOPTIONALDATA_OID)return(NULL);
   const U_SERIAL_DESC List[] = {
      {&Flags,      4,           1, U_LE},
      {&Cap,        4,           1, U_LE},
      {&Inset,      4,           1, U_LE},
      {&StartCap,   4,           1, U_LE},
      {&EndCap,     4,           1, U_LE},
      {&Join,       4,           1, U_LE},
      {&MiterLimit, 4,           1, U_LE},
      {&WidthScale, 4,           1, U_LE},
      {NULL,        8,           2, U_LE}, /* FillHotSpots and LineHotSpots */
      {Clcod->Data, Clcod->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_CUSTOMLINECAPDATA_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_CUSTOMLINECAPOPTIONALDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Fill   U_PSEUDO_OBJ containing a U_PMF_FILLPATHOBJ object (optional)
    \param  Line   U_PSEUDO_OBJ containing a U_PMF_LINEPATH object (optional)

    EMF+ manual 2.2.2.14, Microsoft name: EmfPlusCustomLineCapOptionalData Object
*/
U_PSEUDO_OBJ *U_PMF_CUSTOMLINECAPOPTIONALDATA_set(const U_PSEUDO_OBJ *Fill, const U_PSEUDO_OBJ *Line){
   if(Fill && (Fill->Type  != U_PMF_FILLPATHOBJ_OID))return(NULL);
   if(Line && (Line->Type  != U_PMF_LINEPATH_OID))return(NULL);
   const U_SERIAL_DESC List[] = {
      {(Fill ? Fill->Data : NULL), (Fill ? Fill->Used : 0), 1, U_XE},
      {(Line ? Line->Data : NULL), (Line ? Line->Used : 0), 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_CUSTOMLINECAPOPTIONALDATA_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_CUSTOMSTARTCAPDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Clc       U_PSEUDO_OBJ containing a U_PMF_CUSTOMLINECAPDATA object

    EMF+ manual 2.2.2.15, Microsoft name: EmfPlusCustomStartCapData Object
*/
U_PSEUDO_OBJ *U_PMF_CUSTOMSTARTCAPDATA_set(const U_PSEUDO_OBJ *Clc){
   if(!Clc || Clc->Type  != U_PMF_CUSTOMLINECAP_OID)return(NULL);
   const U_SERIAL_DESC List[] = {
      {Clc->Data, Clc->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_CUSTOMSTARTCAPDATA_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_DASHEDLINEDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Elements   Members in Lengths
    \param  Lengths    Array of U_FLOAT holding lengths of dashes and spaces.

    EMF+ manual 2.2.2.16, Microsoft name: EmfPlusDashedLineData Object
*/
U_PSEUDO_OBJ *U_PMF_DASHEDLINEDATA_set(int32_t Elements, const U_FLOAT *Lengths){
   if(!Lengths)return(NULL);
   const U_SERIAL_DESC List[] = {
      {&Elements, 4, 1,        U_LE},
      {Lengths,   4, Elements, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_DASHEDLINEDATA_OID, List);
   return(po);
}

/**
    \brief  Utility function to create and set a U_PMF_DASHEDLINEDATA PseudoObject from one of a predefined set of patterns
    \return Pointer to PseudoObject, NULL on error
    \param  Unit       Length of the repeat unit
    \param  StdPat     Members in Lengths

    EMF+ manual 2.2.2.16, Microsoft name: EmfPlusDashedLineData Object
*/
U_PSEUDO_OBJ *U_PMF_DASHEDLINEDATA_set2(U_FLOAT Unit, int StdPat){
   uint32_t Elements;
   uint8_t *p;
   U_FLOAT SubUnit;
   U_FLOAT Lengths[8];  /* This is the most dash/spaces that will be needed*/
   int i;
   /* Dot = 1, Dash = 2; Long = 3, 0 = terminate pattern */
   uint8_t SB[U_DD_Types][5] =
      {
         {0,0,0,0,0},  // Solid                   
         {2,0,0,0,0},  // Dash 
         {2,2,0,0,0},  // DashDash
         {2,2,2,0,0},  // DashDashDash
         {2,2,2,2,0},  // DashDashDashDash
         {1,0,0,0,0},  // Dot  
         {1,1,0,0,0},  // DotDot 
         {1,1,1,0,0},  // DotDotDot
         {1,1,1,1,0},  // DotDotDotDot
         {2,1,0,0,0},  // DashDot
         {2,2,1,0,0},  // DashDashDot
         {2,2,1,1,0},  // DashDashDotDot
         {2,2,2,1,0},  // DashDashDashDot
         {2,1,1,0,0},  // DashDotDot
         {2,1,1,1,0},  // DashDotDotDot
         {2,1,2,1,0},  // DashDotDashDot
         {3,0,0,0,0},  // Long 
         {3,3,0,0,0},  // LongLong
         {3,3,3,0,0},  // LongLongLong
         {3,3,3,3,0},  // LongLongLongLong
         {3,1,0,0,0},  // LongDot
         {3,3,1,0,0},  // LongLongDot
         {3,3,1,1,0},  // LongLongDotDot
         {3,3,3,1,0},  // LongLongLongDot
         {3,1,1,0,0},  // LongDotDot
         {3,1,1,1,0},  // LongDotDotDot
         {3,1,3,1,0}   // LongDotLongDot
      };
   if(Unit <= 0                                      ){ return(NULL); }
   if((StdPat <= 0) || (StdPat > U_DD_LongDotLongDot)){ return(NULL); }
   p = &(SB[StdPat][0]);
   for(Elements = 0; *p; p++, Elements++){}
   SubUnit = Unit/((U_FLOAT) Elements);
   Elements *= 2;
   p = &(SB[StdPat][0]);
   for(i=0; *p; p++){
      switch(*p){
         case 0: break;
         case 1: /* dot */
           Lengths[i++] = SubUnit * 0.125;
           Lengths[i++] = SubUnit * 0.875;
           break;
         case 2: /* dash */
           Lengths[i++] = SubUnit * 0.5;
           Lengths[i++] = SubUnit * 0.5;
           break;
         case 3: /* long */
           Lengths[i++] = SubUnit * 0.75;
           Lengths[i++] = SubUnit * 0.25;
           break;
      }
   }

   U_PSEUDO_OBJ *po = U_PMF_DASHEDLINEDATA_set(Elements, Lengths);
   return(po);
}

/**
    \brief  Utility function to create and set a U_PMF_DASHEDLINEDATA PseudoObject from the bits that are set in a uint32_t
    \return Pointer to PseudoObject, NULL on error
    \param  Unit       Length of the repeat unit
    \param  BitPat     uint32_t holding the bit pattern, the lowest order bit MUST be set and the highest order MUST be clear.
    
    Make a line with a dot/dash pattern defined by the bits in the BitPat value.  If a bit is set it is drawn,
    if clear it is not.  Every bit drawn has length Unit/32, and consecutive drawn bits are merged together.  
    The lowest order bit is the first bit that may be drawn, the highest the last.  
    
        Example: if the integer has value 0x13 the pattern produced will be:
        0          ->  2*unit/32 drawn
        2*unit/32  ->  5*unit/32 not drawn
        5*unit/32  ->  6*unit/32 drawn
        6*unit/32  ->  unit      not drawn

    EMF+ manual 2.2.2.16, Microsoft name: EmfPlusDashedLineData Object
*/
U_PSEUDO_OBJ *U_PMF_DASHEDLINEDATA_set3(U_FLOAT Unit, uint32_t BitPat){
   uint32_t Elements=0;
   U_FLOAT SubUnit = Unit/32.0;
   U_FLOAT Lengths[32];  /* This is the most dash/spaces that will be needed*/
   if(!(0x00000001 & BitPat))return(NULL);  /* Pattern must start with a drawn segment, this bit must be set */
   if(  0x80000000 & BitPat )return(NULL);  /* Pattern must end with an undrawn segment, this bit must be clear */
   int i=0;
   int k;
   int lastType=1;
   int newType=0;
   uint32_t j=1;
   Lengths[0]=0;
   for(k=0; k<32; k++, j=j<<1){
      if(j & BitPat){
         if(!lastType){
            newType=1;
         }
      }
      else {
         if(lastType){
            newType=1;
         }
      }
      if(newType){
         i++;
         Lengths[i]=0;
         Elements++;
         lastType = !lastType;
         newType = 0;
      }
      Lengths[i] += SubUnit;
   }
   Elements = i+1;

   U_PSEUDO_OBJ *po = U_PMF_DASHEDLINEDATA_set(Elements, Lengths);
   return(po);
}

/**
    \brief  Create and set a U_PMF_FILLPATHOBJ PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Path       U_PSEUDO_OBJ containing a U_PMF_PATH object

    EMF+ manual 2.2.2.17, Microsoft name: EmfPlusFillPath Object
*/
U_PSEUDO_OBJ *U_PMF_FILLPATHOBJ_set(const U_PSEUDO_OBJ *Path){
   if(!Path || (Path->Type  != U_PMF_PATH_OID))return(NULL);
   const U_SERIAL_DESC List[] = {
      {Path->Data, Path->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_FILLPATHOBJ_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_FOCUSSCALEDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  ScaleX     value 0.0 <-> 1.0
    \param  ScaleY     value 0.0 <-> 1.0

    EMF+ manual 2.2.2.18, Microsoft name: EmfPlusFocusScaleData Object
*/
U_PSEUDO_OBJ *U_PMF_FOCUSSCALEDATA_set(U_FLOAT ScaleX, U_FLOAT ScaleY){
   uint32_t tmp = 2;
   const U_SERIAL_DESC List[] = {
      {&tmp,    4, 1, U_LE},
      {&ScaleX, 4, 1, U_LE},
      {&ScaleY, 4, 1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_FOCUSSCALEDATA_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_GRAPHICSVERSION object (Signature always set to 0xDBC01)
    \return Pointer to PseudoObject, NULL on error
    \param  GrfVersion GraphicsVersion enumeration

    EMF+ manual 2.2.2.19, Microsoft name: EmfPlusGraphicsVersion Object
*/
U_PSEUDO_OBJ *U_PMF_GRAPHICSVERSION_set(int GrfVersion){
   uint32_t tmp;
   tmp  = U_GFVR_PMF << 12; /* signature, can only have this value */
   tmp |= (GrfVersion & U_GFVR_MASKLO);
   const U_SERIAL_DESC List[] = {
      {&tmp, 4, 1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_GRAPHICSVERSION_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_GRAPHICSVERSION object Structure (Signature always set to 0xDBC01)
    \return U_PMF_GRAPHICSVERSION
    \param  GrfVersion GraphicsVersion enumeration

    EMF+ manual 2.2.2.19, Microsoft name: EmfPlusGraphicsVersion Object
*/
U_PMF_GRAPHICSVERSION U_PMF_GRAPHICSVERSIONOBJ_set(int GrfVersion){
   uint32_t tmp;
   tmp  = U_GFVR_PMF << 12; /* signature, can only have this value */
   tmp |= (GrfVersion & U_GFVR_MASKLO);
   return(tmp);
}


/**
    \brief  Create and set a U_PMF_HATCHBRUSHDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Style      HatchStyle enumeration
    \param  Fg         U_PSEUDO_OBJ containing a U_ARGB object, Foreground hatch pattern line color
    \param  Bg         U_PSEUDO_OBJ containing a U_ARGB object, Background hatch pattern line color

    EMF+ manual 2.2.2.20, Microsoft name: EmfPlusHatchBrushData Object
*/
U_PSEUDO_OBJ *U_PMF_HATCHBRUSHDATA_set(uint32_t Style, const U_PSEUDO_OBJ *Fg, const U_PSEUDO_OBJ *Bg){
   if(!Fg ||(Fg->Type != U_PMF_ARGB_OID))return(NULL);
   if(!Bg ||(Bg->Type != U_PMF_ARGB_OID))return(NULL);
   const U_SERIAL_DESC List[] = {
      {&Style,   4,        1, U_LE},
      {Fg->Data, Fg->Used, 1, U_XE},
      {Bg->Data, Bg->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_HATCHBRUSHDATA_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_INTEGER7 PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Value 7 bit signed integer (stored in an integer, range 63 <-> -64, inclusive)

    EMF+ manual 2.2.2.21, Microsoft name: EmfPlusInteger7 Object
*/
U_PSEUDO_OBJ *U_PMF_INTEGER7_set(int Value){
   uint8_t utmp;
   if(Value < -64 || Value > 63)return(NULL);
   utmp = U_MASK_INT7 & *(unsigned int *)&Value;
   U_PSEUDO_OBJ *po =  U_PO_create((char *)&utmp, 1, 1, U_PMF_INTEGER7_OID); /* simple method is OK, no possibility of Endian issues */
   return(po);
}

/**
    \brief  Create and set a U_PMF_INTEGER15 PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Value 15 bit signed integer (stored in an integer, range 32677 <-> -32678, inclusive)

    EMF+ manual 2.2.2.22, Microsoft name: EmfPlusInteger15 Object
*/
U_PSEUDO_OBJ *U_PMF_INTEGER15_set(int Value){
   uint16_t utmp;
   if(Value < -32678 || Value > 32677)return(NULL);
   utmp = U_TEST_INT15 | (U_MASK_INT15 & *(unsigned int *)&Value);
   const U_SERIAL_DESC List[] = {
      {&utmp, 2, 1, U_BE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_INTEGER15_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_LANGUAGEIDENTIFIER value in 4 byte unsigned int, in NATIVE byte order
    \return LID value in least significant two bytes and 0 in most significant two bytes.
    \param  SubLId     Example: code for USA
    \param  PriLId     Example: code for English

    EMF+ manual 2.2.2.23, Microsoft name: EmfPlusLanguageIdentifier Object
*/
U_PMF_LANGUAGEIDENTIFIER U_PMF_LANGUAGEIDENTIFIEROBJ_set(int SubLId, int PriLId){
   U_PMF_LANGUAGEIDENTIFIER utmp32;
   utmp32 = ((SubLId & U_FF_MASK_SUBLID) << U_FF_SHFT_SUBLID) | ((PriLId & U_FF_MASK_PRILID) << U_FF_SHFT_PRILID);
   return(utmp32);
}

/**
    \brief  Create and set a U_PMF_LANGUAGEIDENTIFIER PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  LId       Language Identifier as produced by U_PMF_LANGUAGEIDENTIFIEROBJ_set().

    EMF+ manual 2.2.2.23, Microsoft name: EmfPlusLanguageIdentifier Object
*/
U_PSEUDO_OBJ *U_PMF_LANGUAGEIDENTIFIER_set(U_PMF_LANGUAGEIDENTIFIER LId){
   uint16_t utmp16;
   utmp16 = (LId & U_FF_MASK_LID) << U_FF_SHFT_LID;
   const U_SERIAL_DESC List[] = {
      {&utmp16, 2, 1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_LANGUAGEIDENTIFIER_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_LINEARGRADIENTBRUSHDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Lgbd       U_PMF_LINEARGRADIENTBRUSHDATA object (constant part)
    \param  Lgbod      U_PSEUDO_OBJ containing a U_PMF_LINEARGRADIENTBRUSHOPTIONALDATA object (variable part of a U_PMF_LINEARGRADIENTBRUSHDATA object)
     

    EMF+ manual 2.2.2.24, Microsoft name: EmfPlusLinearGradientBrushData Object
*/
U_PSEUDO_OBJ *U_PMF_LINEARGRADIENTBRUSHDATA_set(const U_PMF_LINEARGRADIENTBRUSHDATA *Lgbd, const U_PSEUDO_OBJ *Lgbod){
   if(!Lgbd || !Lgbod || (Lgbod->Type != U_PMF_LINEARGRADIENTBRUSHOPTIONALDATA_OID))return(NULL);
   const U_SERIAL_DESC List[] = {
      {Lgbd,                                4,           6, U_LE},
      {&(Lgbd->StartColor),                 4,           2, U_XE},
      {&(Lgbd->StartColor),                 4,           2, U_XE}, /* repeat the start/end colors.  Supposedly reserved. */
//    {NULL,                                4,           2, U_LE}, /* zero fill the two Reserved fields, no matter what is passed in */
      {(Lgbod->Used ? Lgbod->Data : NULL),  Lgbod->Used, 1, U_XE}, /* optional Data can exist and Used can be zero, SERIAL_set would throw an error on that */
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_LINEARGRADIENTBRUSHDATA_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_LINEARGRADIENTBRUSHOPTIONALDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Flags      Bits are set that indicate which of the following were included. The caller must clear before passing it in.
    \param  Tm         (optional) U_PSEUDO_OBJ containing a U_PMF_TRANSFORMMATRIX object
    \param  Bc         (optional) U_PSEUDO_OBJ containing a U_PMF_BLENDCOLORS object or NULL
    \param  BfH        (optional) U_PSEUDO_OBJ containing a U_PMF_BLENDFACTORS (H) object or NULL
    \param  BfV        (optional) U_PSEUDO_OBJ containing a U_PMF_BLENDFACTORS (V) object or NULL (WARNING, GDI+ defines this field but does not render it.  DO NOT USE.)
     

    EMF+ manual 2.2.2.25, Microsoft name: EmfPlusLinearGradientBrushOptionalData Object

    
    The rectangular gradients repeat in a tiled pattern.  Tm can rotate and offset the gradient within each tile.
    The gradient wraps when it is offset.
    
*/
U_PSEUDO_OBJ *U_PMF_LINEARGRADIENTBRUSHOPTIONALDATA_set(uint32_t *Flags, const U_PSEUDO_OBJ *Tm, 
      const U_PSEUDO_OBJ *Bc, const U_PSEUDO_OBJ *BfH, const U_PSEUDO_OBJ *BfV){
   if(!Flags                                         )return(NULL);
   if(Tm  && (Tm->Type  != U_PMF_TRANSFORMMATRIX_OID))return(NULL);
   if(Bc  && (Bc->Type  != U_PMF_BLENDCOLORS_OID)    )return(NULL);
   if(BfH && (BfH->Type != U_PMF_BLENDFACTORS_OID)   )return(NULL);
   if(BfV && (BfV->Type != U_PMF_BLENDFACTORS_OID)   )return(NULL);
   if(Bc  && (BfH || BfV)                            )return(NULL);
   const U_SERIAL_DESC List[] = {
      {(Tm  ? Tm->Data  : NULL), (Tm  ? Tm->Used  : 0), 1, U_XE},
      {(Bc  ? Bc->Data  : NULL), (Bc  ? Bc->Used  : 0), 1, U_XE},
      {(BfH ? BfH->Data : NULL), (BfH ? BfH->Used : 0), 1, U_XE},
      {(BfV ? BfV->Data : NULL), (BfV ? BfV->Used : 0), 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_LINEARGRADIENTBRUSHOPTIONALDATA_OID, List);
   if(Tm ){ *Flags |= U_BD_Transform;     }
   if(Bc ){ *Flags |= U_BD_PresetColors;  }
   if(BfH){ *Flags |= U_BD_BlendFactorsH; }
   if(BfV){ *Flags |= U_BD_BlendFactorsV; }
   return(po);
}

/**
    \brief  Create and set a U_PMF_LINEPATH PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Path      U_PSEUDO_OBJ containing a U_PMF_PATH object
     

    EMF+ manual 2.2.2.26, Microsoft name: EmfPlusLinePath Object
*/
U_PSEUDO_OBJ *U_PMF_LINEPATH_set(const U_PSEUDO_OBJ *Path){
   if(!Path || (Path->Type != U_PMF_PATH_OID))return(NULL);
   const U_SERIAL_DESC List[] = {
      {&Path->Data, Path->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_LINEPATH_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_METAFILE object (NOT SUPPORTED!)
    \return Null
     

    EMF+ manual 2.2.2.27, Microsoft name: EmfPlusMetafile Object
*/
U_PSEUDO_OBJ *U_PMF_METAFILE_set(void){
   return(NULL);
}

/**
    \brief  Create and set a U_PMF_PALETTE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Flags      PaletteStyle flags
    \param  Elements   Members in Lengths
    \param  Pd         Array of U_PMF_ARGB holding colors of palettes. (Palette Data)

    EMF+ manual 2.2.2.28, Microsoft name: EmfPlusPalette Object
*/
U_PSEUDO_OBJ *U_PMF_PALETTE_set(uint32_t Flags, uint32_t Elements, const U_PMF_ARGB *Pd){
   if(!Pd)return(NULL);
   const U_SERIAL_DESC List[] = {
      {&Flags,    4, 1,        U_LE},
      {&Elements, 4, 1,        U_LE},
      {Pd,        4, Elements, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_PALETTE_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_PATHGRADIENTBRUSHDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Flags        PaletteStyle flags
    \param  WrapMode     WrapMode enumeration
    \param  CenterColor  U_PMF_ARGB Center color
    \param  Center       Center coordinates
    \param  Gradient     U_PSEUDO_OBJ containing an Array of U_PMF_ARGB holding colors of Gradient
    \param  Boundary     U_PSEUDO_OBJ containing a U_PMF_BOUNDARYPATHDATA or U_PMF_BOUNDARYPOINTDATA object.  (Boundary Data)
    \param  Data         variable part of U_PMF_LINEARGRADIENTBRUSHDATA, exact composition depends on Flags

    EMF+ manual 2.2.2.29, Microsoft name: EmfPlusPathGradientBrushData Object
*/
U_PSEUDO_OBJ *U_PMF_PATHGRADIENTBRUSHDATA_set(uint32_t Flags, int32_t WrapMode, U_PMF_ARGB CenterColor, 
      U_PMF_POINTF Center,
      const U_PSEUDO_OBJ *Gradient, const U_PSEUDO_OBJ *Boundary, const U_PSEUDO_OBJ *Data){
   if( (Flags & U_BD_Path) && (!Boundary || (Boundary->Type != U_PMF_BOUNDARYPATHDATA_OID)))return(NULL);
   if(!(Flags & U_BD_Path) && (!Boundary || (Boundary->Type != U_PMF_BOUNDARYPOINTDATA_OID)))return(NULL);
   if(!Gradient || (Gradient->Type != (U_PMF_ARGB_OID | U_PMF_ARRAY_OID)))return(NULL);
   if(!(Flags & U_BD_Transform) && 
      !(Flags & U_BD_PresetColors) && 
      !(Flags & U_BD_BlendFactorsH) && 
      !(Flags & U_BD_FocusScales) && 
      (!Data || (Data->Type != U_PMF_PATHGRADIENTBRUSHOPTIONALDATA_OID)))return(NULL);

   const U_SERIAL_DESC List[] = {
      {&Flags,                         4,                       1, U_LE},
      {&WrapMode,                      4,                       1, U_LE},
      {&CenterColor,                   4,                       1, U_XE},
      {&Center.X,                      4,                       2, U_LE},
      {Gradient->Data,                 Gradient->Used,          1, U_XE}, /* includes Elements */
      {Boundary->Data,                 Boundary->Used,          1, U_XE},
      {(Data ? Data->Data : NULL),     (Data ? Data->Used : 0), 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_PATHGRADIENTBRUSHDATA_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_PATHGRADIENTBRUSHOPTIONALDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Flags        PaletteStyle flags
    \param  Tm           U_PSEUDO_OBJ containing a U_PMF_TRANSFORMMATRIX.  (Transformation matrix)
    \param  Pd           U_PSEUDO_OBJ containing a U_PMF_BLENDCOLORS or U_PMF_BLENDFACTORS object.  (Pattern Data)
    \param  Fsd          U_PSEUDO_OBJ containing a U_PMF_FOCUSSSCALEDATA object.  (Focus Scale Data)

    EMF+ manual 2.2.2.30, Microsoft name: EmfPlusPathGradientBrushOptionalData Object
*/
U_PSEUDO_OBJ *U_PMF_PATHGRADIENTBRUSHOPTIONALDATA_set(uint32_t Flags,
      const U_PSEUDO_OBJ *Tm, const U_PSEUDO_OBJ *Pd, const U_PSEUDO_OBJ *Fsd){
   if(Tm &&  (Tm->Type != U_PMF_TRANSFORMMATRIX_OID))return(NULL);
   if(Pd && !(Flags & (U_BD_PresetColors | U_BD_BlendFactorsH)))return(NULL);
   if( (Flags & U_BD_PresetColors)  && ((Flags & U_BD_BlendFactorsH)  || !Pd || (Pd->Type != U_PMF_BLENDCOLORS_OID) ))return(NULL);
   if( (Flags & U_BD_BlendFactorsH) && ((Flags & U_BD_PresetColors)   || !Pd || (Pd->Type != U_PMF_BLENDFACTORS_OID)))return(NULL);
   if(Fsd && !(Flags & U_BD_FocusScales))return(NULL);
   if( (Flags & U_BD_FocusScales)   && (!Fsd || (Fsd->Type != U_PMF_BLENDCOLORS_OID) ))return(NULL);
   const U_SERIAL_DESC List[] = {
      {(Tm  ? Tm->Data  : NULL), (Tm ?  Tm->Used  : 0), 1, U_XE},
      {(Pd  ? Pd->Data  : NULL), (Pd ?  Pd->Used  : 0), 1, U_XE},
      {(Fsd ? Fsd->Data : NULL), (Fsd ? Fsd->Used : 0), 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_PATHGRADIENTBRUSHOPTIONALDATA_OID, List);
   return(po);
}

/**
    \brief  Create and set an ARRAY of U_PMF_PATHPOINTTYPE objects
    \return Pointer to PseudoObject, NULL on error
    \param  Elements     Number of entries in Flags and Enumerations
    \param  Ppt          Array of unsigned bytes, lower 4 bits hold the PathPointType flag upper 4 bits hold the PathPointType enumeration.

    EMF+ manual 2.2.2.31, Microsoft name: EmfPlusPathPointType Object
*/
U_PSEUDO_OBJ *U_PMF_PATHPOINTTYPE_set(uint32_t Elements, const uint8_t *Ppt){
   if(!Elements || !Ppt)return(NULL);
   const U_SERIAL_DESC List[] = {
      {&Elements, 4, 1,        U_LE},
      {Ppt,       1, Elements, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_PATHPOINTTYPE_OID | U_PMF_ARRAY_OID, List);
   return(po);
}

/**
    \brief  Create and set an ARRAY of U_PMF_PATHPOINTTYPE objects, with a preceding Elements count
    \return Pointer to PseudoObject, NULL on error
    \param  Elements Number of elements to add. First is added once and Others Elements-1 times.
    \param  First    Apply to first point,      unsigned byte, lower 4 bits hold the PathPointType flag upper 4 bits hold the PathPointType enumeration.
    \param  Others   Apply to all other points, unsigned byte, lower 4 bits hold the PathPointType flag upper 4 bits hold the PathPointType enumeration.

    EMF+ manual 2.2.2.31, Microsoft name: EmfPlusPathPointType Object
*/
U_PSEUDO_OBJ *U_PMF_PATHPOINTTYPE_set2(uint32_t Elements, uint8_t First, uint8_t Others){
   if(!Elements)return(NULL);
   const U_SERIAL_DESC List[] = {
      {&Elements,  4,   1,          U_XE},
      {&First,     1,   1,          U_XE},
      {&Others,    1,   Elements-1, U_RP},  /* replicate the one value N-1 times */
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_PATHPOINTTYPE_OID | U_PMF_ARRAY_OID, List);
   return(po);
}

/**
    \brief  Create and set an ARRAY of U_PMF_PATHPOINTTYPERLE objects
    \return Pointer to PseudoObject containing first the number of elements, then an array of U_PMF_PATHPOINTTYPERLE, NULL on error
    \param  Elements     Number of entries in the arrays
    \param  Bz           Array of unsigned bytes, if not zero, element has Bezier bit set
    \param  RL           Array of unsigned bytes, Run lengths.
    \param  Ppte         Array of unsigned bytes, PathPointType enumerations.

    EMF+ manual 2.2.2.32, Microsoft name: EmfPlusPathPointTypeRLE Object
*/
U_PSEUDO_OBJ *U_PMF_PATHPOINTTYPERLE_set(uint32_t Elements, const uint8_t *Bz, const uint8_t *RL, const uint8_t *Ppte){
   uint8_t utmp;
   if(!Bz || !RL || !Ppte)return(NULL);
   /* allocate space in the structure but put no data in */
   U_PSEUDO_OBJ *po =  U_PO_create(NULL, 4 + 2*Elements, 0, U_PMF_PATHPOINTTYPERLE_OID | U_PMF_ARRAY_OID);
   U_PSEUDO_OBJ *holdpo = po; 
   if(po){
      U_PSEUDO_OBJ *poi = U_PMF_4NUM_set(Elements);
      if(!poi)goto end;
      po = U_PO_append(po, poi->Data, poi->Used);   
      U_PO_free(&poi);
      if(!po)goto end;

      for( ;Elements; Elements--, Bz++, RL++, Ppte++){
         po = U_PO_append(po, (char *)Ppte, 1);
         if(!po)goto end;
         
         if(*RL > 0x3F) goto end; /* run length too big for field */
           
         utmp = (*Bz ? 1 : 0) | ((*RL & 0x3F)<<2); /* bit 1 is not used and is set to 0 */
         po = U_PO_append(po, (char *)&utmp, 1);
         if(!po)goto end;
      }
   }
end:
   if(!po)U_PO_free(&holdpo);
   return(holdpo);
}

/**
    \brief  Create and set a U_PMF_PENDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Unit      UnitType enumeration
    \param  Width     Width in units set by Unit
    \param  Pod       U_PSEUDO_OBJ containing first the PenData flags then a U_PMF_PENOPTIONALDATA object (the second part
                        may be an empty if Flags is 0)
     

    EMF+ manual 2.2.2.33, Microsoft name: EmfPlusPenData Object
*/
U_PSEUDO_OBJ *U_PMF_PENDATA_set(uint32_t Unit, U_FLOAT Width, const U_PSEUDO_OBJ *Pod){
   if(Pod && ((Pod->Type != U_PMF_PENOPTIONALDATA_OID) || Pod->Used < 4))return(NULL);
   const U_SERIAL_DESC List[] = {
      {(Pod ? Pod->Data : NULL),     4,                         1,             U_XE}, /* the Flags field, clear if no optional data */
      {&Unit,                        4,                         1,             U_LE},
      {&Width,                       4,                         1,             U_LE},
          /* next is the (optional) U_PMF_PENOPTIONALDATA part or a terminator */
      {(Pod ? Pod->Data + 4 : NULL), (Pod ? Pod->Used - 4 : 0), (Pod ? 1 : 0), (Pod ? U_XE : U_XX)},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_PENDATA_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_PENOPTIONALDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error.  Returned PO contains first the Flags, then the PO proper.
    \param  Flags           Determines which of the values are stored.
    \param  Tm              U_PSEUDO_OBJ containing a U_PMF_TRANSFORMMATRIX  object (Transformation matrix)
    \param  StartCap        LineCapType enumeration
    \param  EndCap          LineCapType enumeration
    \param  Join            LineJoinType enumeration
    \param  MiterLimit      Maximum (miter length / line width)
    \param  Style           LineStyle enumeration
    \param  DLCap           DashedLineCapType enumeration
    \param  DLOffset        Distance line start to first dash start
    \param  DLData          U_PSEUDO_OBJ containing a U_PMF_DASHEDLINEDATA  object     Dash and space widths
    \param  PenAlignment    PenAlignment enumeration
    \param  CmpndLineData   U_PSEUDO_OBJ containing a U_PMF_COMPOUNDLINEDATA object    Compount Line (parallel lines drawn instead of one)
    \param  CSCapData       U_PSEUDO_OBJ containing a U_PMF_CUSTOMSTARTCAPDATA object  Custom start cap  
    \param  CECapData       U_PSEUDO_OBJ containing a U_PMF_CUSTOMENDCAPDATA object    Custom end cap
     

    EMF+ manual 2.2.2.34, Microsoft name: EmfPlusPenOptionalData Object
*/
U_PSEUDO_OBJ *U_PMF_PENOPTIONALDATA_set(uint32_t Flags, U_PSEUDO_OBJ *Tm, int32_t StartCap, int32_t EndCap, uint32_t Join,
      U_FLOAT MiterLimit, int32_t Style, int32_t DLCap, U_FLOAT DLOffset, 
      U_PSEUDO_OBJ *DLData, int32_t PenAlignment, U_PSEUDO_OBJ *CmpndLineData, U_PSEUDO_OBJ *CSCapData, 
      U_PSEUDO_OBJ *CECapData
   ){

   if((Flags & U_PD_Transform)      && (!Tm            || (Tm->Type            != U_PMF_TRANSFORMMATRIX_OID))   )return(NULL);
   if((Flags & U_PD_DLData)         && (!DLData        || (DLData->Type        != U_PMF_DASHEDLINEDATA_OID))    )return(NULL);
   if((Flags & U_PD_CLData)         && (!CmpndLineData || (CmpndLineData->Type != U_PMF_COMPOUNDLINEDATA_OID))  )return(NULL);
   if((Flags & U_PD_CustomStartCap) && (!CSCapData     || (CSCapData->Type     != U_PMF_CUSTOMSTARTCAPDATA_OID)))return(NULL); 
   if((Flags & U_PD_CustomEndCap)   && (!CECapData     || (CECapData->Type     != U_PMF_CUSTOMENDCAPDATA_OID))  )return(NULL);
   
   /* prepend the Flags field to the PseudoObject proper */
   const U_SERIAL_DESC List[] = {
      {&Flags,                                                         4,                                                         1, U_LE},
      {((Flags & U_PD_Transform)      ? Tm->Data              : NULL), ((Flags & U_PD_Transform)      ? Tm->Used            : 0), 1, U_XE},
      {((Flags & U_PD_StartCap )      ? (char *)&StartCap     : NULL), ((Flags & U_PD_StartCap )      ? 4                   : 0), 1, U_LE},
      {((Flags & U_PD_EndCap )        ? (char *)&EndCap       : NULL), ((Flags & U_PD_EndCap )        ? 4                   : 0), 1, U_LE},
      {((Flags & U_PD_Join )          ? (char *)&Join         : NULL), ((Flags & U_PD_Join )          ? 4                   : 0), 1, U_LE},
      {((Flags & U_PD_MiterLimit )    ? (char *)&MiterLimit   : NULL), ((Flags & U_PD_MiterLimit )    ? 4                   : 0), 1, U_LE},
      {((Flags & U_PD_LineStyle )     ? (char *)&Style        : NULL), ((Flags & U_PD_LineStyle )     ? 4                   : 0), 1, U_LE},
      {((Flags & U_PD_DLCap )         ? (char *)&DLCap        : NULL), ((Flags & U_PD_DLCap )         ? 4                   : 0), 1, U_LE},
      {((Flags & U_PD_DLOffset )      ? (char *)&DLOffset     : NULL), ((Flags & U_PD_DLOffset )      ? 4                   : 0), 1, U_LE},
      {((Flags & U_PD_DLData )        ? DLData->Data          : NULL), ((Flags & U_PD_DLData )        ? DLData->Used        : 0), 1, U_XE},
      {((Flags & U_PD_NonCenter )     ? (char *)&PenAlignment : NULL), ((Flags & U_PD_NonCenter )     ? 4                   : 0), 1, U_LE},
      {((Flags & U_PD_CLData )        ? CmpndLineData->Data   : NULL), ((Flags & U_PD_CLData )        ? CmpndLineData->Used : 0), 1, U_XE},
      {((Flags & U_PD_CustomStartCap) ? CSCapData->Data       : NULL), ((Flags & U_PD_CustomStartCap) ? CSCapData->Used     : 0), 1, U_XE},
      {((Flags & U_PD_CustomEndCap )  ? CECapData->Data       : NULL), ((Flags & U_PD_CustomEndCap )  ? CECapData->Used     : 0), 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_PENOPTIONALDATA_OID, List);
   return(po);
}

/**
    \brief  Create and set an ARRAY of U_PMF_POINT objects
    \return Pointer to PseudoObject, NULL on error
    \param  Elements     Number of pairs of points in Coords
    \param  Coords       Array of X,Y pairs.

    EMF+ manual 2.2.2.35, Microsoft name: EmfPlusPoint Object
*/
U_PSEUDO_OBJ *U_PMF_POINT_set(uint32_t Elements, const U_PMF_POINT *Coords){
   if(!Elements || !Coords)return(NULL);
   const U_SERIAL_DESC List[] = {
      {&Elements, 4, 1,          U_LE},
      { Coords,   2, 2*Elements, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_POINT_OID | U_PMF_ARRAY_OID, List);
   return(po);
}

/**
    \brief  Create and set an ARRAY of U_PMF_POINTF objects, with a leading Elements value
    \return Pointer to PseudoObject, NULL on error
    \param  Elements     Number of pairs of points in Coords
    \param  Coords       Array of X,Y pairs.

    EMF+ manual 2.2.2.36, Microsoft name: EmfPlusPointF Object
*/
U_PSEUDO_OBJ *U_PMF_POINTF_set(uint32_t Elements, const U_PMF_POINTF *Coords){
   if(!Elements || !Coords)return(NULL);
   const U_SERIAL_DESC List[] = {
      {&Elements, 4, 1,          U_LE},
      {Coords,    4, 2*Elements, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_POINTF_OID | U_PMF_ARRAY_OID, List);
   return(po);
}

/**
    \brief  Create and set an ARRAY of U_PMF_POINTR objects
    \return Pointer to PseudoObject, NULL on error
    \param  Elements     Number of pairs of points in Coords
    \param  Coords       Array of X,Y pairs.  These are absolute coordinates, they are converted to Relative here.

    EMF+ manual 2.2.2.37, Microsoft name: EmfPlusPointR Object
*/
U_PSEUDO_OBJ *U_PMF_POINTR_set(uint32_t Elements, const U_PMF_POINTF *Coords){
   int X,Y;
   U_FLOAT Xf,Yf;
   U_PSEUDO_OBJ *poi;
   /* Worst case scenario it is 4 bytes per coord, plus the count  */
   U_PSEUDO_OBJ *po =  U_PO_create(NULL, 4 + 4*Elements, 0, U_PMF_POINTR_OID); /* not exactly an array, so no U_PMF_ARRAY_OID */
   U_PSEUDO_OBJ *holdpo = po;
   if(!po)goto end;

   poi = U_PMF_4NUM_set(Elements);
   po = U_PO_append(po, poi->Data, poi->Used);      
   U_PO_free(&poi);
   if(!po)goto end;

   for(Xf = Yf = 0.0 ;Elements; Elements--, Coords++){
      Xf = U_ROUND(Coords->X) - Xf;
      Yf = U_ROUND(Coords->Y) - Yf;
      X = ( Xf >= UINT16_MAX ? UINT16_MAX : ( Xf <= INT16_MIN ? INT16_MIN : Xf));
      Y = ( Yf >= UINT16_MAX ? UINT16_MAX : ( Yf <= INT16_MIN ? INT16_MIN : Yf));
      Xf = U_ROUND(Coords->X);
      Yf = U_ROUND(Coords->Y);
      
      /* this is not a very efficient method, too much mucking around with memory */

      poi = U_PMF_INTEGER7_set(X);
      if(!poi)poi = U_PMF_INTEGER15_set(X); /* This one must work because of the range checking, above */
      po = U_PO_append(po, poi->Data, poi->Used);      
      U_PO_free(&poi);
      if(!po)goto end;

      poi = U_PMF_INTEGER7_set(Y);
      if(!poi)poi = U_PMF_INTEGER15_set(Y); /* This one must work because of the range checking, above */
      po = U_PO_append(po, poi->Data, poi->Used);      
      U_PO_free(&poi);
      if(!po)goto end;
   }
   /* Because the values stored were some unpredictable combination of 1 and 2 bytes, the last byte may not end
   on a 4 byte boundary.  Make it do so by padding with up to 3 zero bytes.  */

   int residual;
   residual = 3 & po->Used;
   if(residual){ 
      po = U_PO_append(po, NULL, (4 - residual));
      if(!po)goto end;     
   }

end:
   if(!po)U_PO_free(&holdpo);
   return(holdpo);
}

/**
    \brief  Create and set a U_PMF_RECT object
    \return Pointer to PseudoObject, NULL on error
    \param  X          UL X value
    \param  Y          UL Y value
    \param  Width      Width
    \param  Height     Height

    EMF+ manual 2.2.2.38, Microsoft name: EmfPlusRect Object
*/
U_PSEUDO_OBJ *U_PMF_RECT4_set(int16_t X, int16_t Y, int16_t Width, int16_t Height){
   const U_SERIAL_DESC List[] = {
      {&X,      2, 1, U_LE},
      {&Y,      2, 1, U_LE},
      {&Width,  2, 1, U_LE},
      {&Height, 2, 1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_RECT_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_RECT object
    \return Pointer to PseudoObject, NULL on error
    \param  Rect       U_PMF_RECT structures

    EMF+ manual 2.2.2.38, Microsoft name: EmfPlusRect Object
*/
U_PSEUDO_OBJ *U_PMF_RECT_set(U_PMF_RECT *Rect){
   const U_SERIAL_DESC List[] = {
      {Rect, 2, 4, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_RECT_OID, List);
   return(po);
}

/**
    \brief  Create and set an array of U_PMF_RECT objects in a PseudoObject
    \return Pointer to PseudoObject, NULL on error.  PseudoObject contains Elements followed by the array of U_PMF_RECT objects.
    \param  Elements   Number of elements in Rects
    \param  Rects      Array of U_PMF_RECT structures

    EMF+ manual 2.2.2.38, Microsoft name: EmfPlusRect Object
*/
U_PSEUDO_OBJ *U_PMF_RECTN_set(uint32_t Elements, U_PMF_RECT *Rects){
   if(!Rects){ return(NULL); }
   uint32_t count = Elements;
   U_SERIAL_DESC *List = (U_SERIAL_DESC *) malloc((Elements + 2) * sizeof(U_SERIAL_DESC));
   U_SERIAL_DESC *Lptr = List;
   if(!List){ return(NULL); }
   *Lptr++ = (U_SERIAL_DESC){&Elements, 4, 1, U_LE};
   for(; count; count--, Lptr++, Rects++){
      *Lptr = (U_SERIAL_DESC){Rects, 2, 4, U_LE};
   }
   *Lptr = (U_SERIAL_DESC){NULL,0,0,U_XX};
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_RECT_OID | U_PMF_ARRAY_OID, List);
   free(List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_RECTF object in a PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  X          UL X value
    \param  Y          UL Y value
    \param  Width      Width
    \param  Height     Height

    EMF+ manual 2.2.2.39, Microsoft name: EmfPlusRectF Object
*/
U_PSEUDO_OBJ *U_PMF_RECTF4_set(U_FLOAT X, U_FLOAT Y, U_FLOAT Width, U_FLOAT Height){
   const U_SERIAL_DESC List[] = {
      {&X,      4, 1, U_LE},
      {&Y,      4, 1, U_LE},
      {&Width,  4, 1, U_LE},
      {&Height, 4, 1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_RECTF_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_RECTF object in a PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Rect      U_PMF_RECTF structure

    EMF+ manual 2.2.2.39, Microsoft name: EmfPlusRectF Object
*/
U_PSEUDO_OBJ *U_PMF_RECTF_set(U_PMF_RECTF *Rect){
   const U_SERIAL_DESC List[] = {
      {Rect,    4, 4, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_RECTF_OID, List);
   return(po);
}

/**
    \brief  Create and set an array of U_PMF_RECTF objects in a PseudoObject
    \return Pointer to PseudoObject, NULL on error.  PseudoObject contains Elements followed by the array of U_PMF_RECTF objects.
    \param  Elements   Number of elements in Rects
    \param  Rects      Array of U_PMF_RECTF structures

    EMF+ manual 2.2.2.39, Microsoft name: EmfPlusRectF Object
*/
U_PSEUDO_OBJ *U_PMF_RECTFN_set(uint32_t Elements, U_PMF_RECTF *Rects){
   if(!Rects){ return(NULL); }
   uint32_t count = Elements;
   U_SERIAL_DESC *List = (U_SERIAL_DESC *) malloc((Elements + 2) * sizeof(U_SERIAL_DESC));
   U_SERIAL_DESC *Lptr = List;
   if(!List){ return(NULL); }
   *Lptr++ = (U_SERIAL_DESC){&Elements, 4, 1, U_LE};
   for(; count; count--, Lptr++, Rects++){
     *Lptr = (U_SERIAL_DESC){Rects, 4, 4, U_LE}; 
   }
   *Lptr = (U_SERIAL_DESC){NULL,0,0,U_XX};
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_RECTF_OID | U_PMF_ARRAY_OID, List);
   free(List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_REGIONNODE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Type      RegionNodeDataType Enumeration
    \param  Rnd       (optional) U_PSEUDO_OBJ containing a U_PMF_REGIONNODEPATH, U_PMF_RECTF, or U_PMF_REGIONNODECHILDNODES object (Region Node Data)
     

    EMF+ manual 2.2.2.40, Microsoft name: EmfPlusRegionNode Object
*/
U_PSEUDO_OBJ *U_PMF_REGIONNODE_set(int32_t Type, const U_PSEUDO_OBJ *Rnd){
   int32_t pType;
   /* make sure that the type of Rnd agrees with Type */
   if(Rnd){
      pType = U_OID_To_RNDT(Rnd->Type);
      if( pType <  0){                     return(NULL); }
      if((pType >  0) && (pType != Type)){ return(NULL); }
      if((pType == 0) && 
         (
            (Type < U_RNDT_And) || 
            (Type > U_RNDT_Complement)
         )
      ){                                   return(NULL); }
      if((Type == U_RNDT_Rect) && (Rnd->Type != U_PMF_RECTF_OID)){           return(NULL); }
      if((Type == U_RNDT_Path) && (Rnd->Type != U_PMF_REGIONNODEPATH_OID)){  return(NULL); }
      
   }
   else {  /* only U_RNDT_Empty and  U_RNDT_Infinite do not have data */
      if((Type != U_RNDT_Empty)  || 
         (Type != U_RNDT_Infinite)  ){     return(NULL); }
   }
   

   const U_SERIAL_DESC List[] = {
      {&Type,                    4,                     1, U_LE},
      {(Rnd ? Rnd->Data : NULL), (Rnd ? Rnd->Used : 0), 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_REGIONNODE_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_REGIONNODECHILDNODES PseudoObject
    \return Pointer to PseudoObject containing a U_PMF_REGIONNODECHILDNODES_OID object, NULL on error
    \param  Left      U_PSEUDO_OBJ containing a U_PMF_REGIONNODE object
    \param  Right     U_PSEUDO_OBJ containing a U_PMF_REGIONNODE object
     

    EMF+ manual 2.2.2.41, Microsoft name: EmfPlusRegionNodeChildNodes Object
*/
U_PSEUDO_OBJ *U_PMF_REGIONNODECHILDNODES_set(const U_PSEUDO_OBJ *Left, const U_PSEUDO_OBJ *Right){
   if(!Left  || (Left->Type  != U_PMF_REGIONNODE_OID)){ return(NULL); }
   if(!Right || (Right->Type != U_PMF_REGIONNODE_OID)){ return(NULL); }

   const U_SERIAL_DESC List[] = {
      {Left->Data,  Left->Used,  1, U_XE},
      {Right->Data, Right->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_REGIONNODECHILDNODES_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_REGIONNODEPATH PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Path     U_PSEUDO_OBJ containing a U_PMF_PATH object
     

    EMF+ manual 2.2.2.42, Microsoft name: EmfPlusRegionNodePath Object
*/
U_PSEUDO_OBJ *U_PMF_REGIONNODEPATH_set(const U_PSEUDO_OBJ *Path){
   if(!Path  || (Path->Type  != U_PMF_PATH_OID)){ return(NULL); }
   /* PO Used is size_t, might be 8 bytes, value in record must be 4 bytes */
   uint32_t Used = Path->Used;

   const U_SERIAL_DESC List[] = {
      {&Used,      4,	       1, U_LE},
      {Path->Data, Path->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_REGIONNODEPATH_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_SOLIDBRUSHDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Color     U_PSEUDO_OBJ containing a U_PMF_ARGB object
     

    EMF+ manual 2.2.2.43, Microsoft name: EmfPlusSolidBrushData Object
*/
U_PSEUDO_OBJ *U_PMF_SOLIDBRUSHDATA_set(const U_PSEUDO_OBJ *Color){
   if(!Color || (Color->Type != U_PMF_ARGB_OID)){ return(NULL); }

   const U_SERIAL_DESC List[] = {
      {Color->Data, Color->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_SOLIDBRUSHDATA_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_STRINGFORMATDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  TabStopCount  Entries in TabStop array
    \param  TabStops      (optional) Array of tabstop locations
    \param  Ranges        (optional) U_PSEUDO_OBJ containing an array of U_PMF_CHARACTERRANGE objects
     

    EMF+ manual 2.2.2.44, Microsoft name: EmfPlusStringFormatData Object
*/
U_PSEUDO_OBJ *U_PMF_STRINGFORMATDATA_set(uint32_t TabStopCount, U_FLOAT *TabStops, const U_PSEUDO_OBJ *Ranges){
   if(Ranges && (Ranges->Type  != (U_PMF_CHARACTERRANGE_OID | U_PMF_ARRAY_OID))){ return(NULL); }

   const U_SERIAL_DESC List[] = {
      {TabStops,                       TabStopCount*4,              1, U_LE},
      {(Ranges ? Ranges->Data : NULL), (Ranges ? Ranges->Used : 0), 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_STRINGFORMATDATA_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_TEXTUREBRUSHDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Flags         BrushData flags
    \param  WrapMode      WrapMode enumeration
    \param  Tbod          U_PSEUDO_OBJ containing an U_PMF_TEXTUREBRUSHOPTIONALDATA object
     

    EMF+ manual 2.2.2.45, Microsoft name: EmfPlusTextureBrushData Object
*/
U_PSEUDO_OBJ *U_PMF_TEXTUREBRUSHDATA_set(uint32_t Flags, uint32_t WrapMode, const U_PSEUDO_OBJ *Tbod){
   if(Flags & ~U_BD_MASKB){ return(NULL); }  /* a bit was set that is not supported for this record */
   if(WrapMode > U_WM_Clamp){ return(NULL); }
   if(!Tbod || (Tbod->Type  != (U_PMF_TEXTUREBRUSHOPTIONALDATA_OID))){ return(NULL); }

   const U_SERIAL_DESC List[] = {
      {&Flags,     4,          1, U_LE},
      {&WrapMode,  4,          1, U_LE},
      {Tbod->Data, Tbod->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_TEXTUREBRUSHDATA_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_TEXTUREBRUSHOPTIONALDATA PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Tm            (optional) U_PSEUDO_OBJ containing an U_PMF_TRANSFORMMATRIX object
    \param  Image         (optional) U_PSEUDO_OBJ containing an U_PMF_IMAGE object
     

    EMF+ manual 2.2.2.46, Microsoft name: EmfPlusTextureBrushOptionalData Object
*/
U_PSEUDO_OBJ *U_PMF_TEXTUREBRUSHOPTIONALDATA_set(const U_PSEUDO_OBJ *Tm, const U_PSEUDO_OBJ *Image){
   if(Tm    && (Tm->Type    != (U_PMF_TRANSFORMMATRIX_OID))){ return(NULL); }
   if(Image && (Image->Type != (U_PMF_IMAGE_OID))){           return(NULL); }

   const U_SERIAL_DESC List[] = {
      {(Tm ? Tm->Data : NULL),       (Tm ? Tm->Used : 0),       1, U_XE},
      {(Image ? Image->Data : NULL), (Image ? Image->Used : 0), 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_TEXTUREBRUSHOPTIONALDATA_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_TRANSFORMMATRIX PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Tm            U_PMF_TRANSFORMMATRIX_ object
     

    EMF+ manual 2.2.2.47, Microsoft name: EmfPlusTransformMatrix Object
*/
U_PSEUDO_OBJ *U_PMF_TRANSFORMMATRIX_set(U_PMF_TRANSFORMMATRIX *Tm){
   if(!Tm){ return(NULL); }

   const U_SERIAL_DESC List[] = {
      {Tm, 4, 6, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_TRANSFORMMATRIX_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_IE_BLUR PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Radius     Blur radius in pixels
    \param  ExpandEdge 1: expand bitmap by Radius; 0: bitmap size unchanged
     

    EMF+ manual 2.2.3.1, Microsoft name: BlurEffect Object
*/
U_PSEUDO_OBJ *U_PMF_IE_BLUR_set(U_FLOAT Radius, uint32_t ExpandEdge){

   const U_SERIAL_DESC List[] = {
      {&Radius,     4, 1, U_LE},
      {&ExpandEdge, 4, 1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_IE_BLUR_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_IE_BRIGHTNESSCONTRAST PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Brightness  -255 to 255, 0 is unchanged, positive increases, negative decreases
    \param  Contrast    -100 to 100, 0 is unchanged, positive increases, negative decreases
     

    EMF+ manual 2.2.3.2, Microsoft name: BrightnessContrastEffect Object
*/
U_PSEUDO_OBJ *U_PMF_IE_BRIGHTNESSCONTRAST_set(int32_t Brightness, int32_t Contrast){

   const U_SERIAL_DESC List[] = {
      {&Brightness, 4, 1, U_LE},
      {&Contrast,   4, 1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_IE_BRIGHTNESSCONTRAST_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_IE_COLORBALANCE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  CyanRed      -100 to 100, 0 is unchanged, positive increases Red   & decreases Cyan,    negative is opposite
    \param  MagentaGreen -100 to 100, 0 is unchanged, positive increases Green & decreases Magenta, negative is opposite
    \param  YellowBlue   -100 to 100, 0 is unchanged, positive increases Blue  & decreases Yellow,  negative is opposite
     

    EMF+ manual 2.2.3.3, Microsoft name: ColorBalanceEffect Object
*/
U_PSEUDO_OBJ *U_PMF_IE_COLORBALANCE_set(int32_t CyanRed, int32_t MagentaGreen, int32_t YellowBlue){

   const U_SERIAL_DESC List[] = {
      {&CyanRed,      4, 1, U_LE},
      {&MagentaGreen, 4, 1, U_LE},
      {&YellowBlue,   4, 1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_IE_COLORBALANCE_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_IE_COLORCURVE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Adjust       CurveAdjustment enumeration
    \param  Channel      CurveChannel enumeration
    \param  Intensity    adjustment to apply.  "Adjust" determines what field this is and range values.


    EMF+ manual 2.2.3.4, Microsoft name: ColorCurveEffect Object
*/
U_PSEUDO_OBJ *U_PMF_IE_COLORCURVE_set(uint32_t Adjust, uint32_t Channel, int32_t Intensity){

   const U_SERIAL_DESC List[] = {
      {&Adjust,    4, 1, U_LE},
      {&Channel,   4, 1, U_LE},
      {&Intensity, 4, 1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_IE_COLORCURVE_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_IE_COLORLOOKUPTABLE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  BLUT       Blue  color lookup table
    \param  GLUT       Green color lookup table
    \param  RLUT       Red   color lookup table
    \param  ALUT       Alpha color lookup table
    
     

    EMF+ manual 2.2.3.5, Microsoft name: ColorLookupTableEffect Object.
    All tables have 256 entries.
*/
U_PSEUDO_OBJ *U_PMF_IE_COLORLOOKUPTABLE_set(const uint8_t *BLUT, const uint8_t *GLUT, const uint8_t *RLUT, const uint8_t *ALUT){
   if(!BLUT || !GLUT || !RLUT || !ALUT)return(NULL);

   const U_SERIAL_DESC List[] = {
      {BLUT, 1, 256, U_XE},
      {GLUT, 1, 256, U_XE},
      {RLUT, 1, 256, U_XE},
      {ALUT, 1, 256, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_IE_COLORLOOKUPTABLE_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_IE_COLORMATRIX PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Matrix     5 x 5 color transformation matrix, First 4 rows are [{4 multiplier values},0.0] for R,G,B,A, last Row is [{4 color translation valuess}, 1.0]
     

    EMF+ manual 2.2.3.6, Microsoft name: ColorMatrixEffect Object
*/
U_PSEUDO_OBJ *U_PMF_IE_COLORMATRIX_set(const U_FLOAT *Matrix){
   if(!Matrix)return(NULL);

   const U_SERIAL_DESC List[] = {
      {Matrix, 4, 25, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_IE_COLORMATRIX_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_IE_HUESATURATIONLIGHTNESS PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Hue          -180 to 180, 0 is unchanged
    \param  Saturation   -100 to 100, 0 is unchanged
    \param  Lightness    -100 to 100, 0 is unchanged


    EMF+ manual 2.2.3.7, Microsoft name: HueSaturationLightnessEffect Object
*/
U_PSEUDO_OBJ *U_PMF_IE_HUESATURATIONLIGHTNESS_set(int32_t Hue, int32_t Saturation, int32_t Lightness){

   const U_SERIAL_DESC List[] = {
      {&Hue,        4, 1, U_LE},
      {&Saturation, 4, 1, U_LE},
      {&Lightness,  4, 1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_IE_HUESATURATIONLIGHTNESS_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_IE_LEVELS PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Highlight      0 to 100, 100 is unchanged
    \param  Midtone     -100 to   0, 0 is unchanged
    \param  Shadow         0 to 100, 0 is unchanged


    EMF+ manual 2.2.3.8, Microsoft name: LevelsEffect Object
*/
U_PSEUDO_OBJ *U_PMF_IE_LEVELS_set(int32_t Highlight, int32_t Midtone, int32_t Shadow){

   const U_SERIAL_DESC List[] = {
      {&Highlight, 4, 1, U_LE},
      {&Midtone,   4, 1, U_LE},
      {&Shadow,    4, 1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_IE_LEVELS_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_IE_REDEYECORRECTION PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Elements   Number of members in Rects
    \param  Rects      Array of U_RECTL rectangular area(s) to apply red eye correction


    EMF+ manual 2.2.3.9, Microsoft name: RedEyeCorrectionEffect Object
*/
U_PSEUDO_OBJ *U_PMF_IE_REDEYECORRECTION_set(uint32_t Elements, const U_RECTL *Rects){
   if(!Elements || !Rects){return(NULL);}

   const U_SERIAL_DESC List[] = {
      {&Elements, 4,   1,       U_LE},
      {Rects,     4*4, Elements,U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_IE_REDEYECORRECTION_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_IE_SHARPEN PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Radius     Sharpening radius in pixels
    \param  Sharpen    0 to 100, 0 is unchanged


    EMF+ manual 2.2.3.10, Microsoft name: SharpenEffect Object
*/
U_PSEUDO_OBJ *U_PMF_IE_SHARPEN_set(U_FLOAT Radius, int32_t Sharpen){

   const U_SERIAL_DESC List[] = {
      {&Radius,  4, 1, U_LE},
      {&Sharpen, 4, 1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_IE_SHARPEN_OID, List);
   return(po);
}

/**
    \brief  Create and set a U_PMF_IE_TINT PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Hue         -180 to 180, [positive==clockwise] rotation in degrees starting from blue
    \param  Amount      -100 [add black] to 100[add white], 0 is unchanged.  Change in hue on specified axis


    EMF+ manual 2.2.3.11, Microsoft name: TintEffect Object
*/
U_PSEUDO_OBJ *U_PMF_IE_TINT_set(const int32_t Hue, const int32_t Amount){

   const U_SERIAL_DESC List[] = {
      {&Hue,    4, 1, U_LE},
      {&Amount, 4, 1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMF_IE_TINT_OID, List);
   return(po);
}

//! \cond
/* internal routine, not part of the API.
   Returns a PseudoObject containing a U_PMR_CMN_HDR_OID object.
   The type is something like U_PMR_OFFSETCLIP (the record type, NOT the U_PMR_OFFSETCLIP_OID PseudoObject Type!). 
   The U_PMR_RECFLAG bit is added both in the data and in the Type of the PseudoObject.  
   If that bit is already set no harm, no foul.
*/

U_PSEUDO_OBJ *U_PMR_CMN_HDR_set(uint32_t Type, uint16_t Flags, uint32_t DataSize){

   uint32_t Size = 12 + UP4(DataSize); /* The header itself is always 12, PMR records must be a multiple of 4 */
   Type |= U_PMR_RECFLAG;
   uint16_t utmp16 = Type;
   const U_SERIAL_DESC List[] = {
      {&utmp16,   2, 1, U_LE},
      {&Flags,    2, 1, U_LE},    /* Microsoft EMF+ manual is BE, but this field in this implementation is  LE */
      {&Size,     4, 1, U_LE},
      {&DataSize, 4, 1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_CMN_HDR_OID, List);
   return(po);
}
//! \endcond


/**
    \brief  Create and set a U_PMR_STROKEFILLPATH PseudoObject
    \return Pointer to PseudoObject, NULL on error
    

    EMF+ manual mentioned in 2.1.1.1, not otherwise documented, Microsoft name: EmfPlusStrokeFillPath Record, Index 0x37
    
    "This record closes any open figures in a path, strokes the outline of 
     the path by using the current pen, and fills its interior by using the current brush."
     
*/
U_PSEUDO_OBJ *U_PMR_STROKEFILLPATH_set(void){
   int Size = 0;
   uint16_t utmp16 =  0;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_STROKEFILLPATH,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_STROKEFILLPATH_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_OFFSETCLIP PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  dX          horizontal translation offset to apply to clipping region
    \param  dY          vertical   translation offset to apply to clipping region


    EMF+ manual 2.3.1.1, Microsoft name: EmfPlusOffsetClip Record,  Index 0x35
*/
U_PSEUDO_OBJ *U_PMR_OFFSETCLIP_set(U_FLOAT dX, U_FLOAT dY){
   int Size = 2*4;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_OFFSETCLIP,0,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {&dX,      4,        1, U_LE},
      {&dY,      4,        1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_OFFSETCLIP_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_RESETCLIP PseudoObject
    \return Pointer to PseudoObject, NULL on error


    EMF+ manual 2.3.1.2, Microsoft name: EmfPlusResetClip Record, Index 0x31
*/
U_PSEUDO_OBJ *U_PMR_RESETCLIP_set(void){
   int Size = 0;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_RESETCLIP,0,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_RESETCLIP_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_SETCLIPPATH PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  CMenum      CombineMode enumeration..
    \param  PathID      U_PMF_PATH object in the EMF+ object table (0-63, inclusive)


    EMF+ manual 2.3.1.3, Microsoft name: EmfPlusSetClipPath Record, Index 0x33
*/
U_PSEUDO_OBJ *U_PMR_SETCLIPPATH_set(uint32_t PathID, uint32_t CMenum){
   if(PathID>63)return(NULL);
   int Size=0;
   uint16_t utmp16 =  ((PathID & U_FF_MASK_OID8) << U_FF_SHFT_OID8) | ((CMenum & U_FF_MASK_CM4) << U_FF_SHFT_CM4);
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_SETCLIPPATH,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_SETCLIPPATH_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_SETCLIPRECT PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  CMenum      CombineMode enumeration..
    \param  Rect        U_PSEUDO_OBJ containing an U_PMF_RECTF object or an array of U_PMF_RECTF objects (the first is used)
    

    EMF+ manual 2.3.1.4, Microsoft name: EmfPlusSetClipRect Record, Index 0x32
*/
U_PSEUDO_OBJ *U_PMR_SETCLIPRECT_set(uint32_t CMenum, const U_PSEUDO_OBJ *Rect){
   int Size=4*4;
   const char *start;
   uint16_t utmp16 = ((CMenum & U_FF_MASK_CM4) << U_FF_SHFT_CM4);
   if(Rect){
      if(        Rect->Type == U_PMF_RECTF_OID){ 
        start = Rect->Data;
      }
      else if(Rect->Type == (U_PMF_RECTF_OID | U_PMF_ARRAY_OID)){ 
        start = Rect->Data + 4;
      }
      else { return(0); }
   }
   else {    return(0); }

   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_SETCLIPRECT,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,  ph->Used, 1, U_XE},
      {start,     4,        4, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_SETCLIPRECT_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_SETCLIPREGION PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  PathID      U_PMF_PATH object in the EMF+ object table (0-63, inclusive)
    \param  CMenum      CombineMode enumeration..
    

    EMF+ manual 2.3.1.5, Microsoft name: EmfPlusSetClipRegion Record, Index 0x34
*/
U_PSEUDO_OBJ *U_PMR_SETCLIPREGION_set(uint32_t PathID, uint32_t CMenum){
   if(PathID>63)return(NULL);
   int Size=0;
   uint16_t utmp16 =  ((PathID & U_FF_MASK_OID8) << U_FF_SHFT_OID8) | ((CMenum & U_FF_MASK_CM4) << U_FF_SHFT_CM4);

   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_SETCLIPREGION,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_SETCLIPREGION_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_COMMENT PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  cbData      Number of bytes in Data, must be a multiple of 4
    \param  Data        Private data, may be anything.  Stored in PseudoObject without adjusting byte order.
    

    EMF+ manual 2.3.2.1, Microsoft name: EmfPlusComment Record, Index 0x03
*/
U_PSEUDO_OBJ *U_PMR_COMMENT_set(size_t cbData, const void *Data){
   if(UP4(cbData) != cbData){ return(NULL); }
   if(cbData && !Data){       return(NULL); }
   int Size=cbData;

   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_COMMENT,0,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {Data,     cbData,   1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_COMMENT_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_ENDOFFILE PseudoObject
    \return Pointer to PseudoObject, NULL on error


    EMF+ manual 2.3.3.1, Microsoft name: EmfPlusEndOfFile Record, Index 0x02
*/
U_PSEUDO_OBJ *U_PMR_ENDOFFILE_set(void){
   int Size=0;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_ENDOFFILE,0,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_ENDOFFILE_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_GETDC PseudoObject
    \return Pointer to PseudoObject, NULL on error


    EMF+ manual 2.3.3.2, Microsoft name: EmfPlusGetDC Record, Index 0x04
*/
U_PSEUDO_OBJ *U_PMR_GETDC_set(void){
   int Size=0;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_GETDC,0,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_GETDC_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_HEADER PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  IsDual       set = Dual-mode file, clear= EMF+ only file.
    \param  IsVideo      set = video device, clear= printer.  Ignore all other bits.
    \param  Version      U_PSEUDO_OBJ containing a U_PMF_GRAPHICSVERSION object
    \param  LogicalDpiX  Horizontal resolution reference device in DPI
    \param  LogicalDpiY  Vertical   resolution reference device in DPI
    

    EMF+ manual 2.3.3.3, Microsoft name: EmfPlusHeader Record, Index 0x01
*/
U_PSEUDO_OBJ *U_PMR_HEADER_set(int IsDual, int IsVideo, const U_PSEUDO_OBJ *Version, 
      uint32_t LogicalDpiX, uint32_t LogicalDpiY){
   if(!Version || (Version->Type != U_PMF_GRAPHICSVERSION_OID)){ return(NULL); }
   int Size=Version->Used + 3*4;
   uint16_t utmp16 =  (IsDual ? U_PPF_DM : 0);
   uint32_t Flags  =  (IsVideo ? U_PPF_VIDEO : 0);
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_HEADER,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,      ph->Used,      1, U_XE},
      {Version->Data, Version->Used, 1, U_XE},
      {&Flags,        4,             1, U_LE},
      {&LogicalDpiX,  4,             1, U_LE},
      {&LogicalDpiY,  4,             1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_HEADER_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_CLEAR PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Color        U_PSEUDO_OBJ containing a U_PMF_ARGB object.
    
   

    EMF+ manual 2.3.4.1, Microsoft name: EmfPlusClear Record, Index 0x09

    Erase everything preceding, set background ARGB to Color.
*/
U_PSEUDO_OBJ *U_PMR_CLEAR_set(const U_PSEUDO_OBJ *Color){
   if(!Color || (Color->Type != U_PMF_ARGB_OID)){ return(NULL); }
   int Size=Color->Used;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_CLEAR,0,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,    ph->Used,    1, U_XE},
      {Color->Data, Color->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_CLEAR_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_DRAWARC PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
    \param  Start       Start angle, >=0.0, degrees clockwise from 3:00
    \param  Sweep       Sweep angle, -360<= angle <=360, degrees clockwise from Start
    \param  Rect        U_PSEUDO_OBJ containing a U_PMF_RECT or U_PMF_RECTF object
   

    EMF+ manual 2.3.4.2, Microsoft name: EmfPlusDrawArc Record, Index 0x12
*/
U_PSEUDO_OBJ *U_PMR_DRAWARC_set(uint32_t PenID, U_FLOAT Start, U_FLOAT Sweep, const U_PSEUDO_OBJ *Rect){
   int ctype;
   if(PenID>63)return(NULL);
   if(!Rect){                                 return(NULL); }
   else {
      if(     Rect->Type == U_PMF_RECT_OID ){ ctype = 1;    }
      else if(Rect->Type == U_PMF_RECTF_OID){ ctype = 0;    }
      else {                                  return(NULL); }
   }
   int Size = 2*4 + Rect->Used;
   uint16_t utmp16 =  (ctype ? U_PPF_C : 0) | (PenID & U_FF_MASK_OID8) << U_FF_SHFT_OID8;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_DRAWARC,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,   ph->Used,   1, U_XE},
      {&Start,     4,          1, U_LE},
      {&Sweep,     4,          1, U_LE},
      {Rect->Data, Rect->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_DRAWARC_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_DRAWBEZIERS PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
    \param  Points      U_PSEUDO_OBJ containing first Elements, then a U_PMF_POINT, U_PMF_POINTR or U_PMF_POINTF object
   

    EMF+ manual 2.3.4.3, Microsoft name: EmfPlusDrawBeziers Record, Index 0x19
*/
U_PSEUDO_OBJ *U_PMR_DRAWBEZIERS_set(uint32_t PenID, const U_PSEUDO_OBJ *Points){
   int ctype, RelAbs;
   if(PenID>63)return(NULL);
   if(Points){
      if(     Points->Type == U_PMF_POINTR_OID){                     RelAbs = 1; ctype = 0; }
      else if(Points->Type == (U_PMF_POINT_OID  | U_PMF_ARRAY_OID)){ RelAbs = 0; ctype = 1; }
      else if(Points->Type == (U_PMF_POINTF_OID | U_PMF_ARRAY_OID)){ RelAbs = 0; ctype = 0; }
      else {                                                         return(NULL);          }
   }
   else {                                                            return(NULL);          }
   int Size = Points->Used;
   uint16_t utmp16 =  (ctype ? U_PPF_C : 0) | (RelAbs ? U_PPF_P : 0) |(PenID & U_FF_MASK_OID8) << U_FF_SHFT_OID8;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_DRAWBEZIERS,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,     ph->Used,     1, U_XE},
      {Points->Data, Points->Used, 1, U_XE}, /* Includes Elements */
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_DRAWBEZIERS_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_DRAWCLOSEDCURVE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
    \param  Tension     Controls splines, 0 is straight line, >0 is curved
    \param  Points      U_PSEUDO_OBJ containing a U_PMF_POINT, U_PMF_POINTR or U_PMF_POINTF object
   

    EMF+ manual 2.3.4.4, Microsoft name: EmfPlusDrawClosedCurve Record, Index 0x17

    Curve is a cardinal spline.

    References sent by MS support:

       http://alvyray.com/Memos/CG/Pixar/spline77.pdf

       http://msdn.microsoft.com/en-us/library/4cf6we5y(v=vs.110).aspx


*/
U_PSEUDO_OBJ *U_PMR_DRAWCLOSEDCURVE_set(uint32_t PenID, U_FLOAT Tension, const U_PSEUDO_OBJ *Points){
   int ctype, RelAbs;
   if(PenID>63)return(NULL);
   if(Points){
      if(     Points->Type == U_PMF_POINTR_OID){                     RelAbs = 1; ctype = 0; }
      else if(Points->Type == (U_PMF_POINT_OID  | U_PMF_ARRAY_OID)){ RelAbs = 0; ctype = 1; }
      else if(Points->Type == (U_PMF_POINTF_OID | U_PMF_ARRAY_OID)){ RelAbs = 0; ctype = 0; }
      else {                                                         return(NULL);          }
   }
   else {                                                            return(NULL);          }
   int Size = 4 + Points->Used;
   uint16_t utmp16 =  (ctype ? U_PPF_C : 0) | (RelAbs ? U_PPF_P : 0) |(PenID & U_FF_MASK_OID8) << U_FF_SHFT_OID8;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_DRAWCLOSEDCURVE,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,     ph->Used,     1, U_XE},
      {&Tension,     4,            1, U_LE},
      {Points->Data, Points->Used, 1, U_XE}, /* includes Elements*/
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_DRAWCLOSEDCURVE_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_DRAWCURVE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
    \param  Tension     Controls splines, 0 is straight line, >0 is curved
    \param  Offset      The starting point in the list of points, 0 is first.
    \param  NSegs       Number of segments to draw. Starting at Offset go NSegs straight lines, must not run out of points..
    \param  Points      U_PSEUDO_OBJ containing an element count then a series of U_PMF_POINT or U_PMF_POINTF object
   

    EMF+ manual 2.3.4.5, Microsoft name: EmfPlusDrawCurve Record, Index 0x18

    Curve is a cardinal spline, using doubled terminator points to generate curves for the terminal segments.

    References sent by MS support:

       http://alvyray.com/Memos/CG/Pixar/spline77.pdf

       http://msdn.microsoft.com/en-us/library/4cf6we5y(v=vs.110).aspx

*/
U_PSEUDO_OBJ *U_PMR_DRAWCURVE_set(uint32_t PenID, U_FLOAT Tension, uint32_t Offset, uint32_t NSegs, const U_PSEUDO_OBJ *Points){
   int ctype;
   if(PenID>63)return(NULL);
   if(!Points){                                                      return(NULL); }
   if(!NSegs){                                                       return(NULL); }
   else {
      if(     Points->Type == (U_PMF_POINT_OID  | U_PMF_ARRAY_OID)){ ctype = 1;    }
      else if(Points->Type == (U_PMF_POINTF_OID | U_PMF_ARRAY_OID)){ ctype = 0;    }
      else {                                                         return(NULL); }
   }
   uint32_t Elements = (Points->Used - 4)/(ctype ? 4 : 8);  /* This way do not need to worry about byte order */
   if(Offset + NSegs + 1 > Elements){   return(NULL); }
   int Size = 3*4 + Points->Used;
   uint16_t utmp16 =  (ctype ? U_PPF_C : 0) |(PenID & U_FF_MASK_OID8) << U_FF_SHFT_OID8;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_DRAWCURVE,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,         ph->Used,       1, U_XE},
      {&Tension,         4,              1, U_LE},
      {&Offset,          4,              1, U_LE},
      {&NSegs,           4,              1, U_LE},
      {Points->Data,     Points->Used,   1, U_XE}, /* Elements, points */
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_DRAWCURVE_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_DRAWDRIVERSTRING PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  FontID      U_PMF_FONT object in the EMF+ object table (0-63, inclusive)
    \param  BrushID     U_PSEUDO_OBJ containing a U_PMF_ARGB or a U_PMF_4NUM. Color or U_PMF_BRUSH object in the EMF+ object table (0-63, inclusive)
    \param  DSOFlags    DriverStringOptions flags
    \param  HasMatrix   If 1 record contains a TransformMatrix field, if 0 it does not.
    \param  GlyphCount  The number of Elements in Glyphs, must agree with the number of elements in Points.
    \param  Glyphs      If U_DSO_CmapLookup is set in DSOFlags this is an array of UTF16LE characters, otherwise, it is an array of indices into the U_PMF_FONT object indexed by Object_ID in flags.
    \param  Points      U_PSEUDO_OBJ containing a U_PMF_POINTF object
    \param  Tm          U_PSEUDO_OBJ containing a U_PMF_TRANSFORMMATRIX object. Apply to Glyphs & Positions. Present if HasMatrix is 1
   

    EMF+ manual 2.3.4.6, Microsoft name: EmfPlusDrawDriverString Record, Index 0x36
*/
U_PSEUDO_OBJ *U_PMR_DRAWDRIVERSTRING_set(uint32_t FontID, const U_PSEUDO_OBJ *BrushID, 
      uint32_t DSOFlags, uint32_t HasMatrix, uint32_t GlyphCount,
      const uint16_t *Glyphs, const U_PSEUDO_OBJ *Points, const U_PSEUDO_OBJ *Tm){
   int btype;
   if(FontID>63){                                                           return(NULL); }
   if(!Glyphs){                                                             return(NULL); }
   if(!Points || (Points->Type != (U_PMF_POINTF_OID | U_PMF_ARRAY_OID))){   return(NULL); }
   uint32_t Elements = (Points->Used -4)/4;
   if(GlyphCount != Elements){                                              return(NULL); } 
   if(BrushID){ 
      if(      BrushID->Used != 4){                                         return(NULL); }
      else if( BrushID->Type == U_PMF_ARGB_OID){                            btype = 1;    }
      else if( BrushID->Type == U_PMF_4NUM_OID){                            btype = 0;    }
      else {                                                                return(NULL); }
   }
   else {                                                                   return(NULL); } 
   int Size = 4 + BrushID->Used + 3*4 + Elements*2 + (Points->Used - 4);
   if(HasMatrix){
      if(!Tm){                                                              return(NULL); }
      else if(Tm->Type != (U_PMF_TRANSFORMMATRIX_OID)){                     return(NULL); }
      Size += Tm->Used;
   }
   uint16_t utmp16 =  (btype ? U_PPF_B : 0) |(FontID & U_FF_MASK_OID8) << U_FF_SHFT_OID8;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_DRAWDRIVERSTRING,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,ph->Used, 1, U_XE},
      {BrushID->Data,                  BrushID->Used,              1,        U_XE},
      {&DSOFlags,                      4,                          1,        U_LE},
      {&HasMatrix,                     4,                          1,        U_LE},
      {&Elements,                      4,                          1,        U_LE},
      {Glyphs,                         2,                          Elements, U_LE},
      {Points->Data + 4,               Points->Used - 4,           1,        U_XE},  /* omit Elements */
      {(HasMatrix ? Tm->Data : NULL),  (HasMatrix ? Tm->Used : 0), 1,        U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_DRAWDRIVERSTRING_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_DRAWELLIPSE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
    \param  Rect        U_PSEUDO_OBJ containing a U_PMF_RECT or U_PMF_RECTF object
   

    EMF+ manual 2.3.4.7, Microsoft name: EmfPlusDrawEllipse Record, Index 0x0F
*/
U_PSEUDO_OBJ *U_PMR_DRAWELLIPSE_set(uint32_t PenID, const U_PSEUDO_OBJ *Rect){
   if(PenID>63)return(NULL);
   int ctype;
   if(!Rect){                                 return(NULL); }
   else {
      if(     Rect->Type == U_PMF_RECT_OID ){ ctype = 1;    }
      else if(Rect->Type == U_PMF_RECTF_OID){ ctype = 0;    }
      else {                                  return(NULL); }
   }
   int Size = Rect->Used;
   uint16_t utmp16 =  (ctype ? U_PPF_C : 0) | (PenID & U_FF_MASK_OID8) << U_FF_SHFT_OID8;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_DRAWELLIPSE,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,   ph->Used,   1, U_XE},
      {Rect->Data, Rect->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_DRAWELLIPSE_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_DRAWIMAGE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  ImgID       U_PMF_IMAGE object in the EMF+ object table (0-63, inclusive)
    \param  ImgAttrID   index of a U_PMF_IMAGEATTRIBUTES object in the object table
    \param  SrcUnit     UnitType enumeration
    \param  SrcRect     U_PSEUDO_OBJ containing a U_PMF_RECTF object, Source region of image 
    \param  DstRect     U_PSEUDO_OBJ containing a U_PMF_RECT or U_PMF_RECTF object
   

    EMF+ manual 2.3.4.8, Microsoft name: EmfPlusDrawImage Record, Index 0x1A
*/
U_PSEUDO_OBJ *U_PMR_DRAWIMAGE_set(uint32_t ImgID,  
      int32_t ImgAttrID, int32_t SrcUnit, const U_PSEUDO_OBJ *SrcRect, const U_PSEUDO_OBJ *DstRect){
   int ctype;
   if(ImgID>63)return(NULL);
   if(!SrcRect || (SrcRect->Type != U_PMF_RECTF_OID)){  return(NULL); }
   if(!DstRect){                                 return(NULL); }
   else {
      if(     DstRect->Type == U_PMF_RECT_OID ){ ctype = 1;    }
      else if(DstRect->Type == U_PMF_RECTF_OID){ ctype = 0;    }
      else {                                     return(NULL); }
   }
   int Size = 2*4 + SrcRect->Used + DstRect->Used;
   uint16_t utmp16 =  (ctype ? U_PPF_C : 0) | (ImgID & U_FF_MASK_OID8) << U_FF_SHFT_OID8;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_DRAWIMAGE,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,      ph->Used,      1, U_XE},
      {&ImgAttrID,    4,             1, U_LE},
      {&SrcUnit,      4,             1, U_LE},
      {SrcRect->Data, SrcRect->Used, 1, U_XE},
      {DstRect->Data, DstRect->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_DRAWIMAGE_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_DRAWIMAGEPOINTS PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  ImgID       U_PMF_IMAGE object in the EMF+ object table (0-63, inclusive)
    \param  etype       Set: effect from previous U_PMR_SERIALIZABLEOBJECT record will be applied; Clear:  no effect applied
    \param  ImgAttrID   index of a U_PMF_IMAGEATTRIBUTES object in the object table
    \param  SrcUnit     UnitType enumeration
    \param  SrcRect     U_PSEUDO_OBJ containing a U_PMF_RECTF object, Source region of image 
    \param  Points      U_PSEUDO_OBJ containing an array of 3 (U_PMF_POINT, U_PMF_POINTF, or U_PMF_POINTF) objects. These points are the UL, UR, and LL vertices of a parallelogram.
   

    EMF+ manual 2.3.4.9, Microsoft name: EmfPlusDrawImagePoints Record, Index 0x1B


   WARNING!  Windows XP Preview does not show filter effects, whether or not U_PPF_E is set.   They are visible if the EMF+
   file is inserted as an image into PowerPoint.
*/
U_PSEUDO_OBJ *U_PMR_DRAWIMAGEPOINTS_set(uint32_t ImgID, int etype,
      int32_t ImgAttrID, int32_t SrcUnit, const U_PSEUDO_OBJ *SrcRect,
      const U_PSEUDO_OBJ *Points){
   int ctype, RelAbs;
   if(ImgID>63){                                                      return(NULL); }
   if(!SrcRect || (SrcRect->Type != U_PMF_RECTF_OID)){                return(NULL); }
   if(Points){
      if(     Points->Type == U_PMF_POINTR_OID){                     RelAbs = 1; ctype = 0; }
      else if(Points->Type == (U_PMF_POINT_OID  | U_PMF_ARRAY_OID)){ RelAbs = 0; ctype = 1; }
      else if(Points->Type == (U_PMF_POINTF_OID | U_PMF_ARRAY_OID)){ RelAbs = 0; ctype = 0; }
      else {                                                         return(NULL);          }
   }
   else {                                                            return(NULL);          }
   int Size = 2*4 + SrcRect->Used + Points->Used;
   uint16_t utmp16 =  (ctype ? U_PPF_C : 0) | (etype ? U_PPF_E : 0) | (RelAbs ? U_PPF_P : 0) | (ImgID & U_FF_MASK_OID8) << U_FF_SHFT_OID8;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_DRAWIMAGEPOINTS,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,      ph->Used,      1, U_XE},
      {&ImgAttrID,    4,             1, U_LE},
      {&SrcUnit,      4,             1, U_LE},
      {SrcRect->Data, SrcRect->Used, 1, U_XE},
      {Points->Data,  Points->Used,  1, U_XE}, /* includes Elements*/
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_DRAWIMAGEPOINTS_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_DRAWLINES PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
    \param  dtype       Set: path must be closed, Clear: path is open
    \param  Points      U_PSEUDO_OBJ containing an array of 3 U_PMF_POINT, U_PMF_POINTR, or U_PMF_POINTF objects
   

    EMF+ manual 2.3.4.10, Microsoft name: EmfPlusDrawLines Record, Index 0x0D
*/
U_PSEUDO_OBJ *U_PMR_DRAWLINES_set(uint32_t PenID, int dtype, const U_PSEUDO_OBJ *Points){
   int ctype, RelAbs;
   if(PenID>63){                                                      return(NULL); }
   if(Points){
      if(     Points->Type == U_PMF_POINTR_OID){                     RelAbs = 1; ctype = 0; }
      else if(Points->Type == (U_PMF_POINT_OID  | U_PMF_ARRAY_OID)){ RelAbs = 0; ctype = 1; }
      else if(Points->Type == (U_PMF_POINTF_OID | U_PMF_ARRAY_OID)){ RelAbs = 0; ctype = 0; }
      else {                                                         return(NULL);          }
   }
   else {                                                            return(NULL);          }
   int Size = Points->Used;
   uint16_t utmp16 =  (ctype ? U_PPF_C : 0) | (dtype ? U_PPF_D : 0) | (RelAbs ? U_PPF_P : 0) | (PenID & U_FF_MASK_OID8) << U_FF_SHFT_OID8;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_DRAWLINES,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,     ph->Used,     1, U_XE},
      {Points->Data, Points->Used, 1, U_XE}, /* includes Elements */
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_DRAWLINES_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_DRAWPATH PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  PathID      U_PMF_PATH object in the EMF+ object table (0-63, inclusive)
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
    

    EMF+ manual 2.3.4.11, Microsoft name: EmfPlusDrawPath Record, Index 0x15
*/
U_PSEUDO_OBJ *U_PMR_DRAWPATH_set(uint32_t PathID, uint32_t PenID){
   if(PathID>63)return(NULL);
   if(PenID>63)return(NULL);
   int Size = 4;
   uint16_t utmp16 =  (PathID & U_FF_MASK_OID8) << U_FF_SHFT_OID8;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_DRAWPATH,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {&PenID,   4,        1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_DRAWPATH_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_DRAWPIE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
    \param  Start       Start angle, >=0.0, degrees clockwise from 3:00
    \param  Sweep       Sweep angle, -360<= angle <=360, degrees clockwise from Start
    \param  Rect        U_PSEUDO_OBJ containing a U_PMF_RECT or U_PMF_RECTF object
   

    EMF+ manual 2.3.4.12, Microsoft name: EmfPlusDrawPie Record, Index 0x0D
*/
U_PSEUDO_OBJ *U_PMR_DRAWPIE_set(uint32_t PenID, 
      U_FLOAT Start, U_FLOAT Sweep, const U_PSEUDO_OBJ *Rect){
   int ctype;
   if(PenID>63)return(NULL);
   if(!Rect){                                 return(NULL); }
   else {
      if(     Rect->Type == U_PMF_RECT_OID ){ ctype = 1;    }
      else if(Rect->Type == U_PMF_RECTF_OID){ ctype = 0;    }
      else {                                  return(NULL); }
   }
   int Size = 2*4 + Rect->Used;
   uint16_t utmp16 =  (ctype ? U_PPF_C : 0) | (PenID & U_FF_MASK_OID8) << U_FF_SHFT_OID8;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_DRAWPIE,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,   ph->Used,   1, U_XE},
      {&Start,     4,          1, U_LE},
      {&Sweep,     4,          1, U_LE},
      {Rect->Data, Rect->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_DRAWPIE_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_DRAWRECTS PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
    \param  Rects       U_PSEUDO_OBJ containing 1 rect OR  a count N follwed by N rects.  Rects may be either U_PMF_RECT or U_PMF_RECTF
   

    EMF+ manual 2.3.4.13, Microsoft name: EmfPlusDrawRects Record, Index 0x0B
*/
U_PSEUDO_OBJ *U_PMR_DRAWRECTS_set(uint32_t PenID, const U_PSEUDO_OBJ *Rects){
   int ctype;
   int just1;
   uint32_t Elements=1; /* only used when a single rect is passed in, not an array, not even an array with one member*/
   if(PenID>63){ return(NULL); }
   if(Rects){
      if(      (Rects->Type & U_PMF_MASK_OID) == U_PMF_RECT_OID ){   ctype = 1;    }
      else if( (Rects->Type & U_PMF_MASK_OID) == U_PMF_RECTF_OID){   ctype = 0;    }
      else {                                                         return(NULL); }
   }
   else {                                                            return(NULL); }
   just1 = (Rects->Type & U_PMF_ARRAY_OID ? 0 : 1);
   int Size = Rects->Used + (just1 ? 4 : 0); /* Elements in Rects for array, not for single */
   uint16_t utmp16 =  (ctype ? U_PPF_C : 0)| (PenID & U_FF_MASK_OID8) << U_FF_SHFT_OID8;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_DRAWRECTS,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,                           ph->Used,       1, U_XE},
      {(just1 ? (char *)&Elements : NULL), (just1 ? 4: 0), 1, U_LE}, /* element count if a single Rect was passed in, empty otherwise */ 
      {Rects->Data,                        Rects->Used,    1, U_XE}, /* Elements + Array, already stored in Rects, if an array was passed in, just rect if a single */
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_DRAWRECTS_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_DRAWSTRING PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  FontID      U_PMF_FONT object in the EMF+ object table (0-63, inclusive)
    \param  BrushID     U_PSEUDO_OBJ containing a U_PMF_ARGB or a U_PMF_4NUM. Color or U_PMF_BRUSH object in the EMF+ object table (0-63, inclusive)
    \param  FormatID    U_PMF_STRINGFORMAT object in EMF+ Object Table.
    \param  Length      Number of characters in the string.
    \param  Rect        U_PSEUDO_OBJ containing a U_PMF_RECTF object, string's bounding box
    \param  Text        Array of UFT-16LE unicode characters.
   

    EMF+ manual 2.3.4.14, Microsoft name: EmfPlusDrawString Record, Index 0x1C
*/
U_PSEUDO_OBJ *U_PMR_DRAWSTRING_set(uint32_t FontID, const U_PSEUDO_OBJ *BrushID, 
      uint32_t FormatID, uint32_t Length, const U_PSEUDO_OBJ *Rect, const uint16_t *Text){
   int btype; 
   if(FontID>63){                                 return(NULL); }
   if(!Length){                                   return(NULL); }
   else if (!Text){                               return(NULL); }
   if(!Rect || Rect->Type != U_PMF_RECTF_OID){    return(NULL); }
   if(BrushID){  
      if(      BrushID->Used != 4){               return(NULL); }
      else if( BrushID->Type == U_PMF_ARGB_OID){  btype = 1;    }
      else if( BrushID->Type == U_PMF_4NUM_OID){  btype = 0;    }
      else {                                      return(NULL); }
   }
   else {                                         return(NULL); }
   int Size = BrushID->Used + 2*4 + Rect->Used +2*Length;
   uint16_t utmp16 =  (btype ? U_PPF_B : 0)| (FontID & U_FF_MASK_OID8) << U_FF_SHFT_OID8;
   uint32_t pad = (0x1 & Length ? 2 : 0);
   Size+=pad;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_DRAWSTRING,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,      ph->Used,      1,             U_XE},
      {BrushID->Data, BrushID->Used, 1,             U_XE},
      {&FormatID,     4,             1,             U_LE},
      {&Length,       4,             1,             U_LE},
      {Rect->Data,    Rect->Used,    1,             U_XE},
      {Text,          2*Length,      1,             U_XE},
      {NULL,          pad,           (pad ? 1 : 0), (pad ? U_XE : U_XX)}, /* Entire record must be a multiple of 4 */
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_DRAWSTRING_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_FILLCLOSEDCURVE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  ftype       If U_WINDING use winding fill, else use fill
    \param  BrushID     U_PSEUDO_OBJ containing a U_PMF_ARGB or a U_PMF_4NUM. Color or U_PMF_BRUSH object in the EMF+ object table (0-63, inclusive)
    \param  Tension     Controls splines, 0 is straight line, >0 is curved
    \param  Points      U_PSEUDO_OBJ containing a U_PMF_POINT, U_PMF_POINTR or U_PMF_POINTF object
   

    EMF+ manual 2.3.4.15, Microsoft name: EmfPlusFillClosedCurve Record, Index 0x16
*/
U_PSEUDO_OBJ *U_PMR_FILLCLOSEDCURVE_set(int ftype, U_FLOAT Tension, const U_PSEUDO_OBJ *BrushID, const U_PSEUDO_OBJ *Points){
   int btype, ctype, RelAbs;
   int Size=0;
   if(BrushID){ 
      if(      BrushID->Used != 4){              return(NULL); }
      else if( BrushID->Type == U_PMF_ARGB_OID){ btype = 1;    }
      else if( BrushID->Type == U_PMF_4NUM_OID){ btype = 0;    }
      else {                                     return(NULL); }
   }
   else {                                        return(NULL); }
   if(Points){
      if(     Points->Type == U_PMF_POINTR_OID){                     RelAbs = 1; ctype = 0; }
      else if(Points->Type == (U_PMF_POINT_OID  | U_PMF_ARRAY_OID)){ RelAbs = 0; ctype = 1; }
      else if(Points->Type == (U_PMF_POINTF_OID | U_PMF_ARRAY_OID)){ RelAbs = 0; ctype = 0; }
      else {                                                         return(NULL);          }
   }
   else {                                                            return(NULL);          }
   Size = BrushID->Used + 4 + Points->Used;
   uint16_t utmp16 =  (btype ? U_PPF_B : 0) | (ctype ? U_PPF_C : 0) |((ftype == U_WINDING) ? U_PPF_F : 0) |(RelAbs ? U_PPF_P : 0);
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_FILLCLOSEDCURVE,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,      ph->Used,      1, U_XE},
      {BrushID->Data, BrushID->Used, 1, U_XE},
      {&Tension,      4,             1, U_LE},
      {Points->Data,  Points->Used,  1, U_XE}, /* includes Elements */
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_FILLCLOSEDCURVE_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_FILLELLIPSE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  BrushID     U_PSEUDO_OBJ containing a U_PMF_ARGB or a U_PMF_4NUM. Color or U_PMF_BRUSH object in the EMF+ object table (0-63, inclusive)
    \param  Rect        U_PSEUDO_OBJ containing a U_PMF_RECT or U_PMF_RECTF object
   

    EMF+ manual 2.3.4.16, Microsoft name: EmfPlusFillEllipse Record, Index 0x0E
*/
U_PSEUDO_OBJ *U_PMR_FILLELLIPSE_set(const U_PSEUDO_OBJ *BrushID, const U_PSEUDO_OBJ *Rect){
   int btype, ctype;
   if(BrushID){ 
      if(      BrushID->Used != 4){              return(NULL); }
      else if( BrushID->Type == U_PMF_ARGB_OID){ btype = 1;    }
      else if( BrushID->Type == U_PMF_4NUM_OID){ btype = 0;    }
      else {                                     return(NULL); }
   }
   else {                                        return(NULL); } 
   if(Rect){
      if(     Rect->Type == U_PMF_RECT_OID ){    ctype = 1;    }
      else if(Rect->Type == U_PMF_RECTF_OID){    ctype = 0;    }
      else {                                     return(NULL); }
   }
   else {                                        return(NULL); }
   int Size = BrushID->Used + Rect->Used;
   uint16_t utmp16 =  (btype ? U_PPF_B : 0) | (ctype ? U_PPF_C : 0);
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_FILLELLIPSE,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,      ph->Used,      1, U_XE},
      {BrushID->Data, BrushID->Used, 1, U_XE},
      {Rect->Data,    Rect->Used,    1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_FILLELLIPSE_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_FILLPATH PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  PathID      U_PMF_PATH object in the EMF+ object table (0-63, inclusive)
    \param  BrushID     U_PSEUDO_OBJ containing a U_PMF_ARGB or a U_PMF_4NUM. Color or U_PMF_BRUSH object in the EMF+ object table (0-63, inclusive)
    

    EMF+ manual 2.3.4.17, Microsoft name: EmfPlusFillPath Record, Index 0x14
*/
U_PSEUDO_OBJ *U_PMR_FILLPATH_set(uint32_t PathID, const U_PSEUDO_OBJ *BrushID){
   int btype;
   int Size=0;
   if(PathID>63)return(NULL);
   if(BrushID){ 
      if(      BrushID->Used != 4){              return(NULL); }
      else if( BrushID->Type == U_PMF_ARGB_OID){ btype = 1;    }
      else if( BrushID->Type == U_PMF_4NUM_OID){ btype = 0;    }
      else {                                     return(NULL); }
   }
   else {                                        return(NULL); }
   Size = BrushID->Used;
   uint16_t utmp16 =  (btype ? U_PPF_B : 0) | (PathID & U_FF_MASK_OID8) << U_FF_SHFT_OID8 ;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_FILLPATH,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,      ph->Used,      1, U_XE},
      {BrushID->Data, BrushID->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_FILLPATH_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_FILLPIE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Start       Start angle, >=0.0, degrees clockwise from 3:00
    \param  Sweep       Sweep angle, -360<= angle <=360, degrees clockwise from Start
    \param  BrushID     U_PSEUDO_OBJ containing a U_PMF_ARGB or a U_PMF_4NUM. Color or U_PMF_BRUSH object in the EMF+ object table (0-63, inclusive)
    \param  Rect        U_PSEUDO_OBJ containing a U_PMF_RECT or U_PMF_RECTF object
   

    EMF+ manual 2.3.4.18, Microsoft name: EmfPlusFillPie Record, Index 0x10
*/
U_PSEUDO_OBJ *U_PMR_FILLPIE_set(U_FLOAT Start, U_FLOAT Sweep, const U_PSEUDO_OBJ *BrushID, const U_PSEUDO_OBJ *Rect){
   int btype, ctype;
   if(BrushID){ 
      if(      BrushID->Used != 4){              return(NULL); }
      else if( BrushID->Type == U_PMF_ARGB_OID){ btype = 1;    }
      else if( BrushID->Type == U_PMF_4NUM_OID){ btype = 0;    }
      else {                                     return(NULL); }
   }
   else {                                        return(NULL); } 
   if(!Rect){                                    return(NULL); }
   else {
      if(     Rect->Type == U_PMF_RECT_OID ){    ctype = 1;    }
      else if(Rect->Type == U_PMF_RECTF_OID){    ctype = 0;    }
      else {                                     return(NULL); }
   }
   int Size = BrushID->Used + 2*4 + Rect->Used;
   uint16_t utmp16 =  (btype ? U_PPF_B : 0) | (ctype ? U_PPF_C : 0);
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_FILLPIE,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,      ph->Used,      1, U_XE},
      {BrushID->Data, BrushID->Used, 1, U_XE},
      {&Start,        4,             1, U_LE},
      {&Sweep,        4,             1, U_LE},
      {Rect->Data,    Rect->Used,    1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_FILLPIE_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_FILLPOLYGON PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  BrushID     U_PSEUDO_OBJ containing a U_PMF_ARGB or a U_PMF_4NUM. Color or U_PMF_BRUSH object in the EMF+ object table (0-63, inclusive)
    \param  Points      U_PSEUDO_OBJ containing an array of 3 U_PMF_POINT, U_PMF_POINTR, or U_PMF_POINTF objects
   

    EMF+ manual 2.3.4.19, Microsoft name: EmfPlusFillPolygon Record, Index 0x0C
*/
U_PSEUDO_OBJ *U_PMR_FILLPOLYGON_set(const U_PSEUDO_OBJ *BrushID, const U_PSEUDO_OBJ *Points){
   int btype, ctype, RelAbs;
   if(BrushID){ 
      if(      BrushID->Used != 4){                                  return(NULL); }
      else if( BrushID->Type == U_PMF_ARGB_OID){                     btype = 1;    }
      else if( BrushID->Type == U_PMF_4NUM_OID){                     btype = 0;    }
      else {                                                         return(NULL); }
   }
   else {                                                            return(NULL); }
   if(Points){
      if(     Points->Type ==  U_PMF_POINTR_OID){                    RelAbs = 1; ctype = 0; }
      else if(Points->Type == (U_PMF_POINT_OID  | U_PMF_ARRAY_OID)){ RelAbs = 0; ctype = 1; }
      else if(Points->Type == (U_PMF_POINTF_OID | U_PMF_ARRAY_OID)){ RelAbs = 0; ctype = 0; }
      else {                                                         return(NULL);          }
   }
   else {                                                            return(NULL);          }
   int Size = BrushID->Used + Points->Used;
   uint16_t utmp16 =  (btype ? U_PPF_B : 0) |  (ctype ? U_PPF_C : 0) |(RelAbs ? U_PPF_P : 0);
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_FILLPOLYGON,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,      ph->Used,      1, U_XE},
      {BrushID->Data, BrushID->Used, 1, U_XE},
      {Points->Data,  Points->Used,  1, U_XE}, /* includes Elements */
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_FILLPOLYGON_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_FILLRECTS PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  BrushID     U_PSEUDO_OBJ containing a U_PMF_ARGB or a U_PMF_4NUM. Color or U_PMF_BRUSH object in the EMF+ object table (0-63, inclusive)
    \param  Rects       U_PSEUDO_OBJ containing 1 rect OR  a count N followed by N rects.  Rects may be either U_PMF_RECT or U_PMF_RECTF
   

    EMF+ manual 2.3.4.20, Microsoft name: EmfPlusFillRects Record, Index 0x0A
*/
U_PSEUDO_OBJ *U_PMR_FILLRECTS_set(const U_PSEUDO_OBJ *BrushID, const U_PSEUDO_OBJ *Rects){
   int btype, ctype;
   int just1;
   uint32_t Elements=1; /* only used when a single rect is passed in, not an array, not even an array with one member*/
   if(BrushID){ 
      if(      BrushID->Used != 4){                                  return(NULL); }
      else if( BrushID->Type == U_PMF_ARGB_OID){                     btype = 1;    }
      else if( BrushID->Type == U_PMF_4NUM_OID){                     btype = 0;    }
      else {                                                         return(NULL); }
   }
   else {                                                            return(NULL); }
   if(Rects){
      if(      (Rects->Type & U_PMF_MASK_OID) == U_PMF_RECT_OID ){   ctype = 1;    }
      else if( (Rects->Type & U_PMF_MASK_OID) == U_PMF_RECTF_OID){   ctype = 0;    }
      else {                                                         return(NULL); }
   }
   else {                                                            return(NULL); }
   just1 = (Rects->Type & U_PMF_ARRAY_OID ? 0 : 1);
   int Size = BrushID->Used + Rects->Used + (just1 ? 4 : 0); /* Elements in Rects for array, not for single */
   uint16_t utmp16 =  (btype ? U_PPF_B : 0)|(ctype ? U_PPF_C : 0);
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_FILLRECTS,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,                           ph->Used,       1, U_XE},
      {BrushID->Data,                      BrushID->Used,  1, U_XE},
      {(just1 ? (char *)&Elements : NULL), (just1 ? 4: 0), 1, U_LE}, /* element count if a single Rect was passed in, empty otherwise */ 
      {Rects->Data,                        Rects->Used,    1, U_XE}, /* Elements + Array, already stored in Rects, if an array was passed in, just rect if a single */
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_FILLRECTS_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_FILLREGION PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  RgnID       U_PMF_REGION object in the EMF+ object table (0-63, inclusive)
    \param  BrushID     U_PSEUDO_OBJ containing a U_PMF_ARGB or a U_PMF_4NUM. Color or U_PMF_BRUSH object in the EMF+ object table (0-63, inclusive)
    

    EMF+ manual 2.3.4.21, Microsoft name: EmfPlusFillRegion Record, Index 0x13
*/
U_PSEUDO_OBJ *U_PMR_FILLREGION_set(uint32_t RgnID, const U_PSEUDO_OBJ *BrushID){
   int btype;
   if(BrushID){ 
      if(      BrushID->Used != 4){                                  return(NULL); }
      else if( BrushID->Type == U_PMF_ARGB_OID){                     btype = 1;    }
      else if( BrushID->Type == U_PMF_4NUM_OID){                     btype = 0;    }
      else {                                                         return(NULL); }
   }
   else {                                                            return(NULL); }
   int Size = BrushID->Used;
   uint16_t utmp16 =  (btype ? U_PPF_B : 0) | (RgnID & U_FF_MASK_OID8) << U_FF_SHFT_OID8;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_FILLREGION,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,      ph->Used,      1, U_XE},
      {BrushID->Data, BrushID->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_FILLREGION_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_OBJECT PseudoObject from another PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  ObjID       Index for this object in the EMF+ object table (0-63, inclusive)
    \param  Po          U_PSEUDO_OBJ containing an object type that may be stored in the EMF+ object table
*/
U_PSEUDO_OBJ *U_PMR_OBJECT_PO_set(uint32_t ObjID, U_PSEUDO_OBJ *Po){
   if(!Po){ return(NULL); }
   int otype = U_OID_To_OT(Po->Type); /* This will return 0 if the type is not valid for an object */
   if(!otype){ return(NULL); } 
   U_PSEUDO_OBJ *po = U_PMR_OBJECT_set(ObjID, otype, 0, 0, Po->Used, Po->Data); /* 0,0 = rec. not continued, TSize value (ignored)  */
   return(po);
}

/**
    \brief  Create and set a U_PMR_OBJECT PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  ObjID       Index for this object in the EMF+ object table (0-63, inclusive)
    \param  otype       ObjectType enumeration for this Object
    \param  ntype       Set: object definition continues in next record; Clear: this is the sole object definition record
    \param  TSize       If ntype is set the total number of data bytes split across multiple records.  If ntype is clear, it is ignored.
    \param  cbData      Object's data size, in bytes.
    \param  Data        Object's data.  Type from otype.
    

    EMF+ manual 2.3.5.1, Microsoft name: EmfPlusObject Record, Index 0x13

    Normally this is only called by U_PMR_OBJECT_PO_set().
    
    U_PMR_OBJECT records can only hold a maximum of 65020 bytes of data.  If the object is larger than that 
      then multiple U_PMR_OBJECT records are created, one after the other.  If this
      happens each record has cbData following ph, and the ntype flag is set.  If all of the data is less than 65020
      then cbData is NOT entered following ph, and the ntype flag is clear.
    
    Call initially in all cases with ntype clear and TSize = 0.  If the record needs to be fragmented
      the function will call itself recursively to do so.
    
*/
U_PSEUDO_OBJ *U_PMR_OBJECT_set(uint32_t ObjID, int otype, int ntype, uint32_t TSize, size_t cbData, const char *Data){
   uint32_t CSize;
   int      Pad   = UP4(TSize) - TSize;   
   if((otype < U_OT_Brush) || (otype > U_OT_CustomLineCap)){ return(NULL); }
   if(ntype && (cbData > U_OBJRECLIM)){                      return(NULL); }
   if(!Data || !cbData){                                     return(NULL); }
   U_PSEUDO_OBJ *po;
   
   if(!ntype && !TSize && (cbData > U_OBJRECLIM)){  
      ntype = 1;
      TSize = cbData;
      po = U_PO_create(NULL, TSize + 16 * (1 + (TSize/cbData)), 0, U_PMR_OBJECT_OID);
      if(po){
         while(cbData){
            CSize = (cbData > U_OBJRECLIM ? U_OBJRECLIM : cbData);
            U_PSEUDO_OBJ *pot = U_PMR_OBJECT_set(ObjID, otype, ntype, TSize, CSize, Data);
            if(!pot)break;
            U_PSEUDO_OBJ *newpo = U_PO_po_append(po, pot, U_PMF_KEEP_ELEMENTS);
            U_PO_free(&pot);
            if(!newpo)break;
            po = newpo; 
            Data   += U_OBJRECLIM;
            cbData -= CSize;
         }
         if(cbData){ /* some error */
            U_PO_free(&po);
         }
      }
   }
   else {
      /* Send in DataSize, U_PMR_CMN_HDR_set will adjust Header Size with 1-3 pad bytes if needed */
      uint16_t utmp16 =  otype << 8 | (ntype ? U_PPF_N : 0) | (ObjID & U_FF_MASK_OID8) << U_FF_SHFT_OID8;
      U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_OBJECT,utmp16,cbData + (ntype ? 4 : 0));
      const U_SERIAL_DESC List[] = {
         {ph->Data,                 ph->Used,       1,               U_XE               },
         {(ntype ? &TSize : NULL), (ntype ? 4 : 0), (ntype ? 1 : 0), U_LE               },
         {Data,                    cbData,          1,               U_XE               },
         {NULL,                    (Pad ? Pad : 0), (Pad ? 1 : 0),   (Pad ? U_XE : U_XX)}, /* Either 1-3 pad bytes or a terminator */
         {NULL,0,0,U_XX} /* terminator, possibly a second in the list, which is harmless */
      };
      po = U_PMF_SERIAL_set(U_PMR_OBJECT_OID, List);
      U_PO_free(&ph);
   }
   return(po);
}

/**
    \brief  Create and set a U_PMR_SERIALIZABLEOBJECT PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Siepb       U_PSEUDO_OBJ containing a "Serialized image effects parameter block".  One of the ImageEffects objects.

    EMF+ manual 2.3.5.2, Microsoft name: EmfPlusSerializableObject Record, Index 0x38


    This sets an ImageEffect in the renderer, which will be applied to the next EmfPlusDrawImagePoints 
    record that is encountered.  The image effect is "consumed" by that EmfPlusDrawImagePoints record, resetting
    the renderer to its original state.\n  
    
    WARNING!  Windows XP Preview does not show filter effects, whether or not U_PPF_E is set.   They are visible if the EMF+
    file is inserted as an image into PowerPoint.

*/
U_PSEUDO_OBJ *U_PMR_SERIALIZABLEOBJECT_set(const U_PSEUDO_OBJ *Siepb){
   if(!Siepb){                                               return(NULL); }
   uint8_t *GUID = U_OID_To_GUID(Siepb->Type);
   if(!GUID){ return(NULL); }
   /* PO Used is size_t, might be 8 bytes, value in record must be 4 bytes */
   uint32_t Used = Siepb->Used;
   int Size = 16 + 4 + Siepb->Used;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_SERIALIZABLEOBJECT,0,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,    ph->Used,	   1, U_XE},
      {GUID,        16, 	   1, U_XE},
      {&Used,       4,  	   1, U_LE},
      {Siepb->Data, Siepb->Used,   1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_SERIALIZABLEOBJECT_OID, List);
   U_PO_free(&ph);
   free(GUID);
   return(po);
}

/**
    \brief  Create and set a U_PMR_SETANTIALIASMODE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  SMenum      SmoothingMode enumeration
    \param  aatype      Set: anti-aliasing on; Clear: anti-aliasing off
    

    EMF+ manual 2.3.6.1, Microsoft name: EmfPlusSetAntiAliasMode Record, Index 0x1E
*/
U_PSEUDO_OBJ *U_PMR_SETANTIALIASMODE_set(int SMenum, int aatype){
   int Size = 0;
   uint16_t utmp16 =  (aatype ? U_PPF_AA : 0) | (SMenum & U_FF_MASK_AA)<<U_FF_SHFT_AA;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_SETANTIALIASMODE,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_SETANTIALIASMODE_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_SETCOMPOSITINGMODE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  CMenum      CompositingMode enumeration
    

    EMF+ manual 2.3.6.2, Microsoft name: EmfPlusSetCompositingMode Record, Index 0x23
*/
U_PSEUDO_OBJ *U_PMR_SETCOMPOSITINGMODE_set(int CMenum){
   int Size = 0;
   uint16_t utmp16 =  (CMenum & U_FF_MASK_CM)<<U_FF_SHFT_CM;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_SETCOMPOSITINGMODE,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_SETCOMPOSITINGMODE_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_SETCOMPOSITINGQUALITY PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  CQenum      CompositingQuality enumeration
    

    EMF+ manual 2.3.6.3, Microsoft name: EmfPlusSetCompositingQuality Record, Index 0x24
*/
U_PSEUDO_OBJ *U_PMR_SETCOMPOSITINGQUALITY_set(int CQenum){
   int Size = 0;
   uint16_t utmp16 =  (CQenum & U_FF_MASK_CQ)<<U_FF_SHFT_CQ;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_SETCOMPOSITINGQUALITY,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_SETCOMPOSITINGQUALITY_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_SETINTERPOLATIONMODE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  IMenum      InterpolationMode enumeration
    

    EMF+ manual 2.3.6.4, Microsoft name: EmfPlusSetInterpolationMode Record, Index 0x21
*/
U_PSEUDO_OBJ *U_PMR_SETINTERPOLATIONMODE_set(int IMenum){
   int Size = 0;
   uint16_t utmp16 =  (IMenum & U_FF_MASK_IM)<<U_FF_SHFT_IM;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_SETINTERPOLATIONMODE,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_SETINTERPOLATIONMODE_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_SETPIXELOFFSETMODE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  POMenum     PixelOffsetMode enumeration
    

    EMF+ manual 2.3.6.5, Microsoft name: EmfPlusSetPixelOffsetMode Record, Index 0x22
*/
U_PSEUDO_OBJ *U_PMR_SETPIXELOFFSETMODE_set(int POMenum){
   int Size = 0;
   uint16_t utmp16 =  (POMenum & U_FF_MASK_PxOffM) << U_FF_SHFT_PxOffM;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_SETPIXELOFFSETMODE,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_SETPIXELOFFSETMODE_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_SETRENDERINGORIGIN PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  X            X coordinate of rendering origin.
    \param  Y            Y coordinate of rendering origin.
    

    EMF+ manual 2.3.6.6, Microsoft name: EmfPlusSetRenderingOrigin Record, Index 0x1D
*/
U_PSEUDO_OBJ *U_PMR_SETRENDERINGORIGIN_set(int32_t X, int32_t Y){
   int Size = 2*4;
   uint16_t utmp16 =  0;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_SETRENDERINGORIGIN,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {&X,       4,        1, U_LE},
      {&Y,       4,        1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_SETRENDERINGORIGIN_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_SETTEXTCONTRAST PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  TGC    Text Gamma correction value (x 1000).
    

    EMF+ manual 2.3.6.7, Microsoft name: EmfPlusSetTextContrast Record, Index 0x20
*/
U_PSEUDO_OBJ *U_PMR_SETTEXTCONTRAST_set(int TGC){
   int Size = 0;
   uint16_t utmp16 =  (TGC & U_FF_MASK_TGC) << U_FF_SHFT_TGC;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_SETTEXTCONTRAST,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_SETTEXTCONTRAST_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_SETTEXTRENDERINGHINT PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  TRHenum     TextRenderingHint enumeration
    

    EMF+ manual 2.3.6.8, Microsoft name: EmfPlusSetTextRenderingHint Record, Index 0x1F
*/
U_PSEUDO_OBJ *U_PMR_SETTEXTRENDERINGHINT_set(int TRHenum){
   int Size = 0;
   uint16_t utmp16 =  (TRHenum & U_FF_MASK_TRH) << U_FF_SHFT_TRH;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_SETTEXTRENDERINGHINT,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_SETTEXTRENDERINGHINT_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_BEGINCONTAINER PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  UTenum      UnitType enumeration
    \param  DstRect     a U_PSEUDO_OBJ  containing a U_PMF_RECTF object.  with SrcRect specifies a transformation
    \param  SrcRect     a U_PSEUDO_OBJ  containing a U_PMF_RECTF object.  with DstRect specifies a transformation
    \param  StackID     EMF+ Object Stack Index to use for this graphics container
    

    EMF+ manual 2.3.7.1, Microsoft name: EmfPlusBeginContainer Record, Index 0x27
*/
U_PSEUDO_OBJ *U_PMR_BEGINCONTAINER_set(int UTenum, U_PSEUDO_OBJ *DstRect, U_PSEUDO_OBJ *SrcRect, uint32_t StackID){
   if(UTenum < U_UT_World || UTenum > U_UT_Millimeter){ return(NULL); }
   if(!DstRect || (DstRect->Type != U_PMF_RECTF_OID)){  return(NULL); }
   if(!SrcRect || (SrcRect->Type != U_PMF_RECTF_OID)){  return(NULL); }
   int Size = DstRect->Used + SrcRect->Used + 4;
   uint16_t utmp16 =  (UTenum & U_FF_MASK_UT) << U_FF_SHFT_UT;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_BEGINCONTAINER,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,      ph->Used,      1, U_XE},
      {DstRect->Data, DstRect->Used, 1, U_XE},
      {SrcRect->Data, SrcRect->Used, 1, U_XE},
      {&StackID,      4,             1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_BEGINCONTAINER_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_BEGINCONTAINERNOPARAMS PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  StackID     EMF+ Object Stack Index to use for this graphics container
    

    EMF+ manual 2.3.7.2, Microsoft name: EmfPlusBeginContainerNoParams Record, Index 0x28
*/
U_PSEUDO_OBJ *U_PMR_BEGINCONTAINERNOPARAMS_set(int StackID){
   int Size = 4;
   uint16_t utmp16 =  0;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_BEGINCONTAINERNOPARAMS,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {&StackID, 4,        1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_BEGINCONTAINERNOPARAMS_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_ENDCONTAINER PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  StackID     EMF+ Object Stack Index to use for this graphics container
    

    EMF+ manual 2.3.7.3, Microsoft name: EmfPlusEndContainer Record, Index 0x29
*/
U_PSEUDO_OBJ *U_PMR_ENDCONTAINER_set(int StackID){
   int Size = 4;
   uint16_t utmp16 =  0;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_ENDCONTAINER,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {&StackID, 4,        1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_ENDCONTAINER_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_RESTORE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  StackID     EMF+ Graphics State Stack to restore from. Must have been put on the GSS with a U_PMR_SAVE.
    

    EMF+ manual 2.3.7.4, Microsoft name: EmfPlusRestore Record, Index 0x26
*/
U_PSEUDO_OBJ *U_PMR_RESTORE_set(int StackID){
   int Size = 4;
   uint16_t utmp16 =  0;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_RESTORE,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {&StackID, 4,        1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_RESTORE_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_SAVE PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  StackID     EMF+ Graphics State Stack to restore from. Must have been put on the GSS with a U_PMR_SAVE.
    

    EMF+ manual 2.3.7.5, Microsoft name: EmfPlusSave Record, Index 0x25
*/
U_PSEUDO_OBJ *U_PMR_SAVE_set(int StackID){
   int Size = 4;
   uint16_t utmp16 =  0;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_SAVE,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {&StackID, 4,        1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_SAVE_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_SETTSCLIP PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Rects       a U_PSEUDO_OBJ  containing an array of U_PMF_RECT or U_PMF_RECTF objects.
    

    EMF+ manual 2.3.8.1, Microsoft name: EmfPlusSetTSClip Record, Index 0x3A
*/
U_PSEUDO_OBJ *U_PMR_SETTSCLIP_set(U_PSEUDO_OBJ *Rects){
   int ctype;
   uint32_t Elements;
   if(Rects){
      if(     Rects->Type == (U_PMF_RECT_OID  | U_PMF_ARRAY_OID)){ ctype = 1;  Elements = (Rects->Used - 4)/8;  }
      else if(Rects->Type == (U_PMF_RECTF_OID | U_PMF_ARRAY_OID)){ ctype = 0;  Elements = (Rects->Used - 4)/16; }
      else {                                                       return(NULL); }
   }
   else {                                                          return(NULL); }
   int Size = Rects->Used; /* Rects includes Elements */
   uint16_t utmp16 =  (ctype ? U_PPF_K : 0) | (Elements & U_FF_MASK_TSC) << U_FF_SHFT_TSC;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_SETTSCLIP,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,    ph->Used,    1, U_XE},
      {Rects->Data, Rects->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_SETTSCLIP_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_SETTSGRAPHICS PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  vgatype     Set: Palette is VGA basic colors; Clear: Palette is ???
    \param  Tsg         A U_PMF_SETTSGRAPHICS object
    \param  Palette     (optional) a U_PSEUDO_OBJ containing a U_PMF_PALETTE object.
    

    EMF+ manual 2.3.8.2, Microsoft name: EmfPlusSetTSGraphics Record, Index 0x39
*/
U_PSEUDO_OBJ *U_PMR_SETTSGRAPHICS_set(int vgatype, U_PMF_SETTSGRAPHICS *Tsg, U_PSEUDO_OBJ *Palette){
   if(!Tsg){ return(NULL); }
   int Size = sizeof(U_PMF_SETTSGRAPHICS) + (Palette ? Palette->Used : 0);
   uint16_t utmp16 =  (vgatype ? U_PPF_VGA : 0) | (Palette ? U_PPF_PP : 0) ;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_SETTSGRAPHICS,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data,                         ph->Used,                     1, U_XE},
      {&(Tsg->AntiAliasMode),            1,                            1, U_XE},
      {&(Tsg->TextRenderHint),           1,                            1, U_XE},
      {&(Tsg->CompositingMode),          1,                            1, U_XE},
      {&(Tsg->CompositingQuality),       1,                            1, U_XE},
      {&(Tsg->RenderOriginX),            2,                            1, U_LE},
      {&(Tsg->RenderOriginY),            2,                            1, U_LE},
      {&(Tsg->TextContrast),             2,                            1, U_LE},
      {&(Tsg->FilterType),               1,                            1, U_XE},
      {&(Tsg->PixelOffset),              1,                            1, U_XE},
      {&(Tsg->WorldToDevice),            4,                            6, U_LE},
      {(Palette ? Palette->Data : NULL), (Palette ? Palette->Used: 0), 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_SETTSGRAPHICS_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_MULTIPLYWORLDTRANSFORM PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  xmtype      Set: Post multiply; Clear: Pre multiply
    \param  Tm          a U_PSEUDO_OBJ containing a U_PMF_TRANSFORMMATRIX. (Transformation matrix)
    

    EMF+ manual 2.3.9.1, Microsoft name: EmfPlusMultiplyWorldTransform Record, Index 0x2C
*/
U_PSEUDO_OBJ *U_PMR_MULTIPLYWORLDTRANSFORM_set(int xmtype, U_PSEUDO_OBJ *Tm){
   if(!Tm || (Tm->Type != U_PMF_TRANSFORMMATRIX_OID)){ return(NULL); }
   int Size = Tm->Used;
   uint16_t utmp16 =  (xmtype ? U_PPF_XM : 0);
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_MULTIPLYWORLDTRANSFORM,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {Tm->Data, Tm->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_MULTIPLYWORLDTRANSFORM_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_RESETWORLDTRANSFORM PseudoObject
    \return Pointer to PseudoObject, NULL on error
    

    EMF+ manual 2.3.9.2, Microsoft name: EmfPlusResetWorldTransform Record, Index 0x2B
*/
U_PSEUDO_OBJ *U_PMR_RESETWORLDTRANSFORM_set(void){
   int Size = 0;
   uint16_t utmp16 =  0;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_RESETWORLDTRANSFORM,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_RESETWORLDTRANSFORM_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_ROTATEWORLDTRANSFORM PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  xmtype      Set: Post multiply; Clear: Pre multiply
    \param  Angle       Rotation angle, in degrees
    

    EMF+ manual 2.3.9.3, Microsoft name: EmfPlusRotateWorldTransform Record, Index 0x2F
*/
U_PSEUDO_OBJ *U_PMR_ROTATEWORLDTRANSFORM_set(int xmtype, U_FLOAT Angle){
   int Size = 4;
   uint16_t utmp16 =  (xmtype ? U_PPF_XM : 0);
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_ROTATEWORLDTRANSFORM,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {&Angle,   4,        1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_ROTATEWORLDTRANSFORM_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_SCALEWORLDTRANSFORM PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  xmtype      Set: Post multiply; Clear: Pre multiply
    \param  X           Scale in X
    \param  Y           Scale in Y

    EMF+ manual 2.3.9.4, Microsoft name: EmfPlusScaleWorldTransform Record, Index 0x2E
*/
U_PSEUDO_OBJ *U_PMR_SCALEWORLDTRANSFORM_set(int xmtype, U_FLOAT X, U_FLOAT Y){
   int Size = 2*4;
   uint16_t utmp16 =  (xmtype ? U_PPF_XM : 0);
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_SCALEWORLDTRANSFORM,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {&X,       4,        1, U_LE},
      {&Y,       4,        1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_SCALEWORLDTRANSFORM_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_SETPAGETRANSFORM PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  PUenum   Page Unit, in UnitType enumeration
    \param  Scale    Scale factor to convert page space to device space
    
    

    EMF+ manual 2.3.9.5, Microsoft name: EmfPlusSetPageTransform Record, Index 0x30

    Defines Page Space -> Device Space transformation
*/
U_PSEUDO_OBJ *U_PMR_SETPAGETRANSFORM_set(int PUenum, U_FLOAT Scale){
   int Size = 4;
   uint16_t utmp16 =  (PUenum & U_FF_MASK_PU) << U_FF_SHFT_PU;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_SETPAGETRANSFORM,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {&Scale,    4,        1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_SETPAGETRANSFORM_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_SETWORLDTRANSFORM PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  Tm          a U_PSEUDO_OBJ containing a U_PMF_TRANSFORMMATRIX. (Transformation matrix)
    
    

    EMF+ manual 2.3.9.6, Microsoft name: EmfPlusSetWorldTransform Record, Index 0x2A

    Defines  World Space -> Page Space transformation
*/
U_PSEUDO_OBJ *U_PMR_SETWORLDTRANSFORM_set(U_PSEUDO_OBJ *Tm){
   if(!Tm || (Tm->Type != U_PMF_TRANSFORMMATRIX_OID)){ return(NULL); }
   int Size = Tm->Used;
   uint16_t utmp16 = 0;
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_SETWORLDTRANSFORM,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {Tm->Data, Tm->Used, 1, U_XE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_SETWORLDTRANSFORM_OID, List);
   U_PO_free(&ph);
   return(po);
}

/**
    \brief  Create and set a U_PMR_TRANSLATEWORLDTRANSFORM PseudoObject
    \return Pointer to PseudoObject, NULL on error
    \param  xmtype      Set: Post multiply; Clear: Pre multiply
    \param  Dx          X offset
    \param  Dy          Y offset
    

    EMF+ manual 2.3.9.7, Microsoft name: EmfPlusTranslateWorldTransform Record, Index 0x2D
*/
U_PSEUDO_OBJ *U_PMR_TRANSLATEWORLDTRANSFORM_set(int xmtype, U_FLOAT Dx, U_FLOAT Dy){
   int Size = 2*4;
   uint16_t utmp16 =  (xmtype ? U_PPF_XM : 0);
   U_PSEUDO_OBJ *ph = U_PMR_CMN_HDR_set(U_PMR_TRANSLATEWORLDTRANSFORM,utmp16,Size);
   const U_SERIAL_DESC List[] = {
      {ph->Data, ph->Used, 1, U_XE},
      {&Dx,      4,        1, U_LE},
      {&Dy,      4,        1, U_LE},
      {NULL,0,0,U_XX}
   };
   U_PSEUDO_OBJ *po = U_PMF_SERIAL_set(U_PMR_TRANSLATEWORLDTRANSFORM_OID, List);
   U_PO_free(&ph);
   return(po);
}

/*

   end of U_PMF_*_set() functions 
   =====================================================================================
   start of U_PMF_*_get() functions 

      These functions all take a blimit value so that they can check if the data description in the fields
      they process extend beyond the end of the record.

*/


//! \cond
/* core _get functions, not accessed outside of this routine */

/* get copies of up to 0-6 consecutive 4 byte values and a pointer to the rest */
int U_PMF_CORE1_get(const char *contents, void *v1, void *v2, void *v3, void *v4, void *v5, void *v6, const char **vR){
    if(v1){                U_PMF_MEMCPY_SRCSHIFT(v1, &contents, 4);
       if(v2){             U_PMF_MEMCPY_SRCSHIFT(v2, &contents, 4);
          if(v3){          U_PMF_MEMCPY_SRCSHIFT(v3, &contents, 4);
             if(v4){       U_PMF_MEMCPY_SRCSHIFT(v4, &contents, 4);
                if(v5){    U_PMF_MEMCPY_SRCSHIFT(v5, &contents, 4);
                   if(v6){ U_PMF_MEMCPY_SRCSHIFT(v6, &contents, 4); }}}}}}
    if(vR){ *vR = contents; }
    return(1);
}
//! \endcond


/**
    \brief Get data from a U_PMF_BRUSH object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Version    EmfPlusGraphicsVersion object
    \param  Type       BrushType Enumeration
    \param  Data       one of the 5 types of Brush data
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.1.1, Microsoft name: EmfPlusBrush Object

    Caller must check Data for possible memory access violations.
*/
int U_PMF_BRUSH_get(const char *contents, uint32_t *Version, uint32_t *Type, const char **Data, const char *blimit){
    if(!contents || !Version || !Type || !Data || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents,sizeof(U_PMF_BRUSH), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Version, 4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, Type,    4, 1, U_LE);
    U_PMF_PTRSAV_SHIFT(Data, &contents, 0);
    return(1);
}

/**
    \brief Get data from a U_PMF_CUSTOMLINECAP object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Version    EmfPlusGraphicsVersion object
    \param  Type       CustomLineCapData Enumeration
    \param  Data       one of the 2 types of Linecap data
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.1.2, Microsoft name: EmfPlusCustomLineCap Object

    Caller must check Data for possible memory access violations.
*/
int U_PMF_CUSTOMLINECAP_get(const char *contents, uint32_t *Version, uint32_t *Type, const char **Data, const char *blimit){
    if(!contents || !Version || !Type || !Data || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_CUSTOMLINECAP), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Version, 4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, Type,    4, 1, U_LE);
    U_PMF_PTRSAV_SHIFT(Data, &contents, 0);
    return(1);
}

/**
    \brief Get data from a U_PMF_FONT object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Version    EmfPlusGraphicsVersion object
    \param  EmSize     em size in units of SizeUnit
    \param  SizeUnit   UnitType enumeration
    \param  FSFlags    FontStyle flags
    \param  Length     Number of Unicode Characters in FamilyName
    \param  Data       Unicode (UTF-16LE) name of font family

    EMF+ manual 2.2.1.3, Microsoft name: EmfPlusFont Object

    Caller must check Data for possible memory access violations.
*/
int U_PMF_FONT_get(const char *contents, uint32_t *Version, U_FLOAT *EmSize, uint32_t *SizeUnit,
      int32_t *FSFlags, uint32_t *Length, const char **Data, const char *blimit){
    if(!contents || !Version || !EmSize || !SizeUnit || !FSFlags || !Length || !Data || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_FONT), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Version,  4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, EmSize,   4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, SizeUnit, 4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, FSFlags,  4, 1, U_LE);
    contents += 4; /* Reserved field, which is ignored */
    U_PMF_SERIAL_get(&contents, Length,   4, 1, U_LE);
    U_PMF_PTRSAV_SHIFT(Data, &contents, 0);
    return(1);
}


/**
    \brief Get data from a U_PMF_IMAGE object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Version    EmfPlusGraphicsVersion object
    \param  Type       ImageDataType Enumeration
    \param  Data       one of the 2 types of image data

    EMF+ manual 2.2.1.4, Microsoft name: EmfPlusImage Object

    Caller must check Data for possible memory access violations.
*/
int U_PMF_IMAGE_get(const char *contents, uint32_t *Version, uint32_t *Type, const char **Data, const char *blimit){
    if(!contents || !Version || !Type || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_IMAGE), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Version, 4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, Type,    4, 1, U_LE);
    U_PMF_PTRSAV_SHIFT(Data, &contents, 0);
    return(1);
}

/**
    \brief Get data from a U_PMF_IMAGEATTRIBUTES object
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Version     EmfPlusGraphicsVersion object
    \param  WrapMode    WrapMode object
    \param  ClampColor  EmfPlusARGB object
    \param  ObjectClamp ObjectClamp Identifiers

    EMF+ manual 2.2.1.5, Microsoft name: EmfPlusImageAttributes Object
*/
int U_PMF_IMAGEATTRIBUTES_get(const char *contents, uint32_t *Version, uint32_t *WrapMode, uint32_t *ClampColor,
      uint32_t *ObjectClamp, const char *blimit){
    if(!contents || !Version || !WrapMode || !ClampColor || !ObjectClamp || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_IMAGEATTRIBUTES), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Version,    4, 1, U_LE);
    contents += 4; /* Skip Reserved 1*/
    U_PMF_SERIAL_get(&contents, WrapMode,   4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, ClampColor, 4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, ObjectClamp,4, 1, U_LE);
    /* Skip Reserved 2*/
    return(1);
}

/**
    \brief Get data from a U_PMF_PATH object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Version    EmfPlusGraphicsVersion object
    \param  Count      Number of points and point types in this object
    \param  Flags      PathPoint Flags
    \param  Points     array of points type PMFPointR, PMFPoint, or PMFPointF
    \param  Types      array of U_PMF_PATHPOINTTYPERLE and/or U_PMF_PATHPOINTTYPE

    EMF+ manual 2.2.1.6, Microsoft name: EmfPlusPath Object

    Caller must check Types for possible memory access violations if type can be U_PMF_PATHPOINTTYPERLE.
*/
int U_PMF_PATH_get(const char *contents, uint32_t *Version, uint32_t *Count, uint16_t *Flags, 
      const char **Points, const char **Types, const char *blimit){
    if(!contents || !Version || !Count || !Flags || !Points || !Types || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_PATH), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Version,    4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, Count,      4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, Flags,      2, 1, U_LE);
    contents+=2; /* reserved */
    uint32_t sizeP, sizeT;
    if(*Flags      & U_PPF_P){ 
       sizeP  = U_PMF_LEN_REL715(contents,*Count); //DEBUG
       printf("DEBUG U_PMF_PATH_get count:%d LENREL715:%d\n",*Count,sizeP);fflush(stdout);
    }
    else if(*Flags & U_PPF_C){ sizeP = *Count * sizeof(U_PMF_POINT);   }
    else {                     sizeP = *Count * sizeof(U_PMF_POINTF);  }
    if(IS_MEM_UNSAFE(contents, sizeP, blimit))return(0);
    U_PMF_PTRSAV_SHIFT(Points, &contents, 0);
    contents += sizeP;
    /* this limit is correct if there are only U_PMF_PATHPOINTTYPE PointTypes, it is a lower bound if
       there can also be U_PMF_PATHPOINTTYPERLE */
    sizeT = *Count * sizeof(U_PMF_PATHPOINTTYPE);
    if(IS_MEM_UNSAFE(contents, sizeT, blimit))return(0);
    U_PMF_PTRSAV_SHIFT(Types, &contents, 0);
    return(1);
}

/**
    \brief Get data from a U_PMF_PEN object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Version    EmfPlusGraphicsVersion object
    \param  Type       must be zero
    \param  PenData    Pen description
    \param  Brush      Brush Description 

    EMF+ manual 2.2.1.7, Microsoft name: EmfPlusPen Object

    Caller must check Brush and PenData for possible memory access violations.
*/
int U_PMF_PEN_get(const char *contents, uint32_t *Version, uint32_t *Type, const char **PenData, const char **Brush, const char *blimit){
    if(!contents || !Type || !PenData || !Brush || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_PEN), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Version, 4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, Type,    4, 1, U_LE);
    U_PMF_PTRSAV_SHIFT(PenData, &contents, 0);
    *Brush = *PenData + U_PMF_LEN_PENDATA(*PenData);
    return(1);
}
    
/**
    \brief Get data from a U_PMF_REGION object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Version    EmfPlusGraphicsVersion object
    \param  Count      Number of CHILD nodes.  This is one less than the total number of U_PMF_REGIONNODE objects in Nodes.
    \param  Nodes      Nodes defining region

    EMF+ manual 2.2.1.8, Microsoft name: EmfPlusRegion Object
*/
int U_PMF_REGION_get(const char *contents, uint32_t *Version, uint32_t *Count, const char **Nodes, const char *blimit){
    if(!contents || !Version || !Count || !Nodes || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_REGION), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Version, 4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, Count,   4, 1, U_LE);
    U_PMF_PTRSAV_SHIFT(Nodes, &contents, 0);
    return(1);
}

/**
    \brief Get data from a U_PMF_STRINGFORMAT object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Sfs        pointer to U_PMF_STRINGFORMAT structure, with no variable part
    \param  Data       pointer to variable part

    EMF+ manual 2.2.1.9, Microsoft name: EmfPlusStringFormat Object
*/
int U_PMF_STRINGFORMAT_get(const char *contents, U_PMF_STRINGFORMAT *Sfs, const char **Data, const char *blimit){
    if(!contents || !Sfs || !Data || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_STRINGFORMAT), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Sfs, 4, 15, U_LE);
    *Data = contents;
    return(1);
}

/**
    \brief Get data from a U_PMF_ARGB object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Blue       Blue  color (0-255)
    \param  Green      Green color (0-255)
    \param  Red        Red   color (0-255)
    \param  Alpha      Alpha       (0-255)

    EMF+ manual 2.2.2.1, Microsoft name: EmfPlusARGB Object
*/
int U_PMF_ARGB_get(const char *contents, uint8_t *Blue, uint8_t *Green, uint8_t *Red, uint8_t *Alpha, const char *blimit){
    if(!contents || !Blue || !Green || !Red || !Alpha || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_ARGB), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Blue,  1, 1, U_XE);
    U_PMF_SERIAL_get(&contents, Green, 1, 1, U_XE);
    U_PMF_SERIAL_get(&contents, Red,   1, 1, U_XE);
    U_PMF_SERIAL_get(&contents, Alpha, 1, 1, U_XE);
    return(1);
}

/**
    \brief Get data from a U_PMF_BITMAP object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Bs         pointer to U_PMF_BITMAP structure, with no variable part
    \param  Data       pointer to variable part

    EMF+ manual 2.2.2.2, Microsoft name: EmfPlusBitmap Object

    Caller must check Data for possible memory access violations.
*/
int U_PMF_BITMAP_get(const char *contents, U_PMF_BITMAP *Bs, const char **Data, const char *blimit){
    if(!contents || !Bs || !Data || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_BITMAP), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Bs, 4, 5, U_LE); // width, height, stride, pixelformat, type
    U_PMF_PTRSAV_SHIFT(Data, &contents, 0); // bitmapdata
    return(1);
}

/**
    \brief Get data from a U_PMF_BITMAPDATA object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Ps         pointer to U_PMF_PALETTE structure, with no variable part
    \param  Colors     Color part of U_PMF_PALETTE object
    \param  Data       An array of bytes, meaning depends on fields in U_PMF_BITMAP object and the PixelFormat enumeration.

    EMF+ manual 2.2.2.3, Microsoft name: EmfPlusBitmapData Object

    Caller must check Data for possible memory access violations.
*/
int U_PMF_BITMAPDATA_get(const char *contents, U_PMF_PALETTE *Ps, const char **Colors, const char **Data, const char *blimit){
    if(!contents || !Ps || !Colors || !Data  || !blimit){ return(0); }
    /* this structure is entirely optional */
    if(IS_MEM_UNSAFE(contents, 4*2, blimit))return(0);
    U_PMF_SERIAL_get(&contents, Ps, 4, 2, U_LE);
    U_PMF_PTRSAV_SHIFT(Colors, &contents, Ps->Elements * sizeof(U_PMF_ARGB));
    /* difficult to know how big the actual bitmap will be, just return the pointer to it untested */
    U_PMF_PTRSAV_SHIFT(Data,  &contents, 0);
    return(1);
}

/**
    \brief Get data from a U_PMF_BLENDCOLORS object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Elements   Number of members in Positions and Colors
    \param  Positions  Caller must free.  Pointer to memory holding positions along gradient line.  
    \param  Colors     Caller must NOT free memory, Pointer to memory holding colors at positions on gradient line.  

    EMF+ manual 2.2.2.4, Microsoft name: EmfPlusBlendColors Object
*/
int U_PMF_BLENDCOLORS_get(const char *contents, uint32_t *Elements, U_FLOAT **Positions, const char **Colors, const char *blimit){
    if(!contents || !Positions || !Colors || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_BLENDCOLORS), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Elements, 4, 1, U_LE);
    if(IS_MEM_UNSAFE(contents, *Elements * 4, blimit))return(0);
    if(!U_PMF_SERIAL_array_copy_get(&contents, (void **)Positions, 4, *Elements, U_LE,1)){ return(0); }
    U_PMF_PTRSAV_SHIFT(Colors,  &contents, 0);
    return(1);
}

/**
    \brief Get data from a U_PMF_BLENDFACTORS object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Elements   members in each array
    \param  Positions  Caller must free.  Pointer to memory holding positions along gradient line.  
    \param  Factors    Caller must free.  Pointer to memory holding blending factors, 0.0->1.0 values, inclusive along gradient line.  

    EMF+ manual 2.2.2.5, Microsoft name: EmfPlusBlendFactors Object
*/
int U_PMF_BLENDFACTORS_get(const char *contents, uint32_t *Elements, U_FLOAT **Positions, U_FLOAT **Factors, const char *blimit){
    if(!contents || !Elements || !Positions || !Factors || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_BLENDFACTORS), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Elements, 4, 1, U_LE);
    if(IS_MEM_UNSAFE(contents, *Elements * 4 * 2, blimit))return(0);
    if(!U_PMF_SERIAL_array_copy_get(&contents, (void **)Positions,  4, *Elements, U_LE, 1)){ return(0); }
    if(!U_PMF_SERIAL_array_copy_get(&contents, (void **)Factors,    4, *Elements, U_LE, 1)){
       free(*Positions);
       return(0);
    }
    return(1);
}

/**
    \brief Get data from a U_PMF_BOUNDARYPATHDATA object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Size       bytes in Data
    \param  Data       boundary of the brush

    EMF+ manual 2.2.2.6, Microsoft name: EmfPlusBoundaryPathData Object

    Caller must check Data for possible memory access violations.
*/
int U_PMF_BOUNDARYPATHDATA_get(const char *contents, int32_t *Size, const char **Data, const char *blimit){
    if(!contents || !Size || !Data || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_BOUNDARYPATHDATA), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Size, 4, 1, U_LE);
    U_PMF_PTRSAV_SHIFT(Data,  &contents, 0);
    return(1);
}

/**
    \brief Get data from a U_PMF_BOUNDARYPOINTDATA object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Elements   Members in Points
    \param  Points     Caller must free.  Pointer to memory holding points along gradient line. Boundary of the brush.

    EMF+ manual 2.2.2.7, Microsoft name: EmfPlusBoundaryPointData Object
*/
int U_PMF_BOUNDARYPOINTDATA_get(const char *contents, int32_t *Elements, U_PMF_POINTF **Points, const char *blimit){
    if(!contents || !Elements || !Points || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_BOUNDARYPOINTDATA), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Elements, 4, 1, U_LE);
    if(IS_MEM_UNSAFE(contents, *Elements * 2, blimit))return(0);
    if(!U_PMF_SERIAL_array_copy_get(&contents, (void **)Points, 4, *Elements * 2, U_LE, 1)){ return(0); }
    return(1);
}

/**
    \brief Get data from a U_PMF_CHARACTERRANGE object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  First      First position in range
    \param  Length     Range length

    EMF+ manual 2.2.2.8, Microsoft name: EmfPlusCharacterRange Object
*/
int U_PMF_CHARACTERRANGE_get(const char *contents, int32_t *First, int32_t *Length, const char *blimit){
    if(!contents || !First || !Length || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_CHARACTERRANGE), blimit))return(0);
    U_PMF_SERIAL_get(&contents, First,  4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, Length, 4, 1, U_LE);
    return(1);
}

/**
    \brief Get data from a U_PMF_COMPOUNDLINEDATA object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Elements   Members in the array
    \param  Widths     Caller must free.  Pointer to memory holding Line or gap widths (0.0 <-> 1.0, fraction of total line width ).  

    EMF+ manual 2.2.2.9, Microsoft name: EmfPlusCompoundLineData Object
*/
int U_PMF_COMPOUNDLINEDATA_get(const char *contents, int32_t *Elements, U_FLOAT **Widths, const char *blimit){
    if(!contents || !Elements || !Widths || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_COMPOUNDLINEDATA), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Elements, 4, 1, U_LE);
    if(IS_MEM_UNSAFE(contents, *Elements * sizeof(U_FLOAT), blimit))return(0);
    *Widths = (U_FLOAT *)malloc(*Elements * sizeof(U_FLOAT));
    if(!*Widths){ return(0); }
    U_PMF_SERIAL_get(&contents, *Widths, 4, *Elements, U_LE);
    return(1);
}

/**
    \brief Get data from a U_PMF_COMPRESSEDIMAGE object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Data       Stored image in one of the supported formats.

    EMF+ manual 2.2.2.10, Microsoft name: EmfPlusCompressedImage Object
    

    This function does not do anything useful, but it is included so that all objects have a corresponding _get().

    Caller must check Data for possible memory access violations.
*/
int U_PMF_COMPRESSEDIMAGE_get(const char *contents, const char **Data, const char *blimit){
    if(!contents || !Data || !blimit){ return(0); }
    if(contents >= blimit)return(0);
    U_PMF_PTRSAV_SHIFT(Data, &contents, 0);
    return(1);
}

/**
    \brief Get data from a U_PMF_CUSTOMENDCAPDATA object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Size       Bytes in Data
    \param  Data       Description of linecap

    EMF+ manual 2.2.2.11, Microsoft name: EmfPlusCustomEndCapData Object

    Caller must check Data for possible memory access violations.
*/
int U_PMF_CUSTOMENDCAPDATA_get(const char *contents, int32_t *Size, const char **Data, const char *blimit){
    if(!contents || !Size || !Data || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_CUSTOMENDCAPDATA), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Size, 4, 1, U_LE);
    U_PMF_PTRSAV_SHIFT(Data, &contents, 0);
    return(1);
}

/**
    \brief Get data from a U_PMF_CUSTOMLINECAPARROWDATA object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Ccad       pointer to U_PMF_CUSTOMLINECAPARROWDATA structure

    EMF+ manual 2.2.2.12, Microsoft name: EmfPlusCustomLineCapArrowData Object
*/
int U_PMF_CUSTOMLINECAPARROWDATA_get(const char *contents, U_PMF_CUSTOMLINECAPARROWDATA *Ccad, const char *blimit){
    if(!contents || !Ccad || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_CUSTOMLINECAPARROWDATA), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Ccad, 4, 13, U_LE);
    return(1);
}

/**
    \brief Get data from a U_PMF_CUSTOMLINECAPDATA object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Clcd       pointer to U_PMF_CUSTOMLINECAPDATA structure, with no variable part
    \param  Data       variable part of U_PMF_CUSTOMLINECAPDATA

    EMF+ manual 2.2.2.13, Microsoft name: EmfPlusCustomLineCapData Object

    Caller must check Data for possible memory access violations.
*/
int U_PMF_CUSTOMLINECAPDATA_get(const char *contents, U_PMF_CUSTOMLINECAPDATA *Clcd, const char **Data, const char *blimit){
    if(!contents || !Clcd || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_CUSTOMLINECAPDATA), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Clcd, 4, 12, U_LE);
    U_PMF_PTRSAV_SHIFT(Data, &contents, 0);
    return(1);
}

/**
    \brief Get data from a U_PMF_CUSTOMLINECAPOPTIONALDATA object
    \return on success 3,5, or 7 (for varying combinations of data present) or 1 (no data is present), 0 on error
    \param  contents   Record from which to extract data
    \param  Flags      bits set to indicate the presence of FillData and/or LineData
    \param  FillData   Path to fill (optional)
    \param  LineData   Path to stroke (optional)

    EMF+ manual 2.2.2.14, Microsoft name: EmfPlusCustomLineCapOptionalData Object

    Caller must check LineData for possible memory access violations.
*/
int U_PMF_CUSTOMLINECAPOPTIONALDATA_get(const char *contents, uint32_t Flags, const char **FillData, const char **LineData, const char *blimit){
    uint32_t length;
    int status = 1;
    if(!contents || !*FillData || !*LineData || !blimit){ return(0); }
    /* this structure is entirely optional */
    if(Flags & U_CLCD_FillPath){
       if(!FillData){ return(0); }
       if(IS_MEM_UNSAFE(contents, 4, blimit))return(0);
       U_PMF_SERIAL_get(&contents, &length, 4, 1, U_LE);
       if(IS_MEM_UNSAFE(contents, length, blimit))return(0);
       contents -= 4;  /* undo the unneeded shift from preceding */
       U_PMF_PTRSAV_SHIFT(FillData, &contents, 4 + length);
       status += 2; 
    }
    else { *FillData = NULL; }

    if(Flags & U_CLCD_LinePath){
       if(!LineData){ return(0); }
       if(IS_MEM_UNSAFE(contents, 4, blimit))return(0);
       U_PMF_SERIAL_get(&contents, &length, 4, 1, U_LE);
       if(IS_MEM_UNSAFE(contents, length, blimit))return(0);
       contents -= 4;  /* undo the unneeded shift from preceding */
       U_PMF_PTRSAV_SHIFT(LineData, &contents, 0);
       status += 4;
    }
    else { *LineData = NULL; }
    return(status);
}

/**
    \brief Get data from a U_PMF_CUSTOMSTARTCAPDATA object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Size       Bytes in Data
    \param  Data       Description of linecap

    EMF+ manual 2.2.2.15, Microsoft name: EmfPlusCustomStartCapData Object

    Caller must check Data for possible memory access violations.
*/
int U_PMF_CUSTOMSTARTCAPDATA_get(const char *contents, int32_t *Size, const char **Data, const char *blimit){
    if(!contents || !Size || !Data || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_CUSTOMSTARTCAPDATA), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Size, 4, 1, U_LE);
    U_PMF_PTRSAV_SHIFT(Data, &contents, 0);
    return(1);
}

/**
    \brief Get data from a U_PMF_DASHEDLINEDATA object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Elements   Members in the array
    \param  Lengths    Caller must free.  Pointer to memory holding lengths of dashes and spaces.

    EMF+ manual 2.2.2.16, Microsoft name: EmfPlusDashedLineData Object
*/
int U_PMF_DASHEDLINEDATA_get(const char *contents, int32_t *Elements, U_FLOAT **Lengths, const char *blimit){
    if(!contents || !Elements || !Lengths || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_DASHEDLINEDATA), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Elements, 4, 1, U_LE);
    if(IS_MEM_UNSAFE(contents, *Elements * sizeof(U_FLOAT), blimit))return(0);
    *Lengths = (U_FLOAT *)malloc(*Elements * sizeof(U_FLOAT));
    if(!*Lengths){ return(0); }
    U_PMF_SERIAL_get(&contents, *Lengths, 4, *Elements, U_LE);
    return(1);
}

/**
    \brief Get data from a U_PMF_FILLPATHOBJ object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Size       Bytes in Data
    \param  Data       Path specification

    EMF+ manual 2.2.2.17, Microsoft name: EmfPlusFillPath Object

    Caller must check Data for possible memory access violations.
*/
int U_PMF_FILLPATHOBJ_get(const char *contents, int32_t *Size, const char **Data, const char *blimit){
    if(!contents || !Size || !Data || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_FILLPATHO), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Size, 4, 1, U_LE);
    U_PMF_PTRSAV_SHIFT(Data, &contents, 0);
    return(1);
}

/**
    \brief Get data from a U_PMF_FOCUSSCALEDATA object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Count      must be 2
    \param  ScaleX     value 0.0 <-> 1.0
    \param  ScaleY     value 0.0 <-> 1.0

    EMF+ manual 2.2.2.18, Microsoft name: EmfPlusFocusScaleData Object
*/
int U_PMF_FOCUSSCALEDATA_get(const char *contents, uint32_t *Count, U_FLOAT *ScaleX, U_FLOAT *ScaleY, const char *blimit){
    if(!contents || !Count || !ScaleX || !ScaleY || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_FOCUSSCALEDATA), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Count,  4, 1, U_LE);
    if(*Count != 2){ return(0); }
    U_PMF_SERIAL_get(&contents, ScaleX, 4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, ScaleY, 4, 1, U_LE);
    return(1);
}

/**
    \brief Get data from a U_PMF_GRAPHICSVERSION object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Signature  Must be U_GFVR_PMF (0xDBC01)
    \param  GrfVersion GraphicsVersion enumeration

    EMF+ manual 2.2.2.19, Microsoft name: EmfPlusGraphicsVersion Object
*/
int U_PMF_GRAPHICSVERSION_get(const char *contents, int *Signature, int *GrfVersion, const char *blimit){
    uint32_t tmp;
    if(!contents || !Signature || !GrfVersion || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_GRAPHICSVERSION), blimit))return(0);
    memcpy(&tmp, contents, 4);
    *Signature  = tmp >> 12;
    *GrfVersion = tmp & U_GFVR_MASKLO;
    return(1);
}

/**
    \brief Get data from a U_PMF_HATCHBRUSHDATA object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Style      HatchStyle enumeration
    \param  Foreground Hatch pattern line color
    \param  Background Hatch pattern bkground color

    EMF+ manual 2.2.2.20, Microsoft name: EmfPlusHatchBrushData Object
*/
int U_PMF_HATCHBRUSHDATA_get(const char *contents, uint32_t *Style, U_PMF_ARGB *Foreground, U_PMF_ARGB *Background, const char *blimit){
    if(!contents || !Style || !Foreground || !Background || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_HATCHBRUSHDATA), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Style,      4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, Foreground, 4, 1, U_XE);
    U_PMF_SERIAL_get(&contents, Background, 4, 1, U_XE);
    return(1);
}

/**
    \brief Get data from a U_PMF_INTEGER7 object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Value      7 bit signed integer (stored in an integer)
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.21, Microsoft name: EmfPlusInteger7 Object
*/
int U_PMF_INTEGER7_get(const char **contents, U_FLOAT *Value, const char *blimit){
    uint8_t tmp;
    if(!contents || !*contents || !Value || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(*contents, 1, blimit))return(0); /* past end of buffer */
    if(**contents & U_TEST_INT7)return(0);  /* this bit must be 0 in this object type */
    U_PMF_SERIAL_get(contents, &tmp, 1, 1, U_XE);
    if(tmp & U_SIGN_INT7){
       tmp |= U_TEST_INT7;  /* now it has the bit pattern of a signed int8_t */
       *Value = *(int8_t *)&tmp;
    }
    else {
       *Value = tmp;
    }
    return(1);
}

/**
    \brief Get data from a U_PMF_INTEGER15 object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Value      15 bit signed integer (stored in an integer)
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.22, Microsoft name: EmfPlusInteger15 Object
*/
int U_PMF_INTEGER15_get(const char **contents, U_FLOAT *Value, const char *blimit){
    if(!contents || !*contents || !Value || !blimit){ return(0); }
    uint16_t tmp;
    if(IS_MEM_UNSAFE(*contents, 2, blimit))return(0); /* past end of buffer */
    if(!(**contents & U_TEST_INT7))return(0);  /* this bit must be 1 in this object type */
    U_PMF_SERIAL_get(contents, &tmp, 2, 1, U_BE);
    tmp &= U_MASK_INT15;   /* drop the 7/15 flag from the most significant bit */
    if(tmp & U_SIGN_INT15){
       tmp |= U_TEST_INT15;  /* now it has the bit pattern of a signed int16_t */
       *Value = *(int16_t *)&tmp;
    }
    else {
       *Value = tmp;
    }
    return(1);
}

/**
    \brief Get data from a U_PMF_LANGUAGEIDENTIFIER object
    \return 1 on success, 0 on error
    \param  LId        U_PMF_LANGUAGEIDENTIFIER from which to extract data
    \param  SubLId     Example: code for USA
    \param  PriLId     Example: code for English
    


    EMF+ manual 2.2.2.23, Microsoft name: EmfPlusLanguageIdentifier Object

    This type is defined as 16 bits in the manual section, but it is only ever used as part of a 32 bit field!
*/
int U_PMF_LANGUAGEIDENTIFIER_get(U_PMF_LANGUAGEIDENTIFIER LId, int *SubLId, int *PriLId){
    if(!SubLId || !PriLId){ return(0); }
    *SubLId = (LId >> U_FF_SHFT_SUBLID ) & U_FF_MASK_SUBLID;         
    *PriLId = (LId >> U_FF_SHFT_PRILID ) & U_FF_MASK_PRILID;
    /* 16 bits above that are not used */
    return(1);
}

/**
    \brief Get data from a U_PMF_LINEARGRADIENTBRUSHDATA object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Lgbd       U_PMF_LINEARGRADIENTBRUSHDATA structure, with no variable part
    \param  Data       variable part of U_PMF_LINEARGRADIENTBRUSHDATA
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.24, Microsoft name: EmfPlusLinearGradientBrushData Object

    Caller must check Data for possible memory access violations.
*/
int U_PMF_LINEARGRADIENTBRUSHDATA_get(const char *contents, U_PMF_LINEARGRADIENTBRUSHDATA *Lgbd, const char **Data, const char *blimit){
    if(!contents || !Lgbd || !Data || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_LINEARGRADIENTBRUSHDATA), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Lgbd,		      4, 6, U_LE); /* Flags, WrapMode, RectF*/
    U_PMF_SERIAL_get(&contents, &(Lgbd->StartColor),  4, 4, U_XE); /* StartColor, EndColor, Reserved1 & 2 */
    U_PMF_PTRSAV_SHIFT(Data, &contents, 0);
    return(1);
}

/**
    \brief Get data from a U_PMF_LINEARGRADIENTBRUSHOPTIONALDATA object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Flags      BrushData flags - indicates which of the following date fields are present.
    \param  Tm         Transformation matrix
    \param  Bc         U_PMF_BLENDCOLORS object or NULL
    \param  BfH        U_PMF_BLENDFACTORS (H) object or NULL
    \param  BfV        U_PMF_BLENDFACTORS (V) object or NULL (WARNING, GDI+ defines this field but does not render it.  DO NOT USE.)
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.25, Microsoft name: EmfPlusLinearGradientBrushOptionalData Object

*/
int U_PMF_LINEARGRADIENTBRUSHOPTIONALDATA_get(const char *contents, uint32_t Flags, U_PMF_TRANSFORMMATRIX *Tm, 
      const char **Bc, const char **BfH, const char **BfV, const char *blimit){
    uint32_t Elements;
    if(!contents || !Tm|| !Bc || !BfH || !BfV || !blimit){ return(0); }
    /* all of the fields are optional! */
    *Bc = *BfH = *BfV = NULL;
    if(Flags & U_BD_Transform){
       if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_ROTMATRIX), blimit))return(0);
       U_PMF_SERIAL_get(&contents, Tm, 4, 6, U_LE);
    }
    if(Flags & U_BD_PresetColors){
       if(IS_MEM_UNSAFE(contents, 4, blimit))return(0);
       U_PMF_SERIAL_get(&contents, &Elements, 4, 1, U_LE);  /* starts with a 4 byte count*/
       if(IS_MEM_UNSAFE(contents, Elements * ( sizeof(U_FLOAT) + sizeof(U_PMF_ARGB)), blimit))return(0);
       contents-=4; /* back up to the front of the count, as it is part of the data field */
       U_PMF_PTRSAV_SHIFT(Bc, &contents, 0);
    }
    else if(Flags & U_BD_BlendFactorsH){
       U_PMF_SERIAL_get(&contents, &Elements, 4, 1, U_LE);  /* starts with a 4 byte count*/
       if(IS_MEM_UNSAFE(contents, Elements * 2 * sizeof(U_FLOAT), blimit))return(0);
       contents-=4; /* back up to the front of the count, as it is part of the data field */
       U_PMF_PTRSAV_SHIFT(BfH, &contents, 4 + (Elements * 2 * sizeof(U_FLOAT))); /* 4 byte count + 2 * 4bytes * Elements */
       if(Flags & U_BD_BlendFactorsV){
          if(IS_MEM_UNSAFE(contents, Elements * 2 * sizeof(U_FLOAT), blimit))return(0);
          U_PMF_PTRSAV_SHIFT(BfV, &contents, 0);
       }
    }
    else if(Flags & U_BD_BlendFactorsV){
       U_PMF_SERIAL_get(&contents, &Elements, 4, 1, U_LE);  /* starts with a 4 byte count*/
       if(IS_MEM_UNSAFE(contents, Elements * 2 * sizeof(U_FLOAT), blimit))return(0);
       contents-=4; /* back up to the front of the count, as it is part of the data field */
       U_PMF_PTRSAV_SHIFT(BfV, &contents, 0);
    }
    return(1);
}

/**
    \brief Get data from a U_PMF_LINEPATH object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Size       Bytes in Data
    \param  Data       Outline path
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.26, Microsoft name: EmfPlusLinePath Object

    Caller must check Data for possible memory access violations.
*/
int U_PMF_LINEPATH_get(const char *contents, int32_t *Size, const char **Data, const char *blimit){
    if(!contents || !Size  || !Data || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_LINEPATH), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Size, 4, 1, U_LE);
    U_PMF_PTRSAV_SHIFT(Data, &contents, 0);
    return(1);
}

/**
    \brief Get data from a U_PMF_METAFILE object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Type       
    \param  Size       Bytes in Data
    \param  Data       Various types of data, like an EMF metafile, WMF metafile, another EMF+ metafile
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.27, Microsoft name: EmfPlusMetafile Object

    Caller must check Data for possible memory access violations.
*/
int U_PMF_METAFILE_get(const char *contents, uint32_t *Type, uint32_t *Size, const char **Data, const char *blimit){
    if(!contents || !Type  || !Size  || !Data || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_METAFILE), blimit))return(0);
    U_PMF_SERIAL_get(&contents, &Type, 4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, &Size, 4, 1, U_LE);
    U_PMF_PTRSAV_SHIFT(Data, &contents, 0);
    return(1);
}

/**
    \brief Get data from a U_PMF_PALETTE object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Flags      PaletteStyle flags
    \param  Elements   Members in the array
    \param  Colors     Palette data (array of colors)
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.28, Microsoft name: EmfPlusPalette Object
*/
int U_PMF_PALETTE_get(const char *contents, uint32_t *Flags, uint32_t *Elements, const char **Colors, const char *blimit){
    if(!contents || !Flags  || !Elements  || !Colors || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_PALETTE), blimit))return(0);
    U_PMF_SERIAL_get(&contents, &Flags,    4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, &Elements, 4, 1, U_LE);
    if(IS_MEM_UNSAFE(contents, *Elements*sizeof(U_RGBQUAD), blimit))return(0);
    U_PMF_PTRSAV_SHIFT(Colors, &contents, 0);
    return(1);
    
}

/**
    \brief Get data from a U_PMF_PATHGRADIENTBRUSHDATA object
    \return 1 on success, 0 on error
    \param  contents     Record from which to extract data
    \param  Pgbd         constant part of U_PMF_PATHGRADIENTBRUSHDATA object
    \param  Gradient     variable part of U_PMF_LINEARGRADIENTBRUSHDATA, Color Gradient with Elements members
    \param  Boundary     variable part of U_PMF_LINEARGRADIENTBRUSHDATA, U_PMF_BOUNDARYPATHDATA object if BrushDataPath bit set in Flag, else U_PMF_BOUNDARYPOINTDATA object
    \param  Data         variable part of U_PMF_LINEARGRADIENTBRUSHDATA, exact composition depends on Flags
    \param  blimit       one byte past the end of data

    EMF+ manual 2.2.2.29, Microsoft name: EmfPlusPathGradientBrushData Object

    Caller must check Data for possible memory access violations.

*/
int U_PMF_PATHGRADIENTBRUSHDATA_get(const char *contents, U_PMF_PATHGRADIENTBRUSHDATA *Pgbd, const char **Gradient, 
      const char **Boundary, const char **Data, const char *blimit){
    if(!contents || !Pgbd || !Gradient || !Boundary || !Data || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_PATHGRADIENTBRUSHDATA), blimit))return(0);
    uint32_t Size;
    U_PMF_SERIAL_get(&contents,  Pgbd,  	       4, 2, U_LE); /* Flags and WrapMode*/
    U_PMF_SERIAL_get(&contents,  &(Pgbd->CenterColor), 4, 1, U_XE);
    U_PMF_SERIAL_get(&contents,  &(Pgbd->Center),      4, 3, U_LE); /* Center and Elements */
    if(IS_MEM_UNSAFE(contents, Pgbd->Elements * sizeof(U_PMF_ARGB), blimit))return(0);
    U_PMF_PTRSAV_SHIFT(Gradient, &contents, Pgbd->Elements * sizeof(U_PMF_ARGB));
    U_PMF_PTRSAV_SHIFT(Boundary, &contents, 0);
    U_PMF_SERIAL_get(&contents,  &Size,                4, 1, U_LE); /* The first 4 bytes of the Boundary are always a size */
    if(Pgbd->Flags & U_BD_Path){ contents += Size;                   } // U_PMF_BOUNDARYPATHDATA
    else {                       contents += Size*2*sizeof(U_FLOAT); } // U_PMF_BOUNDARYPOINTDATA
    if(Pgbd->Flags & (U_BD_Transform |U_BD_PresetColors | U_BD_BlendFactorsH| U_BD_FocusScales)){ // optional data present
      if(contents >= blimit)return(0); // the size is variable but this must still hold
      U_PMF_PTRSAV_SHIFT(Data,     &contents, 0);
    }
    else { *Data = NULL; } // no optional data present
    return(1);
}

/**
    \brief Get data from a U_PMF_PATHGRADIENTBRUSHOPTIONALDATA object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Flags      bits set to indicate the presence of FillData and/or LineData
    \param  Matrix     Transformation matrix
    \param  Pattern    Blend Pattern
    \param  Data       Focus scales for the brush
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.30, Microsoft name: EmfPlusPathGradientBrushOptionalData Object
*/
int U_PMF_PATHGRADIENTBRUSHOPTIONALDATA_get(const char *contents, uint32_t Flags, U_PMF_TRANSFORMMATRIX *Matrix, 
      const char **Pattern, const char **Data, const char *blimit){
    int varsize;
    if(!contents || !Flags || !Matrix || !Pattern || !Data || !blimit){ return(0); }
    /* this structure is entirely optional */
    if(Flags & U_BD_Transform){
       if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_TRANSFORMMATRIX), blimit))return(0);
       U_PMF_SERIAL_get(&contents, Matrix, 4, 6, U_LE);
    }
    if(Flags &  (U_BD_PresetColors | U_BD_BlendFactorsH)){
       if(IS_MEM_UNSAFE(contents, 4, blimit))return(0);
       uint32_t Elements; 
       U_PMF_SERIAL_get(&contents, &Elements, 4, 1, U_LE);
       contents -= 4;
       varsize=(Elements * 4 * (Flags & U_BD_BlendFactorsH ? 2 :1));
       if(IS_MEM_UNSAFE(contents, varsize, blimit))return(0);
       U_PMF_PTRSAV_SHIFT(Pattern, &contents, varsize);
    }
    else { *Pattern=NULL; }
    if(Flags & U_BD_FocusScales){
       if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_FOCUSSCALEDATA), blimit))return(0);
       U_PMF_PTRSAV_SHIFT(Data, &contents, sizeof(U_PMF_FOCUSSCALEDATA));
    }
    else { *Data=NULL; }
    return(1);
}

/**
    \brief Get data from a U_PMF_PATHPOINTTYPE object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Flags      PathPointType flags
    \param  Type       PathPointType enumeration
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.31, Microsoft name: EmfPlusPathPointType Object

    Note:  order of 4bit fields appears to be shown in the LE column, not as
    documented in the BE column.
*/
int U_PMF_PATHPOINTTYPE_get(const char *contents, int *Flags, int *Type, const char *blimit){
    if(!contents || !Flags || !Type || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, 1, blimit))return(0);
    uint8_t tmp;
    memcpy(&tmp, contents, 1);
    *Flags =(tmp & U_PTP_MASK) >> U_PTP_SHIFT;
    *Type = (tmp & U_PPT_MASK);
    return(1);
}

/**
    \brief Get data from a U_PMF_PATHPOINTTYPERLE object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Bezier     Set: Bezier curve, Clear: straight line
    \param  RL         Run Length
    \param  Ppt        PathPointType enumeration
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.32, Microsoft name: EmfPlusPathPointTypeRLE Object
*/
int U_PMF_PATHPOINTTYPERLE_get(const char *contents, int *Bezier, int *RL, int *Ppt, const char *blimit){
    if(!contents || !Bezier || !RL || !Ppt || !blimit){ return(0); }
    uint16_t tmp;
    if(IS_MEM_UNSAFE(contents, 2, blimit))return(0);
    U_PMF_SERIAL_get(&contents, &tmp, 2, 1, U_LE);
    *Bezier = tmp & U_PPF_BZ;
    *RL     = (tmp >> U_FF_SHFT_RL) & U_FF_MASK_RL;
    *Ppt    = (tmp >> U_FF_SHFT_PPT) & U_FF_MASK_PPT;
    return(1);
}

/**
    \brief Get data from a U_PMF_PENDATA object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Flags      PenData flags
    \param  Unit       UnitType enumeration
    \param  Width      Width in units set by Unit
    \param  Data       Optional pen data, exact composition depends on Flags
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.33, Microsoft name: EmfPlusPenData Object
*/
int U_PMF_PENDATA_get(const char *contents, uint32_t *Flags, uint32_t *Unit, U_FLOAT *Width, const char **Data, const char *blimit){
    if(!contents || !Flags || !Unit || !Width || !Data || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, 3*4, blimit))return(0);
    U_PMF_SERIAL_get(&contents, Flags, 4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, Unit,  4, 1, U_LE);
    U_PMF_SERIAL_get(&contents, Width, 4, 1, U_LE);
    if(contents >= blimit)return(0); // variable data will extend farther, but this much at least must be true
    U_PMF_PTRSAV_SHIFT(Data, &contents, 0);
    return(1);
}

/**
    \brief Get data from a U_PMF_PENOPTIONALDATA object
    \return 1 on success, 0 on error
    \param  contents       Record from which to extract data
    \param  Flags;         PenData Flags - indicated which of the many fields are present.
    \param  Matrix;        Transformation matrix
    \param  StartCap       LineCapType enumeration
    \param  EndCap         LineCapType enumeration
    \param  Join           LineJoinType enumeration
    \param  MiterLimit     Maximum (miter length / line width)
    \param  Style          LineStyle enumeration
    \param  DLCap          DashedLineCapType enumeration
    \param  DLOffset       Distance line start to first dash start
    \param  DLData         Dash and space widths
    \param  Alignment      PenAlignment enumeration
    \param  CmpndLineData  Compount Line (parallel lines drawn instead of one)
    \param  CSCapData      Custom start cap  
    \param  CECapData      Custom end cap
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.34, Microsoft name: EmfPlusPenOptionalData Object

    This object consists of a large number of optional and or variable values, which
    are returned, or not, depending on bits in Flags.
*/
int U_PMF_PENOPTIONALDATA_get(
    const char                *contents,
    uint32_t                   Flags,         // determines which fields are filled
    U_PMF_TRANSFORMMATRIX     *Matrix,
    int32_t                   *StartCap,
    int32_t                   *EndCap,
    uint32_t                  *Join,
    U_FLOAT                   *MiterLimit,
    int32_t                   *Style,
    int32_t                   *DLCap,
    U_FLOAT                   *DLOffset,
    const char               **DLData,
    int32_t                   *Alignment,
    const char               **CmpndLineData,
    const char               **CSCapData,
    const char               **CECapData,
    const char                *blimit){
    if(!contents  ||
       !Flags     || !Matrix     || !StartCap  || !EndCap        ||
       !Join      || !MiterLimit || !Style     || !DLCap         ||
       !DLOffset  || !DLData     || !Alignment || !CmpndLineData || 
       !CSCapData || !CECapData  || !blimit){ return(0); }

    if(Flags & U_PD_Transform){       if(IS_MEM_UNSAFE(contents, 4*6, blimit))return(0);
                                      U_PMF_SERIAL_get(&contents, Matrix,         4, 6, U_LE); 
    }
    if(Flags & U_PD_StartCap){        if(IS_MEM_UNSAFE(contents, 4, blimit))return(0);
                                      U_PMF_SERIAL_get(&contents, StartCap,       4, 1, U_LE); 
    }
    if(Flags & U_PD_EndCap){          if(IS_MEM_UNSAFE(contents, 4, blimit))return(0);
                                      U_PMF_SERIAL_get(&contents, EndCap,         4, 1, U_LE); 
    }
    if(Flags & U_PD_Join){            if(IS_MEM_UNSAFE(contents, 4, blimit))return(0);
                                      U_PMF_SERIAL_get(&contents, Join,           4, 1, U_LE); 
    }
    if(Flags & U_PD_MiterLimit){      if(IS_MEM_UNSAFE(contents, 4, blimit))return(0);
                                      U_PMF_SERIAL_get(&contents, MiterLimit,     4, 1, U_LE); 
    }
    if(Flags & U_PD_LineStyle){       if(IS_MEM_UNSAFE(contents, 4, blimit))return(0);
                                      U_PMF_SERIAL_get(&contents, Style,          4, 1, U_LE); 
    }
    if(Flags & U_PD_DLCap){           if(IS_MEM_UNSAFE(contents, 4, blimit))return(0);
                                      U_PMF_SERIAL_get(&contents, DLCap,          4, 1, U_LE); 
    }
    if(Flags & U_PD_DLOffset){        if(IS_MEM_UNSAFE(contents, 4, blimit))return(0);
                                      U_PMF_SERIAL_get(&contents, DLOffset,       4, 1, U_LE); 
    }
    if(Flags & U_PD_DLData){          if(IS_MEM_UNSAFE(contents, 4, blimit))return(0);
                                      if(IS_MEM_UNSAFE(contents, U_PMF_LEN_FLOATDATA(contents), blimit))return(0);
                                      U_PMF_PTRSAV_SHIFT(   DLData,         &contents, U_PMF_LEN_FLOATDATA(contents)); 
    }
    if(Flags & U_PD_NonCenter){       if(IS_MEM_UNSAFE(contents, 4, blimit))return(0);
                                      U_PMF_SERIAL_get(&contents, Alignment,      4, 1, U_LE); }
    if(Flags & U_PD_CLData){          if(IS_MEM_UNSAFE(contents, 4, blimit))return(0);
                                      if(IS_MEM_UNSAFE(contents, U_PMF_LEN_FLOATDATA(contents), blimit))return(0);
                                      U_PMF_PTRSAV_SHIFT(   CmpndLineData,  &contents, U_PMF_LEN_FLOATDATA(contents)); 
    }
    if(Flags & U_PD_CustomStartCap){  if(IS_MEM_UNSAFE(contents, 4, blimit))return(0);
                                      if(IS_MEM_UNSAFE(contents, U_PMF_LEN_BYTEDATA(contents), blimit))return(0);
                                      U_PMF_PTRSAV_SHIFT(   CSCapData,      &contents, U_PMF_LEN_BYTEDATA(contents));  
    }
    if(Flags & U_PD_CustomEndCap){    if(IS_MEM_UNSAFE(contents, 4, blimit))return(0);
                                      if(IS_MEM_UNSAFE(contents, U_PMF_LEN_BYTEDATA(contents), blimit))return(0);
                                      U_PMF_PTRSAV_SHIFT(   CECapData,      &contents, U_PMF_LEN_BYTEDATA(contents));  
    }
    return(1);
}

/**
    \brief Get data from a U_PMF_POINT object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data.  On return position is offset by sizeof(U_PMF_POINT).
    \param  X          X coordinate
    \param  Y          Y coordinate
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.35, Microsoft name: EmfPlusPoint Object
*/
int U_PMF_POINT_get(const char **contents, U_FLOAT *X, U_FLOAT *Y, const char *blimit){
    if(!contents || !X || !Y || !blimit){ return(0); }
    int16_t tmp;
    if(IS_MEM_UNSAFE(*contents, 2*2, blimit))return(0);
    U_PMF_SERIAL_get(contents, &tmp, 2, 1, U_LE); *X = tmp;
    U_PMF_SERIAL_get(contents, &tmp, 2, 1, U_LE); *Y = tmp;
    return(1);
}

/**
    \brief Get data from a U_PMF_POINTF object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data.  On return position is offset by sizeof(U_PMF_POINTF).
    \param  X          X coordinate
    \param  Y          Y coordinate
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.36, Microsoft name: EmfPlusPointF Object
*/
int U_PMF_POINTF_get(const char **contents, U_FLOAT *X, U_FLOAT *Y, const char *blimit){
    if(!contents || !X || !Y || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(*contents, 4*2, blimit))return(0);
    U_PMF_SERIAL_get(contents, X, 4, 1, U_LE);
    U_PMF_SERIAL_get(contents, Y, 4, 1, U_LE);
    return(1);
}

/**
    \brief Get data from a U_PMF_POINTR object
    \return size in bytes traversed on success, 0 on error
    \param  contents   Record from which to extract data.  On return position is offset by returned size. 
    \param  X          X coordinate
    \param  Y          Y coordinate
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.37, Microsoft name: EmfPlusPointR Object
*/
int U_PMF_POINTR_get(const char **contents, U_FLOAT *X, U_FLOAT *Y, const char *blimit){
    if(!contents || !*contents | !X || !Y || !blimit){ return(0); }
    int size=0;

    if(     U_PMF_INTEGER7_get( contents, X, blimit)){ size +=1; } 
    else if(U_PMF_INTEGER15_get(contents, X, blimit)){ size +=2; }
    else {  return(0); }

    if(     U_PMF_INTEGER7_get( contents, Y, blimit)){ size +=1; } 
    else if(U_PMF_INTEGER15_get(contents, Y, blimit)){ size +=2; }
    else {  return(0); }

    return(size);
}

/**
    \brief Get data from a variable POINTS object, which may be U_PMF_POINTS, U_PMF_POINTF, or U_PMF_POINTR.
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Flags      Record flags (bits U_PPF_C and U_PPF_P are referenced)
    \param  Elements   Number of points to retrieve.
    \param  Points     Caller must free.  Array of U_PMF_POINTF coordinates.
    \param  blimit     one byte past the end of data
    
    This function should never be called directly by end user code.
*/
int U_PMF_VARPOINTS_get(const char *contents, uint16_t Flags, int Elements, U_PMF_POINTF **Points, const char *blimit){
   int status = 0;
   if(!contents  || !Points || !Elements || !blimit){ return(status); }
   U_PMF_POINTF *pts = (U_PMF_POINTF *)malloc(Elements * sizeof(U_PMF_POINTF));
   if(!pts){ return(status); }
   *Points = pts;
   U_FLOAT XF, YF;
   U_FLOAT XFS, YFS;
   
   if(Flags & U_PPF_P){ 
      for(XFS = YFS = 0.0; Elements; Elements--, pts++){
         if(!U_PMF_POINTR_get(&contents, &XF, &YF, blimit))return(0); /* this should never happen */
         XFS      += XF; /* position relative to previous point, first point is always 0,0 */
         YFS      += YF;
         pts->X    = XFS;
         pts->Y    = YFS; 
      }
   }
   else if(Flags & U_PPF_C){
      for(XF = YF = 0.0; Elements; Elements--, pts++){
         if(!U_PMF_POINT_get(&contents, &XF, &XF, blimit))break; /* this should never happen */
         pts->X    = XF;
         pts->Y    = YF; 
      }
   }
   else {
      for(XF = YF = 0.0; Elements; Elements--, pts++){
         (void) U_PMF_POINTF_get(&contents, &XF, &YF, blimit); 
         pts->X    = XF;
         pts->Y    = YF; 
      }
   }
   if(Elements){ /* some error in the preceding */
      free(*Points);
      *Points = NULL;
   }
   else {
      status = 1;
   }
   return(status);
}

/**
    \brief Get data from a U_PMF_RECT object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  X          UL X value
    \param  Y          UL Y value
    \param  Width      Width
    \param  Height     Height
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.38, Microsoft name: EmfPlusRect Object
*/
int U_PMF_RECT_get(const char **contents, int16_t *X, int16_t *Y, int16_t *Width, int16_t *Height, const char *blimit){
    if(!contents || !X || !Y|| !Width || !Height){ return(0); }
    if(IS_MEM_UNSAFE(*contents, 2*4, blimit))return(0);
    U_PMF_SERIAL_get(contents, X,      2, 1, U_LE);
    U_PMF_SERIAL_get(contents, Y,      2, 1, U_LE);
    U_PMF_SERIAL_get(contents, Width,  2, 1, U_LE);
    U_PMF_SERIAL_get(contents, Height, 2, 1, U_LE);
    return(1);
}

/**
    \brief Get data from a U_PMF_RECTF object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  X          UL X value
    \param  Y          UL Y value
    \param  Width      Width
    \param  Height     Height
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.39, Microsoft name: EmfPlusRectF Object
*/
int U_PMF_RECTF_get(const char **contents, U_FLOAT *X, U_FLOAT *Y, U_FLOAT *Width, U_FLOAT *Height, const char *blimit){
    if(!contents || !X || !Y|| !Width || !Height){ return(0); }
    if(IS_MEM_UNSAFE(*contents, 4*4, blimit))return(0);
    U_PMF_SERIAL_get(contents, X,      4, 1, U_LE);
    U_PMF_SERIAL_get(contents, Y,      4, 1, U_LE);
    U_PMF_SERIAL_get(contents, Width,  4, 1, U_LE);
    U_PMF_SERIAL_get(contents, Height, 4, 1, U_LE);
    return(1);
}

/**
    \brief Get data from a variable RECTS object, which may be U_PMF_RECT or U_PMF_RECTF
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Flags      Record flags (bit U_PPF_C is referenced)
    \param  Elements   Number of rects to retrieve.
    \param  Rects      Caller must free.  Array of U_PMF_RECTF coordinates.
    \param  blimit     one byte past the end of data

    Rects in record may be either U_PMF_RECT or U_PMF_RECTF, but this function always
    returns U_PMF_RECTF
*/
int U_PMF_VARRECTS_get(const char **contents, uint16_t Flags, int Elements, U_PMF_RECTF **Rects, const char *blimit){
   int16_t X16, Y16, Width, Height;
   if(!contents || !*contents || !Rects || !blimit){ return(0); }
   U_PMF_RECTF *rts = (U_PMF_RECTF *)malloc(Elements * sizeof(U_PMF_RECTF));
   if(!rts){
      *Rects = NULL;
      return(0);
   }
   
   *Rects = rts;
   if(Flags & U_PPF_C){
      if(IS_MEM_UNSAFE(*contents, Elements*sizeof(U_PMF_RECT), blimit)){
         free(rts);
         return(0);
      }
   }
   else {
      if(IS_MEM_UNSAFE(*contents, Elements*sizeof(U_PMF_RECT), blimit)){
         free(rts);
         return(0);
      }
   }
   for(; Elements; Elements--, rts++){
      if(Flags & U_PPF_C){
         (void) U_PMF_RECT_get(contents, &X16, &Y16, &Width, &Height, blimit);  
         rts->X      = X16;
         rts->Y      = Y16;
         rts->Width  = Width;
         rts->Height = Height;
      }
      else {
         (void) U_PMF_RECTF_get(contents, &(rts->X), &(rts->Y), &(rts->Width), &(rts->Height), blimit); 
      }
   }
   return(1);
}

/**
    \brief Get data from a U_PMF_REGIONNODE object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Type       RegionNodeDataType
    \param  Data       Depending on Type: U_PMF_REGIONNODEPATH, U_PMF_RECTF, or U_PMF_REGIONNODECHILDNODES
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.40, Microsoft name: EmfPlusRegionNode Object

    Caller must check Data for possible memory access violations.
*/
int U_PMF_REGIONNODE_get(const char *contents, uint32_t *Type, const char **Data, const char *blimit){
    if(!contents || !Type || !Data || !blimit){ return(0); }
    /* Important!  This only checks the constant part, the caller must check that returned data doesn't exceed blimit */
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_REGIONNODE), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Type, 4, 1, U_LE);
    if(contents >= blimit)return(0); // returned Data is variable size, this much at least must be true
    U_PMF_PTRSAV_COND(Data, contents, !(*Type == U_RNDT_Empty || *Type == U_RNDT_Infinite ));
    return(1);
}

/**
    There is no U_PMF_REGIONNODECHILDNODES_get!
      
    The Region object is recursive allowing U_PMF_REGIONNODECHILDNODES -> 
       U_PMF_REGIONNODE -> U_PMF_REGIONNODECHILDNODES etc.
    So the data stored in each node must be handled as the tree is followed recursively.

    See U_PMF_REGIONNODECHILDNODES_print() and U_PMF_REGIONNODE_print() for an example.


    EMF+ manual 2.2.2.41, Microsoft name: EmfPlusRegionNodeChildNodes Object
*/

/**
    \brief Get data from a U_PMF_REGIONNODEPATH object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Size       Bytes in Data
    \param  Data       Boundary of region node
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.42, Microsoft name: EmfPlusRegionNodePath Object
*/
int U_PMF_REGIONNODEPATH_get(const char *contents, int32_t *Size, const char **Data, const char *blimit){
    if(!contents || !Size || !Data || !blimit){ return(0); }
    /* Important!  This only checks the constant part, the caller must check that returned data doesn't exceed blimit */
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_REGIONNODEPATH), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Size, 4, 1, U_LE);
    if(contents >= blimit)return(0); // returned Data is variable size, this much at least must be true
    U_PMF_PTRSAV_SHIFT(Data, &contents, 0);
    return(1);
}

/**
    \brief Get data from a U_PMF_SOLIDBRUSHDATA object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Color      Color of brush
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.43, Microsoft name: EmfPlusSolidBrushData Object
*/
int U_PMF_SOLIDBRUSHDATA_get(const char *contents, U_PMF_ARGB *Color, const char *blimit){
    if(!contents || !Color || !blimit){ return(0); }
    if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_SOLIDBRUSHDATA), blimit))return(0);
    U_PMF_SERIAL_get(&contents, Color, 4, 1, U_XE);
    return(1);
}

/**
    \brief Get data from a U_PMF_STRINGFORMATDATA object
    \return 1 on success, 0 on error
    \param  contents      Record from which to extract data
    \param  TabStopCount  Entries in TabStop array
    \param  RangeCount    Entries in CharRange array
    \param  TabStops      Array of tabstop locations
    \param  CharRange     Array of character ranges in the text
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.44, Microsoft name: EmfPlusStringFormatData Object
*/
int U_PMF_STRINGFORMATDATA_get(const char *contents, uint32_t TabStopCount, uint32_t RangeCount, 
      const U_FLOAT **TabStops, const U_PMF_CHARACTERRANGE **CharRange, const char *blimit){
   if(!contents || !TabStops|| !CharRange || !blimit){ return(0); }
   if(IS_MEM_UNSAFE(contents, (TabStopCount + 2*RangeCount)*4, blimit))return(0);
   *TabStops = NULL;
   if(TabStopCount > 0){ U_PMF_SERIAL_get(&contents, TabStops,  4, TabStopCount, U_LE); }
   *CharRange = NULL;
   if(RangeCount   > 0){ U_PMF_SERIAL_get(&contents, CharRange, 4, 2*RangeCount, U_LE); }
   return(1);
}

/**
    \brief Get data from a U_PMF_TEXTUREBRUSHDATA object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Flags      BrushData flags
    \param  WrapMode   WrapMode enumeration
    \param  Data       Optional texture data
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.45, Microsoft name: EmfPlusTextureBrushData Object

    Caller must check Data for possible memory access violations.
*/
int U_PMF_TEXTUREBRUSHDATA_get(const char *contents, uint32_t *Flags, int32_t *WrapMode, const char **Data, const char *blimit){
   if(!contents || !Flags || !WrapMode || !Data || !blimit){ return(0); }
   /* Important!  This only checks the constant part, the caller must check that returned data doesn't exceed blimit */
   if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_TEXTUREBRUSHDATA), blimit))return(0);
   U_PMF_SERIAL_get(&contents, Flags,    4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, WrapMode, 4, 1, U_LE);
   if(contents >= blimit)return(0); // returned Data is variable size, this much at least must be true
   U_PMF_PTRSAV_SHIFT(Data, &contents, 0);
   return(1);
}

/**
    \brief Get data from a U_PMF_TEXTUREBRUSHOPTIONALDATA object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  HasImage   True if this object has an Image
    \param  Matrix     Transformation matrix, NULL if Flag BrushDataTransform is not set.
    \param  Image      Image that contains the texture.
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.46, Microsoft name: EmfPlusTextureBrushOptionalData Object

    Caller must check Image for possible memory access violations.
*/
int U_PMF_TEXTUREBRUSHOPTIONALDATA_get(const char *contents, int HasImage, U_PMF_TRANSFORMMATRIX *Matrix, 
      const char **Image, const char *blimit){
   if(!contents || !Image || !blimit){ return(0); }
   if(Matrix){
      if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_TRANSFORMMATRIX), blimit))return(0);
      U_PMF_SERIAL_get(&contents, Matrix,   4, 6, U_LE);
   }
   if(HasImage){
      if(contents >= blimit)return(0); // returned Data is variable size, this much at least must be true
      U_PMF_PTRSAV_COND(Image, contents, HasImage);
   }
   return(1);
}

/**
    \brief Get data from a U_PMF_TRANSFORMMATRIX object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Matrix     Transformation matrix, present if Flag BrushDataTransform is set.
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.2.47, Microsoft name: EmfPlusTransformMatrix Object
*/
int U_PMF_TRANSFORMMATRIX_get(const char *contents, U_PMF_TRANSFORMMATRIX *Matrix, const char *blimit){
   if(!contents || !Matrix || !blimit){ return(0); }
   if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_TRANSFORMMATRIX), blimit))return(0);
   U_PMF_SERIAL_get(&contents, Matrix, 4, 6, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMF_IE_BLUR object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Radius     Blur radius in pixels
    \param  ExpandEdge 1: expand bitmap by Radius; 0: bitmap size unchanged
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.3.1, Microsoft name: BlurEffect Object
*/
int U_PMF_IE_BLUR_get(const char *contents, U_FLOAT *Radius, uint32_t *ExpandEdge, const char *blimit){
   if(!contents || !Radius || !ExpandEdge || !blimit){ return(0); }
   if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_IE_BLUR), blimit))return(0);
   U_PMF_SERIAL_get(&contents, Radius,     4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, ExpandEdge, 4, 1, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMF_IE_BRIGHTNESSCONTRAST object
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Brightness  -255 to 255, 0 is unchanged, positive increases, negative decreases
    \param  Contrast    -100 to 100, 0 is unchanged, positive increases, negative decreases
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.3.2, Microsoft name: BrightnessContrastEffect Object
*/
int U_PMF_IE_BRIGHTNESSCONTRAST_get(const char *contents, int32_t *Brightness, int32_t *Contrast, const char *blimit){
   if(!contents || !Brightness || !Contrast || !blimit){ return(0); }
   if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_IE_BRIGHTNESSCONTRAST), blimit))return(0);
   U_PMF_SERIAL_get(&contents, Brightness, 4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Contrast,   4, 1, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMF_IE_COLORBALANCE object
    \return 1 on success, 0 on error
    \param  contents     Record from which to extract data
    \param  CyanRed      -100 to 100, 0 is unchanged, positive increases Red   & decreases Cyan,    negative is opposite
    \param  MagentaGreen -100 to 100, 0 is unchanged, positive increases Green & decreases Magenta, negative is opposite
    \param  YellowBlue   -100 to 100, 0 is unchanged, positive increases Blue  & decreases Yellow,  negative is opposite
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.3.3, Microsoft name: ColorBalanceEffect Object
*/
int U_PMF_IE_COLORBALANCE_get(const char *contents, int32_t *CyanRed, int32_t *MagentaGreen, int32_t *YellowBlue, const char *blimit){
   if(!contents || !CyanRed || !MagentaGreen || !YellowBlue || !blimit){ return(0); }
   if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_IE_COLORBALANCE), blimit))return(0);
   U_PMF_SERIAL_get(&contents, CyanRed,      4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, MagentaGreen, 4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, YellowBlue,   4, 1, U_LE);
   return(1);
}


/**
    \brief Get data from a U_PMF_IE_COLORCURVE object
    \return 1 on success, 0 on error
    \param  contents     Record from which to extract data
    \param  Adjust       CurveAdjustment enumeration
    \param  Channel      CurveChannel enumeration
    \param  Intensity    adjustment to apply.  "Adjust" determines what field this is and range values.
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.3.4, Microsoft name: ColorCurveEffect Object
*/
int U_PMF_IE_COLORCURVE_get(const char *contents, uint32_t *Adjust, uint32_t *Channel, int32_t *Intensity, const char *blimit){
   if(!contents || !Adjust || !Channel || !Intensity || !blimit){ return(0); }
   if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_IE_COLORCURVE), blimit))return(0);
   U_PMF_SERIAL_get(&contents, Adjust,    4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Channel,   4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Intensity, 4, 1, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMF_IE_COLORLOOKUPTABLE object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  BLUT       Blue  color lookup table
    \param  GLUT       Green color lookup table
    \param  RLUT       Red   color lookup table
    \param  ALUT       Alpha color lookup table
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.3.5, Microsoft name: ColorLookupTableEffect Object
*/
int U_PMF_IE_COLORLOOKUPTABLE_get(const char *contents, 
      const uint8_t **BLUT, const uint8_t **GLUT, const uint8_t **RLUT, const uint8_t **ALUT, const char *blimit){
   if(!contents || !BLUT || !GLUT || !RLUT || !ALUT || !blimit){ return(0); }
   if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_IE_COLORLOOKUPTABLE) + 4 * 256, blimit))return(0);
   U_PMF_PTRSAV_SHIFT((const char **)BLUT, &contents, 256);
   U_PMF_PTRSAV_SHIFT((const char **)GLUT, &contents, 256);
   U_PMF_PTRSAV_SHIFT((const char **)RLUT, &contents, 256);
   U_PMF_PTRSAV_SHIFT((const char **)ALUT, &contents, 256);    
   return(1);
}

/**
    \brief Get data from a U_PMF_IE_COLORMATRIX object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Matrix     5 x 5 color transformation matrix, First 4 rows are [{4 multiplier values},0.0] for R,G,B,A, last Row is [{4 color translation valuess}, 1.0]
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.3.6, Microsoft name: ColorMatrixEffect Object
*/
int U_PMF_IE_COLORMATRIX_get(const char *contents, U_PMF_IE_COLORMATRIX *Matrix, const char *blimit){
   if(!contents || !Matrix || !blimit){ return(0); }
   /* Important!  This only checks the constant part, the caller must check that returned data doesn't exceed blimit */
   if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_IE_COLORMATRIX), blimit))return(0);
   U_PMF_SERIAL_get(&contents, Matrix, 4, 5*5, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMF_IE_HUESATURATIONLIGHTNESS object
    \return 1 on success, 0 on error
    \param  contents     Record from which to extract data
    \param  Hue          -180 to 180, 0 is unchanged
    \param  Saturation   -100 to 100, 0 is unchanged
    \param  Lightness    -100 to 100, 0 is unchanged
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.3.7, Microsoft name: HueSaturationLightnessEffect Object
*/
int U_PMF_IE_HUESATURATIONLIGHTNESS_get(const char *contents, int32_t *Hue, int32_t *Saturation, int32_t *Lightness, const char *blimit){
   if(!contents || !Hue || !Saturation || !Lightness || !blimit){ return(0); }
   if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_IE_HUESATURATIONLIGHTNESS), blimit))return(0);
   U_PMF_SERIAL_get(&contents, Hue,        4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Saturation, 4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Lightness,  4, 1, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMF_IE_LEVELS object
    \return 1 on success, 0 on error
    \param  contents     Record from which to extract data
    \param  Highlight    0 to 100, 100 is unchanged
    \param  Midtone      -100 to 100, 0 is unchanged
    \param  Shadow       0 to 100, 0 is unchanged
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.3.8, Microsoft name: LevelsEffect Object
*/
int U_PMF_IE_LEVELS_get(const char *contents, int32_t *Highlight, int32_t *Midtone, int32_t *Shadow, const char *blimit){
   if(!contents || !Highlight || !Midtone || !Shadow || !blimit){ return(0); }
   if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_IE_LEVELS), blimit))return(0);
   U_PMF_SERIAL_get(&contents, Highlight, 4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Midtone,   4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Shadow,    4, 1, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMF_IE_REDEYECORRECTION object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Elements   Number of members in Rects
    \param  Rects      Caller must free.  Pointer to memory holding an array of U_RECTL.
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.3.9, Microsoft name: RedEyeCorrectionEffect Object
*/
int U_PMF_IE_REDEYECORRECTION_get(const char *contents, int32_t *Elements, U_RECTL **Rects, const char *blimit){
   if(!contents || !Elements || !Rects || !blimit){ return(0); }
   if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_IE_REDEYECORRECTION), blimit))return(0);
   U_PMF_SERIAL_get(&contents, Elements, 4, 1, U_LE);
   if(IS_MEM_UNSAFE(contents, *Elements * 4, blimit))return(0);
   *Rects = (U_RECTL *) malloc(*Elements * sizeof(U_RECTL));
   if(!*Rects){ return(0); }
   U_PMF_SERIAL_get(&contents, *Rects, 4, *Elements * 4, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMF_IE_SHARPEN object
    \return 1 on success, 0 on error
    \param  contents   Record from which to extract data
    \param  Radius     Sharpening radius in pixels
    \param  Sharpen    0 to 100, 0 is unchanged
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.3.10, Microsoft name: SharpenEffect Object
*/
int U_PMF_IE_SHARPEN_get(const char *contents, U_FLOAT *Radius, int32_t *Sharpen, const char *blimit){
   if(!contents || !Radius || !Sharpen || !blimit){ return(0); }
   if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_IE_SHARPEN), blimit))return(0);
   U_PMF_SERIAL_get(&contents, Radius,  4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Sharpen, 4, 1, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMF_IE_TINT object
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Hue         -180 to 180, [positive==clockwise] rotation in degrees starting from blue
    \param  Amount      -100 [add black] to 100[add white], 0 is unchanged.  Change in hue on specified axis
    \param  blimit     one byte past the end of data

    EMF+ manual 2.2.3.11, Microsoft name: TintEffect Object
*/
int U_PMF_IE_TINT_get(const char *contents, int32_t *Hue, int32_t *Amount, const char *blimit){
   if(!contents || !Hue || !Amount || !blimit){ return(0); }
   if(IS_MEM_UNSAFE(contents, sizeof(U_PMF_IE_TINT), blimit))return(0);
   U_PMF_SERIAL_get(&contents, Hue,    4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Amount, 4, 1, U_LE);
   return(1);
}

/*

   end of U_PMF_*_get() functions 
   =====================================================================================
   start of U_PMR_*_get() functions 

      These functions all assume that the size field in the common EMF+ header has already
      been checked, so that the extent the record claims exists in the data read in for the file.
      Consequently none of them takes a blimit parameter.  They generate a new one from the
      header size field and contents if needed.

*/

int U_PMR_common_stack_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *StackID){
   if(!contents || !StackID){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_RESTORE))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   U_PMF_SERIAL_get(&contents, StackID, 4, 1, U_LE);
   return(1);
}

/* for records that have a type but no associated flag bits or data */
int U_PMR_common_header_get(const char *contents, U_PMF_CMN_HDR *Header){
   /* memory access safe, only uses the common header */
   if(!contents){ return(0); }
   U_PMF_CMN_HDR_get(&contents, Header);
   return(1);
}

/**
    \brief Get data from a U_PMR_OFFSETCLIP record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header (ignore flags)
    \param  dX          horizontal translation offset to apply to clipping region
    \param  dY          vertical   translation offset to apply to clipping region

    EMF+ manual 2.3.1.1, Microsoft name: EmfPlusOffsetClip Record,  Index 0x35
*/
int U_PMR_OFFSETCLIP_get(const char *contents, U_PMF_CMN_HDR *Header,
      U_FLOAT *dX, U_FLOAT *dY){
   if(!contents){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_OFFSETCLIP))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   U_PMF_SERIAL_get(&contents, dX, 4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, dY, 4, 1, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMR_RESETCLIP record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header (ignore flags)

    EMF+ manual 2.3.1.2, Microsoft name: EmfPlusResetClip Record, Index 0x31
*/
int U_PMR_RESETCLIP_get(const char *contents, U_PMF_CMN_HDR *Header){
   if(!contents){ return(0); }
   U_PMF_CMN_HDR_get(&contents, Header);
   return(1);
}

/**
    \brief Get data from a U_PMR_SETCLIPPATH record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  CMenum      CombineMode enumeration..
    \param  PathID      U_PMF_PATH object in the EMF+ object table (0-63, inclusive)

    EMF+ manual 2.3.1.3, Microsoft name: EmfPlusSetClipPath Record, Index 0x33
*/
int U_PMR_SETCLIPPATH_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint32_t *PathID, int *CMenum){
   if(!contents || !PathID || !CMenum){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_SETCLIPPATH))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *CMenum  = (lclHeader.Flags >> U_FF_SHFT_CM4) & U_FF_MASK_CM4;
   *PathID  = (lclHeader.Flags >> U_FF_SHFT_OID8) & U_FF_MASK_OID8;
   return(1);
}

/**
    \brief Get data from a U_PMR_SETCLIPRECT record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header (ignore flags)
    \param  CMenum      Combine mode enumeration.
    \param  Rect        Rectangle used with CombineMode enumeration from Header.Flags

    EMF+ manual 2.3.1.4, Microsoft name: EmfPlusSetClipRect Record, Index 0x32
*/
int U_PMR_SETCLIPRECT_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *CMenum,
      U_PMF_RECTF *Rect){
   if(!contents || !CMenum || !Rect ){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_SETCLIPRECT))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *CMenum  = (lclHeader.Flags >> U_FF_SHFT_CM4) & U_FF_MASK_CM4;
   U_PMF_SERIAL_get(&contents, Rect, 4, 4, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMR_SETCLIPREGION record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  CMenum      CombineMode enumeration..
    \param  PathID      U_PMF_PATH object in the EMF+ object table (0-63, inclusive)

    EMF+ manual 2.3.1.5, Microsoft name: EmfPlusSetClipRegion Record, Index 0x34
*/
int U_PMR_SETCLIPREGION_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint32_t *PathID, int *CMenum){
   if(!contents || !PathID || !CMenum){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_SETCLIPREGION))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *CMenum  = (lclHeader.Flags >> U_FF_SHFT_CM4) & U_FF_MASK_CM4;
   *PathID  = (lclHeader.Flags >> U_FF_SHFT_OID8) & U_FF_MASK_OID8;
   return(1);
}

/**
    \brief Get data from a U_PMR_COMMENT record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header (ignore flags)
    \param  Data        Private data, may be anything

    EMF+ manual 2.3.2.1, Microsoft name: EmfPlusComment Record, Index 0x03
    
    Caller must check Data for possible memory access violations.
*/
int U_PMR_COMMENT_get(const char *contents, U_PMF_CMN_HDR *Header,
      const char **Data){
   if(!contents || !Data){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_COMMENT))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   U_PMF_PTRSAV_SHIFT(Data, &contents, 0);
   return(1);
}

/**
    \brief Get data from a U_PMR_ENDOFFILE record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header  (ignore flags)

    EMF+ manual 2.3.3.1, Microsoft name: EmfPlusEndOfFile Record, Index 0x02
*/
int U_PMR_ENDOFFILE_get(const char *contents, U_PMF_CMN_HDR *Header){
   if(!contents){ return(0); }
   U_PMF_CMN_HDR_get(&contents, Header);
   return(1);
}

/**
    \brief Get data from a U_PMR_GETDC record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header  (ignore flags)

    EMF+ manual 2.3.3.2, Microsoft name: EmfPlusGetDC Record, Index 0x04
*/
int U_PMR_GETDC_get(const char *contents, U_PMF_CMN_HDR *Header){
   if(!contents){ return(0); }
   U_PMF_CMN_HDR_get(&contents, Header);
   return(1);
}

/**
    \brief Get data from a U_PMR_HEADER record
    \return 1 on success, 0 on error
    \param  contents     Record from which to extract data
    \param  Header       Common header  (ignore flags)
    \param  Version      EmfPlusGraphicsVersion object
    \param  IsDual       set = Dual-mode file, clear= EMF+ only file.
    \param  IsVideo      set = video device, clear= printer.  Ignore all other bits.
    \param  LogicalDpiX  Horizontal resolution reference device in DPI
    \param  LogicalDpiY  Vertical   resolution reference device in DPI

    EMF+ manual 2.3.3.3, Microsoft name: EmfPlusHeader Record, Index 0x01
*/
int U_PMR_HEADER_get(const char *contents, U_PMF_CMN_HDR *Header,
      U_PMF_GRAPHICSVERSION *Version, int *IsDual, int *IsVideo, uint32_t *LogicalDpiX, uint32_t *LogicalDpiY){
   if(!contents || !Version || !IsDual || !IsVideo || !LogicalDpiX || !LogicalDpiY){ return(0); }
   uint32_t tmp;

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_HEADER))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *IsDual = (lclHeader.Flags & U_PPF_DM ? 1 : 0 );
   U_PMF_SERIAL_get(&contents, Version,     4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, &tmp,        4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, LogicalDpiX, 4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, LogicalDpiY, 4, 1, U_LE);
   *IsVideo = (tmp & U_PPF_VIDEO ? 1 : 0 );
   return(1);
}

/**
    \brief Get data from a U_PMR_CLEAR record
    \return 1 on success, 0 on error
    \param  contents     Record from which to extract data
    \param  Header       Common header  (ignore flags)
    \param  Color        Erase everything preceding, set background ARGB color.

    EMF+ manual 2.3.4.1, Microsoft name: EmfPlusClear Record, Index 0x09
*/
int U_PMR_CLEAR_get(const char *contents, U_PMF_CMN_HDR *Header,
      U_PMF_ARGB *Color){
   if(!contents || !Color){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_CLEAR))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   U_PMF_SERIAL_get(&contents, Color, 4, 1, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMR_DRAWARC record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
    \param  ctype       Set: int16_t coordinates; Clear: U_FLOAT coordinates
    \param  Start       Start angle, >=0.0, degrees clockwise from 3:00
    \param  Sweep       Sweep angle, -360<= angle <=360, degrees clockwise from Start
    \param  Rect        Caller must free.  Bounding rectangle.  Coordinate type set by ctype.

    EMF+ manual 2.3.4.2, Microsoft name: EmfPlusDrawArc Record, Index 0x12
*/
int U_PMR_DRAWARC_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint32_t *PenID, int *ctype, 
      U_FLOAT *Start, U_FLOAT *Sweep,
      U_PMF_RECTF *Rect){
   if(!contents || !PenID || !ctype || !Start || !Sweep || !Rect){ return(0); }

   const char *blimit = contents;
   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_DRAWARC))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }
   blimit += lclHeader.Size;

   *ctype = (lclHeader.Flags & U_PPF_C ? 1 : 0 );
   *PenID = (lclHeader.Flags >> U_FF_SHFT_OID8) & U_FF_MASK_OID8;
   U_PMF_SERIAL_get(&contents, Start, 4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Sweep, 4, 1, U_LE);
   U_PMF_RECTF *Rects = NULL;
   if(!U_PMF_VARRECTS_get(&contents, lclHeader.Flags, 1, &Rects, blimit))return(0);
   memcpy(Rect,Rects,sizeof(U_PMF_RECTF));
   free(Rects);
   return(1);
}


/**
    \brief Get data from a U_PMR_DRAWBEZIERS record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
    \param  ctype       Set: int16_t coordinates; Clear: U_FLOAT coordinates
    \param  RelAbs      Set: Coordinates are relative; Clear: Coordinates are absolute and their type is set by ctype
    \param  Elements    Number of members in the Data array
    \param  Points      Caller must free.  Array of U_POINT_F = Sequence of points to connect.  Coordinate type set by ctype and RelAbs.

    EMF+ manual 2.3.4.3, Microsoft name: EmfPlusDrawBeziers Record, Index 0x19
*/
int U_PMR_DRAWBEZIERS_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint32_t *PenID, int *ctype, int *RelAbs, 
      uint32_t *Elements, 
      U_PMF_POINTF **Points){
   if(!contents || !PenID || !ctype || !RelAbs || !Elements || !Points){ return(0); }

   const char *blimit = contents;
   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_DRAWBEZIERS))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }
   blimit += lclHeader.Size;

   *ctype  = (lclHeader.Flags & U_PPF_C ? 1 : 0 );
   *RelAbs = (lclHeader.Flags & U_PPF_P ? 1 : 0 );
   *PenID  = (lclHeader.Flags >> U_FF_SHFT_OID8) & U_FF_MASK_OID8;
   U_PMF_SERIAL_get(&contents, Elements, 4, 1, U_LE);
   int status = U_PMF_VARPOINTS_get(contents, lclHeader.Flags, *Elements, Points, blimit );
   return(status);
}

/**
    \brief Get data from a U_PMR_DRAWCLOSEDCURVE record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
    \param  ctype       Set: int16_t coordinates; Clear: U_FLOAT coordinates
    \param  RelAbs      Set: Coordinates are relative; Clear: Coordinates are absolute and their type is set by ctype
    \param  Tension     Controls splines, 0 is straight line, >0 is curved
    \param  Elements    Number of members in the Data array
    \param  Points      Caller must free.  Array of U_POINT_F = Sequence of points to connect.  Coordinate type set by ctype and RelAbs.

    EMF+ manual 2.3.4.4, Microsoft name: EmfPlusDrawClosedCurve Record, Index 0x17
*/
int U_PMR_DRAWCLOSEDCURVE_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint32_t *PenID, int *ctype, int *RelAbs,
      U_FLOAT *Tension, uint32_t *Elements,
      U_PMF_POINTF **Points){
   if(!contents || !PenID || !ctype || !RelAbs || !Tension || !Elements || !Points){ return(0); }

   const char *blimit = contents;
   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_DRAWCLOSEDCURVE))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }
   blimit += lclHeader.Size;

   *ctype  = (lclHeader.Flags & U_PPF_C ? 1 : 0 );
   *RelAbs = (lclHeader.Flags & U_PPF_P ? 1 : 0 );
   *PenID  = (lclHeader.Flags >> U_FF_SHFT_OID8) & U_FF_MASK_OID8;
   U_PMF_SERIAL_get(&contents, Tension,  4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Elements, 4, 1, U_LE);
   U_PMF_VARPOINTS_get(contents, lclHeader.Flags, *Elements, Points, blimit);
   return(1);
}

/**
    \brief Get data from a U_PMR_DRAWCURVE record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
    \param  ctype       Set: int16_t coordinates; Clear: U_FLOAT coordinates
    \param  Tension     Controls splines, 0 is straight line, >0 is curved
    \param  Offset      Element in Points that is the spline's starting point
    \param  NSegs       Number of segments
    \param  Elements    Number of members in Data array
    \param  Points      Caller must free.  Array of U_POINT_F = Sequence of points to connect.  Coordinate type set by ctype and RelAbs.

    EMF+ manual 2.3.4.5, Microsoft name: EmfPlusDrawCurve Record, Index 0x18
*/
int U_PMR_DRAWCURVE_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint32_t *PenID, int *ctype,
      U_FLOAT *Tension, uint32_t *Offset, uint32_t *NSegs, uint32_t *Elements,
      U_PMF_POINTF **Points){
   if(!contents || !PenID || !ctype || !Tension || !Offset || !NSegs || !Elements || !Points){ return(0); }

   const char *blimit = contents;
   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_DRAWCURVE))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }
   blimit += lclHeader.Size;

   *ctype  = (lclHeader.Flags & U_PPF_C ? 1 : 0 );
   *PenID  = (lclHeader.Flags >> U_FF_SHFT_OID8) & U_FF_MASK_OID8;
   U_PMF_SERIAL_get(&contents, Tension,  4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Offset,   4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, NSegs,    4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Elements, 4, 1, U_LE);
   U_PMF_VARPOINTS_get(contents, lclHeader.Flags, *Elements, Points, blimit);
   return(1);
}

/**
    \brief Get data from a U_PMR_DRAWDRIVERSTRING record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  FontID      U_PMF_FONT object in the EMF+ object table (0-63, inclusive)
    \param  btype       Set: BrushID is an U_PFM_ARGB; Clear: index of U_PMF_BRUSH object in EMF+ object table.
    \param  BrushID     Color or index of  U_PMF_BRUSH object in the EMF+ object table, depends on Flags bit0
    \param  DSOFlags    DriverStringOptions flags
    \param  HasMatrix   If 1 record contains a TransformMatrix field, if 0 it does not.
    \param  Elements    Number of members in Glyphs and Positions array
    \param  Glyphs      Caller must free.  If U_DSO_CmapLookup is set in DSOFlags this is an array of UTF16LE characters, otherwise, it is an array of indices into the U_PMF_FONT object indexed by Object_ID in flags.
    \param  Points      Caller must free.  Coordinates of each member of Glyphs.  U_DSO_RealizedAdvance set in DSOFlags Relative then positions are calculated relative to the first glyph which is stored in Positions, otherwise, all glyph positions are stored in Positions.
    \param  Matrix      Caller must free.  Transformation to apply to Glyphs & Positions. Present if HasMatrix is 1

    EMF+ manual 2.3.4.6, Microsoft name: EmfPlusDrawDriverString Record, Index 0x36
*/
int U_PMR_DRAWDRIVERSTRING_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint32_t *FontID, int *btype,
      uint32_t *BrushID, uint32_t *DSOFlags, uint32_t *HasMatrix, uint32_t *Elements,
      uint16_t **Glyphs, U_PMF_POINTF **Points, U_PMF_TRANSFORMMATRIX **Matrix){
   if(!contents || !FontID || !btype || !BrushID || 
      !DSOFlags || !HasMatrix || !Elements || !Glyphs || !Points || !Matrix){ return(0); }

   const char *blimit = contents;
   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_DRAWDRIVERSTRING))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }
   blimit += lclHeader.Size;

   *btype  = (lclHeader.Flags & U_PPF_B ? 1 : 0 );
   *FontID = (lclHeader.Flags >> U_FF_SHFT_OID8) & U_FF_MASK_OID8;
   U_PMF_SERIAL_get(&contents, BrushID,   4, 1, (*btype ? U_XE : U_LE)); /* color is not byte swapped, ID integer is */
   U_PMF_SERIAL_get(&contents, DSOFlags,  4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, HasMatrix, 4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Elements,  4, 1, U_LE);
   if(IS_MEM_UNSAFE(contents, *Elements*2 + *Elements*2*4 + 24, blimit))return(0);
   if(!U_PMF_SERIAL_array_copy_get(&contents, (void **)Glyphs, 2, *Elements,    U_LE, (*DSOFlags & U_DSO_CmapLookup))){      return(0); }
   if(!U_PMF_SERIAL_array_copy_get(&contents, (void **)Points, 4, *Elements *2, U_LE, (*DSOFlags & U_DSO_RealizedAdvance))){ return(0); }
   if(!U_PMF_SERIAL_array_copy_get(&contents, (void **)Matrix, 4, 6,            U_LE, (*HasMatrix))){                        return(0); }
   return(1);
}

/**
    \brief Get data from a U_PMR_DRAWELLIPSE record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
    \param  ctype       Set: int16_t coordinates; Clear: U_FLOAT coordinates
    \param  Rect        Caller must free.  Bounding rectangle.  Coordinate type set by ctype.

    EMF+ manual 2.3.4.7, Microsoft name: EmfPlusDrawEllipse Record, Index 0x0F
*/
int U_PMR_DRAWELLIPSE_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint32_t *PenID, int *ctype, 
      U_PMF_RECTF *Rect){
   if(!contents || !PenID || !ctype || !Rect){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_DRAWELLIPSE))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *ctype  = (lclHeader.Flags & U_PPF_C ? 1 : 0 );
   *PenID  = (lclHeader.Flags >> U_FF_SHFT_OID8) & U_FF_MASK_OID8;
   U_PMF_SERIAL_get(&contents, Rect,  4, 4, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMR_DRAWIMAGE record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  ImgID       U_PMF_IMAGE object in the EMF+ object table (0-63, inclusive)
    \param  ctype       Set: int16_t coordinates; Clear: U_FLOAT coordinates
    \param  ImgAttrID   index of a U_PMF_IMAGEATTRIBUTES object in the object table
    \param  SrcUnit     UnitType enumeration
    \param  SrcRect     Region of image 
    \param  DstRect     Destination rectangle for image.  Coordinate type set by ctype.

    EMF+ manual 2.3.4.8, Microsoft name: EmfPlusDrawImage Record, Index 0x1A
*/
int U_PMR_DRAWIMAGE_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint32_t *ImgID, int *ctype,
      uint32_t *ImgAttrID, int32_t *SrcUnit, U_PMF_RECTF *SrcRect,
      U_PMF_RECTF *DstRect){
   if(!contents || !ImgID || !ctype || !ImgAttrID || !SrcUnit || !SrcRect || !DstRect){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_DRAWIMAGE))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *ctype  = (lclHeader.Flags & U_PPF_C ? 1 : 0 );
   *ImgID  = (lclHeader.Flags >> U_FF_SHFT_OID8) & U_FF_MASK_OID8;
   U_PMF_SERIAL_get(&contents, ImgAttrID, 4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, SrcUnit,   4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, SrcRect,   4, 4, U_LE);
   U_PMF_SERIAL_get(&contents, DstRect,   4, 4, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMR_DRAWIMAGEPOINTS record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  ImgID       U_PMF_IMAGE object in the EMF+ object table (0-63, inclusive)
    \param  ctype       Set: int16_t coordinates; Clear: U_FLOAT coordinates
    \param  etype       Set: effect from previous U_PMR_SERIALIZABLEOBJECT record will be applied; Clear:  no effect applied
    \param  RelAbs      Set: Data is relative, Clear: if it is absolute
    \param  ImgAttrID   EmfPlusImageAttributes object 
    \param  SrcUnit     UnitType enumeration
    \param  SrcRect     Region of image 
    \param  Elements    Number of members in Points, must be 3
    \param  Points      Caller must free.  3 points of a parallelogram..  Coordinate type set by ctype and RelAbs.

    EMF+ manual 2.3.4.9, Microsoft name: EmfPlusDrawImagePoints Record, Index 0x1B
*/
int U_PMR_DRAWIMAGEPOINTS_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint32_t *ImgID, int *ctype, int *etype, int *RelAbs,
      uint32_t *ImgAttrID, int32_t *SrcUnit, U_PMF_RECTF *SrcRect, uint32_t *Elements, 
      U_PMF_POINTF **Points){
   if(!contents || !ImgID || !ctype || !etype || !RelAbs || !ImgAttrID || !SrcUnit || !Elements || !Points){ return(0); }

   const char *blimit = contents;
   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_DRAWIMAGEPOINTS))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }
   blimit += lclHeader.Size;

   *ctype  = (lclHeader.Flags & U_PPF_C ? 1 : 0 );
   *etype  = (lclHeader.Flags & U_PPF_E ? 1 : 0 );
   *RelAbs = (lclHeader.Flags & U_PPF_P ? 1 : 0 );
   *ImgID  = (lclHeader.Flags >> U_FF_SHFT_OID8) & U_FF_MASK_OID8;
   U_PMF_SERIAL_get(&contents, ImgAttrID, 4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, SrcUnit,   4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, SrcRect,   4, 4, U_LE);
   U_PMF_SERIAL_get(&contents, Elements,  4, 1, U_LE);
   U_PMF_VARPOINTS_get(contents, lclHeader.Flags, *Elements, Points, blimit);
   return(1);
}

/**
    \brief Get data from a U_PMR_DRAWLINES record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
    \param  ctype       Set: int16_t coordinates; Clear: U_FLOAT coordinates
    \param  dtype       Set: path must be closed, Clear: path is open
    \param  RelAbs      Set: Coordinates are relative; Clear: Coordinates are absolute and their type is set by ctype
    \param  Elements    Number of members in Points
    \param  Points      Caller must free.  Array of U_POINT_F = Sequence of points to connect.  Coordinate type set by ctype and RelAbs.

    EMF+ manual 2.3.4.10, Microsoft name: EmfPlusDrawLines Record, Index 0x0D
*/
int U_PMR_DRAWLINES_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint32_t *PenID, int *ctype, int *dtype, int *RelAbs,
      uint32_t *Elements,
      U_PMF_POINTF **Points){
   if(!contents || !PenID || !ctype || !dtype || !RelAbs || !Elements || !Points){ return(0); }

   const char *blimit = contents;
   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_DRAWLINES))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }
   blimit += lclHeader.Size;

   *ctype  = (lclHeader.Flags & U_PPF_C ? 1 : 0 );
   *dtype  = (lclHeader.Flags & U_PPF_D ? 1 : 0 );
   *RelAbs = (lclHeader.Flags & U_PPF_P ? 1 : 0 );
   *PenID  = (lclHeader.Flags >> U_FF_SHFT_OID8) & U_FF_MASK_OID8;
   U_PMF_SERIAL_get(&contents, Elements, 4, 1, U_LE);
   U_PMF_VARPOINTS_get(contents, lclHeader.Flags, *Elements, Points, blimit);
   return(1);
}

/**
    \brief Get data from a U_PMR_DRAWPATH record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  PathID      U_PMF_PATH object in the EMF+ object table (0-63, inclusive)
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)

    EMF+ manual 2.3.4.11, Microsoft name: EmfPlusDrawPath Record, Index 0x15
*/
int U_PMR_DRAWPATH_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint32_t *PathID, uint32_t *PenID){
   if(!contents || !PathID || !PenID){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_DRAWPATH))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *PathID  = (lclHeader.Flags >> U_FF_SHFT_OID8) & U_FF_MASK_OID8;
   U_PMF_SERIAL_get(&contents, PenID, 4, 1, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMR_DRAWPIE record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
    \param  ctype       Set: int16_t coordinates; Clear: U_FLOAT coordinates
    \param  Start       Start angle, >=0.0, degrees clockwise from 3:00
    \param  Sweep       Sweep angle, -360<= angle <=360, degrees clockwise from Start
    \param  Rect        Caller must free.  Bounding rectangle.  Coordinate type set by ctype.

    EMF+ manual 2.3.4.12, Microsoft name: EmfPlusDrawPie Record, Index 0x0D
*/
int U_PMR_DRAWPIE_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint32_t *PenID, int *ctype,
      U_FLOAT *Start, U_FLOAT *Sweep,
      U_PMF_RECTF *Rect){
   if(!contents || !PenID || !ctype || !Start || !Sweep || !Rect){ return(0); }

   const char *blimit = contents;
   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_DRAWPIE))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }
   blimit += lclHeader.Size;

   *ctype = (lclHeader.Flags & U_PPF_C ? 1 : 0 );
   *PenID = (lclHeader.Flags >> U_FF_SHFT_OID8) & U_FF_MASK_OID8;
   U_PMF_SERIAL_get(&contents, Start, 4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Sweep, 4, 1, U_LE);
   U_PMF_RECTF *Rects = NULL;
   if(!U_PMF_VARRECTS_get(&contents, lclHeader.Flags, 1, &Rects, blimit))return(0);
   memcpy(Rect,Rects,sizeof(U_PMF_RECTF));
   free(Rects);
   return(1);
}

/**
    \brief Get data from a U_PMR_DRAWRECTS record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  PenID       U_PMF_PEN object in the EMF+ object table (0-63, inclusive)
    \param  ctype       Set: int16_t coordinates; Clear: U_FLOAT coordinates
    \param  Elements    Number of members in Rects
    \param  Rects       Caller must free.  Array of U_PMF_RECTF rectangles to draw.

    EMF+ manual 2.3.4.13, Microsoft name: EmfPlusDrawRects Record, Index 0x0B
    
    Rects in record may be either U_PMF_RECT or U_PMF_RECTF, but this function always
    returns U_PMF_RECTF
*/
int U_PMR_DRAWRECTS_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint32_t *PenID, int *ctype,
      uint32_t *Elements,
      U_PMF_RECTF **Rects){
   if(!contents || !PenID || !Elements || !Rects){ return(0); }

   const char *blimit = contents;
   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_DRAWPIE))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }
   blimit += lclHeader.Size;

   *PenID =  (lclHeader.Flags >> U_FF_SHFT_OID8) & U_FF_MASK_OID8;
   *ctype  = (lclHeader.Flags & U_PPF_C ? 1 : 0 );
   U_PMF_SERIAL_get(&contents, Elements, 4, 1, U_LE);
   U_PMF_VARRECTS_get(&contents, lclHeader.Flags, *Elements, Rects, blimit);
   return(1);
}

/**
    \brief Get data from a U_PMR_DRAWSTRING record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  FontID      U_PMF_FONT object in the EMF+ object table (0-63, inclusive)
    \param  btype       Set: BrushID is an U_PFM_ARGB; Clear: index of U_PMF_BRUSH object in EMF+ object table.
    \param  BrushID     Color or index of  U_PMF_BRUSH object in the EMF+ object table, depending on btype.
    \param  FormatID    U_PMF_STRINGFORMAT object in EMF+ Object Table.
    \param  Elements    Number of characters in the string.
    \param  Rect        String's bounding box.
    \param  String      Caller must free.  Array of UFT-16LE unicode characters. 

    EMF+ manual 2.3.4.14, Microsoft name: EmfPlusDrawString Record, Index 0x1C
*/
int U_PMR_DRAWSTRING_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint32_t *FontID, int *btype,
      uint32_t *BrushID, uint32_t *FormatID, uint32_t *Elements, U_PMF_RECTF *Rect,
      uint16_t **String){
   if(!contents || !FontID || !btype || !BrushID || !FormatID || !Elements || !String){ return(0); }

   const char *blimit = contents;
   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_DRAWPIE))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }
   blimit += lclHeader.Size;

   *btype  = (lclHeader.Flags & U_PPF_B ? 1 : 0 );
   *FontID = (lclHeader.Flags >> U_FF_SHFT_OID8) & U_FF_MASK_OID8;
   U_PMF_SERIAL_get(&contents, BrushID,  4, 1, (*btype ? U_XE : U_LE)); /* color is not byte swapped, ID integer is */
   U_PMF_SERIAL_get(&contents, FormatID, 4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Elements, 4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Rect,     4, 4, U_LE);
   if(IS_MEM_UNSAFE(contents, *Elements * 2, blimit))return(0);
   if(!U_PMF_SERIAL_array_copy_get(&contents, (void **)String, 2, *Elements, U_XE, 1)){ return(0); }
   return(1);
}

/**
    \brief Get data from a U_PMR_FILLCLOSEDCURVE record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  btype       Set: BrushID is an U_PFM_ARGB; Clear: is index of U_PMF_BRUSH object in EMF+ object table.
    \param  ctype       Set: int16_t coordinates; Clear: U_FLOAT coordinates
    \param  ftype       Set: winding fill; Clear: alternate fill
    \param  RelAbs      Set: Coordinates are relative; Clear: Coordinates are absolute and their type is set by ctype
    \param  BrushID     Color or index of  U_PMF_BRUSH object in the EMF+ object table, depending on btype.
    \param  Tension     Controls splines, 0 is straight line, >0 is curved
    \param  Elements    Number of members in Points
    \param  Points      Caller must free.  Array of U_POINT_F = Sequence of points to connect.  Coordinate type set by ctype and RelAbs.

    EMF+ manual 2.3.4.15, Microsoft name: EmfPlusFillClosedCurve Record, Index 0x16
*/
int U_PMR_FILLCLOSEDCURVE_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *btype, int *ctype, int *ftype, int *RelAbs,
      uint32_t *BrushID, U_FLOAT *Tension, uint32_t *Elements,
      U_PMF_POINTF **Points){
   if(!contents || !btype || !ctype || !ftype || !RelAbs || !BrushID || !Tension || !Elements || !Points){ return(0); }

   const char *blimit = contents;
   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_DRAWLINES))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }
   blimit += lclHeader.Size;

   *btype  = (lclHeader.Flags & U_PPF_B ? 1 : 0 );
   *ctype  = (lclHeader.Flags & U_PPF_C ? 1 : 0 );
   *ftype  = (lclHeader.Flags & U_PPF_F ? 1 : 0 );
   *RelAbs = (lclHeader.Flags & U_PPF_P ? 1 : 0 );
   U_PMF_SERIAL_get(&contents, BrushID,  4, 1, (*btype ? U_XE : U_LE)); /* color is not byte swapped, ID integer is */
   U_PMF_SERIAL_get(&contents, Tension,  4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Elements, 4, 1, U_LE);
   U_PMF_VARPOINTS_get(contents, lclHeader.Flags, *Elements, Points, blimit);
   return(1);
}

/**
    \brief Get data from a U_PMR_FILLELLIPSE record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  btype       Set: BrushID is an U_PFM_ARGB; Clear: is index of U_PMF_BRUSH object in EMF+ object table.
    \param  ctype       Set: int16_t coordinates; Clear: U_FLOAT coordinates
    \param  BrushID     Color or index of  U_PMF_BRUSH object in the EMF+ object table, depending on btype.
    \param  Rect        Caller must free.  Bounding box for elliptical pie segment being drawn.  Coordinate type set by ctype.

    EMF+ manual 2.3.4.16, Microsoft name: EmfPlusFillEllipse Record, Index 0x0E
*/
int U_PMR_FILLELLIPSE_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *btype, int *ctype,
      uint32_t *BrushID,
      U_PMF_RECTF *Rect){
   if(!contents || !btype || !ctype || !BrushID || !Rect){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_FILLELLIPSE))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *btype  = (lclHeader.Flags & U_PPF_B ? 1 : 0 );
   *ctype  = (lclHeader.Flags & U_PPF_C ? 1 : 0 );
   U_PMF_SERIAL_get(&contents, BrushID, 4, 1, (*btype ? U_XE : U_LE)); /* color is not byte swapped, ID integer is */
   U_PMF_SERIAL_get(&contents, Rect,    4, 4, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMR_FILLPATH record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  btype       Set: BrushID is an U_PFM_ARGB; Clear: is index of U_PMF_BRUSH object in EMF+ object table.
    \param  PathID      U_PMF_PATH object in the EMF+ object table (0-63, inclusive)
    \param  BrushID     Color or index of  U_PMF_BRUSH object in the EMF+ object table, depending on btype.

    EMF+ manual 2.3.4.17, Microsoft name: EmfPlusFillPath Record, Index 0x14 

    Note: U_PMF_FILLPATHOBJ is the object, U_PMF_FILLPATH is the file record
*/
int U_PMR_FILLPATH_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint32_t *PathID, int *btype,
      uint32_t *BrushID){
   if(!contents || !PathID || !btype || !BrushID){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_FILLPATH))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *btype  = (lclHeader.Flags & U_PPF_B ? 1 : 0 );
   *PathID = (lclHeader.Flags >> U_FF_SHFT_OID8) & U_FF_MASK_OID8;
   U_PMF_SERIAL_get(&contents, BrushID, 4, 1, (*btype ? U_XE : U_LE)); /* color is not byte swapped, ID integer is */
   return(1);
}

/**
    \brief Get data from a U_PMR_FILLPIE record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  btype       Set: BrushID is an U_PFM_ARGB; Clear: is index of U_PMF_BRUSH object in EMF+ object table.
    \param  ctype       Set: int16_t coordinates; Clear: U_FLOAT coordinates
    \param  BrushID     Color or index of  U_PMF_BRUSH object in the EMF+ object table, depending on btype.
    \param  Start       Start angle, >=0.0, degrees clockwise from 3:00
    \param  Sweep       Sweep angle, -360<= angle <=360, degrees clockwise from Start
    \param  Rect        Bounding box for elliptical pie segment being filled.  Coordinate type set by ctype.

    EMF+ manual 2.3.4.18, Microsoft name: EmfPlusFillPie Record, Index 0x10
*/
int U_PMR_FILLPIE_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *btype, int *ctype,
      uint32_t *BrushID, U_FLOAT *Start, U_FLOAT *Sweep,
      U_PMF_RECTF *Rect){
   if(!contents || !btype || !ctype || !BrushID || !Start || !Sweep || !Rect){ return(0); }

   const char *blimit = contents;
   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_FILLPIE))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }
   blimit += lclHeader.Size;

   *btype  = (lclHeader.Flags & U_PPF_B ? 1 : 0 );
   *ctype  = (lclHeader.Flags & U_PPF_C ? 1 : 0 );
   U_PMF_SERIAL_get(&contents, BrushID, 4, 1, (*btype ? U_XE : U_LE)); /* color is not byte swapped, ID integer is */
   U_PMF_SERIAL_get(&contents, Start,   4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Sweep,   4, 1, U_LE);
   U_PMF_RECTF *Rects = NULL;
   if(!U_PMF_VARRECTS_get(&contents, lclHeader.Flags, 1, &Rects, blimit))return(0);
   memcpy(Rect,Rects,sizeof(U_PMF_RECTF));
   free(Rects);
   return(1);
}

/**
    \brief Get data from a U_PMR_FILLPOLYGON record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  btype       Set: BrushID is an U_PFM_ARGB; Clear: is index of U_PMF_BRUSH object in EMF+ object table.
    \param  ctype       Set: int16_t coordinates; Clear: U_FLOAT coordinates
    \param  RelAbs      Set: U_PMF_PathPointTypeRLE and/or U_PMF_PathPointType objects; Clear: only U_PMF_PathPointType
    \param  BrushID     Color or index of  U_PMF_BRUSH object in the EMF+ object table, depending on btype.
    \param  Elements    Number of members in Data.
    \param  Points      Sequence of points to connect with line segments.  Coordinate type set by ctype and RelAbs.

    EMF+ manual 2.3.4.19, Microsoft name: EmfPlusFillPolygon Record, Index 0x0C
*/
int U_PMR_FILLPOLYGON_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *btype, int *ctype, int *RelAbs,
      uint32_t *BrushID, uint32_t *Elements,
      U_PMF_POINTF **Points){
   if(!contents || !btype || !ctype || !RelAbs || !BrushID || !Elements || !Points){ return(0); }

   const char *blimit = contents;
   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_DRAWLINES))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }
   blimit += lclHeader.Size;

   *btype   = (lclHeader.Flags & U_PPF_B ? 1 : 0 );
   *ctype   = (lclHeader.Flags & U_PPF_C ? 1 : 0 );
   *RelAbs  = (lclHeader.Flags & U_PPF_R ? 1 : 0 );
   U_PMF_SERIAL_get(&contents, BrushID,  4, 1, (*btype ? U_XE : U_LE)); /* color is not byte swapped, ID integer is */
   U_PMF_SERIAL_get(&contents, Elements, 4, 1, U_LE);
   U_PMF_VARPOINTS_get(contents, lclHeader.Flags, *Elements, Points, blimit);
   return(1);
}

/**
    \brief Get data from a U_PMR_FILLRECTS record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  btype       Set: BrushID is an U_PFM_ARGB; Clear: is index of U_PMF_BRUSH object in EMF+ object table.
    \param  ctype       Set: int16_t coordinates; Clear: U_FLOAT coordinates
    \param  BrushID     Color or index of  U_PMF_BRUSH object in the EMF+ object table, depending on btype.
    \param  Elements    Number of members in Data.
    \param  Rects       Caller must free.  Array of U_PMF_RECTF rectangles to draw.

    EMF+ manual 2.3.4.20, Microsoft name: EmfPlusFillRects Record, Index 0x0A
    
    EMF+ files have been encountered where BrushID must be a color, because it has a value like FFFF0000 but
    the flags are set wrong, so that U_PPF_B is not set.  Detect these by BrushID >63 for btype=0 and correct.
    If the opposite problem occurs it cannot be reliably detected, so it cannot be corrected.

    Rects in record may be either U_PMF_RECT or U_PMF_RECTF, but this function always
    returns U_PMF_RECTF
*/
int U_PMR_FILLRECTS_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *btype, int *ctype,
      uint32_t *BrushID, uint32_t *Elements,
      U_PMF_RECTF **Rects){
   if(!contents || !btype || !ctype || !BrushID || !Elements || !Rects){ return(0); }

   const char *blimit = contents;
   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_FILLRECTS))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }
   blimit += lclHeader.Size;

   *btype  = (lclHeader.Flags & U_PPF_B ? 1 : 0 );
   *ctype  = (lclHeader.Flags & U_PPF_C ? 1 : 0 );
   U_PMF_SERIAL_get(&contents, BrushID,  4, 1, (*btype ? U_XE : U_LE)); /* color is not byte swapped, ID integer is */
   U_PMF_SERIAL_get(&contents, Elements, 4, 1, U_LE);
   U_PMF_VARRECTS_get(&contents, lclHeader.Flags, *Elements, Rects, blimit);
   /* correct btype, if necessary, for invalid EMF+ input */
   if((*BrushID > 63) & !*btype)*btype=1;
   return(1);
}


/**
    \brief Get data from a U_PMR_FILLREGION record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  RgnID       U_PMF_REGION object in the EMF+ object table (0-63, inclusive)
    \param  btype       Set: BrushID is an U_PFM_ARGB; Clear: is index of U_PMF_BRUSH object in EMF+ object table.
    \param  ctype       Set: int16_t coordinates; Clear: U_FLOAT coordinates
    \param  BrushID     Color or index of  U_PMF_BRUSH object in the EMF+ object table, depending on btype.

    EMF+ manual 2.3.4.21, Microsoft name: EmfPlusFillRegion Record, Index 0x13
*/
int U_PMR_FILLREGION_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint32_t *RgnID, int *btype, int *ctype,
      uint32_t *BrushID){
   if(!contents || !RgnID || !btype || !ctype || !BrushID){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_FILLREGION))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *btype  = (lclHeader.Flags & U_PPF_B ? 1 : 0 );
   *ctype  = (lclHeader.Flags & U_PPF_C ? 1 : 0 );
   *RgnID  = (lclHeader.Flags >> U_FF_SHFT_OID8) & U_FF_MASK_OID8;
   U_PMF_SERIAL_get(&contents, BrushID, 4, 1, (*btype ? U_XE : U_LE)); /* color is not byte swapped, ID integer is */
   return(1);
}

/**
    \brief Get data from a U_PMR_OBJECT record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  ObjID       Index for this object in the EMF+ object table (0-63, inclusive)
    \param  otype       ObjectType enumeration
    \param  ntype       Set: object definition continue bit is set
    \param  TSize       If ntype is set, holds the total number of data bytes split across multiple records.  If ntype is clear, has no meaning.
    \param  Data        Object's data.  Type from otype.

    EMF+ manual 2.3.5.1, Microsoft name: EmfPlusObject Record, Index 0x13
    
    Caller must check Data for possible memory access violations.

    OTHER NOTES:
    All objects are to be stored in the same table and retrieved by index.
    Documentation indicates that this table contains only 64 slots, although the index
       field which references it can code for values 0-127.
    If a new object has the same index as an existing object the old one is deleted and
       the new one goes into its storage slot.
    The continuation bit (U_PPF_N) is documented as indicating that the object is continued into
       the next record.  Examination of emf+ records in emf files produced by PowerPoint 2003
       show that changing the ObjID also serves as a continued record terminator, and that it apparently
       overrides the value for the continue bit.  That is, even though the preceding records said
       that it was continued, the change of ObjID terminates that preceding record without adding
       any more data to it.  In one example the sequential emf+ records were:
       ObjID   type  size  continue
       0       5     65008 Y
       0       5     65008 Y
       0       5     63104 Y
       1       8        24 N
       A DrawImagePoints record followed that referenced ObjID 0. 
    Examination of the records with continue set showed that data in each 
       was preceded by a uint32_t size value equivalent to the size of the 
       data that had been split across multiple records, in this case 
       0x0002F254 = 193108.  It is not clear at present if this size value 
       will also be present at the end of a continued series that terminates 
       by not using the continue bit, rather than changing the ObjID. 
*/
int U_PMR_OBJECT_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint32_t *ObjID, int *otype, int *ntype, uint32_t *TSize,
      const char **Data){
   if(!contents || !ObjID || !otype || !ntype || !Data){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_OBJECT))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *ntype  = (lclHeader.Flags & U_PPF_N ? 1 : 0 );
   *ObjID  = (lclHeader.Flags >> U_FF_SHFT_OID8) & U_FF_MASK_OID8;
   *otype  = (lclHeader.Flags >> U_FF_SHFT_OT) & U_FF_MASK_OT;
   if(*ntype){  U_PMF_SERIAL_get(&contents, TSize, 4, 1,  U_LE); }
   else {       *TSize = 0; }
   U_PMF_PTRSAV_SHIFT(Data, &contents, 0);
   return(1);
}
  
/**
    \brief Get data from a U_PMR_SERIALIZABLEOBJECT record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  GUID        ImageEffects identifier.
    \param  Size        Bytes in Data.
    \param  Data        "Serialized image effects parameter block".  One of the ImageEffects objects.

    EMF+ manual 2.3.5.2, Microsoft name: EmfPlusSerializableObject Record, Index 0x38

    Caller must check Data for possible memory access violations.
*/
int U_PMR_SERIALIZABLEOBJECT_get(const char *contents, U_PMF_CMN_HDR *Header,
      uint8_t *GUID, uint32_t *Size,
      const char **Data){
   if(!contents || !GUID || !Size || !Data){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_SERIALIZABLEOBJECT))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   U_PMF_SERIAL_get(&contents, GUID, 1, 16, U_XE);
   U_PMF_SERIAL_get(&contents, Size, 4, 1,  U_LE);
   U_PMF_PTRSAV_SHIFT(Data, &contents, 0);
   return(1);
}

/**
    \brief Get data from a U_PMR_SETANTIALIASMODE record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  SMenum      SmoothingMode enumeration
    \param  aatype      Set: anti-aliasing on; Clear: anti-aliasing off

    EMF+ manual 2.3.6.1, Microsoft name: EmfPlusSetAntiAliasMode Record, Index 0x1E
*/
int U_PMR_SETANTIALIASMODE_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *SMenum, int *aatype){
   if(!contents || !SMenum || !aatype){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_SETANTIALIASMODE))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *aatype = (lclHeader.Flags & U_PPF_AA ? 1 : 0 );
   *SMenum = (lclHeader.Flags >> U_FF_SHFT_AA) & U_FF_MASK_AA;
   return(1);
}

/**
    \brief Get data from a U_PMR_SETCOMPOSITINGMODE record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  CMenum      CompositingMode enumeration

    EMF+ manual 2.3.6.2, Microsoft name: EmfPlusSetCompositingMode Record, Index 0x23
*/
int U_PMR_SETCOMPOSITINGMODE_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *CMenum){
   /* memory access safe, only uses the common header */
   if(!contents || !CMenum){ return(0); }
   uint16_t Flags = U_PMF_HEADERFLAGS_get(contents);
   *CMenum  = (Flags >> U_FF_SHFT_CM) & U_FF_MASK_CM;
   U_PMF_CMN_HDR_get(&contents, Header);
   return(1);
}

/**
    \brief Get data from a U_PMR_SETCOMPOSITINGQUALITY record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  CQenum      CompositingQuality enumeration

    EMF+ manual 2.3.6.3, Microsoft name: EmfPlusSetCompositingQuality Record, Index 0x24
*/
int U_PMR_SETCOMPOSITINGQUALITY_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *CQenum){
   /* memory access safe, only uses the common header */
   if(!contents || !CQenum){ return(0); }
   uint16_t Flags = U_PMF_HEADERFLAGS_get(contents);
   *CQenum  = (Flags >> U_FF_SHFT_CQ) & U_FF_MASK_CQ;
   U_PMF_CMN_HDR_get(&contents, Header);
   return(1);
}

/**
    \brief Get data from a U_PMR_SETINTERPOLATIONMODE record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  IMenum      InterpolationMode enumeration

    EMF+ manual 2.3.6.4, Microsoft name: EmfPlusSetInterpolationMode Record, Index 0x21
*/
int U_PMR_SETINTERPOLATIONMODE_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *IMenum){
   /* memory access safe, only uses the common header */
   if(!contents || !IMenum){ return(0); }
   uint16_t Flags = U_PMF_HEADERFLAGS_get(contents);
   *IMenum  = (Flags >> U_FF_SHFT_IM) & U_FF_MASK_IM;
   U_PMF_CMN_HDR_get(&contents, Header);
   return(1);
}

/**
    \brief Get data from a U_PMR_SETPIXELOFFSETMODE record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  POMenum     PixelOffsetMode enumeration.

    EMF+ manual 2.3.6.5, Microsoft name: EmfPlusSetPixelOffsetMode Record, Index 0x22
*/
int U_PMR_SETPIXELOFFSETMODE_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *POMenum){
   /* memory access safe, only uses the common header */
   if(!contents || !POMenum){ return(0); }
   uint16_t Flags = U_PMF_HEADERFLAGS_get(contents);
   *POMenum  = (Flags >> U_FF_SHFT_PxOffM) & U_FF_MASK_PxOffM;
   U_PMF_CMN_HDR_get(&contents, Header);
   return(1);
}

/**
    \brief Get data from a U_PMR_SETRENDERINGORIGIN record
    \return 1 on success, 0 on error
    \param  contents     Record from which to extract data
    \param  Header       Common header.
    \param  X            X coordinate of rendering origin.
    \param  Y            Y coordinate of rendering origin.

    EMF+ manual 2.3.6.6, Microsoft name: EmfPlusSetRenderingOrigin Record, Index 0x1D
*/
int U_PMR_SETRENDERINGORIGIN_get(const char *contents, U_PMF_CMN_HDR *Header,
      int32_t *X, int32_t *Y){
   if(!contents || !X || !Y){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   if(!U_PMF_CMN_HDR_get(&contents, &lclHeader))return(0);
   if(lclHeader.Size < sizeof(U_PMF_SETRENDERINGORIGIN))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   U_PMF_SERIAL_get(&contents, X, 4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Y, 4, 1, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMR_SETTEXTCONTRAST record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header.
    \param  TGC         Text Gamma correction value (x 1000).

    EMF+ manual 2.3.6.7, Microsoft name: EmfPlusSetTextContrast Record, Index 0x20
*/
int U_PMR_SETTEXTCONTRAST_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *TGC){
   /* memory access safe, only uses the common header */
   if(!contents || !TGC){ return(0); }
   uint16_t Flags = U_PMF_HEADERFLAGS_get(contents);
   *TGC  = (Flags >> U_FF_SHFT_TGC) & U_FF_MASK_TGC;
   U_PMF_CMN_HDR_get(&contents, Header);
   return(1);
}

/**
    \brief Get data from a U_PMR_SETTEXTRENDERINGHINT record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header.
    \param  TRHenum     TextRenderingHint enumeration

    EMF+ manual 2.3.6.8, Microsoft name: EmfPlusSetTextRenderingHint Record, Index 0x1F
*/
int U_PMR_SETTEXTRENDERINGHINT_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *TRHenum){
   /* memory access safe, only uses the common header */
   if(!contents || !TRHenum){ return(0); }
   uint16_t Flags = U_PMF_HEADERFLAGS_get(contents);
   *TRHenum  = (Flags >> U_FF_SHFT_TRH) & U_FF_MASK_TRH;
   U_PMF_CMN_HDR_get(&contents, Header);
   return(1);
}

/**
    \brief Get data from a U_PMR_BEGINCONTAINER record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  UTenum      UnitType enumeration
    \param  DstRect     with SrcRect specifies a transformation
    \param  SrcRect     with DstRect specifies a transformation
    \param  StackID     EMF+ Object Stack Index to use for this graphics container

    EMF+ manual 2.3.7.1, Microsoft name: EmfPlusBeginContainer Record, Index 0x27
*/
int U_PMR_BEGINCONTAINER_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *UTenum,
      U_PMF_RECTF *DstRect, U_PMF_RECTF *SrcRect, uint32_t *StackID){
   if(!contents || !UTenum || !DstRect || !SrcRect || !StackID){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_SETCLIPREGION))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *UTenum  = (lclHeader.Flags >> U_FF_SHFT_UT) & U_FF_MASK_UT;
   U_PMF_SERIAL_get(&contents, DstRect, 4, 4, U_LE);
   U_PMF_SERIAL_get(&contents, SrcRect, 4, 4, U_LE);
   U_PMF_SERIAL_get(&contents, StackID, 4, 1, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMR_BEGINCONTAINERNOPARAMS record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  StackID     EMF+ Object Stack Index to use for this graphics container

    EMF+ manual 2.3.7.2, Microsoft name: EmfPlusBeginContainerNoParams Record, Index 0x28
*/
int U_PMR_BEGINCONTAINERNOPARAMS_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *StackID){
   return(U_PMR_common_stack_get(contents, Header, StackID));
}

/**
    \brief Get data from a U_PMR_ENDCONTAINER record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  StackID     EMF+ Object Stack Index of this graphics container

    EMF+ manual 2.3.7.3, Microsoft name: EmfPlusEndContainer Record, Index 0x29
*/
int U_PMR_ENDCONTAINER_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *StackID){
   return(U_PMR_common_stack_get(contents, Header, StackID));
}

/**
    \brief Get data from a U_PMR_RESTORE record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  StackID     State (level) to restore from the EMF+ Graphics Stack. Must have been put on the GS with a U_PMR_SAVE.

    EMF+ manual 2.3.7.4, Microsoft name: EmfPlusRestore Record, Index 0x26
*/
int U_PMR_RESTORE_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *StackID){
    return(U_PMR_common_stack_get(contents, Header, StackID));
}

/**
    \brief Get data from a U_PMR_SAVE record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  StackID     State (level) to save.on the EMF+ Graphics Stack

    EMF+ manual 2.3.7.5, Microsoft name: EmfPlusSave Record, Index 0x25
*/
int U_PMR_SAVE_get(const char *contents, U_PMF_CMN_HDR *Header, uint32_t *StackID){
   return(U_PMR_common_stack_get(contents, Header, StackID));
}

/**
    \brief Get data from a U_PMR_SETTSCLIP record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  ctype       Set: int16_t coordinates; Clear: U_FLOAT coordinates
    \param  Elements    Number of members in Data.
    \param  Rects       Caller must free.  Array of rectangles to draw.  Coordinate type set by ctype.

    EMF+ manual 2.3.8.1, Microsoft name: EmfPlusSetTSClip Record, Index 0x3A 
*/
int U_PMR_SETTSCLIP_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *ctype, uint32_t *Elements,
      U_PMF_RECTF **Rects){
   if(!contents || !ctype || !Elements || !Rects){ return(0); }

   const char *blimit = contents;
   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_SETTSCLIP))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }
   blimit += lclHeader.Size;

   *ctype     = (lclHeader.Flags & U_PPF_K ? 1 : 0 );
   *Elements  = (lclHeader.Flags >> U_FF_SHFT_TSC) & U_FF_MASK_TSC;
   U_PMF_VARRECTS_get(&contents, lclHeader.Flags, *Elements, Rects, blimit);
   return(1);
}

/**
    \brief Get data from a U_PMR_SETTSGRAPHICS record
    \return 1 on success, 0 on error
    \param  contents             Record from which to extract data
    \param  Header               Common header
    \param  vgatype              Set: Palette is VGA basic colors; Clear: Palette is ???
    \param  pptype               Set: Palette is present; Clear: Palette is absent.
    \param  AntiAliasMode        SmoothingMode enumeration
    \param  TextRenderHint       TextRenderingHint enumeration
    \param  CompositingMode      CompositingMode enumeration
    \param  CompositingQuality   CompositingQuality enumeration
    \param  RenderOriginX        Origin X for halftoning and dithering
    \param  RenderOriginY        Origin Y for halftoning and dithering
    \param  TextContrast         Gamma correction, range 0 to 12
    \param  FilterType           FilterType enumeraton
    \param  PixelOffset          PixelOffsetMode enumeration
    \param  WorldToDevice        world to device transform
    \param  Data                 Palette (optional)

    EMF+ manual 2.3.8.2, Microsoft name: EmfPlusSetTSGraphics Record, Index 0x39 

    Caller must check Data for possible memory access violations.
*/
int U_PMR_SETTSGRAPHICS_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *vgatype, int *pptype,
      uint8_t *AntiAliasMode, uint8_t *TextRenderHint, uint8_t  *CompositingMode, uint8_t *CompositingQuality,
      int16_t *RenderOriginX, int16_t *RenderOriginY,  uint16_t *TextContrast,    uint8_t *FilterType,
      uint8_t *PixelOffset,   U_PMF_TRANSFORMMATRIX *WorldToDevice,
      const char **Data){
   if(!contents || !vgatype || !pptype ||
      !AntiAliasMode || !TextRenderHint || !CompositingMode || !CompositingQuality ||
      !RenderOriginX || !RenderOriginY ||  !TextContrast ||    !FilterType ||
      !PixelOffset ||   !WorldToDevice || !Data){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_SETTSGRAPHICS))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *vgatype     = (lclHeader.Flags & U_PPF_VGA ? 1 : 0 );
   *pptype      = (lclHeader.Flags & U_PPF_PP  ? 1 : 0 );
   U_PMF_SERIAL_get(&contents, AntiAliasMode,      1, 1, U_XE);
   U_PMF_SERIAL_get(&contents, TextRenderHint,     1, 1, U_XE);
   U_PMF_SERIAL_get(&contents, CompositingMode,    1, 1, U_XE);
   U_PMF_SERIAL_get(&contents, CompositingQuality, 1, 1, U_XE);
   U_PMF_SERIAL_get(&contents, RenderOriginX,      2, 1, U_LE);
   U_PMF_SERIAL_get(&contents, RenderOriginY,      2, 1, U_LE);
   U_PMF_SERIAL_get(&contents, TextContrast,       2, 1, U_LE);
   U_PMF_SERIAL_get(&contents, FilterType,         1, 1, U_XE);
   U_PMF_SERIAL_get(&contents, WorldToDevice,      4, 6, U_LE);
   U_PMF_PTRSAV_COND(Data, contents, *pptype);
   return(1);
}

/**
    \brief Get data from a U_PMR_MULTIPLYWORLDTRANSFORM record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  xmtype      Set: Post multiply; Clear: Pre multiply
    \param  Matrix      Transformation matrix

    EMF+ manual 2.3.9.1, Microsoft name: EmfPlusMultiplyWorldTransform Record, Index 0x2C 
*/
int U_PMR_MULTIPLYWORLDTRANSFORM_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *xmtype,
      U_PMF_TRANSFORMMATRIX *Matrix){
   if(!contents || !xmtype || !Matrix){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_MULTIPLYWORLDTRANSFORM))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *xmtype     = (lclHeader.Flags & U_PPF_XM ? 1 : 0 );
   U_PMF_SERIAL_get(&contents, Matrix, 4, 6, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMR_RESETWORLDTRANSFORM record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header

    EMF+ manual 2.3.9.2, Microsoft name: EmfPlusResetWorldTransform Record, Index 0x2B
*/
int U_PMR_RESETWORLDTRANSFORM_get(const char *contents, U_PMF_CMN_HDR *Header){
   return( U_PMR_common_header_get(contents,Header));
}

/**
    \brief Get data from a U_PMR_ROTATEWORLDTRANSFORM record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  xmtype      Set: Post multiply; Clear: Pre multiply
    \param  Angle       Rotation angle, in degrees

    EMF+ manual 2.3.9.3, Microsoft name: EmfPlusRotateWorldTransform Record, Index 0x2F
*/
int U_PMR_ROTATEWORLDTRANSFORM_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *xmtype,
      U_FLOAT *Angle){
   if(!contents || !xmtype || !Angle){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_ROTATEWORLDTRANSFORM))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *xmtype     = (lclHeader.Flags & U_PPF_XM ? 1 : 0 );
   U_PMF_SERIAL_get(&contents, Angle, 4, 1, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMR_SCALEWORLDTRANSFORM record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  xmtype      Set: Post multiply; Clear: Pre multiply.
    \param  Sx          X scale factor.
    \param  Sy          Y scale factor.

    EMF+ manual 2.3.9.4, Microsoft name: EmfPlusScaleWorldTransform Record, Index 0x2E
*/
int U_PMR_SCALEWORLDTRANSFORM_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *xmtype,
      U_FLOAT *Sx, U_FLOAT *Sy){
   if(!contents || !xmtype || !Sx || !Sy){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_SCALEWORLDTRANSFORM))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *xmtype     = (lclHeader.Flags & U_PPF_XM ? 1 : 0 );
   U_PMF_SERIAL_get(&contents, Sx, 4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Sy, 4, 1, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMR_SETPAGETRANSFORM record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  PUenum      Page Unit, UnitType enumeration
    \param  Scale       Scale factor to convert page space to device space

    EMF+ manual 2.3.9.5, Microsoft name: EmfPlusSetPageTransform Record, Index 0x30
*/
int U_PMR_SETPAGETRANSFORM_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *PUenum,
      U_FLOAT *Scale){
   if(!contents || !PUenum || !Scale){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_SETPAGETRANSFORM))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *PUenum  = (lclHeader.Flags >> U_FF_SHFT_PU) & U_FF_MASK_PU;
   U_PMF_SERIAL_get(&contents, Scale, 4, 1, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMR_SETWORLDTRANSFORM record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  Matrix      Transformation matrix

    EMF+ manual 2.3.9.6, Microsoft name: EmfPlusSetWorldTransform Record, Index 0x2A
*/
int U_PMR_SETWORLDTRANSFORM_get(const char *contents, U_PMF_CMN_HDR *Header,
      U_PMF_TRANSFORMMATRIX *Matrix){
   if(!contents || !Matrix){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_SETWORLDTRANSFORM))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   U_PMF_SERIAL_get(&contents, Matrix, 4, 6, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMR_TRANSLATEWORLDTRANSFORM record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header
    \param  xmtype      Set: Post multiply; Clear: Pre multiply
    \param  Dx          X offset
    \param  Dy          Y offset

    EMF+ manual 2.3.9.7, Microsoft name: EmfPlusTranslateWorldTransform Record, Index 0x2D
*/
int U_PMR_TRANSLATEWORLDTRANSFORM_get(const char *contents, U_PMF_CMN_HDR *Header,
      int *xmtype,
      U_FLOAT *Dx, U_FLOAT *Dy){
   if(!contents || !xmtype || !Dx || !Dy){ return(0); }

   U_PMF_CMN_HDR lclHeader;
   U_PMF_CMN_HDR_get(&contents, &lclHeader);
   if(lclHeader.Size < sizeof(U_PMF_TRANSLATEWORLDTRANSFORM))return(0);
   if(Header){ memcpy(Header,&lclHeader,sizeof(U_PMF_CMN_HDR)); }

   *xmtype     = (lclHeader.Flags & U_PPF_XM ? 1 : 0 );
   U_PMF_SERIAL_get(&contents, Dx, 4, 1, U_LE);
   U_PMF_SERIAL_get(&contents, Dy, 4, 1, U_LE);
   return(1);
}

/**
    \brief Get data from a U_PMR_STROKEFILLPATH record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header  (ignore flags)
    

    EMF+ manual mentioned in 2.1.1.1, not otherwise documented, Microsoft name: EmfPlusStrokeFillPath Record, Index 0x37
    
    "This record closes any open figures in a path, strokes the outline of 
     the path by using the current pen, and fills its interior by using the current brush."
*/
int U_PMR_STROKEFILLPATH_get(const char *contents, U_PMF_CMN_HDR *Header){
   return( U_PMR_common_header_get(contents,Header));
}

/**
    \brief Get data from a U_PMR_MULTIFORMATSTART record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header  (ignore flags)

    EMF+ manual mentioned in 2.1.1.1, reserved, not otherwise documented, Microsoft name: EmfPlusMultiFormatStart Record, Index 0x05 
*/
int U_PMR_MULTIFORMATSTART_get(const char *contents, U_PMF_CMN_HDR *Header){
   return( U_PMR_common_header_get(contents,Header));
}

/**
    \brief Get data from a U_PMR_MULTIFORMATSECTION record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header  (ignore flags)

    EMF+ manual mentioned in 2.1.1.1, reserved, not otherwise documented, Microsoft name: EmfPlusMultiFormatSection Record, Index 0x06 
*/
int U_PMR_MULTIFORMATSECTION_get(const char *contents, U_PMF_CMN_HDR *Header){
   return( U_PMR_common_header_get(contents,Header));
}

/**
    \brief Get data from a U_PMR_MULTIFORMATEND record
    \return 1 on success, 0 on error
    \param  contents    Record from which to extract data
    \param  Header      Common header  (ignore flags)

    EMF+ manual mentioned in 2.1.1.1, reserved, not otherwise documented, Microsoft name: EmfPlusMultiFormatEnd Record, Index 0x06
*/
int U_PMR_MULTIFORMATEND_get(const char *contents, U_PMF_CMN_HDR *Header){
   return( U_PMR_common_header_get(contents,Header));
}


#ifdef __cplusplus
}
#endif
