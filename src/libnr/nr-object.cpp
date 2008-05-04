#define __NR_OBJECT_C__

/*
 * RGBA display list system for inkscape
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *
 * This code is in public domain
 */

#include <string.h>
#include <stdio.h>

#include <typeinfo>

#include <glib/gmem.h>
#include <libnr/nr-macros.h>

#include "nr-object.h"
#include "debug/event-tracker.h"
#include "debug/simple-event.h"
#include "util/share.h"
#include "util/format.h"

unsigned int nr_emit_fail_warning(const gchar *file, unsigned int line, const gchar *method, const gchar *expr)
{
    fprintf (stderr, "File %s line %d (%s): Assertion %s failed\n", file, line, method, expr);
    return 1;
}

/* NRObject */

static NRObjectClass **classes = NULL;
static unsigned int classes_len = 0;
static unsigned int classes_size = 0;

NRType nr_type_is_a(NRType type, NRType test)
{
    nr_return_val_if_fail(type < classes_len, FALSE);
    nr_return_val_if_fail(test < classes_len, FALSE);

    NRObjectClass *c = classes[type];

    while (c) {
	if (c->type == test) {
	    return TRUE;
	}
	c = c->parent;
    }

    return FALSE;
}

void const *nr_object_check_instance_cast(void const *ip, NRType tc)
{
    nr_return_val_if_fail(ip != NULL, NULL);
    nr_return_val_if_fail(nr_type_is_a(((NRObject const *) ip)->klass->type, tc), ip);
    return ip;
}

unsigned int nr_object_check_instance_type(void const *ip, NRType tc)
{
    if (ip == NULL) {
	return FALSE;
    }
    
    return nr_type_is_a(((NRObject const *) ip)->klass->type, tc);
}

NRType nr_object_register_type(NRType parent,
			       gchar const *name,
			       unsigned int csize,
			       unsigned int isize,
			       void (* cinit) (NRObjectClass *),
			       void (* iinit) (NRObject *))
{
    if (classes_len >= classes_size) {
	classes_size += 32;
	classes = g_renew (NRObjectClass *, classes, classes_size);
	if (classes_len == 0) {
	    classes[0] = NULL;
	    classes_len = 1;
	}
    }

    NRType const type = classes_len;
    classes_len += 1;

    classes[type] = (NRObjectClass*) new char[csize];
    NRObjectClass *c = classes[type];

    /* FIXME: is this necessary? */
    memset(c, 0, csize);

    if (classes[parent]) {
	memcpy(c, classes[parent], classes[parent]->csize);
    }

    c->type = type;
    c->parent = classes[parent];
    c->name = strdup(name);
    c->csize = csize;
    c->isize = isize;
    c->cinit = cinit;
    c->iinit = iinit;
    
    c->cinit(c);
    
    return type;
}

static void nr_object_class_init (NRObjectClass *klass);
static void nr_object_init (NRObject *object);
static void nr_object_finalize (NRObject *object);

NRType nr_object_get_type()
{
    static NRType type = 0;

    if (!type) {
	type = nr_object_register_type (0,
					"NRObject",
					sizeof (NRObjectClass),
					sizeof (NRObject),
					(void (*) (NRObjectClass *)) nr_object_class_init,
					(void (*) (NRObject *)) nr_object_init);
    }
    
    return type;
}

static void nr_object_class_init(NRObjectClass *c)
{
    c->finalize = nr_object_finalize;
    c->cpp_ctor = NRObject::invoke_ctor<NRObject>;
}

static void nr_object_init (NRObject */*object*/)
{
}

static void nr_object_finalize (NRObject */*object*/)
{
}

/* Dynamic lifecycle */

static void nr_class_tree_object_invoke_init(NRObjectClass *c, NRObject *object)
{
    if (c->parent) {
	nr_class_tree_object_invoke_init(c->parent, object);
    }
    c->iinit (object);
}

namespace {

namespace Debug = Inkscape::Debug;
namespace Util = Inkscape::Util;

typedef Debug::SimpleEvent<Debug::Event::FINALIZERS> BaseFinalizerEvent;

class FinalizerEvent : public BaseFinalizerEvent {
public:
    FinalizerEvent(NRObject *object)
    : BaseFinalizerEvent(Util::share_static_string("nr-object-finalizer"))
    {
        _addProperty("object", Util::format("%p", object));
        _addProperty("class", Util::share_static_string(typeid(*object).name()));
    }
};

void finalize_object(void *base, void *)
{
    NRObject *object = reinterpret_cast<NRObject *>(base);
    Debug::EventTracker<FinalizerEvent> tracker(object);
    object->klass->finalize(object);
    object->~NRObject();
}

}

NRObject *NRObject::alloc(NRType type)
{
    nr_return_val_if_fail (type < classes_len, NULL);

    NRObjectClass *c = classes[type];

    if ( c->parent && c->cpp_ctor == c->parent->cpp_ctor ) {
	g_error("Cannot instantiate NRObject class %s which has not registered a C++ constructor\n", c->name);
    }

    NRObject *object = reinterpret_cast<NRObject *>(
        ::operator new(c->isize, Inkscape::GC::SCANNED, Inkscape::GC::AUTO,
                       &finalize_object, NULL)
    );
    memset(object, 0xf0, c->isize);

    object->klass = c;
    c->cpp_ctor(object);
    nr_class_tree_object_invoke_init (c, object);

    return object;
}

/* NRActiveObject */

static void nr_active_object_class_init(NRActiveObjectClass *c);
static void nr_active_object_init(NRActiveObject *object);
static void nr_active_object_finalize(NRObject *object);

static NRObjectClass *parent_class;

NRType nr_active_object_get_type()
{
    static NRType type = 0;
    if (!type) {
	type = nr_object_register_type (NR_TYPE_OBJECT,
					"NRActiveObject",
					sizeof (NRActiveObjectClass),
					sizeof (NRActiveObject),
					(void (*) (NRObjectClass *)) nr_active_object_class_init,
					(void (*) (NRObject *)) nr_active_object_init);
    }
    return type;
}

static void nr_active_object_class_init(NRActiveObjectClass *c)
{
    NRObjectClass *object_class = (NRObjectClass *) c;

    parent_class = object_class->parent;

    object_class->finalize = nr_active_object_finalize;
    object_class->cpp_ctor = NRObject::invoke_ctor<NRActiveObject>;
}

static void nr_active_object_init(NRActiveObject */*object*/)
{
}

static void nr_active_object_finalize(NRObject *object)
{
    NRActiveObject *aobject = (NRActiveObject *) object;

    if (aobject->callbacks) {
	for (unsigned int i = 0; i < aobject->callbacks->length; i++) {
	    NRObjectListener *listener = aobject->callbacks->listeners + i;
	    if ( listener->vector->dispose ) {
		listener->vector->dispose(object, listener->data);
	    }
	}
	g_free (aobject->callbacks);
    }

    ((NRObjectClass *) (parent_class))->finalize(object);
}

void nr_active_object_add_listener(NRActiveObject *object,
				   const NRObjectEventVector *vector,
				   unsigned int size,
				   void *data)
{
    if (!object->callbacks) {
	object->callbacks = (NRObjectCallbackBlock*)g_malloc(sizeof(NRObjectCallbackBlock));
	object->callbacks->size = 1;
	object->callbacks->length = 0;
    }
    
    if (object->callbacks->length >= object->callbacks->size) {
	int newsize = object->callbacks->size << 1;
	object->callbacks = (NRObjectCallbackBlock *)
	    g_realloc(object->callbacks, sizeof(NRObjectCallbackBlock) + (newsize - 1) * sizeof (NRObjectListener));
	object->callbacks->size = newsize;
    }
    
    NRObjectListener *listener = object->callbacks->listeners + object->callbacks->length;
    listener->vector = vector;
    listener->size = size;
    listener->data = data;
    object->callbacks->length += 1;
}

void nr_active_object_remove_listener_by_data(NRActiveObject *object, void *data)
{
    if (object->callbacks == NULL) {
	return;
    }
    
    for (unsigned i = 0; i < object->callbacks->length; i++) {
	NRObjectListener *listener = object->callbacks->listeners + i;
	if ( listener->data == data ) {
	    object->callbacks->length -= 1;
	    if ( object->callbacks->length < 1 ) {
		g_free(object->callbacks);
		object->callbacks = NULL;
	    } else if ( object->callbacks->length != i ) {
		*listener = object->callbacks->listeners[object->callbacks->length];
	    }
	    return;
	}
    }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
