/*
 * Inkscape::DeviceManager - a view of input devices available.
 *
 * Copyright 2006  Jon A. Cruz  <jon@joncruz.org>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_DEVICE_MANAGER_H
#define SEEN_INKSCAPE_DEVICE_MANAGER_H


#include <list>
#include <glibmm/ustring.h>
#include <gdkmm/device.h>

namespace Inkscape {

class InputDevice : public Glib::Object {
public:
    virtual Glib::ustring getId() const = 0;
    virtual Glib::ustring getName() const = 0;
    virtual Gdk::InputSource getSource() const = 0;
    virtual Gdk::InputMode getMode() const = 0;
    virtual bool hasCursor() const = 0;
    virtual gint getNumAxes() const = 0;
    virtual gint getNumKeys() const = 0;
    virtual Glib::ustring getLink() const = 0;
    virtual gint getLiveButtons() const = 0;

protected:
    InputDevice();
    virtual ~InputDevice();

private:
    InputDevice(InputDevice const &); // no copy
    void operator=(InputDevice const &); // no assign
};

class DeviceManager : public Glib::Object {
public:
    static DeviceManager& getManager();

    virtual std::list<InputDevice const *> getDevices() = 0;
    virtual sigc::signal<void, const Glib::RefPtr<InputDevice>& > signalDeviceChanged() = 0;
    virtual sigc::signal<void, const Glib::RefPtr<InputDevice>& > signalButtonsChanged() = 0;
    virtual sigc::signal<void, const Glib::RefPtr<InputDevice>& > signalLinkChanged() = 0;

    virtual void addButton(Glib::ustring const & id, gint button) = 0;
    virtual void setLinkedTo(Glib::ustring const & id, Glib::ustring const& link) = 0;

protected:
    DeviceManager();
    virtual ~DeviceManager();

private:
    DeviceManager(DeviceManager const &); // no copy
    void operator=(DeviceManager const &); // no assign
};



} // namespace Inkscape

#endif // SEEN_INKSCAPE_DEVICE_MANAGER_H

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
