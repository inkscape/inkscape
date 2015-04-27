#ifndef __UI_DIALOG_IMAGE_H__
#define __UI_DIALOG_IMAGE_H__
/*
 * A simple image display widget, using Inkscape's own rendering engine
 *
 * Authors:
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004 The Inkscape Organization
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/box.h>
#include <glibmm/ustring.h>

class SPDocument;

namespace Inkscape
{
namespace UI
{
namespace Widget
{


/*#########################################################################
### ImageIcon widget
#########################################################################*/

/**
 * This class is evolved from the SVGPreview widget of the FileChooser
 * This uses Inkscape's renderer to show images in a variety of formats,
 * including SVG
 */
class ImageIcon : public Gtk::VBox
{
public:

    /**
     * Constructor
     */
    ImageIcon();

    /**
     * Construct from a file name
     */
    ImageIcon(const Glib::ustring &fileName);

    /**
     * Copy Constructor
     */
    ImageIcon(const ImageIcon &other);

    /**
     * Destructor
     */
    ~ImageIcon();

    /**
     *
     */
    bool showSvgDocument(const SPDocument *doc);

    /**
     *
     */
    bool showSvgFile(const Glib::ustring &fileName);

    /**
     *
     */
    bool showSvgFromMemory(const char *xmlBuffer);

    /**
     * Show image embedded in SVG
     */
    bool showBitmap(const Glib::ustring &fileName);

    /**
     * Show the "Too large" image
     */
    void showBrokenImage(const Glib::ustring &reason);

    /**
     *
     */
    bool show(const Glib::ustring &fileName);

private:

    /**
     * basic initialization, called by the various constructors
     */
    void init();

    /**
     * The svg document we are currently showing
     */
    SPDocument *document;

    /**
     * The sp_svg_view widget
     */
    Gtk::Widget *viewerGtkmm;

    /**
     * are we currently showing the "broken image" image?
     */
    bool showingBrokenImage;


    /**
     * will be set by showImageIcon as a side-effect of an error
     */
    Glib::ustring bitmapError;


    /**
     * will be set by showImageIcon as a side-effect of an error
     */
    Glib::ustring svgError;

};



} // namespace Widget
} // namespace UI
} // namespace Inkscape


#endif /* __UI_DIALOG_IMAGE_H__ */
/*#########################################################################
### E N D    O F    F I L E
#########################################################################*/
