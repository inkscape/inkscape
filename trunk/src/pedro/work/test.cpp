

#include <stdio.h>

#include "pedroxmpp.h"
#include "pedroconfig.h"

//########################################################################
//# T E S T
//########################################################################


class TestListener : public Pedro::XmppEventListener
{
public:
    TestListener(){}

    virtual ~TestListener(){}

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
                printf("MESSAGE\n");
                printf("from : %s\n", evt.getFrom().c_str());
                printf("msg  : %s\n", evt.getData().c_str());
                break;
                }
            case Pedro::XmppEvent::EVENT_PRESENCE:
                {
                printf("PRESENCE\n");
                printf("from     : %s\n", evt.getFrom().c_str());
                printf("presence : %d\n", evt.getPresence());
                break;
                }
            case Pedro::XmppEvent::EVENT_MUC_MESSAGE:
                {
                printf("MUC GROUP MESSAGE\n");
                printf("group: %s\n", evt.getGroup().c_str());
                printf("from : %s\n", evt.getFrom().c_str());
                printf("msg  : %s\n", evt.getData().c_str());
                break;
                }
            case Pedro::XmppEvent::EVENT_MUC_JOIN:
                {
                printf("MUC JOIN\n");
                printf("group: %s\n", evt.getGroup().c_str());
                printf("from    : %s\n", evt.getFrom().c_str());
                printf("presence: %d\n", evt.getPresence());
                break;
                }
            case Pedro::XmppEvent::EVENT_MUC_LEAVE:
                {
                printf("MUC LEAVE\n");
                printf("group: %s\n", evt.getGroup().c_str());
                printf("from    : %s\n", evt.getFrom().c_str());
                printf("presence: %d\n", evt.getPresence());
                break;
                }
            case Pedro::XmppEvent::EVENT_MUC_PRESENCE:
                {
                printf("MUC PRESENCE\n");
                printf("group   : %s\n", evt.getGroup().c_str());
                printf("from    : %s\n", evt.getFrom().c_str());
                printf("presence: %d\n", evt.getPresence());
                break;
                }

            }
        }
};


bool doTest()
{
    printf("############ TESTING\n");

    char *groupJid = "inkscape@conference.gristle.org";

    Pedro::XmppClient client;
    TestListener listener;
    client.addXmppEventListener(listener);

    //Host, port, user, pass, resource
    if (!client.connect("jabber.org.uk", 443, "ishmal", "PASSWORD", "myclient"))
       {
       printf("Connect failed\n");
       return false;
       }

    //Group jabber id,  nick,  pass
    client.groupChatJoin(groupJid, "mynick", "");

    client.pause(8000);

    //Group jabber id,  nick, msg
    //client.groupChatMessage(groupJid, "hello, world");

    client.pause(3000);

    //client.groupChatGetUserList(groupJid);

    client.pause(3000);

    //client.groupChatPrivateMessage("inkscape2@conference.gristle.org",
    //        "ishmal", "hello, world");
    client.message("ishmal@jabber.org.uk/https", "hey, bob");

    client.pause(60000);

    //Group jabber id,  nick
    client.groupChatLeave(groupJid, "mynick");

    client.pause(1000000);

    client.disconnect();

    return true;
}


bool configTest()
{
    printf("#################################\n");
    printf("## C o n f i g    t e s t\n");
    printf("#################################\n");

    Pedro::XmppConfig config;

    if (!config.readFile("pedro.ini"))
        {
        printf("could not read config file\n");
        return false;
        }

    Pedro::DOMString str = config.toXmlBuffer();

    printf("#################################\n");
    printf("%s\n", str.c_str());

    if (!config.writeFile("pedro2.ini"))
        {
        printf("could not write config file\n");
        return false;
        }


    return true;

};



int main(int argc, char **argv)
{
    if (!configTest())
        return 1;
    return 0;
}

