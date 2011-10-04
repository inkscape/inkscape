/*
 *  TextWrapper.h
 *  testICU
 *
 */

#ifndef my_text_wrapper
#define my_text_wrapper

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pango/pango.h>

#include "libnrtype/boundary-type.h"

// miscanellous but useful data for a given text: chunking into logical pieces
// pieces include sentence/word, needed for example for the word-spacing property,
// and more important stuff like letter (ie visual letters)

struct text_boundary;
struct one_glyph;
struct one_box;
struct one_para;
class font_instance;

class text_wrapper {
public:
    char *utf8_text;  // source text
    gunichar *uni32_text; // ucs4 text computed from utf8_text
    one_glyph *glyph_text; // glyph string computed for uni32_text

    // maps between the 2
    // These should most definitely be size_t, not int.
    // I am quite sure (but not bored enough to actually test it
    // on a 500MHz machine with 256MB RAM ) that this will crash
    // for text longer than 2GB on architectures where
    // sizeof(size_t) != sizeof(int)
    int utf8_length; // utf8_text length
    int uni32_length; // uni32_text length
    int glyph_length; /**< Number of glyph in the glyph_text array.
                       * The size of the array is (glyph_length+1) in fact; the last glyph is kind of a '0' char. */
    int *uni32_codepoint; // uni32_codepoint[i] is the index in uni32_text corresponding to utf8_text[i]
    int *utf8_codepoint;  // utf8_codepoint[i] is the index in utf8_text of the beginning of uni32_text[i]

    // layout
    font_instance *default_font; // font set as the default font (would need at least one alternate per language)
    PangoLayout *pLayout;      // private structure

    // kerning additions
    int last_addition; // index in uni32_text of the beginning of the text added by the last AppendUTF8 call
    double *kern_x;        // dx[i] is the dx for the ith unicode char
    double *kern_y;

    // boundaries, in an array
    unsigned nbBound, maxBound;
    text_boundary *bounds;

    // text organization
    int nbBox, maxBox;
    one_box *boxes;
    int nbPara, maxPara;
    one_para *paras;

    text_wrapper(void);
    virtual ~text_wrapper(void);

    // filling the structure with input data
    void SetDefaultFont(font_instance *iFont);

    /**
     * Append the specified text to utf8_text and uni32_codepoint.
     *
     * Note: Despite the name, the current implementation is primarily suited for a single
     * call to set the text, rather than repeated calls to AppendUTF8: the implementation is
     * Omega(n) in the new total length of the string, rather than just in the length of the
     * text being appended.  This can probably be addressed fairly easily (see comments in
     * code) if this is an issue for new callers.
     *
     * \pre text is valid UTF-8, or null.
     *      Formally: text==NULL || g_utf8_validate(text, len, NULL).
     *
     * \param len Our sole existing caller (widgets/font_selector.cpp) uses len=-1.  N.B. The current
     *   implementation may be buggy for non-negative len, especially for len==0.
     */
    void AppendUTF8(char const *text, int len);

    // adds dx or dy for the text added by the last AppendUTF8() call
    void KernXForLastAddition(double *i_kern_x, int i_len, double scale = 1.0);
    void KernYForLastAddition(double *i_kern_y, int i_len, double scale = 1.0);
    void KernXForLastAddition(GList *i_kern_x, double scale = 1.0);
    void KernYForLastAddition(GList *i_kern_y, double scale = 1.0);
    // compute the layout and stuff
    void DoLayout(void);
    // semi-private: computes boundaries in the input text
    void ChunkText(void);
    // utility function to move to the next element
    bool NextChar(int &st, int &en) const;
    bool NextWord(int &st, int &en) const;
    bool NextPara(int &st, int &en) const;

    // post-processing after the initial layout
    // for the xml-space property: merges consecutive whitespace, and eats leading whitespace in the text
    void MergeWhiteSpace(void);
    // makes vertical 'x' and 'y' fields in the glyph_text based on the computed positions
    void MakeVertical(void);
    // as the names says...
    void AddLetterSpacing(double dx, double dy, int g_st = -1, int g_en = -1);
    // adds the kerning specified by the KernXForLastAddition call to the layout
    void AddDxDy(void);

    // boundary handling
private:
    unsigned AddBoundary(text_boundary const &ib);
public:
    void AddTwinBoundaries(text_boundary const &is, text_boundary const &ie);
    void SortBoundaries(void);
    void MakeTextBoundaries(PangoLogAttr *pAttrs, int nAttr);
    //bool Contains(BoundaryType type, int g_st, int g_en, int &c_st, int &c_en);
    bool IsBound(BoundaryType type, int g_st, int &c_st);

    void MeasureBoxes(void);
    int NbLetter(int g_st, int g_en);
};

#endif


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
