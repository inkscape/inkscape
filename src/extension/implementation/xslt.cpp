/** \file
 * Code for handling XSLT extensions.
 */
/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "xslt.h"


/* Namespaces */
namespace Inkscape {
namespace Extension {
namespace Implementation {

/* Real functions */
/**
    \return    A XSLT object
    \brief     This function creates a XSLT object and sets up the
               variables.

*/
XSLT::XSLT(void) :
    Implementation()
{
}


}  /* Implementation  */
}  /* module  */
}  /* Inkscape  */


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
