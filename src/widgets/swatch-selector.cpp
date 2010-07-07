

#include <glibmm/i18n.h>

#include "swatch-selector.h"

#include "document.h"
#include "gradient-chemistry.h"
#include "gradient-selector.h"
#include "sp-color-notebook.h"
#include "sp-stop.h"
#include "svg/css-ostringstream.h"
#include "svg/svg-color.h"
#include "verbs.h"
#include "xml/node.h"

namespace Inkscape
{
namespace Widgets
{

SwatchSelector::SwatchSelector() :
    Gtk::VBox(),
    _gsel(0),
    _csel(0)
{
    GtkWidget *gsel = sp_gradient_selector_new();
    _gsel = SP_GRADIENT_SELECTOR(gsel);
    g_object_set_data( G_OBJECT(gobj()), "base", this );
    _gsel->setMode(SPGradientSelector::MODE_SWATCH);

    gtk_widget_show(gsel);

    pack_start(*Gtk::manage(Glib::wrap(gsel)));


    GtkWidget *csel = sp_color_selector_new( SP_TYPE_COLOR_NOTEBOOK );
    _csel = SP_COLOR_SELECTOR(csel);
    Gtk::Widget *wrappedCSel = Glib::wrap(csel);
    wrappedCSel->show();
    //gtk_widget_show(csel);


    GObject *obj = G_OBJECT(csel);

    g_signal_connect(obj, "grabbed", G_CALLBACK(_grabbedCb), this);
    g_signal_connect(obj, "dragged", G_CALLBACK(_draggedCb), this);
    g_signal_connect(obj, "released", G_CALLBACK(_releasedCb), this);
    g_signal_connect(obj, "changed", G_CALLBACK(_changedCb), this);

    pack_start(*Gtk::manage(wrappedCSel));
}

SwatchSelector::~SwatchSelector()
{
    _csel = 0; // dtor should be handled by Gtk::manage()
    _gsel = 0;
}

SPGradientSelector *SwatchSelector::getGradientSelector()
{
    return _gsel;
}

void SwatchSelector::_grabbedCb(SPColorSelector * /*csel*/, void * /*data*/)
{
}

void SwatchSelector::_draggedCb(SPColorSelector * /*csel*/, void *data)
{
    if (data) {
        //SwatchSelector *swsel = reinterpret_cast<SwatchSelector*>(data);

        // TODO might have to block cycles

        // Copied from gradient-vector.cpp, but does not appear to cause visible changes:
        /*
        if (swsel->_gsel) {
            SPGradient *gradient = swsel->_gsel->getVector();
            SPGradient *ngr = sp_gradient_ensure_vector_normalized(gradient);
            if (ngr != gradient) {
                // Our master gradient has changed
                // TODO replace with proper - sp_gradient_vector_widget_load_gradient(GTK_WIDGET(swsel->_gsel), ngr);
            }

            sp_gradient_ensure_vector(ngr);


            SPStop* stop = ngr->getFirstStop();
            if (stop) {
                swsel->_csel->base->getColorAlpha(stop->specified_color, &stop->opacity);
                stop->currentColor = false;
                // TODO push refresh
            }
        }
        */
    }
}

void SwatchSelector::_releasedCb(SPColorSelector * /*csel*/, void * /*data*/)
{
}

void SwatchSelector::_changedCb(SPColorSelector */*csel*/, void *data)
{
    if (data) {
        SwatchSelector *swsel = reinterpret_cast<SwatchSelector*>(data);

        // TODO might have to block cycles

        if (swsel->_gsel && swsel->_gsel->getVector()) {
            SPGradient *gradient = swsel->_gsel->getVector();
            SPGradient *ngr = sp_gradient_ensure_vector_normalized(gradient);
            if (ngr != gradient) {
                /* Our master gradient has changed */
                // TODO replace with proper - sp_gradient_vector_widget_load_gradient(GTK_WIDGET(swsel->_gsel), ngr);
            }

            ngr->ensureVector();


            SPStop* stop = ngr->getFirstStop();
            if (stop) {
                SPColor color;
                float alpha = 0;
                guint32 rgb = 0;

                swsel->_csel->base->getColorAlpha( color, alpha );
                rgb = color.toRGBA32( 0x00 );

                // TODO replace with generic shared code that also handles icc-color
                Inkscape::CSSOStringStream os;
                gchar c[64];
                sp_svg_write_color(c, sizeof(c), rgb);
                os << "stop-color:" << c << ";stop-opacity:" << static_cast<gdouble>(alpha) <<";";
                SP_OBJECT_REPR(stop)->setAttribute("style", os.str().c_str());

                sp_document_done(SP_OBJECT_DOCUMENT(ngr), SP_VERB_CONTEXT_GRADIENT,
                                 _("Change swatch color"));
            }
        }
    }
}

void SwatchSelector::connectGrabbedHandler( GCallback handler, void *data )
{
    GObject* obj = G_OBJECT(_gsel);
    g_signal_connect( obj, "grabbed", handler, data );
}

void SwatchSelector::connectDraggedHandler( GCallback handler, void *data )
{
    GObject* obj = G_OBJECT(_gsel);
    g_signal_connect( obj, "dragged", handler, data );
}

void SwatchSelector::connectReleasedHandler( GCallback handler, void *data )
{
    GObject* obj = G_OBJECT(_gsel);
    g_signal_connect( obj, "released", handler, data );
}

void SwatchSelector::connectchangedHandler( GCallback handler, void *data )
{
    GObject* obj = G_OBJECT(_gsel);
    g_signal_connect( obj, "changed", handler, data );
}

void SwatchSelector::setVector(SPDocument */*doc*/, SPGradient *vector)
{
    //GtkVBox * box = gobj();
    _gsel->setVector((vector) ? SP_OBJECT_DOCUMENT(vector) : 0, vector);

    if ( vector && vector->isSolid() ) {
        SPStop* stop = vector->getFirstStop();

        guint32 const colorVal = sp_stop_get_rgba32(stop);
        _csel->base->setAlpha(SP_RGBA32_A_F(colorVal));
        SPColor color( SP_RGBA32_R_F(colorVal), SP_RGBA32_G_F(colorVal), SP_RGBA32_B_F(colorVal) );
        // set its color, from the stored array
        _csel->base->setColor( color );
        gtk_widget_show_all( GTK_WIDGET(_csel) );
    } else {
        gtk_widget_hide( GTK_WIDGET(_csel) );
    }

/*
*/
}

} // namespace Widgets
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
