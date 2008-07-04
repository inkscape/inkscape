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
        son[i] = NULL;
    }

    dad = NULL;
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
    return leafFromDad(NULL, LEFT);
}

AVLTree *AVLTree::leaf(AVLTree *from, Side s)
{
    if (from == son[1 - s]) {
	if (son[s]) {
	    return son[s]->leafFromDad(this, s);
	}
	else if (dad) {
	    return dad->leaf(this, s);
	}
    }
    else if (from == son[s]) {
	if (dad) {
	    return dad->leaf(this, s);
	}
    }

    return NULL;
}

AVLTree *AVLTree::leafFromDad(AVLTree */*from*/, Side s)
{
    if (son[s]) {
	return son[s]->leafFromDad(this, s);
    }

    return this;
}

int
AVLTree::RestoreBalances (AVLTree * from, AVLTree * &racine)
{
  if (from == NULL)
    {
      if (dad)
	return dad->RestoreBalances (this, racine);
    }
  else
    {
      if (balance == 0)
	{
	  if (from == son[LEFT])
	    balance = 1;
	  if (from == son[RIGHT])
	    balance = -1;
	  if (dad)
	    return dad->RestoreBalances (this, racine);
	  return avl_no_err;
	}
      else if (balance > 0)
	{
	  if (from == son[RIGHT])
	    {
	      balance = 0;
	      return avl_no_err;
	    }
	  if (son[LEFT] == NULL)
	    {
//                              cout << "mierda\n";
	      return avl_bal_err;
	    }
	  AVLTree *a = this;
	  AVLTree *b = son[LEFT];
	  AVLTree *e = son[RIGHT];
	  AVLTree *c = son[LEFT]->son[LEFT];
	  AVLTree *d = son[LEFT]->son[RIGHT];
	  if (son[LEFT]->balance > 0)
	    {
	      AVLTree *r = dad;

	      a->dad = b;
	      b->son[RIGHT] = a;
	      a->son[RIGHT] = e;
	      if (e)
		e->dad = a;
	      a->son[LEFT] = d;
	      if (d)
		d->dad = a;
	      b->son[LEFT] = c;
	      if (c)
		c->dad = b;
	      b->dad = r;
	      if (r)
		{
		  if (r->son[LEFT] == a)
		    r->son[LEFT] = b;
		  if (r->son[RIGHT] == a)
		    r->son[RIGHT] = b;
		}
	      if (racine == a)
		racine = b;

	      a->balance = 0;
	      b->balance = 0;
	      return avl_no_err;
	    }
	  else
	    {
	      if (son[LEFT]->son[RIGHT] == NULL)
		{
		  //                              cout << "mierda\n";
		  return avl_bal_err;
		}
	      AVLTree *f = son[LEFT]->son[RIGHT]->son[LEFT];
	      AVLTree *g = son[LEFT]->son[RIGHT]->son[RIGHT];
	      AVLTree *r = dad;

	      a->dad = d;
	      d->son[RIGHT] = a;
	      b->dad = d;
	      d->son[LEFT] = b;
	      a->son[LEFT] = g;
	      if (g)
		g->dad = a;
	      a->son[RIGHT] = e;
	      if (e)
		e->dad = a;
	      b->son[LEFT] = c;
	      if (c)
		c->dad = b;
	      b->son[RIGHT] = f;
	      if (f)
		f->dad = b;

	      d->dad = r;
	      if (r)
		{
		  if (r->son[LEFT] == a)
		    r->son[LEFT] = d;
		  if (r->son[RIGHT] == a)
		    r->son[RIGHT] = d;
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
	  if (from == son[LEFT])
	    {
	      balance = 0;
	      return avl_no_err;
	    }
	  if (son[RIGHT] == NULL)
	    {
//                              cout << "mierda\n";
	      return avl_bal_err;
	    }
	  AVLTree *a = this;
	  AVLTree *b = son[RIGHT];
	  AVLTree *e = son[LEFT];
	  AVLTree *c = son[RIGHT]->son[RIGHT];
	  AVLTree *d = son[RIGHT]->son[LEFT];
	  AVLTree *r = dad;
	  if (son[RIGHT]->balance < 0)
	    {

	      a->dad = b;
	      b->son[LEFT] = a;
	      a->son[LEFT] = e;
	      if (e)
		e->dad = a;
	      a->son[RIGHT] = d;
	      if (d)
		d->dad = a;
	      b->son[RIGHT] = c;
	      if (c)
		c->dad = b;
	      b->dad = r;
	      if (r)
		{
		  if (r->son[LEFT] == a)
		    r->son[LEFT] = b;
		  if (r->son[RIGHT] == a)
		    r->son[RIGHT] = b;
		}
	      if (racine == a)
		racine = b;
	      a->balance = 0;
	      b->balance = 0;
	      return avl_no_err;
	    }
	  else
	    {
	      if (son[RIGHT]->son[LEFT] == NULL)
		{
//                                      cout << "mierda\n";
		  return avl_bal_err;
		}
	      AVLTree *f = son[RIGHT]->son[LEFT]->son[RIGHT];
	      AVLTree *g = son[RIGHT]->son[LEFT]->son[LEFT];

	      a->dad = d;
	      d->son[LEFT] = a;
	      b->dad = d;
	      d->son[RIGHT] = b;
	      a->son[RIGHT] = g;
	      if (g)
		g->dad = a;
	      a->son[LEFT] = e;
	      if (e)
		e->dad = a;
	      b->son[RIGHT] = c;
	      if (c)
		c->dad = b;
	      b->son[LEFT] = f;
	      if (f)
		f->dad = b;

	      d->dad = r;
	      if (r)
		{
		  if (r->son[LEFT] == a)
		    r->son[LEFT] = d;
		  if (r->son[RIGHT] == a)
		    r->son[RIGHT] = d;
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
	  if (dad)
	    {
	      if (this == dad->son[RIGHT])
		return dad->RestoreBalances (1, racine);
	      if (this == dad->son[LEFT])
		return dad->RestoreBalances (-1, racine);
	    }
	  return avl_no_err;
	}
      else if (diff == 0)
	{
	}
      else if (diff > 0)
	{
	  if (son[LEFT] == NULL)
	    {
//                              cout << "un probleme\n";
	      return avl_bal_err;
	    }
	  AVLTree *r = dad;
	  AVLTree *a = this;
	  AVLTree *b = son[RIGHT];
	  AVLTree *e = son[LEFT];
	  AVLTree *f = e->son[RIGHT];
	  AVLTree *g = e->son[LEFT];
	  if (e->balance > 0)
	    {
	      e->son[RIGHT] = a;
	      e->son[LEFT] = g;
	      a->son[RIGHT] = b;
	      a->son[LEFT] = f;
	      if (a)
		a->dad = e;
	      if (g)
		g->dad = e;
	      if (b)
		b->dad = a;
	      if (f)
		f->dad = a;
	      e->dad = r;
	      if (r)
		{
		  if (r->son[LEFT] == a)
		    r->son[LEFT] = e;
		  if (r->son[RIGHT] == a)
		    r->son[RIGHT] = e;
		}
	      if (racine == this)
		racine = e;
	      e->balance = 0;
	      a->balance = 0;
	      if (r)
		{
		  if (e == r->son[RIGHT])
		    return r->RestoreBalances (1, racine);
		  if (e == r->son[LEFT])
		    return r->RestoreBalances (-1, racine);
		}
	      return avl_no_err;
	    }
	  else if (e->balance == 0)
	    {
	      e->son[RIGHT] = a;
	      e->son[LEFT] = g;
	      a->son[RIGHT] = b;
	      a->son[LEFT] = f;
	      if (a)
		a->dad = e;
	      if (g)
		g->dad = e;
	      if (b)
		b->dad = a;
	      if (f)
		f->dad = a;
	      e->dad = r;
	      if (r)
		{
		  if (r->son[LEFT] == a)
		    r->son[LEFT] = e;
		  if (r->son[RIGHT] == a)
		    r->son[RIGHT] = e;
		}
	      if (racine == this)
		racine = e;
	      e->balance = -1;
	      a->balance = 1;
	      return avl_no_err;
	    }
	  else if (e->balance < 0)
	    {
	      if (son[LEFT]->son[RIGHT] == NULL)
		{
//                                      cout << "un probleme\n";
		  return avl_bal_err;
		}
	      AVLTree *i = son[LEFT]->son[RIGHT]->son[RIGHT];
	      AVLTree *j = son[LEFT]->son[RIGHT]->son[LEFT];

	      f->son[RIGHT] = a;
	      f->son[LEFT] = e;
	      a->son[RIGHT] = b;
	      a->son[LEFT] = i;
	      e->son[RIGHT] = j;
	      e->son[LEFT] = g;
	      if (b)
		b->dad = a;
	      if (i)
		i->dad = a;
	      if (g)
		g->dad = e;
	      if (j)
		j->dad = e;
	      if (a)
		a->dad = f;
	      if (e)
		e->dad = f;
	      f->dad = r;
	      if (r)
		{
		  if (r->son[LEFT] == a)
		    r->son[LEFT] = f;
		  if (r->son[RIGHT] == a)
		    r->son[RIGHT] = f;
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
		  if (f == r->son[RIGHT])
		    return r->RestoreBalances (1, racine);
		  if (f == r->son[LEFT])
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
	  if (son[RIGHT] == NULL)
	    {
//                              cout << "un probleme\n";
	      return avl_bal_err;
	    }
	  AVLTree *r = dad;
	  AVLTree *a = this;
	  AVLTree *b = son[LEFT];
	  AVLTree *e = son[RIGHT];
	  AVLTree *f = e->son[LEFT];
	  AVLTree *g = e->son[RIGHT];
	  if (e->balance < 0)
	    {
	      e->son[LEFT] = a;
	      e->son[RIGHT] = g;
	      a->son[LEFT] = b;
	      a->son[RIGHT] = f;
	      if (a)
		a->dad = e;
	      if (g)
		g->dad = e;
	      if (b)
		b->dad = a;
	      if (f)
		f->dad = a;
	      e->dad = r;
	      if (r)
		{
		  if (r->son[LEFT] == a)
		    r->son[LEFT] = e;
		  if (r->son[RIGHT] == a)
		    r->son[RIGHT] = e;
		}
	      if (racine == this)
		racine = e;
	      e->balance = 0;
	      a->balance = 0;
	      if (r)
		{
		  if (e == r->son[RIGHT])
		    return r->RestoreBalances (1, racine);
		  if (e == r->son[LEFT])
		    return r->RestoreBalances (-1, racine);
		}
	      return avl_no_err;
	    }
	  else if (e->balance == 0)
	    {
	      e->son[LEFT] = a;
	      e->son[RIGHT] = g;
	      a->son[LEFT] = b;
	      a->son[RIGHT] = f;
	      if (a)
		a->dad = e;
	      if (g)
		g->dad = e;
	      if (b)
		b->dad = a;
	      if (f)
		f->dad = a;
	      e->dad = r;
	      if (r)
		{
		  if (r->son[LEFT] == a)
		    r->son[LEFT] = e;
		  if (r->son[RIGHT] == a)
		    r->son[RIGHT] = e;
		}
	      if (racine == this)
		racine = e;
	      e->balance = 1;
	      a->balance = -1;
	      return avl_no_err;
	    }
	  else if (e->balance > 0)
	    {
	      if (son[RIGHT]->son[LEFT] == NULL)
		{
//                                      cout << "un probleme\n";
		  return avl_bal_err;
		}
	      AVLTree *i = son[RIGHT]->son[LEFT]->son[LEFT];
	      AVLTree *j = son[RIGHT]->son[LEFT]->son[RIGHT];

	      f->son[LEFT] = a;
	      f->son[RIGHT] = e;
	      a->son[LEFT] = b;
	      a->son[RIGHT] = i;
	      e->son[LEFT] = j;
	      e->son[RIGHT] = g;
	      if (b)
		b->dad = a;
	      if (i)
		i->dad = a;
	      if (g)
		g->dad = e;
	      if (j)
		j->dad = e;
	      if (a)
		a->dad = f;
	      if (e)
		e->dad = f;
	      f->dad = r;
	      if (r)
		{
		  if (r->son[LEFT] == a)
		    r->son[LEFT] = f;
		  if (r->son[RIGHT] == a)
		    r->son[RIGHT] = f;
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
		  if (f == r->son[RIGHT])
		    return r->RestoreBalances (1, racine);
		  if (f == r->son[LEFT])
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
	  if (dad)
	    {
	      if (this == dad->son[RIGHT])
		return dad->RestoreBalances (1, racine);
	      if (this == dad->son[LEFT])
		return dad->RestoreBalances (-1, racine);
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

  if (son[LEFT] && son[RIGHT])
    {
      AVLTree *newMe = son[LEFT]->leafFromDad(this, RIGHT);
      if (newMe == NULL || newMe->son[RIGHT])
	{
//                      cout << "pas normal\n";
	  return avl_rm_err;
	}
      if (newMe == son[LEFT])
	{
	  startNode = newMe;
	  diff = -1;
	  newMe->son[RIGHT] = son[RIGHT];
	  son[RIGHT]->dad = newMe;
	  newMe->dad = dad;
	  if (dad)
	    {
	      if (dad->son[LEFT] == this)
		dad->son[LEFT] = newMe;
	      if (dad->son[RIGHT] == this)
		dad->son[RIGHT] = newMe;
	    }
	}
      else
	{
	  AVLTree *oDad = newMe->dad;
	  startNode = oDad;
	  diff = 1;

	  oDad->son[RIGHT] = newMe->son[LEFT];
	  if (newMe->son[LEFT])
	    newMe->son[LEFT]->dad = oDad;

	  newMe->dad = dad;
	  newMe->son[LEFT] = son[LEFT];
	  newMe->son[RIGHT] = son[RIGHT];
	  if (dad)
	    {
	      if (dad->son[LEFT] == this)
		dad->son[LEFT] = newMe;
	      if (dad->son[RIGHT] == this)
		dad->son[RIGHT] = newMe;
	    }
	  if (son[LEFT])
	    son[LEFT]->dad = newMe;
	  if (son[RIGHT])
	    son[RIGHT]->dad = newMe;
	}
      newMe->balance = balance;
      if (racine == this)
	racine = newMe;
    }
  else if (son[LEFT])
    {
      startNode = dad;
      diff = 0;
      if (dad)
	{
	  if (this == dad->son[LEFT])
	    diff = -1;
	  if (this == dad->son[RIGHT])
	    diff = 1;
	}
      if (dad)
	{
	  if (dad->son[LEFT] == this)
	    dad->son[LEFT] = son[LEFT];
	  if (dad->son[RIGHT] == this)
	    dad->son[RIGHT] = son[LEFT];
	}
      if (son[LEFT]->dad == this)
	son[LEFT]->dad = dad;
      if (racine == this)
	racine = son[LEFT];
    }
  else if (son[RIGHT])
    {
      startNode = dad;
      diff = 0;
      if (dad)
	{
	  if (this == dad->son[LEFT])
	    diff = -1;
	  if (this == dad->son[RIGHT])
	    diff = 1;
	}
      if (dad)
	{
	  if (dad->son[LEFT] == this)
	    dad->son[LEFT] = son[RIGHT];
	  if (dad->son[RIGHT] == this)
	    dad->son[RIGHT] = son[RIGHT];
	}
      if (son[RIGHT]->dad == this)
	son[RIGHT]->dad = dad;
      if (racine == this)
	racine = son[RIGHT];
    }
  else
    {
      startNode = dad;
      diff = 0;
      if (dad)
	{
	  if (this == dad->son[LEFT])
	    diff = -1;
	  if (this == dad->son[RIGHT])
	    diff = 1;
	}
      if (dad)
	{
	  if (dad->son[LEFT] == this)
	    dad->son[LEFT] = NULL;
	  if (dad->son[RIGHT] == this)
	    dad->son[RIGHT] = NULL;
	}
      if (racine == this)
	racine = NULL;
    }
  dad = son[RIGHT] = son[LEFT] = NULL;
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
	  if (insertR == NULL || insertR->son[LEFT])
	    {
//                              cout << "ngou?\n";
	      return avl_ins_err;
	    }
	  insertR->son[LEFT] = this;
	  dad = insertR;
	  insertOn(LEFT, insertR);
	}
      else if (insertType == found_on_right)
	{
	  if (insertL == NULL || insertL->son[RIGHT])
	    {
//                              cout << "ngou?\n";
	      return avl_ins_err;
	    }
	  insertL->son[RIGHT] = this;
	  dad = insertL;
	  insertOn(RIGHT, insertL);
	}
      else if (insertType == found_between)
	{
	  if (insertR == NULL || insertL == NULL
	      || (insertR->son[LEFT] != NULL && insertL->son[RIGHT] != NULL))
	    {
//                              cout << "ngou?\n";
	      return avl_ins_err;
	    }
	  if (insertR->son[LEFT] == NULL)
	    {
	      insertR->son[LEFT] = this;
	      dad = insertR;
	    }
	  else if (insertL->son[RIGHT] == NULL)
	    {
	      insertL->son[RIGHT] = this;
	      dad = insertL;
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

	  if (insertL->son[RIGHT])
	    {
		insertL = insertL->son[RIGHT]->leafFromDad(insertL, LEFT);
	      if (insertL->son[LEFT])
		{
//                                      cout << "ngou?\n";
		  return avl_ins_err;
		}
	      insertL->son[LEFT] = this;
	      this->dad = insertL;
	      insertBetween (insertL->elem[LEFT], insertL);
	    }
	  else
	    {
	      insertL->son[RIGHT] = this;
	      dad = insertL;
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
    
  if (dad)
    {
      if (dad->son[LEFT] == this)
	dad->son[LEFT] = to;
      if (dad->son[RIGHT] == this)
	dad->son[RIGHT] = to;
    }
  if (son[RIGHT])
    {
      son[RIGHT]->dad = to;
    }
  if (son[LEFT])
    {
      son[LEFT]->dad = to;
    }
  to->dad = dad;
  to->son[RIGHT] = son[RIGHT];
  to->son[LEFT] = son[LEFT];
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
