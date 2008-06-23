/*
 * Support classes for the Pedro mini-XMPP client
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
#include <string.h>
#include <sys/stat.h>

#include "pedroutil.h"



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

#endif /* UNIX */

#ifdef HAVE_SSL
RELAYTOOL_SSL
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


static const char *base64encode =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";



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
//########################################################################
//### S H A    1      H A S H I N G
//########################################################################
//########################################################################

void Sha1::hash(unsigned char *dataIn, int len, unsigned char *digest)
{
    Sha1 sha1;
    sha1.append(dataIn, len);
    sha1.finish(digest);
}

static const char *sha1hex = "0123456789abcdef";

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


DOMString Sha1::hashHex(const DOMString &str)
{
    return hashHex((unsigned char *)str.c_str(), str.size());
}


void Sha1::init()
{

    longNr    = 0;
    byteNr    = 0;
    nrBytesHi = 0;
    nrBytesLo = 0;

    // Initialize H with the magic constants (see FIPS180 for constants)
    hashBuf[0] = 0x67452301L;
    hashBuf[1] = 0xefcdab89L;
    hashBuf[2] = 0x98badcfeL;
    hashBuf[3] = 0x10325476L;
    hashBuf[4] = 0xc3d2e1f0L;

    for (int i = 0; i < 4; i++)
        inb[i] = 0;

    for (int i = 0; i < 80; i++)
        inBuf[i] = 0;
}


void Sha1::append(unsigned char ch)
{
    if (nrBytesLo == 0xffffffffL)
        {
        nrBytesHi++;
        nrBytesLo = 0;
        }
    else
        nrBytesLo++;

    inb[byteNr++] = (unsigned long)ch;
    if (byteNr >= 4)
        {
        inBuf[longNr++] = inb[0] << 24 | inb[1] << 16 |
                          inb[2] << 8  | inb[3];
        byteNr = 0;
        }
    if (longNr >= 16)
        {
        transform();
        longNr = 0;
        }
}


void Sha1::append(unsigned char *dataIn, int len)
{
    for (int i = 0; i < len; i++)
        append(dataIn[i]);
}


void Sha1::append(const DOMString &str)
{
    append((unsigned char *)str.c_str(), str.size());
}


void Sha1::finish(unsigned char digest[20])
{
    //snapshot the bit count now before padding
    unsigned long nrBitsLo = (nrBytesLo << 3) & 0xffffffff;
    unsigned long nrBitsHi = (nrBytesHi << 3) | ((nrBytesLo >> 29) & 7);

    //Append terminal char
    append(0x80);

    //pad until we have a 56 of 64 bytes, allowing for 8 bytes at the end
    while (longNr != 14)
        append(0);


    //##### Append length in bits
    append((unsigned char)((nrBitsHi>>24) & 0xff));
    append((unsigned char)((nrBitsHi>>16) & 0xff));
    append((unsigned char)((nrBitsHi>> 8) & 0xff));
    append((unsigned char)((nrBitsHi    ) & 0xff));
    append((unsigned char)((nrBitsLo>>24) & 0xff));
    append((unsigned char)((nrBitsLo>>16) & 0xff));
    append((unsigned char)((nrBitsLo>> 8) & 0xff));
    append((unsigned char)((nrBitsLo    ) & 0xff));


    //copy out answer
    int indx = 0;
    for (int i=0 ; i<5 ; i++)
        {
        digest[indx++] = (unsigned char)((hashBuf[i] >> 24) & 0xff);
        digest[indx++] = (unsigned char)((hashBuf[i] >> 16) & 0xff);
        digest[indx++] = (unsigned char)((hashBuf[i] >>  8) & 0xff);
        digest[indx++] = (unsigned char)((hashBuf[i]      ) & 0xff);
        }

    // Re-initialize the context (also zeroizes contents)
    init();
}



#define SHA_ROTL(X,n) ((((X) << (n)) & 0xffffffff) | (((X) >> (32-(n))) & 0xffffffff))

void Sha1::transform()
{
    unsigned long *W = inBuf;
    unsigned long *H = hashBuf;

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
        TEMP = (SHA_ROTL(A,5) + ((B&C)|((~B)&D)) +
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
        TEMP = (SHA_ROTL(A,5) + ((B&C)|(B&D)|(C&D)) +
                E + W[t] + 0x8f1bbcdcL) & 0xffffffffL;
        E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP;
        }
    for (int t = 60; t <= 79; t++)
        {
        TEMP = (SHA_ROTL(A,5) + (B^C^D) +
                E + W[t] + 0xca62c1d6L) & 0xffffffffL;
        E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP;
        }

    H[0] = (H[0] + A) & 0xffffffffL;
    H[1] = (H[1] + B) & 0xffffffffL;
    H[2] = (H[2] + C) & 0xffffffffL;
    H[3] = (H[3] + D) & 0xffffffffL;
    H[4] = (H[4] + E) & 0xffffffffL;
}



//########################################################################
//########################################################################
//### M D 5      H A S H I N G
//########################################################################
//########################################################################




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

DOMString Md5::hashHex(const DOMString &str)
{
    Md5 md5;
    md5.append(str);
    DOMString ret = md5.finishHex();
    return ret;
}


/**
 * Initialize MD5 polynomials and storage
 */
void Md5::init()
{
    hashBuf[0]  = 0x67452301;
    hashBuf[1]  = 0xefcdab89;
    hashBuf[2]  = 0x98badcfe;
    hashBuf[3]  = 0x10325476;

    nrBytesHi = 0;
    nrBytesLo = 0;
    byteNr    = 0;
    longNr    = 0;
}




/**
 * Update with one character
 */
void Md5::append(unsigned char ch)
{
    if (nrBytesLo == 0xffffffff)
        {
        nrBytesLo = 0;
        nrBytesHi++;
        }
    else
        nrBytesLo++;

    //pack 64 bytes into 16 longs
    inb[byteNr++] = (unsigned long)ch;
    if (byteNr >= 4)
        {
        unsigned long val =
             inb[3] << 24 | inb[2] << 16 | inb[1] << 8 | inb[0];
        inBuf[longNr++] = val;
        byteNr = 0;
        }
    if (longNr >= 16)
        {
        transform();
        longNr = 0;
        }
}


/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
void Md5::append(unsigned char *source, unsigned long len)
{
    while (len--)
        append(*source++);
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
    //snapshot the bit count now before padding
    unsigned long nrBitsLo = (nrBytesLo << 3) & 0xffffffff;
    unsigned long nrBitsHi = (nrBytesHi << 3) | ((nrBytesLo >> 29) & 7);

    //Append terminal char
    append(0x80);

    //pad until we have a 56 of 64 bytes, allowing for 8 bytes at the end
    while (longNr != 14)
        append(0);

    //##### Append length in bits
    append((unsigned char)((nrBitsLo    ) & 0xff));
    append((unsigned char)((nrBitsLo>> 8) & 0xff));
    append((unsigned char)((nrBitsLo>>16) & 0xff));
    append((unsigned char)((nrBitsLo>>24) & 0xff));
    append((unsigned char)((nrBitsHi    ) & 0xff));
    append((unsigned char)((nrBitsHi>> 8) & 0xff));
    append((unsigned char)((nrBitsHi>>16) & 0xff));
    append((unsigned char)((nrBitsHi>>24) & 0xff));

    //copy out answer
    int indx = 0;
    for (int i=0 ; i<4 ; i++)
        {
        digest[indx++] = (unsigned char)((hashBuf[i]      ) & 0xff);
        digest[indx++] = (unsigned char)((hashBuf[i] >>  8) & 0xff);
        digest[indx++] = (unsigned char)((hashBuf[i] >> 16) & 0xff);
        digest[indx++] = (unsigned char)((hashBuf[i] >> 24) & 0xff);
        }

    init();  // Security!  ;-)
}



static const char *md5hex = "0123456789abcdef";

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

// #define F1(x, y, z) (x & y | ~x & z)
#define M(x) ((x) &= 0xffffffff)
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

// ## This is the central step in the MD5 algorithm.
#define MD5STEP(f, w, x, y, z, data, s) \
	( w += (f(x, y, z) + data), M(w), w = w<<s | w>>(32-s), w += x, M(w) )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 * @parm buf points to an array of 4 unsigned 32bit (at least) integers
 * @parm in points to an array of 16 unsigned 32bit (at least) integers
 */
void Md5::transform()
{
    unsigned long *i = inBuf;
    unsigned long a  = hashBuf[0];
    unsigned long b  = hashBuf[1];
    unsigned long c  = hashBuf[2];
    unsigned long d  = hashBuf[3];

    MD5STEP(F1, a, b, c, d, i[ 0] + 0xd76aa478,  7);
    MD5STEP(F1, d, a, b, c, i[ 1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, i[ 2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, i[ 3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, i[ 4] + 0xf57c0faf,  7);
    MD5STEP(F1, d, a, b, c, i[ 5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, i[ 6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, i[ 7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, i[ 8] + 0x698098d8,  7);
    MD5STEP(F1, d, a, b, c, i[ 9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, i[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, i[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, i[12] + 0x6b901122,  7);
    MD5STEP(F1, d, a, b, c, i[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, i[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, i[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, i[ 1] + 0xf61e2562,  5);
    MD5STEP(F2, d, a, b, c, i[ 6] + 0xc040b340,  9);
    MD5STEP(F2, c, d, a, b, i[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, i[ 0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, i[ 5] + 0xd62f105d,  5);
    MD5STEP(F2, d, a, b, c, i[10] + 0x02441453,  9);
    MD5STEP(F2, c, d, a, b, i[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, i[ 4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, i[ 9] + 0x21e1cde6,  5);
    MD5STEP(F2, d, a, b, c, i[14] + 0xc33707d6,  9);
    MD5STEP(F2, c, d, a, b, i[ 3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, i[ 8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, i[13] + 0xa9e3e905,  5);
    MD5STEP(F2, d, a, b, c, i[ 2] + 0xfcefa3f8,  9);
    MD5STEP(F2, c, d, a, b, i[ 7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, i[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, i[ 5] + 0xfffa3942,  4);
    MD5STEP(F3, d, a, b, c, i[ 8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, i[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, i[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, i[ 1] + 0xa4beea44,  4);
    MD5STEP(F3, d, a, b, c, i[ 4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, i[ 7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, i[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, i[13] + 0x289b7ec6,  4);
    MD5STEP(F3, d, a, b, c, i[ 0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, i[ 3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, i[ 6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, i[ 9] + 0xd9d4d039,  4);
    MD5STEP(F3, d, a, b, c, i[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, i[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, i[ 2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, i[ 0] + 0xf4292244,  6);
    MD5STEP(F4, d, a, b, c, i[ 7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, i[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, i[ 5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, i[12] + 0x655b59c3,  6);
    MD5STEP(F4, d, a, b, c, i[ 3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, i[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, i[ 1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, i[ 8] + 0x6fa87e4f,  6);
    MD5STEP(F4, d, a, b, c, i[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, i[ 6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, i[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, i[ 4] + 0xf7537e82,  6);
    MD5STEP(F4, d, a, b, c, i[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, i[ 2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, i[ 9] + 0xeb86d391, 21);

    hashBuf[0] += a;
    hashBuf[1] += b;
    hashBuf[2] += c;
    hashBuf[3] += d;
}





//########################################################################
//########################################################################
//### T H R E A D
//########################################################################
//########################################################################





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


TcpSocket::TcpSocket(const std::string &hostnameArg, int port)
{
    init();
    hostname  = hostnameArg;
    portno    = port;
}




#ifdef HAVE_SSL

static void cryptoLockCallback(int mode, int type, const char */*file*/, int /*line*/)
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
        //how do we pass a context pointer here?
        //error("openssl (lock_dbg_cb): %s (mode=%d, type=%d) at %s:%d",
        //        errstr, mode, type, file, line);
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


void TcpSocket::error(const char *fmt, ...)
{
    static char buf[256];
    lastError = "TcpSocket err: ";
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 255, fmt, args);
    va_end(args);
    lastError.append(buf);
    fprintf(stderr, "%s\n", lastError.c_str());
}


DOMString &TcpSocket::getLastError()
{
    return lastError;
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

bool TcpSocket::getHaveSSL()
{
#ifdef HAVE_SSL
    if (libssl_is_present)
    {
        return true;
    } else {
	return false;
    }
#else
    return false;
#endif
}

void TcpSocket::enableSSL(bool val)
{
    sslEnabled = val;
}

bool TcpSocket::getEnableSSL()
{
    return sslEnabled;
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
#ifndef HAVE_SSL
    error("SSL starttls() error:  client not compiled with SSL enabled");
    return false;
#else /*HAVE_SSL*/
    if (!libssl_is_present)
        {
        error("SSL starttls() error:  the correct version of libssl was not found");
        return false;
        }

    sslStream  = NULL;
    sslContext = NULL;

    //SSL_METHOD *meth = SSLv23_method();
    //SSL_METHOD *meth = SSLv3_client_method();
    SSL_METHOD *meth = TLSv1_client_method();
    sslContext = SSL_CTX_new(meth);
    //SSL_CTX_set_info_callback(sslContext, infoCallback);

    /**
     * For now, let's accept all connections.  Ignore this
     * block of code     
     * 	     
    char *keyFile  = "client.pem";
    char *caList   = "root.pem";
    //#  Load our keys and certificates
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

    //## Load the CAs we trust
    if (!(SSL_CTX_load_verify_locations(sslContext, caList, 0)))
        {
        fprintf(stderr, "Can't read CA list\n");
        disconnect();
        return false;
        }
    */

    /* Connect the SSL socket */
    sslStream  = SSL_new(sslContext);
    SSL_set_fd(sslStream, sock);

    int ret = SSL_connect(sslStream);
    if (ret == 0)
        {
        error("SSL connection not successful");
        disconnect();
        return false;
        }
    else if (ret < 0)
        {
        int err = SSL_get_error(sslStream, ret);
        error("SSL connect error %d", err);
        disconnect();
        return false;
        }

    sslEnabled = true;
    return true;
#endif /* HAVE_SSL */
}


bool TcpSocket::connect()
{
    if (hostname.size()<1)
        {
        error("open: null hostname");
        return false;
        }

    if (portno<1)
        {
        error("open: bad port number");
        return false;
        }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        {
        error("open: error creating socket");
        return false;
        }

    char *c_hostname = (char *)hostname.c_str();
    struct hostent *server = gethostbyname(c_hostname);
    if (!server)
        {
        error("open: could not locate host '%s'", c_hostname);
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
        error("open: could not connect to host '%s'", c_hostname);
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
                    error("Shutdown failed");
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
        error("write: socket closed");
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
                    error("SSL write problem");
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
            error("write: could not send data");
            return false;
            }
        }
    return true;
}

bool TcpSocket::write(char *str)
{
   if (!isConnected())
        {
        error("write(str): socket closed");
        return false;
        }
    int len = strlen(str);

    if (sslEnabled)
        {
#ifdef HAVE_SSL
	if (libssl_is_present)
	{
        int r = SSL_write(sslStream, (unsigned char *)str, len);
        if (r<=0)
            {
            switch(SSL_get_error(sslStream, r))
                {
                default:
                    error("SSL write problem");
                    return -1;
                }
            }
	}
#endif
        }
    else
        {
        if (send(sock, str, len, 0) < 0)
        //if (send(sock, &c, 1, 0) < 0)
            {
            error("write: could not send data");
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
                error("SSL read problem(syscall) %s",
                     ERR_error_string(ERR_get_error(), NULL));
                return -1;
            default:
                error("SSL read problem %s",
                     ERR_error_string(ERR_get_error(), NULL));
                return -1;
            }
	}
#endif
        }
    else
        {
        if (recv(sock, (char *)&ch, 1, 0) <= 0)
            {
            error("read: could not receive data");
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









} //namespace Pedro
//########################################################################
//# E N D    O F     F I L E
//########################################################################











