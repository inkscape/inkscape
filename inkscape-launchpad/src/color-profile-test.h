
#ifndef SEEN_COLOR_PROFILE_TEST_H
#define SEEN_COLOR_PROFILE_TEST_H

#include <cxxtest/TestSuite.h>
#include <cassert>

#include "test-helpers.h"


#include "color-profile.h"
#include "cms-system.h"

class ColorProfileTest : public CxxTest::TestSuite
{
public:
    SPDocument* _doc;

    ColorProfileTest() :
        _doc(0)
    {
    }

    virtual ~ColorProfileTest()
    {
        if ( _doc )
        {
            _doc->doUnref();
        }
    }

    static void createSuiteSubclass( ColorProfileTest*& dst )
    {
        Inkscape::ColorProfile *prof = new Inkscape::ColorProfile();
        if ( prof ) {
            if ( prof->rendering_intent == (guint)Inkscape::RENDERING_INTENT_UNKNOWN ) {
                TS_ASSERT_EQUALS( prof->rendering_intent, (guint)Inkscape::RENDERING_INTENT_UNKNOWN );
                dst = new ColorProfileTest();
            }
            delete prof;
        }
    }

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static ColorProfileTest *createSuite()
    {
        ColorProfileTest* suite = Inkscape::createSuiteAndDocument<ColorProfileTest>( createSuiteSubclass );
        return suite;
    }

    static void destroySuite( ColorProfileTest *suite )
    {
        delete suite; 
    }

    // ---------------------------------------------------------------
    // ---------------------------------------------------------------
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

        Inkscape::ColorProfile *prof = new Inkscape::ColorProfile();
        TS_ASSERT( prof );
        SP_OBJECT(prof)->document = _doc;

        for ( size_t i = 0; i < G_N_ELEMENTS( cases ); i++ ) {
            std::string descr(cases[i].attr);
            SP_OBJECT(prof)->setKeyValue( SP_ATTR_RENDERING_INTENT, cases[i].attr);
            TSM_ASSERT_EQUALS( descr, prof->rendering_intent, (guint)cases[i].intVal );
        }

        delete prof;
    }

    void testSetLocal()
    {
        gchar const* cases[] = {
            "local",
            "something",
        };

        Inkscape::ColorProfile *prof = new Inkscape::ColorProfile();
        TS_ASSERT( prof );
        SP_OBJECT(prof)->document = _doc;

        for ( size_t i = 0; i < G_N_ELEMENTS( cases ); i++ ) {
            SP_OBJECT(prof)->setKeyValue( SP_ATTR_LOCAL, cases[i]);
            TS_ASSERT( prof->local );
            if ( prof->local ) {
                TS_ASSERT_EQUALS( std::string(prof->local), std::string(cases[i]) );
            }
        }
        SP_OBJECT(prof)->setKeyValue( SP_ATTR_LOCAL, NULL);
        TS_ASSERT_EQUALS( prof->local, (gchar*)0 );

        delete prof;
    }

    void testSetName()
    {
        gchar const* cases[] = {
            "name",
            "something",
        };

        Inkscape::ColorProfile *prof = new Inkscape::ColorProfile();
        TS_ASSERT( prof );
        SP_OBJECT(prof)->document = _doc;

        for ( size_t i = 0; i < G_N_ELEMENTS( cases ); i++ ) {
            SP_OBJECT(prof)->setKeyValue( SP_ATTR_NAME, cases[i]);
            TS_ASSERT( prof->name );
            if ( prof->name ) {
                TS_ASSERT_EQUALS( std::string(prof->name), std::string(cases[i]) );
            }
        }
        SP_OBJECT(prof)->setKeyValue( SP_ATTR_NAME, NULL);
        TS_ASSERT_EQUALS( prof->name, (gchar*)0 );

        delete prof;
    }
};

#endif // SEEN_COLOR_PROFILE_TEST_H

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
