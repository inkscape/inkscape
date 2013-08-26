/** @file
 * @brief Unit tests for the Preferences object
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * This file is released into the public domain.
 */

#include <cxxtest/TestSuite.h>
#include "preferences.h"

#include <glibmm/ustring.h>

// test observer
class TestObserver : public Inkscape::Preferences::Observer {
public:
    TestObserver(Glib::ustring const &path) :
        Inkscape::Preferences::Observer(path),
        value(0) {}

    virtual void notify(Inkscape::Preferences::Entry const &val)
    {
        value = val.getInt();
    }
    int value;
};

class PreferencesTest : public CxxTest::TestSuite {
public:
    void setUp() {
        prefs = Inkscape::Preferences::get();
    }
    void tearDown() {
        prefs = NULL;
        Inkscape::Preferences::unload();
    }

    void testStartingState()
    {
        TS_ASSERT_DIFFERS(prefs, static_cast<void*>(0));
        TS_ASSERT_EQUALS(prefs->isWritable(), true);
    }

    void testOverwrite()
    {
        prefs->setInt("/test/intvalue", 123);
        prefs->setInt("/test/intvalue", 321);
        TS_ASSERT_EQUALS(prefs->getInt("/test/intvalue"), 321);
    }

    void testDefaultReturn()
    {
        TS_ASSERT_EQUALS(prefs->getInt("/this/path/does/not/exist", 123), 123);
    }

    void testLimitedReturn()
    {
        prefs->setInt("/test/intvalue", 1000);

        // simple case
        TS_ASSERT_EQUALS(prefs->getIntLimited("/test/intvalue", 123, 0, 500), 123);
        // the below may seem quirky but this behaviour is intended
        TS_ASSERT_EQUALS(prefs->getIntLimited("/test/intvalue", 123, 1001, 5000), 123);
        // corner cases
        TS_ASSERT_EQUALS(prefs->getIntLimited("/test/intvalue", 123, 0, 1000), 1000);
        TS_ASSERT_EQUALS(prefs->getIntLimited("/test/intvalue", 123, 1000, 5000), 1000);
    }

    void testKeyObserverNotification()
    {
        Glib::ustring const path = "/some/random/path";
        TestObserver obs("/some/random");
        obs.value = 1;
        prefs->setInt(path, 5);
        TS_ASSERT_EQUALS(obs.value, 1); // no notifications sent before adding

        prefs->addObserver(obs);
        prefs->setInt(path, 10);
        TS_ASSERT_EQUALS(obs.value, 10);
        prefs->setInt("/some/other/random/path", 10);
        TS_ASSERT_EQUALS(obs.value, 10); // value should not change

        prefs->removeObserver(obs);
        prefs->setInt(path, 15);
        TS_ASSERT_EQUALS(obs.value, 10); // no notifications sent after removal
    }

    void testEntryObserverNotification()
    {
        Glib::ustring const path = "/some/random/path";
        TestObserver obs(path);
        obs.value = 1;
        prefs->setInt(path, 5);
        TS_ASSERT_EQUALS(obs.value, 1); // no notifications sent before adding

        prefs->addObserver(obs);
        prefs->setInt(path, 10);
        TS_ASSERT_EQUALS(obs.value, 10);

        // test that filtering works properly
        prefs->setInt("/some/random/value", 1234);
        TS_ASSERT_EQUALS(obs.value, 10);
        prefs->setInt("/some/randomvalue", 1234);
        TS_ASSERT_EQUALS(obs.value, 10);
        prefs->setInt("/some/random/path2", 1234);
        TS_ASSERT_EQUALS(obs.value, 10);

        prefs->removeObserver(obs);
        prefs->setInt(path, 15);
        TS_ASSERT_EQUALS(obs.value, 10); // no notifications sent after removal
    }

    void testPreferencesEntryMethods()
    {
        prefs->setInt("/test/prefentry", 100);
        Inkscape::Preferences::Entry val = prefs->getEntry("/test/prefentry");
        TS_ASSERT(val.isValid());
        TS_ASSERT_EQUALS(val.getPath(), "/test/prefentry");
        TS_ASSERT_EQUALS(val.getEntryName(), "prefentry");
        TS_ASSERT_EQUALS(val.getInt(), 100);
    }
private:
    Inkscape::Preferences *prefs;
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
