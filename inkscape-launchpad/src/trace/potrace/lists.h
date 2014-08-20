/* Copyright (C) 2001-2011 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */


#ifndef _PS_LISTS_H
#define _PS_LISTS_H

/* here we define some general list macros. Because they are macros,
   they should work on any datatype with a "->next" component. Some of
   them use a "hook". If elt and list are of type t* then hook is of
   type t**. A hook stands for an insertion point in the list, i.e.,
   either before the first element, or between two elements, or after
   the last element. If an operation "sets the hook" for an element,
   then the hook is set to just before the element. One can insert
   something at a hook. One can also unlink at a hook: this means,
   unlink the element just after the hook. By "to unlink", we mean the
   element is removed from the list, but not deleted. Thus, it and its
   components still need to be freed. */

/* Note: these macros are somewhat experimental. Only the ones that
   are actually *used* have been tested. So be careful to test any
   that you use. Looking at the output of the preprocessor, "gcc -E"
   (possibly piped though "indent"), might help too. Also: these
   macros define some internal (local) variables that start with
   "_". */

/* we enclose macro definitions whose body consists of more than one
   statement in MACRO_BEGIN and MACRO_END, rather than '{' and '}'.  The
   reason is that we want to be able to use the macro in a context
   such as "if (...) macro(...); else ...". If we didn't use this obscure
   trick, we'd have to omit the ";" in such cases. */

#define MACRO_BEGIN do {
#define MACRO_END   } while (0)

/* ---------------------------------------------------------------------- */
/* macros for singly-linked lists */

/* traverse list. At the end, elt is set to NULL. */
#define list_forall(elt, list)   for (elt=list; elt!=NULL; elt=elt->next)

/* set elt to the first element of list satisfying boolean condition
   c, or NULL if not found */
#define list_find(elt, list, c) \
  MACRO_BEGIN list_forall(elt, list) if (c) break; MACRO_END

/* like forall, except also set hook for elt. */
#define list_forall2(elt, list, hook) \
  for (elt=list, hook=&list; elt!=NULL; hook=&elt->next, elt=elt->next)

/* same as list_find, except also set hook for elt. */
#define list_find2(elt, list, c, hook) \
  MACRO_BEGIN list_forall2(elt, list, hook) if (c) break; MACRO_END

/* same, except only use hook. */
#define _list_forall_hook(list, hook) \
  for (hook=&list; *hook!=NULL; hook=&(*hook)->next)

/* same, except only use hook. Note: c may only refer to *hook, not elt. */
#define _list_find_hook(list, c, hook) \
  MACRO_BEGIN _list_forall_hook(list, hook) if (c) break; MACRO_END

/* insert element after hook */
#define list_insert_athook(elt, hook) \
  MACRO_BEGIN elt->next = *hook; *hook = elt; MACRO_END

/* insert element before hook */
#define list_insert_beforehook(elt, hook) \
  MACRO_BEGIN elt->next = *hook; *hook = elt; hook=&elt->next; MACRO_END

/* unlink element after hook, let elt be unlinked element, or NULL.
   hook remains. */
#define list_unlink_athook(list, elt, hook) \
  MACRO_BEGIN \
  elt = hook ? *hook : NULL; if (elt) { *hook = elt->next; elt->next = NULL; }\
  MACRO_END

/* unlink the specific element, if it is in the list. Otherwise, set
   elt to NULL */
#define list_unlink(listtype, list, elt)      \
  MACRO_BEGIN  	       	       	       	      \
  listtype **_hook;			      \
  _list_find_hook(list, *_hook==elt, _hook);  \
  list_unlink_athook(list, elt, _hook);	      \
  MACRO_END

/* prepend elt to list */
#define list_prepend(list, elt) \
  MACRO_BEGIN elt->next = list; list = elt; MACRO_END

/* append elt to list. */
#define list_append(listtype, list, elt)     \
  MACRO_BEGIN                                \
  listtype **_hook;                          \
  _list_forall_hook(list, _hook) {}          \
  list_insert_athook(elt, _hook);            \
  MACRO_END

/* unlink the first element that satisfies the condition. */
#define list_unlink_cond(listtype, list, elt, c)     \
  MACRO_BEGIN                                        \
  listtype **_hook;			  	     \
  list_find2(elt, list, c, _hook);                   \
  list_unlink_athook(list, elt, _hook);              \
  MACRO_END

/* let elt be the nth element of the list, starting to count from 0.
   Return NULL if out of bounds.   */
#define list_nth(elt, list, n)                                \
  MACRO_BEGIN                                                 \
  int _x;  /* only evaluate n once */                         \
  for (_x=(n), elt=list; _x && elt; _x--, elt=elt->next) {}   \
  MACRO_END

/* let elt be the nth element of the list, starting to count from 0.
   Return NULL if out of bounds.   */
#define list_nth_hook(elt, list, n, hook)                     \
  MACRO_BEGIN                                                 \
  int _x;  /* only evaluate n once */                         \
  for (_x=(n), elt=list, hook=&list; _x && elt; _x--, hook=&elt->next, elt=elt->next) {}   \
  MACRO_END

/* set n to the length of the list */
#define list_length(listtype, list, n)                   \
  MACRO_BEGIN          	       	       	       	       	 \
  listtype *_elt;   			 		 \
  n=0;					 		 \
  list_forall(_elt, list) 		 		 \
    n++;				 		 \
  MACRO_END

/* set n to the index of the first element satisfying cond, or -1 if
   none found. Also set elt to the element, or NULL if none found. */
#define list_index(list, n, elt, c)                      \
  MACRO_BEGIN				 		 \
  n=0;					 		 \
  list_forall(elt, list) {		 		 \
    if (c) break;			 		 \
    n++;				 		 \
  }					 		 \
  if (!elt)				 		 \
    n=-1;				 		 \
  MACRO_END

/* set n to the number of elements in the list that satisfy condition c */
#define list_count(list, n, elt, c)                      \
  MACRO_BEGIN				 		 \
  n=0;					 		 \
  list_forall(elt, list) {		 		 \
    if (c) n++;				 		 \
  }                                                      \
  MACRO_END

/* let elt be each element of the list, unlinked. At the end, set list=NULL. */
#define list_forall_unlink(elt, list) \
  for (elt=list; elt ? (list=elt->next, elt->next=NULL), 1 : 0; elt=list)

/* reverse a list (efficient) */
#define list_reverse(listtype, list)            \
  MACRO_BEGIN				 	\
  listtype *_list1=NULL, *elt;			\
  list_forall_unlink(elt, list) 		\
    list_prepend(_list1, elt);			\
  list = _list1;				\
  MACRO_END

/* insert the element ELT just before the first element TMP of the
   list for which COND holds. Here COND must be a condition of ELT and
   TMP.  Typical usage is to insert an element into an ordered list:
   for instance, list_insert_ordered(listtype, list, elt, tmp,
   elt->size <= tmp->size).  Note: if we give a "less than or equal"
   condition, the new element will be inserted just before a sequence
   of equal elements. If we give a "less than" condition, the new
   element will be inserted just after a list of equal elements.
   Note: it is much more efficient to construct a list with
   list_prepend and then order it with list_merge_sort, than to
   construct it with list_insert_ordered. */
#define list_insert_ordered(listtype, list, elt, tmp, cond) \
  MACRO_BEGIN                                               \
  listtype **_hook;                                         \
  _list_find_hook(list, (tmp=*_hook, (cond)), _hook);       \
  list_insert_athook(elt, _hook);                           \
  MACRO_END

/* sort the given list, according to the comparison condition.
   Typical usage is list_sort(listtype, list, a, b, a->size <
   b->size).  Note: if we give "less than or equal" condition, each
   segment of equal elements will be reversed in order. If we give a
   "less than" condition, each segment of equal elements will retain
   the original order. The latter is slower but sometimes
   prettier. Average running time: n*n/2. */
#define list_sort(listtype, list, a, b, cond)            \
  MACRO_BEGIN                                            \
  listtype *_newlist=NULL;                               \
  list_forall_unlink(a, list)                            \
    list_insert_ordered(listtype, _newlist, a, b, cond); \
  list = _newlist;                                       \
  MACRO_END

/* a much faster sort algorithm (merge sort, n log n worst case). It
   is required that the list type has an additional, unused next1
   component. Note there is no curious reversal of order of equal
   elements as for list_sort. */

#define list_mergesort(listtype, list, a, b, cond)              \
  MACRO_BEGIN						        \
  listtype *_elt, **_hook1;				    	\
							    	\
  for (_elt=list; _elt; _elt=_elt->next1) {			\
    _elt->next1 = _elt->next;				    	\
    _elt->next = NULL;					    	\
  }							    	\
  do {			                               	    	\
    _hook1 = &(list);				    	    	\
    while ((a = *_hook1) != NULL && (b = a->next1) != NULL ) {  \
      _elt = b->next1;					    	\
      _list_merge_cond(listtype, a, b, cond, *_hook1);      	\
      _hook1 = &((*_hook1)->next1);			    	\
      *_hook1 = _elt;				            	\
    }							    	\
  } while (_hook1 != &(list));                                 	\
  MACRO_END

/* merge two sorted lists. Store result at &result */
#define _list_merge_cond(listtype, a, b, cond, result)   \
  MACRO_BEGIN                                            \
  listtype **_hook;					 \
  _hook = &(result);					 \
  while (1) {                                            \
     if (a==NULL) {					 \
       *_hook = b;					 \
       break;						 \
     } else if (b==NULL) {				 \
       *_hook = a;					 \
       break;						 \
     } else if (cond) {					 \
       *_hook = a;					 \
       _hook = &(a->next);				 \
       a = a->next;					 \
     } else {						 \
       *_hook = b;					 \
       _hook = &(b->next);				 \
       b = b->next;					 \
     }							 \
  }							 \
  MACRO_END

/* ---------------------------------------------------------------------- */
/* macros for doubly-linked lists */

#define dlist_append(head, end, elt)                    \
  MACRO_BEGIN  	       	       	       	       	       	 \
  elt->prev = end;					 \
  elt->next = NULL;					 \
  if (end) {						 \
    end->next = elt;					 \
  } else {  						 \
    head = elt;						 \
  }	    						 \
  end = elt;						 \
  MACRO_END

/* let elt be each element of the list, unlinked. At the end, set list=NULL. */
#define dlist_forall_unlink(elt, head, end) \
  for (elt=head; elt ? (head=elt->next, elt->next=NULL, elt->prev=NULL), 1 : (end=NULL, 0); elt=head)

/* unlink the first element of the list */
#define dlist_unlink_first(head, end, elt)               \
  MACRO_BEGIN				       	       	 \
  elt = head;						 \
  if (head) {						 \
    head = head->next;					 \
    if (head) {						 \
      head->prev = NULL;				 \
    } else {						 \
      end = NULL;					 \
    }    						 \
    elt->prev = NULL;					 \
    elt->next = NULL;					 \
  }							 \
  MACRO_END

#endif /* _PS_LISTS_H */

