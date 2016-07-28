#ifndef SEEN_SYSEQ_H
#define SEEN_SYSEQ_H

/*
 * Auxiliary routines to solve systems of linear equations in several variants and sizes.
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007  Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <vector>
#include "math.h"

namespace SysEq {

enum SolutionKind {
    unique = 0,
    ambiguous,
    no_solution,
    solution_exists // FIXME: remove this; does not yield enough information
};

inline void explain(SolutionKind sol) {
    switch (sol) {
        case SysEq::unique:
            std::cout << "unique" << std::endl;
            break;
        case SysEq::ambiguous:
            std::cout << "ambiguous" << std::endl;
            break;
        case SysEq::no_solution:
            std::cout << "no solution" << std::endl;
            break;
        case SysEq::solution_exists:
            std::cout << "solution exists" << std::endl;
            break;
    }
}

inline double
determinant3x3 (double A[3][3]) {
    return (A[0][0]*A[1][1]*A[2][2] +
            A[0][1]*A[1][2]*A[2][0] +
            A[0][2]*A[1][0]*A[2][1] -
            A[0][0]*A[1][2]*A[2][1] -
            A[0][1]*A[1][0]*A[2][2] -
            A[0][2]*A[1][1]*A[2][0]);
}

/* Determinant of the 3x3 matrix having a, b, and c as columns */
inline double
determinant3v (const double a[3], const double b[3], const double c[3]) {
    return (a[0]*b[1]*c[2] +
            a[1]*b[2]*c[0] +
            a[2]*b[0]*c[1] -
            a[0]*b[2]*c[1] -
            a[1]*b[0]*c[2] -
            a[2]*b[1]*c[0]);
}

/* Copy the elements of A into B */
template <int S, int T>
inline void copy_mat(double A[S][T], double B[S][T]) {
    for (int i = 0; i < S; ++i) {
        for (int j = 0; j < T; ++j) {
            B[i][j] = A[i][j];
        }
    }
}

template <int S, int T>
inline void print_mat (const double A[S][T]) {
    std::cout.setf(std::ios::left, std::ios::internal);
    for (int i = 0; i < S; ++i) {
        for (int j = 0; j < T; ++j) {
            printf ("%8.2f ", A[i][j]);
        }
        std::cout << std::endl;;
    }    
}

/* Multiplication of two matrices */
template <int S, int U, int T>
inline void multiply(double A[S][U], double B[U][T], double res[S][T]) {
    for (int i = 0; i < S; ++i) {
        for (int j = 0; j < T; ++j) {
            double sum = 0;
            for (int k = 0; k < U; ++k) {
                sum += A[i][k] * B[k][j];
            }
            res[i][j] = sum;
        }
    }
}

/*
 * Multiplication of a matrix with a vector (for convenience, because with the previous
 * multiplication function we would always have to write v[i][0] for elements of the vector.
 */
template <int S, int T>
inline void multiply(double A[S][T], double v[T], double res[S]) {
    for (int i = 0; i < S; ++i) {
        double sum = 0;
        for (int k = 0; k < T; ++k) {
            sum += A[i][k] * v[k];
        }
        res[i] = sum;
    }
}

// Remark: Since we are using templates, we cannot separate declarations from definitions (which would
//         result in linker errors but have to include the definitions here for the following functions.
// FIXME: Maybe we should rework all this by using vector<vector<double> > structures for matrices
//        instead of double[S][T]. This would allow us to avoid templates. Would the performance degrade?

/*
 * Find the element of maximal absolute value in row i that 
 * does not lie in one of the columns given in avoid_cols.
 */
template <int S, int T>
static int find_pivot(const double A[S][T], unsigned int i, std::vector<int> const &avoid_cols) {
    if (i >= S) {
        return -1;
    }
    int pos = -1;
    double max = 0;
    for (int j = 0; j < T; ++j) {
        if (std::find(avoid_cols.begin(), avoid_cols.end(), j) != avoid_cols.end()) {
            continue; // skip "forbidden" columns
        }
        if (fabs(A[i][j]) > max) {
            pos = j;
            max = fabs(A[i][j]);
        }
    }
    return pos;
}

/*
 * Performs a single 'exchange step' in the Gauss-Jordan algorithm (i.e., swapping variables in the
 * two vectors).
 */
template <int S, int T>
static void gauss_jordan_step (double A[S][T], int row, int col) {
    double piv = A[row][col]; // pivot element
    /* adapt the entries of the matrix, first outside the pivot row/column */
    for (int k = 0; k < S; ++k) {
        if (k == row) continue;
        for (int l = 0; l < T; ++l) {
            if (l == col) continue;
            A[k][l] -= A[k][col] * A[row][l] / piv;
        }
    }
    /* now adapt the pivot column ... */
    for (int k = 0; k < S; ++k) {
        if (k == row) continue;
        A[k][col]  /= piv;
    }
    /* and the pivot row */
    for (int l = 0; l < T; ++l) {
        if (l == col) continue;
        A[row][l]  /= -piv;
    }
    /* finally, set the element at the pivot position itself */
    A[row][col] = 1/piv;
}

/*
 * Perform Gauss-Jordan elimination on the matrix A, optionally avoiding a given column during pivot search
 */
template <int S, int T>
static std::vector<int> gauss_jordan (double A[S][T], int avoid_col = -1) {
    std::vector<int> cols_used;
    if (avoid_col != -1) {
        cols_used.push_back (avoid_col);
    }
    for (int i = 0; i < S; ++i) {
        /* for each row find a pivot element of maximal absolute value, skipping the columns that were used before */
        int col = find_pivot<S,T>(A, i, cols_used);
        cols_used.push_back(col);
        if (col == -1) {
            // no non-zero elements in the row
            return cols_used;
        }

        /* if pivot search was successful we can perform a Gauss-Jordan step */
        gauss_jordan_step<S,T> (A, i, col);
    }
    if (avoid_col != -1) {
        // since the columns that were used will be needed later on, we need to clean up the column vector
        cols_used.erase(cols_used.begin());
    }
    return cols_used;
}

/* compute the modified value that x[index] needs to assume so that in the end we have x[index]/x[T-1] = val */
template <int S, int T>
static double projectify (std::vector<int> const &cols, const double B[S][T], const double x[T],
                          const int index, const double val) {
    double val_proj = 0.0;
    if (index != -1) {
        int c = -1;
        for (int i = 0; i < S; ++i) {
            if (cols[i] == T-1) {
                c = i;
                break;
            }
        }
        if (c == -1) {
            std::cout << "Something is wrong. Rethink!!" << std::endl;
            return SysEq::no_solution;
        }

        double sp = 0;
        for (int j = 0; j < T; ++j) {
            if (j == index) continue;
            sp += B[c][j] * x[j];
        }
        double mu = 1 - val * B[c][index];
        if (fabs(mu) < 1E-6) {
            std::cout << "No solution since adapted value is too close to zero" << std::endl;
            return SysEq::no_solution;
        }
        val_proj = sp*val/mu;
    } else {
        val_proj = val; // FIXME: Is this correct?
    }
    return val_proj;
}

/**
 * Solve the linear system of equations \a A * \a x = \a v where we additionally stipulate
 * \a x[\a index] = \a val if \a index is not -1. The system is solved using Gauss-Jordan
 * elimination so that we can gracefully handle the case that zero or infinitely many
 * solutions exist.
 *
 * Since our application will be to finding preimages of projective mappings, we provide
 * an additional argument \a proj. If this is true, we find a solution of
 * \a x[\a index]/\a x[\T - 1] = \a val insted (i.e., we want the corresponding coordinate
 * of the _affine image_ of the point with homogeneous coordinate vector \a x to be equal
 * to \a val.
 *
 * Remark: We don't need this but it would be relatively simple to let the calling function
 * prescripe the value of _multiple_ components of the solution vector instead of only a single one.
 */
template <int S, int T> SolutionKind gaussjord_solve (double A[S][T], double x[T], double v[S],
                                                      int index = -1, double val = 0.0, bool proj = false) {
    double B[S][T];
    //copy_mat<S,T>(A,B);
    SysEq::copy_mat<S,T>(A,B);
    std::vector<int> cols = gauss_jordan<S,T>(B, index);
    if (std::find(cols.begin(), cols.end(), -1) != cols.end()) {
        // pivot search failed for some row so the system is not solvable
        return SysEq::no_solution;
    }

    /* the vector x is filled with the coefficients of the desired solution vector at appropriate places;
     * the other components are set to zero, and we additionally set x[index] = val if applicable
     */
    std::vector<int>::iterator k;
    for (int j = 0; j < S; ++j) {
        x[cols[j]] = v[j];
    }
    for (int j = 0; j < T; ++j) {
        k = std::find(cols.begin(), cols.end(), j);
        if (k == cols.end()) {
            x[j] = 0;
        }
    }

    // we need to adapt the value if we we are in the "projective case" (see above)
    double val_new = (proj ? projectify<S,T>(cols, B, x, index, val) : val);

    if (index >= 0 && index < T) {
        // we want the specified coefficient of the solution vector to have a given value
        x[index] = val_new;
    }

    /* the final solution vector is now obtained as the product B*x, where B is the matrix
     * obtained by Gauss-Jordan manipulation of A; we use w as an auxiliary vector and
     * afterwards copy the result back to x
     */
    double w[S];
    SysEq::multiply<S,T>(B,x,w); // initializes w
    for (int j = 0; j < S; ++j) {
        x[cols[j]] = w[j];
    }

    if (S + (index == -1 ? 0 : 1) == T) {
        return SysEq::unique;
    } else {
        return SysEq::ambiguous;
    }
}

} // namespace SysEq

#endif /* __SYSEQ_H__ */

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
