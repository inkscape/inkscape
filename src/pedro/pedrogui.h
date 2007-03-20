#ifndef __PEDROGUI_H__
#define __PEDROGUI_H__
/*
 * Simple demo GUI for the Pedro mini-XMPP client.
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



#include <gtkmm.h>

#include "pedroxmpp.h"
#include "pedroconfig.h"


namespace Pedro
{


class PedroGui;
class GroupChatWindow;

//#########################################################################
//# R O S T E R
//#########################################################################
class Roster : public Gtk::ScrolledWindow
{
public:

    Roster()
        { doSetup(); }

    virtual ~Roster()
        {}

    /**
     * Clear all roster items from the list
     */
    virtual void clear();

    /**
     * Regenerate the roster
     */
    virtual void refresh();


    void setParent(PedroGui *val)
        { parent = val; }

private:

    class CustomTreeView : public Gtk::TreeView
    {
    public:
        CustomTreeView()
           { parent = NULL; }
        virtual ~CustomTreeView()
           {}

        bool on_button_press_event(GdkEventButton* event)
            {
            Gtk::TreeView::on_button_press_event(event);
            if (parent)
                parent->buttonPressCallback(event);
            return true;
            }
        void setParent(Roster *val)
            { parent = val; }

    private:
        Roster *parent;
    };

    void doubleClickCallback(const Gtk::TreeModel::Path &path,
                   Gtk::TreeViewColumn *col);

    void sendFileCallback();
    void chatCallback();
    bool buttonPressCallback(GdkEventButton* event);

    bool doSetup();

    Glib::RefPtr<Gdk::Pixbuf> pixbuf_available;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf_away;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf_chat;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf_dnd;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf_error;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf_offline;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf_xa;

    class RosterColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        RosterColumns()
           {
           add(groupColumn);
           add(statusColumn); add(userColumn);
           add(nameColumn);   add(subColumn);
           }

        Gtk::TreeModelColumn<Glib::ustring> groupColumn;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > statusColumn;
        Gtk::TreeModelColumn<Glib::ustring> userColumn;
        Gtk::TreeModelColumn<Glib::ustring> nameColumn;
        Gtk::TreeModelColumn<Glib::ustring> subColumn;
    };

    RosterColumns rosterColumns;

    Glib::RefPtr<Gtk::UIManager> uiManager;

    Glib::RefPtr<Gtk::TreeStore> treeStore;
    CustomTreeView rosterView;

    PedroGui *parent;
};

//#########################################################################
//# M E S S A G E    L I S T
//#########################################################################
class MessageList : public Gtk::ScrolledWindow
{
public:

    MessageList()
        { doSetup(); }

    virtual ~MessageList()
        {}

    /**
     * Clear all messages from the list
     */
    virtual void clear();

    /**
     * Post a message to the list
     */
    virtual void postMessage(const DOMString &from, const DOMString &msg);

private:

    bool doSetup();

    Gtk::TextView messageList;
    Glib::RefPtr<Gtk::TextBuffer> messageListBuffer;

};

//#########################################################################
//# U S E R    L I S T
//#########################################################################
class UserList : public Gtk::ScrolledWindow
{
public:

    UserList()
        { doSetup(); }

    virtual ~UserList()
        {}

    /**
     * Clear all messages from the list
     */
    virtual void clear();

    /**
     * Post a message to the list
     */
    virtual void addUser(const DOMString &user, const DOMString &show);


    void setParent(GroupChatWindow *val)
        { parent = val; }

private:

    class CustomTreeView : public Gtk::TreeView
    {
    public:
        CustomTreeView()
           { parent = NULL; }
        virtual ~CustomTreeView()
           {}

        bool on_button_press_event(GdkEventButton* event)
            {
            Gtk::TreeView::on_button_press_event(event);
            if (parent)
                parent->buttonPressCallback(event);
            return true;
            }
        void setParent(UserList *val)
            { parent = val; }

    private:
        UserList *parent;
    };

    void doubleClickCallback(const Gtk::TreeModel::Path &path,
                   Gtk::TreeViewColumn *col);

    void sendFileCallback();
    void chatCallback();
    bool buttonPressCallback(GdkEventButton* event);

    bool doSetup();

    Glib::RefPtr<Gdk::Pixbuf> pixbuf_available;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf_away;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf_chat;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf_dnd;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf_error;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf_offline;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf_xa;

    class UserListColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        UserListColumns()
           { add(statusColumn); add(userColumn);  }

        Gtk::TreeModelColumn<Glib::ustring> userColumn;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > statusColumn;
    };

    UserListColumns userListColumns;

    Glib::RefPtr<Gtk::UIManager> uiManager;

    Glib::RefPtr<Gtk::ListStore> userListStore;
    CustomTreeView userList;

    GroupChatWindow *parent;
};


//#########################################################################
//# C H A T    W I N D O W
//#########################################################################
class ChatWindow : public Gtk::Window
{
public:

    ChatWindow(PedroGui &par, const DOMString jid);

    virtual ~ChatWindow();

    virtual DOMString getJid()
        { return jid; }

    virtual void setJid(const DOMString &val)
        { jid = val; }

    virtual bool postMessage(const DOMString &data);

private:

    DOMString jid;

    void leaveCallback();
    void hideCallback();
    void textEnterCallback();

    bool doSetup();

    Gtk::VBox   vbox;
    Gtk::VPaned vPaned;

    MessageList messageList;

    Gtk::Entry inputTxt;

    PedroGui &parent;
};


//#########################################################################
//# G R O U P    C H A T    W I N D O W
//#########################################################################
class GroupChatWindow : public Gtk::Window
{
public:

    GroupChatWindow(PedroGui &par, const DOMString &groupJid,
                                   const DOMString &nick);

    virtual ~GroupChatWindow();


    virtual DOMString getGroupJid()
        { return groupJid; }

    virtual void setGroupJid(const DOMString &val)
        { groupJid = val; }

    virtual DOMString getNick()
        { return nick; }

    virtual void setNick(const DOMString &val)
        { nick = val; }

    virtual bool receiveMessage(const DOMString &from,
                                const DOMString &data);

    virtual bool receivePresence(const DOMString &nick,
                                 bool presence,
                                 const DOMString &show,
                                 const DOMString &status);

    virtual void doSendFile(const DOMString &nick);

    virtual void doChat(const DOMString &nick);


private:

    void textEnterCallback();
    void leaveCallback();
    void hideCallback();

    bool doSetup();

    Gtk::VBox   vbox;
    Gtk::VPaned vPaned;
    Gtk::HPaned hPaned;

    MessageList messageList;

    UserList userList;

    Gtk::Entry inputTxt;

    DOMString groupJid;
    DOMString nick;

    PedroGui &parent;
 };



//#########################################################################
//# C O N F I G    D I A L O G
//#########################################################################

class ConfigDialog : public Gtk::Dialog
{
public:

    ConfigDialog (PedroGui &par) : parent(par)
        { doSetup(); }

    virtual ~ConfigDialog ()
        {}

   DOMString getPass()
       { return passField.get_text(); }
   DOMString getNewPass()
       { return newField.get_text(); }
   DOMString getConfirm()
       { return confField.get_text(); }

protected:

    //Overloaded from Gtk::Dialog
    virtual void on_response(int response_id);

private:

    void okCallback();
    void cancelCallback();

    bool doSetup();

    Gtk::Table       table;

    Gtk::Label       passLabel;
    Gtk::Entry       passField;
    Gtk::Label       newLabel;
    Gtk::Entry       newField;
    Gtk::Label       confLabel;
    Gtk::Entry       confField;

    PedroGui &parent;
};


//#########################################################################
//# P A S S W O R D    D I A L O G
//#########################################################################
class PasswordDialog : public Gtk::Dialog
{
public:

    PasswordDialog (PedroGui &par) : parent(par)
        { doSetup(); }

    virtual ~PasswordDialog ()
        {}

   DOMString getPass()
       { return passField.get_text(); }
   DOMString getNewPass()
       { return newField.get_text(); }
   DOMString getConfirm()
       { return confField.get_text(); }

protected:

    //Overloaded from Gtk::Dialog
    virtual void on_response(int response_id);

private:

    void okCallback();
    void cancelCallback();

    bool doSetup();

    Gtk::Table       table;

    Gtk::Label       passLabel;
    Gtk::Entry       passField;
    Gtk::Label       newLabel;
    Gtk::Entry       newField;
    Gtk::Label       confLabel;
    Gtk::Entry       confField;

    PedroGui &parent;
};



//#########################################################################
//# C H A T   D I A L O G
//#########################################################################
class ChatDialog : public Gtk::Dialog
{
public:

    ChatDialog(PedroGui &par) : parent(par)
        { doSetup(); }

    virtual ~ChatDialog()
        {}

   DOMString getUser()
       { return userField.get_text(); }

   DOMString getText()
       { return textField.get_text(); }

private:

    void okCallback();
    void cancelCallback();

    bool doSetup();

    Gtk::Table table;

    Gtk::Label userLabel;
    Gtk::Entry userField;
    Gtk::Entry textField;

    PedroGui &parent;
};



//#########################################################################
//# G R O U P    C H A T   D I A L O G
//#########################################################################

class GroupChatDialog : public Gtk::Dialog
{
public:

    GroupChatDialog(PedroGui &par) : parent(par)
        { doSetup(); }

    virtual ~GroupChatDialog()
        {}

   DOMString getGroup()
       { return groupField.get_text(); }
   DOMString getHost()
       { return hostField.get_text(); }
   DOMString getPass()
       { return passField.get_text(); }
   DOMString getNick()
       { return nickField.get_text(); }

private:

    void okCallback();
    void cancelCallback();

    bool doSetup();

    Gtk::Table table;

    Gtk::Label groupLabel;
    Gtk::Entry groupField;
    Gtk::Label hostLabel;
    Gtk::Entry hostField;
    Gtk::Label passLabel;
    Gtk::Entry passField;
    Gtk::Label nickLabel;
    Gtk::Entry nickField;

    PedroGui &parent;
};


//#########################################################################
//#  C O N N E C T    D I A L O G
//#########################################################################
class ConnectDialog : public Gtk::Dialog
{
public:

    ConnectDialog (PedroGui &par) : parent(par)
        { doSetup(); }

    virtual ~ConnectDialog ()
        {}

   DOMString getHost()
       { return hostField.get_text(); }
   void setHost(const DOMString &val)
       { hostField.set_text(val); }
   int getPort()
       { return (int)portSpinner.get_value(); }
   void setPort(int val)
       { portSpinner.set_value(val); }
   DOMString getUser()
       { return userField.get_text(); }
   void setUser(const DOMString &val)
       { userField.set_text(val); }
   DOMString getPass()
       { return passField.get_text(); }
   void setPass(const DOMString &val)
       { passField.set_text(val); }
   DOMString getResource()
       { return resourceField.get_text(); }
   void setResource(const DOMString &val)
       { resourceField.set_text(val); }
   bool getRegister()
       { return registerButton.get_active(); }

    /**
     * Regenerate the account list
     */
    virtual void refresh();

private:

    void okCallback();
    void saveCallback();
    void cancelCallback();
    void doubleClickCallback(
                   const Gtk::TreeModel::Path &path,
                   Gtk::TreeViewColumn *col);
    void selectedCallback();

    bool doSetup();

    Gtk::Table       table;

    Gtk::Label       hostLabel;
    Gtk::Entry       hostField;
    Gtk::Label       portLabel;
    Gtk::SpinButton  portSpinner;
    Gtk::Label       userLabel;
    Gtk::Entry       userField;
    Gtk::Label       passLabel;
    Gtk::Entry       passField;
    Gtk::Label       resourceLabel;
    Gtk::Entry       resourceField;
    Gtk::Label       registerLabel;
    Gtk::CheckButton registerButton;

    Glib::RefPtr<Gtk::UIManager> uiManager;


    //##  Account list

    void buttonPressCallback(GdkEventButton* event);

    Gtk::ScrolledWindow accountScroll;

    void connectCallback();

    void modifyCallback();

    void deleteCallback();


    class AccountColumns : public Gtk::TreeModel::ColumnRecord
        {
        public:
            AccountColumns()
                {
                add(nameColumn);
                add(hostColumn);
                }

            Gtk::TreeModelColumn<Glib::ustring> nameColumn;
            Gtk::TreeModelColumn<Glib::ustring> hostColumn;
        };

    AccountColumns accountColumns;

    Glib::RefPtr<Gtk::UIManager> accountUiManager;

    Glib::RefPtr<Gtk::ListStore> accountListStore;
    Gtk::TreeView accountView;


    PedroGui &parent;
};




//#########################################################################
//# F I L E    S E N D    D I A L O G
//#########################################################################

class FileSendDialog : public Gtk::Dialog
{
public:

    FileSendDialog(PedroGui &par) : parent(par)
        { doSetup(); }

    virtual ~FileSendDialog()
        {}

   DOMString getFileName()
       { return fileName; }
   DOMString getJid()
       { return jidField.get_text(); }
  void setJid(const DOMString &val)
       { return jidField.set_text(val); }

private:

    void okCallback();
    void cancelCallback();
    void buttonCallback();

    bool doSetup();

    Gtk::Table table;

    Gtk::Label jidLabel;
    Gtk::Entry jidField;

    DOMString fileName;
    Gtk::Entry fileNameField;

    Gtk::Button selectFileButton;

    PedroGui &parent;
};

//#########################################################################
//# F I L E    R E C E I V E    D I A L O G
//#########################################################################

class FileReceiveDialog : public Gtk::Dialog
{
public:

    FileReceiveDialog(PedroGui &par,
                      const DOMString &jidArg,
                      const DOMString &iqIdArg,
                      const DOMString &streamIdArg,
                      const DOMString &offeredNameArg,
                      const DOMString &descArg,
                      long  sizeArg,
                      const DOMString &hashArg
                      ) : parent(par)
        {
        jid         = jidArg;
        iqId        = iqIdArg;
        streamId    = streamIdArg;
        offeredName = offeredNameArg;
        desc        = descArg;
        fileSize    = sizeArg;
        hash        = hashArg;
        doSetup();
        }

    virtual ~FileReceiveDialog()
        {}

   DOMString getJid()
       { return jid; }
   DOMString getIq()
       { return iqId; }
   DOMString getStreamId()
       { return streamId; }
   DOMString getOfferedName()
       { return offeredName; }
   DOMString getFileName()
       { return fileName; }
   DOMString getDescription()
       { return desc; }
   long getSize()
       { return fileSize; }
   DOMString getHash()
       { return hash; }

private:

    void okCallback();
    void cancelCallback();
    void buttonCallback();

    bool doSetup();

    Gtk::Table table;

    Gtk::Label jidLabel;
    Gtk::Entry jidField;
    Gtk::Label offeredLabel;
    Gtk::Entry offeredField;
    Gtk::Label descLabel;
    Gtk::Entry descField;
    Gtk::Label sizeLabel;
    Gtk::Entry sizeField;
    Gtk::Label hashLabel;
    Gtk::Entry hashField;

    Gtk::Entry fileNameField;

    Gtk::Button selectFileButton;

    DOMString jid;
    DOMString iqId;
    DOMString streamId;
    DOMString offeredName;
    DOMString desc;
    long      fileSize;
    DOMString hash;

    DOMString fileName;

    PedroGui &parent;
};



//#########################################################################
//# M A I N    W I N D O W
//#########################################################################

class PedroGui : public Gtk::Window
{
public:

    PedroGui();

    virtual ~PedroGui();

    //Let everyone share these
    XmppClient client;
    XmppConfig config;


    virtual void error(const char *fmt, ...) G_GNUC_PRINTF(2,3);

    virtual void status(const char *fmt, ...) G_GNUC_PRINTF(2,3);



    void handleConnectEvent();

    void handleDisconnectEvent();

    /**
     *
     */
    virtual void doEvent(const XmppEvent &event);

    /**
     *
     */
    bool checkEventQueue();


    bool chatCreate(const DOMString &userJid);
    bool chatDelete(const DOMString &userJid);
    bool chatDeleteAll();
    bool chatMessage(const DOMString &jid, const DOMString &data);

    bool groupChatCreate(const DOMString &groupJid,
                         const DOMString &nick);
    bool groupChatDelete(const DOMString &groupJid,
                         const DOMString &nick);
    bool groupChatDeleteAll();
    bool groupChatMessage(const DOMString &groupJid,
              const DOMString &from, const DOMString &data);
    bool groupChatPresence(const DOMString &groupJid,
                           const DOMString &nick,
                           bool presence,
                           const DOMString &show,
                           const DOMString &status);

    void doChat(const DOMString &jid);
    void doSendFile(const DOMString &jid);
    void doReceiveFile(const DOMString &jid,
                       const DOMString &iqId,
                       const DOMString &streamId,
                       const DOMString &offeredName,
                       const DOMString &desc,
                       long  size,
                       const DOMString &hash);


    //# File menu
    void connectCallback();
    void chatCallback();
    void groupChatCallback();
    void disconnectCallback();
    void quitCallback();

    //# Edit menu
    void fontCallback();
    void colorCallback();

    //# Transfer menu
    void sendFileCallback();

    //# Registration menu
    void regPassCallback();
    void regCancelCallback();

    //# Help menu
    void aboutCallback();

    //# Configuration file
    bool configLoad();
    bool configSave();


private:

    bool doSetup();

    Gtk::VBox mainBox;

    Gtk::HBox menuBarBox;

    Gtk::Image padlockIcon;
    void padlockEnable();
    void padlockDisable();


    Pango::FontDescription fontDesc;
    Gdk::Color foregroundColor;
    Gdk::Color backgroundColor;

    Gtk::VPaned vPaned;
    MessageList messageList;
    Roster      roster;

    Glib::RefPtr<Gtk::UIManager> uiManager;
    void actionEnable(const DOMString &name, bool val);

    std::vector<ChatWindow *>chats;

    std::vector<GroupChatWindow *>groupChats;
};


} //namespace Pedro

#endif /* __PEDROGUI_H__ */
//#########################################################################
//# E N D    O F    F I L E
//#########################################################################


