/*
**  InkWeb Debuger - help the development with InkWeb.
**
**  Copyright (C) 2009 Aurelio A. Heckert, aurium (a) gmail dot com
**
**  ********* Bugs and New Fetures *************************************
**   If you found any bug on this script or if you want to propose a
**   new feature, please report it in the inkscape bug traker
**   https://bugs.launchpad.net/inkscape/+filebug
**   and assign that to Aurium.
**  ********************************************************************
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU Lesser General Public License as published
**  by the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
**  ********************************************************************
**
**  This script extends InkWeb with methods like log() and viewProperties().
**  So, you must to call this script after the inkweb.js load.
*/

InkWeb.debugVersion = 0.1;

// Prepare InkWeb Debug:
(function (bli, xyz) {
  // Add logging calls to all InkWeb methods:
  for ( var att in InkWeb ) {
    if ( typeof(InkWeb[att]) == "function" ) {
      var code = InkWeb[att].toString()
      beforeCode = 'this.log(this.__callMethodInfo("'+att+'", arguments));\ntry {';
      afterCode = '} catch(e) { this.log( e, "Ups... There is a problem in InkWeb.'+att+'()" ) }';
      code = code
        .replace( /^(function [^{]+[{])/, "$1\n"+ beforeCode +"\n" )
        .replace( /[}]$/, ";\n"+ afterCode +"\n}" );
      eval( "InkWeb."+att+" = "+ code );
      //alert( InkWeb[att] )
    }
  }
})(123,456);

InkWeb.__callMethodInfo = function (funcName, arg) {
  var func = arg.callee;
  var str = 'Called InkWeb.'+funcName+'() with:'
  if ( ! func.argList ) {
    func.argList = func.toString()
                       .replace( /^function [^(]*\(([^)]*)\)(.|\s)*$/, "$1" )
                       .split( /,\s*/ );
  }
  for ( var a,i=0; a=func.argList[i]; i++ ) {
    str += "\n"+ a +" = "+ this.serialize( arg[i], {recursionLimit:2} );
  }
  return str;
}


InkWeb.copySerializeConf = function (conf) {
  return {
    recursionStep:   conf.recursionStep,
    recursionLimit:  conf.recursionLimit,
    showTagElements: conf.showTagElements
  }
}

InkWeb.serialize = function (v, conf) {
  try {
    if ( ! conf ) { conf = {} }
    if ( ! conf.showTagElements ) { conf.showTagElements = false }
    if ( ! conf.recursionLimit ) { conf.recursionLimit = 10 }
    if ( ! conf.recursionStep ) { conf.recursionStep = 0  }
    if ( conf.recursionLimit == 0 ) {
      return '"<<recursion limit>>"';
    }
    conf.recursionLimit--;
    conf.recursionStep++;
    switch ( typeof(v) ) {
      case "undefined":
        v = "undefined";
        break;
      case "string":
        v = '"'+ v
              .replace( /\n/g, "\\n" )
              .replace( /\r/g, "\\r" )
              .replace( /\t/g, "\\t" )
              .replace( /"/g, '"' ) +
            '"';
        break;
      case "boolean":
      case "number":
      case "function":
        v = v.toString();
        break;
      case "object":
        if ( v == null ) {
          v = "null";
        } else {
          if ( v.constructor == Array ) {
            try {
              v = this.__serializeArray(v, conf);
            } catch(e) {
              this.log( e, "InkWeb.serialize(): Forced recursion limit in" +
                            " recursionLimit="+ conf.recursionLimit +
                            " and recursionStep="+ conf.recursionStep
                      );
              v += '"<<forced recursion limit>>"'
            }
          } else {
            // A Hash Object
            if ( v.tagName && ! conf.showTagElements ) {
              // Tags are not allowed.
              v = '"<'+ v.tagName +' id=\\"'+ v.id +'\\">"';
            } else {
              // Ok, serialize this object:
              try {
                v = this.__serializeObject(v, conf);
              } catch(e) {
                this.log( e, "InkWeb.serialize(): Forced recursion limit in" +
                              " recursionLimit="+ conf.recursionLimit +
                              " and recursionStep="+ conf.recursionStep
                        );
                v += '"<<forced recursion limit>>"'
              }
            }
          }
        }
        break;
      default:
        v = '"<<unknown type '+typeof(v)+' : '+v+'>>"';
    }
    return v;
  } catch(e) {
    this.log( e, "Ups... There is a problem in InkWeb.serialize()." );
  }
}

InkWeb.__serializeArray = function (v, conf) {
  try {
    var vStr = "[ ";
    var size = v.length;
    for ( var i=0; i<size; i++ ) {
      if ( i>0 ) { vStr += ", " }
      vStr += this.serialize(v[i], this.copySerializeConf(conf));
    }
    return vStr +" ]";
  } catch(e) {
    this.log( e, "Ups... There is a problem in InkWeb.__serializeArray()." );
  }
}

InkWeb.__serializeObject = function (obj, conf) {
  try {
    var vStr = "{ ";
    var first = true;
    for ( var att in obj ) {
      if ( !first ) { vStr += ", " }
      vStr += this.serialize(att) +':'+
              this.serialize( obj[att], this.copySerializeConf(conf) );
      first = false;
    }
    return vStr +" }";
  } catch(e) {
    this.log( e, "Ups... There is a problem in InkWeb.__serializeObject()." );
  }
}

// Allow log configuration:
InkWeb.mustLog = {
    error: true,
    warning: true,
    sequence: true
  };

// This will keep the log information:
InkWeb.__log__ = [];

InkWeb.log = function (type, msg) {
  /* This method register what was happen with InkWeb
  ** ( if mustLog allows that )
  **
  ** --- Usage ---
  ** this.log( <"sequence"|"warning"|"warn"|errorObject>, <"logMessage"> );
  ** this.log( <"logMessage"> );  // only for sequences
  **
  ** --- Examples ---
  ** Sequence log:
  ** function foo (bar) {
  **   InkWeb.log( 'Call function foo with argument bar="'+bar+'"' );
  **
  ** Warning log:
  ** if ( foo == bar ) {
  **   foo = other;
  **   InkWeb.log( "warn", "foo must not be bar." );
  **
  ** Error log:
  ** try { ... some hard thing ... }
  ** catch (e) { InkWeb.log( e, "Trying to do some hard thing." ) }
  */
  if ( this.mustLog ) {
    if( type.constructor == ReferenceError ) {
      // in a error loging the type argument is the error object.
      var error = type;
      type = "error";
      this.addViewLogBt();
    }
    if( type == "warn" ) {
      // that allows a little simplify in the log call.
      type = "warning";
    }
    if( msg == undefined ) {
      // that allows to log a sequence without tos say the type.
      msg = type;
      type = "sequence";
    }
    var logSize = this.__log__.length
    if ( logSize > 0 &&
         this.__log__[logSize-1].type == type &&
         this.__log__[logSize-1].msg == msg ) {
      this.__log__[logSize-1].happens++
    } else {
      if ( type == "error" && this.mustLog.error ) {
        this.__log__[logSize] = this.__logError( error, msg )
      }
      if ( type == "warning" && this.mustLog.warning ) {
        this.__log__[logSize] = this.__logWarning( msg )
      }
      if ( type == "sequence" && this.mustLog.sequence ) {
        this.__log__[logSize] = this.__logSequence( msg )
      }
    }
  }
}

InkWeb.__logError = function ( error, msg ) {
  return { type:"error", date:new Date(), msg:msg, error:error, happens:1 };
}

InkWeb.__logWarning = function ( msg ) {
  return { type:"warning", date:new Date(), msg:msg, happens:1 };
}

InkWeb.__logSequence = function ( msg ) {
  return { type:"sequence", date:new Date(), msg:msg, happens:1 };
}

InkWeb.logToString = function (conf) {
  /* Show the log in a formated string.
  ** conf attributes:
  **   format: a string to format the log itens.
  **   formatError: to format the error log itens.
  **   sep: the log itens separator string.
  ** format variables:
  **   $F: the item date in the format YYYY-MM-DD
  **   $T: the item time in the format HH:MM:SS
  **   $type: the log type
  **   $logmsg: the text argument in the log call
  **   $times: how much times this item happens in sequence
  **   $error: the error text (if this is a error item)
  **   $index: the position of the item in the log list
  **   $oddeven: return odd or even based in the index.
  */
  if (!conf)        { conf        = {}     }
  if (!conf.sep)    { conf.sep    = "\n\n" }
  if (!conf.format) { conf.format = "$F $T - $type - $logmsg - Happens $times." }
  if (!conf.formatError) { conf.formatError = "$F $T - ERROR - $logmsg - Happens $times.\n$error" }
  /* * * Helper * * */
  function _2d(num) {
    return ( ( num < 10 )? "0"+num : ""+num )
  }
  function _2dMonth(date) {
    var m = date.getMonth() + 1;
    return _2d( m )
  }
  var str = "";
  var logSize = this.__log__.length;
  if ( logSize == 0 ) {
    str = "There are no errors.";
  }
  // View all itens to mount the log string:
  for ( var item,pos=0; item=this.__log__[pos]; pos++ ) {
    var d = item.date;
    // Add log line, converting variables:
    var line = ( (item.type=="error")? conf.formatError : conf.format );
    str += line
           .replace( /\$index/g,   pos )
           .replace( /\$oddeven/g, (pos%2 == 1)? "odd" : "even" )
           .replace( /\$type/g,    item.type )
           .replace( /\$logmsg/g,  item.msg  )
           .replace( /\$error/g,   (item.error)? item.error.message : "" )
           .replace( /\$times/g,   (item.happens>1)? item.happens+" times" : "one time" )
           .replace( /\$F/g, d.getFullYear() +"-"+ _2dMonth(d) +"-"+ _2d(d.getDate()) )
           .replace( /\$T/g, _2d(d.getHours()) +":"+ _2d(d.getMinutes()) +":"+ _2d(d.getSeconds()) )
    // Add separator:
    if ( pos < (logSize-1) ) { str += conf.sep }
  }
  return str;
}

InkWeb.addViewLogBt = function () {
  var svg = document.getElementsByTagName("svg")[0];
  if ( this.__viewLogBt ) {
    svg.appendChild( this.__viewLogBt );
  } else {
    var g = this.el( "g", { onclick: "InkWeb.openLogWindow()", parent: svg } );
    var rect = this.el( "rect", { x: 10, y: 10, width: 60, height: 17, ry: 5,
                                  style: "fill:#C00; stroke:#800; stroke-width:2",
                                  parent: g } );
    var text = this.el( "text", { x: 40, y: 22, text: "View Log",
                                  style: "fill:#FFF; font-size:10px;" +
                                         "font-family:sans-serif;" +
                                         "text-anchor:middle; text-align:center",
                                  parent: g } );
    this.__viewLogBt = g;
  }
}

InkWeb.__openFormatedWindow = function (bodyHTML) {
  var win = window.open("","_blank","width=500,height=500,scrollbars=yes");
  var html =
    '<html><head><title>InkWeb</title>' +
    '<style type="text/css">' +
    'body { font-family:sans-serif; font-size:12px; padding:5px; margin:0px; }' +
    'h1 { font-size:13px; text-align:center; }' +
    '.error    { color: #C00 }' +
    '.warning  { color: #B90 }' +
    '.sequence { color: #06A }' +
    'table { border: 2px solid #ABC }' +
    'th, td { padding:1px 2px; font-size:12px }' +
    'th { text-align:center; background:#CCC; color:#FFF }' +
    '.odd  { background: #F0F0F0 }' +
    '.even { background: #F8F8F8 }' +
    '</style><body>'+ bodyHTML +'</body></html>';
  win.document.write(html);
  win.document.close();
  return win;
}

InkWeb.openLogWindow = function () {
  var html = '<h1>InkWeb Log</h1>\n' +
    '<table border="0" width="100%" cellpadding="2" cellspacing="0"><tr>\n' +
    '<tr><th>Time</th><th>Message</th><th><small>Happens</small></th><tr>\n' +
    this.logToString({
      format:      '<tr class="$type $oddeven" title="$type">' +
                   '<td>$T</td><td>$logmsg</td><td align="right">$times</td></tr>',
      formatError: '<tr class="error $oddeven" title="ERROR">' +
                   '<td>$T</td><td>$logmsg</td><td align="right">$times</td></tr>\n'+
                   '<tr class="error $oddeven"><td colspan="3"><code>$error</code></td></tr>',
      sep: '\n</tr><tr>\n'
    }) +
    '\n</tr></table>'
  var win = this.__openFormatedWindow( html );
  win.document.title = "InkWeb Log"
}


InkWeb.viewProperties = function () {
  // Display object properties.
  this.__openFormatedWindow( "coming soon..." );
}

