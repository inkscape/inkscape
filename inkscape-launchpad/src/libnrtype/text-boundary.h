/** @file
 * @brief Definition of the structure text_boundary
 */
/* This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 */

#ifndef TEXT_BOUNDARY_H_INKSCAPE
#define TEXT_BOUNDARY_H_INKSCAPE

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
