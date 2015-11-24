/*
 *  TextWrapper.cpp
 *  testICU
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "TextWrapper.h"

#include <libnrtype/font-instance.h>
#include "libnrtype/text-boundary.h"
#include "libnrtype/one-glyph.h"
#include "libnrtype/one-box.h"
#include "libnrtype/one-para.h"

#include <svg/svg.h>

text_wrapper::text_wrapper(void)
{
    // voids everything
    utf8_text = NULL;
    uni32_text = NULL;
    glyph_text = NULL;
    utf8_length = 0;
    uni32_length = 0;
    glyph_length = 0;
    utf8_codepoint = NULL;
    uni32_codepoint = NULL;
    default_font = NULL;
    bounds = NULL;
    nbBound = maxBound = 0;
    boxes = NULL;
    nbBox = maxBox = 0;
    paras = NULL;
    nbPara = maxPara = 0;
    kern_x = kern_y = NULL;
    last_addition = -1;
    // inits the pangolayout with default params
    font_factory *font_src = font_factory::Default();
    pLayout = pango_layout_new(font_src->fontContext);
    pango_layout_set_single_paragraph_mode(pLayout, true);
    pango_layout_set_width(pLayout, -1);
}
text_wrapper::~text_wrapper(void)
{
    // frees everything
    //printf("delete\n");
    g_object_unref(pLayout);
    if ( utf8_text ) free(utf8_text);
    if ( uni32_text ) free(uni32_text);
    if ( glyph_text ) free(glyph_text);
    if ( utf8_codepoint ) free(utf8_codepoint);
    if ( uni32_codepoint ) free(uni32_codepoint);
    if ( default_font ) default_font->Unref();
    if ( boxes ) free(boxes);
    if ( paras ) free(paras);
    if ( kern_x ) free(kern_x);
    if ( kern_y ) free(kern_y);
    for (unsigned i = 0; i < nbBound; i++) {
        switch ( bounds[i].type ) {
            default:
                break;
        }
    }
    if ( bounds ) free(bounds);
    default_font = NULL;

}

void text_wrapper::SetDefaultFont(font_instance *iFont)
{
    // refcounts the font for our internal uses
    if ( iFont ) iFont->Ref();
    if ( default_font ) default_font->Unref();
    default_font = iFont;
}

void text_wrapper::AppendUTF8(char const *text, int len)
{
    // appends text to what needs to be handled
    if ( utf8_length <= 0 ) {
        // a first check to prevent the text from containing a leading line return (which
        // is probably a bug anyway)
        if ( text[0] == '\n' || text[0] == '\r' ) {
            /* fixme: Should the below be `0 <= len' ?  The existing code looks wrong
             * for the case that len==0.
             * TODO: Document the meaning of the len parameter. */
            if ( len > 0 ) {
                while ( len > 0 && ( *text == '\n' || *text == '\r' ) ) {text++; len--;}
            } else {
                while ( *text == '\n' || *text == '\r' ) text++;
            }
        }
    }
    if ( len == 0 || text == NULL || *text == 0 ) return;
    g_return_if_fail(g_utf8_validate(text, len, NULL));

    // compute the length
    int const nlen = ( len < 0
                       ? strlen(text)
                       : len );
    /* effic: Use g_utf8_validate's last param to do this. */

    // prepare to store the additional text
    /* effic: (Not an issue for the sole caller at the time of writing.)  This implementation
       takes quadratic time if the text is composed of n appends.  Use a proper data structure.
       STL vector would suffice. */
    char *newdata = static_cast<char*>(realloc(utf8_text, (utf8_length + nlen + 1) * sizeof(char)));
    if (newdata != NULL)
    {
        utf8_text = newdata;
    }
    else
    {
        g_warning("Failed to reallocate utf8_text");
    }
    int* newdata2 = static_cast<int*>(realloc(uni32_codepoint, (utf8_length + nlen + 1) * sizeof(int)));
    if (newdata2 != NULL)
    {
        uni32_codepoint = newdata2;
    }
    else
    {
        g_warning("Failed to reallocate uni32_codepoint");
    }

    // copy the source text in the newly lengthened array
    memcpy(utf8_text + utf8_length, text, nlen * sizeof(char));
    utf8_length += nlen;
    utf8_text[utf8_length] = 0;
    // remember where the text ended, before we recompute it, for the dx/dy we'll add after that (if any)
    last_addition = uni32_length;
    // free old uni32 structures (instead of incrementally putting the text)
    if ( uni32_text ) free(uni32_text);
    if ( utf8_codepoint ) free(utf8_codepoint);
    uni32_text = NULL;
    utf8_codepoint = NULL;
    uni32_length = 0;
    {
        // recompute length of uni32 text
        char *p = utf8_text;
        while ( *p ) {
            p = g_utf8_next_char(p); // since we validated the input text, we can use this 'fast' macro
            uni32_length++;
        }
    }
    // realloc the arrays
    uni32_text = (gunichar*)malloc((uni32_length + 1) * sizeof(gunichar));
    utf8_codepoint = (int*)malloc((uni32_length + 1) * sizeof(int));
    {
        // read the utf8 string and compute codepoints positions
        char *p = utf8_text;
        int i = 0;
        int l_o = 0;
        while ( *p ) {
            // get the new codepoint
            uni32_text[i] = g_utf8_get_char(p);
            // compute the offset in the utf8_string
            unsigned int n_o = (unsigned int)(p - utf8_text);
            // record the codepoint's start
            utf8_codepoint[i] = n_o;
            // record the codepoint's correspondance in the utf8 string
            for (unsigned int j = l_o; j < n_o; j++) uni32_codepoint[j] = i - 1;
            // and move on
            l_o = n_o;
            p = g_utf8_next_char(p);
            i++;
        }
        // the termination of the loop
        for (int j = l_o; j < utf8_length; j++) uni32_codepoint[j] = uni32_length - 1;
        uni32_codepoint[utf8_length] = uni32_length;
        uni32_text[uni32_length] = 0;
        utf8_codepoint[uni32_length] = utf8_length;
    }
    // if needed, fill the dx/dy arrays with 0 for the newly created part
    // these will be filled by a KernXForLastAddition() right after this function
    // note that the SVG spec doesn't require you to give a dx for each codepoint,
    // so setting the dx to 0 is mandatory
    if ( uni32_length > last_addition ) {
        if ( kern_x ) {
            double *newdata = static_cast<double*>(realloc(kern_x, (uni32_length + 1) * sizeof(double)));
            if (newdata != NULL)
            {
                kern_x = newdata;
            }
            else
            {
                g_warning("Failed to reallocate kern_x");
            }
            for (int i = last_addition; i <= uni32_length; i++) kern_x[i] = 0;
        }
        if ( kern_y ) {
            double *newdata = static_cast<double*>(realloc(kern_y, (uni32_length + 1) * sizeof(double)));
            if (newdata != NULL)
            {
                kern_y = newdata;
            }
            else
            {
                g_warning("Failed to reallocate kern_y");
            }
            for (int i = last_addition; i <= uni32_length; i++) kern_y[i] = 0;
        }
    }
}

void text_wrapper::DoLayout(void)
{
    // THE function
    // first some sanity checks
    if ( default_font == NULL ) return;
    if ( uni32_length <= 0 || utf8_length <= 0 ) return;
    // prepare the pangolayout object
    {
        //char *tc = pango_font_description_to_string(default_font->descr);
        //printf("layout with %s\n", tc);
        //free(tc);
    }
    pango_layout_set_font_description(pLayout, default_font->descr);
    pango_layout_set_text(pLayout, utf8_text, utf8_length);
    // reset the glyph string
    if ( glyph_text ) free(glyph_text);
    glyph_text = NULL;
    glyph_length = 0;

    double pango_to_ink = (1.0 / ((double)PANGO_SCALE)); // utility
    int max_g = 0;
    PangoLayoutIter *pIter = pango_layout_get_iter(pLayout); // and go!
    do {
        PangoLayoutLine *pLine = pango_layout_iter_get_line(pIter); // no need for unref
        int plOffset = pLine->start_index; // start of the line in the uni32_text
        PangoRectangle ink_r, log_r;
        pango_layout_iter_get_line_extents(pIter, &ink_r, &log_r);
        double plY = (1.0 / ((double)PANGO_SCALE)) * ((double)log_r.y); // start position of this line of the layout
        double plX = (1.0 / ((double)PANGO_SCALE)) * ((double)log_r.x);
        GSList *curR = pLine->runs; // get ready to iterate over the runs of this line
        while ( curR ) {
            PangoLayoutRun *pRun = (PangoLayoutRun*)curR->data;
            if ( pRun ) {
                int prOffset = pRun->item->offset; // start of the run in the line
                // a run has uniform font/directionality/etc...
                int o_g_l = glyph_length; // save the index of the first glyph we'll add
                for (int i = 0; i < pRun->glyphs->num_glyphs; i++) { // add glyph sequentially, reading them from the run
                    // realloc the structures
                    if ( glyph_length >= max_g ) {
                        max_g = 2 * glyph_length + 1;
                        one_glyph *newdata = static_cast<one_glyph*>(realloc(glyph_text, (max_g + 1) * sizeof(one_glyph)));
                        if (newdata != NULL)
                        {
                            glyph_text = newdata;
                        }
                        else
                        {
                            g_warning("Failed to reallocate glyph_text");
                        }
                    }
                    // fill the glyph info
                    glyph_text[glyph_length].font = pRun->item->analysis.font;
                    glyph_text[glyph_length].gl = pRun->glyphs->glyphs[i].glyph;
                    glyph_text[glyph_length].uni_st = plOffset + prOffset + pRun->glyphs->log_clusters[i];
                    // depending on the directionality, the last uni32 codepoint for this glyph is the first of the next char
                    // or the first of the previous
                    if ( pRun->item->analysis.level == 1 ) {
                        // rtl
                        if ( i < pRun->glyphs->num_glyphs - 1 ) {
                            glyph_text[glyph_length + 1].uni_en = glyph_text[glyph_length].uni_st;
                        }
                        glyph_text[glyph_length].uni_dir = 1;
                        glyph_text[glyph_length + 1].uni_dir = 1; // set the directionality for the next too, so that the last glyph in
                        // the array has the correct direction
                    } else {
                        // ltr
                        if ( i > 0 ) {
                            glyph_text[glyph_length - 1].uni_en = glyph_text[glyph_length].uni_st;
                        }
                        glyph_text[glyph_length].uni_dir = 0;
                        glyph_text[glyph_length + 1].uni_dir = 0;
                    }
                    // set the position
                    // the layout is an infinite line
                    glyph_text[glyph_length].x = plX + pango_to_ink * ((double)pRun->glyphs->glyphs[i].geometry.x_offset);
                    glyph_text[glyph_length].y = plY + pango_to_ink * ((double)pRun->glyphs->glyphs[i].geometry.y_offset);
                    // advance to the next glyph
                    plX += pango_to_ink * ((double)pRun->glyphs->glyphs[i].geometry.width);
                    // and set the next glyph's position, in case it's the terminating glyph
                    glyph_text[glyph_length + 1].x = plX;
                    glyph_text[glyph_length + 1].y = plY;
                    glyph_length++;
                }
                // and finish filling the info
                // notably, the uni_en of the last char in ltr text and the uni_en of the first in rtl are still not set
                if ( pRun->item->analysis.level == 1 ) {
                    // rtl
                    if ( glyph_length > o_g_l ) glyph_text[o_g_l].uni_en = plOffset + prOffset + pRun->item->length;
                } else {
                    if ( glyph_length > 0 ) glyph_text[glyph_length - 1].uni_en = plOffset + prOffset + pRun->item->length;
                }
                // the terminating glyph has glyph_id=0 because it means 'no glyph'
                glyph_text[glyph_length].gl = 0;
                // and is associated with no text (but you cannot set uni_st=uni_en=0, because the termination
                // is expected to be the glyph for the termination of the uni32_text)
                glyph_text[glyph_length].uni_st = glyph_text[glyph_length].uni_en = plOffset + prOffset + pRun->item->length;
            }
            curR = curR->next;
        }
    } while ( pango_layout_iter_next_line(pIter) );
    pango_layout_iter_free(pIter);

    // grunt work done. now some additional info for layout: computing letters, mostly (one letter = several glyphs sometimes)
    PangoLogAttr *pAttrs = NULL;
    int nbAttr = 0;
    // get the layout attrs, they hold the boundaries pango computed
    pango_layout_get_log_attrs(pLayout, &pAttrs, &nbAttr);
    // feed to MakeTextBoundaries which knows what to do with these
    MakeTextBoundaries(pAttrs, nbAttr);
    // the array of boundaries is full, but out-of-order
    SortBoundaries();
    // boundary array is ready to be used, call chunktext to fill the *_start fields of the glyphs, and compute
    // the boxed version of the text for sp-typeset
    ChunkText();
    // get rid of the attributes
    if ( pAttrs ) g_free(pAttrs);

    // cleaning up
    for (int i = 0; i < glyph_length; i++) {
        glyph_text[i].uni_st = uni32_codepoint[glyph_text[i].uni_st];
        glyph_text[i].uni_en = uni32_codepoint[glyph_text[i].uni_en];
        glyph_text[i].x /= 512; // why is this not default_font->parent->fontsize?
        glyph_text[i].y /= 512;
    }
    if ( glyph_length > 0 ) {
        glyph_text[glyph_length].x /= 512;
        glyph_text[glyph_length].y /= 512;
    }
}

void text_wrapper::ChunkText(void)
{
    int c_st = -1, c_en = -1;
    for (int i = 0; i < glyph_length; i++) {
        int g_st = glyph_text[i].uni_st, g_en = glyph_text[i].uni_en;
        glyph_text[i].char_start = false;
        glyph_text[i].word_start = false;
        glyph_text[i].para_start = false;
        // boundaries depend on the directionality
        // letter boundaries correspond to the glyphs starting one letter when you read them left to right (always)
        // because that's the order they are stored into in the glyph_text array
        if ( glyph_text[i].uni_dir == 0 ) {
            if ( IsBound(bnd_char, g_st, c_st) ) { // check if there is a charcater (=letter in pango speak) at this position
                // can be a 'start' boundary or a 'end' boundary, doesn't matter, as long
                // as you get from one letter to the next at this position
                if ( g_st == bounds[c_st].uni_pos ) glyph_text[i].char_start = true;
            }
            if ( IsBound(bnd_word, g_st, c_st) ) {
                if ( g_st == bounds[c_st].uni_pos ) glyph_text[i].word_start = true;
            }
            if ( IsBound(bnd_para, g_st, c_st) ) {
                if ( g_st == bounds[c_st].uni_pos ) glyph_text[i].para_start = true;
            }
        } else {
            if ( IsBound(bnd_char, g_en, c_en) ) {
                if ( g_en == bounds[c_en].uni_pos ) glyph_text[i].char_start = true;
            }
            if ( IsBound(bnd_word, g_en, c_en) ) {
                if ( g_en == bounds[c_en].uni_pos ) glyph_text[i].word_start = true;
            }
            if ( IsBound(bnd_para, g_en, c_en) ) {
                if ( g_en == bounds[c_en].uni_pos ) glyph_text[i].para_start = true;
            }
        }
    }

    if ( glyph_length > 0 ) {
        glyph_text[glyph_length].char_start = true;
        glyph_text[glyph_length].word_start = true;
        glyph_text[glyph_length].para_start = true;
    }
    {
        // doing little boxes
        int g_st = -1, g_en = -1;
        while ( NextWord(g_st, g_en) ) {
            // check uniformity of fonts
            if ( g_st < g_en ) {
                int n_st = g_st;
                int n_en = g_st;
                bool first = true;
                do {
                    n_st = n_en;
                    PangoFont *curPF = glyph_text[n_st].font;
                    do {
                        n_en++;
                    } while ( n_en < g_en && glyph_text[n_en].font == curPF );
                    if ( nbBox >= maxBox ) {
                        maxBox = 2 * nbBox + 1;
                        one_box *newdata = static_cast<one_box*>(realloc(boxes, maxBox * sizeof(one_box)));
                        if (newdata != NULL)
                        {
                            boxes = newdata;
                        }
                        else
                        {
                            g_warning("Failed to reallocate boxes");
                        }
                    }
                    boxes[nbBox].g_st = n_st;
                    boxes[nbBox].g_en = n_en;
                    boxes[nbBox].word_start = first;
                    boxes[nbBox].word_end = (n_en >= g_en);
                    nbBox++;
                    first = false;
                } while ( n_en < g_en );
            }
        }
    }
    {
        // doing little paras
        int g_st = -1, g_en = -1;
        while ( NextPara(g_st, g_en) ) {
            int b_st = 0;
            while ( b_st < nbBox && boxes[b_st].g_st < g_st ) b_st++;
            if ( b_st < nbBox && boxes[b_st].g_st == g_st ) {
                int b_en = b_st;
                while ( b_en < nbBox && boxes[b_en].g_en < g_en ) b_en++;
                if ( b_en < nbBox && boxes[b_en].g_en == g_en ) {
                    if ( nbPara >= maxPara ) {
                        maxPara = 2 * nbPara + 1;
                        one_para *newdata = static_cast<one_para*>(realloc(paras, maxPara * sizeof(one_para)));
                        if (newdata != NULL)
                        {
                            paras = newdata;
                        }
                        else
                        {
                            g_warning("Failed to reallocate paras");
                        }
                    }
                    paras[nbPara].b_st = b_st;
                    paras[nbPara].b_en = b_en;
                    nbPara++;
                }
            }
        }
    }
}

void text_wrapper::MakeVertical(void)
{
    if ( glyph_length <= 0 ) return;
    font_factory *font_src = font_factory::Default();

    // explanation: when laying out text vertically, you must keep the glyphs of a single letter together
    double baseY = glyph_text[0].y;
    double lastY = baseY;
    int g_st = 0, g_en = 0;
    int nbLetter = 0;
    PangoFont *curPF = NULL;
    font_instance *curF = NULL;
    do {
        // move to the next letter boundary
        g_st = g_en;
        do {
            g_en++;
        } while ( g_en < glyph_length && glyph_text[g_en].char_start == false );
        // got a letter
        if ( g_st < g_en && g_en <= glyph_length ) {
            // we need to compute the letter's width (in case sometimes we implement the flushleft and flushright)
            // and the total height for this letter. for example accents usually have 0 width, so this is not
            // stupid
            double n_adv = 0;
            double minX = glyph_text[g_st].x, maxX = glyph_text[g_st].x;
            for (int i = g_st; i < g_en; i++) {
                if ( glyph_text[i].font != curPF ) { // font is not the same as the one of the previous glyph
                    // so we need to update curF
                    if ( curF ) curF->Unref();
                    curF = NULL;
                    curPF = glyph_text[i].font;
                    if ( curPF ) {
                        PangoFontDescription *pfd = pango_font_describe(curPF);
                        curF = font_src->Face(pfd);
                        pango_font_description_free(pfd);
                    }
                }
                double x = ( curF
                             ? curF->Advance(glyph_text[i].gl, true)
                             : 0 );
                if ( x > n_adv ) n_adv = x;
                if ( glyph_text[i].x < minX ) minX = glyph_text[i].x;
                if ( glyph_text[i].x > maxX ) maxX = glyph_text[i].x;
            }
            lastY += n_adv;
            // and put the glyphs of this letter at their new position
            for (int i = g_st; i < g_en; i++) {
                glyph_text[i].x -= minX;
                glyph_text[i].y += lastY;
            }
            g_st = g_en;
        }
        nbLetter++;
    } while ( g_st < glyph_length );
    if ( curF ) curF->Unref();
}

void text_wrapper::MergeWhiteSpace(void)
{
    if ( glyph_length <= 0 ) return;
    // scans the glyphs and shifts them accordingly
    double delta_x = 0, delta_y = 0;
    bool inWhite = true;
    int wpos = 0, rpos = 0; // wpos is the position where we read glyphs, rpos is the position where we write them back
    // since we only eat whitespace, wpos <= rpos
    for (rpos = 0; rpos < glyph_length; rpos++) {
        // copy the glyph at its new position
        glyph_text[wpos].gl = glyph_text[rpos].gl;
        glyph_text[wpos].uni_st = glyph_text[rpos].uni_st;
        glyph_text[wpos].uni_en = glyph_text[rpos].uni_en;
        glyph_text[wpos].font = glyph_text[rpos].font;
        glyph_text[wpos].x = glyph_text[rpos].x - delta_x;
        glyph_text[wpos].y = glyph_text[rpos].y - delta_y;
        wpos++; // move the write position
        if ( g_unichar_isspace(uni32_text[glyph_text[rpos].uni_st]) ) {
            if ( inWhite ) {
                // eat me: 2 steps: first add the shift in position to the cumulated shift
                delta_x += glyph_text[rpos + 1].x - glyph_text[rpos].x;
                delta_y += glyph_text[rpos + 1].y - glyph_text[rpos].y;
                // then move the write position back. this way, we'll overwrite the previous whitespace with the new glyph
                // since this is only done after the first whitespace, we only keep the first whitespace
                wpos--;
            }
            inWhite = true;
        } else {
            inWhite = false;
        }
    }
    // and the terminating glyph (we should probably copy the rest of the glyph's info, too)
    glyph_text[wpos].x = glyph_text[rpos].x - delta_x;
    glyph_text[wpos].y = glyph_text[rpos].y - delta_y;
    // sets the new length
    glyph_length = wpos;
}

// utility: computes the number of letters in the layout
int text_wrapper::NbLetter(int g_st, int g_en)
{
    if ( glyph_length <= 0 ) return 0;
    if ( g_st < 0 || g_st >= g_en ) {
        g_st = 0;
        g_en = glyph_length;
    }
    int nbLetter = 0;
    for (int i = g_st; i < g_en; i++) {
        if ( glyph_text[i].char_start ) nbLetter++;
    }
    return nbLetter;
}

void text_wrapper::AddLetterSpacing(double dx, double dy, int g_st, int g_en)
{
    if ( glyph_length <= 0 ) return;
    if ( g_st < 0 || g_st >= g_en ) {
        g_st = 0;
        g_en = glyph_length;
    }
    int nbLetter = 0;

    // letterspacing means: add 'dx * (nbLetter - 1)' to the x position
    // so we just scan the glyph string
    for (int i = g_st; i < g_en; i++) {
        if ( i > g_st && glyph_text[i].char_start ) nbLetter++;
        glyph_text[i].x += dx * nbLetter;
        glyph_text[i].y += dy * nbLetter;
    }
    if ( glyph_text[g_en].char_start ) nbLetter++;
    glyph_text[g_en].x += dx * nbLetter;
    glyph_text[g_en].y += dy * nbLetter;
}

/** @name Movement commands
 * Miscellaneous functions for moving about glyphs.
 * \a st and \en are start and end glyph indices.
 * The three methods differ only in whether they look for .char_start, .word_start or .para_start.
 * \return True iff a next character was found.  (False iff we've already reached the end.)
 */
//@{
bool text_wrapper::NextChar(int &st, int &en) const
{
    if ( st < 0 || en < 0 ) {st = 0; en = 0;}
    if ( st >= en ) en = st;
    if ( st >= glyph_length || en >= glyph_length ) return false; // finished
    st = en;
    do {
        en++;
    } while ( en < glyph_length && glyph_text[en].char_start == false );
    return true;
}
bool text_wrapper::NextWord(int &st, int &en) const
{
    if ( st < 0 || en < 0 ) {st = 0; en = 0;}
    if ( st >= en ) en = st;
    if ( st >= glyph_length || en >= glyph_length ) return false; // finished
    st = en;
    do {
        en++;
    } while ( en < glyph_length && glyph_text[en].word_start == false );
    return true;
}
bool text_wrapper::NextPara(int &st, int &en) const
{
    if ( st < 0 || en < 0 ) {st = 0; en = 0;}
    if ( st >= en ) en = st;
    if ( st >= glyph_length || en >= glyph_length ) return false; // finished
    st = en;
    do {
        en++;
    } while ( en < glyph_length && glyph_text[en].para_start == false );
    return true;
}
//@}

// boundary handling
/**
 * Append \a ib to our bounds array.
 * \return The index of the new element.
 */
unsigned text_wrapper::AddBoundary(text_boundary const &ib)
{
    if ( nbBound >= maxBound ) {
        maxBound = 2 * nbBound + 1;
        text_boundary *newdata = static_cast<text_boundary*>(realloc(bounds, maxBound * sizeof(text_boundary)));
        if (newdata != NULL)
        {
            bounds = newdata;
        }
        else
        {
            g_warning("Failed to reallocate bounds");
        }
    }
    unsigned const ix = nbBound++;
    bounds[ix] = ib;
    return ix;
}

/**
 * Add the start \& end boundaries \a is \& \a ie to bounds.
 */
void text_wrapper::AddTwinBoundaries(text_boundary const &is, text_boundary const &ie)
{
    unsigned const ns = AddBoundary(is);
    unsigned const ne = AddBoundary(ie);
    bounds[ns].start = true;
    bounds[ns].other = ne;
    bounds[ne].start = false;
    bounds[ne].other = ns;
}

static int CmpBound(void const *a, void const *b) {
    text_boundary const &ta = *reinterpret_cast<text_boundary const *>(a);
    text_boundary const &tb = *reinterpret_cast<text_boundary const *>(b);
    if ( ta.uni_pos < tb.uni_pos ) return -1;
    if ( ta.uni_pos > tb.uni_pos ) return 1;
    /* TODO: I'd guess that for a given uni_pos it would be better for the end boundary to precede the start boundary. */
    if ( ta.start && !tb.start ) return -1;
    if ( !ta.start && tb.start ) return 1;
    return 0;
}
/**
 * Sort this.bounds by b.uni_pos, updating the .other index values appropriately.
 */
void text_wrapper::SortBoundaries(void)
{
    /* effic: If this function (including descendents such as the qsort calll) ever takes
     * non-negligible time, then we can fairly easily improve it by changing MakeBoundaries add in
     * sorted order.  It would just have to remember for itself the index of each start boundary
     * for updating the .other fields appropriately.
     *
     * A simpler speedup is just to change qsort to std::sort, which can inline the comparison
     * function.
     */

    /* The 'other' field needs to be updated after sorting by qsort, so we build the inverse
     * permutation. */
    for (unsigned i = 0; i < nbBound; i++) {
        bounds[i].old_ix = i;
    }
    qsort(bounds, nbBound, sizeof(text_boundary), CmpBound);
    unsigned *const old2new = g_new(unsigned, nbBound);
    for (unsigned new_ix = 0; new_ix < nbBound; new_ix++) { // compute inverse permutation
        old2new[bounds[new_ix].old_ix] = new_ix;
    }
    for (unsigned i = 0; i < nbBound; i++) { // update 'other'
        if ( bounds[i].other < nbBound ) {
            bounds[i].other = old2new[bounds[i].other];
        }
    }
    g_free(old2new);
}
void text_wrapper::MakeTextBoundaries(PangoLogAttr *pAttrs, int nAttr)
{
    if ( pAttrs == NULL || nAttr <= 0 || uni32_length <= 0 ) return;
    if ( nAttr > uni32_length + 1 ) nAttr = uni32_length + 1;
    int last_c_st = -1;
    int last_w_st = -1;
    int last_s_st = -1;
    int last_p_st = 0;
    // reads the text and adds a pair of boundaries each time we encounter a stop
    // last_* are used to keep track of the start of new text chunk
    for (int i = 0; i <= nAttr; i++) {
        text_boundary nbs;
        text_boundary nbe;
        nbs.uni_pos = i;
        nbs.start = true;
        nbe.uni_pos = i;
        nbe.start = false;
        // letters
        if ( i == nAttr || pAttrs[i].is_cursor_position ) {
            if ( last_c_st >= 0 ) {
                nbs.type = nbe.type = bnd_char;
                nbs.uni_pos = last_c_st;
                nbe.uni_pos = i;
                AddTwinBoundaries(nbs, nbe);
            }
            last_c_st = i;
        }
        // words
        if ( i == nAttr || pAttrs[i].is_word_start ) {
            if ( last_w_st >= 0 ) {
                nbs.type = nbe.type = bnd_word;
                nbs.uni_pos = last_w_st;
                nbe.uni_pos = i;
                nbs.data.i = nbe.data.i = ( pAttrs[last_w_st].is_white ? 1 : 0 );
                AddTwinBoundaries(nbs, nbe);
            }
            last_w_st = i;
        }
        if ( i < nAttr && pAttrs[i].is_word_end ) {
            if ( last_w_st >= 0 ) {
                nbs.type = nbe.type = bnd_word;
                nbs.uni_pos = last_w_st;
                nbe.uni_pos = i;
                nbs.data.i = nbe.data.i = ( pAttrs[last_w_st].is_white ? 1 : 0 );
                AddTwinBoundaries(nbs, nbe);
            }
            last_w_st = i;
        }
        // sentences
        if ( i == nAttr || pAttrs[i].is_sentence_boundary ) {
            if ( last_s_st >= 0 ) {
                nbs.type = nbe.type = bnd_sent;
                nbs.uni_pos = last_s_st;
                nbe.uni_pos = i;
                AddTwinBoundaries(nbs, nbe);
            }
            last_s_st = i;
        }
        // paragraphs
        if ( i == nAttr || uni32_text[i] == '\n' || uni32_text[i] == '\r' ) { // too simple to be true?
            nbs.type = nbe.type = bnd_para;
            nbs.uni_pos = last_p_st;
            nbe.uni_pos = i + 1;
            AddTwinBoundaries(nbs, nbe);
            last_p_st = i + 1;
        }
    }
}

bool text_wrapper::IsBound(BoundaryType const bnd_type, int g_st, int &c_st)
{
    if ( c_st < 0 ) c_st = 0;
    int scan_dir = 0;
    while ( unsigned(c_st) < nbBound ) {
        if ( bounds[c_st].uni_pos == g_st && bounds[c_st].type == bnd_type ) {
            return true;
        }
        if ( bounds[c_st].uni_pos < g_st ) {
            if ( scan_dir < 0 ) break;
            c_st++;
            scan_dir = 1;
        } else if ( bounds[c_st].uni_pos > g_st ) {
            if ( scan_dir > 0 ) break;
            c_st--;
            scan_dir = -1;
        } else {
            // good pos, wrong type
            while ( c_st > 0 && bounds[c_st].uni_pos == g_st ) {
                c_st--;
            }
            if ( bounds[c_st].uni_pos < g_st ) c_st++;
            while ( unsigned(c_st) < nbBound && bounds[c_st].uni_pos == g_st ) {
                if ( bounds[c_st].type == bnd_type ) {
                    return true;
                }
                c_st++;
            }
            break;
        }
    }
    return false;
}

/* Unused.  Retained only because I haven't asked cyreve (Richard Hughes) whether he intends ever
 * to use it.  You can probably safely remove it. */
//bool text_wrapper::Contains(BoundaryType const bnd_type, int g_st, int g_en, int &c_st, int &c_en)
//{
//    if ( c_st < 0 ) c_st = 0;
//    bool found = false;
//    int scan_dir = 0;
//    while ( unsigned(c_st) < nbBound ) {
//        if ( bounds[c_st].type == bnd_type ) {
//            if ( bounds[c_st].start ) {
//                c_en = bounds[c_st].other;
//            } else {
//            }
//        }
//        if ( bounds[c_st].type == bnd_type && unsigned(c_en) == bounds[c_st].other ) {
//            if ( g_st >= bounds[c_st].uni_pos && g_en <= bounds[c_en].uni_pos ) {
//                // character found
//                found = true;
//                break;
//            }
//        }
//        if ( bounds[c_st].uni_pos < g_st ) {
//            if ( scan_dir < 0 ) break;
//            c_st++;
//            scan_dir = 1;
//        } else if ( bounds[c_st].uni_pos > g_st ) {
//            if ( scan_dir > 0 ) break;
//            c_st--;
//            scan_dir = -1;
//        } else {
//            // good pos, wrong type
//            while ( c_st > 0 && bounds[c_st].uni_pos == g_st ) {
//                c_st--;
//            }
//            if ( bounds[c_st].uni_pos < g_st ) c_st++;
//            while ( unsigned(c_st) < nbBound && bounds[c_st].uni_pos == g_st ) {
//                if ( bounds[c_st].type == bnd_type ) {
//                    if ( bounds[c_st].start ) {
//                        c_en = bounds[c_st].other;
//                    } else {
//                    }
//                }
//                if ( bounds[c_st].type == bnd_type && unsigned(c_en) == bounds[c_st].other ) {
//                    if ( g_st >= bounds[c_st].uni_pos && g_en <= bounds[c_en].uni_pos ) {
//                        // character found
//                        return true;
//                    }
//                }
//                c_st++;
//            }
//
//            break;
//        }
//    }
//    return found;
//}

void text_wrapper::MeasureBoxes(void)
{
    font_factory *f_src = font_factory::Default();
    for (int i = 0; i < nbBox; i++) {
        boxes[i].ascent = 0;
        boxes[i].descent = 0;
        boxes[i].xheight = 0;
        boxes[i].width = 0;

        PangoFont *curPF = glyph_text[boxes[i].g_st].font;
        if ( curPF ) {
            PangoFontDescription *pfd = pango_font_describe(curPF);
            font_instance *curF = f_src->Face(pfd);
            if ( curF ) {
                curF->FontMetrics(boxes[i].ascent, boxes[i].descent, boxes[i].xheight);
                curF->Unref();
            }
            pango_font_description_free(pfd);
            boxes[i].width = glyph_text[boxes[i].g_en].x - glyph_text[boxes[i].g_st].x;
        }
    }
}


void text_wrapper::KernXForLastAddition(double *i_kern_x, int i_len, double scale)
{
    if ( i_kern_x == NULL || i_len <= 0 || last_addition < 0 || last_addition >= uni32_length || uni32_length <= 0 ) return;
    if ( kern_x == NULL ) {
        kern_x = (double*)malloc((uni32_length + 1) * sizeof(double));
        for (int i = 0; i <= uni32_length; i++) kern_x[i] = 0;
    }
    int last_len = uni32_length - last_addition;
    if ( i_len > last_len ) i_len = last_len;
    for (int i = 0; i < i_len; i++) kern_x[last_addition + i] = i_kern_x[i] * scale;
}

void text_wrapper::KernYForLastAddition(double *i_kern_y, int i_len, double scale)
{
    if ( i_kern_y == NULL || i_len <= 0 || last_addition < 0 || last_addition >= uni32_length || uni32_length <= 0 ) return;
    if ( kern_y == NULL ) {
        kern_y = (double*)malloc((uni32_length + 1) * sizeof(double));
        for (int i = 0; i <= uni32_length; i++) kern_y[i] = 0;
    }
    int last_len = uni32_length - last_addition;
    if ( i_len > last_len ) i_len = last_len;
    for (int i = 0; i < i_len; i++) kern_y[last_addition + i] = i_kern_y[i] * scale;
}

void text_wrapper::KernXForLastAddition(GList *i_kern_x, double scale)
{
    if ( i_kern_x == NULL || last_addition < 0 || last_addition >= uni32_length || uni32_length <= 0 ) return;
    if ( kern_x == NULL ) {
        kern_x = (double*)malloc((uni32_length + 1) * sizeof(double));
        for (int i = 0; i <= uni32_length; i++) kern_x[i] = 0;
    }
    int last_len = uni32_length - last_addition;
    GList *l = i_kern_x;
    for (int i = 0; i < last_len && l && l->data; i++, l = l->next) {
        kern_x[last_addition + i] = ((SVGLength *) l->data)->computed * scale;
    }
}

void text_wrapper::KernYForLastAddition(GList *i_kern_y, double scale)
{
    if ( i_kern_y == NULL || last_addition < 0 || last_addition >= uni32_length || uni32_length <= 0 ) return;
    if ( kern_y == NULL ) {
        kern_y = (double*)malloc((uni32_length + 1) * sizeof(double));
        for (int i = 0; i <= uni32_length; i++) kern_y[i] = 0;
    }
    int last_len = uni32_length - last_addition;
    GList *l = i_kern_y;
    for (int i = 0; i < last_len && l && l->data; i++, l = l->next) {
        kern_y[last_addition + i] = ((SVGLength *) l->data)->computed * scale;
    }
}


void text_wrapper::AddDxDy(void)
{
    if ( glyph_length <= 0 ) return;
    if ( kern_x ) {
        double sum = 0;
        int l_pos = -1;
        for (int i = 0; i < glyph_length; i++) {
            int n_pos = glyph_text[i].uni_st;
            if ( l_pos < n_pos ) {
                for (int j = l_pos + 1; j <= n_pos; j++) sum += kern_x[j];
            } else if ( l_pos > n_pos ) {
                for (int j = l_pos; j > n_pos; j--) sum -= kern_x[j];
            }
            l_pos = n_pos;

            glyph_text[i].x += sum;
        }
        {
            int n_pos = uni32_length;
            if ( l_pos < n_pos ) {
                for (int j = l_pos + 1; j <= n_pos; j++) sum += kern_x[j];
            } else if ( l_pos > n_pos ) {
                for (int j = l_pos; j > n_pos; j--) sum -= kern_x[j];
            }
            // l_pos = n_pos;
            glyph_text[glyph_length].x += sum;
        }
    }
    if ( kern_y ) {
        double sum = 0;
        int l_pos = -1;
        for (int i = 0; i < glyph_length; i++) {
            int n_pos = glyph_text[i].uni_st;
            if ( l_pos < n_pos ) {
                for (int j = l_pos + 1; j <= n_pos; j++) sum += kern_y[j];
            } else if ( l_pos > n_pos ) {
                for (int j = l_pos; j > n_pos; j--) sum -= kern_y[j];
            }
            l_pos = n_pos;

            glyph_text[i].y += sum;
        }
        {
            int n_pos = uni32_length;
            if ( l_pos < n_pos ) {
                for (int j = l_pos + 1; j <= n_pos; j++) sum += kern_y[j];
            } else if ( l_pos > n_pos ) {
                for (int j = l_pos; j > n_pos; j--) sum -= kern_y[j];
            }
            // l_pos = n_pos;
            glyph_text[glyph_length].y += sum;
        }
    }
}


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
