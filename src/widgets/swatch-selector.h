#ifndef SEEN_SP_SWATCH_SELECTOR_H
#define SEEN_SP_SWATCH_SELECTOR_H

#include <gtkmm/box.h>
#include "ui/selected-color.h"

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
    void _grabbedCb();
    void _draggedCb();
    void _releasedCb();
    void _changedCb();

    SPGradientSelector *_gsel;
    Inkscape::UI::SelectedColor _selected_color;
    bool _updating_color;
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

