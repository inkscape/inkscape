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

#include "WPGXParser.h"

using namespace libwpg;

WPGXParser::WPGXParser(WPGInputStream *input, WPGPaintInterface* painter):
  m_input(input), m_painter(painter)
{
}

unsigned char WPGXParser::readU8()
{
	return m_input->getc();
}

unsigned short WPGXParser::readU16()
{
	unsigned short p0 = (unsigned short)readU8();
	unsigned short p1 = (unsigned short)readU8();
	return p0|(p1<<8);
}

unsigned long WPGXParser::readU32()
{
	unsigned long p0 = (unsigned short)readU8();
	unsigned long p1 = (unsigned short)readU8();
	unsigned long p2 = (unsigned short)readU8();
	unsigned long p3 = (unsigned short)readU8();
	return p0|(p1<<8)|(p2<<16)|(p3<<24);
}

char WPGXParser::readS8()
{
	return (char)m_input->getc();
}

short WPGXParser::readS16()
{
	short p0 = readU8();
	short p1 = readS8();
	return p0|(p1<<8);
}

long WPGXParser::readS32()
{
	long p0 = readU8();
	long p1 = readU8();
	long p2 = readU8();
	long p3 = readS8();
	return p0|(p1<<8)|(p2<<16)|(p3<<24);
}

unsigned int WPGXParser::readVariableLengthInteger()
{
	// read a byte
	unsigned char value8 = readU8();
	// if it's in the range 0-0xFE, then we have a 8-bit value
	if (value8<=0xFE) {
		return (unsigned int)value8;
	} else {
		// now read a 16 bit value
		unsigned short value16 = readU16();
		// if the MSB is 1, we have a 32 bit value
		if (value16>>15) {
			// read the next 16 bit value (LSB part, in value16 resides the MSB part)
			unsigned long lvalue16 = readU16();
			unsigned long value32 = value16 & 0x7fff;  // mask out the MSB
			return (value32<<16)+lvalue16;
		} else {
			// we have a 16 bit value, return it
			return (unsigned int)value16;
		}
	}
	
	// unreachable
	return 0;
}
