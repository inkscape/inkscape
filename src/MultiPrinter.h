#ifndef SEEN_MULTI_PRINTER_H
#define SEEN_MULTI_PRINTER_H


#include <cxxtest/Flags.h>

#ifndef _CXXTEST_HAVE_STD
#   define _CXXTEST_HAVE_STD
#endif // _CXXTEST_HAVE_STD

#include <cxxtest/ErrorFormatter.h>
#include <cxxtest/StdValueTraits.h>

#ifdef _CXXTEST_OLD_STD
#   include <iostream.h>
#   include <fstream.h>
#   include <string.h>
#else // !_CXXTEST_OLD_STD
#   include <iostream>
#   include <fstream>
#   include <string>
#endif // _CXXTEST_OLD_STD


#include <cxxtest/TeeListener.h>
#include "TRPIFormatter.h"
#include "PylogFormatter.h"

namespace CxxTest {

class MultiPrinter : public TeeListener
{
public:
    MultiPrinter( const char* baseName = "result" ) :
        TeeListener(),
        _baseName( baseName ),
        _xmlName( _baseName + ".xml" ),
        _logName( _baseName + ".log" ),
        _xmlFile( _xmlName.c_str(), CXXTEST_STD(ios::out)),
        _logFile( _logName.c_str(), CXXTEST_STD(ios::out)),
        _dstOne( new FileAdapter( CXXTEST_STD(cout) ) ),
        _dstXml( new FileAdapter( _xmlFile ) ),
        _dstPylog( new FileAdapter( _logFile ), _baseName.c_str() )
    {
        setFirst( _dstOne );
        setSecond( _subTee );
        _subTee.setFirst( _dstXml );
        _subTee.setSecond( _dstPylog );
    }

    virtual ~MultiPrinter()
    {
        _xmlFile.close();
        _logFile.close();
    }

    int run()
    {
        TestRunner::runAllTests( *this );
        return tracker().failedTests();
    }

protected:
    CXXTEST_STD(string) _baseName;
    CXXTEST_STD(string) _xmlName;
    CXXTEST_STD(string) _logName;
    CXXTEST_STD(fstream) _xmlFile;
    CXXTEST_STD(fstream) _logFile;

    TeeListener _subTee;
    ErrorFormatter _dstOne;
    TRPIFormatter _dstXml;
    PylogFormatter _dstPylog;

private:
    class FileAdapter : public OutputStream
    {
        FileAdapter( const FileAdapter & );
        FileAdapter &operator=( const FileAdapter & );

        CXXTEST_STD(ostream) &_o;

    public:
        FileAdapter( CXXTEST_STD(ostream) &o ) : _o(o) {}
        void flush() { _o.flush(); }
        OutputStream &operator<<( const char *s ) { _o << s; return *this; }
        OutputStream &operator<<( Manipulator m ) { return OutputStream::operator<<( m ); }
        OutputStream &operator<<( unsigned i )
        {
            char s[1 + 3 * sizeof(unsigned)];
            numberToString( i, s );
            _o << s;
            return *this;
        }
    };

};

}

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

#endif //SEEN_MULTI_PRINTER_H
