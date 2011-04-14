/* Copyright (C) 2001-2010 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

/* $Id: decompose.h 227 2010-12-16 05:47:19Z selinger $ */

#ifndef DECOMPOSE_H
#define DECOMPOSE_H

#include "potracelib.h"
#include "progress.h"
#include "curve.h"

int bm_to_pathlist(const potrace_bitmap_t *bm, path_t **plistp, const potrace_param_t *param, progress_t *progress);

#endif /* DECOMPOSE_H */

