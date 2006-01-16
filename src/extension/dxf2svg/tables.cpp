/*
 * Code for the conversion of DXF information in the TABLES section
 *
 * Author:
 *   Matt Squires <squiresm@colorado.edu>
 *
 * Copyright (C) 2005 Matt Squires
 *
 * Released under GNU GPL and LGPL, read the file 'GPL.txt' and 'LGPL.txt' for details
 */

#include"tables.h"
#include<iostream>




int determine_table(char* value){
	// Common Elements as far as I am concerend
	if ( strncmp(value,"LAYER",5) == 0 ) return 0;	
	if ( strncmp(value,"LTYPE",5) == 0 ) return 1;	
	if ( strncmp(value,"STYLE",5) == 0 ) return 2;
	if ( strncmp(value,"UCS",3) == 0 ) return 3;
	if ( strncmp(value,"VIEW",4) == 0 ) return 4;
	if ( strncmp(value,"VPORT",4) == 0 ) return 5;
	if ( strncmp(value,"APPID",5) == 0 ) return 6;
	if ( strncmp(value,"BLOCK_RECORD",12) == 0 ) return 7;
	else return -1;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TABLE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int table::ret_maxN(){
	return max_number;
}





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LAYER
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


layer::layer( std::vector< dxfpair > info){
	// Get the vertex information
	
	//basic_entity( info );
	//static char string[10000];
	char string[10000];
	for (int i = 0; i < info.size(); i++){
		switch( info[i].group_code ){
			case 2:
				info[i].value_char(layer_name);
				break;
			case 6:
				info[i].value_char(ltype_name);
				break;
				
			case 62:
				info[i].value_char(string);
				color_number = atoi(string);
				//std::cout << "I found a color and its number = " << color_number << std::endl;
				break;
			case 290:
				info[i].value_char(string);
				plotting_flag = atoi(string);				
				break;
		}
	}	
}

void layer::display(){
	std::cout << "LAYER\n";
	//std::cout << "\tx = " << x << "\ty = " << y << "\tz = " << z << "\tbulge = " << bulge << std::flush;
}

char* layer::name(char* string){
	return( strcpy(string,layer_name) );
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LTYPE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ltype::ltype( std::vector< dxfpair > info){
	// Get the linetype information
	
	//static char string[10000];
	char string[10000];
	for (int i = 0; i < info.size(); i++){
		switch( info[i].group_code ){
			case 2:
				info[i].value_char(ltype_name);
				break;
			case 3:
				info[i].value_char(descriptive_txt);
				break;
			case 73:
				info[i].value_char(string);
				num_elements = atoi(string);
				break;
			case 40:
				info[i].value_char(string);
				pattern_length = atof(string);				
				break;
			case 49:
				info[i].value_char(string);
				pattern.push_back( atof(string) );
				break;
		}
	}	
}



char* ltype::name(char* string){
	return( strcpy(string,ltype_name) );
}


std::vector< double > ltype::ret_pattern(){
	return pattern;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// tables
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	


tables::tables(std::vector< std::vector< dxfpair > > sections){
	// Read the main information about the entities section and then put it in the enetites class
	int value;
	char string[10000];
	
	for(int i = 0; i < sections.size(); i++){
		//std::cout << "start" << std::endl;
		sections[i][0].value_char(string);
		value = determine_table(string);
		//std::cout << "sections.size() = " << sections.size() << std::endl << "i = " << i << std::endl << "string = " << string << std::endl;
		switch( value ){
			case 0:
				// LAYER
				//std::cout << "tables start layer " << std::endl;
				tables_layer.push_back( layer( sections[i] ) );
				//std::cout << "tables end layer " << std::endl;
				break;
			
			case 1:
				// LTYPE
				//std::cout << "tables start ltype " << std::endl;
				tables_ltype.push_back( ltype( sections[i] ) );
				//std::cout << "tables end ltype " << std::endl;
				break;
				
			//case 3:
			//	break;
			
			default:
				break;
				// Nothing here
		}
		
	}
	
}



		
ltype tables::ret_ltype(char ltype_name[10000], char layer_name[10000]){
	int string_len = 0;
	char name[10000];
	char temp[10000];
	// The ltype information may be given in the entitity or in the layer information
	// Assume that if there is a name defined in the linetype that it trumps any other layer information
	if ( strlen(ltype_name) > 0 ) strcpy(name,ltype_name); 
	else strcpy(name,layer_name);
	for (int i = 0; i < tables_ltype.size();i++){
		string_len = strlen(tables_ltype[i].name(temp));	
		if (strncmp(tables_ltype[i].name(temp),name,string_len) == 0 ) return tables_ltype[i];	
	}
	return tables_ltype[0];
}


layer tables::ret_layer(char layer_name[10000]){
	int string_len = 0;
	char temp[10000];
	
	for (int i = 0; i < tables_layer.size();i++){
		string_len = strlen(tables_layer[i].name(temp));	
		if (strncmp(tables_layer[i].name(temp),layer_name,string_len) == 0 ) return tables_layer[i];	
	}
	return tables_layer[0];
}


std::vector< layer > tables::ret_layers(){
	return tables_layer;
}
