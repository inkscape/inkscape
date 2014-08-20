#ifndef SEEN_EGE_TAGS_H
#define SEEN_EGE_TAGS_H

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

#include <string>
#include <vector>
#include <map>

/*
 * Implements base tagging of http://create.freedesktop.org/wiki/ResourceTagging .
 */

// Note that this API is preliminary and subject to frequent change:

namespace ege
{

class Label
{
public:
    Label();
    Label(std::string const& lang, std::string const& value);
    ~Label();

    std::string lang;
    std::string value;
};

class Tag
{
public:
    Tag();
    Tag(std::string const& key);
    ~Tag();

    std::string key;
    std::vector<Label> labels;
};


/**
 * Contains a set of tags with unique keys, and with locale support.
 *
 */
class TagSet
{
public:
    TagSet();
    ~TagSet();

    std::string const & getLang() const;
    void setLang(std::string const& lang);

    /**
     * Adds or updates a tag.
     *
     * @return true if a tag was updated, false if it was added.
     */
    bool addTag(Tag const& tag);
    std::vector<Tag> const& getTags();

    int getCount( std::string const& key );
    void increment( std::string const& key );
    void decrement( std::string const& key );

private:

    std::string lang;
    std::vector<Tag> tags;
    std::map<std::string, int> counts;
};

} // namespace ege


#endif // SEEN_EGE_TAGS_H
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
