/*
 * A widget for controlling object compositing (filter, opacity, etc.)
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Gustav Broberg <broberg@kth.se>
 *   Niko Kiirala <niko@kiirala.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2004--2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/widget/object-composite-settings.h"

#include <glibmm/i18n.h>

#include "desktop.h"

#include "desktop-style.h"
#include "document.h"
#include "document-undo.h"
#include "filter-chemistry.h"
#include "inkscape.h"
#include "selection.h"
#include "style.h"
#include "sp-item.h"
#include "svg/css-ostringstream.h"
#include "verbs.h"
#include "xml/repr.h"
#include "widgets/icon.h"
#include "ui/icon-names.h"
#include "display/sp-canvas.h"
#include "ui/widget/style-subject.h"
#include "ui/widget/gimpspinscale.h"

namespace Inkscape {
namespace UI {
namespace Widget {

ObjectCompositeSettings::ObjectCompositeSettings(unsigned int verb_code, char const *history_prefix, int flags)
: _verb_code(verb_code),
  _blur_tag(Glib::ustring(history_prefix) + ":blur"),
  _opacity_tag(Glib::ustring(history_prefix) + ":opacity"),
  _opacity_vbox(false, 0),
  _opacity_scale(_("Opacity (%)"), 100.0, 0.0, 100.0, 1.0, 1.0, 1),
  _fe_cb(flags),
  _fe_vbox(false, 0),
  _blocked(false)
{

    // Filter Effects
    pack_start(_fe_vbox, false, false, 2);
    _fe_vbox.pack_start(_fe_cb, false, false, 0);
    _fe_cb.signal_blend_blur_changed().connect(sigc::mem_fun(*this, &ObjectCompositeSettings::_blendBlurValueChanged));

    // Opacity
    pack_start(_opacity_vbox, false, false, 2);
    _opacity_vbox.pack_start(_opacity_scale);

    _opacity_scale.set_appearance("compact");

    _opacity_scale.signal_value_changed().connect(sigc::mem_fun(*this, &ObjectCompositeSettings::_opacityValueChanged));

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    _opacity_scale.set_focuswidget(GTK_WIDGET(desktop->canvas));

    /* SizeGroup keeps the blur and opacity labels aligned in Fill & Stroke dlg */
/*
    GtkSizeGroup *labels = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    gtk_size_group_add_widget(labels, GTK_WIDGET(_opacity_label.gobj()));
    gtk_size_group_add_widget(labels, GTK_WIDGET(_fe_cb.get_blur_label()->gobj()));
*/

    show_all_children();

    // These signals dont properly detect change in desktop, rely on owner dialog to call setSubject() from setTargetDesktop()
    //_desktop_activated = g_signal_connect ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (&ObjectCompositeSettings::_on_desktop_activate), this );
    //_desktop_activated = g_signal_connect ( G_OBJECT (INKSCAPE), "deactivate_desktop", G_CALLBACK (&ObjectCompositeSettings::_on_desktop_deactivate), this );
}

ObjectCompositeSettings::~ObjectCompositeSettings() {
    setSubject(NULL);
}

void ObjectCompositeSettings::setSubject(StyleSubject *subject) {
    _subject_changed.disconnect();
    if (subject) {
        _subject = subject;
        _subject_changed = _subject->connectChanged(sigc::mem_fun(*this, &ObjectCompositeSettings::_subjectChanged));
        _subject->setDesktop(SP_ACTIVE_DESKTOP);
    }
}

void
ObjectCompositeSettings::_blendBlurValueChanged()
{
    if (!_subject) {
        return;
    }

    SPDesktop *desktop = _subject->getDesktop();
    if (!desktop) {
        return;
    }
    SPDocument *document = desktop->getDocument();

    if (_blocked)
        return;
    _blocked = true;

    // FIXME: fix for GTK breakage, see comment in SelectedStyle::on_opacity_changed; here it results in crash 1580903
    //sp_canvas_force_full_redraw_after_interruptions(desktop->getCanvas(), 0);

    Geom::OptRect bbox = _subject->getBounds(SPItem::GEOMETRIC_BBOX);
    double radius;
    if (bbox) {
        double perimeter = bbox->dimensions()[Geom::X] + bbox->dimensions()[Geom::Y];   // fixme: this is only half the perimeter, is that correct?
        radius = _fe_cb.get_blur_value() * perimeter / 400;
    } else {
        radius = 0;
    }

    const Glib::ustring blendmode = _fe_cb.get_blend_mode();

    //apply created filter to every selected item
    std::vector<SPObject*> sel=_subject->list();
    for (std::vector<SPObject*>::const_iterator i = sel.begin() ; i != sel.end() ; ++i ) {
        if (!SP_IS_ITEM(*i)) {
            continue;
        }

        SPItem * item = SP_ITEM(*i);
        SPStyle *style = item->style;
        g_assert(style != NULL);

        if (blendmode != "normal") {
            SPFilter *filter = new_filter_simple_from_item(document, item, blendmode.c_str(), radius);
            sp_style_set_property_url(item, "filter", filter, false);
        } else {
            sp_style_set_property_url(item, "filter", NULL, false);
        }

        if (radius == 0 && item->style->filter.set
            && filter_is_single_gaussian_blur(SP_FILTER(item->style->getFilter()))) {
            remove_filter(item, false);
        }
        else if (radius != 0) {
            SPFilter *filter = modify_filter_gaussian_blur_from_item(document, item, radius);
            sp_style_set_property_url(item, "filter", filter, false);
        }

        //request update
        item->requestDisplayUpdate(( SP_OBJECT_MODIFIED_FLAG |
                                     SP_OBJECT_STYLE_MODIFIED_FLAG ));
    }

    DocumentUndo::maybeDone(document, _blur_tag.c_str(), _verb_code,
                            _("Change blur"));

    // resume interruptibility
    //sp_canvas_end_forced_full_redraws(desktop->getCanvas());

    _blocked = false;
}

void
ObjectCompositeSettings::_opacityValueChanged()
{
    if (!_subject) {
        return;
    }

    SPDesktop *desktop = _subject->getDesktop();
    if (!desktop) {
        return;
    }

    if (_blocked)
        return;
    _blocked = true;

    // FIXME: fix for GTK breakage, see comment in SelectedStyle::on_opacity_changed; here it results in crash 1580903
    // UPDATE: crash fixed in GTK+ 2.10.7 (bug 374378), remove this as soon as it's reasonably common
    // (though this only fixes the crash, not the multiple change events)
    //sp_canvas_force_full_redraw_after_interruptions(desktop->getCanvas(), 0);

    SPCSSAttr *css = sp_repr_css_attr_new ();

    Inkscape::CSSOStringStream os;
    os << CLAMP (_opacity_scale.get_adjustment()->get_value() / 100, 0.0, 1.0);
    sp_repr_css_set_property (css, "opacity", os.str().c_str());

    _subject->setCSS(css);

    sp_repr_css_attr_unref (css);

    DocumentUndo::maybeDone(desktop->getDocument(), _opacity_tag.c_str(), _verb_code,
                            _("Change opacity"));

    // resume interruptibility
    //sp_canvas_end_forced_full_redraws(desktop->getCanvas());

    _blocked = false;
}

void
ObjectCompositeSettings::_subjectChanged() {
    if (!_subject) {
        return;
    }

    SPDesktop *desktop = _subject->getDesktop();
    if (!desktop) {
        return;
    }

    if (_blocked)
        return;
    _blocked = true;

    SPStyle query(desktop->getDocument());
    int result = _subject->queryStyle(&query, QUERY_STYLE_PROPERTY_MASTEROPACITY);

    switch (result) {
        case QUERY_STYLE_NOTHING:
            _opacity_vbox.set_sensitive(false);
            // gtk_widget_set_sensitive (opa, FALSE);
            break;
        case QUERY_STYLE_SINGLE:
        case QUERY_STYLE_MULTIPLE_AVERAGED: // TODO: treat this slightly differently
        case QUERY_STYLE_MULTIPLE_SAME:
            _opacity_vbox.set_sensitive(true);
            _opacity_scale.get_adjustment()->set_value(100 * SP_SCALE24_TO_FLOAT(query.opacity.value));
            break;
    }

    //query now for current filter mode and average blurring of selection
    const int blend_result = _subject->queryStyle(&query, QUERY_STYLE_PROPERTY_BLEND);
    switch(blend_result) {
        case QUERY_STYLE_NOTHING:
            _fe_cb.set_sensitive(false);
            break;
        case QUERY_STYLE_SINGLE:
        case QUERY_STYLE_MULTIPLE_SAME:
            _fe_cb.set_blend_mode(query.filter_blend_mode.value);
            _fe_cb.set_sensitive(true);
            break;
        case QUERY_STYLE_MULTIPLE_DIFFERENT:
            // TODO: set text
            _fe_cb.set_sensitive(false);
            break;
    }

    if(blend_result == QUERY_STYLE_SINGLE || blend_result == QUERY_STYLE_MULTIPLE_SAME) {
        int blur_result = _subject->queryStyle(&query, QUERY_STYLE_PROPERTY_BLUR);
        switch (blur_result) {
            case QUERY_STYLE_NOTHING: //no blurring
                _fe_cb.set_blur_sensitive(false);
                break;
            case QUERY_STYLE_SINGLE:
            case QUERY_STYLE_MULTIPLE_AVERAGED:
            case QUERY_STYLE_MULTIPLE_SAME:
                Geom::OptRect bbox = _subject->getBounds(SPItem::GEOMETRIC_BBOX);
                if (bbox) {
                    double perimeter = bbox->dimensions()[Geom::X] + bbox->dimensions()[Geom::Y];   // fixme: this is only half the perimeter, is that correct?
                    _fe_cb.set_blur_sensitive(true);
                    //update blur widget value
                    float radius = query.filter_gaussianBlur_deviation.value;
                    float percent = radius * 400 / perimeter; // so that for a square, 100% == half side
                    _fe_cb.set_blur_value(percent);
                }
                break;
        }
    }

    _blocked = false;
}

}
}
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
