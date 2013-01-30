/*
 * Some filters for Potrace in Inkscape
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *   Stéphane Gimenez <dev@gim.name>
 *
 * Copyright (C) 2004-2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstdio>
#include <stdlib.h>

#include "imagemap-gdk.h"
#include "filterset.h"
#include "quantize.h"

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

    GrayMap *cannyGm = grayMapSobel(gm, lowThreshold, highThreshold);
    if (!cannyGm)
        return NULL;
    /*cannyGm->writePPM(cannyGm, "canny.ppm");*/

    return cannyGm;
}



/*#########################################################################
### Q U A N T I Z A T I O N
#########################################################################*/

/**
 *  Experimental.  Work on this later
 */
GrayMap *quantizeBand(RgbMap *rgbMap, int nrColors)
{

    RgbMap *gaussMap = rgbMapGaussian(rgbMap);
    //gaussMap->writePPM(gaussMap, "rgbgauss.ppm");

    IndexedMap *qMap = rgbMapQuantize(gaussMap, nrColors);
    //qMap->writePPM(qMap, "rgbquant.ppm");
    gaussMap->destroy(gaussMap);

    GrayMap *gm = GrayMapCreate(rgbMap->width, rgbMap->height);

    // RGB is quantized.  There should now be a small set of (R+G+B)
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
	    // printf("%d %d %d : %d\n", rgb.r, rgb.g, rgb.b, index);
            gm->setPixel(gm, x, y, sum);
            }
        }

    qMap->destroy(qMap);

    return gm;
}


/*#########################################################################
### E N D    O F    F I L E
#########################################################################*/










