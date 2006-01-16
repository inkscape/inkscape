/**
 * Python Interpreter wrapper for Inkscape
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "InkscapeInterpreter.h"

#include <fstream>

namespace Inkscape {
namespace Extension {
namespace Script {

/*
 *
 */
InkscapeInterpreter::InkscapeInterpreter()
{
}

    

/*
 *
 */
InkscapeInterpreter::~InkscapeInterpreter()
{

}

    
    

/*
 *  Interpret an in-memory string
 */
bool InkscapeInterpreter::interpretScript(Glib::ustring &script,
                                 Glib::ustring &output,
                                 Glib::ustring &error)
{
    //do nothing.  let the subclasses implement this
    return true;
}

    
    

/*
 *  Interpret a named file
 */
bool InkscapeInterpreter::interpretUri(Glib::ustring &uri,
                                 Glib::ustring &output,
                                 Glib::ustring &error)
{
    char *curi = (char *)uri.raw().c_str();
    std::ifstream ins(curi);
    if (!ins.good())
        {
        printf("interpretUri: Could not open %s for reading\n", curi);
        return false;
        }
        
    Glib::ustring buf;
    
    while (!ins.eof())
        {
        gunichar ch = (gunichar) ins.get();
        buf.push_back(ch);
        }

    ins.close();
    
    bool ret = interpretScript(buf, output, error);

    return ret;

}



}  // namespace Script
}  // namespace Extension
}  // namespace Inkscape

//#########################################################################
//# E N D    O F    F I L E
//#########################################################################
