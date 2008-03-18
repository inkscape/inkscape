#ifndef __SVGTYPES_H__
#define __SVGTYPES_H__

/**
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
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


// For access to DOM2 core
#include "dom/dom.h"

// For access to DOM2 events
#include "dom/events.h"

// For access to those parts from DOM2 CSS OM used by SVG DOM.
#include "dom/css.h"

// For access to those parts from DOM2 Views OM used by SVG DOM.
#include "dom/views.h"

// For access to the SMIL OM used by SVG DOM.
#include "dom/smil.h"


#include <math.h>



namespace org {
namespace w3c {
namespace dom {
namespace svg {




//local definitions
typedef dom::DOMString DOMString;
typedef dom::DOMException DOMException;
typedef dom::Element Element;
typedef dom::Document Document;
typedef dom::NodeList NodeList;


class SVGElement;
class SVGUseElement;
class SVGAnimatedPreserveAspectRatio;


/*#########################################################################
## SVGException
#########################################################################*/

/**
 *
 */
class SVGException
{
public:
    // unsigned short   code;  //inherited
};

    /**
     * SVGExceptionCode
     */
    typedef enum
        {
        SVG_WRONG_TYPE_ERR           = 0,
        SVG_INVALID_VALUE_ERR        = 1,
        SVG_MATRIX_NOT_INVERTABLE    = 2
        } SVGExceptionCode;





/*#########################################################################
## SVGMatrix
#########################################################################*/

/**
 *  In SVG, a Matrix is defined like this:
 *
 * | a  c  e |
 * | b  d  f |
 * | 0  0  1 |
 *
 */
class SVGMatrix
{
public:


    /**
     *
     */
    virtual double getA()
        { return a; }

    /**
     *
     */
    virtual void setA(double val) throw (DOMException)
        { a = val; }

    /**
     *
     */
    virtual double getB()
        { return b; }

    /**
     *
     */
    virtual void setB(double val) throw (DOMException)
        { b = val; }

    /**
     *
     */
    virtual double getC()
        { return c; }

    /**
     *
     */
    virtual void setC(double val) throw (DOMException)
        { c = val; }

    /**
     *
     */
    virtual double getD()
        { return d; }

    /**
     *
     */
    virtual void setD(double val) throw (DOMException)
        { d = val; }
    /**
     *
     */
    virtual double getE()
        { return e; }

    /**
     *
     */
    virtual void setE(double val) throw (DOMException)
        { e = val; }
    /**
     *
     */
    virtual double getF()
        { return f; }

    /**
     *
     */
    virtual void setF(double val) throw (DOMException)
        { f = val; }


    /**
     * Return the result of postmultiplying this matrix with another.
     */
    virtual SVGMatrix multiply(const SVGMatrix &other)
        {
        SVGMatrix result;
        result.a = a * other.a  +  c * other.b;
        result.b = b * other.a  +  d * other.b;
        result.c = a * other.c  +  c * other.d;
        result.d = b * other.c  +  d * other.d;
        result.e = a * other.e  +  c * other.f  +  e;
        result.f = b * other.e  +  d * other.f  +  f;
        return result;
        }

    /**
     *  Calculate the inverse of this matrix
     *
     */
    virtual SVGMatrix inverse(  ) throw( SVGException )
        {
        /*###########################################
        The determinant of a 3x3 matrix E
           (let's use our own notation for a bit)

            A  B  C
            D  E  F
            G  H  I
        is
            AEI - AFH - BDI + BFG + CDH - CEG

        Since in our affine transforms, G and H==0 and I==1,
        this reduces to:
            AE - BD
        In SVG's naming scheme, that is:  a * d - c * b .  SIMPLE!

        In a similar method of attack, SVG's adjunct matrix is:

           d  -c   cf-ed
          -b   a   eb-af
           0   0   ad-cb

        To get the inverse matrix, we divide the adjunct matrix by
        the determinant.  Notice that (ad-cb)/(ad-cb)==1.  Very cool.
        So what we end up with is this:

           a =  d/(ad-cb)  c = -c/(ad-cb)   e = (cf-ed)/(ad-cb)
           b = -b/(ad-cb)  d =  a/(ad-cb)   f = (eb-af)/(ad-cb)

        (Since this would be in all SVG-DOM implementations,
         somebody needed to document this!  ^^ )
        #############################################*/

        SVGMatrix result;
        double determinant = a * d  -  c * b;
        if (determinant < 1.0e-18)//invertible?
            {
            result.identity();//cop out
            return result;
            }

        double idet = 1.0 / determinant;
        result.a =   d * idet;
        result.b =  -b * idet;
        result.c =  -c * idet;
        result.d =   a * idet;
        result.e =  (c*f - e*d) * idet;
        result.f =  (e*b - a*f) * idet;
        return result;
        }

    /**
     * Equivalent to multiplying by:
     *  | 1  0  x |
     *  | 0  1  y |
     *  | 0  0  1 |
     *
     */
    virtual SVGMatrix translate(double x, double y )
        {
        SVGMatrix result;
        result.a = a;
        result.b = b;
        result.c = c;
        result.d = d;
        result.e = a * x  +  c * y  +  e;
        result.f = b * x  +  d * y  +  f;
        return result;
        }

    /**
     * Equivalent to multiplying by:
     *  | scale  0      0 |
     *  | 0      scale  0 |
     *  | 0      0      1 |
     *
     */
    virtual SVGMatrix scale(double scale)
        {
        SVGMatrix result;
        result.a = a * scale;
        result.b = b * scale;
        result.c = c * scale;
        result.d = d * scale;
        result.e = e;
        result.f = f;
        return result;
        }

    /**
     * Equivalent to multiplying by:
     *  | scaleX  0       0 |
     *  | 0       scaleY  0 |
     *  | 0       0       1 |
     *
     */
    virtual SVGMatrix scaleNonUniform(double scaleX,
                                      double scaleY )
        {
        SVGMatrix result;
        result.a = a * scaleX;
        result.b = b * scaleX;
        result.c = c * scaleY;
        result.d = d * scaleY;
        result.e = e;
        result.f = f;
        return result;
        }

    /**
     * Equivalent to multiplying by:
     *  | cos(a) -sin(a)   0 |
     *  | sin(a)  cos(a)   0 |
     *  | 0       0        1 |
     *
     */
    virtual SVGMatrix rotate (double angle)
        {
        double sina  = sin(angle);
        double msina = -sina;
        double cosa  = cos(angle);
        SVGMatrix result;
        result.a = a * cosa   +  c * sina;
        result.b = b * cosa   +  d + sina;
        result.c = a * msina  +  c * cosa;
        result.d = b * msina  +  d * cosa;
        result.e = e;
        result.f = f;
        return result;
        }

    /**
     * Equivalent to multiplying by:
     *  | cos(a) -sin(a)   0 |
     *  | sin(a)  cos(a)   0 |
     *  | 0       0        1 |
     *  In this case, angle 'a' is computed as the artangent
     *  of the slope y/x .  It is negative if the slope is negative.
     */
    virtual SVGMatrix rotateFromVector(double x, double y)
                                      throw( SVGException )
        {
        double angle = atan(y / x);
        if (y < 0.0)
            angle = -angle;
        SVGMatrix result;
        double sina  = sin(angle);
        double msina = -sina;
        double cosa  = cos(angle);
        result.a = a * cosa   +  c * sina;
        result.b = b * cosa   +  d + sina;
        result.c = a * msina  +  c * cosa;
        result.d = b * msina  +  d * cosa;
        result.e = e;
        result.f = f;
        return result;
        }

    /**
     * Equivalent to multiplying by:
     *  | -1   0   0 |
     *  | 0    1   0 |
     *  | 0    0   1 |
     *
     */
    virtual SVGMatrix flipX(  )
        {
        SVGMatrix result;
        result.a = -a;
        result.b = -b;
        result.c =  c;
        result.d =  d;
        result.e =  e;
        result.f =  f;
        return result;
        }

    /**
     * Equivalent to multiplying by:
     *  | 1   0   0 |
     *  | 0  -1   0 |
     *  | 0   0   1 |
     *
     */
    virtual SVGMatrix flipY( )
        {
        SVGMatrix result;
        result.a =  a;
        result.b =  b;
        result.c = -c;
        result.d = -d;
        result.e =  e;
        result.f =  f;
        return result;
        }

    /**
     *  | 1   tan(a)  0 |
     *  | 0   1       0 |
     *  | 0   0       1 |
     *
     */
    virtual SVGMatrix skewX(double angle)
        {
        double tana = tan(angle);
        SVGMatrix result;
        result.a =  a;
        result.b =  b;
        result.c =  a * tana + c;
        result.d =  b * tana + d;
        result.e =  e;
        result.f =  f;
        return result;
        }

    /**
     * Equivalent to multiplying by:
     *  | 1       0   0 |
     *  | tan(a)  1   0 |
     *  | 0       0   1 |
     *
     */
    virtual SVGMatrix skewY(double angle)
        {
        double tana = tan(angle);
        SVGMatrix result;
        result.a =  a + c * tana;
        result.b =  b + d * tana;
        result.c =  c;
        result.d =  d;
        result.e =  e;
        result.f =  f;
        return result;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGMatrix()
        {
        identity();
        }

    /**
     *
     */
    SVGMatrix(double aArg, double bArg, double cArg,
              double dArg, double eArg, double fArg )
        {
        a = aArg; b = bArg; c = cArg;
        d = dArg; e = eArg; f = fArg;
        }

    /**
     * Copy constructor
     */
    SVGMatrix(const SVGMatrix &other)
        {
        a = other.a;
        b = other.b;
        c = other.c;
        d = other.d;
        e = other.e;
        f = other.f;
        }



    /**
     *
     */
    virtual ~SVGMatrix() {}

protected:

friend class SVGTransform;

    /*
     * Set to the identify matrix
     */
   void identity()
       {
       a = 1.0;
       b = 0.0;
       c = 0.0;
       d = 1.0;
       e = 0.0;
       f = 0.0;
       }

    double a, b, c, d, e, f;

};


/*#########################################################################
## SVGTransform
#########################################################################*/

/**
 *
 */
class SVGTransform
{
public:

    /**
     * Transform Types
     */
    typedef enum
        {
        SVG_TRANSFORM_UNKNOWN   = 0,
        SVG_TRANSFORM_MATRIX    = 1,
        SVG_TRANSFORM_TRANSLATE = 2,
        SVG_TRANSFORM_SCALE     = 3,
        SVG_TRANSFORM_ROTATE    = 4,
        SVG_TRANSFORM_SKEWX     = 5,
        SVG_TRANSFORM_SKEWY     = 6,
        } TransformType;

    /**
     *
     */
    virtual unsigned short getType()
        { return type; }


    /**
     *
     */
    virtual SVGMatrix getMatrix()
        {
        return matrix;
        }

    /**
     *
     */
    virtual double getAngle()
        {
        return angle;
        }


    /**
     *
     */
    virtual void setMatrix(const SVGMatrix &matrixArg)
        {
        type = SVG_TRANSFORM_MATRIX;
        matrix = matrixArg;
        }

    /**
     *
     */
    virtual void setTranslate (double tx, double ty )
        {
        type = SVG_TRANSFORM_TRANSLATE;
        matrix.setA(1.0);
        matrix.setB(0.0);
        matrix.setC(0.0);
        matrix.setD(1.0);
        matrix.setE(tx);
        matrix.setF(ty);
        }

    /**
     *
     */
    virtual void setScale (double sx, double sy )
        {
        type = SVG_TRANSFORM_SCALE;
        matrix.setA(sx);
        matrix.setB(0.0);
        matrix.setC(0.0);
        matrix.setD(sy);
        matrix.setE(0.0);
        matrix.setF(0.0);
        }

    /**
     *
     */
    virtual void setRotate (double angleArg, double cx, double cy)
        {
        angle = angleArg;
        setTranslate(cx, cy);
        type = SVG_TRANSFORM_ROTATE;
        matrix.rotate(angle);
        }

    /**
     *
     */
    virtual void setSkewX (double angleArg)
        {
        angle = angleArg;
        type = SVG_TRANSFORM_SKEWX;
        matrix.identity();
        matrix.skewX(angle);
        }

    /**
     *
     */
    virtual void setSkewY (double angleArg)
        {
        angle = angleArg;
        type = SVG_TRANSFORM_SKEWY;
        matrix.identity();
        matrix.skewY(angle);
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGTransform()
        {
        type = SVG_TRANSFORM_UNKNOWN;
        angle = 0.0;
        }

    /**
     *
     */
    SVGTransform(const SVGTransform &other)
        {
        type   = other.type;
        angle  = other.angle;
        matrix = other.matrix;
        }

    /**
     *
     */
    virtual ~SVGTransform()
        {}

protected:

    int type;
    double angle;

    SVGMatrix matrix;
};






/*#########################################################################
## SVGTransformList
#########################################################################*/

/**
 *
 */
class SVGTransformList
{
public:


    /**
     *
     */
    virtual unsigned long getNumberOfItems()
        { return items.size(); }


    /**
     *
     */
    virtual void clear( ) throw( DOMException )
        { items.clear(); }

    /**
     *
     */
    virtual SVGTransform initialize (const SVGTransform &newItem)
                         throw( DOMException, SVGException )
        {
        items.clear();
        items.push_back(newItem);
        return newItem;
        }

    /**
     *
     */
    virtual SVGTransform getItem (unsigned long index )
                    throw( DOMException )
        {
        if (index>=items.size())
            {
            SVGTransform transform;
            return transform;
            }
        return items[index];
        }

    /**
     *
     */
    virtual SVGTransform insertItemBefore (const SVGTransform &newItem,
                                           unsigned long index )
                                      throw( DOMException, SVGException )
        {
        if (index > items.size())
            items.push_back(newItem);
        else
            {
            std::vector<SVGTransform>::iterator iter = items.begin() + index;
            items.insert(iter, newItem);
            }
        return newItem;
        }

    /**
     *
     */
    virtual SVGTransform replaceItem (const SVGTransform &newItem,
                                       unsigned long index )
                                throw( DOMException, SVGException )
        {
        if (index>=items.size())
            {
            SVGTransform transform;
            return transform;
            }
        else
            {
            std::vector<SVGTransform>::iterator iter = items.begin() + index;
            *iter = newItem;
            }
        return newItem;
        }

    /**
     *
     */
    virtual SVGTransform removeItem (unsigned long index )
                                     throw( DOMException )
        {
        if (index>=items.size())
            {
            SVGTransform transform;
            return transform;
            }
        std::vector<SVGTransform>::iterator iter = items.begin() + index;
        SVGTransform oldItem = *iter;
        items.erase(iter);
        return oldItem;
        }

    /**
     *
     */
    virtual SVGTransform appendItem (const SVGTransform &newItem)
                                  throw( DOMException, SVGException )
        {
        items.push_back(newItem);
        return newItem;
        }

    /**
     *
     */
    virtual SVGTransform createSVGTransformFromMatrix(const SVGMatrix &matrix)
        {
        SVGTransform transform;
        transform.setMatrix(matrix);
        return transform;
        }

    /**
     *
     */
    virtual SVGTransform consolidate()
        {
        SVGMatrix matrix;
        for (unsigned int i=0 ; i<items.size() ; i++)
            matrix = matrix.multiply(items[i].getMatrix());
        SVGTransform transform;
        transform.setMatrix(matrix);
        items.clear();
        items.push_back(transform);
        return transform;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGTransformList()
        {}

    /**
     *
     */
    SVGTransformList(const SVGTransformList &other)
        {
        items = other.items;
        }

    /**
     *
     */
    virtual ~SVGTransformList() {}

protected:

    std::vector<SVGTransform> items;

};






/*#########################################################################
## SVGAnimatedTransformList
#########################################################################*/

/**
 *
 */
class SVGAnimatedTransformList
{
public:

    /**
     *
     */
    virtual SVGTransformList getBaseVal()
        { return baseVal; }

    /**
     *
     */
    virtual SVGTransformList getAnimVal()
        { return animVal; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGAnimatedTransformList()
        {}

    /**
     *
     */
    SVGAnimatedTransformList(const SVGAnimatedTransformList &other)
        {
        baseVal = other.baseVal;
        animVal = other.animVal;
        }

    /**
     *
     */
    virtual ~SVGAnimatedTransformList() {}

protected:

    SVGTransformList baseVal;
    SVGTransformList animVal;

};




/*#########################################################################
## SVGAnimatedBoolean
#########################################################################*/

/**
 *
 */
class SVGAnimatedBoolean
{
public:

    /**
     *
     */
    virtual bool getBaseVal()
        {
        return baseVal;
        }

    /**
     *
     */
    virtual void setBaseVal(bool val) throw (DOMException)
        {
        baseVal = val;
        }

    /**
     *
     */
    virtual bool getAnimVal()
        {
        return animVal;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGAnimatedBoolean()
        {
        baseVal = animVal = false;
        }

    /**
     *
     */
    SVGAnimatedBoolean(const SVGAnimatedBoolean &other)
        {
        baseVal = other.baseVal;
        animVal = other.animVal;
        }

    /**
     *
     */
    virtual ~SVGAnimatedBoolean() {}

protected:

    bool baseVal, animVal;

};




/*#########################################################################
## SVGAnimatedString
#########################################################################*/

/**
 *
 */
class SVGAnimatedString
{
public:

    /**
     *
     */
    virtual DOMString getBaseVal()
        {
        return baseVal;
        }

    /**
     *
     */
    virtual void setBaseVal(const DOMString &val)
                            throw (DOMException)
        {
        baseVal = val;
        }

    /**
     *
     */
    virtual DOMString getAnimVal()
        {
        return animVal;
        }


    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    SVGAnimatedString()
        {
        baseVal = "";
        animVal = "";
        }

    /**
     *
     */
    SVGAnimatedString(const SVGAnimatedString &other)
        {
        baseVal = other.baseVal;
        animVal = other.animVal;
        }

    /**
     *
     */
    virtual ~SVGAnimatedString() {}

protected:

    DOMString baseVal, animVal;

};





/*#########################################################################
## SVGStringList
#########################################################################*/

/**
 *
 */
class SVGStringList
{
public:


    /**
     *
     */
    virtual unsigned long getNumberOfItems()
        {
        return items.size();
        }

    /**
     *
     */
    virtual void clear () throw( DOMException )
       {
       items.clear();
       }

    /**
     *
     */
    virtual DOMString initialize ( const DOMString& newItem )
                    throw( DOMException, SVGException )
        {
        items.clear();
        items.push_back(newItem);
        return newItem;
        }

    /**
     *
     */
    virtual DOMString getItem ( unsigned long index )
                    throw( DOMException )
        {
        if (index >= items.size())
            return "";
        return items[index];
        }

    /**
     *
     */
    virtual DOMString insertItemBefore ( const DOMString& newItem,
                                 unsigned long index )
                               throw( DOMException, SVGException )
        {
        if (index>=items.size())
            {
            items.push_back(newItem);
            }
        else
            {
            std::vector<DOMString>::iterator iter = items.begin() + index;
            items.insert(iter, newItem);
            }
        return newItem;
        }

    /**
     *
     */
    virtual DOMString replaceItem ( const DOMString& newItem,
                                    unsigned long index )
                                throw( DOMException, SVGException )
        {
        if (index>=items.size())
            return "";
        std::vector<DOMString>::iterator iter = items.begin() + index;
        *iter = newItem;
        return newItem;
        }

    /**
     *
     */
    virtual DOMString removeItem ( unsigned long index )
                    throw( DOMException )
        {
        if (index>=items.size())
            return "";
        std::vector<DOMString>::iterator iter = items.begin() + index;
        DOMString oldstr = *iter;
        items.erase(iter);
        return oldstr;
        }

    /**
     *
     */
    virtual DOMString appendItem ( const DOMString& newItem )
                    throw( DOMException, SVGException )
        {
        items.push_back(newItem);
        return newItem;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGStringList() {}

    /**
     *
     */
   SVGStringList(const SVGStringList &other)
       {
       items = other.items;
       }

    /**
     *
     */
    virtual ~SVGStringList() {}

protected:

    std::vector<DOMString>items;

};





/*#########################################################################
## SVGAnimatedEnumeration
#########################################################################*/

/**
 *
 */
class SVGAnimatedEnumeration
{
public:

    /**
     *
     */
    virtual unsigned short getBaseVal()
        {
        return baseVal;
        }

    /**
     *
     */
    virtual void setBaseVal(unsigned short val)
                                     throw (DOMException)
        {
        baseVal = val;
        }

    /**
     *
     */
    virtual unsigned short getAnimVal()
        {
        return animVal;
        }



    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    SVGAnimatedEnumeration()
        {
        baseVal = animVal = 0;
        }

    /**
     *
     */
    SVGAnimatedEnumeration(const SVGAnimatedEnumeration &other)
        {
        baseVal = other.baseVal;
        animVal = other.animVal;
        }

    /**
     *
     */
    virtual ~SVGAnimatedEnumeration() {}

protected:

    int baseVal, animVal;

};





/*#########################################################################
## SVGAnimatedInteger
#########################################################################*/

/**
 *
 */
class SVGAnimatedInteger
{
public:


    /**
     *
     */
    virtual long getBaseVal()
        {
        return baseVal;
        }

    /**
     *
     */
    virtual void setBaseVal(long val) throw (DOMException)
        {
        baseVal = val;
        }

    /**
     *
     */
    virtual long getAnimVal()
        {
        return animVal;
        }



    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    SVGAnimatedInteger()
        { baseVal = animVal = 0L;}


    /**
     *
     */
    SVGAnimatedInteger(long value)
        {
        baseVal = value;
        animVal = 0L;
        }

    /**
     *
     */
    SVGAnimatedInteger(long baseValArg, long animValArg)
        {
        baseVal = baseValArg;
        animVal = animValArg;
        }


    /**
     *
     */
    SVGAnimatedInteger(const SVGAnimatedInteger &other)
        {
        baseVal = other.baseVal;
        animVal = other.animVal;
        }

    /**
     *
     */
    virtual ~SVGAnimatedInteger() {}

protected:

    long baseVal, animVal;

};





/*#########################################################################
## SVGNumber
#########################################################################*/

/**
 *
 */
class SVGNumber
{
public:


    /**
     *
     */
    virtual double getValue()
        {
        return value;
        }

    /**
     *
     */
    virtual void setValue(double val) throw (DOMException)
        {
        value = val;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGNumber()
        {
        value = 0.0;
        }

    /**
     *
     */
    SVGNumber(const SVGNumber &other)
        {
        value = other.value;
        }

    /**
     *
     */
    virtual ~SVGNumber() {}

protected:

    double value;

};





/*#########################################################################
## SVGAnimatedNumber
#########################################################################*/

/**
 *
 */
class SVGAnimatedNumber
{
public:



    /**
     *
     */
    virtual double getBaseVal()
        {
        return baseVal;
        }

    /**
     *
     */
    virtual void setBaseVal(double val) throw (DOMException)
        {
        baseVal = val;
        }

    /**
     *
     */
    virtual double getAnimVal()
        {
        return animVal;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGAnimatedNumber()
        {
        baseVal = animVal = 0.0;
        }


    /**
     *
     */
    SVGAnimatedNumber(double val)
        {
        baseVal = val;
        animVal = 0.0;
        }


    /**
     *
     */
    SVGAnimatedNumber(double baseValArg, double animValArg)
        {
        baseVal = baseValArg;
        animVal = animValArg;
        }

    /**
     *
     */
    SVGAnimatedNumber(const SVGAnimatedNumber &other)
        {
        baseVal = other.baseVal;
        animVal = other.animVal;
        }

    /**
     *
     */
    virtual ~SVGAnimatedNumber() {}

protected:

    double baseVal, animVal;

};





/*#########################################################################
## SVGNumberList
#########################################################################*/

/**
 *
 */
class SVGNumberList
{
public:

    /**
     *
     */
    virtual unsigned long getNumberOfItems()
        {
        return items.size();
        }


    /**
     *
     */
    virtual void clear() throw( DOMException )
        {
        items.clear();
        }

    /**
     *
     */
    virtual SVGNumber initialize (const SVGNumber &newItem)
                    throw( DOMException, SVGException )
        {
        items.clear();
        items.push_back(newItem);
        return newItem;
        }

    /**
     *
     */
    virtual SVGNumber getItem ( unsigned long index )
                                  throw( DOMException )
        {
        if (index>=items.size())
            {
            SVGNumber num;
            return num;
            }
        return items[index];
        }

    /**
     *
     */
    virtual SVGNumber insertItemBefore ( const SVGNumber &newItem,
                                         unsigned long index )
                                         throw( DOMException, SVGException )
        {
        if (index>=items.size())
            {
            items.push_back(newItem);
            }
        else
            {
            std::vector<SVGNumber>::iterator iter = items.begin() + index;
            items.insert(iter, newItem);
            }
        return newItem;
        }

    /**
     *
     */
    virtual SVGNumber replaceItem ( const SVGNumber &newItem,
                                    unsigned long index )
                                    throw( DOMException, SVGException )
        {
        if (index>=items.size())
            {
            SVGNumber num;
            return num;
            }
        std::vector<SVGNumber>::iterator iter = items.begin() + index;
        *iter = newItem;
        return newItem;
        }

    /**
     *
     */
    virtual SVGNumber removeItem ( unsigned long index )
                                  throw( DOMException )
        {
        if (index>=items.size())
            {
            SVGNumber num;
            return num;
            }
        std::vector<SVGNumber>::iterator iter = items.begin() + index;
        SVGNumber oldval = *iter;
        items.erase(iter);
        return oldval;
        }

    /**
     *
     */
    virtual SVGNumber appendItem ( const SVGNumber &newItem )
                                   throw( DOMException, SVGException )
        {
        items.push_back(newItem);
        return newItem;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGNumberList() {}

    /**
     *
     */
    SVGNumberList(const SVGNumberList &other)
        {
        items = other.items;
        }

    /**
     *
     */
    virtual ~SVGNumberList() {}

protected:

    std::vector<SVGNumber>items;

};





/*#########################################################################
## SVGAnimatedNumberList
#########################################################################*/

/**
 *
 */
class SVGAnimatedNumberList
{
public:


    /**
     *
     */
    virtual SVGNumberList &getBaseVal()
        {
        return baseVal;
        }

    /**
     *
     */
    virtual SVGNumberList &getAnimVal()
        {
        return animVal;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGAnimatedNumberList() {}

    /**
     *
     */
    SVGAnimatedNumberList(const SVGAnimatedNumberList &other)
        {
        baseVal = other.baseVal;
        animVal = other.animVal;
        }

    /**
     *
     */
    virtual ~SVGAnimatedNumberList() {}

protected:

    SVGNumberList baseVal;
    SVGNumberList animVal;

};






/*#########################################################################
## SVGLength
#########################################################################*/

/**
 *
 */
class SVGLength
{
public:

    /**
     * Length Unit Types
     */
    typedef enum
        {
        SVG_LENGTHTYPE_UNKNOWN    = 0,
        SVG_LENGTHTYPE_NUMBER     = 1,
        SVG_LENGTHTYPE_PERCENTAGE = 2,
        SVG_LENGTHTYPE_EMS        = 3,
        SVG_LENGTHTYPE_EXS        = 4,
        SVG_LENGTHTYPE_PX         = 5,
        SVG_LENGTHTYPE_CM         = 6,
        SVG_LENGTHTYPE_MM         = 7,
        SVG_LENGTHTYPE_IN         = 8,
        SVG_LENGTHTYPE_PT         = 9,
        SVG_LENGTHTYPE_PC         = 10
        } LengthUnitType;


    /**
     *
     */
    virtual unsigned short getUnitType( )
        {
        return unitType;
        }

    /**
     *
     */
    virtual double getValue( )
        {
        return value;
        }

    /**
     *
     */
    virtual void setValue( double val )  throw (DOMException)
        {
        value = val;
        }

    /**
     *
     */
    virtual double getValueInSpecifiedUnits( )
        {
        double result = 0.0;
        //fill this in
        return result;
        }

    /**
     *
     */
    virtual void setValueInSpecifiedUnits( double /*val*/ )
                                           throw (DOMException)
        {
        //fill this in
        }

    /**
     *
     */
    virtual DOMString getValueAsString( )
        {
        DOMString ret;
        char buf[32];
        snprintf(buf, 31, "%f", value);
        ret.append(buf);
        return ret;
        }

    /**
     *
     */
    virtual void setValueAsString( const DOMString& /*val*/ )
                                   throw (DOMException)
        {
        }


    /**
     *
     */
    virtual void newValueSpecifiedUnits ( unsigned short /*unitType*/, double /*val*/ )
        {
        }

    /**
     *
     */
    virtual void convertToSpecifiedUnits ( unsigned short /*unitType*/ )
        {
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGLength()
        {
        unitType = SVG_LENGTHTYPE_UNKNOWN;
        value    = 0.0;
        }


    /**
     *
     */
    SVGLength(const SVGLength &other)
        {
        unitType  = other.unitType;
        value     = other.value;
        }

    /**
     *
     */
    virtual ~SVGLength() {}

protected:

    int unitType;

    double value;

};






/*#########################################################################
## SVGAnimatedLength
#########################################################################*/

/**
 *
 */
class SVGAnimatedLength
{
public:

    /**
     *
     */
    virtual SVGLength &getBaseVal()
        {
        return baseVal;
        }

    /**
     *
     */
    virtual SVGLength &getAnimVal()
        {
        return animVal;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGAnimatedLength() {}

    /**
     *
     */
    SVGAnimatedLength(const SVGAnimatedLength &other)
        {
        baseVal = other.baseVal;
        animVal = other.animVal;
        }

    /**
     *
     */
    virtual ~SVGAnimatedLength() {}

protected:

    SVGLength baseVal, animVal;

};






/*#########################################################################
## SVGLengthList
#########################################################################*/

/**
 *
 */
class SVGLengthList
{
public:

    /**
     *
     */
    virtual unsigned long getNumberOfItems()
        {
        return items.size();
        }


    /**
     *
     */
    virtual void clear (  ) throw( DOMException )
        {
        items.clear();
        }

    /**
     *
     */
    virtual SVGLength initialize (const SVGLength &newItem )
                    throw( DOMException, SVGException )
        {
        items.clear();
        items.push_back(newItem);
        return newItem;
        }

    /**
     *
     */
    virtual SVGLength getItem (unsigned long index)
                    throw( DOMException )
        {
        if (index>=items.size())
            {
            SVGLength ret;
            return ret;
            }
        return items[index];
        }

    /**
     *
     */
    virtual SVGLength insertItemBefore (const SVGLength &newItem,
                                         unsigned long index )
                                   throw( DOMException, SVGException )
        {
        if (index>=items.size())
            {
            items.push_back(newItem);
            }
        else
            {
            std::vector<SVGLength>::iterator iter = items.begin() + index;
            items.insert(iter, newItem);
            }
        return newItem;
        }

    /**
     *
     */
    virtual SVGLength replaceItem (const SVGLength &newItem,
                                    unsigned long index )
                               throw( DOMException, SVGException )
        {
        if (index>=items.size())
            {
            SVGLength ret;
            return ret;
            }
        std::vector<SVGLength>::iterator iter = items.begin() + index;
        *iter = newItem;
        return newItem;
        }

    /**
     *
     */
    virtual SVGLength removeItem (unsigned long index )
                    throw( DOMException )
        {
        if (index>=items.size())
            {
            SVGLength ret;
            return ret;
            }
        std::vector<SVGLength>::iterator iter = items.begin() + index;
        SVGLength oldval = *iter;
        items.erase(iter);
        return oldval;
        }

    /**
     *
     */
    virtual SVGLength appendItem (const SVGLength &newItem )
                    throw( DOMException, SVGException )
        {
        items.push_back(newItem);
        return newItem;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGLengthList() {}

    /**
     *
     */
    SVGLengthList(const SVGLengthList &other)
        {
        items = other.items;
        }

    /**
     *
     */
    virtual ~SVGLengthList() {}

protected:

    std::vector<SVGLength>items;

};






/*#########################################################################
## SVGAnimatedLengthList
#########################################################################*/

/**
 *
 */
class SVGAnimatedLengthList
{
public:

    /**
     *
     */
    virtual SVGLengthList &getBaseVal()
        {
        return baseVal;
        }

    /**
     *
     */
    virtual SVGLengthList &getAnimVal()
        {
        return animVal;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGAnimatedLengthList() {}

    /**
     *
     */
   SVGAnimatedLengthList(const SVGAnimatedLengthList &other)
        {
        baseVal = other.baseVal;
        animVal = other.animVal;
        }

    /**
     *
     */
    virtual ~SVGAnimatedLengthList() {}

protected:

    SVGLengthList baseVal, animVal;

};






/*#########################################################################
## SVGAngle
#########################################################################*/

/**
 *
 */
class SVGAngle
{
public:

    /**
     *  Angle Unit Types
     */
    typedef enum
        {
        SVG_ANGLETYPE_UNKNOWN     = 0,
        SVG_ANGLETYPE_UNSPECIFIED = 1,
        SVG_ANGLETYPE_DEG         = 2,
        SVG_ANGLETYPE_RAD         = 3,
        SVG_ANGLETYPE_GRAD        = 4
        } AngleUnitType;



    /**
     *
     */
    virtual unsigned short getUnitType()
        {
        return unitType;
        }

    /**
     *
     */
    virtual double getValue()
        {
        return value;
        }

    /**
     *
     */
    virtual void setValue(double val) throw (DOMException)
        {
        value = val;
        }

    /**
     *
     */
    virtual double getValueInSpecifiedUnits()
        {
        double result = 0.0;
        //convert here
        return result;
        }

    /**
     *
     */
    virtual void setValueInSpecifiedUnits(double /*val*/)
                                     throw (DOMException)
        {
        //do conversion
        }

    /**
     *
     */
    virtual DOMString getValueAsString()
        {
        DOMString result;
        char buf[32];
        snprintf(buf, 31, "%f", value);
        result.append(buf);
        return result;
        }

    /**
     *
     */
    virtual void setValueAsString(const DOMString &/*val*/)
                                  throw (DOMException)
        {
        //convert here
        }


    /**
     *
     */
    virtual void newValueSpecifiedUnits (unsigned short /*unitType*/,
                                         double /*valueInSpecifiedUnits*/ )
        {
        //convert here
        }

    /**
     *
     */
    virtual void convertToSpecifiedUnits (unsigned short /*unitType*/ )
        {
        //convert here
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGAngle()
        {
        unitType = SVG_ANGLETYPE_UNKNOWN;
        value    = 0.0;
        }

    /**
     *
     */
    SVGAngle(const SVGAngle &other)
        {
        unitType = other.unitType;
        value    = other.value;
        }

    /**
     *
     */
    virtual ~SVGAngle() {}

protected:

    int unitType;

    double value;

};






/*#########################################################################
## SVGAnimatedAngle
#########################################################################*/

/**
 *
 */
class SVGAnimatedAngle
{
public:

    /**
     *
     */
    virtual SVGAngle getBaseVal()
        {
        return baseVal;
        }

    /**
     *
     */
    virtual SVGAngle getAnimVal()
        {
        return animVal;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGAnimatedAngle() {}

    /**
     *
     */
    SVGAnimatedAngle(const SVGAngle &angle)
        { baseVal = angle; }

    /**
     *
     */
    SVGAnimatedAngle(const SVGAnimatedAngle &other)
        {
        baseVal = other.baseVal;
        animVal = other.animVal;
        }

    /**
     *
     */
    virtual ~SVGAnimatedAngle() {}

protected:

    SVGAngle baseVal, animVal;

};






/*#########################################################################
## SVGICCColor
#########################################################################*/

/**
 *
 */
class SVGICCColor
{
public:

    /**
     *
     */
    virtual DOMString getColorProfile()
        {
        return colorProfile;
        }

    /**
     *
     */
    virtual void setColorProfile(const DOMString &val) throw (DOMException)
        {
        colorProfile = val;
        }

    /**
     *
     */
    virtual SVGNumberList &getColors()
        {
        return colors;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGICCColor() {}

    /**
     *
     */
    SVGICCColor(const SVGICCColor &other)
        {
        colorProfile = other.colorProfile;
        colors       = other.colors;
        }

    /**
     *
     */
    virtual ~SVGICCColor() {}

protected:

    DOMString colorProfile;

    SVGNumberList colors;

};


/*#########################################################################
## SVGColor
#########################################################################*/

/**
 *
 */
class SVGColor : virtual public css::CSSValue
{
public:


    /**
     * Color Types
     */
    typedef enum
        {
        SVG_COLORTYPE_UNKNOWN           = 0,
        SVG_COLORTYPE_RGBCOLOR          = 1,
        SVG_COLORTYPE_RGBCOLOR_ICCCOLOR = 2,
        SVG_COLORTYPE_CURRENTCOLOR      = 3
        } ColorType;


    /**
     *
     */
    virtual unsigned short getColorType()
        {
        return colorType;
        }

    /**
     *
     */
    virtual css::RGBColor getRgbColor()
        {
        css::RGBColor col;
        return col;
        }

    /**
     *
     */
    virtual SVGICCColor getIccColor()
        {
        SVGICCColor col;
        return col;
        }


    /**
     *
     */
    virtual void setRGBColor (const DOMString& /*rgbColor*/ )
                              throw( SVGException )
        {
        }

    /**
     *
     */
    virtual void setRGBColorICCColor (const DOMString& /*rgbColor*/,
                                      const DOMString& /*iccColor*/ )
                                      throw( SVGException )
        {
        }

    /**
     *
     */
    virtual void setColor (unsigned short /*colorType*/,
                           const DOMString& /*rgbColor*/,
                           const DOMString& /*iccColor*/ )
                           throw( SVGException )
        {
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGColor()
        {
        colorType = SVG_COLORTYPE_UNKNOWN;
        }

    /**
     *
     */
    SVGColor(const SVGColor &other) : css::CSSValue(other)
        {
        colorType = other.colorType;
        }

    /**
     *
     */
    virtual ~SVGColor() {}

protected:

    int colorType;

};










/*#########################################################################
## SVGRect
#########################################################################*/

/**
 *
 */
class SVGRect
{
public:

    /**
     *
     */
    virtual double getX()
        {
        return x;
        }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        {
        x = val;
        }

    /**
     *
     */
    virtual double getY()
        {
        return y;
        }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        {
        y = val;
        }

    /**
     *
     */
    virtual double getWidth()
        {
        return width;
        }

    /**
     *
     */
    virtual void setWidth(double val) throw (DOMException)
        {
        width = val;
        }

    /**
     *
     */
    virtual double getHeight()
        {
        return height;
        }

    /**
     *
     */
    virtual void setHeight(double val) throw (DOMException)
        {
        height = val;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGRect()
        {
        x = y = width = height = 0.0;
        }

    /**
     *
     */
    SVGRect(const SVGRect &other)
        {
        x = other.x;
        y = other.y;
        width = other.width;
        height = other.height;
        }

    /**
     *
     */
    virtual ~SVGRect() {}

protected:

    double x, y, width, height;

};






/*#########################################################################
## SVGAnimatedRect
#########################################################################*/

/**
 *
 */
class SVGAnimatedRect
{
public:

    /**
     *
     */
    virtual SVGRect &getBaseVal()
        {
        return baseVal;
        }

    /**
     *
     */
    virtual SVGRect &getAnimVal()
        {
        return animVal;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGAnimatedRect()
        {
        }

    /**
     *
     */
    SVGAnimatedRect(const SVGAnimatedRect &other)
        {
        baseVal = other.baseVal;
        animVal = other.animVal;
        }

    /**
     *
     */
    virtual ~SVGAnimatedRect() {}

protected:

    SVGRect baseVal, animVal;

};



/*#########################################################################
## SVGPoint
#########################################################################*/

/**
 *
 */
class SVGPoint
{
public:



    /**
     *
     */
    virtual double getX()
        { return x; }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    virtual double getY()
        { return y; }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        { y = val; }

    /**
     *
     */
    virtual SVGPoint matrixTransform(const SVGMatrix &/*matrix*/)
        {
        SVGPoint point;
        return point;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPoint()
        { x = y = 0; }

    /**
     *
     */
    SVGPoint(const SVGPoint &other)
        {
        x = other.x;
        y = other.y;
        }

    /**
     *
     */
    virtual ~SVGPoint() {}

protected:

    double x, y;
};






/*#########################################################################
## SVGPointList
#########################################################################*/

/**
 *
 */
class SVGPointList
{
public:

    /**
     *
     */
    virtual unsigned long getNumberOfItems()
        { return items.size(); }

    /**
     *
     */
    virtual void clear() throw( DOMException )
        { items.clear(); }

    /**
     *
     */
    virtual SVGPoint initialize(const SVGPoint &newItem)
                             throw( DOMException, SVGException )
        {
        items.clear();
        items.push_back(newItem);
        return newItem;
        }

    /**
     *
     */
    virtual SVGPoint getItem(unsigned long index )
                             throw( DOMException )
        {
        if (index >= items.size())
            {
            SVGPoint point;
            return point;
            }
        return items[index];
        }

    /**
     *
     */
    virtual SVGPoint insertItemBefore(const SVGPoint &newItem,
                                      unsigned long index )
                                      throw( DOMException, SVGException )
        {
        if (index >= items.size())
            items.push_back(newItem);
        else
            {
            std::vector<SVGPoint>::iterator iter = items.begin() + index;
            items.insert(iter, newItem);
            }
        return newItem;
        }

    /**
     *
     */
    virtual SVGPoint replaceItem(const SVGPoint &newItem,
                                  unsigned long index )
                                  throw( DOMException, SVGException )
        {
        if (index >= items.size())
            {
            SVGPoint point;
            return point;
            }
        std::vector<SVGPoint>::iterator iter = items.begin() + index;
        *iter = newItem;
        return newItem;
        }

    /**
     *
     */
    virtual SVGPoint removeItem(unsigned long index )
                                  throw( DOMException )
        {
        if (index >= items.size())
            {
            SVGPoint point;
            return point;
            }
        std::vector<SVGPoint>::iterator iter = items.begin() + index;
        SVGPoint oldItem = *iter;
        items.erase(iter);
        return oldItem;
        }

    /**
     *
     */
    virtual SVGPoint appendItem(const SVGPoint &newItem)
                              throw( DOMException, SVGException )
        {
        items.push_back(newItem);
        return newItem;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPointList() {}


    /**
     *
     */
    SVGPointList(const SVGPointList &other)
        {
        items = other.items;
        }


    /**
     *
     */
    virtual ~SVGPointList() {}

protected:

    std::vector<SVGPoint> items;

};




/*#########################################################################
## SVGUnitTypes
#########################################################################*/

/**
 *
 */
class SVGUnitTypes
{
public:

    /**
     * Unit Types
     */
    typedef enum
        {
        SVG_UNIT_TYPE_UNKNOWN           = 0,
        SVG_UNIT_TYPE_USERSPACEONUSE    = 1,
        SVG_UNIT_TYPE_OBJECTBOUNDINGBOX = 2
        } UnitType;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGUnitTypes() {}

    /**
     *
     */
    virtual ~SVGUnitTypes() {}

};






/*#########################################################################
## SVGStylable
#########################################################################*/

/**
 *
 */
class SVGStylable
{
public:

    /**
     *
     */
    virtual SVGAnimatedString getClassName()
        {
        return className;
        }

    /**
     *
     */
    virtual css::CSSStyleDeclaration getStyle()
        {
        return style;
        }


    /**
     *
     */
    virtual css::CSSValue getPresentationAttribute (const DOMString& /*name*/ )
        {
        css::CSSValue val;
        //perform a lookup
        return val;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGStylable() {}

    /**
     *
     */
    SVGStylable(const SVGStylable &other)
        {
        className = other.className;
        style     = other.style;
        }

    /**
     *
     */
    virtual ~SVGStylable() {}

protected:

    SVGAnimatedString className;
    css::CSSStyleDeclaration style;

};


/*#########################################################################
## SVGLocatable
#########################################################################*/

/**
 *
 */
class SVGLocatable
{
public:

    /**
     *
     */
    virtual SVGElement *getNearestViewportElement()
        {
        SVGElement *result = NULL;
        return result;
        }

    /**
     *
     */
    virtual SVGElement *getFarthestViewportElement()
        {
        SVGElement *result = NULL;
        return result;
        }

    /**
     *
     */
    virtual SVGRect getBBox (  )
        {
        return bbox;
        }

    /**
     *
     */
    virtual SVGMatrix getCTM (  )
        {
        return ctm;
        }

    /**
     *
     */
    virtual SVGMatrix getScreenCTM (  )
        {
        return screenCtm;
        }

    /**
     *
     */
    virtual SVGMatrix getTransformToElement (const SVGElement &/*element*/)
                    throw( SVGException )
        {
        SVGMatrix result;
        //do calculations
        return result;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGLocatable() {}

    /**
     *
     */
    SVGLocatable(const SVGLocatable &/*other*/)
        {
        }

    /**
     *
     */
    virtual ~SVGLocatable() {}

protected:

    SVGRect bbox;
    SVGMatrix ctm;
    SVGMatrix screenCtm;

};






/*#########################################################################
## SVGTransformable
#########################################################################*/

/**
 *
 */
class SVGTransformable : public SVGLocatable
{
public:


    /**
     *
     */
    virtual SVGAnimatedTransformList &getTransform()
        {
        return transforms;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGTransformable() {}

    /**
     *
     */
    SVGTransformable(const SVGTransformable &other) : SVGLocatable(other)
        {
        transforms = other.transforms;
        }

    /**
     *
     */
    virtual ~SVGTransformable() {}

protected:

    SVGAnimatedTransformList transforms;
};






/*#########################################################################
## SVGTests
#########################################################################*/

/**
 *
 */
class SVGTests
{
public:


    /**
     *
     */
    virtual SVGStringList &getRequiredFeatures()
        {
        return requiredFeatures;
        }

    /**
     *
     */
    virtual SVGStringList &getRequiredExtensions()
        {
        return requiredExtensions;
        }

    /**
     *
     */
    virtual SVGStringList &getSystemLanguage()
        {
        return systemLanguage;
        }


    /**
     *
     */
    virtual bool hasExtension (const DOMString& /*extension*/ )
        {
        return false;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGTests() {}

    /**
     *
     */
    SVGTests(const SVGTests &other)
        {
        requiredFeatures   = other.requiredFeatures;
        requiredExtensions = other.requiredExtensions;
        systemLanguage     = other.systemLanguage;
        }

    /**
     *
     */
    virtual ~SVGTests() {}

protected:

    SVGStringList requiredFeatures;
    SVGStringList requiredExtensions;
    SVGStringList systemLanguage;

};






/*#########################################################################
## SVGLangSpace
#########################################################################*/

/**
 *
 */
class SVGLangSpace
{
public:


    /**
     *
     */
    virtual DOMString getXmllang()
        {
        return xmlLang;
        }

    /**
     *
     */
    virtual void setXmllang(const DOMString &val)
                                     throw (DOMException)
        {
        xmlLang = val;
        }

    /**
     *
     */
    virtual DOMString getXmlspace()
        {
        return xmlSpace;
        }

    /**
     *
     */
    virtual void setXmlspace(const DOMString &val)
                                     throw (DOMException)
        {
        xmlSpace = val;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGLangSpace() {}

    /**
     *
     */
    SVGLangSpace(const SVGLangSpace &other)
        {
        xmlLang  = other.xmlLang;
        xmlSpace = other.xmlSpace;
        }

    /**
     *
     */
    virtual ~SVGLangSpace() {}

protected:

    DOMString xmlLang;
    DOMString xmlSpace;

};






/*#########################################################################
## SVGExternalResourcesRequired
#########################################################################*/

/**
 *
 */
class SVGExternalResourcesRequired
{
public:


    /**
     *
     */
    virtual SVGAnimatedBoolean getExternalResourcesRequired()
        { return required; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGExternalResourcesRequired()
        {  }


    /**
     *
     */
    SVGExternalResourcesRequired(const SVGExternalResourcesRequired &other)
        {
        required = other.required;
        }

    /**
     *
     */
    virtual ~SVGExternalResourcesRequired() {}

protected:

    SVGAnimatedBoolean required;

};






/*#########################################################################
## SVGPreserveAspectRatio
#########################################################################*/

/**
 *
 */
class SVGPreserveAspectRatio
{
public:


    /**
     * Alignment Types
     */
    typedef enum
        {
        SVG_PRESERVEASPECTRATIO_UNKNOWN  = 0,
        SVG_PRESERVEASPECTRATIO_NONE     = 1,
        SVG_PRESERVEASPECTRATIO_XMINYMIN = 2,
        SVG_PRESERVEASPECTRATIO_XMIDYMIN = 3,
        SVG_PRESERVEASPECTRATIO_XMAXYMIN = 4,
        SVG_PRESERVEASPECTRATIO_XMINYMID = 5,
        SVG_PRESERVEASPECTRATIO_XMIDYMID = 6,
        SVG_PRESERVEASPECTRATIO_XMAXYMID = 7,
        SVG_PRESERVEASPECTRATIO_XMINYMAX = 8,
        SVG_PRESERVEASPECTRATIO_XMIDYMAX = 9,
        SVG_PRESERVEASPECTRATIO_XMAXYMAX = 10
        } AlignmentType;


    /**
     * Meet-or-slice Types
     */
    typedef enum
        {
        SVG_MEETORSLICE_UNKNOWN  = 0,
        SVG_MEETORSLICE_MEET     = 1,
        SVG_MEETORSLICE_SLICE    = 2
        } MeetOrSliceType;


    /**
     *
     */
    virtual unsigned short getAlign()
        { return align; }

    /**
     *
     */
    virtual void setAlign(unsigned short val) throw (DOMException)
        { align = val; }

    /**
     *
     */
    virtual unsigned short getMeetOrSlice()
        { return meetOrSlice; }

    /**
     *
     */
    virtual void setMeetOrSlice(unsigned short val) throw (DOMException)
        { meetOrSlice = val; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPreserveAspectRatio()
        {
        align       = SVG_PRESERVEASPECTRATIO_UNKNOWN;
        meetOrSlice = SVG_MEETORSLICE_UNKNOWN;
        }

    /**
     *
     */
    SVGPreserveAspectRatio(const SVGPreserveAspectRatio &other)
        {
        align       = other.align;
        meetOrSlice = other.meetOrSlice;
        }

    /**
     *
     */
    virtual ~SVGPreserveAspectRatio() {}

protected:

    unsigned short align;
    unsigned short meetOrSlice;

};






/*#########################################################################
## SVGAnimatedPreserveAspectRatio
#########################################################################*/

/**
 *
 */
class SVGAnimatedPreserveAspectRatio
{
public:


    /**
     *
     */
    virtual SVGPreserveAspectRatio getBaseVal()
        { return baseVal; }

    /**
     *
     */
    virtual SVGPreserveAspectRatio getAnimVal()
        { return animVal; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGAnimatedPreserveAspectRatio() {}

    /**
     *
     */
    SVGAnimatedPreserveAspectRatio(const SVGAnimatedPreserveAspectRatio &other)
        {
        baseVal = other.baseVal;
        baseVal = other.animVal;
        }

    /**
     *
     */
    virtual ~SVGAnimatedPreserveAspectRatio() {}

protected:

    SVGPreserveAspectRatio baseVal;
    SVGPreserveAspectRatio animVal;

};




/*#########################################################################
## SVGFitToViewBox
#########################################################################*/

/**
 *
 */
class SVGFitToViewBox
{
public:

    /**
     *
     */
    virtual SVGAnimatedRect getViewBox()
        { return viewBox; }

    /**
     *
     */
    virtual SVGAnimatedPreserveAspectRatio getPreserveAspectRatio()
        { return preserveAspectRatio; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGFitToViewBox()
        {}

    /**
     *
     */

    SVGFitToViewBox(const SVGFitToViewBox &other)
        {
        viewBox = other.viewBox;
        preserveAspectRatio = other.preserveAspectRatio;
        }

    /**
     *
     */
    virtual ~SVGFitToViewBox() {}

protected:

    SVGAnimatedRect viewBox;

    SVGAnimatedPreserveAspectRatio preserveAspectRatio;

};


/*#########################################################################
## SVGZoomAndPan
#########################################################################*/

/**
 *
 */
class SVGZoomAndPan
{
public:


    /**
     * Zoom and Pan Types
     */
    typedef enum
        {
        SVG_ZOOMANDPAN_UNKNOWN = 0,
        SVG_ZOOMANDPAN_DISABLE = 1,
        SVG_ZOOMANDPAN_MAGNIFY = 2
        } ZoomAndPanType;


    /**
     *
     */
    virtual unsigned short getZoomAndPan()
        { return zoomAndPan; }

    /**
     *
     */
    virtual void setZoomAndPan(unsigned short val) throw (DOMException)
        { zoomAndPan = val; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGZoomAndPan()
        { zoomAndPan = SVG_ZOOMANDPAN_UNKNOWN; }

    /**
     *
     */
    SVGZoomAndPan(const SVGZoomAndPan &other)
        { zoomAndPan = other.zoomAndPan; }

    /**
     *
     */
    virtual ~SVGZoomAndPan() {}

protected:

    unsigned short zoomAndPan;

};






/*#########################################################################
## SVGViewSpec
#########################################################################*/

/**
 *
 */
class SVGViewSpec : public SVGZoomAndPan,
                    public SVGFitToViewBox
{
public:

    /**
     *
     */
    virtual SVGTransformList getTransform()
        { return transform; }

    /**
     *
     */
    virtual SVGElement *getViewTarget()
        { return viewTarget; }

    /**
     *
     */
    virtual DOMString getViewBoxString()
        {
        DOMString ret;
        return ret;
        }

    /**
     *
     */
    virtual DOMString getPreserveAspectRatioString()
        {
        DOMString ret;
        return ret;
        }

    /**
     *
     */
    virtual DOMString getTransformString()
        {
        DOMString ret;
        return ret;
        }

    /**
     *
     */
    virtual DOMString getViewTargetString()
        {
        DOMString ret;
        return ret;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGViewSpec()
        {
        viewTarget = NULL;
        }

    /**
     *
     */
    SVGViewSpec(const SVGViewSpec &other) : SVGZoomAndPan(other), SVGFitToViewBox(other)
        {
        viewTarget = other.viewTarget;
        transform  = other.transform;
        }

    /**
     *
     */
    virtual ~SVGViewSpec() {}

protected:

    SVGElement *viewTarget;
    SVGTransformList transform;
};






/*#########################################################################
## SVGURIReference
#########################################################################*/

/**
 *
 */
class SVGURIReference
{
public:

    /**
     *
     */
    virtual SVGAnimatedString getHref()
        { return href; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGURIReference() {}

    /**
     *
     */
    SVGURIReference(const SVGURIReference &other)
        {
        href = other.href;
        }

    /**
     *
     */
    virtual ~SVGURIReference() {}

protected:

    SVGAnimatedString href;

};






/*#########################################################################
## SVGCSSRule
#########################################################################*/

/**
 *
 */
class SVGCSSRule : public css::CSSRule
{
public:


    /**
     * Additional CSS RuleType to support ICC color specifications
     */
    typedef enum
        {
        COLOR_PROFILE_RULE = 7
        } ColorProfileRuleType;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGCSSRule()
        { type = COLOR_PROFILE_RULE; }

    /**
     *
     */
    SVGCSSRule(const SVGCSSRule &other) : css::CSSRule(other)
        { type = COLOR_PROFILE_RULE; }

    /**
     *
     */
    virtual ~SVGCSSRule() {}

};



/*#########################################################################
## SVGRenderingIntent
#########################################################################*/

/**
 *
 */
class SVGRenderingIntent
{
public:

    /**
     * Rendering Intent Types
     */
    typedef enum
        {
        RENDERING_INTENT_UNKNOWN               = 0,
        RENDERING_INTENT_AUTO                  = 1,
        RENDERING_INTENT_PERCEPTUAL            = 2,
        RENDERING_INTENT_RELATIVE_COLORIMETRIC = 3,
        RENDERING_INTENT_SATURATION            = 4,
        RENDERING_INTENT_ABSOLUTE_COLORIMETRIC = 5
        } RenderingIntentType;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGRenderingIntent()
        {
        renderingIntentType = RENDERING_INTENT_UNKNOWN;
        }

    /**
     *
     */
    SVGRenderingIntent(const SVGRenderingIntent &other)
        {
        renderingIntentType = other.renderingIntentType;
        }

    /**
     *
     */
    virtual ~SVGRenderingIntent() {}

protected:

    unsigned short renderingIntentType;
};







/*#########################################################################
###########################################################################
## P A T H    S E G M E N T S
###########################################################################
#########################################################################*/

static char const *const pathSegLetters[] =
{
    "@", // PATHSEG_UNKNOWN,
    "z", // PATHSEG_CLOSEPATH
    "M", // PATHSEG_MOVETO_ABS
    "m", // PATHSEG_MOVETO_REL,
    "L", // PATHSEG_LINETO_ABS
    "l", // PATHSEG_LINETO_REL
    "C", // PATHSEG_CURVETO_CUBIC_ABS
    "c", // PATHSEG_CURVETO_CUBIC_REL
    "Q", // PATHSEG_CURVETO_QUADRATIC_ABS,
    "q", // PATHSEG_CURVETO_QUADRATIC_REL
    "A", // PATHSEG_ARC_ABS
    "a", // PATHSEG_ARC_REL,
    "H", // PATHSEG_LINETO_HORIZONTAL_ABS,
    "h", // PATHSEG_LINETO_HORIZONTAL_REL
    "V", // PATHSEG_LINETO_VERTICAL_ABS
    "v", // PATHSEG_LINETO_VERTICAL_REL
    "S", // PATHSEG_CURVETO_CUBIC_SMOOTH_ABS
    "s", // PATHSEG_CURVETO_CUBIC_SMOOTH_REL
    "T", // PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS
    "t"  // PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL
};

/*#########################################################################
## SVGPathSeg
#########################################################################*/

/**
 *
 */
class SVGPathSeg
{
public:



    /**
     *  Path Segment Types
     */
    typedef enum
        {
        PATHSEG_UNKNOWN                      = 0,
        PATHSEG_CLOSEPATH                    = 1,
        PATHSEG_MOVETO_ABS                   = 2,
        PATHSEG_MOVETO_REL                   = 3,
        PATHSEG_LINETO_ABS                   = 4,
        PATHSEG_LINETO_REL                   = 5,
        PATHSEG_CURVETO_CUBIC_ABS            = 6,
        PATHSEG_CURVETO_CUBIC_REL            = 7,
        PATHSEG_CURVETO_QUADRATIC_ABS        = 8,
        PATHSEG_CURVETO_QUADRATIC_REL        = 9,
        PATHSEG_ARC_ABS                      = 10,
        PATHSEG_ARC_REL                      = 11,
        PATHSEG_LINETO_HORIZONTAL_ABS        = 12,
        PATHSEG_LINETO_HORIZONTAL_REL        = 13,
        PATHSEG_LINETO_VERTICAL_ABS          = 14,
        PATHSEG_LINETO_VERTICAL_REL          = 15,
        PATHSEG_CURVETO_CUBIC_SMOOTH_ABS     = 16,
        PATHSEG_CURVETO_CUBIC_SMOOTH_REL     = 17,
        PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS = 18,
        PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL = 19
        } PathSegmentType;

    /**
     *
     */
    virtual unsigned short getPathSegType()
        { return type; }

    /**
     *
     */
    virtual DOMString getPathSegTypeAsLetter()
        {
        int typ = type;
        if (typ<0 || typ>PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL)
            typ = PATHSEG_UNKNOWN;
        char const *ch = pathSegLetters[typ];
        DOMString letter = ch;
        return letter;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSeg()
        { type = PATHSEG_UNKNOWN; }

    /**
     *
     */
    SVGPathSeg(const SVGPathSeg &other)
        {
        type = other.type;
        }

    /**
     *
     */
    virtual ~SVGPathSeg() {}

protected:

    int type;

};






/*#########################################################################
## SVGPathSegClosePath
#########################################################################*/

/**
 *
 */
class SVGPathSegClosePath : public SVGPathSeg
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegClosePath()
        {
        type = PATHSEG_CLOSEPATH;
        }

    /**
     *
     */
    SVGPathSegClosePath(const SVGPathSegClosePath &other) : SVGPathSeg(other)
        {
        type = PATHSEG_CLOSEPATH;
        }

    /**
     *
     */
    virtual ~SVGPathSegClosePath() {}

};




/*#########################################################################
## SVGPathSegMovetoAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegMovetoAbs : public SVGPathSeg
{
public:

    /**
     *
     */
    virtual double getX()
        { return x; }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    virtual double getY()
        { return y; }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        { y = val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegMovetoAbs()
        {
        type = PATHSEG_MOVETO_ABS;
        x = y = 0.0;
        }

    /**
     *
     */
    SVGPathSegMovetoAbs(double xArg, double yArg)
        {
        type = PATHSEG_MOVETO_ABS;
        x = xArg; y = yArg;
        }

    /**
     *
     */
    SVGPathSegMovetoAbs(const SVGPathSegMovetoAbs &other) : SVGPathSeg(other)
        {
        type = PATHSEG_MOVETO_ABS;
        x = other.x; y = other.y;
        }

    /**
     *
     */
    virtual ~SVGPathSegMovetoAbs() {}

protected:

    double x,y;

};






/*#########################################################################
## SVGPathSegMovetoRel
#########################################################################*/

/**
 *
 */
class SVGPathSegMovetoRel : public SVGPathSeg
{
public:

    /**
     *
     */
    virtual double getX()
        { return x; }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    virtual double getY()
        { return y; }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        { y = val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegMovetoRel()
        {
        type = PATHSEG_MOVETO_REL;
        x = y = 0.0;
        }


    /**
     *
     */
    SVGPathSegMovetoRel(double xArg, double yArg)
        {
        type = PATHSEG_MOVETO_REL;
        x = xArg; y = yArg;
        }

    /**
     *
     */
    SVGPathSegMovetoRel(const SVGPathSegMovetoRel &other) : SVGPathSeg(other)
        {
        type = PATHSEG_MOVETO_REL;
        x = other.x; y = other.y;
        }

    /**
     *
     */
    virtual ~SVGPathSegMovetoRel() {}

protected:

    double x,y;
};






/*#########################################################################
## SVGPathSegLinetoAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoAbs : public SVGPathSeg
{
public:

    /**
     *
     */
    virtual double getX()
        { return x; }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    virtual double getY()
        { return y; }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        { y = val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegLinetoAbs()
        {
        type = PATHSEG_LINETO_ABS;
        x = y = 0.0;
        }


    /**
     *
     */
    SVGPathSegLinetoAbs(double xArg, double yArg)
        {
        type = PATHSEG_LINETO_ABS;
        x = xArg; y = yArg;
        }

    /**
     *
     */
    SVGPathSegLinetoAbs(const SVGPathSegLinetoAbs &other) : SVGPathSeg(other)
        {
        type = PATHSEG_LINETO_ABS;
        x = other.x; y = other.y;
        }

    /**
     *
     */
    virtual ~SVGPathSegLinetoAbs() {}

protected:

    double x,y;
};






/*#########################################################################
## SVGPathSegLinetoRel
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoRel : public SVGPathSeg
{
public:

    /**
     *
     */
    virtual double getX()
        { return x; }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    virtual double getY()
        { return y; }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        { y = val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegLinetoRel()
        {
        type = PATHSEG_LINETO_REL;
        x = y = 0.0;
        }


    /**
     *
     */
    SVGPathSegLinetoRel(double xArg, double yArg)
        {
        type = PATHSEG_LINETO_REL;
        x = xArg; y = yArg;
        }

    /**
     *
     */
    SVGPathSegLinetoRel(const SVGPathSegLinetoRel &other) : SVGPathSeg(other)
        {
        type = PATHSEG_LINETO_REL;
        x = other.x; y = other.y;
        }

    /**
     *
     */
    virtual ~SVGPathSegLinetoRel() {}

protected:

    double x,y;
};






/*#########################################################################
## SVGPathSegCurvetoCubicAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoCubicAbs : public SVGPathSeg
{
public:

    /**
     *
     */
    virtual double getX()
        { return x; }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    virtual double getY()
        { return y; }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        { y = val; }

    /**
     *
     */
    virtual double getX1()
        { return x1; }

    /**
     *
     */
    virtual void setX1(double val) throw (DOMException)
        { x1 = val; }

    /**
     *
     */
    virtual double getY1()
        { return y1; }

    /**
     *
     */
    virtual void setY1(double val) throw (DOMException)
        { y1 = val; }


    /**
     *
     */
    virtual double getX2()
        { return x2; }

    /**
     *
     */
    virtual void setX2(double val) throw (DOMException)
        { x2 = val; }

    /**
     *
     */
    virtual double getY2()
        { return y2; }

    /**
     *
     */
    virtual void setY2(double val) throw (DOMException)
        { y2 = val; }


    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    SVGPathSegCurvetoCubicAbs()
        {
        type = PATHSEG_CURVETO_CUBIC_ABS;
        x = y = x1 = y1 = x2 = y2 = 0.0;
        }

    /**
     *
     */
    SVGPathSegCurvetoCubicAbs(double xArg,  double yArg,
                              double x1Arg, double y1Arg,
                              double x2Arg, double y2Arg)
        {
        type = PATHSEG_CURVETO_CUBIC_ABS;
        x  = xArg;   y  = yArg;
        x1 = x1Arg;  y1 = y1Arg;
        x2 = x2Arg;  y2 = y2Arg;
        }

    /**
     *
     */
    SVGPathSegCurvetoCubicAbs(const SVGPathSegCurvetoCubicAbs &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_CURVETO_CUBIC_ABS;
        x  = other.x;  y  = other.y;
        x1 = other.x1; y1 = other.y1;
        x2 = other.x2; y2 = other.y2;
        }

    /**
     *
     */
    virtual ~SVGPathSegCurvetoCubicAbs() {}

protected:

    double x, y, x1, y1, x2, y2;

};






/*#########################################################################
## SVGPathSegCurvetoCubicRel
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoCubicRel : public SVGPathSeg
{
public:

    /**
     *
     */
    virtual double getX()
        { return x; }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    virtual double getY()
        { return y; }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        { y = val; }

    /**
     *
     */
    virtual double getX1()
        { return x1; }

    /**
     *
     */
    virtual void setX1(double val) throw (DOMException)
        { x1 = val; }

    /**
     *
     */
    virtual double getY1()
        { return y1; }

    /**
     *
     */
    virtual void setY1(double val) throw (DOMException)
        { y1 = val; }


    /**
     *
     */
    virtual double getX2()
        { return x2; }

    /**
     *
     */
    virtual void setX2(double val) throw (DOMException)
        { x2 = val; }

    /**
     *
     */
    virtual double getY2()
        { return y2; }

    /**
     *
     */
    virtual void setY2(double val) throw (DOMException)
        { y2 = val; }


    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    SVGPathSegCurvetoCubicRel()
        {
        type = PATHSEG_CURVETO_CUBIC_REL;
        x = y = x1 = y1 = x2 = y2 = 0.0;
        }


    /**
     *
     */
    SVGPathSegCurvetoCubicRel(double xArg,  double yArg,
                              double x1Arg, double y1Arg,
                              double x2Arg, double y2Arg)
        {
        type = PATHSEG_CURVETO_CUBIC_REL;
        x  = xArg;   y  = yArg;
        x1 = x1Arg;  y1 = y1Arg;
        x2 = x2Arg;  y2 = y2Arg;
        }

    /**
     *
     */
    SVGPathSegCurvetoCubicRel(const SVGPathSegCurvetoCubicRel &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_CURVETO_CUBIC_REL;
        x  = other.x;  y  = other.y;
        x1 = other.x1; y1 = other.y1;
        x2 = other.x2; y2 = other.y2;
        }

    /**
     *
     */
    virtual ~SVGPathSegCurvetoCubicRel() {}

protected:

    double x, y, x1, y1, x2, y2;

};






/*#########################################################################
## SVGPathSegCurvetoQuadraticAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoQuadraticAbs : public SVGPathSeg
{
public:

    /**
     *
     */
    virtual double getX()
        { return x; }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    virtual double getY()
        { return y; }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        { y = val; }

    /**
     *
     */
    virtual double getX1()
        { return x1; }

    /**
     *
     */
    virtual void setX1(double val) throw (DOMException)
        { x1 = val; }

    /**
     *
     */
    virtual double getY1()
        { return y1; }

    /**
     *
     */
    virtual void setY1(double val) throw (DOMException)
        { y1 = val; }


    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    SVGPathSegCurvetoQuadraticAbs()
        {
        type = PATHSEG_CURVETO_QUADRATIC_ABS;
        x = y = x1 = y1 = 0.0;
        }

    /**
     *
     */
    SVGPathSegCurvetoQuadraticAbs(double xArg,  double yArg,
                              double x1Arg, double y1Arg)
        {
        type = PATHSEG_CURVETO_QUADRATIC_ABS;
        x  = xArg;   y  = yArg;
        x1 = x1Arg;  y1 = y1Arg;
        }

    /**
     *
     */
    SVGPathSegCurvetoQuadraticAbs(const SVGPathSegCurvetoQuadraticAbs &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_CURVETO_QUADRATIC_ABS;
        x  = other.x;  y  = other.y;
        x1 = other.x1; y1 = other.y1;
        }

    /**
     *
     */
    virtual ~SVGPathSegCurvetoQuadraticAbs() {}

protected:

    double x, y, x1, y1;

};






/*#########################################################################
## SVGPathSegCurvetoQuadraticRel
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoQuadraticRel : public SVGPathSeg
{
public:

    /**
     *
     */
    virtual double getX()
        { return x; }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    virtual double getY()
        { return y; }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        { y = val; }

    /**
     *
     */
    virtual double getX1()
        { return x1; }

    /**
     *
     */
    virtual void setX1(double val) throw (DOMException)
        { x1 = val; }

    /**
     *
     */
    virtual double getY1()
        { return y1; }

    /**
     *
     */
    virtual void setY1(double val) throw (DOMException)
        { y1 = val; }


    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    SVGPathSegCurvetoQuadraticRel()
        {
        type = PATHSEG_CURVETO_QUADRATIC_REL;
        x = y = x1 = y1 = 0.0;
        }


    /**
     *
     */
    SVGPathSegCurvetoQuadraticRel(double xArg,  double yArg,
                                  double x1Arg, double y1Arg)
        {
        type = PATHSEG_CURVETO_QUADRATIC_REL;
        x  = xArg;   y  = yArg;
        x1 = x1Arg;  y1 = y1Arg;
        }

    /**
     *
     */
    SVGPathSegCurvetoQuadraticRel(const SVGPathSegCurvetoQuadraticRel &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_CURVETO_QUADRATIC_REL;
        x  = other.x;  y  = other.y;
        x1 = other.x1; y1 = other.y1;
        }

    /**
     *
     */
    virtual ~SVGPathSegCurvetoQuadraticRel() {}

protected:

    double x, y, x1, y1;

};






/*#########################################################################
## SVGPathSegArcAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegArcAbs : public SVGPathSeg
{
public:

    /**
     *
     */
    virtual double getX()
        { return x; }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    virtual double getY()
        { return y; }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        { y = val; }

    /**
     *
     */
    virtual double getR1()
        { return r1; }

    /**
     *
     */
    virtual void setR1(double val) throw (DOMException)
        { r1 = val; }

    /**
     *
     */
    virtual double getR2()
        { return r2; }

    /**
     *
     */
    virtual void setR2(double val) throw (DOMException)
        { r2 = val; }

    /**
     *
     */
    virtual double getAngle()
        { return angle; }

    /**
     *
     */
    virtual void setAngle(double val) throw (DOMException)
        { angle = val; }

    /**
     *
     */
    virtual bool getLargeArcFlag()
        { return largeArcFlag; }

    /**
     *
     */
    virtual void setLargeArcFlag(bool val) throw (DOMException)
        { largeArcFlag = val; }

    /**
     *
     */
    virtual bool getSweepFlag()
        { return sweepFlag; }

    /**
     *
     */
    virtual void setSweepFlag(bool val) throw (DOMException)
        { sweepFlag = val; }

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    SVGPathSegArcAbs()
        {
        type = PATHSEG_ARC_ABS;
        x = y = r1 = r2 = angle = 0.0;
        largeArcFlag = sweepFlag = false;
        }

    /**
     *
     */
    SVGPathSegArcAbs(double xArg,  double yArg,
                     double r1Arg, double r2Arg,
                     double angleArg,
                     bool largeArcFlagArg,
                     bool sweepFlagArg )

        {
        type = PATHSEG_ARC_ABS;
        x  = xArg;   y  = yArg;
        r1 = r1Arg;  r2 = r2Arg;
        angle        = angleArg;
        largeArcFlag = largeArcFlagArg;
        sweepFlag    = sweepFlagArg;
        }

    /**
     *
     */
    SVGPathSegArcAbs(const SVGPathSegArcAbs &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_ARC_ABS;
        x  = other.x;  y  = other.y;
        r1 = other.r1; r2 = other.r2;
        angle        = other.angle;
        largeArcFlag = other.largeArcFlag;
        sweepFlag    = other.sweepFlag;
        }

    /**
     *
     */
    virtual ~SVGPathSegArcAbs() {}

protected:

    double x, y, r1, r2, angle;
    bool largeArcFlag;
    bool sweepFlag;

};



/*#########################################################################
## SVGPathSegArcRel
#########################################################################*/

/**
 *
 */
class SVGPathSegArcRel : public SVGPathSeg
{
public:

    /**
     *
     */
    virtual double getX()
        { return x; }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    virtual double getY()
        { return y; }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        { y = val; }

    /**
     *
     */
    virtual double getR1()
        { return r1; }

    /**
     *
     */
    virtual void setR1(double val) throw (DOMException)
        { r1 = val; }

    /**
     *
     */
    virtual double getR2()
        { return r2; }

    /**
     *
     */
    virtual void setR2(double val) throw (DOMException)
        { r2 = val; }

    /**
     *
     */
    virtual double getAngle()
        { return angle; }

    /**
     *
     */
    virtual void setAngle(double val) throw (DOMException)
        { angle = val; }

    /**
     *
     */
    virtual bool getLargeArcFlag()
        { return largeArcFlag; }

    /**
     *
     */
    virtual void setLargeArcFlag(bool val) throw (DOMException)
        { largeArcFlag = val; }

    /**
     *
     */
    virtual bool getSweepFlag()
        { return sweepFlag; }

    /**
     *
     */
    virtual void setSweepFlag(bool val) throw (DOMException)
        { sweepFlag = val; }

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    SVGPathSegArcRel()
        {
        type = PATHSEG_ARC_REL;
        x = y = r1 = r2 = angle = 0.0;
        largeArcFlag = sweepFlag = false;
        }


    /**
     *
     */
    SVGPathSegArcRel(double xArg, double yArg,
                     double r1Arg, double r2Arg,
                     double angleArg,
                     bool largeArcFlagArg,
                     bool sweepFlagArg )

        {
        type = PATHSEG_ARC_REL;
        x  = xArg;   y  = yArg;
        r1 = r1Arg;  r2 = r2Arg;
        angle        = angleArg;
        largeArcFlag = largeArcFlagArg;
        sweepFlag    = sweepFlagArg;
        }

    /**
     *
     */
    SVGPathSegArcRel(const SVGPathSegArcRel &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_ARC_REL;
        x  = other.x;  y  = other.y;
        r1 = other.r1; r2 = other.r2;
        angle        = other.angle;
        largeArcFlag = other.largeArcFlag;
        sweepFlag    = other.sweepFlag;
        }

    /**
     *
     */
    virtual ~SVGPathSegArcRel() {}

protected:

    double x, y, r1, r2, angle;
    bool largeArcFlag;
    bool sweepFlag;

};






/*#########################################################################
## SVGPathSegLinetoHorizontalAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoHorizontalAbs : public SVGPathSeg
{
public:

    /**
     *
     */
    virtual double getX()
        { return x; }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        { x = val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegLinetoHorizontalAbs()
        {
        type = PATHSEG_LINETO_HORIZONTAL_ABS;
        x = 0.0;
        }


    /**
     *
     */
    SVGPathSegLinetoHorizontalAbs(double xArg)
        {
        type = PATHSEG_LINETO_HORIZONTAL_ABS;
        x = xArg;
        }

    /**
     *
     */
    SVGPathSegLinetoHorizontalAbs(const SVGPathSegLinetoHorizontalAbs &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_LINETO_HORIZONTAL_ABS;
        x = other.x;
        }

    /**
     *
     */
    virtual ~SVGPathSegLinetoHorizontalAbs() {}

protected:

    double x;

};






/*#########################################################################
## SVGPathSegLinetoHorizontalRel
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoHorizontalRel : public SVGPathSeg
{
public:

    /**
     *
     */
    virtual double getX()
        { return x; }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        { x = val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegLinetoHorizontalRel()
        {
        type = PATHSEG_LINETO_HORIZONTAL_REL;
        x = 0.0;
        }


    /**
     *
     */
    SVGPathSegLinetoHorizontalRel(double xArg)
        {
        type = PATHSEG_LINETO_HORIZONTAL_REL;
        x = xArg;
        }

    /**
     *
     */
    SVGPathSegLinetoHorizontalRel(const SVGPathSegLinetoHorizontalRel &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_LINETO_HORIZONTAL_REL;
        x = other.x;
        }

    /**
     *
     */
    virtual ~SVGPathSegLinetoHorizontalRel() {}

protected:

    double x;

};



/*#########################################################################
## SVGPathSegLinetoVerticalAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoVerticalAbs : public SVGPathSeg
{
public:

    /**
     *
     */
    virtual double getY()
        { return y; }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        { y = val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegLinetoVerticalAbs()
        {
        type = PATHSEG_LINETO_VERTICAL_ABS;
        y = 0.0;
        }


    /**
     *
     */
    SVGPathSegLinetoVerticalAbs(double yArg)
        {
        type = PATHSEG_LINETO_VERTICAL_ABS;
        y = yArg;
        }

    /**
     *
     */
    SVGPathSegLinetoVerticalAbs(const SVGPathSegLinetoVerticalAbs &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_LINETO_VERTICAL_ABS;
        y = other.y;
        }

    /**
     *
     */
    virtual ~SVGPathSegLinetoVerticalAbs() {}

protected:

    double y;

};



/*#########################################################################
## SVGPathSegLinetoVerticalRel
#########################################################################*/

/**
 *
 */
class SVGPathSegLinetoVerticalRel : public SVGPathSeg
{
public:

    /**
     *
     */
    virtual double getY()
        { return y; }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        { y = val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegLinetoVerticalRel()
        {
        type = PATHSEG_LINETO_VERTICAL_REL;
        y = 0.0;
        }


    /**
     *
     */
    SVGPathSegLinetoVerticalRel(double yArg)
        {
        type = PATHSEG_LINETO_VERTICAL_REL;
        y = yArg;
        }

    /**
     *
     */
    SVGPathSegLinetoVerticalRel(const SVGPathSegLinetoVerticalRel &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_LINETO_VERTICAL_REL;
        y = other.y;
        }

    /**
     *
     */
    virtual ~SVGPathSegLinetoVerticalRel() {}

protected:

    double y;

};






/*#########################################################################
## SVGPathSegCurvetoCubicSmoothAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoCubicSmoothAbs : public SVGPathSeg
{
public:

    /**
     *
     */
    virtual double getX()
        { return x; }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    virtual double getY()
        { return y; }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        { y = val; }

    /**
     *
     */
    virtual double getX2()
        { return x2; }

    /**
     *
     */
    virtual void setX2(double val) throw (DOMException)
        { x2 = val; }

    /**
     *
     */
    virtual double getY2()
        { return y2; }

    /**
     *
     */
    virtual void setY2(double val) throw (DOMException)
        { y2 = val; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegCurvetoCubicSmoothAbs()
        {
        type = PATHSEG_CURVETO_CUBIC_SMOOTH_ABS;
        x = y = x2 = y2 = 0.0;
        }


    /**
     *
     */
    SVGPathSegCurvetoCubicSmoothAbs(double xArg,   double yArg,
                                    double x2Arg, double y2Arg)
        {
        type = PATHSEG_CURVETO_CUBIC_SMOOTH_ABS;
        x  = xArg;    y  = yArg;
        x2 = x2Arg;   y2 = y2Arg;
        }

    /**
     *
     */
    SVGPathSegCurvetoCubicSmoothAbs(const SVGPathSegCurvetoCubicSmoothAbs &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_CURVETO_CUBIC_SMOOTH_ABS;
        x  = other.x;  y  = other.y;
        x2 = other.x2; y2 = other.y2;
        }

    /**
     *
     */
    virtual ~SVGPathSegCurvetoCubicSmoothAbs() {}

protected:

    double x, y, x2, y2;

};



/*#########################################################################
## SVGPathSegCurvetoCubicSmoothRel
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoCubicSmoothRel : public SVGPathSeg
{
public:

    /**
     *
     */
    virtual double getX()
        { return x; }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    virtual double getY()
        { return y; }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        { y = val; }

    /**
     *
     */
    virtual double getX2()
        { return x2; }

    /**
     *
     */
    virtual void setX2(double val) throw (DOMException)
        { x2 = val; }

    /**
     *
     */
    virtual double getY2()
        { return y2; }

    /**
     *
     */
    virtual void setY2(double val) throw (DOMException)
        { y2 = val; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegCurvetoCubicSmoothRel()
        {
        type = PATHSEG_CURVETO_CUBIC_SMOOTH_REL;
        x = y = x2 = y2 = 0.0;
        }


    /**
     *
     */
    SVGPathSegCurvetoCubicSmoothRel(double xArg,   double yArg,
                                    double x2Arg, double y2Arg)
        {
        type = PATHSEG_CURVETO_CUBIC_SMOOTH_REL;
        x  = xArg;    y  = yArg;
        x2 = x2Arg;   y2 = y2Arg;
        }

    /**
     *
     */
    SVGPathSegCurvetoCubicSmoothRel(const SVGPathSegCurvetoCubicSmoothRel &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_CURVETO_CUBIC_SMOOTH_REL;
        x  = other.x;  y  = other.y;
        x2 = other.x2; y2 = other.y2;
        }

    /**
     *
     */
    virtual ~SVGPathSegCurvetoCubicSmoothRel() {}

protected:

    double x, y, x2, y2;

};






/*#########################################################################
## SVGPathSegCurvetoQuadraticSmoothAbs
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoQuadraticSmoothAbs : public SVGPathSeg
{
public:

    /**
     *
     */
    virtual double getX()
        { return x; }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    virtual double getY()
        { return y; }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        { y = val; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegCurvetoQuadraticSmoothAbs()
        {
        type = PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS;
        x = y = 0.0;
        }


    /**
     *
     */
    SVGPathSegCurvetoQuadraticSmoothAbs(double xArg, double yArg)
        {
        type = PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS;
        x = xArg;     y = yArg;
        }

    /**
     *
     */
    SVGPathSegCurvetoQuadraticSmoothAbs(const SVGPathSegCurvetoQuadraticSmoothAbs &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS;
        x = y = 0.0;
        }

    /**
     *
     */
    virtual ~SVGPathSegCurvetoQuadraticSmoothAbs() {}

protected:

    double x, y;

};






/*#########################################################################
## SVGPathSegCurvetoQuadraticSmoothRel
#########################################################################*/

/**
 *
 */
class SVGPathSegCurvetoQuadraticSmoothRel : public SVGPathSeg
{
public:

    /**
     *
     */
    virtual double getX()
        { return x; }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    virtual double getY()
        { return y; }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        { y = val; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegCurvetoQuadraticSmoothRel()
        {
        type = PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL;
        x = y = 0.0;
        }


    /**
     *
     */
    SVGPathSegCurvetoQuadraticSmoothRel(double xArg, double yArg)
        {
        type = PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL;
        x = xArg;     y = yArg;
        }

    /**
     *
     */
    SVGPathSegCurvetoQuadraticSmoothRel(const SVGPathSegCurvetoQuadraticSmoothRel &other)
                     : SVGPathSeg(other)
        {
        type = PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL;
        x = y = 0.0;
        }

    /**
     *
     */
    virtual ~SVGPathSegCurvetoQuadraticSmoothRel() {}

protected:

    double x, y;

};






/*#########################################################################
## SVGPathSegList
#########################################################################*/

/**
 *
 */
class SVGPathSegList
{
public:

    /**
     *
     */
    virtual unsigned long getNumberOfItems()
        { return items.size(); }


    /**
     *
     */
    virtual void clear () throw( DOMException )
        { items.clear(); }

    /**
     *
     */
    virtual SVGPathSeg initialize (const SVGPathSeg &newItem)
                    throw( DOMException, SVGException )
        {
        items.clear();
        items.push_back(newItem);
        return newItem;
        }

    /**
     *
     */
    virtual SVGPathSeg getItem (unsigned long index)
                    throw( DOMException )
        {
        if (index >= items.size())
            {
            SVGPathSeg seg;
            return seg;
            }
        return items[index];
        }

    /**
     *
     */
    virtual SVGPathSeg insertItemBefore(const SVGPathSeg &newItem,
                                        unsigned long index )
                          throw( DOMException, SVGException )
        {
        if (index >= items.size())
            items.push_back(newItem);
        else
            {
            std::vector<SVGPathSeg>::iterator iter = items.begin() + index;
            items.insert(iter, newItem);
            }
        return newItem;
        }

    /**
     *
     */
    virtual SVGPathSeg replaceItem(const SVGPathSeg &newItem,
                                   unsigned long index )
                              throw( DOMException, SVGException )
        {
        if (index >= items.size())
            {
            SVGPathSeg seg;
            return seg;
            }
        std::vector<SVGPathSeg>::iterator iter = items.begin() + index;
        *iter = newItem;
        return newItem;
        }

    /**
     *
     */
    virtual SVGPathSeg removeItem (unsigned long index)
                                  throw (DOMException)
        {
        if (index >= items.size())
            {
            SVGPathSeg seg;
            return seg;
            }
        std::vector<SVGPathSeg>::iterator iter = items.begin() + index;
        SVGPathSeg olditem = *iter;
        items.erase(iter);
        return olditem;
        }

    /**
     *
     */
    virtual SVGPathSeg appendItem (const SVGPathSeg &newItem)
                    throw( DOMException, SVGException )
        {
        items.push_back(newItem);
        return newItem;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathSegList() {}


    /**
     *
     */
    SVGPathSegList(const SVGPathSegList &other)
        {
        items = other.items;
        }


    /**
     *
     */
    virtual ~SVGPathSegList() {}

protected:

    std::vector<SVGPathSeg> items;

};






/*#########################################################################
## SVGAnimatedPathData
#########################################################################*/

/**
 *
 */
class SVGAnimatedPathData
{
public:

    /**
     *
     */
    virtual SVGPathSegList getPathSegList()
        {
        SVGPathSegList list;
        return list;
        }

    /**
     *
     */
    virtual SVGPathSegList getNormalizedPathSegList()
        {
        SVGPathSegList list;
        return list;
        }

    /**
     *
     */
    virtual SVGPathSegList getAnimatedPathSegList()
        {
        SVGPathSegList list;
        return list;
        }

    /**
     *
     */
    virtual SVGPathSegList getAnimatedNormalizedPathSegList()
        {
        SVGPathSegList list;
        return list;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
   SVGAnimatedPathData()
        {}

    /**
     *
     */
   SVGAnimatedPathData(const SVGAnimatedPathData &/*other*/)
        {
        }

    /**
     *
     */
    virtual ~SVGAnimatedPathData() {}

};






/*#########################################################################
## SVGAnimatedPoints
#########################################################################*/

/**
 *
 */
class SVGAnimatedPoints
{
public:

    /**
     *
     */
    virtual SVGPointList getPoints()
        { return points; }

    /**
     *
     */
    virtual SVGPointList getAnimatedPoints()
        { return animatedPoints; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGAnimatedPoints() {}

    /**
     *
     */
    SVGAnimatedPoints(const SVGAnimatedPoints &other)
        {
        points         = other.points;
        animatedPoints = other.animatedPoints;
        }

    /**
     *
     */
    virtual ~SVGAnimatedPoints() {}

protected:

    SVGPointList points;
    SVGPointList animatedPoints;

};





/*#########################################################################
## SVGPaint
#########################################################################*/

/**
 *
 */
class SVGPaint : public SVGColor
{
public:


    /**
     * Paint Types
     */
    typedef enum
        {
        SVG_PAINTTYPE_UNKNOWN               = 0,
        SVG_PAINTTYPE_RGBCOLOR              = 1,
        SVG_PAINTTYPE_RGBCOLOR_ICCCOLOR     = 2,
        SVG_PAINTTYPE_NONE                  = 101,
        SVG_PAINTTYPE_CURRENTCOLOR          = 102,
        SVG_PAINTTYPE_URI_NONE              = 103,
        SVG_PAINTTYPE_URI_CURRENTCOLOR      = 104,
        SVG_PAINTTYPE_URI_RGBCOLOR          = 105,
        SVG_PAINTTYPE_URI_RGBCOLOR_ICCCOLOR = 106,
        SVG_PAINTTYPE_URI                   = 107
        } PaintType;


    /**
     *
     */
    virtual unsigned short getPaintType()
        { return paintType; }

    /**
     *
     */
    virtual DOMString getUri()
        { return uri; }

    /**
     *
     */
    virtual void setUri (const DOMString& uriArg )
        { uri = uriArg; }

    /**
     *
     */
    virtual void setPaint (unsigned short paintTypeArg,
                           const DOMString& uriArg,
                           const DOMString& /*rgbColor*/,
                           const DOMString& /*iccColor*/ )
                           throw( SVGException )
        {
        paintType = paintTypeArg;
        uri       = uriArg;
        //do something with rgbColor
        //do something with iccColor;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPaint()
        {
        uri       = "";
        paintType = SVG_PAINTTYPE_UNKNOWN;
        }

    /**
     *
     */
    SVGPaint(const SVGPaint &other) : css::CSSValue(other), SVGColor(other)
        {
        uri       = "";
        paintType = SVG_PAINTTYPE_UNKNOWN;
        }

    /**
     *
     */
    virtual ~SVGPaint() {}

protected:

    unsigned int paintType;
    DOMString uri;

};




/*#########################################################################
## SVGColorProfileRule
#########################################################################*/

/**
 *
 */
class SVGColorProfileRule : public SVGCSSRule,
                            public SVGRenderingIntent
{

public:
   /**
     *
     */
    virtual DOMString getSrc()
        { return src; }

    /**
     *
     */
    virtual void setSrc(const DOMString &val) throw (DOMException)
        { src = val; }

    /**
     *
     */
    virtual DOMString getName()
        { return name; }

    /**
     *
     */
    virtual void setName(const DOMString &val) throw (DOMException)
        { name = val; }

    /**
     *
     */
    virtual unsigned short getRenderingIntent()
        { return renderingIntent; }

    /**
     *
     */
    virtual void setRenderingIntent(unsigned short val) throw (DOMException)
        { renderingIntent = val; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGColorProfileRule() {}

    /**
     *
     */
    SVGColorProfileRule(const SVGColorProfileRule &other)
               : SVGCSSRule(other), SVGRenderingIntent(other)
        {
        renderingIntent = other.renderingIntent;
        src             = other.src;
        name            = other.name;
        }

    /**
     *
     */
    virtual ~SVGColorProfileRule() {}

protected:

    unsigned short renderingIntent;
    DOMString src;
    DOMString name;

};



/*#########################################################################
## SVGFilterPrimitiveStandardAttributes
#########################################################################*/

/**
 *
 */
class SVGFilterPrimitiveStandardAttributes : public SVGStylable
{
public:



    /**
     *
     */
    virtual SVGAnimatedLength getX()
        { return x; }

    /**
     *
     */
    virtual SVGAnimatedLength getY()
        { return y; }

    /**
     *
     */
    virtual SVGAnimatedLength getWidth()
        { return width; }

    /**
     *
     */
    virtual SVGAnimatedLength getHeight()
        { return height; }

    /**
     *
     */
    virtual SVGAnimatedString getResult()
        { return result; }



    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    SVGFilterPrimitiveStandardAttributes()
        {}

    /**
     *
     */
    SVGFilterPrimitiveStandardAttributes(const SVGFilterPrimitiveStandardAttributes &other)
                                 : SVGStylable(other)
        {
        x      = other.x;
        y      = other.y;
        width  = other.width;
        height = other.height;
        result = other.result;
        }

    /**
     *
     */
    virtual ~SVGFilterPrimitiveStandardAttributes() {}

protected:

    SVGAnimatedLength x;
    SVGAnimatedLength y;
    SVGAnimatedLength width;
    SVGAnimatedLength height;
    SVGAnimatedString result;

};











/*#########################################################################
## SVGEvent
#########################################################################*/

/**
 *
 */
class SVGEvent : events::Event
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGEvent() {}

    /**
     *
     */
    SVGEvent(const SVGEvent &other) : events::Event(other)
        {}

    /**
     *
     */
    virtual ~SVGEvent() {}

};




/*#########################################################################
## SVGZoomEvent
#########################################################################*/

/**
 *
 */
class SVGZoomEvent : events::UIEvent
{
public:

    /**
     *
     */
    virtual SVGRect getZoomRectScreen()
        { return zoomRectScreen; }

    /**
     *
     */
    virtual double getPreviousScale()
        { return previousScale; }

    /**
     *
     */
    virtual SVGPoint getPreviousTranslate()
        { return previousTranslate; }

    /**
     *
     */
    virtual double getNewScale()
        { return newScale; }

   /**
     *
     */
    virtual SVGPoint getNewTranslate()
        { return newTranslate; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGZoomEvent()
        {}

    /**
     *
     */
    SVGZoomEvent(const SVGZoomEvent &other) : events::Event(other),
                                              events::UIEvent(other)
        {
        zoomRectScreen    = other.zoomRectScreen;
        previousScale     = other.previousScale;
        previousTranslate = other.previousTranslate;
        newScale          = other.newScale;
        newTranslate      = other.newTranslate;
        }

    /**
     *
     */
    virtual ~SVGZoomEvent() {}

protected:

    SVGRect  zoomRectScreen;
    double   previousScale;
    SVGPoint previousTranslate;
    double   newScale;
    SVGPoint newTranslate;

};



/*#########################################################################
## SVGElementInstance
#########################################################################*/

/**
 *
 */
class SVGElementInstance : public events::EventTarget
{
public:

    /**
     *
     */
    virtual SVGElement *getCorrespondingElement()
        { return correspondingElement; }

    /**
     *
     */
    virtual SVGUseElement *getCorrespondingUseElement()
        { return correspondingUseElement; }

    /**
     *
     */
    virtual SVGElementInstance getParentNode()
        {
        SVGElementInstance ret;
        return ret;
        }

    /**
     *  Since we are using stack types and this is a circular definition,
     *  we will instead implement this as a global function below:
     *   SVGElementInstanceList getChildNodes(const SVGElementInstance instance);
     */
    //virtual SVGElementInstanceList getChildNodes();

    /**
     *
     */
    virtual SVGElementInstance getFirstChild()
        {
        SVGElementInstance ret;
        return ret;
        }

    /**
     *
     */
    virtual SVGElementInstance getLastChild()
        {
        SVGElementInstance ret;
        return ret;
        }

    /**
     *
     */
    virtual SVGElementInstance getPreviousSibling()
        {
        SVGElementInstance ret;
        return ret;
        }

    /**
     *
     */
    virtual SVGElementInstance getNextSibling()
        {
        SVGElementInstance ret;
        return ret;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGElementInstance() {}

    /**
     *
     */
    SVGElementInstance(const SVGElementInstance &other)
                        : events::EventTarget(other)
        {
        }

    /**
     *
     */
    virtual ~SVGElementInstance() {}

protected:

    SVGElement    *correspondingElement;
    SVGUseElement *correspondingUseElement;

};






/*#########################################################################
## SVGElementInstanceList
#########################################################################*/

/**
 *
 */
class SVGElementInstanceList
{
public:


    /**
     *
     */
    virtual unsigned long getLength()
        { return items.size(); }

    /**
     *
     */
    virtual SVGElementInstance item (unsigned long index )
        {
        if (index >= items.size())
            {
            SVGElementInstance ret;
            return ret;
            }
        return items[index];
        }

    /**
     *  This static method replaces the circular definition of:
     *        SVGElementInstanceList SVGElementInstance::getChildNodes()
     *
     */
    static SVGElementInstanceList getChildNodes(const SVGElementInstance &/*instance*/)
        {
        SVGElementInstanceList list;
        return list;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGElementInstanceList() {}

    /**
     *
     */
    SVGElementInstanceList(const SVGElementInstanceList &other)
        {
        items = other.items;
        }

    /**
     *
     */
    virtual ~SVGElementInstanceList() {}

protected:

    std::vector<SVGElementInstance> items;


};













}  //namespace svg
}  //namespace dom
}  //namespace w3c
}  //namespace org

#endif /* __SVGTYPES_H__ */
/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

