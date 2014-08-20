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
document.addEventListener("DOMContentLoaded",  jessyInk_core_mouseHandler_noclick_init, false);

/** Initialisation function.
 *  
 *  This function looks for the objects of the appropriate sub-type and hands them to another function that will add the required methods.
 */
function jessyInk_core_mouseHandler_noclick_init()
{
	var elems = document.getElementsByTagNameNS("https://launchpad.net/jessyink", "mousehandler");

	for (var counter = 0; counter < elems.length; counter++)
	{
		if (elems[counter].getAttributeNS("https://launchpad.net/jessyink", "subtype") == "jessyInk_core_mouseHandler_noclick")
			jessyInk_core_mouseHandler_noclick(elems[counter]);
	}
}

/** Function to initialise an object.
 *
 *  @param obj Object to be initialised.
 */
function jessyInk_core_mouseHandler_noclick(obj)
{
	/** Function supplying a custom mouse handler.
	 *
	 *  @returns A dictionary containing the new mouse handler functions.
	 */
	obj.getMouseHandler = function ()
	{
		var handlerDictio = new Object();

		handlerDictio[SLIDE_MODE] = new Object();
		handlerDictio[SLIDE_MODE][MOUSE_DOWN] = null;

		return handlerDictio;
	}
}

