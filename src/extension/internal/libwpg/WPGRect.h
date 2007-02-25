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

#ifndef __WPGRECT_H__
#define __WPGRECT_H__

#include "WPGColor.h"

namespace libwpg
{

class WPGRect
{
public:
	double x1;
	double y1;
	double x2;
	double y2;

	WPGRect();

	WPGRect(double xx1, double yy1, double xx2, double yy2);

	WPGRect(const WPGRect& rect);

	WPGRect& operator=(const WPGRect& rect);

	const double width() const;
	const double height() const;
};

} // namespace libwpg

#endif // __WPGRECT_H__
