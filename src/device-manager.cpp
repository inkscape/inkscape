
/*
 * Inkscape::DeviceManager - a view of input devices available.
 *
 * Copyright 2010  Jon A. Cruz  <jon@joncruz.org>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <map>
#include <set>
#include <gtk/gtkaccelgroup.h>

#include "device-manager.h"
#include "preferences.h"

#define noDEBUG_VERBOSE 1

static void createFakeList();
GdkDevice fakeout[5];
static GList* fakeList = 0;

static bool isValidDevice(GdkDevice *device)
{
    bool valid = true;
    for (size_t i = 0; (i < G_N_ELEMENTS(fakeout)) && valid; i++) {
        valid = (device != &fakeout[i]);
    }
    return valid;
}

namespace Inkscape {

using std::pair;

static pair<gint, gint> vals[] = {
    pair<gint, gint>(0, 1), pair<gint, gint>(1, 1 << 1), pair<gint, gint>(2, 1 << 2), pair<gint, gint>(3, 1 << 3),
    pair<gint, gint>(4, 1 << 4), pair<gint, gint>(5, 1 << 5), pair<gint, gint>(6, 1 << 6), pair<gint, gint>(7, 1 << 7),
    pair<gint, gint>(8, 1 << 8), pair<gint, gint>(9, 1 << 9), pair<gint, gint>(10, 1 << 10), pair<gint, gint>(11, 1 << 11),
    pair<gint, gint>(12, 1 << 12), pair<gint, gint>(13, 1 << 13), pair<gint, gint>(14, 1 << 14), pair<gint, gint>(15, 1 << 15),
    pair<gint, gint>(16, 1 << 16), pair<gint, gint>(17, 1 << 17), pair<gint, gint>(18, 1 << 18), pair<gint, gint>(19, 1 << 19),
    pair<gint, gint>(20, 1 << 20), pair<gint, gint>(21, 1 << 21), pair<gint, gint>(22, 1 << 22), pair<gint, gint>(23, 1 << 23)
};
static std::map<gint, gint> bitVals(vals, &vals[G_N_ELEMENTS(vals)]);


static const int RUNAWAY_MAX = 1000;

static Glib::ustring getBaseDeviceName(Gdk::InputSource source)
{
    Glib::ustring name;
    switch (source) {
        case GDK_SOURCE_MOUSE:
            name ="pointer";
            break;
        case GDK_SOURCE_PEN:
            name ="pen";
            break;
        case GDK_SOURCE_ERASER:
            name ="eraser";
            break;
        case GDK_SOURCE_CURSOR:
            name ="cursor";
            break;
        default:
            name = "tablet";
    }
    return name;
}

static std::map<Glib::ustring, Gdk::AxisUse> &getStringToAxis()
{
    static bool init = false;
    static std::map<Glib::ustring, Gdk::AxisUse> mapping;
    if (!init) {
        init = true;
        mapping["ignore"]   = Gdk::AXIS_IGNORE;
        mapping["x"]        = Gdk::AXIS_X;
        mapping["y"]        = Gdk::AXIS_Y;
        mapping["pressure"] = Gdk::AXIS_PRESSURE;
        mapping["xtilt"]    = Gdk::AXIS_XTILT;
        mapping["ytilt"]    = Gdk::AXIS_YTILT;
        mapping["wheel"]    = Gdk::AXIS_WHEEL;
    }
    return mapping;
}

std::map<Gdk::AxisUse, Glib::ustring> &getAxisToString()
{
    static bool init = false;
    static std::map<Gdk::AxisUse, Glib::ustring> mapping;
    if (!init) {
        init = true;
        for (std::map<Glib::ustring, Gdk::AxisUse>::iterator it = getStringToAxis().begin(); it != getStringToAxis().end(); ++it) {
            mapping.insert(std::make_pair(it->second, it->first));
        }
    }
    return mapping;
}

static std::map<Glib::ustring, Gdk::InputMode> &getStringToMode()
{
    static bool init = false;
    static std::map<Glib::ustring, Gdk::InputMode> mapping;
    if (!init) {
        init = true;
        mapping["disabled"] = Gdk::MODE_DISABLED;
        mapping["screen"]   = Gdk::MODE_SCREEN;
        mapping["window"]   = Gdk::MODE_WINDOW;
    }
    return mapping;
}

std::map<Gdk::InputMode, Glib::ustring> &getModeToString()
{
    static bool init = false;
    static std::map<Gdk::InputMode, Glib::ustring> mapping;
    if (!init) {
        init = true;
        for (std::map<Glib::ustring, Gdk::InputMode>::iterator it = getStringToMode().begin(); it != getStringToMode().end(); ++it) {
            mapping.insert(std::make_pair(it->second, it->first));
        }
    }
    return mapping;
}



InputDevice::InputDevice()
    : Glib::Object()
{}

InputDevice::~InputDevice() {}

class InputDeviceImpl : public InputDevice {
public:
    InputDeviceImpl(GdkDevice* device, std::set<Glib::ustring> &knownIDs);
    virtual ~InputDeviceImpl() {}

    virtual Glib::ustring getId() const {return id;}
    virtual Glib::ustring getName() const {return name;}
    virtual Gdk::InputSource getSource() const {return source;}
    virtual Gdk::InputMode getMode() const {return static_cast<Gdk::InputMode>(device->mode);}
    virtual bool hasCursor() const {return device->has_cursor;}
    virtual gint getNumAxes() const {return device->num_axes;}
    virtual gint getNumKeys() const {return device->num_keys;}
    virtual Glib::ustring getLink() const {return link;}
    virtual void setLink( Glib::ustring const& link ) {this->link = link;}
    virtual gint getLiveAxes() const {return liveAxes;}
    virtual void setLiveAxes(gint axes) {liveAxes = axes;}
    virtual gint getLiveButtons() const {return liveButtons;}
    virtual void setLiveButtons(gint buttons) {liveButtons = buttons;}

    // internal methods not on public superclass:
    virtual GdkDevice *getDevice() { return device; }

private:
    InputDeviceImpl(InputDeviceImpl const &); // no copy
    void operator=(InputDeviceImpl const &); // no assign

    static Glib::ustring createId(Glib::ustring const &id, Gdk::InputSource source, std::set<Glib::ustring> &knownIDs);

    GdkDevice* device;
    Glib::ustring id;
    Glib::ustring name;
    Gdk::InputSource source;
    Glib::ustring link;
    guint liveAxes;
    guint liveButtons;
};

class IdMatcher : public std::unary_function<Glib::RefPtr<InputDeviceImpl>&, bool> {
public:
    IdMatcher(Glib::ustring const& target):target(target) {}
    bool operator ()(Glib::RefPtr<InputDeviceImpl>& dev) {return dev && (target == dev->getId());}

private:
    Glib::ustring const& target;
};

class LinkMatcher : public std::unary_function<Glib::RefPtr<InputDeviceImpl>&, bool> {
public:
    LinkMatcher(Glib::ustring const& target):target(target) {}
    bool operator ()(Glib::RefPtr<InputDeviceImpl>& dev) {return dev && (target == dev->getLink());}

private:
    Glib::ustring const& target;
};

InputDeviceImpl::InputDeviceImpl(GdkDevice* device, std::set<Glib::ustring> &knownIDs)
    : InputDevice(),
      device(device),
      id(),
      name(device->name ? device->name : ""),
      source(static_cast<Gdk::InputSource>(device->source)),
      link(),
      liveAxes(0),
      liveButtons(0)
{
    id = createId(name, source, knownIDs);
}


Glib::ustring InputDeviceImpl::createId(Glib::ustring const &id,
                                        Gdk::InputSource source,
                                        std::set<Glib::ustring> &knownIDs)
{
    // Start with only allowing printable ASCII. Check later for more refinements.
    bool badName = id.empty() || !id.is_ascii();
    for (Glib::ustring::const_iterator it = id.begin(); (it != id.end()) && !badName; ++it) {
        badName = *it < 0x20;
    }

    Glib::ustring base;
    switch ( source ) {
        case Gdk::SOURCE_MOUSE:
            base = "M:";
            break;
        case Gdk::SOURCE_CURSOR:
            base = "C:";
            break;
        case Gdk::SOURCE_PEN:
            base = "P:";
            break;
        case Gdk::SOURCE_ERASER:
            base = "E:";
            break;
        default:
            base = "?:";
    }

    if (badName) {
        base += getBaseDeviceName(source);
    } else {
        base += id;
    }

    // now ensure that all IDs become unique in a session.
    int num = 1;
    Glib::ustring result = base;
    while ((knownIDs.find(result) != knownIDs.end()) && (num < RUNAWAY_MAX)) {
        result = Glib::ustring::compose("%1%2", base, ++num);
    }

    knownIDs.insert(result);
    return result;
}





class DeviceManagerImpl : public DeviceManager {
public:
    DeviceManagerImpl();

    virtual void loadConfig();
    virtual void saveConfig();

    virtual std::list<Glib::RefPtr<InputDevice const> > getDevices();

    virtual sigc::signal<void, const Glib::RefPtr<InputDevice>& > signalDeviceChanged();
    virtual sigc::signal<void, const Glib::RefPtr<InputDevice>& > signalAxesChanged();
    virtual sigc::signal<void, const Glib::RefPtr<InputDevice>& > signalButtonsChanged();
    virtual sigc::signal<void, const Glib::RefPtr<InputDevice>& > signalLinkChanged();

    virtual void addAxis(Glib::ustring const & id, gint axis);
    virtual void addButton(Glib::ustring const & id, gint button);
    virtual void setLinkedTo(Glib::ustring const & id, Glib::ustring const& link);

    virtual void setMode( Glib::ustring const & id, Gdk::InputMode mode );
    virtual void setAxisUse( Glib::ustring const & id, guint index, Gdk::AxisUse use );
    virtual void setKey( Glib::ustring const & id, guint index, guint keyval, Gdk::ModifierType mods );

protected:
    std::list<Glib::RefPtr<InputDeviceImpl> > devices;

    sigc::signal<void, const Glib::RefPtr<InputDevice>& > signalDeviceChangedPriv;
    sigc::signal<void, const Glib::RefPtr<InputDevice>& > signalAxesChangedPriv;
    sigc::signal<void, const Glib::RefPtr<InputDevice>& > signalButtonsChangedPriv;
    sigc::signal<void, const Glib::RefPtr<InputDevice>& > signalLinkChangedPriv;
};


DeviceManagerImpl::DeviceManagerImpl() :
    DeviceManager(),
    devices()
{
    GList* devList = gdk_devices_list();

    if ( !fakeList ) {
        createFakeList();
    }
    //devList = fakeList;

    std::set<Glib::ustring> knownIDs;

    for ( GList* curr = devList; curr; curr = g_list_next(curr) ) {
        GdkDevice* dev = reinterpret_cast<GdkDevice*>(curr->data);
        if ( dev ) {
#if DEBUG_VERBOSE
            g_message("device: name[%s] source[0x%x] mode[0x%x] cursor[%s] axis count[%d] key count[%d]", dev->name, dev->source, dev->mode,
                      dev->has_cursor?"Yes":"no", dev->num_axes, dev->num_keys);
#endif

            InputDeviceImpl* device = new InputDeviceImpl(dev, knownIDs);
            device->reference();
            devices.push_back(Glib::RefPtr<InputDeviceImpl>(device));
        }
    }
}

void DeviceManagerImpl::loadConfig()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    for (std::list<Glib::RefPtr<InputDeviceImpl> >::iterator it = devices.begin(); it != devices.end(); ++it) {
        if ((*it)->getSource() != Gdk::SOURCE_MOUSE) {
            Glib::ustring path = "/devices/" + (*it)->getId();

            Gdk::InputMode mode = Gdk::MODE_DISABLED;
            Glib::ustring val = prefs->getString(path + "/mode");
            if (getStringToMode().find(val) != getStringToMode().end()) {
                mode = getStringToMode()[val];
            }
            if ((*it)->getMode() != mode) {
                setMode( (*it)->getId(), mode );
            }

            //

            val = prefs->getString(path + "/axes");
            if (!val.empty()) {
                std::vector<Glib::ustring> parts = Glib::Regex::split_simple(";", val);
                for (size_t i = 0; i < parts.size(); ++i) {
                    Glib::ustring name = parts[i];
                    if (getStringToAxis().find(name) != getStringToAxis().end()) {
                        Gdk::AxisUse use = getStringToAxis()[name];
                        setAxisUse( (*it)->getId(), i, use );
                    }
                }
            }

            val = prefs->getString(path + "/keys");
            if (!val.empty()) {
                std::vector<Glib::ustring> parts = Glib::Regex::split_simple(";", val);
                for (size_t i = 0; i < parts.size(); ++i) {
                    Glib::ustring keyStr = parts[i];
                    if (!keyStr.empty()) {
                        guint key = 0;
                        GdkModifierType mods = static_cast<GdkModifierType>(0);
                        gtk_accelerator_parse( keyStr.c_str(), &key, &mods );
                        setKey( (*it)->getId(), i, key, static_cast<Gdk::ModifierType>(mods) );
                    }
                }
            }
        }
    }
}

void DeviceManagerImpl::saveConfig()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    for (std::list<Glib::RefPtr<InputDeviceImpl> >::iterator it = devices.begin(); it != devices.end(); ++it) {
        if ((*it)->getSource() != Gdk::SOURCE_MOUSE) {
            Glib::ustring path = "/devices/" + (*it)->getId();

            prefs->setString( path + "/mode", getModeToString()[(*it)->getMode()].c_str() );

            Glib::ustring tmp;
            for (gint i = 0; i < (*it)->getNumAxes(); ++i) {
                if (i > 0) {
                    tmp += ";";
                }
                tmp += getAxisToString()[static_cast<Gdk::AxisUse>((*it)->getDevice()->axes[i].use)];
            }
            prefs->setString( path + "/axes", tmp );

            tmp = "";
            for (gint i = 0; i < (*it)->getNumKeys(); ++i) {
                if (i > 0) {
                    tmp += ";";
                }
                tmp += gtk_accelerator_name((*it)->getDevice()->keys[i].keyval, (*it)->getDevice()->keys[i].modifiers);
            }
            prefs->setString( path + "/keys", tmp );
        }
    }
}

std::list<Glib::RefPtr<InputDevice const> > DeviceManagerImpl::getDevices()
{
    std::list<Glib::RefPtr<InputDevice const> > tmp;
    for ( std::list<Glib::RefPtr<InputDeviceImpl> >::const_iterator it = devices.begin(); it != devices.end(); ++it ) {
        tmp.push_back(*it);
    }
    return tmp;
}

void DeviceManagerImpl::setMode( Glib::ustring const & id, Gdk::InputMode mode )
{
    std::list<Glib::RefPtr<InputDeviceImpl> >::iterator it = std::find_if(devices.begin(), devices.end(), IdMatcher(id));
    if ( it != devices.end() ) {
        if (isValidDevice((*it)->getDevice())) {
            bool success = gdk_device_set_mode((*it)->getDevice(), static_cast<GdkInputMode>(mode));
            if (success) {
                //(*it)->setMode(mode);
                signalDeviceChangedPriv.emit(*it);
            } else {
                g_warning("Unable to set mode on extended input device [%s]", (*it)->getId().c_str());
            }
        }
    }
}

void DeviceManagerImpl::setAxisUse( Glib::ustring const & id, guint index, Gdk::AxisUse use )
{
    std::list<Glib::RefPtr<InputDeviceImpl> >::iterator it = std::find_if(devices.begin(), devices.end(), IdMatcher(id));
    if ( it != devices.end() ) {
        if (isValidDevice((*it)->getDevice())) {
            gdk_device_set_axis_use((*it)->getDevice(), index, static_cast<GdkAxisUse>(use));
            signalDeviceChangedPriv.emit(*it);
        }
    }
}

void DeviceManagerImpl::setKey( Glib::ustring const & id, guint index, guint keyval, Gdk::ModifierType mods )
{
    //static void setDeviceKey( GdkDevice* device, guint index, guint keyval, GdkModifierType modifiers )
    //

    std::list<Glib::RefPtr<InputDeviceImpl> >::iterator it = std::find_if(devices.begin(), devices.end(), IdMatcher(id));
    if ( it != devices.end() ) {
        if (isValidDevice((*it)->getDevice())) {
            gdk_device_set_key((*it)->getDevice(), index, keyval, static_cast<GdkModifierType>(mods));
            signalDeviceChangedPriv.emit(*it);
        }
    }
}

sigc::signal<void, const Glib::RefPtr<InputDevice>& > DeviceManagerImpl::signalDeviceChanged()
{
    return signalDeviceChangedPriv;
}

sigc::signal<void, const Glib::RefPtr<InputDevice>& > DeviceManagerImpl::signalAxesChanged()
{
    return signalAxesChangedPriv;
}

sigc::signal<void, const Glib::RefPtr<InputDevice>& > DeviceManagerImpl::signalButtonsChanged()
{
    return signalButtonsChangedPriv;
}

sigc::signal<void, const Glib::RefPtr<InputDevice>& > DeviceManagerImpl::signalLinkChanged()
{
    return signalLinkChangedPriv;
}

void DeviceManagerImpl::addAxis(Glib::ustring const & id, gint axis)
{
    if ( axis >= 0 && axis < static_cast<gint>(bitVals.size()) ) {
        std::list<Glib::RefPtr<InputDeviceImpl> >::iterator it = std::find_if(devices.begin(), devices.end(), IdMatcher(id));
        if ( it != devices.end() ) {
            gint mask = bitVals[axis];
            if ( (mask & (*it)->getLiveAxes()) == 0 ) {
                (*it)->setLiveAxes((*it)->getLiveAxes() | mask);

                // Only signal if a new axis was added
                (*it)->reference();
                signalAxesChangedPriv.emit(*it);
            }
        }
    }
}

void DeviceManagerImpl::addButton(Glib::ustring const & id, gint button)
{
    if ( button >= 0 && button < static_cast<gint>(bitVals.size()) ) {
        std::list<Glib::RefPtr<InputDeviceImpl> >::iterator it = std::find_if(devices.begin(), devices.end(), IdMatcher(id));
        if ( it != devices.end() ) {
            gint mask = bitVals[button];
            if ( (mask & (*it)->getLiveButtons()) == 0 ) {
                (*it)->setLiveButtons((*it)->getLiveButtons() | mask);

                // Only signal if a new button was added
                (*it)->reference();
                signalButtonsChangedPriv.emit(*it);
            }
        }
    }
}

void DeviceManagerImpl::setLinkedTo(Glib::ustring const & id, Glib::ustring const& link)
{
    std::list<Glib::RefPtr<InputDeviceImpl> >::iterator it = std::find_if(devices.begin(), devices.end(), IdMatcher(id));
    if ( it != devices.end() ) {
        Glib::RefPtr<InputDeviceImpl> dev = *it;

        Glib::RefPtr<InputDeviceImpl> targetDev;
        if ( !link.empty() ) {
            // Need to be sure the target of the link exists
            it = std::find_if(devices.begin(), devices.end(), IdMatcher(link));
            if ( it != devices.end() ) {
                targetDev = *it;
            }
        }


        if ( (link.empty() && !dev->getLink().empty())
             || (targetDev && (targetDev->getLink() != id)) ) {
            // only muck about if they aren't already linked
            std::list<Glib::RefPtr<InputDeviceImpl> > changedItems;

            if ( targetDev ) {
            // Is something else already using that link?
                it = std::find_if(devices.begin(), devices.end(), LinkMatcher(link));
                if ( it != devices.end() ) {
                    (*it)->setLink("");
                    changedItems.push_back(*it);
                }
            }
            it = std::find_if(devices.begin(), devices.end(), LinkMatcher(id));
            if ( it != devices.end() ) {
                (*it)->setLink("");
                changedItems.push_back(*it);
            }
            if ( targetDev ) {
                targetDev->setLink(id);
                changedItems.push_back(targetDev);
            }
            dev->setLink(link);
            changedItems.push_back(dev);

            for ( std::list<Glib::RefPtr<InputDeviceImpl> >::const_iterator iter = changedItems.begin(); iter != changedItems.end(); ++iter ) {
                (*iter)->reference();
                signalLinkChangedPriv.emit(*iter);
            }
        }
    }
}






static DeviceManagerImpl* theInstance = 0;

DeviceManager::DeviceManager()
    : Glib::Object()
{
}

DeviceManager::~DeviceManager() {
}

DeviceManager& DeviceManager::getManager() {
    if ( !theInstance ) {
        theInstance = new DeviceManagerImpl();
    }

    return *theInstance;
}

} // namespace Inkscape






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
        fakeout[0].name = g_strdup("pad");
        fakeout[0].source = GDK_SOURCE_PEN;
        fakeout[0].mode = GDK_MODE_SCREEN;
        fakeout[0].has_cursor = TRUE;
        fakeout[0].num_axes = 6;
        fakeout[0].axes = padAxes;
        fakeout[0].num_keys = 8;
        fakeout[0].keys = padKeys;

        fakeout[1].name = g_strdup("eraser");
        fakeout[1].source = GDK_SOURCE_ERASER;
        fakeout[1].mode = GDK_MODE_SCREEN;
        fakeout[1].has_cursor = TRUE;
        fakeout[1].num_axes = 6;
        fakeout[1].axes = eraserAxes;
        fakeout[1].num_keys = 7;
        fakeout[1].keys = eraserKeys;

        fakeout[2].name = g_strdup("cursor");
        fakeout[2].source = GDK_SOURCE_CURSOR;
        fakeout[2].mode = GDK_MODE_SCREEN;
        fakeout[2].has_cursor = TRUE;
        fakeout[2].num_axes = 6;
        fakeout[2].axes = cursorAxes;
        fakeout[2].num_keys = 7;
        fakeout[2].keys = cursorKeys;

        fakeout[3].name = g_strdup("stylus");
        fakeout[3].source = GDK_SOURCE_PEN;
        fakeout[3].mode = GDK_MODE_SCREEN;
        fakeout[3].has_cursor = TRUE;
        fakeout[3].num_axes = 6;
        fakeout[3].axes = stylusAxes;
        fakeout[3].num_keys = 7;
        fakeout[3].keys = stylusKeys;

// try to find the first *real* core pointer
        GList* devList = gdk_devices_list();
        while ( devList && devList->data && (((GdkDevice*)devList->data)->source != GDK_SOURCE_MOUSE) ) {
            devList = g_list_next(devList);
        }
        if ( devList && devList->data ) {
            fakeout[4] = *((GdkDevice*)devList->data);
        } else {
            fakeout[4].name = g_strdup("Core Pointer");
            fakeout[4].source = GDK_SOURCE_MOUSE;
            fakeout[4].mode = GDK_MODE_SCREEN;
            fakeout[4].has_cursor = TRUE;
            fakeout[4].num_axes = 2;
            fakeout[4].axes = coreAxes;
            fakeout[4].num_keys = 0;
            fakeout[4].keys = NULL;
        }

        for ( guint pos = 0; pos < G_N_ELEMENTS(fakeout); pos++) {
            fakeList = g_list_append(fakeList, &(fakeout[pos]));
        }
    }
}


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
