/** @file
 * @brief Color swatches dialog
 */
/* Authors:
 *   Jon A. Cruz
 *   John Bintz
 *
 * Copyright (C) 2005 Jon A. Cruz
 * Copyright (C) 2008 John Bintz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <errno.h>
#include <map>

#include <gtk/gtkdialog.h> //for GTK_RESPONSE* types
#include <gtk/gtkdnd.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkseparatormenuitem.h>
#include <glibmm/i18n.h>
#include <gdkmm/pixbuf.h>

#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-style.h"
#include "document.h"
#include "document-private.h"
#include "extension/db.h"
#include "inkscape.h"
#include "inkscape.h"
#include "io/sys.h"
#include "io/resource.h"
#include "message-context.h"
#include "path-prefix.h"
#include "preferences.h"
#include "sp-item.h"
#include "svg/svg-color.h"
#include "sp-gradient-fns.h"
#include "sp-gradient.h"
#include "sp-gradient-vector.h"
#include "swatches.h"
#include "widgets/gradient-vector.h"
#include "widgets/eek-preview.h"
#include "display/nr-plain-stuff.h"

namespace Inkscape {
namespace UI {
namespace Dialogs {


void _loadPaletteFile( gchar const *filename );

/**
 * The color swatch you see on screen as a clickable box.
 */
class ColorItem : public Inkscape::UI::Previewable
{
    friend void _loadPaletteFile( gchar const *filename );
public:
    ColorItem( ege::PaintDef::ColorType type );
    ColorItem( unsigned int r, unsigned int g, unsigned int b,
               Glib::ustring& name );
    virtual ~ColorItem();
    ColorItem(ColorItem const &other);
    virtual ColorItem &operator=(ColorItem const &other);
    virtual Gtk::Widget* getPreview(PreviewStyle style,
                                    ViewType view,
                                    ::PreviewSize size,
                                    guint ratio);
    void buttonClicked(bool secondary = false);
    ege::PaintDef def;
    void* ptr;

private:
    static void _dropDataIn( GtkWidget *widget,
                             GdkDragContext *drag_context,
                             gint x, gint y,
                             GtkSelectionData *data,
                             guint info,
                             guint event_time,
                             gpointer user_data);

    static void _dragGetColorData( GtkWidget *widget,
                                   GdkDragContext *drag_context,
                                   GtkSelectionData *data,
                                   guint info,
                                   guint time,
                                   gpointer user_data);

    static void _wireMagicColors( void* p );
    static void _colorDefChanged(void* data);

    void _linkTint( ColorItem& other, int percent );
    void _linkTone( ColorItem& other, int percent, int grayLevel );

    Gtk::Tooltips tips;
    std::vector<Gtk::Widget*> _previews;

    bool _isLive;
    bool _linkIsTone;
    int _linkPercent;
    int _linkGray;
    ColorItem* _linkSrc;
    std::vector<ColorItem*> _listeners;
};



ColorItem::ColorItem(ege::PaintDef::ColorType type) :
    def(type),
    ptr(0),
    _isLive(false),
    _linkIsTone(false),
    _linkPercent(0),
    _linkGray(0),
    _linkSrc(0)
{
}

ColorItem::ColorItem( unsigned int r, unsigned int g, unsigned int b, Glib::ustring& name ) :
    def( r, g, b, name ),
    ptr(0),
    _isLive(false),
    _linkIsTone(false),
    _linkPercent(0),
    _linkGray(0),
    _linkSrc(0)
{
}

ColorItem::~ColorItem()
{
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


class JustForNow
{
public:
    JustForNow() : _prefWidth(0) {}

    Glib::ustring _name;
    int _prefWidth;
    std::vector<ColorItem*> _colors;
};

static std::vector<JustForNow*> possible;


static std::vector<std::string> mimeStrings;
static std::map<std::string, guint> mimeToInt;

static std::map<ColorItem*, guchar*> previewMap;
static std::map<ColorItem*, SPGradient*> gradMap; // very temporary workaround.

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

static void dragBegin( GtkWidget */*widget*/, GdkDragContext* dc, gpointer data )
{
    ColorItem* item = reinterpret_cast<ColorItem*>(data);
    if ( item )
    {
        using Inkscape::IO::Resource::get_path;
        using Inkscape::IO::Resource::ICONS;
        using Inkscape::IO::Resource::SYSTEM;

        if (item->def.getType() != ege::PaintDef::RGB){
            GError *error = NULL;
            gsize bytesRead = 0;
            gsize bytesWritten = 0;
            gchar *localFilename = g_filename_from_utf8( get_path(SYSTEM, ICONS, "remove-color.png"),
                                                 -1,
                                                 &bytesRead,
                                                 &bytesWritten,
                                                 &error);
            GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file_at_scale(localFilename, 32, 24, FALSE, &error);
            g_free(localFilename);
            gtk_drag_set_icon_pixbuf( dc, pixbuf, 0, 0 );
            return;
        }

        Glib::RefPtr<Gdk::Pixbuf> thumb = Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB, false, 8, 32, 24 );
        guint32 fillWith = (0xff000000 & (item->def.getR() << 24))
                         | (0x00ff0000 & (item->def.getG() << 16))
                         | (0x0000ff00 & (item->def.getB() <<  8));
        thumb->fill( fillWith );
        gtk_drag_set_icon_pixbuf( dc, thumb->gobj(), 0, 0 );
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

static GtkWidget* popupMenu = 0;
static std::vector<GtkWidget*> popupExtras;
static ColorItem* bounceTarget = 0;

static void redirClick( GtkMenuItem *menuitem, gpointer /*user_data*/ )
{
    if ( bounceTarget ) {
        handleClick( GTK_WIDGET(menuitem), bounceTarget );
    }
}

static void redirSecondaryClick( GtkMenuItem *menuitem, gpointer /*user_data*/ )
{
    if ( bounceTarget ) {
        handleSecondaryClick( GTK_WIDGET(menuitem), 0, bounceTarget );
    }
}

static void editGradientImpl( SPGradient* gr )
{
    if ( gr ) {
        GtkWidget *dialog = sp_gradient_vector_editor_new( gr );
        gtk_widget_show( dialog );
    }
}

static void editGradient( GtkMenuItem */*menuitem*/, gpointer /*user_data*/ )
{
    if ( bounceTarget ) {
        SwatchesPanel* swp = bounceTarget->ptr ? reinterpret_cast<SwatchesPanel*>(bounceTarget->ptr) : 0;
        SPDesktop* desktop = swp ? swp->getDesktop() : 0;
        SPDocument *doc = desktop ? desktop->doc() : 0;
        if (doc) {
            std::string targetName(bounceTarget->def.descr);
            const GSList *gradients = sp_document_get_resource_list(doc, "gradient");
            for (const GSList *item = gradients; item; item = item->next) {
                SPGradient* grad = SP_GRADIENT(item->data);
                if ( targetName == grad->id ) {
                    editGradientImpl( grad );
                    break;
                }
            }
        }
    }
}

static void addNewGradient( GtkMenuItem */*menuitem*/, gpointer /*user_data*/ )
{
    if ( bounceTarget ) {
        SwatchesPanel* swp = bounceTarget->ptr ? reinterpret_cast<SwatchesPanel*>(bounceTarget->ptr) : 0;
        SPDesktop* desktop = swp ? swp->getDesktop() : 0;
        SPDocument *doc = desktop ? desktop->doc() : 0;
        if (doc) {
            Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

            Inkscape::XML::Node *repr = xml_doc->createElement("svg:linearGradient");
            repr->setAttribute("osb:paint", "solid");
            Inkscape::XML::Node *stop = xml_doc->createElement("svg:stop");
            stop->setAttribute("offset", "0");
            stop->setAttribute("style", "stop-color:#000;stop-opacity:1;");
            repr->appendChild(stop);
            Inkscape::GC::release(stop);

            SP_OBJECT_REPR( SP_DOCUMENT_DEFS(doc) )->addChild(repr, NULL);

            SPGradient * gr = static_cast<SPGradient *>(doc->getObjectByRepr(repr));

            Inkscape::GC::release(repr);


            editGradientImpl( gr );
            // Work-around for timing of gradient addition change. Must follow edit.
            if ( swp ) {
                swp->handleGradientsChange();
            }
        }
    }
}

static gboolean handleButtonPress( GtkWidget* /*widget*/, GdkEventButton* event, gpointer user_data)
{
    gboolean handled = FALSE;

    if ( (event->button == 3) && (event->type == GDK_BUTTON_PRESS) ) {
        if ( !popupMenu ) {
            popupMenu = gtk_menu_new();
            GtkWidget* child = 0;

            //TRANSLATORS: An item in context menu on a colour in the swatches
            child = gtk_menu_item_new_with_label(_("Set fill"));
            g_signal_connect( G_OBJECT(child),
                              "activate",
                              G_CALLBACK(redirClick),
                              user_data);
            gtk_menu_shell_append(GTK_MENU_SHELL(popupMenu), child);

            //TRANSLATORS: An item in context menu on a colour in the swatches
            child = gtk_menu_item_new_with_label(_("Set stroke"));

            g_signal_connect( G_OBJECT(child),
                              "activate",
                              G_CALLBACK(redirSecondaryClick),
                              user_data);
            gtk_menu_shell_append(GTK_MENU_SHELL(popupMenu), child);

            child = gtk_separator_menu_item_new();
            gtk_menu_shell_append(GTK_MENU_SHELL(popupMenu), child);
            popupExtras.push_back(child);

            child = gtk_menu_item_new_with_label(_("Add"));
            g_signal_connect( G_OBJECT(child),
                              "activate",
                              G_CALLBACK(addNewGradient),
                              user_data );
            gtk_menu_shell_append(GTK_MENU_SHELL(popupMenu), child);
            popupExtras.push_back(child);

            child = gtk_menu_item_new_with_label(_("Delete"));
            gtk_menu_shell_append(GTK_MENU_SHELL(popupMenu), child);
            //popupExtras.push_back(child);
            gtk_widget_set_sensitive( child, FALSE );

            child = gtk_menu_item_new_with_label(_("Edit..."));
            g_signal_connect( G_OBJECT(child),
                              "activate",
                              G_CALLBACK(editGradient),
                              user_data );
            gtk_menu_shell_append(GTK_MENU_SHELL(popupMenu), child);
            popupExtras.push_back(child);

            child = gtk_separator_menu_item_new();
            gtk_menu_shell_append(GTK_MENU_SHELL(popupMenu), child);
            popupExtras.push_back(child);

            child = gtk_menu_item_new_with_label(_("Convert"));
            gtk_menu_shell_append(GTK_MENU_SHELL(popupMenu), child);
            //popupExtras.push_back(child);
            gtk_widget_set_sensitive( child, FALSE );

            gtk_widget_show_all(popupMenu);
        }

        ColorItem* item = reinterpret_cast<ColorItem*>(user_data);
        if ( item ) {
            SwatchesPanel* swp = item->ptr ? reinterpret_cast<SwatchesPanel*>(item->ptr) : 0;
            bool show = swp && (swp->getSelectedIndex() == 0);
            for ( std::vector<GtkWidget*>::iterator it = popupExtras.begin(); it != popupExtras.end(); ++ it) {
                gtk_widget_set_sensitive(*it, show);
            }

            bounceTarget = item;
            if ( popupMenu ) {
                gtk_menu_popup(GTK_MENU(popupMenu), NULL, NULL, NULL, NULL, event->button, event->time);
                handled = TRUE;
            }
        }
    }

    return handled;
}

static void dieDieDie( GtkObject *obj, gpointer user_data )
{
    g_message("die die die %p  %p", obj, user_data );
}

#include "color.h" // for SP_RGBA32_U_COMPOSE

void ColorItem::_dropDataIn( GtkWidget */*widget*/,
                             GdkDragContext */*drag_context*/,
                             gint /*x*/, gint /*y*/,
                             GtkSelectionData */*data*/,
                             guint /*info*/,
                             guint /*event_time*/,
                             gpointer /*user_data*/)
{
}

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

            sp_desktop_apply_css_recursive( (SPItem*)obj, css, true );
            ((SPItem*)obj)->updateRepr();

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

void ColorItem::_colorDefChanged(void* data)
{
    ColorItem* item = reinterpret_cast<ColorItem*>(data);
    if ( item ) {
        for ( std::vector<Gtk::Widget*>::iterator it =  item->_previews.begin(); it != item->_previews.end(); ++it ) {
            Gtk::Widget* widget = *it;
            if ( IS_EEK_PREVIEW(widget->gobj()) ) {
                EekPreview * preview = EEK_PREVIEW(widget->gobj());
                eek_preview_set_color( preview,
                                       (item->def.getR() << 8) | item->def.getR(),
                                       (item->def.getG() << 8) | item->def.getG(),
                                       (item->def.getB() << 8) | item->def.getB() );

                eek_preview_set_linked( preview, (LinkType)((item->_linkSrc ? PREVIEW_LINK_IN:0)
                                                            | (item->_listeners.empty() ? 0:PREVIEW_LINK_OUT)
                                                            | (item->_isLive ? PREVIEW_LINK_OTHER:0)) );

                widget->queue_draw();
            }
        }

        for ( std::vector<ColorItem*>::iterator it = item->_listeners.begin(); it != item->_listeners.end(); ++it ) {
            guint r = item->def.getR();
            guint g = item->def.getG();
            guint b = item->def.getB();

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


        // Look for objects using this color
        {
            SPDesktop *desktop = SP_ACTIVE_DESKTOP;
            if ( desktop ) {
                SPDocument* document = sp_desktop_document( desktop );
                Inkscape::XML::Node *rroot =  sp_document_repr_root( document );
                if ( rroot ) {

                    // Find where this thing came from
                    Glib::ustring paletteName;
                    bool found = false;
                    int index = 0;
                    for ( std::vector<JustForNow*>::iterator it2 = possible.begin(); it2 != possible.end() && !found; ++it2 ) {
                        JustForNow* curr = *it2;
                        index = 0;
                        for ( std::vector<ColorItem*>::iterator zz = curr->_colors.begin(); zz != curr->_colors.end(); ++zz ) {
                            if ( item == *zz ) {
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

                        if ( bruteForce( document, rroot, paletteName, item->def.getR(), item->def.getG(), item->def.getB() ) ) {
                            sp_document_done( document , SP_VERB_DIALOG_SWATCHES,
                                              _("Change color definition"));
                        }
                    }
                }
            }
        }
    }
}


Gtk::Widget* ColorItem::getPreview(PreviewStyle style, ViewType view, ::PreviewSize size, guint ratio)
{
    Gtk::Widget* widget = 0;
    if ( style == PREVIEW_STYLE_BLURB) {
        Gtk::Label *lbl = new Gtk::Label(def.descr);
        lbl->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
        widget = lbl;
    } else {
//         Glib::ustring blank("          ");
//         if ( size == Inkscape::ICON_SIZE_MENU || size == Inkscape::ICON_SIZE_DECORATION ) {
//             blank = " ";
//         }

        GtkWidget* eekWidget = eek_preview_new();
        EekPreview * preview = EEK_PREVIEW(eekWidget);
        Gtk::Widget* newBlot = Glib::wrap(eekWidget);

        if ( previewMap.find(this) == previewMap.end() ){
            eek_preview_set_color( preview, (def.getR() << 8) | def.getR(),
                                   (def.getG() << 8) | def.getG(),
                                   (def.getB() << 8) | def.getB());
        } else {
            guchar* px = previewMap[this];
            int width = 128;
            int height = 16;
            GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data( px, GDK_COLORSPACE_RGB, FALSE, 8,
                                                          width, height, width * 3,
                                                          0, // add delete function
                                                          0
                );
            eek_preview_set_pixbuf( preview, pixbuf );
        }
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

        eek_preview_set_details( preview, (::PreviewStyle)style, (::ViewType)view, (::PreviewSize)size, ratio );
        eek_preview_set_linked( preview, (LinkType)((_linkSrc ? PREVIEW_LINK_IN:0)
                                                    | (_listeners.empty() ? 0:PREVIEW_LINK_OUT)
                                                    | (_isLive ? PREVIEW_LINK_OTHER:0)) );

        def.addCallback( _colorDefChanged, this );

        GValue val = {0, {{0}, {0}}};
        g_value_init( &val, G_TYPE_BOOLEAN );
        g_value_set_boolean( &val, FALSE );
        g_object_set_property( G_OBJECT(preview), "focus-on-click", &val );

/*
        Gtk::Button *btn = new Gtk::Button(blank);
        Gdk::Color color;
        color.set_rgb((_r << 8)|_r, (_g << 8)|_g, (_b << 8)|_b);
        btn->modify_bg(Gtk::STATE_NORMAL, color);
        btn->modify_bg(Gtk::STATE_ACTIVE, color);
        btn->modify_bg(Gtk::STATE_PRELIGHT, color);
        btn->modify_bg(Gtk::STATE_SELECTED, color);

        Gtk::Widget* newBlot = btn;
*/

        tips.set_tip((*newBlot), def.descr);

/*
        newBlot->signal_clicked().connect( sigc::mem_fun(*this, &ColorItem::buttonClicked) );

        sigc::signal<void> type_signal_something;
*/

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
                          G_CALLBACK(handleButtonPress),
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
                          G_CALLBACK(dragBegin),
                          this );

        g_signal_connect( G_OBJECT(newBlot->gobj()),
                          "enter-notify-event",
                          G_CALLBACK(handleEnterNotify),
                          this);

        g_signal_connect( G_OBJECT(newBlot->gobj()),
                          "leave-notify-event",
                          G_CALLBACK(handleLeaveNotify),
                          this);

//         g_signal_connect( G_OBJECT(newBlot->gobj()),
//                           "drag-drop",
//                           G_CALLBACK(dragDropColorData),
//                           this);

        if ( def.isEditable() )
        {
//             gtk_drag_dest_set( GTK_WIDGET(newBlot->gobj()),
//                                GTK_DEST_DEFAULT_ALL,
//                                destColorTargets,
//                                G_N_ELEMENTS(destColorTargets),
//                                GdkDragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE) );


//             g_signal_connect( G_OBJECT(newBlot->gobj()),
//                               "drag-data-received",
//                               G_CALLBACK(_dropDataIn),
//                               this );
        }

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
            case ege::PaintDef::RGB: {
                Glib::ustring colorspec;
                if ( gradMap.find(this) == gradMap.end() ){
                    gchar c[64];
                    guint32 rgba = (def.getR() << 24) | (def.getG() << 16) | (def.getB() << 8) | 0xff;
                    sp_svg_write_color(c, sizeof(c), rgba);
                    colorspec = c;
                } else {
                    SPGradient* grad = gradMap[this];
                    colorspec = "url(#";
                    colorspec += grad->id;
                    colorspec += ")";
                }
                sp_repr_css_set_property( css, attrName, colorspec.c_str() );
                descr = secondary? _("Set stroke color from swatch") : _("Set fill color from swatch");
                break;
            }
        }
        sp_desktop_set_style(desktop, css);
        sp_repr_css_attr_unref(css);

        sp_document_done( sp_desktop_document(desktop), SP_VERB_DIALOG_SWATCHES, descr.c_str() );
    }
}

static char* trim( char* str ) {
    char* ret = str;
    while ( *str && (*str == ' ' || *str == '\t') ) {
        str++;
    }
    ret = str;
    while ( *str ) {
        str++;
    }
    str--;
    while ( str > ret && (( *str == ' ' || *str == '\t' ) || *str == '\r' || *str == '\n') ) {
        *str-- = 0;
    }
    return ret;
}

void skipWhitespace( char*& str ) {
    while ( *str == ' ' || *str == '\t' ) {
        str++;
    }
}

bool parseNum( char*& str, int& val ) {
    val = 0;
    while ( '0' <= *str && *str <= '9' ) {
        val = val * 10 + (*str - '0');
        str++;
    }
    bool retval = !(*str == 0 || *str == ' ' || *str == '\t' || *str == '\r' || *str == '\n');
    return retval;
}


static bool getBlock( std::string& dst, guchar ch, std::string const str )
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

void ColorItem::_wireMagicColors( void* p )
{
    JustForNow* onceMore = reinterpret_cast<JustForNow*>(p);
    if ( onceMore )
    {
        for ( std::vector<ColorItem*>::iterator it = onceMore->_colors.begin(); it != onceMore->_colors.end(); ++it )
        {
            std::string::size_type pos = (*it)->def.descr.find("*{");
            if ( pos != std::string::npos )
            {
                std::string subby = (*it)->def.descr.substr( pos + 2 );
                std::string::size_type endPos = subby.find("}*");
                if ( endPos != std::string::npos )
                {
                    subby.erase( endPos );
                    //g_message("FOUND MAGIC at '%s'", (*it)->def.descr.c_str());
                    //g_message("               '%s'", subby.c_str());

                    if ( subby.find('E') != std::string::npos )
                    {
                        (*it)->def.setEditable( true );
                    }

                    if ( subby.find('L') != std::string::npos )
                    {
                        (*it)->_isLive = true;
                    }

                    std::string part;
                    // Tint. index + 1 more val.
                    if ( getBlock( part, 'T', subby ) ) {
                        guint64 colorIndex = 0;
                        if ( popVal( colorIndex, part ) ) {
                            guint64 percent = 0;
                            if ( popVal( percent, part ) ) {
                                (*it)->_linkTint( *(onceMore->_colors[colorIndex]), percent );
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
                                (*it)->_linkTone( *(onceMore->_colors[colorIndex]), percent, grayLevel );
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


void _loadPaletteFile( gchar const *filename )
{
    char block[1024];
    FILE *f = Inkscape::IO::fopen_utf8name( filename, "r" );
    if ( f ) {
        char* result = fgets( block, sizeof(block), f );
        if ( result ) {
            if ( strncmp( "GIMP Palette", block, 12 ) == 0 ) {
                bool inHeader = true;
                bool hasErr = false;

                JustForNow *onceMore = new JustForNow();

                do {
                    result = fgets( block, sizeof(block), f );
                    block[sizeof(block) - 1] = 0;
                    if ( result ) {
                        if ( block[0] == '#' ) {
                            // ignore comment
                        } else {
                            char *ptr = block;
                            // very simple check for header versus entry
                            while ( *ptr == ' ' || *ptr == '\t' ) {
                                ptr++;
                            }
                            if ( (*ptr == 0) || (*ptr == '\r') || (*ptr == '\n') ) {
                                // blank line. skip it.
                            } else if ( '0' <= *ptr && *ptr <= '9' ) {
                                // should be an entry link
                                inHeader = false;
                                ptr = block;
                                Glib::ustring name("");
                                int r = 0;
                                int g = 0;
                                int b = 0;
                                skipWhitespace(ptr);
                                if ( *ptr ) {
                                    hasErr = parseNum(ptr, r);
                                    if ( !hasErr ) {
                                        skipWhitespace(ptr);
                                        hasErr = parseNum(ptr, g);
                                    }
                                    if ( !hasErr ) {
                                        skipWhitespace(ptr);
                                        hasErr = parseNum(ptr, b);
                                    }
                                    if ( !hasErr && *ptr ) {
                                        char* n = trim(ptr);
                                        if (n != NULL) {
                                            name = n;
                                        }
                                    }
                                    if ( !hasErr ) {
                                        // Add the entry now
                                        Glib::ustring nameStr(name);
                                        ColorItem* item = new ColorItem( r, g, b, nameStr );
                                        onceMore->_colors.push_back(item);
                                    }
                                } else {
                                    hasErr = true;
                                }
                            } else {
                                if ( !inHeader ) {
                                    // Hmmm... probably bad. Not quite the format we want?
                                    hasErr = true;
                                } else {
                                    char* sep = strchr(result, ':');
                                    if ( sep ) {
                                        *sep = 0;
                                        char* val = trim(sep + 1);
                                        char* name = trim(result);
                                        if ( *name ) {
                                            if ( strcmp( "Name", name ) == 0 )
                                            {
                                                onceMore->_name = val;
                                            }
                                            else if ( strcmp( "Columns", name ) == 0 )
                                            {
                                                gchar* endPtr = 0;
                                                guint64 numVal = g_ascii_strtoull( val, &endPtr, 10 );
                                                if ( (numVal == G_MAXUINT64) && (ERANGE == errno) ) {
                                                    // overflow
                                                } else if ( (numVal == 0) && (endPtr == val) ) {
                                                    // failed conversion
                                                } else {
                                                    onceMore->_prefWidth = numVal;
                                                }
                                            }
                                        } else {
                                            // error
                                            hasErr = true;
                                        }
                                    } else {
                                        // error
                                        hasErr = true;
                                    }
                                }
                            }
                        }
                    }
                } while ( result && !hasErr );
                if ( !hasErr ) {
                    possible.push_back(onceMore);
#if ENABLE_MAGIC_COLORS
                    ColorItem::_wireMagicColors( onceMore );
#endif // ENABLE_MAGIC_COLORS
                } else {
                    delete onceMore;
                }
            }
        }

        fclose(f);
    }
}

static void loadEmUp()
{
    static bool beenHere = false;
    if ( !beenHere ) {
        beenHere = true;

        std::list<gchar *> sources;
        sources.push_back( profile_path("palettes") );
        sources.push_back( g_strdup(INKSCAPE_PALETTESDIR) );
        sources.push_back( g_strdup(CREATE_PALETTESDIR) );

        // Use this loop to iterate through a list of possible document locations.
        while (!sources.empty()) {
            gchar *dirname = sources.front();

            if ( Inkscape::IO::file_test( dirname, G_FILE_TEST_EXISTS )
                && Inkscape::IO::file_test( dirname, G_FILE_TEST_IS_DIR )) {
                GError *err = 0;
                GDir *directory = g_dir_open(dirname, 0, &err);
                if (!directory) {
                    gchar *safeDir = Inkscape::IO::sanitizeString(dirname);
                    g_warning(_("Palettes directory (%s) is unavailable."), safeDir);
                    g_free(safeDir);
                } else {
                    gchar *filename = 0;
                    while ((filename = (gchar *)g_dir_read_name(directory)) != NULL) {
                        gchar* lower = g_ascii_strdown( filename, -1 );
//                        if ( g_str_has_suffix(lower, ".gpl") ) {
                            gchar* full = g_build_filename(dirname, filename, NULL);
                            if ( !Inkscape::IO::file_test( full, G_FILE_TEST_IS_DIR ) ) {
                                _loadPaletteFile(full);
                            }
                            g_free(full);
//                      }
                        g_free(lower);
                    }
                    g_dir_close(directory);
                }
            }

            // toss the dirname
            g_free(dirname);
            sources.pop_front();
        }
    }
}









SwatchesPanel& SwatchesPanel::getInstance()
{
    return *new SwatchesPanel();
}


/**
 * Constructor
 */
SwatchesPanel::SwatchesPanel(gchar const* prefsPath) :
    Inkscape::UI::Widget::Panel("", prefsPath, SP_VERB_DIALOG_SWATCHES, "", true),
    _holder(0),
    _clear(0),
    _remove(0),
    _currentIndex(0),
    _currentDesktop(0),
    _currentDocument(0),
    _ptr(0)
{
    Gtk::RadioMenuItem* hotItem = 0;
    _holder = new PreviewHolder();
    _clear = new ColorItem( ege::PaintDef::CLEAR );
    _clear->ptr = this;
    _remove = new ColorItem( ege::PaintDef::NONE );
    _remove->ptr = this;
    {
        JustForNow *docPalette = new JustForNow();

        docPalette->_name = "Auto";
        possible.push_back(docPalette);

        _ptr = docPalette;
    }
    loadEmUp();
    if ( !possible.empty() ) {
        JustForNow* first = 0;
        int index = 0;
        Glib::ustring targetName;
        if ( !_prefs_path.empty() ) {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            targetName = prefs->getString(_prefs_path + "/palette");
            if (!targetName.empty()) {
                for ( std::vector<JustForNow*>::iterator iter = possible.begin(); iter != possible.end(); ++iter ) {
                    if ( (*iter)->_name == targetName ) {
                        first = *iter;
                        break;
                    }
                    index++;
                }
            }
        }

        if ( !first ) {
            first = possible.front();
            _currentIndex = 0;
        } else {
            _currentIndex = index;
        }

        _rebuild();

        Gtk::RadioMenuItem::Group groupOne;

        int i = 0;
        for ( std::vector<JustForNow*>::iterator it = possible.begin(); it != possible.end(); it++ ) {
            JustForNow* curr = *it;
            Gtk::RadioMenuItem* single = manage(new Gtk::RadioMenuItem(groupOne, curr->_name));
            if ( curr == first ) {
                hotItem = single;
            }
            _regItem( single, 3, i );
            i++;
        }
    }


    _getContents()->pack_start(*_holder, Gtk::PACK_EXPAND_WIDGET);
    _setTargetFillable(_holder);

    show_all_children();

    restorePanelPrefs();
    if ( hotItem ) {
        hotItem->set_active();
    }
}

SwatchesPanel::~SwatchesPanel()
{
    _documentConnection.disconnect();
    _resourceConnection.disconnect();

    if ( _clear ) {
        delete _clear;
    }
    if ( _remove ) {
        delete _remove;
    }
    if ( _holder ) {
        delete _holder;
    }
}

void SwatchesPanel::setOrientation( Gtk::AnchorType how )
{
    // Must call the parent class or bad things might happen
    Inkscape::UI::Widget::Panel::setOrientation( how );

    if ( _holder )
    {
        _holder->setOrientation( Gtk::ANCHOR_SOUTH );
    }
}

void SwatchesPanel::setDesktop( SPDesktop* desktop )
{
    if ( desktop != _currentDesktop ) {
        if ( _currentDesktop ) {
            _documentConnection.disconnect();
        }

        _currentDesktop = desktop;

        if ( desktop ) {
            sigc::bound_mem_functor1<void, Inkscape::UI::Dialogs::SwatchesPanel, SPDocument*> first = sigc::mem_fun(*this, &SwatchesPanel::_setDocument);

            sigc::slot<void, SPDocument*> base2 = first;

            sigc::slot<void,SPDesktop*, SPDocument*> slot2 = sigc::hide<0>( base2 );
            _documentConnection = desktop->connectDocumentReplaced( slot2 );

            _setDocument( desktop->doc() );
        } else {
            _setDocument(0);
        }
    }
}

void SwatchesPanel::_setDocument( SPDocument *document )
{
    if ( document != _currentDocument ) {
        if ( _currentDocument ) {
            _resourceConnection.disconnect();
        }
        _currentDocument = document;
        if ( _currentDocument ) {
            _resourceConnection = sp_document_resources_changed_connect(document,
                                                                        "gradient",
                                                                        sigc::mem_fun(*this, &SwatchesPanel::handleGradientsChange));
        }

        handleGradientsChange();
    }
}

void SwatchesPanel::handleGradientsChange()
{
    std::vector<SPGradient*> newList;

    const GSList *gradients = sp_document_get_resource_list(_currentDocument, "gradient");
    for (const GSList *item = gradients; item; item = item->next) {
        SPGradient* grad = SP_GRADIENT(item->data);
        if ( grad->has_stops ) {
            newList.push_back(SP_GRADIENT(item->data));
        }
    }

    if ( _ptr ) {
        JustForNow *docPalette = reinterpret_cast<JustForNow *>(_ptr);
        // TODO delete pointed to objects
        docPalette->_colors.clear();
        if ( !newList.empty() ) {
            for ( std::vector<SPGradient*>::iterator it = newList.begin(); it != newList.end(); ++it )
            {
                SPGradient* grad = *it;
                if ( grad->repr->attribute("osb:paint") ) {
                    sp_gradient_ensure_vector( grad );
                    SPGradientStop first = grad->vector.stops[0];
                    SPColor color = first.color;
                    guint32 together = color.toRGBA32(first.opacity);

                    // At the moment we can't trust the count of 1 vs 2 stops.
                    SPGradientStop second = (*it)->vector.stops[1];
                    SPColor color2 = second.color;
                    guint32 together2 = color2.toRGBA32(second.opacity);

                    if ( (grad->vector.stops.size() <= 2) && (together == together2) ) {
                        // Treat as solid-color
                        Glib::ustring name( grad->id );
                        unsigned int r = SP_RGBA32_R_U(together);
                        unsigned int g = SP_RGBA32_G_U(together);
                        unsigned int b = SP_RGBA32_B_U(together);
                        ColorItem* item = new ColorItem( r, g, b, name );
                        item->ptr = this;
                        docPalette->_colors.push_back(item);
                        gradMap[item] = grad;
                    } else {
                        // Treat as gradient
                        Glib::ustring name( grad->id );
                        unsigned int r = SP_RGBA32_R_U(together);
                        unsigned int g = SP_RGBA32_G_U(together);
                        unsigned int b = SP_RGBA32_B_U(together);
                        ColorItem* item = new ColorItem( r, g, b, name );
                        item->ptr = this;
                        docPalette->_colors.push_back(item);

#define VBLOCK 16
                        gint width = 128;
                        gint height = VBLOCK;
                        guchar* px = g_new( guchar, 3 * height * width );
                        nr_render_checkerboard_rgb( px, width, VBLOCK, 3 * width, 0, 0 );

                        sp_gradient_render_vector_block_rgb( grad,
                                                             px, width, height, 3 * width,
                                                             0, width, TRUE );

                        previewMap[item] = px;
                        gradMap[item] = grad;
                    }
                }
            }
        }
        JustForNow* curr = possible[_currentIndex];
        if (curr == docPalette) {
            _rebuild();
        }
    }
}

void SwatchesPanel::_handleAction( int setId, int itemId )
{
    switch( setId ) {
        case 3:
        {
            if ( itemId >= 0 && itemId < static_cast<int>(possible.size()) ) {
                _currentIndex = itemId;

                if ( !_prefs_path.empty() ) {
                    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                    prefs->setString(_prefs_path + "/palette", possible[_currentIndex]->_name);
                }

                _rebuild();
            }
        }
        break;
    }
}

void SwatchesPanel::_rebuild()
{
    JustForNow* curr = possible[_currentIndex];
    _holder->clear();

    if ( curr->_prefWidth > 0 ) {
        _holder->setColumnPref( curr->_prefWidth );
    }
    _holder->freezeUpdates();
    // TODO restore once 'clear' works _holder->addPreview(_clear);
    _holder->addPreview(_remove);
    for ( std::vector<ColorItem*>::iterator it = curr->_colors.begin(); it != curr->_colors.end(); it++ ) {
        _holder->addPreview(*it);
    }
    _holder->thawUpdates();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
