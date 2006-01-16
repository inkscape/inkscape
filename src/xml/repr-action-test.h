#include <cxxtest/TestSuite.h>

class XmlReprActionTest : public CxxTest::TestSuite
{
public:

    XmlReprActionTest()
    {
    }
    virtual ~XmlReprActionTest() {}

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static XmlReprActionTest *createSuite() { return new XmlReprActionTest(); }
    static void destroySuite( XmlReprActionTest *suite ) { delete suite; }

    void testFoo()
    {
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
