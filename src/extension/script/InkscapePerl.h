#ifndef __INKSCAPE_PERL_H__
#define __INKSCAPE_PERL_H__

/**
 * Perl Interpreter wrapper for Inkscape
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "InkscapeInterpreter.h"
#include <glibmm.h>

namespace Inkscape {
namespace Extension {
namespace Script {


class InkscapePerl : public InkscapeInterpreter
{
public:

    /*
     *
     */
    InkscapePerl();


    /*
     *
     */
    virtual ~InkscapePerl();



    /*
     *
     */
    bool interpretScript(const Glib::ustring &script,
                         Glib::ustring &output,
                         Glib::ustring &error);





private:


};

}  // namespace Script
}  // namespace Extension
}  // namespace Inkscape



#endif /*__INKSCAPE_PERL_H__ */

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
