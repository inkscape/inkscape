
/*
 * Inkscape::DeviceManager - a view of input devices available.
 *
 * Copyright 2006  Jon A. Cruz  <jon@joncruz.org>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <map>

#include "device-manager.h"


static void createFakeList();
GdkDevice fakeout[5];
static GList* fakeList = 0;


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


InputDevice::InputDevice()
    : Glib::Object()
{}

InputDevice::~InputDevice() {}

class InputDeviceImpl : public InputDevice {
public:
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


    InputDeviceImpl(GdkDevice* device);
    virtual ~InputDeviceImpl() {}

private:
    InputDeviceImpl(InputDeviceImpl const &); // no copy
    void operator=(InputDeviceImpl const &); // no assign

    GdkDevice* device;
    Glib::ustring id;
    Glib::ustring name;
    Gdk::InputSource source;
    Glib::ustring link;
    guint liveAxes;
    guint liveButtons;
};

class IdMatcher : public std::unary_function<InputDeviceImpl*, bool> {
public:
    IdMatcher(Glib::ustring const& target):target(target) {}
    bool operator ()(InputDeviceImpl* dev) {return dev && (target == dev->getId());}

private:
    Glib::ustring const& target;
};

class LinkMatcher : public std::unary_function<InputDeviceImpl*, bool> {
public:
    LinkMatcher(Glib::ustring const& target):target(target) {}
    bool operator ()(InputDeviceImpl* dev) {return dev && (target == dev->getLink());}

private:
    Glib::ustring const& target;
};

InputDeviceImpl::InputDeviceImpl(GdkDevice* device)
    : InputDevice(),
      device(device),
      id(),
      name(device->name ? device->name : ""),
      source(static_cast<Gdk::InputSource>(device->source)),
      link(),
      liveAxes(0),
      liveButtons(0)
{
    switch ( source ) {
        case Gdk::SOURCE_MOUSE:
            id = "M:";
            break;
        case Gdk::SOURCE_CURSOR:
            id = "C:";
            break;
        case Gdk::SOURCE_PEN:
            id = "P:";
            break;
        case Gdk::SOURCE_ERASER:
            id = "E:";
            break;
        default:
            id = "?:";
    }
    id += name;
}








class DeviceManagerImpl : public DeviceManager {
public:
    DeviceManagerImpl();
    virtual std::list<InputDevice const *> getDevices();
    virtual sigc::signal<void, const Glib::RefPtr<InputDevice>& > signalDeviceChanged();
    virtual sigc::signal<void, const Glib::RefPtr<InputDevice>& > signalAxesChanged();
    virtual sigc::signal<void, const Glib::RefPtr<InputDevice>& > signalButtonsChanged();
    virtual sigc::signal<void, const Glib::RefPtr<InputDevice>& > signalLinkChanged();

    virtual void addAxis(Glib::ustring const & id, gint axis);
    virtual void addButton(Glib::ustring const & id, gint button);
    virtual void setLinkedTo(Glib::ustring const & id, Glib::ustring const& link);

protected:
    std::list<InputDeviceImpl*> devices;
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
//     devList = fakeList;

    for ( GList* curr = devList; curr; curr = g_list_next(curr) ) {
        GdkDevice* dev = reinterpret_cast<GdkDevice*>(curr->data);
        if ( dev ) {
//             g_message("device: name[%s] source[0x%x] mode[0x%x] cursor[%s] axis count[%d] key count[%d]", dev->name, dev->source, dev->mode,
//                       dev->has_cursor?"Yes":"no", dev->num_axes, dev->num_keys);

            InputDeviceImpl* device = new InputDeviceImpl(dev);
            devices.push_back(device);
        }
    }
}

std::list<InputDevice const *> DeviceManagerImpl::getDevices()
{
    std::list<InputDevice const *> tmp;
    for ( std::list<InputDeviceImpl*>::const_iterator it = devices.begin(); it != devices.end(); ++it ) {
        tmp.push_back(*it);
    }
    return tmp;
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
        std::list<InputDeviceImpl*>::iterator it = std::find_if(devices.begin(), devices.end(), IdMatcher(id));
        if ( it != devices.end() ) {
            gint mask = bitVals[axis];
            if ( (mask & (*it)->getLiveAxes()) == 0 ) {
                (*it)->setLiveAxes((*it)->getLiveAxes() | mask);

                // Only signal if a new axis was added
                (*it)->reference();
                signalAxesChangedPriv.emit(Glib::RefPtr<InputDevice>(*it));
            }
        }
    }
}

void DeviceManagerImpl::addButton(Glib::ustring const & id, gint button)
{
    if ( button >= 0 && button < static_cast<gint>(bitVals.size()) ) {
        std::list<InputDeviceImpl*>::iterator it = std::find_if(devices.begin(), devices.end(), IdMatcher(id));
        if ( it != devices.end() ) {
            gint mask = bitVals[button];
            if ( (mask & (*it)->getLiveButtons()) == 0 ) {
                (*it)->setLiveButtons((*it)->getLiveButtons() | mask);

                // Only signal if a new button was added
                (*it)->reference();
                signalButtonsChangedPriv.emit(Glib::RefPtr<InputDevice>(*it));
            }
        }
    }
}

void DeviceManagerImpl::setLinkedTo(Glib::ustring const & id, Glib::ustring const& link)
{
    std::list<InputDeviceImpl*>::iterator it = std::find_if(devices.begin(), devices.end(), IdMatcher(id));
    if ( it != devices.end() ) {
        InputDeviceImpl* dev = *it;


        InputDeviceImpl* targetDev = 0;
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
            std::list<InputDeviceImpl*> changedItems;

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

            for ( std::list<InputDeviceImpl*>::const_iterator iter = changedItems.begin(); iter != changedItems.end(); ++iter ) {
                (*iter)->reference();
                signalLinkChangedPriv.emit(Glib::RefPtr<InputDevice>(*iter));
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
