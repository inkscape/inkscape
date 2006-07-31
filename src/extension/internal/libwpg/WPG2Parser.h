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

#ifndef __WPG2PARSER_H__
#define __WPG2PARSER_H__

#include "WPGXParser.h"
#include "WPGBrush.h"
#include "WPGPen.h"

#include <map>
#include <stack>

class WPG2TransformMatrix
{
public:
	double element[3][3];

	WPG2TransformMatrix()
	{
		// identity transformation
		element[0][0] = element[1][1] = 1; element[2][2] = 1;
		element[0][1] = element[0][2] = 0;
		element[1][0] = element[1][2] = 0;
		element[2][0] = element[2][1] = 0;
	}

	void transform(long& x, long& y) const
	{
		long rx = (long)(element[0][0]*x + element[1][0]*y + element[2][0]); 
		long ry = (long)(element[0][1]*x + element[1][1]*y + element[2][1]); 
		x = rx;
		y = ry;
	}

	WPGPoint transform(const WPGPoint& p) const
	{
		WPGPoint point;
		point.x = element[0][0]*p.x + element[1][0]*p.y + element[2][0]; 
		point.y = element[0][1]*p.x + element[1][1]*p.y + element[2][1]; 
		return point;
	}

	WPGRect transform(const WPGRect& r) const
	{
		WPGRect rect;
		rect.x1 = element[0][0]*r.x1 + element[1][0]*r.y1 + element[2][0]; 
		rect.y1 = element[0][1]*r.x1 + element[1][1]*r.y1 + element[2][1]; 
		rect.x2 = element[0][0]*r.x2 + element[1][0]*r.y2 + element[2][0]; 
		rect.y2 = element[0][1]*r.x2 + element[1][1]*r.y2 + element[2][1]; 
		return rect;
	}
};

class WPGCompoundPolygon
{
public:
	WPG2TransformMatrix matrix;
	bool isFilled;
	bool isFramed;
	bool isClosed;

	WPGCompoundPolygon(): matrix(), isFilled(true), isFramed(true), isClosed(true) {}
};

class WPG2Parser : public WPGXParser
{
public:
	WPG2Parser(WPGInputStream *input, WPGPaintInterface* painter);
	bool parse();
	
private:
	void handleStartWPG();
	void handleEndWPG();
	void handleLayer();
	void handleCompoundPolygon();
	
	void handlePenStyleDefinition();
	void handlePatternDefinition();
	void handleColorPalette();
	void handleDPColorPalette();
	void handlePenForeColor();
	void handleDPPenForeColor();
	void handlePenBackColor();
	void handleDPPenBackColor();
	void handlePenStyle();
	void handlePenSize();
	void handleDPPenSize();
	void handleBrushGradient();
	void handleDPBrushGradient();
	void handleBrushForeColor();
	void handleDPBrushForeColor();
	void handleBrushBackColor();
	void handleDPBrushBackColor();
	void handleBrushPattern();

	void handlePolyline();
	void handlePolycurve();
	void handleRectangle();
	void handleArc();
	
	void resetPalette();

	// parsing context
	bool m_success;
	bool m_exit;
	unsigned int m_xres;
	unsigned int m_yres;
	long m_xofs;
	long m_yofs;
	long m_width;
	long m_height;
	bool m_doublePrecision;  
	WPGPen m_pen;
	WPGBrush m_brush;
	std::map<unsigned int,WPGDashArray> m_penStyles;
	bool m_layerOpened;
	unsigned int m_layerId;
	WPG2TransformMatrix m_matrix;
	double m_gradientAngle;
	WPGPoint m_gradientRef;
	unsigned m_subIndex;
	std::stack<unsigned> m_indexStack;

	WPGCompoundPolygon m_currentCompound;
	std::stack<WPGCompoundPolygon> m_compoundStack;

	class ObjectCharacterization;
	void parseCharacterization(ObjectCharacterization*);
};

#endif // __WPG2PARSER_H__
