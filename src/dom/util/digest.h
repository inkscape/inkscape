#ifndef __DIGEST_H__
#define __DIGEST_H__
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

/**
 *
 *  This base class and its subclasses  provide an easy API for providing
 *  several different types of secure hashing functions for whatever use
 *  a developer might need.  This is not intended as a high-performance
 *  replacement for the fine implementations already available.  Rather, it
 *  is a small and simple (and maybe a bit slow?) tool for moderate common
 *  hashing requirements, like for communications and authentication.
 *
 *  These hashes are intended to be simple to use.  For example:
 *  Sha256Digest digest;
 *  digest.append("The quick brown dog");
 *  std::string result = digest.finishHex();
 *
 *  There are several forms of append() for convenience.
 *  finish() and finishHex() call reset() for both security and
 *  to prepare for the next use.
 *
 */

#include <vector>
#include <string>


/**
 *  Base class.  Do not use this class directly.  Rather, use of of the
 *  subclasses below.
 *  For all subclasses, overload reset(), update(unsigned char), and finish()
 */
class Digest
{
public:

    /**
     *  Different types of hash algorithms.
     */
    typedef enum
        {
        HASH_NONE,
        HASH_SHA1,
        HASH_SHA224,
        HASH_SHA256,
        HASH_SHA384,
        HASH_SHA512,
        HASH_MD5
        } HashType;

    /**
     *  Constructor, with no type
     */
    Digest() : hashType(HASH_NONE)
        { reset(); }

    /**
     *  Destructor
     */
    virtual ~Digest()
        { reset(); }

    /**
     *  Return one of the enumerated hash types above
     */
    virtual int getType()
        { return hashType; }

    /**
     *  Append a single byte to the hash
     */
    void append(unsigned char ch)
        { update(ch); }

    /**
     *  Append a string to the hash
     */
    virtual void append(const std::string &str)
        {
        for (unsigned int i=0 ; i<str.size() ; i++)
            update((unsigned char)str[i]);
        }

    /**
     *  Append a byte buffer to the hash
     */
    virtual void append(unsigned char *buf, int len)
        {
        for (int i=0 ; i<len ; i++)
            update(buf[i]);
        }

    /**
     *  Append a byte vector to the hash
     */
    virtual void append(const std::vector<unsigned char> buf)
        {
        for (unsigned int i=0 ; i<buf.size() ; i++)
            update(buf[i]);
        }

    /**
     *  Finish the hash and return a hexidecimal version of the computed
     *  value
     */
    virtual std::string finishHex();

    /**
     *  Initialize the fields of this hash engine to its starting values.
     *  Overload this in every subclass
     */
    virtual void reset()
        {}

    /**
     *  Finish the hash and return its computed value
     *  Overload this in every subclass
     */
    virtual std::vector<unsigned char> finish()
        {
        std::vector<unsigned char> ret;
        return ret;
        }

protected:

    /**
     *  Update the hash with a given byte
     *  Overload this in every subclass
     */
    virtual void update(unsigned char /*ch*/)
        {}

    /**
     * The enumerated type of the hash
     */
    int hashType;
};





/**
 *  SHA-1,
 *  Section 6.1, SECURE HASH STANDARD
 *  Federal Information Processing Standards Publication 180-2
 *  http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
 */
class Sha1Digest : public Digest
{
public:

    /**
     *  Constructor
     */
    Sha1Digest()
        { hashType = HASH_SHA1; reset(); }

    /**
     *  Destructor
     */
    virtual ~Sha1Digest()
        { reset(); }

    /**
     *  Overloaded from Digest
     */
    virtual void reset();

    /**
     *  Overloaded from Digest
     */
    virtual std::vector<unsigned char> finish();

protected:

    /**
     *  Overloaded from Digest
     */
    virtual void update(unsigned char val);

private:

    void hashblock();

    unsigned long H[5];
    unsigned long W[80];
    unsigned long long size;
    int lenW;

};






/**
 *  SHA-224,
 *  Section 6.1, SECURE HASH STANDARD
 *  Federal Information Processing Standards Publication 180-2
 *  http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
 */
class Sha224Digest : public Digest
{
public:

    /**
     *  Constructor
     */
    Sha224Digest()
        { hashType = HASH_SHA224; reset(); }

    /**
     *  Destructor
     */
    virtual ~Sha224Digest()
        { reset(); }

    /**
     *  Overloaded from Digest
     */
    virtual void reset();

    /**
     *  Overloaded from Digest
     */
    virtual std::vector<unsigned char> finish();

protected:

    /**
     *  Overloaded from Digest
     */
    virtual void update(unsigned char val);

private:

    void hashblock();

    unsigned long H[8];
    unsigned long W[64];
    unsigned long long size;
    int lenW;

};



/**
 *  SHA-256,
 *  Section 6.1, SECURE HASH STANDARD
 *  Federal Information Processing Standards Publication 180-2
 *  http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
 */
class Sha256Digest : public Digest
{
public:

    /**
     *  Constructor
     */
    Sha256Digest()
        { hashType = HASH_SHA256; reset(); }

    /**
     *  Destructor
     */
    virtual ~Sha256Digest()
        { reset(); }

    /**
     *  Overloaded from Digest
     */
    virtual void reset();

    /**
     *  Overloaded from Digest
     */
    virtual std::vector<unsigned char> finish();

protected:

    /**
     *  Overloaded from Digest
     */
    virtual void update(unsigned char val);

private:

    void hashblock();

    unsigned long H[8];
    unsigned long W[64];
    unsigned long long size;
    int lenW;

};


/**
 *  SHA-384,
 *  Section 6.1, SECURE HASH STANDARD
 *  Federal Information Processing Standards Publication 180-2
 *  http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
 */
class Sha384Digest : public Digest
{
public:

    /**
     *  Constructor
     */
    Sha384Digest()
        { hashType = HASH_SHA384; reset(); }

    /**
     *  Destructor
     */
    virtual ~Sha384Digest()
        { reset(); }

    /**
     *  Overloaded from Digest
     */
    virtual void reset();

    /**
     *  Overloaded from Digest
     */
    virtual std::vector<unsigned char> finish();

protected:

    /**
     *  Overloaded from Digest
     */
    virtual void update(unsigned char val);

private:

    void hashblock();

    unsigned long long H[8];
    unsigned long long W[80];
    unsigned long long size;
    int lenW;

};




/**
 *  SHA-512,
 *  Section 6.1, SECURE HASH STANDARD
 *  Federal Information Processing Standards Publication 180-2
 *  http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
 */
class Sha512Digest : public Digest
{
public:

    /**
     *  Constructor
     */
    Sha512Digest()
        { hashType = HASH_SHA512; reset(); }

    /**
     *  Destructor
     */
    virtual ~Sha512Digest()
        { reset(); }

    /**
     *  Overloaded from Digest
     */
    virtual void reset();

    /**
     *  Overloaded from Digest
     */
    virtual std::vector<unsigned char> finish();

protected:

    /**
     *  Overloaded from Digest
     */
    virtual void update(unsigned char val);

private:

    void hashblock();

    unsigned long long H[8];
    unsigned long long W[80];
    unsigned long long size;
    int lenW;

};









/**
 * IETF RFC 1321, MD5 Specification
 * http://www.ietf.org/rfc/rfc1321.txt
 */
class Md5Digest :  public Digest
{
public:

    /**
     *  Constructor
     */
    Md5Digest()
        { hashType = HASH_MD5; reset(); }

    /**
     *  Destructor
     */
    virtual ~Md5Digest()
        { reset(); }

    /**
     *  Overloaded from Digest
     */
    virtual void reset();

    /**
     *  Overloaded from Digest
     */
    virtual std::vector<unsigned char> finish();

protected:

    /**
     *  Overloaded from Digest
     */
    virtual void update(unsigned char val);

private:

    void hashblock();

    unsigned long hash[8];
    unsigned long W[64];
    unsigned long long size;
    int lenW;

};









#endif /*  __DIGEST_H__ */


