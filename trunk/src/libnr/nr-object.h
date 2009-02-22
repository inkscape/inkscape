#ifndef __NR_OBJECT_H__
#define __NR_OBJECT_H__

/*
 * RGBA display list system for inkscape
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   MenTaLguY <mental@rydia.net>
 *
 * This code is in public domain
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gtypes.h>
#include "gc-managed.h"
#include "gc-finalized.h"
#include "gc-anchored.h"

typedef guint32 NRType;

struct NRObject;
struct NRObjectClass;

#define NR_TYPE_OBJECT (nr_object_get_type ())
#define NR_OBJECT(o) (NR_CHECK_INSTANCE_CAST ((o), NR_TYPE_OBJECT, NRObject))
#define NR_IS_OBJECT(o) (NR_CHECK_INSTANCE_TYPE ((o), NR_TYPE_OBJECT))

#define NR_TYPE_ACTIVE_OBJECT (nr_active_object_get_type ())
#define NR_ACTIVE_OBJECT(o) (NR_CHECK_INSTANCE_CAST ((o), NR_TYPE_ACTIVE_OBJECT, NRActiveObject))
#define NR_IS_ACTIVE_OBJECT(o) (NR_CHECK_INSTANCE_TYPE ((o), NR_TYPE_ACTIVE_OBJECT))

#define nr_return_if_fail(expr) if (!(expr) && nr_emit_fail_warning (__FILE__, __LINE__, "?", #expr)) return
#define nr_return_val_if_fail(expr,val) if (!(expr) && nr_emit_fail_warning (__FILE__, __LINE__, "?", #expr)) return (val)

unsigned int nr_emit_fail_warning (const gchar *file, unsigned int line, const gchar *method, const gchar *expr);

#ifndef NR_DISABLE_CAST_CHECKS
#define NR_CHECK_INSTANCE_CAST(ip, tc, ct) ((ct *) nr_object_check_instance_cast (ip, tc))
#else
#define NR_CHECK_INSTANCE_CAST(ip, tc, ct) ((ct *) ip)
#endif

#define NR_CHECK_INSTANCE_TYPE(ip, tc) nr_object_check_instance_type (ip, tc)
#define NR_OBJECT_GET_CLASS(ip) (((NRObject *) ip)->klass)

NRType nr_type_is_a (NRType type, NRType test);

void const *nr_object_check_instance_cast(void const *ip, NRType tc);
unsigned int nr_object_check_instance_type(void const *ip, NRType tc);

NRType nr_object_register_type (NRType parent,
				gchar const *name,
				unsigned int csize,
				unsigned int isize,
				void (* cinit) (NRObjectClass *),
				void (* iinit) (NRObject *));

/* NRObject */

class NRObject : public Inkscape::GC::Managed<>,
                 public Inkscape::GC::Finalized,
                 public Inkscape::GC::Anchored
{
public:
	NRObjectClass *klass;

	static NRObject *alloc(NRType type);

	template <typename T>
	static void invoke_ctor(NRObject *object) {
		new (object) T();
	}

	/* these can go away eventually */
	NRObject *reference() {
		return Inkscape::GC::anchor(this);
	}
	NRObject *unreference() {
		Inkscape::GC::release(this);
		return NULL;
	}

protected:
	NRObject() {}

private:
	NRObject(NRObject const &); // no copy
	void operator=(NRObject const &); // no assign

	void *operator new(size_t size, void *placement) { (void)size; return placement; }
};

struct NRObjectClass {
	NRType type;
	NRObjectClass *parent;

	gchar *name;
	unsigned int csize;
	unsigned int isize;
	void (* cinit) (NRObjectClass *);
	void (* iinit) (NRObject *);
	void (* finalize) (NRObject *object);
	void (*cpp_ctor)(NRObject *object);
};

NRType nr_object_get_type (void);

/* Dynamic lifecycle */

inline NRObject *nr_object_new (NRType type) {
	return NRObject::alloc(type);
}

inline NRObject *nr_object_ref (NRObject *object) {
	return object->reference();
}
inline NRObject *nr_object_unref (NRObject *object) {
	return object->unreference();
}

/* NRActiveObject */

struct NRObjectEventVector {
	void (* dispose) (NRObject *object, void *data);
};

struct NRObjectListener {
	const NRObjectEventVector *vector;
	unsigned int size;
	void *data;
};

struct NRObjectCallbackBlock {
	unsigned int size;
	unsigned int length;
	NRObjectListener listeners[1];
};

struct NRActiveObject : public NRObject {
	NRActiveObject() : callbacks(NULL) {}
	NRObjectCallbackBlock *callbacks;
};

struct NRActiveObjectClass : public NRObjectClass {
};

NRType nr_active_object_get_type (void);

void nr_active_object_add_listener (NRActiveObject *object, const NRObjectEventVector *vector, unsigned int size, void *data);
void nr_active_object_remove_listener_by_data (NRActiveObject *object, void *data);

#endif

