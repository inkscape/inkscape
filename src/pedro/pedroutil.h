#ifndef __PEDROUTIL_H__ 
#define __PEDROUTIL_H__ 
/*
 * Support classes for the Pedro mini-XMPP client.
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2005-2007 Bob Jamison
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
#include <stdarg.h>
#include <vector>

#include <string>

#include "pedrodom.h"


#ifdef HAVE_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif



namespace Pedro
{




//########################################################################
//########################################################################
//# B A S E    6 4
//########################################################################
//########################################################################


//#################
//# ENCODER
//#################


/**
 * This class is for Base-64 encoding
 */
class Base64Encoder
{

public:

    Base64Encoder()
        {
        reset();
        }

    virtual ~Base64Encoder()
        {}

    virtual void reset()
        {
        outBuf   = 0L;
        bitCount = 0;
        buf = "";
        }

    virtual void append(int ch);

    virtual void append(char *str);

    virtual void append(unsigned char *str, int len);

    virtual void append(const DOMString &str);

    virtual DOMString finish();

    static DOMString encode(const DOMString &str);


private:


    unsigned long outBuf;

    int bitCount;

    DOMString buf;

};




//#################
//# DECODER
//#################

class Base64Decoder
{
public:
    Base64Decoder()
        {
        reset();
        }

    virtual ~Base64Decoder()
        {}

    virtual void reset()
        {
        inCount = 0;
        buf.clear();
        }


    virtual void append(int ch);

    virtual void append(char *str);

    virtual void append(const DOMString &str);

    std::vector<unsigned char> finish();

    static std::vector<unsigned char> decode(const DOMString &str);

    static DOMString decodeToString(const DOMString &str);

private:

    int inBytes[4];
    int inCount;
    std::vector<unsigned char> buf;
};




//########################################################################
//########################################################################
//### S H A    1      H A S H I N G
//########################################################################
//########################################################################

/**
 *  This class performs a slow SHA1 hash on a stream of input data.
 */
class Sha1
{
public:

    /**
     * Constructor
     */
    Sha1()
        { init(); }

    /**
     *
     */
    virtual ~Sha1()
        { init(); }


    /**
     * Static convenience method.  This would be the most commonly used
     * version;
     * @parm digest points to a bufer of 20 unsigned chars
     */
    static void hash(unsigned char *dataIn, int len, unsigned char *digest);

    /**
     * Static convenience method.  This will fill a string with the hex
     * coded string.
     */
    static DOMString hashHex(unsigned char *dataIn, int len);

    /**
     * Static convenience method.
     */
    static DOMString hashHex(const DOMString &str);

    /**
     *  Initialize the context (also zeroizes contents)
     */
    virtual void init();

    /**
     *  Append a single character
     */
    virtual void append(unsigned char ch);

    /**
     *  Append a data buffer
     */
    virtual void append(unsigned char *dataIn, int len);

    /**
     *  Append a String
     */
    virtual void append(const DOMString &str);

    /**
     *
     * @parm digest points to a bufer of 20 unsigned chars
     */
    virtual void finish(unsigned char *digest);


private:

    void transform();

    unsigned long hashBuf[5];
    unsigned long inBuf[80];
    unsigned long nrBytesHi;
    unsigned long nrBytesLo;
    int           longNr;
    int           byteNr;
    unsigned long inb[4];

};





//########################################################################
//########################################################################
//### M D 5      H A S H I N G
//########################################################################
//########################################################################


/**
 * This is a utility version of a simple MD5 hash algorithm.  This is
 * neither efficient nor fast.  It is intended to be a small simple utility
 * for hashing small amounts of data in a non-time-critical place.
 *
 * Note that this is a rewrite whose purpose is to remove any
 * machine dependencies.
 */
class Md5
{
public:

    /**
     * Constructor
     */
    Md5()
        { init(); }

    /**
     * Destructor
     */
    virtual ~Md5()
        {}

    /**
     * Static convenience method.
     * @parm digest points to an buffer of 16 unsigned chars
     */
    static void hash(unsigned char *dataIn,
                     unsigned long len, unsigned char *digest);

    /**
     * Static convenience method.
     * Hash a byte array of a given length
     */
    static DOMString hashHex(unsigned char *dataIn, unsigned long len);

    /**
     * Static convenience method.
     * Hash a String
     */
    static DOMString hashHex(const DOMString &str);

    /**
     *  Initialize the context (also zeroizes contents)
     */
    virtual void init();

    /*
     * Update with one character
     */
    virtual void append(unsigned char ch);

    /**
     * Update with a byte buffer of a given length
     */
    virtual void append(unsigned char *dataIn, unsigned long len);

    /**
     * Update with a string
     */
    virtual void append(const DOMString &str);

    /**
     * Finalize and output the hash.
     * @parm digest points to an buffer of 16 unsigned chars
     */
    virtual void finish(unsigned char *digest);


    /**
     * Same as above , but hex to an output String
     */
    virtual DOMString finishHex();

private:

    void transform();

    unsigned long hashBuf[4];
    unsigned long inBuf[16];
    unsigned long nrBytesHi;
    unsigned long nrBytesLo;

    unsigned long inb[4];  // Buffer for input bytes as longs
    int           byteNr;  // which byte in long
    int           longNr;  // which long in 8 long segment

};





//########################################################################
//########################################################################
//### T H R E A D
//########################################################################
//########################################################################



/**
 * This is the interface for a delegate class which can
 * be run by a Thread.
 * Thread thread(runnable);
 * thread.start();
 */
class Runnable
{
public:

    Runnable()
        {}
    virtual ~Runnable()
        {}

    /**
     * The method of a delegate class which can
     * be run by a Thread.  Thread is completed when this
     * method is done.
     */
    virtual void run() = 0;

};



/**
 *  A simple wrapper of native threads in a portable class.
 *  It can be used either to execute its own run() method, or
 *  delegate to a Runnable class's run() method.
 */
class Thread
{
public:

    /**
     *  Create a thread which will execute its own run() method.
     */
    Thread()
        { runnable = NULL ; started = false; }

    /**
     * Create a thread which will run a Runnable class's run() method.
     */
    Thread(const Runnable &runner)
        { runnable = (Runnable *)&runner; started = false; }

    /**
     *  This does not kill a spawned thread.
     */
    virtual ~Thread()
        {}

    /**
     *  Static method to pause the current thread for a given
     *  number of milliseconds.
     */
    static void sleep(unsigned long millis);

    /**
     *  This method will be executed if the Thread was created with
     *  no delegated Runnable class.  The thread is completed when
     *  the method is done.
     */
    virtual void run()
        {}

    /**
     *  Starts the thread.
     */
    virtual void start();

    /**
     *  Calls either this class's run() method, or that of a Runnable.
     *  A user would normally not call this directly.
     */
    virtual void execute()
        {
        started = true;
        if (runnable)
            runnable->run();
        else
            run();
        }

private:

    Runnable *runnable;

    bool started;

};






//########################################################################
//########################################################################
//### S O C K E T
//########################################################################
//########################################################################



/**
 *  A socket wrapper that provides cross-platform capability, plus SSL
 */
class TcpSocket
{
public:

    TcpSocket();

    TcpSocket(const std::string &hostname, int port);

    TcpSocket(const char *hostname, int port);

    TcpSocket(const TcpSocket &other);

    virtual ~TcpSocket();
    
    void error(const char *fmt, ...);
    
    DOMString &getLastError();

    bool isConnected();

    void enableSSL(bool val);

    bool getEnableSSL();

    bool getHaveSSL();

    bool connect(const std::string &hostname, int portno);

    bool connect(const char *hostname, int portno);

    bool startTls();

    bool connect();

    bool disconnect();

    bool setReceiveTimeout(unsigned long millis);

    long available();

    bool write(int ch);

    bool write(char *str);

    bool write(const std::string &str);

    int read();

    std::string readLine();

private:
    void init();

    DOMString lastError;

    std::string hostname;
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






} //namespace Pedro

#endif /* __PEDROUTIL_H__ */

//########################################################################
//# E N D    O F     F I L E
//########################################################################

