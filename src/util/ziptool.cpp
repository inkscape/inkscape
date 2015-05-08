/*
 * This is intended to be a standalone, reduced capability
 * implementation of Gzip and Zip functionality.  Its
 * targeted use case is for archiving and retrieving single files
 * which use these encoding types.  Being memory based and
 * non-optimized, it is not useful in cases where very large
 * archives are needed or where high performance is desired.
 * However, it should hopefully work very well for smaller,
 * one-at-a-time tasks.  What you get in return is the ability
 * to drop these files into your project and remove the dependencies
 * on ZLib and Info-Zip.  Enjoy.
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2006-2007 Bob Jamison
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
#include <time.h>

#include <string>

#include "ziptool.h"






//########################################################################
//#  A D L E R  3 2
//########################################################################

/**
 * Constructor
 */
Adler32::Adler32()
{
    reset();
}

/**
 * Destructor
 */
Adler32::~Adler32()
{
}

/**
 * Reset Adler-32 checksum to initial value.
 */
void Adler32::reset()
{
    value = 1;
}

// ADLER32_BASE is the largest prime number smaller than 65536
#define ADLER32_BASE 65521

void Adler32::update(unsigned char b)
{
    unsigned long s1 = value & 0xffff;
    unsigned long s2 = (value >> 16) & 0xffff;
    s1 += b & 0xff;
    s2 += s1;
    value = ((s2 % ADLER32_BASE) << 16) | (s1 % ADLER32_BASE);
}

void Adler32::update(char *str)
{
    if (str)
        while (*str)
            update((unsigned char)*str++);
}


/**
 * Returns current checksum value.
 */
unsigned long Adler32::getValue()
{
    return value & 0xffffffffL;
}



//########################################################################
//#  C R C  3 2
//########################################################################

/**
 * Constructor
 */
Crc32::Crc32()
{
    reset();
}

/**
 * Destructor
 */
Crc32::~Crc32()
{
}

static bool crc_table_ready = false;
static unsigned long crc_table[256];

/**
 * make the table for a fast CRC.
 */
static void makeCrcTable()
{
    if (crc_table_ready)
        return;
    for (int n = 0; n < 256; n++)
        {
        unsigned long c = n;
        for (int k = 8;  --k >= 0; )
            {
            if ((c & 1) != 0)
                c = 0xedb88320 ^ (c >> 1);
            else
                c >>= 1;
            }
        crc_table[n] = c;
        }
    crc_table_ready = true;
}


/**
 * Reset CRC-32 checksum to initial value.
 */
void Crc32::reset()
{
    value = 0;
    makeCrcTable();
}

void Crc32::update(unsigned char b)
{
    unsigned long c = ~value;

    c &= 0xffffffff;
    c = crc_table[(c ^ b) & 0xff] ^ (c >> 8);
    value = ~c;
}


void Crc32::update(char *str)
{
    if (str)
        while (*str)
            update((unsigned char)*str++);
}

void Crc32::update(const std::vector<unsigned char> &buf)
{
    std::vector<unsigned char>::const_iterator iter;
    for (iter=buf.begin() ; iter!=buf.end() ; ++iter)
        {
        unsigned char ch = *iter;
        update(ch);
        }
}


/**
 * Returns current checksum value.
 */
unsigned long Crc32::getValue()
{
    return value & 0xffffffffL;
}

//########################################################################
//#  I N F L A T E R
//########################################################################


/**
 *
 */
typedef struct
{
    int *count;  // number of symbols of each length
    int *symbol; // canonically ordered symbols
} Huffman;

/**
 *
 */
class Inflater
{
public:

    Inflater();

    virtual ~Inflater();

    static const int MAXBITS   =  15; // max bits in a code
    static const int MAXLCODES = 286; // max number of literal/length codes
    static const int MAXDCODES =  30; // max number of distance codes
    static const int MAXCODES  = 316; // max codes lengths to read
    static const int FIXLCODES = 288; // number of fixed literal/length codes

    /**
     *
     */
    bool inflate(std::vector<unsigned char> &destination,
                 std::vector<unsigned char> &source);

private:

    /**
     *
     */
    void error(char const *fmt, ...)
    #ifdef G_GNUC_PRINTF
    G_GNUC_PRINTF(2, 3)
    #endif
    ;

    /**
     *
     */
    void trace(char const *fmt, ...)
    #ifdef G_GNUC_PRINTF
    G_GNUC_PRINTF(2, 3)
    #endif
    ;

    /**
     *
     */
    void dump();

    /**
     *
     */
    int buildHuffman(Huffman *h, int *length, int n);

    /**
     *
     */
    bool getBits(int need, int *oval);

    /**
     *
     */
    int doDecode(Huffman *h);

    /**
     *
     */
    bool doCodes(Huffman *lencode, Huffman *distcode);

    /**
     *
     */
    bool doStored();

    /**
     *
     */
    bool doFixed();

    /**
     *
     */
    bool doDynamic();


    std::vector<unsigned char>dest;

    std::vector<unsigned char>src;
    unsigned long srcPos;  //current read position
    int bitBuf;
    int bitCnt;

};


/**
 *
 */
Inflater::Inflater() :
    dest(),
    src(),
    srcPos(0),
    bitBuf(0),
    bitCnt(0)
{
}

/**
 *
 */
Inflater::~Inflater()
{
}

/**
 *
 */
void Inflater::error(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "Inflater error:");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}

/**
 *
 */
void Inflater::trace(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "Inflater:");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}


/**
 *
 */
void Inflater::dump()
{
    for (unsigned int i=0 ; i<dest.size() ; i++)
        {
        fputc(dest[i], stdout);
        }
}

/**
 *
 */
int Inflater::buildHuffman(Huffman *h, int *length, int n)
{
    // count number of codes of each length
    for (int len = 0; len <= MAXBITS; len++)
        h->count[len] = 0;
    for (int symbol = 0; symbol < n; symbol++)
        (h->count[length[symbol]])++;   // assumes lengths are within bounds
    if (h->count[0] == n)               // no codes!
        {
        error("huffman tree will result in failed decode");
        return -1;
        }

    // check for an over-subscribed or incomplete set of lengths
    int left = 1;                // number of possible codes left of current length
    for (int len = 1; len <= MAXBITS; len++)
        {
        left <<= 1;                     // one more bit, double codes left
        left -= h->count[len];          // deduct count from possible codes
        if (left < 0)
            {
            error("huffman over subscribed");
            return -1;
            }
        }

    // generate offsets into symbol table for each length for sorting
    int offs[MAXBITS+1]; //offsets in symbol table for each length
    offs[1] = 0;
    for (int len = 1; len < MAXBITS; len++)
        offs[len + 1] = offs[len] + h->count[len];

    /*
     * put symbols in table sorted by length, by symbol order within each
     * length
     */
    for (int symbol = 0; symbol < n; symbol++)
        if (length[symbol] != 0)
            h->symbol[offs[length[symbol]]++] = symbol;

    // return zero for complete set, positive for incomplete set
    return left;
}


/**
 *
 */
bool Inflater::getBits(int requiredBits, int *oval)
{
    long val = bitBuf;

    //add more bytes if needed
    while (bitCnt < requiredBits)
        {
        if (srcPos >= src.size())
            {
            error("premature end of input");
            return false;
            }
        val |= ((long)(src[srcPos++])) << bitCnt;
        bitCnt += 8;
        }

    //update the buffer and return the data
    bitBuf =  (int)(val >> requiredBits);
    bitCnt -= requiredBits;
    *oval = (int)(val & ((1L << requiredBits) - 1));

    return true;
}


/**
 *
 */
int Inflater::doDecode(Huffman *h)
{
    int bitTmp  = bitBuf;
    int left    = bitCnt;
    int code    = 0;
    int first   = 0;
    int index   = 0;
    int len     = 1;
    int *next = h->count + 1;
    while (true)
        {
        while (left--)
            {
            code   |=  bitTmp & 1;
            bitTmp >>= 1;
            int count  =   *next++;
            if (code < first + count)
                { /* if length len, return symbol */
                bitBuf = bitTmp;
                bitCnt = (bitCnt - len) & 7;
                return h->symbol[index + (code - first)];
                }
            index +=  count;
            first +=  count;
            first <<= 1;
            code  <<= 1;
            len++;
            }
        left = (MAXBITS+1) - len;
        if (left == 0)
            break;
        if (srcPos >= src.size())
            {
            error("premature end of input");
            dump();
            return -1;
            }
        bitTmp = src[srcPos++];
        if (left > 8)
            left = 8;
        }

    error("no end of block found");
    return -1;
}

/**
 *
 */
bool Inflater::doCodes(Huffman *lencode, Huffman *distcode)
{
    static const int lens[29] = { // Size base for length codes 257..285
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    static const int lext[29] = { // Extra bits for length codes 257..285
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    static const int dists[30] = { // Offset base for distance codes 0..29
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
    static const int dext[30] = { // Extra bits for distance codes 0..29
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};

    //decode literals and length/distance pairs
    while (true)
        {
        int symbol = doDecode(lencode);
        if (symbol == 256)
            break;
        if (symbol < 0)
            {
            return false;
            }
        if (symbol < 256) //literal
            {
            dest.push_back(symbol);
            }
        else if (symbol > 256)//length
            {
            symbol -= 257;
            if (symbol >= 29)
                {
                error("invalid fixed code");
                return false;
                }
            int ret;
            if (!getBits(lext[symbol], &ret))
                return false;
            int len = lens[symbol] + ret;

            symbol = doDecode(distcode);//distance
            if (symbol < 0)
                {
                return false;
                }

            if (!getBits(dext[symbol], &ret))
                return false;
            unsigned int dist = dists[symbol] + ret;
            if (dist > dest.size())
                {
                error("distance too far back %d/%d", dist, dest.size());
                dump();
                //printf("pos:%d\n", srcPos);
                return false;
                }

            // copy length bytes from distance bytes back
            //dest.push_back('{');
            while (len--)
                {
                dest.push_back(dest[dest.size() - dist]);
                }
            //dest.push_back('}');

            }
        }

    return true;
}

/**
 */
bool Inflater::doStored()
{
    //trace("### stored ###");

    // clear bits from current byte
    bitBuf = 0;
    bitCnt = 0;

    // length
    if (srcPos + 4 > src.size())
        {
        error("not enough input");
        return false;
        }

    int len = src[srcPos++];
    len |= src[srcPos++] << 8;
    //trace("### len:%d", len);
    // check complement
    if (src[srcPos++] != (~len & 0xff) ||
        src[srcPos++] != ((~len >> 8) & 0xff))
        {
        error("twos complement for storage size do not match");
        return false;
        }

    // copy data
    if (srcPos + len > src.size())
        {
        error("Not enough input for stored block");
        return false;
        }
    while (len--)
        dest.push_back(src[srcPos++]);

    return true;
}

/**
 */
bool Inflater::doFixed()
{
    //trace("### fixed ###");

    static bool firstTime = true;
    static int lencnt[MAXBITS+1], lensym[FIXLCODES];
    static int distcnt[MAXBITS+1], distsym[MAXDCODES];
    static Huffman lencode = {lencnt, lensym};
    static Huffman distcode = {distcnt, distsym};

    if (firstTime)
        {
        firstTime = false;

        int lengths[FIXLCODES];

        // literal/length table
        int symbol = 0;
        for ( ; symbol < 144; symbol++)
            lengths[symbol] = 8;
        for ( ; symbol < 256; symbol++)
            lengths[symbol] = 9;
        for ( ; symbol < 280; symbol++)
            lengths[symbol] = 7;
        for ( ; symbol < FIXLCODES; symbol++)
            lengths[symbol] = 8;
        buildHuffman(&lencode, lengths, FIXLCODES);

        // distance table
        for (int symbol = 0; symbol < MAXDCODES; symbol++)
            lengths[symbol] = 5;
        buildHuffman(&distcode, lengths, MAXDCODES);
        }

    // decode data until end-of-block code
    bool ret = doCodes(&lencode, &distcode);
    return ret;
}

/**
 */
bool Inflater::doDynamic()
{
    //trace("### dynamic ###");
    int lengths[MAXCODES];                      // descriptor code lengths
    int lencnt[MAXBITS+1], lensym[MAXLCODES];   // lencode memory
    int distcnt[MAXBITS+1], distsym[MAXDCODES]; // distcode memory
    Huffman lencode  = {lencnt, lensym};          // length code
    Huffman distcode = {distcnt, distsym};        // distance code
    static const int order[19] =                // permutation of code length codes
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    // get number of lengths in each table, check lengths
    int ret;
    if (!getBits(5, &ret))
        return false;
    int nlen  = ret + 257;
    if (!getBits(5, &ret))
        return false;
    int ndist = ret + 1;
    if (!getBits(4, &ret))
        return false;
    int ncode = ret + 4;
    if (nlen > MAXLCODES || ndist > MAXDCODES)
        {
        error("Bad codes");
        return false;
        }

    // get code length code lengths
    int index = 0;
    for ( ; index < ncode; index++)
        {
        if (!getBits(3, &ret))
            return false;
        lengths[order[index]] = ret;
        }
    for ( ; index < 19; index++)
        lengths[order[index]] = 0;

    // build huffman table for code lengths codes
    if (buildHuffman(&lencode, lengths, 19) != 0)
        return false;

    // read length/literal and distance code length tables
    index = 0;
    while (index < nlen + ndist)
        {
        int symbol = doDecode(&lencode);
        if (symbol < 16)                // length in 0..15
            lengths[index++] = symbol;
        else
            {                          // repeat instruction
            int len = 0;               // assume repeating zeros
            if (symbol == 16)
                {         // repeat last length 3..6 times
                if (index == 0)
                    {
                    error("no last length");
                    return false;
                    }
                len = lengths[index - 1];// last length
                if (!getBits(2, &ret))
                    return false;
                symbol = 3 + ret;
                }
            else if (symbol == 17)      // repeat zero 3..10 times
                {
                if (!getBits(3, &ret))
                    return false;
                symbol = 3 + ret;
                }
            else                        // == 18, repeat zero 11..138 times
                {
                if (!getBits(7, &ret))
                    return false;
                symbol = 11 + ret;
                }
            if (index + symbol > nlen + ndist)
                {
                error("too many lengths");
                return false;
                }
            while (symbol--)            // repeat last or zero symbol times
                lengths[index++] = len;
            }
        }

    // build huffman table for literal/length codes
    int err = buildHuffman(&lencode, lengths, nlen);
    if (err < 0 || (err > 0 && nlen - lencode.count[0] != 1))
        {
        error("incomplete length codes");
        //return false;
        }
    // build huffman table for distance codes
    err = buildHuffman(&distcode, lengths + nlen, ndist);
    if (err < 0 || (err > 0 && nlen - lencode.count[0] != 1))
        {
        error("incomplete dist codes");
        return false;
        }

    // decode data until end-of-block code
    bool retn = doCodes(&lencode, &distcode);
    return retn;
}

/**
 */
bool Inflater::inflate(std::vector<unsigned char> &destination,
                       std::vector<unsigned char> &source)
{
    dest.clear();
    src = source;
    srcPos = 0;
    bitBuf = 0;
    bitCnt = 0;

    while (true)
        {
        int last; // one if last block
        if (!getBits(1, &last))
            return false;
        int type; // block type 0..3
        if (!getBits(2, &type))
            return false;
        switch (type)
            {
            case 0:
                if (!doStored())
                    return false;
                break;
            case 1:
                if (!doFixed())
                    return false;
                break;
            case 2:
                if (!doDynamic())
                    return false;
                break;
            default:
                error("Unknown block type %d", type);
                return false;
            }
        if (last)
            break;
        }

    destination = dest;

    return true;
}






//########################################################################
//#  D E F L A T E R
//########################################################################


#define DEFLATER_BUF_SIZE 32768
class Deflater
{
public:

    /**
     *
     */
    Deflater();

    /**
     *
     */
    virtual ~Deflater();

    /**
     *
     */
    virtual void reset();

    /**
     *
     */
    virtual bool update(int ch);

    /**
     *
     */
    virtual bool finish();

    /**
     *
     */
    virtual std::vector<unsigned char> &getCompressed();

    /**
     *
     */
    bool deflate(std::vector<unsigned char> &dest,
                 const std::vector<unsigned char> &src);

    void encodeDistStatic(unsigned int len, unsigned int dist);

private:

    //debug messages
    void error(char const *fmt, ...)
    #ifdef G_GNUC_PRINTF
    G_GNUC_PRINTF(2, 3)
    #endif
    ;

    void trace(char const *fmt, ...)
    #ifdef G_GNUC_PRINTF
    G_GNUC_PRINTF(2, 3)
    #endif
    ;

    bool compressWindow();

    bool compress();

    std::vector<unsigned char> compressed;

    std::vector<unsigned char> uncompressed;

    std::vector<unsigned char> window;

    unsigned int windowPos;

    //#### Output
    unsigned int outputBitBuf;
    unsigned int outputNrBits;

    void put(int ch);

    void putWord(int ch);

    void putFlush();

    void putBits(unsigned int ch, unsigned int bitsWanted);

    void putBitsR(unsigned int ch, unsigned int bitsWanted);

    //#### Huffman Encode
    void encodeLiteralStatic(unsigned int ch);

    unsigned char windowBuf[DEFLATER_BUF_SIZE];
    //assume 32-bit ints
    unsigned int windowHashBuf[DEFLATER_BUF_SIZE];
};


//########################################################################
//# A P I
//########################################################################


/**
 *
 */
Deflater::Deflater()
{
    reset();
}

/**
 *
 */
Deflater::~Deflater()
{

}

/**
 *
 */
void Deflater::reset()
{
    compressed.clear();
    uncompressed.clear();
    window.clear();
	windowPos = 0;
    outputBitBuf = 0;
    outputNrBits = 0;
    for (int k=0; k<DEFLATER_BUF_SIZE; k++)
    {
        windowBuf[k]=0;
        windowHashBuf[k]=0;
    }
}

/**
 *
 */
bool Deflater::update(int ch)
{
    uncompressed.push_back((unsigned char)(ch & 0xff));
    return true;
}

/**
 *
 */
bool Deflater::finish()
{
    return compress();
}

/**
 *
 */
std::vector<unsigned char> &Deflater::getCompressed()
{
    return compressed;
}


/**
 *
 */
bool Deflater::deflate(std::vector<unsigned char> &dest,
                       const std::vector<unsigned char> &src)
{
    reset();
    uncompressed = src;
    if (!compress())
        return false;
    dest = compressed;
    return true;
}







//########################################################################
//# W O R K I N G    C O D E
//########################################################################


//#############################
//#  M E S S A G E S
//#############################

/**
 *  Print error messages
 */
void Deflater::error(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "Deflater error:");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}

/**
 *  Print trace messages
 */
void Deflater::trace(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "Deflater:");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}




//#############################
//#  O U T P U T
//#############################

/**
 *
 */
void Deflater::put(int ch)
{
    compressed.push_back(ch);
    outputBitBuf = 0;
    outputNrBits = 0;
}

/**
 *
 */
void Deflater::putWord(int ch)
{
    int lo = (ch   ) & 0xff;
    int hi = (ch>>8) & 0xff;
    put(lo);
    put(hi);
}

/**
 *
 */
void Deflater::putFlush()
{
    if (outputNrBits > 0)
        {
        put(outputBitBuf & 0xff);
        }
    outputBitBuf = 0;
    outputNrBits = 0;
}

/**
 *
 */
void Deflater::putBits(unsigned int ch, unsigned int bitsWanted)
{
    //trace("n:%4u, %d\n", ch, bitsWanted);

    while (bitsWanted--)
        {
        //add bits to position 7.  shift right
        outputBitBuf = (outputBitBuf>>1) + (ch<<7 & 0x80);
        ch >>= 1;
        outputNrBits++;
        if (outputNrBits >= 8)
            {
            unsigned char b = outputBitBuf & 0xff;
            //printf("b:%02x\n", b);
            put(b);
            }
        }
}

static unsigned int bitReverse(unsigned int code, unsigned int nrBits)
{
    unsigned int outb = 0;
    while (nrBits--)
        {
        outb = (outb << 1) | (code & 0x01);
        code >>= 1;
        }
    return outb;
}


/**
 *
 */
void Deflater::putBitsR(unsigned int ch, unsigned int bitsWanted)
{
    //trace("r:%4u, %d", ch, bitsWanted);

    unsigned int rcode = bitReverse(ch, bitsWanted);

    putBits(rcode, bitsWanted);

}


//#############################
//#  E N C O D E
//#############################



void Deflater::encodeLiteralStatic(unsigned int ch)
{
    //trace("c: %d", ch);

    if (ch < 144)
        {
        putBitsR(ch + 0x0030 , 8); // 00110000
        }
    else if (ch < 256)
        {
        putBitsR(ch - 144 + 0x0190 , 9); // 110010000
        }
    else if (ch < 280)
        {
        putBitsR(ch - 256 + 0x0000 , 7); // 0000000
        }
    else if (ch < 288)
        {
        putBitsR(ch - 280 + 0x00c0 , 8); // 11000000
        }
    else //out of range
        {
        error("Literal out of range: %d", ch);
        }

}


typedef struct
{
    unsigned int base;
    unsigned int range;
    unsigned int bits;
} LenBase;

LenBase lenBases[] =
{
    {   3,  1, 0 },
    {   4,  1, 0 },
    {   5,  1, 0 },
    {   6,  1, 0 },
    {   7,  1, 0 },
    {   8,  1, 0 },
    {   9,  1, 0 },
    {  10,  1, 0 },
    {  11,  2, 1 },
    {  13,  2, 1 },
    {  15,  2, 1 },
    {  17,  2, 1 },
    {  19,  4, 2 },
    {  23,  4, 2 },
    {  27,  4, 2 },
    {  31,  4, 2 },
    {  35,  8, 3 },
    {  43,  8, 3 },
    {  51,  8, 3 },
    {  59,  8, 3 },
    {  67, 16, 4 },
    {  83, 16, 4 },
    {  99, 16, 4 },
    { 115, 16, 4 },
    { 131, 32, 5 },
    { 163, 32, 5 },
    { 195, 32, 5 },
    { 227, 32, 5 },
    { 258,  1, 0 }
};

typedef struct
{
    unsigned int base;
    unsigned int range;
    unsigned int bits;
} DistBase;

DistBase distBases[] =
{
    {     1,    1,  0 },
    {     2,    1,  0 },
    {     3,    1,  0 },
    {     4,    1,  0 },
    {     5,    2,  1 },
    {     7,    2,  1 },
    {     9,    4,  2 },
    {    13,    4,  2 },
    {    17,    8,  3 },
    {    25,    8,  3 },
    {    33,   16,  4 },
    {    49,   16,  4 },
    {    65,   32,  5 },
    {    97,   32,  5 },
    {   129,   64,  6 },
    {   193,   64,  6 },
    {   257,  128,  7 },
    {   385,  128,  7 },
    {   513,  256,  8 },
    {   769,  256,  8 },
    {  1025,  512,  9 },
    {  1537,  512,  9 },
    {  2049, 1024, 10 },
    {  3073, 1024, 10 },
    {  4097, 2048, 11 },
    {  6145, 2048, 11 },
    {  8193, 4096, 12 },
    { 12289, 4096, 12 },
    { 16385, 8192, 13 },
    { 24577, 8192, 13 }
};

void Deflater::encodeDistStatic(unsigned int len, unsigned int dist)
{

    //## Output length

    if (len < 3 || len > 258)
        {
        error("Length out of range:%d", len);
        return;
        }

    bool found = false;
    for (int i=0 ; i<30 ; i++)
        {
        unsigned int base  = lenBases[i].base;
        unsigned int range = lenBases[i].range;
        if (base + range > len)
            {
            unsigned int lenCode = 257 + i;
            unsigned int length  = len - base;
            //trace("--- %d %d %d %d", len, base, range, length);
            encodeLiteralStatic(lenCode);
            putBits(length, lenBases[i].bits);
            found = true;
            break;
            }
        }
    if (!found)
        {
        error("Length not found in table:%d", len);
        return;
        }

    //## Output distance

    if (dist < 4 || dist > 32768)
        {
        error("Distance out of range:%d", dist);
        return;
        }

    found = false;
    for (int i=0 ; i<30 ; i++)
        {
        unsigned int base  = distBases[i].base;
        unsigned int range = distBases[i].range;
        if (base + range > dist)
            {
            unsigned int distCode = i;
            unsigned int distance = dist - base;
            //error("--- %d %d %d %d", dist, base, range, distance);
            putBitsR(distCode, 5);
            putBits(distance, distBases[i].bits);
            found = true;
            break;
            }
        }
    if (!found)
        {
        error("Distance not found in table:%d", dist);
        return;
        }
}


//#############################
//#  C O M P R E S S
//#############################


/**
 * This method does the dirty work of dictionary
 * compression.  Basically it looks for redundant
 * strings and has the current duplicate refer back
 * to the previous one.
 */
bool Deflater::compressWindow()
{
    windowPos = 0;
    unsigned int windowSize = window.size();
    //### Compress as much of the window as possible

    unsigned int hash = 0;
    //Have each value be a long with the byte at this position,
    //plus the 3 bytes after it in the window
    for (int i=windowSize-1 ; i>=0 ; i--)
        {
        unsigned char ch = window[i];
        windowBuf[i] = ch;
        hash = ((hash<<8) & 0xffffff00) | ch;
        windowHashBuf[i] = hash;
        }

    while (windowPos < windowSize - 3)
        {
        //### Find best match, if any
        unsigned int bestMatchLen  = 0;
        unsigned int bestMatchDist = 0;
        if (windowPos >= 4)
            {
            for (unsigned int lookBack=0 ; lookBack<windowPos-4 ; lookBack++)
                {
                //Check 4-char hashes first, before continuing with string
                if (windowHashBuf[lookBack] == windowHashBuf[windowPos])
                    {
                    unsigned int lookAhead=4;
                    unsigned int lookAheadMax = windowSize - 4 - windowPos;
                    if (lookBack + lookAheadMax >= windowPos -4 )
                        lookAheadMax = windowPos - 4 - lookBack;
                    if (lookAheadMax > 258)
                        lookAheadMax = 258;
                    unsigned char *wp = &(windowBuf[windowPos+4]);
                    unsigned char *lb = &(windowBuf[lookBack+4]);
                    while (lookAhead<lookAheadMax)
                        {
                        if (*lb++ != *wp++)
                            break;
                        lookAhead++;
                        }
                    if (lookAhead > bestMatchLen)
                        {
                        bestMatchLen  = lookAhead;
                        bestMatchDist = windowPos - lookBack;
                        }
                    }
                }
            }
        if (bestMatchLen > 3)
            {
            //Distance encode
            //trace("### distance");
            /*
            printf("### 1 '");
            for (int i=0 ; i < bestMatchLen ; i++)
                fputc(window[windowPos+i], stdout);
            printf("'\n### 2 '");
            for (int i=0 ; i < bestMatchLen ; i++)
                fputc(window[windowPos-bestMatchDist+i], stdout);
            printf("'\n");
            */
            encodeDistStatic(bestMatchLen, bestMatchDist);
            windowPos += bestMatchLen;
            }
        else
            {
            //Literal encode
            //trace("### literal");
            encodeLiteralStatic(windowBuf[windowPos]);
            windowPos++;
            }
        }

    while (windowPos < windowSize)
        encodeLiteralStatic(windowBuf[windowPos++]);

    encodeLiteralStatic(256);
    return true;
}


/**
 *
 */
bool Deflater::compress()
{
    //trace("compress");
    unsigned long total = 0L;
    windowPos = 0;
    std::vector<unsigned char>::iterator iter;
    for (iter = uncompressed.begin(); iter != uncompressed.end() ; )
        {
        total += windowPos;
        trace("total:%ld", total);
        if (windowPos > window.size())
            windowPos = window.size();
        window.erase(window.begin() , window.begin()+windowPos);
        while (window.size() < 32768 && iter != uncompressed.end())
            {
            window.push_back(*iter);
            ++iter;
            }
        if (window.size() >= 32768)
            putBits(0x00, 1); //0  -- more blocks
        else
            putBits(0x01, 1); //1  -- last block
        putBits(0x01, 2); //01 -- static trees
        if (!compressWindow())
            return false;
        }
    putFlush();
    return true;
}





//########################################################################
//#  G Z I P    F I L E
//########################################################################

/**
 * Constructor
 */
GzipFile::GzipFile() :
    data(),
    fileName(),
    fileBuf(),
    fileBufPos(0),
    compressionMethod(0)
{
}

/**
 * Destructor
 */
GzipFile::~GzipFile()
{
}

/**
 *  Print error messages
 */
void GzipFile::error(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "GzipFile error:");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}

/**
 *  Print trace messages
 */
void GzipFile::trace(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "GzipFile:");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}

/**
 *
 */
void GzipFile::put(unsigned char ch)
{
    data.push_back(ch);
}

/**
 *
 */
void GzipFile::setData(const std::vector<unsigned char> &str)
{
    data = str;
}

/**
 *
 */
void GzipFile::clearData()
{
    data.clear();
}

/**
 *
 */
std::vector<unsigned char> &GzipFile::getData()
{
    return data;
}

/**
 *
 */
std::string &GzipFile::getFileName()
{
    return fileName;
}

/**
 *
 */
void GzipFile::setFileName(const std::string &val)
{
    fileName = val;
}



//#####################################
//# U T I L I T Y
//#####################################

/**
 *  Loads a new file into an existing GzipFile
 */
bool GzipFile::loadFile(const std::string &fName)
{
    FILE *f = fopen(fName.c_str() , "rb");
    if (!f)
        {
        error("Cannot open file %s", fName.c_str());
        return false;
        }
    while (true)
        {
        int ch = fgetc(f);
        if (ch < 0)
            break;
        data.push_back(ch);
        }
    fclose(f);
    setFileName(fName);
    return true;
}



//#####################################
//# W R I T E
//#####################################

/**
 *
 */
bool GzipFile::putByte(unsigned char ch)
{
    fileBuf.push_back(ch);
    return true;
}



/**
 *
 */
bool GzipFile::putLong(unsigned long val)
{
    fileBuf.push_back( (unsigned char)((val    ) & 0xff));
    fileBuf.push_back( (unsigned char)((val>> 8) & 0xff));
    fileBuf.push_back( (unsigned char)((val>>16) & 0xff));
    fileBuf.push_back( (unsigned char)((val>>24) & 0xff));
    return true;
}



/**
 *
 */
bool GzipFile::write()
{
    fileBuf.clear();

    putByte(0x1f); //magic
    putByte(0x8b); //magic
    putByte(   8); //compression method
    putByte(0x08); //flags.  say we have a crc and file name

    unsigned long ltime = (unsigned long) time(NULL);
    putLong(ltime);

    //xfl
    putByte(0);
    //OS
    putByte(0);

    //file name
    for (unsigned int i=0 ; i<fileName.size() ; i++)
        putByte(fileName[i]);
    putByte(0);


    //compress
    std::vector<unsigned char> compBuf;
    Deflater deflater;
    if (!deflater.deflate(compBuf, data))
        {
        return false;
        }

    std::vector<unsigned char>::iterator iter;
    for (iter=compBuf.begin() ; iter!=compBuf.end() ; ++iter)
        {
        unsigned char ch = *iter;
        putByte(ch);
        }

    Crc32 crcEngine;
    crcEngine.update(data);
    unsigned long crc = crcEngine.getValue();
    putLong(crc);

    putLong(data.size());

    return true;
}


/**
 *
 */
bool GzipFile::writeBuffer(std::vector<unsigned char> &outBuf)
{
    if (!write())
        return false;
    outBuf.clear();
    outBuf = fileBuf;
    return true;
}


/**
 *
 */
bool GzipFile::writeFile(const std::string &fileName)
{
    if (!write())
        return false;
    FILE *f = fopen(fileName.c_str(), "wb");
    if (!f)
        return false;
    std::vector<unsigned char>::iterator iter;
    for (iter=fileBuf.begin() ; iter!=fileBuf.end() ; ++iter)
        {
        unsigned char ch = *iter;
        fputc(ch, f);
        }
    fclose(f);
    return true;
}


//#####################################
//# R E A D
//#####################################

bool GzipFile::getByte(unsigned char *ch)
{
    if (fileBufPos >= fileBuf.size())
        {
        error("unexpected end of data");
        return false;
        }
    *ch = fileBuf[fileBufPos++];
    return true;
}

/**
 *
 */
bool GzipFile::getLong(unsigned long *val)
{
    if (fileBuf.size() - fileBufPos < 4)
        return false;
    int ch1 = fileBuf[fileBufPos++];
    int ch2 = fileBuf[fileBufPos++];
    int ch3 = fileBuf[fileBufPos++];
    int ch4 = fileBuf[fileBufPos++];
    *val = ((ch4<<24) & 0xff000000L) |
           ((ch3<<16) & 0x00ff0000L) |
           ((ch2<< 8) & 0x0000ff00L) |
           ((ch1    ) & 0x000000ffL);
    return true;
}

bool GzipFile::read()
{
    fileBufPos = 0;

    unsigned char ch;

    //magic cookie
    if (!getByte(&ch))
        return false;
    if (ch != 0x1f)
        {
        error("bad gzip header");
        return false;
        }
    if (!getByte(&ch))
        return false;
    if (ch != 0x8b)
        {
        error("bad gzip header");
        return false;
        }

    //## compression method
    if (!getByte(&ch))
        return false;
    compressionMethod = ch & 0xff;

    //## flags
    if (!getByte(&ch))
        return false;
    //bool ftext    = ch & 0x01;
    bool fhcrc    = ch & 0x02;
    bool fextra   = ch & 0x04;
    bool fname    = ch & 0x08;
    bool fcomment = ch & 0x10;

    //trace("cm:%d ftext:%d fhcrc:%d fextra:%d fname:%d fcomment:%d",
    //         cm, ftext, fhcrc, fextra, fname, fcomment);

    //## file time
    unsigned long ltime;
    if (!getLong(&ltime))
        return false;
    //time_t mtime = (time_t)ltime;

    //## XFL
    if (!getByte(&ch))
        return false;
    //int xfl = ch;

    //## OS
    if (!getByte(&ch))
        return false;
    //int os = ch;

    //std::string timestr = ctime(&mtime);
    //trace("xfl:%d os:%d mtime:%s", xfl, os, timestr.c_str());

    if (fextra)
        {
        if (!getByte(&ch))
            return false;
        long xlen = ch;
        if (!getByte(&ch))
            return false;
        xlen = (xlen << 8) + ch;
        for (long l=0 ; l<xlen ; l++)
            {
            if (!getByte(&ch))
                return false;
            }
        }

    if (fname)
        {
        fileName = "";
	while (true)
	    {
            if (!getByte(&ch))
                return false;
            if (ch==0)
                break;
            fileName.push_back(ch);
            }
        }

    if (fcomment)
        {
	while (true)
	    {
            if (!getByte(&ch))
                return false;
            if (ch==0)
                break;
            }
        }

    if (fhcrc)
        {
        if (!getByte(&ch))
            return false;
        if (!getByte(&ch))
            return false;
        }

    //read remainder of stream
    //compressed data runs up until 8 bytes before end of buffer
    std::vector<unsigned char> compBuf;
    while (fileBufPos < fileBuf.size() - 8)
        {
        if (!getByte(&ch))
            return false;
        compBuf.push_back(ch);
        }
    //uncompress
    data.clear();
    Inflater inflater;
    if (!inflater.inflate(data, compBuf))
        {
        return false;
        }

    //Get the CRC and compare
    Crc32 crcEngine;
    crcEngine.update(data);
    unsigned long calcCrc = crcEngine.getValue();
    unsigned long givenCrc;
    if (!getLong(&givenCrc))
        return false;
    if (givenCrc != calcCrc)
        {
        error("Specified crc, %ud, not what received: %ud",
                givenCrc, calcCrc);
        return false;
        }

    //Get the file size and compare
    unsigned long givenFileSize;
    if (!getLong(&givenFileSize))
        return false;
    if (givenFileSize != data.size())
        {
        error("Specified data size, %ld, not what received: %ld",
                givenFileSize, data.size());
        return false;
        }

    return true;
}



/**
 *
 */
bool GzipFile::readBuffer(const std::vector<unsigned char> &inbuf)
{
    fileBuf = inbuf;
    if (!read())
        return false;
    return true;
}


/**
 *
 */
bool GzipFile::readFile(const std::string &fileName)
{
    fileBuf.clear();
    FILE *f = fopen(fileName.c_str(), "rb");
    if (!f)
        return false;
    while (true)
        {
        int ch = fgetc(f);
        if (ch < 0)
            break;
        fileBuf.push_back(ch);
        }
    fclose(f);
    if (!read())
        return false;
    return true;
}








//########################################################################
//#  Z I P    F I L E
//########################################################################

/**
 * Constructor
 */
ZipEntry::ZipEntry() :
    crc (0L),
    fileName (),
    comment (),
    compressionMethod (8),
    compressedData (),
    uncompressedData (),
    position (0)
{
}

/**
 *
 */
ZipEntry::ZipEntry(const std::string &fileNameArg,
                   const std::string &commentArg) :
    crc (0L),
    fileName (fileNameArg),
    comment (commentArg),
    compressionMethod (8),
    compressedData (),
    uncompressedData (),
    position (0)
{
}

/**
 * Destructor
 */
ZipEntry::~ZipEntry()
{
}


/**
 *
 */
std::string ZipEntry::getFileName()
{
    return fileName;
}

/**
 *
 */
void ZipEntry::setFileName(const std::string &val)
{
    fileName = val;
}

/**
 *
 */
std::string ZipEntry::getComment()
{
    return comment;
}

/**
 *
 */
void ZipEntry::setComment(const std::string &val)
{
    comment = val;
}

/**
 *
 */
unsigned long ZipEntry::getCompressedSize()
{
    return (unsigned long)compressedData.size();
}

/**
 *
 */
int ZipEntry::getCompressionMethod()
{
    return compressionMethod;
}

/**
 *
 */
void ZipEntry::setCompressionMethod(int val)
{
    compressionMethod = val;
}

/**
 *
 */
std::vector<unsigned char> &ZipEntry::getCompressedData()
{
    return compressedData;
}

/**
 *
 */
void ZipEntry::setCompressedData(const std::vector<unsigned char> &val)
{
    compressedData = val;
}

/**
 *
 */
unsigned long ZipEntry::getUncompressedSize()
{
    return (unsigned long)uncompressedData.size();
}

/**
 *
 */
std::vector<unsigned char> &ZipEntry::getUncompressedData()
{
    return uncompressedData;
}

/**
 *
 */
void ZipEntry::setUncompressedData(const std::vector<unsigned char> &val)
{
    uncompressedData = val;
}

/**
 *
 */
unsigned long ZipEntry::getCrc()
{
    return crc;
}

/**
 *
 */
void ZipEntry::setCrc(unsigned long val)
{
    crc = val;
}

/**
 *
 */
void ZipEntry::write(unsigned char ch)
{
    uncompressedData.push_back(ch);
}

/**
 *
 */
void ZipEntry::finish()
{
    Crc32 c32;
    std::vector<unsigned char>::iterator iter;
    for (iter = uncompressedData.begin() ;
           iter!= uncompressedData.end() ; ++iter)
        {
        unsigned char ch = *iter;
        c32.update(ch);
        }
    crc = c32.getValue();
    switch (compressionMethod)
        {
        case 0: //none
            {
            for (iter = uncompressedData.begin() ;
               iter!= uncompressedData.end() ; ++iter)
                {
                unsigned char ch = *iter;
                compressedData.push_back(ch);
                }
            break;
            }
        case 8: //deflate
            {
            Deflater deflater;
            if (!deflater.deflate(compressedData, uncompressedData))
                {
                //some error
                }
            break;
            }
        default:
            {
            printf("error: unknown compression method %d\n",
                    compressionMethod);
            }
        }
}




/**
 *
 */
bool ZipEntry::readFile(const std::string &fileNameArg,
                        const std::string &commentArg)
{
    crc = 0L;
    uncompressedData.clear();
    fileName = fileNameArg;
    comment  = commentArg;
    FILE *f = fopen(fileName.c_str(), "rb");
    if (!f)
        {
        return false;
        }
    while (true)
        {
        int ch = fgetc(f);
        if (ch < 0)
            break;
        uncompressedData.push_back((unsigned char)ch);
        }
    fclose(f);
    finish();
    return true;
}


/**
 *
 */
void ZipEntry::setPosition(unsigned long val)
{
    position = val;
}

/**
 *
 */
unsigned long ZipEntry::getPosition()
{
    return position;
}







/**
 * Constructor
 */
ZipFile::ZipFile() :
    entries(),
    fileBuf(),
    fileBufPos(0),
    comment()
{
}

/**
 * Destructor
 */
ZipFile::~ZipFile()
{
    std::vector<ZipEntry *>::iterator iter;
    for (iter=entries.begin() ; iter!=entries.end() ; ++iter)
        {
        ZipEntry *entry = *iter;
        delete entry;
        }
    entries.clear();
}

/**
 *
 */
void ZipFile::setComment(const std::string &val)
{
    comment = val;
}

/**
 *
 */
std::string ZipFile::getComment()
{
    return comment;
}


/**
 *
 */
std::vector<ZipEntry *> &ZipFile::getEntries()
{
    return entries;
}



//#####################################
//# M E S S A G E S
//#####################################

void ZipFile::error(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "ZipFile error:");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}

void ZipFile::trace(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "ZipFile:");
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}

//#####################################
//# U T I L I T Y
//#####################################

/**
 *
 */
ZipEntry *ZipFile::addFile(const std::string &fileName,
                      const std::string &comment)
{
    ZipEntry *ze = new ZipEntry();
    if (!ze->readFile(fileName, comment))
        {
        delete ze;
        return NULL;
        }
    entries.push_back(ze);
    return ze;
}


/**
 *
 */
ZipEntry *ZipFile::newEntry(const std::string &fileName,
                            const std::string &comment)
{
    ZipEntry *ze = new ZipEntry(fileName, comment);
    entries.push_back(ze);
    return ze;
}


//#####################################
//# W R I T E
//#####################################

/**
 *
 */
bool ZipFile::putLong(unsigned long val)
{
    fileBuf.push_back( ((int)(val    )) & 0xff);
    fileBuf.push_back( ((int)(val>> 8)) & 0xff);
    fileBuf.push_back( ((int)(val>>16)) & 0xff);
    fileBuf.push_back( ((int)(val>>24)) & 0xff);
    return true;
}


/**
 *
 */
bool ZipFile::putInt(unsigned int val)
{
    fileBuf.push_back( (val    ) & 0xff);
    fileBuf.push_back( (val>> 8) & 0xff);
    return true;
}

/**
 *
 */
bool ZipFile::putByte(unsigned char val)
{
    fileBuf.push_back(val);
    return true;
}

/**
 *
 */
bool ZipFile::writeFileData()
{
    std::vector<ZipEntry *>::iterator iter;
    for (iter = entries.begin() ; iter != entries.end() ; ++iter)
        {
        ZipEntry *entry = *iter;
        entry->setPosition(fileBuf.size());
        //##### HEADER
        std::string fname = entry->getFileName();
        putLong(0x04034b50L);
        putInt(20); //versionNeeded
        putInt(0); //gpBitFlag
        //putInt(0); //compression method
        putInt(entry->getCompressionMethod()); //compression method
        putInt(0); //mod time
        putInt(0); //mod date
        putLong(entry->getCrc()); //crc32
        putLong(entry->getCompressedSize());
        putLong(entry->getUncompressedSize());
        putInt(fname.size());//fileName length
        putInt(8);//extra field length
        //file name
        for (unsigned int i=0 ; i<fname.size() ; i++)
            putByte((unsigned char)fname[i]);
        //extra field
        putInt(0x7855);
        putInt(4);
        putInt(100);
        putInt(100);

        //##### DATA
        std::vector<unsigned char> &buf = entry->getCompressedData();
        std::vector<unsigned char>::iterator iter;
        for (iter = buf.begin() ; iter != buf.end() ; ++iter)
            {
            unsigned char ch = (unsigned char) *iter;
            putByte(ch);
            }
        }
    return true;
}

/**
 *
 */
bool ZipFile::writeCentralDirectory()
{
    unsigned long cdPosition = fileBuf.size();
    std::vector<ZipEntry *>::iterator iter;
    for (iter = entries.begin() ; iter != entries.end() ; ++iter)
        {
        ZipEntry *entry = *iter;
        std::string fname   = entry->getFileName();
        std::string ecomment = entry->getComment();
        putLong(0x02014b50L);  //magic cookie
        putInt(2386); //versionMadeBy
        putInt(20); //versionNeeded
        putInt(0); //gpBitFlag
        putInt(entry->getCompressionMethod()); //compression method
        putInt(0); //mod time
        putInt(0); //mod date
        putLong(entry->getCrc()); //crc32
        putLong(entry->getCompressedSize());
        putLong(entry->getUncompressedSize());
        putInt(fname.size());//fileName length
        putInt(4);//extra field length
        putInt(ecomment.size());//comment length
        putInt(0); //disk number start
        putInt(0); //internal attributes
        putLong(0); //external attributes
        putLong(entry->getPosition());

        //file name
        for (unsigned int i=0 ; i<fname.size() ; i++)
            putByte((unsigned char)fname[i]);
        //extra field
        putInt(0x7855);
        putInt(0);
        //comment
        for (unsigned int i=0 ; i<ecomment.size() ; i++)
            putByte((unsigned char)ecomment[i]);
        }
    unsigned long cdSize = fileBuf.size() - cdPosition;

    putLong(0x06054b50L);
    putInt(0);//number of this disk
    putInt(0);//nr of disk with central dir
    putInt(entries.size()); //number of entries on this disk
    putInt(entries.size()); //number of entries total
    putLong(cdSize);  //size of central dir
    putLong(cdPosition); //position of central dir
    putInt(comment.size());//comment size
    for (unsigned int i=0 ; i<comment.size() ; i++)
        putByte(comment[i]);
    return true;
}



/**
 *
 */
bool ZipFile::write()
{
    fileBuf.clear();
    if (!writeFileData())
        return false;
    if (!writeCentralDirectory())
        return false;
    return true;
}


/**
 *
 */
bool ZipFile::writeBuffer(std::vector<unsigned char> &outBuf)
{
    if (!write())
        return false;
    outBuf.clear();
    outBuf = fileBuf;
    return true;
}


/**
 *
 */
bool ZipFile::writeFile(const std::string &fileName)
{
    if (!write())
        return false;
    FILE *f = fopen(fileName.c_str(), "wb");
    if (!f)
        return false;
    std::vector<unsigned char>::iterator iter;
    for (iter=fileBuf.begin() ; iter!=fileBuf.end() ; ++iter)
        {
        unsigned char ch = *iter;
        fputc(ch, f);
        }
    fclose(f);
    return true;
}

//#####################################
//# R E A D
//#####################################

/**
 *
 */
bool ZipFile::getLong(unsigned long *val)
{
    if (fileBuf.size() - fileBufPos < 4)
        return false;
    int ch1 = fileBuf[fileBufPos++];
    int ch2 = fileBuf[fileBufPos++];
    int ch3 = fileBuf[fileBufPos++];
    int ch4 = fileBuf[fileBufPos++];
    *val = ((ch4<<24) & 0xff000000L) |
           ((ch3<<16) & 0x00ff0000L) |
           ((ch2<< 8) & 0x0000ff00L) |
           ((ch1    ) & 0x000000ffL);
    return true;
}

/**
 *
 */
bool ZipFile::getInt(unsigned int *val)
{
    if (fileBuf.size() - fileBufPos < 2)
        return false;
    int ch1 = fileBuf[fileBufPos++];
    int ch2 = fileBuf[fileBufPos++];
    *val = ((ch2<< 8) & 0xff00) |
           ((ch1    ) & 0x00ff);
    return true;
}


/**
 *
 */
bool ZipFile::getByte(unsigned char *val)
{
    if (fileBuf.size() <= fileBufPos)
        return false;
    *val = fileBuf[fileBufPos++];
    return true;
}


/**
 *
 */
bool ZipFile::readFileData()
{
    //printf("#################################################\n");
    //printf("###D A T A\n");
    //printf("#################################################\n");
    while (true)
        {
        unsigned long magicCookie;
        if (!getLong(&magicCookie))
            {
            error("magic cookie not found");
            break;
            }
        trace("###Cookie:%lx", magicCookie);
        if (magicCookie == 0x02014b50L) //central directory
            break;
        if (magicCookie != 0x04034b50L)
            {
            error("file header not found");
            return false;
            }
        unsigned int versionNeeded;
        if (!getInt(&versionNeeded))
            {
            error("bad version needed found");
            return false;
            }
        unsigned int gpBitFlag;
        if (!getInt(&gpBitFlag))
            {
            error("bad bit flag found");
            return false;
            }
        unsigned int compressionMethod;
        if (!getInt(&compressionMethod))
            {
            error("bad compressionMethod found");
            return false;
            }
        unsigned int modTime;
        if (!getInt(&modTime))
            {
            error("bad modTime found");
            return false;
            }
        unsigned int modDate;
        if (!getInt(&modDate))
            {
            error("bad modDate found");
            return false;
            }
        unsigned long crc32;
        if (!getLong(&crc32))
            {
            error("bad crc32 found");
            return false;
            }
        unsigned long compressedSize;
        if (!getLong(&compressedSize))
            {
            error("bad compressedSize found");
            return false;
            }
        unsigned long uncompressedSize;
        if (!getLong(&uncompressedSize))
            {
            error("bad uncompressedSize found");
            return false;
            }
        unsigned int fileNameLength;
        if (!getInt(&fileNameLength))
            {
            error("bad fileNameLength found");
            return false;
            }
        unsigned int extraFieldLength;
        if (!getInt(&extraFieldLength))
            {
            error("bad extraFieldLength found");
            return false;
            }
        std::string fileName;
        for (unsigned int i=0 ; i<fileNameLength ; i++)
            {
            unsigned char ch;
            if (!getByte(&ch))
                break;
            fileName.push_back(ch);
            }
        std::string extraField;
        for (unsigned int i=0 ; i<extraFieldLength ; i++)
            {
            unsigned char ch;
            if (!getByte(&ch))
                break;
            extraField.push_back(ch);
            }
        trace("#########################  DATA");
        trace("FileName           :%d:%s" , fileName.size(), fileName.c_str());
        trace("Extra field        :%d:%s" , extraField.size(), extraField.c_str());
        trace("Version needed     :%d" , versionNeeded);
        trace("Bitflag            :%d" , gpBitFlag);
        trace("Compression Method :%d" , compressionMethod);
        trace("Mod time           :%d" , modTime);
        trace("Mod date           :%d" , modDate);
        trace("CRC                :%lx", crc32);
        trace("Compressed size    :%ld", compressedSize);
        trace("Uncompressed size  :%ld", uncompressedSize);

        //#### Uncompress the data
        std::vector<unsigned char> compBuf;
        if (gpBitFlag & 0x8)//bit 3 was set.  means we dont know compressed size
            {
            unsigned char c1, c2, c3, c4;
            c2 = c3 = c4 = 0;
            while (true)
                {
                unsigned char ch;
                if (!getByte(&ch))
                    {
                    error("premature end of data");
                    break;
                    }
                compBuf.push_back(ch);
                c1 = c2; c2 = c3; c3 = c4; c4 = ch;
                if (c1 == 0x50 && c2 == 0x4b && c3 == 0x07 && c4 == 0x08)
                    {
                    trace("found end of compressed data");
                    //remove the cookie
                    compBuf.erase(compBuf.end() -4, compBuf.end());
                    break;
                    }
                }
            }
        else
            {
            for (unsigned long bnr = 0 ; bnr < compressedSize ; bnr++)
                {
                unsigned char ch;
                if (!getByte(&ch))
                    {
                    error("premature end of data");
                    break;
                    }
                compBuf.push_back(ch);
                }
            }

        printf("### data: ");
        for (int i=0 ; i<10 ; i++)
            printf("%02x ", compBuf[i] & 0xff);
        printf("\n");

        if (gpBitFlag & 0x8)//only if bit 3 set
            {
            /* this cookie was read in the loop above
            unsigned long dataDescriptorSignature ;
            if (!getLong(&dataDescriptorSignature))
                break;
            if (dataDescriptorSignature != 0x08074b50L)
                {
                error("bad dataDescriptorSignature found");
                return false;
                }
            */
            unsigned long crc32;
            if (!getLong(&crc32))
                {
                error("bad crc32 found");
                return false;
                }
            unsigned long compressedSize;
            if (!getLong(&compressedSize))
                {
                error("bad compressedSize found");
                return false;
                }
            unsigned long uncompressedSize;
            if (!getLong(&uncompressedSize))
                {
                error("bad uncompressedSize found");
                return false;
                }
            }//bit 3 was set
        //break;

        std::vector<unsigned char> uncompBuf;
        switch (compressionMethod)
            {
            case 8: //deflate
                {
                Inflater inflater;
                if (!inflater.inflate(uncompBuf, compBuf))
                    {
                    return false;
                    }
                break;
                }
            default:
                {
                error("Unimplemented compression method %d", compressionMethod);
                return false;
                }
            }

        if (uncompressedSize != uncompBuf.size())
            {
            error("Size mismatch.  Expected %ld, received %ld",
                uncompressedSize, uncompBuf.size());
            return false;
            }

        Crc32 crcEngine;
        crcEngine.update(uncompBuf);
        unsigned long crc = crcEngine.getValue();
        if (crc != crc32)
            {
            error("Crc mismatch.  Calculated %08ux, received %08ux", crc, crc32);
            return false;
            }

        ZipEntry *ze = new ZipEntry(fileName, comment);
        ze->setCompressionMethod(compressionMethod);
        ze->setCompressedData(compBuf);
        ze->setUncompressedData(uncompBuf);
        ze->setCrc(crc);
        entries.push_back(ze);


        }
    return true;
}


/**
 *
 */
bool ZipFile::readCentralDirectory()
{
    //printf("#################################################\n");
    //printf("###D I R E C T O R Y\n");
    //printf("#################################################\n");
    while (true)
        {
        //We start with a central directory cookie already
        //Check at the bottom of the loop.
        unsigned int version;
        if (!getInt(&version))
            {
            error("bad version found");
            return false;
            }
        unsigned int versionNeeded;
        if (!getInt(&versionNeeded))
            {
            error("bad version found");
            return false;
            }
        unsigned int gpBitFlag;
        if (!getInt(&gpBitFlag))
            {
            error("bad bit flag found");
            return false;
            }
        unsigned int compressionMethod;
        if (!getInt(&compressionMethod))
            {
            error("bad compressionMethod found");
            return false;
            }
        unsigned int modTime;
        if (!getInt(&modTime))
            {
            error("bad modTime found");
            return false;
            }
        unsigned int modDate;
        if (!getInt(&modDate))
            {
            error("bad modDate found");
            return false;
            }
        unsigned long crc32;
        if (!getLong(&crc32))
            {
            error("bad crc32 found");
            return false;
            }
        unsigned long compressedSize;
        if (!getLong(&compressedSize))
            {
            error("bad compressedSize found");
            return false;
            }
        unsigned long uncompressedSize;
        if (!getLong(&uncompressedSize))
            {
            error("bad uncompressedSize found");
            return false;
            }
        unsigned int fileNameLength;
        if (!getInt(&fileNameLength))
            {
            error("bad fileNameLength found");
            return false;
            }
        unsigned int extraFieldLength;
        if (!getInt(&extraFieldLength))
            {
            error("bad extraFieldLength found");
            return false;
            }
        unsigned int fileCommentLength;
        if (!getInt(&fileCommentLength))
            {
            error("bad fileCommentLength found");
            return false;
            }
        unsigned int diskNumberStart;
        if (!getInt(&diskNumberStart))
            {
            error("bad diskNumberStart found");
            return false;
            }
        unsigned int internalFileAttributes;
        if (!getInt(&internalFileAttributes))
            {
            error("bad internalFileAttributes found");
            return false;
            }
        unsigned long externalFileAttributes;
        if (!getLong(&externalFileAttributes))
            {
            error("bad externalFileAttributes found");
            return false;
            }
        unsigned long localHeaderOffset;
        if (!getLong(&localHeaderOffset))
            {
            error("bad localHeaderOffset found");
            return false;
            }
        std::string fileName;
        for (unsigned int i=0 ; i<fileNameLength ; i++)
            {
            unsigned char ch;
            if (!getByte(&ch))
                break;
            fileName.push_back(ch);
            }
        std::string extraField;
        for (unsigned int i=0 ; i<extraFieldLength ; i++)
            {
            unsigned char ch;
            if (!getByte(&ch))
                break;
            extraField.push_back(ch);
            }
        std::string fileComment;
        for (unsigned int i=0 ; i<fileCommentLength ; i++)
            {
            unsigned char ch;
            if (!getByte(&ch))
                break;
            fileComment.push_back(ch);
            }
        trace("######################### ENTRY");
        trace("FileName           :%s" , fileName.c_str());
        trace("Extra field        :%s" , extraField.c_str());
        trace("File comment       :%s" , fileComment.c_str());
        trace("Version            :%d" , version);
        trace("Version needed     :%d" , versionNeeded);
        trace("Bitflag            :%d" , gpBitFlag);
        trace("Compression Method :%d" , compressionMethod);
        trace("Mod time           :%d" , modTime);
        trace("Mod date           :%d" , modDate);
        trace("CRC                :%lx", crc32);
        trace("Compressed size    :%ld", compressedSize);
        trace("Uncompressed size  :%ld", uncompressedSize);
        trace("Disk nr start      :%ld", diskNumberStart);
        trace("Header offset      :%ld", localHeaderOffset);


        unsigned long magicCookie;
        if (!getLong(&magicCookie))
            {
            error("magic cookie not found");
            return false;
            }
        trace("###Cookie:%lx", magicCookie);
        if (magicCookie  == 0x06054b50L) //end of central directory
            break;
        else if (magicCookie == 0x05054b50L) //signature
            {
            //## Digital Signature
            unsigned int signatureSize;
            if (!getInt(&signatureSize))
                {
                error("bad signatureSize found");
                return false;
                }
            std::string digitalSignature;
            for (unsigned int i=0 ; i<signatureSize ; i++)
                {
                unsigned char ch;
                if (!getByte(&ch))
                    break;
                digitalSignature.push_back(ch);
                }
            trace("######## SIGNATURE :'%s'" , digitalSignature.c_str());
            }
        else if (magicCookie != 0x02014b50L) //central directory
            {
            error("directory file header not found");
            return false;
            }
        }

    unsigned int diskNr;
    if (!getInt(&diskNr))
        {
        error("bad diskNr found");
        return false;
        }
    unsigned int diskWithCd;
    if (!getInt(&diskWithCd))
        {
        error("bad diskWithCd found");
        return false;
        }
    unsigned int nrEntriesDisk;
    if (!getInt(&nrEntriesDisk))
        {
        error("bad nrEntriesDisk found");
        return false;
        }
    unsigned int nrEntriesTotal;
    if (!getInt(&nrEntriesTotal))
        {
        error("bad nrEntriesTotal found");
        return false;
        }
    unsigned long cdSize;
    if (!getLong(&cdSize))
        {
        error("bad cdSize found");
        return false;
        }
    unsigned long cdPos;
    if (!getLong(&cdPos))
        {
        error("bad cdPos found");
        return false;
        }
    unsigned int commentSize;
    if (!getInt(&commentSize))
        {
        error("bad commentSize found");
        return false;
        }
    comment = "";
    for (unsigned int i=0 ; i<commentSize ; i++)
        {
        unsigned char ch;
        if (!getByte(&ch))
            break;
        comment.push_back(ch);
        }
    trace("######## Zip Comment :'%s'" , comment.c_str());

    return true;
}


/**
 *
 */
bool ZipFile::read()
{
    fileBufPos = 0;
    if (!readFileData())
        {
        return false;
        }
    if (!readCentralDirectory())
        {
        return false;
        }
    return true;
}

/**
 *
 */
bool ZipFile::readBuffer(const std::vector<unsigned char> &inbuf)
{
    fileBuf = inbuf;
    if (!read())
        return false;
    return true;
}


/**
 *
 */
bool ZipFile::readFile(const std::string &fileName)
{
    fileBuf.clear();
    FILE *f = fopen(fileName.c_str(), "rb");
    if (!f)
        return false;
    while (true)
        {
        int ch = fgetc(f);
        if (ch < 0)
            break;
        fileBuf.push_back(ch);
        }
    fclose(f);
    if (!read())
        return false;
    return true;
}









//########################################################################
//#  E N D    O F    F I L E
//########################################################################


