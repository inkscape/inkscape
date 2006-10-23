#ifndef SEEN_AXONOMGRID_SNAPPER_H
#define SEEN_AXONOMGRID_SNAPPER_H

/**
 *  \file axonomgrid-snapper.h
 *  \brief Snapping things to axonometricgrids.
 *
 * Author:
 *   Johan Engelen <johan@shouraizou.nl>
 *
 * Copyright (C) 2006 Author
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "line-snapper.h"

namespace Inkscape
{

class AxonomGridSnapper : public LineSnapper                             
{                                                                       
public:                                                                
    AxonomGridSnapper(SPNamedView const *nv, NR::Coord const d);            

private:    
    LineList _getSnapLines(NR::Point const &p) const;
};

}

#endif    

