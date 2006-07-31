/* libwpg
 * Copyright (C) 2006 Ariya Hidayat (ariya@kde.org)
 * Copyright (C) 2004 Marc Oude Kotte (marc@solcon.nl)
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

#include "WPGHeader.h"
#include "libwpg_utils.h"

namespace
{
static inline unsigned short readU16( const void* p )
{
	const unsigned char* ptr = (const unsigned char*) p;
	return ptr[0]+(ptr[1]<<8);
}

static inline unsigned long readU32( const void* p )
{
	const unsigned char* ptr = (const unsigned char*) p;
	return ptr[0]+(ptr[1]<<8)+(ptr[2]<<16)+(ptr[3]<<24);
}
}

using namespace libwpg;

WPGHeader::WPGHeader()
{
		// create a sensible default header
		m_identifier[0] = 0xff;
		m_identifier[1] = 'W'; 
		m_identifier[2] = 'P'; 
		m_identifier[3] = 'C'; 
		m_productType = 0x01;
		m_fileType = 0x16;
		m_encryptionKey = 0x00;
		m_majorVersion = 0x02;
		m_minorVersion = 0x00;
		m_encryptionKey = 0;
		m_startOfPacketData = 0;
		m_entryCount = 0;
		m_resourceComplete = 0;
		m_encryptionBlockOffset = 0;
		m_fileSize = 0;
		m_encryptVersion = 0;
}

bool WPGHeader::load(WPGInputStream *input)
{
	input->seek(0);
	
	unsigned char prefix[26];
	long n = input->read(26, (char*)prefix);
	if(n < 26)
		return false;
	
	m_identifier[0] = prefix[0];  
	m_identifier[1] = prefix[1];  
	m_identifier[2] = prefix[2];  
	m_identifier[3] = prefix[3];  
	m_startOfDocument = readU32(prefix+4);
	m_productType = prefix[8];
	m_fileType = prefix[9];
	m_majorVersion = prefix[10];
	m_minorVersion = prefix[11];
	m_encryptionKey = readU16(prefix+12);
	m_startOfPacketData = readU16(prefix+14);
	
	WPG_DEBUG_MSG(("Header Identifier  = %c%c%c\n", m_identifier[1], 
	m_identifier[2], m_identifier[3]));
	WPG_DEBUG_MSG(("Product type       = 0x%x\n",  m_productType));
	WPG_DEBUG_MSG(("File type          = 0x%x\n",  m_fileType));
	WPG_DEBUG_MSG(("Major version      = 0x%x\n",  m_majorVersion));
	WPG_DEBUG_MSG(("Minor version      = 0x%x\n",  m_minorVersion));
	WPG_DEBUG_MSG(("Encryption key     = 0x%x\n",  m_encryptionKey));
	
	return true;
}

bool WPGHeader::isSupported() const
{
	return (
		(m_identifier[0] == 0xFF) &&
		(m_identifier[1] == 'W') &&
		(m_identifier[2] == 'P') &&
		(m_identifier[3] == 'C') &&
		(m_productType == 0x01) &&
		(m_fileType == 0x16) &&
		(m_encryptionKey == 0x00) &&     // we don't support encryption
		((m_majorVersion == 0x02) || (m_majorVersion == 0x01)) &&
		(m_minorVersion == 0x00) 
);
}

unsigned long WPGHeader::startOfDocument() const
{
	return m_startOfDocument;
}

int WPGHeader::majorVersion() const
{
	return m_majorVersion;
}
