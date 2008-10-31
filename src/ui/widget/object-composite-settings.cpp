/*
 * A widget for controlling object compositing (filter, opacity, etc.)
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Gustav Broberg <broberg@kth.se>
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2004--2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/i18n.h>

#include "desktop-handles.h"
#include "desktop-style.h"
#include "document.h"
#include "filter-chemistry.h"
#include "inkscape.h"
#include "inkscape-stock.h"
#include "selection.h"
#include "style.h"
#include "sp-item.h"
#include "svg/css-ostringstream.h"
#include "verbs.h"
#include "xml/repr.h"
#include "widgets/icon.h"
#include "ui/widget/object-composite-settings.h"
#include "display/sp-canvas.h"

namespace Inkscape {
namespace UI {
namespace Widget {

void ObjectCompositeSettings::_on_desktop_activate(
    Inkscape::Application */*application*/,
    SPDesktop *desktop,
    ObjectCompositeSettings *w
) {
    if (w->_subject) {
        w->_subject->setDesktop(desktop);
    }
}

void ObjectCompositeSettings::_on_desktop_deactivate(
    Inkscape::Application */*application*/,
    SPDesktop *desktop,
    ObjectCompositeSettings *w
) {
    if (w->_subject) {
        w->_subject->setDesktop(NULL);
    }
}

ObjectCompositeSettings::ObjectCompositeSettings(unsigned int verb_code, char const *history_prefix, int flags)
: _verb_code(verb_code),
  _blur_tag(Glib::ustring(history_prefix) + ":blur"),
  _opacity_tag(Glib::ustring(history_prefix) + ":opacity"),
  _opacity_vbox(false, 0),
  _opacity_label_box(false, 0),
  _opacity_label(_("Opacity, %"), 0.0, 1.0, true),
  _opacity_adjustment(100.0, 0.0, 100.0, 1.0, 1.0, 0.0),
  _opacity_hscale(_opacity_adjustment),
  _opacity_spin_button(_opacity_adjustment, 0.01, 1),
  _fe_cb(flags),
  _fe_vbox(false, 0),
  _fe_alignment(1, 1, 1, 1),
  _blocked(false)
{
    // Filter Effects
    pack_start(_fe_vbox, false, false, 2);
    _fe_alignment.set_padding(0, 0, 4, 0);
    _fe_alignment.add(_fe_cb);
    _fe_vbox.pack_start(_fe_alignment, false, false, 0);
    _fe_cb.signal_blend_blur_changed().connect(sigc::mem_fun(*this, &ObjectCompositeSettings::_blendBlurValueChanged));

    // Opacity
    pack_start(_opacity_vbox, false, false, 2);
    _opacity_label_box.pack_start(_opacity_label, false, false, 4);
    _opacity_vbox.pack_start(_opacity_label_box, false, false, 0);
    _opacity_vbox.pack_start(_opacity_hbox, false, false, 0);
    _opacity_hbox.pack_start(_opacity_hscale, true, true, 4);
    _opacity_hbox.pack_start(_opacity_spin_button, false, false, 0);
    _opacity_hscale.set_draw_value(false);
    _opacity_adjustment.signal_value_changed().connect(sigc::mem_fun(*this, &ObjectCompositeSettings::_opacityValueChanged));

    show_all_children();

    _desktop_activated = g_signal_connect ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (&ObjectCompositeSettings::_on_desktop_activate), this );
    _desktop_activated = g_signal_connect ( G_OBJECT (INKSCAPE), "deactivate_desktop", G_CALLBACK (&ObjectCompositeSettings::_on_desktop_deactivate), this );
}

ObjectCompositeSettings::~ObjectCompositeSettings() {
    setSubject(NULL);
    g_signal_handler_disconnect(G_OBJECT(INKSCAPE), _desktop_activated);
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
    SPDocument *document = sp_desktop_document (desktop);

    if (_blocked)
        return;
    _blocked = true;

    // FIXME: fix for GTK breakage, see comment in SelectedStyle::on_opacity_changed; here it results in crash 1580903
    sp_canvas_force_full_redraw_after_interruptions(sp_desktop_canvas(desktop), 0);

    boost::optional<Geom::Rect> bbox = _subject->getBounds(SPItem::GEOMETRIC_BBOX);
    double radius;
    if (bbox) {
        double perimeter = bbox->dimensions()[Geom::X] + bbox->dimensions()[Geom::Y];   // fixme: this is only half the perimeter, is that correct?
        radius = _fe_cb.get_blur_value() * perimeter / 400;
    } else {
        radius = 0;
    }

    const Glib::ustring blendmode = _fe_cb.get_blend_mode();

    //apply created filter to every selected item
    for (StyleSubject::iterator i = _subject->begin() ; i != _subject->end() ; ++i ) {
        if (!SP_IS_ITEM(*i)) {
            continue;
        }

        SPItem * item = SP_ITEM(*i);
        SPStyle *style = SP_OBJECT_STYLE(item);
        g_assert(style != NULL);

        if (blendmode != "normal") {
            SPFilter *filter = new_filter_simple_from_item(document, item, blendmode.c_str(), radius);
            sp_style_set_property_url(item, "filter", filter, false);
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

    sp_document_maybe_done (document, _blur_tag.c_str(), _verb_code,
                            _("Change blur"));

    // resume interruptibility
    sp_canvas_end_forced_full_redraws(sp_desktop_canvas(desktop));

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
    sp_canvas_force_full_redraw_after_interruptions(sp_desktop_canvas(desktop), 0);

    SPCSSAttr *css = sp_repr_css_attr_new ();

    Inkscape::CSSOStringStream os;
    os << CLAMP (_opacity_adjustment.get_value() / 100, 0.0, 1.0);
    sp_repr_css_set_property (css, "opacity", os.str().c_str());

    _subject->setCSS(css);

    sp_repr_css_attr_unref (css);

    sp_document_maybe_done (sp_desktop_document (desktop), _opacity_tag.c_str(), _verb_code,
                            _("Change opacity"));

    // resume interruptibility
    sp_canvas_end_forced_full_redraws(sp_desktop_canvas(desktop));

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

    SPStyle *query = sp_style_new (sp_desktop_document(desktop));
    int result = _subject->queryStyle(query, QUERY_STYLE_PROPERTY_MASTEROPACITY);

    switch (result) {
        case QUERY_STYLE_NOTHING:
            _opacity_hbox.set_sensitive(false);
            // gtk_widget_set_sensitive (opa, FALSE);
            break;
        case QUERY_STYLE_SINGLE:
        case QUERY_STYLE_MULTIPLE_AVERAGED: // TODO: treat this slightly differently
        case QUERY_STYLE_MULTIPLE_SAME:
            _opacity_hbox.set_sensitive(true);
            _opacity_adjustment.set_value(100 * SP_SCALE24_TO_FLOAT(query->opacity.value));
            break;
    }

    //query now for current filter mode and average blurring of selection
    const int blend_result = _subject->queryStyle(query, QUERY_STYLE_PROPERTY_BLEND);
    switch(blend_result) {
        case QUERY_STYLE_NOTHING:
            _fe_cb.set_sensitive(false);
            break;
        case QUERY_STYLE_SINGLE:
        case QUERY_STYLE_MULTIPLE_SAME:
            _fe_cb.set_blend_mode(query->filter_blend_mode.value);
            _fe_cb.set_sensitive(true);
            break;
        case QUERY_STYLE_MULTIPLE_DIFFERENT:
            // TODO: set text
            _fe_cb.set_sensitive(false);
            break;
    }

    if(blend_result == QUERY_STYLE_SINGLE || blend_result == QUERY_STYLE_MULTIPLE_SAME) {
        int blur_result = _subject->queryStyle(query, QUERY_STYLE_PROPERTY_BLUR);
        switch (blur_result) {
            case QUERY_STYLE_NOTHING: //no blurring
                _fe_cb.set_blur_sensitive(false);
                break;
            case QUERY_STYLE_SINGLE:
            case QUERY_STYLE_MULTIPLE_AVERAGED:
            case QUERY_STYLE_MULTIPLE_SAME:
                boost::optional<Geom::Rect> bbox = _subject->getBounds(SPItem::GEOMETRIC_BBOX);
                if (bbox) {
                    double perimeter = bbox->dimensions()[Geom::X] + bbox->dimensions()[Geom::Y];   // fixme: this is only half the perimeter, is that correct?
                    _fe_cb.set_blur_sensitive(true);
                    //update blur widget value
                    float radius = query->filter_gaussianBlur_deviation.value;
                    float percent = radius * 400 / perimeter; // so that for a square, 100% == half side
                    _fe_cb.set_blur_value(percent);
                }
                break;
        }
    }

    sp_style_unref(query);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
