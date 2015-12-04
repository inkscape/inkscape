/** @file
 * @brief Definition of struct one_box
 */

#ifndef LIBNRTYPE_ONE_BOX_H_INKSCAPE
#define LIBNRTYPE_ONE_BOX_H_INKSCAPE

// text chunking 2, the comeback
// this time for sp-typeset
struct one_box {
    int g_st, g_en; ///< First and last glyph of this word.
    double ascent, descent, xheight;
    double width;
    bool word_start, word_end;
};


#endif /* !LIBNRTYPE_ONE_BOX_H_INKSCAPE */

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

