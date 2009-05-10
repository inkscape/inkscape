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

#include <stdio.h>
#include <stdlib.h>

#include "domstring.h"

namespace org
{
namespace w3c
{
namespace dom
{


//#########################################################################
//# C O N S T R U C T O R S
//#########################################################################

DOMString::DOMString()
{
    init();
}

DOMString::DOMString(const DOMString &other)
{
    init();
    chars = other.chars;
}

DOMString::DOMString(const char *str)
{
    init();
    append(str);
}


DOMString::~DOMString()
{
    if (cstring)
        delete[] cstring;
}


/**
 * Called only by Constructors
 */
void DOMString::init()
{
    cstring = NULL;
    chars.clear();
}

//#########################################################################
//# M O D I F Y
//#########################################################################

DOMString &DOMString::append(const DOMString &str)
{
    unsigned int len = str.size();
    for (unsigned int i=0 ; i<len ; i++)
        {
        XMLCh ch = str.charAt(i);
        chars.push_back(ch);
        }
    return *this;
}

DOMString &DOMString::append(const char *str)
{
    if (!str)
        return *this;
    char *s = (char *)str;
    while (*s)
        {
        XMLCh ch = (XMLCh) *s++;
        chars.push_back(ch);
        }
    return *this;
}

DOMString &DOMString::append(const std::string &str)
{
    unsigned int len = str.size();
    for (unsigned int i=0 ; i<len ; i++)
        {
        XMLCh ch = str[i];
        chars.push_back(ch);
        }
    return *this;
}

DOMString &DOMString::assign(const DOMString &str)
{
    clear();
    append(str);
    return *this;
}

DOMString &DOMString::operator=(const DOMString &str)
{
    clear();
    append(str);
    return *this;
}

DOMString &DOMString::assign(const char *str)
{
    clear();
    append(str);
    return *this;
}

DOMString &DOMString::assign(const std::string &str)
{
    clear();
    append(str);
    return *this;
}


void DOMString::erase(unsigned long /*pos*/, unsigned long count)
{
    std::vector<XMLCh>::iterator iter = chars.begin();
    chars.erase(iter, iter + count);
}

DOMString &DOMString::insert(unsigned long pos, const DOMString &str)
{
    DOMString a = substr(0, pos);
    DOMString b = substr(pos, size());
    clear();
    append(a);
    append(str);
    append(b);
    return *this;
}

DOMString &DOMString::insert(unsigned long pos, const char *str)
{
    DOMString a = substr(0, pos);
    DOMString b = substr(pos, size());
    clear();
    append(a);
    append(str);
    append(b);
    return *this;
}

DOMString &DOMString::insert(unsigned long pos, const std::string &str)
{
    DOMString a = substr(0, pos);
    DOMString b = substr(pos, size());
    clear();
    append(a);
    append(str);
    append(b);
    return *this;
}


DOMString &DOMString::prepend(const DOMString &str)
{
    DOMString tmp = *this;
    clear();
    append(str);
    append(tmp);
    return *this;
}

DOMString &DOMString::prepend(const char *str)
{
    DOMString tmp = *this;
    clear();
    append(str);
    append(tmp);
    return *this;
}

DOMString &DOMString::prepend(const std::string &str)
{
    DOMString tmp = *this;
    clear();
    append(str);
    append(tmp);
    return *this;
}

DOMString &DOMString::replace(unsigned long pos, unsigned long count,
                              const DOMString &str)
{
    DOMString a = substr(0, pos);
    DOMString b = substr(pos+count, size());
    clear();
    append(a);
    append(str);
    append(b);
    return *this;
}

DOMString &DOMString::replace(unsigned long pos, unsigned long count,
                              const char *str)
{
    DOMString a = substr(0, pos);
    DOMString b = substr(pos+count, size());
    clear();
    append(a);
    append(str);
    append(b);
    return *this;
}

DOMString &DOMString::replace(unsigned long pos, unsigned long count,
                              const std::string &str)
{
    DOMString a = substr(0, pos);
    DOMString b = substr(pos+count, size());
    clear();
    append(a);
    append(str);
    append(b);
    return *this;
}


DOMString &DOMString::push_back(XMLCh ch)
{
    chars.push_back(ch);
    return *this;
}



void DOMString::clear()
{
    chars.clear();
    if (cstring)
        {
        delete[] cstring;
        cstring = NULL;
        }
}

//#########################################################################
//# Q U E R Y
//#########################################################################

XMLCh DOMString::charAt(unsigned long index) const
{
    return chars[index];
}

XMLCh DOMString::operator[](unsigned long index) const
{
    return chars[index];
}

DOMString DOMString::substr(unsigned long start, unsigned long end) const
{
    DOMString ret;
    for (unsigned long i = start; i<end ; i++)
        ret.push_back(chars[i]);
    return ret;
}

const char *DOMString::c_str()
{
    if (cstring)
        delete[] cstring;

    int length = chars.size();

    cstring = new char[length+1];

    int i=0;
    for ( ; i<length ; i++ )
        cstring[i] = (char) chars[i];

    cstring[i] = '\0';

    return cstring;
}


unsigned long DOMString::size() const
{
    return chars.size();
}



//#########################################################################
//# C O M P A R I S O N
//#########################################################################

int DOMString::compare(const DOMString &str) const
{
    int asize = chars.size();
    int bsize = str.size();

    int diff = 0;
    int index = 0;
    while (index < asize && index < bsize)
        {
        int a = (int) chars[index];
        int b = (int) str[index];
        diff = a - b;
        if (diff)
            return diff;
        index++;
        }

    //equal for their common length. lets see which is longer
    diff = asize - bsize;

    return diff;
}


int DOMString::compare(const char *str) const
{
    if (!str)
        return 1;

    int asize = chars.size();

    int diff = 0;
    int index = 0;
    while (index < asize)
        {
        int a = (int) chars[index];
        int b = (int) str[index];
        if (!b)
            return a;

        diff = a - b;
        if (diff)
            return diff;
        index++;
        }

    diff = str[index]; //char or terminating 0

    return diff;

}


int DOMString::compare(const std::string &str) const
{
    int asize = chars.size();
    int bsize = str.size();

    int diff = 0;
    int index = 0;
    while (index < asize && index < bsize)
        {
        int a = (int) chars[index];
        int b = (int) str[index];
        diff = a - b;
        if (diff)
            return diff;
        index++;
        }

    diff = asize - bsize;

    return diff;



}



//#########################################################################
//# O P E R A T O R S
//#########################################################################
DOMString &operator +(DOMString &a, const char *b)
    { return a.append(b); }

DOMString &operator +(const char *b, DOMString &a)
    { return a.prepend(b); }



}  //namespace dom
}  //namespace w3c
}  //namespace org

/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/



