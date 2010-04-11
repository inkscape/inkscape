/** @file
 * @brief Input devices dialog (new) - implementation
 */
/* Author:
 *   Jon A. Cruz
 *
 * Copyright (C) 2008 Author
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#include <map>
#include <set>
#include <glib/gprintf.h>
#include <glibmm/i18n.h>
#include <gtkmm/alignment.h>
#include <gtkmm/cellrenderercombo.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/enums.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/image.h>
#include <gtkmm/liststore.h>
#include <gtkmm/menubar.h>
#include <gtkmm/notebook.h>
#include <gtkmm/paned.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/table.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>

#include "device-manager.h"
#include "preferences.h"
#include "ui/widget/panel.h"

#include "input.h"

/* XPM */
static char const * core_xpm[] = {
"16 16 4 1",
" 	c None",
".	c #808080",
"+	c #000000",
"@	c #FFFFFF",
"                ",
"                ",
"                ",
"    .++++++.    ",
"    +@+@@+@+    ",
"    +@+@@+@+    ",
"    +.+..+.+    ",
"    +@@@@@@+    ",
"    +@@@@@@+    ",
"    +@@@@@@+    ",
"    +@@@@@@+    ",
"    +@@@@@@+    ",
"    .++++++.    ",
"                ",
"                ",
"                "};

/* XPM */
static char const *eraser[] = {
/* columns rows colors chars-per-pixel */
"16 16 5 1",
"  c black",
". c green",
"X c #808080",
"o c gray100",
"O c None",
/* pixels */
"OOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOO OO",
"OOOOOOOOOOOO . O",
"OOOOOOOOOOO . OO",
"OOOOOOOOOO . OOO",
"OOOOOOOOO . OOOO",
"OOOOOOOO . OOOOO",
"OOOOOOOXo OOOOOO",
"OOOOOOXoXOOOOOOO",
"OOOOOXoXOOOOOOOO",
"OOOOXoXOOOOOOOOO",
"OOOXoXOOOOOOOOOO",
"OOXoXOOOOOOOOOOO",
"OOXXOOOOOOOOOOOO",
"OOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOOOOO"
};

/* XPM */
static char const *mouse[] = {
/* columns rows colors chars-per-pixel */
"16 16 3 1",
"  c black",
". c gray100",
"X c None",
/* pixels */
"XXXXXXXXXXXXXXXX",
"XXXXXXXXXXXXXXXX",
"XXXXXXXXXXXXXXXX",
"XXXXXXXXXXXXXXXX",
"XXXXXXX  XXXXXXX",
"XXXXX  . XXXXXXX",
"XXXX .... XXXXXX",
"XXXX .... XXXXXX",
"XXXXX .... XXXXX",
"XXXXX .... XXXXX",
"XXXXXX .... XXXX",
"XXXXXX .... XXXX",
"XXXXXXX .  XXXXX",
"XXXXXXX  XXXXXXX",
"XXXXXXXXXXXXXXXX",
"XXXXXXXXXXXXXXXX"
};

/* XPM */
static char const *pen[] = {
/* columns rows colors chars-per-pixel */
"16 16 3 1",
"  c black",
". c gray100",
"X c None",
/* pixels */
"XXXXXXXXXXXXXXXX",
"XXXXXXXXXXXXX XX",
"XXXXXXXXXXXX . X",
"XXXXXXXXXXX . XX",
"XXXXXXXXXX . XXX",
"XXXXXXXXX . XXXX",
"XXXXXXXX . XXXXX",
"XXXXXXX . XXXXXX",
"XXXXXX . XXXXXXX",
"XXXXX . XXXXXXXX",
"XXXX . XXXXXXXXX",
"XXX . XXXXXXXXXX",
"XX . XXXXXXXXXXX",
"XX  XXXXXXXXXXXX",
"XXXXXXXXXXXXXXXX",
"XXXXXXXXXXXXXXXX"
};

/* XPM */
static char const *sidebuttons[] = {
/* columns rows colors chars-per-pixel */
"16 16 4 1",
"  c black",
". c #808080",
"o c green",
"O c None",
/* pixels */
"OOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOOOOO",
"O..............O",
"O.OOOOOOOOOOOO.O",
"O   OOOOOOOO   O",
"O o OOOOOOOO o O",
"O o OOOOOOOO o O",
"O   OOOOOOOO   O",
"O.OOOOOOOOOOOO.O",
"O.OOOOOOOOOOOO.O",
"O.OOOOOOOOOOOO.O",
"O.OOOOOOOOOOOO.O",
"O.OOOOOOOOOOOO.O",
"O..............O",
"OOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOOOOO"
};

/* XPM */
static char const *tablet[] = {
/* columns rows colors chars-per-pixel */
"16 16 3 1",
"  c black",
". c gray100",
"X c None",
/* pixels */
"XXXXXXXXXXXXXXXX",
"XXXXXXXXXXXXXXXX",
"X              X",
"X ............ X",
"X ............ X",
"X ............ X",
"X ............ X",
"X ............ X",
"X ............ X",
"X ............ X",
"X ............ X",
"X ............ X",
"X ............ X",
"X              X",
"XXXXXXXXXXXXXXXX",
"XXXXXXXXXXXXXXXX"
};

/* XPM */
static char const *tip[] = {
/* columns rows colors chars-per-pixel */
"16 16 5 1",
"  c black",
". c green",
"X c #808080",
"o c gray100",
"O c None",
/* pixels */
"OOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOOXOO",
"OOOOOOOOOOOOXoXO",
"OOOOOOOOOOOXoXOO",
"OOOOOOOOOOXoXOOO",
"OOOOOOOOOXoXOOOO",
"OOOOOOOOXoXOOOOO",
"OOOOOOO oXOOOOOO",
"OOOOOO . OOOOOOO",
"OOOOO . OOOOOOOO",
"OOOO . OOOOOOOOO",
"OOO . OOOOOOOOOO",
"OO . OOOOOOOOOOO",
"OO  OOOOOOOOOOOO",
"OOOOXXXXXOOOOOOO",
"OOOOOOOOOXXXXXOO"
};

/* XPM */
static char const *button_none[] = {
/* columns rows colors chars-per-pixel */
"8 8 3 1",
"  c black",
". c #808080",
"X c None",
/* pixels */
"XXXXXXXX",
"XX .. XX",
"X .XX. X",
"X.XX X.X",
"X.X XX.X",
"X .XX. X",
"XX .. XX",
"XXXXXXXX"
};
/* XPM */
static char const *button_off[] = {
/* columns rows colors chars-per-pixel */
"8 8 4 1",
"  c black",
". c #808080",
"X c gray100",
"o c None",
/* pixels */
"oooooooo",
"oo.  .oo",
"o. XX .o",
"o XXXX o",
"o XXXX o",
"o. XX .o",
"oo.  .oo",
"oooooooo"
};
/* XPM */
static char const *button_on[] = {
/* columns rows colors chars-per-pixel */
"8 8 3 1",
"  c black",
". c green",
"X c None",
/* pixels */
"XXXXXXXX",
"XX    XX",
"X  ..  X",
"X .... X",
"X .... X",
"X  ..  X",
"XX    XX",
"XXXXXXXX"
};

/* XPM */
static char const * axis_none_xpm[] = {
"24 8 3 1",
" 	c None",
".	c #000000",
"+	c #808080",
"                        ",
"  .++++++++++++++++++.  ",
" .+               . .+. ",
" +          . . .     + ",
" +     . . .          + ",
" .+. .               +. ",
"  .++++++++++++++++++.  ",
"                        "};
/* XPM */
static char const * axis_off_xpm[] = {
"24 8 4 1",
" 	c None",
".	c #808080",
"+	c #000000",
"@	c #FFFFFF",
"                        ",
"  .++++++++++++++++++.  ",
" .+@@@@@@@@@@@@@@@@@@+. ",
" +@@@@@@@@@@@@@@@@@@@@+ ",
" +@@@@@@@@@@@@@@@@@@@@+ ",
" .+@@@@@@@@@@@@@@@@@@+. ",
"  .++++++++++++++++++.  ",
"                        "};
/* XPM */
static char const * axis_on_xpm[] = {
"24 8 3 1",
" 	c None",
".	c #000000",
"+	c #00FF00",
"                        ",
"  ....................  ",
" ..++++++++++++++++++.. ",
" .++++++++++++++++++++. ",
" .++++++++++++++++++++. ",
" ..++++++++++++++++++.. ",
"  ....................  ",
"                        "};

using Inkscape::InputDevice;

namespace Inkscape {
namespace UI {
namespace Dialog {



class DeviceModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:
    Gtk::TreeModelColumn<Glib::ustring>                description;
    Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> >  thumbnail;
    Gtk::TreeModelColumn<Glib::RefPtr<InputDevice const> > device;
    Gtk::TreeModelColumn<Gdk::InputMode>               mode;

    DeviceModelColumns() { add(description); add(thumbnail); add(device); add(mode); }
};

static std::map<Gdk::InputMode, Glib::ustring> &getModeToString()
{
    static std::map<Gdk::InputMode, Glib::ustring> mapping;
    if (mapping.empty()) {
        mapping[Gdk::MODE_DISABLED] = _("Disabled");
        mapping[Gdk::MODE_SCREEN]   = _("Screen");
        mapping[Gdk::MODE_WINDOW]   = _("Window");
    }

    return mapping;
}

static std::map<Glib::ustring, Gdk::InputMode> &getStringToMode()
{
    static std::map<Glib::ustring, Gdk::InputMode> mapping;
    if (mapping.empty()) {
        mapping[_("Disabled")] = Gdk::MODE_DISABLED;
        mapping[_("Screen")]   = Gdk::MODE_SCREEN;
        mapping[_("Window")]   = Gdk::MODE_WINDOW;
    }

    return mapping;
}



class InputDialogImpl : public InputDialog {
public:
    InputDialogImpl();
    virtual ~InputDialogImpl() {}

private:
    class ConfPanel : public Gtk::VBox
    {
    public:
        ConfPanel();
        ~ConfPanel();

        class Blink : public Preferences::Observer
        {
        public:
            Blink(ConfPanel &parent);
            virtual ~Blink();
            virtual void notify(Preferences::Entry const &new_val);

            ConfPanel &parent;
        };

        static void commitCellModeChange(Glib::ustring const &path, Glib::ustring const &newText, Glib::RefPtr<Gtk::TreeStore> store);
        static void setModeCellString(Gtk::CellRenderer *rndr, Gtk::TreeIter const &iter);

        void saveSettings();
        void useExtToggled();

        Glib::RefPtr<Gtk::TreeStore> store;
        Gtk::TreeIter tabletIter;
        Gtk::TreeView tree;
        Gtk::ScrolledWindow treeScroller;
        Blink watcher;
        Gtk::CheckButton useExt;
        Gtk::Button save;
    };

    static DeviceModelColumns &getCols();

    enum PixId {PIX_CORE, PIX_PEN, PIX_MOUSE, PIX_TIP, PIX_TABLET, PIX_ERASER, PIX_SIDEBUTTONS,
                PIX_BUTTONS_NONE, PIX_BUTTONS_ON, PIX_BUTTONS_OFF,
                PIX_AXIS_NONE, PIX_AXIS_ON, PIX_AXIS_OFF};

    static Glib::RefPtr<Gdk::Pixbuf> getPix(PixId id);

    std::map<Glib::ustring, std::set<guint> > buttonMap;
    std::map<Glib::ustring, std::map<guint, std::pair<guint, gdouble> > > axesMap;

    GdkInputSource lastSourceSeen;
    Glib::ustring lastDevnameSeen;

    Glib::RefPtr<Gtk::TreeStore> store;
    Gtk::TreeIter tabletIter;
    Gtk::TreeView tree;
    Gtk::Frame frame2;
    Gtk::Frame testFrame;
    Gtk::ScrolledWindow treeScroller;
    Gtk::ScrolledWindow detailScroller;
    Gtk::HPaned splitter;
    Gtk::VPaned split2;
    Gtk::Label devName;
    Gtk::Label devKeyCount;
    Gtk::Label devAxesCount;
    Gtk::ComboBoxText axesCombo;
    Gtk::ProgressBar axesValues[6];
    Gtk::ComboBoxText buttonCombo;
    Gtk::ComboBoxText linkCombo;
    sigc::connection linkConnection;
    Gtk::Label keyVal;
    Gtk::Entry keyEntry;
    Gtk::Table devDetails;
    Gtk::Notebook topHolder;
    Gtk::Image testThumb;
    Gtk::Image testButtons[24];
    Gtk::Image testAxes[8];
    Gtk::Table imageTable;
    Gtk::EventBox testDetector;

    ConfPanel cfgPanel;

    static void setupTree( Glib::RefPtr<Gtk::TreeStore> store, Gtk::TreeIter &tablet );
    void setupValueAndCombo( gint reported, gint actual, Gtk::Label& label, Gtk::ComboBoxText& combo );
    void updateTestButtons( Glib::ustring const& key, gint hotButton );
    void updateTestAxes( Glib::ustring const& key, GdkDevice* dev );
    void mapAxesValues( Glib::ustring const& key, guint numAxes, gdouble const * axes, GdkDevice* dev);
    Glib::ustring getKeyFor( GdkDevice* device );
    bool eventSnoop(GdkEvent* event);
    void linkComboChanged();
    void resyncToSelection();
    void handleDeviceChange(const Glib::RefPtr<InputDevice>& device);
    void updateDeviceAxes(const Glib::RefPtr<InputDevice>& device);
    void updateDeviceButtons(const Glib::RefPtr<InputDevice>& device);
    static void updateDeviceLinks(const Glib::RefPtr<InputDevice>& device, Gtk::TreeIter &tabletIter, Gtk::TreeView &tree);

    static bool findDevice(const Gtk::TreeModel::iterator& iter,
                           Glib::ustring id,
                           Gtk::TreeModel::iterator* result);
    static bool findDeviceByLink(const Gtk::TreeModel::iterator& iter,
                                 Glib::ustring link,
                                 Gtk::TreeModel::iterator* result);

}; // class InputDialogImpl


DeviceModelColumns &InputDialogImpl::getCols()
{
    static DeviceModelColumns cols;
    return cols;
}

Glib::RefPtr<Gdk::Pixbuf> InputDialogImpl::getPix(PixId id)
{
    static std::map<PixId, Glib::RefPtr<Gdk::Pixbuf> > mappings;

    mappings[PIX_CORE]          = Gdk::Pixbuf::create_from_xpm_data(core_xpm);
    mappings[PIX_PEN]           = Gdk::Pixbuf::create_from_xpm_data(pen);
    mappings[PIX_MOUSE]         = Gdk::Pixbuf::create_from_xpm_data(mouse);
    mappings[PIX_TIP]           = Gdk::Pixbuf::create_from_xpm_data(tip);
    mappings[PIX_TABLET]        = Gdk::Pixbuf::create_from_xpm_data(tablet);
    mappings[PIX_ERASER]        = Gdk::Pixbuf::create_from_xpm_data(eraser);
    mappings[PIX_SIDEBUTTONS]   = Gdk::Pixbuf::create_from_xpm_data(sidebuttons);

    mappings[PIX_BUTTONS_NONE]  = Gdk::Pixbuf::create_from_xpm_data(button_none);
    mappings[PIX_BUTTONS_ON]    = Gdk::Pixbuf::create_from_xpm_data(button_on);
    mappings[PIX_BUTTONS_OFF]   = Gdk::Pixbuf::create_from_xpm_data(button_off);

    mappings[PIX_AXIS_NONE]     = Gdk::Pixbuf::create_from_xpm_data(axis_none_xpm);
    mappings[PIX_AXIS_ON]       = Gdk::Pixbuf::create_from_xpm_data(axis_on_xpm);
    mappings[PIX_AXIS_OFF]      = Gdk::Pixbuf::create_from_xpm_data(axis_off_xpm);

    Glib::RefPtr<Gdk::Pixbuf> pix;
    if (mappings.find(id) != mappings.end()) {
        pix = mappings[id];
    }

    return pix;
}


// Now that we've defined the *Impl class, we can do the method to aquire one.
InputDialog &InputDialog::getInstance()
{
    InputDialog *dialog = new InputDialogImpl();
    return *dialog;
}


InputDialogImpl::InputDialogImpl() :
    InputDialog(),

    lastSourceSeen((GdkInputSource)-1),
    lastDevnameSeen(""),
    store(Gtk::TreeStore::create(getCols())),
    tabletIter(),
    tree(store),
    frame2(),
    testFrame(_("Test Area")),
    treeScroller(),
    detailScroller(),
    splitter(),
    split2(),
    linkCombo(),
    devDetails(12, 2),
    topHolder(),
    imageTable(8, 7),
    testDetector(),
    cfgPanel()
{
    Gtk::Box *contents = _getContents();


    treeScroller.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    treeScroller.add(tree);
    split2.pack1(testFrame);
    split2.pack2(frame2);
    splitter.pack1(treeScroller);
    splitter.pack2(split2);

    testDetector.add(imageTable);
    testFrame.add(testDetector);
    testThumb.set(getPix(PIX_TABLET));
    testThumb.set_padding(24, 24);
    imageTable.attach(testThumb, 0, 8, 0, 1, ::Gtk::EXPAND, ::Gtk::EXPAND);
    {
        guint col = 0;
        guint row = 1;
        for ( guint num = 0; num < G_N_ELEMENTS(testButtons); num++ ) {
            testButtons[num].set(getPix(PIX_BUTTONS_NONE));
            imageTable.attach(testButtons[num], col, col + 1, row, row + 1, ::Gtk::FILL, ::Gtk::FILL);
            col++;
            if (col > 7) {
                col = 0;
                row++;
            }
        }

        col = 0;
        for ( guint num = 0; num < G_N_ELEMENTS(testAxes); num++ ) {
            testAxes[num].set(getPix(PIX_AXIS_NONE));
            imageTable.attach(testAxes[num], col * 2, (col + 1) * 2, row, row + 1, ::Gtk::FILL, ::Gtk::FILL);
            col++;
            if (col > 3) {
                col = 0;
                row++;
            }
        }
    }


    topHolder.append_page(cfgPanel, _("Configuration"));
    topHolder.append_page(splitter, _("Hardware"));
    topHolder.show_all();
    topHolder.set_current_page(0);

    contents->pack_start(topHolder);

    int rowNum = 0;

    Gtk::Label* lbl = Gtk::manage(new Gtk::Label(_("Name:")));
    devDetails.attach(*lbl, 0, 1, rowNum, rowNum+ 1,
                      ::Gtk::FILL,
                      ::Gtk::SHRINK);
    devDetails.attach(devName, 1, 2, rowNum, rowNum + 1,
                      ::Gtk::SHRINK,
                      ::Gtk::SHRINK);

    rowNum++;

    lbl = Gtk::manage(new Gtk::Label(_("Link:")));
    devDetails.attach(*lbl, 0, 1, rowNum, rowNum+ 1,
                      ::Gtk::FILL,
                      ::Gtk::SHRINK);

    linkCombo.append_text(_("None"));
    linkCombo.set_active_text(_("None"));
    linkCombo.set_sensitive(false);
    linkConnection = linkCombo.signal_changed().connect(sigc::mem_fun(*this, &InputDialogImpl::linkComboChanged));

    devDetails.attach(linkCombo, 1, 2, rowNum, rowNum + 1,
                      ::Gtk::FILL,
                      ::Gtk::SHRINK);
    rowNum++;

    lbl = Gtk::manage(new Gtk::Label(_("Axes count:")));
    devDetails.attach(*lbl, 0, 1, rowNum, rowNum+ 1,
                      ::Gtk::FILL,
                      ::Gtk::SHRINK);
    devDetails.attach(devAxesCount, 1, 2, rowNum, rowNum + 1,
                      ::Gtk::SHRINK,
                      ::Gtk::SHRINK);

    rowNum++;

/*
    lbl = Gtk::manage(new Gtk::Label(_("Actual axes count:")));
    devDetails.attach(*lbl, 0, 1, rowNum, rowNum+ 1,
                      ::Gtk::FILL,
                      ::Gtk::SHRINK);
    devDetails.attach(axesCombo, 1, 2, rowNum, rowNum + 1,
                      ::Gtk::SHRINK,
                      ::Gtk::SHRINK);

    rowNum++;
*/

    for ( guint barNum = 0; barNum < static_cast<guint>(G_N_ELEMENTS(axesValues)); barNum++ ) {
        lbl = Gtk::manage(new Gtk::Label(_("axis:")));
        devDetails.attach(*lbl, 0, 1, rowNum, rowNum+ 1,
                          ::Gtk::FILL,
                          ::Gtk::SHRINK);
        devDetails.attach(axesValues[barNum], 1, 2, rowNum, rowNum + 1,
                          ::Gtk::EXPAND,
                          ::Gtk::SHRINK);
        axesValues[barNum].set_sensitive(false);

        rowNum++;
    }

    lbl = Gtk::manage(new Gtk::Label(_("Button count:")));
    devDetails.attach(*lbl, 0, 1, rowNum, rowNum+ 1,
                      ::Gtk::FILL,
                      ::Gtk::SHRINK);
    devDetails.attach(devKeyCount, 1, 2, rowNum, rowNum + 1,
                      ::Gtk::SHRINK,
                      ::Gtk::SHRINK);

    rowNum++;

/*
    lbl = Gtk::manage(new Gtk::Label(_("Actual button count:")));
    devDetails.attach(*lbl, 0, 1, rowNum, rowNum+ 1,
                      ::Gtk::FILL,
                      ::Gtk::SHRINK);
    devDetails.attach(buttonCombo, 1, 2, rowNum, rowNum + 1,
                      ::Gtk::SHRINK,
                      ::Gtk::SHRINK);

    rowNum++;
*/

    devDetails.attach(keyVal, 0, 2, rowNum, rowNum + 1,
                      ::Gtk::FILL,
                      ::Gtk::SHRINK);
    rowNum++;


    testDetector.signal_event().connect(sigc::mem_fun(*this, &InputDialogImpl::eventSnoop));

//     void gdk_input_set_extension_events (GdkWindow        *window,
//                                          gint              mask,
//                                          GdkExtensionMode  mode);

    gtk_widget_set_extension_events( GTK_WIDGET(testDetector.gobj()), GDK_EXTENSION_EVENTS_ALL );
    testDetector.add_events(Gdk::POINTER_MOTION_MASK|Gdk::KEY_PRESS_MASK|Gdk::KEY_RELEASE_MASK |Gdk::PROXIMITY_IN_MASK|Gdk::PROXIMITY_OUT_MASK|Gdk::SCROLL_MASK);

    devDetails.attach(keyEntry, 0, 2, rowNum, rowNum + 1,
                      ::Gtk::FILL,
                      ::Gtk::SHRINK);
    rowNum++;


    devDetails.set_sensitive(false);
    detailScroller.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    detailScroller.add(devDetails);
    frame2.add(detailScroller);

//- 16x16/devices
// gnome-dev-mouse-optical
// input-mouse
// input-tablet
// mouse



    //Add the TreeView's view columns:
    tree.append_column("I", getCols().thumbnail);
    tree.append_column("Bar", getCols().description);

    tree.set_enable_tree_lines();
    tree.set_headers_visible(false);
    tree.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &InputDialogImpl::resyncToSelection));


    setupTree( store, tabletIter );

    Inkscape::DeviceManager::getManager().signalDeviceChanged().connect(sigc::mem_fun(*this, &InputDialogImpl::handleDeviceChange));
    Inkscape::DeviceManager::getManager().signalAxesChanged().connect(sigc::mem_fun(*this, &InputDialogImpl::updateDeviceAxes));
    Inkscape::DeviceManager::getManager().signalButtonsChanged().connect(sigc::mem_fun(*this, &InputDialogImpl::updateDeviceButtons));
    Inkscape::DeviceManager::getManager().signalLinkChanged().connect(sigc::bind<Gtk::TreeIter &, Gtk::TreeView &>(sigc::ptr_fun(&InputDialogImpl::updateDeviceLinks), tabletIter, tree));

    tree.expand_all();
    show_all_children();
}

void InputDialogImpl::setupTree( Glib::RefPtr<Gtk::TreeStore> store, Gtk::TreeIter &tablet )
{
    std::list<Glib::RefPtr<InputDevice const> > devList = Inkscape::DeviceManager::getManager().getDevices();
    if ( !devList.empty() ) {
        Gtk::TreeModel::Row row = *(store->append());
        row[getCols().description] = _("Hardware");

        tablet = store->append(row.children());
        Gtk::TreeModel::Row childrow = *tablet;
        childrow[getCols().description] = _("Tablet");
        childrow[getCols().thumbnail] = getPix(PIX_TABLET);

        for ( std::list<Glib::RefPtr<InputDevice const> >::iterator it = devList.begin(); it != devList.end(); ++it ) {
            Glib::RefPtr<InputDevice const> dev = *it;
            if ( dev ) {
//                 g_message("device: name[%s] source[0x%x] mode[0x%x] cursor[%s] axis count[%d] key count[%d]", dev->getName().c_str(), dev->getSource(), dev->getMode(),
//                           dev->hasCursor() ? "Yes":"no", dev->getNumAxes(), dev->getNumKeys());

//                 if ( dev->getSource() != Gdk::SOURCE_MOUSE ) {
                if ( dev ) {
                    Gtk::TreeModel::Row deviceRow = *(store->append(childrow.children()));
                    deviceRow[getCols().description] = dev->getName();
                    deviceRow[getCols().device] = dev;
                    deviceRow[getCols().mode] = dev->getMode();
                    switch ( dev->getSource() ) {
                        case GDK_SOURCE_MOUSE:
                            deviceRow[getCols().thumbnail] = getPix(PIX_CORE);
                            break;
                        case GDK_SOURCE_PEN:
                            if (deviceRow[getCols().description] == _("pad")) {
                                deviceRow[getCols().thumbnail] = getPix(PIX_SIDEBUTTONS);
                            } else {
                                deviceRow[getCols().thumbnail] = getPix(PIX_TIP);
                            }
                            break;
                        case GDK_SOURCE_CURSOR:
                            deviceRow[getCols().thumbnail] = getPix(PIX_MOUSE);
                            break;
                        case GDK_SOURCE_ERASER:
                            deviceRow[getCols().thumbnail] = getPix(PIX_ERASER);
                            break;
                        default:
                            ; // nothing
                    }
                }
            } else {
                g_warning("Null device in list");
            }
        }
    } else {
        g_warning("No devices found");
    }
}


InputDialogImpl::ConfPanel::ConfPanel() :
    Gtk::VBox(),
    store(Gtk::TreeStore::create(getCols())),
    tabletIter(),
    tree(store),
    treeScroller(),
    watcher(*this),
    useExt(_("Use pressure-sensitive tablet (requires restart)")),
    save(_("Save"))
{
    pack_start(treeScroller);

    treeScroller.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    treeScroller.add(tree);

    class Foo : public Gtk::TreeModel::ColumnRecord {
    public :
        Gtk::TreeModelColumn<Glib::ustring> one;
        Foo() {add(one);}
    };
    static Foo foo;
    Glib::RefPtr<Gtk::ListStore> poppers = Gtk::ListStore::create(foo);
    poppers->reference();

    Gtk::TreeModel::Row row = *(poppers->append());
    row[foo.one] = getModeToString()[Gdk::MODE_DISABLED];
    row = *(poppers->append());
    row[foo.one] = getModeToString()[Gdk::MODE_SCREEN];
    row = *(poppers->append());
    row[foo.one] = getModeToString()[Gdk::MODE_WINDOW];

    Gtk::CellRendererCombo *rendr = new Gtk::CellRendererCombo();
    rendr->property_model().set_value(poppers);
    rendr->property_text_column().set_value(0);
    rendr->property_has_entry() = false;

    //Add the TreeView's view columns:
    tree.append_column("I", getCols().thumbnail);
    tree.append_column("Bar", getCols().description);
    Gtk::TreeViewColumn *col = new Gtk::TreeViewColumn("X", *rendr);
    if (col) {
        tree.append_column(*col);
        col->set_cell_data_func(*rendr, sigc::ptr_fun(setModeCellString));
        rendr->signal_edited().connect(sigc::bind(sigc::ptr_fun(commitCellModeChange), store));
        rendr->property_editable() = true;
    }

    tree.set_enable_tree_lines();
    tree.set_headers_visible(false);

    setupTree( store, tabletIter );

    Inkscape::DeviceManager::getManager().signalLinkChanged().connect(sigc::bind<Gtk::TreeIter &, Gtk::TreeView &>(sigc::ptr_fun(&InputDialogImpl::updateDeviceLinks), tabletIter, tree));

    tree.expand_all();

    useExt.set_active(Preferences::get()->getBool("/options/useextinput/value"));
    useExt.signal_toggled().connect(sigc::mem_fun(*this, &InputDialogImpl::ConfPanel::useExtToggled));
    pack_start(useExt, Gtk::PACK_SHRINK);

    save.signal_clicked().connect(sigc::mem_fun(*this, &InputDialogImpl::ConfPanel::saveSettings));
    Gtk::Alignment *align = new Gtk::Alignment(Gtk::ALIGN_RIGHT, Gtk::ALIGN_TOP, 0, 0);
    align->add(save);
    pack_start(*Gtk::manage(align), Gtk::PACK_SHRINK);
}

InputDialogImpl::ConfPanel::~ConfPanel()
{
}

void InputDialogImpl::ConfPanel::setModeCellString(Gtk::CellRenderer *rndr, Gtk::TreeIter const &iter)
{
    if (iter) {
        Gtk::CellRendererCombo *combo = dynamic_cast<Gtk::CellRendererCombo *>(rndr);
        if (combo) {
            Glib::RefPtr<InputDevice const> dev = (*iter)[getCols().device];
            Gdk::InputMode mode = (*iter)[getCols().mode];
            if (dev && (getModeToString().find(mode) != getModeToString().end())) {
                combo->property_text() = getModeToString()[mode];
            } else {
                combo->property_text() = "";
            }
        }
    }
}

void InputDialogImpl::ConfPanel::commitCellModeChange(Glib::ustring const &path, Glib::ustring const &newText, Glib::RefPtr<Gtk::TreeStore> store)
{
    Gtk::TreeIter iter = store->get_iter(path);
    if (iter) {
        Glib::RefPtr<InputDevice const> dev = (*iter)[getCols().device];
        if (dev && (getStringToMode().find(newText) != getStringToMode().end())) {
            Gdk::InputMode mode = getStringToMode()[newText];
            Inkscape::DeviceManager::getManager().setMode( dev->getId(), mode );
        }
    }
}

void InputDialogImpl::ConfPanel::saveSettings()
{
    Inkscape::DeviceManager::getManager().saveConfig();
}

void InputDialogImpl::ConfPanel::useExtToggled()
{
    bool active = useExt.get_active();
    if (active != Preferences::get()->getBool("/options/useextinput/value")) {
        Preferences::get()->setBool("/options/useextinput/value", active);
        if (active) {
            // As a work-around for a common problem, enable tablet toggles on the calligraphic tool.
            // Covered in Launchpad bug #196195.
            Preferences::get()->setBool("/tools/tweak/usepressure", true);
            Preferences::get()->setBool("/tools/calligraphic/usepressure", true);
            Preferences::get()->setBool("/tools/calligraphic/usetilt", true);
        }
    }
}

InputDialogImpl::ConfPanel::Blink::Blink(ConfPanel &parent) :
    Preferences::Observer("/options/useextinput/value"),
    parent(parent)
{
    Preferences::get()->addObserver(*this);
}

InputDialogImpl::ConfPanel::Blink::~Blink()
{
    Preferences::get()->removeObserver(*this);
}

void InputDialogImpl::ConfPanel::Blink::notify(Preferences::Entry const &new_val)
{
    parent.useExt.set_active(new_val.getBool());
}

void InputDialogImpl::handleDeviceChange(const Glib::RefPtr<InputDevice>& device)
{
//     g_message("OUCH!!!! for %p  hits %s", &device, device->getId().c_str());
    std::vector<Glib::RefPtr<Gtk::TreeStore> > stores;
    stores.push_back(store);
    stores.push_back(cfgPanel.store);

    for (std::vector<Glib::RefPtr<Gtk::TreeStore> >::iterator it = stores.begin(); it != stores.end(); ++it) {
        Gtk::TreeModel::iterator deviceIter;
        (*it)->foreach_iter( sigc::bind<Glib::ustring, Gtk::TreeModel::iterator*>(
                                 sigc::ptr_fun(&InputDialogImpl::findDevice),
                                 device->getId(),
                                 &deviceIter) );
        if ( deviceIter ) {
            Gdk::InputMode mode = device->getMode();
            Gtk::TreeModel::Row row = *deviceIter;
            if (row[getCols().mode] != mode) {
                row[getCols().mode] = mode;
            }
        }
    }
}

void InputDialogImpl::updateDeviceAxes(const Glib::RefPtr<InputDevice>& device)
{
    gint live = device->getLiveAxes();

    std::map<guint, std::pair<guint, gdouble> > existing = axesMap[device->getId()];
    gint mask = 0x1;
    for ( gint num = 0; num < 32; num++, mask <<= 1) {
        if ( (mask & live) != 0 ) {
            if ( (existing.find(num) == existing.end()) || (existing[num].first < 2) ) {
                axesMap[device->getId()][num].first = 2;
                axesMap[device->getId()][num].second = 0.0;
            }
        }
    }
    updateTestAxes( device->getId(), 0 );
}

void InputDialogImpl::updateDeviceButtons(const Glib::RefPtr<InputDevice>& device)
{
    gint live = device->getLiveButtons();
    std::set<guint> existing = buttonMap[device->getId()];
    gint mask = 0x1;
    for ( gint num = 0; num < 32; num++, mask <<= 1) {
        if ( (mask & live) != 0 ) {
            if ( existing.find(num) == existing.end() ) {
                buttonMap[device->getId()].insert(num);
            }
        }
    }
    updateTestButtons(device->getId(), -1);
}


bool InputDialogImpl::findDevice(const Gtk::TreeModel::iterator& iter,
                                 Glib::ustring id,
                                 Gtk::TreeModel::iterator* result)
{
    bool stop = false;
    Glib::RefPtr<InputDevice const> dev = (*iter)[getCols().device];
    if ( dev && (dev->getId() == id) ) {
        if ( result ) {
            *result = iter;
        }
        stop = true;
    }
    return stop;
}

bool InputDialogImpl::findDeviceByLink(const Gtk::TreeModel::iterator& iter,
                                       Glib::ustring link,
                                       Gtk::TreeModel::iterator* result)
{
    bool stop = false;
    Glib::RefPtr<InputDevice const> dev = (*iter)[getCols().device];
    if ( dev && (dev->getLink() == link) ) {
        if ( result ) {
            *result = iter;
        }
        stop = true;
    }
    return stop;
}

void InputDialogImpl::updateDeviceLinks(const Glib::RefPtr<InputDevice>& device, Gtk::TreeIter &tabletIter, Gtk::TreeView &tree)
{
    Glib::RefPtr<Gtk::TreeStore> store = Glib::RefPtr<Gtk::TreeStore>::cast_dynamic(tree.get_model());

//     g_message("Links!!!! for %p  hits [%s]  with link of [%s]", &device, device->getId().c_str(), device->getLink().c_str());
    Gtk::TreeModel::iterator deviceIter;
    store->foreach_iter( sigc::bind<Glib::ustring, Gtk::TreeModel::iterator*>(
                             sigc::ptr_fun(&InputDialogImpl::findDevice),
                             device->getId(),
                             &deviceIter) );

    if ( deviceIter ) {
        // Found the device concerned. Can proceed.

        if ( device->getLink().empty() ) {
            // is now unlinked
//             g_message("Item %s is unlinked", device->getId().c_str());
            if ( deviceIter->parent() != tabletIter ) {
                // Not the child of the tablet. move on up

                Glib::RefPtr<InputDevice const> dev = (*deviceIter)[getCols().device];
                Glib::ustring descr = (*deviceIter)[getCols().description];
                Glib::RefPtr<Gdk::Pixbuf> thumb = (*deviceIter)[getCols().thumbnail];

                Gtk::TreeModel::Row deviceRow = *store->append(tabletIter->children());
                deviceRow[getCols().description] = descr;
                deviceRow[getCols().thumbnail] = thumb;
                deviceRow[getCols().device] = dev;
                deviceRow[getCols().mode] = dev->getMode();

                Gtk::TreeModel::iterator oldParent = deviceIter->parent();
                store->erase(deviceIter);
                if ( oldParent->children().empty() ) {
                    store->erase(oldParent);
                }
            }
        } else {
            // is linking
            if ( deviceIter->parent() == tabletIter ) {
                // Simple case. Not already linked

                Gtk::TreeIter newGroup = store->append(tabletIter->children());
                (*newGroup)[getCols().description] = _("Pen");
                (*newGroup)[getCols().thumbnail] = getPix(PIX_PEN);

                Glib::RefPtr<InputDevice const> dev = (*deviceIter)[getCols().device];
                Glib::ustring descr = (*deviceIter)[getCols().description];
                Glib::RefPtr<Gdk::Pixbuf> thumb = (*deviceIter)[getCols().thumbnail];

                Gtk::TreeModel::Row deviceRow = *store->append(newGroup->children());
                deviceRow[getCols().description] = descr;
                deviceRow[getCols().thumbnail] = thumb;
                deviceRow[getCols().device] = dev;
                deviceRow[getCols().mode] = dev->getMode();


                Gtk::TreeModel::iterator linkIter;
                store->foreach_iter( sigc::bind<Glib::ustring, Gtk::TreeModel::iterator*>(
                                         sigc::ptr_fun(&InputDialogImpl::findDeviceByLink),
                                         device->getId(),
                                         &linkIter) );
                if ( linkIter ) {
                    dev = (*linkIter)[getCols().device];
                    descr = (*linkIter)[getCols().description];
                    thumb = (*linkIter)[getCols().thumbnail];

                    deviceRow = *store->append(newGroup->children());
                    deviceRow[getCols().description] = descr;
                    deviceRow[getCols().thumbnail] = thumb;
                    deviceRow[getCols().device] = dev;
                    deviceRow[getCols().mode] = dev->getMode();
                    Gtk::TreeModel::iterator oldParent = linkIter->parent();
                    store->erase(linkIter);
                    if ( oldParent->children().empty() ) {
                        store->erase(oldParent);
                    }
                }

                Gtk::TreeModel::iterator oldParent = deviceIter->parent();
                store->erase(deviceIter);
                if ( oldParent->children().empty() ) {
                    store->erase(oldParent);
                }
                tree.expand_row(Gtk::TreePath(newGroup), true);
            }
        }
    }
}

void InputDialogImpl::linkComboChanged() {
    Glib::RefPtr<Gtk::TreeSelection> treeSel = tree.get_selection();
    Gtk::TreeModel::iterator iter = treeSel->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring val = row[getCols().description];
        Glib::RefPtr<InputDevice const> dev = row[getCols().device];
        if ( dev ) {
            if ( linkCombo.get_active_row_number() == 0 ) {
                // It is the "None" entry
                DeviceManager::getManager().setLinkedTo(dev->getId(), "");
            } else {
                Glib::ustring linkName = linkCombo.get_active_text();
                std::list<Glib::RefPtr<InputDevice const> > devList = Inkscape::DeviceManager::getManager().getDevices();
                for ( std::list<Glib::RefPtr<InputDevice const> >::const_iterator it = devList.begin(); it != devList.end(); ++it ) {
                    if ( linkName == (*it)->getName() ) {
                        DeviceManager::getManager().setLinkedTo(dev->getId(), (*it)->getId());
                        break;
                    }
                }
            }
        }
    }
}

void InputDialogImpl::resyncToSelection() {
    bool clear = true;
    Glib::RefPtr<Gtk::TreeSelection> treeSel = tree.get_selection();
    Gtk::TreeModel::iterator iter = treeSel->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring val = row[getCols().description];
        Glib::RefPtr<InputDevice const> dev = row[getCols().device];
        if ( dev ) {
            devDetails.set_sensitive(true);

            linkConnection.block();
            linkCombo.clear_items();
            linkCombo.append_text(_("None"));
            linkCombo.set_active(0);
            if ( dev->getSource() != Gdk::SOURCE_MOUSE ) {
                Glib::ustring linked = dev->getLink();
                std::list<Glib::RefPtr<InputDevice const> > devList = Inkscape::DeviceManager::getManager().getDevices();
                for ( std::list<Glib::RefPtr<InputDevice const> >::const_iterator it = devList.begin(); it != devList.end(); ++it ) {
                    if ( ((*it)->getSource() != Gdk::SOURCE_MOUSE) && ((*it) != dev) ) {
                        linkCombo.append_text((*it)->getName().c_str());
                        if ( (linked.length() > 0) && (linked == (*it)->getId()) ) {
                            linkCombo.set_active_text((*it)->getName().c_str());
                        }
                    }
                }
                linkCombo.set_sensitive(true);
            } else {
                linkCombo.set_sensitive(false);
            }
            linkConnection.unblock();

            clear = false;
            devName.set_label(row[getCols().description]);
            setupValueAndCombo( dev->getNumAxes(), dev->getNumAxes(), devAxesCount, axesCombo);
            setupValueAndCombo( dev->getNumKeys(), dev->getNumKeys(), devKeyCount, buttonCombo);
        }
    }

    devDetails.set_sensitive(!clear);
    if (clear) {
        devName.set_label("");
        devAxesCount.set_label("");
        devKeyCount.set_label("");
    }
}

void InputDialogImpl::setupValueAndCombo( gint reported, gint actual, Gtk::Label& label, Gtk::ComboBoxText& combo )
{
    gchar *tmp = g_strdup_printf("%d", reported);
    label.set_label(tmp);
    g_free(tmp);

    combo.clear_items();
    for ( gint i = 1; i <= reported; ++i ) {
        tmp = g_strdup_printf("%d", i);
        combo.append_text(tmp);
        g_free(tmp);
    }

    if ( (1 <= actual) && (actual <= reported) ) {
        combo.set_active(actual - 1);
    }
}

void InputDialogImpl::updateTestButtons( Glib::ustring const& key, gint hotButton )
{
    for ( gint i = 0; i < static_cast<gint>(G_N_ELEMENTS(testButtons)); i++ ) {
        if ( buttonMap[key].find(i) != buttonMap[key].end() ) {
            if ( i == hotButton ) {
                testButtons[i].set(getPix(PIX_BUTTONS_ON));
            } else {
                testButtons[i].set(getPix(PIX_BUTTONS_OFF));
            }
        } else {
            testButtons[i].set(getPix(PIX_BUTTONS_NONE));
        }
    }
}

void InputDialogImpl::updateTestAxes( Glib::ustring const& key, GdkDevice* dev )
{
    static gdouble epsilon = 0.0001;
    {
        Glib::RefPtr<Gtk::TreeSelection> treeSel = tree.get_selection();
        Gtk::TreeModel::iterator iter = treeSel->get_selected();
        if (iter) {
            Gtk::TreeModel::Row row = *iter;
            Glib::ustring val = row[getCols().description];
            Glib::RefPtr<InputDevice const> idev = row[getCols().device];
            if ( !idev || (idev->getId() != key) ) {
                dev = 0;
            }
        }
    }


    for ( gint i = 0; i < static_cast<gint>(G_N_ELEMENTS(testAxes)); i++ ) {
        if ( axesMap[key].find(i) != axesMap[key].end() ) {
            switch ( axesMap[key][i].first ) {
                case 0:
                case 1:
                    testAxes[i].set(getPix(PIX_AXIS_NONE));
                    if ( dev && (i < static_cast<gint>(G_N_ELEMENTS(axesValues)) ) ) {
                        axesValues[i].set_sensitive(false);
                    }
                    break;
                case 2:
                    testAxes[i].set(getPix(PIX_AXIS_OFF));
                    axesValues[i].set_sensitive(true);
                    if ( dev && (i < static_cast<gint>(G_N_ELEMENTS(axesValues)) ) ) {
                        if ( (dev->axes[i].max - dev->axes[i].min) > epsilon ) {
                            axesValues[i].set_sensitive(true);
                            axesValues[i].set_fraction( (axesMap[key][i].second- dev->axes[i].min) / (dev->axes[i].max - dev->axes[i].min) );
                        }
                        gchar* str = g_strdup_printf("%f", axesMap[key][i].second);
                        axesValues[i].set_text(str);
                        g_free(str);
                    }
                    break;
                case 3:
                    testAxes[i].set(getPix(PIX_AXIS_ON));
                    axesValues[i].set_sensitive(true);
                    if ( dev && (i < static_cast<gint>(G_N_ELEMENTS(axesValues)) ) ) {
                        if ( (dev->axes[i].max - dev->axes[i].min) > epsilon ) {
                            axesValues[i].set_sensitive(true);
                            axesValues[i].set_fraction( (axesMap[key][i].second- dev->axes[i].min) / (dev->axes[i].max - dev->axes[i].min) );
                        }
                        gchar* str = g_strdup_printf("%f", axesMap[key][i].second);
                        axesValues[i].set_text(str);
                        g_free(str);
                    }
            }

        } else {
            testAxes[i].set(getPix(PIX_AXIS_NONE));
        }
    }
    if ( !dev ) {
        for ( gint i = 0; i < static_cast<gint>(G_N_ELEMENTS(axesValues)); i++ ) {
            axesValues[i].set_fraction(0.0);
            axesValues[i].set_text("");
            axesValues[i].set_sensitive(false);
        }
    }
}

void InputDialogImpl::mapAxesValues( Glib::ustring const& key, guint numAxes, gdouble const * axes, GdkDevice* dev )
{
    static gdouble epsilon = 0.0001;
    if ( (numAxes > 0) && axes) {
        for ( guint axisNum = 0; axisNum < numAxes; axisNum++ ) {
            // 0 == new, 1 == set value, 2 == changed value, 3 == active
            gdouble diff = axesMap[key][axisNum].second - axes[axisNum];
            switch(axesMap[key][axisNum].first) {
                case 0:
                {
                    axesMap[key][axisNum].first = 1;
                    axesMap[key][axisNum].second = axes[axisNum];
                }
                break;
                case 1:
                {
                    if ( (diff > epsilon) || (diff < -epsilon) ) {
//                         g_message("Axis %d changed on %s]", axisNum, key.c_str());
                        axesMap[key][axisNum].first = 3;
                        axesMap[key][axisNum].second = axes[axisNum];
                        updateTestAxes(key, dev);
                        DeviceManager::getManager().addAxis(key, axisNum);
                    }
                }
                break;
                case 2:
                {
                    if ( (diff > epsilon) || (diff < -epsilon) ) {
                        axesMap[key][axisNum].first = 3;
                        axesMap[key][axisNum].second = axes[axisNum];
                        updateTestAxes(key, dev);
                    }
                }
                break;
                case 3:
                {
                    if ( (diff > epsilon) || (diff < -epsilon) ) {
                        axesMap[key][axisNum].second = axes[axisNum];
                    } else {
                        axesMap[key][axisNum].first = 2;
                        updateTestAxes(key, dev);
                    }
                }
            }
        }
    }
    // std::map<Glib::ustring, std::map<guint, std::pair<guint, gdouble> > > axesMap;
}

Glib::ustring InputDialogImpl::getKeyFor( GdkDevice* device )
{
    Glib::ustring key;
    switch ( device->source ) {
        case GDK_SOURCE_MOUSE:
            key = "M:";
            break;
        case GDK_SOURCE_CURSOR:
            key = "C:";
            break;
        case GDK_SOURCE_PEN:
            key = "P:";
            break;
        case GDK_SOURCE_ERASER:
            key = "E:";
            break;
        default:
            key = "?:";
    }
    key += device->name;

    return key;
}

bool InputDialogImpl::eventSnoop(GdkEvent* event)
{
    int modmod = 0;

    GdkInputSource source = lastSourceSeen;
    Glib::ustring devName = lastDevnameSeen;
    Glib::ustring key;
    gint hotButton = -1;

    switch ( event->type ) {
        case GDK_KEY_PRESS:
        case GDK_KEY_RELEASE:
        {
            GdkEventKey* keyEvt = reinterpret_cast<GdkEventKey*>(event);
            gchar* name = gtk_accelerator_name(keyEvt->keyval, static_cast<GdkModifierType>(keyEvt->state));
            keyVal.set_label(name);
//             g_message("%d KEY    state:0x%08x  0x%04x [%s]", keyEvt->type, keyEvt->state, keyEvt->keyval, name);
            g_free(name);
        }
        break;
        case GDK_BUTTON_PRESS:
            modmod = 1;
            // fallthrough
        case GDK_BUTTON_RELEASE:
        {
            GdkEventButton* btnEvt = reinterpret_cast<GdkEventButton*>(event);
            if ( btnEvt->device ) {
                key = getKeyFor(btnEvt->device);
                source = btnEvt->device->source;
                devName = btnEvt->device->name;

                mapAxesValues(key, btnEvt->device->num_axes, btnEvt->axes, btnEvt->device);
                if ( buttonMap[key].find(btnEvt->button) == buttonMap[key].end() ) {
//                     g_message("New button found for %s = %d", key.c_str(), btnEvt->button);
                    buttonMap[key].insert(btnEvt->button);
                    DeviceManager::getManager().addButton(key, btnEvt->button);
                }
                hotButton = modmod ? btnEvt->button : -1;
                updateTestButtons(key, hotButton);
            }
            gchar* name = gtk_accelerator_name(0, static_cast<GdkModifierType>(btnEvt->state));
            keyVal.set_label(name);
//             g_message("%d BTN    state:0x%08x %c  %4d [%s] dev:%p [%s]  ",
//                       btnEvt->type, btnEvt->state,
//                       (modmod ? '+':'-'),
//                       btnEvt->button, name, btnEvt->device,
//                       (btnEvt->device ? btnEvt->device->name : "null")

//                 );
            g_free(name);
        }
        break;
        case GDK_MOTION_NOTIFY:
        {
            GdkEventMotion* btnMtn = reinterpret_cast<GdkEventMotion*>(event);
            if ( btnMtn->device ) {
                key = getKeyFor(btnMtn->device);
                source = btnMtn->device->source;
                devName = btnMtn->device->name;
                mapAxesValues(key, btnMtn->device->num_axes, btnMtn->axes, btnMtn->device);
            }
            gchar* name = gtk_accelerator_name(0, static_cast<GdkModifierType>(btnMtn->state));
            keyVal.set_label(name);
//             g_message("%d MOV    state:0x%08x         [%s] dev:%p [%s] %3.2f %3.2f %3.2f %3.2f %3.2f %3.2f", btnMtn->type, btnMtn->state,
//                       name, btnMtn->device,
//                       (btnMtn->device ? btnMtn->device->name : "null"),
//                       ((btnMtn->device && btnMtn->axes && (btnMtn->device->num_axes > 0)) ? btnMtn->axes[0]:0),
//                       ((btnMtn->device && btnMtn->axes && (btnMtn->device->num_axes > 1)) ? btnMtn->axes[1]:0),
//                       ((btnMtn->device && btnMtn->axes && (btnMtn->device->num_axes > 2)) ? btnMtn->axes[2]:0),
//                       ((btnMtn->device && btnMtn->axes && (btnMtn->device->num_axes > 3)) ? btnMtn->axes[3]:0),
//                       ((btnMtn->device && btnMtn->axes && (btnMtn->device->num_axes > 4)) ? btnMtn->axes[4]:0),
//                       ((btnMtn->device && btnMtn->axes && (btnMtn->device->num_axes > 5)) ? btnMtn->axes[5]:0)
//                 );
            g_free(name);
        }
        break;
        default:
            ;// nothing
    }


    if ( (lastSourceSeen != source) || (lastDevnameSeen != devName) ) {
        switch (source) {
            case GDK_SOURCE_MOUSE:
            {
                testThumb.set(getPix(PIX_CORE));
            }
            break;
            case GDK_SOURCE_CURSOR:
            {
//                 g_message("flip to cursor");
                testThumb.set(getPix(PIX_MOUSE));
            }
            break;
            case GDK_SOURCE_PEN:
            {
                if (devName == _("pad")) {
//                     g_message("flip to pad");
                    testThumb.set(getPix(PIX_SIDEBUTTONS));
                } else {
//                     g_message("flip to pen");
                    testThumb.set(getPix(PIX_TIP));
                }
            }
            break;
            case GDK_SOURCE_ERASER:
            {
//                 g_message("flip to eraser");
                testThumb.set(getPix(PIX_ERASER));
            }
            break;
//             default:
//                 g_message("gurgle");
        }
        updateTestButtons(key, hotButton);
        lastSourceSeen = source;
        lastDevnameSeen = devName;
    }

    return false;
}


} // end namespace Inkscape
} // end namespace UI
} // end namespace Dialog


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
