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


#include <algorithm>
#include <cstdio>
#include <stdarg.h>
#include <stdlib.h>

#include <sys/stat.h>

#include <time.h>

#include "pedroxmpp.h"
#include "pedrodom.h"
#include "pedroutil.h"

#include <map>



namespace Pedro
{


//########################################################################
//########################################################################
//# X M P P    E V E N T
//########################################################################
//########################################################################


XmppEvent::XmppEvent(int type)
{
    eventType = type;
    presence  = false;
    dom       = NULL;
}

XmppEvent::XmppEvent(const XmppEvent &other)
{
    assign(other);
}

XmppEvent &XmppEvent::operator=(const XmppEvent &other)
{
    assign(other);
    return (*this);
}

XmppEvent::~XmppEvent()
{
    if (dom)
        delete dom;
}

void XmppEvent::assign(const XmppEvent &other)
{
    eventType = other.eventType;
    presence  = other.presence;
    status    = other.status;
    show      = other.show;
    to        = other.to;
    from      = other.from;
    group     = other.group;
    data      = other.data;
    fileName  = other.fileName;
    fileDesc  = other.fileDesc;
    fileSize  = other.fileSize;
    fileHash  = other.fileHash;
    setDOM(other.dom);
}

int XmppEvent::getType() const
{
    return eventType;
}

DOMString XmppEvent::getIqId() const
{
    return iqId;
}

void XmppEvent::setIqId(const DOMString &val)
{
    iqId = val;
}

DOMString XmppEvent::getStreamId() const
{
    return streamId;
}

void XmppEvent::setStreamId(const DOMString &val)
{
    streamId = val;
}

bool XmppEvent::getPresence() const
{
    return presence;
}

void XmppEvent::setPresence(bool val)
{
    presence = val;
}

DOMString XmppEvent::getShow() const
{
    return show;
}

void XmppEvent::setShow(const DOMString &val)
{
    show = val;
}

DOMString XmppEvent::getStatus() const
{
    return status;
}

void XmppEvent::setStatus(const DOMString &val)
{
    status = val;
}

DOMString XmppEvent::getTo() const
{
    return to;
}

void XmppEvent::setTo(const DOMString &val)
{
    to = val;
}

DOMString XmppEvent::getFrom() const
{
    return from;
}

void XmppEvent::setFrom(const DOMString &val)
{
    from = val;
}

DOMString XmppEvent::getGroup() const
{
    return group;
}

void XmppEvent::setGroup(const DOMString &val)
{
    group = val;
}

DOMString XmppEvent::getData() const
{
    return data;
}

void XmppEvent::setData(const DOMString &val)
{
    data = val;
}

DOMString XmppEvent::getFileName() const
{
    return fileName;
}

void XmppEvent::setFileName(const DOMString &val)
{
    fileName = val;
}

DOMString XmppEvent::getFileDesc() const
{
    return fileDesc;
}

void XmppEvent::setFileDesc(const DOMString &val)
{
    fileDesc = val;
}

long XmppEvent::getFileSize() const
{
    return fileSize;
}

void XmppEvent::setFileSize(long val)
{
    fileSize = val;
}

DOMString XmppEvent::getFileHash() const
{
    return fileHash;
}

void XmppEvent::setFileHash(const DOMString &val)
{
    fileHash = val;
}

Element *XmppEvent::getDOM() const
{
    return dom;
}

void XmppEvent::setDOM(const Element *val)
{
    if (!val)
        dom = NULL;
    else
        dom = ((Element *)val)->clone();
}


std::vector<XmppUser> XmppEvent::getUserList() const
{
    return userList;
}

void XmppEvent::setUserList(const std::vector<XmppUser> &val)
{
    userList = val;
}









//########################################################################
//########################################################################
//# X M P P    E V E N T    T A R G E T
//########################################################################
//########################################################################


//###########################
//# CONSTRUCTORS
//###########################

XmppEventTarget::XmppEventTarget()
{
    eventQueueEnabled = false;
}


XmppEventTarget::XmppEventTarget(const XmppEventTarget &other)
{
    listeners         = other.listeners;
    eventQueueEnabled = other.eventQueueEnabled;
}

XmppEventTarget::~XmppEventTarget()
{
}


//###########################
//# M E S S A G E S
//###########################

/**
 *  Print a printf()-like formatted error message
 */
void XmppEventTarget::error(char *fmt, ...)
{
    va_list args;
    va_start(args,fmt);
    gchar * buffer = g_strdup_vprintf(fmt, args);
    va_end(args) ;
    fprintf(stderr, "Error:%s\n", buffer);
    XmppEvent evt(XmppEvent::EVENT_ERROR);
    evt.setData(buffer);
    dispatchXmppEvent(evt);
    g_free(buffer);
}



/**
 *  Print a printf()-like formatted trace message
 */
void XmppEventTarget::status(char *fmt, ...)
{
    va_list args;
    va_start(args,fmt);
    gchar * buffer = g_strdup_vprintf(fmt, args);
    va_end(args) ;
    //printf("Status:%s\n", buffer);
    XmppEvent evt(XmppEvent::EVENT_STATUS);
    evt.setData(buffer);
    dispatchXmppEvent(evt);
    g_free(buffer);
}



//###########################
//# L I S T E N E R S
//###########################

void XmppEventTarget::dispatchXmppEvent(const XmppEvent &event)
{
    std::vector<XmppEventListener *>::iterator iter;
    for (iter = listeners.begin(); iter != listeners.end() ; iter++)
        (*iter)->processXmppEvent(event);
    if (eventQueueEnabled)
        eventQueue.push_back(event);
}

void XmppEventTarget::addXmppEventListener(const XmppEventListener &listener)
{
    XmppEventListener *lsnr = (XmppEventListener *)&listener;
    std::vector<XmppEventListener *>::iterator iter;
    for (iter = listeners.begin(); iter != listeners.end() ; iter++)
        if (*iter == lsnr)
            return;
    listeners.push_back(lsnr);
}

void XmppEventTarget::removeXmppEventListener(const XmppEventListener &listener)
{
    XmppEventListener *lsnr = (XmppEventListener *)&listener;
    std::vector<XmppEventListener *>::iterator iter;
    for (iter = listeners.begin(); iter != listeners.end() ; iter++)
        if (*iter == lsnr)
            listeners.erase(iter);
}

void XmppEventTarget::clearXmppEventListeners()
{
    listeners.clear();
}


//###########################
//# E V E N T    Q U E U E
//###########################

void XmppEventTarget::eventQueueEnable(bool val)
{
    eventQueueEnabled = val;
    if (!eventQueueEnabled)
        eventQueue.clear();
}

int XmppEventTarget::eventQueueAvailable()
{
    return eventQueue.size();
}

XmppEvent XmppEventTarget::eventQueuePop()
{
    if (!eventQueueEnabled || eventQueue.size()<1)
        {
        XmppEvent dummy(XmppEvent::EVENT_NONE);
        return dummy;
        }
    XmppEvent event = *(eventQueue.begin());
    eventQueue.erase(eventQueue.begin());
    return event;
}





//########################################################################
//########################################################################
//# X M P P    S T R E A M
//########################################################################
//########################################################################


/**
 *
 */
class XmppStream
{
public:

    /**
     *
     */
    XmppStream()
        { reset(); }

    /**
     *
     */
    XmppStream(const XmppStream &other)
        { assign(other); }

    /**
     *
     */
    XmppStream &operator=(const XmppStream &other)
        { assign(other); return *this; }

    /**
     *
     */
    virtual ~XmppStream()
        {}

    /**
     *
     */
    virtual void reset()
        {
        state     = XmppClient::STREAM_AVAILABLE;
        seqNr     = 0;
        messageId = "";
        sourceId  = "";
        data.clear();
        }

    /**
     *
     */
    virtual int getState()
        { return state; }

    /**
     *
     */
    virtual void setState(int val)
        { state = val; }

    /**
     *
     */
    virtual DOMString getStreamId()
        { return streamId; }

    /**
     *
     */
    void setStreamId(const DOMString &val)
        { streamId = val; }

    /**
     *
     */
    virtual DOMString getMessageId()
        { return messageId; }

    /**
     *
     */
    void setMessageId(const DOMString &val)
        { messageId = val; }

    /**
     *
     */
    virtual int getSeqNr()
        {
        seqNr++;
        if (seqNr >= 65535)
            seqNr = 0;
        return seqNr;
        }

    /**
     *
     */
    virtual DOMString getPeerId()
        { return sourceId; }

    /**
     *
     */
    virtual void setPeerId(const DOMString &val)
        { sourceId = val; }

    /**
     *
     */
    int available()
        { return data.size(); }

    /**
     *
     */
    void receiveData(std::vector<unsigned char> &newData)
        {
        std::vector<unsigned char>::iterator iter;
        for (iter=newData.begin() ; iter!=newData.end() ; iter++)
            data.push_back(*iter);
        }

    /**
     *
     */
    std::vector<unsigned char> read()
        {
        if (state != XmppClient::STREAM_OPEN)
            {
            std::vector<unsigned char>dummy;
            return dummy;
            }
        std::vector<unsigned char> ret = data;
        data.clear();
        return ret;
        }

private:

    void assign(const XmppStream &other)
       {
       streamId  = other.streamId;
       messageId = other.messageId;
       sourceId  = other.sourceId;
       state     = other.state;
       seqNr     = other.seqNr;
       data      = other.data;
       }


    DOMString streamId;

    DOMString messageId;

    DOMString sourceId;

    int state;

    long seqNr;

    std::vector<unsigned char> data;
};










//########################################################################
//########################################################################
//# X M P P    C L I E N T
//########################################################################
//########################################################################

class ReceiverThread : public Runnable
{
public:

    ReceiverThread(XmppClient &par) : client(par) {}

    virtual ~ReceiverThread() {}

    void run()
      { client.receiveAndProcessLoop(); }

private:

    XmppClient &client;
};





//########################################################################
//# CONSTRUCTORS
//########################################################################

XmppClient::XmppClient()
{
    init();
}


XmppClient::XmppClient(const XmppClient &other) : XmppEventTarget(other)
{
    init();
    assign(other);
}

void XmppClient::assign(const XmppClient &other)
{
    msgId         = other.msgId;
    host          = other.host;
    realm         = other.realm;
    port          = other.port;
    username      = other.username;
    password      = other.password;
    resource      = other.resource;
    connected     = other.connected;
    doRegister    = other.doRegister;
    groupChats    = other.groupChats;
    streamPacket  = other.streamPacket;
}


void XmppClient::init()
{
    sock          = new TcpSocket();
    msgId         = 0;
    connected     = false;
    doRegister    = false;
    streamPacket  = "message";

}

XmppClient::~XmppClient()
{
    disconnect();
    delete sock;
    std::map<DOMString, XmppStream *>::iterator iter;
    for (iter = outputStreams.begin(); iter!=outputStreams.end() ; iter++)
        delete iter->second;
    for (iter = inputStreams.begin(); iter!=inputStreams.end() ; iter++)
        delete iter->second;
    for (iter = fileSends.begin(); iter!=fileSends.end() ; iter++)
        delete iter->second;
    groupChatsClear();
}






//########################################################################
//# UTILILY
//########################################################################

/**
 *
 */
bool XmppClient::pause(unsigned long millis)
{
    Thread::sleep(millis);
    return true;
}


static int strIndex(const DOMString &str, char *key)
{
    unsigned int p = str.find(key);
    if (p == str.npos)
        return -1;
    return p;
}


DOMString XmppClient::toXml(const DOMString &str)
{
    return Parser::encode(str);
}



static DOMString trim(const DOMString &str)
{
    unsigned int i;
    for (i=0 ; i<str.size() ; i++)
        if (!isspace(str[i]))
            break;
    int start = i;
    for (i=str.size() ; i>0 ; i--)
        if (!isspace(str[i-1]))
            break;
    int end = i;
    if (start>=end)
        return "";
    return str.substr(start, end);
}





//########################################################################
//# VARIABLES  (ones that need special handling)
//########################################################################

/**
 *
 */
DOMString XmppClient::getUsername()
{
    return username;
}

/**
 *
 */
void XmppClient::setUsername(const DOMString &val)
{
    int p = strIndex(val, "@");
    if (p > 0)
        {
        username = val.substr(0, p);
        realm    = val.substr(p+1, jid.size()-p-1);
        }
    else
       {
       realm    = host;
       username = val;
       }
}








//########################################################################
//# RECEIVING
//########################################################################


DOMString XmppClient::readStanza()
{

    int  openCount    = 0;
    bool inTag        = false;
    bool slashSeen    = false;
    bool trivialTag   = false;
    bool querySeen    = false;
    bool inQuote      = false;
    bool textSeen     = false;
    DOMString buf;


    time_t timeout = time((time_t *)0) + 180;

    while (true)
        {
        int ch = sock->read();
        //printf("%c", ch); fflush(stdout);
        if (ch<0)
            {
            if (ch == -2) //a simple timeout, not an error
                {
                //Since we are timed out, let's assume that we
                //are between chunks of text.  Let's reset all states.
                //printf("-----#### Timeout\n");
                time_t currentTime = time((time_t *)0);
                if (currentTime > timeout)
                    {
                    timeout = currentTime + 180;
                    if (!write("\n"))
                        {
                        error("ping send error");
                        disconnect();
                        return "";
                        }
                    }
                continue;
                }
            else
                {
                keepGoing = false;
                if (!sock->isConnected())
                    {
                    disconnect();
                    return "";
                    }
                else
                    {
                    error("socket read error: %s", sock->getLastError().c_str());
                    disconnect();
                    return "";
                    }
                }
            }
        buf.push_back(ch);
        if (ch == '<')
            {
            inTag      = true;
            slashSeen  = false;
            querySeen  = false;
            inQuote    = false;
            textSeen   = false;
            trivialTag = false;
            }
        else if (ch == '>')
            {
            if (!inTag)  //unescaped '>' in pcdata? horror
                continue;
            inTag     = false;
            if (!trivialTag && !querySeen)
                {
                if (slashSeen)
                    openCount--;
                else
                    openCount++;
                }
            //printf("# openCount:%d t:%d q:%d\n",
            //      openCount, trivialTag, querySeen);
            //check if we are 'balanced', but not a <?version?> tag
            if (openCount <= 0 && !querySeen)
                {
                break;
                }
            //we know that this one will be open-ended
            if (strIndex(buf, "<stream:stream") >= 0)
                {
                buf.append("</stream:stream>");
                break;
                }
            }
        else if (ch == '/')
            {
            if (inTag && !inQuote)
                {
                slashSeen = true;
                if (textSeen) // <tagName/>  <--looks like this
                    trivialTag = true;
                }
            }
        else if (ch == '?')
            {
            if (inTag && !inQuote)
                querySeen = true;
            }
        else if (ch == '"' || ch == '\'')
            {
            if (inTag)
                inQuote = !inQuote;
            }
        else
            {
            if (inTag && !inQuote && !isspace(ch))
                textSeen = true;
            }
        }
    return buf;
}



static bool isGroupChat(Element *root)
{
    if (!root)
        return false;
    ElementList elems = root->findElements("x");
    for (unsigned int i=0 ; i<elems.size() ; i++)
        {
        DOMString xmlns = elems[i]->getAttribute("xmlns");
        //printf("### XMLNS ### %s\n", xmlns.c_str());
        if (strIndex(xmlns, "http://jabber.org/protocol/muc") >=0 )
            return true;
        }
   return false;
}




static bool parseJid(const DOMString &fullJid,
             DOMString &jid, DOMString &resource)
{
    DOMString str = fullJid;
    jid.clear();
    resource.clear();
    unsigned int p = str.size();
    unsigned int p2 = str.rfind('/', p);
    if (p2 != str.npos)
        {
        resource = str.substr(p2+1, p-(p2+1));
        str = str.substr(0, p);
        p = p2;
        }
    jid = str.substr(0, p);
    printf("fullJid:%s jid:%s rsrc:%s\n",
        fullJid.c_str(), jid.c_str(), resource.c_str());
    return true;
}




bool XmppClient::processMessage(Element *root)
{
    DOMString from    = root->getTagAttribute("message", "from");
    DOMString to      = root->getTagAttribute("message", "to");
    DOMString type    = root->getTagAttribute("message", "type");

    //####Check for embedded namespaces here
    //### FILE TRANSFERS
    if (processFileMessage(root))
        return true;

    //### STREAMS
    if (processInBandByteStreamMessage(root))
        return true;


    //#### NORMAL MESSAGES
    DOMString subject = root->getTagValue("subject");
    DOMString body    = root->getTagValue("body");
    DOMString thread  = root->getTagValue("thread");
    //##rfc 3921, para 2.4.  ignore if no recognizable info
    //if (subject.size() < 1 && thread.size()<1)
    //    return true;

    if (type == "groupchat")
        {
        DOMString fromGid;
        DOMString fromNick;
        parseJid(from, fromGid, fromNick);
        //printf("fromGid:%s  fromNick:%s\n",
        //        fromGid.c_str(), fromNick.c_str());
        DOMString toGid;
        DOMString toNick;
        parseJid(to, toGid, toNick);
        //printf("toGid:%s  toNick:%s\n",
        //        toGid.c_str(), toNick.c_str());

        if (fromNick.size() > 0)//normal group message
            {
            XmppEvent event(XmppEvent::EVENT_MUC_MESSAGE);
            event.setGroup(fromGid);
            event.setFrom(fromNick);
            event.setData(body);
            event.setDOM(root);
            dispatchXmppEvent(event);
            }
        else // from the server itself
            {
            //note the space before, so it doesnt match 'unlocked'
            if (strIndex(body, " locked") >= 0)
                {
                printf("LOCKED!! ;)\n");
                char *fmt =
                "<iq id='create%d' to='%s' type='set'>"
                "<query xmlns='http://jabber.org/protocol/muc#owner'>"
                "<x xmlns='jabber:x:data' type='submit'/>"
                "</query></iq>\n";
                if (!write(fmt, msgId++, fromGid.c_str()))
                    return false;
                }
            }
        }
    else
        {
        XmppEvent event(XmppEvent::EVENT_MESSAGE);
        event.setFrom(from);
        event.setData(body);
        event.setDOM(root);
        dispatchXmppEvent(event);
        }

    return true;
}




bool XmppClient::processPresence(Element *root)
{

    DOMString fullJid     = root->getTagAttribute("presence", "from");
    DOMString to          = root->getTagAttribute("presence", "to");
    DOMString presenceStr = root->getTagAttribute("presence", "type");
    bool presence = true;
    if (presenceStr == "unavailable")
        presence = false;
    DOMString status      = root->getTagValue("status");
    DOMString show        = root->getTagValue("show");

    if (isGroupChat(root))
        {
        DOMString fromGid;
        DOMString fromNick;
        parseJid(fullJid, fromGid, fromNick);
        //printf("fromGid:%s  fromNick:%s\n",
        //        fromGid.c_str(), fromNick.c_str());
        DOMString item_jid = root->getTagAttribute("item", "jid");
        if (item_jid == jid || item_jid == to) //Me
            {
            if (presence)
                {
                groupChatCreate(fromGid);
                groupChatUserAdd(fromGid, fromNick, "");
                groupChatUserShow(fromGid, fromNick, "available");

                XmppEvent event(XmppEvent::EVENT_MUC_JOIN);
                event.setGroup(fromGid);
                event.setFrom(fromNick);
                event.setPresence(presence);
                event.setShow(show);
                event.setStatus(status);
                dispatchXmppEvent(event);
                }
            else
                {
                groupChatDelete(fromGid);
                groupChatUserDelete(fromGid, fromNick);

                XmppEvent event(XmppEvent::EVENT_MUC_LEAVE);
                event.setGroup(fromGid);
                event.setFrom(fromNick);
                event.setPresence(presence);
                event.setShow(show);
                event.setStatus(status);
                dispatchXmppEvent(event);
                }
            }
        else // someone else
            {
            if (presence)
                {
                groupChatUserAdd(fromGid, fromNick, "");
                }
            else
                groupChatUserDelete(fromGid, fromNick);
            groupChatUserShow(fromGid, fromNick, show);
            XmppEvent event(XmppEvent::EVENT_MUC_PRESENCE);
            event.setGroup(fromGid);
            event.setFrom(fromNick);
            event.setPresence(presence);
            event.setShow(show);
            event.setStatus(status);
            dispatchXmppEvent(event);
            }
        }
    else
        {
        DOMString shortJid;
        DOMString dummy;
        parseJid(fullJid, shortJid, dummy);
        rosterShow(shortJid, show); //users in roster do not have resource

        XmppEvent event(XmppEvent::EVENT_PRESENCE);
        event.setFrom(fullJid);
        event.setPresence(presence);
        event.setShow(show);
        event.setStatus(status);
        dispatchXmppEvent(event);
        }

    return true;
}



bool XmppClient::processIq(Element *root)
{
    DOMString from  = root->getTagAttribute("iq", "from");
    DOMString id    = root->getTagAttribute("iq", "id");
    DOMString type  = root->getTagAttribute("iq", "type");
    DOMString xmlns = root->getTagAttribute("query", "xmlns");

    if (id.size()<1)
        return true;

    //Group chat
    if (strIndex(xmlns, "http://jabber.org/protocol/muc") >=0 )
        {
        printf("results of MUC query\n");
        }
    //printf("###IQ xmlns:%s\n", xmlns.c_str());

    //### FILE TRANSFERS
    if (processFileMessage(root))
        return true;

    //### STREAMS
    if (processInBandByteStreamMessage(root))
        return true;
        

    //###Normal Roster stuff
    if (root->getTagAttribute("query", "xmlns") == "jabber:iq:roster")
        {
        roster.clear();
        ElementList elems = root->findElements("item");
        for (unsigned int i=0 ; i<elems.size() ; i++)
            {
            Element *item = elems[i];
            DOMString userJid      = item->getAttribute("jid");
            DOMString name         = item->getAttribute("name");
            DOMString subscription = item->getAttribute("subscription");
            DOMString group        = item->getTagValue("group");
            //printf("jid:%s name:%s sub:%s group:%s\n", userJid.c_str(), name.c_str(),
            //        subscription.c_str(), group.c_str());
            XmppUser user(userJid, name, subscription, group);
            roster.push_back(user);
            }
        XmppEvent event(XmppEvent::XmppEvent::EVENT_ROSTER);
        dispatchXmppEvent(event);
        }

    else if (id.find("regnew") != id.npos)
        {

        }

    else if (id.find("regpass") != id.npos)
        {
        ElementList list = root->findElements("error");
        if (list.size()==0)
            {
            XmppEvent evt(XmppEvent::EVENT_REGISTRATION_CHANGE_PASS);
            evt.setTo(username);
            evt.setFrom(host);
            dispatchXmppEvent(evt);
            return true;
            }

        Element *errElem = list[0];
        DOMString errMsg = "Password change error: ";
        if (errElem->findElements("bad-request").size()>0)
            {
            errMsg.append("password change does not contain complete information");
            }
        else if (errElem->findElements("not-authorized").size()>0)
            {
            errMsg.append("server does not consider the channel safe "
                          "enough to enable a password change");
            }
        else if (errElem->findElements("not-allowed").size()>0)
            {
            errMsg.append("server does not allow password changes");
            }
        else if (errElem->findElements("unexpected-request").size()>0)
            {
            errMsg.append(
             "IQ set does not contain a 'from' address because "
             "the entity is not registered with the server");
            }
        error("%s",(char *)errMsg.c_str());
        }

    else if (id.find("regcancel") != id.npos)
        {
        ElementList list = root->findElements("error");
        if (list.size()==0)
            {
            XmppEvent evt(XmppEvent::EVENT_REGISTRATION_CANCEL);
            evt.setTo(username);
            evt.setFrom(host);
            dispatchXmppEvent(evt);
            return true;
            }

        Element *errElem = list[0];
        DOMString errMsg = "Registration cancel error: ";
        if (errElem->findElements("bad-request").size()>0)
            {
            errMsg.append("The <remove/> element was not the only child element of the <query/> element.");
            }
        else if (errElem->findElements("forbidden").size()>0)
            {
            errMsg.append("sender does not have sufficient permissions to cancel the registration");
            }
        else if (errElem->findElements("not-allowed").size()>0)
            {
            errMsg.append("not allowed to cancel registrations in-band");
            }
        else if (errElem->findElements("registration-required").size()>0)
            {
            errMsg.append("not previously registered");
            }
        else if (errElem->findElements("unexpected-request").size()>0)
            {
            errMsg.append(
                 "IQ set does not contain a 'from' address because "
                 "the entity is not registered with the server");
            }
        error("%s",(char *)errMsg.c_str());
        }

    return true;
}





bool XmppClient::receiveAndProcess()
{
    if (!keepGoing)
        return false;

    Parser parser;

    DOMString recvBuf = readStanza();
    recvBuf = trim(recvBuf);
    if (recvBuf.size() < 1)
        return true;

    //Ugly hack.  Apparently the first char can be dropped on timeouts
    //if (recvBuf[0] != '<')
    //    recvBuf.insert(0, "<");

    status("RECV: %s", recvBuf.c_str());
    Element *root = parser.parse(recvBuf);
    if (!root)
        {
        printf("Bad elem\n");
        return true;
        }

    //#### MESSAGE
    ElementList elems = root->findElements("message");
    if (elems.size()>0)
        {
        if (!processMessage(root))
            return false;
        }

    //#### PRESENCE
    elems = root->findElements("presence");
    if (elems.size()>0)
        {
        if (!processPresence(root))
            return false;
        }

    //#### INFO
    elems = root->findElements("iq");
    if (elems.size()>0)
        {
        if (!processIq(root))
            return false;
        }

    delete root;

    return true;
}


bool XmppClient::receiveAndProcessLoop()
{
    keepGoing = true;
    while (true)
        {
        if (!keepGoing)
            {
            status("Abort requested");
            break;
            }
        if (!receiveAndProcess())
            return false;
        }
    return true;
}




//########################################################################
//# SENDING
//########################################################################


bool XmppClient::write(char *fmt, ...)
{
    bool rc = true;
    va_list args;
    va_start(args,fmt);
    gchar * buffer = g_strdup_vprintf(fmt,args);
    va_end(args) ;
    status("SEND: %s", buffer);
    if (!sock->write(buffer))
        {
        error("Cannot write to socket: %s", sock->getLastError().c_str());
        rc = false;
        }
    g_free(buffer);
    return rc;
}






//########################################################################
//# R E G I S T R A T I O N
//########################################################################

/**
 * Perform JEP-077 In-Band Registration.  Performed synchronously after SSL,
 * before authentication
 */
bool XmppClient::inBandRegistrationNew()
{
    Parser parser;

    char *fmt =
     "<iq type='get' id='regnew%d'>"
         "<query xmlns='jabber:iq:register'/>"
         "</iq>\n\n";
    if (!write(fmt, msgId++))
        return false;

    DOMString recbuf = readStanza();
    status("RECV reg: %s", recbuf.c_str());
    Element *elem = parser.parse(recbuf);
    //elem->print();

    //# does the entity send the newer "instructions" tag?
    ElementList fields = elem->findElements("field");
    std::vector<DOMString> fnames;
    for (unsigned int i=0; i<fields.size() ; i++)
        {
        DOMString fname = fields[i]->getAttribute("var");
        if (fname == "FORM_TYPE")
            continue;
        fnames.push_back(fname);
        status("field name:%s", fname.c_str());
        }

    //Do we have any fields?
    if (fnames.size() == 0)
        {
        //If no fields, maybe the older method was offered
        if (elem->findElements("username").size() == 0 ||
            elem->findElements("password").size() == 0)
            {
            error("server did not offer registration");
            delete elem;
            return false;
            }
        }

    delete elem;

    fmt =
     "<iq type='set' id='regnew%d'>"
         "<query xmlns='jabber:iq:register'>"
         "<username>%s</username>"
         "<password>%s</password>"
         "<email/><name/>"
         "</query>"
         "</iq>\n\n";
    if (!write(fmt, msgId++, toXml(username).c_str(),
                    toXml(password).c_str() ))
        return false;


    recbuf = readStanza();
    status("RECV reg: %s", recbuf.c_str());
    elem = parser.parse(recbuf);
    //elem->print();

    ElementList list = elem->findElements("error");
    if (list.size()>0)
        {
        Element *errElem = list[0];
        DOMString code = errElem->getAttribute("code");
        DOMString errMsg = "Registration error: ";
        if (code == "409")
            {
            errMsg.append("conflict with existing user name");
            }
        else if (code == "406")
            {
            errMsg.append("some registration information was not provided");
            }
        error("%s",(char *)errMsg.c_str());
        delete elem;
        return false;
        }

    delete elem;

    XmppEvent evt(XmppEvent::EVENT_REGISTRATION_NEW);
    evt.setTo(username);
    evt.setFrom(host);
    dispatchXmppEvent(evt);

    return true;
}


/**
 * Perform JEP-077 In-Band Registration.  Performed asynchronously, after login.
 * See processIq() for response handling.
 */
bool XmppClient::inBandRegistrationChangePassword(const DOMString &newpassword)
{
    Parser parser;

    //# Let's try it form-style to allow the common old/new password thing
    char *fmt =
      "<iq type='set' id='regpass%d' to='%s'>"
      "  <query xmlns='jabber:iq:register'>"
      "    <x xmlns='jabber:x:data' type='form'>"
      "      <field type='hidden' var='FORM_TYPE'>"
      "        <value>jabber:iq:register:changepassword</value>"
      "      </field>"
      "      <field type='text-single' var='username'>"
      "        <value>%s</value>"
      "      </field>"
      "      <field type='text-private' var='old_password'>"
      "        <value>%s</value>"
      "      </field>"
      "      <field type='text-private' var='password'>"
      "        <value>%s</value>"
      "      </field>"
      "    </x>"
      "  </query>"
      "</iq>\n\n";

    if (!write(fmt, msgId++, host.c_str(),
             username.c_str(), password.c_str(), newpassword.c_str()))
        return false;

    return true;

}


/**
 * Perform JEP-077 In-Band Registration.  Performed asynchronously, after login.
 * See processIq() for response handling.
 */
bool XmppClient::inBandRegistrationCancel()
{
    Parser parser;

    char *fmt =
     "<iq type='set' id='regcancel%d'>"
          "<query xmlns='jabber:iq:register'><remove/></query>"
          "</iq>\n\n";  
    if (!write(fmt, msgId++))
        return false;

    return true;
}





//########################################################################
//# A U T H E N T I C A T E
//########################################################################



bool XmppClient::iqAuthenticate(const DOMString &streamId)
{
    Parser parser;

    char *fmt =
    "<iq type='get' to='%s' id='auth%d'>"
    "<query xmlns='jabber:iq:auth'><username>%s</username></query>"
    "</iq>\n";
    if (!write(fmt, realm.c_str(), msgId++, username.c_str()))
        return false;

    DOMString recbuf = readStanza();
    status("iq auth recv: '%s'\n", recbuf.c_str());
    Element *elem = parser.parse(recbuf);
    //elem->print();
    DOMString iqType = elem->getTagAttribute("iq", "type");
    //printf("##iqType:%s\n", iqType.c_str());
    delete elem;

    if (iqType != "result")
        {
        error("error:server does not allow login");
        return false;
        }

    bool digest = true;
    if (digest)
        {
        //## Digest authentication
        DOMString digest = streamId;
        digest.append(password);
        digest = Sha1::hashHex(digest);
        //printf("digest:%s\n", digest.c_str());
        fmt =
        "<iq type='set' id='auth%d'>"
        "<query xmlns='jabber:iq:auth'>"
        "<username>%s</username>"
        "<digest>%s</digest>"
        "<resource>%s</resource>"
        "</query>"
        "</iq>\n";
        if (!write(fmt, msgId++, username.c_str(),
                    digest.c_str(), resource.c_str()))
            return false;
        }
    else
        {

        //## Plaintext authentication
        fmt =
        "<iq type='set' id='auth%d'>"
        "<query xmlns='jabber:iq:auth'>"
        "<username>%s</username>"
        "<password>%s</password>"
        "<resource>%s</resource>"
        "</query>"
        "</iq>\n";
        if (!write(fmt, msgId++, username.c_str(),
                   password.c_str(), resource.c_str()))
            return false;
        }

    recbuf = readStanza();
    status("iq auth recv:  '%s'\n", recbuf.c_str());
    elem = parser.parse(recbuf);
    //elem->print();
    iqType = elem->getTagAttribute("iq", "type");
    //printf("##iqType:%s\n", iqType.c_str());
    delete elem;

    if (iqType != "result")
        {
        error("server does not allow login");
        return false;
        }

    return true;
}


/**
 * Parse a sasl challenge to retrieve all of its key=value pairs
 */
static bool saslParse(const DOMString &s, 
                      std::map<DOMString, DOMString> &vals)
{

    vals.clear();

    int p  = 0;
    int siz = s.size();

    while (p < siz)
        {
        DOMString key;
        DOMString value;
        char ch = '\0';

        //# Parse key
        while (p<siz)
            {
            ch = s[p++];
            if (ch == '=')
                break;
            key.push_back(ch);
            }

        //No value?
        if (ch != '=')
            break;

        //# Parse value
        bool quoted = false;
        while (p<siz)
            {
            ch = s[p++];
            if (ch == '"')
                quoted = !quoted;
            else if (ch == ',' && !quoted)
                break;
            else
                value.push_back(ch);
            }

        //printf("# Key: '%s'  Value: '%s'\n", key.c_str(), value.c_str());
        vals[key] = value;
        if (ch != ',')
            break;
        }

    return true;
}



/**
 * Attempt suthentication using the MD5 SASL mechanism
 */
bool XmppClient::saslMd5Authenticate()
{
    Parser parser;
    char *fmt =
    "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' "
        "mechanism='DIGEST-MD5'/>\n";
    if (!write("%s",fmt))
        return false;

    DOMString recbuf = readStanza();
    status("challenge received: '%s'", recbuf.c_str());
    Element *elem = parser.parse(recbuf);
    //elem->print();
    DOMString b64challenge = elem->getTagValue("challenge");
    delete elem;

    if (b64challenge.size() < 1)
        {
        error("login: no SASL challenge offered by server");
        return false;
        }
    DOMString challenge = Base64Decoder::decodeToString(b64challenge);
    status("md5 challenge:'%s'", challenge.c_str());

    std::map<DOMString, DOMString> attrs;
    if (!saslParse(challenge, attrs))
        {
        error("login: error parsing SASL challenge");
        return false;
        }

    DOMString nonce = attrs["nonce"];
    if (nonce.size()==0)
        {
        error("login: no SASL nonce sent by server");
        return false;
        }

    DOMString realm = attrs["realm"];
    if (realm.size()==0)
        {
        //Apparently this is not a problem
        //error("login: no SASL realm sent by server");
        //return false;
        }

    status("SASL recv nonce: '%s' realm:'%s'\n", nonce.c_str(), realm.c_str());

    char idBuf[14];
    snprintf(idBuf, 13, "%dsasl", msgId++);
    DOMString cnonceStr = idBuf;
    DOMString cnonce = Sha1::hashHex(cnonceStr);
    DOMString authzid = username; authzid.append("@"); authzid.append(host);
    DOMString digest_uri = "xmpp/"; digest_uri.append(host);

    //## Make A1
    Md5 md5;
    md5.append(username);
    md5.append(":");
    md5.append(realm);
    md5.append(":");
    md5.append(password);
    unsigned char a1tmp[16];
    md5.finish(a1tmp);
    md5.init();
    md5.append(a1tmp, 16);
    md5.append(":");
    md5.append(nonce);
    md5.append(":");
    md5.append(cnonce);
    //RFC2831 says authzid is optional. Wildfire has trouble with authzid's
    //md5.append(":");
    //md5.append(authzid);
    md5.append("");
    DOMString a1 = md5.finishHex();
    status("##a1:'%s'", a1.c_str());

    //# Make A2
    md5.init();
    md5.append("AUTHENTICATE:");
    md5.append(digest_uri);
    DOMString a2 = md5.finishHex();
    status("##a2:'%s'", a2.c_str());

    //# Now make the response
    md5.init();
    md5.append(a1);
    md5.append(":");
    md5.append(nonce);
    md5.append(":");
    md5.append("00000001");//nc
    md5.append(":");
    md5.append(cnonce);
    md5.append(":");
    md5.append("auth");//qop
    md5.append(":");
    md5.append(a2);
    DOMString response = md5.finishHex();

    DOMString resp;
    resp.append("username=\""); resp.append(username); resp.append("\",");
    resp.append("realm=\"");    resp.append(realm);    resp.append("\",");
    resp.append("nonce=\"");    resp.append(nonce);    resp.append("\",");
    resp.append("cnonce=\"");   resp.append(cnonce);   resp.append("\",");
    resp.append("nc=00000001,qop=auth,");
    resp.append("digest-uri=\""); resp.append(digest_uri); resp.append("\"," );
    //resp.append("authzid=\"");  resp.append(authzid);  resp.append("\",");
    resp.append("response=");   resp.append(response); resp.append(",");
    resp.append("charset=utf-8");
    status("sending response:'%s'", resp.c_str());
    resp = Base64Encoder::encode(resp);
    status("base64 response:'%s'", resp.c_str());
    fmt =
    "<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>%s</response>\n";
    if (!write(fmt, resp.c_str()))
        return false;

    recbuf = readStanza();
    status("server says: '%s'", recbuf.c_str());
    elem = parser.parse(recbuf);
    //elem->print();
    //# Success or failure already?
    if (elem->findElements("success").size() > 0)
        {
        delete elem;
        return true;
        }
    else
        {
        ElementList list = elem->findElements("failure");
        if (list.size() > 0)
            {
            DOMString errmsg = "";
            Element *errmsgElem = list[0]->getFirstChild();
            if (errmsgElem)
                errmsg = errmsgElem->getName();
            error("login: initial md5 authentication failed: %s", errmsg.c_str());
            delete elem;
            return false;
            }
        }
    //# Continue for one more SASL cycle
    b64challenge = elem->getTagValue("challenge");
    delete elem;

    if (b64challenge.size() < 1)
        {
        error("login: no second SASL challenge offered by server");
        return false;
        }

    challenge = Base64Decoder::decodeToString(b64challenge);
    status("md5 challenge: '%s'", challenge.c_str());

    if (!saslParse(challenge, attrs))
        {
        error("login: error parsing SASL challenge");
        return false;
        }

    DOMString rspauth = attrs["rspauth"];
    if (rspauth.size()==0)
        {
        error("login: no SASL respauth sent by server\n");
        return false;
        }

    fmt =
    "<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>\n";
    if (!write("%s",fmt))
        return false;

    recbuf = readStanza();
    status("SASL recv: '%s", recbuf.c_str());
    elem = parser.parse(recbuf);
    //elem->print();
    b64challenge = elem->getTagValue("challenge");
    bool success = (elem->findElements("success").size() > 0);
    delete elem;

    return success;
}



/**
 *  Attempt to authentication using the SASL PLAIN mechanism.  This
 *  is used most commonly my Google Talk.
 */
bool XmppClient::saslPlainAuthenticate()
{
    Parser parser;

    DOMString id = username;
    //id.append("@");
    //id.append(host);
    Base64Encoder encoder;
    encoder.append('\0');
    encoder.append(id);
    encoder.append('\0');
    encoder.append(password);
    DOMString base64Auth = encoder.finish();
    //printf("authbuf:%s\n", base64Auth.c_str());

    char *fmt =
    "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' "
    "mechanism='PLAIN'>%s</auth>\n";
    if (!write(fmt, base64Auth.c_str()))
        return false;
    DOMString recbuf = readStanza();
    status("challenge received: '%s'", recbuf.c_str());
    Element *elem = parser.parse(recbuf);

    bool success = (elem->findElements("success").size() > 0);
    delete elem;

    return success;
}



/**
 * Handshake with SASL, and use one of its offered mechanisms to
 * authenticate.
 * @param streamId used for iq auth fallback is SASL not supported
 */
bool XmppClient::saslAuthenticate(const DOMString &streamId)
{
    Parser parser;

    DOMString recbuf = readStanza();
    status("RECV: '%s'\n", recbuf.c_str());
    Element *elem = parser.parse(recbuf);
    //elem->print();

    //Check for starttls
    bool wantStartTls = false;
    if (elem->findElements("starttls").size() > 0)
        {
        wantStartTls = true;
        if (elem->findElements("required").size() > 0)
            status("login: STARTTLS required");
        else
            status("login: STARTTLS available");
        }

    //# do we want TLS, are we not already running SSL, and can
    //# the client actually do an ssl connection?
    if (wantStartTls && !sock->getEnableSSL() && sock->getHaveSSL())
        {
        delete elem;
        char *fmt =
        "<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>\n";
        if (!write("%s",fmt))
            return false;
        recbuf = readStanza();
        status("RECV: '%s'\n", recbuf.c_str());
        elem = parser.parse(recbuf);
        if (elem->getTagAttribute("proceed", "xmlns").size()<1)
            {
            error("Server rejected TLS negotiation");
            disconnect();
            return false;
            }
        delete elem;
        if (!sock->startTls())
            {
            DOMString tcperr = sock->getLastError();
            error("Could not start TLS: %s", tcperr.c_str());
            disconnect();
            return false;
            }

        fmt =
         "<stream:stream xmlns='jabber:client' "
         "xmlns:stream='http://etherx.jabber.org/streams' "
         "to='%s' version='1.0'>\n\n";
        if (!write(fmt, realm.c_str()))
            return false;

        recbuf = readStanza();
        status("RECVx: '%s'", recbuf.c_str());
        recbuf.append("</stream:stream>");
        elem = parser.parse(recbuf);
        bool success =
        (elem->getTagAttribute("stream:stream", "id").size()>0);
        if (!success)
            {
            error("STARTTLS negotiation failed");
            disconnect();
            return false;
            }
        delete elem;
        recbuf = readStanza();
        status("RECV: '%s'\n", recbuf.c_str());
        elem = parser.parse(recbuf);

        XmppEvent event(XmppEvent::EVENT_SSL_STARTED);
        dispatchXmppEvent(event);
        }

    //register, if user requests
    if (doRegister)
        {
        if (!inBandRegistrationNew())
            return false;
        }

    //check for sasl authentication mechanisms
    ElementList elems = elem->findElements("mechanism");
    if (elems.size() < 1)
        {
        status("login: no SASL mechanism offered by server");
        //fall back to iq
        if (iqAuthenticate(streamId))
            return true;
        return false;
        }
    bool md5Found = false;
    bool plainFound = false;
    for (unsigned int i=0 ; i<elems.size() ; i++)
        {
        DOMString mech = elems[i]->getValue();
        if (mech == "DIGEST-MD5")
            {
            status("MD5 authentication offered");
            md5Found = true;
            }
        else if (mech == "PLAIN")
            {
            status("PLAIN authentication offered");
            plainFound = true;
            }
        }
    delete elem;

    bool success = false;
    if (md5Found)
        {
        success = saslMd5Authenticate();
        }
    else if (plainFound)
        {
        success = saslPlainAuthenticate();
        }
    else
        {
        error("not able to handle sasl authentication mechanisms");
        return false;
        }

    if (success)
        status("###### SASL authentication success\n");
    else
        error("###### SASL authentication failure\n");

    return success;
}






//########################################################################
//# CONNECT
//########################################################################


/**
 * Check if we are connected, and fail with an error if we are not
 */
bool XmppClient::checkConnect()
{
    if (!connected)
        {
        XmppEvent evt(XmppEvent::EVENT_ERROR);
        evt.setData("Attempted operation while disconnected");
        dispatchXmppEvent(evt);
        return false;
        }
    return true;
}



/**
 * Create an XMPP session with a server.  This
 * is basically the transport layer of XMPP.
 */
bool XmppClient::createSession()
{

    Parser parser;
    if (port==443 || port==5223)
        sock->enableSSL(true);
    if (!sock->connect(host, port))
        {
        error("Cannot connect:%s", sock->getLastError().c_str());
        return false;
        }

    if (sock->getEnableSSL())
        {
        XmppEvent event(XmppEvent::EVENT_SSL_STARTED);
        dispatchXmppEvent(event);
        }

    char *fmt =
     "<stream:stream "
          "to='%s' "
          "xmlns='jabber:client' "
          "xmlns:stream='http://etherx.jabber.org/streams' "
          "version='1.0'>\n\n";
    if (!write(fmt, realm.c_str()))
        return false;

    DOMString recbuf = readStanza();
    status("RECV:  '%s'\n", recbuf.c_str());
    recbuf.append("</stream:stream>");
    Element *elem = parser.parse(recbuf);
    //elem->print();
    bool useSasl = false;
    DOMString streamId = elem->getTagAttribute("stream:stream", "id");
    //printf("### StreamID: %s\n", streamId.c_str());
    DOMString streamVersion = elem->getTagAttribute("stream:stream", "version");
    if (streamVersion == "1.0")
        useSasl = true;

    if (useSasl)
        {
        if (!saslAuthenticate(streamId))
            return false;

        fmt =
          "<stream:stream "
          "to='%s' "
          "xmlns='jabber:client' "
          "xmlns:stream='http://etherx.jabber.org/streams' "
          "version='1.0'>\n\n";

        if (!write(fmt, realm.c_str()))
            return false;
        recbuf = readStanza();
        recbuf.append("</stream:stream>\n");
        //printf("now server says:: '%s'\n", recbuf.c_str());
        elem = parser.parse(recbuf);
        //elem->print();
        delete elem;

        recbuf = readStanza();
        //printf("now server says:: '%s'\n", recbuf.c_str());
        elem = parser.parse(recbuf);
        bool hasBind = (elem->findElements("bind").size() > 0);
        //elem->print();
        delete elem;

        if (!hasBind)
            {
            error("no binding provided by server");
            return false;
            }


        }
    else // not SASL
        {
        if (!iqAuthenticate(streamId))
            return false;
        }


    //### Resource binding
    fmt =
    "<iq type='set' id='bind%d'>"
    "<bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'>"
    "<resource>%s</resource>"
    "</bind></iq>\n";
    if (!write(fmt, msgId++, resource.c_str()))
        return false;

    recbuf = readStanza();
    status("bind result: '%s'", recbuf.c_str());
    elem = parser.parse(recbuf);
    //elem->print();
    DOMString bindType = elem->getTagAttribute("iq", "type");
    //printf("##bindType:%s\n", bindType.c_str());
    DOMString givenFullJid = elem->getTagValue("jid");
    delete elem;

    if (bindType != "result")
        {
        error("no binding with server failed");
        return false;
        }
        
    //The server sent us a JID.  We need to listen.
    if (givenFullJid.size()>0)
        {
        DOMString givenJid, givenResource;
        parseJid(givenFullJid, givenJid, givenResource);
        status("given user: %s realm: %s, rsrc: %s",
           givenJid.c_str(), realm.c_str(), givenResource.c_str());
        setResource(givenResource);
        }
        

    fmt =
    "<iq type='set' id='sess%d'>"
    "<session xmlns='urn:ietf:params:xml:ns:xmpp-session'/>"
    "</iq>\n";
    if (!write(fmt, msgId++))
        return false;

    recbuf = readStanza();
    status("session received: '%s'", recbuf.c_str());
    elem = parser.parse(recbuf);
    //elem->print();
    DOMString sessionType = elem->getTagAttribute("iq", "type");
    //printf("##sessionType:%s\n", sessionType.c_str());
    delete elem;

    if (sessionType != "result")
        {
        error("no session provided by server");
        return false;
        }

    //printf("########## COOL #########\n");
    //Now that we are bound, we have a valid JID
    jid = username;
    jid.append("@");
    jid.append(realm);
    jid.append("/");
    jid.append(resource);

    //We are now done with the synchronous handshaking.  Let's go into
    //async mode

    fmt =
     "<iq type='get' id='roster%d'><query xmlns='jabber:iq:roster'/></iq>\n";
    if (!write(fmt, msgId++))
        return false;

    fmt =
     "<iq type='get' id='discoItems%d' to='%s'>"
     "<query xmlns='http://jabber.org/protocol/disco#items'/></iq>\n";
    if (!write(fmt, msgId++, realm.c_str()))
        return false;

    fmt =
    "<iq type='get' id='discoInfo%d' to='conference.%s'>"
    "<query xmlns='http://jabber.org/protocol/disco#info'/></iq>\n";
    if (!write(fmt, msgId++, realm.c_str()))
        return false;

    fmt =
     "<presence/>\n";
    if (!write("%s",fmt))
        return false;

    /*
    recbuf = readStanza();
    status("stream received: '%s'", recbuf.c_str());
    elem = parser.parse(recbuf);
    //elem->print();
    delete elem;
    */

    //We are now logged in
    status("Connected");
    connected = true;
    XmppEvent evt(XmppEvent::EVENT_CONNECTED);
    evt.setData(host);
    dispatchXmppEvent(evt);
    //Thread::sleep(1000000);

    sock->setReceiveTimeout(1000);
    ReceiverThread runner(*this);
    Thread thread(runner);
    thread.start();

    return true;
}



/**
 * Public call to connect
 */
bool XmppClient::connect()
{
    if (!createSession())
        {
        disconnect();
        return false;
        }
    return true;
}


/**
 * Public call to connect
 */
bool XmppClient::connect(DOMString hostArg, int portArg,
                         DOMString usernameArg,
                         DOMString passwordArg,
                         DOMString resourceArg)
{
    host     = hostArg;
    port     = portArg;
    password = passwordArg;
    resource = resourceArg;

    //parse this one
    setUsername(usernameArg);

    bool ret = connect();
    return ret;
}



/**
 * Public call to disconnect
 */
bool XmppClient::disconnect()
{
    if (connected)
        {
        char *fmt =
        "<presence type='unavailable'/>\n";
        write("%s",fmt);
        }
    keepGoing = false;
    connected = false;
    Thread::sleep(2000); //allow receiving thread to quit
    sock->disconnect();
    roster.clear();
    groupChatsClear();
    XmppEvent event(XmppEvent::EVENT_DISCONNECTED);
    event.setData(host);
    dispatchXmppEvent(event);
    return true;
}





//########################################################################
//# ROSTER
//########################################################################

/**
 *  Add an XMPP id to your roster
 */
bool XmppClient::rosterAdd(const DOMString &rosterGroup,
                           const DOMString &otherJid,
                           const DOMString &name)
{
    if (!checkConnect())
        return false;
    char *fmt =
    "<iq type='set' id='roster_%d'>"
    "<query xmlns='jabber:iq:roster'>"
    "<item jid='%s' name='%s'><group>%s</group></item>"
    "</query></iq>\n";
    if (!write(fmt, msgId++, otherJid.c_str(),
         name.c_str(), rosterGroup.c_str()))
        {
        return false;
        }
    return true;
}



/**
 *  Delete an XMPP id from your roster.
 */
bool XmppClient::rosterDelete(const DOMString &otherJid)
{
    if (!checkConnect())
        return false;
    char *fmt =
    "<iq type='set' id='roster_%d'>"
    "<query xmlns='jabber:iq:roster'>"
    "<item jid='%s' subscription='remove'><group>%s</group></item>"
    "</query></iq>\n";
    if (!write(fmt, msgId++, otherJid.c_str()))
        {
        return false;
        }
    return true;
}


/**
 *  Comparison method for sort() call below
 */
static bool xmppRosterCompare(const XmppUser& p1, const XmppUser& p2)
{
    DOMString s1 = p1.group;
    DOMString s2 = p2.group;
    for (unsigned int len=0 ; len<s1.size() && len<s2.size() ; len++)
        {
        int comp = tolower(s1[len]) - tolower(s2[len]);
        if (comp)
            return (comp<0);
        }

    s1 = p1.jid;
    s2 = p2.jid;
    for (unsigned int len=0 ; len<s1.size() && len<s2.size() ; len++)
        {
        int comp = tolower(s1[len]) - tolower(s2[len]);
        if (comp)
            return (comp<0);
        }
    return false;
}



/**
 *  Sort and return the roster that has just been reported by
 *  an XmppEvent::EVENT_ROSTER event.
 */
std::vector<XmppUser> XmppClient::getRoster()
{
    std::vector<XmppUser> ros = roster;
    std::sort(ros.begin(), ros.end(), xmppRosterCompare);
    return ros;
}


/**
 *
 */
void XmppClient::rosterShow(const DOMString &jid, const DOMString &show)
{
    DOMString theShow = show;
    if (theShow == "")
        theShow = "available";

    std::vector<XmppUser>::iterator iter;
    for (iter=roster.begin() ; iter != roster.end() ; iter++)
        {
        if (iter->jid == jid)
            iter->show = theShow;
        }
}






//########################################################################
//# CHAT (individual)
//########################################################################

/**
 * Send a message to an xmpp jid
 */
bool XmppClient::message(const DOMString &user, const DOMString &subj,
                         const DOMString &msg)
{
    if (!checkConnect())
        return false;

    DOMString xmlSubj = toXml(subj);
    DOMString xmlMsg  = toXml(msg);

    if (xmlSubj.size() > 0)
        {
        char *fmt =
        "<message to='%s' from='%s' type='chat'>"
        "<subject>%s</subject><body>%s</body></message>\n";
        if (!write(fmt, user.c_str(), jid.c_str(),
                xmlSubj.c_str(), xmlMsg.c_str()))
            return false;
        }
    else
        {
        char *fmt =
        "<message to='%s' from='%s'>"
        "<body>%s</body></message>\n";
        if (!write(fmt, user.c_str(), jid.c_str(), xmlMsg.c_str()))
            return false;
        }
    return true;
}



/**
 *
 */
bool XmppClient::message(const DOMString &user, const DOMString &msg)
{
    return message(user, "", msg);
}



/**
 *
 */
bool XmppClient::presence(const DOMString &presence)
{
    if (!checkConnect())
        return false;

    DOMString xmlPres = toXml(presence);

    char *fmt =
    "<presence><show>%s</show></presence>\n";
    if (!write(fmt, xmlPres.c_str()))
        return false;
    return true;
}






//########################################################################
//# GROUP  CHAT
//########################################################################

/**
 *
 */
bool XmppClient::groupChatCreate(const DOMString &groupJid)
{
    std::vector<XmppGroupChat *>::iterator iter;
    for (iter=groupChats.begin() ; iter!=groupChats.end() ; iter++)
        {
        if ((*iter)->getGroupJid() == groupJid)
            {
            //error("Group chat '%s' already exists", groupJid.c_str());
            return false;
            }
        }
    XmppGroupChat *chat = new XmppGroupChat(groupJid);
    groupChats.push_back(chat);
    return true;
}



/**
 *
 */
void XmppClient::groupChatDelete(const DOMString &groupJid)
{
    std::vector<XmppGroupChat *>::iterator iter;
    for (iter=groupChats.begin() ; iter!=groupChats.end() ; )
        {
        XmppGroupChat *chat = *iter;
        if (chat->getGroupJid() == groupJid)
            {
            iter = groupChats.erase(iter);
            delete chat;
            }
        else
            iter++;
        }
}



/**
 *
 */
bool XmppClient::groupChatExists(const DOMString &groupJid)
{
    std::vector<XmppGroupChat *>::iterator iter;
    for (iter=groupChats.begin() ; iter!=groupChats.end() ; iter++)
        if ((*iter)->getGroupJid() == groupJid)
            return true;
    return false;
}



/**
 *
 */
void XmppClient::groupChatsClear()
{
    std::vector<XmppGroupChat *>::iterator iter;
    for (iter=groupChats.begin() ; iter!=groupChats.end() ; iter++)
        delete (*iter);
    groupChats.clear();
}




/**
 *
 */
void XmppClient::groupChatUserAdd(const DOMString &groupJid,
                                  const DOMString &nick,
                                  const DOMString &jid)
{
    std::vector<XmppGroupChat *>::iterator iter;
    for (iter=groupChats.begin() ; iter!=groupChats.end() ; iter++)
        {
        if ((*iter)->getGroupJid() == groupJid)
            {
            (*iter)->userAdd(nick, jid);
            }
        }
}



/**
 *
 */
void XmppClient::groupChatUserShow(const DOMString &groupJid,
                                   const DOMString &nick,
                                   const DOMString &show)
{
    std::vector<XmppGroupChat *>::iterator iter;
    for (iter=groupChats.begin() ; iter!=groupChats.end() ; iter++)
        {
        if ((*iter)->getGroupJid() == groupJid)
            {
            (*iter)->userShow(nick, show);
            }
        }
}




/**
 *
 */
void XmppClient::groupChatUserDelete(const DOMString &groupJid,
                                     const DOMString &nick)
{
    std::vector<XmppGroupChat *>::iterator iter;
    for (iter=groupChats.begin() ; iter!=groupChats.end() ; iter++)
        {
        if ((*iter)->getGroupJid() == groupJid)
            {
            (*iter)->userDelete(nick);
            }
        }
}



/**
 *  Comparison method for the sort() below
 */
static bool xmppUserCompare(const XmppUser& p1, const XmppUser& p2)
{
    DOMString s1 = p1.nick;
    DOMString s2 = p2.nick;
    int comp = 0;
    for (unsigned int len=0 ; len<s1.size() && len<s2.size() ; len++)
        {
        comp = tolower(s1[len]) - tolower(s2[len]);
        if (comp)
            break;
        }
    return (comp<0);
}



/**
 *  Return the user list for the named group
 */
std::vector<XmppUser> XmppClient::groupChatGetUserList(
                              const DOMString &groupJid)
{
    if (!checkConnect())
        {
        std::vector<XmppUser> dummy;
        return dummy;
        }

    std::vector<XmppGroupChat *>::iterator iter;
    for (iter=groupChats.begin() ; iter!=groupChats.end() ; iter++)
        {
        if ((*iter)->getGroupJid() == groupJid )
            {
            std::vector<XmppUser> uList = (*iter)->getUserList();
            std::sort(uList.begin(), uList.end(), xmppUserCompare);
            return uList;
            }
        }
    std::vector<XmppUser> dummy;
    return dummy;
}




/**
 *  Try to join a group
 */
bool XmppClient::groupChatJoin(const DOMString &groupJid,
                               const DOMString &nick,
                               const DOMString &pass)
{
    if (!checkConnect())
        return false;

    DOMString user = nick;
    if (user.size()<1)
        user = username;

    char *fmt =
    "<presence to='%s/%s'>"
    "<x xmlns='http://jabber.org/protocol/muc'/></presence>\n";
    if (!write(fmt, groupJid.c_str(), user.c_str()))
        return false;
    return true;
}




/**
 * Leave a group
 */
bool XmppClient::groupChatLeave(const DOMString &groupJid,
                                const DOMString &nick)
{
    if (!checkConnect())
        return false;

    DOMString user = nick;
    if (user.size()<1)
        user = username;

    char *fmt =
    "<presence to='%s/%s' type='unavailable'>"
    "<x xmlns='http://jabber.org/protocol/muc'/></presence>\n";
    if (!write(fmt, groupJid.c_str(), user.c_str()))
        return false;
    return true;
}




/**
 *  Send a message to a group
 */
bool XmppClient::groupChatMessage(const DOMString &groupJid,
                                  const DOMString &msg)
{
    if (!checkConnect())
        {
        return false;
        }

    DOMString xmlMsg = toXml(msg);

    char *fmt =
    "<message from='%s' to='%s' type='groupchat'>"
    "<body>%s</body></message>\n";
    if (!write(fmt, jid.c_str(), groupJid.c_str(), xmlMsg.c_str()))
        return false;
    /*
    char *fmt =
    "<message to='%s' type='groupchat'>"
    "<body>%s</body></message>\n";
    if (!write(fmt, groupJid.c_str(), xmlMsg.c_str()))
        return false;
    */
    return true;
}




/**
 *  Send a message to an individual in a group
 */
bool XmppClient::groupChatPrivateMessage(const DOMString &groupJid,
                                         const DOMString &toNick,
                                         const DOMString &msg)
{
    if (!checkConnect())
        return false;

    DOMString xmlMsg = toXml(msg);

    /*
    char *fmt =
    "<message from='%s' to='%s/%s' type='chat'>"
    "<body>%s</body></message>\n";
    if (!write(fmt, jid.c_str(), groupJid.c_str(),
               toNick.c_str(), xmlMsg.c_str()))
        return false;
    */
    char *fmt =
    "<message to='%s/%s' type='chat'>"
    "<body>%s</body></message>\n";
    if (!write(fmt, groupJid.c_str(),
               toNick.c_str(), xmlMsg.c_str()))
        return false;
    return true;
}




/**
 *  Change your presence within a group
 */
bool XmppClient::groupChatPresence(const DOMString &groupJid,
                                   const DOMString &myNick,
                                   const DOMString &presence)
{
    if (!checkConnect())
        return false;

    DOMString user = myNick;
    if (user.size()<1)
        user = username;

    DOMString xmlPresence = toXml(presence);

    char *fmt =
    "<presence to='%s/%s' type='%s'>"
    "<x xmlns='http://jabber.org/protocol/muc'/></presence>\n";
    if (!write(fmt, groupJid.c_str(),
               user.c_str(), xmlPresence.c_str()))
        return true;
    return true;
}





//########################################################################
//# S T R E A M S
//########################################################################


bool XmppClient::processInBandByteStreamMessage(Element *root)
{
    DOMString from  = root->getAttribute("from");
    DOMString id    = root->getAttribute("id");
    DOMString type  = root->getAttribute("type");

    //### Incoming stream requests
    //Input streams are id's by stream id
    DOMString ibbNamespace = "http://jabber.org/protocol/ibb";

    if (root->getTagAttribute("open", "xmlns") == ibbNamespace)
        {
        DOMString streamId = root->getTagAttribute("open", "sid");
        XmppEvent event(XmppEvent::XmppEvent::EVENT_STREAM_RECEIVE_INIT);
        dispatchXmppEvent(event);
        std::map<DOMString, XmppStream *>::iterator iter =
                     inputStreams.find(streamId);
        if (iter != inputStreams.end())
            {
            XmppStream *ins = iter->second;
            ins->setState(STREAM_OPENING);
            ins->setMessageId(id);
            return true;
            }
        return true;
        }

    else if (root->getTagAttribute("close", "xmlns") == ibbNamespace)
        {
        XmppEvent event(XmppEvent::XmppEvent::EVENT_STREAM_RECEIVE_CLOSE);
        dispatchXmppEvent(event);
        DOMString streamId = root->getTagAttribute("close", "sid");
        std::map<DOMString, XmppStream *>::iterator iter =
                     inputStreams.find(streamId);
        if (iter != inputStreams.end())
            {
            XmppStream *ins = iter->second;
            if (from == ins->getPeerId())
                {
                ins->setState(STREAM_CLOSING);
                ins->setMessageId(id);
                return true;
                }
            }
        return true;
        }

    else if (root->getTagAttribute("data", "xmlns") == ibbNamespace)
        {
        DOMString streamId = root->getTagAttribute("data", "sid");
        std::map<DOMString, XmppStream *>::iterator iter =
                     inputStreams.find(streamId);
        if (iter != inputStreams.end())
            {
            XmppStream *ins = iter->second;
            if (ins->getState() != STREAM_OPEN)
                {
                XmppEvent event(XmppEvent::EVENT_ERROR);
                event.setFrom(from);
                event.setData("received unrequested stream data");
                dispatchXmppEvent(event);
                return true;
                }
            DOMString data = root->getTagValue("data");
            std::vector<unsigned char>binData =
                       Base64Decoder::decode(data);
            ins->receiveData(binData);
            }
        }

    //### Responses to outgoing requests
    //Output streams are id's by message id
    std::map<DOMString, XmppStream *>::iterator iter =
                     outputStreams.find(id);
    if (iter != outputStreams.end())
        {
        XmppStream *outs = iter->second;
        if (type == "error")
            {
            outs->setState(STREAM_ERROR);
            return true;
            }
        else if (type == "result")
            {
            if (outs->getState() == STREAM_OPENING)
                {
                outs->setState(STREAM_OPEN);
                }
            else if (outs->getState() == STREAM_CLOSING)
                {
                outs->setState(STREAM_CLOSED);
                }
            return true;
            }
        }

    return false;
}


/**
 *
 */
bool XmppClient::outputStreamOpen(const DOMString &destId,
                                 const DOMString &streamIdArg)
{
    char buf[32];
    snprintf(buf, 31, "inband%d", getMsgId());
    DOMString messageId = buf;

    //Output streams are id's by message id
    XmppStream *outs = new XmppStream();
    outputStreams[messageId] = outs;

    outs->setState(STREAM_OPENING);

    DOMString streamId = streamIdArg;
    if (streamId.size()<1)
        {
        snprintf(buf, 31, "stream%d", getMsgId());
        DOMString streamId = buf;
        }
    outs->setMessageId(messageId);
    outs->setStreamId(streamId);
    outs->setPeerId(destId);


    char *fmt =
    "<%s type='set' to='%s' id='%s'>"
    "<open sid='%s' block-size='4096'"
    " xmlns='http://jabber.org/protocol/ibb'/></%s>\n";
    if (!write(fmt,
              streamPacket.c_str(),
              destId.c_str(), messageId.c_str(),
              streamId.c_str(),
              streamPacket.c_str()))
        {
        outs->reset();
        return -1;
        }

    int state = outs->getState();
    for (int tim=0 ; tim<20 ; tim++)
        {
        if (state == STREAM_OPEN)
            break;
        else if (state == STREAM_ERROR)
            {
            printf("ERROR\n");
            outs->reset();
            return false;
            }
        Thread::sleep(1000);
        state = outs->getState();
        }
    if (state != STREAM_OPEN)
        {
        printf("TIMEOUT ERROR\n");
        outs->reset();
        return -1;
        }

    return true;
}

/**
 *
 */
bool XmppClient::outputStreamWrite(const DOMString &streamId,
               const std::vector<unsigned char> &buf)
{
    std::map<DOMString, XmppStream *>::iterator iter =
        outputStreams.find(streamId);
    if (iter == outputStreams.end())
        return false;
    XmppStream *outs = iter->second;

    unsigned int len = buf.size();
    unsigned int pos = 0;

    while (pos < len)
        {
        unsigned int pos2 = pos + 1024;
        if (pos2>len)
            pos2 = len;

        Base64Encoder encoder;
        for (unsigned int i=pos ; i<pos2 ; i++)
            encoder.append(buf[i]);
        DOMString b64data = encoder.finish();


        char *fmt =
        "<message to='%s' id='msg%d'>"
        "<data xmlns='http://jabber.org/protocol/ibb' sid='%s' seq='%d'>"
        "%s"
        "</data>"
        "<amp xmlns='http://jabber.org/protocol/amp'>"
        "<rule condition='deliver-at' value='stored' action='error'/>"
        "<rule condition='match-resource' value='exact' action='error'/>"
        "</amp>"
        "</message>\n";
        if (!write(fmt,
              outs->getPeerId().c_str(),
              getMsgId(),
              outs->getStreamId().c_str(),
              outs->getSeqNr(),
              b64data.c_str()))
            {
            outs->reset();
            return false;
            }
        pause(5000);
        
        pos = pos2;
        }
        
    return true;
}

/**
 *
 */
bool XmppClient::outputStreamClose(const DOMString &streamId)
{
    std::map<DOMString, XmppStream *>::iterator iter =
        outputStreams.find(streamId);
    if (iter == outputStreams.end())
        return false;
    XmppStream *outs = iter->second;

    char buf[32];
    snprintf(buf, 31, "inband%d", getMsgId());
    DOMString messageId = buf;
    outs->setMessageId(messageId);

    outs->setState(STREAM_CLOSING);
    char *fmt =
    "<%s type='set' to='%s' id='%s'>"
    "<close sid='%s' xmlns='http://jabber.org/protocol/ibb'/></%s>\n";
    if (!write(fmt,
            streamPacket.c_str(),
            outs->getPeerId().c_str(),
            messageId.c_str(),
            outs->getStreamId().c_str(),
            streamPacket.c_str()
            ))
        return false;

    int state = outs->getState();
    for (int tim=0 ; tim<20 ; tim++)
        {
        if (state == STREAM_CLOSED)
            break;
        else if (state == STREAM_ERROR)
            {
            printf("ERROR\n");
            outs->reset();
            return false;
            }
        Thread::sleep(1000);
        state = outs->getState();
        }
    if (state != STREAM_CLOSED)
        {
        printf("TIMEOUT ERROR\n");
        outs->reset();
        return false;
        }

    delete outs;
    outputStreams.erase(streamId);

    return true;
}


/**
 *
 */
bool XmppClient::inputStreamOpen(const DOMString &fromJid,
                                const DOMString &streamId,
                                const DOMString &iqId)
{
    XmppStream *ins = new XmppStream();

    inputStreams[streamId] = ins;
    ins->reset();
    ins->setPeerId(fromJid);
    ins->setState(STREAM_CLOSED);
    ins->setStreamId(streamId);

    int state = ins->getState();
    for (int tim=0 ; tim<20 ; tim++)
        {
        if (state == STREAM_OPENING)
            break;
        else if (state == STREAM_ERROR)
            {
            printf("ERROR\n");
            ins->reset();
            return false;
            }
        Thread::sleep(1000);
        state = ins->getState();
        }
    if (state != STREAM_OPENING)
        {
        printf("TIMEOUT ERROR\n");
        ins->reset();
        return false;
        }
    char *fmt =
    "<%s type='result' to='%s' id='%s'/>\n";
    if (!write(fmt, streamPacket.c_str(),
           fromJid.c_str(), ins->getMessageId().c_str()))
        {
        return false;
        }

    ins->setState(STREAM_OPEN);
    return true;
}



/**
 *
 */
bool XmppClient::inputStreamClose(const DOMString &streamId)
{
    std::map<DOMString, XmppStream *>::iterator iter =
        inputStreams.find(streamId);
    if (iter == inputStreams.end())
        return false;
    XmppStream *ins = iter->second;

    if (ins->getState() == STREAM_CLOSING)
        {
        char *fmt =
        "<iq type='result' to='%s' id='%s'/>\n";
        if (!write(fmt, ins->getPeerId().c_str(),
                    ins->getMessageId().c_str()))
            {
            return false;
            }
        }
    inputStreams.erase(streamId);
    delete ins;

    return true;
}






//########################################################################
//# FILE   TRANSFERS
//########################################################################


bool XmppClient::processFileMessage(Element *root)
{
    DOMString siNamespace = "http://jabber.org/protocol/si";
    if (root->getTagAttribute("si", "xmlns") != siNamespace)
        return false;

    
    Element *mainElement = root->getFirstChild();
    if (!mainElement)
        return false;

    DOMString from  = mainElement->getAttribute("from");
    DOMString id    = mainElement->getAttribute("id");
    DOMString type  = mainElement->getAttribute("type");

    status("received file message from %s", from.c_str());

    if (type == "set")
        {
        DOMString streamId = root->getTagAttribute("si", "id");
        DOMString fname    = root->getTagAttribute("file", "name");
        DOMString sizeStr  = root->getTagAttribute("file", "size");
        DOMString hash     = root->getTagAttribute("file", "hash");
        XmppEvent event(XmppEvent::XmppEvent::EVENT_FILE_RECEIVE);
        event.setFrom(from);
        event.setIqId(id);
        event.setStreamId(streamId);
        event.setFileName(fname);
        event.setFileHash(hash);
        event.setFileSize(atol(sizeStr.c_str()));
        dispatchXmppEvent(event);
        return true;
        }

    //##expecting result or error
    //file sends id'd by message id's
    std::map<DOMString, XmppStream *>::iterator iter =
        fileSends.find(id);
    if (iter != fileSends.end())
        {
        XmppStream *outf = iter->second;
        if (from != outf->getPeerId())
            return true;
        if (type == "error")
            {
            outf->setState(STREAM_ERROR);
            error("user '%s' rejected file", from.c_str());
            return true;
            }
        else if (type == "result")
            {
            if (outf->getState() == STREAM_OPENING)
                {
                XmppEvent event(XmppEvent::XmppEvent::EVENT_FILE_ACCEPTED);
                event.setFrom(from);
                dispatchXmppEvent(event);
                outf->setState(STREAM_OPEN);
                }
            else if (outf->getState() == STREAM_CLOSING)
                {
                outf->setState(STREAM_CLOSED);
                }
            return true;
            }
        }

    return true;
}






/**
 *
 */
bool XmppClient::fileSend(const DOMString &destJidArg,
                          const DOMString &offeredNameArg,
                          const DOMString &fileNameArg,
                          const DOMString &descriptionArg)
{
    DOMString destJid     = destJidArg;
    DOMString offeredName = offeredNameArg;
    DOMString fileName    = fileNameArg;
    DOMString description = descriptionArg;

    struct stat finfo;
    if (stat(fileName.c_str(), &finfo)<0)
        {
        error("Cannot stat file '%s' for sending", fileName.c_str());
        return false;
        }
    long fileLen = finfo.st_size;
    if (!fileLen > 1000000)
        {
        error("'%s' too large", fileName.c_str());
        return false;
        }
    if (!S_ISREG(finfo.st_mode))
        {
        error("'%s' is not a regular file", fileName.c_str());
        return false;
        }
    FILE *f = fopen(fileName.c_str(), "rb");
    if (!f)
        {
        error("cannot open '%s' for sending", fileName.c_str());
        return false;
        }
    std::vector<unsigned char> sendBuf;
    Md5 md5hash;
    for (long i=0 ; i<fileLen && !feof(f); i++)
        {
        int ch = fgetc(f);
        if (ch<0)
            break;
        md5hash.append((unsigned char)ch);
        sendBuf.push_back((unsigned char)ch);
        }
    fclose(f);
    DOMString hash = md5hash.finishHex();
    printf("Hash:%s\n", hash.c_str());

   
    //## get the last path segment from the whole path
    if (offeredName.size()<1)
        {
        int slashPos = -1;
        for (unsigned int i=0 ; i<fileName.size() ; i++)
            {
            int ch = fileName[i];
            if (ch == '/' || ch == '\\')
                slashPos = i;
            }
        if (slashPos>=0 && slashPos<=(int)(fileName.size()-1))
            {
            offeredName = fileName.substr(slashPos+1,
                                          fileName.size()-slashPos-1);
            printf("offeredName:%s\n", offeredName.c_str());
            }
        }

    char buf[32];
    snprintf(buf, 31, "file%d", getMsgId());
    DOMString messageId = buf;

    XmppStream *outf = new XmppStream();

    outf->setState(STREAM_OPENING);
    outf->setMessageId(messageId);
    fileSends[messageId] = outf;

    snprintf(buf, 31, "stream%d", getMsgId());
    DOMString streamId = buf;
    //outf->setStreamId(streamId);

    outf->setPeerId(destJid);

    char dtgBuf[81];
    struct tm *timeVal = gmtime(&(finfo.st_mtime));
    strftime(dtgBuf, 80, "%Y-%m-%dT%H:%M:%Sz", timeVal);

    char *fmt =
    "<%s type='set' id='%s' to='%s'>"
    "<si xmlns='http://jabber.org/protocol/si' id='%s'"
      " mime-type='text/plain'"
      " profile='http://jabber.org/protocol/si/profile/file-transfer'>"
    "<file xmlns='http://jabber.org/protocol/si/profile/file-transfer'"
          " name='%s' size='%d' hash='%s' date='%s'><desc>%s</desc></file>"
    "<feature xmlns='http://jabber.org/protocol/feature-neg'>"
    "<x xmlns='jabber:x:data' type='form'>"
    "<field var='stream-method' type='list-single'>"
    //"<option><value>http://jabber.org/protocol/bytestreams</value></option>"
    "<option><value>http://jabber.org/protocol/ibb</value></option>"
    "</field></x></feature></si></%s>\n";
    if (!write(fmt, streamPacket.c_str(),
         messageId.c_str(), destJid.c_str(),
         streamId.c_str(), offeredName.c_str(), fileLen,
         hash.c_str(), dtgBuf, description.c_str(),
         streamPacket.c_str()))
        {
        return false;
        }

    int ret = true;
    int state = outf->getState();
    for (int tim=0 ; tim<20 ; tim++)
        {
        printf("##### waiting for open\n");
        if (state == STREAM_OPEN)
            {
            outf->reset();
            break;
            }
        else if (state == STREAM_ERROR)
            {
            printf("ERROR\n");
            outf->reset();
            ret = false;
            }
        Thread::sleep(1000);
        state = outf->getState();
        }
    if (state != STREAM_OPEN)
        {
        printf("TIMEOUT ERROR\n");
        ret = false;
        }

    //free up this resource
    fileSends.erase(messageId);
    delete outf;

    if (!outputStreamOpen(destJid, streamId))
        {
        error("cannot open output stream %s", streamId.c_str());
        return false;
        }

    if (!outputStreamWrite(streamId, sendBuf))
        {
        }

    if (!outputStreamClose(streamId))
        {
        }

    return true;
}


class FileSendThread : public Thread
{
public:

    FileSendThread(XmppClient &par,
                   const DOMString &destJidArg,
                   const DOMString &offeredNameArg,
                   const DOMString &fileNameArg,
                   const DOMString &descriptionArg) : client(par)
        {
        destJid     = destJidArg;
        offeredName = offeredNameArg;
        fileName    = fileNameArg;
        description = descriptionArg;
        }

    virtual ~FileSendThread() {}

    void run()
      {
      client.fileSend(destJid, offeredName,
                      fileName, description);
      }

private:

    XmppClient &client;
    DOMString destJid;
    DOMString offeredName;
    DOMString fileName;
    DOMString description;
};

/**
 *
 */
bool XmppClient::fileSendBackground(const DOMString &destJid,
                                    const DOMString &offeredName,
                                    const DOMString &fileName,
                                    const DOMString &description)
{
    FileSendThread thread(*this, destJid, offeredName,
                           fileName, description);
    thread.start();
    return true;
}


/**
 *
 */
bool XmppClient::fileReceive(const DOMString &fromJid,
                             const DOMString &iqId,
                             const DOMString &streamId,
                             const DOMString &fileName,
                             long  fileSize,
                             const DOMString &fileHash)
{
    char *fmt =
    "<%s type='result' to='%s' id='%s'>"
    "<si xmlns='http://jabber.org/protocol/si'>"
    "<file xmlns='http://jabber.org/protocol/si/profile/file-transfer'/>"
    "<feature xmlns='http://jabber.org/protocol/feature-neg'>"
    "<x xmlns='jabber:x:data' type='submit'>"
    "<field var='stream-method'>"
    "<value>http://jabber.org/protocol/ibb</value>"
    "</field></x></feature></si></%s>\n";
    if (!write(fmt, streamPacket.c_str(),
                    fromJid.c_str(), iqId.c_str(),
                    streamPacket.c_str()))
        {
        return false;
        }

    if (!inputStreamOpen(fromJid, streamId, iqId))
        {
        return false;
        }

    XmppStream *ins = inputStreams[streamId];

    Md5 md5;
    FILE *f = fopen(fileName.c_str(), "wb");
    if (!f)
        {
        return false;
        }

    while (true)
        {
        if (ins->available()<1)
            {
            if (ins->getState() == STREAM_CLOSING)
                break;
            pause(100);
            continue;
            }
        std::vector<unsigned char> ret = ins->read();
        std::vector<unsigned char>::iterator iter;
        for (iter=ret.begin() ; iter!=ret.end() ; iter++)
            {
            unsigned char ch = *iter;
            md5.append(&ch, 1);
            fwrite(&ch, 1, 1, f);
            }
        }

    inputStreamClose(streamId);
    fclose(f);

    DOMString hash = md5.finishHex();
    printf("received file hash:%s\n", hash.c_str());

    return true;
}



class FileReceiveThread : public Thread
{
public:

    FileReceiveThread(XmppClient &par,
                      const DOMString &fromJidArg,
                      const DOMString &iqIdArg,
                      const DOMString &streamIdArg,
                      const DOMString &fileNameArg,
                      long  fileSizeArg,
                      const DOMString &fileHashArg) : client(par)
        {
        fromJid     = fromJidArg;
        iqId        = iqIdArg;
        streamId    = streamIdArg;
        fileName    = fileNameArg;
        fileSize    = fileSizeArg;
        fileHash    = fileHashArg;
        }

    virtual ~FileReceiveThread() {}

    void run()
      {
      client.fileReceive(fromJid, iqId, streamId,
                        fileName, fileSize, fileHash);
      }

private:

    XmppClient &client;
    DOMString fromJid;
    DOMString iqId;
    DOMString streamId;
    DOMString fileName;
    long      fileSize;
    DOMString fileHash;
};

/**
 *
 */
bool XmppClient::fileReceiveBackground(const DOMString &fromJid,
                                       const DOMString &iqId,
                                       const DOMString &streamId,
                                       const DOMString &fileName,
                                       long  fileSize,
                                       const DOMString &fileHash)
{
    FileReceiveThread thread(*this, fromJid, iqId, streamId,
                  fileName, fileSize, fileHash);
    thread.start();
    return true;
}



//########################################################################
//# X M P P    G R O U P    C H A T
//########################################################################

/**
 *
 */
XmppGroupChat::XmppGroupChat(const DOMString &groupJidArg)
{
    groupJid = groupJidArg;
}

/**
 *
 */
XmppGroupChat::XmppGroupChat(const XmppGroupChat &other)
{
    groupJid = other.groupJid;
    userList = other.userList;
}

/**
 *
 */
XmppGroupChat::~XmppGroupChat()
{
}


/**
 *
 */
DOMString XmppGroupChat::getGroupJid()
{
    return groupJid;
}


void XmppGroupChat::userAdd(const DOMString &nick,
                            const DOMString &jid)
{
    std::vector<XmppUser>::iterator iter;
    for (iter= userList.begin() ; iter!=userList.end() ; iter++)
        {
        if (iter->nick == nick)
            return;
        }
    XmppUser user(jid, nick);
    userList.push_back(user);
}

void XmppGroupChat::userShow(const DOMString &nick,
                             const DOMString &show)
{
    DOMString theShow = show;
    if (theShow == "")
        theShow = "available"; // a join message will now have a show
    std::vector<XmppUser>::iterator iter;
    for (iter= userList.begin() ; iter!=userList.end() ; iter++)
        {
        if (iter->nick == nick)
            iter->show = theShow;
        }
}

void XmppGroupChat::userDelete(const DOMString &nick)
{
    std::vector<XmppUser>::iterator iter;
    for (iter= userList.begin() ; iter!=userList.end() ; )
        {
        if (iter->nick == nick)
            iter = userList.erase(iter);
        else
            iter++;
        }
}

std::vector<XmppUser> XmppGroupChat::getUserList() const
{
    return userList;
}









} //namespace Pedro
//########################################################################
//# E N D    O F     F I L E
//########################################################################















