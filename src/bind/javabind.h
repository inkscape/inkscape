#ifndef __JAVABIND_H__
#define __JAVABIND_H__
/**
 * This is a simple mechanism to bind Inkscape to Java, and thence
 * to all of the nice things that can be layered upon that. 
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2007-2008 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <glibmm.h>
#include <vector>


namespace Inkscape
{

namespace Bind
{


/**
 * Select which String implementation we want to use
 */
typedef Glib::ustring String;


/**
 * This is the base class of all things which will be C++ object
 * instances
 */  
class BaseObject
{
public:

    /**
     * Simple constructor
     */	     
    BaseObject()
        {}

    /**
     * Destructor
     */	     
    virtual ~BaseObject()
        {}

};


/**
 *
 */
class Value
{
public:

    /**
     * Types for this value
     */
    typedef enum
        {
        BIND_VOID,
        BIND_INT,
        BIND_BOOLEAN,
        BIND_DOUBLE,
        BIND_STRING,
        BIND_OBJECT
        } ValueType;

    /**
     *
     */
    Value()
        {
        init();
        }

    /**
     *
     */
    Value(const Value &other)
        {
        assign(other);
        }

    /**
     *
     */
    Value &operator=(const Value &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    virtual ~Value()
        {
        }
        
    /**
     *
     */
    int getType()
        { return type; }
                
    /**
     *
     */
    void setBoolean(bool val)
        { type = BIND_BOOLEAN; ival = (int)val; }

    /**
     *
     */
    bool getBoolean()
        {
        if (type == BIND_BOOLEAN)
            return (bool)ival;
        else
            return false;
        }

    /**
     *
     */
    void setInt(int val)
        { type = BIND_INT; ival = val; }

    /**
     *
     */
    bool getInt()
        {
        if (type == BIND_INT)
            return ival;
        else
            return 0;
        }

    /**
     *
     */
    void setDouble(double val)
        { type = BIND_DOUBLE; dval = val; }

    /**
     *
     */
    double getDouble()
        {
        if (type == BIND_DOUBLE)
            return dval;
        else
            return 0.0;
        }

    /**
     *
     */
    void setString(const String &val)
        { type = BIND_STRING; sval = val; }
                
    /**
     *
     */
    String getString()
        {
        if (type == BIND_STRING)
            return sval;
        else
            return "";
        }


private:

    void init()
        {
        type = BIND_INT;
        ival = 0;
        dval = 0.0;
        sval = "";
        }

    void assign(const Value &other)
        {
        type = other.type;
        ival = other.ival;
        dval = other.dval;
        sval = other.sval;
        }

    int    type;
    long   ival;
    double dval;
    String sval;

};





/**
 *
 */
class JavaBindery
{
public:

    /**
     *
     */
    JavaBindery()
        {}
    
    /**
     *
     */
    virtual ~JavaBindery()
        {}
    
    /**
     *
     */
    virtual bool loadJVM()
        {
        return false;
        }
    
    /**
     *
     */
    virtual bool callStatic(int /*type*/,
                            const String &/*className*/,
                            const String &/*methodName*/,
                            const String &/*signature*/,
                            const std::vector<Value> &/*params*/,
                            Value &/*retval*/)
        {
        return false;
        }

    /**
     *
     */
    virtual bool callMain(const String &/*className*/,
	                      const std::vector<String> &/*args*/)
        {
        return false;
        }

    /**
     *
     */
    virtual bool isLoaded()
        {
        return false;
        }

    /**
     *
     */
    virtual bool doBinding()
        {
        return false;
        }
        
    /**
     *
     */
    virtual String getException()
        {
		return "";
		}
        
    virtual String stdOutGet()
        {
        return stdOutBuf;
        }

    virtual void stdOutClear()
        {
        stdOutBuf.clear();
        }

    virtual void stdOut(int ch)
        {
        stdOutBuf.push_back((char)ch);
        }

    virtual String stdErrGet()
        {
        return stdErrBuf;
        }

    virtual void stdErrClear()
        {
        stdErrBuf.clear();
        }

    virtual void stdErr(int ch)
        {
        stdErrBuf.push_back((char)ch);
        }

    virtual String logGet()
        {
        return logBuf;
        }

    virtual void logClear()
        {
        logBuf.clear();
        }

    virtual void log(int ch)
        {
        logBuf.push_back((char)ch);
        if (ch == '\n' || ch == '\r')
            {
            g_message("%s", logBuf.c_str());
            logBuf.clear();
			}
        }


    /**
     *  Return a singleton instance of this bindery
     */
    static JavaBindery *getInstance();
    
protected:


    String stdOutBuf;
    String stdErrBuf;
    String logBuf;

};





} // namespace Bind
} // namespace Inkscape

#endif  /* __JAVABIND_H__ */
//########################################################################
//# E N D    O F    F I L E
//########################################################################

