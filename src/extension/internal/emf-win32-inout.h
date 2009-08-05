/** @file
 * @brief Enhanced Metafile Input/Output
 */
/* Authors:
 *   Ulf Erikson <ulferikson@users.sf.net>
 *
 * Copyright (C) 2006-2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef SEEN_EXTENSION_INTERNAL_EMF_WIN32_H
#define SEEN_EXTENSION_INTERNAL_EMF_WIN32_H
#ifdef WIN32

#include "extension/implementation/implementation.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

class EmfWin32 : Inkscape::Extension::Implementation::Implementation { //This is a derived class

public:
    EmfWin32(); // Empty constructor

    virtual ~EmfWin32();//Destructor

    bool check(Inkscape::Extension::Extension *module); //Can this module load (always yes for now)

    void save(Inkscape::Extension::Output *mod, // Save the given document to the given filename
              Document *doc,
              gchar const *filename);

    virtual Document *open( Inkscape::Extension::Input *mod,
                                const gchar *uri );

    static void init(void);//Initialize the class

private:
};

} } }  /* namespace Inkscape, Extension, Implementation */

#endif /* WIN32 */

#endif /* EXTENSION_INTERNAL_EMF_WIN32_H */

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
