#ifndef __REGISTRYTOOL_H__
#define __REGISTRYTOOL_H__
/**
 * Inkscape Registry Tool
 *
 * This simple tool is intended for allowing Inkscape to append subdirectories
 * to its path.  This will allow extensions and other files to be accesses
 * without explicit user intervention.
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2005 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <string>

class RegistryTool
{
public:

    RegistryTool()
        {}

    virtual ~RegistryTool()
        {}

    bool setStringValue(const std::string &key,
                        const std::string &valueName,
                        const std::string &value);

    bool getExeInfo(std::string &fullPath,
                    std::string &path,
                    std::string &exeName);

    bool setPathInfo();


};

#endif  /* __REGISTRYTOOL_H__ */

