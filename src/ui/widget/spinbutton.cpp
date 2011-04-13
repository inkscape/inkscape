/**
 * \brief SpinButton widget, that allows entry of both '.' and ',' for the decimal, even when in numeric mode.
 */
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
#include "util/expression-evaluator.h"

namespace Inkscape {
namespace UI {
namespace Widget {

/**
 * This callback function should try to convert the entered text to a number and write it to newvalue.
 * It calls a method to evaluate the (potential) mathematical expression.
 *
 * @retval false No conversion done, continue with default handler.
 * @retval true  Conversion successful, don't call default handler. 
 */
int
SpinButton::on_input(double* newvalue)
{
    try {
        Inkscape::Util::GimpEevlQuantity result = Inkscape::Util::gimp_eevl_evaluate (get_text().c_str(), _unit_menu ? &_unit_menu->getUnit() : NULL);
        // check if output dimension corresponds to input unit
        if (_unit_menu && result.dimension != (_unit_menu->getUnit().isAbsolute() ? 1 : 0) ) {
            throw Inkscape::Util::EvaluatorException("Input dimensions do not match with parameter dimensions.","");
        }

        *newvalue = result.value;
    }
    catch(Inkscape::Util::EvaluatorException &e) {
        g_message ("%s", e.what());

        return false;
    }

    return true;
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape

/* 
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
