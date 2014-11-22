#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "gradient-vector.h"
#include <math.h>
#include <gtk/gtk.h>
#include <glibmm/i18n.h>
#include <map>
#include <set>
#include <vector>

#include "ui/dialog-events.h"
#include "sp-color-icc-selector.h"
#include "sp-color-scales.h"
#include "sp-color-slider.h"
#include "svg/svg-icc-color.h"
#include "colorspace.h"
#include "document.h"
#include "inkscape.h"
#include "profile-manager.h"

#define noDEBUG_LCMS

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
#include "color-profile.h"
#include "cms-system.h"
#include "color-profile-cms-fns.h"

#ifdef DEBUG_LCMS
#include "preferences.h"
#endif // DEBUG_LCMS
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)


#ifdef DEBUG_LCMS
extern guint update_in_progress;
#define DEBUG_MESSAGE(key, ...) \
{\
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();\
    bool dump = prefs->getBool("/options/scislac/" #key);\
    bool dumpD = prefs->getBool("/options/scislac/" #key "D");\
    bool dumpD2 = prefs->getBool("/options/scislac/" #key "D2");\
    dumpD &&= ( (update_in_progress == 0) || dumpD2 );\
    if ( dump )\
    {\
        g_message( __VA_ARGS__ );\
\
    }\
    if ( dumpD )\
    {\
        GtkWidget *dialog = gtk_message_dialog_new(NULL,\
                                                   GTK_DIALOG_DESTROY_WITH_PARENT, \
                                                   GTK_MESSAGE_INFO,    \
                                                   GTK_BUTTONS_OK,      \
                                                   __VA_ARGS__          \
                                                   );\
        g_signal_connect_swapped(dialog, "response",\
                                 G_CALLBACK(gtk_widget_destroy),        \
                                 dialog);                               \
        gtk_widget_show_all( dialog );\
    }\
}
#endif // DEBUG_LCMS



G_BEGIN_DECLS

static void sp_color_icc_selector_dispose(GObject *object);
static void sp_color_icc_selector_show_all (GtkWidget *widget);
static void sp_color_icc_selector_hide(GtkWidget *widget);

G_END_DECLS

/**
 * Class containing the parts for a single color component's UI presence.
 */
class ComponentUI
{
public:
    ComponentUI() :
        _component(),
        _adj(0),
        _slider(0),
        _btn(0),
        _label(0),
        _map(0)
    {
    }

    ComponentUI(colorspace::Component const &component) :
        _component(component),
        _adj(0),
        _slider(0),
        _btn(0),
        _label(0),
        _map(0)
    {
    }

    colorspace::Component _component;
    GtkAdjustment *_adj; // Component adjustment
    GtkWidget *_slider;
    GtkWidget *_btn;     // spinbutton
    GtkWidget *_label;   // Label
    guchar *_map;    
};

/**
 * Class that implements the internals of the selector.
 */
class ColorICCSelectorImpl
{
public:

    ColorICCSelectorImpl( ColorICCSelector *owner);

    ~ColorICCSelectorImpl();

    static void _adjustmentChanged ( GtkAdjustment *adjustment, SPColorICCSelector *cs );

    static void _sliderGrabbed( SPColorSlider *slider, SPColorICCSelector *cs );
    static void _sliderReleased( SPColorSlider *slider, SPColorICCSelector *cs );
    static void _sliderChanged( SPColorSlider *slider, SPColorICCSelector *cs );

    static void _profileSelected( GtkWidget* src, gpointer data );
    static void _fixupHit( GtkWidget* src, gpointer data );

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    void _setProfile( SVGICCColor* profile );
    void _switchToProfile( gchar const* name );
#endif
    void _updateSliders( gint ignore );
    void _profilesChanged( std::string const & name );

    ColorICCSelector *_owner;

    gboolean _updating : 1;
    gboolean _dragging : 1;

    guint32 _fixupNeeded;
    GtkWidget* _fixupBtn;
    GtkWidget* _profileSel;

    std::vector<ComponentUI> _compUI;

    GtkAdjustment* _adj; // Channel adjustment
    GtkWidget* _slider;
    GtkWidget* _sbtn; // Spinbutton
    GtkWidget* _label; // Label

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    std::string _profileName;
    Inkscape::ColorProfile* _prof;
    guint _profChannelCount;
    gulong _profChangedID;
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
};

#define XPAD 4
#define YPAD 1

namespace
{

size_t maxColorspaceComponentCount = 0;

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

/**
 * Internal variable to track all known colorspaces.
 */
std::set<cmsUInt32Number> knownColorspaces;

#endif


/**
 * Simple helper to allow bitwise or on GtkAttachOptions.
 */
GtkAttachOptions operator|(GtkAttachOptions lhs, GtkAttachOptions rhs)
{
    return static_cast<GtkAttachOptions>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

/**
 * Helper function to handle GTK2/GTK3 attachment #ifdef code.
 */
void attachToGridOrTable(GtkWidget *parent,
                         GtkWidget *child,
                         guint left,
                         guint top,
                         guint width,
                         guint height,
                         bool hexpand = false,
                         bool centered = false,
                         guint xpadding = XPAD,
                         guint ypadding = YPAD)
{
#if GTK_CHECK_VERSION(3,0,0)
    #if GTK_CHECK_VERSION(3,12,0)
    gtk_widget_set_margin_start( child, xpadding );
    gtk_widget_set_margin_end( child, xpadding );
    #else
    gtk_widget_set_margin_left( child, xpadding );
    gtk_widget_set_margin_right( child, xpadding );
    #endif

    gtk_widget_set_margin_top( child, ypadding );
    gtk_widget_set_margin_bottom( child, ypadding );
    if (hexpand) {
        gtk_widget_set_hexpand(child, TRUE);
    }
    if (centered) {
        gtk_widget_set_halign( child, GTK_ALIGN_CENTER );
        gtk_widget_set_valign( child, GTK_ALIGN_CENTER );
    }
    gtk_grid_attach( GTK_GRID(parent), child, left, top, width, height );
#else
    GtkAttachOptions xoptions = centered ? static_cast<GtkAttachOptions>(0) : hexpand ? (GTK_EXPAND | GTK_FILL) : GTK_FILL;
    GtkAttachOptions yoptions = centered ? static_cast<GtkAttachOptions>(0) : GTK_FILL;

    gtk_table_attach( GTK_TABLE(parent), child, left, left + width, top, top + height, xoptions, yoptions, xpadding, ypadding );
#endif
}

} // namespace

G_DEFINE_TYPE(SPColorICCSelector, sp_color_icc_selector, SP_TYPE_COLOR_SELECTOR);

static void sp_color_icc_selector_class_init(SPColorICCSelectorClass *klass)
{
    static const gchar* nameset[] = {N_("CMS"), 0};
    GObjectClass   *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    SPColorSelectorClass *selector_class = SP_COLOR_SELECTOR_CLASS (klass);

    selector_class->name = nameset;
    selector_class->submode_count = 1;

    object_class->dispose = sp_color_icc_selector_dispose;

    widget_class->show_all = sp_color_icc_selector_show_all;
    widget_class->hide = sp_color_icc_selector_hide;
}


ColorICCSelector::ColorICCSelector( SPColorSelector* csel )
    : ColorSelector( csel ),
      _impl(NULL)
{
}

ColorICCSelector::~ColorICCSelector()
{
    if (_impl)
    {
        delete _impl;
        _impl = 0;
    }
}

void sp_color_icc_selector_init(SPColorICCSelector *cs)
{
    SP_COLOR_SELECTOR(cs)->base = new ColorICCSelector( SP_COLOR_SELECTOR(cs) );

    if ( SP_COLOR_SELECTOR(cs)->base )
    {
        SP_COLOR_SELECTOR(cs)->base->init();
    }
}


/*
icSigRgbData
icSigCmykData
icSigCmyData
*/
#define SPACE_ID_RGB 0
#define SPACE_ID_CMY 1
#define SPACE_ID_CMYK 2


#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
static cmsUInt16Number* getScratch() {
    // bytes per pixel * input channels * width
    static cmsUInt16Number* scritch = static_cast<cmsUInt16Number*>(g_new(cmsUInt16Number, 4 * 1024));

    return scritch;
}

colorspace::Component::Component() :
    name(),
    tip(),
    scale(1)
{
}

colorspace::Component::Component(std::string const &name, std::string const &tip, guint scale) :
    name(name),
    tip(tip),
    scale(scale)
{
}

std::vector<colorspace::Component> colorspace::getColorSpaceInfo( uint32_t space )
{
    static std::map<cmsUInt32Number, std::vector<Component> > sets;
    if (sets.empty())
    {
        sets[cmsSigXYZData].push_back(Component("_X", "X", 2)); //  TYPE_XYZ_16
        sets[cmsSigXYZData].push_back(Component("_Y", "Y", 1));
        sets[cmsSigXYZData].push_back(Component("_Z", "Z", 2));

        sets[cmsSigLabData].push_back(Component("_L", "L", 100)); // TYPE_Lab_16
        sets[cmsSigLabData].push_back(Component("_a", "a", 256));
        sets[cmsSigLabData].push_back(Component("_b", "b", 256));

        //cmsSigLuvData

        sets[cmsSigYCbCrData].push_back(Component("_Y", "Y", 1)); // TYPE_YCbCr_16
        sets[cmsSigYCbCrData].push_back(Component("C_b", "Cb", 1));
        sets[cmsSigYCbCrData].push_back(Component("C_r", "Cr", 1));

        sets[cmsSigYxyData].push_back(Component("_Y", "Y", 1)); // TYPE_Yxy_16
        sets[cmsSigYxyData].push_back(Component("_x", "x", 1));
        sets[cmsSigYxyData].push_back(Component("y", "y", 1));

        sets[cmsSigRgbData].push_back(Component(_("_R:"), _("Red"), 1)); // TYPE_RGB_16
        sets[cmsSigRgbData].push_back(Component(_("_G:"), _("Green"), 1));
        sets[cmsSigRgbData].push_back(Component(_("_B:"), _("Blue"), 1));

        sets[cmsSigGrayData].push_back(Component(_("G:"), _("Gray"), 1)); // TYPE_GRAY_16

        sets[cmsSigHsvData].push_back(Component(_("_H:"), _("Hue"), 360)); // TYPE_HSV_16
        sets[cmsSigHsvData].push_back(Component(_("_S:"), _("Saturation"), 1));
        sets[cmsSigHsvData].push_back(Component("_V:", "Value", 1));

        sets[cmsSigHlsData].push_back(Component(_("_H:"), _("Hue"), 360)); // TYPE_HLS_16
        sets[cmsSigHlsData].push_back(Component(_("_L:"), _("Lightness"), 1));
        sets[cmsSigHlsData].push_back(Component(_("_S:"), _("Saturation"), 1));

        sets[cmsSigCmykData].push_back(Component(_("_C:"), _("Cyan"), 1)); // TYPE_CMYK_16
        sets[cmsSigCmykData].push_back(Component(_("_M:"), _("Magenta"), 1));
        sets[cmsSigCmykData].push_back(Component(_("_Y:"), _("Yellow"), 1));
        sets[cmsSigCmykData].push_back(Component(_("_K:"), _("Black"), 1));

        sets[cmsSigCmyData].push_back(Component(_("_C:"), _("Cyan"), 1)); // TYPE_CMY_16
        sets[cmsSigCmyData].push_back(Component(_("_M:"), _("Magenta"), 1));
        sets[cmsSigCmyData].push_back(Component(_("_Y:"), _("Yellow"), 1));

        for (std::map<cmsUInt32Number, std::vector<Component> >::iterator it = sets.begin(); it != sets.end(); ++it)
        {
            knownColorspaces.insert(it->first);
            maxColorspaceComponentCount = std::max(maxColorspaceComponentCount, it->second.size());
        }
    }

    std::vector<Component> target;

    if (sets.find(space) != sets.end())
    {
        target = sets[space];
    }
    return target;
}


std::vector<colorspace::Component> colorspace::getColorSpaceInfo( Inkscape::ColorProfile *prof )
{
    return getColorSpaceInfo( asICColorSpaceSig(prof->getColorSpace()) );
}

#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

ColorICCSelectorImpl::ColorICCSelectorImpl(ColorICCSelector *owner) :
    _owner(owner),
    _updating( FALSE ),
    _dragging( FALSE ),
    _fixupNeeded(0),
    _fixupBtn(0),
    _profileSel(0),
    _compUI(),
    _adj(0),
    _slider(0),
    _sbtn(0),
    _label(0)
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    ,
    _profileName(),
    _prof(0),
    _profChannelCount(0),
    _profChangedID(0)
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
{
}

ColorICCSelectorImpl::~ColorICCSelectorImpl()
{
    _adj = 0;
    _sbtn = 0;
    _label = 0;
}

void ColorICCSelector::init()
{
    if (_impl) delete(_impl);
	_impl = new ColorICCSelectorImpl(this);
    gint row = 0;

    _impl->_updating = FALSE;
    _impl->_dragging = FALSE;

#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *t = gtk_grid_new();
#else
    GtkWidget *t = gtk_table_new(5, 3, FALSE);
#endif

    gtk_widget_show (t);
    gtk_box_pack_start (GTK_BOX (_csel), t, TRUE, TRUE, 4);

    _impl->_compUI.clear();

    // Create components
    row = 0;


    _impl->_fixupBtn = gtk_button_new_with_label(_("Fix"));
    g_signal_connect( G_OBJECT(_impl->_fixupBtn), "clicked", G_CALLBACK(ColorICCSelectorImpl::_fixupHit), (gpointer)_impl );
    gtk_widget_set_sensitive( _impl->_fixupBtn, FALSE );
    gtk_widget_set_tooltip_text( _impl->_fixupBtn, _("Fix RGB fallback to match icc-color() value.") );
    //gtk_misc_set_alignment( GTK_MISC (_impl->_fixupBtn), 1.0, 0.5 );
    gtk_widget_show( _impl->_fixupBtn );

    attachToGridOrTable(t, _impl->_fixupBtn, 0, row, 1, 1);

    // Combobox and store with 2 columns : label (0) and full name (1)
    GtkListStore *store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
    _impl->_profileSel = gtk_combo_box_new_with_model (GTK_TREE_MODEL (store));

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(_impl->_profileSel), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(_impl->_profileSel), renderer, "text", 0, NULL);

    GtkTreeIter iter;
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter, 0, _("<none>"), 1, _("<none>"), -1);

    gtk_widget_show( _impl->_profileSel );
    gtk_combo_box_set_active( GTK_COMBO_BOX(_impl->_profileSel), 0 );

    attachToGridOrTable(t, _impl->_profileSel, 1, row, 1, 1);

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    _impl->_profChangedID = g_signal_connect( G_OBJECT(_impl->_profileSel), "changed", G_CALLBACK(ColorICCSelectorImpl::_profileSelected), (gpointer)_impl );
#else
    gtk_widget_set_sensitive( _impl->_profileSel, false );
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)


    row++;

    // populate the data for colorspaces and channels:
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    std::vector<colorspace::Component> things = colorspace::getColorSpaceInfo( cmsSigRgbData );
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

    for ( size_t i = 0; i < maxColorspaceComponentCount; i++ ) {
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
        if (i < things.size()) {
            _impl->_compUI.push_back(ComponentUI(things[i]));
        } else {
            _impl->_compUI.push_back(ComponentUI());
        }

        std::string labelStr = (i < things.size()) ? things[i].name.c_str() : "";
#else
        _impl->_compUI.push_back(ComponentUI());

        std::string labelStr = ".";
#endif

        _impl->_compUI[i]._label = gtk_label_new_with_mnemonic( labelStr.c_str() );
        gtk_misc_set_alignment( GTK_MISC (_impl->_compUI[i]._label), 1.0, 0.5 );
        gtk_widget_show( _impl->_compUI[i]._label );

        attachToGridOrTable(t, _impl->_compUI[i]._label, 0, row, 1, 1);

        // Adjustment
        guint scaleValue = _impl->_compUI[i]._component.scale;
        gdouble step = static_cast<gdouble>(scaleValue) / 100.0;
        gdouble page = static_cast<gdouble>(scaleValue) / 10.0;
        gint digits = (step > 0.9) ? 0 : 2;
        _impl->_compUI[i]._adj = GTK_ADJUSTMENT( gtk_adjustment_new( 0.0, 0.0, scaleValue,  step, page, page ) );

        // Slider
        _impl->_compUI[i]._slider = sp_color_slider_new( _impl->_compUI[i]._adj );
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
        gtk_widget_set_tooltip_text( _impl->_compUI[i]._slider, (i < things.size()) ? things[i].tip.c_str() : "" );
#else
        gtk_widget_set_tooltip_text( _impl->_compUI[i]._slider, "." );
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
        gtk_widget_show( _impl->_compUI[i]._slider );

        attachToGridOrTable(t, _impl->_compUI[i]._slider, 1, row, 1, 1, true);

        _impl->_compUI[i]._btn = gtk_spin_button_new( _impl->_compUI[i]._adj, step, digits );
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
        gtk_widget_set_tooltip_text( _impl->_compUI[i]._btn, (i < things.size()) ? things[i].tip.c_str() : "" );
#else
        gtk_widget_set_tooltip_text( _impl->_compUI[i]._btn, "." );
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
        sp_dialog_defocus_on_enter( _impl->_compUI[i]._btn );
        gtk_label_set_mnemonic_widget( GTK_LABEL(_impl->_compUI[i]._label), _impl->_compUI[i]._btn );
        gtk_widget_show( _impl->_compUI[i]._btn );

        attachToGridOrTable(t, _impl->_compUI[i]._btn, 2, row, 1, 1, false, true);

        _impl->_compUI[i]._map = g_new( guchar, 4 * 1024 );
        memset( _impl->_compUI[i]._map, 0x0ff, 1024 * 4 );


        // Signals
        g_signal_connect( G_OBJECT( _impl->_compUI[i]._adj ), "value_changed", G_CALLBACK( ColorICCSelectorImpl::_adjustmentChanged ), _csel );

        g_signal_connect( G_OBJECT( _impl->_compUI[i]._slider ), "grabbed", G_CALLBACK( ColorICCSelectorImpl::_sliderGrabbed ), _csel );
        g_signal_connect( G_OBJECT( _impl->_compUI[i]._slider ), "released", G_CALLBACK( ColorICCSelectorImpl::_sliderReleased ), _csel );
        g_signal_connect( G_OBJECT( _impl->_compUI[i]._slider ), "changed", G_CALLBACK( ColorICCSelectorImpl::_sliderChanged ), _csel );

        row++;
    }

    // Label
    _impl->_label = gtk_label_new_with_mnemonic(_("_A:"));
    gtk_misc_set_alignment(GTK_MISC(_impl->_label), 1.0, 0.5);
    gtk_widget_show(_impl->_label);

    attachToGridOrTable(t, _impl->_label, 0, row, 1, 1);

    // Adjustment
    _impl->_adj = GTK_ADJUSTMENT(gtk_adjustment_new(0.0, 0.0, 255.0, 1.0, 10.0, 10.0));

    // Slider
    _impl->_slider = sp_color_slider_new(_impl->_adj);
    gtk_widget_set_tooltip_text(_impl->_slider, _("Alpha (opacity)"));
    gtk_widget_show(_impl->_slider);

    attachToGridOrTable(t, _impl->_slider, 1, row, 1, 1, true);

    sp_color_slider_set_colors( SP_COLOR_SLIDER( _impl->_slider ),
                                SP_RGBA32_F_COMPOSE( 1.0, 1.0, 1.0, 0.0 ),
                                SP_RGBA32_F_COMPOSE( 1.0, 1.0, 1.0, 0.5 ),
                                SP_RGBA32_F_COMPOSE( 1.0, 1.0, 1.0, 1.0 ) );


    // Spinbutton
    _impl->_sbtn = gtk_spin_button_new(GTK_ADJUSTMENT(_impl->_adj), 1.0, 0);
    gtk_widget_set_tooltip_text(_impl->_sbtn, _("Alpha (opacity)"));
    sp_dialog_defocus_on_enter(_impl->_sbtn);
    gtk_label_set_mnemonic_widget(GTK_LABEL(_impl->_label), _impl->_sbtn);
    gtk_widget_show(_impl->_sbtn);

    attachToGridOrTable(t, _impl->_sbtn, 2, row, 1, 1, false, true);

    // Signals
    g_signal_connect(G_OBJECT(_impl->_adj), "value_changed", G_CALLBACK(ColorICCSelectorImpl::_adjustmentChanged), _csel);

    g_signal_connect(G_OBJECT(_impl->_slider), "grabbed", G_CALLBACK(ColorICCSelectorImpl::_sliderGrabbed), _csel);
    g_signal_connect(G_OBJECT(_impl->_slider), "released", G_CALLBACK(ColorICCSelectorImpl::_sliderReleased), _csel);
    g_signal_connect(G_OBJECT(_impl->_slider), "changed", G_CALLBACK(ColorICCSelectorImpl::_sliderChanged), _csel);
}

static void sp_color_icc_selector_dispose(GObject *object)
{
    if (G_OBJECT_CLASS(sp_color_icc_selector_parent_class)->dispose) {
        G_OBJECT_CLASS(sp_color_icc_selector_parent_class)->dispose(object);
    }
}

static void
sp_color_icc_selector_show_all (GtkWidget *widget)
{
    gtk_widget_show (widget);
}

static void sp_color_icc_selector_hide(GtkWidget *widget)
{
    gtk_widget_hide(widget);
}

GtkWidget *
sp_color_icc_selector_new (void)
{
    SPColorICCSelector *csel;

    csel = static_cast<SPColorICCSelector*>(g_object_new (SP_TYPE_COLOR_ICC_SELECTOR, NULL));

    return GTK_WIDGET (csel);
}


void ColorICCSelectorImpl::_fixupHit( GtkWidget* /*src*/, gpointer data )
{
    ColorICCSelectorImpl* self = reinterpret_cast<ColorICCSelectorImpl*>(data);
    gtk_widget_set_sensitive( self->_fixupBtn, FALSE );
    self->_adjustmentChanged( self->_compUI[0]._adj, SP_COLOR_ICC_SELECTOR(self->_owner->_csel) );
}

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
void ColorICCSelectorImpl::_profileSelected( GtkWidget* /*src*/, gpointer data )
{
    ColorICCSelectorImpl* self = reinterpret_cast<ColorICCSelectorImpl*>(data);

    GtkTreeIter  iter;
    if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(self->_profileSel), &iter)) {
        GtkTreeModel *store = gtk_combo_box_get_model(GTK_COMBO_BOX(self->_profileSel));
        gchar* name = 0;

        gtk_tree_model_get(store, &iter, 1, &name, -1);
        self->_switchToProfile( name );
        gtk_widget_set_tooltip_text(self->_profileSel, name );

        if ( name ) {
            g_free( name );
        }
    }
}
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
void ColorICCSelectorImpl::_switchToProfile( gchar const* name )
{
    bool dirty = false;
    SPColor tmp( _owner->_color );

    if ( name ) {
        if ( tmp.icc && tmp.icc->colorProfile == name ) {
#ifdef DEBUG_LCMS
             g_message("Already at name [%s]", name );
#endif // DEBUG_LCMS
        } else {
#ifdef DEBUG_LCMS
             g_message("Need to switch to profile [%s]", name );
#endif // DEBUG_LCMS
            if ( tmp.icc ) {
                tmp.icc->colors.clear();
            } else {
                tmp.icc = new SVGICCColor();
            }
            tmp.icc->colorProfile = name;
            Inkscape::ColorProfile* newProf = SP_ACTIVE_DOCUMENT->profileManager->find(name);
            if ( newProf ) {
                cmsHTRANSFORM trans = newProf->getTransfFromSRGB8();
                if ( trans ) {
                    guint32 val = _owner->_color.toRGBA32(0);
                    guchar pre[4] = {
                        static_cast<guchar>(SP_RGBA32_R_U(val)),
                        static_cast<guchar>(SP_RGBA32_G_U(val)),
                        static_cast<guchar>(SP_RGBA32_B_U(val)),
                        255};
#ifdef DEBUG_LCMS
                    g_message("Shoving in [%02x] [%02x] [%02x]", pre[0], pre[1], pre[2]);
#endif // DEBUG_LCMS
                    cmsUInt16Number post[4] = {0,0,0,0};
                    cmsDoTransform( trans, pre, post, 1 );
#ifdef DEBUG_LCMS
                    g_message("got on out [%04x] [%04x] [%04x] [%04x]", post[0], post[1], post[2], post[3]);
#endif // DEBUG_LCMS
#if HAVE_LIBLCMS1
                    guint count = _cmsChannelsOf( asICColorSpaceSig(newProf->getColorSpace()) );
#elif HAVE_LIBLCMS2
                    guint count = cmsChannelsOf( asICColorSpaceSig(newProf->getColorSpace()) );
#endif

                    std::vector<colorspace::Component> things = colorspace::getColorSpaceInfo(asICColorSpaceSig(newProf->getColorSpace()));

                    for ( guint i = 0; i < count; i++ ) {
                        gdouble val = (((gdouble)post[i])/65535.0) * (gdouble)((i < things.size()) ? things[i].scale : 1);
#ifdef DEBUG_LCMS
                        g_message("     scaled %d by %d to be %f", i, ((i < things.size()) ? things[i].scale : 1), val);
#endif // DEBUG_LCMS
                        tmp.icc->colors.push_back(val);
                    }
                    cmsHTRANSFORM retrans = newProf->getTransfToSRGB8();
                    if ( retrans ) {
                        cmsDoTransform( retrans, post, pre, 1 );
#ifdef DEBUG_LCMS
                        g_message("  back out [%02x] [%02x] [%02x]", pre[0], pre[1], pre[2]);
#endif // DEBUG_LCMS
                        tmp.set(SP_RGBA32_U_COMPOSE(pre[0], pre[1], pre[2], 0xff));
                    }
                }
            }
            dirty = true;
        }
    } else {
#ifdef DEBUG_LCMS
         g_message("NUKE THE ICC");
#endif // DEBUG_LCMS
        if ( tmp.icc ) {
            delete tmp.icc;
            tmp.icc = 0;
            dirty = true;
            _fixupHit( 0, this );
        } else {
#ifdef DEBUG_LCMS
             g_message("No icc to nuke");
#endif // DEBUG_LCMS
        }
    }

    if ( dirty ) {
#ifdef DEBUG_LCMS
        g_message("+----------------");
        g_message("+   new color is [%s]", tmp.toString().c_str());
#endif // DEBUG_LCMS
        _setProfile( tmp.icc );
        //_adjustmentChanged( _compUI[0]._adj, SP_COLOR_ICC_SELECTOR(_csel) );
        _owner->setColorAlpha( tmp, _owner->_alpha, true );
#ifdef DEBUG_LCMS
        g_message("+_________________");
#endif // DEBUG_LCMS
    }
}
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
void ColorICCSelectorImpl::_profilesChanged( std::string const & name )
{
    GtkComboBox* combo = GTK_COMBO_BOX(_profileSel);

    g_signal_handler_block( G_OBJECT(_profileSel), _profChangedID );

    GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(combo));
    gtk_list_store_clear(store);

    GtkTreeIter iter;
    gtk_list_store_append (store, &iter);
    gtk_list_store_set(store, &iter, 0, _("<none>"), 1, _("<none>"), -1);

    gtk_combo_box_set_active( combo, 0 );

    int index = 1;
    const GSList *current = SP_ACTIVE_DOCUMENT->getResourceList( "iccprofile" );
    while ( current ) {
        SPObject* obj = SP_OBJECT(current->data);
        Inkscape::ColorProfile* prof = reinterpret_cast<Inkscape::ColorProfile*>(obj);

        gtk_list_store_append (store, &iter);
        gtk_list_store_set(store, &iter, 0, gr_ellipsize_text(prof->name, 25).c_str(), 1, prof->name, -1);

        if ( name == prof->name ) {
            gtk_combo_box_set_active( combo, index );
            gtk_widget_set_tooltip_text(_profileSel, prof->name );
        }

        index++;
        current = g_slist_next(current);
    }

    g_signal_handler_unblock( G_OBJECT(_profileSel), _profChangedID );
}
#else
void ColorICCSelectorImpl::_profilesChanged( std::string const & /*name*/ )
{
}
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

// Helpers for setting color value

void ColorICCSelector::_colorChanged()
{
    _impl->_updating = TRUE;
     //sp_color_icc_set_color( SP_COLOR_ICC( _icc ), &color );

#ifdef DEBUG_LCMS
    g_message( "/^^^^^^^^^  %p::_colorChanged(%08x:%s)", this,
               _color.toRGBA32(_alpha), ( (_color.icc) ? _color.icc->colorProfile.c_str(): "<null>" )
               );
#endif // DEBUG_LCMS

#ifdef DEBUG_LCMS
    g_message("FLIPPIES!!!!     %p   '%s'", _color.icc, (_color.icc ? _color.icc->colorProfile.c_str():"<null>"));
#endif // DEBUG_LCMS

    _impl->_profilesChanged( (_color.icc) ? _color.icc->colorProfile : std::string("") );
    ColorScales::setScaled( _impl->_adj, _alpha );

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    _impl->_setProfile( _color.icc );
    _impl->_fixupNeeded = 0;
    gtk_widget_set_sensitive( _impl->_fixupBtn, FALSE );

    if (_impl->_prof) {
        if (_impl->_prof->getTransfToSRGB8() ) {
            cmsUInt16Number tmp[4];
            for ( guint i = 0; i < _impl->_profChannelCount; i++ ) {
                gdouble val = 0.0;
                if ( _color.icc->colors.size() > i ) {
                    if ( _impl->_compUI[i]._component.scale == 256 ) {
                        val = (_color.icc->colors[i] + 128.0) / static_cast<gdouble>(_impl->_compUI[i]._component.scale);
                    } else {
                        val = _color.icc->colors[i] / static_cast<gdouble>(_impl->_compUI[i]._component.scale);
                    }
                }
                tmp[i] = val * 0x0ffff;
            }
            guchar post[4] = {0,0,0,0};
            cmsHTRANSFORM trans = _impl->_prof->getTransfToSRGB8();
            if ( trans ) {
                cmsDoTransform( trans, tmp, post, 1 );
                guint32 other = SP_RGBA32_U_COMPOSE(post[0], post[1], post[2], 255 );
                if ( other != _color.toRGBA32(255) ) {
                    _impl->_fixupNeeded = other;
                    gtk_widget_set_sensitive( _impl->_fixupBtn, TRUE );
#ifdef DEBUG_LCMS
                    g_message("Color needs to change 0x%06x to 0x%06x", _color.toRGBA32(255) >> 8, other >> 8 );
#endif // DEBUG_LCMS
                }
            }
        }
    }
#else
    //(void)color;
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    _impl->_updateSliders( -1 );


    _impl->_updating = FALSE;
#ifdef DEBUG_LCMS
    g_message( "\\_________  %p::_colorChanged()", this );
#endif // DEBUG_LCMS
}

#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
void ColorICCSelectorImpl::_setProfile( SVGICCColor* profile )
{
#ifdef DEBUG_LCMS
    g_message( "/^^^^^^^^^  %p::_setProfile(%s)", this,
               ( (profile) ? profile->colorProfile.c_str() : "<null>")
               );
#endif // DEBUG_LCMS
    bool profChanged = false;
    if ( _prof && (!profile || (_profileName != profile->colorProfile) ) ) {
        // Need to clear out the prior one
        profChanged = true;
        _profileName.clear();
        _prof = 0;
        _profChannelCount = 0;
    } else if ( profile && !_prof ) {
        profChanged = true;
    }

    for ( size_t i = 0; i < _compUI.size(); i++ ) {
        gtk_widget_hide( _compUI[i]._label );
        gtk_widget_hide( _compUI[i]._slider );
        gtk_widget_hide( _compUI[i]._btn );
    }

    if ( profile ) {
        _prof = SP_ACTIVE_DOCUMENT->profileManager->find(profile->colorProfile.c_str());
        if ( _prof && (asICColorProfileClassSig(_prof->getProfileClass()) != cmsSigNamedColorClass) ) {
#if HAVE_LIBLCMS1
            _profChannelCount = _cmsChannelsOf( asICColorSpaceSig(_prof->getColorSpace()) );
#elif HAVE_LIBLCMS2
            _profChannelCount = cmsChannelsOf( asICColorSpaceSig(_prof->getColorSpace()) );
#endif

            if ( profChanged ) {
                std::vector<colorspace::Component> things = colorspace::getColorSpaceInfo(asICColorSpaceSig(_prof->getColorSpace()));
                for (size_t i = 0; (i < things.size()) && (i < _profChannelCount); ++i)
                {
                    _compUI[i]._component = things[i];
                }

                for ( guint i = 0; i < _profChannelCount; i++ ) {
                    gtk_label_set_text_with_mnemonic( GTK_LABEL(_compUI[i]._label), (i < things.size()) ? things[i].name.c_str() : "");

                    gtk_widget_set_tooltip_text( _compUI[i]._slider, (i < things.size()) ?  things[i].tip.c_str() : "" );
                    gtk_widget_set_tooltip_text( _compUI[i]._btn, (i < things.size()) ?  things[i].tip.c_str() : "" );

                    sp_color_slider_set_colors( SP_COLOR_SLIDER(_compUI[i]._slider),
                                                SPColor(0.0, 0.0, 0.0).toRGBA32(0xff),
                                                SPColor(0.5, 0.5, 0.5).toRGBA32(0xff),
                                                SPColor(1.0, 1.0, 1.0).toRGBA32(0xff) );
/*
                    _compUI[i]._adj = GTK_ADJUSTMENT( gtk_adjustment_new( val, 0.0, _fooScales[i],  step, page, page ) );
                    g_signal_connect( G_OBJECT( _compUI[i]._adj ), "value_changed", G_CALLBACK( _adjustmentChanged ), _csel );

                    sp_color_slider_set_adjustment( SP_COLOR_SLIDER(_compUI[i]._slider), _compUI[i]._adj );
                    gtk_spin_button_set_adjustment( GTK_SPIN_BUTTON(_compUI[i]._btn), _compUI[i]._adj );
                    gtk_spin_button_set_digits( GTK_SPIN_BUTTON(_compUI[i]._btn), digits );
*/
                    gtk_widget_show( _compUI[i]._label );
                    gtk_widget_show( _compUI[i]._slider );
                    gtk_widget_show( _compUI[i]._btn );
                    //gtk_adjustment_set_value( _compUI[i]._adj, 0.0 );
                    //gtk_adjustment_set_value( _compUI[i]._adj, val );
                }
                for ( size_t i = _profChannelCount; i < _compUI.size(); i++ ) {
                    gtk_widget_hide( _compUI[i]._label );
                    gtk_widget_hide( _compUI[i]._slider );
                    gtk_widget_hide( _compUI[i]._btn );
                }
            }
        } else {
            // Give up for now on named colors
            _prof = 0;
        }
    }

#ifdef DEBUG_LCMS
    g_message( "\\_________  %p::_setProfile()", this );
#endif // DEBUG_LCMS
}
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

void ColorICCSelectorImpl::_updateSliders( gint ignore )
{
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    if ( _owner->_color.icc )
    {
        for ( guint i = 0; i < _profChannelCount; i++ ) {
            gdouble val = 0.0;
            if ( _owner->_color.icc->colors.size() > i ) {
                if ( _compUI[i]._component.scale == 256 ) {
                    val = (_owner->_color.icc->colors[i] + 128.0) / static_cast<gdouble>(_compUI[i]._component.scale);
                } else {
                    val = _owner->_color.icc->colors[i] / static_cast<gdouble>(_compUI[i]._component.scale);
                }
            }
            gtk_adjustment_set_value( _compUI[i]._adj, val );
        }

        if ( _prof ) {
            if ( _prof->getTransfToSRGB8() ) {
                for ( guint i = 0; i < _profChannelCount; i++ ) {
                    if ( static_cast<gint>(i) != ignore ) {
                        cmsUInt16Number* scratch = getScratch();
                        cmsUInt16Number filler[4] = {0, 0, 0, 0};
                        for ( guint j = 0; j < _profChannelCount; j++ ) {
                            filler[j] = 0x0ffff * ColorScales::getScaled( _compUI[j]._adj );
                        }
                        
                        cmsUInt16Number* p = scratch;
                        for ( guint x = 0; x < 1024; x++ ) {
                            for ( guint j = 0; j < _profChannelCount; j++ ) {
                                if ( j == i ) {
                                    *p++ = x * 0x0ffff / 1024;
                                } else {
                                    *p++ = filler[j];
                                }
                            }
                        }
                        
                        cmsHTRANSFORM trans = _prof->getTransfToSRGB8();
                        if ( trans ) {
                            cmsDoTransform( trans, scratch, _compUI[i]._map, 1024 );
                            sp_color_slider_set_map( SP_COLOR_SLIDER(_compUI[i]._slider), _compUI[i]._map );
                        }
                    }
                }
            }
        }
    }
#else
    (void)ignore;
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)

    guint32 start = _owner->_color.toRGBA32( 0x00 );
    guint32 mid = _owner->_color.toRGBA32( 0x7f );
    guint32 end = _owner->_color.toRGBA32( 0xff );

    sp_color_slider_set_colors( SP_COLOR_SLIDER(_slider), start, mid, end );
}


void ColorICCSelectorImpl::_adjustmentChanged( GtkAdjustment *adjustment, SPColorICCSelector *cs )
{
// // TODO check this. It looks questionable:
//     // if a value is entered between 0 and 1 exclusive, normalize it to (int) 0..255  or 0..100
//     if (adjustment->value > 0.0 && adjustment->value < 1.0) {
//         gtk_adjustment_set_value( adjustment, floor ((adjustment->value) * adjustment->upper + 0.5) );
//     }

#ifdef DEBUG_LCMS
    g_message( "/^^^^^^^^^  %p::_adjustmentChanged()", cs );
#endif // DEBUG_LCMS

     ColorICCSelector* iccSelector = static_cast<ColorICCSelector*>(SP_COLOR_SELECTOR(cs)->base);
     if (iccSelector->_impl->_updating) {
         return;
     }

     iccSelector->_impl->_updating = TRUE;

     gint match = -1;

     SPColor newColor( iccSelector->_color );
     gfloat scaled = ColorScales::getScaled( iccSelector->_impl->_adj );
     if ( iccSelector->_impl->_adj == adjustment ) {
#ifdef DEBUG_LCMS
         g_message("ALPHA");
#endif // DEBUG_LCMS
     } else {
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
         for ( size_t i = 0; i < iccSelector->_impl->_compUI.size(); i++ ) {
             if ( iccSelector->_impl->_compUI[i]._adj == adjustment ) {
                 match = i;
                 break;
             }
         }
         if ( match >= 0 ) {
#ifdef DEBUG_LCMS
             g_message(" channel %d", match );
#endif // DEBUG_LCMS
         }


         cmsUInt16Number tmp[4];
         for ( guint i = 0; i < 4; i++ ) {
             tmp[i] = ColorScales::getScaled( iccSelector->_impl->_compUI[i]._adj ) * 0x0ffff;
         }
         guchar post[4] = {0,0,0,0};

         cmsHTRANSFORM trans = iccSelector->_impl->_prof->getTransfToSRGB8();
         if ( trans ) {
             cmsDoTransform( trans, tmp, post, 1 );
         }

         SPColor other( SP_RGBA32_U_COMPOSE(post[0], post[1], post[2], 255) );
         other.icc = new SVGICCColor();
         if ( iccSelector->_color.icc ) {
             other.icc->colorProfile = iccSelector->_color.icc->colorProfile;
         }

         guint32 prior = iccSelector->_color.toRGBA32(255);
         guint32 newer = other.toRGBA32(255);

         if ( prior != newer ) {
#ifdef DEBUG_LCMS
             g_message("Transformed color from 0x%08x to 0x%08x", prior, newer );
             g_message("      ~~~~ FLIP");
#endif // DEBUG_LCMS
             newColor = other;
             newColor.icc->colors.clear();
             for ( guint i = 0; i < iccSelector->_impl->_profChannelCount; i++ ) {
                 gdouble val = ColorScales::getScaled( iccSelector->_impl->_compUI[i]._adj );
                 val *= iccSelector->_impl->_compUI[i]._component.scale;
                 if ( iccSelector->_impl->_compUI[i]._component.scale == 256 ) {
                     val -= 128;
                 }
                 newColor.icc->colors.push_back( val );
             }
         }
#endif // defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
     }
     iccSelector->_updateInternals( newColor, scaled, iccSelector->_impl->_dragging );
     iccSelector->_impl->_updateSliders( match );

     iccSelector->_impl->_updating = FALSE;
#ifdef DEBUG_LCMS
     g_message( "\\_________  %p::_adjustmentChanged()", cs );
#endif // DEBUG_LCMS
}

void ColorICCSelectorImpl::_sliderGrabbed( SPColorSlider * /*slider*/, SPColorICCSelector * /*cs*/ )
{
//    ColorICCSelector* iccSelector = dynamic_cast<ColorICCSelector*>(SP_COLOR_SELECTOR(cs)->base);
//     if (!iccSelector->_dragging) {
//         iccSelector->_dragging = TRUE;
//         iccSelector->_grabbed();
//         iccSelector->_updateInternals( iccSelector->_color, ColorScales::getScaled( iccSelector->_impl->_adj ), iccSelector->_dragging );
//     }
}

void ColorICCSelectorImpl::_sliderReleased( SPColorSlider * /*slider*/, SPColorICCSelector * /*cs*/ )
{
//     ColorICCSelector* iccSelector = dynamic_cast<ColorICCSelector*>(SP_COLOR_SELECTOR(cs)->base);
//     if (iccSelector->_dragging) {
//         iccSelector->_dragging = FALSE;
//         iccSelector->_released();
//         iccSelector->_updateInternals( iccSelector->_color, ColorScales::getScaled( iccSelector->_adj ), iccSelector->_dragging );
//     }
}

#ifdef DEBUG_LCMS
void ColorICCSelectorImpl::_sliderChanged( SPColorSlider *slider, SPColorICCSelector *cs )
#else
void ColorICCSelectorImpl::_sliderChanged( SPColorSlider * /*slider*/, SPColorICCSelector * /*cs*/ )
#endif // DEBUG_LCMS
{
#ifdef DEBUG_LCMS
    g_message("Changed  %p and %p", slider, cs );
#endif // DEBUG_LCMS
//     ColorICCSelector* iccSelector = dynamic_cast<ColorICCSelector*>(SP_COLOR_SELECTOR(cs)->base);

//     iccSelector->_updateInternals( iccSelector->_color, ColorScales::getScaled( iccSelector->_adj ), iccSelector->_dragging );
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
