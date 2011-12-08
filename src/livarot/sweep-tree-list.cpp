#include <glib.h>
#include "livarot/sweep-tree.h"
#include "livarot/sweep-tree-list.h"


SweepTreeList::SweepTreeList(int s) :
    nbTree(0),
    maxTree(s),
    trees((SweepTree *) g_malloc(s * sizeof(SweepTree))),
    racine(NULL)
{
    /* FIXME: Use new[] for trees initializer above, but watch out for bad things happening when
     * SweepTree::~SweepTree is called.
     */
}


SweepTreeList::~SweepTreeList()
{
    g_free(trees);
    trees = NULL;
}


SweepTree *SweepTreeList::add(Shape *iSrc, int iBord, int iWeight, int iStartPoint, Shape */*iDst*/)
{
    if (nbTree >= maxTree) {
        return NULL;
    }

    int const n = nbTree++;
    trees[n].MakeNew(iSrc, iBord, iWeight, iStartPoint);

    return trees + n;
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
