#ifndef __INKSCAPE_SCRIPT_H__
#define __INKSCAPE_SCRIPT_H__

/**
 * Inkscape Scripting container
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004-2006 Bob Jamison
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"

#include <glibmm.h>

namespace Inkscape {
namespace Extension {
namespace Script {


class InkscapeScript
{
public:

    /**
     * Which type of language?
     */
    typedef enum
        {
        JAVASCRIPT,
        PYTHON,
        RUBY
        } ScriptLanguage;

    /**
     * Creates a generic script interpreter.
     */
    InkscapeScript();

    /**
     *  Destructor
     */
    virtual ~InkscapeScript();

    /**
     * Interprets the script in the 'script' buffer,
     * storing the stdout output in 'output', and any
     * error messages in 'error.'  Language is one of the
     * enumerated types in ScriptLanguage above.
     */
    bool interpretScript(const Glib::ustring &script,
                         Glib::ustring &output,
                         Glib::ustring &error,
                         ScriptLanguage language);

    /**
     * Interprets the script in the named file,
     * storing the stdout output in 'output', and any
     * error messages in 'error.'  Language is one of the
     * enumerated types in ScriptLanguage above.
     */
    bool interpretFile(const Glib::ustring &fname,
                      Glib::ustring &output,
                      Glib::ustring &error,
                      ScriptLanguage language);



}; //class InkscapeScript




}  // namespace Script
}  // namespace Extension
}  // namespace Inkscape



#endif  /* __INKSCAPE_SCRIPT_H__ */

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
