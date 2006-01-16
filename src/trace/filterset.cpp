/*
 * Some filters for Potrace in Inkscape
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Bob Jamison
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <stdlib.h>

#include "imagemap-gdk.h"
#include "filterset.h"


/*#########################################################################
### G A U S S I A N  (smoothing)
#########################################################################*/

/**
 *
 */
static int gaussMatrix[] =
{
     2,  4,  5,  4, 2,
     4,  9, 12,  9, 4,
     5, 12, 15, 12, 5,
     4,  9, 12,  9, 4,
     2,  4,  5,  4, 2
};


/**
 *
 */
GrayMap *grayMapGaussian(GrayMap *me)
{
    int width  = me->width;
    int height = me->height;
    int firstX = 2;
    int lastX  = width-3;
    int firstY = 2;
    int lastY  = height-3;

    GrayMap *newGm = GrayMapCreate(width, height);
    if (!newGm)
        return NULL;

    for (int y = 0 ; y<height ; y++)
        {
        for (int x = 0 ; x<width ; x++)
            {
	    /* image boundaries */
            if (x<firstX || x>lastX || y<firstY || y>lastY)
                {
                newGm->setPixel(newGm, x, y, me->getPixel(me, x, y));
                continue;
                }

            /* all other pixels */
            int gaussIndex = 0;
            unsigned long sum = 0;
            for (int i= y-2 ; i<=y+2 ; i++)
                {
                for (int j= x-2; j<=x+2 ; j++)
                    {
                    int weight = gaussMatrix[gaussIndex++];
                    sum += me->getPixel(me, j, i) * weight;
		    }
	        }
            sum /= 159;
	    newGm->setPixel(newGm, x, y, sum);
	    }
	}

    return newGm;
}





/**
 *
 */
RgbMap *rgbMapGaussian(RgbMap *me)
{
    int width  = me->width;
    int height = me->height;
    int firstX = 2;
    int lastX  = width-3;
    int firstY = 2;
    int lastY  = height-3;

    RgbMap *newGm = RgbMapCreate(width, height);
    if (!newGm)
        return NULL;

    for (int y = 0 ; y<height ; y++)
        {
        for (int x = 0 ; x<width ; x++)
            {
	    /* image boundaries */
            if (x<firstX || x>lastX || y<firstY || y>lastY)
                {
                newGm->setPixelRGB(newGm, x, y, me->getPixel(me, x, y));
                continue;
                }

            /* all other pixels */
            int gaussIndex = 0;
            int sumR       = 0;
            int sumG       = 0;
            int sumB       = 0;
            for (int i= y-2 ; i<=y+2 ; i++)
                {
                for (int j= x-2; j<=x+2 ; j++)
                    {
                    int weight = gaussMatrix[gaussIndex++];
                    RGB rgb = me->getPixel(me, j, i);
                    sumR += weight * (int)rgb.r;
                    sumG += weight * (int)rgb.g;
                    sumB += weight * (int)rgb.b;
		    }
	        }
            RGB rout;
            rout.r = ( sumR / 159 ) & 0xff;
            rout.g = ( sumG / 159 ) & 0xff;
            rout.b = ( sumB / 159 ) & 0xff;
	    newGm->setPixelRGB(newGm, x, y, rout);
	    }
	}

    return newGm;

}




/*#########################################################################
### C A N N Y    E D G E    D E T E C T I O N
#########################################################################*/


static int sobelX[] =
{
    -1,  0,  1 ,
    -2,  0,  2 ,
    -1,  0,  1 
};

static int sobelY[] =
{
     1,  2,  1 ,
     0,  0,  0 ,
    -1, -2, -1 
};



/**
 * Perform Sobel convolution on a GrayMap
 */
static GrayMap *grayMapSobel(GrayMap *gm, 
               double dLowThreshold, double dHighThreshold)
{
    int width  = gm->width;
    int height = gm->height;
    int firstX = 1;
    int lastX  = width-2;
    int firstY = 1;
    int lastY  = height-2;

    GrayMap *newGm = GrayMapCreate(width, height);
    if (!newGm)
        return NULL;

    for (int y = 0 ; y<height ; y++)
        {
        for (int x = 0 ; x<width ; x++)
            {
            unsigned long sum = 0;
	    /* image boundaries */
            if (x<firstX || x>lastX || y<firstY || y>lastY)
                {
                sum = 0;
                }
            else
                {
                /* ### SOBEL FILTERING #### */
                long sumX = 0;
                long sumY = 0;
                int sobelIndex = 0;
                for (int i= y-1 ; i<=y+1 ; i++)
                    {
                    for (int j= x-1; j<=x+1 ; j++)
                        {
                        sumX += gm->getPixel(gm, j, i) * 
                             sobelX[sobelIndex++];
		        }
	            }

                sobelIndex = 0;
                for (int i= y-1 ; i<=y+1 ; i++)
                    {
                    for (int j= x-1; j<=x+1 ; j++)
                        {
                        sumY += gm->getPixel(gm, j, i) * 
                             sobelY[sobelIndex++];
		        }
	            }
                /*###  GET VALUE ### */
                sum = abs(sumX) + abs(sumY);

                if (sum > 765)
                    sum = 765;

#if 0
                /*###  GET ORIENTATION (slow, pedantic way) ### */
                double orient = 0.0;
                if (sumX==0)
                    {
                    if (sumY==0)
                        orient = 0.0;
                    else if (sumY<0)
                        {
                        sumY = -sumY;
                        orient = 90.0;
                        }
                    else
                        orient = 90.0;
                    }
                else
                    {
                    orient = 57.295779515 * atan2( ((double)sumY),((double)sumX) );
                    if (orient < 0.0)
                        orient += 180.0;
                    }

                /*###  GET EDGE DIRECTION ### */
                int edgeDirection = 0;
                if (orient < 22.5)
                    edgeDirection = 0;
	        else if (orient < 67.5)
                    edgeDirection = 45;
	        else if (orient < 112.5)
                    edgeDirection = 90;
	        else if (orient < 157.5)
                    edgeDirection = 135;
#else
                /*###  GET EDGE DIRECTION (fast way) ### */
                int edgeDirection = 0; /*x,y=0*/
                if (sumX==0)
                    {
                    if (sumY!=0)
                        edgeDirection = 90;
                    }
                else
                   {
                   /*long slope = sumY*1024/sumX;*/
                   long slope = (sumY << 10)/sumX;
                   if (slope > 2472 || slope< -2472)  /*tan(67.5)*1024*/
                       edgeDirection = 90;
                   else if (slope > 414) /*tan(22.5)*1024*/
                       edgeDirection = 45;
                   else if (slope < -414) /*-tan(22.5)*1024*/
                       edgeDirection = 135;
                   }

#endif
                /* printf("%ld %ld %f %d\n", sumX, sumY, orient, edgeDirection); */

                /*### Get two adjacent pixels in edge direction ### */
                unsigned long leftPixel;
                unsigned long rightPixel;
                if (edgeDirection == 0)
                    {
                    leftPixel  = gm->getPixel(gm, x-1, y);
                    rightPixel = gm->getPixel(gm, x+1, y);
                    }
                else if (edgeDirection == 45)
                    {
                    leftPixel  = gm->getPixel(gm, x-1, y+1);
                    rightPixel = gm->getPixel(gm, x+1, y-1);
                    }
                else if (edgeDirection == 90)
                    {
                    leftPixel  = gm->getPixel(gm, x, y-1);
                    rightPixel = gm->getPixel(gm, x, y+1);
                    }
                else /*135 */
                    {
                    leftPixel  = gm->getPixel(gm, x-1, y-1);
                    rightPixel = gm->getPixel(gm, x+1, y+1);
                    }

                /*### Compare current value to adjacent pixels ### */
                /*### if less that either, suppress it ### */
                if (sum < leftPixel || sum < rightPixel)
                    sum = 0;
                else
                    {
                    unsigned long highThreshold = 
                          (unsigned long)(dHighThreshold * 765.0);
                    unsigned long lowThreshold = 
                          (unsigned long)(dLowThreshold * 765.0);
                    if (sum >= highThreshold)
                        sum = 765; /* EDGE.  3*255 this needs to be settable */
                    else if (sum < lowThreshold)
                        sum = 0; /* NONEDGE */
                    else
                        {
                        if ( gm->getPixel(gm, x-1, y-1)> highThreshold ||
                             gm->getPixel(gm, x  , y-1)> highThreshold ||
                             gm->getPixel(gm, x+1, y-1)> highThreshold ||
                             gm->getPixel(gm, x-1, y  )> highThreshold ||
                             gm->getPixel(gm, x+1, y  )> highThreshold ||
                             gm->getPixel(gm, x-1, y+1)> highThreshold ||
                             gm->getPixel(gm, x  , y+1)> highThreshold ||
                             gm->getPixel(gm, x+1, y+1)> highThreshold)
                            sum = 765; /* EDGE fix me too */
                        else
                            sum = 0; /* NONEDGE */
                        }
                    }


                }/* else */
            if (sum==0) /* invert light & dark */
                sum = 765;
            else
                sum = 0;
            newGm->setPixel(newGm, x, y, sum);
	    }/* for (x) */
	}/* for (y) */

    return newGm;
}





/**
 *
 */
GrayMap *
grayMapCanny(GrayMap *gm, double lowThreshold, double highThreshold)
{
    if (!gm)
        return NULL;
    GrayMap *gaussGm = grayMapGaussian(gm);
    if (!gaussGm)
        return NULL;
    /*gaussGm->writePPM(gaussGm, "gauss.ppm");*/

    GrayMap *cannyGm = grayMapSobel(gaussGm, lowThreshold, highThreshold);
    if (!cannyGm)
        return NULL;
    /*cannyGm->writePPM(cannyGm, "canny.ppm");*/

    gaussGm->destroy(gaussGm);

    return cannyGm;
}







/**
 *
 */
GdkPixbuf *
gdkCanny(GdkPixbuf *img, double lowThreshold, double highThreshold)
{
    if (!img)
        return NULL;


    GrayMap *grayMap = gdkPixbufToGrayMap(img);
    if (!grayMap)
        return NULL;

    /*grayMap->writePPM(grayMap, "gbefore.ppm");*/

    GrayMap *cannyGm = grayMapCanny(grayMap,lowThreshold, highThreshold);

    grayMap->destroy(grayMap);

    if (!cannyGm)
        return NULL;

    /*grayMap->writePPM(grayMap, "gafter.ppm");*/

    GdkPixbuf *newImg = grayMapToGdkPixbuf(cannyGm);


    return newImg;
}




/*#########################################################################
### Q U A N T I Z A T I O N
#########################################################################*/
typedef struct OctreeNode_def OctreeNode;

/**
 * The basic octree node
 */
struct OctreeNode_def
{
    unsigned long r;
    unsigned long g;
    unsigned long b;
    unsigned int  index;
    unsigned long nrPixels;
    unsigned int  nrChildren;
    OctreeNode *parent;
    OctreeNode *children[8];
};


/**
 * create an octree node, and initialize it
 */
OctreeNode *octreeNodeCreate()
{
    OctreeNode *node = (OctreeNode *)malloc(sizeof(OctreeNode));
    if (!node)
        return NULL;
    node->r             = 0;
    node->g             = 0;
    node->b             = 0;
    node->index         = 0;
    node->nrPixels      = 0;
    node->nrChildren    = 0;
    node->parent        = NULL;
    for (int i=0 ; i<8 ; i++)
        node->children[i] = NULL;
    return node;
}

/**
 *  delete an octree node and its children
 */
void octreeNodeDelete(OctreeNode *node)
{
    if (!node)
        return;
    for (int i=0 ; i<8 ; i++)
        octreeNodeDelete(node->children[i]);
    free(node);
}


/**
 *  delete the children of an octree node
 */
void octreeNodeDeleteChildren(OctreeNode *node)
{
    if (!node)
        return;
    node->nrChildren = 0;
    for (int i=0 ; i<8 ; i++)
        {
        octreeNodeDelete(node->children[i]);
        node->children[i]=NULL;
        }
}




/**
 *  insert an RGB value into an octree node according to its
 *  high-order rgb vector bits
 */
int octreeNodeInsert(OctreeNode *root, RGB rgb, int bitsPerSample)
{
    OctreeNode *node = root;
    int newColor     = FALSE;
    int r            = rgb.r;
    int g            = rgb.g;
    int b            = rgb.b;

    int shift = 7;
    for (int bit=0 ; bit<bitsPerSample ; bit++)
        {
        /* update values of all nodes from the root to the leaf */
        node->r += r;
        node->g += g;
        node->b += b;
        node->nrPixels++;
        int index = (((r >> shift) & 1) << 2) |
	            (((g >> shift) & 1) << 1) |
                    (((b >> shift) & 1)     ) ;

        OctreeNode *child = node->children[index];
        if (!child)
            {
            child                 = octreeNodeCreate();
            node->children[index] = child;
            child->parent         = node;
            node->nrChildren++;
	    newColor              = TRUE;
            }
        node = child; /*next level*/
        shift--;
        }
    return newColor;
}





/**
 *  find an exact match for an RGB value, at the given bits
 *  per sample.  if not found, return -1
 */
int octreeNodeFind(OctreeNode *root, RGB rgb, int bitsPerSample)
{
    OctreeNode *node = root;
    int r            = rgb.r;
    int g            = rgb.g;
    int b            = rgb.b;

    int shift = 7;
    for (int bit=0 ; bit<bitsPerSample ; bit++)
        {
        int index = (((r >> shift) & 1) << 2) |
	            (((g >> shift) & 1) << 1) |
                    (((b >> shift) & 1)     ) ;

        OctreeNode *child = node->children[index];
        if (!child)
            return -1;
        node = child; /*next level*/
        shift--;
        }
    printf("error.  this should not happen\n");
    return -1;
}



static void spaces(int nr)
{
    for (int i=0; i<nr ; i++)
        printf(" ");
}

/**
 *  pretty-print an octree node and its children
 */
void octreeNodePrint(OctreeNode *node, int indent)
{
    spaces(indent); printf("####Node###\n");
    spaces(indent); printf("r :%lu\n", node->r);
    spaces(indent); printf("g :%lu\n", node->g);
    spaces(indent); printf("b :%lu\n", node->b);
    spaces(indent); printf("i :%d\n", node->index);
    for (unsigned int i=0; i<8; i++)
        {
        OctreeNode *child = node->children[i];
        if (!child)
            continue;
        spaces(indent); printf("   child %d :", i);
        octreeNodePrint(child, indent+4);
        }
}

/**
 *  Count how many nodes in this (sub)tree
 */
static int octreeNodeCount(OctreeNode *node)
{
    int count = 1;
    for (unsigned int i=0; i<8; i++)
        {
        OctreeNode *child = node->children[i];
        if (!child)
            continue;
        count += octreeNodeCount(child);
        }
    return count;
}




/**
 * Count all of the leaf nodes in the octree
 */
static void octreeLeafArray(OctreeNode *node, OctreeNode **array, int arraySize, int *len)
{
    if (!node)
        return;
    if (node->nrChildren == 0 && *len < arraySize)
        {
        array[*len] = node;
        *len = *len + 1;
        }
    for (int i=0 ; i<8 ; i++)
        octreeLeafArray(node->children[i], array, arraySize, len);
}



/**
 *  Count all of the leaf nodes in the octree
 */
static int octreeLeafCount(OctreeNode *node)
{
    if (!node)
        return 0;
    if (node->nrChildren == 0)
        return 1;
    int leaves = 0;
    for (int i=0 ; i<8 ; i++)
        leaves += octreeLeafCount(node->children[i]);
    return leaves;
}

/**
 * Mark all of the leaf nodes in the octree with an index nr
 */
static void octreeLeafIndex(OctreeNode *node, int *index)
{
    if (!node)
        return;
    if (node->nrChildren == 0)
        {
        node->index = *index;
        *index = *index + 1;
        return;
        }
    for (int i=0 ; i<8 ; i++)
        octreeLeafIndex(node->children[i], index);
}



/**
 * Find a node that has children, and that also
 * has the lowest pixel count
 */
static void octreefindLowestLeaf(OctreeNode *node, OctreeNode **lowestLeaf)
{
    if (!node)
        return;
    if (node->nrChildren == 0)
        {
        if (node->nrPixels < (*lowestLeaf)->nrPixels)
            *lowestLeaf = node;
        return;
        }
   
    for (int i=0 ; i<8 ; i++)
        octreefindLowestLeaf(node->children[i], lowestLeaf);
}


/**
 * reduce the leaves on an octree to a given number
 */
int octreePrune(OctreeNode *root, int nrColors)
{
    int leafCount = octreeLeafCount(root);

    while (leafCount > nrColors)
        {
        OctreeNode *lowestLeaf = root;    
        octreefindLowestLeaf(root, &lowestLeaf);

        if (!lowestLeaf)
            break; //should never happen

       if (lowestLeaf==root)
            {
            printf("found no leaves\n");
            continue;
            }

        OctreeNode *parent = lowestLeaf->parent;
        if (!parent)
            continue;

        for (int i=0 ; i<8 ; i++)
            {
            OctreeNode *child = parent->children[i];
            if (child == lowestLeaf)
                {
                parent->nrChildren--;
                octreeNodeDelete(child);
                parent->children[i] = NULL;
                break;
                }
            }
        /*printf("leafCount:%d lowPixels:%lu\n",
               leafCount, lowestLeaf->nrPixels);*/
        leafCount = octreeLeafCount(root);
        }
    int index = 0;
    octreeLeafIndex(root, &index);
    
    //printf("leafCount:%d\n", leafCount);
    //octreeNodePrint(root, 0);

    return leafCount;
}



/**
 *
 */
OctreeNode *octreeBuild(RgbMap *rgbMap, int bitsPerSample, int nrColors)
{
    OctreeNode *root = octreeNodeCreate();
    if (!root)
        return NULL;
    for (int y=0 ; y<rgbMap->height ; y++)
        {
        for (int x=0 ; x<rgbMap->width ; x++)
            {
            RGB rgb = rgbMap->getPixel(rgbMap, x, y);
            octreeNodeInsert(root, rgb, bitsPerSample);
            }
        }

    if (octreePrune(root, nrColors)<0)
        {
        octreeNodeDelete(root);
        return NULL;
        }

    /* octreeNodePrint(root, 0); */

    return root;
}


/* Compare two rgb's for brightness, for the qsort() below */
static int compRGB(const void *a, const void *b)
{
    RGB *ra = (RGB *)a;
    RGB *rb = (RGB *)b;
    int ba = ra->r + ra->g + ra->b;
    int bb = rb->r + rb->g + rb->b;
    int comp = ba - bb;
    return comp;
}

/**
 *
 */
RGB *makeRGBPalette(OctreeNode *root, int nrColors)
{

    OctreeNode **palette = (OctreeNode **)malloc(nrColors * sizeof(OctreeNode *));
    if (!palette)
        {
        return NULL;
        }
    int len = 0;
    octreeLeafArray(root, palette, nrColors, &len);

    RGB *rgbpal = (RGB *)malloc(len * sizeof(RGB));
    if (!rgbpal)
        {
        free(palette);
        return NULL;
        }

    for (int i=0; i<len ; i++)
        {
        OctreeNode *node = palette[i];
        RGB rgb;
        if (node->nrPixels == 0)
            {
            continue;
            }
        rgb.r = (unsigned char)( (node->r / node->nrPixels) & 0xff);
        rgb.g = (unsigned char)( (node->g / node->nrPixels) & 0xff);
        rgb.b = (unsigned char)( (node->b / node->nrPixels) & 0xff);
        int index = node->index;
        //printf("Pal: %d %d %d %d\n", rgb.r, rgb.g, rgb.b, index);
        rgbpal[index]=rgb;
        }

    free(palette);
    
    /* sort by brightness, to avoid high contrasts */
    qsort((void *)rgbpal, len, sizeof(RGB), compRGB);

    return rgbpal;
}


/**
 *  Return the closest color in the palette to the request
 */
RGB lookupQuantizedRGB(RGB *rgbpalette, int paletteSize, RGB candidate, int *closestIndex)
{
    /* slow method */
    unsigned long closestMatch = 10000000;
    RGB closestRGB = { 0 , 0, 0 };
    *closestIndex = 0;
    for (int i=0 ; i<paletteSize ; i++)
        {
        RGB entry = rgbpalette[i];
        unsigned long dr    = candidate.r - entry.r;
        unsigned long dg    = candidate.g - entry.g;
        unsigned long db    = candidate.b - entry.b;
        unsigned long match = dr * dr + dg * dg + db * db;
        if (match < closestMatch)
            {
            *closestIndex = i;
            closestMatch  = match;
            closestRGB    = entry;
            }
        }

    return closestRGB;
}


/**
 * Quantize an RGB image to a reduced number of colors.  bitsPerSample
 * is usually 3 - 5 out of 8 to conserve cpu and memory
 */
IndexedMap *rgbMapQuantize(RgbMap *rgbMap, int bitsPerSample, int nrColors)
{
    if (!rgbMap)
        return NULL;

    OctreeNode *otree = octreeBuild(rgbMap, bitsPerSample, nrColors);
    if (!otree)
        {
        return NULL;
        }
        
    /*Make sure we don't request more colors than actually exist*/
    int nodeCount = octreeLeafCount(otree);
    if (nodeCount < nrColors)
        nrColors = nodeCount;

    RGB *rgbpal = makeRGBPalette(otree, nrColors);
    if (!rgbpal)
        {
        octreeNodeDelete(otree);
        return NULL;
        }

    /*We have our original and palette. Make the new one*/
    IndexedMap *newMap = IndexedMapCreate(rgbMap->width, rgbMap->height);
    if (!newMap)
        {
        free(rgbpal);
        octreeNodeDelete(otree);
        return NULL;
        }
     
    /* fill in the color lookup table */
    for (int i=0 ; i< nrColors ; i++)
        {
        newMap->clut[i] = rgbpal[i];
        }
    newMap->nrColors = nrColors;

    for (int y=0 ; y<rgbMap->height ; y++)
        {
        for (int x=0 ; x<rgbMap->width ; x++)
            {
            RGB rgb = rgbMap->getPixel(rgbMap, x, y);
            //int indexNr = octreeNodeFind(otree, rgb, bitsPerSample);
            //printf("i:%d\n", indexNr);
            //RGB quantRgb = rgbpal[indexNr];
            int closestIndex;
            RGB quantRgb = lookupQuantizedRGB(rgbpal, nrColors, rgb, &closestIndex);
            newMap->setPixel(newMap, x, y, (unsigned int)closestIndex); 
            }
        }

    free(rgbpal);
    octreeNodeDelete(otree);

    return newMap;
}



/**
 *  Experimental.  Work on this later
 */
GrayMap *quantizeBand(RgbMap *rgbMap, int nrColors)
{

    int bitsPerSample = 4;

    RgbMap *gaussMap = rgbMapGaussian(rgbMap);
    //gaussMap->writePPM(gaussMap, "rgbgauss.ppm");

    IndexedMap *qMap = rgbMapQuantize(gaussMap, bitsPerSample, nrColors);
    //qMap->writePPM(qMap, "rgbquant.ppm");
    gaussMap->destroy(gaussMap);

    GrayMap *gm = GrayMapCreate(rgbMap->width, rgbMap->height);

    /* RGB is quantized.  There should now be a small set of (R+G+B) */
    for (int y=0 ; y<qMap->height ; y++)
        {
        for (int x=0 ; x<qMap->width ; x++)
            {
            RGB rgb = qMap->getPixelValue(qMap, x, y);
            int sum = rgb.r + rgb.g + rgb.b;
            if (sum & 1)
                sum = 765;
            else
                sum = 0;
            /*printf("%d %d %d : %d\n", rgb.r, rgb.g, rgb.b, index);*/
            gm->setPixel(gm, x, y, sum);
            }
        }

    qMap->destroy(qMap);

    return gm;
}








/*#########################################################################
### E N D    O F    F I L E
#########################################################################*/










