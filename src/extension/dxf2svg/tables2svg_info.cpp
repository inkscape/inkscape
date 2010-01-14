/*
 * Convert DXF table information to a format that is recognized by SVG
 *
 * Author:
 *   Matt Squires <squiresm@colorado.edu>
 *
 * Copyright (C) 2005 Matt Squires
 *
 * Released under GNU GPL and LGPL, read the file 'GPL.txt' and 'LGPL.txt' for details
 */


#include "tables2svg_info.h"
#include <math.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>

char* pattern2dasharray(ltype info, int precision, double scaling, char* out){
	std::vector< double > pattern = info.ret_pattern();
	char temp[50];
	char *out_ptr;
	
	
	if (pattern.size() > 0){
		strcat(out," stroke-dasharray=\"");	
		for(int i = 0; i < pattern.size()-1;i++){
			strcat(out,gcvt(scaling*sqrt(pow(pattern[i],2)),precision,temp) );
			strcat(out,",");
		}
		strcat( out,gcvt(scaling*sqrt(pow(pattern[pattern.size()-1],2)),precision,temp) );
		strcat(out,"\" ");
	}
	
	out_ptr = out;
	return out_ptr;
}
