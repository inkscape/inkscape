#ifndef __INKSCAPE_PYTHON_H__
#define __INKSCAPE_PYTHON_H__

/**
 * Python Interpreter wrapper for Inkscape
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004-2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "InkscapeInterpreter.h"
#include <glibmm.h>



namespace Inkscape
{
namespace Extension
{
namespace Script
{



class InkscapePython : public InkscapeInterpreter
{
public:

    /**
     * Constructor.  Create and initialize python binding.
     * Note that we will only actually start up the Python
     * interpreter when there is a demand.          
     */
    InkscapePython()
        {
        }


    /**
     * Destructor
     */
    virtual ~InkscapePython()
        {
        }



    /**
     *  Overloaded from InkscapeInterpreter
     */
    virtual bool interpretScript(const Glib::ustring &script,
                                 Glib::ustring &output,
                                 Glib::ustring &error);


    virtual void writeStdOut(const Glib::ustring &txt)
        {
        stdOut.append(txt);
        }

    virtual void writeStdErr(const Glib::ustring &txt)
        {
        stdErr.append(txt);
        }

private:

    Glib::ustring stdOut;
    Glib::ustring stdErr;

};




}  // namespace Script
}  // namespace Extension
}  // namespace Inkscape



#endif /*__INKSCAPE_PYTHON_H__ */

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
