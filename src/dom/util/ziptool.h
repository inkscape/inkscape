#ifndef SEEN_ZIPTOOL_H
#define SEEN_ZIPTOOL_H
/**
 * This is intended to be a standalone, reduced capability
 * implementation of Gzip and Zip functionality.  Its
 * targeted use case is for archiving and retrieving single files
 * which use these encoding types.  Being memory based and
 * non-optimized, it is not useful in cases where very large
 * archives are needed or where high performance is desired.
 * However, it should hopefully work well for smaller,
 * one-at-a-time tasks.  What you get in return is the ability
 * to drop these files into your project and remove the dependencies
 * on ZLib and Info-Zip.  Enjoy.
 */
/*
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


#include <vector>
#include <string>


//########################################################################
//#  A D L E R  3 2
//########################################################################

class Adler32
{
public:

    Adler32();

    virtual ~Adler32();

    void reset();

    void update(unsigned char b);

    void update(char *str);

    unsigned long getValue();

private:

    unsigned long value;

};


//########################################################################
//#  C R C  3 2
//########################################################################

class Crc32
{
public:

    Crc32();

    virtual ~Crc32();

    void reset();

    void update(unsigned char b);

    void update(char *str);

    void update(const std::vector<unsigned char> &buf);

    unsigned long getValue();

private:

    unsigned long value;

};






//########################################################################
//#  G Z I P    S T R E A M S
//########################################################################

class GzipFile
{
public:

    /**
     *
     */
    GzipFile();

    /**
     *
     */
    virtual ~GzipFile();

    /**
     *
     */
    virtual void put(unsigned char ch);

    /**
     *
     */
    virtual void setData(const std::vector<unsigned char> &str);

    /**
     *
     */
    virtual void clearData();

    /**
     *
     */
    virtual std::vector<unsigned char> &getData();

    /**
     *
     */
    virtual std::string &getFileName();

    /**
     *
     */
    virtual void setFileName(const std::string &val);


    //######################
    //# U T I L I T Y
    //######################

    /**
     *
     */
    virtual bool readFile(const std::string &fName);

    //######################
    //# W R I T E
    //######################

    /**
     *
     */
    virtual bool write();

    /**
     *
     */
    virtual bool writeBuffer(std::vector<unsigned char> &outbuf);

    /**
     *
     */
    virtual bool writeFile(const std::string &fileName);


    //######################
    //# R E A D
    //######################


    /**
     *
     */
    virtual bool read();

    /**
     *
     */
    virtual bool readBuffer(const std::vector<unsigned char> &inbuf);

    /**
     *
     */
    virtual bool loadFile(const std::string &fileName);



private:

    std::vector<unsigned char> data;
    std::string fileName;

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

    std::vector<unsigned char> fileBuf;
    unsigned long fileBufPos;

    bool getByte(unsigned char *ch);
    bool getLong(unsigned long *val);

    bool putByte(unsigned char ch);
    bool putLong(unsigned long val);

    int compressionMethod;
};




//########################################################################
//#  Z I P    F I L E
//########################################################################


/**
 *
 */
class ZipEntry
{
public:

    /**
     *
     */
    ZipEntry();

    /**
     *
     */
    ZipEntry(const std::string &fileName,
             const std::string &comment);

    /**
     *
     */
    virtual ~ZipEntry();

    /**
     *
     */
    virtual std::string getFileName();

    /**
     *
     */
    virtual void setFileName(const std::string &val);

    /**
     *
     */
    virtual std::string getComment();

    /**
     *
     */
    virtual void setComment(const std::string &val);

    /**
     *
     */
    virtual unsigned long getCompressedSize();

    /**
     *
     */
    virtual int getCompressionMethod();

    /**
     *
     */
    virtual void setCompressionMethod(int val);

    /**
     *
     */
    virtual std::vector<unsigned char> &getCompressedData();

    /**
     *
     */
    virtual void setCompressedData(const std::vector<unsigned char> &val);

    /**
     *
     */
    virtual unsigned long getUncompressedSize();

    /**
     *
     */
    virtual std::vector<unsigned char> &getUncompressedData();

    /**
     *
     */
    virtual void setUncompressedData(const std::vector<unsigned char> &val);

    /**
     *
     */
    virtual void write(unsigned char ch);

    /**
     *
     */
    virtual void finish();

    /**
     *
     */
    virtual unsigned long getCrc();

    /**
     *
     */
    virtual void setCrc(unsigned long crc);

    /**
     *
     */
    virtual bool readFile(const std::string &fileNameArg,
                          const std::string &commentArg);

    /**
     *
     */
    virtual void setPosition(unsigned long val);

    /**
     *
     */
    virtual unsigned long getPosition();

private:

    unsigned long crc;

    std::string fileName;
    std::string comment;

    int compressionMethod;

    std::vector<unsigned char> compressedData;
    std::vector<unsigned char> uncompressedData;

    unsigned long position;
};









/**
 * This class sits over the zlib and gzip code to
 * implement a PKWare or Info-Zip .zip file reader and
 * writer
 */
class ZipFile
{
public:

    /**
     *
     */
    ZipFile();

    /**
     *
     */
    virtual ~ZipFile();

    //######################
    //# V A R I A B L E S
    //######################

    /**
     *
     */
    virtual void setComment(const std::string &val);

    /**
     *
     */
    virtual std::string getComment();

    /**
     * Return the list of entries currently in this file
     */
    std::vector<ZipEntry *> &getEntries();


    //######################
    //# U T I L I T Y
    //######################

    /**
     *
     */
    virtual ZipEntry *addFile(const std::string &fileNameArg,
                              const std::string &commentArg);

    /**
     *
     */
    virtual ZipEntry *newEntry(const std::string &fileNameArg,
                               const std::string &commentArg);

    //######################
    //# W R I T E
    //######################

    /**
     *
     */
    virtual bool write();

    /**
     *
     */
    virtual bool writeBuffer(std::vector<unsigned char> &outbuf);

    /**
     *
     */
    virtual bool writeFile(const std::string &fileName);


    //######################
    //# R E A D
    //######################


    /**
     *
     */
    virtual bool read();

    /**
     *
     */
    virtual bool readBuffer(const std::vector<unsigned char> &inbuf);

    /**
     *
     */
    virtual bool readFile(const std::string &fileName);


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

    //# Private writing methods

    /**
     *
     */
    bool putLong(unsigned long val);

    /**
     *
     */
    bool putInt(unsigned int val);


    /**
     *
     */
    bool putByte(unsigned char val);

    /**
     *
     */
    bool writeFileData();

    /**
     *
     */
    bool writeCentralDirectory();


    //# Private reading methods

    /**
     *
     */
    bool getLong(unsigned long *val);

    /**
     *
     */
    bool getInt(unsigned int *val);

    /**
     *
     */
    bool getByte(unsigned char *val);

    /**
     *
     */
    bool readFileData();

    /**
     *
     */
    bool readCentralDirectory();


    std::vector<ZipEntry *> entries;

    std::vector<unsigned char> fileBuf;
    unsigned long fileBufPos;

    std::string comment;
};






#endif // SEEN_ZIPTOOL_H


//########################################################################
//#  E N D    O F    F I L E
//########################################################################

