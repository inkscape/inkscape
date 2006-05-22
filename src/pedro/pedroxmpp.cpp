/*
 * Implementation the Pedro mini-XMPP client
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
#include <stdarg.h>

#include <sys/stat.h>

#include "pedroxmpp.h"
#include "pedrodom.h"

#include <map>

#ifdef __WIN32__

#include <windows.h>

#else /* UNIX */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <pthread.h>

#endif

#ifdef HAVE_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif


namespace Pedro
{

//########################################################################
//########################################################################
//### U T I L I T Y
//########################################################################
//########################################################################


//########################################################################
//# B A S E    6 4
//########################################################################

//#################
//# ENCODER
//#################


static char *base64encode =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

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



/**
 * Writes the specified byte to the output buffer
 */
void Base64Encoder::append(int ch)
{
    outBuf   <<=  8;
    outBuf   |=  (ch & 0xff);
    bitCount +=  8;
    if (bitCount >= 24)
        {
        int indx  = (int)((outBuf & 0x00fc0000L) >> 18);
        int obyte = (int)base64encode[indx & 63];
        buf.push_back(obyte);

        indx      = (int)((outBuf & 0x0003f000L) >> 12);
        obyte     = (int)base64encode[indx & 63];
        buf.push_back(obyte);

        indx      = (int)((outBuf & 0x00000fc0L) >>  6);
        obyte     = (int)base64encode[indx & 63];
        buf.push_back(obyte);

        indx      = (int)((outBuf & 0x0000003fL)      );
        obyte     = (int)base64encode[indx & 63];
        buf.push_back(obyte);

        bitCount = 0;
        outBuf   = 0L;
        }
}

/**
 * Writes the specified string to the output buffer
 */
void Base64Encoder::append(char *str)
{
    while (*str)
        append((int)*str++);
}

/**
 * Writes the specified string to the output buffer
 */
void Base64Encoder::append(unsigned char *str, int len)
{
    while (len>0)
        {
        append((int)*str++);
        len--;
        }
}

/**
 * Writes the specified string to the output buffer
 */
void Base64Encoder::append(const DOMString &str)
{
    append((char *)str.c_str());
}

/**
 * Closes this output stream and releases any system resources
 * associated with this stream.
 */
DOMString Base64Encoder::finish()
{
    //get any last bytes (1 or 2) out of the buffer
    if (bitCount == 16)
        {
        outBuf <<= 2;  //pad to make 18 bits

        int indx  = (int)((outBuf & 0x0003f000L) >> 12);
        int obyte = (int)base64encode[indx & 63];
        buf.push_back(obyte);

        indx      = (int)((outBuf & 0x00000fc0L) >>  6);
        obyte     = (int)base64encode[indx & 63];
        buf.push_back(obyte);

        indx      = (int)((outBuf & 0x0000003fL)      );
        obyte     = (int)base64encode[indx & 63];
        buf.push_back(obyte);

        buf.push_back('=');
        }
    else if (bitCount == 8)
        {
        outBuf <<= 4; //pad to make 12 bits

        int indx  = (int)((outBuf & 0x00000fc0L) >>  6);
        int obyte = (int)base64encode[indx & 63];
        buf.push_back(obyte);

        indx      = (int)((outBuf & 0x0000003fL)      );
        obyte     = (int)base64encode[indx & 63];
        buf.push_back(obyte);

        buf.push_back('=');
        buf.push_back('=');
        }

    DOMString ret = buf;
    reset();
    return ret;
}


DOMString Base64Encoder::encode(const DOMString &str)
{
    Base64Encoder encoder;
    encoder.append(str);
    DOMString ret = encoder.finish();
    return ret;
}



//#################
//# DECODER
//#################

static int base64decode[] =
{
/*00*/    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
/*08*/    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
/*10*/    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
/*18*/    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
/*20*/    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
/*28*/    -1,   -1,   -1,   62,   -1,   -1,   -1,   63,
/*30*/    52,   53,   54,   55,   56,   57,   58,   59,
/*38*/    60,   61,   -1,   -1,   -1,   -1,   -1,   -1,
/*40*/    -1,    0,    1,    2,    3,    4,    5,    6,
/*48*/     7,    8,    9,   10,   11,   12,   13,   14,
/*50*/    15,   16,   17,   18,   19,   20,   21,   22,
/*58*/    23,   24,   25,   -1,   -1,   -1,   -1,   -1,
/*60*/    -1,   26,   27,   28,   29,   30,   31,   32,
/*68*/    33,   34,   35,   36,   37,   38,   39,   40,
/*70*/    41,   42,   43,   44,   45,   46,   47,   48,
/*78*/    49,   50,   51,   -1,   -1,   -1,   -1,   -1
};

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

/**
 * Appends one char to the decoder
 */
void Base64Decoder::append(int ch)
{
    if (isspace(ch))
        return;
    else if (ch == '=') //padding
        {
        inBytes[inCount++] = 0;
        }
    else
        {
        int byteVal = base64decode[ch & 0x7f];
        //printf("char:%c %d\n", ch, byteVal);
        if (byteVal < 0)
            {
            //Bad lookup value
            }
        inBytes[inCount++] = byteVal;
        }

    if (inCount >=4 )
        {
        unsigned char b0 = ((inBytes[0]<<2) & 0xfc) | ((inBytes[1]>>4) & 0x03);
        unsigned char b1 = ((inBytes[1]<<4) & 0xf0) | ((inBytes[2]>>2) & 0x0f);
        unsigned char b2 = ((inBytes[2]<<6) & 0xc0) | ((inBytes[3]   ) & 0x3f);
        buf.push_back(b0);
        buf.push_back(b1);
        buf.push_back(b2);
        inCount = 0;
        }

}

void Base64Decoder::append(char *str)
{
    while (*str)
        append((int)*str++);
}

void Base64Decoder::append(const DOMString &str)
{
    append((char *)str.c_str());
}

std::vector<unsigned char> Base64Decoder::finish()
{
    std::vector<unsigned char> ret = buf;
    reset();
    return ret;
}

std::vector<unsigned char> Base64Decoder::decode(const DOMString &str)
{
    Base64Decoder decoder;
    decoder.append(str);
    std::vector<unsigned char> ret = decoder.finish();
    return ret;
}

DOMString Base64Decoder::decodeToString(const DOMString &str)
{
    Base64Decoder decoder;
    decoder.append(str);
    std::vector<unsigned char> ret = decoder.finish();
    DOMString buf;
    for (unsigned int i=0 ; i<ret.size() ; i++)
        buf.push_back(ret[i]);
    return buf;
}



//########################################################################
//# S H A   1
//########################################################################

class Sha1
{
public:

    /**
     *
     */
    Sha1()
        { init(); }

    /**
     *
     */
    virtual ~Sha1()
        {}


    /**
     * Static convenience method.  This would be the most commonly used
     * version;
     * @parm digest points to a bufer of 20 unsigned chars
     */
    static void hash(unsigned char *dataIn, int len, unsigned char *digest);

    /**
     * Static convenience method.  This will fill a string with the hex
     * codex string.
     */
    static DOMString hashHex(unsigned char *dataIn, int len);

    /**
     *  Initialize the context (also zeroizes contents)
     */
    virtual void init();

    /**
     *
     */
    virtual void append(unsigned char *dataIn, int len);

    /**
     *
     * @parm digest points to a bufer of 20 unsigned chars
     */
    virtual void finish(unsigned char *digest);


private:

    void hashblock();

    unsigned long H[5];
    unsigned long W[80];
    unsigned long sizeHi,sizeLo;
    int lenW;

};



void Sha1::hash(unsigned char *dataIn, int len, unsigned char *digest)
{
    Sha1 sha1;
    sha1.append(dataIn, len);
    sha1.finish(digest);
}

static char *sha1hex = "0123456789abcdef";

DOMString Sha1::hashHex(unsigned char *dataIn, int len)
{
    unsigned char hashout[20];
    hash(dataIn, len, hashout);
    DOMString ret;
    for (int i=0 ; i<20 ; i++)
        {
        unsigned char ch = hashout[i];
        ret.push_back(sha1hex[ (ch>>4) & 15 ]);
        ret.push_back(sha1hex[ ch      & 15 ]);
        }
    return ret;
}


void Sha1::init()
{

    lenW   = 0;
    sizeHi = 0;
    sizeLo = 0;

    // Initialize H with the magic constants (see FIPS180 for constants)
    H[0] = 0x67452301L;
    H[1] = 0xefcdab89L;
    H[2] = 0x98badcfeL;
    H[3] = 0x10325476L;
    H[4] = 0xc3d2e1f0L;

    for (int i = 0; i < 80; i++)
        W[i] = 0;
}


void Sha1::append(unsigned char *dataIn, int len)
{
    // Read the data into W and process blocks as they get full
    for (int i = 0; i < len; i++)
        {
        W[lenW / 4] <<= 8;
        W[lenW / 4] |= (unsigned long)dataIn[i];
        if ((++lenW) % 64 == 0)
            {
            hashblock();
            lenW = 0;
            }
        sizeLo += 8;
        sizeHi += (sizeLo < 8);
        }
}


void Sha1::finish(unsigned char hashout[20])
{
    unsigned char pad0x80 = 0x80;
    unsigned char pad0x00 = 0x00;
    unsigned char padlen[8];

    // Pad with a binary 1 (e.g. 0x80), then zeroes, then length
    padlen[0] = (unsigned char)((sizeHi >> 24) & 255);
    padlen[1] = (unsigned char)((sizeHi >> 16) & 255);
    padlen[2] = (unsigned char)((sizeHi >>  8) & 255);
    padlen[3] = (unsigned char)((sizeHi >>  0) & 255);
    padlen[4] = (unsigned char)((sizeLo >> 24) & 255);
    padlen[5] = (unsigned char)((sizeLo >> 16) & 255);
    padlen[6] = (unsigned char)((sizeLo >>  8) & 255);
    padlen[7] = (unsigned char)((sizeLo >>  0) & 255);

    append(&pad0x80, 1);

    while (lenW != 56)
        append(&pad0x00, 1);
    append(padlen, 8);

    // Output hash
    for (int i = 0; i < 20; i++)
        {
        hashout[i] = (unsigned char)(H[i / 4] >> 24);
        H[i / 4] <<= 8;
        }

    // Re-initialize the context (also zeroizes contents)
    init();
}


#define SHA_ROTL(X,n) ((((X) << (n)) | ((X) >> (32-(n)))) & 0xffffffffL)

void Sha1::hashblock()
{

    for (int t = 16; t <= 79; t++)
        W[t] = SHA_ROTL(W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16], 1);

    unsigned long A = H[0];
    unsigned long B = H[1];
    unsigned long C = H[2];
    unsigned long D = H[3];
    unsigned long E = H[4];

    unsigned long TEMP;

    for (int t = 0; t <= 19; t++)
        {
        TEMP = (SHA_ROTL(A,5) + (((C^D)&B)^D) +
                E + W[t] + 0x5a827999L) & 0xffffffffL;
        E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP;
        }
    for (int t = 20; t <= 39; t++)
        {
        TEMP = (SHA_ROTL(A,5) + (B^C^D) +
                E + W[t] + 0x6ed9eba1L) & 0xffffffffL;
        E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP;
        }
    for (int t = 40; t <= 59; t++)
        {
        TEMP = (SHA_ROTL(A,5) + ((B&C)|(D&(B|C))) +
                E + W[t] + 0x8f1bbcdcL) & 0xffffffffL;
        E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP;
        }
    for (int t = 60; t <= 79; t++)
        {
        TEMP = (SHA_ROTL(A,5) + (B^C^D) +
                E + W[t] + 0xca62c1d6L) & 0xffffffffL;
        E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP;
        }

    H[0] += A;
    H[1] += B;
    H[2] += C;
    H[3] += D;
    H[4] += E;
}





//########################################################################
//# M D  5
//########################################################################

class Md5
{
public:

    /**
     *
     */
    Md5()
        { init(); }

    /**
     *
     */
    virtual ~Md5()
        {}

    /**
     * Static convenience method.
     * @parm digest points to an buffer of 16 unsigned chars
     */
    static void hash(unsigned char *dataIn,
                     unsigned long len, unsigned char *digest);

    static DOMString Md5::hashHex(unsigned char *dataIn, unsigned long len);

    /**
     *  Initialize the context (also zeroizes contents)
     */
    virtual void init();

    /**
     *
     */
    virtual void append(unsigned char *dataIn, unsigned long len);

    /**
     *
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

    void transform(unsigned long *buf, unsigned long *in);

    unsigned long buf[4];
    unsigned long bits[2];
    unsigned char in[64];

};




void Md5::hash(unsigned char *dataIn, unsigned long len, unsigned char *digest)
{
    Md5 md5;
    md5.append(dataIn, len);
    md5.finish(digest);
}

DOMString Md5::hashHex(unsigned char *dataIn, unsigned long len)
{
    Md5 md5;
    md5.append(dataIn, len);
    DOMString ret = md5.finishHex();
    return ret;
}



/*
 * Note: this code is harmless on little-endian machines.
 */
/*
static void byteReverse(unsigned char *buf, unsigned long longs)
{
    do
        {
        unsigned long t = (unsigned long)
            ((unsigned) buf[3] << 8 | buf[2]) << 16 |
            ((unsigned) buf[1] << 8 | buf[0]);
        *(unsigned long *) buf = t;
        buf += 4;
        } while (--longs);
}
*/

static void md5_memcpy(void *dest, void *src, int n)
{
    unsigned char *s1 = (unsigned char *)dest;
    unsigned char *s2 = (unsigned char *)src;
    while (n--)
        *s1++ = *s2++;
}

static void md5_memset(void *dest, char v, int n)
{
    unsigned char *s = (unsigned char *)dest;
    while (n--)
        *s++ = v;
}

/**
 * Initialize MD5 polynomials and storage
 */
void Md5::init()
{
    buf[0]  = 0x67452301;
    buf[1]  = 0xefcdab89;
    buf[2]  = 0x98badcfe;
    buf[3]  = 0x10325476;

    bits[0] = 0;
    bits[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
void Md5::append(unsigned char *source, unsigned long len)
{

    // Update bitcount
    unsigned long t = bits[0];
    if ((bits[0] = t + ((unsigned long) len << 3)) < t)
	    bits[1]++;// Carry from low to high
    bits[1] += len >> 29;

	//Bytes already in shsInfo->data
    t = (t >> 3) & 0x3f;


    // Handle any leading odd-sized chunks
    if (t)
        {
        unsigned char *p = (unsigned char *) in + t;
        t = 64 - t;
        if (len < t)
            {
            md5_memcpy(p, source, len);
            return;
            }
        md5_memcpy(p, source, t);
        //byteReverse(in, 16);
        transform(buf, (unsigned long *) in);
        source += t;
        len    -= t;
        }

    // Process data in 64-byte chunks
    while (len >= 64)
        {
        md5_memcpy(in, source, 64);
        //byteReverse(in, 16);
        transform(buf, (unsigned long *) in);
        source += 64;
        len    -= 64;
        }

    // Handle any remaining bytes of data.
    md5_memcpy(in, source, len);
}

/*
 * Update context to reflect the concatenation of another string
 */
void Md5::append(const DOMString &str)
{
    append((unsigned char *)str.c_str(), str.size());
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void Md5::finish(unsigned char *digest)
{
    // Compute number of bytes mod 64
    unsigned int count = (bits[0] >> 3) & 0x3F;

    // Set the first char of padding to 0x80.
    // This is safe since there is always at least one byte free
    unsigned char *p = in + count;
    *p++ = 0x80;

    // Bytes of padding needed to make 64 bytes
    count = 64 - 1 - count;

    // Pad out to 56 mod 64
    if (count < 8)
        {
	    // Two lots of padding:  Pad the first block to 64 bytes
	    md5_memset(p, 0, count);
	    //byteReverse(in, 16);
	    transform(buf, (unsigned long *) in);

	    // Now fill the next block with 56 bytes
	    md5_memset(in, 0, 56);
        }
    else
        {
        // Pad block to 56 bytes
        md5_memset(p, 0, count - 8);
        }
    //byteReverse(in, 14);

    // Append length in bits and transform
    ((unsigned long *) in)[14] = bits[0];
    ((unsigned long *) in)[15] = bits[1];

    transform(buf, (unsigned long *) in);
    //byteReverse((unsigned char *) buf, 4);
    md5_memcpy(digest, buf, 16);
    init();  // Security!  ;-)
}

static char *md5hex = "0123456789abcdef";

DOMString Md5::finishHex()
{
    unsigned char hashout[16];
    finish(hashout);
    DOMString ret;
    for (int i=0 ; i<16 ; i++)
        {
        unsigned char ch = hashout[i];
        ret.push_back(md5hex[ (ch>>4) & 15 ]);
        ret.push_back(md5hex[ ch      & 15 ]);
        }
    return ret;
}



//#  The four core functions - F1 is optimized somewhat

//  #define F1(x, y, z) (x & y | ~x & z)
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

// ## This is the central step in the MD5 algorithm.
#define MD5STEP(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 * @parm buf points to an array of 4 unsigned longs
 * @parm in points to an array of 16 unsigned longs
 */
void Md5::transform(unsigned long *buf, unsigned long *in)
{
    unsigned long a = buf[0];
    unsigned long b = buf[1];
    unsigned long c = buf[2];
    unsigned long d = buf[3];

    MD5STEP(F1, a, b, c, d, in[ 0] + 0xd76aa478,  7);
    MD5STEP(F1, d, a, b, c, in[ 1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, in[ 2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, in[ 3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, in[ 4] + 0xf57c0faf,  7);
    MD5STEP(F1, d, a, b, c, in[ 5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, in[ 6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, in[ 7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, in[ 8] + 0x698098d8,  7);
    MD5STEP(F1, d, a, b, c, in[ 9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122,  7);
    MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, in[ 1] + 0xf61e2562,  5);
    MD5STEP(F2, d, a, b, c, in[ 6] + 0xc040b340,  9);
    MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, in[ 0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, in[ 5] + 0xd62f105d,  5);
    MD5STEP(F2, d, a, b, c, in[10] + 0x02441453,  9);
    MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, in[ 4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, in[ 9] + 0x21e1cde6,  5);
    MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6,  9);
    MD5STEP(F2, c, d, a, b, in[ 3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, in[ 8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905,  5);
    MD5STEP(F2, d, a, b, c, in[ 2] + 0xfcefa3f8,  9);
    MD5STEP(F2, c, d, a, b, in[ 7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, in[ 5] + 0xfffa3942,  4);
    MD5STEP(F3, d, a, b, c, in[ 8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, in[ 1] + 0xa4beea44,  4);
    MD5STEP(F3, d, a, b, c, in[ 4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, in[ 7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6,  4);
    MD5STEP(F3, d, a, b, c, in[ 0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, in[ 3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, in[ 6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, in[ 9] + 0xd9d4d039,  4);
    MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, in[ 2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, in[ 0] + 0xf4292244,  6);
    MD5STEP(F4, d, a, b, c, in[ 7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, in[ 5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3,  6);
    MD5STEP(F4, d, a, b, c, in[ 3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, in[ 1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, in[ 8] + 0x6fa87e4f,  6);
    MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, in[ 6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, in[ 4] + 0xf7537e82,  6);
    MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, in[ 2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, in[ 9] + 0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}







//########################################################################
//########################################################################
//### T H R E A D
//########################################################################
//########################################################################


//########################################################################
//### T H R E A D
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





#ifdef __WIN32__


static DWORD WINAPI WinThreadFunction(LPVOID context)
{
    Thread *thread = (Thread *)context;
    thread->execute();
    return 0;
}


void Thread::start()
{
    DWORD dwThreadId;
    HANDLE hThread = CreateThread(NULL, 0, WinThreadFunction,
               (LPVOID)this,  0,  &dwThreadId);
    //Make sure the thread is started before 'this' is deallocated
    while (!started)
        sleep(10);
    CloseHandle(hThread);
}

void Thread::sleep(unsigned long millis)
{
    Sleep(millis);
}

#else /* UNIX */


void *PthreadThreadFunction(void *context)
{
    Thread *thread = (Thread *)context;
    thread->execute();
    return NULL;
}


void Thread::start()
{
    pthread_t thread;

    int ret = pthread_create(&thread, NULL,
            PthreadThreadFunction, (void *)this);
    if (ret != 0)
        printf("Thread::start: thread creation failed: %s\n", strerror(ret));

    //Make sure the thread is started before 'this' is deallocated
    while (!started)
        sleep(10);

}

void Thread::sleep(unsigned long millis)
{
    timespec requested;
    requested.tv_sec = millis / 1000;
    requested.tv_nsec = (millis % 1000 ) * 1000000L;
    nanosleep(&requested, NULL);
}

#endif


//########################################################################
//########################################################################
//### S O C K E T
//########################################################################
//########################################################################





class TcpSocket
{
public:

    TcpSocket();

    TcpSocket(const std::string &hostname, int port);

    TcpSocket(const char *hostname, int port);

    TcpSocket(const TcpSocket &other);

    virtual ~TcpSocket();

    bool isConnected();

    void enableSSL(bool val);

    bool getEnableSSL();

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



//#########################################################################
//# U T I L I T Y
//#########################################################################

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


TcpSocket::TcpSocket(const char *hostnameArg, int port)
{
    init();
    hostname  = hostnameArg;
    portno    = port;
}

TcpSocket::TcpSocket(const std::string &hostnameArg, int port)
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
        sslStream  = NULL;
        sslContext = NULL;
	    CRYPTO_set_locking_callback(cryptoLockCallback);
        CRYPTO_set_id_callback(cryptoIdCallback);
        SSL_library_init();
        SSL_load_error_strings();
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

bool TcpSocket::getEnableSSL()
{
    return sslEnabled;
}


bool TcpSocket::connect(const char *hostnameArg, int portnoArg)
{
    hostname = hostnameArg;
    portno   = portnoArg;
    return connect();
}

bool TcpSocket::connect(const std::string &hostnameArg, int portnoArg)
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

    int ret = SSL_connect(sslStream);
    if (ret == 0)
        {
        fprintf(stderr, "SSL connection not successful\n");
        disconnect();
        return false;
        }
    else if (ret < 0)
        {
        int err = SSL_get_error(sslStream, ret);
        fprintf(stderr, "SSL connect error %d\n", err);
        disconnect();
        return false;
        }

    sslEnabled = true;
#endif /*HAVE_SSL*/
    return true;
}


bool TcpSocket::connect()
{
    if (hostname.size()<1)
        {
        fprintf(stderr, "open: null hostname\n");
        return false;
        }

    if (portno<1)
        {
        fprintf(stderr, "open: bad port number\n");
        return false;
        }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        {
        fprintf(stderr, "open: error creating socket\n");
        return false;
        }

    char *c_hostname = (char *)hostname.c_str();
    struct hostent *server = gethostbyname(c_hostname);
    if (!server)
        {
        fprintf(stderr, "open: could not locate host '%s'\n", c_hostname);
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
        fprintf(stderr, "open: could not connect to host '%s'\n", c_hostname);
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
        return SSL_pending(sslStream);
#endif
        }
    return count;
}



bool TcpSocket::write(int ch)
{
    if (!isConnected())
        {
        fprintf(stderr, "write: socket closed\n");
        return false;
        }
    unsigned char c = (unsigned char)ch;

    if (sslEnabled)
        {
#ifdef HAVE_SSL
        int r = SSL_write(sslStream, &c, 1);
        if (r<=0)
            {
            switch(SSL_get_error(sslStream, r))
                {
                default:
                    fprintf(stderr, "SSL write problem");
                    return -1;
                }
            }
#endif
        }
    else
        {
        if (send(sock, (const char *)&c, 1, 0) < 0)
        //if (send(sock, &c, 1, 0) < 0)
            {
            fprintf(stderr, "write: could not send data\n");
            return false;
            }
        }
    return true;
}

bool TcpSocket::write(char *str)
{
   if (!isConnected())
        {
        fprintf(stderr, "write(str): socket closed\n");
        return false;
        }
    int len = strlen(str);

    if (sslEnabled)
        {
#ifdef HAVE_SSL
        int r = SSL_write(sslStream, (unsigned char *)str, len);
        if (r<=0)
            {
            switch(SSL_get_error(sslStream, r))
                {
                default:
                    fprintf(stderr, "SSL write problem");
                    return -1;
                }
            }
#endif
        }
    else
        {
        if (send(sock, str, len, 0) < 0)
        //if (send(sock, &c, 1, 0) < 0)
            {
            fprintf(stderr, "write: could not send data\n");
            return false;
            }
        }
    return true;
}

bool TcpSocket::write(const std::string &str)
{
    return write((char *)str.c_str());
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
            Thread::sleep(20);
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
                fprintf(stderr, "SSL read problem(syscall) %s\n",
                     ERR_error_string(ERR_get_error(), NULL));
                return -1;
            default:
                fprintf(stderr, "SSL read problem %s\n",
                     ERR_error_string(ERR_get_error(), NULL));
                return -1;
            }
#endif
        }
    else
        {
        if (recv(sock, (char *)&ch, 1, 0) <= 0)
            {
            fprintf(stderr, "read: could not receive data\n");
            disconnect();
            return -1;
            }
        }
    return (int)ch;
}

std::string TcpSocket::readLine()
{
    std::string ret;

    while (isConnected())
        {
        int ch = read();
        if (ch<0)
            return ret;
        if (ch=='\r' || ch=='\n')
            return ret;
        ret.push_back((char)ch);
        }

    return ret;
}


//########################################################################
//########################################################################
//### X M P P
//########################################################################
//########################################################################




//########################################################################
//# X M P P    E V E N T
//########################################################################


XmppEvent::XmppEvent(int type)
{
    eventType = type;
    presence  = false;
    dom       = NULL;
}

XmppEvent::XmppEvent(const XmppEvent &other)
{
    assign(other);
}

XmppEvent &XmppEvent::operator=(const XmppEvent &other)
{
    assign(other);
    return (*this);
}

XmppEvent::~XmppEvent()
{
    if (dom)
        delete dom;
}

void XmppEvent::assign(const XmppEvent &other)
{
    eventType = other.eventType;
    presence  = other.presence;
    status    = other.status;
    show      = other.show;
    to        = other.to;
    from      = other.from;
    group     = other.group;
    data      = other.data;
    fileName  = other.fileName;
    fileDesc  = other.fileDesc;
    fileSize  = other.fileSize;
    fileHash  = other.fileHash;
    setDOM(other.dom);
}

int XmppEvent::getType() const
{
    return eventType;
}

DOMString XmppEvent::getIqId() const
{
    return iqId;
}

void XmppEvent::setIqId(const DOMString &val)
{
    iqId = val;
}

DOMString XmppEvent::getStreamId() const
{
    return streamId;
}

void XmppEvent::setStreamId(const DOMString &val)
{
    streamId = val;
}

bool XmppEvent::getPresence() const
{
    return presence;
}

void XmppEvent::setPresence(bool val)
{
    presence = val;
}

DOMString XmppEvent::getShow() const
{
    return show;
}

void XmppEvent::setShow(const DOMString &val)
{
    show = val;
}

DOMString XmppEvent::getStatus() const
{
    return status;
}

void XmppEvent::setStatus(const DOMString &val)
{
    status = val;
}

DOMString XmppEvent::getTo() const
{
    return to;
}

void XmppEvent::setTo(const DOMString &val)
{
    to = val;
}

DOMString XmppEvent::getFrom() const
{
    return from;
}

void XmppEvent::setFrom(const DOMString &val)
{
    from = val;
}

DOMString XmppEvent::getGroup() const
{
    return group;
}

void XmppEvent::setGroup(const DOMString &val)
{
    group = val;
}

DOMString XmppEvent::getData() const
{
    return data;
}

void XmppEvent::setData(const DOMString &val)
{
    data = val;
}

DOMString XmppEvent::getFileName() const
{
    return fileName;
}

void XmppEvent::setFileName(const DOMString &val)
{
    fileName = val;
}

DOMString XmppEvent::getFileDesc() const
{
    return fileDesc;
}

void XmppEvent::setFileDesc(const DOMString &val)
{
    fileDesc = val;
}

long XmppEvent::getFileSize() const
{
    return fileSize;
}

void XmppEvent::setFileSize(long val)
{
    fileSize = val;
}

DOMString XmppEvent::getFileHash() const
{
    return fileHash;
}

void XmppEvent::setFileHash(const DOMString &val)
{
    fileHash = val;
}

Element *XmppEvent::getDOM() const
{
    return dom;
}

void XmppEvent::setDOM(const Element *val)
{
    if (!val)
        dom = NULL;
    else
        dom = ((Element *)val)->clone();
}


std::vector<XmppUser> XmppEvent::getUserList() const
{
    return userList;
}

void XmppEvent::setUserList(const std::vector<XmppUser> &val)
{
    userList = val;
}

//########################################################################
//# X M P P    E V E N T    T A R G E T
//########################################################################

//###########################
//# CONSTRUCTORS
//###########################

XmppEventTarget::XmppEventTarget()
{
    eventQueueEnabled = false;
}


XmppEventTarget::XmppEventTarget(const XmppEventTarget &other)
{
    listeners         = other.listeners;
    eventQueueEnabled = other.eventQueueEnabled;
}

XmppEventTarget::~XmppEventTarget()
{
}


//###########################
//# M E S S A G E S
//###########################

void XmppEventTarget::error(char *fmt, ...)
{
    va_list args;
    va_start(args,fmt);
    vsnprintf(targetWriteBuf, targetWriteBufLen, fmt, args);
    va_end(args) ;
    printf("Error:%s\n", targetWriteBuf);
    XmppEvent evt(XmppEvent::EVENT_ERROR);
    evt.setData(targetWriteBuf);
    dispatchXmppEvent(evt);
}

void XmppEventTarget::status(char *fmt, ...)
{
    va_list args;
    va_start(args,fmt);
    vsnprintf(targetWriteBuf, targetWriteBufLen, fmt, args);
    va_end(args) ;
    //printf("Status:%s\n", targetWriteBuf);
    XmppEvent evt(XmppEvent::EVENT_STATUS);
    evt.setData(targetWriteBuf);
    dispatchXmppEvent(evt);
}



//###########################
//# L I S T E N E R S
//###########################

void XmppEventTarget::dispatchXmppEvent(const XmppEvent &event)
{
    std::vector<XmppEventListener *>::iterator iter;
    for (iter = listeners.begin(); iter != listeners.end() ; iter++)
        (*iter)->processXmppEvent(event);
    if (eventQueueEnabled)
        eventQueue.push_back(event);
}

void XmppEventTarget::addXmppEventListener(const XmppEventListener &listener)
{
    XmppEventListener *lsnr = (XmppEventListener *)&listener;
    std::vector<XmppEventListener *>::iterator iter;
    for (iter = listeners.begin(); iter != listeners.end() ; iter++)
        if (*iter == lsnr)
            return;
    listeners.push_back(lsnr);
}

void XmppEventTarget::removeXmppEventListener(const XmppEventListener &listener)
{
    XmppEventListener *lsnr = (XmppEventListener *)&listener;
    std::vector<XmppEventListener *>::iterator iter;
    for (iter = listeners.begin(); iter != listeners.end() ; iter++)
        if (*iter == lsnr)
            listeners.erase(iter);
}

void XmppEventTarget::clearXmppEventListeners()
{
    listeners.clear();
}


//###########################
//# E V E N T    Q U E U E
//###########################

void XmppEventTarget::eventQueueEnable(bool val)
{
    eventQueueEnabled = val;
    if (!eventQueueEnabled)
        eventQueue.clear();
}

int XmppEventTarget::eventQueueAvailable()
{
    return eventQueue.size();
}

XmppEvent XmppEventTarget::eventQueuePop()
{
    if (!eventQueueEnabled || eventQueue.size()<1)
        {
        XmppEvent dummy(XmppEvent::EVENT_NONE);
        return dummy;
        }
    XmppEvent event = *(eventQueue.begin());
    eventQueue.erase(eventQueue.begin());
    return event;
}


//########################################################################
//# X M P P    S T R E A M
//########################################################################

/**
 *
 */
class XmppStream
{
public:

    /**
     *
     */
    XmppStream();

    /**
     *
     */
    virtual ~XmppStream();

    /**
     *
     */
    virtual void reset();

    /**
     *
     */
    virtual int getState();

    /**
     *
     */
    virtual void setState(int val);

    /**
     *
     */
    virtual DOMString getStreamId();

    /**
     *
     */
    void setStreamId(const DOMString &val);

    /**
     *
     */
    virtual DOMString getIqId();

    /**
     *
     */
    void setIqId(const DOMString &val);

    /**
     *
     */
    virtual int getSeqNr();

    /**
     *
     */
    virtual DOMString getPeerId();

    /**
     *
     */
    virtual void setPeerId(const DOMString &val);

    /**
     *
     */
    int available();

    /**
     *
     */
    void receiveData(std::vector<unsigned char> &newData);

    /**
     *
     */
    std::vector<unsigned char> read();

private:


    DOMString streamId;

    DOMString iqId;

    DOMString sourceId;

    int state;

    long seqNr;

    std::vector<unsigned char> data;
};


/**
 *
 */
XmppStream::XmppStream()
{
    reset();
}

/**
 *
 */
XmppStream::~XmppStream()
{
    reset();
}

/**
 *
 */
void XmppStream::reset()
{
    state = XmppClient::STREAM_AVAILABLE;
    seqNr = 0;
    data.clear();
}

/**
 *
 */
int XmppStream::getState()
{
    return state;
}

/**
 *
 */
void XmppStream::setState(int val)
{
    state = val;
}

/**
 *
 */
DOMString XmppStream::getStreamId()
{
    return streamId;
}

/**
 *
 */
void XmppStream::setStreamId(const DOMString &val)
{
    streamId = val;
}

/**
 *
 */
DOMString XmppStream::getIqId()
{
    return iqId;
}


/**
 *
 */
void XmppStream::setIqId(const DOMString &val)
{
    iqId = val;
}

/**
 *  Source or destination JID
 */
void XmppStream::setPeerId(const DOMString &val)
{
    sourceId = val;
}

/**
 *  Source or destination JID
 */
DOMString XmppStream::getPeerId()
{
    return sourceId;
}

/**
 *  Stream packet sequence number
 */
int XmppStream::getSeqNr()
{
    seqNr++;
    if (seqNr >= 65535)
        seqNr = 0;
    return seqNr;
}

/**
 *
 */
int XmppStream::available()
{
    return data.size();
}

/**
 *
 */
void XmppStream::receiveData(std::vector<unsigned char> &newData)
{
    std::vector<unsigned char>::iterator iter;
    for (iter=newData.begin() ; iter!=newData.end() ; iter++)
        data.push_back(*iter);
}

/**
 *
 */
std::vector<unsigned char> XmppStream::read()
{
    if (state != XmppClient::STREAM_OPEN)
        {
        std::vector<unsigned char>dummy;
        return dummy;
        }
    std::vector<unsigned char> ret = data;
    data.clear();
    return ret;
}






//########################################################################
//# X M P P    C L I E N T
//########################################################################
class ReceiverThread : public Runnable
{
public:

    ReceiverThread(XmppClient &par) : client(par) {}

    virtual ~ReceiverThread() {}

    void run()
      { client.receiveAndProcessLoop(); }

private:

    XmppClient &client;
};


//###########################
//# CONSTRUCTORS
//###########################

XmppClient::XmppClient()
{
    init();
}


XmppClient::XmppClient(const XmppClient &other) : XmppEventTarget(other)
{
    init();
    assign(other);
}

void XmppClient::assign(const XmppClient &other)
{
    msgId         = other.msgId;
    host          = other.host;
    realm         = other.realm;
    port          = other.port;
    username      = other.username;
    password      = other.password;
    resource      = other.resource;
    connected     = other.connected;
    doRegister    = other.doRegister;
    groupChats    = other.groupChats;
}


void XmppClient::init()
{
    sock          = new TcpSocket();
    msgId         = 0;
    connected     = false;
    doRegister    = false;

    for (int i=0 ; i<outputStreamCount ; i++)
        {
        outputStreams[i] = new XmppStream();
        }
    for (int i=0 ; i<inputStreamCount ; i++)
        {
        inputStreams[i] = new XmppStream();
        }
    for (int i=0 ; i<fileSendCount ; i++)
        {
        fileSends[i] = new XmppStream();
        }
}

XmppClient::~XmppClient()
{
    disconnect();
    delete sock;
    for (int i=0 ; i<outputStreamCount ; i++)
        {
        delete outputStreams[i];
        }
    for (int i=0 ; i<inputStreamCount ; i++)
        {
        delete inputStreams[i];
        }
    for (int i=0 ; i<fileSendCount ; i++)
        {
        delete fileSends[i];
        }
    groupChatsClear();
}

//##############################################
//# UTILILY
//##############################################

/**
 *
 */
bool XmppClient::pause(unsigned long millis)
{
    Thread::sleep(millis);
    return true;
}


static int strIndex(const DOMString &str, char *key)
{
    unsigned int p = str.find(key);
    if (p == DOMString::npos)
        return -1;
    return p;
}


DOMString XmppClient::toXml(const DOMString &str)
{
    return Parser::encode(str);
}

static DOMString trim(const DOMString &str)
{
    unsigned int i;
    for (i=0 ; i<str.size() ; i++)
        if (!isspace(str[i]))
            break;
    int start = i;
    for (i=str.size() ; i>0 ; i--)
        if (!isspace(str[i-1]))
            break;
    int end = i;
    if (start>=end)
        return "";
    return str.substr(start, end);
}

//##############################################
//# VARIABLES  (ones that need special handling)
//##############################################
/**
 *
 */
DOMString XmppClient::getUsername()
{
    return username;
}

/**
 *
 */
void XmppClient::setUsername(const DOMString &val)
{
    int p = strIndex(val, "@");
    if (p > 0)
        {
        username = val.substr(0, p);
        realm    = val.substr(p+1, jid.size()-p-1);
        }
    else
       {
       realm    = host;
       username = val;
       }
}

//##############################################
//# CONNECTION
//##############################################

//#######################
//# RECEIVING
//#######################
DOMString XmppClient::readStanza()
{
    int  openCount    = 0;
    bool inTag        = false;
    bool slashSeen    = false;
    bool trivialTag   = false;
    bool querySeen    = false;
    bool inQuote      = false;
    bool textSeen     = false;
    DOMString buf;

    while (true)
        {
        int ch = sock->read();
        //printf("%c", ch); fflush(stdout);
        if (ch<0)
            {
            if (ch == -2) //a simple timeout, not an error
                {
                //Since we are timed out, let's assume that we
                //are between chunks of text.  Let's reset all states.
                //printf("-----#### Timeout\n");
                continue;
                }
            else
                {
                keepGoing = false;
                if (!sock->isConnected())
                    {
                    disconnect();
                    return "";
                    }
                else
                    {
                    error("socket read error");
                    disconnect();
                    return "";
                    }
                }
            }
        buf.push_back(ch);
        if (ch == '<')
            {
            inTag      = true;
            slashSeen  = false;
            querySeen  = false;
            inQuote    = false;
            textSeen   = false;
            trivialTag = false;
            }
        else if (ch == '>')
            {
            if (!inTag)  //unescaped '>' in pcdata? horror
                continue;
            inTag     = false;
            if (!trivialTag && !querySeen)
                {
                if (slashSeen)
                    openCount--;
                else
                    openCount++;
                }
            //printf("# openCount:%d t:%d q:%d\n",
            //      openCount, trivialTag, querySeen);
            //check if we are 'balanced', but not a <?version?> tag
            if (openCount <= 0 && !querySeen)
                {
                break;
                }
            //we know that this one will be open-ended
            if (strIndex(buf, "<stream:stream") >= 0)
                {
                buf.append("</stream:stream>");
                break;
                }
            }
        else if (ch == '/')
            {
            if (inTag && !inQuote)
                {
                slashSeen = true;
                if (textSeen) // <tagName/>  <--looks like this
                    trivialTag = true;
                }
            }
        else if (ch == '?')
            {
            if (inTag && !inQuote)
                querySeen = true;
            }
        else if (ch == '"' || ch == '\'')
            {
            if (inTag)
                inQuote = !inQuote;
            }
        else
            {
            if (inTag && !inQuote && !isspace(ch))
                textSeen = true;
            }
        }
    return buf;
}



static bool isGroupChat(Element *root)
{
    if (!root)
        return false;
    std::vector<Element *>elems = root->findElements("x");
    for (unsigned int i=0 ; i<elems.size() ; i++)
        {
        DOMString xmlns = elems[i]->getAttribute("xmlns");
        //printf("### XMLNS ### %s\n", xmlns.c_str());
        if (strIndex(xmlns, "http://jabber.org/protocol/muc") >=0 )
            return true;
        }
   return false;
}




static bool parseJid(const DOMString &fullJid,
             DOMString &jid, DOMString &resource)
{
    int p = strIndex(fullJid, "/");
    if (p < 0)
        {
        jid = fullJid;
        resource = "";
        return true;
        }
    jid = fullJid.substr(0, p);
    resource = fullJid.substr(p+1, fullJid.size()-p-1);
    return true;
}




bool XmppClient::processMessage(Element *root)
{
    DOMString from    = root->getTagAttribute("message", "from");
    DOMString to      = root->getTagAttribute("message", "to");
    DOMString type    = root->getTagAttribute("message", "type");

    //####Check for embedded namespaces here
    //# IN BAND BYTESTREAMS
    DOMString ibbNamespace = "http://jabber.org/protocol/ibb";
    if (root->getTagAttribute("data", "xmlns") == ibbNamespace)
        {
        DOMString streamId = root->getTagAttribute("data", "sid");
        if (streamId.size() > 0)
            {
            for (int i=0 ; i<inputStreamCount ; i++)
                {
                XmppStream *ins = inputStreams[i];
                //printf("##ins:%s  streamid:%s\n",
                //    ins->getStreamId().c_str(),  streamId.c_str());
                if (ins->getStreamId() == streamId)
                    {
                    //# We have a winner
                    if (ins->getState() != STREAM_OPEN)
                        {
                        XmppEvent event(XmppEvent::EVENT_ERROR);
                        event.setFrom(from);
                        event.setData("received unrequested stream data");
                        dispatchXmppEvent(event);
                        return true;
                        }
                    DOMString data = root->getTagValue("data");
                    std::vector<unsigned char>binData =
                               Base64Decoder::decode(data);
                    ins->receiveData(binData);
                    }
                }
            }
        }


    //#### NORMAL MESSAGES
    DOMString subject = root->getTagValue("subject");
    DOMString body    = root->getTagValue("body");
    DOMString thread  = root->getTagValue("thread");
    //##rfc 3921, para 2.4.  ignore if no recognizable info
    if (subject.size() < 1 && body.size()<1 && thread.size()<1)
        return true;

    if (type == "groupchat")
        {
        DOMString fromGid;
        DOMString fromNick;
        parseJid(from, fromGid, fromNick);
        //printf("fromGid:%s  fromNick:%s\n",
        //        fromGid.c_str(), fromNick.c_str());
        DOMString toGid;
        DOMString toNick;
        parseJid(to, toGid, toNick);
        //printf("toGid:%s  toNick:%s\n",
        //        toGid.c_str(), toNick.c_str());

        if (fromNick.size() > 0)//normal group message
            {
            XmppEvent event(XmppEvent::EVENT_MUC_MESSAGE);
            event.setGroup(fromGid);
            event.setFrom(fromNick);
            event.setData(body);
            event.setDOM(root);
            dispatchXmppEvent(event);
            }
        else // from the server itself
            {
            //note the space before, so it doesnt match 'unlocked'
            if (strIndex(body, " locked") >= 0)
                {
                printf("LOCKED!! ;)\n");
                char *fmt =
                "<iq from='%s' id='create%d' to='%s' type='set'>"
                "<query xmlns='http://jabber.org/protocol/muc#owner'>"
                "<x xmlns='jabber:x:data' type='submit'/>"
                "</query></iq>\n";
                if (!write(fmt, jid.c_str(), msgId++, fromGid.c_str()))
                    return false;
                }
            }
        }
    else
        {
        XmppEvent event(XmppEvent::EVENT_MESSAGE);
        event.setFrom(from);
        event.setData(body);
        event.setDOM(root);
        dispatchXmppEvent(event);
        }

    return true;
}




bool XmppClient::processPresence(Element *root)
{

    DOMString fullJid     = root->getTagAttribute("presence", "from");
    DOMString to          = root->getTagAttribute("presence", "to");
    DOMString presenceStr = root->getTagAttribute("presence", "type");
    bool presence = true;
    if (presenceStr == "unavailable")
        presence = false;
    DOMString status      = root->getTagValue("status");
    DOMString show        = root->getTagValue("show");

    if (isGroupChat(root))
        {
        DOMString fromGid;
        DOMString fromNick;
        parseJid(fullJid, fromGid, fromNick);
        //printf("fromGid:%s  fromNick:%s\n",
        //        fromGid.c_str(), fromNick.c_str());
        DOMString item_jid = root->getTagAttribute("item", "jid");
        if (item_jid == jid) //Me
            {
            if (presence)
                {
                groupChatCreate(fromGid);
                groupChatUserAdd(fromGid, fromNick, "");
                groupChatUserShow(fromGid, fromNick, "available");

                XmppEvent event(XmppEvent::EVENT_MUC_JOIN);
                event.setGroup(fromGid);
                event.setFrom(fromNick);
                event.setPresence(presence);
                event.setShow(show);
                event.setStatus(status);
                dispatchXmppEvent(event);
                }
            else
                {
                groupChatDelete(fromGid);
                groupChatUserDelete(fromGid, fromNick);

                XmppEvent event(XmppEvent::EVENT_MUC_LEAVE);
                event.setGroup(fromGid);
                event.setFrom(fromNick);
                event.setPresence(presence);
                event.setShow(show);
                event.setStatus(status);
                dispatchXmppEvent(event);
                }
            }
        else // someone else
            {
            if (presence)
                {
                groupChatUserAdd(fromGid, fromNick, "");
                }
            else
                groupChatUserDelete(fromGid, fromNick);
            groupChatUserShow(fromGid, fromNick, show);
            XmppEvent event(XmppEvent::EVENT_MUC_PRESENCE);
            event.setGroup(fromGid);
            event.setFrom(fromNick);
            event.setPresence(presence);
            event.setShow(show);
            event.setStatus(status);
            dispatchXmppEvent(event);
            }
        }
    else
        {
        DOMString shortJid;
        DOMString dummy;
        parseJid(fullJid, shortJid, dummy);
        rosterShow(shortJid, show); //users in roster do not have resource

        XmppEvent event(XmppEvent::EVENT_PRESENCE);
        event.setFrom(fullJid);
        event.setPresence(presence);
        event.setShow(show);
        event.setStatus(status);
        dispatchXmppEvent(event);
        }

    return true;
}



bool XmppClient::processIq(Element *root)
{
    DOMString from  = root->getTagAttribute("iq", "from");
    DOMString id    = root->getTagAttribute("iq", "id");
    DOMString type  = root->getTagAttribute("iq", "type");
    DOMString xmlns = root->getTagAttribute("query", "xmlns");

    if (id.size()<1)
        return true;

    //Group chat
    if (strIndex(xmlns, "http://jabber.org/protocol/muc") >=0 )
        {
        printf("results of MUC query\n");
        }
    //printf("###IQ xmlns:%s\n", xmlns.c_str());

    //### FILE TRANSFERS
    DOMString siNamespace = "http://jabber.org/protocol/si";
    if (root->getTagAttribute("si", "xmlns") == siNamespace)
        {
        if (type == "set")
            {
            DOMString streamId = root->getTagAttribute("si", "id");
            DOMString fname    = root->getTagAttribute("file", "name");
            DOMString sizeStr  = root->getTagAttribute("file", "size");
            DOMString hash     = root->getTagAttribute("file", "hash");
            XmppEvent event(XmppEvent::XmppEvent::EVENT_FILE_RECEIVE);
            event.setFrom(from);
            event.setIqId(id);
            event.setStreamId(streamId);
            event.setFileName(fname);
            event.setFileHash(hash);
            event.setFileSize(atol(sizeStr.c_str()));
            dispatchXmppEvent(event);
            }
        else  //result
            {
            printf("Got result\n");
            for (int i=0 ; i<fileSendCount ; i++)
                {
                XmppStream *outf = fileSends[i];
                if (outf->getIqId() == id &&
                    from == outf->getPeerId())
                    {
                    if (type == "error")
                        {
                        outf->setState(STREAM_ERROR);
                        error("user '%s' rejected file", from.c_str());
                        return true;
                        }
                    else if (type == "result")
                        {
                        if (outf->getState() == STREAM_OPENING)
                            {
                            XmppEvent event(XmppEvent::XmppEvent::EVENT_FILE_ACCEPTED);
                            event.setFrom(from);
                            dispatchXmppEvent(event);
                            outf->setState(STREAM_OPEN);
                            }
                        else if (outf->getState() == STREAM_CLOSING)
                            {
                            outf->setState(STREAM_CLOSED);
                            }
                        return true;
                        }
                    }
                }
            }
        return true;
        }


    //### IN-BAND BYTESTREAMS
    //### Incoming stream requests
    DOMString ibbNamespace = "http://jabber.org/protocol/ibb";
    if (root->getTagAttribute("open", "xmlns") == ibbNamespace)
        {
        DOMString streamId = root->getTagAttribute("open", "sid");
        XmppEvent event(XmppEvent::XmppEvent::EVENT_STREAM_RECEIVE_INIT);
        dispatchXmppEvent(event);
        if (streamId.size()>0)
            {
            for (int i=0 ; i<inputStreamCount ; i++)
                {
                XmppStream *ins = inputStreams[i];
                if (ins->getStreamId() == streamId)
                    {
                    ins->setState(STREAM_OPENING);
                    ins->setIqId(id);
                    return true;
                    }
                }
            }
        return true;
        }
    else if (root->getTagAttribute("close", "xmlns") == ibbNamespace)
        {
        XmppEvent event(XmppEvent::XmppEvent::EVENT_STREAM_RECEIVE_CLOSE);
        dispatchXmppEvent(event);
        DOMString streamId = root->getTagAttribute("close", "sid");
        if (streamId.size()>0)
            {
            for (int i=0 ; i<inputStreamCount ; i++)
                {
                XmppStream *ins = inputStreams[i];
                if (ins->getStreamId() == streamId &&
                    from == ins->getPeerId())
                    {
                    ins->setState(STREAM_CLOSING);
                    ins->setIqId(id);
                    return true;
                    }
                }
            }
        return true;
        }
    //### Responses to outgoing requests
    for (int i=0 ; i<outputStreamCount ; i++)
        {
        XmppStream *outs = outputStreams[i];
        if (outs->getIqId() == id)
            {
            if (type == "error")
                {
                outs->setState(STREAM_ERROR);
                return true;
                }
            else if (type == "result")
                {
                if (outs->getState() == STREAM_OPENING)
                    {
                    outs->setState(STREAM_OPEN);
                    }
                else if (outs->getState() == STREAM_CLOSING)
                    {
                    outs->setState(STREAM_CLOSED);
                    }
                return true;
                }
            }
        }

    //###Normal Roster stuff
    if (root->getTagAttribute("query", "xmlns") == "jabber:iq:roster")
        {
        roster.clear();
        std::vector<Element *>elems = root->findElements("item");
        for (unsigned int i=0 ; i<elems.size() ; i++)
            {
            Element *item = elems[i];
            DOMString userJid      = item->getAttribute("jid");
            DOMString name         = item->getAttribute("name");
            DOMString subscription = item->getAttribute("subscription");
            DOMString group        = item->getTagValue("group");
            //printf("jid:%s name:%s sub:%s group:%s\n", userJid.c_str(), name.c_str(),
            //        subscription.c_str(), group.c_str());
            XmppUser user(userJid, name, subscription, group);
            roster.push_back(user);
            }
        XmppEvent event(XmppEvent::XmppEvent::EVENT_ROSTER);
        dispatchXmppEvent(event);
        }

    return true;
}



bool XmppClient::receiveAndProcess()
{
    if (!keepGoing)
        return false;

    Parser parser;

    DOMString recvBuf = readStanza();
    recvBuf = trim(recvBuf);
    if (recvBuf.size() < 1)
        return true;

    //Ugly hack.  Apparently the first char can be dropped on timeouts
    //if (recvBuf[0] != '<')
    //    recvBuf.insert(0, "<");

    status("RECV: %s", recvBuf.c_str());
    Element *root = parser.parse(recvBuf);
    if (!root)
        {
        printf("Bad elem\n");
        return true;
        }

    //#### MESSAGE
    std::vector<Element *>elems = root->findElements("message");
    if (elems.size()>0)
        {
        if (!processMessage(root))
            return false;
        }

    //#### PRESENCE
    elems = root->findElements("presence");
    if (elems.size()>0)
        {
        if (!processPresence(root))
            return false;
        }

    //#### INFO
    elems = root->findElements("iq");
    if (elems.size()>0)
        {
        if (!processIq(root))
            return false;
        }

    delete root;

    return true;
}


bool XmppClient::receiveAndProcessLoop()
{
    keepGoing = true;
    while (true)
        {
        if (!keepGoing)
            {
            printf("Abort requested\n");
            break;
            }
        if (!receiveAndProcess())
            return false;
        }
    return true;
}

//#######################
//# SENDING
//#######################

bool XmppClient::write(char *fmt, ...)
{
    va_list args;
    va_start(args,fmt);
    vsnprintf((char *)writeBuf, writeBufLen, fmt,args);
    va_end(args) ;
    status("SEND: %s", writeBuf);
    if (!sock->write((char *)writeBuf))
        {
        error("Cannot write to socket");
        return false;
        }
    return true;
}

//#######################
//# CONNECT
//#######################

bool XmppClient::checkConnect()
{
    if (!connected)
        {
        XmppEvent evt(XmppEvent::EVENT_ERROR);
        evt.setData("Attempted operation while disconnected");
        dispatchXmppEvent(evt);
        return false;
        }
    return true;
}

bool XmppClient::iqAuthenticate(const DOMString &streamId)
{
    Parser parser;

    char *fmt =
    "<iq type='get' to='%s' id='auth%d'>"
    "<query xmlns='jabber:iq:auth'><username>%s</username></query>"
    "</iq>\n";
    if (!write(fmt, realm.c_str(), msgId++, username.c_str()))
        return false;

    DOMString recbuf = readStanza();
    //printf("iq received: '%s'\n", recbuf.c_str());
    Element *elem = parser.parse(recbuf);
    //elem->print();
    DOMString iqType = elem->getTagAttribute("iq", "type");
    //printf("##iqType:%s\n", iqType.c_str());
    delete elem;

    if (iqType != "result")
        {
        error("error:server does not allow login");
        return false;
        }

    bool digest = true;
    if (digest)
        {
        //## Digest authentication
        DOMString digest = streamId;
        digest.append(password);
        digest = Sha1::hashHex((unsigned char *)digest.c_str(), digest.size());
        //printf("digest:%s\n", digest.c_str());
        fmt =
        "<iq type='set' id='auth%d'>"
        "<query xmlns='jabber:iq:auth'>"
        "<username>%s</username>"
        "<digest>%s</digest>"
        "<resource>%s</resource>"
        "</query>"
        "</iq>\n";
        if (!write(fmt, msgId++, username.c_str(),
                    digest.c_str(), resource.c_str()))
            return false;
        }
    else
        {

        //## Plaintext authentication
        fmt =
        "<iq type='set' id='auth%d'>"
        "<query xmlns='jabber:iq:auth'>"
        "<username>%s</username>"
        "<password>%s</password>"
        "<resource>%s</resource>"
        "</query>"
        "</iq>\n";
        if (!write(fmt, msgId++, username.c_str(),
                   password.c_str(), resource.c_str()))
            return false;
        }

    recbuf = readStanza();
    //printf("iq received: '%s'\n", recbuf.c_str());
    elem = parser.parse(recbuf);
    //elem->print();
    iqType = elem->getTagAttribute("iq", "type");
    //printf("##iqType:%s\n", iqType.c_str());
    delete elem;

    if (iqType != "result")
        {
        error("server does not allow login");
        return false;
        }

    return true;
}


static bool
saslParse(const DOMString &s, std::map<DOMString, DOMString> &vals)
{

    vals.clear();

    int p  = 0;
    int siz = s.size();

    while (p < siz)
        {
        DOMString key;
        DOMString value;
        char ch = '\0';

        //# Parse key
        while (p<siz)
            {
            ch = s[p++];
            if (ch == '=')
                break;
            key.push_back(ch);
            }

        //No value?
        if (ch != '=')
            break;

        //# Parse value
        bool quoted = false;
        while (p<siz)
            {
            ch = s[p++];
            if (ch == '"')
                quoted = !quoted;
            else if (ch == ',' && !quoted)
                break;
            else
                value.push_back(ch);
            }

        //printf("# Key: '%s'  Value: '%s'\n", key.c_str(), value.c_str());
        vals[key] = value;
        if (ch != ',')
            break;
        }

    return true;
}





bool XmppClient::saslMd5Authenticate()
{
    Parser parser;
    char *fmt =
    "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' "
    "mechanism='DIGEST-MD5'/>\n";
    if (!write(fmt))
        return false;

    DOMString recbuf = readStanza();
    status("challenge received: '%s'", recbuf.c_str());
    Element *elem = parser.parse(recbuf);
    //elem->print();
    DOMString b64challenge = elem->getTagValue("challenge");
    delete elem;

    if (b64challenge.size() < 1)
        {
        error("login: no SASL challenge offered by server");
        return false;
        }
    DOMString challenge = Base64Decoder::decodeToString(b64challenge);
    status("challenge:'%s'", challenge.c_str());

    std::map<DOMString, DOMString> attrs;
    if (!saslParse(challenge, attrs))
        {
        error("login: error parsing SASL challenge");
        return false;
        }

    DOMString nonce = attrs["nonce"];
    if (nonce.size()==0)
        {
        error("login: no SASL nonce sent by server");
        return false;
        }

    DOMString realm = attrs["realm"];
    if (realm.size()==0)
        {
        error("login: no SASL realm sent by server");
        return false;
        }

    status("SASL recv nonce: '%s' realm:'%s'\n", nonce.c_str(), realm.c_str());

    char idBuf[10];
    snprintf(idBuf, 9, "%dsasl", msgId++);
    DOMString cnonce = Sha1::hashHex((unsigned char *)idBuf, 7);
    DOMString authzid = username; authzid.append("@"); authzid.append(host);
    DOMString digest_uri = "xmpp/"; digest_uri.append(host);

    //## Make A1
    Md5 md5;
    md5.append(username);
    md5.append(":");
    md5.append(realm);
    md5.append(":");
    md5.append(password);
    unsigned char a1tmp[16];
    md5.finish(a1tmp);
    md5.init();
    md5.append(a1tmp, 16);
    md5.append(":");
    md5.append(nonce);
    md5.append(":");
    md5.append(cnonce);
    //RFC2831 says authzid is optional. Wildfire has trouble with authzid's
    //md5.append(":");
    //md5.append(authzid);
    md5.append("");
    DOMString a1 = md5.finishHex();
    status("##a1:'%s'", a1.c_str());

    //# Make A2
    md5.init();
    md5.append("AUTHENTICATE:");
    md5.append(digest_uri);
    DOMString a2 = md5.finishHex();
    status("##a2:'%s'", a2.c_str());

    //# Now make the response
    md5.init();
    md5.append(a1);
    md5.append(":");
    md5.append(nonce);
    md5.append(":");
    md5.append("00000001");//nc
    md5.append(":");
    md5.append(cnonce);
    md5.append(":");
    md5.append("auth");//qop
    md5.append(":");
    md5.append(a2);
    DOMString response = md5.finishHex();

    DOMString resp;
    resp.append("username=\""); resp.append(username); resp.append("\",");
    resp.append("realm=\"");    resp.append(realm);    resp.append("\",");
    resp.append("nonce=\"");    resp.append(nonce);    resp.append("\",");
    resp.append("cnonce=\"");   resp.append(cnonce);   resp.append("\",");
    resp.append("nc=00000001,qop=auth,");
    resp.append("digest-uri=\""); resp.append(digest_uri); resp.append("\"," );
    //resp.append("authzid=\"");  resp.append(authzid);  resp.append("\",");
    resp.append("response=");   resp.append(response); resp.append(",");
    resp.append("charset=utf-8");
    status("sending response:'%s'", resp.c_str());
    resp = Base64Encoder::encode(resp);
    status("base64 response:'%s'", resp.c_str());
    fmt =
    "<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>%s</response>\n";
    if (!write(fmt, resp.c_str()))
        return false;

    recbuf = readStanza();
    status("server says:: '%s'", recbuf.c_str());
    elem = parser.parse(recbuf);
    //elem->print();
    b64challenge = elem->getTagValue("challenge");
    delete elem;

    if (b64challenge.size() < 1)
        {
        error("login: no second SASL challenge offered by server");
        return false;
        }

    challenge = Base64Decoder::decodeToString(b64challenge);
    status("challenge: '%s'", challenge.c_str());

    if (!saslParse(challenge, attrs))
        {
        error("login: error parsing SASL challenge");
        return false;
        }

    DOMString rspauth = attrs["rspauth"];
    if (rspauth.size()==0)
        {
        error("login: no SASL respauth sent by server\n");
        return false;
        }

    fmt =
    "<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>\n";
    if (!write(fmt))
        return false;

    recbuf = readStanza();
    status("SASL recv: '%s", recbuf.c_str());
    elem = parser.parse(recbuf);
    //elem->print();
    b64challenge = elem->getTagValue("challenge");
    bool success = (elem->findElements("success").size() > 0);
    delete elem;

    return success;
}

bool XmppClient::saslPlainAuthenticate()
{
    Parser parser;

    DOMString id = username;
    //id.append("@");
    //id.append(host);
    Base64Encoder encoder;
    encoder.append('\0');
    encoder.append(id);
    encoder.append('\0');
    encoder.append(password);
    DOMString base64Auth = encoder.finish();
    //printf("authbuf:%s\n", base64Auth.c_str());

    char *fmt =
    "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' "
    "mechanism='PLAIN'>%s</auth>\n";
    if (!write(fmt, base64Auth.c_str()))
        return false;
    DOMString recbuf = readStanza();
    status("challenge received: '%s'", recbuf.c_str());
    Element *elem = parser.parse(recbuf);

    bool success = (elem->findElements("success").size() > 0);
    delete elem;

    return success;
}



bool XmppClient::saslAuthenticate()
{
    Parser parser;

    DOMString recbuf = readStanza();
    status("RECV: '%s'\n", recbuf.c_str());
    Element *elem = parser.parse(recbuf);
    //elem->print();

    //Check for starttls
    bool wantStartTls = false;
    if (elem->findElements("starttls").size() > 0)
        {
        wantStartTls = true;
        if (elem->findElements("required").size() > 0)
            status("login: STARTTLS required");
        else
            status("login: STARTTLS available");
        }

    if (wantStartTls && !sock->getEnableSSL())
        {
        delete elem;
        char *fmt =
        "<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>\n";
        if (!write(fmt))
            return false;
        recbuf = readStanza();
        status("RECV: '%s'\n", recbuf.c_str());
        elem = parser.parse(recbuf);
        if (elem->getTagAttribute("proceed", "xmlns").size()<1)
            {
            error("Server rejected TLS negotiation");
            disconnect();
            return false;
            }
        delete elem;
        if (!sock->startTls())
            {
            error("Could not start TLS");
            disconnect();
            return false;
            }

        fmt =
         "<stream:stream xmlns='jabber:client' "
         "xmlns:stream='http://etherx.jabber.org/streams' "
         "to='%s' version='1.0'>\n\n";
        if (!write(fmt, realm.c_str()))
            return false;

        recbuf = readStanza();
        status("RECVx: '%s'", recbuf.c_str());
        recbuf.append("</stream:stream>");
        elem = parser.parse(recbuf);
        bool success =
        (elem->getTagAttribute("stream:stream", "id").size()>0);
        if (!success)
            {
            error("STARTTLS negotiation failed");
            disconnect();
            return false;
            }
        delete elem;
        recbuf = readStanza();
        status("RECV: '%s'\n", recbuf.c_str());
        elem = parser.parse(recbuf);
        }

    //register, if user requests
    if (doRegister)
        {
        if (!inBandRegistration())
            return false;
        }

    //check for sasl authentication mechanisms
    std::vector<Element *> elems =
               elem->findElements("mechanism");
    if (elems.size() < 1)
        {
        error("login: no SASL mechanism offered by server");
        return false;
        }
    bool md5Found = false;
    bool plainFound = false;
    for (unsigned int i=0 ; i<elems.size() ; i++)
        {
        DOMString mech = elems[i]->getValue();
        if (mech == "DIGEST-MD5")
            {
            status("MD5 authentication offered");
            md5Found = true;
            }
        else if (mech == "PLAIN")
            {
            status("PLAIN authentication offered");
            plainFound = true;
            }
        }
    delete elem;

    bool success = false;
    if (md5Found)
        {
        success = saslMd5Authenticate();
        }
    else if (plainFound)
        {
        success = saslPlainAuthenticate();
        }
    else
        {
        error("not able to handle sasl authentication mechanisms");
        return false;
        }

    if (success)
        status("###### SASL authentication success\n");
    else
        error("###### SASL authentication failure\n");

    return success;
}


/**
 * Perform JEP-077 In-Band Registration
 */
bool XmppClient::inBandRegistration()
{
    Parser parser;

    char *fmt =
     "<iq type='get' id='reg1'>"
         "<query xmlns='jabber:iq:register'/>"
         "</iq>\n\n";
    if (!write(fmt))
        return false;

    DOMString recbuf = readStanza();
    status("RECV reg: %s", recbuf.c_str());
    Element *elem = parser.parse(recbuf);
    //elem->print();

    //# does the entity send the "instructions" tag?
    std::vector<Element *> fields = elem->findElements("field");
    std::vector<DOMString> fnames;
    for (unsigned int i=0; i<fields.size() ; i++)
        {
        DOMString fname = fields[i]->getAttribute("var");
        if (fname == "FORM_TYPE")
            continue;
        fnames.push_back(fname);
        status("field name:%s", fname.c_str());
        }

    delete elem;

    if (fnames.size() == 0)
        {
        error("server did not offer registration");
        return false;
        }


    fmt =
     "<iq type='set' id='reg2'>"
         "<query xmlns='jabber:iq:register'>"
         "<username>%s</username>"
         "<password>%s</password>"
         "<email/><name/>"
         "</query>"
         "</iq>\n\n";
    if (!write(fmt, toXml(username).c_str(),
                    toXml(password).c_str() ))
        return false;


    recbuf = readStanza();
    status("RECV reg: %s", recbuf.c_str());
    elem = parser.parse(recbuf);
    //elem->print();

    std::vector<Element *> list = elem->findElements("error");
    if (list.size()>0)
        {
        Element *errElem = list[0];
        DOMString code = errElem->getAttribute("code");
        DOMString errMsg = "Registration error. ";
        if (code == "409")
            {
            errMsg.append("conflict with existing user name");
            }
        if (code == "406")
            {
            errMsg.append("some registration information was not provided");
            }
        error((char *)errMsg.c_str());
        delete elem;
        return false;
        }

    delete elem;

    XmppEvent evt(XmppEvent::EVENT_REGISTRATION_NEW);
    evt.setTo(username);
    evt.setFrom(host);
    dispatchXmppEvent(evt);

    return true;
}


bool XmppClient::createSession()
{

    Parser parser;
    if (port==443 || port==5223)
        sock->enableSSL(true);
    if (!sock->connect(host, port))
        {
        return false;
        }

    char *fmt =
     "<stream:stream "
          "to='%s' "
          "xmlns='jabber:client' "
          "xmlns:stream='http://etherx.jabber.org/streams' "
          "version='1.0'>\n\n";
    if (!write(fmt, realm.c_str()))
        return false;

    DOMString recbuf = readStanza();
    //printf("received: '%s'\n", recbuf.c_str());
    recbuf.append("</stream:stream>");
    Element *elem = parser.parse(recbuf);
    //elem->print();
    bool useSasl = false;
    DOMString streamId = elem->getTagAttribute("stream:stream", "id");
    //printf("### StreamID: %s\n", streamId.c_str());
    DOMString streamVersion = elem->getTagAttribute("stream:stream", "version");
    if (streamVersion == "1.0")
        useSasl = true;

    if (useSasl)
        {
        if (!saslAuthenticate())
            return false;
        fmt =
          "<stream:stream "
          "to='%s' "
          "xmlns='jabber:client' "
          "xmlns:stream='http://etherx.jabber.org/streams' "
          "version='1.0'>\n\n";

        if (!write(fmt, realm.c_str()))
            return false;
        recbuf = readStanza();
        recbuf.append("</stream:stream>\n");
        //printf("now server says:: '%s'\n", recbuf.c_str());
        elem = parser.parse(recbuf);
        //elem->print();
        delete elem;

        recbuf = readStanza();
        //printf("now server says:: '%s'\n", recbuf.c_str());
        elem = parser.parse(recbuf);
        bool hasBind = (elem->findElements("bind").size() > 0);
        //elem->print();
        delete elem;

        if (!hasBind)
            {
            error("no binding provided by server");
            return false;
            }


        }
    else // not SASL
        {
        if (!iqAuthenticate(streamId))
            return false;
        }


    //### Resource binding
    fmt =
    "<iq type='set' id='bind%d'>"
    "<bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'>"
    "<resource>%s</resource>"
    "</bind></iq>\n";
    if (!write(fmt, msgId++, resource.c_str()))
        return false;

    recbuf = readStanza();
    status("bind result: '%s'", recbuf.c_str());
    elem = parser.parse(recbuf);
    //elem->print();
    DOMString bindType = elem->getTagAttribute("iq", "type");
    //printf("##bindType:%s\n", bindType.c_str());
    delete elem;

    if (bindType != "result")
        {
        error("no binding with server failed");
        return false;
        }

    fmt =
    "<iq type='set' id='sess%d'>"
    "<session xmlns='urn:ietf:params:xml:ns:xmpp-session'/>"
    "</iq>\n";
    if (!write(fmt, msgId++))
        return false;

    recbuf = readStanza();
    status("session received: '%s'", recbuf.c_str());
    elem = parser.parse(recbuf);
    //elem->print();
    DOMString sessionType = elem->getTagAttribute("iq", "type");
    //printf("##sessionType:%s\n", sessionType.c_str());
    delete elem;

    if (sessionType != "result")
        {
        error("no session provided by server");
        return false;
        }

    //printf("########## COOL #########\n");
    //Now that we are bound, we have a valid JID
    jid = username;
    jid.append("@");
    jid.append(realm);
    jid.append("/");
    jid.append(resource);

    //We are now done with the synchronous handshaking.  Let's go into
    //async mode

    fmt =
     "<iq type='get' id='roster%d'><query xmlns='jabber:iq:roster'/></iq>\n";
    if (!write(fmt, msgId++))
        return false;

    fmt =
     "<iq type='get' id='discoItems%d' to='%s'>"
     "<query xmlns='http://jabber.org/protocol/disco#items'/></iq>\n";
    if (!write(fmt, msgId++, realm.c_str()))
        return false;

    fmt =
    "<iq type='get' id='discoInfo%d' to='conference.%s'>"
    "<query xmlns='http://jabber.org/protocol/disco#info'/></iq>\n";
    if (!write(fmt, msgId++, realm.c_str()))
        return false;

    fmt =
     "<presence/>\n";
    if (!write(fmt))
        return false;

    /*
    recbuf = readStanza();
    status("stream received: '%s'", recbuf.c_str());
    elem = parser.parse(recbuf);
    //elem->print();
    delete elem;
    */

    //We are now logged in
    status("Connected");
    connected = true;
    XmppEvent evt(XmppEvent::EVENT_CONNECTED);
    evt.setData(host);
    dispatchXmppEvent(evt);
    //Thread::sleep(1000000);

    sock->setReceiveTimeout(1000);
    ReceiverThread runner(*this);
    Thread thread(runner);
    thread.start();

    return true;
}

bool XmppClient::connect()
{
    if (!createSession())
        {
        disconnect();
        return false;
        }
    return true;
}


bool XmppClient::connect(DOMString hostArg, int portArg,
                         DOMString usernameArg,
                         DOMString passwordArg,
                         DOMString resourceArg)
{
    host     = hostArg;
    port     = portArg;
    password = passwordArg;
    resource = resourceArg;

    //parse this one
    setUsername(usernameArg);

    bool ret = connect();
    return ret;
}

bool XmppClient::disconnect()
{
    if (connected)
        {
        char *fmt =
        "<presence from='%s' type='unavailable'/>\n";
        write(fmt, jid.c_str());
        }
    keepGoing = false;
    connected = false;
    Thread::sleep(3000); //allow receiving thread to quit
    sock->disconnect();
    roster.clear();
    groupChatsClear();
    XmppEvent event(XmppEvent::EVENT_DISCONNECTED);
    event.setData(host);
    dispatchXmppEvent(event);
    return true;
}

//#######################
//# ROSTER
//#######################

bool XmppClient::rosterAdd(const DOMString &rosterGroup,
                           const DOMString &otherJid,
                           const DOMString &name)
{
    if (!checkConnect())
        return false;
    char *fmt =
    "<iq from='%s' type='set' id='roster_%d'>"
    "<query xmlns='jabber:iq:roster'>"
    "<item jid='%s' name='%s'><group>%s</group></item>"
    "</query></iq>\n";
    if (!write(fmt, jid.c_str(), msgId++, otherJid.c_str(),
         name.c_str(), rosterGroup.c_str()))
        {
        return false;
        }
    return true;
}

bool XmppClient::rosterDelete(const DOMString &otherJid)
{
    if (!checkConnect())
        return false;
    char *fmt =
    "<iq from='%s' type='set' id='roster_%d'>"
    "<query xmlns='jabber:iq:roster'>"
    "<item jid='%s' subscription='remove'><group>%s</group></item>"
    "</query></iq>\n";
    if (!write(fmt, jid.c_str(), msgId++, otherJid.c_str()))
        {
        return false;
        }
    return true;
}


static bool xmppRosterCompare(const XmppUser& p1, const XmppUser& p2)
{
    DOMString s1 = p1.group;
    DOMString s2 = p2.group;
    for (unsigned int len=0 ; len<s1.size() && len<s2.size() ; len++)
        {
        int comp = tolower(s1[len]) - tolower(s2[len]);
        if (comp)
            return (comp<0);
        }

    s1 = p1.jid;
    s2 = p2.jid;
    for (unsigned int len=0 ; len<s1.size() && len<s2.size() ; len++)
        {
        int comp = tolower(s1[len]) - tolower(s2[len]);
        if (comp)
            return (comp<0);
        }
    return false;
}

std::vector<XmppUser> XmppClient::getRoster()
{
    std::vector<XmppUser> ros = roster;
    std::sort(ros.begin(), ros.end(), xmppRosterCompare);
    return ros;
}

void XmppClient::rosterShow(const DOMString &jid, const DOMString &show)
{
    DOMString theShow = show;
    if (theShow == "")
        theShow = "available";

    std::vector<XmppUser>::iterator iter;
    for (iter=roster.begin() ; iter != roster.end() ; iter++)
        {
        if (iter->jid == jid)
            iter->show = theShow;
        }
}

//#######################
//# CHAT (individual)
//#######################

bool XmppClient::message(const DOMString &user, const DOMString &subj,
                         const DOMString &msg)
{
    if (!checkConnect())
        return false;

    DOMString xmlSubj = toXml(subj);
    DOMString xmlMsg  = toXml(msg);

    if (xmlSubj.size() > 0)
        {
        char *fmt =
        "<message from='%s' to='%s' type='chat'>"
        "<subject>%s</subject><body>%s</body></message>\n";
        if (!write(fmt, jid.c_str(), user.c_str(),
                xmlSubj.c_str(), xmlMsg.c_str()))
            return false;
        }
    else
        {
        char *fmt =
        "<message from='%s' to='%s'>"
        "<body>%s</body></message>\n";
        if (!write(fmt, jid.c_str(), user.c_str(), xmlMsg.c_str()))
            return false;
        }
    return true;
}



bool XmppClient::message(const DOMString &user, const DOMString &msg)
{
    return message(user, "", msg);
}



bool XmppClient::presence(const DOMString &presence)
{
    if (!checkConnect())
        return false;

    DOMString xmlPres = toXml(presence);

    char *fmt =
    "<presence from='%s'><show>%s</show></presence>\n";
    if (!write(fmt, jid.c_str(), xmlPres.c_str()))
        return false;
    return true;
}

//#######################
//# GROUP  CHAT
//#######################

bool XmppClient::groupChatCreate(const DOMString &groupJid)
{
    std::vector<XmppGroupChat *>::iterator iter;
    for (iter=groupChats.begin() ; iter!=groupChats.end() ; iter++)
        {
        if ((*iter)->getGroupJid() == groupJid)
            {
            error("Group chat '%s' already exists", groupJid.c_str());
            return false;
            }
        }
    XmppGroupChat *chat = new XmppGroupChat(groupJid);
    groupChats.push_back(chat);
    return true;
}

/**
 *
 */
void XmppClient::groupChatDelete(const DOMString &groupJid)
{
    std::vector<XmppGroupChat *>::iterator iter;
    for (iter=groupChats.begin() ; iter!=groupChats.end() ; )
        {
        XmppGroupChat *chat = *iter;
        if (chat->getGroupJid() == groupJid)
            {
            iter = groupChats.erase(iter);
            delete chat;
            }
        else
            iter++;
        }
}

/**
 *
 */
bool XmppClient::groupChatExists(const DOMString &groupJid)
{
    std::vector<XmppGroupChat *>::iterator iter;
    for (iter=groupChats.begin() ; iter!=groupChats.end() ; iter++)
        if ((*iter)->getGroupJid() == groupJid)
            return true;
    return false;
}

/**
 *
 */
void XmppClient::groupChatsClear()
{
    std::vector<XmppGroupChat *>::iterator iter;
    for (iter=groupChats.begin() ; iter!=groupChats.end() ; iter++)
        delete (*iter);
    groupChats.clear();
}


/**
 *
 */
void XmppClient::groupChatUserAdd(const DOMString &groupJid,
                                  const DOMString &nick,
                                  const DOMString &jid)
{
    std::vector<XmppGroupChat *>::iterator iter;
    for (iter=groupChats.begin() ; iter!=groupChats.end() ; iter++)
        {
        if ((*iter)->getGroupJid() == groupJid)
            {
            (*iter)->userAdd(nick, jid);
            }
        }
}

/**
 *
 */
void XmppClient::groupChatUserShow(const DOMString &groupJid,
                                   const DOMString &nick,
                                   const DOMString &show)
{
    std::vector<XmppGroupChat *>::iterator iter;
    for (iter=groupChats.begin() ; iter!=groupChats.end() ; iter++)
        {
        if ((*iter)->getGroupJid() == groupJid)
            {
            (*iter)->userShow(nick, show);
            }
        }
}

/**
 *
 */
void XmppClient::groupChatUserDelete(const DOMString &groupJid,
                                     const DOMString &nick)
{
    std::vector<XmppGroupChat *>::iterator iter;
    for (iter=groupChats.begin() ; iter!=groupChats.end() ; iter++)
        {
        if ((*iter)->getGroupJid() == groupJid)
            {
            (*iter)->userDelete(nick);
            }
        }
}

static bool xmppUserCompare(const XmppUser& p1, const XmppUser& p2)
{
    DOMString s1 = p1.nick;
    DOMString s2 = p2.nick;
    int comp = 0;
    for (unsigned int len=0 ; len<s1.size() && len<s2.size() ; len++)
        {
        comp = tolower(s1[len]) - tolower(s2[len]);
        if (comp)
            break;
        }
    return (comp<0);
}


std::vector<XmppUser> XmppClient::groupChatGetUserList(
                              const DOMString &groupJid)
{
    if (!checkConnect())
        {
        std::vector<XmppUser> dummy;
        return dummy;
        }

    std::vector<XmppGroupChat *>::iterator iter;
    for (iter=groupChats.begin() ; iter!=groupChats.end() ; iter++)
        {
        if ((*iter)->getGroupJid() == groupJid )
            {
            std::vector<XmppUser> uList = (*iter)->getUserList();
            std::sort(uList.begin(), uList.end(), xmppUserCompare);
            return uList;
            }
        }
    std::vector<XmppUser> dummy;
    return dummy;
}

bool XmppClient::groupChatJoin(const DOMString &groupJid,
                               const DOMString &nick,
                               const DOMString &pass)
{
    if (!checkConnect())
        return false;

    DOMString user = nick;
    if (user.size()<1)
        user = username;

    char *fmt =
    "<presence to='%s/%s'>"
    "<x xmlns='http://jabber.org/protocol/muc'/></presence>\n";
    if (!write(fmt, groupJid.c_str(), user.c_str()))
        return false;
    return true;
}


bool XmppClient::groupChatLeave(const DOMString &groupJid,
                                const DOMString &nick)
{
    if (!checkConnect())
        return false;

    DOMString user = nick;
    if (user.size()<1)
        user = username;

    char *fmt =
    "<presence to='%s/%s' type='unavailable'>"
    "<x xmlns='http://jabber.org/protocol/muc'/></presence>\n";
    if (!write(fmt, groupJid.c_str(), user.c_str()))
        return false;
    return true;
}


bool XmppClient::groupChatMessage(const DOMString &groupJid,
                                  const DOMString &msg)
{
    if (!checkConnect())
        {
        return false;
        }

    DOMString xmlMsg = toXml(msg);

    char *fmt =
    "<message from='%s' to='%s' type='groupchat'>"
    "<body>%s</body></message>\n";
    if (!write(fmt, jid.c_str(), groupJid.c_str(), xmlMsg.c_str()))
        return false;
    return true;
}

bool XmppClient::groupChatPrivateMessage(const DOMString &groupJid,
                                         const DOMString &toNick,
                                         const DOMString &msg)
{
    if (!checkConnect())
        return false;

    DOMString xmlMsg = toXml(msg);

    char *fmt =
    "<message from='%s' to='%s/%s' type='chat'>"
    "<body>%s</body></message>\n";
    if (!write(fmt, jid.c_str(), groupJid.c_str(),
               toNick.c_str(), xmlMsg.c_str()))
        return false;
    return true;
}

bool XmppClient::groupChatPresence(const DOMString &groupJid,
                                   const DOMString &myNick,
                                   const DOMString &presence)
{
    if (!checkConnect())
        return false;

    DOMString user = myNick;
    if (user.size()<1)
        user = username;

    DOMString xmlPresence = toXml(presence);

    char *fmt =
    "<presence from='%s' to='%s/%s' type='unavailable'>"
    "<x xmlns='http://jabber.org/protocol/muc'/></presence>\n";
    if (!write(fmt, jid.c_str(), groupJid.c_str(), user.c_str(), xmlPresence.c_str()))
        return true;
    return true;
}



//#######################
//# S T R E A M S
//#######################


/**
 *
 */
int XmppClient::outputStreamOpen(const DOMString &destId,
                                 const DOMString &streamIdArg)
{
    int i;
    for (i=0; i<outputStreamCount ; i++)
        if (outputStreams[i]->getState() == STREAM_AVAILABLE)
            break;
    if (i>=outputStreamCount)
        {
        error("No available output streams");
        return -1;
        }
    int streamNr = i;
    XmppStream *outs = outputStreams[streamNr];

    outs->setState(STREAM_OPENING);

    char buf[32];
    snprintf(buf, 31, "inband%d", getMsgId());
    DOMString iqId = buf;

    DOMString streamId = streamIdArg;
    if (streamId.size()<1)
        {
        snprintf(buf, 31, "stream%d", getMsgId());
        DOMString streamId = buf;
        }
    outs->setIqId(iqId);
    outs->setStreamId(streamId);
    outs->setPeerId(destId);

    char *fmt =
    "<iq type='set' from='%s' to='%s' id='%s'>"
    "<open sid='%s' block-size='4096'"
    " xmlns='http://jabber.org/protocol/ibb'/></iq>\n";
    if (!write(fmt, jid.c_str(),
              destId.c_str(), iqId.c_str(),
              streamId.c_str()))
        {
        outs->reset();
        return -1;
        }

    int state = outs->getState();
    for (int tim=0 ; tim<20 ; tim++)
        {
        if (state == STREAM_OPEN)
            break;
        else if (state == STREAM_ERROR)
            {
            printf("ERROR\n");
            outs->reset();
            return -1;
            }
        Thread::sleep(1000);
        state = outs->getState();
        }
    if (state != STREAM_OPEN)
        {
        printf("TIMEOUT ERROR\n");
        outs->reset();
        return -1;
        }

    return streamNr;
}

/**
 *
 */
int XmppClient::outputStreamWrite(int streamNr,
                      const unsigned char *buf, unsigned long len)
{
    XmppStream *outs = outputStreams[streamNr];

    unsigned long outLen = 0;
    unsigned char *p = (unsigned char *)buf;

    while (outLen < len)
        {
        unsigned long chunksize = 1024;
        if (chunksize + outLen > len)
            chunksize = len - outLen;

        Base64Encoder encoder;
        encoder.append(p, chunksize);
        DOMString b64data = encoder.finish();
        p      += chunksize;
        outLen += chunksize;

        char *fmt =
        "<message from='%s' to='%s' id='msg%d'>"
        "<data xmlns='http://jabber.org/protocol/ibb' sid='%s' seq='%d'>"
        "%s"
        "</data>"
        "<amp xmlns='http://jabber.org/protocol/amp'>"
        "<rule condition='deliver-at' value='stored' action='error'/>"
        "<rule condition='match-resource' value='exact' action='error'/>"
        "</amp>"
        "</message>\n";
        if (!write(fmt, jid.c_str(),
              outs->getPeerId().c_str(),
              getMsgId(),
              outs->getStreamId().c_str(),
              outs->getSeqNr(),
              b64data.c_str()))
            {
            outs->reset();
            return -1;
            }
        pause(5000);
        }
    return outLen;
}

/**
 *
 */
int XmppClient::outputStreamClose(int streamNr)
{
    XmppStream *outs = outputStreams[streamNr];

    char buf[32];
    snprintf(buf, 31, "inband%d", getMsgId());
    DOMString iqId = buf;
    outs->setIqId(iqId);

    outs->setState(STREAM_CLOSING);
    char *fmt =
    "<iq type='set' from='%s' to='%s' id='%s'>"
    "<close sid='%s' xmlns='http://jabber.org/protocol/ibb'/></iq>\n";
    if (!write(fmt, jid.c_str(),
                    outs->getPeerId().c_str(),
                    iqId.c_str(),
                    outs->getStreamId().c_str()))
        return false;

    int state = outs->getState();
    for (int tim=0 ; tim<20 ; tim++)
        {
        if (state == STREAM_CLOSED)
            break;
        else if (state == STREAM_ERROR)
            {
            printf("ERROR\n");
            outs->reset();
            return -1;
            }
        Thread::sleep(1000);
        state = outs->getState();
        }
    if (state != STREAM_CLOSED)
        {
        printf("TIMEOUT ERROR\n");
        outs->reset();
        return -1;
        }

    outs->reset();
    return 1;
}


/**
 *
 */
int XmppClient::inputStreamOpen(const DOMString &fromJid, const DOMString &streamId,
                                const DOMString &iqId)
{
    int i;
    for (i=0 ; i<inputStreamCount ; i++)
        {
        if (inputStreams[i]->getState() == STREAM_AVAILABLE)
            break;
        }
    if (i>=inputStreamCount)
        {
        error("No available input streams");
        return -1;
        }
    int streamNr = i;
    XmppStream *ins = inputStreams[streamNr];
    ins->reset();
    ins->setPeerId(fromJid);
    ins->setState(STREAM_CLOSED);
    ins->setStreamId(streamId);

    int state = ins->getState();
    for (int tim=0 ; tim<20 ; tim++)
        {
        if (state == STREAM_OPENING)
            break;
        else if (state == STREAM_ERROR)
            {
            printf("ERROR\n");
            ins->reset();
            return -1;
            }
        Thread::sleep(1000);
        state = ins->getState();
        }
    if (state != STREAM_OPENING)
        {
        printf("TIMEOUT ERROR\n");
        ins->reset();
        return -1;
        }
    char *fmt =
    "<iq type='result' from='%s' to='%s' id='%s'/>\n";
    if (!write(fmt, jid.c_str(),  fromJid.c_str(), ins->getIqId().c_str()))
        {
        return -1;
        }

    ins->setState(STREAM_OPEN);
    return streamNr;
}



/**
 *
 */
int XmppClient::inputStreamAvailable(int streamNr)
{
    XmppStream *ins = inputStreams[streamNr];
    return ins->available();
}

/**
 *
 */
std::vector<unsigned char> XmppClient::inputStreamRead(int streamNr)
{
    XmppStream *ins = inputStreams[streamNr];
    return ins->read();
}

/**
 *
 */
bool XmppClient::inputStreamClosing(int streamNr)
{
    XmppStream *ins = inputStreams[streamNr];
    if (ins->getState() == STREAM_CLOSING)
        return true;
    return false;
}


/**
 *
 */
int XmppClient::inputStreamClose(int streamNr)
{
    int ret=1;
    XmppStream *ins = inputStreams[streamNr];
    if (ins->getState() == STREAM_CLOSING)
        {
        char *fmt =
        "<iq type='result' from='%s' to='%s' id='%s'/>\n";
        if (!write(fmt, jid.c_str(),  ins->getPeerId().c_str(),
                    ins->getIqId().c_str()))
            {
            ret = -1;
            }
        }
    ins->reset();
    return ret;
}




//#######################
//# FILE   TRANSFERS
//#######################


/**
 *
 */
bool XmppClient::fileSend(const DOMString &destJidArg,
                          const DOMString &offeredNameArg,
                          const DOMString &fileNameArg,
                          const DOMString &descriptionArg)
{
    DOMString destJid     = destJidArg;
    DOMString offeredName = offeredNameArg;
    DOMString fileName    = fileNameArg;
    DOMString description = descriptionArg;

    int i;
    for (i=0; i<fileSendCount ; i++)
        if (fileSends[i]->getState() == STREAM_AVAILABLE)
            break;
    if (i>=fileSendCount)
        {
        error("No available file send streams");
        return false;
        }
    int fileSendNr = i;
    XmppStream *outf = fileSends[fileSendNr];

    outf->setState(STREAM_OPENING);

    struct stat finfo;
    if (stat(fileName.c_str(), &finfo)<0)
        {
        error("Cannot stat file '%s' for sending", fileName.c_str());
        return false;
        }
    long fileLen = finfo.st_size;
    if (!fileLen > 1000000)
        {
        error("'%s' too large", fileName.c_str());
        return false;
        }
    if (!S_ISREG(finfo.st_mode))
        {
        error("'%s' is not a regular file", fileName.c_str());
        return false;
        }
    FILE *f = fopen(fileName.c_str(), "rb");
    if (!f)
        {
        error("cannot open '%s' for sending", fileName.c_str());
        return false;
        }
    unsigned char *sendBuf = (unsigned char *)malloc(fileLen+1);
    if (!sendBuf)
        {
        error("cannot cannot allocate send buffer for %s", fileName.c_str());
        return false;
        }
    for (long i=0 ; i<fileLen && !feof(f); i++)
        {
        sendBuf[i] = fgetc(f);
        }
    fclose(f);

    //## get the last path segment from the whole path
    if (offeredName.size()<1)
        {
        int slashPos = -1;
        for (unsigned int i=0 ; i<fileName.size() ; i++)
            {
            int ch = fileName[i];
            if (ch == '/' || ch == '\\')
                slashPos = i;
            }
        if (slashPos>=0 && slashPos<=(int)(fileName.size()-1))
            {
            offeredName = fileName.substr(slashPos+1,
                                          fileName.size()-slashPos-1);
            printf("offeredName:%s\n", offeredName.c_str());
            }
        }

    char buf[32];
    snprintf(buf, 31, "file%d", getMsgId());
    DOMString iqId = buf;
    outf->setIqId(iqId);

    snprintf(buf, 31, "stream%d", getMsgId());
    DOMString streamId = buf;
    //outf->setStreamId(streamId);

    DOMString hash = Md5::hashHex(sendBuf, fileLen);
    printf("Hash:%s\n", hash.c_str());

    outf->setPeerId(destJid);

    char dtgBuf[81];
    struct tm *timeVal = gmtime(&(finfo.st_mtime));
    strftime(dtgBuf, 80, "%Y-%m-%dT%H:%M:%Sz", timeVal);

    char *fmt =
    "<iq type='set' id='%s' to='%s'>"
    "<si xmlns='http://jabber.org/protocol/si' id='%s'"
      " mime-type='text/plain'"
      " profile='http://jabber.org/protocol/si/profile/file-transfer'>"
    "<file xmlns='http://jabber.org/protocol/si/profile/file-transfer'"
          " name='%s' size='%d' hash='%s' date='%s'><desc>%s</desc></file>"
    "<feature xmlns='http://jabber.org/protocol/feature-neg'>"
    "<x xmlns='jabber:x:data' type='form'>"
    "<field var='stream-method' type='list-single'>"
    //"<option><value>http://jabber.org/protocol/bytestreams</value></option>"
    "<option><value>http://jabber.org/protocol/ibb</value></option>"
    "</field></x></feature></si></iq>\n";
    if (!write(fmt, iqId.c_str(), destJid.c_str(),
         streamId.c_str(), offeredName.c_str(), fileLen,
         hash.c_str(), dtgBuf, description.c_str()))
        {
        free(sendBuf);
        return false;
        }

    int state = outf->getState();
    for (int tim=0 ; tim<20 ; tim++)
        {
        printf("##### waiting for open\n");
        if (state == STREAM_OPEN)
            {
            outf->reset();
            break;
            }
        else if (state == STREAM_ERROR)
            {
            printf("ERROR\n");
            outf->reset();
            return false;
            }
        Thread::sleep(1000);
        state = outf->getState();
        }
    if (state != STREAM_OPEN)
        {
        printf("TIMEOUT ERROR\n");
        outf->reset();
        return false;
        }

    //free up this reqource
    outf->reset();

    int  streamNr = outputStreamOpen(destJid, streamId);
    if (streamNr<0)
        {
        error("cannot open output stream %s", streamId.c_str());
        outf->reset();
        return false;
        }

    int ret = outputStreamWrite(streamNr, sendBuf, fileLen);

    if (ret<0)
        {
        }

    outputStreamClose(streamNr);

    free(sendBuf);
    return true;
}


class FileSendThread : public Thread
{
public:

    FileSendThread(XmppClient &par,
                   const DOMString &destJidArg,
                   const DOMString &offeredNameArg,
                   const DOMString &fileNameArg,
                   const DOMString &descriptionArg) : client(par)
        {
        destJid     = destJidArg;
        offeredName = offeredNameArg;
        fileName    = fileNameArg;
        description = descriptionArg;
        }

    virtual ~FileSendThread() {}

    void run()
      {
      client.fileSend(destJid, offeredName,
                      fileName, description);
      }

private:

    XmppClient &client;
    DOMString destJid;
    DOMString offeredName;
    DOMString fileName;
    DOMString description;
};

/**
 *
 */
bool XmppClient::fileSendBackground(const DOMString &destJid,
                                    const DOMString &offeredName,
                                    const DOMString &fileName,
                                    const DOMString &description)
{
    FileSendThread thread(*this, destJid, offeredName,
                           fileName, description);
    thread.start();
    return true;
}


/**
 *
 */
bool XmppClient::fileReceive(const DOMString &fromJid,
                             const DOMString &iqId,
                             const DOMString &streamId,
                             const DOMString &fileName,
                             long  fileSize,
                             const DOMString &fileHash)
{
    char *fmt =
    "<iq type='result' to='%s' id='%s'>"
    "<si xmlns='http://jabber.org/protocol/si'>"
    "<file xmlns='http://jabber.org/protocol/si/profile/file-transfer'/>"
    "<feature xmlns='http://jabber.org/protocol/feature-neg'>"
    "<x xmlns='jabber:x:data' type='submit'>"
    "<field var='stream-method'>"
    "<value>http://jabber.org/protocol/ibb</value>"
    "</field></x></feature></si></iq>\n";
    if (!write(fmt, fromJid.c_str(), iqId.c_str()))
        {
        return false;
        }

    int streamNr = inputStreamOpen(fromJid, streamId, iqId);
    if (streamNr < 0)
        {
        return false;
        }


    Md5 md5;
    FILE *f = fopen(fileName.c_str(), "wb");
    if (!f)
        {
        return false;
        }

    while (true)
        {
        if (inputStreamAvailable(streamNr)<1)
            {
            if (inputStreamClosing(streamNr))
                break;
            pause(100);
            continue;
            }
        std::vector<unsigned char> ret = inputStreamRead(streamNr);
        std::vector<unsigned char>::iterator iter;
        for (iter=ret.begin() ; iter!=ret.end() ; iter++)
            {
            unsigned char ch = *iter;
            md5.append(&ch, 1);
            fwrite(&ch, 1, 1, f);
            }
        }

    inputStreamClose(streamNr);
    fclose(f);

    DOMString hash = md5.finishHex();
    printf("received file hash:%s\n", hash.c_str());

    return true;
}



class FileReceiveThread : public Thread
{
public:

    FileReceiveThread(XmppClient &par,
                      const DOMString &fromJidArg,
                      const DOMString &iqIdArg,
                      const DOMString &streamIdArg,
                      const DOMString &fileNameArg,
                      long  fileSizeArg,
                      const DOMString &fileHashArg) : client(par)
        {
        fromJid     = fromJidArg;
        iqId        = iqIdArg;
        streamId    = streamIdArg;
        fileName    = fileNameArg;
        fileSize    = fileSizeArg;
        fileHash    = fileHashArg;
        }

    virtual ~FileReceiveThread() {}

    void run()
      {
      client.fileReceive(fromJid, iqId, streamId,
                        fileName, fileSize, fileHash);
      }

private:

    XmppClient &client;
    DOMString fromJid;
    DOMString iqId;
    DOMString streamId;
    DOMString fileName;
    long      fileSize;
    DOMString fileHash;
};

/**
 *
 */
bool XmppClient::fileReceiveBackground(const DOMString &fromJid,
                                       const DOMString &iqId,
                                       const DOMString &streamId,
                                       const DOMString &fileName,
                                       long  fileSize,
                                       const DOMString &fileHash)
{
    FileReceiveThread thread(*this, fromJid, iqId, streamId,
                  fileName, fileSize, fileHash);
    thread.start();
    return true;
}



//########################################################################
//# X M P P    G R O U P    C H A T
//########################################################################

/**
 *
 */
XmppGroupChat::XmppGroupChat(const DOMString &groupJidArg)
{
    groupJid = groupJidArg;
}

/**
 *
 */
XmppGroupChat::XmppGroupChat(const XmppGroupChat &other)
{
    groupJid = other.groupJid;
    userList = other.userList;
}

/**
 *
 */
XmppGroupChat::~XmppGroupChat()
{
}


/**
 *
 */
DOMString XmppGroupChat::getGroupJid()
{
    return groupJid;
}


void XmppGroupChat::userAdd(const DOMString &nick,
                            const DOMString &jid)
{
    std::vector<XmppUser>::iterator iter;
    for (iter= userList.begin() ; iter!=userList.end() ; iter++)
        {
        if (iter->nick == nick)
            return;
        }
    XmppUser user(jid, nick);
    userList.push_back(user);
}

void XmppGroupChat::userShow(const DOMString &nick,
                             const DOMString &show)
{
    DOMString theShow = show;
    if (theShow == "")
        theShow = "available"; // a join message will now have a show
    std::vector<XmppUser>::iterator iter;
    for (iter= userList.begin() ; iter!=userList.end() ; iter++)
        {
        if (iter->nick == nick)
            iter->show = theShow;
        }
}

void XmppGroupChat::userDelete(const DOMString &nick)
{
    std::vector<XmppUser>::iterator iter;
    for (iter= userList.begin() ; iter!=userList.end() ; )
        {
        if (iter->nick == nick)
            iter = userList.erase(iter);
        else
            iter++;
        }
}

std::vector<XmppUser> XmppGroupChat::getUserList() const
{
    return userList;
}






} //namespace Pedro
//########################################################################
//# E N D    O F     F I L E
//########################################################################















