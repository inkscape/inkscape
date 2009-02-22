#ifndef __XMPP_H__
#define __XMPP_H__
/*
 * API for the Pedro mini-XMPP client.
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

#include <stdio.h>
#include <glib.h>
#include <vector>
#include <map>

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





/**
 * Class that emits information from a client
 */
class XmppEvent
{

public:

    /**
     * People might want to refer to these docs to understand
     * the XMPP terminology used here.
     * http://www.ietf.org/rfc/rfc3920.txt      -- Xmpp Core
     * http://www.ietf.org/rfc/rfc3921.txt      -- Messaging and presence
     * http://www.jabber.org/jeps/jep-0077.html -- In-Band registration
     * http://www.jabber.org/jeps/jep-0045.html -- Multiuser Chat
     * http://www.jabber.org/jeps/jep-0047.html -- In-Band byte streams
     * http://www.jabber.org/jeps/jep-0096.html -- File transfer
     */

    /**
     *  No event type.  Default
     */
    static const int EVENT_NONE                 =  0;

    /**
     *  Client emits a status message.  Message is in getData().
     */
    static const int EVENT_STATUS               =  1;

    /**
     *  Client emits an error message.  Message is in getData().
     */
    static const int EVENT_ERROR                =  2;

    /**
     *  Client has connected to a host.  Host name is in getData().
     */
    static const int EVENT_CONNECTED            = 10;

    /**
     *  Client has disconnected from a host.  Host name is in getData().
     */
    static const int EVENT_DISCONNECTED         = 11;

    /**
     *  Client has begun speaking to the server in SSL.  This is usually
     *  emitted just before EVENT_CONNECTED,  since authorization has not
     *  yet taken place.
     */
    static const int EVENT_SSL_STARTED          = 12;

    /**
     *  Client has successfully registered a new account on a server.
     *  The server is in getFrom(), the user in getTo()
     */
    static const int EVENT_REGISTRATION_NEW     = 20;

    /**
     *  Client has successfully changed the password of an existing account on a server.
     *  The server is in getFrom(), the user in getTo()
     */
    static const int EVENT_REGISTRATION_CHANGE_PASS  = 21;

    /**
     *  Client has successfully cancelled an existing account on a server.
     *  The server is in getFrom(), the user in getTo()
     */
    static const int EVENT_REGISTRATION_CANCEL  = 22;

    /**
     *  A <presence> packet has been received.
     *  getFrom()     returns the full jabber id
     *  getPresence() returns the available/unavailable boolean
     *  getShow()     returns the jabber 'show' string: 'show', 'away', 'xa', etc
     *  getStatus()   returns a status message, sent from a client
     *  Note: if a presence packet is determined to be MUC, it is
     *    rather sent as an EVENT_MUC_JOIN, LEAVE, or PRESENCE
     */
    static const int EVENT_PRESENCE             = 30;

    /**
     *  Client has just received a complete roster.  The collected information
     *  can be found at client.getRoster(), and is a std::vector of XmppUser
     *  records.
     */
    static const int EVENT_ROSTER               = 31;

    /**
     *  Client has just received a message packet.
     *  getFrom() returns the full jabber id of the sender
     *  getData() returns the text of the message
     *  getDom()  returns the DOM treelet for this stanza.  This is provided
     *                to make message extension easier.
     *  Note: if a message packet is determined to be MUC, it is
     *    rather sent as an EVENT_MUC_MESSAGE
     */
    static const int EVENT_MESSAGE              = 32;

    /**
     *  THIS user has just joined a multi-user chat group.
     *  getGroup()    returns the group name
     *  getFrom()     returns the nick of the user in the group
     *  getPresence() returns the available/unavailable boolean
     *  getShow()     returns the jabber 'show' string: 'show', 'away', 'xa', etc
     *  getStatus()   returns a status message, sent from a client
     */
    static const int EVENT_MUC_JOIN             = 40;

    /**
     *  THIS user has just left a multi-user chat group.
     *  getGroup()    returns the group name
     *  getFrom()     returns the nick of the user in the group
     *  getPresence() returns the available/unavailable boolean
     *  getShow()     returns the jabber 'show' string: 'show', 'away', 'xa', etc
     *  getStatus()   returns a status message, sent from a client
     */
    static const int EVENT_MUC_LEAVE            = 41;

    /**
     *  Presence for another user in a multi-user chat room.
     *  getGroup()    returns the group name
     *  getFrom()     returns the nick of the user in the group
     *  getPresence() returns the available/unavailable boolean
     *  getShow()     returns the jabber 'show' string: 'show', 'away', 'xa', etc
     *  getStatus()   returns a status message, sent from a client
     */
    static const int EVENT_MUC_PRESENCE         = 42;

    /**
     *  Client has just received a message packet from a multi-user chat room
     *  getGroup() returns the group name
     *  getFrom()  returns the full jabber id of the sender
     *  getData()  returns the text of the message
     *  getDom()   returns the DOM treelet for this stanza.  This is provided
     *             to make message extension easier.
     */
    static const int EVENT_MUC_MESSAGE          = 43;

    /**
     *  Client has begun receiving a stream
     */
    static const int EVENT_STREAM_RECEIVE_INIT  = 50;

    /**
     *  Client receives another stream packet.
     */
    static const int EVENT_STREAM_RECEIVE       = 51;

    /**
     * Client has received the end of a stream
     */
    static const int EVENT_STREAM_RECEIVE_CLOSE = 52;

    /**
     * Other client has accepted a file.
     */
    static const int EVENT_FILE_ACCEPTED        = 60;

    /**
     * This client has just received a file.
     */
    static const int EVENT_FILE_RECEIVE         = 61;

    /**
     *  Constructs an event with one of the types above.
     */
    XmppEvent(int type);

    /**
     *  Copy constructor
     */
    XmppEvent(const XmppEvent &other);

    /**
     *  Assignment
     */
    virtual XmppEvent &operator=(const XmppEvent &other);

    /**
     *  Destructor
     */
    virtual ~XmppEvent();

    /**
     *  Assignment
     */
    virtual void assign(const XmppEvent &other);

    /**
     *  Return the event type.
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

/**
 * Class that receives and processes an XmppEvent.  Users should inherit
 * from this class, and overload processXmppEvent() to perform their event
 * handling
 */
class XmppEventListener
{
public:

    /**
     *  Constructor  
     */
    XmppEventListener()
        {}

    /**
     *  Assignment
     */
    XmppEventListener(const XmppEventListener &/*other*/)
        {}


    /**
     * Destructor
     */
    virtual ~XmppEventListener()
        {}

    /**
     *  Overload this method to provide your application-specific
     *  event handling.  Use event.getType() to decide what to do
     *  with the event.
     */
    virtual void processXmppEvent(const XmppEvent &/*event*/)
        {}

};



//########################################################################
//# X M P P    E V E N T    T A R G E T
//########################################################################

/**
 * A base class for classes that emit XmppEvents.
 *
 * Note: terminology: 'target' is the common term for this, although it
 * seems odd that a 'target' is the source of the events.  It is clearer
 * if you consider that the greater system targets this class with events,
 * and this class delegates the handling to its listeners.
 */
class XmppEventTarget
{
public:

    /**
     * Constructor
     */
    XmppEventTarget();

    /**
     *  Copy constructor
     */
    XmppEventTarget(const XmppEventTarget &other);

    /**
     * Destructor
     */
    virtual ~XmppEventTarget();


    //###########################
    //# M E S S A G E S
    //###########################


    /**
     * Send an error message to all subscribers
     */
    void error(const char *fmt, ...) G_GNUC_PRINTF(2,3);


    /**
     * Send a status message to all subscribers
     */
    void status(const char *fmt, ...) G_GNUC_PRINTF(2,3);

    //###########################
    //# LISTENERS
    //###########################

    /**
     *  Subscribe a subclass of XmppEventListener to this target's events.
     */
    virtual void addXmppEventListener(const XmppEventListener &listener);

    /**
     *  Unsubscribe a subclass of XmppEventListener from this target's events.
     */
    virtual void removeXmppEventListener(const XmppEventListener &listener);

    /**
     *  Remove all event subscribers
     */
    virtual void clearXmppEventListeners();

    /**
     *  This sends an event to all registered listeners.
     */
    virtual void dispatchXmppEvent(const XmppEvent &event);

    /**
     *  By enabling this, you provide an alternate way to get XmppEvents.
     *  Any event sent to dispatchXmppEvent() is also sent to this queue,
     *  so that it can be later be picked up by eventQueuePop();
     *  This can sometimes be good for GUI's which can't always respond
     *  repidly or asynchronously.
     */
    void eventQueueEnable(bool val);

    /**
     *  Return true if there is one or more XmppEvents waiting in the event
     *  queue.  This is used to avoid calling eventQueuePop() when there is
     *  nothing in the queue.
     */
    int eventQueueAvailable();

    /**
     *  Return the next XmppEvent in the queue.  Users should check that
     *  eventQueueAvailable() is greater than 0 before calling this.  If
     *  people forget to do this, an event of type XmppEvent::EVENT_NONE
     *  is generated and returned.
     */
    XmppEvent eventQueuePop();


private:

    std::vector<XmppEventListener *> listeners;

    std::vector<XmppEvent> eventQueue;
    bool eventQueueEnabled;
};





//########################################################################
//# X M P P    C L I E N T
//########################################################################

//forward declarations
class TcpSocket;
class XmppChat;
class XmppGroupChat;
class XmppStream;


/**
 *  This is the actual XMPP (Jabber) client.
 */
class XmppClient : public XmppEventTarget
{

public:

    //###########################
    //# CONSTRUCTORS
    //###########################

    /**
     * Constructor
     */
    XmppClient();

    /**
     *  Copy constructor
     */
    XmppClient(const XmppClient &other);

    /**
     *  Assignment
     */
    void assign(const XmppClient &other);

    /**
     * Destructor
     */
    virtual ~XmppClient();


    //###########################
    //# UTILITY
    //###########################

    /**
     *  Pause execution of the app for a given number of
     *  milliseconds.  Use this rarely, only when really needed.
     */
    virtual bool pause(unsigned long millis);

    /**
     *  Process a string so that it can safely be
     *  placed in XML as PCDATA
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
    virtual bool write(const char *fmt, ...) G_GNUC_PRINTF(2,3);

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
    virtual void setJid(const DOMString &val)
        { jid = val; }

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
    //# REGISTRATION
    //#######################

    /**
     *  Set whether the client should to in-band registration
     *  before authentication.  Causes inBandRegistrationNew() to be called
     *  synchronously, before async is started.
     */
    virtual void setDoRegister(bool val)
        { doRegister = val; }

    /**
     * Change the password of an existing account with a server
     */
    bool inBandRegistrationChangePassword(const DOMString &newPassword);

    /**
     * Cancel an existing account with a server
     */
    bool inBandRegistrationCancel();


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
             groupChatGetUserList(const DOMString &groupJid);

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
    virtual bool outputStreamOpen(const DOMString &jid,
                                 const DOMString &streamId);

    /**
     *
     */
    virtual bool outputStreamWrite(const DOMString &streamId,
               const std::vector<unsigned char> &buf);

    /**
     *
     */
    virtual bool outputStreamClose(const DOMString &streamId);

    /**
     *
     */
    virtual bool inputStreamOpen(const DOMString &jid,
                                const DOMString &streamId,
                                const DOMString &iqId);

    /**
     *
     */
    virtual bool inputStreamClose(const DOMString &streamId);


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

    bool saslAuthenticate(const DOMString &streamId);

    bool iqAuthenticate(const DOMString &streamId);

    /**
     * Register a new account with a server.  Not done by user
     */
    bool inBandRegistrationNew();

    bool keepGoing;

    bool doRegister;

    std::vector<XmppGroupChat *>groupChats;

    //#### Roster
    std::vector<XmppUser>roster;


    //#### Streams
    
    bool processInBandByteStreamMessage(Element *root);
    
    DOMString streamPacket;

    std::map<DOMString, XmppStream *> outputStreams;

    std::map<DOMString, XmppStream *> inputStreams;


    //#### File send
    
    bool processFileMessage(Element *root);

    std::map<DOMString, XmppStream *> fileSends;

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

