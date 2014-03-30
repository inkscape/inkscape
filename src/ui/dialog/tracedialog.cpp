/**
 * @file
 * Bitmap tracing settings dialog - implementation.
 */
/* Authors:
 *   Bob Jamison <rjamison@titan.com>
 *   Stéphane Gimenez <dev@gim.name>
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004-2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "tracedialog.h"
#include <gtkmm/notebook.h>
#include <gtkmm/frame.h>
#include "ui/widget/spinbutton.h"
#include "ui/widget/frame.h"
#include <gtkmm/radiobutton.h>
#include <gtkmm/stock.h>

#include <gtk/gtk.h> //for GTK_RESPONSE* types
#include <glibmm/i18n.h>

#include "desktop.h"
#include "desktop-tracker.h"
#include "selection.h"

#include "trace/potrace/inkscape-potrace.h"
#include "inkscape.h"


namespace Inkscape {
namespace UI {
namespace Dialog {


//#########################################################################
//## I M P L E M E N T A T I O N
//#########################################################################

/**
 * A dialog for adjusting bitmap->vector tracing parameters
 */
class TraceDialogImpl : public TraceDialog
{

    public:


    /**
     * Constructor
     */
    TraceDialogImpl();

    /**
     * Destructor
     */
    ~TraceDialogImpl();

    /**
     * Callback from OK or Cancel
     */
    void responseCallback(int response_id);

    private:

    /**
     * This is the big almighty McGuffin
     */
    Inkscape::Trace::Tracer tracer;

    /**
     * This does potrace processing
     * Only preview if do_i_trace is false
     */
    void potraceProcess(bool do_i_trace);

    /**
     * Abort processing
     */
    void abort();

    void previewCallback();
    void previewLiveCallback();
    void onSettingsChange();
    void onSelectionModified( guint flags );
    void onSetDefaults();

    void setDesktop(SPDesktop *desktop);
    void setTargetDesktop(SPDesktop *desktop);

    //############ General items

    Gtk::HBox             mainHBox;

    Gtk::Button           *mainOkButton;
    Gtk::Button           *mainCancelButton;
    Gtk::Button           *mainResetButton;

    //######## Left pannel

    Gtk::VBox             leftVBox;

    //#### Notebook

    Gtk::Notebook         notebook;

    //## Modes

    Gtk::VBox             modePageBox;
    Gtk::RadioButtonGroup modeGroup;

    //# Single scan mode
    //brightness
    UI::Widget::Frame     modeBrightnessFrame;
    Gtk::VBox             modeBrightnessVBox;
    Gtk::HBox             modeBrightnessBox;
    Gtk::RadioButton      modeBrightnessRadioButton;
    Gtk::Label            modeBrightnessSpinnerLabel;
    Inkscape::UI::Widget::SpinButton modeBrightnessSpinner;
    //edge detection
    UI::Widget::Frame     modeCannyFrame;
    Gtk::HBox             modeCannyBox;
    Gtk::VBox             modeCannyVBox;
    Gtk::RadioButton      modeCannyRadioButton;
    //Gtk::HSeparator     modeCannySeparator;
    //Gtk::Label          modeCannyLoSpinnerLabel;
    //Inkscape::UI::Widget::SpinButton     modeCannyLoSpinner;
    Gtk::Label            modeCannyHiSpinnerLabel;
    Inkscape::UI::Widget::SpinButton       modeCannyHiSpinner;
    //quantization
    UI::Widget::Frame     modeQuantFrame;
    Gtk::HBox             modeQuantBox;
    Gtk::VBox             modeQuantVBox;
    Gtk::RadioButton      modeQuantRadioButton;
    Gtk::Label            modeQuantNrColorLabel;
    Inkscape::UI::Widget::SpinButton       modeQuantNrColorSpinner;
    //params
    Gtk::CheckButton      modeInvertButton;
    Gtk::HBox             modeInvertBox;

    //# Multiple path scanning mode
    UI::Widget::Frame     modeMultiScanFrame;
    Gtk::VBox             modeMultiScanVBox;
    //brightness
    Gtk::HBox             modeMultiScanHBox1;
    Gtk::RadioButton      modeMultiScanBrightnessRadioButton;
    Inkscape::UI::Widget::SpinButton       modeMultiScanNrColorSpinner;
    //colors
    Gtk::HBox             modeMultiScanHBox2;
    Gtk::RadioButton      modeMultiScanColorRadioButton;
    //grays
    Gtk::HBox             modeMultiScanHBox3;
    Gtk::RadioButton      modeMultiScanMonoRadioButton;
    Gtk::Label            modeMultiScanNrColorLabel;
    //params
    Gtk::HBox             modeMultiScanHBox4;
    Gtk::CheckButton      modeMultiScanStackButton;
    Gtk::CheckButton      modeMultiScanSmoothButton;
    Gtk::CheckButton      modeMultiScanBackgroundButton;

    //## Options

    Gtk::VBox             optionsPageBox;

    // potrace parameters

    UI::Widget::Frame     optionsFrame;
    Gtk::VBox             optionsVBox;
    Gtk::HBox             optionsSpecklesBox;
    Gtk::CheckButton      optionsSpecklesButton;
    Gtk::Label            optionsSpecklesSizeLabel;
    Inkscape::UI::Widget::SpinButton optionsSpecklesSizeSpinner;
    Gtk::HBox             optionsCornersBox;
    Gtk::CheckButton      optionsCornersButton;
    Gtk::Label            optionsCornersThresholdLabel;
    Inkscape::UI::Widget::SpinButton optionsCornersThresholdSpinner;
    Gtk::HBox             optionsOptimBox;
    Gtk::CheckButton      optionsOptimButton;
    Gtk::Label            optionsOptimToleranceLabel;
    Inkscape::UI::Widget::SpinButton optionsOptimToleranceSpinner;


    //#### Credits

    Gtk::VBox             potraceCreditsVBox;
    Gtk::Label            potraceCreditsLabel;

    //######## Right pannel

    Gtk::VBox             rightVBox;

    //#### SIOX selection

    Gtk::HBox             sioxBox;
    Gtk::CheckButton      sioxButton;

    //#### Preview

    UI::Widget::Frame     previewFrame;
    Gtk::VBox             previewVBox;
    Gtk::Button           previewButton;
    Gtk::CheckButton      previewLiveButton;
    gboolean              previewLive;
    Gtk::Image            previewImage;

    SPDesktop *desktop;
    DesktopTracker deskTrack;
    sigc::connection desktopChangeConn;
    sigc::connection selectChangedConn;
    sigc::connection selectModifiedConn;

};

void TraceDialogImpl::setDesktop(SPDesktop *desktop)
{
    Panel::setDesktop(desktop);
    deskTrack.setBase(desktop);
}

void TraceDialogImpl::setTargetDesktop(SPDesktop *desktop)
{
    if (this->desktop != desktop) {
        if (this->desktop) {
            selectChangedConn.disconnect();
            selectModifiedConn.disconnect();
        }
        this->desktop = desktop;
        if (desktop && desktop->selection) {
            selectChangedConn = desktop->selection->connectChanged(sigc::hide(sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange)));
            selectModifiedConn = desktop->selection->connectModified(sigc::hide<0>(sigc::mem_fun(*this, &TraceDialogImpl::onSelectionModified)));

        }
        onSettingsChange();
    }
}

//#########################################################################
//## E V E N T S
//#########################################################################

/**
 * This does potrace processing
 * Only preview if do_i_trace is false
 */
void TraceDialogImpl::potraceProcess(bool do_i_trace)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop)
        desktop->setWaitingCursor();

    //##### Get the tracer and engine
    Inkscape::Trace::Potrace::PotraceTracingEngine pte;

    /* inversion */
    bool invert = modeInvertButton.get_active();
    pte.setInvert(invert);

    //##### Get the preprocessor settings
    /* siox -- performed by Tracer, and before any of the others */
    if (sioxButton.get_active())
        tracer.enableSiox(true);
    else
        tracer.enableSiox(false);

    /* one of the following */
    if (modeBrightnessRadioButton.get_active())
        pte.setTraceType(Inkscape::Trace::Potrace::TRACE_BRIGHTNESS);
    else if (modeMultiScanBrightnessRadioButton.get_active())
        pte.setTraceType(Inkscape::Trace::Potrace::TRACE_BRIGHTNESS_MULTI);
    else if (modeCannyRadioButton.get_active())
        pte.setTraceType(Inkscape::Trace::Potrace::TRACE_CANNY);
    else if (modeQuantRadioButton.get_active())
        pte.setTraceType(Inkscape::Trace::Potrace::TRACE_QUANT);
    else if (modeMultiScanColorRadioButton.get_active())
        {
        pte.setTraceType(Inkscape::Trace::Potrace::TRACE_QUANT_COLOR);
        pte.setInvert(false);
        }
    else if (modeMultiScanMonoRadioButton.get_active())
        {
        pte.setTraceType(Inkscape::Trace::Potrace::TRACE_QUANT_MONO);
        pte.setInvert(false);
        }

    /* params */
    int paramsSpecklesSize =
      optionsSpecklesButton.get_active() ?
      optionsSpecklesSizeSpinner.get_value_as_int() :
      0;
    pte.setParamsTurdSize(paramsSpecklesSize);
    double paramsCornersThreshold =
      optionsCornersButton.get_active() ?
      optionsCornersThresholdSpinner.get_value() :
      0.;
    pte.setParamsAlphaMax(paramsCornersThreshold);
    bool paramsOptim = optionsOptimButton.get_active();
    pte.setParamsOptiCurve(paramsOptim);
    double paramsOptimTolerance = optionsOptimToleranceSpinner.get_value();
    pte.setParamsOptTolerance(paramsOptimTolerance);

    //##### Get the single-scan settings
    /* brightness */
    double brightnessThreshold = modeBrightnessSpinner.get_value();
    pte.setBrightnessThreshold(brightnessThreshold);
    /* canny */
    double cannyHighThreshold = modeCannyHiSpinner.get_value();
    pte.setCannyHighThreshold(cannyHighThreshold);
    /* quantization */
    int quantNrColors = modeQuantNrColorSpinner.get_value_as_int();
    pte.setQuantizationNrColors(quantNrColors);

    //##### Get multiple-scan settings
    int multiScanNrColors = modeMultiScanNrColorSpinner.get_value_as_int();
    pte.setMultiScanNrColors(multiScanNrColors);
    bool do_i_stack = modeMultiScanStackButton.get_active();
    pte.setMultiScanStack(do_i_stack);
    bool do_i_smooth = modeMultiScanSmoothButton.get_active();
    pte.setMultiScanSmooth(do_i_smooth);
    bool do_i_remove_background = modeMultiScanBackgroundButton.get_active();
    pte.setMultiScanRemoveBackground(do_i_remove_background);

    //##### Get intermediate bitmap image
    Glib::RefPtr<Gdk::Pixbuf> pixbuf = tracer.getSelectedImage();
    if (pixbuf)
         {
         Glib::RefPtr<Gdk::Pixbuf> preview = pte.preview(pixbuf);
         if (preview)
             {
             int width  = preview->get_width();
             int height = preview->get_height();
             const Gtk::Allocation& vboxAlloc = previewImage.get_allocation();
             double scaleFX = vboxAlloc.get_width() / (double)width;
             double scaleFY = vboxAlloc.get_height() / (double)height;
             double scaleFactor = scaleFX > scaleFY ? scaleFY : scaleFX;
             int newWidth  = (int) (((double)width)  * scaleFactor);
             int newHeight = (int) (((double)height) * scaleFactor);
             Glib::RefPtr<Gdk::Pixbuf> scaledPreview =
                    preview->scale_simple(newWidth, newHeight,
                       Gdk::INTERP_NEAREST);
             //g_object_unref(preview);
             previewImage.set(scaledPreview);
             }
         }

    //##### Convert
    if (do_i_trace)
        {
        if (mainCancelButton)
            mainCancelButton->set_sensitive(true);
        if (mainOkButton)
            mainOkButton->set_sensitive(false);
        tracer.trace(&pte);
        if (mainCancelButton)
            mainCancelButton->set_sensitive(false);
        if (mainOkButton)
            mainOkButton->set_sensitive(true);
        }

    if (desktop)
        desktop->clearWaitingCursor();
}


/**
 * Abort processing
 */
void TraceDialogImpl::abort()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop)
        desktop->setWaitingCursor();

    if (mainCancelButton)
        mainCancelButton->set_sensitive(false);
    if (mainOkButton)
        mainOkButton->set_sensitive(true);

    //### Make the abort() call to the tracer
    tracer.abort();
}



//#########################################################################
//## E V E N T S
//#########################################################################

/**
 * Callback for when any setting changes
 */
void TraceDialogImpl::onSettingsChange()
{
    if (previewLive) {
        previewCallback();
    }
}

void TraceDialogImpl::onSelectionModified( guint flags )
{
    if (flags & ( SP_OBJECT_MODIFIED_FLAG |
                   SP_OBJECT_PARENT_MODIFIED_FLAG |
                   SP_OBJECT_STYLE_MODIFIED_FLAG) ) {
        onSettingsChange();
    }
}

/**
 * Callback for when users resets defaults
 */
void TraceDialogImpl::onSetDefaults()
{

    // temporarily disable live update
    gboolean wasLive = previewLive;
    previewLive = false;

    modeBrightnessRadioButton.set_active(true);
    modeBrightnessSpinner.set_value(0.45);
    modeCannyHiSpinner.set_value(0.65);
    modeMultiScanNrColorSpinner.set_value(8.0);
    modeMultiScanNrColorSpinner.set_value(8.0);
    optionsSpecklesSizeSpinner.set_value(2);
    optionsCornersThresholdSpinner.set_value(1.0);
    optionsOptimToleranceSpinner.set_value(0.2);

    modeInvertButton.set_active(false);
    modeMultiScanSmoothButton.set_active(true);
    modeMultiScanStackButton.set_active(true);
    modeMultiScanBackgroundButton.set_active(false);
    optionsSpecklesButton.set_active(true);
    optionsCornersButton.set_active(true);
    optionsOptimButton.set_active(true);
    sioxButton.set_active(false);

    previewLive = wasLive;
    onSettingsChange();

}

/**
 * Callback from the Preview button.  Can be called from elsewhere.
 */
void TraceDialogImpl::previewCallback()
{
    potraceProcess(false);
}

/**
 * Callback from the Preview Live button.
 */
void TraceDialogImpl::previewLiveCallback()
{
    previewLive = previewLiveButton.get_active();
    previewButton.set_sensitive(!previewLive);
    onSettingsChange();
}

/**
 * Default response from the dialog.  Let's intercept it
 */
void TraceDialogImpl::responseCallback(int response_id)
{
    if (response_id == GTK_RESPONSE_OK) {
        // for now, we assume potrace, as it's the only one we have
        potraceProcess(true);
    } else if (response_id == GTK_RESPONSE_CANCEL) {
        abort();
    } else if (response_id == GTK_RESPONSE_HELP) {
        onSetDefaults();
    } else {
        hide();
        return;
    }

}



//#########################################################################
//## C O N S T R U C T O R    /    D E S T R U C T O R
//#########################################################################
/**
 * Constructor
 */
TraceDialogImpl::TraceDialogImpl() :
    TraceDialog()
{

    Gtk::Box *contents = _getContents();

#define MARGIN 2
    //#### begin left panel

    //### begin notebook

    //## begin mode page

    //# begin single scan

    // brightness

    modeBrightnessRadioButton.set_label(_("_Brightness cutoff"));
    modeGroup = modeBrightnessRadioButton.get_group();
    modeBrightnessRadioButton.set_use_underline(true);
    modeBrightnessBox.pack_start(modeBrightnessRadioButton, false, false, MARGIN);
    modeBrightnessRadioButton.set_tooltip_text(_("Trace by a given brightness level"));

    modeBrightnessSpinner.set_digits(3);
    modeBrightnessSpinner.set_increments(0.01, 0);
    modeBrightnessSpinner.set_range(0.0, 1.0);
    modeBrightnessSpinner.set_value(0.45);
    modeBrightnessBox.pack_end(modeBrightnessSpinner, false, false, MARGIN);
    modeBrightnessSpinner.set_tooltip_text(_("Brightness cutoff for black/white"));
    modeBrightnessSpinner.get_adjustment()->signal_value_changed().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );

    modeBrightnessSpinnerLabel.set_label(_("_Threshold:"));
    modeBrightnessSpinnerLabel.set_use_underline(true);
    modeBrightnessSpinnerLabel.set_mnemonic_widget(modeBrightnessSpinner);
    modeBrightnessBox.pack_end(modeBrightnessSpinnerLabel, false, false, MARGIN);

    modeBrightnessVBox.pack_start(modeBrightnessBox, false, false, MARGIN);

    modeBrightnessFrame.set_label(_("Single scan: creates a path"));

    // canny edge detection
    // TRANSLATORS: "Canny" is the name of the inventor of this edge detection method

    modeCannyRadioButton.set_label(_("_Edge detection"));
    modeCannyRadioButton.set_group(modeGroup);
    modeCannyRadioButton.set_use_underline(true);
    modeCannyBox.pack_start(modeCannyRadioButton, false, false, MARGIN);
    modeCannyRadioButton.set_tooltip_text(_("Trace with optimal edge detection by J. Canny's algorithm"));
    modeCannyRadioButton.signal_clicked().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );

    /*
    modeCannyBox.pack_start(modeCannySeparator);
    modeCannyLoSpinnerLabel.set_label(_("Low"));
    modeCannyBox.pack_start(modeCannyLoSpinnerLabel);
    modeCannyLoSpinner.set_digits(5);
    modeCannyLoSpinner.set_increments(0.01, 0);
    modeCannyLoSpinner.set_range(0.0, 1.0);
    modeCannyLoSpinner.set_value(0.1);
    modeCannyBox.pack_start(modeCannyLoSpinner);
    */
    modeCannyHiSpinner.set_digits(3);
    modeCannyHiSpinner.set_increments(0.01, 0);
    modeCannyHiSpinner.set_range(0.0, 1.0);
    modeCannyHiSpinner.set_value(0.65);
    modeCannyBox.pack_end(modeCannyHiSpinner, false, false, MARGIN);
    modeCannyHiSpinner.set_tooltip_text(_("Brightness cutoff for adjacent pixels (determines edge thickness)"));
    modeCannyHiSpinner.get_adjustment()->signal_value_changed().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );

    modeCannyHiSpinnerLabel.set_label(_("T_hreshold:"));
    modeCannyHiSpinnerLabel.set_use_underline(true);
    modeCannyHiSpinnerLabel.set_mnemonic_widget(modeCannyHiSpinner);
    modeCannyBox.pack_end(modeCannyHiSpinnerLabel, false, false, MARGIN);

    modeBrightnessVBox.pack_start(modeCannyBox, false, false, MARGIN);

    // quantization
    // TRANSLATORS: Color Quantization: the process of reducing the number
    // of colors in an image by selecting an optimized set of representative
    // colors and then re-applying this reduced set to the original image.

    modeQuantRadioButton.set_label(_("Color _quantization"));
    modeQuantRadioButton.set_group(modeGroup);
    modeQuantRadioButton.set_use_underline(true);
    modeQuantBox.pack_start(modeQuantRadioButton, false, false, MARGIN);
    modeQuantRadioButton.set_tooltip_text(_("Trace along the boundaries of reduced colors"));
    modeQuantRadioButton.signal_clicked().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );

    modeQuantNrColorSpinner.set_digits(0);
    modeQuantNrColorSpinner.set_increments(1.0, 0);
    modeQuantNrColorSpinner.set_range(2.0, 64.0);
    modeQuantNrColorSpinner.set_value(8.0);
    modeQuantBox.pack_end(modeQuantNrColorSpinner, false, false, MARGIN);
    modeQuantNrColorSpinner.set_tooltip_text(_("The number of reduced colors"));
    modeQuantNrColorSpinner.get_adjustment()->signal_value_changed().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );

    modeQuantNrColorLabel.set_label(_("_Colors:"));
    modeQuantNrColorLabel.set_mnemonic_widget(modeQuantNrColorSpinner);
    modeQuantNrColorLabel.set_use_underline(true);
    modeQuantBox.pack_end(modeQuantNrColorLabel, false, false, MARGIN);

    modeBrightnessVBox.pack_start(modeQuantBox, false, false, MARGIN);

    // swap black and white
    modeInvertButton.set_label(_("_Invert image"));
    modeInvertButton.set_active(false);
    modeInvertButton.set_use_underline(true);
    modeInvertBox.pack_start(modeInvertButton, false, false, MARGIN);
    modeBrightnessVBox.pack_start(modeInvertBox, false, false, MARGIN);
    modeInvertButton.set_tooltip_text(_("Invert black and white regions"));
    modeInvertButton.signal_clicked().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );

    modeBrightnessFrame.add(modeBrightnessVBox);
    modePageBox.pack_start(modeBrightnessFrame, false, false, 0);

    //# end single scan

    //# begin multiple scan

    modeMultiScanBrightnessRadioButton.set_label(_("B_rightness steps"));
    modeMultiScanBrightnessRadioButton.set_group(modeGroup);
    modeMultiScanBrightnessRadioButton.set_use_underline(true);
    modeMultiScanHBox1.pack_start(modeMultiScanBrightnessRadioButton, false, false, MARGIN);
    modeMultiScanBrightnessRadioButton.set_tooltip_text(_("Trace the given number of brightness levels"));
    modeMultiScanBrightnessRadioButton.signal_clicked().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );

    modeMultiScanNrColorSpinner.set_digits(0);
    modeMultiScanNrColorSpinner.set_increments(1.0, 0);
    modeMultiScanNrColorSpinner.set_range(2.0, 256.0);
    modeMultiScanNrColorSpinner.set_value(8.0);
    modeMultiScanHBox1.pack_end(modeMultiScanNrColorSpinner, false, false, MARGIN);
    modeMultiScanNrColorLabel.set_label(_("Sc_ans:"));
    modeMultiScanNrColorLabel.set_use_underline(true);
    modeMultiScanNrColorLabel.set_mnemonic_widget(modeMultiScanNrColorSpinner);
    modeMultiScanHBox1.pack_end(modeMultiScanNrColorLabel, false, false, MARGIN);
    modeMultiScanNrColorSpinner.set_tooltip_text(_("The desired number of scans"));
    modeMultiScanNrColorSpinner.get_adjustment()->signal_value_changed().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );

    modeMultiScanVBox.pack_start(modeMultiScanHBox1, false, false, MARGIN);

    modeMultiScanColorRadioButton.set_label(_("Co_lors"));
    modeMultiScanColorRadioButton.set_group(modeGroup);
    modeMultiScanColorRadioButton.set_use_underline(true);
    modeMultiScanHBox2.pack_start(modeMultiScanColorRadioButton, false, false, MARGIN);
    modeMultiScanColorRadioButton.set_tooltip_text(_("Trace the given number of reduced colors"));
    modeMultiScanColorRadioButton.signal_clicked().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );

    modeMultiScanVBox.pack_start(modeMultiScanHBox2, false, false, MARGIN);

    modeMultiScanMonoRadioButton.set_label(_("_Grays"));
    modeMultiScanMonoRadioButton.set_group(modeGroup);
    modeMultiScanMonoRadioButton.set_use_underline(true);
    modeMultiScanHBox3.pack_start(modeMultiScanMonoRadioButton, false, false, MARGIN);
    modeMultiScanMonoRadioButton.set_tooltip_text(_("Same as Colors, but the result is converted to grayscale"));
    modeMultiScanMonoRadioButton.signal_clicked().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );

    modeMultiScanVBox.pack_start(modeMultiScanHBox3, false, false, MARGIN);

    // TRANSLATORS: "Smooth" is a verb here
    modeMultiScanSmoothButton.set_label(_("S_mooth"));
    modeMultiScanSmoothButton.set_use_underline(true);
    modeMultiScanSmoothButton.set_active(true);
    modeMultiScanHBox4.pack_start(modeMultiScanSmoothButton, false, false, MARGIN);
    modeMultiScanSmoothButton.set_tooltip_text(_("Apply Gaussian blur to the bitmap before tracing"));
    modeMultiScanSmoothButton.signal_clicked().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );

    // TRANSLATORS: "Stack" is a verb here
    modeMultiScanStackButton.set_label(_("Stac_k scans"));
    modeMultiScanStackButton.set_use_underline(true);
    modeMultiScanStackButton.set_active(true);
    modeMultiScanHBox4.pack_start(modeMultiScanStackButton, false, false, MARGIN);
    modeMultiScanStackButton.set_tooltip_text(_("Stack scans on top of one another (no gaps) instead of tiling (usually with gaps)"));
    modeMultiScanStackButton.signal_clicked().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );


    modeMultiScanBackgroundButton.set_label(_("Remo_ve background"));
    modeMultiScanBackgroundButton.set_use_underline(true);
    modeMultiScanBackgroundButton.set_active(false);
    modeMultiScanHBox4.pack_start(modeMultiScanBackgroundButton, false, false, MARGIN);
    // TRANSLATORS: "Layer" refers to one of the stacked paths in the multiscan
    modeMultiScanBackgroundButton.set_tooltip_text(_("Remove bottom (background) layer when done"));
    modeMultiScanBackgroundButton.signal_clicked().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );

    modeMultiScanVBox.pack_start(modeMultiScanHBox4, false, false, MARGIN);

    modeMultiScanFrame.set_label(_("Multiple scans: creates a group of paths"));
    //modeQuantFrame.set_shadow_type(Gtk::SHADOW_NONE);
    modeMultiScanFrame.add(modeMultiScanVBox);
    modePageBox.pack_start(modeMultiScanFrame, false, false, 0);

    //# end multiple scan

    //## end mode page

    notebook.append_page(modePageBox, _("_Mode"), true);

    //## begin option page

    //# potrace parameters

    optionsSpecklesButton.set_label(_("Suppress _speckles"));
    optionsSpecklesButton.set_use_underline(true);
    optionsSpecklesButton.set_tooltip_text(_("Ignore small spots (speckles) in the bitmap"));
    optionsSpecklesButton.set_active(true);
    optionsSpecklesButton.signal_clicked().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );
    optionsSpecklesBox.pack_start(optionsSpecklesButton, false, false, MARGIN);
    optionsSpecklesSizeSpinner.set_digits(0);
    optionsSpecklesSizeSpinner.set_increments(1, 0);
    optionsSpecklesSizeSpinner.set_range(0, 1000);
    optionsSpecklesSizeSpinner.set_value(2);
    optionsSpecklesSizeSpinner.set_tooltip_text(_("Speckles of up to this many pixels will be suppressed"));
    optionsSpecklesSizeSpinner.get_adjustment()->signal_value_changed().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );
    optionsSpecklesBox.pack_end(optionsSpecklesSizeSpinner, false, false, MARGIN);
    optionsSpecklesSizeLabel.set_label(_("S_ize:"));
    optionsSpecklesSizeLabel.set_use_underline(true);
    optionsSpecklesSizeLabel.set_mnemonic_widget(optionsSpecklesSizeSpinner);
    optionsSpecklesBox.pack_end(optionsSpecklesSizeLabel, false, false, MARGIN);

    optionsCornersButton.set_label(_("Smooth _corners"));
    optionsCornersButton.set_use_underline(true);
    optionsCornersButton.set_tooltip_text(_("Smooth out sharp corners of the trace"));
    optionsCornersButton.set_active(true);
    optionsCornersButton.signal_clicked().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );
    optionsCornersBox.pack_start(optionsCornersButton, false, false, MARGIN);
    optionsCornersThresholdSpinner.set_digits(2);
    optionsCornersThresholdSpinner.set_increments(0.01, 0);
    optionsCornersThresholdSpinner.set_range(0.0, 1.34);
    optionsCornersThresholdSpinner.set_value(1.0);
    optionsCornersBox.pack_end(optionsCornersThresholdSpinner, false, false, MARGIN);
    optionsCornersThresholdSpinner.set_tooltip_text(_("Increase this to smooth corners more"));
    optionsCornersThresholdSpinner.get_adjustment()->signal_value_changed().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );
    optionsCornersThresholdLabel.set_label(_("_Threshold:"));
    optionsCornersThresholdLabel.set_use_underline(true);
    optionsCornersThresholdLabel.set_mnemonic_widget(optionsCornersThresholdSpinner);
    optionsCornersBox.pack_end(optionsCornersThresholdLabel, false, false, MARGIN);

    optionsOptimButton.set_label(_("Optimize p_aths"));
    optionsOptimButton.set_use_underline(true);
    optionsOptimButton.set_active(true);
    optionsOptimButton.set_tooltip_text(_("Try to optimize paths by joining adjacent Bezier curve segments"));
    optionsOptimButton.signal_clicked().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );
    optionsOptimBox.pack_start(optionsOptimButton, false, false, MARGIN);
    optionsOptimToleranceSpinner.set_digits(2);
    optionsOptimToleranceSpinner.set_increments(0.05, 0);
    optionsOptimToleranceSpinner.set_range(0.0, 5.0);
    optionsOptimToleranceSpinner.set_value(0.2);
    optionsOptimBox.pack_end(optionsOptimToleranceSpinner, false, false, MARGIN);
    optionsOptimToleranceSpinner.set_tooltip_text(_("Increase this to reduce the number of nodes in the trace by more aggressive optimization"));
    optionsOptimToleranceSpinner.get_adjustment()->signal_value_changed().connect( sigc::mem_fun(*this, &TraceDialogImpl::onSettingsChange) );
    optionsOptimToleranceLabel.set_label(_("To_lerance:"));
    optionsOptimToleranceLabel.set_use_underline(true);
    optionsOptimToleranceLabel.set_mnemonic_widget(optionsOptimToleranceSpinner);
    optionsOptimBox.pack_end(optionsOptimToleranceLabel, false, false, MARGIN);

    optionsVBox.pack_start(optionsSpecklesBox, false, false, MARGIN);
    optionsVBox.pack_start(optionsCornersBox, false, false, MARGIN);
    optionsVBox.pack_start(optionsOptimBox, false, false, MARGIN);
    optionsFrame.set_label(_("Options"));
    optionsFrame.add(optionsVBox);
    optionsPageBox.pack_start(optionsFrame, false, false, 0);

    //## end option page

    notebook.append_page(optionsPageBox, _("O_ptions"), true);

    //### credits

    potraceCreditsLabel.set_text(_("Inkscape bitmap tracing\nis based on Potrace,\ncreated by Peter Selinger\n\nhttp://potrace.sourceforge.net"));
    potraceCreditsVBox.pack_start(potraceCreditsLabel, false, false, MARGIN);

    notebook.append_page(potraceCreditsVBox, _("Credits"));

    //### end notebook

    leftVBox.pack_start(notebook, true, true, MARGIN);

    //#### end left panel

    mainHBox.pack_start(leftVBox, false, false, 0);

    //#### begin right panel

    //## SIOX

    sioxButton.set_label(_("SIOX _foreground selection"));
    sioxButton.set_use_underline(true);
    sioxBox.pack_start(sioxButton, false, false, MARGIN);
    sioxButton.set_tooltip_text(_("Cover the area you want to select as the foreground"));
    rightVBox.pack_start(sioxBox, false, false, 0);

    //## preview
    Gtk::HBox *previewButtonHBox = Gtk::manage(new Gtk::HBox(false, MARGIN ));
    previewLiveButton.set_label(_("Live Preview"));
    previewLiveButton.set_use_underline(true);
    previewLiveCallback();
    previewLiveButton.signal_clicked().connect(
         sigc::mem_fun(*this, &TraceDialogImpl::previewLiveCallback) );

    previewButton.set_label(_("_Update"));
    previewButton.set_use_underline(true);
    previewButton.signal_clicked().connect(
         sigc::mem_fun(*this, &TraceDialogImpl::previewCallback) );
    previewButtonHBox->pack_start(previewLiveButton, false, false, 0);
    previewButtonHBox->pack_end(previewButton, true, true, 0);
    previewVBox.pack_end(*previewButtonHBox, false, false, 0);
    // I guess it's correct to call the "intermediate bitmap" a preview of the trace
    previewButton.set_tooltip_text(_("Preview the intermediate bitmap with the current settings, without actual tracing"));
    previewImage.set_size_request(200,200);
    //previewImage.set_alignment (Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER);
    previewVBox.pack_start(previewImage, true, true, 0);
    previewFrame.set_label(_("Preview"));
    //previewFrame.set_shadow_type(Gtk::SHADOW_NONE);
    previewFrame.add(previewVBox);

    rightVBox.pack_start(previewFrame, true, true, MARGIN);

    //#### end right panel

    mainHBox.pack_start(rightVBox, true, true, 0);

    //#### Global stuff

    contents->pack_start(mainHBox);
    mainResetButton = addResponseButton(_("Reset"), GTK_RESPONSE_HELP, true);
    mainResetButton ->set_tooltip_text(_("Reset all settings to defaults"));

    //## The OK button
    mainCancelButton = addResponseButton(Gtk::Stock::STOP, GTK_RESPONSE_CANCEL);
    if (mainCancelButton) {
	mainCancelButton->set_tooltip_text(_("Abort a trace in progress"));
	mainCancelButton->set_sensitive(false);
    }
    mainOkButton = addResponseButton(Gtk::Stock::OK, GTK_RESPONSE_OK);
    mainOkButton->set_tooltip_text(_("Execute the trace"));

    show_all_children();

    desktopChangeConn = deskTrack.connectDesktopChanged( sigc::mem_fun(*this, &TraceDialogImpl::setTargetDesktop) );
    deskTrack.connect(GTK_WIDGET(gobj()));

    //## Connect the signal
    signalResponse().connect(
	sigc::mem_fun(*this, &TraceDialogImpl::responseCallback));
}

/**
 * Factory method.  Use this to create a new TraceDialog
 */
TraceDialog &TraceDialog::getInstance()
{
    TraceDialog *dialog = new TraceDialogImpl();
    return *dialog;
}


/**
 * Constructor
 */
TraceDialogImpl::~TraceDialogImpl()
{
    selectChangedConn.disconnect();
    selectModifiedConn.disconnect();
    desktopChangeConn.disconnect();

}


} //namespace Dialog
} //namespace UI
} //namespace Inkscape

//#########################################################################
//## E N D    O F    F I L E
//#########################################################################



