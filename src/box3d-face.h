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
    Box3DFace(SP3DBox *box, NR::Point &A, NR::Point &B, NR::Point &C, NR::Point &D,
	                    Box3D::Axis plane, Box3D::FrontOrRear rel_pos);
    Box3DFace(Box3DFace const &box3dface);
    ~Box3DFace();

    NR::Point operator[](unsigned int i);
    void draw(SP3DBox *box3d, SPCurve *c);

    /***
    void set_shape(NR::Point const ul, NR::Point const lr,
                   Box3D::Axis const dir1, Box3D::Axis const dir2,
                   unsigned int shift_count = 0, NR::Maybe<NR::Point> pt_align = NR::Nothing(),
                   bool align_along_PL = false);
    ***/
    void set_corners (NR::Point &A, NR::Point &B, NR::Point &C, NR::Point &D);
    void set_face (NR::Point const A, NR::Point const C, Box3D::Axis const dir1, Box3D::Axis const dir2);
    
    void hook_path_to_3dbox();
    void set_path_repr();
    void set_curve();
    gchar * svg_repr_string();

private:
    NR::Point *corners[4];

    Box3D::Axis dir1;
    Box3D::Axis dir2;
    
    SPPath *path;
    SP3DBox *parent_box3d;
};

#endif
