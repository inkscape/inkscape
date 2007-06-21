#ifndef __SP_3DBOX_FACE_H__
#define __SP_3DBOX_FACE_H__

/*
 * Face of a 3D box ('perspectivic rectangle')
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007      Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "perspective-line.h"
#include "display/curve.h"
#include "sp-path.h"
#include "sp-object.h"
#include "inkscape.h"
#include "desktop-style.h"
#include "desktop.h"
#include "xml/document.h"

class SP3DBox;

class Box3DFace {
public:
    Box3DFace(SP3DBox *box3d);
    //Box3DFace(SP3DBox *box3d, NR::Point const ul, NR::Point const lr,
    //          Box3D::PerspDir const dir1, Box3D::PerspDir const dir2,
    //          unsigned int shift_count = 0, NR::Maybe<NR::Point> pt_align = NR::Nothing(), bool align_along_PL = false);
    Box3DFace(Box3DFace const &box3dface);
    NR::Point operator[](unsigned int i);
    void draw(SP3DBox *box3d, SPCurve *c);

    void set_shape(NR::Point const ul, NR::Point const lr,
                   Box3D::PerspDir const dir1, Box3D::PerspDir const dir2,
                   unsigned int shift_count = 0, NR::Maybe<NR::Point> pt_align = NR::Nothing(),
                   bool align_along_PL = false);
    
    void hook_path_to_3dbox();
    void set_path_repr();
    void set_curve();
    gchar * svg_repr_string();

private:
    NR::Point corner1;
    NR::Point corner2;
    NR::Point corner3;
    NR::Point corner4;

    Box3D::PerspDir dir1;
    Box3D::PerspDir dir2;
    
    SPPath *path;
    SP3DBox *parent_box3d; // the parent box
};

#endif
