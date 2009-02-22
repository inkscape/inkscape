

#include <stdio.h>

#include "pedroxmpp.h"

//########################################################################
//# T E S T
//########################################################################


class TestListener : public Pedro::XmppEventListener
{
public:
    TestListener()
        {
        incoming = false;
        }

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
            case Pedro::XmppEvent::EVENT_FILE_RECEIVE:
                {
                printf("FILE RECEIVE\n");
                from     = evt.getFrom();
                streamId = evt.getStreamId();
                iqId     = evt.getIqId();
                fileName = evt.getFileName();
                fileHash = evt.getFileHash();
                fileSize = evt.getFileSize();
                incoming = true;
                break;
                }

            }
        }
        
    Pedro::DOMString from;
    Pedro::DOMString streamId;
    Pedro::DOMString iqId;
    Pedro::DOMString fileName;
    Pedro::DOMString fileHash;
    long      fileSize;
    bool incoming;
};


bool doTest()
{
    printf("############ RECEIVING FILE\n");

    Pedro::XmppClient client;
    TestListener listener;
    client.addXmppEventListener(listener);

    //Host, port, user, pass, resource
    if (!client.connect("jabber.org.uk", 443, "ishmal", "PASSWORD", "filerec"))
       {
       printf("Connect failed\n");
       return false;
       }

    while (true)
        {
        printf("####Waiting for file\n");
        if (listener.incoming)
            break;
        client.pause(2000);
        }

    printf("#####GOT A FILE\n");
/*
TODO: Just Commented out to compile
    if (!client.fileReceive(listener.from, 
                            listener.iqId,
                            listener.streamId, 
                            listener.fileName,
                            "text.sav",
                            listener.fileHash))
        {
        return false;
        }   
*/
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

