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

#ifndef __WPGSTRING_H__
#define __WPGSTRING_H__

namespace libwpg
{
class WPGStringPrivate;
class WPGString
{
public:
	WPGString();
	WPGString(const char * str);
	
	~WPGString();

	const bool empty() const;

	const char * cstr() const;

	const long length() const;

	WPGString& operator=(const WPGString& str);
	WPGString& operator=(const char * str);
private:
	WPGStringPrivate * d;
};

} // namespace libwpg

#endif // __WPGSTRING_H__
