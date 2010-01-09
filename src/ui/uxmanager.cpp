/** \file
 * Desktop widget implementation
 */
/* Authors:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2010 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "uxmanager.h"
#include "util/ege-tags.h"
#include "widgets/toolbox.h"

namespace Inkscape {
namespace UI {

UXManager* instance = 0;

UXManager* UXManager::getInstance()
{
    if (!instance) {
        instance = new UXManager();
    }
    return instance;
}


UXManager::UXManager()
{
    ege::TagSet tags;
    tags.setLang("en");

    tags.addTag(ege::Tag("General"));
    tags.addTag(ege::Tag("Icons"));
}

UXManager::~UXManager()
{
}


void UXManager::connectToDesktop( std::vector<GtkWidget *> const & toolboxes, SPDesktop *desktop )
{
    for (std::vector<GtkWidget*>::const_iterator it = toolboxes.begin(); it != toolboxes.end(); ++it ) {
        ToolboxFactory::setToolboxDesktop( *it, desktop );
    }
}


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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
