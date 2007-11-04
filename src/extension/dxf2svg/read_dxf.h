/* Header file for reading dxf information and basic parsing.  Interprting information is found in other files
*/

#ifndef READ_DXF_H
#define READ_DXF_H

#include<vector>



class dxfpair{
public:
    dxfpair(int gcode, char val[10000]);
    virtual ~dxfpair();

    char * value_char(char *string);

    // Leave this data public
    int group_code;
    std::vector< char > value;
};




int section(char* value);  // Convert the section titles into integers

std::vector< std::vector< dxfpair > > dxf_get_sections(char* filename);

std::vector< std::vector< dxfpair > > separate_parts( std::vector< dxfpair > section );  // Find where the major sections are and break into smaller parts

#endif
