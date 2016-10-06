#ifndef INKSCAPE_LIVEPATHEFFECT_PARAMETER_FONT_H
#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_FONT_H

/*
 * Inkscape::LivePathEffectParameters
 *
 * Authors:
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include <glib.h>
#include <gtkmm.h>
#include "live_effects/parameter/parameter.h"

namespace Inkscape {

namespace LivePathEffect {

class FontButtonParam : public Parameter {
public:
    FontButtonParam( const Glib::ustring& label,
               const Glib::ustring& tip,
               const Glib::ustring& key,
               Inkscape::UI::Widget::Registry* wr,
               Effect* effect,
               const Glib::ustring default_value = "");
    virtual ~FontButtonParam() {}

    virtual Gtk::Widget * param_newWidget();
    virtual bool param_readSVGValue(const gchar * strvalue);
    void param_update_default(const Glib::ustring defvalue);
    virtual gchar * param_getSVGValue() const;

    void param_setValue(const Glib::ustring newvalue);
    
    virtual void param_set_default();

    const Glib::ustring get_value() const { return defvalue; };

private:
    FontButtonParam(const FontButtonParam&);
    FontButtonParam& operator=(const FontButtonParam&);
    Glib::ustring value;
    Glib::ustring defvalue;

};

} //namespace LivePathEffect

} //namespace Inkscape

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
