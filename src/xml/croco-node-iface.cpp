#include <cstring>
#include <string>
#include <glib.h>

#include "xml/croco-node-iface.h"
#include "xml/node.h"

static char const *
local_part(char const *const qname)
{
    char const *ret = std::strrchr(qname, ':');
    if (ret)
        return ++ret;
    else
        return qname;
}

namespace Inkscape {
namespace XML {

extern "C" {

static CRXMLNodePtr get_parent(CRXMLNodePtr n) { return static_cast<Node const *>(n)->parent(); }
static CRXMLNodePtr get_first_child(CRXMLNodePtr n) { return static_cast<Node const *>(n)->firstChild(); }
static CRXMLNodePtr get_next(CRXMLNodePtr n) { return static_cast<Node const *>(n)->next(); }

static CRXMLNodePtr get_prev(CRXMLNodePtr cn)
{
    Node const *n = static_cast<Node const *>(cn);
    unsigned const n_pos = n->position();
    if (n_pos) {
        return n->parent()->nthChild(n_pos - 1);
    } else {
        return NULL;
    }
}

static char *get_attr(CRXMLNodePtr n, char const *a)
{
    return g_strdup(static_cast<Node const *>(n)->attribute(a));
}

static char const *get_local_name(CRXMLNodePtr n) { return local_part(static_cast<Node const *>(n)->name()); }
static gboolean is_element_node(CRXMLNodePtr n) { return static_cast<Node const *>(n)->type() == ELEMENT_NODE; }
}

/**
 * Interface for XML nodes used by libcroco.
 *
 * This structure defines operations on Inkscape::XML::Node used by the libcroco
 * CSS parsing library.
 */
CRNodeIface const croco_node_iface = {
    get_parent,
    get_first_child,
    get_next,
    get_prev,
    get_local_name,
    get_attr,
    g_free,
    is_element_node
};

}
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
