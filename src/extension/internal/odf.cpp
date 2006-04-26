/**
 * OpenDocument <drawing> input and output
 *
 * This is an an entry in the extensions mechanism to begin to enable
 * the inputting and outputting of OpenDocument Format (ODF) files from
 * within Inkscape.  Although the initial implementations will be very lossy
 * do to the differences in the models of SVG and ODF, they will hopefully
 * improve greatly with time.
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2006 Bob Jamison
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


//# Inkscape includes
#include "clear-n_.h"
#include "inkscape.h"
#include <style.h>
#include "display/curve.h"
#include "libnr/n-art-bpath.h"
#include "extension/system.h"

#include "xml/repr.h"
#include "xml/attribute-record.h"
#include "sp-image.h"
#include "sp-path.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "svg/svg.h"
#include "text-editing.h"


//# DOM-specific includes
#include "dom/dom.h"
#include "dom/util/ziptool.h"
#include "dom/io/domstream.h"
#include "dom/io/bufferstream.h"






namespace Inkscape
{
namespace Extension
{
namespace Internal
{

//# Shorthand notation
typedef org::w3c::dom::DOMString DOMString;
typedef org::w3c::dom::io::OutputStreamWriter OutputStreamWriter;
typedef org::w3c::dom::io::BufferOutputStream BufferOutputStream;


//########################################################################
//# C L A S S    SingularValueDecomposition
//########################################################################
#include <math.h>

/**
 *
 * ====================================================
 *
 * NOTE:
 * This class is ported almost verbatim from the public domain
 * JAMA Matrix package.  It is modified to handle only 3x3 matrices
 * and our NR::Matrix affine transform class.  We give full
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

    SingularValueDecomposition (const NR::Matrix &matrixArg)
        {
        matrix = matrixArg;
        calculate();
        }

    virtual ~SingularValueDecomposition()
        {}

    /**
     * Return the left singular vectors
     * @return     U
     */
    NR::Matrix getU();

    /**
     * Return the right singular vectors
     * @return     V
     */
    NR::Matrix getV();

    /**
     * Return the right singular vectors
     * @return  U x Vtransposed
     */
    NR::Matrix getUVt();

    /**
     *  Return the s[0] value
     */
    double getS0();

    /**
     *  Return the s[1] value
     */
    double getS1();

    /**
     *  Return the s[2] value
     */
    double getS2();

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

      NR::Matrix matrix;
      double A[3][3];
      double U[3][3];
      double s[3];
      double V[3][3];

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
      A[0][0] = matrix[0];
      A[0][1] = matrix[2];
      A[0][2] = matrix[4];
      A[1][0] = matrix[1];
      A[1][1] = matrix[3];
      A[1][2] = matrix[5];
      A[2][0] = 0.0;
      A[2][1] = 0.0;
      A[2][2] = 1.0;

      double e[3];
      double work[3];
      bool wantu = true;
      bool wantv = true;
      int m  = 3;
      int n  = 3;
      int nu = 3;

      // Reduce A to bidiagonal form, storing the diagonal elements
      // in s and the super-diagonal elements in e.

      int nct = 2;
      int nrt = 1;
      for (int k = 0; k < 2; k++) {
         if (k < nct) {

            // Compute the transformation for the k-th column and
            // place the k-th diagonal in s[k].
            // Compute 2-norm of k-th column without under/overflow.
            s[k] = 0;
            for (int i = k; i < m; i++) {
               s[k] = svd_hypot(s[k],A[i][k]);
            }
            if (s[k] != 0.0) {
               if (A[k][k] < 0.0) {
                  s[k] = -s[k];
               }
               for (int i = k; i < m; i++) {
                  A[i][k] /= s[k];
               }
               A[k][k] += 1.0;
            }
            s[k] = -s[k];
         }
         for (int j = k+1; j < n; j++) {
            if ((k < nct) & (s[k] != 0.0))  {

            // Apply the transformation.

               double t = 0;
               for (int i = k; i < m; i++) {
                  t += A[i][k]*A[i][j];
               }
               t = -t/A[k][k];
               for (int i = k; i < m; i++) {
                  A[i][j] += t*A[i][k];
               }
            }

            // Place the k-th row of A into e for the
            // subsequent calculation of the row transformation.

            e[j] = A[k][j];
         }
         if (wantu & (k < nct)) {

            // Place the transformation in U for subsequent back
            // multiplication.

            for (int i = k; i < m; i++) {
               U[i][k] = A[i][k];
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
                     work[i] += e[j]*A[i][j];
                  }
               }
               for (int j = k+1; j < n; j++) {
                  double t = -e[j]/e[k+1];
                  for (int i = k+1; i < m; i++) {
                     A[i][j] += t*work[i];
                  }
               }
            }
            if (wantv) {

            // Place the transformation in V for subsequent
            // back multiplication.

               for (int i = k+1; i < n; i++) {
                  V[i][k] = e[i];
               }
            }
         }
      }

      // Set up the final bidiagonal matrix or order p.

      int p = 3;
      if (nct < n) {
         s[nct] = A[nct][nct];
      }
      if (m < p) {
         s[p-1] = 0.0;
      }
      if (nrt+1 < p) {
         e[nrt] = A[nrt][p-1];
      }
      e[p-1] = 0.0;

      // If required, generate U.

      if (wantu) {
         for (int j = nct; j < nu; j++) {
            for (int i = 0; i < m; i++) {
               U[i][j] = 0.0;
            }
            U[j][j] = 1.0;
         }
         for (int k = nct-1; k >= 0; k--) {
            if (s[k] != 0.0) {
               for (int j = k+1; j < nu; j++) {
                  double t = 0;
                  for (int i = k; i < m; i++) {
                     t += U[i][k]*U[i][j];
                  }
                  t = -t/U[k][k];
                  for (int i = k; i < m; i++) {
                     U[i][j] += t*U[i][k];
                  }
               }
               for (int i = k; i < m; i++ ) {
                  U[i][k] = -U[i][k];
               }
               U[k][k] = 1.0 + U[k][k];
               for (int i = 0; i < k-1; i++) {
                  U[i][k] = 0.0;
               }
            } else {
               for (int i = 0; i < m; i++) {
                  U[i][k] = 0.0;
               }
               U[k][k] = 1.0;
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
                     t += V[i][k]*V[i][j];
                  }
                  t = -t/V[k+1][k];
                  for (int i = k+1; i < n; i++) {
                     V[i][j] += t*V[i][k];
                  }
               }
            }
            for (int i = 0; i < n; i++) {
               V[i][k] = 0.0;
            }
            V[k][k] = 1.0;
         }
      }

      // Main iteration loop for the singular values.

      int pp = p-1;
      int iter = 0;
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
                        t = cs*V[i][j] + sn*V[i][p-1];
                        V[i][p-1] = -sn*V[i][j] + cs*V[i][p-1];
                        V[i][j] = t;
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
                        t = cs*U[i][j] + sn*U[i][k-1];
                        U[i][k-1] = -sn*U[i][j] + cs*U[i][k-1];
                        U[i][j] = t;
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
                        t = cs*V[i][j] + sn*V[i][j+1];
                        V[i][j+1] = -sn*V[i][j] + cs*V[i][j+1];
                        V[i][j] = t;
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
                        t = cs*U[i][j] + sn*U[i][j+1];
                        U[i][j+1] = -sn*U[i][j] + cs*U[i][j+1];
                        U[i][j] = t;
                     }
                  }
               }
               e[p-2] = f;
               iter = iter + 1;
            }
            break;

            // Convergence.

            case 4: {

               // Make the singular values positive.

               if (s[k] <= 0.0) {
                  s[k] = (s[k] < 0.0 ? -s[k] : 0.0);
                  if (wantv) {
                     for (int i = 0; i <= pp; i++) {
                        V[i][k] = -V[i][k];
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
                        t = V[i][k+1]; V[i][k+1] = V[i][k]; V[i][k] = t;
                     }
                  }
                  if (wantu && (k < m-1)) {
                     for (int i = 0; i < m; i++) {
                        t = U[i][k+1]; U[i][k+1] = U[i][k]; U[i][k] = t;
                     }
                  }
                  k++;
               }
               iter = 0;
               p--;
            }
            break;
         }
      }


}



/**
 * Return the left singular vectors
 * @return     U
 */
NR::Matrix SingularValueDecomposition::getU()
{
    NR::Matrix mat(U[0][0], U[1][0], U[0][1],
                   U[1][1], U[0][2], U[1][2]);
    return mat;
}

/**
 * Return the right singular vectors
 * @return     V
 */

NR::Matrix SingularValueDecomposition::getV()
{
    NR::Matrix mat(V[0][0], V[1][0], V[0][1],
                   V[1][1], V[0][2], V[1][2]);
    return mat;
}

/**
 * Return the right singular vectors
 * @return  U x Vtransposed
 */

NR::Matrix SingularValueDecomposition::getUVt()
{
    //instead of sum(row*column),  sum(column, column)
    double a = U[0][0] * V[0][0] + U[1][0] * V[1][0];
    double b = U[0][0] * V[0][1] + U[1][0] * V[1][1];
    double c = U[0][1] * V[0][0] + U[1][1] * V[1][0];
    double d = U[0][1] * V[0][1] + U[1][1] * V[1][1];
    double e = U[0][2] * V[0][0] + U[1][2] * V[1][0];
    double f = U[0][2] * V[0][1] + U[1][2] * V[1][1];

    NR::Matrix mat(a, b, c, d, e, f);
    return mat;
}

/**
 *  Return the s[0] value
 */
double SingularValueDecomposition::getS0()
{
    return s[0];
}

/**
 *  Return the s[1] value
 */
double SingularValueDecomposition::getS1()
{
    return s[1];
}

/**
 *  Return the s[2] value
 */
double SingularValueDecomposition::getS2()
{
    return s[2];
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
#define pxToCm  0.04
#define piToRad 0.0174532925
#define docHeightCm 22.86


//########################################################################
//# O U T P U T
//########################################################################

static std::string getAttribute( Inkscape::XML::Node *node, char *attrName)
{
    std::string val;
    char *valstr = (char *)node->attribute(attrName);
    if (valstr)
        val = (const char *)valstr;
    return val;
}


static std::string getExtension(const std::string &fname)
{
    std::string ext;

    unsigned int pos = fname.rfind('.');
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


static std::string formatTransform(NR::Matrix &tf)
{
    std::string str;
    if (!tf.test_identity())
        {
        char buf[128];
        snprintf(buf, 127, "matrix(%.3f %.3f %.3f %.3f %.3f %.3f)",
                tf[0], tf[1], tf[2], tf[3], tf[4], tf[5]);
        str = buf;
        }
    return str;
}


/**
 * An affine transformation Q may be decomposed via
 * singular value decomposition into
 *
 * T = UDVt
 *   = (UDUt)UVt
 * ('t' means transposed)
 * where U and V are orthonormal matrices and D is a diagonal
 * matrix. The decomposition may be interpreted as such:
 * the image is firstly rotated by UVt. The image is then
 * rotated by Ut, stretched in the coordinate directions
 * by D then rotated back by U. The net effect is a slant operation in
 * some tilt direction followed by an isotropic scale. If rot(x)
 * is a matrix that rotates by x we can rewrite this as
 * T = rot(-tau)( k 0, 0 1) rot(tau) rot(theta) S
 * where S is a scaling matrix, k is a multiplying (contraction)
 * factor related to slant, tau is the tilt direction and theta
 * is the initial rotation angle.
 */
/*
static void analyzeTransform1(NR::Matrix &tf)
{
    SingularValueDecomposition svd(tf);
    double scale1 = svd.getS0();
    double rotate = svd.getS1();
    double scale2 = svd.getS2();
    NR::Matrix u   = svd.getU();
    NR::Matrix v   = svd.getV();
    NR::Matrix uvt = svd.getUVt();
    //g_message("s1:%f  rot:%f  s2:%f", scale1, rotate, scale2);
    std::string us = formatTransform(u);
    //g_message("u:%s", us.c_str());
    std::string vs = formatTransform(v);
    //g_message("v:%s", vs.c_str());
    std::string uvts = formatTransform(uvt);
    //g_message("uvt:%s", uvts.c_str());
}
*/


static void analyzeTransform2(NR::Matrix &tf,
           double &xskew, double &yskew, double &xscale, double &yscale)
{
    //Let's calculate some of the qualities of the transform directly
    //Make a unit rect and transform it
    NR::Point top_left(0.0, 0.0);
    NR::Point top_right(1.0, 0.0);
    NR::Point bottom_left(0.0, 1.0);
    NR::Point bottom_right(1.0, 1.0);
    top_left     *= tf;
    top_right    *= tf;
    bottom_left  *= tf;
    bottom_right *= tf;
    double dx_tr = top_right[NR::X]   - top_left[NR::X];
    double dy_tr = top_right[NR::Y]   - top_left[NR::Y];
    double dx_bl = bottom_left[NR::X] - top_left[NR::X];
    double dy_bl = bottom_left[NR::Y] - top_left[NR::Y];

    double xskew_angle = 0.0;
    double yskew_angle = 0.0;
    if (fabs(dx_tr) < 1.0e-10 && fabs(dy_tr) > 1.0e-10)  //90 degrees?
        {
        if (dy_tr>0)
            yskew_angle = pi / 2.0;
        else
            yskew_angle = -pi / 2.0;
        }
    else
        {
        yskew_angle = atan(dy_tr / dx_tr);
        }

    if (fabs(dy_bl) < 1.0e-10 && fabs(dx_bl) > 1.0e-10)  //90 degrees?
        {
        if (dx_bl>0)
            xskew_angle = pi / 2.0;
        else
            xskew_angle = -pi / 2.0;
        }
    else
        {
        xskew_angle = atan(dx_bl / dy_bl);
        }

    double expected_xsize = 1.0 + dx_bl;
    xscale =
        ( bottom_right[NR::X] - top_left[NR::X] )/ expected_xsize;
    double expected_ysize = 1.0 + dy_tr;
    yscale =
        ( bottom_right[NR::Y] - top_left[NR::Y] )/ expected_ysize;

    //g_message("xskew:%f yskew:%f xscale:%f yscale:%f",
    //      xskew_angle, yskew_angle, xscale, yscale);
    xskew = xskew_angle;
    yskew = xskew_angle;
}




/**
 * Method descends into the repr tree, converting image and style info
 * into forms compatible in ODF.
 */
void
OdfOutput::preprocess(ZipFile &zf, Inkscape::XML::Node *node)
{

    std::string nodeName = node->name();
    std::string id       = getAttribute(node, "id");

    if (nodeName == "image" || nodeName == "svg:image")
        {
        //g_message("image");
        std::string href = getAttribute(node, "xlink:href");
        if (href.size() > 0)
            {
            std::string oldName = href;
            std::string ext = getExtension(oldName);
            if (ext == ".jpeg")
                ext = ".jpg";
            if (imageTable.find(oldName) == imageTable.end())
                {
                char buf[64];
                snprintf(buf, 63, "Pictures/image%d%s",
                    imageTable.size(), ext.c_str());
                std::string newName = buf;
                imageTable[oldName] = newName;
                std::string comment = "old name was: ";
                comment.append(oldName);
                URI oldUri(oldName);
                //g_message("oldpath:%s", oldUri.getNativePath().c_str());
                //# if relative to the documentURI, get proper path
                URI resUri = documentUri.resolve(oldUri);
                DOMString pathName = resUri.getNativePath();
                //g_message("native path:%s", pathName.c_str());
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



    SPObject *reprobj = SP_ACTIVE_DOCUMENT->getObjectByRepr(node);
    if (!reprobj)
        return;
    if (!SP_IS_ITEM(reprobj))
        {
        return;
        }
    SPItem *item = SP_ITEM(reprobj);
    SPStyle *style = SP_OBJECT_STYLE(item);
    if (style && id.size()>0)
        {
        StyleInfo si;
        if (style->fill.type == SP_PAINT_TYPE_COLOR)
            {
            guint32 fillCol =
                sp_color_get_rgba32_ualpha(&style->fill.value.color, 0);
            char buf[16];
            int r = (fillCol >> 24) & 0xff;
            int g = (fillCol >> 16) & 0xff;
            int b = (fillCol >>  8) & 0xff;
            //g_message("## %s %lx", id.c_str(), (unsigned int)fillCol);
            snprintf(buf, 15, "#%02x%02x%02x", r, g, b);
            si.fillColor = buf;
            si.fill      = "solid";
            double opacityPercent = 100.0 *
                 (SP_SCALE24_TO_FLOAT(style->fill_opacity.value));
            snprintf(buf, 15, "%.2f%%", opacityPercent);
            si.fillOpacity = buf;
            }
        if (style->stroke.type == SP_PAINT_TYPE_COLOR)
            {
            guint32 strokeCol =
                sp_color_get_rgba32_ualpha(&style->stroke.value.color, 0);
            char buf[16];
            int r = (strokeCol >> 24) & 0xff;
            int g = (strokeCol >> 16) & 0xff;
            int b = (strokeCol >>  8) & 0xff;
            snprintf(buf, 15, "#%02x%02x%02x", r, g, b);
            si.strokeColor = buf;
            snprintf(buf, 15, "%.2fpt", style->stroke_width.value);
            si.strokeWidth = buf;
            si.stroke      = "solid";
            double opacityPercent = 100.0 *
                 (SP_SCALE24_TO_FLOAT(style->stroke_opacity.value));
            snprintf(buf, 15, "%.2f%%", opacityPercent);
            si.strokeOpacity = buf;
            }

        //Look for existing identical style;
        bool styleMatch = false;
        std::vector<StyleInfo>::iterator iter;
        for (iter=styleTable.begin() ; iter!=styleTable.end() ; iter++)
            {
            if (si.equals(*iter))
                {
                //map to existing styleTable entry
                std::string styleName = iter->name;
                //g_message("found duplicate style:%s", styleName.c_str());
                styleLookupTable[id] = styleName;
                styleMatch = true;
                break;
                }
            }
        //None found, make a new pair or entries
        if (!styleMatch)
            {
            char buf[16];
            snprintf(buf, 15, "style%d", styleTable.size());
            std::string styleName = buf;
            si.name = styleName;
            styleTable.push_back(si);
            styleLookupTable[id] = styleName;
            }
        }


    for (Inkscape::XML::Node *child = node->firstChild() ;
            child ; child = child->next())
        preprocess(zf, child);
}



bool OdfOutput::writeManifest(ZipFile &zf)
{
    BufferOutputStream bouts;
    OutputStreamWriter outs(bouts);

    time_t tim;
    time(&tim);

    outs.printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    outs.printf("<!DOCTYPE manifest:manifest PUBLIC \"-//OpenOffice.org//DTD Manifest 1.0//EN\" \"Manifest.dtd\">\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<!--\n");
    outs.printf("*************************************************************************\n");
    outs.printf("  file:  manifest.xml\n");
    outs.printf("  Generated by Inkscape: %s", ctime(&tim)); //ctime has its own <cr>
    outs.printf("  http://www.inkscape.org\n");
    outs.printf("*************************************************************************\n");
    outs.printf("-->\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\">\n");
    outs.printf("    <manifest:file-entry manifest:media-type=\"application/vnd.oasis.opendocument.graphics\" manifest:full-path=\"/\"/>\n");
    outs.printf("    <manifest:file-entry manifest:media-type=\"text/xml\" manifest:full-path=\"content.xml\"/>\n");
    outs.printf("    <manifest:file-entry manifest:media-type=\"text/xml\" manifest:full-path=\"meta.xml\"/>\n");
    outs.printf("    <!--List our images here-->\n");
    std::map<std::string, std::string>::iterator iter;
    for (iter = imageTable.begin() ; iter!=imageTable.end() ; iter++)
        {
        std::string oldName = iter->first;
        std::string newName = iter->second;

        std::string ext = getExtension(oldName);
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
        outs.printf((char *)newName.c_str());
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


bool OdfOutput::writeMeta(ZipFile &zf)
{
    BufferOutputStream bouts;
    OutputStreamWriter outs(bouts);

    time_t tim;
    time(&tim);

    outs.printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<!--\n");
    outs.printf("*************************************************************************\n");
    outs.printf("  file:  meta.xml\n");
    outs.printf("  Generated by Inkscape: %s", ctime(&tim)); //ctime has its own <cr>
    outs.printf("  http://www.inkscape.org\n");
    outs.printf("*************************************************************************\n");
    outs.printf("-->\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<office:document-meta\n");
    outs.printf("xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"\n");
    outs.printf("xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n");
    outs.printf("xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n");
    outs.printf("xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\"\n");
    outs.printf("xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\"\n");
    outs.printf("xmlns:ooo=\"http://openoffice.org/2004/office\"\n");
    outs.printf("xmlns:smil=\"urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0\"\n");
    outs.printf("xmlns:anim=\"urn:oasis:names:tc:opendocument:xmlns:animation:1.0\"\n");
    outs.printf("office:version=\"1.0\">\n");
    outs.printf("<office:meta>\n");
    outs.printf("    <meta:generator>Inkscape.org - 0.44</meta:generator>\n");
    outs.printf("    <meta:initial-creator>clark kent</meta:initial-creator>\n");
    outs.printf("    <meta:creation-date>2006-04-13T17:12:29</meta:creation-date>\n");
    outs.printf("    <dc:creator>clark kent</dc:creator>\n");
    outs.printf("    <dc:date>2006-04-13T17:13:20</dc:date>\n");
    outs.printf("    <dc:language>en-US</dc:language>\n");
    outs.printf("    <meta:editing-cycles>2</meta:editing-cycles>\n");
    outs.printf("    <meta:editing-duration>PT56S</meta:editing-duration>\n");
    outs.printf("    <meta:user-defined meta:name=\"Info 1\"/>\n");
    outs.printf("    <meta:user-defined meta:name=\"Info 2\"/>\n");
    outs.printf("    <meta:user-defined meta:name=\"Info 3\"/>\n");
    outs.printf("    <meta:user-defined meta:name=\"Info 4\"/>\n");
    outs.printf("    <meta:document-statistic meta:object-count=\"2\"/>\n");
    outs.printf("</office:meta>\n");
    outs.printf("</office:document-meta>\n");
    outs.printf("\n");
    outs.printf("\n");


    outs.close();

    //Make our entry
    ZipEntry *ze = zf.newEntry("meta.xml", "ODF info file");
    ze->setUncompressedData(bouts.getBuffer());
    ze->finish();

    return true;
}


bool OdfOutput::writeStyle(Writer &outs)
{
    outs.printf("<office:automatic-styles>\n");
    outs.printf("<!-- ####### 'Standard' styles ####### -->\n");
    outs.printf("<style:style style:name=\"dp1\" style:family=\"drawing-page\"/>\n");
    outs.printf("<style:style style:name=\"gr1\" style:family=\"graphic\" style:parent-style-name=\"standard\">\n");
    outs.printf("  <style:graphic-properties draw:stroke=\"none\" draw:fill=\"none\"\n");
    outs.printf("       draw:textarea-horizontal-align=\"center\"\n");
    outs.printf("       draw:textarea-vertical-align=\"middle\" draw:color-mode=\"standard\"\n");
    outs.printf("       draw:luminance=\"0%\" draw:contrast=\"0%\" draw:gamma=\"100%\" draw:red=\"0%\"\n");
    outs.printf("       draw:green=\"0%\" draw:blue=\"0%\" fo:clip=\"rect(0cm 0cm 0cm 0cm)\"\n");
    outs.printf("       draw:image-opacity=\"100%\" style:mirror=\"none\"/>\n");
    outs.printf("</style:style>\n");
    outs.printf("<style:style style:name=\"P1\" style:family=\"paragraph\">\n");
    outs.printf("  <style:paragraph-properties fo:text-align=\"center\"/>\n");
    outs.printf("</style:style>\n");

    //##  Dump our style table
    outs.printf("<!-- ####### Styles from Inkscape document ####### -->\n");
    std::vector<StyleInfo>::iterator iter;
    for (iter = styleTable.begin() ; iter != styleTable.end() ; iter++)
        {
        outs.printf("<style:style style:name=\"%s\"", iter->name.c_str());
        StyleInfo s(*iter);
        outs.printf(" style:family=\"graphic\" style:parent-style-name=\"standard\">\n");
        outs.printf("  <style:graphic-properties");
        outs.printf(" draw:fill=\"%s\" ", s.fill.c_str());
        if (s.fill != "none")
            {
            outs.printf(" draw:fill-color=\"%s\" ", s.fillColor.c_str());
            outs.printf(" draw:fill-opacity=\"%s\" ", s.fillOpacity.c_str());
            }
        outs.printf(" draw:stroke=\"%s\" ", s.stroke.c_str());
        if (s.stroke != "none")
            {
            outs.printf(" svg:stroke-width=\"%s\" ", s.strokeWidth.c_str());
            outs.printf(" svg:stroke-color=\"%s\" ", s.strokeColor.c_str());
            outs.printf(" svg:stroke-opacity=\"%s\" ", s.strokeOpacity.c_str());
            }
        outs.printf("/>\n");
        outs.printf("</style:style>\n");
        }

    outs.printf("</office:automatic-styles>\n");
    outs.printf("\n");

    return true;
}


static void
writePath(Writer &outs, NArtBpath const *bpath,
          NR::Matrix &tf, double xoff, double yoff)
{
    bool closed = false;
    NArtBpath *bp = (NArtBpath *)bpath;
    for (  ; bp->code != NR_END; bp++)
        {
        NR::Point const p1(bp->c(1) * tf);
	NR::Point const p2(bp->c(2) * tf);
	NR::Point const p3(bp->c(3) * tf);
	double x1 = (p1[NR::X] * pxToCm - xoff) * 1000.0;
	double y1 = (p1[NR::Y] * pxToCm - yoff) * 1000.0;
	double x2 = (p2[NR::X] * pxToCm - xoff) * 1000.0;
	double y2 = (p2[NR::Y] * pxToCm - yoff) * 1000.0;
	double x3 = (p3[NR::X] * pxToCm - xoff) * 1000.0;
	double y3 = (p3[NR::Y] * pxToCm - yoff) * 1000.0;

        switch (bp->code)
            {
            case NR_LINETO:
                outs.printf("L %.3f,%.3f ",  x3 , y3);
                break;

            case NR_CURVETO:
                outs.printf("C %.3f,%.3f %.3f,%.3f %.3f,%.3f ",
                              x1, y1, x2, y2, x3, y3);
                break;

            case NR_MOVETO_OPEN:
            case NR_MOVETO:
                if (closed)
                    outs.printf("z ");
                closed = ( bp->code == NR_MOVETO );
                outs.printf("M %.3f,%.3f ",  x3 , y3);
                break;

            default:
                break;

            }

        }

    if (closed)
        outs.printf("z");;

}



bool OdfOutput::writeTree(Writer &outs, Inkscape::XML::Node *node)
{
    //# Get the SPItem, if applicable
    SPObject *reprobj = SP_ACTIVE_DOCUMENT->getObjectByRepr(node);
    if (!reprobj)
        return true;
    if (!SP_IS_ITEM(reprobj))
        {
        return true;
        }
    SPItem *item = SP_ITEM(reprobj);


    std::string nodeName = node->name();
    std::string id       = getAttribute(node, "id");

    NR::Matrix tf        = sp_item_i2d_affine(item);
    NR::Rect bbox        = sp_item_bbox_desktop(item);

    //Flip Y into document coordinates
    double doc_height    = sp_document_height(SP_ACTIVE_DOCUMENT);
    NR::Matrix doc2dt_tf = NR::Matrix(NR::scale(1, -1));
    doc2dt_tf            = doc2dt_tf * NR::Matrix(NR::translate(0, doc_height));
    tf                   = tf   * doc2dt_tf;
    bbox                 = bbox * doc2dt_tf;

    double x      = pxToCm * bbox.min()[NR::X];
    double y      = pxToCm * bbox.min()[NR::Y];
    double width  = pxToCm * ( bbox.max()[NR::X] - bbox.min()[NR::X] );
    double height = pxToCm * ( bbox.max()[NR::Y] - bbox.min()[NR::Y] );


    double xskew;
    double yskew;
    double xscale;
    double yscale;
    analyzeTransform2(tf, xskew, yskew, xscale, yscale);

    double item_xskew;
    double item_yskew;
    double item_xscale;
    double item_yscale;
    analyzeTransform2(item->transform,
        item_xskew, item_yskew, item_xscale, item_yscale);

    //# Do our stuff
    SPCurve *curve = NULL;

    //g_message("##### %s #####", nodeName.c_str());

    if (nodeName == "svg" || nodeName == "svg:svg")
        {
        //# Iterate through the children
        for (Inkscape::XML::Node *child = node->firstChild() ; child ; child = child->next())
            {
            if (!writeTree(outs, child))
                return false;
            }
        return true;
        }
    else if (nodeName == "g" || nodeName == "svg:g")
        {
        if (id.size() > 0)
            outs.printf("<draw:g id=\"%s\">\n", id.c_str());
        else
            outs.printf("<draw:g>\n");
        //# Iterate through the children
        for (Inkscape::XML::Node *child = node->firstChild() ; child ; child = child->next())
            {
            if (!writeTree(outs, child))
                return false;
            }
        if (id.size() > 0)
            outs.printf("</draw:g> <!-- id=\"%s\" -->\n", id.c_str());
        else
            outs.printf("</draw:g>\n");
        return true;
        }
    else if (nodeName == "image" || nodeName == "svg:image")
        {
        if (!SP_IS_IMAGE(item))
            {
            g_warning("<image> is not an SPImage.  Why?  ;-)");
            return false;
            }

        SPImage *img   = SP_IMAGE(item);
        double ix      = img->x.value;
        double iy      = img->y.value;
        double iwidth  = img->width.value;
        double iheight = img->height.value;

        NR::Rect ibbox(NR::Point(ix, iy), NR::Point(ix+iwidth, iy+iheight));
        ibbox = ibbox * tf;
        ix      = pxToCm * ibbox.min()[NR::X];
        iy      = pxToCm * ibbox.min()[NR::Y];
        //iwidth  = pxToCm * ( ibbox.max()[NR::X] - ibbox.min()[NR::X] );
        //iheight = pxToCm * ( ibbox.max()[NR::Y] - ibbox.min()[NR::Y] );
        iwidth  = pxToCm * xscale * iwidth;
        iheight = pxToCm * yscale * iheight;

        NR::Matrix itemTransform = NR::Matrix(NR::scale(1, -1));
        itemTransform = itemTransform * item->transform;
        itemTransform = itemTransform * NR::Matrix(NR::scale(1, -1));

        std::string itemTransformString = formatTransform(itemTransform);

        std::string href = getAttribute(node, "xlink:href");
        std::map<std::string, std::string>::iterator iter = imageTable.find(href);
        if (iter == imageTable.end())
            {
            g_warning("image '%s' not in table", href.c_str());
            return false;
            }
        std::string newName = iter->second;

        outs.printf("<draw:frame ");
        if (id.size() > 0)
            outs.printf("id=\"%s\" ", id.c_str());
        outs.printf("draw:style-name=\"gr1\" draw:text-style-name=\"P1\" draw:layer=\"layout\" ");
        //no x or y.  make them the translate transform, last one
        outs.printf("svg:width=\"%.3fcm\" svg:height=\"%.3fcm\" ",
                                  iwidth, iheight);
        if (itemTransformString.size() > 0)
            outs.printf("draw:transform=\"%s translate(%.3fcm, %.3fcm)\" ",
                itemTransformString.c_str(), ix, iy);

        outs.printf(">\n");
        outs.printf("    <draw:image xlink:href=\"%s\" xlink:type=\"simple\"\n",
                              newName.c_str());
        outs.printf("        xlink:show=\"embed\" xlink:actuate=\"onLoad\">\n");
        outs.printf("        <text:p/>\n");
        outs.printf("    </draw:image>\n");
        outs.printf("</draw:frame>\n");
        return true;
        }
    else if (SP_IS_SHAPE(item))
        {
        //g_message("### %s is a shape", nodeName.c_str());
        curve = sp_shape_get_curve(SP_SHAPE(item));
        }
    else if (SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item))
        {
        curve = te_get_layout(item)->convertToCurves();
        }

    if (curve)
        {
        //### Default <path> output

        outs.printf("<draw:path ");
        if (id.size()>0)
            outs.printf("id=\"%s\" ", id.c_str());

        std::map<std::string, std::string>::iterator iter;
        iter = styleLookupTable.find(id);
        if (iter != styleLookupTable.end())
            {
            std::string styleName = iter->second;
            outs.printf("draw:style-name=\"%s\" ", styleName.c_str());
            }

        outs.printf("draw:layer=\"layout\" svg:x=\"%.3fcm\" svg:y=\"%.3fcm\" ",
                       x, y);
	outs.printf("svg:width=\"%.3fcm\" svg:height=\"%.3fcm\" ",
	               width, height);
	outs.printf("svg:viewBox=\"0.0 0.0 %.3f %.3f\"\n",
	               width * 1000.0, height * 1000.0);

	outs.printf("    svg:d=\"");
	writePath(outs, curve->bpath, tf, x, y);
	outs.printf("\"");

	outs.printf(">\n");
        outs.printf("</draw:path>\n");


        sp_curve_unref(curve);
        }

    return true;
}




bool OdfOutput::writeContent(ZipFile &zf, Inkscape::XML::Node *node)
{
    BufferOutputStream bouts;
    OutputStreamWriter outs(bouts);

    time_t tim;
    time(&tim);

    outs.printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<!--\n");
    outs.printf("*************************************************************************\n");
    outs.printf("  file:  content.xml\n");
    outs.printf("  Generated by Inkscape: %s", ctime(&tim)); //ctime has its own <cr>
    outs.printf("  http://www.inkscape.org\n");
    outs.printf("*************************************************************************\n");
    outs.printf("-->\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<office:document-content\n");
    outs.printf("    xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"\n");
    outs.printf("    xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\"\n");
    outs.printf("    xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\"\n");
    outs.printf("    xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\"\n");
    outs.printf("    xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\"\n");
    outs.printf("    xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"\n");
    outs.printf("    xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n");
    outs.printf("    xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n");
    outs.printf("    xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\"\n");
    outs.printf("    xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\"\n");
    outs.printf("    xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\"\n");
    outs.printf("    xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\"\n");
    outs.printf("    xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\"\n");
    outs.printf("    xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\"\n");
    outs.printf("    xmlns:math=\"http://www.w3.org/1998/Math/MathML\"\n");
    outs.printf("    xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\"\n");
    outs.printf("    xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\"\n");
    outs.printf("    xmlns:ooo=\"http://openoffice.org/2004/office\"\n");
    outs.printf("    xmlns:ooow=\"http://openoffice.org/2004/writer\"\n");
    outs.printf("    xmlns:oooc=\"http://openoffice.org/2004/calc\"\n");
    outs.printf("    xmlns:dom=\"http://www.w3.org/2001/xml-events\"\n");
    outs.printf("    xmlns:xforms=\"http://www.w3.org/2002/xforms\"\n");
    outs.printf("    xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n");
    outs.printf("    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
    outs.printf("    xmlns:smil=\"urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0\"\n");
    outs.printf("    xmlns:anim=\"urn:oasis:names:tc:opendocument:xmlns:animation:1.0\"\n");
    outs.printf("    office:version=\"1.0\">\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<office:scripts/>\n");
    outs.printf("\n");
    outs.printf("\n");
    //AffineTransform trans = new AffineTransform();
    //trans.scale(12.0, 12.0);
    outs.printf("<!-- ######### CONVERSION FROM SVG STARTS ######## -->\n");
    outs.printf("<!--\n");
    outs.printf("*************************************************************************\n");
    outs.printf("  S T Y L E S\n");
    outs.printf("  Style entries have been pulled from the svg style and\n");
    outs.printf("  representation attributes in the SVG tree.  The tree elements\n");
    outs.printf("  then refer to them by name, in the ODF manner\n");
    outs.printf("*************************************************************************\n");
    outs.printf("-->\n");
    outs.printf("\n");
    outs.printf("\n");

    if (!writeStyle(outs))
        {
        g_warning("Failed to write styles");
        return false;
        }

    outs.printf("\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<!--\n");
    outs.printf("*************************************************************************\n");
    outs.printf("  D R A W I N G\n");
    outs.printf("  This section is the heart of SVG-ODF conversion.  We are\n");
    outs.printf("  starting with simple conversions, and will slowly evolve\n");
    outs.printf("  into a 'smarter' translation as time progresses.  Any help\n");
    outs.printf("  in improving .odg export is welcome.\n");
    outs.printf("*************************************************************************\n");
    outs.printf("-->\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<office:body>\n");
    outs.printf("<office:drawing>\n");
    outs.printf("<draw:page draw:name=\"page1\" draw:style-name=\"dp1\"\n");
    outs.printf("        draw:master-page-name=\"Default\">\n");
    outs.printf("\n");
    outs.printf("\n");

    if (!writeTree(outs, node))
        {
        g_warning("Failed to convert SVG tree");
        return false;
        }

    outs.printf("\n");
    outs.printf("\n");

    outs.printf("</draw:page>\n");
    outs.printf("</office:drawing>\n");

    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<!-- ######### CONVERSION FROM SVG ENDS ######## -->\n");
    outs.printf("\n");
    outs.printf("\n");

    outs.printf("</office:body>\n");
    outs.printf("</office:document-content>\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("\n");
    outs.printf("<!--\n");
    outs.printf("*************************************************************************\n");
    outs.printf("  E N D    O F    F I L E\n");
    outs.printf("  Have a nice day  - ishmal\n");
    outs.printf("*************************************************************************\n");
    outs.printf("-->\n");
    outs.printf("\n");
    outs.printf("\n");



    //Make our entry
    ZipEntry *ze = zf.newEntry("content.xml", "ODF master content file");
    ze->setUncompressedData(bouts.getBuffer());
    ze->finish();

    return true;
}




/**
 * Descends into the SVG tree, mapping things to ODF when appropriate
 */
void
OdfOutput::save(Inkscape::Extension::Output *mod, SPDocument *doc, gchar const *uri)
{
    //g_message("native file:%s\n", uri);
    documentUri = URI(uri);

    ZipFile zf;
    styleTable.clear();
    styleLookupTable.clear();
    imageTable.clear();
    preprocess(zf, doc->rroot);

    if (!writeManifest(zf))
        {
        g_warning("Failed to write manifest");
        return;
        }

    if (!writeMeta(zf))
        {
        g_warning("Failed to write metafile");
        return;
        }

    if (!writeContent(zf, doc->rroot))
        {
        g_warning("Failed to write content");
        return;
        }

    if (!zf.writeFile(uri))
        {
        return;
        }
}


/**
 * This is the definition of PovRay output.  This function just
 * calls the extension system with the memory allocated XML that
 * describes the data.
*/
void
OdfOutput::init()
{
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension>\n"
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
bool
OdfOutput::check (Inkscape::Extension::Extension *module)
{
    /* We don't need a Key
    if (NULL == Inkscape::Extension::db.get(SP_MODULE_KEY_OUTPUT_POV))
        return FALSE;
    */

    return TRUE;
}



//########################################################################
//# I N P U T
//########################################################################



//#######################
//# L A T E R  !!!  :-)
//#######################













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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
