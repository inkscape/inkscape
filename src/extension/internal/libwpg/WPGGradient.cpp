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

libwpg::WPGGradient::WPGGradient()
{
	d = new WPGGradientPrivate;
	d->angle = 0.0;
}

libwpg::WPGGradient::~WPGGradient()
{
	delete d;
}

libwpg::WPGGradient::WPGGradient(const libwpg::WPGGradient& g)
{
	d = new libwpg::WPGGradientPrivate;
	d->angle = g.d->angle;
	d->gradientStops = g.d->gradientStops;
}
	
libwpg::WPGGradient& libwpg::WPGGradient::operator=(const libwpg::WPGGradient& g)
{
	d->angle = g.d->angle;
	d->gradientStops = g.d->gradientStops;
	return *this;
}

double libwpg::WPGGradient::angle() const
{
	return d->angle;
}

void libwpg::WPGGradient::setAngle(double a)
{
	d->angle = a;
}
	
unsigned libwpg::WPGGradient::count() const
{
	return d->gradientStops.size();
}

double libwpg::WPGGradient::stopOffset(unsigned index) const
{
	return d->gradientStops[index].offset;
}

libwpg::WPGColor libwpg::WPGGradient::stopColor(unsigned index) const
{
	return d->gradientStops[index].color;
}

void libwpg::WPGGradient::clear()
{
	d->gradientStops.clear();
}

void libwpg::WPGGradient::addStop(double offset, const libwpg::WPGColor& color)
{
	libwpg::WPGGradientStop stop(offset, color);
	d->gradientStops.push_back(stop);
}
