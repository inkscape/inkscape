#ifndef __SP_TEXT_CONTEXT_H__
#define __SP_TEXT_CONTEXT_H__

/*
 * TextTool
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <sigc++/connection.h>

#include "ui/tools/tool-base.h"
#include <2geom/point.h>
#include "libnrtype/Layout-TNG.h"

#define SP_TEXT_CONTEXT(obj) (dynamic_cast<Inkscape::UI::Tools::TextTool*>((Inkscape::UI::Tools::ToolBase*)obj))
#define SP_IS_TEXT_CONTEXT(obj) (dynamic_cast<const Inkscape::UI::Tools::TextTool*>((const Inkscape::UI::Tools::ToolBase*)obj) != NULL)

typedef struct _GtkIMContext GtkIMContext;

struct SPCtrlLine;

namespace Inkscape {
namespace UI {
namespace Tools {

class TextTool : public ToolBase {
public:
	TextTool();
	virtual ~TextTool();

    sigc::connection sel_changed_connection;
    sigc::connection sel_modified_connection;
    sigc::connection style_set_connection;
    sigc::connection style_query_connection;

    GtkIMContext *imc;

    SPItem *text; // the text we're editing, or NULL if none selected

    /* Text item position in root coordinates */
    Geom::Point pdoc;
    /* Insertion point position */
    Inkscape::Text::Layout::iterator text_sel_start;
    Inkscape::Text::Layout::iterator text_sel_end;

    gchar uni[9];
    bool unimode;
    guint unipos;

    SPCtrlLine *cursor;
    SPCanvasItem *indicator;
    SPCanvasItem *frame; // hiliting the first frame of flowtext; FIXME: make this a list to accommodate arbitrarily many chained shapes
    std::vector<SPCanvasItem*> text_selection_quads;
    gint timeout;
    bool show;
    bool phase;
    bool nascent_object; // true if we're clicked on canvas to put cursor, but no text typed yet so ->text is still NULL

    bool over_text; // true if cursor is over a text object

    guint dragging : 2; // dragging selection over text

    bool creating; // dragging rubberband to create flowtext
    SPCanvasItem *grabbed; // we grab while we are creating, to get events even if the mouse goes out of the window
    Geom::Point p0; // initial point if the flowtext rect

    /* Preedit String */
    gchar* preedit_string;

	static const std::string prefsPath;

	virtual void setup();
	virtual void finish();
	virtual bool root_handler(GdkEvent* event);
	virtual bool item_handler(SPItem* item, GdkEvent* event);

	virtual const std::string& getPrefsPath();

private:
    void _selectionChanged(Inkscape::Selection *selection);
    void _selectionModified(Inkscape::Selection *selection, guint flags);
    bool _styleSet(SPCSSAttr const *css);
    int _styleQueried(SPStyle *style, int property);
};

bool sp_text_paste_inline(ToolBase *ec);
Glib::ustring sp_text_get_selected_text(ToolBase const *ec);
SPCSSAttr *sp_text_get_style_at_cursor(ToolBase const *ec);
bool sp_text_delete_selection(ToolBase *ec);
void sp_text_context_place_cursor (TextTool *tc, SPObject *text, Inkscape::Text::Layout::iterator where);
void sp_text_context_place_cursor_at (TextTool *tc, SPObject *text, Geom::Point const p);
Inkscape::Text::Layout::iterator *sp_text_context_get_cursor_position(TextTool *tc, SPObject *text);

}
}
}

#endif
