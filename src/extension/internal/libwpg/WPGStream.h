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

#ifndef __WPGSTREAM_H__
#define __WPGSTREAM_H__

namespace libwpg
{

class WPGInputStream
{
public:
	virtual ~WPGInputStream() {}
	virtual unsigned char getc() = 0;
	virtual long read(long n, char* buffer) = 0;
	virtual long tell() = 0;
	virtual void seek(long offset) = 0;
	virtual bool atEnd() = 0;

	virtual bool isOle() = 0;
	virtual WPGInputStream *getWPGOleStream() = 0;
};

} // namespace wpg

#endif // __WPGSTREAM_H__
