#ifndef REBASE_HREFS_H_SEEN
#define REBASE_HREFS_H_SEEN

#include <glib/gtypes.h>
#include "util/list.h"
#include "xml/attribute-record.h"
struct Document;

namespace Inkscape {
namespace XML {

gchar *calc_abs_doc_base(gchar const *doc_base);

void rebase_hrefs(Document *doc, gchar const *new_base, bool spns);

Inkscape::Util::List<AttributeRecord const> rebase_href_attrs(
    gchar const *old_abs_base,
    gchar const *new_abs_base,
    Inkscape::Util::List<AttributeRecord const> attributes);

}
}


#endif /* !REBASE_HREFS_H_SEEN */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vi: set autoindent shiftwidth=4 tabstop=8 filetype=cpp expandtab softtabstop=4 encoding=utf-8 textwidth=99 :
