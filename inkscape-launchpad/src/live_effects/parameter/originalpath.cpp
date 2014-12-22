/*
 * Copyright (C) Johan Engelen 2012 <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if GLIBMM_DISABLE_DEPRECATED && HAVE_GLIBMM_THREADS_H
#include <glibmm/threads.h>
#endif

#include <gtkmm/box.h>
#include "live_effects/parameter/originalpath.h"

#include "widgets/icon.h"
#include <glibmm/i18n.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>

#include "uri.h"
#include "sp-shape.h"
#include "sp-text.h"
#include "display/curve.h"
#include "live_effects/effect.h"

#include "inkscape.h"
#include "desktop.h"
#include "selection.h"
#include "ui/icon-names.h"

namespace Inkscape {

namespace LivePathEffect {

OriginalPathParam::OriginalPathParam( const Glib::ustring& label, const Glib::ustring& tip,
                      const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                      Effect* effect)
    : PathParam(label, tip, key, wr, effect, "")
{
    oncanvas_editable = false;
}

OriginalPathParam::~OriginalPathParam()
{

}

Gtk::Widget *
OriginalPathParam::param_newWidget()
{
    Gtk::HBox *_widget = Gtk::manage(new Gtk::HBox());

    { // Label
        Gtk::Label *pLabel = Gtk::manage(new Gtk::Label(param_label));
        static_cast<Gtk::HBox*>(_widget)->pack_start(*pLabel, true, true);
        pLabel->set_tooltip_text(param_tooltip);
    }

    { // Paste path to link button
        Gtk::Widget *pIcon = Gtk::manage( sp_icon_get_icon( INKSCAPE_ICON("edit-paste"), Inkscape::ICON_SIZE_BUTTON) );
        Gtk::Button *pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(sigc::mem_fun(*this, &OriginalPathParam::on_link_button_click));
        static_cast<Gtk::HBox*>(_widget)->pack_start(*pButton, true, true);
        pButton->set_tooltip_text(_("Link to path"));
    }

    { // Select original button
        Gtk::Widget *pIcon = Gtk::manage( sp_icon_get_icon("edit-select-original", Inkscape::ICON_SIZE_BUTTON) );
        Gtk::Button *pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(sigc::mem_fun(*this, &OriginalPathParam::on_select_original_button_click));
        static_cast<Gtk::HBox*>(_widget)->pack_start(*pButton, true, true);
        pButton->set_tooltip_text(_("Select original"));
    }

    static_cast<Gtk::HBox*>(_widget)->show_all_children();

    return dynamic_cast<Gtk::Widget *> (_widget);
}

void
OriginalPathParam::linked_modified_callback(SPObject *linked_obj, guint /*flags*/)
{
    SPCurve *curve = NULL;
    if (SP_IS_SHAPE(linked_obj)) {
        curve = SP_SHAPE(linked_obj)->getCurveBeforeLPE();
    }
    if (SP_IS_TEXT(linked_obj)) {
        curve = SP_TEXT(linked_obj)->getNormalizedBpath();
    }

    if (curve == NULL) {
        // curve invalid, set empty pathvector
        _pathvector = Geom::PathVector();
    } else {
        _pathvector = curve->get_pathvector();
        curve->unref();
    }

    must_recalculate_pwd2 = true;
    emit_changed();
    SP_OBJECT(param_effect->getLPEObj())->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

void
OriginalPathParam::linked_transformed_callback(Geom::Affine const * /*rel_transf*/, SPItem * /*moved_item*/)
{
/** \todo find good way to compensate for referenced path transform, like done for normal clones.
 *        See sp-use.cpp: sp_use_move_compensate */
}


void
OriginalPathParam::on_select_original_button_click()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    SPItem *original = ref.getObject();
    if (desktop == NULL || original == NULL) {
        return;
    }
    Inkscape::Selection *selection = desktop->getSelection();
    selection->clear();
    selection->set(original);
}

} /* namespace LivePathEffect */

} /* namespace Inkscape */

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
