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

#include "WPGGradient.h"
#include "WPGColor.h"

#include <vector>

namespace libwpg
{

class WPGGradientStop
{
public:
	double offset;
	WPGColor color;
	
	WPGGradientStop(): offset(0) {}
	
	WPGGradientStop(double ofs, const WPGColor& c): offset(ofs), color(c) {}
};

class WPGGradientPrivate
{
public:
    double angle;
	std::vector<WPGGradientStop> gradientStops;
};

} // namespace libwpg

using namespace libwpg;

WPGGradient::WPGGradient()
{
	d = new WPGGradientPrivate;
	d->angle = 0.0;
}

WPGGradient::~WPGGradient()
{
	delete d;
}

WPGGradient::WPGGradient(const WPGGradient& g)
{
	d = new WPGGradientPrivate;
	d->angle = g.d->angle;
	d->gradientStops = g.d->gradientStops;
}
	
WPGGradient& WPGGradient::operator=(const WPGGradient& g)
{
	d->angle = g.d->angle;
	d->gradientStops = g.d->gradientStops;
	return *this;
}

double WPGGradient::angle() const
{
	return d->angle;
}

void WPGGradient::setAngle(double a)
{
	d->angle = a;
}
	
unsigned WPGGradient::count() const
{
	return d->gradientStops.size();
}

double WPGGradient::stopOffset(unsigned index) const
{
	return d->gradientStops[index].offset;
}

WPGColor WPGGradient::stopColor(unsigned index) const
{
	return d->gradientStops[index].color;
}

void WPGGradient::clear()
{
	d->gradientStops.clear();
}

void WPGGradient::addStop(double offset, const WPGColor& color)
{
	WPGGradientStop stop(offset, color);
	d->gradientStops.push_back(stop);
}
