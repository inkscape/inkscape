/** \file
 * SVG <animate> implementation.
 *
 * N.B. This file is currently just a stub file with no meaningful implementation.
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-animation.h"

# define log_set_attr(_classname, _key, _value) static_cast<void>(0)

/* Animation base class */

static void sp_animation_class_init(SPAnimationClass *klass);
static void sp_animation_init(SPAnimation *animation);

static void sp_animation_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_animation_release(SPObject *object);
static void sp_animation_set(SPObject *object, unsigned int key, gchar const *value);

G_DEFINE_TYPE(SPAnimation, sp_animation, SP_TYPE_OBJECT);

static void sp_animation_class_init(SPAnimationClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;

    sp_object_class->build = sp_animation_build;
    sp_object_class->release = sp_animation_release;
    sp_object_class->set = sp_animation_set;
}

static void sp_animation_init(SPAnimation * /*animation*/)
{
}


static void sp_animation_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if ((SP_OBJECT_CLASS(sp_animation_parent_class))->build)
        (SP_OBJECT_CLASS(sp_animation_parent_class))->build(object, document, repr);

    object->readAttr( "xlink:href" );
    object->readAttr( "attributeName" );
    object->readAttr( "attributeType" );
    object->readAttr( "begin" );
    object->readAttr( "dur" );
    object->readAttr( "end" );
    object->readAttr( "min" );
    object->readAttr( "max" );
    object->readAttr( "restart" );
    object->readAttr( "repeatCount" );
    object->readAttr( "repeatDur" );
    object->readAttr( "fill" );
}

static void sp_animation_release(SPObject */*object*/)
{
}

static void sp_animation_set(SPObject *object, unsigned int key, gchar const *value)
{
    log_set_attr("SPAnimation", key, value);

    if ((SP_OBJECT_CLASS(sp_animation_parent_class))->set)
        (SP_OBJECT_CLASS(sp_animation_parent_class))->set(object, key, value);
}

/* Interpolated animation base class */

static void sp_ianimation_class_init(SPIAnimationClass *klass);
static void sp_ianimation_init(SPIAnimation *animation);

static void sp_ianimation_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_ianimation_release(SPObject *object);
static void sp_ianimation_set(SPObject *object, unsigned int key, gchar const *value);

G_DEFINE_TYPE(SPIanimationClass, sp_ianimation, SP_TYPE_OBJECT);

static void sp_ianimation_class_init(SPIAnimationClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;

    sp_object_class->build = sp_ianimation_build;
    sp_object_class->release = sp_ianimation_release;
    sp_object_class->set = sp_ianimation_set;
}

static void sp_ianimation_init(SPIAnimation * /*animation*/)
{
}


static void sp_ianimation_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if ((SP_OBJECT_CLASS(sp_ianimation_parent_class))->build)
        (SP_OBJECT_CLASS(sp_ianimation_parent_class))->build(object, document, repr);

    object->readAttr( "calcMode" );
    object->readAttr( "values" );
    object->readAttr( "keyTimes" );
    object->readAttr( "keySplines" );
    object->readAttr( "from" );
    object->readAttr( "to" );
    object->readAttr( "by" );
    object->readAttr( "additive" );
    object->readAttr( "accumulate" );
}

static void sp_ianimation_release(SPObject * /*object*/)
{
}

static void sp_ianimation_set(SPObject *object, unsigned int key, gchar const *value)
{
    log_set_attr("SPIAnimation", key, value);

    if ((SP_OBJECT_CLASS(sp_ianimation_parent_class))->set)
        (SP_OBJECT_CLASS(sp_ianimation_parent_class))->set(object, key, value);
}

/* SVG <animate> */

static void sp_animate_class_init(SPAnimateClass *klass);
static void sp_animate_init(SPAnimate *animate);

static void sp_animate_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_animate_release(SPObject *object);
static void sp_animate_set(SPObject *object, unsigned int key, gchar const *value);

G_DEFINE_TYPE(SPAnimate, sp_animate, SP_TYPE_IANIMATION);

static void sp_animate_class_init(SPAnimateClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;

    sp_object_class->build = sp_animate_build;
    sp_object_class->release = sp_animate_release;
    sp_object_class->set = sp_animate_set;
}

static void sp_animate_init(SPAnimate * /*animate*/)
{
}


static void sp_animate_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if ((SP_OBJECT_CLASS(sp_animate_parent_class))->build)
        (SP_OBJECT_CLASS(sp_animate_parent_class))->build(object, document, repr);
}

static void sp_animate_release(SPObject * /*object*/)
{
}

static void sp_animate_set(SPObject *object, unsigned int key, gchar const *value)
{
    log_set_attr("SPAnimate", key, value);

    if ((SP_OBJECT_CLASS(sp_animate_parent_class))->set)
        (SP_OBJECT_CLASS(sp_animate_parent_class))->set(object, key, value);
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
