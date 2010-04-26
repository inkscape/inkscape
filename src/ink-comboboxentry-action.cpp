/*
 * A subclass of GtkAction that wraps a GtkComboBoxEntry.
 * Features:
 *   Setting GtkEntryBox width in characters.
 *   Passing a function for formatting cells.
 *   Displaying a warning if text isn't in list.
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

#include <gtk/gtk.h>
#include <gtk/gtktoolitem.h>
#include <gtk/gtkcomboboxentry.h>
#include <gtk/gtkentrycompletion.h>

#include "ink-comboboxentry-action.h"

// Must handle both tool and menu items!
static GtkWidget* create_tool_item( GtkAction* action );
static GtkWidget* create_menu_item( GtkAction* action );

// Internal
static gint get_active_row_from_text( Ink_ComboBoxEntry_Action* action, const gchar* target_text );

// Callbacks
static void combo_box_changed_cb( GtkComboBoxEntry* widget, gpointer data );
static void entry_activate_cb( GtkEntry* widget, gpointer data );
static gboolean match_selected_cb( GtkEntryCompletion* widget, GtkTreeModel* model, GtkTreeIter* iter, gpointer data );

enum {
  PROP_MODEL = 1,
  PROP_COMBOBOX,
  PROP_ENTRY,
  PROP_WIDTH,
  PROP_CELL_DATA_FUNC,
  PROP_POPUP
};

enum {
  CHANGED = 0,
  ACTIVATED,
  N_SIGNALS
};
static guint signals[N_SIGNALS] = {0};

static GtkActionClass *ink_comboboxentry_action_parent_class = NULL;
static GQuark gDataName = 0;

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
    action->combobox = GTK_COMBO_BOX_ENTRY( g_value_get_object( value ));
    break;

  case PROP_ENTRY:
    action->entry = GTK_ENTRY( g_value_get_object( value ));
    break;

  case PROP_WIDTH:
    action->width = g_value_get_int( value );
    break;

  case PROP_CELL_DATA_FUNC:
    action->cell_data_func = g_value_get_pointer( value );
    break;

  case PROP_POPUP:
    action->popup  = g_value_get_boolean( value );
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

  case PROP_WIDTH:
    g_value_set_int (value, action->width);
    break;

  case PROP_CELL_DATA_FUNC:
    g_value_set_pointer (value, action->cell_data_func);
    break;

  case PROP_POPUP:
    g_value_set_boolean (value, action->popup);
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


static void ink_comboboxentry_action_class_init (Ink_ComboBoxEntry_ActionClass *klass)
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

  ink_comboboxentry_action_parent_class = GTK_ACTION_CLASS(g_type_class_peek_parent (klass) );

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
                                   PROP_WIDTH,
                                   g_param_spec_int ("width",
                                                     "EntryBox width",
                                                     "EntryBox width (characters)",
                                                     -1.0, 100, -1.0,
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
                                   PROP_POPUP,
                                   g_param_spec_boolean ("popup",
                                                         "Entry Popup",
                                                         "Entry Popup",
                                                         false,
                                                         (GParamFlags)G_PARAM_READWRITE));

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
  action->text = NULL;
  action->entry_completion = NULL;
#if !GTK_CHECK_VERSION(2,16,0)
  action->indicator = NULL;
#endif
  action->popup = false;
  action->warning = NULL;
  action->altx_name = NULL;
}

GType ink_comboboxentry_action_get_type ()
{
  static GType ink_comboboxentry_action_type = 0;

  if (!ink_comboboxentry_action_type) {
    static const GTypeInfo ink_comboboxentry_action_info = {
      sizeof(Ink_ComboBoxEntry_ActionClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ink_comboboxentry_action_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(Ink_ComboBoxEntry_Action),
      0,    /* n_preallocs */
      (GInstanceInitFunc)ink_comboboxentry_action_init, /* instance_init */
      NULL  /* value_table */
    };

    ink_comboboxentry_action_type = g_type_register_static (GTK_TYPE_ACTION,
                                                            "Ink_ComboBoxEntry_Action",
                                                            &ink_comboboxentry_action_info,
                                                            (GTypeFlags)0 );
  }

  return ink_comboboxentry_action_type;
}


Ink_ComboBoxEntry_Action *ink_comboboxentry_action_new (const gchar   *name,
                                                        const gchar   *label,
                                                        const gchar   *tooltip,
                                                        const gchar   *stock_id,
                                                        GtkTreeModel  *model,
                                                        gint           width,
                                                        void          *cell_data_func )
{
  g_return_val_if_fail (name != NULL, NULL);

  return (Ink_ComboBoxEntry_Action*)g_object_new (INK_COMBOBOXENTRY_TYPE_ACTION,
                                                  "name",           name,
                                                  "label",          label,
                                                  "tooltip",        tooltip,
                                                  "stock-id",       stock_id,
                                                  "model",          model,
                                                  "width",          width,
                                                  "cell_data_func", cell_data_func,
                                                  NULL);
}

// Create a widget for a toolbar.
GtkWidget* create_tool_item( GtkAction* action )
{
  GtkWidget* item = 0;

  if ( INK_COMBOBOXENTRY_IS_ACTION( action ) && INK_COMBOBOXENTRY_ACTION(action)->model ) {

    Ink_ComboBoxEntry_Action* ink_comboboxentry_action = INK_COMBOBOXENTRY_ACTION( action );

    item = GTK_WIDGET( gtk_tool_item_new() );

    GtkWidget* comboBoxEntry = gtk_combo_box_entry_new_with_model( ink_comboboxentry_action->model, 0 );

    {
        GtkWidget *align = gtk_alignment_new(0, 0.5, 0, 0);
#if GTK_CHECK_VERSION(2,16,0)
        gtk_container_add( GTK_CONTAINER(align), comboBoxEntry );
#else // GTK_CHECK_VERSION(2,16,0)
        GtkWidget *hbox = gtk_hbox_new( FALSE, 0 );
        ink_comboboxentry_action->indicator = gtk_image_new_from_stock(GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_SMALL_TOOLBAR);
        gtk_box_pack_start( GTK_BOX(hbox), comboBoxEntry, TRUE, TRUE, 0 );
        gtk_box_pack_start( GTK_BOX(hbox), ink_comboboxentry_action->indicator, FALSE, FALSE, 0 );
        gtk_container_add( GTK_CONTAINER(align), hbox );
#endif // GTK_CHECK_VERSION(2,16,0)
        gtk_container_add( GTK_CONTAINER(item), align );
    }

    ink_comboboxentry_action->combobox = GTK_COMBO_BOX_ENTRY(comboBoxEntry);

    gtk_combo_box_set_active( GTK_COMBO_BOX( comboBoxEntry ), ink_comboboxentry_action->active );

    g_signal_connect( G_OBJECT(comboBoxEntry), "changed", G_CALLBACK(combo_box_changed_cb), action );

    // Optionally add formatting...
    if( ink_comboboxentry_action->cell_data_func != NULL ) {
      GtkCellRenderer *cell = gtk_cell_renderer_text_new();
      gtk_cell_layout_clear( GTK_CELL_LAYOUT( comboBoxEntry ) );
      gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( comboBoxEntry ), cell, true );
      gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT( comboBoxEntry ), cell,
                                          GtkCellLayoutDataFunc (ink_comboboxentry_action->cell_data_func),
                                          NULL, NULL );
    }

    // Get reference to GtkEntry and fiddle a bit with it.
    GtkWidget *child = gtk_bin_get_child( GTK_BIN(comboBoxEntry) );
    if( child && GTK_IS_ENTRY( child ) ) {
      ink_comboboxentry_action->entry = GTK_ENTRY(child);

      // Change width
      if( ink_comboboxentry_action->width > 0 ) {
          gtk_entry_set_width_chars (GTK_ENTRY (child), ink_comboboxentry_action->width );
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

    }

#if GTK_CHECK_VERSION(2,16,0)
    gtk_action_connect_proxy( GTK_ACTION( action ), item );
#endif

    gtk_widget_show_all( item );

  } else {

    item = ink_comboboxentry_action_parent_class->create_tool_item( action );

  }

  return item;
}

// Create a drop-down menu.
GtkWidget* create_menu_item( GtkAction* action )
{
  GtkWidget* item = 0;

    item = ink_comboboxentry_action_parent_class->create_menu_item( action );
    g_warning( "ink_comboboxentry_action: create_menu_item not implemented" );
    // One can easily modify ege-select-one-action routine to implement this.
  return item;
}

// Setters/Getters ---------------------------------------------------

GtkTreeModel *ink_comboboxentry_action_get_model( Ink_ComboBoxEntry_Action* action ) {

  return action->model;
}

GtkComboBoxEntry *ink_comboboxentry_action_get_comboboxentry( Ink_ComboBoxEntry_Action* action ) {

  return action->combobox;
}

gchar* ink_comboboxentry_action_get_active_text( Ink_ComboBoxEntry_Action* action ) {

  gchar* text = g_strdup( action->text );
  return text;
}

gboolean ink_comboboxentry_action_set_active_text( Ink_ComboBoxEntry_Action* ink_comboboxentry_action, const gchar* text ) {

  g_free( ink_comboboxentry_action->text );
  ink_comboboxentry_action->text = g_strdup( text );

  // Get active row or -1 if none
  ink_comboboxentry_action->active = get_active_row_from_text( ink_comboboxentry_action, ink_comboboxentry_action->text );

  // Set active row, check that combobox has been created.
  if( ink_comboboxentry_action->combobox ) {
    gtk_combo_box_set_active( GTK_COMBO_BOX( ink_comboboxentry_action->combobox ), ink_comboboxentry_action->active );
  }

  // Fiddle with entry
  if( ink_comboboxentry_action->entry ) {

    // Explicitly set text in GtkEntry box (won't be set if text not in list).
    gtk_entry_set_text( ink_comboboxentry_action->entry, text );

    // Show or hide warning
    if( ink_comboboxentry_action->active == -1 && ink_comboboxentry_action->warning != NULL ) {
#if GTK_CHECK_VERSION(2,16,0)
      gtk_entry_set_icon_from_icon_name( ink_comboboxentry_action->entry,
                                         GTK_ENTRY_ICON_SECONDARY,
                                         GTK_STOCK_DIALOG_WARNING );
      // Can't add tooltip until icon set
      gtk_entry_set_icon_tooltip_text( ink_comboboxentry_action->entry,
                                       GTK_ENTRY_ICON_SECONDARY,
                                       ink_comboboxentry_action->warning );
#else // GTK_CHECK_VERSION(2,16,0)
      gtk_image_set_from_stock( GTK_IMAGE(ink_comboboxentry_action->indicator), GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_SMALL_TOOLBAR);
      gtk_widget_set_tooltip_text( ink_comboboxentry_action->indicator, ink_comboboxentry_action->warning );
#endif // GTK_CHECK_VERSION(2,16,0)
    } else {
#if GTK_CHECK_VERSION(2,16,0)
      gtk_entry_set_icon_from_icon_name( GTK_ENTRY(ink_comboboxentry_action->entry),
                                         GTK_ENTRY_ICON_SECONDARY,
                                         NULL );
#else // GTK_CHECK_VERSION(2,16,0)
      gtk_image_set_from_stock( GTK_IMAGE(ink_comboboxentry_action->indicator), NULL, GTK_ICON_SIZE_SMALL_TOOLBAR);
      gtk_widget_set_tooltip_text( ink_comboboxentry_action->indicator, NULL );
#endif // GTK_CHECK_VERSION(2,16,0)
    }
  }

  // Return if active text in list
  gboolean found = ( ink_comboboxentry_action->active != -1 );
  return found;
}

void ink_comboboxentry_action_set_width( Ink_ComboBoxEntry_Action* action, gint width ) {

  action->width = width;

  // Widget may not have been created....
  if( action->entry ) {
    gtk_entry_set_width_chars( GTK_ENTRY(action->entry), width );
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
    gtk_object_destroy( GTK_OBJECT( action->entry_completion ) );
    action->entry_completion = 0;
  }
}

void     ink_comboboxentry_action_set_warning( Ink_ComboBoxEntry_Action* action, const gchar* warning ) {

  g_free( action->warning );
  action->warning = g_strdup( warning );

  // Widget may not have been created....
  if( action->entry ) {
#if GTK_CHECK_VERSION(2,16,0)
    gtk_entry_set_icon_tooltip_text( GTK_ENTRY(action->entry),
                                     GTK_ENTRY_ICON_SECONDARY,
                                     action->warning );
#else // GTK_CHECK_VERSION(2,16,0)
    gtk_image_set_from_stock( GTK_IMAGE(action->indicator), action->warning ? GTK_STOCK_DIALOG_WARNING : 0, GTK_ICON_SIZE_SMALL_TOOLBAR );
#endif // GTK_CHECK_VERSION(2,16,0)
  }
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

// Return row of active text or -1 if not found.
gint get_active_row_from_text( Ink_ComboBoxEntry_Action* action, const gchar* target_text ) {

  // Check if text in list
  gint row = 0;
  gboolean found = false;
  GtkTreeIter iter;
  gboolean valid = gtk_tree_model_get_iter_first( action->model, &iter );
  while ( valid ) {

    // Get text from list entry
    gchar* text = 0;
    gtk_tree_model_get( action->model, &iter, 0, &text, -1 ); // Column 0

    // Check for match
    if( strcmp( target_text, text ) == 0 ){
      found = true;
      break;
    }
    ++row;
    valid = gtk_tree_model_iter_next( action->model, &iter );
  }

  if( !found ) row = -1;

  return row;

}


// Callbacks ---------------------------------------------------

static void combo_box_changed_cb( GtkComboBoxEntry* widget, gpointer data ) {

  // Two things can happen to get here:
  //   An item is selected in the drop-down menu.
  //   Text is typed.
  // We only react here if an item is selected.

  // Get action
  Ink_ComboBoxEntry_Action *act = INK_COMBOBOXENTRY_ACTION( data );

  // Check if item selected:
  gint newActive = gtk_combo_box_get_active( GTK_COMBO_BOX( widget ));
  if( newActive >= 0 ) {

    if( newActive != act->active ) {
      act->active = newActive;
      g_free( act->text );
      act->text = gtk_combo_box_get_active_text( GTK_COMBO_BOX( widget ));

      // Now let the world know
      g_signal_emit( G_OBJECT(act), signals[CHANGED], 0 );

    }
  }
}

static void entry_activate_cb( GtkEntry* widget, gpointer data ) {

  // Get text from entry box.. check if it matches a menu entry.

  // Get action
  Ink_ComboBoxEntry_Action *ink_comboboxentry_action = INK_COMBOBOXENTRY_ACTION( data );

  // Get text
  g_free( ink_comboboxentry_action->text );
  ink_comboboxentry_action->text = g_strdup( gtk_entry_get_text( widget ) );

  // Get row
  ink_comboboxentry_action->active =
    get_active_row_from_text( ink_comboboxentry_action, ink_comboboxentry_action->text );

  // Set active row
  gtk_combo_box_set_active( GTK_COMBO_BOX( ink_comboboxentry_action->combobox), ink_comboboxentry_action->active );

  // Now let the world know
  g_signal_emit( G_OBJECT(ink_comboboxentry_action), signals[CHANGED], 0 );

}

static gboolean match_selected_cb( GtkEntryCompletion* /*widget*/, GtkTreeModel* model, GtkTreeIter* iter, gpointer data )
{
  // Get action
  Ink_ComboBoxEntry_Action *ink_comboboxentry_action = INK_COMBOBOXENTRY_ACTION( data );
  GtkEntry *entry = ink_comboboxentry_action->entry;

  if( entry) {
    gchar *family = 0;
    gtk_tree_model_get(model, iter, 0, &family, -1);

    // Set text in GtkEntry
    gtk_entry_set_text (GTK_ENTRY (entry), family );

    // Set text in GtkAction
    g_free( ink_comboboxentry_action->text );
    ink_comboboxentry_action->text = family;

    // Get row
    ink_comboboxentry_action->active =
      get_active_row_from_text( ink_comboboxentry_action, ink_comboboxentry_action->text );

    // Set active row
    gtk_combo_box_set_active( GTK_COMBO_BOX( ink_comboboxentry_action->combobox), ink_comboboxentry_action->active );

    // Now let the world know
    g_signal_emit( G_OBJECT(ink_comboboxentry_action), signals[CHANGED], 0 );

    return true;
  }
  return false;
}

