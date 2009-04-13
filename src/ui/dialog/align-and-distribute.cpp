/** @file
 * @brief Align and Distribute dialog - implementation
 */
/* Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Aubanel MONNIER <aubi@libertysurf.fr>
 *   Frank Felfe <innerspace@iname.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 1999-2004, 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/spinbutton.h>

#include "desktop-handles.h"
#include "unclump.h"
#include "document.h"
#include "enums.h"
#include "graphlayout/graphlayout.h"
#include "inkscape.h"
#include "macros.h"
#include "node-context.h"  //For access to ShapeEditor
#include "preferences.h"
#include "removeoverlap/removeoverlap.h"
#include "selection.h"
#include "shape-editor.h" //For node align/distribute methods
#include "sp-flowtext.h"
#include "sp-item-transform.h"
#include "sp-text.h"
#include "text-editing.h"
#include "tools-switch.h"
#include "ui/icon-names.h"
#include "util/glib-list-iterators.h"
#include "verbs.h"
#include "widgets/icon.h"

#include "align-and-distribute.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

/////////helper classes//////////////////////////////////

class Action {
public :
    Action(const Glib::ustring &id,
           const Glib::ustring &tiptext,
           guint row, guint column,
           Gtk::Table &parent,
           Gtk::Tooltips &tooltips,
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
        tooltips.set_tip(*pButton, tiptext);
        parent.attach(*pButton, column, column+1, row, row+1, Gtk::FILL, Gtk::FILL);
    }
    virtual ~Action(){}

    AlignAndDistribute &_dialog;

private :
    virtual void on_button_click(){}

    Glib::ustring _id;
    Gtk::Table &_parent;
};


class ActionAlign : public Action {
public :
    struct Coeffs {
       double mx0, mx1, my0, my1;
       double sx0, sx1, sy0, sy1;
    };
    ActionAlign(const Glib::ustring &id,
                const Glib::ustring &tiptext,
                guint row, guint column,
                AlignAndDistribute &dialog,
                guint coeffIndex):
        Action(id, tiptext, row, column,
               dialog.align_table(), dialog.tooltips(), dialog),
        _index(coeffIndex),
        _dialog(dialog)
    {}

private :

    virtual void on_button_click() {
        //Retreive selected objects
        SPDesktop *desktop = _dialog.getDesktop();
        if (!desktop) return;

        Inkscape::Selection *selection = sp_desktop_selection(desktop);
        if (!selection) return;

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        bool sel_as_group = prefs->getBool("/dialogs/align/sel-as-groups");

        using Inkscape::Util::GSListConstIterator;
        std::list<SPItem *> selected;
        selected.insert<GSListConstIterator<SPItem *> >(selected.end(), selection->itemList(), NULL);
        if (selected.empty()) return;

        Geom::Point mp; //Anchor point
        AlignAndDistribute::AlignTarget target = _dialog.getAlignTarget();
        const Coeffs &a= _allCoeffs[_index];
        switch (target)
        {
        case AlignAndDistribute::LAST:
        case AlignAndDistribute::FIRST:
        case AlignAndDistribute::BIGGEST:
        case AlignAndDistribute::SMALLEST:
        {
            //Check 2 or more selected objects
            std::list<SPItem *>::iterator second(selected.begin());
            ++second;
            if (second == selected.end())
                return;
            //Find the master (anchor on which the other objects are aligned)
            std::list<SPItem *>::iterator master(
                _dialog.find_master (
                    selected,
                    (a.mx0 != 0.0) ||
                    (a.mx1 != 0.0) )
                );
            //remove the master from the selection
            SPItem * thing = *master;
            // TODO: either uncomment or remove the following commented lines, depending on which
            //       behaviour of moving objects makes most sense; also cf. discussion at
            //       https://bugs.launchpad.net/inkscape/+bug/255933
            /*if (!sel_as_group) { */
                selected.erase(master);
            /*}*/
            //Compute the anchor point
            Geom::OptRect b = sp_item_bbox_desktop (thing);
            if (b) {
                mp = Geom::Point(a.mx0 * b->min()[Geom::X] + a.mx1 * b->max()[Geom::X],
                               a.my0 * b->min()[Geom::Y] + a.my1 * b->max()[Geom::Y]);
            } else {
                return;
            }
            break;
        }

        case AlignAndDistribute::PAGE:
            mp = Geom::Point(a.mx1 * sp_document_width(sp_desktop_document(desktop)),
                           a.my1 * sp_document_height(sp_desktop_document(desktop)));
            break;

        case AlignAndDistribute::DRAWING:
        {
            Geom::OptRect b = sp_item_bbox_desktop
                ( (SPItem *) sp_document_root (sp_desktop_document (desktop)) );
            if (b) {
                mp = Geom::Point(a.mx0 * b->min()[Geom::X] + a.mx1 * b->max()[Geom::X],
                               a.my0 * b->min()[Geom::Y] + a.my1 * b->max()[Geom::Y]);
            } else {
                return;
            }
            break;
        }

        case AlignAndDistribute::SELECTION:
        {
            Geom::OptRect b =  selection->bounds();
            if (b) {
                mp = Geom::Point(a.mx0 * b->min()[Geom::X] + a.mx1 * b->max()[Geom::X],
                               a.my0 * b->min()[Geom::Y] + a.my1 * b->max()[Geom::Y]);
            } else {
                return;
            }
            break;
        }

        default:
            g_assert_not_reached ();
            break;
        };  // end of switch

        // Top hack: temporarily set clone compensation to unmoved, so that we can align/distribute
        // clones with their original (and the move of the original does not disturb the
        // clones). The only problem with this is that if there are outside-of-selection clones of
        // a selected original, they will be unmoved too, possibly contrary to user's
        // expecation. However this is a minor point compared to making align/distribute always
        // work as expected, and "unmoved" is the default option anyway.
        int saved_compensation = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
        prefs->setInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);

        bool changed = false;
        Geom::OptRect b;
        if (sel_as_group)
            b = selection->bounds();

        //Move each item in the selected list separately
        for (std::list<SPItem *>::iterator it(selected.begin());
             it != selected.end();
             it++)
        {
            sp_document_ensure_up_to_date(sp_desktop_document (desktop));
            if (!sel_as_group)
                b = sp_item_bbox_desktop (*it);
            if (b) {
                Geom::Point const sp(a.sx0 * b->min()[Geom::X] + a.sx1 * b->max()[Geom::X],
                                     a.sy0 * b->min()[Geom::Y] + a.sy1 * b->max()[Geom::Y]);
                Geom::Point const mp_rel( mp - sp );
                if (LInfty(mp_rel) > 1e-9) {
                    sp_item_move_rel(*it, Geom::Translate(mp_rel));
                    changed = true;
                }
            }
        }

        // restore compensation setting
        prefs->setInt("/options/clonecompensation/value", saved_compensation);

        if (changed) {
            sp_document_done ( sp_desktop_document (desktop) , SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
                               _("Align"));
        }


    }
    guint _index;
    AlignAndDistribute &_dialog;

    static const Coeffs _allCoeffs[10];

};
ActionAlign::Coeffs const ActionAlign::_allCoeffs[10] = {
    {1., 0., 0., 0., 0., 1., 0., 0.},
    {1., 0., 0., 0., 1., 0., 0., 0.},
    {.5, .5, 0., 0., .5, .5, 0., 0.},
    {0., 1., 0., 0., 0., 1., 0., 0.},
    {0., 1., 0., 0., 1., 0., 0., 0.},
    {0., 0., 0., 1., 0., 0., 1., 0.},
    {0., 0., 0., 1., 0., 0., 0., 1.},
    {0., 0., .5, .5, 0., 0., .5, .5},
    {0., 0., 1., 0., 0., 0., 1., 0.},
    {0., 0., 1., 0., 0., 0., 0., 1.}
};

BBoxSort::BBoxSort(SPItem *pItem, Geom::Rect bounds, Geom::Dim2 orientation, double kBegin, double kEnd) :
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
               dialog.distribute_table(), dialog.tooltips(), dialog),
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

        Inkscape::Selection *selection = sp_desktop_selection(desktop);
        if (!selection) return;

        using Inkscape::Util::GSListConstIterator;
        std::list<SPItem *> selected;
        selected.insert<GSListConstIterator<SPItem *> >(selected.end(), selection->itemList(), NULL);
        if (selected.empty()) return;

        //Check 2 or more selected objects
        std::list<SPItem *>::iterator second(selected.begin());
        ++second;
        if (second == selected.end()) return;


        std::vector< BBoxSort  > sorted;
        for (std::list<SPItem *>::iterator it(selected.begin());
            it != selected.end();
            ++it)
        {
            Geom::OptRect bbox = sp_item_bbox_desktop(*it);
            if (bbox) {
                sorted.push_back(BBoxSort(*it, *bbox, _orientation, _kBegin, _kEnd));
            }
        }
        //sort bbox by anchors
        std::sort(sorted.begin(), sorted.end());

        // see comment in ActionAlign above
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
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
                  it ++ )
            {
                if (!NR_DF_TEST_CLOSE (pos, it->bbox.min()[_orientation], 1e-6)) {
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
                if (!NR_DF_TEST_CLOSE (pos, it.anchor, 1e-6)) {
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
            sp_document_done ( sp_desktop_document (desktop), SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
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
               dialog.nodes_table(), dialog.tooltips(), dialog),
        _orientation(orientation),
        _distribute(distribute)
    {}

private :
    Geom::Dim2 _orientation;
    bool _distribute;
    virtual void on_button_click()
    {

        if (!_dialog.getDesktop()) return;
        SPEventContext *event_context = sp_desktop_event_context(_dialog.getDesktop());
        if (!SP_IS_NODE_CONTEXT (event_context)) return ;

        if (_distribute)
            event_context->shape_editor->distribute((Geom::Dim2)_orientation);
        else
            event_context->shape_editor->align((Geom::Dim2)_orientation);

    }
};

class ActionRemoveOverlaps : public Action {
private:
    Gtk::Label removeOverlapXGapLabel;
    Gtk::Label removeOverlapYGapLabel;
    Gtk::SpinButton removeOverlapXGap;
    Gtk::SpinButton removeOverlapYGap;

public:
    ActionRemoveOverlaps(Glib::ustring const &id,
                         Glib::ustring const &tiptext,
                         guint row,
                         guint column,
                         AlignAndDistribute &dialog) :
        Action(id, tiptext, row, column + 4,
               dialog.removeOverlap_table(), dialog.tooltips(), dialog)
    {
        dialog.removeOverlap_table().set_col_spacings(3);

        removeOverlapXGap.set_digits(1);
        removeOverlapXGap.set_size_request(60, -1);
        removeOverlapXGap.set_increments(1.0, 0);
        removeOverlapXGap.set_range(-1000.0, 1000.0);
        removeOverlapXGap.set_value(0);
        dialog.tooltips().set_tip(removeOverlapXGap,
                                  _("Minimum horizontal gap (in px units) between bounding boxes"));
        /* TRANSLATORS: Horizontal gap. Only put "H:" equivalent in the translation */
        removeOverlapXGapLabel.set_label(Q_("gap|H:"));

        removeOverlapYGap.set_digits(1);
        removeOverlapYGap.set_size_request(60, -1);
        removeOverlapYGap.set_increments(1.0, 0);
        removeOverlapYGap.set_range(-1000.0, 1000.0);
        removeOverlapYGap.set_value(0);
        dialog.tooltips().set_tip(removeOverlapYGap,
                                  _("Minimum vertical gap (in px units) between bounding boxes"));
        /* TRANSLATORS: Vertical gap */
        removeOverlapYGapLabel.set_label(_("V:"));

        dialog.removeOverlap_table().attach(removeOverlapXGapLabel, column, column+1, row, row+1, Gtk::FILL, Gtk::FILL);
        dialog.removeOverlap_table().attach(removeOverlapXGap, column+1, column+2, row, row+1, Gtk::FILL, Gtk::FILL);
        dialog.removeOverlap_table().attach(removeOverlapYGapLabel, column+2, column+3, row, row+1, Gtk::FILL, Gtk::FILL);
        dialog.removeOverlap_table().attach(removeOverlapYGap, column+3, column+4, row, row+1, Gtk::FILL, Gtk::FILL);

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
        removeoverlap(sp_desktop_selection(_dialog.getDesktop())->itemList(),
                      xGap, yGap);

        // restore compensation setting
        prefs->setInt("/options/clonecompensation/value", saved_compensation);

        sp_document_done(sp_desktop_document(_dialog.getDesktop()), SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
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
        Action(id, tiptext, row, column + 4,
               dialog.graphLayout_table(), dialog.tooltips(), dialog)
    {}

private :
    virtual void on_button_click()
    {
        if (!_dialog.getDesktop()) return;

        // see comment in ActionAlign above
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        int saved_compensation = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
        prefs->setInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);

        graphlayout(sp_desktop_selection(_dialog.getDesktop())->itemList());

        // restore compensation setting
        prefs->setInt("/options/clonecompensation/value", saved_compensation);

        sp_document_done(sp_desktop_document(_dialog.getDesktop()), SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
                         _("Arrange connector network"));
    }
};

class ActionUnclump : public Action {
public :
    ActionUnclump(const Glib::ustring &id,
               const Glib::ustring &tiptext,
               guint row,
               guint column,
               AlignAndDistribute &dialog):
        Action(id, tiptext, row, column,
               dialog.distribute_table(), dialog.tooltips(), dialog)
    {}

private :
    virtual void on_button_click()
    {
        if (!_dialog.getDesktop()) return;

        // see comment in ActionAlign above
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        int saved_compensation = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
        prefs->setInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);

        unclump ((GSList *) sp_desktop_selection(_dialog.getDesktop())->itemList());

        // restore compensation setting
        prefs->setInt("/options/clonecompensation/value", saved_compensation);

        sp_document_done (sp_desktop_document (_dialog.getDesktop()), SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
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
               dialog.distribute_table(), dialog.tooltips(), dialog)
    {}

private :
    virtual void on_button_click()
    {
        SPDesktop *desktop = _dialog.getDesktop();
        if (!desktop) return;

        Inkscape::Selection *selection = sp_desktop_selection(desktop);
        if (!selection) return;

        using Inkscape::Util::GSListConstIterator;
        std::list<SPItem *> selected;
        selected.insert<GSListConstIterator<SPItem *> >(selected.end(), selection->itemList(), NULL);
        if (selected.empty()) return;

        //Check 2 or more selected objects
        if (selected.size() < 2) return;

        Geom::OptRect sel_bbox = selection->bounds();
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
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        int saved_compensation = prefs->getInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);
        prefs->setInt("/options/clonecompensation/value", SP_CLONE_COMPENSATION_UNMOVED);

        for (std::list<SPItem *>::iterator it(selected.begin());
            it != selected.end();
            ++it)
        {
            sp_document_ensure_up_to_date(sp_desktop_document (desktop));
            Geom::OptRect item_box = sp_item_bbox_desktop (*it);
            if (item_box) {
                // find new center, staying within bbox
                double x = _dialog.randomize_bbox->min()[Geom::X] + (*item_box)[Geom::X].extent() /2 +
                    g_random_double_range (0, (*_dialog.randomize_bbox)[Geom::X].extent() - (*item_box)[Geom::X].extent());
                double y = _dialog.randomize_bbox->min()[Geom::Y] + (*item_box)[Geom::Y].extent()/2 +
                    g_random_double_range (0, (*_dialog.randomize_bbox)[Geom::Y].extent() - (*item_box)[Geom::Y].extent());
                // displacement is the new center minus old:
                Geom::Point t = Geom::Point (x, y) - 0.5*(item_box->max() + item_box->min());
                sp_item_move_rel(*it, Geom::Translate(t));
            }
        }

        // restore compensation setting
        prefs->setInt("/options/clonecompensation/value", saved_compensation);

        sp_document_done (sp_desktop_document (desktop), SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
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

bool operator< (const Baselines &a, const Baselines &b)
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
               Gtk::Table &table,
               Geom::Dim2 orientation, bool distribute):
        Action(id, tiptext, row, column,
               table, dialog.tooltips(), dialog),
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

        Inkscape::Selection *selection = sp_desktop_selection(desktop);
        if (!selection) return;

        using Inkscape::Util::GSListConstIterator;
        std::list<SPItem *> selected;
        selected.insert<GSListConstIterator<SPItem *> >(selected.end(), selection->itemList(), NULL);
        if (selected.empty()) return;

        //Check 2 or more selected objects
        if (selected.size() < 2) return;

        Geom::Point b_min = Geom::Point (HUGE_VAL, HUGE_VAL);
        Geom::Point b_max = Geom::Point (-HUGE_VAL, -HUGE_VAL);

        std::vector<Baselines> sorted;

        for (std::list<SPItem *>::iterator it(selected.begin());
            it != selected.end();
            ++it)
        {
            if (SP_IS_TEXT (*it) || SP_IS_FLOWTEXT (*it)) {
                Inkscape::Text::Layout const *layout = te_get_layout(*it);
                Geom::Point base = layout->characterAnchorPoint(layout->begin()) * sp_item_i2d_affine(*it);
                if (base[Geom::X] < b_min[Geom::X]) b_min[Geom::X] = base[Geom::X];
                if (base[Geom::Y] < b_min[Geom::Y]) b_min[Geom::Y] = base[Geom::Y];
                if (base[Geom::X] > b_max[Geom::X]) b_max[Geom::X] = base[Geom::X];
                if (base[Geom::Y] > b_max[Geom::Y]) b_max[Geom::Y] = base[Geom::Y];

                Baselines b (*it, base, _orientation);
                sorted.push_back(b);
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
                sp_document_done (sp_desktop_document (desktop), SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
                                  _("Distribute text baselines"));
            }

        } else {
            for (std::list<SPItem *>::iterator it(selected.begin());
                 it != selected.end();
                 ++it)
            {
                if (SP_IS_TEXT (*it) || SP_IS_FLOWTEXT (*it)) {
                    Inkscape::Text::Layout const *layout = te_get_layout(*it);
                    Geom::Point base = layout->characterAnchorPoint(layout->begin()) * sp_item_i2d_affine(*it);
                    Geom::Point t(0.0, 0.0);
                    t[_orientation] = b_min[_orientation] - base[_orientation];
                    sp_item_move_rel(*it, Geom::Translate(t));
                    changed = true;
                }
            }

            if (changed) {
                sp_document_done (sp_desktop_document (desktop), SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
                                  _("Align text baselines"));
            }
        }
    }
};



void on_tool_changed(Inkscape::Application */*inkscape*/, SPEventContext */*context*/, AlignAndDistribute *daad)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop && sp_desktop_event_context(desktop))
        daad->setMode(tools_active(desktop) == TOOLS_NODES);
}

void on_selection_changed(Inkscape::Application */*inkscape*/, Inkscape::Selection */*selection*/, AlignAndDistribute *daad)
{
    daad->randomize_bbox = Geom::OptRect();
}

/////////////////////////////////////////////////////////




AlignAndDistribute::AlignAndDistribute()
    : UI::Widget::Panel ("", "/dialogs/align", SP_VERB_DIALOG_ALIGN_DISTRIBUTE),
      randomize_bbox(),
      _alignFrame(_("Align")),
      _distributeFrame(_("Distribute")),
      _removeOverlapFrame(_("Remove overlaps")),
      _graphLayoutFrame(_("Connector network layout")),
      _nodesFrame(_("Nodes")),
      _alignTable(2, 6, true),
      _distributeTable(3, 6, true),
      _removeOverlapTable(1, 5, false),
      _graphLayoutTable(1, 5, false),
      _nodesTable(1, 4, true),
      _anchorLabel(_("Relative to: ")),
      _selgrpLabel(_("Treat selection as group: "))
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    //Instanciate the align buttons
    addAlignButton(INKSCAPE_ICON_ALIGN_HORIZONTAL_RIGHT_TO_ANCHOR,
                   _("Align right edges of objects to the left edge of the anchor"),
                   0, 0);
    addAlignButton(INKSCAPE_ICON_ALIGN_HORIZONTAL_LEFT,
                   _("Align left edges"),
                   0, 1);
    addAlignButton(INKSCAPE_ICON_ALIGN_HORIZONTAL_CENTER,
                   _("Center objects horizontally"),
                   0, 2);
    addAlignButton(INKSCAPE_ICON_ALIGN_HORIZONTAL_RIGHT,
                   _("Align right sides"),
                   0, 3);
    addAlignButton(INKSCAPE_ICON_ALIGN_HORIZONTAL_LEFT_TO_ANCHOR,
                   _("Align left edges of objects to the right edge of the anchor"),
                   0, 4);
    addAlignButton(INKSCAPE_ICON_ALIGN_VERTICAL_BOTTOM_TO_ANCHOR,
                   _("Align bottom edges of objects to the top edge of the anchor"),
                   1, 0);
    addAlignButton(INKSCAPE_ICON_ALIGN_VERTICAL_TOP,
                   _("Align top edges"),
                   1, 1);
    addAlignButton(INKSCAPE_ICON_ALIGN_VERTICAL_CENTER,
                   _("Center on horizontal axis"),
                   1, 2);
    addAlignButton(INKSCAPE_ICON_ALIGN_VERTICAL_BOTTOM,
                   _("Align bottom edges"),
                   1, 3);
    addAlignButton(INKSCAPE_ICON_ALIGN_VERTICAL_TOP_TO_ANCHOR,
                   _("Align top edges of objects to the bottom edge of the anchor"),
                   1, 4);

    //Baseline aligns
    addBaselineButton(INKSCAPE_ICON_ALIGN_HORIZONTAL_BASELINE,
                   _("Align baseline anchors of texts horizontally"),
                      0, 5, this->align_table(), Geom::X, false);
    addBaselineButton(INKSCAPE_ICON_ALIGN_VERTICAL_BASELINE,
                   _("Align baselines of texts"),
                     1, 5, this->align_table(), Geom::Y, false);

    //The distribute buttons
    addDistributeButton(INKSCAPE_ICON_DISTRIBUTE_HORIZONTAL_GAPS,
                        _("Make horizontal gaps between objects equal"),
                        0, 4, true, Geom::X, .5, .5);

    addDistributeButton(INKSCAPE_ICON_DISTRIBUTE_HORIZONTAL_LEFT,
                        _("Distribute left edges equidistantly"),
                        0, 1, false, Geom::X, 1., 0.);
    addDistributeButton(INKSCAPE_ICON_DISTRIBUTE_HORIZONTAL_CENTER,
                        _("Distribute centers equidistantly horizontally"),
                        0, 2, false, Geom::X, .5, .5);
    addDistributeButton(INKSCAPE_ICON_DISTRIBUTE_HORIZONTAL_RIGHT,
                        _("Distribute right edges equidistantly"),
                        0, 3, false, Geom::X, 0., 1.);

    addDistributeButton(INKSCAPE_ICON_DISTRIBUTE_VERTICAL_GAPS,
                        _("Make vertical gaps between objects equal"),
                        1, 4, true, Geom::Y, .5, .5);

    addDistributeButton(INKSCAPE_ICON_DISTRIBUTE_VERTICAL_TOP,
                        _("Distribute top edges equidistantly"),
                        1, 1, false, Geom::Y, 0, 1);
    addDistributeButton(INKSCAPE_ICON_DISTRIBUTE_VERTICAL_CENTER,
                        _("Distribute centers equidistantly vertically"),
                        1, 2, false, Geom::Y, .5, .5);
    addDistributeButton(INKSCAPE_ICON_DISTRIBUTE_VERTICAL_BOTTOM,
                        _("Distribute bottom edges equidistantly"),
                        1, 3, false, Geom::Y, 1., 0.);

    //Baseline distribs
    addBaselineButton(INKSCAPE_ICON_DISTRIBUTE_HORIZONTAL_BASELINE,
                   _("Distribute baseline anchors of texts horizontally"),
                      0, 5, this->distribute_table(), Geom::X, true);
    addBaselineButton(INKSCAPE_ICON_DISTRIBUTE_VERTICAL_BASELINE,
                   _("Distribute baselines of texts vertically"),
                     1, 5, this->distribute_table(), Geom::Y, true);

    //Randomize & Unclump
    addRandomizeButton(INKSCAPE_ICON_DISTRIBUTE_RANDOMIZE,
                        _("Randomize centers in both dimensions"),
                        2, 2);
    addUnclumpButton(INKSCAPE_ICON_DISTRIBUTE_UNCLUMP,
                        _("Unclump objects: try to equalize edge-to-edge distances"),
                        2, 4);

    //Remove overlaps
    addRemoveOverlapsButton(INKSCAPE_ICON_DISTRIBUTE_REMOVE_OVERLAPS,
                            _("Move objects as little as possible so that their bounding boxes do not overlap"),
                            0, 0);
    //Graph Layout
    addGraphLayoutButton(INKSCAPE_ICON_DISTRIBUTE_GRAPH,
                            _("Nicely arrange selected connector network"),
                            0, 0);

    //Node Mode buttons
    // NOTE: "align nodes vertically" means "move nodes vertically until they align on a common
    // _horizontal_ line". This is analogous to what the "align-vertical-center" means.
    // There is no doubt some ambiguity. For this reason the descriptions are different.
    addNodeButton(INKSCAPE_ICON_ALIGN_HORIZONTAL_NODES,
                  _("Align selected nodes to a common vertical line"),
                  0, Geom::Y, false);
    addNodeButton(INKSCAPE_ICON_ALIGN_VERTICAL_NODES,
                  _("Align selected nodes to a common horizontal line"),
                  1, Geom::X, false);
    addNodeButton(INKSCAPE_ICON_DISTRIBUTE_HORIZONTAL_NODE,
                  _("Distribute selected nodes horizontally"),
                  2, Geom::X, true);
    addNodeButton(INKSCAPE_ICON_DISTRIBUTE_VERTICAL_NODE,
                  _("Distribute selected nodes vertically"),
                  3, Geom::Y, true);

    //Rest of the widgetry

    _combo.append_text(_("Last selected"));
    _combo.append_text(_("First selected"));
    _combo.append_text(_("Biggest object"));
    _combo.append_text(_("Smallest object"));
    _combo.append_text(_("Page"));
    _combo.append_text(_("Drawing"));
    _combo.append_text(_("Selection"));

    _combo.set_active(prefs->getInt("/dialogs/align/align-to", 6));
    _combo.signal_changed().connect(sigc::mem_fun(*this, &AlignAndDistribute::on_ref_change));

    _anchorBox.pack_start(_anchorLabel);
    _anchorBox.pack_start(_combo);

    _selgrpBox.pack_start(_selgrpLabel);
    _selgrpBox.pack_start(_selgrp);
    _selgrp.set_active(prefs->getBool("/dialogs/align/sel-as-groups"));
    _selgrp.signal_toggled().connect(sigc::mem_fun(*this, &AlignAndDistribute::on_selgrp_toggled));

    _alignBox.pack_start(_anchorBox);
    _alignBox.pack_start(_selgrpBox);
    _alignBox.pack_start(_alignTable);

    _alignFrame.add(_alignBox);
    _distributeFrame.add(_distributeTable);
    _removeOverlapFrame.add(_removeOverlapTable);
    _graphLayoutFrame.add(_graphLayoutTable);
    _nodesFrame.add(_nodesTable);

    Gtk::Box *contents = _getContents();
    contents->set_spacing(4);

    // Notebook for individual transformations

    contents->pack_start(_alignFrame, true, true);
    contents->pack_start(_distributeFrame, true, true);
    contents->pack_start(_removeOverlapFrame, true, true);
    contents->pack_start(_graphLayoutFrame, true, true);
    contents->pack_start(_nodesFrame, true, true);

    //Connect to the global tool change signal
    g_signal_connect (G_OBJECT (INKSCAPE), "set_eventcontext", G_CALLBACK (on_tool_changed), this);

    // Connect to the global selection change, to invalidate cached randomize_bbox
    g_signal_connect (G_OBJECT (INKSCAPE), "change_selection", G_CALLBACK (on_selection_changed), this);
    randomize_bbox = Geom::OptRect();

    show_all_children();

    on_tool_changed (NULL, NULL, this); // set current mode
}

AlignAndDistribute::~AlignAndDistribute()
{
    sp_signal_disconnect_by_data (G_OBJECT (INKSCAPE), this);

    for (std::list<Action *>::iterator it = _actionList.begin();
         it != _actionList.end();
         it ++)
        delete *it;
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
        &Gtk::Widget::show_all : &Gtk::Widget::hide_all;

    //Act on widgets used in selection mode
  void ( Gtk::Widget::*mSel) ()  = nodeEdit ?
      &Gtk::Widget::hide_all : &Gtk::Widget::show_all;


    ((_alignFrame).*(mSel))();
    ((_distributeFrame).*(mSel))();
    ((_removeOverlapFrame).*(mSel))();
    ((_graphLayoutFrame).*(mSel))();
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

void AlignAndDistribute::addBaselineButton(const Glib::ustring &id, const Glib::ustring tiptext,
                                    guint row, guint col, Gtk::Table &table, Geom::Dim2 orientation, bool distribute)
{
    _actionList.push_back(
        new ActionBaseline(
            id, tiptext, row, col,
            *this, table, orientation, distribute));
}




std::list<SPItem *>::iterator AlignAndDistribute::find_master( std::list<SPItem *> &list, bool horizontal){
    std::list<SPItem *>::iterator master = list.end();
    switch (getAlignTarget()) {
    case LAST:
        return list.begin();
        break;

    case FIRST:
        return --(list.end());
        break;

    case BIGGEST:
    {
        gdouble max = -1e18;
        for (std::list<SPItem *>::iterator it = list.begin(); it != list.end(); it++) {
            Geom::OptRect b = sp_item_bbox_desktop (*it);
            if (b) {
                gdouble dim = (*b)[horizontal ? Geom::X : Geom::Y].extent();
                if (dim > max) {
                    max = dim;
                    master = it;
                }
            }
        }
        return master;
        break;
    }

    case SMALLEST:
    {
        gdouble max = 1e18;
        for (std::list<SPItem *>::iterator it = list.begin(); it != list.end(); it++) {
            Geom::OptRect b = sp_item_bbox_desktop (*it);
            if (b) {
                gdouble dim = (*b)[horizontal ? Geom::X : Geom::Y].extent();
                if (dim < max) {
                    max = dim;
                    master = it;
                }
            }
        }
        return master;
        break;
    }

    default:
        g_assert_not_reached ();
        break;

    } // end of switch statement
    return master;
}

AlignAndDistribute::AlignTarget AlignAndDistribute::getAlignTarget()const {
    return AlignTarget(_combo.get_active_row_number());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
