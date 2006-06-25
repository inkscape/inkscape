

#include <stdio.h>
#include <string.h>

#include "pedroxmpp.h"

//########################################################################
//# T E S T
//########################################################################

using namespace Pedro;


class Listener : public Pedro::XmppEventListener
{
public:
    Listener(){}

    virtual ~Listener(){}

    virtual void processXmppEvent(const Pedro::XmppEvent &evt)
        {
        int typ = evt.getType();
        switch (typ)
            {
            case Pedro::XmppEvent::EVENT_STATUS:
                {
                printf("STATUS: %s\n", evt.getData().c_str());
                break;
                }
            case Pedro::XmppEvent::EVENT_ERROR:
                {
                printf("ERROR: %s\n", evt.getData().c_str());
                break;
                }
            case Pedro::XmppEvent::EVENT_CONNECTED:
                {
                printf("CONNECTED\n");
                break;
                }
            case Pedro::XmppEvent::EVENT_DISCONNECTED:
                {
                printf("DISCONNECTED\n");
                break;
                }
            case Pedro::XmppEvent::EVENT_MESSAGE:
                {
                printf("<%s> %s\n", evt.getFrom().c_str(), evt.getData().c_str());
                break;
                }
            case Pedro::XmppEvent::EVENT_PRESENCE:
                {
                printf("PRESENCE\n");
                printf("from     : %s\n", evt.getFrom().c_str());
                //printf("presence : %s\n", evt.getPresence().c_str());
                // TODO: Just Commented out to compile
                break;
                }
            case Pedro::XmppEvent::EVENT_MUC_MESSAGE:
                {
                printf("<%s> %s\n", evt.getFrom().c_str(), evt.getData().c_str());
                break;
                }
            case Pedro::XmppEvent::EVENT_MUC_JOIN:
                {
                printf("MUC JOIN\n");
                printf("group: %s\n", evt.getGroup().c_str());
                printf("from    : %s\n", evt.getFrom().c_str());
                //printf("presence: %s\n", evt.getPresence().c_str());
                // TODO: Just Commented out to compile
                break;
                }
            case Pedro::XmppEvent::EVENT_MUC_LEAVE:
                {
                printf("MUC LEAVE\n");
                printf("group: %s\n", evt.getGroup().c_str());
                printf("from    : %s\n", evt.getFrom().c_str());
                //printf("presence: %s\n", evt.getPresence().c_str());
                // TODO: Just Commented out to compile
                break;
                }
            case Pedro::XmppEvent::EVENT_MUC_PRESENCE:
                {
                printf("MUC PRESENCE\n");
                printf("group   : %s\n", evt.getGroup().c_str());
                printf("from    : %s\n", evt.getFrom().c_str());
                //printf("presence: %s\n", evt.getPresence().c_str());
                // TODO: Just Commented out to compile
                break;
                }

            }
        }
};


class CommandLineGroupChat
{
public:
    CommandLineGroupChat(const DOMString &hostArg,
                         int portArg,
                         const DOMString &userArg,
                         const DOMString &passArg,
                         const DOMString &resourceArg,
                         const DOMString &groupJidArg,
                         const DOMString &nickArg)
        {
        host     = hostArg;
        port     = portArg;
        user     = userArg;
        pass     = passArg;
        resource = resourceArg;
        groupJid = groupJidArg;
        nick     = nickArg;
        }
    ~CommandLineGroupChat()
        {
        client.disconnect();
        }

    virtual bool run();
    virtual bool processCommandLine();

private:

    DOMString host;
    int       port;
    DOMString user;
    DOMString pass;
    DOMString resource;
    DOMString groupJid;
    DOMString nick;

    XmppClient client;

};


bool CommandLineGroupChat::run()
{
    Listener listener;
    client.addXmppEventListener(listener);

    //Host, port, user, pass, resource
    if (!client.connect(host, port, user, pass, resource))
        {
        return false;
        }

    //Group jabber id,  nick,  pass
    if (!client.groupChatJoin(groupJid, nick, ""))
        {
        printf("failed join\n");
        return false;
        }

    //Allow receive buffer to clear out
    client.pause(10000);

    while (true)
        {
        if (!processCommandLine())
            break;
        }

    //Group jabber id,  nick
    client.groupChatLeave(groupJid, nick);


    client.disconnect();

    return true;
}


bool CommandLineGroupChat::processCommandLine()
{
    char buf[512];
    printf("send>:");
    fgets(buf, 511, stdin);

    if (buf[0]=='/')
        {
        if (strncmp(buf, "/q", 2)==0)
            return false;
        else
           {
           printf("Unknown command\n");
           return true;
           }
        }

    else
        {
        DOMString msg = buf;
        if (msg.size() > 0 )
            {
            if (!client.groupChatMessage(groupJid, buf))
                {
                 printf("failed message send\n");
                return false;
               }
            }
        }

    return true;
}


int main(int argc, char **argv)
{
    if (argc!=8)
        {
        printf("usage: %s host port user pass resource groupid nick\n", argv[0]);
        return 1;
        }
    int port = atoi(argv[2]);
    CommandLineGroupChat groupChat(argv[1], port, argv[3], argv[4],
                                   argv[5], argv[6], argv[7]);
    if (!groupChat.run())
        return 1;
    return 0;
}

