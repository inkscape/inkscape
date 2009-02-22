#ifndef SEEN_SP_CONN_END
#define SEEN_SP_CONN_END

#include <glib/gtypes.h>
#include <sigc++/connection.h>

#include "sp-use-reference.h"


class SPConnEnd {
public:
    SPConnEnd(SPObject *owner);

    SPUseReference ref;
    gchar *href;

    /** Change of href string (not a modification of the attributes of the referrent). */
    sigc::connection _changed_connection;

    /** Called when the attached object gets deleted. */
    sigc::connection _delete_connection;

    /** A sigc connection for transformed signal, used to do move compensation. */
    sigc::connection _transformed_connection;

    void setAttacherHref(gchar const *);

private:
    SPConnEnd(SPConnEnd const &);
    SPConnEnd &operator=(SPConnEnd const &);
};

void sp_conn_end_href_changed(SPObject *old_ref, SPObject *ref,
                              SPConnEnd *connEnd, SPPath *path, unsigned const handle_ix);
void sp_conn_adjust_invalid_path(SPPath *const path);
void sp_conn_adjust_path(SPPath *const path);
void sp_conn_end_detach(SPObject *const owner, unsigned const handle_ix);


#endif /* !SEEN_SP_CONN_END */

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
