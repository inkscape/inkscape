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
#include "WPGSVGGenerator.h"
#include <sstream>

/**
Analyzes the content of an input stream to see if it can be parsed
\param input The input stream
\return A value that indicates whether the content from the input
stream is a WordPerfect Graphics that libwpg is able to parse
*/
bool libwpg::WPGraphics::isSupported(libwpg::WPGInputStream* input)
{
	WPGHeader header;
	if(!header.load(input))
		return false;
	
	return header.isSupported();
}

/**
Parses the input stream content. It will make callbacks to the functions provided by a
WPGPaintInterface class implementation when needed. This is often commonly called the
'main parsing routine'.
\param input The input stream
\param painter A WPGPainterInterface implementation
\return A value that indicates whether the parsing was successful
*/
bool libwpg::WPGraphics::parse(libwpg::WPGInputStream* input, libwpg::WPGPaintInterface* painter)
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

	bool retval;
	switch (header.majorVersion()) {
		case 0x01: // WPG1
			WPG_DEBUG_MSG(("Parsing WPG1\n"));
			parser = new WPG1Parser(input, painter);
			retval = parser->parse();
			break;
		case 0x02: // WPG2
			WPG_DEBUG_MSG(("Parsing WPG2\n"));
			parser = new WPG2Parser(input, painter);
			retval = parser->parse();
			break;
		default: // other :-)
			WPG_DEBUG_MSG(("Unknown format\n"));
			return false;
	}
	
	if (parser)
		delete parser;
	
	return retval;
}

/**
Parses the input stream content and generates a valid Scalable Vector Graphics
Provided as a convenience function for applications that support SVG internally.
\param input The input stream
\param output The output string whose content is the resulting SVG
\return A value that indicates whether the SVG generation was successful.
*/
bool libwpg::WPGraphics::generateSVG(libwpg::WPGInputStream* input, libwpg::WPGString& output)
{
	std::ostringstream tmpOutputStream;
	libwpg::WPGSVGGenerator generator(tmpOutputStream);
	bool result;
	if (result = libwpg::WPGraphics::parse(input, &generator))
		output = WPGString(tmpOutputStream.str().c_str());
	else
		output = WPGString("");
	return result;
}
