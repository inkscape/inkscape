/*
 * Vector, VectorView, ConstVectorView classes wrap the gsl vector routines;
 * "views" mimic the semantic of C++ references: any operation performed
 * on a "view" is actually performed on the "viewed object"
 *
 * Authors:
 * 		Marco Cecchetti <mrcekets at gmail.com>
 *
 * Copyright 2008  authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */




#ifndef _NL_VECTOR_H_
#define _NL_VECTOR_H_

#include <cassert>
#include <algorithm> // for std::swap
#include <vector>
#include <sstream>
#include <string>


#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>


namespace Geom { namespace NL {

namespace detail
{

class BaseVectorImpl
{
  public:
	double const& operator[](size_t i) const
	{
		return *gsl_vector_const_ptr(m_vector, i);
	}

	const gsl_vector* get_gsl_vector() const
	{
		return m_vector;
	}
	bool is_zero() const
	{
		return gsl_vector_isnull(m_vector);
	}

	bool is_positive() const
	{
	    for ( size_t i = 0; i < size(); ++i )
	    {
	        if ( (*this)[i] <= 0 ) return false;
	    }
		return true;
	}

	bool is_negative() const
	{
        for ( size_t i = 0; i < size(); ++i )
        {
            if ( (*this)[i] >= 0 ) return false;
        }
        return true;
	}

	bool is_non_negative() const
	{
		for ( size_t i = 0; i < size(); ++i )
		{
			if ( (*this)[i] < 0 ) return false;
		}
		return true;
	}

	double max() const
	{
		return gsl_vector_max(m_vector);
	}

	double min() const
	{
		return gsl_vector_min(m_vector);
	}

	size_t max_index() const
	{
		return gsl_vector_max_index(m_vector);
	}

	size_t min_index() const
	{
		return gsl_vector_min_index(m_vector);
	}

	size_t size() const
	{
		return m_size;
	}

	std::string str() const;

	virtual ~BaseVectorImpl()
	{
	}

  protected:
	size_t m_size;
	gsl_vector* m_vector;

};  // end class BaseVectorImpl


inline
bool operator== (BaseVectorImpl const& v1, BaseVectorImpl const& v2)
{
	if (v1.size() != v2.size())	return false;

	for (size_t i = 0; i < v1.size(); ++i)
	{
		if (v1[i] != v2[i])  return false;
	}
	return true;
}

template< class charT >
inline
std::basic_ostream<charT> &
operator<< (std::basic_ostream<charT> & os, const BaseVectorImpl & _vector)
{
	if (_vector.size() == 0 ) return os;
	os << "[" << _vector[0];
	for (unsigned int i = 1; i < _vector.size(); ++i)
	{
		os << ", " << _vector[i];
	}
	os << "]";
	return os;
}

inline
std::string BaseVectorImpl::str() const
{
	std::ostringstream oss;
	oss << (*this);
	return oss.str();
}

inline
double dot(BaseVectorImpl const& v1, BaseVectorImpl const& v2)
{
    double result;
    gsl_blas_ddot(v1.get_gsl_vector(), v2.get_gsl_vector(), &result);
    return result;
}


class VectorImpl : public BaseVectorImpl
{
  public:
	typedef BaseVectorImpl base_type;

  public:
	void set_all(double x)
	{
		gsl_vector_set_all(m_vector, x);
	}

	void set_basis(size_t i)
	{
		gsl_vector_set_basis(m_vector, i);
	}

	using base_type::operator[];

	double & operator[](size_t i)
	{
		return *gsl_vector_ptr(m_vector, i);
	}

	using base_type::get_gsl_vector;

	gsl_vector* get_gsl_vector()
	{
		return m_vector;
	}

	void swap_elements(size_t i, size_t j)
	{
		gsl_vector_swap_elements(m_vector, i, j);
	}

	void reverse()
	{
		gsl_vector_reverse(m_vector);
	}

	VectorImpl & scale(double x)
	{
		gsl_vector_scale(m_vector, x);
		return (*this);
	}

	VectorImpl & translate(double x)
	{
		gsl_vector_add_constant(m_vector, x);
		return (*this);
	}

	VectorImpl & operator+=(base_type const& _vector)
	{
		gsl_vector_add(m_vector, _vector.get_gsl_vector());
		return (*this);
	}

	VectorImpl & operator-=(base_type const& _vector)
	{
		gsl_vector_sub(m_vector, _vector.get_gsl_vector());
		return (*this);
	}

};  // end class VectorImpl

}  // end namespace detail


using detail::operator==;
using detail::operator<<;

class Vector : public detail::VectorImpl
{
  public:
	typedef detail::VectorImpl base_type;

  public:
	Vector(size_t n)
	{
		m_size = n;
		m_vector = gsl_vector_alloc(n);
	}

	Vector(size_t n, double x)
	{
		m_size = n;
		m_vector = gsl_vector_alloc(n);
		gsl_vector_set_all(m_vector, x);
	}

	// create a vector with n elements all set to zero
	// but the i-th that is set to 1
	Vector(size_t n, size_t i)
	{
		m_size = n;
		m_vector = gsl_vector_alloc(n);
		gsl_vector_set_basis(m_vector, i);
	}

	Vector(Vector const& _vector)
        : base_type()
	{
		m_size = _vector.size();
		m_vector = gsl_vector_alloc(size());
		gsl_vector_memcpy(m_vector, _vector.m_vector);
	}

	explicit
	Vector(base_type::base_type const& _vector)
	{
		m_size = _vector.size();
		m_vector = gsl_vector_alloc(size());
		gsl_vector_memcpy(m_vector, _vector.get_gsl_vector());
	}

	virtual ~Vector()
	{
		gsl_vector_free(m_vector);
	}


	Vector & operator=(Vector const& _vector)
	{
		assert( size() == _vector.size() );
		gsl_vector_memcpy(m_vector, _vector.m_vector);
		return (*this);
	}

	Vector & operator=(base_type::base_type const& _vector)
	{
		assert( size() == _vector.size() );
		gsl_vector_memcpy(m_vector, _vector.get_gsl_vector());
		return (*this);
	}

	Vector & scale(double x)
	{
		return static_cast<Vector&>( base_type::scale(x) );
	}

	Vector & translate(double x)
	{
		return static_cast<Vector&>( base_type::translate(x) );
	}

	Vector & operator+=(base_type::base_type const& _vector)
	{
		return static_cast<Vector&>( base_type::operator+=(_vector) );
	}

	Vector & operator-=(base_type::base_type const& _vector)
	{
		return static_cast<Vector&>( base_type::operator-=(_vector) );
	}

	friend
	void swap(Vector & v1, Vector & v2);
	friend
	void swap_any(Vector & v1, Vector & v2);

}; // end class Vector


// warning! these operations invalidate any view of the passed vector objects
inline
void swap(Vector & v1, Vector & v2)
{
    assert(v1.size() == v2.size());
    using std::swap;
    swap(v1.m_vector, v2.m_vector);
}

inline
void swap_any(Vector & v1, Vector & v2)
{
	using std::swap;
    swap(v1.m_vector, v2.m_vector);
    swap(v1.m_size, v2.m_size);
}


class ConstVectorView : public detail::BaseVectorImpl
{
  public:
	typedef detail::BaseVectorImpl base_type;

  public:
	ConstVectorView(const base_type & _vector, size_t n, size_t offset = 0)
		: m_vector_view( gsl_vector_const_subvector(_vector.get_gsl_vector(), offset, n) )
	{
		m_size = n;
		m_vector = const_cast<gsl_vector*>( &(m_vector_view.vector) );
	}

	ConstVectorView(const base_type & _vector, size_t n, size_t offset , size_t stride)
		: m_vector_view( gsl_vector_const_subvector_with_stride(_vector.get_gsl_vector(), offset, stride, n) )
	{
		m_size = n;
		m_vector = const_cast<gsl_vector*>( &(m_vector_view.vector) );
	}

    ConstVectorView(const double* _vector, size_t n, size_t offset = 0)
        : m_vector_view( gsl_vector_const_view_array(_vector + offset, n) )
    {
        m_size = n;
        m_vector = const_cast<gsl_vector*>( &(m_vector_view.vector) );
    }

    ConstVectorView(const double* _vector, size_t n, size_t offset, size_t stride)
        : m_vector_view( gsl_vector_const_view_array_with_stride(_vector + offset, stride, n) )
    {
        m_size = n;
        m_vector = const_cast<gsl_vector*>( &(m_vector_view.vector) );
    }

	explicit
	ConstVectorView(gsl_vector_const_view  _gsl_vector_view)
		: m_vector_view(_gsl_vector_view)
	{
		m_vector = const_cast<gsl_vector*>( &(m_vector_view.vector) );
		m_size = m_vector->size;
	}

    explicit
    ConstVectorView(const std::vector<double>&  _vector)
        : m_vector_view( gsl_vector_const_view_array(&(_vector[0]), _vector.size()) )
    {
        m_vector = const_cast<gsl_vector*>( &(m_vector_view.vector) );
        m_size = _vector.size();
    }

	ConstVectorView(const ConstVectorView & _vector)
		: base_type(),
		  m_vector_view(_vector.m_vector_view)
	{
		m_size = _vector.size();
		m_vector = const_cast<gsl_vector*>( &(m_vector_view.vector) );
	}

	ConstVectorView(const base_type & _vector)
		: m_vector_view(gsl_vector_const_subvector(_vector.get_gsl_vector(), 0, _vector.size()))
	{
		m_size = _vector.size();
		m_vector = const_cast<gsl_vector*>( &(m_vector_view.vector) );
	}

  private:
	gsl_vector_const_view m_vector_view;

}; // end class  ConstVectorView




class VectorView : public detail::VectorImpl
{
  public:
	typedef detail::VectorImpl base_type;

  public:
	VectorView(base_type & _vector, size_t n, size_t offset = 0, size_t stride = 1)
	{
		m_size = n;
		if (stride == 1)
		{
			m_vector_view
				= gsl_vector_subvector(_vector.get_gsl_vector(), offset, n);
			m_vector = &(m_vector_view.vector);
		}
		else
		{
			m_vector_view
				= gsl_vector_subvector_with_stride(_vector.get_gsl_vector(), offset, stride, n);
			m_vector = &(m_vector_view.vector);
		}
	}

    VectorView(double* _vector, size_t n, size_t offset = 0, size_t stride = 1)
    {
        m_size = n;
        if (stride == 1)
        {
            m_vector_view
                = gsl_vector_view_array(_vector + offset, n);
            m_vector = &(m_vector_view.vector);
        }
        else
        {
            m_vector_view
                = gsl_vector_view_array_with_stride(_vector + offset, stride, n);
            m_vector = &(m_vector_view.vector);
        }

    }

	VectorView(const VectorView & _vector)
        : base_type()
	{
		m_size = _vector.size();
		m_vector_view = _vector.m_vector_view;
		m_vector = &(m_vector_view.vector);
	}

	VectorView(Vector & _vector)
	{
		m_size = _vector.size();
		m_vector_view = gsl_vector_subvector(_vector.get_gsl_vector(), 0, size());
		m_vector = &(m_vector_view.vector);
	}

	explicit
	VectorView(gsl_vector_view _gsl_vector_view)
		: m_vector_view(_gsl_vector_view)
	{
		m_vector = &(m_vector_view.vector);
		m_size = m_vector->size;
	}

	explicit
	VectorView(std::vector<double> & _vector)
	{
	    m_size = _vector.size();
	    m_vector_view = gsl_vector_view_array(&(_vector[0]), _vector.size());
	    m_vector = &(m_vector_view.vector);
	}

	VectorView & operator=(VectorView const& _vector)
	{
		assert( size() == _vector.size() );
		gsl_vector_memcpy(m_vector, _vector.get_gsl_vector());
		return (*this);
	}

	VectorView & operator=(base_type::base_type const& _vector)
	{
		assert( size() == _vector.size() );
		gsl_vector_memcpy(m_vector, _vector.get_gsl_vector());
		return (*this);
	}

	VectorView & scale(double x)
	{
		return static_cast<VectorView&>( base_type::scale(x) );
	}

	VectorView & translate(double x)
	{
		return static_cast<VectorView&>( base_type::translate(x) );
	}

	VectorView & operator+=(base_type::base_type const& _vector)
	{
		return static_cast<VectorView&>( base_type::operator+=(_vector) );
	}

	VectorView & operator-=(base_type::base_type const& _vector)
	{
		return static_cast<VectorView&>( base_type::operator-=(_vector) );
	}

	friend
	void swap_view(VectorView & v1, VectorView & v2);

  private:
	gsl_vector_view m_vector_view;

}; // end class VectorView


inline
void swap_view(VectorView & v1, VectorView & v2)
{
	assert( v1.size() == v2.size() );
	using std::swap;
	swap(v1.m_vector_view, v2.m_vector_view); // not swap m_vector too
}

inline
const VectorView & const_vector_view_cast (const ConstVectorView & view)
{
    const detail::BaseVectorImpl & bvi
        = static_cast<const detail::BaseVectorImpl &>(view);
    const VectorView & vv = reinterpret_cast<const VectorView &>(bvi);
    return vv;
}

inline
VectorView & const_vector_view_cast (ConstVectorView & view)
{
    detail::BaseVectorImpl & bvi
        = static_cast<detail::BaseVectorImpl &>(view);
    VectorView & vv = reinterpret_cast<VectorView &>(bvi);
    return vv;
}


} } // end namespaces


#endif /*_NL_VECTOR_H_*/

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
