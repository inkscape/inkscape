/* Authors:
 *   Jon A. Cruz
 *   Abhishek Sharma
 *   Tavmjong Bah
 *
 * Copyright (C) 2010 Jon A. Cruz
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <vector>

#include "glyphs.h"

#include <glibmm/i18n.h>
#include <gtkmm/alignment.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/entry.h>
#include <gtkmm/iconview.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>

#if WITH_GTKMM_3_0
# include <gtkmm/grid.h>
#else
# include <gtkmm/table.h>
#endif

#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/widget.h>

#include "desktop.h"
#include "document.h" // for SPDocumentUndo::done()
#include "document-undo.h"
#include "libnrtype/font-instance.h"
#include "sp-flowtext.h"
#include "sp-text.h"
#include "verbs.h"
#include "widgets/font-selector.h"
#include "text-editing.h"
#include "selection.h"

namespace Inkscape {
namespace UI {
namespace Dialog {


GlyphsPanel &GlyphsPanel::getInstance()
{
    return *new GlyphsPanel();
}


static std::map<GUnicodeScript, Glib::ustring> & getScriptToName()
{
    static bool init = false;
    static std::map<GUnicodeScript, Glib::ustring> mappings;
    if (!init) {
        init = true;
        mappings[G_UNICODE_SCRIPT_INVALID_CODE]         = _("all");
        mappings[G_UNICODE_SCRIPT_COMMON]               = _("common");
        mappings[G_UNICODE_SCRIPT_INHERITED]            = _("inherited");
        mappings[G_UNICODE_SCRIPT_ARABIC]               = _("Arabic");
        mappings[G_UNICODE_SCRIPT_ARMENIAN]             = _("Armenian");
        mappings[G_UNICODE_SCRIPT_BENGALI]              = _("Bengali");
        mappings[G_UNICODE_SCRIPT_BOPOMOFO]             = _("Bopomofo");
        mappings[G_UNICODE_SCRIPT_CHEROKEE]             = _("Cherokee");
        mappings[G_UNICODE_SCRIPT_COPTIC]               = _("Coptic");
        mappings[G_UNICODE_SCRIPT_CYRILLIC]             = _("Cyrillic");
        mappings[G_UNICODE_SCRIPT_DESERET]              = _("Deseret");
        mappings[G_UNICODE_SCRIPT_DEVANAGARI]           = _("Devanagari");
        mappings[G_UNICODE_SCRIPT_ETHIOPIC]             = _("Ethiopic");
        mappings[G_UNICODE_SCRIPT_GEORGIAN]             = _("Georgian");
        mappings[G_UNICODE_SCRIPT_GOTHIC]               = _("Gothic");
        mappings[G_UNICODE_SCRIPT_GREEK]                = _("Greek");
        mappings[G_UNICODE_SCRIPT_GUJARATI]             = _("Gujarati");
        mappings[G_UNICODE_SCRIPT_GURMUKHI]             = _("Gurmukhi");
        mappings[G_UNICODE_SCRIPT_HAN]                  = _("Han");
        mappings[G_UNICODE_SCRIPT_HANGUL]               = _("Hangul");
        mappings[G_UNICODE_SCRIPT_HEBREW]               = _("Hebrew");
        mappings[G_UNICODE_SCRIPT_HIRAGANA]             = _("Hiragana");
        mappings[G_UNICODE_SCRIPT_KANNADA]              = _("Kannada");
        mappings[G_UNICODE_SCRIPT_KATAKANA]             = _("Katakana");
        mappings[G_UNICODE_SCRIPT_KHMER]                = _("Khmer");
        mappings[G_UNICODE_SCRIPT_LAO]                  = _("Lao");
        mappings[G_UNICODE_SCRIPT_LATIN]                = _("Latin");
        mappings[G_UNICODE_SCRIPT_MALAYALAM]            = _("Malayalam");
        mappings[G_UNICODE_SCRIPT_MONGOLIAN]            = _("Mongolian");
        mappings[G_UNICODE_SCRIPT_MYANMAR]              = _("Myanmar");
        mappings[G_UNICODE_SCRIPT_OGHAM]                = _("Ogham");
        mappings[G_UNICODE_SCRIPT_OLD_ITALIC]           = _("Old Italic");
        mappings[G_UNICODE_SCRIPT_ORIYA]                = _("Oriya");
        mappings[G_UNICODE_SCRIPT_RUNIC]                = _("Runic");
        mappings[G_UNICODE_SCRIPT_SINHALA]              = _("Sinhala");
        mappings[G_UNICODE_SCRIPT_SYRIAC]               = _("Syriac");
        mappings[G_UNICODE_SCRIPT_TAMIL]                = _("Tamil");
        mappings[G_UNICODE_SCRIPT_TELUGU]               = _("Telugu");
        mappings[G_UNICODE_SCRIPT_THAANA]               = _("Thaana");
        mappings[G_UNICODE_SCRIPT_THAI]                 = _("Thai");
        mappings[G_UNICODE_SCRIPT_TIBETAN]              = _("Tibetan");
        mappings[G_UNICODE_SCRIPT_CANADIAN_ABORIGINAL]  = _("Canadian Aboriginal");
        mappings[G_UNICODE_SCRIPT_YI]                   = _("Yi");
        mappings[G_UNICODE_SCRIPT_TAGALOG]              = _("Tagalog");
        mappings[G_UNICODE_SCRIPT_HANUNOO]              = _("Hanunoo");
        mappings[G_UNICODE_SCRIPT_BUHID]                = _("Buhid");
        mappings[G_UNICODE_SCRIPT_TAGBANWA]             = _("Tagbanwa");
        mappings[G_UNICODE_SCRIPT_BRAILLE]              = _("Braille");
        mappings[G_UNICODE_SCRIPT_CYPRIOT]              = _("Cypriot");
        mappings[G_UNICODE_SCRIPT_LIMBU]                = _("Limbu");
        mappings[G_UNICODE_SCRIPT_OSMANYA]              = _("Osmanya");
        mappings[G_UNICODE_SCRIPT_SHAVIAN]              = _("Shavian");
        mappings[G_UNICODE_SCRIPT_LINEAR_B]             = _("Linear B");
        mappings[G_UNICODE_SCRIPT_TAI_LE]               = _("Tai Le");
        mappings[G_UNICODE_SCRIPT_UGARITIC]             = _("Ugaritic");
        mappings[G_UNICODE_SCRIPT_NEW_TAI_LUE]          = _("New Tai Lue");
        mappings[G_UNICODE_SCRIPT_BUGINESE]             = _("Buginese");
        mappings[G_UNICODE_SCRIPT_GLAGOLITIC]           = _("Glagolitic");
        mappings[G_UNICODE_SCRIPT_TIFINAGH]             = _("Tifinagh");
        mappings[G_UNICODE_SCRIPT_SYLOTI_NAGRI]         = _("Syloti Nagri");
        mappings[G_UNICODE_SCRIPT_OLD_PERSIAN]          = _("Old Persian");
        mappings[G_UNICODE_SCRIPT_KHAROSHTHI]           = _("Kharoshthi");
        mappings[G_UNICODE_SCRIPT_UNKNOWN]              = _("unassigned");
        mappings[G_UNICODE_SCRIPT_BALINESE]             = _("Balinese");
        mappings[G_UNICODE_SCRIPT_CUNEIFORM]            = _("Cuneiform");
        mappings[G_UNICODE_SCRIPT_PHOENICIAN]           = _("Phoenician");
        mappings[G_UNICODE_SCRIPT_PHAGS_PA]             = _("Phags-pa");
        mappings[G_UNICODE_SCRIPT_NKO]                  = _("N'Ko");
        mappings[G_UNICODE_SCRIPT_KAYAH_LI]             = _("Kayah Li");
        mappings[G_UNICODE_SCRIPT_LEPCHA]               = _("Lepcha");
        mappings[G_UNICODE_SCRIPT_REJANG]               = _("Rejang");
        mappings[G_UNICODE_SCRIPT_SUNDANESE]            = _("Sundanese");
        mappings[G_UNICODE_SCRIPT_SAURASHTRA]           = _("Saurashtra");
        mappings[G_UNICODE_SCRIPT_CHAM]                 = _("Cham");
        mappings[G_UNICODE_SCRIPT_OL_CHIKI]             = _("Ol Chiki");
        mappings[G_UNICODE_SCRIPT_VAI]                  = _("Vai");
        mappings[G_UNICODE_SCRIPT_CARIAN]               = _("Carian");
        mappings[G_UNICODE_SCRIPT_LYCIAN]               = _("Lycian");
        mappings[G_UNICODE_SCRIPT_LYDIAN]               = _("Lydian");
    }
    return mappings;
}

typedef std::pair<gunichar, gunichar> Range;
typedef std::pair<Range, Glib::ustring> NamedRange;

static std::vector<NamedRange> & getRanges()
{
    static bool init = false;
    static std::vector<NamedRange> ranges;
    if (!init) {
        init = true;
        ranges.push_back(std::make_pair(std::make_pair(0x0000, 0xFFFD), _("all")));
        ranges.push_back(std::make_pair(std::make_pair(0x0000, 0x007F), _("Basic Latin")));
        ranges.push_back(std::make_pair(std::make_pair(0x0080, 0x00FF), _("Latin-1 Supplement")));
        ranges.push_back(std::make_pair(std::make_pair(0x0100, 0x017F), _("Latin Extended-A")));
        ranges.push_back(std::make_pair(std::make_pair(0x0180, 0x024F), _("Latin Extended-B")));
        ranges.push_back(std::make_pair(std::make_pair(0x0250, 0x02AF), _("IPA Extensions")));
        ranges.push_back(std::make_pair(std::make_pair(0x02B0, 0x02FF), _("Spacing Modifier Letters")));
        ranges.push_back(std::make_pair(std::make_pair(0x0300, 0x036F), _("Combining Diacritical Marks")));
        ranges.push_back(std::make_pair(std::make_pair(0x0370, 0x03FF), _("Greek and Coptic")));
        ranges.push_back(std::make_pair(std::make_pair(0x0400, 0x04FF), _("Cyrillic")));
        ranges.push_back(std::make_pair(std::make_pair(0x0500, 0x052F), _("Cyrillic Supplement")));
        ranges.push_back(std::make_pair(std::make_pair(0x0530, 0x058F), _("Armenian")));
        ranges.push_back(std::make_pair(std::make_pair(0x0590, 0x05FF), _("Hebrew")));
        ranges.push_back(std::make_pair(std::make_pair(0x0600, 0x06FF), _("Arabic")));
        ranges.push_back(std::make_pair(std::make_pair(0x0700, 0x074F), _("Syriac")));
        ranges.push_back(std::make_pair(std::make_pair(0x0750, 0x077F), _("Arabic Supplement")));
        ranges.push_back(std::make_pair(std::make_pair(0x0780, 0x07BF), _("Thaana")));
        ranges.push_back(std::make_pair(std::make_pair(0x07C0, 0x07FF), _("NKo")));
        ranges.push_back(std::make_pair(std::make_pair(0x0800, 0x083F), _("Samaritan")));
        ranges.push_back(std::make_pair(std::make_pair(0x0900, 0x097F), _("Devanagari")));
        ranges.push_back(std::make_pair(std::make_pair(0x0980, 0x09FF), _("Bengali")));
        ranges.push_back(std::make_pair(std::make_pair(0x0A00, 0x0A7F), _("Gurmukhi")));
        ranges.push_back(std::make_pair(std::make_pair(0x0A80, 0x0AFF), _("Gujarati")));
        ranges.push_back(std::make_pair(std::make_pair(0x0B00, 0x0B7F), _("Oriya")));
        ranges.push_back(std::make_pair(std::make_pair(0x0B80, 0x0BFF), _("Tamil")));
        ranges.push_back(std::make_pair(std::make_pair(0x0C00, 0x0C7F), _("Telugu")));
        ranges.push_back(std::make_pair(std::make_pair(0x0C80, 0x0CFF), _("Kannada")));
        ranges.push_back(std::make_pair(std::make_pair(0x0D00, 0x0D7F), _("Malayalam")));
        ranges.push_back(std::make_pair(std::make_pair(0x0D80, 0x0DFF), _("Sinhala")));
        ranges.push_back(std::make_pair(std::make_pair(0x0E00, 0x0E7F), _("Thai")));
        ranges.push_back(std::make_pair(std::make_pair(0x0E80, 0x0EFF), _("Lao")));
        ranges.push_back(std::make_pair(std::make_pair(0x0F00, 0x0FFF), _("Tibetan")));
        ranges.push_back(std::make_pair(std::make_pair(0x1000, 0x109F), _("Myanmar")));
        ranges.push_back(std::make_pair(std::make_pair(0x10A0, 0x10FF), _("Georgian")));
        ranges.push_back(std::make_pair(std::make_pair(0x1100, 0x11FF), _("Hangul Jamo")));
        ranges.push_back(std::make_pair(std::make_pair(0x1200, 0x137F), _("Ethiopic")));
        ranges.push_back(std::make_pair(std::make_pair(0x1380, 0x139F), _("Ethiopic Supplement")));
        ranges.push_back(std::make_pair(std::make_pair(0x13A0, 0x13FF), _("Cherokee")));
        ranges.push_back(std::make_pair(std::make_pair(0x1400, 0x167F), _("Unified Canadian Aboriginal Syllabics")));
        ranges.push_back(std::make_pair(std::make_pair(0x1680, 0x169F), _("Ogham")));
        ranges.push_back(std::make_pair(std::make_pair(0x16A0, 0x16FF), _("Runic")));
        ranges.push_back(std::make_pair(std::make_pair(0x1700, 0x171F), _("Tagalog")));
        ranges.push_back(std::make_pair(std::make_pair(0x1720, 0x173F), _("Hanunoo")));
        ranges.push_back(std::make_pair(std::make_pair(0x1740, 0x175F), _("Buhid")));
        ranges.push_back(std::make_pair(std::make_pair(0x1760, 0x177F), _("Tagbanwa")));
        ranges.push_back(std::make_pair(std::make_pair(0x1780, 0x17FF), _("Khmer")));
        ranges.push_back(std::make_pair(std::make_pair(0x1800, 0x18AF), _("Mongolian")));
        ranges.push_back(std::make_pair(std::make_pair(0x18B0, 0x18FF), _("Unified Canadian Aboriginal Syllabics Extended")));
        ranges.push_back(std::make_pair(std::make_pair(0x1900, 0x194F), _("Limbu")));
        ranges.push_back(std::make_pair(std::make_pair(0x1950, 0x197F), _("Tai Le")));
        ranges.push_back(std::make_pair(std::make_pair(0x1980, 0x19DF), _("New Tai Lue")));
        ranges.push_back(std::make_pair(std::make_pair(0x19E0, 0x19FF), _("Khmer Symbols")));
        ranges.push_back(std::make_pair(std::make_pair(0x1A00, 0x1A1F), _("Buginese")));
        ranges.push_back(std::make_pair(std::make_pair(0x1A20, 0x1AAF), _("Tai Tham")));
        ranges.push_back(std::make_pair(std::make_pair(0x1B00, 0x1B7F), _("Balinese")));
        ranges.push_back(std::make_pair(std::make_pair(0x1B80, 0x1BBF), _("Sundanese")));
        ranges.push_back(std::make_pair(std::make_pair(0x1C00, 0x1C4F), _("Lepcha")));
        ranges.push_back(std::make_pair(std::make_pair(0x1C50, 0x1C7F), _("Ol Chiki")));
        ranges.push_back(std::make_pair(std::make_pair(0x1CD0, 0x1CFF), _("Vedic Extensions")));
        ranges.push_back(std::make_pair(std::make_pair(0x1D00, 0x1D7F), _("Phonetic Extensions")));
        ranges.push_back(std::make_pair(std::make_pair(0x1D80, 0x1DBF), _("Phonetic Extensions Supplement")));
        ranges.push_back(std::make_pair(std::make_pair(0x1DC0, 0x1DFF), _("Combining Diacritical Marks Supplement")));
        ranges.push_back(std::make_pair(std::make_pair(0x1E00, 0x1EFF), _("Latin Extended Additional")));
        ranges.push_back(std::make_pair(std::make_pair(0x1F00, 0x1FFF), _("Greek Extended")));
        ranges.push_back(std::make_pair(std::make_pair(0x2000, 0x206F), _("General Punctuation")));
        ranges.push_back(std::make_pair(std::make_pair(0x2070, 0x209F), _("Superscripts and Subscripts")));
        ranges.push_back(std::make_pair(std::make_pair(0x20A0, 0x20CF), _("Currency Symbols")));
        ranges.push_back(std::make_pair(std::make_pair(0x20D0, 0x20FF), _("Combining Diacritical Marks for Symbols")));
        ranges.push_back(std::make_pair(std::make_pair(0x2100, 0x214F), _("Letterlike Symbols")));
        ranges.push_back(std::make_pair(std::make_pair(0x2150, 0x218F), _("Number Forms")));
        ranges.push_back(std::make_pair(std::make_pair(0x2190, 0x21FF), _("Arrows")));
        ranges.push_back(std::make_pair(std::make_pair(0x2200, 0x22FF), _("Mathematical Operators")));
        ranges.push_back(std::make_pair(std::make_pair(0x2300, 0x23FF), _("Miscellaneous Technical")));
        ranges.push_back(std::make_pair(std::make_pair(0x2400, 0x243F), _("Control Pictures")));
        ranges.push_back(std::make_pair(std::make_pair(0x2440, 0x245F), _("Optical Character Recognition")));
        ranges.push_back(std::make_pair(std::make_pair(0x2460, 0x24FF), _("Enclosed Alphanumerics")));
        ranges.push_back(std::make_pair(std::make_pair(0x2500, 0x257F), _("Box Drawing")));
        ranges.push_back(std::make_pair(std::make_pair(0x2580, 0x259F), _("Block Elements")));
        ranges.push_back(std::make_pair(std::make_pair(0x25A0, 0x25FF), _("Geometric Shapes")));
        ranges.push_back(std::make_pair(std::make_pair(0x2600, 0x26FF), _("Miscellaneous Symbols")));
        ranges.push_back(std::make_pair(std::make_pair(0x2700, 0x27BF), _("Dingbats")));
        ranges.push_back(std::make_pair(std::make_pair(0x27C0, 0x27EF), _("Miscellaneous Mathematical Symbols-A")));
        ranges.push_back(std::make_pair(std::make_pair(0x27F0, 0x27FF), _("Supplemental Arrows-A")));
        ranges.push_back(std::make_pair(std::make_pair(0x2800, 0x28FF), _("Braille Patterns")));
        ranges.push_back(std::make_pair(std::make_pair(0x2900, 0x297F), _("Supplemental Arrows-B")));
        ranges.push_back(std::make_pair(std::make_pair(0x2980, 0x29FF), _("Miscellaneous Mathematical Symbols-B")));
        ranges.push_back(std::make_pair(std::make_pair(0x2A00, 0x2AFF), _("Supplemental Mathematical Operators")));
        ranges.push_back(std::make_pair(std::make_pair(0x2B00, 0x2BFF), _("Miscellaneous Symbols and Arrows")));
        ranges.push_back(std::make_pair(std::make_pair(0x2C00, 0x2C5F), _("Glagolitic")));
        ranges.push_back(std::make_pair(std::make_pair(0x2C60, 0x2C7F), _("Latin Extended-C")));
        ranges.push_back(std::make_pair(std::make_pair(0x2C80, 0x2CFF), _("Coptic")));
        ranges.push_back(std::make_pair(std::make_pair(0x2D00, 0x2D2F), _("Georgian Supplement")));
        ranges.push_back(std::make_pair(std::make_pair(0x2D30, 0x2D7F), _("Tifinagh")));
        ranges.push_back(std::make_pair(std::make_pair(0x2D80, 0x2DDF), _("Ethiopic Extended")));
        ranges.push_back(std::make_pair(std::make_pair(0x2DE0, 0x2DFF), _("Cyrillic Extended-A")));
        ranges.push_back(std::make_pair(std::make_pair(0x2E00, 0x2E7F), _("Supplemental Punctuation")));
        ranges.push_back(std::make_pair(std::make_pair(0x2E80, 0x2EFF), _("CJK Radicals Supplement")));
        ranges.push_back(std::make_pair(std::make_pair(0x2F00, 0x2FDF), _("Kangxi Radicals")));
        ranges.push_back(std::make_pair(std::make_pair(0x2FF0, 0x2FFF), _("Ideographic Description Characters")));
        ranges.push_back(std::make_pair(std::make_pair(0x3000, 0x303F), _("CJK Symbols and Punctuation")));
        ranges.push_back(std::make_pair(std::make_pair(0x3040, 0x309F), _("Hiragana")));
        ranges.push_back(std::make_pair(std::make_pair(0x30A0, 0x30FF), _("Katakana")));
        ranges.push_back(std::make_pair(std::make_pair(0x3100, 0x312F), _("Bopomofo")));
        ranges.push_back(std::make_pair(std::make_pair(0x3130, 0x318F), _("Hangul Compatibility Jamo")));
        ranges.push_back(std::make_pair(std::make_pair(0x3190, 0x319F), _("Kanbun")));
        ranges.push_back(std::make_pair(std::make_pair(0x31A0, 0x31BF), _("Bopomofo Extended")));
        ranges.push_back(std::make_pair(std::make_pair(0x31C0, 0x31EF), _("CJK Strokes")));
        ranges.push_back(std::make_pair(std::make_pair(0x31F0, 0x31FF), _("Katakana Phonetic Extensions")));
        ranges.push_back(std::make_pair(std::make_pair(0x3200, 0x32FF), _("Enclosed CJK Letters and Months")));
        ranges.push_back(std::make_pair(std::make_pair(0x3300, 0x33FF), _("CJK Compatibility")));
        ranges.push_back(std::make_pair(std::make_pair(0x3400, 0x4DBF), _("CJK Unified Ideographs Extension A")));
        ranges.push_back(std::make_pair(std::make_pair(0x4DC0, 0x4DFF), _("Yijing Hexagram Symbols")));
        ranges.push_back(std::make_pair(std::make_pair(0x4E00, 0x9FFF), _("CJK Unified Ideographs")));
        ranges.push_back(std::make_pair(std::make_pair(0xA000, 0xA48F), _("Yi Syllables")));
        ranges.push_back(std::make_pair(std::make_pair(0xA490, 0xA4CF), _("Yi Radicals")));
        ranges.push_back(std::make_pair(std::make_pair(0xA4D0, 0xA4FF), _("Lisu")));
        ranges.push_back(std::make_pair(std::make_pair(0xA500, 0xA63F), _("Vai")));
        ranges.push_back(std::make_pair(std::make_pair(0xA640, 0xA69F), _("Cyrillic Extended-B")));
        ranges.push_back(std::make_pair(std::make_pair(0xA6A0, 0xA6FF), _("Bamum")));
        ranges.push_back(std::make_pair(std::make_pair(0xA700, 0xA71F), _("Modifier Tone Letters")));
        ranges.push_back(std::make_pair(std::make_pair(0xA720, 0xA7FF), _("Latin Extended-D")));
        ranges.push_back(std::make_pair(std::make_pair(0xA800, 0xA82F), _("Syloti Nagri")));
        ranges.push_back(std::make_pair(std::make_pair(0xA830, 0xA83F), _("Common Indic Number Forms")));
        ranges.push_back(std::make_pair(std::make_pair(0xA840, 0xA87F), _("Phags-pa")));
        ranges.push_back(std::make_pair(std::make_pair(0xA880, 0xA8DF), _("Saurashtra")));
        ranges.push_back(std::make_pair(std::make_pair(0xA8E0, 0xA8FF), _("Devanagari Extended")));
        ranges.push_back(std::make_pair(std::make_pair(0xA900, 0xA92F), _("Kayah Li")));
        ranges.push_back(std::make_pair(std::make_pair(0xA930, 0xA95F), _("Rejang")));
        ranges.push_back(std::make_pair(std::make_pair(0xA960, 0xA97F), _("Hangul Jamo Extended-A")));
        ranges.push_back(std::make_pair(std::make_pair(0xA980, 0xA9DF), _("Javanese")));
        ranges.push_back(std::make_pair(std::make_pair(0xAA00, 0xAA5F), _("Cham")));
        ranges.push_back(std::make_pair(std::make_pair(0xAA60, 0xAA7F), _("Myanmar Extended-A")));
        ranges.push_back(std::make_pair(std::make_pair(0xAA80, 0xAADF), _("Tai Viet")));
        ranges.push_back(std::make_pair(std::make_pair(0xABC0, 0xABFF), _("Meetei Mayek")));
        ranges.push_back(std::make_pair(std::make_pair(0xAC00, 0xD7AF), _("Hangul Syllables")));
        ranges.push_back(std::make_pair(std::make_pair(0xD7B0, 0xD7FF), _("Hangul Jamo Extended-B")));
        ranges.push_back(std::make_pair(std::make_pair(0xD800, 0xDB7F), _("High Surrogates")));
        ranges.push_back(std::make_pair(std::make_pair(0xDB80, 0xDBFF), _("High Private Use Surrogates")));
        ranges.push_back(std::make_pair(std::make_pair(0xDC00, 0xDFFF), _("Low Surrogates")));
        ranges.push_back(std::make_pair(std::make_pair(0xE000, 0xF8FF), _("Private Use Area")));
        ranges.push_back(std::make_pair(std::make_pair(0xF900, 0xFAFF), _("CJK Compatibility Ideographs")));
        ranges.push_back(std::make_pair(std::make_pair(0xFB00, 0xFB4F), _("Alphabetic Presentation Forms")));
        ranges.push_back(std::make_pair(std::make_pair(0xFB50, 0xFDFF), _("Arabic Presentation Forms-A")));
        ranges.push_back(std::make_pair(std::make_pair(0xFE00, 0xFE0F), _("Variation Selectors")));
        ranges.push_back(std::make_pair(std::make_pair(0xFE10, 0xFE1F), _("Vertical Forms")));
        ranges.push_back(std::make_pair(std::make_pair(0xFE20, 0xFE2F), _("Combining Half Marks")));
        ranges.push_back(std::make_pair(std::make_pair(0xFE30, 0xFE4F), _("CJK Compatibility Forms")));
        ranges.push_back(std::make_pair(std::make_pair(0xFE50, 0xFE6F), _("Small Form Variants")));
        ranges.push_back(std::make_pair(std::make_pair(0xFE70, 0xFEFF), _("Arabic Presentation Forms-B")));
        ranges.push_back(std::make_pair(std::make_pair(0xFF00, 0xFFEF), _("Halfwidth and Fullwidth Forms")));
        ranges.push_back(std::make_pair(std::make_pair(0xFFF0, 0xFFFF), _("Specials")));
    }

    return ranges;
}

class GlyphColumns : public Gtk::TreeModel::ColumnRecord
{
public:
    Gtk::TreeModelColumn<gunichar> code;
    Gtk::TreeModelColumn<Glib::ustring> name;

    GlyphColumns()
    {
        add(code);
        add(name);
    }
};

GlyphColumns *GlyphsPanel::getColumns()
{
    static GlyphColumns *columns = new GlyphColumns();

    return columns;
}

/**
 * Constructor
 */
GlyphsPanel::GlyphsPanel(gchar const *prefsPath) :
    Inkscape::UI::Widget::Panel("", prefsPath, SP_VERB_DIALOG_GLYPHS, "", false),
    store(Gtk::ListStore::create(*getColumns())),
    iconView(0),
    entry(0),
    label(0),
    insertBtn(0),
    scriptCombo(0),
    fsel(0),
    targetDesktop(0),
    deskTrack(),
    instanceConns(),
    desktopConns()
{
#if WITH_GTKMM_3_0
    Gtk::Grid *table = new Gtk::Grid();
#else
    Gtk::Table *table = new Gtk::Table(3, 1, false);
#endif

    _getContents()->pack_start(*Gtk::manage(table), Gtk::PACK_EXPAND_WIDGET);
    guint row = 0;

// -------------------------------

    GtkWidget *fontsel = sp_font_selector_new();
    fsel = SP_FONT_SELECTOR(fontsel);
    sp_font_selector_set_fontspec( fsel, sp_font_selector_get_fontspec(fsel), 12.0 );

    gtk_widget_set_size_request (fontsel, 0, 150);
    g_signal_connect( G_OBJECT(fontsel), "font_set", G_CALLBACK(fontChangeCB), this );

#if WITH_GTKMM_3_0
    table->attach(*Gtk::manage(Glib::wrap(fontsel)), 0, row, 3, 1);
#else
    table->attach(*Gtk::manage(Glib::wrap(fontsel)),
                  0, 3, row, row + 1,
                  Gtk::SHRINK|Gtk::FILL, Gtk::SHRINK|Gtk::FILL);
#endif

    row++;


// -------------------------------

    {
        Gtk::Label *label = new Gtk::Label(_("Script: "));

#if WITH_GTKMM_3_0
        table->attach( *Gtk::manage(label), 0, row, 1, 1);
#else
        table->attach( *Gtk::manage(label),
                       0, 1, row, row + 1,
                       Gtk::SHRINK, Gtk::SHRINK);
#endif

        scriptCombo = new Gtk::ComboBoxText();
        for (std::map<GUnicodeScript, Glib::ustring>::iterator it = getScriptToName().begin(); it != getScriptToName().end(); ++it)
        {
            scriptCombo->append(it->second);
        }

        scriptCombo->set_active_text(getScriptToName()[G_UNICODE_SCRIPT_INVALID_CODE]);
        sigc::connection conn = scriptCombo->signal_changed().connect(sigc::mem_fun(*this, &GlyphsPanel::rebuild));
        instanceConns.push_back(conn);
        Gtk::Alignment *align = Gtk::manage(new Gtk::Alignment(Gtk::ALIGN_START, Gtk::ALIGN_START, 0.0, 0.0));
        align->add(*Gtk::manage(scriptCombo));

#if WITH_GTKMM_3_0
        align->set_hexpand();
        table->attach( *align, 1, row, 1, 1);
#else
        table->attach( *align,
                       1, 2, row, row + 1,
                       Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);
#endif
    }

    row++;

// -------------------------------

    {
        Gtk::Label *label = new Gtk::Label(_("Range: "));

#if WITH_GTKMM_3_0
        table->attach( *Gtk::manage(label), 0, row, 1, 1);
#else
        table->attach( *Gtk::manage(label),
                       0, 1, row, row + 1,
                       Gtk::SHRINK, Gtk::SHRINK);
#endif

        rangeCombo = new Gtk::ComboBoxText();
        for ( std::vector<NamedRange>::iterator it = getRanges().begin(); it != getRanges().end(); ++it ) {
            rangeCombo->append(it->second);
        }

        rangeCombo->set_active_text(getRanges()[1].second);
        sigc::connection conn = rangeCombo->signal_changed().connect(sigc::mem_fun(*this, &GlyphsPanel::rebuild));
        instanceConns.push_back(conn);
        Gtk::Alignment *align = new Gtk::Alignment(Gtk::ALIGN_START, Gtk::ALIGN_START, 0.0, 0.0);
        align->add(*Gtk::manage(rangeCombo));

#if WITH_GTKMM_3_0
        align->set_hexpand();
        table->attach( *Gtk::manage(align), 1, row, 1, 1);
#else
        table->attach( *Gtk::manage(align),
                       1, 2, row, row + 1,
                       Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);
#endif
    }

    row++;

// -------------------------------

    GlyphColumns *columns = getColumns();

    iconView = new Gtk::IconView(static_cast<Glib::RefPtr<Gtk::TreeModel> >(store));
    iconView->set_text_column(columns->name);
    //iconView->set_columns(16);

    sigc::connection conn;
    conn = iconView->signal_item_activated().connect(sigc::mem_fun(*this, &GlyphsPanel::glyphActivated));
    instanceConns.push_back(conn);
    conn = iconView->signal_selection_changed().connect(sigc::mem_fun(*this, &GlyphsPanel::glyphSelectionChanged));
    instanceConns.push_back(conn);


    Gtk::ScrolledWindow *scroller = new Gtk::ScrolledWindow();
    scroller->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
    scroller->add(*Gtk::manage(iconView));

#if WITH_GTKMM_3_0
    scroller->set_hexpand();
    scroller->set_vexpand();
    table->attach(*Gtk::manage(scroller), 0, row, 3, 1);
#else
    table->attach(*Gtk::manage(scroller),
                  0, 3, row, row + 1,
                  Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
#endif

    row++;

// -------------------------------

    Gtk::HBox *box = new Gtk::HBox();

    entry = new Gtk::Entry();
    conn = entry->signal_changed().connect(sigc::mem_fun(*this, &GlyphsPanel::calcCanInsert));
    instanceConns.push_back(conn);
    entry->set_width_chars(18);
    box->pack_start(*Gtk::manage(entry), Gtk::PACK_SHRINK);

    Gtk::Label *pad = new Gtk::Label("    ");
    box->pack_start(*Gtk::manage(pad), Gtk::PACK_SHRINK);

    label = new Gtk::Label("      ");
    box->pack_start(*Gtk::manage(label), Gtk::PACK_SHRINK);

    pad = new Gtk::Label("");
    box->pack_start(*Gtk::manage(pad), Gtk::PACK_EXPAND_WIDGET);

    insertBtn = new Gtk::Button(_("Append"));
    conn = insertBtn->signal_clicked().connect(sigc::mem_fun(*this, &GlyphsPanel::insertText));
    instanceConns.push_back(conn);
    insertBtn->set_can_default();
    insertBtn->set_sensitive(false);

    box->pack_end(*Gtk::manage(insertBtn), Gtk::PACK_SHRINK);

#if WITH_GTKMM_3_0
    box->set_hexpand();
    table->attach( *Gtk::manage(box), 0, row, 3, 1);
#else
    table->attach( *Gtk::manage(box),
                   0, 3, row, row + 1,
                   Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
#endif

    row++;

// -------------------------------


    show_all_children();

    restorePanelPrefs();

    // Connect this up last
    conn = deskTrack.connectDesktopChanged( sigc::mem_fun(*this, &GlyphsPanel::setTargetDesktop) );
    instanceConns.push_back(conn);
    deskTrack.connect(GTK_WIDGET(gobj()));
}

GlyphsPanel::~GlyphsPanel()
{
    for (std::vector<sigc::connection>::iterator it =  instanceConns.begin(); it != instanceConns.end(); ++it) {
        it->disconnect();
    }
    instanceConns.clear();
    for (std::vector<sigc::connection>::iterator it = desktopConns.begin(); it != desktopConns.end(); ++it) {
        it->disconnect();
    }
    desktopConns.clear();
}


void GlyphsPanel::setDesktop(SPDesktop *desktop)
{
    Panel::setDesktop(desktop);
    deskTrack.setBase(desktop);
}

void GlyphsPanel::setTargetDesktop(SPDesktop *desktop)
{
    if (targetDesktop != desktop) {
        if (targetDesktop) {
            for (std::vector<sigc::connection>::iterator it = desktopConns.begin(); it != desktopConns.end(); ++it) {
                it->disconnect();
            }
            desktopConns.clear();
        }

        targetDesktop = desktop;

        if (targetDesktop && targetDesktop->selection) {
            sigc::connection conn = desktop->selection->connectChanged(sigc::hide(sigc::bind(sigc::mem_fun(*this, &GlyphsPanel::readSelection), true, true)));
            desktopConns.push_back(conn);

            // Text selection within selected items has changed:
            conn = desktop->connectToolSubselectionChanged(sigc::hide(sigc::bind(sigc::mem_fun(*this, &GlyphsPanel::readSelection), true, false)));
            desktopConns.push_back(conn);

            // Must check flags, so can't call performUpdate() directly.
            conn = desktop->selection->connectModified(sigc::hide<0>(sigc::mem_fun(*this, &GlyphsPanel::selectionModifiedCB)));
            desktopConns.push_back(conn);

            readSelection(true, true);
        }
    }
}

// Append selected glyphs to selected text
void GlyphsPanel::insertText()
{
    SPItem *textItem = 0;
    std::vector<SPItem*> itemlist=targetDesktop->selection->itemList();
        for(std::vector<SPItem*>::const_iterator i=itemlist.begin(); itemlist.end() != i; ++i) {
            if (SP_IS_TEXT(*i) || SP_IS_FLOWTEXT(*i)) {
            textItem = *i;
            break;
        }
    }

    if (textItem) {
        Glib::ustring glyphs;
        if (entry->get_text_length() > 0) {
            glyphs = entry->get_text();
        } else {

#if WITH_GTKMM_3_0
            std::vector<Gtk::TreePath> itemArray = iconView->get_selected_items();
#else
            Gtk::IconView::ArrayHandle_TreePaths itemArray = iconView->get_selected_items();
#endif

            if (!itemArray.empty()) {
                Gtk::TreeModel::Path const & path = *itemArray.begin();
                Gtk::ListStore::iterator row = store->get_iter(path);
                gunichar ch = (*row)[getColumns()->code];
                glyphs = ch;
            }
        }

        if (!glyphs.empty()) {
            Glib::ustring combined;
            gchar *str = sp_te_get_string_multiline(textItem);
            if (str) {
                combined = str;
                g_free(str);
                str = 0;
            }
            combined += glyphs;
            sp_te_set_repr_text_multiline(textItem, combined.c_str());
            DocumentUndo::done(targetDesktop->doc(), SP_VERB_CONTEXT_TEXT, _("Append text"));
        }
    }
}

void GlyphsPanel::glyphActivated(Gtk::TreeModel::Path const & path)
{
    Gtk::ListStore::iterator row = store->get_iter(path);
    gunichar ch = (*row)[getColumns()->code];
    Glib::ustring tmp;
    tmp += ch;

    int startPos = 0;
    int endPos = 0;
    if (entry->get_selection_bounds(startPos, endPos)) {
        // there was something selected.
        entry->delete_text(startPos, endPos);
    }
    startPos = entry->get_position();
    entry->insert_text(tmp, -1, startPos);
    entry->set_position(startPos);
}

void GlyphsPanel::glyphSelectionChanged()
{
#if WITH_GTKMM_3_0
    std::vector<Gtk::TreePath> itemArray = iconView->get_selected_items();
#else
    Gtk::IconView::ArrayHandle_TreePaths itemArray = iconView->get_selected_items();
#endif

    if (itemArray.empty()) {
        label->set_text("      ");
    } else {
        Gtk::TreeModel::Path const & path = *itemArray.begin();
        Gtk::ListStore::iterator row = store->get_iter(path);
        gunichar ch = (*row)[getColumns()->code];


        Glib::ustring scriptName;
        GUnicodeScript script = g_unichar_get_script(ch);
        std::map<GUnicodeScript, Glib::ustring> mappings = getScriptToName();
        if (mappings.find(script) != mappings.end()) {
            scriptName = mappings[script];
        }
        gchar * tmp = g_strdup_printf("U+%04X %s", ch, scriptName.c_str());
        label->set_text(tmp);
    }
    calcCanInsert();
}

void GlyphsPanel::fontChangeCB(SPFontSelector * /*fontsel*/, Glib::ustring /*fontspec*/, GlyphsPanel *self)
{
    if (self) {
        self->rebuild();
    }
}

void GlyphsPanel::selectionModifiedCB(guint flags)
{
    bool style = ((flags & ( SP_OBJECT_CHILD_MODIFIED_FLAG |
                             SP_OBJECT_STYLE_MODIFIED_FLAG  )) != 0 );

    bool content = ((flags & ( SP_OBJECT_CHILD_MODIFIED_FLAG |
                               SP_TEXT_CONTENT_MODIFIED_FLAG  )) != 0 );

    readSelection(style, content);
}

void GlyphsPanel::calcCanInsert()
{
    int items = 0;
    std::vector<SPItem*> itemlist=targetDesktop->selection->itemList();
    for(std::vector<SPItem*>::const_iterator i=itemlist.begin(); itemlist.end() != i; ++i) {
        if (SP_IS_TEXT(*i) || SP_IS_FLOWTEXT(*i)) {
            ++items;
        }
    }

    bool enable = (items == 1);
    if (enable) {
        enable &= (!iconView->get_selected_items().empty()
                   || (entry->get_text_length() > 0));
    }

    if (enable != insertBtn->is_sensitive()) {
        insertBtn->set_sensitive(enable);
    }
}

void GlyphsPanel::readSelection( bool updateStyle, bool /*updateContent*/ )
{
    calcCanInsert();

    if (targetDesktop && updateStyle) {
        //SPStyle query(SP_ACTIVE_DOCUMENT);

        //int result_family = sp_desktop_query_style(targetDesktop, &query, QUERY_STYLE_PROPERTY_FONTFAMILY);
        //int result_style = sp_desktop_query_style(targetDesktop, &query, QUERY_STYLE_PROPERTY_FONTSTYLE);
        //int result_numbers = sp_desktop_query_style(targetDesktop, &query, QUERY_STYLE_PROPERTY_FONTNUMBERS);

    }
}


void GlyphsPanel::rebuild()
{
    Glib::ustring fontspec = fsel ? sp_font_selector_get_fontspec(fsel) : "";

    font_instance* font = 0;
    if( !fontspec.empty() ) {
        font = font_factory::Default()->FaceFromFontSpecification( fontspec.c_str() );
    }

    if (font) {
        //double  sp_font_selector_get_size (SPFontSelector *fsel);

        GUnicodeScript script = G_UNICODE_SCRIPT_INVALID_CODE;
        Glib::ustring scriptName = scriptCombo->get_active_text();
        std::map<GUnicodeScript, Glib::ustring> items = getScriptToName();
        for (std::map<GUnicodeScript, Glib::ustring>::iterator it = items.begin(); it != items.end(); ++it) {
            if (scriptName == it->second) {
                script = it->first;
                break;
            }
        }

        // Disconnect the model while we update it. Simple work-around for 5x+ performance boost.
        Glib::RefPtr<Gtk::ListStore> tmp = Gtk::ListStore::create(*getColumns());
        iconView->set_model(tmp);

        gunichar lower = 0x0001;
        gunichar upper = 0xFFFD;
        int active = rangeCombo->get_active_row_number();
        if (active >= 0) {
            lower = getRanges()[active].first.first;
            upper = getRanges()[active].first.second;
        }
        std::vector<gunichar> present;
        for (gunichar ch = lower; ch <= upper; ch++) {
            int glyphId = font->MapUnicodeChar(ch);
            if (glyphId > 0) {
                if ((script == G_UNICODE_SCRIPT_INVALID_CODE) || (script == g_unichar_get_script(ch))) {
                    present.push_back(ch);
                }
            }
        }

        GlyphColumns *columns = getColumns();
        store->clear();
        for (std::vector<gunichar>::iterator it = present.begin(); it != present.end(); ++it)
        {
            Gtk::ListStore::iterator row = store->append();
            Glib::ustring tmp;
            tmp += *it;
            (*row)[columns->code] = *it;
            (*row)[columns->name] = tmp;
        }

        // Reconnect the model once it has been updated:
        iconView->set_model(store);
    }
}


} // namespace Dialogs
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
