
#include "util/longest-common-suffix.h"
#include "xml/repr.h"
#include "xml/node-iterators.h"
#include "repr-sorting.h"

static bool same_repr(Inkscape::XML::Node const &a, Inkscape::XML::Node const &b)
{
  /* todo: I'm not certain that it's legal to take the address of a reference.  Check the exact wording of the spec on this matter. */
    return &a == &b;
}

Inkscape::XML::Node const *LCA(Inkscape::XML::Node const *a, Inkscape::XML::Node const *b)
{
    using Inkscape::Algorithms::longest_common_suffix;
    Inkscape::XML::Node const *ancestor = longest_common_suffix<Inkscape::XML::NodeConstParentIterator>(
        a, b, NULL, &same_repr);
    bool OK = false;
    if (ancestor) {
        if (ancestor->type() != Inkscape::XML::DOCUMENT_NODE) {
            OK = true;
        }
    }
    if ( OK ) {
        return ancestor;
    } else {
        return NULL;
    }
}

Inkscape::XML::Node *LCA(Inkscape::XML::Node *a, Inkscape::XML::Node *b)
{
    Inkscape::XML::Node const *tmp = LCA(const_cast<Inkscape::XML::Node const *>(a), const_cast<Inkscape::XML::Node const *>(b));
    return const_cast<Inkscape::XML::Node *>(tmp);
}

Inkscape::XML::Node const *AncetreFils(Inkscape::XML::Node const *descendent, Inkscape::XML::Node const *ancestor)
{
    Inkscape::XML::Node const *result = 0;
    if ( descendent && ancestor ) {
        if (descendent->parent() == ancestor) {
            result = descendent;
        } else {
            result = AncetreFils(descendent->parent(), ancestor);
        }
    }
    return result;
}

Inkscape::XML::Node *AncetreFils(Inkscape::XML::Node *descendent, Inkscape::XML::Node *ancestor)
{
    Inkscape::XML::Node const * tmp = AncetreFils(const_cast<Inkscape::XML::Node const*>(descendent), const_cast<Inkscape::XML::Node const*>(ancestor));
    return const_cast<Inkscape::XML::Node *>(tmp);
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
