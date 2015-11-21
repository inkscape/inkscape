/*
 * Factory for ToolBase tree
 *
 * Authors:
 *   Markus Engel
 *
 * Copyright (C) 2013 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "tool-factory.h"

#include "ui/tools/arc-tool.h"
#include "ui/tools/box3d-tool.h"
#include "ui/tools/calligraphic-tool.h"
#include "ui/tools/connector-tool.h"
#include "ui/tools/dropper-tool.h"
#include "ui/tools/eraser-tool.h"

#if HAVE_POTRACE
# include "ui/tools/flood-tool.h"
#endif

#include "ui/tools/gradient-tool.h"
#include "ui/tools/lpe-tool.h"
#include "ui/tools/measure-tool.h"
#include "ui/tools/mesh-tool.h"
#include "ui/tools/node-tool.h"
#include "ui/tools/pencil-tool.h"
#include "ui/tools/pen-tool.h"
#include "ui/tools/rect-tool.h"
#include "ui/tools/select-tool.h"
#include "ui/tools/spiral-tool.h"
#include "ui/tools/spray-tool.h"
#include "ui/tools/star-tool.h"
#include "ui/tools/text-tool.h"
#include "ui/tools/tool-base.h"
#include "ui/tools/tweak-tool.h"
#include "ui/tools/zoom-tool.h"

using namespace Inkscape::UI::Tools;

ToolBase *ToolFactory::createObject(std::string const& id)
{
    ToolBase *tool = NULL;

    if (id == "/tools/shapes/arc")
        tool = new ArcTool;
    else if (id == "/tools/shapes/3dbox")
        tool = new Box3dTool;
    else if (id == "/tools/calligraphic")
        tool = new CalligraphicTool;
    else if (id == "/tools/connector")
        tool = new ConnectorTool;
    else if (id == "/tools/dropper")
        tool = new DropperTool;
    else if (id == "/tools/eraser")
        tool = new EraserTool;
#if HAVE_POTRACE
    else if (id == "/tools/paintbucket")
        tool = new FloodTool;
#endif
    else if (id == "/tools/gradient")
        tool = new GradientTool;
    else if (id == "/tools/lpetool")
        tool = new LpeTool;
    else if (id == "/tools/measure")
        tool = new MeasureTool;
    else if (id == "/tools/mesh")
        tool = new MeshTool;
    else if (id == "/tools/nodes")
        tool = new NodeTool;
    else if (id == "/tools/freehand/pencil")
        tool = new PencilTool;
    else if (id == "/tools/freehand/pen")
        tool = new PenTool;
    else if (id == "/tools/shapes/rect")
        tool = new RectTool;
    else if (id == "/tools/select")
        tool = new SelectTool;
    else if (id == "/tools/shapes/spiral")
        tool = new SpiralTool;
    else if (id == "/tools/spray")
        tool = new SprayTool;
    else if (id == "/tools/shapes/star")
        tool = new StarTool;
    else if (id == "/tools/text")
        tool = new TextTool;
    else if (id == "/tools/tweak")
        tool = new TweakTool;
    else if (id == "/tools/zoom")
        tool = new ZoomTool;
    else
        fprintf(stderr, "WARNING: unknown tool: %s", id.c_str());

    return tool;
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
