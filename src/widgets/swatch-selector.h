#ifndef SEEN_SP_SWATCH_SELECTOR_H
#define SEEN_SP_SWATCH_SELECTOR_H

#include <gtkmm/box.h>

class SPDocument;
class SPGradient;
struct SPColorSelector;
struct SPGradientSelector;

namespace Inkscape
{
namespace Widgets
{

class SwatchSelector : public Gtk::VBox
{
public:
    SwatchSelector();
    virtual ~SwatchSelector();

    void connectGrabbedHandler( GCallback handler, void *data );
    void connectDraggedHandler( GCallback handler, void *data );
    void connectReleasedHandler( GCallback handler, void *data );
    void connectchangedHandler( GCallback handler, void *data );

    void setVector(SPDocument *doc, SPGradient *vector);

    SPGradientSelector *getGradientSelector();

private:
    static void _grabbedCb(SPColorSelector *csel, void *data);
    static void _draggedCb(SPColorSelector *csel, void *data);
    static void _releasedCb(SPColorSelector *csel, void *data);
    static void _changedCb(SPColorSelector *csel, void *data);

    SPGradientSelector *_gsel;
    SPColorSelector *_csel;
};


} // namespace Widgets
} // namespace Inkscape

#endif // SEEN_SP_SWATCH_SELECTOR_H

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

