#ifndef INKSCAPE_UNIT_CONSTANTS_H
#define INKSCAPE_UNIT_CONSTANTS_H

// 72 points per inch divided by the SVG-recommended value of 90 pixels per inch for computer screen
// For now it is constant throughout Inkscape, later we may make it changeable.
// Ideally this should be the only place to change it, but this is not guaranteed (be careful!)
#define DEVICESCALE 0.8    

#define PT_PER_IN 72.0
#define PT_PER_PX DEVICESCALE
#define PT_PER_PC 12.0
#define PX_PER_PT (1/DEVICESCALE)
#define PX_PER_PC (PX_PER_PT * PT_PER_PC)
#define PX_PER_IN (PT_PER_IN / PT_PER_PX)
#define PC_PER_IN (PT_PER_IN / PT_PER_PC)
#define M_PER_IN 0.0254
#define M_PER_PX (M_PER_IN / PX_PER_IN)
#define CM_PER_IN 2.54
#define MM_PER_IN 25.4
#define MM_PER_MM 1.0
#define MM_PER_CM 10.0
#define MM_PER_M 1000.0
#define IN_PER_PT (1 / PT_PER_IN)
#define IN_PER_PX (1 / PX_PER_IN)
#define IN_PER_CM (1 / CM_PER_IN)
#define IN_PER_MM (1 / MM_PER_IN)
#define IN_PER_FT 12.0
#define FT_PER_IN (1 / IN_PER_FT)
#define PT_PER_CM (PT_PER_IN / CM_PER_IN)
#define PX_PER_CM (PX_PER_IN / CM_PER_IN)
#define M_PER_PT (M_PER_IN / PT_PER_IN)
#define PT_PER_M (PT_PER_IN / M_PER_IN)
#define PX_PER_M (PX_PER_IN / M_PER_IN)
#define CM_PER_PT (CM_PER_IN / PT_PER_IN)
#define CM_PER_PX (CM_PER_IN / PX_PER_IN)
#define MM_PER_PT (MM_PER_IN / PT_PER_IN)
#define PT_PER_MM (PT_PER_IN / MM_PER_IN)
#define PX_PER_MM (PX_PER_IN / MM_PER_IN)
#define MM_PER_PX (MM_PER_IN / PX_PER_IN)
#define PX_PER_FT (PX_PER_IN / FT_PER_IN)
#define PT_PER_PT 1.0
#define PC_PER_PC 1.0
#define IN_PER_IN 1.0
#define PX_PER_PX 1.0
#define FT_PER_FT 1.0

#endif /* !INKSCAPE_UNIT_CONSTANTS_H */
