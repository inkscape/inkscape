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

#ifndef __WPGPEN_H__
#define __WPGPEN_H__

#include "WPGColor.h"

namespace libwpg
{

class WPGDashArrayPrivate;

class WPGDashArray
{
public:
	WPGDashArray();
	~WPGDashArray();
	WPGDashArray(const WPGDashArray&);
	WPGDashArray& operator=(const WPGDashArray&);
	unsigned count() const;
	double at(unsigned i) const;
	void add(double p);

private:
	WPGDashArrayPrivate *d;
};

class WPGPen
{
public:
	WPGColor foreColor;
	WPGColor backColor;
	double width;
	double height;
	bool solid;
	WPGDashArray dashArray;

	WPGPen(); 
	WPGPen(const WPGColor& fore);
	WPGPen(const WPGColor& fore, const WPGColor& back);
	WPGPen(const WPGPen& pen);

	WPGPen& operator=(const WPGPen& pen);
	
	static WPGPen NoPen;
};

} // namespace libwpg


#endif // __WPGPEN_H__
