/** @file
 * @brief Definition of the BoundaryType enum
 */

#ifndef LIBNRTYPE_BOUNDARY_TYPE_H_INKSCAPE
#define LIBNRTYPE_BOUNDARY_TYPE_H_INKSCAPE

/**
 * The different kinds of semantic boundaries in text; or rather,
 * the different things that may be delimited by a text_boundary.
 */
enum BoundaryType {
    bnd_none = 0,
    bnd_char,
    bnd_word,
    bnd_sent,  /**< Sentence.  Not currently used, and pango (1.8) does a bad job of determining
                * sentence boundaries anyway. */
    bnd_para
};


#endif /* !LIBNRTYPE_BOUNDARY_TYPE_H_INKSCAPE */

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
