/*
 * Read Blocks from file and convert to vectors of entities
 *
 * Author:
 *   Matt Squires <squiresm@colorado.edu>
 *
 * Copyright (C) 2005 Matt Squires
 *
 * Released under GNU GPL and LGPL, read the file 'GPL.txt' and 'LGPL.txt' for details
 */


#include"blocks.h"
#include<iostream>

block::block(std::vector< std::vector< dxfpair > > sections) : entities( sections ){
	// Inherit most of the functionality of the entitites section
	
	basic_entity( sections[0] );
	block_info( sections[0] );
}

char* block::name(char* string){
	return( strcpy(string,block_name) );
}


void block::block_info( std::vector< dxfpair > info){
	static char string[10000];
	for (int i = 0; i < info.size(); i++){
		switch( info[i].group_code ){
			case 2: // Block name
				strcpy( string," "); // Clear the string out
				info[i].value_char(string);
				strcpy(block_name,string);
				break;
		}
	}
}






blocks::blocks(std::vector< std::vector< dxfpair > > sections){
	// Read the main information about the entities section and then put it in the enetites class
	int value;
	char string[10000];
	std::vector< dxfpair > single_line;
	std::vector< std::vector< dxfpair > > ents;
	ents.clear();
	single_line.clear();
	
	int n_loop = sections.size();
	n_loop--;
	//for(int i = 0; i < (sections.size()-1); i++){ // It is odd but the last value seems to be bad so don't use it
	// I am not really sure if I need the -1.  I needed it once upon a time to make things work but I don't have time to test it well right now 
	// But sections.size() is an unsigned int so when you subtract 1 it becomes 4294967295 and tries to run the loop so work around that by making n_loop that is signed
	for(int i = 0; i < n_loop; i++){ // It is odd but the last value seems to be bad so don't use it
		sections[i][0].value_char(string);
		ents.clear();  // First clear out the pline information
		
		
		// Get everything from the start of the BLOCK designation to an ENDBLK value
		if ( strncmp(string,"BLOCK",5) == 0 && (i < sections.size())){
			do{
				ents.push_back( sections[i] );
				sections[++i][0].value_char(string);
			}while( strncmp(string,"ENDBLK",6) != 0  && (i < sections.size()-1) );
			blocks_blocks.push_back( block( ents ) );
		}
	}
}

block blocks::ret_block(char block_name[10000]){
	int string_len = 0;
	char temp[10000];
	
	for (int i = 0; i < blocks_blocks.size();i++){
		string_len = strlen(blocks_blocks[i].name(temp));	
		if (strncmp(blocks_blocks[i].name(temp),block_name,string_len) == 0 ) return blocks_blocks[i];	
	}
	return blocks_blocks[0];
}



