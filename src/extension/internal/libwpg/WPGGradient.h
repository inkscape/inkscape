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

#ifndef __WPGGRADIENT_H__
#define __WPGGRADIENT_H__

#include "WPGColor.h"

namespace libwpg
{

class WPGGradientPrivate;

class WPGGradient
{
public:
	
	WPGGradient();
	
	~WPGGradient();
	
	WPGGradient(const WPGGradient&);
	
	WPGGradient& operator=(const WPGGradient&);
	
	double angle() const; // in radiant
	
	void setAngle(double angle);

	unsigned count() const;
	
	double stopOffset(unsigned index) const;
	
	WPGColor stopColor(unsigned index) const;
	
	void clear();
	
	void addStop(double offset, const WPGColor& color);
	
private:
	WPGGradientPrivate *d;
};

} // namespace libwpg

#endif // __WPGGRADIENT_H__
