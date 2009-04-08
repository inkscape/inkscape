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

InkWeb.reGetStyleAttVal = function (att) {
  return new RegExp( "(^|.*;)[ ]*"+ att +":([^;]*)(;.*|$)" )
}

InkWeb.getStyle = function (el, att) {
  // This method is needed because el.style is only working
  // to HTML style in the Firefox 3.0
  if ( typeof(el) == "string" )
    el = document.getElementById(el);
  var style = el.getAttribute("style");
  var match = this.reGetStyleAttVal(att).exec(style);
  if ( match ) {
    return match[2];
  } else {
    return false;
  }
}

InkWeb.setStyle = function (el, att, val) {
  if ( typeof(el) == "string" )
    el = document.getElementById(el);
  var style = el.getAttribute("style");
  re = this.reGetStyleAttVal(att);
  if ( re.test(style) ) {
    style = style.replace( re, "$1"+ att +":"+ val +"$3" );
  } else {
    style += ";"+ att +":"+ val;
  }
  el.setAttribute( "style", style );
  return val
}

InkWeb.transmitAtt = function (conf) {
  conf.att = conf.att.split( /\s+/ );
  if ( typeof(conf.from) == "string" )
    conf.from = document.getElementById( conf.from );
  if ( ! conf.to.join )
    conf.to = [ conf.to ];
  for ( var toEl,elN=0; toEl=conf.to[elN]; elN++ ) {
    if ( typeof(toEl) == "string" )
      toEl = document.getElementById( toEl );
    for ( var i=0; i<conf.att.length; i++ ) {
      var val = this.getStyle( conf.from, conf.att[i] );
      if ( val ) {
        this.setStyle( toEl, conf.att[i], val );
      } else {
        val = conf.from.getAttribute(conf.att[i]);
        toEl.setAttribute( conf.att[i], val );
      }
    }
  }
}

InkWeb.setAtt = function (conf) {
  if ( typeof(conf.el) == "string" )
    conf.el = document.getElementById( conf.el );
  conf.att = conf.att.split( /\s+/ );
  conf.val = conf.val.split( /\s+/ );
  var att;
  for ( var i=0; att=conf.att[i]; i++ ) {
    if (
         att == "width"  ||
         att == "height" ||
         att == "x"  ||
         att == "y"  ||
         att == "cx" ||
         att == "cy" ||
         att == "r"  ||
         att == "rx" ||
         att == "ry" ||
         att == "transform"
       ) {
      conf.el.setAttribute( att, conf.val[i] );
    } else {
      this.setStyle( conf.el, att, conf.val[i] );
    }
  }
}

