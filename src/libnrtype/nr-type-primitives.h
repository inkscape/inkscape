#ifndef __NR_TYPE_PRIMITIVES_H__
#define __NR_TYPE_PRIMITIVES_H__

/*
 * Typeface and script library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   g++ port: Nathan Hurst <njh@mail.csse.monash.edu.au>
 *
 * This code is in public domain
 */

#include <glib/gtypes.h>

struct NRNameList;
struct NRStyleList;
struct NRTypeDict;

typedef void (* NRNameListDestructor) (NRNameList *list);
typedef void (* NRStyleListDestructor) (NRStyleList *list);

struct NRNameList {
	guint length;
      guchar **names;
      guchar **families;
	NRNameListDestructor destructor;
};

struct NRStyleRecord {
	const char *name;
      const char *descr;
};

struct NRStyleList {
	guint length;
      NRStyleRecord *records;
	NRStyleListDestructor destructor;
};

void nr_name_list_release (NRNameList *list);
void nr_style_list_release (NRStyleList *list);

NRTypeDict *nr_type_dict_new (void);

void nr_type_dict_insert (NRTypeDict *td, const gchar *key, void *val);

void *nr_type_dict_lookup (NRTypeDict *td, const gchar *key);

#endif
