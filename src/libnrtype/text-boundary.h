#ifndef TEXT_BOUNDARY_H_INKSCAPE
#define TEXT_BOUNDARY_H_INKSCAPE

/** \file Definition of text_boundary. */

/*
 * License: May be redistributed with or without modifications under the terms of the Gnu General
 * Public License as published by the Free Software Foundation, version 2 or (at your option) any
 * later version.
 */

#include "libnrtype/boundary-type.h"


/**
 * A character/word/paragraph boundary in the text, used by TextWrapper.
 *
 * (Boundaries are paired.)
 */
struct text_boundary {
    /** Index of the boundary in the text: first char of the text chunk if 'start' boundary, char
     *  right after the boundary otherwise.
     */
    int uni_pos;
    BoundaryType type;    ///< Kind of boundary.
    bool start;   ///< Indicates whether this marks the beginning or end of a chunk.
    unsigned other;   ///< Index in bounds[] of the corresponding end/beginning boundary.
    unsigned old_ix;  ///< Temporary storage used solely SortBoundaries.
    /// Data for this boundary; usually, one int is enough.
    union {
        int i;
        double f;
        void *p;
    } data;
};


#endif /* !TEXT_BOUNDARY_H_INKSCAPE */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
