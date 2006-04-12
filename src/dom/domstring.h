#ifndef __DOMSTRING_H__
#define __DOMSTRING_H__
/**
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2005 Bob Jamison
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


#include <vector>
#include <string>

namespace org
{
namespace w3c
{
namespace dom
{

/**
 *
 */
typedef unsigned short XMLCh;

class DOMString
{
public:

    //###############################
    //# C O N S T R U C T O R S
    //###############################

    /**
     *
     */
    DOMString();

    /**
     *
     */
    DOMString(const char *str);


    /**
     *
     */
    DOMString(const DOMString &str);


    /**
     *
     */
    DOMString(const std::string &str);

    /**
     *
     */
    virtual ~DOMString();


    //###############################
    //# M O D I F Y
    //###############################



    /**
     *
     */
    virtual DOMString &append(const DOMString &str);
    virtual DOMString &operator +(const DOMString &str)
        { return append(str); }
    virtual DOMString &operator +=(const DOMString &str)
        { return append(str); }

    /**
     *
     */
    virtual DOMString &append(const char *str);

    /**
     *
     */
    virtual DOMString &append(const std::string &str);


    /**
     *
     */
    virtual DOMString &assign(const DOMString &str);

    /**
     *
     */
    DOMString &operator =(const DOMString &a);

    /**
     *
     */
    virtual DOMString &assign(const char *str);

    /**
     *
     */
    virtual DOMString &assign(const std::string &str);

    /**
     *
     */
    virtual void erase(unsigned long pos, unsigned long count);

    /**
     *
     */
    virtual DOMString &insert(unsigned long pos, const DOMString &str);

    /**
     *
     */
    virtual DOMString &insert(unsigned long pos, const char *str);

    /**
     *
     */
    virtual DOMString &insert(unsigned long pos, const std::string &str);


    /**
     *
     */
    virtual DOMString &prepend(const DOMString &str);

    /**
     *
     */
    virtual DOMString &prepend(const char *str);

    /**
     *
     */
    virtual DOMString &prepend(const std::string &str);


    /**
     *
     */
    virtual DOMString &replace(unsigned long pos, unsigned long count,
                               const DOMString &str);

    /**
     *
     */
    virtual DOMString &replace(unsigned long pos, unsigned long count,
                               const char *str);

    /**
     *
     */
    virtual DOMString &replace(unsigned long pos, unsigned long count,
                               const std::string &str);

    /**
     *
     */
    virtual DOMString &push_back(XMLCh ch);

    /**
     *
     */
    virtual void clear();

    //###############################
    //# Q U E R Y
    //###############################

    /**
     *
     */
    virtual DOMString substr(unsigned long  start, unsigned long end) const;

    /**
     *
     */
    virtual XMLCh charAt(unsigned long index) const;

    /**
     *
     */
    virtual XMLCh operator[](unsigned long index) const;

    /**
     *
     */
    virtual unsigned long size() const;

    /**
     *
     */
    virtual const char *c_str();

    //###############################
    //# C O M P A R I S O N
    //###############################

    /**
     *
     */
    virtual int compare(const DOMString &str) const;
    virtual bool operator <(const DOMString &str) const
        { return (compare(str)<0) ; }
    virtual bool operator <=(const DOMString &str) const
        { return (compare(str)<=0) ; }
    virtual bool operator >(const DOMString &str) const
        { return (compare(str)>0) ; }
    virtual bool operator >=(const DOMString &str) const
        { return (compare(str)>=0) ; }
    virtual bool operator !=(const DOMString &str) const
        { return (compare(str)!=0) ; }
    virtual bool operator ==(const DOMString &str) const
        { return (compare(str)==0) ; }

    /**
     *
     */
    virtual int compare(const char *str) const;
    virtual bool operator <(const char *str) const
        { return (compare(str)<0) ; }
    virtual bool operator <=(const char *str) const
        { return (compare(str)<=0) ; }
    virtual bool operator >(const char *str) const
        { return (compare(str)>0) ; }
    virtual bool operator >=(const char *str) const
        { return (compare(str)>=0) ; }
    virtual bool operator !=(const char *str) const
        { return (compare(str)!=0) ; }
    virtual bool operator ==(const char *str) const
        { return (compare(str)==0) ; }

    /**
     *
     */
    virtual int compare(const std::string &str) const;
    virtual bool operator <(const std::string &str) const
        { return (compare(str)<0) ; }
    virtual bool operator <=(const std::string &str) const
        { return (compare(str)<=0) ; }
    virtual bool operator >(const std::string &str) const
        { return (compare(str)>0) ; }
    virtual bool operator >=(const std::string &str) const
        { return (compare(str)>=0) ; }
    virtual bool operator !=(const std::string &str) const
        { return (compare(str)!=0) ; }
    virtual bool operator ==(const std::string &str) const
        { return (compare(str)==0) ; }




protected:

    void init();

    char *cstring;

    std::vector<XMLCh> chars;

};  // class DOMString




//###############################
//# O P E R A T O R S
//###############################

DOMString &operator +(DOMString &a, const char *b);

DOMString &operator +(const char *b, DOMString &a);



}  //namespace dom
}  //namespace w3c
}  //namespace org

#endif // __DOMSTRING_H__
//#########################################################################
//## E N D    O F    F I L E
//#########################################################################


