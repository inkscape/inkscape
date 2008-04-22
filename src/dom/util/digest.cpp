/**
 * Secure Hashing Tool
 * *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2006-2008 Bob Jamison
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

#include "digest.h"


//########################################################################
//##  U T I L I T Y
//########################################################################

/**
 * Use this to print out a 64-bit int when otherwise difficult
 */
/*
static void pl(unsigned long long val)
{
    for (int shift=56 ; shift>=0 ; shift-=8)
        {
        int ch = (val >> shift) & 0xff;
        printf("%02x", ch);
        }
}
*/



/**
 * 32/64-bit-isms.  These truncate their arguments to
 * unsigned 32-bit or unsigned 64-bit.  For our math,
 * we only require that unsigned longs be AT LEAST 32 bits,
 * and that unsigned long longs be AT LEAST 64 bits.  Their
 * exact lengths do not matter.
 */
#define TR32(x) ((x) & 0xffffffffL)
#define TR64(x) ((x) & 0xffffffffffffffffLL)


static const char *hexDigits = "0123456789abcdef";

static std::string toHex(const std::vector<unsigned char> &bytes)
{
    std::string str;
    std::vector<unsigned char>::const_iterator iter;
    for (iter = bytes.begin() ; iter != bytes.end() ; iter++)
       {
       unsigned char ch = *iter;
       str.push_back(hexDigits[(ch>>4) & 0x0f]);
       str.push_back(hexDigits[(ch   ) & 0x0f]);
       }
    return str;
}


//########################################################################
//##  D I G E S T
//########################################################################


/**
 *
 */
std::string Digest::finishHex()
{
    std::vector<unsigned char> hash = finish();
    std::string str = toHex(hash);
    return str;
}

/**
 * Convenience method.  This is a simple way of getting a hash
 */
std::vector<unsigned char> Digest::hash(Digest::HashType typ,
                       unsigned char *buf,
                       int len)
{
    std::vector<unsigned char> ret;
    switch (typ)
        {
        case HASH_MD5:
            {
            Md5 digest;
            digest.append(buf, len);
            ret = digest.finish();
            break;
            }
        case HASH_SHA1:
            {
            Sha1 digest;
            digest.append(buf, len);
            ret = digest.finish();
            break;
            }
        case HASH_SHA224:
            {
            Sha224 digest;
            digest.append(buf, len);
            ret = digest.finish();
            break;
            }
        case HASH_SHA256:
            {
            Sha256 digest;
            digest.append(buf, len);
            ret = digest.finish();
            break;
            }
        case HASH_SHA384:
            {
            Sha384 digest;
            digest.append(buf, len);
            ret = digest.finish();
            break;
            }
        case HASH_SHA512:
            {
            Sha512 digest;
            digest.append(buf, len);
            ret = digest.finish();
            break;
            }
        default:
            {
            break;
            }
        }
    return ret;
}


/**
 * Convenience method.  Same as above, but for a std::string
 */
std::vector<unsigned char> Digest::hash(Digest::HashType typ,
                       const std::string &str)
{
    return hash(typ, (unsigned char *)str.c_str(), str.size());
}

/**
 * Convenience method.  Return a hexidecimal string of the hash of the buffer.
 */
std::string Digest::hashHex(Digest::HashType typ,
               unsigned char *buf,
               int len)
{
    std::vector<unsigned char> dig = hash(typ, buf, len);
    return toHex(dig);
}

/**
 * Convenience method.  Return a hexidecimal string of the hash of the
 *  string argument
 */
std::string Digest::hashHex(Digest::HashType typ,
               const std::string &str)
{
    std::vector<unsigned char> dig = hash(typ, str);
    return toHex(dig);
}



//4.1.1 and 4.1.2
#define SHA_ROTL(X,n) ((((X) << (n)) & 0xffffffffL) | (((X) >> (32-(n))) & 0xffffffffL))
#define SHA_Ch(x,y,z)  ((z)^((x)&((y)^(z))))
#define SHA_Maj(x,y,z) (((x)&(y))^((z)&((x)^(y))))


//########################################################################
//##  S H A 1
//########################################################################


/**
 *
 */
void Sha1::reset()
{
    longNr    = 0;
    byteNr    = 0;

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

    clearByteCount();
}


/**
 *
 */
void Sha1::update(unsigned char ch)
{
    incByteCount();

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


void Sha1::transform()
{
    unsigned long *W = inBuf;
    unsigned long *H = hashBuf;

    //for (int t = 0; t < 16 ; t++)
    //    printf("%2d %08lx\n", t, W[t]);

    //see 6.1.2
    for (int t = 16; t < 80 ; t++)
        W[t] = SHA_ROTL((W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]), 1);

    unsigned long a = H[0];
    unsigned long b = H[1];
    unsigned long c = H[2];
    unsigned long d = H[3];
    unsigned long e = H[4];

    unsigned long T;

    int t = 0;
    for ( ; t < 20 ; t++)
        {
        //see 4.1.1 for the boolops on B,C, and D
        T = TR32(SHA_ROTL(a,5) + ((b&c)|((~b)&d)) +  //Ch(b,c,d))
                e + 0x5a827999L + W[t]);
        e = d; d = c; c = SHA_ROTL(b, 30); b = a; a = T;
        //printf("%2d %08lx %08lx %08lx %08lx %08lx\n", t, a, b, c, d, e);
        }
    for ( ; t < 40 ; t++)
        {
        T = TR32(SHA_ROTL(a,5) + (b^c^d) + e + 0x6ed9eba1L + W[t]);
        e = d; d = c; c = SHA_ROTL(b, 30); b = a; a = T;
        //printf("%2d %08lx %08lx %08lx %08lx %08lx\n", t, a, b, c, d, e);
        }
    for ( ; t < 60 ; t++)
        {
        T = TR32(SHA_ROTL(a,5) + ((b&c)^(b&d)^(c&d)) +
                e + 0x8f1bbcdcL + W[t]);
        e = d; d = c; c = SHA_ROTL(b, 30); b = a; a = T;
        //printf("%2d %08lx %08lx %08lx %08lx %08lx\n", t, a, b, c, d, e);
        }
    for ( ; t < 80 ; t++)
        {
        T = TR32(SHA_ROTL(a,5) + (b^c^d) +
                e + 0xca62c1d6L + W[t]);
        e = d; d = c; c = SHA_ROTL(b, 30); b = a; a = T;
        //printf("%2d %08lx %08lx %08lx %08lx %08lx\n", t, a, b, c, d, e);
        }

    H[0] = TR32(H[0] + a);
    H[1] = TR32(H[1] + b);
    H[2] = TR32(H[2] + c);
    H[3] = TR32(H[3] + d);
    H[4] = TR32(H[4] + e);
}




/**
 *
 */
std::vector<unsigned char> Sha1::finish()
{
    //snapshot the bit count now before padding
    getBitCount();

    //Append terminal char
    update(0x80);

    //pad until we have a 56 of 64 bytes, allowing for 8 bytes at the end
    while ((nrBytesLo & 63) != 56)
        update(0);

    //##### Append length in bits
    appendBitCount();

    //copy out answer
    std::vector<unsigned char> res;
    for (int i=0 ; i<5 ; i++)
        {
        res.push_back((unsigned char)((hashBuf[i] >> 24) & 0xff));
        res.push_back((unsigned char)((hashBuf[i] >> 16) & 0xff));
        res.push_back((unsigned char)((hashBuf[i] >>  8) & 0xff));
        res.push_back((unsigned char)((hashBuf[i]      ) & 0xff));
        }

    // Re-initialize the context (also zeroizes contents)
    reset();

    return res;
}




//########################################################################
//##  SHA224
//########################################################################


/**
 * SHA-224 and SHA-512 share the same operations and constants
 */ 

#define    SHA_Rot32(x,s) ((((x) >> s)&0xffffffffL) | (((x) << (32 - s))&0xffffffffL))
#define    SHA_SIGMA0(x)  (SHA_Rot32(x,  2) ^ SHA_Rot32(x, 13) ^ SHA_Rot32(x, 22))
#define    SHA_SIGMA1(x)  (SHA_Rot32(x,  6) ^ SHA_Rot32(x, 11) ^ SHA_Rot32(x, 25))
#define    SHA_sigma0(x)  (SHA_Rot32(x,  7) ^ SHA_Rot32(x, 18) ^ ((x) >> 3))
#define    SHA_sigma1(x)  (SHA_Rot32(x, 17) ^ SHA_Rot32(x, 19) ^ ((x) >> 10))


static unsigned long sha256table[64] =
{
    0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL,
    0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
    0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL,
    0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
    0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
    0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
    0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL,
    0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL,
    0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL,
    0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
    0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL,
    0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
    0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL,
    0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
    0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
    0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};





/**
 *
 */
void Sha224::reset()
{
    longNr   = 0;
    byteNr   = 0;

    // Initialize H with the magic constants (see FIPS180 for constants)
    hashBuf[0] = 0xc1059ed8L;
    hashBuf[1] = 0x367cd507L;
    hashBuf[2] = 0x3070dd17L;
    hashBuf[3] = 0xf70e5939L;
    hashBuf[4] = 0xffc00b31L;
    hashBuf[5] = 0x68581511L;
    hashBuf[6] = 0x64f98fa7L;
    hashBuf[7] = 0xbefa4fa4L;

    for (int i = 0 ; i < 64 ; i++)
        inBuf[i] = 0;

    for (int i = 0 ; i < 4 ; i++)
        inb[i] = 0;

    clearByteCount();
}


/**
 *
 */
void Sha224::update(unsigned char ch)
{
    incByteCount();

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


void Sha224::transform()
{
    unsigned long *W = inBuf;
    unsigned long *H = hashBuf;

    //for (int t = 0; t < 16 ; t++)
    //    printf("%2d %08lx\n", t, W[t]);

    //see 6.2.2
    for (int t = 16; t < 64 ; t++)
        W[t] = TR32(SHA_sigma1(W[t-2]) + W[t-7] + SHA_sigma0(W[t-15]) + W[t-16]);

    unsigned long a = H[0];
    unsigned long b = H[1];
    unsigned long c = H[2];
    unsigned long d = H[3];
    unsigned long e = H[4];
    unsigned long f = H[5];
    unsigned long g = H[6];
    unsigned long h = H[7];

    for (int t = 0 ; t < 64 ; t++)
        {
        //see 4.1.1 for the boolops
        unsigned long T1 = TR32(h + SHA_SIGMA1(e) + SHA_Ch(e,f,g) +
            sha256table[t] + W[t]);
        unsigned long T2 = TR32(SHA_SIGMA0(a) + SHA_Maj(a,b,c));
        h = g; g = f; f = e; e = TR32(d  + T1); d = c; c = b; b = a; a = TR32(T1 + T2);
        //printf("%2d %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n",
        //         t, a, b, c, d, e, f, g, h);
        }

    H[0] = TR32(H[0] + a);
    H[1] = TR32(H[1] + b);
    H[2] = TR32(H[2] + c);
    H[3] = TR32(H[3] + d);
    H[4] = TR32(H[4] + e);
    H[5] = TR32(H[5] + f);
    H[6] = TR32(H[6] + g);
    H[7] = TR32(H[7] + h);
}



/**
 *
 */
std::vector<unsigned char> Sha224::finish()
{
    //save our size before padding
    getBitCount();
    
    // Pad with a binary 1 (0x80)
    update(0x80);
    //append 0's to make a 56-byte buf.
    while ((nrBytesLo & 63) != 56)
        update(0);

    //##### Append length in bits
    appendBitCount();

    // Output hash
    std::vector<unsigned char> ret;
    for (int i = 0 ; i < 7 ; i++)
        {
        ret.push_back((unsigned char)((hashBuf[i] >> 24) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i] >> 16) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i] >>  8) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i]      ) & 0xff));
        }

    // Re-initialize the context (also zeroizes contents)
    reset();

    return ret;

}



//########################################################################
//##  SHA256
//########################################################################


/**
 *
 */
void Sha256::reset()
{
    longNr   = 0;
    byteNr   = 0;

    // Initialize H with the magic constants (see FIPS180 for constants)
    hashBuf[0] = 0x6a09e667L;
    hashBuf[1] = 0xbb67ae85L;
    hashBuf[2] = 0x3c6ef372L;
    hashBuf[3] = 0xa54ff53aL;
    hashBuf[4] = 0x510e527fL;
    hashBuf[5] = 0x9b05688cL;
    hashBuf[6] = 0x1f83d9abL;
    hashBuf[7] = 0x5be0cd19L;

    for (int i = 0 ; i < 64 ; i++)
        inBuf[i] = 0;
    for (int i = 0 ; i < 4 ; i++)
        inb[i] = 0;

    clearByteCount();
}


/**
 *
 */
void Sha256::update(unsigned char ch)
{
    incByteCount();

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




void Sha256::transform()
{
    unsigned long *H = hashBuf;
    unsigned long *W = inBuf;

    //for (int t = 0; t < 16 ; t++)
    //    printf("%2d %08lx\n", t, W[t]);

    //see 6.2.2
    for (int t = 16; t < 64 ; t++)
        W[t] = TR32(SHA_sigma1(W[t-2]) + W[t-7] + SHA_sigma0(W[t-15]) + W[t-16]);

    unsigned long a = H[0];
    unsigned long b = H[1];
    unsigned long c = H[2];
    unsigned long d = H[3];
    unsigned long e = H[4];
    unsigned long f = H[5];
    unsigned long g = H[6];
    unsigned long h = H[7];

    for (int t = 0 ; t < 64 ; t++)
        {
        //see 4.1.1 for the boolops
        unsigned long T1 = TR32(h + SHA_SIGMA1(e) + SHA_Ch(e,f,g) +
            sha256table[t] + W[t]);
        unsigned long T2 = TR32(SHA_SIGMA0(a) + SHA_Maj(a,b,c));
        h = g; g = f; f = e; e = TR32(d  + T1); d = c; c = b; b = a; a = TR32(T1 + T2);
        //printf("%2d %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n",
        //         t, a, b, c, d, e, f, g, h);
        }

    H[0] = TR32(H[0] + a);
    H[1] = TR32(H[1] + b);
    H[2] = TR32(H[2] + c);
    H[3] = TR32(H[3] + d);
    H[4] = TR32(H[4] + e);
    H[5] = TR32(H[5] + f);
    H[6] = TR32(H[6] + g);
    H[7] = TR32(H[7] + h);
}



/**
 *
 */
std::vector<unsigned char> Sha256::finish()
{
    //save our size before padding
    getBitCount();
    
    // Pad with a binary 1 (0x80)
    update(0x80);
    //append 0's to make a 56-byte buf.
    while ((nrBytesLo & 63) != 56)
        update(0);

    //##### Append length in bits
    appendBitCount();

    // Output hash
    std::vector<unsigned char> ret;
    for (int i = 0 ; i < 8 ; i++)
        {
        ret.push_back((unsigned char)((hashBuf[i] >> 24) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i] >> 16) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i] >>  8) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i]      ) & 0xff));
        }

    // Re-initialize the context (also zeroizes contents)
    reset();

    return ret;

}



//########################################################################
//##  SHA384
//########################################################################


/**
 * SHA-384 and SHA-512 share the same operations and constants
 */
  
#undef SHA_SIGMA0
#undef SHA_SIGMA1
#undef SHA_sigma0
#undef SHA_sigma1

#define SHA_Rot64(x,s) (((x) >> s) | ((x) << (64 - s)))
#define SHA_SIGMA0(x)  (SHA_Rot64(x, 28) ^ SHA_Rot64(x, 34) ^ SHA_Rot64(x, 39))
#define SHA_SIGMA1(x)  (SHA_Rot64(x, 14) ^ SHA_Rot64(x, 18) ^ SHA_Rot64(x, 41))
#define SHA_sigma0(x)  (SHA_Rot64(x,  1) ^ SHA_Rot64(x,  8) ^ ((x) >> 7))
#define SHA_sigma1(x)  (SHA_Rot64(x, 19) ^ SHA_Rot64(x, 61) ^ ((x) >> 6))


static unsigned long long sha512table[80] =
{
    0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL,
    0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
    0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL,
    0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
    0xd807aa98a3030242ULL, 0x12835b0145706fbeULL,
    0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
    0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL,
    0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
    0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL,
    0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
    0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL,
    0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
    0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL,
    0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
    0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL,
    0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
    0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL,
    0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
    0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL,
    0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
    0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL,
    0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
    0xd192e819d6ef5218ULL, 0xd69906245565a910ULL,
    0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
    0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL,
    0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
    0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL,
    0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
    0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL,
    0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
    0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL,
    0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
    0xca273eceea26619cULL, 0xd186b8c721c0c207ULL,
    0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
    0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL,
    0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
    0x28db77f523047d84ULL, 0x32caab7b40c72493ULL,
    0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
    0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL,
    0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};




/**
 *
 */
void Sha384::reset()
{
    longNr   = 0;
    byteNr   = 0;

    // SHA-384 differs from SHA-512 by these constants
    hashBuf[0] = 0xcbbb9d5dc1059ed8ULL;
    hashBuf[1] = 0x629a292a367cd507ULL;
    hashBuf[2] = 0x9159015a3070dd17ULL;
    hashBuf[3] = 0x152fecd8f70e5939ULL;
    hashBuf[4] = 0x67332667ffc00b31ULL;
    hashBuf[5] = 0x8eb44a8768581511ULL;
    hashBuf[6] = 0xdb0c2e0d64f98fa7ULL;
    hashBuf[7] = 0x47b5481dbefa4fa4ULL;

    for (int i = 0 ; i < 80 ; i++)
        inBuf[i] = 0;
    for (int i = 0 ; i < 8 ; i++)
        inb[i] = 0;

    clearByteCount();
}


/**
 *  Note that this version of update() handles 64-bit inBuf
 *  values. 
 */
void Sha384::update(unsigned char ch)
{
    incByteCount();

    inb[byteNr++] = (unsigned long long)ch;
    if (byteNr >= 8)
        {
        inBuf[longNr++] = inb[0] << 56 | inb[1] << 48 |
                          inb[2] << 40 | inb[3] << 32 |
                          inb[4] << 24 | inb[5] << 16 |
                          inb[6] <<  8 | inb[7];
        byteNr = 0;
        }
    if (longNr >= 16)
        {
        transform();
        longNr = 0;
        }
}




void Sha384::transform()
{
    unsigned long long *H = hashBuf;
    unsigned long long *W = inBuf;

    /*
    for (int t = 0; t < 16 ; t++)
        {
        printf("%2d ", t);
        pl(W[t]);
        printf("\n");
        }
    */

    //see 6.2.2
    for (int t = 16; t < 80 ; t++)
        W[t] = TR64(SHA_sigma1(W[t-2]) + W[t-7] + SHA_sigma0(W[t-15]) + W[t-16]);

    unsigned long long a = H[0];
    unsigned long long b = H[1];
    unsigned long long c = H[2];
    unsigned long long d = H[3];
    unsigned long long e = H[4];
    unsigned long long f = H[5];
    unsigned long long g = H[6];
    unsigned long long h = H[7];

    for (int t = 0 ; t < 80 ; t++)
        {
        //see 4.1.1 for the boolops
        unsigned long long T1 = TR64(h + SHA_SIGMA1(e) + SHA_Ch(e,f,g) +
            sha512table[t] + W[t]);
        unsigned long long T2 = TR64(SHA_SIGMA0(a) + SHA_Maj(a,b,c));
        h = g; g = f; f = e; e = TR64(d  + T1); d = c; c = b; b = a; a = TR64(T1 + T2);
        }

    H[0] = TR64(H[0] + a);  
    H[1] = TR64(H[1] + b);  
    H[2] = TR64(H[2] + c);  
    H[3] = TR64(H[3] + d);  
    H[4] = TR64(H[4] + e);  
    H[5] = TR64(H[5] + f);  
    H[6] = TR64(H[6] + g);  
    H[7] = TR64(H[7] + h);  
}



/**
 *
 */
std::vector<unsigned char> Sha384::finish()
{
    //save our size before padding
    getBitCount();
    
    // Pad with a binary 1 (0x80)
    update((unsigned char)0x80);
    //append 0's to make a 112-byte buf.
    //we will loop around once if already over 112
    while ((nrBytesLo & 127) != 112)
        update(0);
        
    //append 128-bit size
    //64 upper bits
    for (int i = 0 ; i < 8 ; i++)
        update((unsigned char)0x00);
    //64 lower bits
    //##### Append length in bits
    appendBitCount();

    // Output hash
    //for SHA-384, we use the left-most 6 64-bit words
    std::vector<unsigned char> ret;
    for (int i = 0 ; i < 6 ; i++)
        {
        ret.push_back((unsigned char)((hashBuf[i] >> 56) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i] >> 48) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i] >> 40) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i] >> 32) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i] >> 24) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i] >> 16) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i] >>  8) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i]      ) & 0xff));
        }

    // Re-initialize the context (also zeroizes contents)
    reset();

    return ret;

}


//########################################################################
//##  SHA512
//########################################################################





/**
 *
 */
void Sha512::reset()
{
    longNr   = 0;
    byteNr   = 0;

    // Initialize H with the magic constants (see FIPS180 for constants)
    hashBuf[0] = 0x6a09e667f3bcc908ULL;
    hashBuf[1] = 0xbb67ae8584caa73bULL;
    hashBuf[2] = 0x3c6ef372fe94f82bULL;
    hashBuf[3] = 0xa54ff53a5f1d36f1ULL;
    hashBuf[4] = 0x510e527fade682d1ULL;
    hashBuf[5] = 0x9b05688c2b3e6c1fULL;
    hashBuf[6] = 0x1f83d9abfb41bd6bULL;
    hashBuf[7] = 0x5be0cd19137e2179ULL;

    for (int i = 0 ; i < 80 ; i++)
        inBuf[i] = 0;
    for (int i = 0 ; i <  8 ; i++)
        inb[i] = 0;

    clearByteCount();
}


/**
 *  Note that this version of update() handles 64-bit inBuf
 *  values. 
 */
void Sha512::update(unsigned char ch)
{
    incByteCount();

    inb[byteNr++] = (unsigned long long)ch;
    if (byteNr >= 8)
        {
        inBuf[longNr++] = inb[0] << 56 | inb[1] << 48 |
                          inb[2] << 40 | inb[3] << 32 |
                          inb[4] << 24 | inb[5] << 16 |
                          inb[6] <<  8 | inb[7];
        byteNr = 0;
        }
    if (longNr >= 16)
        {
        transform();
        longNr = 0;
        }
}




void Sha512::transform()
{
    unsigned long long *W = inBuf;
    unsigned long long *H = hashBuf;

    /*
    for (int t = 0; t < 16 ; t++)
        {
        printf("%2d ", t);
        pl(W[t]);
        printf("\n");
        }
    */

    //see 6.2.2
    for (int t = 16; t < 80 ; t++)
        W[t] = TR64(SHA_sigma1(W[t-2]) + W[t-7] + SHA_sigma0(W[t-15]) + W[t-16]);

    unsigned long long a = H[0];
    unsigned long long b = H[1];
    unsigned long long c = H[2];
    unsigned long long d = H[3];
    unsigned long long e = H[4];
    unsigned long long f = H[5];
    unsigned long long g = H[6];
    unsigned long long h = H[7];

    for (int t = 0 ; t < 80 ; t++)
        {
        //see 4.1.1 for the boolops
        unsigned long long T1 = TR64(h + SHA_SIGMA1(e) + SHA_Ch(e,f,g) +
            sha512table[t] + W[t]);
        unsigned long long T2 = TR64(SHA_SIGMA0(a) + SHA_Maj(a,b,c));
        h = g; g = f; f = e; e = TR64(d  + T1); d = c; c = b; b = a; a = TR64(T1 + T2);
        }

    H[0] = TR64(H[0] + a);  
    H[1] = TR64(H[1] + b);  
    H[2] = TR64(H[2] + c);  
    H[3] = TR64(H[3] + d);  
    H[4] = TR64(H[4] + e);  
    H[5] = TR64(H[5] + f);  
    H[6] = TR64(H[6] + g);  
    H[7] = TR64(H[7] + h);  
}



/**
 *
 */
std::vector<unsigned char> Sha512::finish()
{
    //save our size before padding
    getBitCount();
    
    // Pad with a binary 1 (0x80)
    update(0x80);
    //append 0's to make a 112-byte buf.
    //we will loop around once if already over 112
    while ((nrBytesLo & 127) != 112)
        update(0);
        
    //append 128-bit size
    //64 upper bits
    for (int i = 0 ; i < 8 ; i++)
        update((unsigned char)0x00);
    //64 lower bits
    //##### Append length in bits
    appendBitCount();

    // Output hash
    std::vector<unsigned char> ret;
    for (int i = 0 ; i < 8 ; i++)
        {
        ret.push_back((unsigned char)((hashBuf[i] >> 56) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i] >> 48) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i] >> 40) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i] >> 32) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i] >> 24) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i] >> 16) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i] >>  8) & 0xff));
        ret.push_back((unsigned char)((hashBuf[i]      ) & 0xff));
        }

    // Re-initialize the context (also zeroizes contents)
    reset();

    return ret;

}



//########################################################################
//##  M D 5
//########################################################################

/**
 *
 */
void Md5::reset()
{
    hashBuf[0]  = 0x67452301;
    hashBuf[1]  = 0xefcdab89;
    hashBuf[2]  = 0x98badcfe;
    hashBuf[3]  = 0x10325476;

    for (int i=0 ; i<16 ; i++)
        inBuf[i] = 0;
    for (int i=0 ; i<4 ; i++)
        inb[i] = 0;

    clearByteCount();

    byteNr    = 0;
    longNr    = 0;
}


/**
 *
 */
void Md5::update(unsigned char ch)
{
    incByteCount();

    //pack 64 bytes into 16 longs
    inb[byteNr++] = (unsigned long)ch;
    if (byteNr >= 4)
        {
        //note the little-endianness
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



//#  The four core functions - F1 is optimized somewhat

// #define F1(x, y, z) (x & y | ~x & z)
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

// ## This is the central step in the MD5 algorithm.
#define MD5STEP(f, w, x, y, z, data, s) \
    ( w = TR32(w + (f(x, y, z) + data)), w = w<<s | w>>(32-s), w = TR32(w + x) )

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

    hashBuf[0] = TR32(hashBuf[0] + a);
    hashBuf[1] = TR32(hashBuf[1] + b);
    hashBuf[2] = TR32(hashBuf[2] + c);
    hashBuf[3] = TR32(hashBuf[3] + d);
}


/**
 *
 */
std::vector<unsigned char> Md5::finish()
{
    //snapshot the bit count now before padding
    getBitCount();

    //Append terminal char
    update(0x80);

    //pad until we have a 56 of 64 bytes, allowing for 8 bytes at the end
    while (longNr != 14)
        update(0);

    //##### Append length in bits
    // Don't use appendBitCount(), since md5 is little-endian
    update((unsigned char)((nrBitsLo    ) & 0xff));
    update((unsigned char)((nrBitsLo>> 8) & 0xff));
    update((unsigned char)((nrBitsLo>>16) & 0xff));
    update((unsigned char)((nrBitsLo>>24) & 0xff));
    update((unsigned char)((nrBitsHi    ) & 0xff));
    update((unsigned char)((nrBitsHi>> 8) & 0xff));
    update((unsigned char)((nrBitsHi>>16) & 0xff));
    update((unsigned char)((nrBitsHi>>24) & 0xff));

    //copy out answer
    std::vector<unsigned char> res;
    for (int i=0 ; i<4 ; i++)
        {
        //note the little-endianness
        res.push_back((unsigned char)((hashBuf[i]      ) & 0xff));
        res.push_back((unsigned char)((hashBuf[i] >>  8) & 0xff));
        res.push_back((unsigned char)((hashBuf[i] >> 16) & 0xff));
        res.push_back((unsigned char)((hashBuf[i] >> 24) & 0xff));
        }

    reset();  // Security!  ;-)

    return res;
}






//########################################################################
//## T E S T S
//########################################################################

/**
 * Compile this file alone with -DDIGEST_TEST to run the
 * tests below:
 * > gcc -DDIGEST_TEST digest.cpp -o testdigest
 * > testdigest   
 * 
 * If you add any new algorithms to this suite, then it is highly
 * recommended that you add it to these tests and run it.   
 */
 
#ifdef DIGEST_TEST


typedef struct
{
    const char *msg;
    const char *val;
} TestPair;

static TestPair md5tests[] =
{
  {
  "",
  "d41d8cd98f00b204e9800998ecf8427e"
  },
  {
  "a",
  "0cc175b9c0f1b6a831c399e269772661"
  },
  {
  "abc",
  "900150983cd24fb0d6963f7d28e17f72"
  },
  {
  "message digest",
  "f96b697d7cb7938d525a2f31aaf161d0"
  },
  {
  "abcdefghijklmnopqrstuvwxyz",
  "c3fcd3d76192e4007dfb496cca67e13b"
  },
  {
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
  "d174ab98d277d9f5a5611c2c9f419d9f"
  },
  {
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
  "57edf4a22be3c955ac49da2e2107b67a"
  },
  {
  NULL,
  NULL
  }
};



static TestPair sha1tests[] =
{
  {
  "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
  "84983e441c3bd26ebaae4aa1f95129e5e54670f1"
  },
  {
  NULL,
  NULL
  }
};

static TestPair sha224tests[] =
{
  {
  "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
  "75388b16512776cc5dba5da1fd890150b0c6455cb4f58b1952522525"
  },
  {
  NULL,
  NULL
  }
};

static TestPair sha256tests[] =
{
  {
  "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
  "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"
  },
  {
  NULL,
  NULL
  }
};

static TestPair sha384tests[] =
{
  {
  "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
       "hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
  "09330c33f71147e83d192fc782cd1b4753111b173b3b05d22fa08086e3b0f712"
       "fcc7c71a557e2db966c3e9fa91746039"
  },
  {
  NULL,
  NULL
  }
};

static TestPair sha512tests[] =
{
  {
  "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
       "hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
  "8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018"
       "501d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909"
  },
  {
  NULL,
  NULL
  }
};


bool hashTests(Digest &digest, TestPair *tp)
{
    for (TestPair *pair = tp ; pair->msg ; pair++)
        {
        digest.reset();
        std::string msg = pair->msg;
        std::string val = pair->val;
        digest.append(msg);
        std::string res = digest.finishHex();
        printf("### Msg '%s':\n   hash '%s'\n   exp  '%s'\n",
            msg.c_str(), res.c_str(), val.c_str());
        if (res != val)
            {
            printf("ERROR: Hash mismatch\n");
            return false;
            }
        }
    return true;
}


bool millionATest(Digest &digest, const std::string &exp)
{
    digest.reset();
    for (int i=0 ; i<1000000 ; i++)
        digest.append('a');
    std::string res = digest.finishHex();
    printf("\nHash of 1,000,000 'a'\n   calc %s\n   exp  %s\n",
         res.c_str(), exp.c_str());
    if (res != exp)
        {
        printf("ERROR: Mismatch.\n");
        return false;
        }
    return true;
}

static bool doTests()
{
    printf("##########################################\n");
    printf("## MD5\n");
    printf("##########################################\n");
    Md5 md5;
    if (!hashTests(md5, md5tests))
        return false;
    if (!millionATest(md5, "7707d6ae4e027c70eea2a935c2296f21"))
        return false;
    printf("\n\n\n");
    printf("##########################################\n");
    printf("## SHA1\n");
    printf("##########################################\n");
    Sha1 sha1;
    if (!hashTests(sha1, sha1tests))
        return false;
    if (!millionATest(sha1, "34aa973cd4c4daa4f61eeb2bdbad27316534016f"))
        return false;
    printf("\n\n\n");
    printf("##########################################\n");
    printf("## SHA224\n");
    printf("##########################################\n");
    Sha224 sha224;
    if (!hashTests(sha224, sha224tests))
        return false;
    if (!millionATest(sha224,
         "20794655980c91d8bbb4c1ea97618a4bf03f42581948b2ee4ee7ad67"))
        return false;
    printf("\n\n\n");
    printf("##########################################\n");
    printf("## SHA256\n");
    printf("##########################################\n");
    Sha256 sha256;
    if (!hashTests(sha256, sha256tests))
        return false;
    if (!millionATest(sha256,
         "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0"))
        return false;
    printf("\n\n\n");
    printf("##########################################\n");
    printf("## SHA384\n");
    printf("##########################################\n");
    Sha384 sha384;
    if (!hashTests(sha384, sha384tests))
        return false;
    /**/
    if (!millionATest(sha384,
         "9d0e1809716474cb086e834e310a4a1ced149e9c00f248527972cec5704c2a5b"
                "07b8b3dc38ecc4ebae97ddd87f3d8985"))
        return false;
    /**/
    printf("\n\n\n");
    printf("##########################################\n");
    printf("## SHA512\n");
    printf("##########################################\n");
    Sha512 sha512;
    if (!hashTests(sha512, sha512tests))
        return false;
    if (!millionATest(sha512,
         "e718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973eb"
             "de0ff244877ea60a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b"))
        return false;
    return true;
}


int main(int argc, char **argv)
{
    doTests();
    printf("####### done ########\n");
    return 0;
}


#endif /* DIGEST_TEST */

//########################################################################
//## E N D    O F    F I L E
//########################################################################
