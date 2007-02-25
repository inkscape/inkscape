/* libwpg
 * Copyright (C) 2006 Fridrich Strba (fridrich.strba@bluewin.ch)
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

#include "WPGColor.h"

libwpg::WPGColor::WPGColor():
	red(0),
	green(0),
	blue(0),
	alpha(0)
{}

libwpg::WPGColor::WPGColor(int r, int g, int b):
	red(r),
	green(g),
	blue(b),
	alpha(0)
{}

libwpg::WPGColor::WPGColor(int r, int g, int b, int a):
	red(r),
	green(g),
	blue(b),
	alpha(a)
{}

libwpg::WPGColor::WPGColor(const WPGColor& color):
	red(color.red),
	green(color.green),
	blue(color.blue),
	alpha(color.alpha)
{}

libwpg::WPGColor& libwpg::WPGColor::operator=(const libwpg::WPGColor& color)
{
	red = color.red;
	green = color.green;
	blue = color.blue;
	alpha = color.alpha;
	return *this;
}
