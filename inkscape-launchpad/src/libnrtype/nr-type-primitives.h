#ifndef SEEN_NR_TYPE_PRIMITIVES_H
#define SEEN_NR_TYPE_PRIMITIVES_H

/**
 * Typeface and script library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   c++ port: Nathan Hurst <njh@mail.csse.monash.edu.au>
 *
 * This code is in public domain
 */

#include <glib.h>

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

#endif // !SEEN_NR_TYPE_PRIMITIVES_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
