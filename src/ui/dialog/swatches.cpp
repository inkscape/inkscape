/**
 * @file
 * Color swatches dialog.
 */
/* Authors:
 *   Jon A. Cruz
 *   John Bintz
 *   Abhishek Sharma
 *
 * Copyright (C) 2005 Jon A. Cruz
 * Copyright (C) 2008 John Bintz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <errno.h>
#include <map>
#include <algorithm>
#include <set>

#include "swatches.h"
#include <gtkmm/radiomenuitem.h>

#include <glibmm/i18n.h>
#include <glibmm/main.h>
#include <glibmm/timer.h>
#include <gdkmm/pixbuf.h>

#include "color-item.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-style.h"
#include "document.h"
#include "document-private.h"
#include "document-undo.h"
#include "extension/db.h"
#include "inkscape.h"
#include "inkscape.h"
#include "io/sys.h"
#include "io/resource.h"
#include "message-context.h"
#include "path-prefix.h"
#include "preferences.h"
#include "sp-item.h"
#include "sp-gradient.h"
#include "sp-gradient-vector.h"
#include "style.h"
#include "ui/previewholder.h"
#include "widgets/desktop-widget.h"
#include "widgets/gradient-vector.h"
#include "widgets/eek-preview.h"
#include "display/cairo-utils.h"
#include "sp-gradient-reference.h"
#include "dialog-manager.h"
#include "selection.h"
#include "verbs.h"
#include "gradient-chemistry.h"
#include "helper/action.h"
#include "helper/action-context.h"

namespace Inkscape {
namespace UI {
namespace Dialogs {

#define VBLOCK 16
#define PREVIEW_PIXBUF_WIDTH 128

void _loadPaletteFile( gchar const *filename, gboolean user=FALSE );

std::list<SwatchPage*> userSwatchPages;
std::list<SwatchPage*> systemSwatchPages;
static std::map<SPDocument*, SwatchPage*> docPalettes;
static std::vector<DocTrack*> docTrackings;
static std::map<SwatchesPanel*, SPDocument*> docPerPanel;


class SwatchesPanelHook : public SwatchesPanel
{
public:
    static void convertGradient( GtkMenuItem *menuitem, gpointer userData );
    static void deleteGradient( GtkMenuItem *menuitem, gpointer userData );
};

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

static GtkWidget* popupMenu = 0;
static GtkWidget *popupSubHolder = 0;
static GtkWidget *popupSub = 0;
static std::vector<Glib::ustring> popupItems;
static std::vector<GtkWidget*> popupExtras;
static ColorItem* bounceTarget = 0;
static SwatchesPanel* bouncePanel = 0;

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

static void editGradientImpl( SPDesktop* desktop, SPGradient* gr )
{
    if ( gr ) {
        bool shown = false;
        if ( desktop && desktop->doc() ) {
            Inkscape::Selection *selection = desktop->getSelection();
            GSList const *items = selection->itemList();
            if (items) {
                SPStyle *query = sp_style_new( desktop->doc() );
                int result = objects_query_fillstroke(const_cast<GSList *>(items), query, true);
                if ( (result == QUERY_STYLE_MULTIPLE_SAME) || (result == QUERY_STYLE_SINGLE) ) {
                    // could be pertinent
                    if (query->fill.isPaintserver()) {
                        SPPaintServer* server = query->getFillPaintServer();
                        if ( SP_IS_GRADIENT(server) ) {
                            SPGradient* grad = SP_GRADIENT(server);
                            if ( grad->isSwatch() && grad->getId() == gr->getId()) {
                                desktop->_dlg_mgr->showDialog("FillAndStroke");
                                shown = true;
                            }
                        }
                    }
                }
                sp_style_unref(query);
            }
        }

        if (!shown) {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            if (prefs->getBool("/dialogs/gradienteditor/showlegacy", false)) {
                // Legacy gradient dialog
                GtkWidget *dialog = sp_gradient_vector_editor_new( gr );
                gtk_widget_show( dialog );
            } else {
                // Invoke the gradient tool
                Inkscape::Verb *verb = Inkscape::Verb::get( SP_VERB_CONTEXT_GRADIENT );
                if ( verb ) {
                    SPAction *action = verb->get_action( Inkscape::ActionContext( ( Inkscape::UI::View::View * ) SP_ACTIVE_DESKTOP ) );
                    if ( action ) {
                        sp_action_perform( action, NULL );
                    }
                }
            }
        }
    }
}

static void editGradient( GtkMenuItem */*menuitem*/, gpointer /*user_data*/ )
{
    if ( bounceTarget ) {
        SwatchesPanel* swp = bouncePanel;
        SPDesktop* desktop = swp ? swp->getDesktop() : 0;
        SPDocument *doc = desktop ? desktop->doc() : 0;
        if (doc) {
            std::string targetName(bounceTarget->def.descr);
            const GSList *gradients = doc->getResourceList("gradient");
            for (const GSList *item = gradients; item; item = item->next) {
                SPGradient* grad = SP_GRADIENT(item->data);
                if ( targetName == grad->getId() ) {
                    editGradientImpl( desktop, grad );
                    break;
                }
            }
        }
    }
}

void SwatchesPanelHook::convertGradient( GtkMenuItem * /*menuitem*/, gpointer userData )
{
    if ( bounceTarget ) {
        SwatchesPanel* swp = bouncePanel;
        SPDesktop* desktop = swp ? swp->getDesktop() : 0;
        SPDocument *doc = desktop ? desktop->doc() : 0;
        gint index = GPOINTER_TO_INT(userData);
        if ( doc && (index >= 0) && (static_cast<guint>(index) < popupItems.size()) ) {
            Glib::ustring targetName = popupItems[index];

            const GSList *gradients = doc->getResourceList("gradient");
            for (const GSList *item = gradients; item; item = item->next) {
                SPGradient* grad = SP_GRADIENT(item->data);
                if ( targetName == grad->getId() ) {
                    grad->setSwatch();
                    DocumentUndo::done(doc, SP_VERB_CONTEXT_GRADIENT,
                                       _("Add gradient stop"));
                    break;
                }
            }
        }
    }
}

void SwatchesPanelHook::deleteGradient( GtkMenuItem */*menuitem*/, gpointer /*userData*/ )
{
    if ( bounceTarget ) {
        SwatchesPanel* swp = bouncePanel;
        SPDesktop* desktop = swp ? swp->getDesktop() : 0;
        sp_gradient_unset_swatch(desktop, bounceTarget->def.descr);
    }
}

static SwatchesPanel* findContainingPanel( GtkWidget *widget )
{
    SwatchesPanel *swp = 0;

    std::map<GtkWidget*, SwatchesPanel*> rawObjects;
    for (std::map<SwatchesPanel*, SPDocument*>::iterator it = docPerPanel.begin(); it != docPerPanel.end(); ++it) {
        rawObjects[GTK_WIDGET(it->first->gobj())] = it->first;
    }

    for (GtkWidget* curr = widget; curr && !swp; curr = gtk_widget_get_parent(curr)) {
        if (rawObjects.find(curr) != rawObjects.end()) {
            swp = rawObjects[curr];
        }
    }

    return swp;
}

static void removeit( GtkWidget *widget, gpointer data )
{
    gtk_container_remove( GTK_CONTAINER(data), widget );
}

/* extern'ed from colot-item.cpp */
gboolean colorItemHandleButtonPress( GtkWidget* widget, GdkEventButton* event, gpointer user_data );

gboolean colorItemHandleButtonPress( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
{
    gboolean handled = FALSE;

    if ( event && (event->button == 3) && (event->type == GDK_BUTTON_PRESS) ) {
        SwatchesPanel* swp = findContainingPanel( widget );

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

            child = gtk_menu_item_new_with_label(_("Delete"));
            g_signal_connect( G_OBJECT(child),
                              "activate",
                              G_CALLBACK(SwatchesPanelHook::deleteGradient),
                              user_data );
            gtk_menu_shell_append(GTK_MENU_SHELL(popupMenu), child);
            popupExtras.push_back(child);
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
            //gtk_widget_set_sensitive( child, FALSE );
            {
                popupSubHolder = child;
                popupSub = gtk_menu_new();
                gtk_menu_item_set_submenu( GTK_MENU_ITEM(child), popupSub );
            }

            gtk_widget_show_all(popupMenu);
        }

        if ( user_data ) {
            ColorItem* item = reinterpret_cast<ColorItem*>(user_data);
            bool show = swp && (swp->getSelectedIndex() == 0);
            for ( std::vector<GtkWidget*>::iterator it = popupExtras.begin(); it != popupExtras.end(); ++ it) {
                gtk_widget_set_sensitive(*it, show);
            }

            bounceTarget = item;
            bouncePanel = swp;
            popupItems.clear();
            if ( popupMenu ) {
                gtk_container_foreach(GTK_CONTAINER(popupSub), removeit, popupSub);
                bool processed = false;
                GtkWidget *wdgt = gtk_widget_get_ancestor(widget, SP_TYPE_DESKTOP_WIDGET);
                if ( wdgt ) {
                    SPDesktopWidget *dtw = SP_DESKTOP_WIDGET(wdgt);
                    if ( dtw && dtw->desktop ) {
                        // Pick up all gradients with vectors
                        const GSList *gradients = (dtw->desktop->doc())->getResourceList("gradient");
                        gint index = 0;
                        for (const GSList *curr = gradients; curr; curr = curr->next) {
                            SPGradient* grad = SP_GRADIENT(curr->data);
                            if ( grad->hasStops() && !grad->isSwatch() ) {
                                //gl = g_slist_prepend(gl, curr->data);
                                processed = true;
                                GtkWidget *child = gtk_menu_item_new_with_label(grad->getId());
                                gtk_menu_shell_append(GTK_MENU_SHELL(popupSub), child);

                                popupItems.push_back(grad->getId());
                                g_signal_connect( G_OBJECT(child),
                                                  "activate",
                                                  G_CALLBACK(SwatchesPanelHook::convertGradient),
                                                  GINT_TO_POINTER(index) );
                                index++;
                            }
                        }

                        gtk_widget_show_all(popupSub);
                    }
                }
                gtk_widget_set_sensitive( popupSubHolder, processed );

                gtk_menu_popup(GTK_MENU(popupMenu), NULL, NULL, NULL, NULL, event->button, event->time);
                handled = TRUE;
            }
        }
    }

    return handled;
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

static void skipWhitespace( char*& str ) {
    while ( *str == ' ' || *str == '\t' ) {
        str++;
    }
}

static bool parseNum( char*& str, int& val ) {
    val = 0;
    while ( '0' <= *str && *str <= '9' ) {
        val = val * 10 + (*str - '0');
        str++;
    }
    bool retval = !(*str == 0 || *str == ' ' || *str == '\t' || *str == '\r' || *str == '\n');
    return retval;
}


void _loadPaletteFile( gchar const *filename, gboolean user/*=FALSE*/ )
{
    char block[1024];
    FILE *f = Inkscape::IO::fopen_utf8name( filename, "r" );
    if ( f ) {
        char* result = fgets( block, sizeof(block), f );
        if ( result ) {
            if ( strncmp( "GIMP Palette", block, 12 ) == 0 ) {
                bool inHeader = true;
                bool hasErr = false;

                SwatchPage *onceMore = new SwatchPage();

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
                                skipWhitespace(ptr);
                                if ( *ptr ) {
                                    int r = 0;
                                    int g = 0;
                                    int b = 0;
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
                                            name = g_dpgettext2(NULL, "Palette", n);
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
                    if (user)
                        userSwatchPages.push_back(onceMore);
                    else
                        systemSwatchPages.push_back(onceMore);
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

static bool
compare_swatch_names(SwatchPage const *a, SwatchPage const *b) {

    return g_utf8_collate(a->_name.c_str(), b->_name.c_str()) < 0;
}

static void loadEmUp()
{
    static bool beenHere = false;
    gboolean userPalete = true;
    if ( !beenHere ) {
        beenHere = true;

        std::list<gchar *> sources;
        sources.push_back( Inkscape::Application::profile_path("palettes") );
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
                          if ( !g_str_has_suffix(lower, "~") ) {
                            gchar* full = g_build_filename(dirname, filename, NULL);
                            if ( !Inkscape::IO::file_test( full, G_FILE_TEST_IS_DIR ) ) {
                                _loadPaletteFile(full, userPalete);
                            }
                            g_free(full);
                          }
//                      }
                        g_free(lower);
                    }
                    g_dir_close(directory);
                }
            }

            // toss the dirname
            g_free(dirname);
            sources.pop_front();
            userPalete = false;
        }
    }

   // Sort the list of swatches by name, grouped by user/system
   userSwatchPages.sort(compare_swatch_names);
   systemSwatchPages.sort(compare_swatch_names);

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
    _currentDocument(0)
{
    Gtk::RadioMenuItem* hotItem = 0;
    _holder = new PreviewHolder();
    _clear = new ColorItem( ege::PaintDef::CLEAR );
    _remove = new ColorItem( ege::PaintDef::NONE );
    if (docPalettes.empty()) {
        SwatchPage *docPalette = new SwatchPage();

        docPalette->_name = "Auto";
        docPalettes[0] = docPalette;
    }

    loadEmUp();
    if ( !systemSwatchPages.empty() ) {
        SwatchPage* first = 0;
        int index = 0;
        Glib::ustring targetName;
        if ( !_prefs_path.empty() ) {
            Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            targetName = prefs->getString(_prefs_path + "/palette");
            if (!targetName.empty()) {
                if (targetName == "Auto") {
                    first = docPalettes[0];
                } else {
                    //index++;
                    std::vector<SwatchPage*> pages = _getSwatchSets();
                    for ( std::vector<SwatchPage*>::iterator iter = pages.begin(); iter != pages.end(); ++iter ) {
                        if ( (*iter)->_name == targetName ) {
                            first = *iter;
                            break;
                        }
                        index++;
                    }
                }
            }
        }

        if ( !first ) {
            first = docPalettes[0];
            _currentIndex = 0;
        } else {
            _currentIndex = index;
        }

        _rebuild();

        Gtk::RadioMenuItem::Group groupOne;

        int i = 0;
        std::vector<SwatchPage*> swatchSets = _getSwatchSets();
        for ( std::vector<SwatchPage*>::iterator it = swatchSets.begin(); it != swatchSets.end(); ++it) {
            SwatchPage* curr = *it;
            Gtk::RadioMenuItem* single = Gtk::manage(new Gtk::RadioMenuItem(groupOne, curr->_name));
            if ( curr == first ) {
                hotItem = single;
            }
            _regItem( single, 3, i );

            i++;
        }
    }

    if (Glib::ustring(prefsPath) == "/dialogs/swatches") {
        Gtk::Requisition sreq;
#if WITH_GTKMM_3_0
        Gtk::Requisition sreq_natural;
        get_preferred_size(sreq_natural, sreq);
#else
        sreq = size_request();
#endif
        int minHeight = 60;
        if (sreq.height < minHeight) {
            set_size_request(70, minHeight);
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
    _trackDocument( this, 0 );

    _documentConnection.disconnect();
    _selChanged.disconnect();

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

void SwatchesPanel::setOrientation(SPAnchorType how)
{
    // Must call the parent class or bad things might happen
    Inkscape::UI::Widget::Panel::setOrientation( how );

    if ( _holder )
    {
        _holder->setOrientation(SP_ANCHOR_SOUTH);
    }
}

void SwatchesPanel::setDesktop( SPDesktop* desktop )
{
    if ( desktop != _currentDesktop ) {
        if ( _currentDesktop ) {
            _documentConnection.disconnect();
            _selChanged.disconnect();
        }

        _currentDesktop = desktop;

        if ( desktop ) {
            _currentDesktop->selection->connectChanged(
                sigc::hide(sigc::mem_fun(*this, &SwatchesPanel::_updateFromSelection)));

            _currentDesktop->selection->connectModified(
                sigc::hide(sigc::hide(sigc::mem_fun(*this, &SwatchesPanel::_updateFromSelection))));

            _currentDesktop->connectToolSubselectionChanged(
                sigc::hide(sigc::mem_fun(*this, &SwatchesPanel::_updateFromSelection)));

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


class DocTrack
{
public:
    DocTrack(SPDocument *doc, sigc::connection &gradientRsrcChanged, sigc::connection &defsChanged, sigc::connection &defsModified) :
        doc(doc->doRef()),
        updatePending(false),
        lastGradientUpdate(0.0),
        gradientRsrcChanged(gradientRsrcChanged),
        defsChanged(defsChanged),
        defsModified(defsModified)
    {
        if ( !timer ) {
            timer = new Glib::Timer();
            refreshTimer = Glib::signal_timeout().connect( sigc::ptr_fun(handleTimerCB), 33 );
        }
        timerRefCount++;
    }

    ~DocTrack()
    {
        timerRefCount--;
        if ( timerRefCount <= 0 ) {
            refreshTimer.disconnect();
            timerRefCount = 0;
            if ( timer ) {
                timer->stop();
                delete timer;
                timer = 0;
            }
        }
        if (doc) {
            gradientRsrcChanged.disconnect();
            defsChanged.disconnect();
            defsModified.disconnect();
            doc->doUnref();
            doc = NULL;
        }
    }

    static bool handleTimerCB();

    /**
     * Checks if update should be queued or executed immediately.
     *
     * @return true if the update was queued and should not be immediately executed.
     */
    static bool queueUpdateIfNeeded(SPDocument *doc);

    static Glib::Timer *timer;
    static int timerRefCount;
    static sigc::connection refreshTimer;

    SPDocument *doc;
    bool updatePending;
    double lastGradientUpdate;
    sigc::connection gradientRsrcChanged;
    sigc::connection defsChanged;
    sigc::connection defsModified;

private:
    DocTrack(DocTrack const &); // no copy
    DocTrack &operator=(DocTrack const &); // no assign
};

Glib::Timer *DocTrack::timer = 0;
int DocTrack::timerRefCount = 0;
sigc::connection DocTrack::refreshTimer;

static const double DOC_UPDATE_THREASHOLD  = 0.090;

bool DocTrack::handleTimerCB()
{
    double now = timer->elapsed();

    std::vector<DocTrack *> needCallback;
    for (std::vector<DocTrack *>::iterator it = docTrackings.begin(); it != docTrackings.end(); ++it) {
        DocTrack *track = *it;
        if ( track->updatePending && ( (now - track->lastGradientUpdate) >= DOC_UPDATE_THREASHOLD) ) {
            needCallback.push_back(track);
        }
    }

    for (std::vector<DocTrack *>::iterator it = needCallback.begin(); it != needCallback.end(); ++it) {
        DocTrack *track = *it;
        if ( std::find(docTrackings.begin(), docTrackings.end(), track) != docTrackings.end() ) { // Just in case one gets deleted while we are looping
            // Note: calling handleDefsModified will call queueUpdateIfNeeded and thus update the time and flag.
            SwatchesPanel::handleDefsModified(track->doc);
        }
    }

    return true;
}

bool DocTrack::queueUpdateIfNeeded( SPDocument *doc )
{
    bool deferProcessing = false;
    for (std::vector<DocTrack*>::iterator it = docTrackings.begin(); it != docTrackings.end(); ++it) {
        DocTrack *track = *it;
        if ( track->doc == doc ) {
            double now = timer->elapsed();
            double elapsed = now - track->lastGradientUpdate;

            if ( elapsed < DOC_UPDATE_THREASHOLD ) {
                deferProcessing = true;
                track->updatePending = true;
            } else {
                track->lastGradientUpdate = now;
                track->updatePending = false;
            }

            break;
        }
    }
    return deferProcessing;
}

void SwatchesPanel::_trackDocument( SwatchesPanel *panel, SPDocument *document )
{
    SPDocument *oldDoc = NULL;
    if (docPerPanel.find(panel) != docPerPanel.end()) {
        oldDoc = docPerPanel[panel];
        if (!oldDoc) {
            docPerPanel.erase(panel); // Should not be needed, but clean up just in case.
        }
    }
    if (oldDoc != document) {
        if (oldDoc) {
            docPerPanel[panel] = NULL;
            bool found = false;
            for (std::map<SwatchesPanel*, SPDocument*>::iterator it = docPerPanel.begin(); (it != docPerPanel.end()) && !found; ++it) {
                found = (it->second == document);
            }
            if (!found) {
                for (std::vector<DocTrack*>::iterator it = docTrackings.begin(); it != docTrackings.end(); ++it){
                    if ((*it)->doc == oldDoc) {
                        delete *it;
                        docTrackings.erase(it);
                        break;
                    }
                }
            }
        }

        if (document) {
            bool found = false;
            for (std::map<SwatchesPanel*, SPDocument*>::iterator it = docPerPanel.begin(); (it != docPerPanel.end()) && !found; ++it) {
                found = (it->second == document);
            }
            docPerPanel[panel] = document;
            if (!found) {
                sigc::connection conn1 = document->connectResourcesChanged( "gradient", sigc::bind(sigc::ptr_fun(&SwatchesPanel::handleGradientsChange), document) );
                sigc::connection conn2 = document->getDefs()->connectRelease( sigc::hide(sigc::bind(sigc::ptr_fun(&SwatchesPanel::handleDefsModified), document)) );
                sigc::connection conn3 = document->getDefs()->connectModified( sigc::hide(sigc::hide(sigc::bind(sigc::ptr_fun(&SwatchesPanel::handleDefsModified), document))) );

                DocTrack *dt = new DocTrack(document, conn1, conn2, conn3);
                docTrackings.push_back(dt);

                if (docPalettes.find(document) == docPalettes.end()) {
                    SwatchPage *docPalette = new SwatchPage();
                    docPalette->_name = "Auto";
                    docPalettes[document] = docPalette;
                }
            }
        }
    }
}

void SwatchesPanel::_setDocument( SPDocument *document )
{
    if ( document != _currentDocument ) {
        _trackDocument(this, document);
        _currentDocument = document;
        handleGradientsChange( document );
    }
}

static void recalcSwatchContents(SPDocument* doc,
                boost::ptr_vector<ColorItem> &tmpColors,
                std::map<ColorItem*, cairo_pattern_t*> &previewMappings,
                std::map<ColorItem*, SPGradient*> &gradMappings)
{
    std::vector<SPGradient*> newList;

    const GSList *gradients = doc->getResourceList("gradient");
    for (const GSList *item = gradients; item; item = item->next) {
        SPGradient* grad = SP_GRADIENT(item->data);
        if ( grad->isSwatch() ) {
            newList.push_back(SP_GRADIENT(item->data));
        }
    }

    if ( !newList.empty() ) {
        std::reverse(newList.begin(), newList.end());
        for ( std::vector<SPGradient*>::iterator it = newList.begin(); it != newList.end(); ++it )
        {
            SPGradient* grad = *it;

            cairo_surface_t *preview = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                PREVIEW_PIXBUF_WIDTH, VBLOCK);
            cairo_t *ct = cairo_create(preview);

            Glib::ustring name( grad->getId() );
            ColorItem* item = new ColorItem( 0, 0, 0, name );

            cairo_pattern_t *check = ink_cairo_pattern_create_checkerboard();
            cairo_pattern_t *gradient = sp_gradient_create_preview_pattern(grad, PREVIEW_PIXBUF_WIDTH);
            cairo_set_source(ct, check);
            cairo_paint(ct);
            cairo_set_source(ct, gradient);
            cairo_paint(ct);

            cairo_destroy(ct);
            cairo_pattern_destroy(gradient);
            cairo_pattern_destroy(check);

            cairo_pattern_t *prevpat = cairo_pattern_create_for_surface(preview);
            cairo_surface_destroy(preview);

            previewMappings[item] = prevpat;

            tmpColors.push_back(item);
            gradMappings[item] = grad;
        }
    }
}

void SwatchesPanel::handleGradientsChange(SPDocument *document)
{
    SwatchPage *docPalette = (docPalettes.find(document) != docPalettes.end()) ? docPalettes[document] : 0;
    if (docPalette) {
        boost::ptr_vector<ColorItem> tmpColors;
        std::map<ColorItem*, cairo_pattern_t*> tmpPrevs;
        std::map<ColorItem*, SPGradient*> tmpGrads;
        recalcSwatchContents(document, tmpColors, tmpPrevs, tmpGrads);

        for (std::map<ColorItem*, cairo_pattern_t*>::iterator it = tmpPrevs.begin(); it != tmpPrevs.end(); ++it) {
            it->first->setPattern(it->second);
            cairo_pattern_destroy(it->second);
        }

        for (std::map<ColorItem*, SPGradient*>::iterator it = tmpGrads.begin(); it != tmpGrads.end(); ++it) {
            it->first->setGradient(it->second);
        }

        docPalette->_colors.swap(tmpColors);

        // Figure out which SwatchesPanel instances are affected and update them.

        for (std::map<SwatchesPanel*, SPDocument*>::iterator it = docPerPanel.begin(); it != docPerPanel.end(); ++it) {
            if (it->second == document) {
                SwatchesPanel* swp = it->first;
                std::vector<SwatchPage*> pages = swp->_getSwatchSets();
                SwatchPage* curr = pages[swp->_currentIndex];
                if (curr == docPalette) {
                    swp->_rebuild();
                }
            }
        }
    }
}

void SwatchesPanel::handleDefsModified(SPDocument *document)
{
    SwatchPage *docPalette = (docPalettes.find(document) != docPalettes.end()) ? docPalettes[document] : 0;
    if (docPalette && !DocTrack::queueUpdateIfNeeded(document) ) {
        boost::ptr_vector<ColorItem> tmpColors;
        std::map<ColorItem*, cairo_pattern_t*> tmpPrevs;
        std::map<ColorItem*, SPGradient*> tmpGrads;
        recalcSwatchContents(document, tmpColors, tmpPrevs, tmpGrads);

        if ( tmpColors.size() != docPalette->_colors.size() ) {
            handleGradientsChange(document);
        } else {
            int cap = std::min(docPalette->_colors.size(), tmpColors.size());
            for (int i = 0; i < cap; i++) {
                ColorItem *newColor = &tmpColors[i];
                ColorItem *oldColor = &docPalette->_colors[i];
                if ( (newColor->def.getType() != oldColor->def.getType()) ||
                     (newColor->def.getR() != oldColor->def.getR()) ||
                     (newColor->def.getG() != oldColor->def.getG()) ||
                     (newColor->def.getB() != oldColor->def.getB()) ) {
                    oldColor->def.setRGB(newColor->def.getR(), newColor->def.getG(), newColor->def.getB());
                }
                if (tmpGrads.find(newColor) != tmpGrads.end()) {
                    oldColor->setGradient(tmpGrads[newColor]);
                }
                if ( tmpPrevs.find(newColor) != tmpPrevs.end() ) {
                    oldColor->setPattern(tmpPrevs[newColor]);
                }
            }
        }

        for (std::map<ColorItem*, cairo_pattern_t*>::iterator it = tmpPrevs.begin(); it != tmpPrevs.end(); ++it) {
            cairo_pattern_destroy(it->second);
        }
    }
}


std::vector<SwatchPage*> SwatchesPanel::_getSwatchSets() const
{
    std::vector<SwatchPage*> tmp;
    if (docPalettes.find(_currentDocument) != docPalettes.end()) {
        tmp.push_back(docPalettes[_currentDocument]);
    }

    tmp.insert(tmp.end(), userSwatchPages.begin(), userSwatchPages.end());
    tmp.insert(tmp.end(), systemSwatchPages.begin(), systemSwatchPages.end());

    return tmp;
}

void SwatchesPanel::_updateFromSelection()
{
    SwatchPage *docPalette = (docPalettes.find(_currentDocument) != docPalettes.end()) ? docPalettes[_currentDocument] : 0;
    if ( docPalette ) {
        Glib::ustring fillId;
        Glib::ustring strokeId;

        SPStyle *tmpStyle = sp_style_new( sp_desktop_document(_currentDesktop) );
        int result = sp_desktop_query_style( _currentDesktop, tmpStyle, QUERY_STYLE_PROPERTY_FILL );
        switch (result) {
            case QUERY_STYLE_SINGLE:
            case QUERY_STYLE_MULTIPLE_AVERAGED:
            case QUERY_STYLE_MULTIPLE_SAME:
            {
                if (tmpStyle->fill.set && tmpStyle->fill.isPaintserver()) {
                    SPPaintServer* server = tmpStyle->getFillPaintServer();
                    if ( SP_IS_GRADIENT(server) ) {
                        SPGradient* target = 0;
                        SPGradient* grad = SP_GRADIENT(server);

                        if ( grad->isSwatch() ) {
                            target = grad;
                        } else if ( grad->ref ) {
                            SPGradient *tmp = grad->ref->getObject();
                            if ( tmp && tmp->isSwatch() ) {
                                target = tmp;
                            }
                        }
                        if ( target ) {
                            //XML Tree being used directly here while it shouldn't be
                            gchar const* id = target->getRepr()->attribute("id");
                            if ( id ) {
                                fillId = id;
                            }
                        }
                    }
                }
                break;
            }
        }

        result = sp_desktop_query_style( _currentDesktop, tmpStyle, QUERY_STYLE_PROPERTY_STROKE );
        switch (result) {
            case QUERY_STYLE_SINGLE:
            case QUERY_STYLE_MULTIPLE_AVERAGED:
            case QUERY_STYLE_MULTIPLE_SAME:
            {
                if (tmpStyle->stroke.set && tmpStyle->stroke.isPaintserver()) {
                    SPPaintServer* server = tmpStyle->getStrokePaintServer();
                    if ( SP_IS_GRADIENT(server) ) {
                        SPGradient* target = 0;
                        SPGradient* grad = SP_GRADIENT(server);
                        if ( grad->isSwatch() ) {
                            target = grad;
                        } else if ( grad->ref ) {
                            SPGradient *tmp = grad->ref->getObject();
                            if ( tmp && tmp->isSwatch() ) {
                                target = tmp;
                            }
                        }
                        if ( target ) {
                            //XML Tree being used directly here while it shouldn't be
                            gchar const* id = target->getRepr()->attribute("id");
                            if ( id ) {
                                strokeId = id;
                            }
                        }
                    }
                }
                break;
            }
        }
        sp_style_unref(tmpStyle);

        for ( boost::ptr_vector<ColorItem>::iterator it = docPalette->_colors.begin(); it != docPalette->_colors.end(); ++it ) {
            ColorItem* item = &*it;
            bool isFill = (fillId == item->def.descr);
            bool isStroke = (strokeId == item->def.descr);
            item->setState( isFill, isStroke );
        }
    }
}

void SwatchesPanel::_handleAction( int setId, int itemId )
{
    switch( setId ) {
        case 3:
        {
            std::vector<SwatchPage*> pages = _getSwatchSets();
            if ( itemId >= 0 && itemId < static_cast<int>(pages.size()) ) {
                _currentIndex = itemId;

                if ( !_prefs_path.empty() ) {
                    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                    prefs->setString(_prefs_path + "/palette", pages[_currentIndex]->_name);
                }

                _rebuild();
            }
        }
        break;
    }
}

void SwatchesPanel::_rebuild()
{
    std::vector<SwatchPage*> pages = _getSwatchSets();
    SwatchPage* curr = pages[_currentIndex];
    _holder->clear();

    if ( curr->_prefWidth > 0 ) {
        _holder->setColumnPref( curr->_prefWidth );
    }
    _holder->freezeUpdates();
    // TODO restore once 'clear' works _holder->addPreview(_clear);
    _holder->addPreview(_remove);
    for ( boost::ptr_vector<ColorItem>::iterator it = curr->_colors.begin(); it != curr->_colors.end(); ++it) {
        _holder->addPreview(&*it);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
