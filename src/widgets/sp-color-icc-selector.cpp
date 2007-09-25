#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <math.h>
#include <gtk/gtksignal.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktable.h>
#include <gtk/gtkspinbutton.h>
#include <glibmm/i18n.h>
#include "../dialogs/dialog-events.h"
#include "sp-color-icc-selector.h"
#include "sp-color-scales.h"
#include "svg/svg-icc-color.h"
#include "inkscape.h"

#define noDEBUG_LCMS

#if ENABLE_LCMS
#include "color-profile-fns.h"
#include "color-profile.h"
//#define DEBUG_LCMS
#ifdef DEBUG_LCMS
#include "prefs-utils.h"
#include <gtk/gtkmessagedialog.h>
#endif // DEBUG_LCMS
#endif // ENABLE_LCMS


#ifdef DEBUG_LCMS
extern guint update_in_progress;
#define DEBUG_MESSAGE(key, ...) \
{\
    gint dump = prefs_get_int_attribute_limited("options.scislac", #key, 0, 0, 1);\
    gint dumpD = prefs_get_int_attribute_limited("options.scislac", #key"D", 0, 0, 1);\
    gint dumpD2 = prefs_get_int_attribute_limited("options.scislac", #key"D2", 0, 0, 1);\
    dumpD &= ( (update_in_progress == 0) || dumpD2 );\
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

static void sp_color_icc_selector_class_init (SPColorICCSelectorClass *klass);
static void sp_color_icc_selector_init (SPColorICCSelector *cs);
static void sp_color_icc_selector_destroy (GtkObject *object);

static void sp_color_icc_selector_show_all (GtkWidget *widget);
static void sp_color_icc_selector_hide_all (GtkWidget *widget);


G_END_DECLS

static SPColorSelectorClass *parent_class;

#define XPAD 4
#define YPAD 1

GType
sp_color_icc_selector_get_type (void)
{
    static GType type = 0;
    if (!type) {
        static const GTypeInfo info = {
            sizeof (SPColorICCSelectorClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) sp_color_icc_selector_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (SPColorICCSelector),
            0,    /* n_preallocs */
            (GInstanceInitFunc) sp_color_icc_selector_init,
            0,    /* value_table */
        };

        type = g_type_register_static (SP_TYPE_COLOR_SELECTOR,
                                       "SPColorICCSelector",
                                       &info,
                                       static_cast< GTypeFlags > (0) );
    }
    return type;
}

static void
sp_color_icc_selector_class_init (SPColorICCSelectorClass *klass)
{
    static const gchar* nameset[] = {N_("CMS"), 0};
    GtkObjectClass *object_class;
    GtkWidgetClass *widget_class;
    SPColorSelectorClass *selector_class;

    object_class = (GtkObjectClass *) klass;
    widget_class = (GtkWidgetClass *) klass;
    selector_class = SP_COLOR_SELECTOR_CLASS (klass);

    parent_class = SP_COLOR_SELECTOR_CLASS (g_type_class_peek_parent (klass));

    selector_class->name = nameset;
    selector_class->submode_count = 1;

    object_class->destroy = sp_color_icc_selector_destroy;

    widget_class->show_all = sp_color_icc_selector_show_all;
    widget_class->hide_all = sp_color_icc_selector_hide_all;
}

ColorICCSelector::ColorICCSelector( SPColorSelector* csel )
    : ColorSelector( csel ),
      _updating( FALSE ),
      _dragging( FALSE ),
      _fooCount(0),
      _fooScales(0),
      _fooAdj(0),
      _fooSlider(0),
      _fooBtn(0),
      _fooLabel(0),
      _fooMap(0),
      _adj(0),
      _sbtn(0),
      _label(0),
      _tt(0)
#if ENABLE_LCMS
    ,
      _profileName(""),
      _profIntent(Inkscape::RENDERING_INTENT_UNKNOWN),
      _profileSpace(icSigRgbData),
      _profileClass(icSigInputClass),
      _prof(0),
      _destProf(0),
      _transf(0),
      _profChannelCount(0)
#endif // ENABLE_LCMS
{
}

ColorICCSelector::~ColorICCSelector()
{
    _adj = 0;
    _sbtn = 0;
    _label = 0;
}

void sp_color_icc_selector_init (SPColorICCSelector *cs)
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


static icUInt16Number* getScratch() {
    // bytes per pixel * input channels * width
    static icUInt16Number* scritch = static_cast<icUInt16Number*>(g_new(icUInt16Number, 4 * 1024));

    return scritch;
}

struct MapMap {
    DWORD space;
    DWORD inForm;
};

void getThings( DWORD space, gchar const**& namers, gchar const**& tippies, bool const*& trickies, guint const*& scalies, DWORD& inputFormat ) {

    MapMap possible[] = {
        {icSigXYZData,   TYPE_XYZ_16},
        {icSigLabData,   TYPE_Lab_16},
        //icSigLuvData
        {icSigYCbCrData, TYPE_YCbCr_16},
        {icSigYxyData,   TYPE_Yxy_16},
        {icSigRgbData,   TYPE_RGB_16},
        {icSigGrayData,  TYPE_GRAY_16},
        {icSigHsvData,   TYPE_HSV_16},
        {icSigHlsData,   TYPE_HLS_16},
        {icSigCmykData,  TYPE_CMYK_16},
        {icSigCmyData,   TYPE_CMY_16},
    };

    static gchar const *names[][6] = {
        {"_X", "_Y", "_Z", "", "", ""},
        {"_L", "_a", "_b", "", "", ""},
        //
        {"_Y", "C_b", "C_r", "", "", ""},
        {"_Y", "_x", "y", "", "", ""},
        {_("_R"), _("_G"), _("_B"), "", "", ""},
        {_("_G"), "", "", "", "", ""},
        {_("_H"), _("_S"), "_V", "", "", ""},
        {_("_H"), _("_L"), _("_S"), "", "", ""},
        {_("_C"), _("_M"), _("_Y"), _("_K"), "", ""},
        {_("_C"), _("_M"), _("_Y"), "", "", ""},
    };

    static gchar const *tips[][6] = {
        {"X", "Y", "Z", "", "", ""},
        {"L", "a", "b", "", "", ""},
        //
        {"Y", "Cb", "Cr", "", "", ""},
        {"Y", "x", "y", "", "", ""},
        {_("Red"), _("Green"), _("Blue"), "", "", ""},
        {_("Gray"), "", "", "", "", ""},
        {_("Hue"), _("Saturation"), "Value", "", "", ""},
        {_("Hue"), _("Lightness"), _("Saturation"), "", "", ""},
        {_("Cyan"), _("Magenta"), _("Yellow"), _("Black"), "", ""},
        {_("Cyan"), _("Magenta"), _("Yellow"), "", "", ""},
    };

    static bool tricksies[][6] = {
        {false, false, false, false, false, false}, // XYZ       0~2, 0~1, 0~2
        {false, true,  true,  false, false, false}, // Lab       0~100, -128~128, -128~128
        // Luv 0~100, -128~128, -128~128
        {false, true,  true,  false, false, false}, // YCbCr     ?
        {false, true,  true,  false, false, false}, // Yxy       0~1, 0~1, 0~1
        {false, false, false, false, false, false}, // RGB       0~1, 0~1, 0~1
        {false, false, false, false, false, false}, // Gray      0~1
        {true,  false, false, false, false, false}, // HSV       0~360, 0~1, 0~1
        {true,  false, false, false, false, false}, // HLS       0~360, 0~1, 0~1
        {false, false, false, false, false, false}, // CMYK      0~1, 0~1, 0~1, 0~1
        {false, false, false, false, false, false}, // CMY
    };

    static guint scales[][6] = {
        {2, 1, 2, 1, 1, 1},
        {100, 256, 256, 1, 1, 1},
        //
        {1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1},
        {360, 1, 1, 1, 1, 1},
        {360, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1},
    };

    int index = 0;
    for ( guint i = 0; i < G_N_ELEMENTS(possible); i++ ) {
        if ( possible[i].space == space ) {
            index = i;
            break;
        }
    }

    inputFormat = possible[index].inForm;
    namers = names[index];
    tippies = tips[index];
    trickies = tricksies[index];
    scalies = scales[index];
}


void ColorICCSelector::init()
{
    GtkWidget *t;
    gint row = 0;

    _updating = FALSE;
    _dragging = FALSE;

    _tt = gtk_tooltips_new();

    t = gtk_table_new (5, 3, FALSE);
    gtk_widget_show (t);
    gtk_box_pack_start (GTK_BOX (_csel), t, TRUE, TRUE, 0);

    DWORD inputFormat = TYPE_RGB_16;
    //guint partCount = _cmsChannelsOf( icSigRgbData );
    gchar const** names = 0;
    gchar const** tips = 0;
    bool const* tricky = 0;
    getThings( icSigRgbData, names, tips, tricky, _fooScales, inputFormat );


    /* Create components */
    row = 0;


    _fooCount = 4;
    _fooAdj = new GtkAdjustment*[_fooCount];
    _fooSlider = new GtkWidget*[_fooCount];
    _fooBtn = new GtkWidget*[_fooCount];
    _fooLabel = new GtkWidget*[_fooCount];
    _fooMap = new guchar*[_fooCount];

    for ( guint i = 0; i < _fooCount; i++ ) {
        /* Label */
        _fooLabel[i] = gtk_label_new_with_mnemonic( names[i] );
        gtk_misc_set_alignment( GTK_MISC (_fooLabel[i]), 1.0, 0.5 );
        gtk_widget_show( _fooLabel[i] );
        gtk_table_attach( GTK_TABLE (t), _fooLabel[i], 0, 1, row, row + 1, GTK_FILL, GTK_FILL, XPAD, YPAD );

        /* Adjustment */
        gdouble step = static_cast<gdouble>(_fooScales[i]) / 100.0;
        gdouble page = static_cast<gdouble>(_fooScales[i]) / 10.0;
        gint digits = (step > 0.9) ? 0 : 2;
        _fooAdj[i] = GTK_ADJUSTMENT( gtk_adjustment_new( 0.0, 0.0, _fooScales[i],  step, page, page ) );

        /* Slider */
        _fooSlider[i] = sp_color_slider_new( _fooAdj[i] );
        gtk_tooltips_set_tip( _tt, _fooSlider[i], tips[i], NULL );
        gtk_widget_show( _fooSlider[i] );
        gtk_table_attach( GTK_TABLE (t), _fooSlider[i], 1, 2, row, row + 1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)GTK_FILL, XPAD, YPAD );

        _fooBtn[i] = gtk_spin_button_new( _fooAdj[i], step, digits );
        gtk_tooltips_set_tip( _tt, _fooBtn[i], tips[i], NULL );
        sp_dialog_defocus_on_enter( _fooBtn[i] );
        gtk_label_set_mnemonic_widget( GTK_LABEL(_fooLabel[i]), _fooBtn[i] );
        gtk_widget_show( _fooBtn[i] );
        gtk_table_attach( GTK_TABLE (t), _fooBtn[i], 2, 3, row, row + 1, (GtkAttachOptions)0, (GtkAttachOptions)0, XPAD, YPAD );

        _fooMap[i] = g_new( guchar, 4 * 1024 );
        memset( _fooMap[i], 0x0ff, 1024 * 4 );


        /* Signals */
        gtk_signal_connect( GTK_OBJECT( _fooAdj[i] ), "value_changed", GTK_SIGNAL_FUNC( _adjustmentChanged ), _csel );

        gtk_signal_connect( GTK_OBJECT( _fooSlider[i] ), "grabbed", GTK_SIGNAL_FUNC( _sliderGrabbed ), _csel );
        gtk_signal_connect( GTK_OBJECT( _fooSlider[i] ), "released", GTK_SIGNAL_FUNC( _sliderReleased ), _csel );
        gtk_signal_connect( GTK_OBJECT( _fooSlider[i] ), "changed", GTK_SIGNAL_FUNC( _sliderChanged ), _csel );

        row++;
    }

    /* Label */
    _label = gtk_label_new_with_mnemonic (_("_A"));
    gtk_misc_set_alignment (GTK_MISC (_label), 1.0, 0.5);
    gtk_widget_show (_label);
    gtk_table_attach (GTK_TABLE (t), _label, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);

    /* Adjustment */
    _adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 255.0, 1.0, 10.0, 10.0);

    /* Slider */
    _slider = sp_color_slider_new (_adj);
    gtk_tooltips_set_tip (_tt, _slider, _("Alpha (opacity)"), NULL);
    gtk_widget_show (_slider);
    gtk_table_attach (GTK_TABLE (t), _slider, 1, 2, row, row + 1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)GTK_FILL, XPAD, YPAD);

    sp_color_slider_set_colors( SP_COLOR_SLIDER( _slider ),
                                SP_RGBA32_F_COMPOSE( 1.0, 1.0, 1.0, 0.0 ),
                                SP_RGBA32_F_COMPOSE( 1.0, 1.0, 1.0, 0.5 ),
                                SP_RGBA32_F_COMPOSE( 1.0, 1.0, 1.0, 1.0 ) );


    /* Spinbutton */
    _sbtn = gtk_spin_button_new (GTK_ADJUSTMENT (_adj), 1.0, 0);
    gtk_tooltips_set_tip (_tt, _sbtn, _("Alpha (opacity)"), NULL);
    sp_dialog_defocus_on_enter (_sbtn);
    gtk_label_set_mnemonic_widget (GTK_LABEL(_label), _sbtn);
    gtk_widget_show (_sbtn);
    gtk_table_attach (GTK_TABLE (t), _sbtn, 2, 3, row, row + 1, (GtkAttachOptions)0, (GtkAttachOptions)0, XPAD, YPAD);

    /* Signals */
    gtk_signal_connect (GTK_OBJECT (_adj), "value_changed",
                        GTK_SIGNAL_FUNC (_adjustmentChanged), _csel);

    gtk_signal_connect (GTK_OBJECT (_slider), "grabbed",
                        GTK_SIGNAL_FUNC (_sliderGrabbed), _csel);
    gtk_signal_connect (GTK_OBJECT (_slider), "released",
                        GTK_SIGNAL_FUNC (_sliderReleased), _csel);
    gtk_signal_connect (GTK_OBJECT (_slider), "changed",
                        GTK_SIGNAL_FUNC (_sliderChanged), _csel);
}

static void
sp_color_icc_selector_destroy (GtkObject *object)
{
    if (((GtkObjectClass *) (parent_class))->destroy)
        (* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

static void
sp_color_icc_selector_show_all (GtkWidget *widget)
{
    gtk_widget_show (widget);
}

static void
sp_color_icc_selector_hide_all (GtkWidget *widget)
{
    gtk_widget_hide (widget);
}

GtkWidget *
sp_color_icc_selector_new (void)
{
    SPColorICCSelector *csel;

    csel = (SPColorICCSelector*)gtk_type_new (SP_TYPE_COLOR_ICC_SELECTOR);

    return GTK_WIDGET (csel);
}

/* Helpers for setting color value */

void ColorICCSelector::_colorChanged( const SPColor& color, gfloat alpha )
{
    _updating = TRUE;
//     sp_color_icc_set_color( SP_COLOR_ICC( _icc ), &color );

#ifdef DEBUG_LCMS
    g_message( "/^^^^^^^^^  %p::_colorChanged(%08x:%s)", this,
               color.toRGBA32(alpha), ( (color.icc) ? color.icc->colorProfile.c_str(): "<null>" )
               );
#endif // DEBUG_LCMS

#ifdef DEBUG_LCMS
    g_message("FLIPPIES!!!!     %p   '%s'", color.icc, (color.icc?color.icc->colorProfile.c_str():"<null>"));
#endif // DEBUG_LCMS

    ColorScales::setScaled( _adj, alpha );

#if ENABLE_LCMS
    _setProfile( color.icc );

#endif // ENABLE_LCMS
    _updateSliders();


    _updating = FALSE;
#ifdef DEBUG_LCMS
    g_message( "\\_________  %p::_colorChanged()", this );
#endif // DEBUG_LCMS
}

#if ENABLE_LCMS
void ColorICCSelector::_setProfile( SVGICCColor* profile )
{
#ifdef DEBUG_LCMS
    g_message( "/^^^^^^^^^  %p::_setProfile(%s)", this,
               ( (profile) ? profile->colorProfile.c_str() : "<null>")
               );
#endif // DEBUG_LCMS
    if ( _prof && (!profile || (_profileName != profile->colorProfile) ) ) {
        // Need to clear out the prior one
        _profileName.clear();
        _profIntent = Inkscape::RENDERING_INTENT_UNKNOWN;
        _profileSpace = icSigRgbData;
        _profileClass = icSigInputClass;
        _prof = 0;
        if ( _transf ) {
            cmsDeleteTransform( _transf );
            _transf = 0;
        }
        if ( _destProf ) {
            cmsCloseProfile( _destProf );
            _destProf = 0;
        }
        _profChannelCount = 0;
    }

    if ( profile ) {
        _prof = Inkscape::colorprofile_get_handle( inkscape_active_document(),//SP_OBJECT_DOCUMENT( object ),
                                                   &_profIntent,
                                                   profile->colorProfile.c_str() );
        if ( _prof ) {
            _profileSpace = cmsGetColorSpace( _prof );
            _profileClass = cmsGetDeviceClass( _prof );
            if ( _profileClass != icSigNamedColorClass ) {
                int intent = INTENT_PERCEPTUAL;
                switch ( _profIntent ) {
                    case Inkscape::RENDERING_INTENT_RELATIVE_COLORIMETRIC:
                        intent = INTENT_RELATIVE_COLORIMETRIC;
                        break;
                    case Inkscape::RENDERING_INTENT_SATURATION:
                        intent = INTENT_SATURATION;
                        break;
                    case Inkscape::RENDERING_INTENT_ABSOLUTE_COLORIMETRIC:
                        intent = INTENT_ABSOLUTE_COLORIMETRIC;
                        break;
                    case Inkscape::RENDERING_INTENT_PERCEPTUAL:
                    case Inkscape::RENDERING_INTENT_UNKNOWN:
                    case Inkscape::RENDERING_INTENT_AUTO:
                    default:
                        intent = INTENT_PERCEPTUAL;
                }
                _destProf = cmsCreate_sRGBProfile();

                _profChannelCount = _cmsChannelsOf( _profileSpace );

                DWORD inputFormat = TYPE_RGB_16;
                gchar const** names = 0;
                gchar const** tips = 0;
                bool const* tricky = 0;
                getThings( _profileSpace, names, tips, tricky, _fooScales, inputFormat );

                _transf = cmsCreateTransform( _prof, inputFormat, _destProf, TYPE_RGBA_8, intent, 0 );
                (void)names;
                (void)tips;
                (void)tricky;


            } else {
                // Give up for now on named colors
                _prof = 0;
            }
        }
    }
#ifdef DEBUG_LCMS
    g_message( "\\_________  %p::_setProfile()", this );
#endif // DEBUG_LCMS
}
#endif // ENABLE_LCMS

void ColorICCSelector::_updateSliders()
{
#ifdef ENABLE_LCMS
    if ( _color.icc )
    {
        DWORD inputFormat = TYPE_RGB_16;
        gchar const** names = 0;
        gchar const** tips = 0;
        bool const* tricky = 0;
        getThings( _profileSpace, names, tips, tricky, _fooScales, inputFormat );

        for ( guint i = 0; i < _fooCount; i++ ) {
            gtk_label_set_text_with_mnemonic( GTK_LABEL(_fooLabel[i]), names[i]);

            gtk_tooltips_set_tip( _tt, _fooSlider[i], tips[i], NULL );
            gtk_tooltips_set_tip( _tt, _fooBtn[i], tips[i], NULL );

            sp_color_slider_set_colors( SP_COLOR_SLIDER(_fooSlider[i]),
                                        SPColor(0.0, 0.0, 0.0).toRGBA32(0xff),
                                        SPColor(0.5, 0.5, 0.5).toRGBA32(0xff),
                                        SPColor(1.0, 1.0, 1.0).toRGBA32(0xff) );

            if ( i < _profChannelCount ) {
/*
                gdouble step = static_cast<gdouble>(_fooScales[i]) / 100.0;
                gdouble page = static_cast<gdouble>(_fooScales[i]) / 10.0;
                gint digits = (step > 0.9) ? 0 : 2;
*/
                gdouble val = (_color.icc->colors.size() > i) ? (_color.icc->colors[i] / static_cast<gdouble>(_fooScales[i])) : 0.0;

/*
                _fooAdj[i] = GTK_ADJUSTMENT( gtk_adjustment_new( val, 0.0, _fooScales[i],  step, page, page ) );
                gtk_signal_connect( GTK_OBJECT( _fooAdj[i] ), "value_changed", GTK_SIGNAL_FUNC( _adjustmentChanged ), _csel );

                sp_color_slider_set_adjustment( SP_COLOR_SLIDER(_fooSlider[i]), _fooAdj[i] );
                gtk_spin_button_set_adjustment( GTK_SPIN_BUTTON(_fooBtn[i]), _fooAdj[i] );
                gtk_spin_button_set_digits( GTK_SPIN_BUTTON(_fooBtn[i]), digits );
*/
                gtk_widget_show( _fooLabel[i] );
                gtk_widget_show( _fooSlider[i] );
                gtk_widget_show( _fooBtn[i] );
                gtk_adjustment_set_value( _fooAdj[i], val );
            } else {
                gtk_widget_hide( _fooLabel[i] );
                gtk_widget_hide( _fooSlider[i] );
                gtk_widget_hide( _fooBtn[i] );
                gtk_adjustment_set_value( _fooAdj[i], 0.0 );
            }

        }

        if ( _transf ) {
            for ( guint i = 0; i < _profChannelCount; i++ ) {
                if ( tricky[i] ) {
                    icUInt16Number* scratch = getScratch();
                    icUInt16Number filler[4] = {0, 0, 0, 0};
                    for ( guint j = 0; j < _profChannelCount; j++ ) {
                        filler[j] = 0x0ffff * ColorScales::getScaled( _fooAdj[i] );
                    }

                    icUInt16Number* p = scratch;
                    for ( guint x = 0; x < 1024; x++ ) {
                        if ( _profileSpace == icSigLabData && (i == 1 || i == 2) ) {
                            p = scratch + 3 * ( (x + 512) % 1024);
                        }
                        for ( guint j = 0; j < _profChannelCount; j++ ) {
                            if ( j == i ) {
                                *p++ = x * 0x0ffff / 1024;
                            } else {
                                *p++ = filler[j];
                            }
                        }
                    }

                    cmsDoTransform( _transf, scratch, _fooMap[i], 1024 );
                    sp_color_slider_set_map( SP_COLOR_SLIDER(_fooSlider[i]), _fooMap[i] );
                } else {
                    icUInt16Number tmp[4];
                    for ( guint j = 0; j < _profChannelCount; j++ ) {
                        tmp[j] = ColorScales::getScaled( _fooAdj[j] ) * 0x0ffff;
                    }
                    tmp[i] = 0;
                    guchar post[4] = {0,0,0,0};
                    cmsDoTransform( _transf, tmp, post, 1 );
                    guint32 lowColor = SP_RGBA32_U_COMPOSE(post[0], post[1], post[2], 255);

                    tmp[i] = 0x0ffff/2;
                    cmsDoTransform( _transf, tmp, post, 1 );
                    guint32 midColor = SP_RGBA32_U_COMPOSE(post[0], post[1], post[2], 255);

                    tmp[i] = 0x0ffff;
                    cmsDoTransform( _transf, tmp, post, 1 );
                    guint32 highColor = SP_RGBA32_U_COMPOSE(post[0], post[1], post[2], 255);

                    sp_color_slider_set_colors( SP_COLOR_SLIDER(_fooSlider[i]), lowColor, midColor, highColor );
                }
            }
        }
    }
#endif // ENABLE_LCMS

    guint32 start = _color.toRGBA32( 0x00 );
    guint32 mid = _color.toRGBA32( 0x7f );
    guint32 end = _color.toRGBA32( 0xff );

    sp_color_slider_set_colors( SP_COLOR_SLIDER(_slider), start, mid, end );

}


void ColorICCSelector::_adjustmentChanged( GtkAdjustment *adjustment, SPColorICCSelector *cs )
{
// // TODO check this. It looks questionable:
//     // if a value is entered between 0 and 1 exclusive, normalize it to (int) 0..255  or 0..100
//     if (adjustment->value > 0.0 && adjustment->value < 1.0) {
//         gtk_adjustment_set_value( adjustment, floor ((adjustment->value) * adjustment->upper + 0.5) );
//     }

#ifdef DEBUG_LCMS
    g_message( "/^^^^^^^^^  %p::_adjustmentChanged()", cs );
#endif // DEBUG_LCMS

     ColorICCSelector* iccSelector = (ColorICCSelector*)(SP_COLOR_SELECTOR(cs)->base);
     if (iccSelector->_updating) {
         return;
     }

     iccSelector->_updating = TRUE;

     SPColor newColor( iccSelector->_color );
     gfloat scaled = ColorScales::getScaled( iccSelector->_adj );
     if ( iccSelector->_adj == adjustment ) {
#ifdef DEBUG_LCMS
         g_message("ALPHA");
#endif // DEBUG_LCMS
     } else {
         gint match = -1;
         for ( guint i = 0; i < iccSelector->_fooCount; i++ ) {
             if ( iccSelector->_fooAdj[i] == adjustment ) {
                 match = i;
                 break;
             }
         }
         if ( match >= 0 ) {
#ifdef DEBUG_LCMS
             g_message(" channel %d", match );
#endif // DEBUG_LCMS
         }


         icUInt16Number tmp[4];
         for ( guint i = 0; i < 4; i++ ) {
             tmp[i] = ColorScales::getScaled( iccSelector->_fooAdj[i] ) * 0x0ffff;
         }
         guchar post[4] = {0,0,0,0};

         cmsDoTransform( iccSelector->_transf, tmp, post, 1 );

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
             for ( guint i = 0; i < iccSelector->_profChannelCount; i++ ) {
                 gdouble val = ColorScales::getScaled( iccSelector->_fooAdj[i] );
                 if ( iccSelector->_fooScales ) {
                     val *= iccSelector->_fooScales[i];
                 }
                 newColor.icc->colors.push_back( val );
             }
         }

     }
     iccSelector->_updateInternals( newColor, scaled, iccSelector->_dragging );
     iccSelector->_updateSliders();

     iccSelector->_updating = FALSE;
#ifdef DEBUG_LCMS
     g_message( "\\_________  %p::_adjustmentChanged()", cs );
#endif // DEBUG_LCMS
}

void ColorICCSelector::_sliderGrabbed( SPColorSlider *slider, SPColorICCSelector *cs )
{
    (void)slider;
    (void)cs;
//    ColorICCSelector* iccSelector = (ColorICCSelector*)(SP_COLOR_SELECTOR(cs)->base);
//     if (!iccSelector->_dragging) {
//         iccSelector->_dragging = TRUE;
//         iccSelector->_grabbed();
//         iccSelector->_updateInternals( iccSelector->_color, ColorScales::getScaled( iccSelector->_adj ), iccSelector->_dragging );
//     }
}

void ColorICCSelector::_sliderReleased( SPColorSlider *slider, SPColorICCSelector *cs )
{
    (void)slider;
    (void)cs;
//     ColorICCSelector* iccSelector = (ColorICCSelector*)(SP_COLOR_SELECTOR(cs)->base);
//     if (iccSelector->_dragging) {
//         iccSelector->_dragging = FALSE;
//         iccSelector->_released();
//         iccSelector->_updateInternals( iccSelector->_color, ColorScales::getScaled( iccSelector->_adj ), iccSelector->_dragging );
//     }
}

void ColorICCSelector::_sliderChanged( SPColorSlider *slider, SPColorICCSelector *cs )
{
    (void)slider;
    (void)cs;
#ifdef DEBUG_LCMS
    g_message("Changed  %p and %p", slider, cs );
#endif // DEBUG_LCMS
//     ColorICCSelector* iccSelector = (ColorICCSelector*)(SP_COLOR_SELECTOR(cs)->base);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
