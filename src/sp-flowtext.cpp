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
#include "desktop-handles.h"
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


#include "livarot/Shape.h"

#include "display/nr-arena-glyphs.h"


static void sp_flowtext_class_init(SPFlowtextClass *klass);
static void sp_flowtext_init(SPFlowtext *group);
static void sp_flowtext_dispose(GObject *object);

static void sp_flowtext_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref);
static void sp_flowtext_remove_child(SPObject *object, Inkscape::XML::Node *child);
static void sp_flowtext_update(SPObject *object, SPCtx *ctx, guint flags);
static void sp_flowtext_modified(SPObject *object, guint flags);
static Inkscape::XML::Node *sp_flowtext_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_flowtext_build(SPObject *object, Document *document, Inkscape::XML::Node *repr);
static void sp_flowtext_set(SPObject *object, unsigned key, gchar const *value);

static void sp_flowtext_bbox(SPItem const *item, NRRect *bbox, Geom::Matrix const &transform, unsigned const flags);
static void sp_flowtext_print(SPItem *item, SPPrintContext *ctx);
static gchar *sp_flowtext_description(SPItem *item);
static NRArenaItem *sp_flowtext_show(SPItem *item, NRArena *arena, unsigned key, unsigned flags);
static void sp_flowtext_hide(SPItem *item, unsigned key);

static SPItemClass *parent_class;

GType
sp_flowtext_get_type(void)
{
    static GType group_type = 0;
    if (!group_type) {
        GTypeInfo group_info = {
            sizeof(SPFlowtextClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc) sp_flowtext_class_init,
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof(SPFlowtext),
            16,     /* n_preallocs */
            (GInstanceInitFunc) sp_flowtext_init,
            NULL,   /* value_table */
        };
        group_type = g_type_register_static(SP_TYPE_ITEM, "SPFlowtext", &group_info, (GTypeFlags)0);
    }
    return group_type;
}

static void
sp_flowtext_class_init(SPFlowtextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;
    SPItemClass *item_class = (SPItemClass *) klass;

    parent_class = (SPItemClass *)g_type_class_ref(SP_TYPE_ITEM);

    object_class->dispose = sp_flowtext_dispose;

    sp_object_class->child_added = sp_flowtext_child_added;
    sp_object_class->remove_child = sp_flowtext_remove_child;
    sp_object_class->update = sp_flowtext_update;
    sp_object_class->modified = sp_flowtext_modified;
    sp_object_class->write = sp_flowtext_write;
    sp_object_class->build = sp_flowtext_build;
    sp_object_class->set = sp_flowtext_set;

    item_class->bbox = sp_flowtext_bbox;
    item_class->print = sp_flowtext_print;
    item_class->description = sp_flowtext_description;
    item_class->show = sp_flowtext_show;
    item_class->hide = sp_flowtext_hide;
}

static void
sp_flowtext_init(SPFlowtext *group)
{
    group->par_indent = 0;
    new (&group->layout) Inkscape::Text::Layout();
}

static void
sp_flowtext_dispose(GObject *object)
{
    SPFlowtext *group = (SPFlowtext*)object;

    group->layout.~Layout();
}

static void
sp_flowtext_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    if (((SPObjectClass *) (parent_class))->child_added)
        (* ((SPObjectClass *) (parent_class))->child_added)(object, child, ref);

    object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/* fixme: hide (Lauris) */

static void
sp_flowtext_remove_child(SPObject *object, Inkscape::XML::Node *child)
{
    if (((SPObjectClass *) (parent_class))->remove_child)
        (* ((SPObjectClass *) (parent_class))->remove_child)(object, child);

    object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_flowtext_update(SPObject *object, SPCtx *ctx, unsigned flags)
{
    SPFlowtext *group = SP_FLOWTEXT(object);
    SPItemCtx *ictx = (SPItemCtx *) ctx;
    SPItemCtx cctx = *ictx;

    if (((SPObjectClass *) (parent_class))->update)
        ((SPObjectClass *) (parent_class))->update(object, ctx, flags);

    if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    GSList *l = NULL;
    for (SPObject *child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        g_object_ref(G_OBJECT(child));
        l = g_slist_prepend(l, child);
    }
    l = g_slist_reverse(l);
    while (l) {
        SPObject *child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);
        if (flags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            if (SP_IS_ITEM(child)) {
                SPItem const &chi = *SP_ITEM(child);
                cctx.i2doc = chi.transform * ictx->i2doc;
                cctx.i2vp = chi.transform * ictx->i2vp;
                child->updateDisplay((SPCtx *)&cctx, flags);
            } else {
                child->updateDisplay(ctx, flags);
            }
        }
        g_object_unref(G_OBJECT(child));
    }

    group->rebuildLayout();

    NRRect paintbox;
    sp_item_invoke_bbox(group, &paintbox, Geom::identity(), TRUE);
    for (SPItemView *v = group->display; v != NULL; v = v->next) {
        group->_clearFlow(NR_ARENA_GROUP(v->arenaitem));
        nr_arena_group_set_style(NR_ARENA_GROUP(v->arenaitem), SP_OBJECT_STYLE(object));
        // pass the bbox of the flowtext object as paintbox (used for paintserver fills)
        group->layout.show(NR_ARENA_GROUP(v->arenaitem), &paintbox);
    }
}

static void
sp_flowtext_modified(SPObject *object, guint flags)
{
    SPObject *ft = SP_FLOWTEXT (object);
    SPObject *region = NULL;

    if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    // FIXME: the below stanza is copied over from sp_text_modified, consider factoring it out
    if (flags & ( SP_OBJECT_STYLE_MODIFIED_FLAG )) {
        SPFlowtext *text = SP_FLOWTEXT(object);
        NRRect paintbox;
        sp_item_invoke_bbox(text, &paintbox, Geom::identity(), TRUE);
        for (SPItemView* v = text->display; v != NULL; v = v->next) {
            text->_clearFlow(NR_ARENA_GROUP(v->arenaitem));
            nr_arena_group_set_style(NR_ARENA_GROUP(v->arenaitem), SP_OBJECT_STYLE(object));
            text->layout.show(NR_ARENA_GROUP(v->arenaitem), &paintbox);
        }
    }

    for (SPObject *o = sp_object_first_child(SP_OBJECT(ft)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
        if (SP_IS_FLOWREGION(o)) {
            region = o;
            break;
        }
    }

    if (!region) return;

    if (flags || (region->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
        region->emitModified(flags); // pass down to the region only
    }
}

static void
sp_flowtext_build(SPObject *object, Document *document, Inkscape::XML::Node *repr)
{
    object->_requireSVGVersion(Inkscape::Version(1, 2));

    if (((SPObjectClass *) (parent_class))->build) {
        (* ((SPObjectClass *) (parent_class))->build)(object, document, repr);
    }

    sp_object_read_attr(object, "inkscape:layoutOptions");     // must happen after css has been read
}

static void
sp_flowtext_set(SPObject *object, unsigned key, gchar const *value)
{
    SPFlowtext *group = (SPFlowtext *) object;

    switch (key) {
        case SP_ATTR_LAYOUT_OPTIONS: {
            // deprecated attribute, read for backward compatibility only
            SPCSSAttr *opts = sp_repr_css_attr((SP_OBJECT(group))->repr, "inkscape:layoutOptions");
            {
                gchar const *val = sp_repr_css_property(opts, "justification", NULL);
                if (val != NULL && !object->style->text_align.set) {
                    if ( strcmp(val, "0") == 0 || strcmp(val, "false") == 0 ) {
                        object->style->text_align.value = SP_CSS_TEXT_ALIGN_LEFT;
                    } else {
                        object->style->text_align.value = SP_CSS_TEXT_ALIGN_JUSTIFY;
                    }
                    object->style->text_align.set = TRUE;
                    object->style->text_align.inherit = FALSE;
                    object->style->text_align.computed = object->style->text_align.value;
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
                    group->par_indent = 0.0;
                } else {
                    sp_repr_get_double((Inkscape::XML::Node*)opts, "par-indent", &group->par_indent);
                }
            }
            sp_repr_css_attr_unref(opts);
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }
        default:
            if (((SPObjectClass *) (parent_class))->set) {
                (* ((SPObjectClass *) (parent_class))->set)(object, key, value);
            }
            break;
    }
}

static Inkscape::XML::Node *
sp_flowtext_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    if ( flags & SP_OBJECT_WRITE_BUILD ) {
        if ( repr == NULL ) repr = xml_doc->createElement("svg:flowRoot");
        GSList *l = NULL;
        for (SPObject *child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
            Inkscape::XML::Node *c_repr = NULL;
            if ( SP_IS_FLOWDIV(child) || SP_IS_FLOWPARA(child) || SP_IS_FLOWREGION(child) || SP_IS_FLOWREGIONEXCLUDE(child)) {
                c_repr = child->updateRepr(xml_doc, NULL, flags);
            }
            if ( c_repr ) l = g_slist_prepend(l, c_repr);
        }
        while ( l ) {
            repr->addChild((Inkscape::XML::Node *) l->data, NULL);
            Inkscape::GC::release((Inkscape::XML::Node *) l->data);
            l = g_slist_remove(l, l->data);
        }
    } else {
        for (SPObject *child = sp_object_first_child(object) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
            if ( SP_IS_FLOWDIV(child) || SP_IS_FLOWPARA(child) || SP_IS_FLOWREGION(child) || SP_IS_FLOWREGIONEXCLUDE(child) ) {
                child->updateRepr(flags);
            }
        }
    }

    if (((SPObjectClass *) (parent_class))->write)
        ((SPObjectClass *) (parent_class))->write(object, xml_doc, repr, flags);

    return repr;
}

static void
sp_flowtext_bbox(SPItem const *item, NRRect *bbox, Geom::Matrix const &transform, unsigned const /*flags*/)
{
    SPFlowtext *group = SP_FLOWTEXT(item);
    group->layout.getBoundingBox(bbox, transform);

    // Add stroke width
    SPStyle* style=SP_OBJECT_STYLE (item);
    if ( !style->stroke.isNone() ) {
        double const scale = transform.descrim();
        if ( fabs(style->stroke_width.computed * scale) > 0.01 ) { // sinon c'est 0=oon veut pas de bord
            double const width = MAX(0.125, style->stroke_width.computed * scale);
            if ( fabs(bbox->x1 - bbox->x0) > -0.00001 && fabs(bbox->y1 - bbox->y0) > -0.00001 ) {
                bbox->x0-=0.5*width;
                bbox->x1+=0.5*width;
                bbox->y0-=0.5*width;
                bbox->y1+=0.5*width;
            }
        }
    }
}

static void
sp_flowtext_print(SPItem *item, SPPrintContext *ctx)
{
    SPFlowtext *group = SP_FLOWTEXT(item);

    NRRect pbox;
    sp_item_invoke_bbox(item, &pbox, Geom::identity(), TRUE);
    NRRect bbox;
    Geom::OptRect bbox_maybe = sp_item_bbox_desktop(item);
    if (!bbox_maybe) {
        return;
    }
    bbox = NRRect(from_2geom(*bbox_maybe));

    NRRect dbox;
    dbox.x0 = 0.0;
    dbox.y0 = 0.0;
    dbox.x1 = sp_document_width(SP_OBJECT_DOCUMENT(item));
    dbox.y1 = sp_document_height(SP_OBJECT_DOCUMENT(item));
    Geom::Matrix const ctm (sp_item_i2d_affine(item));

    group->layout.print(ctx, &pbox, &dbox, &bbox, ctm);
}


static gchar *sp_flowtext_description(SPItem *item)
{
    Inkscape::Text::Layout const &layout = SP_FLOWTEXT(item)->layout;
    int const nChars = layout.iteratorToCharIndex(layout.end());
    if (SP_FLOWTEXT(item)->has_internal_frame())
        return g_strdup_printf(ngettext("<b>Flowed text</b> (%d character)", "<b>Flowed text</b> (%d characters)", nChars), nChars);
    else
        return g_strdup_printf(ngettext("<b>Linked flowed text</b> (%d character)", "<b>Linked flowed text</b> (%d characters)", nChars), nChars);
}

static NRArenaItem *
sp_flowtext_show(SPItem *item, NRArena *arena, unsigned/* key*/, unsigned /*flags*/)
{
    SPFlowtext *group = (SPFlowtext *) item;
    NRArenaGroup *flowed = NRArenaGroup::create(arena);
    nr_arena_group_set_transparent(flowed, FALSE);

    nr_arena_group_set_style(flowed, group->style);

    // pass the bbox of the flowtext object as paintbox (used for paintserver fills)
    NRRect paintbox;
    sp_item_invoke_bbox(item, &paintbox, Geom::identity(), TRUE);
    group->layout.show(flowed, &paintbox);

    return flowed;
}

static void
sp_flowtext_hide(SPItem *item, unsigned int key)
{
    if (((SPItemClass *) parent_class)->hide)
        ((SPItemClass *) parent_class)->hide(item, key);
}


/*
 *
 */

void SPFlowtext::_buildLayoutInput(SPObject *root, Shape const *exclusion_shape, std::list<Shape> *shapes, SPObject **pending_line_break_object)
{
    Inkscape::Text::Layout::OptionalTextTagAttrs pi;
    bool with_indent = false;

    if (SP_IS_FLOWPARA(root)) {
        // emulate par-indent with the first char's kern
        SPObject *t = root;
        for ( ; t != NULL && !SP_IS_FLOWTEXT(t); t = SP_OBJECT_PARENT(t)){};
        if (SP_IS_FLOWTEXT(t)) {
            double indent = SP_FLOWTEXT(t)->par_indent;
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
        if (SP_IS_FLOWREGIONBREAK(*pending_line_break_object)) {
            layout.appendControlCode(Inkscape::Text::Layout::SHAPE_BREAK, *pending_line_break_object);
        } else {
            layout.appendControlCode(Inkscape::Text::Layout::PARAGRAPH_BREAK, *pending_line_break_object);
        }
        *pending_line_break_object = NULL;
    }

    for (SPObject *child = sp_object_first_child(root) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        if (SP_IS_STRING(child)) {
            if (*pending_line_break_object) {
                if (SP_IS_FLOWREGIONBREAK(*pending_line_break_object))
                    layout.appendControlCode(Inkscape::Text::Layout::SHAPE_BREAK, *pending_line_break_object);
                else {
                    layout.appendControlCode(Inkscape::Text::Layout::PARAGRAPH_BREAK, *pending_line_break_object);
                }
                *pending_line_break_object = NULL;
            }
            if (with_indent)
                layout.appendText(SP_STRING(child)->string, root->style, child, &pi);
            else
                layout.appendText(SP_STRING(child)->string, root->style, child);
        } else if (SP_IS_FLOWREGION(child)) {
            std::vector<Shape*> const &computed = SP_FLOWREGION(child)->computed;
            for (std::vector<Shape*>::const_iterator it = computed.begin() ; it != computed.end() ; it++) {
                shapes->push_back(Shape());
                if (exclusion_shape->hasEdges())
                    shapes->back().Booleen(*it, const_cast<Shape*>(exclusion_shape), bool_op_diff);
                else
                    shapes->back().Copy(*it);
                layout.appendWrapShape(&shapes->back());
            }
        }
        else if (!SP_IS_FLOWREGIONEXCLUDE(child) && !sp_repr_is_meta_element(child->repr))
            _buildLayoutInput(child, exclusion_shape, shapes, pending_line_break_object);
    }

    if (SP_IS_FLOWDIV(root) || SP_IS_FLOWPARA(root) || SP_IS_FLOWREGIONBREAK(root) || SP_IS_FLOWLINE(root)) {
        if (!root->hasChildren())
            layout.appendText("", root->style, root);
        *pending_line_break_object = root;
    }
}

Shape* SPFlowtext::_buildExclusionShape() const
{
    Shape *shape = new Shape;
    Shape *shape_temp = new Shape;

    for (SPObject *child = children ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
        // RH: is it right that this shouldn't be recursive?
        if ( SP_IS_FLOWREGIONEXCLUDE(child) ) {
            SPFlowregionExclude *c_child = SP_FLOWREGIONEXCLUDE(child);
            if (c_child->computed == NULL || !c_child->computed->hasEdges())
                continue;
            if (shape->hasEdges()) {
                shape_temp->Booleen(shape, c_child->computed, bool_op_union);
                std::swap(shape, shape_temp);
            } else
                shape->Copy(c_child->computed);
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
    //g_print(layout.dumpAsText().c_str());
}

void SPFlowtext::_clearFlow(NRArenaGroup *in_arena)
{
    nr_arena_item_request_render(NR_ARENA_ITEM(in_arena));
    for (NRArenaItem *child = in_arena->children; child != NULL; ) {
        NRArenaItem *nchild = child->next;

        nr_arena_glyphs_group_clear(NR_ARENA_GLYPHS_GROUP(child));
        nr_arena_item_remove_child(NR_ARENA_ITEM(in_arena), child);

        child = nchild;
    }
}

Inkscape::XML::Node *
SPFlowtext::getAsText()
{
    if (!this->layout.outputExists()) return NULL;

    SPItem *item = SP_ITEM(this);

    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(SP_OBJECT_DOCUMENT(this));
    Inkscape::XML::Node *repr = xml_doc->createElement("svg:text");
    repr->setAttribute("xml:space", "preserve");
    repr->setAttribute("style", SP_OBJECT_REPR(this)->attribute("style"));
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
            if (!item->transform.isIdentity()) {
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

            SPObject *source_obj = 0;
            void *rawptr = 0;
            Glib::ustring::iterator span_text_start_iter;
            this->layout.getSourceOfCharacter(it, &rawptr, &span_text_start_iter);
            source_obj = SP_OBJECT (rawptr);
            gchar *style_text = sp_style_write_difference((SP_IS_STRING(source_obj) ? source_obj->parent : source_obj)->style, this->style);
            if (style_text && *style_text) {
                span_tspan->setAttribute("style", style_text);
                g_free(style_text);
            }

            if (SP_IS_STRING(source_obj)) {
                Glib::ustring *string = &SP_STRING(source_obj)->string;
                SPObject *span_end_obj = 0;
                void *rawptr = 0;
                Glib::ustring::iterator span_text_end_iter;
                this->layout.getSourceOfCharacter(it_span_end, &rawptr, &span_text_end_iter);
                span_end_obj = SP_OBJECT(rawptr);
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

SPItem *SPFlowtext::get_frame(SPItem *after)
{
    SPObject *ft = SP_OBJECT (this);
    SPObject *region = NULL;

    for (SPObject *o = sp_object_first_child(SP_OBJECT(ft)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
        if (SP_IS_FLOWREGION(o)) {
            region = o;
            break;
        }
    }

    if (!region) return NULL;

    bool past = false;
    SPItem *frame = NULL;

    for (SPObject *o = sp_object_first_child(SP_OBJECT(region)) ; o != NULL ; o = SP_OBJECT_NEXT(o) ) {
        if (SP_IS_ITEM(o)) {
            if (after == NULL || past) {
                frame = SP_ITEM(o);
            } else {
                if (SP_ITEM(o) == after) {
                    past = true;
                }
            }
        }
    }

    if (!frame) return NULL;

    if (SP_IS_USE (frame)) {
        return sp_use_get_original(SP_USE(frame));
    } else {
        return frame;
    }
}

bool SPFlowtext::has_internal_frame()
{
    SPItem *frame = get_frame(NULL);

    return (frame && SP_OBJECT(this)->isAncestorOf(SP_OBJECT(frame)) && SP_IS_RECT(frame));
}


SPItem *create_flowtext_with_internal_frame (SPDesktop *desktop, Geom::Point p0, Geom::Point p1)
{
    Document *doc = sp_desktop_document (desktop);

    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);
    Inkscape::XML::Node *root_repr = xml_doc->createElement("svg:flowRoot");
    root_repr->setAttribute("xml:space", "preserve"); // we preserve spaces in the text objects we create
    SPItem *ft_item = SP_ITEM(desktop->currentLayer()->appendChildRepr(root_repr));
    SPObject *root_object = doc->getObjectByRepr(root_repr);
    g_assert(SP_IS_FLOWTEXT(root_object));

    Inkscape::XML::Node *region_repr = xml_doc->createElement("svg:flowRegion");
    root_repr->appendChild(region_repr);
    SPObject *region_object = doc->getObjectByRepr(region_repr);
    g_assert(SP_IS_FLOWREGION(region_object));

    Inkscape::XML::Node *rect_repr = xml_doc->createElement("svg:rect"); // FIXME: use path!!! after rects are converted to use path
    region_repr->appendChild(rect_repr);

    SPObject *rect = doc->getObjectByRepr(rect_repr);

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

    sp_rect_position_set(SP_RECT(rect), x0, y0, w, h);
    SP_OBJECT(rect)->updateRepr();

    Inkscape::XML::Node *para_repr = xml_doc->createElement("svg:flowPara");
    root_repr->appendChild(para_repr);
    SPObject *para_object = doc->getObjectByRepr(para_repr);
    g_assert(SP_IS_FLOWPARA(para_object));

    Inkscape::XML::Node *text = xml_doc->createTextNode("");
    para_repr->appendChild(text);

    Inkscape::GC::release(root_repr);
    Inkscape::GC::release(region_repr);
    Inkscape::GC::release(para_repr);
    Inkscape::GC::release(rect_repr);

    ft_item->transform = sp_item_i2doc_affine(SP_ITEM(desktop->currentLayer())).inverse();

    return ft_item;
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
