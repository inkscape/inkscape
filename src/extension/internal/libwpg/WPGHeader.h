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

#ifndef __WPGHEADER_H__
#define __WPGHEADER_H__

#include "WPGStream.h"

class WPGHeader 
{
public:
	WPGHeader();
	
	bool load(libwpg::WPGInputStream *input);
	
	bool isSupported() const;
	
	unsigned long startOfDocument() const;
	
	int majorVersion() const;

private:
	unsigned char m_identifier[4];          // should always be 0xFF followed by "WPC"
	unsigned long m_startOfDocument;        // index into file
	unsigned char m_productType;            // should always be 1 for WPG files
	unsigned char m_fileType;               // should always be 22 for WPG files
	unsigned char m_majorVersion;           // 2 for WPG 8.0 files
	unsigned char m_minorVersion;           // 0 for WPG 8.0 files
	unsigned int  m_encryptionKey;          // 0 when not encrypted
	unsigned int  m_startOfPacketData;      // unused, since according to the docs no packets are defined
	unsigned char m_entryCount;             // number of entries in extension
	unsigned char m_resourceComplete;       // resource completeness indicator
	unsigned int  m_encryptionBlockOffset;  // encryption block offset
	unsigned long m_fileSize;               // size of the entire wpg file
	unsigned int  m_encryptVersion;         // encryption version information
};

#endif // WPGHEADER
