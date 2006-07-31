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

#include "WPGraphics.h"
#include "WPGHeader.h"
#include "WPGStream.h"
#include "WPGXParser.h"
#include "WPG1Parser.h"
#include "WPG2Parser.h"
#include "libwpg_utils.h"

using namespace libwpg;

bool WPGraphics::isSupported(WPGInputStream* input)
{
	WPGHeader header;
	if(!header.load(input))
		return false;
	
	return header.isSupported();
}

bool WPGraphics::parse(WPGInputStream* input, WPGPaintInterface* painter)
{
	WPGXParser *parser = 0;
	
	WPG_DEBUG_MSG(("Loading header...\n"));
	WPGHeader header;
	if(!header.load(input))
		return false;
	
	if(!header.isSupported())
	{
		WPG_DEBUG_MSG(("Unsupported file format!\n"));
		return false;  
	}
	
	// seek to the start of document
	input->seek(header.startOfDocument());
	
	switch (header.majorVersion()) {
		case 0x01: // WPG1
			WPG_DEBUG_MSG(("Parsing WPG1\n"));
			parser = new WPG1Parser(input, painter);
			parser->parse();
			break;
		case 0x02: // WPG2
			WPG_DEBUG_MSG(("Parsing WPG2\n"));
			parser = new WPG2Parser(input, painter);
			parser->parse();
			break;
		default: // other :-)
			WPG_DEBUG_MSG(("Unknown format\n"));
			break;
	}
	
	delete parser;
	
	return false;
}
 
