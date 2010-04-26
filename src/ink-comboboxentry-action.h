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

#ifndef SEEN_INK_COMBOBOXENTRY_ACTION
#define SEEN_INK_COMBOBOXENTRY_ACTION

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtkaction.h>
#include <gtk/gtktreemodel.h>


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
  GtkComboBoxEntry   *combobox;
  GtkEntry           *entry;
  GtkEntryCompletion *entry_completion;
#if !GTK_CHECK_VERSION(2,16,0)
  GtkWidget          *indicator;
#endif

  gpointer            cell_data_func; // drop-down menu format

  gint                active;     // Index of active menu item (-1 if not in list).
  gchar              *text;       // Text of active menu item or entry box.
  gint                width;      // Width of GtkComboBoxEntry in characters.
  gboolean            popup;      // Do we pop-up an entry-completion dialog?
  gchar              *warning;    // Text for warning that entry isn't in list.
  gchar              *altx_name;  // Target for Alt-X keyboard shortcut.
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
                                                         gint          width = -1,
                                                         gpointer cell_data_func = NULL );

GtkTreeModel     *ink_comboboxentry_action_get_model( Ink_ComboBoxEntry_Action* action );
GtkComboBoxEntry *ink_comboboxentry_action_get_comboboxentry( Ink_ComboBoxEntry_Action* action );

gchar*   ink_comboboxentry_action_get_active_text( Ink_ComboBoxEntry_Action* action );
gboolean ink_comboboxentry_action_set_active_text( Ink_ComboBoxEntry_Action* action, const gchar* text );

void     ink_comboboxentry_action_set_width( Ink_ComboBoxEntry_Action* action, gint width );

void     ink_comboboxentry_action_popup_enable(  Ink_ComboBoxEntry_Action* action );
void     ink_comboboxentry_action_popup_disable( Ink_ComboBoxEntry_Action* action );

void     ink_comboboxentry_action_set_warning( Ink_ComboBoxEntry_Action* action, const gchar* warning );

void     ink_comboboxentry_action_set_altx_name( Ink_ComboBoxEntry_Action* action, const gchar* altx_name );

#endif /* SEEN_INK_COMBOBOXENTRY_ACTION */
