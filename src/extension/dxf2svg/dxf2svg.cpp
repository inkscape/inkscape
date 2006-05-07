/*
 * Build a SVG from an dxf, will support conversion to Inkscape types
 *
 * Author:
 *   Matt Squires <squiresm@colorado.edu>
 *
 * Copyright (C) 2005 Matt Squires
 *
 * Released under GNU GPL and LGPL, read the file 'GPL.txt' and 'LGPL.txt' for details
 */


#include<fstream>
#include<iostream>
#include"read_dxf.h"
#include"entities.h"
#include"blocks.h"
#include"entities2elements.h"








int main(int argc,char *argv[]){
	// Later include options for different conversions like converting as much as possible into paths
	int ink = 1; // Assume for now there is no inkscape stuff to add extra
	
	if(argc > 1){
		double scaling = 90;  // converstion from in to pt
		
		// Read the DXF file
		std::vector< std::vector< dxfpair > >  output, entities_info, tables_info, blocks_info;
		//std::cout << "About to read file \n" << std::endl;
		output =  dxf_get_sections(argv[1]);
		//std::cout << "Finished reading file \n" << std::endl;
		
		entities_info = separate_parts(output[4]); // Entities is the 5th part of the file.
		entities ents(entities_info); // Sort entities into their respective parts
		
		tables_info = separate_parts(output[2]); // Tables is the 3rd part of a dxf file.
		tables tbls(tables_info);  // Sort the information in the tables
			
		blocks_info = separate_parts(output[3]); // Tables is the 4th part of a dxf file.
		blocks blks(blocks_info);  // Sort the information in the tables
		
		
		
		// Get the various file informations
		/*std::vector< polyline > plines = ents.ret_plines();
		std::vector< lwpolyline > lwplines = ents.ret_lwplines();
		std::vector< arc > arcs = ents.ret_arcs();
		std::vector< circle > circs = ents.ret_circles();
		std::vector< line > lns = ents.ret_lines();
		std::vector< text > txts = ents.ret_texts();
		std::vector< insert > ins = ents.ret_inserts();
		*/
		

		std::vector< layer >  layers = tbls.ret_layers();
		
		char units[5] = "in";
		char tmp_char[100000];
		char layer_string[500];
		
		if (ink < 1){
			// Write a general svg header
			std::cout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n\t\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n<svg xmlns=\"http://www.w3.org/2000/svg\"\n\txmlns:xlink=\"http://www.w3.org/1999/xlink\">\n";
			std::cout << "\tx=\"0.00000000\"\n\ty=\"0.00000000\"\n\twidth=\"744.09448\"\n\theight=\"-1052.3622\"" << std::endl;
		}
		else{
			std::cout << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << std::endl;
			std::cout << "<!-- Created with dxf2svg -->" << std::endl;
			std::cout << "<svg" << std::endl;
   			std::cout << "\txmlns:dc=\"http://purl.org/dc/elements/1.1/\"" << std::endl;
   			std::cout << "\txmlns:cc=\"http://web.resource.org/cc/\"" << std::endl;
   			std::cout << "\txmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"" << std::endl;
   			std::cout << "\txmlns:svg=\"http://www.w3.org/2000/svg\"" << std::endl;
   			std::cout << "\txmlns=\"http://www.w3.org/2000/svg\"" << std::endl;
   			std::cout << "\txmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\"" << std::endl;
   			std::cout << "\txmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\"" << std::endl;
   			std::cout << "\t>" << std::endl;

		}

		
		// Now write SVG elements to file
		if ( layers.size() < 1 ){
			write_all(0, ents, tbls, blks, scaling, units, tmp_char);
		}
		else{
			for (int i = 0; i < layers.size(); i++){
				std::cout << "\t<g\n\t\tinkscape:label=\"" << layers[i].name(layer_string) <<  "\"\n\t\tinkscape:groupmode=\"layer\"\n\t\tid=\"layer" << i+1 << "\">" << std::endl;
				write_by_layer(0, ents, tbls, blks, scaling, units, layers[i].name(layer_string), tmp_char);
				std::cout << "\t</g>" << std::endl;
			}
		}
		
		// Close SVG
		std::cout << "</svg>";
	}
	
	return 0;
}
