/**
 *
 * This class contains an ODF Document.
 * Initially, we are just concerned with .odg content.xml + resources
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2006-2008 Bob Jamison
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
 *
 */

#include "odfdocument.h"


namespace odf
{


//########################################################################
//# I M A G E    D A T A
//########################################################################


/**
 *
 */
ImageData::ImageData(const std::string &fname,
              const std::vector<unsigned char> &buf)
{
    fileName = fname;
    data     = buf;
}

/**
 *
 */
ImageData::ImageData(const ImageData &other)
{
    fileName = other.fileName;
    data     = other.data;
}

/**
 *
 */
ImageData::~ImageData()
{
}

/**
 *
 */
std::string ImageData::getFileName()
{
    return fileName;
}

/**
 *
 */
void ImageData::setFileName(const std::string &val)
{
    fileName = val;
}

/**
 *
 */
std::vector<unsigned char> &ImageData::getData()
{
    return data;
}

/**
 *
 */
void ImageData::setData(const std::vector<unsigned char> &buf)
{
    data = buf;
}





//########################################################################
//# O D F    D O C U M E N T
//########################################################################



/**
 *
 */
OdfDocument::OdfDocument()
{
}


/**
 *
 */
OdfDocument::OdfDocument(const OdfDocument &other)
{
    content = other.content;
    images  = other.images;
}


/**
 *
 */
OdfDocument::~OdfDocument()
{
}

/**
 *
 */
bool OdfDocument::readFile(const std::string &/*fileName*/)
{
    return true;
}

/**
 *
 */
bool OdfDocument::writeFile(const std::string &/*fileName*/)
{
    return true;
}





} //namespace odf




//########################################################################
//# E N D    O F    F I L E
//########################################################################

