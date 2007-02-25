/* libwpg
 * Copyright (C) 2006 Ariya Hidayat (ariya@kde.org)
 * Copyright (C) 2004 Marc Oude Kotte (marc@solcon.nl)
 * Copyright (C) 2005 Fridrich Strba (fridrich.strba@bluewin.ch)
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

#ifndef __WPGXPARSER_H__
#define __WPGXPARSER_H__

#include "WPGPaintInterface.h"
#include "WPGStream.h"
#include "WPGColor.h"

#include <map>

class WPGXParser
{
public:
	WPGXParser(libwpg::WPGInputStream *input, libwpg::WPGPaintInterface* painter);
	virtual ~WPGXParser() {};
	virtual bool parse() = 0;
	
	unsigned char readU8();
	unsigned short readU16();  
	unsigned long readU32();  
	char readS8();
	short readS16();  
	long readS32();  
	unsigned int readVariableLengthInteger();
	
protected:
	libwpg::WPGInputStream* m_input;
	libwpg::WPGPaintInterface* m_painter;
	std::map<int,libwpg::WPGColor> m_colorPalette;
};

#endif // __WPGXPARSER_H__
