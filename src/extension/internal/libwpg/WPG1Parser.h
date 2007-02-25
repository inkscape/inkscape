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

#ifndef __WPG1PARSER_H__
#define __WPG1PARSER_H__

#include "WPGXParser.h"
#include "WPGBrush.h"
#include "WPGPen.h"

class WPG1Parser : public WPGXParser
{
public:
	WPG1Parser(libwpg::WPGInputStream *input, libwpg::WPGPaintInterface* painter);
	bool parse();

private:
	void handleStartWPG();
	void handleEndWPG();

	void handleFillAttributes();
	void handleLineAttributes();
	void handleColormap();

	void handleLine();
	void handlePolyline();
	void handleRectangle();
	void handlePolygon();
	void handleEllipse();
	
	void resetPalette();

	// parsing context
	bool m_success;
	bool m_exit;
	int m_width;
	int m_height;
	libwpg::WPGPen m_pen;
	libwpg::WPGBrush m_brush;
};

#endif // __WPG1PARSER_H__
