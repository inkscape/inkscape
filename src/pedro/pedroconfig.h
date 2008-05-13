#ifndef __PEDROCONFIG_H__
#define __PEDROCONFIG_H__
/*
 * Implementation the Pedro mini-XMPP client
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



#include "pedrodom.h"
#include "pedroxmpp.h"

#include <vector>



namespace Pedro
{


/**
 * Individual account record
 */
class XmppAccount
{

public:

    /**
     *
     */
    XmppAccount()
        { init(); }

    /**
     *
     */
    XmppAccount(const XmppAccount &other)
        { assign(other); }

    /**
     *
     */
    XmppAccount operator=(const XmppAccount &other)
        { assign(other); return *this; }

    /**
     *
     */
    virtual ~XmppAccount()
        {}


    /**
     *
     */
    virtual DOMString getName() const
        { return name; }

    /**
     *
     */
    virtual void setName(const DOMString &val)
        { name = val; }

    /**
     *
     */
    virtual DOMString getHost() const
        { return host; }

    /**
     *
     */
    virtual void setHost(const DOMString &val)
        { host = val; }

    /**
     *
     */
    virtual int getPort() const
        { return port; }

    /**
     *
     */
    virtual void setPort(int val)
        { port = val; }

    /**
     *
     */
    virtual DOMString getUsername() const
        { return username; }

    /**
     *
     */
    virtual void setUsername(const DOMString &val)
        { username = val; }

    /**
     *
     */
    virtual DOMString getPassword() const
        { return password; }

    /**
     *
     */
    virtual void setPassword(const DOMString &val)
        { password = val; }



private:

    void init()
        {
        name     = "noname";
        host     = "jabber.org";
        port     = 5222;
        username = "nobody";
        password = "nopass";
        }

    void assign(const XmppAccount &other)
        {
        name     = other.name;
        host     = other.host;
        port     = other.port;
        username = other.username;
        password = other.password;
        }

    DOMString name;
    DOMString host;
    int port;
    DOMString username;
    DOMString password;

};



/**
 * Configuration record
 */
class XmppConfig : XmppEventTarget
{

public:

    /**
     *
     */
    XmppConfig()
        { init(); }

    /**
     *
     */
    XmppConfig(const XmppConfig &other) : XmppEventTarget(other)
        { assign(other); }

    /**
     *
     */
    virtual XmppConfig &operator=(const XmppConfig &other)
        { assign(other); return *this; }

    /**
     *
     */
    virtual ~XmppConfig()
        {}


    /**
     *  Parse a configuration xml chunk from a memory buffer
     */
    virtual bool read(const DOMString &buffer);

    /**
     * Parse a configuration file
     */
    virtual bool readFile(const DOMString &fileName);

    /**
     * Ouputs this object as a string formatted in XML
     */
    virtual DOMString toXmlBuffer();

    /**
     * Write a configuration file
     */
    virtual bool writeFile(const DOMString &fileName);

    /**
     *
     */
    virtual std::vector<XmppAccount> &getAccounts();

    /**
     *
     */
    virtual DOMString getMucGroup();

    /**
     *
     */
    virtual void setMucGroup(const DOMString &val);

    /**
     *
     */
    virtual DOMString getMucHost();

    /**
     *
     */
    virtual void setMucHost(const DOMString &val);

    /**
     *
     */
    virtual DOMString getMucNick();

    /**
     *
     */
    virtual void setMucNick(const DOMString &val);

    /**
     *
     */
    virtual DOMString getMucPassword();

    /**
     *
     */
    virtual void setMucPassword(const DOMString &val);

    /**
     *
     */
    virtual bool accountAdd(const XmppAccount &account);

    /**
     *
     */
    virtual bool accountExists(const DOMString &accountName);

    /**
     *
     */
    virtual void accountRemove(const DOMString &accountName);

    /**
     *
     */
    bool accountFind(const DOMString &accountName,
                     XmppAccount &retVal);


private:

    void init()
        {
        }

    void assign(const XmppConfig &other)
        {
        accounts = other.accounts;
        }

    //# Group stuff
    DOMString mucGroup;

    DOMString mucHost;

    DOMString mucNick;

    DOMString mucPassword;

    std::vector<XmppAccount> accounts;

};




} //namespace Pedro

#endif /* __PEDROCONFIG_H__ */

//########################################################################
//# E N D    O F     F I L E
//########################################################################
