
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
#include <algorithm>

#include <gtk/gtkdialog.h> //for GTK_RESPONSE* types
#include <gtk/gtkdnd.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkseparatormenuitem.h>
#include <glibmm/i18n.h>
#include <gdkmm/pixbuf.h>

#include "color-item.h"
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
#include "sp-gradient-fns.h"
#include "sp-gradient.h"
#include "sp-gradient-vector.h"
#include "swatches.h"
#include "style.h"
#include "ui/previewholder.h"
#include "widgets/desktop-widget.h"
#include "widgets/gradient-vector.h"
#include "widgets/eek-preview.h"
#include "display/nr-plain-stuff.h"
#include "sp-gradient-reference.h"


namespace Inkscape {
namespace UI {
namespace Dialogs {

#define VBLOCK 16
#define PREVIEW_PIXBUF_WIDTH 128

void _loadPaletteFile( gchar const *filename );


class DocTrack;

std::vector<SwatchPage*> possible;
static std::map<SPDocument*, SwatchPage*> docPalettes;
static std::vector<DocTrack*> docTrackings;
static std::map<SwatchesPanel*, SPDocument*> docPerPanel;


class SwatchesPanelHook : public SwatchesPanel
{
public:
    static void convertGradient( GtkMenuItem *menuitem, gpointer userData );
    static void addNewGradient( GtkMenuItem *menuitem, gpointer user_data );
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
        SwatchesPanel* swp = bouncePanel;
        SPDesktop* desktop = swp ? swp->getDesktop() : 0;
        SPDocument *doc = desktop ? desktop->doc() : 0;
        if (doc) {
            std::string targetName(bounceTarget->def.descr);
            const GSList *gradients = sp_document_get_resource_list(doc, "gradient");
            for (const GSList *item = gradients; item; item = item->next) {
                SPGradient* grad = SP_GRADIENT(item->data);
                if ( targetName == grad->getId() ) {
                    editGradientImpl( grad );
                    break;
                }
            }
        }
    }
}

void SwatchesPanelHook::addNewGradient( GtkMenuItem */*menuitem*/, gpointer /*user_data*/ )
{
    if ( bounceTarget ) {
        SwatchesPanel* swp = bouncePanel;
        SPDesktop* desktop = swp ? swp->getDesktop() : 0;
        SPDocument *doc = desktop ? desktop->doc() : 0;
        if (doc) {
            Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);
            SPGradient * gr = 0;
            {
                Inkscape::XML::Node *repr = xml_doc->createElement("svg:linearGradient");
                Inkscape::XML::Node *stop = xml_doc->createElement("svg:stop");
                stop->setAttribute("offset", "0");
                stop->setAttribute("style", "stop-color:#000;stop-opacity:1;");
                repr->appendChild(stop);
                Inkscape::GC::release(stop);

                SP_OBJECT_REPR( SP_DOCUMENT_DEFS(doc) )->addChild(repr, NULL);

                gr = static_cast<SPGradient *>(doc->getObjectByRepr(repr));
                Inkscape::GC::release(repr);
            }

            gr->setSwatch();

            editGradientImpl( gr );
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

            const GSList *gradients = sp_document_get_resource_list(doc, "gradient");
            for (const GSList *item = gradients; item; item = item->next) {
                SPGradient* grad = SP_GRADIENT(item->data);
                if ( targetName == grad->getId() ) {
                    grad->setSwatch();
                    sp_document_done(doc, SP_VERB_CONTEXT_GRADIENT,
                                     _("Add gradient stop"));
                    break;
                }
            }
        }
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

gboolean colorItemHandleButtonPress( GtkWidget* widget, GdkEventButton* event, gpointer user_data )
{
    gboolean handled = FALSE;

    if ( (event->button == 3) && (event->type == GDK_BUTTON_PRESS) ) {
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

            child = gtk_menu_item_new_with_label(_("Add"));
            g_signal_connect( G_OBJECT(child),
                              "activate",
                              G_CALLBACK(SwatchesPanelHook::addNewGradient),
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
            //gtk_widget_set_sensitive( child, FALSE );
            {
                popupSubHolder = child;
                popupSub = gtk_menu_new();
                gtk_menu_item_set_submenu( GTK_MENU_ITEM(child), popupSub );
            }

            gtk_widget_show_all(popupMenu);
        }

        ColorItem* item = reinterpret_cast<ColorItem*>(user_data);
        if ( item ) {
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
                        const GSList *gradients = sp_document_get_resource_list(dtw->desktop->doc(), "gradient");
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
    if ( !possible.empty() ) {
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
                    index++;
                    for ( std::vector<SwatchPage*>::iterator iter = possible.begin(); iter != possible.end(); ++iter ) {
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
        for ( std::vector<SwatchPage*>::iterator it = swatchSets.begin(); it != swatchSets.end(); it++ ) {
            SwatchPage* curr = *it;
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
        doc(doc),
        gradientRsrcChanged(gradientRsrcChanged),
        defsChanged(defsChanged),
        defsModified(defsModified)
    {
    }

    ~DocTrack()
    {
        if (doc) {
            gradientRsrcChanged.disconnect();
            defsChanged.disconnect();
            defsModified.disconnect();
        }
    }

    SPDocument *doc;
    sigc::connection gradientRsrcChanged;
    sigc::connection defsChanged;
    sigc::connection defsModified;

private:
    DocTrack(DocTrack const &); // no copy
    DocTrack &operator=(DocTrack const &); // no assign
};

void SwatchesPanel::_trackDocument( SwatchesPanel *panel, SPDocument *document )
{
    SPDocument *oldDoc = 0;
    if (docPerPanel.find(panel) != docPerPanel.end()) {
        oldDoc = docPerPanel[panel];
        if (!oldDoc) {
            docPerPanel.erase(panel); // Should not be needed, but clean up just in case.
        }
    }
    if (oldDoc != document) {
        if (oldDoc) {
            docPerPanel[panel] = 0;
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
                sigc::connection conn1 = sp_document_resources_changed_connect( document, "gradient", sigc::bind(sigc::ptr_fun(&SwatchesPanel::handleGradientsChange), document) );
                sigc::connection conn2 = SP_DOCUMENT_DEFS(document)->connectRelease( sigc::hide(sigc::bind(sigc::ptr_fun(&SwatchesPanel::handleDefsModified), document)) );
                sigc::connection conn3 = SP_DOCUMENT_DEFS(document)->connectModified( sigc::hide(sigc::hide(sigc::bind(sigc::ptr_fun(&SwatchesPanel::handleDefsModified), document))) );

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

    std::set<SPDocument*> docs;
    for (std::map<SwatchesPanel*, SPDocument*>::iterator it = docPerPanel.begin(); it != docPerPanel.end(); ++it) {
        docs.insert(it->second);
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
                std::vector<ColorItem*> &tmpColors,
                std::map<ColorItem*, guchar*> &previewMappings,
                std::map<ColorItem*, SPGradient*> &gradMappings)
{
    std::vector<SPGradient*> newList;

    const GSList *gradients = sp_document_get_resource_list(doc, "gradient");
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
            grad->ensureVector();
            SPGradientStop first = grad->vector.stops[0];
            SPColor color = first.color;
            guint32 together = color.toRGBA32(first.opacity);

            SPGradientStop second = (*it)->vector.stops[1];
            SPColor color2 = second.color;

            Glib::ustring name( grad->getId() );
            unsigned int r = SP_RGBA32_R_U(together);
            unsigned int g = SP_RGBA32_G_U(together);
            unsigned int b = SP_RGBA32_B_U(together);
            ColorItem* item = new ColorItem( r, g, b, name );

            gint width = PREVIEW_PIXBUF_WIDTH;
            gint height = VBLOCK;
            guchar* px = g_new( guchar, 3 * height * width );
            nr_render_checkerboard_rgb( px, width, height, 3 * width, 0, 0 );

            sp_gradient_render_vector_block_rgb( grad,
                                                 px, width, height, 3 * width,
                                                 0, width, TRUE );

            previewMappings[item] = px;

            tmpColors.push_back(item);
            gradMappings[item] = grad;
        }
    }
}

void SwatchesPanel::handleGradientsChange(SPDocument *document)
{
    SwatchPage *docPalette = (docPalettes.find(document) != docPalettes.end()) ? docPalettes[document] : 0;
    if (docPalette) {
        std::vector<ColorItem*> tmpColors;
        std::map<ColorItem*, guchar*> tmpPrevs;
        std::map<ColorItem*, SPGradient*> tmpGrads;
        recalcSwatchContents(document, tmpColors, tmpPrevs, tmpGrads);

        for (std::map<ColorItem*, guchar*>::iterator it = tmpPrevs.begin(); it != tmpPrevs.end(); ++it) {
            it->first->setPixData(it->second, PREVIEW_PIXBUF_WIDTH, VBLOCK);
        }

        for (std::map<ColorItem*, SPGradient*>::iterator it = tmpGrads.begin(); it != tmpGrads.end(); ++it) {
            it->first->setGradient(it->second);
        }

        docPalette->_colors.swap(tmpColors);
        for (std::vector<ColorItem*>::iterator it = tmpColors.begin(); it != tmpColors.end(); ++it) {
            delete *it;
        }


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
    if (docPalette) {
        std::vector<ColorItem*> tmpColors;
        std::map<ColorItem*, guchar*> tmpPrevs;
        std::map<ColorItem*, SPGradient*> tmpGrads;
        recalcSwatchContents(document, tmpColors, tmpPrevs, tmpGrads);

        if ( tmpColors.size() != docPalette->_colors.size() ) {
            handleGradientsChange(document);
        } else {
            int cap = std::min(docPalette->_colors.size(), tmpColors.size());
            for (int i = 0; i < cap; i++) {
                ColorItem* newColor = tmpColors[i];
                ColorItem* oldColor = docPalette->_colors[i];
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
                    oldColor->setPixData(tmpPrevs[newColor], PREVIEW_PIXBUF_WIDTH, VBLOCK);
                }
            }
        }
    }
}

std::vector<SwatchPage*> SwatchesPanel::_getSwatchSets() const
{
    std::vector<SwatchPage*> tmp;
    if (docPalettes.find(_currentDocument) != docPalettes.end()) {
        tmp.push_back(docPalettes[_currentDocument]);
    }

    tmp.insert(tmp.end(), possible.begin(), possible.end());

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
                            gchar const* id = target->repr->attribute("id");
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
                            gchar const* id = target->repr->attribute("id");
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

        for ( std::vector<ColorItem*>::iterator it = docPalette->_colors.begin(); it != docPalette->_colors.end(); ++it ) {
            ColorItem* item = *it;
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
