#ifndef __DOM_SOCKET_H__
#define __DOM_SOCKET_H__
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

#include "dom/dom.h"

#ifdef HAVE_SSL
#include <openssl/ssl.h>
#endif

namespace org
{
namespace w3c
{
namespace dom
{
namespace io
{

class TcpSocket
{
public:

    TcpSocket();

    TcpSocket(const DOMString &hostname, int port);

    TcpSocket(const TcpSocket &other);

    virtual ~TcpSocket();

    bool isConnected();

    void enableSSL(bool val);

    bool connect(const DOMString &hostname, int portno);

    bool startTls();

    bool connect();

    bool disconnect();

    bool setReceiveTimeout(unsigned long millis);

    long available();

    bool write(int ch);

    bool write(const DOMString &str);

    int read();

    bool readLine(DOMString &result);

private:

    void init();

    DOMString hostname;
    int  portno;
    int  sock;
    bool connected;

    bool sslEnabled;

    unsigned long receiveTimeout;

#ifdef HAVE_SSL
    SSL_CTX *sslContext;
    SSL *sslStream;
#endif

};




}  //namespace io
}  //namespace dom
}  //namespace w3c
}  //namespace org

#endif /* __DOM_SOCKET_H__ */
//#########################################################################
//# E N D    O F    F I L E
//#########################################################################

