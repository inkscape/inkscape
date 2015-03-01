/** @file
 * @brief Key-value pair representing an attribute
 */

#ifndef SEEN_XML_SP_REPR_ATTR_H
#define SEEN_XML_SP_REPR_ATTR_H

#include <glib.h>
#include "inkgc/gc-managed.h"
#include "util/share.h"

#define SP_REPR_ATTRIBUTE_KEY(a) g_quark_to_string((a)->key)
#define SP_REPR_ATTRIBUTE_VALUE(a) ((a)->value)

namespace Inkscape {
namespace XML {

/**
 * @brief Key-value pair representing an attribute
 *
 * Internally, the attributes of each node in the XML tree are
 * represented by this structure.
 */
struct AttributeRecord : public Inkscape::GC::Managed<> {
    AttributeRecord(GQuark k, Inkscape::Util::ptr_shared<char> v)
    : key(k), value(v) {}

    /** @brief GQuark corresponding to the name of the attribute */
    GQuark key;
    /** @brief Shared pointer to the value of the attribute */
    Inkscape::Util::ptr_shared<char> value;

    // accept default copy constructor and assignment operator
};

}
}

#endif /* !SEEN_XML_SP_REPR_ATTR_H */

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
