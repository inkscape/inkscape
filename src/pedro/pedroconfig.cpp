/*
 * Implementation the Pedro mini-XMPP client
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2005-2007 Bob Jamison
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


/*
====================================================
We are expecting an xml file with this format:

<pedro>

    <!-- zero to many of these -->
    <account>
        <name>Jabber's Main Server</name>
        <host>jabber.org</host>
        <port>5222</port>
        <username>myname</username>
        <password>mypassword</password>
    </account>

</pedro>


====================================================
*/



#include "pedroconfig.h"
#include "pedrodom.h"

#include <stdio.h>
#include <cstring>
#include <string>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>

namespace Pedro
{


static long  getInt(const DOMString &s)
{
    char *start = (char *) s.c_str();
    char *end;
    long val = strtol(start, &end, 10);
    if (end == start) // did we read more than 1 digit?
        val = 0L;
    return val;
}



bool XmppConfig::read(const DOMString &buffer)
{
    Parser parser;

    Element *root = parser.parse(buffer);

    if (!root)
        {
        error("Error in configuration syntax");
        return false;
        }

    accounts.clear();

    std::vector<Element *> mucElems = root->findElements("muc");
    if (mucElems.size() > 0)
        {
        Element *elem = mucElems[0];
        mucGroup    = elem->getTagValue("group");
        mucHost     = elem->getTagValue("host");
        mucNick     = elem->getTagValue("nick");
        mucPassword = elem->getTagValue("password");
        }

    std::vector<Element *> accountElems = root->findElements("account");

    for (unsigned int i=0 ; i<accountElems .size() ; i++)
        {
        XmppAccount account;
        Element *elem = accountElems [i];

        DOMString str = elem->getTagValue("name");
        if (str.size()==0)
            str = "unnamed account";
        account.setName(str);

        str = elem->getTagValue("host");
        if (str.size()==0)
            str = "jabber.org";
        account.setHost(str);

        str = elem->getTagValue("port");
        int port = (int) getInt(str);
        if (port == 0)
            port = 5222;
        account.setPort(port);

        str = elem->getTagValue("username");
        if (str.size()==0)
            str = "noname";
        account.setUsername(str);

        str = elem->getTagValue("password");
        if (str.size()==0)
            str = "nopass";
        account.setPassword(str);

        accounts.push_back(account);
        }


    delete root;

    return true;
}






bool XmppConfig::readFile(const DOMString &fileName)
{

    FILE *f = fopen(fileName.c_str(), "rb");
    if (!f)
        {
        error("Could not open configuration file '%s' for reading",
              fileName.c_str());
        return false;
        }

    DOMString buffer;
    while (!feof(f))
        {
        char ch = (char) fgetc(f);
        buffer.push_back(ch);
        }
    fclose(f);

    if (!read(buffer))
        return false;

    return true;
}


DOMString XmppConfig::toXmlBuffer()
{

    DOMString buf;

    char fmtbuf[32];

    buf.append("<pedro>\n");
    buf.append("    <muc>\n");
    buf.append("        <group>");
    buf.append(mucGroup);
    buf.append("</group>\n");
    buf.append("        <host>");
    buf.append(mucHost);
    buf.append("</host>\n");
    buf.append("        <nick>");
    buf.append(mucNick);
    buf.append("</nick>\n");
    buf.append("        <password>");
    buf.append(mucPassword);
    buf.append("</password>\n");
    buf.append("    </muc>\n");

    for (unsigned int i = 0 ; i<accounts.size() ; i++)
        {
        XmppAccount acc = accounts[i];
        buf.append("    <account>\n");
        buf.append("        <name>");
        buf.append(acc.getName());
        buf.append("</name>\n");
        buf.append("        <host>");
        buf.append(acc.getHost());
        buf.append("</host>\n");
        buf.append("        <port>");
        snprintf(fmtbuf, 31, "%d", acc.getPort());
        buf.append(fmtbuf);
        buf.append("</port>\n");
        buf.append("        <username>");
        buf.append(acc.getUsername());
        buf.append("</username>\n");
        buf.append("        <password>");
        buf.append(acc.getPassword());
        buf.append("</password>\n");
        buf.append("    </account>\n");
        }

    buf.append("</pedro>\n");

    return buf;
}




bool XmppConfig::writeFile(const DOMString &fileName)
{

    FILE *f = fopen(fileName.c_str(), "wb");
    if (!f)
        {
        error("Could not open configuration file '%s' for writing",
              fileName.c_str());
        return false;
        }

    DOMString buffer = toXmlBuffer();
    char *s = (char *)buffer.c_str();
    size_t len = (size_t) strlen(s);  //in case we have wide chars

    if (fwrite(s, 1, len, f) != len)
        {
        return false;
        }
    fclose(f);

    if (!read(buffer))
        return false;

    return true;
}


/**
 *
 */
DOMString XmppConfig::getMucGroup()
{
    return mucGroup;
}

/**
 *
 */
void XmppConfig::setMucGroup(const DOMString &val)
{
    mucGroup = val;
}

/**
 *
 */
DOMString XmppConfig::getMucHost()
{
    return mucHost;
}

/**
 *
 */
void XmppConfig::setMucHost(const DOMString &val)
{
    mucHost = val;
}

/**
 *
 */
DOMString XmppConfig::getMucNick()
{
    return mucNick;
}

/**
 *
 */
void XmppConfig::setMucNick(const DOMString &val)
{
    mucNick = val;
}

/**
 *
 */
DOMString XmppConfig::getMucPassword()
{
    return mucPassword;
}

/**
 *
 */
void XmppConfig::setMucPassword(const DOMString &val)
{
    mucPassword = val;
}



/**
 *
 */
std::vector<XmppAccount> &XmppConfig::getAccounts()
{
    return accounts;
}


/**
 *
 */
bool XmppConfig::accountAdd(const XmppAccount &account)
{
    DOMString name = account.getName();
    if (name.size() < 1)
        return false;
    if (accountExists(name))
        return false;
    accounts.push_back(account);
    return true;
}


/**
 *
 */
bool XmppConfig::accountExists(const DOMString &accountName)
{
    if (accountName.size() < 1)
        return false;
    std::vector<XmppAccount>::iterator iter;
    for (iter = accounts.begin() ; iter!= accounts.end() ; iter++)
        {
        if (iter->getName() == accountName)
            return true;
        }
    return false;
}



/**
 *
 */
void XmppConfig::accountRemove(const DOMString &accountName)
{
    if (accountName.size() < 1)
        return;
    std::vector<XmppAccount>::iterator iter;
    for (iter = accounts.begin() ; iter!= accounts.end() ; )
        {
        if (iter->getName() == accountName)
            iter = accounts.erase(iter);
        else
            iter++;
        }        
}


/**
 *
 */
bool XmppConfig::accountFind(const DOMString &accountName,
                             XmppAccount &retVal)
{
    if (accountName.size() < 1)
        return false;
    std::vector<XmppAccount>::iterator iter;
    for (iter = accounts.begin() ; iter!= accounts.end() ; iter++)
        {
        if (iter->getName() == accountName)
            {
            retVal = (*iter);
            return true;
            }
        }        
    return false;
}





} //namespace Pedro
//########################################################################
//# E N D    O F     F I L E
//########################################################################
