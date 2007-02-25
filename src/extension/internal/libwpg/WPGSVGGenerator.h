/* libwpg
 * Copyright (C) 2006 Ariya Hidayat (ariya@kde.org)
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * For further information visit http://libwpg.sourceforge.net
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#ifndef __WPGSVGGENERATOR_H__
#define __WPGSVGGENERATOR_H__

#include <stdio.h>
#include <iostream>
#include <locale>
#include "libwpg.h"
#include "WPGStreamImplementation.h"

namespace libwpg
{

class WPGSVGGenerator : public WPGPaintInterface {
public:
	WPGSVGGenerator(std::ostream & output_sink);
	~WPGSVGGenerator();

	void startDocument(double imageWidth, double imageHeight);
	void endDocument();
	void startLayer(unsigned int id);
	void endLayer(unsigned int id);

	void setPen(const libwpg::WPGPen& pen);
	void setBrush(const libwpg::WPGBrush& brush);
	void setFillRule(FillRule rule);

	void drawRectangle(const libwpg::WPGRect& rect, double rx, double ry);
	void drawEllipse(const libwpg::WPGPoint& center, double rx, double ry);
	void drawPolygon(const libwpg::WPGPointArray& vertices);
	void drawPath(const libwpg::WPGPath& path);

private:
	libwpg::WPGPen m_pen;
	libwpg::WPGBrush m_brush;
	FillRule m_fillRule;
	int m_gradientIndex;
	void writeStyle();

	std::ostream & m_outputSink;
	std::locale m_oldLocale;
};

} // namespace libwpg

#endif // __WPGSVGGENERATOR_H__
