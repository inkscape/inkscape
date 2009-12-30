#ifndef __SP_SPRAY_CONTEXT_H__
#define __SP_SPRAY_CONTEXT_H__

/*
 * Spray Tool
 *
 * Authors:
 *   Pierre-Antoine MARC
 *   Pierre CACLIN
 *   Aurel-Aimé MARMION   
 *   Julien LERAY
 *   Benoît LAVORATA
 *   Vincent MONTAGNE
 *   Pierre BARBRY-BLOT
 *
 * Copyright (C) 2009 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "event-context.h"
#include <display/display-forward.h>
#include <libnr/nr-point.h>
//#include "ui/widget/spray-option.h"
#include "ui/dialog/dialog.h"

#define SP_TYPE_SPRAY_CONTEXT (sp_spray_context_get_type())
#define SP_SPRAY_CONTEXT(o) (GTK_CHECK_CAST((o), SP_TYPE_SPRAY_CONTEXT, SPSprayContext))
#define SP_SPRAY_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_CAST((k), SP_TYPE_SPRAY_CONTEXT, SPSprayContextClass))
#define SP_IS_SPRAY_CONTEXT(o) (GTK_CHECK_TYPE((o), SP_TYPE_SPRAY_CONTEXT))
#define SP_IS_SPRAY_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_TYPE((k), SP_TYPE_SPRAY_CONTEXT))

class SPSprayContext;
class SPSprayContextClass;

namespace Inkscape {
  namespace UI {
      namespace Dialog {
          class Dialog;
      }
  }
}


#define SAMPLING_SIZE 8        /* fixme: ?? */

#define TC_MIN_PRESSURE      0.0
#define TC_MAX_PRESSURE      1.0
#define TC_DEFAULT_PRESSURE  0.35

enum {
    SPRAY_MODE_COPY,
    SPRAY_MODE_CLONE,
    SPRAY_MODE_SINGLE_PATH,    
    SPRAY_OPTION,
};

struct SPSprayContext
{
    SPEventContext event_context;
    //Inkscape::UI::Dialog::Dialog *dialog_option;//Attribut de type SprayOptionClass, localisé dans scr/ui/dialog    
    /* extended input data */
    gdouble pressure;

    /* attributes */
    guint dragging : 1;           /* mouse state: mouse is dragging */
    guint usepressure : 1;
    guint usetilt : 1;
    bool usetext ;

    double width;
    double ratio;
    double tilt;
    double rotation_variation;
    double force;
    double population;
    double scale_variation;
    double scale;
    double mean;
    double standard_deviation;
   
    gint distrib;

    gint mode;

    Inkscape::MessageContext *_message_context;

    bool is_drawing;

    bool is_dilating;
    bool has_dilated;
    Geom::Point last_push;
    SPCanvasItem *dilate_area;

    bool do_h;
    bool do_s;
    bool do_l;
    bool do_o;

    sigc::connection style_set_connection;
};

struct SPSprayContextClass
{
    SPEventContextClass parent_class;
};

GtkType sp_spray_context_get_type(void);


#endif

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

