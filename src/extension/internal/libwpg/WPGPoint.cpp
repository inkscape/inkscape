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

#include "WPGPoint.h"

#include <vector>

namespace libwpg
{

class WPGPointArrayPrivate
{
public:
	std::vector<WPGPoint> points;
};

}
	
using namespace libwpg;


WPGPointArray::WPGPointArray()
{
	d = new WPGPointArrayPrivate;
}

WPGPointArray::~WPGPointArray()
{
	delete d;
}

WPGPointArray::WPGPointArray(const WPGPointArray& pa)
{
	d = new WPGPointArrayPrivate;
	d->points = pa.d->points;
}

WPGPointArray& WPGPointArray::operator=(const WPGPointArray& pa)
{
	d->points = pa.d->points;
	return *this;
}

unsigned WPGPointArray::count() const
{
	return d->points.size();
}

WPGPoint& WPGPointArray::at(unsigned i)
{
	return d->points[i];
}

const WPGPoint& WPGPointArray::at(unsigned i) const
{
	return d->points[i];
}

const WPGPoint& WPGPointArray::operator[](unsigned i) const
{
	return d->points[i];
}

void WPGPointArray::add(const WPGPoint& p)
{
	d->points.push_back(p);
}
