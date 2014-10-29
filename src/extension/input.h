/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifndef INKSCAPE_EXTENSION_INPUT_H__
#define INKSCAPE_EXTENSION_INPUT_H__

#include <exception>
#include <glib.h>
#include "extension.h"

class SPDocument;

namespace Inkscape {
namespace Extension {

class Input : public Extension {
    gchar *mimetype;             /**< What is the mime type this inputs? */
    gchar *extension;            /**< The extension of the input files */
    gchar *filetypename;         /**< A userfriendly name for the file type */
    gchar *filetypetooltip;      /**< A more detailed description of the filetype */

public: /* this is a hack for this release, this will be private shortly */
    gchar *output_extension;     /**< Setting of what output extension should be used */

public:
    struct open_failed : public std::exception {
        virtual ~open_failed() throw() {}
        const char *what() const throw() { return "Open failed"; }
    };
    struct no_extension_found : public std::exception {
        virtual ~no_extension_found() throw() {}
        const char *what() const throw() { return "No suitable input extension found"; }
    };
    struct open_cancelled : public std::exception {
        virtual ~open_cancelled() throw() {}
        const char *what() const throw() { return "Open was cancelled"; }
    };

                  Input                (Inkscape::XML::Node * in_repr,
                                        Implementation::Implementation * in_imp);
    virtual      ~Input                (void);
    virtual bool  check                (void);
    SPDocument *  open                 (gchar const *uri);
    gchar *       get_mimetype         (void);
    gchar *       get_extension        (void);
    gchar *       get_filetypename     (void);
    gchar *       get_filetypetooltip  (void);
    bool          prefs                (gchar const *uri);
};

} }  /* namespace Inkscape, Extension */
#endif /* INKSCAPE_EXTENSION_INPUT_H__ */

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
