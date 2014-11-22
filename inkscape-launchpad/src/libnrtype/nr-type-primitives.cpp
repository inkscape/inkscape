/**
 * Typeface and script library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

/* This should be enough for approximately 10000 fonts */
#define NR_DICTSIZE 2777

#include <cstdlib>
#include <string.h>
#include <glib.h>

#include "nr-type-primitives.h"

/**
 * An entry in a list of key->value pairs
 */
struct NRTDEntry {
    NRTDEntry *next;
    const gchar *key;
    void *val;
};

/**
 * Type Dictionary, consisting of size number of key-value entries
 */
struct NRTypeDict {
    unsigned int size;
    NRTDEntry **entries;
};

static NRTDEntry *nr_td_entry_new (void);

/**
 * Calls the destructor for each item in list
 */
void
nr_name_list_release (NRNameList *list)
{
    if (list->destructor) {
        list->destructor (list);
    }
}

void
nr_style_list_release (NRStyleList *list)
{
    if (list->destructor) {
        list->destructor (list);
    }
}

/**
 * Creates a new typeface dictionary of size NR_DICTSIZE
 * and initalizes all the entries to NULL
 */
NRTypeDict *
nr_type_dict_new (void)
{
    NRTypeDict *td;
    int i;

    td = g_new (NRTypeDict, 1);

    td->size = NR_DICTSIZE;
    td->entries = g_new (NRTDEntry *, td->size);
    for (i = 0; i < NR_DICTSIZE; i++) {
        td->entries[i] = NULL;
    }

    return td;
}

/**
 * Hashes a string and returns the int
 */
static unsigned int
nr_str_hash (const gchar *p)
{
    unsigned int h;

    h = *p;

    if (h != 0) {
        for (p += 1; *p; p++) h = (h << 5) - h + *p;
    }

    return h;
}

/**
 * Inserts a key/value into a typeface dictionary
 */
void
nr_type_dict_insert (NRTypeDict *td, const gchar *key, void *val)
{
    if (key) {
        NRTDEntry *tde;
        unsigned int hval;

        hval = nr_str_hash (key) % td->size;

        for (tde = td->entries[hval]; tde; tde = tde->next) {
            if (!strcmp (key, tde->key)) {
                tde->val = val;
                return;
            }
        }

        tde = nr_td_entry_new ();
        tde->next = td->entries[hval];
        tde->key = key;
        tde->val = val;
        td->entries[hval] = tde;
    }
}

/**
 * Looks up the given key from the typeface dictionary
 */
void *
nr_type_dict_lookup (NRTypeDict *td, const gchar *key)
{
    if (key) {
        NRTDEntry *tde;
        unsigned int hval;
        hval = nr_str_hash (key) % td->size;
        for (tde = td->entries[hval]; tde; tde = tde->next) {
            if (!strcmp (key, tde->key)) return tde->val;
        }
    }

    return NULL;
}

#define NR_TDE_BLOCK_SIZE 32

static NRTDEntry *nr_tde_free_list;

/**
 * Creates a new TDEntry
 */
static NRTDEntry *
nr_td_entry_new (void)
{
    NRTDEntry *tde;

    if (!nr_tde_free_list) {
        int i;
        nr_tde_free_list = g_new (NRTDEntry, NR_TDE_BLOCK_SIZE);
        for (i = 0; i < (NR_TDE_BLOCK_SIZE - 1); i++) {
            nr_tde_free_list[i].next = nr_tde_free_list + i + 1;
        }
        nr_tde_free_list[i].next = NULL;
    }

    tde = nr_tde_free_list;
    nr_tde_free_list = tde->next;

    return tde;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
