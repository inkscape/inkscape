/** @file
 * @brief EEK color definition
 */
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
 * The Original Code is Eek Color Definition.
 *
 * The Initial Developer of the Original Code is
 * Jon A. Cruz.
 * Portions created by the Initial Developer are Copyright (C) 2006
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

#ifndef SEEN_EEK_COLOR_DEF_H
#define SEEN_EEK_COLOR_DEF_H

#include <string>
#include <vector>

namespace eek
{

typedef void (*ColorCallback)( void* data );


class ColorDef
{
public:
    ColorDef();
    ColorDef( unsigned int r, unsigned int g, unsigned int b, const std::string& description );
    virtual ~ColorDef();

    ColorDef( ColorDef const &other );
    virtual ColorDef& operator=( ColorDef const &other );

    void setRGB( unsigned int r, unsigned int g, unsigned int b );
    unsigned int getR() const { return r; }
    unsigned int getG() const { return g; }
    unsigned int getB() const { return b; }

    void addCallback( ColorCallback cb, void* data );
    void removeCallback( ColorCallback cb, void* data );

    bool isEditable() const { return editable; }
    void setEditable( bool edit ) { editable = edit; }

    std::string descr;

protected:
    unsigned int r;
    unsigned int g;
    unsigned int b;
    bool none;
    bool editable;

private:
    class HookData;

    std::vector<HookData*> _listeners;
};


} // namespace eek

#endif // SEEN_EEK_COLOR_DEF_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
