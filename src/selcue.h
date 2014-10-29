#ifndef SEEN_SP_SELCUE_H
#define SEEN_SP_SELCUE_H

/*
 * Helper object for showing selected items
 *
 * Authors:
 *   bulia byak <bulia@users.sf.net>
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstddef>
#include <list>
#include <sigc++/sigc++.h>

class  SPDesktop;
struct SPCanvasItem;

namespace Inkscape {

class Selection;

class SelCue
{
public:
    SelCue(SPDesktop *desktop);
    ~SelCue();

    enum Type {
        NONE,
        MARK,
        BBOX
    };

private:
    class BoundingBoxPrefsObserver: public Preferences::Observer
    {
    public:
        BoundingBoxPrefsObserver(SelCue &sel_cue);

        void notify(Preferences::Entry const &val);

    private:
        SelCue &_sel_cue;
    };

    friend class Inkscape::SelCue::BoundingBoxPrefsObserver;

    void _updateItemBboxes();
    void _updateItemBboxes(Inkscape::Preferences *prefs);
    void _updateItemBboxes(int mode, int prefs_bbox);
    void _newItemBboxes();
    void _newTextBaselines();
    void _boundingBoxPrefsChanged(int prefs_bbox);

    SPDesktop *_desktop;
    Selection *_selection;
    sigc::connection _sel_changed_connection;
    sigc::connection _sel_modified_connection;
    std::vector<SPCanvasItem*> _item_bboxes;
    std::vector<SPCanvasItem*> _text_baselines;

    BoundingBoxPrefsObserver _bounding_box_prefs_observer;
};

}
  
#endif


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
