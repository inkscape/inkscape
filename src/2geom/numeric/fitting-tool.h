/*
 * Fitting Tools
 *
 * Authors:
 *      Marco Cecchetti <mrcekets at gmail.com>
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


#ifndef _NL_FITTING_TOOL_H_
#define _NL_FITTING_TOOL_H_


#include <2geom/numeric/vector.h>
#include <2geom/numeric/matrix.h>

#include <2geom/point.h>

#include <vector>


/*
 *  The least_square_fitter class represents a tool for solving a fitting
 *  problem with respect to a given model that represents an expression
 *  dependent from a parameter where the coefficients of this expression
 *  are the unknowns of the fitting problem.
 *  The minimizing solution is found by computing the pseudo-inverse
 *  of the problem matrix
 */


namespace Geom { namespace NL {

namespace detail {

template< typename ModelT>
class lsf_base
{
  public:
    typedef ModelT                                   model_type;
    typedef typename model_type::parameter_type      parameter_type;
    typedef typename model_type::value_type          value_type;

    lsf_base( model_type const& _model, size_t forecasted_samples )
        : m_model(_model),
          m_total_samples(0),
          m_matrix(forecasted_samples, m_model.size()),
          m_psdinv_matrix(NULL)
    {
    }

    // compute pseudo inverse
    void update()
    {
        if (total_samples() == 0) return;
        if (m_psdinv_matrix != NULL)
        {
            delete m_psdinv_matrix;
        }
        MatrixView mv(m_matrix, 0, 0, total_samples(), m_matrix.columns());
        m_psdinv_matrix = new Matrix( pseudo_inverse(mv) );
        assert(m_psdinv_matrix != NULL);
    }

    size_t total_samples() const
    {
        return m_total_samples;
    }

    bool is_full() const
    {
        return (total_samples() == m_matrix.rows());
    }

    void clear()
    {
        m_total_samples = 0;
    }

    virtual
    ~lsf_base()
    {
        if (m_psdinv_matrix != NULL)
        {
            delete m_psdinv_matrix;
        }
    }

  protected:
    const model_type &      m_model;
    size_t                  m_total_samples;
    Matrix                  m_matrix;
    Matrix*                 m_psdinv_matrix;

};  // end class lsf_base




template< typename ModelT, typename ValueType = typename ModelT::value_type>
class lsf_solution
{
};

// a fitting process on samples with value of type double
// produces a solution of type Vector
template< typename ModelT>
class lsf_solution<ModelT, double>
    : public lsf_base<ModelT>
{
public:
    typedef ModelT                                  model_type;
    typedef typename model_type::parameter_type     parameter_type;
    typedef typename model_type::value_type         value_type;
    typedef Vector                                  solution_type;
    typedef lsf_base<model_type>                    base_type;

    using base_type::m_model;
    using base_type::m_psdinv_matrix;
    using base_type::total_samples;

public:
    lsf_solution<ModelT, double>( model_type const& _model,
                                  size_t forecasted_samples )
        : base_type(_model, forecasted_samples),
          m_solution(_model.size())
    {
    }

    template< typename VectorT >
    solution_type& result(VectorT const& sample_values)
    {
        assert(sample_values.size() == total_samples());
        ConstVectorView sv(sample_values);
        m_solution = (*m_psdinv_matrix) * sv;
        return m_solution;
    }

    // a comparison between old sample values and the new ones is performed
    // in order to minimize computation
    // prerequisite:
    //     old_sample_values.size() == new_sample_values.size()
    //     no update() call can be performed between two result invocations
    template< typename VectorT >
    solution_type& result( VectorT const& old_sample_values,
                           VectorT const& new_sample_values )
    {
        assert(old_sample_values.size() == total_samples());
        assert(new_sample_values.size() == total_samples());
        Vector diff(total_samples());
        for (size_t i = 0; i < diff.size(); ++i)
        {
            diff[i] = new_sample_values[i] - old_sample_values[i];
        }
        Vector column(m_model.size());
        Vector delta(m_model.size(), 0.0);
        for (size_t i = 0; i < diff.size(); ++i)
        {
            if (diff[i] != 0)
            {
                column = m_psdinv_matrix->column_view(i);
                column.scale(diff[i]);
                delta += column;
            }
        }
        m_solution += delta;
        return m_solution;
    }

    solution_type& result()
    {
        return m_solution;
    }

private:
    solution_type m_solution;

}; // end class lsf_solution<ModelT, double>


// a fitting process on samples with value of type Point
// produces a solution of type Matrix (with 2 columns)
template< typename ModelT>
class lsf_solution<ModelT, Point>
    : public lsf_base<ModelT>
{
public:
    typedef ModelT                                  model_type;
    typedef typename model_type::parameter_type     parameter_type;
    typedef typename model_type::value_type         value_type;
    typedef Matrix                                  solution_type;
    typedef lsf_base<model_type>                    base_type;

    using base_type::m_model;
    using base_type::m_psdinv_matrix;
    using base_type::total_samples;

public:
    lsf_solution<ModelT, Point>( model_type const& _model,
                                 size_t forecasted_samples )
        : base_type(_model, forecasted_samples),
          m_solution(_model.size(), 2)
    {
    }

    solution_type& result(std::vector<Point> const& sample_values)
    {
        assert(sample_values.size() == total_samples());
        Matrix svm(total_samples(), 2);
        for (size_t i = 0; i < total_samples(); ++i)
        {
            svm(i, X) = sample_values[i][X];
            svm(i, Y) = sample_values[i][Y];
        }
        m_solution = (*m_psdinv_matrix) * svm;
        return m_solution;
    }

    // a comparison between old sample values and the new ones is performed
    // in order to minimize computation
    // prerequisite:
    //     old_sample_values.size() == new_sample_values.size()
    //     no update() call can to be performed between two result invocations
    solution_type& result( std::vector<Point> const& old_sample_values,
                           std::vector<Point> const& new_sample_values )
    {
        assert(old_sample_values.size() == total_samples());
        assert(new_sample_values.size() == total_samples());
        Matrix diff(total_samples(), 2);
        for (size_t i = 0; i < total_samples(); ++i)
        {
            diff(i, X) =  new_sample_values[i][X] - old_sample_values[i][X];
            diff(i, Y) =  new_sample_values[i][Y] - old_sample_values[i][Y];
        }
        Vector column(m_model.size());
        Matrix delta(m_model.size(), 2, 0.0);
        VectorView deltax = delta.column_view(X);
        VectorView deltay = delta.column_view(Y);
        for (size_t i = 0; i < total_samples(); ++i)
        {
            if (diff(i, X) != 0)
            {
                column = m_psdinv_matrix->column_view(i);
                column.scale(diff(i, X));
                deltax += column;
            }
            if (diff(i, Y) != 0)
            {
                column = m_psdinv_matrix->column_view(i);
                column.scale(diff(i, Y));
                deltay += column;
            }
        }
        m_solution += delta;
        return m_solution;
    }

    solution_type& result()
    {
        return m_solution;
    }

private:
    solution_type m_solution;

}; // end class lsf_solution<ModelT, Point>




template< typename ModelT,
          bool WITH_FIXED_TERMS = ModelT::WITH_FIXED_TERMS >
class lsf_with_fixed_terms
{
};


// fitting tool for completely unknown models
template< typename ModelT>
class lsf_with_fixed_terms<ModelT, false>
    : public lsf_solution<ModelT>
{
  public:
    typedef ModelT                                      model_type;
    typedef typename model_type::parameter_type         parameter_type;
    typedef typename model_type::value_type             value_type;
    typedef lsf_solution<model_type>                    base_type;
    typedef typename base_type::solution_type           solution_type;

    using base_type::total_samples;
    using base_type::is_full;
    using base_type::m_matrix;
    using base_type::m_total_samples;
    using base_type::m_model;

  public:
      lsf_with_fixed_terms<ModelT, false>( model_type const& _model,
                                           size_t forecasted_samples )
        : base_type(_model, forecasted_samples)
    {
    }

    void append(parameter_type const& sample_parameter)
    {
        assert(!is_full());
        VectorView row = m_matrix.row_view(total_samples());
        m_model.feed(row, sample_parameter);
        ++m_total_samples;
    }

    void append_copy(size_t sample_index)
    {
        assert(!is_full());
        assert(sample_index < total_samples());
        VectorView dest_row = m_matrix.row_view(total_samples());
        VectorView source_row = m_matrix.row_view(sample_index);
        dest_row = source_row;
        ++m_total_samples;
    }

}; // end class lsf_with_fixed_terms<ModelT, false>


// fitting tool for partially known models
template< typename ModelT>
class lsf_with_fixed_terms<ModelT, true>
    : public lsf_solution<ModelT>
{
  public:
    typedef ModelT                                      model_type;
    typedef typename model_type::parameter_type         parameter_type;
    typedef typename model_type::value_type             value_type;
    typedef lsf_solution<model_type>                    base_type;
    typedef typename base_type::solution_type           solution_type;

    using base_type::total_samples;
    using base_type::is_full;
    using base_type::m_matrix;
    using base_type::m_total_samples;
    using base_type::m_model;

  public:
    lsf_with_fixed_terms<ModelT, true>( model_type const& _model,
                                        size_t forecasted_samples )
        : base_type(_model, forecasted_samples),
          m_vector(forecasted_samples),
          m_vector_view(NULL)
    {
    }
    void append(parameter_type const& sample_parameter)
    {
        assert(!is_full());
        VectorView row = m_matrix.row_view(total_samples());
        m_model.feed(row, m_vector[total_samples()], sample_parameter);
        ++m_total_samples;
    }

    void append_copy(size_t sample_index)
    {
        assert(!is_full());
        assert(sample_index < total_samples());
        VectorView dest_row = m_matrix.row_view(total_samples());
        VectorView source_row = m_matrix.row_view(sample_index);
        dest_row = source_row;
        m_vector[total_samples()] = m_vector[sample_index];
        ++m_total_samples;
    }

    void update()
    {
        base_type::update();
        if (total_samples() == 0) return;
        if (m_vector_view != NULL)
        {
            delete m_vector_view;
        }
        m_vector_view = new VectorView(m_vector, base_type::total_samples());
        assert(m_vector_view != NULL);
    }

    virtual
    ~lsf_with_fixed_terms<model_type, true>()
    {
        if (m_vector_view != NULL)
        {
            delete m_vector_view;
        }
    }

  protected:
    Vector          m_vector;
    VectorView*     m_vector_view;

}; // end class lsf_with_fixed_terms<ModelT, true>


} // end namespace detail




template< typename ModelT,
          typename ValueType = typename ModelT::value_type,
          bool WITH_FIXED_TERMS = ModelT::WITH_FIXED_TERMS >
class least_squeares_fitter
{
};


template< typename ModelT, typename ValueType >
class least_squeares_fitter<ModelT, ValueType, false>
    : public detail::lsf_with_fixed_terms<ModelT>
{
  public:
    typedef ModelT                                      model_type;
    typedef detail::lsf_with_fixed_terms<model_type>    base_type;
    typedef typename base_type::parameter_type          parameter_type;
    typedef typename base_type::value_type              value_type;
    typedef typename base_type::solution_type           solution_type;

  public:
    least_squeares_fitter<ModelT, ValueType, false>( model_type const& _model,
                                                     size_t forecasted_samples )
          : base_type(_model, forecasted_samples)
  {
  }
}; // end class least_squeares_fitter<ModelT, ValueType, true>


template< typename ModelT>
class least_squeares_fitter<ModelT, double, true>
    : public detail::lsf_with_fixed_terms<ModelT>
{
  public:
    typedef ModelT                                      model_type;
    typedef detail::lsf_with_fixed_terms<model_type>    base_type;
    typedef typename base_type::parameter_type          parameter_type;
    typedef typename base_type::value_type              value_type;
    typedef typename base_type::solution_type           solution_type;

    using base_type::m_vector_view;
    //using base_type::result; // VSC legacy support
    solution_type& result( std::vector<Point> const& old_sample_values,
                           std::vector<Point> const& new_sample_values )
    {
        return base_type::result(old_sample_values, new_sample_values);
    }
 
    solution_type& result()
    {
        return base_type::result();
    }

  public:
    least_squeares_fitter<ModelT, double, true>( model_type const& _model,
                                                 size_t forecasted_samples )
        : base_type(_model, forecasted_samples)
    {
    }

    template< typename VectorT >
    solution_type& result(VectorT const& sample_values)
    {
        assert(sample_values.size() == m_vector_view->size());
        Vector sv(sample_values.size());
        for (size_t i = 0; i < sv.size(); ++i)
            sv[i] = sample_values[i] - (*m_vector_view)[i];
        return base_type::result(sv);
    }

}; // end class least_squeares_fitter<ModelT, double, true>


template< typename ModelT>
class least_squeares_fitter<ModelT, Point, true>
    : public detail::lsf_with_fixed_terms<ModelT>
{
  public:
    typedef ModelT                                      model_type;
    typedef detail::lsf_with_fixed_terms<model_type>    base_type;
    typedef typename base_type::parameter_type          parameter_type;
    typedef typename base_type::value_type              value_type;
    typedef typename base_type::solution_type           solution_type;

    using base_type::m_vector_view;
    //using base_type::result; // VCS legacy support
    solution_type& result( std::vector<Point> const& old_sample_values,
                           std::vector<Point> const& new_sample_values )
    {
        return base_type::result(old_sample_values, new_sample_values);
    }
 
    solution_type& result()
    {
        return base_type::result();
    }


  public:
    least_squeares_fitter<ModelT, Point, true>( model_type const& _model,
                                                size_t forecasted_samples )
        : base_type(_model, forecasted_samples)
    {
    }

    solution_type& result(std::vector<Point> const& sample_values)
    {
        assert(sample_values.size() == m_vector_view->size());
        NL::Matrix sv(sample_values.size(), 2);
        for (size_t i = 0; i < sample_values.size(); ++i)
        {
            sv(i, X) = sample_values[i][X] - (*m_vector_view)[i];
            sv(i, Y) = sample_values[i][Y] - (*m_vector_view)[i];
        }
        return base_type::result(sv);
    }

};  // end class least_squeares_fitter<ModelT, Point, true>


}  // end namespace NL
}  // end namespace Geom



#endif  // _NL_FITTING_TOOL_H_


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
