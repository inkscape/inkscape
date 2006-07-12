/**
 * \brief Pairing heap datastructure implementation
 *
 * Based on example code in "Data structures and Algorithm Analysis in C++"
 * by Mark Allen Weiss, used and released under the LGPL by permission
 * of the author.
 *
 * No promises about correctness.  Use at your own risk!
 *
 * Authors:
 *   Mark Allen Weiss
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */
#ifndef PAIRING_HEAP_H_
#define PAIRING_HEAP_H_
#include <stdlib.h>
#include <fstream>
// Pairing heap class
//
// CONSTRUCTION: with no parameters
//
// ******************PUBLIC OPERATIONS*********************
// PairNode & insert( x ) --> Insert x
// deleteMin( minItem )   --> Remove (and optionally return) smallest item
// T findMin( )  --> Return smallest item
// bool isEmpty( )        --> Return true if empty; else false
// bool isFull( )         --> Return true if empty; else false
// void makeEmpty( )      --> Remove all items
// void decreaseKey( PairNode p, newVal )
//                        --> Decrease value in node p
// ******************ERRORS********************************
// Throws Underflow as warranted


// Node and forward declaration because g++ does
// not understand nested classes.
template <class T> 
class PairingHeap;

template <class T>
std::ostream& operator<< (std::ostream &os,const PairingHeap<T> &b);

template <class T>
class PairNode
{
	friend std::ostream& operator<< <T>(std::ostream &os,const PairingHeap<T> &b);
	T   element;
	PairNode    *leftChild;
	PairNode    *nextSibling;
	PairNode    *prev;

	PairNode( const T & theElement ) :
	       	element( theElement ),
		leftChild(NULL), nextSibling(NULL), prev(NULL)
       	{ }
	friend class PairingHeap<T>;
};

template <class T>
class Comparator
{
public:
	virtual bool isLessThan(T const &lhs, T const &rhs) const = 0;
};

template <class T>
class PairingHeap
{
	friend std::ostream& operator<< <T>(std::ostream &os,const PairingHeap<T> &b);
public:
	PairingHeap( bool (*lessThan)(T const &lhs, T const &rhs) );
	PairingHeap( const PairingHeap & rhs );
	~PairingHeap( );

	bool isEmpty( ) const;
	bool isFull( ) const;
	int size();

	PairNode<T> *insert( const T & x );
	const T & findMin( ) const;
	void deleteMin( );
	const T extractMin( ) {
		T v = findMin();
		deleteMin();
		return v;
	}
	void makeEmpty( );
	void decreaseKey( PairNode<T> *p, const T & newVal );
	void merge( PairingHeap<T> *rhs )
	{	
		PairNode<T> *broot=rhs->getRoot();
		if (root == NULL) {
			if(broot != NULL) {
				root = broot;
			}
		} else {
			compareAndLink(root, broot);
		}
		counter+=rhs->size();
	}

	const PairingHeap & operator=( const PairingHeap & rhs );
protected:
	PairNode<T> * getRoot() {
		PairNode<T> *r=root;
		root=NULL;
		return r;
	}
private:
	PairNode<T> *root;
	bool (*lessThan)(T const &lhs, T const &rhs);
	int counter;
	void reclaimMemory( PairNode<T> *t ) const;
	void compareAndLink( PairNode<T> * & first, PairNode<T> *second ) const;
	PairNode<T> * combineSiblings( PairNode<T> *firstSibling ) const;
	PairNode<T> * clone( PairNode<T> * t ) const;
};

#include "PairingHeap.cpp"
#endif
