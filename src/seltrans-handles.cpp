#define SP_SELTRANS_HANDLES_C

#include "seltrans-handles.h"


SPSelTransHandle const handles_scale[] = {
//anchor         cursor                   control  action                request                       x    y
 {GTK_ANCHOR_SE, GDK_TOP_LEFT_CORNER,     0,       sp_sel_trans_scale,   sp_sel_trans_scale_request,   0,   1},
 {GTK_ANCHOR_S,  GDK_TOP_SIDE,            3,       sp_sel_trans_stretch, sp_sel_trans_stretch_request, 0.5, 1},
 {GTK_ANCHOR_SW, GDK_TOP_RIGHT_CORNER,    1,       sp_sel_trans_scale,   sp_sel_trans_scale_request,   1,   1},
 {GTK_ANCHOR_W,  GDK_RIGHT_SIDE,          2,       sp_sel_trans_stretch, sp_sel_trans_stretch_request, 1,   0.5},
 {GTK_ANCHOR_NW, GDK_BOTTOM_RIGHT_CORNER, 0,       sp_sel_trans_scale,   sp_sel_trans_scale_request,   1,   0},
 {GTK_ANCHOR_N,  GDK_BOTTOM_SIDE,         3,       sp_sel_trans_stretch, sp_sel_trans_stretch_request, 0.5, 0},
 {GTK_ANCHOR_NE, GDK_BOTTOM_LEFT_CORNER,  1,       sp_sel_trans_scale,   sp_sel_trans_scale_request,   0,   0},
 {GTK_ANCHOR_E,  GDK_LEFT_SIDE,           2,       sp_sel_trans_stretch, sp_sel_trans_stretch_request, 0,   0.5}
};

SPSelTransHandle const handles_rotate[] = {
 {GTK_ANCHOR_SE, GDK_EXCHANGE,            4,       sp_sel_trans_rotate,  sp_sel_trans_rotate_request,  0,   1},
 {GTK_ANCHOR_S,  GDK_SB_H_DOUBLE_ARROW,   5,       sp_sel_trans_skew,    sp_sel_trans_skew_request,    0.5, 1},
 {GTK_ANCHOR_SW, GDK_EXCHANGE,            6,       sp_sel_trans_rotate,  sp_sel_trans_rotate_request,  1,   1},
 {GTK_ANCHOR_W,  GDK_SB_V_DOUBLE_ARROW,   7,       sp_sel_trans_skew,    sp_sel_trans_skew_request,    1,   0.5},
 {GTK_ANCHOR_NW, GDK_EXCHANGE,            8,       sp_sel_trans_rotate,  sp_sel_trans_rotate_request,  1,   0},
 {GTK_ANCHOR_N,  GDK_SB_H_DOUBLE_ARROW,   9,       sp_sel_trans_skew,    sp_sel_trans_skew_request,    0.5, 0},
 {GTK_ANCHOR_NE, GDK_EXCHANGE,            10,      sp_sel_trans_rotate,  sp_sel_trans_rotate_request,  0,   0},
 {GTK_ANCHOR_E,  GDK_SB_V_DOUBLE_ARROW,   11,      sp_sel_trans_skew,    sp_sel_trans_skew_request,    0,   0.5}
};

SPSelTransHandle const handle_center =
  {GTK_ANCHOR_CENTER, GDK_CROSSHAIR,      12,      sp_sel_trans_center,  sp_sel_trans_center_request,  0.5, 0.5};


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
