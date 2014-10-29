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
#include <cstdlib>

#include "svg/stringstream.h"

class NumberOptNumber {

public:

    float number; 

    float optNumber;

    unsigned int _set : 1;

    unsigned int optNumber_set : 1;

    NumberOptNumber()
    {
        number = 0.0;
        optNumber = 0.0;

        _set = FALSE;
        optNumber_set = FALSE;
    }

    float getNumber()
    {
        if(_set)
            return number;
        return -1;
    }

    float getOptNumber()
    {
        if(optNumber_set)
            return optNumber;
        return -1;
    }

    void setOptNumber(float num)
    {
        optNumber_set = true;
        optNumber = num;
    }

    void setNumber(float num)
    {
        _set = true;
        number = num;
    }

    bool optNumIsSet(){
        return optNumber_set;
    }

    bool numIsSet(){
        return _set;
    }
    
    char *getValueString()
    {
        Inkscape::SVGOStringStream os;

        if( _set )
        {

            if( optNumber_set )
            {
                os << number << " " << optNumber;
            }
            else {
                os << number;
            }
        }
        return g_strdup(os.str().c_str());
    }

    void set(char const *str)
    {
        if(!str)
            return;

        char **values = g_strsplit(str, " ", 2);

        if( values[0] != NULL )
        {
            number = g_ascii_strtod(values[0], NULL);
            _set = TRUE;

            if( values[1] != NULL )
            {
                optNumber = g_ascii_strtod(values[1], NULL);
                optNumber_set = TRUE;
            }
            else
                optNumber_set = FALSE;
        }
        else {
                _set = FALSE;
                optNumber_set = FALSE;
        }

        g_strfreev(values);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
