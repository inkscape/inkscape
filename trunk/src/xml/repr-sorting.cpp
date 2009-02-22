
#include "algorithms/longest-common-suffix.h"
#include "xml/repr.h"
#include "xml/node-iterators.h"

static bool
same_repr(Inkscape::XML::Node &a, Inkscape::XML::Node &b)
{
  /* todo: I'm not certain that it's legal to take the address of a reference.  Check the exact wording of the spec on this matter. */
    return &a == &b;
}

Inkscape::XML::Node *
LCA(Inkscape::XML::Node *a, Inkscape::XML::Node *b)
{
    using Inkscape::Algorithms::longest_common_suffix;
    Inkscape::XML::Node *ancestor = longest_common_suffix<Inkscape::XML::NodeParentIterator>(
        a, b, NULL, &same_repr
    );
    if ( ancestor && ancestor->type() != Inkscape::XML::DOCUMENT_NODE ) {
        return ancestor;
    } else {
        return NULL;
    }
}

/**
 * Returns a child of \a ancestor such that ret is itself an ancestor of \a descendent.
 *
 * The current version returns NULL if ancestor or descendent is NULL, though future versions may
 * call g_log.  Please update this comment if you rely on the current behaviour.
 */
Inkscape::XML::Node *
AncetreFils(Inkscape::XML::Node *descendent, Inkscape::XML::Node *ancestor)
{
    if (descendent == NULL || ancestor == NULL)
        return NULL;
    if (sp_repr_parent(descendent) == ancestor)
        return descendent;
    return AncetreFils(sp_repr_parent(descendent), ancestor);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
