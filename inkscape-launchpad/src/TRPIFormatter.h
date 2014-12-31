#ifndef TRPI_FORMATTER_H_SEEN
#define TRPI_FORMATTER_H_SEEN

#include <cxxtest/Flags.h>

#ifndef _CXXTEST_HAVE_STD
#   define _CXXTEST_HAVE_STD
#endif // _CXXTEST_HAVE_STD

#include <cxxtest/ErrorFormatter.h>
#include <cxxtest/StdValueTraits.h>

#ifdef _CXXTEST_OLD_STD
#   include <iostream.h>
#else // !_CXXTEST_OLD_STD
#   include <iostream>
#endif // _CXXTEST_OLD_STD

namespace CxxTest
{
class TRPIFormatter : public TestListener
{
public:
    TRPIFormatter( OutputStream *o ) :
        _o(o),
        _runPassed(true),
        _suiteIndex(-1),
        _testIndex(-1)
    {}
    virtual ~TRPIFormatter() { delete outputStream(); }

    virtual void enterWorld( const WorldDescription & /*desc*/ )
    {
        _status.clear();

        (*_o) << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << endl;
        (*_o) << "<component name=\"inkscape\" version=\"0.43+svn\" xmlns=\"http://www.spikesource.com/xsd/2005/04/TRPI\">" << endl;
        (*_o) << "    <description>TBD</description>" << endl;
        (*_o) << "    <summary>single-line info</summary>" << endl;
        (*_o) << "    <license></license>" << endl;
        (*_o) << "    <vendor></vendor>" << endl;
        (*_o) << "    <release>devel SVN</release>" << endl;
        (*_o) << "    <url>http://www.inkscape.org/</url>" << endl;
//         (*_o) << "    <root></root>" << endl;
        (*_o) << "    <platform>\?\?\?</platform>" << endl;
        _o->flush();
    }

    virtual void leaveWorld( const WorldDescription & desc )
    {
        (*_o) << "    <build status=\"" << (_runPassed?"pass":"fail") <<  "\"/>" << endl;

        for ( unsigned int i = 0; i < desc.numSuites(); i++ ) {
            const SuiteDescription& suite = desc.suiteDescription(i);
            for ( unsigned int j = 0; j < suite.numTests(); j++ ) {
                const TestDescription& test = suite.testDescription(j);
                (*_o) << "    <test suite-type=\"unit\">" << endl;
                (*_o) << "        <result executed=\"" << (unsigned)1
                      << "\" passed=\"" << (unsigned)(_status[i][j] ? 1:0)
                      << "\" failed=\"" << (unsigned)(_status[i][j] ? 0:1)
                      << "\" skipped=\"" << (unsigned)0
                      << "\"/>" << endl;

                (*_o) << "        <suiteName>" << test.suiteName() << "</suiteName>" << endl;
                (*_o) << "        <testName>" << test.testName() << "</testName>" << endl;

//                 (*_o) << "        <report name=\"" << test.suiteName() << "|" << test.testName() << "\" path=\"index.html\"/>" << endl;

                (*_o) << "        <expected-result executed=\"" << (unsigned)1
                      << "\" passed=\"" << (unsigned)1
                      << "\" failed=\"" << (unsigned)0
                      << "\" skipped=\"" << (unsigned)0
                      << "\"/>" << endl;
                (*_o) << "    </test>" << endl;
            }
        }

//         (*_o) << "    <coverage-report />" << endl;
//         (*_o) << "    <code-convention-report />" << endl;
        (*_o) << "</component>" << endl;
    }

    virtual void enterSuite( const SuiteDescription & desc )
    {
        (void)desc;
        _suiteIndex++;
        _testIndex = -1;
        while ( (_suiteIndex >= 0) && ((int)_status.size() <= _suiteIndex) ) {
            std::vector<bool> tmp;
            _status.push_back(tmp);
        }
    }

    virtual void leaveSuite( const SuiteDescription & desc )
    {
        (void)desc;
    }

    virtual void enterTest( const TestDescription & desc )
    {
        (void)desc;
        if ( _suiteIndex >= 0 && (int)_status.size() > _suiteIndex ) {
            _testIndex++;
            while ( (_testIndex >= 0) && ((int)_status[_suiteIndex].size() <= _testIndex) ) {
                bool tmp = true;
                _status[_suiteIndex].push_back(tmp);
            }
        }
    }

    virtual void leaveTest( const TestDescription & desc )
    {
        (void)desc;
    }



    virtual void failedTest( const char * /*file*/, unsigned /*line*/,
                             const char * /*expression*/ ) { _failCurrent(); }
    virtual void failedAssert( const char * /*file*/, unsigned /*line*/,
                               const char * /*expression*/ ) { _failCurrent(); }
    virtual void failedAssertEquals( const char * /*file*/, unsigned /*line*/,
                                     const char * /*xStr*/, const char * /*yStr*/,
                                     const char * /*x*/, const char * /*y*/ ) { _failCurrent(); }
    virtual void failedAssertSameData( const char * /*file*/, unsigned /*line*/,
                                       const char * /*xStr*/, const char * /*yStr*/,
                                       const char * /*sizeStr*/, const void * /*x*/,
                                       const void * /*y*/, unsigned /*size*/ ) { _failCurrent(); }
    virtual void failedAssertDelta( const char * /*file*/, unsigned /*line*/,
                                    const char * /*xStr*/, const char * /*yStr*/,
                                    const char * /*dStr*/, const char * /*x*/,
                                    const char * /*y*/, const char * /*d*/ ) { _failCurrent(); }
    virtual void failedAssertDiffers( const char * /*file*/, unsigned /*line*/,
                                      const char * /*xStr*/, const char * /*yStr*/,
                                      const char * /*value*/ ) { _failCurrent(); }
    virtual void failedAssertLessThan( const char * /*file*/, unsigned /*line*/,
                                       const char * /*xStr*/, const char * /*yStr*/,
                                       const char * /*x*/, const char * /*y*/ ) { _failCurrent(); }
    virtual void failedAssertLessThanEquals( const char * /*file*/, unsigned /*line*/,
                                             const char * /*xStr*/, const char * /*yStr*/,
                                             const char * /*x*/, const char * /*y*/ ) { _failCurrent(); }
    virtual void failedAssertPredicate( const char * /*file*/, unsigned /*line*/,
                                        const char * /*predicate*/, const char * /*xStr*/, const char * /*x*/ ) { _failCurrent(); }
    virtual void failedAssertRelation( const char * /*file*/, unsigned /*line*/,
                                       const char * /*relation*/, const char * /*xStr*/, const char * /*yStr*/,
                                       const char * /*x*/, const char * /*y*/ ) { _failCurrent(); }
    virtual void failedAssertThrows( const char * /*file*/, unsigned /*line*/,
                                     const char * /*expression*/, const char * /*type*/,
                                     bool /*otherThrown*/ ) { _failCurrent(); }
    virtual void failedAssertThrowsNot( const char * /*file*/, unsigned /*line*/,
                                        const char * /*expression*/ ) { _failCurrent(); }

protected:
    OutputStream *outputStream() const
    {
        return _o;
    }

    void _failCurrent() {
        _runPassed = false;
        if ( _suiteIndex < (int)_status.size() ) {
            if ( _testIndex < (int)_status[_suiteIndex].size() ) {
                _status[_suiteIndex][_testIndex] = false;
            }
        }
    }

private:
    static void endl( OutputStream &o )
    {
        OutputStream::endl( o );
    }

    OutputStream *_o;
    std::vector< std::vector<bool> > _status;
    bool _runPassed;
    int _suiteIndex;
    int _testIndex;
};

} // namespace CxxTest

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

#endif // TRPI_FORMATTER_H_SEEN
