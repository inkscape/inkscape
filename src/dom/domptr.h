#ifndef __DOMPTR_H__
#define __DOMPTR_H__
/**
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 * 
 * More thorough explanations of the various classes and their algorithms
 * can be found there.
 *     
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2006-2008 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *  
 * =======================================================================
 *  NOTES:
 * 
 *  Notice that many of the classes defined here are pure virtual.  In other
 *  words, they are purely unimplemented interfaces.  For the implementations
 *  of them, look in domimpl.h and domimpl.cpp.
 *  
 *  Also, note that there is a domptr.cpp file that has a couple of necessary
 *  functions which cannot be in a .h file
 *             
 */

#include <functional>

namespace org
{
namespace w3c
{
namespace dom
{



/*#########################################################################
## NodePtr
#########################################################################*/

/**
 * A simple Smart Pointer class that handles Nodes and all of its
 * descendants.  This is very similar to shared_ptr, but it is customized
 * to handle our needs. 
 */ 
template<class T> class Ptr
{
public:

    /**
     * Simple constructor
     */ 
    Ptr()
        { _ref = 0; }

    /**
     * Constructor upon a reference
     */ 
    template<class Y> Ptr(const Ptr<Y> &other)
        {
        _ref = other._ref;
	    incrementRefCount(_ref);
        }

    /**
     * Constructor upon a reference
     */ 
    Ptr(T * refArg, bool addRef = true)
        {
        _ref = refArg;
        if(addRef)
		    incrementRefCount(_ref);
        }


    /**
     * Copy constructor
     */ 
    Ptr(const Ptr &other)
        {
        _ref = other._ref;
	    incrementRefCount(_ref);
        }

    /**
     * Destructor
     */ 
    virtual ~Ptr()
    {
        decrementRefCount(_ref);
    }


    /**
     * Assignment operator
     */ 
    template<class Y> Ptr &operator=(const Ptr<Y> &other)
        {
        decrementRefCount(_ref);
        _ref = other._ref;
        incrementRefCount(_ref);
        return *this;
        }

    /**
     * Assignment operator
     */ 
    Ptr &operator=(const Ptr &other)
        {
        decrementRefCount(_ref);
        _ref = other._ref;
        incrementRefCount(_ref);
        return *this;
        }

    /**
     * Assignment operator
     */ 
    template<class Y> Ptr &operator=(Y * ref)
        {
        decrementRefCount(_ref);
        _ref = ref;
        incrementRefCount(_ref);
        return *this;
        }

    /**
     * Assignment operator
     */ 
    template<class Y> Ptr &operator=(const Y * ref)
        {
        decrementRefCount(_ref);
        _ref = (Y *) ref;
        incrementRefCount(_ref);
        return *this;
        }

    /**
     * Return the reference
     */ 
    T * get() const
        {
        return _ref;
        }

    /**
     * Dereference operator
     */ 
    T &operator*() const
        {
        return *_ref;
        }

    /**
     * Point-to operator
     */ 
    T *operator->() const
        {
        return _ref;
        }

    /**
     * NOT bool operator.  How to check if we are null without a comparison
     */	     
    bool operator! () const
        {
        return (_ref == 0);
        }

    /**
     * Swap what I reference with the other guy
     */	     
    void swap(Ptr &other)
        {
        T *tmp = _ref;
        _ref = other._ref;
        other._ref = tmp;
        }

    //The referenced item
    T *_ref;
};


/**
 * Global definitions.  Many of these are used to mimic behaviour of
 * a real pointer 
 */

/**
 * Equality
 */ 
template<class T, class U> inline bool
   operator==(const Ptr<T> &a, const Ptr<U> &b)
{
    return a.get() == b.get();
}

/**
 * Inequality
 */ 
template<class T, class U> inline bool
     operator!=(const Ptr<T> &a, const Ptr<U> &b)
{
    return a.get() != b.get();
}

/**
 * Equality
 */ 
template<class T> inline bool
     operator==(const Ptr<T> &a, T * b)
{
    return a.get() == b;
}

/**
 * Inequality
 */ 
template<class T> inline bool
     operator!=(const Ptr<T> &a, T * b)
{
    return a.get() != b;
}

/**
 * Equality
 */ 
template<class T> inline bool
     operator==(T * a, const Ptr<T> &b)
{
    return a == b.get();
}

/**
 * Inequality
 */ 
template<class T> inline bool
     operator!=(T * a, const Ptr<T> &b)
{
    return a != b.get();
}


/**
 * Less than
 */ 
template<class T> inline bool
     operator<(const Ptr<T> &a, const Ptr<T> &b)
{
    return std::less<T *>()(a.get(), b.get());
}

/**
 * Swap
 */ 
template<class T> void
     swap(Ptr<T> &a, Ptr<T> &b)
{
    a.swap(b);
}


/**
 * Get the pointer globally, for <algo>
 */ 
template<class T> T * 
    get_pointer(const Ptr<T> &p)
{
    return p.get();
}

/**
 * Static cast
 */ 
template<class T, class U> Ptr<T>
     static_pointer_cast(const Ptr<U> &p)
{
    return static_cast<T *>(p.get());
}

/**
 * Const cast
 */ 
template<class T, class U> Ptr<T>
     const_pointer_cast(const Ptr<U> &p)
{
    return const_cast<T *>(p.get());
}

/**
 * Dynamic cast
 */ 
template<class T, class U> Ptr<T>
     dynamic_pointer_cast(const Ptr<U> &p)
{
    return dynamic_cast<T *>(p.get());
}



}  //namespace dom
}  //namespace w3c
}  //namespace org


#endif // __DOMPTR_H__


/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/




