#ifndef SEEN_SP_MESH_CONTEXT_H
#define SEEN_SP_MESH_CONTEXT_H

/*
 * Mesh drawing and editing tool
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org.
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 2012 Tavmjong Bah
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2005,2010 Authors
 *
 * Released under GNU GPL
 */

#include <stddef.h>
#include <sigc++/sigc++.h>
#include "ui/tools/tool-base.h"
#include "sp-mesh-array.h"

#define SP_MESH_CONTEXT(obj) (dynamic_cast<Inkscape::UI::Tools::MeshTool*>((Inkscape::UI::Tools::ToolBase*)obj))
#define SP_IS_MESH_CONTEXT(obj) (dynamic_cast<const Inkscape::UI::Tools::MeshTool*>((const Inkscape::UI::Tools::ToolBase*)obj) != NULL)

namespace Inkscape {
namespace UI {
namespace Tools {

class MeshTool : public ToolBase {
public:
    MeshTool();
    virtual ~MeshTool();

    Geom::Point origin;

    Geom::Point mousepoint_doc; // stores mousepoint when over_line in doc coords

    sigc::connection *selcon;
    sigc::connection *subselcon;

    static const std::string prefsPath;

    virtual void setup();
    virtual void set(const Inkscape::Preferences::Entry& val);
    virtual bool root_handler(GdkEvent* event);

    virtual const std::string& getPrefsPath();

private:
    void selection_changed(Inkscape::Selection* sel);

    bool cursor_addnode;
    bool node_added;
    bool show_handles;
    bool edit_fill;
    bool edit_stroke;


};

void sp_mesh_context_select_next(ToolBase *event_context);
void sp_mesh_context_select_prev(ToolBase *event_context);
void sp_mesh_context_corner_operation(MeshTool *event_context, MeshCornerOperation operation );
void sp_mesh_context_fit_mesh_in_bbox(MeshTool *event_context);

}
}
}

#endif // SEEN_SP_MESH_CONTEXT_H


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
