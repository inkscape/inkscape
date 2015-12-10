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
#include <set>

#include <glib-object.h>

#include "display/sodipodi-ctrl.h" // for SP_TYPE_CTRL
#include "display/sp-canvas-item.h"
#include "display/sp-ctrlline.h"
#include "display/sp-ctrlcurve.h"
#include "preferences.h"

using Inkscape::ControlFlags;

namespace {

std::map<Inkscape::ControlType, std::vector<int> > sizeTable;

// Note: The following operator overloads are local to this file at the moment to discourage flag manipulation elsewhere.
/*
ControlFlags operator |(ControlFlags lhs, ControlFlags rhs)
{
    return static_cast<ControlFlags>(static_cast<int>(lhs) | static_cast<int>(rhs));
}
*/

ControlFlags operator &(ControlFlags lhs, ControlFlags rhs)
{
    return static_cast<ControlFlags>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

ControlFlags operator ^(ControlFlags lhs, ControlFlags rhs)
{
    return static_cast<ControlFlags>(static_cast<int>(lhs) ^ static_cast<int>(rhs));
}

ControlFlags& operator ^=(ControlFlags &lhs, ControlFlags rhs)
{
    lhs = lhs ^ rhs;
    return lhs;
}

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
    ControlManagerImpl(ControlManager &manager);

    ~ControlManagerImpl() {}

    SPCanvasItem *createControl(SPCanvasGroup *parent, ControlType type);

    void setControlSize(int size, bool force = false);

    void track(SPCanvasItem *anchor);

    sigc::connection connectCtrlSizeChanged(const sigc::slot<void> &slot);

    void updateItem(SPCanvasItem *item);

    bool setControlType(SPCanvasItem *item, ControlType type);

    void setSelected(SPCanvasItem *item, bool selected);

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


    ControlManager &_manager;
    sigc::signal<void> _sizeChangedSignal;
    PrefListener _prefHook;
    int _size;
    std::vector<SPCanvasItem *> _itemList;
    std::map<Inkscape::ControlType, std::vector<int> > _sizeTable;
    std::map<Inkscape::ControlType, GType> _typeTable;
    std::map<Inkscape::ControlType, SPCtrlShapeType> _ctrlToShape;
    std::set<Inkscape::ControlType> _sizeChangers;
};

ControlManagerImpl::ControlManagerImpl(ControlManager &manager) :
    _manager(manager),
    _sizeChangedSignal(),
    _prefHook(*this),
    _size(3),
    _itemList(),
    _sizeTable()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->addObserver(_prefHook);

    _size = prefs->getIntLimited("/options/grabsize/value", 3, 1, 7);

    _typeTable[CTRL_TYPE_UNKNOWN] = SP_TYPE_CTRL;
    _typeTable[CTRL_TYPE_ADJ_HANDLE] = SP_TYPE_CTRL;
    _typeTable[CTRL_TYPE_ANCHOR] = SP_TYPE_CTRL;
    _typeTable[CTRL_TYPE_INVISIPOINT] = SP_TYPE_CTRL;
    _typeTable[CTRL_TYPE_NODE_AUTO] = SP_TYPE_CTRL;
    _typeTable[CTRL_TYPE_NODE_CUSP] = SP_TYPE_CTRL;
    _typeTable[CTRL_TYPE_NODE_SMOOTH] = SP_TYPE_CTRL;
    _typeTable[CTRL_TYPE_NODE_SYMETRICAL] = SP_TYPE_CTRL;

    _typeTable[CTRL_TYPE_LINE] = SP_TYPE_CTRLLINE;


    // -------
    _ctrlToShape[CTRL_TYPE_UNKNOWN] = SP_CTRL_SHAPE_DIAMOND;
    _ctrlToShape[CTRL_TYPE_NODE_CUSP] = SP_CTRL_SHAPE_DIAMOND;
    _ctrlToShape[CTRL_TYPE_NODE_SMOOTH] = SP_CTRL_SHAPE_SQUARE;
    _ctrlToShape[CTRL_TYPE_NODE_AUTO] = SP_CTRL_SHAPE_CIRCLE;
    _ctrlToShape[CTRL_TYPE_NODE_SYMETRICAL] = SP_CTRL_SHAPE_SQUARE;

    _ctrlToShape[CTRL_TYPE_ADJ_HANDLE] = SP_CTRL_SHAPE_CIRCLE;
    _ctrlToShape[CTRL_TYPE_INVISIPOINT] = SP_CTRL_SHAPE_SQUARE;

    // -------

    _sizeChangers.insert(CTRL_TYPE_NODE_AUTO);
    _sizeChangers.insert(CTRL_TYPE_NODE_CUSP);
    _sizeChangers.insert(CTRL_TYPE_NODE_SMOOTH);
    _sizeChangers.insert(CTRL_TYPE_NODE_SYMETRICAL);

    // -------

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
        int sizes[] = {5, 7, 9, 10, 11, 12, 13};
        _sizeTable[CTRL_TYPE_NODE_AUTO] = std::vector<int>(sizes, sizes + (sizeof(sizes) / sizeof(sizes[0])));
        _sizeTable[CTRL_TYPE_NODE_CUSP] = std::vector<int>(sizes, sizes + (sizeof(sizes) / sizeof(sizes[0])));
    }
    {
        int sizes[] = {3, 5, 7, 8, 9, 10, 11};
        _sizeTable[CTRL_TYPE_NODE_SMOOTH] = std::vector<int>(sizes, sizes + (sizeof(sizes) / sizeof(sizes[0])));
        _sizeTable[CTRL_TYPE_NODE_SYMETRICAL] = std::vector<int>(sizes, sizes + (sizeof(sizes) / sizeof(sizes[0])));
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
        case CTRL_TYPE_NODE_AUTO:
        case CTRL_TYPE_NODE_CUSP:
        case CTRL_TYPE_NODE_SMOOTH:
        case CTRL_TYPE_NODE_SYMETRICAL:
        {
            SPCtrlShapeType shape = _ctrlToShape[_ctrlToShape.count(type) ? type : CTRL_TYPE_UNKNOWN];
            item = sp_canvas_item_new(parent, SP_TYPE_CTRL,
                                      "shape", shape,
                                      "size", targetSize,
                                      NULL);
            break;
        }
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

        if (_sizeChangers.count(item->ctrlType) && _manager.isSelected(item)) {
            target += 2;
        }
        g_object_set(item, "size", target, NULL);

        sp_canvas_item_request_update(item);
    }
}

bool ControlManagerImpl::setControlType(SPCanvasItem *item, ControlType type)
{
    bool accepted = false;
    if (item && (item->ctrlType == type)) {
        // nothing to do
        accepted = true;
    } else if (item) {
        if (_ctrlToShape.count(type) && (_typeTable[type] == _typeTable[item->ctrlType])) { // compatible?
            double targetSize = _sizeTable[type][_size - 1];
            if (_manager.isSelected(item) && _sizeChangers.count(item->ctrlType)) {
                targetSize += 2.0;
            }
            SPCtrlShapeType targetShape = _ctrlToShape[type];
            g_object_set(item, "shape", targetShape, "size", targetSize, NULL);
            item->ctrlType = type;
            accepted = true;
        }
    }

    return accepted;
}


void ControlManagerImpl::setSelected(SPCanvasItem *item, bool selected)
{
    if (_manager.isSelected(item) != selected) {
        item->ctrlFlags ^= CTRL_FLAG_SELECTED; // toggle, since we know it is different

        // TODO refresh colors
        double targetSize = _sizeTable[item->ctrlType][_size - 1];
        if (selected && _sizeChangers.count(item->ctrlType)) {
            targetSize += 2.0;
        }
        g_object_set(item, "size", targetSize, NULL);
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
    _impl(new ControlManagerImpl(*this))
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

SPCtrlCurve *ControlManager::createControlCurve(SPCanvasGroup *parent, Geom::Point const &p0, Geom::Point const &p1, Geom::Point const &p2, Geom::Point const &p3, CtrlLineType type)
{
    SPCtrlCurve *line = SP_CTRLCURVE(sp_canvas_item_new(parent, SP_TYPE_CTRLCURVE, NULL));
    if (line) {
        line->ctrlType = CTRL_TYPE_LINE;

        line->setRgba32((type == CTLINE_PRIMARY) ? LINE_COLOR_PRIMARY :
                        (type == CTLINE_SECONDARY) ? LINE_COLOR_SECONDARY : LINE_COLOR_TERTIARY);
        line->setCoords(p0, p1, p2, p3);
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

bool ControlManager::setControlType(SPCanvasItem *item, ControlType type)
{
    return _impl->setControlType(item, type);
}

bool ControlManager::isActive(SPCanvasItem *item) const
{
    return (item->ctrlFlags & CTRL_FLAG_ACTIVE) != 0;
}

void ControlManager::setActive(SPCanvasItem *item, bool active)
{
    if (isActive(item) != active) {
        item->ctrlFlags ^= CTRL_FLAG_ACTIVE; // toggle, since we know it is different
        // TODO refresh size/colors
    }
}

bool ControlManager::isPrelight(SPCanvasItem *item) const
{
    return (item->ctrlFlags & CTRL_FLAG_PRELIGHT) != 0;
}

void ControlManager::setPrelight(SPCanvasItem *item, bool prelight)
{
    if (isPrelight(item) != prelight) {
        item->ctrlFlags ^= CTRL_FLAG_PRELIGHT; // toggle, since we know it is different
        // TODO refresh size/colors
    }
}

bool ControlManager::isSelected(SPCanvasItem *item) const
{
    return (item->ctrlFlags & CTRL_FLAG_SELECTED) != 0;
}

void ControlManager::setSelected(SPCanvasItem *item, bool selected)
{
    _impl->setSelected(item, selected);
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
