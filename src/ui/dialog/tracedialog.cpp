/** @file
 * @brief Bitmap tracing settings dialog - implementation
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

#include <gtkmm/notebook.h>
#include <gtkmm/frame.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/stock.h>

#include <gtk/gtkdialog.h> //for GTK_RESPONSE* types
#include <glibmm/i18n.h>



#include "tracedialog.h"
#include "trace/potrace/inkscape-potrace.h"


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

    //############ General items

    Gtk::HBox             mainHBox;
    Gtk::Tooltips         tips;

    Gtk::Button           *mainOkButton;
    Gtk::Button           *mainCancelButton;

    //######## Left pannel

    Gtk::VBox             leftVBox;

    //#### Notebook

    Gtk::Notebook         notebook;

    //## Modes

    Gtk::VBox             modePageBox;
    Gtk::RadioButtonGroup modeGroup;

    //# Single scan mode
    //brightness
    Gtk::Frame            modeBrightnessFrame;
    Gtk::VBox             modeBrightnessVBox;
    Gtk::HBox             modeBrightnessBox;
    Gtk::RadioButton      modeBrightnessRadioButton;
    Gtk::Label            modeBrightnessSpinnerLabel;
    Gtk::SpinButton       modeBrightnessSpinner;
    //edge detection
    Gtk::Frame            modeCannyFrame;
    Gtk::HBox             modeCannyBox;
    Gtk::VBox             modeCannyVBox;
    Gtk::RadioButton      modeCannyRadioButton;
    //Gtk::HSeparator     modeCannySeparator;
    //Gtk::Label          modeCannyLoSpinnerLabel;
    //Gtk::SpinButton     modeCannyLoSpinner;
    Gtk::Label            modeCannyHiSpinnerLabel;
    Gtk::SpinButton       modeCannyHiSpinner;
    //quantization
    Gtk::Frame            modeQuantFrame;
    Gtk::HBox             modeQuantBox;
    Gtk::VBox             modeQuantVBox;
    Gtk::RadioButton      modeQuantRadioButton;
    Gtk::Label            modeQuantNrColorLabel;
    Gtk::SpinButton       modeQuantNrColorSpinner;
    //params
    Gtk::CheckButton      modeInvertButton;
    Gtk::HBox             modeInvertBox;

    //# Multiple path scanning mode
    Gtk::Frame            modeMultiScanFrame;
    Gtk::VBox             modeMultiScanVBox;
    //brightness
    Gtk::HBox             modeMultiScanHBox1;
    Gtk::RadioButton      modeMultiScanBrightnessRadioButton;
    Gtk::SpinButton       modeMultiScanNrColorSpinner;
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

    Gtk::Frame            optionsFrame;
    Gtk::VBox             optionsVBox;
    Gtk::HBox             optionsSpecklesBox;
    Gtk::CheckButton      optionsSpecklesButton;
    Gtk::Label            optionsSpecklesSizeLabel;
    Gtk::SpinButton       optionsSpecklesSizeSpinner;
    Gtk::HBox             optionsCornersBox;
    Gtk::CheckButton      optionsCornersButton;
    Gtk::Label            optionsCornersThresholdLabel;
    Gtk::SpinButton       optionsCornersThresholdSpinner;
    Gtk::HBox             optionsOptimBox;
    Gtk::CheckButton      optionsOptimButton;
    Gtk::Label            optionsOptimToleranceLabel;
    Gtk::SpinButton       optionsOptimToleranceSpinner;


    //#### Credits

    Gtk::Frame            potraceCreditsFrame;
    Gtk::VBox             potraceCreditsVBox;
    Gtk::Label            potraceCreditsLabel;

    //######## Right pannel

    Gtk::VBox             rightVBox;

    //#### SIOX selection

    Gtk::HBox             sioxBox;
    Gtk::CheckButton      sioxButton;

    //#### Preview

    Gtk::Frame            previewFrame;
    Gtk::VBox             previewVBox;
    Gtk::Button           previewButton;
    Gtk::Image            previewImage;

};



//#########################################################################
//## E V E N T S
//#########################################################################

/**
 * This does potrace processing
 * Only preview if do_i_trace is false
 */
void TraceDialogImpl::potraceProcess(bool do_i_trace)
{
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
             double scaleFX = 200.0 / (double)width;
             double scaleFY = 200.0 / (double)height;
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

}


/**
 * Abort processing
 */
void TraceDialogImpl::abort()
{
    //### Do some GUI thing

    //### Make the abort() call to the tracer
    tracer.abort();

}



//#########################################################################
//## E V E N T S
//#########################################################################

/**
 * Callback from the Preview button.  Can be called from elsewhere.
 */
void TraceDialogImpl::previewCallback()
{
    potraceProcess(false);
}

/**
 * Default response from the dialog.  Let's intercept it
 */
void TraceDialogImpl::responseCallback(int response_id)
{
    if (response_id == GTK_RESPONSE_OK)
        {
            // for now, we assume potrace, as it's the only one we have
            potraceProcess(true);
        }
    else if (response_id == GTK_RESPONSE_CANCEL)
        {
        abort();
        }
    else
        {
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

    modeBrightnessRadioButton.set_label(_("Brightness cutoff"));
    modeGroup = modeBrightnessRadioButton.get_group();
    modeBrightnessBox.pack_start(modeBrightnessRadioButton, false, false, MARGIN);
    tips.set_tip(modeBrightnessRadioButton,
                 _("Trace by a given brightness level"));

    modeBrightnessSpinner.set_digits(3);
    modeBrightnessSpinner.set_increments(0.01, 0.1);
    modeBrightnessSpinner.set_range(0.0, 1.0);
    modeBrightnessSpinner.set_value(0.45);
    modeBrightnessBox.pack_end(modeBrightnessSpinner, false, false, MARGIN);
    tips.set_tip(modeBrightnessSpinner,
                 _("Brightness cutoff for black/white"));

    modeBrightnessSpinnerLabel.set_label(_("Threshold:"));
    modeBrightnessBox.pack_end(modeBrightnessSpinnerLabel, false, false, MARGIN);

    modeBrightnessVBox.pack_start(modeBrightnessBox, false, false, MARGIN);

    modeBrightnessFrame.set_label(_("Single scan: creates a path"));

    // canny edge detection
    // TRANSLATORS: "Canny" is the name of the inventor of this edge detection method

    modeCannyRadioButton.set_label(_("Edge detection"));
    modeCannyRadioButton.set_group(modeGroup);
    modeCannyBox.pack_start(modeCannyRadioButton, false, false, MARGIN);
    tips.set_tip(modeCannyRadioButton,
                 _("Trace with optimal edge detection by J. Canny's algorithm"));
    /*
    modeCannyBox.pack_start(modeCannySeparator);
    modeCannyLoSpinnerLabel.set_label(_("Low"));
    modeCannyBox.pack_start(modeCannyLoSpinnerLabel);
    modeCannyLoSpinner.set_digits(5);
    modeCannyLoSpinner.set_increments(0.01, 0.1);
    modeCannyLoSpinner.set_range(0.0, 1.0);
    modeCannyLoSpinner.set_value(0.1);
    modeCannyBox.pack_start(modeCannyLoSpinner);
    */
    modeCannyHiSpinner.set_digits(3);
    modeCannyHiSpinner.set_increments(0.01, 0.1);
    modeCannyHiSpinner.set_range(0.0, 1.0);
    modeCannyHiSpinner.set_value(0.65);
    modeCannyBox.pack_end(modeCannyHiSpinner, false, false, MARGIN);
    tips.set_tip(modeCannyHiSpinner,
                 _("Brightness cutoff for adjacent pixels (determines edge thickness)"));

    modeCannyHiSpinnerLabel.set_label(_("Threshold:"));
    modeCannyBox.pack_end(modeCannyHiSpinnerLabel, false, false, MARGIN);

    modeBrightnessVBox.pack_start(modeCannyBox, false, false, MARGIN);

    // quantization
    // TRANSLATORS: Color Quantization: the process of reducing the number
    // of colors in an image by selecting an optimized set of representative
    // colors and then re-applying this reduced set to the original image.

    modeQuantRadioButton.set_label(_("Color quantization"));
    modeQuantRadioButton.set_group(modeGroup);
    modeQuantBox.pack_start(modeQuantRadioButton, false, false, MARGIN);
    tips.set_tip(modeQuantRadioButton,
                 _("Trace along the boundaries of reduced colors"));

    modeQuantNrColorSpinner.set_digits(0);
    modeQuantNrColorSpinner.set_increments(1.0, 4.0);
    modeQuantNrColorSpinner.set_range(2.0, 64.0);
    modeQuantNrColorSpinner.set_value(8.0);
    modeQuantBox.pack_end(modeQuantNrColorSpinner, false, false, MARGIN);
    tips.set_tip(modeQuantNrColorSpinner,
                 _("The number of reduced colors"));

    modeQuantNrColorLabel.set_label(_("Colors:"));
    modeQuantBox.pack_end(modeQuantNrColorLabel, false, false, MARGIN);

    modeBrightnessVBox.pack_start(modeQuantBox, false, false, MARGIN);

    // swap black and white
    modeInvertButton.set_label(_("Invert image"));
    modeInvertButton.set_active(false);
    modeInvertBox.pack_start(modeInvertButton, false, false, MARGIN);
    modeBrightnessVBox.pack_start(modeInvertBox, false, false, MARGIN);
    tips.set_tip(modeInvertButton,
                 _("Invert black and white regions"));

    modeBrightnessFrame.add(modeBrightnessVBox);
    modePageBox.pack_start(modeBrightnessFrame, false, false, 0);

    //# end single scan

    //# begin multiple scan

    modeMultiScanBrightnessRadioButton.set_label(_("Brightness steps"));
    modeMultiScanBrightnessRadioButton.set_group(modeGroup);
    modeMultiScanHBox1.pack_start(modeMultiScanBrightnessRadioButton, false, false, MARGIN);
    tips.set_tip(modeMultiScanBrightnessRadioButton,
                 _("Trace the given number of brightness levels"));

    modeMultiScanNrColorSpinner.set_digits(0);
    modeMultiScanNrColorSpinner.set_increments(1.0, 4.0);
    modeMultiScanNrColorSpinner.set_range(2.0, 256.0);
    modeMultiScanNrColorSpinner.set_value(8.0);
    modeMultiScanHBox1.pack_end(modeMultiScanNrColorSpinner, false, false, MARGIN);
    modeMultiScanNrColorLabel.set_label(_("Scans:"));
    modeMultiScanHBox1.pack_end(modeMultiScanNrColorLabel, false, false, MARGIN);
    tips.set_tip(modeMultiScanNrColorSpinner,
                 _("The desired number of scans"));

    modeMultiScanVBox.pack_start(modeMultiScanHBox1, false, false, MARGIN);

    modeMultiScanColorRadioButton.set_label(_("Colors"));
    modeMultiScanColorRadioButton.set_group(modeGroup);
    modeMultiScanHBox2.pack_start(modeMultiScanColorRadioButton, false, false, MARGIN);
    tips.set_tip(modeMultiScanColorRadioButton,
                 _("Trace the given number of reduced colors"));

    modeMultiScanVBox.pack_start(modeMultiScanHBox2, false, false, MARGIN);

    modeMultiScanMonoRadioButton.set_label(_("Grays"));
    modeMultiScanMonoRadioButton.set_group(modeGroup);
    modeMultiScanHBox3.pack_start(modeMultiScanMonoRadioButton, false, false, MARGIN);
    tips.set_tip(modeMultiScanMonoRadioButton,
                 _("Same as Colors, but the result is converted to grayscale"));

    modeMultiScanVBox.pack_start(modeMultiScanHBox3, false, false, MARGIN);

    // TRANSLATORS: "Smooth" is a verb here
    modeMultiScanSmoothButton.set_label(_("Smooth"));
    modeMultiScanSmoothButton.set_active(true);
    modeMultiScanHBox4.pack_start(modeMultiScanSmoothButton, false, false, MARGIN);
    tips.set_tip(modeMultiScanSmoothButton,
                 _("Apply Gaussian blur to the bitmap before tracing"));

    // TRANSLATORS: "Stack" is a verb here
    modeMultiScanStackButton.set_label(_("Stack scans"));
    modeMultiScanStackButton.set_active(true);
    modeMultiScanHBox4.pack_start(modeMultiScanStackButton, false, false, MARGIN);
    tips.set_tip(modeMultiScanStackButton, _("Stack scans on top of one another (no gaps) instead of tiling (usually with gaps)"));


    modeMultiScanBackgroundButton.set_label(_("Remove background"));
    modeMultiScanBackgroundButton.set_active(false);
    modeMultiScanHBox4.pack_start(modeMultiScanBackgroundButton, false, false, MARGIN);
    // TRANSLATORS: "Layer" refers to one of the stacked paths in the multiscan
    tips.set_tip(modeMultiScanBackgroundButton,
                 _("Remove bottom (background) layer when done"));

    modeMultiScanVBox.pack_start(modeMultiScanHBox4, false, false, MARGIN);

    modeMultiScanFrame.set_label(_("Multiple scans: creates a group of paths"));
    //modeQuantFrame.set_shadow_type(Gtk::SHADOW_NONE);
    modeMultiScanFrame.add(modeMultiScanVBox);
    modePageBox.pack_start(modeMultiScanFrame, false, false, 0);

    //# end multiple scan

    //## end mode page

    notebook.append_page(modePageBox, _("Mode"));

    //## begin option page

    //# potrace parameters

    optionsSpecklesButton.set_label(_("Suppress speckles"));
    tips.set_tip(optionsSpecklesButton,
                 _("Ignore small spots (speckles) in the bitmap"));
    optionsSpecklesButton.set_active(true);
    optionsSpecklesBox.pack_start(optionsSpecklesButton, false, false, MARGIN);
    optionsSpecklesSizeSpinner.set_digits(0);
    optionsSpecklesSizeSpinner.set_increments(1, 10);
    optionsSpecklesSizeSpinner.set_range(0, 1000);
    optionsSpecklesSizeSpinner.set_value(2);
    tips.set_tip(optionsSpecklesSizeSpinner,
                 _("Speckles of up to this many pixels will be suppressed"));
    optionsSpecklesBox.pack_end(optionsSpecklesSizeSpinner, false, false, MARGIN);
    optionsSpecklesSizeLabel.set_label(_("Size:"));
    optionsSpecklesBox.pack_end(optionsSpecklesSizeLabel, false, false, MARGIN);

    optionsCornersButton.set_label(_("Smooth corners"));
    tips.set_tip(optionsCornersButton,
                 _("Smooth out sharp corners of the trace"));
    optionsCornersButton.set_active(true);
    optionsCornersBox.pack_start(optionsCornersButton, false, false, MARGIN);
    optionsCornersThresholdSpinner.set_digits(2);
    optionsCornersThresholdSpinner.set_increments(0.01, 0.1);
    optionsCornersThresholdSpinner.set_range(0.0, 1.34);
    optionsCornersThresholdSpinner.set_value(1.0);
    optionsCornersBox.pack_end(optionsCornersThresholdSpinner, false, false, MARGIN);
    tips.set_tip(optionsCornersThresholdSpinner,
                 _("Increase this to smooth corners more"));
    optionsCornersThresholdLabel.set_label(_("Threshold:"));
    optionsCornersBox.pack_end(optionsCornersThresholdLabel, false, false, MARGIN);

    optionsOptimButton.set_label(_("Optimize paths"));
    optionsOptimButton.set_active(true);
    tips.set_tip(optionsOptimButton,
                 _("Try to optimize paths by joining adjacent Bezier curve segments"));
    optionsOptimBox.pack_start(optionsOptimButton, false, false, MARGIN);
    optionsOptimToleranceSpinner.set_digits(2);
    optionsOptimToleranceSpinner.set_increments(0.05, 0.25);
    optionsOptimToleranceSpinner.set_range(0.0, 5.0);
    optionsOptimToleranceSpinner.set_value(0.2);
    optionsOptimBox.pack_end(optionsOptimToleranceSpinner, false, false, MARGIN);
    tips.set_tip(optionsOptimToleranceSpinner,
                 _("Increase this to reduce the number of nodes in the trace by more aggressive optimization"));
    optionsOptimToleranceLabel.set_label(_("Tolerance:"));
    optionsOptimBox.pack_end(optionsOptimToleranceLabel, false, false, MARGIN);

    optionsVBox.pack_start(optionsSpecklesBox, false, false, MARGIN);
    optionsVBox.pack_start(optionsCornersBox, false, false, MARGIN);
    optionsVBox.pack_start(optionsOptimBox, false, false, MARGIN);
    optionsFrame.set_label(_("Options"));
    optionsFrame.add(optionsVBox);
    optionsPageBox.pack_start(optionsFrame, false, false, 0);

    //## end option page

    notebook.append_page(optionsPageBox, _("Options"));

    //### end notebook

    leftVBox.pack_start(notebook, true, true, MARGIN);

    //### credits

    potraceCreditsLabel.set_text(_("Thanks to Peter Selinger, http://potrace.sourceforge.net"));
    potraceCreditsVBox.pack_start(potraceCreditsLabel, false, false, MARGIN);
    potraceCreditsFrame.set_label(_("Credits"));
    potraceCreditsFrame.set_shadow_type(Gtk::SHADOW_NONE);
    potraceCreditsFrame.add(potraceCreditsVBox);

    leftVBox.pack_start(potraceCreditsFrame, false, false, 0);

    //#### end left panel

    mainHBox.pack_start(leftVBox);

    //#### begin right panel

    //## SIOX

    sioxButton.set_label(_("SIOX foreground selection"));
    sioxBox.pack_start(sioxButton, false, false, MARGIN);
    tips.set_tip(sioxButton,
                 _("Cover the area you want to select as the foreground"));
    rightVBox.pack_start(sioxBox, false, false, 0);

    //## preview

    previewButton.set_label(_("Update"));
    previewButton.signal_clicked().connect(
         sigc::mem_fun(*this, &TraceDialogImpl::previewCallback) );
    previewVBox.pack_end(previewButton, false, false, 0);
    // I guess it's correct to call the "intermediate bitmap" a preview of the trace
    tips.set_tip(previewButton,
                 _("Preview the intermediate bitmap with the current settings, without actual tracing"));
    previewImage.set_size_request(200,200);
    //previewImage.set_alignment (Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER);
    previewVBox.pack_start(previewImage, true, true, 0);
    previewFrame.set_label(_("Preview"));
    //previewFrame.set_shadow_type(Gtk::SHADOW_NONE);
    previewFrame.add(previewVBox);

    rightVBox.pack_start(previewFrame, true, true, MARGIN);

    //#### end right panel

    mainHBox.pack_start(rightVBox);

    //#### Global stuff

    contents->pack_start(mainHBox);

    //## The OK button
    mainCancelButton = addResponseButton(Gtk::Stock::STOP, GTK_RESPONSE_CANCEL);
    if (mainCancelButton) {
	tips.set_tip((*mainCancelButton), _("Abort a trace in progress"));
	mainCancelButton->set_sensitive(false);
    }
    mainOkButton = addResponseButton(Gtk::Stock::OK, GTK_RESPONSE_OK);
    tips.set_tip((*mainOkButton), _("Execute the trace"));

    show_all_children();

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

}


} //namespace Dialog
} //namespace UI
} //namespace Inkscape

//#########################################################################
//## E N D    O F    F I L E
//#########################################################################



