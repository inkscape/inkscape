#ifndef __ODF_DOCUMENT_H__
#define __ODF_DOCUMENT_H__
/**
 *
 * This class contains an ODF Document.
 * Initially, we are just concerned with .odg content.xml + resources
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2006 Bob Jamison
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * For more information, please write to rwjj@earthlink.net
 *
 *  RWJ :  080207: Changed to GPL2 by me
 */

#include <vector>
#include <string>

#include "dom/dom.h"

namespace odf
{


//########################################################################
//# I M A G E    D A T A
//########################################################################

/**
 *
 */
class ImageData
{
public:

    /**
     *
     */
    ImageData(const std::string &fileName,
              const std::vector<unsigned char> &buf);

    /**
     *
     */
    ImageData(const ImageData &other);

    /**
     *
     */
    virtual ~ImageData();

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
    virtual std::vector<unsigned char> &getData();

    /**
     *
     */
    virtual void setData(const std::vector<unsigned char> &buf);

private:

    std::string fileName;

    std::vector<unsigned char> data;

};





//########################################################################
//# O D F    D O C U M E N T
//########################################################################


/**
 *
 */
class OdfDocument
{
public:

    /**
     *
     */
    OdfDocument();

    /**
     *  Copy constructor
     */
    OdfDocument(const OdfDocument &other);

    /**
     *
     */
    virtual ~OdfDocument();

    /**
     *
     */
    virtual bool readFile(const std::string &fileName);

    /**
     *
     */
    virtual bool writeFile(const std::string &fileName);


private:

    org::w3c::dom::Document *content;

    std::vector<ImageData> images;

};

} //namespace odf



#endif /*__ODF_DOCUMENT_H__*/

//########################################################################
//# E N D    O F    F I L E
//########################################################################

