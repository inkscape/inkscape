#ifndef __XMLWRITER_H__
#define __XMLWRITER_H__

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
 * Copyright (C) 2005-2008 Bob Jamison
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


#include "dom.h"
#include <stdio.h>
#include <glib.h>


namespace org
{
namespace w3c
{
namespace dom
{



class XmlWriter
{
public:

    XmlWriter();

    virtual ~XmlWriter();


    void write(const NodePtr node);

    void writeFile(FILE *f, const NodePtr node);


protected:

    int indent;

    void spaces();

    void po(const char *str, ...) G_GNUC_PRINTF(2,3);

    void pos(const DOMString &str);

    DOMString buf;


};


}  //namespace dom
}  //namespace w3c
}  //namespace org







#endif  /* __XMLWRITER_H__ */





