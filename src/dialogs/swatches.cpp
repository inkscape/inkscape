/*
 * A simple panel for color swatches
 *
 * Authors:
 *   Jon A. Cruz
 *
 * Copyright (C) 2005 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtkdialog.h> //for GTK_RESPONSE* types
#include <gtk/gtkdnd.h>

#include <glibmm/i18n.h>
#include "inkscape.h"
#include "document.h"
#include "desktop-handles.h"
#include "extension/db.h"
#include "inkscape.h"
#include "svg/svg.h"
#include "desktop-style.h"
#include "io/sys.h"
#include "path-prefix.h"
#include "swatches.h"

#include "eek-preview.h"

namespace Inkscape {
namespace UI {
namespace Dialogs {

SwatchesPanel* SwatchesPanel::instance = 0;


ColorItem::ColorItem( unsigned int r, unsigned int g, unsigned int b, Glib::ustring& name ) :
    _r(r),
    _g(g),
    _b(b),
    _name(name)
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
        _r = other._r;
        _g = other._g;
        _b = other._b;
        _name = other._name;
    }
    return *this;
}

typedef enum {
    XCOLOR_DATA = 0,
    TEXT_DATA
} colorFlavorType;

static const GtkTargetEntry color_entries[] = {
    {"application/x-color", 0, XCOLOR_DATA},
    {"text/plain", 0, TEXT_DATA},
};

static void dragGetColorData( GtkWidget *widget,
                              GdkDragContext *drag_context,
                              GtkSelectionData *data,
                              guint info,
                              guint time,
                              gpointer user_data)
{
    static GdkAtom typeXColor = gdk_atom_intern("application/x-color", FALSE);
    static GdkAtom typeText = gdk_atom_intern("text/plain", FALSE);

    ColorItem* item = reinterpret_cast<ColorItem*>(user_data);
    if ( info == 1 ) {
        gchar* tmp = g_strdup_printf("#%02x%02x%02x", item->_r, item->_g, item->_b);

        gtk_selection_data_set( data,
                                typeText,
                                8, // format
                                (guchar*)tmp,
                                strlen((const char*)tmp) + 1);
        g_free(tmp);
        tmp = 0;
    } else {
        guchar tmp[8];
        tmp[0] = item->_r;
        tmp[1] = item->_r;
        tmp[2] = item->_g;
        tmp[3] = item->_g;
        tmp[4] = item->_b;
        tmp[5] = item->_b;
        tmp[6] = 0x0ff;
        tmp[7] = 0x0ff;
        gtk_selection_data_set( data,
                                typeXColor,
                                8, // format
                                tmp,
                                (3+1) * 2);
    }
}

//"drag-drop"
gboolean dragDropColorData( GtkWidget *widget,
                            GdkDragContext *drag_context,
                            gint x,
                            gint y,
                            guint time,
                            gpointer user_data)
{
// TODO finish
    return TRUE;
}

static void bouncy( GtkWidget* widget, gpointer callback_data ) {
    ColorItem* item = reinterpret_cast<ColorItem*>(callback_data);
    if ( item ) {
        item->buttonClicked(false);
    }
}

static void bouncy2( GtkWidget* widget, gint arg1, gpointer callback_data ) {
    ColorItem* item = reinterpret_cast<ColorItem*>(callback_data);
    if ( item ) {
        item->buttonClicked(true);
    }
}

Gtk::Widget* ColorItem::getPreview(PreviewStyle style, ViewType view, Gtk::BuiltinIconSize size)
{
    Gtk::Widget* widget = 0;
    if ( style == PREVIEW_STYLE_BLURB ) {
        Gtk::Label *lbl = new Gtk::Label(_name);
        lbl->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
        widget = lbl;
    } else {
        Glib::ustring blank("          ");
        if ( size == Gtk::ICON_SIZE_MENU ) {
            blank = " ";
        }

        GtkWidget* eekWidget = eek_preview_new();
        EekPreview * preview = EEK_PREVIEW(eekWidget);
        Gtk::Widget* newBlot = Glib::wrap(eekWidget);

        eek_preview_set_color( preview, (_r << 8)|_r, (_g << 8)|_g, (_b << 8)|_b);

        eek_preview_set_details( preview, (::PreviewStyle)style, (::ViewType)view, (::GtkIconSize)size );

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

        tips.set_tip((*newBlot), _name);

/*
        newBlot->signal_clicked().connect( sigc::mem_fun(*this, &ColorItem::buttonClicked) );

        sigc::signal<void> type_signal_something;
*/
        g_signal_connect( G_OBJECT(newBlot->gobj()),
                          "clicked",
                          G_CALLBACK(bouncy),
                          this);

        g_signal_connect( G_OBJECT(newBlot->gobj()),
                          "alt-clicked",
                          G_CALLBACK(bouncy2),
                          this);

        gtk_drag_source_set( GTK_WIDGET(newBlot->gobj()),
                             GDK_BUTTON1_MASK,
                             color_entries,
                             G_N_ELEMENTS(color_entries),
                             GdkDragAction(GDK_ACTION_MOVE | GDK_ACTION_COPY) );

        g_signal_connect( G_OBJECT(newBlot->gobj()),
                          "drag-data-get",
                          G_CALLBACK(dragGetColorData),
                          this);

        g_signal_connect( G_OBJECT(newBlot->gobj()),
                          "drag-drop",
                          G_CALLBACK(dragDropColorData),
                          this);

        widget = newBlot;
    }

    return widget;
}

void ColorItem::buttonClicked(bool secondary)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop) {
        char const * attrName = secondary ? "stroke" : "fill";
        guint32 rgba = (_r << 24) | (_g << 16) | (_b << 8) | 0xff;
        gchar c[64];
        sp_svg_write_color(c, 64, rgba);

        SPCSSAttr *css = sp_repr_css_attr_new();
        sp_repr_css_set_property( css, attrName, c );
        sp_desktop_set_style(desktop, css);

        sp_repr_css_attr_unref(css);
        sp_document_done (SP_DT_DOCUMENT (desktop));
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
    while ( str > ret && ( *str == ' ' || *str == '\t' ) || *str == '\r' || *str == '\n' ) {
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


class JustForNow
{
public:
    Glib::ustring _name;
    std::vector<ColorItem*> _colors;
};

static std::vector<JustForNow*> possible;

static void loadPaletteFile( gchar const *filename )
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
                            if ( *ptr == 0 ) {
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
                                            if ( strcmp( "Name", name ) == 0 ) {
                                                onceMore->_name = val;
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

        // Use this loop to iterate through a list of possible document locations.
        while (!sources.empty()) {
            gchar *dirname = sources.front();

            if ( Inkscape::IO::file_test( dirname, (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR) ) ) {
                GError *err = 0;
                GDir *directory = g_dir_open(dirname, 0, &err);
                if (!directory) {
                    gchar *safeDir = Inkscape::IO::sanitizeString(dirname);
                    g_warning(_("Palettes directory (%s) is unavailable."), safeDir);
                    g_free(safeDir);
                } else {
                    gchar *filename = 0;
                    while ((filename = (gchar *)g_dir_read_name(directory)) != NULL) {
                        gchar* full = g_build_filename(dirname, filename, NULL);
                        if ( !Inkscape::IO::file_test( full, (GFileTest)(G_FILE_TEST_IS_DIR ) ) ) {
                            loadPaletteFile(full);
                        }
                        g_free(full);
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
    if ( !instance ) {
        instance = new SwatchesPanel();
    }

    return *instance;
}



/**
 * Constructor
 */
SwatchesPanel::SwatchesPanel() :
    Inkscape::UI::Widget::Panel ("dialogs.swatches"),
    _holder(0)
{
    _holder = new PreviewHolder();
    loadEmUp();

    if ( !possible.empty() ) {
        JustForNow* first = possible.front();
        for ( std::vector<ColorItem*>::iterator it = first->_colors.begin(); it != first->_colors.end(); it++ ) {
            _holder->addPreview(*it);
        }

        Gtk::RadioMenuItem::Group groupOne;
        int i = 0;
        for ( std::vector<JustForNow*>::iterator it = possible.begin(); it != possible.end(); it++ ) {
            JustForNow* curr = *it;
            Gtk::RadioMenuItem* single = manage(new Gtk::RadioMenuItem(groupOne, curr->_name));
            _regItem( single, 3, i );
            i++;
        }

    }


    pack_start(*_holder, Gtk::PACK_EXPAND_WIDGET);
    _setTargetFillable(_holder);

    show_all_children();

    restorePanelPrefs();
}

SwatchesPanel::~SwatchesPanel()
{
}

void SwatchesPanel::_handleAction( int setId, int itemId )
{
    switch( setId ) {
        case 3:
        {
            if ( itemId >= 0 && itemId < static_cast<int>(possible.size()) ) {
                _holder->clear();
                JustForNow* curr = possible[itemId];
                for ( std::vector<ColorItem*>::iterator it = curr->_colors.begin(); it != curr->_colors.end(); it++ ) {
                    _holder->addPreview(*it);
                }
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
