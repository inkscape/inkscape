#include <stdlib.h>
#include <glib.h>
#include "../utest/utest.h"

#include "repr.h"
#include "event-fns.h"

int main(int argc, char *argv[]) {
	Inkscape::XML::Document *document;
	Inkscape::XML::Node *a, *b, *c, *root;

	Inkscape::GC::init();

	document = sp_repr_document_new("test");
	root = sp_repr_document_root(document);

	utest_start("XML Transactions");

	a = sp_repr_new("a");
	b = sp_repr_new("b");
	c = sp_repr_new("c");

	UTEST_TEST("rollback of node addition") {
		sp_repr_begin_transaction(document);
		UTEST_ASSERT(sp_repr_parent(a) == NULL);

		root->appendChild(a);
		UTEST_ASSERT(sp_repr_parent(a) == root);

		sp_repr_rollback(document);
		UTEST_ASSERT(sp_repr_parent(a) == NULL);
	}

	UTEST_TEST("rollback of node removal") {
		root->appendChild(a);

		sp_repr_begin_transaction(document);
		UTEST_ASSERT(sp_repr_parent(a) == root);

		sp_repr_unparent(a);
		UTEST_ASSERT(sp_repr_parent(a) == NULL);

		sp_repr_rollback(document);
		UTEST_ASSERT(sp_repr_parent(a) == root);
	}

	sp_repr_unparent(a);

	UTEST_TEST("rollback of node reordering") {
		root->appendChild(a);
		root->appendChild(b);
		root->appendChild(c);

		sp_repr_begin_transaction(document);
		UTEST_ASSERT(sp_repr_next(a) == b);
		UTEST_ASSERT(sp_repr_next(b) == c);
		UTEST_ASSERT(sp_repr_next(c) == NULL);

		root->changeOrder(b, c);
		UTEST_ASSERT(sp_repr_next(a) == c);
		UTEST_ASSERT(sp_repr_next(b) == NULL);
		UTEST_ASSERT(sp_repr_next(c) == b);

		sp_repr_rollback(document);
		UTEST_ASSERT(sp_repr_next(a) == b);
		UTEST_ASSERT(sp_repr_next(b) == c);
		UTEST_ASSERT(sp_repr_next(c) == NULL);
	}

	sp_repr_unparent(a);
	sp_repr_unparent(b);
	sp_repr_unparent(c);

	/* lots more tests needed ... */

	return utest_end() ? 0 : 1;
}
