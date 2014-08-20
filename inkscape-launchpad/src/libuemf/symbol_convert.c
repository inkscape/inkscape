/** @file
 * @brief Windows-only Enhanced Metafile input and output.
 */
/* Authors:
 *   David mathog <mathog@caltech.edu>
 *
 * Copyright (C) 2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 * References:
 *  see unicode-convert.h
 *
 *  v1.4 08/21/2012 Changed this so that the incoming routines use uint32_t and the outgoing use uint16_t.  This gets rid
 *       of wchar_t, which was different sizes on windows/linux, and required lots of ifdef's elsewhere in the code.
 *  v1.3 04/03/2012 Bullets were a problem.  Symbol bullet -> Times New Roman Bullet looks OK, but
 *       it looked bad going the other way.  Changed mapping Symbol bullet to 2219 (Bullet operator, math
 *       symbol.)  That way Symbol bullet can map in and out, while other font bullet an remain in that
 *       font's bullet glyph.
 *  v1.2 03/26/2012 Introduced bug into SingleUnicodeToNon repaired.
 *  v1.1 03/25/2012 Changed ampersand mapping on Wingdings (to avoid normal Ampersand mapping
 *     to Wingdings ampersand when not intended.  Fixed access bugs for when no conversion is
 *     mapped in UnicodeToNon and SingleUnicodeToNon
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>
#include "symbol_convert.h"


static bool hold_symb=0;  // if any of these change, (re)generate the map table
static bool hold_wing=0;
static bool hold_zdng=0;
static bool hold_pua=0;
static unsigned char *from_unicode=NULL;
static unsigned char *to_font=NULL;

/* The following tables were created from the files
  adobe-dingbats.enc.gz
  adobe-symbol.enc.gz
  adobe-standard.enc.gz
  
  which came as part of the X11-font-encodings rpm on Mandriva 2010.
  The original source for the data must have been Adobe.
  Some also from:
  ftp://ftp.unicode.org/Public/MAPPINGS/VENDORS/ADOBE/symbol.txt
  http://www.csn.ul.ie/~caolan/wingdings/proposal/
  www.renderx.com/Tests/zapf-dingbats.pdf

  The intent is as follows:
  
  on conversion from ASCII/extended -> Unicode use the appropriate table for
  the font and change  font + code (symbol, zapf dingbats, wingdings).
  Going the other way, set up two transfer tables,
  the first is unicode -> 0-FF values, and the seond is unicode -> cvt_to_font.
  These tables are filled dingbats, wingdings, then symbols, so with the rightmost one getting
  precedence if both contain the symbol.
  
  Whereever possible do NOT map two input characters to the same output character, use a slightly
  off character if it is somewhat close and disambiguates.

v 1.0.0 14-MAR-2012, David Mathog  

*/

static unsigned int wingdings_convert[256]={
   0xFFFD,   // 0x00  no replacement
   0xFFFD,   // 0x01  no replacement
   0xFFFD,   // 0x02  no replacement
   0xFFFD,   // 0x03  no replacement
   0xFFFD,   // 0x04  no replacement
   0xFFFD,   // 0x05  no replacement
   0xFFFD,   // 0x06  no replacement
   0xFFFD,   // 0x07  no replacement
   0xFFFD,   // 0x08  no replacement
   0xFFFD,   // 0x09  no replacement
   0xFFFD,   // 0x0A  no replacement
   0xFFFD,   // 0x0B  no replacement
   0xFFFD,   // 0x0C  no replacement
   0xFFFD,   // 0x0D  no replacement
   0xFFFD,   // 0x0E  no replacement
   0xFFFD,   // 0x0F  no replacement
   0xFFFD,   // 0x10  no replacement
   0xFFFD,   // 0x11  no replacement
   0xFFFD,   // 0x12  no replacement
   0xFFFD,   // 0x13  no replacement
   0xFFFD,   // 0x14  no replacement
   0xFFFD,   // 0x15  no replacement
   0xFFFD,   // 0x16  no replacement
   0xFFFD,   // 0x17  no replacement
   0xFFFD,   // 0x18  no replacement
   0xFFFD,   // 0x19  no replacement
   0xFFFD,   // 0x1A  no replacement
   0xFFFD,   // 0x1B  no replacement
   0xFFFD,   // 0x1C  no replacement
   0xFFFD,   // 0x1D  no replacement
   0xFFFD,   // 0x1E  no replacement
   0xFFFD,   // 0x1F  no replacement
   0x0020,   // 0x20  SPACE    
   0x270E,   // 0x21  LOWER RIGHT PENCIL (close, but not exact)
   0x2702,   // 0x22  BLACK SCISSORS    
   0x2701,   // 0x23  UPPER BLADE SCISSORS    
   0xFFFD,   // 0x24  no replacement
   0xFFFD,   // 0x25  no replacement
   0xFFFD,   // 0x26  no replacement
   0xFFFD,   // 0x27  no replacement
   0x260E,   // 0x28  BLACK TELEPHONE    
   0x2706,   // 0x29  TELEPHONE LOCATION SIGN (close, but not exact)
   0x2709,   // 0x2A  ENVELOPE    
   0x2709,   // 0x2B  ENVELOPE (close, but not exact)
   0xFFFD,   // 0x2C  no replacement
   0xFFFD,   // 0x2D  no replacement
   0xFFFD,   // 0x2E  no replacement
   0xFFFD,   // 0x2F  no replacement
   0xFFFD,   // 0x30  no replacement
   0xFFFD,   // 0x31  no replacement
   0xFFFD,   // 0x32  no replacement
   0xFFFD,   // 0x33  no replacement
   0xFFFD,   // 0x34  no replacement
   0xFFFD,   // 0x35  no replacement
   0x231B,   // 0x36  HOURGLASS   
   0x2328,   // 0x37  KEYBOARD   
   0xFFFD,   // 0x38  no replacement
   0xFFFD,   // 0x39  no replacement
   0xFFFD,   // 0x3A  no replacement
   0xFFFD,   // 0x3B  no replacement
   0xFFFD,   // 0x3C  no replacement
   0xFFFD,   // 0x3D  no replacement
   0x2707,   // 0x3E  TAPE DRIVE
   0x270D,   // 0x3F  WRITING HAND    
   0x270D,   // 0x40  WRITING HAND (close, but not exact)
   0x270C,   // 0x41  VICTORY HAND   
   0xFFFD,   // 0x42  3 FINGER UP HAND (no replacement)
   0xFFFD,   // 0x43  THUMBS UP HAND (no replacement)
   0xFFFD,   // 0x44  THUMBS DOWN HAND (no replacement)
   0x261C,   // 0x45  WHITE LEFT POINTING INDEX
   0x261E,   // 0x46  WHITE RIGHT POINTING INDEX   
   0x261D,   // 0x47  WHITE UP POINTING INDEX   
   0x261F,   // 0x48  WHITE DOWN POINTING INDEX
   0xFFFD,   // 0x49  OPEN HAND (no replacement)
   0x263A,   // 0x4A  WHITE SMILING FACE   
   0x263A,   // 0x4B  WHITE SMILING FACE (close, but not exact)
   0x2639,   // 0x4C  WHITE FROWNING FACE   
   0xFFFD,   // 0x4D  BOMB (no replacement. 1F4A3)
   0x2620,   // 0x4E  SKULL AND CROSSBONES    
   0x2690,   // 0x4F  WHITE FLAG (not exact)
   0x2691,   // 0x50  WHITE PENANT (use BLACK FLAG)
   0x2708,   // 0x51  AIRPLANE
   0x263C,   // 0x52  WHITE SUN WITH RAYS (close, but not exact)
   0x2602,   // 0x53  RAINDROP (use UMBRELLA)
   0x2744,   // 0x54  SNOWFLAKE    
   0x271D,   // 0x55  WHITE LATIN CROSS (use BLACK CROSS)
   0x271E,   // 0x56  SHADOWED WHITE LATIN CROSS
   0x271F,   // 0x57  CELTIC CROSS (use OUTLINED LATIN CROSS)
   0x2720,   // 0x58  MALTESE CROSS
   0x2721,   // 0x59  STAR OF DAVID   
   0x262A,   // 0x5A  STAR AND CRESCENT    
   0x262F,   // 0x5B  YIN YANG   
   0x0950,   // 0x5C  DEVANGARI OM    CORRECT|CLOSE: Perhaps PROPOSE SACRED OM ?
   0x2638,   // 0x5D  WHEEL OF DHARMA    
   0x2648,   // 0x5E  ARIES   
   0x2649,   // 0x5F  TAURUS   
   0x264A,   // 0x60  GEMINI   
   0x264B,   // 0x61  CANCER   
   0x264C,   // 0x62  LEO   
   0x264D,   // 0x63  VIRGO
   0x264E,   // 0x64  LIBRA   
   0x264F,   // 0x65  SCORPIUS   
   0x2650,   // 0x66  SAGITTARIUS   
   0x2651,   // 0x67  CAPRICORN
   0x2652,   // 0x68  AQUARIUS   
   0x2653,   // 0x69  PISCES   
   0xFFFD,   // 0x6A  LOWER CASE AMPERSAND)?) (no replacement)
   0xFF06,   // 0x6B  AMPERSAND (use FULL WIDTH AMPERSAND, close, but not exact. Do NOT use 0026, or it maps normal Ampersands to Wingdings Ampersand)
   0x25CF,   // 0x6C  BLACK CIRCLE
   0x274D,   // 0x6D  SHADOWED WHITE CIRCLE (close, but not exact)
   0x25A0,   // 0x6E  BLACK SQUARE   
   0x25A3,   // 0x6F  WHITE SQUARE IN BLACK RECTANGLE (use BLACK SQUSRE in WHITE SQUARE)
   0x25A1,   // 0x70  WHITE SQUARE (close, but not exact)
   0x2751,   // 0x71  LOWER RIGHT SHADOWED WHITE SQUARE   
   0x2752,   // 0x72  UPPER RIGHT SHADOWED WHITE SQUARE   
   0x25CA,   // 0x73  LOZENGE (close, but not exact)
   0x25CA,   // 0x74  LOZENGE (close, but not exact)
   0x25C6,   // 0x75  BLACK DIAMOND    
   0x2756,   // 0x76  BLACK DIAMOND MINUS WHITE X    
   0x25C6,   // 0x77  BLACK DIAMOND (close, but not exact)
   0x2327,   // 0x78  X IN A RECTANGLE BOX    
   0x2353,   // 0x79  APL FUNCTIONAL SYMBOL QUAD UP CARET(close, but not exact)
   0x2318,   // 0x7A  PLACE OF INTEREST SIGN   
   0x2740,   // 0x7B  WHITE FLORETTE (close, but not exact)
   0x273F,   // 0x7C  BLACK FLORETTE (close, but not exact)
   0x275D,   // 0x7D  HEAVY DOUBLE TURNED COMMA QUOTATION MARK ORNAMENT    
   0x275E,   // 0x7E  HEAVY DOUBLE COMMA QUOTATION MARK ORNAMENT    
   0xFFFD,   // 0x7F  unused    
   0x24EA,   // 0x80  CIRCLED DIGIT ZERO    
   0x2460,   // 0x81  CIRCLED DIGIT ONE   
   0x2461,   // 0x82  CIRCLED DIGIT TWO    
   0x2462,   // 0x83  CIRCLED DIGIT THREE   
   0x2463,   // 0x84  CIRCLED DIGIT FOUR   
   0x2464,   // 0x85  CIRCLED DIGIT FIVE   
   0x2465,   // 0x86  CIRCLED DIGIT SIX
   0x2466,   // 0x87  CIRCLED DIGIT SEVEN   
   0x2467,   // 0x88  CIRCLED DIGIT EIGHT   
   0x2468,   // 0x89  CIRCLED DIGIT NINE   
   0x2469,   // 0x8A  CIRCLED NUMBER TEN   
   0xFFFD,   // 0x8B  no replacement
   0x2776,   // 0x8C  DINGBAT NEGATIVE CIRCLED DIGIT ONE   
   0x2777,   // 0x8D  DINGBAT NEGATIVE CIRCLED DIGIT TWO   
   0x2778,   // 0x8E  DINGBAT NEGATIVE CIRCLED DIGIT THREE   
   0x2779,   // 0x8F  DINGBAT NEGATIVE CIRCLED DIGIT FOUR   
   0x277A,   // 0x90  DINGBAT NEGATIVE CIRCLED DIGIT FIVE   
   0x277B,   // 0x91  DINGBAT NEGATIVE CIRCLED DIGIT SIX   
   0x277C,   // 0x92  DINGBAT NEGATIVE CIRCLED DIGIT SEVEN   
   0x277D,   // 0x93  DINGBAT NEGATIVE CIRCLED DIGIT EIGHT
   0x277E,   // 0x94  DINGBAT NEGATIVE CIRCLED DIGIT NINE   
   0x277F,   // 0x95  DINGBAT NEGATIVE CIRCLED NUMBER TEN   
   0xFFFD,   // 0x96  ROTATED FLORAL HEART BULLET (no good replacement)
   0xFFFD,   // 0x97  REVERSED ROTATED FLORAL HEART BULLET (no good replacement)
   0xFFFD,   // 0x98  REVERSED ROTATED FLORAL HEART BULLET (no good replacement)
   0xFFFD,   // 0x99  ROTATED FLORAL HEART BULLET (no good replacement)
   0xFFFD,   // 0x9A  ROTATED FLORAL HEART BULLET (no good replacement)
   0xFFFD,   // 0x9B  REVERSED ROTATED FLORAL HEART BULLET (no good replacement)
   0xFFFD,   // 0x9C  REVERSED ROTATED FLORAL HEART BULLET (no good replacement)
   0xFFFD,   // 0x9D  ROTATED FLORAL HEART BULLET (no good replacement)
   0x2219,   // 0x9E  BULLET (use BULLET operator, so normal font BULLET will not convert to Symbol BULLET)
   0x25CF,   // 0x9F  BLACK CIRCLE (close, but not exact)
   0x25AA,   // 0xA0  BLACK VERY SMALL SQUARE
   0x26AA,   // 0xA1  WHITE CIRCLE (use MEDIUM WHITE CIRCLE)
   0x25CB,   // 0xA2  HEAVY WHITE CIRCLE (use WHITE CIRCLE)
   0x25CD,   // 0xA3  HEAVIEST CIRCLE (use CIRCLE WITH VERTICAL FILL)
   0x25C9,   // 0xA4  CIRCLE WITH A CENTRAL DOT (close, dot much bigger)
   0x25CE,   // 0xA5  BULLSEYE
   0x274D,   // 0xA6  SHADOWED WHITE CIRCLE (close, but not exact)
   0xFFED,   // 0xA7  BLACK SMALL SQUARE
   0x2610,   // 0xA8  WHITE SQUARE (close, but not exact, different fro 25A1)
   0xFFFD,   // 0xA9  no replacement
   0x2726,   // 0xAA  BLACK FOUR POINTED STAR    MAYBE
   0x2605,   // 0xAB  BLACK STAR
   0x2736,   // 0xAC  SIX POINTED BLACK STAR   
   0x2737,   // 0xAD  EIGHT POINTED RECTILINEAR BLACK STAR   
   0x2738,   // 0xAE  TWELVE POINTED BLACK STAR   
   0x2735,   // 0xAF  EIGHT POINTED PINWHEEL STAR   
   0xFFFD,   // 0xB0  no replacement
   0xFFFD,   // 0xB1  no replacement
   0x2727,   // 0xB2  WHITE FOUR POINTED STAR   
   0x2726,   // 0xB3  ROTATED WHITE FOUR POINTED STAR (use BLACK FOUR POINTED STAR)
   0xFFFD,   // 0xB4  REPLACEMENT CHARACTER (close, but not exact)
   0x272A,   // 0xB5  CIRCLED WHITE STAR
   0x2730,   // 0xB6  SHADOWED WHITE STAR
   0xFFFD,   // 0xB7  ANALOG CLOCK  1 (no replacement)
   0xFFFD,   // 0xB8  ANALOG CLOCK  2 (no replacement)
   0xFFFD,   // 0xB9  ANALOG CLOCK  3 (no replacement)
   0xFFFD,   // 0xBA  ANALOG CLOCK  4 (no replacement)
   0xFFFD,   // 0xBB  ANALOG CLOCK  5 (no replacement)
   0xFFFD,   // 0xBC  ANALOG CLOCK  6 (no replacement)
   0xFFFD,   // 0xBD  ANALOG CLOCK  7 (no replacement)
   0xFFFD,   // 0xBE  ANALOG CLOCK  8 (no replacement)
   0xFFFD,   // 0xBF  ANALOG CLOCK  9 (no replacement)
   0xFFFD,   // 0xC0  ANALOG CLOCK 10 (no replacement)
   0xFFFD,   // 0xC1  ANALOG CLOCK 11 (no replacement)
   0xFFFD,   // 0xC2  ANALOG CLOCK 12 (no replacement)
   0x21B2,   // 0xC3  TURN ARROW DOWN AND LEFT    (Meaning close, shape differs)
   0x21B3,   // 0xC4  TURN ARROW DOWN AND RIGHT   (Meaning close, shape differs)
   0x21B0,   // 0xC5  TURN ARROW UP AND LEFT      (Meaning close, shape differs)
   0x21B1,   // 0xC6  TURN ARROW UP AND RIGHT     (Meaning close, shape differs)
   0x2B11,   // 0xC7  TURN ARROW LEFT AND UP      (Meaning close, shape differs)
   0x2B0F,   // 0xC8  TURN ARROW RIGHT AND UP     (Meaning close, shape differs)
   0x2B10,   // 0xC9  TURN ARROW LEFT AND DOWN    (Meaning close, shape differs)
   0x2B0E,   // 0xCA  TURN ARROW RIGHT AND DOWN   (Meaning close, shape differs)
   0xFFFD,   // 0xCB  no replacement
   0xFFFD,   // 0xCC  no replacement
   0xFFFD,   // 0xCD  no replacement
   0xFFFD,   // 0xCE  no replacement
   0xFFFD,   // 0xCF  no replacement
   0xFFFD,   // 0xD0  no replacement
   0xFFFD,   // 0xD1  no replacement
   0xFFFD,   // 0xD2  no replacement
   0xFFFD,   // 0xD3  no replacement
   0xFFFD,   // 0xD4  no replacement
   0x232B,   // 0xD5  ERASE TO THE LEFT
   0x2326,   // 0xD6  ERASE TO THE RIGHT   
   0x25C0,   // 0xD7  THREE-D LIGHTED LEFT  ARROWHEAD (Use BLACK LEFT  TRIANGLE)
   0x25B6,   // 0xD8  THREE-D LIGHTED RIGHT ARROWHEAD (Use BLACK RIGHT TRIANGLE, 27A2 is exact but has no other directions)
   0x25B2,   // 0xD9  THREE-D LIGHTED UP  ARROWHEAD (Use BLACK UP  TRIANGLE)
   0x25BC,   // 0xDA  THREE-D LIGHTED DOWN  ARROWHEAD (Use BLACK DOWN  TRIANGLE)
   0xFFFD,   // 0xDB  no replacement
   0x27B2,   // 0xDC  CIRCLED HEAVY WHITE RIGHTWARDS ARROW   
   0xFFFD,   // 0xDD  no replacement
   0xFFFD,   // 0xDE  no replacement
   0x2190,   // 0xDF  LEFT ARROW
   0x2192,   // 0xE0  RIGHT ARROW
   0x2191,   // 0xE1  UP ARROW
   0x2193,   // 0xE2  DOWN ARROW
   0x2196,   // 0xE3  UPPER LEFT ARROW
   0x2197,   // 0xE4  UPPER RIGHT ARROW
   0x2199,   // 0xE5  LOWER LEFT ARROW
   0x2198,   // 0xE6  LOWER RIGHT ARROW
   0x2B05,   // 0xE7  HEAVY LEFT BLACK ARROW (same as regular BLACK ARROW)
   0x2B08,   // 0xE8  HEAVY RIGHT BLACK ARROW (same as regular BLACK ARROW)
   0x2B06,   // 0xE9  HEAVY UP BLACK ARROW (no equiv BLACK ARROW)
   0x2B07,   // 0xEA  HEAVY DOWN BLACK ARROW (same as regular BLACK ARROW)
   0x2B09,   // 0xEB  HEAVY UPPER LEFT BLACK ARROW same as regular BLACK ARROW)
   0x2B08,   // 0xEC  HEAVY UPPER RIGHT BLACK ARROW same as regular BLACK ARROW)
   0x2B0B,   // 0xED  HEAVY LOWER LEFT BLACK ARROW (same as regular BLACK ARROW) 
   0x2B0A,   // 0xEE  HEAVY LOWER RIGHT BLACK ARROW (same as regular BLACK ARROW)
   0x21E6,   // 0xEF  LEFTWARDS WHITE ARROW
   0x21E8,   // 0xF0  RIGHTWARDS WHITE ARROW   
   0x21E7,   // 0xF1  UPWARDS WHITE ARROW
   0x21E9,   // 0xF2  DOWNWARDS WHITE ARROW   
   0x21D4,   // 0xF3  LEFT RIGHT DOUBLE ARROW   
   0x21D5,   // 0xF4  UP DOWN DOUBLE ARROW   
   0x21D6,   // 0xF5  NORTH WEST DOUBLE ARROW (close, but not exact)
   0x21D7,   // 0xF6  NORTH EAST DOUBLE ARROW (close, but not exact)
   0x21D9,   // 0xF7  SOUTH WEST DOUBLE ARROW (close, but not exact)
   0x21D8,   // 0xF8  SOUTH EAST DOUBLE ARROW (close, but not exact)
   0xFFFD,   // 0xF9  no replacement
   0xFFFD,   // 0xFA  no replacement
   0x2717,   // 0xFB  BALLOT X   
   0x2713,   // 0xFC  CHECK MARK   
   0x2612,   // 0xFD  BALLOT BOX WITH X   
   0x2611,   // 0xFE  BALLOT BOX WITH CHECK   
   0xFFFD    // 0xFF  no replacement
};

/* characters from zapf dingbat font, conversion to a unicode font.  Change both the
   code and the font on conversion.  These are untested as the development machine did
   not have the font installed. */
static unsigned int  dingbats_convert[256]={
   0xFFFD,   // 0x00  no replacement
   0xFFFD,   // 0x01  no replacement
   0xFFFD,   // 0x02  no replacement
   0xFFFD,   // 0x03  no replacement
   0xFFFD,   // 0x04  no replacement
   0xFFFD,   // 0x05  no replacement
   0xFFFD,   // 0x06  no replacement
   0xFFFD,   // 0x07  no replacement
   0xFFFD,   // 0x08  no replacement
   0xFFFD,   // 0x09  no replacement
   0xFFFD,   // 0x0A  no replacement
   0xFFFD,   // 0x0B  no replacement
   0xFFFD,   // 0x0C  no replacement
   0xFFFD,   // 0x0D  no replacement
   0xFFFD,   // 0x0E  no replacement
   0xFFFD,   // 0x0F  no replacement
   0xFFFD,   // 0x10  no replacement
   0xFFFD,   // 0x11  no replacement
   0xFFFD,   // 0x12  no replacement
   0xFFFD,   // 0x13  no replacement
   0xFFFD,   // 0x14  no replacement
   0xFFFD,   // 0x15  no replacement
   0xFFFD,   // 0x16  no replacement
   0xFFFD,   // 0x17  no replacement
   0xFFFD,   // 0x18  no replacement
   0xFFFD,   // 0x19  no replacement
   0xFFFD,   // 0x1A  no replacement
   0xFFFD,   // 0x1B  no replacement
   0xFFFD,   // 0x1C  no replacement
   0xFFFD,   // 0x1D  no replacement
   0xFFFD,   // 0x1E  no replacement
   0xFFFD,   // 0x1F  no replacement
   0x0020,   // 0x20  SPACE
   0x2701,   // 0x21  UPPER BLADE SCISSORS
   0x2702,   // 0x22  BLACK SCISSORS
   0x2703,   // 0x23  LOWER BLADE SCISSORS
   0x2704,   // 0x24  WHITE SCISSORS
   0x260E,   // 0x25  BLACK TELEPHONE
   0x2706,   // 0x26  TELEPHONE LOCATION SIGN
   0x2707,   // 0x27  TAPE DRIVE
   0x2708,   // 0x28  AIRPLANE
   0x2709,   // 0x29  ENVELOPE
   0x261B,   // 0x2A  BLACK RIGHT POINTING INDEX
   0x261E,   // 0x2B  WHITE RIGHT POINTING INDEX
   0x270C,   // 0x2C  VICTORY HAND
   0x270D,   // 0x2D  WRITING HAND
   0x270E,   // 0x2E  LOWER RIGHT PENCIL
   0x270F,   // 0x2F  PENCIL
   0x2710,   // 0x30  UPPER RIGHT PENCIL
   0x2711,   // 0x31  WHITE NIB
   0x2712,   // 0x32  BLACK NIB
   0x2713,   // 0x33  CHECK MARK
   0x2714,   // 0x34  HEAVY CHECK MARK
   0x2715,   // 0x35  MULTIPLICATION X
   0x2716,   // 0x36  HEAVY MULTIPLICATION X
   0x2717,   // 0x37  BALLOT X
   0x2718,   // 0x38  HEAVY BALLOT X
   0x2719,   // 0x39  OUTLINED GREEK CROSS
   0x271A,   // 0x3A  HEAVY GREEK CROSS
   0x271B,   // 0x3B  OPEN CENTRE CROSS
   0x271C,   // 0x3C  HEAVY OPEN CENTRE CROSS
   0x271D,   // 0x3D  LATIN CROSS
   0x271E,   // 0x3E  SHADOWED WHITE LATIN CROSS
   0x271F,   // 0x3F  OUTLINED LATIN CROSS
   0x2720,   // 0x40  MALTESE CROSS
   0x2721,   // 0x41  STAR OF DAVID
   0x2722,   // 0x42  FOUR TEARDROP-SPOKED ASTERISK
   0x2723,   // 0x43  FOUR BALLOON-SPOKED ASTERISK
   0x2724,   // 0x44  HEAVY FOUR BALLOON-SPOKED ASTERISK
   0x2725,   // 0x45  FOUR CLUB-SPOKED ASTERISK
   0x2726,   // 0x46  BLACK FOUR POINTED STAR
   0x2727,   // 0x47  WHITE FOUR POINTED STAR
   0x2605,   // 0x48  BLACK STAR
   0x2729,   // 0x49  STRESS OUTLINED WHITE STAR
   0x272A,   // 0x4A  CIRCLED WHITE STAR
   0x272B,   // 0x4B  OPEN CENTRE BLACK STAR
   0x272C,   // 0x4C  BLACK CENTRE WHITE STAR
   0x272D,   // 0x4D  OUTLINED BLACK STAR
   0x272E,   // 0x4E  HEAVY OUTLINED BLACK STAR
   0x272F,   // 0x4F  PINWHEEL STAR
   0x2730,   // 0x50  SHADOWED WHITE STAR
   0x2731,   // 0x51  HEAVY ASTERISK
   0x2732,   // 0x52  OPEN CENTRE ASTERISK
   0x2733,   // 0x53  EIGHT SPOKED ASTERISK
   0x2734,   // 0x54  EIGHT POINTED BLACK STAR
   0x2735,   // 0x55  EIGHT POINTED PINWHEEL STAR
   0x2736,   // 0x56  SIX POINTED BLACK STAR
   0x2737,   // 0x57  EIGHT POINTED RECTILINEAR BLACK STAR
   0x2738,   // 0x58  HEAVY EIGHT POINTED RECTILINEAR BLACK STAR
   0x2739,   // 0x59  TWELVE POINTED BLACK STAR
   0x273A,   // 0x5A  SIXTEEN POINTED ASTERISK
   0x273B,   // 0x5B  TEARDROP-SPOKED ASTERISK
   0x273C,   // 0x5C  OPEN CENTRE TEARDROP-SPOKED ASTERISK
   0x273D,   // 0x5D  HEAVY TEARDROP-SPOKED ASTERISK
   0x273E,   // 0x5E  SIX PETALLED BLACK AND WHITE FLORETTE
   0x273F,   // 0x5F  BLACK FLORETTE
   0x2740,   // 0x60  WHITE FLORETTE
   0x2741,   // 0x61  EIGHT PETALLED OUTLINED BLACK FLORETTE
   0x2742,   // 0x62  CIRCLED OPEN CENTRE EIGHT POINTED STAR
   0x2743,   // 0x63  HEAVY TEARDROP-SPOKED PINWHEEL ASTERISK
   0x2744,   // 0x64  SNOWFLAKE
   0x2745,   // 0x65  TIGHT TRIFOLIATE SNOWFLAKE
   0x2746,   // 0x66  HEAVY CHEVRON SNOWFLAKE
   0x2747,   // 0x67  SPARKLE
   0x2748,   // 0x68  HEAVY SPARKLE
   0x2749,   // 0x69  BALLOON-SPOKED ASTERISK
   0x274A,   // 0x6A  EIGHT TEARDROP-SPOKED PROPELLER ASTERISK
   0x274B,   // 0x6B  HEAVY EIGHT TEARDROP-SPOKED PROPELLER ASTERISK
   0x25CF,   // 0x6C  BLACK CIRCLE
   0x274D,   // 0x6D  SHADOWED WHITE CIRCLE
   0x25A0,   // 0x6E  BLACK SQUARE
   0x274F,   // 0x6F  LOWER RIGHT DROP-SHADOWED WHITE SQUARE
   0x2750,   // 0x70  UPPER RIGHT DROP-SHADOWED WHITE SQUARE
   0x2751,   // 0x71  LOWER RIGHT SHADOWED WHITE SQUARE
   0x2752,   // 0x72  UPPER RIGHT SHADOWED WHITE SQUARE
   0x25B2,   // 0x73  BLACK UP-POINTING TRIANGLE
   0x25BC,   // 0x74  BLACK DOWN-POINTING TRIANGLE
   0x25C6,   // 0x75  BLACK DIAMOND
   0x2756,   // 0x76  BLACK DIAMOND MINUS WHITE X
   0x25D7,   // 0x77  RIGHT HALF BLACK CIRCLE
   0x2758,   // 0x78  LIGHT VERTICAL BAR
   0x2759,   // 0x79  MEDIUM VERTICAL BAR
   0x275A,   // 0x7A  HEAVY VERTICAL BAR
   0x275B,   // 0x7B  HEAVY SINGLE TURNED COMMA QUOTATION MARK ORNAMENT
   0x275C,   // 0x7C  HEAVY SINGLE COMMA QUOTATION MARK ORNAMENT
   0x275D,   // 0x7D  HEAVY DOUBLE TURNED COMMA QUOTATION MARK ORNAMENT
   0x275E,   // 0x7E  HEAVY DOUBLE COMMA QUOTATION MARK ORNAMENT
   0xFFFD,   // 0x7F  no replacement
   0xF8D7,   // 0x80  MEDIUM LEFT PARENTHESIS ORNAMENT
   0xF8D8,   // 0x81  MEDIUM RIGHT PARENTHESIS ORNAMENT
   0xF8D9,   // 0x82  MEDIUM FLATTENED LEFT PARENTHESIS ORNAMENT
   0xF8DA,   // 0x83  MEDIUM FLATTENED RIGHT PARENTHESIS ORNAMENT
   0xF8DB,   // 0x84  MEDIUM LEFT-POINTING ANGLE BRACKET ORNAMENT
   0xF8DC,   // 0x85  MEDIUM RIGHT-POINTING ANGLE BRACKET ORNAMENT
   0xF8DD,   // 0x86  HEAVY LEFT-POINTING ANGLE QUOTATION MARK ORNAMENT
   0xF8DE,   // 0x87  HEAVY RIGHT-POINTING ANGLE QUOTATION MARK ORNAMENT
   0xF8DF,   // 0x88  HEAVY LEFT-POINTING ANGLE BRACKET ORNAMENT
   0xF8E0,   // 0x89  HEAVY RIGHT-POINTING ANGLE BRACKET ORNAMENT
   0xF8E1,   // 0x8A  LIGHT LEFT TORTOISE SHELL BRACKET ORNAMENT
   0xF8E2,   // 0x8B  LIGHT RIGHT TORTOISE SHELL BRACKET ORNAMENT
   0xF8E3,   // 0x8C  MEDIUM LEFT CURLY BRACKET ORNAMENT
   0xF8E4,   // 0x8D  MEDIUM RIGHT CURLY BRACKET ORNAMENT
   0xFFFD,   // 0x8E  no replacement
   0xFFFD,   // 0x8F  no replacement
   0xFFFD,   // 0x90  no replacement
   0xFFFD,   // 0x91  no replacement
   0xFFFD,   // 0x92  no replacement
   0xFFFD,   // 0x93  no replacement
   0xFFFD,   // 0x94  no replacement
   0xFFFD,   // 0x95  no replacement
   0xFFFD,   // 0x96  no replacement
   0xFFFD,   // 0x97  no replacement
   0xFFFD,   // 0x98  no replacement
   0xFFFD,   // 0x99  no replacement
   0xFFFD,   // 0x9A  no replacement
   0xFFFD,   // 0x9B  no replacement
   0xFFFD,   // 0x9C  no replacement
   0xFFFD,   // 0x9D  no replacement
   0xFFFD,   // 0x9E  no replacement
   0xFFFD,   // 0x9F  no replacement
   0xFFFD,   // 0xA0  no replacement
   0x2761,   // 0xA1  CURVED STEM PARAGRAPH SIGN ORNAMENT
   0x2762,   // 0xA2  HEAVY EXCLAMATION MARK ORNAMENT
   0x2763,   // 0xA3  HEAVY HEART EXCLAMATION MARK ORNAMENT
   0x2764,   // 0xA4  HEAVY BLACK HEART
   0x2765,   // 0xA5  ROTATED HEAVY BLACK HEART BULLET
   0x2766,   // 0xA6  FLORAL HEART
   0x2767,   // 0xA7  ROTATED FLORAL HEART BULLET
   0x2663,   // 0xA8  BLACK CLUB SUIT
   0x2666,   // 0xA9  BLACK DIAMOND SUIT
   0x2665,   // 0xAA  BLACK HEART SUIT
   0x2660,   // 0xAB  BLACK SPADE SUIT
   0x2460,   // 0xAC  CIRCLED DIGIT ONE
   0x2461,   // 0xAD  CIRCLED DIGIT TWO
   0x2462,   // 0xAE  CIRCLED DIGIT THREE
   0x2463,   // 0xAF  CIRCLED DIGIT FOUR
   0x2464,   // 0xB0  CIRCLED DIGIT FIVE
   0x2465,   // 0xB1  CIRCLED DIGIT SIX
   0x2466,   // 0xB2  CIRCLED DIGIT SEVEN
   0x2467,   // 0xB3  CIRCLED DIGIT EIGHT
   0x2468,   // 0xB4  CIRCLED DIGIT NINE
   0x2469,   // 0xB5  CIRCLED NUMBER TEN
   0x2776,   // 0xB6  DINGBAT NEGATIVE CIRCLED DIGIT ONE
   0x2777,   // 0xB7  DINGBAT NEGATIVE CIRCLED DIGIT TWO
   0x2778,   // 0xB8  DINGBAT NEGATIVE CIRCLED DIGIT THREE
   0x2779,   // 0xB9  DINGBAT NEGATIVE CIRCLED DIGIT FOUR
   0x277A,   // 0xBA  DINGBAT NEGATIVE CIRCLED DIGIT FIVE
   0x277B,   // 0xBB  DINGBAT NEGATIVE CIRCLED DIGIT SIX
   0x277C,   // 0xBC  DINGBAT NEGATIVE CIRCLED DIGIT SEVEN
   0x277D,   // 0xBD  DINGBAT NEGATIVE CIRCLED DIGIT EIGHT
   0x277E,   // 0xBE  DINGBAT NEGATIVE CIRCLED DIGIT NINE
   0x277F,   // 0xBF  DINGBAT NEGATIVE CIRCLED NUMBER TEN
   0x2780,   // 0xC0  DINGBAT CIRCLED SANS-SERIF DIGIT ONE
   0x2781,   // 0xC1  DINGBAT CIRCLED SANS-SERIF DIGIT TWO
   0x2782,   // 0xC2  DINGBAT CIRCLED SANS-SERIF DIGIT THREE
   0x2783,   // 0xC3  DINGBAT CIRCLED SANS-SERIF DIGIT FOUR
   0x2784,   // 0xC4  DINGBAT CIRCLED SANS-SERIF DIGIT FIVE
   0x2785,   // 0xC5  DINGBAT CIRCLED SANS-SERIF DIGIT SIX
   0x2786,   // 0xC6  DINGBAT CIRCLED SANS-SERIF DIGIT SEVEN
   0x2787,   // 0xC7  DINGBAT CIRCLED SANS-SERIF DIGIT EIGHT
   0x2788,   // 0xC8  DINGBAT CIRCLED SANS-SERIF DIGIT NINE
   0x2789,   // 0xC9  DINGBAT CIRCLED SANS-SERIF NUMBER TEN
   0x278A,   // 0xCA  DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT ONE
   0x278B,   // 0xCB  DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT TWO
   0x278C,   // 0xCC  DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT THREE
   0x278D,   // 0xCD  DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT FOUR
   0x278E,   // 0xCE  DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT FIVE
   0x278F,   // 0xCF  DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT SIX
   0x2790,   // 0xD0  DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT SEVEN
   0x2791,   // 0xD1  DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT EIGHT
   0x2792,   // 0xD2  DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT NINE
   0x2793,   // 0xD3  DINGBAT NEGATIVE CIRCLED SANS-SERIF NUMBER TEN
   0x2794,   // 0xD4  HEAVY WIDE-HEADED RIGHTWARDS ARROW
   0x2192,   // 0xD5  RIGHTWARDS ARROW
   0x2194,   // 0xD6  LEFT RIGHT ARROW
   0x2195,   // 0xD7  UP DOWN ARROW
   0x2798,   // 0xD8  HEAVY SOUTH EAST ARROW
   0x2799,   // 0xD9  HEAVY RIGHTWARDS ARROW
   0x279A,   // 0xDA  HEAVY NORTH EAST ARROW
   0x279B,   // 0xDB  DRAFTING POINT RIGHTWARDS ARROW
   0x279C,   // 0xDC  HEAVY ROUND-TIPPED RIGHTWARDS ARROW
   0x279D,   // 0xDD  TRIANGLE-HEADED RIGHTWARDS ARROW
   0x279E,   // 0xDE  HEAVY TRIANGLE-HEADED RIGHTWARDS ARROW
   0x279F,   // 0xDF  DASHED TRIANGLE-HEADED RIGHTWARDS ARROW
   0x27A0,   // 0xE0  HEAVY DASHED TRIANGLE-HEADED RIGHTWARDS ARROW
   0x27A1,   // 0xE1  BLACK RIGHTWARDS ARROW
   0x27A2,   // 0xE2  THREE-D TOP-LIGHTED RIGHTWARDS ARROWHEAD
   0x27A3,   // 0xE3  THREE-D BOTTOM-LIGHTED RIGHTWARDS ARROWHEAD
   0x27A4,   // 0xE4  BLACK RIGHTWARDS ARROWHEAD
   0x27A5,   // 0xE5  HEAVY BLACK CURVED DOWNWARDS AND RIGHTWARDS ARROW
   0x27A6,   // 0xE6  HEAVY BLACK CURVED UPWARDS AND RIGHTWARDS ARROW
   0x27A7,   // 0xE7  SQUAT BLACK RIGHTWARDS ARROW
   0x27A8,   // 0xE8  HEAVY CONCAVE-POINTED BLACK RIGHTWARDS ARROW
   0x27A9,   // 0xE9  RIGHT-SHADED WHITE RIGHTWARDS ARROW
   0x27AA,   // 0xEA  LEFT-SHADED WHITE RIGHTWARDS ARROW
   0x27AB,   // 0xEB  BACK-TILTED SHADOWED WHITE RIGHTWARDS ARROW
   0x27AC,   // 0xEC  FRONT-TILTED SHADOWED WHITE RIGHTWARDS ARROW
   0x27AD,   // 0xED  HEAVY LOWER RIGHT-SHADOWED WHITE RIGHTWARDS ARROW
   0x27AE,   // 0xEE  HEAVY UPPER RIGHT-SHADOWED WHITE RIGHTWARDS ARROW
   0x27AF,   // 0xEF  NOTCHED LOWER RIGHT-SHADOWED WHITE RIGHTWARDS ARROW
   0xFFFD,   // 0xF0  no replacement
   0x27B1,   // 0xF1  NOTCHED UPPER RIGHT-SHADOWED WHITE RIGHTWARDS ARROW
   0x27B2,   // 0xF2  CIRCLED HEAVY WHITE RIGHTWARDS ARROW
   0x27B3,   // 0xF3  WHITE-FEATHERED RIGHTWARDS ARROW
   0x27B4,   // 0xF4  BLACK-FEATHERED SOUTH EAST ARROW
   0x27B5,   // 0xF5  BLACK-FEATHERED RIGHTWARDS ARROW
   0x27B6,   // 0xF6  BLACK-FEATHERED NORTH EAST ARROW
   0x27B7,   // 0xF7  HEAVY BLACK-FEATHERED SOUTH EAST ARROW
   0x27B8,   // 0xF8  HEAVY BLACK-FEATHERED RIGHTWARDS ARROW
   0x27B9,   // 0xF9  HEAVY BLACK-FEATHERED NORTH EAST ARROW
   0x27BA,   // 0xFA  TEARDROP-BARBED RIGHTWARDS ARROW
   0x27BB,   // 0xFB  HEAVY TEARDROP-SHANKED RIGHTWARDS ARROW
   0x27BC,   // 0xFC  WEDGE-TAILED RIGHTWARDS ARROW
   0x27BD,   // 0xFD  HEAVY WEDGE-TAILED RIGHTWARDS ARROW
   0x27BE,   // 0xFE  OPEN-OUTLINED RIGHTWARDS ARROW
   0xFFFD    // 0xFF  no replacement
};

/* characters from symbol font, conversion to a unicode font.  Change both the
   code and the font on conversion. */
static unsigned int symbol_convert[256]={
   0xFFFD,   // 0x00  no replacement
   0xFFFD,   // 0x01  no replacement
   0xFFFD,   // 0x02  no replacement
   0xFFFD,   // 0x03  no replacement
   0xFFFD,   // 0x04  no replacement
   0xFFFD,   // 0x05  no replacement
   0xFFFD,   // 0x06  no replacement
   0xFFFD,   // 0x07  no replacement
   0xFFFD,   // 0x08  no replacement
   0xFFFD,   // 0x09  no replacement
   0xFFFD,   // 0x0A  no replacement
   0xFFFD,   // 0x0B  no replacement
   0xFFFD,   // 0x0C  no replacement
   0xFFFD,   // 0x0D  no replacement
   0xFFFD,   // 0x0E  no replacement
   0xFFFD,   // 0x0F  no replacement
   0xFFFD,   // 0x10  no replacement
   0xFFFD,   // 0x11  no replacement
   0xFFFD,   // 0x12  no replacement
   0xFFFD,   // 0x13  no replacement
   0xFFFD,   // 0x14  no replacement
   0xFFFD,   // 0x15  no replacement
   0xFFFD,   // 0x16  no replacement
   0xFFFD,   // 0x17  no replacement
   0xFFFD,   // 0x18  no replacement
   0xFFFD,   // 0x19  no replacement
   0xFFFD,   // 0x1A  no replacement
   0xFFFD,   // 0x1B  no replacement
   0xFFFD,   // 0x1C  no replacement
   0xFFFD,   // 0x1D  no replacement
   0xFFFD,   // 0x1E  no replacement
   0xFFFD,   // 0x1F  no replacement
   0x0020,   // 0x20  SPACE
   0x0021,   // 0x21  EXCLAMATION MARK
   0x2200,   // 0x22  FOR ALL
   0x0023,   // 0x23  NUMBER SIGN
   0x2203,   // 0x24  THERE EXISTS
   0x0025,   // 0x25  PERCENT SIGN
   0x0026,   // 0x26  AMPERSAND
   0x220B,   // 0x27  CONTAINS AS MEMBER
   0x0028,   // 0x28  OPENING PARENTHESIS
   0x0029,   // 0x29  CLOSING PARENTHESIS
   0x2217,   // 0x2A  ASTERISK OPERATOR
   0x002B,   // 0x2B  PLUS SIGN
   0x002C,   // 0x2C  COMMA
   0x2212,   // 0x2D  MINUS SIGN
   0x002E,   // 0x2E  PERIOD
   0x002F,   // 0x2F  SLASH
   0x0030,   // 0x30  DIGIT ZERO
   0x0031,   // 0x31  DIGIT ONE
   0x0032,   // 0x32  DIGIT TWO
   0x0033,   // 0x33  DIGIT THREE
   0x0034,   // 0x34  DIGIT FOUR
   0x0035,   // 0x35  DIGIT FIVE
   0x0036,   // 0x36  DIGIT SIX
   0x0037,   // 0x37  DIGIT SEVEN
   0x0038,   // 0x38  DIGIT EIGHT
   0x0039,   // 0x39  DIGIT NINE
   0x003A,   // 0x3A  COLON
   0x003B,   // 0x3B  SEMICOLON
   0x003C,   // 0x3C  LESS-THAN SIGN
   0x003D,   // 0x3D  EQUALS SIGN
   0x003E,   // 0x3E  GREATER-THAN SIGN
   0x003F,   // 0x3F  QUESTION MARK
   0x2245,   // 0x40  APPROXIMATELY EQUAL TO
   0x0391,   // 0x41  GREEK CAPITAL LETTER ALPHA
   0x0392,   // 0x42  GREEK CAPITAL LETTER BETA
   0x03A7,   // 0x43  GREEK CAPITAL LETTER CHI
   0x0394,   // 0x44  GREEK CAPITAL LETTER DELTA
   0x0395,   // 0x45  GREEK CAPITAL LETTER EPSILON
   0x03A6,   // 0x46  GREEK CAPITAL LETTER PHI
   0x0393,   // 0x47  GREEK CAPITAL LETTER GAMMA
   0x0397,   // 0x48  GREEK CAPITAL LETTER ETA
   0x0399,   // 0x49  GREEK CAPITAL LETTER IOTA
   0x03D1,   // 0x4A  GREEK SMALL LETTER SCRIPT THETA
   0x039A,   // 0x4B  GREEK CAPITAL LETTER KAPPA
   0x039B,   // 0x4C  GREEK CAPITAL LETTER LAMBDA
   0x039C,   // 0x4D  GREEK CAPITAL LETTER MU
   0x039D,   // 0x4E  GREEK CAPITAL LETTER NU
   0x039F,   // 0x4F  GREEK CAPITAL LETTER OMICRON
   0x03A0,   // 0x50  GREEK CAPITAL LETTER PI
   0x0398,   // 0x51  GREEK CAPITAL LETTER THETA
   0x03A1,   // 0x52  GREEK CAPITAL LETTER RHO
   0x03A3,   // 0x53  GREEK CAPITAL LETTER SIGMA
   0x03A4,   // 0x54  GREEK CAPITAL LETTER TAU
   0x03A5,   // 0x55  GREEK CAPITAL LETTER UPSILON
   0x03C2,   // 0x56  GREEK SMALL LETTER FINAL SIGMA
   0x03A9,   // 0x57  GREEK CAPITAL LETTER OMEGA
   0x039E,   // 0x58  GREEK CAPITAL LETTER XI
   0x03A8,   // 0x59  GREEK CAPITAL LETTER PSI
   0x0396,   // 0x5A  GREEK CAPITAL LETTER ZETA
   0x005B,   // 0x5B  OPENING SQUARE BRACKET
   0x2234,   // 0x5C  THEREFORE
   0x005D,   // 0x5D  CLOSING SQUARE BRACKET
   0x22A5,   // 0x5E  UP TACK
   0x005F,   // 0x5F  SPACING UNDERSCORE
   0x203E,   // 0x60  SPACING OVERSCORE
   0x03B1,   // 0x61  GREEK SMALL LETTER ALPHA
   0x03B2,   // 0x62  GREEK SMALL LETTER BETA
   0x03C7,   // 0x63  GREEK SMALL LETTER CHI
   0x03B4,   // 0x64  GREEK SMALL LETTER DELTA
   0x03B5,   // 0x65  GREEK SMALL LETTER EPSILON
   0x03C6,   // 0x66  GREEK SMALL LETTER PHI
   0x03B3,   // 0x67  GREEK SMALL LETTER GAMMA
   0x03B7,   // 0x68  GREEK SMALL LETTER ETA
   0x03B9,   // 0x69  GREEK SMALL LETTER IOTA
   0x03D5,   // 0x6A  GREEK SMALL LETTER SCRIPT PHI
   0x03BA,   // 0x6B  GREEK SMALL LETTER KAPPA
   0x03BB,   // 0x6C  GREEK SMALL LETTER LAMBDA
   0x03BC,   // 0x6D  GREEK SMALL LETTER MU
   0x03BD,   // 0x6E  GREEK SMALL LETTER NU
   0x03BF,   // 0x6F  GREEK SMALL LETTER OMICRON
   0x03C0,   // 0x70  GREEK SMALL LETTER PI
   0x03B8,   // 0x71  GREEK SMALL LETTER THETA
   0x03C1,   // 0x72  GREEK SMALL LETTER RHO
   0x03C3,   // 0x73  GREEK SMALL LETTER SIGMA
   0x03C4,   // 0x74  GREEK SMALL LETTER TAU
   0x03C5,   // 0x75  GREEK SMALL LETTER UPSILON
   0x03D6,   // 0x76  GREEK SMALL LETTER OMEGA PI
   0x03C9,   // 0x77  GREEK SMALL LETTER OMEGA
   0x03BE,   // 0x78  GREEK SMALL LETTER XI
   0x03C8,   // 0x79  GREEK SMALL LETTER PSI
   0x03B6,   // 0x7A  GREEK SMALL LETTER ZETA
   0x007B,   // 0x7B  OPENING CURLY BRACKET
   0x007C,   // 0x7C  VERTICAL BAR
   0x007D,   // 0x7D  CLOSING CURLY BRACKET
   0x223C,   // 0x7E  TILDE OPERATOR
   0xFFFD,   // 0x7F  no replacement
   0xFFFD,   // 0x80  no replacement
   0xFFFD,   // 0x81  no replacement
   0xFFFD,   // 0x82  no replacement
   0xFFFD,   // 0x83  no replacement
   0xFFFD,   // 0x84  no replacement
   0xFFFD,   // 0x85  no replacement
   0xFFFD,   // 0x86  no replacement
   0xFFFD,   // 0x87  no replacement
   0xFFFD,   // 0x88  no replacement
   0xFFFD,   // 0x89  no replacement
   0xFFFD,   // 0x8A  no replacement
   0xFFFD,   // 0x8B  no replacement
   0xFFFD,   // 0x8C  no replacement
   0xFFFD,   // 0x8D  no replacement
   0xFFFD,   // 0x8E  no replacement
   0xFFFD,   // 0x8F  no replacement
   0xFFFD,   // 0x90  no replacement
   0xFFFD,   // 0x91  no replacement
   0xFFFD,   // 0x92  no replacement
   0xFFFD,   // 0x93  no replacement
   0xFFFD,   // 0x94  no replacement
   0xFFFD,   // 0x95  no replacement
   0xFFFD,   // 0x96  no replacement
   0xFFFD,   // 0x97  no replacement
   0xFFFD,   // 0x98  no replacement
   0xFFFD,   // 0x99  no replacement
   0xFFFD,   // 0x9A  no replacement
   0xFFFD,   // 0x9B  no replacement
   0xFFFD,   // 0x9C  no replacement
   0xFFFD,   // 0x9D  no replacement
   0xFFFD,   // 0x9E  no replacement
   0xFFFD,   // 0x9F  no replacement
   0x20AC,   // 0xA0  EURO SIGN
   0x03D2,   // 0xA1  GREEK CAPITAL LETTER UPSILON HOOK
   0x2032,   // 0xA2  PRIME
   0x2264,   // 0xA3  LESS THAN OR EQUAL TO
   0x2044,   // 0xA4  FRACTION SLASH
   0x221E,   // 0xA5  INFINITY
   0x0192,   // 0xA6  LATIN SMALL LETTER SCRIPT F
   0x2663,   // 0xA7  BLACK CLUB SUIT
   0x2666,   // 0xA8  BLACK DIAMOND SUIT
   0x2665,   // 0xA9  BLACK HEART SUIT
   0x2660,   // 0xAA  BLACK SPADE SUIT
   0x2194,   // 0xAB  LEFT RIGHT ARROW
   0x2190,   // 0xAC  LEFT ARROW
   0x2191,   // 0xAD  UP ARROW
   0x2192,   // 0xAE  RIGHT ARROW
   0x2193,   // 0xAF  DOWN ARROW
   0x00B0,   // 0xB0  DEGREE SIGN
   0x00B1,   // 0xB1  PLUS-OR-MINUS SIGN
   0x2033,   // 0xB2  DOUBLE PRIME
   0x2265,   // 0xB3  GREATER THAN OR EQUAL TO
   0x00D7,   // 0xB4  MULTIPLICATION SIGN
   0x221D,   // 0xB5  PROPORTIONAL TO
   0x2202,   // 0xB6  PARTIAL DIFFERENTIAL
   0x2219,   // 0xB7  BULLET (use BULLET operator, so normal font BULLET will not convert to Symbol BULLET)
   0x00F7,   // 0xB8  DIVISION SIGN
   0x2260,   // 0xB9  NOT EQUAL TO
   0x2261,   // 0xBA  IDENTICAL TO
   0x2248,   // 0xBB  ALMOST EQUAL TO
   0x2026,   // 0xBC  HORIZONTAL ELLIPSIS
   0xF8E6,   // 0xBD  VERTICAL ARROW EXTENDER
   0xF8E7,   // 0xBE  HORIZONTAL ARROW EXTENDER
   0x21B5,   // 0xBF  DOWN ARROW WITH CORNER LEFT
   0x2135,   // 0xC0  FIRST TRANSFINITE CARDINAL
   0x2111,   // 0xC1  BLACK-LETTER I
   0x211C,   // 0xC2  BLACK-LETTER R
   0x2118,   // 0xC3  SCRIPT P
   0x2297,   // 0xC4  CIRCLED TIMES
   0x2295,   // 0xC5  CIRCLED PLUS
   0x2205,   // 0xC6  EMPTY SET
   0x2229,   // 0xC7  INTERSECTION
   0x222A,   // 0xC8  UNION
   0x2283,   // 0xC9  SUPERSET OF
   0x2287,   // 0xCA  SUPERSET OF OR EQUAL TO
   0x2284,   // 0xCB  NOT A SUBSET OF
   0x2282,   // 0xCC  SUBSET OF
   0x2286,   // 0xCD  SUBSET OF OR EQUAL TO
   0x2208,   // 0xCE  ELEMENT OF
   0x2209,   // 0xCF  NOT AN ELEMENT OF
   0x2220,   // 0xD0  ANGLE
   0x2207,   // 0xD1  NABLA
   0x00AE,   // 0xD2  REGISTERED TRADE MARK SIGN
   0x00A9,   // 0xD3  COPYRIGHT SIGN
   0x2122,   // 0xD4  TRADEMARK
   0x220F,   // 0xD5  N-ARY PRODUCT
   0x221A,   // 0xD6  SQUARE ROOT
   0x22C5,   // 0xD7  DOT OPERATOR
   0x00AC,   // 0xD8  NOT SIGN
   0x2227,   // 0xD9  LOGICAL AND
   0x2228,   // 0xDA  LOGICAL OR
   0x21D4,   // 0xDB  LEFT RIGHT DOUBLE ARROW
   0x21D0,   // 0xDC  LEFT DOUBLE ARROW
   0x21D1,   // 0xDD  UP DOUBLE ARROW
   0x21D2,   // 0xDE  RIGHT DOUBLE ARROW
   0x21D3,   // 0xDF  DOWN DOUBLE ARROW
   0x25CA,   // 0xE0  LOZENGE
   0x2329,   // 0xE1  BRA
   0x00AE,   // 0xE2  REGISTERED TRADE MARK SIGN
   0x00A9,   // 0xE3  COPYRIGHT SIGN
   0x2122,   // 0xE4  TRADEMARK
   0x2211,   // 0xE5  N-ARY SUMMATION
   0x239B,   // 0xE6  LEFT PAREN TOP
   0x239C,   // 0xE7  LEFT PAREN EXTENDER
   0x239D,   // 0xE8  LEFT PAREN BOTTOM
   0x23A1,   // 0xE9  LEFT SQUARE BRACKET TOP
   0x23A2,   // 0xEA  LEFT SQUARE BRACKET EXTENDER
   0x23A3,   // 0xEB  LEFT SQUARE BRACKET BOTTOM
   0x23A7,   // 0xEC  LEFT CURLY BRACKET TOP
   0x23A8,   // 0xED  LEFT CURLY BRACKET MID
   0x23A9,   // 0xEE  LEFT CURLY BRACKET BOTTOM
   0x23AA,   // 0xEF  CURLY BRACKET EXTENDER
   0xFFFD,   // 0xF0  no replacement
   0x232A,   // 0xF1  KET
   0x222B,   // 0xF2  INTEGRAL
   0x2320,   // 0xF3  TOP HALF INTEGRAL
   0x23AE,   // 0xF4  INTEGRAL EXTENDER
   0x2321,   // 0xF5  BOTTOM HALF INTEGRAL
   0x239E,   // 0xF6  RIGHT PAREN TOP
   0x239F,   // 0xF7  RIGHT PAREN EXTENDER
   0x23A0,   // 0xF8  RIGHT PAREN BOTTOM
   0x23A4,   // 0xF9  RIGHT SQUARE BRACKET TOP
   0x23A5,   // 0xFA  RIGHT SQUARE BRACKET EXTENDER
   0x23A6,   // 0xFB  RIGHT SQUARE BRACKET BOTTOM
   0x23AB,   // 0xFC  RIGHT CURLY BRACKET TOP
   0x23AC,   // 0xFD  RIGHT CURLY BRACKET MID
   0x23AD,   // 0xFE  RIGHT CURLY BRACKET BOTTOM
   0xFFFD    // 0xFF  no replacement
};

/* Use this for debugging */
#include <stdio.h>
void UC_log_message(char *text){
FILE *fp;
  fp=fopen("c:/temp/debug.txt","a");
  fprintf(fp,"%s",text);
  fclose(fp);
}


//if any character is in the MS private use area (F020 through F0FF) subtract F000, for use with Symbol and Wingdings* from older software
void msdepua (uint32_t *text)
{
    while(*text){
        if(*text >= 0xF020 && *text <= 0xF0FF){ *text -= 0xF000; }
        text++;
    }
}

//move characters up to MS private use area (F020 through F0FF) 
void msrepua (uint16_t *text)
{
    while(*text){
        if(*text >= 0x20 && *text <= 0xFF){ *text += 0xF000; }
        text++;
    }
}

// Returns the font classification code
int isNon(char *font){
int retval;
    if(!strcmp(font,"Symbol")){
      retval=CVTSYM;
    }
    else if(!strcmp(font,"Wingdings")){
      retval=CVTWDG;
    }
    else if(!strcmp(font,"ZapfDingbats")){
      retval=CVTZDG;
    }
    else {
      retval=CVTNON;
    }
    return retval;
}

// Returns the font name, given the classification code, or NULL
// The returned value must NOT be free'd
char *FontName(int code){
char *cptr;
static char name_symbol[]="Symbol";
static char name_wingdings[]="Wingdings";
static char name_zapfdingbats[]="ZapfDingbats";
    switch(code){
      case CVTSYM: cptr=&name_symbol[0];        break;
      case CVTWDG: cptr=&name_wingdings[0];     break;
      case CVTZDG: cptr=&name_zapfdingbats[0];  break;
      default:     cptr=NULL;                   break;
    }
    return(cptr);
}


// Goes through the uint32_t string converting as needed.
int NonToUnicode(uint32_t *text, char *font){
int retval;
unsigned int *convert_from=NULL;
    retval=isNon(font);
    switch(retval){
      case CVTSYM: convert_from=symbol_convert;    break;
      case CVTWDG: convert_from=wingdings_convert; break;
      case CVTZDG: convert_from=dingbats_convert;  break;
      default:                                     return(retval);  //no conversion
    }
    while(*text){
        if(*text > 0xFF){ *text = 0xFFFD;                }  // out of range
        else {            *text = convert_from[*text];   }
        text++;
    }
    return(retval);
}

//returns 1 if tables are defines for UnicodeToNon translation
int CanUTN(void){
  if(from_unicode)return(1);
  return(0);
}

//translates from Unicode to some non unicode font until the target font changes.
//A target font change is like symbol -> wingdings, or symbol -> no translation
//returns the number of characters changed in ecount
//returns the enum value for the destination value in edest
void UnicodeToNon(uint16_t *text, int *ecount, int *edest){
int count=0;
unsigned char target=0;
    if(to_font){
      if(text && (target=to_font[*text])){ //There is actually something here to convert
          while(*text && target==to_font[*text]){
              *text=from_unicode[*text] + (hold_pua ? 0xF000 : 0 );
              text++;
              count++;
          }
      }
      *ecount=count;
      *edest=target;
    }
    else {   // no translation tables, so change nothing and return
      *ecount=0;
      *edest=CVTNON;
    }
}

//Indicates the type of translation for a single character, Unicode to some non unicode
//returns the enum value for the destination value.
//If no translation tables are defined returns CVTNON (no conversions)
int SingleUnicodeToNon(uint16_t text){
    if(to_font){return(to_font[text]); }
    else {      return(CVTNON);        }
}

void table_filler(unsigned int *src, int code){
unsigned int i;
   for(i=0;i<0x100;i++){
     if(src[i] == 0xFFFD)continue;  /* no mapping Unicode -> nonUnicode */
     if(src[i] == i)continue;       /* no remapping of spaces back to spaces, for instance */
     from_unicode[src[i]] = i;
     to_font[src[i]]      = code;
   }
}

//possibly (re)generate the tables
void TableGen(bool new_symb,bool new_wing, bool new_zdng, bool new_pua){
int i;
    if(hold_symb != new_symb || hold_wing != new_wing 
       || hold_zdng != new_zdng || hold_pua != new_pua ){ // must (re)generate tables
       if(!from_unicode){ // create arrays
          from_unicode = (unsigned char *) calloc(0x10000,sizeof(unsigned char));
          to_font      = (unsigned char *) calloc(0x10000,sizeof(unsigned char));
          // should check here for malloc error
       }
       hold_symb = new_symb;
       hold_wing = new_wing;
       hold_zdng = new_zdng;
       hold_pua  = new_pua;
       for(i=0;i<0x10000;i++){ from_unicode[i] = to_font[i] = 0; }
       if(hold_zdng)table_filler(&dingbats_convert[0],CVTZDG);
       if(hold_wing)table_filler(&wingdings_convert[0],CVTWDG);
       if(hold_symb)table_filler(&symbol_convert[0],CVTSYM);
   }
}

#ifdef __cplusplus
}
#endif
