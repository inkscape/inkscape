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

#ifndef __WPGPATH_H__
#define __WPGPATH_H__

#include "WPGPoint.h"

namespace libwpg
{

class WPGPathElement
{
public:
	typedef enum 
	{
		NullElement,
		MoveToElement,
		LineToElement,
		CurveToElement
	} Type;
	
	Type type;
	WPGPoint point;
	WPGPoint extra1;
	WPGPoint extra2;
	
	WPGPathElement();
};

class WPGPathPrivate;

class WPGPath
{
public:

	bool closed;
	
	WPGPath();
	
	~WPGPath();
	
	WPGPath(const WPGPath&);
	
	WPGPath& operator=(const WPGPath&);
	
	unsigned count() const;
	
	WPGPathElement element(unsigned index) const;
	
	void moveTo(const WPGPoint& point);
	
	void lineTo(const WPGPoint& point);

	void curveTo(const WPGPoint& c1, const WPGPoint& c2, const WPGPoint& endPoint);
	
	void addElement(const WPGPathElement& element);
	
	void append(const WPGPath& path);
	
private:
	WPGPathPrivate *d;
};

} // namespace libwpg

#endif // __WPGPATH_H__
