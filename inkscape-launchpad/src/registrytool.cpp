/*
 * Inkscape Registry Tool
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2005-2008 Bob Jamison
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

#include "registrytool.h"

#include <windows.h>
#include <string>
#include <cstdio>

#include <glibmm/ustring.h>

typedef struct
{
    HKEY       key;
    int        strlen;
    const char *str;
} KeyTableEntry;



KeyTableEntry keyTable[] =
{
    { HKEY_CLASSES_ROOT,   18, "HKEY_CLASSES_ROOT\\"   },
    { HKEY_CURRENT_CONFIG, 20, "HKEY_CURRENT_CONFIG\\" },
    { HKEY_CURRENT_USER,   18, "HKEY_CURRENT_USER\\"   },
    { HKEY_LOCAL_MACHINE,  19, "HKEY_LOCAL_MACHINE\\"  },
    { HKEY_USERS,          11, "HKEY_USERS\\"          },
    { NULL,                 0, NULL                    }
};


bool RegistryTool::setStringValue(const Glib::ustring &keyNameArg,
                                  const Glib::ustring &valueName,
                                  const Glib::ustring &value)
{
    Glib::ustring keyName = keyNameArg;
    bool ret = false;

    HKEY rootKey = HKEY_LOCAL_MACHINE; //default root
    //Trim out the root key if necessary
    for (KeyTableEntry *entry = keyTable; entry->key; entry++)
        {
        if (keyName.compare(0, entry->strlen, entry->str)==0)
            {
            rootKey = entry->key;
            keyName = keyName.substr(entry->strlen);
            }
        }
    //printf("trimmed string: '%s'\n", keyName.c_str());

    //Get or create the key
    gunichar2 *keyw       = g_utf8_to_utf16(keyName.data(), -1, 0,0,0);
    gunichar2 *valuenamew = g_utf8_to_utf16(valueName.data(), -1, 0,0,0);
    gunichar2 *valuew     = g_utf8_to_utf16(value.data(), -1, 0,0,0);

    HKEY key;
    if (RegCreateKeyExW(rootKey, (WCHAR*) keyw,
                       0, NULL, REG_OPTION_NON_VOLATILE,
                       KEY_WRITE, NULL, &key, NULL))
    {
       fprintf(stderr, "RegistryTool: Could not create the registry key '%s'\n", keyName.c_str());
       goto fail;
    }

    // Set the value
    if (RegSetValueExW(key, (WCHAR*) valuenamew,
          0,  REG_SZ, (LPBYTE) valuew, (DWORD) (2*value.size() + 2)))
    {
       fprintf(stderr, "RegistryTool: Could not set the value '%s'\n", value.c_str());
       goto failkey;
    }

    ret = true;
    
    failkey:
    RegCloseKey(key);
    
    fail:
    g_free(keyw);
    g_free(valuenamew);
    return ret;
}



bool RegistryTool::getExeInfo(Glib::ustring &fullPath,
                              Glib::ustring &path,
                              Glib::ustring &exeName)
{
    const int pathbuf = 2048;
    gunichar2 pathw[pathbuf];
    GetModuleFileNameW(NULL, (WCHAR*) pathw, pathbuf);

    gchar *utf8path = g_utf16_to_utf8(pathw, -1, 0,0,0);
    fullPath = utf8path;
    g_free(utf8path);

    path     = "";
    exeName  = "";
    Glib::ustring::size_type pos = fullPath.rfind('\\');
    if (pos != fullPath.npos)
        {
        path = fullPath.substr(0, pos);
        exeName = fullPath.substr(pos+1);
        }

    return true;
}



bool RegistryTool::setPathInfo()
{
    Glib::ustring fullPath;
    Glib::ustring path;
    Glib::ustring exeName;

    if (!getExeInfo(fullPath, path, exeName))
        return false;

    //printf("full:'%s' path:'%s' exe:'%s'\n",
    //    fullPath.c_str(), path.c_str(), exeName.c_str());

    Glib::ustring keyName =
    "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
    keyName.append(exeName);

    Glib::ustring valueName = "";
    Glib::ustring value     = fullPath;

    if (!setStringValue(keyName, valueName, value))
        return false;

    //add our subdirectories
    Glib::ustring appPath = path;
    appPath.append("\\python;");
    appPath.append(path);
    appPath.append("\\perl");
    valueName = "Path";
    value     = appPath;

    if (!setStringValue(keyName, valueName, value))
        return false;

    return true;
}


#ifdef TESTREG


/*
 * Compile this file with
 *      g++ -DTESTREG registrytool.cpp -o registrytool
 *  to run these tests.
 */
 
 
    
void testReg()
{
    RegistryTool rt;
    char *key =
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\inkscape.exe";
    char const *name  = "";
    char const *value = "c:\\inkscape\\inkscape.exe";
    if (!rt.setStringValue(key, name, value))
        {
        printf("Test failed\n");
        }
    else
        {
        printf("Test succeeded\n");
        }
    name  = "Path";
    value = "c:\\inkscape\\python";
    if (!rt.setStringValue(key, name, value))
        {
        printf("Test failed\n");
        }
    else
        {
        printf("Test succeeded\n");
        }
}


void testPath()
{
    RegistryTool rt;
    rt.setPathInfo();
}


int main(int argc, char **argv)
{
    //testReg();
    testPath();
    return 0;
}

#endif /* TESTREG */

//########################################################################
//# E N D    O F    F I L E
//########################################################################
