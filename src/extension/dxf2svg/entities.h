/*  Class for interpereting the entities found in a DXF file
Matt Squires
Google SOC
2 July 05
*/

#ifndef DXF_ENTITIES_H
#define DXF_ENTITIES_H

#include"read_dxf.h"
#include<vector>




class entity{
	public:
		void basic_entity( std::vector< dxfpair > info); // Extract all of the typical entity information (e.g. layer name, positions)
		void entity_display();
		double ret_x();
		double ret_y();
		double ret_z();
		char* ret_layer_name(char* string);
		char* ret_ltype_name(char* string);
		double ret_min_x();
		double ret_max_x();
		double ret_min_y();
		double ret_max_y();	
		
	
	protected:
		char layer[10000];
		char linetype[10000];
		double x;
		double y;
		double z;
		double min_x;
		double max_x;
		double min_y;
		double max_y;
		void reset_extents();
		void test_coord(double x, double y);
};





class vertex : public entity {
	public:
		vertex( std::vector< dxfpair > info);
		void display();
		double ret_bulge();
	
	private:
		double bulge;
};





class polyline : public entity {
	public:
		polyline( std::vector< std::vector< dxfpair > > sections );
		std::vector< vertex > ret_points();
		double bulge(int point);
		double bulge_r(int point);
		double bulge_start_angle(int point);
		double bulge_end_angle(int point);
		bool is_closed();
		void display();
	
	private:
		double buldge;
		int pline_flag; // 70
		double start_width; // 40
		double end_width; // 41
		int curves_flag;
		std::vector< vertex > points;
};

class lwpolyline : public entity {
	public:
		lwpolyline( std::vector< dxfpair > section );
		std::vector< vertex > ret_points();
		double bulge(int point);
		double bulge_r(int point);
		double bulge_start_angle(int point);
		double bulge_end_angle(int point);
		bool is_closed();
		void display();
	
	private:
		double buldge;
		int pline_flag; // 70
		double start_width; // 40
		double end_width; // 41
		int curves_flag;
		std::vector< vertex > points;
};

class arc : public entity {
	public:
		arc( std::vector< dxfpair > info);
		double ret_radius();
		double ret_srt_ang();
		double ret_end_ang();
		void display();
	
	private:
		double radius;
		double start_angle;
		double end_angle;
};




class circle : public entity {
	public:
		circle( std::vector< dxfpair > info);
		void display();
		double ret_radius();
	
	private:
		double radius;
};


class line : public entity {
	public:
		line( std::vector< dxfpair > info);
		void display();
		double ret_xf();
		double ret_yf();
		double ret_zf();
	
	private:
		double xf;
		double yf;
		double zf;
};

class ellipse : public entity {
	public:
		ellipse( std::vector< dxfpair > info);
		void display();
		double ret_x_ma;
		double ret_y_ma;
		double ret_z_ma;
		double ret_ratio;
		double ret_start_p;
		double ret_end_p;
		
	
	private:
		double x_major_axis;
		double y_major_axis;
		double z_major_axis;
		double ratio;
		double start_param;
		double end_param;
};



class text : public entity {
	public:
		text( std::vector< dxfpair > info);
		void display();
		char * ret_text(char *string);
		double ret_txt_ht();
		double ret_txt_rot();
	
	private:
		char dxf_text[10000];
		double text_height; // dxf 40
		double text_rotation; //dxf 50
};


class insert : public entity {
	public:
		insert( std::vector< dxfpair > info);
		void display();
		char* name(char* string);
		double ret_x_sf;
		double ret_y_sf;
		double ret_z_sf;
		double ret_rotation;		
	
	private:
		char block_name[10000];
		double x_scale_factor;
		double y_scale_factor;
		double z_scale_factor;
		double rotation;
};






class entities{
	// Well I said that I would only use STL containers internally, but I would have to use a dynamically linked list, and I haven't done for a long time soo STL is my crutch.
	// I also think that there are others in my same boat that prefer stl containers because they are much easier to use
	public:
		entities(std::vector< std::vector< dxfpair > > sections); // Put the various entities into their respective vectors
		void display_all();
		std::vector< polyline > ret_plines();
		std::vector< circle > ret_circles();
		std::vector< line > ret_lines();
		std::vector< text > ret_texts();
		std::vector< ellipse > ret_ellipses();
		std::vector< arc > ret_arcs();
		std::vector< lwpolyline > ret_lwplines();
		std::vector< insert > ret_inserts();
		// Overload the return function to depend on the layer
		std::vector< polyline > ret_plines(char * layer);
		std::vector< circle > ret_circles(char * layer);
		std::vector< line > ret_lines(char * layer);
		std::vector< text > ret_texts(char * layer);
		std::vector< ellipse > ret_ellipses(char * layer);
		std::vector< arc > ret_arcs(char * layer);
		std::vector< lwpolyline > ret_lwplines(char * layer);
		std::vector< insert > ret_inserts(char * layer);
		
		int plines_size();
		int circles_size();
		int lines_size();
		int texts_size();
		
		
		
	private:
		void add_polyline(polyline pline);
		void add_circle(circle circ);
		void add_line(line ln);
		
		std::vector< polyline > ents_polyline;
		std::vector< arc > ents_arc;
		std::vector< circle > ents_circle;
		std::vector< line > ents_line;
		std::vector< ellipse > ents_ellipse;
		std::vector< text > ents_text;
		std::vector< lwpolyline > ents_lwpolyline;
		std::vector< insert > ents_insert;
		
		
};



#endif
