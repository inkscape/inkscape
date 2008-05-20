#ifndef _NL_VECTOR_H_
#define _NL_VECTOR_H_

#include <cassert>
#include <utility>

#include <gsl/gsl_vector.h>


namespace Geom { namespace NL {

class Vector;
void swap(Vector & v1, Vector & v2);


class Vector
{
public:
	Vector(size_t n)
		: m_size(n)
	{
		m_vector = gsl_vector_alloc(n);
	}
	
	Vector(size_t n, double x)
		: m_size(n)
	{
		m_vector = gsl_vector_alloc(n);
		gsl_vector_set_all(m_vector, x);
	}
	
	// create a vector with n elements all set to zero 
	// but the i-th that is set to 1
	Vector(size_t n, size_t i)
		: m_size(n)
	{
		m_vector = gsl_vector_alloc(n);
		gsl_vector_set_basis(m_vector, i);
	}
	
	Vector(Vector const& _vector)
		: m_size(_vector.size())
	{
		m_vector = gsl_vector_alloc(size());
		gsl_vector_memcpy(m_vector, _vector.m_vector);
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
	
	void set_all(double x = 0)
	{
		gsl_vector_set_all(m_vector, x);
	}

	void set_basis(size_t i)
	{
		gsl_vector_set_basis(m_vector, i);
	}
	
	double const& operator[](size_t i) const
	{
		return *gsl_vector_const_ptr(m_vector, i);
	}
	
	double & operator[](size_t i)
	{
		return *gsl_vector_ptr(m_vector, i);
	}
	
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
	
	Vector & scale(double x)
	{
		gsl_vector_scale(m_vector, x);
		return (*this);
	}
	
	Vector & translate(double x)
	{
		gsl_vector_add_constant(m_vector, x);
		return (*this);
	}
	
	Vector & operator+=(Vector const& _vector)
	{
		gsl_vector_add(m_vector, _vector.m_vector);
		return (*this);
	}
	
	Vector & operator-=(Vector const& _vector)
	{
		gsl_vector_sub(m_vector, _vector.m_vector);
		return (*this);
	}
	
	bool is_zero() const
	{
		return gsl_vector_isnull(m_vector);
	}
	
	bool is_positive() const
	{
		return gsl_vector_ispos(m_vector);
	}
	
	bool is_negative() const
	{
		return gsl_vector_isneg(m_vector);
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
	
	friend
	void swap(Vector & v1, Vector & v2);
	
	size_t size() const
	{
		return m_size;
	}
	
private:
	size_t m_size;
	gsl_vector* m_vector;
};

void swap(Vector & v1, Vector & v2)
{
	assert( v1.size() == v2.size() );
	std::swap(v1.m_vector, v2.m_vector);
}

} } // end namespaces


#endif /*_NL_VECTOR_H_*/
