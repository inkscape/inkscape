#define __SP_COLOR_NOTEBOOK_C__

/*
 * A notebook with RGB, CMYK, CMS, HSL, and Wheel pages
 *
 * Author:
 *	 Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 *
 * This code is in public domain
 */

#undef SPCS_PREVIEW
#define noDUMP_CHANGE_INFO

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>
#include <cstdlib>
#include <gtk/gtk.h>
#include <glibmm/i18n.h>

#include "../dialogs/dialog-events.h"
#include "../preferences.h"
#include "sp-color-notebook.h"
#include "spw-utilities.h"
#include "sp-color-scales.h"
#include "sp-color-icc-selector.h"
#include "sp-color-wheel-selector.h"

struct SPColorNotebookTracker {
	const gchar* name;
	const gchar* className;
	GType type;
	guint submode;
	gboolean enabledFull;
	gboolean enabledBrief;
	SPColorNotebook *backPointer;
};

static void sp_color_notebook_class_init (SPColorNotebookClass *klass);
static void sp_color_notebook_init (SPColorNotebook *colorbook);
static void sp_color_notebook_destroy (GtkObject *object);

static void sp_color_notebook_show_all (GtkWidget *widget);
static void sp_color_notebook_hide_all (GtkWidget *widget);

static SPColorSelectorClass *parent_class;

#define XPAD 4
#define YPAD 1

GType sp_color_notebook_get_type(void)
{
    static GtkType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPColorNotebookClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_color_notebook_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPColorNotebook),
            0, // n_preallocs
            (GInstanceInitFunc)sp_color_notebook_init,
            0 // value_table
        };
        type = g_type_register_static(SP_TYPE_COLOR_SELECTOR, "SPColorNotebook", &info, static_cast<GTypeFlags>(0));
    }
    return type;
}

static void
sp_color_notebook_class_init (SPColorNotebookClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	SPColorSelectorClass *selector_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;
	selector_class = SP_COLOR_SELECTOR_CLASS (klass);

	parent_class = SP_COLOR_SELECTOR_CLASS (g_type_class_peek_parent (klass));

	object_class->destroy = sp_color_notebook_destroy;

	widget_class->show_all = sp_color_notebook_show_all;
	widget_class->hide_all = sp_color_notebook_hide_all;
}

static void
sp_color_notebook_switch_page(GtkNotebook *notebook,
                              GtkNotebookPage *page,
                              guint page_num,
                              SPColorNotebook *colorbook)
{
    if ( colorbook )
    {
        ColorNotebook* nb = (ColorNotebook*)(SP_COLOR_SELECTOR(colorbook)->base);
        nb->switchPage( notebook, page, page_num );

        // remember the page we seitched to
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setInt("/colorselector/page", page_num);
    }
}

void ColorNotebook::switchPage(GtkNotebook*,
                              GtkNotebookPage*,
                              guint page_num)
{
    SPColorSelector* csel;
    GtkWidget* widget;

    if ( gtk_notebook_get_current_page (GTK_NOTEBOOK (_book)) >= 0 )
    {
        csel = getCurrentSelector();
        csel->base->getColorAlpha(_color, &_alpha);
    }
    widget = gtk_notebook_get_nth_page (GTK_NOTEBOOK (_book), page_num);
    if ( widget && SP_IS_COLOR_SELECTOR (widget) )
    {
        csel = SP_COLOR_SELECTOR (widget);
        csel->base->setColorAlpha( _color, _alpha );

        // Temporary workaround to undo a spurious GRABBED
        _released();
    }
}

static gint sp_color_notebook_menu_handler( GtkWidget *widget, GdkEvent *event )
{
    if (event->type == GDK_BUTTON_PRESS)
    {
        SPColorSelector* csel = SP_COLOR_SELECTOR(widget);
        ((ColorNotebook*)(csel->base))->menuHandler( event );

        /* Tell calling code that we have handled this event; the buck
         * stops here. */
        return TRUE;
    }

    /* Tell calling code that we have not handled this event; pass it on. */
    return FALSE;
}

gint ColorNotebook::menuHandler( GdkEvent* event )
{
    GdkEventButton *bevent = (GdkEventButton *) event;
    gtk_menu_popup (GTK_MENU( _popup ), NULL, NULL, NULL, NULL,
                    bevent->button, bevent->time);
    return TRUE;
}

static void sp_color_notebook_menuitem_response (GtkMenuItem *menuitem, gpointer user_data)
{
	gboolean active = FALSE;

	active = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (menuitem));
	SPColorNotebookTracker *entry = reinterpret_cast< SPColorNotebookTracker* > (user_data);
	if ( entry )
	{
		if ( active )
		{
			((ColorNotebook*)(SP_COLOR_SELECTOR(entry->backPointer)->base))->addPage(entry->type, entry->submode);
		}
		else
		{
			((ColorNotebook*)(SP_COLOR_SELECTOR(entry->backPointer)->base))->removePage(entry->type, entry->submode);
		}
	}
}

static void
sp_color_notebook_init (SPColorNotebook *colorbook)
{
    SP_COLOR_SELECTOR(colorbook)->base = new ColorNotebook( SP_COLOR_SELECTOR(colorbook) );

    if ( SP_COLOR_SELECTOR(colorbook)->base )
    {
        SP_COLOR_SELECTOR(colorbook)->base->init();
    }
}

void ColorNotebook::init()
{
	GtkWidget* table = 0;
	guint row = 0;
	guint i = 0;
	guint j = 0;
	GType *selector_types = 0;
	guint	selector_type_count = 0;

	GtkTooltips *tt = gtk_tooltips_new ();

	/* tempory hardcoding to get types loaded */
	SP_TYPE_COLOR_SCALES;
	SP_TYPE_COLOR_WHEEL_SELECTOR;
#if ENABLE_LCMS
	SP_TYPE_COLOR_ICC_SELECTOR;
#endif // ENABLE_LCMS

	/* REJON: Comment out the next line to not use the normal GTK Color
           wheel. */

//        SP_TYPE_COLOR_GTKSELECTOR;

	_updating = FALSE;
	_updatingrgba = FALSE;
	_btn = 0;
	_popup = 0;
	_trackerList = g_ptr_array_new ();

	_book = gtk_notebook_new ();
	gtk_widget_show (_book);

	selector_types = g_type_children (SP_TYPE_COLOR_SELECTOR, &selector_type_count);

	for ( i = 0; i < selector_type_count; i++ )
	{
		if (!g_type_is_a (selector_types[i], SP_TYPE_COLOR_NOTEBOOK))
		{
			guint howmany = 1;
			gpointer klass = gtk_type_class (selector_types[i]);
			if ( klass && SP_IS_COLOR_SELECTOR_CLASS (klass) )
			{
				SPColorSelectorClass *ck = SP_COLOR_SELECTOR_CLASS (klass);
				howmany = MAX (1, ck->submode_count);
				for ( j = 0; j < howmany; j++ )
				{
					SPColorNotebookTracker *entry = reinterpret_cast< SPColorNotebookTracker* > (malloc(sizeof(SPColorNotebookTracker)));
					if ( entry )
					{
						memset( entry, 0, sizeof(SPColorNotebookTracker) );
						entry->name = ck->name[j];
						entry->type = selector_types[i];
						entry->submode = j;
						entry->enabledFull = TRUE;
						entry->enabledBrief = TRUE;
						entry->backPointer = SP_COLOR_NOTEBOOK(_csel);

						g_ptr_array_add (_trackerList, entry);
					}
				}
			}
		}
	}

	for ( i = 0; i < _trackerList->len; i++ )
	{
		SPColorNotebookTracker *entry =
          reinterpret_cast< SPColorNotebookTracker* > (g_ptr_array_index (_trackerList, i));
		if ( entry )
		{
			addPage(entry->type, entry->submode);
		}
	}

	table = gtk_table_new (2, 3, FALSE);
	gtk_widget_show (table);

	gtk_box_pack_start (GTK_BOX (_csel), table, TRUE, TRUE, 0);

	gtk_table_attach (GTK_TABLE (table), _book, 0, 2, row, row + 1,
                      static_cast<GtkAttachOptions>(GTK_EXPAND|GTK_FILL),
                      static_cast<GtkAttachOptions>(GTK_EXPAND|GTK_FILL),
                      XPAD, YPAD);

	// restore the last active page
	Inkscape::Preferences *prefs = Inkscape::Preferences::get();
	gtk_notebook_set_current_page (GTK_NOTEBOOK (_book), prefs->getInt("/colorselector/page", 0));

	{
		gboolean found = FALSE;

		_popup = gtk_menu_new();
		GtkMenu *menu = GTK_MENU (_popup);

		for ( i = 0; i < _trackerList->len; i++ )
		{
			SPColorNotebookTracker *entry = reinterpret_cast< SPColorNotebookTracker* > (g_ptr_array_index (_trackerList, i));
			if ( entry )
			{
				GtkWidget *item = gtk_check_menu_item_new_with_label (_(entry->name));
				gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), entry->enabledFull);
				gtk_widget_show (item);
				gtk_menu_append (menu, item);

				g_signal_connect (G_OBJECT (item), "activate",
								  G_CALLBACK (sp_color_notebook_menuitem_response),
								  reinterpret_cast< gpointer > (entry) );
				found = TRUE;
			}
		}

		GtkWidget *arrow = gtk_arrow_new(GTK_ARROW_RIGHT, GTK_SHADOW_NONE);
		gtk_widget_show (arrow);

		_btn = gtk_button_new ();
		gtk_widget_show (_btn);
		gtk_container_add (GTK_CONTAINER (_btn), arrow);

		GtkWidget *align = gtk_alignment_new (1.0, 0.0, 0.0, 0.0);
		gtk_widget_show (align);
		gtk_container_add (GTK_CONTAINER (align), _btn);

		// uncomment to reenable the "show/hide modes" menu,
		// but first fix it so it remembers its settings in prefs and does not take that much space (entire vertical column!)
		//gtk_table_attach (GTK_TABLE (table), align, 2, 3, row, row + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);

		gtk_signal_connect_object(GTK_OBJECT(_btn), "event", GTK_SIGNAL_FUNC (sp_color_notebook_menu_handler), GTK_OBJECT(_csel));
		if ( !found )
		{
			gtk_widget_set_sensitive (_btn, FALSE);
		}
	}

	row++;

	/* Create RGBA entry and color preview */
	GtkWidget *rgbabox = gtk_hbox_new (FALSE, 0);

	_rgbal = gtk_label_new_with_mnemonic (_("RGBA_:"));
	gtk_misc_set_alignment (GTK_MISC (_rgbal), 1.0, 0.5);
	gtk_box_pack_start(GTK_BOX(rgbabox), _rgbal, TRUE, TRUE, 2);

	_rgbae = gtk_entry_new ();
	sp_dialog_defocus_on_enter (_rgbae);
	gtk_entry_set_max_length (GTK_ENTRY (_rgbae), 8);
	gtk_entry_set_width_chars (GTK_ENTRY (_rgbae), 8);
	gtk_tooltips_set_tip (tt, _rgbae, _("Hexadecimal RGBA value of the color"), NULL);
	gtk_box_pack_start(GTK_BOX(rgbabox), _rgbae, FALSE, FALSE, 0);
	gtk_label_set_mnemonic_widget (GTK_LABEL(_rgbal), _rgbae);

	sp_set_font_size_smaller (rgbabox);
	gtk_widget_show_all (rgbabox);
	gtk_table_attach (GTK_TABLE (table), rgbabox, 1, 2, row, row + 1, GTK_FILL, GTK_SHRINK, XPAD, YPAD);

#ifdef SPCS_PREVIEW
	_p = sp_color_preview_new (0xffffffff);
	gtk_widget_show (_p);
	gtk_table_attach (GTK_TABLE (table), _p, 2, 3, row, row + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);
#endif

	_switchId = g_signal_connect(GTK_OBJECT (_book), "switch-page",
								GTK_SIGNAL_FUNC (sp_color_notebook_switch_page), SP_COLOR_NOTEBOOK(_csel));

	_entryId = gtk_signal_connect (GTK_OBJECT (_rgbae), "changed", GTK_SIGNAL_FUNC (ColorNotebook::_rgbaEntryChangedHook), _csel);
}

static void
sp_color_notebook_destroy (GtkObject *object)
{
	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

ColorNotebook::~ColorNotebook()
{
	if ( _trackerList )
	{
		g_ptr_array_free (_trackerList, TRUE);
		_trackerList = 0;
	}

	if ( _switchId )
	{
		if ( _book )
		{
			g_signal_handler_disconnect (_book, _switchId);
			_switchId = 0;
		}
	}
}

static void
sp_color_notebook_show_all (GtkWidget *widget)
{
	gtk_widget_show (widget);
}

static void
sp_color_notebook_hide_all (GtkWidget *widget)
{
	gtk_widget_hide (widget);
}

GtkWidget *
sp_color_notebook_new (void)
{
	SPColorNotebook *colorbook;

	colorbook = (SPColorNotebook*)gtk_type_new (SP_TYPE_COLOR_NOTEBOOK);

	return GTK_WIDGET (colorbook);
}

ColorNotebook::ColorNotebook( SPColorSelector* csel )
    : ColorSelector( csel )
{
}

SPColorSelector* ColorNotebook::getCurrentSelector()
{
    SPColorSelector* csel = NULL;
    gint current_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (_book));

    if ( current_page >= 0 )
    {
        GtkWidget* widget = gtk_notebook_get_nth_page (GTK_NOTEBOOK (_book), current_page);
        if ( SP_IS_COLOR_SELECTOR (widget) )
        {
            csel = SP_COLOR_SELECTOR (widget);
        }
    }

    return csel;
}

void ColorNotebook::_colorChanged()
{
    SPColorSelector* cselPage = getCurrentSelector();
    if ( cselPage )
    {
        cselPage->base->setColorAlpha( _color, _alpha );
    }

    _updateRgbaEntry( _color, _alpha );
}

void ColorNotebook::_rgbaEntryChangedHook(GtkEntry *entry, SPColorNotebook *colorbook)
{
    ((ColorNotebook*)(SP_COLOR_SELECTOR(colorbook)->base))->_rgbaEntryChanged( entry );
}

void ColorNotebook::_rgbaEntryChanged(GtkEntry* entry)
{
    if (_updating) return;
    if (_updatingrgba) return;

    const gchar *t = gtk_entry_get_text( entry );

    if (t) {
        gchar *e = 0;
        guint rgba = strtoul (t, &e, 16);
        if ( e != t ) {
            ptrdiff_t len=e-t;
            if ( len < 8 ) {
                rgba = rgba << ( 4 * ( 8 - len ) );
            }
            _updatingrgba = TRUE;
            SPColor color( rgba );
            setColorAlpha( color, SP_RGBA32_A_F(rgba), true );
            _updatingrgba = FALSE;
        }
    }
}

void ColorNotebook::_updateRgbaEntry( const SPColor& color, gfloat alpha )
{
    g_return_if_fail( ( 0.0 <= alpha ) && ( alpha <= 1.0 ) );

    if ( !_updatingrgba )
    {
        gchar s[32];
        guint32 rgba;

        /* Update RGBA entry */
        rgba = color.toRGBA32( alpha );

        g_snprintf (s, 32, "%08x", rgba);
        const gchar* oldText = gtk_entry_get_text( GTK_ENTRY( _rgbae ) );
        if ( strcmp( oldText, s ) != 0 )
        {
            g_signal_handler_block( _rgbae, _entryId );
            gtk_entry_set_text( GTK_ENTRY(_rgbae), s );
            g_signal_handler_unblock( _rgbae, _entryId );
        }
    }
}

void ColorNotebook::_entryGrabbed (SPColorSelector *, SPColorNotebook *colorbook)
{
    ColorNotebook* nb = (ColorNotebook*)(SP_COLOR_SELECTOR(colorbook)->base);
    nb->_grabbed();
}

void ColorNotebook::_entryDragged (SPColorSelector *csel, SPColorNotebook *colorbook)
{
	gboolean oldState;
    ColorNotebook* nb = (ColorNotebook*)(SP_COLOR_SELECTOR(colorbook)->base);

	oldState = nb->_dragging;

	nb->_dragging = TRUE;
	nb->_entryModified( csel, colorbook );

	nb->_dragging = oldState;
}

void ColorNotebook::_entryReleased (SPColorSelector *, SPColorNotebook *colorbook)
{
    ColorNotebook* nb = (ColorNotebook*)(SP_COLOR_SELECTOR(colorbook)->base);
    nb->_released();
}

void ColorNotebook::_entryChanged (SPColorSelector *csel, SPColorNotebook *colorbook)
{
	gboolean oldState;
    ColorNotebook* nb = (ColorNotebook*)(SP_COLOR_SELECTOR(colorbook)->base);

	oldState = nb->_dragging;

	nb->_dragging = FALSE;
	nb->_entryModified( csel, colorbook );

	nb->_dragging = oldState;
}

void ColorNotebook::_entryModified (SPColorSelector *csel, SPColorNotebook *colorbook)
{
	g_return_if_fail (colorbook != NULL);
	g_return_if_fail (SP_IS_COLOR_NOTEBOOK (colorbook));
	g_return_if_fail (csel != NULL);
	g_return_if_fail (SP_IS_COLOR_SELECTOR (csel));

    ColorNotebook* nb = (ColorNotebook*)(SP_COLOR_SELECTOR(colorbook)->base);
    SPColor color;
    gfloat alpha = 1.0;

    csel->base->getColorAlpha( color, &alpha );
    nb->_updateRgbaEntry( color, alpha );
    nb->_updateInternals( color, alpha, nb->_dragging );
}

GtkWidget* ColorNotebook::addPage(GType page_type, guint submode)
{
	GtkWidget *page;

	page = sp_color_selector_new( page_type );
	if ( page )
	{
		GtkWidget* tab_label = 0;
		SPColorSelector* csel;

		csel = SP_COLOR_SELECTOR (page);
		if ( submode > 0 )
		{
			csel->base->setSubmode( submode );
		}
		gtk_widget_show (page);
        int index = csel->base ? csel->base->getSubmode() : 0;
        const gchar* str = _(SP_COLOR_SELECTOR_GET_CLASS (csel)->name[index]);
//         g_message( "Hitting up for tab for '%s'", str );
		tab_label = gtk_label_new(_(str));
		gtk_notebook_append_page( GTK_NOTEBOOK (_book), page, tab_label );
		gtk_signal_connect (GTK_OBJECT (page), "grabbed", GTK_SIGNAL_FUNC (_entryGrabbed), _csel);
		gtk_signal_connect (GTK_OBJECT (page), "dragged", GTK_SIGNAL_FUNC (_entryDragged), _csel);
		gtk_signal_connect (GTK_OBJECT (page), "released", GTK_SIGNAL_FUNC (_entryReleased), _csel);
		gtk_signal_connect (GTK_OBJECT (page), "changed", GTK_SIGNAL_FUNC (_entryChanged), _csel);
	}

	return page;
}

GtkWidget* ColorNotebook::getPage(GType page_type, guint submode)
{
	gint count = 0;
	gint i = 0;
	GtkWidget* page = 0;

//	  count = gtk_notebook_get_n_pages (_book);
	count = 200;
	for ( i = 0; i < count && !page; i++ )
	{
		page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (_book), i);
		if ( page )
		{
			SPColorSelector* csel;
			guint pagemode;
			csel = SP_COLOR_SELECTOR (page);
			pagemode = csel->base->getSubmode();
			if ( G_TYPE_FROM_INSTANCE (page) == page_type
				 && pagemode == submode )
			{
				// found it.
				break;
			}
			else
			{
				page = 0;
			}
		}
		else
		{
			break;
		}
	}
	return page;
}

void ColorNotebook::removePage( GType page_type, guint submode )
{
	GtkWidget *page = 0;

	page = getPage(page_type, submode);
	if ( page )
	{
		gint where = gtk_notebook_page_num (GTK_NOTEBOOK (_book), page);
		if ( where >= 0 )
		{
			if ( gtk_notebook_get_current_page (GTK_NOTEBOOK (_book)) == where )
			{
//                 getColorAlpha(_color, &_alpha);
			}
			gtk_notebook_remove_page (GTK_NOTEBOOK (_book), where);
		}
	}
}

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
