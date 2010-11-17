

#include <cxxtest/TestSuite.h>

#include "verbs.h"

class VerbsTest : public CxxTest::TestSuite
{
public:

    class TestHook : public Inkscape::Verb {
    public:
        static int getInternalTableSize() { return _getBaseListSize(); }

    private:
        TestHook();
    };

    void testEnumLength()
    {
        TS_ASSERT_DIFFERS( 0, static_cast<int>(SP_VERB_LAST) );
        TS_ASSERT_EQUALS( static_cast<int>(SP_VERB_LAST) + 1, TestHook::getInternalTableSize() );
    }

    void testEnumFixed()
    {
        TS_ASSERT_EQUALS( 0, static_cast<int>(SP_VERB_INVALID) );
        TS_ASSERT_EQUALS( 1, static_cast<int>(SP_VERB_NONE) );

        TS_ASSERT_DIFFERS( 0, static_cast<int>(SP_VERB_LAST) );
        TS_ASSERT_DIFFERS( 1, static_cast<int>(SP_VERB_LAST) );
    }

    void testFetch()
    {
        for ( int i = 0; i < static_cast<int>(SP_VERB_LAST); i++ )
        {
            char tmp[16];
            snprintf( tmp, sizeof(tmp), "Verb# %d", i );
            tmp[sizeof(tmp)-1] = 0;
            std::string descr(tmp);

            Inkscape::Verb* verb = Inkscape::Verb::get(i);
            TSM_ASSERT( descr, verb );
            if ( verb )
            {
                TSM_ASSERT_EQUALS( descr, verb->get_code(), static_cast<unsigned int>(i) );

                if ( i != static_cast<int>(SP_VERB_INVALID) )
                {
                    TSM_ASSERT( descr, verb->get_id() );
                    TSM_ASSERT( descr, verb->get_name() );

                    Inkscape::Verb* bounced = verb->getbyid( verb->get_id() );
                    // TODO - put this back once verbs are fixed
                    //TSM_ASSERT( descr, bounced );
                    if ( bounced )
                    {
                        TSM_ASSERT_EQUALS( descr, bounced->get_code(), static_cast<unsigned int>(i) );
                    }
                    else
                    {
                        TS_FAIL( std::string("Unable to getbyid() for ") + descr + std::string(" ID: '") + std::string(verb->get_id()) + std::string("'") );
                    }
                }
                else
                {
                    TSM_ASSERT( std::string("SP_VERB_INVALID"), !verb->get_id() );
                    TSM_ASSERT( std::string("SP_VERB_INVALID"), !verb->get_name() );
                }
            }
        }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
