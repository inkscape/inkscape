/*
 * Code for handling XSLT extensions
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2006-2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __INKSCAPE_EXTENSION_IMPEMENTATION_XSLT_H__
#define __INKSCAPE_EXTENSION_IMPEMENTATION_XSLT_H__

#include "implementation.h"

#include "libxml/tree.h"
#include "libxslt/xslt.h"
#include "libxslt/xsltInternals.h"

namespace Inkscape {
namespace XML {
class Node;
}
}


namespace Inkscape {
namespace Extension {
namespace Implementation {

class XSLT : public Implementation {
private:
    std::string _filename;
    xmlDocPtr _parsedDoc;
    xsltStylesheetPtr _stylesheet;

    Glib::ustring solve_reldir(Inkscape::XML::Node *reprin);
public:
    XSLT (void);

    bool load(Inkscape::Extension::Extension *module);
    void unload(Inkscape::Extension::Extension *module);

    bool check(Inkscape::Extension::Extension *module);

    SPDocument *open(Inkscape::Extension::Input *module,
                     gchar const *filename);
    void save(Inkscape::Extension::Output *module, SPDocument *doc, gchar const *filename);
};

}  /* Inkscape  */
}  /* Extension  */
}  /* Implementation  */
#endif /* __INKSCAPE_EXTENSION_IMPEMENTATION_XSLT_H__ */

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
