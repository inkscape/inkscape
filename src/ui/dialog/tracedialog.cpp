/*
 * A simple dialog for setting the parameters for autotracing a
 * bitmap <image> into an svg <path>
 *
 * Authors:
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

    void potracePreviewCallback();

    Gtk::Notebook   notebook;
    Gtk::Tooltips   tips;

    //########## General items
    Gtk::Frame            sioxFrame;
    Gtk::VBox             sioxVBox;
    Gtk::HBox             sioxBox;
    Gtk::CheckButton      sioxButton;

    //########## Potrace items
    Gtk::VBox             potraceBox;
    Gtk::RadioButtonGroup potraceGroup;
    Gtk::CheckButton      potraceInvertButton;
    Gtk::HBox             potraceInvertBox;
    Gtk::Button           *potraceOkButton;
    Gtk::Button           *potraceCancelButton;

    //params
    Gtk::Frame            potraceParamsFrame;
    Gtk::VBox             potraceParamsVBox;
    Gtk::HBox             potraceParamsSpecklesBox;
    Gtk::CheckButton      potraceParamsSpecklesButton;
    Gtk::Label            potraceParamsSpecklesSizeLabel;
    Gtk::SpinButton       potraceParamsSpecklesSizeSpinner;
    Gtk::HBox             potraceParamsCornersBox;
    Gtk::CheckButton      potraceParamsCornersButton;
    Gtk::Label            potraceParamsCornersThresholdLabel;
    Gtk::SpinButton       potraceParamsCornersThresholdSpinner;
    Gtk::HBox             potraceParamsOptimBox;
    Gtk::CheckButton      potraceParamsOptimButton;
    Gtk::Label            potraceParamsOptimToleranceLabel;
    Gtk::SpinButton       potraceParamsOptimToleranceSpinner;

    //brightness
    Gtk::Frame            potraceBrightnessFrame;
    Gtk::VBox             potraceBrightnessVBox;
    Gtk::HBox             potraceBrightnessBox;
    Gtk::RadioButton      potraceBrightnessRadioButton;
    Gtk::Label            potraceBrightnessSpinnerLabel;
    Gtk::SpinButton       potraceBrightnessSpinner;


    //edge detection
    Gtk::Frame            potraceCannyFrame;
    Gtk::HBox             potraceCannyBox;
    Gtk::VBox             potraceCannyVBox;
    Gtk::RadioButton      potraceCannyRadioButton;
    //Gtk::HSeparator     potraceCannySeparator;
    //Gtk::Label          potraceCannyLoSpinnerLabel;
    //Gtk::SpinButton     potraceCannyLoSpinner;
    Gtk::Label            potraceCannyHiSpinnerLabel;
    Gtk::SpinButton       potraceCannyHiSpinner;

    //quantization
    Gtk::Frame            potraceQuantFrame;
    Gtk::HBox             potraceQuantBox;
    Gtk::VBox             potraceQuantVBox;
    Gtk::RadioButton      potraceQuantRadioButton;
    Gtk::Label            potraceQuantNrColorLabel;
    Gtk::SpinButton       potraceQuantNrColorSpinner;

    //multiple path scanning
    Gtk::Frame            potraceMultiScanFrame;
    Gtk::VBox             potraceMultiScanVBox;

    Gtk::HBox             potraceMultiScanHBox1;
    Gtk::RadioButton      potraceMultiScanBrightnessRadioButton;
    Gtk::SpinButton       potraceMultiScanNrColorSpinner;

    Gtk::HBox             potraceMultiScanHBox2;
    Gtk::RadioButton      potraceMultiScanColorRadioButton;

    Gtk::HBox             potraceMultiScanHBox3;
    Gtk::RadioButton      potraceMultiScanMonoRadioButton;
    Gtk::Label            potraceMultiScanNrColorLabel;

    Gtk::CheckButton      potraceMultiScanStackButton;
    Gtk::CheckButton      potraceMultiScanSmoothButton;
    Gtk::CheckButton      potraceMultiScanBackgroundButton;


    //preview
    Gtk::Frame            potracePreviewFrame;
    Gtk::HBox             potracePreviewBox;
    Gtk::Button           potracePreviewButton;
    Gtk::Image            potracePreviewImage;

    //credits
    Gtk::Frame            potraceCreditsFrame;
    Gtk::VBox             potraceCreditsVBox;
    Gtk::Label            potraceCreditsLabel;

    //########## Other items
    Gtk::VBox             otherBox;



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
    bool invert = potraceInvertButton.get_active();
    pte.setInvert(invert);

    //##### Get the preprocessor settings
    /* siox -- performed by Tracer, and before any of the others */
    if (sioxButton.get_active())
        tracer.enableSiox(true);
    else
        tracer.enableSiox(false);

    /* one of the following */
    if (potraceBrightnessRadioButton.get_active())
        pte.setTraceType(Inkscape::Trace::Potrace::TRACE_BRIGHTNESS);
    else if (potraceMultiScanBrightnessRadioButton.get_active())
        pte.setTraceType(Inkscape::Trace::Potrace::TRACE_BRIGHTNESS_MULTI);
    else if (potraceCannyRadioButton.get_active())
        pte.setTraceType(Inkscape::Trace::Potrace::TRACE_CANNY);
    else if (potraceQuantRadioButton.get_active())
        pte.setTraceType(Inkscape::Trace::Potrace::TRACE_QUANT);
    else if (potraceMultiScanColorRadioButton.get_active())
        {
        pte.setTraceType(Inkscape::Trace::Potrace::TRACE_QUANT_COLOR);
        pte.setInvert(false);
        }
    else if (potraceMultiScanMonoRadioButton.get_active())
        {
        pte.setTraceType(Inkscape::Trace::Potrace::TRACE_QUANT_MONO);
        pte.setInvert(false);
        }

    /* params */
    int paramsSpecklesSize =
      potraceParamsSpecklesButton.get_active() ?
      potraceParamsSpecklesSizeSpinner.get_value_as_int() :
      0;
    pte.setParamsTurdSize(paramsSpecklesSize);
    double paramsCornersThreshold =
      potraceParamsCornersButton.get_active() ?
      potraceParamsCornersThresholdSpinner.get_value() :
      0.;
    pte.setParamsAlphaMax(paramsCornersThreshold);
    bool paramsOptim = potraceParamsOptimButton.get_active();
    pte.setParamsOptiCurve(paramsOptim);
    double paramsOptimTolerance = potraceParamsOptimToleranceSpinner.get_value();
    pte.setParamsOptTolerance(paramsOptimTolerance);

    //##### Get the single-scan settings

    /* brightness */
    double brightnessThreshold = potraceBrightnessSpinner.get_value();
    pte.setBrightnessThreshold(brightnessThreshold);

    /* canny */
    double cannyHighThreshold = potraceCannyHiSpinner.get_value();
    pte.setCannyHighThreshold(cannyHighThreshold);

    /* quantization */
    int quantNrColors = potraceQuantNrColorSpinner.get_value_as_int();
    pte.setQuantizationNrColors(quantNrColors);

    //##### Get multiple-scan settings
    int multiScanNrColors = potraceMultiScanNrColorSpinner.get_value_as_int();
    pte.setMultiScanNrColors(multiScanNrColors);
    bool do_i_stack = potraceMultiScanStackButton.get_active();
    pte.setMultiScanStack(do_i_stack);
    bool do_i_smooth = potraceMultiScanSmoothButton.get_active();
    pte.setMultiScanSmooth(do_i_smooth);
    bool do_i_remove_background = potraceMultiScanBackgroundButton.get_active();
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
             double scaleFactor = 100.0 / (double)height;
             int newWidth  = (int) (((double)width)  * scaleFactor);
             int newHeight = (int) (((double)height) * scaleFactor);
             Glib::RefPtr<Gdk::Pixbuf> scaledPreview =
                    preview->scale_simple(newWidth, newHeight,
                       Gdk::INTERP_NEAREST);
             //g_object_unref(preview);
             potracePreviewImage.set(scaledPreview);
             }
         }

    //##### Convert
    if (do_i_trace)
        {
        if (potraceCancelButton)
            potraceCancelButton->set_sensitive(true);
        if (potraceOkButton)
            potraceOkButton->set_sensitive(false);
        tracer.trace(&pte);
        if (potraceCancelButton)
            potraceCancelButton->set_sensitive(false);
        if (potraceOkButton)
            potraceOkButton->set_sensitive(true);
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
void TraceDialogImpl::potracePreviewCallback()
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
        int panelNr = notebook.get_current_page();
        //g_message("selected panel:%d\n", panelNr);

        if (panelNr == 0)
            {
            potraceProcess(true);
            }
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
TraceDialogImpl::TraceDialogImpl()
{

    Gtk::VBox *mainVBox = get_vbox();

#define MARGIN 4

    /*#### SIOX ####*/
    //# for now, put at the top of the potrace box.  something better later
    sioxButton.set_label(_("SIOX foreground selection"));
    sioxBox.pack_start(sioxButton, false, false, MARGIN);
    tips.set_tip(sioxButton, 
        _("Cover the area you want to select as the foreground"));
    sioxVBox.pack_start(sioxBox, false, false, MARGIN);
    sioxFrame.set_label(_("SIOX"));
    sioxFrame.add(sioxVBox);
    potraceBox.pack_start(sioxFrame, false, false, 0);


    //##Set up the Potrace panel

    /*#### params ####*/
    potraceParamsSpecklesButton.set_label(_("Speckles Filtering"));
    potraceParamsSpecklesButton.set_active(true);
    //tips.set_tip(potraceParamsSpecklesButton, _(""));
    potraceParamsSpecklesBox.pack_start(potraceParamsSpecklesButton, false, false, MARGIN);
    potraceParamsSpecklesSizeSpinner.set_digits(0);
    potraceParamsSpecklesSizeSpinner.set_increments(1, 10);
    potraceParamsSpecklesSizeSpinner.set_range(0, 1000);
    potraceParamsSpecklesSizeSpinner.set_value(2);
    tips.set_tip(potraceParamsSpecklesSizeSpinner,
		 _("Suppress speckles of up to this many pixels"));
    potraceParamsSpecklesBox.pack_end(potraceParamsSpecklesSizeSpinner, false, false, MARGIN);
    potraceParamsSpecklesSizeLabel.set_label(_("Size:"));
    potraceParamsSpecklesBox.pack_end(potraceParamsSpecklesSizeLabel, false, false, MARGIN);

    potraceParamsCornersButton.set_label(_("Corner Smoothing"));
    potraceParamsCornersButton.set_active(true);
    //tips.set_tip(potraceParamsCornersButton, _(""));
    potraceParamsCornersBox.pack_start(potraceParamsCornersButton, false, false, MARGIN);
    potraceParamsCornersThresholdSpinner.set_digits(2);
    potraceParamsCornersThresholdSpinner.set_increments(0.01, 0.1);
    potraceParamsCornersThresholdSpinner.set_range(0.0, 1.34);
    potraceParamsCornersThresholdSpinner.set_value(1.0);
    potraceParamsCornersBox.pack_end(potraceParamsCornersThresholdSpinner, false, false, MARGIN);
    tips.set_tip(potraceParamsCornersThresholdSpinner,
		 _("The higher this value, the smoother corners will be produced"));
    potraceParamsCornersThresholdLabel.set_label(_("Threshold:"));
    potraceParamsCornersBox.pack_end(potraceParamsCornersThresholdLabel, false, false, MARGIN);

    potraceParamsOptimButton.set_label(_("Curve Optimization"));
    potraceParamsOptimButton.set_active(true);
    tips.set_tip(potraceParamsOptimButton,
		 _("Try to join adjacent Bezier curve segments"));
    potraceParamsOptimBox.pack_start(potraceParamsOptimButton, false, false, MARGIN);
    potraceParamsOptimToleranceSpinner.set_digits(2);
    potraceParamsOptimToleranceSpinner.set_increments(0.05, 0.25);
    potraceParamsOptimToleranceSpinner.set_range(0.0, 5.0);
    potraceParamsOptimToleranceSpinner.set_value(0.2);
    potraceParamsOptimBox.pack_end(potraceParamsOptimToleranceSpinner, false, false, MARGIN);
    tips.set_tip(potraceParamsOptimToleranceSpinner,
		 _("Set the curve optimization tolerance"));
    potraceParamsOptimToleranceLabel.set_label(_("Tolerance:"));
    potraceParamsOptimBox.pack_end(potraceParamsOptimToleranceLabel, false, false, MARGIN);

    potraceParamsVBox.pack_start(potraceParamsSpecklesBox, false, false, MARGIN);
    potraceParamsVBox.pack_start(potraceParamsCornersBox, false, false, MARGIN);
    potraceParamsVBox.pack_start(potraceParamsOptimBox, false, false, MARGIN);
    potraceParamsFrame.set_label(_("Parameters"));
    potraceParamsFrame.add(potraceParamsVBox);
    potraceBox.pack_start(potraceParamsFrame, false, false, 0);

    /*#### brightness ####*/
    potraceBrightnessRadioButton.set_label(_("Brightness"));
    potraceGroup = potraceBrightnessRadioButton.get_group();
    potraceBrightnessBox.pack_start(potraceBrightnessRadioButton, false, false, MARGIN);
    tips.set_tip(potraceBrightnessRadioButton, _("Trace by a given brightness level"));

    potraceBrightnessSpinner.set_digits(3);
    potraceBrightnessSpinner.set_increments(0.01, 0.1);
    potraceBrightnessSpinner.set_range(0.0, 1.0);
    potraceBrightnessSpinner.set_value(0.45);
    potraceBrightnessBox.pack_end(potraceBrightnessSpinner, false, false, MARGIN);
    tips.set_tip(potraceBrightnessSpinner, _("Brightness cutoff for black/white"));

    potraceBrightnessSpinnerLabel.set_label(_("Threshold:"));
    potraceBrightnessBox.pack_end(potraceBrightnessSpinnerLabel, false, false, MARGIN);

    potraceBrightnessVBox.pack_start(potraceBrightnessBox, false, false, MARGIN);

    potraceBrightnessFrame.set_label(_("Image Brightness"));
    //potraceBrightnessFrame.set_shadow_type(Gtk::SHADOW_NONE);
    potraceBrightnessFrame.add(potraceBrightnessVBox);
    potraceBox.pack_start(potraceBrightnessFrame, false, false, 0);

    /*#### canny edge detection ####*/
    // TRANSLATORS: "Canny" is the name of the inventor of this edge detection method
    potraceCannyRadioButton.set_label(_("Optimal Edge Detection (Canny)"));
    potraceCannyRadioButton.set_group(potraceGroup);
    potraceCannyBox.pack_start(potraceCannyRadioButton, false, false, MARGIN);
    tips.set_tip(potraceCannyRadioButton, _("Trace with edge detection by J. Canny's algorithm"));
    /*
    potraceCannyBox.pack_start(potraceCannySeparator);
    potraceCannyLoSpinnerLabel.set_label(_("Low"));
    potraceCannyBox.pack_start(potraceCannyLoSpinnerLabel);
    potraceCannyLoSpinner.set_digits(5);
    potraceCannyLoSpinner.set_increments(0.01, 0.1);
    potraceCannyLoSpinner.set_range(0.0, 1.0);
    potraceCannyLoSpinner.set_value(0.1);
    potraceCannyBox.pack_start(potraceCannyLoSpinner);
    */
    potraceCannyHiSpinner.set_digits(3);
    potraceCannyHiSpinner.set_increments(0.01, 0.1);
    potraceCannyHiSpinner.set_range(0.0, 1.0);
    potraceCannyHiSpinner.set_value(0.65);
    potraceCannyBox.pack_end(potraceCannyHiSpinner, false, false, MARGIN);
    tips.set_tip(potraceCannyHiSpinner, _("Brightness cutoff for adjacent pixels (determines edge thickness)"));

    potraceCannyHiSpinnerLabel.set_label(_("Threshold:"));
    potraceCannyBox.pack_end(potraceCannyHiSpinnerLabel, false, false, MARGIN);

    potraceCannyVBox.pack_start(potraceCannyBox, false, false, MARGIN);

    potraceCannyFrame.set_label(_("Edge Detection"));
    //potraceCannyFrame.set_shadow_type(Gtk::SHADOW_NONE);
    potraceCannyFrame.add(potraceCannyVBox);
    potraceBox.pack_start(potraceCannyFrame, false, false, 0);

    /*#### quantization ####*/
    // TRANSLATORS: Color Quantization: the process of reducing the number of colors
    //  in an image by selecting an optimized set of representative colors and then
    //  re-applying this reduced set to the original image.
    potraceQuantRadioButton.set_label(_("Color Quantization"));
    potraceQuantRadioButton.set_group(potraceGroup);
    potraceQuantBox.pack_start(potraceQuantRadioButton, false, false, MARGIN);
    tips.set_tip(potraceQuantRadioButton, _("Trace along the boundaries of reduced colors"));

    potraceQuantNrColorSpinner.set_digits(2);
    potraceQuantNrColorSpinner.set_increments(1.0, 4.0);
    potraceQuantNrColorSpinner.set_range(2.0, 64.0);
    potraceQuantNrColorSpinner.set_value(8.0);
    potraceQuantBox.pack_end(potraceQuantNrColorSpinner, false, false, MARGIN);
    tips.set_tip(potraceQuantNrColorSpinner, _("The number of reduced colors"));

    potraceQuantNrColorLabel.set_label(_("Colors:"));
    potraceQuantBox.pack_end(potraceQuantNrColorLabel, false, false, MARGIN);

    potraceQuantVBox.pack_start(potraceQuantBox, false, false, MARGIN);

    potraceQuantFrame.set_label(_("Quantization / Reduction"));
    //potraceQuantFrame.set_shadow_type(Gtk::SHADOW_NONE);
    potraceQuantFrame.add(potraceQuantVBox);
    potraceBox.pack_start(potraceQuantFrame, false, false, 0);

    /*#### Multiple scanning####*/
    //----Hbox1
    potraceMultiScanBrightnessRadioButton.set_label(_("Brightness"));
    potraceMultiScanBrightnessRadioButton.set_group(potraceGroup);
    potraceMultiScanHBox1.pack_start(potraceMultiScanBrightnessRadioButton, false, false, MARGIN);
    tips.set_tip(potraceMultiScanBrightnessRadioButton, _("Trace the given number of brightness levels"));

    potraceMultiScanNrColorSpinner.set_digits(0);
    potraceMultiScanNrColorSpinner.set_increments(1.0, 4.0);
    potraceMultiScanNrColorSpinner.set_range(2.0, 256.0);
    potraceMultiScanNrColorSpinner.set_value(8.0);
    potraceMultiScanHBox1.pack_end(potraceMultiScanNrColorSpinner, false, false, MARGIN);
    potraceMultiScanNrColorLabel.set_label(_("Scans:"));
    potraceMultiScanHBox1.pack_end(potraceMultiScanNrColorLabel, false, false, MARGIN);
    tips.set_tip(potraceMultiScanNrColorSpinner, _("The desired number of scans"));

    potraceMultiScanVBox.pack_start(potraceMultiScanHBox1, false, false, MARGIN);

    //----Hbox2
    potraceMultiScanColorRadioButton.set_label(_("Color"));
    potraceMultiScanColorRadioButton.set_group(potraceGroup);
    potraceMultiScanHBox2.pack_start(potraceMultiScanColorRadioButton, false, false, MARGIN);
    tips.set_tip(potraceMultiScanColorRadioButton, _("Trace the given number of reduced colors"));


    // TRANSLATORS: "Layer" refers to one of the stacked paths in the multiscan
    potraceMultiScanBackgroundButton.set_label(_("Remove background"));
    potraceMultiScanBackgroundButton.set_active(false);
    potraceMultiScanHBox2.pack_end(potraceMultiScanBackgroundButton, false, false, MARGIN);
    tips.set_tip(potraceMultiScanBackgroundButton, _("Remove bottom (background) layer when done"));

    potraceMultiScanVBox.pack_start(potraceMultiScanHBox2, false, false, MARGIN);

    //---Hbox3
    potraceMultiScanMonoRadioButton.set_label(_("Monochrome"));
    potraceMultiScanMonoRadioButton.set_group(potraceGroup);
    potraceMultiScanHBox3.pack_start(potraceMultiScanMonoRadioButton, false, false, MARGIN);
    tips.set_tip(potraceMultiScanMonoRadioButton, _("Same as Color, but convert result to grayscale"));

    // TRANSLATORS: "Stack" is a verb here
    potraceMultiScanStackButton.set_label(_("Stack"));
    potraceMultiScanStackButton.set_active(true);
    potraceMultiScanHBox3.pack_end(potraceMultiScanStackButton, false, false, MARGIN);
    tips.set_tip(potraceMultiScanStackButton, _("Stack scans vertically (no gaps) or tile horizontally (usually with gaps)"));

    // TRANSLATORS: "Smooth" is a verb here
    potraceMultiScanSmoothButton.set_label(_("Smooth"));
    potraceMultiScanSmoothButton.set_active(true);
    potraceMultiScanHBox3.pack_end(potraceMultiScanSmoothButton, false, false, MARGIN);
    tips.set_tip(potraceMultiScanSmoothButton, _("Apply Gaussian blur to the bitmap before tracing"));


    potraceMultiScanVBox.pack_start(potraceMultiScanHBox3, false, false, MARGIN);

    potraceMultiScanFrame.set_label(_("Multiple Scanning"));
    //potraceQuantFrame.set_shadow_type(Gtk::SHADOW_NONE);
    potraceMultiScanFrame.add(potraceMultiScanVBox);
    potraceBox.pack_start(potraceMultiScanFrame, false, false, 0);

    /*#### Preview ####*/
    potracePreviewButton.set_label(_("Preview"));
    potracePreviewButton.signal_clicked().connect(
         sigc::mem_fun(*this, &TraceDialogImpl::potracePreviewCallback) );
    potracePreviewBox.pack_end(potracePreviewButton, false, false, 0);//do not expand
    tips.set_tip(potracePreviewButton, _("Preview the result without actual tracing"));


    potracePreviewImage.set_size_request(100,100);
    //potracePreviewImage.set_alignment (Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER);
    potracePreviewBox.pack_start(potracePreviewImage, true, true, 0);
    potracePreviewFrame.set_label(_("Preview")); // I guess it's correct to call the "intermediate bitmap" a preview of the trace
    //potracePreviewFrame.set_shadow_type(Gtk::SHADOW_NONE);
    potracePreviewFrame.add(potracePreviewBox);
    potraceBox.pack_start(potracePreviewFrame, true, true, 0);

    /*#### swap black and white ####*/
    potraceInvertButton.set_label(_("Invert"));
    potraceInvertButton.set_active(false);
    potraceInvertBox.pack_end(potraceInvertButton, false, false, MARGIN);
    potraceBox.pack_start(potraceInvertBox, false, false, MARGIN);
    tips.set_tip(potraceInvertButton, _("Invert black and white regions for single traces"));

    /*#### Credits ####*/
    potraceCreditsLabel.set_text(
							 _("Thanks to Peter Selinger, http://potrace.sourceforge.net")
                         );
    potraceCreditsVBox.pack_start(potraceCreditsLabel, false, false, MARGIN);
    potraceCreditsFrame.set_label(_("Credits"));
    potraceCreditsFrame.set_shadow_type(Gtk::SHADOW_NONE);
    potraceCreditsFrame.add(potraceCreditsVBox);
    potraceBox.pack_start(potraceCreditsFrame, false, false, 0);

    /*done */
    // TRANSLATORS: Potrace is an application for transforming bitmaps into
    //  vector graphics (http://potrace.sourceforge.net/)
    notebook.append_page(potraceBox, _("Potrace"));

    //##Set up the Other panel
    // This may be reenabled when we have another tracer; now an empty tab is confusing so I'm disabling it
    //    notebook.append_page(otherBox, _("Other"));

    //##Put the notebook on the dialog
    mainVBox->pack_start(notebook);

    //## The OK button
    potraceCancelButton = add_button(Gtk::Stock::STOP, GTK_RESPONSE_CANCEL);
    if (potraceCancelButton)
        {
        tips.set_tip((*potraceCancelButton), _("Abort a trace in progress"));
        potraceCancelButton->set_sensitive(false);
        }
    potraceOkButton     = add_button(Gtk::Stock::OK,   GTK_RESPONSE_OK);
    tips.set_tip((*potraceOkButton), _("Execute the trace"));

    show_all_children();

    //## Connect the signal
    signal_response().connect(
         sigc::mem_fun(*this, &TraceDialogImpl::responseCallback) );
}

/**
 * Factory method.  Use this to create a new TraceDialog
 */
TraceDialog *TraceDialog::create()
{
    TraceDialog *dialog = new TraceDialogImpl();
    return dialog;
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



