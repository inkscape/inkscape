#ifndef __XMPP_H__
#define __XMPP_H__
/*
 * API for the Pedro mini-XMPP client.
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

#include <stdio.h>
#include <vector>

#include <string>

#include "pedrodom.h"

namespace Pedro
{

typedef std::string DOMString;


//########################################################################
//# X M P P    E V E N T
//########################################################################
class XmppUser
{
public:
    XmppUser()
        {
        }
    XmppUser(const DOMString &jidArg, const DOMString &nickArg)
        {
        jid  = jidArg;
        nick = nickArg;
        }
    XmppUser(const DOMString &jidArg,          const DOMString &nickArg,
             const DOMString &subscriptionArg, const DOMString &groupArg)
        {
        jid          = jidArg;
        nick         = nickArg;
        subscription = subscriptionArg;
        group        = groupArg;
        }
    XmppUser(const XmppUser &other)
        {
        jid          = other.jid;
        nick         = other.nick;
        subscription = other.subscription;
        group        = other.group;
        show         = other.show;
        }
    XmppUser &operator=(const XmppUser &other)
        {
        jid          = other.jid;
        nick         = other.nick;
        subscription = other.subscription;
        group        = other.group;
        show         = other.show;
        return *this;
        }
    virtual ~XmppUser()
        {}
    DOMString jid;
    DOMString nick;
    DOMString subscription;
    DOMString group;
    DOMString show;
};

class XmppEvent
{

public:

typedef enum
    {
    EVENT_NONE,
    EVENT_STATUS,
    EVENT_ERROR,
    EVENT_CONNECTED,
    EVENT_DISCONNECTED,
    EVENT_PRESENCE,
    EVENT_REGISTRATION_NEW,
    EVENT_ROSTER,
    EVENT_MESSAGE,
    EVENT_MUC_JOIN,
    EVENT_MUC_LEAVE,
    EVENT_MUC_PRESENCE,
    EVENT_MUC_MESSAGE,
    EVENT_STREAM_RECEIVE_INIT,
    EVENT_STREAM_RECEIVE,
    EVENT_STREAM_RECEIVE_CLOSE,
    EVENT_FILE_ACCEPTED,
    EVENT_FILE_RECEIVE
    } XmppEventType;


    /**
     *
     */
    XmppEvent(int type);

    /**
     *
     */
    XmppEvent(const XmppEvent &other);

    /**
     *
     */
    virtual XmppEvent &operator=(const XmppEvent &other);

    /**
     *
     */
    virtual ~XmppEvent();

    /**
     *
     */
    virtual void assign(const XmppEvent &other);

    /**
     *
     */
    virtual int getType() const;


    /**
     *
     */
    virtual DOMString getIqId() const;


    /**
     *
     */
    virtual void setIqId(const DOMString &val);

    /**
     *
     */
    virtual DOMString getStreamId() const;


    /**
     *
     */
    virtual void setStreamId(const DOMString &val);

    /**
     *
     */
    virtual bool getPresence() const;


    /**
     *
     */
    virtual void setPresence(bool val);

    /**
     *
     */
    virtual DOMString getShow() const;


    /**
     *
     */
    virtual void setShow(const DOMString &val);

    /**
     *
     */
    virtual DOMString getStatus() const;

    /**
     *
     */
    virtual void setStatus(const DOMString &val);

    /**
     *
     */
    virtual DOMString getTo() const;

    /**
     *
     */
    virtual void setTo(const DOMString &val);

    /**
     *
     */
    virtual DOMString getFrom() const;

    /**
     *
     */
    virtual void setFrom(const DOMString &val);

    /**
     *
     */
    virtual DOMString getGroup() const;

    /**
     *
     */
    virtual void setGroup(const DOMString &val);

    /**
     *
     */
    virtual Element *getDOM() const;


    /**
     *
     */
    virtual void setDOM(const Element *val);

    /**
     *
     */
    virtual std::vector<XmppUser> getUserList() const;

    /**
     *
     */
    virtual void setUserList(const std::vector<XmppUser> &userList);

    /**
     *
     */
    virtual DOMString getFileName() const;


    /**
     *
     */
    virtual void setFileName(const DOMString &val);


    /**
     *
     */
    virtual DOMString getFileDesc() const;


    /**
     *
     */
    virtual void setFileDesc(const DOMString &val);


    /**
     *
     */
    virtual long getFileSize() const;


    /**
     *
     */
    virtual void setFileSize(long val);

    /**
     *
     */
    virtual DOMString getFileHash() const;

    /**
     *
     */
    virtual void setFileHash(const DOMString &val);

    /**
     *
     */
    virtual DOMString getData() const;


    /**
     *
     */
    virtual void setData(const DOMString &val);

private:

    int eventType;

    DOMString iqId;

    DOMString streamId;

    bool      presence;

    DOMString show;

    DOMString status;

    DOMString to;

    DOMString from;

    DOMString group;

    DOMString data;

    DOMString fileName;
    DOMString fileDesc;
    long      fileSize;
    DOMString fileHash;

    Element *dom;

    std::vector<XmppUser>userList;

};


//########################################################################
//# X M P P    E V E N T    L I S T E N E R
//########################################################################

class XmppEventListener
{
public:

    /**
     *
     */
    XmppEventListener()
        {}

    /**
     *
     */
    XmppEventListener(const XmppEventListener &other)
        {}


    /**
     *
     */
    virtual ~XmppEventListener()
        {}

    /**
     *
     */
    virtual void processXmppEvent(const XmppEvent &event)
        {}

};



//########################################################################
//# X M P P    E V E N T    T A R G E T
//########################################################################

class XmppEventTarget
{
public:

    /**
     *
     */
    XmppEventTarget();

    /**
     *
     */
    XmppEventTarget(const XmppEventTarget &other);

    /**
     *
     */
    virtual ~XmppEventTarget();


    //###########################
    //# M E S S A G E S
    //###########################


    /**
     * Send an error message to all subscribers
     */
    void error(char *fmt, ...);


    /**
     * Send a status message to all subscribers
     */
    void status(char *fmt, ...);

    //###########################
    //# LISTENERS
    //###########################

    /**
     *
     */
    virtual void dispatchXmppEvent(const XmppEvent &event);

    /**
     *
     */
    virtual void addXmppEventListener(const XmppEventListener &listener);

    /**
     *
     */
    virtual void removeXmppEventListener(const XmppEventListener &listener);

    /**
     *
     */
    virtual void clearXmppEventListeners();

    /**
     *
     */
    void eventQueueEnable(bool val);

    /**
     *
     */
    int eventQueueAvailable();

    /**
     *
     */
    XmppEvent eventQueuePop();


private:

    std::vector<XmppEventListener *> listeners;

    std::vector<XmppEvent> eventQueue;
    bool eventQueueEnabled;

    static const int targetWriteBufLen = 2048;

    char targetWriteBuf[targetWriteBufLen];
};





//########################################################################
//# X M P P    C L I E N T
//########################################################################


class TcpSocket;
class XmppChat;
class XmppGroupChat;
class XmppStream;

class XmppClient : public XmppEventTarget
{

public:

    //###########################
    //# CONSTRUCTORS
    //###########################

    /**
     *
     */
    XmppClient();

    /**
     *
     */
    XmppClient(const XmppClient &other);

    /**
     *
     */
    void assign(const XmppClient &other);

    /**
     *
     */
    virtual ~XmppClient();


    //###########################
    //# UTILITY
    //###########################

    /**
     *
     */
    virtual bool pause(unsigned long millis);

    /**
     *
     */
    DOMString toXml(const DOMString &str);

    //###########################
    //# CONNECTION
    //###########################

    /**
     *
     */
    virtual bool connect();

    /**
     *
     */
    virtual bool connect(DOMString host, int port,
                         DOMString userName,
                         DOMString password,
                         DOMString resource);

    /**
     *
     */
    virtual bool disconnect();


    /**
     *
     */
    virtual bool write(char *fmt, ...);

    //#######################
    //# V A R I A B L E S
    //#######################

    /**
     *
     */
    virtual bool isConnected()
        { return connected; }

    /**
     *
     */
    virtual DOMString getHost()
        { return host; }

    /**
     *
     */
    virtual void setHost(const DOMString &val)
        { host = val; }

    /**
     *
     */
    virtual DOMString getRealm()
        { return realm; }

    /**
     *
     */
    virtual void setRealm(const DOMString &val)
        { realm = val; }

    /**
     *
     */
    virtual int getPort()
        { return port; }

    /**
     *
     */
    virtual void setPort(int val)
        { port = val; }

    /**
     *
     */
    virtual DOMString getUsername();

    /**
     *
     */
    virtual void setUsername(const DOMString &val);

    /**
     *
     */
    virtual DOMString getPassword()
        { return password; }

    /**
     *
     */
    virtual void setPassword(const DOMString &val)
        { password = val; }

    /**
     *
     */
    virtual DOMString getResource()
        { return resource; }

    /**
     *
     */
    virtual void setResource(const DOMString &val)
        { resource = val; }

    /**
     *
     */
    virtual DOMString getJid()
        { return jid; }

    /**
     *
     */
    virtual int getMsgId()
        { return msgId++; }

    /**
     *
     */
    virtual void setDoRegister(bool val)
        { doRegister = val; }


    /**
     *
     */
    virtual bool getDoRegister()
        { return doRegister; }



    //#######################
    //# P R O C E S S I N G
    //#######################


    /**
     *
     */
    bool processMessage(Element *root);

    /**
     *
     */
    bool processPresence(Element *root);

    /**
     *
     */
    bool processIq(Element *root);

    /**
     *
     */
    virtual bool receiveAndProcess();

    /**
     *
     */
    virtual bool receiveAndProcessLoop();

    //#######################
    //# ROSTER
    //#######################

    /**
     *
     */
    bool rosterAdd(const DOMString &rosterGroup,
                   const DOMString &otherJid,
                   const DOMString &name);

    /**
     *
     */
    bool rosterDelete(const DOMString &otherJid);

    /**
     *
     */
    std::vector<XmppUser> getRoster();

    /**
     *
     */
    virtual void rosterShow(const DOMString &jid, const DOMString &show);

    //#######################
    //# CHAT (individual)
    //#######################

    /**
     *
     */
    virtual bool message(const DOMString &user, const DOMString &subj,
                         const DOMString &text);

    /**
     *
     */
    virtual bool message(const DOMString &user, const DOMString &text);

    /**
     *
     */
    virtual bool presence(const DOMString &presence);

    //#######################
    //# GROUP CHAT
    //#######################

    /**
     *
     */
    virtual bool groupChatCreate(const DOMString &groupJid);

    /**
     *
     */
    virtual void groupChatDelete(const DOMString &groupJid);

    /**
     *
     */
    bool groupChatExists(const DOMString &groupJid);

    /**
     *
     */
    virtual void groupChatsClear();

    /**
     *
     */
    virtual void groupChatUserAdd(const DOMString &groupJid,
                                  const DOMString &nick,
                                  const DOMString &jid);
    /**
     *
     */
    virtual void groupChatUserShow(const DOMString &groupJid,
                                   const DOMString &nick,
                                   const DOMString &show);

    /**
     *
     */
    virtual void groupChatUserDelete(const DOMString &groupJid,
                                     const DOMString &nick);

    /**
     *
     */
    virtual std::vector<XmppUser>
          XmppClient::groupChatGetUserList(const DOMString &groupJid);

    /**
     *
     */
    virtual bool groupChatJoin(const DOMString &groupJid,
                               const DOMString &nick,
                               const DOMString &pass);

    /**
     *
     */
    virtual bool groupChatLeave(const DOMString &groupJid,
                                const DOMString &nick);

    /**
     *
     */
    virtual bool groupChatMessage(const DOMString &groupJid,
                                  const DOMString &msg);

    /**
     *
     */
    virtual bool groupChatPrivateMessage(const DOMString &groupJid,
                                         const DOMString &toNick,
                                         const DOMString &msg);

    /**
     *
     */
    virtual bool groupChatPresence(const DOMString &groupJid,
                                   const DOMString &nick,
                                   const DOMString &presence);


    //#######################
    //# STREAMS
    //#######################

    typedef enum
        {
        STREAM_AVAILABLE,
        STREAM_OPENING,
        STREAM_OPEN,
        STREAM_CLOSING,
        STREAM_CLOSED,
        STREAM_ERROR
        } StreamStates;

    /**
     *
     */
    virtual int outputStreamOpen(const DOMString &jid,
                                 const DOMString &streamId);

    /**
     *
     */
    virtual int outputStreamWrite(int streamId,
                          const unsigned char *buf, unsigned long len);

    /**
     *
     */
    virtual int outputStreamClose(int streamId);

    /**
     *
     */
    virtual int inputStreamOpen(const DOMString &jid,
                                const DOMString &streamId,
                                const DOMString &iqId);

    /**
     *
     */
    virtual int inputStreamAvailable(int streamId);

    /**
     *
     */
    virtual std::vector<unsigned char> inputStreamRead(int streamId);

    /**
     *
     */
    virtual bool inputStreamClosing(int streamId);

    /**
     *
     */
    virtual int inputStreamClose(int streamId);


    //#######################
    //# FILE   TRANSFERS
    //#######################

    /**
     *
     */
    virtual bool fileSend(const DOMString &destJid,
                          const DOMString &offeredName,
                          const DOMString &fileName,
                          const DOMString &description);

    /**
     *
     */
    virtual bool fileSendBackground(const DOMString &destJid,
                                    const DOMString &offeredName,
                                    const DOMString &fileName,
                                    const DOMString &description);

    /**
     *
     */
    virtual bool fileReceive(const DOMString &fromJid,
                             const DOMString &iqId,
                             const DOMString &streamId,
                             const DOMString &fileName,
                             long  fileSize,
                             const DOMString &fileHash);
    /**
     *
     */
    virtual bool fileReceiveBackground(const DOMString &fromJid,
                                       const DOMString &iqId,
                                       const DOMString &streamId,
                                       const DOMString &fileName,
                                       long  fileSize,
                                       const DOMString &fileHash);


private:

    void init();

    DOMString host;

    /**
     * will be same as host, unless username is
     * user@realm
     */
    DOMString realm;

    int port;

    DOMString username;

    DOMString password;

    DOMString resource;

    DOMString jid;

    int msgId;

    TcpSocket *sock;

    bool connected;

    bool createSession();

    bool checkConnect();

    DOMString readStanza();

    bool saslMd5Authenticate();

    bool saslPlainAuthenticate();

    bool saslAuthenticate();

    bool iqAuthenticate(const DOMString &streamId);

    bool inBandRegistration();

    bool keepGoing;

    bool doRegister;

    static const int writeBufLen = 2048;

    unsigned char writeBuf[writeBufLen];

    std::vector<XmppGroupChat *>groupChats;

    static const int outputStreamCount = 16;

    XmppStream *outputStreams[outputStreamCount];

    static const int inputStreamCount = 16;

    XmppStream *inputStreams[inputStreamCount];

    static const int fileSendCount = 16;

    XmppStream *fileSends[fileSendCount];

    std::vector<XmppUser>roster;
};




//########################################################################
//# X M P P    G R O U P    C H A T
//########################################################################

/**
 *
 */
class XmppGroupChat
{
public:

    /**
     *
     */
    XmppGroupChat(const DOMString &groupJid);

    /**
     *
     */
    XmppGroupChat(const XmppGroupChat &other);

    /**
     *
     */
    virtual ~XmppGroupChat();

    /**
     *
     */
    virtual DOMString getGroupJid();

    /**
     *
     */
    virtual void userAdd(const DOMString &nick,
                         const DOMString &jid);
    /**
     *
     */
    virtual void userShow(const DOMString &nick,
                          const DOMString &show);

    /**
     *
     */
    virtual void userDelete(const DOMString &nick);

    /**
     *
     */
    virtual std::vector<XmppUser> getUserList() const;


private:

    DOMString groupJid;

    std::vector<XmppUser>userList;

};










} //namespace Pedro

#endif /* __XMPP_H__ */

//########################################################################
//# E N D    O F     F I L E
//########################################################################

