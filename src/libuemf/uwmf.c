/**
  @file uwmf.c
  
  @brief Functions for manipulating WMF files and structures.

  [U_WMR*]_set all take data and return a pointer to memory holding the constructed record.  
    If something goes wrong a NULL pointer is returned.
  [U_WMR*]_get takes a pointer to memory and returns the length of that record as well
     as the values from it (in the provided fields, passed by reference.)
     If something goes wrong, a size of 0 is returned.

  The _set material comes first, then all of the _get material.
  
  Compile with "U_VALGRIND" defined defined to enable code which lets valgrind check each record for
  uninitialized data.
  
  Compile with "SOL8" defined for Solaris 8 or 9 (Sparc).
*/

/*
File:      uwmf.c
Version:   0.0.14
Date:      24-MAR-2014
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2014 David Mathog and California Institute of Technology (Caltech)
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h> /* for offsetof() */
#include <string.h>
#include <iconv.h>
#include <wchar.h>
#include <errno.h>
#include <string.h>
#include <limits.h> // for INT_MAX, INT_MIN
#include <math.h>   // for U_ROUND()
#if 0
#include <windef.h>    //Not actually used, looking for collisions
#include <winnt.h>    //Not actually used, looking for collisions
#include <wingdi.h>   //Not actually used, looking for collisions
#endif
#include "uwmf.h"
#include "uwmf_endian.h"

/**
    \brief Look up the full numeric type of a WMR record by type. 
        
    \return Full numeric value for this type of WMR record,  Returns 0xFFFFFFFF if out of range.
    \param idx WMR record type. 
    
*/
uint32_t U_wmr_values(int idx){
   int ret;
   int U_WMR_VALUES[256]={
      0x0000, //!< U_WMR_EOF
      0x0201, //!< U_WMR_SETBKCOLOR
      0x0102, //!< U_WMR_SETBKMODE
      0x0103, //!< U_WMR_SETMAPMODE
      0x0104, //!< U_WMR_SETROP2
      0x0105, //!< U_WMR_SETRELABS
      0x0106, //!< U_WMR_SETPOLYFILLMODE
      0x0107, //!< U_WMR_SETSTRETCHBLTMODE
      0x0108, //!< U_WMR_SETTEXTCHAREXTRA
      0x0209, //!< U_WMR_SETTEXTCOLOR
      0x020A, //!< U_WMR_SETTEXTJUSTIFICATION
      0x020B, //!< U_WMR_SETWINDOWORG
      0x020C, //!< U_WMR_SETWINDOWEXT
      0x020D, //!< U_WMR_SETVIEWPORTORG
      0x020E, //!< U_WMR_SETVIEWPORTEXT
      0x020F, //!< U_WMR_OFFSETWINDOWORG
      0x0410, //!< U_WMR_SCALEWINDOWEXT
      0x0211, //!< U_WMR_OFFSETVIEWPORTORG
      0x0412, //!< U_WMR_SCALEVIEWPORTEXT
      0x0213, //!< U_WMR_LINETO
      0x0214, //!< U_WMR_MOVETO
      0x0415, //!< U_WMR_EXCLUDECLIPRECT
      0x0416, //!< U_WMR_INTERSECTCLIPRECT
      0x0817, //!< U_WMR_ARC
      0x0418, //!< U_WMR_ELLIPSE
      0x0419, //!< U_WMR_FLOODFILL
      0x081A, //!< U_WMR_PIE
      0x041B, //!< U_WMR_RECTANGLE
      0x061C, //!< U_WMR_ROUNDRECT
      0x061D, //!< U_WMR_PATBLT
      0x001E, //!< U_WMR_SAVEDC
      0x041F, //!< U_WMR_SETPIXEL
      0x0220, //!< U_WMR_OFFSETCLIPRGN
      0x0521, //!< U_WMR_TEXTOUT
      0x0922, //!< U_WMR_BITBLT
      0x0B23, //!< U_WMR_STRETCHBLT
      0x0324, //!< U_WMR_POLYGON
      0x0325, //!< U_WMR_POLYLINE
      0x0626, //!< U_WMR_ESCAPE
      0x0127, //!< U_WMR_RESTOREDC
      0x0228, //!< U_WMR_FILLREGION
      0x0429, //!< U_WMR_FRAMEREGION
      0x012A, //!< U_WMR_INVERTREGION
      0x012B, //!< U_WMR_PAINTREGION
      0x012C, //!< U_WMR_SELECTCLIPREGION
      0x012D, //!< U_WMR_SELECTOBJECT
      0x012E, //!< U_WMR_SETTEXTALIGN
      0x062F, //!< U_WMR_DRAWTEXT
      0x0830, //!< U_WMR_CHORD
      0x0231, //!< U_WMR_SETMAPPERFLAGS
      0x0A32, //!< U_WMR_EXTTEXTOUT
      0x0D33, //!< U_WMR_SETDIBTODEV
      0x0234, //!< U_WMR_SELECTPALETTE
      0x0035, //!< U_WMR_REALIZEPALETTE
      0x0436, //!< U_WMR_ANIMATEPALETTE
      0x0037, //!< U_WMR_SETPALENTRIES
      0x0538, //!< U_WMR_POLYPOLYGON
      0x0139, //!< U_WMR_RESIZEPALETTE
      0x003A, //!< U_WMR_3A
      0x003B, //!< U_WMR_3B
      0x003C, //!< U_WMR_3C
      0x003D, //!< U_WMR_3D
      0x003E, //!< U_WMR_3E
      0x003F, //!< U_WMR_3F
      0x0940, //!< U_WMR_DIBBITBLT
      0x0B41, //!< U_WMR_DIBSTRETCHBLT
      0x0142, //!< U_WMR_DIBCREATEPATTERNBRUSH
      0x0F43, //!< U_WMR_STRETCHDIB
      0x0044, //!< U_WMR_44
      0x0045, //!< U_WMR_45
      0x0046, //!< U_WMR_46
      0x0047, //!< U_WMR_47
      0x0548, //!< U_WMR_EXTFLOODFILL
      0x0049, //!< U_WMR_49
      0x004A, //!< U_WMR_4A
      0x004B, //!< U_WMR_4B
      0x014C, //!< U_WMR_4C
      0x014D, //!< U_WMR_4D
      0x004E, //!< U_WMR_4E
      0x004F, //!< U_WMR_4F
      0x0050, //!< U_WMR_50
      0x0051, //!< U_WMR_51
      0x0052, //!< U_WMR_52
      0x0053, //!< U_WMR_53
      0x0054, //!< U_WMR_54
      0x0055, //!< U_WMR_55
      0x0056, //!< U_WMR_56
      0x0057, //!< U_WMR_57
      0x0058, //!< U_WMR_58
      0x0059, //!< U_WMR_59
      0x005A, //!< U_WMR_5A
      0x005B, //!< U_WMR_5B
      0x005C, //!< U_WMR_5C
      0x005D, //!< U_WMR_5D
      0x005E, //!< U_WMR_5E
      0x005F, //!< U_WMR_5F
      0x0060, //!< U_WMR_60
      0x0061, //!< U_WMR_61
      0x0062, //!< U_WMR_62
      0x0063, //!< U_WMR_63
      0x0064, //!< U_WMR_64
      0x0065, //!< U_WMR_65
      0x0066, //!< U_WMR_66
      0x0067, //!< U_WMR_67
      0x0068, //!< U_WMR_68
      0x0069, //!< U_WMR_69
      0x006A, //!< U_WMR_6A
      0x006B, //!< U_WMR_6B
      0x006C, //!< U_WMR_6C
      0x006D, //!< U_WMR_6D
      0x006E, //!< U_WMR_6E
      0x006F, //!< U_WMR_6F
      0x0070, //!< U_WMR_70
      0x0071, //!< U_WMR_71
      0x0072, //!< U_WMR_72
      0x0073, //!< U_WMR_73
      0x0074, //!< U_WMR_74
      0x0075, //!< U_WMR_75
      0x0076, //!< U_WMR_76
      0x0077, //!< U_WMR_77
      0x0078, //!< U_WMR_78
      0x0079, //!< U_WMR_79
      0x007A, //!< U_WMR_7A
      0x007B, //!< U_WMR_7B
      0x007C, //!< U_WMR_7C
      0x007D, //!< U_WMR_7D
      0x007E, //!< U_WMR_7E
      0x007F, //!< U_WMR_7F
      0x0080, //!< U_WMR_80
      0x0081, //!< U_WMR_81
      0x0082, //!< U_WMR_82
      0x0083, //!< U_WMR_83
      0x0084, //!< U_WMR_84
      0x0085, //!< U_WMR_85
      0x0086, //!< U_WMR_86
      0x0087, //!< U_WMR_87
      0x0088, //!< U_WMR_88
      0x0089, //!< U_WMR_89
      0x008A, //!< U_WMR_8A
      0x008B, //!< U_WMR_8B
      0x008C, //!< U_WMR_8C
      0x008D, //!< U_WMR_8D
      0x008E, //!< U_WMR_8E
      0x008F, //!< U_WMR_8F
      0x0090, //!< U_WMR_90
      0x0091, //!< U_WMR_91
      0x0092, //!< U_WMR_92
      0x0093, //!< U_WMR_93
      0x0094, //!< U_WMR_94
      0x0095, //!< U_WMR_95
      0x0096, //!< U_WMR_96
      0x0097, //!< U_WMR_97
      0x0098, //!< U_WMR_98
      0x0099, //!< U_WMR_99
      0x009A, //!< U_WMR_9A
      0x009B, //!< U_WMR_9B
      0x009C, //!< U_WMR_9C
      0x009D, //!< U_WMR_9D
      0x009E, //!< U_WMR_9E
      0x009F, //!< U_WMR_9F
      0x00A0, //!< U_WMR_A0
      0x00A1, //!< U_WMR_A1
      0x00A2, //!< U_WMR_A2
      0x00A3, //!< U_WMR_A3
      0x00A4, //!< U_WMR_A4
      0x00A5, //!< U_WMR_A5
      0x00A6, //!< U_WMR_A6
      0x00A7, //!< U_WMR_A7
      0x00A8, //!< U_WMR_A8
      0x00A9, //!< U_WMR_A9
      0x00AA, //!< U_WMR_AA
      0x00AB, //!< U_WMR_AB
      0x00AC, //!< U_WMR_AC
      0x00AD, //!< U_WMR_AD
      0x00AE, //!< U_WMR_AE
      0x00AF, //!< U_WMR_AF
      0x00B0, //!< U_WMR_B0
      0x00B1, //!< U_WMR_B1
      0x00B2, //!< U_WMR_B2
      0x00B3, //!< U_WMR_B3
      0x00B4, //!< U_WMR_B4
      0x00B5, //!< U_WMR_B5
      0x00B6, //!< U_WMR_B6
      0x00B7, //!< U_WMR_B7
      0x00B8, //!< U_WMR_B8
      0x00B9, //!< U_WMR_B9
      0x00BA, //!< U_WMR_BA
      0x00BB, //!< U_WMR_BB
      0x00BC, //!< U_WMR_BC
      0x00BD, //!< U_WMR_BD
      0x00BE, //!< U_WMR_BE
      0x00BF, //!< U_WMR_BF
      0x00C0, //!< U_WMR_C0
      0x00C1, //!< U_WMR_C1
      0x00C2, //!< U_WMR_C2
      0x00C3, //!< U_WMR_C3
      0x00C4, //!< U_WMR_C4
      0x00C5, //!< U_WMR_C5
      0x00C6, //!< U_WMR_C6
      0x00C7, //!< U_WMR_C7
      0x00C8, //!< U_WMR_C8
      0x00C9, //!< U_WMR_C9
      0x00CA, //!< U_WMR_CA
      0x00CB, //!< U_WMR_CB
      0x00CC, //!< U_WMR_CC
      0x00CD, //!< U_WMR_CD
      0x00CE, //!< U_WMR_CE
      0x00CF, //!< U_WMR_CF
      0x00D0, //!< U_WMR_D0
      0x00D1, //!< U_WMR_D1
      0x00D2, //!< U_WMR_D2
      0x00D3, //!< U_WMR_D3
      0x00D4, //!< U_WMR_D4
      0x00D5, //!< U_WMR_D5
      0x00D6, //!< U_WMR_D6
      0x00D7, //!< U_WMR_D7
      0x00D8, //!< U_WMR_D8
      0x00D9, //!< U_WMR_D9
      0x00DA, //!< U_WMR_DA
      0x00DB, //!< U_WMR_DB
      0x00DC, //!< U_WMR_DC
      0x00DD, //!< U_WMR_DD
      0x00DE, //!< U_WMR_DE
      0x00DF, //!< U_WMR_DF
      0x00E0, //!< U_WMR_E0
      0x00E1, //!< U_WMR_E1
      0x00E2, //!< U_WMR_E2
      0x00E3, //!< U_WMR_E3
      0x00E4, //!< U_WMR_E4
      0x00E5, //!< U_WMR_E5
      0x00E6, //!< U_WMR_E6
      0x00E7, //!< U_WMR_E7
      0x00E8, //!< U_WMR_E8
      0x00E9, //!< U_WMR_E9
      0x00EA, //!< U_WMR_EA
      0x00EB, //!< U_WMR_EB
      0x00EC, //!< U_WMR_EC
      0x00ED, //!< U_WMR_ED
      0x00EE, //!< U_WMR_EE
      0x00EF, //!< U_WMR_EF
      0x01F0, //!< U_WMR_DELETEOBJECT
      0x00F1, //!< U_WMR_F1
      0x00F2, //!< U_WMR_F2
      0x00F3, //!< U_WMR_F3
      0x00F4, //!< U_WMR_F4
      0x00F5, //!< U_WMR_F5
      0x00F6, //!< U_WMR_F6
      0x00F7, //!< U_WMR_CREATEPALETTE
      0x00F8, //!< U_WMR_CREATEBRUSH
      0x01F9, //!< U_WMR_CREATEPATTERNBRUSH
      0x02FA, //!< U_WMR_CREATEPENINDIRECT
      0x02FB, //!< U_WMR_CREATEFONTINDIRECT
      0x02FC, //!< U_WMR_CREATEBRUSHINDIRECT
      0x02FD, //!< U_WMR_CREATEBITMAPINDIRECT
      0x06FE, //!< U_WMR_CREATEBITMAP
      0x06FF  //!< U_WMR_CREATEREGION
   };
   if(idx<U_WMR_MIN || idx > U_WMR_MAX){ ret = 0xFFFFFFFF; }
   else {                                ret = U_WMR_VALUES[idx]; }
   return(ret);
}

/**
    \brief Look up the name of the WMR record by type.  Returns U_WMR_INVALID if out of range.
        
    \return name of the WMR record, "U_WMR_INVALID" if out of range.
    \param idx WMR record type. 
    
*/
const char *U_wmr_names(int idx){
   int ret;
   static const char *U_WMR_NAMES[257]={
      "U_WMR_EOF",
      "U_WMR_SETBKCOLOR",
      "U_WMR_SETBKMODE",
      "U_WMR_SETMAPMODE",
      "U_WMR_SETROP2",
      "U_WMR_SETRELABS",
      "U_WMR_SETPOLYFILLMODE",
      "U_WMR_SETSTRETCHBLTMODE",
      "U_WMR_SETTEXTCHAREXTRA",
      "U_WMR_SETTEXTCOLOR",
      "U_WMR_SETTEXTJUSTIFICATION",
      "U_WMR_SETWINDOWORG",
      "U_WMR_SETWINDOWEXT",
      "U_WMR_SETVIEWPORTORG",
      "U_WMR_SETVIEWPORTEXT",
      "U_WMR_OFFSETWINDOWORG",
      "U_WMR_SCALEWINDOWEXT",
      "U_WMR_OFFSETVIEWPORTORG",
      "U_WMR_SCALEVIEWPORTEXT",
      "U_WMR_LINETO",
      "U_WMR_MOVETO",
      "U_WMR_EXCLUDECLIPRECT",
      "U_WMR_INTERSECTCLIPRECT",
      "U_WMR_ARC",
      "U_WMR_ELLIPSE",
      "U_WMR_FLOODFILL",
      "U_WMR_PIE",
      "U_WMR_RECTANGLE",
      "U_WMR_ROUNDRECT",
      "U_WMR_PATBLT",
      "U_WMR_SAVEDC",
      "U_WMR_SETPIXEL",
      "U_WMR_OFFSETCLIPRGN",
      "U_WMR_TEXTOUT",
      "U_WMR_BITBLT",
      "U_WMR_STRETCHBLT",
      "U_WMR_POLYGON",
      "U_WMR_POLYLINE",
      "U_WMR_ESCAPE",
      "U_WMR_RESTOREDC",
      "U_WMR_FILLREGION",
      "U_WMR_FRAMEREGION",
      "U_WMR_INVERTREGION",
      "U_WMR_PAINTREGION",
      "U_WMR_SELECTCLIPREGION",
      "U_WMR_SELECTOBJECT",
      "U_WMR_SETTEXTALIGN",
      "U_WMR_DRAWTEXT",
      "U_WMR_CHORD",
      "U_WMR_SETMAPPERFLAGS",
      "U_WMR_EXTTEXTOUT",
      "U_WMR_SETDIBTODEV",
      "U_WMR_SELECTPALETTE",
      "U_WMR_REALIZEPALETTE",
      "U_WMR_ANIMATEPALETTE",
      "U_WMR_SETPALENTRIES",
      "U_WMR_POLYPOLYGON",
      "U_WMR_RESIZEPALETTE",
      "U_WMR_3A",
      "U_WMR_3B",
      "U_WMR_3C",
      "U_WMR_3D",
      "U_WMR_3E",
      "U_WMR_3F",
      "U_WMR_DIBBITBLT",
      "U_WMR_DIBSTRETCHBLT",
      "U_WMR_DIBCREATEPATTERNBRUSH",
      "U_WMR_STRETCHDIB",
      "U_WMR_44",
      "U_WMR_45",
      "U_WMR_46",
      "U_WMR_47",
      "U_WMR_EXTFLOODFILL",
      "U_WMR_49",
      "U_WMR_4A",
      "U_WMR_4B",
      "U_WMR_4C",
      "U_WMR_4D",
      "U_WMR_4E",
      "U_WMR_4F",
      "U_WMR_50",
      "U_WMR_51",
      "U_WMR_52",
      "U_WMR_53",
      "U_WMR_54",
      "U_WMR_55",
      "U_WMR_56",
      "U_WMR_57",
      "U_WMR_58",
      "U_WMR_59",
      "U_WMR_5A",
      "U_WMR_5B",
      "U_WMR_5C",
      "U_WMR_5D",
      "U_WMR_5E",
      "U_WMR_5F",
      "U_WMR_60",
      "U_WMR_61",
      "U_WMR_62",
      "U_WMR_63",
      "U_WMR_64",
      "U_WMR_65",
      "U_WMR_66",
      "U_WMR_67",
      "U_WMR_68",
      "U_WMR_69",
      "U_WMR_6A",
      "U_WMR_6B",
      "U_WMR_6C",
      "U_WMR_6D",
      "U_WMR_6E",
      "U_WMR_6F",
      "U_WMR_70",
      "U_WMR_71",
      "U_WMR_72",
      "U_WMR_73",
      "U_WMR_74",
      "U_WMR_75",
      "U_WMR_76",
      "U_WMR_77",
      "U_WMR_78",
      "U_WMR_79",
      "U_WMR_7A",
      "U_WMR_7B",
      "U_WMR_7C",
      "U_WMR_7D",
      "U_WMR_7E",
      "U_WMR_7F",
      "U_WMR_80",
      "U_WMR_81",
      "U_WMR_82",
      "U_WMR_83",
      "U_WMR_84",
      "U_WMR_85",
      "U_WMR_86",
      "U_WMR_87",
      "U_WMR_88",
      "U_WMR_89",
      "U_WMR_8A",
      "U_WMR_8B",
      "U_WMR_8C",
      "U_WMR_8D",
      "U_WMR_8E",
      "U_WMR_8F",
      "U_WMR_90",
      "U_WMR_91",
      "U_WMR_92",
      "U_WMR_93",
      "U_WMR_94",
      "U_WMR_95",
      "U_WMR_96",
      "U_WMR_97",
      "U_WMR_98",
      "U_WMR_99",
      "U_WMR_9A",
      "U_WMR_9B",
      "U_WMR_9C",
      "U_WMR_9D",
      "U_WMR_9E",
      "U_WMR_9F",
      "U_WMR_A0",
      "U_WMR_A1",
      "U_WMR_A2",
      "U_WMR_A3",
      "U_WMR_A4",
      "U_WMR_A5",
      "U_WMR_A6",
      "U_WMR_A7",
      "U_WMR_A8",
      "U_WMR_A9",
      "U_WMR_AA",
      "U_WMR_AB",
      "U_WMR_AC",
      "U_WMR_AD",
      "U_WMR_AE",
      "U_WMR_AF",
      "U_WMR_B0",
      "U_WMR_B1",
      "U_WMR_B2",
      "U_WMR_B3",
      "U_WMR_B4",
      "U_WMR_B5",
      "U_WMR_B6",
      "U_WMR_B7",
      "U_WMR_B8",
      "U_WMR_B9",
      "U_WMR_BA",
      "U_WMR_BB",
      "U_WMR_BC",
      "U_WMR_BD",
      "U_WMR_BE",
      "U_WMR_BF",
      "U_WMR_C0",
      "U_WMR_C1",
      "U_WMR_C2",
      "U_WMR_C3",
      "U_WMR_C4",
      "U_WMR_C5",
      "U_WMR_C6",
      "U_WMR_C7",
      "U_WMR_C8",
      "U_WMR_C9",
      "U_WMR_CA",
      "U_WMR_CB",
      "U_WMR_CC",
      "U_WMR_CD",
      "U_WMR_CE",
      "U_WMR_CF",
      "U_WMR_D0",
      "U_WMR_D1",
      "U_WMR_D2",
      "U_WMR_D3",
      "U_WMR_D4",
      "U_WMR_D5",
      "U_WMR_D6",
      "U_WMR_D7",
      "U_WMR_D8",
      "U_WMR_D9",
      "U_WMR_DA",
      "U_WMR_DB",
      "U_WMR_DC",
      "U_WMR_DD",
      "U_WMR_DE",
      "U_WMR_DF",
      "U_WMR_E0",
      "U_WMR_E1",
      "U_WMR_E2",
      "U_WMR_E3",
      "U_WMR_E4",
      "U_WMR_E5",
      "U_WMR_E6",
      "U_WMR_E7",
      "U_WMR_E8",
      "U_WMR_E9",
      "U_WMR_EA",
      "U_WMR_EB",
      "U_WMR_EC",
      "U_WMR_ED",
      "U_WMR_EE",
      "U_WMR_EF",
      "U_WMR_DELETEOBJECT",
      "U_WMR_F1",
      "U_WMR_F2",
      "U_WMR_F3",
      "U_WMR_F4",
      "U_WMR_F5",
      "U_WMR_F6",
      "U_WMR_CREATEPALETTE",
      "U_WMR_CREATEBRUSH",
      "U_WMR_CREATEPATTERNBRUSH",
      "U_WMR_CREATEPENINDIRECT",
      "U_WMR_CREATEFONTINDIRECT",
      "U_WMR_CREATEBRUSHINDIRECT",
      "U_WMR_CREATEBITMAPINDIRECT",
      "U_WMR_CREATEBITMAP",
      "U_WMR_CREATEREGION"
   };
   if(idx<U_WMR_MIN || idx > U_WMR_MAX){ ret = 256; }
   else {                                ret = idx; }
   return(U_WMR_NAMES[ret]);
}

/**
    \brief Text description of Escape record type.
    \return name of the WMR record, "UNKNOWN_ESCAPE" if out of range.
    \param idx Escape record type.     
*/
const char *U_wmr_escnames(int idx){
   const char *name;
   if(idx>=1 && idx <= 0x0023){
      switch(idx){
          case    0x0001:  name = "NEWFRAME";                      break;
          case    0x0002:  name = "ABORTDOC";                      break;
          case    0x0003:  name = "NEXTBAND";                      break;
          case    0x0004:  name = "SETCOLORTABLE";                 break;
          case    0x0005:  name = "GETCOLORTABLE";                 break;
          case    0x0006:  name = "FLUSHOUT";                      break;
          case    0x0007:  name = "DRAFTMODE";                     break;
          case    0x0008:  name = "QUERYESCSUPPORT";               break;
          case    0x0009:  name = "SETABORTPROC";                  break;
          case    0x000A:  name = "STARTDOC";                      break;
          case    0x000B:  name = "ENDDOC";                        break;
          case    0x000C:  name = "GETPHYSPAGESIZE";               break;
          case    0x000D:  name = "GETPRINTINGOFFSET";             break;
          case    0x000E:  name = "GETSCALINGFACTOR";              break;
          case    0x000F:  name = "META_ESCAPE_ENHANCED_METAFILE"; break;
          case    0x0010:  name = "SETPENWIDTH";                   break;
          case    0x0011:  name = "SETCOPYCOUNT";                  break;
          case    0x0012:  name = "SETPAPERSOURCE";                break;
          case    0x0013:  name = "PASSTHROUGH";                   break;
          case    0x0014:  name = "GETTECHNOLOGY";                 break;
          case    0x0015:  name = "SETLINECAP";                    break;
          case    0x0016:  name = "SETLINEJOIN";                   break;
          case    0x0017:  name = "SETMITERLIMIT";                 break;
          case    0x0018:  name = "BANDINFO";                      break;
          case    0x0019:  name = "DRAWPATTERNRECT";               break;
          case    0x001A:  name = "GETVECTORPENSIZE";              break;
          case    0x001B:  name = "GETVECTORBRUSHSIZE";            break;
          case    0x001C:  name = "ENABLEDUPLEX";                  break;
          case    0x001D:  name = "GETSETPAPERBINS";               break;
          case    0x001E:  name = "GETSETPRINTORIENT";             break;
          case    0x001F:  name = "ENUMPAPERBINS";                 break;
          case    0x0020:  name = "SETDIBSCALING";                 break;
          case    0x0021:  name = "EPSPRINTING";                   break;
          case    0x0022:  name = "ENUMPAPERMETRICS";              break;
          case    0x0023:  name = "GETSETPAPERMETRICS";            break;
      }
   }
   else if(idx == 0x0025){ name = "POSTSCRIPT_DATA";        }
   else if(idx == 0x0026){ name = "POSTSCRIPT_IGNORE";      }
   else if(idx == 0x002A){ name = "GETDEVICEUNITS";         }
   else if(idx == 0x0100){ name = "GETEXTENDEDTEXTMETRICS"; }
   else if(idx == 0x0102){ name = "GETPAIRKERNTABLE";       }
   else if(idx == 0x0200){ name = "EXTTEXTOUT";             }
   else if(idx == 0x0201){ name = "GETFACENAME";            }
   else if(idx == 0x0202){ name = "DOWNLOADFACE";           }
   else if(idx == 0x0801){ name = "METAFILE_DRIVER";        }
   else if(idx == 0x0C01){ name = "QUERYDIBSUPPORT";        }
   else if(idx == 0x1000){ name = "BEGIN_PATH";             }
   else if(idx == 0x1001){ name = "CLIP_TO_PATH";           }
   else if(idx == 0x1002){ name = "END_PATH";               }
   else if(idx == 0x100E){ name = "OPEN_CHANNEL";           }
   else if(idx == 0x100F){ name = "DOWNLOADHEADER";         }
   else if(idx == 0x1010){ name = "CLOSE_CHANNEL";          }
   else if(idx == 0x1013){ name = "POSTSCRIPT_PASSTHROUGH"; }
   else if(idx == 0x1014){ name = "ENCAPSULATED_POSTSCRIPT";}
   else if(idx == 0x1015){ name = "POSTSCRIPT_IDENTIFY";    }
   else if(idx == 0x1016){ name = "POSTSCRIPT_INJECTION";   }
   else if(idx == 0x1017){ name = "CHECKJPEGFORMAT";        }
   else if(idx == 0x1018){ name = "CHECKPNGFORMAT";         }
   else if(idx == 0x1019){ name = "GET_PS_FEATURESETTING";  }
   else if(idx == 0x101A){ name = "MXDC_ESCAPE";            }
   else if(idx == 0x11D8){ name = "SPCLPASSTHROUGH2";       }
   else {                  name = "UNKNOWN_ESCAPE";         }
   return(name);
}

//! \cond
/* one prototype from uwmf_endian.  Put it here because end user should never need to see it, so
not in uemf.h or uwmf_endian.h */
void U_swap2(void *ul, unsigned int count);
//! \endcond

/**
    \brief Derive from bounding box and start and end arc, for WMF arc, chord, or pie records, the center, start, and end points, and the bounding rectangle.
    
    \return 0 on success, other values on errors.
    \param rclBox16   Bounding box of Arc
    \param ArcStart16 Coordinates for Start of Arc
    \param ArcEnd16   Coordinates for End of Arc
    \param f1         1 if rotation angle >= 180, else 0
    \param f2         Rotation direction, 1 if counter clockwise, else 0
    \param center     Center coordinates
    \param start      Start coordinates (point on the ellipse defined by rect)
    \param end        End coordinates (point on the ellipse defined by rect)
    \param size       W,H of the x,y axes of the bounding rectangle.
*/
int wmr_arc_points(
       U_RECT16          rclBox16,
       U_POINT16         ArcStart16,
       U_POINT16         ArcEnd16,
       int              *f1,
       int               f2,
       U_PAIRF          *center,
       U_PAIRF          *start,
       U_PAIRF          *end,
       U_PAIRF          *size
    ){
    U_RECTL rclBox;
    U_POINTL ArcStart,ArcEnd;
    rclBox.left   = rclBox16.left;
    rclBox.top    = rclBox16.top;
    rclBox.right  = rclBox16.right;
    rclBox.bottom = rclBox16.bottom;
    ArcStart.x    = ArcStart16.x;
    ArcStart.y    = ArcStart16.y;
    ArcEnd.x      = ArcEnd16.x;
    ArcEnd.y      = ArcEnd16.y;
    return emr_arc_points_common(&rclBox, &ArcStart, &ArcEnd, f1, f2, center, start, end, size);
}

/**
    \brief A U_RECT16 may have its values swapped, L<->R and T<->B, this extracts the leftmost as left, and so forth.
    \param rc         U_RECT156 binary contents of an WMF file
    \param left       the leftmost  of rc.left and rc.right
    \param top        the topmost of rc.top and rc.bottom
    \param right      the rightmost of rc.left and rc.right
    \param bottom     the bottommost of rc.top and rc.bottom
*/
void U_sanerect16(U_RECT16 rc, double *left, double *top, double *right, double *bottom){
     if(rc.left < rc.right) { *left = rc.left;     *right  = rc.right;  }
     else {                   *left = rc.right;    *right  = rc.left;   }
     if(rc.top  < rc.bottom){ *top  = rc.top;      *bottom = rc.bottom; }
     else{                    *top  = rc.bottom;   *bottom = rc.top;    }
}

/* **********************************************************************************************
These definitions are for code pieces that are used many times in the following implementation.  These
definitions are not needed in end user code, so they are here rather than in uwmf.h.
*********************************************************************************************** */

/**
    \brief Get record size in bytes from U_WMR* record, which may not be aligned
    \return number of bytes in record.
*/
uint32_t U_wmr_size(const U_METARECORD *record){
   uint32_t Size16;
   memcpy(&Size16,record, 4);
   return(2*Size16); 
}

//! \cond    should never be called directly
#define SET_CB_FROM_PXBMI(A,B,C,D,E,F)    /* A=Px, B=Bmi, C=cbImage, D=cbImage4, E=cbBmi, F=cbPx */ \
   if(A){\
     if(!B)return(NULL);  /* size is derived from U_BITMAPINFO, but NOT from its size field, go figure*/ \
     C = F;\
     D = UP4(C);          /*  pixel array might not be a multiples of 4 bytes*/ \
     E    = U_SIZE_BITMAPINFOHEADER +  4 * get_real_color_count((char *)&(B->bmiHeader));  /*  bmiheader + colortable*/ \
   }\
   else { C = 0; D = 0; E=0; }
//! \endcond

/**
    \brief Create and return a U_FONT structure.
    \return pointer to the created U_FONT structure.
    \param Height          Height in Logical units
    \param Width           Average Width in Logical units
    \param Escapement      Angle in 0.1 degrees betweem escapement vector and X axis
    \param Orientation     Angle in 0.1 degrees between baseline and X axis
    \param Weight          LF_Weight Enumeration
    \param Italic          LF_Italic Enumeration
    \param Underline       LF_Underline Enumeration
    \param StrikeOut       LF_StrikeOut Enumeration
    \param CharSet         LF_CharSet Enumeration
    \param OutPrecision    LF_OutPrecision Enumeration
    \param ClipPrecision   LF_ClipPrecision Enumeration
    \param Quality         LF_Quality Enumeration
    \param PitchAndFamily  LF_PitchAndFamily Enumeration
    \param FaceName        Name of font.  ANSI Latin1, null terminated.
*/
U_FONT *U_FONT_set(
       int16_t  Height,             //!< Height in Logical units
       int16_t  Width,              //!< Average Width in Logical units
       int16_t  Escapement,         //!< Angle in 0.1 degrees betweem escapement vector and X axis
       int16_t  Orientation,        //!< Angle in 0.1 degrees between baseline and X axis
       int16_t  Weight,             //!< LF_Weight Enumeration
       uint8_t  Italic,             //!< LF_Italic Enumeration
       uint8_t  Underline,          //!< LF_Underline Enumeration
       uint8_t  StrikeOut,          //!< LF_StrikeOut Enumeration
       uint8_t  CharSet,            //!< LF_CharSet Enumeration
       uint8_t  OutPrecision,       //!< LF_OutPrecision Enumeration
       uint8_t  ClipPrecision,      //!< LF_ClipPrecision Enumeration
       uint8_t  Quality,            //!< LF_Quality Enumeration
       uint8_t  PitchAndFamily,     //!< LF_PitchAndFamily Enumeration
       char    *FaceName            //!< Name of font.  ANSI Latin1, null terminated.
    ){
    U_FONT *font;
    int slen = 1 + strlen(FaceName);  /* include terminator  */
    if(slen & 1)slen++;               /* storage length even */ 
    font = (U_FONT *) calloc(1,slen + U_SIZE_FONT_CORE); /* use calloc to auto fill in terminating '\0'*/
    if(font){
       font->Height         =  Height;           
       font->Width          =  Width;           
       font->Escapement     =  Escapement;      
       font->Orientation    =  Orientation;     
       font->Weight         =  Weight;          
       font->Italic         =  Italic;          
       font->Underline      =  Underline;       
       font->StrikeOut      =  StrikeOut;       
       font->CharSet        =  CharSet;         
       font->OutPrecision   =  OutPrecision;    
       font->ClipPrecision  =  ClipPrecision;   
       font->Quality        =  Quality;         
       font->PitchAndFamily =  PitchAndFamily;  
       strcpy((char *)&font->FaceName, FaceName);
    } 
    return(font);    
}

/**
    \brief Create and return a U_PLTENTRY structure.
    \return the created U_PLTENTRY structure.
    \param Color           Color for the U_PLTENTRY
*/
U_PLTNTRY U_PLTNTRY_set(U_COLORREF Color){
    U_PLTNTRY pe;
    pe.Value =  Color.Reserved;
    pe.Blue  =  Color.Blue;
    pe.Green =  Color.Green;
    pe.Red   =  Color.Red;
    return(pe);
}

/**
    \brief Create and return a U_PALETTE structure.
    \return pointer to the created U_PALETTE structure.
    \param Start       Either 0x0300 or an offset into the Palette table
    \param NumEntries  Number of U_LOGPLTNTRY objects
    \param PalEntries  Pointer to array of PaletteEntry Objects
*/
U_PALETTE *U_PLTENTRY_set(
       uint16_t            Start,              //!< Either 0x0300 or an offset into the Palette table
       uint16_t            NumEntries,         //!< Number of U_LOGPLTNTRY objects
       U_PLTNTRY          *PalEntries          //!< Pointer to array of PaletteEntry Objects
    ){
    U_PALETTE *Palette = NULL;
    if(NumEntries){
       Palette = malloc(4 + 4*NumEntries);
       if(Palette){
          Palette->Start      = Start;
          Palette->NumEntries = NumEntries;
          memcpy(&Palette->PalEntries, PalEntries, NumEntries*4);
       }
    }
    return(Palette); 
}

/**
    \brief Create and return a U_PEN structure.
    \return the created U_PEN structure.
    \param  Style    PenStyle Enumeration
    \param  Width    Width of Pen
    \param  Color    Pen Color.
*/
U_PEN U_PEN_set(
      uint16_t            Style,              //!< PenStyle Enumeration
      uint16_t            Width,              //!< Width of Pen
      U_COLORREF          Color               //!< Pen Color.
   ){
   U_PEN p;
   p.Style          = Style;
   p.Widthw[0]      = Width;
   p.Widthw[1]      = 0;     	 	     /* ignored */
   p.Color.Red      = Color.Red;
   p.Color.Green    = Color.Green;
   p.Color.Blue     = Color.Blue;
   p.Color.Reserved = Color.Reserved;
   return(p);
}

/**
    \brief Create and return a U_RECT16 structure from Upper Left and Lower Right corner points.
    \param ul upper left corner of rectangle
    \param lr lower right corner of rectangle
*/
U_RECT16 U_RECT16_set(
      U_POINT16 ul,
      U_POINT16 lr
   ){
   U_RECT16 rect;
   rect.left     =   ul.x;
   rect.top      =   ul.y;
   rect.right    =   lr.x;
   rect.bottom   =   lr.y;
   return(rect);
}

/**
    \brief Create and return a U_BITMAP16 structure
    \return pointer to the U_BITMAP16 structure, or NULL on failure
    \param Type       bitmap Type (not described at all in the WMF PDF)
    \param Width      bitmap width in pixels.
    \param Height     bitmap height in scan lines.
    \param LineN      each array line in Bits is a multiple of this (4 for a DIB)
    \param BitsPixel  number of adjacent color bits on each plane (R bits + G bits + B bits ????)
    \param Bits       bitmap pixel data. Bytes contained = (((Width * BitsPixel + 15) >> 4) << 1) * Height
*/
U_BITMAP16 *U_BITMAP16_set(
      const int16_t      Type,
      const int16_t      Width,
      const int16_t      Height,
      const int16_t      LineN,
      const uint8_t      BitsPixel,
      const char        *Bits
   ){
   U_BITMAP16 *bm16;
   uint32_t    irecsize;
   int         cbBits,iHeight;
   int         usedbytes;
   int16_t     WidthBytes;                                           //  total bytes per scan line (used and padding).

   usedbytes  = (Width * BitsPixel + 7)/8;                           // width of line in fully and partially occupied bytes
   WidthBytes =  (LineN * ((usedbytes + (LineN - 1) ) / LineN));     // Account for padding required by line alignment in the pixel array
   
   iHeight = (Height < 0 ? -Height : Height); /* DIB can use a negative height, but it does not look like a Bitmap16 object can */
   cbBits = WidthBytes * iHeight;
   if(!Bits || cbBits<=0)return(NULL);
   irecsize = U_SIZE_BITMAP16 + cbBits;
   bm16 = (U_BITMAP16 *) malloc(irecsize);
   if(bm16){
      bm16->Type       = Type;          
      bm16->Width      = Width;         
      bm16->Height     = iHeight;        
      bm16->WidthBytes = WidthBytes;    
      bm16->Planes     = 1;        
      bm16->BitsPixel  = BitsPixel;     
      memcpy((char *)bm16 + U_SIZE_BITMAP16,Bits,cbBits);
   }
   return(bm16);
}

/**
    \brief Create and return a U_SCAN structure
    \return U_SCAN structure
    \param count      Number of entries in the ScanLines array
    \param top        Y coordinate of the top scanline
    \param bottom     Y coordinate of the bottom scanline
    \param ScanLines  Array of 16 bit left/right pairs, array has 2*count entries
*/
U_SCAN *U_SCAN_set(
       uint16_t  count,                         //!< Number of entries in the ScanLines array
       uint16_t  top,                           //!< Y coordinate of the top scanline
       uint16_t  bottom,                        //!< Y coordinate of the bottom scanline
       uint16_t *ScanLines                      //!< Array of 16 bit left/right pairs, array has 2*count entries
    ){
    U_SCAN *scan=NULL;
    int size = 6 + count*4;
    scan = malloc(size);
    if(scan){
      scan->count     = count;   
      scan->top       = top;     
      scan->bottom    = bottom;  
      memcpy(&scan->ScanLines,ScanLines,4*count);
    }
    return(scan);
}

/**
    \brief Create and return a U_REGION structure
    \return pointer to created U_REGION structure or NULL on error
    \param  Size    aScans in bytes + regions size in bytes (size of this header plus all U_SCAN objects?)
    \param  sCount  number of scan objects in region (docs say scanlines, but then no way to add sizes)
    \param  sMax    largest number of points in any scan
    \param  sRect   bounding rectangle
    \param  aScans  series of U_SCAN objects to append. This is also an array of uint16_t, but should be handled as a bunch of U_SCAN objects tightly packed into the buffer.
*/
U_REGION *U_REGION_set(
       int16_t             Size,               //!< aScans in bytes + regions size in bytes (size of this header plus all U_SCAN objects?)
       int16_t             sCount,             //!< number of scan objects in region (docs say scanlines, but then no way to add sizes)
       int16_t             sMax,               //!< largest number of points in any scan
       U_RECT16            sRect,              //!< bounding rectangle
       uint16_t           *aScans              //!< series of U_SCAN objects to append. This is also an array of uint16_t, but should be handled as a bunch of U_SCAN objects tightly packed into the buffer.
   ){
   U_REGION *region=NULL;
   char *psc;
   int scansize,i,off;
   psc = (char *)aScans;
   for(scansize=i=0; i<sCount; i++){
      off       = 6 + 4*(((U_SCAN *)psc)->count);
      scansize += off;
      psc      += off;
   }
   region = malloc(U_SIZE_REGION + scansize);
   if(region){
      region->ignore1  =  0;   
      region->Type     =  0x0006;     
      region->ignore2  =  0;  
      region->Size     =  Size;     
      region->sCount   =  sCount;   
      region->sMax     =  sMax;     
      region->sRect    =  sRect;    
      memcpy(&region->aScans,aScans,scansize);
   }
   return(region);
}


/**
    \brief Create and return a U_WLOGBRUSH structure.
    \return the created U_WLOGBRUSH structure.
    \param  Style    BrushStyle Enumeration
    \param  Color    Brush Color value 
    \param  Hatch    HatchStyle Enumeration
*/
U_WLOGBRUSH U_WLOGBRUSH_set(
      uint16_t            Style,              //!< BrushStyle Enumeration
      U_COLORREF          Color,              //!< Brush Color value 
      uint16_t            Hatch               //!< HatchStyle Enumeration
   ){
   U_WLOGBRUSH lb;
   lb.Style          = Style;
   lb.Color.Red      = Color.Red;
   lb.Color.Green    = Color.Green;
   lb.Color.Blue     = Color.Blue;
   lb.Color.Reserved = Color.Reserved;
   lb.Hatch     = Hatch;
   return(lb);
}


/**
    \brief Create and return a U_PAIRF structure.
    \return pointer to the created U_PAIRF structure.
    \param  x  x value
    \param  y  y value
*/
U_PAIRF *U_PAIRF_set(
      float  x,              //!< x value
      float  y               //!< y value
   ){
   U_PAIRF *pf=malloc(U_SIZE_PAIRF);
   if(pf){
      pf->x = x;
      pf->y = y;
   }
   return(pf);
}

/* **********************************************************************************************
These functions are used for Image conversions and other
utility operations.  Character type conversions are in uwmf_utf.c
*********************************************************************************************** */

/**
    \brief Calculate the int16_t checksum of the buffer for the number of positions specified.  This is XOR of all values.
    \return checksum
    \param buf   array of uint16_t values
    \param count number of members in buf
    
*/
int16_t U_16_checksum(int16_t *buf, int count){
   int16_t result=0;
   for(;count;count--){
     result ^= *buf++;
   }
   return(result);
}

/**
    \brief Dump a WMFHANDLES structure.  Not for use in production code.
    \param string  Text to output before dumping eht structure
    \param handle  Handle
    \param wht     WMFHANDLES structure to dump
*/
void dumpwht(
     char         *string, 
     unsigned int *handle,
     WMFHANDLES   *wht
  ){
  uint32_t i;
  printf("%s\n",string);
  printf("lo: %d hi: %d peak: %d\n", wht->lolimit, wht->hilimit, wht->peak);
  if(handle){
    printf("handle: %d \n",*handle);
  }
  for(i=0;i<=5;i++){
     printf("table[%d]: %d\n",i,wht->table[i]);
  }
}

/**
    \brief Make up an approximate dx array to pass to U_WMREXTTEXTOUT_set(), based on character height and weight.
    
    Take abs. value of character height, get width by multiplying by 0.6, and correct weight
    approximately, with formula (measured on screen for one text line of Arial).
    Caller is responsible for free() on the returned pointer.
    
    \return pointer to dx array
    \param height  character height (absolute value will be used)
    \param weight  LF_Weight Enumeration (character weight) 
    \param members Number of entries to put into dx
    
*/
int16_t *dx16_set(
      int32_t  height,
      uint32_t weight,
      uint32_t members
   ){
   uint32_t i, width;
   int16_t *dx;
   dx = (int16_t *) malloc(members * sizeof(int16_t));
   if(dx){
       if(U_FW_DONTCARE == weight)weight=U_FW_NORMAL;
       width = (uint32_t) U_ROUND(((float) (height > 0 ? height : -height)) * 0.6 * (0.00024*(float) weight + 0.904));
       for ( i = 0; i < members; i++ ){ dx[i] = (width > INT16_MAX ? INT16_MAX : width); }
   }
   return(dx);
}
/**
    \brief Look up the properties (a bit map) of a type of WMR record.
          Bits that may be set are defined in "Draw Properties" in uemf.h, they are U_DRAW_NOTEMPTY, etc..
    \return bitmap of WMR record properties, or U_WMR_INVALID on error or release of all memory.
    \param type WMR record type.  If U_WMR_INVALID release memory. (There is no U_WMR_INVALID WMR record type)
    
*/
uint32_t U_wmr_properties(uint32_t type){
   static uint32_t *table=NULL;
   uint32_t result = U_WMR_INVALID;  // initialized to indicate an error (on a lookup) or nothing (on a memory release) 
   if(type == U_WMR_INVALID){
      if(table)free(table);
      table=NULL;
   }
   else if(type<=U_WMR_MAX){ // type is uint so always >=0, no need to test U_WMR_MIN, which is 0.
      if(!table){
         table = (uint32_t *) malloc(sizeof(uint32_t)*(1 + U_WMR_MAX));
         if(!table)return(result); 
   //                                                              0x200  0x100  0x80 0x40 0x20 0x10 0x08 0x04 0x02 0x01
   //                      properties (U_DRAW_*)                                 TEXT      ALTERS    ONLYTO    VISIBLE   
   //                                                              NOFILL OBJECT      PATH      FORCE     CLOSED    NOTEMPTY
   //                               Record Type
         table[0x00] = 0x0A0;    // U_WMREOF                        0      0      1    0    1    0    0    0    0    0  Force out any pending draw
         table[0x01] = 0x020;    // U_WMRSETBKCOLOR                 0      0      0    0    1    0    0    0    0    0
         table[0x02] = 0x020;    // U_WMRSETBKMODE                  0      0      0    0    1    0    0    0    0    0
         table[0x03] = 0x0A0;    // U_WMRSETMAPMODE                 0      0      1    0    1    0    0    0    0    0
         table[0x04] = 0x0A0;    // U_WMRSETROP2                    0      0      1    0    1    0    0    0    0    0
         table[0x05] = 0x000;    // U_WMRSETRELABS                  0      0      0    0    0    0    0    0    0    0  No idea what this is supposed to do
         table[0x06] = 0x0A0;    // U_WMRSETPOLYFILLMODE            0      0      1    0    1    0    0    0    0    0
         table[0x07] = 0x0A0;    // U_WMRSETSTRETCHBLTMODE          0      0      1    0    1    0    0    0    0    0
         table[0x08] = 0x000;    // U_WMRSETTEXTCHAREXTRA           0      0      0    0    0    0    0    0    0    0
         table[0x09] = 0x020;    // U_WMRSETTEXTCOLOR               0      0      0    0    1    0    0    0    0    0
         table[0x0A] = 0x020;    // U_WMRSETTEXTJUSTIFICATION       0      0      0    0    1    0    0    0    0    0
         table[0x0B] = 0x0A0;    // U_WMRSETWINDOWORG               0      0      1    0    1    0    0    0    0    0
         table[0x0C] = 0x0A0;    // U_WMRSETWINDOWEXT               0      0      1    0    1    0    0    0    0    0
         table[0x0D] = 0x0A0;    // U_WMRSETVIEWPORTORG             0      0      1    0    1    0    0    0    0    0
         table[0x0E] = 0x0A0;    // U_WMRSETVIEWPORTEXT             0      0      1    0    1    0    0    0    0    0
         table[0x0F] = 0x000;    // U_WMROFFSETWINDOWORG            0      0      0    0    0    0    0    0    0    0
         table[0x10] = 0x000;    // U_WMRSCALEWINDOWEXT             0      0      0    0    0    0    0    0    0    0
         table[0x11] = 0x0A0;    // U_WMROFFSETVIEWPORTORG          0      0      1    0    1    0    0    0    0    0
         table[0x12] = 0x0A0;    // U_WMRSCALEVIEWPORTEXT           0      0      1    0    1    0    0    0    0    0
         table[0x13] = 0x28B;    // U_WMRLINETO                     1      0      1    0    0    0    1    0    1    1
         table[0x14] = 0x289;    // U_WMRMOVETO                     1      0      1    0    0    0    1    0    0    1
         table[0x15] = 0x0A0;    // U_WMREXCLUDECLIPRECT            0      0      1    0    1    0    0    0    0    0
         table[0x16] = 0x0A0;    // U_WMRINTERSECTCLIPRECT          0      0      1    0    1    0    0    0    0    0
         table[0x17] = 0x283;    // U_WMRARC                        1      0      1    0    0    0    0    0    1    1
         table[0x18] = 0x087;    // U_WMRELLIPSE                    0      0      1    0    0    0    0    1    1    1
         table[0x19] = 0x082;    // U_WMRFLOODFILL                  0      0      1    0    0    0    0    0    1    0
         table[0x1A] = 0x087;    // U_WMRPIE                        0      0      1    0    0    0    0    1    1    1
         table[0x1B] = 0x087;    // U_WMRRECTANGLE                  0      0      1    0    0    0    0    1    1    1
         table[0x1C] = 0x087;    // U_WMRROUNDRECT                  0      0      1    0    0    0    0    1    1    1
         table[0x1D] = 0x000;    // U_WMRPATBLT                     0      0      1    0    0    0    0    1    1    1
         table[0x1E] = 0x0A0;    // U_WMRSAVEDC                     0      0      1    0    1    0    0    0    0    0
         table[0x1F] = 0x082;    // U_WMRSETPIXEL                   0      0      1    0    0    0    0    0    1    0
         table[0x20] = 0x0A0;    // U_WMROFFSETCLIPRGN              0      0      1    0    1    0    0    0    0    0
         table[0x21] = 0x002;    // U_WMRTEXTOUT                    0      0      0    0    0    0    0    0    1    0
         table[0x22] = 0x082;    // U_WMRBITBLT                     0      0      1    0    0    0    0    0    1    0
         table[0x23] = 0x082;    // U_WMRSTRETCHBLT                 0      0      1    0    0    0    0    0    1    0
         table[0x24] = 0x083;    // U_WMRPOLYGON                    0      0      1    0    0    0    0    0    1    1
         table[0x25] = 0x283;    // U_WMRPOLYLINE                   1      0      1    0    0    0    0    0    1    1
         table[0x26] = 0x0A0;    // U_WMRESCAPE                     0      0      1    0    1    0    0    0    0    0
         table[0x27] = 0x0A0;    // U_WMRRESTOREDC                  0      0      1    0    1    0    0    0    0    0
         table[0x28] = 0x082;    // U_WMRFILLREGION                 0      0      1    0    0    0    0    0    1    0
         table[0x29] = 0x082;    // U_WMRFRAMEREGION                0      0      1    0    0    0    0    0    1    0
         table[0x2A] = 0x082;    // U_WMRINVERTREGION               0      0      1    0    0    0    0    0    1    0
         table[0x2B] = 0x082;    // U_WMRPAINTREGION                0      0      1    0    0    0    0    0    1    0
         table[0x2C] = 0x0A0;    // U_WMRSELECTCLIPREGION           0      0      1    0    1    0    0    0    0    0
         table[0x2D] = 0x020;    // U_WMRSELECTOBJECT               0      0      0    0    1    0    0    0    0    0
         table[0x2E] = 0x020;    // U_WMRSETTEXTALIGN               0      0      0    0    1    0    0    0    0    0
         table[0x2F] = 0x002;    // U_WMRDRAWTEXT                   0      0      0    0    0    0    0    0    1    0 no idea what this is supposed to do
         table[0x30] = 0x087;    // U_WMRCHORD                      0      0      1    0    0    0    0    1    1    1
         table[0x31] = 0x0A0;    // U_WMRSETMAPPERFLAGS             0      0      1    0    1    0    0    0    0    0
         table[0x32] = 0x002;    // U_WMREXTTEXTOUT                 0      0      0    0    0    0    0    0    1    0
         table[0x33] = 0x000;    // U_WMRSETDIBTODEV                0      0      0    0    0    0    0    0    0    0
         table[0x34] = 0x0A0;    // U_WMRSELECTPALETTE              0      0      1    0    1    0    0    0    0    0
         table[0x35] = 0x0A0;    // U_WMRREALIZEPALETTE             0      0      1    0    1    0    0    0    0    0
         table[0x36] = 0x0A0;    // U_WMRANIMATEPALETTE             0      0      1    0    1    0    0    0    0    0
         table[0x37] = 0x0A0;    // U_WMRSETPALENTRIES              0      0      1    0    1    0    0    0    0    0
         table[0x38] = 0x087;    // U_WMRPOLYPOLYGON                0      0      1    0    0    0    0    1    1    1
         table[0x39] = 0x0A0;    // U_WMRRESIZEPALETTE              0      0      1    0    1    0    0    0    0    0
         table[0x3A] = 0x000;    // U_WMR3A                         0      0      0    0    0    0    0    0    0    0
         table[0x3B] = 0x000;    // U_WMR3B                         0      0      0    0    0    0    0    0    0    0
         table[0x3C] = 0x000;    // U_WMR3C                         0      0      0    0    0    0    0    0    0    0
         table[0x3D] = 0x000;    // U_WMR3D                         0      0      0    0    0    0    0    0    0    0
         table[0x3E] = 0x000;    // U_WMR3E                         0      0      0    0    0    0    0    0    0    0
         table[0x3F] = 0x000;    // U_WMR3F                         0      0      0    0    0    0    0    0    0    0
         table[0x40] = 0x0A0;    // U_WMRDIBBITBLT                  0      0      1    0    1    0    0    0    0    0
         table[0x41] = 0x0A0;    // U_WMRDIBSTRETCHBLT              0      0      1    0    1    0    0    0    0    0
         table[0x42] = 0x080;    // U_WMRDIBCREATEPATTERNBRUSH      0      0      1    0    0    0    0    0    0    0  Not selected yet, so no change in drawing conditions
         table[0x43] = 0x0A0;    // U_WMRSTRETCHDIB                 0      0      1    0    1    0    0    0    0    0
         table[0x44] = 0x000;    // U_WMR44                         0      0      0    0    0    0    0    0    0    0
         table[0x45] = 0x000;    // U_WMR45                         0      0      0    0    0    0    0    0    0    0
         table[0x46] = 0x000;    // U_WMR46                         0      0      0    0    0    0    0    0    0    0
         table[0x47] = 0x000;    // U_WMR47                         0      0      0    0    0    0    0    0    0    0
         table[0x48] = 0x082;    // U_WMREXTFLOODFILL               0      0      1    0    0    0    0    0    1    0
         table[0x49] = 0x000;    // U_WMR49                         0      0      0    0    0    0    0    0    0    0
         table[0x4A] = 0x000;    // U_WMR4A                         0      0      0    0    0    0    0    0    0    0
         table[0x4B] = 0x000;    // U_WMR4B                         0      0      0    0    0    0    0    0    0    0
         table[0x4C] = 0x000;    // U_WMR4C                         0      0      0    0    0    0    0    0    0    0
         table[0x4D] = 0x000;    // U_WMR4D                         0      0      0    0    0    0    0    0    0    0
         table[0x4E] = 0x000;    // U_WMR4E                         0      0      0    0    0    0    0    0    0    0
         table[0x4F] = 0x000;    // U_WMR4F                         0      0      0    0    0    0    0    0    0    0
         table[0x50] = 0x000;    // U_WMR50                         0      0      0    0    0    0    0    0    0    0
         table[0x51] = 0x000;    // U_WMR51                         0      0      0    0    0    0    0    0    0    0
         table[0x52] = 0x000;    // U_WMR52                         0      0      0    0    0    0    0    0    0    0
         table[0x53] = 0x000;    // U_WMR53                         0      0      0    0    0    0    0    0    0    0
         table[0x54] = 0x000;    // U_WMR54                         0      0      0    0    0    0    0    0    0    0
         table[0x55] = 0x000;    // U_WMR55                         0      0      0    0    0    0    0    0    0    0
         table[0x56] = 0x000;    // U_WMR56                         0      0      0    0    0    0    0    0    0    0
         table[0x57] = 0x000;    // U_WMR57                         0      0      0    0    0    0    0    0    0    0
         table[0x58] = 0x000;    // U_WMR58                         0      0      0    0    0    0    0    0    0    0
         table[0x59] = 0x000;    // U_WMR59                         0      0      0    0    0    0    0    0    0    0
         table[0x5A] = 0x000;    // U_WMR5A                         0      0      0    0    0    0    0    0    0    0
         table[0x5B] = 0x000;    // U_WMR5B                         0      0      0    0    0    0    0    0    0    0
         table[0x5C] = 0x000;    // U_WMR5C                         0      0      0    0    0    0    0    0    0    0
         table[0x5D] = 0x000;    // U_WMR5D                         0      0      0    0    0    0    0    0    0    0
         table[0x5E] = 0x000;    // U_WMR5E                         0      0      0    0    0    0    0    0    0    0
         table[0x5F] = 0x000;    // U_WMR5F                         0      0      0    0    0    0    0    0    0    0
         table[0x60] = 0x000;    // U_WMR60                         0      0      0    0    0    0    0    0    0    0
         table[0x61] = 0x000;    // U_WMR61                         0      0      0    0    0    0    0    0    0    0
         table[0x62] = 0x000;    // U_WMR62                         0      0      0    0    0    0    0    0    0    0
         table[0x63] = 0x000;    // U_WMR63                         0      0      0    0    0    0    0    0    0    0
         table[0x64] = 0x000;    // U_WMR64                         0      0      0    0    0    0    0    0    0    0
         table[0x65] = 0x000;    // U_WMR65                         0      0      0    0    0    0    0    0    0    0
         table[0x66] = 0x000;    // U_WMR66                         0      0      0    0    0    0    0    0    0    0
         table[0x67] = 0x000;    // U_WMR67                         0      0      0    0    0    0    0    0    0    0
         table[0x68] = 0x000;    // U_WMR68                         0      0      0    0    0    0    0    0    0    0
         table[0x69] = 0x000;    // U_WMR69                         0      0      0    0    0    0    0    0    0    0
         table[0x6A] = 0x000;    // U_WMR6A                         0      0      0    0    0    0    0    0    0    0
         table[0x6B] = 0x000;    // U_WMR6B                         0      0      0    0    0    0    0    0    0    0
         table[0x6C] = 0x000;    // U_WMR6C                         0      0      0    0    0    0    0    0    0    0
         table[0x6D] = 0x000;    // U_WMR6D                         0      0      0    0    0    0    0    0    0    0
         table[0x6E] = 0x000;    // U_WMR6E                         0      0      0    0    0    0    0    0    0    0
         table[0x6F] = 0x000;    // U_WMR6F                         0      0      0    0    0    0    0    0    0    0
         table[0x70] = 0x000;    // U_WMR70                         0      0      0    0    0    0    0    0    0    0
         table[0x71] = 0x000;    // U_WMR71                         0      0      0    0    0    0    0    0    0    0
         table[0x72] = 0x000;    // U_WMR72                         0      0      0    0    0    0    0    0    0    0
         table[0x73] = 0x000;    // U_WMR73                         0      0      0    0    0    0    0    0    0    0
         table[0x74] = 0x000;    // U_WMR74                         0      0      0    0    0    0    0    0    0    0
         table[0x75] = 0x000;    // U_WMR75                         0      0      0    0    0    0    0    0    0    0
         table[0x76] = 0x000;    // U_WMR76                         0      0      0    0    0    0    0    0    0    0
         table[0x77] = 0x000;    // U_WMR77                         0      0      0    0    0    0    0    0    0    0
         table[0x78] = 0x000;    // U_WMR78                         0      0      0    0    0    0    0    0    0    0
         table[0x79] = 0x000;    // U_WMR79                         0      0      0    0    0    0    0    0    0    0
         table[0x7A] = 0x000;    // U_WMR7A                         0      0      0    0    0    0    0    0    0    0
         table[0x7B] = 0x000;    // U_WMR7B                         0      0      0    0    0    0    0    0    0    0
         table[0x7C] = 0x000;    // U_WMR7C                         0      0      0    0    0    0    0    0    0    0
         table[0x7D] = 0x000;    // U_WMR7D                         0      0      0    0    0    0    0    0    0    0
         table[0x7E] = 0x000;    // U_WMR7E                         0      0      0    0    0    0    0    0    0    0
         table[0x7F] = 0x000;    // U_WMR7F                         0      0      0    0    0    0    0    0    0    0
         table[0x80] = 0x000;    // U_WMR80                         0      0      0    0    0    0    0    0    0    0
         table[0x81] = 0x000;    // U_WMR81                         0      0      0    0    0    0    0    0    0    0
         table[0x82] = 0x000;    // U_WMR82                         0      0      0    0    0    0    0    0    0    0
         table[0x83] = 0x000;    // U_WMR83                         0      0      0    0    0    0    0    0    0    0
         table[0x84] = 0x000;    // U_WMR84                         0      0      0    0    0    0    0    0    0    0
         table[0x85] = 0x000;    // U_WMR85                         0      0      0    0    0    0    0    0    0    0
         table[0x86] = 0x000;    // U_WMR86                         0      0      0    0    0    0    0    0    0    0
         table[0x87] = 0x000;    // U_WMR87                         0      0      0    0    0    0    0    0    0    0
         table[0x88] = 0x000;    // U_WMR88                         0      0      0    0    0    0    0    0    0    0
         table[0x89] = 0x000;    // U_WMR89                         0      0      0    0    0    0    0    0    0    0
         table[0x8A] = 0x000;    // U_WMR8A                         0      0      0    0    0    0    0    0    0    0
         table[0x8B] = 0x000;    // U_WMR8B                         0      0      0    0    0    0    0    0    0    0
         table[0x8C] = 0x000;    // U_WMR8C                         0      0      0    0    0    0    0    0    0    0
         table[0x8D] = 0x000;    // U_WMR8D                         0      0      0    0    0    0    0    0    0    0
         table[0x8E] = 0x000;    // U_WMR8E                         0      0      0    0    0    0    0    0    0    0
         table[0x8F] = 0x000;    // U_WMR8F                         0      0      0    0    0    0    0    0    0    0
         table[0x90] = 0x000;    // U_WMR90                         0      0      0    0    0    0    0    0    0    0
         table[0x91] = 0x000;    // U_WMR91                         0      0      0    0    0    0    0    0    0    0
         table[0x92] = 0x000;    // U_WMR92                         0      0      0    0    0    0    0    0    0    0
         table[0x93] = 0x000;    // U_WMR93                         0      0      0    0    0    0    0    0    0    0
         table[0x94] = 0x000;    // U_WMR94                         0      0      0    0    0    0    0    0    0    0
         table[0x95] = 0x000;    // U_WMR95                         0      0      0    0    0    0    0    0    0    0
         table[0x96] = 0x000;    // U_WMR96                         0      0      0    0    0    0    0    0    0    0
         table[0x97] = 0x000;    // U_WMR97                         0      0      0    0    0    0    0    0    0    0
         table[0x98] = 0x000;    // U_WMR98                         0      0      0    0    0    0    0    0    0    0
         table[0x99] = 0x000;    // U_WMR99                         0      0      0    0    0    0    0    0    0    0
         table[0x9A] = 0x000;    // U_WMR9A                         0      0      0    0    0    0    0    0    0    0
         table[0x9B] = 0x000;    // U_WMR9B                         0      0      0    0    0    0    0    0    0    0
         table[0x9C] = 0x000;    // U_WMR9C                         0      0      0    0    0    0    0    0    0    0
         table[0x9D] = 0x000;    // U_WMR9D                         0      0      0    0    0    0    0    0    0    0
         table[0x9E] = 0x000;    // U_WMR9E                         0      0      0    0    0    0    0    0    0    0
         table[0x9F] = 0x000;    // U_WMR9F                         0      0      0    0    0    0    0    0    0    0
         table[0xA0] = 0x000;    // U_WMRA0                         0      0      0    0    0    0    0    0    0    0
         table[0xA1] = 0x000;    // U_WMRA1                         0      0      0    0    0    0    0    0    0    0
         table[0xA2] = 0x000;    // U_WMRA2                         0      0      0    0    0    0    0    0    0    0
         table[0xA3] = 0x000;    // U_WMRA3                         0      0      0    0    0    0    0    0    0    0
         table[0xA4] = 0x000;    // U_WMRA4                         0      0      0    0    0    0    0    0    0    0
         table[0xA5] = 0x000;    // U_WMRA5                         0      0      0    0    0    0    0    0    0    0
         table[0xA6] = 0x000;    // U_WMRA6                         0      0      0    0    0    0    0    0    0    0
         table[0xA7] = 0x000;    // U_WMRA7                         0      0      0    0    0    0    0    0    0    0
         table[0xA8] = 0x000;    // U_WMRA8                         0      0      0    0    0    0    0    0    0    0
         table[0xA9] = 0x000;    // U_WMRA9                         0      0      0    0    0    0    0    0    0    0
         table[0xAA] = 0x000;    // U_WMRAA                         0      0      0    0    0    0    0    0    0    0
         table[0xAB] = 0x000;    // U_WMRAB                         0      0      0    0    0    0    0    0    0    0
         table[0xAC] = 0x000;    // U_WMRAC                         0      0      0    0    0    0    0    0    0    0
         table[0xAD] = 0x000;    // U_WMRAD                         0      0      0    0    0    0    0    0    0    0
         table[0xAE] = 0x000;    // U_WMRAE                         0      0      0    0    0    0    0    0    0    0
         table[0xAF] = 0x000;    // U_WMRAF                         0      0      0    0    0    0    0    0    0    0
         table[0xB0] = 0x000;    // U_WMRB0                         0      0      0    0    0    0    0    0    0    0
         table[0xB1] = 0x000;    // U_WMRB1                         0      0      0    0    0    0    0    0    0    0
         table[0xB2] = 0x000;    // U_WMRB2                         0      0      0    0    0    0    0    0    0    0
         table[0xB3] = 0x000;    // U_WMRB3                         0      0      0    0    0    0    0    0    0    0
         table[0xB4] = 0x000;    // U_WMRB4                         0      0      0    0    0    0    0    0    0    0
         table[0xB5] = 0x000;    // U_WMRB5                         0      0      0    0    0    0    0    0    0    0
         table[0xB6] = 0x000;    // U_WMRB6                         0      0      0    0    0    0    0    0    0    0
         table[0xB7] = 0x000;    // U_WMRB7                         0      0      0    0    0    0    0    0    0    0
         table[0xB8] = 0x000;    // U_WMRB8                         0      0      0    0    0    0    0    0    0    0
         table[0xB9] = 0x000;    // U_WMRB9                         0      0      0    0    0    0    0    0    0    0
         table[0xBA] = 0x000;    // U_WMRBA                         0      0      0    0    0    0    0    0    0    0
         table[0xBB] = 0x000;    // U_WMRBB                         0      0      0    0    0    0    0    0    0    0
         table[0xBC] = 0x000;    // U_WMRBC                         0      0      0    0    0    0    0    0    0    0
         table[0xBD] = 0x000;    // U_WMRBD                         0      0      0    0    0    0    0    0    0    0
         table[0xBE] = 0x000;    // U_WMRBE                         0      0      0    0    0    0    0    0    0    0
         table[0xBF] = 0x000;    // U_WMRBF                         0      0      0    0    0    0    0    0    0    0
         table[0xC0] = 0x000;    // U_WMRC0                         0      0      0    0    0    0    0    0    0    0
         table[0xC1] = 0x000;    // U_WMRC1                         0      0      0    0    0    0    0    0    0    0
         table[0xC2] = 0x000;    // U_WMRC2                         0      0      0    0    0    0    0    0    0    0
         table[0xC3] = 0x000;    // U_WMRC3                         0      0      0    0    0    0    0    0    0    0
         table[0xC4] = 0x000;    // U_WMRC4                         0      0      0    0    0    0    0    0    0    0
         table[0xC5] = 0x000;    // U_WMRC5                         0      0      0    0    0    0    0    0    0    0
         table[0xC6] = 0x000;    // U_WMRC6                         0      0      0    0    0    0    0    0    0    0
         table[0xC7] = 0x000;    // U_WMRC7                         0      0      0    0    0    0    0    0    0    0
         table[0xC8] = 0x000;    // U_WMRC8                         0      0      0    0    0    0    0    0    0    0
         table[0xC9] = 0x000;    // U_WMRC9                         0      0      0    0    0    0    0    0    0    0
         table[0xCA] = 0x000;    // U_WMRCA                         0      0      0    0    0    0    0    0    0    0
         table[0xCB] = 0x000;    // U_WMRCB                         0      0      0    0    0    0    0    0    0    0
         table[0xCC] = 0x000;    // U_WMRCC                         0      0      0    0    0    0    0    0    0    0
         table[0xCD] = 0x000;    // U_WMRCD                         0      0      0    0    0    0    0    0    0    0
         table[0xCE] = 0x000;    // U_WMRCE                         0      0      0    0    0    0    0    0    0    0
         table[0xCF] = 0x000;    // U_WMRCF                         0      0      0    0    0    0    0    0    0    0
         table[0xD0] = 0x000;    // U_WMRD0                         0      0      0    0    0    0    0    0    0    0
         table[0xD1] = 0x000;    // U_WMRD1                         0      0      0    0    0    0    0    0    0    0
         table[0xD2] = 0x000;    // U_WMRD2                         0      0      0    0    0    0    0    0    0    0
         table[0xD3] = 0x000;    // U_WMRD3                         0      0      0    0    0    0    0    0    0    0
         table[0xD4] = 0x000;    // U_WMRD4                         0      0      0    0    0    0    0    0    0    0
         table[0xD5] = 0x000;    // U_WMRD5                         0      0      0    0    0    0    0    0    0    0
         table[0xD6] = 0x000;    // U_WMRD6                         0      0      0    0    0    0    0    0    0    0
         table[0xD7] = 0x000;    // U_WMRD7                         0      0      0    0    0    0    0    0    0    0
         table[0xD8] = 0x000;    // U_WMRD8                         0      0      0    0    0    0    0    0    0    0
         table[0xD9] = 0x000;    // U_WMRD9                         0      0      0    0    0    0    0    0    0    0
         table[0xDA] = 0x000;    // U_WMRDA                         0      0      0    0    0    0    0    0    0    0
         table[0xDB] = 0x000;    // U_WMRDB                         0      0      0    0    0    0    0    0    0    0
         table[0xDC] = 0x000;    // U_WMRDC                         0      0      0    0    0    0    0    0    0    0
         table[0xDD] = 0x000;    // U_WMRDD                         0      0      0    0    0    0    0    0    0    0
         table[0xDE] = 0x000;    // U_WMRDE                         0      0      0    0    0    0    0    0    0    0
         table[0xDF] = 0x000;    // U_WMRDF                         0      0      0    0    0    0    0    0    0    0
         table[0xE0] = 0x000;    // U_WMRE0                         0      0      0    0    0    0    0    0    0    0
         table[0xE1] = 0x000;    // U_WMRE1                         0      0      0    0    0    0    0    0    0    0
         table[0xE2] = 0x000;    // U_WMRE2                         0      0      0    0    0    0    0    0    0    0
         table[0xE3] = 0x000;    // U_WMRE3                         0      0      0    0    0    0    0    0    0    0
         table[0xE4] = 0x000;    // U_WMRE4                         0      0      0    0    0    0    0    0    0    0
         table[0xE5] = 0x000;    // U_WMRE5                         0      0      0    0    0    0    0    0    0    0
         table[0xE6] = 0x000;    // U_WMRE6                         0      0      0    0    0    0    0    0    0    0
         table[0xE7] = 0x000;    // U_WMRE7                         0      0      0    0    0    0    0    0    0    0
         table[0xE8] = 0x000;    // U_WMRE8                         0      0      0    0    0    0    0    0    0    0
         table[0xE9] = 0x000;    // U_WMRE9                         0      0      0    0    0    0    0    0    0    0
         table[0xEA] = 0x000;    // U_WMREA                         0      0      0    0    0    0    0    0    0    0
         table[0xEB] = 0x000;    // U_WMREB                         0      0      0    0    0    0    0    0    0    0
         table[0xEC] = 0x000;    // U_WMREC                         0      0      0    0    0    0    0    0    0    0
         table[0xED] = 0x000;    // U_WMRED                         0      0      0    0    0    0    0    0    0    0
         table[0xEE] = 0x000;    // U_WMREE                         0      0      0    0    0    0    0    0    0    0
         table[0xEF] = 0x000;    // U_WMREF                         0      0      0    0    0    0    0    0    0    0
         table[0xF0] = 0x020;    // U_WMRDELETEOBJECT               0      0      0    0    1    0    0    0    0    0
         table[0xF1] = 0x000;    // U_WMRF1                         0      0      0    0    0    0    0    0    0    0
         table[0xF2] = 0x000;    // U_WMRF2                         0      0      0    0    0    0    0    0    0    0
         table[0xF3] = 0x000;    // U_WMRF3                         0      0      0    0    0    0    0    0    0    0
         table[0xF4] = 0x000;    // U_WMRF4                         0      0      0    0    0    0    0    0    0    0
         table[0xF5] = 0x000;    // U_WMRF5                         0      0      0    0    0    0    0    0    0    0
         table[0xF6] = 0x000;    // U_WMRF6                         0      0      0    0    0    0    0    0    0    0
         table[0xF7] = 0x120;    // U_WMRCREATEPALETTE              0      1      0    0    1    0    0    0    0    0 Not selected yet, so no change in drawing conditions
         table[0xF8] = 0x120;    // U_WMRCREATEBRUSH                0      1      0    0    1    0    0    0    0    0 "
         table[0xF9] = 0x120;    // U_WMRCREATEPATTERNBRUSH         0      1      0    0    1    0    0    0    0    0 "
         table[0xFA] = 0x120;    // U_WMRCREATEPENINDIRECT          0      1      0    0    1    0    0    0    0    0 "
         table[0xFB] = 0x120;    // U_WMRCREATEFONTINDIRECT         0      1      0    0    1    0    0    0    0    0 "
         table[0xFC] = 0x120;    // U_WMRCREATEBRUSHINDIRECT        0      1      0    0    1    0    0    0    0    0 "
         table[0xFD] = 0x020;    // U_WMRCREATEBITMAPINDIRECT       0      0      0    0    1    0    0    0    0    0 "
         table[0xFE] = 0x020;    // U_WMRCREATEBITMAP               0      0      0    0    1    0    0    0    0    0 "
         table[0xFF] = 0x120;    // U_WMRCREATEREGION               0      1      0    0    1    0    0    0    0    0 "
      }
      result = table[type];
   }
   return(result);
}

/* **********************************************************************************************
These functions are for setting up, appending to, and then tearing down an WMF structure, including
writing the final data structure out to a file.
*********************************************************************************************** */

/**
    \brief Duplicate an WMR record.
    \param wmr record to duplicate
*/
char *wmr_dup(
      const char *wmr
   ){
   char *dup;
   uint32_t  irecsize;

   if(!wmr)return(NULL);
   memcpy(&irecsize,wmr,4); /* Size16_4 field is at offset 0 */
   irecsize *= 2;
   dup=malloc(irecsize);
   if(dup){ memcpy(dup,wmr,irecsize); }
   return(dup);
}
    

/* some of these functions are identical to the emf ones, handled by defines in uemf.h,use the emf versions */

/**
    \brief Start constructing an wmf in memory. Supply the file name and initial size.
    \return 0 for success, >=0 for failure.
    \param name  WMF filename (will be opened)
    \param initsize Initialize WMF in memory to hold this many bytes
    \param chunksize When needed increase WMF in memory by this number of bytes
    \param wt WMF in memory
    
    
*/
int  wmf_start(
      const char       *name,
      const uint32_t   initsize,
      const uint32_t   chunksize,
      WMFTRACK       **wt
   ){
   FILE *fp;
   WMFTRACK *wtl=NULL;

   if(initsize < 1)return(1);
   if(chunksize < 1)return(2);
   if(!name)return(3);
   wtl = (WMFTRACK *) malloc(sizeof(WMFTRACK));
   if(!wtl)return(4);
   wtl->buf = malloc(initsize);  // no need to zero the memory
   if(!wtl->buf){
      free(wtl);
      return(5);
   }
   fp=wmf_fopen(name,U_WRITE);
   if(!fp){
      free(wtl->buf);
      free(wtl);
      return(6);
   }
   wtl->fp         =  fp;    
   wtl->allocated  =  initsize;
   wtl->used       =  0;
   wtl->records    =  0;
   wtl->PalEntries =  0;
   wtl->chunk      =  chunksize;
   wtl->largest    =  0;            /* only used by WMF */
   wtl->sumObjects =  0;            /* only used by WMF */
   (void) wmf_highwater(U_HIGHWATER_CLEAR);
   *wt=wtl;
   return(0);
}

/**
    \brief Release memory for an wmf structure in memory. Call this after wmf_finish().
    \return 0 on success, >=1 on failure
    \param wt WMF in memory
*/
int wmf_free(
      WMFTRACK **wt
   ){    
   WMFTRACK *wtl;
   if(!wt)return(1);
   wtl=*wt;
   if(!wtl)return(2);
   free(wtl->buf);
   free(wtl);
   *wt=NULL;
   (void)wmf_highwater(U_HIGHWATER_CLEAR);
   return(0);
}

/**
    \brief  Finalize the emf in memory and write it to the file.
    \return 0 on success, >=1 on failure
    \param wt WMF in memory
*/
int  wmf_finish(
      WMFTRACK   *wt
   ){
   char *record;
   int off;
   uint32_t tmp;
   uint16_t tmp16;

   if(!wt->fp)return(1);   // This could happen if something stomps on memory, otherwise should be caught in wmf_start

   // Set the header fields which were unknown up until this point
  
   
   if(((U_WMRPLACEABLE *) wt->buf)->Key == 0x9AC6CDD7){ off = U_SIZE_WMRPLACEABLE; }
   else {                                              off = 0;                   }
   
   record = (wt->buf + off);
   tmp = (wt->used)/2;
   memcpy(record + offsetof(U_WMRHEADER,Sizew),      &tmp, 4);    /* 16 bit words in file. not aligned */
   tmp = (wt->largest)/2;
   memcpy(record + offsetof(U_WMRHEADER,maxSize), &tmp, 4);   /*  16 bit words in largest record, not aligned */
   uint32_t maxobj = wmf_highwater(U_HIGHWATER_READ);
   if(maxobj > UINT16_MAX)return(3);
   tmp16 = maxobj;
   memcpy(record + offsetof(U_WMRHEADER,nObjects), &tmp16, 2);   /*  Total number of brushes, pens, and other graphics objects defined in this file */
  
#if U_BYTE_SWAP
    //This is a Big Endian machine, WMF data must be  Little Endian
    U_wmf_endian(wt->buf,wt->used,1); 
#endif

   (void) U_wmr_properties(U_WMR_INVALID);     /* force the release of the lookup table memory, returned value is irrelevant */
   if(1 != fwrite(wt->buf,wt->used,1,wt->fp))return(2);
   (void) fclose(wt->fp);
   wt->fp=NULL;
   return(0);
}

/**
    \brief Retrieve contents of an WMF file by name.
    \return 0 on success, >=1 on failure
    \param filename Name of file to open, including the path
    \param contents Contents of the file.  Buffer must be free()'d by caller.
    \param length   Number of bytes in Contents
*/
int wmf_readdata(
      const char   *filename,
      char        **contents,
      size_t       *length
   ){    
   FILE     *fp;
   int       status=0;

   *contents=NULL;
   fp=wmf_fopen(filename,U_READ);
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
            //This is a Big Endian machine, WMF data is Little Endian
            U_wmf_endian(*contents,*length,0);  // LE to BE
#endif
          }
      }
      fclose(fp);
   }
    return(status);
}

/**
    \brief Append an WMF record to a wmf in memory. This may reallocate buf memory.
    \return 0 for success, >=1 for failure.
    \param rec     Record to append to WMF in memory
    \param wt      WMF in memory
    \param freerec If true, free rec after append    
*/
int  wmf_append(
      U_METARECORD     *rec,
      WMFTRACK         *wt,
      int               freerec
   ){
   size_t deficit;
   uint32_t wp;
   uint32_t size;
   
   size = U_wmr_size(rec);
#ifdef U_VALGRIND
   printf("\nbefore \n");
   printf(" probe %d\n",memprobe(rec, size));
   printf("after \n");
#endif
   if(!rec)return(1);
   if(!wt)return(2);
   if(size + wt->used > wt->allocated){
      deficit = size + wt->used - wt->allocated;
      if(deficit < wt->chunk)deficit = wt->chunk;
      wt->allocated += deficit;
      wt->buf = realloc(wt->buf,wt->allocated);
      if(!wt->buf)return(3);
   }
   memcpy(wt->buf + wt->used, rec, size);
   wt->used += size;
   wt->records++;
   if(wt->largest < size)wt->largest=size;
   /* does the record create an object: brush, font, palette, pen, or region ? 
      Following EOF properties comes back as U_WMR_INVALID */
   wp = U_wmr_properties(U_WMRTYPE(rec));
   if((wp !=  U_WMR_INVALID) && (U_DRAW_OBJECT & wp))wt->sumObjects++;
   if(freerec){ free(rec); }
   return(0);
}

/**
    \brief Append an WMF header to a wmf in memory. This may reallocate buf memory.
    WMF header is not a normal record, method used to figure out its size is different.
    \return 0 for success, >=1 for failure.
    \param rec     header to append to WMF in memory
    \param wt      WMF in memory
    \param freerec If true, free rec after append    
*/
int  wmf_header_append(
      U_METARECORD     *rec,
      WMFTRACK         *wt,
      int               freerec
   ){
   size_t deficit;
   unsigned int hsize;
   
   hsize = (((U_WMRPLACEABLE *) rec)->Key == 0x9AC6CDD7 ? U_SIZE_WMRHEADER + U_SIZE_WMRPLACEABLE: U_SIZE_WMRHEADER);
   
#ifdef U_VALGRIND
   printf("\nbefore \n");
   printf(" probe %d\n",memprobe(rec, hsize));
   printf("after \n");
#endif
   if(!rec)return(1);
   if(!wt)return(2);
   if(U_wmr_size(rec) + wt->used > wt->allocated){
      deficit = hsize + wt->used - wt->allocated;
      if(deficit < wt->chunk)deficit = wt->chunk;
      wt->allocated += deficit;
      wt->buf = realloc(wt->buf,wt->allocated);
      if(!wt->buf)return(3);
   }
   memcpy(wt->buf + wt->used, rec, hsize);
   wt->used += hsize;
   /* do NOT increment records count, this is not strictly a record */
   if(wt->largest < hsize)wt->largest=hsize;
   if(freerec){ free(rec); }
   return(0);
}

/**
    \brief Keep track of the largest number used.
    \return The largest object number used.
    \param setval U_HIGHWATER_READ only return value, U_HIGHWATER_CLEAR also set value to 0, anything else, set to this if higher than stored.
*/
int wmf_highwater(uint32_t setval){
   static uint32_t value=0;
   uint32_t retval;
   if(setval == U_HIGHWATER_READ){
      retval = value;
   }
   else if(setval == U_HIGHWATER_CLEAR){
      retval = value;
      value = 0;
   }
   else {
      if(setval > value)value = setval; 
      retval = value;
   }
   return(retval);
}

/**
    \brief Create a handle table. Entries filled with 0 are empty, entries >0 hold a handle.
    \return 0 for success, >=1 for failure.
    \param initsize Initialize with space for this number of handles
    \param chunksize When needed increase space by this number of handles
    \param wht WMF handle table    
*/
int wmf_htable_create(
      uint32_t     initsize,
      uint32_t     chunksize,
      WMFHANDLES **wht
   ){
   WMFHANDLES *whtl;
   
   if(initsize<1)return(1);
   if(chunksize<1)return(2);
   whtl = (WMFHANDLES *) malloc(sizeof(WMFHANDLES));
   if(!whtl)return(3);
   whtl->table = malloc(initsize * sizeof(uint32_t));
   if(!whtl->table){
      free(whtl);
      return(4);
   }
   memset(whtl->table , 0, initsize * sizeof(uint32_t));  // zero all slots in the table
   whtl->allocated = initsize;
   whtl->chunk     = chunksize;
   whtl->table[0]  = 0;         // This slot isn't actually ever used
   whtl->lolimit   = 1;         // first available table entry
   whtl->hilimit   = 0;         // no entries in the table yet.
   whtl->peak      = 0;         // no entries in the table ever
   *wht            = whtl;
   return(0);
}

/**
    \brief Delete an entry from the handle table. Move it back onto the stack. The specified slot is filled with a 0.
    \return 0 for success, >=1 for failure.
    \param ih  handle
    \param wht WMF handle table
    
*/
int wmf_htable_delete(
      uint32_t    *ih,
      WMFHANDLES  *wht
   ){
   if(!wht)return(1);
   if(!wht->table)return(2);
   if(*ih < 1)return(4);           // invalid handle
   if(!wht->table[*ih])return(5);  // requested table position was not in use
   wht->table[*ih]=0;              // remove handle from table
   while(wht->hilimit>0 && !wht->table[wht->hilimit]){  // adjust hilimit
     wht->hilimit--;
   }
   if(*ih < wht->lolimit)wht->lolimit = *ih;  // adjust lolimit
   *ih=0;                                     // invalidate handle variable, so a second delete will of it is not possible
   return(0);
}

/**
    \brief Returns the index of the first free slot.  
    Call realloc() if needed.  The slot is set to handle (indicates occupied) and the peak value is adjusted.
    \return 0 for success, >=1 for failure.
    \param ih  handle
    \param wht WMF handle table
*/
int wmf_htable_insert(
      uint32_t   *ih,
      WMFHANDLES *wht
   ){
   size_t newsize;

   if(!wht)return(1);
   if(!wht->table)return(2);
   if(!ih)return(4);
   if(wht->lolimit >= wht->allocated - 1){  // need to reallocate
     newsize=wht->allocated + wht->chunk;
     wht->table = realloc(wht->table,newsize * sizeof(uint32_t));
     if(!wht->table)return(5);
     memset(&wht->table[wht->allocated] , 0, wht->chunk * sizeof(uint32_t));  // zero all NEW slots in the table
     wht->allocated = newsize;
   }
   *ih = wht->lolimit;              // handle that is inserted in first available slot
   wht->table[*ih] = *ih;           // handle goes into preexisting (but zero) slot in table, handle number is the same as the slot number
   if(*ih > wht->hilimit){    
      wht->hilimit = *ih;
      (void) wmf_highwater(wht->hilimit);
   }
   if(*ih > wht->peak){
      wht->peak    = *ih;
   }
   /* Find the next available slot, it will be at least one higher than the present position, and will have a zero in it. */
   wht->lolimit++;
   while(wht->lolimit<= wht->hilimit && wht->table[wht->lolimit]){ wht->lolimit++; }
   return(0);
}

/**
    \brief Free all memory in an htable.  Sets the pointer to NULL.
    \return 0 for success, >=1 for failure.
    \param wht  WMF handle table
*/
int wmf_htable_free(
      WMFHANDLES **wht
   ){
   WMFHANDLES *whtl;
   if(!wht)return(1);
   whtl = *wht;
   if(!whtl)return(2);
   if(!whtl->table)return(3);
   free(whtl->table);
   free(whtl);
   *wht=NULL;
   return(0);
}


/* **********************************************************************************************
These functions create standard structures used in the WMR records.
*********************************************************************************************** */

// hide these from Doxygen
//! \cond
/* **********************************************************************************************
These functions contain shared code used by various U_WMR*_print functions.  These should NEVER be called
by end user code and to further that end prototypes are NOT provided and they are hidden from Doxygen.   
*********************************************************************************************** */

/* These definitons only used here */
#define U_SIZE_WMR_NOARGS                6
#define U_SIZE_WMR_1ARG16                8
#define U_SIZE_WMR_2ARG16               10
#define U_SIZE_WMR_3ARG16               12
#define U_SIZE_WMR_4ARG16               14
#define U_SIZE_WMR_5ARG16               16
#define U_SIZE_WMR_6ARG16               18
#define U_SIZE_WMR_8ARG16               22

char *U_WMRCORENONE_set(char *string){
   printf("unimplemented creator for:%s\n",string);
   return(NULL);
}

void U_WMRCORE_SETRECHEAD(char *record, uint32_t irecsize, int iType){
   uint32_t  Size16;
   Size16 = irecsize/2;
   memcpy(record,&Size16,4); /*Size16_4 is at offset 0 in the record */
   ((U_METARECORD *) record)->iType   = iType;
   ((U_METARECORD *) record)->xb      = U_WMR_XB_FROM_TYPE(iType);
}

/* records that have no arguments */
char *U_WMRCORE_NOARGS_set(
      int iType
){
   char *record=NULL;
   uint32_t  irecsize;
   irecsize  = U_SIZE_METARECORD;
   record = malloc(irecsize);
   if(record)U_WMRCORE_SETRECHEAD(record,irecsize,iType);
   return(record);
}


/* records like U_WMRFLOODFILL and others. all args are optional, Color is not */
char *U_WMRCORE_1U16_CRF_2U16_set(
      int        iType,
      uint16_t   *arg1,
      U_COLORREF Color,
      uint16_t   *arg2,
      uint16_t   *arg3
){
   char *record=NULL;
   uint32_t  irecsize,off;
   irecsize  = U_SIZE_METARECORD + U_SIZE_COLORREF;
   if(arg1)irecsize+=2;
   if(arg2)irecsize+=2;
   if(arg3)irecsize+=2;
   record = malloc(irecsize);
   if(record){
      U_WMRCORE_SETRECHEAD(record,irecsize,iType);
      off = U_SIZE_METARECORD;
      if(arg1){ memcpy(record + off, arg1,    2); off+=2; }
                memcpy(record + off, &Color,  4); off+=4;
      if(arg2){ memcpy(record + off, arg2,    2); off+=2; }
      if(arg3){ memcpy(record + off, arg3,    2);         }
   }
   return(record);
}

/* records that have a single uint16_t argument like U_WMRSETMAPMODE
   May also be used with int16_t with appropriate casts */
char *U_WMRCORE_1U16_set(
      int        iType,
      uint16_t   arg1
){
   char *record=NULL;
   uint32_t  irecsize,off;
   irecsize  = U_SIZE_WMR_1ARG16;
   record = malloc(irecsize);
   if(record){
      U_WMRCORE_SETRECHEAD(record,irecsize,iType);
      off = U_SIZE_METARECORD;
      memcpy(record+off,&arg1,2);
   }
   return(record);
}

/* records that have two uint16_t arguments like U_WMRSETBKMODE 
   May also be used with int16_t with appropriate casts */
char *U_WMRCORE_2U16_set(
      int        iType,
      uint16_t   arg1,
      uint16_t   arg2
){
   char *record=NULL;
   uint32_t  irecsize,off;
   irecsize  = U_SIZE_WMR_2ARG16;
   record = malloc(irecsize);
   if(record){
      U_WMRCORE_SETRECHEAD(record,irecsize,iType);
      off = U_SIZE_METARECORD;
      memcpy(record+off,&arg1,2); off+=2;
      memcpy(record+off,&arg2,2);
   }
   return(record);
}

/* records that have four uint16_t arguments like U_WMRSCALEWINDOWEXT
   May also be used with int16_t with appropriate casts  */
char *U_WMRCORE_4U16_set(
      int        iType,
      uint16_t   arg1,
      uint16_t   arg2,
      uint16_t   arg3,
      uint16_t   arg4
){
   char *record=NULL;
   uint32_t  irecsize, off;
   irecsize  = U_SIZE_WMR_4ARG16;
   record = malloc(irecsize);
   if(record){
      U_WMRCORE_SETRECHEAD(record,irecsize,iType);
      off = U_SIZE_METARECORD;
      memcpy(record+off,&arg1,2); off+=2;
      memcpy(record+off,&arg2,2); off+=2;
      memcpy(record+off,&arg3,2); off+=2;
      memcpy(record+off,&arg4,2);
   }
   return(record);
}

/* records that have five uint16_t arguments like U_WMRCREATEPENINDIRECT
   May also be used with int16_t with appropriate casts  */
char *U_WMRCORE_5U16_set(
      int        iType,
      uint16_t   arg1,
      uint16_t   arg2,
      uint16_t   arg3,
      uint16_t   arg4,
      uint16_t   arg5
){
   char *record=NULL;
   uint32_t  irecsize, off;
   irecsize  = U_SIZE_WMR_5ARG16;
   record = malloc(irecsize);
   if(record){
      U_WMRCORE_SETRECHEAD(record,irecsize,iType);
      off = U_SIZE_METARECORD;
      memcpy(record+off,&arg1,2); off+=2;
      memcpy(record+off,&arg2,2); off+=2;
      memcpy(record+off,&arg3,2); off+=2;
      memcpy(record+off,&arg4,2); off+=2;
      memcpy(record+off,&arg5,2);
   }
   return(record);
}

/* records that have size uint16_t arguments like U_ROUNDREC
   May also be used with int16_t with appropriate casts  */
char *U_WMRCORE_6U16_set(
      int        iType,
      uint16_t   arg1,
      uint16_t   arg2,
      uint16_t   arg3,
      uint16_t   arg4,
      uint16_t   arg5,
      uint16_t   arg6
){
   char *record=NULL;
   uint32_t  irecsize, off;
   irecsize  = U_SIZE_WMR_6ARG16;
   record = malloc(irecsize);
   if(record){
      U_WMRCORE_SETRECHEAD(record,irecsize,iType);
      off = U_SIZE_METARECORD;
      memcpy(record+off,&arg1,2); off+=2;
      memcpy(record+off,&arg2,2); off+=2;
      memcpy(record+off,&arg3,2); off+=2;
      memcpy(record+off,&arg4,2); off+=2;
      memcpy(record+off,&arg5,2); off+=2;
      memcpy(record+off,&arg6,2);
   }
   return(record);
}

/* records that have eight uint16_t arguments like U_WMRARC
   May also be used with int16_t with appropriate casts  */
char *U_WMRCORE_8U16_set(
      int        iType,
      uint16_t   arg1,
      uint16_t   arg2,
      uint16_t   arg3,
      uint16_t   arg4,
      uint16_t   arg5,
      uint16_t   arg6,
      uint16_t   arg7,
      uint16_t   arg8
){
   char *record=NULL;
   uint32_t  irecsize, off;
   irecsize  = U_SIZE_WMR_8ARG16;
   record = malloc(irecsize);
   if(record){
      U_WMRCORE_SETRECHEAD(record,irecsize,iType);
      off = U_SIZE_METARECORD;
      memcpy(record+off,&arg1,2); off+=2;
      memcpy(record+off,&arg2,2); off+=2;
      memcpy(record+off,&arg3,2); off+=2;
      memcpy(record+off,&arg4,2); off+=2;
      memcpy(record+off,&arg5,2); off+=2;
      memcpy(record+off,&arg6,2); off+=2;
      memcpy(record+off,&arg7,2); off+=2;
      memcpy(record+off,&arg8,2);
   }
   return(record);
}

/* records that have
  arg1 an (optional) (u)int16
  arg2 an (optional( (u)int16
  N16 number of (u)int16_t cells in array. may be zero
  array of N16 (u)int16_t cells  (or any structure that is 2N bytes in size), should be NULL if N16 is 0.
  like U_WMRCREATEBRUSHINDIRECT with arg1=arg2=NULL
*/
char *U_WMRCORE_2U16_N16_set(
      int             iType,
      const uint16_t *arg1,
      const uint16_t *arg2,
      const uint16_t  N16,
      const void     *array
   ){
   char *record=NULL;
   uint32_t  irecsize, off;
   irecsize  = U_SIZE_METARECORD + N16*2;
   if(arg1)irecsize += 2;
   if(arg2)irecsize += 2;
   record = malloc(irecsize);
   if(record){
      U_WMRCORE_SETRECHEAD(record,irecsize,iType);
      off = U_SIZE_METARECORD;
      if(arg1){ memcpy(record+off,arg1,2); off+=2; }
      if(arg2){ memcpy(record+off,arg2,2); off+=2; }
      if(N16){  memcpy(record+off,array,2*N16);    }
   }
   return(record);
}



/* records that set a U_PALETTE , then a count and then a uint16_t list like U_WMRANIMATEPALETTE
   May also be used with int16_t with appropriate casts  */
char *U_WMRCORE_PALETTE_set(
      int        iType,
      const U_PALETTE *Palette
){
   char *record=NULL;
   uint32_t  irecsize, off, nPE;
   nPE = 4*Palette->NumEntries;
   if(!nPE)return(NULL);  /* What would it mean to load an empty palette??? */
   irecsize  = U_SIZE_METARECORD + 2 + 2 + nPE;
   record = malloc(irecsize);
   if(record){
      U_WMRCORE_SETRECHEAD(record,irecsize,iType);
      off = U_SIZE_METARECORD;
         memcpy(record+off, &Palette->Start,      2);   off+=2;
         memcpy(record+off, &Palette->NumEntries, 2);   off+=2;
         memcpy(record+off, &Palette->PalEntries, nPE);
   }
   return(record);
}

//! \endcond

/* **********************************************************************************************
These functions are simpler or more convenient ways to generate the specified types of WMR records.  
Each should be called in preference to the underlying "base" WMR function.
*********************************************************************************************** */


/**
    \brief Allocate and construct a U_WMRDELETEOBJECT record and also delete the requested object from the table.
    Use this function instead of calling U_WMRDELETEOBJECT_set() directly.
    Object Pointer in WMF (caller) is 0->N, but in htable it is 1->N+1, make that correction here.  
    \return pointer to the U_WMRDELETEOBJECT record, or NULL on error.
    \param ihObject  Pointer to handle to delete.  This value is set to 0xFFFFFFFF if the function succeeds.
    \param wht       WMF handle table
    
    Note that calling this function should always be conditional on the specifed object being defined.  It is easy to
    write a program where deleteobject_set() is called in a sequence where, at the time, we know that ihObject is defined.
    Then a later modification, possibly quite far away in the code, causes it to be undefined.  That distant change will
    result in a failure when this function reutrns.  That problem cannot be handled here because the only  values which
    may be returned are a valid U_WMRDELETEOBJECT record or a NULL, and other errors could result in the NULL.  
    So the object must be checked before the call.
*/
char *wdeleteobject_set(
      uint32_t    *ihObject,
      WMFHANDLES  *wht
   ){
   uint32_t saveObject=*ihObject;                   /* caller 0->N */
   *ihObject += 1;                                  /* caller 0->N --> 1->N+1 table*/
   if(wmf_htable_delete(ihObject,wht))return(NULL); /* invalid handle or other problem, cannot be deleted */
   *ihObject = 0xFFFFFFFF;                          /* EMF would have set to 0, but 0 is an allowed index in WMF */
   return(U_WMRDELETEOBJECT_set(saveObject));       /* caller 0->N */
}

/**
    \brief Allocate and construct a U_WMRSELECTOBJECT record, checks that the handle specified is one that can actually be selected.
    Use this function instead of calling U_WMRSELECTOBJECT_set() directly.
    Object Pointer in WMF (caller) is 0->N, so is record, so no correction to 1->N+1 needed here.  
    \return pointer to the U_WMRSELECTOBJECT record, or NULL on error.
    \param ihObject  handle to select
    \param wht       WMF handle table
*/
char *wselectobject_set(
      uint32_t    ihObject,
      WMFHANDLES *wht
   ){
   /* WMF has no stock objects! */
   if(ihObject > wht->hilimit)return(NULL);   // handle this high is not in the table
   /* caller uses 0->N, table uses 1->N+1 */
   if(!wht->table[ihObject+1])return(NULL);   // handle is not in the table, so cannot be selected
   /* file uses 0->N */
   return(U_WMRSELECTOBJECT_set(ihObject));
}

/**
    \brief Allocate and construct a U_WMRCREATEPENINDIRECT record, create a handle and returns it
    Use this function instead of calling U_WMRCREATEPENINDIRECT_set() directly.
    Object Pointer in WMF (caller) is 0->N, but in htable it is 1->N+1, make that correction here.  
    \return pointer to the U_WMRCREATEPENINDIRECT record, or NULL on error.
    \param ihPen handle to be used by new object
    \param wht   WMF handle table 
    \param pen   Pen parameters (U_PEN)
*/
char *wcreatepenindirect_set(
      uint32_t   *ihPen,
      WMFHANDLES *wht,
      U_PEN       pen
   ){
   if(wmf_htable_insert(ihPen, wht))return(NULL);
   *ihPen -= 1;  /* 1->N+1 --> 0->N */
   return(U_WMRCREATEPENINDIRECT_set(pen));
}

/**
    \brief Allocate and construct a U_WMRCREATEBRUSHINDIRECT record, create a handle and returns it
    Use this function instead of calling U_WMRCREATEBRUSHINDIRECT_set() directly.
    Object Pointer in WMF (caller) is 0->N, but in htable it is 1->N+1, make that correction here.  
    \return pointer to the U_WMRCREATEBRUSHINDIRECT record, or NULL on error.
    \param ihBrush handle to be used by new object 
    \param wht     WMF handle table 
    \param lb      Brush parameters  
*/
char *wcreatebrushindirect_set(
      uint32_t    *ihBrush,
      WMFHANDLES  *wht,
      U_WLOGBRUSH   lb
   ){
   if(wmf_htable_insert(ihBrush, wht))return(NULL);
   *ihBrush -= 1;  /* 1->N+1 --> 0->N */
   return(U_WMRCREATEBRUSHINDIRECT_set(lb));
}

/**
    \brief Allocate and construct a U_WMRDIBCREATEPATTERNBRUSH record from a DIB.
    Use this function instead of calling U_WMRDIBCREATEPATTERNBRUSH_set() directly.
    \return pointer to the U_WMRDIBCREATEPATTERNBRUSH record, or NULL on error.
    \param ihBrush handle to be used by new object 
    \param wht     WMF handle table 
    \param iUsage  DIBColors enumeration
    \param Bmi     Bitmap info
    \param cbPx    Size in bytes of pixel array (row stride * height, there may be some padding at the end of each row)
    \param Px      (Optional) bitmapbuffer (pixel array section )
*/
char *wcreatedibpatternbrush_srcdib_set(
      uint32_t            *ihBrush,
      WMFHANDLES          *wht,
      const uint32_t       iUsage, 
      const U_BITMAPINFO  *Bmi,
      const uint32_t       cbPx,
      const char          *Px
      
   ){
   if(wmf_htable_insert(ihBrush, wht))return(NULL);
   *ihBrush -= 1;  /* 1->N+1 --> 0->N */
   return(U_WMRDIBCREATEPATTERNBRUSH_set(U_BS_DIBPATTERNPT, iUsage, Bmi, cbPx, Px,NULL));
}

/**
    \brief Allocate and construct a U_WMRDIBCREATEPATTERNBRUSH record from a U_BITMAP16 object.
    Use this function instead of calling U_WMRDIBCREATEPATTERNBRUSH_set() directly.
    \return pointer to the U_WMRDIBCREATEPATTERNBRUSH record, or NULL on error.
    \param ihBrush handle to be used by new object 
    \param wht     WMF handle table 
    \param iUsage  DIBColors enumeration
    \param Bm16    Pointer to a Bitmap16 object
*/
char *wcreatedibpatternbrush_srcbm16_set(
      uint32_t            *ihBrush,
      WMFHANDLES          *wht,
      const uint32_t       iUsage, 
      const U_BITMAP16    *Bm16
   ){
   if(wmf_htable_insert(ihBrush, wht))return(NULL);
   *ihBrush -= 1;  /* 1->N+1 --> 0->N */
   return(U_WMRDIBCREATEPATTERNBRUSH_set(U_BS_PATTERN, iUsage, NULL, 0, NULL, Bm16));
}

/**
    \brief Allocate and construct a U_WMRCREATEPATTERNBRUSH record, create a handle and returns it
    Use this function instead of calling U_WMRCREATEPATTERNBRUSH_set() directly.
    WARNING - U_WMRCREATEPATTERNBRUSH has been declared obsolete and application support is spotty -
    use U_WMRDIBCREATEPATTERNBRUSH instead.
    \return pointer to the U_WMRCREATEPATTERNBRUSH record, or NULL on error.
    \param ihBrush handle to be used by new object 
    \param wht     WMF handle table 
    \param Bm16    Pointer to a Bitmap16 structure (only first 10 bytes are used).
    \param Pattern Pointer to a byte array described by Bm16.  (Pattern may be a pointer to the BM16 Bits field.)
*/
char *wcreatepatternbrush_set(
      uint32_t            *ihBrush,
      WMFHANDLES          *wht,
      U_BITMAP16          *Bm16,
      char                *Pattern       
   ){
   if(wmf_htable_insert(ihBrush, wht))return(NULL);
   *ihBrush -= 1;  /* 1->N+1 --> 0->N */
   return(U_WMRCREATEPATTERNBRUSH_set(Bm16, Pattern));
}

/**
    \brief Allocate and construct a U_WMRCREATEFONTINDIRECT record, create a handle and returns it
    Use this function instead of calling U_WMRCREATEFONTINDIRECT_set() directly.
    Object Pointer in WMF (caller) is 0->N, but in htable it is 1->N+1, make that correction here.  
    \return pointer to the U_WMRCREATEFONTINDIRECT record, or NULL on error.
    \param ihFont  Font handle, will be created and returned
    \param wht     Pointer to structure holding all WMF handles
    \param uf      Pointer to Font parameters as U_FONT *
*/
char *wcreatefontindirect_set(
      uint32_t   *ihFont,
      WMFHANDLES *wht,
      U_FONT     *uf
   ){
   if(wmf_htable_insert(ihFont, wht))return(NULL);
   *ihFont -= 1;  /* 1->N+1 --> 0->N */
   return(U_WMRCREATEFONTINDIRECT_set(uf));
}

/**
    \brief Allocate and construct a U_WMRCREATEPALETTE record, create a handle and returns it
    Use this function instead of calling U_WMRCREATEPALETTE_set() directly.
    Object Pointer in WMF (caller) is 0->N, but in htable it is 1->N+1, make that correction here.  
    \return pointer to the U_WMRCREATEPALETTE record, or NULL on error.
    \param ihPal  Palette handle, will be created and returned
    \param wht    Pointer to structure holding all WMF handles
    \param up     Palette parameters
*/
char *wcreatepalette_set(
      uint32_t     *ihPal,
      WMFHANDLES   *wht,
      U_PALETTE    *up
   ){
   if(wmf_htable_insert(ihPal, wht))return(NULL);
   *ihPal -= 1;  /* 1->N+1 --> 0->N */
   return(U_WMRCREATEPALETTE_set(up));
}

/**
    \brief Allocate and construct a U_WMRSETPALENTRIES record, create a handle and returns it
    Use this function instead of calling U_WMRSETPALENTRIES_set() directly.
    Object Pointer in WMF (caller) is 0->N, but in htable it is 1->N+1, make that correction here.  
    \return pointer to the U_WMRSETPALENTRIES record, or NULL on error.
    \param ihPal       Palette handle, will be created and returned
    \param wht         Pointer to structure holding all WMF handles
    \param Palettes    Values to set with
*/
char *wsetpaletteentries_set(
      uint32_t               *ihPal,
      WMFHANDLES             *wht,
      const U_PALETTE        *Palettes
   ){
   if(wmf_htable_insert(ihPal, wht))return(NULL);
   *ihPal -= 1;  /* 1->N+1 --> 0->N */
   return(U_WMRSETPALENTRIES_set(Palettes));
}

/**
    \brief Allocate and construct a U_WMRCREATEREGION record, create a handle and returns it
    Use this function instead of calling U_WMRCREATEREGION() directly.
    Object Pointer in WMF (caller) is 0->N, but in htable it is 1->N+1, make that correction here.  
    \return pointer to the U_REGIONS record, or NULL on error.
    \param ihReg       Region handle, will be created and returned
    \param wht         Pointer to structure holding all WMF handles
    \param Region     Values to set with
*/
char *wcreateregion_set(
      uint32_t               *ihReg,
      WMFHANDLES             *wht,
      const U_REGION         *Region
   ){
   if(wmf_htable_insert(ihReg, wht))return(NULL);
   *ihReg -= 1;  /* 1->N+1 --> 0->N */
   return(U_WMRCREATEREGION_set(Region));
}
/* A few escape functions are implemented, just those that set a state or a single value */

/**
    \brief Allocate and construct the specified U_WMRESCAPE structure, create a handle and returns it
    Use this function instead of calling U_WMRESCAPE_set() directly.
    \return pointer to the U_WMRESCAPE structure, or NULL on error.
*/
char *wbegin_path_set(void){
   return(U_WMRESCAPE_set(U_MFE_BEGIN_PATH,0,NULL));
}

/**
    \brief Allocate and construct the specified U_WMRESCAPE structure, create a handle and returns it
    Use this function instead of calling U_WMRESCAPE_set() directly.
    \return pointer to the U_WMRESCAPE structure, or NULL on error.
*/
char *wend_path_set(void){
   return(U_WMRESCAPE_set(U_MFE_END_PATH,0,NULL));
}

/**
    \brief Allocate and construct the specified U_WMRESCAPE structure, create a handle and returns it
    Use this function instead of calling U_WMRESCAPE_set() directly.
    \return pointer to the U_WMRESCAPE structure, or NULL on error.
    \param Type       PostScriptCap Enumeration, anything else is an error
*/
char *wlinecap_set(
      int32_t               Type
   ){
   char *record =NULL;
   if(Type == U_WPS_CAP_NOTSET || 
      Type == U_WPS_CAP_FLAT   ||
      Type == U_WPS_CAP_ROUND  ||
      Type == U_WPS_CAP_SQUARE){ record = U_WMRESCAPE_set(U_MFE_SETLINECAP,4,&Type); }
   return(record);
}

/**
    \brief Allocate and construct the specified U_WMRESCAPE structure, create a handle and returns it
    Use this function instead of calling U_WMRESCAPE_set() directly.
    \return pointer to the U_WMRESCAPE structure, or NULL on error.
    \param Type       PostScriptCap Enumeration, anything else is an error
*/
char *wlinejoin_set(
      int32_t               Type
   ){
   char *record =NULL;
   if(Type == U_WPS_JOIN_NOTSET || 
      Type == U_WPS_JOIN_MITER  ||
      Type == U_WPS_JOIN_ROUND  ||
      Type == U_WPS_JOIN_BEVEL){ record = U_WMRESCAPE_set(U_MFE_SETLINEJOIN,4,&Type); }
   return(record);
}

/**
    \brief Allocate and construct the specified U_WMRESCAPE structure, create a handle and returns it
    Use this function instead of calling U_WMRESCAPE_set() directly.
    \return pointer to the U_WMRESCAPE structure, or NULL on error.
    \param limit       PostScriptCap Enumeration, anything else is an error
*/
char *wmiterlimit_set(
      int32_t               limit
   ){
   return(U_WMRESCAPE_set(U_MFE_SETMITERLIMIT,4,&limit));
}

/* **********************************************************************************************
These are the core WMR functions, each creates a particular type of record.  
All return these records via a char* pointer, which is NULL if the call failed.  
They are listed in order by the corresponding U_WMR_* index number.  
*********************************************************************************************** */

/**
    \brief Set up fields for a (placeable) WMR_HEADER.  Most of the fields are blank and are not set until all is written.
    Typically values are something like (8.5,11.0), 1440 (Letter paper, 1440 DPI).
    The scaled paper size must fit in the range 0<->32767 inclusive, because it must be represented by a signed 16bit number.
    If the size + dpi result in out of range values a failure will result.
    \return pointer to the WMF header record, or NULL on failure
    \param size    Pointer to page size (if NULL, not a placeable header) in inches.  Values must be positive and scaled 
    \param dpi     Logical units/inch.  If 0 defaults to 1440.
*/
char *U_WMRHEADER_set(
      U_PAIRF     *size,
      unsigned int dpi
   ){
   char *record=NULL;
   uint32_t  irecsize,off;
   double xmax,ymax;
   int16_t xm16,ym16;
   irecsize = (size ? U_SIZE_WMRHEADER + U_SIZE_WMRPLACEABLE : U_SIZE_WMRHEADER);
   record = calloc(1,irecsize);  /* most will be zero*/
   off = 0;
   if(record){
      if(size){ /* placeable */
         if(!dpi)dpi=1440;
         xmax = U_ROUND((double) size->x * (double) dpi);
         ymax = U_ROUND((double) size->y * (double) dpi);
         if(xmax < 0 || ymax < 0 || xmax > 32767 || ymax > 32767){
            free(record);
            return(NULL);
         }
         xm16 = xmax;
         ym16 = ymax;
         ((U_WMRPLACEABLE *) record)->Key         = 0x9AC6CDD7;
         ((U_WMRPLACEABLE *) record)->HWmf        = 0;  /* Manual says number of 16 bit words in record, but all WMF examples had it as 0 */
         ((U_WMRPLACEABLE *) record)->Dst.left    = 0;
         ((U_WMRPLACEABLE *) record)->Dst.top     = 0;
         ((U_WMRPLACEABLE *) record)->Dst.right   = xm16;
         ((U_WMRPLACEABLE *) record)->Dst.bottom  = ym16;
         ((U_WMRPLACEABLE *) record)->Inch        = dpi;
         ((U_WMRPLACEABLE *) record)->Reserved    = 0;
         ((U_WMRPLACEABLE *) record)->Checksum    = U_16_checksum((int16_t *)record,10);
         off = U_SIZE_WMRPLACEABLE;
      }
      ((U_WMRHEADER *) (record + off))->iType     = 1;
      ((U_WMRHEADER *) (record + off))->version   = U_METAVERSION300;
      ((U_WMRHEADER *) (record + off))->Size16w   = U_SIZE_WMRHEADER/2;
   }
   return(record);
}


/**
    \brief Allocate and construct a U_WMREOF record
    \return pointer to the U_WMREOF record, or NULL on error.
*/
char *U_WMREOF_set(void){
   return U_WMRCORE_NOARGS_set(U_WMR_EOF);
}                            

/**
    \brief Create and return a U_WMRSETBKCOLOR record
    \return pointer to the U_WMRSETBKCOLOR record, or NULL on error
    \param Color Background Color.
*/
char *U_WMRSETBKCOLOR_set(U_COLORREF Color){
   return U_WMRCORE_1U16_CRF_2U16_set(U_WMR_SETBKCOLOR,NULL,Color,NULL,NULL);
}

/**
    \brief Create and return a U_WMRSETBKMODE record
    \return pointer to the U_WMRSETBKMODE record, or NULL on error
    \param Mode MixMode Enumeration
*/
char *U_WMRSETBKMODE_set(uint16_t Mode){
   return U_WMRCORE_2U16_set(U_WMR_SETBKMODE, Mode, 0);
}

/**
    \brief Create and return a U_WMRSETMAPMODE record
    \return pointer to the U_WMRSETMAPMODE record, or NULL on error
    \param Mode MapMode Enumeration
*/
char *U_WMRSETMAPMODE_set(uint16_t Mode){
   return U_WMRCORE_1U16_set(U_WMR_SETMAPMODE, Mode);
}

/**
    \brief Create and return a U_WMRSETROP2 record
    \return pointer to the U_WMRSETROP2 record, or NULL on error
    \param Mode Binary Raster Operation Enumeration
*/
char *U_WMRSETROP2_set(uint16_t Mode){
   return U_WMRCORE_2U16_set(U_WMR_SETROP2, Mode, 0);
}

/**
    \brief Allocate and construct a U_WMRSETRELABS record
    \return pointer to the U_WMRSETRELABS record, or NULL on error.
*/
char *U_WMRSETRELABS_set(void){
   return U_WMRCORE_NOARGS_set(U_WMR_SETRELABS);
}

/**
    \brief Create and return a U_WMRSETPOLYFILLMODE record
    \return pointer to the U_WMRSETPOLYFILLMODE record, or NULL on error
    \param Mode PolyFillMode Enumeration
*/
char *U_WMRSETPOLYFILLMODE_set(uint16_t Mode){
   return U_WMRCORE_2U16_set(U_WMR_SETPOLYFILLMODE, Mode, 0);
}

/**
    \brief Create and return a U_WMRSETSTRETCHBLTMODE record
    \return pointer to the U_WMRSETSTRETCHBLTMODE record, or NULL on error
    \param Mode StretchMode Enumeration
*/
char *U_WMRSETSTRETCHBLTMODE_set(uint16_t Mode){
   return U_WMRCORE_2U16_set(U_WMR_SETSTRETCHBLTMODE, Mode, 0);
}

/**
    \brief Create and return a U_WMRSETTEXTCHAREXTRA record
    \return pointer to the U_WMRSETTEXTCHAREXTRA record, or NULL on error
    \param Mode Extra space in logical units to add to each character
*/
char *U_WMRSETTEXTCHAREXTRA_set(uint16_t Mode){
   return U_WMRCORE_1U16_set(U_WMR_SETTEXTCHAREXTRA, Mode);
}

/**
    \brief Create and return a U_WMRSETTEXTCOLOR record
    \return pointer to the U_WMRSETTEXTCOLOR record, or NULL on error
    \param Color Text Color.
*/
char *U_WMRSETTEXTCOLOR_set(U_COLORREF Color){
   return U_WMRCORE_1U16_CRF_2U16_set(U_WMR_SETTEXTCOLOR,NULL,Color,NULL,NULL);
}

/**
    \brief Create and return a U_WMRSETTEXTJUSTIFICATION record
    \return pointer to the U_WMRSETTEXTJUSTIFICATION record, or NULL on error
    \param Count Number of space characters in the line.
    \param Extra Number of extra space characters to add to the line.
*/
char *U_WMRSETTEXTJUSTIFICATION_set(uint16_t Count, uint16_t Extra){
   return U_WMRCORE_2U16_set(U_WMR_SETBKMODE, Count, Extra);
}

/**
    \brief Create and return a U_WMRSETWINDOWORG record
    \return pointer to the U_WMRSETWINDOWORG record, or NULL on error
    \param coord Window Origin.
*/
char *U_WMRSETWINDOWORG_set(U_POINT16 coord){
   return U_WMRCORE_2U16_set(U_WMR_SETWINDOWORG, U_U16(coord.y), U_U16(coord.x));
}

/**
    \brief Create and return a U_WMRSETWINDOWEXT record
    \return pointer to the U_WMRSETWINDOWEXT record, or NULL on error
    \param extent Window Extent.
*/
char *U_WMRSETWINDOWEXT_set(U_POINT16 extent){
   return U_WMRCORE_2U16_set(U_WMR_SETWINDOWEXT, U_U16(extent.y), U_U16(extent.x));
}

/**
    \brief Create and return a U_WMRSETVIEWPORTORG record
    \return pointer to the U_WMRSETVIEWPORTORG record, or NULL on error
    \param coord Viewport Origin.
*/
char *U_WMRSETVIEWPORTORG_set(U_POINT16 coord){
   return U_WMRCORE_2U16_set(U_WMR_SETVIEWPORTORG, U_U16(coord.y), U_U16(coord.x));
}

/**
    \brief Create and return a U_WMRSETVIEWPORTEXT record
    \return pointer to the U_WMRSETVIEWPORTEXT record, or NULL on error
    \param extent Viewport Extent.
*/
char *U_WMRSETVIEWPORTEXT_set(U_POINT16 extent){
   return U_WMRCORE_2U16_set(U_WMR_SETWINDOWEXT, U_U16(extent.y), U_U16(extent.x));
}

/**
    \brief Create and return a U_WMROFFSETWINDOWORG record
    \return pointer to the U_WMROFFSETWINDOWORG record, or NULL on error
    \param offset Window offset in device units.
*/
char *U_WMROFFSETWINDOWORG_set(U_POINT16 offset){
   return U_WMRCORE_2U16_set(U_WMR_OFFSETWINDOWORG, U_U16(offset.y), U_U16(offset.x));
}

/**
    \brief Create and return a U_WMRSCALEWINDOWEXT record
    \return pointer to the U_WMRSCALEWINDOWEXT record, or NULL on error
    \param Denom {X,Y} denominators.
    \param Num   {X,Y} numerators.
*/
char *U_WMRSCALEWINDOWEXT_set(U_POINT16 Denom, U_POINT16 Num){
   return U_WMRCORE_4U16_set(U_WMR_SCALEWINDOWEXT, U_U16(Denom.y), U_U16(Num.y), U_U16(Denom.x), U_U16(Num.x));
}

/**
    \brief Create and return a U_WMROFFSETVIEWPORTORG record
    \return pointer to the U_WMROFFSETVIEWPORTORG record, or NULL on error
    \param offset Viewport offset in device units.
*/
char *U_WMROFFSETVIEWPORTORG_set(U_POINT16 offset){
   return U_WMRCORE_2U16_set(U_WMR_OFFSETVIEWPORTORG, U_U16(offset.y), U_U16(offset.x));
}

/**
    \brief Create and return a U_WMRSCALEVIEWPORTEXT record
    \return pointer to the U_WMRSCALEVIEWPORTEXT record, or NULL on error
    \param Denom {X,Y} denominators.
    \param Num   {X,Y} numerators.
*/
char *U_WMRSCALEVIEWPORTEXT_set(U_POINT16 Denom, U_POINT16 Num){
   return U_WMRCORE_4U16_set(U_WMR_SCALEVIEWPORTEXT, U_U16(Denom.y), U_U16(Num.y), U_U16(Denom.x), U_U16(Num.x));
}

/**
    \brief Create and return a U_WMRLINETO record
    \return pointer to the U_WMRLINETO record, or NULL on error
    \param coord   Draw line to {X,Y}.
*/
char *U_WMRLINETO_set(U_POINT16 coord){
   return U_WMRCORE_2U16_set(U_WMR_LINETO, U_U16(coord.y), U_U16(coord.x));
}

/**
    \brief Create and return a U_WMRMOVETO record
    \return pointer to the U_WMRMOVETO record, or NULL on error
    \param coord Move to {X,Y}.
*/
char *U_WMRMOVETO_set(U_POINT16 coord){
   return U_WMRCORE_2U16_set(U_WMR_MOVETO, U_U16(coord.y), U_U16(coord.x));
}

/**
    \brief Create and return a U_WMREXCLUDECLIPRECT record
    \return pointer to the U_WMREXCLUDECLIPRECT record, or NULL on error
    \param rect Exclude rect from clipping region.
*/
char *U_WMREXCLUDECLIPRECT_set(U_RECT16 rect){
   return U_WMRCORE_4U16_set(
      U_WMR_EXCLUDECLIPRECT,
      U_U16(rect.bottom), 
      U_U16(rect.right),
      U_U16(rect.top),
      U_U16(rect.left)
   );
}

/**
    \brief Create and return a U_WMRINTERSECTCLIPRECT record
    \return pointer to the U_WMRINTERSECTCLIPRECT record, or NULL on error
    \param rect Clipping region is intersection of existing clipping region with rect.
*/
char *U_WMRINTERSECTCLIPRECT_set(U_RECT16 rect){
   return U_WMRCORE_4U16_set(
      U_WMR_INTERSECTCLIPRECT,
      U_U16(rect.bottom), 
      U_U16(rect.right),
      U_U16(rect.top),
      U_U16(rect.left)
   );
}

/**
    \brief Create and return a U_WMRARC record
    \return pointer to the U_WMRARC record, or NULL on error
    \param StartArc   Start of Arc
    \param EndArc     End of Arc
    \param rect       Bounding rectangle.
*/
char *U_WMRARC_set(U_POINT16 StartArc, U_POINT16 EndArc, U_RECT16 rect){
   return U_WMRCORE_8U16_set(
      U_WMR_ARC,
      U_U16(EndArc.y), 
      U_U16(EndArc.x),
      U_U16(StartArc.y),
      U_U16(StartArc.x),
      U_U16(rect.bottom), 
      U_U16(rect.right),
      U_U16(rect.top),
      U_U16(rect.left)
    );
}

/**
    \brief Create and return a U_WMRELLIPSE record
    \return pointer to the U_WMRELLIPSE record, or NULL on error
    \param rect Bounding rectangle for Ellipse.
*/
char *U_WMRELLIPSE_set(U_RECT16 rect){
   return U_WMRCORE_4U16_set(
      U_WMR_ELLIPSE,
      U_U16(rect.bottom), 
      U_U16(rect.right),
      U_U16(rect.top),
      U_U16(rect.left)
   );
}

/**
    \brief Create and return a U_WMRFLOODFILL record
    \return pointer to the U_WMRFLOODFILL record, or NULL on error
    \param Mode   FloodFill Enumeration.
    \param Color  Color to Fill with.
    \param coord  Location to start fill.
*/
char *U_WMRFLOODFILL_set(uint16_t Mode, U_COLORREF Color, U_POINT16 coord){
   return  U_WMRCORE_1U16_CRF_2U16_set(
      U_WMR_FLOODFILL,
      &Mode,
      Color,
      U_P16(coord.y),
      U_P16(coord.x)
   );
}

/**
    \brief Create and return a U_WMRPIE record
    \return pointer to the U_WMRPIE record, or NULL on error
    \param Radial1    Start of Pie
    \param Radial2    End of Pie
    \param rect       Bounding rectangle.
*/
char *U_WMRPIE_set(U_POINT16 Radial1, U_POINT16 Radial2, U_RECT16 rect){
   return U_WMRCORE_8U16_set(
      U_WMR_PIE,
      U_U16(Radial2.y), 
      U_U16(Radial2.x),
      U_U16(Radial1.y),
      U_U16(Radial1.x),
      U_U16(rect.bottom), 
      U_U16(rect.right),
      U_U16(rect.top),
      U_U16(rect.left)
    );
}

/**
    \brief Create and return a U_WMRRECTANGLE record
    \return pointer to the U_WMRRECTANGLE record, or NULL on error
    \param rect       Boundaries.
*/
char *U_WMRRECTANGLE_set(U_RECT16 rect){
   return U_WMRCORE_4U16_set(
      U_WMR_RECTANGLE,
      U_U16(rect.bottom), 
      U_U16(rect.right),
      U_U16(rect.top),
      U_U16(rect.left)
   );
}

/**
    \brief Create and return a U_WMRROUNDRECT record
    \return pointer to the U_WMRROUNDRECT record, or NULL on error
    \param Width      Horizontal rounding length.
    \param Height     Vertical rounding length.
    \param rect       Boundaries.
*/
char *U_WMRROUNDRECT_set(int16_t Width, int16_t Height, U_RECT16 rect){
   return U_WMRCORE_6U16_set(
      U_WMR_ROUNDRECT,
      U_U16(Height), 
      U_U16(Width),
      U_U16(rect.bottom), 
      U_U16(rect.right),
      U_U16(rect.top),
      U_U16(rect.left)
   );
}

/**
    \brief Allocate and construct a U_WMRPATBLT record.
    \return pointer to the U_WMRPATBLT record, or NULL on error.
    \param Dst       Destination UL corner in logical units
    \param cwh       W & H for Dst and Src in logical units
    \param dwRop3    RasterOPeration Enumeration
*/
char *U_WMRPATBLT_set(
      U_POINT16      Dst,
      U_POINT16      cwh,
      uint32_t       dwRop3
   ){
   char *record=NULL;
   uint32_t  irecsize;
   U_WMRPATBLT *pmr;

   irecsize = U_SIZE_WMRPATBLT;
   record   = malloc(irecsize);
   if(record){
      U_WMRCORE_SETRECHEAD(record,irecsize,U_WMR_PATBLT);
      pmr = (U_WMRPATBLT *) record;
      memcpy(pmr->rop3w, &dwRop3, 4);
      pmr->Height   = cwh.y;
      pmr->Width    = cwh.x;
      pmr->yDst     = Dst.y;
      pmr->xDst     = Dst.x;
   }
   return(record);
}

/**
    \brief Allocate and construct a U_WMRSAVEDC record
    \return pointer to the U_WMRSAVEDC record, or NULL on error.
*/
char *U_WMRSAVEDC_set(void){
   return U_WMRCORE_NOARGS_set(U_WMR_SAVEDC);
}

/**
    \brief Allocate and construct a U_WMRSETPIXEL record
    \return pointer to the U_WMRSETPIXEL record, or NULL on error.
    \param  Color   U_COLORREF color of the pixel
    \param  Coord   U_POINT16 coordinates of the pixel
*/
char *U_WMRSETPIXEL_set(U_COLORREF Color, U_POINT16 Coord){
   return  U_WMRCORE_1U16_CRF_2U16_set(
      U_WMR_SETPIXEL,
      NULL,
      Color,
      U_P16(Coord.y),
      U_P16(Coord.x)
   );
}

/**
    \brief Allocate and construct a U_WMROFFSETCLIPRGN record
    \return pointer to the U_WMROFFSETCLIPRGN record, or NULL on error.
    \param  offset  U_POINT16 x,y offset to apply to the clipping region.
*/
char *U_WMROFFSETCLIPRGN_set(U_POINT16 offset){
   return U_WMRCORE_2U16_set(U_WMR_OFFSETCLIPRGN, U_U16(offset.y), U_U16(offset.x));
}

/**
    \brief Allocate and construct a U_WMRTEXTOUT record.
    \return pointer to the U_WMRTEXTOUT record, or NULL on error.
    \param Dst       Destinationin logical units
    \param string    Null terminated string to write.  The terminator is NOT placed in the record!
*/
char *U_WMRTEXTOUT_set(U_POINT16 Dst, char *string){
   char *record=NULL;
   uint32_t  irecsize,off;
   int  L2;
   int16_t Length;
   irecsize  = 2 + U_SIZE_METARECORD + 4;  /* core + length + Dst */
   Length = strlen(string);
   L2 = ( Length & 1 ? Length + 1 : Length);
   irecsize += L2;
   record = malloc(irecsize);
   if(record){
      U_WMRCORE_SETRECHEAD(record,irecsize,U_WMR_TEXTOUT);
      off = U_SIZE_METARECORD;
         memcpy(record+off,&Length,2);          off+=2;
         memcpy(record+off,string,Length);      off+=Length;
      if(Length!=L2){ 
         memset(record+off,0,1);                off+=1;
      }
      memcpy(record+off,&Dst.y,2);              off+=2;
      memcpy(record+off,&Dst.x,2);
   }
   return(record);
}

/**
    \brief Allocate and construct a U_WMRBITBLT record.
     Note that unlike U_EMRBITBLT there is no scaling available - the Src and Dst
      rectangles must be the same size.
    \return pointer to the U_WMRBITBLT record, or NULL on error.
    \param Dst       Destination UL corner in logical units
    \param cwh       W & H for Dst and Src in logical units
    \param Src       Source UL corner in logical units
    \param dwRop3    RasterOPeration Enumeration
    \param Bm16      (Optional) bitmap16 object
*/
char *U_WMRBITBLT_set(
      U_POINT16            Dst,
      U_POINT16            cwh,
      U_POINT16            Src,
      uint32_t             dwRop3,
      const U_BITMAP16    *Bm16
   ){
   char *record=NULL;
   uint32_t  irecsize;
   int   cbBm16,cbBm164,off;
   U_WMRBITBLT_PX   *pmr_px;
   U_WMRBITBLT_NOPX *pmr_nopx;

   if(Bm16){
      cbBm16  = U_SIZE_BITMAP16 + (((Bm16->Width * Bm16->BitsPixel + 15) >> 4) << 1) * Bm16->Height;
      cbBm164 = UP4(cbBm16);
      irecsize = U_SIZE_WMRBITBLT_PX + cbBm164;
      record   = malloc(irecsize);
      if(record){
         U_WMRCORE_SETRECHEAD(record,irecsize,U_WMR_BITBLT);
         pmr_px = (U_WMRBITBLT_PX *) record;
         memcpy(pmr_px->rop3w, &dwRop3, 4);
         pmr_px->ySrc     = Src.y;
         pmr_px->xSrc     = Src.x;
         pmr_px->Height   = cwh.y;
         pmr_px->Width    = cwh.x;
         pmr_px->yDst     = Dst.y;
         pmr_px->xDst     = Dst.x;
         off = U_SIZE_WMRBITBLT_PX;
         memcpy(record + off, Bm16, cbBm16);        off += cbBm16;   
         if(cbBm164 - cbBm16)memset(record+off,0,cbBm164 - cbBm16);
      }
   }
   else {   
      irecsize = U_SIZE_WMRBITBLT_NOPX;
      record   = malloc(irecsize);
      if(record){
         U_WMRCORE_SETRECHEAD(record,irecsize,U_WMR_BITBLT);
         pmr_nopx = (U_WMRBITBLT_NOPX *) record;
         memcpy(pmr_nopx->rop3w, &dwRop3, 4);
         pmr_nopx->ySrc     = Src.y;
         pmr_nopx->xSrc     = Src.x;
         pmr_nopx->Height   = cwh.y;
         pmr_nopx->Width    = cwh.x;
         pmr_nopx->ignore   = 0;
         pmr_nopx->yDst     = Dst.y;
         pmr_nopx->xDst     = Dst.x;
      }
   }
   return(record);
}

/**
    \brief Allocate and construct a U_WMRSTRETCHBLT record.
    \return pointer to the U_WMRSTRETCHBLT record, or NULL on error.
    \param Dst       Destination UL corner in logical units
    \param cDst      Destination W & H in logical units
    \param Src       Source UL corner in logical units
    \param cSrc      Source W & H in logical units
    \param dwRop3    RasterOPeration Enumeration
    \param Bm16      (Optional) bitmap16 object
*/
char *U_WMRSTRETCHBLT_set(
      U_POINT16            Dst,
      U_POINT16            cDst,
      U_POINT16            Src,
      U_POINT16            cSrc,
      uint32_t             dwRop3,
      const U_BITMAP16    *Bm16
   ){
   char *record=NULL;
   uint32_t  irecsize;
   int   cbBm16,cbBm164,off;
   U_WMRSTRETCHBLT_PX   *pmr_px;
   U_WMRSTRETCHBLT_NOPX *pmr_nopx;

   if(Bm16){
      cbBm16  = U_SIZE_BITMAP16 + (((Bm16->Width * Bm16->BitsPixel + 15) >> 4) << 1) * Bm16->Height;
      cbBm164 = UP4(cbBm16);
      irecsize = U_SIZE_WMRSTRETCHBLT_PX + cbBm164;
      record   = malloc(irecsize);
      if(record){
         U_WMRCORE_SETRECHEAD(record,irecsize,U_WMR_STRETCHBLT);
         pmr_px = (U_WMRSTRETCHBLT_PX *) record;
         memcpy(pmr_px->rop3w, &dwRop3, 4);
         pmr_px->hSrc     = cSrc.y;
         pmr_px->wSrc     = cSrc.x;
         pmr_px->ySrc     = Src.y;
         pmr_px->xSrc     = Src.x;
         pmr_px->hDst     = cDst.y;
         pmr_px->wDst     = cDst.x;
         pmr_px->yDst     = Dst.y;
         pmr_px->xDst     = Dst.x;
         off = U_SIZE_WMRSTRETCHBLT_PX;
         memcpy(record + off, Bm16, cbBm16);       off += cbBm16;   
         if(cbBm164 - cbBm16)memset(record+off,0,cbBm164 - cbBm16);
      }
   }
   else {   
      irecsize = U_SIZE_WMRSTRETCHBLT_NOPX;
      record   = malloc(irecsize);
      if(record){
         U_WMRCORE_SETRECHEAD(record,irecsize,U_WMR_STRETCHBLT);
         pmr_nopx = (U_WMRSTRETCHBLT_NOPX *) record;
         memcpy(pmr_nopx->rop3w, &dwRop3, 4);
         pmr_nopx->hSrc     = cSrc.y;
         pmr_nopx->wSrc     = cSrc.x;
         pmr_nopx->ySrc     = Src.y;
         pmr_nopx->xSrc     = Src.x;
         pmr_nopx->ignore   = 0;
         pmr_nopx->hDst     = cDst.y;
         pmr_nopx->wDst     = cDst.x;
         pmr_nopx->yDst     = Dst.y;
         pmr_nopx->xDst     = Dst.x;
      }
   }
   return(record);
}

/**
    \brief Allocate and construct a U_WMRPOLYGON record.
    \return pointer to the U_WMRPOLYGON record, or NULL on error.
    \param Length    Number of points in the Polygon
    \param Data      Array of Length points
*/
char *U_WMRPOLYGON_set(uint16_t Length, const U_POINT16 *Data){
   return U_WMRCORE_2U16_N16_set(U_WMR_POLYGON, NULL, &Length, 2*Length, Data);
}

/**
    \brief Allocate and construct a U_WMRPOLYLINE record.
    \return pointer to the U_WMRPOLYLINE record, or NULL on error.
    \param Length    Number of points in the Polyline
    \param Data      Array of Length points
*/
char *U_WMRPOLYLINE_set(uint16_t Length, const U_POINT16 *Data){
   return U_WMRCORE_2U16_N16_set(U_WMR_POLYLINE, NULL, &Length, 2*Length, Data);
}

/**
    \brief Allocate and construct a U_WMRESCAPE record.
    WARNING! Only three Escape record types are fully supported: SETLINECAP, SETLINEJOIN, SETMITERLIMIT.
    Even these should not be set here directly, instead use the wsetlinecap_set(), wsetlinejoin_set(), 
    or wsetmiterlimit_set() functions.
    Escape records created with this function, with the exception of the three named above, will not have
    the byte orders in Data adjusted automatically.  The user code must set Data to be little endian no
    matter what the endianness of the current platform where the user code is running.
    \return pointer to the U_WMRESCAPE record, or NULL on error.
    \param Escape    Escape function
    \param Length    Bytes in the Data
    \param Data      Array of Length bytes
*/
char *U_WMRESCAPE_set(uint16_t Escape, uint16_t Length, const void *Data){
   return U_WMRCORE_2U16_N16_set(U_WMR_ESCAPE, &Escape, &Length, Length/2, Data);
}

/**
    \brief Allocate and construct a U_WMRRESTOREDC record
    \return pointer to the U_WMRRESTOREDC record, or NULL on error.
    \param  DC  Drawing Context to restore.  (negative is relative to current, positive is absolute)
*/
char *U_WMRRESTOREDC_set(int16_t DC){
   return U_WMRCORE_1U16_set(U_WMR_SETMAPMODE, DC);
}

/**
    \brief Allocate and construct a U_WMRFILLREGION record.
    \return pointer to the U_WMRFILLREGION record, or NULL on error.
    \param Region    Region to fill
    \param Brush     Brush to fill with
*/
char *U_WMRFILLREGION_set(uint16_t Region, uint16_t Brush){
   return U_WMRCORE_2U16_set(U_WMR_FILLREGION, Region, Brush);
}

/**
    \brief Allocate and construct a U_WMRFRAMEREGION record.
    \return pointer to the U_WMRFRAMEREGION record, or NULL on error.
    \param  Region  Index of region to frame in object table
    \param  Brush   Index of brush to use in frame in object table
    \param  Height  in logical units (of frame)
    \param  Width   in logical units (of frame)
*/
char *U_WMRFRAMEREGION_set(uint16_t Region, uint16_t Brush, int16_t Height, int16_t Width){
   return U_WMRCORE_4U16_set(U_WMR_FRAMEREGION, Region, Brush, U_U16(Height),  U_U16(Width));
}

/**
    \brief Allocate and construct a U_WMRINVERTREGION record.
    \return pointer to the U_WMRINVERTREGION record, or NULL on error.
    \param  Region  Index of region to invert.
*/
char *U_WMRINVERTREGION_set(uint16_t Region){
   return U_WMRCORE_1U16_set(U_WMR_INVERTREGION, Region);
}

/**
    \brief Allocate and construct a U_WMRPAINTREGION record.
    \return pointer to the U_WMRPAINTREGION record, or NULL on error.
    \param  Region  Index of region to paint with the current Brush.
*/
char *U_WMRPAINTREGION_set(uint16_t Region){
   return U_WMRCORE_1U16_set(U_WMR_PAINTREGION, Region);
}

/**
    \brief Allocate and construct a U_WMRSELECTCLIPREGION record.
    \return pointer to the U_WMRSELECTCLIPREGION record, or NULL on error.
    \param  Region  Index of region to become clipping region..
*/
char *U_WMRSELECTCLIPREGION_set(uint16_t Region){
   return U_WMRCORE_1U16_set(U_WMR_SELECTCLIPREGION, Region);
}

/**
    \brief Allocate and construct a U_WMRSELECTOBJECT record.
    \return pointer to the U_WMRSELECTOBJECT record, or NULL on error.
    \param  object  Index of object which is made active.
*/
char *U_WMRSELECTOBJECT_set(uint16_t object){
   return U_WMRCORE_1U16_set(U_WMR_SELECTOBJECT, object);
}

/**
    \brief Allocate and construct a U_WMRSETTEXTALIGN record.
    \return pointer to the U_WMRSETTEXTALIGN record, or NULL on error.
    \param  Mode  TextAlignment Enumeration.
*/
char *U_WMRSETTEXTALIGN_set(uint16_t Mode){
   return U_WMRCORE_2U16_set(U_WMR_SETTEXTALIGN, Mode, 0);
}

/** in GDI and Wine, not in WMF manual..
*/
 char *U_WMRDRAWTEXT_set(void){
   return U_WMRCORENONE_set("U_WMRDRAWTEXT");
}

/**
    \brief Create and return a U_WMRCHORD record
    \return pointer to the U_WMRCHORD record, or NULL on error
    \param Radial1    Start of Chord
    \param Radial2    End of Chord
    \param rect       Bounding rectangle.
*/
char *U_WMRCHORD_set(U_POINT16 Radial1, U_POINT16 Radial2, U_RECT16 rect){
   return U_WMRCORE_8U16_set(
      U_WMR_CHORD,
      U_U16(Radial2.y), 
      U_U16(Radial2.x),
      U_U16(Radial1.y),
      U_U16(Radial1.x),
      U_U16(rect.bottom), 
      U_U16(rect.right),
      U_U16(rect.top),
      U_U16(rect.left)
    );
}

/**
    \brief Allocate and construct a U_WMRSETMAPPERFLAGS record.
    \return pointer to the U_WMRSETMAPPERFLAGS record, or NULL on error.
    \param  Mode  If 1 bit set font mapper selects only matching aspect fonts.
*/
char *U_WMRSETMAPPERFLAGS_set(uint32_t Mode){
   return U_WMRCORE_2U16_set(U_WMR_SETMAPPERFLAGS, 0xFFFF & Mode, Mode>>16);
}

/**
    \brief Allocate and construct a U_WMREXTTEXTOUT record.
    \return pointer to the U_WMREXTTEXTOUT record, or NULL on error.
    \param Dst     {X,Y} coordinates where the string is to be written.
    \param Length  Stringlength in bytes
    \param Opts    ExtTextOutOptions Flags
    \param string  String to write (Latin1 encoding)
    \param dx      Kerning information.  Must have same number of entries as Length.
    \param rect    Used when when U_ETO_OPAQUE or U_ETO_CLIPPED bits are set in Opts
*/
char *U_WMREXTTEXTOUT_set(U_POINT16 Dst, int16_t Length, uint16_t Opts, 
   const char *string, int16_t *dx, U_RECT16 rect){

   char *record=NULL;
   uint32_t  irecsize,off;
   int  slen;
   irecsize  = U_SIZE_METARECORD + 8; /* 8 = y,x,Length,Opts*/
   slen = ( Length & 1 ? Length + 1 : Length);
   irecsize += slen;
   if(dx)irecsize += 2*Length;
   if(Opts & (U_ETO_OPAQUE | U_ETO_CLIPPED)){
      irecsize += U_SIZE_RECT16;
   }
   record = malloc(irecsize);
   if(record){
      U_WMRCORE_SETRECHEAD(record,irecsize,U_WMR_EXTTEXTOUT);
      off = U_SIZE_METARECORD;
         memcpy(record+off,&Dst.y,2);              off+=2;
         memcpy(record+off,&Dst.x,2);              off+=2;
         memcpy(record+off,&Length,2);             off+=2;
         memcpy(record+off,&Opts,2);               off+=2;
      if(Opts & (U_ETO_OPAQUE | U_ETO_CLIPPED)){
         memcpy(record+off,&rect.bottom,2);        off+=2;
         memcpy(record+off,&rect.right, 2);        off+=2;
         memcpy(record+off,&rect.top,   2);        off+=2;
         memcpy(record+off,&rect.left,  2);        off+=2;
      } 
         memcpy(record+off,string,strlen(string)); off+=Length;
      if(Length!=slen){ 
         memset(record+off,0,1);                   off+=1;
      }
      if(dx){
         memcpy(record+off,dx,2*Length);
      }
   }
   return(record);
}

/**
    \brief Allocate and construct a U_WMRSETDIBTODEV record
    \return pointer to the U_WMRSETDIBTODEV record, or NULL on error.
*/
char *U_WMRSETDIBTODEV_set(void){
   return U_WMRCORENONE_set("U_WMRSETDIBTODEV");
}

/**
    \brief Allocate and construct a U_WMRSELECTPALETTE record
    \return pointer to the U_WMRSELECTPALETTE record, or NULL on error.
    \param Palette  Index of Palette to make active.
*/
char *U_WMRSELECTPALETTE_set(uint16_t Palette){
   return U_WMRCORE_1U16_set(U_WMR_SELECTPALETTE, Palette);
}

/**
    \brief Allocate and construct a U_WMRREALIZEPALETTE record
    \return pointer to the U_WMRREALIZEPALETTE record, or NULL on error.
*/
char *U_WMRREALIZEPALETTE_set(void){
   return U_WMRCORE_NOARGS_set(U_WMR_REALIZEPALETTE);
}

/**
    \brief Allocate and construct a U_WMRSETPALENTRIES record
    \return pointer to the U_WMRSETPALENTRIES record, or NULL on error.
    \param Palette  Redefines a set of RGB values for the current active Palette.
*/
char *U_WMRANIMATEPALETTE_set(U_PALETTE *Palette){
   return U_WMRCORE_PALETTE_set(U_WMR_ANIMATEPALETTE, Palette);
}

/**
    \brief Allocate and construct a U_WMRSETPALENTRIES record
    \return pointer to the U_WMRSETPALENTRIES record, or NULL on error.
    \param Palette  Defines a set of RGB values for the current active Palette.
*/
char *U_WMRSETPALENTRIES_set(const U_PALETTE *Palette){
   return U_WMRCORE_PALETTE_set(U_WMR_SETPALENTRIES, Palette);
}

/**
    \brief Allocate and construct a U_WMR_POLYPOLYGON record.
    \return pointer to the U_WMR_POLYPOLYGON record, or NULL on error.
    \param nPolys       Number of elements in aPolyCounts
    \param aPolyCounts  Number of points in each poly (sequential)
    \param Points       array of points
*/
char *U_WMRPOLYPOLYGON_set(
      const uint16_t   nPolys,
      const uint16_t  *aPolyCounts,
      const U_POINT16 *Points
   ){
   char      *record;
   uint32_t   irecsize;
   int        i,cbPolys,cbPoints,off;
   
   cbPolys = sizeof(uint16_t)*nPolys;
   for(i=cbPoints=0; i<nPolys; i++){ cbPoints += U_SIZE_POINT16*aPolyCounts[i]; }
   
   if(nPolys==0 || cbPoints==0)return(NULL);
   
   irecsize  = U_SIZE_METARECORD + 2 + cbPolys + cbPoints; /* core WMR + nPolys + two array sizes in bytes */
   record = malloc(irecsize);
   if(record){
      U_WMRCORE_SETRECHEAD(record,irecsize,U_WMR_POLYPOLYGON);
      off = U_SIZE_METARECORD;
      memcpy(record + off, &nPolys,      2);       off+=2;
      memcpy(record + off,  aPolyCounts, cbPolys); off+=cbPolys;
      memcpy(record + off,  Points,      cbPoints);
   }
   return(record);
}

/**
    \brief Allocate and construct a U_WMRRESIZEPALETTE record
    \return pointer to the U_WMRRESIZEPALETTE record, or NULL on error.
    \param Palette  Changes the size of the currently active Palette.
*/
char *U_WMRRESIZEPALETTE_set(uint16_t Palette){
   return U_WMRCORE_1U16_set(U_WMR_RESIZEPALETTE, Palette);
}

//! \cond
char *U_WMR3A_set(void){
   return U_WMRCORENONE_set("U_WMR3A");
}

char *U_WMR3B_set(void){
   return U_WMRCORENONE_set("U_WMR3B");
}

char *U_WMR3C_set(void){
   return U_WMRCORENONE_set("U_WMR3C");
}

char *U_WMR3D_set(void){
   return U_WMRCORENONE_set("U_WMR3D");
}

char *U_WMR3E_set(void){
   return U_WMRCORENONE_set("U_WMR3E");
}

char *U_WMR3F_set(void){
   return U_WMRCORENONE_set("U_WMR3F");
}
//! \endcond

// U_WMRDIBBITBLT_set
/**
    \brief Allocate and construct a U_WMRDIBITBLT record.
    \return pointer to the U_WMRDIBITBLT record, or NULL on error.
    \param Dst       Destination UL corner in logical units
    \param Src       Source UL corner in logical units
    \param cwh       W & H in logical units of Src and Dst
    \param dwRop3    RasterOPeration Enumeration
    \param Bmi       (Optional) bitmapbuffer (U_BITMAPINFO section)
    \param cbPx      Size in bytes of pixel array (row STRIDE * height, there may be some padding at the end of each row)
    \param Px        (Optional) bitmapbuffer (pixel array section )
*/
char *U_WMRDIBBITBLT_set(
      U_POINT16            Dst,
      U_POINT16            cwh,
      U_POINT16            Src,
      uint32_t             dwRop3,
      const U_BITMAPINFO  *Bmi,
      uint32_t             cbPx,
      const char          *Px
   ){
   char *record=NULL;
   uint32_t  irecsize;
   int   cbImage,cbImage4,cbBmi,off;
   U_WMRDIBBITBLT_PX   *pmr_px;
   U_WMRDIBBITBLT_NOPX *pmr_nopx;


   if(Px && Bmi){
      SET_CB_FROM_PXBMI(Px,Bmi,cbImage,cbImage4,cbBmi,cbPx);   
      irecsize = U_SIZE_WMRDIBBITBLT_PX + cbBmi + cbImage4;
      record   = malloc(irecsize);
      if(record){
         U_WMRCORE_SETRECHEAD(record,irecsize,U_WMR_DIBBITBLT);
         pmr_px = (U_WMRDIBBITBLT_PX *) record;
         memcpy(pmr_px->rop3w, &dwRop3, 4);
         pmr_px->ySrc     = Src.y;
         pmr_px->xSrc     = Src.x;
         pmr_px->Height   = cwh.y;
         pmr_px->Width    = cwh.x;
         pmr_px->yDst     = Dst.y;
         pmr_px->xDst     = Dst.x;
         off = U_SIZE_WMRDIBBITBLT_PX;
         memcpy(record + off, Bmi, cbBmi);        off += cbBmi;   
         memcpy(record + off, Px,  cbPx);         off += cbPx;
         if(cbImage4 - cbImage)memset(record+off,0,cbImage4 - cbImage);
      }
   }
   else if(!Px && !Bmi){   
      irecsize = U_SIZE_WMRDIBBITBLT_NOPX;
      record   = malloc(irecsize);
      if(record){
         U_WMRCORE_SETRECHEAD(record,irecsize,U_WMR_DIBBITBLT);
         pmr_nopx = (U_WMRDIBBITBLT_NOPX *) record;
         memcpy(pmr_nopx->rop3w, &dwRop3, 4);
         pmr_nopx->ySrc     = Src.y;
         pmr_nopx->xSrc     = Src.x;
         pmr_nopx->ignore   = 0;
         pmr_nopx->Height   = cwh.y;
         pmr_nopx->Width    = cwh.x;
         pmr_nopx->yDst     = Dst.y;
         pmr_nopx->xDst     = Dst.x;
      }
   }
   return(record);
}

/**
    \brief Allocate and construct a U_WMRSTRETCHDIB record.
    \return pointer to the U_WMRSTRETCHDIB record, or NULL on error.
    \param Dst       Destination UL corner in logical units
    \param cDst      Destination W & H in logical units
    \param Src       Source UL corner in logical units
    \param cSrc      Source W & H in logical units
    \param dwRop3    RasterOPeration Enumeration
    \param Bmi       (Optional) bitmapbuffer (U_BITMAPINFO section)
    \param cbPx      Size in bytes of pixel array (row STRIDE * height, there may be some padding at the end of each row)
    \param Px        (Optional) bitmapbuffer (pixel array section )
*/
char *U_WMRDIBSTRETCHBLT_set(
      U_POINT16            Dst,
      U_POINT16            cDst,
      U_POINT16            Src,
      U_POINT16            cSrc,
      uint32_t             dwRop3,
      const U_BITMAPINFO  *Bmi,
      uint32_t             cbPx,
      const char          *Px
   ){
   char *record=NULL;
   uint32_t  irecsize;
   int   cbImage,cbImage4,cbBmi,off;
   U_WMRDIBSTRETCHBLT_PX   *pmr_px;
   U_WMRDIBSTRETCHBLT_NOPX *pmr_nopx;
   if(Px && Bmi){
      SET_CB_FROM_PXBMI(Px,Bmi,cbImage,cbImage4,cbBmi,cbPx);
      irecsize = U_SIZE_WMRDIBSTRETCHBLT_PX + cbBmi + cbImage4;
      record   = malloc(irecsize);
      if(record){
         U_WMRCORE_SETRECHEAD(record,irecsize,U_WMR_DIBSTRETCHBLT);
         pmr_px = (U_WMRDIBSTRETCHBLT_PX *) record;
         memcpy(pmr_px->rop3w, &dwRop3, 4);
         pmr_px->hSrc     = cSrc.y;
         pmr_px->wSrc     = cSrc.x;
         pmr_px->ySrc     = Src.y;
         pmr_px->xSrc     = Src.x;
         pmr_px->hDst     = cDst.y;
         pmr_px->wDst     = cDst.x;
         pmr_px->yDst     = Dst.y;
         pmr_px->xDst     = Dst.x;
         off = U_SIZE_WMRDIBSTRETCHBLT_PX;
         memcpy(record + off, Bmi, cbBmi);        off += cbBmi;   
         memcpy(record + off, Px,  cbPx);         off += cbPx;
         if(cbImage4 - cbImage)memset(record+off,0,cbImage4 - cbImage);
      }
   }
   else if(!Px && !Bmi){   
      irecsize = U_SIZE_WMRDIBSTRETCHBLT_NOPX;
      record   = malloc(irecsize);
      if(record){
         U_WMRCORE_SETRECHEAD(record,irecsize,U_WMR_DIBSTRETCHBLT);
         pmr_nopx = (U_WMRDIBSTRETCHBLT_NOPX *) record;
         memcpy(pmr_nopx->rop3w, &dwRop3, 4);
         pmr_nopx->hSrc     = cSrc.y;
         pmr_nopx->wSrc     = cSrc.x;
         pmr_nopx->ySrc     = Src.y;
         pmr_nopx->xSrc     = Src.x;
         pmr_nopx->ignore   = 0;
         pmr_nopx->hDst     = cDst.y;
         pmr_nopx->wDst     = cDst.x;
         pmr_nopx->yDst     = Dst.y;
         pmr_nopx->xDst     = Dst.x;
      }
   }
   return(record);
}

/**
    \brief Allocate and construct a U_WMRDIBCREATEPATTERNBRUSH record.
      Accepts an image as either a DIB (Bmi/CbPx/Px defined) or a Bitmap16 (Bm16 defined).
    \return pointer to the U_WMRDIBCREATEPATTERNBRUSH record, or NULL on error.
    \param Style   BrushStyle Enumeration
    \param iUsage  DIBcolors Enumeration
    \param Bm16    pointer to U_BITMAP16 object for Style U_BS_PATTERN only
    \param cbPx    Size in bytes of pixel array (row stride * height, there may be some padding at the end of each row), for use with Bmi
    \param Px      bitmap buffer, for use with Bmi
    \param Bmi     pointer to U_BITMAPINFO  for all Style OTHER than U_BS_PATTERN
*/
char *U_WMRDIBCREATEPATTERNBRUSH_set(
       const uint16_t      Style, 
       const uint16_t      iUsage,
       const U_BITMAPINFO *Bmi,   
       const uint32_t      cbPx,  
       const char         *Px,     
       const U_BITMAP16   *Bm16  
   ){
   char *record=NULL;
   uint32_t  irecsize;
   int   cbImage,cbImage4,cbBmi,cbBm16,cbBm164,off;
   
   if(Style==U_BS_PATTERN && Bm16){
      cbBm16  = U_SIZE_BITMAP16 + (((Bm16->Width * Bm16->BitsPixel + 15) >> 4) << 1) * Bm16->Height;
      cbBm164 = UP4(cbBm16);
      irecsize = U_SIZE_WMRDIBCREATEPATTERNBRUSH + cbBm164;
      record   = malloc(irecsize);
      if(record){
         U_WMRCORE_SETRECHEAD(record,irecsize,U_WMR_DIBCREATEPATTERNBRUSH);
         off = U_SIZE_METARECORD;
         memcpy(record + off, &Style,  2);       off+=2;
         memcpy(record + off, &iUsage, 2);       off+=2;
         memcpy(record + off, Bm16,    cbBm16);  off += cbBm16;   
         if(cbBm164 - cbBm16)memset(record+off,0,cbBm164 - cbBm16);
      }
   }
   else if(Bmi && Px){
      SET_CB_FROM_PXBMI(Px,Bmi,cbImage,cbImage4,cbBmi,cbPx);
      irecsize = U_SIZE_WMRDIBCREATEPATTERNBRUSH + cbBmi + cbImage4;
      record   = malloc(irecsize);
      if(record){
         U_WMRCORE_SETRECHEAD(record,irecsize,U_WMR_DIBCREATEPATTERNBRUSH);
         off = U_SIZE_METARECORD;
         memcpy(record + off, &Style,  2);       off+=2;
         memcpy(record + off, &iUsage, 2);       off+=2;
         memcpy(record + off, Bmi,     cbBmi);   off += cbBmi;   
         memcpy(record + off, Px,      cbImage); off += cbImage;
         if(cbImage4 - cbImage)memset(record + off, 0, cbImage4 - cbImage);
      }
   }
   return(record);
}

/**
    \brief Allocate and construct a U_WMRSTRETCHDIB record.
    \return pointer to the U_WMRSTRETCHDIB record, or NULL on error.
    \param Dst       Destination UL corner in logical units
    \param cDst      Destination W & H in logical units
    \param Src       Source UL corner in logical units
    \param cSrc      Source W & H in logical units
    \param cUsage    DIBColors Enumeration
    \param dwRop3    RasterOPeration Enumeration
    \param Bmi       (Optional) bitmapbuffer (U_BITMAPINFO section)
    \param cbPx      Size in bytes of pixel array (row STRIDE * height, there may be some padding at the end of each row)
    \param Px        (Optional) bitmapbuffer (pixel array section )
*/
char *U_WMRSTRETCHDIB_set(
      U_POINT16            Dst,
      U_POINT16            cDst,
      U_POINT16            Src,
      U_POINT16            cSrc,
      uint16_t             cUsage,
      uint32_t             dwRop3,
      const U_BITMAPINFO  *Bmi,
      uint32_t             cbPx,
      const char          *Px
   ){
   char *record;
   uint32_t  irecsize;
   int   cbImage,cbImage4,cbBmi,off;
   U_WMRSTRETCHDIB *pmr;

   SET_CB_FROM_PXBMI(Px,Bmi,cbImage,cbImage4,cbBmi,cbPx);
   
   irecsize = U_SIZE_WMRSTRETCHDIB + cbBmi + cbImage4;
   record   = malloc(irecsize);
   if(record){
      U_WMRCORE_SETRECHEAD(record,irecsize,U_WMR_STRETCHDIB);
      pmr = (U_WMRSTRETCHDIB *) record;
      memcpy(pmr->rop3w, &dwRop3, 4);
      pmr->cUsage   = cUsage;
      pmr->hSrc     = cSrc.y;
      pmr->wSrc     = cSrc.x;
      pmr->ySrc     = Src.y;
      pmr->xSrc     = Src.x;
      pmr->hDst     = cDst.y;
      pmr->wDst     = cDst.x;
      pmr->yDst     = Dst.y;
      pmr->xDst     = Dst.x;
      off = U_SIZE_WMRSTRETCHDIB;
      if(cbBmi){
         memcpy(record + off, Bmi, cbBmi);        off += cbBmi;   
         memcpy(record + off, Px,  cbPx);         off += cbPx;
         if(cbImage4 - cbImage)memset(record+off,0,cbImage4 - cbImage);
      }
   }
   return(record);
}

//! \cond
char *U_WMR44_set(void){
   return U_WMRCORENONE_set("U_WMR44");
}

char *U_WMR45_set(void){
   return U_WMRCORENONE_set("U_WMR45");
}

char *U_WMR46_set(void){
   return U_WMRCORENONE_set("U_WMR46");
}

char *U_WMR47_set(void){
   return U_WMRCORENONE_set("U_WMR47");
}
//! \endcond

/**
    \brief Create and return a U_WMREXTFLOODFILL record
    \return pointer to the U_WMREXTFLOODFILL record, or NULL on error
    \param Mode   FloodFill Enumeration.
    \param Color  Color to Fill with.
    \param coord  Location to start fill.
*/
char *U_WMREXTFLOODFILL_set(uint16_t Mode, U_COLORREF Color, U_POINT16 coord){
   return  U_WMRCORE_1U16_CRF_2U16_set(
      U_WMR_EXTFLOODFILL,
      &Mode,
      Color,
      U_P16(coord.y),
      U_P16(coord.x)
   );
}

//! \cond
char *U_WMR49_set(void){
   return U_WMRCORENONE_set("U_WMR49");
}

char *U_WMR4A_set(void){
   return U_WMRCORENONE_set("U_WMR4A");
}

char *U_WMR4B_set(void){
   return U_WMRCORENONE_set("U_WMR4B");
}

char *U_WMR4C_set(void){
   return U_WMRCORENONE_set("U_WMRRESETDOC");
}

char *U_WMR4D_set(void){
   return U_WMRCORENONE_set("U_WMRSTARTDOC");
}

char *U_WMR4E_set(void){
   return U_WMRCORENONE_set("U_WMR4E");
}

char *U_WMR4F_set(void){
   return U_WMRCORENONE_set("U_WMRSTARTPAGE");
}

char *U_WMR50_set(void){
   return U_WMRCORENONE_set("U_WMRENDPAGE");
}

char *U_WMR51_set(void){
   return U_WMRCORENONE_set("U_WMR51");
}

char *U_WMRABORTDOC_set(void){
   return U_WMRCORENONE_set("U_WMRABORTDOC");
}

char *U_WMR53_set(void){
   return U_WMRCORENONE_set("U_WMR53");
}

char *U_WMR54_set(void){
   return U_WMRCORENONE_set("U_WMR54");
}

char *U_WMR55_set(void){
   return U_WMRCORENONE_set("U_WMR55");
}

char *U_WMR56_set(void){
   return U_WMRCORENONE_set("U_WMR56");
}

char *U_WMR57_set(void){
   return U_WMRCORENONE_set("U_WMR57");
}

char *U_WMR58_set(void){
   return U_WMRCORENONE_set("U_WMR58");
}

char *U_WMR59_set(void){
   return U_WMRCORENONE_set("U_WMR59");
}

char *U_WMR5A_set(void){
   return U_WMRCORENONE_set("U_WMR5A");
}

char *U_WMR5B_set(void){
   return U_WMRCORENONE_set("U_WMR5B");
}

char *U_WMR5C_set(void){
   return U_WMRCORENONE_set("U_WMR5C");
}

char *U_WMR5D_set(void){
   return U_WMRCORENONE_set("U_WMR5D");
}

char *U_WMR5E_set(void){
   return U_WMRCORENONE_set("U_WMRENDDOC");
}

char *U_WMR5F_set(void){
   return U_WMRCORENONE_set("U_WMR5F");
}

char *U_WMR60_set(void){
   return U_WMRCORENONE_set("U_WMR60");
}

char *U_WMR61_set(void){
   return U_WMRCORENONE_set("U_WMR61");
}

char *U_WMR62_set(void){
   return U_WMRCORENONE_set("U_WMR62");
}

char *U_WMR63_set(void){
   return U_WMRCORENONE_set("U_WMR63");
}

char *U_WMR64_set(void){
   return U_WMRCORENONE_set("U_WMR64");
}

char *U_WMR65_set(void){
   return U_WMRCORENONE_set("U_WMR65");
}

char *U_WMR66_set(void){
   return U_WMRCORENONE_set("U_WMR66");
}

char *U_WMR67_set(void){
   return U_WMRCORENONE_set("U_WMR67");
}

char *U_WMR68_set(void){
   return U_WMRCORENONE_set("U_WMR68");
}

char *U_WMR69_set(void){
   return U_WMRCORENONE_set("U_WMR69");
}

char *U_WMR6A_set(void){
   return U_WMRCORENONE_set("U_WMR6A");
}

char *U_WMR6B_set(void){
   return U_WMRCORENONE_set("U_WMR6B");
}

char *U_WMR6C_set(void){
   return U_WMRCORENONE_set("U_WMR6C");
}

char *U_WMR6D_set(void){
   return U_WMRCORENONE_set("U_WMR6D");
}

char *U_WMR6E_set(void){
   return U_WMRCORENONE_set("U_WMR6E");
}

char *U_WMR6F_set(void){
   return U_WMRCORENONE_set("U_WMR6F");
}

char *U_WMR70_set(void){
   return U_WMRCORENONE_set("U_WMR70");
}

char *U_WMR71_set(void){
   return U_WMRCORENONE_set("U_WMR71");
}

char *U_WMR72_set(void){
   return U_WMRCORENONE_set("U_WMR72");
}

char *U_WMR73_set(void){
   return U_WMRCORENONE_set("U_WMR73");
}

char *U_WMR74_set(void){
   return U_WMRCORENONE_set("U_WMR74");
}

char *U_WMR75_set(void){
   return U_WMRCORENONE_set("U_WMR75");
}

char *U_WMR76_set(void){
   return U_WMRCORENONE_set("U_WMR76");
}

char *U_WMR77_set(void){
   return U_WMRCORENONE_set("U_WMR77");
}

char *U_WMR78_set(void){
   return U_WMRCORENONE_set("U_WMR78");
}

char *U_WMR79_set(void){
   return U_WMRCORENONE_set("U_WMR79");
}

char *U_WMR7A_set(void){
   return U_WMRCORENONE_set("U_WMR7A");
}

char *U_WMR7B_set(void){
   return U_WMRCORENONE_set("U_WMR7B");
}

char *U_WMR7C_set(void){
   return U_WMRCORENONE_set("U_WMR7C");
}

char *U_WMR7D_set(void){
   return U_WMRCORENONE_set("U_WMR7D");
}

char *U_WMR7E_set(void){
   return U_WMRCORENONE_set("U_WMR7E");
}

char *U_WMR7F_set(void){
   return U_WMRCORENONE_set("U_WMR7F");
}

char *U_WMR80_set(void){
   return U_WMRCORENONE_set("U_WMR80");
}

char *U_WMR81_set(void){
   return U_WMRCORENONE_set("U_WMR81");
}

char *U_WMR82_set(void){
   return U_WMRCORENONE_set("U_WMR82");
}

char *U_WMR83_set(void){
   return U_WMRCORENONE_set("U_WMR83");
}

char *U_WMR84_set(void){
   return U_WMRCORENONE_set("U_WMR84");
}

char *U_WMR85_set(void){
   return U_WMRCORENONE_set("U_WMR85");
}

char *U_WMR86_set(void){
   return U_WMRCORENONE_set("U_WMR86");
}

char *U_WMR87_set(void){
   return U_WMRCORENONE_set("U_WMR87");
}

char *U_WMR88_set(void){
   return U_WMRCORENONE_set("U_WMR88");
}

char *U_WMR89_set(void){
   return U_WMRCORENONE_set("U_WMR89");
}

char *U_WMR8A_set(void){
   return U_WMRCORENONE_set("U_WMR8A");
}

char *U_WMR8B_set(void){
   return U_WMRCORENONE_set("U_WMR8B");
}

char *U_WMR8C_set(void){
   return U_WMRCORENONE_set("U_WMR8C");
}

char *U_WMR8D_set(void){
   return U_WMRCORENONE_set("U_WMR8D");
}

char *U_WMR8E_set(void){
   return U_WMRCORENONE_set("U_WMR8E");
}

char *U_WMR8F_set(void){
   return U_WMRCORENONE_set("U_WMR8F");
}

char *U_WMR90_set(void){
   return U_WMRCORENONE_set("U_WMR90");
}

char *U_WMR91_set(void){
   return U_WMRCORENONE_set("U_WMR91");
}

char *U_WMR92_set(void){
   return U_WMRCORENONE_set("U_WMR92");
}

char *U_WMR93_set(void){
   return U_WMRCORENONE_set("U_WMR93");
}

char *U_WMR94_set(void){
   return U_WMRCORENONE_set("U_WMR94");
}

char *U_WMR95_set(void){
   return U_WMRCORENONE_set("U_WMR95");
}

char *U_WMR96_set(void){
   return U_WMRCORENONE_set("U_WMR96");
}

char *U_WMR97_set(void){
   return U_WMRCORENONE_set("U_WMR97");
}

char *U_WMR98_set(void){
   return U_WMRCORENONE_set("U_WMR98");
}

char *U_WMR99_set(void){
   return U_WMRCORENONE_set("U_WMR99");
}

char *U_WMR9A_set(void){
   return U_WMRCORENONE_set("U_WMR9A");
}

char *U_WMR9B_set(void){
   return U_WMRCORENONE_set("U_WMR9B");
}

char *U_WMR9C_set(void){
   return U_WMRCORENONE_set("U_WMR9C");
}

char *U_WMR9D_set(void){
   return U_WMRCORENONE_set("U_WMR9D");
}

char *U_WMR9E_set(void){
   return U_WMRCORENONE_set("U_WMR9E");
}

char *U_WMR9F_set(void){
   return U_WMRCORENONE_set("U_WMR9F");
}

char *U_WMRA0_set(void){
   return U_WMRCORENONE_set("U_WMRA0");
}

char *U_WMRA1_set(void){
   return U_WMRCORENONE_set("U_WMRA1");
}

char *U_WMRA2_set(void){
   return U_WMRCORENONE_set("U_WMRA2");
}

char *U_WMRA3_set(void){
   return U_WMRCORENONE_set("U_WMRA3");
}

char *U_WMRA4_set(void){
   return U_WMRCORENONE_set("U_WMRA4");
}

char *U_WMRA5_set(void){
   return U_WMRCORENONE_set("U_WMRA5");
}

char *U_WMRA6_set(void){
   return U_WMRCORENONE_set("U_WMRA6");
}

char *U_WMRA7_set(void){
   return U_WMRCORENONE_set("U_WMRA7");
}

char *U_WMRA8_set(void){
   return U_WMRCORENONE_set("U_WMRA8");
}

char *U_WMRA9_set(void){
   return U_WMRCORENONE_set("U_WMRA9");
}

char *U_WMRAA_set(void){
   return U_WMRCORENONE_set("U_WMRAA");
}

char *U_WMRAB_set(void){
   return U_WMRCORENONE_set("U_WMRAB");
}

char *U_WMRAC_set(void){
   return U_WMRCORENONE_set("U_WMRAC");
}

char *U_WMRAD_set(void){
   return U_WMRCORENONE_set("U_WMRAD");
}

char *U_WMRAE_set(void){
   return U_WMRCORENONE_set("U_WMRAE");
}

char *U_WMRAF_set(void){
   return U_WMRCORENONE_set("U_WMRAF");
}

char *U_WMRB0_set(void){
   return U_WMRCORENONE_set("U_WMRB0");
}

char *U_WMRB1_set(void){
   return U_WMRCORENONE_set("U_WMRB1");
}

char *U_WMRB2_set(void){
   return U_WMRCORENONE_set("U_WMRB2");
}

char *U_WMRB3_set(void){
   return U_WMRCORENONE_set("U_WMRB3");
}

char *U_WMRB4_set(void){
   return U_WMRCORENONE_set("U_WMRB4");
}

char *U_WMRB5_set(void){
   return U_WMRCORENONE_set("U_WMRB5");
}

char *U_WMRB6_set(void){
   return U_WMRCORENONE_set("U_WMRB6");
}

char *U_WMRB7_set(void){
   return U_WMRCORENONE_set("U_WMRB7");
}

char *U_WMRB8_set(void){
   return U_WMRCORENONE_set("U_WMRB8");
}

char *U_WMRB9_set(void){
   return U_WMRCORENONE_set("U_WMRB9");
}

char *U_WMRBA_set(void){
   return U_WMRCORENONE_set("U_WMRBA");
}

char *U_WMRBB_set(void){
   return U_WMRCORENONE_set("U_WMRBB");
}

char *U_WMRBC_set(void){
   return U_WMRCORENONE_set("U_WMRBC");
}

char *U_WMRBD_set(void){
   return U_WMRCORENONE_set("U_WMRBD");
}

char *U_WMRBE_set(void){
   return U_WMRCORENONE_set("U_WMRBE");
}

char *U_WMRBF_set(void){
   return U_WMRCORENONE_set("U_WMRBF");
}

char *U_WMRC0_set(void){
   return U_WMRCORENONE_set("U_WMRC0");
}

char *U_WMRC1_set(void){
   return U_WMRCORENONE_set("U_WMRC1");
}

char *U_WMRC2_set(void){
   return U_WMRCORENONE_set("U_WMRC2");
}

char *U_WMRC3_set(void){
   return U_WMRCORENONE_set("U_WMRC3");
}

char *U_WMRC4_set(void){
   return U_WMRCORENONE_set("U_WMRC4");
}

char *U_WMRC5_set(void){
   return U_WMRCORENONE_set("U_WMRC5");
}

char *U_WMRC6_set(void){
   return U_WMRCORENONE_set("U_WMRC6");
}

char *U_WMRC7_set(void){
   return U_WMRCORENONE_set("U_WMRC7");
}

char *U_WMRC8_set(void){
   return U_WMRCORENONE_set("U_WMRC8");
}

char *U_WMRC9_set(void){
   return U_WMRCORENONE_set("U_WMRC9");
}

char *U_WMRCA_set(void){
   return U_WMRCORENONE_set("U_WMRCA");
}

char *U_WMRCB_set(void){
   return U_WMRCORENONE_set("U_WMRCB");
}

char *U_WMRCC_set(void){
   return U_WMRCORENONE_set("U_WMRCC");
}

char *U_WMRCD_set(void){
   return U_WMRCORENONE_set("U_WMRCD");
}

char *U_WMRCE_set(void){
   return U_WMRCORENONE_set("U_WMRCE");
}

char *U_WMRCF_set(void){
   return U_WMRCORENONE_set("U_WMRCF");
}

char *U_WMRD0_set(void){
   return U_WMRCORENONE_set("U_WMRD0");
}

char *U_WMRD1_set(void){
   return U_WMRCORENONE_set("U_WMRD1");
}

char *U_WMRD2_set(void){
   return U_WMRCORENONE_set("U_WMRD2");
}

char *U_WMRD3_set(void){
   return U_WMRCORENONE_set("U_WMRD3");
}

char *U_WMRD4_set(void){
   return U_WMRCORENONE_set("U_WMRD4");
}

char *U_WMRD5_set(void){
   return U_WMRCORENONE_set("U_WMRD5");
}

char *U_WMRD6_set(void){
   return U_WMRCORENONE_set("U_WMRD6");
}

char *U_WMRD7_set(void){
   return U_WMRCORENONE_set("U_WMRD7");
}

char *U_WMRD8_set(void){
   return U_WMRCORENONE_set("U_WMRD8");
}

char *U_WMRD9_set(void){
   return U_WMRCORENONE_set("U_WMRD9");
}

char *U_WMRDA_set(void){
   return U_WMRCORENONE_set("U_WMRDA");
}

char *U_WMRDB_set(void){
   return U_WMRCORENONE_set("U_WMRDB");
}

char *U_WMRDC_set(void){
   return U_WMRCORENONE_set("U_WMRDC");
}

char *U_WMRDD_set(void){
   return U_WMRCORENONE_set("U_WMRDD");
}

char *U_WMRDE_set(void){
   return U_WMRCORENONE_set("U_WMRDE");
}

char *U_WMRDF_set(void){
   return U_WMRCORENONE_set("U_WMRDF");
}

char *U_WMRE0_set(void){
   return U_WMRCORENONE_set("U_WMRE0");
}

char *U_WMRE1_set(void){
   return U_WMRCORENONE_set("U_WMRE1");
}

char *U_WMRE2_set(void){
   return U_WMRCORENONE_set("U_WMRE2");
}

char *U_WMRE3_set(void){
   return U_WMRCORENONE_set("U_WMRE3");
}

char *U_WMRE4_set(void){
   return U_WMRCORENONE_set("U_WMRE4");
}

char *U_WMRE5_set(void){
   return U_WMRCORENONE_set("U_WMRE5");
}

char *U_WMRE6_set(void){
   return U_WMRCORENONE_set("U_WMRE6");
}

char *U_WMRE7_set(void){
   return U_WMRCORENONE_set("U_WMRE7");
}

char *U_WMRE8_set(void){
   return U_WMRCORENONE_set("U_WMRE8");
}

char *U_WMRE9_set(void){
   return U_WMRCORENONE_set("U_WMRE9");
}

char *U_WMREA_set(void){
   return U_WMRCORENONE_set("U_WMREA");
}

char *U_WMREB_set(void){
   return U_WMRCORENONE_set("U_WMREB");
}

char *U_WMREC_set(void){
   return U_WMRCORENONE_set("U_WMREC");
}

char *U_WMRED_set(void){
   return U_WMRCORENONE_set("U_WMRED");
}

char *U_WMREE_set(void){
   return U_WMRCORENONE_set("U_WMREE");
}

char *U_WMREF_set(void){
   return U_WMRCORENONE_set("U_WMREF");
}
//! \endcond

/**
    \brief Create and return a U_WMRDELETEOBJECT record
    \return pointer to the U_WMRDELETEOBJECT record, or NULL on error
    \param object Index of object to delete.
*/
char *U_WMRDELETEOBJECT_set(uint16_t object){
   return U_WMRCORE_1U16_set(U_WMR_DELETEOBJECT, object);
}

//! \cond
char *U_WMRF1_set(void){
   return U_WMRCORENONE_set("U_WMRF1");
}

char *U_WMRF2_set(void){
   return U_WMRCORENONE_set("U_WMRF2");
}

char *U_WMRF3_set(void){
   return U_WMRCORENONE_set("U_WMRF3");
}

char *U_WMRF4_set(void){
   return U_WMRCORENONE_set("U_WMRF4");
}

char *U_WMRF5_set(void){
   return U_WMRCORENONE_set("U_WMRF5");
}

char *U_WMRF6_set(void){
   return U_WMRCORENONE_set("U_WMRF6");
}
//! \endcond

/**
    \brief Create and return a U_WMRCREATEPALETTE record
    \return pointer to the U_WMRCREATEPALETTE record, or NULL on error
    \param Palette Create a Palette object.
*/
char *U_WMRCREATEPALETTE_set(U_PALETTE *Palette){
   return U_WMRCORE_PALETTE_set(U_WMR_CREATEPALETTE, Palette);
}

//! \cond
char *U_WMRF8_set(void){
   return U_WMRCORENONE_set("U_WMRF8");
}
//! \endcond

/**
    \brief Allocate and construct a U_WMRCREATEPATTERNBRUSH record.
    WARNING - U_WMRCREATEPATTERNBRUSH has been declared obsolete and application support is spotty -
    use U_WMRDIBCREATEPATTERNBRUSH instead.
    \return pointer to the U_WMRCREATEPATTERNBRUSH record, or NULL on error.
    \param Bm16         Pointer to a Bitmap16 Object, only the first 10 bytes are used.
    \param Pattern      byte array pattern, described by Bm16, for brush
*/
char *U_WMRCREATEPATTERNBRUSH_set(
     U_BITMAP16 *Bm16,
     char       *Pattern
   ){
   char *record;
   uint32_t  irecsize,off,cbPat;
   if(!Bm16 || !Pattern)return(NULL);
   
   cbPat =  (((Bm16->Width * Bm16->BitsPixel + 15) >> 4) << 1) * Bm16->Height;
   irecsize  = U_SIZE_METARECORD + 14 + 18 + cbPat;  /* core WMR + truncated Bm16 + 18 spaces bytes + pattern */
   record = malloc(irecsize);
   if(record){
      U_WMRCORE_SETRECHEAD(record,irecsize,U_WMR_CREATEPATTERNBRUSH);
      off = U_SIZE_METARECORD;
      memcpy(record + off, Bm16,        14);       off+=14;  /* Truncated bitmap16 object, last 4 bytes are to be ignored*/
      memset(record + off, 0,           18);       off+=18;  /* 18 bytes of zero, which are ignored */
      memcpy(record + off, Pattern,  cbPat);                 /* The pattern array */
   }
   return(record);
}

/**
    \brief Allocate and construct a U_WMRCREATEPENINDIRECT record.
    \return pointer to the U_WMRCREATEPENINDIRECT record, or NULL on error.
    \param pen          Parameters of the pen object to create.
*/
char *U_WMRCREATEPENINDIRECT_set(U_PEN pen){
    return U_WMRCORE_2U16_N16_set(U_WMR_CREATEPENINDIRECT, NULL, NULL, U_SIZE_PEN/2, &pen);
}

/**
    \brief Allocate and construct a U_WMRCREATEFONTINDIRECT record.
    \return pointer to the U_WMRCREATEFONTINDIRECT record, or NULL on error.
    \param font Parameters of the font object to create.
*/
char *U_WMRCREATEFONTINDIRECT_set(U_FONT *font){
   char *record=NULL;
   uint32_t  irecsize,off,flen;
   flen = 1 + strlen((char *)font->FaceName); /* include the null terminator in the count */
   if(flen & 1) flen++;                       /* make the allocation end line up at an even byte */
   irecsize  = U_SIZE_METARECORD + U_SIZE_FONT_CORE + flen;
   record = calloc(1,irecsize);
   if(record){
      U_WMRCORE_SETRECHEAD(record,irecsize,U_WMR_CREATEFONTINDIRECT);
      off = U_SIZE_METARECORD;
      memcpy(record+off,font,U_SIZE_FONT_CORE + flen);
   }
   return(record);
}

/**
    \brief Allocate and construct a U_WMRCREATEBRUSHINDIRECT record.
    \return pointer to the U_WMRCREATEBRUSHINDIRECT record, or NULL on error.
    \param brush Parameters of the brush object to create.
*/
char *U_WMRCREATEBRUSHINDIRECT_set(U_WLOGBRUSH brush){
    return U_WMRCORE_2U16_N16_set(U_WMR_CREATEBRUSHINDIRECT, NULL, NULL, U_SIZE_WLOGBRUSH/2, &brush);
}

/** in GDI and Wine, not in WMF manual.
*/
char *U_WMRCREATEBITMAPINDIRECT_set(void){
   return U_WMRCORENONE_set("U_WMRCREATEBITMAPINDIRECT");
}

/** in GDI and Wine, not in WMF manual.
*/
 char *U_WMRCREATEBITMAP_set(void){
   return U_WMRCORENONE_set("U_WMRCREATEBITMAP");
}

/**
    \brief Allocate and construct a U_WMRCREATEREGION record.
    \return pointer to the U_WMRCREATEREGION record, or NULL on error.
    \param region Parameters of the region object to create.
*/
char *U_WMRCREATEREGION_set(const U_REGION *region){
   char *record=NULL;
   uint32_t  irecsize,off;
   irecsize  = U_SIZE_METARECORD + region->Size;
   record = malloc(irecsize);
   if(record){
      U_WMRCORE_SETRECHEAD(record,irecsize,U_WMR_CREATEREGION);
      off = U_SIZE_METARECORD;
      memcpy(record+off,region,region->Size);
   }
   return(record);
}


/* all of the *_set are above, all of the *_get are below */

/* **********************************************************************************************
These functions are used for Image conversions and other
utility operations.  Character type conversions are in uwmf_utf.c
*********************************************************************************************** */


/**
    \brief Make up an approximate dx array to pass to U_WMREXTTEXTOUT_get(), based on character height and weight.
    
    Take abs. value of character height, get width by multiplying by 0.6, and correct weight
    approximately, with formula (measured on screen for one text line of Arial).
    Caller is responsible for free() on the returned pointer.
    
    \return pointer to dx array
    \param height  character height (absolute value will be used)
    \param weight  LF_Weight Enumeration (character weight) 
    \param members Number of entries to put into dx
    
*/
int16_t *dx16_get(
      int32_t  height,
      uint32_t weight,
      uint32_t members
   ){
   uint32_t i, width;
   int16_t *dx;
   dx = (int16_t *) malloc(members * sizeof(int16_t));
   if(dx){
       if(U_FW_DONTCARE == weight)weight=U_FW_NORMAL;
       width = (uint32_t) U_ROUND(((float) (height > 0 ? height : -height)) * 0.6 * (0.00024*(float) weight + 0.904));
       for ( i = 0; i < members; i++ ){ dx[i] = (width > INT16_MAX ? INT16_MAX : width); }
   }
   return(dx);
}

/**
    \brief Return the size of a WMF record, or 0 if it is found to be invalid.
    A valid record will have a size that does not cause it to extend
    beyond the end of data in memory. 
    A valid record will not be smaller than the smallest possible WMF record.
    \return size of the record in bytes, 0 on failure
    \param  contents   record to extract data from
    \param  blimit     one byte past the last WMF record in memory.
*/
size_t U_WMRRECSAFE_get(
      const char *contents, 
      const char *blimit
   ){
   size_t size=0;
   uint32_t Size16;
   memcpy(&Size16, contents + offsetof(U_METARECORD,Size16_4), 4);
   size = 2*Size16;
   /* Record is not self consistent - described size past the end of WMF in memory */
   if(size < U_SIZE_METARECORD ||
      contents + size - 1 >= blimit ||
      contents + size - 1 <  contents)size=0;
   return(size);
}


/* **********************************************************************************************
These functions create standard structures used in the WMR records.
*********************************************************************************************** */

// hide these from Doxygen
//! \cond
/* **********************************************************************************************
These functions contain shared code used by various U_WMR*_get functions.  These should NEVER be called
by end user code and to further that end prototypes are NOT provided and they are hidden from Doxygen.   
*********************************************************************************************** */

int U_WMRCORENONE_get(char *string){
   printf("unimplemented creator for:%s\n",string);
   return(0);
}

/*  Returns the record size in bytes for a valid record, or 0 for an invalid record.
    A valid record's size is at least as large as the minimum size passed in through minsize.
    Use U_WMRRECSAFE_get() to check if the record extends too far in memory.
*/
int U_WMRCORE_RECSAFE_get(
      const char *contents, 
      int         minsize
){
   int size=0;
   uint32_t Size16;
   memcpy(&Size16, contents + offsetof(U_METARECORD,Size16_4),4);
   size = 2*Size16;
   if(size < minsize)size=0;
   return(size);
}


/* records like U_WMRFLOODFILL and others. all args are optional, Color is not */
int U_WMRCORE_1U16_CRF_2U16_get(
      const char *contents,
      uint16_t   *arg1,
      U_COLORREF *Color,
      uint16_t   *arg2,
      uint16_t   *arg3
   ){
   int  size = 0;
   int  off  = U_SIZE_METARECORD;
   if(arg1){  memcpy(arg1,   contents + off, 2); off+=2; size+=2;}
              memcpy(Color,  contents + off, 4); off+=4; size+=4;
   if(arg2){  memcpy(arg2,   contents + off, 2); off+=2; size+=2;}
   if(arg3){  memcpy(arg3,   contents + off, 2);         size+=2;}
   return(size);
}

/* records that have a single uint16_t argument like U_WMRSETMAPMODE
   May also be used with int16_t with appropriate casts */
int U_WMRCORE_1U16_get(
      const char *contents, 
      int         minsize,
      uint16_t   *arg1
   ){
   int  size = U_WMRCORE_RECSAFE_get(contents, minsize);
   int  off  = U_SIZE_METARECORD;
   if(!size)return(0);
   memcpy(arg1, contents + off, 2);
   return(size);
}

/* records that have two uint16_t arguments like U_WMRSETBKMODE 
   May also be used with int16_t with appropriate casts */
int U_WMRCORE_2U16_get(
      const char *contents, 
      int         minsize,
      uint16_t   *arg1,
      uint16_t   *arg2
   ){
   int  size = U_WMRCORE_RECSAFE_get(contents, minsize);
   int  off  = U_SIZE_METARECORD;
   memcpy(arg1, contents + off, 2); off+=2;
   memcpy(arg2, contents + off, 2);
   return(size);
}

/* records that have four uint16_t arguments like U_WMRSCALEWINDOWEXT
   May also be used with int16_t with appropriate casts  */
int U_WMRCORE_4U16_get(
      const char *contents,
      int         minsize,
      uint16_t   *arg1,
      uint16_t   *arg2,
      uint16_t   *arg3,
      uint16_t   *arg4
   ){
   int  size = U_WMRCORE_RECSAFE_get(contents, minsize);
   int  off  = U_SIZE_METARECORD;
   if(!size)return(0);
   memcpy(arg1, contents + off, 2); off+=2;
   memcpy(arg2, contents + off, 2); off+=2;
   memcpy(arg3, contents + off, 2); off+=2;
   memcpy(arg4, contents + off, 2);
   return(size);
}

/* records that have five uint16_t arguments like U_WMRCREATEPENINDIRECT
   May also be used with int16_t with appropriate casts  */
int U_WMRCORE_5U16_get(
      const char *contents,
      int         minsize,
      uint16_t   *arg1,
      uint16_t   *arg2,
      uint16_t   *arg3,
      uint16_t   *arg4,
      uint16_t   *arg5
   ){
   int  size = U_WMRCORE_RECSAFE_get(contents, minsize);
   int  off  = U_SIZE_METARECORD;
   if(!size)return(0);
   memcpy(arg1, contents + off, 2); off+=2;
   memcpy(arg2, contents + off, 2); off+=2;
   memcpy(arg3, contents + off, 2); off+=2;
   memcpy(arg4, contents + off, 2); off+=2;
   memcpy(arg5, contents + off, 2);
   return(size);
}

/* records that have six uint16_t arguments like U_ROUNDREC
   May also be used with int16_t with appropriate casts  */
int U_WMRCORE_6U16_get(
      const char *contents,
      int         minsize,
      uint16_t   *arg1,
      uint16_t   *arg2,
      uint16_t   *arg3,
      uint16_t   *arg4,
      uint16_t   *arg5,
      uint16_t   *arg6
   ){
   int  size = U_WMRCORE_RECSAFE_get(contents, minsize);
   int  off  = U_SIZE_METARECORD;
   if(!size)return(0);
   memcpy(arg1, contents + off, 2); off+=2;
   memcpy(arg2, contents + off, 2); off+=2;
   memcpy(arg3, contents + off, 2); off+=2;
   memcpy(arg4, contents + off, 2); off+=2;
   memcpy(arg5, contents + off, 2); off+=2;
   memcpy(arg6, contents + off, 2);
   return(size);
}

/* records that have eight uint16_t arguments like U_WMRARC
   May also be used with int16_t with appropriate casts  */
int U_WMRCORE_8U16_get(
      const char *contents,
      int         minsize,
      uint16_t   *arg1,
      uint16_t   *arg2,
      uint16_t   *arg3,
      uint16_t   *arg4,
      uint16_t   *arg5,
      uint16_t   *arg6,
      uint16_t   *arg7,
      uint16_t   *arg8
   ){
   int  size = U_WMRCORE_RECSAFE_get(contents, minsize);
   int  off  = U_SIZE_METARECORD;
   if(!size)return(0);
   memcpy(arg1, contents + off, 2); off+=2;
   memcpy(arg2, contents + off, 2); off+=2;
   memcpy(arg3, contents + off, 2); off+=2;
   memcpy(arg4, contents + off, 2); off+=2;
   memcpy(arg5, contents + off, 2); off+=2;
   memcpy(arg6, contents + off, 2); off+=2;
   memcpy(arg7, contents + off, 2); off+=2;
   memcpy(arg8, contents + off, 2); 
   return(size);
}

/* records that have
  arg1 an (optional) (u)int16
  arg2 an (optional( (u)int16
  array of data cells or just a bunch of data.  Passed as a char because the structures in the WMF in memory may
    not be aligned properly for those structures.  Caller has to take them apart - carefully.
  like U_WMRCREATEBRUSHINDIRECT with arg1=arg2=NULL
*/
int U_WMRCORE_2U16_N16_get(
      const char *contents,
      int         minsize,
      uint16_t   *arg1,
      uint16_t   *arg2,
      const char **array
   ){
   int  size = U_WMRCORE_RECSAFE_get(contents, minsize);
   int  off  = U_SIZE_METARECORD;
   if(!size)return(0);
   if(arg1){ memcpy(arg1, contents + off, 2); off+=2; }
   if(arg2){ memcpy(arg2, contents + off, 2); off+=2; }
   *array =   (contents + off);
   return(size);
}



/* records that get a U_PALETTE like U_WMRANIMATEPALETTE.  Fills in the first two fields of U_PALETTE only, and returns
   returns a separateepointer to the PalEntries[] array.  This pointer is most likely not aligned with the data.
 */
int U_WMRCORE_PALETTE_get(
      const char *contents,
      int         minsize,
      U_PALETTE  *Palette,
      const char **PalEntries 
   ){
   int  size = U_WMRCORE_RECSAFE_get(contents, minsize);
   if(!size)return(0);
   contents += offsetof(U_WMRANIMATEPALETTE, Palette);
   memset(Palette, 0,        (U_SIZE_PALETTE));
   memcpy(Palette, contents, (U_SIZE_PALETTE));
   *PalEntries = (contents + offsetof(U_PALETTE, PalEntries));
   return(size);
}

//! \endcond

/**
    \brief Return parameters from a bitmapcoreheader.
    All are returned as 32 bit integers, regardless of their internal representation.
    
    \param BmiCh       char * pointer to a U_BITMAPCOREHEADER.    Note, data may not be properly aligned.
    \param Size        size of the coreheader in bytes
    \param Width;      Width of pixel array
    \param Height;     Height of pixel array
    \param BitCount    Pixel Format (BitCount Enumeration)
*/
void U_BITMAPCOREHEADER_get(
       const char *BmiCh,
       int32_t    *Size,
       int32_t    *Width,
       int32_t    *Height,
       int32_t    *BitCount
    ){
    uint32_t utmp4;
    uint16_t utmp2;
    memcpy(&utmp4,   BmiCh + offsetof(U_BITMAPCOREHEADER,Size_4),   4); *Size      = utmp4;   
    memcpy(&utmp2,   BmiCh + offsetof(U_BITMAPCOREHEADER,Width),    2); *Width     = utmp2;   
    memcpy(&utmp2,   BmiCh + offsetof(U_BITMAPCOREHEADER,Height),   2); *Height    = utmp2;  
    memcpy(&utmp2,   BmiCh + offsetof(U_BITMAPCOREHEADER,BitCount), 2); *BitCount  = utmp2;
}

/**
    \brief Return parameters from a bitinfoheader.
    All are returned as 32 bit integers, regardless of their internal representation.
    
    \param Bmih             char * pointer to a U_BITMAPINFOHEADER.    Note, data may not be properly aligned.
    \param Size             Structure size in bytes
    \param Width            Bitmap width in pixels
    \param Height           Bitmap height in pixels, may be negative.
    \param Planes           Planes (must be 1)
    \param BitCount         BitCount Enumeration (determines number of RBG colors)
    \param Compression      BI_Compression Enumeration
    \param SizeImage        Image size in bytes or 0 = "default size (calculated from geometry?)"
    \param XPelsPerMeter    X Resolution in pixels/meter
    \param YPelsPerMeter    Y Resolution in pixels/meter
    \param ClrUsed          Number of bmciColors in U_BITMAPINFO/U_BITMAPCOREINFO that are used by the bitmap
    \param ClrImportant     Number of bmciColors needed (0 means all).


*/
void U_BITMAPINFOHEADER_get(
       const char *Bmih,
       uint32_t   *Size,          
       int32_t    *Width,         
       int32_t    *Height,        
       uint32_t   *Planes,        
       uint32_t   *BitCount,      
       uint32_t   *Compression,   
       uint32_t   *SizeImage,     
       int32_t    *XPelsPerMeter, 
       int32_t    *YPelsPerMeter, 
       uint32_t   *ClrUsed,       
       uint32_t   *ClrImportant  
    ){
    int32_t   tmp4;
    uint32_t utmp4;
    uint16_t utmp2;
    
    memcpy(&utmp4,  Bmih + offsetof(U_BITMAPINFOHEADER,biSize         ),   4);   *Size          = utmp4;
    memcpy( &tmp4,  Bmih + offsetof(U_BITMAPINFOHEADER,biWidth        ),   4);   *Width         =  tmp4;
    memcpy( &tmp4,  Bmih + offsetof(U_BITMAPINFOHEADER,biHeight       ),   4);   *Height        =  tmp4;
    memcpy(&utmp2,  Bmih + offsetof(U_BITMAPINFOHEADER,biPlanes       ),   2);   *Planes        = utmp2;
    memcpy(&utmp2,  Bmih + offsetof(U_BITMAPINFOHEADER,biBitCount     ),   2);   *BitCount      = utmp2;
    memcpy(&utmp4,  Bmih + offsetof(U_BITMAPINFOHEADER,biCompression  ),   4);   *Compression   = utmp4;
    memcpy(&utmp4,  Bmih + offsetof(U_BITMAPINFOHEADER,biSizeImage    ),   4);   *SizeImage     = utmp4;
    memcpy( &tmp4,  Bmih + offsetof(U_BITMAPINFOHEADER,biXPelsPerMeter),   4);   *XPelsPerMeter =  tmp4;
    memcpy( &tmp4,  Bmih + offsetof(U_BITMAPINFOHEADER,biYPelsPerMeter),   4);   *YPelsPerMeter =  tmp4;
    memcpy(&utmp4,  Bmih + offsetof(U_BITMAPINFOHEADER,biClrUsed      ),   4);   *ClrUsed       = utmp4;
    memcpy(&utmp4,  Bmih + offsetof(U_BITMAPINFOHEADER,biClrImportant ),   4);   *ClrImportant  = utmp4;
}

/**
    \brief Assume a packed DIB and get the parameters from it, use by DBI_to_RGBA()
    
    \return BI_Compression Enumeration.  For anything other than U_BI_RGB values other than px may not be valid.
    \param dib         pointer to the start of the DIB in the record
    \param px          pointer to DIB pixel array
    \param ct          pointer to DIB color table
    \param numCt       DIB color table number of entries, for PNG or JPG returns the number of bytes in the image
    \param width       Width of pixel array
    \param height      Height of pixel array (always returned as a positive number)
    \param colortype   DIB BitCount Enumeration
    \param invert      If DIB rows are in opposite order from RGBA rows
*/
int wget_DIB_params(
       const char   *dib,
       const char  **px,
       const U_RGBQUAD **ct,
       int32_t      *numCt,
       int32_t      *width,
       int32_t      *height,
       int32_t      *colortype,
       int32_t      *invert
   ){
   uint32_t bic;
   int32_t Size;
   bic = U_BI_RGB;  // this information is not in the coreheader;
   U_BITMAPCOREHEADER_get(dib, &Size, width, height, colortype);
   if(Size != 0xC ){ //BitmapCoreHeader
       /* if biCompression is not U_BI_RGB some or all of the following might not hold real values.
       Ignore most of the information returned from the bitmapinfoheader.
       */
       uint32_t uig4;
       int32_t  ig4;
       U_BITMAPINFOHEADER_get(dib, &uig4, width, height,&uig4, (uint32_t *) colortype, &bic, &uig4, &ig4, &ig4,&uig4, &uig4);
   }
   if(*height < 0){
      *height = -*height;
      *invert = 1;
   }
   else {
      *invert = 0;
   }
   *px = dib + U_SIZE_BITMAPINFOHEADER;
   if(bic == U_BI_RGB){
      *numCt     = get_real_color_count(dib);
      if(*numCt){ 
         *ct = (U_RGBQUAD *) (dib + U_SIZE_BITMAPINFOHEADER); 
         *px += U_SIZE_COLORREF * (*numCt);
      }
      else {      *ct = NULL;                                            }
   }
   else {
      memcpy(numCt,  dib + offsetof(U_BITMAPINFOHEADER,biSizeImage),    4);
      *ct        = NULL;
   }
   return(bic);
}



/* **********************************************************************************************
These are the core WMR functions, each extracts data from a particular type of record.
In general routines fill in structures which have been passed in by the caller, and zero them
if that (optional) structure is not present.
Because the WMF records may not be aligned they are generally copied into the supplied
  aligned structs, so that the caller may retrieve fields with the usual sorts of
  structure operations: Struct.field or (*Struct)->field.
A few routines return pointers to data regions in the record.
They are listed in order by the corresponding U_WMR_* index number.  
*********************************************************************************************** */

/**
    \brief Get data from a (placeable) WMR_HEADER.  
    \return size of the record in bytes, 0 on failure
    \param  contents   record to extract data from
    \param  blimit     one byte past the last WMF record in memory.
    \param  Placeable  U_WMRPLACEABLE data, if any
    \param  Header     U_WMRHEADER data, if any
*/
int wmfheader_get(
      const char      *contents, 
      const char      *blimit,
      U_WMRPLACEABLE  *Placeable,
      U_WMRHEADER     *Header
   ){
   uint32_t Key;
   int size=0;
   if(!Placeable || !Header || contents + 4 >= blimit)return(0);
   memcpy(&Key, contents + offsetof(U_WMRPLACEABLE,Key), 4);
   if(Key == 0x9AC6CDD7){
      size     += U_SIZE_WMRPLACEABLE;
      if(contents + size >= blimit)return(0);
      memcpy(Placeable, contents, U_SIZE_WMRPLACEABLE);
      contents += U_SIZE_WMRPLACEABLE;
   }
   else {
      memset(Placeable, 0, U_SIZE_WMRPLACEABLE);
   }
   if(contents + size + U_SIZE_WMRHEADER >= blimit)return(0);
   size += 2* (*(uint16_t *)(contents + offsetof(U_WMRHEADER,Size16w))); 
   if(contents + size >= blimit)return(0);
   memcpy(Header, contents, U_SIZE_WMRHEADER);
   return(size);
}


/**
    \brief Get data from a  U_WMREOF record
    \return size of record in bytes, or 0 on error
    \param  contents   record to extract data from
*/
int U_WMREOF_get(
      const char *contents
   ){
   return(U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMREOF)));
}                            

/**
    \brief Retrieve values from a U_WMRSETBKCOLOR record
    \return length of the U_WMRSETBKCOLOR record, or NULL on error
    \param  contents   record to extract data from
    \param  Color Background Color.
*/
int U_WMRSETBKCOLOR_get(
      const char  *contents, 
      U_COLORREF  *Color
   ){
   int size = U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMRSETBKCOLOR));
   if(!size)return(0);
   memcpy(Color,contents + offsetof(U_WMRSETBKCOLOR,Color),U_SIZE_COLORREF);
   return(size);
}

/**
    \brief Retrieve values from a U_WMRSETBKMODE record
    \return length of the U_WMRSETBKMODE record, or NULL on error
    \param  contents   record to extract data from
    \param  Mode MixMode Enumeration
*/
int U_WMRSETBKMODE_get(
      const char *contents, 
      uint16_t   *Mode
   ){
   return(U_WMRCORE_1U16_get(contents, (U_SIZE_WMRSETBKMODE), Mode));
}

/**
    \brief Retrieve values from a U_WMRSETMAPMODE record
    \return length of the U_WMRSETMAPMODE record, or NULL on error
    \param  contents   record to extract data from
    \param  Mode MapMode Enumeration
*/
int U_WMRSETMAPMODE_get(
      const char *contents, 
      uint16_t   *Mode
   ){
   return(U_WMRCORE_1U16_get(contents, (U_SIZE_WMRSETMAPMODE), Mode));
}

/**
    \brief Retrieve values from a U_WMRSETROP2 record
    \return length of the U_WMRSETROP2 record, or NULL on error
    \param  contents   record to extract data from
    \param  Mode Binary Raster Operation Enumeration
*/
int U_WMRSETROP2_get(
      const char *contents, 
      uint16_t   *Mode
   ){
   return(U_WMRCORE_1U16_get(contents, (U_SIZE_WMRSETROP2), Mode));
}

/**
    \brief Get data from a  U_WMRSETRELABS record
    \return length of the U_WMRSETRELABS record in bytes, or 0 on error
    \param  contents   record to extract data from
*/
int U_WMRSETRELABS_get(
      const char *contents 
   ){
   return(U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMRSETRELABS)));
}                            

/**
    \brief Retrieve values from a U_WMRSETPOLYFILLMODE record
    \return length of the U_WMRSETPOLYFILLMODE record, or NULL on error
    \param  contents   record to extract data from
    \param  Mode PolyFillMode Enumeration
*/
int U_WMRSETPOLYFILLMODE_get(
      const char *contents, 
      uint16_t   *Mode
   ){
   return(U_WMRCORE_1U16_get(contents, (U_SIZE_WMRSETPOLYFILLMODE), Mode));
}

/**
    \brief Retrieve values from a U_WMRSETSTRETCHBLTMODE record
    \return length of the U_WMRSETSTRETCHBLTMODE record, or NULL on error
    \param  contents   record to extract data from
    \param  Mode StretchMode Enumeration
*/
int U_WMRSETSTRETCHBLTMODE_get(
      const char *contents, 
      uint16_t   *Mode
   ){
   return(U_WMRCORE_1U16_get(contents, (U_SIZE_WMRSETSTRETCHBLTMODE), Mode));
}

/**
    \brief Retrieve values from a U_WMRSETTEXTCHAREXTRA record
    \return length of the U_WMRSETTEXTCHAREXTRA record, or NULL on error
    \param  contents   record to extract data from
    \param  Mode Extra space in logical units to add to each character
*/
int U_WMRSETTEXTCHAREXTRA_get(
      const char *contents, 
      uint16_t   *Mode
   ){
   return(U_WMRCORE_1U16_get(contents, (U_SIZE_WMRSETTEXTCHAREXTRA), Mode));
}

/**
    \brief Retrieve values from a U_WMRSETTEXTCOLOR record
    \return length of the U_WMRSETTEXTCOLOR record, or NULL on error
    \param  contents   record to extract data from
    \param  Color Text Color.
*/
int U_WMRSETTEXTCOLOR_get(
      const char  *contents, 
      U_COLORREF  *Color
   ){
   int size = U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMRSETTEXTCOLOR));
   if(!size)return(0);
   memcpy(Color,contents + offsetof(U_WMRSETTEXTCOLOR,Color),U_SIZE_COLORREF);
   return(size);
}

/**
    \brief Retrieve values from a U_WMRSETTEXTJUSTIFICATION record
    \return length of the U_WMRSETTEXTJUSTIFICATION record, or NULL on error
    \param  contents   record to extract data from
    \param  Count Number of space characters in the line.
    \param  Extra Number of extra space characters to add to the line.
*/
int U_WMRSETTEXTJUSTIFICATION_get(
      const char *contents, 
      uint16_t   *Count,
      uint16_t   *Extra
   ){
   return(U_WMRCORE_2U16_get(contents, (U_SIZE_WMRSETTEXTJUSTIFICATION), Count, Extra));
}

/**
    \brief Retrieve values from a U_WMRSETWINDOWORG record
    \return length of the U_WMRSETWINDOWORG record, or NULL on error
    \param  contents   record to extract data from
    \param  coord Window Origin.
*/
int U_WMRSETWINDOWORG_get(
      const char *contents, 
      U_POINT16 * coord
   ){
   return(U_WMRCORE_2U16_get(contents, (U_SIZE_WMRSETWINDOWORG), U_P16(coord->y), U_P16(coord->x)));
}

/**
    \brief Retrieve values from a U_WMRSETWINDOWEXT record
    \return length of the U_WMRSETWINDOWEXT record, or NULL on error
    \param  contents   record to extract data from
    \param  extent     Window Extent.
*/
int U_WMRSETWINDOWEXT_get(
      const char *contents, 
      U_POINT16 * extent
   ){
   return(U_WMRCORE_2U16_get(contents, (U_SIZE_WMRSETWINDOWEXT), U_P16(extent->y), U_P16(extent->x)));
}

/**
    \brief Retrieve values from a U_WMRSETVIEWPORTORG record
    \return length of the U_WMRSETVIEWPORTORG record, or NULL on error
    \param  contents   record to extract data from
    \param  coord Viewport Origin.
*/
int U_WMRSETVIEWPORTORG_get(
      const char *contents, 
      U_POINT16 * coord
   ){
   return(U_WMRCORE_2U16_get(contents, (U_SIZE_WMRSETVIEWPORTORG), U_P16(coord->y), U_P16(coord->x)));

}

/**
    \brief Retrieve values from a U_WMRSETVIEWPORTEXT record
    \return length of the U_WMRSETVIEWPORTEXT record, or NULL on error
    \param  contents   record to extract data from
    \param  extent     Viewport Extent.
*/
int U_WMRSETVIEWPORTEXT_get(
      const char *contents, 
      U_POINT16 * extent
   ){
   return(U_WMRCORE_2U16_get(contents, (U_SIZE_WMRSETVIEWPORTEXT), U_P16(extent->y), U_P16(extent->x)));
}

/**
    \brief Retrieve values from a U_WMROFFSETWINDOWORG record
    \return length of the U_WMROFFSETWINDOWORG record, or NULL on error
    \param  contents   record to extract data from
    \param  offset Window offset in device units.
*/
int U_WMROFFSETWINDOWORG_get(
      const char *contents, 
      U_POINT16 * offset
   ){
   return(U_WMRCORE_2U16_get(contents, (U_SIZE_WMROFFSETWINDOWORG), U_P16(offset->y), U_P16(offset->x)));
}

/**
    \brief Retrieve values from a U_WMRSCALEWINDOWEXT record
    \return length of the U_WMRSCALEWINDOWEXT record, or NULL on error
    \param  contents   record to extract data from
    \param  Denom {X,Y} denominators.
    \param  Num   {X,Y} numerators.
*/
int U_WMRSCALEWINDOWEXT_get(
      const char *contents, 
     U_POINT16 *  Denom, 
     U_POINT16 *  Num
   ){
   return(U_WMRCORE_4U16_get(contents, (U_SIZE_WMRSCALEWINDOWEXT), U_P16(Denom->y), U_P16(Denom->x), U_P16(Num->y), U_P16(Num->x)));
}

/**
    \brief Retrieve values from a U_WMROFFSETVIEWPORTORG record
    \return length of the U_WMROFFSETVIEWPORTORG record, or NULL on error
    \param  contents   record to extract data from
    \param  offset Viewport offset in device units.
*/
int U_WMROFFSETVIEWPORTORG_get(
      const char *contents,
      U_POINT16 * offset
   ){
   return(U_WMRCORE_2U16_get(contents, (U_SIZE_WMROFFSETVIEWPORTORG), U_P16(offset->y), U_P16(offset->x)));
}

/**
    \brief Retrieve values from a U_WMRSCALEVIEWPORTEXT record
    \return length of the U_WMRSCALEVIEWPORTEXT record, or NULL on error
    \param  contents   record to extract data from
    \param  Denom {X,Y} denominators.
    \param  Num   {X,Y} numerators.
*/
int U_WMRSCALEVIEWPORTEXT_get(
      const char *contents, 
     U_POINT16 *  Denom, 
     U_POINT16 *  Num
   ){
   return(U_WMRCORE_4U16_get(contents, (U_SIZE_WMRSCALEVIEWPORTEXT), U_P16(Denom->y), U_P16(Denom->x), U_P16(Num->y), U_P16(Num->x)));
}

/**
    \brief Retrieve values from a U_WMRLINETO record
    \return length of the U_WMRLINETO record, or NULL on error
    \param  contents   record to extract data from
    \param  coord Draw line to {X,Y}.
*/
int U_WMRLINETO_get(
      const char *contents, 
      U_POINT16 * coord
   ){
   return(U_WMRCORE_2U16_get(contents, (U_SIZE_WMRLINETO), U_P16(coord->y), U_P16(coord->x)));
}

/**
    \brief Retrieve values from a U_WMRMOVETO record
    \return length of the U_WMRMOVETO record, or NULL on error
    \param  contents   record to extract data from
    \param  coord Move to {X,Y}.
*/
int U_WMRMOVETO_get(
      const char *contents, 
      U_POINT16 * coord
   ){
   return(U_WMRCORE_2U16_get(contents, (U_SIZE_WMRMOVETO), U_P16(coord->y), U_P16(coord->x)));
}

/**
    \brief Retrieve values from a U_WMREXCLUDECLIPRECT record
    \return length of the U_WMREXCLUDECLIPRECT record, or NULL on error
    \param  contents   record to extract data from
    \param  rect Exclude rect from clipping region.
*/
int U_WMREXCLUDECLIPRECT_get(
      const char *contents,
      U_RECT16   *rect
   ){
   return(U_WMRCORE_4U16_get(contents, (U_SIZE_WMREXCLUDECLIPRECT), U_P16(rect->bottom), U_P16(rect->right), U_P16(rect->top), U_P16(rect->left)));
}

/**
    \brief Retrieve values from a U_WMRINTERSECTCLIPRECT record
    \return length of the U_WMRINTERSECTCLIPRECT record, or NULL on error
    \param  contents   record to extract data from
    \param  rect Clipping region is intersection of existing clipping region with rect.
*/
int U_WMRINTERSECTCLIPRECT_get(
      const char *contents, 
      U_RECT16   *rect
   ){
   return(U_WMRCORE_4U16_get(contents, (U_SIZE_WMRINTERSECTCLIPRECT), U_P16(rect->bottom), U_P16(rect->right), U_P16(rect->top), U_P16(rect->left)));
}

/**
    \brief Retrieve values from a U_WMRARC record
    \return length of the U_WMRARC record, or NULL on error
    \param  contents   record to extract data from
    \param  StartArc   Start of Arc
    \param  EndArc     End of Arc
    \param  rect       Bounding rectangle.
*/
int U_WMRARC_get(
      const char *contents, 
      U_POINT16  *StartArc,
      U_POINT16  *EndArc,
      U_RECT16   *rect
   ){
   return U_WMRCORE_8U16_get(
      contents,
      (U_SIZE_WMRARC),
      U_P16(EndArc->y),
      U_P16(EndArc->x),
      U_P16(StartArc->y),
      U_P16(StartArc->x),
      U_P16(rect->bottom), 
      U_P16(rect->right),
      U_P16(rect->top),
      U_P16(rect->left)
    );
}

/**
    \brief Retrieve values from a U_WMRELLIPSE record
    \return length of the U_WMRELLIPSE record, or NULL on error
    \param  contents   record to extract data from
    \param  rect Bounding rectangle for Ellipse.
*/
int U_WMRELLIPSE_get(
      const char *contents,
      U_RECT16   *rect
   ){
   return U_WMRCORE_4U16_get(
      contents,
      (U_SIZE_WMRELLIPSE),
      U_P16(rect->bottom), 
      U_P16(rect->right),
      U_P16(rect->top),
      U_P16(rect->left)
   );
}

/**
    \brief Retrieve values from a U_WMRFLOODFILL record
    \return length of the U_WMRFLOODFILL record, or NULL on error
    \param  contents   record to extract data from
    \param  Mode   FloodFill Enumeration.
    \param  Color  Color to Fill with.
    \param  coord  Location to start fill.
*/
int U_WMRFLOODFILL_get(
      const char *contents,
      uint16_t   *Mode, 
      U_COLORREF *Color, 
      U_POINT16  *coord
   ){
   return  U_WMRCORE_1U16_CRF_2U16_get(
      contents,
      Mode,
      Color,
      U_P16(coord->y),
      U_P16(coord->x)
   );
}

/**
    \brief Retrieve values from a U_WMRPIE record
    \return length of the U_WMRPIE record, or NULL on error
    \param  contents   record to extract data from
    \param  Radial1    Start of Pie
    \param  Radial2    End of Pie
    \param  rect       Bounding rectangle.
*/
int U_WMRPIE_get(
      const char *contents,
      U_POINT16 *Radial1,
      U_POINT16 *Radial2,
      U_RECT16  *rect
   ){
   return U_WMRCORE_8U16_get(
      contents,
      (U_SIZE_WMRPIE),
      U_P16(Radial2->y), 
      U_P16(Radial2->x),
      U_P16(Radial1->y),
      U_P16(Radial1->x),
      U_P16(rect->bottom), 
      U_P16(rect->right),
      U_P16(rect->top),
      U_P16(rect->left)
    );
}

/**
    \brief Retrieve values from a U_WMRRECTANGLE record
    \return length of the U_WMRRECTANGLE record, or NULL on error
    \param  contents   record to extract data from
    \param  rect       Boundaries.
*/
int U_WMRRECTANGLE_get(
      const char *contents,
      U_RECT16   *rect
   ){
   return U_WMRCORE_4U16_get(
      contents,
      (U_SIZE_WMRRECTANGLE),
      U_P16(rect->bottom), 
      U_P16(rect->right),
      U_P16(rect->top),
      U_P16(rect->left)
   );
}

/**
    \brief Retrieve values from a U_WMRROUNDRECT record
    \return length of the U_WMRROUNDRECT record, or NULL on error
    \param  contents   record to extract data from
    \param  Width      Horizontal rounding length.
    \param  Height     Vertical rounding length.
    \param  rect       Boundaries.
*/
int U_WMRROUNDRECT_get(
      const char *contents,
      int16_t    *Width, 
      int16_t    *Height,
      U_RECT16   *rect
   ){
   return U_WMRCORE_6U16_get(
      contents,
      (U_SIZE_WMRROUNDRECT),
      U_PP16(Height), 
      U_PP16(Width),
      U_P16(rect->bottom), 
      U_P16(rect->right),
      U_P16(rect->top),
      U_P16(rect->left)
   );
}

/**
    \brief Get data from a  U_WMRPATBLT record.
    \return length of the U_WMRPATBLT record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Dst       Destination UL corner in logical units
    \param  cwh       W & H for Dst and Src in logical units
    \param  dwRop3    RasterOPeration Enumeration
*/
int U_WMRPATBLT_get(
      const char     *contents,
      U_POINT16 *     Dst,
      U_POINT16 *     cwh,
      uint32_t       *dwRop3
   ){
   int  size = U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMRPATBLT));
   if(!size)return(0);
   memcpy(dwRop3,                   ( contents + offsetof(U_WMRPATBLT, rop3w)), 4);
           cwh->y     = *(int16_t *)( contents + offsetof(U_WMRPATBLT, Height    ));
           cwh->x     = *(int16_t *)( contents + offsetof(U_WMRPATBLT, Width     ));
           Dst->y     = *(int16_t *)( contents + offsetof(U_WMRPATBLT, yDst      ));
           Dst->x     = *(int16_t *)( contents + offsetof(U_WMRPATBLT, xDst      ));
   return(size);
}

/**
    \brief Get data from a  U_WMRSAVEDC record
    \return length of the U_WMRSAVEDC record in bytes, or 0 on error
    \param  contents   record to extract data from
*/
int U_WMRSAVEDC_get(
      const char *contents
   ){
   return(U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMRSAVEDC)));
}

/**
    \brief Get data from a  U_WMRSETPIXEL record
    \return length of the U_WMRSETPIXEL record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Color      pointer to a U_COLORREF variable where the color will be stored.
    \param  Coord      pointer to a U_POINT16 variable where the coordinates will be stored.
*/
int U_WMRSETPIXEL_get(
      const char *contents,
      U_COLORREF *Color, 
      U_POINT16  *Coord){
   return  U_WMRCORE_1U16_CRF_2U16_get(
      contents,
      NULL,
      Color,
      U_P16(Coord->y),
      U_P16(Coord->x)
   );
}

/**
    \brief Get data from a  U_WMROFFSETCLIPRGN record
    \return length of the U_WMROFFSETCLIPRGN record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  offset     pointer to a U_POINT16 variable where the x,y offsets will be stored.
*/
int U_WMROFFSETCLIPRGN_get(
      const char *contents,
      U_POINT16  *offset
   ){
   return U_WMRCORE_2U16_get(contents, (U_SIZE_WMROFFSETCLIPRGN), U_P16(offset->y), U_P16(offset->x));
}

/**
    \brief Get data from a  U_WMRTEXTOUT record
    \return length of the U_WMRTEXTOUT record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Dst        coordinates where text will be written
    \param  Length     Number of characters in string.
    \param  string     Pointer to string in WMF buffer in memory.  This text is generally NOT null terminated!!!
*/
int U_WMRTEXTOUT_get(
      const char  *contents,
      U_POINT16 *  Dst, 
      int16_t     *Length,
      const char **string
   ){
   int16_t L2;
   int  off;
   int  size = U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMRTEXTOUT));
   if(!size)return(0);
   *Length = *(int16_t *)(contents + offsetof(U_WMRTEXTOUT, Length));
   *string = contents + offsetof(U_WMRTEXTOUT, String);  /* May not be null terminated!!! */
   L2 = *Length;
   if(L2 & 1)L2++;
   off = U_SIZE_METARECORD + 2 + L2;
   memcpy(&Dst->y, contents + off, 2); off+=2;
   memcpy(&Dst->x, contents + off, 2);
   return(size);
}

/**
    \brief Get data from a  U_WMRBITBLT record.
     Note that unlike U_EMRBITBLT there is no scaling available - the Src and Dst
      rectangles must be the same size.
    \return length of the U_WMRBITBLT record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Dst        Destination UL corner in logical units
    \param  cwh        W & H for Dst and Src in logical units
    \param  Src        Source UL corner in logical units
    \param  dwRop3     RasterOPeration Enumeration
    \param  Bm16       bitmap16 object (fields in it are all 0 if no bitmap is used)
    \param  px         pointer to bitmap in memory, or NULL if not used
*/
int U_WMRBITBLT_get(
      const char  *contents,
      U_POINT16 *  Dst,
      U_POINT16 *  cwh,
      U_POINT16 *  Src,
      uint32_t    *dwRop3,
      U_BITMAP16  *Bm16,
      const char **px
   ){
   uint8_t   xb;
   uint32_t  size = U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMRBITBLT_NOPX));
   if(!size)return(0);
   xb               = *(uint8_t *)( contents + offsetof(U_METARECORD, xb));
   if(U_TEST_NOPXB(size,xb)){ /* no bitmap */
      memcpy(dwRop3,              ( contents + offsetof(U_WMRBITBLT_NOPX, rop3w)), 4);
             Src->y = *(int16_t *)( contents + offsetof(U_WMRBITBLT_NOPX, ySrc      ));
             Src->x = *(int16_t *)( contents + offsetof(U_WMRBITBLT_NOPX, xSrc      ));
             cwh->y = *(int16_t *)( contents + offsetof(U_WMRBITBLT_NOPX, Height    ));
             cwh->x = *(int16_t *)( contents + offsetof(U_WMRBITBLT_NOPX, Width     ));
             Dst->y = *(int16_t *)( contents + offsetof(U_WMRBITBLT_NOPX, yDst      ));
             Dst->x = *(int16_t *)( contents + offsetof(U_WMRBITBLT_NOPX, xDst      ));
      memset(Bm16, 0, U_SIZE_BITMAP16);
             *px    = NULL;
   }
   else { /* yes bitmap */
      memcpy(dwRop3,              ( contents + offsetof(U_WMRBITBLT_PX, rop3w)), 4);
             Src->y = *(int16_t *)( contents + offsetof(U_WMRBITBLT_PX, ySrc      ));
             Src->x = *(int16_t *)( contents + offsetof(U_WMRBITBLT_PX, xSrc      ));
             cwh->y = *(int16_t *)( contents + offsetof(U_WMRBITBLT_PX, Height    ));
             cwh->x = *(int16_t *)( contents + offsetof(U_WMRBITBLT_PX, Width     ));
             Dst->y = *(int16_t *)( contents + offsetof(U_WMRBITBLT_PX, yDst      ));
             Dst->x = *(int16_t *)( contents + offsetof(U_WMRBITBLT_PX, xDst      ));
      memcpy(Bm16,                ( contents + offsetof(U_WMRBITBLT_PX, bitmap)), U_SIZE_BITMAP16);
             *px       =          ( contents + offsetof(U_WMRBITBLT_PX, bitmap) + U_SIZE_BITMAP16);
   }
   return(size);
}

/**
    \brief Get data from a  U_WMRSTRETCHBLT record.
    \return length of the U_WMRSTRETCHBLT record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Dst        Destination UL corner in logical units
    \param  cDst       Destination W & H in logical units
    \param  Src        Source UL corner in logical units
    \param  cSrc       Source W & H in logical units
    \param  dwRop3     RasterOPeration Enumeration
    \param  Bm16        bitmap16 object (fields in it are all 0 if no bitmap is used)
    \param  px         pointer to bitmap in memory, or NULL if not used
*/
int U_WMRSTRETCHBLT_get(
      const char  *contents,
      U_POINT16 *  Dst,
      U_POINT16 *  cDst,
      U_POINT16 *  Src,
      U_POINT16 *  cSrc,
      uint32_t    *dwRop3,
      U_BITMAP16  *Bm16,
      const char **px
   ){
   uint8_t   xb;
   uint32_t  size = U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMRSTRETCHBLT_NOPX));
   if(!size)return(0);
   xb                = *(uint8_t *)( contents + offsetof(U_METARECORD, xb));
   if(U_TEST_NOPXB(size,xb)){ /* no bitmap */
      memcpy(dwRop3,               ( contents + offsetof(U_WMRSTRETCHBLT_NOPX, rop3w)), 4);
             cSrc->y = *(int16_t *)( contents + offsetof(U_WMRSTRETCHBLT_NOPX, hSrc     ));
             cSrc->x = *(int16_t *)( contents + offsetof(U_WMRSTRETCHBLT_NOPX, wSrc     ));
             Src->y  = *(int16_t *)( contents + offsetof(U_WMRSTRETCHBLT_NOPX, ySrc     ));
             Src->x  = *(int16_t *)( contents + offsetof(U_WMRSTRETCHBLT_NOPX, xSrc     ));
             cDst->y = *(int16_t *)( contents + offsetof(U_WMRSTRETCHBLT_NOPX, hDst     ));
             cDst->x = *(int16_t *)( contents + offsetof(U_WMRSTRETCHBLT_NOPX, wDst     ));
             Dst->y  = *(int16_t *)( contents + offsetof(U_WMRSTRETCHBLT_NOPX, yDst     ));
             Dst->x  = *(int16_t *)( contents + offsetof(U_WMRSTRETCHBLT_NOPX, xDst     ));
      memset(Bm16, 0, U_SIZE_BITMAP16);
             *px     = NULL;
   }
   else { /* yes bitmap */
      memcpy(dwRop3,               ( contents + offsetof(U_WMRSTRETCHBLT_PX, rop3w)), 4);
             cSrc->y = *(int16_t *)( contents + offsetof(U_WMRSTRETCHBLT_PX, hSrc     ));
             cSrc->x = *(int16_t *)( contents + offsetof(U_WMRSTRETCHBLT_PX, wSrc     ));
             Src->y  = *(int16_t *)( contents + offsetof(U_WMRSTRETCHBLT_PX, ySrc     ));
             Src->x  = *(int16_t *)( contents + offsetof(U_WMRSTRETCHBLT_PX, xSrc     ));
             cDst->y = *(int16_t *)( contents + offsetof(U_WMRSTRETCHBLT_PX, hDst     ));
             cDst->x = *(int16_t *)( contents + offsetof(U_WMRSTRETCHBLT_PX, wDst     ));
             Dst->y  = *(int16_t *)( contents + offsetof(U_WMRSTRETCHBLT_PX, yDst     ));
             Dst->x  = *(int16_t *)( contents + offsetof(U_WMRSTRETCHBLT_PX, xDst     ));
      memcpy(Bm16,                 ( contents + offsetof(U_WMRSTRETCHBLT_PX, bitmap)), U_SIZE_BITMAP16);
             *px     =             ( contents + offsetof(U_WMRSTRETCHBLT_PX, bitmap) + U_SIZE_BITMAP16);
   }
   return(size);
}

/**
    \brief Get data from a  U_WMRPOLYGON record.
    \return length of the U_WMRPOLYGON record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Length     Number of points in the Polygon
    \param  Data       pointer to array of U_POINT16 in memory.  Pointer may not be aligned properly for structures.
*/
int U_WMRPOLYGON_get(
      const char   *contents,
      uint16_t     *Length, 
      const char  **Data
    ){
    return U_WMRCORE_2U16_N16_get(contents, (U_SIZE_WMRPOLYGON), NULL, Length, Data);
}

/**
    \brief Get data from a  U_WMRPOLYLINE record.
    \return length of the U_WMRPOLYLINE record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Length     Number of points in the Polyline
    \param  Data       pointer to array of U_POINT16 in memory.  Pointer may not be aligned properly for structures.
*/
int U_WMRPOLYLINE_get(
      const char  *contents,
      uint16_t    *Length, 
      const char **Data
    ){
    return U_WMRCORE_2U16_N16_get(contents, (U_SIZE_WMRPOLYLINE), NULL, Length, Data);
}

/**
    \brief Get data from a  U_WMRESCAPE record.
    WARNING! Only three Escape record types are fully supported: SETLINECAP, SETLINEJOIN, SETMITERLIMIT.
    Even these should not be set here directly, instead use the wsetlinecap_get(), wsetlinejoin_get(), 
    or wsetmiterlimit_get() functions.
    Escape records created with this function, with the exception of the three named above, will not have
    the byte orders in Data adjusted automatically.  The user code must set Data to be little endian no
    matter what the endianness of the current platform where the user code is running.
    \return length of the U_WMRESCAPE record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Escape    Escape function
    \param  Length    Bytes in the Data
    \param  Data      Array of Length bytes
*/
int U_WMRESCAPE_get(
      const char  *contents,
      uint16_t    *Escape, 
      uint16_t    *Length, 
      const char **Data
   ){
   return U_WMRCORE_2U16_N16_get(contents, (U_SIZE_WMRESCAPE), Escape, Length, Data);
}

/**
    \brief Get data from a  U_WMRRESTOREDC record
    \return length of the U_WMRRESTOREDC record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  DC         DC to restore (relative if negative, absolute if positive)
*/
int U_WMRRESTOREDC_get(
      const char *contents,
      int16_t    *DC
   ){
   return U_WMRCORE_1U16_get(contents, (U_SIZE_WMRRESTOREDC), (uint16_t *)DC); // signed, but it is just a memcpy, so this works
}

/**
    \brief Get data from a  U_WMRFILLREGION record.
    \return length of the U_WMRFILLREGION record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Region    Region to fill
    \param  Brush     Brush to fill with
*/
int U_WMRFILLREGION_get(
      const char *contents,
      uint16_t   *Region, 
      uint16_t   *Brush
   ){
   return U_WMRCORE_2U16_get(contents, (U_SIZE_WMRFILLREGION), Region, Brush);
}

/**
    \brief Get data from a  U_WMRFRAMEREGION record.
    \return length of the U_WMRFRAMEREGION record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Region  Index of region to frame in object table
    \param  Brush   Index of brush to use in frame in object table
    \param  Height  in logical units (of frame)
    \param  Width   in logical units (of frame)
*/
int U_WMRFRAMEREGION_get(
      const char *contents,
      uint16_t   *Region,
      uint16_t   *Brush,
      int16_t    *Height,
      int16_t    *Width
   ){
   return U_WMRCORE_4U16_get(contents, (U_SIZE_WMRFRAMEREGION), Region, Brush, U_PP16(Height), U_PP16(Width));
}

/**
    \brief Get data from a  U_WMRINVERTREGION record.
    \return length of the U_WMRINVERTREGION record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Region  Index of region to invert.
*/
int U_WMRINVERTREGION_get(
      const char *contents,
      uint16_t   *Region
   ){
   return U_WMRCORE_1U16_get(contents, (U_SIZE_WMRINVERTREGION), Region);
}

/**
    \brief Get data from a  U_WMRPAINTREGION record.
    \return length of the U_WMRPAINTREGION record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Region  Index of region to paint with the current Brush.
*/
int U_WMRPAINTREGION_get(
      const char *contents,
      uint16_t   *Region
   ){
   return U_WMRCORE_1U16_get(contents, (U_SIZE_WMRPAINTREGION), Region);
}

/**
    \brief Get data from a  U_WMRSELECTCLIPREGION record.
    \return length of the U_WMRSELECTCLIPREGION record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Region  Index of region to become clipping region..
*/
int U_WMRSELECTCLIPREGION_get(
      const char *contents, 
      uint16_t   *Region
   ){
   return U_WMRCORE_1U16_get(contents, (U_SIZE_WMRSELECTCLIPREGION), Region);

}

/**
    \brief Get data from a  U_WMRSELECTOBJECT record.
    \return length of the U_WMRSELECTOBJECT record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Object  Index of object which is made active.
*/
int U_WMRSELECTOBJECT_get(
      const char *contents,
      uint16_t   *Object
   ){
   return U_WMRCORE_1U16_get(contents, (U_SIZE_WMRSELECTOBJECT), Object);
}

/**
    \brief Get data from a  U_WMRSETTEXTALIGN record.
    \return length of the U_WMRSETTEXTALIGN record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Mode  TextAlignment Enumeration.
*/
int U_WMRSETTEXTALIGN_get(
      const char *contents,
      uint16_t   *Mode
   ){
   return U_WMRCORE_1U16_get(contents, (U_SIZE_WMRSETTEXTALIGN), Mode);
}

/** in GDI and Wine, not in WMF manual.
*/
int U_WMRDRAWTEXT_get(void){  /* in Wine, not in WMF PDF. */
   return U_WMRCORENONE_get("U_WMRDRAWTEXT");
}

/**
    \brief Retrieve values from a U_WMRCHORD record
    \return length of the U_WMRCHORD record, or NULL on error
    \param  contents   record to extract data from
    \param  Radial1    Start of Chord
    \param  Radial2    End of Chord
    \param  rect       Bounding rectangle.
*/
int U_WMRCHORD_get(
      const char *contents,
      U_POINT16  *Radial1, 
      U_POINT16  *Radial2, 
      U_RECT16   *rect
   ){
   return U_WMRCORE_8U16_get(
      contents,
      (U_SIZE_WMRCHORD),
      U_P16(Radial2->y), 
      U_P16(Radial2->x),
      U_P16(Radial1->y),
      U_P16(Radial1->x),
      U_P16(rect->bottom), 
      U_P16(rect->right),
      U_P16(rect->top),
      U_P16(rect->left)
    );
}

/**
    \brief Get data from a  U_WMRSETMAPPERFLAGS record.
    \return length of the U_WMRSETMAPPERFLAGS record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Mode  If 1 bit set font mapper selects only matching aspect fonts.
*/
int U_WMRSETMAPPERFLAGS_get(
      const char *contents, 
      uint32_t   *Mode
   ){
   int size = U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMRSETMAPPERFLAGS));
   if(!size)return(0);
   memcpy(Mode, contents + U_SIZE_METARECORD, 4);
   return(size);
}

/**
    \brief Get data from a  U_WMREXTTEXTOUT record.
    \return length of the U_WMREXTTEXTOUT record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Dst        {X,Y} coordinates where the string is to be written.
    \param  Length     Stringlength in bytes
    \param  Opts       ExtTextOutOptions Flags
    \param  string     String to write (Latin1 encoding)
    \param  dx         Kerning information.  Must have same number of entries as Length.
    \param  rect       Used when when U_ETO_OPAQUE or U_ETO_CLIPPED bits are set in Opts
*/
int U_WMREXTTEXTOUT_get(
      const char      *contents, 
      U_POINT16 *      Dst,
      int16_t         *Length,
      uint16_t        *Opts, 
      const char     **string, 
      const int16_t  **dx, 
      U_RECT16        *rect
   ){
   int  size = U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMREXTTEXTOUT));
   int  off  = U_SIZE_METARECORD;
   if(!size)return(0);
   Dst->y   = *(int16_t *)( contents + offsetof(U_WMREXTTEXTOUT, y      ));
   Dst->x   = *(int16_t *)( contents + offsetof(U_WMREXTTEXTOUT, x      ));
   *Length  = *(int16_t *)( contents + offsetof(U_WMREXTTEXTOUT, Length ));
   *Opts    = *(uint16_t *)(contents + offsetof(U_WMREXTTEXTOUT, Opts   ));
   off      = U_SIZE_WMREXTTEXTOUT;
   if(*Opts & (U_ETO_OPAQUE | U_ETO_CLIPPED)){   memcpy(rect, (contents + off), U_SIZE_RECT16); off += U_SIZE_RECT16; }
   else {                                        memset(rect, 0,                U_SIZE_RECT16);                          }
   *string = (contents + off);
   off  += 2*((*Length +1)/2);
   if(*Length){    *dx = (int16_t *)(contents  + off); }
   else {          *dx = NULL;                         }
   return(size);
}

/**
    \brief Get data from a  U_WMRSETDIBTODEV record
    \return length of the U_WMRSETDIBTODEV record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Dst        UL corner of Dst rect in logical units
    \param  cwh        Width and Height in logical units
    \param  Src        UL corner of Src rect in logical units
    \param  cUsage     ColorUsage enumeration
    \param  ScanCount  Number of scan lines in Src
    \param  StartScan  First Scan line in Src
    \param  dib        DeviceIndependentBitmap object
*/
int U_WMRSETDIBTODEV_get(
      const char  *contents, 
      U_POINT16 *  Dst,
      U_POINT16 *  cwh,
      U_POINT16 *  Src,
      uint16_t    *cUsage,
      uint16_t    *ScanCount,
      uint16_t    *StartScan,
      const char **dib
   ){
   int  size = U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMRSETDIBTODEV));
   if(!size)return(0);
   *cUsage     = *(uint16_t *)(contents + offsetof(U_WMRSETDIBTODEV, cUsage    ));
   *ScanCount  = *(uint16_t *)(contents + offsetof(U_WMRSETDIBTODEV, ScanCount ));
   *StartScan  = *(uint16_t *)(contents + offsetof(U_WMRSETDIBTODEV, StartScan ));
   Src->y      = *(int16_t *)( contents + offsetof(U_WMRSETDIBTODEV, ySrc      ));
   Src->x      = *(int16_t *)( contents + offsetof(U_WMRSETDIBTODEV, xSrc      ));
   cwh->y      = *(int16_t *)( contents + offsetof(U_WMRSETDIBTODEV, Height    ));
   cwh->x      = *(int16_t *)( contents + offsetof(U_WMRSETDIBTODEV, Width     ));
   Dst->y      = *(int16_t *)( contents + offsetof(U_WMRSETDIBTODEV, yDst      ));
   Dst->x      = *(int16_t *)( contents + offsetof(U_WMRSETDIBTODEV, xDst      ));
   *dib        =             ( contents + offsetof(U_WMRSETDIBTODEV, dib       ));
   return(size);
}

/**
    \brief Get data from a  U_WMRSELECTPALETTE record
    \return length of the U_WMRSELECTPALETTE record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Palette  Index of Palette to make active.
*/
int U_WMRSELECTPALETTE_get(
      const char  *contents, 
      uint16_t    *Palette
   ){
   return U_WMRCORE_1U16_get(contents, (U_SIZE_WMRSELECTPALETTE), Palette);
}

/**
    \brief Get data from a  U_WMRREALIZEPALETTE record
    \return length of the U_WMRREALIZEPALETTE record in bytes, or 0 on error
    \param  contents   record to extract data from
*/
int U_WMRREALIZEPALETTE_get(
      const char  *contents
   ){
   return U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMRREALIZEPALETTE));
}

/**
    \brief Get data from a  U_WMRSETPALENTRIES record
    \return length of the U_WMRSETPALENTRIES record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Palette    Redefines a set of RGB values for the current active Palette.
    \param  PalEntries Array of Palette Entries
*/
int U_WMRANIMATEPALETTE_get(
      const char  *contents,
      U_PALETTE   *Palette,
      const char **PalEntries 
   ){
   return U_WMRCORE_PALETTE_get(contents, (U_SIZE_WMRANIMATEPALETTE), Palette, PalEntries);
}

/**
    \brief Get data from a  U_WMRSETPALENTRIES record
    \return length of the U_WMRSETPALENTRIES record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Palette  Defines a set of RGB values for the current active Palette.
    \param  PalEntries Array of Palette Entries
*/
int U_WMRSETPALENTRIES_get(
      const char  *contents, 
      U_PALETTE   *Palette,
      const char **PalEntries 
   ){
   return U_WMRCORE_PALETTE_get(contents, (U_SIZE_WMRSETPALENTRIES), Palette, PalEntries);
}

/**
    \brief Get data from a  U_WMR_POLYPOLYGON record.
    \return length of the U_WMR_POLYPOLYGON record in bytes, or 0 on error
    \param  contents     record to extract data from
    \param  nPolys       Number of elements in aPolyCounts
    \param  aPolyCounts  Number of points in each poly (sequential)
    \param  Points       pointer to array of U_POINT16 in memory.  Probably not aligned.
*/
int U_WMRPOLYPOLYGON_get(
      const char        *contents, 
      uint16_t          *nPolys,
      const uint16_t   **aPolyCounts,
      const char       **Points
   ){
   int        size = U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMRPOLYPOLYGON));
   if(!size)return(0);
   contents +=                             offsetof(U_WMRPOLYPOLYGON, PPolygon);
   memcpy(nPolys,               contents + offsetof(U_POLYPOLYGON, nPolys), 2);
   *aPolyCounts =  (uint16_t *)(contents + offsetof(U_POLYPOLYGON, aPolyCounts));
   *Points = (contents + offsetof(U_POLYPOLYGON, aPolyCounts) + *nPolys*2);
   return(size);
}

/**
    \brief Get data from a  U_WMRRESIZEPALETTE record
    \return length of the U_WMRRESIZEPALETTE record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Palette  Changes the size of the currently active Palette.
*/
int U_WMRRESIZEPALETTE_get(
      const char  *contents, 
      uint16_t    *Palette
   ){
   return U_WMRCORE_1U16_get(contents, (U_SIZE_WMRRESIZEPALETTE), Palette);
}

//! \cond
int U_WMR3A_get(void){
   return U_WMRCORENONE_get("U_WMR3A");
}

int U_WMR3B_get(void){
   return U_WMRCORENONE_get("U_WMR3B");
}

int U_WMR3C_get(void){
   return U_WMRCORENONE_get("U_WMR3C");
}

int U_WMR3D_get(void){
   return U_WMRCORENONE_get("U_WMR3D");
}

int U_WMR3E_get(void){
   return U_WMRCORENONE_get("U_WMR3E");
}

int U_WMR3F_get(void){
   return U_WMRCORENONE_get("U_WMR3F");
}
//! \endcond

// U_WMRDIBBITBLT_get
/**
    \brief Get data from a  U_WMRDIBITBLT record.
    \return length of the U_WMRDIBITBLT record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Dst       Destination UL corner in logical units
    \param  Src       Source UL corner in logical units
    \param  cwh       W & H in logical units of Src and Dst
    \param  dwRop3    RasterOPeration Enumeration
    \param  dib       pointer to dib in WMF in memory.  Most likely not aligned.
*/
int U_WMRDIBBITBLT_get(
      const char  *contents, 
      U_POINT16 *  Dst,
      U_POINT16 *  cwh,
      U_POINT16 *  Src,
      uint32_t    *dwRop3,
      const char **dib
   ){
   uint8_t  xb;
   uint32_t size = U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMRDIBBITBLT_NOPX));
   if(!size)return(0);
   xb                    = *(uint8_t *)( contents + offsetof(U_METARECORD, xb));
   if(U_TEST_NOPXB(size,xb)){ /* no bitmap */
      memcpy(dwRop3,                   ( contents + offsetof(U_WMRDIBBITBLT_NOPX, rop3w)), 4);
              Src->y     = *(int16_t *)( contents + offsetof(U_WMRDIBBITBLT_NOPX, ySrc      ));
              Src->x     = *(int16_t *)( contents + offsetof(U_WMRDIBBITBLT_NOPX, xSrc      ));
              cwh->y     = *(int16_t *)( contents + offsetof(U_WMRDIBBITBLT_NOPX, Height    ));
              cwh->x     = *(int16_t *)( contents + offsetof(U_WMRDIBBITBLT_NOPX, Width     ));
              Dst->y     = *(int16_t *)( contents + offsetof(U_WMRDIBBITBLT_NOPX, yDst      ));
              Dst->x     = *(int16_t *)( contents + offsetof(U_WMRDIBBITBLT_NOPX, xDst      ));
              *dib       = NULL;
   }
   else { /* yes bitmap */
      memcpy(dwRop3,                   ( contents + offsetof(U_WMRDIBBITBLT_PX, rop3w)), 4);
              Src->y     = *(int16_t *)( contents + offsetof(U_WMRDIBBITBLT_PX, ySrc      ));
              Src->x     = *(int16_t *)( contents + offsetof(U_WMRDIBBITBLT_PX, xSrc      ));
              cwh->y     = *(int16_t *)( contents + offsetof(U_WMRDIBBITBLT_PX, Height    ));
              cwh->x     = *(int16_t *)( contents + offsetof(U_WMRDIBBITBLT_PX, Width     ));
              Dst->y     = *(int16_t *)( contents + offsetof(U_WMRDIBBITBLT_PX, yDst      ));
              Dst->x     = *(int16_t *)( contents + offsetof(U_WMRDIBBITBLT_PX, xDst      ));
              *dib       =             ( contents + offsetof(U_WMRDIBBITBLT_PX, dib       ));
   }
   return(size);
}

/**
    \brief Get data from a  U_WMRSTRETCHDIB record.
    \return length of the U_WMRSTRETCHDIB record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Dst       Destination UL corner in logical units
    \param  cDst      Destination W & H in logical units
    \param  Src       Source UL corner in logical units
    \param  cSrc      Source W & H in logical units
    \param  dwRop3    RasterOPeration Enumeration
    \param  dib       pointer to dib in WMF in memory.  Most likely not aligned.
*/
int U_WMRDIBSTRETCHBLT_get(
      const char  *contents, 
      U_POINT16 *  Dst,
      U_POINT16 *  cDst,
      U_POINT16 *  Src,
      U_POINT16 *  cSrc,
      uint32_t    *dwRop3,
      const char **dib
   ){
   uint8_t  xb;
   uint32_t size = U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMRDIBSTRETCHBLT_NOPX));
   if(!size)return(0);
   xb                    = *(uint8_t *)( contents + offsetof(U_METARECORD, xb));
   if(U_TEST_NOPXB(size,xb)){ /* no bitmap */
      memcpy(dwRop3 ,                  ( contents + offsetof(U_WMRDIBSTRETCHBLT_NOPX, rop3w)), 4);
              Src->y     = *(int16_t *)( contents + offsetof(U_WMRDIBSTRETCHBLT_NOPX, ySrc      ));
              Src->x     = *(int16_t *)( contents + offsetof(U_WMRDIBSTRETCHBLT_NOPX, xSrc      ));
              cSrc->y    = *(int16_t *)( contents + offsetof(U_WMRDIBSTRETCHBLT_NOPX, hSrc      ));
              cSrc->x    = *(int16_t *)( contents + offsetof(U_WMRDIBSTRETCHBLT_NOPX, wSrc      ));
              Dst->y     = *(int16_t *)( contents + offsetof(U_WMRDIBSTRETCHBLT_NOPX, yDst      ));
              Dst->x     = *(int16_t *)( contents + offsetof(U_WMRDIBSTRETCHBLT_NOPX, xDst      ));
              cDst->y    = *(int16_t *)( contents + offsetof(U_WMRDIBSTRETCHBLT_NOPX, hDst      ));
              cDst->x    = *(int16_t *)( contents + offsetof(U_WMRDIBSTRETCHBLT_NOPX, wDst      ));
              *dib       = NULL;
   }
   else { /* yes bitmap */
      memcpy(dwRop3 ,                  ( contents + offsetof(U_WMRDIBSTRETCHBLT_PX, rop3w)), 4);
              Src->y     = *(int16_t *)( contents + offsetof(U_WMRDIBSTRETCHBLT_PX, ySrc      ));
              Src->x     = *(int16_t *)( contents + offsetof(U_WMRDIBSTRETCHBLT_PX, xSrc      ));
              cSrc->y    = *(int16_t *)( contents + offsetof(U_WMRDIBSTRETCHBLT_PX, hSrc      ));
              cSrc->x    = *(int16_t *)( contents + offsetof(U_WMRDIBSTRETCHBLT_PX, wSrc      ));
              Dst->y     = *(int16_t *)( contents + offsetof(U_WMRDIBSTRETCHBLT_PX, yDst      ));
              Dst->x     = *(int16_t *)( contents + offsetof(U_WMRDIBSTRETCHBLT_PX, xDst      ));
              cDst->y    = *(int16_t *)( contents + offsetof(U_WMRDIBSTRETCHBLT_PX, hDst      ));
              cDst->x    = *(int16_t *)( contents + offsetof(U_WMRDIBSTRETCHBLT_PX, wDst      ));
              *dib       =             ( contents + offsetof(U_WMRDIBSTRETCHBLT_PX, dib       ));
   }
   return(size);
}

/**
    \brief Get data from a  U_WMRDIBCREATEPATTERNBRUSH record.
      Returns an image as either a DIB (Bmi/CbPx/Px defined) or a Bitmap16 (Bm16 defined).
      WARNING - U_WMRCREATEPATTERNBRUSH has been declared obsolete and application support is spotty -
      this function is still valid though, for those instances where old WMF input files are encountered.
    \return length of the U_WMRDIBCREATEPATTERNBRUSH record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Style      BrushStyle Enumeration                                                                                              
    \param  cUsage     DIBcolors Enumeration                                                                                               
    \param  Bm16       pointer to a U_BITMAP16 in WMF in memory.  Most likely not aligned.  NULL if dib  is used instead.                                                           
    \param  dib        pointer to a dib        in WMF in memory.  Most likely not aligned.  NULL if Bm16 is used instead.
 */
int U_WMRDIBCREATEPATTERNBRUSH_get(
      const char  *contents, 
      uint16_t    *Style, 
      uint16_t    *cUsage,
      const char **Bm16,
      const char **dib
   ){
   int  size = U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMRDIBCREATEPATTERNBRUSH));
   if(!size)return(0);

   *Style   = *(uint16_t *)(contents + offsetof(U_WMRDIBCREATEPATTERNBRUSH, Style  ));
   *cUsage  = *(uint16_t *)(contents + offsetof(U_WMRDIBCREATEPATTERNBRUSH, cUsage ));
   if(*Style == U_BS_PATTERN){
      *Bm16 = (contents + offsetof(U_WMRDIBCREATEPATTERNBRUSH, Src));
      *dib  = NULL;
      /* The WMF spec says that Style == U_BS_PATTERN  _SHOULD_ be a bitmap16.  
         However there are instances when it is actually a DIB. U_WMRDIBCREATEPATTERNBRUSH_get
         tries to detect this by looking for bogus values when the BM16 is interpreted as such,
         and if it finds them, then it returns a dib instead. 
      */
      U_BITMAP16 TmpBm16;
      memcpy(&TmpBm16, *Bm16, U_SIZE_BITMAP16);
      if(TmpBm16.Width  <= 0 || TmpBm16.Height <= 0 || TmpBm16.Planes != 1 || TmpBm16.BitsPixel == 0){
         *Bm16 = NULL;
         *dib  = (contents + offsetof(U_WMRDIBCREATEPATTERNBRUSH, Src));
      }
   }
   else { /* from DIB */
      *Bm16 = NULL;
      *dib  = (contents + offsetof(U_WMRDIBCREATEPATTERNBRUSH, Src));
   }
   return(size);
}

/**
    \brief Get data from a  U_WMRSTRETCHDIB record.
    \return length of the U_WMRSTRETCHDIB record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Dst       Destination UL corner in logical units
    \param  cDst      Destination W & H in logical units
    \param  Src       Source UL corner in logical units
    \param  cSrc      Source W & H in logical units
    \param  cUsage    DIBColors Enumeration
    \param  dwRop3    RasterOPeration Enumeration
    \param  dib       (Optional) device independent bitmap
*/
int U_WMRSTRETCHDIB_get(
      const char  *contents, 
      U_POINT16 *  Dst,
      U_POINT16 *  cDst,
      U_POINT16 *  Src,
      U_POINT16 *  cSrc,
      uint16_t    *cUsage,
      uint32_t    *dwRop3,
      const char **dib
   ){
   int  size = U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMRSTRETCHDIB));
   if(!size)return(0);

   memcpy(dwRop3,            ( contents + offsetof(U_WMRSTRETCHDIB, rop3w)), 4);
   *cUsage    = *(uint16_t *)( contents + offsetof(U_WMRSTRETCHDIB, cUsage    ));
   cSrc->y    = *(int16_t *)(  contents + offsetof(U_WMRSTRETCHDIB, hSrc      ));
   cSrc->x    = *(int16_t *)(  contents + offsetof(U_WMRSTRETCHDIB, wSrc      )); 
   Src->y     = *(int16_t *)(  contents + offsetof(U_WMRSTRETCHDIB, ySrc      ));  
   Src->x     = *(int16_t *)(  contents + offsetof(U_WMRSTRETCHDIB, xSrc      ));  
   cDst->y    = *(int16_t *)(  contents + offsetof(U_WMRSTRETCHDIB, hDst      ));
   cDst->x    = *(int16_t *)(  contents + offsetof(U_WMRSTRETCHDIB, wDst      )); 
   Dst->y     = *(int16_t *)(  contents + offsetof(U_WMRSTRETCHDIB, yDst      ));  
   Dst->x     = *(int16_t *)(  contents + offsetof(U_WMRSTRETCHDIB, xDst      ));
   *dib       =             (  contents + offsetof(U_WMRSTRETCHDIB, dib       ));
   return(size);
}

//! \cond
int U_WMR44_get(void){
   return U_WMRCORENONE_get("U_WMR44");
}

int U_WMR45_get(void){
   return U_WMRCORENONE_get("U_WMR45");
}

int U_WMR46_get(void){
   return U_WMRCORENONE_get("U_WMR46");
}

int U_WMR47_get(void){
   return U_WMRCORENONE_get("U_WMR47");
}
//! \endcond

/**
    \brief Retrieve values from a U_WMREXTFLOODFILL record
    \return length of the U_WMREXTFLOODFILL record, or NULL on error
    \param  contents   record to extract data from
    \param  Mode       FloodFill Enumeration.
    \param  Color      Color to Fill with.
    \param  coord      Location to start fill.
*/
int U_WMREXTFLOODFILL_get(
      const char  *contents, 
      uint16_t    *Mode, 
      U_COLORREF  *Color, 
      U_POINT16 *  coord
   ){
   return  U_WMRCORE_1U16_CRF_2U16_get(
      contents,
      Mode,
      Color,
      U_P16(coord->y),
      U_P16(coord->x)
   );
}

//! \cond
int U_WMR49_get(void){
   return U_WMRCORENONE_get("U_WMR49");
}

int U_WMR4A_get(void){
   return U_WMRCORENONE_get("U_WMR4A");
}

int U_WMR4B_get(void){
   return U_WMRCORENONE_get("U_WMR4B");
}

int U_WMR4C_get(void){
   return U_WMRCORENONE_get("U_WMRRESETDOC");
}

int U_WMR4D_get(void){
   return U_WMRCORENONE_get("U_WMRSTARTDOC");
}

int U_WMR4E_get(void){
   return U_WMRCORENONE_get("U_WMR4E");
}

int U_WMR4F_get(void){
   return U_WMRCORENONE_get("U_WMRSTARTPAGE");
}

int U_WMR50_get(void){
   return U_WMRCORENONE_get("U_WMRENDPAGE");
}

int U_WMR51_get(void){
   return U_WMRCORENONE_get("U_WMR51");
}

int U_WMRABORTDOC_get(void){
   return U_WMRCORENONE_get("U_WMRABORTDOC");
}

int U_WMR53_get(void){
   return U_WMRCORENONE_get("U_WMR53");
}

int U_WMR54_get(void){
   return U_WMRCORENONE_get("U_WMR54");
}

int U_WMR55_get(void){
   return U_WMRCORENONE_get("U_WMR55");
}

int U_WMR56_get(void){
   return U_WMRCORENONE_get("U_WMR56");
}

int U_WMR57_get(void){
   return U_WMRCORENONE_get("U_WMR57");
}

int U_WMR58_get(void){
   return U_WMRCORENONE_get("U_WMR58");
}

int U_WMR59_get(void){
   return U_WMRCORENONE_get("U_WMR59");
}

int U_WMR5A_get(void){
   return U_WMRCORENONE_get("U_WMR5A");
}

int U_WMR5B_get(void){
   return U_WMRCORENONE_get("U_WMR5B");
}

int U_WMR5C_get(void){
   return U_WMRCORENONE_get("U_WMR5C");
}

int U_WMR5D_get(void){
   return U_WMRCORENONE_get("U_WMR5D");
}

int U_WMR5E_get(void){
   return U_WMRCORENONE_get("U_WMRENDDOC");
}

int U_WMR5F_get(void){
   return U_WMRCORENONE_get("U_WMR5F");
}

int U_WMR60_get(void){
   return U_WMRCORENONE_get("U_WMR60");
}

int U_WMR61_get(void){
   return U_WMRCORENONE_get("U_WMR61");
}

int U_WMR62_get(void){
   return U_WMRCORENONE_get("U_WMR62");
}

int U_WMR63_get(void){
   return U_WMRCORENONE_get("U_WMR63");
}

int U_WMR64_get(void){
   return U_WMRCORENONE_get("U_WMR64");
}

int U_WMR65_get(void){
   return U_WMRCORENONE_get("U_WMR65");
}

int U_WMR66_get(void){
   return U_WMRCORENONE_get("U_WMR66");
}

int U_WMR67_get(void){
   return U_WMRCORENONE_get("U_WMR67");
}

int U_WMR68_get(void){
   return U_WMRCORENONE_get("U_WMR68");
}

int U_WMR69_get(void){
   return U_WMRCORENONE_get("U_WMR69");
}

int U_WMR6A_get(void){
   return U_WMRCORENONE_get("U_WMR6A");
}

int U_WMR6B_get(void){
   return U_WMRCORENONE_get("U_WMR6B");
}

int U_WMR6C_get(void){
   return U_WMRCORENONE_get("U_WMR6C");
}

int U_WMR6D_get(void){
   return U_WMRCORENONE_get("U_WMR6D");
}

int U_WMR6E_get(void){
   return U_WMRCORENONE_get("U_WMR6E");
}

int U_WMR6F_get(void){
   return U_WMRCORENONE_get("U_WMR6F");
}

int U_WMR70_get(void){
   return U_WMRCORENONE_get("U_WMR70");
}

int U_WMR71_get(void){
   return U_WMRCORENONE_get("U_WMR71");
}

int U_WMR72_get(void){
   return U_WMRCORENONE_get("U_WMR72");
}

int U_WMR73_get(void){
   return U_WMRCORENONE_get("U_WMR73");
}

int U_WMR74_get(void){
   return U_WMRCORENONE_get("U_WMR74");
}

int U_WMR75_get(void){
   return U_WMRCORENONE_get("U_WMR75");
}

int U_WMR76_get(void){
   return U_WMRCORENONE_get("U_WMR76");
}

int U_WMR77_get(void){
   return U_WMRCORENONE_get("U_WMR77");
}

int U_WMR78_get(void){
   return U_WMRCORENONE_get("U_WMR78");
}

int U_WMR79_get(void){
   return U_WMRCORENONE_get("U_WMR79");
}

int U_WMR7A_get(void){
   return U_WMRCORENONE_get("U_WMR7A");
}

int U_WMR7B_get(void){
   return U_WMRCORENONE_get("U_WMR7B");
}

int U_WMR7C_get(void){
   return U_WMRCORENONE_get("U_WMR7C");
}

int U_WMR7D_get(void){
   return U_WMRCORENONE_get("U_WMR7D");
}

int U_WMR7E_get(void){
   return U_WMRCORENONE_get("U_WMR7E");
}

int U_WMR7F_get(void){
   return U_WMRCORENONE_get("U_WMR7F");
}

int U_WMR80_get(void){
   return U_WMRCORENONE_get("U_WMR80");
}

int U_WMR81_get(void){
   return U_WMRCORENONE_get("U_WMR81");
}

int U_WMR82_get(void){
   return U_WMRCORENONE_get("U_WMR82");
}

int U_WMR83_get(void){
   return U_WMRCORENONE_get("U_WMR83");
}

int U_WMR84_get(void){
   return U_WMRCORENONE_get("U_WMR84");
}

int U_WMR85_get(void){
   return U_WMRCORENONE_get("U_WMR85");
}

int U_WMR86_get(void){
   return U_WMRCORENONE_get("U_WMR86");
}

int U_WMR87_get(void){
   return U_WMRCORENONE_get("U_WMR87");
}

int U_WMR88_get(void){
   return U_WMRCORENONE_get("U_WMR88");
}

int U_WMR89_get(void){
   return U_WMRCORENONE_get("U_WMR89");
}

int U_WMR8A_get(void){
   return U_WMRCORENONE_get("U_WMR8A");
}

int U_WMR8B_get(void){
   return U_WMRCORENONE_get("U_WMR8B");
}

int U_WMR8C_get(void){
   return U_WMRCORENONE_get("U_WMR8C");
}

int U_WMR8D_get(void){
   return U_WMRCORENONE_get("U_WMR8D");
}

int U_WMR8E_get(void){
   return U_WMRCORENONE_get("U_WMR8E");
}

int U_WMR8F_get(void){
   return U_WMRCORENONE_get("U_WMR8F");
}

int U_WMR90_get(void){
   return U_WMRCORENONE_get("U_WMR90");
}

int U_WMR91_get(void){
   return U_WMRCORENONE_get("U_WMR91");
}

int U_WMR92_get(void){
   return U_WMRCORENONE_get("U_WMR92");
}

int U_WMR93_get(void){
   return U_WMRCORENONE_get("U_WMR93");
}

int U_WMR94_get(void){
   return U_WMRCORENONE_get("U_WMR94");
}

int U_WMR95_get(void){
   return U_WMRCORENONE_get("U_WMR95");
}

int U_WMR96_get(void){
   return U_WMRCORENONE_get("U_WMR96");
}

int U_WMR97_get(void){
   return U_WMRCORENONE_get("U_WMR97");
}

int U_WMR98_get(void){
   return U_WMRCORENONE_get("U_WMR98");
}

int U_WMR99_get(void){
   return U_WMRCORENONE_get("U_WMR99");
}

int U_WMR9A_get(void){
   return U_WMRCORENONE_get("U_WMR9A");
}

int U_WMR9B_get(void){
   return U_WMRCORENONE_get("U_WMR9B");
}

int U_WMR9C_get(void){
   return U_WMRCORENONE_get("U_WMR9C");
}

int U_WMR9D_get(void){
   return U_WMRCORENONE_get("U_WMR9D");
}

int U_WMR9E_get(void){
   return U_WMRCORENONE_get("U_WMR9E");
}

int U_WMR9F_get(void){
   return U_WMRCORENONE_get("U_WMR9F");
}

int U_WMRA0_get(void){
   return U_WMRCORENONE_get("U_WMRA0");
}

int U_WMRA1_get(void){
   return U_WMRCORENONE_get("U_WMRA1");
}

int U_WMRA2_get(void){
   return U_WMRCORENONE_get("U_WMRA2");
}

int U_WMRA3_get(void){
   return U_WMRCORENONE_get("U_WMRA3");
}

int U_WMRA4_get(void){
   return U_WMRCORENONE_get("U_WMRA4");
}

int U_WMRA5_get(void){
   return U_WMRCORENONE_get("U_WMRA5");
}

int U_WMRA6_get(void){
   return U_WMRCORENONE_get("U_WMRA6");
}

int U_WMRA7_get(void){
   return U_WMRCORENONE_get("U_WMRA7");
}

int U_WMRA8_get(void){
   return U_WMRCORENONE_get("U_WMRA8");
}

int U_WMRA9_get(void){
   return U_WMRCORENONE_get("U_WMRA9");
}

int U_WMRAA_get(void){
   return U_WMRCORENONE_get("U_WMRAA");
}

int U_WMRAB_get(void){
   return U_WMRCORENONE_get("U_WMRAB");
}

int U_WMRAC_get(void){
   return U_WMRCORENONE_get("U_WMRAC");
}

int U_WMRAD_get(void){
   return U_WMRCORENONE_get("U_WMRAD");
}

int U_WMRAE_get(void){
   return U_WMRCORENONE_get("U_WMRAE");
}

int U_WMRAF_get(void){
   return U_WMRCORENONE_get("U_WMRAF");
}

int U_WMRB0_get(void){
   return U_WMRCORENONE_get("U_WMRB0");
}

int U_WMRB1_get(void){
   return U_WMRCORENONE_get("U_WMRB1");
}

int U_WMRB2_get(void){
   return U_WMRCORENONE_get("U_WMRB2");
}

int U_WMRB3_get(void){
   return U_WMRCORENONE_get("U_WMRB3");
}

int U_WMRB4_get(void){
   return U_WMRCORENONE_get("U_WMRB4");
}

int U_WMRB5_get(void){
   return U_WMRCORENONE_get("U_WMRB5");
}

int U_WMRB6_get(void){
   return U_WMRCORENONE_get("U_WMRB6");
}

int U_WMRB7_get(void){
   return U_WMRCORENONE_get("U_WMRB7");
}

int U_WMRB8_get(void){
   return U_WMRCORENONE_get("U_WMRB8");
}

int U_WMRB9_get(void){
   return U_WMRCORENONE_get("U_WMRB9");
}

int U_WMRBA_get(void){
   return U_WMRCORENONE_get("U_WMRBA");
}

int U_WMRBB_get(void){
   return U_WMRCORENONE_get("U_WMRBB");
}

int U_WMRBC_get(void){
   return U_WMRCORENONE_get("U_WMRBC");
}

int U_WMRBD_get(void){
   return U_WMRCORENONE_get("U_WMRBD");
}

int U_WMRBE_get(void){
   return U_WMRCORENONE_get("U_WMRBE");
}

int U_WMRBF_get(void){
   return U_WMRCORENONE_get("U_WMRBF");
}

int U_WMRC0_get(void){
   return U_WMRCORENONE_get("U_WMRC0");
}

int U_WMRC1_get(void){
   return U_WMRCORENONE_get("U_WMRC1");
}

int U_WMRC2_get(void){
   return U_WMRCORENONE_get("U_WMRC2");
}

int U_WMRC3_get(void){
   return U_WMRCORENONE_get("U_WMRC3");
}

int U_WMRC4_get(void){
   return U_WMRCORENONE_get("U_WMRC4");
}

int U_WMRC5_get(void){
   return U_WMRCORENONE_get("U_WMRC5");
}

int U_WMRC6_get(void){
   return U_WMRCORENONE_get("U_WMRC6");
}

int U_WMRC7_get(void){
   return U_WMRCORENONE_get("U_WMRC7");
}

int U_WMRC8_get(void){
   return U_WMRCORENONE_get("U_WMRC8");
}

int U_WMRC9_get(void){
   return U_WMRCORENONE_get("U_WMRC9");
}

int U_WMRCA_get(void){
   return U_WMRCORENONE_get("U_WMRCA");
}

int U_WMRCB_get(void){
   return U_WMRCORENONE_get("U_WMRCB");
}

int U_WMRCC_get(void){
   return U_WMRCORENONE_get("U_WMRCC");
}

int U_WMRCD_get(void){
   return U_WMRCORENONE_get("U_WMRCD");
}

int U_WMRCE_get(void){
   return U_WMRCORENONE_get("U_WMRCE");
}

int U_WMRCF_get(void){
   return U_WMRCORENONE_get("U_WMRCF");
}

int U_WMRD0_get(void){
   return U_WMRCORENONE_get("U_WMRD0");
}

int U_WMRD1_get(void){
   return U_WMRCORENONE_get("U_WMRD1");
}

int U_WMRD2_get(void){
   return U_WMRCORENONE_get("U_WMRD2");
}

int U_WMRD3_get(void){
   return U_WMRCORENONE_get("U_WMRD3");
}

int U_WMRD4_get(void){
   return U_WMRCORENONE_get("U_WMRD4");
}

int U_WMRD5_get(void){
   return U_WMRCORENONE_get("U_WMRD5");
}

int U_WMRD6_get(void){
   return U_WMRCORENONE_get("U_WMRD6");
}

int U_WMRD7_get(void){
   return U_WMRCORENONE_get("U_WMRD7");
}

int U_WMRD8_get(void){
   return U_WMRCORENONE_get("U_WMRD8");
}

int U_WMRD9_get(void){
   return U_WMRCORENONE_get("U_WMRD9");
}

int U_WMRDA_get(void){
   return U_WMRCORENONE_get("U_WMRDA");
}

int U_WMRDB_get(void){
   return U_WMRCORENONE_get("U_WMRDB");
}

int U_WMRDC_get(void){
   return U_WMRCORENONE_get("U_WMRDC");
}

int U_WMRDD_get(void){
   return U_WMRCORENONE_get("U_WMRDD");
}

int U_WMRDE_get(void){
   return U_WMRCORENONE_get("U_WMRDE");
}

int U_WMRDF_get(void){
   return U_WMRCORENONE_get("U_WMRDF");
}

int U_WMRE0_get(void){
   return U_WMRCORENONE_get("U_WMRE0");
}

int U_WMRE1_get(void){
   return U_WMRCORENONE_get("U_WMRE1");
}

int U_WMRE2_get(void){
   return U_WMRCORENONE_get("U_WMRE2");
}

int U_WMRE3_get(void){
   return U_WMRCORENONE_get("U_WMRE3");
}

int U_WMRE4_get(void){
   return U_WMRCORENONE_get("U_WMRE4");
}

int U_WMRE5_get(void){
   return U_WMRCORENONE_get("U_WMRE5");
}

int U_WMRE6_get(void){
   return U_WMRCORENONE_get("U_WMRE6");
}

int U_WMRE7_get(void){
   return U_WMRCORENONE_get("U_WMRE7");
}

int U_WMRE8_get(void){
   return U_WMRCORENONE_get("U_WMRE8");
}

int U_WMRE9_get(void){
   return U_WMRCORENONE_get("U_WMRE9");
}

int U_WMREA_get(void){
   return U_WMRCORENONE_get("U_WMREA");
}

int U_WMREB_get(void){
   return U_WMRCORENONE_get("U_WMREB");
}

int U_WMREC_get(void){
   return U_WMRCORENONE_get("U_WMREC");
}

int U_WMRED_get(void){
   return U_WMRCORENONE_get("U_WMRED");
}

int U_WMREE_get(void){
   return U_WMRCORENONE_get("U_WMREE");
}

int U_WMREF_get(void){
   return U_WMRCORENONE_get("U_WMREF");
}
//! \endcond

/**
    \brief Get data from a  U_WMRDELETEOBJECT record.
    \return length of the U_WMRDELETEOBJECT record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Object  Index of object which is made active.
*/
int U_WMRDELETEOBJECT_get(
      const char *contents,
      uint16_t   *Object
   ){
   return U_WMRCORE_1U16_get(contents, (U_SIZE_WMRDELETEOBJECT), Object);
}

//! \cond
int U_WMRF1_get(void){
   return U_WMRCORENONE_get("U_WMRF1");
}

int U_WMRF2_get(void){
   return U_WMRCORENONE_get("U_WMRF2");
}

int U_WMRF3_get(void){
   return U_WMRCORENONE_get("U_WMRF3");
}

int U_WMRF4_get(void){
   return U_WMRCORENONE_get("U_WMRF4");
}

int U_WMRF5_get(void){
   return U_WMRCORENONE_get("U_WMRF5");
}

int U_WMRF6_get(void){
   return U_WMRCORENONE_get("U_WMRF6");
}
//! \endcond

/**
    \brief Retrieve values from a U_WMRCREATEPALETTE record
    \return length of the U_WMRCREATEPALETTE record, or NULL on error
    \param  contents   record to extract data from
    \param  Palette    Create a Palette object.
    \param  PalEntries Array of Palette Entries
*/
int U_WMRCREATEPALETTE_get(
      const char  *contents,
      U_PALETTE   *Palette,
      const char **PalEntries 
   ){
   return U_WMRCORE_PALETTE_get(contents, (U_SIZE_WMRCREATEPALETTE), Palette, PalEntries);
}

//! \cond
int U_WMRF8_get(void){
   return U_WMRCORENONE_get("U_WMRF8");
}
//! \endcond

/**
    \brief Get data from a  U_WMRCREATEPATTERNBRUSH record.
    Warning - application support for U_WMRCREATEPATTERNBRUSH is spotty, better to use U_WMRDIBCREATEPATTERNBRUSH.
    \return length of the U_WMRCREATEPATTERNBRUSH record in bytes, or 0 on error
    \param  contents     record to extract data from
    \param  Bm16         truncated Bitmap16 structure from record, only tge first 14 bytes hold data.
    \param  pasize       Number of bytes in Pattern
    \param  Pattern      byte array pattern, described by Bm16, for brush
*/
int U_WMRCREATEPATTERNBRUSH_get(
      const char   *contents,
      U_BITMAP16   *Bm16,
      int          *pasize,
      const char  **Pattern
   ){
   int off = U_SIZE_METARECORD;
   /* size in next one is 
      6 (core header) + 14 (truncated bitmap16) + 18 bytes reserved + 2 bytes (at least) for data */
   int  size = U_WMRCORE_RECSAFE_get(contents, (U_SIZE_METARECORD + 14 + 18 + 2));
   if(!size)return(0);
   memset(Bm16, 0, U_SIZE_BITMAP16); 
   /* BM16 is truncated in this record type to 14 bytes, last 4 bytes must be ignored, so they are not even copied */
   memcpy(Bm16, contents + off, 10);  
   *pasize = (((Bm16->Width * Bm16->BitsPixel + 15) >> 4) << 1) * Bm16->Height;
   off += 32;  /* skip [14 bytes of truncated bitmap16 object and 18 bytes of reserved */
   *Pattern = (contents + off); 
   return(size);
}

/**
    \brief Get data from a  U_WMRCREATEPENINDIRECT record.
    \return length of the U_WMRCREATEPENINDIRECT record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  pen        pointer to a U_PEN object to fill.
*/
int U_WMRCREATEPENINDIRECT_get(
      const char   *contents,
      U_PEN        *pen
   ){
   int  size = U_WMRCORE_RECSAFE_get(contents, (U_SIZE_WMRCREATEPENINDIRECT));
   if(!size)return(0);
   memcpy(pen, contents + offsetof(U_WMRCREATEPENINDIRECT, pen), U_SIZE_PEN);
   return(size);
}

/**
    \brief Get data from a  U_WMRCREATEFONTINDIRECT record.
    \return length of the U_WMRCREATEFONTINDIRECT record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  font      pointer to array of U_FONT structure in memory.  Pointer may not be aligned properly for structure.
*/
int U_WMRCREATEFONTINDIRECT_get(
      const char   *contents,
      const char  **font
   ){
   return U_WMRCORE_2U16_N16_get(contents, (U_SIZE_WMRCREATEFONTINDIRECT), NULL, NULL, font);
}

/**
    \brief Get data from a  U_WMRCREATEBRUSHINDIRECT record.
    \return length of the U_WMRCREATEBRUSHINDIRECT record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  brush      pointer to U_WLOGBRUSH structure in memory.  Pointer may not be aligned properly for structure.
*/
int U_WMRCREATEBRUSHINDIRECT_get(
      const char   *contents,
      const char  **brush
    ){
    return U_WMRCORE_2U16_N16_get(contents, (U_SIZE_WMRCREATEBRUSHINDIRECT), NULL, NULL, brush);
}

/** in GDI and Wine, not in WMF manual.
*/
int U_WMRCREATEBITMAPINDIRECT_get(void){
   return U_WMRCORENONE_get("U_WMRCREATEBITMAPINDIRECT");
}

/** in GDI and Wine, not in WMF manual.
*/
int U_WMRCREATEBITMAP_get(void){
   return U_WMRCORENONE_get("U_WMRCREATEBITMAP");
}

/**
    \brief Get data from a  U_WMRCREATEREGION record.
    \return length of the U_WMRCREATEREGION record in bytes, or 0 on error
    \param  contents   record to extract data from
    \param  Region     pointer to U_REGION structure in memory.  Pointer may not be aligned properly for structure.
*/
int U_WMRCREATEREGION_get(
      const char   *contents,
      const char  **Region
    ){
    return U_WMRCORE_2U16_N16_get(contents, (U_SIZE_WMRCREATEREGION), NULL, NULL, Region);
}



#ifdef __cplusplus
}
#endif
