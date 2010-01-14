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
 * Copyright (C) 2006 Bob Jamison
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



#include "httpclient.h"

namespace org
{
namespace w3c
{
namespace dom
{
namespace io
{




/**
 *
 */
HttpClient::HttpClient()
{
}


/**
 *
 */
HttpClient::~HttpClient()
{
}


/**
 *
 */
bool HttpClient::openGet(const URI &uri)
{

    socket.disconnect();

    if (uri.getScheme() == URI::SCHEME_HTTP)
        socket.enableSSL(false);
    else if (uri.getScheme() == URI::SCHEME_HTTPS)
        socket.enableSSL(true);
    else
        {
        //printf("Bad proto scheme:%d\n", uri.getScheme());
        return false;
        }

    DOMString host = uri.getHost();
    int port       = uri.getPort();
    DOMString path = uri.getPath();
    if (path.size() == 0)
        path = "/";

    //printf("host:%s port:%d, path:%s\n", host.c_str(), port, path.c_str());

    if (!socket.connect(host, port))
        {
        return false;
        }

    DOMString msg = "GET ";
    msg.append(path);
    msg.append(" HTTP/1.0\r\n\r\n");
    //printf("msg:'%s'\n", msg.c_str());

    //# Make the request
    if (!socket.write(msg))
        {
        return false;
        }

    //# Read the HTTP headers
    while (true)
        {
        if (!socket.readLine(msg))
            return false;
        //printf("header:'%s'\n", msg.c_str());
        if (msg.size() < 1)
            break;
        }

    return true;
}


/**
 *
 */
int HttpClient::read()
{
    int ret = socket.read();
    return ret;
}

/**
 *
 */
bool HttpClient::write(int ch)
{
    if (!socket.write(ch))
        return false;
    return true;
}

/**
 *
 */
bool HttpClient::write(const DOMString &msg)
{
    if (!socket.write(msg))
        return false;
    return true;
}

/**
 *
 */
bool HttpClient::close()
{
    socket.disconnect();
    return true;
}



}  //namespace io
}  //namespace dom
}  //namespace w3c
}  //namespace org


//#########################################################################
//# E N D    O F    F I L E
//#########################################################################

