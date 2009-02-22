#ifndef SEEN_LIBNR_NR_PATH_CODE_H
#define SEEN_LIBNR_NR_PATH_CODE_H

/** \file
 * NRPathcode enum definition
 */

typedef enum {
    NR_MOVETO,        ///< Start of closed subpath
    NR_MOVETO_OPEN,   ///< Start of open subpath
    NR_CURVETO,       ///< Bezier curve segment
    NR_LINETO,        ///< Line segment
    NR_END            ///< End record
} NRPathcode;


#endif /* !SEEN_LIBNR_NR_PATH_CODE_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
