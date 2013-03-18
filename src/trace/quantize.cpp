/*
 * Quantization for Inkscape
 *
 * Authors:
 *   Stéphane Gimenez <dev@gim.name>
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cassert>
#include <cstdio>
#include <stdlib.h>
#include <new>

#include "pool.h"
#include "imagemap.h"
#include "quantize.h"

typedef struct Ocnode_def Ocnode;

/**
 * an octree node datastructure
 */
struct Ocnode_def
{
    Ocnode *parent;           // parent node
    Ocnode **ref;             // node's reference
    Ocnode *child[8];         // children
    int nchild;               // number of children
    int width;                // width level of this node
    RGB rgb;                  // rgb's prefix of that node
    unsigned long weight;     // number of pixels this node accounts for
    unsigned long rs, gs, bs; // sum of pixels colors this node accounts for
    int nleaf;                // number of leaves under this node
    unsigned long mi;         // minimum impact
};

/*
-- algorithm principle:

- inspired by the octree method, we associate a tree to a given color map

- nodes in those trees have this shape:

                                parent
                                   |
        color_prefix(stored in rgb):width
     colors_sum(stored in rs,gs,bs)/weight
         /               |               \
     child1           child2           child3

- (grayscale) trees associated to pixels with colors 87 = 0b1010111 and
  69 = 0b1000101 are:

           .                 .    <-- roots of the trees
           |                 |
    1010111:0  and    1000101:0   <-- color prefixes, written in binary form
         87/1              69/1   <-- color sums, written in decimal form

- the result of merging the two trees is:

                   .
                   |
                 10:5       <----- longest common prefix and binary width
                156/2       <---.  of the covered color range.
            /            \      |
    1000101:0      1010111:0    '- sum of colors and quantity of pixels
         69/1           87/1       this node accounts for

  one should consider three cases when two trees are to be merged:
  - one tree range is included in the range of the other one, and the first
    tree has to be inserted as a child (or merged with the corresponding
    child) of the other.
  - their ranges are the same, and their children have to be merged under
    a single root.
  - ranges have no intersection, and a fork node has to be created (like in
    the given example).

- a tree for an image is built dividing the image in 2 parts and merging
  the trees obtained recursively for the two parts. a tree for a one pixel
  part is a leaf like one of those which were given above.

- last, this tree is reduced a specified number of leaves, deleting first
  leaves with minimal impact i.e. [ weight * 2^(2*parentwidth) ] value :
  a fair approximation of the impact a leaf removal would have on the final
  result : it's the corresponding covered area times the square of the
  introduced color distance.

  deletion of a node A below a node with only two children is done as
  follows :

  - when the brother is a leaf, the brother is deleted as well, both nodes
    are then represented by their father.

     |               |
     .       ==>     .
    / \
   A   .

  - otherwise the deletion of A deletes also his father, which plays no
    role anymore:

     |                |
     .       ==>       \
    / \                 |
   A   .                .
      / \              / \

  in that way, every leaf removal operation really decreases the remaining
  total number of leaves by one.

- very last, color indexes are attributed to leaves; associated colors are
  averages, computed from weight and color components sums.

-- improvements to the usual octree method:

- since this algorithm shall often be used to perform quantization using a
  very low (2-16) set of colors and not with a usual 256 value, we choose
  more carefully which nodes are to be deleted.

- depth of leaves is not fixed to an arbitrary number (which should be 8
  when color components are in 0-255), so there is no need to go down to a
  depth of 8 for each pixel (at full precision), unless it is really
  required.

- tree merging also fastens the overall tree building, and intermediate
  processing could be done.

- a huge optimization against the stupid removal algorithm (i.e. find a best
  match over the whole tree, remove it and do it again) was implemented:
  nodes are marked with the minimal impact of the removal of a leaf below
  it. we proceed to the removal recursively. we stop when current removal
  level is above the current node minimal, otherwise reached leaves are
  removed, and every change over minimal impacts is propagated back to the
  whole tree when the recursion ends.

-- specific optimizations

- pool allocation is used to allocate nodes (increased performance on large
  images).

*/

inline RGB operator>>(RGB rgb, int s)
{
  RGB res;
  res.r = rgb.r >> s; res.g = rgb.g >> s; res.b = rgb.b >> s;
  return res;
}
inline bool operator==(RGB rgb1, RGB rgb2)
{
  return (rgb1.r == rgb2.r && rgb1.g == rgb2.g && rgb1.b == rgb2.b);
}

inline int childIndex(RGB rgb)
{
    return (((rgb.r)&1)<<2) | (((rgb.g)&1)<<1) | (((rgb.b)&1));
}

/**
 * allocate a new node
 */
inline Ocnode *ocnodeNew(pool<Ocnode> *pool)
{
    Ocnode *node = pool->draw();
    node->ref = NULL;
    node->parent = NULL;
    node->nchild = 0;
    for (int i = 0; i < 8; i++) node->child[i] = NULL;
    node->mi = 0;
    return node;
}

inline void ocnodeFree(pool<Ocnode> *pool, Ocnode *node) {
    pool->drop(node);
}


/**
 * free a full octree
 */
static void octreeDelete(pool<Ocnode> *pool, Ocnode *node)
{
    if (!node) return;
    for (int i = 0; i < 8; i++)
        octreeDelete(pool, node->child[i]);
    ocnodeFree(pool, node);
}

/**
 *  pretty-print an octree, debugging purposes
 */
#if 0
static void ocnodePrint(Ocnode *node, int indent)
{
    if (!node) return;
    printf("width:%d weight:%lu rgb:%6x nleaf:%d mi:%lu\n",
           node->width,
           node->weight,
           (unsigned int)(
           ((node->rs / node->weight) << 16) +
           ((node->gs / node->weight) << 8) +
           (node->bs / node->weight)),
           node->nleaf,
           node->mi
           );
    for (int i = 0; i < 8; i++) if (node->child[i])
        {
        for (int k = 0; k < indent; k++) printf(" ");//indentation
        printf("[%d:%p] ", i, node->child[i]);
        ocnodePrint(node->child[i], indent+2);
        }
}

void octreePrint(Ocnode *node)
{
    printf("<<octree>>\n");
    if (node) printf("[r:%p] ", node); ocnodePrint(node, 2);
}
#endif

/**
 * builds a single <rgb> color leaf at location <ref>
 */
static void ocnodeLeaf(pool<Ocnode> *pool, Ocnode **ref, RGB rgb)
{
    assert(ref);
    Ocnode *node = ocnodeNew(pool);
    node->width = 0;
    node->rgb = rgb;
    node->rs = rgb.r; node->gs = rgb.g; node->bs = rgb.b;
    node->weight = 1;
    node->nleaf = 1;
    node->mi = 0;
    node->ref = ref;
    *ref = node;
}

/**
 *  merge nodes <node1> and <node2> at location <ref> with parent <parent>
 */
static int octreeMerge(pool<Ocnode> *pool, Ocnode *parent, Ocnode **ref, Ocnode *node1, Ocnode *node2)
{
    assert(ref);
    if (!node1 && !node2) return 0;
    assert(node1 != node2);
    if (parent && !*ref) parent->nchild++;
    if (!node1)
        {
        *ref = node2; node2->ref = ref; node2->parent = parent;
        return node2->nleaf;
        }
    if (!node2)
        {
        *ref = node1; node1->ref = ref; node1->parent = parent;
        return node1->nleaf;
        }
    int dwitdth = node1->width - node2->width;
    if (dwitdth > 0 && node1->rgb == node2->rgb >> dwitdth)
        {
        //place node2 below node1
        { *ref = node1; node1->ref = ref; node1->parent = parent; }
        int i = childIndex(node2->rgb >> (dwitdth - 1));
        node1->rs += node2->rs; node1->gs += node2->gs; node1->bs += node2->bs;
        node1->weight += node2->weight;
	node1->mi = 0;
        if (node1->child[i]) node1->nleaf -= node1->child[i]->nleaf;
        node1->nleaf +=
          octreeMerge(pool, node1, &node1->child[i], node1->child[i], node2);
        return node1->nleaf;
        }
    else if (dwitdth < 0 && node2->rgb == node1->rgb >> (-dwitdth))
        {
        //place node1 below node2
        { *ref = node2; node2->ref = ref; node2->parent = parent; }
        int i = childIndex(node1->rgb >> (-dwitdth - 1));
        node2->rs += node1->rs; node2->gs += node1->gs; node2->bs += node1->bs;
        node2->weight += node1->weight;
	node2->mi = 0;
        if (node2->child[i]) node2->nleaf -= node2->child[i]->nleaf;
        node2->nleaf +=
          octreeMerge(pool, node2, &node2->child[i], node2->child[i], node1);
        return node2->nleaf;
        }
    else
        {
        //nodes have either no intersection or the same root
        Ocnode *newnode;
        newnode = ocnodeNew(pool);
        newnode->rs = node1->rs + node2->rs;
        newnode->gs = node1->gs + node2->gs;
        newnode->bs = node1->bs + node2->bs;
        newnode->weight = node1->weight + node2->weight;
        { *ref = newnode; newnode->ref = ref; newnode->parent = parent; }
        if (dwitdth == 0 && node1->rgb == node2->rgb)
            {
            //merge the nodes in <newnode>
            newnode->width = node1->width; // == node2->width
            newnode->rgb = node1->rgb;     // == node2->rgb
            newnode->nchild = 0;
            newnode->nleaf = 0;
            if (node1->nchild == 0 && node2->nchild == 0)
                newnode->nleaf = 1;
            else
                for (int i = 0; i < 8; i++)
		  if (node1->child[i] || node2->child[i])
                    newnode->nleaf +=
		      octreeMerge(pool, newnode, &newnode->child[i],
				  node1->child[i], node2->child[i]);
            ocnodeFree(pool, node1); ocnodeFree(pool, node2);
            return newnode->nleaf;
            }
        else
            {
            //use <newnode> as a fork node with children <node1> and <node2>
            int newwidth =
              node1->width > node2->width ? node1->width : node2->width;
            RGB rgb1 = node1->rgb >> (newwidth - node1->width);
            RGB rgb2 = node2->rgb >> (newwidth - node2->width);
            //according to the previous tests <rgb1> != <rgb2> before the loop
            while (!(rgb1 == rgb2))
              { rgb1 = rgb1 >> 1; rgb2 = rgb2 >> 1; newwidth++; };
            newnode->width = newwidth;
            newnode->rgb = rgb1;  // == rgb2
            newnode->nchild = 2;
            newnode->nleaf = node1->nleaf + node2->nleaf;
            int i1 = childIndex(node1->rgb >> (newwidth - node1->width - 1));
            int i2 = childIndex(node2->rgb >> (newwidth - node2->width - 1));
            node1->parent = newnode;
            node1->ref = &newnode->child[i1];
            newnode->child[i1] = node1;
            node2->parent = newnode;
            node2->ref = &newnode->child[i2];
            newnode->child[i2] = node2;
            return newnode->nleaf;
            }
        }
}

/**
 * upatade mi value for leaves
 */
static inline void ocnodeMi(Ocnode *node)
{
    node->mi = node->parent ?
       node->weight << (2 * node->parent->width) : 0;
}

/**
 * remove leaves whose prune impact value is lower than <lvl>. at most
 * <count> leaves are removed, and <count> is decreased on each removal.
 * all parameters including minimal impact values are regenerated.
 */
static void ocnodeStrip(pool<Ocnode> *pool, Ocnode **ref, int *count, unsigned long lvl)
{
    Ocnode *node = *ref;
    if (!count || !node) return;
    assert(ref == node->ref);
    if (node->nchild == 0) // leaf node
        {
        if (!node->mi) ocnodeMi(node); //mi generation may be required
	if (node->mi > lvl) return; //leaf is above strip level
        ocnodeFree(pool, node);
        *ref = NULL;
        (*count)--;
        }
    else
        {
	if (node->mi && node->mi > lvl) //node is above strip level
            return;
        node->nchild = 0;
        node->nleaf = 0;
        node->mi = 0;
        Ocnode **lonelychild = NULL;
        for (int i = 0; i < 8; i++) if (node->child[i])
            {
            ocnodeStrip(pool, &node->child[i], count, lvl);
            if (node->child[i])
                {
                lonelychild = &node->child[i];
                node->nchild++;
                node->nleaf += node->child[i]->nleaf;
                if (!node->mi || node->mi > node->child[i]->mi)
                    node->mi = node->child[i]->mi;
                }
            }
      // tree adjustments
        if (node->nchild == 0)
            {
            (*count)++;
            node->nleaf = 1;
	    ocnodeMi(node);
            }
        else if (node->nchild == 1)
            {
            if ((*lonelychild)->nchild == 0)
                {
                //remove the <lonelychild> leaf under a 1 child node
                node->nchild = 0;
                node->nleaf = 1;
		ocnodeMi(node);
                ocnodeFree(pool, *lonelychild);
                *lonelychild = NULL;
                }
            else
                {
                //make a bridge to <lonelychild> over a 1 child node
                (*lonelychild)->parent = node->parent;
                (*lonelychild)->ref = ref;
                ocnodeFree(pool, node);
                *ref = *lonelychild;
                }
            }
        }
}

/**
 * reduce the leaves of an octree to a given number
 */
static void octreePrune(pool<Ocnode> *pool, Ocnode **ref, int ncolor)
{
  assert(ref);
  assert(ncolor > 0);
  //printf("pruning down to %d colors:\n", ncolor);//debug
  int n = (*ref)->nleaf - ncolor;
  if (!*ref || n <= 0) return;
  while (n > 0)
      {
      //printf("removals to go: %10d\t", n);//debug
      //printf("current prune impact: %10lu\n", (*ref)->mi);//debug
      //calling strip with global minimum impact of the tree
      ocnodeStrip(pool, ref, &n, (*ref)->mi);
      }
}

/**
 * build an octree associated to the area of a color map <rgbmap>,
 * included in the specified (x1,y1)--(x2,y2) rectangle.
 */
static void octreeBuildArea(pool<Ocnode> *pool, RgbMap *rgbmap, Ocnode **ref,
                            int x1, int y1, int x2, int y2, int ncolor)
{
    int dx = x2 - x1, dy = y2 - y1;
    int xm = x1 + dx/2, ym = y1 + dy/2;
    Ocnode *ref1 = NULL;
    Ocnode *ref2 = NULL;
    if (dx == 1 && dy == 1)
        ocnodeLeaf(pool, ref, rgbmap->getPixel(rgbmap, x1, y1));
    else if (dx > dy)
        {
	octreeBuildArea(pool, rgbmap, &ref1, x1, y1, xm, y2, ncolor);
	octreeBuildArea(pool, rgbmap, &ref2, xm, y1, x2, y2, ncolor);
	octreeMerge(pool, NULL, ref, ref1, ref2);
	}
    else
        {
	octreeBuildArea(pool, rgbmap, &ref1, x1, y1, x2, ym, ncolor);
	octreeBuildArea(pool, rgbmap, &ref2, x1, ym, x2, y2, ncolor);
	octreeMerge(pool, NULL, ref, ref1, ref2);
	}

    //octreePrune(ref, 2*ncolor);
    //affects result quality for almost same performance :/
}

/**
 * build an octree associated to the <rgbmap> color map,
 * pruned to <ncolor> colors.
 */
static Ocnode *octreeBuild(pool<Ocnode> *pool, RgbMap *rgbmap, int ncolor)
{
    //create the octree
    Ocnode *node = NULL;
    octreeBuildArea(pool,
                    rgbmap, &node,
                    0, 0, rgbmap->width, rgbmap->height, ncolor
                    );

    //prune the octree
    octreePrune(pool, &node, ncolor);

    //octreePrint(node);//debug

    return node;
}

/**
 * compute the color palette associated to an octree.
 */
static void octreeIndex(Ocnode *node, RGB *rgbpal, int *index)
{
    if (!node) return;
    if (node->nchild == 0)
        {
        rgbpal[*index].r = node->rs / node->weight;
        rgbpal[*index].g = node->gs / node->weight;
        rgbpal[*index].b = node->bs / node->weight;
        (*index)++;
        }
    else
        for (int i = 0; i < 8; i++)
            if (node->child[i])
                octreeIndex(node->child[i], rgbpal, index);
}

/**
 * compute the squared distance between two colors
 */
static int distRGB(RGB rgb1, RGB rgb2)
{
    return
      (rgb1.r - rgb2.r) * (rgb1.r - rgb2.r)
    + (rgb1.g - rgb2.g) * (rgb1.g - rgb2.g)
    + (rgb1.b - rgb2.b) * (rgb1.b - rgb2.b);
}

/**
 * find the index of closest color in a palette
 */
static int findRGB(RGB *rgbpal, int ncolor, RGB rgb)
{
    //assert(ncolor > 0);
    //assert(rgbpal);
    int index = -1, dist = 0;
    for (int k = 0; k < ncolor; k++)
        {
        int d = distRGB(rgbpal[k], rgb);
        if (index == -1 || d < dist) { dist = d; index = k; }
        }
    return index;
}

/**
 * (qsort) compare two colors for brightness
 */
static int compRGB(const void *a, const void *b)
{
    RGB *ra = (RGB *)a, *rb = (RGB *)b;
    return (ra->r + ra->g + ra->b) - (rb->r + rb->g + rb->b);
}

/**
 * quantize an RGB image to a reduced number of colors.
 */
IndexedMap *rgbMapQuantize(RgbMap *rgbmap, int ncolor)
{
    assert(rgbmap);
    assert(ncolor > 0);

    IndexedMap *newmap = 0;

    pool<Ocnode> pool;

    Ocnode *tree = 0;
    try {
        tree = octreeBuild(&pool, rgbmap, ncolor);
    }
    catch (std::bad_alloc &ex) {
        //should do smthg else?
    }

    if ( tree ) {
        RGB *rgbpal = new RGB[ncolor];
        int indexes = 0;
        octreeIndex(tree, rgbpal, &indexes);

        octreeDelete(&pool, tree);

        // stacking with increasing contrasts
        qsort((void *)rgbpal, indexes, sizeof(RGB), compRGB);

        // make the new map
        newmap = IndexedMapCreate(rgbmap->width, rgbmap->height);
        if (newmap) {
            // fill in the color lookup table
            for (int i = 0; i < indexes; i++) {
                newmap->clut[i] = rgbpal[i];
            }
            newmap->nrColors = indexes;

            // fill in new map pixels
            for (int y = 0; y < rgbmap->height; y++) {
                for (int x = 0; x < rgbmap->width; x++) {
                    RGB rgb = rgbmap->getPixel(rgbmap, x, y);
                    int index = findRGB(rgbpal, ncolor, rgb);
                    newmap->setPixel(newmap, x, y, index);
                }
            }
        }
        delete[] rgbpal;
    }

    return newmap;
}
