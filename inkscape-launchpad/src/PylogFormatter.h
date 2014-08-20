#ifndef PYLOG_FORMATTER_H_SEEN
#define PYLOG_FORMATTER_H_SEEN

#include <cstring>
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
class PylogFormatter : public TestListener
{
public:
    PylogFormatter( OutputStream *o, const char* name = "test" ) :
        _o(o),
        _base( _chompPath( name ) ),
        _runPassed(true),
        _suiteIndex(-1),
        _testIndex(-1)
    {}
    virtual ~PylogFormatter() { delete outputStream(); }

    virtual void enterWorld( const WorldDescription & /*desc*/ )
    {
        (*_o) << "**************************************************" << endl;
        _o->flush();
    }

    virtual void leaveWorld( const WorldDescription & desc )
    {
        using std::strlen;
        unsigned int skippedCount = 0;
        unsigned int failedCount = 0;
        unsigned int warnedCount = 0;
        unsigned int passedCount = 0;

        for ( unsigned int i = 0; i < desc.numSuites(); i++ ) {
            const SuiteDescription& suite = desc.suiteDescription(i);
            for ( unsigned int j = 0; j < suite.numTests(); j++ ) {
                const TestDescription& test = suite.testDescription(j);

                // Test Name
                (*_o) << _base.c_str() << "_";
                _padOut( i, 3 );
                (*_o) << "_";
                _padOut( j, 3);
                (*_o) << "    ";

                // Test Description
                (*_o) << test.suiteName() << "_|_" << test.testName();
                (*_o) << " ";

                unsigned const sent = strlen( test.suiteName() ) + strlen( test.testName() ) + 1;
                for ( unsigned z = sent; z < 56; z++ ) {
                    (*_o) << " ";
                }

                (*_o) << " : ";

                switch ( _status[i][j] ) {
                    case OK:
                        (*_o) << "PASS";
                        passedCount++;
                        break;
                    case SKIPPED:
                        (*_o) << "OMIT";
                        skippedCount++;
                        break;
                    case WARNING:
                        (*_o) << "WARNING";
                        warnedCount++;
                        break;
                    case ERROR:
                        (*_o) << "FAILURE";
                        failedCount++;
                        break;
                }

                (*_o) << endl;
                for ( CXXTEST_STD(vector)<CXXTEST_STD(string)>::iterator it = _messages[i][j].begin(); it < _messages[i][j].end(); ++it ) {
                    (*_o) << "        " << (*it).c_str() << endl;
                }
            }
        }

        (*_o) << "**************************************************" << endl;
        (*_o) << "Command line asked for " << desc.numTotalTests() << " of " << desc.numTotalTests() << " tests" << endl;
        (*_o) << "Of those: "
              << skippedCount << " Skipped, "
              << failedCount << " Failed, "
              << warnedCount << " Warned, "
              << passedCount << " Passed"
              << endl;
    }


    virtual void enterSuite( const SuiteDescription & desc )
    {
        (void)desc;
        _suiteIndex++;
        _testIndex = -1;
        while ( (_suiteIndex >= 0) && ((int)_status.size() <= _suiteIndex) ) {
            CXXTEST_STD(vector)<ErrorLevel> tmp;
            _status.push_back(tmp);
            CXXTEST_STD(vector)<CXXTEST_STD(vector)<CXXTEST_STD(string)> > tmp2;
            _messages.push_back(tmp2);
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
                ErrorLevel tmp = OK;
                _status[_suiteIndex].push_back(tmp);
                CXXTEST_STD(vector)<CXXTEST_STD(string)> tmp2;
                _messages[_suiteIndex].push_back(tmp2);
            }
        }
    }

    virtual void leaveTest( const TestDescription & desc )
    {
        (void)desc;
    }

    virtual void trace( const char * file, unsigned line,
                        const char * expression )
    {
        CXXTEST_STD(string)tmp(expression);
        _traceCurrent( file, line, tmp );
    }

    virtual void warning( const char * file, unsigned line,
                          const char * expression )
    {
        CXXTEST_STD(string)tmp(expression);
        _warnCurrent( file, line, tmp );
    }

    virtual void failedTest( const char * file, unsigned line,
                             const char * expression )
    {
        CXXTEST_STD(string)tmp(expression);
        _failCurrent( file, line, tmp );
    }

    virtual void failedAssert( const char * file, unsigned line,
                               const char * expression )
    {
        CXXTEST_STD(string)tmp(expression);
        _failCurrent( file, line, tmp );
    }

    virtual void failedAssertEquals( const char * file, unsigned line,
                                     const char * xStr, const char * yStr,
                                     const char * x, const char * y )
    {
        CXXTEST_STD(string)tmp;
        tmp += "Expected (";
        tmp += xStr;
        tmp += " == ";
        tmp += yStr;
        tmp += "), found (";
        tmp += x;
        tmp += " != ";
        tmp += y;
        tmp += ")";
        _failCurrent( file, line, tmp );
    }

    virtual void failedAssertSameData( const char * file, unsigned line,
                                       const char * /*xStr*/, const char * /*yStr*/,
                                       const char * /*sizeStr*/, const void * /*x*/,
                                       const void * /*y*/, unsigned /*size*/ )
    {
        CXXTEST_STD(string)tmp("TODO - fill in error details");
        _failCurrent( file, line, tmp );
    }

    virtual void failedAssertDelta( const char * file, unsigned line,
                                    const char * /*xStr*/, const char * /*yStr*/,
                                    const char * /*dStr*/, const char * /*x*/,
                                    const char * /*y*/, const char * /*d*/ )
    {
        CXXTEST_STD(string)tmp("TODO - fill in error details");
        _failCurrent( file, line, tmp );
    }

    virtual void failedAssertDiffers( const char * file, unsigned line,
                                      const char * xStr, const char * yStr,
                                      const char * value )
    {
        CXXTEST_STD(string)tmp;
        tmp += "Expected (";
        tmp += xStr;
        tmp += " != ";
        tmp += yStr;
        tmp += "), found (";
        tmp += value;
        tmp += ")";
        _failCurrent( file, line, tmp );
    }

    virtual void failedAssertLessThan( const char * file, unsigned line,
                                       const char * xStr, const char * yStr,
                                       const char * x, const char * y )
    {
        CXXTEST_STD(string)tmp;
        tmp += "Expected (";
        tmp += xStr;
        tmp += " < ";
        tmp += yStr;
        tmp += "), found (";
        tmp += x;
        tmp += " >= ";
        tmp += y;
        tmp += ")";
        _failCurrent( file, line, tmp );
    }

    virtual void failedAssertLessThanEquals( const char * file, unsigned line,
                                             const char * xStr, const char * yStr,
                                             const char * x, const char * y )
    {
        CXXTEST_STD(string)tmp;
        tmp += "Expected (";
        tmp += xStr;
        tmp += " <= ";
        tmp += yStr;
        tmp += "), found (";
        tmp += x;
        tmp += " > ";
        tmp += y;
        tmp += ")";
        _failCurrent( file, line, tmp );
    }

    virtual void failedAssertPredicate( const char * file, unsigned line,
                                        const char * /*predicate*/, const char * /*xStr*/, const char * /*x*/ )
    {
        CXXTEST_STD(string)tmp("TODO - fill in error details");
        _failCurrent( file, line, tmp );
    }

    virtual void failedAssertRelation( const char * file, unsigned line,
                                       const char * /*relation*/, const char * /*xStr*/, const char * /*yStr*/,
                                       const char * /*x*/, const char * /*y*/ )
    {
        CXXTEST_STD(string)tmp("TODO - fill in error details");
        _failCurrent( file, line, tmp);
    }

    virtual void failedAssertThrows( const char * file, unsigned line,
                                     const char * /*expression*/, const char * /*type*/,
                                     bool /*otherThrown*/ )
    {
        CXXTEST_STD(string)tmp("TODO - fill in error details");
        _failCurrent( file, line, tmp );
    }

    virtual void failedAssertThrowsNot( const char * file, unsigned line,
                                        const char * expression )
    {
        CXXTEST_STD(string)tmp(expression);
        _failCurrent( file, line, tmp );
    }

protected:

    enum ErrorLevel {
        OK,
        SKIPPED,
        WARNING,
        ERROR
    };

    OutputStream *outputStream() const
    {
        return _o;
    }

    void _traceCurrent( const char* /*file*/, unsigned /*line*/, const CXXTEST_STD(string)& errMsg ) {
        _runPassed = false;
        if ( _suiteIndex < (int)_status.size() ) {
            if ( _testIndex < (int)_status[_suiteIndex].size() ) {
                _messages[_suiteIndex][_testIndex].push_back( errMsg );
            }
        }
    }

    void _warnCurrent( const char* /*file*/, unsigned /*line*/, const CXXTEST_STD(string)& errMsg ) {
        _runPassed = false;
        if ( _suiteIndex < (int)_status.size() ) {
            if ( _testIndex < (int)_status[_suiteIndex].size() ) {
                if ( _status[_suiteIndex][_testIndex] != ERROR ) {
                    _status[_suiteIndex][_testIndex] = WARNING;
                }

                _messages[_suiteIndex][_testIndex].push_back( errMsg );
            }
        }
    }

    void _failCurrent( const char* /*file*/, unsigned /*line*/, const CXXTEST_STD(string)& errMsg ) {
        _runPassed = false;
        if ( _suiteIndex < (int)_status.size() ) {
            if ( _testIndex < (int)_status[_suiteIndex].size() ) {
                _status[_suiteIndex][_testIndex] = ERROR;

                _messages[_suiteIndex][_testIndex].push_back( errMsg );
            }
        }
    }

    void _padOut( unsigned int num, int digits )
    {
        int match = 1;
        for ( int j = 1; j < digits; j++ ) {
            match *= 10;
        }

        for ( unsigned int i = match; i > 1 && i > num; i /= 10 )
        {
            (*_o) << "0";
        }
        (*_o) << num;
    }

private:
    static void endl( OutputStream &o )
    {
        OutputStream::endl( o );
    }

    static CXXTEST_STD(string) _chompPath( const char* str )
    {
        CXXTEST_STD(string) tmp( str );
        if ( tmp.length() > 2 && tmp[0] == '.' && tmp[1] == '/' ) {
            tmp = tmp.substr( 2 );
        }
        return tmp;
    }

    OutputStream *_o;
    CXXTEST_STD(string) _base;
    CXXTEST_STD(vector)< CXXTEST_STD(vector)<ErrorLevel> > _status;
    CXXTEST_STD(vector)< CXXTEST_STD(vector)< CXXTEST_STD(vector)<CXXTEST_STD(string)> > > _messages;

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

#endif // PYLOG_FORMATTER_H_SEEN
