/*
   Copyright 2005, 2006 by Gerald Friedland, Kristian Jantz and Lars Knipping

   Conversion to C++ for Inkscape by Bob Jamison

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 */
#include "siox.h"

#include <string>

#include <stdarg.h> //for error() and trace()
#include <math.h>   //sqrt(), pow(), round(), etc


namespace org
{
namespace siox
{

//########################################################################
//##  U T I L S    (originally Utils.java)
//########################################################################

/**
 * Collection of auxiliary image processing methods used by the
 * SioxSegmentator mainly for postprocessing.
 *
 * @author G. Friedland, K. Jantz, L. Knipping
 * @version 1.05
 *
 * Conversion to C++ by Bob Jamison
 *
 */


/** Caches color conversion values to speed up RGB->CIELAB conversion.*/
static std::map<unsigned long, CLAB> RGB_TO_LAB;

//forward decls
static void premultiplyMatrix(float alpha, float *cm, int cmSize);
//static float colordiffsq(unsigned long rgb0, unsigned long rgb1);
//static int getAlpha(unsigned long argb);
static int getRed(unsigned long rgb);
static int getGreen(unsigned long rgb);
static int getBlue(unsigned long rgb);
//static unsigned long packPixel(int a, int r, int g, int b);
static CLAB rgbToClab(unsigned long rgb);

/**
 * Applies the morphological dilate operator.
 *
 * Can be used to close small holes in the given confidence matrix.
 *
 * @param cm Confidence matrix to be processed.
 * @param xres Horizontal resolution of the matrix.
 * @param yres Vertical resolution of the matrix.
 */
static void dilate(float *cm, int xres, int yres)
{
    for (int y=0; y<yres; y++) {
        for (int x=0; x<xres-1; x++) {
            int idx=(y*xres)+x;
            if (cm[idx+1]>cm[idx])
                cm[idx]=cm[idx+1];
            }
        }
    for (int y=0; y<yres; y++) {
            for (int x=xres-1; x>=1; x--) {
            int idx=(y*xres)+x;
            if (cm[idx-1]>cm[idx])
                cm[idx]=cm[idx-1];
        }
    }
    for (int y=0; y<yres-1; y++) {
        for (int x=0; x<xres; x++) {
        int idx=(y*xres)+x;
        if (cm[((y+1)*xres)+x] > cm[idx])
            cm[idx]=cm[((y+1)*xres)+x];
        }
    }
    for (int y=yres-1; y>=1; y--) {
        for (int x=0; x<xres; x++) {
        int idx=(y*xres)+x;
        if (cm[((y-1)*xres)+x] > cm[idx])
            cm[idx]=cm[((y-1)*xres)+x];
        }
    }
}

/**
 * Applies the morphological erode operator.
 *
 * @param cm Confidence matrix to be processed.
 * @param xres Horizontal resolution of the matrix.
 * @param yres Vertical resolution of the matrix.
 */
static void erode(float *cm, int xres, int yres)
{
    for (int y=0; y<yres; y++) {
        for (int x=0; x<xres-1; x++) {
            int idx=(y*xres)+x;
            if (cm[idx+1] < cm[idx])
                cm[idx]=cm[idx+1];
        }
    }
    for (int y=0; y<yres; y++) {
        for (int x=xres-1; x>=1; x--) {
            int idx=(y*xres)+x;
            if (cm[idx-1] < cm[idx])
                cm[idx]=cm[idx-1];
        }
    }
    for (int y=0; y<yres-1; y++) {
        for (int x=0; x<xres; x++) {
            int idx=(y*xres)+x;
            if (cm[((y+1)*xres)+x] < cm[idx])
                cm[idx]=cm[((y+1)*xres)+x];
        }
    }
    for (int y=yres-1; y>=1; y--) {
        for (int x=0; x<xres; x++) {
            int idx=(y*xres)+x;
            if (cm[((y-1)*xres)+x] < cm[idx])
                cm[idx]=cm[((y-1)*xres)+x];
        }
    }
}

/**
 * Normalizes the matrix to values to [0..1].
 *
 * @param cm The matrix to be normalized.
 */
static void normalizeMatrix(float *cm, int cmSize)
{
    float max=0.0f;
    for (int i=0; i<cmSize; i++) {
        if (max<cm[i])
            max=cm[i];
    }
    if (max<=0.0)
        return;
    else if (max==1.00)
        return;

    float alpha=1.00f/max;
    premultiplyMatrix(alpha, cm, cmSize);
}

/**
 * Multiplies matrix with the given scalar.
 *
 * @param alpha The scalar value.
 * @param cm The matrix of values be multiplied with alpha.
 * @param cmSize The matrix length.
 */
static void premultiplyMatrix(float alpha, float *cm, int cmSize)
{
    for (int i=0; i<cmSize; i++)
        cm[i]=alpha*cm[i];
}

/**
 * Blurs confidence matrix with a given symmetrically weighted kernel.
 * <P>
 * In the standard case confidence matrix entries are between 0...1 and
 * the weight factors sum up to 1.
 *
 * @param cm The matrix to be smoothed.
 * @param xres Horizontal resolution of the matrix.
 * @param yres Vertical resolution of the matrix.
 * @param f1 Weight factor for the first pixel.
 * @param f2 Weight factor for the mid-pixel.
 * @param f3 Weight factor for the last pixel.
 */
static void smoothcm(float *cm, int xres, int yres,
                     float f1, float f2, float f3)
{
    for (int y=0; y<yres; y++) {
        for (int x=0; x<xres-2; x++) {
            int idx=(y*xres)+x;
            cm[idx]=f1*cm[idx]+f2*cm[idx+1]+f3*cm[idx+2];
        }
    }
    for (int y=0; y<yres; y++) {
        for (int x=xres-1; x>=2; x--) {
            int idx=(y*xres)+x;
            cm[idx]=f3*cm[idx-2]+f2*cm[idx-1]+f1*cm[idx];
        }
    }
    for (int y=0; y<yres-2; y++) {
        for (int x=0; x<xres; x++) {
            int idx=(y*xres)+x;
            cm[idx]=f1*cm[idx]+f2*cm[((y+1)*xres)+x]+f3*cm[((y+2)*xres)+x];
        }
    }
    for (int y=yres-1; y>=2; y--) {
        for (int x=0; x<xres; x++) {
            int idx=(y*xres)+x;
            cm[idx]=f3*cm[((y-2)*xres)+x]+f2*cm[((y-1)*xres)+x]+f1*cm[idx];
        }
    }
}

/**
 * Squared Euclidian distance of p and q.
 * <P>
 * Usage hint: When only comparisons between Euclidian distances without
 * actual values are needed, the squared distance can be preferred
 * for being faster to compute.
 *
 * @param p First euclidian point coordinates.
 * @param pSize Length of coordinate array.
 * @param q Second euclidian point coordinates.
 *        Dimension must not be smaller than that of p.
 *        Any extra dimensions will be ignored.
 * @return Squared euclidian distance of p and q.
 * @see #euclid
 */
static float sqrEuclidianDist(float *p, int pSize, float *q)
{
    float sum=0;
    for (int i=0; i<pSize; i++)
        sum+=(p[i]-q[i])*(p[i]-q[i]);
    return sum;
}

/**
 * Squared Euclidian distance of p and q.
 * <P>
 * Usage hint: When only comparisons between Euclidian distances without
 * actual values are needed, the squared distance can be preferred
 * for being faster to compute.
 *
 * @param p CLAB value
 * @param q second CLAB value
 * @return Squared euclidian distance of p and q.
 * @see #euclid
 */
static float sqrEuclidianDist(const CLAB &p, const CLAB &q)
{
    float sum=0;
    sum += (p.C - q.C) * (p.C - q.C);
    sum += (p.L - q.L) * (p.L - q.L);
    sum += (p.A - q.A) * (p.A - q.A);
    sum += (p.B - q.B) * (p.B - q.B);
    return sum;
}

/**
 * Euclidian distance of p and q.
 *
 * @param p First euclidian point coordinates.
 * @param pSize Length of coordinate array.
 * @param q Second euclidian point coordinates.
 *        Dimension must not be smaller than that of p.
 *        Any extra dimensions will be ignored.
 * @return Squared euclidian distance of p and q.
 * @see #sqrEuclidianDist
 */
/*
static float euclid(float *p, int pSize, float *q)
{
    return (float)sqrt(sqrEuclidianDist(p, pSize, q));
}
*/

/**
 * Computes Euclidian distance of two RGB color values.
 *
 * @param rgb0 First color value.
 * @param rgb1 Second color value.
 * @return Euclidian distance between the two color values.
 */
/*
static float colordiff(long rgb0, long rgb1)
{
    return (float)sqrt(colordiffsq(rgb0, rgb1));
}
*/

/**
 * Computes squared euclidian distance of two RGB color values.
 * <P>
 * Note: Faster to compute than colordiff
 *
 * @param rgb0 First color value.
 * @param rgb1 Second color value.
 * @return Squared Euclidian distance between the two color values.
 */
/*
static float colordiffsq(long rgb0, long rgb1)
{
    int rDist=getRed(rgb0)   - getRed(rgb1);
    int gDist=getGreen(rgb0) - getGreen(rgb1);
    int bDist=getBlue(rgb0)  - getBlue(rgb1);

    return (float)(rDist*rDist+gDist*gDist+bDist*bDist);
}
*/

/**
 * Averages two ARGB colors.
 *
 * @param argb0 First color value.
 * @param argb1 Second color value.
 * @return The averaged ARGB color.
 */
/*
static long average(long argb0, long argb1)
{
    long ret = packPixel(
         (getAlpha(argb0) + getAlpha(argb1))/2,
         (getRed(argb0)   + getRed(argb1)  )/2,
         (getGreen(argb0) + getGreen(argb1))/2,
         (getBlue(argb0)  + getBlue(argb1) )/2);
    return ret;
}
*/

/**
 * Computes squared euclidian distance in CLAB space for two colors
 * given as RGB values.
 *
 * @param rgb0 First color value.
 * @param rgb1 Second color value.
 * @return Squared Euclidian distance in CLAB space.
 */
static float labcolordiffsq(unsigned long rgb1, unsigned long rgb2)
{
    CLAB c1 = rgbToClab(rgb1);
    CLAB c2 = rgbToClab(rgb2);
    float euclid=0.0f;
    euclid += (c1.L - c2.L) * (c1.L - c2.L);
    euclid += (c1.A - c2.A) * (c1.A - c2.A);
    euclid += (c1.B - c2.B) * (c1.B - c2.B);
    return euclid;
}


/**
 * Computes squared euclidian distance in CLAB space for two colors
 * given as RGB values.
 *
 * @param rgb0 First color value.
 * @param rgb1 Second color value.
 * @return Euclidian distance in CLAB space.
 */
static float labcolordiff(unsigned long rgb0, unsigned long rgb1)
{
    return (float)sqrt(labcolordiffsq(rgb0, rgb1));
}


/**
 * Converts 24-bit RGB values to {l,a,b} float values.
 * <P>
 * The conversion used is decribed at
 * <a href="http://www.easyrgb.com/math.php?MATH=M7#text7">CLAB Conversion</a>
 * for reference white D65. Note that that the conversion is computational
 * expensive. Result are cached to speed up further conversion calls.
 *
 * @param rgb RGB color value,
 * @return CLAB color value tripel.
 */
static CLAB rgbToClab(unsigned long rgb)
{
    std::map<unsigned long, CLAB>::iterator iter = RGB_TO_LAB.find(rgb);
    if (iter != RGB_TO_LAB.end())
        {
        CLAB res = iter->second;
        return res;
        }

    int R=getRed(rgb);
    int G=getGreen(rgb);
    int B=getBlue(rgb);

    float var_R=(R/255.0f); //R = From 0 to 255
    float var_G=(G/255.0f); //G = From 0 to 255
    float var_B=(B/255.0f); //B = From 0 to 255

    if (var_R>0.04045)
        var_R=(float) pow((var_R+0.055f)/1.055f, 2.4);
    else
        var_R=var_R/12.92f;

    if (var_G>0.04045)
        var_G=(float) pow((var_G+0.055f)/1.055f, 2.4);
    else
        var_G=var_G/12.92f;

    if (var_B>0.04045)
        var_B=(float) pow((var_B+0.055f)/1.055f, 2.4);
    else
        var_B=var_B/12.92f;

    var_R=var_R*100.0f;
    var_G=var_G*100.0f;
    var_B=var_B*100.0f;

    // Observer. = 2�, Illuminant = D65
    float X=var_R*0.4124f + var_G*0.3576f + var_B*0.1805f;
    float Y=var_R*0.2126f + var_G*0.7152f + var_B*0.0722f;
    float Z=var_R*0.0193f + var_G*0.1192f + var_B*0.9505f;

    float var_X=X/95.047f;
    float var_Y=Y/100.0f;
    float var_Z=Z/108.883f;

    if (var_X>0.008856f)
        var_X=(float) pow(var_X, 0.3333f);
    else
        var_X=(7.787f*var_X)+(16.0f/116.0f);

    if (var_Y>0.008856f)
        var_Y=(float) pow(var_Y, 0.3333f);
    else
        var_Y=(7.787f*var_Y)+(16.0f/116.0f);

    if (var_Z>0.008856f)
        var_Z=(float) pow(var_Z, 0.3333f);
    else
        var_Z=(7.787f*var_Z)+(16.0f/116.0f);

    CLAB lab((116.0f*var_Y)-16.0f , 500.0f*(var_X-var_Y), 200.0f*(var_Y-var_Z));

    RGB_TO_LAB[rgb] = lab;

    return lab;
}

/**
 * Converts an CLAB value to a RGB color value.
 * <P>
 * This is the reverse operation to rgbToClab.
 * @param clab CLAB value.
 * @return RGB value.
 * @see #rgbToClab
 */
/*
static long clabToRGB(const CLAB &clab)
{
    float L=clab.L;
    float a=clab.A;
    float b=clab.B;

    float var_Y=(L+16.0f)/116.0f;
    float var_X=a/500.0f+var_Y;
    float var_Z=var_Y-b/200.0f;

    float var_yPow3=(float)pow(var_Y, 3.0);
    float var_xPow3=(float)pow(var_X, 3.0);
    float var_zPow3=(float)pow(var_Z, 3.0);

    if (var_yPow3>0.008856f)
        var_Y=var_yPow3;
    else
        var_Y=(var_Y-16.0f/116.0f)/7.787f;

    if (var_xPow3>0.008856f)
        var_X=var_xPow3;
    else
        var_X=(var_X-16.0f/116.0f)/7.787f;

    if (var_zPow3>0.008856f)
        var_Z=var_zPow3;
    else
        var_Z=(var_Z-16.0f/116.0f)/7.787f;

    float X=95.047f  * var_X; //ref_X= 95.047  Observer=2�, Illuminant=D65
    float Y=100.0f   * var_Y; //ref_Y=100.000
    float Z=108.883f * var_Z; //ref_Z=108.883

    var_X=X/100.0f; //X = From 0 to ref_X
    var_Y=Y/100.0f; //Y = From 0 to ref_Y
    var_Z=Z/100.0f; //Z = From 0 to ref_Y

    float var_R=(float)(var_X *  3.2406f + var_Y * -1.5372f + var_Z * -0.4986f);
    float var_G=(float)(var_X * -0.9689f + var_Y *  1.8758f + var_Z *  0.0415f);
    float var_B=(float)(var_X *  0.0557f + var_Y * -0.2040f + var_Z *  1.0570f);

    if (var_R>0.0031308f)
        var_R=(float)(1.055f*pow(var_R, (1.0f/2.4f))-0.055f);
    else
        var_R=12.92f*var_R;

    if (var_G>0.0031308f)
        var_G=(float)(1.055f*pow(var_G, (1.0f/2.4f))-0.055f);
    else
        var_G=12.92f*var_G;

    if (var_B>0.0031308f)
        var_B=(float)(1.055f*pow(var_B, (1.0f/2.4f))-0.055f);
    else
        var_B=12.92f*var_B;

    int R = (int)lround(var_R*255.0f);
    int G = (int)lround(var_G*255.0f);
    int B = (int)lround(var_B*255.0f);

    return packPixel(0xFF, R, G, B);
}
*/

/**
 * Sets the alpha byte of a pixel.
 *
 * Constructs alpha to values from 0 to 255.
 * @param alpha Alpha value from 0 (transparent) to 255 (opaque).
 * @param rgb The 24bit rgb color to be combined with the alpga value.
 * @return An ARBG calor value.
 */
static long setAlpha(int alpha, unsigned long rgb)
{
    if (alpha>255)
        alpha=0;
    else if (alpha<0)
        alpha=0;
    return (alpha<<24)|(rgb&0xFFFFFF);
}

/**
 * Sets the alpha byte of a pixel.
 *
 * Constricts alpha to values from 0 to 255.
 * @param alpha Alpha value from 0.0f (transparent) to 1.0f (opaque).
 * @param rgb The 24bit rgb color to be combined with the alpga value.
 * @return An ARBG calor value.
 */
static long setAlpha(float alpha, unsigned long rgb)
{
    return setAlpha((int)(255.0f*alpha), rgb);
}

/**
 * Limits the values of a,r,g,b to values from 0 to 255 and puts them
 * together into an 32 bit integer.
 *
 * @param a Alpha part, the first byte.
 * @param r Red part, the second byte.
 * @param g Green part, the third byte.
 * @param b Blue part, the fourth byte.
 * @return A ARBG value packed to an int.
 */
/*
static long packPixel(int a, int r, int g, int b)
{
    if (a<0)
        a=0;
    else if (a>255)
        a=255;

    if (r<0)
        r=0;
    else if (r>255)
        r=255;

    if (g<0)
        g=0;
    else if (g>255)
        g=255;

    if (b<0)
        b=0;
    else if (b>255)
        b=255;

    return (a<<24)|(r<<16)|(g<<8)|b;
}
*/

/**
 * Returns the alpha component of an ARGB value.
 *
 * @param argb An ARGB color value.
 * @return The alpha component, ranging from 0 to 255.
 */
/*
static int getAlpha(unsigned long argb)
{
    return (argb>>24)&0xFF;
}
*/

/**
 * Returns the red component of an (A)RGB value.
 *
 * @param rgb An (A)RGB color value.
 * @return The red component, ranging from 0 to 255.
 */
static int getRed(unsigned long rgb)
{
    return (rgb>>16)&0xFF;
}


/**
 * Returns the green component of an (A)RGB value.
 *
 * @param rgb An (A)RGB color value.
 * @return The green component, ranging from 0 to 255.
 */
static int getGreen(unsigned long rgb)
{
    return (rgb>>8)&0xFF;
}

/**
 * Returns the blue component of an (A)RGB value.
 *
 * @param rgb An (A)RGB color value.
 * @return The blue component, ranging from 0 to 255.
 */
static int getBlue(unsigned long rgb)
{
    return (rgb)&0xFF;
}

/**
 * Returns a string representation of a CLAB value.
 *
 * @param clab The CLAB value.
 * @param clabSize Size of the CLAB value.
 * @return A string representation of the CLAB value.
 */
/*
static std::string clabToString(const CLAB &clab)
{
    std::string buff;
    char nbuf[60];
    snprintf(nbuf, 59, "%5.3f, %5.3f, %5.3f", clab.L, clab.A, clab.B);
    buff = nbuf;
    return buff;
}
*/

//########################################################################
//##  C O L O R    S I G N A T U R E    (originally ColorSignature.java)
//########################################################################

/**
 * Representation of a color signature.
 * <br><br>
 * This class implements a clustering algorithm based on a modified kd-tree.
 * The splitting rule is to simply divide the given interval into two equally
 * sized subintervals.
 * In the <code>stageone()</code>, approximate clusters are found by building
 * up such a tree and stopping when an interval at a node has become smaller
 * than the allowed cluster diameter, which is given by <code>limits</code>.
 * At this point, clusters may be split in several nodes.<br>
 * Therefore, in <code>stagetwo()</code>, nodes that belong to several clusters
 * are recombined by another k-d tree clustering on the prior cluster
 * centroids. To guarantee a proper level of abstraction, clusters that contain
 * less than 0.01% of the pixels of the entire sample are removed. Please
 * refer to the file NOTICE to get links to further documentation.
 *
 * @author Gerald Friedland, Lars Knipping
 * @version 1.02
 *
 * Conversion to C++ by Bob Jamison
 *
 */

/**
 * Stage one of clustering.
 * @param points float[][] the input points in LAB space
 * @param depth int used for recursion, start with 0
 * @param clusters ArrayList an Arraylist to store the clusters
 * @param limits float[] the cluster diameters
 */
static void stageone(std::vector<CLAB> &points,
                     int depth,
                     std::vector< std::vector<CLAB> > &clusters,
                     float *limits)
{
    if (points.size() < 1)
        return;

    int dims=3;
    int curdim=depth%dims;
    float min = 0.0f;
    float max = 0.0f;
    if (curdim == 0)
        {
        min=points[0].C;
        max=points[0].C;
        // find maximum and minimum
        for (unsigned int i=1; i<points.size(); i++) {
            if (min>points[i].C)
                min=points[i].C;
            if (max<points[i].C)
                max=points[i].C;
            }
        }
    else if (curdim == 1)
        {
        min=points[0].L;
        max=points[0].L;
        // find maximum and minimum
        for (unsigned int i=1; i<points.size(); i++) {
            if (min>points[i].L)
                min=points[i].L;
            if (max<points[i].L)
                max=points[i].L;
            }
        }
    else if (curdim == 2)
        {
        min=points[0].A;
        max=points[0].A;
        // find maximum and minimum
        for (unsigned int i=1; i<points.size(); i++) {
            if (min>points[i].A)
                min=points[i].A;
            if (max<points[i].A)
                max=points[i].A;
            }
        }
    else if (curdim == 3)
        {
        min=points[0].B;
        max=points[0].B;
        // find maximum and minimum
        for (unsigned int i=1; i<points.size(); i++) {
            if (min>points[i].B)
                min=points[i].B;
            if (max<points[i].B)
                max=points[i].B;
            }
        }

    if (max-min>limits[curdim]) { // Split according to Rubner-Rule
        // split
        float pivotvalue=((max-min)/2.0f)+min;

        std::vector<CLAB> smallerpoints; // allocate mem
        std::vector<CLAB> biggerpoints;

        for (unsigned int i=0; i<points.size(); i++) { // do actual split
            float v = 0.0f;
            if (curdim==0)
                v = points[i].C;
            else if (curdim==1)
                v = points[i].L;
            else if (curdim==2)
                v = points[i].A;
            else if (curdim==3)
                v = points[i].B;
            if (v <= pivotvalue) {
                smallerpoints.push_back(points[i]);
            } else {
                biggerpoints.push_back(points[i]);
            }
        } // create subtrees
        stageone(smallerpoints, depth+1, clusters, limits);
        stageone(biggerpoints,  depth+1, clusters, limits);
    } else { // create leave
        clusters.push_back(points);
    }
}

/**
 * Stage two of clustering.
 * @param points float[][] the input points in LAB space
 * @param depth int used for recursion, start with 0
 * @param clusters ArrayList an Arraylist to store the clusters
 * @param limits float[] the cluster diameters
 * @param total int the total number of points as given to stageone
 * @param threshold should be 0.01 - abstraction threshold
 */
static void stagetwo(std::vector<CLAB> &points,
                     int depth,
                     std::vector< std::vector<CLAB> > &clusters,
                     float *limits, int total, float threshold)
{
    if (points.size() < 1)
        return;

    int curdim=depth%3; // without cardinality
    float min = 0.0f;
    float max = 0.0f;
    if (curdim == 0)
        {
        min=points[0].L;
        max=points[0].L;
        // find maximum and minimum
        for (unsigned int i=1; i<points.size(); i++) {
            if (min>points[i].L)
                min=points[i].L;
            if (max<points[i].L)
                max=points[i].L;
            }
        }
    else if (curdim == 1)
        {
        min=points[0].A;
        max=points[0].A;
        // find maximum and minimum
        for (unsigned int i=1; i<points.size(); i++) {
            if (min>points[i].A)
                min=points[i].A;
            if (max<points[i].A)
                max=points[i].A;
            }
        }
    else if (curdim == 2)
        {
        min=points[0].B;
        max=points[0].B;
        // find maximum and minimum
        for (unsigned int i=1; i<points.size(); i++) {
            if (min>points[i].B)
                min=points[i].B;
            if (max<points[i].B)
                max=points[i].B;
            }
        }


    if (max-min>limits[curdim]) { // Split according to Rubner-Rule
        // split
        float pivotvalue=((max-min)/2.0f)+min;

        std::vector<CLAB> smallerpoints; // allocate mem
        std::vector<CLAB> biggerpoints;

        for (unsigned int i=0; i<points.size(); i++) { // do actual split
            float v = 0.0f;
	    if (curdim==0)
	        v = points[i].L;
	    else if (curdim==1)
	        v = points[i].A;
	    else if (curdim==2)
	        v = points[i].B;

            if (v <= pivotvalue) {
                smallerpoints.push_back(points[i]);
            } else {
                biggerpoints.push_back(points[i]);
            }
        } // create subtrees
        stagetwo(smallerpoints, depth+1, clusters, limits, total, threshold);
        stagetwo(biggerpoints,  depth+1, clusters, limits, total, threshold);
    } else { // create leave
        float sum=0.0;
        for (unsigned int i=0; i<points.size(); i++) {
            sum+=points[i].B;
        }
        if (((sum*100.0)/total)>=threshold) {
            CLAB point;
            for (unsigned int i=0; i<points.size(); i++) {
                point.C += points[i].C;
                point.L += points[i].L;
                point.A += points[i].A;
                point.B += points[i].B;
            }
            point.C /= points.size();
            point.L /= points.size();
            point.A /= points.size();
            point.B /= points.size();

            std::vector<CLAB> newCluster;
            newCluster.push_back(point);
            clusters.push_back(newCluster);
        }
    }
}

/**
 * Create a color signature for the given set of pixels.
 * @param input float[][] a set of pixels in LAB space
 * @param length int the number of pixels that should be processed from the input
 * @param limits float[] the cluster diameters for each dimension
 * @param threshold float the abstraction threshold
 * @return float[][] a color siganture containing cluster centroids in LAB space
 */
static std::vector<CLAB> createSignature(std::vector<CLAB> &input,
                                         float *limits, float threshold)
{
    std::vector< std::vector<CLAB> > clusters1;
    std::vector< std::vector<CLAB> > clusters2;

    stageone(input, 0, clusters1, limits);

    std::vector<CLAB> centroids;
    for (unsigned int i=0; i<clusters1.size(); i++) {
        std::vector<CLAB> cluster = clusters1[i];
        CLAB centroid; // +1 for the cardinality
        for (unsigned int k=0; k<cluster.size(); k++) {
            centroid.L += cluster[k].L;
            centroid.A += cluster[k].A;
            centroid.B += cluster[k].B;
        }
        centroid.C =  cluster.size();
        centroid.L /= cluster.size();
        centroid.A /= cluster.size();
        centroid.B /= cluster.size();

        centroids.push_back(centroid);
    }
    stagetwo(centroids, 0, clusters2, limits, input.size(), threshold); // 0.1 -> see paper by tomasi

    std::vector<CLAB> res;
    for (unsigned int i=0 ; i<clusters2.size() ; i++)
        for (unsigned int j=0 ; j<clusters2[i].size() ; j++)
            res.push_back(clusters2[i][j]);

    return res;
}



//########################################################################
//##  S I O X  S E G M E N T A T O R    (originally SioxSegmentator.java)
//########################################################################

//### NOTE: Doxygen comments are in siox.h

/** Confidence corresponding to a certain foreground region (equals one). */
const float SioxSegmentator::CERTAIN_FOREGROUND_CONFIDENCE=1.0f;

/** Confidence for a region likely being foreground.*/
const float SioxSegmentator::FOREGROUND_CONFIDENCE=0.8f;

/** Confidence for foreground or background type being equally likely.*/
const float SioxSegmentator::UNKNOWN_REGION_CONFIDENCE=0.5f;

/** Confidence for a region likely being background.*/
const float SioxSegmentator::BACKGROUND_CONFIDENCE=0.1f;

/** Confidence corresponding to a certain background reagion (equals zero). */
const float SioxSegmentator::CERTAIN_BACKGROUND_CONFIDENCE=0.0f;


SioxSegmentator::SioxSegmentator(int w, int h, float *limitsArg, int limitsSize)
{

    imgWidth   = w;
    imgHeight  = h;
    labelField = new int[imgWidth*imgHeight];

    if (!limitsArg) {
        limits = new float[3];
        limits[0] = 0.64f;
        limits[1] = 1.28f;
        limits[2] = 2.56f;
    } else {
        limits = new float[limitsSize];
        for (int i=0 ; i<limitsSize ; i++)
            limits[i] = limitsArg[i];
    }

    float negLimits[3];
    negLimits[0] = -limits[0];
    negLimits[1] = -limits[1];
    negLimits[2] = -limits[2];
    clusterSize = sqrEuclidianDist(limits, 3, negLimits);

    segmentated=false;

    origImage = NULL;
}

SioxSegmentator::~SioxSegmentator()
{
    delete labelField;
    delete limits;
    delete origImage;
}


/**
 * Error logging
 */
void SioxSegmentator::error(char *fmt, ...)
{
    va_list args;
    fprintf(stderr, "SioxSegmentator error:");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args) ;
    fprintf(stderr, "\n");
}

/**
 * Trace logging
 */
void SioxSegmentator::trace(char *fmt, ...)
{
    va_list args;
    fprintf(stderr, "SioxSegmentator:");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args) ;
    fprintf(stderr, "\n");
}



bool SioxSegmentator::segmentate(unsigned long *image, int imageSize,
                                 float *cm, int cmSize,
                                 int smoothness, double sizeFactorToKeep)
{
    segmentated=false;

    hs.clear();

    // save image for drb
    origImage=new long[imageSize];
    for (int i=0 ; i<imageSize ; i++)
        origImage[i] = image[i];

    // create color signatures
    for (int i=0; i<cmSize; i++) {
        if (cm[i]<=BACKGROUND_CONFIDENCE)
            knownBg.push_back(rgbToClab(image[i]));
        else if (cm[i]>=FOREGROUND_CONFIDENCE)
            knownFg.push_back(rgbToClab(image[i]));
    }

    bgSignature = createSignature(knownBg, limits, BACKGROUND_CONFIDENCE);
    fgSignature = createSignature(knownFg, limits, BACKGROUND_CONFIDENCE);

    if (bgSignature.size() < 1) {
        // segmentation impossible
        return false;
    }

    // classify using color signatures,
    // classification cached in hashmap for drb and speedup purposes
    for (int i=0; i<cmSize; i++) {
        if (cm[i]>=FOREGROUND_CONFIDENCE) {
            cm[i]=CERTAIN_FOREGROUND_CONFIDENCE;
            continue;
        }
        if (cm[i]>BACKGROUND_CONFIDENCE) {
            bool isBackground=true;
            std::map<unsigned long, Tupel>::iterator iter = hs.find(i);
            Tupel tupel(0.0f, 0, 0.0f, 0);
            if (iter == hs.end()) {
                CLAB lab = rgbToClab(image[i]);
                float minBg = sqrEuclidianDist(lab, bgSignature[0]);
                int minIndex=0;
                for (unsigned int j=1; j<bgSignature.size(); j++) {
                    float d = sqrEuclidianDist(lab, bgSignature[j]);
                    if (d<minBg) {
                        minBg=d;
                        minIndex=j;
                    }
                }
                tupel.minBgDist=minBg;
                tupel.indexMinBg=minIndex;
                float minFg = 1.0e6f;
                minIndex=-1;
                for (unsigned int j=0 ; j<fgSignature.size() ; j++) {
                    float d = sqrEuclidianDist(lab, fgSignature[j]);
                    if (d<minFg) {
                        minFg=d;
                        minIndex=j;
                    }
                }
                tupel.minFgDist=minFg;
                tupel.indexMinFg=minIndex;
                if (fgSignature.size()==0) {
                    isBackground=(minBg<=clusterSize);
                    // remove next line to force behaviour of old algorithm
                    error("foreground signature does not exist");
                    return false;
                } else {
                    isBackground=minBg<minFg;
                }
                hs[image[i]] = tupel;
            } else {
                isBackground=tupel.minBgDist<=tupel.minFgDist;
            }
            if (isBackground) {
                cm[i]=CERTAIN_BACKGROUND_CONFIDENCE;
            } else {
                cm[i]=CERTAIN_FOREGROUND_CONFIDENCE;
            }
        } else {
            cm[i]=CERTAIN_BACKGROUND_CONFIDENCE;
        }
    }

    // postprocessing
    smoothcm(cm, imgWidth, imgHeight, 0.33f, 0.33f, 0.33f); // average
    normalizeMatrix(cm, cmSize);
    erode(cm, imgWidth, imgHeight);
    keepOnlyLargeComponents(cm, cmSize, UNKNOWN_REGION_CONFIDENCE, sizeFactorToKeep);

    for (int i=0; i<smoothness; i++) {
        smoothcm(cm, imgWidth, imgHeight, 0.33f, 0.33f, 0.33f); // average
    }

    normalizeMatrix(cm, cmSize);

    for (int i=0; i<cmSize; i++) {
        if (cm[i]>=UNKNOWN_REGION_CONFIDENCE) {
            cm[i]=CERTAIN_FOREGROUND_CONFIDENCE;
        } else {
            cm[i]=CERTAIN_BACKGROUND_CONFIDENCE;
        }
    }

    keepOnlyLargeComponents(cm, cmSize, UNKNOWN_REGION_CONFIDENCE, sizeFactorToKeep);
    fillColorRegions(cm, cmSize, image);
    dilate(cm, imgWidth, imgHeight);

    segmentated=true;
    return true;
}



void SioxSegmentator::keepOnlyLargeComponents(float *cm, int cmSize,
                                              float threshold,
                                              double sizeFactorToKeep)
{
    int idx = 0;
    for (int i=0 ; i<imgHeight ; i++)
        for (int j=0 ; j<imgWidth ; j++)
            labelField[idx++] = -1;

    int curlabel = 0;
    int maxregion= 0;
    int maxblob  = 0;

    // slow but easy to understand:
    std::vector<int> labelSizes;
    for (int i=0 ; i<cmSize ; i++) {
        regionCount=0;
        if (labelField[i]==-1 && cm[i]>=threshold) {
            regionCount=depthFirstSearch(cm, i, threshold, curlabel++);
            labelSizes.push_back(regionCount);
        }

        if (regionCount>maxregion) {
            maxregion=regionCount;
            maxblob=curlabel-1;
        }
    }

    for (int i=0 ; i<cmSize ; i++) {
        if (labelField[i]!=-1) {
            // remove if the component is to small
            if (labelSizes[labelField[i]]*sizeFactorToKeep < maxregion)
                cm[i]=CERTAIN_BACKGROUND_CONFIDENCE;

            // add maxblob always to foreground
            if (labelField[i]==maxblob)
                cm[i]=CERTAIN_FOREGROUND_CONFIDENCE;
        }
    }
}



int SioxSegmentator::depthFirstSearch(float *cm, int i, float threshold, int curLabel)
{
    // stores positions of labeled pixels, where the neighbours
    // should still be checked for processing:
    std::vector<int> pixelsToVisit;
    int componentSize=0;
    if (labelField[i]==-1 && cm[i]>=threshold) { // label #i
        labelField[i] = curLabel;
        ++componentSize;
        pixelsToVisit.push_back(i);
    }
    while (pixelsToVisit.size() > 0) {
        int pos=pixelsToVisit[pixelsToVisit.size()-1];
        pixelsToVisit.erase(pixelsToVisit.end()-1);
        int x=pos%imgWidth;
        int y=pos/imgWidth;
        // check all four neighbours
        int left = pos-1;
        if (x-1>=0 && labelField[left]==-1 && cm[left]>=threshold) {
            labelField[left]=curLabel;
            ++componentSize;
            pixelsToVisit.push_back(left);
        }
        int right = pos+1;
        if (x+1<imgWidth && labelField[right]==-1 && cm[right]>=threshold) {
            labelField[right]=curLabel;
            ++componentSize;
            pixelsToVisit.push_back(right);
        }
        int top = pos-imgWidth;
        if (y-1>=0 && labelField[top]==-1 && cm[top]>=threshold) {
            labelField[top]=curLabel;
            ++componentSize;
            pixelsToVisit.push_back(top);
        }
        int bottom = pos+imgWidth;
        if (y+1<imgHeight && labelField[bottom]==-1
            && cm[bottom]>=threshold) {
            labelField[bottom]=curLabel;
            ++componentSize;
            pixelsToVisit.push_back(bottom);
        }
    }
    return componentSize;
}

void SioxSegmentator::subpixelRefine(int x, int y, int brushmode,
                             float threshold, float *cf, int brushsize)
{
    subpixelRefine(x-brushsize, y-brushsize,
                   2*brushsize, 2*brushsize,
                   brushmode, threshold, cf);
}


bool SioxSegmentator::subpixelRefine(int xa, int ya, int dx, int dy,
                                     int brushmode,
                                     float threshold, float *cf)
{
    if (!segmentated) {
        error("no segmentation yet");
        return false;
    }

    int x0 = (xa > 0) ? xa : 0;
    int y0 = (ya > 0) ? ya : 0;

    int xTo = (imgWidth  - 1 < xa+dx ) ? imgWidth-1  : xa+dx;
    int yTo = (imgHeight - 1 < ya+dy ) ? imgHeight-1 : ya+dy;

    for (int ey=y0; ey<yTo; ++ey) {
        for (int ex=x0; ex<xTo; ++ex) {
            /*  we are using a rect, not necessary
            if (!area.contains(ex, ey)) {
                continue;
            }
            */
            unsigned long val=origImage[ey*imgWidth+ex];
            unsigned long orig=val;
            float minDistBg = 0.0f;
            float minDistFg = 0.0f;
            std::map<unsigned long, Tupel>::iterator iter = hs.find(val);
            if (iter != hs.end()) {
                minDistBg=(float) sqrt((float)iter->second.minBgDist);
                minDistFg=(float) sqrt((float)iter->second.minFgDist);
            } else {
                continue;
            }
            if (ADD_EDGE == brushmode) { // handle adder
                if (cf[ey*imgWidth+ex]<FOREGROUND_CONFIDENCE) { // postprocessing wins
                  float alpha;
                  if (minDistFg==0) {
                      alpha=CERTAIN_FOREGROUND_CONFIDENCE;
                  } else {
                      alpha = (minDistBg/minDistFg < CERTAIN_FOREGROUND_CONFIDENCE) ?
                          minDistBg/minDistFg : CERTAIN_FOREGROUND_CONFIDENCE;
                  }
                  if (alpha<threshold) { // background with certain confidence decided by user.
                      alpha=CERTAIN_BACKGROUND_CONFIDENCE;
                  }
                  val = setAlpha(alpha, orig);
                  cf[ey*imgWidth+ex]=alpha;
                }
            } else if (SUB_EDGE == brushmode) { // handle substractor
                if (cf[ey*imgWidth+ex]>FOREGROUND_CONFIDENCE) {
                    // foreground, we want to take something away
                    float alpha;
                    if (minDistBg==0) {
                        alpha=CERTAIN_BACKGROUND_CONFIDENCE;
                    } else {
                        alpha=CERTAIN_FOREGROUND_CONFIDENCE-
                            (minDistFg/minDistBg < CERTAIN_FOREGROUND_CONFIDENCE) ? // more background -> >1
                             minDistFg/minDistBg : CERTAIN_FOREGROUND_CONFIDENCE;
                        // bg = gf -> 1
                        // more fg -> <1
                    }
                    if (alpha<threshold) { // background with certain confidence decided by user
                        alpha=CERTAIN_BACKGROUND_CONFIDENCE;
                    }
                    val = setAlpha(alpha, orig);
                    cf[ey*imgWidth+ex]=alpha;
                }
            } else {
                error("unknown brush mode: %d", brushmode);
                return false;
            }
        }
    }
    return true;
}



void SioxSegmentator::fillColorRegions(float *cm, int cmSize, unsigned long *image)
{
    int idx = 0;
    for (int i=0 ; i<imgHeight ; i++)
        for (int j=0 ; i<imgWidth ; j++)
            labelField[idx++] = -1;

    //int maxRegion=0; // unused now
    std::vector<int> pixelsToVisit;
    for (int i=0; i<cmSize; i++) { // for all pixels
        if (labelField[i]!=-1 || cm[i]<UNKNOWN_REGION_CONFIDENCE) {
            continue; // already visited or bg
        }
        int origColor=image[i];
        int curLabel=i+1;
        labelField[i]=curLabel;
        cm[i]=CERTAIN_FOREGROUND_CONFIDENCE;
        // int componentSize = 1;
        pixelsToVisit.push_back(i);
        // depth first search to fill region
        while (pixelsToVisit.size() > 0) {
            int pos=pixelsToVisit[pixelsToVisit.size()-1];
            pixelsToVisit.erase(pixelsToVisit.end()-1);
            int x=pos%imgWidth;
            int y=pos/imgWidth;
            // check all four neighbours
            int left = pos-1;
            if (x-1>=0 && labelField[left]==-1
                && labcolordiff(image[left], origColor)<1.0) {
                labelField[left]=curLabel;
                cm[left]=CERTAIN_FOREGROUND_CONFIDENCE;
                // ++componentSize;
                pixelsToVisit.push_back(left);
            }
            int right = pos+1;
            if (x+1<imgWidth && labelField[right]==-1
                && labcolordiff(image[right], origColor)<1.0) {
                labelField[right]=curLabel;
                cm[right]=CERTAIN_FOREGROUND_CONFIDENCE;
                // ++componentSize;
                pixelsToVisit.push_back(right);
            }
            int top = pos-imgWidth;
            if (y-1>=0 && labelField[top]==-1
                && labcolordiff(image[top], origColor)<1.0) {
                labelField[top]=curLabel;
                cm[top]=CERTAIN_FOREGROUND_CONFIDENCE;
                // ++componentSize;
                pixelsToVisit.push_back(top);
            }
            int bottom = pos+imgWidth;
            if (y+1<imgHeight && labelField[bottom]==-1
                && labcolordiff(image[bottom], origColor)<1.0) {
                labelField[bottom]=curLabel;
                cm[bottom]=CERTAIN_FOREGROUND_CONFIDENCE;
                // ++componentSize;
                pixelsToVisit.push_back(bottom);
            }
        }
        //if (componentSize>maxRegion) {
        //    maxRegion=componentSize;
        //}
    }
}














} //namespace siox
} //namespace org



