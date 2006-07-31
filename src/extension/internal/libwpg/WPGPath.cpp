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

#include "WPGPath.h"
#include "WPGPoint.h"

#include <vector>

namespace libwpg
{

class WPGPathPrivate
{
public:
	std::vector<WPGPathElement> elements;
};

} // namespace libwpg

using namespace libwpg;

WPGPath::WPGPath()
{
	d = new WPGPathPrivate;
	closed = true;
}
	
WPGPath::~WPGPath()
{
	delete d;
}
	
WPGPath::WPGPath(const WPGPath& path)
{
	d = new WPGPathPrivate;
	d->elements = path.d->elements;
}
	
WPGPath& WPGPath::operator=(const WPGPath& path)
{
	d->elements = path.d->elements;
	return *this;
}
	
unsigned WPGPath::count() const
{
	return d->elements.size();
}
	
WPGPathElement WPGPath::element(unsigned index) const
{
	return d->elements[index];
}
	
void WPGPath::moveTo(const WPGPoint& point)
{
	WPGPathElement element;
	element.type = WPGPathElement::MoveToElement;
	element.point = point;
	addElement(element);
}
	
void WPGPath::lineTo(const WPGPoint& point)
{
	WPGPathElement element;
	element.type = WPGPathElement::LineToElement;
	element.point = point;
	addElement(element);
}
	
void WPGPath::curveTo(const WPGPoint& c1, const WPGPoint& c2, const WPGPoint& endPoint)
{
	WPGPathElement element;
	element.type = WPGPathElement::CurveToElement;
	element.point = endPoint;
	element.extra1 = c1;
	element.extra2 = c2;
	addElement(element);
}
	
void WPGPath::addElement(const WPGPathElement& element)
{
	d->elements.push_back(element);
}

void WPGPath::append(const WPGPath& path)
{
	for(unsigned i = 0; i < path.count(); i++)
		addElement(path.element(i));
}
