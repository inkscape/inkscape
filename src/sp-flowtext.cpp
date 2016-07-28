/*
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <glibmm/i18n.h>
#include <cstring>
#include <string>

#include "attributes.h"
#include "xml/repr.h"
#include "style.h"
#include "inkscape.h"
#include "document.h"
#include "selection.h"

#include "desktop.h"

#include "xml/repr.h"

#include "sp-flowdiv.h"
#include "sp-flowregion.h"
#include "sp-flowtext.h"
#include "sp-string.h"
#include "sp-use.h"
#include "sp-rect.h"
#include "text-tag-attributes.h"
#include "text-chemistry.h"
#include "text-editing.h"
#include "sp-text.h"

#include "libnrtype/font-instance.h"

#include "livarot/Shape.h"

#include "display/drawing-text.h"

SPFlowtext::SPFlowtext() : SPItem(),
    par_indent(0),
    _optimizeScaledText(false)
{
}

SPFlowtext::~SPFlowtext() {
}

void SPFlowtext::child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref) {
	SPItem::child_added(child, ref);

	this->requestModified(SP_OBJECT_MODIFIED_FLAG);
}


/* fixme: hide (Lauris) */

void SPFlowtext::remove_child(Inkscape::XML::Node* child) {
	SPItem::remove_child(child);

	this->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

void SPFlowtext::update(SPCtx* ctx, unsigned int flags) {
    SPItemCtx *ictx = (SPItemCtx *) ctx;
    SPItemCtx cctx = *ictx;

    unsigned childflags = flags;
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        childflags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }
    childflags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList *l = NULL;

    for (SPObject *child = this->firstChild() ; child ; child = child->getNext() ) {
        sp_object_ref(child);
        l = g_slist_prepend(l, child);
    }

    l = g_slist_reverse(l);

    while (l) {
        SPObject *child = reinterpret_cast<SPObject *>(l->data);
        g_assert(child != NULL);
        l = g_slist_remove(l, child);

        if (childflags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            SPItem *item = dynamic_cast<SPItem *>(child);
            if (item) {
                SPItem const &chi = *item;
                cctx.i2doc = chi.transform * ictx->i2doc;
                cctx.i2vp = chi.transform * ictx->i2vp;
                child->updateDisplay((SPCtx *)&cctx, childflags);
            } else {
                child->updateDisplay(ctx, childflags);
            }
        }

        sp_object_unref(child);
    }

    SPItem::update(ctx, flags);

    this->rebuildLayout();

    Geom::OptRect pbox = this->geometricBounds();

    for (SPItemView *v = this->display; v != NULL; v = v->next) {
        Inkscape::DrawingGroup *g = dynamic_cast<Inkscape::DrawingGroup *>(v->arenaitem);
        this->_clearFlow(g);
        g->setStyle(this->style);
        // pass the bbox of the flowtext object as paintbox (used for paintserver fills)
        this->layout.show(g, pbox);
    }
}

void SPFlowtext::modified(unsigned int flags) {
    SPObject *region = NULL;

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
    	flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    // FIXME: the below stanza is copied over from sp_text_modified, consider factoring it out
    if (flags & ( SP_OBJECT_STYLE_MODIFIED_FLAG )) {
        Geom::OptRect pbox = geometricBounds();

        for (SPItemView* v = display; v != NULL; v = v->next) {
            Inkscape::DrawingGroup *g = dynamic_cast<Inkscape::DrawingGroup *>(v->arenaitem);
            _clearFlow(g);
            g->setStyle(style);
            layout.show(g, pbox);
        }
    }

    for ( SPObject *o = this->firstChild() ; o ; o = o->getNext() ) {
        if (dynamic_cast<SPFlowregion *>(o)) {
            region = o;
            break;
        }
    }

    if (region) {
        if (flags || (region->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            region->emitModified(flags); // pass down to the region only
        }
    }
}

void SPFlowtext::build(SPDocument* doc, Inkscape::XML::Node* repr) {
    this->_requireSVGVersion(Inkscape::Version(1, 2));

    SPItem::build(doc, repr);

    this->readAttr( "inkscape:layoutOptions" );     // must happen after css has been read
}

void SPFlowtext::set(unsigned int key, const gchar* value) {
    switch (key) {
        case SP_ATTR_LAYOUT_OPTIONS: {
            // deprecated attribute, read for backward compatibility only
            //XML Tree being directly used while it shouldn't be.
            SPCSSAttr *opts = sp_repr_css_attr(this->getRepr(), "inkscape:layoutOptions");
            {
                gchar const *val = sp_repr_css_property(opts, "justification", NULL);

                if (val != NULL && !this->style->text_align.set) {
                    if ( strcmp(val, "0") == 0 || strcmp(val, "false") == 0 ) {
                        this->style->text_align.value = SP_CSS_TEXT_ALIGN_LEFT;
                    } else {
                        this->style->text_align.value = SP_CSS_TEXT_ALIGN_JUSTIFY;
                    }

                    this->style->text_align.set = TRUE;
                    this->style->text_align.inherit = FALSE;
                    this->style->text_align.computed = this->style->text_align.value;
                }
            }
            /* no equivalent css attribute for these two (yet)
            {
                gchar const *val = sp_repr_css_property(opts, "layoutAlgo", NULL);
                if ( val == NULL ) {
                    group->algo = 0;
                } else {
                    if ( strcmp(val, "better") == 0 ) {     // knuth-plass, never worked for general cases
                        group->algo = 2;
                    } else if ( strcmp(val, "simple") == 0 ) {   // greedy, but allowed lines to be compressed by up to 20% if it would make them fit
                        group->algo = 1;
                    } else if ( strcmp(val, "default") == 0 ) {    // the same one we use, a standard greedy
                        group->algo = 0;
                    }
                }
            }
            */
            {   // This would probably translate to padding-left, if SPStyle had it.
                gchar const *val = sp_repr_css_property(opts, "par-indent", NULL);

                if ( val == NULL ) {
                    this->par_indent = 0.0;
                } else {
                    this->par_indent = g_ascii_strtod(val, NULL);
                }
            }

            sp_repr_css_attr_unref(opts);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }

        default:
        	SPItem::set(key, value);
            break;
    }
}

Inkscape::XML::Node* SPFlowtext::write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, guint flags) {
    if ( flags & SP_OBJECT_WRITE_BUILD ) {
        if ( repr == NULL ) {
            repr = doc->createElement("svg:flowRoot");
        }

        GSList *l = NULL;

        for (SPObject *child = this->firstChild() ; child ; child = child->getNext() ) {
            Inkscape::XML::Node *c_repr = NULL;

            if ( dynamic_cast<SPFlowdiv *>(child) || dynamic_cast<SPFlowpara *>(child) || dynamic_cast<SPFlowregion *>(child) || dynamic_cast<SPFlowregionExclude *>(child)) {
                c_repr = child->updateRepr(doc, NULL, flags);
            }

            if ( c_repr ) {
                l = g_slist_prepend(l, c_repr);
            }
        }

        while ( l ) {
            repr->addChild((Inkscape::XML::Node *) l->data, NULL);
            Inkscape::GC::release((Inkscape::XML::Node *) l->data);
            l = g_slist_remove(l, l->data);
        }
    } else {
        for (SPObject *child = this->firstChild() ; child ; child = child->getNext() ) {
            if ( dynamic_cast<SPFlowdiv *>(child) || dynamic_cast<SPFlowpara *>(child) || dynamic_cast<SPFlowregion *>(child) || dynamic_cast<SPFlowregionExclude *>(child)) {
                child->updateRepr(flags);
            }
        }
    }

    this->rebuildLayout();  // copied from update(), see LP Bug 1339305

    SPItem::write(doc, repr, flags);

    return repr;
}

Geom::OptRect SPFlowtext::bbox(Geom::Affine const &transform, SPItem::BBoxType type) const {
    Geom::OptRect bbox = this->layout.bounds(transform);

    // Add stroke width
    // FIXME this code is incorrect
    if (bbox && type == SPItem::VISUAL_BBOX && !this->style->stroke.isNone()) {
        double scale = transform.descrim();
        bbox->expandBy(0.5 * this->style->stroke_width.computed * scale);
    }

    return bbox;
}

void SPFlowtext::print(SPPrintContext *ctx) {
    Geom::OptRect pbox, bbox, dbox;
    pbox = this->geometricBounds();
    bbox = this->desktopVisualBounds();
    dbox = Geom::Rect::from_xywh(Geom::Point(0,0), this->document->getDimensions());

    Geom::Affine const ctm (this->i2dt_affine());

    this->layout.print(ctx, pbox, dbox, bbox, ctm);
}

const char* SPFlowtext::displayName() const {
    if (has_internal_frame()) {
        return _("Flowed Text");
    } else {
        return _("Linked Flowed Text");
    }
}

gchar* SPFlowtext::description() const {
    int const nChars = layout.iteratorToCharIndex(layout.end());
    char const *trunc = (layout.inputTruncated()) ? _(" [truncated]") : "";

    return g_strdup_printf(ngettext("(%d character%s)", "(%d characters%s)", nChars), nChars, trunc);
}

void SPFlowtext::snappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs) const {
    if (snapprefs->isTargetSnappable(Inkscape::SNAPTARGET_TEXT_BASELINE)) {
        // Choose a point on the baseline for snapping from or to, with the horizontal position
        // of this point depending on the text alignment (left vs. right)
        Inkscape::Text::Layout const *layout = te_get_layout((SPItem *) this);

        if (layout != NULL && layout->outputExists()) {
            boost::optional<Geom::Point> pt = layout->baselineAnchorPoint();

            if (pt) {
                p.push_back(Inkscape::SnapCandidatePoint((*pt) * this->i2dt_affine(), Inkscape::SNAPSOURCE_TEXT_ANCHOR, Inkscape::SNAPTARGET_TEXT_ANCHOR));
            }
        }
    }
}

Inkscape::DrawingItem* SPFlowtext::show(Inkscape::Drawing &drawing, unsigned int /*key*/, unsigned int /*flags*/) {
    Inkscape::DrawingGroup *flowed = new Inkscape::DrawingGroup(drawing);
    flowed->setPickChildren(false);
    flowed->setStyle(this->style);

    // pass the bbox of the flowtext object as paintbox (used for paintserver fills)
    Geom::OptRect bbox = this->geometricBounds();
    this->layout.show(flowed, bbox);

    return flowed;
}

void SPFlowtext::hide(unsigned int key) {
    for (SPItemView* v = this->display; v != NULL; v = v->next) {
        if (v->key == key) {
            Inkscape::DrawingGroup *g = dynamic_cast<Inkscape::DrawingGroup *>(v->arenaitem);
            this->_clearFlow(g);
        }
    }
}


/*
 *
 */
void SPFlowtext::_buildLayoutInput(SPObject *root, Shape const *exclusion_shape, std::list<Shape> *shapes, SPObject **pending_line_break_object)
{
    Inkscape::Text::Layout::OptionalTextTagAttrs pi;
    bool with_indent = false;

    if (dynamic_cast<SPFlowpara *>(root)) {

        layout.strut.reset();
        if (style) {
            font_instance *font = font_factory::Default()->FaceFromStyle( style );
            if (font) {
                font->FontMetrics(layout.strut.ascent, layout.strut.descent, layout.strut.xheight);
                font->Unref();
            }
            layout.strut *= style->font_size.computed;
            if (style->line_height.normal ) {
                layout.strut.computeEffective( Inkscape::Text::Layout::LINE_HEIGHT_NORMAL ); 
            } else if (style->line_height.unit == SP_CSS_UNIT_NONE) {
                layout.strut.computeEffective( style->line_height.computed );
            } else {
                if( style->font_size.computed > 0.0 ) {
                    layout.strut.computeEffective( style->line_height.computed/style->font_size.computed );
                }
            }
        }

        // emulate par-indent with the first char's kern
        SPObject *t = root;
        SPFlowtext *ft = NULL;
        while (t && !ft) {
            ft = dynamic_cast<SPFlowtext *>(t);
            t = t->parent;
        }

        if (ft) {
            double indent = ft->par_indent;
            if (indent != 0) {
                with_indent = true;
                SVGLength sl;
                sl.value = sl.computed = indent;
                sl._set = true;
                pi.dx.push_back(sl);
            }
        }
    }

    if (*pending_line_break_object) {
        if (dynamic_cast<SPFlowregionbreak *>(*pending_line_break_object)) {
            layout.appendControlCode(Inkscape::Text::Layout::SHAPE_BREAK, *pending_line_break_object);
        } else {
            layout.appendControlCode(Inkscape::Text::Layout::PARAGRAPH_BREAK, *pending_line_break_object);
        }
        *pending_line_break_object = NULL;
    }

    for (SPObject *child = root->firstChild() ; child ; child = child->getNext() ) {
        SPString *str = dynamic_cast<SPString *>(child);
        if (str) {
            if (*pending_line_break_object) {
                if (dynamic_cast<SPFlowregionbreak *>(*pending_line_break_object))
                    layout.appendControlCode(Inkscape::Text::Layout::SHAPE_BREAK, *pending_line_break_object);
                else {
                    layout.appendControlCode(Inkscape::Text::Layout::PARAGRAPH_BREAK, *pending_line_break_object);
                }
                *pending_line_break_object = NULL;
            }
            if (with_indent) {
                layout.appendText(str->string, root->style, child, &pi);
            } else {
                layout.appendText(str->string, root->style, child);
            }
        } else {
            SPFlowregion *region = dynamic_cast<SPFlowregion *>(child);
            if (region) {
                std::vector<Shape*> const &computed = region->computed;
                for (std::vector<Shape*>::const_iterator it = computed.begin() ; it != computed.end() ; ++it) {
                    shapes->push_back(Shape());
                    if (exclusion_shape->hasEdges()) {
                        shapes->back().Booleen(*it, const_cast<Shape*>(exclusion_shape), bool_op_diff);
                    } else {
                        shapes->back().Copy(*it);
                    }
                    layout.appendWrapShape(&shapes->back());
                }
            }
            //Xml Tree is being directly used while it shouldn't be.
            else if (!dynamic_cast<SPFlowregionExclude *>(child) && !sp_repr_is_meta_element(child->getRepr())) {
                _buildLayoutInput(child, exclusion_shape, shapes, pending_line_break_object);
            }
        }
    }

    if (dynamic_cast<SPFlowdiv *>(root) || dynamic_cast<SPFlowpara *>(root) || dynamic_cast<SPFlowregionbreak *>(root) || dynamic_cast<SPFlowline *>(root)) {
        if (!root->hasChildren()) {
            layout.appendText("", root->style, root);
        }
        *pending_line_break_object = root;
    }
}

Shape* SPFlowtext::_buildExclusionShape() const
{
    Shape *shape = new Shape();
    Shape *shape_temp = new Shape();

    for (SPObject *child = children ; child ; child = child->getNext() ) {
        // RH: is it right that this shouldn't be recursive?
        SPFlowregionExclude *c_child = dynamic_cast<SPFlowregionExclude *>(child);
        if ( c_child && c_child->computed && c_child->computed->hasEdges() ) {
            if (shape->hasEdges()) {
                shape_temp->Booleen(shape, c_child->computed, bool_op_union);
                std::swap(shape, shape_temp);
            } else {
                shape->Copy(c_child->computed);
            }
        }
    }

    delete shape_temp;

    return shape;
}

void SPFlowtext::rebuildLayout()
{
    std::list<Shape> shapes;

    layout.clear();
    Shape *exclusion_shape = _buildExclusionShape();
    SPObject *pending_line_break_object = NULL;
    _buildLayoutInput(this, exclusion_shape, &shapes, &pending_line_break_object);
    delete exclusion_shape;
    layout.calculateFlow();
    //g_print("%s", layout.dumpAsText().c_str());
}

void SPFlowtext::_clearFlow(Inkscape::DrawingGroup *in_arena)
{
    in_arena->clearChildren();
}

Inkscape::XML::Node *SPFlowtext::getAsText()
{
    if (!this->layout.outputExists()) {
        return NULL;
    }

    Inkscape::XML::Document *xml_doc = this->document->getReprDoc();
    Inkscape::XML::Node *repr = xml_doc->createElement("svg:text");
    repr->setAttribute("xml:space", "preserve");
    repr->setAttribute("style", this->getRepr()->attribute("style"));
    Geom::Point anchor_point = this->layout.characterAnchorPoint(this->layout.begin());
    sp_repr_set_svg_double(repr, "x", anchor_point[Geom::X]);
    sp_repr_set_svg_double(repr, "y", anchor_point[Geom::Y]);

    for (Inkscape::Text::Layout::iterator it = this->layout.begin() ; it != this->layout.end() ; ) {
        Inkscape::XML::Node *line_tspan = xml_doc->createElement("svg:tspan");
        line_tspan->setAttribute("sodipodi:role", "line");

        Inkscape::Text::Layout::iterator it_line_end = it;
        it_line_end.nextStartOfLine();

        while (it != it_line_end) {

            Inkscape::XML::Node *span_tspan = xml_doc->createElement("svg:tspan");
            Geom::Point anchor_point = this->layout.characterAnchorPoint(it);
            // use kerning to simulate justification and whatnot
            Inkscape::Text::Layout::iterator it_span_end = it;
            it_span_end.nextStartOfSpan();
            Inkscape::Text::Layout::OptionalTextTagAttrs attrs;
            this->layout.simulateLayoutUsingKerning(it, it_span_end, &attrs);
            // set x,y attributes only when we need to
            bool set_x = false;
            bool set_y = false;
            if (!this->transform.isIdentity()) {
                set_x = set_y = true;
            } else {
                Inkscape::Text::Layout::iterator it_chunk_start = it;
                it_chunk_start.thisStartOfChunk();
                if (it == it_chunk_start) {
                    set_x = true;
                    // don't set y so linespacing adjustments and things will still work
                }
                Inkscape::Text::Layout::iterator it_shape_start = it;
                it_shape_start.thisStartOfShape();
                if (it == it_shape_start)
                    set_y = true;
            }
            if (set_x && !attrs.dx.empty())
                attrs.dx[0] = 0.0;
            TextTagAttributes(attrs).writeTo(span_tspan);
            if (set_x)
                sp_repr_set_svg_double(span_tspan, "x", anchor_point[Geom::X]);  // FIXME: this will pick up the wrong end of counter-directional runs
            if (set_y)
                sp_repr_set_svg_double(span_tspan, "y", anchor_point[Geom::Y]);
            if (line_tspan->childCount() == 0) {
                sp_repr_set_svg_double(line_tspan, "x", anchor_point[Geom::X]);  // FIXME: this will pick up the wrong end of counter-directional runs
                sp_repr_set_svg_double(line_tspan, "y", anchor_point[Geom::Y]);
            }

            void *rawptr = 0;
            Glib::ustring::iterator span_text_start_iter;
            this->layout.getSourceOfCharacter(it, &rawptr, &span_text_start_iter);
            SPObject *source_obj = reinterpret_cast<SPObject *>(rawptr);

            Glib::ustring style_text = (dynamic_cast<SPString *>(source_obj) ? source_obj->parent : source_obj)->style->write( SP_STYLE_FLAG_IFDIFF, this->style);
            if (!style_text.empty()) {
                span_tspan->setAttribute("style", style_text.c_str());
            }

            SPString *str = dynamic_cast<SPString *>(source_obj);
            if (str) {
                Glib::ustring *string = &(str->string); // TODO fixme: dangerous, unsafe premature-optimization
                void *rawptr = 0;
                Glib::ustring::iterator span_text_end_iter;
                this->layout.getSourceOfCharacter(it_span_end, &rawptr, &span_text_end_iter);
                SPObject *span_end_obj = reinterpret_cast<SPObject *>(rawptr);
                if (span_end_obj != source_obj) {
                    if (it_span_end == this->layout.end()) {
                        span_text_end_iter = span_text_start_iter;
                        for (int i = this->layout.iteratorToCharIndex(it_span_end) - this->layout.iteratorToCharIndex(it) ; i ; --i)
                            ++span_text_end_iter;
                    } else
                        span_text_end_iter = string->end();    // spans will never straddle a source boundary
                }

                if (span_text_start_iter != span_text_end_iter) {
                    Glib::ustring new_string;
                    while (span_text_start_iter != span_text_end_iter)
                        new_string += *span_text_start_iter++;    // grr. no substr() with iterators
                    Inkscape::XML::Node *new_text = xml_doc->createTextNode(new_string.c_str());
                    span_tspan->appendChild(new_text);
                    Inkscape::GC::release(new_text);
                }
            }
            it = it_span_end;

            line_tspan->appendChild(span_tspan);
            Inkscape::GC::release(span_tspan);
        }
        repr->appendChild(line_tspan);
        Inkscape::GC::release(line_tspan);
    }

    return repr;
}

SPItem const *SPFlowtext::get_frame(SPItem const *after) const
{
    SPItem *item = const_cast<SPFlowtext *>(this)->get_frame(after);
    return item;
}

SPItem *SPFlowtext::get_frame(SPItem const *after)
{
    SPItem *frame = 0;

    SPObject *region = 0;
    for (SPObject *o = firstChild() ; o ; o = o->getNext() ) {
        if (dynamic_cast<SPFlowregion *>(o)) {
            region = o;
            break;
        }
    }

    if (region) {
        bool past = false;

        for (SPObject *o = region->firstChild() ; o ; o = o->getNext() ) {
            SPItem *item = dynamic_cast<SPItem *>(o);
            if (item) {
                if ( (after == NULL) || past ) {
                    frame = item;
                } else {
                    if (item == after) {
                        past = true;
                    }
                }
            }
        }

        SPUse *use = dynamic_cast<SPUse *>(frame);
        if ( use ) {
            frame = use->get_original();
        }
    }
    return frame;
}

bool SPFlowtext::has_internal_frame() const
{
    SPItem const *frame = get_frame(NULL);

    return (frame && isAncestorOf(frame) && dynamic_cast<SPRect const *>(frame));
}


SPItem *create_flowtext_with_internal_frame (SPDesktop *desktop, Geom::Point p0, Geom::Point p1)
{
    SPDocument *doc = desktop->getDocument();

    Inkscape::XML::Document *xml_doc = doc->getReprDoc();
    Inkscape::XML::Node *root_repr = xml_doc->createElement("svg:flowRoot");
    root_repr->setAttribute("xml:space", "preserve"); // we preserve spaces in the text objects we create
    SPItem *ft_item = dynamic_cast<SPItem *>(desktop->currentLayer()->appendChildRepr(root_repr));
    g_assert(ft_item != NULL);
    SPObject *root_object = doc->getObjectByRepr(root_repr);
    g_assert(dynamic_cast<SPFlowtext *>(root_object) != NULL);

    Inkscape::XML::Node *region_repr = xml_doc->createElement("svg:flowRegion");
    root_repr->appendChild(region_repr);
    SPObject *region_object = doc->getObjectByRepr(region_repr);
    g_assert(dynamic_cast<SPFlowregion *>(region_object) != NULL);

    Inkscape::XML::Node *rect_repr = xml_doc->createElement("svg:rect"); // FIXME: use path!!! after rects are converted to use path
    region_repr->appendChild(rect_repr);

    SPRect *rect = dynamic_cast<SPRect *>(doc->getObjectByRepr(rect_repr));
    g_assert(rect != NULL);

    p0 *= desktop->dt2doc();
    p1 *= desktop->dt2doc();
    using Geom::X;
    using Geom::Y;
    Geom::Coord const x0 = MIN(p0[X], p1[X]);
    Geom::Coord const y0 = MIN(p0[Y], p1[Y]);
    Geom::Coord const x1 = MAX(p0[X], p1[X]);
    Geom::Coord const y1 = MAX(p0[Y], p1[Y]);
    Geom::Coord const w  = x1 - x0;
    Geom::Coord const h  = y1 - y0;

    rect->setPosition(x0, y0, w, h);
    rect->updateRepr();

    Inkscape::XML::Node *para_repr = xml_doc->createElement("svg:flowPara");
    root_repr->appendChild(para_repr);
    SPObject *para_object = doc->getObjectByRepr(para_repr);
    g_assert(dynamic_cast<SPFlowpara *>(para_object) != NULL);

    Inkscape::XML::Node *text = xml_doc->createTextNode("");
    para_repr->appendChild(text);

    Inkscape::GC::release(root_repr);
    Inkscape::GC::release(region_repr);
    Inkscape::GC::release(para_repr);
    Inkscape::GC::release(rect_repr);

  
    SPItem *item = dynamic_cast<SPItem *>(desktop->currentLayer());
    g_assert(item != NULL);
    ft_item->transform = item->i2doc_affine().inverse();

    return ft_item;
}

Geom::Affine SPFlowtext::set_transform (Geom::Affine const &xform)
{
    if ((this->_optimizeScaledText && !xform.withoutTranslation().isNonzeroUniformScale())
        || (!this->_optimizeScaledText && !xform.isNonzeroUniformScale())) {
        this->_optimizeScaledText = false;
        return xform;
    }
    this->_optimizeScaledText = false;
    
    SPText *text = reinterpret_cast<SPText *>(this);
    
    double const ex = xform.descrim();
    if (ex == 0) {
        return xform;
    }

    SPObject *region = NULL;
    for ( SPObject *o = this->firstChild() ; o ; o = o->getNext() ) {
        if (dynamic_cast<SPFlowregion *>(o)) {
            region = o;
            break;
        }
    }
    if (region) {
        SPRect *rect = dynamic_cast<SPRect *>(region->firstChild());
        if (rect) {
            rect->set_i2d_affine(xform * rect->i2dt_affine());
            rect->doWriteTransform(rect->getRepr(), rect->transform, NULL, true);
        }
    }

    Geom::Affine ret(xform);
    ret[0] /= ex;
    ret[1] /= ex;
    ret[2] /= ex;
    ret[3] /= ex;

    // Adjust font size
    text->_adjustFontsizeRecursive (this, ex);

    // Adjust stroke width
    this->adjust_stroke_width_recursive (ex);

    // Adjust pattern fill
    this->adjust_pattern(xform * ret.inverse());

    // Adjust gradient fill
    this->adjust_gradient(xform * ret.inverse());

    this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_TEXT_LAYOUT_MODIFIED_FLAG);

    return Geom::Affine();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
