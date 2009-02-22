/**
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 * 
 * More thorough explanations of the various classes and their algorithms
 * can be found there.
 *     
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2006-2008 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *  
 */
#ifndef __UCD_H__
#define __UCD_H__


/************************************************
** Unicode character classification
************************************************/


/**
 * Enumerated Unicode general category types
 */
typedef enum UniCharType
{
    UNI_UNASSIGNED                =  0,  /* Cn */
    UNI_UPPERCASE_LETTER          =  1,  /* Lu */
    UNI_LOWERCASE_LETTER          =  2,  /* Ll */
    UNI_TITLECASE_LETTER          =  3,  /* Lt */
    UNI_MODIFIER_LETTER           =  4,  /* Lm */
    UNI_OTHER_LETTER              =  5,  /* Lo */
    UNI_NON_SPACING_MARK          =  6,  /* Mn */
    UNI_ENCLOSING_MARK            =  7,  /* Me */
    UNI_COMBINING_SPACING_MARK    =  8,  /* Mc */
    UNI_DECIMAL_DIGIT_NUMBER      =  9,  /* Nd */
    UNI_LETTER_NUMBER             = 10,  /* Nl */
    UNI_OTHER_NUMBER              = 11,  /* No */
    UNI_SPACE_SEPARATOR           = 12,  /* Zs */
    UNI_LINE_SEPARATOR            = 13,  /* Zl */
    UNI_PARAGRAPH_SEPARATOR       = 14,  /* Zp */
    UNI_CONTROL                   = 15,  /* Cc */
    UNI_FORMAT                    = 16,  /* Cf */
    UNI_UNUSED_RESERVE            = 17,  /* xx */
    UNI_PRIVATE_USE               = 18,  /* Co */
    UNI_SURROGATE                 = 19,  /* Cs */
    UNI_DASH_PUNCTUATION          = 20,  /* Pd */
    UNI_START_PUNCTUATION         = 21,  /* Ps */
    UNI_END_PUNCTUATION           = 22,  /* Pe */
    UNI_CONNECTOR_PUNCTUATION     = 23,  /* Pc */
    UNI_OTHER_PUNCTUATION         = 24,  /* Po */
    UNI_MATH_SYMBOL               = 25,  /* Sm */
    UNI_CURRENCY_SYMBOL           = 26,  /* Sc */
    UNI_MODIFIER_SYMBOL           = 27,  /* Sk */
    UNI_OTHER_SYMBOL              = 28,  /* So */
    UNI_INITIAL_QUOTE_PUNCTUATION = 29,  /* Pi */
    UNI_FINAL_QUOTE_PUNCTUATION   = 30   /* Pf */
} UnicodeCharType;


/**
 * Get the raw table entry for this Unicode codepoint
 * @param ch the Unicode codepoint to test
 * @return the raw UCD property table entry 
 */
unsigned int uni_code(int ch);


/**
 * Get the Unicode General Category of ths character
 * @param ch the Unicode codepoint to test
 * @return the 'UniCharType' General Category enumeration (above)
 */
unsigned int uni_type(int ch);


/**
 * Test if this Unicode code point is lower case
 * @param ch the Unicode codepoint to test
 * @return 1 if successful, else 0
 */
int uni_is_lower(int ch);


/**
 * Test if this Unicode code point is upper case
 * @param ch the Unicode codepoint to test
 * @return 1 if successful, else 0
 */
int uni_is_upper(int ch);


/**
 * Test if this Unicode code point is title case
 * @param ch the Unicode codepoint to test
 * @return 1 if successful, else 0
 */
int uni_is_title(int ch);


/**
 * Test if this Unicode code point is a numeric digit
 * @param ch the Unicode codepoint to test
 * @return 1 if successful, else 0
 */
int uni_is_digit(int ch);


/**
 * Test if this Unicode code point is defined in the database
 * @param ch the Unicode codepoint to test
 * @return 1 if successful, else 0
 */
int uni_is_defined(int ch);

/**
 * Test if this Unicode code point is a letter
 * @param ch the Unicode codepoint to test
 * @return 1 if successful, else 0
 */
int uni_is_letter(int ch);


/**
 * Test if this Unicode code point is a letter or a digit
 * @param ch the Unicode codepoint to test
 * @return 1 if successful, else 0
 */
int uni_is_letter_or_digit(int ch);

/**
 * Test if this Unicode code point is considered to be a space
 * @param ch the Unicode codepoint to test
 * @return 1 if successful, else 0
 */
int uni_is_space(int ch);


/************************************************
** Unicode case conversion
************************************************/

/**
 * Convert the given codepoint to its lower case mapping.
 * If there is none, return the codepoint.
 * @param ch the Unicode codepoint to convert
 * @return the converted codepoint
 */
int uni_to_lower(int ch);

/**
 * Convert the given codepoint to its upper case mapping.
 * If there is none, return the codepoint.
 * @param ch the Unicode codepoint to convert
 * @return the converted codepoint
 */
int uni_to_upper(int ch);

/**
 * Convert the given codepoint to its title case mapping.
 * If there is none, return the codepoint.
 * @param ch the Unicode codepoint to convert
 * @return the converted codepoint
 */
int uni_to_title(int ch);


/************************************************
** Unicode blocks
************************************************/



/**
 * Used to hold the information for a Unicode codepoint
 * block
 */
typedef struct
{
    /**
     * Low end of the block range
     */
    unsigned long low;
    /**
     * High end of the block range
     */
    unsigned long high;
    /**
     * Name string for the block
     */
    const char    *name;
} UcdBlockData;


/**
 * Return the Unicode block (defined below) for the given
 * codepoint.  If not found, return UCD_BLOCK_NO_BLOCK.
 * @param ch the Unicode codepoint to search
 * @return the block
 */
int uni_block(int ch);


/**
 * Return the Unicode block data for the enumerated block number.
 * @param nr the Unicode block number
 * @return the block data if found, else NULL
 */
UcdBlockData *uni_block_data(int blockNr);



           
/**
 * The Unicode codepoint blocks as defined in Blocks.txt.
 * Block list has 171 entries
 */
typedef enum
{
    /*   0, 000000 - 00007f */  UCD_BLOCK_BASIC_LATIN,
    /*   2, 000100 - 00017f */  UCD_BLOCK_LATIN_EXTENDED_A,
    /*   4, 000250 - 0002af */  UCD_BLOCK_IPA_EXTENSIONS,
    /*   6, 000300 - 00036f */  UCD_BLOCK_COMBINING_DIACRITICAL_MARKS,
    /*   8, 000400 - 0004ff */  UCD_BLOCK_CYRILLIC,
    /*  10, 000530 - 00058f */  UCD_BLOCK_ARMENIAN,
    /*  12, 000600 - 0006ff */  UCD_BLOCK_ARABIC,
    /*  14, 000750 - 00077f */  UCD_BLOCK_ARABIC_SUPPLEMENT,
    /*  16, 0007c0 - 0007ff */  UCD_BLOCK_NKO,
    /*  18, 000980 - 0009ff */  UCD_BLOCK_BENGALI,
    /*  20, 000a80 - 000aff */  UCD_BLOCK_GUJARATI,
    /*  22, 000b80 - 000bff */  UCD_BLOCK_TAMIL,
    /*  24, 000c80 - 000cff */  UCD_BLOCK_KANNADA,
    /*  26, 000d80 - 000dff */  UCD_BLOCK_SINHALA,
    /*  28, 000e80 - 000eff */  UCD_BLOCK_LAO,
    /*  30, 001000 - 00109f */  UCD_BLOCK_MYANMAR,
    /*  32, 001100 - 0011ff */  UCD_BLOCK_HANGUL_JAMO,
    /*  34, 001380 - 00139f */  UCD_BLOCK_ETHIOPIC_SUPPLEMENT,
    /*  36, 001400 - 00167f */  UCD_BLOCK_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS,
    /*  38, 0016a0 - 0016ff */  UCD_BLOCK_RUNIC,
    /*  40, 001720 - 00173f */  UCD_BLOCK_HANUNOO,
    /*  42, 001760 - 00177f */  UCD_BLOCK_TAGBANWA,
    /*  44, 001800 - 0018af */  UCD_BLOCK_MONGOLIAN,
    /*  46, 001950 - 00197f */  UCD_BLOCK_TAI_LE,
    /*  48, 0019e0 - 0019ff */  UCD_BLOCK_KHMER_SYMBOLS,
    /*  50, 001b00 - 001b7f */  UCD_BLOCK_BALINESE,
    /*  52, 001c00 - 001c4f */  UCD_BLOCK_LEPCHA,
    /*  54, 001d00 - 001d7f */  UCD_BLOCK_PHONETIC_EXTENSIONS,
    /*  56, 001dc0 - 001dff */  UCD_BLOCK_COMBINING_DIACRITICAL_MARKS_SUPPLEMENT,
    /*  58, 001f00 - 001fff */  UCD_BLOCK_GREEK_EXTENDED,
    /*  60, 002070 - 00209f */  UCD_BLOCK_SUPERSCRIPTS_AND_SUBSCRIPTS,
    /*  62, 0020d0 - 0020ff */  UCD_BLOCK_COMBINING_DIACRITICAL_MARKS_FOR_SYMBOLS,
    /*  64, 002150 - 00218f */  UCD_BLOCK_NUMBER_FORMS,
    /*  66, 002200 - 0022ff */  UCD_BLOCK_MATHEMATICAL_OPERATORS,
    /*  68, 002400 - 00243f */  UCD_BLOCK_CONTROL_PICTURES,
    /*  70, 002460 - 0024ff */  UCD_BLOCK_ENCLOSED_ALPHANUMERICS,
    /*  72, 002580 - 00259f */  UCD_BLOCK_BLOCK_ELEMENTS,
    /*  74, 002600 - 0026ff */  UCD_BLOCK_MISCELLANEOUS_SYMBOLS,
    /*  76, 0027c0 - 0027ef */  UCD_BLOCK_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A,
    /*  78, 002800 - 0028ff */  UCD_BLOCK_BRAILLE_PATTERNS,
    /*  80, 002980 - 0029ff */  UCD_BLOCK_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B,
    /*  82, 002b00 - 002bff */  UCD_BLOCK_MISCELLANEOUS_SYMBOLS_AND_ARROWS,
    /*  84, 002c60 - 002c7f */  UCD_BLOCK_LATIN_EXTENDED_C,
    /*  86, 002d00 - 002d2f */  UCD_BLOCK_GEORGIAN_SUPPLEMENT,
    /*  88, 002d80 - 002ddf */  UCD_BLOCK_ETHIOPIC_EXTENDED,
    /*  90, 002e00 - 002e7f */  UCD_BLOCK_SUPPLEMENTAL_PUNCTUATION,
    /*  92, 002f00 - 002fdf */  UCD_BLOCK_KANGXI_RADICALS,
    /*  94, 003000 - 00303f */  UCD_BLOCK_CJK_SYMBOLS_AND_PUNCTUATION,
    /*  96, 0030a0 - 0030ff */  UCD_BLOCK_KATAKANA,
    /*  98, 003130 - 00318f */  UCD_BLOCK_HANGUL_COMPATIBILITY_JAMO,
    /* 100, 0031a0 - 0031bf */  UCD_BLOCK_BOPOMOFO_EXTENDED,
    /* 102, 0031f0 - 0031ff */  UCD_BLOCK_KATAKANA_PHONETIC_EXTENSIONS,
    /* 104, 003300 - 0033ff */  UCD_BLOCK_CJK_COMPATIBILITY,
    /* 106, 004dc0 - 004dff */  UCD_BLOCK_YIJING_HEXAGRAM_SYMBOLS,
    /* 108, 00a000 - 00a48f */  UCD_BLOCK_YI_SYLLABLES,
    /* 110, 00a500 - 00a63f */  UCD_BLOCK_VAI,
    /* 112, 00a700 - 00a71f */  UCD_BLOCK_MODIFIER_TONE_LETTERS,
    /* 114, 00a800 - 00a82f */  UCD_BLOCK_SYLOTI_NAGRI,
    /* 116, 00a880 - 00a8df */  UCD_BLOCK_SAURASHTRA,
    /* 118, 00a930 - 00a95f */  UCD_BLOCK_REJANG,
    /* 120, 00ac00 - 00d7af */  UCD_BLOCK_HANGUL_SYLLABLES,
    /* 122, 00db80 - 00dbff */  UCD_BLOCK_HIGH_PRIVATE_USE_SURROGATES,
    /* 124, 00e000 - 00f8ff */  UCD_BLOCK_PRIVATE_USE_AREA,
    /* 126, 00fb00 - 00fb4f */  UCD_BLOCK_ALPHABETIC_PRESENTATION_FORMS,
    /* 128, 00fe00 - 00fe0f */  UCD_BLOCK_VARIATION_SELECTORS,
    /* 130, 00fe20 - 00fe2f */  UCD_BLOCK_COMBINING_HALF_MARKS,
    /* 132, 00fe50 - 00fe6f */  UCD_BLOCK_SMALL_FORM_VARIANTS,
    /* 134, 00ff00 - 00ffef */  UCD_BLOCK_HALFWIDTH_AND_FULLWIDTH_FORMS,
    /* 136, 010000 - 01007f */  UCD_BLOCK_LINEAR_B_SYLLABARY,
    /* 138, 010100 - 01013f */  UCD_BLOCK_AEGEAN_NUMBERS,
    /* 140, 010190 - 0101cf */  UCD_BLOCK_ANCIENT_SYMBOLS,
    /* 142, 010280 - 01029f */  UCD_BLOCK_LYCIAN,
    /* 144, 010300 - 01032f */  UCD_BLOCK_OLD_ITALIC,
    /* 146, 010380 - 01039f */  UCD_BLOCK_UGARITIC,
    /* 148, 010400 - 01044f */  UCD_BLOCK_DESERET,
    /* 150, 010480 - 0104af */  UCD_BLOCK_OSMANYA,
    /* 152, 010900 - 01091f */  UCD_BLOCK_PHOENICIAN,
    /* 154, 010a00 - 010a5f */  UCD_BLOCK_KHAROSHTHI,
    /* 156, 012400 - 01247f */  UCD_BLOCK_CUNEIFORM_NUMBERS_AND_PUNCTUATION,
    /* 158, 01d100 - 01d1ff */  UCD_BLOCK_MUSICAL_SYMBOLS,
    /* 160, 01d300 - 01d35f */  UCD_BLOCK_TAI_XUAN_JING_SYMBOLS,
    /* 162, 01d400 - 01d7ff */  UCD_BLOCK_MATHEMATICAL_ALPHANUMERIC_SYMBOLS,
    /* 164, 01f030 - 01f09f */  UCD_BLOCK_DOMINO_TILES,
    /* 166, 02f800 - 02fa1f */  UCD_BLOCK_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT,
    /* 168, 0e0100 - 0e01ef */  UCD_BLOCK_VARIATION_SELECTORS_SUPPLEMENT,
    /* 170, 100000 - 10ffff */  UCD_BLOCK_SUPPLEMENTARY_PRIVATE_USE_AREA_B,
    /* 171, 000000 - 10ffff */  UCD_BLOCK_NO_BLOCK
} UnicodeBlocks;


#endif /* __UCD_H__ */


