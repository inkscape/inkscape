/*
 * OpenDocument <drawing> input and output
 *
 * This is an an entry in the extensions mechanism to begin to enable
 * the inputting and outputting of OpenDocument Format (ODF) files from
 * within Inkscape.  Although the initial implementations will be very lossy
 * due to the differences in the models of SVG and ODF, they will hopefully
 * improve greatly with time.  People should consider this to be a framework
 * that can be continously upgraded for ever improving fidelity.  Potential
 * developers should especially look in preprocess() and writeTree() to see how
 * the SVG tree is scanned, read, translated, and then written to ODF.
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 *
 * Authors:
 *   Bob Jamison
 *   Abhishek Sharma
 *   Kris De Gussem
 *
 * Copyright (C) 2006, 2007 Bob Jamison
 * Copyright (C) 2013 Kris De Gussem
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
 */



#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "odf.h"

//# System includes
#include <stdio.h>
#include <time.h>
#include <vector>
#include <math.h>

//# Inkscape includes
#include "clear-n_.h"
#include "inkscape.h"
#include <style.h>
#include "display/curve.h"
#include <2geom/pathvector.h>
#include <2geom/curves.h>
#include <2geom/transforms.h>
#include <helper/geom.h>
#include "helper/geom-curves.h"
#include "extension/system.h"

#include "xml/repr.h"
#include "xml/attribute-record.h"
#include "sp-image.h"
#include "sp-gradient.h"
#include "sp-stop.h"
#include "gradient-chemistry.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "sp-path.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "svg/svg.h"
#include "text-editing.h"
#include "util/units.h"

#include "uri.h"

#include "inkscape-version.h"
#include "document.h"
#include "extension/extension.h"

#include "io/inkscapestream.h"
#include "io/bufferstream.h"
#include <util/ziptool.h>
#include <iomanip>
namespace Inkscape
{
namespace Extension
{
namespace Internal
{
//# Shorthand notation
typedef Inkscape::IO::BufferOutputStream BufferOutputStream;
typedef Inkscape::IO::OutputStreamWriter OutputStreamWriter;
typedef Inkscape::IO::StringOutputStream StringOutputStream;


//########################################################################
//# C L A S S    SingularValueDecomposition
//########################################################################
#include <math.h>

class SVDMatrix
{
public:

    SVDMatrix()
        {
        init();
        }

    SVDMatrix(unsigned int rowSize, unsigned int colSize)
        {
        init();
        rows = rowSize;
        cols = colSize;
        size = rows * cols;
        d    = new double[size];
        for (unsigned int i=0 ; i<size ; i++)
            d[i] = 0.0;
        }

    SVDMatrix(double *vals, unsigned int rowSize, unsigned int colSize)
        {
        init();
        rows = rowSize;
        cols = colSize;
        size = rows * cols;
        d    = new double[size];
        for (unsigned int i=0 ; i<size ; i++)
            d[i] = vals[i];
        }


    SVDMatrix(const SVDMatrix &other)
        {
        init();
        assign(other);
        }

    SVDMatrix &operator=(const SVDMatrix &other)
        {
        assign(other);
        return *this;
        }

    virtual ~SVDMatrix()
        {
        delete[] d;
        }

     double& operator() (unsigned int row, unsigned int col)
         {
         if (row >= rows || col >= cols)
             return badval;
         return d[cols*row + col];
         }

     double operator() (unsigned int row, unsigned int col) const
         {
         if (row >= rows || col >= cols)
             return badval;
         return d[cols*row + col];
         }

     unsigned int getRows()
         {
         return rows;
         }

     unsigned int getCols()
         {
         return cols;
         }

     SVDMatrix multiply(const SVDMatrix &other)
         {
         if (cols != other.rows)
             {
             SVDMatrix dummy;
             return dummy;
             }
         SVDMatrix result(rows, other.cols);
         for (unsigned int i=0 ; i<rows ; i++)
             {
             for (unsigned int j=0 ; j<other.cols ; j++)
             {
                 double sum = 0.0;
                 for (unsigned int k=0 ; k<cols ; k++)
                     {
                     //sum += a[i][k] * b[k][j];
                     sum += d[i*cols +k] * other(k, j);
                     }
                 result(i, j) = sum;
                 }

             }
         return result;
         }

     SVDMatrix transpose()
         {
         SVDMatrix result(cols, rows);
         for (unsigned int i=0 ; i<rows ; i++){
             for (unsigned int j=0 ; j<cols ; j++){
                 result(j, i) = d[i*cols + j];
             }
         }
         return result;
         }

private:


    virtual void init()
        {
        badval = 0.0;
        d      = NULL;
        rows   = 0;
        cols   = 0;
        size   = 0;
        }

     void assign(const SVDMatrix &other)
        {
        if (d)
            {
            delete[] d;
            d = 0;
            }
        rows = other.rows;
        cols = other.cols;
        size = other.size;
        badval = other.badval;
        d = new double[size];
        for (unsigned int i=0 ; i<size ; i++){
            d[i] = other.d[i];
            }
        }

    double badval;

    double *d;
    unsigned int rows;
    unsigned int cols;
    unsigned int size;
};



/**
 *
 * ====================================================
 *
 * NOTE:
 * This class is ported almost verbatim from the public domain
 * JAMA Matrix package.  It is modified to handle only 3x3 matrices
 * and our Geom::Affine affine transform class.  We give full
 * attribution to them, along with many thanks.  JAMA can be found at:
 *     http://math.nist.gov/javanumerics/jama
 *
 * ====================================================
 *
 * Singular Value Decomposition.
 * <P>
 * For an m-by-n matrix A with m >= n, the singular value decomposition is
 * an m-by-n orthogonal matrix U, an n-by-n diagonal matrix S, and
 * an n-by-n orthogonal matrix V so that A = U*S*V'.
 * <P>
 * The singular values, sigma[k] = S[k][k], are ordered so that
 * sigma[0] >= sigma[1] >= ... >= sigma[n-1].
 * <P>
 * The singular value decompostion always exists, so the constructor will
 * never fail.  The matrix condition number and the effective numerical
 * rank can be computed from this decomposition.
 */
class SingularValueDecomposition
{
public:

   /** Construct the singular value decomposition
   @param A    Rectangular matrix
   @return     Structure to access U, S and V.
   */

    SingularValueDecomposition (const SVDMatrix &mat) :
        A (mat),
        U (),
        s (NULL),
        s_size (0),
        V ()
        {
        calculate();
        }

    virtual ~SingularValueDecomposition()
        {
        delete[] s;
        }

    /**
     * Return the left singular vectors
     * @return     U
     */
    SVDMatrix &getU();

    /**
     * Return the right singular vectors
     * @return     V
     */
    SVDMatrix &getV();

    /**
     *  Return the s[index] value
     */    double getS(unsigned int index);

    /**
     * Two norm
     * @return max(S)
     */
    double norm2();

    /**
     * Two norm condition number
     *  @return max(S)/min(S)
     */
    double cond();

    /**
     *  Effective numerical matrix rank
     *  @return     Number of nonnegligible singular values.
     */
    int rank();

private:

      void calculate();

      SVDMatrix A;
      SVDMatrix U;
      double *s;
      unsigned int s_size;
      SVDMatrix V;

};


static double svd_hypot(double a, double b)
{
    double r;

    if (fabs(a) > fabs(b))
        {
        r = b/a;
        r = fabs(a) * sqrt(1+r*r);
        }
    else if (b != 0)
        {
        r = a/b;
        r = fabs(b) * sqrt(1+r*r);
        }
    else
        {
        r = 0.0;
        }
    return r;
}



void SingularValueDecomposition::calculate()
{
      // Initialize.
      int m = A.getRows();
      int n = A.getCols();

      int nu = (m > n) ? m : n;
      s_size = (m+1 < n) ? m+1 : n;
      s = new double[s_size];
      U = SVDMatrix(m, nu);
      V = SVDMatrix(n, n);
      double *e = new double[n];
      double *work = new double[m];
      bool wantu = true;
      bool wantv = true;

      // Reduce A to bidiagonal form, storing the diagonal elements
      // in s and the super-diagonal elements in e.

      int nct = (m-1<n) ? m-1 : n;
      int nrtx = (n-2<m) ? n-2 : m;
      int nrt = (nrtx>0) ? nrtx : 0;
      for (int k = 0; k < 2; k++) {
         if (k < nct) {

            // Compute the transformation for the k-th column and
            // place the k-th diagonal in s[k].
            // Compute 2-norm of k-th column without under/overflow.
            s[k] = 0;
            for (int i = k; i < m; i++) {
               s[k] = svd_hypot(s[k],A(i, k));
            }
            if (s[k] != 0.0) {
               if (A(k, k) < 0.0) {
                  s[k] = -s[k];
               }
               for (int i = k; i < m; i++) {
                  A(i, k) /= s[k];
               }
               A(k, k) += 1.0;
            }
            s[k] = -s[k];
         }
         for (int j = k+1; j < n; j++) {
            if ((k < nct) & (s[k] != 0.0))  {

            // Apply the transformation.

               double t = 0;
               for (int i = k; i < m; i++) {
                  t += A(i, k) * A(i, j);
               }
               t = -t/A(k, k);
               for (int i = k; i < m; i++) {
                  A(i, j) += t*A(i, k);
               }
            }

            // Place the k-th row of A into e for the
            // subsequent calculation of the row transformation.

            e[j] = A(k, j);
         }
         if (wantu & (k < nct)) {

            // Place the transformation in U for subsequent back
            // multiplication.

            for (int i = k; i < m; i++) {
               U(i, k) = A(i, k);
            }
         }
         if (k < nrt) {

            // Compute the k-th row transformation and place the
            // k-th super-diagonal in e[k].
            // Compute 2-norm without under/overflow.
            e[k] = 0;
            for (int i = k+1; i < n; i++) {
               e[k] = svd_hypot(e[k],e[i]);
            }
            if (e[k] != 0.0) {
               if (e[k+1] < 0.0) {
                  e[k] = -e[k];
               }
               for (int i = k+1; i < n; i++) {
                  e[i] /= e[k];
               }
               e[k+1] += 1.0;
            }
            e[k] = -e[k];
            if ((k+1 < m) & (e[k] != 0.0)) {

            // Apply the transformation.

               for (int i = k+1; i < m; i++) {
                  work[i] = 0.0;
               }
               for (int j = k+1; j < n; j++) {
                  for (int i = k+1; i < m; i++) {
                     work[i] += e[j]*A(i, j);
                  }
               }
               for (int j = k+1; j < n; j++) {
                  double t = -e[j]/e[k+1];
                  for (int i = k+1; i < m; i++) {
                     A(i, j) += t*work[i];
                  }
               }
            }
            if (wantv) {

            // Place the transformation in V for subsequent
            // back multiplication.

               for (int i = k+1; i < n; i++) {
                  V(i, k) = e[i];
               }
            }
         }
      }

      // Set up the final bidiagonal matrix or order p.

      int p = (n < m+1) ? n : m+1;
      if (nct < n) {
         s[nct] = A(nct, nct);
      }
      if (m < p) {
         s[p-1] = 0.0;
      }
      if (nrt+1 < p) {
         e[nrt] = A(nrt, p-1);
      }
      e[p-1] = 0.0;

      // If required, generate U.

      if (wantu) {
         for (int j = nct; j < nu; j++) {
            for (int i = 0; i < m; i++) {
               U(i, j) = 0.0;
            }
            U(j, j) = 1.0;
         }
         for (int k = nct-1; k >= 0; k--) {
            if (s[k] != 0.0) {
               for (int j = k+1; j < nu; j++) {
                  double t = 0;
                  for (int i = k; i < m; i++) {
                     t += U(i, k)*U(i, j);
                  }
                  t = -t/U(k, k);
                  for (int i = k; i < m; i++) {
                     U(i, j) += t*U(i, k);
                  }
               }
               for (int i = k; i < m; i++ ) {
                  U(i, k) = -U(i, k);
               }
               U(k, k) = 1.0 + U(k, k);
               for (int i = 0; i < k-1; i++) {
                  U(i, k) = 0.0;
               }
            } else {
               for (int i = 0; i < m; i++) {
                  U(i, k) = 0.0;
               }
               U(k, k) = 1.0;
            }
         }
      }

      // If required, generate V.

      if (wantv) {
         for (int k = n-1; k >= 0; k--) {
            if ((k < nrt) & (e[k] != 0.0)) {
               for (int j = k+1; j < nu; j++) {
                  double t = 0;
                  for (int i = k+1; i < n; i++) {
                     t += V(i, k)*V(i, j);
                  }
                  t = -t/V(k+1, k);
                  for (int i = k+1; i < n; i++) {
                     V(i, j) += t*V(i, k);
                  }
               }
            }
            for (int i = 0; i < n; i++) {
               V(i, k) = 0.0;
            }
            V(k, k) = 1.0;
         }
      }

      // Main iteration loop for the singular values.

      int pp = p-1;
      //double eps = pow(2.0,-52.0);
      //double tiny = pow(2.0,-966.0);
      //let's just calculate these now
      //a double can be e ± 308.25, so this is safe
      double eps = 2.22e-16;
      double tiny = 1.6e-291;
      while (p > 0) {
         int k,kase;

         // Here is where a test for too many iterations would go.

         // This section of the program inspects for
         // negligible elements in the s and e arrays.  On
         // completion the variables kase and k are set as follows.

         // kase = 1     if s(p) and e[k-1] are negligible and k<p
         // kase = 2     if s(k) is negligible and k<p
         // kase = 3     if e[k-1] is negligible, k<p, and
         //              s(k), ..., s(p) are not negligible (qr step).
         // kase = 4     if e(p-1) is negligible (convergence).

         for (k = p-2; k >= -1; k--) {
            if (k == -1) {
               break;
            }
            if (fabs(e[k]) <=
                  tiny + eps*(fabs(s[k]) + fabs(s[k+1]))) {
               e[k] = 0.0;
               break;
            }
         }
         if (k == p-2) {
            kase = 4;
         } else {
            int ks;
            for (ks = p-1; ks >= k; ks--) {
               if (ks == k) {
                  break;
               }
               double t = (ks != p ? fabs(e[ks]) : 0.) +
                          (ks != k+1 ? fabs(e[ks-1]) : 0.);
               if (fabs(s[ks]) <= tiny + eps*t)  {
                  s[ks] = 0.0;
                  break;
               }
            }
            if (ks == k) {
               kase = 3;
            } else if (ks == p-1) {
               kase = 1;
            } else {
               kase = 2;
               k = ks;
            }
         }
         k++;

         // Perform the task indicated by kase.

         switch (kase) {

            // Deflate negligible s(p).

            case 1: {
               double f = e[p-2];
               e[p-2] = 0.0;
               for (int j = p-2; j >= k; j--) {
                  double t = svd_hypot(s[j],f);
                  double cs = s[j]/t;
                  double sn = f/t;
                  s[j] = t;
                  if (j != k) {
                     f = -sn*e[j-1];
                     e[j-1] = cs*e[j-1];
                  }
                  if (wantv) {
                     for (int i = 0; i < n; i++) {
                        t = cs*V(i, j) + sn*V(i, p-1);
                        V(i, p-1) = -sn*V(i, j) + cs*V(i, p-1);
                        V(i, j) = t;
                     }
                  }
               }
            }
            break;

            // Split at negligible s(k).

            case 2: {
               double f = e[k-1];
               e[k-1] = 0.0;
               for (int j = k; j < p; j++) {
                  double t = svd_hypot(s[j],f);
                  double cs = s[j]/t;
                  double sn = f/t;
                  s[j] = t;
                  f = -sn*e[j];
                  e[j] = cs*e[j];
                  if (wantu) {
                     for (int i = 0; i < m; i++) {
                        t = cs*U(i, j) + sn*U(i, k-1);
                        U(i, k-1) = -sn*U(i, j) + cs*U(i, k-1);
                        U(i, j) = t;
                     }
                  }
               }
            }
            break;

            // Perform one qr step.

            case 3: {

               // Calculate the shift.

               double scale = fabs(s[p-1]);
               double d = fabs(s[p-2]);
               if (d>scale) scale=d;
               d = fabs(e[p-2]);
               if (d>scale) scale=d;
               d = fabs(s[k]);
               if (d>scale) scale=d;
               d = fabs(e[k]);
               if (d>scale) scale=d;
               double sp = s[p-1]/scale;
               double spm1 = s[p-2]/scale;
               double epm1 = e[p-2]/scale;
               double sk = s[k]/scale;
               double ek = e[k]/scale;
               double b = ((spm1 + sp)*(spm1 - sp) + epm1*epm1)/2.0;
               double c = (sp*epm1)*(sp*epm1);
               double shift = 0.0;
               if ((b != 0.0) | (c != 0.0)) {
                  shift = sqrt(b*b + c);
                  if (b < 0.0) {
                     shift = -shift;
                  }
                  shift = c/(b + shift);
               }
               double f = (sk + sp)*(sk - sp) + shift;
               double g = sk*ek;

               // Chase zeros.

               for (int j = k; j < p-1; j++) {
                  double t = svd_hypot(f,g);
                  double cs = f/t;
                  double sn = g/t;
                  if (j != k) {
                     e[j-1] = t;
                  }
                  f = cs*s[j] + sn*e[j];
                  e[j] = cs*e[j] - sn*s[j];
                  g = sn*s[j+1];
                  s[j+1] = cs*s[j+1];
                  if (wantv) {
                     for (int i = 0; i < n; i++) {
                        t = cs*V(i, j) + sn*V(i, j+1);
                        V(i, j+1) = -sn*V(i, j) + cs*V(i, j+1);
                        V(i, j) = t;
                     }
                  }
                  t = svd_hypot(f,g);
                  cs = f/t;
                  sn = g/t;
                  s[j] = t;
                  f = cs*e[j] + sn*s[j+1];
                  s[j+1] = -sn*e[j] + cs*s[j+1];
                  g = sn*e[j+1];
                  e[j+1] = cs*e[j+1];
                  if (wantu && (j < m-1)) {
                     for (int i = 0; i < m; i++) {
                        t = cs*U(i, j) + sn*U(i, j+1);
                        U(i, j+1) = -sn*U(i, j) + cs*U(i, j+1);
                        U(i, j) = t;
                     }
                  }
               }
               e[p-2] = f;
            }
            break;

            // Convergence.

            case 4: {

               // Make the singular values positive.

               if (s[k] <= 0.0) {
                  s[k] = (s[k] < 0.0 ? -s[k] : 0.0);
                  if (wantv) {
                     for (int i = 0; i <= pp; i++) {
                        V(i, k) = -V(i, k);
                     }
                  }
               }

               // Order the singular values.

               while (k < pp) {
                  if (s[k] >= s[k+1]) {
                     break;
                  }
                  double t = s[k];
                  s[k] = s[k+1];
                  s[k+1] = t;
                  if (wantv && (k < n-1)) {
                     for (int i = 0; i < n; i++) {
                        t = V(i, k+1); V(i, k+1) = V(i, k); V(i, k) = t;
                     }
                  }
                  if (wantu && (k < m-1)) {
                     for (int i = 0; i < m; i++) {
                        t = U(i, k+1); U(i, k+1) = U(i, k); U(i, k) = t;
                     }
                  }
                  k++;
               }
               p--;
            }
            break;
         }
      }

    delete [] e;
    delete [] work;

}


/**
 * Return the left singular vectors
 * @return     U
 */
SVDMatrix &SingularValueDecomposition::getU()
{
    return U;
}

/**
 * Return the right singular vectors
 * @return     V
 */

SVDMatrix &SingularValueDecomposition::getV()
{
    return V;
}

/**
 *  Return the s[0] value
 */
double SingularValueDecomposition::getS(unsigned int index)
{
    if (index >= s_size)
        return 0.0;
    return s[index];
}

/**
 * Two norm
 * @return     max(S)
 */
double SingularValueDecomposition::norm2()
{
    return s[0];
}

/**
 * Two norm condition number
 *  @return     max(S)/min(S)
 */

double SingularValueDecomposition::cond()
{
    return s[0]/s[2];
}

/**
 *  Effective numerical matrix rank
 *  @return     Number of nonnegligible singular values.
 */
int SingularValueDecomposition::rank()
{
    double eps = pow(2.0,-52.0);
    double tol = 3.0*s[0]*eps;
    int r = 0;
    for (int i = 0; i < 3; i++)
        {
        if (s[i] > tol)
            r++;
        }
    return r;
}

//########################################################################
//# E N D    C L A S S    SingularValueDecomposition
//########################################################################





#define pi 3.14159
//#define pxToCm  0.0275
#define pxToCm  0.03
#define piToRad 0.0174532925
#define docHeightCm 22.86


//########################################################################
//# O U T P U T
//########################################################################

/**
 * Get the value of a node/attribute pair
 */
static Glib::ustring getAttribute( Inkscape::XML::Node *node, char const *attrName)
{
    Glib::ustring val;
    char const *valstr = node->attribute(attrName);
    if (valstr)
        val = valstr;
    return val;
}


/**
 * Get the extension suffix from the end of a file name
 */
static Glib::ustring getExtension(const Glib::ustring &fname)
{
    Glib::ustring ext;

    std::string::size_type pos = fname.rfind('.');
    if (pos == fname.npos)
        {
        ext = "";
        }
    else
        {
        ext = fname.substr(pos);
        }
    return ext;
}

static Glib::ustring formatTransform(Geom::Affine &tf)
{
    Glib::ustring str;
    if (!tf.isIdentity())
        {
        StringOutputStream outs;
        OutputStreamWriter out(outs);
        out.printf("matrix(%.3f %.3f %.3f %.3f %.3f %.3f)",
                tf[0], tf[1], tf[2], tf[3], tf[4], tf[5]);
        str = outs.getString();
        }
    return str;
}


/**
 * Get the general transform from SVG pixels to
 * ODF cm
 */
static Geom::Affine getODFTransform(const SPItem *item)
{
    //### Get SVG-to-ODF transform
    Geom::Affine tf (item->i2dt_affine());
    //Flip Y into document coordinates
    double doc_height    = SP_ACTIVE_DOCUMENT->getHeight().value("px");
    Geom::Affine doc2dt_tf = Geom::Affine(Geom::Scale(1.0, -1.0));                    /// @fixme hardcoded desktop transform
    doc2dt_tf            = doc2dt_tf * Geom::Affine(Geom::Translate(0, doc_height));
    tf                   = tf * doc2dt_tf;
    tf                   = tf * Geom::Affine(Geom::Scale(pxToCm));
    return tf;
}


/**
 * Get the bounding box of an item, as mapped onto
 * an ODF document, in cm.
 */
static Geom::OptRect getODFBoundingBox(const SPItem *item)
{
    // TODO: geometric or visual?
    Geom::OptRect bbox = item->documentVisualBounds();
    if (bbox) {
        *bbox *= Geom::Affine(Geom::Scale(pxToCm));
    }
    return bbox;
}


/**
 * Get the transform for an item, correcting for
 * handedness reversal
 */
static Geom::Affine getODFItemTransform(const SPItem *item)
{
    Geom::Affine itemTransform (Geom::Scale(1, -1));  /// @fixme hardcoded doc2dt transform?
    itemTransform = itemTransform * (Geom::Affine)item->transform;
    itemTransform = itemTransform * Geom::Scale(1, -1);
    return itemTransform;
}


/**
 * Get some fun facts from the transform
 */
static void analyzeTransform(Geom::Affine &tf,
                             double &rotate, double &/*xskew*/, double &/*yskew*/,
                             double &xscale, double &yscale)
{
    SVDMatrix mat(2, 2);
    mat(0, 0) = tf[0];
    mat(0, 1) = tf[1];
    mat(1, 0) = tf[2];
    mat(1, 1) = tf[3];

    SingularValueDecomposition svd(mat);

    SVDMatrix U = svd.getU();
    SVDMatrix V = svd.getV();
    SVDMatrix Vt = V.transpose();
    SVDMatrix UVt = U.multiply(Vt);
    double s0 = svd.getS(0);
    double s1 = svd.getS(1);
    xscale = s0;
    yscale = s1;
    rotate = UVt(0,0);
}

static void gatherText(Inkscape::XML::Node *node, Glib::ustring &buf)
{
    if (node->type() == Inkscape::XML::TEXT_NODE)
        {
        char *s = (char *)node->content();
        if (s)
            buf.append(s);
        }

    for (Inkscape::XML::Node *child = node->firstChild() ;
                child != NULL; child = child->next())
        {
        gatherText(child, buf);
        }

}


/**
 * FIRST PASS.
 * Method descends into the repr tree, converting image, style, and gradient info
 * into forms compatible in ODF.
 */
void OdfOutput::preprocess(ZipFile &zf, Inkscape::XML::Node *node)
{
    Glib::ustring nodeName = node->name();
    Glib::ustring id       = getAttribute(node, "id");

    //### First, check for metadata
    if (nodeName == "metadata" || nodeName == "svg:metadata")
        {
        Inkscape::XML::Node *mchild = node->firstChild() ;
        if (!mchild || strcmp(mchild->name(), "rdf:RDF"))
            return;
        Inkscape::XML::Node *rchild = mchild->firstChild() ;
        if (!rchild || strcmp(rchild->name(), "cc:Work"))
            return;
        for (Inkscape::XML::Node *cchild = rchild->firstChild() ;
            cchild ; cchild = cchild->next())
            {
            Glib::ustring ccName = cchild->name();
            Glib::ustring ccVal;
            gatherText(cchild, ccVal);
            //g_message("ccName: %s  ccVal:%s", ccName.c_str(), ccVal.c_str());
            metadata[ccName] = ccVal;
            }
        return;
        }

    //Now consider items.
    SPObject *reprobj = SP_ACTIVE_DOCUMENT->getObjectByRepr(node);
    if (!reprobj)
    {
        return;
    }
    if (!SP_IS_ITEM(reprobj))
    {
        return;
    }

    if (nodeName == "image" || nodeName == "svg:image")
        {
        Glib::ustring href = getAttribute(node, "xlink:href");
        if (href.size() > 0)
            {
            Glib::ustring oldName = href;
            Glib::ustring ext = getExtension(oldName);
            if (ext == ".jpeg")
                ext = ".jpg";
            if (imageTable.find(oldName) == imageTable.end())
                {
                char buf[64];
                snprintf(buf, sizeof(buf), "Pictures/image%u%s",
                         static_cast<unsigned int>(imageTable.size()), ext.c_str());
                Glib::ustring newName = buf;
                imageTable[oldName] = newName;
                Glib::ustring comment = "old name was: ";
                comment.append(oldName);
                Inkscape::URI oldUri(oldName.c_str());
                //# if relative to the documentURI, get proper path
                std::string pathName = documentUri.getFullPath(oldUri.getFullPath(""));
                ZipEntry *ze = zf.addFile(pathName, comment);
                if (ze)
                    {
                    ze->setFileName(newName);
                    }
                else
                    {
                    g_warning("Could not load image file '%s'", pathName.c_str());
                    }
                }
            }
        }

    for (Inkscape::XML::Node *child = node->firstChild() ;
            child ; child = child->next())
        preprocess(zf, child);
}


/**
 * Writes the manifest.  Currently it only changes according to the
 * file names of images packed into the zip file.
 */
bool OdfOutput::writeManifest(ZipFile &zf)
{
    BufferOutputStream bouts;
    OutputStreamWriter outs(bouts);

    time_t tim;
    time(&tim);

    outs.writeString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    outs.writeString("<!DOCTYPE manifest:manifest PUBLIC \"-//OpenOffice.org//DTD Manifest 1.0//EN\" \"Manifest.dtd\">\n");
    outs.writeString("\n");
    outs.writeString("\n");
    outs.writeString("<!--\n");
    outs.writeString("*************************************************************************\n");
    outs.writeString("  file:  manifest.xml\n");
    outs.printf     ("  Generated by Inkscape: %s", ctime(&tim)); //ctime has its own <cr>
    outs.writeString("  http://www.inkscape.org\n");
    outs.writeString("*************************************************************************\n");
    outs.writeString("-->\n");
    outs.writeString("\n");
    outs.writeString("\n");
    outs.writeString("<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\">\n");
    outs.writeString("    <manifest:file-entry manifest:media-type=\"application/vnd.oasis.opendocument.graphics\" manifest:full-path=\"/\"/>\n");
    outs.writeString("    <manifest:file-entry manifest:media-type=\"text/xml\" manifest:full-path=\"content.xml\"/>\n");
    outs.writeString("    <manifest:file-entry manifest:media-type=\"text/xml\" manifest:full-path=\"styles.xml\"/>\n");
    outs.writeString("    <manifest:file-entry manifest:media-type=\"text/xml\" manifest:full-path=\"meta.xml\"/>\n");
    outs.writeString("    <!--List our images here-->\n");
    std::map<Glib::ustring, Glib::ustring>::iterator iter;
    for (iter = imageTable.begin() ; iter!=imageTable.end() ; ++iter)
        {
        Glib::ustring oldName = iter->first;
        Glib::ustring newName = iter->second;

        Glib::ustring ext = getExtension(oldName);
        if (ext == ".jpeg")
            ext = ".jpg";
        outs.printf("    <manifest:file-entry manifest:media-type=\"");
        if (ext == ".gif")
            outs.printf("image/gif");
        else if (ext == ".png")
            outs.printf("image/png");
        else if (ext == ".jpg")
            outs.printf("image/jpeg");
        outs.printf("\" manifest:full-path=\"");
        outs.writeString(newName.c_str());
        outs.printf("\"/>\n");
        }
    outs.printf("</manifest:manifest>\n");

    outs.close();

    //Make our entry
    ZipEntry *ze = zf.newEntry("META-INF/manifest.xml", "ODF file manifest");
    ze->setUncompressedData(bouts.getBuffer());
    ze->finish();

    return true;
}


/**
 * This writes the document meta information to meta.xml
 */
bool OdfOutput::writeMeta(ZipFile &zf)
{
    BufferOutputStream bouts;
    OutputStreamWriter outs(bouts);

    time_t tim;
    time(&tim);

    std::map<Glib::ustring, Glib::ustring>::iterator iter;
    Glib::ustring InkscapeVersion = Glib::ustring("Inkscape.org - ") + Inkscape::version_string;
    Glib::ustring creator = InkscapeVersion;
    iter = metadata.find("dc:creator");
    if (iter != metadata.end())
    {
        creator = iter->second;
    }
    
    Glib::ustring date;
    Glib::ustring moddate;
    char buf [80];
    time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime (buf,80,"%Y-%m-%d %H:%M:%S",timeinfo);
    moddate = Glib::ustring(buf);
    
    iter = metadata.find("dc:date");
    if (iter != metadata.end())
    {
        date = iter->second;
    }
    else
    {
        date = moddate;
    }

    outs.writeString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    outs.writeString("\n");
    outs.writeString("<!--\n");
    outs.writeString("*************************************************************************\n");
    outs.writeString("  file:  meta.xml\n");
    outs.printf     ("  Generated by Inkscape: %s", ctime(&tim)); //ctime has its own <cr>
    outs.writeString("  http://www.inkscape.org\n");
    outs.writeString("*************************************************************************\n");
    outs.writeString("-->\n");
    outs.writeString("\n");
    outs.writeString("<office:document-meta\n");
    outs.writeString("xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"\n");
    outs.writeString("xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n");
    outs.writeString("xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n");
    outs.writeString("xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\"\n");
    outs.writeString("xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\"\n");
    outs.writeString("xmlns:ooo=\"http://openoffice.org/2004/office\"\n");
    outs.writeString("xmlns:smil=\"urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0\"\n");
    outs.writeString("xmlns:anim=\"urn:oasis:names:tc:opendocument:xmlns:animation:1.0\"\n");
    outs.writeString("office:version=\"1.0\">\n");
    outs.writeString("<office:meta>\n");
    Glib::ustring tmp = Glib::ustring::compose("    <meta:generator>%1</meta:generator>\n", InkscapeVersion);
    tmp += Glib::ustring::compose("    <meta:initial-creator>%1</meta:initial-creator>\n", creator);
    tmp += Glib::ustring::compose("    <meta:creation-date>%1</meta:creation-date>\n", date);
    tmp += Glib::ustring::compose("    <dc:date>%1</dc:date>\n", moddate);
    outs.writeUString(tmp);
    for (iter = metadata.begin() ; iter != metadata.end() ; ++iter)
    {
        Glib::ustring name  = iter->first;
        Glib::ustring value = iter->second;
        if (!name.empty() && !value.empty())
        {
            tmp = Glib::ustring::compose("    <%1>%2</%3>\n", name, value, name);
            outs.writeUString(tmp);
        }
    }
    // outs.writeString("    <meta:editing-cycles>2</meta:editing-cycles>\n");
    // outs.writeString("    <meta:editing-duration>PT56S</meta:editing-duration>\n");
    // outs.writeString("    <meta:user-defined meta:name=\"Info 1\"/>\n");
    // outs.writeString("    <meta:user-defined meta:name=\"Info 2\"/>\n");
    // outs.writeString("    <meta:user-defined meta:name=\"Info 3\"/>\n");
    // outs.writeString("    <meta:user-defined meta:name=\"Info 4\"/>\n");
    // outs.writeString("    <meta:document-statistic meta:object-count=\"2\"/>\n");
    outs.writeString("</office:meta>\n");
    outs.writeString("</office:document-meta>\n");
    outs.close();

    //Make our entry
    ZipEntry *ze = zf.newEntry("meta.xml", "ODF info file");
    ze->setUncompressedData(bouts.getBuffer());
    ze->finish();

    return true;
}


/**
 * Writes an SVG path as an ODF <draw:path> and returns the number of points written
 */
static int
writePath(Writer &outs, Geom::PathVector const &pathv,
          Geom::Affine const &tf, double xoff, double yoff)
{
    using Geom::X;
    using Geom::Y;

    int nrPoints  = 0;

    // convert the path to only lineto's and cubic curveto's:
    Geom::PathVector pv = pathv_to_linear_and_cubic_beziers(pathv * tf * Geom::Translate(xoff, yoff) * Geom::Scale(1000.));

        for (Geom::PathVector::const_iterator pit = pv.begin(); pit != pv.end(); ++pit) {

            double destx = pit->initialPoint()[X];
            double desty = pit->initialPoint()[Y];
            if (fabs(destx)<1.0) destx = 0.0;   // Why is this needed? Shouldn't we just round all numbers then?
            if (fabs(desty)<1.0) desty = 0.0;
            outs.printf("M %.3f %.3f ", destx, desty);
            nrPoints++;

            for (Geom::Path::const_iterator cit = pit->begin(); cit != pit->end_closed(); ++cit) {

                if( is_straight_curve(*cit) )
                {
                    double destx = cit->finalPoint()[X];
                    double desty = cit->finalPoint()[Y];
                    if (fabs(destx)<1.0) destx = 0.0;   // Why is this needed? Shouldn't we just round all numbers then?
                    if (fabs(desty)<1.0) desty = 0.0;
                    outs.printf("L %.3f %.3f ",  destx, desty);
                }
                else if(Geom::CubicBezier const *cubic = dynamic_cast<Geom::CubicBezier const*>(&*cit)) {
                    std::vector<Geom::Point> points = cubic->controlPoints();
                    for (unsigned i = 1; i <= 3; i++) {
                        if (fabs(points[i][X])<1.0) points[i][X] = 0.0;   // Why is this needed? Shouldn't we just round all numbers then?
                        if (fabs(points[i][Y])<1.0) points[i][Y] = 0.0;
                    }
                    outs.printf("C %.3f %.3f %.3f %.3f %.3f %.3f ", points[1][X],points[1][Y], points[2][X],points[2][Y], points[3][X],points[3][Y]);
                }
                else {
                    g_error ("logical error, because pathv_to_linear_and_cubic_beziers was used");
                }

                nrPoints++;
            }

            if (pit->closed()) {
                outs.printf("Z");
            }
        }

    return nrPoints;
}

bool OdfOutput::processStyle(SPItem *item, const Glib::ustring &id, const Glib::ustring &gradientNameFill, const Glib::ustring &gradientNameStroke, Glib::ustring& output)
{
    output.clear();
    if (!item)
    {
        return false;
    }
    
    SPStyle *style = item->style;
    if (!style)
    {
        return false;
    }
    
    StyleInfo si;

    // FILL
    if (style->fill.isColor())
    {
        guint32 fillCol = style->fill.value.color.toRGBA32( 0 );
        char buf[16];
        int r = (fillCol >> 24) & 0xff;
        int g = (fillCol >> 16) & 0xff;
        int b = (fillCol >>  8) & 0xff;
        snprintf(buf, 15, "#%02x%02x%02x", r, g, b);
        si.fillColor = buf;
        si.fill      = "solid";
        double opacityPercent = 100.0 *
             (SP_SCALE24_TO_FLOAT(style->fill_opacity.value));
        snprintf(buf, 15, "%.3f%%", opacityPercent);
        si.fillOpacity = buf;
    }
    else if (style->fill.isPaintserver())
    {
        SPGradient *gradient = SP_GRADIENT(SP_STYLE_FILL_SERVER(style));
        if (gradient)
        {
            si.fill = "gradient";
        }
    }

    // STROKE
    if (style->stroke.isColor())
    {
        guint32 strokeCol = style->stroke.value.color.toRGBA32( 0 );
        char buf[16];
        int r = (strokeCol >> 24) & 0xff;
        int g = (strokeCol >> 16) & 0xff;
        int b = (strokeCol >>  8) & 0xff;
        snprintf(buf, 15, "#%02x%02x%02x", r, g, b);
        si.strokeColor = buf;
        snprintf(buf, 15, "%.3fpt", style->stroke_width.value);
        si.strokeWidth = buf;
        si.stroke      = "solid";
        double opacityPercent = 100.0 *
             (SP_SCALE24_TO_FLOAT(style->stroke_opacity.value));
        snprintf(buf, 15, "%.3f%%", opacityPercent);
        si.strokeOpacity = buf;
    }
    else if (style->stroke.isPaintserver())
    {
        SPGradient *gradient = SP_GRADIENT(SP_STYLE_STROKE_SERVER(style));
        if (gradient)
        {
            si.stroke = "gradient";
        }
    }

    //Look for existing identical style;
    bool styleMatch = false;
    std::vector<StyleInfo>::iterator iter;
    for (iter=styleTable.begin() ; iter!=styleTable.end() ; ++iter)
    {
        if (si.equals(*iter))
        {
            //map to existing styleTable entry
            Glib::ustring styleName = iter->name;
            styleLookupTable[id] = styleName;
            styleMatch = true;
            break;
        }
    }

    // Dont need a new style
    if (styleMatch)
    {
        return false;
    }

    Glib::ustring styleName = Glib::ustring::compose("style%1", styleTable.size());
    si.name = styleName;
    styleTable.push_back(si);
    styleLookupTable[id] = styleName;

    output = Glib::ustring::compose ("<style:style style:name=\"%1\" style:family=\"graphic\" style:parent-style-name=\"standard\">\n", si.name);
    output += "<style:graphic-properties";
    if (si.fill == "gradient")
    {
        output += Glib::ustring::compose (" draw:fill=\"gradient\" draw:fill-gradient-name=\"%1\"", gradientNameFill);
    }
    else
    {
        output += Glib::ustring(" draw:fill=\"") + si.fill + "\"";
        if(si.fill != "none")
        {
            output += Glib::ustring::compose(" draw:fill-color=\"%1\"", si.fillColor);
        }
    }
    if (si.stroke == "gradient")
    {
        //does not seem to be supported by Open Office.org
        output += Glib::ustring::compose (" draw:stroke=\"gradient\" draw:stroke-gradient-name=\"%1\"", gradientNameStroke);
    }
    else
    {
        output += Glib::ustring(" draw:stroke=\"") + si.stroke + "\"";
        if (si.stroke != "none")
        {
            output += Glib::ustring::compose (" svg:stroke-width=\"%1\" svg:stroke-color=\"%2\" ", si.strokeWidth, si.strokeColor);
        }
    }
    output += "/>\n</style:style>\n";

    return true;
}

bool OdfOutput::processGradient(SPItem *item,
                                const Glib::ustring &id, Geom::Affine &/*tf*/,
                                Glib::ustring& gradientName, Glib::ustring& output, bool checkFillGradient)
{
    output.clear();
    if (!item)
    {
        return false;
    }

    SPStyle *style = item->style;
    if (!style)
    {
        return false;
    }

    if ((checkFillGradient? (!style->fill.isPaintserver()) : (!style->stroke.isPaintserver())))
    {
        return false;
    }

    //## Gradient
    SPGradient *gradient = SP_GRADIENT((checkFillGradient?(SP_STYLE_FILL_SERVER(style)) :(SP_STYLE_STROKE_SERVER(style))));

    if (gradient == NULL)
    {
        return false;
    }
    GradientInfo gi;
    SPGradient *grvec = gradient->getVector(FALSE);
    for (SPStop *stop = grvec->getFirstStop();
         stop ; stop = stop->getNextStop())
    {
        unsigned long rgba = stop->get_rgba32();
        unsigned long rgb  = (rgba >> 8) & 0xffffff;
        double opacity     = (static_cast<double>(rgba & 0xff)) / 256.0;
        GradientStop gs(rgb, opacity);
        gi.stops.push_back(gs);
    }

    Glib::ustring gradientName2;
    if (SP_IS_LINEARGRADIENT(gradient))
    {
        gi.style = "linear";
        SPLinearGradient *linGrad = SP_LINEARGRADIENT(gradient);
        gi.x1 = linGrad->x1.value;
        gi.y1 = linGrad->y1.value;
        gi.x2 = linGrad->x2.value;
        gi.y2 = linGrad->y2.value;
        gradientName2 = Glib::ustring::compose("ImportedLinearGradient%1", gradientTable.size());
    }
    else if (SP_IS_RADIALGRADIENT(gradient))
    {
        gi.style = "radial";
        SPRadialGradient *radGrad = SP_RADIALGRADIENT(gradient);
        Geom::OptRect bbox = item->documentVisualBounds();
        gi.cx = (radGrad->cx.value-bbox->left())/bbox->width();
        gi.cy = (radGrad->cy.value-bbox->top())/bbox->height();
        gradientName2 = Glib::ustring::compose("ImportedRadialGradient%1", gradientTable.size());
    }
    else
    {
        g_warning("not a supported gradient type");
        return false;
    }

    //Look for existing identical style;
    bool gradientMatch = false;
    std::vector<GradientInfo>::iterator iter;
    for (iter=gradientTable.begin() ; iter!=gradientTable.end() ; ++iter)
    {
        if (gi.equals(*iter))
        {
            //map to existing gradientTable entry
            gradientName = iter->name;
            gradientLookupTable[id] = gradientName;
            gradientMatch = true;
            break;
        }
    }

    if (gradientMatch)
    {
        return true;
    }

    // No match, let us write a new entry
    gradientName = gradientName2;
    gi.name = gradientName;
    gradientTable.push_back(gi);
    gradientLookupTable[id] = gradientName;

    // int gradientCount = gradientTable.size();
    char buf[128];
    if (gi.style == "linear")
    {
        /*
        ===================================================================
        LINEAR gradient.  We need something that looks like this:
        <draw:gradient draw:name="Gradient_20_7"
            draw:display-name="Gradient 7"
            draw:style="linear"
            draw:start-color="#008080" draw:end-color="#993366"
            draw:start-intensity="100%" draw:end-intensity="100%"
            draw:angle="150" draw:border="0%"/>
        ===================================================================
        */
        if (gi.stops.size() < 2)
        {
            g_warning("Need at least 2 stops for a linear gradient");
            return false;
        }
        output += Glib::ustring::compose("<draw:gradient draw:name=\"%1\"", gi.name);
        output += Glib::ustring::compose(" draw:display-name=\"%1\"", gi.name);
        output += " draw:style=\"linear\"";
        snprintf(buf, 127, " draw:start-color=\"#%06lx\" draw:end-color=\"#%06lx\"", gi.stops[0].rgb, gi.stops[1].rgb);
        output += buf;
        //TODO: apply maths, to define begin of gradient, taking gradient begin and end, as well as object boundary into account
        double angle = (gi.y2-gi.y1);
        angle = (angle != 0.) ? (atan((gi.x2-gi.x1)/(gi.y2-gi.y1))* 180. / pi) : 90;
        angle = (angle < 0)?(180+angle):angle;
        angle = angle * 10; //why do we need this: precision?????????????
        output += Glib::ustring::compose(" draw:start-intensity=\"%1\" draw:end-intensity=\"%2\" draw:angle=\"%3\"/>\n",
            gi.stops[0].opacity * 100.0, gi.stops[1].opacity * 100.0, angle);// draw:border=\"0%%\"
    }
    else if (gi.style == "radial")
    {
        /*
        ===================================================================
        RADIAL gradient.  We need something that looks like this:
        <!-- radial gradient, light gray to white, centered, 0% border -->
        <draw:gradient draw:name="radial_20_borderless"
            draw:display-name="radial borderless"
            draw:style="radial"
            draw:cx="50%" draw:cy="50%"
            draw:start-color="#999999" draw:end-color="#ffffff"
            draw:border="0%"/>
        ===================================================================
        */
        if (gi.stops.size() < 2)
        {
            g_warning("Need at least 2 stops for a radial gradient");
            return false;
        }
        output += Glib::ustring::compose("<draw:gradient draw:name=\"%1\" draw:display-name=\"%1\" ", gi.name);
        snprintf(buf, 127, "draw:cx=\"%05.3f\" draw:cy=\"%05.3f\" ", gi.cx*100, gi.cy*100);
        output += Glib::ustring("draw:style=\"radial\" ") + buf;
        snprintf(buf, 127, "draw:start-color=\"#%06lx\" draw:end-color=\"#%06lx\" ", gi.stops[0].rgb, gi.stops[1].rgb);
        output += buf;
        snprintf(buf, 127, "draw:start-intensity=\"%f%%\" draw:end-intensity=\"%f%%\" ", gi.stops[0].opacity*100.0, gi.stops[1].opacity*100.0);
        output += buf;
        output += "/>\n";//draw:border=\"0%\"
    }
    else
    {
        g_warning("unsupported gradient style '%s'", gi.style.c_str());
        return false;
    }
    return true;
}


/**
 * SECOND PASS.
 * This is the main SPObject tree output to ODF.
 */
bool OdfOutput::writeTree(Writer &couts, Writer &souts,
                          Inkscape::XML::Node *node)
{
    //# Get the SPItem, if applicable
    SPObject *reprobj = SP_ACTIVE_DOCUMENT->getObjectByRepr(node);
    if (!reprobj)
    {
        return true;
    }
    if (!SP_IS_ITEM(reprobj))
    {
        return true;
    }
    SPItem *item = SP_ITEM(reprobj);

    Glib::ustring nodeName = node->name();
    Glib::ustring id       = getAttribute(node, "id");
    Geom::Affine tf        = getODFTransform(item);//Get SVG-to-ODF transform
    Geom::OptRect bbox = getODFBoundingBox(item);//Get ODF bounding box params for item
    if (!bbox) {
        return true;
    }

    double bbox_x        = bbox->min()[Geom::X];
    double bbox_y        = bbox->min()[Geom::Y];
    double bbox_width    = (*bbox)[Geom::X].extent();
    double bbox_height   = (*bbox)[Geom::Y].extent();

    double rotate;
    double xskew;
    double yskew;
    double xscale;
    double yscale;
    analyzeTransform(tf, rotate, xskew, yskew, xscale, yscale);

    //# Do our stuff
    SPCurve *curve = NULL;

    if (nodeName == "svg" || nodeName == "svg:svg")
    {
        //# Iterate through the children
        for (Inkscape::XML::Node *child = node->firstChild() ;
               child ; child = child->next())
        {
            if (!writeTree(couts, souts, child))
            {
                return false;
            }
        }
        return true;
    }
    else if (nodeName == "g" || nodeName == "svg:g")
    {
        if (!id.empty())
        {
            couts.printf("<draw:g id=\"%s\">\n", id.c_str());
        }
        else
        {
            couts.printf("<draw:g>\n");
        }
        //# Iterate through the children
        for (Inkscape::XML::Node *child = node->firstChild() ;
               child ; child = child->next())
        {
            if (!writeTree(couts, souts, child))
            {
                return false;
            }
        }
        if (!id.empty())
        {
            couts.printf("</draw:g> <!-- id=\"%s\" -->\n", id.c_str());
        }
        else
        {
            couts.printf("</draw:g>\n");
        }
        return true;
    }

    //# GRADIENT
    Glib::ustring gradientNameFill;
    Glib::ustring gradientNameStroke;
    Glib::ustring outputFill;
    Glib::ustring outputStroke;
    Glib::ustring outputStyle;
    
    processGradient(item, id, tf, gradientNameFill, outputFill, 1);
    processGradient(item, id, tf, gradientNameStroke, outputStroke, 0);
    souts.writeUString(outputFill);
    souts.writeUString(outputStroke);
    
    //# STYLE
    processStyle(item, id, gradientNameFill, gradientNameStroke, outputStyle);
    souts.writeUString(outputStyle);

    //# ITEM DATA
    if (nodeName == "image" || nodeName == "svg:image")
    {
        if (!SP_IS_IMAGE(item))
        {
            g_warning("<image> is not an SPImage.");
            return false;
        }

        SPImage *img   = SP_IMAGE(item);
        double ix      = img->x.value;
        double iy      = img->y.value;
        double iwidth  = img->width.value;
        double iheight = img->height.value;

        Geom::Rect ibbox(Geom::Point(ix, iy), Geom::Point(ix+iwidth, iy+iheight));
        ibbox = ibbox * tf;
        ix      = ibbox.min()[Geom::X];
        iy      = ibbox.min()[Geom::Y];
        iwidth  = xscale * iwidth;
        iheight = yscale * iheight;

        Geom::Affine itemTransform = getODFItemTransform(item);

        Glib::ustring itemTransformString = formatTransform(itemTransform);

        Glib::ustring href = getAttribute(node, "xlink:href");
        std::map<Glib::ustring, Glib::ustring>::iterator iter = imageTable.find(href);
        if (iter == imageTable.end())
        {
            g_warning("image '%s' not in table", href.c_str());
            return false;
        }
        Glib::ustring newName = iter->second;

        couts.printf("<draw:frame ");
        if (!id.empty())
        {
            couts.printf("id=\"%s\" ", id.c_str());
        }
        couts.printf("draw:style-name=\"gr1\" draw:text-style-name=\"P1\" draw:layer=\"layout\" ");
        //no x or y.  make them the translate transform, last one
        couts.printf("svg:width=\"%.3fcm\" svg:height=\"%.3fcm\" ",
                                  iwidth, iheight);
        if (!itemTransformString.empty())
        {
            couts.printf("draw:transform=\"%s translate(%.3fcm, %.3fcm)\" ",
                           itemTransformString.c_str(), ix, iy);
        }
        else
        {
            couts.printf("draw:transform=\"translate(%.3fcm, %.3fcm)\" ", ix, iy);
        }

        couts.writeString(">\n");
        couts.printf("    <draw:image xlink:href=\"%s\" xlink:type=\"simple\"\n",
                              newName.c_str());
        couts.writeString("        xlink:show=\"embed\" xlink:actuate=\"onLoad\">\n");
        couts.writeString("        <text:p/>\n");
        couts.writeString("    </draw:image>\n");
        couts.writeString("</draw:frame>\n");
        return true;
    }
    else if (SP_IS_SHAPE(item))
    {
        curve = SP_SHAPE(item)->getCurve();
    }
    else if (SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item))
    {
        curve = te_get_layout(item)->convertToCurves();
    }

    if (curve)
    {
        //### Default <path> output
        couts.writeString("<draw:path ");
        if (!id.empty())
        {
            couts.printf("id=\"%s\" ", id.c_str());
        }

        std::map<Glib::ustring, Glib::ustring>::iterator siter;
        siter = styleLookupTable.find(id);
        if (siter != styleLookupTable.end())
        {
            Glib::ustring styleName = siter->second;
            couts.printf("draw:style-name=\"%s\" ", styleName.c_str());
        }

        couts.printf("draw:layer=\"layout\" svg:x=\"%.3fcm\" svg:y=\"%.3fcm\" ",
                       bbox_x, bbox_y);
        couts.printf("svg:width=\"%.3fcm\" svg:height=\"%.3fcm\" ",
                       bbox_width, bbox_height);
        couts.printf("svg:viewBox=\"0.0 0.0 %.3f %.3f\"",
                       bbox_width * 1000.0, bbox_height * 1000.0);

        couts.printf(" svg:d=\"");
        int nrPoints = writePath(couts, curve->get_pathvector(),
                             tf, bbox_x, bbox_y);
        couts.writeString("\"");

        couts.writeString(">\n");
        couts.printf("    <!-- %d nodes -->\n", nrPoints);
        couts.writeString("</draw:path>\n\n");

        curve->unref();
    }

    return true;
}


/**
 * Write the header for the content.xml file
 */
bool OdfOutput::writeStyleHeader(Writer &outs)
{
    time_t tim;
    time(&tim);

    outs.writeString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    outs.writeString("\n");
    outs.writeString("<!--\n");
    outs.writeString("*************************************************************************\n");
    outs.writeString("  file:  styles.xml\n");
    outs.printf     ("  Generated by Inkscape: %s", ctime(&tim)); //ctime has its own <cr>
    outs.writeString("  http://www.inkscape.org\n");
    outs.writeString("*************************************************************************\n");
    outs.writeString("-->\n");
    outs.writeString("\n");
    outs.writeString("<office:document-styles\n");
    outs.writeString("    xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"\n");
    outs.writeString("    xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\"\n");
    outs.writeString("    xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\"\n");
    outs.writeString("    xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\"\n");
    outs.writeString("    xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\"\n");
    outs.writeString("    xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"\n");
    outs.writeString("    xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n");
    outs.writeString("    xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n");
    outs.writeString("    xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\"\n");
    outs.writeString("    xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\"\n");
    outs.writeString("    xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\"\n");
    outs.writeString("    xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\"\n");
    outs.writeString("    xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\"\n");
    outs.writeString("    xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\"\n");
    outs.writeString("    xmlns:math=\"http://www.w3.org/1998/Math/MathML\"\n");
    outs.writeString("    xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\"\n");
    outs.writeString("    xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\"\n");
    outs.writeString("    xmlns:ooo=\"http://openoffice.org/2004/office\"\n");
    outs.writeString("    xmlns:ooow=\"http://openoffice.org/2004/writer\"\n");
    outs.writeString("    xmlns:oooc=\"http://openoffice.org/2004/calc\"\n");
    outs.writeString("    xmlns:dom=\"http://www.w3.org/2001/xml-events\"\n");
    outs.writeString("    xmlns:xforms=\"http://www.w3.org/2002/xforms\"\n");
    outs.writeString("    xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n");
    outs.writeString("    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
    outs.writeString("    xmlns:smil=\"urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0\"\n");
    outs.writeString("    xmlns:anim=\"urn:oasis:names:tc:opendocument:xmlns:animation:1.0\"\n");
    outs.writeString("    office:version=\"1.0\">\n");
    outs.writeString("\n");
    outs.writeString("<!--\n");
    outs.writeString("*************************************************************************\n");
    outs.writeString("  S T Y L E S\n");
    outs.writeString("  Style entries have been pulled from the svg style and\n");
    outs.writeString("  representation attributes in the SVG tree.  The tree elements\n");
    outs.writeString("  then refer to them by name, in the ODF manner\n");
    outs.writeString("*************************************************************************\n");
    outs.writeString("-->\n");
    outs.writeString("\n");
    outs.writeString("<office:styles>\n");
    outs.writeString("\n");

    return true;
}


/**
 * Write the footer for the style.xml file
 */
bool OdfOutput::writeStyleFooter(Writer &outs)
{
    outs.writeString("\n");
    outs.writeString("</office:styles>\n");
    outs.writeString("\n");
    outs.writeString("<office:automatic-styles>\n");
    outs.writeString("<!-- ####### 'Standard' styles ####### -->\n");
    outs.writeString("<style:style style:name=\"dp1\" style:family=\"drawing-page\"/>\n");
    outs.writeString("<style:style style:name=\"standard\" style:family=\"graphic\">\n");
    
///TODO: add default document style here
    
    outs.writeString("</style:style>\n");
    outs.writeString("<style:style style:name=\"gr1\" style:family=\"graphic\" style:parent-style-name=\"standard\">\n");
    outs.writeString("  <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\"\n");
    outs.writeString("       draw:textarea-horizontal-align=\"center\"\n");
    outs.writeString("       draw:textarea-vertical-align=\"middle\" draw:color-mode=\"standard\"\n");
    outs.writeString("       draw:luminance=\"0%\" draw:contrast=\"0%\" draw:gamma=\"100%\" draw:red=\"0%\"\n");
    outs.writeString("       draw:green=\"0%\" draw:blue=\"0%\" fo:clip=\"rect(0cm 0cm 0cm 0cm)\"\n");
    outs.writeString("       draw:image-opacity=\"100%\" style:mirror=\"none\"/>\n");
    outs.writeString("</style:style>\n");
    outs.writeString("<style:style style:name=\"P1\" style:family=\"paragraph\">\n");
    outs.writeString("  <style:paragraph-properties fo:text-align=\"center\"/>\n");
    outs.writeString("</style:style>\n");
    outs.writeString("</office:automatic-styles>\n");
    outs.writeString("\n");
    outs.writeString("<office:master-styles>\n");
    outs.writeString("<draw:layer-set>\n");
    outs.writeString("    <draw:layer draw:name=\"layout\"/>\n");
    outs.writeString("    <draw:layer draw:name=\"background\"/>\n");
    outs.writeString("    <draw:layer draw:name=\"backgroundobjects\"/>\n");
    outs.writeString("    <draw:layer draw:name=\"controls\"/>\n");
    outs.writeString("    <draw:layer draw:name=\"measurelines\"/>\n");
    outs.writeString("</draw:layer-set>\n");
    outs.writeString("\n");
    outs.writeString("<style:master-page style:name=\"Default\"\n");
    outs.writeString("    style:page-master-name=\"PM1\" draw:style-name=\"dp1\"/>\n");
    outs.writeString("</office:master-styles>\n");
    outs.writeString("\n");
    outs.writeString("</office:document-styles>\n");

    return true;
}


/**
 * Write the header for the content.xml file
 */
bool OdfOutput::writeContentHeader(Writer &outs)
{
    time_t tim;
    time(&tim);

    outs.writeString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    outs.writeString("\n");
    outs.writeString("<!--\n");
    outs.writeString("*************************************************************************\n");
    outs.writeString("  file:  content.xml\n");
    outs.printf     ("  Generated by Inkscape: %s", ctime(&tim)); //ctime has its own <cr>
    outs.writeString("  http://www.inkscape.org\n");
    outs.writeString("*************************************************************************\n");
    outs.writeString("-->\n");
    outs.writeString("\n");
    outs.writeString("<office:document-content\n");
    outs.writeString("    xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"\n");
    outs.writeString("    xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\"\n");
    outs.writeString("    xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\"\n");
    outs.writeString("    xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\"\n");
    outs.writeString("    xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\"\n");
    outs.writeString("    xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"\n");
    outs.writeString("    xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n");
    outs.writeString("    xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n");
    outs.writeString("    xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\"\n");
    outs.writeString("    xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\"\n");
    outs.writeString("    xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\"\n");
    outs.writeString("    xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\"\n");
    outs.writeString("    xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\"\n");
    outs.writeString("    xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\"\n");
    outs.writeString("    xmlns:math=\"http://www.w3.org/1998/Math/MathML\"\n");
    outs.writeString("    xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\"\n");
    outs.writeString("    xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\"\n");
    outs.writeString("    xmlns:ooo=\"http://openoffice.org/2004/office\"\n");
    outs.writeString("    xmlns:ooow=\"http://openoffice.org/2004/writer\"\n");
    outs.writeString("    xmlns:oooc=\"http://openoffice.org/2004/calc\"\n");
    outs.writeString("    xmlns:dom=\"http://www.w3.org/2001/xml-events\"\n");
    outs.writeString("    xmlns:xforms=\"http://www.w3.org/2002/xforms\"\n");
    outs.writeString("    xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n");
    outs.writeString("    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
    outs.writeString("    xmlns:smil=\"urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0\"\n");
    outs.writeString("    xmlns:anim=\"urn:oasis:names:tc:opendocument:xmlns:animation:1.0\"\n");
    outs.writeString("    office:version=\"1.0\">\n");
    outs.writeString("<office:scripts/>\n");
    outs.writeString("\n");
    outs.writeString("<!--\n");
    outs.writeString("*************************************************************************\n");
    outs.writeString("  D R A W I N G\n");
    outs.writeString("  This section is the heart of SVG-ODF conversion.  We are\n");
    outs.writeString("  starting with simple conversions, and will slowly evolve\n");
    outs.writeString("  into a 'smarter' translation as time progresses.  Any help\n");
    outs.writeString("  in improving .odg export is welcome.\n");
    outs.writeString("*************************************************************************\n");
    outs.writeString("-->\n");
    outs.writeString("\n");
    outs.writeString("<office:body>\n");
    outs.writeString("<office:drawing>\n");
    outs.writeString("<draw:page draw:name=\"page1\" draw:style-name=\"dp1\"\n");
    outs.writeString("        draw:master-page-name=\"Default\">\n");
    outs.writeString("\n");
    return true;
}


/**
 * Write the footer for the content.xml file
 */
bool OdfOutput::writeContentFooter(Writer &outs)
{
    outs.writeString("\n");
    outs.writeString("</draw:page>\n");
    outs.writeString("</office:drawing>\n");
    outs.writeString("\n");
    outs.writeString("<!-- ######### CONVERSION FROM SVG ENDS ######## -->\n");
    outs.writeString("\n");
    outs.writeString("</office:body>\n");
    outs.writeString("</office:document-content>\n");
    return true;
}


/**
 * Write the content.xml file.  Writes the namesspace headers, then
 * calls writeTree().
 */
bool OdfOutput::writeContent(ZipFile &zf, Inkscape::XML::Node *node)
{
    //Content.xml stream
    BufferOutputStream cbouts;
    OutputStreamWriter couts(cbouts);

    if (!writeContentHeader(couts))
    {
        return false;
    }

    //Style.xml stream
    BufferOutputStream sbouts;
    OutputStreamWriter souts(sbouts);

    if (!writeStyleHeader(souts))
    {
        return false;
    }

    //# Descend into the tree, doing all of our conversions
    //# to both files at the same time
    char *oldlocale = g_strdup (setlocale (LC_NUMERIC, NULL));
    setlocale (LC_NUMERIC, "C");
    if (!writeTree(couts, souts, node))
    {
        g_warning("Failed to convert SVG tree");
        setlocale (LC_NUMERIC, oldlocale);
        g_free (oldlocale);
        return false;
    }
    setlocale (LC_NUMERIC, oldlocale);
    g_free (oldlocale);

    //# Finish content file
    if (!writeContentFooter(couts))
    {
        return false;
    }

    ZipEntry *ze = zf.newEntry("content.xml", "ODF master content file");
    ze->setUncompressedData(cbouts.getBuffer());
    ze->finish();

    //# Finish style file
    if (!writeStyleFooter(souts))
    {
        return false;
    }

    ze = zf.newEntry("styles.xml", "ODF style file");
    ze->setUncompressedData(sbouts.getBuffer());
    ze->finish();

    return true;
}


/**
 * Resets class to its pristine condition, ready to use again
 */
void OdfOutput::reset()
{
    metadata.clear();
    styleTable.clear();
    styleLookupTable.clear();
    gradientTable.clear();
    gradientLookupTable.clear();
    imageTable.clear();
}


/**
 * Descends into the SVG tree, mapping things to ODF when appropriate
 */
void OdfOutput::save(Inkscape::Extension::Output */*mod*/, SPDocument *doc, gchar const *filename)
{
    reset();

    documentUri = Inkscape::URI(filename);

    ZipFile zf;
    preprocess(zf, doc->rroot);

    if (!writeManifest(zf))
        {
        g_warning("Failed to write manifest");
        return;
        }

    if (!writeContent(zf, doc->rroot))
        {
        g_warning("Failed to write content");
        return;
        }

    if (!writeMeta(zf))
        {
        g_warning("Failed to write metafile");
        return;
        }

    if (!zf.writeFile(filename))
        {
        return;
        }
}


/**
 * This is the definition of PovRay output.  This function just
 * calls the extension system with the memory allocated XML that
 * describes the data.
*/
void OdfOutput::init()
{
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("OpenDocument Drawing Output") "</name>\n"
            "<id>org.inkscape.output.odf</id>\n"
            "<output>\n"
                "<extension>.odg</extension>\n"
                "<mimetype>text/x-povray-script</mimetype>\n"
                "<filetypename>" N_("OpenDocument drawing (*.odg)") "</filetypename>\n"
                "<filetypetooltip>" N_("OpenDocument drawing file") "</filetypetooltip>\n"
            "</output>\n"
        "</inkscape-extension>",
        new OdfOutput());
}

/**
 * Make sure that we are in the database
 */
bool OdfOutput::check (Inkscape::Extension::Extension */*module*/)
{
    /* We don't need a Key
    if (NULL == Inkscape::Extension::db.get(SP_MODULE_KEY_OUTPUT_POV))
        return FALSE;
    */

    return TRUE;
}

}  //namespace Internal
}  //namespace Extension
}  //namespace Inkscape


//########################################################################
//# E N D    O F    F I L E
//########################################################################

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
