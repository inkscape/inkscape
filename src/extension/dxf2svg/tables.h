/* Code for the conversion of DXF information in the TABLES section
Matt Squires
SoC
2005*/

#ifndef DXF_TABLES_H
#define DXF_TABLES_H

#include<vector>
#include"read_dxf.h"

class table{
	public:
		int ret_maxN();
	private:
		int max_number;
};


class layer : public table{
	public:
		layer( std::vector< dxfpair > info);
		void display();
		char* name(char* string);
	private:
		char layer_name[10000];
		char ltype_name[10000]; // The layer may also hold the ltype infomation
		int color_number;
		int plotting_flag;
	
};


class ltype : public table{
	public:
		ltype( std::vector< dxfpair > info);
		char* name(char* string);
		std::vector< double > ret_pattern();
	private:
		char ltype_name[10000];
		char descriptive_txt[10000];
		int num_elements;
		double pattern_length;
		std::vector< double > pattern;
	
};


class tables{
	// Well I said that I would only use STL containers internally, but I would have to use a dynamically linked list, and I haven't done for a long time soo STL is my crutch.
	public:
		tables(std::vector< std::vector< dxfpair > > sections); // Put the various entities into their respective vectors
		void display_all();
		
		ltype ret_ltype(char ltype_name[10000], char layer_name[10000]);
		layer ret_layer(char layer_name[10000]);
		
		std::vector< layer > ret_layers();
		
		
	private:
		//void add_dimstyle(polyline pline);
		void add_layer(layer layr);
		void add_ltype(ltype line_type);
		
		std::vector< layer > tables_layer;
		std::vector< ltype > tables_ltype;
};


#endif
