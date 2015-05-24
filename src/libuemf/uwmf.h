/**
  @file uwmf.h
  
  @brief Structures, definitions, and function prototypes for WMF files.

  WMF file Record structure information has been derived from Mingw and Wine header files, and from
  Microsoft's WMF Information pdf, release date July 5,2012, link from here:
  
     http://msdn2.microsoft.com/en-us/library/cc250370.aspx
  
  If the direct link fails the document may be found
  by searching for: "[MS-WMF]: Windows Metafile Format"
  
  *********************************** IMPORTANT!!!  **********************************************
  WMF is a 16 bit file type that has some 32 bit integers embedded in it.  In
  a few cases these 32 bit fields are not aligned in the structures defined in uwmf.h, but
  in most cases they are.  So when creating  the individual WMF records the functions in
  uwmf.c can usually use a regular assignment operation for the 32 bit fields.  However, once the
  records are part of a WMF file in memory there is no guaranty that any 32 bit type will be correctly
  aligned.  Similarly, many WMF structures contain embedded other structures which would "naturally"
  be passed by pointer, but since their alignment may not be what malloc() would have created for that
  type, the outcome of that operation is not defined by the C standard. (Per Eric Sosman, section
  6.3.2.3p7 of the standard.)
  
  For this reason, the _print, _swap and any read operations must pass structures with unknown alignment
  as a (char *), and pull out the data using memcpy() or some equivalent
  that will not segfault when it tries to read a 32 bit value that is not aligned
  on a 4 byte boundary.  Failure to do so will result in nonportable code. You have been warned!
  
  Problem areas:
     The Size16_4 field of all WMF records may NOT be assumed to 4 byte aligned.
     DIB's U_BITMAPINFOHEADER 32 bit fields may not be aligned.
  *********************************** IMPORTANT!!!  **********************************************

*/

/*
File:      uwmf.h
Version:   0.0.12
Date:      28-APR-2015
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2015 David Mathog and California Institute of Technology (Caltech)
*/

#ifndef _UWMF_
#define _UWMF_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "uemf.h"        /* many structures/defs in common, pull in the EMF ones as a basis */
#include "uemf_utf.h"
#include "uwmf_endian.h"


/** HighWater Enumeration               not in WMF manual
  @{
*/
#define U_HIGHWATER_READ  0x00000000 //!< nondestructive read of highwater value
#define U_HIGHWATER_CLEAR 0xFFFFFFFF //!< destructive read, value is reset to 0
/** @} */




// ***********************************************************************************
// Value Enumerations and other predefined constants, alphabetical order by group

/*     RecordType Enumeration                 WMF manual 2.1.1.1 */
/** WMF manual 2.1.1.1
  \brief WMR Record types
*/
enum U_WMR_TYPES{
   U_WMR_EOF,                    //!< 0x0000  U_WMREOF                               record
   U_WMR_SETBKCOLOR,             //!< 0x0201  U_WMRSETBKCOLOR                        record
   U_WMR_SETBKMODE,              //!< 0x0102  U_WMRSETBKMODE                         record
   U_WMR_SETMAPMODE,             //!< 0x0103  U_WMRSETMAPMODE                        record
   U_WMR_SETROP2,                //!< 0x0104  U_WMRSETROP2                           record
   U_WMR_SETRELABS,              //!< 0x0105  U_WMRSETRELABS                         record
   U_WMR_SETPOLYFILLMODE,        //!< 0x0106  U_WMRSETPOLYFILLMODE                   record
   U_WMR_SETSTRETCHBLTMODE,      //!< 0x0107  U_WMRSETSTRETCHBLTMODE                 record
   U_WMR_SETTEXTCHAREXTRA,       //!< 0x0108  U_WMRSETTEXTCHAREXTRA                  record
   U_WMR_SETTEXTCOLOR,           //!< 0x0209  U_WMRSETTEXTCOLOR                      record
   U_WMR_SETTEXTJUSTIFICATION,   //!< 0x020A  U_WMRSETTEXTJUSTIFICATION              record
   U_WMR_SETWINDOWORG,           //!< 0x020B  U_WMRSETWINDOWORG                      record
   U_WMR_SETWINDOWEXT,           //!< 0x020C  U_WMRSETWINDOWEXT                      record
   U_WMR_SETVIEWPORTORG,         //!< 0x020D  U_WMRSETVIEWPORTORG                    record
   U_WMR_SETVIEWPORTEXT,         //!< 0x020E  U_WMRSETVIEWPORTEXT                    record
   U_WMR_OFFSETWINDOWORG,        //!< 0x020F  U_WMROFFSETWINDOWORG                   record
   U_WMR_SCALEWINDOWEXT,         //!< 0x0410  U_WMRSCALEWINDOWEXT                    record
   U_WMR_OFFSETVIEWPORTORG,      //!< 0x0211  U_WMROFFSETVIEWPORTORG                 record
   U_WMR_SCALEVIEWPORTEXT,       //!< 0x0412  U_WMRSCALEVIEWPORTEXT                  record
   U_WMR_LINETO,                 //!< 0x0213  U_WMRLINETO                            record
   U_WMR_MOVETO,                 //!< 0x0214  U_WMRMOVETO                            record
   U_WMR_EXCLUDECLIPRECT,        //!< 0x0415  U_WMREXCLUDECLIPRECT                   record
   U_WMR_INTERSECTCLIPRECT,      //!< 0x0416  U_WMRINTERSECTCLIPRECT                 record
   U_WMR_ARC,                    //!< 0x0817  U_WMRARC                               record
   U_WMR_ELLIPSE,                //!< 0x0418  U_WMRELLIPSE                           record
   U_WMR_FLOODFILL,              //!< 0x0419  U_WMRFLOODFILL                         record
   U_WMR_PIE,                    //!< 0x081A  U_WMRPIE                               record
   U_WMR_RECTANGLE,              //!< 0x041B  U_WMRRECTANGLE                         record
   U_WMR_ROUNDRECT,              //!< 0x061C  U_WMRROUNDRECT                         record
   U_WMR_PATBLT,                 //!< 0x061D  U_WMRPATBLT                            record
   U_WMR_SAVEDC,                 //!< 0x001E  U_WMRSAVEDC                            record
   U_WMR_SETPIXEL,               //!< 0x041F  U_WMRSETPIXEL                          record
   U_WMR_OFFSETCLIPRGN,          //!< 0x0220  U_WMROFFSETCLIPRGN                     record
   U_WMR_TEXTOUT,                //!< 0x0521  U_WMRTEXTOUT                           record
   U_WMR_BITBLT,                 //!< 0x0922  U_WMRBITBLT                            record
   U_WMR_STRETCHBLT,             //!< 0x0B23  U_WMRSTRETCHBLT                        record
   U_WMR_POLYGON,                //!< 0x0324  U_WMRPOLYGON                           record
   U_WMR_POLYLINE,               //!< 0x0325  U_WMRPOLYLINE                          record
   U_WMR_ESCAPE,                 //!< 0x0626  U_WMRESCAPE                            record
   U_WMR_RESTOREDC,              //!< 0x0127  U_WMRRESTOREDC                         record
   U_WMR_FILLREGION,             //!< 0x0228  U_WMRFILLREGION                        record
   U_WMR_FRAMEREGION,            //!< 0x0429  U_WMRFRAMEREGION                       record
   U_WMR_INVERTREGION,           //!< 0x012A  U_WMRINVERTREGION                      record
   U_WMR_PAINTREGION,            //!< 0x012B  U_WMRPAINTREGION                       record
   U_WMR_SELECTCLIPREGION,       //!< 0x012C  U_WMRSELECTCLIPREGION                  record
   U_WMR_SELECTOBJECT,           //!< 0x012D  U_WMRSELECTOBJECT                      record
   U_WMR_SETTEXTALIGN,           //!< 0x012E  U_WMRSETTEXTALIGN                      record
   U_WMR_DRAWTEXT,               //!< 0x062F  U_WMRDRAWTEXT                          record
   U_WMR_CHORD,                  //!< 0x0830  U_WMRCHORD                             record
   U_WMR_SETMAPPERFLAGS,         //!< 0x0231  U_WMRSETMAPPERFLAGS                    record
   U_WMR_EXTTEXTOUT,             //!< 0x0A32  U_WMREXTTEXTOUT                        record
   U_WMR_SETDIBTODEV,            //!< 0x0D33  U_WMRSETDIBTODEV                       record
   U_WMR_SELECTPALETTE,          //!< 0x0234  U_WMRSELECTPALETTE                     record
   U_WMR_REALIZEPALETTE,         //!< 0x0035  U_WMRREALIZEPALETTE                    record
   U_WMR_ANIMATEPALETTE,         //!< 0x0436  U_WMRANIMATEPALETTE                    record
   U_WMR_SETPALENTRIES,          //!< 0x0037  U_WMRSETPALENTRIES                     record
   U_WMR_POLYPOLYGON,            //!< 0x0538  U_WMRPOLYPOLYGON                       record
   U_WMR_RESIZEPALETTE,          //!< 0x0139  U_WMRRESIZEPALETTE                     record
   U_WMR_3A,                     //!< 0x003A  U_WMR3A                                record
   U_WMR_3B,                     //!< 0x003B  U_WMR3B                                record
   U_WMR_3C,                     //!< 0x003C  U_WMR3C                                record
   U_WMR_3D,                     //!< 0x003D  U_WMR3D                                record
   U_WMR_3E,                     //!< 0x003E  U_WMR3E                                record
   U_WMR_3F,                     //!< 0x003F  U_WMR3F                                record
   U_WMR_DIBBITBLT,              //!< 0x0940  U_WMRDIBBITBLT                         record
   U_WMR_DIBSTRETCHBLT,          //!< 0x0B41  U_WMRDIBSTRETCHBLT                     record
   U_WMR_DIBCREATEPATTERNBRUSH,  //!< 0x0142  U_WMRDIBCREATEPATTERNBRUSH             record
   U_WMR_STRETCHDIB,             //!< 0x0F43  U_WMRSTRETCHDIB                        record
   U_WMR_44,                     //!< 0x0044  U_WMR44                                record
   U_WMR_45,                     //!< 0x0045  U_WMR45                                record
   U_WMR_46,                     //!< 0x0046  U_WMR46                                record
   U_WMR_47,                     //!< 0x0047  U_WMR47                                record
   U_WMR_EXTFLOODFILL,           //!< 0x0548  U_WMREXTFLOODFILL                      record
   U_WMR_49,                     //!< 0x0049  U_WMR49                                record
   U_WMR_4A,                     //!< 0x004A  U_WMR4A                                record
   U_WMR_4B,                     //!< 0x004B  U_WMR4B                                record
   U_WMR_4C,                     //!< 0x014C  U_WMR4C                                record
   U_WMR_4D,                     //!< 0x014D  U_WMR4D                                record
   U_WMR_4E,                     //!< 0x004E  U_WMR4E                                record
   U_WMR_4F,                     //!< 0x004F  U_WMR4F                                record
   U_WMR_50,                     //!< 0x0050  U_WMR50                                record
   U_WMR_51,                     //!< 0x0051  U_WMR51                                record
   U_WMR_52,                     //!< 0x0052  U_WMR52                                record
   U_WMR_53,                     //!< 0x0053  U_WMR53                                record
   U_WMR_54,                     //!< 0x0054  U_WMR54                                record
   U_WMR_55,                     //!< 0x0055  U_WMR55                                record
   U_WMR_56,                     //!< 0x0056  U_WMR56                                record
   U_WMR_57,                     //!< 0x0057  U_WMR57                                record
   U_WMR_58,                     //!< 0x0058  U_WMR58                                record
   U_WMR_59,                     //!< 0x0059  U_WMR59                                record
   U_WMR_5A,                     //!< 0x005A  U_WMR5A                                record
   U_WMR_5B,                     //!< 0x005B  U_WMR5B                                record
   U_WMR_5C,                     //!< 0x005C  U_WMR5C                                record
   U_WMR_5D,                     //!< 0x005D  U_WMR5D                                record
   U_WMR_5E,                     //!< 0x005E  U_WMR5E                                record
   U_WMR_5F,                     //!< 0x005F  U_WMR5F                                record
   U_WMR_60,                     //!< 0x0060  U_WMR60                                record
   U_WMR_61,                     //!< 0x0061  U_WMR61                                record
   U_WMR_62,                     //!< 0x0062  U_WMR62                                record
   U_WMR_63,                     //!< 0x0063  U_WMR63                                record
   U_WMR_64,                     //!< 0x0064  U_WMR64                                record
   U_WMR_65,                     //!< 0x0065  U_WMR65                                record
   U_WMR_66,                     //!< 0x0066  U_WMR66                                record
   U_WMR_67,                     //!< 0x0067  U_WMR67                                record
   U_WMR_68,                     //!< 0x0068  U_WMR68                                record
   U_WMR_69,                     //!< 0x0069  U_WMR69                                record
   U_WMR_6A,                     //!< 0x006A  U_WMR6A                                record
   U_WMR_6B,                     //!< 0x006B  U_WMR6B                                record
   U_WMR_6C,                     //!< 0x006C  U_WMR6C                                record
   U_WMR_6D,                     //!< 0x006D  U_WMR6D                                record
   U_WMR_6E,                     //!< 0x006E  U_WMR6E                                record
   U_WMR_6F,                     //!< 0x006F  U_WMR6F                                record
   U_WMR_70,                     //!< 0x0070  U_WMR70                                record
   U_WMR_71,                     //!< 0x0071  U_WMR71                                record
   U_WMR_72,                     //!< 0x0072  U_WMR72                                record
   U_WMR_73,                     //!< 0x0073  U_WMR73                                record
   U_WMR_74,                     //!< 0x0074  U_WMR74                                record
   U_WMR_75,                     //!< 0x0075  U_WMR75                                record
   U_WMR_76,                     //!< 0x0076  U_WMR76                                record
   U_WMR_77,                     //!< 0x0077  U_WMR77                                record
   U_WMR_78,                     //!< 0x0078  U_WMR78                                record
   U_WMR_79,                     //!< 0x0079  U_WMR79                                record
   U_WMR_7A,                     //!< 0x007A  U_WMR7A                                record
   U_WMR_7B,                     //!< 0x007B  U_WMR7B                                record
   U_WMR_7C,                     //!< 0x007C  U_WMR7C                                record
   U_WMR_7D,                     //!< 0x007D  U_WMR7D                                record
   U_WMR_7E,                     //!< 0x007E  U_WMR7E                                record
   U_WMR_7F,                     //!< 0x007F  U_WMR7F                                record
   U_WMR_80,                     //!< 0x0080  U_WMR80                                record
   U_WMR_81,                     //!< 0x0081  U_WMR81                                record
   U_WMR_82,                     //!< 0x0082  U_WMR82                                record
   U_WMR_83,                     //!< 0x0083  U_WMR83                                record
   U_WMR_84,                     //!< 0x0084  U_WMR84                                record
   U_WMR_85,                     //!< 0x0085  U_WMR85                                record
   U_WMR_86,                     //!< 0x0086  U_WMR86                                record
   U_WMR_87,                     //!< 0x0087  U_WMR87                                record
   U_WMR_88,                     //!< 0x0088  U_WMR88                                record
   U_WMR_89,                     //!< 0x0089  U_WMR89                                record
   U_WMR_8A,                     //!< 0x008A  U_WMR8A                                record
   U_WMR_8B,                     //!< 0x008B  U_WMR8B                                record
   U_WMR_8C,                     //!< 0x008C  U_WMR8C                                record
   U_WMR_8D,                     //!< 0x008D  U_WMR8D                                record
   U_WMR_8E,                     //!< 0x008E  U_WMR8E                                record
   U_WMR_8F,                     //!< 0x008F  U_WMR8F                                record
   U_WMR_90,                     //!< 0x0090  U_WMR90                                record
   U_WMR_91,                     //!< 0x0091  U_WMR91                                record
   U_WMR_92,                     //!< 0x0092  U_WMR92                                record
   U_WMR_93,                     //!< 0x0093  U_WMR93                                record
   U_WMR_94,                     //!< 0x0094  U_WMR94                                record
   U_WMR_95,                     //!< 0x0095  U_WMR95                                record
   U_WMR_96,                     //!< 0x0096  U_WMR96                                record
   U_WMR_97,                     //!< 0x0097  U_WMR97                                record
   U_WMR_98,                     //!< 0x0098  U_WMR98                                record
   U_WMR_99,                     //!< 0x0099  U_WMR99                                record
   U_WMR_9A,                     //!< 0x009A  U_WMR9A                                record
   U_WMR_9B,                     //!< 0x009B  U_WMR9B                                record
   U_WMR_9C,                     //!< 0x009C  U_WMR9C                                record
   U_WMR_9D,                     //!< 0x009D  U_WMR9D                                record
   U_WMR_9E,                     //!< 0x009E  U_WMR9E                                record
   U_WMR_9F,                     //!< 0x009F  U_WMR9F                                record
   U_WMR_A0,                     //!< 0x00A0  U_WMRA0                                record
   U_WMR_A1,                     //!< 0x00A1  U_WMRA1                                record
   U_WMR_A2,                     //!< 0x00A2  U_WMRA2                                record
   U_WMR_A3,                     //!< 0x00A3  U_WMRA3                                record
   U_WMR_A4,                     //!< 0x00A4  U_WMRA4                                record
   U_WMR_A5,                     //!< 0x00A5  U_WMRA5                                record
   U_WMR_A6,                     //!< 0x00A6  U_WMRA6                                record
   U_WMR_A7,                     //!< 0x00A7  U_WMRA7                                record
   U_WMR_A8,                     //!< 0x00A8  U_WMRA8                                record
   U_WMR_A9,                     //!< 0x00A9  U_WMRA9                                record
   U_WMR_AA,                     //!< 0x00AA  U_WMRAA                                record
   U_WMR_AB,                     //!< 0x00AB  U_WMRAB                                record
   U_WMR_AC,                     //!< 0x00AC  U_WMRAC                                record
   U_WMR_AD,                     //!< 0x00AD  U_WMRAD                                record
   U_WMR_AE,                     //!< 0x00AE  U_WMRAE                                record
   U_WMR_AF,                     //!< 0x00AF  U_WMRAF                                record
   U_WMR_B0,                     //!< 0x00B0  U_WMRB0                                record
   U_WMR_B1,                     //!< 0x00B1  U_WMRB1                                record
   U_WMR_B2,                     //!< 0x00B2  U_WMRB2                                record
   U_WMR_B3,                     //!< 0x00B3  U_WMRB3                                record
   U_WMR_B4,                     //!< 0x00B4  U_WMRB4                                record
   U_WMR_B5,                     //!< 0x00B5  U_WMRB5                                record
   U_WMR_B6,                     //!< 0x00B6  U_WMRB6                                record
   U_WMR_B7,                     //!< 0x00B7  U_WMRB7                                record
   U_WMR_B8,                     //!< 0x00B8  U_WMRB8                                record
   U_WMR_B9,                     //!< 0x00B9  U_WMRB9                                record
   U_WMR_BA,                     //!< 0x00BA  U_WMRBA                                record
   U_WMR_BB,                     //!< 0x00BB  U_WMRBB                                record
   U_WMR_BC,                     //!< 0x00BC  U_WMRBC                                record
   U_WMR_BD,                     //!< 0x00BD  U_WMRBD                                record
   U_WMR_BE,                     //!< 0x00BE  U_WMRBE                                record
   U_WMR_BF,                     //!< 0x00BF  U_WMRBF                                record
   U_WMR_C0,                     //!< 0x00C0  U_WMRC0                                record
   U_WMR_C1,                     //!< 0x00C1  U_WMRC1                                record
   U_WMR_C2,                     //!< 0x00C2  U_WMRC2                                record
   U_WMR_C3,                     //!< 0x00C3  U_WMRC3                                record
   U_WMR_C4,                     //!< 0x00C4  U_WMRC4                                record
   U_WMR_C5,                     //!< 0x00C5  U_WMRC5                                record
   U_WMR_C6,                     //!< 0x00C6  U_WMRC6                                record
   U_WMR_C7,                     //!< 0x00C7  U_WMRC7                                record
   U_WMR_C8,                     //!< 0x00C8  U_WMRC8                                record
   U_WMR_C9,                     //!< 0x00C9  U_WMRC9                                record
   U_WMR_CA,                     //!< 0x00CA  U_WMRCA                                record
   U_WMR_CB,                     //!< 0x00CB  U_WMRCB                                record
   U_WMR_CC,                     //!< 0x00CC  U_WMRCC                                record
   U_WMR_CD,                     //!< 0x00CD  U_WMRCD                                record
   U_WMR_CE,                     //!< 0x00CE  U_WMRCE                                record
   U_WMR_CF,                     //!< 0x00CF  U_WMRCF                                record
   U_WMR_D0,                     //!< 0x00D0  U_WMRD0                                record
   U_WMR_D1,                     //!< 0x00D1  U_WMRD1                                record
   U_WMR_D2,                     //!< 0x00D2  U_WMRD2                                record
   U_WMR_D3,                     //!< 0x00D3  U_WMRD3                                record
   U_WMR_D4,                     //!< 0x00D4  U_WMRD4                                record
   U_WMR_D5,                     //!< 0x00D5  U_WMRD5                                record
   U_WMR_D6,                     //!< 0x00D6  U_WMRD6                                record
   U_WMR_D7,                     //!< 0x00D7  U_WMRD7                                record
   U_WMR_D8,                     //!< 0x00D8  U_WMRD8                                record
   U_WMR_D9,                     //!< 0x00D9  U_WMRD9                                record
   U_WMR_DA,                     //!< 0x00DA  U_WMRDA                                record
   U_WMR_DB,                     //!< 0x00DB  U_WMRDB                                record
   U_WMR_DC,                     //!< 0x00DC  U_WMRDC                                record
   U_WMR_DD,                     //!< 0x00DD  U_WMRDD                                record
   U_WMR_DE,                     //!< 0x00DE  U_WMRDE                                record
   U_WMR_DF,                     //!< 0x00DF  U_WMRDF                                record
   U_WMR_E0,                     //!< 0x00E0  U_WMRE0                                record
   U_WMR_E1,                     //!< 0x00E1  U_WMRE1                                record
   U_WMR_E2,                     //!< 0x00E2  U_WMRE2                                record
   U_WMR_E3,                     //!< 0x00E3  U_WMRE3                                record
   U_WMR_E4,                     //!< 0x00E4  U_WMRE4                                record
   U_WMR_E5,                     //!< 0x00E5  U_WMRE5                                record
   U_WMR_E6,                     //!< 0x00E6  U_WMRE6                                record
   U_WMR_E7,                     //!< 0x00E7  U_WMRE7                                record
   U_WMR_E8,                     //!< 0x00E8  U_WMRE8                                record
   U_WMR_E9,                     //!< 0x00E9  U_WMRE9                                record
   U_WMR_EA,                     //!< 0x00EA  U_WMREA                                record
   U_WMR_EB,                     //!< 0x00EB  U_WMREB                                record
   U_WMR_EC,                     //!< 0x00EC  U_WMREC                                record
   U_WMR_ED,                     //!< 0x00ED  U_WMRED                                record
   U_WMR_EE,                     //!< 0x00EE  U_WMREE                                record
   U_WMR_EF,                     //!< 0x00EF  U_WMREF                                record
   U_WMR_DELETEOBJECT,           //!< 0x01F0  U_WMRDELETEOBJECT                      record
   U_WMR_F1,                     //!< 0x00F1  U_WMRF1                                record
   U_WMR_F2,                     //!< 0x00F2  U_WMRF2                                record
   U_WMR_F3,                     //!< 0x00F3  U_WMRF3                                record
   U_WMR_F4,                     //!< 0x00F4  U_WMRF4                                record
   U_WMR_F5,                     //!< 0x00F5  U_WMRF5                                record
   U_WMR_F6,                     //!< 0x00F6  U_WMRF6                                record
   U_WMR_CREATEPALETTE,          //!< 0x00F7  U_WMRCREATEPALETTE                     record
   U_WMR_F8         ,            //!< 0x00F8  U_WMRF8                                record
   U_WMR_CREATEPATTERNBRUSH,     //!< 0x01F9  U_WMRCREATEPATTERNBRUSH                record
   U_WMR_CREATEPENINDIRECT,      //!< 0x02FA  U_WMRCREATEPENINDIRECT                 record
   U_WMR_CREATEFONTINDIRECT,     //!< 0x02FB  U_WMRCREATEFONTINDIRECT                record
   U_WMR_CREATEBRUSHINDIRECT,    //!< 0x02FC  U_WMRCREATEBRUSHINDIRECT               record
   U_WMR_CREATEBITMAPINDIRECT,   //!< 0x02FD  U_WMRCREATEBITMAPINDIRECT              record
   U_WMR_CREATEBITMAP,           //!< 0x06FE  U_WMRCREATEBITMAP                      record
   U_WMR_CREATEREGION,           //!< 0x06FF  U_WMRCREATEREGION                      record
}; 
#define U_WMR_MIN 0                  //!< Minimum U_WMR_ value.
#define U_WMR_MAX 255                //!< Maximum U_WMR_ value.
#define U_WMR_MASK  0xFF             //!< Mask for enumerator (lower) byte
#define U_WMR_INVALID         0xFFFFFFFF //!< Indicates "Not a valid U_WMR_* value"


/** BinaryRasterOperation Enumeration      WMF manual 2.1.1.2
  
  Same as U_EMF_EMRSETROP2 in uemf.h
*/

/** BitCount Enumeration                   WMF manual 2.1.1.3
  \defgroup U_WMF_AltBitCount_Qualifiers WMF Alternate names for the values under EMF Bitcount Enumeration in uemf.h
  @{
*/
#define  BI_BITCOUNT_0  U_BCBM_EXPLICIT   //!< Derived from JPG or PNG compressed image or ?
#define  BI_BITCOUNT_1  U_BCBM_MONOCHROME //!< 2 colors.    bmiColors array has two entries
#define  BI_BITCOUNT_2  U_BCBM_COLOR4     //!< 2^4 colors.  bmiColors array has 16 entries
#define  BI_BITCOUNT_3  U_BCBM_COLOR8     //!< 2^8 colors.  bmiColors array has 256 entries
#define  BI_BITCOUNT_4  U_BCBM_COLOR16    //!< 2^16 colors. bmiColors is not used. Pixels are 5 bits B,G,R with 1 unused bit
#define  BI_BITCOUNT_5  U_BCBM_COLOR24    //!< 2^24 colors. bmiColors is not used. Pixels are U_RGBTRIPLE.
#define  BI_BITCOUNT_6  U_BCBM_COLOR32    //!< 2^32 colors. bmiColors is not used. Pixels are U_RGBQUAD.
/** @} */

/*  BrushStyle Enumeration                WMF manual 2.1.1.4
  Same as "EMF LB_Style Enumeration" in uemf.h
*/

/* CharacterSet Enumeration               WMF manual 2.1.1.5
  Same as "EMF LF_CharSet Enumeration" in uemf.h
*/

/** ColorUsage Enumeration                 WMF manual 2.1.1.6
  For cUsage fields in various DIB related records.
  \defgroup U_WMF_Extra_iUsageSrc_Qualifiers WMF Extra DIBColors Enumeration
  WMF is the same as "EMF DIBColors Enumeration" in uemf.h, except it also supports
  this one extra value.
  @{
*/
#define U_DIB_PAL_INDICES 2  //!< No color table, pixels are logical palette indices.
/** @} */

/** Compression Enumeration                WMF manual 2.1.1.7
  Same as "EMF BI_Compression Enumeration" in uemf.h with these additions
  \defgroup U_WMF_EXTRA_BITMAPINFOHEADER_biCompression_Qualifiers WMF Extra BI_Compression Enumerations, none are implemented
  @{
*/
#define U_BI_CMYK       0x000B //!< CMYK uncompressed
#define U_BI_CMYKRLE8   0x000C //!< CMYK RLE8 compression
#define U_BI_CMYKRLE4 = 0x000D //!< CMYK RLE4 compression
/** @} */

/* FamilyFont Enumeration                 WMF manual 2.1.1.8
    Only used in a PitchAndFamily object, defined there
*/

/* FloodFill Enumeration                  WMF manual 2.1.1.9
    Same as "EMF FloodFill Enumeration" in uemf.h
*/

/* FontQuality Enumeration                WMF manual 2.1.1.10
    Same as "EMF LF_Quality Enumeration" in uemf.h
*/

/* GamutMappingIntent Enumeration         WMF manual 2.1.1.11
    Same as "EMF LCS_Intent Enumeration" in uemf.h
*/

/* HatchStyle Enumeration                 WMF manual 2.1.1.12
    Same as "EMF HatchStyle Enumeration" in uemf.h
*/

/* Layout Enumeration                     WMF manual 2.1.1.13
    Same as "EMF Mirroring Enumeration" in uemf.h
*/

/** LogicalColorSpace Enumeration WMF manual 2.1.1.14
    Not used presently, applies in BitmapV4Header
  @{
*/
/*  U_LCS_CALIBRATED_RGB is defined in uemf.h under LCS_CSType Enumeration, WMF manual also defines it, but do not replicate define.
#define U_LCS_CALIBRATED_RGB        0x00000000    //!< calibrated RGB  
*/
#define U_LCS_sRGB                  0x73524742    //!< ASCII for "sRGB"
#define U_LCS_WINDOWS_COLOR_SPACE   0x57696E20    //!< ASCII for "Win "
/** @} */

/* LogicalColorSpaceV5 Enumeration        WMF manual 2.1.1.15
    Same as "EMF Profile Enumeration" in uemf.h
*/

/* MapMode Enumeration                    WMF manual 2.1.1.16
    Same as "EMF MapMode Enumeration" in uemf.h
*/
 
/** MetaFilesEscape Enumeration      WMF manual 2.1.1.17
  \defgroup U_WMF_MFEscape_Qualifiers WMF Metafile Escape record types
  For U_WMRESCAPE eFunc field
  @{
*/
#define U_MFE_NEWFRAME                      0x0001   //!< NEWFRAME                      escape type
#define U_MFE_ABORTDOC                      0x0002   //!< ABORTDOC                      escape type
#define U_MFE_NEXTBAND                      0x0003   //!< NEXTBAND                      escape type
#define U_MFE_SETCOLORTABLE                 0x0004   //!< SETCOLORTABLE                 escape type
#define U_MFE_GETCOLORTABLE                 0x0005   //!< GETCOLORTABLE                 escape type
#define U_MFE_FLUSHOUT                      0x0006   //!< FLUSHOUT                      escape type
#define U_MFE_DRAFTMODE                     0x0007   //!< DRAFTMODE                     escape type
#define U_MFE_QUERYESCSUPPORT               0x0008   //!< QUERYESCSUPPORT               escape type
#define U_MFE_SETABORTPROC                  0x0009   //!< SETABORTPROC                  escape type
#define U_MFE_STARTDOC                      0x000A   //!< STARTDOC                      escape type
#define U_MFE_ENDDOC                        0x000B   //!< ENDDOC                        escape type
#define U_MFE_GETPHYSPAGESIZE               0x000C   //!< GETPHYSPAGESIZE               escape type
#define U_MFE_GETPRINTINGOFFSET             0x000D   //!< GETPRINTINGOFFSET             escape type
#define U_MFE_GETSCALINGFACTOR              0x000E   //!< GETSCALINGFACTOR              escape type
#define U_MFE_META_ESCAPE_ENHANCED_METAFILE 0x000F   //!< META_ESCAPE_ENHANCED_METAFILE escape type
#define U_MFE_SETPENWIDTH                   0x0010   //!< SETPENWIDTH                   escape type
#define U_MFE_SETCOPYCOUNT                  0x0011   //!< SETCOPYCOUNT                  escape type
#define U_MFE_SETPAPERSOURCE                0x0012   //!< SETPAPERSOURCE                escape type
#define U_MFE_PASSTHROUGH                   0x0013   //!< PASSTHROUGH                   escape type
#define U_MFE_GETTECHNOLOGY                 0x0014   //!< GETTECHNOLOGY                 escape type
#define U_MFE_SETLINECAP                    0x0015   //!< SETLINECAP                    escape type
#define U_MFE_SETLINEJOIN                   0x0016   //!< SETLINEJOIN                   escape type
#define U_MFE_SETMITERLIMIT                 0x0017   //!< SETMITERLIMIT                 escape type
#define U_MFE_BANDINFO                      0x0018   //!< BANDINFO                      escape type
#define U_MFE_DRAWPATTERNRECT               0x0019   //!< DRAWPATTERNRECT               escape type
#define U_MFE_GETVECTORPENSIZE              0x001A   //!< GETVECTORPENSIZE              escape type
#define U_MFE_GETVECTORBRUSHSIZE            0x001B   //!< GETVECTORBRUSHSIZE            escape type
#define U_MFE_ENABLEDUPLEX                  0x001C   //!< ENABLEDUPLEX                  escape type
#define U_MFE_GETSETPAPERBINS               0x001D   //!< GETSETPAPERBINS               escape type
#define U_MFE_GETSETPRINTORIENT             0x001E   //!< GETSETPRINTORIENT             escape type
#define U_MFE_ENUMPAPERBINS                 0x001F   //!< ENUMPAPERBINS                 escape type
#define U_MFE_SETDIBSCALING                 0x0020   //!< SETDIBSCALING                 escape type
#define U_MFE_EPSPRINTING                   0x0021   //!< EPSPRINTING                   escape type
#define U_MFE_ENUMPAPERMETRICS              0x0022   //!< ENUMPAPERMETRICS              escape type
#define U_MFE_GETSETPAPERMETRICS            0x0023   //!< GETSETPAPERMETRICS            escape type
#define U_MFE_POSTSCRIPT_DATA               0x0025   //!< POSTSCRIPT_DATA               escape type
#define U_MFE_POSTSCRIPT_IGNORE             0x0026   //!< POSTSCRIPT_IGNORE             escape type
#define U_MFE_GETDEVICEUNITS                0x002A   //!< GETDEVICEUNITS                escape type
#define U_MFE_GETEXTENDEDTEXTMETRICS        0x0100   //!< GETEXTENDEDTEXTMETRICS        escape type
#define U_MFE_GETPAIRKERNTABLE              0x0102   //!< GETPAIRKERNTABLE              escape type
#define U_MFE_EXTTEXTOUT                    0x0200   //!< EXTTEXTOUT                    escape type
#define U_MFE_GETFACENAME                   0x0201   //!< GETFACENAME                   escape type
#define U_MFE_DOWNLOADFACE                  0x0202   //!< DOWNLOADFACE                  escape type
#define U_MFE_METAFILE_DRIVER               0x0801   //!< METAFILE_DRIVER               escape type
#define U_MFE_QUERYDIBSUPPORT               0x0C01   //!< QUERYDIBSUPPORT               escape type
#define U_MFE_BEGIN_PATH                    0x1000   //!< BEGIN_PATH                    escape type
#define U_MFE_CLIP_TO_PATH                  0x1001   //!< CLIP_TO_PATH                  escape type
#define U_MFE_END_PATH                      0x1002   //!< END_PATH                      escape type
#define U_MFE_OPEN_CHANNEL                  0x100E   //!< OPEN_CHANNEL                  escape type
#define U_MFE_DOWNLOADHEADER                0x100F   //!< DOWNLOADHEADER                escape type
#define U_MFE_CLOSE_CHANNEL                 0x1010   //!< CLOSE_CHANNEL                 escape type
#define U_MFE_POSTSCRIPT_PASSTHROUGH        0x1013   //!< POSTSCRIPT_PASSTHROUGH        escape type
#define U_MFE_ENCAPSULATED_POSTSCRIPT       0x1014   //!< ENCAPSULATED_POSTSCRIPT       escape type
#define U_MFE_POSTSCRIPT_IDENTIFY           0x1015   //!< POSTSCRIPT_IDENTIFY           escape type
#define U_MFE_POSTSCRIPT_INJECTION          0x1016   //!< POSTSCRIPT_INJECTION          escape type
#define U_MFE_CHECKJPEGFORMAT               0x1017   //!< CHECKJPEGFORMAT               escape type
#define U_MFE_CHECKPNGFORMAT                0x1018   //!< CHECKPNGFORMAT                escape type
#define U_MFE_GET_PS_FEATURESETTING         0x1019   //!< GET_PS_FEATURESETTING         escape type
#define U_MFE_MXDC_ESCAPE                   0x101A   //!< MXDC_ESCAPE                   escape type
#define U_MFE_SPCLPASSTHROUGH2              0x11D8   //!< SPCLPASSTHROUGH2              escape type
/** @} */

/** MetafileType Enumeration               WMF manual 2.1.1.18
  @{
*/
#define U_MEMORYMETAFILE  0x0001 //!< memory metafile (never used by libUWMF) 
#define U_DISKMETAFILE    0x0002 //!< disk metafile (always used by libUWMF)
/** @} */

/** MetafileVersion Enumeration            WMF manual 2.1.1.19
  @{
*/

#define U_METAVERSION100  0x0100 //!< DIBs not allowed
#define U_METAVERSION300  0x0300 //!< DIBs allowed
/** @} */

/* MixMode Enumeration                    WMF manual 2.1.1.20
    Same as "EMF BackgroundMode Enumeration" in uemf.h
*/

/* OutPrecision Enumeration               WMF manual 2.1.1.21
    Same as "EMF LF_OutPrecision Enumeration" in uemf.h
*/

/** PaletteEntryFlag Enumeration           WMF manual 2.1.1.22
  @{
*/
#define U_PC_RESERVED    0x01 //!< used for animation
#define U_PC_EXPLICIT    0x02 //!< low order word is palette index
#define U_PC_NOCOLLAPSE  0x04 //!< store as new color in palette, do not match to existing color
/** @} */

/** PenStyle Enumeration                   WMF manual 2.1.1.23
    Same as "EMF PenStyle Enumeration" in uemf.h,
    EXCEPT no values >0xFFFF are used, in particular there is no U_PS_GEOMETRIC (ie, all are U_PS_COSMETIC).
    Apparently because there is no U_PS_GEOMETRIC, U_PS_JOIN* and U_PS_ENDCAP* are also ignored by XP SP3 Preview
    (which defaults to a rounded cap) and PowerPoint 2003 (which defaults to square cap).  The behavior
    was the same when escape records for JOIN and ENDCAP are used.  Bottom line, WMF line formatting seems
    to be very hit and miss from application to application.
*/

/* PitchFont Enumeration                  WMF manual 2.1.1.24
    These are only used in PitchAndFamily object, defined there.
*/

/* PolyFillMode Enumeration               WMF manual 2.1.1.25
    These are the first two emtries in "EMF PolygonFillMode Enumeration" in uemf.h

*/

/** PostScriptCap Enumeration              WMF manual 2.1.1.26
   These are used in Escape Cap
  @{
*/
#define U_WPS_CAP_NOTSET -2
#define U_WPS_CAP_FLAT    0
#define U_WPS_CAP_ROUND   1
#define U_WPS_CAP_SQUARE  2
/** @} */

/* PostScriptClipping Enumeration         WMF manual 2.1.1.27
    PostFeatureSetting Enumeration         WMF manual 2.1.1.28

    These are used by postscript drivers, not supported by libUWEMF.
*/

/**     PostScrioptJoin Enumeration            WMF manual 2.1.1.29
   These are used in Escape Cap
  @{
*/
#define U_WPS_JOIN_NOTSET -2
#define U_WPS_JOIN_MITER   0
#define U_WPS_JOIN_ROUND   1
#define U_WPS_JOIN_BEVEL   2
/** @} */

/* StretchMode Enumeration                WMF manual 2.1.1.30
    Same as "EMF StretchMode Enumeration" in uemf.h

*/

/* TernaryRasterOperation  Enumeration    WMF manual 2.1.1.31
    Same as "EMF Ternary Raster Operation Enumeration" in uemf.h
    Only partially supported in libUWMF.h    
*/

/* ClipPrecision Flags                    WMF manual 2.1.2.1
    Same as "EMF LF_ClipPrecision Enumeration" in uemf.h
*/

/* ExtTextOutOptions Flags                WMF manual 2.1.2.2
    These are a subset of  "EMF ExtTextOutOptions Enumeration"  in uemf.h

    Not defined for WMF: U_ETO_NONE, U_ETO_GRAYED, U_ETO_NORECT,
       U_ETO_SMALL_CHARS,U_ETO_IGNORELANGUAGE,U_ETO_REVERSE_INDEX_MAP

    Defined for WMF: U_ETO_OPAQUE, U_ETO_CLIPPED, U_ETO_GLYPH_INDEX,      
       U_ETO_RTLREADING,_ETO_NUMERICSLOCAL,U_ETO_NUMERICSLATIN,   
       U_ETO_PDY
*/

/*  TextAlignment Enumeration              WMF manual 2.1.2.3
    VertialTextAlignment Enumeration       WMF manual 2.1.2.4
    These are both in  "EMF TextAlignment Enumeration"  in uemf.h
*/



//  ***************************************************************************
//  Miscellaneous Values
/*  TextAlignmentMode Flags                WMF manual 2.1.2.3
    VerticalTextAlignmentMode Flags        WMF manual 2.1.2.4
    Same as "EMF TextAlignment Enumeration" in uemf.h
*/

/** \defgroup U_WMF_MinimumRecord_sizes WMF Size in bytes of core record types.

    The size of the object/record is USUALLY not the same
    as the sizeof() of the corresponding struct, so in general it is unsafe to use sizeof() with this code.

    Always use the U_SIZE_x instead!!!!  

    Note that some records may actually be much, much longer than their minimum size as they include strings,
    bitmaps, and such.
    
        Documentation for each value is: 
            + = same as struct size
               or
            X = different from struct size
               followed by
            Number (sizeof(struct) == size of the struct in bytes.)
    
  @{
*/
#define U_SIZE_PAIRF                                  8         /**< +    8 this might be different on 64 bit platform */
#define U_SIZE_COLORREF                               4         /**< +    4 */
#define U_SIZE_BRUSH                                  8         /**< +    8 */
#define U_SIZE_FONT                                  19         /**< X   20 */
#define U_SIZE_FONT_CORE                             18         /**< X   20 Like U_FONT, but minus the FaceName part */
#define U_SIZE_PLTNTRY                                4         /**< +    4 */
#define U_SIZE_PALETTE                                8         /**< +    8 */
#define U_SIZE_PEN                                   10         /**< +   10 */
#define U_SIZE_POINT16                                4         /**< +    4 */
#define U_SIZE_RECT16                                 8         /**< +    8 */
#define U_SIZE_REGION                                20         /**< X   22   20 is minums the variable part */
#define U_SIZE_BITMAP16                              10         /**< +   10 */
#define U_SIZE_BITMAPCOREHEADER                      12         /**< +   12 */
#define U_SIZE_BITMAPINFOHEADER                      40         /**< +   40 */
#define U_SIZE_BITMAPV4HEADER                       108         /**< ?  108 not tested */
#define U_SIZE_BITMAPV5HEADER                       124         /**< ?  124 not tested */
#define U_SIZE_WLOGBRUSH                              8         /**< +    8 */
#define U_SIZE_POLYPOLYGON                            4         /**< +    4 */
#define U_SIZE_SCAN                                   8         /**< +    8 */
#define U_SIZE_METARECORD                             6         /**< X    8 */
#define U_SIZE_WMRPLACEABLE                          22         /**< X   24 */
#define U_SIZE_WMRHEADER                             18         /**< X   20 */
#define U_SIZE_WMREOF                                 6         /**< X    8 */
#define U_SIZE_WMRSETRELABS                           6         /**< X    8 */
#define U_SIZE_WMRSAVEDC                              6         /**< X    8 */
#define U_SIZE_WMRRESTOREDC                           8         /**< *    8 */
#define U_SIZE_WMRREALIZEPALETTE                      6         /**< X    8 */
#define U_SIZE_WMRSETBKCOLOR                         10         /**< X   12 */
#define U_SIZE_WMRSETTEXTCOLOR                       10         /**< X   12 */
#define U_SIZE_WMRSETBKMODE                           8         /**< X   12 last 2 bytes are optional */
#define U_SIZE_WMRSETROP2                             8         /**< X   12 last 2 bytes are optional */
#define U_SIZE_WMRSETPOLYFILLMODE                     8         /**< X   12 last 2 bytes are optional */
#define U_SIZE_WMRSETSTRETCHBLTMODE                   8         /**< X   12 last 2 bytes are optional */
#define U_SIZE_WMRSETTEXTALIGN                        8         /**< X   12 last 2 bytes are optional */
#define U_SIZE_WMRSETMAPMODE                          8         /**< +    8 */
#define U_SIZE_WMRSETTEXTCHAREXTRA                    8         /**< +    8 */
#define U_SIZE_WMRSETTEXTJUSTIFICATION               10         /**< X   12 */
#define U_SIZE_WMRSETWINDOWORG                       10         /**< X   12 */
#define U_SIZE_WMRSETWINDOWEXT                       10         /**< X   12 */
#define U_SIZE_WMRSETVIEWPORTORG                     10         /**< X   12 */
#define U_SIZE_WMRSETVIEWPORTEXT                     10         /**< X   12 */
#define U_SIZE_WMROFFSETWINDOWORG                    10         /**< X   12 */
#define U_SIZE_WMROFFSETVIEWPORTORG                  10         /**< X   12 */
#define U_SIZE_WMRLINETO                             10         /**< X   12 */
#define U_SIZE_WMRMOVETO                             10         /**< X   12 */
#define U_SIZE_WMROFFSETCLIPRGN                      10         /**< X   12 */
#define U_SIZE_WMRSCALEWINDOWEXT                     14         /**< X   16 */
#define U_SIZE_WMRSCALEVIEWPORTEXT                   14         /**< X   16 */
#define U_SIZE_WMREXCLUDECLIPRECT                    14         /**< X   16 */
#define U_SIZE_WMRINTERSECTCLIPRECT                  14         /**< X   16 */
#define U_SIZE_WMRARC                                22         /**< X   24 */
#define U_SIZE_WMRELLIPSE                            14         /**< X   16 */
#define U_SIZE_WMRRECTANGLE                          14         /**< X   16 */
#define U_SIZE_WMRFLOODFILL                          16         /**< +   16 */
#define U_SIZE_WMREXTFLOODFILL                       16         /**< +   16 */
#define U_SIZE_WMRSETPIXEL                           14         /**< X   16 */
#define U_SIZE_WMRPIE                                22         /**< X   24 */
#define U_SIZE_WMRCHORD                              22         /**< X   24 */
#define U_SIZE_WMRROUNDRECT                          18         /**< X   20 */
#define U_SIZE_WMRPATBLT                             18         /**< X   20 */
#define U_SIZE_WMRTEXTOUT                             8         /**< X   12 (not including String,y,x) */
#define U_SIZE_WMRBITBLT_NOPX                        24         /**< +   24 */
#define U_SIZE_WMRBITBLT_PX                          22         /**< X   32 */
#define U_SIZE_WMRSTRETCHBLT_NOPX                    28         /**< +   28 */
#define U_SIZE_WMRSTRETCHBLT_PX                      26         /**< X   36 */
#define U_SIZE_WMRPOLYGON                            10         /**< X   12 */
#define U_SIZE_WMRPOLYLINE                           10         /**< X   12 */
#define U_SIZE_WMRESCAPE                             10         /**< X   12 Data field could be completely absent */
#define U_SIZE_WMRFILLREGION                         10         /**< X   12 */
#define U_SIZE_WMRFRAMEREGION                        14         /**< X   16 */
#define U_SIZE_WMRINVERTREGION                        8         /**< +    8 */
#define U_SIZE_WMRPAINTREGION                         8         /**< +    8 */
#define U_SIZE_WMRSELECTCLIPREGION                    8         /**< +    8 */
#define U_SIZE_WMRSELECTOBJECT                        8         /**< +    8 */
#define U_SIZE_WMRSELECTPALETTE                       8         /**< +    8 */
#define U_SIZE_WMRRESIZEPALETTE                       8         /**< +    8 */
#define U_SIZE_WMRDELETEOBJECT                        8         /**< +    8 */
#define U_SIZE_WMRDRAWTEXT                            6         /**< X    8 */
#define U_SIZE_WMRCREATEBITMAPINDIRECT                6         /**< X    8 */
#define U_SIZE_WMRCREATEBITMAP                        6         /**< X    8 */
#define U_SIZE_WMRSETMAPPERFLAGS                     10         /**< X   12 */
#define U_SIZE_WMREXTTEXTOUT                         14         /**< X   16 */
#define U_SIZE_WMRSETDIBTODEV                        22         /**< X   28 */
#define U_SIZE_WMRANIMATEPALETTE                     14         /**< X   16 */
#define U_SIZE_WMRSETPALENTRIES                      14         /**< X   16 */
#define U_SIZE_WMRCREATEPALETTE                      14         /**< X   16 */
#define U_SIZE_WMRPOLYPOLYGON                        10         /**< X   12 */
#define U_SIZE_WMRDIBBITBLT_NOPX                     24         /**< +   24 */
#define U_SIZE_WMRDIBBITBLT_PX                       22         /**< X   24 */
#define U_SIZE_WMRDIBSTRETCHBLT_NOPX                 28         /**< +   28 */
#define U_SIZE_WMRDIBSTRETCHBLT_PX                   26         /**< X   28 */
#define U_SIZE_WMRDIBCREATEPATTERNBRUSH              10         /**< X   12 */
#define U_SIZE_WMRSTRETCHDIB                         28         /**< X   32 */
#define U_SIZE_WMRCREATEPATTERNBRUSH                  6         /**< X    8 */
#define U_SIZE_WMRCREATEPENINDIRECT                  16         /**< +   16 */
#define U_SIZE_WMRCREATEFONTINDIRECT                 26         /**< X   28 */
#define U_SIZE_WMRCREATEBRUSHINDIRECT                14         /**< X   16 */
#define U_SIZE_WMRCREATEREGION                       26         /**< X   28 */
#define U_SIZE_WMRCREATEREGION_CORE                  24         /**< X   28 Like U_SIZE_WMRCREATEREGION minus the variable part */
/** @} */


//  ***************************************************************************
//  Macros

/** \defgroup U_WMF_Common_macros WMF Common Macros
Because Size16_4 may not be aligned no tests should dereference it directly from a pointer. 
in NOPX tests cast causes uint8_t to promote to uint32_t, without it c++ compiler complains about
comparison of int with unsigned int
  @{
*/
#define U_TEST_NOPX2(A,B) (A ==   (uint32_t) (B + 3))          //!< A is Size16_4 (extracted and aligned),   B = xb  true if no bitmap associated with the structure, used with some BLT records.
#define U_TEST_NOPXB(A,B) (A/2 == (uint32_t) (B + 3))          //!< A is Size16_4 (extracted and aligned)*2, B = xb, true if no bitmap associated with the structure, used with some BLT records.
#define U_WMRTYPE(A) (((U_METARECORD *)A)->iType)              //!< Get iType from U_WMR* record.
#define U_WMRXB(A)   (((U_METARECORD *)A)->xb)                 //!< Get xb from U_WMR* record.
#define U_WMR_XB_FROM_TYPE(A) ((uint8_t) (U_wmr_values(A)>>8)) //!< Get xb from type value.
#define U_U16(A)  (*(uint16_t *)&A)                            //!< interpret a 16 bit type as uint16_t.
#define U_P16(A)  ( (uint16_t *)&A)                            //!< pass any 16 bit type as a pointer to a uint16_t.
#define U_PP16(A) ( (uint16_t *) A)                            //!< pass any pointer to a 16 bit type as a pointer to a uint16_t.

/** @} */

/* ************************************************************
    WMF structures OTHER than those corresponding to complete U_WMR_* records
   ************************************************************ */
   
/** Brush Object                           WMF manual 2.2.1.1

  Documentation is muddy, bColor and bHatch fields have different meanings depending on
  the value of bStyle.  Unclear if bHatch bytes are present in some cases from the
  documentation.

        style                Color                Data
        U_BS_SOLID           ColorRef Object      Not used (bytes present???)
        U_BS_NULL            ignored              ignored  (bytes present???).
        U_BS_PATTERN         ignored              Bitmap16 object holding patern
        U_BS_DIBPATTERNPT    ColorUsage Enum      DIB object
        U_BS_HATCHED         ColorRef Object      HatchStyle Enumeration
*/
typedef struct {
    uint16_t            Style;              //!< BrushStyle Enumeration
    U_COLORREF          Color;              //!< Brush Color value, 32 bit value is not aligned.
    uint8_t             Data[1];            //!< Brush pattern information, variable size and format
} U_BRUSH;
   

/** Font Object                             WMF manual 2.2.1.2
  Warning, only pass by pointer, passing by value will will truncate in Facename!
*/
typedef struct {
    int16_t             Height;             //!< Height in Logical units
    int16_t             Width;              //!< Average Width in Logical units
    int16_t             Escapement;         //!< Angle in 0.1 degrees betweem escapement vector and X axis
    int16_t             Orientation;        //!< Angle in 0.1 degrees between baseline and X axis
    int16_t             Weight;             //!< LF_Weight Enumeration
    uint8_t             Italic;             //!< LF_Italic Enumeration
    uint8_t             Underline;          //!< LF_Underline Enumeration
    uint8_t             StrikeOut;          //!< LF_StrikeOut Enumeration
    uint8_t             CharSet;            //!< LF_CharSet Enumeration
    uint8_t             OutPrecision;       //!< LF_OutPrecision Enumeration
    uint8_t             ClipPrecision;      //!< LF_ClipPrecision Enumeration
    uint8_t             Quality;            //!< LF_Quality Enumeration
    uint8_t             PitchAndFamily;     //!< LF_PitchAndFamily Enumeration
    uint8_t             FaceName[1];        //!< Name of font.  ANSI Latin1, null terminated.
} U_FONT;
 
/**   PaletteEntry Object                  WMF manual 2.2.2.13
   Note, NOT compatiable with U_LOGPLTNTRY
   Out of PDF order because needed for next struture.
*/
typedef struct {
    uint8_t             Value;              //!< 0 or PaletteEntryFlag Enumeration
    uint8_t             Blue;               //!< Palette entry Blue Intensity
    uint8_t             Green;              //!< Palette entry Green Intensity
    uint8_t             Red;                //!< Palette entry Red Intensity
} U_PLTNTRY;

/**  Palette Object                         WMF manual 2.2.1.3
  NOT Same as "EMF LogPalette Object" in uemf.h because Palette Entries have reversed colors.
  Values for palVersion are expanded
  
  Start must be 0x0300 (as for EMF) with U_WMRCREATEPALETTE but is an offset
  for U_WMRSETPALENTRIES and U_ANIMATEPALETTE
*/
typedef struct { 
    uint16_t            Start;              //!< Either 0x0300 or an offset into the Palette table
    uint16_t            NumEntries;         //!< Number of U_LOGPLTNTRY objects
    U_PLTNTRY           PalEntries[1];      //!< Array of PaletteEntry Objects
} U_PALETTE;

/**  Pen Object                             WMF manual 2.2.1.4
*/
typedef struct {
    uint16_t            Style;              //!< PenStyle Enumeration
    uint16_t            Widthw[2];          //!< reassemble/store the Pen Width in object dimensions using Widthw, the 32 bit value is not aligned
    U_COLORREF          Color;              //!< Pen Color, the 32 bit value is not aligned.
} U_PEN;

/**   Rect Object                          WMF manual 2.2.2.18
   \brief Coordinates of the upper left, lower right corner.
   Note that the coordinate system is 0,0 in the upper left corner
   of the screen an N,M in the lower right corner.
   Microsoft name: RECT Object COLLIDES with EMF Rect Object.
   
   This one is out of order because it is needed early.
*/
typedef struct {
    int16_t  left;                          //!< left coordinate
    int16_t  top;                           //!< top coordinate
    int16_t  right;                         //!< right coordinate
    int16_t  bottom;                        //!< bottom coordinate
} U_RECT16;

#define U_RCL16_DEF (U_RECT16){0,0,-1,-1}  //!< Use this when no bounds are needed. 

/**  Region Object                         WMF manual 2.2.1.5
*/
typedef struct {
    uint16_t            ignore1;            //!< unused value
    uint16_t            Type;               //!< must be 0x0006.
    uint16_t            ignore2;            //!< unused value
    int16_t             Size;               //!< aScans in bytes + regions size in bytes (size of this header plus all U_SCAN objects?)
    int16_t             sCount;             //!< number of scanlines in region
    int16_t             sMax;               //!< largest number of points in any scan
    U_RECT16            sRect;              //!< bounding rectangle
    uint16_t            aScans[1];          //!< series of appended U_SCAN objects
} U_REGION;

/**   Bitmap16 Object                      WMF manual 2.2.2.1
      
        The U_BITMAP16 core is always followed by 
        uint8_t             Bits[1];           //!<  bitmap pixel data. Bytes contained = (((Width * BitsPixel + 15) >> 4) << 1) * Height
        Note that in U_WMRCREATEPATTERNBRUSH Bits is always [4].
      
*/
typedef struct {
    int16_t             Type;               //!<  "bitmap type"  MS PDF does not define this field beyond this.
    int16_t             Width;              //!<  bitmap width in pixels.
    int16_t             Height;             //!<  bitmap height in scan lines.
    int16_t             WidthBytes;         //!<  bytes per scan line.
    uint8_t             Planes;             //!<  must be 1.
    uint8_t             BitsPixel;          //!<  number of adjacent color bits on each plane (R bits + G bits + B bits ????)
} U_BITMAP16;

/**   BitmapCoreHeader Object              WMF manual 2.2.2.2
*/
typedef struct {
    uint16_t            Size_4[2];          //!<  size of U_BITMAPCOREHEADER in bytes.
    uint16_t            Width;              //!<  DIB width in pixels.
    uint16_t            Height;             //!<  DIB height in pixels.
    uint16_t            Planes;             //!<  must be 1
    uint16_t            BitCount;           //!<  Pixel Format (BitCount Enumeration)
} U_BITMAPCOREHEADER;


/**   BitmapInfoHeader Object              WMF manual 2.2.2.3
  Same as "EMF BITMAPINFOHEADER Object" in uemf.h
  use U_BITMAPINFOHEADER
*/

//! \cond
/**   BitmapV4Header Object                WMF manual 2.2.2.4
*/
typedef struct {
    uint32_t            bV4Size;
    int32_t             bV4Width;
    int32_t             bV4Height;
    uint16_t            bV4Planes;
    uint16_t            bV4BitCount;
    uint32_t            bV4Compression;
    uint32_t            bV4SizeImage;
    int32_t             bV4XPelsPerMeter;
    int32_t             bV4YPelsPerMeter;
    uint32_t            bV4ClrUsed;
    uint32_t            bV4ClrImportant;
    uint32_t            bV4RedMask;
    uint32_t            bV4GreenMask;
    uint32_t            bV4BlueMask;
    uint32_t            bV4AlphaMask;
    uint32_t            bV4CSType;
    U_CIEXYZTRIPLE      bV4EndPoints;
    uint32_t            bV4GammaRed;
    uint32_t            bV4GammaGreen;
    uint32_t            bV4GammaBlue;
} U_BITMAPV4HEADER;  //!< For ?


/**   BitmapV5Header Object                WMF manual 2.2.2.5
*/
typedef struct {
    uint32_t            bV5Size;
    int32_t             bV5Width;
    int32_t             bV5Height;
    uint16_t            bV5Planes;
    uint16_t            bV5BitCount;
    uint32_t            bV5Compression;
    uint32_t            bV5SizeImage;
    int32_t             bV5XPelsPerMeter;
    int32_t             bV5YPelsPerMeter;
    uint32_t            bV5ClrUsed;
    uint32_t            bV5ClrImportant;
    uint32_t            bV5RedMask;
    uint32_t            bV5GreenMask;
    uint32_t            bV5BlueMask;
    uint32_t            bV5AlphaMask;
    uint32_t            bV5CSType;
    U_CIEXYZTRIPLE      bV5Endpoints;
    uint32_t            bV5GammaRed;
    uint32_t            bV5GammaGreen;
    uint32_t            bV5GammaBlue;
    uint32_t            bV5Intent;
    uint32_t            bV5ProfileData;
    uint32_t            bV5ProfileSize;
    uint32_t            bV5Reserved;
} U_BITMAPV5HEADER;  //!< For ?
//! \endcond



/**   CIEXYZ Object                        WMF manual 2.2.2.6
    Same as "EMF CIEXYZ Object" in uemf.h
*/

/**   CIEXYZTriple Object                  WMF manual 2.2.2.7
    Same as "EMF CIEXYZTRIPLE Object" in uemf.h
*/

/**   ColorRef Object                      WMF manual 2.2.2.8
    Same as "EMF COLORREF Object" in uemf.h
*/

/**   DeviceIndependentBitmap Object       WMF manual 2.2.2.9
This "object" has an organization, but not one that can be easily expressed with a C struct.  It consists of
three parts, all of which have variable size:

        DIBHeaderInfo  BitmapCoreHeader or BitmapInfoHeader Object
        Colors         Array of RGBQuad Objects or uint16_t that make a color table, as determined from the DIBHeaderInfo field.
        BitMapBuffer   Array of bytes containing the image.

*/

/**   WLogBrush Object                      WMF manual 2.2.2.10
  Not compatible with EMF LogBrush object!

        style                Color                Hatch
        U_BS_SOLID           ColorRef Object      Not used (bytes present???)
        U_BS_NULL            ignored              ignored  (bytes present???).
        U_BS_PATTERN         ignored              not used     (Action is not strictly defined)
        U_BS_DIBPATTERN      ignored              not used     (Action is not strictly defined)
        U_BS_DIBPATTERNPT    ignored              not used     (Action is not strictly defined)
        U_BS_HATCHED         ColorRef Object      HatchStyle Enumeration
*/
typedef struct {
    uint16_t            Style;              //!< BrushStyle Enumeration
    U_COLORREF          Color;              //!< Brush Color value, 32 bit value is not aligned.
    uint16_t            Hatch;              //!< HatchStyle Enumeration
} U_WLOGBRUSH;

/*   LogColorSpace Object                 WMF manual 2.2.2.11
    Same as "EMF LOGCOLORSPACEA Object" in uemf.h
    use U_LOGCOLORSPACEA
*/

/*   LogColorSpaceW Object                WMF manual 2.2.2.12
    Same as "EMF LOGCOLORSPACEW Object" in uemf.h
    use U_LOGCOLORSPACEW
*/


/*   PaletteEntry Object                  WMF manual 2.2.2.13
    moved up before Palette Object */

/* PitchAndFamily Enumerations            WMF manual 2.2.2.14
    Same as "EMF LF_PitchAndFamily Enumeration" in uemf.h
*/

/*   PointL Object                        WMF manual 2.2.2.15
    Same as "EMF Point Object" in uemf.h
*/

/*   PointS Object                        WMF manual 2.2.2.16
    Same as "EMF POINTS Object" in uemf.h
*/

/*   PolyPolygon Object                   WMF manual 2.2.2.17 */
/** WMF manual 2.2.2.17

  There is an array "aPoints" of uint16_t after aPolyCounts that holds the coordinates.

  Presumably it is in order [x1,y1],[x2,y2],etc.  The documentation does not say, it might have
  y then x.

  aPoints starts at aPolyCounts[nPolys]
*/
typedef struct {
    uint16_t            nPolys;             //!< Number of polygons
    uint16_t            aPolyCounts[1];     //!< Number of points in each polygon (sequential)
} U_POLYPOLYGON;

/*   Rect Object                          WMF manual 2.2.2.18
     This one is out of order, had to be created much earlier than this
*/

/*   RectL Object                         WMF manual 2.2.2.19
    Same as "EMF RECT Object" in uemf.h
*/

/*   RGBQuad Object                       WMF manual 2.2.2.20
    Same as "EMF RGBQUAD Object" in uemf.h
*/

/**   Scan Object                          WMF manual 2.2.2.21 */
/** WMF manual 2.2.2.21

      Mandatory field "count2" must follow ScanLines, but it cannot be placed into the struct because
      ScanLines has variable size.  "count2" is 
      an uint16_t value which must have the same value as count.
*/
typedef struct {
    uint16_t  count;                         //!< Number of entries in the ScanLines array
    uint16_t  top;                           //!< Y coordinate of the top scanline
    uint16_t  bottom;                        //!< Y coordinate of the bottom scanline
    uint16_t  ScanLines[1];                  //!< Array of 16 bit left/right pairs
} U_SCAN;

/**   SizeL Object                         WMF manual 2.2.2.22
    Same as "EMF SIZEL Object" in uemf.h
*/


/** First three fields of MOST WMF records (not WMR_HEADER and WMR_PLACEABLE!)

    This Sshould only used for accessing size and type fields.  
    It is NOT used as a prefix like U_EMR in uemf.h because it may cause alignment issues.
    Microsoft name: WMF Object
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
} U_METARECORD;

/** WMF manual 2.3.2.3 META_PLACEABLE
        If present this must immediately precede the header.  
        It is not enumerated as an WMR record type.
        This only ever occurs at the start of a WMF file, so the two uint32_t values will always be aligned.
*/
typedef struct {
    uint32_t            Key;                //!< MUST be 0x9AC6CDD7
    uint16_t            HWmf;               //!< 0.  (Always.  Manual says total number of 16bit words in record, but no examples found like that)
    U_RECT16            Dst;                //!< Destination bounding box in logical units
    uint16_t            Inch;               //!< Logical units/inch (convention if not specified:  1440 logical units/inch)
    uint32_t            Reserved;           //!< must be 0
    uint16_t            Checksum;           //!< Checksum of preceding 10 16 bit values
} U_WMRPLACEABLE;

/** WMF manual 2.3.2.2 META_HEADER 
*/
typedef struct {
    uint8_t             iType;              //!< RecordType Enumeration, must be 1
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            Size16w;            //!< Total number of 16bit words in record
    uint16_t            version;            //!< Metafile version Enumeration
    uint16_t            Sizew[2];           //!< reassemble/store the Size (16 bit words in entire file) using Sizew, the 32 bit value is not aligned
    uint16_t            nObjects;           //!< Total number of brushes, pens, and other graphics objects defined in this file
    uint32_t            maxSize;            //!< Largest record in file, in number of 16bit words (This uint32_t is aligned)
    uint16_t            nMembers;           //!< Unused, should be 0
} U_WMRHEADER;


// ***********************************************************************************
// The following structures correspond to U_WMR_# records

/* Index 00 U_WMREOF                        WMF manual 2.3.2.1 META_EOF */
/** WMF manual 2.3.2.1 META_EOF
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
} U_WMREOF,
  U_WMRSETRELABS,                           //!< WMF manual 2.3.5.21
  U_WMRSAVEDC,                              //!< WMF manual 2.3.5.11
  U_WMRREALIZEPALETTE;                      //!< WMF manual 2.3.5.8

/* Index 01 U_WMRSETBKCOLOR                 WMF manual 2.3.5.14 */
/** WMF manual 2.3.5.14
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    U_COLORREF          Color;              //!< Color value, the 32 bit value is not aligned.
} U_WMRSETBKCOLOR,
  U_WMRSETTEXTCOLOR;                        //!< WMF manual 2.3.5.26

/* Index 02 U_WMRSETBKMODE                  WMF manual 2.3.5.15 */
/** WMF manual 2.3.5.15
mode = MixMode Enumeration.
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            Mode;               //!< Various Enumeraton.
    uint16_t            Reserved;           //!< Ignore (ALSO OPTIONAL - FIELD MAY NOT BE PRESENT!!!!)
} U_WMRSETBKMODE,
  U_WMRSETPOLYFILLMODE,                     //!< WMF manual 2.3.5.20 Mode = PolyFillMode Enumeration.
  U_WMRSETROP2,                             //!< WMF manual 2.3.5.22 Binary Raster Operation Enumeration.
  U_WMRSETSTRETCHBLTMODE,                   //!< WMF manual 2.3.5.23 Mode = StretchMode Enumeration
  U_WMRSETTEXTALIGN;                        //!< WMF manual 2.3.5.24 Mode = TextAlignment Enumeration.

/* Index 03 U_WMRSETMAPMODE                 WMF manual 2.3.5.17 */
/** WMF manual 2.3.5.17
Mode = MapMode Enumeration.
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            Mode;               //!< Various Enumeraton and other
} U_WMRSETMAPMODE,
  U_WMRSETTEXTCHAREXTRA;                    //!< WMF manual 2.3.5.25, Mode = Extra space in logical units to add to each character

/* Index 04 U_WMRSETROP2                    WMF manual 2.3.5.22  See Index 02 */

/* Index 05 U_WMRSETRELABS                  WMF manual 2.3.5.21  See Index 00*/

/* Index 06 U_WMRSETPOLYFILLMODE            WMF manual 2.3.5.20 See Index 02
   Index 07 U_WMRSETSTRETCHBLTMODE          WMF manual 2.3.5.23 */

/* Index 08 U_WMRSETTEXTCHAREXTRA           WMF manual 2.3.5.25 See Index 03*/
   
/* Index 09 U_WMRSETTEXTCOLOR               WMF manual 2.3.5.26 see Index 01 */

/* Index 0A U_WMRSETTEXTJUSTIFICATION       WMF manual 2.3.5.27 */
/** WMF manual 2.3.5.27
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            Count;              //!< Number of space characters in the line
    uint16_t            Extra;              //!< Number of extra space characters to add to the line
} U_WMRSETTEXTJUSTIFICATION;

/* Index 0B U_WMRSETWINDOWORG               WMF manual 2.3.5.31
   Index 0C U_WMRSETWINDOWEXT               WMF manual 2.3.5.30
   Index 0D U_WMRSETVIEWPORTORG             WMF manual 2.3.5.29
   Index 0E U_WMRSETVIEWPORTEXT             WMF manual 2.3.5.28
   Index 0F U_WMROFFSETWINDOWORG            WMF manual 2.3.5.7
   Index 0F U_WMROFFSETVIEWPORTORG          WMF manual 2.3.5.6
   Index 13 U_WMRLINETO                     WMF manual 2.3.3.10
   Index 14 U_WMRMOVETO                     WMF manual 2.3.3.4
   Index 20 U_WMROFFSETCLIPRGN              WMF manual 2.3.5.5
*/
/** WMF manual 2.3.5.31
Window X,Y origin
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    int16_t             y;                  //!< Y value (note order!)
    int16_t             x;                  //!< X value
} U_WMRSETWINDOWORG,
  U_WMRSETWINDOWEXT,                        //!< WMF manual 2.3.5.30, Window X,Y extent
  U_WMRSETVIEWPORTORG,                      //!< WMF manual 2.3.5.29, Viewport X,Y origin
  U_WMRSETVIEWPORTEXT,                      //!< WMF manual 2.3.5.28, Viewport X,Y extent
  U_WMROFFSETWINDOWORG,                     //!< WMF manual 2.3.5.7,  Window X,Y offset in device units
  U_WMROFFSETVIEWPORTORG,                   //!< WMF manual 2.3.5.6,  Viewport X,Y offset in device units
  U_WMRLINETO,                              //!< WMF manual 2.3.3.10, Endpoint X,Y  in logical units
  U_WMRMOVETO,                              //!< WMF manual 2.3.3.4,  Destination X,Y in logical units
  U_WMROFFSETCLIPRGN;                       //!< WMF manual 2.3.5.5,  Y offset in logical units

/* Index 10 U_WMRSCALEWINDOWEXT             WMF manual 2.3.5.13 
   Index 12 U_WMRSCALEVIEWPORTEXT           WMF manual 2.3.5.12
*/
/** WMF manual 2.3.5.13
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    int16_t             yDenom;             //!< Y denominator
    int16_t             yNum;               //!< Y numerator
    int16_t             xDenom;             //!< X denominator
    int16_t             xNum;               //!< X numerator
} U_WMRSCALEWINDOWEXT,
  U_WMRSCALEVIEWPORTEXT;                    //!< WMF manual 2.3.5.12

/* Index 11 U_WMROFFSETVIEWPORTORG          WMF manual 2.3.5.6  see Index 0B */

/* Index 12 U_WMRSCALEVIEWPORTEXT           WMF manual 2.3.5.12 see Index 10 */
   
/* Index 13 U_WMRLINETO                     WMF manual 2.3.3.10 see index 0B
   Index 14 U_WMRMOVETO                     WMF manual 2.3.5.4  */

/* Index 15 U_WMREXCLUDECLIPRECT            WMF manual 2.3.5.2
   Index 16 U_WMRINTERSECTCLIPRECT          WMF manual 2.3.5.3 
*/
/** WMF manual 2.3.5.2
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    int16_t             Bottom;             //!< Coordinates in logical units
    int16_t             Right;              //!< Coordinates in logical units
    int16_t             Top;                //!< Coordinates in logical units
    int16_t             Left;               //!< Coordinates in logical units
} U_WMREXCLUDECLIPRECT,    
  U_WMRINTERSECTCLIPRECT;                   //!< WMF manual 2.3.5.3

/* Index 17 U_WMRARC                        WMF manual 2.3.3.1  */
/** WMF manual 2.3.3.1
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    int16_t             yEndArc;            //!< Coordinates in logical units
    int16_t             xEndArc;            //!< Coordinates in logical units
    int16_t             yStartArc;          //!< Coordinates in logical units
    int16_t             xStartArc;          //!< Coordinates in logical units
    int16_t             Bottom;             //!< Coordinates in logical units
    int16_t             Right;              //!< Coordinates in logical units
    int16_t             Top;                //!< Coordinates in logical units
    int16_t             Left;               //!< Coordinates in logical units
} U_WMRARC;  

/* Index 18 U_WMRELLIPSE                    WMF manual 2.3.3.3  
   Index 1B U_WMRRECTANGLE                  WMF manual 2.3.3.17
*/
/** WMF manual 2.3.3.3
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    int16_t             Bottom;             //!< Coordinates in logical units
    int16_t             Right;              //!< Coordinates in logical units
    int16_t             Top;                //!< Coordinates in logical units
    int16_t             Left;               //!< Coordinates in logical units
} U_WMRELLIPSE,
  U_WMRRECTANGLE;                           //!< WMF manual 2.3.3.17

/* Index 19 U_WMRFLOODFILL                  WMF manual 2.3.3.7
   Index 48 U_WMREXTFLOODFILL               WMF manual 2.3.3.4
*/
/** WMF manual 2.3.3.7
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    int16_t             Mode;               //!< FloodFill Enumeration
    U_COLORREF          Color;              //!< Color
    int16_t             y;                  //!< Y
    int16_t             x;                  //!< X
} U_WMRFLOODFILL,
  U_WMREXTFLOODFILL;                        //!< WMF manual 2.3.3.7

/* Index 1A U_WMRPIE                        WMF manual 2.3.3.13 
   Index 30 U_WMRCHORD                      WMF manual 2.3.3.2
*/
/** WMF manual 2.3.3.13
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    int16_t             yRadial2;           //!< in logical units
    int16_t             xRadial2;           //!< in logical units
    int16_t             yRadial1;           //!< in logical units
    int16_t             xRadial1;           //!< in logical units
    int16_t             Bottom;             //!< in logical units
    int16_t             Right;              //!< in logical units
    int16_t             Top;                //!< in logical units
    int16_t             Left;               //!< in logical units
} U_WMRPIE,
  U_WMRCHORD;                               //!< WMF manual 2.3.3.2

/* Index 1B U_WMRRECTANGLE                  WMF manual 2.3.3.17 See Index 18 */

/* Index 1C U_WMRROUNDRECT                  WMF manual 2.3.3.18 */
/** WMF manual 2.3.3.18
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    int16_t             Height;             //!< in logical units (rounded corner)
    int16_t             Width;              //!< in logical units (rounded corner)
    int16_t             Bottom;             //!< in logical units
    int16_t             Right;              //!< in logical units
    int16_t             Top;                //!< in logical units
    int16_t             Left;               //!< in logical units
} U_WMRROUNDRECT;

/* Index 1D U_WMRPATBLT                     WMF manual 2.3.3.12
*/
/** WMF manual 2.3.3.12
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            rop3w[2];           //!< reassemble/store the rop3 Ternary raster operation using rop3w, as the 32 bit value is not aligned
    int16_t             Height;             //!< in logical units (of Rect to Fill)
    int16_t             Width;              //!< in logical units (of Rect to Fill)
    int16_t             yDst;               //!< in logical units (UL corner to fill)
    int16_t             xDst;               //!< in logical units (UL corner to fill)
} U_WMRPATBLT;

/* Index 1E U_WMRSAVEDC                     WMF manual 2.3.5.11 See Index 00*/

/* Index 1F U_WMRSETPIXEL                   WMF manual 2.3.3.19 */
/** WMF manual 2.3.3.19
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    U_COLORREF          Color;              //!< Color
    int16_t             y;                  //!< Y
    int16_t             x;                  //!< X
} U_WMRSETPIXEL;

/* Index 20 U_WMROFFSETCLIPRGN              WMF manual 2.3.5.5  See Index 0B*/

/* Index 21 U_WMRTEXTOUT                    WMF manual 2.3.3.20
*/
/** WMF manual 2.3.3.20
Also part of the record, following String, and so at variable positions:
        
int16_t             y;                  start position
    
int16_t             x;                  start position
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    int16_t             Length;             //!< Stringlength in bytes
    uint8_t             String;             //!< String to write, storage area must be 2n bytes.
} U_WMRTEXTOUT;

/* Index 22 U_WMRBITBLT                     WMF manual 2.3.1.1
*/
/** WMF manual 2.3.1.1

   This is a variable structure the core/invariant part extends to xSrc.
   
   if RecordSize == ((xb) + 3) then there is no bitmap and use the _NOPX form, otherwise use the _PX form
   Use Macro U_TEST_NOPX2

*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            rop3w[2];           //!< reassemble/store the Ternary raster operation rop3 value using rop3w, the 32 bit value is not aligned.
    int16_t             ySrc;               //!< in logical units (UL corner of Src rect)
    int16_t             xSrc;               //!< in logical units (UL corner of Src rect)
    int16_t             ignore;             //!< ignore
    int16_t             Height;             //!< in logical units (of Src and Dst rects)
    int16_t             Width;              //!< in logical units (of Src and Dst rects)
    int16_t             yDst;               //!< in logical units (UL corner of Dst rect)
    int16_t             xDst;               //!< in logical units (UL corner of Dst rect)
} U_WMRBITBLT_NOPX;

/** WMF manual 2.3.1.1

   This is a variable structure the core/invariant part extends to xSrc.
   
   if RecordSize == ((xb) + 3) then there is no bitmap and use the _NOPX form, otherwise use the _PX form
   Use Macro U_TEST_NOPX2

*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            rop3w[2];           //!< reassemble/store the Ternary raster operation rop3 value using rop3w, the 32 bit value is not aligned.
    int16_t             ySrc;               //!< in logical units (UL corner of Src rect)
    int16_t             xSrc;               //!< in logical units (UL corner of Src rect)
    int16_t             Height;             //!< in logical units (of Src and Dst rects)
    int16_t             Width;              //!< in logical units (of Src and Dst rects)
    int16_t             yDst;               //!< in logical units (UL corner of Dst rect)
    int16_t             xDst;               //!< in logical units (UL corner of Dst rect)
    U_BITMAP16          bitmap;             //!< Src bitmap
} U_WMRBITBLT_PX;


/* Index 23 U_WMRSTRETCHBLT                 WMF manual 2.3.1.5 */
/** WMF manual 2.3.1.5

   This is a variable structure the core/invariant part extends to xSrc.
   
   if RecordSize == ((xb) + 3) then there is no bitmap and use the _NOPX form, otherwise use the _PX form
   Use Macro U_TEST_NOPX2.
*/

typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            rop3w[2];           //!< reassemble/store the Ternary raster operation rop3 value using rop3w, the 32 bit value is not aligned.
    int16_t             hSrc;               //!< Height in logical units of Src rect
    int16_t             wSrc;               //!< Wdith  in logical units of Dst rect
    int16_t             ySrc;               //!< in logical units (UL corner of Src rect)
    int16_t             xSrc;               //!< in logical units (UL corner of Src rect)
    int16_t             ignore;             //!< ignored
    int16_t             hDst;               //!< Height in logical units of Dst rect
    int16_t             wDst;               //!< Wdith  in logical units of Dst rect
    int16_t             yDst;               //!< in logical units (UL corner of Dst rect)
    int16_t             xDst;               //!< in logical units (UL corner of Dst rect)
} U_WMRSTRETCHBLT_NOPX;


/* Index 23 U_WMRSTRETCHBLT                 WMF manual 2.3.1.5 */
/** WMF manual 2.3.1.5

   This is a variable structure the core/invariant part extends to xSrc.
   
   if RecordSize == ((xb) + 3) then there is no bitmap and use the _NOPX form, otherwise use the _PX form
   Use Macro U_TEST_NOPX2.
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            rop3w[2];           //!< reassemble/store the Ternary raster operation rop3 value using rop3w, the 32 bit value is not aligned.
    int16_t             hSrc;               //!< Height in logical units of Src rect
    int16_t             wSrc;               //!< Wdith  in logical units of Dst rect
    int16_t             ySrc;               //!< in logical units (UL corner of Src rect)
    int16_t             xSrc;               //!< in logical units (UL corner of Src rect)
    int16_t             hDst;               //!< Height in logical units of Dst rect
    int16_t             wDst;               //!< Wdith  in logical units of Dst rect
    int16_t             yDst;               //!< in logical units (UL corner of Dst rect)
    int16_t             xDst;               //!< in logical units (UL corner of Dst rect)
    U_BITMAP16          bitmap;             //!< Src bitmap
} U_WMRSTRETCHBLT_PX;

/* Index 24 U_WMRPOLYGON                    WMF manual 2.3.3.15
   Index 25 U_WMRPOLYLINE                   WMF manual 2.3.3.14
*/
/** WMF manual 2.3.3.15
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    int16_t             nPoints;            //!< Number of points in aPoints
    U_POINT16           aPoints[1];         //!< Array of points
} U_WMRPOLYGON,
  U_WMRPOLYLINE;                            //!< WMF manual 2.3.3.14

/* Index 26 U_WMRESCAPE                     WMF manual 2.3.6.1  */
/** WMF manual 2.3.6.1
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            eFunc;              //!< Escape function
    uint16_t            nBytes;             //!< bytes in the data array
    uint8_t             Data[1];            //!< data array
} U_WMRESCAPE;

/* Index 27 U_WMRRESTOREDC                  WMF manual 2.3.5.10 */
/** WMF manual 2.3.5.10
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    int16_t             DC;                 //!< DC to restore (negative is relative to current, positive is absolute)
} U_WMRRESTOREDC;

/* Index 28 U_WMRFILLREGION                 WMF manual 2.3.3.6  */
/**  WMF manual 2.3.3.6
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            Region;             //!< Index of region to fill in object table
    uint16_t            Brush;              //!< Index of brush to use in object table
} U_WMRFILLREGION;

/* Index 29 U_WMRFRAMEREGION                WMF manual 2.3.3.8  */
/**  WMF manual 2.3.3.8
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            Region;             //!< Index of region to frame in object table
    uint16_t            Brush;              //!< Index of brush to use in frame in object table
    int16_t             Height;             //!< in logical units (of frame)
    int16_t             Width;              //!< in logical units (of frame)
} U_WMRFRAMEREGION;

/* Index 2A U_WMRINVERTREGION               WMF manual 2.3.3.9  
   Index 2B U_WMRPAINTREGION                WMF manual 2.3.3.11
   Index 2C U_WMRSELECTCLIPREGION           WMF manual 2.3.4.9 
   Index 2D U_WMRSELECTOBJECT               WMF manual 2.3.4.10
   Index 34 U_WMRSELECTPALETTE              WMF manual 2.3.4.11
   Index 39 U_WMRRESIZEPALETTE              WMF manual 2.3.5.9
   Index F0 U_WMRDELETEOBJECT               WMF manual 2.3.4.7
*/
/**  WMF manual 2.3.3.9
invert region
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            index;              //!< (usually) index of region/object in object table
} U_WMRINVERTREGION,
  U_WMRPAINTREGION,                         //!< WMF manual 2.3.3.11, paint region
  U_WMRSELECTCLIPREGION,                    //!< WMF manual 2.3.4.9,  select as clip region
  U_WMRSELECTOBJECT,                        //!< WMF manual 2.3.4.10, select object
  U_WMRSELECTPALETTE,                       //!< WMF manual 2.3.4.11, select palette object
  U_WMRRESIZEPALETTE,                       //!< WMF manual 2.3.5.9,  resize the system palette to "index"
  U_WMRDELETEOBJECT;                        //!< WMF manual 2.3.4.7,  delete object

/* Index 2E U_WMRSETTEXTALIGN               WMF manual 2.3.5.24  See Index 02 */

/* Index 2F U_WMRDRAWTEXT                   in GDI and Wine, not documented in WMF manual. 
   Index FE U_WMRCREATEBITMAP               in GDI and Wine, not documented in WMF manual. 
   Index FD U_WMRCREATEBITMAPINDIRECT       in GDI and Wine, not documented in WMF manual. 

   no documentation found, this part of these records, at least, must be correct    */
/** in GDI and Wine, not documented in WMF manual.
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
} U_WMRDRAWTEXT,
  U_WMRCREATEBITMAPINDIRECT,                //!< in GDI and Wine, not documented in WMF manual.
  U_WMRCREATEBITMAP;                        //!< in GDI and Wine, not documented in WMF manual.
 
/* Index 30 U_WMRCHORD                      WMF manual 2.3.3.2  See Index 1A */

/* Index 31 U_WMRSETMAPPERFLAGS             WMF manual 2.3.5.18 */
/** WMF manual 2.3.5.18
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            valuew[2];          //!< if 1 bit set font mapper selects only matching aspect fonts. reassemble/store the value using valuew, the 32 bit value is not aligned.
} U_WMRSETMAPPERFLAGS;

/* Index 32 U_WMREXTTEXTOUT                 WMF manual 2.3.3.5
*/
/** WMF manual 2.3.3.5

   Variable size record.  Optional fields which follow the struct fields are:
   
        U_RECT16   Rect;    Only present when U_ETO_OPAQUE or U_ETO_CLIPPED bits are set in Opts
        uint8_t    String;  String to write, storage area must be 2n bytes.
        int16_t    Dx;      Kerning information.  Must have same number of entries as Length.
                            Dx is present when
                            2*Size16_4[2] -14 - 2*((Length + 1)/2)) - 8*(Opts & (U_ETO_OPAQUE | U_ETO_CLIPPED)) == 2*Length
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    int16_t             y;                  //!< in logical units (draw point)
    int16_t             x;                  //!< in logical units (draw point)
    int16_t             Length;             //!< Stringlength in bytes
    uint16_t            Opts;               //!< ExtTextOutOptions Flags
} U_WMREXTTEXTOUT;

/* Index 33 U_WMRSETDIBTODEV                WMF manual 2.3.1.4 */
/**  WMF manual 2.3.1.4

   Constant part of record is shown.  It is followed by a DeviceIndependentBitmap Object
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            cUsage;             //!< ColorUsage Enumeration
    uint16_t            ScanCount;          //!< Number of scan lines in Src
    uint16_t            StartScan;          //!< First Scan line in Src
    int16_t             ySrc;               //!< in logical units (UL corner of Src rect)
    int16_t             xSrc;               //!< in logical units (UL corner of Src rect)
    int16_t             Height;             //!< in logical units (of Src and Dst)
    int16_t             Width;              //!< in logical units (of Src and Dst)
    int16_t             yDst;               //!< in logical units (UL corner of Dst rect)
    int16_t             xDst;               //!< in logical units (UL corner of Dst rect)
    uint8_t             dib[1];             //!< DeviceIndependentBitmap object
} U_WMRSETDIBTODEV;

/* Index 34 U_WMRSELECTPALETTE              WMF manual 2.3.4.11 See Index 2A */

/* Index 35 U_WMRREALIZEPALETTE             WMF manual 2.3.5.8  See Index 00 */

/* Index 36 U_WMRANIMATEPALETTE             WMF manual 2.3.5.1  
   Index 37 U_WMRSETPALENTRIES              WMF manual 2.3.5.19
   Index F7 U_WMRCREATEPALETTE              WMF manual 2.3.4.3
*/
/** WMF manual 2.3.5.1
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    U_PALETTE           Palette;            //!< Palette object
} U_WMRANIMATEPALETTE,
  U_WMRSETPALENTRIES,                       //!< WMF manual 2.3.5.19
  U_WMRCREATEPALETTE;                       //!< WMF manual 2.3.4.3

/* Index 38 U_WMRPOLYPOLYGON                WMF manual 2.3.3.16 */
/** WMF manual 2.3.3.16
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    U_POLYPOLYGON       PPolygon;           //!< PolyPolygon object (size is variable!)
} U_WMRPOLYPOLYGON;

/* Index 39 U_WMRRESIZEPALETTE              WMF manual 2.3.5.9  See Index 2A */

/* Index 40 U_WMRDIBBITBLT                  WMF manual 2.3.1.2 
*/
/** WMF manual 2.3.1.2 

   The PX form is a variable structure the core/invariant part extends to xDst, and that is
   followed by a DeviceInvariantBitmap object which starts at "dib".
   The NOPX form is a constant structure.
   
   if RecordSize == ((xb) + 3) then there is no bitmap and use the _NOPX form, otherwise use the _PX form
   Use Macro U_TEST_NOPX2.
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            rop3w[2];           //!< reassemble/store the Ternary raster operation rop3 value using rop3w, the 32 bit value is not aligned.
    int16_t             ySrc;               //!< in logical units (UL corner of Src rect)
    int16_t             xSrc;               //!< in logical units (UL corner of Src rect)
    uint16_t            ignore;             //!< ignore
    int16_t             Height;             //!< in logical units (of Src and Dst)
    int16_t             Width;              //!< in logical units (of Src and Dst)
    int16_t             yDst;               //!< in logical units (UL corner of Dst rect)
    int16_t             xDst;               //!< in logical units (UL corner of Dst rect)
} U_WMRDIBBITBLT_NOPX;

/** WMF manual 2.3.1.2 

   The PX form is a variable structure the core/invariant part extends to xDst, and that is
   followed by a DeviceInvariantBitmap object which starts at "dib".
   The NOPX form is a constant structure.
   
   if RecordSize == ((xb) + 3) then there is no bitmap and use the _NOPX form, otherwise use the _PX form
   Use Macro U_TEST_NOPX2.
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            rop3w[2];           //!< reassemble/store the Ternary raster operation rop3 value using rop3w, the 32 bit value is not aligned.
    int16_t             ySrc;               //!< in logical units (UL corner of Src rect)
    int16_t             xSrc;               //!< in logical units (UL corner of Src rect)
    int16_t             Height;             //!< in logical units (of Src and Dst)
    int16_t             Width;              //!< in logical units (of Src and Dst)
    int16_t             yDst;               //!< in logical units (UL corner of Dst rect)
    int16_t             xDst;               //!< in logical units (UL corner of Dst rect)
    uint8_t             dib[1];             //!< DeviceIndependentBitmap object
} U_WMRDIBBITBLT_PX;

/* Index 41 U_WMRDIBSTRETCHBLT              WMF manual 2.3.1.3 */
/** WMF manual 2.3.1.3 

   The PX form is a variable structure the core/invariant part extends to xDst, and that is
   followed by a DeviceInvariantBitmap object which starts at "dib".
   The NOPX form is a constant structure.
   
   if RecordSize == ((xb) + 3) then there is no bitmap and use the _NOPX form, otherwise use the _PX form
   Use Macro U_TEST_NOPX2.
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            rop3w[2];           //!< reassemble/store the Ternary raster operation rop3 value using rop3w, the 32 bit value is not aligned.
    int16_t             hSrc;               //!< in logical units (of Src)
    int16_t             wSrc;               //!< in logical units (of Src)
    int16_t             ySrc;               //!< in logical units (UL corner of Src rect)
    int16_t             xSrc;               //!< in logical units (UL corner of Src rect)
    uint16_t            ignore;             //!< ignore
    int16_t             hDst;               //!< in logical units (of Dst)
    int16_t             wDst;               //!< in logical units (of Dst)
    int16_t             yDst;               //!< in logical units (UL corner of Dst rect)
    int16_t             xDst;               //!< in logical units (UL corner of Dst rect)
} U_WMRDIBSTRETCHBLT_NOPX;

/** WMF manual 2.3.1.3 

   The PX form is a variable structure the core/invariant part extends to xDst, and that is
   followed by a DeviceInvariantBitmap object which starts at "dib".
   The NOPX form is a constant structure.
   
   if RecordSize == ((xb) + 3) then there is no bitmap and use the _NOPX form, otherwise use the _PX form
   Use Macro U_TEST_NOPX2.
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            rop3w[2];           //!< reassemble/store the Ternary raster operation rop3 value using rop3w, the 32 bit value is not aligned.
    int16_t             hSrc;               //!< in logical units (of Src)
    int16_t             wSrc;               //!< in logical units (of Src)
    int16_t             ySrc;               //!< in logical units (UL corner of Src rect)
    int16_t             xSrc;               //!< in logical units (UL corner of Src rect)
    int16_t             hDst;               //!< in logical units (of Dst)
    int16_t             wDst;               //!< in logical units (of Dst)
    int16_t             yDst;               //!< in logical units (UL corner of Dst rect)
    int16_t             xDst;               //!< in logical units (UL corner of Dst rect)
    uint8_t             dib[1];             //!< DeviceIndependentBitmap object
} U_WMRDIBSTRETCHBLT_PX;


/* Index 42 U_WMRDIBCREATEPATTERNBRUSH      WMF manual 2.3.4.8
*/
/** WMF manual 2.3.4.8

        style                cUsage                Brush created
        U_BS_SOLID                                 like U_BS_DIBPATTERNPT
        U_BS_NULL                                  like U_BS_DIBPATTERNPT
        U_BS_HATCHED                               like U_BS_DIBPATTERNPT
        U_BS_DIBPATTERNPT    ColorUsage enumer.    U_BS_DIBPATTERNPT brush from DIB in Src
        U_BS_PATTERN         ColorUsage enumer.    U_BS_PATTERN brush from Bitmap16 object in Src
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            Style;              //!< BrushStyle Enumeration
    uint16_t            cUsage;             //!< See table above
    uint8_t             Src[1];             //!< DeviceIndependentBitmap or Bitmap16 object
} U_WMRDIBCREATEPATTERNBRUSH;

/* Index 43 U_WMRSTRETCHDIB                 WMF manual 2.3.1.6 */
/** WMF manual 2.3.1.6
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    uint16_t            rop3w[2];           //!< reassemble/store the Ternary raster operation rop3 value using rop3w, the 32 bit value is not aligned.
    uint16_t            cUsage;             //!< ColorUsage Enumeration
    int16_t             hSrc;               //!< in logical units (of Src)
    int16_t             wSrc;               //!< in logical units (of Src)
    int16_t             ySrc;               //!< in logical units (UL corner of Src rect)
    int16_t             xSrc;               //!< in logical units (UL corner of Src rect)
    int16_t             hDst;               //!< in logical units (of Dst)
    int16_t             wDst;               //!< in logical units (of Dst)
    int16_t             yDst;               //!< in logical units (UL corner of Dst rect)
    int16_t             xDst;               //!< in logical units (UL corner of Dst rect)
    uint8_t             dib[1];             //!< DeviceIndependentBitmap object
} U_WMRSTRETCHDIB;

/* Index 48 U_WMREXTFLOODFILL               WMF manual 2.3.3.4  See Index 19*/
/* Index 4C U_WMR4C                                          */ 
/* Index 4D U_WMR4D                                          */ 
/* Index 4F U_WMR4F                                          */ 
/* Index 50 U_WMR50                                          */ 
/* Index 52 U_WMR52                                          */ 
/* Index 5E U_WMR5E                                          */ 
/* Index 5F U_WMR5F                                          */  
/* Index 60 U_WMR60                                          */  
/* Index 61 U_WMR61                                          */  
/* Index 62 U_WMR62                                          */  
/* Index 63 U_WMR63                                          */  
/* Index 64 U_WMR64                                          */  
/* Index 65 U_WMR65                                          */  
/* Index 66 U_WMR66                                          */  
/* Index 67 U_WMR67                                          */  
/* Index 68 U_WMR68                                          */  
/* Index 69 U_WMR69                                          */  
/* Index 6A U_WMR6A                                          */  
/* Index 6B U_WMR6B                                          */  
/* Index 6C U_WMR6C                                          */  
/* Index 6D U_WMR6D                                          */  
/* Index 6E U_WMR6E                                          */  
/* Index 6F U_WMR6F                                          */  
/* Index 70 U_WMR70                                          */  
/* Index 71 U_WMR71                                          */  
/* Index 72 U_WMR72                                          */  
/* Index 73 U_WMR73                                          */  
/* Index 74 U_WMR74                                          */  
/* Index 75 U_WMR75                                          */  
/* Index 76 U_WMR76                                          */  
/* Index 77 U_WMR77                                          */  
/* Index 78 U_WMR78                                          */  
/* Index 79 U_WMR79                                          */  
/* Index 7A U_WMR7A                                          */  
/* Index 7B U_WMR7B                                          */  
/* Index 7C U_WMR7C                                          */  
/* Index 7D U_WMR7D                                          */  
/* Index 7E U_WMR7E                                          */  
/* Index 7F U_WMR7F                                          */  
/* Index 80 U_WMR80                                          */  
/* Index 81 U_WMR81                                          */  
/* Index 82 U_WMR82                                          */  
/* Index 83 U_WMR83                                          */  
/* Index 84 U_WMR84                                          */  
/* Index 85 U_WMR85                                          */  
/* Index 86 U_WMR86                                          */  
/* Index 87 U_WMR87                                          */  
/* Index 88 U_WMR88                                          */  
/* Index 89 U_WMR89                                          */  
/* Index 8A U_WMR8A                                          */  
/* Index 8B U_WMR8B                                          */  
/* Index 8C U_WMR8C                                          */  
/* Index 8D U_WMR8D                                          */  
/* Index 8E U_WMR8E                                          */  
/* Index 8F U_WMR8F                                          */  
/* Index 90 U_WMR90                                          */  
/* Index 91 U_WMR91                                          */  
/* Index 92 U_WMR92                                          */  
/* Index 93 U_WMR93                                          */  
/* Index 94 U_WMR94                                          */  
/* Index 95 U_WMR95                                          */  
/* Index 96 U_WMR96                                          */  
/* Index 97 U_WMR97                                          */  
/* Index 98 U_WMR98                                          */  
/* Index 99 U_WMR99                                          */  
/* Index 9A U_WMR9A                                          */  
/* Index 9B U_WMR9B                                          */  
/* Index 9C U_WMR9C                                          */  
/* Index 9D U_WMR9D                                          */  
/* Index 9E U_WMR9E                                          */  
/* Index 9F U_WMR9F                                          */  
/* Index A0 U_WMRA0                                          */  
/* Index A1 U_WMRA1                                          */  
/* Index A2 U_WMRA2                                          */  
/* Index A3 U_WMRA3                                          */  
/* Index A4 U_WMRA4                                          */  
/* Index A5 U_WMRA5                                          */  
/* Index A6 U_WMRA6                                          */  
/* Index A7 U_WMRA7                                          */  
/* Index A8 U_WMRA8                                          */  
/* Index A9 U_WMRA9                                          */  
/* Index AA U_WMRAA                                          */  
/* Index AB U_WMRAB                                          */  
/* Index AC U_WMRAC                                          */  
/* Index AD U_WMRAD                                          */  
/* Index AE U_WMRAE                                          */  
/* Index AF U_WMRAF                                          */  
/* Index B0 U_WMRB0                                          */  
/* Index B1 U_WMRB1                                          */  
/* Index B2 U_WMRB2                                          */  
/* Index B3 U_WMRB3                                          */  
/* Index B4 U_WMRB4                                          */  
/* Index B5 U_WMRB5                                          */  
/* Index B6 U_WMRB6                                          */  
/* Index B7 U_WMRB7                                          */  
/* Index B8 U_WMRB8                                          */  
/* Index B9 U_WMRB9                                          */  
/* Index BA U_WMRBA                                          */  
/* Index BB U_WMRBB                                          */  
/* Index BC U_WMRBC                                          */  
/* Index BD U_WMRBD                                          */  
/* Index BE U_WMRBE                                          */  
/* Index BF U_WMRBF                                          */  
/* Index C0 U_WMRC0                                          */  
/* Index C1 U_WMRC1                                          */  
/* Index C2 U_WMRC2                                          */  
/* Index C3 U_WMRC3                                          */  
/* Index C4 U_WMRC4                                          */  
/* Index C5 U_WMRC5                                          */  
/* Index C6 U_WMRC6                                          */  
/* Index C7 U_WMRC7                                          */  
/* Index C8 U_WMRC8                                          */  
/* Index C9 U_WMRC9                                          */  
/* Index CA U_WMRCA                                          */  
/* Index CB U_WMRCB                                          */  
/* Index CC U_WMRCC                                          */  
/* Index CD U_WMRCD                                          */  
/* Index CE U_WMRCE                                          */  
/* Index CF U_WMRCF                                          */  
/* Index D0 U_WMRD0                                          */  
/* Index D1 U_WMRD1                                          */  
/* Index D2 U_WMRD2                                          */  
/* Index D3 U_WMRD3                                          */  
/* Index D4 U_WMRD4                                          */  
/* Index D5 U_WMRD5                                          */  
/* Index D6 U_WMRD6                                          */  
/* Index D7 U_WMRD7                                          */  
/* Index D8 U_WMRD8                                          */  
/* Index D9 U_WMRD9                                          */  
/* Index DA U_WMRDA                                          */  
/* Index DB U_WMRDB                                          */  
/* Index DC U_WMRDC                                          */  
/* Index DD U_WMRDD                                          */  
/* Index DE U_WMRDE                                          */  
/* Index DF U_WMRDF                                          */  
/* Index E0 U_WMRE0                                          */  
/* Index E1 U_WMRE1                                          */  
/* Index E2 U_WMRE2                                          */  
/* Index E3 U_WMRE3                                          */  
/* Index E4 U_WMRE4                                          */  
/* Index E5 U_WMRE5                                          */  
/* Index E6 U_WMRE6                                          */  
/* Index E7 U_WMRE7                                          */  
/* Index E8 U_WMRE8                                          */  
/* Index E9 U_WMRE9                                          */  
/* Index EA U_WMREA                                          */  
/* Index EB U_WMREB                                          */  
/* Index EC U_WMREC                                          */  
/* Index ED U_WMRED                                          */  
/* Index EE U_WMREE                                          */  
/* Index EF U_WMREF                                          */  
/* Index F0 U_WMRDELETEOBJECT               WMF manual 2.3.4.7  See Index 2A */       
/* Index F1 U_WMRF1                                          */  
/* Index F2 U_WMRF2                                          */  
/* Index F3 U_WMRF3                                          */  
/* Index F4 U_WMRF4                                          */  
/* Index F5 U_WMRF5                                          */  

/* Index F7 U_WMRCREATEPALETTE              WMF manual 2.3.4.3  See Index 36*/

/* Index F8 U_WMRF8                                          */  
/* Index F9 U_WMRCREATEPATTERNBRUSH         WMF manual 2.3.4.4 */ 
/** WMF manual 2.3.4.4

  WARNING - U_WMRCREATEPATTERNBRUSH has been declared obsolete and application support is spotty -
  use U_WMRDIBCREATEPATTERNBRUSH instead.

  This record is peculiar...
  
  After the core structure there is:
  
        A truncated U_BITMAP16. Only the first 14 bytes are present, and the last 4 bytes (bits section) are ignored.
        18 zero bytes (reserved)
        A pattern.  The pattern is a byte array whose size is set by the fields in the U_BITMAP16 structure as follows:

        (((Width * BitsPixel + 15) >> 4) << 1) * Height

  brush created has style U_BS_PATTERN.
  
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
} U_WMRCREATEPATTERNBRUSH;

/* Index FA U_WMRCREATEPENINDIRECT          WMF manual 2.3.4.5  */
/** WMF manual 2.3.4.5 
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    U_PEN               pen;                //!< Pen Object
} U_WMRCREATEPENINDIRECT;

/* Index FB U_WMRCREATEFONTINDIRECT         WMF manual 2.3.4.2  */
/** WMF manual 2.3.4.2
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    U_FONT              font;               //!< Font Object
} U_WMRCREATEFONTINDIRECT;

/* Index FC U_WMRCREATEBRUSHINDIRECT        WMF manual 2.3.4.1  */
/** WMF manual 2.3.4.1
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    U_WLOGBRUSH         brush;              //!< WLogBrush Object
} U_WMRCREATEBRUSHINDIRECT;

/* Index FD U_WMRCREATEBITMAPINDIRECT       in GDI and Wine, not in WMF manual, see index 2F */  

/* Index FE U_WMRCREATEBITMAP               in GDI and Wine, not in WMF manual, see index 2F */  

/* Index FF U_WMRCREATEREGION               WMF manual 2.3.4.6  */  
/**  WMF manual 2.3.4.6
*/
typedef struct {
    uint16_t            Size16_4[2];        //!< Total number of 16bit words in record
    uint8_t             iType;              //!< RecordType Enumeration
    uint8_t             xb;                 //!< Extra high order byte associated with record type
    U_REGION            region;             //!< Region Object
} U_WMRCREATEREGION;



// ************************************************************************************************
// Utility function structures

/**
  Storage for keeping track of properties of the growing WMF file as records are added.
*/
typedef struct {
    FILE               *fp;                 //!< Open file
    size_t              allocated;          //!< Size of the buffer
    size_t              used;               //!< Amount consumed
    uint32_t            records;            //!< Number of records already contained
    uint16_t            ignore;             //!< size padding,not used
    uint32_t            PalEntries;         //!< Number of PalEntries (set from U_EMREOF)
    uint32_t            chunk;              //!< Number of bytes to add when more space is needed
    char               *buf;                //!< Buffer for constructing the EMF in memory 
    uint32_t            largest;            //!< Largest record size, in bytes (used by WMF, not by EMF)
    uint32_t            sumObjects;         //!< Number of objects appended  (used by WMF, not by EMF) [ also see wmf_highwater() ]
} WMFTRACK;

/**
  The various create functions need a place to put their handles, these are stored in the table below.
  We don't actually do anything much with these handles, that is up to whatever program finally plays back the WMF, but
  we do need to keep track of the numbers so that they are not accidentally reused. (Also WMF files have rules
  about how object handles must be numbered, for instance, the lowest possible number must always be used. These
  are different from EMF object handles.) This structure is used for staying in conformance with these rules.
  
  There are no stock objects in WMF files.
*/
typedef struct {
    uint32_t           *table;              //!< Array Buffer for constructing the WMF in memory 
    size_t              allocated;          //!< Slots in the buffer
    size_t              chunk;              //!< Number to add if a realloc is required
    uint32_t            lolimit;            //!< Lowest unoccupied table slot, may be a hole created by a deleteobject.
    uint32_t            hilimit;            //!< Highest table slot occupied (currently)
    uint32_t            peak;               //!< Highest table slot occupied (ever)
} WMFHANDLES;

//! \cond
// ************************************************************************************************
// Prototypes (_set first, then _get)
char        *wmr_dup(const char *wmr);
int          wmf_start(const char *name, uint32_t initsize, uint32_t chunksize, WMFTRACK **wt);
int          wmf_free(WMFTRACK **wt);
int          wmf_finish(WMFTRACK *wt);
int          wmf_append(U_METARECORD *rec, WMFTRACK *wt, int freerec);
int          wmf_header_append(U_METARECORD *rec,WMFTRACK *et, int freerec);
int          wmf_readdata(const char *filename, char **contents, size_t*length);
#define      wmf_fopen    emf_fopen
int          wmf_highwater(uint32_t setval);
int          wmf_htable_create(uint32_t initsize, uint32_t chunksize, WMFHANDLES **wht);
int          wmf_htable_delete(uint32_t *ih, WMFHANDLES *wht);
int          wmf_htable_insert(uint32_t *ih, WMFHANDLES *wht);
int          wmf_htable_free(WMFHANDLES **wht);
int16_t      U_16_checksum(int16_t *buf, int count);
int16_t     *dx16_set( int32_t  height, uint32_t weight, uint32_t members);
uint32_t     U_wmr_properties(uint32_t type);

uint32_t     U_wmr_size(const U_METARECORD *record);
uint32_t     U_wmr_values(int idx);
const char  *U_wmr_names(int idx);
const char  *U_wmr_escnames(int idx);

void         U_sanerect16(U_RECT16 rc, double *left, double *top, double *right, double *bottom);


U_FONT      *U_FONT_set(int16_t Height, int16_t Width, int16_t Escapement, int16_t Orientation,
                        int16_t Weight, uint8_t Italic, uint8_t Underline, uint8_t StrikeOut, 
                        uint8_t CharSet, uint8_t OutPrecision, uint8_t ClipPrecision, 
                        uint8_t Quality, uint8_t PitchAndFamily, char *FaceName);
U_PLTNTRY    U_PLTNTRY_set(U_COLORREF Color);
U_PALETTE   *U_PLTENTRY_set(uint16_t Start, uint16_t NumEntries, U_PLTNTRY *Entries);
U_PEN        U_PEN_set(uint16_t Style, uint16_t Width, U_COLORREF Color);
U_RECT16     U_RECT16_set(U_POINT16 ul,U_POINT16 lr);
U_BITMAP16  *U_BITMAP16_set(const int16_t Type, const int16_t Width, const int16_t Height, 
                const int16_t LineN, const uint8_t BitsPixel, const char *Bits);
U_SCAN      *U_SCAN_set(uint16_t count, uint16_t top, uint16_t bottom, uint16_t *ScanLines);
U_REGION    *U_REGION_set(int16_t Size, int16_t sCount, int16_t sMax, U_RECT16 sRect, uint16_t *aScans);
U_WLOGBRUSH  U_WLOGBRUSH_set(uint16_t Style, U_COLORREF Color, uint16_t Hatch);
U_PAIRF     *U_PAIRF_set(float x, float y);

char        *wdeleteobject_set(uint32_t *ihObject, WMFHANDLES  *wht);
char        *wselectobject_set(uint32_t ihObject, WMFHANDLES *wht );
char        *wcreatepenindirect_set(uint32_t *ihPen, WMFHANDLES *wht, U_PEN pen);
char        *wcreatebrushindirect_set(uint32_t *ihBrush, WMFHANDLES *wht, U_WLOGBRUSH lb);
char        *wcreatedibpatternbrush_srcdib_set(uint32_t *ihBrush, WMFHANDLES *wht, 
                uint32_t iUsage, const U_BITMAPINFO *Bmi, uint32_t cbPx, const char *Px);
char        *wcreatedibpatternbrush_srcbm16_set(uint32_t *ihBrush, WMFHANDLES *wht, 
                uint32_t iUsage,  const U_BITMAP16 *Bm16);
char        *wcreatepatternbrush_set(uint32_t *ihBrush, WMFHANDLES *wht, U_BITMAP16 *Bm16, char *Pattern);
char        *wcreatefontindirect_set(uint32_t *ihFont, WMFHANDLES *wht, U_FONT *uf);
char        *wcreatepalette_set(uint32_t *ihPal, WMFHANDLES *wht, U_PALETTE *up);
char        *wsetpaletteentries_set(uint32_t *ihPal, WMFHANDLES *wht, const U_PALETTE *Palletes);
char        *wcreateregion_set(uint32_t *ihReg,  WMFHANDLES *wht, const U_REGION *Region);
char        *wbegin_path_set(void);
char        *wend_path_set(void);
char        *wlinecap_set(int32_t Type);
char        *wlinejoin_set(int32_t Type);
char        *wmiterlimit_set(int32_t limit);


char        *U_WMRHEADER_set(U_PAIRF *size,unsigned int dpi);
char        *U_WMREOF_set(void);
char        *U_WMRSETBKCOLOR_set(U_COLORREF Color);
char        *U_WMRSETBKMODE_set(uint16_t Mode);
char        *U_WMRSETMAPMODE_set(uint16_t Mode);
char        *U_WMRSETROP2_set(uint16_t Mode);
char        *U_WMRSETRELABS_set(void);
char        *U_WMRSETPOLYFILLMODE_set(uint16_t Mode);
char        *U_WMRSETSTRETCHBLTMODE_set(uint16_t Mode);
char        *U_WMRSETTEXTCHAREXTRA_set(uint16_t Mode);
char        *U_WMRSETTEXTCOLOR_set(U_COLORREF Color);
char        *U_WMRSETTEXTJUSTIFICATION_set(uint16_t Count, uint16_t Extra);
char        *U_WMRSETWINDOWORG_set(U_POINT16 coord);
char        *U_WMRSETWINDOWEXT_set(U_POINT16 extent);
char        *U_WMRSETVIEWPORTORG_set(U_POINT16 coord);
char        *U_WMRSETVIEWPORTEXT_set(U_POINT16 extent);
char        *U_WMROFFSETWINDOWORG_set(U_POINT16 offset);
char        *U_WMRSCALEWINDOWEXT_set(U_POINT16 Denom, U_POINT16 Num);
char        *U_WMROFFSETVIEWPORTORG_set(U_POINT16 offset);
char        *U_WMRSCALEVIEWPORTEXT_set(U_POINT16 Denom, U_POINT16 Num);
char        *U_WMRLINETO_set(U_POINT16 coord);
char        *U_WMRMOVETO_set(U_POINT16 coord);
char        *U_WMREXCLUDECLIPRECT_set(U_RECT16 rect);
char        *U_WMRINTERSECTCLIPRECT_set(U_RECT16 rect);
char        *U_WMRARC_set(U_POINT16 StartArc, U_POINT16 EndArc, U_RECT16 rect);
char        *U_WMRELLIPSE_set(U_RECT16 rect);
char        *U_WMRFLOODFILL_set(uint16_t Mode, U_COLORREF Color, U_POINT16 coord);
char        *U_WMRPIE_set(U_POINT16 Radial1, U_POINT16 Radial2, U_RECT16 rect);
char        *U_WMRRECTANGLE_set(U_RECT16 rect);
char        *U_WMRROUNDRECT_set(int16_t Width, int16_t Height, U_RECT16 rect);
char        *U_WMRPATBLT_set(U_POINT16 Dst, U_POINT16 cwh, uint32_t dwRop3);
char        *U_WMRSAVEDC_set(void);
char        *U_WMRSETPIXEL_set(U_COLORREF Color, U_POINT16 coord);
char        *U_WMROFFSETCLIPRGN_set(U_POINT16 offset);
char        *U_WMRTEXTOUT_set(U_POINT16 Dst, char *string);
char        *U_WMRBITBLT_set(U_POINT16 Dst, U_POINT16 cwh, U_POINT16 Src,
                uint32_t dwRop3, const U_BITMAP16 *Bm16);
char        *U_WMRSTRETCHBLT_set(U_POINT16 Dst, U_POINT16 cDst, U_POINT16 Src,
                U_POINT16 cSrc, uint32_t dwRop3, const U_BITMAP16 *Bm16);
char        *U_WMRPOLYGON_set(uint16_t Length, const U_POINT16 * Data);
char        *U_WMRPOLYLINE_set(uint16_t Length, const U_POINT16 * Data);
char        *U_WMRESCAPE_set(uint16_t Escape, uint16_t Length,  const void *Data);
char        *U_WMRRESTOREDC_set(int16_t DC);
char        *U_WMRFILLREGION_set(uint16_t Region, uint16_t Brush);
char        *U_WMRFRAMEREGION_set(uint16_t Region, uint16_t Brush, int16_t Height, int16_t Width);
char        *U_WMRINVERTREGION_set(uint16_t Region);
char        *U_WMRPAINTREGION_set(uint16_t Region);
char        *U_WMRSELECTCLIPREGION_set(uint16_t Region);
char        *U_WMRSELECTOBJECT_set(uint16_t object);
char        *U_WMRSETTEXTALIGN_set(uint16_t Mode);
char        *U_WMRDRAWTEXT_set(void);  /* in GDI and Wine, not in WMF manual. */
char        *U_WMRCHORD_set(U_POINT16 Radial1, U_POINT16 Radial2, U_RECT16 rect);
char        *U_WMRSETMAPPERFLAGS_set(uint32_t Mode);
char        *U_WMREXTTEXTOUT_set(U_POINT16 Dst, int16_t Length, uint16_t Opts, const char *string, int16_t *dx, U_RECT16 rect);
char        *U_WMRSETDIBTODEV_set(void);
char        *U_WMRSELECTPALETTE_set(uint16_t Palette);
char        *U_WMRREALIZEPALETTE_set(void);
char        *U_WMRANIMATEPALETTE_set(U_PALETTE *Palette);
char        *U_WMRSETPALENTRIES_set(const U_PALETTE *Palette);
char        *U_WMRPOLYPOLYGON_set(const uint16_t, const uint16_t *aPolyCounts, const U_POINT16 * points);
char        *U_WMRRESIZEPALETTE_set(uint16_t Palette);
char        *U_WMR3A_set(void);
char        *U_WMR3B_set(void);
char        *U_WMR3C_set(void);
char        *U_WMR3D_set(void);
char        *U_WMR3E_set(void);
char        *U_WMR3F_set(void);
char        *U_WMRDIBBITBLT_set(U_POINT16 Dst, U_POINT16 cwh, U_POINT16 Src,
                uint32_t dwRop3, const U_BITMAPINFO * Bmi, uint32_t cbPx, const char *Px);
char        *U_WMRDIBSTRETCHBLT_set(U_POINT16 Dst, U_POINT16 cDst, U_POINT16 Src,
                U_POINT16 cSrc, uint32_t dwRop3, const U_BITMAPINFO *Bmi, uint32_t cbPx, const char *Px);
char        *U_WMRDIBCREATEPATTERNBRUSH_set(const uint16_t Style, const uint16_t iUsage, 
                const U_BITMAPINFO *Bmi, uint32_t cbPx, const char *Px, const U_BITMAP16 *Bm16);  
char        *U_WMRSTRETCHDIB_set(U_POINT16 Dest, U_POINT16 cDest, U_POINT16 Src, U_POINT16 cSrc,
              const uint16_t cUsage, uint32_t dwRop3, const U_BITMAPINFO *Bmi, uint32_t cbPx, const char *Px);
char        *U_WMR44_set(void);
char        *U_WMR45_set(void);
char        *U_WMR46_set(void);
char        *U_WMR47_set(void);
char        *U_WMREXTFLOODFILL_set(uint16_t Mode, U_COLORREF Color, U_POINT16 coord);
char        *U_WMR49_set(void);
char        *U_WMR4A_set(void);
char        *U_WMR4B_set(void);
char        *U_WMR4C_set(void);
char        *U_WMR4D_set(void);
char        *U_WMR4E_set(void);
char        *U_WMR4F_set(void);
char        *U_WMR50_set(void);
char        *U_WMR51_set(void);
char        *U_WMRABORTDOC_set(void);
char        *U_WMR53_set(void);
char        *U_WMR54_set(void);
char        *U_WMR55_set(void);
char        *U_WMR56_set(void);
char        *U_WMR57_set(void);
char        *U_WMR58_set(void);
char        *U_WMR59_set(void);
char        *U_WMR5A_set(void);
char        *U_WMR5B_set(void);
char        *U_WMR5C_set(void);
char        *U_WMR5D_set(void);
char        *U_WMR5E_set(void);
char        *U_WMR5F_set(void);
char        *U_WMR60_set(void);
char        *U_WMR61_set(void);
char        *U_WMR62_set(void);
char        *U_WMR63_set(void);
char        *U_WMR64_set(void);
char        *U_WMR65_set(void);
char        *U_WMR66_set(void);
char        *U_WMR67_set(void);
char        *U_WMR68_set(void);
char        *U_WMR69_set(void);
char        *U_WMR6A_set(void);
char        *U_WMR6B_set(void);
char        *U_WMR6C_set(void);
char        *U_WMR6D_set(void);
char        *U_WMR6E_set(void);
char        *U_WMR6F_set(void);
char        *U_WMR70_set(void);
char        *U_WMR71_set(void);
char        *U_WMR72_set(void);
char        *U_WMR73_set(void);
char        *U_WMR74_set(void);
char        *U_WMR75_set(void);
char        *U_WMR76_set(void);
char        *U_WMR77_set(void);
char        *U_WMR78_set(void);
char        *U_WMR79_set(void);
char        *U_WMR7A_set(void);
char        *U_WMR7B_set(void);
char        *U_WMR7C_set(void);
char        *U_WMR7D_set(void);
char        *U_WMR7E_set(void);
char        *U_WMR7F_set(void);
char        *U_WMR80_set(void);
char        *U_WMR81_set(void);
char        *U_WMR82_set(void);
char        *U_WMR83_set(void);
char        *U_WMR84_set(void);
char        *U_WMR85_set(void);
char        *U_WMR86_set(void);
char        *U_WMR87_set(void);
char        *U_WMR88_set(void);
char        *U_WMR89_set(void);
char        *U_WMR8A_set(void);
char        *U_WMR8B_set(void);
char        *U_WMR8C_set(void);
char        *U_WMR8D_set(void);
char        *U_WMR8E_set(void);
char        *U_WMR8F_set(void);
char        *U_WMR90_set(void);
char        *U_WMR91_set(void);
char        *U_WMR92_set(void);
char        *U_WMR93_set(void);
char        *U_WMR94_set(void);
char        *U_WMR95_set(void);
char        *U_WMR96_set(void);
char        *U_WMR97_set(void);
char        *U_WMR98_set(void);
char        *U_WMR99_set(void);
char        *U_WMR9A_set(void);
char        *U_WMR9B_set(void);
char        *U_WMR9C_set(void);
char        *U_WMR9D_set(void);
char        *U_WMR9E_set(void);
char        *U_WMR9F_set(void);
char        *U_WMRA0_set(void);
char        *U_WMRA1_set(void);
char        *U_WMRA2_set(void);
char        *U_WMRA3_set(void);
char        *U_WMRA4_set(void);
char        *U_WMRA5_set(void);
char        *U_WMRA6_set(void);
char        *U_WMRA7_set(void);
char        *U_WMRA8_set(void);
char        *U_WMRA9_set(void);
char        *U_WMRAA_set(void);
char        *U_WMRAB_set(void);
char        *U_WMRAC_set(void);
char        *U_WMRAD_set(void);
char        *U_WMRAE_set(void);
char        *U_WMRAF_set(void);
char        *U_WMRB0_set(void);
char        *U_WMRB1_set(void);
char        *U_WMRB2_set(void);
char        *U_WMRB3_set(void);
char        *U_WMRB4_set(void);
char        *U_WMRB5_set(void);
char        *U_WMRB6_set(void);
char        *U_WMRB7_set(void);
char        *U_WMRB8_set(void);
char        *U_WMRB9_set(void);
char        *U_WMRBA_set(void);
char        *U_WMRBB_set(void);
char        *U_WMRBC_set(void);
char        *U_WMRBD_set(void);
char        *U_WMRBE_set(void);
char        *U_WMRBF_set(void);
char        *U_WMRC0_set(void);
char        *U_WMRC1_set(void);
char        *U_WMRC2_set(void);
char        *U_WMRC3_set(void);
char        *U_WMRC4_set(void);
char        *U_WMRC5_set(void);
char        *U_WMRC6_set(void);
char        *U_WMRC7_set(void);
char        *U_WMRC8_set(void);
char        *U_WMRC9_set(void);
char        *U_WMRCA_set(void);
char        *U_WMRCB_set(void);
char        *U_WMRCC_set(void);
char        *U_WMRCD_set(void);
char        *U_WMRCE_set(void);
char        *U_WMRCF_set(void);
char        *U_WMRD0_set(void);
char        *U_WMRD1_set(void);
char        *U_WMRD2_set(void);
char        *U_WMRD3_set(void);
char        *U_WMRD4_set(void);
char        *U_WMRD5_set(void);
char        *U_WMRD6_set(void);
char        *U_WMRD7_set(void);
char        *U_WMRD8_set(void);
char        *U_WMRD9_set(void);
char        *U_WMRDA_set(void);
char        *U_WMRDB_set(void);
char        *U_WMRDC_set(void);
char        *U_WMRDD_set(void);
char        *U_WMRDE_set(void);
char        *U_WMRDF_set(void);
char        *U_WMRE0_set(void);
char        *U_WMRE1_set(void);
char        *U_WMRE2_set(void);
char        *U_WMRE3_set(void);
char        *U_WMRE4_set(void);
char        *U_WMRE5_set(void);
char        *U_WMRE6_set(void);
char        *U_WMRE7_set(void);
char        *U_WMRE8_set(void);
char        *U_WMRE9_set(void);
char        *U_WMREA_set(void);
char        *U_WMREB_set(void);
char        *U_WMREC_set(void);
char        *U_WMRED_set(void);
char        *U_WMREE_set(void);
char        *U_WMREF_set(void);
char        *U_WMRDELETEOBJECT_set(uint16_t object);
char        *U_WMRF1_set(void);
char        *U_WMRF2_set(void);
char        *U_WMRF3_set(void);
char        *U_WMRF4_set(void);
char        *U_WMRF5_set(void);
char        *U_WMRF6_set(void);
char        *U_WMRCREATEPALETTE_set(U_PALETTE *Palette);
char        *U_WMRF8_set(void);
char        *U_WMRCREATEPATTERNBRUSH_set(U_BITMAP16 *Bm16, char *Pattern);
char        *U_WMRCREATEPENINDIRECT_set(U_PEN pen);
char        *U_WMRCREATEFONTINDIRECT_set(U_FONT *font);
char        *U_WMRCREATEBRUSHINDIRECT_set(U_WLOGBRUSH brush);
char        *U_WMRCREATEBITMAPINDIRECT_set(void);      /* in GDI and Wine, not in WMF manual. */
char        *U_WMRCREATEBITMAP_set(void);              /* in GDI and Wine, not in WMF manual. */
char        *U_WMRCREATEREGION_set(const U_REGION *region);

int16_t     *dx16_get( int32_t height, uint32_t weight, uint32_t members);
size_t       U_WMRRECSAFE_get(const char *contents, const char *blimit);
int          wmfheader_get(const char *contents, const char *blimit, U_WMRPLACEABLE *Placeable, U_WMRHEADER *Header);
int          wmr_arc_points(U_RECT16 rclBox, U_POINT16 ArcStart, U_POINT16 ArcEnd, 
                int *f1, int f2, U_PAIRF *center, U_PAIRF *start, U_PAIRF *end, U_PAIRF *size );
void         U_BITMAPINFOHEADER_get(const char *Bmih, uint32_t *Size, int32_t *Width, int32_t *Height, 
                uint32_t *Planes, uint32_t *BitCount, uint32_t *Compression, uint32_t *SizeImage, 
                int32_t *XPelsPerMeter, int32_t *YPelsPerMeter, uint32_t *ClrUsed, uint32_t *ClrImportant);
void         U_BITMAPCOREHEADER_get(const char *BmiCh, uint32_t *Size, int32_t *Width, int32_t *Height, int32_t *BitCount);
int          wget_DIB_params(const char *dib, const char **px, const U_RGBQUAD **ct, uint32_t *numCt, 
                int32_t *width, int32_t *height, int32_t *colortype, int32_t *invert);
int          U_WMREOF_get(const char *contents);
int          U_WMRSETBKCOLOR_get(const char *contents, U_COLORREF *Color);
int          U_WMRSETBKMODE_get(const char *contents, uint16_t *Mode);
int          U_WMRSETMAPMODE_get(const char *contents, uint16_t *Mode);
int          U_WMRSETROP2_get(const char *contents, uint16_t *Mode);
int          U_WMRSETRELABS_get(const char *contents);
int          U_WMRSETPOLYFILLMODE_get(const char *contents, uint16_t *Mode);
int          U_WMRSETSTRETCHBLTMODE_get(const char *contents, uint16_t *Mode);
int          U_WMRSETTEXTCHAREXTRA_get(const char *contents, uint16_t *Mode);
int          U_WMRSETTEXTCOLOR_get(const char *contents, U_COLORREF *Color);
int          U_WMRSETTEXTJUSTIFICATION_get(const char *contents, uint16_t *Count, uint16_t *Extra);
int          U_WMRSETWINDOWORG_get(const char *contents, U_POINT16 * coord);
int          U_WMRSETWINDOWEXT_get(const char *contents, U_POINT16 * extent);
int          U_WMRSETVIEWPORTORG_get(const char *contents, U_POINT16 * coord);
int          U_WMRSETVIEWPORTEXT_get(const char *contents, U_POINT16 * extent);
int          U_WMROFFSETWINDOWORG_get(const char *contents, U_POINT16 * offset);
int          U_WMRSCALEWINDOWEXT_get(const char *contents, U_POINT16 * Denom, U_POINT16 * Num);
int          U_WMROFFSETVIEWPORTORG_get(const char *contents, U_POINT16 * offset);
int          U_WMRSCALEVIEWPORTEXT_get(const char *contents, U_POINT16 * Denom, U_POINT16 * Num);
int          U_WMRLINETO_get(const char *contents, U_POINT16 * coord);
int          U_WMRMOVETO_get(const char *contents, U_POINT16 * coord);
int          U_WMREXCLUDECLIPRECT_get(const char *contents, U_RECT16 * rect);
int          U_WMRINTERSECTCLIPRECT_get(const char *contents, U_RECT16 * rect);
int          U_WMRARC_get(const char *contents, U_POINT16 * StartArc, U_POINT16 * EndArc, U_RECT16 * rect);
int          U_WMRELLIPSE_get(const char *contents, U_RECT16 * rect);
int          U_WMRFLOODFILL_get(const char *contents, uint16_t *Mode, U_COLORREF *Color, U_POINT16 * coord);
int          U_WMRPIE_get(const char *contents, U_POINT16 * Radial1, U_POINT16 * Radial2, U_RECT16 * rect);
int          U_WMRRECTANGLE_get(const char *contents, U_RECT16 * rect);
int          U_WMRROUNDRECT_get(const char *contents, int16_t *Width, int16_t *Height, U_RECT16 * rect);
int          U_WMRPATBLT_get(const char *contents, U_POINT16 * Dst, U_POINT16 * cwh, uint32_t *dwRop3);
int          U_WMRSAVEDC_get(const char *contents);
int          U_WMRSETPIXEL_get(const char *contents, U_COLORREF *Color, U_POINT16 * coord);
int          U_WMROFFSETCLIPRGN_get(const char *contents, U_POINT16 * offset);
int          U_WMRTEXTOUT_get(const char *contents, U_POINT16 * Dst, int16_t *Length, const char **string);
int          U_WMRBITBLT_get(const char *contents, U_POINT16 * Dst, U_POINT16 * cwh, U_POINT16 * Src, uint32_t *dwRop3, U_BITMAP16 *Bm16, const char **px);
int          U_WMRSTRETCHBLT_get(const char *contents, U_POINT16 * Dst, U_POINT16 * cDst, U_POINT16 * Src, U_POINT16 * cSrc, uint32_t *dwRop3, U_BITMAP16 *Bm16, const char **px);
int          U_WMRPOLYGON_get(const char *contents, uint16_t *Length, const char **Data);
int          U_WMRPOLYLINE_get(const char *contents, uint16_t *Length, const char **Data);
int          U_WMRESCAPE_get(const char *contents, uint16_t *Escape, uint16_t *Length, const char **Data);
int          U_WMRRESTOREDC_get(const char *contents, int16_t *DC);
int          U_WMRFILLREGION_get(const char *contents, uint16_t *Region, uint16_t *Brush);
int          U_WMRFRAMEREGION_get(const char *contents, uint16_t *Region, uint16_t *Brush, int16_t *Height, int16_t *Width);
int          U_WMRINVERTREGION_get(const char *contents, uint16_t *Region);
int          U_WMRPAINTREGION_get(const char *contents, uint16_t *Region);
int          U_WMRSELECTCLIPREGION_get(const char *contents, uint16_t *Region);
int          U_WMRSELECTOBJECT_get(const char *contents, uint16_t *Object);
int          U_WMRSETTEXTALIGN_get(const char *contents, uint16_t *Mode);
int          U_WMRDRAWTEXT_get(void); /* in GDI and Wine, not in WMF manual. */
int          U_WMRCHORD_get(const char *contents, U_POINT16 * Radial1, U_POINT16 * Radial2, U_RECT16 * rect);
int          U_WMRSETMAPPERFLAGS_get(const char *contents, uint32_t *Mode);
int          U_WMREXTTEXTOUT_get(const char *contents, U_POINT16 * Dst, int16_t *Length, uint16_t *Opts, const char **string, const int16_t **dx, U_RECT16 * rect);
int          U_WMRSETDIBTODEV_get(const char *contents, U_POINT16 * Dst, U_POINT16 * cwh, U_POINT16 * Src, uint16_t *cUsage, uint16_t *ScanCount, uint16_t *StartScan, const char **dib);
int          U_WMRSELECTPALETTE_get(const char *contents, uint16_t *Palette);
int          U_WMRREALIZEPALETTE_get(const char *contents);
int          U_WMRANIMATEPALETTE_get(const char *contents, U_PALETTE *Palette, const char **PalEntries);
int          U_WMRSETPALENTRIES_get(const char *contents, U_PALETTE *Palette, const char **PalEntries);
int          U_WMRPOLYPOLYGON_get(const char *contents, uint16_t *nPolys, const uint16_t **aPolyCounts, const char **Points);
int          U_WMRRESIZEPALETTE_get(const char *contents, uint16_t *Palette);
int          U_WMR3A_get(void);
int          U_WMR3B_get(void);
int          U_WMR3C_get(void);
int          U_WMR3D_get(void);
int          U_WMR3E_get(void);
int          U_WMR3F_get(void);
int          U_WMRDIBBITBLT_get(const char *contents, U_POINT16 * Dst, U_POINT16 * cwh, U_POINT16 * Src, uint32_t *dwRop3, const char **dib);
int          U_WMRDIBSTRETCHBLT_get(const char *contents, U_POINT16 * Dst, U_POINT16 * cDst, U_POINT16 * Src, U_POINT16 * cSrc, uint32_t *dwRop3, const char **dib);
int          U_WMRDIBCREATEPATTERNBRUSH_get(const char *contents, uint16_t *Style, uint16_t *cUsage, const char **Bm16, const char **dib);
int          U_WMRSTRETCHDIB_get(const char *contents, U_POINT16 * Dst, U_POINT16 * cDst, U_POINT16 * Src, U_POINT16 * cSrc, uint16_t *cUsage, uint32_t *dwRop3, const char **dib);
int          U_WMR44_get(void);
int          U_WMR45_get(void);
int          U_WMR46_get(void);
int          U_WMR47_get(void);
int          U_WMREXTFLOODFILL_get(const char *contents, uint16_t *Mode, U_COLORREF *Color, U_POINT16 * coord);
int          U_WMR49_get(void);
int          U_WMR4A_get(void);
int          U_WMR4B_get(void);
int          U_WMR4C_get(void);
int          U_WMR4D_get(void);
int          U_WMR4E_get(void);
int          U_WMR4F_get(void);
int          U_WMR50_get(void);
int          U_WMR51_get(void);
int          U_WMRABORTDOC_get(void);
int          U_WMR53_get(void);
int          U_WMR54_get(void);
int          U_WMR55_get(void);
int          U_WMR56_get(void);
int          U_WMR57_get(void);
int          U_WMR58_get(void);
int          U_WMR59_get(void);
int          U_WMR5A_get(void);
int          U_WMR5B_get(void);
int          U_WMR5C_get(void);
int          U_WMR5D_get(void);
int          U_WMR5E_get(void);
int          U_WMR5F_get(void);
int          U_WMR60_get(void);
int          U_WMR61_get(void);
int          U_WMR62_get(void);
int          U_WMR63_get(void);
int          U_WMR64_get(void);
int          U_WMR65_get(void);
int          U_WMR66_get(void);
int          U_WMR67_get(void);
int          U_WMR68_get(void);
int          U_WMR69_get(void);
int          U_WMR6A_get(void);
int          U_WMR6B_get(void);
int          U_WMR6C_get(void);
int          U_WMR6D_get(void);
int          U_WMR6E_get(void);
int          U_WMR6F_get(void);
int          U_WMR70_get(void);
int          U_WMR71_get(void);
int          U_WMR72_get(void);
int          U_WMR73_get(void);
int          U_WMR74_get(void);
int          U_WMR75_get(void);
int          U_WMR76_get(void);
int          U_WMR77_get(void);
int          U_WMR78_get(void);
int          U_WMR79_get(void);
int          U_WMR7A_get(void);
int          U_WMR7B_get(void);
int          U_WMR7C_get(void);
int          U_WMR7D_get(void);
int          U_WMR7E_get(void);
int          U_WMR7F_get(void);
int          U_WMR80_get(void);
int          U_WMR81_get(void);
int          U_WMR82_get(void);
int          U_WMR83_get(void);
int          U_WMR84_get(void);
int          U_WMR85_get(void);
int          U_WMR86_get(void);
int          U_WMR87_get(void);
int          U_WMR88_get(void);
int          U_WMR89_get(void);
int          U_WMR8A_get(void);
int          U_WMR8B_get(void);
int          U_WMR8C_get(void);
int          U_WMR8D_get(void);
int          U_WMR8E_get(void);
int          U_WMR8F_get(void);
int          U_WMR90_get(void);
int          U_WMR91_get(void);
int          U_WMR92_get(void);
int          U_WMR93_get(void);
int          U_WMR94_get(void);
int          U_WMR95_get(void);
int          U_WMR96_get(void);
int          U_WMR97_get(void);
int          U_WMR98_get(void);
int          U_WMR99_get(void);
int          U_WMR9A_get(void);
int          U_WMR9B_get(void);
int          U_WMR9C_get(void);
int          U_WMR9D_get(void);
int          U_WMR9E_get(void);
int          U_WMR9F_get(void);
int          U_WMRA0_get(void);
int          U_WMRA1_get(void);
int          U_WMRA2_get(void);
int          U_WMRA3_get(void);
int          U_WMRA4_get(void);
int          U_WMRA5_get(void);
int          U_WMRA6_get(void);
int          U_WMRA7_get(void);
int          U_WMRA8_get(void);
int          U_WMRA9_get(void);
int          U_WMRAA_get(void);
int          U_WMRAB_get(void);
int          U_WMRAC_get(void);
int          U_WMRAD_get(void);
int          U_WMRAE_get(void);
int          U_WMRAF_get(void);
int          U_WMRB0_get(void);
int          U_WMRB1_get(void);
int          U_WMRB2_get(void);
int          U_WMRB3_get(void);
int          U_WMRB4_get(void);
int          U_WMRB5_get(void);
int          U_WMRB6_get(void);
int          U_WMRB7_get(void);
int          U_WMRB8_get(void);
int          U_WMRB9_get(void);
int          U_WMRBA_get(void);
int          U_WMRBB_get(void);
int          U_WMRBC_get(void);
int          U_WMRBD_get(void);
int          U_WMRBE_get(void);
int          U_WMRBF_get(void);
int          U_WMRC0_get(void);
int          U_WMRC1_get(void);
int          U_WMRC2_get(void);
int          U_WMRC3_get(void);
int          U_WMRC4_get(void);
int          U_WMRC5_get(void);
int          U_WMRC6_get(void);
int          U_WMRC7_get(void);
int          U_WMRC8_get(void);
int          U_WMRC9_get(void);
int          U_WMRCA_get(void);
int          U_WMRCB_get(void);
int          U_WMRCC_get(void);
int          U_WMRCD_get(void);
int          U_WMRCE_get(void);
int          U_WMRCF_get(void);
int          U_WMRD0_get(void);
int          U_WMRD1_get(void);
int          U_WMRD2_get(void);
int          U_WMRD3_get(void);
int          U_WMRD4_get(void);
int          U_WMRD5_get(void);
int          U_WMRD6_get(void);
int          U_WMRD7_get(void);
int          U_WMRD8_get(void);
int          U_WMRD9_get(void);
int          U_WMRDA_get(void);
int          U_WMRDB_get(void);
int          U_WMRDC_get(void);
int          U_WMRDD_get(void);
int          U_WMRDE_get(void);
int          U_WMRDF_get(void);
int          U_WMRE0_get(void);
int          U_WMRE1_get(void);
int          U_WMRE2_get(void);
int          U_WMRE3_get(void);
int          U_WMRE4_get(void);
int          U_WMRE5_get(void);
int          U_WMRE6_get(void);
int          U_WMRE7_get(void);
int          U_WMRE8_get(void);
int          U_WMRE9_get(void);
int          U_WMREA_get(void);
int          U_WMREB_get(void);
int          U_WMREC_get(void);
int          U_WMRED_get(void);
int          U_WMREE_get(void);
int          U_WMREF_get(void);
int          U_WMRDELETEOBJECT_get(const char *contents, uint16_t *Object);
int          U_WMRF1_get(void);
int          U_WMRF2_get(void);
int          U_WMRF3_get(void);
int          U_WMRF4_get(void);
int          U_WMRF5_get(void);
int          U_WMRF6_get(void);
int          U_WMRCREATEPALETTE_get(const char *contents, U_PALETTE *Palette, const char **PalEntries);
int          U_WMRF8_get(void);
int          U_WMRCREATEPATTERNBRUSH_get(const char *contents, U_BITMAP16 *Bm16, int *pasize, const char **Pattern);
int          U_WMRCREATEPENINDIRECT_get(const char *contents, U_PEN *pen);
int          U_WMRCREATEFONTINDIRECT_get(const char *contents, const char **font);
int          U_WMRCREATEBRUSHINDIRECT_get(const char *contents, const char **brush);
int          U_WMRCREATEBITMAPINDIRECT_get(void);
int          U_WMRCREATEBITMAP_get(void);
int          U_WMRCREATEREGION_get(const char *contents, const char **Region);
//! \endcond


#ifdef __cplusplus
}
#endif

#endif /* _UWMF_ */
