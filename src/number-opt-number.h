#ifndef SEEN_NUMBER_OPT_NUMBER_H
#define SEEN_NUMBER_OPT_NUMBER_H

/** \file
 * <number-opt-number> implementation.
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib.h>
#include <glib/gprintf.h>
//todo: use glib instead of stdlib
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

class NumberOptNumber {

public:

    gfloat number; 

    gfloat optNumber;

    guint _set : 1;

    guint optNumber_set : 1;

    NumberOptNumber()
    {
        number = 0.0;
        optNumber = 0.0;

        _set = FALSE;
        optNumber_set = FALSE;
    }

    gfloat getNumber()
    {
        if(_set)
            return number;
        return -1;
    }

    gfloat getOptNumber()
    {
        if(optNumber_set)
            return optNumber;
        return -1;
    }

    void setOptNumber(gfloat num)
    {
        optNumber_set = true;
        optNumber = num;
    }

    void setNumber(gfloat num)
    {
        _set = true;
        number = num;
    }

    gchar *getValueString(gchar *str)
    {
        if( _set )
        {

            if( optNumber_set )
            {
                g_sprintf(str, "%lf %lf", number, optNumber);
            }
            else {
                g_sprintf(str, "%lf", number);
            }
        }
        return str;
    }

    void set(gchar const *str)
    {
        if(!str)
            return;

        gchar **values = g_strsplit(str, " ", 2);

        if( values[0] != NULL )
        {
            sscanf(values[0], "%f", &number);
            _set = TRUE;

            if( values[1] != NULL )
            {
  //              optNumber = g_ascii_strtod(values[1], NULL);
				sscanf(values[1], "%f", &optNumber);
                optNumber_set = TRUE;
            }
            else
                optNumber_set = FALSE;
        }
        else {
                _set = FALSE;
                optNumber_set = FALSE;
        }
    }

};

#endif /* !SEEN_NUMBER_OPT_NUMBER_H */

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
