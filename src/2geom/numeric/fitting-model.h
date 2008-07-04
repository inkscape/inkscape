/*
 * Fitting Models for Geom Types
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


#ifndef _NL_FITTING_MODEL_H_
#define _NL_FITTING_MODEL_H_


#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/bezier.h>
#include <2geom/bezier-curve.h>
#include <2geom/poly.h>
#include <2geom/ellipse.h>
#include <2geom/utils.h>


namespace Geom { namespace NL {


/*
 *   completely unknown models must inherit from this template class;
 *   example: the model a*x^2 + b*x + c = 0 to be solved wrt a, b, c;
 *   example: the model A(t) = known_sample_value_at(t) to be solved wrt
 *       the coefficients of the curve A(t) expressed in S-Basis form;
 *   parameter type: the type of x and t variable in the examples above;
 *   value type:     the type of the known sample values (in the first example
 *                   is constant )
 *   instance type:  the type of the objects produced by using
 *                   the fitting raw data solution
 */
template< typename ParameterType, typename ValueType, typename InstanceType >
class LinearFittingModel
{
  public:
    typedef ParameterType       parameter_type;
    typedef ValueType           value_type;
    typedef InstanceType        instance_type;

    static const bool WITH_FIXED_TERMS = false;

    /*
     * a LinearFittingModel must implement the following methods:
     *
     * void feed( VectorView & vector,
     *            parameter_type const& sample_parameter ) const;
     *
     * size_t size() const;
     *
     * void instance(instance_type &, raw_type const& raw_data) const;
     *
     */
};


/*
 *   partially known models must inherit from this template class
 *   example: the model a*x^2 + 2*x + c = 0 to be solved wrt a and c
 */
template< typename ParameterType, typename ValueType, typename InstanceType >
class LinearFittingModelWithFixedTerms
{
  public:
    typedef ParameterType       parameter_type;
    typedef ValueType           value_type;
    typedef InstanceType        instance_type;

    static const bool WITH_FIXED_TERMS = true;

    /*
     * a LinearFittingModelWithFixedTerms must implement the following methods:
     *
     * void feed( VectorView & vector,
     *            value_type & fixed_term,
     *            parameter_type const& sample_parameter ) const;
     *
     * size_t size() const;
     *
     * void instance(instance_type &, raw_type const& raw_data) const;
     *
     */


};


// incomplete model, it can be inherited to make up different kinds of
// instance type; the raw data is a vector of coefficients of a polynomial
// rapresented in standard power basis
template< typename InstanceType >
class LFMPowerBasis
    : public LinearFittingModel<double, double, InstanceType>
{
  public:
    LFMPowerBasis(size_t degree)
        : m_size(degree + 1)
    {
    }

    void feed( VectorView & coeff, double sample_parameter ) const
    {
        coeff[0] = 1;
        double x_i = 1;
        for (size_t i = 1; i < coeff.size(); ++i)
        {
          x_i *= sample_parameter;
          coeff[i] = x_i;
        }
    }

    size_t size() const
    {
        return m_size;
    }

  private:
    size_t m_size;
};


// this model generates Geom::Poly objects
class LFMPoly
    : public LFMPowerBasis<Poly>
{
  public:
    LFMPoly(size_t degree)
        : LFMPowerBasis<Poly>(degree)
    {
    }

    void instance(Poly & poly, ConstVectorView const& raw_data) const
    {
        poly.clear();
        poly.resize(size());
        for (size_t i = 0; i < raw_data.size(); ++i)
        {
            poly[i] =  raw_data[i];
        }
    }
};


// incomplete model, it can be inherited to make up different kinds of
// instance type; the raw data is a vector of coefficients of a polynomial
// rapresented in standard power basis with leading term coefficient equal to 1
template< typename InstanceType >
class LFMNormalizedPowerBasis
    : public LinearFittingModelWithFixedTerms<double, double, InstanceType>
{
  public:
    LFMNormalizedPowerBasis(size_t _degree)
        : m_model( _degree - 1)
    {
        assert(_degree > 0);
    }


    void feed( VectorView & coeff,
               double & known_term,
               double sample_parameter ) const
    {
        m_model.feed(coeff, sample_parameter);
        known_term = coeff[m_model.size()-1] * sample_parameter;
    }

    size_t size() const
    {
        return m_model.size();
    }

  private:
    LFMPowerBasis<InstanceType> m_model;
};


// incomplete model, it can be inherited to make up different kinds of
// instance type; the raw data is a vector of coefficients of the equation
// of an ellipse curve
template< typename InstanceType >
class LFMEllipseEquation
    : public LinearFittingModelWithFixedTerms<Point, double, InstanceType>
{
  public:
    void feed( VectorView & coeff, double & fixed_term, Point const& p ) const
    {
        coeff[0] = p[X] * p[Y];
        coeff[1] = p[Y] * p[Y];
        coeff[2] = p[X];
        coeff[3] = p[Y];
        coeff[4] = 1;
        fixed_term = p[X] * p[X];
    }

    size_t size() const
    {
        return 5;
    }
};


// this model generates Ellipse curves
class LFMEllipse
    : public LFMEllipseEquation<Ellipse>
{
  public:
    void instance(Ellipse & e, ConstVectorView const& coeff) const
    {
        e.set(1, coeff[0], coeff[1], coeff[2], coeff[3], coeff[4]);
    }
};


// this model generates SBasis objects
class LFMSBasis
    : public LinearFittingModel<double, double, SBasis>
{
  public:
    LFMSBasis( size_t _order )
        : m_size( 2*(_order+1) ),
          m_order(_order)
    {
    }

    void feed( VectorView & coeff, double t ) const
    {
        double u0 = 1-t;
        double u1 = t;
        double s = u0 * u1;
        coeff[0] = u0;
        coeff[1] = u1;
        for (size_t i = 2; i < size(); i+=2)
        {
            u0 *= s;
            u1 *= s;
            coeff[i] = u0;
            coeff[i+1] = u1;
        }
    }

    size_t size() const
    {
        return m_size;
    }

    void instance(SBasis & sb, ConstVectorView const& raw_data) const
    {
        sb.clear();
        sb.resize(m_order+1);
        for (unsigned int i = 0, k = 0; i < raw_data.size(); i+=2, ++k)
        {
            sb[k][0] = raw_data[i];
            sb[k][1] = raw_data[i+1];
        }
    }

  private:
    size_t m_size;
    size_t m_order;
};


// this model generates D2<SBasis> objects
class LFMD2SBasis
    : public LinearFittingModel< double, Point, D2<SBasis> >
{
  public:
    LFMD2SBasis( size_t _order )
        : mosb(_order)
    {
    }

    void feed( VectorView & coeff, double t ) const
    {
        mosb.feed(coeff, t);
    }

    size_t size() const
    {
        return mosb.size();
    }

    void instance(D2<SBasis> & d2sb, ConstMatrixView const& raw_data) const
    {
        mosb.instance(d2sb[X], raw_data.column_const_view(X));
        mosb.instance(d2sb[Y], raw_data.column_const_view(Y));
    }

  private:
    LFMSBasis mosb;
};


// this model generates Bezier objects
class LFMBezier
    : public LinearFittingModel<double, double, Bezier>
{
  public:
    LFMBezier( size_t _order )
        : m_size(_order + 1),
          m_order(_order)
    {
        binomial_coefficients(m_bc, m_order);
    }

    void feed( VectorView & coeff, double t ) const
    {
        double s = 1;
        for (size_t i = 0; i < size(); ++i)
        {
            coeff[i] = s * m_bc[i];
            s *= t;
        }
        double u = 1-t;
        s = 1;
        for (size_t i = size()-1; i > 0; --i)
        {
            coeff[i] *= s;
            s *= u;
        }
        coeff[0] *= s;
    }

    size_t size() const
    {
        return m_size;
    }

    void instance(Bezier & b, ConstVectorView const& raw_data) const
    {
        assert(b.size() == raw_data.size());
        for (unsigned int i = 0; i < raw_data.size(); ++i)
        {
            b[i] = raw_data[i];
        }
    }

  private:
    size_t m_size;
    size_t m_order;
    std::vector<size_t> m_bc;
};


// this model generates Bezier curves
template< unsigned int N >
class LFMBezierCurve
    : public LinearFittingModel< double, Point, BezierCurve<N> >
{
  public:
    LFMBezierCurve( size_t _order )
        : mob(_order)
    {
    }

    void feed( VectorView & coeff, double t ) const
    {
        mob.feed(coeff, t);
    }

    size_t size() const
    {
        return mob.size();
    }

    void instance(BezierCurve<N> & bc, ConstMatrixView const& raw_data) const
    {
        Bezier bx(size()-1);
        Bezier by(size()-1);
        mob.instance(bx, raw_data.column_const_view(X));
        mob.instance(by, raw_data.column_const_view(Y));
        bc = BezierCurve<N>(bx, by);
    }

  private:
    LFMBezier mob;
};

}  // end namespace NL
}  // end namespace Geom


#endif // _NL_FITTING_MODEL_H_


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
