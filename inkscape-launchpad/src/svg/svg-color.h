#ifndef SVG_SVG_COLOR_H_SEEN
#define SVG_SVG_COLOR_H_SEEN

typedef unsigned int guint32;
struct SVGICCColor;

guint32 sp_svg_read_color(char const *str, unsigned int dfl);
guint32 sp_svg_read_color(char const *str, char const **end_ptr, guint32 def);
void sp_svg_write_color(char *buf, unsigned int buflen, unsigned int rgba32);

bool sp_svg_read_icc_color( char const *str, char const **end_ptr, SVGICCColor* dest );
bool sp_svg_read_icc_color( char const *str, SVGICCColor* dest );
void icc_color_to_sRGB(SVGICCColor* dest, unsigned char* r, unsigned char* g, unsigned char* b);

#endif /* !SVG_SVG_COLOR_H_SEEN */
