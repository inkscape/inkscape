/*
 * Compatible key defines for earlier GTK+.
 *
 *
 * Authors:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 20012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef COMPAT_KEY_SYMS_H_SEEN
#define COMPAT_KEY_SYMS_H_SEEN

#if !GTK_CHECK_VERSION(2,22,0)

#define GDK_KEY_Up 0xff52
#define GDK_KEY_KP_Up 0xff97
#define GDK_KEY_Page_Up 0xff55
#define GDK_KEY_KP_Page_Up 0xff9a
#define GDK_KEY_Down 0xff54
#define GDK_KEY_KP_Down 0xff99
#define GDK_KEY_Page_Down 0xff56
#define GDK_KEY_KP_Page_Down 0xff9b
#define GDK_KEY_Left 0xff51
#define GDK_KEY_KP_Left 0xff96
#define GDK_KEY_Right 0xff53
#define GDK_KEY_KP_Right 0xff98
#define GDK_KEY_Home 0xff50
#define GDK_KEY_KP_Home 0xff95
#define GDK_KEY_End 0xff57
#define GDK_KEY_KP_End 0xff9c
#define GDK_KEY_a 0x061
#define GDK_KEY_A 0x041
#define GDK_KEY_b 0x062
#define GDK_KEY_B 0x042
#define GDK_KEY_c 0x063
#define GDK_KEY_C 0x043
#define GDK_KEY_d 0x064
#define GDK_KEY_D 0x044
#define GDK_KEY_g 0x067
#define GDK_KEY_G 0x047
#define GDK_KEY_h 0x068
#define GDK_KEY_H 0x048
#define GDK_KEY_i 0x069
#define GDK_KEY_I 0x049
#define GDK_KEY_j 0x06a
#define GDK_KEY_J 0x04a
#define GDK_KEY_k 0x06b
#define GDK_KEY_K 0x04b
#define GDK_KEY_l 0x06c
#define GDK_KEY_L 0x04c
#define GDK_KEY_M 0x04d
#define GDK_KEY_m 0x06d

#define GDK_KEY_P 0x050
#define GDK_KEY_p 0x070
#define GDK_KEY_q 0x071
#define GDK_KEY_Q 0x051
#define GDK_KEY_r 0x072
#define GDK_KEY_R 0x052
#define GDK_KEY_s 0x073
#define GDK_KEY_S 0x053
#define GDK_KEY_u 0x075
#define GDK_KEY_U 0x055
#define GDK_KEY_v 0x076
#define GDK_KEY_V 0x056
#define GDK_KEY_w 0x077
#define GDK_KEY_W 0x057
#define GDK_KEY_x 0x078
#define GDK_KEY_X 0x058
#define GDK_KEY_y 0x079
#define GDK_KEY_Y 0x059
#define GDK_KEY_z 0x07a
#define GDK_KEY_Z 0x05a
#define GDK_KEY_Escape 0xff1b
#define GDK_KEY_Control_L 0xffe3
#define GDK_KEY_Control_R 0xffe4
#define GDK_KEY_Alt_L 0xffe9
#define GDK_KEY_Alt_R 0xffea
#define GDK_KEY_Shift_L 0xffe1
#define GDK_KEY_Shift_R 0xffe2
#define GDK_KEY_Meta_L 0xffe7
#define GDK_KEY_Meta_R 0xffe8
#define GDK_KEY_KP_Add 0xffab
#define GDK_KEY_KP_Subtract 0xffad
#define GDK_KEY_KP_0 0xffb0
#define GDK_KEY_KP_1 0xffb1
#define GDK_KEY_KP_2 0xffb2
#define GDK_KEY_KP_3 0xffb3
#define GDK_KEY_KP_4 0xffb4
#define GDK_KEY_KP_5 0xffb5
#define GDK_KEY_KP_6 0xffb6
#define GDK_KEY_KP_7 0xffb7
#define GDK_KEY_KP_8 0xffb8
#define GDK_KEY_KP_9 0xffb9
#define GDK_KEY_F1 0xffbe
#define GDK_KEY_F2 0xffbf
#define GDK_KEY_F3 0xffc0
#define GDK_KEY_F4 0xffc1
#define GDK_KEY_F5 0xffc2
#define GDK_KEY_F6 0xffc3
#define GDK_KEY_F7 0xffc4
#define GDK_KEY_F8 0xffc5
#define GDK_KEY_F9 0xffc6
#define GDK_KEY_F10 0xffc7
#define GDK_KEY_F11 0xffc8
#define GDK_KEY_Insert 0xff63
#define GDK_KEY_KP_Insert 0xff9e
#define GDK_KEY_Delete 0xffff
#define GDK_KEY_KP_Delete 0xff9f
#define GDK_KEY_BackSpace 0xff08
#define GDK_KEY_Return 0xff0d
#define GDK_KEY_KP_Enter 0xff8d
#define GDK_KEY_space 0x020
#define GDK_KEY_KP_Space 0xff80
#define GDK_KEY_Tab 0xff09
#define GDK_KEY_ISO_Left_Tab 0xfe20
#define GDK_KEY_bracketleft 0x05b
#define GDK_KEY_bracketright 0x05d
#define GDK_KEY_parenright 0x029
#define GDK_KEY_parenleft 0x028
#define GDK_KEY_braceleft 0x07b
#define GDK_KEY_braceright 0x07d
#define GDK_KEY_less 0x03c
#define GDK_KEY_greater 0x03e
#define GDK_KEY_comma 0x02c
#define GDK_KEY_period 0x02e
#define GDK_KEY_0 0x030
#define GDK_KEY_1 0x031
#define GDK_KEY_2 0x032
#define GDK_KEY_3 0x033
#define GDK_KEY_4 0x034
#define GDK_KEY_5 0x035
#define GDK_KEY_6 0x036
#define GDK_KEY_7 0x037
#define GDK_KEY_8 0x038
#define GDK_KEY_9 0x039

#define GDK_KEY_VoidSymbol 0xffffff

#endif // !GTK_CHECK_VERSION(2,22,0)

#endif // COMPAT_KEY_SYMS_H_SEEN
