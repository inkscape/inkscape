/**
 * @file
 * Align and Distribute dialog - implementation.
 */
/* Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Aubanel MONNIER <aubi@libertysurf.fr>
 *   Frank Felfe <innerspace@iname.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Tim Dwyer <tgdwyer@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2004, 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "align-and-distribute.h"
#include <2geom/transforms.h>
#include "ui/widget/spinbutton.h"

#include "unclump.h"
#include "document.h"
#include "enums.h"
#include "graphlayout.h"
#include "inkscape.h"
#include "macros.h"
#include "preferences.h"
#include "removeoverlap.h"
#include "selection.h"
#include "sp-flowtext.h"
#include "sp-item-transform.h"
#include "sp-text.h"
#include "text-editing.h"
#include "ui/tools-switch.h"
#include "ui/icon-names.h"
#include "ui/tools/node-tool.h"
#include "ui/tool/multi-path-manipulator.h"
#include "verbs.h"
#include "widgets/icon.h"
#include "sp-root.h"
#include "document-undo.h"
#include "desktop.h"

#include <glibmm/i18n.h>


namespace Inkscape {
namespace UI {
namespace Dialog {

/////////helper classes//////////////////////////////////

Action::Action(const Glib::ustring &id,
       const Glib::ustring &tiptext,
       guint row, guint column,
#if WITH_GTKMM_3_0
   Gtk::Grid &parent,
#else
   Gtk::Table &parent,
#endif
       AlignAndDistribute &dialog):
    _dialog(dialog),
    _id(id),
    _parent(parent)
{
    Gtk::Widget*  pIcon = Gtk::manage( sp_icon_get_icon( _id, Inkscape::ICON_SIZE_LARGE_TOOLBAR) );
    Gtk::Button * pButton = Gtk::manage(new Gtk::Button());
    pButton->set_relief(Gtk::RELIEF_NONE);
    pIcon->show();
    pButton->add(*pIcon);
    pButton->show();

    pButton->signal_clicked()
        .connect(sigc::mem_fun(*this, &Action::on_button_click));
    pButton->set_tooltip_text(tiptext);
#if WITH_GTKMM_3_0
    parent.attach(*pButton, column, row, 1, 1);
#else
    parent.attach(*pButton, column, column+1, row, row+1, Gtk::FILL, Gtk::FILL);
#endif
}


void ActionAlign::do_action(SPDesktop *desktop, int index)
{
    Inkscape::Selection *selection = desktop->getSelection();
    if (!selection) return;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool sel_as_group = prefs->getBool("/dialogs/align/sel-as-groups");

    std::vector<SPItem*> selected(selection->itemList());
    if (selected.empty()) return;

    const Coeffs &a = _allCoeffs[index];
    SPItem *focus = NULL;
    Geom::OptRect b = Geom::OptRect();
    Selection::CompareSize horiz = (a.mx0 != 0.0) || (a.mx1 != 0.0)
        ? Selection::HORIZONTAL : Selection::VERTICAL;

    switch (AlignTarget(prefs->getInt("/dialogs/align/align-to", 6)))
    {
    case LAST:
        focus = SP_ITEM(*selected.begin());
        break;
    case FIRST:
        focus = SP_ITEM(*--(selected.end()));
        break;
    case BIGGEST:
        focus = selection->largestItem(horiz);
        break;
    case SMALLEST:
        focus = selection->smallestItem(horiz);
        break;
    case PAGE:
        b = desktop->getDocument()->preferredBounds();
        break;
    case DRAWING:
        b = desktop->getDocument()->getRoot()->desktopPreferredBounds();
        break;
    case SELECTION:
        b = selection->preferredBounds();
        break;
    default:
        g_assert_not_reached ();
        break;
    };

    if(focus)
        b = focus->desktopPreferredBounds();
    g_return_if_fail(b);

    // Generate the move point from the selected bounding box
    Geom::Point mp = Geom::Point(a.mx0 * b->min()[Geom::X] + a.mx1 * b->max()[Geom::X],
                                 a.my0 * b->min()[Geom::Y] + a.my1 * b->max()[Geom::Y]);

    bool changed = false;
    if (sel_as_group)
        b = selection->preferredBounds();

    //Move each item in the selected list separately
    for (std::vector<SPItem*>::iterator it(selected.begin());
         it != selected.end(); ++it)
    {
    	SPItem* item= *it;
        desktop->getDocument()->ensureUpToDate();
        if (!sel_as_group)
            b = (item)->desktopPreferredBounds();
        if (b && (!focus || (item) != focus)) {
            Geom::Point const sp(a.sx0 * b->min()[Geom::X] + a.sx1 * b->max()[Geom::X],
                                 a.sy0 * b->min()[Geom::Y] + a.sy1 * b->max()[Geom::Y]);
            Geom::Point const mp_rel( mp - sp );
            if (LInfty(mp_rel) > 1e-9) {
                sp_item_move_rel(item, Geom::Translate(mp_rel));
                changed = true;
            }
        }
    }

    if (changed) {
        DocumentUndo::done( desktop->getDocument() , SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
                            _("Align"));
    }
}


ActionAlign::Coeffs const ActionAlign::_allCoeffs[11] = {
    {1., 0., 0., 0., 0., 1., 0., 0., SP_VERB_ALIGN_HORIZONTAL_RIGHT_TO_ANCHOR},
    {1., 0., 0., 0., 1., 0., 0., 0., SP_VERB_ALIGN_HORIZONTAL_LEFT},
    {.5, .5, 0., 0., .5, .5, 0., 0., SP_VERB_ALIGN_HORIZONTAL_CENTER},
    {0., 1., 0., 0., 0., 1., 0., 0., SP_VERB_ALIGN_HORIZONTAL_RIGHT},
    {0., 1., 0., 0., 1., 0., 0., 0., SP_VERB_ALIGN_HORIZONTAL_LEFT_TO_ANCHOR},
    {0., 0., 0., 1., 0., 0., 1., 0., SP_VERB_ALIGN_VERTICAL_BOTTOM_TO_ANCHOR},
    {0., 0., 0., 1., 0., 0., 0., 1., SP_VERB_ALIGN_VERTICAL_TOP},
    {0., 0., .5, .5, 0., 0., .5, .5, SP_VERB_ALIGN_VERTICAL_CENTER},
    {0., 0., 1., 0., 0., 0., 1., 0., SP_VERB_ALIGN_VERTICAL_BOTTOM},
    {0., 0., 1., 0., 0., 0., 0., 1., SP_VERB_ALIGN_VERTICAL_TOP_TO_ANCHOR},
    {.5, .5, .5, .5, .5, .5, .5, .5, SP_VERB_ALIGN_VERTICAL_HORIZONTAL_CENTER}
};

void ActionAlign::do_verb_action(SPDesktop *desktop, int verb)
{
    do_action(desktop, verb_to_coeff(verb));
}

int ActionAlign::verb_to_coeff(int verb) {

    for(guint i = 0; i < G_N_ELEMENTS(_allCoeffs); i++) {
        if (_allCoeffs[i].verb_id == verb) {
            return i;
        }
    }

    return -1;
}

BBoxSort::BBoxSort(SPItem *pItem, Geom::Rect const &bounds, Geom::Dim2 orientation, double kBegin, double kEnd) :
        item(pItem),
        bbox (bounds)
{
        anchor = kBegin * bbox.min()[orientation] + kEnd * bbox.max()[orientation];
}
BBoxSort::BBoxSort(const BBoxSort &rhs) :
        //NOTE :  this copy ctor is called O(sort) when sorting the vector
        //this is bad. The vector should be a vector of pointers.
        //But I'll wait the bohem GC before doing that
        item(rhs.item), anchor(rhs.anchor), bbox(rhs.bbox)
{
}

bool operator< (const BBoxSort &a, const BBoxSort &b)
{
    return (a.anchor < b.anchor);
}

class ActionDistribute : public Action {
public :
    ActionDistribute(const Glib::ustring &id,
                     const Glib::ustring &tiptext,
                     guint row, guint column,
                     AlignAndDistribute &dialog,
                     bool onInterSpace,
                     Geom::Dim2 orientation,
                     double kBegin, double kEnd
        ):
        Action(id, tiptext, row, column,
               dialog.distribute_table(), dialog),
        _dialog(dialog),
        _onInterSpace(onInterSpace),
        _orientation(orientation),
        _kBegin(kBegin),
        _kEnd( kEnd)
    {}

private :
    virtual void on_button_click() {
        //Retreive selected objects
        SPDesktop *desktop = _dialog.getDesktop();
        if (!desktop) return;

        Inkscape::Selection *selection = desktop->getSelection();
        if (!selection) return;

        std::vector<SPItem*> selected(selection->itemList());
        if (selected.empty()) return;

        //Check 2 or more selected objects
        std::vector<SPItem*>::iterator second(selected.begin());
        ++second;
        if (second == selected.end()) return;

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        int prefs_bbox = prefs->getBool("/tools/bounding_box");
        std::vector< BBoxSort  > sorted;
        for (std::vector<SPItem*>::iterator it(selected.begin());
            it != selected.end();
            ++it){
            SPItem *item = *it;
            Geom::OptRect bbox = !prefs_bbox ? (item)->desktopVisualBounds() : (item)->desktopGeometricBounds();
            if (bbox) {
                sorted.push_back(BBoxSort(item, *bbox, _orientation, _kBegin, _kEnd));
            }
        }
        //sort bbox by anchors
        std::sort(sorted.begin(), sorted.end());

        // see comment in ActionAlign above
        int saved_compensation = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
        prefs->setInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);

        unsigned int len = sorted.size();
        bool changed = false;
        if (_onInterSpace)
        {
            //overall bboxes span
            float dist = (sorted.back().bbox.max()[_orientation] -
                          sorted.front().bbox.min()[_orientation]);
            //space eaten by bboxes
            float span = 0;
            for (unsigned int i = 0; i < len; i++)
            {
                span += sorted[i].bbox[_orientation].extent();
            }
            //new distance between each bbox
            float step = (dist - span) / (len - 1);
            float pos = sorted.front().bbox.min()[_orientation];
            for ( std::vector<BBoxSort> ::iterator it (sorted.begin());
                  it < sorted.end();
                  ++it )
            {
                if (!Geom::are_near(pos, it->bbox.min()[_orientation], 1e-6)) {
                    Geom::Point t(0.0, 0.0);
                    t[_orientation] = pos - it->bbox.min()[_orientation];
                    sp_item_move_rel(it->item, Geom::Translate(t));
                    changed = true;
                }
                pos += it->bbox[_orientation].extent();
                pos += step;
            }
        }
        else
        {
            //overall anchor span
            float dist = sorted.back().anchor - sorted.front().anchor;
            //distance between anchors
            float step = dist / (len - 1);

            for ( unsigned int i = 0; i < len ; i ++ )
            {
                BBoxSort & it(sorted[i]);
                //new anchor position
                float pos = sorted.front().anchor + i * step;
                //Don't move if we are really close
                if (!Geom::are_near(pos, it.anchor, 1e-6)) {
                    //Compute translation
                    Geom::Point t(0.0, 0.0);
                    t[_orientation] = pos - it.anchor;
                    //translate
                    sp_item_move_rel(it.item, Geom::Translate(t));
                    changed = true;
                }
            }
        }

        // restore compensation setting
        prefs->setInt("/options/clonecompensation/value", saved_compensation);

        if (changed) {
            DocumentUndo::done( desktop->getDocument(), SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
                                _("Distribute"));
        }
    }
    guint _index;
    AlignAndDistribute &_dialog;
    bool _onInterSpace;
    Geom::Dim2 _orientation;

    double _kBegin;
    double _kEnd;

};


class ActionNode : public Action {
public :
    ActionNode(const Glib::ustring &id,
               const Glib::ustring &tiptext,
               guint column,
               AlignAndDistribute &dialog,
               Geom::Dim2 orientation, bool distribute):
        Action(id, tiptext, 0, column,
               dialog.nodes_table(), dialog),
        _orientation(orientation),
        _distribute(distribute)
    {}

private :
    Geom::Dim2 _orientation;
    bool _distribute;

    virtual void on_button_click() {
        if (!_dialog.getDesktop()) {
        	return;
        }

        Inkscape::UI::Tools::ToolBase *event_context = _dialog.getDesktop()->getEventContext();

        if (!INK_IS_NODE_TOOL(event_context)) {
        	return;
        }

        Inkscape::UI::Tools::NodeTool *nt = INK_NODE_TOOL(event_context);

        if (_distribute) {
            nt->_multipath->distributeNodes(_orientation);
        } else {
            nt->_multipath->alignNodes(_orientation);
        }
    }
};

class ActionRemoveOverlaps : public Action {
private:
    Gtk::Label removeOverlapXGapLabel;
    Gtk::Label removeOverlapYGapLabel;
    Inkscape::UI::Widget::SpinButton removeOverlapXGap;
    Inkscape::UI::Widget::SpinButton removeOverlapYGap;

public:
    ActionRemoveOverlaps(Glib::ustring const &id,
                         Glib::ustring const &tiptext,
                         guint row,
                         guint column,
                         AlignAndDistribute &dialog) :
        Action(id, tiptext, row, column + 4,
               dialog.removeOverlap_table(), dialog)
    {
#if WITH_GTKMM_3_0
        dialog.removeOverlap_table().set_column_spacing(3);
#else
        dialog.removeOverlap_table().set_col_spacings(3);
#endif

        removeOverlapXGap.set_digits(1);
        removeOverlapXGap.set_size_request(60, -1);
        removeOverlapXGap.set_increments(1.0, 0);
        removeOverlapXGap.set_range(-1000.0, 1000.0);
        removeOverlapXGap.set_value(0);
        removeOverlapXGap.set_tooltip_text(_("Minimum horizontal gap (in px units) between bounding boxes"));
        //TRANSLATORS: "H:" stands for horizontal gap
        removeOverlapXGapLabel.set_text_with_mnemonic(C_("Gap", "_H:"));
        removeOverlapXGapLabel.set_mnemonic_widget(removeOverlapXGap);

        removeOverlapYGap.set_digits(1);
        removeOverlapYGap.set_size_request(60, -1);
        removeOverlapYGap.set_increments(1.0, 0);
        removeOverlapYGap.set_range(-1000.0, 1000.0);
        removeOverlapYGap.set_value(0);
        removeOverlapYGap.set_tooltip_text(_("Minimum vertical gap (in px units) between bounding boxes"));
        /* TRANSLATORS: Vertical gap */
        removeOverlapYGapLabel.set_text_with_mnemonic(C_("Gap", "_V:"));
        removeOverlapYGapLabel.set_mnemonic_widget(removeOverlapYGap);

#if WITH_GTKMM_3_0
        dialog.removeOverlap_table().attach(removeOverlapXGapLabel, column, row, 1, 1);
        dialog.removeOverlap_table().attach(removeOverlapXGap, column+1, row, 1, 1);
        dialog.removeOverlap_table().attach(removeOverlapYGapLabel, column+2, row, 1, 1);
        dialog.removeOverlap_table().attach(removeOverlapYGap, column+3, row, 1, 1);
#else
        dialog.removeOverlap_table().attach(removeOverlapXGapLabel, column, column+1, row, row+1, Gtk::FILL, Gtk::FILL);
        dialog.removeOverlap_table().attach(removeOverlapXGap, column+1, column+2, row, row+1, Gtk::FILL, Gtk::FILL);
        dialog.removeOverlap_table().attach(removeOverlapYGapLabel, column+2, column+3, row, row+1, Gtk::FILL, Gtk::FILL);
        dialog.removeOverlap_table().attach(removeOverlapYGap, column+3, column+4, row, row+1, Gtk::FILL, Gtk::FILL);
#endif
    }

private :
    virtual void on_button_click()
    {
        if (!_dialog.getDesktop()) return;

        // see comment in ActionAlign above
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        int saved_compensation = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
        prefs->setInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);

        // xGap and yGap are the minimum space required between bounding rectangles.
        double const xGap = removeOverlapXGap.get_value();
        double const yGap = removeOverlapYGap.get_value();
        removeoverlap(_dialog.getDesktop()->getSelection()->itemList(), xGap, yGap);

        // restore compensation setting
        prefs->setInt("/options/clonecompensation/value", saved_compensation);

        DocumentUndo::done(_dialog.getDesktop()->getDocument(), SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
                           _("Remove overlaps"));
    }
};

class ActionGraphLayout : public Action {
public:
    ActionGraphLayout(Glib::ustring const &id,
                         Glib::ustring const &tiptext,
                         guint row,
                         guint column,
                         AlignAndDistribute &dialog) :
        Action(id, tiptext, row, column,
               dialog.rearrange_table(), dialog)
    {}

private :
    virtual void on_button_click()
    {
        if (!_dialog.getDesktop()) return;

        // see comment in ActionAlign above
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        int saved_compensation = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
        prefs->setInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);

        graphlayout(_dialog.getDesktop()->getSelection()->itemList());

        // restore compensation setting
        prefs->setInt("/options/clonecompensation/value", saved_compensation);

        DocumentUndo::done(_dialog.getDesktop()->getDocument(), SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
                           _("Arrange connector network"));
    }
};

class ActionExchangePositions : public Action {
public:
    enum SortOrder {
	None,
	ZOrder,
	Clockwise
    };

    ActionExchangePositions(Glib::ustring const &id,
                         Glib::ustring const &tiptext,
                         guint row,
                         guint column,
                         AlignAndDistribute &dialog, SortOrder order = None) :
        Action(id, tiptext, row, column,
               dialog.rearrange_table(), dialog),
        sortOrder(order)
    {};


private :
    const SortOrder sortOrder;
    static boost::optional<Geom::Point> center;

    static bool sort_compare(const SPItem * a,const SPItem * b) {
        if (a == NULL) return false;
        if (b == NULL) return true;
        if (center) {
            Geom::Point point_a = a->getCenter() - (*center);
            Geom::Point point_b = b->getCenter() - (*center);
            // First criteria: Sort according to the angle to the center point
            double angle_a = atan2(double(point_a[Geom::Y]), double(point_a[Geom::X]));
            double angle_b = atan2(double(point_b[Geom::Y]), double(point_b[Geom::X]));
            if (angle_a != angle_b) return (angle_a < angle_b);
            // Second criteria: Sort according to the distance the center point
            Geom::Coord length_a = point_a.length();
            Geom::Coord length_b = point_b.length();
            if (length_a != length_b) return (length_a > length_b);
        }
        // Last criteria: Sort according to the z-coordinate
        return sp_item_repr_compare_position(a,b)<0;
    }

    virtual void on_button_click()
    {
        SPDesktop *desktop = _dialog.getDesktop();
        if (!desktop) return;

        Inkscape::Selection *selection = desktop->getSelection();
        if (!selection) return;

        std::vector<SPItem*> selected(selection->itemList());
        if (selected.empty()) return;

        //Check 2 or more selected objects
        if (selected.size() < 2) return;

        // see comment in ActionAlign above
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        int saved_compensation = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
        prefs->setInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);

        // sort the list
	if (sortOrder != None) {
		if (sortOrder == Clockwise) {
			center = selection->center();
		} else { // sorting by ZOrder is outomatically done by not setting the center
			center.reset();
		}
		sort(selected.begin(),selected.end(),sort_compare);
	}
	std::vector<SPItem*>::iterator it(selected.begin());
	SPItem* item = *it;
	Geom::Point p1 =  item->getCenter();
	for (++it ;it != selected.end(); ++it)
	{
		item = *it;
		Geom::Point p2 = item->getCenter();
		Geom::Point delta = p1 - p2;
		sp_item_move_rel(item,Geom::Translate(delta[Geom::X],delta[Geom::Y] ));
		p1 = p2;
	}
	Geom::Point p2 = selected.front()->getCenter();
	Geom::Point delta = p1 - p2;
	sp_item_move_rel(selected.front(),Geom::Translate(delta[Geom::X],delta[Geom::Y] ));

        // restore compensation setting
        prefs->setInt("/options/clonecompensation/value", saved_compensation);

        DocumentUndo::done(_dialog.getDesktop()->getDocument(), SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
                           _("Exchange Positions"));
    }
};

// instantiae the private static member
boost::optional<Geom::Point> ActionExchangePositions::center;

class ActionUnclump : public Action {
public :
    ActionUnclump(const Glib::ustring &id,
               const Glib::ustring &tiptext,
               guint row,
               guint column,
               AlignAndDistribute &dialog):
        Action(id, tiptext, row, column,
               dialog.rearrange_table(), dialog)
    {}

private :
    virtual void on_button_click()
    {
        if (!_dialog.getDesktop()) return;

        // see comment in ActionAlign above
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        int saved_compensation = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
        prefs->setInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
        std::vector<SPItem*> x(_dialog.getDesktop()->getSelection()->itemList());
        unclump (x);

        // restore compensation setting
        prefs->setInt("/options/clonecompensation/value", saved_compensation);

        DocumentUndo::done(_dialog.getDesktop()->getDocument(), SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
                           _("Unclump"));
    }
};

class ActionRandomize : public Action {
public :
    ActionRandomize(const Glib::ustring &id,
               const Glib::ustring &tiptext,
               guint row,
               guint column,
               AlignAndDistribute &dialog):
        Action(id, tiptext, row, column,
               dialog.rearrange_table(), dialog)
    {}

private :
    virtual void on_button_click()
    {
        SPDesktop *desktop = _dialog.getDesktop();
        if (!desktop) return;

        Inkscape::Selection *selection = desktop->getSelection();
        if (!selection) return;

        std::vector<SPItem*> selected(selection->itemList());
        if (selected.empty()) return;

        //Check 2 or more selected objects
        if (selected.size() < 2) return;

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        int prefs_bbox = prefs->getBool("/tools/bounding_box");
        Geom::OptRect sel_bbox = !prefs_bbox ? selection->visualBounds() : selection->geometricBounds();
        if (!sel_bbox) {
            return;
        }

        // This bbox is cached between calls to randomize, so that there's no growth nor shrink
        // nor drift on sequential randomizations. Discard cache on global (or better active
        // desktop's) selection_change signal.
        if (!_dialog.randomize_bbox) {
            _dialog.randomize_bbox = *sel_bbox;
        }

        // see comment in ActionAlign above
        int saved_compensation = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
        prefs->setInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);

        for (std::vector<SPItem*>::iterator it(selected.begin());
            it != selected.end();
            ++it)
        {
        	SPItem* item = *it;
            desktop->getDocument()->ensureUpToDate();
            Geom::OptRect item_box = !prefs_bbox ? (item)->desktopVisualBounds() : (item)->desktopGeometricBounds();
            if (item_box) {
                // find new center, staying within bbox
                double x = _dialog.randomize_bbox->min()[Geom::X] + (*item_box)[Geom::X].extent() /2 +
                    g_random_double_range (0, (*_dialog.randomize_bbox)[Geom::X].extent() - (*item_box)[Geom::X].extent());
                double y = _dialog.randomize_bbox->min()[Geom::Y] + (*item_box)[Geom::Y].extent()/2 +
                    g_random_double_range (0, (*_dialog.randomize_bbox)[Geom::Y].extent() - (*item_box)[Geom::Y].extent());
                // displacement is the new center minus old:
                Geom::Point t = Geom::Point (x, y) - 0.5*(item_box->max() + item_box->min());
                sp_item_move_rel(item, Geom::Translate(t));
            }
        }

        // restore compensation setting
        prefs->setInt("/options/clonecompensation/value", saved_compensation);

        DocumentUndo::done(desktop->getDocument(), SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
                           _("Randomize positions"));
    }
};

struct Baselines
{
    SPItem *_item;
    Geom::Point _base;
    Geom::Dim2 _orientation;
    Baselines(SPItem *item, Geom::Point base, Geom::Dim2 orientation) :
        _item (item),
        _base (base),
        _orientation (orientation)
    {}
};

static bool operator< (const Baselines &a, const Baselines &b)
{
    return (a._base[a._orientation] < b._base[b._orientation]);
}

class ActionBaseline : public Action {
public :
    ActionBaseline(const Glib::ustring &id,
               const Glib::ustring &tiptext,
               guint row,
               guint column,
               AlignAndDistribute &dialog,
#if WITH_GTKMM_3_0
               Gtk::Grid &table,
#else
               Gtk::Table &table,
#endif
               Geom::Dim2 orientation, bool distribute):
        Action(id, tiptext, row, column,
               table, dialog),
        _orientation(orientation),
        _distribute(distribute)
    {}

private :
    Geom::Dim2 _orientation;
    bool _distribute;
    virtual void on_button_click()
    {
        SPDesktop *desktop = _dialog.getDesktop();
        if (!desktop) return;

        Inkscape::Selection *selection = desktop->getSelection();
        if (!selection) return;

        std::vector<SPItem*> selected(selection->itemList());
        if (selected.empty()) return;

        //Check 2 or more selected objects
        if (selected.size() < 2) return;

        Geom::Point b_min = Geom::Point (HUGE_VAL, HUGE_VAL);
        Geom::Point b_max = Geom::Point (-HUGE_VAL, -HUGE_VAL);

        std::vector<Baselines> sorted;

        for (std::vector<SPItem*>::iterator it(selected.begin());
            it != selected.end();
            ++it)
        {
        	SPItem* item = *it;
            if (SP_IS_TEXT (item) || SP_IS_FLOWTEXT (item)) {
                Inkscape::Text::Layout const *layout = te_get_layout(item);
                boost::optional<Geom::Point> pt = layout->baselineAnchorPoint();
                if (pt) {
                    Geom::Point base = *pt * (item)->i2dt_affine();
                    if (base[Geom::X] < b_min[Geom::X]) b_min[Geom::X] = base[Geom::X];
                    if (base[Geom::Y] < b_min[Geom::Y]) b_min[Geom::Y] = base[Geom::Y];
                    if (base[Geom::X] > b_max[Geom::X]) b_max[Geom::X] = base[Geom::X];
                    if (base[Geom::Y] > b_max[Geom::Y]) b_max[Geom::Y] = base[Geom::Y];
                    Baselines b (item, base, _orientation);
                    sorted.push_back(b);
                }
            }
        }

        if (sorted.size() <= 1) return;

        //sort baselines
        std::sort(sorted.begin(), sorted.end());

        bool changed = false;

        if (_distribute) {
            double step = (b_max[_orientation] - b_min[_orientation])/(sorted.size() - 1);
            for (unsigned int i = 0; i < sorted.size(); i++) {
                SPItem *item = sorted[i]._item;
                Geom::Point base = sorted[i]._base;
                Geom::Point t(0.0, 0.0);
                t[_orientation] = b_min[_orientation] + step * i - base[_orientation];
                sp_item_move_rel(item, Geom::Translate(t));
                changed = true;
            }

            if (changed) {
                DocumentUndo::done(desktop->getDocument(), SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
                                    _("Distribute text baselines"));
            }

        } else {
            for (std::vector<SPItem*>::iterator it(selected.begin());
                 it != selected.end();
                 ++it)
            {
            	SPItem* item = *it;
                if (SP_IS_TEXT (item) || SP_IS_FLOWTEXT (item)) {
                    Inkscape::Text::Layout const *layout = te_get_layout(item);
                    boost::optional<Geom::Point> pt = layout->baselineAnchorPoint();
                    if (pt) {
                        Geom::Point base = *pt * (item)->i2dt_affine();
                        Geom::Point t(0.0, 0.0);
                        t[_orientation] = b_min[_orientation] - base[_orientation];
                        sp_item_move_rel(item, Geom::Translate(t));
                        changed = true;
                    }
                }
            }

            if (changed) {
                DocumentUndo::done(desktop->getDocument(), SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
                                   _("Align text baselines"));
            }
        }
    }
};



static void on_tool_changed(AlignAndDistribute *daad)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop && desktop->getEventContext())
        daad->setMode(tools_active(desktop) == TOOLS_NODES);
}

static void on_selection_changed(AlignAndDistribute *daad)
{
    daad->randomize_bbox = Geom::OptRect();
}

/////////////////////////////////////////////////////////




AlignAndDistribute::AlignAndDistribute()
    : UI::Widget::Panel ("", "/dialogs/align", SP_VERB_DIALOG_ALIGN_DISTRIBUTE),
      randomize_bbox(),
      _alignFrame(_("Align")),
      _distributeFrame(_("Distribute")),
      _rearrangeFrame(_("Rearrange")),
      _removeOverlapFrame(_("Remove overlaps")),
      _nodesFrame(_("Nodes")),
#if WITH_GTKMM_3_0
      _alignTable(),
      _distributeTable(),
      _rearrangeTable(),
      _removeOverlapTable(),
      _nodesTable(),
#else
      _alignTable(2, 6, true),
      _distributeTable(2, 6, true),
      _rearrangeTable(1, 5, false),
      _removeOverlapTable(1, 5, false),
      _nodesTable(1, 4, true),
#endif
      _anchorLabel(_("Relative to: ")),
      _selgrpLabel(_("_Treat selection as group: "), 1)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    //Instanciate the align buttons
    addAlignButton(INKSCAPE_ICON("align-horizontal-right-to-anchor"),
                   _("Align right edges of objects to the left edge of the anchor"),
                   0, 0);
    addAlignButton(INKSCAPE_ICON("align-horizontal-left"),
                   _("Align left edges"),
                   0, 1);
    addAlignButton(INKSCAPE_ICON("align-horizontal-center"),
                   _("Center on vertical axis"),
                   0, 2);
    addAlignButton(INKSCAPE_ICON("align-horizontal-right"),
                   _("Align right sides"),
                   0, 3);
    addAlignButton(INKSCAPE_ICON("align-horizontal-left-to-anchor"),
                   _("Align left edges of objects to the right edge of the anchor"),
                   0, 4);
    addAlignButton(INKSCAPE_ICON("align-vertical-bottom-to-anchor"),
                   _("Align bottom edges of objects to the top edge of the anchor"),
                   1, 0);
    addAlignButton(INKSCAPE_ICON("align-vertical-top"),
                   _("Align top edges"),
                   1, 1);
    addAlignButton(INKSCAPE_ICON("align-vertical-center"),
                   _("Center on horizontal axis"),
                   1, 2);
    addAlignButton(INKSCAPE_ICON("align-vertical-bottom"),
                   _("Align bottom edges"),
                   1, 3);
    addAlignButton(INKSCAPE_ICON("align-vertical-top-to-anchor"),
                   _("Align top edges of objects to the bottom edge of the anchor"),
                   1, 4);

    //Baseline aligns
    addBaselineButton(INKSCAPE_ICON("align-horizontal-baseline"),
                   _("Align baseline anchors of texts horizontally"),
                      0, 5, this->align_table(), Geom::X, false);
    addBaselineButton(INKSCAPE_ICON("align-vertical-baseline"),
                   _("Align baselines of texts"),
                     1, 5, this->align_table(), Geom::Y, false);

    //The distribute buttons
    addDistributeButton(INKSCAPE_ICON("distribute-horizontal-gaps"),
                        _("Make horizontal gaps between objects equal"),
                        0, 4, true, Geom::X, .5, .5);

    addDistributeButton(INKSCAPE_ICON("distribute-horizontal-left"),
                        _("Distribute left edges equidistantly"),
                        0, 1, false, Geom::X, 1., 0.);
    addDistributeButton(INKSCAPE_ICON("distribute-horizontal-center"),
                        _("Distribute centers equidistantly horizontally"),
                        0, 2, false, Geom::X, .5, .5);
    addDistributeButton(INKSCAPE_ICON("distribute-horizontal-right"),
                        _("Distribute right edges equidistantly"),
                        0, 3, false, Geom::X, 0., 1.);

    addDistributeButton(INKSCAPE_ICON("distribute-vertical-gaps"),
                        _("Make vertical gaps between objects equal"),
                        1, 4, true, Geom::Y, .5, .5);

    addDistributeButton(INKSCAPE_ICON("distribute-vertical-top"),
                        _("Distribute top edges equidistantly"),
                        1, 1, false, Geom::Y, 0, 1);
    addDistributeButton(INKSCAPE_ICON("distribute-vertical-center"),
                        _("Distribute centers equidistantly vertically"),
                        1, 2, false, Geom::Y, .5, .5);
    addDistributeButton(INKSCAPE_ICON("distribute-vertical-bottom"),
                        _("Distribute bottom edges equidistantly"),
                        1, 3, false, Geom::Y, 1., 0.);

    //Baseline distribs
    addBaselineButton(INKSCAPE_ICON("distribute-horizontal-baseline"),
                   _("Distribute baseline anchors of texts horizontally"),
                      0, 5, this->distribute_table(), Geom::X, true);
    addBaselineButton(INKSCAPE_ICON("distribute-vertical-baseline"),
                   _("Distribute baselines of texts vertically"),
                     1, 5, this->distribute_table(), Geom::Y, true);

    // Rearrange
    //Graph Layout
    addGraphLayoutButton(INKSCAPE_ICON("distribute-graph"),
                            _("Nicely arrange selected connector network"),
                            0, 0);
    addExchangePositionsButton(INKSCAPE_ICON("exchange-positions"),
                            _("Exchange positions of selected objects - selection order"),
                            0, 1);
    addExchangePositionsByZOrderButton(INKSCAPE_ICON("exchange-positions-zorder"),
                            _("Exchange positions of selected objects - stacking order"),
                            0, 2);
    addExchangePositionsClockwiseButton(INKSCAPE_ICON("exchange-positions-clockwise"),
                            _("Exchange positions of selected objects - clockwise rotate"),
                            0, 3);

    //Randomize & Unclump
    addRandomizeButton(INKSCAPE_ICON("distribute-randomize"),
                        _("Randomize centers in both dimensions"),
                        0, 4);
    addUnclumpButton(INKSCAPE_ICON("distribute-unclump"),
                        _("Unclump objects: try to equalize edge-to-edge distances"),
                        0, 5);

    //Remove overlaps
    addRemoveOverlapsButton(INKSCAPE_ICON("distribute-remove-overlaps"),
                            _("Move objects as little as possible so that their bounding boxes do not overlap"),
                            0, 0);

    //Node Mode buttons
    // NOTE: "align nodes vertically" means "move nodes vertically until they align on a common
    // _horizontal_ line". This is analogous to what the "align-vertical-center" icon means.
    // There is no doubt some ambiguity. For this reason the descriptions are different.
    addNodeButton(INKSCAPE_ICON("align-vertical-node"),
                  _("Align selected nodes to a common horizontal line"),
                  0, Geom::X, false);
    addNodeButton(INKSCAPE_ICON("align-horizontal-node"),
                  _("Align selected nodes to a common vertical line"),
                  1, Geom::Y, false);
    addNodeButton(INKSCAPE_ICON("distribute-horizontal-node"),
                  _("Distribute selected nodes horizontally"),
                  2, Geom::X, true);
    addNodeButton(INKSCAPE_ICON("distribute-vertical-node"),
                  _("Distribute selected nodes vertically"),
                  3, Geom::Y, true);

    //Rest of the widgetry

    _combo.append(_("Last selected"));
    _combo.append(_("First selected"));
    _combo.append(_("Biggest object"));
    _combo.append(_("Smallest object"));
    _combo.append(_("Page"));
    _combo.append(_("Drawing"));
    _combo.append(_("Selection Area"));
    _combo.set_active(prefs->getInt("/dialogs/align/align-to", 6));
    _combo.signal_changed().connect(sigc::mem_fun(*this, &AlignAndDistribute::on_ref_change));

    _anchorBox.pack_end(_combo, false, false);
    _anchorBox.pack_end(_anchorLabel, false, false);

    _selgrpLabel.set_mnemonic_widget(_selgrp);
    _selgrpBox.pack_end(_selgrp, false, false);
    _selgrpBox.pack_end(_selgrpLabel, false, false);
    _selgrp.set_active(prefs->getBool("/dialogs/align/sel-as-groups"));
    _selgrp.signal_toggled().connect(sigc::mem_fun(*this, &AlignAndDistribute::on_selgrp_toggled));

    // Right align the buttons
    _alignTableBox.pack_end(_alignTable, false, false);
    _distributeTableBox.pack_end(_distributeTable, false, false);
    _rearrangeTableBox.pack_end(_rearrangeTable, false, false);
    _removeOverlapTableBox.pack_end(_removeOverlapTable, false, false);
    _nodesTableBox.pack_end(_nodesTable, false, false);

    _alignBox.pack_start(_anchorBox);
    _alignBox.pack_start(_selgrpBox);
    _alignBox.pack_start(_alignTableBox);

    _alignFrame.add(_alignBox);
    _distributeFrame.add(_distributeTableBox);
    _rearrangeFrame.add(_rearrangeTableBox);
    _removeOverlapFrame.add(_removeOverlapTableBox);
    _nodesFrame.add(_nodesTableBox);

    Gtk::Box *contents = _getContents();
    contents->set_spacing(4);

    // Notebook for individual transformations

    contents->pack_start(_alignFrame, true, true);
    contents->pack_start(_distributeFrame, true, true);
    contents->pack_start(_rearrangeFrame, true, true);
    contents->pack_start(_removeOverlapFrame, true, true);
    contents->pack_start(_nodesFrame, true, true);

    //Connect to the global tool change signal
    _toolChangeConn = INKSCAPE.signal_eventcontext_set.connect(sigc::hide<0>(sigc::bind(sigc::ptr_fun(&on_tool_changed), this)));

    // Connect to the global selection change, to invalidate cached randomize_bbox
    _selChangeConn = INKSCAPE.signal_selection_changed.connect(sigc::hide<0>(sigc::bind(sigc::ptr_fun(&on_selection_changed), this)));
    randomize_bbox = Geom::OptRect();

    _desktopChangeConn = _deskTrack.connectDesktopChanged( sigc::mem_fun(*this, &AlignAndDistribute::setDesktop) );
    _deskTrack.connect(GTK_WIDGET(gobj()));

    show_all_children();

    on_tool_changed (this); // set current mode
}

AlignAndDistribute::~AlignAndDistribute()
{
    for (std::list<Action *>::iterator it = _actionList.begin();
         it != _actionList.end();  ++it) {
        delete *it;
    }

    _toolChangeConn.disconnect();
    _selChangeConn.disconnect();
    _desktopChangeConn.disconnect();
    _deskTrack.disconnect();
}

void AlignAndDistribute::setTargetDesktop(SPDesktop *desktop)
{
    if (_desktop != desktop) {
        _desktop = desktop;
        on_tool_changed (this);
    }
}


void AlignAndDistribute::on_ref_change(){
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt("/dialogs/align/align-to", _combo.get_active_row_number());

    //Make blink the master
}

void AlignAndDistribute::on_selgrp_toggled(){
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt("/dialogs/align/sel-as-groups", _selgrp.get_active());

    //Make blink the master
}




void AlignAndDistribute::setMode(bool nodeEdit)
{
    //Act on widgets used in node mode
    void ( Gtk::Widget::*mNode) ()  = nodeEdit ?
        &Gtk::Widget::show_all : &Gtk::Widget::hide;

    //Act on widgets used in selection mode
  void ( Gtk::Widget::*mSel) ()  = nodeEdit ?
      &Gtk::Widget::hide : &Gtk::Widget::show_all;

    ((_alignFrame).*(mSel))();
    ((_distributeFrame).*(mSel))();
    ((_rearrangeFrame).*(mSel))();
    ((_removeOverlapFrame).*(mSel))();
    ((_nodesFrame).*(mNode))();

}
void AlignAndDistribute::addAlignButton(const Glib::ustring &id, const Glib::ustring tiptext,
                                 guint row, guint col)
{
    _actionList.push_back(
        new ActionAlign(
            id, tiptext, row, col,
            *this , col + row * 5));
}
void AlignAndDistribute::addDistributeButton(const Glib::ustring &id, const Glib::ustring tiptext,
                                      guint row, guint col, bool onInterSpace,
                                      Geom::Dim2 orientation, float kBegin, float kEnd)
{
    _actionList.push_back(
        new ActionDistribute(
            id, tiptext, row, col, *this ,
            onInterSpace, orientation,
            kBegin, kEnd
            )
        );
}

void AlignAndDistribute::addNodeButton(const Glib::ustring &id, const Glib::ustring tiptext,
                   guint col, Geom::Dim2 orientation, bool distribute)
{
    _actionList.push_back(
        new ActionNode(
            id, tiptext, col,
            *this, orientation, distribute));
}

void AlignAndDistribute::addRemoveOverlapsButton(const Glib::ustring &id, const Glib::ustring tiptext,
                                      guint row, guint col)
{
    _actionList.push_back(
        new ActionRemoveOverlaps(
            id, tiptext, row, col, *this)
        );
}

void AlignAndDistribute::addGraphLayoutButton(const Glib::ustring &id, const Glib::ustring tiptext,
                                      guint row, guint col)
{
    _actionList.push_back(
        new ActionGraphLayout(
            id, tiptext, row, col, *this)
        );
}

void AlignAndDistribute::addExchangePositionsButton(const Glib::ustring &id, const Glib::ustring tiptext,
                                      guint row, guint col)
{
    _actionList.push_back(
        new ActionExchangePositions(
            id, tiptext, row, col, *this)
        );
}

void AlignAndDistribute::addExchangePositionsByZOrderButton(const Glib::ustring &id, const Glib::ustring tiptext,
                                      guint row, guint col)
{
    _actionList.push_back(
        new ActionExchangePositions(
            id, tiptext, row, col, *this, ActionExchangePositions::ZOrder)
        );
}

void AlignAndDistribute::addExchangePositionsClockwiseButton(const Glib::ustring &id, const Glib::ustring tiptext,
                                      guint row, guint col)
{
    _actionList.push_back(
        new ActionExchangePositions(
            id, tiptext, row, col, *this, ActionExchangePositions::Clockwise)
        );
}

void AlignAndDistribute::addUnclumpButton(const Glib::ustring &id, const Glib::ustring tiptext,
                                      guint row, guint col)
{
    _actionList.push_back(
        new ActionUnclump(
            id, tiptext, row, col, *this)
        );
}

void AlignAndDistribute::addRandomizeButton(const Glib::ustring &id, const Glib::ustring tiptext,
                                      guint row, guint col)
{
    _actionList.push_back(
        new ActionRandomize(
            id, tiptext, row, col, *this)
        );
}

#if WITH_GTKMM_3_0
void AlignAndDistribute::addBaselineButton(const Glib::ustring &id, const Glib::ustring tiptext,
                                    guint row, guint col, Gtk::Grid &table, Geom::Dim2 orientation, bool distribute)
#else
void AlignAndDistribute::addBaselineButton(const Glib::ustring &id, const Glib::ustring tiptext,
                                    guint row, guint col, Gtk::Table &table, Geom::Dim2 orientation, bool distribute)
#endif
{
    _actionList.push_back(
        new ActionBaseline(
            id, tiptext, row, col,
            *this, table, orientation, distribute));
}

} // namespace Dialog
} // namespace UI
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
