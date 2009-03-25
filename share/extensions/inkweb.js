/*
**  InkWeb - Inkscape's Javscript features for the open vector web
**
**  Copyright (C) 2009 Aurelio A. Heckert, aurium (a) gmail dot com
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

var InkWeb = {

  version: 0.01,

  NS: {
    svg:      "http://www.w3.org/2000/svg",
    sodipodi: "http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd",
    inkscape: "http://www.inkscape.org/namespaces/inkscape",
    cc:       "http://creativecommons.org/ns#",
    dc:       "http://purl.org/dc/elements/1.1/",
    rdf:      "http://www.w3.org/1999/02/22-rdf-syntax-ns#",
    xlink:    "http://www.w3.org/1999/xlink",
    xml:      "http://www.w3.org/XML/1998/namespace"
  }

};

InkWeb.transmitAtt = function (conf) {
  if ( typeof(conf.from) == "string" )
    conf.from = document.getElementById(conf.from);
  if ( typeof(conf.to) == "string" )
    conf.to = document.getElementById(conf.to);
  var s = conf.from.getAttribute("style")
  var re = new RegExp("(^|.*;)[ ]*"+conf.att+":([^;]*)(;.*|$)")
  if ( re.test(s) ) {
    var val = s.replace( re, "$2" );
  } else {
    var val = conf.from.getAttribute(conf.att);
  }
  conf.to.setAttribute( conf.att, val );
}
