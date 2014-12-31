/*
 * A simple image display widget, using Inkscape's own rendering engine
 *
 * Authors:
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *   Abhishek Sharma
 *
 * Copyright (C) 2004 The Inkscape Organization
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */




#include "imageicon.h"
#include <sys/stat.h>
#include "svg-view-widget.h"
#include "document.h"
#include "inkscape.h"
#include <glibmm/convert.h>
#include <glibmm/fileutils.h>


namespace Inkscape
{
namespace UI
{
namespace Widget
{



typedef enum {
    SVG_TYPES,
    IMPORT_TYPES,
    EXPORT_TYPES
    } FileDialogType;


/*#########################################################################
### ImageIcon widget
#########################################################################*/


/**
 * Constructor
 */
ImageIcon::ImageIcon()
{
    init();
}

/**
 * Construct from a file name
 */
ImageIcon::ImageIcon(const Glib::ustring &fileName)
{
    init();
    showSvgFile(fileName);
}

/**
 * Copy Constructor
 */
ImageIcon::ImageIcon(const ImageIcon &other) 
             : sigc::trackable(), Glib::ObjectBase(), Gtk::VBox()
{
    init();
    document           = other.document;
    viewerGtkmm        = other.viewerGtkmm;
    showingBrokenImage = other.showingBrokenImage;
}

/**
 * Destructor
 */
ImageIcon::~ImageIcon()
{
    if (document)
        document->doUnref();
}


/**
 * basic initialization, called by the various constructors
 */
void ImageIcon::init()
{
    //  \FIXME Why?
    if (!Inkscape::Application::exists())
        Inkscape::Application::create("", false);
    document = NULL;
    viewerGtkmm = NULL;
    //set_size_request(150,150);
    showingBrokenImage = false;
}


bool ImageIcon::showSvgDocument(const SPDocument *docArg)
{
    if (document)
        document->doUnref();

    SPDocument *doc = const_cast<SPDocument *>(docArg);

    doc->doRef();
    document = doc;

    //This should remove it from the box, and free resources
    //if (viewerGtkmm)
    //    viewerGtkmm->destroy();

    GtkWidget *viewerGtk  = sp_svg_view_widget_new(doc);
    viewerGtkmm = Glib::wrap(viewerGtk);

    viewerGtkmm->show();
    pack_start(*viewerGtkmm, TRUE, TRUE, 0);

    //GtkWidget *vbox = GTK_WIDGET(gobj());
    //gtk_box_pack_start(GTK_BOX(vbox), viewerGtk, TRUE, TRUE, 0);

    return true;
}

bool ImageIcon::showSvgFile(const Glib::ustring &theFileName)
{
    Glib::ustring fileName = theFileName;

    fileName = Glib::filename_to_utf8(fileName);

    SPDocument *doc = SPDocument::createNewDoc (fileName.c_str(), 0);
    if (!doc) {
        g_warning("SVGView: error loading document '%s'\n", fileName.c_str());
        return false;
    }

    showSvgDocument(doc);

    doc->doUnref();

    return true;
}



bool ImageIcon::showSvgFromMemory(const char *xmlBuffer)
{
    if (!xmlBuffer)
        return false;

    gint len = (gint)strlen(xmlBuffer);
    SPDocument *doc = SPDocument::createNewDocFromMem(xmlBuffer, len, 0);
    if (!doc) {
        g_warning("SVGView: error loading buffer '%s'\n",xmlBuffer);
        return false;
    }

    showSvgDocument(doc);

    doc->doUnref();

    return true;
}



bool ImageIcon::showBitmap(const Glib::ustring &theFileName)
{
    Glib::ustring fileName = theFileName;


    /*#####################################
    # LET'S HAVE SOME FUN WITH SVG!
    # Instead of just loading an image, why
    # don't we make a lovely little svg and
    # display it nicely?
    #####################################*/

    //Arbitrary size of svg doc -- rather 'portrait' shaped
    gint previewWidth  = 400;
    gint previewHeight = 600;

    //Get some image info. Smart pointer does not need to be deleted
    Glib::RefPtr<Gdk::Pixbuf> img = Gdk::Pixbuf::create_from_file(fileName);
    gint imgWidth  = img->get_width();
    gint imgHeight = img->get_height();

    //Find the minimum scale to fit the image inside the preview area
    double scaleFactorX = (0.9 *(double)previewWidth)  / ((double)imgWidth);
    double scaleFactorY = (0.9 *(double)previewHeight) / ((double)imgHeight);
    double scaleFactor = scaleFactorX;
    if (scaleFactorX > scaleFactorY)
        scaleFactor = scaleFactorY;

    //Now get the resized values
    gint scaledImgWidth  = (int) (scaleFactor * (double)imgWidth);
    gint scaledImgHeight = (int) (scaleFactor * (double)imgHeight);

    //center the image on the area
    gint imgX = (previewWidth  - scaledImgWidth)  / 2;
    gint imgY = (previewHeight - scaledImgHeight) / 2;

    //wrap a rectangle around the image
    gint rectX      = imgX-1;
    gint rectY      = imgY-1;
    gint rectWidth  = scaledImgWidth +2;
    gint rectHeight = scaledImgHeight+2;

    //Our template.  Modify to taste
    gchar const *xformat =
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
          "<svg\n"
          "xmlns=\"http://www.w3.org/2000/svg\"\n"
          "xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
          "width=\"%d\" height=\"%d\">\n"
          "<rect\n"
          "  style=\"fill:#eeeeee;stroke:none\"\n"
          "  x=\"-100\" y=\"-100\" width=\"4000\" height=\"4000\"/>\n"
          "<image x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\"\n"
          "xlink:href=\"%s\"/>\n"
          "<rect\n"
          "  style=\"fill:none;"
          "    stroke:#000000;stroke-width:1.0;"
          "    stroke-linejoin:miter;stroke-opacity:1.0000000;"
          "    stroke-miterlimit:4.0000000;stroke-dasharray:none\"\n"
          "  x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\"/>\n"
          "</svg>\n\n";

    //if (!Glib::get_charset()) //If we are not utf8
    fileName = Glib::filename_to_utf8(fileName);

    //Fill in the template
    /* FIXME: Do proper XML quoting for fileName. */
    gchar *xmlBuffer = g_strdup_printf(xformat,
           previewWidth, previewHeight,
           imgX, imgY, scaledImgWidth, scaledImgHeight,
           fileName.c_str(),
           rectX, rectY, rectWidth, rectHeight);

    //g_message("%s\n", xmlBuffer);

    //now show it!
    showSvgFromMemory(xmlBuffer);
    g_free(xmlBuffer);

    return true;
}



void ImageIcon::showBrokenImage(const Glib::ustring &errorMessage)
{
    //Are we already showing it?
    if (showingBrokenImage)
        return;

    //Our template.  Modify to taste
    gchar const *xformat =
        "<?xml version='1.0' encoding='UTF-8' standalone='no'?>"
        "<!-- Created with Inkscape (http://www.inkscape.org/) -->"
        "<svg"
        "   xmlns:xml='http://www.w3.org/XML/1998/namespace'"
        "   xmlns:svg='http://www.w3.org/2000/svg'"
        "   xmlns='http://www.w3.org/2000/svg'"
        "   id='svg2'"
        "   height='64.000000'"
        "   width='64.000000'"
        "   y='0.00000000'"
        "   x='0.00000000'"
        "   version='1.0'>"
        "  <defs"
        "     id='defs3' />"
        "  <rect"
        "     id='rect1723'"
        "     style='fill:#fefefe;fill-opacity:1.0000000;stroke:#fe0101;stroke-opacity:1.0000000'"
        "     y='0.74437392'"
        "     x='0.52374262'"
        "     height='62.698864'"
        "     width='62.701584' />"
        "  <path"
        "     id='path1725'"
        "     style='fill:none;fill-opacity:0.75000000;fill-rule:evenodd;stroke:#fe0101;stroke-width:0.99443889px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1.0000000'"
        "     d='M 0.35436684,0.57368969 L 62.861950,63.081272' />"
        "  <path"
        "     id='path1727'"
        "     style='fill:none;fill-opacity:0.75000000;fill-rule:evenodd;stroke:#fe0101;stroke-width:0.99443889px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1.0000000'"
        "     d='M 0.53194379,63.258849 L 63.217104,0.92884358' />"
        "  <path"
        "     id='text1729'"
        "     style='font-size:12.000000;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;line-height:100.00000%;writing-mode:lr-tb;text-anchor:start;fill:#000000;fill-opacity:1.0000000;stroke:none;stroke-width:1.0000000px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1.0000000;font-family:Arial Black'"
        "     d='M 11.027113,15.116791 L 13.421921,15.116791 L 13.421921,18.076798 C 13.658875,17.828198 13.926907,17.641741 14.226014,17.517426 C 14.529009,17.393131 14.863071,17.330979 15.228229,17.330969 C 15.981815,17.330979 16.605278,17.602888 17.098629,18.146717 C 17.591951,18.686677 17.838621,19.463583 17.838631,20.477433 C 17.838621,21.153343 17.725971,21.749619 17.500671,22.266259 C 17.275361,22.779022 16.962660,23.163581 16.562557,23.419968 C 16.166333,23.672456 15.725439,23.798710 15.239874,23.798710 C 14.824228,23.798710 14.443547,23.709359 14.097831,23.530668 C 13.837556,23.390830 13.553992,23.128627 13.247118,22.744057 L 13.247118,23.658862 L 11.027113,23.658862 L 11.027113,15.116791 M 13.404438,20.547352 C 13.404438,21.079536 13.503494,21.466054 13.701606,21.706887 C 13.903597,21.943842 14.158034,22.062320 14.464918,22.062320 C 14.748482,22.062320 14.985437,21.945791 15.175782,21.712715 C 15.370006,21.475760 15.467113,21.079536 15.467123,20.524052 C 15.467113,20.034599 15.371945,19.675289 15.181610,19.446090 C 14.995153,19.216912 14.767903,19.102313 14.499872,19.102313 C 14.177455,19.102313 13.913313,19.222739 13.707434,19.463573 C 13.505433,19.700537 13.404438,20.061797 13.404438,20.547352 M 18.986502,17.470807 L 21.206507,17.470807 L 21.206507,18.484677 C 21.420152,18.045732 21.639635,17.744675 21.864935,17.581518 C 22.094123,17.414492 22.375749,17.330979 22.709820,17.330969 C 23.059425,17.330979 23.442055,17.439740 23.857701,17.657264 L 23.123527,19.347034 C 22.843831,19.230506 22.622419,19.172242 22.459271,19.172232 C 22.148509,19.172242 21.907666,19.300425 21.736752,19.556801 C 21.492021,19.918071 21.369655,20.593971 21.369665,21.584522 L 21.369665,23.658862 L 18.986502,23.658862 L 18.986502,17.470807 M 24.149042,20.582316 C 24.149042,19.638385 24.467571,18.861480 25.104638,18.251600 C 25.741695,17.637853 26.602114,17.330979 27.685903,17.330969 C 28.925063,17.330979 29.861228,17.690289 30.494417,18.408921 C 31.003281,18.987724 31.257718,19.700537 31.257728,20.547352 C 31.257718,21.499070 30.941129,22.279853 30.307960,22.889723 C 29.678659,23.495714 28.806586,23.798710 27.691730,23.798710 C 26.697291,23.798710 25.893188,23.546212 25.279440,23.041226 C 24.525835,22.415813 24.149042,21.596177 24.149042,20.582316 M 26.526367,20.576489 C 26.526367,21.128094 26.637078,21.535973 26.858500,21.800116 C 27.083800,22.064269 27.365425,22.196340 27.703385,22.196340 C 28.045214,22.196340 28.324899,22.066208 28.542443,21.805944 C 28.763855,21.545679 28.874566,21.128094 28.874566,20.553179 C 28.874566,20.017127 28.763855,19.618964 28.542443,19.358689 C 28.321021,19.094546 28.047163,18.962475 27.720867,18.962465 C 27.375141,18.962475 27.089627,19.096485 26.864327,19.364517 C 26.639017,19.628669 26.526367,20.032660 26.526367,20.576489 M 32.388127,15.116791 L 34.812072,15.116791 L 34.812072,19.521837 L 36.583416,17.470807 L 39.502631,17.470807 L 37.282626,19.638375 L 39.630824,23.658862 L 36.956330,23.658862 L 35.703566,21.176653 L 34.812072,22.050665 L 34.812072,23.658862 L 32.388127,23.658862 L 32.388127,15.116791 M 46.832739,21.153343 L 42.078078,21.153343 C 42.120799,21.534024 42.223744,21.817598 42.386891,22.004056 C 42.616080,22.272087 42.915187,22.406107 43.284223,22.406107 C 43.517290,22.406107 43.738702,22.347833 43.948479,22.231295 C 44.076662,22.157498 44.214561,22.027365 44.362175,21.840908 L 46.698719,22.056492 C 46.341337,22.678017 45.910148,23.124739 45.405172,23.396658 C 44.900176,23.664689 44.175718,23.798710 43.231777,23.798710 C 42.412140,23.798710 41.767306,23.684110 41.297285,23.454922 C 40.827253,23.221855 40.436856,22.854768 40.126104,22.353661 C 39.819220,21.848675 39.665779,21.256287 39.665779,20.576489 C 39.665779,19.609248 39.974601,18.826515 40.592247,18.228291 C 41.213762,17.630086 42.070302,17.330979 43.161858,17.330969 C 44.047525,17.330979 44.746735,17.464989 45.259497,17.733020 C 45.772250,18.001051 46.162646,18.389509 46.430688,18.898373 C 46.698709,19.407248 46.832729,20.069564 46.832739,20.885312 L 46.832739,21.153343 M 44.420439,20.017117 C 44.373820,19.558750 44.249515,19.230506 44.047525,19.032394 C 43.849413,18.834282 43.587209,18.735226 43.260914,18.735226 C 42.884111,18.735226 42.583054,18.884779 42.357764,19.183887 C 42.214028,19.370354 42.122739,19.648091 42.083906,20.017117 L 44.420439,20.017117 M 47.934001,17.470807 L 50.142351,17.470807 L 50.142351,18.478849 C 50.472535,18.067092 50.806606,17.773812 51.144556,17.599000 C 51.482507,17.420319 51.894264,17.330979 52.379838,17.330969 C 53.036317,17.330979 53.549080,17.527142 53.918116,17.919478 C 54.291021,18.307935 54.477478,18.910038 54.477488,19.725776 L 54.477488,23.658862 L 52.094325,23.658862 L 52.094325,20.256021 C 52.094325,19.867563 52.022457,19.593705 51.878731,19.434436 C 51.735004,19.271298 51.533004,19.189714 51.272750,19.189714 C 50.985287,19.189714 50.752220,19.298486 50.573530,19.516019 C 50.394839,19.733553 50.305499,20.123950 50.305499,20.687200 L 50.305499,23.658862 L 47.934001,23.658862 L 47.934001,17.470807' />"
        "  <path"
        "     id='text1733'"
        "     style='font-size:12.000000;font-style:normal;font-variant:normal;font-weight:normal;font-stretch:normal;line-height:100.00000%;writing-mode:lr-tb;text-anchor:start;fill:#000000;fill-opacity:1.0000000;stroke:none;stroke-width:1.0000000px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1.0000000;font-family:Arial Black'"
        "     d='M 13.747801,34.415696 L 16.119299,34.415696 L 16.119299,36.029710 L 13.747801,36.029710 L 13.747801,34.415696 M 13.747801,36.769712 L 16.119299,36.769712 L 16.119299,42.957766 L 13.747801,42.957766 L 13.747801,36.769712 M 17.640095,36.769712 L 19.854272,36.769712 L 19.854272,37.672871 C 20.172801,37.296078 20.493269,37.028047 20.815696,36.868768 C 21.141991,36.709508 21.534327,36.629883 21.992704,36.629874 C 22.486035,36.629883 22.876432,36.717285 23.163894,36.892077 C 23.451337,37.066890 23.686352,37.327154 23.868931,37.672871 C 24.241836,37.268880 24.581735,36.995022 24.888619,36.851295 C 25.195493,36.703681 25.574225,36.629883 26.024845,36.629874 C 26.689090,36.629883 27.207670,36.827986 27.580595,37.224210 C 27.953500,37.616546 28.139957,38.232243 28.139967,39.071300 L 28.139967,42.957766 L 25.762641,42.957766 L 25.762641,39.432560 C 25.762631,39.152874 25.708246,38.945056 25.599484,38.809097 C 25.440214,38.595452 25.242102,38.488619 25.005157,38.488619 C 24.725462,38.488619 24.500161,38.589624 24.329247,38.791615 C 24.158323,38.993615 24.072861,39.317971 24.072871,39.764683 L 24.072871,42.957766 L 21.695536,42.957766 L 21.695536,39.549099 C 21.695536,39.277179 21.679993,39.092671 21.648926,38.995554 C 21.598419,38.840173 21.511017,38.715868 21.386723,38.622640 C 21.262408,38.525523 21.116742,38.476974 20.949706,38.476964 C 20.677787,38.476974 20.454426,38.579908 20.279624,38.785787 C 20.104821,38.991666 20.017420,39.329626 20.017420,39.799647 L 20.017420,42.957766 L 17.640095,42.957766 L 17.640095,36.769712 M 31.717620,38.774132 L 29.450995,38.535238 C 29.536457,38.139014 29.658813,37.828252 29.818082,37.602952 C 29.981230,37.373764 30.214307,37.175651 30.517302,37.008616 C 30.734826,36.888199 31.033943,36.794970 31.414624,36.728930 C 31.795305,36.662899 32.207063,36.629883 32.649906,36.629874 C 33.360771,36.629883 33.931797,36.670665 34.362976,36.752239 C 34.794155,36.829935 35.153476,36.995022 35.440938,37.247510 C 35.642928,37.422322 35.802188,37.670932 35.918736,37.993339 C 36.035264,38.311877 36.093529,38.616812 36.093538,38.908153 L 36.093538,41.640911 C 36.093529,41.932251 36.111011,42.161440 36.145975,42.328476 C 36.184818,42.491623 36.266392,42.701390 36.390707,42.957766 L 34.164874,42.957766 C 34.075524,42.798497 34.017250,42.678081 33.990062,42.596507 C 33.962874,42.511045 33.935676,42.378973 33.908488,42.200282 C 33.597726,42.499390 33.288903,42.713045 32.982029,42.841228 C 32.562495,43.012152 32.074991,43.097614 31.519508,43.097614 C 30.781445,43.097614 30.220134,42.926690 29.835565,42.584852 C 29.454883,42.243014 29.264538,41.821540 29.264538,41.320443 C 29.264538,40.850411 29.402437,40.463903 29.678244,40.160907 C 29.954042,39.857922 30.462916,39.632612 31.204857,39.484997 C 32.094413,39.306316 32.671267,39.182011 32.935419,39.112082 C 33.199562,39.038285 33.479248,38.943117 33.774477,38.826569 C 33.774467,38.535238 33.714264,38.331299 33.593847,38.214760 C 33.473421,38.098232 33.261715,38.039958 32.958719,38.039958 C 32.570271,38.039958 32.278931,38.102110 32.084707,38.226415 C 31.933204,38.323532 31.810838,38.506101 31.717620,38.774132 M 33.774477,40.021069 C 33.448172,40.137607 33.108273,40.240542 32.754790,40.329882 C 32.273103,40.458075 31.968169,40.584319 31.839975,40.708624 C 31.707904,40.836817 31.641863,40.982483 31.641873,41.145640 C 31.641863,41.332098 31.705965,41.485530 31.834158,41.605956 C 31.966219,41.722484 32.158504,41.780759 32.411002,41.780759 C 32.675145,41.780759 32.919876,41.716667 33.145176,41.588474 C 33.374365,41.460281 33.535573,41.304900 33.628802,41.122331 C 33.725909,40.935873 33.774467,40.695030 33.774477,40.399811 L 33.774477,40.021069 M 41.996080,36.769712 L 44.216085,36.769712 L 44.216085,42.613989 L 44.221913,42.887847 C 44.221903,43.276295 44.138390,43.645331 43.971354,43.994936 C 43.808196,44.348430 43.588724,44.633943 43.312926,44.851476 C 43.041007,45.069010 42.693341,45.226330 42.269939,45.323447 C 41.850405,45.420554 41.368718,45.469112 40.824890,45.469112 C 39.581841,45.469112 38.727240,45.282655 38.261107,44.909741 C 37.798842,44.536826 37.567715,44.037667 37.567715,43.412255 C 37.567715,43.334569 37.571603,43.229686 37.579369,43.097614 L 39.880948,43.359818 C 39.939213,43.573463 40.028563,43.721078 40.148979,43.802652 C 40.323782,43.923068 40.543255,43.983282 40.807407,43.983282 C 41.149246,43.983282 41.403683,43.891992 41.570719,43.709423 C 41.741633,43.526854 41.827095,43.208315 41.827095,42.753827 L 41.827095,41.815713 C 41.594018,42.091521 41.360952,42.291572 41.127885,42.415877 C 40.762737,42.610101 40.368452,42.707218 39.945050,42.707218 C 39.117637,42.707218 38.449503,42.345958 37.940629,41.623428 C 37.579369,41.110676 37.398739,40.432827 37.398739,39.589880 C 37.398739,38.626528 37.631806,37.892353 38.097949,37.387358 C 38.564093,36.882372 39.173962,36.629883 39.927568,36.629874 C 40.409244,36.629883 40.805468,36.711457 41.116230,36.874595 C 41.430871,37.037753 41.724151,37.307733 41.996080,37.684526 L 41.996080,36.769712 M 39.764410,39.770510 C 39.764410,40.217232 39.859578,40.549365 40.049923,40.766898 C 40.240269,40.980544 40.490818,41.087366 40.801580,41.087366 C 41.096799,41.087366 41.343470,40.976665 41.541582,40.755243 C 41.743572,40.529943 41.844577,40.191983 41.844577,39.741383 C 41.844577,39.290773 41.739694,38.946996 41.529937,38.710041 C 41.320160,38.469198 41.063784,38.348781 40.760798,38.348781 C 40.457802,38.348781 40.215020,38.459492 40.032451,38.680904 C 39.853760,38.898437 39.764410,39.261646 39.764410,39.770510 M 52.507607,40.452248 L 47.752946,40.452248 C 47.795667,40.832929 47.898612,41.116503 48.061759,41.302961 C 48.290948,41.570992 48.590055,41.705012 48.959091,41.705012 C 49.192158,41.705012 49.413580,41.646738 49.623347,41.530200 C 49.751530,41.456402 49.889429,41.326270 50.037043,41.139813 L 52.373587,41.355397 C 52.016205,41.976922 51.585026,42.423643 51.080040,42.695563 C 50.575044,42.963594 49.850586,43.097614 48.906644,43.097614 C 48.087008,43.097614 47.442184,42.983015 46.972153,42.753827 C 46.502121,42.520760 46.111734,42.153673 45.800972,41.652565 C 45.494088,41.147579 45.340656,40.555192 45.340656,39.875394 C 45.340656,38.908153 45.649469,38.125420 46.267115,37.527195 C 46.888640,36.928991 47.745170,36.629883 48.836725,36.629874 C 49.722393,36.629883 50.421612,36.763894 50.934375,37.031925 C 51.447127,37.299956 51.837514,37.688414 52.105555,38.197278 C 52.373577,38.706152 52.507597,39.368469 52.507607,40.184217 L 52.507607,40.452248 M 50.095317,39.316022 C 50.048698,38.857655 49.924393,38.529411 49.722403,38.331299 C 49.524281,38.133187 49.262077,38.034131 48.935782,38.034131 C 48.558979,38.034131 48.257932,38.183684 48.032632,38.482792 C 47.888896,38.669259 47.797616,38.946996 47.758774,39.316022 L 50.095317,39.316022' />"
        "  <text"
        "     xml:space='preserve'"
        "     id='text1644'"
        "     style='font-size:12.000000;font-style:normal;font-weight:normal;fill:#000000;fill-opacity:1.0000000;stroke:none;stroke-width:1.0000000px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1.0000000;font-family:Sans'"
        "     y='59.288345'"
        "     x='12.760736'><tspan"
        "       id='tspan1646'"
        "       y='59.288345'"
        "       x='12.760736'>%s</tspan></text>"
        "</svg>";

    //Fill in the template
    char *cErrorMessage = const_cast<char *>(errorMessage.c_str());
    gchar *xmlBuffer = g_strdup_printf(xformat, cErrorMessage);

    //g_message("%s\n", xmlBuffer);

    //now show it!
    showSvgFromMemory(xmlBuffer);
    g_free(xmlBuffer);
    showingBrokenImage = true;

}



static bool
hasSuffix(const Glib::ustring &str, Glib::ustring &ext)
{
    int strLen = str.length();
    int extLen = ext.length();
    if (extLen > strLen)
        {
        return false;
        }
    int strpos = strLen-1;
    for (int extpos = extLen-1 ; extpos>=0 ; extpos--, strpos--)
        {
        Glib::ustring::value_type ch = str[strpos];
        if (ch != ext[extpos])
            {
            if ( ((ch & 0xff80) != 0) ||
                 static_cast<Glib::ustring::value_type>( g_ascii_tolower( static_cast<gchar>(0x07f & ch) ) ) != ext[extpos] )
                {
                return false;
                }
            }
        }
    return true;
}


/**
 * Return true if the image is loadable by Gdk, else false
 */
static bool
isValidImageIconFile(const Glib::ustring &fileName)
{
    std::vector<Gdk::PixbufFormat>formats = Gdk::Pixbuf::get_formats();
    for (unsigned int i=0; i<formats.size(); i++)
        {
        Gdk::PixbufFormat format = formats[i];
        std::vector<Glib::ustring>extensions = format.get_extensions();
        for (unsigned int j=0; j<extensions.size(); j++)
            {
            Glib::ustring ext = extensions[j];
            if (hasSuffix(fileName, ext))
                {
                return true;
                }
            }
        }
    return false;
}

/// \fixme This function is almost an exact duplicate of SVGPreview::set() in ui/dialog/filedialogimpl-gtkmm.cpp.
bool ImageIcon::show(const Glib::ustring &fileName)
{
    if (!Glib::file_test(fileName, Glib::FILE_TEST_EXISTS)) {
        showBrokenImage("File does not exist");
        return false;
    }

    if (Glib::file_test(fileName, Glib::FILE_TEST_IS_REGULAR)) {
        gchar *fName = const_cast<gchar *>(fileName.c_str()); // this const-cast seems not necessary, was it put there because of older sys/stat.h version?
        struct stat info;
        if (stat(fName, &info)) // stat returns 0 upon success
        {
            showBrokenImage("Cannot get file info");
            return false;
        }
        if (info.st_size > 0x150000L) {
            showBrokenImage("File too large");
            return false;
        }
    }

    Glib::ustring svg = ".svg";
    Glib::ustring svgz = ".svgz";

    if (hasSuffix(fileName, svg) || hasSuffix(fileName, svgz)) {
        if (!showSvgFile(fileName)) {
            showBrokenImage(bitmapError);
            return false;
        }
        return true;
    } else if (isValidImageIconFile(fileName)) {
        if (!showBitmap(fileName)) {
            showBrokenImage(bitmapError);
            return false;
        }
        return true;
    } else {
        showBrokenImage("unsupported file type");
        return false;
    }
}







} // namespace Widget
} // namespace UI
} // namespace Inkscape


/*#########################################################################
### E N D    O F    F I L E
#########################################################################*/



