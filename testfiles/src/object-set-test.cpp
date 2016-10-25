/*
 * Multiindex container for selection
 *
 * Authors:
 *   Adrian Boguszewski
 *
 * Copyright (C) 2016 Adrian Boguszewski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include <gtest/gtest.h>
#include <doc-per-case-test.h>
#include <src/sp-factory.h>
#include <src/sp-rect.h>
#include <src/object-set.h>
#include <xml/node.h>
#include <src/xml/text-node.h>
#include <src/xml/simple-document.h>

using namespace Inkscape;
using namespace Inkscape::XML;

class ObjectSetTest: public DocPerCaseTest {
public:
    ObjectSetTest() {
        A = new SPObject();
        B = new SPObject();
        C = new SPObject();
        D = new SPObject();
        E = new SPObject();
        F = new SPObject();
        G = new SPObject();
        H = new SPObject();
        X = new SPObject();
        set = new ObjectSet();
        set2 = new ObjectSet();
        auto sd = new SimpleDocument();
        auto xt = new TextNode(Util::share_string("x"), sd);
        auto ht = new TextNode(Util::share_string("h"), sd);
        auto gt = new TextNode(Util::share_string("g"), sd);
        auto ft = new TextNode(Util::share_string("f"), sd);
        auto et = new TextNode(Util::share_string("e"), sd);
        auto dt = new TextNode(Util::share_string("d"), sd);
        auto ct = new TextNode(Util::share_string("c"), sd);
        auto bt = new TextNode(Util::share_string("b"), sd);
        auto at = new TextNode(Util::share_string("a"), sd);
        X->invoke_build(_doc, xt, 0);
        H->invoke_build(_doc, ht, 0);
        G->invoke_build(_doc, gt, 0);
        F->invoke_build(_doc, ft, 0);
        E->invoke_build(_doc, et, 0);
        D->invoke_build(_doc, dt, 0);
        C->invoke_build(_doc, ct, 0);
        B->invoke_build(_doc, bt, 0);
        A->invoke_build(_doc, at, 0);
    }
    ~ObjectSetTest() {
        delete set;
        delete set2;
        delete X;
        delete H;
        delete G;
        delete F;
        delete E;
        delete D;
        delete C;
        delete B;
        delete A;
    }
    SPObject* A;
    SPObject* B;
    SPObject* C;
    SPObject* D;
    SPObject* E;
    SPObject* F;
    SPObject* G;
    SPObject* H;
    SPObject* X;
    ObjectSet* set;
    ObjectSet* set2;
};

TEST_F(ObjectSetTest, Basics) {
    EXPECT_EQ(0, set->size());
    set->add(A);
    EXPECT_EQ(1, set->size());
    EXPECT_TRUE(set->includes(A));
    set->add(B);
    set->add(C);
    EXPECT_EQ(3, set->size());
    EXPECT_TRUE(set->includes(B));
    EXPECT_TRUE(set->includes(C));
    EXPECT_FALSE(set->includes(D));
    EXPECT_FALSE(set->includes(X));
    EXPECT_FALSE(set->includes(nullptr));
    set->remove(A);
    EXPECT_EQ(2, set->size());
    EXPECT_FALSE(set->includes(A));
    set->clear();
    EXPECT_EQ(0, set->size());
    bool resultNull = set->add((SPObject*)nullptr);
    EXPECT_FALSE(resultNull);
    EXPECT_EQ(0, set->size());
    bool resultNull2 = set->remove(nullptr);
    EXPECT_FALSE(resultNull2);
}

TEST_F(ObjectSetTest, Advanced) {
    set->add(A);
    set->add(B);
    set->add(C);
    EXPECT_TRUE(set->includes(C));
    set->toggle(C);
    EXPECT_EQ(2, set->size());
    EXPECT_FALSE(set->includes(C));
    set->toggle(D);
    EXPECT_EQ(3, set->size());
    EXPECT_TRUE(set->includes(D));
    set->toggle(D);
    EXPECT_EQ(2, set->size());
    EXPECT_FALSE(set->includes(D));
    EXPECT_EQ(nullptr, set->single());
    set->set(X);
    EXPECT_EQ(1, set->size());
    EXPECT_TRUE(set->includes(X));
    EXPECT_EQ(X, set->single());
    EXPECT_FALSE(set->isEmpty());
    set->clear();
    EXPECT_TRUE(set->isEmpty());
    std::vector<SPObject*> list1 {A, B, C, D};
    std::vector<SPObject*> list2 {E, F};
    set->addList(list1);
    EXPECT_EQ(4, set->size());
    set->addList(list2);
    EXPECT_EQ(6, set->size());
    EXPECT_TRUE(set->includes(A));
    EXPECT_TRUE(set->includes(B));
    EXPECT_TRUE(set->includes(C));
    EXPECT_TRUE(set->includes(D));
    EXPECT_TRUE(set->includes(E));
    EXPECT_TRUE(set->includes(F));
    set->setList(list2);
    EXPECT_EQ(2, set->size());
    EXPECT_TRUE(set->includes(E));
    EXPECT_TRUE(set->includes(F));
}

TEST_F(ObjectSetTest, Items) {
    // cannot test smallestItem and largestItem functions due to too many dependencies
    // uncomment if the problem is fixed
    SPRect* rect10x100 = (SPRect *) SPFactory::createObject("svg:rect");
//    rect10x100->invoke_build(_doc, _doc->rroot, 1);
    SPRect* rect20x40 = (SPRect *) SPFactory::createObject("svg:rect");
//    rect20x40->invoke_build(_doc, _doc->rroot, 1);
//    SPRect* rect30x30 = (SPRect *) SPFactory::createObject("svg:rect");
//    rect30x30->invoke_build(_doc, _doc->rroot, 1);
//    rect10x100->width = 10;
//    rect10x100->height = 100;
//    rect20x40->width = 20;
//    rect20x40->height = 40;
//    rect30x30->width = 30;
//    rect30x30->height = 30;
    set->add(rect10x100);
    EXPECT_EQ(rect10x100, set->singleItem());
    EXPECT_EQ(rect10x100->getRepr(), set->singleRepr());
    set->add(rect20x40);
    EXPECT_EQ(nullptr, set->singleItem());
    EXPECT_EQ(nullptr, set->singleRepr());
//    set->add(rect30x30);
//    EXPECT_EQ(3, set->size());
//    EXPECT_EQ(rect10x100, set->smallestItem(ObjectSet::CompareSize::HORIZONTAL));
//    EXPECT_EQ(rect30x30, set->smallestItem(ObjectSet::CompareSize::VERTICAL));
//    EXPECT_EQ(rect20x40, set->smallestItem(ObjectSet::CompareSize::AREA));
//    EXPECT_EQ(rect30x30, set->largestItem(ObjectSet::CompareSize::HORIZONTAL));
//    EXPECT_EQ(rect10x100, set->largestItem(ObjectSet::CompareSize::VERTICAL));
//    EXPECT_EQ(rect10x100, set->largestItem(ObjectSet::CompareSize::AREA));
}

TEST_F(ObjectSetTest, Ranges) {
    std::vector<SPObject*> objs {A, D, B, E, C, F};
    set->add(objs.begin() + 1, objs.end() - 1);
    EXPECT_EQ(4, set->size());
    auto it = set->objects().begin();
    EXPECT_EQ(D, *it++);
    EXPECT_EQ(B, *it++);
    EXPECT_EQ(E, *it++);
    EXPECT_EQ(C, *it++);
    EXPECT_EQ(set->objects().end(), it);
    SPObject* rect1 = SPFactory::createObject("svg:rect");
    SPObject* rect2 = SPFactory::createObject("svg:rect");
    SPObject* rect3 = SPFactory::createObject("svg:rect");
    set->add(rect1);
    set->add(rect2);
    set->add(rect3);
    EXPECT_EQ(7, set->size());
    auto xmlNode = set->xmlNodes().begin();
    EXPECT_EQ(3, boost::distance(set->xmlNodes()));
    EXPECT_EQ(rect1->getRepr(), *xmlNode++);
    EXPECT_EQ(rect2->getRepr(), *xmlNode++);
    EXPECT_EQ(rect3->getRepr(), *xmlNode++);
    EXPECT_EQ(set->xmlNodes().end(), xmlNode);
    auto item = set->items().begin();
    EXPECT_EQ(3, boost::distance(set->items()));
    EXPECT_EQ(rect1, *item++);
    EXPECT_EQ(rect2, *item++);
    EXPECT_EQ(rect3, *item++);
    EXPECT_EQ(set->items().end(), item);
}

TEST_F(ObjectSetTest, Autoremoving) {
    set->add(A);
    EXPECT_TRUE(set->includes(A));
    EXPECT_EQ(1, set->size());
    A->releaseReferences();
    EXPECT_EQ(0, set->size());
}

TEST_F(ObjectSetTest, BasicDescendants) {
    A->attach(B, nullptr);
    B->attach(C, nullptr);
    A->attach(D, nullptr);
    bool resultB = set->add(B);
    bool resultB2 = set->add(B);
    EXPECT_TRUE(resultB);
    EXPECT_FALSE(resultB2);
    EXPECT_TRUE(set->includes(B));
    bool resultC = set->add(C);
    EXPECT_FALSE(resultC);
    EXPECT_FALSE(set->includes(C));
    EXPECT_EQ(1, set->size());
    bool resultA = set->add(A);
    EXPECT_TRUE(resultA);
    EXPECT_EQ(1, set->size());
    EXPECT_TRUE(set->includes(A));
    EXPECT_FALSE(set->includes(B));
}

TEST_F(ObjectSetTest, AdvancedDescendants) {
    A->attach(B, nullptr);
    A->attach(C, nullptr);
    A->attach(X, nullptr);
    B->attach(D, nullptr);
    B->attach(E, nullptr);
    C->attach(F, nullptr);
    C->attach(G, nullptr);
    C->attach(H, nullptr);
    set->add(A);
    bool resultF = set->remove(F);
    EXPECT_TRUE(resultF);
    EXPECT_EQ(4, set->size());
    EXPECT_FALSE(set->includes(F));
    EXPECT_TRUE(set->includes(B));
    EXPECT_TRUE(set->includes(G));
    EXPECT_TRUE(set->includes(H));
    EXPECT_TRUE(set->includes(X));
    bool resultF2 = set->add(F);
    EXPECT_TRUE(resultF2);
    EXPECT_EQ(5, set->size());
    EXPECT_TRUE(set->includes(F));
}

TEST_F(ObjectSetTest, Removing) {
    A->attach(B, nullptr);
    A->attach(C, nullptr);
    A->attach(X, nullptr);
    B->attach(D, nullptr);
    B->attach(E, nullptr);
    C->attach(F, nullptr);
    C->attach(G, nullptr);
    C->attach(H, nullptr);
    bool removeH = set->remove(H);
    EXPECT_FALSE(removeH);
    set->add(A);
    bool removeX = set->remove(X);
    EXPECT_TRUE(removeX);
    EXPECT_EQ(2, set->size());
    EXPECT_TRUE(set->includes(B));
    EXPECT_TRUE(set->includes(C));
    EXPECT_FALSE(set->includes(X));
    EXPECT_FALSE(set->includes(A));
    bool removeX2 = set->remove(X);
    EXPECT_FALSE(removeX2);
    EXPECT_EQ(2, set->size());
    bool removeA = set->remove(A);
    EXPECT_FALSE(removeA);
    EXPECT_EQ(2, set->size());
    bool removeC = set->remove(C);
    EXPECT_TRUE(removeC);
    EXPECT_EQ(1, set->size());
    EXPECT_TRUE(set->includes(B));
    EXPECT_FALSE(set->includes(C));
}

TEST_F(ObjectSetTest, TwoSets) {
    A->attach(B, nullptr);
    A->attach(C, nullptr);
    set->add(A);
    set2->add(A);
    EXPECT_EQ(1, set->size());
    EXPECT_EQ(1, set2->size());
    set->remove(B);
    EXPECT_EQ(1, set->size());
    EXPECT_TRUE(set->includes(C));
    EXPECT_EQ(1, set2->size());
    EXPECT_TRUE(set2->includes(A));
    C->releaseReferences();
    EXPECT_EQ(0, set->size());
    EXPECT_EQ(1, set2->size());
    EXPECT_TRUE(set2->includes(A));
}

TEST_F(ObjectSetTest, SetRemoving) {
    ObjectSet *objectSet = new ObjectSet();
    A->attach(B, nullptr);
    objectSet->add(A);
    objectSet->add(C);
    EXPECT_EQ(2, objectSet->size());
    delete objectSet;
    EXPECT_STREQ(nullptr, A->getId());
    EXPECT_STREQ(nullptr, C->getId());
}
