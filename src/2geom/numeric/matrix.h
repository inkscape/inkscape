#ifndef _NL_MATRIX_H_
#define _NL_MATRIX_H_


#include <cassert>
#include <utility>
#include <algorithm>

#include <gsl/gsl_matrix.h>



namespace Geom { namespace NL {

class Matrix;
void swap(Matrix & m1, Matrix & m2);


class Matrix
{
public:
	// the matrix is not inizialized
	Matrix(size_t n1, size_t n2)
		: m_rows(n1), m_columns(n2)
	{
		m_matrix = gsl_matrix_alloc(n1, n2);
	}
	
	Matrix(size_t n1, size_t n2, double x)
		:m_rows(n1), m_columns(n2)
	{
		m_matrix = gsl_matrix_alloc(n1, n2);
		gsl_matrix_set_all(m_matrix, x);
	}
	
	Matrix( Matrix const& _matrix )
		: m_rows(_matrix.rows()), m_columns(_matrix.columns())
	{
		m_matrix = gsl_matrix_alloc(rows(), columns());
		gsl_matrix_memcpy(m_matrix, _matrix.m_matrix);
	}
	
//	explicit
//	Matrix( gsl_matrix* m, size_t n1, size_t n2)
//		: m_rows(n1), m_columns(n2), m_matrix(m)
//	{	
//	}
	
	Matrix & operator=(Matrix const& _matrix)
	{
		assert( rows() == _matrix.rows() && columns() ==  _matrix.columns() );
		gsl_matrix_memcpy(m_matrix, _matrix.m_matrix);
		
		return *this;
	}
	
	virtual ~Matrix()
	{
		gsl_matrix_free(m_matrix);
	}
	
	void set_all( double x = 0)
	{
		gsl_matrix_set_all(m_matrix, x);
	}
	
	void set_identity()
	{
		gsl_matrix_set_identity(m_matrix);
	}
	
	const double & operator() (size_t i, size_t j) const
	{
		return *gsl_matrix_const_ptr(m_matrix, i, j);
	}
	
	double & operator() (size_t i, size_t j)
	{
		return *gsl_matrix_ptr(m_matrix, i, j);
	}
	
	gsl_matrix* get_gsl_matrix()
	{
		return m_matrix;
	}
	
	void swap_rows(size_t i, size_t j)
	{
		 gsl_matrix_swap_rows(m_matrix, i, j);
	}
	
	void swap_columns(size_t i, size_t j)
	{
		gsl_matrix_swap_columns(m_matrix, i, j);
	}
	
	Matrix & transpose()
	{
		gsl_matrix_transpose(m_matrix);
		return (*this);
	}
	
	Matrix & scale(double x)
	{
		gsl_matrix_scale(m_matrix, x);
		return (*this);
	}
	
	Matrix & translate(double x)
	{
		 gsl_matrix_add_constant(m_matrix, x);
		 return (*this);
	}
	
	Matrix & operator+=(Matrix const& _matrix)
	{
		gsl_matrix_add(m_matrix, _matrix.m_matrix);
		return (*this);
	}
	
	Matrix & operator-=(Matrix const& _matrix)
	{
		gsl_matrix_sub(m_matrix, _matrix.m_matrix);
		return (*this);
	}
	
	bool is_zero() const
	{
		return gsl_matrix_isnull(m_matrix);
	}
	
	bool is_positive() const
	{
		return gsl_matrix_ispos(m_matrix);
	}
	
	bool is_negative() const
	{
		return gsl_matrix_isneg(m_matrix);
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
	
	friend 
	void swap(Matrix & m1, Matrix & m2);
	
	size_t rows() const
	{
		return m_rows;
	}
	
	size_t columns() const
	{
		return m_columns;
	}
	
private:
	size_t m_rows, m_columns;
	gsl_matrix* m_matrix;
};

void swap(Matrix & m1, Matrix & m2)
{
	assert( m1.rows() == m2.rows() && m1.columns() ==  m2.columns() );
	std::swap(m1.m_matrix, m2.m_matrix);
}


} } // end namespaces

#endif /*_NL_MATRIX_H_*/
