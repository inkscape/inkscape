/* libwpg
 * Copyright (C) 2006 Ariya Hidayat (ariya@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA  02111-1301 USA
 *
 * For further information visit http://libwpg.sourceforge.net
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#ifndef __WPGSTREAMIMPLEMENTATION_H__
#define __WPGSTREAMIMPLEMENTATION_H__

#include "WPGStream.h"

namespace libwpg
{

class WPGFileStreamPrivate;

class WPGFileStream: public WPGInputStream
{
public:
	explicit WPGFileStream(const char* filename);
	~WPGFileStream();
	
	unsigned char getc();
	long read(long n, char* buffer);
	long tell();
	void seek(long offset);
	bool atEnd();

	bool isOle();
	WPGInputStream *getWPGOleStream();

private:
	WPGFileStreamPrivate* d;
	WPGFileStream(const WPGFileStream&); // copy is not allowed
	WPGFileStream& operator=(const WPGFileStream&); // assignment is not allowed
};

class WPGMemoryStreamPrivate;

class WPGMemoryStream: public WPGInputStream
{
public:
	WPGMemoryStream(const char *data, const unsigned int dataSize);
	~WPGMemoryStream();

	unsigned char getc();
	long read(long n, char* buffer);
	long tell();
	void seek(long offset);
	bool atEnd();

	bool isOle();
	WPGInputStream *getWPGOleStream();

private:
	WPGMemoryStreamPrivate* d;
	WPGMemoryStream(const WPGMemoryStream&); // copy is not allowed
	WPGMemoryStream& operator=(const WPGMemoryStream&); // assignment is not allowed
};

} // namespace libwpg

#endif // __WPGSTREAMIMPLEMENTATION_H__
