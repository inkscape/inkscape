#ifndef SEEN_REGISTRYTOOL_H
#define SEEN_REGISTRYTOOL_H
/*
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

namespace Glib {
    class ustring;
}

/**
 * Inkscape Registry Tool
 *
 * This simple tool is intended for allowing Inkscape to append subdirectories
 * to its path.  This will allow extensions and other files to be accesses
 * without explicit user intervention.
 */
class RegistryTool
{
public:

    RegistryTool()
        {}

    virtual ~RegistryTool()
        {}

    /**
     * Set the string value of a key/name registry entry.
     */ 
    bool setStringValue(const Glib::ustring &key,
                        const Glib::ustring &valueName,
                        const Glib::ustring &value);

    /**
     * Get the full path, directory, and base file name of this running executable.
     */ 
    bool getExeInfo(Glib::ustring &fullPath,
                    Glib::ustring &path,
                    Glib::ustring &exeName);

    /**
     * Append our subdirectories to the Application Path for this
     * application.
     */  
    bool setPathInfo();


};

#endif // SEEN_REGISTRYTOOL_H

