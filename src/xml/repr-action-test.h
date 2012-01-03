#include <cxxtest/TestSuite.h>

#include <cstdlib>
#include <glib.h>

#include "repr.h"
#include "event-fns.h"

static void * const null_ptr = 0;

class XmlReprActionTest : public CxxTest::TestSuite
{
    Inkscape::XML::Document *document;
    Inkscape::XML::Node *a, *b, *c, *root;

public:

    XmlReprActionTest()
    {
        Inkscape::GC::init();

        document = sp_repr_document_new("test");
        root = document->root();

        a = document->createElement("a");
        b = document->createElement("b");
        c = document->createElement("c");
    }
    virtual ~XmlReprActionTest() {}

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static XmlReprActionTest *createSuite() { return new XmlReprActionTest(); }
    static void destroySuite( XmlReprActionTest *suite ) { delete suite; }

    void testRollbackOfNodeAddition()
    {
        sp_repr_begin_transaction(document);
        TS_ASSERT_EQUALS(a->parent() , null_ptr);

        root->appendChild(a);
        TS_ASSERT_EQUALS(a->parent() , root);

        sp_repr_rollback(document);
        TS_ASSERT_EQUALS(a->parent() , null_ptr);
    }

    void testRollbackOfNodeRemoval()
    {
        root->appendChild(a);

        sp_repr_begin_transaction(document);
        TS_ASSERT_EQUALS(a->parent() , root);

        sp_repr_unparent(a);
        TS_ASSERT_EQUALS(a->parent() , null_ptr);

        sp_repr_rollback(document);
        TS_ASSERT_EQUALS(a->parent() , root);

        sp_repr_unparent(a);
    }

    void testRollbackOfNodeReordering()
    {
        root->appendChild(a);
        root->appendChild(b);
        root->appendChild(c);

        sp_repr_begin_transaction(document);
        TS_ASSERT_EQUALS(a->next() , b);
        TS_ASSERT_EQUALS(b->next() , c);
        TS_ASSERT_EQUALS(c->next() , null_ptr);

        root->changeOrder(b, c);
        TS_ASSERT_EQUALS(a->next() , c);
        TS_ASSERT_EQUALS(b->next() , null_ptr);
        TS_ASSERT_EQUALS(c->next() , b);

        sp_repr_rollback(document);
        TS_ASSERT_EQUALS(a->next() , b);
        TS_ASSERT_EQUALS(b->next() , c);
        TS_ASSERT_EQUALS(c->next() , null_ptr);

        sp_repr_unparent(a);
        sp_repr_unparent(b);
        sp_repr_unparent(c);
    }

    /* lots more tests needed ... */
};

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
