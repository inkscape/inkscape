/*
 * A subclass of GtkAction that wraps a GtkComboBoxEntry.
 * Features:
 *   Setting GtkEntryBox width in characters.
 *   Passing a function for formatting cells.
 *   Displaying a warning if text isn't in list.
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

#ifndef SEEN_INK_COMBOBOXENTRY_ACTION
#define SEEN_INK_COMBOBOXENTRY_ACTION

#include <gtk/gtk.h>

#define INK_COMBOBOXENTRY_TYPE_ACTION           (ink_comboboxentry_action_get_type())
#define INK_COMBOBOXENTRY_ACTION(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), INK_COMBOBOXENTRY_TYPE_ACTION, Ink_ComboBoxEntry_Action))
#define INK_COMBOBOXENTRY_ACTION_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass),  INK_COMBOBOXENTRY_TYPE_ACTION, Ink_ComboBoxEntry_ActionClass))
#define INK_COMBOBOXENTRY_IS_ACTION(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), INK_COMBOBOXENTRY_TYPE_ACTION))
#define INK_COMBOBOXENTRY_ACTION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj),  INK_COMBOBOXENTRY_TYPE_ACTION, Ink_ComboBoxEntry_ActionClass))

typedef struct _Ink_ComboBoxEntry_ActionClass Ink_ComboBoxEntry_ActionClass;
typedef struct _Ink_ComboBoxEntry_Action      Ink_ComboBoxEntry_Action;

struct _Ink_ComboBoxEntry_ActionClass {
  GtkActionClass parent_class;

  void (*changed)   (Ink_ComboBoxEntry_Action* action);
  void (*activated) (Ink_ComboBoxEntry_Action* action);
};

struct _Ink_ComboBoxEntry_Action {
  GtkAction parent_instance;

  GtkTreeModel       *model;
  GtkComboBox        *combobox;
  GtkEntry           *entry;
  GtkEntryCompletion *entry_completion;
  GtkWidget          *indicator;

  gpointer            cell_data_func; // drop-down menu format
  gpointer            separator_func;

  gint                active;     // Index of active menu item (-1 if not in list).
  gchar              *text;       // Text of active menu item or entry box.
  gint                entry_width;// Width of GtkEntry in characters.
  gint                extra_width;// Extra Width of GtkComboBox.. to widen drop-down list in list mode.
  gboolean            popup;      // Do we pop-up an entry-completion dialog?
  gchar              *info;       // Text for tooltip info about entry.
  gpointer            info_cb;    // Callback for clicking info icon.
  gint                info_cb_id;
  gboolean            info_cb_blocked;
  gchar              *warning;    // Text for tooltip warning that entry isn't in list.
  gpointer            warning_cb; // Callback for clicking warning icon.
  gint                warning_cb_id;
  gboolean            warning_cb_blocked;
  gchar              *altx_name;  // Target for Alt-X keyboard shortcut.
  GtkWidget          *focusWidget;
};


GType ink_comboboxentry_action_get_type (void);

/**
 * Creates a GtkAction subclass that wraps a GtkComboBoxEntry object.
 */
Ink_ComboBoxEntry_Action *ink_comboboxentry_action_new ( const gchar  *name,
							 const gchar  *label,
							 const gchar  *tooltip,
							 const gchar  *stock_id,
							 GtkTreeModel *model,
							 gint          entry_width = -1,
							 gint          extra_width = -1,
							 gpointer cell_data_func = NULL,
							 gpointer separator_func = NULL,
							 GtkWidget* focusWidget = NULL);

GtkTreeModel     *ink_comboboxentry_action_get_model( Ink_ComboBoxEntry_Action* action );
GtkComboBox      *ink_comboboxentry_action_get_comboboxentry( Ink_ComboBoxEntry_Action* action );

gchar*   ink_comboboxentry_action_get_active_text( Ink_ComboBoxEntry_Action* action );
gboolean ink_comboboxentry_action_set_active_text( Ink_ComboBoxEntry_Action* action, const gchar* text, int row=-1 );

void     ink_comboboxentry_action_set_entry_width( Ink_ComboBoxEntry_Action* action, gint entry_width );
void     ink_comboboxentry_action_set_extra_width( Ink_ComboBoxEntry_Action* action, gint extra_width );

void     ink_comboboxentry_action_popup_enable(  Ink_ComboBoxEntry_Action* action );
void     ink_comboboxentry_action_popup_disable( Ink_ComboBoxEntry_Action* action );

void     ink_comboboxentry_action_set_info(      Ink_ComboBoxEntry_Action* action, const gchar* info );
void     ink_comboboxentry_action_set_info_cb(   Ink_ComboBoxEntry_Action* action, gpointer info_cb );
void     ink_comboboxentry_action_set_warning(   Ink_ComboBoxEntry_Action* action, const gchar* warning_cb );
void     ink_comboboxentry_action_set_warning_cb(Ink_ComboBoxEntry_Action* action, gpointer warning );
void     ink_comboboxentry_action_set_tooltip(   Ink_ComboBoxEntry_Action* action, const gchar* tooltip );

void     ink_comboboxentry_action_set_altx_name( Ink_ComboBoxEntry_Action* action, const gchar* altx_name );

#endif /* SEEN_INK_COMBOBOXENTRY_ACTION */
