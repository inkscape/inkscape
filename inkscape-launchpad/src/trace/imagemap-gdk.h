#ifndef __GRAYMAP_GDK_H__
#define __GRAYMAP_GDK_H__

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#include "imagemap.h"

#include <gdk-pixbuf/gdk-pixbuf.h>

/*#########################################################################
### I M A G E    M A P --- GDK
#########################################################################*/



#ifdef __cplusplus
extern "C" {
#endif

GrayMap *gdkPixbufToGrayMap(GdkPixbuf *buf);
GdkPixbuf *grayMapToGdkPixbuf(GrayMap *grayMap);
PackedPixelMap *gdkPixbufToPackedPixelMap(GdkPixbuf *buf);
RgbMap *gdkPixbufToRgbMap(GdkPixbuf *buf);
GdkPixbuf *indexedMapToGdkPixbuf(IndexedMap *iMap);


#ifdef __cplusplus
}
#endif


#endif /* __GRAYMAP_GDK_H__ */

/*#########################################################################
### E N D    O F    F I L E
#########################################################################*/
