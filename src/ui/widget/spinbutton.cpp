/*
 * Author:
 *   Johan B. C. Engelen
 *
 * Copyright (C) 2011 Author
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "spinbutton.h"

#include "unit-menu.h"
#include "unit-tracker.h"
#include "util/expression-evaluator.h"
#include "ui/tools/tool-base.h"

namespace Inkscape {
namespace UI {
namespace Widget {


void
SpinButton::connect_signals() {
    signal_input().connect(sigc::mem_fun(*this, &SpinButton::on_input));
    signal_focus_in_event().connect(sigc::mem_fun(*this, &SpinButton::on_my_focus_in_event));
    signal_key_press_event().connect(sigc::mem_fun(*this, &SpinButton::on_my_key_press_event));
};

int SpinButton::on_input(double* newvalue)
{
    try {
        Inkscape::Util::EvaluatorQuantity result;
        if (_unit_menu || _unit_tracker) {
            Unit const *unit = NULL;
            if (_unit_menu) {
                unit = _unit_menu->getUnit();
            } else {
                unit = _unit_tracker->getActiveUnit();
            }
            Inkscape::Util::ExpressionEvaluator eval = Inkscape::Util::ExpressionEvaluator(get_text().c_str(), unit);
            result = eval.evaluate();
            // check if output dimension corresponds to input unit
            if (result.dimension != (unit->isAbsolute() ? 1 : 0) ) {
                throw Inkscape::Util::EvaluatorException("Input dimensions do not match with parameter dimensions.","");
            }
        } else {
            Inkscape::Util::ExpressionEvaluator eval = Inkscape::Util::ExpressionEvaluator(get_text().c_str(), NULL);
            result = eval.evaluate();
        }

        *newvalue = result.value;
    }
    catch(Inkscape::Util::EvaluatorException &e) {
        g_message ("%s", e.what());

        return false;
    }

    return true;
}

bool SpinButton::on_my_focus_in_event(GdkEventFocus* /*event*/)
{
    _on_focus_in_value = get_value();
    return false; // do not consume the event
}

bool SpinButton::on_my_key_press_event(GdkEventKey* event)
{
    switch (Inkscape::UI::Tools::get_group0_keyval (event)) {
    case GDK_KEY_Escape:
        undo();
        return true; // I consumed the event
        break;
    case GDK_KEY_z:
    case GDK_KEY_Z:
        if (event->state & GDK_CONTROL_MASK) {
            undo();
            return true; // I consumed the event
        }
        break;
    default:
        break;
    }

    return false; // do not consume the event
}

void SpinButton::undo()
{
    set_value(_on_focus_in_value);
}


} // namespace Widget
} // namespace UI
} // namespace Inkscape

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
