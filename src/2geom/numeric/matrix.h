/*
 * Matrix, MatrixView, ConstMatrixView classes wrap the gsl matrix routines;
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




#ifndef _NL_MATRIX_H_
#define _NL_MATRIX_H_

#include <2geom/exception.h>
#include <2geom/numeric/vector.h>

#include <cassert>
#include <utility>    // for std::pair
#include <algorithm>  // for std::swap
#include <sstream>
#include <string>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>


namespace Geom { namespace NL {

namespace detail
{

class BaseMatrixImpl
{
  public:
	virtual ~BaseMatrixImpl()
	{
	}

	ConstVectorView row_const_view(size_t i) const
	{
		return ConstVectorView(gsl_matrix_const_row(m_matrix, i));
	}

	ConstVectorView column_const_view(size_t i) const
	{
		return ConstVectorView(gsl_matrix_const_column(m_matrix, i));
	}

	const double & operator() (size_t i, size_t j) const
	{
		return *gsl_matrix_const_ptr(m_matrix, i, j);
	}

	const gsl_matrix* get_gsl_matrix() const
	{
		return m_matrix;
	}

	bool is_zero() const
	{
		return gsl_matrix_isnull(m_matrix);
	}

	bool is_positive() const
	{
        for ( unsigned int i = 0; i < rows(); ++i )
        {
            for ( unsigned int j = 0; j < columns(); ++j )
            {
                if ( (*this)(i,j) <= 0 ) return false;
            }
        }
        return true;
	}

	bool is_negative() const
	{
        for ( unsigned int i = 0; i < rows(); ++i )
        {
            for ( unsigned int j = 0; j < columns(); ++j )
            {
                if ( (*this)(i,j) >= 0 ) return false;
            }
        }
        return true;
	}

	bool is_non_negative() const
	{
		for ( unsigned int i = 0; i < rows(); ++i )
		{
			for ( unsigned int j = 0; j < columns(); ++j )
			{
				if ( (*this)(i,j) < 0 ) return false;
			}
		}
		return true;
	}

	double max() const
	{
		return gsl_matrix_max(m_matrix);
	}

	double min() const
	{
		return gsl_matrix_min(m_matrix);
	}

	std::pair<size_t, size_t>
	max_index() const
	{
		std::pair<size_t, size_t> indices;
		gsl_matrix_max_index(m_matrix, &(indices.first), &(indices.second));
		return indices;
	}

	std::pair<size_t, size_t>
	min_index() const
	{
		std::pair<size_t, size_t> indices;
		gsl_matrix_min_index(m_matrix, &(indices.first), &(indices.second));
		return indices;
	}

	size_t rows() const
	{
		return m_rows;
	}

	size_t columns() const
	{
		return m_columns;
	}

	std::string str() const;

  protected:
	size_t m_rows, m_columns;
	gsl_matrix* m_matrix;

};  // end class BaseMatrixImpl


inline
bool operator== (BaseMatrixImpl const& m1, BaseMatrixImpl const& m2)
{
	if (m1.rows() != m2.rows() || m1.columns() != m2.columns()) return false;

	for (size_t i = 0; i < m1.rows(); ++i)
		for (size_t j = 0; j < m1.columns(); ++j)
			if (m1(i,j) != m2(i,j)) return false;

	return true;
}

template< class charT >
inline
std::basic_ostream<charT> &
operator<< (std::basic_ostream<charT> & os, const BaseMatrixImpl & _matrix)
{
	if (_matrix.rows() == 0 || _matrix.columns() == 0) return os;

	os << "[[" << _matrix(0,0);
	for (size_t j = 1; j < _matrix.columns(); ++j)
	{
		os << ", " << _matrix(0,j);
	}
	os << "]";

	for (size_t i = 1; i < _matrix.rows(); ++i)
	{
		os << ", [" << _matrix(i,0);
		for (size_t j = 1; j < _matrix.columns(); ++j)
		{
			os << ", " << _matrix(i,j);
		}
		os << "]";
	}
	os << "]";
	return os;
}

inline
std::string BaseMatrixImpl::str() const
{
	std::ostringstream oss;
	oss << (*this);
	return oss.str();
}


class MatrixImpl : public BaseMatrixImpl
{
  public:

	typedef BaseMatrixImpl base_type;

	void set_all( double x )
	{
		gsl_matrix_set_all(m_matrix, x);
	}

	void set_identity()
	{
		gsl_matrix_set_identity(m_matrix);
	}

    using base_type::operator(); // VSC legacy support
    const double & operator() (size_t i, size_t j) const
    {
        return base_type::operator ()(i, j);
    }

    double & operator() (size_t i, size_t j)
	{
            return *gsl_matrix_ptr(m_matrix, i, j);
	}

	using base_type::get_gsl_matrix;

	gsl_matrix* get_gsl_matrix()
	{
		return m_matrix;
	}

	VectorView row_view(size_t i)
	{
		return VectorView(gsl_matrix_row(m_matrix, i));
	}

	VectorView column_view(size_t i)
	{
		return VectorView(gsl_matrix_column(m_matrix, i));
	}

	void swap_rows(size_t i, size_t j)
	{
		 gsl_matrix_swap_rows(m_matrix, i, j);
	}

	void swap_columns(size_t i, size_t j)
	{
		gsl_matrix_swap_columns(m_matrix, i, j);
	}

	MatrixImpl & transpose()
	{
	    assert(columns() == rows());
		gsl_matrix_transpose(m_matrix);
		return (*this);
	}

	MatrixImpl & scale(double x)
	{
		gsl_matrix_scale(m_matrix, x);
		return (*this);
	}

	MatrixImpl & translate(double x)
	{
		 gsl_matrix_add_constant(m_matrix, x);
		 return (*this);
	}

	MatrixImpl & operator+=(base_type const& _matrix)
	{
		gsl_matrix_add(m_matrix, _matrix.get_gsl_matrix());
		return (*this);
	}

	MatrixImpl & operator-=(base_type const& _matrix)
	{
		gsl_matrix_sub(m_matrix, _matrix.get_gsl_matrix());
		return (*this);
	}

}; // end class MatrixImpl

}  // end namespace detail


using detail::operator==;
using detail::operator<<;


template <size_t N>
class ConstBaseSymmetricMatrix;


class Matrix: public detail::MatrixImpl
{
  public:
	typedef detail::MatrixImpl base_type;

  public:
	// the matrix is not inizialized
	Matrix(size_t n1, size_t n2)
	{
		m_rows = n1;
		m_columns = n2;
		m_matrix = gsl_matrix_alloc(n1, n2);
	}

	Matrix(size_t n1, size_t n2, double x)
	{
		m_rows = n1;
		m_columns = n2;
		m_matrix = gsl_matrix_alloc(n1, n2);
		gsl_matrix_set_all(m_matrix, x);
	}

	Matrix(Matrix const& _matrix)
        : base_type()
	{
		m_rows = _matrix.rows();
		m_columns = _matrix.columns();
		m_matrix = gsl_matrix_alloc(rows(), columns());
		gsl_matrix_memcpy(m_matrix, _matrix.get_gsl_matrix());
	}

	explicit
	Matrix(base_type::base_type const& _matrix)
	{
		m_rows = _matrix.rows();
		m_columns = _matrix.columns();
		m_matrix = gsl_matrix_alloc(rows(), columns());
		gsl_matrix_memcpy(m_matrix, _matrix.get_gsl_matrix());
	}

	template <size_t N>
	explicit
    Matrix(ConstBaseSymmetricMatrix<N> const& _smatrix)
	{
	    m_rows = N;
	    m_columns = N;
	    m_matrix = gsl_matrix_alloc(N, N);
	    for (size_t i = 0; i < N; ++i)
	        for (size_t j = 0; j < N ; ++j)
	            (*gsl_matrix_ptr(m_matrix, i, j)) = _smatrix(i,j);
	}

	Matrix & operator=(Matrix const& _matrix)
	{
		assert( rows() == _matrix.rows() && columns() ==  _matrix.columns() );
		gsl_matrix_memcpy(m_matrix, _matrix.get_gsl_matrix());
		return *this;
	}

	Matrix & operator=(base_type::base_type const& _matrix)
	{
		assert( rows() == _matrix.rows() && columns() ==  _matrix.columns() );
		gsl_matrix_memcpy(m_matrix, _matrix.get_gsl_matrix());
		return *this;
	}

	template <size_t N>
	Matrix & operator=(ConstBaseSymmetricMatrix<N> const& _smatrix)
	{
	    assert (rows() == N && columns() ==  N);
	    for (size_t i = 0; i < N; ++i)
	        for (size_t j = 0; j < N ; ++j)
	            (*this)(i,j) = _smatrix(i,j);
	    return *this;
	}

	virtual ~Matrix()
	{
		gsl_matrix_free(m_matrix);
	}

	Matrix & transpose()
	{
		return static_cast<Matrix &>( base_type::transpose() );
	}

	Matrix & scale(double x)
	{
		return static_cast<Matrix &>( base_type::scale(x) );
	}

	Matrix & translate(double x)
	{
		return static_cast<Matrix &>( base_type::translate(x) );
	}

	Matrix & operator+=(base_type::base_type const& _matrix)
	{
		return static_cast<Matrix &>( base_type::operator+=(_matrix) );
	}

	Matrix & operator-=(base_type::base_type const& _matrix)
	{
		return static_cast<Matrix &>( base_type::operator-=(_matrix) );
	}

	friend
	void swap(Matrix & m1, Matrix & m2);
	friend
	void swap_any(Matrix & m1, Matrix & m2);

};  // end class Matrix


// warning! this operation invalidates any view of the passed matrix objects
inline
void swap(Matrix & m1, Matrix & m2)
{
    assert(m1.rows() == m2.rows() && m1.columns() == m2.columns());
    using std::swap;
    swap(m1.m_matrix, m2.m_matrix);
}

inline void swap_any(Matrix &m1, Matrix &m2)
{
    using std::swap;
    swap(m1.m_matrix, m2.m_matrix);
    swap(m1.m_rows, m2.m_rows);
    swap(m1.m_columns, m2.m_columns);
}



class ConstMatrixView : public detail::BaseMatrixImpl
{
  public:
	typedef detail::BaseMatrixImpl base_type;

  public:
	ConstMatrixView(const base_type & _matrix, size_t k1, size_t k2, size_t n1, size_t n2)
		: m_matrix_view( gsl_matrix_const_submatrix(_matrix.get_gsl_matrix(), k1, k2, n1, n2) )
	{
		m_rows = n1;
		m_columns = n2;
		m_matrix = const_cast<gsl_matrix*>( &(m_matrix_view.matrix) );
	}

	ConstMatrixView(const ConstMatrixView & _matrix)
		: base_type(),
		  m_matrix_view(_matrix.m_matrix_view)
	{
		m_rows = _matrix.rows();
		m_columns = _matrix.columns();
		m_matrix = const_cast<gsl_matrix*>( &(m_matrix_view.matrix) );
	}

	ConstMatrixView(const base_type & _matrix)
		: m_matrix_view(gsl_matrix_const_submatrix(_matrix.get_gsl_matrix(), 0, 0, _matrix.rows(), _matrix.columns()))
	{
		m_rows = _matrix.rows();
		m_columns = _matrix.columns();
		m_matrix = const_cast<gsl_matrix*>( &(m_matrix_view.matrix) );
	}

  private:
	gsl_matrix_const_view m_matrix_view;

};  // end class ConstMatrixView




class MatrixView : public detail::MatrixImpl
{
  public:
	typedef detail::MatrixImpl base_type;

  public:
	MatrixView(base_type & _matrix, size_t k1, size_t k2, size_t n1, size_t n2)
	{
		m_rows = n1;
		m_columns = n2;
		m_matrix_view
			= gsl_matrix_submatrix(_matrix.get_gsl_matrix(), k1, k2, n1, n2);
		m_matrix = &(m_matrix_view.matrix);
	}

	MatrixView(const MatrixView & _matrix)
        : base_type()
	{
		m_rows = _matrix.rows();
		m_columns = _matrix.columns();
		m_matrix_view = _matrix.m_matrix_view;
		m_matrix = &(m_matrix_view.matrix);
	}

	MatrixView(Matrix & _matrix)
	{
		m_rows = _matrix.rows();
		m_columns = _matrix.columns();
		m_matrix_view
			= gsl_matrix_submatrix(_matrix.get_gsl_matrix(), 0, 0, rows(), columns());
		m_matrix = &(m_matrix_view.matrix);
	}

	MatrixView & operator=(MatrixView const& _matrix)
	{
		assert( rows() == _matrix.rows() && columns() ==  _matrix.columns() );
		gsl_matrix_memcpy(m_matrix, _matrix.m_matrix);
		return *this;
	}

	MatrixView & operator=(base_type::base_type const& _matrix)
	{
		assert( rows() == _matrix.rows() && columns() ==  _matrix.columns() );
		gsl_matrix_memcpy(m_matrix, _matrix.get_gsl_matrix());
		return *this;
	}

	MatrixView & transpose()
	{
		return static_cast<MatrixView &>( base_type::transpose() );
	}

	MatrixView & scale(double x)
	{
		return static_cast<MatrixView &>( base_type::scale(x) );
	}

	MatrixView & translate(double x)
	{
		return static_cast<MatrixView &>( base_type::translate(x) );
	}

	MatrixView & operator+=(base_type::base_type const& _matrix)
	{
		return static_cast<MatrixView &>( base_type::operator+=(_matrix) );
	}

	MatrixView & operator-=(base_type::base_type const& _matrix)
	{
		return static_cast<MatrixView &>( base_type::operator-=(_matrix) );
	}

	friend
	void swap_view(MatrixView & m1, MatrixView & m2);

  private:
	gsl_matrix_view m_matrix_view;

};  // end class MatrixView


inline
void swap_view(MatrixView & m1, MatrixView & m2)
{
    assert(m1.rows() == m2.rows() && m1.columns() == m2.columns());
    using std::swap;
    swap(m1.m_matrix_view, m2.m_matrix_view);
}

Vector operator*( detail::BaseMatrixImpl const& A,
                  detail::BaseVectorImpl const& v );

Matrix operator*( detail::BaseMatrixImpl const& A,
                  detail::BaseMatrixImpl const& B );

Matrix pseudo_inverse(detail::BaseMatrixImpl const& A);

double trace (detail::BaseMatrixImpl const& A);

double det (detail::BaseMatrixImpl const& A);

} } // end namespaces

#endif /*_NL_MATRIX_H_*/

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
