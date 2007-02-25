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

#include "WPGString.h"
#include <string>

namespace libwpg
{
class WPGStringPrivate
{
public:
	std::string str;
};

} // namespace libwpg

libwpg::WPGString::WPGString() :
	d(new WPGStringPrivate())
{
}


libwpg::WPGString::WPGString(const char * str):
	d(new WPGStringPrivate())
{
	d->str = str;
}

libwpg::WPGString::~WPGString()
{
	delete d;
}

const bool libwpg::WPGString::empty() const
{
	return d->str.empty();
}

const char * libwpg::WPGString::cstr() const
{
	return d->str.c_str();
}

const long libwpg::WPGString::length() const
{
	return d->str.length();
}

libwpg::WPGString& libwpg::WPGString::operator=(const libwpg::WPGString& str)
{
	d->str = str.d->str;
	return *this;
}

libwpg::WPGString& libwpg::WPGString::operator=(const char * str)
{
	d->str = str;
	return *this;
}
