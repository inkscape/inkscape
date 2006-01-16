/**
 * Inkscape Scripting container
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "InkscapeScript.h"

#include "InkscapeInterpreter.h"

#ifdef WITH_PERL
# include "InkscapePerl.h"
#endif

#ifdef WITH_PYTHON
# include "InkscapePython.h"
#endif


namespace Inkscape {
namespace Extension {
namespace Script {


/**
 *
 */
InkscapeScript::InkscapeScript()
{



}




/**
 *
 */
InkscapeScript::~InkscapeScript()
{


}




/**
 *
 */
bool InkscapeScript::interpretScript(Glib::ustring &script,
                                 Glib::ustring &output,
                                 Glib::ustring &error,
                                 ScriptLanguage language)
{
#ifndef __GNUC__
    static char const __FUNCTION__[] = "interpretScript";
#endif
    char * langname=NULL;
    InkscapeInterpreter *interp = NULL;
    //if() instead of switch() lets us scope vars
    if (language == InkscapeScript::PERL)
        {
#ifdef WITH_PERL
        langname="Perl";
        interp = new InkscapePerl();
#endif
        }
    else if (language == InkscapeScript::PYTHON)
        {
#ifdef WITH_PYTHON
        langname="Python";
        interp = new InkscapePython();
#endif
        }
    else
        {
        //replace with g_error
        fprintf(stderr, "%s: Unknown Script Language type: %d\n",
                        __FUNCTION__, language);
        return false;
        }
        
    if (!interp)
        {
        fprintf(stderr, "%s: error starting Language '%s'\n",
                        __FUNCTION__, langname);
        return false;
        }

    if (!interp->interpretScript(script, output, error))
        {
        fprintf(stderr, "%s: error in executing %s script\n",
                        __FUNCTION__, langname);
        return false;
        }
        
    delete interp;
    
    return true;
}

/**
 *
 */
bool InkscapeScript::interpretUri(Glib::ustring &uri,
                                 Glib::ustring &output,
                                 Glib::ustring &error,
                                 ScriptLanguage language)
{

    InkscapeInterpreter *interp = NULL;
    //if() instead of switch() lets us scope vars
    if (language == InkscapeScript::PERL)
        {
#ifdef WITH_PERL
        interp = new InkscapePerl();
#endif
        }
    else if (language == InkscapeScript::PYTHON)
        {
#ifdef WITH_PYTHON
        interp = new InkscapePython();
#endif
        }
    else
        {
        //replace with g_error
        fprintf(stderr, "Unknown Script Language type:%d\n", language);
        return false;
        }
        
    if (!interp)
        return false;

    if (!interp->interpretUri(uri, output, error))
        {
        fprintf(stderr, "error in executing script '%s'\n", uri.raw().c_str());
        return false;
        }
        
    return true;
}









}  // namespace Script
}  // namespace Extension
}  // namespace Inkscape

//#########################################################################
//# E N D    O F    F I L E
//#########################################################################

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
