#ifndef SEEN_SP_MARKER_LOC_H
#define SEEN_SP_MARKER_LOC_H

/**
 * These enums are to allow us to have 4-element arrays that represent a set of marker locations
 * (all, start, mid, and end).  This allows us to iterate through the array in places where we need
 * to do a process across all of the markers, instead of separate code stanzas for each.
 *
 * IMPORTANT:  the code assumes that the locations have the values as written below! so don't change the values!!!
 */
enum SPMarkerLoc {
    SP_MARKER_LOC = 0,
    SP_MARKER_LOC_START = 1,
    SP_MARKER_LOC_MID = 2,
    SP_MARKER_LOC_END = 3,
    SP_MARKER_LOC_QTY = 4
};


#endif /* !SEEN_SP_MARKER_LOC_H */

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
