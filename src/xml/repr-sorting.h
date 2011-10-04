/** @file
 * @brief Some functions relevant sorting reprs by position within document.
 * @todo Functions in this file have non-English names. Determine what they do and rename
 *       accordingly.
 */
 
#ifndef SEEN_XML_REPR_SORTING_H
#define SEEN_XML_REPR_SORTING_H

namespace Inkscape {
namespace XML {

class Node;

} // namespace XML
} // namespace Inkscape


Inkscape::XML::Node *LCA(Inkscape::XML::Node *a, Inkscape::XML::Node *b);
Inkscape::XML::Node const *LCA(Inkscape::XML::Node const *a, Inkscape::XML::Node const *b);

/**
 * Returns a child of \a ancestor such that ret is itself an ancestor of \a descendent.
 *
 * The current version returns NULL if ancestor or descendent is NULL, though future versions may
 * call g_log.  Please update this comment if you rely on the current behaviour.
 */
Inkscape::XML::Node const *AncetreFils(Inkscape::XML::Node const *descendent, Inkscape::XML::Node const *ancestor);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
