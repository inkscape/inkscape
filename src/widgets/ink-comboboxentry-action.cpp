/*
 * A subclass of GtkAction that wraps a GtkComboBoxEntry.
 * Features:
 *   Setting GtkEntryBox width in characters.
 *   Passing a function for formatting cells.
 *   Displaying a warning if entry text isn't in list.
 *   Check comma separated values in text against list. (Useful for font-family fallbacks.)
 *   Setting names for GtkComboBoxEntry and GtkEntry (actionName_combobox, actionName_entry)
 *     to allow setting resources.
 *
 * Author(s):
 *   Tavmjong Bah
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2010 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
 * We must provide for both a toolbar item and a menu item.
 * As we don't know which widgets are used (or even constructed),
 * we must keep track of things like active entry ourselves.
 */

#include <iostream>
#include <string.h>
#include <glibmm/ustring.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "widgets/ink-comboboxentry-action.h"
#include "ui/icon-names.h"

// Must handle both tool and menu items!
static GtkWidget* create_tool_item( GtkAction* action );
static GtkWidget* create_menu_item( GtkAction* action );

// Internal
static gint get_active_row_from_text( Ink_ComboBoxEntry_Action* action, const gchar* target_text, gboolean exclude = false, gboolean ignore_case = false );
static Glib::ustring check_comma_separated_text( Ink_ComboBoxEntry_Action* action );

// Callbacks
static void combo_box_changed_cb( GtkComboBox* widget, gpointer data );
static void entry_activate_cb( GtkEntry* widget, gpointer data );
static gboolean match_selected_cb( GtkEntryCompletion* widget, GtkTreeModel* model, GtkTreeIter* iter, gpointer data );
static gboolean keypress_cb( GtkWidget *widget, GdkEventKey *event, gpointer data );

enum {
  PROP_MODEL = 1,
  PROP_COMBOBOX,
  PROP_ENTRY,
  PROP_ENTRY_WIDTH,
  PROP_EXTRA_WIDTH,
  PROP_CELL_DATA_FUNC,
  PROP_SEPARATOR_FUNC,
  PROP_POPUP,
  PROP_FOCUS_WIDGET
};

enum {
  CHANGED = 0,
  ACTIVATED,
  N_SIGNALS
};
static guint signals[N_SIGNALS] = {0};

static GQuark gDataName = 0;

static void ink_comboboxentry_action_init (Ink_ComboBoxEntry_Action *action);
static void ink_comboboxentry_action_class_init (Ink_ComboBoxEntry_ActionClass *klass);

G_DEFINE_TYPE(Ink_ComboBoxEntry_Action, ink_comboboxentry_action, GTK_TYPE_ACTION);

static void ink_comboboxentry_action_finalize (GObject *object)
{
  // Free any allocated resources.

  G_OBJECT_CLASS (ink_comboboxentry_action_parent_class)->finalize (object);
}


static void ink_comboboxentry_action_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
  Ink_ComboBoxEntry_Action *action = INK_COMBOBOXENTRY_ACTION (object);

  switch(property_id) {

  case PROP_MODEL:
    action->model = GTK_TREE_MODEL( g_value_get_object( value ));
    break;

  case PROP_COMBOBOX:
    action->combobox = GTK_COMBO_BOX (g_value_get_object (value));
    break;

  case PROP_ENTRY:
    action->entry = GTK_ENTRY( g_value_get_object( value ));
    break;

  case PROP_ENTRY_WIDTH:
    action->entry_width = g_value_get_int( value );
    break;

  case PROP_EXTRA_WIDTH:
    action->extra_width = g_value_get_int( value );
    break;

  case PROP_CELL_DATA_FUNC:
    action->cell_data_func = g_value_get_pointer( value );
    break;

  case PROP_SEPARATOR_FUNC:
    action->separator_func = g_value_get_pointer( value );
    break;

  case PROP_POPUP:
    action->popup  = g_value_get_boolean( value );
    break;

  case PROP_FOCUS_WIDGET:
   action->focusWidget = (GtkWidget*)g_value_get_pointer( value );
   break;


  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}


static void ink_comboboxentry_action_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
  Ink_ComboBoxEntry_Action *action = INK_COMBOBOXENTRY_ACTION (object);

  switch(property_id) {

  case PROP_MODEL:
    g_value_set_object (value, action->model);
    break;

  case PROP_COMBOBOX:
    g_value_set_object (value, action->combobox);
    break;

  case PROP_ENTRY:
    g_value_set_object (value, action->entry);
    break;

  case PROP_ENTRY_WIDTH:
    g_value_set_int (value, action->entry_width);
    break;

  case PROP_EXTRA_WIDTH:
    g_value_set_int (value, action->extra_width);
    break;

  case PROP_CELL_DATA_FUNC:
    g_value_set_pointer (value, action->cell_data_func);
    break;

  case PROP_SEPARATOR_FUNC:
    g_value_set_pointer (value, action->separator_func);
    break;

  case PROP_POPUP:
    g_value_set_boolean (value, action->popup);
    break;

  case PROP_FOCUS_WIDGET:
    g_value_set_pointer (value, action->focusWidget);
    break;


  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
ink_comboboxentry_action_connect_proxy (GtkAction *action,
                                        GtkWidget *proxy)
{
  /* Override any proxy properties. */
  //  if (GTK_IS_MENU_ITEM (proxy)) {
  //  }

  GTK_ACTION_CLASS (ink_comboboxentry_action_parent_class)->connect_proxy (action, proxy);
}

static void
ink_comboboxentry_action_class_init (Ink_ComboBoxEntry_ActionClass *klass)
{

  GObjectClass     *gobject_class = G_OBJECT_CLASS (klass);
  GtkActionClass *gtkaction_class = GTK_ACTION_CLASS (klass);

  gtkaction_class->connect_proxy  = ink_comboboxentry_action_connect_proxy;

  gobject_class->finalize      = ink_comboboxentry_action_finalize;
  gobject_class->set_property  = ink_comboboxentry_action_set_property;
  gobject_class->get_property  = ink_comboboxentry_action_get_property;

  gDataName = g_quark_from_string("ink_comboboxentry-action");

  klass->parent_class.create_tool_item = create_tool_item;
  klass->parent_class.create_menu_item = create_menu_item;

  g_object_class_install_property (
                                   gobject_class,
                                   PROP_MODEL,
                                   g_param_spec_object ("model",
                                                        "Tree Model",
                                                        "Tree Model",
                                                        GTK_TYPE_TREE_MODEL,
                                                        (GParamFlags)G_PARAM_READWRITE));
  g_object_class_install_property (
                                   gobject_class,
                                   PROP_COMBOBOX,
                                   g_param_spec_object ("combobox",
                                                        "GtkComboBoxEntry",
                                                        "GtkComboBoxEntry",
                                                        GTK_TYPE_WIDGET,
                                                        (GParamFlags)G_PARAM_READABLE));
  g_object_class_install_property (
                                   gobject_class,
                                   PROP_ENTRY,
                                   g_param_spec_object ("entry",
                                                        "GtkEntry",
                                                        "GtkEntry",
                                                        GTK_TYPE_WIDGET,
                                                        (GParamFlags)G_PARAM_READABLE));
  g_object_class_install_property (
                                   gobject_class,
                                   PROP_ENTRY_WIDTH,
                                   g_param_spec_int ("entry_width",
                                                     "EntryBox width",
                                                     "EntryBox width (characters)",
                                                     -1.0, 100, -1.0,
                                                     (GParamFlags)G_PARAM_READWRITE));

  g_object_class_install_property (
                                   gobject_class,
                                   PROP_EXTRA_WIDTH,
                                   g_param_spec_int ("extra_width",
                                                     "Extra width",
                                                     "Extra width (px)",
                                                     -1.0, 500, -1.0,
                                                     (GParamFlags)G_PARAM_READWRITE));

  g_object_class_install_property (
                                   gobject_class,
                                   PROP_CELL_DATA_FUNC,
                                   g_param_spec_pointer ("cell_data_func",
                                                         "Cell Data Func",
                                                         "Cell Deta Function",
                                                         (GParamFlags)G_PARAM_READWRITE));

  g_object_class_install_property (
                                   gobject_class,
                                   PROP_SEPARATOR_FUNC,
                                   g_param_spec_pointer ("separator_func",
                                                         "Separator Func",
                                                         "Separator Function",
                                                         (GParamFlags)G_PARAM_READWRITE));

  g_object_class_install_property (
                                   gobject_class,
                                   PROP_POPUP,
                                   g_param_spec_boolean ("popup",
                                                         "Entry Popup",
                                                         "Entry Popup",
                                                         false,
                                                         (GParamFlags)G_PARAM_READWRITE));

  g_object_class_install_property( gobject_class,
                                   PROP_FOCUS_WIDGET,
                                   g_param_spec_pointer( "focus-widget",
                                                         "Focus Widget",
                                                         "The widget to return focus to",
                                                         (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT) ) );

  // We need to know when GtkComboBoxEvent or Menu ready for reading
  signals[CHANGED] = g_signal_new( "changed",
                                   G_TYPE_FROM_CLASS(klass),
                                   G_SIGNAL_RUN_FIRST,
                                   G_STRUCT_OFFSET(Ink_ComboBoxEntry_ActionClass, changed),
                                   NULL, NULL,
                                   g_cclosure_marshal_VOID__VOID,
                                   G_TYPE_NONE, 0);

  // Probably not needed... originally to keep track of key-presses.
  signals[ACTIVATED] = g_signal_new( "activated",
                                   G_TYPE_FROM_CLASS(klass),
                                   G_SIGNAL_RUN_FIRST,
                                   G_STRUCT_OFFSET(Ink_ComboBoxEntry_ActionClass, activated),
                                   NULL, NULL,
                                   g_cclosure_marshal_VOID__VOID,
                                   G_TYPE_NONE, 0);

}

static void ink_comboboxentry_action_init (Ink_ComboBoxEntry_Action *action)
{
  action->active = -1;
  action->text = strdup("");
  action->entry_completion = NULL;
  action->indicator = NULL;
  action->popup = false;
  action->info = NULL;
  action->info_cb = NULL;
  action->info_cb_id = 0;
  action->info_cb_blocked = false;
  action->warning = NULL;
  action->warning_cb = NULL;
  action->warning_cb_id = 0;
  action->warning_cb_blocked = false;
  action->altx_name = NULL;
  action->focusWidget = NULL;
}

Ink_ComboBoxEntry_Action *ink_comboboxentry_action_new (const gchar   *name,
                                                        const gchar   *label,
                                                        const gchar   *tooltip,
                                                        const gchar   *stock_id,
                                                        GtkTreeModel  *model,
                                                        gint           entry_width,
                                                        gint           extra_width,
                                                        void          *cell_data_func,
                                                        void          *separator_func,
                                                        GtkWidget      *focusWidget)
{
  g_return_val_if_fail (name != NULL, NULL);

  return (Ink_ComboBoxEntry_Action*)g_object_new (INK_COMBOBOXENTRY_TYPE_ACTION,
                                                  "name",           name,
                                                  "label",          label,
                                                  "tooltip",        tooltip,
                                                  "stock-id",       stock_id,
                                                  "model",          model,
                                                  "entry_width",    entry_width,
                                                  "extra_width",    extra_width,
                                                  "cell_data_func", cell_data_func,
                                                  "separator_func", separator_func,
                                                  "focus-widget",   focusWidget,
                                                  NULL);
}

// Create a widget for a toolbar.
GtkWidget* create_tool_item( GtkAction* action )
{
  GtkWidget* item = 0;

  if ( INK_COMBOBOXENTRY_IS_ACTION( action ) && INK_COMBOBOXENTRY_ACTION(action)->model ) {

    Ink_ComboBoxEntry_Action* ink_comboboxentry_action = INK_COMBOBOXENTRY_ACTION( action );

    gchar *action_name = g_strdup( gtk_action_get_name( action ) );
    gchar *combobox_name = g_strjoin( NULL, action_name, "_combobox", NULL );
    gchar *entry_name =    g_strjoin( NULL, action_name, "_entry", NULL );
    g_free( action_name );

    item = GTK_WIDGET( gtk_tool_item_new() );

    GtkWidget* comboBoxEntry = gtk_combo_box_new_with_model_and_entry (ink_comboboxentry_action->model);
    gtk_combo_box_set_entry_text_column (GTK_COMBO_BOX (comboBoxEntry), 0);

    // Name it so we can muck with it using an RC file
    gtk_widget_set_name( comboBoxEntry, combobox_name );
    g_free( combobox_name );

    {
#if GTK_CHECK_VERSION(3,0,0)
        gtk_widget_set_halign(comboBoxEntry, GTK_ALIGN_START);
        gtk_widget_set_hexpand(comboBoxEntry, FALSE);
        gtk_widget_set_vexpand(comboBoxEntry, FALSE);
        gtk_container_add(GTK_CONTAINER(item), comboBoxEntry);
#else
        GtkWidget *align = gtk_alignment_new(0, 0.5, 0, 0);
        gtk_container_add( GTK_CONTAINER(align), comboBoxEntry );
        gtk_container_add( GTK_CONTAINER(item), align );
#endif
    }

    ink_comboboxentry_action->combobox = GTK_COMBO_BOX (comboBoxEntry);

    //gtk_combo_box_set_active( GTK_COMBO_BOX( comboBoxEntry ), ink_comboboxentry_action->active );
    gtk_combo_box_set_active( GTK_COMBO_BOX( comboBoxEntry ), 0 );

    g_signal_connect( G_OBJECT(comboBoxEntry), "changed", G_CALLBACK(combo_box_changed_cb), action );

    // Optionally add separator function...
    if( ink_comboboxentry_action->separator_func != NULL ) {
       gtk_combo_box_set_row_separator_func( ink_comboboxentry_action->combobox,
					    GtkTreeViewRowSeparatorFunc (ink_comboboxentry_action->separator_func),
      					    NULL, NULL );
    }

    // FIXME: once gtk3 migration is done this can be removed
    // https://bugzilla.gnome.org/show_bug.cgi?id=734915
    gtk_widget_show_all (comboBoxEntry);

    // Optionally add formatting...
    if( ink_comboboxentry_action->cell_data_func != NULL ) {
      GtkCellRenderer *cell = gtk_cell_renderer_text_new();
      gtk_cell_layout_clear( GTK_CELL_LAYOUT( comboBoxEntry ) );
      gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( comboBoxEntry ), cell, true );
      gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT( comboBoxEntry ), cell,
                                          GtkCellLayoutDataFunc (ink_comboboxentry_action->cell_data_func),
                                          NULL, NULL );
    }

    // Optionally widen the combobox width... which widens the drop-down list in list mode.
    if( ink_comboboxentry_action->extra_width > 0 ) {
      GtkRequisition req;
#if GTK_CHECK_VERSION(3,0,0)
      gtk_widget_get_preferred_size(GTK_WIDGET(ink_comboboxentry_action->combobox), &req, NULL);
#else
      gtk_widget_size_request( GTK_WIDGET( ink_comboboxentry_action->combobox ), &req );
#endif
      gtk_widget_set_size_request( GTK_WIDGET( ink_comboboxentry_action->combobox ),
				   req.width + ink_comboboxentry_action->extra_width, -1 );
    }

    // Get reference to GtkEntry and fiddle a bit with it.
    GtkWidget *child = gtk_bin_get_child( GTK_BIN(comboBoxEntry) );

    // Name it so we can muck with it using an RC file
    gtk_widget_set_name( child, entry_name );
    g_free( entry_name );

    if( child && GTK_IS_ENTRY( child ) ) {

      ink_comboboxentry_action->entry = GTK_ENTRY(child);

      // Change width
      if( ink_comboboxentry_action->entry_width > 0 ) {
          gtk_entry_set_width_chars (GTK_ENTRY (child), ink_comboboxentry_action->entry_width );
      }

      // Add pop-up entry completion if required
      if( ink_comboboxentry_action->popup ) {
          ink_comboboxentry_action_popup_enable( ink_comboboxentry_action );
      }

      // Add altx_name if required
      if( ink_comboboxentry_action->altx_name ) {
          g_object_set_data( G_OBJECT( child ), ink_comboboxentry_action->altx_name, ink_comboboxentry_action->entry );
      }

      // Add signal for GtkEntry to check if finished typing.
      g_signal_connect( G_OBJECT(child), "activate", G_CALLBACK(entry_activate_cb), action );
      g_signal_connect( G_OBJECT(child), "key-press-event", G_CALLBACK(keypress_cb), action );
    }

    gtk_activatable_set_related_action( GTK_ACTIVATABLE (item), GTK_ACTION( action ) );
    gtk_widget_show_all( item );

  } else {

    item = GTK_ACTION_CLASS(ink_comboboxentry_action_parent_class)->create_tool_item( action );

  }

  return item;
}

// Create a drop-down menu.
GtkWidget* create_menu_item( GtkAction* action )
{
  GtkWidget* item = 0;

    item = GTK_ACTION_CLASS(ink_comboboxentry_action_parent_class)->create_menu_item( action );
    g_warning( "ink_comboboxentry_action: create_menu_item not implemented" );
    // One can easily modify ege-select-one-action routine to implement this.
  return item;
}

// Setters/Getters ---------------------------------------------------

GtkTreeModel *ink_comboboxentry_action_get_model( Ink_ComboBoxEntry_Action* action ) {

  return action->model;
}

GtkComboBox *ink_comboboxentry_action_get_comboboxentry( Ink_ComboBoxEntry_Action* action ) {

  return action->combobox;
}

gchar* ink_comboboxentry_action_get_active_text( Ink_ComboBoxEntry_Action* action ) {

  gchar* text = g_strdup( action->text );
  return text;
}

/*
 * For the font-family list we need to handle two cases:
 *   Text is in list store:
 *     In this case we use row number as the font-family list can have duplicate
 *     entries, one in the document font part and one in the system font part. In
 *     order that scrolling through the list works properly we must distinguish
 *     between the two.
 *   Text is not in the list store (i.e. default font-family is not on system):
 *     In this case we have a row number of -1, and the text must be set by hand.
 */
gboolean ink_comboboxentry_action_set_active_text( Ink_ComboBoxEntry_Action* action, const gchar* text, int row ) {

  if( strcmp( action->text, text ) != 0 ) { 
    g_free( action->text );
    action->text = g_strdup( text );
  }

  // Get active row or -1 if none
  if( row < 0 ) {
    row = get_active_row_from_text( action, action->text );
  }
  action->active = row;

  // Set active row, check that combobox has been created.
  if( action->combobox ) {
    gtk_combo_box_set_active( GTK_COMBO_BOX( action->combobox ), action->active );
  }

  // Fiddle with entry
  if( action->entry ) {

    // Explicitly set text in GtkEntry box (won't be set if text not in list).
    gtk_entry_set_text( action->entry, text );

    // Show or hide warning  -- this might be better moved to text-toolbox.cpp
    if( action->info_cb_id != 0 &&
	!action->info_cb_blocked ) {
      g_signal_handler_block (G_OBJECT(action->entry),
			      action->info_cb_id );
      action->info_cb_blocked = true;
    }
    if( action->warning_cb_id != 0 &&
	!action->warning_cb_blocked ) {
      g_signal_handler_block (G_OBJECT(action->entry),
			      action->warning_cb_id );
      action->warning_cb_blocked = true;
    }

    bool set = false;
    if( action->warning != NULL ) {
      Glib::ustring missing = check_comma_separated_text( action );
      if( !missing.empty() ) {
	  gtk_entry_set_icon_from_icon_name( action->entry,
					     GTK_ENTRY_ICON_SECONDARY,
					     INKSCAPE_ICON("dialog-warning") );
	// Can't add tooltip until icon set
	Glib::ustring warning = action->warning;
	warning += ": ";
	warning += missing;
	gtk_entry_set_icon_tooltip_text( action->entry,
					 GTK_ENTRY_ICON_SECONDARY,
					 warning.c_str() );

	if( action->warning_cb ) {

	  // Add callback if we haven't already
	  if( action->warning_cb_id == 0 ) {
	    action->warning_cb_id =
	      g_signal_connect( G_OBJECT(action->entry),
				"icon-press",
				G_CALLBACK(action->warning_cb),
				action);
	  }
	  // Unblock signal
	  if( action->warning_cb_blocked ) {
	    g_signal_handler_unblock (G_OBJECT(action->entry),
				      action->warning_cb_id );
	    action->warning_cb_blocked = false;
	  }
	}
	set = true;
      }
    }
 
    if( !set && action->info != NULL ) {
      gtk_entry_set_icon_from_icon_name( GTK_ENTRY(action->entry),
					 GTK_ENTRY_ICON_SECONDARY,
					 INKSCAPE_ICON("edit-select-all") );
      gtk_entry_set_icon_tooltip_text( action->entry,
				       GTK_ENTRY_ICON_SECONDARY,
				       action->info );

      if( action->info_cb ) {
	// Add callback if we haven't already
	if( action->info_cb_id == 0 ) {
	  action->info_cb_id =
	    g_signal_connect( G_OBJECT(action->entry),
			      "icon-press",
			      G_CALLBACK(action->info_cb),
			      action);
	}
	// Unblock signal
	if( action->info_cb_blocked ) {
	  g_signal_handler_unblock (G_OBJECT(action->entry),
				    action->info_cb_id );
	  action->info_cb_blocked = false;
	}
      }
      set = true;
    }

    if( !set ) {
      gtk_entry_set_icon_from_icon_name( GTK_ENTRY(action->entry),
					 GTK_ENTRY_ICON_SECONDARY,
					 NULL );
    }
  }

  // Return if active text in list
  gboolean found = ( action->active != -1 );
  return found;
}

void ink_comboboxentry_action_set_entry_width( Ink_ComboBoxEntry_Action* action, gint entry_width ) {

  action->entry_width = entry_width;

  // Widget may not have been created....
  if( action->entry ) {
    gtk_entry_set_width_chars( GTK_ENTRY(action->entry), entry_width );
  }
}

void ink_comboboxentry_action_set_extra_width( Ink_ComboBoxEntry_Action* action, gint extra_width ) {

  action->extra_width = extra_width;

  // Widget may not have been created....
  if( action->combobox ) {
    GtkRequisition req;
#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_get_preferred_size(GTK_WIDGET(action->combobox), &req, NULL);
#else
    gtk_widget_size_request( GTK_WIDGET( action->combobox ), &req );
#endif
    gtk_widget_set_size_request( GTK_WIDGET( action->combobox ), req.width + action->extra_width, -1 );
  }
}

void ink_comboboxentry_action_popup_enable( Ink_ComboBoxEntry_Action* action ) {

  action->popup = true;

  // Widget may not have been created....
  if( action->entry ) {

    // Check we don't already have a GtkEntryCompletion
    if( action->entry_completion ) return;

    action->entry_completion = gtk_entry_completion_new();

    gtk_entry_set_completion( action->entry, action->entry_completion );
    gtk_entry_completion_set_model( action->entry_completion, action->model );
    gtk_entry_completion_set_text_column( action->entry_completion, 0 );
    gtk_entry_completion_set_popup_completion( action->entry_completion, true );
    gtk_entry_completion_set_inline_completion( action->entry_completion, false );
    gtk_entry_completion_set_inline_selection( action->entry_completion, true );

    g_signal_connect (G_OBJECT (action->entry_completion),  "match-selected", G_CALLBACK (match_selected_cb), action );


  }
}

void ink_comboboxentry_action_popup_disable( Ink_ComboBoxEntry_Action* action ) {

  action->popup = false;

  if( action->entry_completion ) {
    gtk_widget_destroy(GTK_WIDGET(action->entry_completion));
    action->entry_completion = 0;
  }
}
void     ink_comboboxentry_action_set_tooltip( Ink_ComboBoxEntry_Action* action, const gchar* tooltip ) {

  // Widget may not have been created....
  if( action->entry ) {
      gtk_widget_set_tooltip_text ( GTK_WIDGET(action->entry), tooltip);
  }
  if( action->combobox ) {
      gtk_widget_set_tooltip_text ( GTK_WIDGET(action->combobox), tooltip);
  }

}

void     ink_comboboxentry_action_set_info( Ink_ComboBoxEntry_Action* action, const gchar* info ) {

  g_free( action->info );
  action->info = g_strdup( info );

  // Widget may not have been created....
  if( action->entry ) {
    gtk_entry_set_icon_tooltip_text( GTK_ENTRY(action->entry),
                                     GTK_ENTRY_ICON_SECONDARY,
                                     action->info );
  }
}

void     ink_comboboxentry_action_set_info_cb( Ink_ComboBoxEntry_Action* action, gpointer info_cb ) {

  action->info_cb = info_cb;
}

void     ink_comboboxentry_action_set_warning( Ink_ComboBoxEntry_Action* action, const gchar* warning ) {

  g_free( action->warning );
  action->warning = g_strdup( warning );

  // Widget may not have been created....
  if( action->entry ) {
    gtk_entry_set_icon_tooltip_text( GTK_ENTRY(action->entry),
                                     GTK_ENTRY_ICON_SECONDARY,
                                     action->warning );
  }
}

void     ink_comboboxentry_action_set_warning_cb( Ink_ComboBoxEntry_Action* action, gpointer warning_cb ) {

  action->warning_cb = warning_cb;
}

void     ink_comboboxentry_action_set_altx_name( Ink_ComboBoxEntry_Action* action, const gchar* altx_name ) {

  g_free( action->altx_name );
  action->altx_name = g_strdup( altx_name );

  // Widget may not have been created....
  if( action->entry ) {
    g_object_set_data( G_OBJECT(action->entry), action->altx_name, action->entry );
  }
}

// Internal ---------------------------------------------------

// Return row of active text or -1 if not found. If exclude is true,
// use 3d colunm if available to exclude row from checking (useful to
// skip rows added for font-families included in doc and not on
// system)
gint get_active_row_from_text( Ink_ComboBoxEntry_Action* action, const gchar* target_text,
			       gboolean exclude, gboolean ignore_case ) {

  // Check if text in list
  gint row = 0;
  gboolean found = false;
  GtkTreeIter iter;
  gboolean valid = gtk_tree_model_get_iter_first( action->model, &iter );
  while ( valid ) {

    // See if we should exclude a row
    gboolean check = true;  // If true, font-family is on system.
    if( exclude && gtk_tree_model_get_n_columns( action->model ) > 2 ) {
      gtk_tree_model_get( action->model, &iter, 2, &check, -1 );
    }

    if( check ) {
      // Get text from list entry
      gchar* text = 0;
      gtk_tree_model_get( action->model, &iter, 0, &text, -1 ); // Column 0

      if( !ignore_case ) {
	// Case sensitive compare
	if( strcmp( target_text, text ) == 0 ){
	  found = true;
	  break;
	}
      } else {
	// Case insensitive compare
	gchar* target_text_casefolded = g_utf8_casefold( target_text, -1 );
	gchar* text_casefolded        = g_utf8_casefold( text, -1 );
	gboolean equal = (strcmp( target_text_casefolded, text_casefolded ) == 0 );
	g_free( text_casefolded );
	g_free( target_text_casefolded );
	if( equal ) {
	  found = true;
	  break;
	}
      }
    }

    ++row;
    valid = gtk_tree_model_iter_next( action->model, &iter );
  }

  if( !found ) row = -1;

  return row;

}

// Checks if all comma separated text fragments are in the list and
// returns a ustring with a list of missing fragments.
// This is useful for checking if all fonts in a font-family fallback
// list are available on the system.
//
// This routine could also create a Pango Markup string to show which
// fragments are invalid in the entry box itself. See:
// http://developer.gnome.org/pango/stable/PangoMarkupFormat.html
// However... it appears that while one can retrieve the PangoLayout
// for a GtkEntry box, it is only a copy and changing it has no effect.
//   PangoLayout * pl = gtk_entry_get_layout( entry );
//   pango_layout_set_markup( pl, "NEW STRING", -1 ); // DOESN'T WORK
static Glib::ustring check_comma_separated_text( Ink_ComboBoxEntry_Action* action ) {

  Glib::ustring missing;

  // Parse fallback_list using a comma as deliminator
  gchar** tokens = g_strsplit( action->text, ",", 0 );

  gint i = 0;
  while( tokens[i] != NULL ) {

    // Remove any surrounding white space.
    g_strstrip( tokens[i] );

    if( get_active_row_from_text( action, tokens[i], true, true ) == -1 ) {
      missing += tokens[i];
      missing += ", ";
    }
    ++i;
  }
  g_strfreev( tokens );

  // Remove extra comma and space from end.
  if( missing.size() >= 2 ) {
    missing.resize( missing.size()-2 );
  }
  return missing;
}

// Callbacks ---------------------------------------------------

static void combo_box_changed_cb( GtkComboBox* widget, gpointer data ) {

  // Two things can happen to get here:
  //   An item is selected in the drop-down menu.
  //   Text is typed.
  // We only react here if an item is selected.

  // Get action
  Ink_ComboBoxEntry_Action *action = INK_COMBOBOXENTRY_ACTION( data );

  // Check if item selected:
  gint newActive = gtk_combo_box_get_active(widget);
  if( newActive >= 0 && newActive != action->active ) {

    action->active = newActive;

    GtkTreeIter iter;
    if( gtk_combo_box_get_active_iter( GTK_COMBO_BOX( action->combobox ), &iter ) ) {

      gchar* text = 0;
      gtk_tree_model_get( action->model, &iter, 0, &text, -1 );
      gtk_entry_set_text( action->entry, text );

      g_free( action->text );
      action->text = text;
    }

    // Now let the world know
    g_signal_emit( G_OBJECT(action), signals[CHANGED], 0 );
  }
}

static void entry_activate_cb( GtkEntry* widget, gpointer data ) {

  // Get text from entry box.. check if it matches a menu entry.

  // Get action
  Ink_ComboBoxEntry_Action *action = INK_COMBOBOXENTRY_ACTION( data );

  // Get text
  g_free( action->text );
  action->text = g_strdup( gtk_entry_get_text( widget ) );

  // Get row
  action->active =
    get_active_row_from_text( action, action->text );

  // Set active row
  gtk_combo_box_set_active( GTK_COMBO_BOX( action->combobox), action->active );

  // Now let the world know
  g_signal_emit( G_OBJECT(action), signals[CHANGED], 0 );

}

static gboolean match_selected_cb( GtkEntryCompletion* /*widget*/, GtkTreeModel* model, GtkTreeIter* iter, gpointer data )
{
  // Get action
  Ink_ComboBoxEntry_Action *action = INK_COMBOBOXENTRY_ACTION( data );
  GtkEntry *entry = action->entry;

  if( entry) {
    gchar *family = 0;
    gtk_tree_model_get(model, iter, 0, &family, -1);

    // Set text in GtkEntry
    gtk_entry_set_text (GTK_ENTRY (entry), family );

    // Set text in GtkAction
    g_free( action->text );
    action->text = family;

    // Get row
    action->active =
      get_active_row_from_text( action, action->text );

    // Set active row
    gtk_combo_box_set_active( GTK_COMBO_BOX( action->combobox), action->active );

    // Now let the world know
    g_signal_emit( G_OBJECT(action), signals[CHANGED], 0 );

    return true;
  }
  return false;
}

static void ink_comboboxentry_action_defocus( Ink_ComboBoxEntry_Action* action )
{
    if ( action->focusWidget ) {
        gtk_widget_grab_focus( action->focusWidget );
    }
}

gboolean keypress_cb( GtkWidget * /*widget*/, GdkEventKey *event, gpointer data )
{
    gboolean wasConsumed = FALSE; /* default to report event not consumed */
    guint key = 0;
    Ink_ComboBoxEntry_Action* action = INK_COMBOBOXENTRY_ACTION( data );
    gdk_keymap_translate_keyboard_state( gdk_keymap_get_for_display( gdk_display_get_default() ),
                                         event->hardware_keycode, (GdkModifierType)event->state,
                                         0, &key, 0, 0, 0 );

    switch ( key ) {

        // TODO Add bindings for Tab/LeftTab
        case GDK_KEY_Escape:
        {
            //gtk_spin_button_set_value( GTK_SPIN_BUTTON(widget), action->private_data->lastVal );
            ink_comboboxentry_action_defocus( action );
            wasConsumed = TRUE;
        }
        break;

        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
        {
            ink_comboboxentry_action_defocus( action );
            //wasConsumed = TRUE;
        }
        break;


    }

    return wasConsumed;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
