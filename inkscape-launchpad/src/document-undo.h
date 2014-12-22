#ifndef SEEN_SP_DOCUMENT_UNDO_H
#define SEEN_SP_DOCUMENT_UNDO_H

namespace Glib {
    class ustring;
}

typedef struct _GObject GObject;

class SPDesktop;
class SPDocument;
struct InkscapeApplication;

namespace Inkscape {

class DocumentUndo
{
public:

    /**
     * Set undo sensitivity.
     *
     * \note
     *   Since undo sensitivity needs to be nested, setting undo sensitivity
     *   should be done like this:
     *\verbatim
     bool saved = DocumentUndo::getUndoSensitive(document);
     DocumentUndo::setUndoSensitive(document, false);
     ... do stuff ...
     DocumentUndo::setUndoSensitive(document, saved);  \endverbatim
    */
    static void setUndoSensitive(SPDocument *doc, bool sensitive);

    static bool getUndoSensitive(SPDocument const *document);

    static void clearUndo(SPDocument *document);

    static void clearRedo(SPDocument *document);

    static void done(SPDocument *document, unsigned int event_type, Glib::ustring const &event_description);

    static void maybeDone(SPDocument *document, const gchar *keyconst, unsigned int event_type, Glib::ustring const &event_description);

    static void resetKey(SPDocument *doc);

    static void cancel(SPDocument *document);

    static gboolean undo(SPDocument *document);

    static gboolean redo(SPDocument *document);
};

} // namespace Inkscape

#endif // SEEN_SP_DOCUMENT_UNDO_H

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
