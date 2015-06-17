#ifndef SEEN_OBJECT_TEST_H
#define SEEN_OBJECT_TEST_H

#include <cassert>
#include <ctime>
#include <cxxtest/TestSuite.h>
#include <string>
#include <vector>

#include "document.h"
#include "sp-item-group.h"
#include "sp-object.h"
#include "sp-path.h"
#include "sp-root.h"
#include "xml/document.h"
#include "xml/node.h"

class ObjectTest : public CxxTest::TestSuite
{
public:
    virtual ~ObjectTest() {}

    static ObjectTest *createSuite() { return new ObjectTest(); }
    static void destroySuite(ObjectTest *suite) { delete suite; }

    void testObjects()
    {
        clock_t begin, end;
        // Sample document
        // svg:svg
        // svg:defs
        // svg:path
        // svg:linearGradient
        // svg:stop
        // svg:filter
        // svg:feGaussianBlur (feel free to implement for other filters)
        // svg:clipPath
        // svg:rect
        // svg:g
        // svg:use
        // svg:circle
        // svg:ellipse
        // svg:text
        // svg:polygon
        // svg:polyline
        // svg:image
        // svg:line
        
        char const *docString = 
        "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n"
        "<!-- just a comment -->\n"
        "<title id=\"title\">SVG test</title>\n"
        "<defs>\n"
        "  <path id=\"P\" d=\"M -21,-4 -5,0 -18,12 -3,4 -4,21 0,5 12,17 4,2 21,3 5,-1 17,-12 2,-4 3,-21 -1,-5 -12,-18 -4,-3z\"/>\n"
        "  <linearGradient id=\"LG\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\">\n"
        "    <stop offset=\"0%\" style=\"stop-color:#ffff00;stop-opacity:1\"/>\n"
        "    <stop offset=\"100%\" style=\"stop-color:red;stop-opacity:1\"/>\n"
        "  </linearGradient>\n"
        "  <clipPath id=\"clip\" clipPathUnits=\"userSpaceOnUse\">\n"
        "    <rect x=\"10\" y=\"10\" width=\"100\" height=\"100\"/>\n"
        "  </clipPath>\n"
        "  <filter style=\"color-interpolation-filters:sRGB\" id=\"filter\" x=\"-0.15\" width=\"1.34\" y=\"0\" height=\"1\">\n"
        "    <feGaussianBlur stdDeviation=\"4.26\"/>\n"
        "  </filter>\n"
        "</defs>\n"

        "<g id=\"G\" transform=\"skewX(10.5) translate(9,5)\">\n"
        "  <use id=\"U\" xlink:href=\"#P\" opacity=\"0.5\" fill=\"#1dace3\" transform=\"rotate(4)\"/>\n"
        "  <circle id=\"C\" cx=\"45.5\" cy=\"67\" r=\"23\" fill=\"#000\"/>\n"
        "  <ellipse id=\"E\" cx=\"200\" cy=\"70\" rx=\"85\" ry=\"55\" fill=\"url(#LG)\"/>\n"
        "  <text id=\"T\" fill=\"#fff\" style=\"font-size:45;font-family:Verdana\" x=\"150\" y=\"86\">TEST</text>\n"
        "  <polygon id=\"PG\" points=\"60,20 100,40 100,80 60,100 20,80 20,40\" clip-path=\"url(#clip)\" filter=\"url(#filter)\"/>\n"
        "  <polyline id=\"PL\" points=\"0,40 40,40 40,80 80,80 80,120 120,120 120,160\" style=\"fill:none;stroke:red;stroke-width:4\"/>\n"
        "  <image id=\"I\" xlink:href=\"data:image/svg+xml;base64,PHN2ZyBoZWlnaHQ9IjE4MCIgd2lkdGg9IjUwMCI+PHBhdGggZD0iTTAsNDAgNDAsNDAgNDAs" // this is one line
        "ODAgODAsODAgODAsMTIwIDEyMCwxMjAgMTIwLDE2MCIgc3R5bGU9ImZpbGw6d2hpdGU7c3Ryb2tlOnJlZDtzdHJva2Utd2lkdGg6NCIvPjwvc3ZnPgo=\"/>\n"
        "  <line id=\"L\" x1=\"20\" y1=\"100\" x2=\"100\" y2=\"20\" stroke=\"black\" stroke-width=\"2\"/>\n"
        "</g>\n"
        "</svg>\n";

        begin = clock();
        SPDocument *doc = SPDocument::createNewDocFromMem(docString, strlen(docString), false);
        end = clock();
        
        assert(doc != NULL); // cannot continue if doc is null, abort!
        assert(doc->getRoot() != NULL);
        
        SPRoot *root = doc->getRoot();
        assert(root->getRepr() != NULL);
        assert(root->hasChildren());

        std::cout << "Took " << double(end - begin) / double(CLOCKS_PER_SEC) << " seconds to construct the test document\n";

        SPPath *path = dynamic_cast<SPPath *>(doc->getObjectById("P"));
        testClones(path);

        SPGroup *group = dynamic_cast<SPGroup *>(doc->getObjectById("G"));
        testGrouping(group);

        // Test parent behavior
        SPObject *child = root->firstChild();
        assert(child != NULL);
        TS_ASSERT(child->parent == root);
        TS_ASSERT(child->document == doc);
        TS_ASSERT(root->isAncestorOf(child));

        // Test list behavior
        SPObject *next = child->getNext();
        SPObject *prev = next;
        TS_ASSERT(next->getPrev() == child);
        prev = next;
        next = next->getNext();
        while (next != NULL) {
            // Walk the list
            TS_ASSERT(next->getPrev() == prev);
            prev = next;
            next = next->getNext();
        }
        TS_ASSERT(child->lastChild() == next);

        // Test hrefcount
        TS_ASSERT(path->isReferenced());
    }
    
    void testClones(SPPath *path)
    {
        clock_t begin, end;

        assert(path != NULL);

        // Since we don't yet have any clean way to do this (FIXME), we'll abuse the XML tree a bit.
        Inkscape::XML::Node *node = path->getRepr();
        assert(node != NULL);

        Inkscape::XML::Document *xml_doc = node->document();

        Inkscape::XML::Node *parent = node->parent();
        assert(parent != NULL);

        TS_TRACE("Benchmarking clones...");
        const size_t num_clones = 10000;

        std::string href(std::string("#") + std::string(path->getId()));
        std::vector<Inkscape::XML::Node *> clones(num_clones, NULL);

        begin = clock();
        // Create num_clones clones of this path and stick them in the document
        for (size_t i = 0; i < num_clones; ++i) {
            Inkscape::XML::Node *clone = xml_doc->createElement("svg:use");
            Inkscape::GC::release(clone);
            clone->setAttribute("xlink:href", href.c_str());
            parent->addChild(clone, node);
            clones[i] = clone;
        }
        end = clock();

        std::cout << "Took " << double(end - begin) / double(CLOCKS_PER_SEC) << " seconds to write " << num_clones << " clones of a path\n";

        begin = clock();
        // Remove those clones
        for (size_t i = num_clones - 1; i >= 1; --i) {
            parent->removeChild(clones[i]);
        }
        end = clock();

        std::cout << "Took " << double(end - begin) / double(CLOCKS_PER_SEC) << " seconds to remove " << num_clones << " clones of a path\n";
    }

    void testGrouping(SPGroup *group)
    {
        clock_t begin, end;

        assert(group != NULL);

        // Since we don't yet have any clean way to do this (FIXME), we'll abuse the XML tree a bit.
        Inkscape::XML::Node *node = group->getRepr();
        assert(node != NULL);

        Inkscape::XML::Document *xml_doc = node->document();

        TS_TRACE("Benchmarking groups...");
        const size_t num_elements = 10000;
        
        Inkscape::XML::Node *new_group = xml_doc->createElement("svg:g");
        Inkscape::GC::release(new_group);
        node->addChild(new_group, NULL);

        std::vector<Inkscape::XML::Node *> elements(num_elements, NULL);

        begin = clock();
        for (size_t i = 0; i < num_elements; ++i) {
            Inkscape::XML::Node *circle = xml_doc->createElement("svg:circle");
            Inkscape::GC::release(circle);
            circle->setAttribute("cx", "2048");
            circle->setAttribute("cy", "1024");
            circle->setAttribute("r", "1.5");
            new_group->addChild(circle, NULL);
            elements[i] = circle;
        }
        end = clock();

        std::cout << "Took " << double(end - begin) / double(CLOCKS_PER_SEC) << " seconds to write " << num_elements << " elements into a group\n";

        SPGroup *n_group = dynamic_cast<SPGroup *>(group->get_child_by_repr(new_group));
        assert(n_group != NULL);

        begin = clock();
        std::vector<SPItem*> ch;
        sp_item_group_ungroup(n_group, ch, false);
        end = clock();

        std::cout << "Took " << double(end - begin) / double(CLOCKS_PER_SEC) << " seconds to ungroup a <g> with " << num_elements << " elements\n";
        std::cout << "  Note: sp_item_group_ungroup_handle_clones() is responsible\n  for most of the time as it is linear in number of elements\n  which results in quadratic behavior for ungrouping." << std::endl;

        begin = clock();
        // Remove those elements
        for (size_t i = num_elements - 1; i >= 1; --i) {
            elements[i]->parent()->removeChild(elements[i]);
        }
        end = clock();

        std::cout << "Took " << double(end - begin) / double(CLOCKS_PER_SEC) << " seconds to remove " << num_elements << " elements\n";
    }
};
#endif // SEEN_OBJECT_TEST_H

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
