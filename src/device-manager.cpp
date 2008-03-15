
/*
 * Inkscape::DeviceManager - a view of input devices available.
 *
 * Copyright 2006  Jon A. Cruz  <jon@joncruz.org>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>

#include "device-manager.h"


static void createFakeList();
GdkDevice fakeout[5];
static GList* fakeList = 0;


namespace Inkscape {

InputDevice::InputDevice() {}
InputDevice::~InputDevice() {}

class InputDeviceImpl : public InputDevice {
public:
    virtual Glib::ustring getName() const {return Glib::ustring(device->name);}
    virtual Gdk::InputSource getSource() const {return static_cast<Gdk::InputSource>(device->source);}
    virtual Gdk::InputMode getMode() const {return static_cast<Gdk::InputMode>(device->mode);}
    virtual bool hasCursor() const {return device->has_cursor;}
    virtual gint getNumAxes() const {return device->num_axes;}
    virtual gint getNumKeys() const {return device->num_keys;}

    InputDeviceImpl(GdkDevice* device);
    virtual ~InputDeviceImpl() {}

private:
    InputDeviceImpl(InputDeviceImpl const &); // no copy
    void operator=(InputDeviceImpl const &); // no assign

    GdkDevice* device;
};

InputDeviceImpl::InputDeviceImpl(GdkDevice* device)
    : InputDevice(),
      device(device)
{
}








class DeviceManagerImpl : public DeviceManager {
public:
    DeviceManagerImpl();
    virtual std::list<InputDevice const *> getDevices();

protected:
    std::list<InputDeviceImpl*> devices;
};

DeviceManagerImpl::DeviceManagerImpl() :
    DeviceManager(),
    devices()
{
    GList* devList = gdk_devices_list();

    if ( !fakeList ) {
        createFakeList();
    }
    devList = fakeList;

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


static DeviceManagerImpl* theInstance = 0;

DeviceManager::DeviceManager() {
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
