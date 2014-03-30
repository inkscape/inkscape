/**
 * @file
 * Implementation of the file dialog interfaces defined in filedialogimpl.h.
 */
/* Authors:
 *   Bob Jamison
 *   Joel Holdsworth
 *   Bruno Dilly
 *   Other dudes from The Inkscape Organization
 *   Abhishek Sharma
 *
 * Copyright (C) 2004-2007 Bob Jamison
 * Copyright (C) 2006 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2007-2008 Joel Holdsworth
 * Copyright (C) 2004-2007 The Inkscape Organization
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "filedialogimpl-gtkmm.h"
#include "dialogs/dialog-events.h"
#include "interface.h"
#include "io/sys.h"
#include "path-prefix.h"
#include "preferences.h"

#ifdef WITH_GNOME_VFS
#include <libgnomevfs/gnome-vfs.h>
#endif

#include <gtkmm/expander.h>
#include <gtkmm/stock.h>

#include <glibmm/convert.h>
#include <glibmm/fileutils.h>
#include <glibmm/i18n.h>
#include <glibmm/miscutils.h>

#include "extension/input.h"
#include "extension/output.h"
#include "extension/db.h"
#include "svg-view-widget.h"
#include "inkscape.h"

// Routines from file.cpp
#undef INK_DUMP_FILENAME_CONV

#ifdef INK_DUMP_FILENAME_CONV
void dump_str(const gchar *str, const gchar *prefix);
void dump_ustr(const Glib::ustring &ustr);
#endif



namespace Inkscape {
namespace UI {
namespace Dialog {



//########################################################################
//### U T I L I T Y
//########################################################################

void fileDialogExtensionToPattern(Glib::ustring &pattern, Glib::ustring &extension)
{
    for (unsigned int i = 0; i < extension.length(); ++i) {
        Glib::ustring::value_type ch = extension[i];
        if (Glib::Unicode::isalpha(ch)) {
            pattern += '[';
            pattern += Glib::Unicode::toupper(ch);
            pattern += Glib::Unicode::tolower(ch);
            pattern += ']';
        } else {
            pattern += ch;
        }
    }
}


void findEntryWidgets(Gtk::Container *parent, std::vector<Gtk::Entry *> &result)
{
    if (!parent) {
        return;
    }
    std::vector<Gtk::Widget *> children = parent->get_children();
    for (unsigned int i = 0; i < children.size(); ++i) {
        Gtk::Widget *child = children[i];
        GtkWidget *wid = child->gobj();
        if (GTK_IS_ENTRY(wid))
            result.push_back(dynamic_cast<Gtk::Entry *>(child));
        else if (GTK_IS_CONTAINER(wid))
            findEntryWidgets(dynamic_cast<Gtk::Container *>(child), result);
    }
}

void findExpanderWidgets(Gtk::Container *parent, std::vector<Gtk::Expander *> &result)
{
    if (!parent)
        return;
    std::vector<Gtk::Widget *> children = parent->get_children();
    for (unsigned int i = 0; i < children.size(); ++i) {
        Gtk::Widget *child = children[i];
        GtkWidget *wid = child->gobj();
        if (GTK_IS_EXPANDER(wid))
            result.push_back(dynamic_cast<Gtk::Expander *>(child));
        else if (GTK_IS_CONTAINER(wid))
            findExpanderWidgets(dynamic_cast<Gtk::Container *>(child), result);
    }
}


/*#########################################################################
### SVG Preview Widget
#########################################################################*/

bool SVGPreview::setDocument(SPDocument *doc)
{
    if (document)
        document->doUnref();

    doc->doRef();
    document = doc;

    // This should remove it from the box, and free resources
    if (viewerGtk)
        Gtk::Container::remove(*viewerGtk);

    viewerGtk = Glib::wrap(sp_svg_view_widget_new(doc));
    Gtk::VBox *vbox = Glib::wrap(gobj());
    vbox->pack_start(*viewerGtk, TRUE, TRUE, 0);
    viewerGtk->show();
    return true;
}


bool SVGPreview::setFileName(Glib::ustring &theFileName)
{
    Glib::ustring fileName = theFileName;

    fileName = Glib::filename_to_utf8(fileName);

    /**
     * I don't know why passing false to keepalive is bad.  But it
     * prevents the display of an svg with a non-ascii filename
     */
    SPDocument *doc = SPDocument::createNewDoc(fileName.c_str(), true);
    if (!doc) {
        g_warning("SVGView: error loading document '%s'\n", fileName.c_str());
        return false;
    }

    setDocument(doc);

    doc->doUnref();

    return true;
}



bool SVGPreview::setFromMem(char const *xmlBuffer)
{
    if (!xmlBuffer)
        return false;

    gint len = (gint)strlen(xmlBuffer);
    SPDocument *doc = SPDocument::createNewDocFromMem(xmlBuffer, len, 0);
    if (!doc) {
        g_warning("SVGView: error loading buffer '%s'\n", xmlBuffer);
        return false;
    }

    setDocument(doc);

    doc->doUnref();

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

    // Arbitrary size of svg doc -- rather 'portrait' shaped
    gint previewWidth = 400;
    gint previewHeight = 600;

    // Get some image info. Smart pointer does not need to be deleted
    Glib::RefPtr<Gdk::Pixbuf> img(NULL);
    try
    {
        img = Gdk::Pixbuf::create_from_file(fileName);
    }
    catch (const Glib::FileError &e)
    {
        g_message("caught Glib::FileError in SVGPreview::showImage");
        return;
    }
    catch (const Gdk::PixbufError &e)
    {
        g_message("Gdk::PixbufError in SVGPreview::showImage");
        return;
    }
    catch (...)
    {
        g_message("Caught ... in SVGPreview::showImage");
        return;
    }

    gint imgWidth = img->get_width();
    gint imgHeight = img->get_height();

    // Find the minimum scale to fit the image inside the preview area
    double scaleFactorX = (0.9 * (double)previewWidth) / ((double)imgWidth);
    double scaleFactorY = (0.9 * (double)previewHeight) / ((double)imgHeight);
    double scaleFactor = scaleFactorX;
    if (scaleFactorX > scaleFactorY)
        scaleFactor = scaleFactorY;

    // Now get the resized values
    gint scaledImgWidth = (int)(scaleFactor * (double)imgWidth);
    gint scaledImgHeight = (int)(scaleFactor * (double)imgHeight);

    // center the image on the area
    gint imgX = (previewWidth - scaledImgWidth) / 2;
    gint imgY = (previewHeight - scaledImgHeight) / 2;

    // wrap a rectangle around the image
    gint rectX = imgX - 1;
    gint rectY = imgY - 1;
    gint rectWidth = scaledImgWidth + 2;
    gint rectHeight = scaledImgHeight + 2;

    // Our template.  Modify to taste
    gchar const *xformat = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                           "<svg\n"
                           "xmlns=\"http://www.w3.org/2000/svg\"\n"
                           "xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
                           "width=\"%d\" height=\"%d\">\n" //# VALUES HERE
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
                           "    font-family:Sans\"\n"
                           "  x=\"10\" y=\"26\">%d x %d</text>\n" //# VALUES HERE
                           "</svg>\n\n";

    // if (!Glib::get_charset()) //If we are not utf8
    fileName = Glib::filename_to_utf8(fileName);

    // Fill in the template
    /* FIXME: Do proper XML quoting for fileName. */
    gchar *xmlBuffer =
        g_strdup_printf(xformat, previewWidth, previewHeight, imgX, imgY, scaledImgWidth, scaledImgHeight,
                        fileName.c_str(), rectX, rectY, rectWidth, rectHeight, imgWidth, imgHeight);

    // g_message("%s\n", xmlBuffer);

    // now show it!
    setFromMem(xmlBuffer);
    g_free(xmlBuffer);
}



void SVGPreview::showNoPreview()
{
    // Are we already showing it?
    if (showingNoPreview)
        return;

    // Arbitrary size of svg doc -- rather 'portrait' shaped
    gint previewWidth = 300;
    gint previewHeight = 600;

    // Our template.  Modify to taste
    gchar const *xformat =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<svg\n"
        "xmlns=\"http://www.w3.org/2000/svg\"\n"
        "xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
        "width=\"%d\" height=\"%d\">\n" //# VALUES HERE
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
        "font-family:Sans;text-anchor:middle;writing-mode:lr\"\n"
        "x=\"190\" y=\"240\">%s</text></g>\n" //# VALUE HERE
        "</svg>\n\n";

    // Fill in the template
    gchar *xmlBuffer = g_strdup_printf(xformat, previewWidth, previewHeight, _("No preview"));

    // g_message("%s\n", xmlBuffer);

    // now show it!
    setFromMem(xmlBuffer);
    g_free(xmlBuffer);
    showingNoPreview = true;
}


/**
 * Inform the user that the svg file is too large to be displayed.
 * This does not check for sizes of embedded images (yet)
 */
void SVGPreview::showTooLarge(long fileLength)
{

    // Arbitrary size of svg doc -- rather 'portrait' shaped
    gint previewWidth = 300;
    gint previewHeight = 600;

    // Our template.  Modify to taste
    gchar const *xformat =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<svg\n"
        "xmlns=\"http://www.w3.org/2000/svg\"\n"
        "xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
        "width=\"%d\" height=\"%d\">\n" //# VALUES HERE
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
        "font-family:Sans;text-anchor:middle;writing-mode:lr\"\n"
        "x=\"170\" y=\"215\">%5.1f MB</text>\n" //# VALUE HERE
        "<text xml:space=\"preserve\"\n"
        "style=\"font-size:24.000000;font-style:normal;font-variant:normal;font-weight:bold;"
        "font-stretch:normal;fill:#000000;fill-opacity:1.0000000;stroke:none;stroke-width:1.0000000pt;"
        "stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1.0000000;"
        "font-family:Sans;text-anchor:middle;writing-mode:lr\"\n"
        "x=\"180\" y=\"245\">%s</text>\n" //# VALUE HERE
        "</svg>\n\n";

    // Fill in the template
    double floatFileLength = ((double)fileLength) / 1048576.0;
    // printf("%ld %f\n", fileLength, floatFileLength);
    gchar *xmlBuffer =
        g_strdup_printf(xformat, previewWidth, previewHeight, floatFileLength, _("too large for preview"));

    // g_message("%s\n", xmlBuffer);

    // now show it!
    setFromMem(xmlBuffer);
    g_free(xmlBuffer);
}

bool SVGPreview::set(Glib::ustring &fileName, int dialogType)
{

    if (!Glib::file_test(fileName, Glib::FILE_TEST_EXISTS)) {
        showNoPreview();
        return false;
    }


    if (Glib::file_test(fileName, Glib::FILE_TEST_IS_DIR)) {
        showNoPreview();
        return false;
    }

    if (Glib::file_test(fileName, Glib::FILE_TEST_IS_REGULAR)) {
        Glib::ustring fileNameUtf8 = Glib::filename_to_utf8(fileName);
        gchar *fName = const_cast<gchar *>(
            fileNameUtf8.c_str()); // const-cast probably not necessary? (not necessary on Windows version of stat())
        struct stat info;
        if (g_stat(fName, &info)) // stat returns 0 upon success
        {
            g_warning("SVGPreview::set() : %s : %s", fName, strerror(errno));
            return false;
        }
        if (info.st_size > 0xA00000L) {
            showingNoPreview = false;
            showTooLarge(info.st_size);
            return false;
        }
    }

    Glib::ustring svg = ".svg";
    Glib::ustring svgz = ".svgz";

    if ((dialogType == SVG_TYPES || dialogType == IMPORT_TYPES) &&
        (hasSuffix(fileName, svg) || hasSuffix(fileName, svgz))) {
        bool retval = setFileName(fileName);
        showingNoPreview = false;
        return retval;
    } else if (isValidImageFile(fileName)) {
        showImage(fileName);
        showingNoPreview = false;
        return true;
    } else {
        showNoPreview();
        return false;
    }
}


SVGPreview::SVGPreview()
{
    if (!INKSCAPE)
        inkscape_application_init("", false);
    document = NULL;
    viewerGtk = NULL;
    set_size_request(150, 150);
    showingNoPreview = false;
}

SVGPreview::~SVGPreview()
{
}


/*#########################################################################
### F I L E     D I A L O G    B A S E    C L A S S
#########################################################################*/

void FileDialogBaseGtk::internalSetup()
{
    // Open executable file dialogs don't need the preview panel
    if (_dialogType != EXE_TYPES) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        bool enablePreview = prefs->getBool(preferenceBase + "/enable_preview", true);

        previewCheckbox.set_label(Glib::ustring(_("Enable preview")));
        previewCheckbox.set_active(enablePreview);

        previewCheckbox.signal_toggled().connect(sigc::mem_fun(*this, &FileDialogBaseGtk::_previewEnabledCB));

        // Catch selection-changed events, so we can adjust the text widget
        signal_update_preview().connect(sigc::mem_fun(*this, &FileDialogBaseGtk::_updatePreviewCallback));

        //###### Add a preview widget
        set_preview_widget(svgPreview);
        set_preview_widget_active(enablePreview);
        set_use_preview_label(false);
    }
}


void FileDialogBaseGtk::cleanup(bool showConfirmed)
{
    if (_dialogType != EXE_TYPES) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        if (showConfirmed) {
            prefs->setBool(preferenceBase + "/enable_preview", previewCheckbox.get_active());
        }
    }
}


void FileDialogBaseGtk::_previewEnabledCB()
{
    bool enabled = previewCheckbox.get_active();
    set_preview_widget_active(enabled);
    if (enabled) {
        _updatePreviewCallback();
    } else {
        // Clears out any current preview image.
        svgPreview.showNoPreview();
    }
}



/**
 * Callback for checking if the preview needs to be redrawn
 */
void FileDialogBaseGtk::_updatePreviewCallback()
{
    Glib::ustring fileName = get_preview_filename();
    bool enabled = previewCheckbox.get_active();

#ifdef WITH_GNOME_VFS
    if (fileName.empty() && gnome_vfs_initialized()) {
        fileName = get_preview_uri();
    }
#endif

    if (enabled && !fileName.empty()) {
        svgPreview.set(fileName, _dialogType);
    } else {
        svgPreview.showNoPreview();
    }
}


/*#########################################################################
### F I L E    O P E N
#########################################################################*/

/**
 * Constructor.  Not called directly.  Use the factory.
 */
FileOpenDialogImplGtk::FileOpenDialogImplGtk(Gtk::Window &parentWindow, const Glib::ustring &dir,
                                             FileDialogType fileTypes, const Glib::ustring &title)
    : FileDialogBaseGtk(parentWindow, title, Gtk::FILE_CHOOSER_ACTION_OPEN, fileTypes, "/dialogs/open")
{


    if (_dialogType == EXE_TYPES) {
        /* One file at a time */
        set_select_multiple(false);
    } else {
        /* And also Multiple Files */
        set_select_multiple(true);
    }

#ifdef WITH_GNOME_VFS
    if (gnome_vfs_initialized()) {
        set_local_only(false);
    }
#endif

    /* Initalize to Autodetect */
    extension = NULL;
    /* No filename to start out with */
    myFilename = "";

    /* Set our dialog type (open, import, etc...)*/
    _dialogType = fileTypes;


    /* Set the pwd and/or the filename */
    if (dir.size() > 0) {
        Glib::ustring udir(dir);
        Glib::ustring::size_type len = udir.length();
        // leaving a trailing backslash on the directory name leads to the infamous
        // double-directory bug on win32
        if (len != 0 && udir[len - 1] == '\\')
            udir.erase(len - 1);
        if (_dialogType == EXE_TYPES) {
            set_filename(udir.c_str());
        } else {
            set_current_folder(udir.c_str());
        }
    }

    if (_dialogType != EXE_TYPES) {
        set_extra_widget(previewCheckbox);
    }

    //###### Add the file types menu
    createFilterMenu();

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    set_default(*add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK));

    //###### Allow easy access to our examples folder
    if (Inkscape::IO::file_test(INKSCAPE_EXAMPLESDIR, G_FILE_TEST_EXISTS) &&
        Inkscape::IO::file_test(INKSCAPE_EXAMPLESDIR, G_FILE_TEST_IS_DIR) && g_path_is_absolute(INKSCAPE_EXAMPLESDIR)) {
        add_shortcut_folder(INKSCAPE_EXAMPLESDIR);
    }
}

/**
 * Destructor
 */
FileOpenDialogImplGtk::~FileOpenDialogImplGtk()
{
}

void FileOpenDialogImplGtk::addFilterMenu(Glib::ustring name, Glib::ustring pattern)
{

#if WITH_GTKMM_3_0
    Glib::RefPtr<Gtk::FileFilter> allFilter = Gtk::FileFilter::create();
    allFilter->set_name(_(name.c_str()));
    allFilter->add_pattern(pattern);
#else
    Gtk::FileFilter allFilter;
    allFilter.set_name(_(name.c_str()));
    allFilter.add_pattern(pattern);
#endif
    extensionMap[Glib::ustring(_("All Files"))] = NULL;
    add_filter(allFilter);
}

void FileOpenDialogImplGtk::createFilterMenu()
{
    if (_dialogType == CUSTOM_TYPE) {
        return;
    }

    if (_dialogType == EXE_TYPES) {
#if WITH_GTKMM_3_0
        Glib::RefPtr<Gtk::FileFilter> allFilter = Gtk::FileFilter::create();
        allFilter->set_name(_("All Files"));
        allFilter->add_pattern("*");
#else
        Gtk::FileFilter allFilter;
        allFilter.set_name(_("All Files"));
        allFilter.add_pattern("*");
#endif
        extensionMap[Glib::ustring(_("All Files"))] = NULL;
        add_filter(allFilter);
    } else {
#if WITH_GTKMM_3_0
        Glib::RefPtr<Gtk::FileFilter> allInkscapeFilter = Gtk::FileFilter::create();
        allInkscapeFilter->set_name(_("All Inkscape Files"));

        Glib::RefPtr<Gtk::FileFilter> allFilter = Gtk::FileFilter::create();
        allFilter->set_name(_("All Files"));
        allFilter->add_pattern("*");

        Glib::RefPtr<Gtk::FileFilter> allImageFilter = Gtk::FileFilter::create();
        allImageFilter->set_name(_("All Images"));

        Glib::RefPtr<Gtk::FileFilter> allVectorFilter = Gtk::FileFilter::create();
        allVectorFilter->set_name(_("All Vectors"));

        Glib::RefPtr<Gtk::FileFilter> allBitmapFilter = Gtk::FileFilter::create();
        allBitmapFilter->set_name(_("All Bitmaps"));
#else
        Gtk::FileFilter allInkscapeFilter;
        allInkscapeFilter.set_name(_("All Inkscape Files"));

        Gtk::FileFilter allFilter;
        allFilter.set_name(_("All Files"));
        allFilter.add_pattern("*");

        Gtk::FileFilter allImageFilter;
        allImageFilter.set_name(_("All Images"));

        Gtk::FileFilter allVectorFilter;
        allVectorFilter.set_name(_("All Vectors"));

        Gtk::FileFilter allBitmapFilter;
        allBitmapFilter.set_name(_("All Bitmaps"));
#endif
        extensionMap[Glib::ustring(_("All Inkscape Files"))] = NULL;
        add_filter(allInkscapeFilter);

        extensionMap[Glib::ustring(_("All Files"))] = NULL;
        add_filter(allFilter);

        extensionMap[Glib::ustring(_("All Images"))] = NULL;
        add_filter(allImageFilter);

        extensionMap[Glib::ustring(_("All Vectors"))] = NULL;
        add_filter(allVectorFilter);

        extensionMap[Glib::ustring(_("All Bitmaps"))] = NULL;
        add_filter(allBitmapFilter);

        // patterns added dynamically below
        Inkscape::Extension::DB::InputList extension_list;
        Inkscape::Extension::db.get_input_list(extension_list);

        for (Inkscape::Extension::DB::InputList::iterator current_item = extension_list.begin();
             current_item != extension_list.end(); ++current_item)
        {
            Inkscape::Extension::Input *imod = *current_item;

            // FIXME: would be nice to grey them out instead of not listing them
            if (imod->deactivated())
                continue;

            Glib::ustring upattern("*");
            Glib::ustring extension = imod->get_extension();
            fileDialogExtensionToPattern(upattern, extension);

            Glib::ustring uname(_(imod->get_filetypename()));

#if WITH_GTKMM_3_0
            Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
            filter->set_name(uname);
            filter->add_pattern(upattern);
#else
            Gtk::FileFilter filter;
            filter.set_name(uname);
            filter.add_pattern(upattern);
#endif

            add_filter(filter);
            extensionMap[uname] = imod;

// g_message("ext %s:%s '%s'\n", ioext->name, ioext->mimetype, upattern.c_str());
#if WITH_GTKMM_3_0
            allInkscapeFilter->add_pattern(upattern);
            if (strncmp("image", imod->get_mimetype(), 5) == 0)
                allImageFilter->add_pattern(upattern);
#else
            allInkscapeFilter.add_pattern(upattern);
            if (strncmp("image", imod->get_mimetype(), 5) == 0)
                allImageFilter.add_pattern(upattern);
#endif

            // uncomment this to find out all mime types supported by Inkscape import/open
            // g_print ("%s\n", imod->get_mimetype());

            // I don't know of any other way to define "bitmap" formats other than by listing them
            if (strncmp("image/png", imod->get_mimetype(), 9) == 0 ||
                strncmp("image/jpeg", imod->get_mimetype(), 10) == 0 ||
                strncmp("image/gif", imod->get_mimetype(), 9) == 0 ||
                strncmp("image/x-icon", imod->get_mimetype(), 12) == 0 ||
                strncmp("image/x-navi-animation", imod->get_mimetype(), 22) == 0 ||
                strncmp("image/x-cmu-raster", imod->get_mimetype(), 18) == 0 ||
                strncmp("image/x-xpixmap", imod->get_mimetype(), 15) == 0 ||
                strncmp("image/bmp", imod->get_mimetype(), 9) == 0 ||
                strncmp("image/vnd.wap.wbmp", imod->get_mimetype(), 18) == 0 ||
                strncmp("image/tiff", imod->get_mimetype(), 10) == 0 ||
                strncmp("image/x-xbitmap", imod->get_mimetype(), 15) == 0 ||
                strncmp("image/x-tga", imod->get_mimetype(), 11) == 0 ||
                strncmp("image/x-pcx", imod->get_mimetype(), 11) == 0)
            {
#if WITH_GTKMM_3_0
                allBitmapFilter->add_pattern(upattern);
#else
                allBitmapFilter.add_pattern(upattern);
#endif
             } else {
#if WITH_GTKMM_3_0
                allVectorFilter->add_pattern(upattern);
#else
                allVectorFilter.add_pattern(upattern);
#endif
            }
        }
    }
    return;
}

/**
 * Show this dialog modally.  Return true if user hits [OK]
 */
bool FileOpenDialogImplGtk::show()
{
    set_modal(TRUE); // Window
    sp_transientize(GTK_WIDGET(gobj())); // Make transient
    gint b = run(); // Dialog
    svgPreview.showNoPreview();
    hide();

    if (b == Gtk::RESPONSE_OK) {
        // This is a hack, to avoid the warning messages that
        // Gtk::FileChooser::get_filter() returns
        // should be:  Gtk::FileFilter *filter = get_filter();
        GtkFileChooser *gtkFileChooser = Gtk::FileChooser::gobj();
        GtkFileFilter *filter = gtk_file_chooser_get_filter(gtkFileChooser);
        if (filter) {
            // Get which extension was chosen, if any
            extension = extensionMap[gtk_file_filter_get_name(filter)];
        }
        myFilename = get_filename();
#ifdef WITH_GNOME_VFS
        if (myFilename.empty() && gnome_vfs_initialized())
            myFilename = get_uri();
#endif
        cleanup(true);
        return true;
    } else {
        cleanup(false);
        return false;
    }
}



/**
 * Get the file extension type that was selected by the user. Valid after an [OK]
 */
Inkscape::Extension::Extension *FileOpenDialogImplGtk::getSelectionType()
{
    return extension;
}


/**
 * Get the file name chosen by the user.   Valid after an [OK]
 */
Glib::ustring FileOpenDialogImplGtk::getFilename(void)
{
    return myFilename;
}


/**
 * To Get Multiple filenames selected at-once.
 */
std::vector<Glib::ustring> FileOpenDialogImplGtk::getFilenames()
{
#if WITH_GTKMM_3_0
    std::vector<std::string> result_tmp = get_filenames();

    // Copy filenames to a vector of type Glib::ustring
    std::vector<Glib::ustring> result;

    for (std::vector<std::string>::iterator it = result_tmp.begin(); it != result_tmp.end(); ++it)
        result.push_back(*it);

#else
    std::vector<Glib::ustring> result = get_filenames();
#endif

#ifdef WITH_GNOME_VFS
    if (result.empty() && gnome_vfs_initialized())
        result = get_uris();
#endif
    return result;
}

Glib::ustring FileOpenDialogImplGtk::getCurrentDirectory()
{
    return get_current_folder();
}



//########################################################################
//# F I L E    S A V E
//########################################################################

/**
 * Constructor
 */
FileSaveDialogImplGtk::FileSaveDialogImplGtk(Gtk::Window &parentWindow, const Glib::ustring &dir,
                                             FileDialogType fileTypes, const Glib::ustring &title,
                                             const Glib::ustring & /*default_key*/, const gchar *docTitle,
                                             const Inkscape::Extension::FileSaveMethod save_method)
    : FileDialogBaseGtk(parentWindow, title, Gtk::FILE_CHOOSER_ACTION_SAVE, fileTypes,
                        (save_method == Inkscape::Extension::FILE_SAVE_METHOD_SAVE_COPY) ? "/dialogs/save_copy"
                                                                                         : "/dialogs/save_as")
    , save_method(save_method)
{
    FileSaveDialog::myDocTitle = docTitle;

    /* One file at a time */
    set_select_multiple(false);

#ifdef WITH_GNOME_VFS
    if (gnome_vfs_initialized()) {
        set_local_only(false);
    }
#endif

    /* Initalize to Autodetect */
    extension = NULL;
    /* No filename to start out with */
    myFilename = "";

    /* Set our dialog type (save, export, etc...)*/
    _dialogType = fileTypes;

    /* Set the pwd and/or the filename */
    if (dir.size() > 0) {
        Glib::ustring udir(dir);
        Glib::ustring::size_type len = udir.length();
        // leaving a trailing backslash on the directory name leads to the infamous
        // double-directory bug on win32
        if ((len != 0) && (udir[len - 1] == '\\')) {
            udir.erase(len - 1);
        }
        myFilename = udir;
    }

    //###### Add the file types menu
    // createFilterMenu();

    //###### Do we want the .xxx extension automatically added?
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    fileTypeCheckbox.set_label(Glib::ustring(_("Append filename extension automatically")));
    if (save_method == Inkscape::Extension::FILE_SAVE_METHOD_SAVE_COPY) {
        fileTypeCheckbox.set_active(prefs->getBool("/dialogs/save_copy/append_extension", true));
    } else {
        fileTypeCheckbox.set_active(prefs->getBool("/dialogs/save_as/append_extension", true));
    }

    if (_dialogType != CUSTOM_TYPE)
        createFileTypeMenu();

    fileTypeComboBox.set_size_request(200, 40);
    fileTypeComboBox.signal_changed().connect(sigc::mem_fun(*this, &FileSaveDialogImplGtk::fileTypeChangedCallback));


    childBox.pack_start(checksBox);
    childBox.pack_end(fileTypeComboBox);
    checksBox.pack_start(fileTypeCheckbox);
    checksBox.pack_start(previewCheckbox);

    set_extra_widget(childBox);

    // Let's do some customization
    fileNameEntry = NULL;
    Gtk::Container *cont = get_toplevel();
    std::vector<Gtk::Entry *> entries;
    findEntryWidgets(cont, entries);
    // g_message("Found %d entry widgets\n", entries.size());
    if (!entries.empty()) {
        // Catch when user hits [return] on the text field
        fileNameEntry = entries[0];
        fileNameEntry->signal_activate().connect(
            sigc::mem_fun(*this, &FileSaveDialogImplGtk::fileNameEntryChangedCallback));
    }

    // Let's do more customization
    std::vector<Gtk::Expander *> expanders;
    findExpanderWidgets(cont, expanders);
    // g_message("Found %d expander widgets\n", expanders.size());
    if (!expanders.empty()) {
        // Always show the file list
        Gtk::Expander *expander = expanders[0];
        expander->set_expanded(true);
    }

    // allow easy access to the user's own templates folder
    gchar *templates = profile_path("templates");
    if (Inkscape::IO::file_test(templates, G_FILE_TEST_EXISTS) &&
        Inkscape::IO::file_test(templates, G_FILE_TEST_IS_DIR) && g_path_is_absolute(templates)) {
        add_shortcut_folder(templates);
    }
    g_free(templates);


    // if (extension == NULL)
    //    checkbox.set_sensitive(FALSE);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    set_default(*add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK));

    show_all_children();
}

/**
 * Destructor
 */
FileSaveDialogImplGtk::~FileSaveDialogImplGtk()
{
}

/**
 * Callback for fileNameEntry widget
 */
void FileSaveDialogImplGtk::fileNameEntryChangedCallback()
{
    if (!fileNameEntry)
        return;

    Glib::ustring fileName = fileNameEntry->get_text();
    if (!Glib::get_charset()) // If we are not utf8
        fileName = Glib::filename_to_utf8(fileName);

    // g_message("User hit return.  Text is '%s'\n", fileName.c_str());

    if (!Glib::path_is_absolute(fileName)) {
        // try appending to the current path
        // not this way: fileName = get_current_folder() + "/" + fileName;
        std::vector<Glib::ustring> pathSegments;
        pathSegments.push_back(get_current_folder());
        pathSegments.push_back(fileName);
        fileName = Glib::build_filename(pathSegments);
    }

    // g_message("path:'%s'\n", fileName.c_str());

    if (Glib::file_test(fileName, Glib::FILE_TEST_IS_DIR)) {
        set_current_folder(fileName);
    } else if (/*Glib::file_test(fileName, Glib::FILE_TEST_IS_REGULAR)*/ 1) {
        // dialog with either (1) select a regular file or (2) cd to dir
        // simulate an 'OK'
        set_filename(fileName);
        response(Gtk::RESPONSE_OK);
    }
}



/**
 * Callback for fileNameEntry widget
 */
void FileSaveDialogImplGtk::fileTypeChangedCallback()
{
    int sel = fileTypeComboBox.get_active_row_number();
    if ((sel < 0) || (sel >= (int)fileTypes.size()))
        return;

    FileType type = fileTypes[sel];
    // g_message("selected: %s\n", type.name.c_str());

    extension = type.extension;
#if WITH_GTKMM_3_0
    Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
    filter->add_pattern(type.pattern);
#else
    Gtk::FileFilter filter;
    filter.add_pattern(type.pattern);
#endif
    set_filter(filter);

    updateNameAndExtension();
}

void FileSaveDialogImplGtk::addFileType(Glib::ustring name, Glib::ustring pattern)
{
    //#Let user choose
    FileType guessType;
    guessType.name = name;
    guessType.pattern = pattern;
    guessType.extension = NULL;
    fileTypeComboBox.append(guessType.name);
    fileTypes.push_back(guessType);


    fileTypeComboBox.set_active(0);
    fileTypeChangedCallback(); // call at least once to set the filter
}

void FileSaveDialogImplGtk::createFileTypeMenu()
{
    Inkscape::Extension::DB::OutputList extension_list;
    Inkscape::Extension::db.get_output_list(extension_list);
    knownExtensions.clear();

    for (Inkscape::Extension::DB::OutputList::iterator current_item = extension_list.begin();
         current_item != extension_list.end(); ++current_item) {
        Inkscape::Extension::Output *omod = *current_item;

        // FIXME: would be nice to grey them out instead of not listing them
        if (omod->deactivated())
            continue;

        FileType type;
        type.name = (_(omod->get_filetypename()));
        type.pattern = "*";
        Glib::ustring extension = omod->get_extension();
        knownExtensions.insert(extension.casefold());
        fileDialogExtensionToPattern(type.pattern, extension);
        type.extension = omod;
        fileTypeComboBox.append(type.name);
        fileTypes.push_back(type);
    }

    //#Let user choose
    FileType guessType;
    guessType.name = _("Guess from extension");
    guessType.pattern = "*";
    guessType.extension = NULL;
    fileTypeComboBox.append(guessType.name);
    fileTypes.push_back(guessType);


    fileTypeComboBox.set_active(0);
    fileTypeChangedCallback(); // call at least once to set the filter
}



/**
 * Show this dialog modally.  Return true if user hits [OK]
 */
bool FileSaveDialogImplGtk::show()
{
    change_path(myFilename);
    set_modal(TRUE); // Window
    sp_transientize(GTK_WIDGET(gobj())); // Make transient
    gint b = run(); // Dialog
    svgPreview.showNoPreview();
    set_preview_widget_active(false);
    hide();

    if (b == Gtk::RESPONSE_OK) {
        updateNameAndExtension();
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();

        // Store changes of the "Append filename automatically" checkbox back to preferences.
        if (save_method == Inkscape::Extension::FILE_SAVE_METHOD_SAVE_COPY) {
            prefs->setBool("/dialogs/save_copy/append_extension", fileTypeCheckbox.get_active());
        } else {
            prefs->setBool("/dialogs/save_as/append_extension", fileTypeCheckbox.get_active());
        }

        Inkscape::Extension::store_file_extension_in_prefs((extension != NULL ? extension->get_id() : ""), save_method);

        cleanup(true);

        return true;
    } else {
        cleanup(false);
        return false;
    }
}


/**
 * Get the file extension type that was selected by the user. Valid after an [OK]
 */
Inkscape::Extension::Extension *FileSaveDialogImplGtk::getSelectionType()
{
    return extension;
}

void FileSaveDialogImplGtk::setSelectionType(Inkscape::Extension::Extension *key)
{
    // If no pointer to extension is passed in, look up based on filename extension.
    if (!key) {
        // Not quite UTF-8 here.
        gchar *filenameLower = g_ascii_strdown(myFilename.c_str(), -1);
        for (int i = 0; !key && (i < (int)fileTypes.size()); i++) {
            Inkscape::Extension::Output *ext = dynamic_cast<Inkscape::Extension::Output *>(fileTypes[i].extension);
            if (ext && ext->get_extension()) {
                gchar *extensionLower = g_ascii_strdown(ext->get_extension(), -1);
                if (g_str_has_suffix(filenameLower, extensionLower)) {
                    key = fileTypes[i].extension;
                }
                g_free(extensionLower);
            }
        }
        g_free(filenameLower);
    }

    // Ensure the proper entry in the combo box is selected.
    if (key) {
        extension = key;
        gchar const *extensionID = extension->get_id();
        if (extensionID) {
            for (int i = 0; i < (int)fileTypes.size(); i++) {
                Inkscape::Extension::Extension *ext = fileTypes[i].extension;
                if (ext) {
                    gchar const *id = ext->get_id();
                    if (id && (strcmp(extensionID, id) == 0)) {
                        int oldSel = fileTypeComboBox.get_active_row_number();
                        if (i != oldSel) {
                            fileTypeComboBox.set_active(i);
                        }
                        break;
                    }
                }
            }
        }
    }
}

Glib::ustring FileSaveDialogImplGtk::getCurrentDirectory()
{
    return get_current_folder();
}


/*void
FileSaveDialogImplGtk::change_title(const Glib::ustring& title)
{
    set_title(title);
}*/

/**
  * Change the default save path location.
  */
void FileSaveDialogImplGtk::change_path(const Glib::ustring &path)
{
    myFilename = path;

    if (Glib::file_test(myFilename, Glib::FILE_TEST_IS_DIR)) {
        // fprintf(stderr,"set_current_folder(%s)\n",myFilename.c_str());
        set_current_folder(myFilename);
    } else {
        // fprintf(stderr,"set_filename(%s)\n",myFilename.c_str());
        if (Glib::file_test(myFilename, Glib::FILE_TEST_EXISTS)) {
            set_filename(myFilename);
        } else {
            std::string dirName = Glib::path_get_dirname(myFilename);
            if (dirName != get_current_folder()) {
                set_current_folder(dirName);
            }
        }
        Glib::ustring basename = Glib::path_get_basename(myFilename);
        // fprintf(stderr,"set_current_name(%s)\n",basename.c_str());
        try
        {
            set_current_name(Glib::filename_to_utf8(basename));
        }
        catch (Glib::ConvertError &e)
        {
            g_warning("Error converting save filename to UTF-8.");
            // try a fallback.
            set_current_name(basename);
        }
    }
}

void FileSaveDialogImplGtk::updateNameAndExtension()
{
    // Pick up any changes the user has typed in.
    Glib::ustring tmp = get_filename();
#ifdef WITH_GNOME_VFS
    if (tmp.empty() && gnome_vfs_initialized()) {
        tmp = get_uri();
    }
#endif
    if (!tmp.empty()) {
        myFilename = tmp;
    }

    Inkscape::Extension::Output *newOut = extension ? dynamic_cast<Inkscape::Extension::Output *>(extension) : 0;
    if (fileTypeCheckbox.get_active() && newOut) {
        // Append the file extension if it's not already present and display it in the file name entry field
        appendExtension(myFilename, newOut);
        change_path(myFilename);
    }
}


#ifdef NEW_EXPORT_DIALOG

//########################################################################
//# F I L E     E X P O R T
//########################################################################

/**
 * Callback for fileNameEntry widget
 */
void FileExportDialogImpl::fileNameEntryChangedCallback()
{
    if (!fileNameEntry)
        return;

    Glib::ustring fileName = fileNameEntry->get_text();
    if (!Glib::get_charset()) // If we are not utf8
        fileName = Glib::filename_to_utf8(fileName);

    // g_message("User hit return.  Text is '%s'\n", fileName.c_str());

    if (!Glib::path_is_absolute(fileName)) {
        // try appending to the current path
        // not this way: fileName = get_current_folder() + "/" + fileName;
        std::vector<Glib::ustring> pathSegments;
        pathSegments.push_back(get_current_folder());
        pathSegments.push_back(fileName);
        fileName = Glib::build_filename(pathSegments);
    }

    // g_message("path:'%s'\n", fileName.c_str());

    if (Glib::file_test(fileName, Glib::FILE_TEST_IS_DIR)) {
        set_current_folder(fileName);
    } else if (/*Glib::file_test(fileName, Glib::FILE_TEST_IS_REGULAR)*/ 1) {
        // dialog with either (1) select a regular file or (2) cd to dir
        // simulate an 'OK'
        set_filename(fileName);
        response(Gtk::RESPONSE_OK);
    }
}



/**
 * Callback for fileNameEntry widget
 */
void FileExportDialogImpl::fileTypeChangedCallback()
{
    int sel = fileTypeComboBox.get_active_row_number();
    if ((sel < 0) || (sel >= (int)fileTypes.size()))
        return;

    FileType type = fileTypes[sel];
    // g_message("selected: %s\n", type.name.c_str());
    Gtk::FileFilter filter;
    filter.add_pattern(type.pattern);
    set_filter(filter);
}



void FileExportDialogImpl::createFileTypeMenu()
{
    Inkscape::Extension::DB::OutputList extension_list;
    Inkscape::Extension::db.get_output_list(extension_list);

    for (Inkscape::Extension::DB::OutputList::iterator current_item = extension_list.begin();
         current_item != extension_list.end(); ++current_item) {
        Inkscape::Extension::Output *omod = *current_item;

        // FIXME: would be nice to grey them out instead of not listing them
        if (omod->deactivated())
            continue;

        FileType type;
        type.name = (_(omod->get_filetypename()));
        type.pattern = "*";
        Glib::ustring extension = omod->get_extension();
        fileDialogExtensionToPattern(type.pattern, extension);
        type.extension = omod;
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
    fileTypeChangedCallback(); // call at least once to set the filter
}


/**
 * Constructor
 */
FileExportDialogImpl::FileExportDialogImpl(Gtk::Window &parentWindow, const Glib::ustring &dir,
                                           FileDialogType fileTypes, const Glib::ustring &title,
                                           const Glib::ustring & /*default_key*/)
    : FileDialogBaseGtk(parentWindow, title, Gtk::FILE_CHOOSER_ACTION_SAVE, fileTypes, "/dialogs/export")
    , sourceX0Spinner("X0", _("Left edge of source"))
    , sourceY0Spinner("Y0", _("Top edge of source"))
    , sourceX1Spinner("X1", _("Right edge of source"))
    , sourceY1Spinner("Y1", _("Bottom edge of source"))
    , sourceWidthSpinner("Width", _("Source width"))
    , sourceHeightSpinner("Height", _("Source height"))
    , destWidthSpinner("Width", _("Destination width"))
    , destHeightSpinner("Height", _("Destination height"))
    , destDPISpinner("DPI", _("Resolution (dots per inch)"))
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    append_extension = prefs->getBool("/dialogs/save_export/append_extension", true);

    /* One file at a time */
    set_select_multiple(false);

#ifdef WITH_GNOME_VFS
    if (gnome_vfs_initialized()) {
        set_local_only(false);
    }
#endif

    /* Initalize to Autodetect */
    extension = NULL;
    /* No filename to start out with */
    myFilename = "";

    /* Set our dialog type (save, export, etc...)*/
    _dialogType = fileTypes;

    /* Set the pwd and/or the filename */
    if (dir.size() > 0) {
        Glib::ustring udir(dir);
        Glib::ustring::size_type len = udir.length();
        // leaving a trailing backslash on the directory name leads to the infamous
        // double-directory bug on win32
        if ((len != 0) && (udir[len - 1] == '\\'))
            udir.erase(len - 1);
        set_current_folder(udir.c_str());
    }

    //#########################################
    //## EXTRA WIDGET -- SOURCE SIDE
    //#########################################

    //##### Export options buttons/spinners, etc
    documentButton.set_label(_("Document"));
    scopeBox.pack_start(documentButton);
    scopeGroup = documentButton.get_group();

    pageButton.set_label(_("Page"));
    pageButton.set_group(scopeGroup);
    scopeBox.pack_start(pageButton);

    selectionButton.set_label(_("Selection"));
    selectionButton.set_group(scopeGroup);
    scopeBox.pack_start(selectionButton);

    customButton.set_label(C_("Export dialog", "Custom"));
    customButton.set_group(scopeGroup);
    scopeBox.pack_start(customButton);

    sourceBox.pack_start(scopeBox);



    // dimension buttons
    sourceTable.resize(3, 3);
    sourceTable.attach(sourceX0Spinner, 0, 1, 0, 1);
    sourceTable.attach(sourceY0Spinner, 1, 2, 0, 1);
    sourceUnitsSpinner.setUnitType(UNIT_TYPE_LINEAR);
    sourceTable.attach(sourceUnitsSpinner, 2, 3, 0, 1);
    sourceTable.attach(sourceX1Spinner, 0, 1, 1, 2);
    sourceTable.attach(sourceY1Spinner, 1, 2, 1, 2);
    sourceTable.attach(sourceWidthSpinner, 0, 1, 2, 3);
    sourceTable.attach(sourceHeightSpinner, 1, 2, 2, 3);

    sourceBox.pack_start(sourceTable);
    sourceFrame.set_label(_("Source"));
    sourceFrame.add(sourceBox);
    exportOptionsBox.pack_start(sourceFrame);


    //#########################################
    //## EXTRA WIDGET -- SOURCE SIDE
    //#########################################


    destTable.resize(3, 3);
    destTable.attach(destWidthSpinner, 0, 1, 0, 1);
    destTable.attach(destHeightSpinner, 1, 2, 0, 1);
    destUnitsSpinner.setUnitType(UNIT_TYPE_LINEAR);
    destTable.attach(destUnitsSpinner, 2, 3, 0, 1);
    destTable.attach(destDPISpinner, 0, 1, 1, 2);

    destBox.pack_start(destTable);


    cairoButton.set_label(_("Cairo"));
    otherOptionBox.pack_start(cairoButton);

    antiAliasButton.set_label(_("Antialias"));
    otherOptionBox.pack_start(antiAliasButton);

    backgroundButton.set_label(_("Background"));
    otherOptionBox.pack_start(backgroundButton);

    destBox.pack_start(otherOptionBox);



    //###### File options
    //###### Do we want the .xxx extension automatically added?
    fileTypeCheckbox.set_label(Glib::ustring(_("Append filename extension automatically")));
    fileTypeCheckbox.set_active(append_extension);
    destBox.pack_start(fileTypeCheckbox);

    //###### File type menu
    createFileTypeMenu();
    fileTypeComboBox.set_size_request(200, 40);
    fileTypeComboBox.signal_changed().connect(sigc::mem_fun(*this, &FileExportDialogImpl::fileTypeChangedCallback));

    destBox.pack_start(fileTypeComboBox);

    destFrame.set_label(_("Destination"));
    destFrame.add(destBox);
    exportOptionsBox.pack_start(destFrame);

    //##### Put the two boxes and their parent onto the dialog
    exportOptionsBox.pack_start(sourceFrame);
    exportOptionsBox.pack_start(destFrame);

    set_extra_widget(exportOptionsBox);



    // Let's do some customization
    fileNameEntry = NULL;
    Gtk::Container *cont = get_toplevel();
    std::vector<Gtk::Entry *> entries;
    findEntryWidgets(cont, entries);
    // g_message("Found %d entry widgets\n", entries.size());
    if (!entries.empty()) {
        // Catch when user hits [return] on the text field
        fileNameEntry = entries[0];
        fileNameEntry->signal_activate().connect(
            sigc::mem_fun(*this, &FileExportDialogImpl::fileNameEntryChangedCallback));
    }

    // Let's do more customization
    std::vector<Gtk::Expander *> expanders;
    findExpanderWidgets(cont, expanders);
    // g_message("Found %d expander widgets\n", expanders.size());
    if (!expanders.empty()) {
        // Always show the file list
        Gtk::Expander *expander = expanders[0];
        expander->set_expanded(true);
    }


    // if (extension == NULL)
    //    checkbox.set_sensitive(FALSE);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    set_default(*add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK));

    show_all_children();
}

/**
 * Destructor
 */
FileExportDialogImpl::~FileExportDialogImpl()
{
}



/**
 * Show this dialog modally.  Return true if user hits [OK]
 */
bool FileExportDialogImpl::show()
{
    Glib::ustring s = Glib::filename_to_utf8(get_current_folder());
    if (s.length() == 0) {
        s = getcwd(NULL, 0);
    }
    set_current_folder(Glib::filename_from_utf8(s)); // hack to force initial dir listing
    set_modal(TRUE); // Window
    sp_transientize(GTK_WIDGET(gobj())); // Make transient
    gint b = run(); // Dialog
    svgPreview.showNoPreview();
    hide();

    if (b == Gtk::RESPONSE_OK) {
        int sel = fileTypeComboBox.get_active_row_number();
        if (sel >= 0 && sel < (int)fileTypes.size()) {
            FileType &type = fileTypes[sel];
            extension = type.extension;
        }
        myFilename = get_filename();
#ifdef WITH_GNOME_VFS
        if (myFilename.empty() && gnome_vfs_initialized()) {
            myFilename = get_uri();
        }
#endif

        /*

        // FIXME: Why do we have more code

        append_extension = checkbox.get_active();
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setBool("/dialogs/save_export/append_extension", append_extension);
        prefs->setBool("/dialogs/save_export/default", ( extension != NULL ? extension->get_id() : "" ));
        */
        return true;
    } else {
        return false;
    }
}


/**
 * Get the file extension type that was selected by the user. Valid after an [OK]
 */
Inkscape::Extension::Extension *FileExportDialogImpl::getSelectionType()
{
    return extension;
}


/**
 * Get the file name chosen by the user.   Valid after an [OK]
 */
Glib::ustring FileExportDialogImpl::getFilename()
{
    return myFilename;
}

#endif // NEW_EXPORT_DIALOG


} // namespace Dialog
} // namespace UI
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
