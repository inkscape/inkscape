/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is EGE Tagging Support.
 *
 * The Initial Developer of the Original Code is
 * Jon A. Cruz.
 * Portions created by the Initial Developer are Copyright (C) 2009
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#if HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#if HAVE_LIBINTL_H
#include <libintl.h>
#endif // HAVE_LIBINTL_H

#if !defined(_)
#define _(s) gettext(s)
#endif // !defined(_)

#include <set>
#include <algorithm>
#include <functional>

#include "ege-tags.h"

#include <glib.h>

namespace ege
{

Label::Label(std::string const& lang, std::string const& value) :
    lang(lang),
    value(value)
{
}

Label::~Label()
{
}

// =========================================================================

Tag::~Tag()
{
}

Tag::Tag(std::string const& key) :
    key(key)
{
}

// =========================================================================

TagSet::TagSet() :
    lang(),
    tags(),
    counts()
{
}

TagSet::~TagSet()
{
}

void TagSet::setLang(std::string const& lang)
{
    if (lang != this->lang) {
        this->lang = lang;
    }
}


struct sameLang : public std::binary_function<Label, Label, bool> {
    bool operator()(Label const& x, Label const& y) const { return (x.lang == y.lang); }
};


bool TagSet::addTag(Tag const& tag)
{
    bool present = false;

    for ( std::vector<Tag>::iterator it = tags.begin(); (it != tags.end()) && !present; ++it ) {
        if (tag.key == it->key) {
            present = true;

            for ( std::vector<Label>::const_iterator it2 = tag.labels.begin(); it2 != tag.labels.end(); ++it2 ) {
                std::vector<Label>::iterator itOld = std::find_if( it->labels.begin(), it->labels.end(), std::bind2nd(sameLang(), *it2) );
                if (itOld != it->labels.end()) {
                    itOld->value = it2->value;
                } else {
                    it->labels.push_back(*it2);
                }
            }
        }
    }

    if (!present) {
        tags.push_back(tag);
        counts[tag.key] = 0;
    }

    return present;
}


std::vector<Tag> const& TagSet::getTags()
{
    return tags;
}

int TagSet::getCount( std::string const& key )
{
    int count = 0;
    if ( counts.find(key) != counts.end() ) {
        count = counts[key];
    }
    return count;
}

void TagSet::increment( std::string const& key )
{
    if ( counts.find(key) != counts.end() ) {
        counts[key]++;
    } else {
        Tag tag(key);
        tags.push_back(tag);
        counts[key] = 1;
    }
}

void TagSet::decrement( std::string const& key )
{
    if ( counts.find(key) != counts.end() ) {
        counts[key]--;
    }
}

} // namespace ege

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
