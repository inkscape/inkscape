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

#ifndef __WPGPOINT_H__
#define __WPGPOINT_H__

#include "WPGColor.h"

namespace libwpg
{

class WPGPoint
{
public:
	double x;
	double y;

	WPGPoint();

	WPGPoint(double xx, double yy);

	WPGPoint(const WPGPoint& point);

	WPGPoint& operator=(const WPGPoint& point);
};

class WPGPointArrayPrivate;

class WPGPointArray
{
public:
	WPGPointArray();
	~WPGPointArray();
	WPGPointArray(const WPGPointArray&);
	WPGPointArray& operator=(const WPGPointArray&);
	unsigned count() const;
	WPGPoint& at(unsigned i);
	const WPGPoint& at(unsigned i) const;
	const WPGPoint& operator[](unsigned i) const;
	void add(const WPGPoint& p);
	
private:
	WPGPointArrayPrivate *d;
};

} // namespace libwpg

#endif // __WPGPOINT_H__
