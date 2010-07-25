/** @file
 * @brief A simple dialog for previewing icon representation.
 */
/* Authors:
 *   Jon A. Cruz
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004 Bob Jamison
 * Copyright (C) 2005,2010 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>
#include <glib/gmem.h>
#include <glibmm/i18n.h>
#include <gtkmm/alignment.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/stock.h>

#include "desktop.h"
#include "desktop-handles.h"
#include "display/nr-arena.h"
#include "document.h"
#include "inkscape.h"
#include "preferences.h"
#include "selection.h"
#include "sp-root.h"
#include "xml/repr.h"

#include "icon-preview.h"

extern "C" {
// takes doc, root, icon, and icon name to produce pixels
guchar *
sp_icon_doc_icon( SPDocument *doc, NRArenaItem *root,
                  const gchar *name, unsigned int psize );
}

namespace Inkscape {
namespace UI {
namespace Dialog {


IconPreviewPanel &IconPreviewPanel::getInstance()
{
    IconPreviewPanel *instance = new IconPreviewPanel();

    instance->refreshPreview();

    return *instance;
}

//#########################################################################
//## E V E N T S
//#########################################################################

void IconPreviewPanel::on_button_clicked(int which)
{
    if ( hot != which ) {
        buttons[hot]->set_active( false );

        hot = which;
        updateMagnify();
        _getContents()->queue_draw();
    }
}




//#########################################################################
//## C O N S T R U C T O R    /    D E S T R U C T O R
//#########################################################################
/**
 * Constructor
 */
IconPreviewPanel::IconPreviewPanel() :
    UI::Widget::Panel("", "/dialogs/iconpreview", SP_VERB_VIEW_ICON_PREVIEW),
    deskTrack(),
    desktop(0),
    document(0),
    timer(0),
    pending(false),
    targetId(),
    hot(1),
    selectionButton(0),
    desktopChangeConn(),
    docReplacedConn(),
    docModConn(),
    selChangedConn()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    numEntries = 0;

    bool pack = prefs->getBool("/iconpreview/pack", true);

    std::vector<Glib::ustring> pref_sizes = prefs->getAllDirs("/iconpreview/sizes/default");
    std::vector<int> rawSizes;

    for (std::vector<Glib::ustring>::iterator i = pref_sizes.begin(); i != pref_sizes.end(); ++i) {
        if (prefs->getBool(*i + "/show", true)) {
            int sizeVal = prefs->getInt(*i + "/value", -1);
            if (sizeVal > 0) {
                rawSizes.push_back(sizeVal);
            }
        }
    }

    if ( !rawSizes.empty() ) {
        numEntries = rawSizes.size();
        sizes = new int[numEntries];
        int i = 0;
        for ( std::vector<int>::iterator it = rawSizes.begin(); it != rawSizes.end(); ++it, ++i ) {
            sizes[i] = *it;
        }
    }

    if ( numEntries < 1 )
    {
        numEntries = 5;
        sizes = new int[numEntries];
        sizes[0] = 16;
        sizes[1] = 24;
        sizes[2] = 32;
        sizes[3] = 48;
        sizes[4] = 128;
    }

    pixMem = new guchar*[numEntries];
    images = new Gtk::Image*[numEntries];
    labels = new Glib::ustring*[numEntries];
    buttons = new Gtk::ToggleToolButton*[numEntries];


    for ( int i = 0; i < numEntries; i++ ) {
        char *label = g_strdup_printf(_("%d x %d"), sizes[i], sizes[i]);
        labels[i] = new Glib::ustring(label);
        g_free(label);
        pixMem[i] = 0;
        images[i] = 0;
    }


    magLabel.set_label( *labels[hot] );

    Gtk::VBox* magBox = new Gtk::VBox();

    Gtk::Frame *magFrame = Gtk::manage(new Gtk::Frame(_("Magnified:")));
    magFrame->add( magnified );

    magBox->pack_start( *magFrame, Gtk::PACK_EXPAND_WIDGET );
    magBox->pack_start( magLabel, Gtk::PACK_SHRINK );


    Gtk::VBox *verts = new Gtk::VBox();
    Gtk::HBox *horiz = 0;
    int previous = 0;
    int avail = 0;
    for ( int i = numEntries - 1; i >= 0; --i ) {
        pixMem[i] = new guchar[4 * sizes[i] * sizes[i]];
        memset( pixMem[i], 0x00, 4 *  sizes[i] * sizes[i] );

        GdkPixbuf *pb = gdk_pixbuf_new_from_data( pixMem[i], GDK_COLORSPACE_RGB, TRUE, 8, sizes[i], sizes[i], sizes[i] * 4, /*(GdkPixbufDestroyNotify)g_free*/NULL, NULL );
        GtkImage* img = GTK_IMAGE( gtk_image_new_from_pixbuf( pb ) );
        images[i] = Glib::wrap(img);
        Glib::ustring label(*labels[i]);
        buttons[i] = new Gtk::ToggleToolButton(label);
        buttons[i]->set_active( i == hot );
        if ( prefs->getBool("/iconpreview/showFrames", true) ) {
            Gtk::Frame *frame = new Gtk::Frame();
            frame->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
            frame->add(*images[i]);
            buttons[i]->set_icon_widget(*Gtk::manage(frame));
        } else {
            buttons[i]->set_icon_widget(*images[i]);
        }

        tips.set_tip((*buttons[i]), label);

        buttons[i]->signal_clicked().connect( sigc::bind<int>( sigc::mem_fun(*this, &IconPreviewPanel::on_button_clicked), i) );


        Gtk::Alignment *align = Gtk::manage(new Gtk::Alignment(0.5, 0.5, 0, 0));
        align->add(*buttons[i]);

        int pad = 12;
        if ( !pack || ( (avail == 0) && (previous == 0) ) ) {
            verts->pack_end(*align, Gtk::PACK_SHRINK);
            previous = sizes[i];
            avail = sizes[i];
        } else {
            if ((avail < pad) || ((sizes[i] > avail) && (sizes[i] < previous))) {
                horiz = 0;
            }
            if ((horiz == 0) && (sizes[i] <= previous)) {
                avail = previous;
            }
            if (sizes[i] <= avail) {
                if (!horiz) {
                    horiz = Gtk::manage(new Gtk::HBox());
                    avail = previous;
                    verts->pack_end(*horiz, Gtk::PACK_SHRINK);
                }
                horiz->pack_start(*align, Gtk::PACK_EXPAND_WIDGET);
                avail -= sizes[i];
                avail -= pad; // a little extra for padding
            } else {
                horiz = 0;
                verts->pack_end(*align, Gtk::PACK_SHRINK);
            }
        }
    }

    iconBox.pack_start(splitter);
    splitter.pack1( *magBox, true, true );
    Gtk::Frame *actuals = Gtk::manage(new Gtk::Frame(_("Actual Size:")));
    actuals->add(*verts);
    splitter.pack2( *actuals, false, false );


    selectionButton = new Gtk::CheckButton(_("Selection")); // , GTK_RESPONSE_APPLY
    magBox->pack_start( *selectionButton, Gtk::PACK_SHRINK );
    tips.set_tip((*selectionButton), _("Selection only or whole document"));
    selectionButton->signal_clicked().connect( sigc::mem_fun(*this, &IconPreviewPanel::modeToggled) );

    gint val = prefs->getBool("/iconpreview/selectionOnly");
    selectionButton->set_active( val != 0 );


    _getContents()->pack_start(iconBox, Gtk::PACK_SHRINK);

    show_all_children();

    // Connect this up last
    desktopChangeConn = deskTrack.connectDesktopChanged( sigc::mem_fun(*this, &IconPreviewPanel::setDesktop) );
    deskTrack.connect(GTK_WIDGET(gobj()));
}

IconPreviewPanel::~IconPreviewPanel()
{
    setDesktop(0);
    if (timer) {
        timer->stop();
        delete timer;
        timer = 0;
    }

    selChangedConn.disconnect();
    docModConn.disconnect();
    docReplacedConn.disconnect();
    desktopChangeConn.disconnect();
    deskTrack.disconnect();
}

//#########################################################################
//## M E T H O D S
//#########################################################################


void IconPreviewPanel::setDesktop( SPDesktop* desktop )
{
    Panel::setDesktop(desktop);

    SPDocument *newDoc = (desktop) ? desktop->doc() : 0;

    if ( desktop != this->desktop ) {
        docReplacedConn.disconnect();
        selChangedConn.disconnect();

        this->desktop = Panel::getDesktop();
        if ( this->desktop ) {
            docReplacedConn = this->desktop->connectDocumentReplaced(sigc::hide<0>(sigc::mem_fun(this, &IconPreviewPanel::setDocument)));
            if (this->desktop->selection) {
                selChangedConn = desktop->selection->connectChanged(sigc::hide(sigc::mem_fun(this, &IconPreviewPanel::queueRefresh)));
            }
        }
    }
    setDocument(newDoc);
    deskTrack.setBase(desktop);
}

void IconPreviewPanel::setDocument( SPDocument *document )
{
    if (this->document != document) {
        docModConn.disconnect();

        this->document = document;
        if (this->document) {
            docModConn = this->document->connectModified(sigc::hide(sigc::mem_fun(this, &IconPreviewPanel::queueRefresh)));
            queueRefresh();
        }
    }
}

void IconPreviewPanel::refreshPreview()
{
    SPDesktop *desktop = getDesktop();
    if (!timer) {
        timer = new Glib::Timer();
    }
    if (timer->elapsed() < 0.1) {
        // Do not refresh too quickly
        queueRefresh();
    } else if ( desktop ) {
        bool hold = Inkscape::Preferences::get()->getBool("/iconpreview/selectionHold", true);
        if ( selectionButton && selectionButton->get_active() )
        {
            SPObject *target = (hold && !targetId.empty()) ? desktop->doc()->getObjectById( targetId.c_str() ) : 0;
            if ( !target ) {
                targetId.clear();
                Inkscape::Selection * sel = sp_desktop_selection(desktop);
                if ( sel ) {
                    //g_message("found a selection to play with");

                    GSList const *items = sel->itemList();
                    while ( items && !target ) {
                        SPItem* item = SP_ITEM( items->data );
                        SPObject * obj = SP_OBJECT(item);
                        gchar const *id = obj->getId();
                        if ( id ) {
                            targetId = id;
                            target = obj;
                        }

                        items = g_slist_next(items);
                    }
                }
            }
            if ( target ) {
                renderPreview(target);
            }
        } else {
            SPObject *target = desktop->currentRoot();
            if ( target ) {
                renderPreview(target);
            }
        }
        timer->reset();
    }
}

bool IconPreviewPanel::refreshCB()
{
    bool callAgain = true;
    if (!timer) {
        timer = new Glib::Timer();
    }
    if ( timer->elapsed() > 0.1 ) {
        callAgain = false;
        refreshPreview();
        pending = false;
    }
    return callAgain;
}

void IconPreviewPanel::queueRefresh()
{
    if (!pending) {
        pending = true;
        if (!timer) {
            timer = new Glib::Timer();
        }
        Glib::signal_idle().connect( sigc::mem_fun(this, &IconPreviewPanel::refreshCB), Glib::PRIORITY_DEFAULT_IDLE );
    }
}

void IconPreviewPanel::modeToggled()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool selectionOnly = (selectionButton && selectionButton->get_active());
    prefs->setBool("/iconpreview/selectionOnly", selectionOnly);
    if ( !selectionOnly ) {
        targetId.clear();
    }

    refreshPreview();
}

void IconPreviewPanel::renderPreview( SPObject* obj )
{
    SPDocument * doc = SP_OBJECT_DOCUMENT(obj);
    gchar const * id = obj->getId();

//    g_message(" setting up to render '%s' as the icon", id );

    NRArenaItem *root = NULL;

    /* Create new arena */
    NRArena *arena = NRArena::create();

    /* Create ArenaItem and set transform */
    unsigned int visionkey = sp_item_display_key_new(1);

    root = sp_item_invoke_show ( SP_ITEM( SP_DOCUMENT_ROOT(doc) ),
                                 arena, visionkey, SP_ITEM_SHOW_DISPLAY );

    for ( int i = 0; i < numEntries; i++ ) {
        guchar * px = sp_icon_doc_icon( doc, root, id, sizes[i] );
//         g_message( " size %d %s", sizes[i], (px ? "worked" : "failed") );
        if ( px ) {
            memcpy( pixMem[i], px, sizes[i] * sizes[i] * 4 );
            g_free( px );
            px = 0;
        } else {
            memset( pixMem[i], 0, sizes[i] * sizes[i] * 4 );
        }
        images[i]->queue_draw();
    }
    updateMagnify();

    sp_item_invoke_hide(SP_ITEM(sp_document_root(doc)), visionkey);
    nr_object_unref((NRObject *) arena);
}

void IconPreviewPanel::updateMagnify()
{
    Glib::RefPtr<Gdk::Pixbuf> buf = images[hot]->get_pixbuf()->scale_simple( 128, 128, Gdk::INTERP_NEAREST );
    magLabel.set_label( *labels[hot] );
    magnified.set( buf );
    magnified.queue_draw();
    magnified.get_parent()->queue_draw();
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
