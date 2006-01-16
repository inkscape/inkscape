/* test read_dxf  */
#include"read_dxf.h"
#include"entities.h"
#include"blocks.h"
#include"tables.h"
#include<iostream>
#include"entities2elements.h"

using namespace std;

int main(){
	std::vector< std::vector< dxfpair > >  output, entities_info, blocks_info, tables_info;
	//output =  dxf_get_sections("circ_sqr.dxf");
	//output =  dxf_get_sections("Labjack.dxf");
	//output =  dxf_get_sections("mini_post.dxf");
	//output =  dxf_get_sections("../8599-E0W.dxf");
	//output =  dxf_get_sections("../bulged_lwpoly.dxf");
	//output =  dxf_get_sections("../aspheric_lens.dxf");
	output =  dxf_get_sections("../layers_colors.dxf");
	/*std::vector< dxfpair > header;
	std::vector< dxfpair > classes;
	std::vector< dxfpair > tables;
	std::vector< dxfpair > blocks;
	std::vector< dxfpair > entities;
	std::vector< dxfpair > objects;
	std::vector< dxfpair > thumbnailimage;
	*/
	
	
	//dxf_get_sections("circ_sqr.dxf",header,classes,tables,blocks,entities,objects,thumbnailimage);
	//dxf_get_sections("mini_post.dxf",header,classes,tables,blocks,entities,objects,thumbnailimage);
	
	//cout << entities.size() << endl;
	for (int i=0;i<output.size();i++) cout << "output[i].size()" << i << " = " << output[i].size() << endl;
	//cout << endl;
	//for (int i=0;i<output[0].size();i++) cout << output[0][i].group_code << "\t"  << output[0][i].value << endl;
	//cout << endl;
	//for (int i=0;i<output[4].size();i++) cout << output[4][i].group_code << "\t" << output[4][i].value << endl;
	
	// Now display the different parts of the entities section
	//cout << endl;
	for (int i=0;i < output[4].size();i++){
		//cout << output[4][i].group_code << "  " << output[4][i].value[0] << output[4][i].value[1] << endl;
	}
	cout << endl;
	entities_info = separate_parts(output[4]);
	cout << "entities_info.size() = " << entities_info.size() << endl;
	
	for (int i=0;i < entities_info.size();i++){
		//for (int j=0;j < entities_info[i].size();j++) cout << entities_info[i][j].group_code << "  " << entities_info[i][j].value[0] << entities_info[i][j].value[1] << entities_info[i][j].value[2] << entities_info[i][j].value[3] << entities_info[i][j].value[4] << "\n";
		//cout << endl << endl << endl;
	}
	
	cout << "blocks separate parts" << endl;
	blocks_info = separate_parts(output[3]); // Tables is the 4th part of a dxf file.
	cout << "sort separated parts" << endl;
	blocks blks(blocks_info);  // Sort the information in the tables
	cout << "blocks_info.size() = " << blocks_info.size() << endl;
	
	for (int i=0;i < blocks_info.size();i++){
		//for (int j=0;j < blocks_info[i].size();j++) cout << blocks_info[i][j].group_code << "  " << blocks_info[i][j].value[0] << blocks_info[i][j].value[1] << blocks_info[i][j].value[2] << blocks_info[i][j].value[3] << blocks_info[i][j].value[4] << "\n";
		//cout << endl << endl << endl;
	}
	
	
	tables_info = separate_parts(output[2]); // Tables is the 3rd part of a dxf file.
	cout << "tables_info.size() = " << tables_info.size() << endl;
	for (int i=0;i < tables_info.size();i++){
		//for (int j=0;j < tables_info[i].size();j++) cout << tables_info[i][j].group_code << "  " << tables_info[i][j].value[0] << tables_info[i][j].value[1] << tables_info[i][j].value[2] << tables_info[i][j].value[3] << tables_info[i][j].value[4] << "\n";
		//cout << "i = " << i << endl << endl;
	}
	tables tbls(tables_info);  // Sort the information in the tables
	
	
	
	cout << "This should change \n";
	
	cout << "\n\nNow display all parsed info \n";
	entities ents(entities_info);
	ents.display_all();
	
	//char units[5] = "in";
	char units[5] = "";
	
	cout << "\nNow for the begining of the SVG...\n\n";
	
	cout << "Polyline conversion\n";
	//std::vector< polyline > pl_tmp= ents.ret_plines();
	//cout << "pline2pline\n" << pline2pline(pl_tmp[0], units ) << endl;
	//cout << "pline2pline\n" << pline2pline(ents.ret_plines()[0], units ) << endl;
	//cout << "pline2path\n" << pline2path(ents.ret_plines()[0], units );
	
	char tmp_char[10000];
	
	cout << "\nCircle conversion\n";
	//cout << "circle2circle\n" << circle2circle(ents.ret_circles()[0], 3, units, tmp_char) << endl;
	
	return 0;
}
