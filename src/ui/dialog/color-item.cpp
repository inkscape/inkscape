/**
 * @file
 * Inkscape color swatch UI item.
 */
/* Authors:
 *   Jon A. Cruz
 *   Abhishek Sharma
 *
 * Copyright (C) 2010 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>

#include <gtkmm/label.h>
#include <glibmm/i18n.h>
#include <cairo.h>
#include <gtk/gtk.h>

#include "color-item.h"

#include "desktop.h"

#include "desktop-style.h"
#include "display/cairo-utils.h"
#include "document.h"
#include "document-undo.h"
#include "inkscape.h" // for SP_ACTIVE_DESKTOP
#include "io/resource.h"
#include "io/sys.h"
#include "message-context.h"
#include "sp-gradient.h"
#include "sp-item.h"
#include "svg/svg-color.h"
#include "xml/node.h"
#include "xml/repr.h"
#include "verbs.h"
#include "widgets/gradient-vector.h"

#include "color.h" // for SP_RGBA32_U_COMPOSE


namespace Inkscape {
namespace UI {
namespace Dialogs {

static std::vector<std::string> mimeStrings;
static std::map<std::string, guint> mimeToInt;


#if ENABLE_MAGIC_COLORS
// TODO remove this soon:
extern std::vector<SwatchPage*> possible;
#endif // ENABLE_MAGIC_COLORS


#if ENABLE_MAGIC_COLORS
static bool bruteForce( SPDocument* document, Inkscape::XML::Node* node, Glib::ustring const& match, int r, int g, int b )
{
    bool changed = false;

    if ( node ) {
        gchar const * val = node->attribute("inkscape:x-fill-tag");
        if ( val  && (match == val) ) {
            SPObject *obj = document->getObjectByRepr( node );

            gchar c[64] = {0};
            sp_svg_write_color( c, sizeof(c), SP_RGBA32_U_COMPOSE( r, g, b, 0xff ) );
            SPCSSAttr *css = sp_repr_css_attr_new();
            sp_repr_css_set_property( css, "fill", c );

            sp_desktop_apply_css_recursive( obj, css, true );
            static_cast<SPItem*>(obj)->updateRepr();

            changed = true;
        }

        val = node->attribute("inkscape:x-stroke-tag");
        if ( val  && (match == val) ) {
            SPObject *obj = document->getObjectByRepr( node );

            gchar c[64] = {0};
            sp_svg_write_color( c, sizeof(c), SP_RGBA32_U_COMPOSE( r, g, b, 0xff ) );
            SPCSSAttr *css = sp_repr_css_attr_new();
            sp_repr_css_set_property( css, "stroke", c );

            sp_desktop_apply_css_recursive( (SPItem*)obj, css, true );
            ((SPItem*)obj)->updateRepr();

            changed = true;
        }

        Inkscape::XML::Node* first = node->firstChild();
        changed |= bruteForce( document, first, match, r, g, b );

        changed |= bruteForce( document, node->next(), match, r, g, b );
    }

    return changed;
}
#endif // ENABLE_MAGIC_COLORS

static void handleClick( GtkWidget* /*widget*/, gpointer callback_data ) {
    ColorItem* item = reinterpret_cast<ColorItem*>(callback_data);
    if ( item ) {
        item->buttonClicked(false);
    }
}

static void handleSecondaryClick( GtkWidget* /*widget*/, gint /*arg1*/, gpointer callback_data ) {
    ColorItem* item = reinterpret_cast<ColorItem*>(callback_data);
    if ( item ) {
        item->buttonClicked(true);
    }
}

static gboolean handleEnterNotify( GtkWidget* /*widget*/, GdkEventCrossing* /*event*/, gpointer callback_data ) {
    ColorItem* item = reinterpret_cast<ColorItem*>(callback_data);
    if ( item ) {
        SPDesktop *desktop = SP_ACTIVE_DESKTOP;
        if ( desktop ) {
            gchar* msg = g_strdup_printf(_("Color: <b>%s</b>; <b>Click</b> to set fill, <b>Shift+click</b> to set stroke"),
                                         item->def.descr.c_str());
            desktop->tipsMessageContext()->set(Inkscape::INFORMATION_MESSAGE, msg);
            g_free(msg);
        }
    }
    return FALSE;
}

static gboolean handleLeaveNotify( GtkWidget* /*widget*/, GdkEventCrossing* /*event*/, gpointer callback_data ) {
    ColorItem* item = reinterpret_cast<ColorItem*>(callback_data);
    if ( item ) {
        SPDesktop *desktop = SP_ACTIVE_DESKTOP;
        if ( desktop ) {
            desktop->tipsMessageContext()->clear();
        }
    }
    return FALSE;
}

static void dieDieDie( GObject *obj, gpointer user_data )
{
    g_message("die die die %p  %p", obj, user_data );
}

static bool getBlock( std::string& dst, guchar ch, std::string const & str )
{
    bool good = false;
    std::string::size_type pos = str.find(ch);
    if ( pos != std::string::npos )
    {
        std::string::size_type pos2 = str.find( '(', pos );
        if ( pos2 != std::string::npos ) {
            std::string::size_type endPos = str.find( ')', pos2 );
            if ( endPos != std::string::npos ) {
                dst = str.substr( pos2 + 1, (endPos - pos2 - 1) );
                good = true;
            }
        }
    }
    return good;
}

static bool popVal( guint64& numVal, std::string& str )
{
    bool good = false;
    std::string::size_type endPos = str.find(',');
    if ( endPos == std::string::npos ) {
        endPos = str.length();
    }

    if ( endPos != std::string::npos && endPos > 0 ) {
        std::string xxx = str.substr( 0, endPos );
        const gchar* ptr = xxx.c_str();
        gchar* endPtr = 0;
        numVal = g_ascii_strtoull( ptr, &endPtr, 10 );
        if ( (numVal == G_MAXUINT64) && (ERANGE == errno) ) {
            // overflow
        } else if ( (numVal == 0) && (endPtr == ptr) ) {
            // failed conversion
        } else {
            good = true;
            str.erase( 0, endPos + 1 );
        }
    }

    return good;
}

// TODO resolve this more cleanly:
extern gboolean colorItemHandleButtonPress( GtkWidget* /*widget*/, GdkEventButton* event, gpointer user_data);

static void colorItemDragBegin( GtkWidget */*widget*/, GdkDragContext* dc, gpointer data )
{
    ColorItem* item = reinterpret_cast<ColorItem*>(data);
    if ( item )
    {
        using Inkscape::IO::Resource::get_path;
        using Inkscape::IO::Resource::ICONS;
        using Inkscape::IO::Resource::SYSTEM;
        int width = 32;
        int height = 24;

        if (item->def.getType() != ege::PaintDef::RGB){
            GError *error = NULL;
            gsize bytesRead = 0;
            gsize bytesWritten = 0;
            gchar *localFilename = g_filename_from_utf8( get_path(SYSTEM, ICONS, "remove-color.png"),
                                                 -1,
                                                 &bytesRead,
                                                 &bytesWritten,
                                                 &error);
            GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file_at_scale(localFilename, width, height, FALSE, &error);
            g_free(localFilename);
            gtk_drag_set_icon_pixbuf( dc, pixbuf, 0, 0 );
        } else {
            GdkPixbuf* pixbuf = 0;
            if ( item->getGradient() ){
                cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
                cairo_pattern_t *gradient = sp_gradient_create_preview_pattern(item->getGradient(), width);
                cairo_t *ct = cairo_create(s);
                cairo_set_source(ct, gradient);
                cairo_paint(ct);
                cairo_destroy(ct);
                cairo_pattern_destroy(gradient);
                cairo_surface_flush(s);

                pixbuf = ink_pixbuf_create_from_cairo_surface(s);
            } else {
                Glib::RefPtr<Gdk::Pixbuf> thumb = Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB, false, 8, width, height );
                guint32 fillWith = (0xff000000 & (item->def.getR() << 24))
                    | (0x00ff0000 & (item->def.getG() << 16))
                    | (0x0000ff00 & (item->def.getB() <<  8));
                thumb->fill( fillWith );
                pixbuf = thumb->gobj();
                g_object_ref(G_OBJECT(pixbuf));
            }
            gtk_drag_set_icon_pixbuf( dc, pixbuf, 0, 0 );
        }
    }

}

//"drag-drop"
// gboolean dragDropColorData( GtkWidget *widget,
//                             GdkDragContext *drag_context,
//                             gint x,
//                             gint y,
//                             guint time,
//                             gpointer user_data)
// {
// // TODO finish

//     return TRUE;
// }


SwatchPage::SwatchPage()
    : _prefWidth(0)
{
}

SwatchPage::~SwatchPage()
{
}


ColorItem::ColorItem(ege::PaintDef::ColorType type) :
    def(type),
    _isFill(false),
    _isStroke(false),
    _isLive(false),
    _linkIsTone(false),
    _linkPercent(0),
    _linkGray(0),
    _linkSrc(0),
    _grad(0),
    _pattern(0)
{
}

ColorItem::ColorItem( unsigned int r, unsigned int g, unsigned int b, Glib::ustring& name ) :
    def( r, g, b, name ),
    _isFill(false),
    _isStroke(false),
    _isLive(false),
    _linkIsTone(false),
    _linkPercent(0),
    _linkGray(0),
    _linkSrc(0),
    _grad(0),
    _pattern(0)
{
}

ColorItem::~ColorItem()
{
    if (_pattern != NULL) {
        cairo_pattern_destroy(_pattern);
    }
}

ColorItem::ColorItem(ColorItem const &other) :
    Inkscape::UI::Previewable()
{
    if ( this != &other ) {
        *this = other;
    }
}

ColorItem &ColorItem::operator=(ColorItem const &other)
{
    if ( this != &other ) {
        def = other.def;

        // TODO - correct linkage
        _linkSrc = other._linkSrc;
        g_message("Erk!");
    }
    return *this;
}

void ColorItem::setState( bool fill, bool stroke )
{
    if ( (_isFill != fill) || (_isStroke != stroke) ) {
        _isFill = fill;
        _isStroke = stroke;

        for ( std::vector<Gtk::Widget*>::iterator it = _previews.begin(); it != _previews.end(); ++it ) {
            Gtk::Widget* widget = *it;
            if ( IS_EEK_PREVIEW(widget->gobj()) ) {
                EekPreview * preview = EEK_PREVIEW(widget->gobj());

                int val = eek_preview_get_linked( preview );
                val &= ~(PREVIEW_FILL | PREVIEW_STROKE);
                if ( _isFill ) {
                    val |= PREVIEW_FILL;
                }
                if ( _isStroke ) {
                    val |= PREVIEW_STROKE;
                }
                eek_preview_set_linked( preview, static_cast<LinkType>(val) );
            }
        }
    }
}

void ColorItem::setGradient(SPGradient *grad)
{
    if (_grad != grad) {
        _grad = grad;
        // TODO regen and push to listeners
    }

    setName( gr_prepare_label(_grad) );
}

void ColorItem::setName(const Glib::ustring name)
{
    //def.descr = name;

    for ( std::vector<Gtk::Widget*>::iterator it = _previews.begin(); it != _previews.end(); ++it ) {
        Gtk::Widget* widget = *it;
        if ( IS_EEK_PREVIEW(widget->gobj()) ) {
            gtk_widget_set_tooltip_text(GTK_WIDGET(widget->gobj()), name.c_str());
        }
        else if ( GTK_IS_LABEL(widget->gobj()) ) {
            gtk_label_set_text(GTK_LABEL(widget->gobj()), name.c_str());
        }
    }
}

void ColorItem::setPattern(cairo_pattern_t *pattern)
{
    if (pattern) {
        cairo_pattern_reference(pattern);
    }
    if (_pattern) {
        cairo_pattern_destroy(_pattern);
    }
    _pattern = pattern;

    _updatePreviews();
}

void ColorItem::_dragGetColorData( GtkWidget */*widget*/,
                                   GdkDragContext */*drag_context*/,
                                   GtkSelectionData *data,
                                   guint info,
                                   guint /*time*/,
                                   gpointer user_data)
{
    ColorItem* item = reinterpret_cast<ColorItem*>(user_data);
    std::string key;
    if ( info < mimeStrings.size() ) {
        key = mimeStrings[info];
    } else {
        g_warning("ERROR: unknown value (%d)", info);
    }

    if ( !key.empty() ) {
        char* tmp = 0;
        int len = 0;
        int format = 0;
        item->def.getMIMEData(key, tmp, len, format);
        if ( tmp ) {
            GdkAtom dataAtom = gdk_atom_intern( key.c_str(), FALSE );
            gtk_selection_data_set( data, dataAtom, format, (guchar*)tmp, len );
            delete[] tmp;
        }
    }
}

void ColorItem::_dropDataIn( GtkWidget */*widget*/,
                             GdkDragContext */*drag_context*/,
                             gint /*x*/, gint /*y*/,
                             GtkSelectionData */*data*/,
                             guint /*info*/,
                             guint /*event_time*/,
                             gpointer /*user_data*/)
{
}

void ColorItem::_colorDefChanged(void* data)
{
    ColorItem* item = reinterpret_cast<ColorItem*>(data);
    if ( item ) {
        item->_updatePreviews();
    }
}

void ColorItem::_updatePreviews()
{
    for ( std::vector<Gtk::Widget*>::iterator it =  _previews.begin(); it != _previews.end(); ++it ) {
        Gtk::Widget* widget = *it;
        if ( IS_EEK_PREVIEW(widget->gobj()) ) {
            EekPreview * preview = EEK_PREVIEW(widget->gobj());

            _regenPreview(preview);

            widget->queue_draw();
        }
    }

    for ( std::vector<ColorItem*>::iterator it = _listeners.begin(); it != _listeners.end(); ++it ) {
        guint r = def.getR();
        guint g = def.getG();
        guint b = def.getB();

        if ( (*it)->_linkIsTone ) {
            r = ( ((*it)->_linkPercent * (*it)->_linkGray) + ((100 - (*it)->_linkPercent) * r) ) / 100;
            g = ( ((*it)->_linkPercent * (*it)->_linkGray) + ((100 - (*it)->_linkPercent) * g) ) / 100;
            b = ( ((*it)->_linkPercent * (*it)->_linkGray) + ((100 - (*it)->_linkPercent) * b) ) / 100;
        } else {
            r = ( ((*it)->_linkPercent * 255) + ((100 - (*it)->_linkPercent) * r) ) / 100;
            g = ( ((*it)->_linkPercent * 255) + ((100 - (*it)->_linkPercent) * g) ) / 100;
            b = ( ((*it)->_linkPercent * 255) + ((100 - (*it)->_linkPercent) * b) ) / 100;
        }

        (*it)->def.setRGB( r, g, b );
    }


#if ENABLE_MAGIC_COLORS
    // Look for objects using this color
    {
        SPDesktop *desktop = SP_ACTIVE_DESKTOP;
        if ( desktop ) {
            SPDocument* document = desktop->getDocument();
            Inkscape::XML::Node *rroot =  document->getReprRoot();
            if ( rroot ) {

                // Find where this thing came from
                Glib::ustring paletteName;
                bool found = false;
                int index = 0;
                for ( std::vector<SwatchPage*>::iterator it2 = possible.begin(); it2 != possible.end() && !found; ++it2 ) {
                    SwatchPage* curr = *it2;
                    index = 0;
                    for ( boost::ptr_vector<ColorItem>::iterator zz = curr->_colors.begin(); zz != curr->_colors.end(); ++zz ) {
                        if ( this == &*zz ) {
                            found = true;
                            paletteName = curr->_name;
                            break;
                        } else {
                            index++;
                        }
                    }
                }

                if ( !paletteName.empty() ) {
                    gchar* str = g_strdup_printf("%d|", index);
                    paletteName.insert( 0, str );
                    g_free(str);
                    str = 0;

                    if ( bruteForce( document, rroot, paletteName, def.getR(), def.getG(), def.getB() ) ) {
                        SPDocumentUndo::done( document , SP_VERB_DIALOG_SWATCHES,
                                          _("Change color definition"));
                    }
                }
            }
        }
    }
#endif // ENABLE_MAGIC_COLORS

}

void ColorItem::_regenPreview(EekPreview * preview)
{
    if ( def.getType() != ege::PaintDef::RGB ) {
        using Inkscape::IO::Resource::get_path;
        using Inkscape::IO::Resource::ICONS;
        using Inkscape::IO::Resource::SYSTEM;
        GError *error = NULL;
        gsize bytesRead = 0;
        gsize bytesWritten = 0;
        gchar *localFilename = g_filename_from_utf8( get_path(SYSTEM, ICONS, "remove-color.png"),
                                                     -1,
                                                     &bytesRead,
                                                     &bytesWritten,
                                                     &error);
        GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(localFilename, &error);
        if (!pixbuf) {
            g_warning("Null pixbuf for %p [%s]", localFilename, localFilename );
        }
        g_free(localFilename);

        eek_preview_set_pixbuf( preview, pixbuf );
    }
    else if ( !_pattern ){
        eek_preview_set_color( preview,
                               (def.getR() << 8) | def.getR(),
                               (def.getG() << 8) | def.getG(),
                               (def.getB() << 8) | def.getB() );
    } else {
        // These correspond to PREVIEW_PIXBUF_WIDTH and VBLOCK from swatches.cpp
        // TODO: the pattern to draw should be in the widget that draws the preview,
        //       so the preview can be scalable
        int w = 128;
        int h = 16;

        cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
        cairo_t *ct = cairo_create(s);
        cairo_set_source(ct, _pattern);
        cairo_paint(ct);
        cairo_destroy(ct);
        cairo_surface_flush(s);

        GdkPixbuf* pixbuf = ink_pixbuf_create_from_cairo_surface(s);
        eek_preview_set_pixbuf( preview, pixbuf );
    }

    eek_preview_set_linked( preview, (LinkType)((_linkSrc ? PREVIEW_LINK_IN:0)
                                                | (_listeners.empty() ? 0:PREVIEW_LINK_OUT)
                                                | (_isLive ? PREVIEW_LINK_OTHER:0)) );
}

Gtk::Widget* ColorItem::getPreview(PreviewStyle style, ViewType view, ::PreviewSize size, guint ratio, guint border)
{
    Gtk::Widget* widget = 0;
    if ( style == PREVIEW_STYLE_BLURB) {
        Gtk::Label *lbl = new Gtk::Label(def.descr);
        lbl->set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
        widget = lbl;
    } else {
        GtkWidget* eekWidget = eek_preview_new();
        gtk_widget_set_name( eekWidget, "ColorItemPreview" );

        EekPreview * preview = EEK_PREVIEW(eekWidget);
        Gtk::Widget* newBlot = Glib::wrap(eekWidget);
        _regenPreview(preview);

        eek_preview_set_details( preview, 
                                 (::ViewType)view,
                                 (::PreviewSize)size,
                                 ratio,
                                 border );

        def.addCallback( _colorDefChanged, this );
        eek_preview_set_focus_on_click(preview, FALSE);
        newBlot->set_tooltip_text(def.descr);

        g_signal_connect( G_OBJECT(newBlot->gobj()),
                          "clicked",
                          G_CALLBACK(handleClick),
                          this);

        g_signal_connect( G_OBJECT(newBlot->gobj()),
                          "alt-clicked",
                          G_CALLBACK(handleSecondaryClick),
                          this);

        g_signal_connect( G_OBJECT(newBlot->gobj()),
                          "button-press-event",
                          G_CALLBACK(colorItemHandleButtonPress),
                          this);

        {
            std::vector<std::string> listing = def.getMIMETypes();
            int entryCount = listing.size();
            GtkTargetEntry* entries = new GtkTargetEntry[entryCount];
            GtkTargetEntry* curr = entries;
            for ( std::vector<std::string>::iterator it = listing.begin(); it != listing.end(); ++it ) {
                curr->target = g_strdup(it->c_str());
                curr->flags = 0;
                if ( mimeToInt.find(*it) == mimeToInt.end() ){
                    // these next lines are order-dependent:
                    mimeToInt[*it] = mimeStrings.size();
                    mimeStrings.push_back(*it);
                }
                curr->info = mimeToInt[curr->target];
                curr++;
            }
            gtk_drag_source_set( GTK_WIDGET(newBlot->gobj()),
                                 GDK_BUTTON1_MASK,
                                 entries, entryCount,
                                 GdkDragAction(GDK_ACTION_MOVE | GDK_ACTION_COPY) );
            for ( int i = 0; i < entryCount; i++ ) {
                g_free(entries[i].target);
            }
            delete[] entries;
        }

        g_signal_connect( G_OBJECT(newBlot->gobj()),
                          "drag-data-get",
                          G_CALLBACK(ColorItem::_dragGetColorData),
                          this);

        g_signal_connect( G_OBJECT(newBlot->gobj()),
                          "drag-begin",
                          G_CALLBACK(colorItemDragBegin),
                          this );

        g_signal_connect( G_OBJECT(newBlot->gobj()),
                          "enter-notify-event",
                          G_CALLBACK(handleEnterNotify),
                          this);

        g_signal_connect( G_OBJECT(newBlot->gobj()),
                          "leave-notify-event",
                          G_CALLBACK(handleLeaveNotify),
                          this);

        g_signal_connect( G_OBJECT(newBlot->gobj()),
                          "destroy",
                          G_CALLBACK(dieDieDie),
                          this);


        widget = newBlot;
    }

    _previews.push_back( widget );

    return widget;
}

void ColorItem::buttonClicked(bool secondary)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop) {
        char const * attrName = secondary ? "stroke" : "fill";

        SPCSSAttr *css = sp_repr_css_attr_new();
        Glib::ustring descr;
        switch (def.getType()) {
            case ege::PaintDef::CLEAR: {
                // TODO actually make this clear
                sp_repr_css_set_property( css, attrName, "none" );
                descr = secondary? _("Remove stroke color") : _("Remove fill color");
                break;
            }
            case ege::PaintDef::NONE: {
                sp_repr_css_set_property( css, attrName, "none" );
                descr = secondary? _("Set stroke color to none") : _("Set fill color to none");
                break;
            }
//mark
            case ege::PaintDef::RGB: {
                Glib::ustring colorspec;
                if ( _grad ){
                    colorspec = "url(#";
                    colorspec += _grad->getId();
                    colorspec += ")";
                } else {
                    gchar c[64];
                    guint32 rgba = (def.getR() << 24) | (def.getG() << 16) | (def.getB() << 8) | 0xff;
                    sp_svg_write_color(c, sizeof(c), rgba);
                    colorspec = c;
                }
//end mark
                sp_repr_css_set_property( css, attrName, colorspec.c_str() );
                descr = secondary? _("Set stroke color from swatch") : _("Set fill color from swatch");
                break;
            }
        }
        sp_desktop_set_style(desktop, css);
        sp_repr_css_attr_unref(css);

        DocumentUndo::done( desktop->getDocument(), SP_VERB_DIALOG_SWATCHES, descr.c_str() );
    }
}

void ColorItem::_wireMagicColors( SwatchPage *colorSet )
{
    if ( colorSet )
    {
        for ( boost::ptr_vector<ColorItem>::iterator it = colorSet->_colors.begin(); it != colorSet->_colors.end(); ++it )
        {
            std::string::size_type pos = it->def.descr.find("*{");
            if ( pos != std::string::npos )
            {
                std::string subby = it->def.descr.substr( pos + 2 );
                std::string::size_type endPos = subby.find("}*");
                if ( endPos != std::string::npos )
                {
                    subby.erase( endPos );
                    //g_message("FOUND MAGIC at '%s'", (*it)->def.descr.c_str());
                    //g_message("               '%s'", subby.c_str());

                    if ( subby.find('E') != std::string::npos )
                    {
                        it->def.setEditable( true );
                    }

                    if ( subby.find('L') != std::string::npos )
                    {
                        it->_isLive = true;
                    }

                    std::string part;
                    // Tint. index + 1 more val.
                    if ( getBlock( part, 'T', subby ) ) {
                        guint64 colorIndex = 0;
                        if ( popVal( colorIndex, part ) ) {
                            guint64 percent = 0;
                            if ( popVal( percent, part ) ) {
                                it->_linkTint( colorSet->_colors[colorIndex], percent );
                            }
                        }
                    }

                    // Shade/tone. index + 1 or 2 more val.
                    if ( getBlock( part, 'S', subby ) ) {
                        guint64 colorIndex = 0;
                        if ( popVal( colorIndex, part ) ) {
                            guint64 percent = 0;
                            if ( popVal( percent, part ) ) {
                                guint64 grayLevel = 0;
                                if ( !popVal( grayLevel, part ) ) {
                                    grayLevel = 0;
                                }
                                it->_linkTone( colorSet->_colors[colorIndex], percent, grayLevel );
                            }
                        }
                    }

                }
            }
        }
    }
}


void ColorItem::_linkTint( ColorItem& other, int percent )
{
    if ( !_linkSrc )
    {
        other._listeners.push_back(this);
        _linkIsTone = false;
        _linkPercent = percent;
        if ( _linkPercent > 100 )
            _linkPercent = 100;
        if ( _linkPercent < 0 )
            _linkPercent = 0;
        _linkGray = 0;
        _linkSrc = &other;

        ColorItem::_colorDefChanged(&other);
    }
}

void ColorItem::_linkTone( ColorItem& other, int percent, int grayLevel )
{
    if ( !_linkSrc )
    {
        other._listeners.push_back(this);
        _linkIsTone = true;
        _linkPercent = percent;
        if ( _linkPercent > 100 )
            _linkPercent = 100;
        if ( _linkPercent < 0 )
            _linkPercent = 0;
        _linkGray = grayLevel;
        _linkSrc = &other;

        ColorItem::_colorDefChanged(&other);
    }
}

} // namespace Dialogs
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
