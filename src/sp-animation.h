#ifndef __SP_ANIMATION_H__
#define __SP_ANIMATION_H__

/*
 * SVG <animate> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object.h"



/* Animation base class */

#define SP_TYPE_ANIMATION (sp_animation_get_type ())
#define SP_ANIMATION(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_ANIMATION, SPAnimation))
#define SP_IS_ANIMATION(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_ANIMATION))

class SPAnimation;
class SPAnimationClass;

struct SPAnimation : public SPObject {
};

struct SPAnimationClass {
	SPObjectClass parent_class;
};

GType sp_animation_get_type (void);

/* Interpolated animation base class */

#define SP_TYPE_IANIMATION (sp_ianimation_get_type ())
#define SP_IANIMATION(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_IANIMATION, SPIAnimation))
#define SP_IS_IANIMATION(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_IANIMATION))

class SPIAnimation;
class SPIAnimationClass;

struct SPIAnimation : public SPAnimation {
};

struct SPIAnimationClass {
	SPAnimationClass parent_class;
};

GType sp_ianimation_get_type (void);

/* SVG <animate> */

#define SP_TYPE_ANIMATE (sp_animate_get_type ())
#define SP_ANIMATE(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_ANIMATE, SPAnimate))
#define SP_IS_ANIMATE(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_ANIMATE))

class SPAnimate;
class SPAnimateClass;

struct SPAnimate : public SPIAnimation {
};

struct SPAnimateClass {
	SPIAnimationClass parent_class;
};

GType sp_animate_get_type (void);



#endif
