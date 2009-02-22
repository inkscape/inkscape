/* Read Blocks from file and convert to vectors of entities
Matt Squires
Google SOC 2005
*/

#ifndef DXF_BLOCKS_H
#define DXF_BLOCKS_H

#include"read_dxf.h"
#include"entities.h"
#include<vector>


class block : public entity, public entities{// : public entities, {
	public:
		block( std::vector< std::vector< dxfpair > > sections ); // Group all of the blocks as entities
		char* name(char* string);
		//void blocks_display();
		
	
	protected:
		char block_name[10000];
		double rotation;
		
	private:
		void block_info(std::vector< dxfpair > info);
};

class blocks{
	public:
		blocks(std::vector< std::vector< dxfpair > > sections);
		block ret_block(char block_name[10000]);
		
	protected:
		std::vector< block > blocks_blocks;
};

#endif
