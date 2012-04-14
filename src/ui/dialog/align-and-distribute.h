/** @file
 * @brief Align and Distribute dialog
 */
/* Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Aubanel MONNIER <aubi@libertysurf.fr>
 *   Frank Felfe <innerspace@iname.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2004, 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_DIALOG_ALIGN_AND_DISTRIBUTE_H
#define INKSCAPE_UI_DIALOG_ALIGN_AND_DISTRIBUTE_H

#include <list>
#include "ui/widget/panel.h"
#include <gtkmm/frame.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/label.h>
#include "2geom/rect.h"

#if WITH_GTKMM_3_0
#include <gtkmm/checkbutton.h>
#include <gtkmm/grid.h>
#else
#include <gtkmm/table.h>
#endif

class SPItem;

namespace Inkscape {
namespace UI {
namespace Dialog {

class Action;


class AlignAndDistribute : public Widget::Panel {
public:
    AlignAndDistribute();
    virtual ~AlignAndDistribute();

    static AlignAndDistribute &getInstance() { return *new AlignAndDistribute(); }

    enum AlignTarget { LAST=0, FIRST, BIGGEST, SMALLEST, PAGE, DRAWING, SELECTION };

    AlignTarget getAlignTarget() const;

#if WITH_GTKMM_3_0
    Gtk::Grid &align_table(){return _alignTable;}
    Gtk::Grid &distribute_table(){return _distributeTable;}
    Gtk::Grid &rearrange_table(){return _rearrangeTable;}
    Gtk::Grid &removeOverlap_table(){return _removeOverlapTable;}
    Gtk::Grid &nodes_table(){return _nodesTable;}
#else
    Gtk::Table &align_table(){return _alignTable;}
    Gtk::Table &distribute_table(){return _distributeTable;}
    Gtk::Table &rearrange_table(){return _rearrangeTable;}
    Gtk::Table &removeOverlap_table(){return _removeOverlapTable;}
    Gtk::Table &nodes_table(){return _nodesTable;}
#endif

    std::list<SPItem *>::iterator find_master(std::list <SPItem *> &list, bool horizontal);
    void setMode(bool nodeEdit);

    Geom::OptRect randomize_bbox;

protected:

    void on_ref_change();
    void on_selgrp_toggled();
    void addDistributeButton(const Glib::ustring &id, const Glib::ustring tiptext, 
                                      guint row, guint col, bool onInterSpace, 
                                      Geom::Dim2 orientation, float kBegin, float kEnd);
    void addAlignButton(const Glib::ustring &id, const Glib::ustring tiptext, 
                        guint row, guint col);
    void addNodeButton(const Glib::ustring &id, const Glib::ustring tiptext, 
                        guint col, Geom::Dim2 orientation, bool distribute);
    void addRemoveOverlapsButton(const Glib::ustring &id,
                        const Glib::ustring tiptext,
                        guint row, guint col);
    void addGraphLayoutButton(const Glib::ustring &id,
                        const Glib::ustring tiptext,
                        guint row, guint col);
    void addExchangePositionsButton(const Glib::ustring &id,
                        const Glib::ustring tiptext,
                        guint row, guint col);
    void addExchangePositionsByZOrderButton(const Glib::ustring &id,
                        const Glib::ustring tiptext,
                        guint row, guint col);
    void addExchangePositionsClockwiseButton(const Glib::ustring &id,
                        const Glib::ustring tiptext,
                        guint row, guint col);
    void addUnclumpButton(const Glib::ustring &id, const Glib::ustring tiptext, 
                        guint row, guint col);
    void addRandomizeButton(const Glib::ustring &id, const Glib::ustring tiptext, 
                        guint row, guint col);
#if WITH_GTKMM_3_0
    void addBaselineButton(const Glib::ustring &id, const Glib::ustring tiptext,
                           guint row, guint col, Gtk::Grid &table, Geom::Dim2 orientation, bool distribute);
#else
    void addBaselineButton(const Glib::ustring &id, const Glib::ustring tiptext,
                           guint row, guint col, Gtk::Table &table, Geom::Dim2 orientation, bool distribute);
#endif

    std::list<Action *> _actionList;
    Gtk::Frame _alignFrame, _distributeFrame, _rearrangeFrame, _removeOverlapFrame, _nodesFrame;
#if WITH_GTKMM_3_0
    Gtk::Grid _alignTable, _distributeTable, _rearrangeTable, _removeOverlapTable, _nodesTable;
#else
    Gtk::Table _alignTable, _distributeTable, _rearrangeTable, _removeOverlapTable, _nodesTable;
#endif
    Gtk::HBox _anchorBox;
    Gtk::HBox _selgrpBox;
    Gtk::VBox _alignBox;
    Gtk::Label _anchorLabel;
    Gtk::Label _selgrpLabel;
    Gtk::CheckButton _selgrp;
    Gtk::ComboBoxText _combo;

private:
    AlignAndDistribute(AlignAndDistribute const &d);
    AlignAndDistribute& operator=(AlignAndDistribute const &d);
};


struct BBoxSort
{
    SPItem *item;
    float anchor;
    Geom::Rect bbox;
    BBoxSort(SPItem *pItem, Geom::Rect const &bounds, Geom::Dim2 orientation, double kBegin, double kEnd);
    BBoxSort(const BBoxSort &rhs);
};
bool operator< (const BBoxSort &a, const BBoxSort &b);

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif // INKSCAPE_UI_DIALOG_ALIGN_AND_DISTRIBUTE_H

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
