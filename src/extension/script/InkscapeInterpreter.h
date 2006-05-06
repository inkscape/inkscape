#ifndef __INKSCAPE_INTERPRETER_H__
#define __INKSCAPE_INTERPRETER_H__

/**
 * Base class for interpreter implementations, (InkscapePython, etc)
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm.h>

namespace Inkscape {
namespace Extension {
namespace Script {


class InkscapeInterpreter
{
public:

    /**
     *
     */
    InkscapeInterpreter();

    /**
     *
     */
    virtual ~InkscapeInterpreter();

    /**
     *
     */
    virtual bool interpretScript(const Glib::ustring &script,
                                 Glib::ustring &output,
                                 Glib::ustring &error);

    /**
     *
     */
    virtual bool interpretUri(const Glib::ustring &uri,
                              Glib::ustring &output,
                              Glib::ustring &error);



}; //class InkscapeInterpreter




}  // namespace Script
}  // namespace Extension
}  // namespace Inkscape



#endif  /* __INKSCAPE_INTERPRETER_H__ */
//#########################################################################
//# E N D    O F    F I L E
//#########################################################################


