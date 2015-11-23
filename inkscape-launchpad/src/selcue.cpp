/*
 * Helper object for showing selected items
 *
 * Authors:
 *   bulia byak <bulia@users.sf.net>
 *   Carl Hetherington <inkscape@carlh.net>
 *   Abhishek Sharma
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string.h>

#include "desktop.h"

#include "selection.h"
#include "display/sp-canvas-util.h"
#include "display/sodipodi-ctrl.h"
#include "display/sodipodi-ctrlrect.h"
#include "libnrtype/Layout-TNG.h"
#include "text-editing.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "preferences.h"
#include "selcue.h"

Inkscape::SelCue::BoundingBoxPrefsObserver::BoundingBoxPrefsObserver(SelCue &sel_cue) :
    Observer("/tools/bounding_box"),
    _sel_cue(sel_cue)
{
}

void Inkscape::SelCue::BoundingBoxPrefsObserver::notify(Preferences::Entry const &val)
{
    _sel_cue._boundingBoxPrefsChanged(static_cast<int>(val.getBool()));
}

Inkscape::SelCue::SelCue(SPDesktop *desktop)
    : _desktop(desktop),
      _bounding_box_prefs_observer(*this)
{
    _selection = _desktop->getSelection();

    _sel_changed_connection = _selection->connectChanged(
        sigc::hide(sigc::mem_fun(*this, &Inkscape::SelCue::_newItemBboxes))
        );

    {
        void(SelCue::*modifiedSignal)() = &SelCue::_updateItemBboxes;
        _sel_modified_connection = _selection->connectModified(
            sigc::hide(sigc::hide(sigc::mem_fun(*this, modifiedSignal)))
        );
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    _updateItemBboxes(prefs);
    prefs->addObserver(_bounding_box_prefs_observer);
}

Inkscape::SelCue::~SelCue()
{
    _sel_changed_connection.disconnect();
    _sel_modified_connection.disconnect();

    for (std::vector<SPCanvasItem*>::iterator i = _item_bboxes.begin(); i != _item_bboxes.end(); ++i) {
        sp_canvas_item_destroy(*i);
    }
    _item_bboxes.clear();

    for (std::vector<SPCanvasItem*>::iterator i = _text_baselines.begin(); i != _text_baselines.end(); ++i) {
        sp_canvas_item_destroy(*i);
    }
    _text_baselines.clear();
}

void Inkscape::SelCue::_updateItemBboxes()
{
    _updateItemBboxes(Inkscape::Preferences::get());
}

void Inkscape::SelCue::_updateItemBboxes(Inkscape::Preferences *prefs)
{
    gint mode = prefs->getInt("/options/selcue/value", MARK);
    if (mode == NONE) {
        return;
    }

    g_return_if_fail(_selection != NULL);

    int prefs_bbox = prefs->getBool("/tools/bounding_box");

    _updateItemBboxes(mode, prefs_bbox);
}

void Inkscape::SelCue::_updateItemBboxes(gint mode, int prefs_bbox)
{
    const std::vector<SPItem*> items = _selection->itemList();
    if (_item_bboxes.size() != items.size()) {
        _newItemBboxes();
        return;
    }

    int bcount = 0;
    std::vector<SPItem*> ll=_selection->itemList();
    for (std::vector<SPItem*>::const_iterator l = ll.begin(); l != ll.end(); ++l) {
        SPItem *item = *l;
        SPCanvasItem* box = _item_bboxes[bcount ++];

        if (box) {
            Geom::OptRect const b = (prefs_bbox == 0) ?
                item->desktopVisualBounds() : item->desktopGeometricBounds();

            if (b) {
                sp_canvas_item_show(box);
                if (mode == MARK) {
                    SP_CTRL(box)->moveto(Geom::Point(b->min()[Geom::X], b->max()[Geom::Y]));
                } else if (mode == BBOX) {
                    SP_CTRLRECT(box)->setRectangle(*b);
                }
            } else { // no bbox
                sp_canvas_item_hide(box);
            }
        }
    }

    _newTextBaselines();
}


void Inkscape::SelCue::_newItemBboxes()
{
    for (std::vector<SPCanvasItem*>::iterator i = _item_bboxes.begin(); i != _item_bboxes.end(); ++i) {
        sp_canvas_item_destroy(*i);
    }
    _item_bboxes.clear();

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gint mode = prefs->getInt("/options/selcue/value", MARK);
    if (mode == NONE) {
        return;
    }

    g_return_if_fail(_selection != NULL);

    int prefs_bbox = prefs->getBool("/tools/bounding_box");
    
    std::vector<SPItem*> ll=_selection->itemList();
    for (std::vector<SPItem*>::const_iterator l = ll.begin(); l != ll.end(); ++l) {
        SPItem *item = *l;

        Geom::OptRect const b = (prefs_bbox == 0) ?
            item->desktopVisualBounds() : item->desktopGeometricBounds();

        SPCanvasItem* box = NULL;

        if (b) {
            if (mode == MARK) {
                box = sp_canvas_item_new(_desktop->getControls(),
                                         SP_TYPE_CTRL,
                                         "mode", SP_CTRL_MODE_XOR,
                                         "shape", SP_CTRL_SHAPE_DIAMOND,
                                         "size", 5.0,
                                         "filled", TRUE,
                                         "fill_color", 0x000000ff,
                                         "stroked", FALSE,
                                         "stroke_color", 0x000000ff,
                                         NULL);
                sp_canvas_item_show(box);
                SP_CTRL(box)->moveto(Geom::Point(b->min()[Geom::X], b->max()[Geom::Y]));

                sp_canvas_item_move_to_z(box, 0); // just low enough to not get in the way of other draggable knots

            } else if (mode == BBOX) {
                box = sp_canvas_item_new(_desktop->getControls(),
                                         SP_TYPE_CTRLRECT,
                                         NULL);

                SP_CTRLRECT(box)->setRectangle(*b);
                SP_CTRLRECT(box)->setColor(0x000000a0, 0, 0);
                SP_CTRLRECT(box)->setDashed(true);
                SP_CTRLRECT(box)->setShadow(1, 0xffffffff);

                sp_canvas_item_move_to_z(box, 0);
            }
        }

        if (box) {
            _item_bboxes.push_back(box);
        }
    }

    _newTextBaselines();
}

void Inkscape::SelCue::_newTextBaselines()
{
    for (std::vector<SPCanvasItem*>::iterator i = _text_baselines.begin(); i != _text_baselines.end(); ++i) {
        sp_canvas_item_destroy(*i);
    }
    _text_baselines.clear();

    std::vector<SPItem*> ll = _selection->itemList();
    for (std::vector<SPItem*>::const_iterator l=ll.begin();l!=ll.end();++l) {
        SPItem *item = *l;

        SPCanvasItem* baseline_point = NULL;
        if (SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item)) { // visualize baseline
            Inkscape::Text::Layout const *layout = te_get_layout(item);
            if (layout != NULL && layout->outputExists()) {
                boost::optional<Geom::Point> pt = layout->baselineAnchorPoint();
                if (pt) {
                    baseline_point = sp_canvas_item_new(_desktop->getControls(), SP_TYPE_CTRL,
                        "mode", SP_CTRL_MODE_XOR,
                        "size", 4.0,
                        "filled", 0,
                        "stroked", 1,
                        "stroke_color", 0x000000ff,
                        NULL);

                    sp_canvas_item_show(baseline_point);
                    SP_CTRL(baseline_point)->moveto((*pt) * item->i2dt_affine());
                    sp_canvas_item_move_to_z(baseline_point, 0);
                }
            }
        }

        if (baseline_point) {
               _text_baselines.push_back(baseline_point);
        }
    }
}

void Inkscape::SelCue::_boundingBoxPrefsChanged(int prefs_bbox)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gint mode = prefs->getInt("/options/selcue/value", MARK);
    if (mode == NONE) {
        return;
    }

    g_return_if_fail(_selection != NULL);

    _updateItemBboxes(mode, prefs_bbox);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
