/*
 *  AVL.cpp
 *  nlivarot
 *
 *  Created by fred on Mon Jun 16 2003.
 *
 */

#include "AVL.h"

/*
 * the algorithm explanation for this code comes from purists.org, which seems to have disappeared since
 * it's a classic AVL tree rebalancing, nothing fancy
 */

AVLTree::AVLTree()
{
    MakeNew();
}

AVLTree::~AVLTree()
{
    MakeDelete();
}

void AVLTree::MakeNew()
{
    for (int i = 0; i < 2; i++)
    {
        elem[i] = NULL;
        child[i] = NULL;
    }

    parent = NULL;
    balance = 0;
}

void AVLTree::MakeDelete()
{
    for (int i = 0; i < 2; i++) {
        if (elem[i]) {
            elem[i]->elem[1 - i] = elem[1 - i];
        }
        elem[i] = NULL;
    }
}

AVLTree *AVLTree::Leftmost()
{
    return leafFromParent(NULL, LEFT);
}

AVLTree *AVLTree::leaf(AVLTree *from, Side s)
{
    if (from == child[1 - s]) {
	if (child[s]) {
	    return child[s]->leafFromParent(this, s);
	}
	else if (parent) {
	    return parent->leaf(this, s);
	}
    }
    else if (from == child[s]) {
	if (parent) {
	    return parent->leaf(this, s);
	}
    }

    return NULL;
}

AVLTree *AVLTree::leafFromParent(AVLTree */*from*/, Side s)
{
    if (child[s]) {
	return child[s]->leafFromParent(this, s);
    }

    return this;
}

int
AVLTree::RestoreBalances (AVLTree * from, AVLTree * &racine)
{
  if (from == NULL)
    {
      if (parent)
	return parent->RestoreBalances (this, racine);
    }
  else
    {
      if (balance == 0)
	{
	  if (from == child[LEFT])
	    balance = 1;
	  if (from == child[RIGHT])
	    balance = -1;
	  if (parent)
	    return parent->RestoreBalances (this, racine);
	  return avl_no_err;
	}
      else if (balance > 0)
	{
	  if (from == child[RIGHT])
	    {
	      balance = 0;
	      return avl_no_err;
	    }
	  if (child[LEFT] == NULL)
	    {
//                              cout << "mierda\n";
	      return avl_bal_err;
	    }
	  AVLTree *a = this;
	  AVLTree *b = child[LEFT];
	  AVLTree *e = child[RIGHT];
	  AVLTree *c = child[LEFT]->child[LEFT];
	  AVLTree *d = child[LEFT]->child[RIGHT];
	  if (child[LEFT]->balance > 0)
	    {
	      AVLTree *r = parent;

	      a->parent = b;
	      b->child[RIGHT] = a;
	      a->child[RIGHT] = e;
	      if (e)
		e->parent = a;
	      a->child[LEFT] = d;
	      if (d)
		d->parent = a;
	      b->child[LEFT] = c;
	      if (c)
		c->parent = b;
	      b->parent = r;
	      if (r)
		{
		  if (r->child[LEFT] == a)
		    r->child[LEFT] = b;
		  if (r->child[RIGHT] == a)
		    r->child[RIGHT] = b;
		}
	      if (racine == a)
		racine = b;

	      a->balance = 0;
	      b->balance = 0;
	      return avl_no_err;
	    }
	  else
	    {
	      if (child[LEFT]->child[RIGHT] == NULL)
		{
		  //                              cout << "mierda\n";
		  return avl_bal_err;
		}
	      AVLTree *f = child[LEFT]->child[RIGHT]->child[LEFT];
	      AVLTree *g = child[LEFT]->child[RIGHT]->child[RIGHT];
	      AVLTree *r = parent;

	      a->parent = d;
	      d->child[RIGHT] = a;
	      b->parent = d;
	      d->child[LEFT] = b;
	      a->child[LEFT] = g;
	      if (g)
		g->parent = a;
	      a->child[RIGHT] = e;
	      if (e)
		e->parent = a;
	      b->child[LEFT] = c;
	      if (c)
		c->parent = b;
	      b->child[RIGHT] = f;
	      if (f)
		f->parent = b;

	      d->parent = r;
	      if (r)
		{
		  if (r->child[LEFT] == a)
		    r->child[LEFT] = d;
		  if (r->child[RIGHT] == a)
		    r->child[RIGHT] = d;
		}
	      if (racine == a)
		racine = d;

	      int old_bal = d->balance;
	      d->balance = 0;
	      if (old_bal == 0)
		{
		  a->balance = 0;
		  b->balance = 0;
		}
	      else if (old_bal > 0)
		{
		  a->balance = -1;
		  b->balance = 0;
		}
	      else if (old_bal < 0)
		{
		  a->balance = 0;
		  b->balance = 1;
		}
	      return avl_no_err;
	    }
	}
      else if (balance < 0)
	{
	  if (from == child[LEFT])
	    {
	      balance = 0;
	      return avl_no_err;
	    }
	  if (child[RIGHT] == NULL)
	    {
//                              cout << "mierda\n";
	      return avl_bal_err;
	    }
	  AVLTree *a = this;
	  AVLTree *b = child[RIGHT];
	  AVLTree *e = child[LEFT];
	  AVLTree *c = child[RIGHT]->child[RIGHT];
	  AVLTree *d = child[RIGHT]->child[LEFT];
	  AVLTree *r = parent;
	  if (child[RIGHT]->balance < 0)
	    {

	      a->parent = b;
	      b->child[LEFT] = a;
	      a->child[LEFT] = e;
	      if (e)
		e->parent = a;
	      a->child[RIGHT] = d;
	      if (d)
		d->parent = a;
	      b->child[RIGHT] = c;
	      if (c)
		c->parent = b;
	      b->parent = r;
	      if (r)
		{
		  if (r->child[LEFT] == a)
		    r->child[LEFT] = b;
		  if (r->child[RIGHT] == a)
		    r->child[RIGHT] = b;
		}
	      if (racine == a)
		racine = b;
	      a->balance = 0;
	      b->balance = 0;
	      return avl_no_err;
	    }
	  else
	    {
	      if (child[RIGHT]->child[LEFT] == NULL)
		{
//                                      cout << "mierda\n";
		  return avl_bal_err;
		}
	      AVLTree *f = child[RIGHT]->child[LEFT]->child[RIGHT];
	      AVLTree *g = child[RIGHT]->child[LEFT]->child[LEFT];

	      a->parent = d;
	      d->child[LEFT] = a;
	      b->parent = d;
	      d->child[RIGHT] = b;
	      a->child[RIGHT] = g;
	      if (g)
		g->parent = a;
	      a->child[LEFT] = e;
	      if (e)
		e->parent = a;
	      b->child[RIGHT] = c;
	      if (c)
		c->parent = b;
	      b->child[LEFT] = f;
	      if (f)
		f->parent = b;

	      d->parent = r;
	      if (r)
		{
		  if (r->child[LEFT] == a)
		    r->child[LEFT] = d;
		  if (r->child[RIGHT] == a)
		    r->child[RIGHT] = d;
		}
	      if (racine == a)
		racine = d;
	      int old_bal = d->balance;
	      d->balance = 0;
	      if (old_bal == 0)
		{
		  a->balance = 0;
		  b->balance = 0;
		}
	      else if (old_bal > 0)
		{
		  a->balance = 0;
		  b->balance = -1;
		}
	      else if (old_bal < 0)
		{
		  a->balance = 1;
		  b->balance = 0;
		}
	      return avl_no_err;
	    }
	}
    }
  return avl_no_err;
}

int
AVLTree::RestoreBalances (int diff, AVLTree * &racine)
{
  if (balance > 0)
    {
      if (diff < 0)
	{
	  balance = 0;
	  if (parent)
	    {
	      if (this == parent->child[RIGHT])
		return parent->RestoreBalances (1, racine);
	      if (this == parent->child[LEFT])
		return parent->RestoreBalances (-1, racine);
	    }
	  return avl_no_err;
	}
      else if (diff == 0)
	{
	}
      else if (diff > 0)
	{
	  if (child[LEFT] == NULL)
	    {
//                              cout << "un probleme\n";
	      return avl_bal_err;
	    }
	  AVLTree *r = parent;
	  AVLTree *a = this;
	  AVLTree *b = child[RIGHT];
	  AVLTree *e = child[LEFT];
	  AVLTree *f = e->child[RIGHT];
	  AVLTree *g = e->child[LEFT];
	  if (e->balance > 0)
	    {
	      e->child[RIGHT] = a;
	      e->child[LEFT] = g;
	      a->child[RIGHT] = b;
	      a->child[LEFT] = f;
	      if (a)
		a->parent = e;
	      if (g)
		g->parent = e;
	      if (b)
		b->parent = a;
	      if (f)
		f->parent = a;
	      e->parent = r;
	      if (r)
		{
		  if (r->child[LEFT] == a)
		    r->child[LEFT] = e;
		  if (r->child[RIGHT] == a)
		    r->child[RIGHT] = e;
		}
	      if (racine == this)
		racine = e;
	      e->balance = 0;
	      a->balance = 0;
	      if (r)
		{
		  if (e == r->child[RIGHT])
		    return r->RestoreBalances (1, racine);
		  if (e == r->child[LEFT])
		    return r->RestoreBalances (-1, racine);
		}
	      return avl_no_err;
	    }
	  else if (e->balance == 0)
	    {
	      e->child[RIGHT] = a;
	      e->child[LEFT] = g;
	      a->child[RIGHT] = b;
	      a->child[LEFT] = f;
	      if (a)
		a->parent = e;
	      if (g)
		g->parent = e;
	      if (b)
		b->parent = a;
	      if (f)
		f->parent = a;
	      e->parent = r;
	      if (r)
		{
		  if (r->child[LEFT] == a)
		    r->child[LEFT] = e;
		  if (r->child[RIGHT] == a)
		    r->child[RIGHT] = e;
		}
	      if (racine == this)
		racine = e;
	      e->balance = -1;
	      a->balance = 1;
	      return avl_no_err;
	    }
	  else if (e->balance < 0)
	    {
	      if (child[LEFT]->child[RIGHT] == NULL)
		{
//                                      cout << "un probleme\n";
		  return avl_bal_err;
		}
	      AVLTree *i = child[LEFT]->child[RIGHT]->child[RIGHT];
	      AVLTree *j = child[LEFT]->child[RIGHT]->child[LEFT];

	      f->child[RIGHT] = a;
	      f->child[LEFT] = e;
	      a->child[RIGHT] = b;
	      a->child[LEFT] = i;
	      e->child[RIGHT] = j;
	      e->child[LEFT] = g;
	      if (b)
		b->parent = a;
	      if (i)
		i->parent = a;
	      if (g)
		g->parent = e;
	      if (j)
		j->parent = e;
	      if (a)
		a->parent = f;
	      if (e)
		e->parent = f;
	      f->parent = r;
	      if (r)
		{
		  if (r->child[LEFT] == a)
		    r->child[LEFT] = f;
		  if (r->child[RIGHT] == a)
		    r->child[RIGHT] = f;
		}
	      if (racine == this)
		racine = f;
	      int oBal = f->balance;
	      f->balance = 0;
	      if (oBal > 0)
		{
		  a->balance = -1;
		  e->balance = 0;
		}
	      else if (oBal == 0)
		{
		  a->balance = 0;
		  e->balance = 0;
		}
	      else if (oBal < 0)
		{
		  a->balance = 0;
		  e->balance = 1;
		}
	      if (r)
		{
		  if (f == r->child[RIGHT])
		    return r->RestoreBalances (1, racine);
		  if (f == r->child[LEFT])
		    return r->RestoreBalances (-1, racine);
		}
	      return avl_no_err;
	    }
	}
    }
  else if (balance == 0)
    {
      if (diff < 0)
	{
	  balance = -1;
	}
      else if (diff == 0)
	{
	}
      else if (diff > 0)
	{
	  balance = 1;
	}
      return avl_no_err;
    }
  else if (balance < 0)
    {
      if (diff < 0)
	{
	  if (child[RIGHT] == NULL)
	    {
//                              cout << "un probleme\n";
	      return avl_bal_err;
	    }
	  AVLTree *r = parent;
	  AVLTree *a = this;
	  AVLTree *b = child[LEFT];
	  AVLTree *e = child[RIGHT];
	  AVLTree *f = e->child[LEFT];
	  AVLTree *g = e->child[RIGHT];
	  if (e->balance < 0)
	    {
	      e->child[LEFT] = a;
	      e->child[RIGHT] = g;
	      a->child[LEFT] = b;
	      a->child[RIGHT] = f;
	      if (a)
		a->parent = e;
	      if (g)
		g->parent = e;
	      if (b)
		b->parent = a;
	      if (f)
		f->parent = a;
	      e->parent = r;
	      if (r)
		{
		  if (r->child[LEFT] == a)
		    r->child[LEFT] = e;
		  if (r->child[RIGHT] == a)
		    r->child[RIGHT] = e;
		}
	      if (racine == this)
		racine = e;
	      e->balance = 0;
	      a->balance = 0;
	      if (r)
		{
		  if (e == r->child[RIGHT])
		    return r->RestoreBalances (1, racine);
		  if (e == r->child[LEFT])
		    return r->RestoreBalances (-1, racine);
		}
	      return avl_no_err;
	    }
	  else if (e->balance == 0)
	    {
	      e->child[LEFT] = a;
	      e->child[RIGHT] = g;
	      a->child[LEFT] = b;
	      a->child[RIGHT] = f;
	      if (a)
		a->parent = e;
	      if (g)
		g->parent = e;
	      if (b)
		b->parent = a;
	      if (f)
		f->parent = a;
	      e->parent = r;
	      if (r)
		{
		  if (r->child[LEFT] == a)
		    r->child[LEFT] = e;
		  if (r->child[RIGHT] == a)
		    r->child[RIGHT] = e;
		}
	      if (racine == this)
		racine = e;
	      e->balance = 1;
	      a->balance = -1;
	      return avl_no_err;
	    }
	  else if (e->balance > 0)
	    {
	      if (child[RIGHT]->child[LEFT] == NULL)
		{
//                                      cout << "un probleme\n";
		  return avl_bal_err;
		}
	      AVLTree *i = child[RIGHT]->child[LEFT]->child[LEFT];
	      AVLTree *j = child[RIGHT]->child[LEFT]->child[RIGHT];

	      f->child[LEFT] = a;
	      f->child[RIGHT] = e;
	      a->child[LEFT] = b;
	      a->child[RIGHT] = i;
	      e->child[LEFT] = j;
	      e->child[RIGHT] = g;
	      if (b)
		b->parent = a;
	      if (i)
		i->parent = a;
	      if (g)
		g->parent = e;
	      if (j)
		j->parent = e;
	      if (a)
		a->parent = f;
	      if (e)
		e->parent = f;
	      f->parent = r;
	      if (r)
		{
		  if (r->child[LEFT] == a)
		    r->child[LEFT] = f;
		  if (r->child[RIGHT] == a)
		    r->child[RIGHT] = f;
		}
	      if (racine == this)
		racine = f;
	      int oBal = f->balance;
	      f->balance = 0;
	      if (oBal > 0)
		{
		  a->balance = 0;
		  e->balance = -1;
		}
	      else if (oBal == 0)
		{
		  a->balance = 0;
		  e->balance = 0;
		}
	      else if (oBal < 0)
		{
		  a->balance = 1;
		  e->balance = 0;
		}
	      if (r)
		{
		  if (f == r->child[RIGHT])
		    return r->RestoreBalances (1, racine);
		  if (f == r->child[LEFT])
		    return r->RestoreBalances (-1, racine);
		}
	      return avl_no_err;
	    }
	}
      else if (diff == 0)
	{
	}
      else if (diff > 0)
	{
	  balance = 0;
	  if (parent)
	    {
	      if (this == parent->child[RIGHT])
		return parent->RestoreBalances (1, racine);
	      if (this == parent->child[LEFT])
		return parent->RestoreBalances (-1, racine);
	    }
	  return avl_no_err;
	}
    }
  return avl_no_err;
}

/*
 * removal
 */
int
AVLTree::Remove (AVLTree * &racine, bool rebalance)
{
  AVLTree *startNode = NULL;
  int remDiff = 0;
  int res = Remove (racine, startNode, remDiff);
  if (res == avl_no_err && rebalance && startNode)
    res = startNode->RestoreBalances (remDiff, racine);
  return res;
}

int
AVLTree::Remove (AVLTree * &racine, AVLTree * &startNode, int &diff)
{
  if (elem[LEFT])
    elem[LEFT]->elem[RIGHT] = elem[RIGHT];
  if (elem[RIGHT])
    elem[RIGHT]->elem[LEFT] = elem[LEFT];
  elem[LEFT] = elem[RIGHT] = NULL;

  if (child[LEFT] && child[RIGHT])
    {
      AVLTree *newMe = child[LEFT]->leafFromParent(this, RIGHT);
      if (newMe == NULL || newMe->child[RIGHT])
	{
//                      cout << "pas normal\n";
	  return avl_rm_err;
	}
      if (newMe == child[LEFT])
	{
	  startNode = newMe;
	  diff = -1;
	  newMe->child[RIGHT] = child[RIGHT];
	  child[RIGHT]->parent = newMe;
	  newMe->parent = parent;
	  if (parent)
	    {
	      if (parent->child[LEFT] == this)
		parent->child[LEFT] = newMe;
	      if (parent->child[RIGHT] == this)
		parent->child[RIGHT] = newMe;
	    }
	}
      else
	{
	  AVLTree *oParent = newMe->parent;
	  startNode = oParent;
	  diff = 1;

	  oParent->child[RIGHT] = newMe->child[LEFT];
	  if (newMe->child[LEFT])
	    newMe->child[LEFT]->parent = oParent;

	  newMe->parent = parent;
	  newMe->child[LEFT] = child[LEFT];
	  newMe->child[RIGHT] = child[RIGHT];
	  if (parent)
	    {
	      if (parent->child[LEFT] == this)
		parent->child[LEFT] = newMe;
	      if (parent->child[RIGHT] == this)
		parent->child[RIGHT] = newMe;
	    }
	  if (child[LEFT])
	    child[LEFT]->parent = newMe;
	  if (child[RIGHT])
	    child[RIGHT]->parent = newMe;
	}
      newMe->balance = balance;
      if (racine == this)
	racine = newMe;
    }
  else if (child[LEFT])
    {
      startNode = parent;
      diff = 0;
      if (parent)
	{
	  if (this == parent->child[LEFT])
	    diff = -1;
	  if (this == parent->child[RIGHT])
	    diff = 1;
	}
      if (parent)
	{
	  if (parent->child[LEFT] == this)
	    parent->child[LEFT] = child[LEFT];
	  if (parent->child[RIGHT] == this)
	    parent->child[RIGHT] = child[LEFT];
	}
      if (child[LEFT]->parent == this)
	child[LEFT]->parent = parent;
      if (racine == this)
	racine = child[LEFT];
    }
  else if (child[RIGHT])
    {
      startNode = parent;
      diff = 0;
      if (parent)
	{
	  if (this == parent->child[LEFT])
	    diff = -1;
	  if (this == parent->child[RIGHT])
	    diff = 1;
	}
      if (parent)
	{
	  if (parent->child[LEFT] == this)
	    parent->child[LEFT] = child[RIGHT];
	  if (parent->child[RIGHT] == this)
	    parent->child[RIGHT] = child[RIGHT];
	}
      if (child[RIGHT]->parent == this)
	child[RIGHT]->parent = parent;
      if (racine == this)
	racine = child[RIGHT];
    }
  else
    {
      startNode = parent;
      diff = 0;
      if (parent)
	{
	  if (this == parent->child[LEFT])
	    diff = -1;
	  if (this == parent->child[RIGHT])
	    diff = 1;
	}
      if (parent)
	{
	  if (parent->child[LEFT] == this)
	    parent->child[LEFT] = NULL;
	  if (parent->child[RIGHT] == this)
	    parent->child[RIGHT] = NULL;
	}
      if (racine == this)
	racine = NULL;
    }
  parent = child[RIGHT] = child[LEFT] = NULL;
  balance = 0;
  return avl_no_err;
}

/*
 * insertion
 */
int
AVLTree::Insert (AVLTree * &racine, int insertType, AVLTree * insertL,
		 AVLTree * insertR, bool rebalance)
{
  int res = Insert (racine, insertType, insertL, insertR);
  if (res == avl_no_err && rebalance)
    res = RestoreBalances ((AVLTree *) NULL, racine);
  return res;
}

int
AVLTree::Insert (AVLTree * &racine, int insertType, AVLTree * insertL,
		 AVLTree * insertR)
{
  if (racine == NULL)
    {
      racine = this;
      return avl_no_err;
    }
  else
    {
      if (insertType == not_found)
	{
//                      cout << "pb avec l'arbre de raster\n";
	  return avl_ins_err;
	}
      else if (insertType == found_on_left)
	{
	  if (insertR == NULL || insertR->child[LEFT])
	    {
//                              cout << "ngou?\n";
	      return avl_ins_err;
	    }
	  insertR->child[LEFT] = this;
	  parent = insertR;
	  insertOn(LEFT, insertR);
	}
      else if (insertType == found_on_right)
	{
	  if (insertL == NULL || insertL->child[RIGHT])
	    {
//                              cout << "ngou?\n";
	      return avl_ins_err;
	    }
	  insertL->child[RIGHT] = this;
	  parent = insertL;
	  insertOn(RIGHT, insertL);
	}
      else if (insertType == found_between)
	{
	  if (insertR == NULL || insertL == NULL
	      || (insertR->child[LEFT] != NULL && insertL->child[RIGHT] != NULL))
	    {
//                              cout << "ngou?\n";
	      return avl_ins_err;
	    }
	  if (insertR->child[LEFT] == NULL)
	    {
	      insertR->child[LEFT] = this;
	      parent = insertR;
	    }
	  else if (insertL->child[RIGHT] == NULL)
	    {
	      insertL->child[RIGHT] = this;
	      parent = insertL;
	    }
	  insertBetween (insertL, insertR);
	}
      else if (insertType == found_exact)
	{
	  if (insertL == NULL)
	    {
//                              cout << "ngou?\n";
	      return avl_ins_err;
	    }
	  // et on insere

	  if (insertL->child[RIGHT])
	    {
		insertL = insertL->child[RIGHT]->leafFromParent(insertL, LEFT);
	      if (insertL->child[LEFT])
		{
//                                      cout << "ngou?\n";
		  return avl_ins_err;
		}
	      insertL->child[LEFT] = this;
	      this->parent = insertL;
	      insertBetween (insertL->elem[LEFT], insertL);
	    }
	  else
	    {
	      insertL->child[RIGHT] = this;
	      parent = insertL;
	      insertBetween (insertL, insertL->elem[RIGHT]);
	    }
	}
      else
	{
	  //                      cout << "code incorrect\n";
	  return avl_ins_err;
	}
    }
  return avl_no_err;
}

void
AVLTree::Relocate (AVLTree * to)
{
  if (elem[LEFT])
    elem[LEFT]->elem[RIGHT] = to;
  if (elem[RIGHT])
    elem[RIGHT]->elem[LEFT] = to;
  to->elem[LEFT] = elem[LEFT];
  to->elem[RIGHT] = elem[RIGHT];
    
  if (parent)
    {
      if (parent->child[LEFT] == this)
	parent->child[LEFT] = to;
      if (parent->child[RIGHT] == this)
	parent->child[RIGHT] = to;
    }
  if (child[RIGHT])
    {
      child[RIGHT]->parent = to;
    }
  if (child[LEFT])
    {
      child[LEFT]->parent = to;
    }
  to->parent = parent;
  to->child[RIGHT] = child[RIGHT];
  to->child[LEFT] = child[LEFT];
}


void AVLTree::insertOn(Side s, AVLTree *of)
{
  elem[1 - s] = of;
  if (of)
    of->elem[s] = this;
}

void AVLTree::insertBetween(AVLTree *l, AVLTree *r)
{
  if (l)
    l->elem[RIGHT] = this;
  if (r)
    r->elem[LEFT] = this;
  elem[LEFT] = l;
  elem[RIGHT] = r;
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
