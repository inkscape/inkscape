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

#include "WPGPen.h"

#include <vector>

namespace libwpg
{
class WPGDashArrayPrivate
{
public:
	std::vector<double> dashes;
};
}
	
libwpg::WPGDashArray::WPGDashArray() : d(new libwpg::WPGDashArrayPrivate())
{
}

libwpg::WPGDashArray::~WPGDashArray()
{
	delete d;
}

libwpg::WPGDashArray::WPGDashArray(const libwpg::WPGDashArray& dash):
	d(new libwpg::WPGDashArrayPrivate())
{
	d->dashes = dash.d->dashes;
}

libwpg::WPGDashArray& libwpg::WPGDashArray::operator=(const libwpg::WPGDashArray& dash)
{
	d->dashes = dash.d->dashes;
	return *this;
}

unsigned libwpg::WPGDashArray::count() const
{
	return d->dashes.size();
}

double libwpg::WPGDashArray::at(unsigned i) const
{
	return d->dashes[i];
}

void libwpg::WPGDashArray::add(double p)
{
	d->dashes.push_back(p);
}

libwpg::WPGPen::WPGPen(): 
	foreColor(0,0,0), 
	backColor(0xFF,0xFF,0xFF), 
	width(0), 
	height(0), 
	solid(true) ,
	dashArray(WPGDashArray())
{
}

libwpg::WPGPen::WPGPen(const WPGColor& fore): 
	foreColor(fore), 
	backColor(0xFF,0xFF,0xFF), 
	width(0), 
	height(0), 
	solid(true),
	dashArray(WPGDashArray()) 
{
}

libwpg::WPGPen::WPGPen(const WPGColor& fore, const WPGColor& back): 
	foreColor(fore),  
	backColor(back), 
	width(0), 
	height(0), 
	solid(true) ,
	dashArray(WPGDashArray())
{
}

libwpg::WPGPen::WPGPen(const WPGPen& pen):
	foreColor(pen.foreColor),
	backColor(pen.backColor),
	width(pen.width),
	height(pen.height),
	solid(pen.solid),
	dashArray(pen.dashArray)
{
}

libwpg::WPGPen& libwpg::WPGPen::operator=(const libwpg::WPGPen& pen)
{ 
	foreColor = pen.foreColor; 
	backColor = pen.backColor;
	width = pen.width;
	height = pen.height;
	solid = pen.solid;
	dashArray = pen.dashArray;
	return *this;
}

