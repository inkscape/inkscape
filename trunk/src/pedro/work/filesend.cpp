

#include <stdio.h>

#include "pedroxmpp.h"

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
    printf("############ SENDING FILE\n");

    Pedro::XmppClient client;
    TestListener listener;
    client.addXmppEventListener(listener);

    //Host, port, user, pass, resource
    if (!client.connect("jabber.org.uk", 443, "ishmal", "PASSWORD", "filesend"))
       {
       printf("Connect failed\n");
       return false;
       }


    if (!client.fileSend("ishmal@jabber.org.uk/filerec", 
                         "server.pem" , "server.pem",
                         "a short story by edgar allen poe"))
        {
        return false;
        }   

    printf("OK\n");
    client.pause(1000000);

    client.disconnect();

    return true;
}

int main(int argc, char **argv)
{
    if (!doTest())
        return 1;
    return 0;
}

