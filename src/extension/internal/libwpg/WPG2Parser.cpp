/* libwpg
 * Copyright (C) 2006 Ariya Hidayat (ariya@kde.org)
 * Copyright (C) 2005 Fridrich Strba (fridrich.strba@bluewin.ch)
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

#include "WPG2Parser.h"
#include "WPGPaintInterface.h"
#include "libwpg_utils.h"

#include <math.h>
#include <vector>

// MSVC++ 6.0 does not have the macro defined, so we define it
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const unsigned char defaultWPG2PaletteRed[] = {
	0x00, 0xFF, 0x7F, 0xBF, 0x00, 0x00, 0x00, 0x7F,
	0x7F, 0x7F, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	0x7B, 0x91, 0xA7, 0xBD, 0xD3, 0xE9, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x7B, 0x91, 0xA7, 0xBD, 0xD3, 0xE9, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x7B, 0x91, 0xA7, 0xBD, 0xD3, 0xE9, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x3D, 0x48, 0x53, 0x5E, 0x69, 0x74, 0x7F, 0x8A,
	0x95, 0xA0, 0xAB, 0xB6, 0xC1, 0xCC, 0xD7, 0xE2,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16,
	0x2C, 0x42, 0x58, 0x6E, 0x84, 0x9A, 0xB0, 0xC6,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16,
	0x2C, 0x42, 0x58, 0x6E, 0x84, 0x9A, 0xB0, 0xC6,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16,
	0x2C, 0x42, 0x58, 0x6E, 0x84, 0x9A, 0xB0, 0xC6,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16,
	0x2C, 0x42, 0x58, 0x6E, 0x84, 0x9A, 0xB0, 0xC6,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16,
	0x2C, 0x42, 0x58, 0x6E, 0x84, 0x9A, 0xB0, 0xC6,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16,
	0x2C, 0x42, 0x58, 0x6E, 0x84, 0x9A, 0xB0, 0xC6,
	0x3D, 0x48, 0x53, 0x5E, 0x69, 0x74, 0x7F, 0x8A,
	0x95, 0xA0, 0xAB, 0xB6, 0xC1, 0xCC, 0xD7, 0xE2,
	0x7B, 0x91, 0xA7, 0xBD, 0xD3, 0xE4, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x7B, 0x91, 0xA7, 0xBD, 0xD3, 0xE4, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x33, 0x47, 0x61, 0x73, 0x87, 0x9C, 0xB0, 0xC7,
	0xCC, 0xD4, 0xDB, 0xE3, 0xE8, 0xF0, 0xF7, 0xFF,
};

static const unsigned char defaultWPG2PaletteGreen[] = {
	0x00, 0xFF, 0x7F, 0xBF, 0x00, 0x7F, 0x7F, 0x00,
	0x00, 0x7F, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16,
	0x2C, 0x42, 0x58, 0x6E, 0x84, 0x9A, 0xB0, 0xC6,
	0x3D, 0x48, 0x53, 0x5E, 0x69, 0x74, 0x7F, 0x8A,
	0x95, 0xA0, 0xAB, 0xB6, 0xC1, 0xCC, 0xD7, 0xE2,
	0x7B, 0x91, 0xA7, 0xBD, 0xD3, 0xE9, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x7B, 0x91, 0xA7, 0xBD, 0xD3, 0xE9, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x7B, 0x91, 0xA7, 0xBD, 0xD3, 0xE9, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x7B, 0x91, 0xA7, 0xBD, 0xD3, 0xE9, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x7B, 0x91, 0xA7, 0xBD, 0xD3, 0xE9, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x56, 0x64, 0x72, 0x80, 0x8E, 0x9C, 0xAA, 0xB1,
	0xB8, 0xBF, 0xC6, 0xCD, 0xD4, 0xDB, 0xE2, 0xE9,
	0x2B, 0x32, 0x39, 0x40, 0x47, 0x4E, 0x55, 0x63,
	0x71, 0x7F, 0x8D, 0x9B, 0xA9, 0xB7, 0xC5, 0xD3,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16,
	0x2C, 0x42, 0x58, 0x6E, 0x84, 0x9A, 0xB0, 0xC6,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16,
	0x2C, 0x42, 0x58, 0x6E, 0x84, 0x9A, 0xB0, 0xC6,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16,
	0x2C, 0x42, 0x58, 0x6E, 0x84, 0x9A, 0xB0, 0xC6,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16,
	0x2C, 0x42, 0x58, 0x6E, 0x84, 0x9A, 0xB0, 0xC6,
	0x29, 0x38, 0x45, 0x4F, 0x5C, 0x63, 0x69, 0xD4,
	0x87, 0x8F, 0x9C, 0xA8, 0xB3, 0xC4, 0xCF, 0xE0,
};

static const unsigned char defaultWPG2PaletteBlue[] = {
	0x00, 0xFF, 0x7F, 0xBF, 0x7F, 0x00, 0x7F, 0x00,
	0x7F, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16,
	0x2C, 0x42, 0x58, 0x6E, 0x84, 0x9A, 0xB0, 0xC6,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16,
	0x2C, 0x42, 0x58, 0x6E, 0x84, 0x9A, 0xB0, 0xC6,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16,
	0x2C, 0x42, 0x58, 0x6E, 0x84, 0x9A, 0xB0, 0xC6,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16,
	0x2C, 0x42, 0x58, 0x6E, 0x84, 0x9A, 0xB0, 0xC6,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16,
	0x2C, 0x42, 0x58, 0x6E, 0x84, 0x9A, 0xB0, 0xC6,
	0x3D, 0x48, 0x53, 0x5E, 0x69, 0x74, 0x7F, 0x8A,
	0x95, 0xA0, 0xAB, 0xB6, 0xC1, 0xCC, 0xD7, 0xE2,
	0x7B, 0x91, 0xA7, 0xBD, 0xD3, 0xE9, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x7B, 0x91, 0xA7, 0xB0, 0xD3, 0xE4, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x7B, 0x91, 0xA7, 0xBD, 0xD3, 0xE4, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x7B, 0x91, 0xA7, 0xBD, 0xD3, 0xE4, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x7B, 0x91, 0xA7, 0xBD, 0xD3, 0xE4, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x7B, 0x91, 0xA7, 0xBD, 0xD3, 0xE4, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x3D, 0x48, 0x53, 0x5E, 0x69, 0x74, 0x7F, 0x8A,
	0x95, 0xA0, 0xAB, 0xB6, 0xC1, 0xCC, 0xD7, 0xE2,
	0x11, 0x17, 0x1C, 0x24, 0x29, 0x2B, 0x2B, 0x30,
	0x47, 0x57, 0x69, 0x78, 0x8C, 0x9C, 0xB0, 0xC7,
};

class WPG2Parser::ObjectCharacterization
{
public:
	bool taper;
	bool translate;
	bool skew;
	bool scale;
	bool rotate;
	bool hasObjectId;
	bool editLock;
	bool windingRule;
	bool filled;
	bool closed;
	bool framed;

	unsigned long objectId;
	unsigned long lockFlags;
	long rotationAngle;
	long sxcos;
	long sycos;
	long kxsin;
	long kysin;
	long txinteger;
	short txfraction;
	long tyinteger;
	short tyfraction;
	long px;
	long py;

	WPG2TransformMatrix matrix;

	ObjectCharacterization():
		taper(false),
		translate(false),
		skew(false),
		scale(false),
		rotate(false),
		hasObjectId(false),
		editLock(false),
		objectId(0),
		lockFlags(0),
		windingRule(false),
		filled(false),
		closed(false),
		framed(true),
		rotationAngle(0),
		sxcos(0),
		sycos(0),
		kxsin(0),
		kysin(0),
		txinteger(0),
		txfraction(0),
		tyinteger(0),
		tyfraction(0),
		px(0),
		py(0)
			{}
};

WPG2Parser::WPG2Parser(WPGInputStream *input, WPGPaintInterface* painter):
	WPGXParser(input, painter),
	m_success(true), m_exit(false),
	m_xres(1200), m_yres(1200),
	m_xofs(0), m_yofs(0),
	m_width(0), m_height(0),
	m_doublePrecision(false),
	m_layerOpened(false), m_layerId(0),
	m_subIndex(0)
{
}

bool WPG2Parser::parse()
{
	typedef void (WPG2Parser::*Method)();

	struct RecordHandler
	{
		int type;
		const char *name;
		Method handler;
	};

	static const struct RecordHandler handlers[] =
	{
		{ 0x01, "Start WPG",            &WPG2Parser::handleStartWPG },	 
		{ 0x02, "End WPG",              &WPG2Parser::handleEndWPG },	 
		{ 0x03, "Form Settings",        0 },     // ignored
		{ 0x04, "Ruler Settings",       0 },     // ignored
		{ 0x05, "Grid Settings",        0 },     // ignored
		{ 0x06, "Layer",                &WPG2Parser::handleLayer },
		{ 0x08, "Pen Style Definition", &WPG2Parser::handlePenStyleDefinition },
		{ 0x09, "Pattern Definition",   0 },
		{ 0x0a, "Comment",              0 },     // ignored
		{ 0x0b, "Color Transfer",       0 },
		{ 0x0c, "Color Palette",        &WPG2Parser::handleColorPalette },
		{ 0x0d, "DP Color Palette",     &WPG2Parser::handleDPColorPalette },
		{ 0x0e, "Bitmap Data",          0 },
		{ 0x0f, "Text Data",            0 },
		{ 0x10, "Chart Style",          0 },     // ignored
		{ 0x11, "Chart Data",           0 },     // ignored
		{ 0x12, "Object Image",         0 },
		{ 0x15, "Polyline",             &WPG2Parser::handlePolyline },
		{ 0x16, "Polyspline",           0 },
		{ 0x17, "Polycurve",            &WPG2Parser::handlePolycurve },
		{ 0x18, "Rectangle",            &WPG2Parser::handleRectangle },
		{ 0x19, "Arc",                  &WPG2Parser::handleArc },
		{ 0x1a, "Compound Polygon",     &WPG2Parser::handleCompoundPolygon },
		{ 0x1b, "Bitmap",               0 },
		{ 0x1c, "Text Line",            0 },
		{ 0x1d, "Text Block",           0 },
		{ 0x1e, "Text Path",            0 },
		{ 0x1f, "Chart",                0 },
		{ 0x20, "Group",                0 },
		{ 0x21, "Object Capsule",       0 },
		{ 0x22, "Font Settings",        0 },
		{ 0x25, "Pen Fore Color",       &WPG2Parser::handlePenForeColor },
		{ 0x26, "DP Pen Fore Color",    &WPG2Parser::handleDPPenForeColor },
		{ 0x27, "Pen Back Color",       &WPG2Parser::handlePenBackColor },
		{ 0x28, "DP Pen Back Color",    &WPG2Parser::handleDPPenBackColor },
		{ 0x29, "Pen Style",            &WPG2Parser::handlePenStyle },
		{ 0x2a, "Pen Pattern",          0 },
		{ 0x2b, "Pen Size",             &WPG2Parser::handlePenSize },
		{ 0x2c, "DP Pen Size",          &WPG2Parser::handleDPPenSize  },
		{ 0x2d, "Line Cap",             0 },
		{ 0x2e, "Line Join",            0 },
		{ 0x2f, "Brush Gradient",       &WPG2Parser::handleBrushGradient },
		{ 0x30, "DP Brush Gradient",    &WPG2Parser::handleDPBrushGradient },
		{ 0x31, "Brush Fore Color",     &WPG2Parser::handleBrushForeColor },
		{ 0x32, "DP Brush Fore Color",  &WPG2Parser::handleDPBrushForeColor },
		{ 0x33, "Brush Back Color",     &WPG2Parser::handleBrushBackColor },
		{ 0x34, "DP Brush Back Color",  &WPG2Parser::handleDPBrushBackColor },
		{ 0x35, "Brush Pattern",        &WPG2Parser::handleBrushPattern },
		{ 0x36, "Horizontal Line",      0 },
		{ 0x37, "Vertical Line",        0 },
		{ 0x38, "Poster Settings",      0 },
		{ 0x39, "Image State",          0 },
		{ 0x3a, "Envelope Definition",  0 },
		{ 0x3b, "Envelope",             0 },
		{ 0x3c, "Texture Definition",   0 },
		{ 0x3d, "Brush Texture",        0 },
		{ 0x3e, "Texture Alignment",    0 },
		{ 0x3f, "Pen Texture ",         0 },
		{ 0x00, 0, 0 } // end marker
	};	 

	// initialization
	m_success = true;
	m_exit = false;
	m_xres = m_yres = 1200;
	m_doublePrecision = false;
	m_layerOpened = false;
	m_matrix = WPG2TransformMatrix();
	m_subIndex = 0;
	while(!m_indexStack.empty())
		m_indexStack.pop();
	
	// default style
	m_pen.foreColor = WPGColor(0,0,0);
	m_pen.backColor = WPGColor(0,0,0);
	m_pen.width = 0.001;
	m_pen.height = 0.001;
	m_pen.solid = true;
	m_pen.dashArray = WPGDashArray();
	m_brush.foreColor = WPGColor(0,0,0);
	m_brush.backColor = WPGColor(0,0,0);
	resetPalette();

	while(!m_input->atEnd())
	{
		long recordPos = m_input->tell();
		int recordClass = readU8();
		int recordType = readU8();
		int extension = readVariableLengthInteger();
		int length = readVariableLengthInteger();
		long nextPos = m_input->tell() + length;
		
		// inside a subgroup, one less sub record
		if(m_subIndex > 0)
			m_subIndex--;

		// search function to handler this record
		int index = -1;
		for(int i = 0; (index < 0) && handlers[i].name; i++)
			if(handlers[i].type == recordType)
				index = i;
				
		WPG_DEBUG_MSG(("\n"));
		if(index < 0)
		{
			WPG_DEBUG_MSG(("Unknown record type 0x%02x at %d  size %d  extension %d\n", 
				recordType, recordPos, length, extension));
		}
		else
		{
			Method recordHandler = handlers[index].handler;
			
			if(!recordHandler)
				WPG_DEBUG_MSG(("Record '%s' (ignored) type 0x%02x at %d  size %d  extension %d\n",
					handlers[index].name, recordType, recordPos, length, extension));
			else
			{
				WPG_DEBUG_MSG(("Record '%s' type 0x%02x at %d  size %d  extension %d\n", 
					handlers[index].name, recordType, recordPos, length, extension));
					
				// invoke the handler for this record
				(this->*recordHandler)();
			}
		}

		// we enter another subgroup, save the index to stack
		if(extension > 0)
		{
			m_indexStack.push(m_subIndex);
			m_subIndex = extension;
		}

		//if(m_input->tell() > nextPos)
		{
			//WPG_DEBUG_MSG(("Record 0x%x consumes more bytes than necessary!\n", recordType));
			WPG_DEBUG_MSG(("Current stream position: %d\n", m_input->tell()));
		}
	
		if(m_exit) break;
		
		m_input->seek(nextPos);
	}

	return m_success;
}

static const char* describePrecision(unsigned char precision)
{
	const char* result = "Unknown";
	switch(precision)
	{
		case 0: result = "single"; break;
		case 1: result = "double"; break;
		default: break;
	}
	return result;
}
	
static const char* describeGradient(unsigned char gradientType)
{
	const char* result = "Unknown";
	switch(gradientType)
	{
		case 0: result = "None"; break;
		case 1: result = "Linear"; break;
		case 2: result = "Polygonal"; break;
		case 3: result = "Concentric Circles"; break;
		case 4: result = "Convergent Circles"; break;
		case 5: result = "Concentric Ellipses"; break;
		case 6: result = "Convergent Ellipses"; break; 
		case 7: result = "Concentric Squares"; break; 
		case 8: result = "Convergent Squares"; break; 
		case 9: result = "Concentric Rectangles"; break; 
		case 10: result = "Convergent Rectangles"; break;
		default: break;
	}
	return result;
}

#define TO_DOUBLE(x) ( (m_doublePrecision) ? ((double)(x)/65536.0) : (double)(x) )
#define TRANSFORM_XY(x,y) { m_matrix.transform((x),(y)); (x)-= m_xofs; (y)-= m_yofs; (y)=m_height-(y); }

void WPG2Parser::handleStartWPG()
{
	unsigned int horizontalUnit = readU16();
	unsigned int verticalUnit = readU16();
	unsigned char precision = readU8();
	
	// sanity check
	m_xres = horizontalUnit;
	m_yres = verticalUnit;
	if((horizontalUnit==0) || (verticalUnit==0))
	{
		m_xres = m_yres = 1200;
		WPG_DEBUG_MSG(("Warning ! Insane unit of measure"));
	}
	
	// danger if we do not recognize the precision code
	if(precision != 0)
	if(precision != 1)
	{
		m_success = false;
		m_exit = true;
		return;
	}
	m_doublePrecision = (precision == 1);
	
	long viewportX1 = (m_doublePrecision) ? readS32() : readS16();
	long viewportY1 = (m_doublePrecision) ? readS32() : readS16();
	long viewportX2 = (m_doublePrecision) ? readS32() : readS16();
	long viewportY2 = (m_doublePrecision) ? readS32() : readS16();
	
	long imageX1 = (m_doublePrecision) ? readS32() : readS16();
	long imageY1 = (m_doublePrecision) ? readS32() : readS16();
	long imageX2 = (m_doublePrecision) ? readS32() : readS16();
	long imageY2 = (m_doublePrecision) ? readS32() : readS16();

	// used to adjust coordinates	
	m_xofs = (imageX1 < imageX2) ? imageX1 : imageX2;
	m_yofs = (imageY1 < imageY2) ? imageY1 : imageX2;
	m_width = (imageX2 > imageX1 ) ? imageX2-imageX1 : imageX1-imageX2;
	m_height = (imageY2 > imageY1) ? imageY2-imageY1 : imageY1-imageY2;
	
	WPG_DEBUG_MSG(("StartWPG\n"));
	WPG_DEBUG_MSG(("  Horizontal unit of measure : %d pixels/inch\n", horizontalUnit));
	WPG_DEBUG_MSG(("    Vertical unit of measure : %d pixels/inch\n", verticalUnit));
	WPG_DEBUG_MSG(("              Data precision : %d (%s)\n", precision, describePrecision(precision)));
	WPG_DEBUG_MSG(("                 Viewport X1 : %d\n", viewportX1));
	WPG_DEBUG_MSG(("                 Viewport Y1 : %d\n", viewportY1));
	WPG_DEBUG_MSG(("                 Viewport X2 : %d\n", viewportX2));
	WPG_DEBUG_MSG(("                 Viewport Y2 : %d\n", viewportY2));
	WPG_DEBUG_MSG(("                    Image X1 : %d\n", imageX1));
	WPG_DEBUG_MSG(("                    Image Y1 : %d\n", imageY1));
	WPG_DEBUG_MSG(("                    Image X2 : %d\n", imageX2));
	WPG_DEBUG_MSG(("                    Image Y2 : %d\n", imageY2));
	WPG_DEBUG_MSG(("                    X offset : %d\n", m_xofs));
	WPG_DEBUG_MSG(("                    Y offset : %d\n", m_yofs));
	WPG_DEBUG_MSG(("                       width : %d\n", m_width));
	WPG_DEBUG_MSG(("                      height : %d\n", m_height));
	
	double width = (TO_DOUBLE(m_width)) / m_xres;
	double height = (TO_DOUBLE(m_height)) / m_yres;
	
	m_painter->startDocument(width, height);

	static const int WPG2_defaultPenDashes[] = {
		1,   291,  0,                            // style #0 (actually solid)
		1,   218, 73,                            // style #1
		1,   145, 73,                            // style #2
		1,    73, 73,                            // style #3
		1,    36, 36,                            // style #4
		1,    18, 18,                            // style #5
		1,    18, 55,                            // style #6
		3,    18, 55,  18, 55, 18, 127,          // style #7
		2,   164, 55,  18, 55,                   // style #8
		3,   145, 36, 138, 36, 18,  36,          // style #9
		3,    91, 55,  91, 55, 18,  55,          // style #10
		4,    91, 36,  91, 36, 18,  36, 18, 36,  // style #11
		2,   182, 73,  73, 73,                   // style #12
		3,   182, 36,  55, 36, 55,  36,          // style #13
		3,   255, 73, 255, 73, 73,  73,          // style #14
		4,   273, 36, 273, 36, 55,  36, 55, 36,  // style #15
		0 // end marker
	};
	
	// create default pen styles
	int styleNo = 0;
	for(int i = 0; i < sizeof(WPG2_defaultPenDashes)/sizeof(WPG2_defaultPenDashes[0]);)
	{
		int segments = 2 * WPG2_defaultPenDashes[i++];
		if(segments == 0) break;
		WPGDashArray dashArray;
		for(int j = 0; j < segments; j++, i++)
			dashArray.add(WPG2_defaultPenDashes[i]*3.6/218.0);
		m_penStyles[styleNo] = dashArray;
		styleNo++; 
	}
}

void WPG2Parser::handleEndWPG()
{
	// sentinel
	if(m_layerOpened)
		m_painter->endLayer(m_layerId);

	m_painter->endDocument();
	m_exit = true;
	
	WPG_DEBUG_MSG(("EndWPG\n"));
}

void WPG2Parser::handleLayer()
{
	m_layerId = readU16();

	// close previous one
	if(m_layerOpened)
		m_painter->endLayer(m_layerId);

	m_painter->startLayer(m_layerId);
	m_layerOpened = true;

	WPG_DEBUG_MSG(("Layer\n"));
	WPG_DEBUG_MSG(("  Id: %d\n", m_layerId));
}

void WPG2Parser::handleCompoundPolygon()
{
	ObjectCharacterization objCh;
	parseCharacterization(&objCh);
}

void WPG2Parser::handlePenStyleDefinition()
{
	unsigned int style = readU16();
	unsigned int segments = readU16();

	WPGDashArray dashArray;
	for(int i = 0; i < segments; i++)
	{
		unsigned int p = (m_doublePrecision) ? readU32() : readU16();
		unsigned int q = (m_doublePrecision) ? readU32() : readU16();
		dashArray.add(TO_DOUBLE(p)*3.6/218.0);
		dashArray.add(TO_DOUBLE(q)*3.6/218.0);
	}
	m_penStyles[style] = dashArray;

	WPG_DEBUG_MSG(("PenStyleDefinition\n"));
	WPG_DEBUG_MSG(("          Style : %d\n", style));
	WPG_DEBUG_MSG(("  Segment pairs : %d\n", segments));
}

// TODO
void WPG2Parser::handlePatternDefinition()
{
	WPG_DEBUG_MSG(("PatternDefinition\n"));
}

void WPG2Parser::handleColorPalette()
{
	unsigned startIndex = readU16();
	unsigned numEntries = readU16();

	WPG_DEBUG_MSG(("Color Palette\n"));
	for(int i = 0; i < numEntries; i++)
	{
		WPGColor color;
		color.red = readU8();
		color.green = readU8();
		color.blue = readU8();
		color.alpha = readU8();
		m_colorPalette[startIndex+i] = color;
		WPG_DEBUG_MSG(("Index#%d: RGB %d %d %d\n", startIndex+i, color.red, color.green, color.blue));
	}
}

void WPG2Parser::handleDPColorPalette()
{
	unsigned startIndex = readU16();
	unsigned numEntries = readU16();

	WPG_DEBUG_MSG(("Color Palette\n"));
	for(int i = 0; i < numEntries; i++)
	{
		WPGColor color;
		color.red = readU16() >> 8 ;
		color.green = readU16() >> 8 ;
		color.blue = readU16() >> 8 ;
		color.alpha = readU16() >> 8 ;
		m_colorPalette[startIndex+i] = color;
		WPG_DEBUG_MSG(("Index#%d: RGB %d %d %d\n", startIndex+i, color.red, color.green, color.blue));
	}
}

void WPG2Parser::handlePenForeColor()
{
	unsigned char red = readU8();
	unsigned char green = readU8();
	unsigned char blue = readU8();
	unsigned char alpha = readU8();

	m_pen.foreColor = WPGColor(red, green, blue, alpha);

	WPG_DEBUG_MSG(("PenForeColor\n"));
	WPG_DEBUG_MSG(("   Foreground color (RGBA): %d %d %d %d\n", red, green, blue, alpha));
}

void WPG2Parser::handleDPPenForeColor()
{
	// we just ignore the least significant 8 bits
	unsigned int red = (m_doublePrecision)   ? readU16()>>8 : readU8();
	unsigned int green = (m_doublePrecision) ? readU16()>>8 : readU8();
	unsigned int blue = (m_doublePrecision)  ? readU16()>>8 : readU8();
	unsigned int alpha = (m_doublePrecision) ? readU16()>>8 : readU8();

	m_pen.foreColor = WPGColor(red, green, blue, alpha);

	WPG_DEBUG_MSG(("PenForeColor\n"));
	WPG_DEBUG_MSG(("   Foreground color (RGBA): %d %d %d %d\n", red, green, blue, alpha));
}

void WPG2Parser::handlePenBackColor()
{
	unsigned char red = readU8();
	unsigned char green = readU8();
	unsigned char blue = readU8();
	unsigned char alpha = readU8();

	m_pen.backColor = WPGColor(red, green, blue, alpha);

	WPG_DEBUG_MSG(("PenBackColor\n"));
	WPG_DEBUG_MSG(("   Background color (RGBA): %d %d %d %d\n", red, green, blue, alpha));
}

void WPG2Parser::handleDPPenBackColor()
{
	// we just ignore the least significant 8 bits
	unsigned int red = (m_doublePrecision)   ? readU16()>>8 : readU8();
	unsigned int green = (m_doublePrecision) ? readU16()>>8 : readU8();
	unsigned int blue = (m_doublePrecision)  ? readU16()>>8 : readU8();
	unsigned int alpha = (m_doublePrecision) ? readU16()>>8 : readU8();

	m_pen.backColor = WPGColor(red, green, blue, alpha);

	WPG_DEBUG_MSG(("PenBackColor\n"));
	WPG_DEBUG_MSG(("   Background color (RGBA): %d %d %d %d\n", red, green, blue, alpha));
}

void WPG2Parser::handlePenStyle()
{
	unsigned int style = readU16();

	m_pen.dashArray = m_penStyles[style];
	m_pen.solid = (style == 0);

	WPG_DEBUG_MSG(("PenStyle\n"));
	WPG_DEBUG_MSG(("   Pen style : %d\n", style));
	WPG_DEBUG_MSG(("   Segments : %d\n", m_pen.dashArray.count()));
}

void WPG2Parser::handlePenSize()
{
	unsigned int width = readU16();
	unsigned int height = readU16();

	m_pen.width = TO_DOUBLE(width) / m_xres;
	m_pen.height = TO_DOUBLE(height) / m_yres;

	WPG_DEBUG_MSG(("PenSize\n"));
	WPG_DEBUG_MSG(("   Width: %d\n", width));
	WPG_DEBUG_MSG(("   Height: %d\n", height));
}

void WPG2Parser::handleDPPenSize()
{
	unsigned long width = readU32();
	unsigned long height = readU32();

	m_pen.width = TO_DOUBLE(width) / m_xres / 256;
	m_pen.height = TO_DOUBLE(height) / m_yres / 256;

	WPG_DEBUG_MSG(("PenSize\n"));
	WPG_DEBUG_MSG(("   Width: %d\n", width));
	WPG_DEBUG_MSG(("   Height: %d\n", height));
}

void WPG2Parser::handleBrushGradient()
{
	unsigned angleFraction = readU16();
	unsigned angleInteger = readU16();
	unsigned xref = readU16();
	unsigned yref = readU16();
	unsigned flag = readU16();
	bool granular = flag & (1<<6);
	bool anchor = flag & (1<<7);
	
	// TODO: get gradient extent
	
	m_gradientAngle = angleInteger + (double)angleFraction/65536.0;
	m_gradientRef.x = xref;
	m_gradientRef.y = yref;
	
	WPG_DEBUG_MSG(("       Gradient angle : %d.%d\n", angleInteger, angleFraction));
	WPG_DEBUG_MSG(("   Gradient reference : %d.%d\n", xref, yref));
	WPG_DEBUG_MSG(("   Granular : %s\n", (granular ? "yes" : "no")));
	WPG_DEBUG_MSG(("   Anchored : %s\n", (anchor ? "yes" : "no")));
}

void WPG2Parser::handleDPBrushGradient()
{
	unsigned angleFraction = readU16();
	unsigned angleInteger = readU16();
	unsigned xref = readU16();
	unsigned yref = readU16();
	unsigned flag = readU16();
	bool granular = flag & (1<<6);
	bool anchor = flag & (1<<7);
	
	// TODO: get gradient extent (in double precision)
	
	m_gradientAngle = angleFraction + (double)angleInteger/65536.0;
	m_gradientRef.x = xref;
	m_gradientRef.y = yref;

	WPG_DEBUG_MSG(("       Gradient angle : %d.%d\n", angleInteger, angleFraction));
	WPG_DEBUG_MSG(("   Gradient reference : %d.%d\n", xref, yref));
	WPG_DEBUG_MSG(("   Granular : %s\n", (granular ? "yes" : "no")));
	WPG_DEBUG_MSG(("   Anchored : %s\n", (anchor ? "yes" : "no")));
}

void WPG2Parser::handleBrushForeColor()
{
	unsigned char gradientType = readU8();
	WPG_DEBUG_MSG(("   Gradient type : %d (%s)\n", gradientType, describeGradient(gradientType)));

	if(gradientType == 0)
	{
		unsigned char red = readU8();
		unsigned char green = readU8();
		unsigned char blue = readU8();
		unsigned char alpha = readU8();
		WPG_DEBUG_MSG(("   Foreground color (RGBA): %d %d %d %d\n", red, green, blue, alpha));

		m_brush.foreColor = WPGColor(red, green, blue, alpha);
		if(m_brush.style == WPGBrush::NoBrush)
			m_brush.style = WPGBrush::Solid;
	}
	else
	{
		unsigned count = readU16();
		std::vector<WPGColor> colors;
		std::vector<double> positions;
		WPG_DEBUG_MSG(("  Gradient colors : %d\n", count));

		for(unsigned i = 0; i < count; i++)
		{
			unsigned char red = readU8();
			unsigned char green = readU8();
			unsigned char blue = readU8();
			unsigned char alpha = readU8();
			WPGColor color(red, green, blue, alpha);
			colors.push_back(color);
			WPG_DEBUG_MSG(("   Color #%d (RGBA): %d %d %d %d\n", i+1, red, green, blue, alpha));
		}

		for(unsigned j = 0; j < count-1; j++)
		{
			unsigned pos = readU16();
			positions.push_back(TO_DOUBLE(pos));
			WPG_DEBUG_MSG(("   Position #%d : %d\n", j+1, pos));
		}
		
		// looks like Corel Presentations only create 2 colors gradient
		// and they are actually in reverse order
		if(count == 2)
		{
			double xref = (double)m_gradientRef.x/65536.0;
			double yref = (double)m_gradientRef.y/65536.0;
			double angle = m_gradientAngle*M_PI/180.0;
			double tanangle = tan(angle);
			double ref = (tanangle<1e2) ? (yref+xref*tanangle)/(1+tanangle) : xref;
			WPGGradient gradient;
			gradient.setAngle(-m_gradientAngle); // upside down
			gradient.addStop(0, colors[1]);
			gradient.addStop(ref, colors[0]);
			if((m_gradientRef.x != 65535) && (m_gradientRef.y != 65536))
				gradient.addStop(1, colors[1]);
			m_brush.gradient = gradient;
			m_brush.style = WPGBrush::Gradient;
		}
	}
}

void WPG2Parser::handleDPBrushForeColor()
{
	unsigned char gradientType = readU8();

	// we just ignore the least significant 8 bits
	unsigned int red = (m_doublePrecision)   ? readU16()>>8 : readU8();
	unsigned int green = (m_doublePrecision) ? readU16()>>8 : readU8();
	unsigned int blue = (m_doublePrecision)  ? readU16()>>8 : readU8();
	unsigned int alpha = (m_doublePrecision) ? readU16()>>8 : readU8();

	m_brush.foreColor = WPGColor(red, green, blue, alpha);
	if(m_brush.style == WPGBrush::NoBrush)
		m_brush.style = WPGBrush::Solid;

	WPG_DEBUG_MSG(("BrushForeColor\n"));
	WPG_DEBUG_MSG(("   Gradient type : %d (%s)\n", gradientType, describeGradient(gradientType)));
	WPG_DEBUG_MSG(("   Foreground color (RGBA): %d %d %d %d\n", red, green, blue, alpha));
}

void WPG2Parser::handleBrushBackColor()
{
	unsigned char red = readU8();
	unsigned char green = readU8();
	unsigned char blue = readU8();
	unsigned char alpha = readU8();

	m_brush.backColor = WPGColor(red, green, blue, alpha);
	if(m_brush.style == WPGBrush::NoBrush)
		m_brush.style = WPGBrush::Solid;

	WPG_DEBUG_MSG(("BrushBackColor\n"));
	WPG_DEBUG_MSG(("   Backround color (RGBA): %d %d %d %d\n", red, green, blue, alpha));
}

void WPG2Parser::handleDPBrushBackColor()
{
	// we just ignore the least significant 8 bits
	unsigned int red = (m_doublePrecision)   ? readU16()>>8 : readU8();
	unsigned int green = (m_doublePrecision) ? readU16()>>8 : readU8();
	unsigned int blue = (m_doublePrecision)  ? readU16()>>8 : readU8();
	unsigned int alpha = (m_doublePrecision) ? readU16()>>8 : readU8();

	m_brush.backColor = WPGColor(red, green, blue, alpha);
	if(m_brush.style == WPGBrush::NoBrush)
		m_brush.style = WPGBrush::Solid;

	WPG_DEBUG_MSG(("PenBackColor\n"));
	WPG_DEBUG_MSG(("   Background color (RGBA): %d %d %d %d\n", red, green, blue, alpha));
}

void WPG2Parser::handleBrushPattern()
{
	unsigned int pattern = readU16();

	// TODO

	WPG_DEBUG_MSG(("BrushPattern\n"));
	WPG_DEBUG_MSG(("   Pattern : %d\n", pattern));
}

void WPG2Parser::parseCharacterization(ObjectCharacterization* ch)
{
	// sanity check
	if(!ch) return;

	// identity
	ch->matrix = WPG2TransformMatrix();

	unsigned int flags = readU16();
	ch->taper = (flags & 0x01) != 0;
	ch->translate = (flags & 0x02) != 0;
	ch->skew = (flags & 0x04) != 0;
	ch->scale = (flags & 0x08) != 0;
	ch->rotate = (flags & 0x10) != 0;
	ch->hasObjectId = (flags & 0x20) != 0;
	ch->editLock = (flags & 0x80) != 0;
	ch->windingRule = (flags & (1<<12)) != 0;
	ch->filled = (flags & (1<<13)) != 0;
	ch->closed = (flags & (1<<14)) != 0;
	ch->framed = (flags & (1<<15)) != 0;
	
	if(ch->editLock) ch->lockFlags = readU32();

	// object ID can be 2 or 4 bytes
	if(ch->hasObjectId) ch->objectId = readU16();
	if(ch->objectId >> 15) ch->objectId = ((ch->objectId  & 0x7fff) << 16) | readU16();

	if(ch->rotate) ch->rotationAngle = readS32();

	if(ch->rotate || ch->scale)
	{
		ch->sxcos = readS32();
		ch->sycos = readS32();
		ch->matrix.element[0][0] = (double)(ch->sxcos)/65536;
		ch->matrix.element[1][1] = (double)(ch->sxcos)/65536;
	}

	if(ch->rotate || ch->skew)
	{
		ch->kxsin = readS32();
		ch->kysin = readS32();
		ch->matrix.element[1][0] = (double)(ch->kxsin)/65536;
		ch->matrix.element[0][1] = (double)(ch->kysin)/65536;
	}

	if(ch->translate)
	{
		ch->txfraction = readU16();
		ch->txinteger = readS32();
		ch->tyfraction = readU16();
		ch->tyinteger = readS32();
		ch->matrix.element[2][0] = (double)(ch->txinteger);
		ch->matrix.element[2][1] = (double)(ch->tyinteger);
	}

	if(ch->taper)
	{
		ch->px = readS32();
		ch->py = readS32();
		ch->matrix.element[0][2] = (double)(ch->px);
		ch->matrix.element[1][2] = (double)(ch->py);
	}

	WPG_DEBUG_MSG(("ObjectCharacterization\n"));
	WPG_DEBUG_MSG(("       taper : %s\n", (ch->taper ? "yes" : "no")));
	WPG_DEBUG_MSG(("   translate : %s\n", (ch->translate ? "yes" : "no")));
	WPG_DEBUG_MSG(("        skew : %s\n", (ch->skew ? "yes" : "no")));
	WPG_DEBUG_MSG(("       scale : %s\n", (ch->scale ? "yes" : "no")));
	WPG_DEBUG_MSG(("      rotate : %s\n", (ch->rotate ? "yes" : "no")));
	WPG_DEBUG_MSG((" hasObjectId : %s\n", (ch->hasObjectId ? "yes" : "no")));
	WPG_DEBUG_MSG(("    editLock : %s\n", (ch->editLock ? "yes" : "no")));
	if(ch->editLock) WPG_DEBUG_MSG(("  lock flags : 0x%x\n", ch->lockFlags));
	if(ch->hasObjectId) WPG_DEBUG_MSG(("   object ID : 0x%x\n", ch->objectId));
	if(ch->translate) WPG_DEBUG_MSG(("    tx : %d %d\n", ch->txinteger, ch->txfraction));
	if(ch->translate) WPG_DEBUG_MSG(("    ty : %d %d\n", ch->tyinteger, ch->tyfraction));
	WPG_DEBUG_MSG(("transform matrix:\n"));
	WPG_DEBUG_MSG(("%f %f %f\n", ch->matrix.element[0][0], ch->matrix.element[0][1],ch->matrix.element[0][2]));
	WPG_DEBUG_MSG(("%f %f %f\n", ch->matrix.element[1][0], ch->matrix.element[1][1],ch->matrix.element[1][2]));
	WPG_DEBUG_MSG(("%f %f %f\n", ch->matrix.element[2][0], ch->matrix.element[2][1],ch->matrix.element[2][2]));
}

void WPG2Parser::handlePolyline()
{
	ObjectCharacterization objCh;
	parseCharacterization(&objCh);
	m_matrix = objCh.matrix;

	unsigned long count = readU16();

	WPGPointArray points;
	for(unsigned long i = 0; i < count; i++ )
	{
		long x = (m_doublePrecision) ? readS32() : readS16();
		long y = (m_doublePrecision) ? readS32() : readS16();
		TRANSFORM_XY(x,y);
		WPGPoint p(TO_DOUBLE(x)/m_xres, TO_DOUBLE(y)/m_yres);
		points.add(p);
	}

	m_painter->setBrush( objCh.filled ? m_brush : WPGBrush() );
	m_painter->setPen( objCh.framed ? m_pen : WPGPen() );
	if(objCh.windingRule)
		m_painter->setFillRule(WPGPaintInterface::WindingFill);
	else
		m_painter->setFillRule(WPGPaintInterface::AlternatingFill);
	m_painter->drawPolygon(points);

	WPG_DEBUG_MSG(("Polyline\n"));
	WPG_DEBUG_MSG(("   Vertices count : %d\n", count));
	for(int j = 0; j < count; j++ )
		WPG_DEBUG_MSG(("        Point #%d : %g,%g\n", j+1, points[j].x, points[j].y));
}

void WPG2Parser::handlePolycurve()
{
	ObjectCharacterization objCh;
	parseCharacterization(&objCh);
	m_matrix = objCh.matrix;

	unsigned int count = readU16();

	WPGPointArray vertices;
	WPGPointArray controlPoints;
	for(int i = 0; i < count; i++ )
	{
		long ix = (m_doublePrecision) ? readS32() : readS16();
		long iy = (m_doublePrecision) ? readS32() : readS16();
		TRANSFORM_XY(ix,iy);
		WPGPoint initialPoint( TO_DOUBLE(ix)/m_xres, TO_DOUBLE(iy)/m_yres );

		long ax = (m_doublePrecision) ? readS32() : readS16();
		long ay = (m_doublePrecision) ? readS32() : readS16();
		TRANSFORM_XY(ax,ay);
		WPGPoint anchorPoint( TO_DOUBLE(ax)/m_xres, TO_DOUBLE(ay)/m_yres );

		long tx = (m_doublePrecision) ? readS32() : readS16();
		long ty = (m_doublePrecision) ? readS32() : readS16();
		TRANSFORM_XY(tx,ty);
		WPGPoint terminalPoint( TO_DOUBLE(tx)/m_xres, TO_DOUBLE(ty)/m_yres );

		vertices.add(anchorPoint);
		if(i > 0)
			controlPoints.add(initialPoint);
		controlPoints.add(terminalPoint);
	}

	WPGPath path;
	path.moveTo(vertices[0]);
	for(unsigned j = 1; j < vertices.count(); j++)
		path.curveTo(controlPoints[j*2-2], controlPoints[j*2-1], vertices[j]);

	m_painter->setBrush( objCh.filled ? m_brush : WPGBrush() );
	m_painter->setPen( objCh.framed ? m_pen : WPGPen() );
	if(objCh.windingRule)
		m_painter->setFillRule(WPGPaintInterface::WindingFill);
	else
		m_painter->setFillRule(WPGPaintInterface::AlternatingFill);
	m_painter->drawPath(path);
}

void WPG2Parser::handleRectangle()
{
	ObjectCharacterization objCh;
	parseCharacterization(&objCh);
	m_matrix = objCh.matrix;
	
	long x1 = (m_doublePrecision) ? readS32() : readS16();
	long y1 = (m_doublePrecision) ? readS32() : readS16();
	TRANSFORM_XY(x1,y1);

	long x2 = (m_doublePrecision) ? readS32() : readS16();
	long y2 = (m_doublePrecision) ? readS32() : readS16();
	TRANSFORM_XY(x2,y2);
	
	long xs1 = (x1 <= x2) ? x1 : x2;
	long xs2 = (x1 <= x2) ? x2 : x1;
	long ys1 = (y1 <= y2) ? y1 : y2;
	long ys2 = (y1 <= y2) ? y2 : y1;

	long rx = (m_doublePrecision) ? readS32() : readS16();
	long ry = (m_doublePrecision) ? readS32() : readS16();

	WPGRect rect;
	rect.x1 = TO_DOUBLE(xs1) / m_xres;
	rect.x2 = TO_DOUBLE(xs2) / m_xres;
	rect.y1 = TO_DOUBLE(ys1) / m_yres;
	rect.y2 = TO_DOUBLE(ys2) / m_yres;
	double roundx = TO_DOUBLE(rx)/m_xres;
	double roundy = TO_DOUBLE(ry)/m_yres;

	m_painter->setBrush( objCh.filled ? m_brush : WPGBrush() );
	m_painter->setPen( objCh.framed ? m_pen : WPGPen() );
	m_painter->drawRectangle(rect, roundx, roundy);
	
	WPG_DEBUG_MSG(("Rectangle\n"));
	WPG_DEBUG_MSG(("      X1 : %d\n", x1));
	WPG_DEBUG_MSG(("      Y1 : %d\n", y1));
	WPG_DEBUG_MSG(("      X2 : %d\n", x2));
	WPG_DEBUG_MSG(("      Y2 : %d\n", y2));
	WPG_DEBUG_MSG((" Round X : %d\n", rx));
	WPG_DEBUG_MSG((" Round Y : %d\n", ry));
}

void WPG2Parser::handleArc()
{
	ObjectCharacterization objCh;
	parseCharacterization(&objCh);
	m_matrix = objCh.matrix;
	
	long cx = (m_doublePrecision) ? readS32() : readS16();
	long cy = (m_doublePrecision) ? readS32() : readS16();

	long radx = (m_doublePrecision) ? readS32() : readS16();
	long rady = (m_doublePrecision) ? readS32() : readS16();

	long ix = (m_doublePrecision) ? readS32() : readS16();
	long iy = (m_doublePrecision) ? readS32() : readS16();
	TRANSFORM_XY(ix,iy);

	long ex = (m_doublePrecision) ? readS32() : readS16();
	long ey = (m_doublePrecision) ? readS32() : readS16();
	TRANSFORM_XY(ex,ey);

	if((ix==ex) && (iy==ey))
	{
		WPGPoint center;
		center.x = TO_DOUBLE(cx) / m_xres;
		center.y = TO_DOUBLE(cy) / m_xres;
		double rx = TO_DOUBLE(radx) / m_xres;
		double ry = TO_DOUBLE(rady) / m_xres;
		
		m_painter->setBrush( objCh.filled ? m_brush : WPGBrush() );
		m_painter->setPen( objCh.framed ? m_pen : WPGPen() );
		m_painter->drawEllipse(center, rx, ry);
	}

	WPG_DEBUG_MSG(("Arc\n"));
	WPG_DEBUG_MSG(("   Center point x : %d\n", cx));
	WPG_DEBUG_MSG(("   Center point y : %d\n", cy));
	WPG_DEBUG_MSG(("         Radius x : %d\n", radx));
	WPG_DEBUG_MSG(("         Radius y : %d\n", rady));
	WPG_DEBUG_MSG(("  Initial point x : %d\n", ix));
	WPG_DEBUG_MSG(("  Initial point y : %d\n", iy));
	WPG_DEBUG_MSG(("      End point x : %d\n", ex));
	WPG_DEBUG_MSG(("      End point y : %d\n", ey));
}


void WPG2Parser::resetPalette()
{
	m_colorPalette.clear();
	for (int i=0; i<256; i++)
	{
		WPGColor color;
		color.red = defaultWPG2PaletteRed[i];
		color.green = defaultWPG2PaletteGreen[i];
		color.blue = defaultWPG2PaletteBlue[i];
		m_colorPalette[i] = color;
	}
}
