

/* XPM */
static char *eraser[] = {
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
static char *mouse[] = {
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
static char *pen[] = {
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
static char *sidebuttons[] = {
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
static char *tablet[] = {
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
static char *tip[] = {
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
"OOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOOOOO"
};






#include <map>
#include <set>
#include <glib/gprintf.h>
#include <glibmm/i18n.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/enums.h>
#include <gtkmm/frame.h>
#include <gtkmm/image.h>
#include <gtkmm/menubar.h>
#include <gtkmm/notebook.h>
#include <gtkmm/paned.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/table.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>

#include "ui/widget/panel.h"

#include "input.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

static void createFakeList();
GdkDevice fakeout[5];
static GList* fakeList = 0;



class MyModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:
    Gtk::TreeModelColumn<Glib::ustring>                filename;
    Gtk::TreeModelColumn<Glib::ustring>                description;
    Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> >  thumbnail;
    Gtk::TreeModelColumn<GdkDevice*>                   device;

    MyModelColumns() { add(filename); add(description); add(thumbnail); add(device); }
};

class InputDialogImpl : public InputDialog {
public:
    InputDialogImpl();
    virtual ~InputDialogImpl() {}

private:
    Glib::RefPtr<Gdk::Pixbuf> penPix;
    Glib::RefPtr<Gdk::Pixbuf> mousePix;
    Glib::RefPtr<Gdk::Pixbuf> tipPix;
    Glib::RefPtr<Gdk::Pixbuf> tabletPix;
    Glib::RefPtr<Gdk::Pixbuf> eraserPix;
    Glib::RefPtr<Gdk::Pixbuf> sidebuttonsPix;

    std::map<Glib::ustring, std::set<guint> > buttonMap;

    GdkInputSource lastSourceSeen;
    Glib::ustring lastDevnameSeen;

    MyModelColumns cols;
    Glib::RefPtr<Gtk::TreeStore> store;
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
    Gtk::ComboBoxText buttonCombo;
    Gtk::ComboBoxText modeCombo;
    Gtk::Label keyVal;
    Gtk::Entry keyEntry;
    Gtk::Table devDetails;
    Gtk::HPaned confSplitter;
    Gtk::Notebook topHolder;
    Gtk::Image testThumb;
    Gtk::EventBox testDetector;

    void setupValueAndCombo( gint reported, gint actual, Gtk::Label& label, Gtk::ComboBoxText& combo );
    bool eventSnoop(GdkEvent* event);
    void foo();
};


// Now that we've defined the *Impl class, we can do the method to aquire one.
InputDialog &InputDialog::getInstance()
{
    InputDialog *dialog = new InputDialogImpl();
    return *dialog;
}


InputDialogImpl::InputDialogImpl() :
    InputDialog(),

    penPix(Gdk::Pixbuf::create_from_xpm_data(pen)),
    mousePix(Gdk::Pixbuf::create_from_xpm_data(mouse)),
    tipPix(Gdk::Pixbuf::create_from_xpm_data(tip)),
    tabletPix(Gdk::Pixbuf::create_from_xpm_data(tablet)),
    eraserPix(Gdk::Pixbuf::create_from_xpm_data(eraser)),
    sidebuttonsPix(Gdk::Pixbuf::create_from_xpm_data(sidebuttons)),

    lastSourceSeen((GdkInputSource)-1),
    lastDevnameSeen(""),
    cols(),
    store(Gtk::TreeStore::create(cols)),
    tree(store),
    frame2(),
    testFrame("Test Area"),
    treeScroller(),
    detailScroller(),
    splitter(),
    split2(),
    modeCombo(),
    devDetails(6, 2),
    confSplitter(),
    topHolder()
{
    Gtk::Box *contents = _getContents();


    treeScroller.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    treeScroller.add(tree);
    split2.pack1(testFrame);
    split2.pack2(frame2);
    splitter.pack1(treeScroller);
    splitter.pack2(split2);

    testDetector.add(testThumb);
    testFrame.add(testDetector);
    testThumb.set(tabletPix);
    testThumb.set_padding(24, 24);

    topHolder.append_page(confSplitter, "Configuration");
    topHolder.append_page(splitter, "Hardware");
//     confSplitter.show_all();
//     splitter.show_all();
    topHolder.show_all();
    topHolder.set_current_page(1);

    contents->pack_start(topHolder);

    int rowNum = 0;

    Gtk::Label* lbl = Gtk::manage(new Gtk::Label("Name:"));
    devDetails.attach(*lbl, 0, 1, rowNum, rowNum+ 1,
                      ::Gtk::FILL,
                      ::Gtk::SHRINK);
    devDetails.attach(devName, 1, 2, rowNum, rowNum + 1,
                      ::Gtk::SHRINK,
                      ::Gtk::SHRINK);

    rowNum++;

    lbl = Gtk::manage(new Gtk::Label("Mode:"));
    devDetails.attach(*lbl, 0, 1, rowNum, rowNum+ 1,
                      ::Gtk::FILL,
                      ::Gtk::SHRINK);

    modeCombo.append_text("Disabled");
    modeCombo.append_text("Screen");
    modeCombo.append_text("Window");
    modeCombo.set_active_text("Disabled");
    modeCombo.set_sensitive(false);

    devDetails.attach(modeCombo, 1, 2, rowNum, rowNum + 1,
                      ::Gtk::FILL,
                      ::Gtk::SHRINK);
    rowNum++;

    lbl = Gtk::manage(new Gtk::Label("Reported axes count:"));
    devDetails.attach(*lbl, 0, 1, rowNum, rowNum+ 1,
                      ::Gtk::FILL,
                      ::Gtk::SHRINK);
    devDetails.attach(devAxesCount, 1, 2, rowNum, rowNum + 1,
                      ::Gtk::SHRINK,
                      ::Gtk::SHRINK);

    rowNum++;

    lbl = Gtk::manage(new Gtk::Label("Actual axes count:"));
    devDetails.attach(*lbl, 0, 1, rowNum, rowNum+ 1,
                      ::Gtk::FILL,
                      ::Gtk::SHRINK);
    devDetails.attach(axesCombo, 1, 2, rowNum, rowNum + 1,
                      ::Gtk::SHRINK,
                      ::Gtk::SHRINK);

    rowNum++;

    lbl = Gtk::manage(new Gtk::Label("Reported button count:"));
    devDetails.attach(*lbl, 0, 1, rowNum, rowNum+ 1,
                      ::Gtk::FILL,
                      ::Gtk::SHRINK);
    devDetails.attach(devKeyCount, 1, 2, rowNum, rowNum + 1,
                      ::Gtk::SHRINK,
                      ::Gtk::SHRINK);

    rowNum++;

    lbl = Gtk::manage(new Gtk::Label("Actual button count:"));
    devDetails.attach(*lbl, 0, 1, rowNum, rowNum+ 1,
                      ::Gtk::FILL,
                      ::Gtk::SHRINK);
    devDetails.attach(buttonCombo, 1, 2, rowNum, rowNum + 1,
                      ::Gtk::SHRINK,
                      ::Gtk::SHRINK);

    rowNum++;

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



    Gtk::TreeModel::Row row;
    Gtk::TreeModel::Row childrow;
    Gtk::TreeModel::Row deviceRow;


    //Add the TreeView's view columns:
    tree.append_column("I", cols.thumbnail);
    tree.append_column("Bar", cols.description);

    tree.set_enable_tree_lines();
    tree.set_headers_visible(false);
    tree.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &InputDialogImpl::foo));


    GList* devList = gdk_devices_list();
    if ( !fakeList ) {
        createFakeList();
    }
    devList = fakeList;
    if ( devList ) {
        g_message("Found some");

        {
            GdkModifierType  defaultModMask = static_cast<GdkModifierType>(gtk_accelerator_get_default_mod_mask());
            gchar* name = gtk_accelerator_name(GDK_a, defaultModMask);
            gchar* label = gtk_accelerator_get_label(GDK_a, defaultModMask);
            g_message("Name: [%s]  label:[%s]", name, label);
            g_free(name);
            g_free(label);
        }

        row = *(store->append());
        row[cols.description] = "Hardware";

        childrow = *(store->append(row.children()));
        childrow[cols.description] = "Tablet";
        childrow[cols.thumbnail] = tabletPix;

        for ( GList* curr = devList; curr; curr = g_list_next(curr) ) {
            GdkDevice* dev = reinterpret_cast<GdkDevice*>(curr->data);
            if ( dev ) {
                g_message("device: name[%s] source[0x%x] mode[0x%x] cursor[%s] axis count[%d] key count[%d]", dev->name, dev->source, dev->mode,
                          dev->has_cursor?"Yes":"no", dev->num_axes, dev->num_keys);

                if ( dev->source != GDK_SOURCE_MOUSE ) {
                    deviceRow = *(store->append(childrow.children()));
                    deviceRow[cols.description] = dev->name;
                    deviceRow[cols.device] = dev;
                    switch ( dev->source ) {
                        case GDK_SOURCE_PEN:
                            if (deviceRow[cols.description] == "pad") {
                                deviceRow[cols.thumbnail] = sidebuttonsPix;
                            } else {
                                deviceRow[cols.thumbnail] = tipPix;
                            }
                            break;
                        case GDK_SOURCE_CURSOR:
                            deviceRow[cols.thumbnail] = mousePix;
                            break;
                        case GDK_SOURCE_ERASER:
                            deviceRow[cols.thumbnail] = eraserPix;
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
        g_message("NO DEVICES FOUND");
    }


    tree.expand_all();
    show_all_children();
}

void InputDialogImpl::foo() {
    bool clear = true;
    Glib::RefPtr<Gtk::TreeSelection> treeSel = tree.get_selection();
    Gtk::TreeModel::iterator iter = treeSel->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring val = row[cols.description];
        GdkDevice* dev = row[cols.device];
        if ( dev ) {
            devDetails.set_sensitive(true);
            modeCombo.set_sensitive(true);
            switch( dev->mode ) {
                case GDK_MODE_DISABLED:
                    modeCombo.set_active(0);
                    break;
                case GDK_MODE_SCREEN:
                    modeCombo.set_active(1);
                    break;
                case GDK_MODE_WINDOW:
                    modeCombo.set_active(2);
                    break;
                default:
                    ;
            }
            clear = false;
            devName.set_label(row[cols.description]);
            setupValueAndCombo( dev->num_axes, dev->num_axes, devAxesCount, axesCombo);
            setupValueAndCombo( dev->num_keys, dev->num_keys, devKeyCount, buttonCombo);
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

bool InputDialogImpl::eventSnoop(GdkEvent* event)
{
    int modmod = 0;

    GdkInputSource source = lastSourceSeen;
    Glib::ustring devName = lastDevnameSeen;

    switch ( event->type ) {
        case GDK_KEY_PRESS:
        case GDK_KEY_RELEASE:
        {
            GdkEventKey* keyEvt = reinterpret_cast<GdkEventKey*>(event);
            gchar* name = gtk_accelerator_name(keyEvt->keyval, static_cast<GdkModifierType>(keyEvt->state));
            keyVal.set_label(name);
            g_message("%d KEY    state:0x%08x  0x%04x [%s]", keyEvt->type, keyEvt->state, keyEvt->keyval, name);
            g_free(name);
        }
        break;
        case GDK_BUTTON_PRESS:
            modmod = 1;
            // fallthrough
        case GDK_BUTTON_RELEASE:
        {
            GdkEventButton* btnEvt = reinterpret_cast<GdkEventButton*>(event);
            Glib::ustring key;
            if ( btnEvt->device ) {
                source = btnEvt->device->source;
                devName = btnEvt->device->name;
                switch ( btnEvt->device->source ) {
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
                key += devName;

                if ( buttonMap[key].find(btnEvt->button) == buttonMap[key].end() ) {
                    g_message("New button found for %s = %d", key.c_str(), btnEvt->button);
                    buttonMap[key].insert(btnEvt->button);
                }
            }
            gchar* name = gtk_accelerator_name(0, static_cast<GdkModifierType>(btnEvt->state));
            keyVal.set_label(name);
            g_message("%d BTN    state:0x%08x %c  %4d [%s] dev:%p [%s]  ",
                      btnEvt->type, btnEvt->state,
                      (modmod ? '+':'-'),
                      btnEvt->button, name, btnEvt->device,
                      (btnEvt->device ? btnEvt->device->name : "null")

                );
            g_free(name);
        }
        break;
        case GDK_MOTION_NOTIFY:
        {
            GdkEventMotion* btnMtn = reinterpret_cast<GdkEventMotion*>(event);
            if ( btnMtn->device ) {
                source = btnMtn->device->source;
                devName = btnMtn->device->name;
            }
            gchar* name = gtk_accelerator_name(0, static_cast<GdkModifierType>(btnMtn->state));
            keyVal.set_label(name);
            g_message("%d MOV    state:0x%08x         [%s] dev:%p [%s] %3.2f %3.2f %3.2f %3.2f %3.2f %3.2f", btnMtn->type, btnMtn->state,
                      name, btnMtn->device,
                      (btnMtn->device ? btnMtn->device->name : "null"),
                      ((btnMtn->device && btnMtn->axes && (btnMtn->device->num_axes > 0)) ? btnMtn->axes[0]:0),
                      ((btnMtn->device && btnMtn->axes && (btnMtn->device->num_axes > 1)) ? btnMtn->axes[1]:0),
                      ((btnMtn->device && btnMtn->axes && (btnMtn->device->num_axes > 2)) ? btnMtn->axes[2]:0),
                      ((btnMtn->device && btnMtn->axes && (btnMtn->device->num_axes > 3)) ? btnMtn->axes[3]:0),
                      ((btnMtn->device && btnMtn->axes && (btnMtn->device->num_axes > 4)) ? btnMtn->axes[4]:0),
                      ((btnMtn->device && btnMtn->axes && (btnMtn->device->num_axes > 5)) ? btnMtn->axes[5]:0)
                );
            g_free(name);
        }
        break;
        default:
            ;// nothing
    }


    if ( (lastSourceSeen != source) || (lastDevnameSeen != devName) ) {
        g_message("FLIPPIES  %d => %d", lastSourceSeen, source);
        switch (source) {
            case GDK_SOURCE_MOUSE:
            {
                testThumb.set(mousePix);
            }
            break;
            case GDK_SOURCE_CURSOR:
            {
                g_message("flip to cursor");
                testThumb.set(mousePix);
            }
            break;
            case GDK_SOURCE_PEN:
            {
                if (devName == "pad") {
                    g_message("flip to pad");
                    testThumb.set(sidebuttonsPix);
                } else {
                    g_message("flip to pen");
                    testThumb.set(tipPix);
                }
            }
            break;
            case GDK_SOURCE_ERASER:
            {
                g_message("flip to eraser");
                testThumb.set(eraserPix);
            }
            break;
            default:
                g_message("gurgle");
        }
        lastSourceSeen = source;
        lastDevnameSeen = devName;
    }

    return false;
}





GdkDeviceAxis padAxes[] = {{GDK_AXIS_X, 0.0, 0.0},
                           {GDK_AXIS_Y, 0.0, 0.0},
                           {GDK_AXIS_PRESSURE, 0.0, 1.0},
                           {GDK_AXIS_XTILT, -1.0, 1.0},
                           {GDK_AXIS_YTILT, -1.0, 1.0},
                           {GDK_AXIS_WHEEL, 0.0, 1.0}};
GdkDeviceKey padKeys[] = {{0, (GdkModifierType)0}, {0, (GdkModifierType)0}, {0, (GdkModifierType)0}, {0, (GdkModifierType)0},
                          {0, (GdkModifierType)0}, {0, (GdkModifierType)0}, {0, (GdkModifierType)0}, {0, (GdkModifierType)0}};

GdkDeviceAxis eraserAxes[] = {{GDK_AXIS_X, 0.0, 0.0},
                              {GDK_AXIS_Y, 0.0, 0.0},
                              {GDK_AXIS_PRESSURE, 0.0, 1.0},
                              {GDK_AXIS_XTILT, -1.0, 1.0},
                              {GDK_AXIS_YTILT, -1.0, 1.0},
                              {GDK_AXIS_WHEEL, 0.0, 1.0}};
GdkDeviceKey eraserKeys[] = {{0, (GdkModifierType)0}, {0, (GdkModifierType)0}, {0, (GdkModifierType)0}, {0, (GdkModifierType)0},
                             {0, (GdkModifierType)0}, {0, (GdkModifierType)0}, {0, (GdkModifierType)0}};

GdkDeviceAxis cursorAxes[] = {{GDK_AXIS_X, 0.0, 0.0},
                              {GDK_AXIS_Y, 0.0, 0.0},
                              {GDK_AXIS_PRESSURE, 0.0, 1.0},
                              {GDK_AXIS_XTILT, -1.0, 1.0},
                              {GDK_AXIS_YTILT, -1.0, 1.0},
                              {GDK_AXIS_WHEEL, 0.0, 1.0}};
GdkDeviceKey cursorKeys[] = {{0, (GdkModifierType)0}, {0, (GdkModifierType)0}, {0, (GdkModifierType)0}, {0, (GdkModifierType)0},
                             {0, (GdkModifierType)0}, {0, (GdkModifierType)0}, {0, (GdkModifierType)0}};

GdkDeviceAxis stylusAxes[] = {{GDK_AXIS_X, 0.0, 0.0},
                              {GDK_AXIS_Y, 0.0, 0.0},
                              {GDK_AXIS_PRESSURE, 0.0, 1.0},
                              {GDK_AXIS_XTILT, -1.0, 1.0},
                              {GDK_AXIS_YTILT, -1.0, 1.0},
                              {GDK_AXIS_WHEEL, 0.0, 1.0}};
GdkDeviceKey stylusKeys[] = {{0, (GdkModifierType)0}, {0, (GdkModifierType)0}, {0, (GdkModifierType)0}, {0, (GdkModifierType)0},
                             {0, (GdkModifierType)0}, {0, (GdkModifierType)0}, {0, (GdkModifierType)0}};


GdkDeviceAxis coreAxes[] = {{GDK_AXIS_X, 0.0, 0.0},
                            {GDK_AXIS_Y, 0.0, 0.0}};


static void createFakeList() {
    if ( !fakeList ) {
        fakeout[0].name = "pad";
        fakeout[0].source = GDK_SOURCE_PEN;
        fakeout[0].mode = GDK_MODE_SCREEN;
        fakeout[0].has_cursor = TRUE;
        fakeout[0].num_axes = 6;
        fakeout[0].axes = padAxes;
        fakeout[0].num_keys = 8;
        fakeout[0].keys = padKeys;

        fakeout[1].name = "eraser";
        fakeout[1].source = GDK_SOURCE_ERASER;
        fakeout[1].mode = GDK_MODE_SCREEN;
        fakeout[1].has_cursor = TRUE;
        fakeout[1].num_axes = 6;
        fakeout[1].axes = eraserAxes;
        fakeout[1].num_keys = 7;
        fakeout[1].keys = eraserKeys;

        fakeout[2].name = "cursor";
        fakeout[2].source = GDK_SOURCE_CURSOR;
        fakeout[2].mode = GDK_MODE_SCREEN;
        fakeout[2].has_cursor = TRUE;
        fakeout[2].num_axes = 6;
        fakeout[2].axes = cursorAxes;
        fakeout[2].num_keys = 7;
        fakeout[2].keys = cursorKeys;

        fakeout[3].name = "stylus";
        fakeout[3].source = GDK_SOURCE_PEN;
        fakeout[3].mode = GDK_MODE_SCREEN;
        fakeout[3].has_cursor = TRUE;
        fakeout[3].num_axes = 6;
        fakeout[3].axes = stylusAxes;
        fakeout[3].num_keys = 7;
        fakeout[3].keys = stylusKeys;

        fakeout[4].name = "Core Pointer";
        fakeout[4].source = GDK_SOURCE_MOUSE;
        fakeout[4].mode = GDK_MODE_SCREEN;
        fakeout[4].has_cursor = TRUE;
        fakeout[4].num_axes = 2;
        fakeout[4].axes = coreAxes;
        fakeout[4].num_keys = 0;
        fakeout[4].keys = NULL;

        for ( guint pos = 0; pos < G_N_ELEMENTS(fakeout); pos++) {
            fakeList = g_list_append(fakeList, &(fakeout[pos]));
        }
    }
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
