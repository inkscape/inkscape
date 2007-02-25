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

libwpg::WPGPoint::WPGPoint():
	x(0.0),
	y(0.0)
{}

libwpg::WPGPoint::WPGPoint(double xx, double yy):
	x(xx),
	y(yy)
{}

libwpg::WPGPoint::WPGPoint(const WPGPoint& point):
	x(point.x),
	y(point.y)
{}

libwpg::WPGPoint& libwpg::WPGPoint::operator=(const libwpg::WPGPoint& point)
{
	x = point.x;
	y = point.y;
	return *this;
}
	
libwpg::WPGPointArray::WPGPointArray(): d(new libwpg::WPGPointArrayPrivate())
{}

libwpg::WPGPointArray::~WPGPointArray()
{
	delete d;
}

libwpg::WPGPointArray::WPGPointArray(const libwpg::WPGPointArray& pa):
	d(new libwpg::WPGPointArrayPrivate())
{
	d->points = pa.d->points;
}

libwpg::WPGPointArray& libwpg::WPGPointArray::operator=(const libwpg::WPGPointArray& pa)
{
	d->points = pa.d->points;
	return *this;
}

unsigned libwpg::WPGPointArray::count() const
{
	return d->points.size();
}

libwpg::WPGPoint& libwpg::WPGPointArray::at(unsigned i)
{
	return d->points[i];
}

const libwpg::WPGPoint& libwpg::WPGPointArray::at(unsigned i) const
{
	return d->points[i];
}

const libwpg::WPGPoint& libwpg::WPGPointArray::operator[](unsigned i) const
{
	return d->points[i];
}

void libwpg::WPGPointArray::add(const libwpg::WPGPoint& p)
{
	d->points.push_back(p);
}
