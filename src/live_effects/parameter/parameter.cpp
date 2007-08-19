#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_CPP

/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/parameter.h"
#include "live_effects/effect.h"
#include "svg/svg.h"

#include <gtkmm.h>
#include "ui/widget/scalar.h"

#include "svg/stringstream.h"

#include "verbs.h"

#define noLPEREALPARAM_DEBUG

namespace Inkscape {

namespace LivePathEffect {


Parameter::Parameter( const Glib::ustring& label, const Glib::ustring& tip,
                      const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                      Effect* effect )
{
    param_label = label;
    param_tooltip = tip;
    param_key = key;
    param_wr = wr;
    param_effect = effect;
}



/*###########################################
 *   REAL PARAM
 */
RealParam::RealParam( const Glib::ustring& label, const Glib::ustring& tip,
                      const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                      Effect* effect, gdouble default_value)
    : Parameter(label, tip, key, wr, effect)
{
    defvalue = default_value;
    value = defvalue;
    rsu = NULL;
}

RealParam::~RealParam()
{
    if (rsu)
        delete rsu;
}

bool
RealParam::param_readSVGValue(const gchar * strvalue)
{
    double newval;
    unsigned int success = sp_svg_number_read_d(strvalue, &newval);
    if (success == 1) {
        param_set_value(newval);
        return true;
    }
    return false;
}

gchar *
RealParam::param_writeSVGValue() const
{
    Inkscape::SVGOStringStream os;
    os << value;
    gchar * str = g_strdup(os.str().c_str());
    return str;
}

void
RealParam::param_set_default() 
{
    param_set_value(defvalue);
}

void
RealParam::param_set_value(gdouble val) 
{
    value = val;
    if (rsu)
        rsu->setValue(value);
}


Gtk::Widget *
RealParam::param_getWidget()
{
    if (!rsu) {
        rsu = new Inkscape::UI::Widget::RegisteredScalar();
        rsu->init(param_label, param_tooltip, param_key, *param_wr, param_effect->getRepr(), param_effect->getSPDoc());
        rsu->setValue(value);
        rsu->set_undo_parameters(SP_VERB_DIALOG_LIVE_PATH_EFFECT, _("Change scalar parameter"));
    }
    return dynamic_cast<Gtk::Widget *> (rsu->getS());
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
