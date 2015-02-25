#include <libcroco/cr-parser.h>
#include "xml/node-event-vector.h"
#include "xml/repr.h"
#include "document.h"
#include "sp-style-elem.h"
#include "attributes.h"
#include "style.h"
using Inkscape::XML::TEXT_NODE;

SPStyleElem::SPStyleElem() : SPObject() {
    media_set_all(this->media);
    this->is_css = false;
}

SPStyleElem::~SPStyleElem() {
}

void SPStyleElem::set(unsigned int key, const gchar* value) {
    switch (key) {
        case SP_ATTR_TYPE: {
            if (!value) {
                /* TODO: `type' attribute is required.  Give error message as per
                   http://www.w3.org/TR/SVG11/implnote.html#ErrorProcessing. */
                is_css = false;
            } else {
                /* fixme: determine what whitespace is allowed.  Will probably need to ask on SVG
                  list; though the relevant RFC may give info on its lexer. */
                is_css = ( g_ascii_strncasecmp(value, "text/css", 8) == 0
                           && ( value[8] == '\0' ||
                                value[8] == ';'    ) );
            }
            break;
        }

#if 0 /* unfinished */
        case SP_ATTR_MEDIA: {
            parse_media(style_elem, value);
            break;
        }
#endif

        /* title is ignored. */
        default: {
            SPObject::set(key, value);
            break;
        }
    }
}


static void
child_add_rm_cb(Inkscape::XML::Node *, Inkscape::XML::Node *, Inkscape::XML::Node *,
                void *const data)
{
    SPObject *obj = reinterpret_cast<SPObject *>(data);
    g_assert(data != NULL);
    obj->read_content();
}

static void
content_changed_cb(Inkscape::XML::Node *, gchar const *, gchar const *,
                   void *const data)
{
    SPObject *obj = reinterpret_cast<SPObject *>(data);
    g_assert(data != NULL);
    obj->read_content();
}

static void
child_order_changed_cb(Inkscape::XML::Node *, Inkscape::XML::Node *,
                       Inkscape::XML::Node *, Inkscape::XML::Node *,
                       void *const data)
{
    SPObject *obj = reinterpret_cast<SPObject *>(data);
    g_assert(data != NULL);
    obj->read_content();
}

Inkscape::XML::Node* SPStyleElem::write(Inkscape::XML::Document* xml_doc, Inkscape::XML::Node* repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:style");
    }

    if (flags & SP_OBJECT_WRITE_BUILD) {
        g_warning("nyi: Forming <style> content for SP_OBJECT_WRITE_BUILD.");
        /* fixme: Consider having the CRStyleSheet be a member of SPStyleElem, and then
           pretty-print to a string s, then repr->addChild(xml_doc->createTextNode(s), NULL). */
    }
    if (is_css) {
        repr->setAttribute("type", "text/css");
    }
    /* todo: media */

    SPObject::write(xml_doc, repr, flags);

    return repr;
}


/** Returns the concatenation of the content of the text children of the specified object. */
static GString *
concat_children(Inkscape::XML::Node const &repr)
{
    GString *ret = g_string_sized_new(0);
    // effic: 0 is just to catch bugs.  Increase to something reasonable.
    for (Inkscape::XML::Node const *rch = repr.firstChild(); rch != NULL; rch = rch->next()) {
        if ( rch->type() == TEXT_NODE ) {
            ret = g_string_append(ret, rch->content());
        }
    }
    return ret;
}



/* Callbacks for SAC-style libcroco parser. */

enum StmtType { NO_STMT, FONT_FACE_STMT, NORMAL_RULESET_STMT };

struct ParseTmp
{
    CRStyleSheet *const stylesheet;
    StmtType stmtType;
    CRStatement *currStmt;
    unsigned magic;
    static unsigned const ParseTmp_magic = 0x23474397;  // from /dev/urandom

    ParseTmp(CRStyleSheet *const stylesheet) :
        stylesheet(stylesheet),
        stmtType(NO_STMT),
        currStmt(NULL),
        magic(ParseTmp_magic)
    { }

    bool hasMagic() const {
        return magic == ParseTmp_magic;
    }

    ~ParseTmp()
    {
        g_return_if_fail(hasMagic());
        magic = 0;
    }
};

static void
start_selector_cb(CRDocHandler *a_handler,
                  CRSelector *a_sel_list)
{
    g_return_if_fail(a_handler && a_sel_list);
    g_return_if_fail(a_handler->app_data != NULL);
    ParseTmp &parse_tmp = *static_cast<ParseTmp *>(a_handler->app_data);
    g_return_if_fail(parse_tmp.hasMagic());
    if ( (parse_tmp.currStmt != NULL)
         || (parse_tmp.stmtType != NO_STMT) ) {
        g_warning("Expecting currStmt==NULL and stmtType==0 (NO_STMT) at start of ruleset, but found currStmt=%p, stmtType=%u",
                  static_cast<void *>(parse_tmp.currStmt), unsigned(parse_tmp.stmtType));
        // fixme: Check whether we need to unref currStmt if non-NULL.
    }
    CRStatement *ruleset = cr_statement_new_ruleset(parse_tmp.stylesheet, a_sel_list, NULL, NULL);
    g_return_if_fail(ruleset && ruleset->type == RULESET_STMT);
    parse_tmp.stmtType = NORMAL_RULESET_STMT;
    parse_tmp.currStmt = ruleset;
}

static void
end_selector_cb(CRDocHandler *a_handler,
                CRSelector *a_sel_list)
{
    g_return_if_fail(a_handler && a_sel_list);
    g_return_if_fail(a_handler->app_data != NULL);
    ParseTmp &parse_tmp = *static_cast<ParseTmp *>(a_handler->app_data);
    g_return_if_fail(parse_tmp.hasMagic());
    CRStatement *const ruleset = parse_tmp.currStmt;
    if (parse_tmp.stmtType == NORMAL_RULESET_STMT
        && ruleset
        && ruleset->type == RULESET_STMT
        && ruleset->kind.ruleset->sel_list == a_sel_list)
    {
        parse_tmp.stylesheet->statements = cr_statement_append(parse_tmp.stylesheet->statements,
                                                               ruleset);
    } else {
        g_warning("Found stmtType=%u, stmt=%p, stmt.type=%u, ruleset.sel_list=%p, a_sel_list=%p.",
                  unsigned(parse_tmp.stmtType),
                  ruleset,
                  unsigned(ruleset->type),
                  ruleset->kind.ruleset->sel_list,
                  a_sel_list);
    }
    parse_tmp.currStmt = NULL;
    parse_tmp.stmtType = NO_STMT;
}

static void
start_font_face_cb(CRDocHandler *a_handler,
                   CRParsingLocation *)
{
    g_return_if_fail(a_handler->app_data != NULL);
    ParseTmp &parse_tmp = *static_cast<ParseTmp *>(a_handler->app_data);
    g_return_if_fail(parse_tmp.hasMagic());
    if (parse_tmp.stmtType != NO_STMT || parse_tmp.currStmt != NULL) {
        g_warning("Expecting currStmt==NULL and stmtType==0 (NO_STMT) at start of @font-face, but found currStmt=%p, stmtType=%u",
                  static_cast<void *>(parse_tmp.currStmt), unsigned(parse_tmp.stmtType));
        // fixme: Check whether we need to unref currStmt if non-NULL.
    }
    parse_tmp.stmtType = FONT_FACE_STMT;
    parse_tmp.currStmt = NULL;
}

static void
end_font_face_cb(CRDocHandler *a_handler)
{
    g_return_if_fail(a_handler->app_data != NULL);
    ParseTmp &parse_tmp = *static_cast<ParseTmp *>(a_handler->app_data);
    g_return_if_fail(parse_tmp.hasMagic());
    if (parse_tmp.stmtType != FONT_FACE_STMT || parse_tmp.currStmt != NULL) {
        g_warning("Expecting currStmt==NULL and stmtType==1 (FONT_FACE_STMT) at end of @font-face, but found currStmt=%p, stmtType=%u",
                  static_cast<void *>(parse_tmp.currStmt), unsigned(parse_tmp.stmtType));
        // fixme: Check whether we need to unref currStmt if non-NULL.
        parse_tmp.currStmt = NULL;
    }
    parse_tmp.stmtType = NO_STMT;
}

static void
property_cb(CRDocHandler *const a_handler,
            CRString *const a_name,
            CRTerm *const a_value, gboolean const a_important)
{
    g_return_if_fail(a_handler && a_name);
    g_return_if_fail(a_handler->app_data != NULL);
    ParseTmp &parse_tmp = *static_cast<ParseTmp *>(a_handler->app_data);
    g_return_if_fail(parse_tmp.hasMagic());
    if (parse_tmp.stmtType == FONT_FACE_STMT) {
        if (parse_tmp.currStmt != NULL) {
            g_warning("Found non-NULL currStmt %p though stmtType==FONT_FACE_STMT.", parse_tmp.currStmt);
        }
        /* We currently ignore @font-face descriptors. */
        return;
    }
    CRStatement *const ruleset = parse_tmp.currStmt;
    g_return_if_fail(ruleset
                     && ruleset->type == RULESET_STMT
                     && parse_tmp.stmtType == NORMAL_RULESET_STMT);
    CRDeclaration *const decl = cr_declaration_new(ruleset, cr_string_dup(a_name), a_value);
    g_return_if_fail(decl);
    decl->important = a_important;
    CRStatus const append_status = cr_statement_ruleset_append_decl(ruleset, decl);
    g_return_if_fail(append_status == CR_OK);
}

void SPStyleElem::read_content() {
    /* fixme: If there's more than one <style> element in a document, then the document stylesheet
     * will be set to a random one of them, even switching between them.
     *
     * However, I don't see in the spec what's supposed to happen when there are multiple <style>
     * elements.  The wording suggests that <style>'s content should be a full stylesheet.
     * http://www.w3.org/TR/REC-CSS2/cascade.html#cascade says that "The author specifies style
     * sheets for a source document according to the conventions of the document language. For
     * instance, in HTML, style sheets may be included in the document or linked externally."
     * (Note the plural in both sentences.)  Whereas libcroco's CRCascade allows only one author
     * stylesheet.  CRStyleSheet has no next/prev members that I can see, nor can I see any append
     * stuff.
     *
     * Dodji replies "right, that's *bug*"; just an unexpected oversight.
     */

    //XML Tree being used directly here while it shouldn't be.
    GString *const text = concat_children(*getRepr());
    CRParser *parser = cr_parser_new_from_buf(reinterpret_cast<guchar *>(text->str), text->len,
                                              CR_UTF_8, FALSE);

    /* I see a cr_statement_parse_from_buf for returning a CRStatement*, but no corresponding
       cr_stylesheet_parse_from_buf.  And cr_statement_parse_from_buf takes a char*, not a
       CRInputBuf, and doesn't provide a way for calling it in a loop over the one buffer.
       (I.e. it doesn't tell us where it got up to in the buffer.)

       There's also the generic cr_parser_parse_stylesheet (or just cr_parser_parse), but that
       just calls user-supplied callbacks rather than constructing a CRStylesheet.
    */
    CRDocHandler *sac_handler = cr_doc_handler_new();
    // impl: ref_count inited to 0, so cr_parser_destroy suffices to delete sac_handler.
    g_return_if_fail(sac_handler);  // out of memory
    CRStyleSheet *const stylesheet = cr_stylesheet_new(NULL);
    ParseTmp parse_tmp(stylesheet);
    sac_handler->app_data = &parse_tmp;
    sac_handler->start_selector = start_selector_cb;
    sac_handler->end_selector = end_selector_cb;
    sac_handler->start_font_face = start_font_face_cb;
    sac_handler->end_font_face = end_font_face_cb;
    sac_handler->property = property_cb;
    /* todo: start_media, end_media. */
    /* todo: Test error condition. */
    cr_parser_set_sac_handler(parser, sac_handler);
    CRStatus const parse_status = cr_parser_parse(parser);
    g_assert(sac_handler->app_data == &parse_tmp);
    if (parse_status == CR_OK) {
        cr_cascade_set_sheet(document->style_cascade, stylesheet, ORIGIN_AUTHOR);
    } else {
        if (parse_status != CR_PARSING_ERROR) {
            g_printerr("parsing error code=%u\n", unsigned(parse_status));
            /* Better than nothing.  TODO: Improve libcroco's error handling.  At a minimum, add a
               strerror-like function so that we can give a string rather than an integer. */
            /* TODO: Improve error diagnosis stuff in inkscape.  We'd like a panel showing the
               errors/warnings/unsupported features of the current document. */
        }
    }
    cr_parser_destroy(parser);
    //requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);

    // Style references via class= do not, and actually cannot, use autoupdating URIReferences.
    // Therefore, if an object refers to a stylesheet which has not yet loaded when the object is
    // being loaded (e.g. if the stylesheet is below or inside the object in XML), its class= has
    // no effect (bug 1491639).  Below is a partial hack that fixes this for a single case: when
    // the <style> is a child of the object that uses a style from it. It just forces the parent of
    // <style> to reread its style as soon as the stylesheet is fully loaded. Naturally, this won't
    // work if the user of the stylesheet is its grandparent or precedent.
    if ( parent ) {
        parent->style->readFromObject( parent );
    }
}

/**
 * Does addListener(fns, data) on \a repr and all of its descendents.
 */
static void
rec_add_listener(Inkscape::XML::Node &repr,
                 Inkscape::XML::NodeEventVector const *const fns, void *const data)
{
    repr.addListener(fns, data);
    for (Inkscape::XML::Node *child = repr.firstChild(); child != NULL; child = child->next()) {
        rec_add_listener(*child, fns, data);
    }
}

void SPStyleElem::build(SPDocument *document, Inkscape::XML::Node *repr) {
    read_content();

    readAttr( "type" );
    readAttr( "media" );

    static Inkscape::XML::NodeEventVector const nodeEventVector = {
        child_add_rm_cb,   // child_added
        child_add_rm_cb,   // child_removed
        NULL,   // attr_changed
        content_changed_cb,   // content_changed
        child_order_changed_cb,   // order_changed
    };
    rec_add_listener(*repr, &nodeEventVector, this);

    SPObject::build(document, repr);
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
