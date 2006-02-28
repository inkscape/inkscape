/**
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2005 Bob Jamison
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
 */

#include "charclass.h"


/**
 * (impl) LetterOrDigit ::=
 *        Letter | Digit
 */
bool isLetterOrDigit(int ch)
{
    if (isLetter(ch))
        return true;
    if (isDigit(ch))
        return true;
    return false;
}

/**
 * (84) Letter ::=
 *        BaseChar | Ideographic
 */
bool isLetter(int ch)
{
    if (isBaseChar(ch))
        return true;
    if (isIdeographic(ch))
        return true;
    return false;
}


/**
 * (85) BaseChar ::=
 */
bool isBaseChar(int ch)
{

    if ( (0x0041 <= ch && ch <= 0x005A) |
         (0x0061 <= ch && ch <= 0x007A) |
         (0x00C0 <= ch && ch <= 0x00D6) |
         (0x00D8 <= ch && ch <= 0x00F6) |
         (0x00F8 <= ch && ch <= 0x00FF) |
         (0x0100 <= ch && ch <= 0x0131) |
         (0x0134 <= ch && ch <= 0x013E) |
         (0x0141 <= ch && ch <= 0x0148) |
         (0x014A <= ch && ch <= 0x017E) |
         (0x0180 <= ch && ch <= 0x01C3) |
         (0x01CD <= ch && ch <= 0x01F0) |
         (0x01F4 <= ch && ch <= 0x01F5) |
         (0x01FA <= ch && ch <= 0x0217) |
         (0x0250 <= ch && ch <= 0x02A8) |
         (0x02BB <= ch && ch <= 0x02C1) |
         ch == 0x0386 |
         (0x0388 <= ch && ch <= 0x038A) |
         ch == 0x038C |
         (0x038E <= ch && ch <= 0x03A1) |
         (0x03A3 <= ch && ch <= 0x03CE) |
         (0x03D0 <= ch && ch <= 0x03D6) |
         ch == 0x03DA |
         ch == 0x03DC |
         ch == 0x03DE |
         ch == 0x03E0 |
         (0x03E2 <= ch && ch <= 0x03F3) |
         (0x0401 <= ch && ch <= 0x040C) |
         (0x040E <= ch && ch <= 0x044F) |
         (0x0451 <= ch && ch <= 0x045C) |
         (0x045E <= ch && ch <= 0x0481) |
         (0x0490 <= ch && ch <= 0x04C4) |
         (0x04C7 <= ch && ch <= 0x04C8) |
         (0x04CB <= ch && ch <= 0x04CC) |
         (0x04D0 <= ch && ch <= 0x04EB) |
         (0x04EE <= ch && ch <= 0x04F5) |
         (0x04F8 <= ch && ch <= 0x04F9) |
         (0x0531 <= ch && ch <= 0x0556) |
         ch == 0x0559 |
         (0x0561 <= ch && ch <= 0x0586) |
         (0x05D0 <= ch && ch <= 0x05EA) |
         (0x05F0 <= ch && ch <= 0x05F2) |
         (0x0621 <= ch && ch <= 0x063A) |
         (0x0641 <= ch && ch <= 0x064A) |
         (0x0671 <= ch && ch <= 0x06B7) |
         (0x06BA <= ch && ch <= 0x06BE) |
         (0x06C0 <= ch && ch <= 0x06CE) |
         (0x06D0 <= ch && ch <= 0x06D3) |
         ch == 0x06D5 |
         (0x06E5 <= ch && ch <= 0x06E6) |
         (0x0905 <= ch && ch <= 0x0939) |
         ch == 0x093D |
         (0x0958 <= ch && ch <= 0x0961) |
         (0x0985 <= ch && ch <= 0x098C) |
         (0x098F <= ch && ch <= 0x0990) |
         (0x0993 <= ch && ch <= 0x09A8) |
         (0x09AA <= ch && ch <= 0x09B0) |
         ch == 0x09B2 |
         (0x09B6 <= ch && ch <= 0x09B9) |
         (0x09DC <= ch && ch <= 0x09DD) |
         (0x09DF <= ch && ch <= 0x09E1) |
         (0x09F0 <= ch && ch <= 0x09F1) |
         (0x0A05 <= ch && ch <= 0x0A0A) |
         (0x0A0F <= ch && ch <= 0x0A10) |
         (0x0A13 <= ch && ch <= 0x0A28) |
         (0x0A2A <= ch && ch <= 0x0A30) |
         (0x0A32 <= ch && ch <= 0x0A33) |
         (0x0A35 <= ch && ch <= 0x0A36) |
         (0x0A38 <= ch && ch <= 0x0A39) |
         (0x0A59 <= ch && ch <= 0x0A5C) |
         ch == 0x0A5E |
         (0x0A72 <= ch && ch <= 0x0A74) |
         (0x0A85 <= ch && ch <= 0x0A8B) |
         ch == 0x0A8D |
         (0x0A8F <= ch && ch <= 0x0A91) |
         (0x0A93 <= ch && ch <= 0x0AA8) |
         (0x0AAA <= ch && ch <= 0x0AB0) |
         (0x0AB2 <= ch && ch <= 0x0AB3) |
         (0x0AB5 <= ch && ch <= 0x0AB9) |
         ch == 0x0ABD |
         ch == 0x0AE0 |
         (0x0B05 <= ch && ch <= 0x0B0C) |
         (0x0B0F <= ch && ch <= 0x0B10) |
         (0x0B13 <= ch && ch <= 0x0B28) |
         (0x0B2A <= ch && ch <= 0x0B30) |
         (0x0B32 <= ch && ch <= 0x0B33) |
         (0x0B36 <= ch && ch <= 0x0B39) |
         ch == 0x0B3D |
         (0x0B5C <= ch && ch <= 0x0B5D) |
         (0x0B5F <= ch && ch <= 0x0B61) |
         (0x0B85 <= ch && ch <= 0x0B8A) |
         (0x0B8E <= ch && ch <= 0x0B90) |
         (0x0B92 <= ch && ch <= 0x0B95) |
         (0x0B99 <= ch && ch <= 0x0B9A) |
         ch == 0x0B9C |
         (0x0B9E <= ch && ch <= 0x0B9F) |
         (0x0BA3 <= ch && ch <= 0x0BA4) |
         (0x0BA8 <= ch && ch <= 0x0BAA) |
         (0x0BAE <= ch && ch <= 0x0BB5) |
         (0x0BB7 <= ch && ch <= 0x0BB9) |
         (0x0C05 <= ch && ch <= 0x0C0C) |
         (0x0C0E <= ch && ch <= 0x0C10) |
         (0x0C12 <= ch && ch <= 0x0C28) |
         (0x0C2A <= ch && ch <= 0x0C33) |
         (0x0C35 <= ch && ch <= 0x0C39) |
         (0x0C60 <= ch && ch <= 0x0C61) |
         (0x0C85 <= ch && ch <= 0x0C8C) |
         (0x0C8E <= ch && ch <= 0x0C90) |
         (0x0C92 <= ch && ch <= 0x0CA8) |
         (0x0CAA <= ch && ch <= 0x0CB3) |
         (0x0CB5 <= ch && ch <= 0x0CB9) |
         ch == 0x0CDE |
         (0x0CE0 <= ch && ch <= 0x0CE1) |
         (0x0D05 <= ch && ch <= 0x0D0C) |
         (0x0D0E <= ch && ch <= 0x0D10) |
         (0x0D12 <= ch && ch <= 0x0D28) |
         (0x0D2A <= ch && ch <= 0x0D39) |
         (0x0D60 <= ch && ch <= 0x0D61) |
         (0x0E01 <= ch && ch <= 0x0E2E) |
         ch == 0x0E30 |
         (0x0E32 <= ch && ch <= 0x0E33) |
         (0x0E40 <= ch && ch <= 0x0E45) |
         (0x0E81 <= ch && ch <= 0x0E82) |
         ch == 0x0E84 |
         (0x0E87 <= ch && ch <= 0x0E88) |
         ch == 0x0E8A |
         ch == 0x0E8D |
         (0x0E94 <= ch && ch <= 0x0E97) |
         (0x0E99 <= ch && ch <= 0x0E9F) |
         (0x0EA1 <= ch && ch <= 0x0EA3) |
         ch == 0x0EA5 |
         ch == 0x0EA7 |
         (0x0EAA <= ch && ch <= 0x0EAB) |
         (0x0EAD <= ch && ch <= 0x0EAE) |
         ch == 0x0EB0 |
         (0x0EB2 <= ch && ch <= 0x0EB3) |
         ch == 0x0EBD |
         (0x0EC0 <= ch && ch <= 0x0EC4) |
         (0x0F40 <= ch && ch <= 0x0F47) |
         (0x0F49 <= ch && ch <= 0x0F69) |
         (0x10A0 <= ch && ch <= 0x10C5) |
         (0x10D0 <= ch && ch <= 0x10F6) |
         ch == 0x1100 |
         (0x1102 <= ch && ch <= 0x1103) |
         (0x1105 <= ch && ch <= 0x1107) |
         ch == 0x1109 |
         (0x110B <= ch && ch <= 0x110C) |
         (0x110E <= ch && ch <= 0x1112) |
         ch == 0x113C |
         ch == 0x113E |
         ch == 0x1140 |
         ch == 0x114C |
         ch == 0x114E |
         ch == 0x1150 |
         (0x1154 <= ch && ch <= 0x1155) |
         ch == 0x1159 |
         (0x115F <= ch && ch <= 0x1161) |
         ch == 0x1163 |
         ch == 0x1165 |
         ch == 0x1167 |
         ch == 0x1169 |
         (0x116D <= ch && ch <= 0x116E) |
         (0x1172 <= ch && ch <= 0x1173) |
         ch == 0x1175 |
         ch == 0x119E |
         ch == 0x11A8 |
         ch == 0x11AB |
         (0x11AE <= ch && ch <= 0x11AF) |
         (0x11B7 <= ch && ch <= 0x11B8) |
         ch == 0x11BA |
         (0x11BC <= ch && ch <= 0x11C2) |
         ch == 0x11EB |
         ch == 0x11F0 |
         ch == 0x11F9 |
         (0x1E00 <= ch && ch <= 0x1E9B) |
         (0x1EA0 <= ch && ch <= 0x1EF9) |
         (0x1F00 <= ch && ch <= 0x1F15) |
         (0x1F18 <= ch && ch <= 0x1F1D) |
         (0x1F20 <= ch && ch <= 0x1F45) |
         (0x1F48 <= ch && ch <= 0x1F4D) |
         (0x1F50 <= ch && ch <= 0x1F57) |
         ch == 0x1F59 |
         ch == 0x1F5B |
         ch == 0x1F5D |
         (0x1F5F <= ch && ch <= 0x1F7D) |
         (0x1F80 <= ch && ch <= 0x1FB4) |
         (0x1FB6 <= ch && ch <= 0x1FBC) |
         ch == 0x1FBE |
         (0x1FC2 <= ch && ch <= 0x1FC4) |
         (0x1FC6 <= ch && ch <= 0x1FCC) |
         (0x1FD0 <= ch && ch <= 0x1FD3) |
         (0x1FD6 <= ch && ch <= 0x1FDB) |
         (0x1FE0 <= ch && ch <= 0x1FEC) |
         (0x1FF2 <= ch && ch <= 0x1FF4) |
         (0x1FF6 <= ch && ch <= 0x1FFC) |
         ch == 0x2126 |
         (0x212A <= ch && ch <= 0x212B) |
         ch == 0x212E |
         (0x2180 <= ch && ch <= 0x2182) |
         (0x3041 <= ch && ch <= 0x3094) |
         (0x30A1 <= ch && ch <= 0x30FA) |
         (0x3105 <= ch && ch <= 0x312C) |
         (0xAC00 <= ch && ch <= 0xD7A3)    )
        return true;
    return false;
}



/**
 * (86)	Ideographic ::=
 */
bool isIdeographic(int ch)
{
    if ( (0x4E00 <= ch && ch <=0x9FA5) |
          ch == 0x3007 |
         (0x3021 <= ch && ch <=0x3029)  )
        return true;
    return false;
}

/**
 * (87)	CombiningChar ::=
 */
bool isCombiningChar(int ch)
{
    if ( (0x0300 <= ch && ch <= 0x0345) |
         (0x0360 <= ch && ch <= 0x0361) |
         (0x0483 <= ch && ch <= 0x0486) |
         (0x0591 <= ch && ch <= 0x05A1) |
         (0x05A3 <= ch && ch <= 0x05B9) |
         (0x05BB <= ch && ch <= 0x05BD) |
         ch == 0x05BF |
         (0x05C1 <= ch && ch <= 0x05C2) |
         ch == 0x05C4 |
         (0x064B <= ch && ch <= 0x0652) |
         ch == 0x0670 |
         (0x06D6 <= ch && ch <= 0x06DC) |
         (0x06DD <= ch && ch <= 0x06DF) |
         (0x06E0 <= ch && ch <= 0x06E4) |
         (0x06E7 <= ch && ch <= 0x06E8) |
         (0x06EA <= ch && ch <= 0x06ED) |
         (0x0901 <= ch && ch <= 0x0903) |
         ch == 0x093C |
         (0x093E <= ch && ch <= 0x094C) |
         ch == 0x094D |
         (0x0951 <= ch && ch <= 0x0954) |
         (0x0962 <= ch && ch <= 0x0963) |
         (0x0981 <= ch && ch <= 0x0983) |
         ch == 0x09BC |
         ch == 0x09BE |
         ch == 0x09BF |
         (0x09C0 <= ch && ch <= 0x09C4) |
         (0x09C7 <= ch && ch <= 0x09C8) |
         (0x09CB <= ch && ch <= 0x09CD) |
         ch == 0x09D7 |
         (0x09E2 <= ch && ch <= 0x09E3) |
         ch == 0x0A02 |
         ch == 0x0A3C |
         ch == 0x0A3E |
         ch == 0x0A3F |
         (0x0A40 <= ch && ch <= 0x0A42) |
         (0x0A47 <= ch && ch <= 0x0A48) |
         (0x0A4B <= ch && ch <= 0x0A4D) |
         (0x0A70 <= ch && ch <= 0x0A71) |
         (0x0A81 <= ch && ch <= 0x0A83) |
         ch == 0x0ABC |
         (0x0ABE <= ch && ch <= 0x0AC5) |
         (0x0AC7 <= ch && ch <= 0x0AC9) |
         (0x0ACB <= ch && ch <= 0x0ACD) |
         (0x0B01 <= ch && ch <= 0x0B03) |
         ch == 0x0B3C |
         (0x0B3E <= ch && ch <= 0x0B43) |
         (0x0B47 <= ch && ch <= 0x0B48) |
         (0x0B4B <= ch && ch <= 0x0B4D) |
         (0x0B56 <= ch && ch <= 0x0B57) |
         (0x0B82 <= ch && ch <= 0x0B83) |
         (0x0BBE <= ch && ch <= 0x0BC2) |
         (0x0BC6 <= ch && ch <= 0x0BC8) |
         (0x0BCA <= ch && ch <= 0x0BCD) |
         ch == 0x0BD7 |
         (0x0C01 <= ch && ch <= 0x0C03) |
         (0x0C3E <= ch && ch <= 0x0C44) |
         (0x0C46 <= ch && ch <= 0x0C48) |
         (0x0C4A <= ch && ch <= 0x0C4D) |
         (0x0C55 <= ch && ch <= 0x0C56) |
         (0x0C82 <= ch && ch <= 0x0C83) |
         (0x0CBE <= ch && ch <= 0x0CC4) |
         (0x0CC6 <= ch && ch <= 0x0CC8) |
         (0x0CCA <= ch && ch <= 0x0CCD) |
         (0x0CD5 <= ch && ch <= 0x0CD6) |
         (0x0D02 <= ch && ch <= 0x0D03) |
         (0x0D3E <= ch && ch <= 0x0D43) |
         (0x0D46 <= ch && ch <= 0x0D48) |
         (0x0D4A <= ch && ch <= 0x0D4D) |
         ch == 0x0D57 |
         ch == 0x0E31 |
         (0x0E34 <= ch && ch <= 0x0E3A) |
         (0x0E47 <= ch && ch <= 0x0E4E) |
         ch == 0x0EB1 |
         (0x0EB4 <= ch && ch <= 0x0EB9) |
         (0x0EBB <= ch && ch <= 0x0EBC) |
         (0x0EC8 <= ch && ch <= 0x0ECD) |
         (0x0F18 <= ch && ch <= 0x0F19) |
         ch == 0x0F35 |
         ch == 0x0F37 |
         ch == 0x0F39 |
         ch == 0x0F3E |
         ch == 0x0F3F |
         (0x0F71 <= ch && ch <= 0x0F84) |
         (0x0F86 <= ch && ch <= 0x0F8B) |
         (0x0F90 <= ch && ch <= 0x0F95) |
         ch == 0x0F97 |
         (0x0F99 <= ch && ch <= 0x0FAD) |
         (0x0FB1 <= ch && ch <= 0x0FB7) |
         ch == 0x0FB9 |
         (0x20D0 <= ch && ch <= 0x20DC) |
         ch == 0x20E1 |
         (0x302A <= ch && ch <= 0x302F) |
         ch == 0x3099 |
         ch == 0x309A      )
        return true;
    return false;
}


/**
 * (88)	Digit ::=
 */
bool isDigit(int ch)
{
    if ( (0x0030 <= ch && ch <= 0x0039) |
         (0x0660 <= ch && ch <= 0x0669) |
         (0x06F0 <= ch && ch <= 0x06F9) |
         (0x0966 <= ch && ch <= 0x096F) |
         (0x09E6 <= ch && ch <= 0x09EF) |
         (0x0A66 <= ch && ch <= 0x0A6F) |
         (0x0AE6 <= ch && ch <= 0x0AEF) |
         (0x0B66 <= ch && ch <= 0x0B6F) |
         (0x0BE7 <= ch && ch <= 0x0BEF) |
         (0x0C66 <= ch && ch <= 0x0C6F) |
         (0x0CE6 <= ch && ch <= 0x0CEF) |
         (0x0D66 <= ch && ch <= 0x0D6F) |
         (0x0E50 <= ch && ch <= 0x0E59) |
         (0x0ED0 <= ch && ch <= 0x0ED9) |
         (0x0F20 <= ch && ch <= 0x0F29)   )
        return true;
    return false;
}


/**
 * (89)	Extender ::=
 */
bool isExtender(int ch)
{
    if ( ch == 0x00B7 |
         ch == 0x02D0 |
         ch == 0x02D1 |
         ch == 0x0387 |
         ch == 0x0640 |
         ch == 0x0E46 |
         ch == 0x0EC6 |
         ch == 0x3005 |
         (0x3031 <= ch && ch <= 0x3035) |
         (0x309D <= ch && ch <= 0x309E) |
         (0x30FC <= ch && ch <= 0x30FE)   )
        return true;
    return false;
}










/**
 *
 * Following are from unicode.org, in the UnicodeData file
 * in the Unicode Database
 */

/**
 * UNICODE general class Zs
 */
bool isSpaceSeparator(int ch)
{
    if (ch == 0x0020 ||
        ch == 0x200A ||
        ch == 0x2003 ||
        ch == 0x205F ||
        ch == 0x2005 ||
        ch == 0x202F ||
        ch == 0x2000 ||
        ch == 0x180E ||
        ch == 0x2001 ||
        ch == 0x2004 ||
        ch == 0x3000 ||
        ch == 0x2008 ||
        ch == 0x2006 ||
        ch == 0x2002 ||
        ch == 0x2007 ||
        ch == 0x2009 ||
        ch == 0x00A0 ||
        ch == 0x1680)
        return true;
    return false;
}

/**
 * UNICODE general class Zl
 */
bool isLineSeparator(int ch)
{
    if (ch == 0x2028)
        return true;
    return false;
}

/**
 * UNICODE general class Zp
 */
bool isParagraphSeparator(int ch)
{
    if (ch == 0x2029)
        return true;
    return false;
}

/**
 * The union of the 3 space types.
 */
bool isSpaceChar(int ch)
{
    if ( isSpaceSeparator(ch) ||
         isLineSeparator(ch)  ||
         isParagraphSeparator(ch))
        return true;
    return false;
}

/**
 * 3 spaces in isSpaceChar() which don't break
 */
bool isNonBreakingSpace(int ch)
{
    if (ch == 0x00A0 || ch == 0x2007 || ch == 0x202F)
        return true;
    return false;
}

/**
 *
 */
bool isWhitespace(int ch)
{
    if (isSpaceChar(ch) && !isNonBreakingSpace(ch))
        return true;
    if (ch == 0x0009 || // HORIZONTAL TABULATION
        ch == 0x000A || // LINE FEED.
        ch == 0x000B || // VERTICAL TABULATION.
        ch == 0x000C || // FORM FEED.
        ch == 0x000D || // CARRIAGE RETURN.
        ch == 0x001C || // FILE SEPARATOR.
        ch == 0x001D || // GROUP SEPARATOR.
        ch == 0x001E || // RECORD SEPARATOR.
        ch == 0x001F)   // UNIT SEPARATOR.
        return true;
    return false;
}
























