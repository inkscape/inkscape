/** \file Some functions relevant sorting reprs by position within document. */

namespace Inkscape {
namespace XML {
class Node;
}
}


Inkscape::XML::Node *LCA(Inkscape::XML::Node *a, Inkscape::XML::Node *b);
Inkscape::XML::Node *AncetreFils(Inkscape::XML::Node *descendent, Inkscape::XML::Node *ancestor);
