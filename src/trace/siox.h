#ifndef __SIOX_SEGMENTATOR_H__
#define __SIOX_SEGMENTATOR_H__
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

#include <map>
#include <vector>

namespace org
{
namespace siox
{

/**
 * Image segmentator based on
 *<em>SIOX: Simple Interactive Object Extraction</em>.
 * <P>
 * To segmentate an image one has to perform the following steps.
 * <OL><LI>Construct an instance of <code>SioxSegmentator</code>.
 * </LI><LI>Create a confidence matrix, where each entry marks its
 *      corresponding image pixel to belong to the foreground, to the
 *      background, or being of unknown type.
 * </LI><LI>Call <code>segmentate</code> on the image with the confidence
 *       matrix. This stores the result as new foreground confidence into
 *       the confidence matrix, with each entry being either
 *       zero (<code>CERTAIN_BACKGROUND_CONFIDENCE</code>) or one
 *       (<code>CERTAIN_FOREGROUND_CONFIDENCE</code>).
 * </LI><LI>Optionally call <code>subpixelRefine</code> to areas
 *      where pixels contain both foreground and background (e.g.
 *      object borders or highly detailed features like flowing hairs).
 *      The pixel are then assigned confidence values bwetween zero and
 *      one to give them a measure of "foregroundness".
 *      This step may be repeated as often as needed.
 * </LI></OL>
 * <P>
 * For algorithm documentation refer to
 * G. Friedland, K. Jantz, L. Knipping, R. Rojas:<i>
 * Image Segmentation by Uniform Color Clustering
 *  -- Approach and Benchmark Results</i>,
 * <A HREF="http://www.inf.fu-berlin.de/inst/pubs/tr-b-05-07.pdf">Technical Report B-05-07</A>,
 * Department of Computer Science, Freie Universitaet Berlin, June 2005.<br>
 * <P>
 * See <A HREF="http://www.siox.org/" target="_new">http://www.siox.org</A> for more information.<br>
 * <P>
 * Algorithm idea by Gerald Friedland.
 *
 * @author Gerald Friedland, Kristian Jantz, Lars Knipping
 * @version 1.12
 */

/**
 * Helper class for storing the minimum distances to a cluster centroid
 * in background and foreground and the index to the centroids in each
 * signature for a given color.
 */
class Tupel {
public:

    Tupel()
        {
        minBgDist  = 0.0f;
        indexMinBg = 0;
        minFgDist  = 0.0f;
        indexMinFg = 0;
        }
    Tupel(float minBgDistArg, long indexMinBgArg,
          float minFgDistArg, long indexMinFgArg)
        {
	minBgDist  = minBgDistArg;
	indexMinBg = indexMinBgArg;
	minFgDist  = minFgDistArg;
	indexMinFg = indexMinFgArg;
        }
    Tupel(const Tupel &other)
        {
	minBgDist  = other.minBgDist;
	indexMinBg = other.indexMinBg;
	minFgDist  = other.minFgDist;
	indexMinFg = other.indexMinFg;
        }
    Tupel &operator=(const Tupel &other)
        {
	minBgDist  = other.minBgDist;
	indexMinBg = other.indexMinBg;
	minFgDist  = other.minFgDist;
	indexMinFg = other.indexMinFg;
	return *this;
        }
    virtual ~Tupel()
        {}

    float minBgDist;
    long  indexMinBg;
    float minFgDist;
    long  indexMinFg;

 };


class CLAB
{
public:
    CLAB()
        {
        C = L = A = B = 0.0f;
        }
    CLAB(float lArg, float aArg, float bArg)
        {
        C = 0.0f;
        L = lArg;
        A = aArg;
        B = bArg;
        }
    CLAB(const CLAB &other)
        {
        C = other.C;
        L = other.L;
        A = other.A;
        B = other.B;
        }
    CLAB &operator=(const CLAB &other)
        {
        C = other.C;
        L = other.L;
        A = other.A;
        B = other.B;
        return *this;
        }
    virtual ~CLAB()
        {}

    float C;
    float L;
    float A;
    float B;
};


class SioxSegmentator
{
public:

    /** Confidence corresponding to a certain foreground region (equals one). */
    static const float CERTAIN_FOREGROUND_CONFIDENCE;  //=1.0f;

    /** Confidence for a region likely being foreground.*/
    static const float FOREGROUND_CONFIDENCE;  //=0.8f;

    /** Confidence for foreground or background type being equally likely.*/
    static const float UNKNOWN_REGION_CONFIDENCE;  //=0.5f;

    /** Confidence for a region likely being background.*/
    static const float BACKGROUND_CONFIDENCE;  //=0.1f;

    /** Confidence corresponding to a certain background reagion (equals zero). */
    static const float CERTAIN_BACKGROUND_CONFIDENCE;  //=0.0f;


    /**
     * Constructs a SioxSegmentator Object to be used for image segmentation.
     *
     * @param w X resolution of the image to be segmentated.
     * @param h Y resolution of the image to be segmentated.
     * @param limits Size of the cluster on LAB axises.
     *        If <code>null</code>, the default value {0.64f,1.28f,2.56f}
     *        is used.
     */
    SioxSegmentator(int w, int h, float *limitsArg, int limitsSize);

    /**
     * Destructor
     */
    virtual ~SioxSegmentator();


    /**
     * Segmentates the given image with information from the confidence
     * matrix.
     * <P>
     * The confidence entries  of <code>BACKGROUND_CONFIDENCE</code> or less
     * are mark known background pixel for the segmentation, those
     * of at least <code>FOREGROUND_CONFIDENCE</code> mark known
     * foreground pixel for the segmentation. Any other entry is treated
     * as region of unknown affiliation.
     * <P>
     * As result, each pixel is classified either as foregroound or
     * background, stored back into its <code>cm</code> entry as confidence
     * <code>CERTAIN_FOREGROUND_CONFIDENCE</code> or
     * <code>CERTAIN_BACKGROUND_CONFIDENCE</code>.
     *
     * @param image Pixel data of the image to be segmentated.
     *        Every integer represents one ARGB-value.
     * @param imageSize number of values in image
     * @param cm Confidence matrix specifying the probability of an image
     *        belonging to the foreground before and after the segmentation.
     * @param smoothness Number of smoothing steps in the post processing.
     * @param sizeFactorToKeep Segmentation retains the largest connected
     *        foreground component plus any component with size at least
     *        <CODE>sizeOfLargestComponent/sizeFactorToKeep</CODE>.
     * @return <CODE>true</CODE> if the segmentation algorithm succeeded,
     *         <CODE>false</CODE> if segmentation is impossible
     */
    bool segmentate(unsigned long *image, int imageSize,
                    float *cm, int cmSize,
                    int smoothness, double sizeFactorToKeep);

    /**
     * Clears given confidence matrix except entries for the largest connected
     * component and every component with
     * <CODE>size*sizeFactorToKeep >= sizeOfLargestComponent</CODE>.
     *
     * @param cm  Confidence matrix to be analysed
     * @param threshold Pixel visibility threshold.
     *        Exactly those cm entries larger than threshold are considered
     *        to be a "visible" foreground pixel.
     * @param sizeFactorToKeep This method keeps the largest connected
     *        component plus any component with size at least
     *        <CODE>sizeOfLargestComponent/sizeFactorToKeep</CODE>.
     */
    void keepOnlyLargeComponents(float *cm, int cmSize,
                                 float threshold,
                                 double sizeFactorToKeep);

    /**
     * Depth first search pixels in a foreground component.
     *
     * @param cm confidence matrix to be searched.
     * @param i starting position as index to confidence matrix.
     * @param threshold defines the minimum value at which a pixel is
     *        considered foreground.
     * @param curlabel label no of component.
     * @return size in pixel of the component found.
     */
    int depthFirstSearch(float *cm, int i, float threshold, int curLabel);

    /**
     * Refines the classification stored in the confidence matrix by modifying
     * the confidences for regions which have characteristics to both
     * foreground and background if they fall into the specified square.
     * <P>
     * The can be used in displaying the image by assigning the alpha values
     * of the pixels according to the confidence entries.
     * <P>
     * In the algorithm descriptions and examples GUIs this step is referrered
     * to as <EM>Detail Refinement (Brush)</EM>.
     *
     * @param x Horizontal coordinate of the squares center.
     * @param y Vertical coordinate of the squares center.
     * @param brushmode Mode of the refinement applied, <CODE>ADD_EDGE</CODE>
     *        or <CODE>SUB_EDGE</CODE>. Add mode only modifies pixels
     *        formerly classified as background, sub mode only those
     *        formerly classified as foreground.
     * @param threshold Threshold for the add and sub refinement, deciding
     *        at the confidence level to stop at.
     * @param cf The confidence matrix to modify, generated by
     *        <CODE>segmentate</CODE>, possibly already refined by privious
     *        calls to <CODE>subpixelRefine</CODE>.
     * @param brushsize Halfed diameter of the square shaped brush.
     *
     * @see #segmentate
     */
    void SioxSegmentator::subpixelRefine(int x, int y, int brushmode,
                             float threshold, float *cf, int brushsize);

    /**
     * Refines the classification stored in the confidence matrix by modifying
     * the confidences for regions which have characteristics to both
     * foreground and background if they fall into the specified area.
     * <P>
     * The can be used in displaying the image by assigning the alpha values
     * of the pixels according to the confidence entries.
     * <P>
     * In the algorithm descriptions and examples GUIs this step is referrered
     * to as <EM>Detail Refinement (Brush)</EM>.
     *
     * @param area Area in which the reworking of the segmentation is
     *        applied to.
     * @param brushmode Mode of the refinement applied, <CODE>ADD_EDGE</CODE>
     *        or <CODE>SUB_EDGE</CODE>. Add mode only modifies pixels
     *        formerly classified as background, sub mode only those
     *        formerly classified as foreground.
     * @param threshold Threshold for the add and sub refinement, deciding
     *        at the confidence level to stop at.
     * @param cf The confidence matrix to modify, generated by
     *        <CODE>segmentate</CODE>, possibly already refined by privious
     *        calls to <CODE>subpixelRefine</CODE>.
     *
     * @see #segmentate
     */
    bool SioxSegmentator::subpixelRefine(int xa, int ya, int dx, int dy,
                                     int brushmode,
                                     float threshold, float *cf);
    /**
     * A region growing algorithms used to fill up the confidence matrix
     * with <CODE>CERTAIN_FOREGROUND_CONFIDENCE</CODE> for corresponding
     * areas of equal colors.
     * <P>
     * Basically, the method works like the <EM>Magic Wand<EM> with a
     * tolerance threshold of zero.
     *
     * @param cm confidence matrix to be searched
     * @param image image to be searched
     */
    void fillColorRegions(float *cm, int cmSize, unsigned long *image);

private:

    /**
     * Prevent this from being used
     */
    SioxSegmentator();

    /** error logging **/
    void error(char *format, ...);

    /** trace logging **/
    void trace(char *format, ...);

    typedef enum
        {
        ADD_EDGE, /** Add mode for the subpixel refinement. */
        SUB_EDGE  /** Subtract mode for the subpixel refinement. */
        } BrushMode;

    // instance fields:

    /** Horizontal resolution of the image to be segmentated. */
    int imgWidth;

    /** Vertical resolution of the image to be segmentated. */
    int imgHeight;

    /** Stores component label (index) by pixel it belongs to. */
    int *labelField;

    /**
     * LAB color values of pixels that are definitly known background.
     * Entries are of form {l,a,b}.
     */
    std::vector<CLAB> knownBg;

    /**
     * LAB color values of pixels that are definitly known foreground.
     * Entries are of form {l,a,b}.
     */
    std::vector<CLAB> knownFg;

    /** Holds background signature (a characteristic subset of the bg.) */
    std::vector<CLAB> bgSignature;

    /** Holds foreground signature (a characteristic subset of the fg).*/
    std::vector<CLAB> fgSignature;

    /** Size of cluster on lab axis. */
    float *limits;

    /** Maximum distance of two lab values. */
    float clusterSize;

    /**
     * Stores Tupels for fast access to nearest background/foreground pixels.
     */
    std::map<unsigned long, Tupel> hs;

    /** Size of the biggest blob.*/
    int regionCount;

    /** Copy of the original image, needed for detail refinement. */
    long *origImage;
    long origImageSize;

    /** A flag that stores if the segmentation algorithm has already ran.*/
    bool segmentated;

};

} //namespace siox
} //namespace org

#endif /* __SIOX_SEGMENTATOR_H__ */

