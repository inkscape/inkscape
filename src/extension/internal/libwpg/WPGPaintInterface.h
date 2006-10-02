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

#ifndef __WPGPAINTINTERFACE_H__
#define __WPGPAINTINTERFACE_H__

#include "WPGBrush.h"
#include "WPGPath.h"
#include "WPGPen.h"
#include "WPGPoint.h"
#include "WPGRect.h"

namespace libwpg
{

class WPGPaintInterface {
public:
	virtual ~WPGPaintInterface() {}

	// none of the other callback functions will be called before this function is called
	virtual void startDocument(double width, double height) = 0;

	virtual void setPen(const WPGPen& pen) = 0;
	
	virtual void setBrush(const WPGBrush& brush) = 0;

	typedef enum { AlternatingFill, WindingFill } FillRule;
	virtual void setFillRule(FillRule rule ) = 0;

	virtual void startLayer(unsigned int id) = 0;

	virtual void endLayer(unsigned int id) = 0;

	virtual void drawRectangle(const WPGRect& rect, double rx, double ry) = 0;

	virtual void drawEllipse(const WPGPoint& center, double rx, double ry) = 0;

	virtual void drawPolygon(const WPGPointArray& vertices) = 0;

	virtual void drawPath(const WPGPath& path) = 0;

	// none of the other callback functions will be called after this function is called
	virtual void endDocument() = 0;
};

} // namespace libwpg

#endif // __WPGPAINTINTERFACE_H__
 
