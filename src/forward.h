#ifndef FORWARD_H_SEEN
#define FORWARD_H_SEEN

/*
 * Forward declarations of most used objects
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib-object.h>

/* Generic containers */

namespace Inkscape {
struct Application;
struct ApplicationClass;
namespace XML {
class Document;
class DocumentTree;
}
}

/* Editing window */

class SPDesktop;
class SPDesktopClass;

class SPDesktopWidget;
class SPDesktopWidgetClass;

GType sp_desktop_get_type ();

class SPEventContext;
class SPEventContextClass;

#define SP_TYPE_EVENT_CONTEXT (sp_event_context_get_type ())
#define SP_EVENT_CONTEXT(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_EVENT_CONTEXT, SPEventContext))
#define SP_IS_EVENT_CONTEXT(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_EVENT_CONTEXT))

GType sp_event_context_get_type ();

/* Document tree */

class SPDocumentClass;

#define SP_TYPE_DOCUMENT (sp_document_get_type ())
#define SP_DOCUMENT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_DOCUMENT, Document))
#define SP_IS_DOCUMENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_DOCUMENT))

GType sp_document_get_type ();

/* Objects */

class SPObject;
class SPObjectClass;

#define SP_TYPE_OBJECT (sp_object_get_type ())
#define SP_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_OBJECT, SPObject))
#define SP_OBJECT_CLASS(clazz) (G_TYPE_CHECK_CLASS_CAST((clazz), SP_TYPE_OBJECT, SPObjectClass))
#define SP_IS_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_OBJECT))

GType sp_object_get_type ();

class SPItem;
class SPItemClass;

#define SP_TYPE_ITEM (sp_item_get_type ())
#define SP_ITEM(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SP_TYPE_ITEM, SPItem))
#define SP_ITEM_CLASS(clazz) (G_TYPE_CHECK_CLASS_CAST((clazz), SP_TYPE_ITEM, SPItemClass))
#define SP_IS_ITEM(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SP_TYPE_ITEM))

GType sp_item_get_type ();

class SPGroup;
class SPGroupClass;

class SPDefs;
class SPDefsClass;

class SPRoot;
class SPRootClass;

class SPNamedView;
class SPNamedViewClass;

class SPGuide;
class SPGuideClass;

class SPObjectGroup;
class SPObjectGroupClass;

struct SPMarker;
struct SPMarkerClass;
class SPMarkerReference;

class SPPath;
class SPPathClass;

class SPShape;
class SPShapeClass;

class SPPolygon;
class SPPolygonClass;

class SPEllipse;
class SPEllipseClass;

class SPCircle;
class SPCircleClass;

class SPArc;
class SPArcClass;

class SPChars;
class SPCharsClass;

class SPText;
class SPTextClass;

class SPTSpan;
class SPTSpanClass;

class SPString;
class SPStringClass;

class SPPaintServer;
class SPPaintServerClass;

class SPStop;
class SPStopClass;

class SPGradient;
class SPGradientClass;
class SPGradientReference;

class SPLinearGradient;
class SPLinearGradientClass;

class SPRadialGradient;
class SPRadialGradientClass;

class SPPattern;

class SPClipPath;
class SPClipPathClass;
class SPClipPathReference;

class SPMaskReference;

class SPAvoidRef;

class SPAnchor;
class SPAnchorClass;

/* Misc */

class ColorRGBA;

class SPColor;

class SPStyle;

class SPEvent;

class SPPrintContext;

namespace Inkscape {
namespace UI {
namespace View {
class View;
};
};
};

class SPViewWidget;
class SPViewWidgetClass;

class StopOnTrue;

namespace Inkscape {
class URI;
class URIReference;
}

struct box_solution;


/* verbs */

typedef int sp_verb_t;
namespace Inkscape {
    class Verb;
}

#endif // FORWARD_H_SEEN

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
