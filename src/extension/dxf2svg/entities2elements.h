/* Method for converting dxf entities to SVG elements
There are multiple ways for converting different items

If possible most DXF enetities will be converted to paths because that is the most flexable object

Matt Squires
SoC 2005

*/

#include"entities.h"
#include"tables.h"
#include"blocks.h"

// The names indicate the DXF entitiy first and the SVG element last

// Common elements
char* to_arc(double bulge, double r, double start_ang, double end_ang, int precision,char* delim, char * units, double scaling, char *out); // This is used for arcs, polylines, and lwpolylines

// DXF Polyline -> SVG

char* pline2path(polyline pline, char * units, double scaling, tables plot_info);  // Convert a dxf polyline to a SVG path.  This is the closest conversion of the DXF polyline to an SVG element
char* pline2pline(polyline pline, char * units, double scaling, tables plot_info);  // Convert a dxf polyline to a SVG polyline.  The conversion is not 1:1 because the SVG pline doesn't support closed objects or curves
char* pline2polygon(polyline pline, char * units, double scaling, tables plot_info);  // Convert a dxf polyline to a SVG polygon.  The conversion is not 1:1 because the SVG polygone assumes a closed path.  If the pline is not closed it will be forced closed

// DXF LWPolyline -> SVG
char* lwpline2path(lwpolyline pline, char * units, double scaling, tables plot_info);  // Convert a dxf polyline to a SVG path.  This is the closest conversion of the DXF polyline to an SVG element

// DXF ARC -> SVG
char* arc2path(arc a, int precision, char * units, double scaling, tables plot_info, char *out); // So far this appears to be the only way to convert arcs into something recognized by SVG


// DXF Circle -> SVG
char* circle2circle(circle circ, int precision, char * units, double scaling, tables plot_info, char *out); // Direct conversion of DXF circle to SVG circle
char* circle2path(circle circ, int precision, char * units, double scaling, tables plot_info, char *out); // Conversion of DXF circle to SVG circle assuming the path will represent the circle


// DXF Line -> SVG
char* line2line(line ln, int precision, char * units, double scaling, tables plot_info, char *out);  // Directly convert DXF to SVG because it works
char* line2path(line ln, int precision, char * units, double scaling, tables plot_info, char *out);  // Convert DXF line to SVG path

// DXF Text -> SVG
char* text2text(text txt, int precision, char * units, double scaling, tables plot_info, char *out); // Directly convert DXF to SVF assuming VERANDA font

// DXF Insert -> SVG
char* insert2group(insert in, int precision, char * units, double scaling, tables plot_info, blocks blks, char *out);

// Write all of the possible enties with a variety of options

char* write_by_layer(int output_type, entities &ents, tables &tbls, blocks &blks, double scaling, char * units, char * layer, char * out); // Write out the entities by layer designation
char* write_all(int output_type, entities &ents, tables &tbls, blocks &blks, double scaling, char * units, char * out);


