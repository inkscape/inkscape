/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2006 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */



#ifndef INKSCAPE_EXTENSION_OUTPUT_H__
#define INKSCAPE_EXTENSION_OUTPUT_H__

#include "extension.h"
class SPDocument;

namespace Inkscape {
namespace Extension {

class Output : public Extension {
    gchar *mimetype;             /**< What is the mime type this inputs? */
    gchar *extension;            /**< The extension of the input files */
    gchar *filetypename;         /**< A userfriendly name for the file type */
    gchar *filetypetooltip;      /**< A more detailed description of the filetype */
    bool   dataloss;             /**< The extension causes data loss on save */

public:
    class save_failed {};        /**< Generic failure for an undescribed reason */
    class save_cancelled {};     /**< Saving was cancelled */
    class no_extension_found {}; /**< Failed because we couldn't find an extension to match the filename */
    class file_read_only {};     /**< The existing file can not be opened for writing */

                 Output (Inkscape::XML::Node * in_repr,
                         Implementation::Implementation * in_imp);
    virtual     ~Output (void);
    virtual bool check                (void);
    void         save (SPDocument *doc,
                       gchar const *uri);
    bool         prefs (void);
    gchar *      get_mimetype(void);
    gchar *      get_extension(void);
    gchar *      get_filetypename(void);
    gchar *      get_filetypetooltip(void);
    bool         causes_dataloss(void) { return dataloss; };
};

} }  /* namespace Inkscape, Extension */
#endif /* INKSCAPE_EXTENSION_OUTPUT_H__ */

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
