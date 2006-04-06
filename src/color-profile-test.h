

#include <cxxtest/TestSuite.h>
#include <cassert>

#include "inkscape-private.h"
#include "sp-object.h"
#include "document.h"

#include "color-profile.h"
#include "color-profile-fns.h"

using Inkscape::ColorProfile;

/// Dummy functions to keep linker happy
#if !defined(DUMMY_MAIN_TEST_CALLS_SEEN)
#define DUMMY_MAIN_TEST_CALLS_SEEN
int sp_main_gui (int, char const**) { return 0; }
int sp_main_console (int, char const**) { return 0; }
#endif // DUMMY_MAIN_TEST_CALLS_SEEN

class ColorProfileTest : public CxxTest::TestSuite
{
public:

    ColorProfileTest() :
        TestSuite(),
        _doc(0)
    {
    }
    virtual ~ColorProfileTest() {}

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static ColorProfileTest *createSuite()
    {
        ColorProfileTest* suite = 0;
        bool canRun = false;

        g_type_init();
        Inkscape::GC::init();

        ColorProfile *prof = static_cast<ColorProfile *>(g_object_new(COLORPROFILE_TYPE, NULL));
        canRun = prof;
        canRun &= prof->rendering_intent == (guint)Inkscape::RENDERING_INTENT_UNKNOWN;
        TS_ASSERT_EQUALS( prof->rendering_intent, (guint)Inkscape::RENDERING_INTENT_UNKNOWN );
        g_object_unref(prof);

        if ( canRun ) {
            // Create the global inkscape object.
            static_cast<void>(g_object_new(inkscape_get_type(), NULL));
            SPDocument* tmp = sp_document_new_dummy();
            if ( tmp ) {
                suite = new ColorProfileTest();
                suite->_doc = tmp;
            }
        }

        return suite;
    }

    static void destroySuite( ColorProfileTest *suite )
    {
        delete suite; 
    }


    SPDocument* _doc;

    // ---------------------------------------------------------------

    void testSetRenderingIntent()
    {
        struct {
            gchar const *attr;
            guint intVal;
        }
        const cases[] = {
            {"auto", (guint)Inkscape::RENDERING_INTENT_AUTO},
            {"perceptual", (guint)Inkscape::RENDERING_INTENT_PERCEPTUAL},
            {"relative-colorimetric", (guint)Inkscape::RENDERING_INTENT_RELATIVE_COLORIMETRIC},
            {"saturation", (guint)Inkscape::RENDERING_INTENT_SATURATION},
            {"absolute-colorimetric", (guint)Inkscape::RENDERING_INTENT_ABSOLUTE_COLORIMETRIC},
            {"something-else", (guint)Inkscape::RENDERING_INTENT_UNKNOWN},
            {"auto2", (guint)Inkscape::RENDERING_INTENT_UNKNOWN},
        };

        ColorProfile *prof = static_cast<ColorProfile *>(g_object_new(COLORPROFILE_TYPE, NULL));
        TS_ASSERT( prof );
        SP_OBJECT(prof)->document = _doc;

        for ( size_t i = 0; i < G_N_ELEMENTS( cases ); i++ ) {
            std::string descr(cases[i].attr);
            sp_object_set(SP_OBJECT(prof), SP_ATTR_RENDERING_INTENT, cases[i].attr);
            TSM_ASSERT_EQUALS( descr, prof->rendering_intent, (guint)cases[i].intVal );
        }

        g_object_unref(prof);
    }

    void testSetLocal()
    {
        gchar const* cases[] = {
            "local",
            "something",
        };

        ColorProfile *prof = static_cast<ColorProfile *>(g_object_new(COLORPROFILE_TYPE, NULL));
        TS_ASSERT( prof );
        SP_OBJECT(prof)->document = _doc;

        for ( size_t i = 0; i < G_N_ELEMENTS( cases ); i++ ) {
            sp_object_set(SP_OBJECT(prof), SP_ATTR_LOCAL, cases[i]);
            TS_ASSERT( prof->local );
            if ( prof->local ) {
                TS_ASSERT_EQUALS( std::string(prof->local), std::string(cases[i]) );
            }
        }
        sp_object_set(SP_OBJECT(prof), SP_ATTR_LOCAL, NULL);
        TS_ASSERT_EQUALS( prof->local, (gchar*)0 );

        g_object_unref(prof);
    }

    void testSetName()
    {
        gchar const* cases[] = {
            "name",
            "something",
        };

        ColorProfile *prof = static_cast<ColorProfile *>(g_object_new(COLORPROFILE_TYPE, NULL));
        TS_ASSERT( prof );
        SP_OBJECT(prof)->document = _doc;

        for ( size_t i = 0; i < G_N_ELEMENTS( cases ); i++ ) {
            sp_object_set(SP_OBJECT(prof), SP_ATTR_NAME, cases[i]);
            TS_ASSERT( prof->name );
            if ( prof->name ) {
                TS_ASSERT_EQUALS( std::string(prof->name), std::string(cases[i]) );
            }
        }
        sp_object_set(SP_OBJECT(prof), SP_ATTR_NAME, NULL);
        TS_ASSERT_EQUALS( prof->name, (gchar*)0 );

        g_object_unref(prof);
    }
};

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
