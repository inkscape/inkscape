/*
 * Central facade for accessing and managing on-canvas controls.
 *
 * Author:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright 2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "control-manager.h"

#include <algorithm>
#include <glib.h>
#include <glib-object.h>

#include "display/sodipodi-ctrl.h" // for SP_TYPE_CTRL
#include "display/sp-canvas-item.h"
#include "display/sp-ctrlline.h"
#include "display/sp-ctrlpoint.h"
#include "preferences.h"

namespace {

std::map<Inkscape::ControlType, std::vector<int> > sizeTable;

} // namespace

#define FILL_COLOR_NORMAL 0xffffff7f
#define FILL_COLOR_MOUSEOVER 0xff0000ff

// Default color for line:
#define LINE_COLOR_PRIMARY 0x0000ff7f
#define LINE_COLOR_SECONDARY 0xff00007f
#define LINE_COLOR_TERTIARY 0xffff007f

namespace Inkscape {

class ControlManagerImpl
{
public:
    ControlManagerImpl();

    ~ControlManagerImpl() {}

    SPCanvasItem *createControl(SPCanvasGroup *parent, ControlType type);

    void setControlSize(int size, bool force = false);

    void track(SPCanvasItem *anchor);

    sigc::connection connectCtrlSizeChanged(const sigc::slot<void> &slot);

    void updateItem(SPCanvasItem *item);

private:
    static void thingFinalized(gpointer data, GObject *wasObj);

    void thingFinalized(GObject *wasObj);

    class PrefListener : public Inkscape::Preferences::Observer
    {
    public:
        PrefListener(ControlManagerImpl &manager) : Inkscape::Preferences::Observer("/options/grabsize/value"), _mgr(manager) {}
        virtual ~PrefListener() {}

        virtual void notify(Inkscape::Preferences::Entry const &val) {
            int size = val.getIntLimited(3, 1, 7);
            _mgr.setControlSize(size);
        }

        ControlManagerImpl &_mgr;
    };

    sigc::signal<void> _sizeChangedSignal;
    PrefListener _prefHook;
    int _size;
    std::vector<SPCanvasItem *> _itemList;
    std::map<Inkscape::ControlType, std::vector<int> > _sizeTable;
};

ControlManagerImpl::ControlManagerImpl() :
    _sizeChangedSignal(),
    _prefHook(*this),
    _size(3),
    _itemList(),
    _sizeTable()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->addObserver(_prefHook);

    _size = prefs->getIntLimited("/options/grabsize/value", 3, 1, 7);

    {
        int sizes[] = {8, 8, 8, 8, 8, 8, 8};
        _sizeTable[CTRL_TYPE_UNKNOWN] = std::vector<int>(sizes, sizes + (sizeof(sizes) / sizeof(sizes[0])));
    }
    {
        int sizes[] = {2, 4, 6, 8, 10, 12, 14};
        _sizeTable[CTRL_TYPE_ANCHOR] = std::vector<int>(sizes, sizes + (sizeof(sizes) / sizeof(sizes[0])));
    }
    {
        int sizes[] = {2, 4, 7, 8, 9, 10, 12};
        _sizeTable[CTRL_TYPE_ADJ_HANDLE] = std::vector<int>(sizes, sizes + (sizeof(sizes) / sizeof(sizes[0])));
    }
    {
        int sizes[] = {4, 6, 8, 10, 12, 14, 16};
        _sizeTable[CTRL_TYPE_POINT] = std::vector<int>(sizes, sizes + (sizeof(sizes) / sizeof(sizes[0])));
        _sizeTable[CTRL_TYPE_ROTATE] = std::vector<int>(sizes, sizes + (sizeof(sizes) / sizeof(sizes[0])));
        _sizeTable[CTRL_TYPE_SIZER] = std::vector<int>(sizes, sizes + (sizeof(sizes) / sizeof(sizes[0])));
        _sizeTable[CTRL_TYPE_SHAPER] = std::vector<int>(sizes, sizes + (sizeof(sizes) / sizeof(sizes[0])));
    }
    {
        int sizes[] = {2, 3, 4, 7, 8, 9, 10};
        _sizeTable[CTRL_TYPE_ORIGIN] = std::vector<int>(sizes, sizes + (sizeof(sizes) / sizeof(sizes[0])));
    }
    {
        int sizes[] = {1, 1, 1, 1, 1, 1, 1};
        _sizeTable[CTRL_TYPE_INVISIPOINT] = std::vector<int>(sizes, sizes + (sizeof(sizes) / sizeof(sizes[0])));
    }
}


void ControlManagerImpl::setControlSize(int size, bool force)
{
    if ((size < 1) || (size > 7)) {
        g_warning("Illegal logical size set: %d", size);
    } else if (force || (size != _size)) {
        _size = size;

        for (std::vector<SPCanvasItem *>::iterator it = _itemList.begin(); it != _itemList.end(); ++it)
        {
            if (*it) {
                updateItem(*it);
            }
        }

        _sizeChangedSignal.emit();
    }
}

SPCanvasItem *ControlManagerImpl::createControl(SPCanvasGroup *parent, ControlType type)
{
    SPCanvasItem *item = 0;
    double targetSize = _sizeTable[type][_size - 1];
    switch (type)
    {
        case CTRL_TYPE_ADJ_HANDLE:
            item = sp_canvas_item_new(parent, SP_TYPE_CTRL,
                                      "shape", SP_CTRL_SHAPE_CIRCLE,
                                      "size", targetSize,
                                      "filled", 0,
                                      "fill_color", 0xff00007f,
                                      "stroked", 1,
                                      "stroke_color", 0x0000ff7f,
                                      NULL);
            break;
        case CTRL_TYPE_ANCHOR:
            item = sp_canvas_item_new(parent, SP_TYPE_CTRL,
                                      "size", targetSize,
                                      "filled", 1,
                                      "fill_color", FILL_COLOR_NORMAL,
                                      "stroked", 1,
                                      "stroke_color", 0x000000ff,
                                      NULL);
            break;
        case CTRL_TYPE_ORIGIN:
            item = sp_canvas_item_new(parent, SP_TYPE_CTRLPOINT,
                                      "size", targetSize,
                                      NULL);
            break;
        case CTRL_TYPE_INVISIPOINT:
            item = sp_canvas_item_new(parent, SP_TYPE_CTRL,
                                      "shape", SP_CTRL_SHAPE_SQUARE,
                                      "size", targetSize,
                                      NULL);
            break;
        case CTRL_TYPE_UNKNOWN:
        default:
            item = sp_canvas_item_new(parent, SP_TYPE_CTRL, NULL);
    }
    if (item) {
        item->ctrlType = type;
    }
    return item;
}

void ControlManagerImpl::track(SPCanvasItem *item)
{
    g_object_weak_ref( G_OBJECT(item), ControlManagerImpl::thingFinalized, this );

    _itemList.push_back(item);

    setControlSize(_size, true);
}

sigc::connection ControlManagerImpl::connectCtrlSizeChanged(const sigc::slot<void> &slot)
{
    return _sizeChangedSignal.connect(slot);
}

void ControlManagerImpl::updateItem(SPCanvasItem *item)
{
    if (item) {
        double target = _sizeTable[item->ctrlType][_size - 1];
        if ((item->ctrlType == CTRL_TYPE_ORIGIN) && SP_IS_CTRLPOINT(item)) {
            sp_ctrlpoint_set_radius(SP_CTRLPOINT(item), target / 2.0);
        } else {
            sp_canvas_item_set(item, "size", target, NULL);
        }
        sp_canvas_item_request_update(item);
    }
}

void ControlManagerImpl::thingFinalized(gpointer data, GObject *wasObj)
{
    if (data) {
        reinterpret_cast<ControlManagerImpl *>(data)->thingFinalized(wasObj);
    }
}

void ControlManagerImpl::thingFinalized(GObject *wasObj)
{
    SPCanvasItem *wasItem = reinterpret_cast<SPCanvasItem *>(wasObj);
    if (wasItem)
    {
        std::vector<SPCanvasItem *>::iterator it = std::find(_itemList.begin(), _itemList.end(), wasItem);
        if (it != _itemList.end())
        {
            _itemList.erase(it);
        }
    }
}


// ----------------------------------------------------

ControlManager::ControlManager() :
    _impl(new ControlManagerImpl())
{
}

ControlManager::~ControlManager()
{
}

ControlManager &ControlManager::getManager()
{
    static ControlManager instance;

    return instance;
}


SPCanvasItem *ControlManager::createControl(SPCanvasGroup *parent, ControlType type)
{
    return _impl->createControl(parent, type);
}

SPCtrlLine *ControlManager::createControlLine(SPCanvasGroup *parent, CtrlLineType type)
{
    SPCtrlLine *line = SP_CTRLLINE(sp_canvas_item_new(parent, SP_TYPE_CTRLLINE, NULL));
    if (line) {
        line->ctrlType = CTRL_TYPE_LINE;

        line->setRgba32((type == CTLINE_PRIMARY) ? LINE_COLOR_PRIMARY :
                        (type == CTLINE_SECONDARY) ? LINE_COLOR_SECONDARY : LINE_COLOR_TERTIARY);
    }
    return line;
}

SPCtrlLine *ControlManager::createControlLine(SPCanvasGroup *parent, Geom::Point const &p1, Geom::Point const &p2, CtrlLineType type)
{
    SPCtrlLine *line = createControlLine(parent, type);
    if (line) {
        line->setCoords(p1, p2);
    }
    return line;
}

void ControlManager::track(SPCanvasItem *item)
{
    _impl->track(item);
}

sigc::connection ControlManager::connectCtrlSizeChanged(const sigc::slot<void> &slot)
{
    return _impl->connectCtrlSizeChanged(slot);
}

void ControlManager::updateItem(SPCanvasItem *item)
{
    return _impl->updateItem(item);
}

} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
