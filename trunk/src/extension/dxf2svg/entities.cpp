/*
 * Class for interpereting the entities found in a DXF file
 *
 * Author:
 *   Matt Squires <squiresm@colorado.edu>
 *
 * Copyright (C) 2005 Matt Squires
 *
 * Released under GNU GPL and LGPL, read the file 'GPL.txt' and 'LGPL.txt' for details
 */


#include"entities.h"
#include<iostream>
#include<math.h>

int detmine_entity(char* value){
	// Common Elements as far as I am concerend
	if ( strncmp(value,"POLYLINE",8) == 0 ) return 0;	
	if ( strncmp(value,"ARC",3) == 0 ) return 1;	
	if ( strncmp(value,"CIRCLE",6) == 0 ) return 2;
	if ( strncmp(value,"LINE",4) == 0 ) return 3;
	if ( strncmp(value,"SPLINE",6) == 0 ) return 4;
	if ( strncmp(value,"XLINE",5) == 0 ) return 5;
	if ( strncmp(value,"RAY",3) == 0 ) return 6;
	if ( strncmp(value,"DIMENSION",9) == 0 ) return 7;
	if ( strncmp(value,"ELLIPSE",7) == 0 ) return 8;
	if ( strncmp(value,"INSERT",6) == 0 ) return 9;
	if ( strncmp(value,"VERTEX",6) == 0 ) return 10;
	if ( strncmp(value,"TEXT",4) == 0 ) return 11;
	
	// Less Common eletities as far as I am concerend
	if ( strncmp(value,"3DSOLID",7) == 0 ) return 12;
	if ( strncmp(value,"ACAD_PROXY_ENTITY",17) == 0 ) return 13;	
	if ( strncmp(value,"ARCALIGNEDTEXT",14) == 0 ) return 14;
	if ( strncmp(value,"ATTDEF",6) == 0 ) return 15;
	if ( strncmp(value,"ATTRIB",6) == 0 ) return 16;
	if ( strncmp(value,"BODY",4) == 0 ) return 17;
	if ( strncmp(value,"HATCH",5) == 0 ) return 18;
	if ( strncmp(value,"IMAGE",5) == 0 ) return 19;
	if ( strncmp(value,"LEADER",6) == 0 ) return 20;
	if ( strncmp(value,"LWPOLYLINE",10) == 0 ) return 21;
	if ( strncmp(value,"MLINE",5) == 0 ) return 22;
	if ( strncmp(value,"MTEXT",5) == 0 ) return 23;
	if ( strncmp(value,"OLEFRAME",8) == 0 ) return 24;
	if ( strncmp(value,"POINT",5) == 0 ) return 25;	
	if ( strncmp(value,"REGION",6) == 0 ) return 26;
	if ( strncmp(value,"RTEXT",5) == 0 ) return 27;
	if ( strncmp(value,"SEQEND",6) == 0 ) return 28;
	if ( strncmp(value,"SHAPE",5) == 0 ) return 29;
	if ( strncmp(value,"SOLID",5) == 0 ) return 30;	
	if ( strncmp(value,"3DFACE",6) == 0 ) return 31;
	if ( strncmp(value,"TOLERANCE",9) == 0 ) return 32;
	if ( strncmp(value,"TRACE",5) == 0 ) return 33;
	if ( strncmp(value,"VIEWPORT",8) == 0 ) return 34;
	if ( strncmp(value,"WIPEOUT",7) == 0 ) return 35;
	else return -1;
}


void entity::basic_entity( std::vector< dxfpair > info){
	// Extract all of the typical entity information (e.g. layer name, positions)
	static char string[10000];
	for (int i = 0; i < info.size(); i++){
		switch( info[i].group_code ){
			case 6:
				strcpy( string," "); // Clear the string out
				info[i].value_char(string);
				strcpy(linetype,string);
				break;
			case 8:
				strcpy( string," ");  // Clear the string out
				info[i].value_char(string);
				strcpy(layer,string);
				break;
			case 10:
				info[i].value_char(string);
				x = atof(string);
				if ( x < min_x ){
					min_x = x;
				}
				if ( x > max_x ){
					max_x = x;
				}				
				break;
			case 20:
				info[i].value_char(string);
				y = atof(string);
				if ( y < min_y ){
					min_y = y;
				}
				if ( y > max_y ){
					max_y = y;
				}
				break;
			case 30:
				info[i].value_char(string);
				z = atof(string);
				break;
		}
	}
	
}

void entity::entity_display(){
	std::cout << "\tlayer = " << layer << "\n\tlinetype = " << linetype << "\n\tx = " << x << "\ty = " << y << "\tz = " << z << std::flush;
}

double entity::ret_x(){
	return x;
}

double entity::ret_y(){
	return y;
}

double entity::ret_z(){
	return z;
}


char* entity::ret_layer_name(char* string){
	return( strcpy(string,layer) );
}

char* entity::ret_ltype_name(char* string){
	return( strcpy(string,linetype) );
}

double entity::ret_min_x(){
	return min_x;
}

double entity::ret_max_x(){
	return max_x;
}

double entity::ret_min_y(){
	return min_y;
}

double entity::ret_max_y(){
	return max_y;
}

void entity::test_coord(double x, double y){
	if ( x < min_x ){
		min_x = x;
	}
	if ( x > max_x ){
		max_x = x;
	}
	if ( y < min_y ){
		min_y = y;
	}
	if ( y > max_y ){
		max_y = y;
	}
}


void entity::reset_extents(){
	min_x = -1e20;
	max_x = 1e20;
	min_y = -1e20;
	max_y = 1e20;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// VERTEX
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


vertex::vertex( std::vector< dxfpair > info){
	// Get the vertex information
	
	basic_entity( info );
	static char string[10000];
	for (int i = 0; i < info.size(); i++){
		switch( info[i].group_code ){
			case 42:
				info[i].value_char(string);
				bulge = atof(string);
				break;
		}
	}	
}

double vertex::ret_bulge(){
	return bulge;
}

void vertex::display(){
	std::cout << "VERTEX\n";
	std::cout << "\tx = " << x << "\ty = " << y << "\tz = " << z << "\tbulge = " << bulge << std::flush;
}






/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// POLYLINE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// The polyline is handled a little differently compared to the other entities because a POLYLINE is built from a bunch of VERTEX entities
polyline::polyline( std::vector< std::vector< dxfpair > > sections ){
	reset_extents();
	// get the polyline information
	basic_entity( sections[0] );
	points.clear();
	static char string[10000];
	//char string[10000];
	pline_flag = 0;
	curves_flag = 0;
	for (int i = 0; i < sections[0].size(); i++){
		switch( sections[0][i].group_code ){
			case 70:
				sections[0][i].value_char(string);
				pline_flag = atoi(string);
				break;
			case 40:
				sections[0][i].value_char(string);
				start_width = atoi(string);
				break;
			case 41:
				sections[0][i].value_char(string);
				end_width = atoi(string);
				break;
			case 75:
				sections[0][i].value_char(string);
				curves_flag = atoi(string);
				break;
		}
	}
	// Now add the VERTEX entities to the POLYLINE
	for (int i = 1; i < sections.size(); i++){
		points.push_back( vertex( sections[i] ) );
	}
}


std::vector< vertex > polyline::ret_points(){
	return points;
}

double polyline::bulge(int point){
	return points[point].ret_bulge();
}

double polyline::bulge_r(int point){
	// Make sure we are not exceeding the bounds of the points vector
	if (point >= (points.size()-1)) return 0;
	
	double dx = points[point+1].ret_x() - points[point].ret_x();
	double dy = points[point+1].ret_y() - points[point].ret_y();
	double bulge = points[point].ret_bulge();
	double l = sqrt(dx*dx + dy*dy);
	double r = fabs(l*(bulge*bulge+1)/bulge/4);
	
	return r;
}

double polyline::bulge_start_angle(int point){
	// Make sure we are not exceeding the bounds of the points vector
	if (point >= (points.size()-1)) return 0;
	
	double dx = points[point+1].ret_x() - points[point].ret_x();
	double dy = points[point+1].ret_y() - points[point].ret_y();
	double bulge = points[point].ret_bulge();
	double xmid = dx/2 + points[point].ret_x();
	double ymid = dy/2 + points[point].ret_y();
	double l = sqrt(dx*dx + dy*dy);
	double r = fabs(l*(bulge*bulge+1)/bulge/4);
	
	double a = fabs(bulge*l/2);
	double sb = bulge/fabs(bulge); //sign of bulge
	double theta_p = 4*atan(bulge);	
	double theta_c;
	dx != 0 ? theta_c = atan(dy/dx) : theta_c = 1.57079632679489661923;  // Check to make sure that dx is not zero and will give a negative number
	if (dx > 0)	sb *= -1; // Correct for different point ordering and bulge direction

	double cx = xmid + sb*(r-a)*sin(theta_c);
	double cy = ymid - sb*(r-a)*cos(theta_c);
	
	// Now calculate the angle 
	double theta = asin(points[point].ret_x()/r);
	if (dy < 0) theta = 6.2831853 - theta;  // The angle is greater than pi so fix this because max(asin) = pi
	
	return theta;	
}

double polyline::bulge_end_angle(int point){
	// Make sure we are not exceeding the bounds of the points vector
	if (point >= (points.size()-1)) return 0;
	
	double dx = points[point+1].ret_x() - points[point].ret_x();
	double dy = points[point+1].ret_y() - points[point].ret_y();
	double bulge = points[point].ret_bulge();
	double xmid = dx/2 + points[point].ret_x();
	double ymid = dy/2 + points[point].ret_y();
	double l = sqrt(dx*dx + dy*dy);
	double r = fabs(l*(bulge*bulge+1)/bulge/4);
	
	double a = fabs(bulge*l/2);
	double sb = bulge/fabs(bulge); //sign of bulge
	double theta_p = 4*atan(bulge);	
	double theta_c;
	dx != 0 ? theta_c = atan(dy/dx) : theta_c = 1.57079632679489661923;  // Check to make sure that dx is not zero and will give a negative number
	if (dx > 0)	sb *= -1; // Correct for different point ordering and bulge direction

	double cx = xmid + sb*(r-a)*sin(theta_c);
	double cy = ymid - sb*(r-a)*cos(theta_c);
	
	// Now calculate the angle 
	double theta = asin(points[point+1].ret_x()/r);
	if (dy < 0) theta = 6.2831853 - theta;  // The angle is greater than pi so fix this because max(asin) = pi
	
	return theta;	
}


bool polyline::is_closed(){
	// pline-flag holds info about closed pline in the 1 bit.  The info is bit wise encoded so use bit wise operators
	return bool(pline_flag&1);
}

void polyline::display(){
	std::cout << "POLYLINE\n";
	entity_display();
	std::cout << std::endl;
	for (int i = 0; i < points.size(); i++){
		points[i].display();
		std::cout << std::endl;
	}
	std::cout << std::endl;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LWPOLYLINE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// The lwpolyline is different than the polyline because there are no vertex entities.  Use the same basic process as the polyline but parse it out differently
lwpolyline::lwpolyline( std::vector< dxfpair > section ){
	
	// First break up the data into the same format that is used by the polyline entity
	std::vector< std::vector< dxfpair > > sections;
	std::vector< dxfpair > first;
	std::vector< dxfpair > others;
	
	sections.clear();
	first.clear();
	others.clear();
	
	int gc;  // make a shorter name for group_code;
	
	int vertex_part;
	int already_found = 0;
	
	for (int i = 0; i < section.size(); i++){
		gc = section[i].group_code;
		// Encode bitwise information to keep track of what has been found
		vertex_part = 0;
		if (gc == 10 ) vertex_part += 1;
		if (gc == 20 ) vertex_part += 2;
		if (gc == 30 ) vertex_part += 4;
		if (gc == 40 ) vertex_part += 8;
		if (gc == 41 ) vertex_part += 16;
		if (gc == 42 ) vertex_part += 32;
		//std::cout << "\n\nvertex_part = " << vertex_part << std::endl << "already_found = " << already_found << std::endl << "(vertex_part&already_found) = " << (vertex_part&already_found) << std::endl;
		if ( vertex_part == 0 ){
			// If header stuff has been found save it under first.
			// I.E. in a polyline the first set of information is linetype and layer, all of what should be in here
			first.push_back( section[i] );			
		}
		else if ( (vertex_part&already_found) == 0 ){
			// Now work on what would be the vertex information
			// New information is still being found so keep saving it
			others.push_back( section[i] );	
			//std::cout << "add to others" << std::endl;
			already_found += vertex_part;  // Keep track of what has been found			
		}
		else{
			sections.push_back( others );
			//std::cout << "sections.size() = " << sections.size() << std::endl;
			// Now clear the information out and start over
			others.clear();
			others.push_back( section[i] );
			already_found = vertex_part;			
		}
	}
	
	// Now put on the last data that was found 
	if (others.size() > 0 ){
		sections.push_back( others );
	}
	
	reset_extents();
	basic_entity( first );
	points.clear();
	static char string[10000];
	//char string[10000];
	pline_flag = 0;
	curves_flag = 0;
	for (int i = 0; i < first.size(); i++){
		switch( sections[0][i].group_code ){
			case 70:
				first[i].value_char(string);
				pline_flag = atoi(string);
				break;
			case 40:
				first[i].value_char(string);
				start_width = atoi(string);
				break;
			case 41:
				first[i].value_char(string);
				end_width = atoi(string);
				break;
			case 75:
				first[i].value_char(string);
				curves_flag = atoi(string);
				break;
		}
	}
	// Now add the VERTEX entities to the POLYLINE
	for (int i = 0; i < sections.size(); i++){
		points.push_back( vertex( sections[i] ) );
	}
}



std::vector< vertex > lwpolyline::ret_points(){
	return points;
}


double lwpolyline::bulge(int point){
	return points[point].ret_bulge();
}

double lwpolyline::bulge_r(int point){
	// Make sure we are not exceeding the bounds of the points vector
	if (point >= (points.size()-1)) return 0;
	
	double dx = points[point+1].ret_x() - points[point].ret_x();
	double dy = points[point+1].ret_y() - points[point].ret_y();
	double bulge = points[point].ret_bulge();
	double l = sqrt(dx*dx + dy*dy);
	double r = fabs(l*(bulge*bulge+1)/bulge/4);
	
	return r;
}

double lwpolyline::bulge_start_angle(int point){
	// Make sure we are not exceeding the bounds of the points vector
	if (point >= (points.size()-1)) return 0;
	
	double dx = points[point+1].ret_x() - points[point].ret_x();
	double dy = points[point+1].ret_y() - points[point].ret_y();
	double bulge = points[point].ret_bulge();
	double xmid = dx/2 + points[point].ret_x();
	double ymid = dy/2 + points[point].ret_y();
	double l = sqrt(dx*dx + dy*dy);
	double r = fabs(l*(bulge*bulge+1)/bulge/4);
	
	double a = fabs(bulge*l/2);
	double sb = bulge/fabs(bulge); //sign of bulge
	double theta_p = 4*atan(bulge);	
	double theta_c;
	dx != 0 ? theta_c = atan(dy/dx) : theta_c = 1.57079632679489661923;  // Check to make sure that dx is not zero and will give a negative number
	if (dx > 0)	sb *= -1; // Correct for different point ordering and bulge direction

	double cx = xmid + sb*(r-a)*sin(theta_c);
	double cy = ymid - sb*(r-a)*cos(theta_c);
	
	// Now calculate the angle 
	double theta = asin(points[point].ret_x()/r);
	if (dy < 0) theta = 6.2831853 - theta;  // The angle is greater than pi so fix this because max(asin) = pi
	
	return theta;	
}

double lwpolyline::bulge_end_angle(int point){
	// Make sure we are not exceeding the bounds of the points vector
	if (point >= (points.size()-1)) return 0;
	
	double dx = points[point+1].ret_x() - points[point].ret_x();
	double dy = points[point+1].ret_y() - points[point].ret_y();
	double bulge = points[point].ret_bulge();
	double xmid = dx/2 + points[point].ret_x();
	double ymid = dy/2 + points[point].ret_y();
	double l = sqrt(dx*dx + dy*dy);
	double r = fabs(l*(bulge*bulge+1)/bulge/4);
	
	double a = fabs(bulge*l/2);
	double sb = bulge/fabs(bulge); //sign of bulge
	double theta_p = 4*atan(bulge);	
	double theta_c;
	dx != 0 ? theta_c = atan(dy/dx) : theta_c = 1.57079632679489661923;  // Check to make sure that dx is not zero and will give a negative number
	if (dx > 0)	sb *= -1; // Correct for different point ordering and bulge direction

	double cx = xmid + sb*(r-a)*sin(theta_c);
	double cy = ymid - sb*(r-a)*cos(theta_c);
	
	// Now calculate the angle 
	double theta = asin(points[point+1].ret_x()/r);
	if (dy < 0) theta = 6.2831853 - theta;  // The angle is greater than pi so fix this because max(asin) = pi
	
	return theta;	
}

bool lwpolyline::is_closed(){
	// pline-flag holds info about closed pline in the 1 bit.  The info is bit wise encoded so use bit wise operators
	return bool(pline_flag&1);
}

void lwpolyline::display(){
	std::cout << "lwpolyline\n";
	entity_display();
	std::cout << std::endl;
	for (int i = 0; i < points.size(); i++){
		points[i].display();
		std::cout << std::endl;
	}
	std::cout << std::endl;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ARC
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


arc::arc( std::vector< dxfpair > info){
	
	reset_extents();
	basic_entity( info );
	static char string[10000];
	for (int i = 0; i < info.size(); i++){
		switch( info[i].group_code ){
			case 40:
				info[i].value_char(string);
				radius = atof(string);
				test_coord(x+radius,y+radius);
				test_coord(x-radius,y-radius);
				break;
			case 50:
				info[i].value_char(string);
				start_angle = atof(string);
				break;
			case 51:
				info[i].value_char(string);
				end_angle = atof(string);
				break;
			default:
				break;
		}
	}
}

double arc::ret_radius(){
	return radius;
}


double arc::ret_srt_ang(){
	return start_angle;
}


double arc::ret_end_ang(){
	return end_angle;
}


void arc::display(){
	std::cout << "ARC\n";
	entity_display();
	std::cout << "\n\tradius = " << radius << "\tstart_angle = " << start_angle << "end_angle = " << end_angle << std::flush;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CIRCLE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


circle::circle( std::vector< dxfpair > info){
	
	reset_extents();
	basic_entity( info );
	static char string[10000];
	for (int i = 0; i < info.size(); i++){
		switch( info[i].group_code ){
			case 40:
				info[i].value_char(string);
				radius = atof(string);
				test_coord(x+radius,y+radius);
				test_coord(x-radius,y-radius);
				break;
		}
	}
}

void circle::display(){
	std::cout << "CIRCLE\n";
	entity_display();
	std::cout << "\n\tradius = " << radius << std::flush;
}


double circle::ret_radius(){
	return radius;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LINE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


line::line( std::vector< dxfpair > info){
	
	reset_extents();
	basic_entity( info );
	static char string[10000];
	for (int i = 0; i < info.size(); i++){
		switch( info[i].group_code ){
			case 11:
				info[i].value_char(string);
				xf = atof(string);
				break;
			case 21:
				info[i].value_char(string);
				yf = atof(string);
				break;
			case 31:
				info[i].value_char(string);
				zf = atof(string);
				break;
		}
	}
	test_coord(xf,yf);
		
}

void line::display(){
	std::cout << "LINE\n";
	entity_display();
	std::cout << "\n\txf = " << xf << "\tyf = " << yf << "\tzf = " << zf << std::flush;
}


double line::ret_xf(){
	return xf;
}

double line::ret_yf(){
	return yf;
}

double line::ret_zf(){
	return zf;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TEXT
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

text::text( std::vector< dxfpair > info){
	
	reset_extents();
	basic_entity( info );
	static char string[10000];
	for (int i = 0; i < info.size(); i++){
		switch( info[i].group_code ){
			case 1:
				info[i].value_char(dxf_text); // directly copy the text into a string
				break;
			case 40:
				info[i].value_char(string);
				text_height = atof(string);
				break;
			case 50:
				info[i].value_char(string);
				text_rotation = atof(string);
				break;
		}
	}
}

char * text::ret_text(char *string){
	return( strcpy(string,dxf_text) );
}

double text::ret_txt_ht(){
	return text_height;
}

double text::ret_txt_rot(){
	return text_rotation;
}

void text::display(){
	char tmp[10000];
	std::cout << "TEXT\n";
	entity_display();
	std::cout << "\ndxf_text = " << ret_text(tmp) << std::flush;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INSERT
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

insert::insert( std::vector< dxfpair > info){
	
	basic_entity( info );
	static char string[10000];
	for (int i = 0; i < info.size(); i++){
		switch( info[i].group_code ){
			case 2:
				info[i].value_char(block_name); // directly copy the text into a string
				break;
			case 41:
				info[i].value_char(string);
				x_scale_factor = atof(string);
				break;
			case 42:
				info[i].value_char(string);
				y_scale_factor = atof(string);
				break;
			case 43:
				info[i].value_char(string);
				z_scale_factor = atof(string);
				break;
			case 50:
				info[i].value_char(string);
				rotation = atof(string);
				break;
		}
	}
}


char * insert::name(char *string){
	return( strcpy(string,block_name) );
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// entities
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	


entities::entities(std::vector< std::vector< dxfpair > > sections){
	// Read the main information about the entities section and then put it in the enetites class
	int value;
	char string[10000];
	std::vector< dxfpair > single_line;
	std::vector< std::vector< dxfpair > > pline;
	pline.clear();
	single_line.clear();
	
	for(int i = 0; i < sections.size(); i++){
		sections[i][0].value_char(string);
		value = detmine_entity(string);
		switch( value ){
			case 0:
				// Get everything from the start of the polyline designation to an SEQEND value
				pline.clear();  // First clear out the pline information
				do{
					pline.push_back( sections[i] );
					sections[++i][0].value_char(string);
				}while( strncmp(string,"SEQEND",6) != 0 );
				ents_polyline.push_back( polyline( pline ) );
				break;
			
			case 1:
				// ARC
				ents_arc.push_back( arc( sections[i] ) );
				break;
				
			case 2:
				// CIRCLE
				ents_circle.push_back( circle( sections[i] ) );
				break;
				
			case 3:
				// LINE
				ents_line.push_back( line( sections[i] ) );
				break;
			case 11:
				// TEXT
				ents_text.push_back( text( sections[i] ) );
				break;
			case 21:
				// LWPOLYLINE
				ents_lwpolyline.push_back( lwpolyline( sections[i] ) );
			case 9:
				// INSERT
				ents_insert.push_back( insert( sections[i] ) );
			
			//default:
				// Nothing here
		}
		
	}
	
}


// Maybe all of this could be turned into fewer function by using templates, but no time right now.  MBS

std::vector< polyline > entities::ret_plines(){
	std::vector< polyline > pls;
	pls.clear();
	for(int i = 0; i < ents_polyline.size();i++){
		pls.push_back( ents_polyline[i] );
	}
	return pls;
}

std::vector< lwpolyline > entities::ret_lwplines(){
	std::vector< lwpolyline > lwpls;
	lwpls.clear();
	for(int i = 0; i < ents_lwpolyline.size();i++){
		lwpls.push_back( ents_lwpolyline[i] );
	}
	return lwpls;
}


std::vector< arc > entities::ret_arcs(){
	std::vector< arc > a;
	a.clear();
	for(int i = 0; i < ents_arc.size();i++){
		a.push_back( ents_arc[i] );
	}
	return a;
}

std::vector< circle > entities::ret_circles(){
	std::vector< circle > circs;
	circs.clear();
	for(int i = 0; i < ents_circle.size();i++){
		circs.push_back( ents_circle[i] );
	}
	return circs;
}


std::vector< line > entities::ret_lines(){
	std::vector< line > lns;
	lns.clear();
	for(int i = 0; i < ents_line.size();i++){
		lns.push_back( ents_line[i] );
	}
	return lns;
}


std::vector< text > entities::ret_texts(){
	std::vector< text > txts;
	txts.clear();
	for(int i = 0; i < ents_text.size();i++){
		txts.push_back( ents_text[i] );
	}
	return txts;
}


std::vector< insert > entities::ret_inserts(){
	std::vector< insert > ins;
	ins.clear();
	for(int i = 0; i < ents_insert.size();i++){
		ins.push_back( ents_insert[i] );
	}
	return ins;
}



// Overload the return function to depend on the layer
std::vector< polyline > entities::ret_plines(char * layer){
	char temp[10000];
	std::vector< polyline > pls;
	pls.clear();
	
	for(int i = 0; i < ents_polyline.size();i++){
		if ( strcmp( layer,ents_polyline[i].ret_layer_name(temp) ) == 0 ){
			pls.push_back( ents_polyline[i] );
		}
	}
	return pls;
}


std::vector< lwpolyline > entities::ret_lwplines(char * layer){
	char temp[10000];
	std::vector< lwpolyline > lwpls;
	lwpls.clear();
	
	for(int i = 0; i < ents_lwpolyline.size();i++){
		if ( strcmp( layer,ents_lwpolyline[i].ret_layer_name(temp) ) == 0 ){
			lwpls.push_back( ents_lwpolyline[i] );
		}
	}
	return lwpls;
}


std::vector< circle > entities::ret_circles(char * layer){
	char temp[10000];
	std::vector< circle > circs;
	circs.clear();
	
	for(int i = 0; i < ents_circle.size();i++){
		if ( strcmp( layer,ents_circle[i].ret_layer_name(temp) ) == 0 ){
			circs.push_back( ents_circle[i] );
		}
	}
	return circs;
}


std::vector< line > entities::ret_lines(char * layer){
	char temp[10000];
	std::vector< line > lns;
	lns.clear();
	
	for(int i = 0; i < ents_line.size();i++){
		if ( strcmp( layer,ents_line[i].ret_layer_name(temp) ) == 0 ){
			lns.push_back( ents_line[i] );
		}
	}
	return lns;
}


std::vector< text > entities::ret_texts(char * layer){
	char temp[10000];
	std::vector< text > txts;
	txts.clear();
	
	for(int i = 0; i < ents_text.size();i++){
		if ( strcmp( layer,ents_text[i].ret_layer_name(temp) ) == 0 ){
			txts.push_back( ents_text[i] );
		}
	}
	return txts;
}


/*std::vector< ellipse > entities::ret_ellipses(char * layer){
	char temp[10000];
	std::vector< polyline > pls;
	pls.clear();
	
	for(int i = 0; i < ents_polyline.size();i++){
		if ( strcmp( layer,ents_polyline[i].ret_layer_name(temp) ) == 0 ){
			pls.push_back( ents_polyline[i] );
		}
	}
	return pls;
}*/


std::vector< arc > entities::ret_arcs(char * layer){
	char temp[10000];
	std::vector< arc > a;
	a.clear();
	
	for(int i = 0; i < ents_arc.size();i++){
		if ( strcmp( layer,ents_arc[i].ret_layer_name(temp) ) == 0 ){
			a.push_back( ents_arc[i] );
		}
	}
	return a;
}


std::vector< insert > entities::ret_inserts(char * layer){
	char temp[10000];
	std::vector< insert > ins;
	ins.clear();
	
	for(int i = 0; i < ents_insert.size();i++){
		if ( strcmp( layer,ents_insert[i].ret_layer_name(temp) ) == 0 ){
			ins.push_back( ents_insert[i] );
		}
	}
	return ins;
}




void entities::display_all(){
	for (int i = 0; i < ents_polyline.size(); i++){
		ents_polyline[i].display();
	}
	std::cout << std::endl;
	for (int i = 0; i < ents_lwpolyline.size(); i++){
		ents_lwpolyline[i].display();
	}
	std::cout << std::endl;
	for (int i = 0; i < ents_circle.size(); i++){
		ents_circle[i].display();
	}
	std::cout << std::endl;
	for (int i = 0; i < ents_text.size(); i++){
		ents_text[i].display();
	}
}


