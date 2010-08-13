// Copyright 2008, 2009 Hannes Hochreiner
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.

// Add event listener for initialisation.
document.addEventListener("DOMContentLoaded",  jessyInk_core_mouseHandler_zoomControl_init, false);

/** Initialisation function.
 *  
 *  This function looks for the objects of the appropriate sub-type and hands them to another function that will add the required methods.
 */
function jessyInk_core_mouseHandler_zoomControl_init()
{
	var elems = document.getElementsByTagNameNS("https://launchpad.net/jessyink", "mousehandler");

	for (var counter = 0; counter < elems.length; counter++)
	{
		if (elems[counter].getAttributeNS("https://launchpad.net/jessyink", "subtype") == "jessyInk_core_mouseHandler_zoomControl")
			jessyInk_core_mouseHandler_zoomControl(elems[counter]);
	}
}

/** Function to initialise an object.
 *
 *  @param obj Object to be initialised.
 */
function jessyInk_core_mouseHandler_zoomControl(obj)
{
	// Last dragging position.
	obj.dragging_last;
	// Flag to indicate whether dragging is active currently.
	obj.dragging_active = false;
	// Flag to indicate whether dragging is working currently.
	obj.dragging_working = false;
	// Flag to indicate whether the user clicked.
	obj.click = false;

	/** Function supplying a custom mouse handler.
	 *
	 *  @returns A dictionary containing the new mouse handler functions.
	 */
	obj.getMouseHandler = function ()
	{
		var handlerDictio = new Object();

		handlerDictio[SLIDE_MODE] = new Object();
		handlerDictio[SLIDE_MODE][MOUSE_DOWN] = obj.mousedown;
		handlerDictio[SLIDE_MODE][MOUSE_MOVE] = obj.mousemove;
		handlerDictio[SLIDE_MODE][MOUSE_UP] = obj.mouseup;
		handlerDictio[SLIDE_MODE][MOUSE_WHEEL] = obj.mousewheel;

		return handlerDictio;
	}

	/** Event handler for mouse clicks.
	 *
	 *  @param e Event object.
	 */
	obj.mouseclick = function (e)
	{
		var elem = obj.getAdHocViewBbox(slides[activeSlide]["viewGroup"], obj.getCoords(e));

		processingEffect = true;

		effectArray = new Array();

		effectArray[0] = new Object();
		effectArray[0]["effect"] = "view";
		effectArray[0]["dir"] = 1;
		effectArray[0]["element"] = slides[activeSlide]["viewGroup"];
		effectArray[0]["options"] = new Object();
		effectArray[0]["options"]["length"] = 200;

		if (elem == null)
			effectArray[0]["options"]["matrixNew"] = (new matrixSVG()).fromSVGElements(1, 0, 0, 1, 0, 0);
		else
			effectArray[0]["options"]["matrixNew"] = obj.pointMatrixToTransformation(obj.rectToMatrix(elem)).mult((new matrixSVG()).fromSVGMatrix(slides[activeSlide].viewGroup.getScreenCTM()).inv().mult((new matrixSVG()).fromSVGMatrix(elem.parentNode.getScreenCTM())).inv());

		transCounter = 0;
		startTime = (new Date()).getTime();
		lastFrameTime = null;
		effect(1);

		return false;
	}

	/** Function to search for the element the user clicked on.
	 *
	 *	This function searches for the element with the highest z-order, which encloses the point the user clicked on
	 *	and which view box fits entierly into the currently visible part of the slide.
	 *
	 *  @param elem Element to start the search from.
	 *  @param pnt Point where the user clicked.
	 *  @returns The element the user clicked on or null, if no element could be found.
	 */
	obj.getAdHocViewBbox = function (elem, pnt)
	{
		var children = elem.childNodes;
		
		for (var counter = 0; counter < children.length; counter++)
		{
			if (children[counter].getBBox)
			{
				var childPointList = obj.projectRect(children[counter].getBBox(), children[counter].getScreenCTM());

				var viewBbox = document.documentElement.createSVGRect();

				viewBbox.x = 0.0;
				viewBbox.y = 0.0;
				viewBbox.width = WIDTH;
				viewBbox.height = HEIGHT;

				var screenPointList = obj.projectRect(viewBbox, slides[activeSlide]["element"].getScreenCTM());

				if (obj.pointsWithinRect([pnt], childPointList) && obj.pointsWithinRect(childPointList, screenPointList))
					return children[counter];

				child = obj.getAdHocViewBbox(children[counter], pnt);

				if (child != null)
					return child;
			}
		}

		return null;
	}

	/** Function to project a rectangle using the projection matrix supplied.
	 *
	 *  @param rect The rectangle to project.
	 *  @param projectionMatrix The projection matrix.
	 *  @returns A list of the four corners of the projected rectangle starting from the upper left corner and going counter-clockwise.
	 */
	obj.projectRect = function (rect, projectionMatrix)
	{
		var pntUL = document.documentElement.createSVGPoint();
		pntUL.x = rect.x;
		pntUL.y = rect.y;
		pntUL = pntUL.matrixTransform(projectionMatrix);

		var pntLL = document.documentElement.createSVGPoint();
		pntLL.x = rect.x;
		pntLL.y = rect.y + rect.height;
		pntLL = pntLL.matrixTransform(projectionMatrix);

		var pntUR = document.documentElement.createSVGPoint();
		pntUR.x = rect.x + rect.width;
		pntUR.y = rect.y;
		pntUR = pntUR.matrixTransform(projectionMatrix);

		var pntLR = document.documentElement.createSVGPoint();
		pntLR.x = rect.x + rect.width;
		pntLR.y = rect.y + rect.height;
		pntLR = pntLR.matrixTransform(projectionMatrix);

		return [pntUL, pntLL, pntUR, pntLR];
	}

	/** Function to determine whether all the points supplied in a list are within a rectangle.
	 *
	 *  @param pnts List of points to check.
	 *  @param pointList List of points representing the four corners of the rectangle.
	 *  @return True, if all points are within the rectangle; false, otherwise.
	 */
	obj.pointsWithinRect = function (pnts, pointList)
	{
		var pntUL = pointList[0];
		var pntLL = pointList[1];
		var pntUR = pointList[2];

		var matrixOrig = (new matrixSVG()).fromElements(pntUL.x, pntLL.x, pntUR.x, pntUL.y, pntLL.y, pntUR.y, 1, 1, 1);
		var matrixProj = (new matrixSVG()).fromElements(0, 0, 1, 0, 1, 0, 1, 1, 1);

		var matrixProjection = matrixProj.mult(matrixOrig.inv());

		for (var blockCounter = 0; blockCounter < Math.ceil(pnts.length / 3.0); blockCounter++)
		{
			var subPnts = new Array();

			for (var pntCounter = 0; pntCounter < 3.0; pntCounter++)
			{
				if (blockCounter * 3.0 + pntCounter < pnts.length)
					subPnts[pntCounter] = pnts[blockCounter * 3.0 + pntCounter];
				else
				{
					var tmpPnt = document.documentElement.createSVGPoint();
					
					tmpPnt.x = 0.0;
					tmpPnt.y = 0.0;

					subPnts[pntCounter] = tmpPnt;
				}
			}

			var matrixPnt = (new matrixSVG).fromElements(subPnts[0].x, subPnts[1].x, subPnts[2].x, subPnts[0].y, subPnts[1].y, subPnts[2].y, 1, 1, 1);
			var matrixTrans = matrixProjection.mult(matrixPnt);

			for (var pntCounter = 0; pntCounter < 3.0; pntCounter++)
			{
				if (blockCounter * 3.0 + pntCounter < pnts.length)
				{
					if ((pntCounter == 0) && !((matrixTrans.e11 > 0.01) && (matrixTrans.e11 < 0.99) && (matrixTrans.e21 > 0.01) && (matrixTrans.e21 < 0.99)))
						return false;
					else if ((pntCounter == 1) && !((matrixTrans.e12 > 0.01) && (matrixTrans.e12 < 0.99) && (matrixTrans.e22 > 0.01) && (matrixTrans.e22 < 0.99)))
						return false;
					else if ((pntCounter == 2) && !((matrixTrans.e13 > 0.01) && (matrixTrans.e13 < 0.99) && (matrixTrans.e23 > 0.01) && (matrixTrans.e23 < 0.99)))
						return false;
				}
			}
		}

		return true;
	}

	/** Event handler for mouse movements.
	 *
	 *  @param e Event object.
	 */
	obj.mousemove = function (e)
	{
		obj.click = false;

		if (!obj.dragging_active || obj.dragging_working)
			return false;

		obj.dragging_working = true;

		var p = obj.getCoords(e);

		if (slides[activeSlide].viewGroup.transform.baseVal.numberOfItems < 1)
		{
			var matrix = (new matrixSVG()).fromElements(1, 0, 0, 0, 1, 0, 0, 0, 1);
		}
		else
		{
			var matrix = (new matrixSVG()).fromSVGMatrix(slides[activeSlide].viewGroup.transform.baseVal.consolidate().matrix);
		}

		matrix.e13 += p.x - obj.dragging_last.x;
		matrix.e23 += p.y - obj.dragging_last.y;

		slides[activeSlide]["viewGroup"].setAttribute("transform", matrix.toAttribute());

		obj.dragging_last = p;
		obj.dragging_working = false;

		return false;
	}

	/** Event handler for mouse down.
	 *
	 *  @param e Event object.
	 */
	obj.mousedown = function (e)
	{
		if (obj.dragging_active)
			return false;

		var value = 0;

		if (e.button)
			value = e.button;
		else if (e.which)
			value = e.which;

		if (value == 1)
		{
			obj.dragging_last = obj.getCoords(e);
			obj.dragging_active = true;
			obj.click = true;
		}

		return false;
	}

	/** Event handler for mouse up.
	 *
	 *  @param e Event object.
	 */
	obj.mouseup = function (e)
	{
		obj.dragging_active = false;

		if (obj.click)
			return obj.mouseclick(e);
		else
			return false;
	}

	/** Function to get the coordinates of a point corrected for the offset of the viewport.
	 *
	 *  @param e Point.
	 *  @returns Coordinates of the point corrected for the offset of the viewport.
	 */
	obj.getCoords = function (e)
	{
		var svgPoint = document.documentElement.createSVGPoint();
		svgPoint.x = e.clientX + window.pageXOffset;
		svgPoint.y = e.clientY + window.pageYOffset;

		return svgPoint;
	}

	/** Event handler for scrolling.
	 *
	 *  @param e Event object.
	 */
	obj.mousewheel = function(e)
	{
		var p = obj.projectCoords(obj.getCoords(e));

		if (slides[activeSlide].viewGroup.transform.baseVal.numberOfItems < 1)
		{
			var matrix = (new matrixSVG()).fromElements(1, 0, 0, 0, 1, 0, 0, 0, 1);
		}
		else
		{
			var matrix = (new matrixSVG()).fromSVGMatrix(slides[activeSlide].viewGroup.transform.baseVal.consolidate().matrix);
		}

		if (e.wheelDelta)
		{ // IE Opera
			delta = e.wheelDelta/120;
		}
		else if (e.detail)
		{ // MOZ
			delta = -e.detail/3;
		}

		var widthOld = p.x * matrix.e11 + p.y * matrix.e12;
		var heightOld = p.x * matrix.e21 + p.y * matrix.e22;

		matrix.e11 *= (1.0 - delta / 20.0);
		matrix.e12 *= (1.0 - delta / 20.0);
		matrix.e21 *= (1.0 - delta / 20.0);
		matrix.e22 *= (1.0 - delta / 20.0);

		var widthNew = p.x * matrix.e11 + p.y * matrix.e12;
		var heightNew = p.x * matrix.e21 + p.y * matrix.e22;

		matrix.e13 += (widthOld - widthNew);
		matrix.e23 += (heightOld - heightNew);

		slides[activeSlide]["viewGroup"].setAttribute("transform", matrix.toAttribute());

		return false;
	}

	/** Function to project a point to screen coordinates.
	 *
	 *	@param Point.
	 *	@returns The point projected to screen coordinates.
	 */
	obj.projectCoords = function(pnt)
	{
		var matrix = slides[activeSlide]["element"].getScreenCTM();

		if (slides[activeSlide]["viewGroup"])
			matrix = slides[activeSlide]["viewGroup"].getScreenCTM();

		pnt = pnt.matrixTransform(matrix.inverse());
		return pnt;
	}

	/** Function to convert a rectangle into a point matrix.
	 *
	 *	The function figures out a rectangle that encloses the rectangle given and has the same width/height ratio as the viewport of the presentation.
	 *
	 *  @param rect Rectangle.
	 *  @return The upper left, upper right and lower right corner of the rectangle in a point matrix.
	 */
	obj.rectToMatrix = function(rect)
	{
		rectWidth = rect.getBBox().width;
		rectHeight = rect.getBBox().height;
		rectX = rect.getBBox().x;
		rectY = rect.getBBox().y;
		rectXcorr = 0;
		rectYcorr = 0;

		scaleX = WIDTH / rectWidth;
		scaleY = HEIGHT / rectHeight;

		if (scaleX > scaleY)
		{
			scaleX = scaleY;
			rectXcorr -= (WIDTH / scaleX - rectWidth) / 2;
			rectWidth = WIDTH / scaleX;
		}	
		else
		{
			scaleY = scaleX;
			rectYcorr -= (HEIGHT / scaleY - rectHeight) / 2;
			rectHeight = HEIGHT / scaleY;
		}

		if (rect.transform.baseVal.numberOfItems < 1)
		{
			mRectTrans = (new matrixSVG()).fromElements(1, 0, 0, 0, 1, 0, 0, 0, 1);
		}
		else
		{
			mRectTrans = (new matrixSVG()).fromSVGMatrix(rect.transform.baseVal.consolidate().matrix);
		}

		newBasePoints = (new matrixSVG()).fromElements(rectX, rectX, rectX, rectY, rectY, rectY, 1, 1, 1);
		newVectors = (new matrixSVG()).fromElements(rectXcorr, rectXcorr + rectWidth, rectXcorr + rectWidth, rectYcorr, rectYcorr, rectYcorr + rectHeight, 0, 0, 0);

		return mRectTrans.mult(newBasePoints.add(newVectors));
	}

	/** Function to return a transformation matrix from a point matrix.
	 *
	 *	@param mPoints The point matrix.
	 *	@returns The transformation matrix.
	 */
	obj.pointMatrixToTransformation = function(mPoints)
	{
		mPointsOld = (new matrixSVG()).fromElements(0, WIDTH, WIDTH, 0, 0, HEIGHT, 1, 1, 1);

		return mPointsOld.mult(mPoints.inv());
	}
}

