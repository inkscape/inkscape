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

#include <gtk/gtkdialog.h> //for GTK_RESPONSE* types
#include <gtk/gtkdnd.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkseparatormenuitem.h>

#include <glibmm/i18n.h>
#include <gdkmm/pixbuf.h>
#include "inkscape.h"
#include "desktop.h"
#include "message-context.h"
#include "document.h"
#include "desktop-handles.h"
#include "extension/db.h"
#include "inkscape.h"
#include "svg/svg-color.h"
#include "desktop-style.h"
#include "io/sys.h"
#include "path-prefix.h"
#include "swatches.h"
#include "sp-item.h"
#include "preferences.h"

#include "eek-preview.h"

namespace Inkscape {
namespace UI {
namespace Dialogs {

ColorItem::ColorItem() : _isRemove(true){};
ColorItem::ColorItem( unsigned int r, unsigned int g, unsigned int b, Glib::ustring& name ) :
    def( r, g, b, name ),
    _isRemove(false),
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



typedef enum {
    APP_X_INKY_COLOR_ID = 0,
    APP_X_INKY_COLOR = 0,
    APP_X_COLOR,
    TEXT_DATA
} colorFlavorType;

//TODO: warning: deprecated conversion from string constant to ‘gchar*’
//
//Turn out to be warnings that we should probably leave in place. The
// pointers/types used need to be read-only. So until we correct the using
// code, those warnings are actually desired. They say "Hey! Fix this". We
// definitely don't want to hide/ignore them. --JonCruz
static const GtkTargetEntry sourceColorEntries[] = {
#if ENABLE_MAGIC_COLORS
//    {"application/x-inkscape-color-id", GTK_TARGET_SAME_APP, APP_X_INKY_COLOR_ID},
    {"application/x-inkscape-color", 0, APP_X_INKY_COLOR},
#endif // ENABLE_MAGIC_COLORS
    {"application/x-color", 0, APP_X_COLOR},
    {"text/plain", 0, TEXT_DATA},
};

void ColorItem::_dragGetColorData( GtkWidget *widget,
                                   GdkDragContext *drag_context,
                                   GtkSelectionData *data,
                                   guint info,
                                   guint time,
                                   gpointer user_data)
{
    (void)widget;
    (void)drag_context;
    (void)time;
    static GdkAtom typeXColor = gdk_atom_intern("application/x-color", FALSE);
    static GdkAtom typeText = gdk_atom_intern("text/plain", FALSE);

    ColorItem* item = reinterpret_cast<ColorItem*>(user_data);
    if ( info == TEXT_DATA ) {
        gchar* tmp = g_strdup_printf("#%02x%02x%02x", item->def.getR(), item->def.getG(), item->def.getB() );

        gtk_selection_data_set( data,
                                typeText,
                                8, // format
                                (guchar*)tmp,
                                strlen((const char*)tmp) + 1);
        g_free(tmp);
        tmp = 0;
    } else if ( info == APP_X_INKY_COLOR ) {
        Glib::ustring paletteName;

        // Find where this thing came from
        bool found = false;
        int index = 0;
        for ( std::vector<JustForNow*>::iterator it = possible.begin(); it != possible.end() && !found; ++it ) {
            JustForNow* curr = *it;
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

//         if ( found ) {
//             g_message("Found the color at entry %d in palette '%s'", index, paletteName.c_str() );
//         } else {
//             g_message("Unable to find the color");
//         }
        int itemCount = 4 + 2 + 1 + paletteName.length();

        guint16* tmp = new guint16[itemCount];
        tmp[0] = (item->def.getR() << 8) | item->def.getR();
        tmp[1] = (item->def.getG() << 8) | item->def.getG();
        tmp[2] = (item->def.getB() << 8) | item->def.getB();
        tmp[3] = 0xffff;
        tmp[4] = (item->_isLive || !item->_listeners.empty() || (item->_linkSrc != 0) ) ? 1 : 0;

        tmp[5] = index;
        tmp[6] = paletteName.length();
        for ( unsigned int i = 0; i < paletteName.length(); i++ ) {
            tmp[7 + i] = paletteName[i];
        }
        gtk_selection_data_set( data,
                                typeXColor,
                                16, // format
                                reinterpret_cast<const guchar*>(tmp),
                                itemCount * 2);
        delete[] tmp;
    } else {
        guint16 tmp[4];
        tmp[0] = (item->def.getR() << 8) | item->def.getR();
        tmp[1] = (item->def.getG() << 8) | item->def.getG();
        tmp[2] = (item->def.getB() << 8) | item->def.getB();
        tmp[3] = 0xffff;
        gtk_selection_data_set( data,
                                typeXColor,
                                16, // format
                                reinterpret_cast<const guchar*>(tmp),
                                (3+1) * 2);
    }
}

static void dragBegin( GtkWidget *widget, GdkDragContext* dc, gpointer data )
{
    (void)widget;
    ColorItem* item = reinterpret_cast<ColorItem*>(data);
    if ( item )
    {
        if (item->isRemove()){
            GError *error = NULL;
            gchar *filepath = (gchar *) g_strdup_printf("%s/remove-color.png", INKSCAPE_PIXMAPDIR);
            gsize bytesRead = 0;
            gsize bytesWritten = 0;
            gchar *localFilename = g_filename_from_utf8( filepath,
                                                 -1,
                                                 &bytesRead,
                                                 &bytesWritten,
                                                 &error);
            GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file_at_scale(localFilename, 32, 24, FALSE, &error);
            g_free(localFilename);
            g_free(filepath);
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

static void handleClick( GtkWidget* widget, gpointer callback_data ) {
    (void)widget;
    ColorItem* item = reinterpret_cast<ColorItem*>(callback_data);
    if ( item ) {
        item->buttonClicked(false);
    }
}

static void handleSecondaryClick( GtkWidget* widget, gint arg1, gpointer callback_data ) {
    (void)widget;
    (void)arg1;
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
static ColorItem* bounceTarget = 0;

static void redirClick( GtkMenuItem *menuitem, gpointer user_data )
{
    (void)user_data;
    if ( bounceTarget ) {
        handleClick( GTK_WIDGET(menuitem), bounceTarget );
    }
}

static void redirSecondaryClick( GtkMenuItem *menuitem, gpointer user_data )
{
    (void)user_data;
    if ( bounceTarget ) {
        handleSecondaryClick( GTK_WIDGET(menuitem), 0, bounceTarget );
    }
}

static gboolean handleButtonPress( GtkWidget* widget, GdkEventButton* event, gpointer user_data)
{
    (void)widget;
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

            gtk_widget_show_all(popupMenu);
        }

        ColorItem* item = reinterpret_cast<ColorItem*>(user_data);
        if ( item ) {
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

//TODO: warning: deprecated conversion from string constant to ‘gchar*’
//
//Turn out to be warnings that we should probably leave in place. The
// pointers/types used need to be read-only. So until we correct the using
// code, those warnings are actually desired. They say "Hey! Fix this". We
// definitely don't want to hide/ignore them. --JonCruz
static const GtkTargetEntry destColorTargets[] = {
#if ENABLE_MAGIC_COLORS
//    {"application/x-inkscape-color-id", GTK_TARGET_SAME_APP, APP_X_INKY_COLOR_ID},
    {"application/x-inkscape-color", 0, APP_X_INKY_COLOR},
#endif // ENABLE_MAGIC_COLORS
    {"application/x-color", 0, APP_X_COLOR},
};

#include "color.h" // for SP_RGBA32_U_COMPOSE

void ColorItem::_dropDataIn( GtkWidget *widget,
                             GdkDragContext *drag_context,
                             gint x, gint y,
                             GtkSelectionData *data,
                             guint info,
                             guint event_time,
                             gpointer user_data)
{
    (void)widget;
    (void)drag_context;
    (void)x;
    (void)y;
    (void)event_time;
//     g_message("    droppy droppy   %d", info);
     switch (info) {
         case APP_X_INKY_COLOR:
         {
             if ( data->length >= 8 ) {
                 // Careful about endian issues.
                 guint16* dataVals = (guint16*)data->data;
                 if ( user_data ) {
                     ColorItem* item = reinterpret_cast<ColorItem*>(user_data);
                     if ( item->def.isEditable() ) {
                         // Shove on in the new value
                         item->def.setRGB( 0x0ff & (dataVals[0] >> 8), 0x0ff & (dataVals[1] >> 8), 0x0ff & (dataVals[2] >> 8) );
                     }
                 }
             }
             break;
         }
         case APP_X_COLOR:
         {
             if ( data->length == 8 ) {
                 // Careful about endian issues.
                 guint16* dataVals = (guint16*)data->data;
//                  {
//                      gchar c[64] = {0};
//                      sp_svg_write_color( c, 64,
//                                          SP_RGBA32_U_COMPOSE(
//                                              0x0ff & (dataVals[0] >> 8),
//                                              0x0ff & (dataVals[1] >> 8),
//                                              0x0ff & (dataVals[2] >> 8),
//                                              0xff // can't have transparency in the color itself
//                                              //0x0ff & (data->data[3] >> 8),
//                                              ));
//                  }
                 if ( user_data ) {
                     ColorItem* item = reinterpret_cast<ColorItem*>(user_data);
                     if ( item->def.isEditable() ) {
                         // Shove on in the new value
                         item->def.setRGB( 0x0ff & (dataVals[0] >> 8), 0x0ff & (dataVals[1] >> 8), 0x0ff & (dataVals[2] >> 8) );
                     }
                 }
             }
             break;
         }
         default:
             g_message("unknown drop type");
     }

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

        eek_preview_set_color( preview, (def.getR() << 8) | def.getR(), (def.getG() << 8) | def.getG(), (def.getB() << 8) | def.getB());
        preview->_isRemove = _isRemove;

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

        gtk_drag_source_set( GTK_WIDGET(newBlot->gobj()),
                             GDK_BUTTON1_MASK,
                             sourceColorEntries,
                             G_N_ELEMENTS(sourceColorEntries),
                             GdkDragAction(GDK_ACTION_MOVE | GDK_ACTION_COPY) );

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
            gtk_drag_dest_set( GTK_WIDGET(newBlot->gobj()),
                               GTK_DEST_DEFAULT_ALL,
                               destColorTargets,
                               G_N_ELEMENTS(destColorTargets),
                               GdkDragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE) );


            g_signal_connect( G_OBJECT(newBlot->gobj()),
                              "drag-data-received",
                              G_CALLBACK(_dropDataIn),
                              this );
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
    if (!desktop) return;
    char const * attrName = secondary ? "stroke" : "fill";

    gchar c[64];
    if (!_isRemove){
        guint32 rgba = (def.getR() << 24) | (def.getG() << 16) | (def.getB() << 8) | 0xff;
        sp_svg_write_color(c, sizeof(c), rgba);
    }

    SPCSSAttr *css = sp_repr_css_attr_new();
    sp_repr_css_set_property( css, attrName, _isRemove ? "none" : c );
    sp_desktop_set_style(desktop, css);
    sp_repr_css_attr_unref(css);

    if (_isRemove){
        sp_document_done (sp_desktop_document (desktop), SP_VERB_DIALOG_SWATCHES, 
                      secondary? _("Remove stroke color") : _("Remove fill color"));
    } else {
        sp_document_done (sp_desktop_document (desktop), SP_VERB_DIALOG_SWATCHES, 
                      secondary? _("Set stroke color from swatch") : _("Set fill color from swatch"));
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
    _holder(0)
{
    Gtk::RadioMenuItem* hotItem = 0;
    _holder = new PreviewHolder();
    _remove = new ColorItem();
    loadEmUp();
    if ( !possible.empty() ) {
        JustForNow* first = 0;
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
                }
            }
        }

        if ( !first ) {
            first = possible.front();
        }

        if ( first->_prefWidth > 0 ) {
            _holder->setColumnPref( first->_prefWidth );
        }
        _holder->freezeUpdates();
        _holder->addPreview(_remove);
        for ( std::vector<ColorItem*>::iterator it = first->_colors.begin(); it != first->_colors.end(); it++ ) {
            _holder->addPreview(*it);
        }
        _holder->thawUpdates();

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
    if (_remove) delete _remove;
    if (_holder) delete _holder;
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

void SwatchesPanel::_handleAction( int setId, int itemId )
{
    switch( setId ) {
        case 3:
        {
            if ( itemId >= 0 && itemId < static_cast<int>(possible.size()) ) {
                _holder->clear();
                JustForNow* curr = possible[itemId];

                if ( !_prefs_path.empty() ) {
                    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                    prefs->setString(_prefs_path + "/palette", curr->_name);
                }

                if ( curr->_prefWidth > 0 ) {
                    _holder->setColumnPref( curr->_prefWidth );
                }
                _holder->freezeUpdates();
                _holder->addPreview(_remove);
                for ( std::vector<ColorItem*>::iterator it = curr->_colors.begin(); it != curr->_colors.end(); it++ ) {
                    _holder->addPreview(*it);
                }
                _holder->thawUpdates();
            }
        }
        break;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
