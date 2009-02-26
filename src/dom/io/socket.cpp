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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>   // needed on Solaris 8
#endif

#include <cstdio>
#include "socket.h"
#include "dom/util/thread.h"

#ifdef __WIN32__
#include <windows.h>
#else /* unix */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/ioctl.h>

#endif

#ifdef HAVE_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>

RELAYTOOL_SSL
#endif


namespace org
{
namespace w3c
{
namespace dom
{
namespace io
{

static void mybzero(void *s, size_t n)
{
    unsigned char *p = (unsigned char *)s;
    while (n > 0)
        {
        *p++ = (unsigned char)0;
        n--;
        }
}

static void mybcopy(void *src, void *dest, size_t n)
{
    unsigned char *p = (unsigned char *)dest;
    unsigned char *q = (unsigned char *)src;
    while (n > 0)
        {
        *p++ = *q++;
        n--;
        }
}



//#########################################################################
//# T C P    C O N N E C T I O N
//#########################################################################

TcpSocket::TcpSocket()
{
    init();
}


TcpSocket::TcpSocket(const DOMString &hostnameArg, int port)
{
    init();
    hostname  = hostnameArg;
    portno    = port;
}


#ifdef HAVE_SSL

static void cryptoLockCallback(int mode, int type, const char *file, int line)
{
    //printf("########### LOCK\n");
    static int modes[CRYPTO_NUM_LOCKS]; /* = {0, 0, ... } */
    const char *errstr = NULL;

    int rw = mode & (CRYPTO_READ|CRYPTO_WRITE);
    if (!((rw == CRYPTO_READ) || (rw == CRYPTO_WRITE)))
        {
        errstr = "invalid mode";
        goto err;
        }

    if (type < 0 || type >= CRYPTO_NUM_LOCKS)
        {
        errstr = "type out of bounds";
        goto err;
        }

    if (mode & CRYPTO_LOCK)
        {
        if (modes[type])
            {
            errstr = "already locked";
            /* must not happen in a single-threaded program
             * (would deadlock)
             */
            goto err;
            }

        modes[type] = rw;
        }
    else if (mode & CRYPTO_UNLOCK)
        {
        if (!modes[type])
            {
             errstr = "not locked";
             goto err;
             }

        if (modes[type] != rw)
            {
            errstr = (rw == CRYPTO_READ) ?
                  "CRYPTO_r_unlock on write lock" :
                  "CRYPTO_w_unlock on read lock";
            }

        modes[type] = 0;
        }
    else
        {
        errstr = "invalid mode";
        goto err;
        }

    err:
    if (errstr)
        {
        /* we cannot use bio_err here */
        fprintf(stderr, "openssl (lock_dbg_cb): %s (mode=%d, type=%d) at %s:%d\n",
                errstr, mode, type, file, line);
        }
}

static unsigned long cryptoIdCallback()
{
#ifdef __WIN32__
    unsigned long ret = (unsigned long) GetCurrentThreadId();
#else
    unsigned long ret = (unsigned long) pthread_self();
#endif
    return ret;
}

#endif


TcpSocket::TcpSocket(const TcpSocket &other)
{
    init();
    sock      = other.sock;
    hostname  = other.hostname;
    portno    = other.portno;
}

static bool tcp_socket_inited = false;

void TcpSocket::init()
{
    if (!tcp_socket_inited)
        {
#ifdef __WIN32__
        WORD wVersionRequested = MAKEWORD( 2, 2 );
        WSADATA wsaData;
        WSAStartup( wVersionRequested, &wsaData );
#endif
#ifdef HAVE_SSL
        if (libssl_is_present)
            {
            sslStream  = NULL;
            sslContext = NULL;
            CRYPTO_set_locking_callback(cryptoLockCallback);
            CRYPTO_set_id_callback(cryptoIdCallback);
            SSL_library_init();
            SSL_load_error_strings();
            }
#endif
        tcp_socket_inited = true;
        }
    sock           = -1;
    connected      = false;
    hostname       = "";
    portno         = -1;
    sslEnabled     = false;
    receiveTimeout = 0;
}

TcpSocket::~TcpSocket()
{
    disconnect();
}

bool TcpSocket::isConnected()
{
    if (!connected || sock < 0)
        return false;
    return true;
}

void TcpSocket::enableSSL(bool val)
{
    sslEnabled = val;
}


bool TcpSocket::connect(const DOMString &hostnameArg, int portnoArg)
{
    hostname = hostnameArg;
    portno   = portnoArg;
    return connect();
}



#ifdef HAVE_SSL
/*
static int password_cb(char *buf, int bufLen, int rwflag, void *userdata)
{
    char *password = "password";
    if (bufLen < (int)(strlen(password)+1))
        return 0;

    strcpy(buf,password);
    int ret = strlen(password);
    return ret;
}

static void infoCallback(const SSL *ssl, int where, int ret)
{
    switch (where)
        {
        case SSL_CB_ALERT:
            {
            printf("## %d SSL ALERT: %s\n",  where, SSL_alert_desc_string_long(ret));
            break;
            }
        default:
            {
            printf("## %d SSL: %s\n",  where, SSL_state_string_long(ssl));
            break;
            }
        }
}
*/
#endif


bool TcpSocket::startTls()
{
#ifdef HAVE_SSL
    if (libssl_is_present)
    {
    sslStream  = NULL;
    sslContext = NULL;

    //SSL_METHOD *meth = SSLv23_method();
    //SSL_METHOD *meth = SSLv3_client_method();
    SSL_METHOD *meth = TLSv1_client_method();
    sslContext = SSL_CTX_new(meth);
    //SSL_CTX_set_info_callback(sslContext, infoCallback);

#if 0
    char *keyFile  = "client.pem";
    char *caList   = "root.pem";
    /* Load our keys and certificates*/
    if (!(SSL_CTX_use_certificate_chain_file(sslContext, keyFile)))
        {
        fprintf(stderr, "Can't read certificate file\n");
        disconnect();
        return false;
        }

    SSL_CTX_set_default_passwd_cb(sslContext, password_cb);

    if (!(SSL_CTX_use_PrivateKey_file(sslContext, keyFile, SSL_FILETYPE_PEM)))
        {
        fprintf(stderr, "Can't read key file\n");
        disconnect();
        return false;
        }

    /* Load the CAs we trust*/
    if (!(SSL_CTX_load_verify_locations(sslContext, caList, 0)))
        {
        fprintf(stderr, "Can't read CA list\n");
        disconnect();
        return false;
        }
#endif

    /* Connect the SSL socket */
    sslStream  = SSL_new(sslContext);
    SSL_set_fd(sslStream, sock);

    if (SSL_connect(sslStream)<=0)
        {
        fprintf(stderr, "SSL connect error\n");
        disconnect();
        return false;
        }

    sslEnabled = true;
    }
#endif /*HAVE_SSL*/
    return true;
}


bool TcpSocket::connect()
{
    if (hostname.size()<1)
        {
        printf("open: null hostname\n");
        return false;
        }

    if (portno<1)
        {
        printf("open: bad port number\n");
        return false;
        }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        {
        printf("open: error creating socket\n");
        return false;
        }

    char *c_hostname = (char *)hostname.c_str();
    struct hostent *server = gethostbyname(c_hostname);
    if (!server)
        {
        printf("open: could not locate host '%s'\n", c_hostname);
        return false;
        }

    struct sockaddr_in serv_addr;
    mybzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    mybcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    int ret = ::connect(sock, (const sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret < 0)
        {
        printf("open: could not connect to host '%s'\n", c_hostname);
        return false;
        }

     if (sslEnabled)
        {
        if (!startTls())
            return false;
        }
    connected = true;
    return true;
}

bool TcpSocket::disconnect()
{
    bool ret  = true;
    connected = false;
#ifdef HAVE_SSL
    if (libssl_is_present)
    {
    if (sslEnabled)
        {
        if (sslStream)
            {
            int r = SSL_shutdown(sslStream);
            switch(r)
                {
                case 1:
                    break; /* Success */
                case 0:
                case -1:
                default:
                    //printf("Shutdown failed");
                    ret = false;
                }
            SSL_free(sslStream);
            }
        if (sslContext)
            SSL_CTX_free(sslContext);
        }
    sslStream  = NULL;
    sslContext = NULL;
    }
#endif /*HAVE_SSL*/

#ifdef __WIN32__
    closesocket(sock);
#else
    ::close(sock);
#endif
    sock = -1;
    sslEnabled = false;

    return ret;
}



bool TcpSocket::setReceiveTimeout(unsigned long millis)
{
    receiveTimeout = millis;
    return true;
}

/**
 * For normal sockets, return the number of bytes waiting to be received.
 * For SSL, just return >0 when something is ready to be read.
 */
long TcpSocket::available()
{
    if (!isConnected())
        return -1;

    long count = 0;
#ifdef __WIN32__
    if (ioctlsocket(sock, FIONREAD, (unsigned long *)&count) != 0)
        return -1;
#else
    if (ioctl(sock, FIONREAD, &count) != 0)
        return -1;
#endif
    if (count<=0 && sslEnabled)
        {
#ifdef HAVE_SSL
        if (libssl_is_present)
            {
            return SSL_pending(sslStream);
            }
#endif
        }
    return count;
}



bool TcpSocket::write(int ch)
{
    if (!isConnected())
        {
        printf("write: socket closed\n");
        return false;
        }
    unsigned char c = (unsigned char)ch;

    if (sslEnabled)
        {
#ifdef HAVE_SSL
        if (libssl_is_present)
            {
            int r = SSL_write(sslStream, &c, 1);
            if (r<=0)
                {
                switch(SSL_get_error(sslStream, r))
                    {
                    default:
                    printf("SSL write problem");
                    return -1;
                    }
                }
            }
#endif
        }
    else
        {
        if (send(sock, (const char *)&c, 1, 0) < 0)
        //if (send(sock, &c, 1, 0) < 0)
            {
            printf("write: could not send data\n");
            return false;
            }
        }
    return true;
}

bool TcpSocket::write(const DOMString &strArg)
{
    DOMString str = strArg;

    if (!isConnected())
        {
        printf("write(str): socket closed\n");
        return false;
        }
    int len = str.size();

    if (sslEnabled)
        {
#ifdef HAVE_SSL
        if (libssl_is_present)
            {
           int r = SSL_write(sslStream, (unsigned char *)str.c_str(), len);
            if (r<=0)
                {
                switch(SSL_get_error(sslStream, r))
                    {
                    default:
                        printf("SSL write problem");
                        return -1;
                    }
                }
            }
#endif
        }
    else
        {
        if (send(sock, str.c_str(), len, 0) < 0)
        //if (send(sock, &c, 1, 0) < 0)
            {
            printf("write: could not send data\n");
            return false;
            }
        }
    return true;
}

int TcpSocket::read()
{
    if (!isConnected())
        return -1;

    //We'll use this loop for timeouts, so that SSL and plain sockets
    //will behave the same way
    if (receiveTimeout > 0)
        {
        unsigned long tim = 0;
        while (true)
            {
            int avail = available();
            if (avail > 0)
                break;
            if (tim >= receiveTimeout)
                return -2;
            org::w3c::dom::util::Thread::sleep(20);
            tim += 20;
            }
        }

    //check again
    if (!isConnected())
        return -1;

    unsigned char ch;
    if (sslEnabled)
        {
#ifdef HAVE_SSL
        if (libssl_is_present)
            {
            if (!sslStream)
                return -1;
            int r = SSL_read(sslStream, &ch, 1);
            unsigned long err = SSL_get_error(sslStream, r);
            switch (err)
                {
                case SSL_ERROR_NONE:
                     break;
                case SSL_ERROR_ZERO_RETURN:
                    return -1;
                case SSL_ERROR_SYSCALL:
                    printf("SSL read problem(syscall) %s\n",
                         ERR_error_string(ERR_get_error(), NULL));
                    return -1;
                default:
                    printf("SSL read problem %s\n",
                         ERR_error_string(ERR_get_error(), NULL));
                    return -1;
                }
            }
#endif
        }
    else
        {
        int ret = recv(sock, (char *)&ch, 1, 0);
        if (ret <= 0)
            {
            if (ret<0)
                printf("read: could not receive data\n");
            disconnect();
            return -1;
            }
        }
    return (int)ch;
}

bool TcpSocket::readLine(DOMString &result)
{
    result = "";

    while (isConnected())
        {
        int ch = read();
        if (ch<0)
            return true;
        else if (ch=='\r') //we want canonical Net '\r\n' , so skip this
            {}
        else if (ch=='\n')
            return true;
        else
            result.push_back((char)ch);
        }

    return true;
}

}  //namespace io
}  //namespace dom
}  //namespace w3c
}  //namespace org


//#########################################################################
//# E N D    O F    F I L E
//#########################################################################

