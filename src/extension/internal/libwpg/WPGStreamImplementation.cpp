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

#include "WPGStreamImplementation.h"
#include "WPGOLEStream.h"
#include "libwpg.h"

#include <fstream>
#include <sstream>
#include <string>

// MSVC++ 6.0 defines getc as macro, so undefine it
#undef getc

namespace libwpg
{

class WPGFileStreamPrivate
{
public:
	WPGFileStreamPrivate();
	std::fstream file;
	std::stringstream buffer;
};

class WPGMemoryStreamPrivate
{
public:
	WPGMemoryStreamPrivate(const std::string str);
	std::stringstream buffer;
};

} // namespace libwpg

libwpg::WPGFileStreamPrivate::WPGFileStreamPrivate() :
	file(),
	buffer(std::ios::binary | std::ios::in | std::ios::out)
{
}

libwpg::WPGMemoryStreamPrivate::WPGMemoryStreamPrivate(const std::string str) :
	buffer(str, std::ios::binary | std::ios::in)
{
}


libwpg::WPGFileStream::WPGFileStream(const char* filename)
{
	d = new libwpg::WPGFileStreamPrivate;
	
	d->file.open( filename, std::ios::binary | std::ios::in );
}

libwpg::WPGFileStream::~WPGFileStream()
{
	delete d;
}

unsigned char libwpg::WPGFileStream::getc()
{
	return d->file.get();
}

long libwpg::WPGFileStream::read(long nbytes, char* buffer)
{
	long nread = 0;
	
	if(d->file.good())
	{
	long curpos = d->file.tellg();
	d->file.read(buffer, nbytes); 
	nread = (long)d->file.tellg() - curpos;
	}
	
	return nread;
}

long libwpg::WPGFileStream::tell()
{
	return d->file.good() ? (long)d->file.tellg() : -1L;
}

void libwpg::WPGFileStream::seek(long offset)
{
	if(d->file.good())
		d->file.seekg(offset);
}

bool libwpg::WPGFileStream::atEnd()
{
	return d->file.eof();
}

bool libwpg::WPGFileStream::isOle()
{
	if (d->buffer.str().empty())
		d->buffer << d->file.rdbuf();
	Storage tmpStorage( d->buffer );
	if (tmpStorage.isOle())
		return true;
	return false;
}

libwpg::WPGInputStream* libwpg::WPGFileStream::getWPGOleStream()
{
	if (d->buffer.str().empty())
		d->buffer << d->file.rdbuf();
	libwpg::Storage *tmpStorage = new libwpg::Storage( d->buffer );
	libwpg::Stream tmpStream( tmpStorage, "PerfectOffice_MAIN" );
	if (!tmpStorage || (tmpStorage->result() != Storage::Ok)  || !tmpStream.size())
	{
		if (tmpStorage)
			delete tmpStorage;
		return (libwpg::WPGInputStream*)0;
	}
	
	unsigned char *tmpBuffer = new unsigned char[tmpStream.size()];
	unsigned long tmpLength;
	tmpLength = tmpStream.read(tmpBuffer, tmpStream.size());

	// sanity check
	if (tmpLength > tmpStream.size() || tmpLength < tmpStream.size())
	/* something went wrong here and we do not trust the
	   resulting buffer */
	{
		if (tmpStorage)
			delete tmpStorage;
		return (libwpg::WPGInputStream*)0;
	}

	delete tmpStorage;
	return new libwpg::WPGMemoryStream((const char *)tmpBuffer, tmpLength);
}


libwpg::WPGMemoryStream::WPGMemoryStream(const char *data, const unsigned int dataSize)
{
	d = new libwpg::WPGMemoryStreamPrivate(std::string(data, dataSize));
}

libwpg::WPGMemoryStream::~WPGMemoryStream()
{
	delete d;
}

unsigned char libwpg::WPGMemoryStream::getc()
{
	return d->buffer.get();
}

long libwpg::WPGMemoryStream::read(long nbytes, char* buffer)
{
	long nread = 0;
	
	if(d->buffer.good())
	{
	long curpos = d->buffer.tellg();
	d->buffer.read(buffer, nbytes); 
	nread = (long)d->buffer.tellg() - curpos;
	}
	
	return nread;
}

long libwpg::WPGMemoryStream::tell()
{
	return d->buffer.good() ? (long)d->buffer.tellg() : -1L;
}

void libwpg::WPGMemoryStream::seek(long offset)
{
	if(d->buffer.good())
		d->buffer.seekg(offset);
}

bool libwpg::WPGMemoryStream::atEnd()
{
	return d->buffer.eof();
}

bool libwpg::WPGMemoryStream::isOle()
{
	libwpg::Storage tmpStorage( d->buffer );
	if (tmpStorage.isOle())
		return true;
	return false;
}

libwpg::WPGInputStream* libwpg::WPGMemoryStream::getWPGOleStream()
{
	libwpg::Storage *tmpStorage = new libwpg::Storage( d->buffer );
	libwpg::Stream tmpStream( tmpStorage, "PerfectOffice_MAIN" );
	if (!tmpStorage || (tmpStorage->result() != libwpg::Storage::Ok)  || !tmpStream.size())
	{
		if (tmpStorage)
			delete tmpStorage;
		return (libwpg::WPGInputStream*)0;
	}
	
	unsigned char *tmpBuffer = new unsigned char[tmpStream.size()];
	unsigned long tmpLength;
	tmpLength = tmpStream.read(tmpBuffer, tmpStream.size());

	// sanity check
	if (tmpLength > tmpStream.size() || tmpLength < tmpStream.size())
	/* something went wrong here and we do not trust the
	   resulting buffer */
	{
		if (tmpStorage)
			delete tmpStorage;
		return (libwpg::WPGInputStream*)0;
	}

	delete tmpStorage;
	return new libwpg::WPGMemoryStream((const char *)tmpBuffer, tmpLength);
}
