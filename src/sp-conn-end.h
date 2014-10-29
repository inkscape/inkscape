#ifndef SEEN_SP_CONN_END
#define SEEN_SP_CONN_END

#include <cstddef>
#include <sigc++/connection.h>

#include "sp-use-reference.h"
#include "conn-avoid-ref.h"

class SPPath;

class SPConnEnd {
public:
    SPConnEnd(SPObject *owner);

    SPUseReference ref;
    char *href;

    /** Change of href string (not a modification of the attributes of the referrent). */
    sigc::connection _changed_connection;

    /** Called when the attached object gets deleted. */
    sigc::connection _delete_connection;

    /** A sigc connection for transformed signal, used to do move compensation. */
    sigc::connection _transformed_connection;

    /** A sigc connection for owning group transformed, used to do move compensation. */
    sigc::connection _group_connection;

    void setAttacherHref(char const * value, SPPath * unused);
    //void setAttacherEndpoint(char const *, SPPath *); // not defined


private:
    SPConnEnd(SPConnEnd const &); // no copy
    SPConnEnd &operator=(SPConnEnd const &); // no assign
};

void sp_conn_end_href_changed(SPObject *old_ref, SPObject *ref,
                              SPConnEnd *connEnd, SPPath *path, unsigned const handle_ix);
void sp_conn_reroute_path(SPPath *const path);
void sp_conn_reroute_path_immediate(SPPath *const path);
void sp_conn_redraw_path(SPPath *const path);
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
