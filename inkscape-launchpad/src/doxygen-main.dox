/**
 * @file
 * Doxygen documentation - main page and namespace documentation.
 */
/* Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2005-2008 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

// Note: % before a word prevents that word from being linkified

/**
 * Main %Inkscape namespace.
 *
 * This namespace contains all code internal to %Inkscape.
 */
namespace Inkscape {

/**
 * Some STL-style algorithms.
 *
 * This namespace contains a few generic algorithms used with the %XML tree.
 */
namespace Algorithms {}


/**
 * Debugging utilities.
 *
 * This namespace contains various debugging code which can help developers
 * to pinpoint problems with their (or others') code.
 */
namespace Debug {}

/**
 * Rendering-related code.
 *
 * This namespace contains code related to the renderer.
 */
namespace Display {}

/**
 * Extension support.
 *
 * This namespace contains the extension subsystem and implementations
 * of the internal extensions. This includes input and output filters, bitmap
 * extensions, and printing.
 */
namespace Extension {}

/**
 * Boehm-GC based garbage collector.
 *
 * This namespace contains code related to the garbage collector and base
 * classes for %GC-managed objects.
 */
namespace GC {}

/**
 * Low-level IO code.
 *
 * This namespace contains low level IO-related code, including a homegrown
 * streams implementation, routines for formatting SVG output, and some
 * file handling utility functions.
 */
namespace IO {}

/**
 * Live Path Effects code.
 *
 * This namespace contains classes and functions related to the implementation
 * of Live Path Effects, which apply arbitrary transformation to a path and
 * update whenever the original path is modified.
 */
namespace LivePathEffect {}

/**
 * Tracing backend.
 *
 * This namespace contains the integrated potrace-based tracing backend, used
 * in the Trace Bitmap and Paint Bucket features.
 */
namespace Trace {}

/**
 * User interface code.
 *
 * This namespace contains everything related to the user interface of Inkscape.
 */
namespace UI {

/**
 * Dialog code.
 *
 * This namespace contains all code related to dialogs.
 */
namespace Dialog {}

/**
 * Custom widgets.
 *
 * This namespace contains custom user interface widgets used thorough
 * Inkscape.
 */
namespace Widget {}

} // namespace UI

/**
 * Miscellaneous supporting code.
 *
 * This namespace contains miscellaneous low-level code: an implementation of
 * garbage-collected lists, tuples, generic pointer iterators and length unit
 * handling.
 */
namespace Util {}

/**
 * @Inkscape %XML tree.
 *
 * This namespace contains classes and functions that comprise the XML tree
 * of Inkscape documents.
 *
 * SVG documents in Inkscape are represented as two parallel hierarchies
 * of nodes: the object tree, which contains all information about
 * the document's state when loaded, and the %XML tree, which contains all
 * information about the document's %XML representation. For this reason
 * this tree is also called the "repr tree", and %XML nodes are called "reprs".
 *
 * The central class is XML::Node, which provides all operations. It should be
 * noted that nodes are currently typeless and operations not valid for their
 * type simply do nothing (like trying to iterate over children of a text node).
 * In addition to standard DOM operations, the %XML tree supports observers -
 * objects derived from Xml::NodeObserver which receive notifications about
 * changes in the document tree.
 */
namespace XML {}

} // namespace Inkscape

/** \mainpage Inkscape Source Code Documentation
 * While the standard doxygen documentation can be accessed through the links
 * in the header, the following documents are additionally available to the
 * interested reader.
 *
 * \section groups Main directory documentation
 * Inkscape's classes and files in the main directory can be grouped into
 * the following categories:
 *
 * - \subpage ObjectTree - inkscape's SVG canvas
 * - \subpage Tools - the tools UI
 * - \subpage UI - inkscape's user interface
 * - \subpage XmlTree - XML backbone of the document
 * - \subpage Rendering - rendering and buffering
 * - \subpage OtherServices - what doesn't fit in the above
 *
 * See also the <a href="dirs.html">other directories</a> until doxygen
 * allows setting links to those doc files.
 *
 * \section extlinks Links to external documentation
 *
 * \subsection liblinks External documentation on libraries used in inkscape
 *
 * C++:
 * <a href="http://www.gtkmm.org/documentation.shtml">gtkmm</a>
 * <a href="http://www.gtkmm.org/docs/gtkmm-2.4/docs/reference/html/namespaceAtk.html">atkmm</a>
 * <a href="http://www.gtkmm.org/docs/gtkmm-2.4/docs/reference/html/namespaceGdk.html">gdkmm</a>
 * <a href="http://www.gtkmm.org/docs/glibmm-2.4/docs/reference/html/namespaceGlib.html">glibmm</a>
 * <a href="http://www.gtkmm.org/docs/pangomm-1.4/docs/reference/html/namespacePango.html">pangomm</a>
 * <a href="http://libsigc.sourceforge.net/libsigc2/docs/index.html">libsigc++</a>
 * C:
 * <a href="http://www.gtk.org/api/">GTK+</a>
 * <a href="http://developer.gnome.org/doc/API/2.0/gdk-pixbuf/index.html">gdk-pixbuf</a>
 * <a href="http://developer.gnome.org/doc/API/2.0/gobject/index.html">GObject</a>
 * <a href="http://developer.gnome.org/doc/API/2.0/atk/index.html">atk</a>
 * <a href="http://developer.gnome.org/doc/API/2.0/pango/index.html">pango</a>
 * <a href="http://developer.gnome.org/doc/API/2.0/ORBit/index.html">ORBit</a>
 * <a href="http://developer.gnome.org/doc/API/2.0/libbonobo/index.html">bonobo</a>
 * <a href="http://developer.gnome.org/doc/API/2.0/bonobo-activation/index.html">bonobo-activation</a>
 * <a href="http://xmlsoft.org/XSLT/html/libxslt-lib.html#LIBXSLT-LIB">libxslt</a>
 * <a href="http://xmlsoft.org/html/index.html">libxml2</a>
 * Legacy:
 * <a href="http://developer.gnome.org/doc/API/2.0/gnome-vfs-2.0/">GnomeVFS</a>
 *
 * \subsection stdlinks External standards documentation
 *
 * <a href="http://www.w3.org/TR/SVG/">SVG1.1</a>
 * <a href="http://www.w3.org/TR/SVG12/">SVG1.2</a>
 * <a href="http://www.w3.org/TR/SVGMobile/">SVGMobile</a>
 * <a href="http://www.w3.org/Graphics/SVG/Test/">SVGTest</a>
 * <a href="http://www.libpng.org/pub/png/">PNG</a>
 * <a href="http://www.w3.org/TR/xslt">XSLT</a>
 * <a href="http://partners.adobe.com/public/developer/ps/index_specs.html">PostScript</a>
 * <a href="http://developer.gnome.org/projects/gup/hig/">Gnome-HIG</a>
 */

/** \page ObjectTree Object Tree Classes and Files
 * Inkscape::ObjectHierarchy [\ref object-hierarchy.cpp, \ref object-hierarchy.h]
 * - SPObject [\ref sp-object.cpp, \ref sp-object.h, \ref object-edit.cpp, \ref sp-object-repr.cpp]
 *   - SPDefs [\ref sp-defs.cpp, \ref sp-defs.h]
 *   - SPFlowline [\ref sp-flowdiv.cpp, \ref sp-flowdiv.h]
 *   - SPFlowregionbreak [\ref sp-flowdiv.cpp, \ref sp-flowdiv.h]
 *   - SPGuide [\ref sp-guide.cpp, \ref sp-guide.h]
 *   - SPItem [\ref sp-item.cpp, \ref sp-item.h, \ref sp-item-notify-moveto.cpp, \ref sp-item-rm-unsatisfied-cns.cpp, \ref sp-item-transform.cpp, \ref sp-item-update-cns.cpp, ]
 *     - SPFlowdiv [\ref sp-flowdiv.cpp, \ref sp-flowdiv.h]
 *     - SPFlowpara [\ref sp-flowdiv.cpp, \ref sp-flowdiv.h]
 *     - SPFlowregion [\ref sp-flowregion.cpp, \ref sp-flowregion.h]
 *     - SPFlowregionExclude [\ref sp-flowregion.cpp, \ref sp-flowregion.h]
 *     - SPFlowtext [\ref sp-flowtext.cpp, \ref sp-flowtext.h]
 *     - SPFlowtspan [\ref sp-flowdiv.cpp, \ref sp-flowdiv.h]
 *     - SPGroup [\ref sp-item-group.cpp, \ref sp-item-group.h]
 *       - SPAnchor [\ref sp-anchor.cpp, \ref sp-anchor.h]
 *       - SPMarker [\ref marker.cpp, \ref marker.h]
 *       - SPRoot [\ref sp-root.cpp, \ref sp-root.h]
 *       - SPSymbol [\ref sp-symbol.cpp, \ref sp-symbol.h]
 *     - SPImage [\ref sp-image.cpp, \ref sp-image.h]
 *     - SPShape [\ref sp-shape.cpp, \ref sp-shape.h]
 *       - SPGenericEllipse [\ref sp-ellipse.cpp, \ref sp-ellipse.h]
 *         - SPArc
 *         - SPCircle
 *         - SPEllipse
 *       - SPLine [\ref sp-line.cpp, \ref sp-line.h]
 *       - SPOffset [\ref sp-offset.cpp, \ref sp-offset.h]
 *       - SPPath [\ref sp-path.cpp, \ref sp-path.h, \ref path-chemistry.cpp, \ref splivarot.cpp]
 *       - SPPolygon [\ref sp-polygon.cpp, \ref sp-polygon.h]
 *         - SPStar [\ref sp-star.cpp, \ref sp-star.h]
 *       - SPPolyLine [\ref sp-polyline.cpp, \ref sp-polyline.h]
 *       - SPRect [\ref sp-rect.cpp, \ref sp-rect.h]
 *       - SPSpiral [\ref sp-spiral.cpp, \ref sp-spiral.h]
 *     - SPText [\ref sp-text.cpp, \ref sp-text.h, \ref text-chemistry.cpp, \ref text-editing.cpp]
 *     - SPTextPath [\ref sp-tspan.cpp, \ref sp-tspan.h]
 *     - SPTSpan [\ref sp-tspan.cpp, \ref sp-tspan.h]
 *     - SPUse [\ref sp-use.cpp, \ref sp-use.h]
 *   - SPMetadata [\ref sp-metadata.cpp, \ref sp-metadata.h]
 *   - SPObjectGroup [\ref sp-object-group.cpp, \ref sp-object-group.h]
 *     - SPClipPath [\ref sp-clippath.cpp, \ref sp-clippath.h]
 *     - SPMask [\ref sp-mask.cpp, \ref sp-mask.h]
 *     - SPNamedView [\ref sp-namedview.cpp, \ref sp-namedview.h]
 *   - SPPaintServer [\ref sp-paint-server.cpp, \ref sp-paint-server.h]
 *     - SPGradient [\ref sp-gradient.cpp, \ref sp-gradient.h, \ref gradient-chemistry.cpp, \ref sp-gradient-reference.h, \ref sp-gradient-spread.h, \ref sp-gradient-units.h, \ref sp-gradient-vector.h]
 *       - SPLinearGradient
 *       - SPRadialGradient
 *     - SPPattern [\ref sp-pattern.cpp, \ref sp-pattern.h]
 *   - SPStop [\ref sp-stop.h]
 *   - SPString [\ref sp-string.cpp, \ref sp-string.h]
 *   - SPStyleElem [\ref sp-style-elem.cpp, \ref sp-style-elem.h]
 *
 */
/** \page Tools Tools Related Classes and Files
 *
 * SelCue [\ref selcue.cpp, \ref selcue.h, \ref rubberband.cpp]
 * Inkscape::Selection [\ref selection.cpp, \ref selection.h, \ref selection-chemistry.cpp]
 * SPSelTrans [\ref seltrans.cpp, \ref seltrans.h]
 *
 * \section Event Context Class Hierarchy
 *
 *- ToolBase[\ref event-context.cpp, \ref event-context.h]
 * - ArcTool [\ref arc-context.cpp, \ref arc-context.h]
 * - SPDrawContext [\ref draw-context.cpp, \ref draw-context.h]
 *   - PenTool [\ref pen-context.cpp, \ref pen-context.h]
 *   - PencilTool [\ref pencil-context.cpp, \ref pencil-context.h]
 *   - ConnectorTool [\ref connector-context.cpp, \ref connector-context.h, \ref sp-conn-end.cpp, \ref sp-conn-end-pair.cpp]
 * - GradientTool [\ref gradient-context.cpp, \ref gradient-context.h, \ref gradient-drag.cpp, \ref gradient-toolbar.cpp]
 * - RectTool [\ref rect-context.cpp, \ref rect-context.h]
 * - SelectTool [\ref select-context.cpp, \ref select-context.h]
 * - SpiralTool [\ref spiral-context.cpp, \ref spiral-context.h]
 * - StarTool [\ref star-context.cpp, \ref star-context.h]
 * - FloodContext [\ref flood-context.cpp, \ref flood-context.h]
 * - Box3dTool [\ref box3d-context.cpp, \ref box3d-context.h]
 *
 * SPNodeContext [\ref node-context.cpp, \ref node-context.h]
 *
 * ZoomTool [\ref zoom-context.cpp, \ref zoom-context.h]
 *
 * CalligraphicTool [\ref dyna-draw-context.cpp, \ref dyna-draw-context.h]
 *
 * DropperTool [\ref dropper-context.cpp, \ref dropper-context.h]
 */
/** \page UI User Interface Classes and Files
 *
 * - Inkscape::UI::View::View [\ref ui/view/view.cpp, \ref ui/view/view.h]
 *   - SPDesktop [\ref desktop.cpp, \ref desktop-events.cpp, \ref desktop-handles.cpp, \ref desktop-style.cpp, \ref desktop.h, \ref desktop-events.h, \ref desktop-handles.h, \ref desktop-style.h]
 *   - SPSVGView [\ref svg-view.cpp, \ref svg-view.h]
 *
 * SPDesktopWidget [\ref desktop-widget.h] SPSVGSPViewWidget [\ref svg-view.cpp]
 * SPDocument [\ref document.cpp, \ref document.h]
 *
 * SPDrawAnchor [\ref draw-anchor.cpp, \ref draw-anchor.h]
 * SPKnot [\ref knot.cpp, \ref knot.h, \ref knot-enums.h]
 * SPKnotHolder [\ref knotholder.cpp, \ref knotholder.h, \ref knot-holder-entity.h]
 *
 * [\ref layer-fns.cpp, \ref selection-describer.h]
 * Inkscape::MessageContext [\ref message-context.h]
 * Inkscape::MessageStack [\ref message-stack.h, \ref message.h]
 *
 * Snapper, GridSnapper, GuideSnapper [\ref snap.cpp, \ref snap.h]
 *
 * SPGuide [\ref sp-guide.cpp, \ref sp-guide.h, \ref satisfied-guide-cns.cpp, \ref sp-guide-attachment.h, \ref sp-guide-constraint.h]
 *
 * [\ref help.cpp] [\ref inkscape.cpp] [\ref inkscape-stock.cpp]
 * [\ref interface.cpp] [\ref main.cpp, \ref winmain.cpp]
 * [\ref menus-skeleton.h, \ref preferences-skeleton.h]
 * [\ref select-toolbar.cpp] [\ref shortcuts.cpp]
 * [\ref sp-cursor.cpp] [\ref text-edit.cpp] [\ref toolbox.cpp]
 * Inkscape::Verb [\ref verbs.h]
 *
 */
/** \page XmlTree CSS/XML Tree Classes and Files
 *
 * SPStyle [\ref style.cpp, \ref style.h]
 * Media [\ref media.cpp, \ref media.h]
 * [\ref attributes.cpp, \ref attributes.h]
 *
 * - Inkscape::URIReference [\ref uri-references.cpp, \ref uri-references.h]
 *   - SPClipPathReference [\ref sp-clippath.h]
 *   - SPGradientReference [\ref sp-gradient-reference.h]
 *   - SPMarkerReference [\ref marker.h]
 *   - SPMaskReference [\ref sp-mask.h]
 *   - SPUseReference [\ref sp-use-reference.h]
 *     - SPUsePath
 */
/** \page Rendering Rendering Related Classes and Files
 *
 * SPColor [\ref color.cpp, \ref color.h, \ref color-rgba.h]
 * [\ref geom.cpp] [\ref mod360.cpp]
 */
/** \page OtherServices Classes and Files From Other Services
 * [\ref inkview.cpp, \ref slideshow.cpp]
 *
 * Inkscape::GC
 *
 * [\ref prefs-utils.cpp] [\ref print.cpp]
 *
 * - Inkscape::GZipBuffer [\ref streams-gzip.h]
 * - Inkscape::JarBuffer [\ref streams-jar.h]
 * - Inkscape::ZlibBuffer [\ref streams-zlib.h]
 * - Inkscape::URIHandle [\ref streams-handles.h]
 *   - Inkscape::FileHandle
 * [\ref dir-util.cpp] [\ref file.cpp]
 * Inkscape::URI [\ref uri.h, \ref extract-uri.cpp, \ref uri-references.cpp]
 * Inkscape::BadURIException [\ref bad-uri-exception.h]
 *
 * Inkscape::Whiteboard::UndoStackObserver [\ref undo-stack-observer.cpp, \ref composite-undo-stack-observer.cpp]
 * [\ref document-undo.cpp]
 *
 * {\ref dialogs/} [\ref decimal-round.h] [\ref enums.h]
 */


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
