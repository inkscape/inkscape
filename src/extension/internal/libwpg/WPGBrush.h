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

#ifndef __WPGBRUSH_H__
#define __WPGBRUSH_H__

#include "WPGColor.h"
#include "WPGGradient.h"

namespace libwpg
{

class WPGBrush
{
public:

	typedef enum
	{
		NoBrush,
		Solid,
		Pattern,
		Gradient
	} WPGBrushStyle;
	
	WPGBrushStyle style;
	WPGColor foreColor;
	WPGColor backColor;
	
	WPGGradient gradient;

	WPGBrush();

	explicit WPGBrush(const WPGColor& fore);

	WPGBrush(const WPGColor& fore, const WPGColor& back);

	WPGBrush(const WPGBrush& brush);

	WPGBrush& operator=(const WPGBrush& brush);
};

} // namespace libwpg

#endif // __WPGBRUSH_H__
