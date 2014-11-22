#ifndef SEEN_SP_COLOR_NOTEBOOK_H
#define SEEN_SP_COLOR_NOTEBOOK_H

/*
 * A notebook with RGB, CMYK, CMS, HSL, and Wheel pages
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 *
 * This code is in public domain
 */

#include "sp-color-selector.h"

#include <glib.h>

struct SPColorNotebook;

class ColorNotebook: public ColorSelector
{
public:
    ColorNotebook( SPColorSelector* csel );
    virtual ~ColorNotebook();

    virtual void init();

    SPColorSelector* getCurrentSelector();
    void switchPage( GtkNotebook *notebook, GtkWidget *page, guint page_num );

    GtkWidget* addPage( GType page_type, guint submode );
    void removePage( GType page_type, guint submode );
    GtkWidget* getPage( GType page_type, guint submode );

    gint menuHandler( GdkEvent* event );

protected:
    static void _rgbaEntryChangedHook( GtkEntry* entry, SPColorNotebook *colorbook );
    static void _entryGrabbed( SPColorSelector *csel, SPColorNotebook *colorbook );
    static void _entryDragged( SPColorSelector *csel, SPColorNotebook *colorbook );
    static void _entryReleased( SPColorSelector *csel, SPColorNotebook *colorbook );
    static void _entryChanged( SPColorSelector *csel, SPColorNotebook *colorbook );
    static void _entryModified( SPColorSelector *csel, SPColorNotebook *colorbook );
    static void _buttonClicked(GtkWidget *widget,  SPColorNotebook *colorbook);
    static void _picker_clicked(GtkWidget *widget,  SPColorNotebook *colorbook);

    virtual void _colorChanged();

    void _rgbaEntryChanged( GtkEntry* entry );
    void _updateRgbaEntry( const SPColor& color, gfloat alpha );
    void _setCurrentPage(int i);

    gboolean _updating : 1;
    gboolean _updatingrgba : 1;
    gboolean _dragging : 1;
    gulong _switchId;
    gulong _entryId;
    GtkWidget *_book;
    GtkWidget *_buttonbox;
    GtkWidget **_buttons;
    GtkWidget *_rgbal, *_rgbae; /* RGBA entry */
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    GtkWidget *_box_outofgamut, *_box_colormanaged, *_box_toomuchink;
#endif //defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
    GtkWidget *_btn_picker;
    GtkWidget *_p; /* Color preview */
    GtkWidget *_btn;
    GtkWidget *_popup;
    GPtrArray *_trackerList;

private:
    // By default, disallow copy constructor and assignment operator
    ColorNotebook( const ColorNotebook& obj );
    ColorNotebook& operator=( const ColorNotebook& obj );
};



#define SP_TYPE_COLOR_NOTEBOOK (sp_color_notebook_get_type ())
#define SP_COLOR_NOTEBOOK(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_COLOR_NOTEBOOK, SPColorNotebook))
#define SP_COLOR_NOTEBOOK_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), SP_TYPE_COLOR_NOTEBOOK, SPColorNotebookClass))
#define SP_IS_COLOR_NOTEBOOK(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_COLOR_NOTEBOOK))
#define SP_IS_COLOR_NOTEBOOK_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), SP_TYPE_COLOR_NOTEBOOK))

struct SPColorNotebook {
    SPColorSelector parent;    /* Parent */
};

struct SPColorNotebookClass {
    SPColorSelectorClass parent_class;

    void (* grabbed) (SPColorNotebook *rgbsel);
    void (* dragged) (SPColorNotebook *rgbsel);
    void (* released) (SPColorNotebook *rgbsel);
    void (* changed) (SPColorNotebook *rgbsel);
};

GType sp_color_notebook_get_type(void);

GtkWidget *sp_color_notebook_new (void);

/* void sp_color_notebook_set_mode (SPColorNotebook *csel, SPColorNotebookMode mode); */
/* SPColorNotebookMode sp_color_notebook_get_mode (SPColorNotebook *csel); */



#endif // SEEN_SP_COLOR_NOTEBOOK_H

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

