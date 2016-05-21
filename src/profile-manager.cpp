/*
 * Inkscape::ProfileManager - a view of a document's color profiles.
 *
 * Copyright 2007  Jon A. Cruz  <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <algorithm>

#include "profile-manager.h"
#include "document.h"
#include "color-profile.h"

#include <cstring>

namespace Inkscape {

ProfileManager::ProfileManager(SPDocument *document) :
    _doc(document),
    _knownProfiles()
{
    _resource_connection = _doc->connectResourcesChanged(  "iccprofile", sigc::mem_fun(*this, &ProfileManager::_resourcesChanged) );
}

ProfileManager::~ProfileManager()
{
    _resource_connection.disconnect();
    _doc = 0;
}

void ProfileManager::_resourcesChanged()
{
    std::vector<SPObject*> newList;
    if (_doc) {
        std::vector<SPObject *> current = _doc->getResourceList( "iccprofile" );
        newList = current;
    }
    sort( newList.begin(), newList.end() );

    std::vector<SPObject*> diff1;
    std::set_difference( _knownProfiles.begin(), _knownProfiles.end(), newList.begin(), newList.end(),
                         std::insert_iterator<std::vector<SPObject*> >(diff1, diff1.begin()) );

    std::vector<SPObject*> diff2;
    std::set_difference( newList.begin(), newList.end(), _knownProfiles.begin(), _knownProfiles.end(),
                         std::insert_iterator<std::vector<SPObject*> >(diff2, diff2.begin()) );

    if ( !diff1.empty() ) {
        for ( std::vector<SPObject*>::iterator it = diff1.begin(); it < diff1.end(); ++it ) {
            SPObject* tmp = *it;
            _knownProfiles.erase( remove(_knownProfiles.begin(), _knownProfiles.end(), tmp), _knownProfiles.end() );
            if ( includes(tmp) ) {
                _removeOne(tmp);
            }
        }
    }

    if ( !diff2.empty() ) {
        for ( std::vector<SPObject*>::iterator it = diff2.begin(); it < diff2.end(); ++it ) {
            SPObject* tmp = *it;
            _knownProfiles.push_back(tmp);
            _addOne(tmp);
        }
        sort( _knownProfiles.begin(), _knownProfiles.end() );
    }
}

ColorProfile* ProfileManager::find(gchar const* name)
{
    ColorProfile* match = 0;
    if ( name ) {
        unsigned int howMany = childCount(NULL);
        for ( unsigned int index = 0; index < howMany; index++ ) {
            SPObject *obj = nthChildOf(NULL, index);
            ColorProfile* prof = reinterpret_cast<ColorProfile*>(obj);
            if (prof && (prof->name && !strcmp(name, prof->name))) {
                match = prof;
                break;
            }
        }
    }
    return match;
}

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
