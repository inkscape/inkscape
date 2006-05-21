/*
 * Implementation of the file dialog interfaces defined in filedialog.h
 *
 * Authors:
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004 The Inkscape Organization
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



//Temporary ugly hack
//Remove these after the get_filter() calls in
//show() on both classes are fixed
#include <gtk/gtkfilechooser.h>

//Another hack
#include <gtk/gtkentry.h>
#include <gtk/gtkexpander.h>

#include <sys/stat.h>
#include <glibmm/i18n.h>
#include <gtkmm/box.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/menubar.h>
#include <gtkmm/menu.h>
#include <gtkmm/entry.h>
#include <gtkmm/expander.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/stock.h>
#include <gdkmm/pixbuf.h>

#include "prefs-utils.h"
#include <dialogs/dialog-events.h>
#include <extension/input.h>
#include <extension/output.h>
#include <extension/db.h>
#include "inkscape.h"
#include "svg-view-widget.h"
#include "filedialog.h"
#include "gc-core.h"

#undef INK_DUMP_FILENAME_CONV

#ifdef INK_DUMP_FILENAME_CONV
void dump_str( const gchar* str, const gchar* prefix );
void dump_ustr( const Glib::ustring& ustr );
#endif

namespace Inkscape {
namespace UI {
namespace Dialogs {

void FileDialogExtensionToPattern (Glib::ustring &pattern, gchar * in_file_extension);

/*#########################################################################
### SVG Preview Widget
#########################################################################*/
/**
 * Simple class for displaying an SVG file in the "preview widget."
 * Currently, this is just a wrapper of the sp_svg_view Gtk widget.
 * Hopefully we will eventually replace with a pure Gtkmm widget.
 */
class SVGPreview : public Gtk::VBox
{
public:
    SVGPreview();
    ~SVGPreview();

    bool setDocument(SPDocument *doc);

    bool setFileName(Glib::ustring &fileName);

    bool setFromMem(char const *xmlBuffer);

    bool set(Glib::ustring &fileName, int dialogType);

    bool setURI(URI &uri);

    /**
     * Show image embedded in SVG
     */
    void showImage(Glib::ustring &fileName);

    /**
     * Show the "No preview" image
     */
    void showNoPreview();

    /**
     * Show the "Too large" image
     */
    void showTooLarge(long fileLength);

private:
    /**
     * The svg document we are currently showing
     */
    SPDocument *document;

    /**
     * The sp_svg_view widget
     */
    GtkWidget *viewerGtk;

    /**
     * are we currently showing the "no preview" image?
     */
    bool showingNoPreview;

};


bool SVGPreview::setDocument(SPDocument *doc)
{
    if (document)
        sp_document_unref(document);

    sp_document_ref(doc);
    document = doc;

    //This should remove it from the box, and free resources
    if (viewerGtk) {
        gtk_widget_destroy(viewerGtk);
    }

    viewerGtk  = sp_svg_view_widget_new(doc);
    GtkWidget *vbox = (GtkWidget *)gobj();
    gtk_box_pack_start(GTK_BOX(vbox), viewerGtk, TRUE, TRUE, 0);
    gtk_widget_show(viewerGtk);

    return true;
}

bool SVGPreview::setFileName(Glib::ustring &theFileName)
{
    Glib::ustring fileName = theFileName;

    fileName = Glib::filename_to_utf8(fileName);

    SPDocument *doc = sp_document_new (fileName.c_str(), 0);
    if (!doc) {
        g_warning("SVGView: error loading document '%s'\n", fileName.c_str());
        return false;
    }

    setDocument(doc);

    sp_document_unref(doc);

    return true;
}



bool SVGPreview::setFromMem(char const *xmlBuffer)
{
    if (!xmlBuffer)
        return false;

    gint len = (gint)strlen(xmlBuffer);
    SPDocument *doc = sp_document_new_from_mem(xmlBuffer, len, 0);
    if (!doc) {
        g_warning("SVGView: error loading buffer '%s'\n",xmlBuffer);
        return false;
    }

    setDocument(doc);

    sp_document_unref(doc);

    Inkscape::GC::request_early_collection();

    return true;
}



void SVGPreview::showImage(Glib::ustring &theFileName)
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
          "<text\n"
          "  style=\"font-size:24.000000;font-style:normal;font-weight:normal;"
          "    fill:#000000;fill-opacity:1.0000000;stroke:none;"
          "    font-family:Bitstream Vera Sans\"\n"
          "  x=\"10\" y=\"26\">%d x %d</text>\n"
          "</svg>\n\n";

    //if (!Glib::get_charset()) //If we are not utf8
    fileName = Glib::filename_to_utf8(fileName);

    //Fill in the template
    /* FIXME: Do proper XML quoting for fileName. */
    gchar *xmlBuffer = g_strdup_printf(xformat,
           previewWidth, previewHeight,
           imgX, imgY, scaledImgWidth, scaledImgHeight,
           fileName.c_str(),
           rectX, rectY, rectWidth, rectHeight,
           imgWidth, imgHeight);

    //g_message("%s\n", xmlBuffer);

    //now show it!
    setFromMem(xmlBuffer);
    g_free(xmlBuffer);
}



void SVGPreview::showNoPreview()
{
    //Are we already showing it?
    if (showingNoPreview)
        return;

    //Arbitrary size of svg doc -- rather 'portrait' shaped
    gint previewWidth  = 300;
    gint previewHeight = 600;

    //Our template.  Modify to taste
    gchar const *xformat =
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
          "<svg\n"
          "xmlns=\"http://www.w3.org/2000/svg\"\n"
          "xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
          "width=\"%d\" height=\"%d\">\n"
          "<g transform=\"translate(-190,24.27184)\" style=\"opacity:0.12\">\n"
          "<path\n"
          "style=\"font-size:12;fill:#ffffff;fill-rule:evenodd;stroke:#000000;stroke-width:0.936193pt\"\n"
          "d=\"M 397.64309 320.25301 L 280.39197 282.517 L 250.74227 124.83447 L 345.08225 "
          "29.146783 L 393.59996 46.667064 L 483.89679 135.61619 L 397.64309 320.25301 z \"\n"
          "id=\"whiteSpace\" />\n"
          "<path\n"
          "style=\"font-size:12;fill-rule:evenodd;stroke-width:1pt;fill:#000000;fill-opacity:1\"\n"
          "d=\"M 476.95792 339.17168 C 495.78197 342.93607 499.54842 356.11361 495.78197 359.87802 "
          "C 492.01856 363.6434 482.6065 367.40781 475.07663 361.76014 C 467.54478 "
          "356.11361 467.54478 342.93607 476.95792 339.17168 z \"\n"
          "id=\"droplet01\" />\n"
          "<path\n"
          "style=\"font-size:12;fill-rule:evenodd;stroke-width:1pt;fill:#000000;fill-opacity:1\"\n"
          "d=\"M 286.46194 340.42914 C 284.6277 340.91835 269.30405 327.71337 257.16909 333.8338 "
          "C 245.03722 339.95336 236.89276 353.65666 248.22676 359.27982 C 259.56184 364.90298 "
          "267.66433 358.41867 277.60113 351.44119 C 287.53903 344.46477 "
          "287.18046 343.1206 286.46194 340.42914 z \"\n"
          "id=\"droplet02\" />\n"
          "<path\n"
          "style=\"font-size:12;fill-rule:evenodd;stroke-width:1pt;fill:#000000;fill-opacity:1\"\n"
          "d=\"M 510.35756 306.92856 C 520.59494 304.36879 544.24333 306.92856 540.47688 321.98634 "
          "C 536.71354 337.04806 504.71297 331.39827 484.00371 323.87156 C 482.12141 "
          "308.81083 505.53237 308.13423 510.35756 306.92856 z \"\n"
          "id=\"droplet03\" />\n"
          "<path\n"
          "style=\"font-size:12;fill-rule:evenodd;stroke-width:1pt;fill:#000000;fill-opacity:1\"\n"
          "d=\"M 359.2403 21.362537 C 347.92693 21.362537 336.6347 25.683095 327.96556 34.35223 "
          "L 173.87387 188.41466 C 165.37697 196.9114 161.1116 207.95813 160.94269 219.04577 "
          "L 160.88418 219.04577 C 160.88418 219.08524 160.94076 219.12322 160.94269 219.16279 "
          "C 160.94033 219.34888 160.88418 219.53256 160.88418 219.71865 L 161.14748 219.71865 "
          "C 164.0966 230.93917 240.29699 245.24198 248.79866 253.74346 C 261.63771 266.58263 "
          "199.5652 276.01151 212.4041 288.85074 C 225.24316 301.68979 289.99433 313.6933 302.8346 "
          "326.53254 C 315.67368 339.37161 276.5961 353.04289 289.43532 365.88196 C 302.27439 "
          "378.72118 345.40201 362.67257 337.5908 396.16198 C 354.92909 413.50026 391.10302 "
          "405.2208 415.32417 387.88252 C 428.16323 375.04345 390.6948 376.17577 403.53397 "
          "363.33668 C 416.37304 350.49745 448.78128 350.4282 476.08902 319.71589 C 465.09739 "
          "302.62116 429.10801 295.34136 441.94719 282.50217 C 454.78625 269.66311 479.74708 "
          "276.18423 533.60644 251.72479 C 559.89837 239.78398 557.72636 230.71459 557.62567 "
          "219.71865 C 557.62356 219.48727 557.62567 219.27892 557.62567 219.04577 L 557.56716 "
          "219.04577 C 557.3983 207.95812 553.10345 196.9114 544.60673 188.41466 L 390.54428 "
          "34.35223 C 381.87515 25.683095 370.55366 21.362537 359.2403 21.362537 z M 357.92378 "
          "41.402939 C 362.95327 41.533963 367.01541 45.368018 374.98006 50.530832 L 447.76915 "
          "104.50827 C 448.56596 105.02498 449.32484 105.564 450.02187 106.11735 C 450.7189 106.67062 "
          "451.3556 107.25745 451.95277 107.84347 C 452.54997 108.42842 453.09281 109.01553 453.59111 "
          "109.62808 C 454.08837 110.24052 454.53956 110.86661 454.93688 111.50048 C 455.33532 112.13538 "
          "455.69164 112.78029 455.9901 113.43137 C 456.28877 114.08363 456.52291 114.75639 456.7215 "
          "115.42078 C 456.92126 116.08419 457.08982 116.73973 457.18961 117.41019 C 457.28949 "
          "118.08184 457.33588 118.75535 457.33588 119.42886 L 414.21245 98.598549 L 409.9118 "
          "131.16055 L 386.18512 120.04324 L 349.55654 144.50131 L 335.54288 96.1703 L 317.4919 "
          "138.4453 L 267.08369 143.47735 L 267.63956 121.03795 C 267.63956 115.64823 296.69685 "
          "77.915899 314.39075 68.932902 L 346.77721 45.674327 C 351.55594 42.576634 354.90608 "
          "41.324327 357.92378 41.402939 z M 290.92738 261.61333 C 313.87149 267.56365 339.40299 "
          "275.37038 359.88393 275.50997 L 360.76161 284.72563 C 343.2235 282.91785 306.11346 "
          "274.45012 297.36372 269.98057 L 290.92738 261.61333 z \"\n"
          "id=\"mountainDroplet\" />\n"
          "</g> <g transform=\"translate(-20,0)\">\n"
          "<text xml:space=\"preserve\"\n"
          "style=\"font-size:32.000000;font-style:normal;font-variant:normal;font-weight:bold;"
          "font-stretch:normal;fill:#000000;fill-opacity:1.0000000;stroke:none;stroke-width:1.0000000pt;"
          "stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1.0000000;"
          "font-family:Bitstream Vera Sans;text-anchor:middle;writing-mode:lr\"\n"
          "x=\"190\" y=\"240\">%s</text></g>\n"
          "</svg>\n\n";

    //Fill in the template
    gchar *xmlBuffer = g_strdup_printf(xformat,
           previewWidth, previewHeight, _("No preview"));

    //g_message("%s\n", xmlBuffer);

    //now show it!
    setFromMem(xmlBuffer);
    g_free(xmlBuffer);
    showingNoPreview = true;

}

void SVGPreview::showTooLarge(long fileLength)
{

    //Arbitrary size of svg doc -- rather 'portrait' shaped
    gint previewWidth  = 300;
    gint previewHeight = 600;

    //Our template.  Modify to taste
    gchar const *xformat =
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
          "<svg\n"
          "xmlns=\"http://www.w3.org/2000/svg\"\n"
          "xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
          "width=\"%d\" height=\"%d\">\n"
          "<g transform=\"translate(-170,24.27184)\" style=\"opacity:0.12\">\n"
          "<path\n"
          "style=\"font-size:12;fill:#ffffff;fill-rule:evenodd;stroke:#000000;stroke-width:0.936193pt\"\n"
          "d=\"M 397.64309 320.25301 L 280.39197 282.517 L 250.74227 124.83447 L 345.08225 "
          "29.146783 L 393.59996 46.667064 L 483.89679 135.61619 L 397.64309 320.25301 z \"\n"
          "id=\"whiteSpace\" />\n"
          "<path\n"
          "style=\"font-size:12;fill-rule:evenodd;stroke-width:1pt;fill:#000000;fill-opacity:1\"\n"
          "d=\"M 476.95792 339.17168 C 495.78197 342.93607 499.54842 356.11361 495.78197 359.87802 "
          "C 492.01856 363.6434 482.6065 367.40781 475.07663 361.76014 C 467.54478 "
          "356.11361 467.54478 342.93607 476.95792 339.17168 z \"\n"
          "id=\"droplet01\" />\n"
          "<path\n"
          "style=\"font-size:12;fill-rule:evenodd;stroke-width:1pt;fill:#000000;fill-opacity:1\"\n"
          "d=\"M 286.46194 340.42914 C 284.6277 340.91835 269.30405 327.71337 257.16909 333.8338 "
          "C 245.03722 339.95336 236.89276 353.65666 248.22676 359.27982 C 259.56184 364.90298 "
          "267.66433 358.41867 277.60113 351.44119 C 287.53903 344.46477 "
          "287.18046 343.1206 286.46194 340.42914 z \"\n"
          "id=\"droplet02\" />\n"
          "<path\n"
          "style=\"font-size:12;fill-rule:evenodd;stroke-width:1pt;fill:#000000;fill-opacity:1\"\n"
          "d=\"M 510.35756 306.92856 C 520.59494 304.36879 544.24333 306.92856 540.47688 321.98634 "
          "C 536.71354 337.04806 504.71297 331.39827 484.00371 323.87156 C 482.12141 "
          "308.81083 505.53237 308.13423 510.35756 306.92856 z \"\n"
          "id=\"droplet03\" />\n"
          "<path\n"
          "style=\"font-size:12;fill-rule:evenodd;stroke-width:1pt;fill:#000000;fill-opacity:1\"\n"
          "d=\"M 359.2403 21.362537 C 347.92693 21.362537 336.6347 25.683095 327.96556 34.35223 "
          "L 173.87387 188.41466 C 165.37697 196.9114 161.1116 207.95813 160.94269 219.04577 "
          "L 160.88418 219.04577 C 160.88418 219.08524 160.94076 219.12322 160.94269 219.16279 "
          "C 160.94033 219.34888 160.88418 219.53256 160.88418 219.71865 L 161.14748 219.71865 "
          "C 164.0966 230.93917 240.29699 245.24198 248.79866 253.74346 C 261.63771 266.58263 "
          "199.5652 276.01151 212.4041 288.85074 C 225.24316 301.68979 289.99433 313.6933 302.8346 "
          "326.53254 C 315.67368 339.37161 276.5961 353.04289 289.43532 365.88196 C 302.27439 "
          "378.72118 345.40201 362.67257 337.5908 396.16198 C 354.92909 413.50026 391.10302 "
          "405.2208 415.32417 387.88252 C 428.16323 375.04345 390.6948 376.17577 403.53397 "
          "363.33668 C 416.37304 350.49745 448.78128 350.4282 476.08902 319.71589 C 465.09739 "
          "302.62116 429.10801 295.34136 441.94719 282.50217 C 454.78625 269.66311 479.74708 "
          "276.18423 533.60644 251.72479 C 559.89837 239.78398 557.72636 230.71459 557.62567 "
          "219.71865 C 557.62356 219.48727 557.62567 219.27892 557.62567 219.04577 L 557.56716 "
          "219.04577 C 557.3983 207.95812 553.10345 196.9114 544.60673 188.41466 L 390.54428 "
          "34.35223 C 381.87515 25.683095 370.55366 21.362537 359.2403 21.362537 z M 357.92378 "
          "41.402939 C 362.95327 41.533963 367.01541 45.368018 374.98006 50.530832 L 447.76915 "
          "104.50827 C 448.56596 105.02498 449.32484 105.564 450.02187 106.11735 C 450.7189 106.67062 "
          "451.3556 107.25745 451.95277 107.84347 C 452.54997 108.42842 453.09281 109.01553 453.59111 "
          "109.62808 C 454.08837 110.24052 454.53956 110.86661 454.93688 111.50048 C 455.33532 112.13538 "
          "455.69164 112.78029 455.9901 113.43137 C 456.28877 114.08363 456.52291 114.75639 456.7215 "
          "115.42078 C 456.92126 116.08419 457.08982 116.73973 457.18961 117.41019 C 457.28949 "
          "118.08184 457.33588 118.75535 457.33588 119.42886 L 414.21245 98.598549 L 409.9118 "
          "131.16055 L 386.18512 120.04324 L 349.55654 144.50131 L 335.54288 96.1703 L 317.4919 "
          "138.4453 L 267.08369 143.47735 L 267.63956 121.03795 C 267.63956 115.64823 296.69685 "
          "77.915899 314.39075 68.932902 L 346.77721 45.674327 C 351.55594 42.576634 354.90608 "
          "41.324327 357.92378 41.402939 z M 290.92738 261.61333 C 313.87149 267.56365 339.40299 "
          "275.37038 359.88393 275.50997 L 360.76161 284.72563 C 343.2235 282.91785 306.11346 "
          "274.45012 297.36372 269.98057 L 290.92738 261.61333 z \"\n"
          "id=\"mountainDroplet\" />\n"
          "</g>\n"
          "<text xml:space=\"preserve\"\n"
          "style=\"font-size:32.000000;font-style:normal;font-variant:normal;font-weight:bold;"
          "font-stretch:normal;fill:#000000;fill-opacity:1.0000000;stroke:none;stroke-width:1.0000000pt;"
          "stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1.0000000;"
          "font-family:Bitstream Vera Sans;text-anchor:middle;writing-mode:lr\"\n"
          "x=\"170\" y=\"215\">%5.1f MB</text>\n"
          "<text xml:space=\"preserve\"\n"
          "style=\"font-size:24.000000;font-style:normal;font-variant:normal;font-weight:bold;"
          "font-stretch:normal;fill:#000000;fill-opacity:1.0000000;stroke:none;stroke-width:1.0000000pt;"
          "stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1.0000000;"
          "font-family:Bitstream Vera Sans;text-anchor:middle;writing-mode:lr\"\n"
          "x=\"180\" y=\"245\">%s</text>\n"
          "</svg>\n\n";

    //Fill in the template
    double floatFileLength = ((double)fileLength) / 1048576.0;
    //printf("%ld %f\n", fileLength, floatFileLength);
    gchar *xmlBuffer = g_strdup_printf(xformat,
           previewWidth, previewHeight, floatFileLength,
           _("too large for preview"));

    //g_message("%s\n", xmlBuffer);

    //now show it!
    setFromMem(xmlBuffer);
    g_free(xmlBuffer);

}

static bool
hasSuffix(Glib::ustring &str, Glib::ustring &ext)
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
isValidImageFile(Glib::ustring &fileName)
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

bool SVGPreview::set(Glib::ustring &fileName, int dialogType)
{

    if (!Glib::file_test(fileName, Glib::FILE_TEST_EXISTS))
        return false;

    gchar *fName = (gchar *)fileName.c_str();
    //g_message("fname:%s\n", fName);


    if (Glib::file_test(fileName, Glib::FILE_TEST_IS_REGULAR))
        {
        struct stat info;
        if (stat(fName, &info))
            {
            return FALSE;
            }
        long fileLen = info.st_size;
        if (fileLen > 0x150000L)
            {
            showingNoPreview = false;
            showTooLarge(fileLen);
            return FALSE;
            }
        }

    Glib::ustring svg = ".svg";
    Glib::ustring svgz = ".svgz";

    if ((dialogType == SVG_TYPES || dialogType == IMPORT_TYPES) &&
           (hasSuffix(fileName, svg) || hasSuffix(fileName, svgz)   )
         )
        {
        bool retval = setFileName(fileName);
        showingNoPreview = false;
        return retval;
        }
    else if (isValidImageFile(fileName))
        {
        showImage(fileName);
        showingNoPreview = false;
        return true;
        }
    else
        {
        showNoPreview();
        return false;
        }
}


SVGPreview::SVGPreview()
{
    if (!INKSCAPE)
        inkscape_application_init("",false);
    document = NULL;
    viewerGtk = NULL;
    set_size_request(150,150);
    showingNoPreview = false;
}

SVGPreview::~SVGPreview()
{

}





/*#########################################################################
### F I L E    O P E N
#########################################################################*/

/**
 * Our implementation class for the FileOpenDialog interface..
 */
class FileOpenDialogImpl : public FileOpenDialog, public Gtk::FileChooserDialog
{
public:
    FileOpenDialogImpl(char const *dir,
                       FileDialogType fileTypes,
                       char const *title);

    virtual ~FileOpenDialogImpl();

    bool show();

    Inkscape::Extension::Extension *getSelectionType();

    gchar *getFilename();

    Glib::SListHandle<Glib::ustring> getFilenames ();
protected:



private:


    /**
     * What type of 'open' are we? (open, import, place, etc)
     */
    FileDialogType dialogType;

    /**
     * Our svg preview widget
     */
    SVGPreview svgPreview;

    /**
     * Callback for seeing if the preview needs to be drawn
     */
    void updatePreviewCallback();

    /**
     * Fix to allow the user to type the file name
     */
    Gtk::Entry fileNameEntry;

    /**
     *  Create a filter menu for this type of dialog
     */
    void createFilterMenu();

    /**
     * Callback for user input into fileNameEntry
     */
    void fileNameEntryChangedCallback();

    /**
     * Callback for user changing which item is selected on the list
     */
    void fileSelectedCallback();


    /**
     * Filter name->extension lookup
     */
    std::map<Glib::ustring, Inkscape::Extension::Extension *> extensionMap;

    /**
     * The extension to use to write this file
     */
    Inkscape::Extension::Extension *extension;

    /**
     * Filename that was given
     */
    Glib::ustring myFilename;

};





/**
 * Callback for checking if the preview needs to be redrawn
 */
void FileOpenDialogImpl::updatePreviewCallback()
{
    Glib::ustring fileName = get_preview_filename();
    if (fileName.length() < 1)
        return;
    svgPreview.set(fileName, dialogType);
}





/**
 * Callback for fileNameEntry widget
 */
void FileOpenDialogImpl::fileNameEntryChangedCallback()
{
    Glib::ustring fileName = fileNameEntry.get_text();

    // TODO remove this leak
    fileName = Glib::filename_from_utf8(fileName);

    //g_message("User hit return.  Text is '%s'\n", fName.c_str());

    if (!Glib::path_is_absolute(fileName)) {
        //try appending to the current path
        // not this way: fileName = get_current_folder() + "/" + fName;
        std::vector<Glib::ustring> pathSegments;
        pathSegments.push_back( get_current_folder() );
        pathSegments.push_back( fileName );
        fileName = Glib::build_filename(pathSegments);
    }

    //g_message("path:'%s'\n", fName.c_str());

    if (Glib::file_test(fileName, Glib::FILE_TEST_IS_DIR)) {
        set_current_folder(fileName);
    } else if (Glib::file_test(fileName, Glib::FILE_TEST_IS_REGULAR)) {
        //dialog with either (1) select a regular file or (2) cd to dir
        //simulate an 'OK'
        set_filename(fileName);
        response(Gtk::RESPONSE_OK);
    }
}





/**
 * Callback for fileNameEntry widget
 */
void FileOpenDialogImpl::fileSelectedCallback()
{
    Glib::ustring fileName     = get_filename();
    if (!Glib::get_charset()) //If we are not utf8
        fileName = Glib::filename_to_utf8(fileName);
    //g_message("User selected '%s'\n",
    //       filename().c_str());

#ifdef INK_DUMP_FILENAME_CONV
    ::dump_ustr( get_filename() );
#endif
    fileNameEntry.set_text(fileName);
}




void FileOpenDialogImpl::createFilterMenu()
{
    //patterns added dynamically below
    Gtk::FileFilter allImageFilter;
    allImageFilter.set_name(_("All Images"));
    extensionMap[Glib::ustring(_("All Images"))]=NULL;
    add_filter(allImageFilter);

    Gtk::FileFilter allFilter;
    allFilter.set_name(_("All Files"));
    extensionMap[Glib::ustring(_("All Files"))]=NULL;
    allFilter.add_pattern("*");
    add_filter(allFilter);

    //patterns added dynamically below
    Gtk::FileFilter allInkscapeFilter;
    allInkscapeFilter.set_name(_("All Inkscape Files"));
    extensionMap[Glib::ustring(_("All Inkscape Files"))]=NULL;
    add_filter(allInkscapeFilter);

    Inkscape::Extension::DB::InputList extension_list;
    Inkscape::Extension::db.get_input_list(extension_list);

    for (Inkscape::Extension::DB::InputList::iterator current_item = extension_list.begin();
         current_item != extension_list.end(); current_item++)
    {
        Inkscape::Extension::Input * imod = *current_item;

        // FIXME: would be nice to grey them out instead of not listing them
        if (imod->deactivated()) continue;

        Glib::ustring upattern("*");
        FileDialogExtensionToPattern (upattern, imod->get_extension());

        Gtk::FileFilter filter;
        Glib::ustring uname(_(imod->get_filetypename()));
        filter.set_name(uname);
        filter.add_pattern(upattern);
        add_filter(filter);
        extensionMap[uname] = imod;

        //g_message("ext %s:%s '%s'\n", ioext->name, ioext->mimetype, upattern.c_str());
        allInkscapeFilter.add_pattern(upattern);
        if ( strncmp("image", imod->get_mimetype(), 5)==0 )
            allImageFilter.add_pattern(upattern);
    }

    return;
}



/**
 * Constructor.  Not called directly.  Use the factory.
 */
FileOpenDialogImpl::FileOpenDialogImpl(char const *dir,
                                       FileDialogType fileTypes,
                                       char const *title) :
                 Gtk::FileChooserDialog(Glib::ustring(title))
{


    /* One file at a time */
    /* And also Multiple Files */
    set_select_multiple(true);

    /* Initalize to Autodetect */
    extension = NULL;
    /* No filename to start out with */
    myFilename = "";

    /* Set our dialog type (open, import, etc...)*/
    dialogType = fileTypes;


    /* Set the pwd and/or the filename */
    if (dir != NULL)
    {
        Glib::ustring udir(dir);
        Glib::ustring::size_type len = udir.length();
        // leaving a trailing backslash on the directory name leads to the infamous
        // double-directory bug on win32
        if (len != 0 && udir[len - 1] == '\\') udir.erase(len - 1);
        set_current_folder(udir.c_str());
    }

    //###### Add the file types menu
    createFilterMenu();

    //###### Add a preview widget
    set_preview_widget(svgPreview);
    set_preview_widget_active(true);
    set_use_preview_label (false);

    //Catch selection-changed events, so we can adjust the text widget
    signal_update_preview().connect(
         sigc::mem_fun(*this, &FileOpenDialogImpl::updatePreviewCallback) );


    //###### Add a text entry bar, and tie it to file chooser events
    fileNameEntry.set_text(get_current_folder());
    set_extra_widget(fileNameEntry);
    fileNameEntry.grab_focus();

    //Catch when user hits [return] on the text field
    fileNameEntry.signal_activate().connect(
         sigc::mem_fun(*this, &FileOpenDialogImpl::fileNameEntryChangedCallback) );

    //Catch selection-changed events, so we can adjust the text widget
    signal_selection_changed().connect(
         sigc::mem_fun(*this, &FileOpenDialogImpl::fileSelectedCallback) );

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_OK);

}





/**
 * Public factory.  Called by file.cpp, among others.
 */
FileOpenDialog *FileOpenDialog::create(char const *path,
                                       FileDialogType fileTypes,
                                       char const *title)
{
    FileOpenDialog *dialog = new FileOpenDialogImpl(path, fileTypes, title);
    return dialog;
}




/**
 * Destructor
 */
FileOpenDialogImpl::~FileOpenDialogImpl()
{

}


/**
 * Show this dialog modally.  Return true if user hits [OK]
 */
bool
FileOpenDialogImpl::show()
{
    set_current_folder(get_current_folder()); //hack to force initial dir listing
    set_modal (TRUE);                      //Window
    sp_transientize((GtkWidget *)gobj());  //Make transient
    gint b = run();                        //Dialog
    svgPreview.showNoPreview();
    hide();

    if (b == Gtk::RESPONSE_OK)
        {
        //This is a hack, to avoid the warning messages that
        //Gtk::FileChooser::get_filter() returns
        //should be:  Gtk::FileFilter *filter = get_filter();
        GtkFileChooser *gtkFileChooser = Gtk::FileChooser::gobj();
        GtkFileFilter *filter = gtk_file_chooser_get_filter(gtkFileChooser);
        if (filter)
            {
            //Get which extension was chosen, if any
            extension = extensionMap[gtk_file_filter_get_name(filter)];
            }
        myFilename = get_filename();
        return TRUE;
        }
    else
       {
       return FALSE;
       }
}




/**
 * Get the file extension type that was selected by the user. Valid after an [OK]
 */
Inkscape::Extension::Extension *
FileOpenDialogImpl::getSelectionType()
{
    return extension;
}


/**
 * Get the file name chosen by the user.   Valid after an [OK]
 */
gchar *
FileOpenDialogImpl::getFilename (void)
{
    return g_strdup(myFilename.c_str());
}


/**
 * To Get Multiple filenames selected at-once.
 */
Glib::SListHandle<Glib::ustring>FileOpenDialogImpl::getFilenames()
{    
    return get_filenames();
}






/*#########################################################################
# F I L E    S A V E
#########################################################################*/

class FileType
{
    public:
    FileType() {}
    ~FileType() {}
    Glib::ustring name;
    Glib::ustring pattern;
    Inkscape::Extension::Extension *extension;
};

/**
 * Our implementation of the FileSaveDialog interface.
 */
class FileSaveDialogImpl : public FileSaveDialog, public Gtk::FileChooserDialog
{

public:
    FileSaveDialogImpl(char const *dir,
                       FileDialogType fileTypes,
                       char const *title,
                       char const *default_key);

    virtual ~FileSaveDialogImpl();

    bool show();

    Inkscape::Extension::Extension *getSelectionType();

    gchar *getFilename();


private:

    /**
     * What type of 'open' are we? (save, export, etc)
     */
    FileDialogType dialogType;

    /**
     * Our svg preview widget
     */
    SVGPreview svgPreview;

    /**
     * Fix to allow the user to type the file name
     */
    Gtk::Entry *fileNameEntry;

    /**
     * Callback for seeing if the preview needs to be drawn
     */
    void updatePreviewCallback();



    /**
     * Allow the specification of the output file type
     */
    Gtk::HBox fileTypeBox;

    /**
     * Allow the specification of the output file type
     */
    Gtk::ComboBoxText fileTypeComboBox;


    /**
     *  Data mirror of the combo box
     */
    std::vector<FileType> fileTypes;

    //# Child widgets
    Gtk::CheckButton fileTypeCheckbox;


    /**
     * Callback for user input into fileNameEntry
     */
    void fileTypeChangedCallback();

    /**
     *  Create a filter menu for this type of dialog
     */
    void createFileTypeMenu();


    bool append_extension;

    /**
     * The extension to use to write this file
     */
    Inkscape::Extension::Extension *extension;

    /**
     * Callback for user input into fileNameEntry
     */
    void fileNameEntryChangedCallback();

    /**
     * Filename that was given
     */
    Glib::ustring myFilename;
};






/**
 * Callback for checking if the preview needs to be redrawn
 */
void FileSaveDialogImpl::updatePreviewCallback()
{
    Glib::ustring fileName = get_preview_filename();
    if (!fileName.c_str())
        return;
    bool retval = svgPreview.set(fileName, dialogType);
    set_preview_widget_active(retval);
}



/**
 * Callback for fileNameEntry widget
 */
void FileSaveDialogImpl::fileNameEntryChangedCallback()
{
    if (!fileNameEntry)
        return;

    Glib::ustring fileName = fileNameEntry->get_text();
    if (!Glib::get_charset()) //If we are not utf8
        fileName = Glib::filename_to_utf8(fileName);

    //g_message("User hit return.  Text is '%s'\n", fileName.c_str());

    if (!Glib::path_is_absolute(fileName)) {
        //try appending to the current path
        // not this way: fileName = get_current_folder() + "/" + fileName;
        std::vector<Glib::ustring> pathSegments;
        pathSegments.push_back( get_current_folder() );
        pathSegments.push_back( fileName );
        fileName = Glib::build_filename(pathSegments);
    }

    //g_message("path:'%s'\n", fileName.c_str());

    if (Glib::file_test(fileName, Glib::FILE_TEST_IS_DIR)) {
        set_current_folder(fileName);
    } else if (/*Glib::file_test(fileName, Glib::FILE_TEST_IS_REGULAR)*/1) {
        //dialog with either (1) select a regular file or (2) cd to dir
        //simulate an 'OK'
        set_filename(fileName);
        response(Gtk::RESPONSE_OK);
    }
}



/**
 * Callback for fileNameEntry widget
 */
void FileSaveDialogImpl::fileTypeChangedCallback()
{
    int sel = fileTypeComboBox.get_active_row_number();
    if (sel<0 || sel >= (int)fileTypes.size())
        return;
    FileType type = fileTypes[sel];
    //g_message("selected: %s\n", type.name.c_str());
    Gtk::FileFilter filter;
    filter.add_pattern(type.pattern);
    set_filter(filter);
}



void FileSaveDialogImpl::createFileTypeMenu()
{
    Inkscape::Extension::DB::OutputList extension_list;
    Inkscape::Extension::db.get_output_list(extension_list);

    for (Inkscape::Extension::DB::OutputList::iterator current_item = extension_list.begin();
         current_item != extension_list.end(); current_item++)
    {
        Inkscape::Extension::Output * omod = *current_item;

        // FIXME: would be nice to grey them out instead of not listing them
        if (omod->deactivated()) continue;

        FileType type;
        type.name     = (_(omod->get_filetypename()));
        type.pattern  = "*";
        FileDialogExtensionToPattern (type.pattern, omod->get_extension());
        type.extension= omod;
        fileTypeComboBox.append_text(type.name);
        fileTypes.push_back(type);
    }

    //#Let user choose
    FileType guessType;
    guessType.name = _("Guess from extension");
    guessType.pattern = "*";
    guessType.extension = NULL;
    fileTypeComboBox.append_text(guessType.name);
    fileTypes.push_back(guessType);


    fileTypeComboBox.set_active(0);
    fileTypeChangedCallback(); //call at least once to set the filter
}


void findEntryWidgets(Gtk::Container *parent, std::vector<Gtk::Entry *> &result)
{
    if (!parent)
        return;
    std::vector<Gtk::Widget *> children = parent->get_children();
    for (unsigned int i=0; i<children.size() ; i++)
        {
        Gtk::Widget *child = children[i];
        GtkWidget *wid = child->gobj();
        if (GTK_IS_ENTRY(wid))
           result.push_back((Gtk::Entry *)child);
        else if (GTK_IS_CONTAINER(wid))
            findEntryWidgets((Gtk::Container *)child, result);
        }

}

void findExpanderWidgets(Gtk::Container *parent, std::vector<Gtk::Expander *> &result)
{
    if (!parent)
        return;
    std::vector<Gtk::Widget *> children = parent->get_children();
    for (unsigned int i=0; i<children.size() ; i++)
        {
        Gtk::Widget *child = children[i];
        GtkWidget *wid = child->gobj();
        if (GTK_IS_EXPANDER(wid))
           result.push_back((Gtk::Expander *)child);
        else if (GTK_IS_CONTAINER(wid))
            findExpanderWidgets((Gtk::Container *)child, result);
        }

}


/**
 * Constructor
 */
FileSaveDialogImpl::FileSaveDialogImpl(char const *dir,
                                       FileDialogType fileTypes,
                                       char const *title,
                                       char const *default_key) :
                                       Gtk::FileChooserDialog(Glib::ustring(title),
                                           Gtk::FILE_CHOOSER_ACTION_SAVE)
{
    append_extension = (bool)prefs_get_int_attribute("dialogs.save_as", "append_extension", 1);

    /* One file at a time */
    set_select_multiple(false);

    /* Initalize to Autodetect */
    extension = NULL;
    /* No filename to start out with */
    myFilename = "";

    /* Set our dialog type (save, export, etc...)*/
    dialogType = fileTypes;

    /* Set the pwd and/or the filename */
    if (dir != NULL)
    {
        Glib::ustring udir(dir);
        Glib::ustring::size_type len = udir.length();
        // leaving a trailing backslash on the directory name leads to the infamous
        // double-directory bug on win32
        if (len != 0 && udir[len - 1] == '\\') udir.erase(len - 1);
        set_current_folder(udir.c_str());
    }

    //###### Add the file types menu
    //createFilterMenu();

    //###### Do we want the .xxx extension automatically added?
    fileTypeCheckbox.set_label(Glib::ustring(_("Append filename extension automatically")));
    fileTypeCheckbox.set_active(append_extension);

    fileTypeBox.pack_start(fileTypeCheckbox);
    createFileTypeMenu();
    fileTypeComboBox.set_size_request(200,40);
    fileTypeComboBox.signal_changed().connect(
         sigc::mem_fun(*this, &FileSaveDialogImpl::fileTypeChangedCallback) );

    fileTypeBox.pack_start(fileTypeComboBox);

    set_extra_widget(fileTypeBox);
    //get_vbox()->pack_start(fileTypeBox, false, false, 0);
    //get_vbox()->reorder_child(fileTypeBox, 2);

    //###### Add a preview widget
    set_preview_widget(svgPreview);
    set_preview_widget_active(true);
    set_use_preview_label (false);

    //Catch selection-changed events, so we can adjust the text widget
    signal_update_preview().connect(
         sigc::mem_fun(*this, &FileSaveDialogImpl::updatePreviewCallback) );


    //Let's do some customization
    fileNameEntry = NULL;
    Gtk::Container *cont = get_toplevel();
    std::vector<Gtk::Entry *> entries;
    findEntryWidgets(cont, entries);
    //g_message("Found %d entry widgets\n", entries.size());
    if (entries.size() >=1 )
        {
        //Catch when user hits [return] on the text field
        fileNameEntry = entries[0];
        fileNameEntry->signal_activate().connect(
             sigc::mem_fun(*this, &FileSaveDialogImpl::fileNameEntryChangedCallback) );
        }

    //Let's do more customization
    std::vector<Gtk::Expander *> expanders;
    findExpanderWidgets(cont, expanders);
    //g_message("Found %d expander widgets\n", expanders.size());
    if (expanders.size() >=1 )
        {
        //Always show the file list
        Gtk::Expander *expander = expanders[0];
        expander->set_expanded(true);
        }


    //if (extension == NULL)
    //    checkbox.set_sensitive(FALSE);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::SAVE,   Gtk::RESPONSE_OK);

    show_all_children();
}



/**
 * Public factory method.  Used in file.cpp
 */
FileSaveDialog *FileSaveDialog::create(char const *path,
                                       FileDialogType fileTypes,
                                       char const *title,
                                       char const *default_key)
{
    FileSaveDialog *dialog = new FileSaveDialogImpl(path, fileTypes, title, default_key);
    return dialog;
}





/**
 * Destructor
 */
FileSaveDialogImpl::~FileSaveDialogImpl()
{
}




/**
 * Show this dialog modally.  Return true if user hits [OK]
 */
bool
FileSaveDialogImpl::show()
{
    set_current_folder(get_current_folder()); //hack to force initial dir listing
    set_modal (TRUE);                      //Window
    sp_transientize((GtkWidget *)gobj());  //Make transient
    gint b = run();                        //Dialog
    svgPreview.showNoPreview();
    hide();

    if (b == Gtk::RESPONSE_OK)
        {
        int sel = fileTypeComboBox.get_active_row_number ();
        if (sel>=0 && sel< (int)fileTypes.size())
            {
            FileType &type = fileTypes[sel];
            extension = type.extension;
            }
        myFilename = get_filename();

        /*

        // FIXME: Why do we have more code

        append_extension = checkbox.get_active();
        prefs_set_int_attribute("dialogs.save_as", "append_extension", append_extension);
        prefs_set_string_attribute("dialogs.save_as", "default",
                  ( extension != NULL ? extension->get_id() : "" ));
        */
        return TRUE;
        }
    else
        {
        return FALSE;
        }
}


/**
 * Get the file extension type that was selected by the user. Valid after an [OK]
 */
Inkscape::Extension::Extension *
FileSaveDialogImpl::getSelectionType()
{
    return extension;
}


/**
 * Get the file name chosen by the user.   Valid after an [OK]
 */
gchar *
FileSaveDialogImpl::getFilename()
{
    return g_strdup(myFilename.c_str());
}

/**
    \brief  A quick function to turn a standard extension into a searchable
            pattern for the file dialogs
    \param  pattern  The patter that the extension should be written to
    \param  in_file_extension  The C string that represents the extension

    This function just goes through the string, and takes all characters
    and puts a [<upper><lower>] so that both are searched and shown in
    the file dialog.  This function edits the pattern string to make
    this happen.
*/
void
FileDialogExtensionToPattern (Glib::ustring &pattern, gchar * in_file_extension)
{
    Glib::ustring tmp(in_file_extension);

    for ( guint i = 0; i < tmp.length(); i++ ) {
        Glib::ustring::value_type ch = tmp.at(i);
        if ( Glib::Unicode::isalpha(ch) ) {
            pattern += '[';
            pattern += Glib::Unicode::toupper(ch);
            pattern += Glib::Unicode::tolower(ch);
            pattern += ']';
        } else {
            pattern += ch;
        }
    }
}

} //namespace Dialogs
} //namespace UI
} //namespace Inkscape


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
