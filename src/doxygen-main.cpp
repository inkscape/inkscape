/** \file
 * Contains Doxygen documentation - main page.
 *
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005-2006 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/** \mainpage The Inkscape Source Code Documentation
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
 * <a href="http://www.gtkmm.org/gtkmm2/docs/">Gtkmm</a>
 * <a href="http://www.gtkmm.org/gtkmm2/docs/reference/html/dir_000003.html">atkmm</a>
 * <a href="http://www.gtkmm.org/gtkmm2/docs/reference/html/dir_000009.html">gdkmm</a>
 * <a href="http://www.gtkmm.org/gtkmm2/docs/reference/html/dir_000007.html">pangomm</a>
 * <a href="http://libsigc.sourceforge.net/libsigc1_2/reference/html/modules.html">libsigc++</a>
 * <a href="http://www.gtk.org/api/">GTK+</a>
 * <a href="http://developer.gnome.org/doc/API/2.0/gdk-pixbuf/index.html">gdk-pixbuf</a>
 * <a href="http://developer.gnome.org/doc/API/2.0/gobject/index.html">GObject</a>
 * <a href="http://developer.gnome.org/doc/API/2.0/atk/index.html">atk</a>
 * <a href="http://developer.gnome.org/doc/API/2.0/pango/index.html">pango</a>
 * <a href="http://developer.gnome.org/doc/API/2.0/gnome-vfs-2.0/">GnomeVFS</a>
 * <a href="http://libsigc.sourceforge.net/libsigc2/docs/index.html">libsigc</a>
 * <a href="http://developer.gnome.org/doc/API/2.0/ORBit/index.html">ORBit</a>
 * <a href="http://developer.gnome.org/doc/API/2.0/libbonobo/index.html">bonobo</a>
 * <a href="http://developer.gnome.org/doc/API/2.0/bonobo-activation/index.html">bonobo-activation</a>
 * <a href="http://xmlsoft.org/XSLT/html/libxslt-lib.html#LIBXSLT-LIB">libxslt</a>
 * <a href="http://xmlsoft.org/html/index.html">libxml2</a>
 *
 * \subsection stdlinks External standards documentation
 *
 * <a href="http://www.w3.org/TR/SVG/">SVG1.1</a>
 * <a href="http://www.w3.org/TR/SVG12/">SVG1.2</a>
 * <a href="http://www.w3.org/TR/SVGMobile/">SVGMobile</a>
 * <a href="http://www.w3.org/Graphics/SVG/Test/">SVGTest</a>
 * <a href="http://www.libpng.org/pub/png/">PNG</a>
 * <a href="http://www.w3.org/TR/xslt">XSLT</a>
 * <a href="http://partners.adobe.com/public/developer/ps/index_specs.html">PS</a>
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
 *       - SPPath [\ref sp-path.cpp, \ref sp-path.h, \ref path-chemistry.cpp, \ref nodepath.cpp, \ref nodepath.h, \ref splivarot.cpp]
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
 *   - SPSkeleton [\ref sp-skeleton.cpp, \ref sp-skeleton.h]
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
 *- SPEventContext[\ref event-context.cpp, \ref event-context.h]
 * - SPArcContext [\ref arc-context.cpp, \ref arc-context.h]
 * - SPDrawContext [\ref draw-context.cpp, \ref draw-context.h]
 *   - SPPenContext [\ref pen-context.cpp, \ref pen-context.h]
 *   - SPPencilContext [\ref pencil-context.cpp, \ref pencil-context.h]
 *   - SPConnectorContext [\ref connector-context.cpp, \ref connector-context.h, \ref sp-conn-end.cpp, \ref sp-conn-end-pair.cpp]
 * - SPGradientContext [\ref gradient-context.cpp, \ref gradient-context.h, \ref gradient-drag.cpp, \ref gradient-toolbar.cpp]
 * - SPRectContext [\ref rect-context.cpp, \ref rect-context.h]
 * - SPSelectContext [\ref select-context.cpp, \ref select-context.h]
 * - SPSpiralContext [\ref spiral-context.cpp, \ref spiral-context.h]
 * - SPStarContext [\ref star-context.cpp, \ref star-context.h]
 * - FloodContext [\ref flood-context.cpp, \ref flood-context.h]
 * - Box3DContext [\ref box3d-context.cpp, \ref box3d-context.h]
 *
 * SPNodeContext [\ref node-context.cpp, \ref node-context.h]
 *
 * SPZoomContext [\ref zoom-context.cpp, \ref zoom-context.h]
 *
 * SPDynaDrawContext [\ref dyna-draw-context.cpp, \ref dyna-draw-context.h]
 *
 * SPDropperContext [\ref dropper-context.cpp, \ref dropper-context.h]
 */
/** \page UI User Interface Classes and Files
 *
 * - Inkscape::UI::View::View [\ref ui/view/view.cpp, \ref ui/view/view.h]
 *   - Inkscape::UI::View::Edit [\ref ui/view/edit.cpp, \ref ui/view/edit.h]
 *   - SPDesktop [\ref desktop.cpp, \ref desktop-affine.cpp, \ref desktop-events.cpp, \ref desktop-handles.cpp, \ref desktop-style.cpp, \ref desktop.h, \ref desktop-affine.h, \ref desktop-events.h, \ref desktop-handles.h, \ref desktop-style.h]
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
 * [\ref interface.cpp, \ref memeq.h] [\ref main.cpp, \ref winmain.cpp]
 * [\ref menus-skeleton.h, \ref preferences-skeleton.h]
 * [\ref context-menu.cpp] [\ref select-toolbar.cpp] [\ref shortcuts.cpp]
 * [\ref sp-cursor.cpp] [\ref text-edit.cpp] [\ref toolbox.cpp, \ref ui/widget/toolbox.cpp]
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
 * [\ref inkview.cpp, \ref slideshow.cpp] [\ref sp-animation.cpp]
 *
 * Inkscape::GC
 *
 * [\ref sp-metrics.cpp, \ref sp-metrics.h]
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
 * {\ref dialogs/} [\ref approx-equal.h] [\ref decimal-round.h] [\ref enums.h] [\ref unit-constants.h]
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
