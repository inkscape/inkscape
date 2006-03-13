#ifndef SVG_SVG_COLOR_H_SEEN
#define SVG_SVG_COLOR_H_SEEN

#include <glib/gtypes.h>

unsigned int sp_svg_read_color(gchar const *str, unsigned int dfl);
int sp_svg_write_color(char *buf, unsigned buflen, unsigned int color);


#endif /* !SVG_SVG_COLOR_H_SEEN */
