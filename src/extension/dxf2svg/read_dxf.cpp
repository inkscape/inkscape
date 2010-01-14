/*
 * For reading and slight parsing of dxf files
 *
 * Author:
 *   Matt Squires <squiresm@colorado.edu>
 *
 * Copyright (C) 2005 Matt Squires
 *
 * Released under GNU GPL and LGPL, read the file 'GPL.txt' and 'LGPL.txt' for details
 */



#include <fstream>
#include <string>
#include "read_dxf.h"

#include <iostream>
#include <string.h>
#include <stdlib.h>
using namespace std;


int MAX_STR_LN = 10000;

int section(char* value){
	if ( strncmp(value,"HEADER",6) == 0 ) return 0;
	if ( strncmp(value,"CLASSES",7) == 0 ) return 1;
	if ( strncmp(value,"TABLES",6) == 0 ) return 2;
	if ( strncmp(value,"BLOCKS",6) == 0 ) return 3;
	if ( strncmp(value,"ENTITIES",8) == 0 ) return 4;
	if ( strncmp(value,"OBJECTS",7) == 0 ) return 5;
	if ( strncmp(value,"THUMBNAILIMAGE",14) == 0 ) return 6;
}


dxfpair::dxfpair(int gcode, char val[10000]){
		group_code = gcode;
		// Dynamically save the strings, otherwise the memory uses is bad
		
		for (int i = 0; i < strlen(val); i++){
			value.push_back(val[i]);
		}		
}


dxfpair::~dxfpair(){
	//delete [] value;
}

char * dxfpair::value_char(char *string){
	int size = value.size();
	while( ( size > 0 ) && int(value[size-1]) < 33){
		// Strip off any control characters and spaces off the end of the string
		size--;
	}
	for(int i = 0; i < size; i++){
		string[i] = value[i];
	}
	string[size]=0;
}

std::vector< std::vector< dxfpair > > dxf_get_sections(char* filename){
	// In the dxf format information is paired into group codes that indicate the information that follows on the next line.  The information on the next line is called the value
	
	int n =0;
	
	int group_code;
	char value[MAX_STR_LN];
	
	int section_num;
	
	
	
	
	std::vector< std::vector< dxfpair > > out;
	
	std::vector< dxfpair > header;
	std::vector< dxfpair > classes;
	std::vector< dxfpair > tables;
	std::vector< dxfpair > blocks;
	std::vector< dxfpair > entities;
	std::vector< dxfpair > objects;
	std::vector< dxfpair > thumbnailimage;
	
	header.clear();
	classes.clear();
	tables.clear();
	blocks.clear();
	entities.clear();
	objects.clear();
	thumbnailimage.clear();
	
	
	// Open dxf file for reading
	std::ifstream file(filename);
	
	if (!file.is_open()){
		exit (1);  // Change this to an exception
	}
	
	// Find the first SECTION header
	
	while ( (!file.eof()) ){ 
		n++;
		
		// get the first group code and value
		file.getline(value,MAX_STR_LN);
		group_code = atoi(value);
		file.getline(value,MAX_STR_LN);
		
		do{	
				
			// TO DO     set all the chars to be caps for later comparison
		
			// Find the SECTION codes
			if ( (group_code == 0 ) && ( strncmp(value,"SECTION",7) == 0 ) ){
				// Directly after a section value is the type of section ( e.g. HEADER, TABLES )
				file.getline(value,MAX_STR_LN);
				group_code = atoi(value);
				file.getline(value,MAX_STR_LN);
				section_num = section( value );
				if ( group_code == 2 ){
					// Make sure the the group code is 2 for the SECTION name
					// This is a big block of mostly repetitive code, it will result in larger code, but would be faster than putting the switch in another while loop.  If I still live in a time when file size mattered alot I would change it
					//std::cout << "section_num = " << section_num << std::endl;
					switch ( section_num ){
						case 0:
							file.getline(value,MAX_STR_LN);
							group_code = atoi(value);
							file.getline(value,MAX_STR_LN);
							do{
								header.push_back( dxfpair( group_code, value ) );
								file.getline(value,MAX_STR_LN);
								group_code = atoi(value);
								file.getline(value,MAX_STR_LN);
							}while( ( (group_code != 0) || strncmp(value,"ENDSEC",6) != 0 ) && (!file.eof()) );  // I put in the (group_code != 0) in the hope that it will be a faster bool compare than the string compare.  Test this later
							break;
						case 1:
							file.getline(value,MAX_STR_LN);
							group_code = atoi(value);
							file.getline(value,MAX_STR_LN);
							if ( (group_code != 0) || (strncmp(value,"ENDSEC",6) != 0) ){
								// Some dxf files have blank sections.  These are not handled by the do/while loop so break about if needed
								do{
									classes.push_back( dxfpair( group_code, value ) );
									file.getline(value,MAX_STR_LN);
									group_code = atoi(value);
									file.getline(value,MAX_STR_LN);
								}while( ( (group_code != 0) || strncmp(value,"ENDSEC",6) != 0 ) && (!file.eof()) );  // I put in the (group_code != 0) in the hope that it will be a faster bool compare than the string compare.  Test this later
							}
							break;
						case 2:
							file.getline(value,MAX_STR_LN);
							group_code = atoi(value);
							file.getline(value,MAX_STR_LN);
							do{
								tables.push_back( dxfpair( group_code, value ) );
								file.getline(value,MAX_STR_LN);
								group_code = atoi(value);
								file.getline(value,MAX_STR_LN);
							}while( ( (group_code != 0) || strncmp(value,"ENDSEC",6) != 0 ) && (!file.eof()) );
							break;
						case 3:
							file.getline(value,MAX_STR_LN);
							group_code = atoi(value);
							file.getline(value,MAX_STR_LN);
							do{
								blocks.push_back( dxfpair( group_code, value ) );
								file.getline(value,MAX_STR_LN);
								group_code = atoi(value);
								file.getline(value,MAX_STR_LN);
							}while( ( (group_code != 0) || strncmp(value,"ENDSEC",6) != 0 ) && (!file.eof()) );  // I put in the (group_code != 0) in the hope that it will be a faster bool compare than the string compare.  Test this later
							break;
						case 4:
							file.getline(value,MAX_STR_LN);
							group_code = atoi(value);
							file.getline(value,MAX_STR_LN);
							do{
								entities.push_back( dxfpair( group_code, value ) );
								file.getline(value,MAX_STR_LN);
								group_code = atoi(value);
								file.getline(value,MAX_STR_LN);
							}while( ( (group_code != 0) || strncmp(value,"ENDSEC",6) != 0 ) && (!file.eof()) );  // I put in the (group_code != 0) in the hope that it will be a faster bool compare than the string compare.  Test this later
							break;
						case 5:
							file.getline(value,MAX_STR_LN);
							group_code = atoi(value);
							file.getline(value,MAX_STR_LN);
							do{
								objects.push_back( dxfpair( group_code, value ) );
								file.getline(value,MAX_STR_LN);
								group_code = atoi(value);
								file.getline(value,MAX_STR_LN);
							}while( ( (group_code != 0) || strncmp(value,"ENDSEC",6) != 0 ) && (!file.eof()) );  // I put in the (group_code != 0) in the hope that it will be a faster bool compare than the string compare.  Test this later
							break;
						case 6:
							file.getline(value,MAX_STR_LN);
							group_code = atoi(value);
							file.getline(value,MAX_STR_LN);
							do{
								thumbnailimage.push_back( dxfpair( group_code, value ) );
								file.getline(value,MAX_STR_LN);
								group_code = atoi(value);
								file.getline(value,MAX_STR_LN);
							}while( ( (group_code != 0) || strncmp(value,"ENDSEC",6) != 0 ) && (!file.eof()) );  // I put in the (group_code != 0) in the hope that it will be a faster bool compare than the string compare.  Test this later
							break;
						default:
							file.getline(value,MAX_STR_LN);
							group_code = atoi(value);
							file.getline(value,MAX_STR_LN);	
					}
				}
			}	
			file.getline(value,MAX_STR_LN);
			group_code = atoi(value);
			file.getline(value,MAX_STR_LN);
			
			n++;
		}while( ( strncmp(value,"EOF",3) != 0 ) && (!file.eof()) ); 
	}
	
	out.push_back(header);
	out.push_back(classes);
	out.push_back(tables);
	out.push_back(blocks);
	out.push_back(entities);
	out.push_back(objects);
	out.push_back(thumbnailimage);
	
	return out;
}


std::vector< std::vector< dxfpair > > separate_parts( std::vector< dxfpair > section){
	//std::cout << "1" << std::endl;
	//std::cout << "section.size() = " << section.size() << std::endl;
	// Find where the major sections are and break into smaller parts
	// Major section is defined as anything beween group_code 0 to 0
	std::vector< dxfpair > inner;
	std::vector< std::vector< dxfpair > > outer;
	//std::cout << "2" << std::endl;
	for (int i = 0; i < section.size(); i++){
		//std::cout << "i = " << i << std::endl;
		//std::cout << "section[i].value.size() = " << section[i].value.size() << std::endl;
		
		// Make sure no control codes like LF or CR are making it past this section
		if ( (section[i].value.size() > 0) && int(section[i].value.back()) < 32 ){
			 section[i].value.pop_back();
		 }
		//for(int j = 0;j < section[i].value.size();j++ ) std::cout << section[i].value[j];
		//std::cout << std::endl;
		
		inner.push_back( section[i] );
		
		// If the next group code is 0 then push the previously found info on outer and start looking for data again
		if (section[i+1].group_code == 0){
			//std::cout << "inner.push_back" << std::endl;
			outer.push_back( inner );
			inner.clear();
		}
	}
	// Because putting the data on outer depends on find a GC=0 the last bit of data may be left behind so it inner has data in it put it on outer
	if ( inner.size() > 0 ){
		outer.push_back( inner );
		inner.clear();
	}
	//std::cout << "3" << std::endl;
	if (section.back().group_code == 0){
		//outer.push_back( inner ); // Put the last part on if there is information, but I don't think it needs to.
	}
	
	return outer;
}

