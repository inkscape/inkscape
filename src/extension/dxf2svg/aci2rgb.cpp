#include <iostream>
#include <sstream>
#include <string>


char* RGB(double R, double G, double B);
char* RGB(double R, double G, double B){
	int r = int (R);
	int g = int (G);
	int b = int (B);
	
	char out[6];
	char *chr_ptr;
	string output;
	stringstream oss;
	
	if (r < 16 ){
		oss << 0;
	}	
	oss << hex << r;
	
	if (g < 16 ){
		oss << 0;
	}
	oss << hex << g;
	
	if (b < 16 ){
		oss << 0;
	}
	oss << hex << b;
	
	output = oss.str();
	
	for (int i = 0; i < 6; i++){
		out[i] = output[i];
	}
	chr_ptr = &out;
	return chr_ptr;
}
	

float aci_to_rgb(int aci);

float aci_to_rgb(int aci)
 {
 	aci = abs(aci);			// hidden layers have negative color values
 	if (aci<10 || aci>249)	// values of these ranges are special colors
 	{
 		switch (aci)
 		{
 			case 1: return RGB(255,0,0);		// basic colors
 			case 2: return RGB(255,255,0);
 			case 3: return RGB(0,255,0);
 			case 4: return RGB(0,255,255);
 			case 5: return RGB(0,0,255);
 			case 6: return RGB(255,0,255);
 			case 7: return RGB(255,255,255);
 			case 8: return RGB(128,128,128);
 			case 9: return RGB(192,192,192);
 			case 250: return RGB(51,51,51);		// grey shades
 			case 251: return RGB(91,91,91);
 			case 252: return RGB(132,132,132);
 			case 253: return RGB(173,173,173);
 			case 254: return RGB(214,214,214);
 			case 255: return RGB(255,255,255);
 			case 256:							// "by layer"
 			// Here you should decide how to handle "by layer" logical color.
 			// Maybe it is a good idea to return a value like -1.
 			// The outer code will find what is the color of the layer which
 			// this entity belongs to.
 				return -1;
 		}
 	}
 	// for all the rest of ACI codes
 	float H,S,L,	R,G,B;
 	int remainder = aci % 10;					
 	H = 1.5f * (aci - remainder - 10);	// hue in range 0-360
 	S = ((aci % 2) ? 0.5f : 1.0f);		// odd colors have 50% of saturation, even - 100%
 	// set lighteness, the last digit of aci code stands for this
 	if (reminder == 0 || reminder == 1) L = 1.0f; 
 	if (reminder == 2 || reminder == 3) L = 0.8f;
 	if (reminder == 4 || reminder == 5) L = 0.6f;
 	if (reminder == 6 || reminder == 7) L = 0.5f;
 	if (reminder == 8 || reminder == 9) L = 0.3f;
 	// here we have H,S,L set already
 	// let's convert it to RGB, first without consideration of S and L
 	if (H<=120)
 	{
 		R = (120-H)/60;
 		G = H/60;
 		B = 0;
 	}
  	if (H>120 && H<=240)
 	{
 		R = 0;
 		G = (240-H)/60;
 		B = (H-120)/60;
 	}
 	if (H>240 && H<=360)
 	{
 		R = (H-240)/60;
 		G = 0;
 		B = (360-H)/60;
 	}
 	R = min(R, 1);
 	G = min(G, 1);
 	B = min(B, 1);
 	// influence of S and L
 	float max_value = max(R,max(G,B));
 	R = (max_value-S*(max_value-R)) * L * 255;
 	G = (max_value-S*(max_value-G)) * L * 255;
 	B = (max_value-S*(max_value-B)) * L * 255;
 	return RGB(R,G,B);
 }