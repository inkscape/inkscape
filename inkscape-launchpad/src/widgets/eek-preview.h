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
 * The Original Code is Eek Preview Stuffs.
 *
 * The Initial Developer of the Original Code is
 * Jon A. Cruz.
 * Portions created by the Initial Developer are Copyright (C) 2005-2008
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

#ifndef SEEN_EEK_PREVIEW_H
#define SEEN_EEK_PREVIEW_H

#include <gtk/gtk.h>

/**
 * @file
 * Generic implementation of an object that can be shown by a preview.
 */

G_BEGIN_DECLS

#define EEK_PREVIEW_TYPE            (eek_preview_get_type())
#define EEK_PREVIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST( (obj), EEK_PREVIEW_TYPE, EekPreview))
#define EEK_PREVIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST( (klass), EEK_PREVIEW_TYPE, EekPreviewClass))
#define IS_EEK_PREVIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE( (obj), EEK_PREVIEW_TYPE))
#define IS_EEK_PREVIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE( (klass), EEK_PREVIEW_TYPE))
#define EEK_PREVIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS( (obj), EEK_PREVIEW_TYPE, EekPreviewClass))

typedef enum {
    PREVIEW_STYLE_ICON = 0,
    PREVIEW_STYLE_PREVIEW,
    PREVIEW_STYLE_NAME,
    PREVIEW_STYLE_BLURB,
    PREVIEW_STYLE_ICON_NAME,
    PREVIEW_STYLE_ICON_BLURB,
    PREVIEW_STYLE_PREVIEW_NAME,
    PREVIEW_STYLE_PREVIEW_BLURB
} PreviewStyle;

typedef enum {
    VIEW_TYPE_LIST = 0,
    VIEW_TYPE_GRID
} ViewType;

typedef enum {
    PREVIEW_SIZE_TINY = 0,
    PREVIEW_SIZE_SMALL,
    PREVIEW_SIZE_MEDIUM,
    PREVIEW_SIZE_BIG,
    PREVIEW_SIZE_BIGGER,
    PREVIEW_SIZE_HUGE
} PreviewSize;

typedef enum {
  PREVIEW_LINK_NONE = 0,
  PREVIEW_LINK_IN = 1,
  PREVIEW_LINK_OUT = 2,
  PREVIEW_LINK_OTHER = 4,
  PREVIEW_FILL = 8,
  PREVIEW_STROKE = 16,
  PREVIEW_LINK_ALL = 31
} LinkType;

typedef enum {
    BORDER_NONE = 0,
    BORDER_SOLID,
    BORDER_WIDE,
    BORDER_SOLID_LAST_ROW,
} BorderStyle;

typedef struct _EekPreview       EekPreview;
typedef struct _EekPreviewClass  EekPreviewClass;

struct _EekPreview
{
    GtkDrawingArea drawing;
};

struct _EekPreviewClass
{
    GtkDrawingAreaClass parent_class;

    void (*clicked) (EekPreview* splat);
};

GType      eek_preview_get_type(void) G_GNUC_CONST;
GtkWidget* eek_preview_new(void);

void eek_preview_set_details(EekPreview   *preview,
                             ViewType      view,
                             PreviewSize   size,
                             guint         ratio, 
                             guint         border);
void eek_preview_set_color( EekPreview* splat, int r, int g, int b );
void eek_preview_set_pixbuf( EekPreview* splat, GdkPixbuf* pixbuf );

void eek_preview_set_linked( EekPreview* splat, LinkType link );
LinkType eek_preview_get_linked( EekPreview* splat );

gboolean eek_preview_get_focus_on_click( EekPreview* preview );
void eek_preview_set_focus_on_click( EekPreview* preview, gboolean focus_on_click );

void eek_preview_set_size_mappings( guint count, GtkIconSize const* sizes );

G_END_DECLS

#endif /* SEEN_EEK_PREVIEW_H */

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
