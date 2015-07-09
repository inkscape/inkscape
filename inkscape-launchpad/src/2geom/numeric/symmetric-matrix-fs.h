/*
 * SymmetricMatrix, ConstSymmetricMatrixView, SymmetricMatrixView template
 * classes implement fixed size symmetric matrix; "views" mimic the semantic
 * of C++ references: any operation performed on a "view" is actually performed
 * on the "viewed object"
 *
 * Authors:
 *      Marco Cecchetti <mrcekets at gmail.com>
 *
 * Copyright 2009  authors
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


#ifndef _NL_SYMMETRIC_MATRIX_FS_H_
#define _NL_SYMMETRIC_MATRIX_FS_H_


#include <2geom/numeric/vector.h>
#include <2geom/numeric/matrix.h>
#include <2geom/utils.h>
#include <2geom/exception.h>

#include <boost/static_assert.hpp>

#include <cassert>
#include <utility>     // for std::pair
#include <algorithm>   // for std::swap, std::copy
#include <sstream>
#include <string>



namespace Geom { namespace NL {


namespace detail
{

template <size_t I, size_t J, bool B = (I < J)>
struct index
{
    static const size_t K = index<J, I>::K;
};

template <size_t I, size_t J>
struct index<I, J, false>
{
    static const size_t K = (((I+1) * I) >> 1) + J;
};

} // end namespace detail




template <size_t N>
class ConstBaseSymmetricMatrix;

template <size_t N>
class BaseSymmetricMatrix;

template <size_t N>
class SymmetricMatrix;

template <size_t N>
class ConstSymmetricMatrixView;

template <size_t N>
class SymmetricMatrixView;



// declaration needed for friend clause
template <size_t N>
bool operator== (ConstBaseSymmetricMatrix<N> const& _smatrix1,
                 ConstBaseSymmetricMatrix<N> const& _smatrix2);




template <size_t N>
class ConstBaseSymmetricMatrix
{
  public:
    const static size_t DIM = N;
    const static size_t DATA_SIZE = ((DIM+1) * DIM) / 2;

  public:

    ConstBaseSymmetricMatrix (VectorView const& _data)
        : m_data(_data)
    {
    }

    double operator() (size_t i, size_t j) const
    {
        return m_data[get_index(i,j)];
    }

    template <size_t I, size_t J>
    double get() const
    {
        BOOST_STATIC_ASSERT ((I < N && J < N));
        return m_data[detail::index<I, J>::K];
    }


    size_t rows() const
    {
        return DIM;
    }

    size_t columns() const
    {
        return DIM;
    }

    bool is_zero() const
    {
        return m_data.is_zero();
    }

    bool is_positive() const
    {
        return m_data.is_positive();
    }

    bool is_negative() const
    {
        return m_data.is_negative();
    }

    bool is_non_negative() const
    {
        return m_data.is_non_negative();
    }

    double min() const
    {
        return m_data.min();
    }

    double max() const
    {
        return m_data.max();
    }

    std::pair<size_t, size_t>
    min_index() const
    {
        std::pair<size_t, size_t> indices(0,0);
        double min_value = m_data[0];
        for (size_t i = 1; i < DIM; ++i)
        {
            for (size_t j = 0; j <= i; ++j)
            {
                if (min_value > (*this)(i,j))
                {
                    min_value = (*this)(i,j);
                    indices.first = i;
                    indices.second = j;
                }
            }
        }
        return indices;
    }

    std::pair<size_t, size_t>
    max_index() const
    {
        std::pair<size_t, size_t> indices(0,0);
        double max_value = m_data[0];
        for (size_t i = 1; i < DIM; ++i)
        {
            for (size_t j = 0; j <= i; ++j)
            {
                if (max_value < (*this)(i,j))
                {
                    max_value = (*this)(i,j);
                    indices.first = i;
                    indices.second = j;
                }
            }
        }
        return indices;
    }

    size_t min_on_row_index (size_t i) const
    {
        size_t idx = 0;
        double min_value = (*this)(i,0);
        for (size_t j = 1; j < DIM; ++j)
        {
            if (min_value > (*this)(i,j))
            {
                min_value = (*this)(i,j);
                idx = j;
            }
        }
        return idx;
    }

    size_t max_on_row_index (size_t i) const
    {
        size_t idx = 0;
        double max_value = (*this)(i,0);
        for (size_t j = 1; j < DIM; ++j)
        {
            if (max_value < (*this)(i,j))
            {
                max_value = (*this)(i,j);
                idx = j;
            }
        }
        return idx;
    }

    size_t min_on_column_index (size_t j) const
    {
        return min_on_row_index(j);
    }

    size_t max_on_column_index (size_t j) const
    {
        return max_on_row_index(j);
    }

    size_t min_on_diag_index () const
    {
        size_t idx = 0;
        double min_value = (*this)(0,0);
        for (size_t i = 1; i < DIM; ++i)
        {
            if (min_value > (*this)(i,i))
            {
                min_value = (*this)(i,i);
                idx = i;
            }
        }
        return idx;
    }

    size_t max_on_diag_index () const
    {
        size_t idx = 0;
        double max_value = (*this)(0,0);
        for (size_t i = 1; i < DIM; ++i)
        {
            if (max_value < (*this)(i,i))
            {
                max_value = (*this)(i,i);
                idx = i;
            }
        }
        return idx;
    }

    std::string str() const;

    ConstSymmetricMatrixView<N-1> main_minor_const_view() const;

    SymmetricMatrix<N> operator- () const;

    Vector operator* (ConstVectorView _vector) const
    {
        assert (_vector.size() == DIM);
        Vector result(DIM, 0.0);

        for (size_t i = 0; i < DIM; ++i)
        {
            for (size_t j = 0; j < DIM; ++j)
            {
                result[i] += (*this)(i,j) * _vector[j];
            }
        }
        return result;
    }

  protected:
    static size_t get_index (size_t i, size_t j)
    {
        if (i < j) return get_index (j, i);
        size_t k = (i+1) * i;
        k >>= 1;
        k += j;
        return k;
    }

  protected:
    ConstVectorView get_data() const
    {
      return m_data;
    }

    friend
    bool operator==<N> (ConstBaseSymmetricMatrix const& _smatrix1,
                        ConstBaseSymmetricMatrix const& _smatrix2);

  protected:
    VectorView m_data;

}; //end ConstBaseSymmetricMatrix


template <size_t N>
class BaseSymmetricMatrix : public ConstBaseSymmetricMatrix<N>
{
  public:
    typedef ConstBaseSymmetricMatrix<N> base_type;


  public:

    BaseSymmetricMatrix (VectorView const& _data)
        : base_type(_data)
    {
    }

    double operator() (size_t i, size_t j) const 
    {
        return m_data[base_type::get_index(i,j)];
    }

    double& operator() (size_t i, size_t j)
    {
        return m_data[base_type::get_index(i,j)];
    }

    template <size_t I, size_t J>
    double& get()
    {
        BOOST_STATIC_ASSERT ((I < N && J < N));
        return m_data[detail::index<I, J>::K];
    }

    void set_all (double x)
    {
        m_data.set_all(x);
    }

    SymmetricMatrixView<N-1> main_minor_view();

    BaseSymmetricMatrix& transpose() const
    {
        return (*this);
    }

    BaseSymmetricMatrix& translate (double c)
    {
        m_data.translate(c);
        return (*this);
    }

    BaseSymmetricMatrix& scale (double c)
    {
        m_data.scale(c);
        return (*this);
    }

    BaseSymmetricMatrix& operator+= (base_type const& _smatrix)
    {
        m_data += (static_cast<const BaseSymmetricMatrix &>(_smatrix).m_data);
        return (*this);
    }

    BaseSymmetricMatrix& operator-= (base_type const& _smatrix)
    {
        m_data -= (static_cast<const BaseSymmetricMatrix &>(_smatrix).m_data);
        return (*this);
    }

    using base_type::DIM;
    using base_type::DATA_SIZE;
    using base_type::m_data;
    using base_type::operator-;
    using base_type::operator*;

}; //end BaseSymmetricMatrix


template <size_t N>
class SymmetricMatrix : public BaseSymmetricMatrix<N>
{
  public:
    typedef BaseSymmetricMatrix<N> base_type;
    typedef typename  base_type::base_type base_base_type;

    using base_type::DIM;
    using base_type::DATA_SIZE;
    using base_type::m_data;

  public:
    SymmetricMatrix ()
        : base_type (VectorView(m_adata, DATA_SIZE))
    {
    }

    explicit
    SymmetricMatrix (ConstVectorView _data)
        : base_type (VectorView(m_adata, DATA_SIZE))
    {
        assert (_data.size() == DATA_SIZE);
        m_data = _data;
    }

    explicit
    SymmetricMatrix (const double _data[DATA_SIZE])
        : base_type (VectorView(m_adata, DATA_SIZE))
    {
        std::copy (_data, _data + DATA_SIZE, m_adata);
    }

    SymmetricMatrix (SymmetricMatrix const& _smatrix)
        : base_type (VectorView(m_adata, DATA_SIZE))
    {
        m_data = _smatrix.m_data;
    }

    explicit
    SymmetricMatrix (base_base_type const& _smatrix)
        : base_type (VectorView(m_adata, DATA_SIZE))
    {
        m_data = static_cast<const ConstSymmetricMatrixView<DIM> &>(_smatrix).m_data;
    }

    explicit
    SymmetricMatrix (ConstMatrixView const& _matrix)
        : base_type (VectorView(m_adata, DATA_SIZE))
    {
        assert (_matrix.rows() == N && _matrix.columns() == N);
        for (size_t i = 0; i < N; ++i)
            for (size_t j = 0; j <= i ; ++j)
                (*this)(i,j) = _matrix(i,j);
    }

    SymmetricMatrix& operator= (SymmetricMatrix const& _smatrix)
    {
        m_data = _smatrix.m_data;
        return (*this);
    }

    SymmetricMatrix& operator= (base_base_type const& _smatrix)
    {

        m_data = static_cast<const ConstSymmetricMatrixView<DIM> &>(_smatrix).m_data;
        return (*this);
    }

    SymmetricMatrix& operator= (ConstMatrixView const& _matrix)
    {
        assert (_matrix.rows() == N && _matrix.columns() == N);
        for (size_t i = 0; i < N; ++i)
            for (size_t j = 0; j <= i ; ++j)
                (*this)(i,j) = _matrix(i,j);

        return (*this);
    }

    // needed for accessing m_adata
    friend class ConstSymmetricMatrixView<DIM>;
    friend class SymmetricMatrixView<DIM>;
  private:
      double m_adata[DATA_SIZE];
}; //end SymmetricMatrix


template <size_t N>
class ConstSymmetricMatrixView : public ConstBaseSymmetricMatrix<N>
{
  public:
    typedef ConstBaseSymmetricMatrix<N> base_type;

    using base_type::DIM;
    using base_type::DATA_SIZE;
    using base_type::m_data;


  public:

    explicit
    ConstSymmetricMatrixView (ConstVectorView _data)
        : base_type (const_vector_view_cast(_data))
    {
        assert (_data.size() == DATA_SIZE);
    }

    explicit
    ConstSymmetricMatrixView (const double _data[DATA_SIZE])
        : base_type (const_vector_view_cast (ConstVectorView (_data, DATA_SIZE)))
    {
    }

    ConstSymmetricMatrixView (const ConstSymmetricMatrixView & _smatrix)
        : base_type (_smatrix.m_data)
    {
    }

    ConstSymmetricMatrixView (const base_type & _smatrix)
        : base_type (static_cast<const ConstSymmetricMatrixView &>(_smatrix).m_data)
    {
    }

}; //end SymmetricMatrix


// declaration needed for friend clause
template <size_t N>
void swap_view(SymmetricMatrixView<N> & m1, SymmetricMatrixView<N> & m2);


template <size_t N>
class SymmetricMatrixView : public BaseSymmetricMatrix<N>
{
  public:
    typedef BaseSymmetricMatrix<N> base_type;
    typedef typename  base_type::base_type base_base_type;

    using base_type::DIM;
    using base_type::DATA_SIZE;
    using base_type::m_data;

  public:

    explicit
    SymmetricMatrixView (VectorView _data)
        : base_type (_data)
    {
        assert (_data.size() == DATA_SIZE);
    }

    explicit
    SymmetricMatrixView (double _data[DATA_SIZE])
        : base_type (VectorView (_data, DATA_SIZE))
    {
    }

    SymmetricMatrixView (const SymmetricMatrixView & _smatrix)
        : base_type (_smatrix.m_data)
    {
    }

    SymmetricMatrixView (SymmetricMatrix<DIM> & _smatrix)
        : base_type (VectorView (_smatrix.m_adata, DATA_SIZE))
    {
    }

    SymmetricMatrixView& operator= (const SymmetricMatrixView & _smatrix)
    {
        m_data = _smatrix.m_data;
        return (*this);
    }

    SymmetricMatrixView& operator= (const base_base_type & _smatrix)
    {
        m_data = static_cast<const ConstSymmetricMatrixView<DIM> &>(_smatrix).m_data;
        return (*this);
    }

    friend
    void swap_view<N>(SymmetricMatrixView & m1, SymmetricMatrixView & m2);

}; //end SymmetricMatrix




/*
 * class ConstBaseSymmetricMatrix methods
 */

template <size_t N>
inline
std::string ConstBaseSymmetricMatrix<N>::str() const
{
    std::ostringstream oss;
    oss << (*this);
    return oss.str();
}

template <size_t N>
inline
ConstSymmetricMatrixView<N-1>
ConstBaseSymmetricMatrix<N>::main_minor_const_view() const
{
    ConstVectorView data(m_data.get_gsl_vector()->data, DATA_SIZE - DIM);
    ConstSymmetricMatrixView<N-1> mm(data);
    return mm;
}

template <size_t N>
inline
SymmetricMatrix<N> ConstBaseSymmetricMatrix<N>::operator- () const
{
    SymmetricMatrix<N> result;
    for (size_t i = 0; i < DATA_SIZE; ++i)
    {
        result.m_data[i] = -m_data[i];
    }
    return result;
}


/*
 * class ConstBaseSymmetricMatrix friend free functions
 */

template <size_t N>
inline
bool operator== (ConstBaseSymmetricMatrix<N> const& _smatrix1,
                 ConstBaseSymmetricMatrix<N> const& _smatrix2)
{
    return (_smatrix1.m_data == _smatrix2.m_data);
}

/*
 * class ConstBaseSymmetricMatrix related free functions
 */

template< size_t N, class charT >
inline
std::basic_ostream<charT> &
operator<< (std::basic_ostream<charT> & os,
            const ConstBaseSymmetricMatrix<N> & _matrix)
{
    os << "[[" << _matrix(0,0);
    for (size_t j = 1; j < N; ++j)
    {
        os << ", " << _matrix(0,j);
    }
    os << "]";
    for (size_t i = 1; i < N; ++i)
    {
        os << "\n [" << _matrix(i,0);
        for (size_t j = 1; j < N; ++j)
        {
            os << ", " << _matrix(i,j);
        }
        os << "]";
    }
    os << "]";
    return os;
}


/*
 * class ConstBaseSymmetricMatrix specialized methods
 */

template<>
inline
size_t ConstBaseSymmetricMatrix<2>::get_index (size_t i, size_t j)
{
    return (i+j);
}

template<>
inline
size_t ConstBaseSymmetricMatrix<3>::get_index (size_t i, size_t j)
{
    size_t k = i + j;
    if (i == 2 || j == 2)  ++k;
    return k;
}


/*
 * class BaseSymmetricMatrix methods
 */

template <size_t N>
inline
SymmetricMatrixView<N-1> BaseSymmetricMatrix<N>::main_minor_view()
{
    VectorView data(m_data.get_gsl_vector()->data, DATA_SIZE - DIM);
    SymmetricMatrixView<N-1> mm(data);
    return mm;
}


/*
 * class SymmetricMatrixView friend free functions
 */

template <size_t N>
inline
void swap_view(SymmetricMatrixView<N> & m1, SymmetricMatrixView<N> & m2)
{
    swap_view(m1.m_data, m2.m_data);
}

} /* end namespace NL*/ } /* end namespace Geom*/




#endif  // _NL_SYMMETRIC_MATRIX_FS_H_




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
