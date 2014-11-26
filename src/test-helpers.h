#ifndef SEEN_TEST_HELPERS_H
#define SEEN_TEST_HELPERS_H


#include <cxxtest/TestSuite.h>

#include "document.h"
#include "inkscape.h"


// Dummy functions to keep linker happy
#if !defined(DUMMY_MAIN_TEST_CALLS_SEEN)
#define DUMMY_MAIN_TEST_CALLS_SEEN
int sp_main_gui (int, char const**) { return 0; }
int sp_main_console (int, char const**) { return 0; }
#endif // DUMMY_MAIN_TEST_CALLS_SEEN

namespace Inkscape
{

template <class T>
T* createSuiteAndDocument( void (*fun)(T*&) )
{
    T* suite = 0;

#if !GLIB_CHECK_VERSION(2,36,0)
    g_type_init();
#endif

    Inkscape::GC::init();
    if ( !Inkscape::Application::exists() )
    {
        // Create the global inkscape object.
        Inkscape::Application::create("", false);
    }

    SPDocument* tmp = SPDocument::createNewDoc( NULL, TRUE, true );
    if ( tmp ) {
        fun( suite );
        if ( suite )
        {
            suite->_doc = tmp;
        }
        else
        {
            tmp->doUnref();
        }
    }

    return suite;
}

} // namespace Inkscape

#endif // SEEN_TEST_HELPERS_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
