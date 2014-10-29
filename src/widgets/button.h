#ifndef SEEN_SP_BUTTON_H
#define SEEN_SP_BUTTON_H

/**
 * Generic button widget
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * This code is in public domain
 */

#define SP_TYPE_BUTTON (sp_button_get_type ())
#define SP_BUTTON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_BUTTON, SPButton))
#define SP_IS_BUTTON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_BUTTON))

#include <gtk/gtk.h>
#include <sigc++/connection.h>
#include "icon-size.h"

struct SPAction;

namespace Inkscape {
namespace UI {
namespace View {
class View;
}
}
}

typedef enum {
	SP_BUTTON_TYPE_NORMAL,
	SP_BUTTON_TYPE_TOGGLE
} SPButtonType;

struct SPBChoiceData {
	guchar *px;
};

struct SPButton {
	GtkToggleButton widget;
	SPButtonType type;
	Inkscape::IconSize lsize;
	unsigned int psize;
	SPAction *action;
	SPAction *doubleclick_action;

	sigc::connection c_set_active;
	sigc::connection c_set_sensitive;
};

struct SPButtonClass {
	GtkToggleButtonClass parent_class;
};

#define SP_BUTTON_IS_DOWN(b) gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (b))

GType sp_button_get_type (void);

GtkWidget *sp_button_new (Inkscape::IconSize size, SPButtonType type, SPAction *action, SPAction *doubleclick_action);

void sp_button_toggle_set_down (SPButton *button, gboolean down);

GtkWidget *sp_button_new_from_data (Inkscape::IconSize size,
				    SPButtonType type,
				    Inkscape::UI::View::View *view,
				    const gchar *name,
				    const gchar *tip);

#endif // !SEEN_SP_BUTTON_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
