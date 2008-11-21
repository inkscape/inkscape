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

#include <gtkmm/notebook.h>
#include <glibmm/i18n.h>

#include <list>
#include <gtkmm/frame.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/table.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/label.h>
#include "libnr/nr-dim2.h"
#include "libnr/nr-rect.h"


#include "ui/widget/panel.h"
#include "ui/widget/notebook-page.h"

using namespace Inkscape::UI::Widget;


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

    Gtk::Table &align_table(){return _alignTable;}
    Gtk::Table &distribute_table(){return _distributeTable;}
    Gtk::Table &removeOverlap_table(){return _removeOverlapTable;}
    Gtk::Table &graphLayout_table(){return _graphLayoutTable;}
    Gtk::Table &nodes_table(){return _nodesTable;}
    Gtk::Tooltips &tooltips(){return _tooltips;}

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
    void addUnclumpButton(const Glib::ustring &id, const Glib::ustring tiptext, 
                        guint row, guint col);
    void addRandomizeButton(const Glib::ustring &id, const Glib::ustring tiptext, 
                        guint row, guint col);
    void addBaselineButton(const Glib::ustring &id, const Glib::ustring tiptext,
                           guint row, guint col, Gtk::Table &table, Geom::Dim2 orientation, bool distribute);

    std::list<Action *> _actionList;
    Gtk::Frame _alignFrame, _distributeFrame, _removeOverlapFrame, _graphLayoutFrame, _nodesFrame;
    Gtk::Table _alignTable, _distributeTable, _removeOverlapTable, _graphLayoutTable, _nodesTable;
    Gtk::HBox _anchorBox;
    Gtk::HBox _selgrpBox;
    Gtk::VBox _alignBox;
    Gtk::Label _anchorLabel;
    Gtk::Label _selgrpLabel;
    Gtk::CheckButton _selgrp;
    Gtk::ComboBoxText _combo;
    Gtk::Tooltips _tooltips;

private:
    AlignAndDistribute(AlignAndDistribute const &d);
    AlignAndDistribute& operator=(AlignAndDistribute const &d);
};


struct BBoxSort
{
    SPItem *item;
    float anchor;
    Geom::Rect bbox;
    BBoxSort(SPItem *pItem, Geom::Rect bounds, Geom::Dim2 orientation, double kBegin, double kEnd);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
