/** @file
 * @brief Some functions relevant sorting reprs by position within document.
 * @todo Functions in this file have non-English names. Determine what they do and rename
 *       accordingly.
 */
 
#ifndef SEEN_XML_REPR_SORTING_H
#define SEEN_XML_REPR_SORTING_H

#include "xml/xml-forward.h"

Inkscape::XML::Node *LCA(Inkscape::XML::Node *a, Inkscape::XML::Node *b);
Inkscape::XML::Node *AncetreFils(Inkscape::XML::Node *descendent, Inkscape::XML::Node *ancestor);

#endif // SEEN_XML_REPR_SOTRING_H
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
